#include "stdafx.h"
#include "DNLogTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "DNExtManager.h"
#include "Log.h"
#include "Util.h"

CDNLogTask::CDNLogTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNLogTask::~CDNLogTask(void)
{
}

void CDNLogTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_ADDPARTY_STARTLOG:
		{
			TQAddPartyStartLog* pLog = reinterpret_cast<TQAddPartyStartLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddPartyStartLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPARTY_STARTLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPARTY_STARTLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);

			break;
		}

	case QUERY_ADDPARTY_ENDLOG:
		{
			TQAddPartyEndLog* pLog = reinterpret_cast<TQAddPartyEndLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddPartyEndLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPARTY_ENDLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPARTY_ENDLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDSTAGE_STARTLOG:
		{
			TQAddStageStartLog* pLog = reinterpret_cast<TQAddStageStartLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddStageStartLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_STARTLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_STARTLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDSTAGE_CLEARLOG:
		{
			TQAddStageClearLog* pLog = reinterpret_cast<TQAddStageClearLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddStageClearLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_CLEARLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_CLEARLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDSTAGE_REWARDLOG:
		{
			TQAddStageRewardLog* pLog = reinterpret_cast<TQAddStageRewardLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddStageRewardLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_REWARDLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_REWARDLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDSTAGE_ENDLOG:
		{
			TQAddStageEndLog* pLog = reinterpret_cast<TQAddStageEndLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddStageEndLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_ENDLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDSTAGE_ENDLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDPVP_STARTLOG:
		{
			TQAddPvPStartLog* pLog = reinterpret_cast<TQAddPvPStartLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddPvPStartLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_STARTLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_STARTLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDPVP_RESULTLOG:
		{
			TQAddPvPResultLog* pLog = reinterpret_cast<TQAddPvPResultLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddPvPResultLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_RESULTLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_RESULTLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}

	case QUERY_ADDPVP_ENDLOG:
		{
			TQAddPvPEndLog* pLog = reinterpret_cast<TQAddPvPEndLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddPvPEndLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_ENDLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDPVP_ENDLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
			break;
		}
#if defined( PRE_ADD_58761 )
	case QUERY_ADDNESTGATE_STARTLOG:
		{
			TQAddNestGateStartLog* pLog = reinterpret_cast<TQAddNestGateStartLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddNestGateStartLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_STARTLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_STARTLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
		}
		break;

	case QUERY_ADDNESTGATE_ENDLOG:
		{
			TQAddNestGateEndLog * pLog = (TQAddNestGateEndLog*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddNestGateEndLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_ENDLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_ENDLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
		}
		break;

	case QUERY_ADDNESTGATE_CLEARLOG:
		{
			TQAddNestGateClearLog* pLog = reinterpret_cast<TQAddNestGateClearLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddNestGateClearLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_CLEARLOG:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_ADDNESTGATE_CLEARLOG:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
		}
		break;
	case QUERY_ADDNESTDEATHLOG:
		{
			TQNestDeathLog* pLog = reinterpret_cast<TQNestDeathLog*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pLog->cWorldSetID);
			if (pWorldDB)
			{
				nRet = pWorldDB->QueryAddNestDeathLog( pLog );
			}
			else
			{
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [TQNestPlayerDeathLog:%d] WorldDB not found\r\n", pLog->nAccountDBID, pLog->cWorldSetID);
			}

			if( nRet != ERROR_NONE )
				g_Log.Log( LogType::_ERROR, pLog->cWorldSetID, pLog->nAccountDBID, 0, 0, L"[ADBID:%d] [TQNestPlayerDeathLog:%d] (Ret:%d)\r\n", pLog->nAccountDBID, pLog->cWorldSetID, nRet);
		}
		break;
#endif
	}
}