#include "stdafx.h"
#include "IocpManager.h"
#include "Connection.h"
#include "MiniDump.h"
#include "Log.h"
#include "SendThread.h"
#ifdef _USE_ACCEPTEX
#include "SocketContextMgr.h"
#include "AcceptorEx.h"
#endif
#if defined(_USE_ACCEPTEX)
#else
#include "EventAcceptor.h"
#endif //#if defined(_USE_ACCEPTEX)
#include "VarArg.h"
#include <fstream>
#if !defined(_WIN32)
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <ifaddrs.h>
#include <mutex>
#include <queue>
#include <deque>
#include <unordered_map>
#endif
#if defined (_LOGINSERVER)
extern TLoginConfig g_Config;
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#if defined(_SERVER)

#if !defined(_WIN32)
struct CIocpManager::LinuxEpollContext
{
	int epollFd = -1;
	int wakeFd  = -1;

	struct CustomItem
	{
		CSocketContext* key = nullptr;
		TIOContext*     io  = nullptr;
		DWORD           bytes = 0;
		bool            ok = true;
	};
	std::mutex             queueMtx;
	std::queue<CustomItem> customQueue;

	struct PendingSend
	{
		TIOContext* io     = nullptr;
		size_t      offset = 0;
		size_t      total  = 0;
	};
	struct SockState
	{
		bool recvArmed  = false;
		bool connecting = false;
		std::deque<PendingSend> sendQueue;
	};
	std::mutex sockMtx;
	std::unordered_map<CSocketContext*, SockState> sockStates;

	bool Init()
	{
		epollFd = ::epoll_create1(EPOLL_CLOEXEC);
		if (epollFd < 0) return false;
		wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
		if (wakeFd < 0)
		{
			::close(epollFd);
			epollFd = -1;
			return false;
		}
		struct epoll_event ev{};
		ev.events = EPOLLIN;
		ev.data.ptr = nullptr;
		::epoll_ctl(epollFd, EPOLL_CTL_ADD, wakeFd, &ev);
		return true;
	}

	void Close()
	{
		if (wakeFd >= 0)  { ::close(wakeFd);  wakeFd = -1; }
		if (epollFd >= 0) { ::close(epollFd); epollFd = -1; }
		std::lock_guard<std::mutex> lk(sockMtx);
		sockStates.clear();
	}

	void Wake()
	{
		if (wakeFd >= 0)
		{
			uint64_t one = 1;
			(void)::write(wakeFd, &one, sizeof(one));
		}
	}

	void PushCustom(CSocketContext* key, TIOContext* io, DWORD bytes, bool ok = true)
	{
		{
			std::lock_guard<std::mutex> lk(queueMtx);
			CustomItem item;
			item.key   = key;
			item.io    = io;
			item.bytes = bytes;
			item.ok    = ok;
			customQueue.push(item);
		}
		Wake();
	}

	bool PopCustom(CustomItem& out)
	{
		std::lock_guard<std::mutex> lk(queueMtx);
		if (customQueue.empty()) return false;
		out = customQueue.front();
		customQueue.pop();
		return true;
	}

	void UpdateEpollLocked(CSocketContext* ctx, const SockState& st, bool preferAdd)
	{
		if (epollFd < 0 || !ctx || ctx->m_Socket == INVALID_SOCKET) return;
		uint32_t mask = EPOLLRDHUP | EPOLLONESHOT;
		if (st.connecting)
		{
			mask |= EPOLLOUT;
		}
		else
		{
			if (st.recvArmed) mask |= EPOLLIN;
			if (!st.sendQueue.empty()) mask |= EPOLLOUT;
		}
		struct epoll_event ev{};
		ev.events   = mask;
		ev.data.ptr = ctx;
		if (preferAdd)
		{
			if (::epoll_ctl(epollFd, EPOLL_CTL_ADD, ctx->m_Socket, &ev) != 0 && errno == EEXIST)
				::epoll_ctl(epollFd, EPOLL_CTL_MOD, ctx->m_Socket, &ev);
		}
		else
		{
			if (::epoll_ctl(epollFd, EPOLL_CTL_MOD, ctx->m_Socket, &ev) != 0 && errno == ENOENT)
				::epoll_ctl(epollFd, EPOLL_CTL_ADD, ctx->m_Socket, &ev);
		}
	}

	void RemoveSocket(CSocketContext* ctx)
	{
		if (!ctx) return;
		std::lock_guard<std::mutex> lk(sockMtx);
		sockStates.erase(ctx);
		if (epollFd >= 0 && ctx->m_Socket != INVALID_SOCKET)
			::epoll_ctl(epollFd, EPOLL_CTL_DEL, ctx->m_Socket, nullptr);
	}
};

void CIocpManager::PostCustomCompletion(DWORD dwBytesTransferred, CSocketContext* pSocketContext, TIOContext* pIOContext)
{
	if (m_pLinuxEpoll)
		m_pLinuxEpoll->PushCustom(pSocketContext, pIOContext, dwBytesTransferred, true);
}
#endif

CIocpManager::CIocpManager(void)
:
#if !defined(_WIN32)
  m_pLinuxEpoll(nullptr),
#endif
  m_WaitThreadCount(0), m_SendIOCount(0), m_pSendThread(NULL), m_hIOCP(INVALID_HANDLE_VALUE)
{
	m_bThreadSwitch = true;
	m_nAddSendBufSize = 0;
	m_nPostSendSize = 0;		//buffer pop
	m_nRealSendSize = 0;
	m_nAddRecvBufSize = 0;
	m_nPostRecvBufSize = 0;
	m_nProcessBufSize = 0;
	m_pAcceptor = NULL;
}

CIocpManager::~CIocpManager(void)
{
	Final();
}

int CIocpManager::Init(int nSocketCountMax, int nWorkerThreadSize)
{
	WSADATA WsaData;
	int Ret = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (Ret < 0) return -1;

#if !defined(_WIN32)
	m_pLinuxEpoll = new LinuxEpollContext();
	if (!m_pLinuxEpoll->Init())
	{
		delete m_pLinuxEpoll;
		m_pLinuxEpoll = nullptr;
		return -1;
	}
	m_hIOCP = reinterpret_cast<HANDLE>(m_pLinuxEpoll);
#else
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIOCP == NULL) return -1;
#endif

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	int Count = nWorkerThreadSize == 0 ? (SysInfo.dwNumberOfProcessors * 2) + 1 : nWorkerThreadSize;

	_GetHostIPAddress();
#ifdef _USE_ACCEPTEX
	m_uiWorkerThreadCount = Count;
	if( !CSocketContextMgr::CreateInstance( this ) )
		return -1;
	if( !CSocketContextMgr::GetInstance().bInitialize() )
		return -1;
	if( !CAcceptorEx::CreateInstance( this ) )
		return -1;
#else
	for( int i=0 ; i<nSocketCountMax ; ++i)
	{
		CSocketContext* pSocketContext = new CSocketContext;
		if (pSocketContext)
			m_SocketContexts.push_back(pSocketContext);
	}
#endif

	for( int i=0 ; i<Count; ++i )
	{
		HANDLE handle;
		UINT nThreadID;
		handle = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, this, 0, &nThreadID);
#ifdef _USE_ACCEPTEX
		m_vWorkerThreadHandle.push_back( handle );
#else
		if (handle != INVALID_HANDLE_VALUE)
			CloseHandle(handle);
#endif
	}

	// Create SendThread
	m_pSendThread = new CSendThread;
	if( m_pSendThread == NULL || !m_pSendThread->Start() )
		return -1;
#if defined(_USE_ACCEPTEX)
#else
	m_pAcceptor = new EventAcceptor(this);
	// Start Accept Thread.
	m_pAcceptor->Start();
