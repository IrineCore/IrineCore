#include "stdafx.h"
#include "DNGuildWarManager.h"
#include "DNGameDataManager.h"
#include "DNDBConnectionManager.h"
#include "EtUIXML.h"
#include "DNUserItem.h"
#include "DNGuildBase.h"

extern TVillageConfig g_Config;
CDNGuildWarManager * g_pGuildWarManager = NULL;

CDNGuildWarManager::CDNGuildWarManager()
{
	m_PreWinGuildUID.Reset();
	Reset();
}

CDNGuildWarManager::~CDNGuildWarManager()
{
	
}

void CDNGuildWarManager::Reset()
{
	memset(&m_sGuildWarRankingInfo, 0, sizeof(m_sGuildWarRankingInfo));
	m_nGuildWarTrialRankingCount = 0;
	memset(&m_sGuildWarTrialRankingInfo, 0, sizeof(m_sGuildWarTrialRankingInfo));
	m_bFinalProgress = false;
	memset(&m_nGuildWarMissionRankingCharacterDBID,0, sizeof(m_nGuildWarMissionRankingCharacterDBID));
	memset(&m_nGuildWarDailyAwardCharacterDBID,0, sizeof(m_nGuildWarDailyAwardCharacterDBID));

	memset(&m_sGuildWarMissionGuildRankingInfo, 0, sizeof(m_sGuildWarMissionGuildRankingInfo));
	memset(&m_sGuildWarMissionRankingInfo, 0, sizeof(m_sGuildWarMissionRankingInfo));
	memset(&m_sGuildWarDailyAward, 0, sizeof(m_sGuildWarDailyAward));

	memset(m_wszVoteGuildName, 0, sizeof(m_wszVoteGuildName));
	memset(&m_sGuildWarFinalInfo, 0, sizeof(m_sGuildWarFinalInfo));

	memset(&m_sGuildWarSchedule, 0, sizeof(m_sGuildWarSchedule));
	memset(&m_sGuildWarFinalSchedule, 0, sizeof(m_sGuildWarFinalSchedule));

	m_cGuildWarFinalPart = GUILDWAR_FINALPART_16;
	m_tGuildWarFinalPartBeginTime = 0;
	m_bGetGuildWarVote = false;
	m_nVoteCount = 0;
	m_cStepIndex = 0;
	m_nBlueTeamPoint = 0;
	m_nRedTeamPoint = 0;
	m_wScheduleID = GUILDWARSCHEDULE_DEF;
	m_wWinersWeightRate = 0;
	m_bGuildWarStats = false;
	m_bGuildWarStats = 0;
	m_cSecretTeam = 0;
	m_nSecretRandomSeed = 0;
	memset (m_vSecretMissionID, 0x00, sizeof(m_vSecretMissionID));
	m_dwPreWinSkillCoolTime = 0;
}

bool CDNGuildWarManager::GetRestriction(int nSubCmd, TGuildUID GuildUID)
{
	if( m_cStepIndex == GUILDWAR_STEP_NONE)	
		return false;		// No restriction

	switch (nSubCmd)
	{
	case eGuild::CS_INVITEGUILDMEMBREQ: // Join
		{
			if (m_cStepIndex == GUILDWAR_STEP_PREPARATION)
				return false;
			// During reward and finals periods, block joining only for guilds that advanced to the Round of 16
			if( m_cStepIndex == GUILDWAR_STEP_REWARD )
			{				
				if( GetFinalProgress() == false )
					return false;	// No restriction.

				for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; ++i )
				{
					if( !m_sGuildWarFinalInfo[i].GuildUID.IsSet() )
						continue;
					if( m_sGuildWarFinalInfo[i].GuildUID == GuildUID )
						return true;
				}
				return false;
			}
		}
		break;	
	case eGuild::CS_LEAVEGUILDMEMB:	// Withdraw
	case eGuild::CS_EXILEGUILDMEMB:	// Expel	
		{
			if (m_cStepIndex == GUILDWAR_STEP_TRIAL || m_cStepIndex == GUILDWAR_STEP_REWARD)
				return false;
		}
		break;

	case eGuild::CS_DISMISSGUILD: // Disband / Dismantle
		{
			if (m_cStepIndex == GUILDWAR_STEP_END)
				return false;
		}
	}
	return true;
}

void CDNGuildWarManager::SetGuildWarSechdule(MASetGuildWarEventTime* SetGuildWarEventTime)
{
	memcpy(m_sGuildWarSchedule, &SetGuildWarEventTime->sGuildWarTime[GUILDWAR_STEP_PREPARATION], sizeof(m_sGuildWarSchedule));
	memcpy(m_sGuildWarFinalSchedule, &SetGuildWarEventTime->sFinalPartTime[GUILDWAR_FINALPART_FINAL], sizeof(m_sGuildWarFinalSchedule));
	m_bFinalProgress = SetGuildWarEventTime->bFinalProgress;
}

