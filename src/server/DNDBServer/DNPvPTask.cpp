#include "stdafx.h"
#include "DNPvPTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

#if defined( PRE_WORLDCOMBINE_PVP  )
extern TDBConfig g_Config;
#endif

CDNPvPTask::CDNPvPTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNPvPTask::~CDNPvPTask(void)
{
}

void CDNPvPTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_UPDATEPVPDATA:
		{
			TQUpdatePvPData* pPvP = reinterpret_cast<TQUpdatePvPData*>(pData);
			TAUpdatePvPData PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID	= pPvP->nAccountDBID;
			PvP.Type			= pPvP->Type;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryModPvPScore( pPvP, &PvP );

				if (PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_UPDATEPVPDATA:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_UPDATEPVPDATA:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			if( PvP.Type != PvPCommon::QueryUpdatePvPDataType::Disconnect )
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			break;
		}
	case QUERY_GET_PVP_GHOULSCORES :
		{
			TQGetPVPGhoulScores* pPvP = reinterpret_cast<TQGetPVPGhoulScores*>(pData);
			TAGetPVPGhoulScores PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryGetPVPGhoulScores( pPvP->biCharacterDBID, &PvP );

				if (PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GET_PVP_GHOULSCORES:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GET_PVP_GHOULSCORES:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			break;				
		}
		break;
	case QUERY_ADD_PVP_GHOULSCORES :
		{
			TQAddPVPGhoulScores* pPvP = reinterpret_cast<TQAddPVPGhoulScores*>(pData);

			int nRetCode = 0;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				nRetCode = pWorldDB->QueryAddPVPGhoulScores( pPvP );

				if (nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GET_PVP_GHOULSCORES:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR,  pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GET_PVP_GHOULSCORES:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_ADD_PVP_LADDERRESULT:
		{
			TQAddPvPLadderResult* pPvP = reinterpret_cast<TQAddPvPLadderResult*>(pData);
			TAAddPvPLadderResult PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;
			PvP.Type			= pPvP->Type;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryAddPvPLadderResult( pPvP, &PvP );

				if (PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_ADD_PVP_LADDERRESULT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_ADD_PVP_LADDERRESULT:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			break;
		}

	case QUERY_GETLIST_PVP_LADDERSCORE:
		{
			TQGetListPvPLadderScore* pPvP = reinterpret_cast<TQGetListPvPLadderScore*>(pData);
			TAGetListPvPLadderScore PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryGetListPvPLadderScore( pPvP, &PvP );

				if (PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GETLIST_PVP_LADDERSCORE:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GETLIST_PVP_LADDERSCORE:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			int iSize = sizeof(PvP)-sizeof(PvP.Data);
			if( PvP.nRetCode == ERROR_NONE )
				iSize += (sizeof(PvP.Data)-sizeof(PvP.Data.LadderScore)+(PvP.Data.cLadderTypeCount*sizeof(PvP.Data.LadderScore[0])));
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), iSize );
			break;
		}

	case QUERY_GETLIST_PVP_LADDERSCORE_BYJOB:
		{
			TQGetListPvPLadderScoreByJob* pPvP = reinterpret_cast<TQGetListPvPLadderScoreByJob*>(pData);
			TAGetListPvPLadderScoreByJob PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryGetListPvPLadderScoreByJob( pPvP, &PvP );

				if (PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GETLIST_PVP_LADDERSCORE_BYJOB:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GETLIST_PVP_LADDERSCORE_BYJOB:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			int iSize = sizeof(PvP)-sizeof(PvP.Data);
			if( PvP.nRetCode == ERROR_NONE )
				iSize += (sizeof(PvP.Data)-sizeof(PvP.Data.LadderScoreByJob)+(PvP.Data.cJobCount*sizeof(PvP.Data.LadderScoreByJob[0])));
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), iSize );
			break;
		}

	case QUERY_INIT_PVP_LADDERGRADEPOINT:
		{
			TQInitPvPLadderGradePoint* pPvP = reinterpret_cast<TQInitPvPLadderGradePoint*>(pData);
			TAInitPvPLadderGradePoint PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryInitPvPLadderGradePoint( pPvP, &PvP );
				if( PvP.nRetCode != ERROR_NONE )
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_INIT_PVP_LADDERGRADEPOINT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_INIT_PVP_LADDERGRADEPOINT:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			if( PvP.nRetCode != ERROR_NONE )
				m_pConnection->QueryResultError( pPvP->nAccountDBID, PvP.nRetCode, nMainCmd, nSubCmd );

			break;
		}

	case QUERY_USE_PVP_LADDERPOINT:
		{
			TQUsePvPLadderPoint* pPvP = reinterpret_cast<TQUsePvPLadderPoint*>(pData);
			TAUsePvPLadderPoint PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryUsePvPLadderPoint( pPvP, &PvP );
				if( PvP.nRetCode != ERROR_NONE )
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_USE_PVP_LADDERPOINT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
				else
				{
					if( PvP.iPvPLadderPointAfter != pPvP->iAfterLadderPoint )
						PvP.nRetCode = ERROR_LADDERSYSTEM_LADDERPOINT_INVALID;
				}
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_USE_PVP_LADDERPOINT:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);

			if( PvP.nRetCode != ERROR_NONE )
				m_pConnection->QueryResultError( pPvP->nAccountDBID, PvP.nRetCode, nMainCmd, nSubCmd );

			break;
		}

	case QUERY_ADD_PVP_LADDERCUCOUNT:
		{
			TQAddPvPLadderCUCount* pPvP = reinterpret_cast<TQAddPvPLadderCUCount*>(pData);

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				nRet = pWorldDB->QueryAddPvPLadderCUCount( pPvP );
				if( nRet != ERROR_NONE )
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_ADD_PVP_LADDERCUCOUNT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRet);
			}

			break;
		}

	case QUERY_MOD_PVP_LADDERSCORE_FORCHEAT:
		{
			TQModPvPLadderScoresForCheat* pPvP = reinterpret_cast<TQModPvPLadderScoresForCheat*>(pData);
			TAModPvPLadderScoresForCheat PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID					= pPvP->nAccountDBID;
			PvP.nRetCode						= ERROR_DB;
			PvP.cPvPLadderCode				= pPvP->cPvPLadderCode;
			PvP.iPvPLadderPoint				= pPvP->iPvPLadderPoint;
			PvP.iPvPLadderGradePoint			= pPvP->iPvPLadderGradePoint;
			PvP.iHiddenPvPLadderGradePoint	= pPvP->iHiddenPvPLadderGradePoint;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryModPvPLadderScoresForCheat( pPvP );
				if( PvP.nRetCode != ERROR_NONE )
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_MOD_PVP_LADDERSCORE_FORCHEAT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			break;
		}
	case QUERY_GETLIST_PVPLADDERRANKING:
		{
			TQGetListPvPLadderRanking* pPvP = reinterpret_cast<TQGetListPvPLadderRanking*>(pData);
			TAGetListPvPLadderRanking PvP;
			memset( &PvP, 0, sizeof(PvP) );

			PvP.nAccountDBID	= pPvP->nAccountDBID;
			PvP.nRetCode		= ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryGetListPvPLadderRanking( pPvP, &PvP );
				if( PvP.nRetCode != ERROR_NONE )
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_GETLIST_PVPLADDERRANKING:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);
			}

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			break;
		}