#endif //#if defined(_USE_ACCEPTEX)
	return 0;
}

void CIocpManager::Final()
{
	// Terminate SendThread
	SAFE_DELETE( m_pSendThread );

	// Clean up acceptors first
	CloseAcceptors();
#if defined(_USE_ACCEPTEX)
#else
	SAFE_DELETE( m_pAcceptor );	
#endif //#if defined(_USE_ACCEPTEX)

	if (!m_SocketContexts.empty()){
		TSocketContextList::iterator iter;
		for (iter = m_SocketContexts.begin(); iter != m_SocketContexts.end(); ++iter){
			SAFE_DELETE(*iter);
		}
		m_SocketContexts.clear();
	}

#if !defined(_WIN32)
	if (m_pLinuxEpoll)
	{
		m_pLinuxEpoll->Close();
		delete m_pLinuxEpoll;
		m_pLinuxEpoll = nullptr;
		m_hIOCP = INVALID_HANDLE_VALUE;
	}
#else
	if (m_hIOCP != INVALID_HANDLE_VALUE){
		CloseHandle(m_hIOCP);
		m_hIOCP = INVALID_HANDLE_VALUE;
	}
#endif

	WSACleanup();
}

int CIocpManager::AttachSocket(CSocketContext *pSocketContext)
{
	if (pSocketContext->m_Socket == INVALID_SOCKET) 
		return -1;

	// 2009.02.03 kimbap
	// Call BufferClear() before AttachSocket. Moved here from OnAccept()
	//{
		CConnection* pConnection = static_cast<CConnection*>(pSocketContext->GetParam());
		if( pConnection == NULL )
			return -1;

		pConnection->BufferClear();
	//}
	///////////////////////////////////////////////////////////////////////////////////////////////
	if( pConnection->GetIocpHandle() == INVALID_HANDLE_VALUE )	
	{
#if !defined(_WIN32)
		unsigned long nb = 1;
		ioctlsocket(pSocketContext->m_Socket, FIONBIO, &nb);
		pConnection->SetIocpHandle(m_hIOCP);
#else
		HANDLE handle = CreateIoCompletionPort((HANDLE)pSocketContext->m_Socket, m_hIOCP, (ULONG_PTR)pSocketContext, 0);
		if (!handle) 
			return -1;
		pConnection->SetIocpHandle(handle);
#endif
	}

	int Ret = PostRecv(pSocketContext);
	pSocketContext->DelRef();

	return Ret;
}

int CIocpManager::DetachSocket(CSocketContext *pSocketContext, wchar_t *pwszIdent)
{
	if (pSocketContext == NULL)
	{
		return 0;
	}

	if (pSocketContext->DelRef() == 0)	
	{
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
		if( !pSocketContext->Detach() )
			return 0;	//Already detached
#endif
		bool bRet = DelSocket(pSocketContext);
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		if( bRet && pwszIdent && wcscmp( pwszIdent, L"PostSend") == 0 )
			return IN_DISCONNECT;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		std::wstring wstrDetachReason;
		pSocketContext->GetDetachReason(wstrDetachReason);
		if( wstrDetachReason.length() == 0)	
			pSocketContext->SetDetachReason(pwszIdent);
		OnDisconnected(pSocketContext);
	}

	return 0;
}

bool CIocpManager::DelSocket(CSocketContext *pSocketContext)
{
	if (pSocketContext->m_Socket != INVALID_SOCKET)
	{
#if !defined(_WIN32)
		if (m_pLinuxEpoll)
			m_pLinuxEpoll->RemoveSocket(pSocketContext);
#endif
		closesocket(pSocketContext->m_Socket);
		pSocketContext->m_Socket = INVALID_SOCKET;	
		return true;
	}	
	return false;
}

int CIocpManager::AddAcceptConnection(const int nKey, const int nPort, int nBackLog)
{
#ifdef _USE_ACCEPTEX
	UINT uiPoolSize = (nKey == CONNECTIONKEY_USER) ? g_Config.nIocpMax : 10;

	if( !CAcceptorEx::GetInstance().bCreateListenSocket( nPort, uiPoolSize, nKey ) )
		return -1;
#else
	if ( m_pAcceptor->Open(nKey, nPort, nBackLog) < 0 )
		return -1;
#endif // #ifdef _USE_ACCEPTEX
	return 0;
}

void CIocpManager::_OnConnect( CSocketContext* pSocketContext )
{
	// Check if connection succeeded.
#if !defined(_WIN32)
	int soErr = -1;
	socklen_t errLen = sizeof(soErr);
	int err = getsockopt(pSocketContext->m_Socket, SOL_SOCKET, SO_ERROR, &soErr, &errLen);
	bool connectedOk = (err == 0 && soErr == 0);
	if (connectedOk)
	{
		{
			CConnection* pConnection = static_cast<CConnection*>(pSocketContext->GetParam());
			if( !pConnection )
				return;
#else
	int nSecond;
	int nByte = sizeof(nSecond);	
	int err = getsockopt( pSocketContext->m_Socket, SOL_SOCKET, SO_CONNECT_TIME, (char *)&nSecond, (PINT)&nByte );
	if ( err == NO_ERROR ) 
	{
		if (nSecond != 0xFFFFFFFF) // 0xFFFFFFFF indicates connection failure
		{
			// Connection succeeded
			CConnection* pConnection = static_cast<CConnection*>(pSocketContext->GetParam());
			if( !pConnection )
				return;

			setsockopt( pSocketContext->m_Socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0 );
#endif

			pSocketContext->m_RecvIO.mode = IOPOST_RECV;

			OnAccept(pSocketContext, pConnection->GetIp(), pConnection->GetPort());

			if (!pSocketContext->GetParam())
			{
				OnConnectFail(pSocketContext);
				closesocket(pSocketContext->m_Socket);
				ClearSocketContext(pSocketContext);
				return;
			}

			pSocketContext->IncRef();

			if (AttachSocket(pSocketContext) < 0)
			{
				OnConnectFail(pSocketContext);
				closesocket(pSocketContext->m_Socket);
				OnDisconnected(pSocketContext);				
				return;
			}

			pConnection->SetConnecting(false);
			OnConnected(pSocketContext);			
			return;
		}		
	}
	// Connection failed here.
	OnConnectFail(pSocketContext);
	closesocket(pSocketContext->m_Socket);
	ClearSocketContext(pSocketContext);
	return;
}

int CIocpManager::AddConnectionEx(CConnection* pCon, const int nKey, const char *pIp, const int nPort)
{
#if !defined(_WIN32)
	SOCKET Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET) return -1;
	if (!SetSocketOption(Socket)) { closesocket(Socket); return -1; }
	unsigned long nb = 1;
	ioctlsocket(Socket, FIONBIO, &nb);

	sockaddr_in targetAddr{};
	targetAddr.sin_family = AF_INET;
	targetAddr.sin_addr.s_addr = inet_addr(pIp);
	targetAddr.sin_port = htons(nPort);

#ifdef _USE_ACCEPTEX
	CSocketContext* pSocketContext = CSocketContextMgr::GetInstance().pGetSocketContext();
	if (!pSocketContext) { closesocket(Socket); return -1; }
	pSocketContext->m_Socket     = Socket;
	pSocketContext->m_dwKeyParam = nKey;
	pSocketContext->m_RecvIO.mode = IOPOST_CONNECT;
	pSocketContext->m_RecvIO.Len  = 0;
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
	{
		pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
		pSocketContext->m_SendIOList[i]->Len  = 0;
	}
#else
	pSocketContext->m_SendIO.mode = IOPOST_SEND;
	pSocketContext->m_SendIO.Len  = 0;
#endif
	pSocketContext->SetListenID(1000);
#else
	CSocketContext* pSocketContext = GetSocketContext(nKey, Socket, &targetAddr);
	if (!pSocketContext) { closesocket(Socket); return -1; }
	pSocketContext->m_RecvIO.mode = IOPOST_CONNECT;
#endif
	pSocketContext->SetParam(pCon);
	pCon->SetIocpHandle(m_hIOCP);

	int rc = ::connect(Socket, reinterpret_cast<struct sockaddr*>(&targetAddr), sizeof(targetAddr));
	if (rc == 0)
	{
		if (m_pLinuxEpoll)
			m_pLinuxEpoll->PushCustom(pSocketContext, &pSocketContext->m_RecvIO, 0, true);
		return 0;
	}
	if (errno == EINPROGRESS)
	{
		if (m_pLinuxEpoll)
		{
			std::lock_guard<std::mutex> lk(m_pLinuxEpoll->sockMtx);
			auto& st = m_pLinuxEpoll->sockStates[pSocketContext];
			st.connecting = true;
			m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, true);
		}
		return 0;
	}
	closesocket(Socket);
	PutSocketContext(pSocketContext);
	return -1;
#else
	SOCKET Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == INVALID_SOCKET) return -1;

	if (!SetSocketOption(Socket)) return -1;

	LPFN_CONNECTEX lpfnConnectEx = NULL;
	DWORD dwBytes = 0;
	GUID GuidConnectEx = WSAID_CONNECTEX;

	int nRet = ::WSAIoctl( Socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidConnectEx, sizeof(GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), &dwBytes, NULL, NULL);
	if( nRet == SOCKET_ERROR ) 
	{
		closesocket(Socket);
		return -1;
	}

	sockaddr_in localAddr;
	memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;

	// Bind with the created socket..
	nRet = ::bind(Socket, (struct sockaddr*)&localAddr, sizeof(localAddr));
	if( nRet == SOCKET_ERROR ) 
	{
		closesocket(Socket);
		return -1;
	}

	sockaddr_in targetAddr;
	targetAddr.sin_family = AF_INET;
	targetAddr.sin_addr.s_addr = inet_addr(pIp);
	targetAddr.sin_port = htons(nPort);	