int CDNGuildWarManager::GetGuildWarPoint(UINT nGuildDBID)
{
	int nRank = GUILDWAR_FINALS_TEAM_MAX+1;
	for (int i=0; i<GUILDWAR_FINALS_TEAM_MAX; i++)
	{
		if (m_sGuildWarRankingInfo[i].nGuildDBID == nGuildDBID)
		{
			nRank = m_sGuildWarRankingInfo[i].wRanking;
			break;
		}
	}

	
	TGuildWarPoint* pWarPoint = g_pDataManager->GetGuildWarPointData(nRank);
	if (!pWarPoint)
		return GUILDWARPOINT_DEF;

	return pWarPoint->nRewardGuildPoint;
}

// Finals-qualifying guild
void CDNGuildWarManager::SetGuildWarRankingInfo(TAGetGuildWarPointGuildTotal* pGetGuildWarPointGuildTotal )
{	for( int i=0; i<pGetGuildWarPointGuildTotal->nTotalCount; ++i)
	{
		m_sGuildWarRankingInfo[i].wRanking = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].wRanking;
		m_sGuildWarRankingInfo[i].nTotalPoint = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].nOpeningPointSum;
		m_sGuildWarRankingInfo[i].wCurGuildUserCount = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].wGuildMemberCount;
		memcpy(m_sGuildWarRankingInfo[i].wszGuildName, pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].wszGuildName, sizeof(m_sGuildWarRankingInfo[i].wszGuildName));
		memcpy(m_sGuildWarRankingInfo[i].wszGuildMasterName, pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].wszMasterName, sizeof(m_sGuildWarRankingInfo[i].wszGuildMasterName));

		m_sGuildWarRankingInfo[i].wMaxGuildUserCount = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].nMaxMemberCount;
		m_sGuildWarRankingInfo[i].nGuildDBID = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[i].nGuildDBID;	
	}	
	// Setup complete.. this is the last one.
	m_bGuildWarStats = true;
}
// Set guild ranking by category
void CDNGuildWarManager::SetGuildWarMissionGuildRankingInfo(TAGetGuildWarPointGuildPartTotal* pGetGuildWarPointGuildPartTotal)
{
	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; ++i)
	{
		if( pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].nPoint > 0 )
		{
			memcpy(m_sGuildWarMissionGuildRankingInfo[i].wszGuildName, pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].wszGuildName
				, sizeof(m_sGuildWarMissionGuildRankingInfo[i].wszGuildName));
			memcpy(m_sGuildWarMissionGuildRankingInfo[i].wszGuildMasterName, pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].wszCharName
				, sizeof(m_sGuildWarMissionGuildRankingInfo[i].wszGuildMasterName));
			m_sGuildWarMissionGuildRankingInfo[i].nTotalPoint = pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].nPoint;
		}
	}
}
// Set individual ranking by category
void CDNGuildWarManager::SetGuildWarMissionRankingInfo(TAGetGuildWarPointPartTotal* pGetGuildWarPointPartTotal)
{
	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; ++i)
	{
		if( pGetGuildWarPointPartTotal->GuildWarPointPartTotal[i].nPoint > 0 )
		{
			memcpy(m_sGuildWarMissionRankingInfo[i].wszGuildName, pGetGuildWarPointPartTotal->GuildWarPointPartTotal[i].wszGuildName
				, sizeof(m_sGuildWarMissionGuildRankingInfo[i].wszGuildName));
			memcpy(m_sGuildWarMissionRankingInfo[i].wszCharName, pGetGuildWarPointPartTotal->GuildWarPointPartTotal[i].wszCharName
				, sizeof(m_sGuildWarMissionGuildRankingInfo[i].wszGuildMasterName));
			m_sGuildWarMissionRankingInfo[i].nTotalPoint = pGetGuildWarPointPartTotal->GuildWarPointPartTotal[i].nPoint;
			m_nGuildWarMissionRankingCharacterDBID[i] = pGetGuildWarPointPartTotal->GuildWarPointPartTotal[i].biCharacterDBID;
		}
	}
}
// Awards by date
void CDNGuildWarManager::SetGuildDailyAward(TAGetGuildWarPointDaily* pGetGuildWarPointDaily)
{
	for( int i=0; i<GUILDWAR_DAILY_AWARD_MAX; ++i)
	{
		memcpy(m_sGuildWarDailyAward[i].wszCharName, pGetGuildWarPointDaily->sGuildWarPointDaily[i].wszCharName, sizeof(m_sGuildWarDailyAward[i].wszCharName));
		m_sGuildWarDailyAward[i].nGuildUID = TGuildUID(g_Config.nWorldSetID, pGetGuildWarPointDaily->sGuildWarPointDaily[i].nGuildDBID);
		m_nGuildWarDailyAwardCharacterDBID[i] = pGetGuildWarPointDaily->sGuildWarPointDaily[i].biCharacterDBID;
	}
}