#ifdef PRE_MOD_PVPRANK
	case QUERY_UPDATE_PVPRANKCRITERIA:
		{
			TQUpdatePvPRankCriteria * pPacket = (TQUpdatePvPRankCriteria*)pData;

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB )
			{
				nRetCode = pWorldDB->QuerySetPvPRankCriteria(pPacket->nThresholdPvPExp);
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_UPDATE_PVPRANKCRITERIA:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
				else
				{
					g_Log.Log(LogType::_NORMAL, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_UPDATE_PVPRANKCRITERIA:%d] Threshold[%d]\r\n", pPacket->cWorldSetID, pPacket->nThresholdPvPExp);
					m_pConnection->AddSendData(nMainCmd, nSubCmd, NULL, 0);
				}
			}
		}
		break;

	case QUERY_FORCE_CALCPVPRANK:
		{
			TQForceCalcPvPRank * pPacket = (TQForceCalcPvPRank*)pData;
			
			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryCalcPvPRank();
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_UPDATE_PVPRANKCRITERIA:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
		}
		break;
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
	case QUERY_GET_PVP_RANKBOARD:
		{
			TQGetPvPRankBoard * pPacket = (TQGetPvPRankBoard*)pData;
			TAGetPvPRankBoard PvPRankBoard;
			memset( &PvPRankBoard, 0, sizeof(PvPRankBoard) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPRankInfo(pPacket, pPacket->biCharacterDBID, NULL, &PvPRankBoard.MyRanking );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[QUERY_GET_PVPRANKBOARDL:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[QUERY_GET_PVPRANKBOARDL:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPRankBoard.nRetCode = nRetCode;
			PvPRankBoard.nAccountDBID = pPacket->nAccountDBID;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPRankBoard), sizeof(PvPRankBoard) );
		}
		break;

	case QUERY_GET_PVP_RANKLIST:
		{
			TQGetPvPRankList * pPacket = (TQGetPvPRankList*)pData;
			TAGetPvPRankList PvPRankList;
			memset( &PvPRankList, 0, sizeof(PvPRankList) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPRankList(pPacket, &PvPRankList );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVPRANKLIST:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVPRANKBOARDL:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPRankList.nRetCode = nRetCode;
			PvPRankList.nAccountDBID = pPacket->nAccountDBID;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPRankList), sizeof(PvPRankList) );			
		}
		break;
	case QUERY_GET_PVP_RANKINFO:
		{
			TQGetPvPRankInfo * pPacket = (TQGetPvPRankInfo*)pData;
			TAGetPvPRankInfo PvPRankInfo;
			memset( &PvPRankInfo, 0, sizeof(PvPRankInfo) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPRankInfo(pPacket, 0, pPacket->wszCharName ,&PvPRankInfo.RankingInfo );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVPRANKINFO:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVPRANKINFO:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPRankInfo.nRetCode = nRetCode;
			PvPRankInfo.nAccountDBID = pPacket->nAccountDBID;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPRankInfo), sizeof(PvPRankInfo) );

		}
		break;

	case QUERY_GET_PVP_LADDER_RANKBOARD:
		{
			TQGetPvPLadderRankBoard * pPacket = (TQGetPvPLadderRankBoard*)pData;
			TAGetPvPLadderRankBoard PvPLadderRankBoard;
			memset( &PvPLadderRankBoard, 0, sizeof(PvPLadderRankBoard) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPLadderRankInfo(pPacket, pPacket->biCharacterDBID, NULL, pPacket->cPvPLadderCode, &PvPLadderRankBoard.MyRanking );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[QUERY_GET_PVP_LADDER_RANKBOARD:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, 0, L"[QUERY_GET_PVP_LADDER_RANKBOARD:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPLadderRankBoard.nRetCode = nRetCode;
			PvPLadderRankBoard.nAccountDBID = pPacket->nAccountDBID;
			PvPLadderRankBoard.cPvPLadderCode = pPacket->cPvPLadderCode;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPLadderRankBoard), sizeof(PvPLadderRankBoard) );
		}
		break;

	case QUERY_GET_PVP_LADDER_RANKLIST:
		{
			TQGetPvPLadderRankList * pPacket = (TQGetPvPLadderRankList*)pData;
			TAGetPvPLadderRankList PvPLadderRankList;
			memset( &PvPLadderRankList, 0, sizeof(PvPLadderRankList) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPLadderRankList(pPacket, &PvPLadderRankList );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVP_LADDER_RANKLIST:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVP_LADDER_RANKLIST:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPLadderRankList.nRetCode = nRetCode;
			PvPLadderRankList.nAccountDBID = pPacket->nAccountDBID;
			PvPLadderRankList.cPvPLadderCode = pPacket->cPvPLadderCode;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPLadderRankList), sizeof(PvPLadderRankList) );			
		}
		break;
	case QUERY_GET_PVP_LADDER_RANKINFO:
		{
			TQGetPvPLadderRankInfo * pPacket = (TQGetPvPLadderRankInfo*)pData;
			TAGetPvPLadderRankInfo PvPLadderRankInfo;
			memset( &PvPLadderRankInfo, 0, sizeof(PvPLadderRankInfo) );

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID );
			if (pWorldDB)
			{
				nRetCode = pWorldDB->QueryGetPvPLadderRankInfo(pPacket, 0, pPacket->wszCharName, pPacket->cPvPLadderCode, &PvPLadderRankInfo.RankingInfo );
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVP_LADDER_RANKINFO:%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
			}
			else
				g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_GET_PVP_LADDER_RANKINFO:%d] pWorldDB Error\r\n", pPacket->cWorldSetID);

			PvPLadderRankInfo.nRetCode = nRetCode;
			PvPLadderRankInfo.nAccountDBID = pPacket->nAccountDBID;
			PvPLadderRankInfo.cPvPLadderCode = pPacket->cPvPLadderCode;
			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvPLadderRankInfo), sizeof(PvPLadderRankInfo) );

		}
		break;
	case QUERY_FORCE_AGGREGATE_PVP_RANKING:
		{
			TQForceAggregatePvPRank * pPacket = (TQForceAggregatePvPRank*)pData;

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPacket->cWorldSetID);
			if (pWorldDB)
			{
				if( pPacket->bIsPvPLadder )
				{
					nRetCode = pWorldDB->QueryAggregatePvPLadderRank();
					if(nRetCode != ERROR_NONE)
						g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_FORCE_AGGREGATE_PVP_RANKING(ladder):%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
				}
				else
				{
					nRetCode = pWorldDB->QueryAggregatePvPRank();
					if(nRetCode != ERROR_NONE)
						g_Log.Log(LogType::_ERROR, pPacket->cWorldSetID, pPacket->nAccountDBID, 0, 0, L"[QUERY_FORCE_AGGREGATE_PVP_RANKING(pvp):%d] (Ret:%d)\r\n", pPacket->cWorldSetID, nRetCode);
				}
			}
		}
		break;
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
	case QUERY_ADD_WORLDPVPROOM:
		{
			TQAddWorldPvPRoom * pPvP = (TQAddWorldPvPRoom*)pData;

			TAAddWorldPvPRoom PvP;
			memset(&PvP, 0, sizeof(PvP));

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			int nRoomDBIndex = 0;
			PvP.cWorldSetID = pPvP->cWorldSetID;
			PvP.nAccountDBID = pPvP->nAccountDBID;
			PvP.nRoomIndex = pPvP->TMissionRoom.nItemID;
			PvP.nRoomID = pPvP->nRoomID;			
			
			if (pWorldDB)
			{	
				PvP.nRetCode = pWorldDB->QueryAddWorldPvPRoom(pPvP, nRoomDBIndex);
				PvP.nPvPRoomDBIndex = nRoomDBIndex;
				if( PvP.nAccountDBID > 0 )
				{
					PvP.Data.nDBRoomID = nRoomDBIndex;
					PvP.Data.nWorldID = pPvP->cWorldSetID;
					PvP.Data.nServerID = pPvP->nServerID;
					PvP.Data.nRoomID = pPvP->nRoomID;
					PvP.Data.cModeID = pPvP->TMissionRoom.cModeID;
					_wcscpy(PvP.Data.wszRoomName, _countof(PvP.Data.wszRoomName), pPvP->wszRoomName, (int)(wcslen(pPvP->wszRoomName)));
					PvP.Data.cMaxPlayers = pPvP->TMissionRoom.cMaxPlayers;
					PvP.Data.nMapID = pPvP->TMissionRoom.nMapID;
					PvP.Data.nPlayTimeSec = pPvP->TMissionRoom.nPlayTime;
					PvP.Data.nWinCondition = pPvP->TMissionRoom.nWinCondition;
				}				
				
				if(PvP.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_ADD_WORLDPVPROOM:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode);				
				
				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), sizeof(PvP) );
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_ADD_WORLDPVPROOM:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_ADD_WORLDPVPROOMMEMBER:
		{
			TQAddWorldPvPRoomMember * pPvP = (TQAddWorldPvPRoomMember*)pData;
			
			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if (pWorldDB)
			{	
				nRetCode = pWorldDB->QueryAddWorldPvPRoomMember(pPvP);			
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_ADD_WORLDPVPROOMMEMBER:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode);				

			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_ADD_WORLDPVPROOMMEMBER:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_DEL_WORLDPVPROOM:
		{
			TQDelWorldPvPRoom * pPvP = (TQDelWorldPvPRoom*)pData;
			
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if (pWorldDB)
			{	
				int nRetCode = ERROR_DB; 
				nRetCode = pWorldDB->QueryDelWorldPvPRoom(pPvP->nWorldPvPRoomDBIndex);
				
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_DEL_WORLDPVPROOM:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode);								
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_DEL_WORLDPVPROOM:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_DEL_WORLDPVPROOMMEMBER:
		{
			TQDelWorldPvPRoomMember * pPvP = (TQDelWorldPvPRoomMember*)pData;

			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if (pWorldDB)
			{	
				nRetCode = pWorldDB->QueryDelWorldPvPRoomMember(pPvP);			
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_DEL_WORLDPVPROOMMEMBER:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode);				

			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_DEL_WORLDPVPROOMMEMBER:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_GETLIST_WORLDPVPROOM:
		{
			TQGetListWorldPvPRoom * pPvP = (TQGetListWorldPvPRoom*)pData;

			TAGetListWorldPvPRoom PvP;
			memset(&PvP, 0, sizeof(PvP));
			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.nCombineWorldDBID );
			int nRoomIndex = 0;
			if (pWorldDB)
			{	
				nRetCode = pWorldDB->QueryGetListWorldPvPRoom(pPvP->nCombineWorldID, &PvP);
				if(nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_GETLIST_WORLDPVPROOM:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode);				

				int iSize = sizeof(PvP)-sizeof(PvP.WorldPvPRoomData)+(PvP.nCount*sizeof(PvP.WorldPvPRoomData[0]));

				m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&PvP), iSize );
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_GETLIST_WORLDPVPROOM:%d] pWorldDB Error\r\n", pPvP->cWorldSetID);
		}
		break;
	case QUERY_DEL_WORLDPVPROOMFORSERVER:
		{
			TQDelWorldPvPRoomForServer * pPvP = (TQDelWorldPvPRoomForServer*)pData;

			int nRetCode = ERROR_DB;
			
			if( g_Config.nCombineWorldDBID > 0 )
			{
				pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, g_Config.nCombineWorldDBID );
				if( pWorldDB )
				{
					nRetCode = pWorldDB->QueryDelWorldPvPRoomForServer(pPvP->nServerID);

					if( nRetCode != ERROR_NONE )
						g_Log.Log( LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_DEL_WORLDPVPROOMFORSERVER:%d] (Ret:%d) SID:%d\r\n", g_Config.nCombineWorldDBID, nRetCode, pPvP->nServerID );
				}
			}
			else
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_DEL_WORLDPVPROOMFORSERVER:%d] pWorldDB Error\r\n", g_Config.nCombineWorldDBID);
		}
		break;
	case QUERY_UPDATE_WORLDPVPROOM:
		{
			TQUpdateWorldPvPRoom * pPvP = (TQUpdateWorldPvPRoom*)pData;

			TAUpdateWorldPvPRoom PvP;
			memset(&PvP, 0, sizeof(PvP));
			PvP.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				PvP.nRetCode = pMembershipDB->QueryUpdateWorldPvPRoom(pPvP, &PvP);
				if (ERROR_NONE != PvP.nRetCode) {
					g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_UPDATE_WORLDPVPROOM] Query Error Result:%d\r\n", PvP.nRetCode);
				}
			}
			else{
				g_Log.Log(LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_UPDATE_WORLDPVPROOM] Query not found\r\n");
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&PvP, sizeof(PvP));
		}
		break;
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
	case QUERY_ADD_PVP_TOURNAMENTRESULT :
		{
			TQAddPVPTournamentResult* pPvP = (TQAddPVPTournamentResult*)pData;
			int nRetCode = ERROR_DB;
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				nRetCode = pWorldDB->QueryAddPVPTournamentResult(pPvP);
				if( nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_ADD_PVP_TOURNAMENTRESULT:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode );
			}
		}
		break;
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_FIX_76282 )
	case SYNC_GOPVPLOBBY :
		{
			TQHeader * pPvP = (TQHeader*)pData;

			TAHeader PvP;
			memset(&PvP, 0, sizeof(PvP));
			PvP.nRetCode = ERROR_NONE;
			PvP.nAccountDBID = pPvP->nAccountDBID;

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&PvP, sizeof(PvP));
		}
		break;