#ifdef _USE_ACCEPTEX
	CSocketContext* pSocketContext = CSocketContextMgr::GetInstance().pGetSocketContext();
	if( !pSocketContext )
	{
		closesocket( Socket );
		return -1;
	}
	pSocketContext->m_Socket		= Socket;
	pSocketContext->m_dwKeyParam	= nKey;
	pSocketContext->m_RecvIO.mode	= IOPOST_RECV;
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
	{
		pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
		pSocketContext->m_SendIOList[i]->Len	= 0;
	}
#else
	pSocketContext->m_SendIO.mode	= IOPOST_SEND;
	pSocketContext->m_SendIO.Len	= 0;
#endif

	pSocketContext->m_RecvIO.Len	= 0;

	pSocketContext->SetListenID( 1000 );
#else
	CSocketContext *pSocketContext = GetSocketContext(nKey, Socket, &targetAddr);
	if (!pSocketContext){
		closesocket(Socket);
		return -1;
	}
#endif

	pSocketContext->SetParam(pCon);

	HANDLE handle = CreateIoCompletionPort((HANDLE)Socket, m_hIOCP, (ULONG_PTR)pSocketContext, 0);
	if (!handle) 
	{
		closesocket(Socket);
		return -1;
	}
	pCon->SetIocpHandle(handle);

	//Post as RecvIO..
	pSocketContext->m_RecvIO.mode = IOPOST_CONNECT;
	BOOL bRet = lpfnConnectEx( Socket, (struct sockaddr*)&targetAddr, sizeof(struct sockaddr), NULL, 0, NULL, (LPWSAOVERLAPPED)&pSocketContext->m_RecvIO);	
	if (bRet == FALSE)	{
		nRet = WSAGetLastError();
		if (nRet != WSA_IO_PENDING)
		{
			closesocket(Socket);
			PutSocketContext(pSocketContext);
			return -1;
		}
	}
	
	return 0;	
#endif // _WIN32
}

int CIocpManager::AddConnection(const int nKey, const char *pIp, const int nPort)
{
	SOCKET Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == INVALID_SOCKET) return -1;

	if (!SetSocketOption(Socket)) return -1;

	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(pIp);
	sa.sin_port = htons(nPort);

	if (connect(Socket, (sockaddr*)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR){
		closesocket(Socket);
		return -1;
	}

#ifdef _USE_ACCEPTEX
	CSocketContext* pSocketContext = CSocketContextMgr::GetInstance().pGetSocketContext();
	if( !pSocketContext )
	{
		closesocket( Socket );
		return -1;
	}
	pSocketContext->m_Socket		= Socket;
	pSocketContext->m_dwKeyParam	= nKey;
	pSocketContext->m_RecvIO.mode	= IOPOST_RECV;
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
	{
		pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
		pSocketContext->m_SendIOList[i]->Len	= 0;
	}
#else
	pSocketContext->m_SendIO.mode	= IOPOST_SEND;
	pSocketContext->m_SendIO.Len	= 0;
#endif
	
	pSocketContext->m_RecvIO.Len	= 0;

	pSocketContext->SetListenID( 1000 );
#else
	CSocketContext *pSocketContext = GetSocketContext(nKey, Socket, &sa);
	if (!pSocketContext){
		closesocket(Socket);
		return -1;
	}
#endif

	OnAccept(pSocketContext, pIp, nPort);

	if (!pSocketContext->GetParam()){
		closesocket(Socket);
		PutSocketContext(pSocketContext);
		return -1;
	}

	pSocketContext->IncRef();

	if (AttachSocket(pSocketContext) < 0){
		closesocket(Socket);
		OnDisconnected(pSocketContext);
		PutSocketContext(pSocketContext);
		return -1;
	}

	OnConnected(pSocketContext);

	return 0;
}

int CIocpManager::CreateConnection(const int nKey, const char * pIp, const int nPort, UINT nSessionKey)
{
	SOCKET Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == INVALID_SOCKET) return -1;

	if (!SetSocketOption(Socket)) return -1;

	sockaddr_in sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(pIp);
	sa.sin_port = htons(nPort);

	if (connect(Socket, (sockaddr*)&sa, sizeof(sockaddr_in)) == SOCKET_ERROR){
		closesocket(Socket);
		return -1;
	}

	CSocketContext *pSocketContext = GetSocketContext(nKey, Socket, &sa);
	if (!pSocketContext){
		closesocket(Socket);
		return -1;
	}

	OnAccept(pSocketContext, pIp, nPort, nSessionKey);

	if (!pSocketContext->GetParam()){
		closesocket(Socket);
		PutSocketContext(pSocketContext);
		return -1;
	}

	pSocketContext->IncRef();

	if (AttachSocket(pSocketContext) < 0){
		closesocket(Socket);
		OnDisconnected(pSocketContext);
		PutSocketContext(pSocketContext);
		return -1;
	}
	return 0;
}

bool CIocpManager::SetSocketOption (SOCKET Socket)
{
	int boNoDelay = 1;
	if (setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&boNoDelay, sizeof(boNoDelay)) == SOCKET_ERROR){
		return false;
	}

	LINGER Linger;
	Linger.l_onoff = 1;
	Linger.l_linger = 0;

	if (setsockopt(Socket, SOL_SOCKET, SO_LINGER, (char*)&Linger, sizeof(LINGER)) == SOCKET_ERROR){
		return false;
	}

	int SendBuf = 51200;
	if (setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (char*)&SendBuf, sizeof(SendBuf)) == SOCKET_ERROR){
		return false;
	}
	return true;
}

