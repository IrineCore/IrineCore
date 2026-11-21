#pragma once

#include "AuthManager.h"

class CDNUserSession;

class CDNAuthManager : public CAuthManager
{
public:
	CDNAuthManager();

	int QueryStoreAuth(char cLastServerType, CDNUserSession *pUserSesion);
	int QueryCheckAuth(INT64 biCertifyingKey, CDNUserSession *pUserSesion);
	int QueryCheckAuth(CDNDBConnection * pDBCon, INT64 biCertifyingKey, BYTE cThreadID, int nAccountDBID, int nSessionID);
	int QueryResetAuth(char cWorldSetID, UINT nAccountDBID, UINT nSessionID);
	int QueryResetAuthServerClose();

private:
	void FlushResetAuthLIst();
	int QueryResetAuthServer();
	int QueryWorldUserCount() { return 0; }
	
private:
	
};

extern CDNAuthManager* g_pAuthManager;

