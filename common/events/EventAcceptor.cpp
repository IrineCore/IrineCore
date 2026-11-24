#include "stdafx.h"
#include "EventAcceptor.h"
#include "IocpManager.h"
#include "Log.h"
#include "ListenSocket.h"
#if !defined(_WIN32)
#include <poll.h>
#include <sys/eventfd.h>
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#if defined(_SERVER) && !defined(_USE_ACCEPTEX)

EventAcceptor::EventAcceptor(CIocpManager* pIOCP, const char* szThreadNick /* = "EventAcceptor" */ )
: Thread(szThreadNick), m_pIOCP(pIOCP)
#if !defined(_WIN32)
, m_wakeFd(-1)
#endif
{
	m_hAddListenPort = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEndThread = CreateEvent(NULL, FALSE, FALSE, NULL);
#if !defined(_WIN32)
	m_wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
#endif
}

EventAcceptor::~EventAcceptor()
{
	if( m_hAddListenPort != INVALID_HANDLE_VALUE )
		CloseHandle(m_hAddListenPort);
	if( m_hEndThread != INVALID_HANDLE_VALUE )
		CloseHandle(m_hEndThread);
#if !defined(_WIN32)
	if (m_wakeFd >= 0) { ::close(m_wakeFd); m_wakeFd = -1; }
#endif

	m_Lock.Lock();
	std::list<CListenSocket*>::iterator iter;
	for (iter = m_listListenSocket.begin(); iter != m_listListenSocket.end(); iter++)	
		delete (*iter);
	
	m_listListenSocket.clear();
	m_Lock.UnLock();
}

int EventAcceptor::Open( const int nKey, u_short nPortNo, int nBackLogCount /* = SOMAXCONN */)
{
	if (!m_pIOCP)
		return -1;

	if( m_hThreadHandle == INVALID_HANDLE_VALUE )
		return -1;

	CListenSocket* pListenSocket = new CListenSocket();
	if( pListenSocket == NULL )
	{
		g_Log.Log( LogType::_FILELOG, "Failed to allocate CListenSocket\r\n" );
		return -1;
	}

	// Initialize ListenSocket
	if( !pListenSocket->bInitialize( nPortNo, nKey ) )
	{
		delete pListenSocket;
		return -1;
	}

	// Configure ListenSocket	
	m_Lock.Lock();
	m_listListenSocket.push_back( pListenSocket );
	m_Lock.UnLock();

	SetEvent(m_hAddListenPort);
#if !defined(_WIN32)
	if (m_wakeFd >= 0)
	{
		uint64_t one = 1;
		(void)::write(m_wakeFd, &one, sizeof(one));
	}
#endif
	
	return 0;
}

void EventAcceptor::Close()
{	
	if( m_hThreadHandle != INVALID_HANDLE_VALUE)
	{
#if !defined(_WIN32)
		m_bRunning = false;
		if (m_wakeFd >= 0)
		{
			uint64_t one = 1;
			(void)::write(m_wakeFd, &one, sizeof(one));
		}
#endif
		SetEvent(m_hEndThread);
		if (WaitForSingleObject(m_hThreadHandle, INFINITE) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hThreadHandle);
			CloseHandle(m_hAddListenPort);
			CloseHandle(m_hEndThread);
			m_hThreadHandle = INVALID_HANDLE_VALUE;			
			m_hAddListenPort = INVALID_HANDLE_VALUE;			
			m_hEndThread = INVALID_HANDLE_VALUE;
#if !defined(_WIN32)
			if (m_wakeFd >= 0) { ::close(m_wakeFd); m_wakeFd = -1; }
#endif
		}		
	}
}

#if !defined(_WIN32)
void EventAcceptor::InitListenEvent(HANDLE*, int&) {}
void EventAcceptor::ProcessEvent(HANDLE*, int) {}

void EventAcceptor::Run()
{
	while (m_bRunning)
	{
		std::vector<struct pollfd> pfds;
		std::vector<CListenSocket*> sockets;
		if (m_wakeFd >= 0)
		{
			struct pollfd wfd{};
			wfd.fd = m_wakeFd;
			wfd.events = POLLIN;
			pfds.push_back(wfd);
		}
		m_Lock.Lock();
		for (std::list<CListenSocket*>::iterator iter = m_listListenSocket.begin(); iter != m_listListenSocket.end(); ++iter)
		{
			if (*iter && (*iter)->GetSocket() != INVALID_SOCKET)
			{
				struct pollfd pfd{};
				pfd.fd = (*iter)->GetSocket();
				pfd.events = POLLIN;
				pfds.push_back(pfd);
				sockets.push_back(*iter);
			}
		}
		m_Lock.UnLock();

		if (pfds.empty())
		{
			Sleep(10);
			continue;
		}

		int nRet = ::poll(pfds.data(), static_cast<nfds_t>(pfds.size()), 200);
		if (nRet <= 0)
			continue;

		size_t sockStart = 0;
		if (m_wakeFd >= 0)
		{
			sockStart = 1;
			if (pfds[0].revents & POLLIN)
			{
				uint64_t val = 0;
				(void)::read(m_wakeFd, &val, sizeof(val));
				if (!m_bRunning)
					break;
			}
		}

		for (size_t i = 0; i < sockets.size(); ++i)
		{
			if (!(pfds[sockStart + i].revents & POLLIN))
				continue;

			CListenSocket* pListenSocket = sockets[i];
			for (;;)
			{
				SOCKADDR_IN Addr{};
				socklen_t nLen = sizeof(SOCKADDR_IN);
				SOCKET Socket = ::accept(pListenSocket->GetSocket(), reinterpret_cast<sockaddr*>(&Addr), &nLen);
				if (Socket == INVALID_SOCKET)
				{
					if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)
						g_Log.Log(LogType::_ACCEPTORERROR, L"Accept Socket Invalid [RetCode:%d]\n", errno);
					break;
				}

				if (!m_pIOCP->SetSocketOption(Socket))
				{
					closesocket(Socket);
					continue;
				}

				CSocketContext* pSocketContext = m_pIOCP->GetSocketContext(pListenSocket->GetConnectionKey(), Socket, &Addr);
				if (pSocketContext)
				{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
					CScopeInterlocked Scope(&pSocketContext->m_lActiveCount);
					if (Scope.bIsDelete())
					{
						closesocket(Socket);
						continue;
					}
#endif
					m_pIOCP->OnAccept(pSocketContext, inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
					if (pSocketContext->GetParam())
					{
						pSocketContext->IncRef();
						int nAttach = m_pIOCP->AttachSocket(pSocketContext);
						if (nAttach == 0)
							m_pIOCP->OnConnected(pSocketContext);
						else
						{
							closesocket(Socket);
							m_pIOCP->OnDisconnected(pSocketContext);
							pSocketContext->Clear();
							m_pIOCP->PutSocketContext(pSocketContext);
						}
					}
					else
					{
						closesocket(Socket);
						m_pIOCP->PutSocketContext(pSocketContext);
					}
				}
				else
				{
					closesocket(Socket);
				}
			}
		}
	}
	g_Log.Log(LogType::_NORMAL, L"### EventAcceptor Thread Finalize\r\n");
}
#else

