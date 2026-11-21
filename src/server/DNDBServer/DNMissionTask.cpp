#include "stdafx.h"
#include "DNMissionTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

CDNMissionTask::CDNMissionTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNMissionTask::~CDNMissionTask(void)
{
}

void CDNMissionTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_MISSIONGAIN:
		{
			TQMissionGain *pUpdate = (TQMissionGain*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QueryUpsGainMissionBin(pUpdate->biCharacterDBID, pUpdate->MissionGain);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MISSIONGAIN:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MISSIONGAIN:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;

	case QUERY_MISSIONACHIEVE:
		{
			TQMissionAchieve *pUpdate = (TQMissionAchieve*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QueryAddAchieveMission(pUpdate->biCharacterDBID, pUpdate->nMissionID, pUpdate->cMissionType, pUpdate->MissionAchieve);

				switch (nRet)
				{
				case ERROR_NONE:
				case 103125:	// Mission already achieved
					break;

				default:
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MISSIONACHIEVE:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);
					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MISSIONACHIEVE:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;

	case QUERY_SETDAILYMISSION:
		{
			TQSetDailyMission *pUpdate = (TQSetDailyMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_DAY, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETDAILYMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETDAILYMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;

	case QUERY_SETWEEKLYMISSION:
		{
			TQSetWeeklyMission *pUpdate = (TQSetWeeklyMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_WEEK, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETWEEKLYMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETWEEKLYMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;

#if defined(PRE_ADD_MONTHLY_MISSION)
	case QUERY_SETMONTHLYMISSION:
		{
			TQSetMonthlyMission *pUpdate = (TQSetMonthlyMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_MONTH, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETMONTHLYMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETMONTHLYMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

	case QUERY_SETWEEKENDEVENTMISSION:
		{
			TQSetWeekendEventMission *pUpdate = (TQSetWeekendEventMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_WEEKENDEVENT, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETEVENTMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETEVENTMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
	case QUERY_SETGUILDWARMISSION:
		{
			TQSetGuildWarMission *pUpdate = (TQSetGuildWarMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_GUILDWAR, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETGUILDWARMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETGUILDWARMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
	case QUERY_SETGUILDCOMMONMISSION:
		{
			TQSetGuildCommonMission *pUpdate = (TQSetGuildCommonMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_GUILDCOMMON, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETGUILDCOMMONMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETGUILDCOMMONMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
	case QUERY_SETWEEKENDREPEATMISSION:
		{
			TQSetWeekendRepeatMission *pUpdate = (TQSetWeekendRepeatMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_WEEKENDREPEAT, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETWEEKENDREPEATMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETWEEKENDREPEATMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;

	case QUERY_SETPCBANGMISSION:
		{
			TQSetPCBangMission *pUpdate = (TQSetPCBangMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QuerySetDailyWeeklyMission(pUpdate->biCharacterDBID, DAILYMISSION_PCBANG, pUpdate->nMissionIDs);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETPCBANGMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_SETPCBANGMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
	case QUERY_MODDAILYWEEKLYMISSION:
		{
			TQModDailyWeeklyMission *pUpdate = (TQModDailyWeeklyMission*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pUpdate->cWorldSetID);
			if (pWorldDB){
				nRet = pWorldDB->QueryModDailyWeeklyMissionDetail(pUpdate->biCharacterDBID, pUpdate->cDailyMissionType, pUpdate->cMissionIndex, pUpdate->Mission, pUpdate->nMissionScore);
				if (nRet != ERROR_NONE){
					g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MODDAILYWEEKLYMISSION:%d] Query Error Ret:%d\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID, nRet);

					m_pConnection->QueryResultError(pUpdate->nAccountDBID, nRet, nMainCmd, nSubCmd);	// DB returned some error value, so just disconnect
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pUpdate->cWorldSetID, pUpdate->nAccountDBID, pUpdate->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_MODDAILYWEEKLYMISSION:%d] pWorldDB not found\r\n", pUpdate->biCharacterDBID, pUpdate->cWorldSetID);
			}
		}
		break;
	case QUERY_CLEARMISSION :
		{
			TQClearMission *pClearMission = (TQClearMission*)pData;
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pClearMission->cWorldSetID);
			TAHeader tReturn;			
			tReturn.nAccountDBID = pClearMission->nAccountDBID;
			tReturn.nRetCode = ERROR_DB;
			if (pWorldDB)
			{				
				tReturn.nRetCode = pWorldDB->QueryDelAllMissionsForCheat(pClearMission->biCharacterDBID);
			}
			else
				g_Log.Log(LogType::_ERROR, pClearMission->cWorldSetID, pClearMission->nAccountDBID, pClearMission->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_CLEARMISSION:%d] pWorldDB not found\r\n", pClearMission->biCharacterDBID, pClearMission->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&tReturn), sizeof(tReturn) );			
		}
		break;
	}
}