int CIocpManager::PostRecv(CSocketContext *pSocketContext)
{
	if (pSocketContext->m_Socket == INVALID_SOCKET) return -1;
	if (pSocketContext->AddRef() == 0) return -1;

	if( pSocketContext->m_RecvIO.Len >= INTERNALBUFFERLENMAX )
	{
		DetachSocket(pSocketContext, L"PostRecv-BufferOverflow");
		return -1;
	}

#if !defined(_WIN32)
	if (!m_pLinuxEpoll)
	{
		DetachSocket(pSocketContext, L"PostRecv");
		return -1;
	}
	{
		std::lock_guard<std::mutex> lk(m_pLinuxEpoll->sockMtx);
		if (pSocketContext->m_Socket == INVALID_SOCKET)
		{
			// Socket closed concurrently
		}
		else
		{
			auto& st = m_pLinuxEpoll->sockStates[pSocketContext];
			st.recvArmed = true;
			m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, false);
			return 0;
		}
	}
	DetachSocket(pSocketContext, L"PostRecv");
	return -1;
#else
	WSABUF wsabuf;
	wsabuf.buf = pSocketContext->m_RecvIO.buffer + pSocketContext->m_RecvIO.Len;
	wsabuf.len = INTERNALBUFFERLENMAX - pSocketContext->m_RecvIO.Len;

	DWORD dwBytesRecvd = 0, dwFlags = 0;
	int Ret = WSARecv(pSocketContext->m_Socket, &wsabuf, 1, &dwBytesRecvd, &dwFlags, (LPWSAOVERLAPPED)&pSocketContext->m_RecvIO, NULL);
	if (Ret == SOCKET_ERROR){
		Ret = WSAGetLastError();
		if (Ret != WSA_IO_PENDING){
			DetachSocket(pSocketContext, L"PostRecv");
			return -1;
		}
	}

	return 0;
#endif
}

#ifdef _USE_SENDCONTEXTPOOL
int CIocpManager::PostSend(CSocketContext *pSocketContext, TIOContext * pContext)
{
	//Synchronized at a higher level!
	if (pSocketContext->m_Socket == INVALID_SOCKET || pSocketContext->AddRef() == 0 || pContext->Len <= 0)
		return -1;

#if !defined(_WIN32)
	if (!m_pLinuxEpoll)
	{
		DetachSocket(pSocketContext, L"PostSend");
		return -1;
	}
	size_t totalLen = static_cast<size_t>(pContext->Len);
	bool sendErr = false;
	bool completedNow = false;
	{
		std::lock_guard<std::mutex> lk(m_pLinuxEpoll->sockMtx);
		SOCKET sock = pSocketContext->m_Socket;
		if (sock == INVALID_SOCKET)
		{
			sendErr = true;
		}
		else
		{
			auto& st = m_pLinuxEpoll->sockStates[pSocketContext];
			if (st.sendQueue.empty())
			{
				size_t sentTotal = 0;
				while (sentTotal < totalLen)
				{
					ssize_t n = ::send(sock, pContext->buffer + sentTotal, totalLen - sentTotal, MSG_NOSIGNAL);
					if (n > 0)
					{
						sentTotal += static_cast<size_t>(n);
					}
					else if (n < 0 && errno == EINTR)
					{
						continue;
					}
					else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
					{
						LinuxEpollContext::PendingSend ps{ pContext, sentTotal, totalLen };
						st.sendQueue.push_back(ps);
						m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, false);
						break;
					}
					else
					{
						sendErr = true;
						break;
					}
				}
				if (!sendErr && sentTotal == totalLen)
					completedNow = true;
			}
			else
			{
				LinuxEpollContext::PendingSend ps{ pContext, 0, totalLen };
				st.sendQueue.push_back(ps);
				m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, false);
			}
		}
	}
	if (sendErr)
	{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		int DetachRet = DetachSocket(pSocketContext, L"PostSend");
		if (DetachRet == IN_DISCONNECT)
			return IN_DISCONNECT;
#else
		DetachSocket(pSocketContext, L"PostSend");
#endif
		return -1;
	}
	m_nRealSendSize += pContext->Len;
	InterlockedIncrement(&m_SendIOCount);
	if (completedNow)
		m_pLinuxEpoll->PushCustom(pSocketContext, pContext, static_cast<DWORD>(totalLen), true);
	return 0;
#else
	//Where physical sending is attempted. If async type, the completion message is handled by the worker.
	WSABUF wsabuf;
	wsabuf.buf = pContext->buffer;
	wsabuf.len = pContext->Len;

	DWORD dwBytesSent = 0, dwFlags = 0;
	int Ret = WSASend(pSocketContext->m_Socket, &wsabuf, 1, &dwBytesSent, dwFlags, (LPWSAOVERLAPPED)pContext, NULL);

	if (Ret == SOCKET_ERROR){
		Ret = WSAGetLastError();
		if (Ret != WSA_IO_PENDING){
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			int DetachRet = DetachSocket(pSocketContext, L"PostSend");
			if( DetachRet == IN_DISCONNECT )
				return IN_DISCONNECT;
#else
			DetachSocket(pSocketContext, L"PostSend");
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			return -1;
		}
	}

	//Test SendCount
	m_nRealSendSize += pContext->Len;

	InterlockedIncrement (&m_SendIOCount);	
	return 0;
#endif
}

void CIocpManager::SendComplete(CSocketContext *pSocketContext, TIOContext * pContext, int nSize)
{
	bool bDetach = false;
	{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
		if( Scope.bIsDelete() )
			return;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

		CConnection *pCon = (CConnection*)pSocketContext->GetParam();

		DecSendIOCount();

		if (pCon && pCon->SendComplete(pContext, nSize) == false)
			bDetach = true;
	}

	if( bDetach )
		DetachSocket(pSocketContext, L"IOPOST_SEND");
}
#else
int CIocpManager::PostSend(CSocketContext *pSocketContext)
{
	//Synchronized at a higher level!
	if (pSocketContext->m_Socket == INVALID_SOCKET || pSocketContext->AddRef() == 0 || pSocketContext->m_SendIO.Len == 0)
		return -1;

#if !defined(_WIN32)
	if (!m_pLinuxEpoll)
	{
		DetachSocket(pSocketContext, L"PostSend");
		return -1;
	}
	TIOContext* pContext = &pSocketContext->m_SendIO;
	size_t totalLen = static_cast<size_t>(pContext->Len);
	bool sendErr = false;
	bool completedNow = false;
	{
		std::lock_guard<std::mutex> lk(m_pLinuxEpoll->sockMtx);
		SOCKET sock = pSocketContext->m_Socket;
		if (sock == INVALID_SOCKET)
		{
			sendErr = true;
		}
		else
		{
			auto& st = m_pLinuxEpoll->sockStates[pSocketContext];
			if (st.sendQueue.empty())
			{
				size_t sentTotal = 0;
				while (sentTotal < totalLen)
				{
					ssize_t n = ::send(sock, pContext->buffer + sentTotal, totalLen - sentTotal, MSG_NOSIGNAL);
					if (n > 0)
					{
						sentTotal += static_cast<size_t>(n);
					}
					else if (n < 0 && errno == EINTR)
					{
						continue;
					}
					else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
					{
						LinuxEpollContext::PendingSend ps{ pContext, sentTotal, totalLen };
						st.sendQueue.push_back(ps);
						m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, false);
						break;
					}
					else
					{
						sendErr = true;
						break;
					}
				}
				if (!sendErr && sentTotal == totalLen)
					completedNow = true;
			}
			else
			{
				LinuxEpollContext::PendingSend ps{ pContext, 0, totalLen };
				st.sendQueue.push_back(ps);
				m_pLinuxEpoll->UpdateEpollLocked(pSocketContext, st, false);
			}
		}
	}
	if (sendErr)
	{
		DetachSocket(pSocketContext, L"PostSend");
		return -1;
	}
	m_nRealSendSize += pContext->Len;
	InterlockedIncrement(&m_SendIOCount);
	if (completedNow)
		m_pLinuxEpoll->PushCustom(pSocketContext, pContext, static_cast<DWORD>(totalLen), true);
	return 0;
