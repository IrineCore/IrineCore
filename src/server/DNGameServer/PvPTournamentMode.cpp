
#include "stdafx.h"
#include "PvPTournamentMode.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "PvPAllKillModeState.hpp"
#include "PvPScoreSystem.h"
#include "PvPAllKillScoreSystem.h"
#include "DNAggroSystem.h"
#include "DNMissionSystem.h"
#include "DNDBConnection.h"

#if defined(PRE_ADD_PVP_TOURNAMENT)
CPvPTournamentMode::CPvPTournamentMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket )
:CPvPAllKillMode( pGameRoom, pPvPGameModeTable, pPacket )
{	
	memset(m_sTournamentMatchInfo, 0, sizeof(m_sTournamentMatchInfo));	
	memset(m_cTournamentActorIndex, 0, sizeof(m_cTournamentActorIndex));
	m_cTournamentType = pPacket->cMaxUser;
	m_cTournamentArea = 2;
}

CPvPTournamentMode::~CPvPTournamentMode()
{
	
}

void CPvPTournamentMode::ChangeState( eState State )
{
	IPvPAllKillModeState* pPrevState = m_pState;
	IPvPAllKillModeState* pNewState = NULL;

	switch( State )
	{
	case eState::None:
		{
			pNewState = new CPvPAllKillModeNoneState( this );			
			break;
		}
	case eState::SelectPlayer:
		{			
			pNewState = new CPvPAllKillModeSelectPlayerState( this );
			SendIdleTick(PvPCommon::AllKillMode::Time::SelectPlayerTick);
			break;
		}
	case eState::Starting:
		{
			if( CheckTournamentActiveActor() )
				pNewState = new CPvPAllKillModeStartingState( this );
			else
			{
				// If it's a bye, check if the match is completely over..
				if ( CheckLastRound() )
				{					
					_ProcessFinishRound(PvPCommon::Team::Max, PvPCommon::FinishReason::OpponentTeamAllGone );
					return;
				}
				else
				{
					//Return to Select state..
					pNewState = new CPvPAllKillModeSelectPlayerState( this );
					SendIdleTick(PvPCommon::AllKillMode::Time::SelectPlayerTick);
				}
			}
			break;
		}
	case eState::Playing:
		{
			pNewState = new CPvPAllKillModePlayingState( this );
			break;
		}
	case eState::FinishRound:
		{
			pNewState = new CPvPAllKillModeFinishRoundState( this );
			break;
		}
	}

	if( pPrevState )
		pPrevState->EndState();
	pNewState->BeginState();

	SAFE_DELETE( pPrevState );
	m_pState = pNewState;
}

void CPvPTournamentMode::_BeforeProcess( LOCAL_TIME LocalTime, float fDelta )
{
	m_pState->BeforeProcess( fDelta );

	if( m_pState->GetState() == eState::None )
	{
		SendTournamentMatchList();
#if defined( PRE_PVP_GAMBLEROOM )
		if(GetGameRoom() && GetGameRoom()->GetGambleRoomDBID() > 0 )
		{
			AddGambleMemberDB();
		}
#endif
		ChangeState( SelectPlayer );
	}
}

void CPvPTournamentMode::_AfterProcess( LOCAL_TIME LocalTime, float fDelta )
{
	m_pState->AfterProcess( fDelta );

	switch( m_pState->GetState() )
	{
	case SelectPlayer:
		{
			if( m_pState->GetElapsedTick() >= PvPCommon::AllKillMode::Time::SelectPlayerTick )
				ChangeState( Starting );
			break;
		}
	case Starting:
		{
			if( m_fStartDelta <= 0.f )
				ChangeState( Playing );
			break;
		}
	case Playing:
		{
			if( m_bFinishRoundFlag == true )
				ChangeState( FinishRound );
			break;
		}
	case FinishRound:
		{
			if( m_fFinishRoundDelta <= 0.f )
				ChangeState( SelectPlayer );
			break;
		}
	}
}

BYTE CPvPTournamentMode::GetTournamentRanking(UINT uiSessionID)
{
	BYTE cRank = 0;
	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
	{		
		if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID == uiSessionID )
		{
			if( (m_sTournamentMatchInfo[i].cTournamentStep == PvPCommon::Tournament::TOURNAMENT_4) && !m_sTournamentMatchInfo[i].bAbuse )			
				cRank = 3;						
			else if( m_sTournamentMatchInfo[i].cTournamentStep == PvPCommon::Tournament::TOURNAMENT_FINAL && !m_sTournamentMatchInfo[i].bAbuse )			
				cRank = m_sTournamentMatchInfo[i].bWin ? 1 : 2;				
			break;
		}
	}
	return cRank;
}

