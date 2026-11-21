#include "stdafx.h"
#include "DNDWCTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLWorld.h"
#include "Log.h"
#include "Util.h"

#if defined( PRE_ADD_DWC )

CDNDWCTask::CDNDWCTask(CDNConnection* pConnection)
	: CDNMessageTask(pConnection)
{
}

CDNDWCTask::~CDNDWCTask()
{
}

void CDNDWCTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	switch (nSubCmd)
	{
	case QUERY_ADD_DWCTEAM:
		{
			TQAddDWCTeam* pQ = reinterpret_cast<TQAddDWCTeam*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QUERY_ADD_DWCTEAM] pWorldDB not found\r\n");
				break;
			}
			TAAddDWCTeam Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			_wcscpy(Answer.wszTeamName, _countof(Answer.wszTeamName), pQ->wszTeamName, (int)wcslen(pQ->wszTeamName));
			Answer.nRetCode = pWorldDB->QueryAddDWCTeam(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_ADD_DWCTEAMMEMBER:
		{
			TQAddDWCTeamMember* pQ = reinterpret_cast<TQAddDWCTeamMember*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QUERY_ADD_DWCTEAMMEMBER] pWorldDB not found\r\n");
				break;
			}
			TAAddDWCTeamMember Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nTeamID = pQ->nTeamID;
			_wcscpy(Answer.wszCharacterName, _countof(Answer.wszCharacterName), pQ->wszCharacterName, (int)wcslen(pQ->wszCharacterName));
			Answer.nRetCode = pWorldDB->QueryAddDWCTeamMember(pQ);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_DEL_DWCTEAMMEMBER:
		{
			TQDelDWCTeamMember* pQ = reinterpret_cast<TQDelDWCTeamMember*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QUERY_DEL_DWCTEAMMEMBER] pWorldDB not found\r\n");
				break;
			}
			TADelDWCTeamMember Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nTeamID = pQ->nTeamID;
			Answer.biCharacterDBID = pQ->biCharacterDBID;
			_wcscpy(Answer.wszCharacterName, _countof(Answer.wszCharacterName), pQ->wszCharacterName, (int)wcslen(pQ->wszCharacterName));
			Answer.nRetCode = pWorldDB->QueryDelDWCTeamMember(pQ);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_GET_DWCTEAM_BY_CHAR:
		{
			TQGetDWCTeamByChar* pQ = reinterpret_cast<TQGetDWCTeamByChar*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, pQ->biCharacterDBID, 0, L"[QUERY_GET_DWCTEAM_BY_CHAR] pWorldDB not found\r\n");
				break;
			}
			TAGetDWCTeam Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.bNeedMemberList = pQ->bNeedMemberList;
			Answer.nRetCode = pWorldDB->QueryGetDWCTeamByChar(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_GETLIST_DWCTEAMMEMBER:
		{
			TQGetListDWCTeamMember* pQ = reinterpret_cast<TQGetListDWCTeamMember*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QUERY_GETLIST_DWCTEAMMEMBER] pWorldDB not found\r\n");
				break;
			}
			TAGetListDWCTeamMember Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nTeamID = pQ->nTeamID;
			Answer.nRetCode = pWorldDB->QueryGetListDWCTeamMember(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_GETPAGE_DWCRANKING:
		{
			TQGetPageDWCRanking* pQ = reinterpret_cast<TQGetPageDWCRanking*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QUERY_GETPAGE_DWCRANKING] pWorldDB not found\r\n");
				break;
			}
			TAGetPageDWCRanking Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nPageNum = pQ->nPageNum;
			Answer.nRetCode = pWorldDB->QueryGetPageDWCRanking(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_GET_DWCRANKING:
		{
			TQGetDWCRanking* pQ = reinterpret_cast<TQGetDWCRanking*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QUERY_GET_DWCRANKING] pWorldDB not found\r\n");
				break;
			}
			TAGetDWCRanking Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nRetCode = pWorldDB->QueryGetDWCRanking(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	case QUERY_GET_DWCCHANNEL:
		{
			TQGetDWCChannel* pQ = reinterpret_cast<TQGetDWCChannel*>(pData);
			CDNSQLWorld* pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pQ->cWorldSetID);
			if (!pWorldDB)
			{
				g_Log.Log(LogType::_ERROR, pQ->cWorldSetID, pQ->nAccountDBID, 0, 0, L"[QUERY_GET_DWCCHANNEL] pWorldDB not found\r\n");
				break;
			}
			TAGetDWCChannel Answer;
			memset(&Answer, 0, sizeof(Answer));
			Answer.nAccountDBID = pQ->nAccountDBID;
			Answer.nRetCode = pWorldDB->QueryGetDWCChannel(pQ, &Answer);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, reinterpret_cast<char*>(&Answer), sizeof(Answer));
		}
		break;

	default:
		break;
	}
}

#endif // #if defined( PRE_ADD_DWC )