#else
	//Where physical sending is attempted. If async type, the completion message is handled by the worker.
	WSABUF wsabuf;
	wsabuf.buf = pSocketContext->m_SendIO.buffer;
	wsabuf.len = pSocketContext->m_SendIO.Len;

	DWORD dwBytesSent = 0, dwFlags = 0;
	int Ret = WSASend(pSocketContext->m_Socket, &wsabuf, 1, &dwBytesSent, dwFlags, (LPWSAOVERLAPPED)&pSocketContext->m_SendIO, NULL);

	if (Ret == SOCKET_ERROR){
		Ret = WSAGetLastError();
		if (Ret != WSA_IO_PENDING){
			DetachSocket(pSocketContext, L"PostSend");
			return -1;
		}
	}

	//Test SendCount
	m_nRealSendSize += pSocketContext->m_SendIO.Len;

	InterlockedIncrement (&m_SendIOCount);	
	return 0;
#endif
}

void CIocpManager::SendComplete(CSocketContext *pSocketContext, int nSize)
{
	bool bDetach = false;
	{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
		if( Scope.bIsDelete() )
			return;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

		CConnection *pCon = (CConnection*)pSocketContext->GetParam();

		DecSendIOCount();
		if (pCon && pCon->SendComplete(nSize) == false)
			bDetach = true;
	}
	
	if( bDetach )
		DetachSocket(pSocketContext, L"IOPOST_SEND");
}
#endif

void CIocpManager::PutSocketContext(CSocketContext *pSocketContext)
{
	m_SocketLock.Lock();
	if( std::find( m_SocketContexts.begin(), m_SocketContexts.end(), pSocketContext ) == m_SocketContexts.end() )
		m_SocketContexts.push_back(pSocketContext);
	m_SocketLock.UnLock();
}

CSocketContext* CIocpManager::GetSocketContext (int nKey, SOCKET Socket, sockaddr_in *pAddr)
{
	CSocketContext *pSocketContext = NULL;

	m_SocketLock.Lock();
	if (!m_SocketContexts.empty()){
		pSocketContext = m_SocketContexts.front();
		m_SocketContexts.pop_front();

		pSocketContext->m_Socket = Socket;
#ifdef _USE_SENDCONTEXTPOOL
		pSocketContext->m_RecvIO.mode = IOPOST_RECV;
		pSocketContext->m_RecvIO.Len = 0;
		
		//Run it
		pSocketContext->m_SendIOLock.Lock();
		for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
		{
			pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
			pSocketContext->m_SendIOList[i]->Len = 0;
		}
		pSocketContext->m_SendIOLock.UnLock();
#else
		pSocketContext->m_RecvIO.mode = IOPOST_RECV;
		pSocketContext->m_RecvIO.Len = 0;
		pSocketContext->m_SendIO.mode = IOPOST_SEND;
		pSocketContext->m_SendIO.Len = 0;
#endif
		pSocketContext->m_dwKeyParam = nKey;
	}
	else
	{
		_DANGER_POINT();
	}

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	if( pSocketContext )
		pSocketContext->m_lActiveCount = 0;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	if( pSocketContext )
		pSocketContext->m_lDetached = 0;
#endif	//#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)

	m_SocketLock.UnLock();

	return pSocketContext;
}

void CIocpManager::ClearSocketContext(CSocketContext *pSocketContext)
{
#ifdef _USE_ACCEPTEX
	return;
#else
	if( pSocketContext )
	{
		pSocketContext->Clear();
		PutSocketContext(pSocketContext);
	}
	else
	{
		_DANGER_POINT();
	}
#endif
}

int CIocpManager::GetSocketContextCount()
{
	int nCount = 0;
	m_SocketLock.Lock();
	nCount = static_cast<int>(m_SocketContexts.size());
	m_SocketLock.UnLock();

	return nCount;
}

void CIocpManager::ThreadStop()
{
	m_bThreadSwitch = false;

#ifdef _USE_ACCEPTEX
	int Count = m_uiWorkerThreadCount;
#else
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	int Count = (SysInfo.dwNumberOfProcessors * 2) + 1;
#endif

	for( int i=0 ; i<Count ; ++i)
#if !defined(_WIN32)
		if (m_pLinuxEpoll) m_pLinuxEpoll->PushCustom(nullptr, nullptr, 0, true);
#else
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
#endif

#ifdef _USE_ACCEPTEX
	for( int i=0 ; i<Count ; ++i )
	{
		WaitForSingleObject( m_vWorkerThreadHandle[i], INFINITE );
		CloseHandle( m_vWorkerThreadHandle[i] );
	}
#endif
}

void CIocpManager::CloseAcceptors()
{
#ifdef _USE_ACCEPTEX

	SAFE_DELETE( m_pSendThread );

	if( CAcceptorEx::GetInstancePtr() )
		delete CAcceptorEx::GetInstancePtr();

	if( CSocketContextMgr::GetInstancePtr() )
		delete CSocketContextMgr::GetInstancePtr();
#else
	if( m_pAcceptor)
		m_pAcceptor->Close();
#endif
}

void CIocpManager::AddSendCall( CSocketContext* pSocketContext )
{
	if( m_pSendThread )
		m_pSendThread->PushSendQueue( pSocketContext );
	else
		_DANGER_POINT();
}

#ifdef _USE_ACCEPTEX

#include <mswsock.h>

#if !defined(_WIN32)
void CIocpManager::OnAcceptLinux( CSocketContext* pSocketContext, const sockaddr_in* pPeerAddr )
{
	if (!pSocketContext || !pPeerAddr) return;
	SOCKET socket = pSocketContext->m_Socket;
	pSocketContext->m_RecvIO.mode = IOPOST_RECV;
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
		pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
#else
	pSocketContext->m_SendIO.mode = IOPOST_SEND;
#endif
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	pSocketContext->m_lActiveCount = 0;
#endif
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	pSocketContext->m_lDetached = 0;
#endif
	OnAccept( pSocketContext, inet_ntoa(pPeerAddr->sin_addr), ntohs(pPeerAddr->sin_port) );
	if (pSocketContext->GetParam())
	{
		pSocketContext->IncRef();
		int Ret = AttachSocket( pSocketContext );
		if (Ret == 0)
		{
			OnConnected( pSocketContext );
		}
		else
		{
			closesocket( socket );
			OnDisconnected( pSocketContext );
			pSocketContext->Clear();
			PutSocketContext( pSocketContext );
		}
	}
	else
	{
		closesocket( socket );
		PutSocketContext( pSocketContext );
	}
}
void CIocpManager::_OnAccept( CSocketContext* ) {}
#else
void CIocpManager::_OnAccept( CSocketContext* pSocketContext )
{
	sockaddr_in* pLocalAddr = NULL;
	sockaddr_in* pPeerAddr  = NULL;
	INT32 iLocalLen, iPeerLen;

	GetAcceptExSockaddrs( pSocketContext->m_RecvIO.buffer, 0, sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, reinterpret_cast<sockaddr**>(&pLocalAddr), &iLocalLen, reinterpret_cast<sockaddr**>(&pPeerAddr), &iPeerLen );

	pSocketContext->OnAccept( pLocalAddr, pPeerAddr );
	
	SOCKET ListenSocket = CAcceptorEx::GetInstance().GetSocket( pSocketContext->uiGetListenID() );
	SOCKET socket		= pSocketContext->m_Socket;

	setsockopt( socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&ListenSocket), sizeof(SOCKET) );
	//CreateIoCompletionPort( (HANDLE)socket, m_hIOCP, (ULONG_PTR)pSocketContext, 0 );

	CAcceptorEx::GetInstance().bOnAccept( pSocketContext->uiGetListenID() );

	// Configure KeepAlive
	/*
	tcp_keepalive sKeepAlive;
	sKeepAlive.onoff				= 1;
	sKeepAlive.keepalivetime		= 10000;
	sKeepAlive.keepaliveinterval	= 1000;
	DWORD dwBytesReturned	= 0;
	WSAIoctl( socket, SIO_KEEPALIVE_VALS, &sKeepAlive, sizeof(sKeepAlive), 0, 0, &dwBytesReturned, 0, 0 );
	*/
	
	pSocketContext->m_RecvIO.mode	= IOPOST_RECV;	
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)pSocketContext->m_SendIOList.size(); i++)
		pSocketContext->m_SendIOList[i]->mode = IOPOST_SEND;