bool CPvPTournamentMode::_CheckFinishGameMode( UINT& uiWinTeam )
{	
	if( uiWinTeam == PvPCommon::Team::Max )
	{		
		if( m_FinishDetailReason == PvPCommon::FinishDetailReason::HPnContinuousWinDraw )
		{
			// Set both as losers here due to abusing.			
			char cATeamIndex = GetTournamentActorIndex(PvPCommon::TeamIndex::A);
			if( cATeamIndex >= 0 && cATeamIndex < PvPCommon::Common::PvPTournamentUserMax )
			{
				m_sTournamentMatchInfo[cATeamIndex].bWin = false;					
				m_sTournamentMatchInfo[cATeamIndex].bAbuse = true;
			}
			char cBTeamIndex = GetTournamentActorIndex(PvPCommon::TeamIndex::B);
			if( cBTeamIndex >= 0 && cBTeamIndex < PvPCommon::Common::PvPTournamentUserMax )
			{
				m_sTournamentMatchInfo[cBTeamIndex].bWin = false;
				m_sTournamentMatchInfo[cBTeamIndex].bAbuse = true;
			}
			m_uiWinSessionID = 0;
			SendTournamentMatchList();
			return false;
		}
		else if( CheckLastRound() )	
		{
			SendLastResult();
			return true;		
		}
		return false;
	}
	PvPCommon::TeamIndex::eCode WinIndex = PvPCommon::Team2Index( static_cast<PvPCommon::Team::eTeam>(uiWinTeam) );		
	PvPCommon::TeamIndex::eCode LoseIndex = (WinIndex == PvPCommon::TeamIndex::A) ? PvPCommon::TeamIndex::B : PvPCommon::TeamIndex::A ;

	// Set winner and loser here.
	DnActorHandle hWinActor = GetActiveActor( WinIndex );
	if( hWinActor )
	{
		CDNUserSession * pSession = GetGameRoom()->GetUserSession(hWinActor->GetSessionID());
		if( pSession )
		{
			char cIndex = GetTournamentActorIndex(WinIndex);
			if( cIndex != -1)		
			{
				m_sTournamentMatchInfo[cIndex].bWin = true;
				m_cCurrentTournamentStep = m_sTournamentMatchInfo[cIndex].cTournamentStep;
			}
			m_uiWinSessionID = pSession->GetSessionID();		
		}		
	}
	// The loser may have left, so actor and session might not exist.
	if( LoseIndex != PvPCommon::Team::Max )
	{
		char cLoseIndex = GetTournamentActorIndex(LoseIndex);
		if( cLoseIndex >= 0 && cLoseIndex < PvPCommon::Common::PvPTournamentUserMax )
			m_sTournamentMatchInfo[cLoseIndex].bWin = false;
	}	

	SendTournamentMatchList();

	// Check if it is the last match.
	if( CheckLastRound() )
	{
		uiWinTeam = PvPCommon::Team::Max;
		SendLastResult();
		return true; // Finish it..
	}
	return false;
}

void CPvPTournamentMode::SetTournamentUserInfo(MAGAPVP_TOURNAMENT_INFO* pPacket)
{
	for( int i=0; i<pPacket->nCount; ++i)
	{
		memcpy(&m_sTournamentMatchInfo[i].sTournamentUserInfo, &pPacket->sTournamentUserInfo[i], sizeof(STournamentUserInfo));		
	}	
}

#if defined( PRE_WORLDCOMBINE_PVP )

void CPvPTournamentMode::SetTournamentUserInfo( int nIndex, CDNUserSession* pGameSession, bool bIgnorePrevData/*= true*/ )
{
	if(!bIgnorePrevData)
	{
		if( m_sTournamentMatchInfo[nIndex].sTournamentUserInfo.uiAccountDBID > 0 )
			return;
	}
	
	if( pGameSession )
	{		
		STournamentUserInfo UserInfo;
		memset(&UserInfo,0,sizeof(UserInfo));
		UserInfo.uiAccountDBID = pGameSession->GetAccountDBID();
		UserInfo.biCharacterDBID = pGameSession->GetCharacterDBID();
		UserInfo.cJob = pGameSession->GetUserJob();
		UserInfo.uiSessionID = pGameSession->GetSessionID();
		_wcscpy( UserInfo.wszCharName, _countof(UserInfo.wszCharName), pGameSession->GetCharacterName(), (int)wcslen(pGameSession->GetCharacterName()) );
		memcpy(&m_sTournamentMatchInfo[nIndex].sTournamentUserInfo, &UserInfo, sizeof(STournamentUserInfo));	
	}	
}

