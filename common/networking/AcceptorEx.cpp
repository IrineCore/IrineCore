
#include "stdafx.h"
#include "AcceptorEx.h"
#include "ListenSocket.h"
#include "IocpManager.h"
#include "SocketContext.h"
#include "SocketContextMgr.h"
#include "Log.h"

#ifdef _USE_ACCEPTEX

CAcceptorEx::CAcceptorEx( CIocpManager* pIOCP )
: m_pIOCP( pIOCP )
{
}

CAcceptorEx::~CAcceptorEx()
{
	for( UINT i=0 ; i<m_vListenSocket.size() ; ++i )
		delete m_vListenSocket[i];
	m_vListenSocket.clear();
}

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
	ScopeLock<CSyncLock> Lock( m_Lock );

	if( !pSocketContext->bInitAcceptContext() )
		return false;

	UINT uiListenID = pSocketContext->uiGetListenID();

	if( m_vUsedSocketSize[uiListenID] )
		--m_vUsedSocketSize[uiListenID];
	else
		return false;

	return true;
}

#endif // #ifdef _USE_ACCEPTEX
