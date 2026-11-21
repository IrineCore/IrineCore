#include "StdAfx.h"
#include "DnGuildWarTask.h"
#include "DnInterface.h"
#include "DnGuildWarRoomDlg.h"
#include "DnGuildTask.h"
#include "DnTableDB.h"
#include "IDnSkillUsableChecker.h"
#include "IDnSkillProcessor.h"
#include "DnUIString.h"
#include "DnSkillTreeDlg.h"
#include "DnMainMenuDlg.h"
#include "DnSkillTask.h"
#include "DnGuildWarSituationMng.h"
#include "DnPvPGameTask.h"
#include "TaskManager.h"
#include "DnMutatorGuildWar.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"
#include "DnGuildWarFinalTournamentListDlg.h"
#include "SyncTimer.h"
#include "DnInterfaceString.h"
#include "DnOccupationTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnGuildWarTask::CDnGuildWarTask()
: CTaskListener( false )
, m_nGuildWarState( PvPCommon::OccupationSystemState::None )
, m_nGuildWarSkillUsedPoint( 0 )
, m_nGuildWarSkillLeavePoint( MAX_GUILD_WAR_SKILL_POINT )
, m_biGuildWarFestivalPoint( 0 )
, m_CurGuildWarFinalRoundStep( GUILDWAR_FINALPART_NONE )
, m_CurGuildWarStep( GUILDWAR_STEP_NONE )
, m_bGuildWarFinalProgress( false )
, m_bSetForceUpdateAlarm( false )
{
}

CDnGuildWarTask::~CDnGuildWarTask()
{
	m_vecPreliminaryResultGuildRanking.clear();
	m_vecEventGuildRankInfo.clear();
	m_vecEventPersonalRankInfo.clear();
	m_vecGuildWarShopItemInfo.clear();
}

bool CDnGuildWarTask::Initialize()
{
#if defined(PRE_FIX_NEXTSKILLINFO)
	InitSkillLevelTableIDList();
#endif // PRE_FIX_NEXTSKILLINFO
	return true;
}

void CDnGuildWarTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	if( nMainCmd == SC_GUILD )
	{
		switch( nSubCmd )
		{
			// Receive data from server and open the window
			case eGuild::SC_GUILDWAR_TRIAL_RESULT_UIOPEN: OnRecvGuildWarTrialResultOpenMsg(); break;
			case eGuild::SC_GUILDWAR_TOURNAMENT_INFO_UIOPEN: OnRecvGuildWarTournamentOpenMsg(); break;
			case eGuild::SC_GUILDWAR_STATUS_OPEN: OnRecvGuildWarStatusOpenMsg( (SCGuildWarStatus*)pData ); break;
			case eGuild::SC_GUILDWAR_TRIAL_RESULT_OPEN: OnRecvGuildWarTrialResultInfoMsg( (SCGuildWarTrialResult*)pData ); break;
			case eGuild::SC_GUILDWAR_TOURNAMENT_INFO_OPEN: OnRecvGuildWarTournamentInfoMsg( (SCGuildTournamentInfo*)pData ); break;
			case eGuild::SC_GUILDWAR_FESTIVAL_POINT: OnRecvGuildWarFestivalPoint( (SCGuildWarFestivalPoint*)pData ); break;
			case eGuild::SC_GUILDWAR_COMPENSATION: OnRecvGuildWarGetReward( (SCGuildWarCompensation*)pData ); break;
			case eGuild::SC_GUILDWAR_VOTE: OnRecvGuildWarVote( (SCGuildWarVote*)pData ); break;
			case eGuild::SC_GUILDWAR_VOTE_TOP: OnRecvGuildWarVoteTop( (SCGuildWarVoteTop*)pData ); break;
			case eGuild::SC_GUILDWAR_EVENT_TIME: OnRecvGuildWarEventTime( (SCGuildWarEventTime*)pData ); break;
			case eGuild::SC_GUILDWAR_BUYED_ITEM_COUNT_LIST: OnRecvGuildWarShopItemInfoList( (SCBuyedItemCount*)pData ); break;
			case eGuild::SC_GUILDWAR_BUYED_ITEM_COUNT: OnRecvGuildWarShopItemInfo( (SCBuyedItemCount*)pData ); break;
		}
	}
	else if( nMainCmd == SC_PVP )
	{
		switch (nSubCmd)
		{
			case ePvP::SC_PVP_MEMBERINDEX:
				{
					SCPvPMemberIndex * pPacket = (SCPvPMemberIndex*)pData;

					if (pPacket->nRetCode == ERROR_NONE)
					{
						if( GetInterface().GetGuildWarRoomDlg() )
							GetInterface().GetGuildWarRoomDlg()->RefreshSlot( pPacket );
						else if( GetInterface().GetGuildWarSituation() )
							GetInterface().GetGuildWarSituation()->MoveUser( pPacket );
					}
					else
					{
						if( GetInterface().GetGuildWarRoomDlg() )
							GetInterface().GetGuildWarRoomDlg()->RevertSlot();

						GetInterface().ServerMessageBox(pPacket->nRetCode);		//Output message if abnormal. May differ mid-game, consult designer (e.g. chat message handling mid-game)
					}
					break;
				}

			case ePvP::SC_PVP_MEMBERGRADE:
				{
					SCPvPMemberGrade * pPacket = (SCPvPMemberGrade*)pData;

					if (pPacket->nRetCode == ERROR_NONE)
					{
						CDnGuildWarSituationMng * pMng = GetInterface().GetGuildWarSituation();
						if( pMng )
							pMng->SetUserState( pPacket->nSessionID, pPacket->uiUserState );
					}
					else if( pPacket->nRetCode == ERROR_PVP_GUILDWAR_SECONDARYMASTER_ASIGNFAIL )
						GetInterface().MessageBox( 126222 );	// Up to 3 sub-masters can be appointed.
					else
						GetInterface().ServerMessageBox(pPacket->nRetCode);
					break;
				}
			case ePvP::SC_PVP_OCCUPATION_MODESTATE:		OnRecvGuildWarModeState( (SCPvPOccupationModeState*)pData );	break;
		}
	}
}

void CDnGuildWarTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if (m_GuildWarEventTime.empty() == false)
	{
		const __time64_t tCurTime = CSyncTimer::GetInstance().GetCurTime();

		bool bGWStepProcessd = false;
		std::map<eGuildWarStepType, SGuildWarEventTime>::const_iterator iter = m_GuildWarEventTime.begin();
		for (; iter != m_GuildWarEventTime.end(); ++iter)
		{
			const SGuildWarEventTime& curTime = (*iter).second;
			if (curTime.term.tBeginTime <= tCurTime && curTime.term.tEndTime > tCurTime)
			{
				const eGuildWarStepType& step = (*iter).first;
				if (m_CurGuildWarStep != step || m_bSetForceUpdateAlarm)
				{
					m_CurGuildWarStep = step;
					if (m_CurGuildWarStep == GUILDWAR_STEP_REWARD)
					{
						std::map<eGuildFinalPart, SGuildWarEventTime>::const_iterator roundIter = m_GuildWarFinalRoundTime.begin();
						for (; roundIter != m_GuildWarFinalRoundTime.end(); ++roundIter)
						{
							const SGuildWarEventTime& curRoundTime = (*roundIter).second;
							if (curRoundTime.term.tBeginTime <= tCurTime && curRoundTime.term.tEndTime > tCurTime)
							{
								const eGuildFinalPart& curRoundStep = (*roundIter).first;
								if (curRoundStep != m_CurGuildWarFinalRoundStep)
								{
									m_CurGuildWarFinalRoundStep = curRoundStep;
									break;
								}
							}
						}
					}

					bool bShow = (m_CurGuildWarStep != GUILDWAR_STEP_NONE && m_CurGuildWarStep != GUILDWAR_STEP_END);
					GetInterface().RefreshGuildWarMark(bShow, true);

					m_bSetForceUpdateAlarm = false;
				}

				bGWStepProcessd = true;
				break;
			}
		}

		if (bGWStepProcessd == false && m_CurGuildWarStep != GUILDWAR_STEP_NONE)
		{
			m_CurGuildWarStep = GUILDWAR_STEP_NONE;
			m_CurGuildWarFinalRoundStep = GUILDWAR_FINALPART_NONE;

			GetInterface().RefreshGuildWarMark(false, false);
		}
	}
}

void CDnGuildWarTask::GetGuildWarEventTime(std::wstring& startTime, std::wstring& endTime, eGuildWarStepType type) const
{
	startTime.clear();
	endTime.clear();

	std::map<eGuildWarStepType, SGuildWarEventTime>::const_iterator iter = m_GuildWarEventTime.find(type);
	if (iter != m_GuildWarEventTime.end())
	{
		const SGuildWarEventTime& eventTime = (*iter).second;
		startTime	= eventTime.timeString[GUILDWAR_EVENT_START];
		endTime		= eventTime.timeString[GUILDWAR_EVENT_END];
	}
}

void CDnGuildWarTask::GetGuildWarFinalRoundTime(std::wstring& startTime, std::wstring& endTime, eGuildFinalPart step) const
{
	startTime.clear();
	endTime.clear();

	std::map<eGuildFinalPart, SGuildWarEventTime>::const_iterator iter = m_GuildWarFinalRoundTime.find(step);
	if (iter != m_GuildWarFinalRoundTime.end())
	{
		const SGuildWarEventTime& eventTime = (*iter).second;
		startTime	= eventTime.timeString[GUILDWAR_EVENT_START];
		endTime		= eventTime.timeString[GUILDWAR_EVENT_END];
	}
}

void CDnGuildWarTask::UseGuildWarSkillPoint()
{
	m_nGuildWarSkillUsedPoint++;
	m_nGuildWarSkillLeavePoint--;

	if( m_nGuildWarSkillLeavePoint < 0 )
		m_nGuildWarSkillLeavePoint = 0;

	CDnSkillTreeDlg* pSkillTreeDlg = static_cast<CDnSkillTreeDlg*>( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::SKILL_DIALOG ) );
	if( pSkillTreeDlg && pSkillTreeDlg->IsShow() )
		pSkillTreeDlg->UpdateGuildWarSkillPointInfo();
}

void CDnGuildWarTask::OnRecvGuildWarTrialResultOpenMsg()
{
	GetInterface().OpenGuildWarPreliminaryResult();
}

void CDnGuildWarTask::OnRecvGuildWarTournamentOpenMsg()
{
	GetInterface().OpenGuildWarFinalTournamentList();
}

void CDnGuildWarTask::OnRecvGuildWarStatusOpenMsg( SCGuildWarStatus* pPacket )
{
	_ASSERT( pPacket );

	// Clear Data
	m_stPreliminaryStatus.nBluePoint = 0;
	m_stPreliminaryStatus.nRedPoint = 0;
	m_stPreliminaryStatus.nMyPoint = 0;
	m_stPreliminaryStatus.nMyGuildPoint = 0;

	m_stPreliminaryStatus.nBluePoint = pPacket->nBluePoint;
	m_stPreliminaryStatus.nRedPoint = pPacket->nRedPoint;
	m_stPreliminaryStatus.nMyPoint = pPacket->nMyPoint;
	m_stPreliminaryStatus.nMyGuildPoint = pPacket->nMyGuildPoint;

	m_vecPreliminaryResultGuildRanking.clear();
	for( int i=0; i<pPacket->nRankingCount; i++ )
	{
		stJoinGuildRankInfo stGuildInfo;
		stGuildInfo.m_nGuildRank = pPacket->sGuildWarPointTrialRanking[i].wRanking;
		stGuildInfo.m_nGuildUserCount = pPacket->sGuildWarPointTrialRanking[i].wCurGuildUserCount;
		stGuildInfo.m_nGuildUserMax = pPacket->sGuildWarPointTrialRanking[i].wMaxGuildUserCount;
		stGuildInfo.m_nTotalPoints = pPacket->sGuildWarPointTrialRanking[i].nTotalPoint;
		stGuildInfo.m_strGuildMasterName = pPacket->sGuildWarPointTrialRanking[i].wszGuildMasterName;
		stGuildInfo.m_strGuildName = pPacket->sGuildWarPointTrialRanking[i].wszGuildName;

		m_vecPreliminaryResultGuildRanking.push_back( stGuildInfo );
	}

	GetInterface().SetGuildWarPreliminaryResult();
}