void CPvPTournamentMode::SetTournamentUserInfo()
{
	int nUserCount = GetGameRoom()->GetLiveUserCount();

	DWORD dwCount = 0;

	DnActorHandle hActor;
	for( DWORD i=0; i<GetGameRoom()->GetUserCount(); i++ ) 
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		hActor = GetGameRoom()->GetUserData(i)->GetActorHandle();
		if( pSession && pSession->GetActorHandle() && !pSession->GetActorHandle()->IsDie() )
		{			
			STournamentUserInfo UserInfo;
			memset(&UserInfo,0,sizeof(UserInfo));
			UserInfo.uiAccountDBID = pSession->GetAccountDBID();
			UserInfo.biCharacterDBID = pSession->GetCharacterDBID();
			UserInfo.cJob = pSession->GetUserJob();
			UserInfo.uiSessionID = pSession->GetSessionID();
			_wcscpy( UserInfo.wszCharName, _countof(UserInfo.wszCharName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );
			memcpy(&m_sTournamentMatchInfo[dwCount].sTournamentUserInfo, &UserInfo, sizeof(STournamentUserInfo));	
			dwCount++;
		}
	}
}
void CPvPTournamentMode::SetWorldPvPRoomStart()
{
	if( GetGameRoom()->GetBreakIntoUserCount() == 0)
	{		
		if( !CheckTournamentTeam() )
			SetTournamentUserInfo();
		SendTournamentUserInfo();
		SetStartSetting(true);
	}	
}

bool CPvPTournamentMode::CheckTournamentTeam()
{	
	for( DWORD i=0; i<GetGameRoom()->GetUserCount(); i++ ) 
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);	
		if( pSession && pSession->GetActorHandle() && !pSession->GetActorHandle()->IsDie() )
		{	
			bool bExitUser = false;
			for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax;i++ )
			{
				if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID == pSession->GetAccountDBID() )
				{
					bExitUser = true;
					break;
				}
			}	
			if( !bExitUser )
				return false;
		}
	}
	return true;
}

void CPvPTournamentMode::SendTournamentUserInfo()
{	
	PvPCommon::UserInfoList	UserInfoList;
	memset(&UserInfoList,0,sizeof(UserInfoList));
	WCHAR* pwOffset = UserInfoList.wszBuffer;
	BYTE cUserCount = 0;

	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax;i++ )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserSession(m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID);	
		if( pSession && pSession->GetActorHandle() && !pSession->GetActorHandle()->IsDie() )
		{	
			UserInfoList.uiSessionID[cUserCount]			= pSession->GetSessionID();
			UserInfoList.iJobID[cUserCount]				= pSession->GetUserJob();
			UserInfoList.cLevel[cUserCount]				= pSession->GetLevel();
			UserInfoList.cPvPLevel[cUserCount]			= pSession->GetPvPData()->cLevel;
			UserInfoList.usTeam[cUserCount]				= pSession->GetTeam();
			UserInfoList.cTeamIndex[cUserCount]				= i;
			UserInfoList.uiUserState[cUserCount]			= pSession->GetPvPUserState();
			UserInfoList.cCharacterNameLen[cUserCount]	= static_cast<BYTE>(wcslen(pSession->GetCharacterName()));
			memcpy( pwOffset, pSession->GetCharacterName(), sizeof(WCHAR)*UserInfoList.cCharacterNameLen[cUserCount] );
			pwOffset += UserInfoList.cCharacterNameLen[cUserCount];

			++cUserCount;	
		}
	}	

	int nSize = static_cast<int>(sizeof(PvPCommon::UserInfoList)-sizeof(UserInfoList.wszBuffer)+(sizeof(WCHAR)*(pwOffset-UserInfoList.wszBuffer)));

	if( cUserCount > 0 )
	{
		for( DWORD i=0; i<GetGameRoom()->GetUserCount(); i++ ) 
		{
			CDNUserSession* pSession = GetGameRoom()->GetUserData(i);	
			if( pSession )
			{
				pSession->SendWorldPvPRoomTournamentUserInfo( cUserCount, &UserInfoList, nSize);
			}
		}
	}	

	return;
}

#endif

