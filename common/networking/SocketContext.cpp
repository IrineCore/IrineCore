#include "stdafx.h"
#include "SocketContext.h"
#ifdef _USE_ACCEPTEX
#include <mswsock.h>
#include "Log.h"
#include "ListenSocket.h"
#include "AcceptorEx.h"
#endif

#if defined(_SERVER)

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#ifdef _USE_SENDCONTEXTPOOL
TIOContext * CSocketContext::GetSendIO()
{
	ScopeLock<CSyncLock> Lock( m_SendIOLock );

	std::vector <TIOContext*>::iterator ii;
	for (ii = m_SendIOList.begin(); ii != m_SendIOList.end(); ii++)
	{
		if ((*ii)->Len == 0)
			return (*ii);
	}
	return NULL;
}

bool CSocketContext::ReleaseSendIO(TIOContext * pIO, int nSize)
{
	ScopeLock<CSyncLock> Lock( m_SendIOLock );

	std::vector <TIOContext*>::iterator ii;
	for (ii = m_SendIOList.begin(); ii != m_SendIOList.end(); ii++)
	{
		if ((*ii) == pIO )
		{
			if( pIO->Len >= nSize )
			{
				pIO->Len -= nSize;
				if( pIO->Len == 0 )
				{
					pIO->mode = IOPOST_SEND;
					return false;
				}
			}
		}
	}
	return true;
}
#endif

CSocketContext::CSocketContext(void)
{
	Clear();
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	m_lActiveCount = 0;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	m_lDetached = 0;
#endif
}

CSocketContext::~CSocketContext(void)
{
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)m_SendIOList.size(); i++)
		SAFE_DELETE(m_SendIOList[i]);
#endif
}

void CSocketContext::Clear()
{
	m_Socket = INVALID_SOCKET;
//#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
//	m_lActiveCount = 0;
//#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#ifdef _USE_SENDCONTEXTPOOL
	memset(&m_RecvIO, 0, sizeof(TIOContext));

	m_SendIOLock.Lock();
	if (m_SendIOList.empty())
	{
		for (int i = 0; i < SENDIOPOOLSIZE; i++)
		{
			TIOContext * p = new TIOContext;
			memset(p, 0, sizeof(TIOContext));
			p->pSocketContext = this;
			m_SendIOList.push_back(p);
		}
	}
	else
	{
		for (int i = 0; i < SENDIOPOOLSIZE; i++)
		{
			memset(m_SendIOList[i], 0, sizeof(TIOContext));
			m_SendIOList[i]->pSocketContext = this;
		}
	}
	m_SendIOLock.UnLock();
#else
	memset(&m_RecvIO, 0, sizeof(TIOContext));
	memset(&m_SendIO, 0, sizeof(TIOContext));
#endif
	m_dwKeyParam = 0;

	m_RefLock.Lock();
	m_nRefCount = 0;
	m_RefLock.UnLock();

	SetParam(NULL);

#ifdef _USE_ACCEPTEX
	m_dwCloseTime = 0;
	m_RecvIO.pSocketContext	= this;
#ifdef _USE_SENDCONTEXTPOOL
	for (int i = 0; i < (int)m_SendIOList.size(); i++)
		m_SendIOList[i]->pSocketContext = this;
#else
	m_SendIO.pSocketContext	= this;
#endif
#endif
}

void CSocketContext::SetParam(void *pParam)
{
	m_RefLock.Lock();
	m_pParam = pParam;
	m_RefLock.UnLock();
}

void* CSocketContext::GetParam()
{
	void* pParam = NULL;
	m_RefLock.Lock();
	pParam = m_pParam;
	m_RefLock.UnLock();

	return pParam;
}

void CSocketContext::IncRef()
{
	m_RefLock.Lock();
	++m_nRefCount;
	m_RefLock.UnLock();
}

int CSocketContext::AddRef()
{
	int Count = 0;
	m_RefLock.Lock();
	if (m_nRefCount > 0)
		Count = ++m_nRefCount;
	m_RefLock.UnLock();

	return Count;
}

int CSocketContext::DelRef()
{
	int Count = 0;
	m_RefLock.Lock();
	if (m_nRefCount > 0)
		Count = --m_nRefCount;
	m_RefLock.UnLock();

	return Count;
}

#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
bool CSocketContext::Detach()
{
	if( InterlockedCompareExchange(&m_lDetached, 1, 0) == 0 )
		return true;
	else
		return false;
}

#endif

#ifdef _USE_ACCEPTEX

#include <Mswsock.h>

bool CSocketContext::_bInitSocket( const BYTE cIOMode )
{
	if( m_Socket != INVALID_SOCKET )
	{
		closesocket( m_Socket );
		m_Socket = INVALID_SOCKET;
	}
	
	Clear();

	m_dwKeyParam	= CAcceptorEx::GetInstance().GetConnectionKey( m_uiListenID );
	m_Socket		= WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( m_Socket == INVALID_SOCKET )
		return false;

	m_RecvIO.mode = cIOMode;

	CListenSocket::SetDefaultSocketOption( m_Socket );

	return true;
}

void CSocketContext::OnAccept( sockaddr_in* pLocalAddr, sockaddr_in* pPeerAddr )
{
	INT32 iAddrLen = sizeof(sockaddr_in);
	char szLocalIP[MAX_PATH];
	char szPeerIP[MAX_PATH];
	DWORD dwStringLen = MAX_PATH;

	WSAAddressToStringA( (LPSOCKADDR)pLocalAddr, iAddrLen, 0, szLocalIP, &dwStringLen );
	WSAAddressToStringA( (LPSOCKADDR)pPeerAddr, iAddrLen, 0, szPeerIP, &dwStringLen );

	//g_Log.LogA( "[Accept] ListenID=%d\r\n", m_uiListenID );
	//g_Log.LogA( "LocalAddr IP=%s\r\n", szLocalIP );
	//g_Log.LogA( "PeerAddr IP=%s\r\n", szPeerIP );
}

void CSocketContext::OnDisconnect()
{
	if( m_Socket != INVALID_SOCKET )
		closesocket( m_Socket );

	Clear();
	m_dwCloseTime = timeGetTime();
}

bool CSocketContext::bInitAcceptContext()
{
	// P.S.> Synchronized by CAcceptorEx::m_Lock instead of its own LOCK because this method itself performs the Accept operation ???

	if( !_bInitSocket( IOPOST_ACCEPT ) )
		return false;

	SOCKET ListenSocket = CAcceptorEx::GetInstance().GetSocket( m_uiListenID );
	if( ListenSocket == INVALID_SOCKET )
		return false;

	DWORD dwBytesReceived = 0;

	if( !AcceptEx( ListenSocket, m_Socket, m_RecvIO.buffer, 0, sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16, &dwBytesReceived, &m_RecvIO ) )
	{
		INT32 iError = GetLastError();
		if( iError != ERROR_IO_PENDING )
		{
			g_Log.Log( LogType::_ERROR, "AcceptEx() failed Error=%d\r\n", iError );
			return false;
		}
	}

	return true;
}

#endif

void CSocketContext::SetDetachReason(const WCHAR * pReason)
{
	if (pReason == NULL) return;
	ScopeLock <CSyncLock> sync(m_DetachReasonLock);
	m_wstrDestachReason = pReason;
}

void CSocketContext::GetDetachReason(std::wstring &wstrReason)
{
	ScopeLock <CSyncLock> sync(m_DetachReasonLock);
	wstrReason = m_wstrDestachReason;
}

#endif	// #if defined(_SERVER)