void CDnGuildWarTask::OnRecvGuildWarTrialResultInfoMsg( SCGuildWarTrialResult* pPacket )
{
	_ASSERT( pPacket );

	if( !CDnGuildTask::IsActive() ) return;
	TGuild* pGuild = GetGuildTask().GetGuildInfo();

	tstring strMyGuildName;
	if( pGuild && pGuild->IsSet() )
		strMyGuildName = pGuild->GuildView.wszGuildName;

	// Clear Data
	m_vecPreliminaryResultGuildRanking.clear();
	m_vecEventGuildRankInfo.clear();
	m_vecEventPersonalRankInfo.clear();
	m_vecDailyBestChar.clear();

	m_stPreliminaryResult.nBlueFinalPoint = 0;
	m_stPreliminaryResult.nRedFinalPoint = 0;
	m_stPreliminaryResult.bJoinGuildWar = 0;
	m_stPreliminaryResult.bReward = false;
	m_stPreliminaryResult.bMyWin = false;

	m_stPreliminaryResult.nBlueFinalPoint = pPacket->nBluePoint;
	m_stPreliminaryResult.nRedFinalPoint = pPacket->nRedPoint;
	m_stPreliminaryResult.bJoinGuildWar = pPacket->bGuildWar;

	stJoinGuildRankInfo stGuildInfo;

	if( pGuild && pGuild->IsSet() && m_stPreliminaryResult.bJoinGuildWar )	 // Only participating guilds have my guild info
	{
		int nGuildUserCount = 0;
		int nGuildUserMax = 0;
		nGuildUserMax = pGuild->wGuildSize;
		nGuildUserCount = GetGuildTask().GetGuildMemberCount();

		m_stPreliminaryResult.bReward = pPacket->bReward;
		if( ( pGuild->GuildView.cTeamColorCode == 1 && pPacket->nBluePoint > pPacket->nRedPoint )
			|| ( pGuild->GuildView.cTeamColorCode == 2 && pPacket->nBluePoint < pPacket->nRedPoint ) )
			m_stPreliminaryResult.bMyWin = true;
		else
			m_stPreliminaryResult.bMyWin = false;

		stGuildInfo.m_nGuildRank = pPacket->sMyGuildWarRankingInfo.wRanking;
		stGuildInfo.m_nGuildUserCount = nGuildUserCount;
		stGuildInfo.m_nGuildUserMax = nGuildUserMax;
		stGuildInfo.m_nTotalPoints = pPacket->sMyGuildWarRankingInfo.nTotalPoint;
		stGuildInfo.m_strGuildMasterName = GetGuildTask().GetGuildMasterName();
		stGuildInfo.m_strGuildName = strMyGuildName;

		m_stPreliminaryResult.nFestivalPoint = pPacket->nFestivalPoint;
		m_stPreliminaryResult.nFestivalPointAdd = pPacket->nFestivalPointAdd;
	}

	m_vecPreliminaryResultGuildRanking.push_back( stGuildInfo );

	for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; i++ )
	{
		stGuildInfo.m_nGuildRank = pPacket->sGuildWarRankingInfo[i].wRanking;
		stGuildInfo.m_nGuildUserCount = pPacket->sGuildWarRankingInfo[i].wCurGuildUserCount;
		stGuildInfo.m_nGuildUserMax = pPacket->sGuildWarRankingInfo[i].wMaxGuildUserCount;
		stGuildInfo.m_nTotalPoints = pPacket->sGuildWarRankingInfo[i].nTotalPoint;
		stGuildInfo.m_strGuildMasterName = pPacket->sGuildWarRankingInfo[i].wszGuildMasterName;
		stGuildInfo.m_strGuildName = pPacket->sGuildWarRankingInfo[i].wszGuildName;

		m_vecPreliminaryResultGuildRanking.push_back( stGuildInfo );
	}

	stEventRankInfo stEventInfo;

	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; i++ )
	{
		stEventInfo.m_nRank = 1;
		stEventInfo.m_nTotalPoints = pPacket->sGuildWarMissionGuildRankingInfo[i].nTotalPoint;
		stEventInfo.m_strName1 = pPacket->sGuildWarMissionGuildRankingInfo[i].wszGuildName;
		stEventInfo.m_strName2 = pPacket->sGuildWarMissionGuildRankingInfo[i].wszGuildMasterName;

		m_vecEventGuildRankInfo.push_back( stEventInfo );
	}

	if( m_stPreliminaryResult.bJoinGuildWar )
	{
		for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; i++ )
		{
			stEventInfo.m_nRank = pPacket->sMyGuildWarMissionGuildRankingInfo[i].wRanking;
			stEventInfo.m_nTotalPoints = pPacket->sMyGuildWarMissionGuildRankingInfo[i].nTotalPoint;
			stEventInfo.m_strName1 = strMyGuildName;
			stEventInfo.m_strName2 = GetGuildTask().GetGuildMasterName();

			m_vecEventGuildRankInfo.push_back( stEventInfo );
		}
	}

	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; i++ )
	{
		stEventInfo.m_nRank = 1;
		stEventInfo.m_nTotalPoints = pPacket->sGuildWarMissionRankingInfo[i].nTotalPoint;
		stEventInfo.m_strName1 = pPacket->sGuildWarMissionRankingInfo[i].wszCharName;
		stEventInfo.m_strName2 = pPacket->sGuildWarMissionRankingInfo[i].wszGuildName;

		m_vecEventPersonalRankInfo.push_back( stEventInfo );
	}

	if( m_stPreliminaryResult.bJoinGuildWar )
	{
		for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; i++ )
		{
			stEventInfo.m_nRank = pPacket->sMyGuildWarMissionRaningInfo[i].wRanking;
			stEventInfo.m_nTotalPoints = pPacket->sMyGuildWarMissionRaningInfo[i].nTotalPoint;
			stEventInfo.m_strName1 = CDnActor::s_hLocalActor->GetName();
			stEventInfo.m_strName2 = strMyGuildName;

			m_vecEventPersonalRankInfo.push_back( stEventInfo );
		}
	}

	for( int i=0; i<GUILDWAR_DAILY_AWARD_MAX; i++ )
		m_vecDailyBestChar.push_back( pPacket->sGuildWarDailyAward[i].wszCharName );	
	
	GetInterface().SetGuildWarPreliminaryResult();
}

void CDnGuildWarTask::OnRecvGuildWarTournamentInfoMsg( SCGuildTournamentInfo* pPacket )
{
	_ASSERT( pPacket );

	// Clear Data
	for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; i++ )
	{
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildUID.Reset();
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_strGuildName = L"";
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildSelfView.Reset();
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_bWin = false;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_cMatchTypeCode = 0;
	}

	m_stGuildTournamentInfo.m_cMatchTypeCode = 0;
	m_stGuildTournamentInfo.m_tStartTime = 0;
	m_stGuildTournamentInfo.m_bPopularityVote = false;


	for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; i++ )
	{
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildUID = pPacket->TournamentInfo[i].GuildUID;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_strGuildName = pPacket->TournamentInfo[i].wszGuildName;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildSelfView.GuildUID = pPacket->TournamentInfo[i].GuildUID;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildSelfView.wGuildMark = pPacket->TournamentInfo[i].wGuildMark;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildSelfView.wGuildMarkBG = pPacket->TournamentInfo[i].wGuildMarkBG;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_tGuildSelfView.wGuildMarkBorder = pPacket->TournamentInfo[i].wGuildMarkBorder;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_bWin = pPacket->TournamentInfo[i].bWin;
		m_stGuildTournamentInfo.m_TournamentGuild[i].m_cMatchTypeCode = pPacket->TournamentInfo[i].cMatchTypeCode;
	}

	m_stGuildTournamentInfo.m_cMatchTypeCode = pPacket->cMatchTypeCode;
	m_stGuildTournamentInfo.m_tStartTime = pPacket->tStartTime;
	m_stGuildTournamentInfo.m_bPopularityVote = pPacket->bPopularityVote;

	GetInterface().SetGuildWarFinalTournamentList();
}

