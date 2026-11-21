#pragma once

#include "ConnectionManager.h"

#ifdef _GAMESERVER
class CDNUserSession;
#endif
class CDNDBConnection;
class CDNDBConnectionManager: public CConnectionManager
{
public:
	CDNDBConnectionManager(void);
	void Reconnect(DWORD CurTick);

	CConnection* AddConnection(const char *pIp, const USHORT nPort);

	CDNDBConnection* GetDBConnection( UINT uiRandomSeed, BYTE& cThreadID );
	CDNDBConnection* GetDBConnection( BYTE& cThreadID );	

#ifdef PRE_MOD_PVPRANK
	void UpdatedPvPRankCriteria() { m_bUpdatedPvPRankCriteria = true; }
	bool IsPvPRankCriteria() { return m_bUpdatedPvPRankCriteria; }
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined( PRE_FIX_67546 )
#if defined( _VILLAGESERVER )	
	CDNDBConnection* GetVillageDBConnection( int nChannelID, BYTE& cThreadID );
	void AddVillageDBConnectionInfo( int nSessionID, int* nCount );
#endif
#endif
	void SendResetErrorCheck( UINT uiAccountDBID );

private:
	UINT m_LastUseDBID;
	DWORD m_dwReconnectTick;

#ifdef PRE_MOD_PVPRANK
	DWORD m_dwUpdatePvPRankCriteria;
	bool m_bUpdatedPvPRankCriteria;
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined( PRE_FIX_67546 )
	std::map<int,TChannelDBInfo> m_mapChannelDBInfo;		// Channel ID, connection info
	std::map<int,TDBConnectionInfo> m_mapConnectionDBInfo;	// DB ID, DB info
#endif
};

extern CDNDBConnectionManager* g_pDBConnectionManager;

class CDNDBQueryAuto
{
public:
	CDNDBQueryAuto(CDNDBConnectionManager *pDBConnectionManager, bool bDoReset, bool bDoFlush);
	~CDNDBQueryAuto();
private:
	CDNDBConnectionManager *m_pDBConnectionManager;
	bool m_bEnable;
	bool m_bDoFlush;
	DWORD m_nThreadID;
};