bool CPvPTournamentMode::CheckLastRound()
{
	if( m_cTournamentStep == PvPCommon::Tournament::TOURNAMENT_FINAL && m_cTournamentStepCount == 1) // If finals.		
		return true;
	else
	{
		// If both are true, it is not the last..
		int nWinUserCount = 0;
		char cLastIndex = -1;
		for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
		{
			if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID > 0 )
			{				
				if( !m_sTournamentMatchInfo[i].bLeave && (m_sTournamentMatchInfo[i].bWin == true || m_sTournamentMatchInfo[i].bStartMatch == false) ) // Either no one played the match or someone won
				{
					++nWinUserCount;
					cLastIndex = i;
				}
				if( nWinUserCount >= 2)
					return false;
			}
		}
		// At this time, force-set the last one to 1st place.
		if( cLastIndex != -1 )
		{
			m_sTournamentMatchInfo[cLastIndex].cTournamentStep = PvPCommon::Tournament::TOURNAMENT_FINAL;
			m_sTournamentMatchInfo[cLastIndex].bWin = true;
			m_uiWinSessionID = m_sTournamentMatchInfo[cLastIndex].sTournamentUserInfo.uiSessionID;
		}
		return true;
	}
	return false;
}

void CPvPTournamentMode::OnLeaveUser( DnActorHandle hActor )
{
	if( hActor )
	{
		for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
		{
			if(m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID == hActor->GetSessionID())
			{
				m_sTournamentMatchInfo[i].bLeave = true;
			}
		}
	}	
	CPvPAllKillMode::OnLeaveUser( hActor );	
}

void CPvPTournamentMode::OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason )
{
	UINT uiWinTeam = PvPCommon::Team::Max;
	m_FinishDetailReason = PvPCommon::FinishDetailReason::None;

	switch( Reason )
	{
	case PvPCommon::FinishReason::OpponentTeamAllDead:
		{
			DnActorHandle hATeamActor = GetActiveActor( PvPCommon::TeamIndex::A );
			DnActorHandle hBTeamActor = GetActiveActor( PvPCommon::TeamIndex::B );

			if( !hATeamActor || !hBTeamActor )
				return;

			if( hATeamActor->IsDie() && hBTeamActor->IsDie() )
				return;

			if( !(hATeamActor->IsDie()) && !(hBTeamActor->IsDie()) )
				return;

			uiWinTeam = hATeamActor->IsDie() ? PvPCommon::Team::B : PvPCommon::Team::A;
			break;
		}
	case PvPCommon::FinishReason::OpponentTeamAllGone:
		{
			DnActorHandle hATeamActor = GetActiveActor( PvPCommon::TeamIndex::A );
			DnActorHandle hBTeamActor = GetActiveActor( PvPCommon::TeamIndex::B );

			if( hATeamActor && hBTeamActor )
				return;

			if( hATeamActor && !hBTeamActor )
				uiWinTeam = PvPCommon::Team::A;
			else if( !hATeamActor && hBTeamActor )
				uiWinTeam = PvPCommon::Team::B;
			break;
		}
	case PvPCommon::FinishReason::TimeOver:
		{
			DnActorHandle hATeamActor = GetActiveActor( PvPCommon::TeamIndex::A );
			DnActorHandle hBTeamActor = GetActiveActor( PvPCommon::TeamIndex::B );

			if( hATeamActor && hBTeamActor )
			{
				m_FinishDetailReason = PvPCommon::FinishDetailReason::SuperiorHPWin;

				int iAHPPercent = hATeamActor->GetHPPercent();
				int iBHPPercent = hBTeamActor->GetHPPercent();

				if( iAHPPercent > iBHPPercent )
					uiWinTeam = PvPCommon::Team::A;
				else if( iAHPPercent < iBHPPercent )
					uiWinTeam = PvPCommon::Team::B;
				else
				{
					// Check via aggro whether it actually fought
					CDNAggroSystem::AggroStruct* pATargetStruct = hATeamActor->GetAggroSystem()->GetAggroStruct(hBTeamActor);
					CDNAggroSystem::AggroStruct* pBTargetStruct = hBTeamActor->GetAggroSystem()->GetAggroStruct(hATeamActor);
					
					if( pATargetStruct && pBTargetStruct )
					{
						if( pATargetStruct->iAggro > pBTargetStruct->iAggro )						
						{
							m_FinishDetailReason = PvPCommon::FinishDetailReason::SuperiorContinuousWin;
							uiWinTeam = PvPCommon::Team::B;
						}
						else if ( pATargetStruct->iAggro < pBTargetStruct->iAggro )						
						{
							m_FinishDetailReason = PvPCommon::FinishDetailReason::SuperiorContinuousWin;
							uiWinTeam = PvPCommon::Team::A;
						}
						else
						{
							uiWinTeam = PvPCommon::Team::Max;
							m_FinishDetailReason = PvPCommon::FinishDetailReason::HPnContinuousWinDraw;
						}						
					}
					else 
					{
						uiWinTeam = PvPCommon::Team::Max;
						m_FinishDetailReason = PvPCommon::FinishDetailReason::HPnContinuousWinDraw;
					}
				}
			}
			else if( !hATeamActor && !hBTeamActor )
			{
				uiWinTeam = PvPCommon::Team::Max;
			}
			else
			{
				uiWinTeam = hATeamActor ? PvPCommon::Team::A : PvPCommon::Team::B;
			}

			break;
		}
	default:
		{
			return;
		}
	}

	_ProcessFinishRound( uiWinTeam, Reason );
}