void CDnGuildWarTask::OnRecvGuildWarModeState( SCPvPOccupationModeState* pPacket )
{
	_ASSERT( pPacket );

	m_nGuildWarState = pPacket->nState;
	if( pPacket->nState == PvPCommon::OccupationSystemState::Wait )	// Reset skill points on initial entry
	{
		ResetGuildWarSkillPoint();
	}
	else if( pPacket->nState == PvPCommon::OccupationSystemState::End )
	{
		ResetGuildWarSkillPoint();
	}
}

void CDnGuildWarTask::OnRecvGuildWarShopItemInfoList( SCBuyedItemCount* pPacket )
{
	_ASSERT( pPacket );
	m_vecGuildWarShopItemInfo.clear();

	for( int i=0; i<pPacket->count; i++ )
	{
		stGuildWarShopItemInfo sGuildWarShopItemInfo;
		sGuildWarShopItemInfo.m_nItemID = pPacket->items[i].id;
		sGuildWarShopItemInfo.m_nCurrentBuyCount = pPacket->items[i].count;

		m_vecGuildWarShopItemInfo.push_back( sGuildWarShopItemInfo );
	}
}

void CDnGuildWarTask::OnRecvGuildWarShopItemInfo( SCBuyedItemCount* pPacket )
{
	_ASSERT( pPacket );

	for( int i=0; i<pPacket->count; i++ )
	{
		bool bExistItem = false;
		for( int j=0; j<(int)m_vecGuildWarShopItemInfo.size(); j++ )
		{
			if( m_vecGuildWarShopItemInfo[j].m_nItemID == pPacket->items[i].id )
			{
				m_vecGuildWarShopItemInfo[j].m_nCurrentBuyCount = m_vecGuildWarShopItemInfo[j].m_nCurrentBuyCount + pPacket->items[i].count;
				bExistItem = true;
				break;
			}
		}

		if( !bExistItem )
		{
			stGuildWarShopItemInfo sGuildWarShopItemInfo;
			sGuildWarShopItemInfo.m_nItemID = pPacket->items[i].id;
			sGuildWarShopItemInfo.m_nCurrentBuyCount = pPacket->items[i].count;

			m_vecGuildWarShopItemInfo.push_back( sGuildWarShopItemInfo );			
		}
	}
}

int CDnGuildWarTask::GetGuildWarShopItemCurrentBuyCount( int nItemID )
{
	int nItemCurrentBuyCount = 0;
	for( int i=0; i<static_cast<int>( m_vecGuildWarShopItemInfo.size() ); i++ )
	{
		if( m_vecGuildWarShopItemInfo[i].m_nItemID == nItemID )
		{
			nItemCurrentBuyCount = m_vecGuildWarShopItemInfo[i].m_nCurrentBuyCount;
			break;
		}
	}

	return nItemCurrentBuyCount;
}

void CDnGuildWarTask::OnRecvGuildWarFestivalPoint( SCGuildWarFestivalPoint *pPacket )
{
	_ASSERT( pPacket );
	m_biGuildWarFestivalPoint = pPacket->biGuildWarFestivalPoint;
}

void CDnGuildWarTask::OnRecvGuildWarGetReward( SCGuildWarCompensation* pPacket )
{
	_ASSERT( pPacket );
	if( pPacket->nRetCode == ERROR_NONE )
	{
		GetInterface().OpenMovieAlarmDlg( NULL, 2.0f, 0, 0, 0, 126246 );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126246 ), false );		
	}
}

void CDnGuildWarTask::SendGuildWarVote( int nGuildIndex )
{
	ASSERT( nGuildIndex >= 0 && nGuildIndex < GUILDWAR_FINALS_TEAM_MAX );

	if( nGuildIndex < 0 || nGuildIndex >= GUILDWAR_FINALS_TEAM_MAX )
		return;

	CSGuildWarVote stGuildWarVote;
	stGuildWarVote.GuildUID = m_stGuildTournamentInfo.m_TournamentGuild[nGuildIndex].m_tGuildUID;

	CClientSessionManager::GetInstance().SendPacket( CS_GUILD, eGuild::CS_GUILDWAR_VOTE, (char*)&stGuildWarVote, int(sizeof(stGuildWarVote)) );
}

void CDnGuildWarTask::OnRecvGuildWarVote( SCGuildWarVote* pPacket )
{
	_ASSERT( pPacket );
	if( pPacket->nRetCode == ERROR_NONE )
	{
		m_stGuildTournamentInfo.m_bPopularityVote = false;
		CDnGuildWarFinalTournamentListDlg* pDnGuildWarFinalTournamentListDlg = GetInterface().GetGuildWarFinalTournamentList();
		if( pDnGuildWarFinalTournamentListDlg )
			pDnGuildWarFinalTournamentListDlg->SetTournamentUI();

		WCHAR wszTemp[1024] = {0, };
		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126249 ), pPacket->nAddFestivalPoint );
		GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", wszTemp, false );
	}
}

void CDnGuildWarTask::OnRecvGuildWarVoteTop( SCGuildWarVoteTop* pPacket )
{
	_ASSERT( pPacket );

	WCHAR wszTemp[1024] = {0, };
	if( pPacket->nVoteCount == 0 )
		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126248 ) );
	else
		swprintf_s( wszTemp, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126247 ), pPacket->wszGuildName, pPacket->nVoteCount );

	GetInterface().MessageBox( wszTemp );
}