void EventAcceptor::InitListenEvent(HANDLE* hEvents, int& nEventCount )
{
	m_Lock.Lock();
	nEventCount = 2;
	std::list<CListenSocket*>::iterator iter;
	for (iter = m_listListenSocket.begin(); iter != m_listListenSocket.end(); ++iter)
	{
		WSAResetEvent(hEvents[nEventCount]);
		WSAEventSelect((*iter)->GetSocket(), hEvents[nEventCount], FD_ACCEPT);

		++nEventCount;
	}
	m_Lock.UnLock();
}

void EventAcceptor::ProcessEvent(HANDLE* hEvents, int nEventCount)
{
	WSANETWORKEVENTS	EventRet;

	SOCKADDR_IN Addr;
	int nLen = sizeof(SOCKADDR_IN);
	CSocketContext *pSocketContext;
	CListenSocket* pListenSocket;
	int nRet = 0;

	m_Lock.Lock();
	std::list<CListenSocket*>::iterator iter = m_listListenSocket.begin();

	for(int i=2; i<nEventCount; ++i, ++iter)
	{
		pListenSocket = *iter;

		SOCKET Socket;

		WSAEnumNetworkEvents(pListenSocket->GetSocket(), hEvents[i], &EventRet);
		if( !EventRet.lNetworkEvents)
			continue;

		Socket = accept( pListenSocket->GetSocket(), (sockaddr*)&Addr, &nLen );

		if (Socket == INVALID_SOCKET) 
		{
			int nWSAret = WSAGetLastError();
			g_Log.Log(LogType::_ACCEPTORERROR, L"Accept Socket Invalid [RetCode:%d]\n", nWSAret);
			continue;
		}

		if (!m_pIOCP->SetSocketOption(Socket))
		{
			closesocket(Socket);
			continue;
		}

		pSocketContext = m_pIOCP->GetSocketContext(pListenSocket->GetConnectionKey(), Socket, &Addr);
		if (pSocketContext)
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
			if( Scope.bIsDelete() )
			{
				closesocket(Socket);
				continue;
			}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

			m_pIOCP->OnAccept(pSocketContext, inet_ntoa(Addr.sin_addr), ntohs(Addr.sin_port));
			if (pSocketContext->GetParam())
			{
				pSocketContext->IncRef();

				nRet = m_pIOCP->AttachSocket(pSocketContext);
				if (nRet == 0)
					m_pIOCP->OnConnected(pSocketContext);				
				else 
				{
					closesocket(Socket);
					m_pIOCP->OnDisconnected(pSocketContext);
					pSocketContext->Clear();
					m_pIOCP->PutSocketContext(pSocketContext);
				}
			}
			else
			{
				closesocket(Socket);
				m_pIOCP->PutSocketContext(pSocketContext);
			}
		}
		else
			closesocket(Socket);
	}
	m_Lock.UnLock();
}

void EventAcceptor::Run()
{
	HANDLE	hEvents[MAXIMUM_WAIT_OBJECTS] = { m_hAddListenPort, m_hEndThread, INVALID_HANDLE_VALUE, };
	int		nEventCount = 2;
	int		nRet;	

	// Add ListenSocket; up to 62 Listen sockets can be added excluding the Thread termination event.
	for ( int i = 2; i < MAXIMUM_WAIT_OBJECTS; i++ )
	{
		hEvents[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
		nEventCount++;
	}

	while ( true )
	{
		nRet = WSAWaitForMultipleEvents(nEventCount, hEvents, FALSE, INFINITE, FALSE);

		if (nRet == WAIT_OBJECT_0) // The first one is the one being added...
		{
			InitListenEvent(hEvents, nEventCount);
			continue;
		}
		else if (nRet == WAIT_OBJECT_0 + 1) // Exit on 2nd..
			break;

		ProcessEvent(hEvents, nEventCount);
	}

	for ( int i =2; i < MAXIMUM_WAIT_OBJECTS; i++ )
		CloseHandle( hEvents[i] );

	g_Log.Log( LogType::_NORMAL, L"### EventAcceptor Thread Finalize\r\n" );

}
#endif // _WIN32

#endif	// #if defined(_SERVER) && !defined(_USE_ACCEPTEX)