void CPvPTournamentMode::_ProcessFinishRoundMode( CDNUserSession* pSession, bool bIsWin, UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor )
		return;

	if( bIsActiveActor( hActor) == false )
		return;

	if( bIsWin == true )			
		hActor->CmdRefreshHPSP( hActor->GetMaxHP(), hActor->GetMaxSP() );	
	else	
		hActor->CmdRefreshHPSP( 0,0 );

	PvPCommon::TeamIndex::eCode TeamIndex = PvPCommon::Team2Index( static_cast<PvPCommon::Team::eTeam>(hActor->GetTeam()) );
	if( TeamIndex == PvPCommon::TeamIndex::Max )
		return;	

	char cIndex = GetTournamentActorIndex(TeamIndex);	
	if( cIndex != -1)		
	{	
		pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 4, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Tournament,
			EventSystem::PvPTournamentType, m_cTournamentType, EventSystem::PvPTournamentAdvance, m_sTournamentMatchInfo[cIndex].cTournamentStep,
			EventSystem::PvPTournamentWin, bIsWin == true ? 1 : 0);
	}
}

bool CPvPTournamentMode::CheckTournamentActiveActor()
{	
	while(true) // Exit condition is when 1:1 is formed or the finals are set.
	{
		if( m_cTournamentStep == PvPCommon::Tournament::TOURNAMENT_FINAL && m_cTournamentStepCount == 1) // If finals.		
			return false;

		char cATeamIndex = -1;
		char cBTeamIndex = -1;		
		CDNUserSession* pATeamUser = NULL;
		CDNUserSession* pBTeamUser = NULL;
		GetTournamentActiveActorIndex(m_cTournamentStep*2, cATeamIndex, cBTeamIndex);			

		if( cATeamIndex >= 0 && cATeamIndex < PvPCommon::Common::PvPTournamentUserMax)	
		{
			pATeamUser = GetGameRoom()->GetUserSession(m_sTournamentMatchInfo[cATeamIndex].sTournamentUserInfo.uiSessionID);					
			m_sTournamentMatchInfo[cATeamIndex].cTournamentStep = m_cTournamentStep;
			m_sTournamentMatchInfo[cATeamIndex].bStartMatch = true;
		}
		if( cBTeamIndex >= 0 && cBTeamIndex < PvPCommon::Common::PvPTournamentUserMax)
		{
			pBTeamUser = GetGameRoom()->GetUserSession(m_sTournamentMatchInfo[cBTeamIndex].sTournamentUserInfo.uiSessionID);				
			m_sTournamentMatchInfo[cBTeamIndex].cTournamentStep = m_cTournamentStep;
			m_sTournamentMatchInfo[cBTeamIndex].bStartMatch = true;
		}

		++ m_cTournamentStepCount;
		if( m_cTournamentStep/2 == m_cTournamentStepCount && m_cTournamentStep > PvPCommon::Tournament::TOURNAMENT_FINAL )
		{
			//Next
			m_cTournamentStepCount = 0;
			m_cTournamentStep = m_cTournamentStep/2;
			m_cTournamentArea = m_cTournamentArea*2;
		}

		if( (pATeamUser && pATeamUser->GetActorHandle()) && (pBTeamUser == NULL || !pBTeamUser->GetActorHandle()) ) // Bye (auto-advance)	
		{
			m_sTournamentMatchInfo[cATeamIndex].bWin = true;
			if( cBTeamIndex >= 0)
				m_sTournamentMatchInfo[cBTeamIndex].bWin = false;
			m_uiWinSessionID = pATeamUser->GetSessionID();
			SendTournamentDefaultWin(m_sTournamentMatchInfo[cATeamIndex].cTournamentStep, pATeamUser->GetSessionID());
			SendTournamentMatchList();
			return false;
		}
		else if( (pATeamUser == NULL || !pATeamUser->GetActorHandle()) && (pBTeamUser && pBTeamUser->GetActorHandle()) )
		{
			if( cATeamIndex >= 0 )
				m_sTournamentMatchInfo[cATeamIndex].bWin = false;
			m_sTournamentMatchInfo[cBTeamIndex].bWin = true;
			m_uiWinSessionID = pBTeamUser->GetSessionID();
			SendTournamentDefaultWin(m_sTournamentMatchInfo[cBTeamIndex].cTournamentStep, pBTeamUser->GetSessionID());
			SendTournamentMatchList();
			return false;
		}
		else if( (pATeamUser == NULL || !pATeamUser->GetActorHandle()) && (pBTeamUser == NULL || !pBTeamUser->GetActorHandle()) )
		{
			if( cATeamIndex >= 0 )			
				m_sTournamentMatchInfo[cATeamIndex].bWin = false;							
			if( cBTeamIndex >= 0)			
				m_sTournamentMatchInfo[cBTeamIndex].bWin = false;			
		}
		else if( (pATeamUser && pATeamUser->GetActorHandle()) && (pBTeamUser && pBTeamUser->GetActorHandle()) )
		{
			pATeamUser->GetActorHandle()->CmdChangeTeam(PvPCommon::Team::A);				
			pBTeamUser->GetActorHandle()->CmdChangeTeam(PvPCommon::Team::B);
			SetActiveActor( PvPCommon::TeamIndex::A, pATeamUser->GetActorHandle(), false );				
			SetActiveActor( PvPCommon::TeamIndex::B, pBTeamUser->GetActorHandle(), false );	
			SetTournamentActorIndex(PvPCommon::TeamIndex::A, cATeamIndex);
			SetTournamentActorIndex(PvPCommon::TeamIndex::B, cBTeamIndex);
			return true;
		}		
	}
	return false;
}