void CDnGuildWarTask::OnRecvGuildWarEventTime( SCGuildWarEventTime* pPacket )
{
	if (pPacket == NULL)
	{
		_ASSERT(0);
		return;
	}

	m_GuildWarEventTime.clear();

	SGuildWarEventTime eventTime;
	eventTime.term = pPacket->tGuildWarTime[GUILDWAR_STEP_PREPARATION - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarEventTime.insert(std::make_pair(GUILDWAR_STEP_PREPARATION, eventTime));

	eventTime.term = pPacket->tGuildWarTime[GUILDWAR_STEP_TRIAL - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarEventTime.insert(std::make_pair(GUILDWAR_STEP_TRIAL, eventTime));

	eventTime.term = pPacket->tGuildWarTime[GUILDWAR_STEP_REWARD - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarEventTime.insert(std::make_pair(GUILDWAR_STEP_REWARD, eventTime));

	m_GuildWarFinalRoundTime.clear();

	eventTime.term = pPacket->tFinalPartTime[GUILDWAR_FINALPART_FINAL - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarFinalRoundTime.insert(std::make_pair(GUILDWAR_FINALPART_FINAL, eventTime));


	eventTime.term = pPacket->tFinalPartTime[GUILDWAR_FINALPART_4 - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarFinalRoundTime.insert(std::make_pair(GUILDWAR_FINALPART_4, eventTime));

	eventTime.term = pPacket->tFinalPartTime[GUILDWAR_FINALPART_8 - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarFinalRoundTime.insert(std::make_pair(GUILDWAR_FINALPART_8, eventTime));

	eventTime.term = pPacket->tFinalPartTime[GUILDWAR_FINALPART_16 - 1];
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_START], eventTime.term.tBeginTime);
	DN_INTERFACE::STRING::GetTimeText(eventTime.timeString[GUILDWAR_EVENT_END], eventTime.term.tEndTime);
	m_GuildWarFinalRoundTime.insert(std::make_pair(GUILDWAR_FINALPART_16, eventTime));

	m_CurGuildWarFinalRoundStep = GUILDWAR_FINALPART_NONE;
	m_CurGuildWarStep			= GUILDWAR_STEP_NONE;

	m_bGuildWarFinalProgress = pPacket->bFinalProgress;

}

//////////////////////////////////////////////////////////////////////////
// CDnGuildWarSkill
//////////////////////////////////////////////////////////////////////////

DnSkillHandle CDnGuildWarSkill::CreateSkill( DnActorHandle hActor, int iSkillTableID, int iLevel )
{
#if defined(_DEBUG)
	if( false == ValidateSkillData( iSkillTableID, iLevel ) )
		return DnSkillHandle();
#endif

	// If creation is requested with level 0, it is an unlocked but unacquired skill.
	bool bAcquired = (iLevel != 0);
	if( false == bAcquired )
		iLevel = 1;		// Cannot create with level 0, so set to 1

	CDnGuildWarSkill* pNewSkill = NULL;

	// Attach skill activation conditions and activation processors.
	// 5 each, 10 parameters in total. Subject to change.
	pNewSkill = new CDnGuildWarSkill( hActor );

	// Initialize 
	bool bResult = pNewSkill->Initialize( iSkillTableID, iLevel );
	if( false == bResult )
	{
		SAFE_DELETE( pNewSkill );
		return CDnSkill::Identity();
	}

	//pNewSkill->SetSkillLevelIDOffset( iSkillLevelTableIDOffset );

	if( bAcquired )
	{
		pNewSkill->SetHasActor( hActor );
		pNewSkill->Acquire();
	}

	return pNewSkill->GetMySmartPtr();
}

bool CDnGuildWarSkill::Initialize( int iSkillTableID, int iLevel )
{
	m_iSkillID = iSkillTableID;

	if( false == LoadSkillLevelData( iSkillTableID, iLevel ) )
		return false;

	// Info to collect separately after initialization.
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );

	// Set icon image
	SetIconImageIndex( pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_IconImageIndex" )->GetInteger() );

	// When resetting, reset based on PVE as default value.
	m_iSelectedSkillLevelDataApplyType = PVE;
	m_bEnableTooltip = ( pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillTip" )->GetInteger() ) ? true : false;

	return true;
}

bool CDnGuildWarSkill::LoadSkillLevelData( int iSkillTableID, int iLevel )
{
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );

	vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillTableID, vlSkillLevelList ) == 0 ) 
		return false;

	if ( vlSkillLevelList.empty() )
		return false;

	int iSkillLevelDataApplyType = PVP;
	// If there is no data for the target, simply do nothing and return.
	if( vlSkillLevelList.empty() )
		return true;

	int iSkillLevelTableIDOffset = vlSkillLevelList.front();
	this->SetSkillLevelIDOffset( iSkillLevelTableIDOffset, iSkillLevelDataApplyType );

	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == iLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	if( -1 == iSkillLevelTableID )
		return false;

#if defined(PRE_FIX_NEXTSKILLINFO)
	//NextLevel might not be contiguous.. So find and store the next level data table ID here.
	int iMinSkillLevelTableID = -1;
	int iNextSkillLevelTableID = -1;

	int nMinLevel = 9999;	//Set the max level value around this size for now..
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iTempLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iTempLevel == (iLevel + 1) )
			iNextSkillLevelTableID = vlSkillLevelList.at( i );

		//Set the lowest skill level info.
		if (nMinLevel > iTempLevel)
		{
			nMinLevel = iTempLevel;
			iMinSkillLevelTableID = vlSkillLevelList.at(i);
		}		
	}
#endif // PRE_FIX_NEXTSKILLINFO

	// Attach skill activation conditions and activation processors.
	// 5 each, 10 parameters in total. Subject to change.
	char caLabel[ 32 ];
	int iCheckerParamOffset = 0;
	int iProcessorParamOffset = 0;
	for( int i = 0; i < 5; ++i )
	{
		// Find the activation condition object name. If the parameter field is empty, creation functions return NULL
		sprintf_s( caLabel, "_UsableChecker%d", i + 1 );
		int iUsableChecker = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();

		sprintf_s( caLabel, "_Processor%d", i + 1 );
		int iProcessor = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLabel )->GetInteger();

		int iOffsetCheck = iCheckerParamOffset;
		IDnSkillUsableChecker* pUsableChecker = IDnSkillUsableChecker::Create( m_hActor, iUsableChecker, iSkillLevelTableID, &iCheckerParamOffset );

		if( NULL != pUsableChecker )
		{
			if( (iCheckerParamOffset - iOffsetCheck) != pUsableChecker->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameter. Check OutputDebug output!" );
			}

			this->AddUsableCheckers( pUsableChecker, iSkillLevelDataApplyType );
		}

		// Not used on the client. Just pass it as a local variable.
		set<string> setUseActionName;
		iOffsetCheck = iProcessorParamOffset;
		IDnSkillProcessor* pSkillProcessor = IDnSkillProcessor::Create( m_hActor, iProcessor, iSkillLevelTableID, &iProcessorParamOffset, setUseActionName );

		if( NULL != pSkillProcessor )
		{
			if( (iProcessorParamOffset - iOffsetCheck) != pSkillProcessor->GetNumArgument() )
			{
				OutputDebug( "[SkillLevelTable Error!] Invalid parameter count for %d.\n", iSkillLevelTableID );
				_ASSERT( !"Invalid SkillLevelTable parameter. Check OutputDebug output!" );
			}

			this->AddProcessor( pSkillProcessor, iSkillLevelDataApplyType );
		}
	}

	// skill table
	int iStringID = 0;
	iStringID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_NameID" )->GetInteger();
	m_strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, iStringID );
	m_strStaticName = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_StaticName" )->GetString();

	char caLable[ 64 ];
	ZeroMemory( caLable, sizeof(caLable) );
	for( int i = 0; i < 2; ++i )
	{
		sprintf_s( caLable,  "_NeedWeaponType%d", i+1 );
		int iEquipType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		if( -1 != iEquipType )
			m_aeNeedEquipType[ i ] = (CDnWeapon::EquipTypeEnum)iEquipType;
		else
			m_aeNeedEquipType[ i ] = CDnWeapon::EquipTypeEnum_Amount;
	}

	//m_iMaxLevel = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_MaxLevel" )->GetInteger();

	// Max level in the skill table cannot be trusted. -_-
	// Update with actual count. If already initialized with a value and it differs, assert!!
	if( 0 == m_iMaxLevel )
	{
		m_iMaxLevel = (int)vlSkillLevelList.size();
	}
	else
	{
		_ASSERT( m_iMaxLevel == (int)vlSkillLevelList.size() );
	}

	m_eSkillType = (SkillTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillType" )->GetInteger();
	m_eDurationType = (DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_DurationType" )->GetInteger();
	m_eTargetType = (TargetTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_TargetType" )->GetInteger();
	m_iBuffIconImageIndex = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_BuffIconImageIndex" )->GetInteger();

	m_iDissolvable = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Dissolvable" )->GetInteger();
	m_iDuplicateCount = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectAmassCount" )->GetInteger();
	m_iSkillDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_SkillDuplicate" )->GetInteger();
	m_iEffectDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_EffectDuplicate" )->GetInteger();
	m_iNeedJobClassID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_NeedJob" )->GetInteger();
	//m_eElement = (CDnState::ElementEnum)pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_Element" )->GetInteger();
	if( (CDnState::ElementEnum)-1 == m_eElement )
		m_eElement = CDnState::ElementEnum_Amount;

	m_iUnlockSkillBookItemID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_UnlockSkillBookItemID" )->GetInteger();
	m_iUnlockPrice = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_UnlockPrice" )->GetInteger();
	m_iExclusiveID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_DuplicatedSkillType" )->GetInteger();
	m_iBaseSkillID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_BaseSkillID" )->GetInteger();

	// skill level table
	m_iSkillLevelID = iSkillLevelTableID;
	m_iLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();

	if( m_iLevel < m_iMaxLevel )
#if defined(PRE_FIX_NEXTSKILLINFO)
		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iNextSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
#else
		m_iNextLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_NeedSkillPoint" )->GetInteger();
#endif // PRE_FIX_NEXTSKILLINFO

	else
		m_iNextLevelSkillPoint = 0;

#if defined(PRE_FIX_NEXTSKILLINFO)
	m_iFirstLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iMinSkillLevelTableID, "_NeedSkillPoint" )->GetInteger();
#else
	m_iFirstLevelSkillPoint = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID-(m_iLevel-1), "_NeedSkillPoint" )->GetInteger();
#endif // PRE_FIX_NEXTSKILLINFO

	m_iNeedItemID[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedItem" )->GetInteger();
	m_iNeedItemDecreaseCount[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_NeedItemDecreaseCount" )->GetInteger();
	m_iIncreaseRange[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_AddRange" )->GetInteger();
	m_iDecreaseHP[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DecreaseHP" )->GetInteger();
	m_iDecreaseMP[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DecreaseSP" )->GetInteger();
	m_iLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_LevelLimit" )->GetInteger();
	m_fDelayTime[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_DelayTime" )->GetInteger() / 1000.f;
	m_fOriginalDelayTime[ iSkillLevelDataApplyType ] = m_fDelayTime[ iSkillLevelDataApplyType ];
	m_iCPScore = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_CPScore" )->GetInteger();

#if defined(PRE_FIX_NEXTSKILLINFO)
	m_iFirstLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( iMinSkillLevelTableID, "_LevelLimit" )->GetInteger();
#else
	m_iFirstLevelLimit = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID-(m_iLevel-1), "_LevelLimit" )->GetInteger();
#endif // PRE_FIX_NEXTSKILLINFO


	m_fHPConsumeType[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_HPConsumeType" )->GetFloat();
	m_fMPConsumeType[ iSkillLevelDataApplyType ] = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SPConsumeType" )->GetFloat();

	// The base template of the skill description stays the same and only arguments change, so separate only the arguments by mode.
	m_iSkillExplainStringID = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillExplanationID" )->GetInteger();

	char *szParam = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillExplanationIDParam" )->GetString();
	MakeUIStringUseVariableParam( m_strSkillExplainString[ iSkillLevelDataApplyType ], m_iSkillExplainStringID, szParam );

	m_EffectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_StateEffectTableID" )->GetString();

	// Assuming level data for the same skill is contiguous in the skill level table.
	if( m_iLevel < m_iMaxLevel )
	{
#if defined(PRE_FIX_NEXTSKILLINFO)
		DNTableCell* pSoxField = pSkillLevelTable->GetFieldFromLablePtr( iNextSkillLevelTableID, "_SkillExplanationID" );
		if ( pSoxField )
		{
			m_iNextSkillExplainStringID = pSkillLevelTable->GetFieldFromLablePtr( iNextSkillLevelTableID, "_SkillExplanationID" )->GetInteger();

			szParam = pSkillLevelTable->GetFieldFromLablePtr( iNextSkillLevelTableID, "_SkillExplanationIDParam" )->GetString();
			MakeUIStringUseVariableParam( m_strNextSkillExplainString[iSkillLevelDataApplyType ], m_iNextSkillExplainStringID, szParam );
		}
		else
		{
			OutputDebug("Level data for the same skill in SkillLevelTable is not contiguous???\n");
			m_iNextSkillExplainStringID = -1;
		}
#else
		DNTableCell* pSoxField = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_SkillExplanationID" );
		if ( pSoxField )
		{
			m_iNextSkillExplainStringID = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_SkillExplanationID" )->GetInteger();

			szParam = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID+1, "_SkillExplanationIDParam" )->GetString();
			MakeUIStringUseVariableParam( m_strNextSkillExplainString[iSkillLevelDataApplyType ], m_iNextSkillExplainStringID, szParam );
		}
		else
		{
			OutputDebug("Level data for the same skill in SkillLevelTable is not contiguous???\n");
			m_iNextSkillExplainStringID = -1;
		}
#endif // PRE_FIX_NEXTSKILLINFO
	}
	else
		m_iNextSkillExplainStringID = -1;

	// Load state effect info
	StateEffectStruct StateEffect;
	for( int i = 0; i < 5; ++i ) 
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		int iApplyType = pSkillTable->GetFieldFromLablePtr( iSkillTableID, caLable )->GetInteger();
		bool bApplyAll = (StateEffectApplyType::ApplyAll == iApplyType);		// Apply-to-all. Add 2 identical state effects with different targets.

		if( bApplyAll )
			StateEffect.ApplyType = StateEffectApplyType::ApplySelf;
		else
			StateEffect.ApplyType = (StateEffectApplyType)iApplyType;

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetString();

		sprintf_s( caLable, "_EffectClassValue%dDuration", i + 1 );
		StateEffect.nDurationTime = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetInteger();

		m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );

		// If apply-to-all, switch to target and add the same one once more.
		if( bApplyAll )
		{
			StateEffect.ApplyType = StateEffectApplyType::ApplyTarget;
			m_vlStateEffectList[ iSkillLevelDataApplyType ].push_back( StateEffect );
		}
	}

	m_SkillInfo[ iSkillLevelDataApplyType ].iSkillID = m_iSkillID;
	m_SkillInfo[ iSkillLevelDataApplyType ].iSkillLevelID = m_iSkillLevelID;
	m_SkillInfo[ iSkillLevelDataApplyType ].eDurationType = m_eDurationType;
	m_SkillInfo[ iSkillLevelDataApplyType ].eSkillType = m_eSkillType;
	m_SkillInfo[ iSkillLevelDataApplyType ].iSkillBufIcon = pSkillTable->GetFieldFromLablePtr( iSkillTableID, "_BuffIconImageIndex" )->GetInteger();
	m_SkillInfo[ iSkillLevelDataApplyType ].effectOutputIDs = m_EffectOutputIDs;
	m_SkillInfo[ iSkillLevelDataApplyType ].iBuffIconImageIndex = m_iBuffIconImageIndex;
	m_SkillInfo[ iSkillLevelDataApplyType ].eTargetType = m_eTargetType;
	if( m_hActor )
		m_SkillInfo[ iSkillLevelDataApplyType ].iSkillUserTeam = m_hActor->GetTeam();

#if defined(PRE_FIX_NEXTSKILLINFO)
	m_SkillInfo[ iSkillLevelDataApplyType ].nSkillLevel = m_iLevel;
#endif // PRE_FIX_NEXTSKILLINFO

	UpdateRangeChecker( iSkillLevelDataApplyType );

	RefreshDecreaseMP( iSkillLevelDataApplyType );

	if( 0.0f == m_fHPConsumeType[ iSkillLevelDataApplyType ] )
	{
		m_iNeedHP[ iSkillLevelDataApplyType ] = m_iDecreaseHP[ iSkillLevelDataApplyType ];
	}
	else
	{
		if( m_hActor )
			m_iNeedHP[ iSkillLevelDataApplyType ] = int((float)m_hActor->GetMaxHP() * m_fHPConsumeType[ iSkillLevelDataApplyType ]);
	}

	return true;
}

MIInventoryItem::ItemConditionEnum CDnGuildWarSkill::GetItemCondition()
{
	MIInventoryItem::ItemConditionEnum eItemCondition =  ItemConditionEnum::Usable;

	if( m_hActor )
	{
		if( m_hActor->IsDie() ) 
			return ItemConditionEnum::Unusable;

		// Cannot use skills that the player has not acquired.
		if( !IsAcquired() )
			return ItemConditionEnum::Unusable;
	}

	if( CanExecuteGuildWarSkill() == CDnSkill::UsingResult::Failed )
		return ItemConditionEnum::Unusable;

	bool bCheckResource = false;
	if( CDnOccupationTask::IsActive() )
	{
		DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );
		if( pSkillLevelTable == NULL )
			return eItemCondition;
		int nItemID = pSkillLevelTable->GetItemIDFromField( "_SkillIndex", GetClassID() );
		int nNeedResource = pSkillLevelTable->GetFieldFromLablePtr( nItemID, "_DecreaseTIC" )->GetInteger();

		int nCurrentResource = GetOccupationTask().GetResource( (PvPCommon::Team::eTeam)CDnActor::s_hLocalActor->GetTeam() ); 
		if( nCurrentResource >= nNeedResource )	// Check if there are usable resources
			bCheckResource = true;
	}
	if( !bCheckResource )
		eItemCondition = ItemConditionEnum::Unusable;

	return eItemCondition;
}

CDnSkill::UsingResult CDnGuildWarSkill::CanExecuteGuildWarSkill()
{
	UsingResult eResult = UsingResult::Failed;

	if( !m_hActor )
		return UsingResult::Failed;

	// Player cannot use skills while in cannon mode.
	if( m_hActor->IsPlayerActor() )
	{
		if( static_cast<CDnPlayerActor*>(m_hActor.GetPointer())->IsCannonMode() )
			return UsingResult::Failed;
	}

	// Check remaining SP, HP, level, etc.
	// Aura or toggle skills can be turned off regardless of MP if currently active.
	if( !IsToggleOn() && !IsAuraOn() )
		if( m_hActor->GetSP() < m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ] )
			return UsingResult::Failed;

	// Skills like Warrior's Relieve must be usable even while asleep or stunned.
	// Considered adding an ignore-unattackable trigger type to the table, but it is currently only meaningful for Relieve, so...
	// Observe how it is used elsewhere before standardizing.
	// First, skip incapacitation check for skills with Dissolve state effect.
	// IgnorectCantAction must be enabled on the State signal of the skill's action for the skill action to trigger.
	bool bPassCheckCantAction = false;
	for( DWORD i = 0; i < GetStateEffectCount(); ++i )
	{
		CDnSkill::StateEffectStruct* pSE = GetStateEffectFromIndex( i );
		if( STATE_BLOW::BLOW_069 == pSE->nID )
		{
			bPassCheckCantAction = true;
			break;
		}
	}

	if( false == bPassCheckCantAction )
	{
		if( 0 < m_hActor->GetCantActionSEReferenceCount() )
			return UsingResult::Failed;
	}

	// Skills that consume MP cannot be used if there is a skill-disable state effect.
	if( 0 < m_hActor->GetCantUseSkillSEReferenceCount() )
		if( 0 < m_iNeedMP[ m_iSelectedSkillLevelDataApplyType ] )
			return UsingResult::Failed;

	// HP
	if( m_hActor->GetHP() < m_iNeedHP[ m_iSelectedSkillLevelDataApplyType ] )
		return UsingResult::Failed;

	if( m_hActor->GetLevel() < m_iLevelLimit )
		return UsingResult::Failed;

	// If a required weapon exists, check if equipped. Max 2. Skill can be used if either one is met.
	if( false == IsSatisfyWeapon() )
		return UsingResult::Failed;

	// Check job. 
	// In towns, only the recent job is set for users in sight rather than their full job history, so 
	// Do not check classes of other users encountered in village.
	// In dungeons, full party member history is received, so check all.
	bool bNeedCheckJob = true;
	if( CDnWorld::MapTypeVillage == CDnWorld::GetInstance().GetMapType() )
		if( NULL == dynamic_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer()) )
			bNeedCheckJob = false;

	if( bNeedCheckJob && m_hActor->GetClassID() <= CDnActor::Reserved6  )
	{
		CDnPlayerActor* pActor = dynamic_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( NULL == pActor )
			return UsingResult::Failed;
		if( 0 != m_iNeedJobClassID )
		{
			if( pActor->IsPassJob( m_iNeedJobClassID ) == false ) 
				return UsingResult::Failed;
		}

		// Not allowed if dead
		if( m_hActor->IsDie() ) 
			return UsingResult::Failed;
		// Cannot activate skill unless in battle mode!
		if( !pActor->IsBattleMode() )
		{
			if( !CDnSkillTask::IsActive() || !GetSkillTask().IsGuildWarSkill( GetClassID() ) )
				return UsingResult::Failed;
		}
	}

	// Cannot activate skill if cooldown has not finished
	switch( m_eDurationType )
	{
		case Instantly:
		case Buff:
		case Debuff:
		case SummonOnOff:
			{
				if( m_fCoolTime > 0.0f )
					return UsingResult::FailedByCooltime;
			}
			break;

		case TimeToggle: 
			break;

		case ActiveToggle: 
		case ActiveToggleForSummon:
			break;

		case Aura: 
			break;
	}

	if( m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].empty() )
		eResult = UsingResult::Success;
	else
	{
		int iNumChecker = (int)m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].size();

		for( int iChecker = 0; iChecker < iNumChecker; ++iChecker )
		{
			IDnSkillUsableChecker* pChecker = m_vlpUsableCheckers[ m_iSelectedSkillLevelDataApplyType ].at( iChecker );

			if( static_cast<CDnPlayerActor*>(m_hActor.GetPointer())->IsVehicleMode() && pChecker->GetType() == IDnSkillUsableChecker::GROUNDMOVABLE_CHECKER )
				eResult = UsingResult::Success;
			else
			{
				if( false == pChecker->CanUse() )
				{
					eResult = UsingResult::FailedByUsableChecker;
					break;
				}
				else
					eResult = UsingResult::Success;
			}
		}
	}

	return eResult;
}