#else
	pSocketContext->m_SendIO.mode	= IOPOST_SEND;
#endif
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	pSocketContext->m_lActiveCount = 0;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	pSocketContext->m_lDetached = 0;
#endif	//#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)


	OnAccept( pSocketContext, inet_ntoa(pPeerAddr->sin_addr), ntohs(pPeerAddr->sin_port) );
	if( pSocketContext->GetParam() )
	{
		pSocketContext->IncRef();

		int Ret = AttachSocket( pSocketContext );
		if (Ret == 0)
		{
			OnConnected( pSocketContext );
		}
		else 
		{
			closesocket( socket );
			OnDisconnected( pSocketContext );
			pSocketContext->Clear();
			PutSocketContext( pSocketContext );
#if defined(_FINAL_BUILD)
			g_Log.Log( LogType::_NORMAL, L"[CIocpManager::_OnAccept] closesocket - AttachSocket\r\n");
#endif	// #if defined(_FINAL_BUILD)
		}
	}
	else 
	{
		closesocket( socket );
		PutSocketContext( pSocketContext );
#if defined(_FINAL_BUILD)
		g_Log.Log( LogType::_NORMAL, L"[CIocpManager::_OnAccept] closesocket - pSocketContext->GetParam()\r\n");
#endif	// #if defined(_FINAL_BUILD)
	}


	/*
	SOCKET sListenSocket = CAcceptor<T>::GetInstance().GetListenSocket();
	SOCKET sSocket = _pcOV->GetSocket();
	setsockopt( sSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&sListenSocket), sizeof(SOCKET) );
	CreateIoCompletionPort( (HANDLE)sSocket, m_hIOCP, (ULONG_PTR)_pcOV, 0 );

	CAcceptor<T>::GetInstance().OnAccept( _pcOV->stGetListenID() );

	Util::CS cs( &m_cConnectListCS );

	OnBeginningRecvPost( _pcOV );

	return m_fptFunc[Common::FuncPtr::eAccept_Index].Call( _pcOV, 0, 0 );
	*/
}
#endif // _WIN32

#endif

void CIocpManager::OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort)
{
}

void CIocpManager::OnConnected(CSocketContext *pSocketContext)
{
}

void CIocpManager::OnConnectFail(CSocketContext *pSocketContext)
{	
	CConnection* pConnection = static_cast<CConnection*>(pSocketContext->GetParam());
	if( !pConnection )
		return;

	pConnection->SetConnecting(false);
}

void CIocpManager::OnDisconnected(CSocketContext *pSocketContext)
{
	CConnection *pCon = (CConnection*)pSocketContext->GetParam();

	if (pCon){
		ClearSocketContext(pSocketContext);
		pCon->SetSocketContext(NULL, NULL);
		delete pCon;
		pCon = NULL;
	}
}