void CPvPTournamentMode::GetTournamentActiveActorIndex(char cStep, char& cATeamIndex, char& cBTeamIndex)
{
	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
	{
		if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID > 0 )
		{		
			if( m_sTournamentMatchInfo[i].cTournamentStep == PvPCommon::Tournament::TOURNAMENT_NONE || 
				(m_sTournamentMatchInfo[i].cTournamentStep == cStep && m_sTournamentMatchInfo[i].bWin == true) )
			{
				if(cATeamIndex == -1)
					cATeamIndex = i;
				else if(cBTeamIndex == -1)
				{
					// Check if currently in the same Group as A.
					if( cATeamIndex/m_cTournamentArea == i/m_cTournamentArea )
					{
						cBTeamIndex = i;
						break;
					}				
				}
			}
		}
	}
}

void CPvPTournamentMode::OnSuccessBreakInto( CDNUserSession* pGameSession )
{
	CPvPAllKillMode::OnSuccessBreakInto(pGameSession);
	SendTournamentMatchList(pGameSession);
}

char CPvPTournamentMode::GetTournamentActorIndex(PvPCommon::TeamIndex::eCode Team)
{	
	return m_cTournamentActorIndex[Team];	
}

void CPvPTournamentMode::SetTournamentActorIndex( PvPCommon::TeamIndex::eCode Team, BYTE cActorIndex )
{
	if( cActorIndex >= PvPCommon::Common::PvPTournamentUserMax )
		return;
	m_cTournamentActorIndex[Team] = cActorIndex;
}

void CPvPTournamentMode::SendTournamentMatchList(CDNUserSession* pBreakIntoUserSession)
{
	SCPvPTournamentMatchList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
	{
		if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID > 0 )
		{
			TxPacket.sTournamentUserInfo[i].cTournamentStep = m_sTournamentMatchInfo[i].cTournamentStep;
			TxPacket.sTournamentUserInfo[i].bWin = m_sTournamentMatchInfo[i].bWin;
			TxPacket.sTournamentUserInfo[i].cJob = m_sTournamentMatchInfo[i].sTournamentUserInfo.cJob;
			TxPacket.sTournamentUserInfo[i].uiSessionID = m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID;
			memcpy(TxPacket.sTournamentUserInfo[i].wszCharName, m_sTournamentMatchInfo[i].sTournamentUserInfo.wszCharName, sizeof(TxPacket.sTournamentUserInfo[i].wszCharName));
			++TxPacket.nCount;
		}	
	}

