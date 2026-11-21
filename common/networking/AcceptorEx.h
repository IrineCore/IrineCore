
#ifdef _USE_ACCEPTEX

#pragma once

#include "Singleton.h"

class CIocpManager;
class CListenSocket;
class CSocketContext;

class CAcceptorEx : public CSingleton< CAcceptorEx >
{
public:

	CAcceptorEx( CIocpManager* pIOCP );
	~CAcceptorEx();

	static CAcceptorEx* CreateInstance( CIocpManager* pIOCP )
	{
		return new CAcceptorEx( pIOCP );
	}

	// Create socket pool
	SOCKET	GetSocket( const UINT uiListenID );
	int		GetConnectionKey( const UINT uiListenID );
	bool	bCreateListenSocket( const USHORT unPort, const UINT uiPoolSize, const int iKey );
	bool	bOnAccept( const UINT uiListenID );
	bool	bOnClose( CSocketContext* pSocketContext );

private:

	bool	_bCreateSocket( const UINT uiListenID, bool bIncPoolSize = true);
	bool	_bCreateSocketPool( const UINT uiListenID, const USHORT unPort, const UINT uiPoolSize );

	CIocpManager*				m_pIOCP;

	std::vector<CListenSocket*>	m_vListenSocket;
	std::vector<UINT>			m_vSocketPoolSize;
	std::vector<UINT>			m_vUsedSocketSize;
	std::vector<int>			m_vConnectionKey;

	CSyncLock					m_Lock;
};

#endif // #ifdef _USE_ACCEPTEX