void CDNGuildWarManager::SetGuildWarStep(char cStepIndex, short wScheduleID, short wWinersWeightRate )
{
	m_cStepIndex = cStepIndex;
	m_wScheduleID = wScheduleID;
	m_wWinersWeightRate = wWinersWeightRate;	
}

void CDNGuildWarManager::SetGuildTournamentInfo(SGuildTournamentInfo* pGuildTournamentInfo)
{
	memcpy(m_sGuildWarFinalInfo, pGuildTournamentInfo, sizeof(m_sGuildWarFinalInfo));	
	g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SetGuildTournamentInfo !!\r\n");
}
void CDNGuildWarManager::SetGuildTournamentResult(MAVISetGuildWarFinalResult* pGuildTournamentResult)
{
	if( pGuildTournamentResult->cWinGuildIndex >= 0 && pGuildTournamentResult->cWinGuildIndex < GUILDWAR_FINALS_TEAM_MAX)
	{
		m_sGuildWarFinalInfo[pGuildTournamentResult->cWinGuildIndex].bWin = true;
		m_sGuildWarFinalInfo[pGuildTournamentResult->cWinGuildIndex].cMatchTypeCode = pGuildTournamentResult->cMatchTypeCode;
	}

	if( pGuildTournamentResult->cLoseGuildIndex >= 0 && pGuildTournamentResult->cLoseGuildIndex < GUILDWAR_FINALS_TEAM_MAX)
	{
		m_sGuildWarFinalInfo[pGuildTournamentResult->cLoseGuildIndex].bWin = false;
		m_sGuildWarFinalInfo[pGuildTournamentResult->cLoseGuildIndex].cMatchTypeCode = pGuildTournamentResult->cMatchTypeCode;
	}	
}

void CDNGuildWarManager::SetGuildWarTrialRankingInfo(MASetGuildWarPointRunningTotal* pSetGuildWarPointRunningTotal )
{
	m_nGuildWarTrialRankingCount = 0;
	memset(m_sGuildWarTrialRankingInfo, 0, sizeof(m_sGuildWarTrialRankingInfo));

	for( int i=0; i<GUILDWAR_TRIAL_POINT_TEAM_MAX; ++i )
	{
		if( pSetGuildWarPointRunningTotal->sGuildWarPointRunningTotal[i].wRanking == 0)
			break;
		memcpy(&m_sGuildWarTrialRankingInfo[i], &pSetGuildWarPointRunningTotal->sGuildWarPointRunningTotal[i], sizeof(SGuildWarRankingInfo));
		++m_nGuildWarTrialRankingCount;
	}
}

void CDNGuildWarManager::SetGuildWarFinalPart(char cGuildFinalPart, __time64_t tFinalPartBeginTime)
{
	m_cGuildWarFinalPart = cGuildFinalPart;
	m_tGuildWarFinalPartBeginTime = tFinalPartBeginTime;
}

bool CDNGuildWarManager::IsGuildWarFinal(TGuildUID nGuildUID)
{
	for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; ++i )
	{
		if( m_sGuildWarFinalInfo[i].GuildUID == nGuildUID )
			return true;
	}
	return false;
}

int CDNGuildWarManager::CalcGuildWarFestivalPoint(char cTeamType, INT64 nChracterDBID, int nGuildWarPoint)
{	
	//Mission points earned by character + (mission points earned by character * total special awards won * 0.1), x2 if guild preliminaries won
	int nAwardCount = GetGuildAwardCount(nChracterDBID);
	bool bWinTeam = IsGuildWarTrialWIn(cTeamType);
	if( bWinTeam )
		return (int)((nGuildWarPoint + (nGuildWarPoint*nAwardCount*0.1f))*m_wWinersWeightRate);
	return (int)(nGuildWarPoint + (nGuildWarPoint*nAwardCount*0.1f));	
}