#if defined( PRE_PVP_GAMBLEROOM )
	CDNPvPGameRoom * pPvPRoom = static_cast<CDNPvPGameRoom*>(GetGameRoom());
	if(pPvPRoom)
	{
		pPvPRoom->SetGamblePlayerCount(TxPacket.nCount);		
		TxPacket.cGambleType = pPvPRoom->GetGambleType();
		TxPacket.nGamblePrice = pPvPRoom->GetGamblePrice();
	}	
#endif

	int nLen = sizeof(SCPvPTournamentMatchList) - sizeof(TxPacket.sTournamentUserInfo) + (sizeof(TPvPTournamentUserInfo)*TxPacket.nCount);

	if( pBreakIntoUserSession )
	{
		pBreakIntoUserSession->AddSendData( SC_PVP, ePvP::SC_PVP_TOURNAMENT_MATCHLIST, reinterpret_cast<char*>(&TxPacket), nLen );
		return;
	}

	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( pSession == NULL )
			continue;

		pSession->AddSendData( SC_PVP, ePvP::SC_PVP_TOURNAMENT_MATCHLIST, reinterpret_cast<char*>(&TxPacket), nLen );
	}
}

void CPvPTournamentMode::SendTournamentDefaultWin(char cTournamentStep, UINT uiWinSessionID)
{
	SCPvPTournamentDefaultWin TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cTournamentStep = cTournamentStep;
	TxPacket.uiWinSessionID = uiWinSessionID;

	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( pSession == NULL )
			continue;

		pSession->AddSendData( SC_PVP, ePvP::SC_PVP_TOURNAMENT_DEFAULTWIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	}
}


void CPvPTournamentMode::SendIdleTick(UINT uiIdleTick)
{
	SCPvPTournamentIdleTick TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiCurTick = timeGetTime();
	TxPacket.uiIdleTick = uiIdleTick;

	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( pSession == NULL )
			continue;

		pSession->AddSendData( SC_PVP, ePvP::SC_PVP_TOURNAMENT_IDLE_TICK, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	}
}
void CPvPTournamentMode::SendLastResult()
{
	// Send to participants up to 4th place.
	SendTournamentTop4();
	// Insert log into DB
	CDNDBConnection* pDBCon = GetGameRoom()->GetDBConnection();
	if( pDBCon )
	{
#if defined( PRE_PVP_GAMBLEROOM )
		pDBCon->QueryAddTournamentResult(GetGameRoom()->GetDBThreadID(), GetGameRoom()->GetWorldSetID(), &m_sTournamentMatchInfo[0], GetGameRoom()->GetGambleRoomDBID());
#else
		pDBCon->QueryAddTournamentResult(GetGameRoom()->GetDBThreadID(), GetGameRoom()->GetWorldSetID(), &m_sTournamentMatchInfo[0]);
#endif
	}
}

void CPvPTournamentMode::SendTournamentTop4()
{
	SCPvPTournamentTop4 TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));	

#if defined( PRE_PVP_GAMBLEROOM )
	CDNPvPGameRoom * pPvPRoom = static_cast<CDNPvPGameRoom*>(GetGameRoom());
	BYTE cGambleRoomType = 0;		
	INT64 nGambleRoomDBID = 0;
	TPvPGambleRoomWinData Member[PvPGambleRoom::Max];
	memset(Member, 0, sizeof(Member));
	
	if(pPvPRoom)
	{
		if( pPvPRoom->GetGambleType() > 0 )
		{
			cGambleRoomType = pPvPRoom->GetGambleType();
			nGambleRoomDBID = pPvPRoom->GetGambleRoomDBID();
			int nTotalPrice = pPvPRoom->GetGamblePrice() * pPvPRoom->GetGamblePlayerCount();
			float fRate = CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PvPGambleFirstWinnerRate);
			int nGold = 1;
			if( pPvPRoom->GetGambleType() == PvPGambleRoom::Gold )
				nGold = 10000;
			Member[PvPGambleRoom::firstWinner].nPrice = ((int)(nTotalPrice * fRate) * nGold);

			fRate = CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::PvPGambleSecondWinnerRate);
			Member[PvPGambleRoom::SecondWinner].nPrice = ((int)(nTotalPrice * fRate) * nGold);
		}
	}
