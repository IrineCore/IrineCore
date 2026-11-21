#pragma once

#include "ConnectionManager.h"

class CDNMasterConnection;
class CDNMasterConnectionManager: public CConnectionManager
{
public:
	CDNMasterConnectionManager(void);
	virtual ~CDNMasterConnectionManager(void);
	
	CConnection* AddConnection(const char *pIp, const USHORT nPort);

	void GetChannelListByMapIdx(int nWorldSetID, TDBListCharData* pSelectCharData, sChannelInfo *ChannelArray, BYTE &cCount, TMapChannel *ChannelList);
	UINT GetWaitUserAmount();

#if defined(PRE_MOD_SELECT_CHAR)
	bool SendDetachUserConnectionByWorldSetID(char cWorldSetID, UINT nAccountDBID, bool bIsReconnectLogin, bool bIsDuplicate=false, UINT nSessionID=0);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	bool SendDetachUserConnectionByWorldSetID(char cWorldSetID, UINT nAccountDBID, BYTE cAccountLevel, bool bIsReconnectLogin, const BYTE * pMachineID = NULL, DWORD dwGRC = 0, bool bIsDuplicate=false, UINT nSessionID=0, const char * pszIP = NULL);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	void SendConCountInfoAll(DWORD CurTick, int nServerID, int nCurConCount, int nMaxConCount);
	void SendDetachUserOtherLogin(UINT nAccountDBID, int nLoginServerID);
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	void SendDetachUserbyIP(const char * pszIP);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
};

extern CDNMasterConnectionManager* g_pMasterConnectionManager;
