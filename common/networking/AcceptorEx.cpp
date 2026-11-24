
#include "stdafx.h"
#include "AcceptorEx.h"
#include "ListenSocket.h"
#include "IocpManager.h"
#include "SocketContext.h"
#include "SocketContextMgr.h"
#include "Log.h"
#if !defined(_WIN32)
#include <poll.h>
#include <sys/eventfd.h>
#endif

#ifdef _USE_ACCEPTEX

CAcceptorEx::CAcceptorEx( CIocpManager* pIOCP )
: m_pIOCP( pIOCP )
#if !defined(_WIN32)
, m_acceptThread{}, m_bAcceptThreadStarted(false), m_bAcceptRunning(false), m_wakeFd(-1)
#endif
{
#if !defined(_WIN32)
	m_wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	m_bAcceptRunning = true;
	if (::pthread_create(&m_acceptThread, nullptr, _LinuxAcceptThread, this) == 0)
		m_bAcceptThreadStarted = true;
#endif
}

CAcceptorEx::~CAcceptorEx()
{
#if !defined(_WIN32)
	m_bAcceptRunning = false;
	if (m_wakeFd >= 0)
	{
		uint64_t one = 1;
		(void)::write(m_wakeFd, &one, sizeof(one));
	}
	if (m_bAcceptThreadStarted)
	{
		::pthread_join(m_acceptThread, nullptr);
		m_bAcceptThreadStarted = false;
	}
	if (m_wakeFd >= 0) { ::close(m_wakeFd); m_wakeFd = -1; }
#endif
	for( UINT i=0 ; i<m_vListenSocket.size() ; ++i )
		delete m_vListenSocket[i];
	m_vListenSocket.clear();
}

#if !defined(_WIN32)
void* CAcceptorEx::_LinuxAcceptThread(void* pParam)
{
	static_cast<CAcceptorEx*>(pParam)->_LinuxAcceptLoop();
	return nullptr;
}

void CAcceptorEx::_LinuxAcceptLoop()
{
	while (m_bAcceptRunning)
	{
		std::vector<struct pollfd> pfds;
		std::vector<UINT> listenIds;
		if (m_wakeFd >= 0)
		{
			struct pollfd wfd{};
			wfd.fd = m_wakeFd;
			wfd.events = POLLIN;
			pfds.push_back(wfd);
		}
		{
			ScopeLock<CSyncLock> Lock(m_Lock);
			for (UINT i = 0; i < m_vListenSocket.size(); ++i)
			{
				if (m_vListenSocket[i] && m_vListenSocket[i]->GetSocket() != INVALID_SOCKET)
				{
					struct pollfd pfd{};
					pfd.fd = m_vListenSocket[i]->GetSocket();
					pfd.events = POLLIN;
					pfds.push_back(pfd);
					listenIds.push_back(i);
				}
			}
		}
		if (pfds.empty()) { Sleep(10); continue; }
		int nRet = ::poll(pfds.data(), static_cast<nfds_t>(pfds.size()), 200);
		if (nRet <= 0) continue;
		size_t startIdx = (m_wakeFd >= 0) ? 1 : 0;
		if (m_wakeFd >= 0 && (pfds[0].revents & POLLIN))
		{
			uint64_t v = 0;
			(void)::read(m_wakeFd, &v, sizeof(v));
			if (!m_bAcceptRunning) break;
		}
		for (size_t i = 0; i < listenIds.size(); ++i)
		{
			if (!(pfds[startIdx + i].revents & POLLIN)) continue;
			UINT listenId = listenIds[i];
			SOCKET listenSock = GetSocket(listenId);
			int connKey = GetConnectionKey(listenId);
			for (;;)
			{
				sockaddr_in peerAddr{};
				socklen_t peerLen = sizeof(peerAddr);
				SOCKET clientSock = ::accept(listenSock, reinterpret_cast<sockaddr*>(&peerAddr), &peerLen);
				if (clientSock == INVALID_SOCKET) break;
				CListenSocket::SetDefaultSocketOption(clientSock);
				CSocketContext* pCtx = CSocketContextMgr::GetInstance().pGetSocketContext();
				if (!pCtx) { closesocket(clientSock); continue; }
				pCtx->m_Socket     = clientSock;
				pCtx->m_dwKeyParam = connKey;
				pCtx->SetListenID(listenId);
				m_pIOCP->OnAcceptLinux(pCtx, &peerAddr);
			}
		}
	}
}
#endif