UINT __stdcall CIocpManager::WorkerThread(void *pParam)
{
	g_Log.Log(LogType::_FILELOG, L"[Thread-Start] WorkerThread - TID : %d\r\n", ::GetCurrentThreadId());

	CIocpManager *pIOCP = (CIocpManager*)pParam;
#if !defined(_WIN32)
	LinuxEpollContext* pEpoll = pIOCP->m_pLinuxEpoll;
	while (pIOCP->m_bThreadSwitch && pEpoll)
	{
		LinuxEpollContext::CustomItem item;
		if (pEpoll->PopCustom(item))
		{
			if (!item.key)
				return 0;

			CSocketContext* pSocketContext = item.key;
			TIOContext* pIOContext = item.io;
			DWORD dwBytesTransferred = item.bytes;

			if (!pIOContext)
				continue;

			if (pIOContext->mode != IOPOST_CONNECT && dwBytesTransferred == 0)
			{
				pIOCP->DetachSocket(pSocketContext, L"GQCS");
				continue;
			}

			switch (pIOContext->mode)
			{
			case IOPOST_CONNECT:
				pIOCP->_OnConnect(pSocketContext);
				break;
			case IOPOST_RECV:
				if (&pSocketContext->m_RecvIO == pIOContext)
				{
					pSocketContext->m_RecvIO.Len += dwBytesTransferred;
					pIOCP->OnReceive(pSocketContext, dwBytesTransferred);
					pIOCP->PostRecv(pSocketContext);
					pIOCP->DetachSocket(pSocketContext, L"IOPOST_RECV");
				}
				break;
			case IOPOST_SEND:
#ifdef _USE_SENDCONTEXTPOOL
				if (pSocketContext == pIOContext->pSocketContext)
					pIOCP->SendComplete(pSocketContext, pIOContext, dwBytesTransferred);
#else
				if (&pSocketContext->m_SendIO == pIOContext)
					pIOCP->SendComplete(pSocketContext, dwBytesTransferred);
#endif
				break;
			}
			continue;
		}

		pIOCP->IncThreadCount();
		struct epoll_event ev{};
		int nReady = ::epoll_wait(pEpoll->epollFd, &ev, 1, 100);
		pIOCP->DecThreadCount();

		if (nReady <= 0)
			continue;

		if (ev.data.ptr == nullptr)
		{
			uint64_t val = 0;
			(void)::read(pEpoll->wakeFd, &val, sizeof(val));
			continue;
		}

		CSocketContext* pSocketContext = static_cast<CSocketContext*>(ev.data.ptr);
		bool doConnect = false;
		bool doRecv    = false;
		bool sendError = false;
		std::vector<std::pair<TIOContext*, int>> completedSends;

		{
			std::lock_guard<std::mutex> lk(pEpoll->sockMtx);
			auto it = pEpoll->sockStates.find(pSocketContext);
			if (it == pEpoll->sockStates.end())
				continue;

			auto& st = it->second;
			SOCKET sock = pSocketContext->m_Socket;
			if (st.connecting)
			{
				st.connecting = false;
				doConnect = true;
			}
			else
			{
				if ((ev.events & EPOLLOUT) && !st.sendQueue.empty() && sock != INVALID_SOCKET)
				{
					while (!st.sendQueue.empty())
					{
						auto& ps = st.sendQueue.front();
						while (ps.offset < ps.total)
						{
							ssize_t n = ::send(sock, ps.io->buffer + ps.offset, ps.total - ps.offset, MSG_NOSIGNAL);
							if (n > 0)
							{
								ps.offset += static_cast<size_t>(n);
							}
							else if (n < 0 && errno == EINTR)
							{
								continue;
							}
							else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
							{
								break;
							}
							else
							{
								sendError = true;
								break;
							}
						}
						if (sendError)
							break;
						if (ps.offset >= ps.total)
						{
							completedSends.push_back(std::make_pair(ps.io, static_cast<int>(ps.total)));
							st.sendQueue.pop_front();
						}
						else
						{
							break;
						}
					}
				}

				if ((ev.events & (EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP)) && st.recvArmed)
				{
					st.recvArmed = false;
					doRecv = true;
				}

				if (!sendError && (st.recvArmed || !st.sendQueue.empty()))
					pEpoll->UpdateEpollLocked(pSocketContext, st, false);
			}
		}

		if (doConnect)
		{
			pIOCP->_OnConnect(pSocketContext);
			continue;
		}

		for (size_t i = 0; i < completedSends.size(); ++i)
		{
#ifdef _USE_SENDCONTEXTPOOL
			pIOCP->SendComplete(pSocketContext, completedSends[i].first, completedSends[i].second);
#else
			pIOCP->SendComplete(pSocketContext, completedSends[i].second);
#endif
		}

		if (doRecv)
		{
			SOCKET sock = pSocketContext->m_Socket;
			if (sock == INVALID_SOCKET)
			{
				pIOCP->DetachSocket(pSocketContext, L"GQCS");
			}
			else
			{
				int space = INTERNALBUFFERLENMAX - pSocketContext->m_RecvIO.Len;
				if (space <= 0)
				{
					pIOCP->DetachSocket(pSocketContext, L"PostRecv-BufferOverflow");
				}
				else
				{
					ssize_t n = ::recv(sock, pSocketContext->m_RecvIO.buffer + pSocketContext->m_RecvIO.Len, static_cast<size_t>(space), 0);
					if (n > 0)
					{
						pSocketContext->m_RecvIO.Len += static_cast<int>(n);
						pIOCP->OnReceive(pSocketContext, static_cast<DWORD>(n));
						pIOCP->PostRecv(pSocketContext);
						pIOCP->DetachSocket(pSocketContext, L"IOPOST_RECV");
					}
					else if (n == 0)
					{
						pIOCP->DetachSocket(pSocketContext, L"GQCS");
					}
					else if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
					{
						std::lock_guard<std::mutex> lk(pEpoll->sockMtx);
						auto it = pEpoll->sockStates.find(pSocketContext);
						if (it != pEpoll->sockStates.end() && pSocketContext->m_Socket != INVALID_SOCKET)
						{
							it->second.recvArmed = true;
							pEpoll->UpdateEpollLocked(pSocketContext, it->second, false);
						}
					}
					else
					{
						pIOCP->DetachSocket(pSocketContext, L"GQCSError");
					}
				}
			}
		}
		else if (sendError)
		{
			pIOCP->DetachSocket(pSocketContext, L"PostSend");
		}
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] WorkerThread - TID : %d\r\n", ::GetCurrentThreadId());
	return 0;
#else
	HANDLE hIOCP = pIOCP->GetIocpHandle();
	int Ret = 0;
	CSocketContext *pSocketContext;
	TIOContext *pIOContext;
	DWORD dwBytesTransferred;

	while (pIOCP->m_bThreadSwitch)
	{
		__try {
			pIOCP->IncThreadCount();
			Ret = GetQueuedCompletionStatus(hIOCP, &dwBytesTransferred, (PULONG_PTR)&pSocketContext, (LPOVERLAPPED*)&pIOContext, INFINITE);
			pIOCP->DecThreadCount();

			if( Ret == FALSE )
			{
				if (pIOContext == NULL)
					continue;
				else if (pIOContext != NULL)
				{
	#ifdef _USE_ACCEPTEX
					if( !pSocketContext )
						continue;
	#endif
					int Error = WSAGetLastError();
					if( Error == ERROR_NETNAME_DELETED)
					{
						pIOCP->DetachSocket( pSocketContext, L"GQCSError" );
						continue;
					}
				}
			}

	#ifdef _USE_ACCEPTEX
			if( pIOContext && pIOContext->mode == IOPOST_ACCEPT )
				pSocketContext = pIOContext->pSocketContext;
	#endif
			if( !pSocketContext ) 
				return 0;
	#if defined(_USE_ACCEPTEX)		
			if( pIOContext->mode != IOPOST_CONNECT && pIOContext->mode != IOPOST_ACCEPT && dwBytesTransferred == 0)
	#else
			if( pIOContext->mode != IOPOST_CONNECT && dwBytesTransferred == 0)
	#endif
			{
				pIOCP->DetachSocket(pSocketContext, L"GQCS");
				continue;
			}

			switch (pIOContext->mode)
			{
	#ifdef _USE_ACCEPTEX
				case IOPOST_ACCEPT:
				{
					__try {
						pIOCP->_OnAccept( pSocketContext );
					}
					__except(CExceptionCodeLog(GetExceptionCode(), L"WorkerThread - IOPOST_ACCEPT"), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
						// This code block is never executed
					}
					break;
				}
	#endif
				case IOPOST_CONNECT :
					{
						__try{
							pIOCP->_OnConnect( pSocketContext );
						}
						__except(CExceptionCodeLog(GetExceptionCode(), L"WorkerThread - IOPOST_CONNECT"), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
							// This code block is never executed
						}
						break;
					}
				case IOPOST_RECV:
				{
					__try {
						if (&pSocketContext->m_RecvIO != pIOContext)
							continue;

						pSocketContext->m_RecvIO.Len += dwBytesTransferred;
						pIOCP->OnReceive(pSocketContext, dwBytesTransferred);
						pIOCP->PostRecv(pSocketContext);
						pIOCP->DetachSocket(pSocketContext, L"IOPOST_RECV");
					}
					__except(CExceptionCodeLog(GetExceptionCode(), L"WorkerThread - IOPOST_RECV"), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
						// This code block is never executed
					}
					break;
				}

				case IOPOST_SEND:
				{
					__try {

	#ifdef _USE_SENDCONTEXTPOOL
						if (pSocketContext != pIOContext->pSocketContext)
							continue;

						pIOCP->SendComplete(pSocketContext, pIOContext, dwBytesTransferred);
	#else
						if (&pSocketContext->m_SendIO != pIOContext)
							continue;

						//Send is handled by SendThread, and Complete Sign is handled by workerthread.
						pIOCP->SendComplete(pSocketContext, dwBytesTransferred);
	#endif
					}
					__except(CExceptionCodeLog(GetExceptionCode(), L"WorkerThread - IOPOST_SEND"), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
						// This code block is never executed
					}
					break;
				}
			}
		}
		__except(CExceptionCodeLog(GetExceptionCode(), L"WorkerThread"), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
			// This code block is never executed
		}
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] WorkerThread - TID : %d\r\n", ::GetCurrentThreadId());

	return 0;
#endif // _WIN32
}

void CIocpManager::StoreMiniDump()
{
	WCHAR szFileName[256];
	WCHAR szHead[256];

	if (GetModuleFileName((HMODULE)GetModuleHandle(NULL), szFileName, 256) > 0)
	{
		if (wcschr(szFileName, '\\'))
			wcscpy(szHead, wcsrchr(szFileName, '\\') + 1);
		else
			wcscpy(szHead, szFileName);

		if (wcschr(szHead, '.'))
			wcscpy(wcschr(szHead, '.'), L"");
	}	else
		wcscpy(szHead, L"exe");

	_swprintf(szFileName, _T("%s.%d%d%d%d.dmp"), szHead, rand()%10, rand()%10, rand()%10, rand()%10);
	::StoreMiniDump(szFileName, MiniDumpWithFullMemory);
}

