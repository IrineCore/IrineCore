#include "stdafx.h"
#include "DNAuthManager.h"
#include "DNSQLMembershipManager.h"
#include "DNExtManager.h"
#include "DNLogConnection.h"
#include "DNMasterConnectionManager.h"
#include "Log.h"
#include "Util.h"
#include "DNIocpManager.h"
#include "DNUserConnection.h"

extern TLoginConfig g_Config;
CDNAuthManager* g_pAuthManager = NULL;

CDNAuthManager::CDNAuthManager()
{
}

int CDNAuthManager::QueryStoreAuth(char cLastServerType, CDNUserConnection *pUserCon)
{
	int iResult = ERROR_NONE;	

	g_Log.Log((0 > iResult)?(LogType::_ERROR):(LogType::_NORMAL), pUserCon, L"[Auth] StoreAuth (REQ) - Result:%d, nAccountDBID:%d (%s/%s), CertifyingKey:%I64d\r\n", 
		iResult, pUserCon->GetAccountDBID(), pUserCon->GetAccountName(), (pUserCon->GetSelectCharacterName())?(pUserCon->GetSelectCharacterName()):(L""), pUserCon->GetCertifyingKey());

	iResult = ERROR_DB;

	iResult = g_pSQLMembershipManager->QueryStoreAuth(pUserCon->GetAccountDBID(), pUserCon->GetCertifyingKey(), GetServerID(), pUserCon->GetWorldSetID(), pUserCon->GetSessionID(), pUserCon->GetSelectCharacterDBID(),
		pUserCon->GetAccountName(), (pUserCon->GetSelectCharacterName())?(pUserCon->GetSelectCharacterName()):(L""), pUserCon->GetAdult(), pUserCon->GetAge(), pUserCon->GetDailyCreateCount(), pUserCon->GetPrmInt1(), cLastServerType);	

	g_Log.Log((ERROR_NONE != iResult)?(LogType::_ERROR):(LogType::_NORMAL), pUserCon, L"[Auth] StoreAuth (ACK) - Result:%d, nAccountDBID:%d (%s/%s), CertifyingKey:%I64d\r\n",
		iResult, pUserCon->GetAccountDBID(), pUserCon->GetAccountName(), pUserCon->GetSelectCharacterName(), pUserCon->GetCertifyingKey());

	return (iResult);
}

int CDNAuthManager::QueryResetAuth(char cWorldID, UINT nAccountDBID, UINT nSessionID)
{
	int iResult = ERROR_DB;

	iResult = g_pSQLMembershipManager->QueryResetAuth(nAccountDBID, nSessionID);

	g_Log.Log((0 > iResult)?(LogType::_ERROR):(LogType::_NORMAL), cWorldID, nAccountDBID, 0, nSessionID, L"[Auth] ResetAuth (REQ) - Result:%d, nAccountDBID:%d, TID:%d\r\n", iResult, nAccountDBID, ::GetCurrentThreadId());

	return(iResult);
}

int CDNAuthManager::QuerySetWorldIDAuth(char cWorldID, UINT nAccountDBID)
{
	int iResult = ERROR_DB;

	iResult = g_pSQLMembershipManager->QuerySetWorldID(cWorldID, nAccountDBID);

	return(iResult);
}

int CDNAuthManager::QueryResetAuthServer()
{
	int iResult = ERROR_DB;

	int nServerID = GetServerID();
	if (!nServerID) {
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	iResult = g_pSQLMembershipManager->QueryResetAuthServer(GetServerID());
	if (ERROR_NONE == iResult) {
		SetResetAuthServer(true);

		// Start server
		if (g_pIocpManager->AddAcceptConnection(CONNECTIONKEY_USER, g_Config.nClientAcceptPort, 1000) < 0){
			g_Log.Log(LogType::_FILELOG, L"ClientAcceptPort Fail(%d)\r\n", g_Config.nClientAcceptPort);
		}
		else {
			g_Log.Log(LogType::_FILELOG, L"ClientAcceptPort (%d)\r\n", g_Config.nClientAcceptPort);
		}
	}
	DN_ASSERT(ERROR_NONE == iResult,	"Invalidl!");

	return(iResult);
}

int CDNAuthManager::QueryResetAuthServerClose()
{
	int iResult = ERROR_DB;

	int nServerID = GetServerID();
	if (!nServerID) {
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	iResult = g_pSQLMembershipManager->QueryResetAuthServer(GetServerID());
	g_Log.Log(LogType::_NORMAL, L"QueryResetAuthServerClose %d \r\n", GetServerID());

	return iResult;
}