bool CDNGuildWarManager::IsGuildWarTrialWIn(char cTeamType)
{
	bool bWinTeam = false;	
	if( cTeamType == GUILDWAR_TEAM_BLUE)	
		bWinTeam = m_nBlueTeamPoint > m_nRedTeamPoint ? true : false;
	else
		bWinTeam = m_nRedTeamPoint > m_nBlueTeamPoint ? true : false;
	return bWinTeam;
}

int CDNGuildWarManager::GetGuildAwardCount(INT64 nChracterDBID)
{
	int nAwardCount = 0;	
	// By category
	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; ++i )
	{
		if( m_nGuildWarMissionRankingCharacterDBID[i] == nChracterDBID )
			++nAwardCount;
	}
	for( int i=0; i<GUILDWAR_DAILY_AWARD_MAX; ++i )
	{
		if( m_nGuildWarDailyAwardCharacterDBID[i] == nChracterDBID )
			++nAwardCount;
	}
	return nAwardCount;
}

void CDNGuildWarManager::SetGuildSecretMission(MASetGuildWarSecretMission* pSecret)
{
	m_cSecretTeam = pSecret->cTeamCode;
	m_nSecretRandomSeed = pSecret->nRandomSeed;
	if( m_nSecretRandomSeed == -1 ) // Clear / Release
	{
		memset (m_vSecretMissionID, 0x00, sizeof(m_vSecretMissionID));
		return;
	}

	g_pDataManager->GetRandomGuildMission(m_nSecretRandomSeed, m_vSecretMissionID);
}

bool CDNGuildWarManager::IsSecretMission (BYTE cTeamCode, int nMissionID)
{
	if (m_cSecretTeam != cTeamCode)
		return false;

	for (int i=0; i<GUILDWARMISSION_MAXGROUP; i++)
	{
		if (m_vSecretMissionID[i] == nMissionID)
			return true;
	}

	return false;
}

void CDNGuildWarManager::CalcTeamSecret()
{
	if (m_nBlueTeamPoint == m_nRedTeamPoint)
	{
		m_cSecretTeam = 0;
		return;
	}

	float fSecretRate = 0.0f;
	if (m_nBlueTeamPoint > m_nRedTeamPoint)
		fSecretRate = (float)m_nRedTeamPoint / (float)m_nBlueTeamPoint;
	else
		fSecretRate = (float)m_nBlueTeamPoint / (float)m_nRedTeamPoint;
	
	if (fSecretRate <= 0)
	{
		DN_ASSERT( false, "fSecretRate <= 0" );
		return;
	}

	fSecretRate = 1.0f - fSecretRate;

	// Determine whether to stop secret mode in case master server cannot judge secret status.
	if (fSecretRate < 0.02f)		
		m_cSecretTeam = 0;			// Clear / Release
}

void CDNGuildWarManager::OnRecvMaGuildChangeMark( MAGuildChangeMark* pPacket )
{
	for( UINT i=0 ; i<_countof(m_sGuildWarFinalInfo) ; ++i )
	{
		if( m_sGuildWarFinalInfo[i].GuildUID.IsSet() == false )
			continue;
		if( m_sGuildWarFinalInfo[i].GuildUID != pPacket->GuildUID )
			continue;
	
		m_sGuildWarFinalInfo[i].wGuildMark			= pPacket->wMark;
		m_sGuildWarFinalInfo[i].wGuildMarkBG		= pPacket->wMarkBG;
		m_sGuildWarFinalInfo[i].wGuildMarkBorder	= pPacket->wMarkBorder;
		break;
	}
}

// Change guild name
void CDNGuildWarManager::OnRecvMaGuildChangeName( MAGuildChangeName* pPacket )
{
	for( UINT i=0 ; i<_countof(m_sGuildWarFinalInfo) ; ++i )
	{
		if( m_sGuildWarFinalInfo[i].GuildUID.IsSet() == false )
			continue;
		if( m_sGuildWarFinalInfo[i].GuildUID != pPacket->GuildUID )
			continue;

		memcpy(m_sGuildWarFinalInfo[i].wszGuildName, pPacket->wszGuildName, sizeof(m_sGuildWarFinalInfo[i].wszGuildName));		
		// Should we re-pull stats in this case?
		// Get preliminary statistics info..
		// Order: QueryGetGuildWarPointPartTotal, QueryGetGuildWarPointGuildPartTotal, QueryGetGuildWarPointDaily, QueryGetGuildWarPointGuildTotal
		if ( g_pDBConnectionManager )
		{
			BYTE cThreadID;
			CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
			if( pDBCon )
			{
				pDBCon->QueryGetGuildWarPointPartTotal(cThreadID, g_Config.nWorldSetID, 0, 'A', 0);
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] ChangeGuildName and retry stats !!\r\n");
			}
		}
		break;
	}
}
