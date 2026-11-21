#include "stdafx.h"
#include "EventAcceptor.h"
#include "IocpManager.h"
#include "Log.h"
#include "ListenSocket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#if defined(_SERVER) && !defined(_USE_ACCEPTEX)

EventAcceptor::EventAcceptor(CIocpManager* pIOCP, const char* szThreadNick /* = "EventAcceptor" */ )
: Thread(szThreadNick), m_pIOCP(pIOCP)
{
	m_hAddListenPort = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hEndThread = CreateEvent(NULL, FALSE, FALSE, NULL);
}

EventAcceptor::~EventAcceptor()
{
	if( m_hAddListenPort != INVALID_HANDLE_VALUE )
		CloseHandle(m_hAddListenPort);
	if( m_hEndThread != INVALID_HANDLE_VALUE )
		CloseHandle(m_hEndThread);

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
	
	return 0;
}

void EventAcceptor::Close()
{	
	if( m_hThreadHandle != INVALID_HANDLE_VALUE)
	{
		SetEvent(m_hEndThread);
		if (WaitForSingleObject(m_hThreadHandle, INFINITE) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hThreadHandle);
			CloseHandle(m_hAddListenPort);
			CloseHandle(m_hEndThread);
			m_hThreadHandle = INVALID_HANDLE_VALUE;			
			m_hAddListenPort = INVALID_HANDLE_VALUE;			
			m_hEndThread = INVALID_HANDLE_VALUE;
		}		
	}
}

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

#endif	// #if defined(_SERVER) && !defined(_USE_ACCEPTEX)
