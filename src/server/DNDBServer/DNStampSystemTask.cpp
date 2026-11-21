#include "stdafx.h"

#include "DNStampSystemTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"

#if defined( PRE_ADD_STAMPSYSTEM )

CDNStampSystemTask::CDNStampSystemTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{
}

CDNStampSystemTask::~CDNStampSystemTask(void)
{
}

void CDNStampSystemTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_GETLIST_COMPLETECHALLENGES:
		{
			TQGetListCompleteChallenges* pStamp = reinterpret_cast<TQGetListCompleteChallenges*>(pData);
			TAGetListCompleteChallenges Stamp;
			memset( &Stamp, 0, sizeof(Stamp) );

			Stamp.nRetCode = ERROR_DB;
			Stamp.nAccountDBID = pStamp->nAccountDBID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pStamp->cWorldSetID );
			if( pWorldDB )
			{
				Stamp.nRetCode = pWorldDB->QueryGetListCompleteChallenges( pStamp, &Stamp );

				if( Stamp.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_GETLIST_COMPLETECHALLENGES:%d] Query Error Ret:%d\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID, Stamp.nRetCode );
			}
			else
				g_Log.Log(LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_GETLIST_COMPLETECHALLENGES:%d] pWorldDB Error\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID );

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Stamp), sizeof(Stamp) );
		}
		break;
	case QUERY_INIT_COMPLETECHALLENGE:
		{
			TQInitCompleteChallenge* pStamp = reinterpret_cast<TQInitCompleteChallenge*>(pData);
			TAHeader Stamp;
			memset( &Stamp, 0, sizeof(Stamp) );

			Stamp.nRetCode = ERROR_DB;
			Stamp.nAccountDBID = pStamp->nAccountDBID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pStamp->cWorldSetID );
			if( pWorldDB )
			{
				Stamp.nRetCode = pWorldDB->QueryInitCompleteChallenge( pStamp );

				if( Stamp.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_INIT_COMPLETECHALLENGE:%d] Query Error Ret:%d\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID, Stamp.nRetCode );
			}
			else
				g_Log.Log(LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_INIT_COMPLETECHALLENGE:%d] pWorldDB Error\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID );

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Stamp), sizeof(Stamp) );
		}
		break;
	case QUERY_ADD_COMPLETECHALLENGE:
		{
			TQAddCompleteChallenge* pStamp = reinterpret_cast<TQAddCompleteChallenge*>(pData);
			TAHeader Stamp;
			memset( &Stamp, 0, sizeof(Stamp) );

			Stamp.nRetCode = ERROR_DB;
			Stamp.nAccountDBID = pStamp->nAccountDBID;

			pWorldDB = g_SQLConnectionManager.FindWorldDB( nThreadID, pStamp->cWorldSetID );
			if( pWorldDB )
			{
				Stamp.nRetCode = pWorldDB->QueryAddCompleteChallenge( pStamp );

				if( Stamp.nRetCode != ERROR_NONE )
					g_Log.Log( LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_ADD_COMPLETECHALLENGE:%d] Query Error Ret:%d\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID, Stamp.nRetCode );
			}
			else
				g_Log.Log(LogType::_ERROR, pStamp->cWorldSetID, pStamp->nAccountDBID, pStamp->biCharacterDBID, 0, L"[CDBID:%lld] [QUERY_ADD_COMPLETECHALLENGE:%d] pWorldDB Error\r\n", pStamp->biCharacterDBID, pStamp->cWorldSetID );

			m_pConnection->AddSendData( nMainCmd, nSubCmd, reinterpret_cast<char*>(&Stamp), sizeof(Stamp) );
		}
		break;
	}
}

#endif // #if defined( PRE_ADD_STAMPSYSTEM )