#ifdef _USE_ACCEPTEX

bool CIocpManager::bOnReUse( CSocketContext* pSocketContext )
{
	if( !CAcceptorEx::GetInstance().bOnClose( pSocketContext ) )
		return false;

	//g_Log.LogA( "CIocpManager::OnReUse() Socket reused\r\n", pSocketContext->m_Socket );
	return true;
}

#endif

#include <iprtrmib.h>

#define CXIP_A(IP)	((IP&0xFF000000)>>24)
#define CXIP_B(IP)	((IP&0x00FF0000)>>16)
#define CXIP_C(IP)	((IP&0x0000FF00)>>8)
#define CXIP_D(IP)	(IP&0x000000FF)

void CIocpManager::_GetHostIPAddress()
{
#if !defined(_WIN32)
	DWORD dwPrivateIP = 0;
	DWORD dwPublicIP  = 0;
	struct ifaddrs* ifaddr = nullptr;
	if (::getifaddrs(&ifaddr) == 0)
	{
		for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
		{
			if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
			DWORD dwIP = ntohl(reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr)->sin_addr.s_addr);
			if (CXIP_A(dwIP) == 127 || dwIP == 0) continue;
			bool bPrivate = (CXIP_A(dwIP) == 10) ||
			                (CXIP_A(dwIP) == 172 && CXIP_B(dwIP) >= 16 && CXIP_B(dwIP) <= 31) ||
			                (CXIP_A(dwIP) == 192 && CXIP_B(dwIP) == 168);
			if (bPrivate)
			{
				if (!dwPrivateIP || dwPrivateIP > dwIP) dwPrivateIP = dwIP;
			}
			else if (!dwPublicIP)
			{
				dwPublicIP = dwIP;
			}
		}
		::freeifaddrs(ifaddr);
	}
	if (!dwPublicIP) dwPublicIP = dwPrivateIP ? dwPrivateIP : ntohl(inet_addr("127.0.0.1"));
	if (!dwPrivateIP) dwPrivateIP = dwPublicIP;
	in_addr pubAddr{}, privAddr{};
	pubAddr.s_addr  = htonl(dwPublicIP);
	privAddr.s_addr = htonl(dwPrivateIP);
	m_strPublicIP  = inet_ntoa(pubAddr);
	m_strPrivateIP = inet_ntoa(privAddr);
	return;
#else
	DWORD dwPrivateIP		= 0;
	DWORD dwPrivateIPMask	= 0;
	DWORD dwPublicIP		= 0;
	DWORD dwPublicIPMask	= 0;

	HMODULE hIPHLP = LoadLibrary( _T("iphlpapi.dll") );
	if( hIPHLP )
	{
		typedef	BOOL (WINAPI * LPGIPT)(PMIB_IPADDRTABLE pIpAddrTable, PULONG pdwSize, BOOL bOrder);
		LPGIPT fnGetIpAddrTable=(LPGIPT)GetProcAddress(hIPHLP, "GetIpAddrTable");
		if( fnGetIpAddrTable )
		{
			PMIB_IPADDRTABLE pIPAddrTable;
			DWORD dwSize=0;

			pIPAddrTable=(MIB_IPADDRTABLE *)malloc(sizeof(MIB_IPADDRTABLE));
			if(!pIPAddrTable) 
			{
				FreeLibrary(hIPHLP);
				return;
			}

			if( fnGetIpAddrTable(pIPAddrTable, &dwSize, 0)==ERROR_INSUFFICIENT_BUFFER )
			{
				free(pIPAddrTable);
				pIPAddrTable=(MIB_IPADDRTABLE *)malloc(dwSize);
				if(!pIPAddrTable) 
				{
					FreeLibrary(hIPHLP);
					return;
				}
			}

			if( fnGetIpAddrTable(pIPAddrTable, &dwSize, 0) == NO_ERROR )
			{ 
				for( DWORD i=0; i<pIPAddrTable->dwNumEntries ; ++i )
				{
					DWORD	dwIP		= ntohl(pIPAddrTable->table[i].dwAddr);
					BOOL	bPrivate	= false;

					if(CXIP_A(dwIP)==127)
					{
						continue;
					}
					else if(CXIP_A(dwIP)==10)
					{
						bPrivate=true;
					}
					else if(CXIP_A(dwIP)==172)
					{
						if(CXIP_B(dwIP)>=16 && CXIP_B(dwIP)<=31) 
							bPrivate=TRUE;
					}
					else if(CXIP_A(dwIP)==192)
					{
						if(CXIP_B(dwIP)==168)
						{
							bPrivate=TRUE;
						}
					}

					if(bPrivate)
					{
						if( !dwPrivateIP || dwPrivateIP>dwIP )
						{
							dwPrivateIP=dwIP;
							dwPrivateIPMask=ntohl(pIPAddrTable->table[i].dwMask);
						}
					}
					else
					{
						if( !dwPublicIP )
						{
							dwPublicIP=dwIP;
							dwPublicIPMask=ntohl(pIPAddrTable->table[i].dwMask);
						}
					}

					if( dwPrivateIP &&  dwPublicIP) 
						break;
				}
			}
			else
			{
				FreeLibrary(hIPHLP);
				return;
			}

			BOOL bIPAdjust=FALSE;
			// Check Public IP
			if(dwPrivateIP && !dwPublicIP)
			{
				bIPAdjust=TRUE;

				for(DWORD i=0; i<pIPAddrTable->dwNumEntries; ++i)
				{
					DWORD dwIP=ntohl(pIPAddrTable->table[i].dwAddr);
					BOOL bPrivate=FALSE;

					if(CXIP_A(dwIP)==127)
					{
						continue;
					}
					else if(CXIP_A(dwIP)==10)
					{
						bPrivate=TRUE;
					}
					else if(CXIP_A(dwIP)==172)
					{
						if(CXIP_B(dwIP)>=16 && CXIP_B(dwIP)<=31) 
							bPrivate=TRUE;
					}
					else if(CXIP_A(dwIP)==192)
					{
						if(CXIP_B(dwIP)==168)
						{
							bPrivate=TRUE;
						}
					}

					if(bPrivate && dwPrivateIP!=dwIP)
					{
						dwPublicIP=dwIP;
						dwPublicIPMask=ntohl(pIPAddrTable->table[i].dwMask);
						break;
					}
				}
			}
			// Check Not Found Public IP
			if(!dwPublicIP)
			{
				dwPublicIP		= dwPrivateIP;
				dwPublicIPMask	= dwPrivateIPMask;
			}
			else
			{
				if( bIPAdjust && dwPrivateIP>dwPublicIP )
				{
					DWORD dwIP		= dwPrivateIP;
					DWORD dwIPMask	= dwPrivateIPMask;

					dwPrivateIP		= dwPublicIP;
					dwPrivateIPMask	= dwPublicIPMask;
					dwPublicIP		= dwIP;
					dwPublicIPMask	= dwIPMask;
				}
			}

			// Clear
			free(pIPAddrTable);
		}
		else
		{
			FreeLibrary(hIPHLP);
			return;
		}
	}
	else
	{
		return;
	}

	FreeLibrary(hIPHLP);

	// Check IP
	if(!dwPrivateIP && !dwPublicIP)
		return;

	DWORD dwNPublicIP = htonl(dwPublicIP);
	m_strPublicIP = inet_ntoa( *((in_addr*)&dwNPublicIP) );

	DWORD dwNPrivateIP = htonl(dwPrivateIP);
	m_strPrivateIP = inet_ntoa( *((in_addr*)&dwNPrivateIP) );
#endif // _WIN32
}

#endif	// #if defined(_SERVER)