#include "stdafx.h"
#include "DNAuthTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

extern TDBConfig g_Config;

CDNAuthTask::CDNAuthTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNAuthTask::~CDNAuthTask(void)
{
}

void CDNAuthTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;

	switch (nSubCmd)
	{
	case QUERY_STOREAUTH:				// Set auth info (right before server transfer) (used by GA, VI)
		{
			TQStoreAuth* pAuth = reinterpret_cast<TQStoreAuth*>(pData);
			TAStoreAuth Auth;
			memset(&Auth, 0, sizeof(Auth));

			Auth.nAccountDBID = pAuth->nAccountDBID;
			Auth.nSessionID = pAuth->nSessionID;
			Auth.nRetCode = ERROR_DB;		// SQL error occurred

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Auth.nRetCode = pMembershipDB->QueryStoreAuth(pAuth->nAccountDBID, pAuth->biCertifyingKey, pAuth->nCurServerID, pAuth->cWorldSetID, pAuth->nSessionID, pAuth->biCharacterDBID, pAuth->wszAccountName, pAuth->wszCharacterName, pAuth->bIsAdult, pAuth->nAge, pAuth->nDailyCreateCount, pAuth->nPrmInt1, pAuth->cLastServerType);
				if (ERROR_NONE != Auth.nRetCode) {
					g_Log.Log(LogType::_ERROR, pAuth->cWorldSetID, pAuth->nAccountDBID, pAuth->biCharacterDBID, pAuth->nSessionID, L"[ADBID:%d] [QUERY_STOREAUTH] Query Error Result:%d\r\n", pAuth->nAccountDBID, Auth.nRetCode);
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pAuth->cWorldSetID, pAuth->nAccountDBID, pAuth->biCharacterDBID, pAuth->nSessionID, L"[CDBID:%lld] [QUERY_STOREAUTH] Query not found\r\n", pAuth->biCharacterDBID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Auth, sizeof(Auth));
		}
		break;

	case QUERY_CHECKAUTH:				// Check auth info (right after server transfer) (used by GA, VI)
		{
			TQCheckAuth* pAuth = reinterpret_cast<TQCheckAuth*>(pData);
			TACheckAuth Auth;
			memset(&Auth, 0, sizeof(Auth));

			Auth.nAccountDBID = pAuth->nAccountDBID;
			Auth.biCertifyingKey = pAuth->biCertifyingKey;
			Auth.nRetCode = ERROR_DB;		// SQL error occurred

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Auth.nRetCode = pMembershipDB->QueryCheckAuth(pAuth->nAccountDBID, pAuth->biCertifyingKey, pAuth->nCurServerID, Auth.cWorldSetID, Auth.nSessionID, Auth.biCharacterDBID, static_cast<LPWSTR*>(static_cast<LPVOID>(&Auth.wszAccountName)), static_cast<LPWSTR*>(static_cast<LPVOID>(&Auth.wszCharacterName)), Auth.cAccountLevel, Auth.bIsAdult, Auth.nAge, Auth.nDailyCreateCount, Auth.nPrmInt1, Auth.cLastServerType);
				if (ERROR_NONE != Auth.nRetCode) {
					g_Log.Log(LogType::_ERROR, 0, pAuth->nAccountDBID, 0, 0, L"[ADBID:%d][QUERY_CHECKAUTH] Query Error Result:%d\r\n", pAuth->nAccountDBID, Auth.nRetCode);
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, 0, pAuth->nAccountDBID, 0, 0, L"[QUERY_CHECKAUTH] Query not found\r\n");
			}

			if (Auth.nSessionID == 0)
				Auth.nSessionID = pAuth->nSessionID;

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Auth, sizeof(Auth));
			if( Auth.nRetCode == ERROR_NONE)
			{
				// Delete if it happens to exist here.
				g_pSPErrorCheckManager->Del(pAuth->nAccountDBID);
			}
		}
		break;
	case QUERY_RESETERRORCHECK :
		{
			TQHeader* pReset = reinterpret_cast<TQHeader*>(pData);
			g_pSPErrorCheckManager->Del(pReset->nAccountDBID);
		}
		break;
	case QUERY_RESETAUTH:				// Reset auth info (specific account) (used by GA, VI)
		{
			TQResetAuth* pAuth = reinterpret_cast<TQResetAuth*>(pData);
			TAResetAuth Auth;
			memset(&Auth, 0, sizeof(Auth));

			Auth.nAccountDBID = pAuth->nAccountDBID;
			Auth.nSessionID = pAuth->nSessionID;
			Auth.nRetCode = ERROR_DB;		// SQL error occurred

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Auth.nRetCode = pMembershipDB->QueryResetAuth(pAuth->nAccountDBID, pAuth->nSessionID);
				if (ERROR_NONE != Auth.nRetCode) {
					//g_Log.Log(LogType::_ERROR, 0, pAuth->nAccountDBID, 0, pAuth->nSessionID, L"[ADBID:%d][QUERY_RESETAUTH] Query Error Result:%d\r\n", pAuth->nAccountDBID, Auth.nRetCode);
				}
			}
			else{
				//g_Log.Log(LogType::_ERROR, 0, pAuth->nAccountDBID, 0, pAuth->nSessionID, L"[QUERY_RESETAUTH] Query not found\r\n");
			}
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Auth, sizeof(Auth));
		}
		break;

	case QUERY_RESETAUTHSERVER:				// Reset auth info (specific server)
		{
			TQResetAuthServer* pAuth = reinterpret_cast<TQResetAuthServer*>(pData);
			TAResetAuthServer Auth;
			memset(&Auth, 0, sizeof(Auth));

			Auth.nResult = ERROR_DB;		// SQL error occurred
			Auth.nServerID = g_Config.nManagedID;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Auth.nResult = pMembershipDB->QueryResetAuthServer(pAuth->nServerID);
				if (ERROR_NONE != Auth.nResult) {
					g_Log.Log(LogType::_ERROR, L"[QUERY_RESETAUTHSERVER] Query Error Result:%d\r\n", Auth.nResult);
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, L"[QUERY_RESETAUTHSERVER] Query not found\r\n");
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Auth, sizeof(Auth));
		}
		break;
	}
}