#endif
	BYTE cRank = 0;
	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
	{
		if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID > 0 )
		{
			cRank = 0;		
			if( (m_sTournamentMatchInfo[i].cTournamentStep == PvPCommon::Tournament::TOURNAMENT_4) && !m_sTournamentMatchInfo[i].bAbuse )			
				cRank = 3;						
			else if( (m_sTournamentMatchInfo[i].cTournamentStep == PvPCommon::Tournament::TOURNAMENT_FINAL) && !m_sTournamentMatchInfo[i].bAbuse )			
				cRank = m_sTournamentMatchInfo[i].bWin ? 1 : 2;

			if( cRank > 0)
			{			
				CDNUserSession* pUser = GetGameRoom()->GetUserSession(m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID);
				if( pUser ) // Only connected users
				{
					if( cRank == 3 && TxPacket.uiSessionID[2] > 0) // 2 people for 3rd place
						TxPacket.uiSessionID[3] = m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID;
					else
					{
						TxPacket.uiSessionID[cRank-1] = m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID;
#if defined( PRE_PVP_GAMBLEROOM )
						if( cGambleRoomType > 0 && cRank <= PvPGambleRoom::Max)
						{
							if( cGambleRoomType == PvPGambleRoom::Gold )
							{
								int nLogCode = 0;
								if(cRank == 1 )
									nLogCode = DBDNWorldDef::CoinChangeCode::GambleFirstWinner;
								else
									nLogCode = DBDNWorldDef::CoinChangeCode::GambleSecondWinner;

								pUser->AddCoin( Member[cRank-1].nPrice, nLogCode, nGambleRoomDBID, true );								
							}
							else if( cGambleRoomType == PvPGambleRoom::Petal && pUser->GetDBConnection() )
							{
								int nLogCode = 0;
								if(cRank == 1 )
									nLogCode = DBDNWorldDef::CoinChangeCode::GambleFirstWinPetal;
								else
									nLogCode = DBDNWorldDef::CoinChangeCode::GambleSecondWinPetal;

								pUser->GetDBConnection()->QueryPetalLog(pUser, Member[cRank-1].nPrice, 0, DBDNWorldDef::CoinChangeCode::GambleFirstWinPetal, nGambleRoomDBID);								
							}
							TxPacket.nGamblePrice[cRank-1] = Member[cRank-1].nPrice;
							Member[cRank-1].biCharacterDBID = pUser->GetCharacterDBID();
						}						
#endif
					}
				}
			}
		}		
	}

	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( pSession == NULL )
			continue;

		pSession->AddSendData( SC_PVP, ePvP::SC_PVP_TOURNAMENT_TOP4, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	}

#if defined( PRE_PVP_GAMBLEROOM )
	if( GetGameRoom()->GetDBConnection() && cGambleRoomType > 0 )
	{
		GetGameRoom()->GetDBConnection()->QueryEndGambleRoom(GetGameRoom()->GetDBThreadID(), GetGameRoom()->GetWorldSetID(), GetGameRoom()->GetGambleRoomDBID(), Member);
	}
#endif

}
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

#if defined( PRE_PVP_GAMBLEROOM )
void CPvPTournamentMode::AddGambleMemberDB()
{
	if( !GetGameRoom()->bIsPvPRoom() )
		return;
	
	CDNPvPGameRoom * pPvPRoom = static_cast<CDNPvPGameRoom*>(GetGameRoom());
	
	int nGamblePrice = pPvPRoom->GetGamblePrice();

	if( pPvPRoom->GetGambleType() == PvPGambleRoom::Gold )
		nGamblePrice *= 10000 ;

	INT64 nGambleRoomDBID = pPvPRoom->GetGambleRoomDBID(); 
	for( int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i )
	{
		if( m_sTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID > 0 )
		{
			CDNUserSession* pUser = GetGameRoom()->GetUserSession(m_sTournamentMatchInfo[i].sTournamentUserInfo.uiSessionID);
			if( pUser && pUser->GetDBConnection() ) // Only connected users
			{
				// Participating user
				pUser->GetDBConnection()->QueryAddGambleRoomMember( pUser->GetDBThreadID(), pUser->GetWorldSetID(), nGambleRoomDBID, pUser->GetCharacterDBID());
				// Delete entry fee
				if( pPvPRoom->GetGambleType() == PvPGambleRoom::Gold )
					pUser->DelCoin( nGamblePrice, DBDNWorldDef::CoinChangeCode::GamblePrice, nGambleRoomDBID, true );
				else if( pPvPRoom->GetGambleType() == PvPGambleRoom::Petal )
					pUser->GetDBConnection()->QueryUsePetal(pUser->GetDBThreadID(), pUser->GetWorldSetID(), pUser->GetAccountDBID(), pUser->GetCharacterDBID(), nGamblePrice, DBDNWorldDef::CoinChangeCode::GambleEnterPetal, nGambleRoomDBID);
			}			
		}		
	}
}
#endif