#endif // #if defined( PRE_FIX_76282 )
#if defined( PRE_PVP_GAMBLEROOM )
	case QUERY_ADD_GAMBLEROOM :
		{
			TQAddGambleRoom* pPvP = (TQAddGambleRoom*)pData;
			TAAddGambleRoom PvP;
			memset(&PvP, 0, sizeof(PvP));

			PvP.nRoomID = pPvP->nRoomID;
			PvP.cWorldSetID = pPvP->cWorldSetID;
			
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				PvP.nRetCode = pWorldDB->QueryAddGambleRoom(pPvP, &PvP);
				if( PvP.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_ADD_GAMBLEROOM:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, PvP.nRetCode );
				
				m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&PvP, sizeof(PvP));
			}
		}
		break;

	case QUERY_ADD_GAMBLEROOMMEMBER :
		{
			TQAddGambleRoomMember* pPvP = (TQAddGambleRoomMember*)pData;
			
			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				int nRetCode = pWorldDB->QueryAddGambleRoomMember(pPvP);
				if( nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, pPvP->biCharacterDBID, 0, L"[QUERY_ADD_GAMBLEROOMMEMBER:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode );
			}
		}
		break;
	case QUERY_END_GAMBLEROOM :
		{
			TQEndGambleRoom* pPvP = (TQEndGambleRoom*)pData;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pPvP->cWorldSetID );
			if( pWorldDB )
			{
				int nRetCode = pWorldDB->QueryEndGambleRoom(pPvP);
				if( nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pPvP->cWorldSetID, pPvP->nAccountDBID, 0, 0, L"[QUERY_END_GAMBLEROOM:%d] (Ret:%d)\r\n", pPvP->cWorldSetID, nRetCode );
			}
		}
		break;
#endif
	}
}