void CDnGuildWarTask::ResetGuildWarSkillPoint()
{
	m_nGuildWarSkillUsedPoint = 0;
	m_nGuildWarSkillLeavePoint = MAX_GUILD_WAR_SKILL_POINT;
	GetSkillTask().LoadGuildWarSkillList( (PvPCommon::Team::eTeam)CDnActor::s_hLocalActor->GetTeam() );
}


#if defined(PRE_FIX_NEXTSKILLINFO)
SKILL_LEVEL_INFO CDnGuildWarTask::ms_SkillLevelTableInfo;
SKILL_LEVEL_INFO* CDnGuildWarTask::GetSkillLevelTableIDList(int nSkillID, int nApplyType)
{
	if (nApplyType >= 2 || nApplyType < 0)
		return &ms_SkillLevelTableInfo;

	SKILL_LEVEL_TABLEID_LIST::iterator findIter = m_SkillLevelTableIDList[nApplyType].find(nSkillID);
	if (findIter != m_SkillLevelTableIDList[nApplyType].end())
		return &findIter->second;

	return &ms_SkillLevelTableInfo;
}

void CDnGuildWarTask::InitSkillLevelTableIDList()
{
	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );

	if (pSkillTable == NULL || pSkillLevelTable == NULL)
		return;

	int nSkillCount = pSkillTable->GetItemCount();
	for (int i = 0; i < nSkillCount; ++i)
	{
		int nSkillID = pSkillTable->GetItemID(i);

		//int nSkillID = pSkillTable->GetFieldFromLablePtr(nItemID, "id")->GetInteger();

		std::vector<int> vlSkillLevelList;
		pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", nSkillID, vlSkillLevelList );

		// Check and filter whether the target is PvE or PvP.
		vector<int>::iterator iterLevelList = vlSkillLevelList.begin();
		for( iterLevelList; iterLevelList != vlSkillLevelList.end(); ++iterLevelList)
		{
			int iSkillLevelTableID = *iterLevelList;
			int iApplyType = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_ApplyType" )->GetInteger();
			int iSkillLevel = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, "_SkillLevel" )->GetInteger();

			AddSkillLevelTableID(nSkillID, iSkillLevel, iSkillLevelTableID, iApplyType);
		}
	}
}

void CDnGuildWarTask::AddSkillLevelTableID(int nSkillID, int nSkillLevel, int nSkillLevelTableID, int nApplyType)
{
	if (nApplyType >= 2 || nApplyType < 0)
		return;

	SKILL_LEVEL_TABLEID_LIST::iterator findIter = m_SkillLevelTableIDList[nApplyType].find(nSkillID);
	if (findIter != m_SkillLevelTableIDList[nApplyType].end())
	{
		//If an existing list exists, add to that list..
		findIter->second.AddSkillLevelTableID(nSkillLevel, nSkillLevelTableID);
	}
	else
	{
		SKILL_LEVEL_INFO skillLevelInfo;
		skillLevelInfo.AddSkillLevelTableID(nSkillLevel, nSkillLevelTableID);

		m_SkillLevelTableIDList[nApplyType].insert(std::make_pair(nSkillID, skillLevelInfo));
	}
}
#endif // PRE_FIX_NEXTSKILLINFO