bool CAcceptorEx::_bCreateSocket( const UINT uiListenID, bool bIncPoolSize )
{
	CSocketContext* pSocketContext = CSocketContextMgr::GetInstance().pGetSocketContext();
	if( pSocketContext == NULL )
		return false;

	pSocketContext->SetListenID( uiListenID );
	if( !pSocketContext->bInitAcceptContext() )
		return false;

	if (bIncPoolSize) {
		ScopeLock<CSyncLock> Lock( m_Lock );

		++m_vSocketPoolSize[uiListenID];
	}

	return true;
}

bool CAcceptorEx::_bCreateSocketPool( const UINT uiListenID, const USHORT unPort, const UINT uiPoolSize )
{
#if !defined(_WIN32)
	if (m_wakeFd >= 0)
	{
		uint64_t one = 1;
		(void)::write(m_wakeFd, &one, sizeof(one));
	}
	g_Log.Log( LogType::_FILEDBLOG, "[%d] Listen Socket created PORT=%d SocketPoolSize=%d\r\n", uiListenID, unPort, uiPoolSize );
	return true;
#else
	// Create SocketPool
	for( UINT i=0 ; i<uiPoolSize ; ++i )
	{
		if( !_bCreateSocket( uiListenID ) )
			return false;
	}

	// Bind ListenSocket to IOCP
	if( CreateIoCompletionPort( reinterpret_cast<HANDLE>(m_vListenSocket[uiListenID]->GetSocket()), m_pIOCP->GetIocpHandle(), 0, 0 ) == 0 )
		return false;

	g_Log.Log( LogType::_FILEDBLOG, "[%d] Listen Socket created PORT=%d SocketPoolSize=%d\r\n", uiListenID, unPort, uiPoolSize );

	return true;
#endif
}

SOCKET CAcceptorEx::GetSocket( const UINT uiListenID )
{
	if( m_vListenSocket.size() <= uiListenID )
		return INVALID_SOCKET;

	return m_vListenSocket[uiListenID]->GetSocket();
}

int	CAcceptorEx::GetConnectionKey( const UINT uiListenID )
{
	if( m_vConnectionKey.size() > uiListenID )
		return m_vConnectionKey[uiListenID];

	return CONNECTIONKEY_DEFAULT;
}

bool CAcceptorEx::bCreateListenSocket( const USHORT unPort, const UINT uiPoolSize, const int iKey )
{
	CListenSocket* pListenSocket = new (std::nothrow) CListenSocket();
	if( pListenSocket == NULL )
	{
		g_Log.Log( LogType::_FILELOG, "Failed to allocate CListenSocket\r\n" );
		return false;
	}
	
	// Initialize ListenSocket
	if( !pListenSocket->bInitialize( unPort ) )
	{
		delete pListenSocket;
		return false;
	}

	// Configure ListenSocket
	UINT uiListenID = static_cast<UINT>(m_vListenSocket.size());
	m_vListenSocket.push_back( pListenSocket );

	// Initialize SocketPool info
	m_vSocketPoolSize.push_back( 0 );
	m_vUsedSocketSize.push_back( 0 );
	m_vConnectionKey.push_back( iKey );

	return _bCreateSocketPool( uiListenID, unPort, uiPoolSize );
}

bool CAcceptorEx::bOnAccept( const UINT uiListenID )
{
	bool bDoCreate = false;
	{
		ScopeLock<CSyncLock> Lock( m_Lock );

		++m_vUsedSocketSize[uiListenID];

		// If used Sockets exceed 50% of SocketPoolSize, change to unconditionally create.
		if( m_vUsedSocketSize[uiListenID] >= static_cast<UINT>(m_vSocketPoolSize[uiListenID]*0.5f) ) {
			++m_vSocketPoolSize[uiListenID];

			bDoCreate = true;
		}
	}

	if (bDoCreate) {
		if( !_bCreateSocket( uiListenID, false ) ) {
			ScopeLock<CSyncLock> Lock( m_Lock );

			--m_vSocketPoolSize[uiListenID];

			return false;
		}

		g_Log.Log( LogType::_FILEDBLOG, "[ListenID=%d] SocketPool exhausted, allocating extra Socket\r\n", uiListenID );
	}

	return true;
}

bool CAcceptorEx::bOnClose( CSocketContext* pSocketContext )
{
#if !defined(_WIN32)
	if (pSocketContext) pSocketContext->Clear();
	return true;
#else
	ScopeLock<CSyncLock> Lock( m_Lock );

	if( !pSocketContext->bInitAcceptContext() )
		return false;

	UINT uiListenID = pSocketContext->uiGetListenID();

	if( m_vUsedSocketSize[uiListenID] )
		--m_vUsedSocketSize[uiListenID];
	else
		return false;

	return true;
#endif
}

#endif // #ifdef _USE_ACCEPTEX
