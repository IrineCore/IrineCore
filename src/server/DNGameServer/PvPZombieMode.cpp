
#include "stdafx.h"
#include "PvPZombieMode.h"
#include "DnActor.h"
#include "DNUserSession.h"
#include "DNPvPGameRoom.h"
#include "DnPlayerActor.h"
#include "DnStateBlow.h"
#include "DNGameDataManager.h"
#include "DNDBConnection.h"
#include "DNMissionSystem.h"

CPvPZombieMode::CPvPZombieMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket )
:CPvPRoundMode( pGameRoom, pPvPGameModeTable, pPacket )
{
	memset(&m_MutationGroup, 0, sizeof(TMonsterMutationGroup));
	m_nGhoulCount = 0;
	m_bInitializeZombie = false;
}

CPvPZombieMode::~CPvPZombieMode()
{

}

void CPvPZombieMode::OnInitializeActor( CDNUserSession* pSession )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	if( !hActor )
		return;

	if( pSession->GetTeam() == PvPCommon::Team::Observer )
		return;

	hActor->SetTeam( PvPCommon::Team::Human );
	pSession->SetTeam( hActor->GetTeam() );

	_AddHumanBlow( hActor );
}

void CPvPZombieMode::Process( LOCAL_TIME LocalTime, float fDelta )
{
	_ProcessSelectZombie();

	CPvPRoundMode::Process( LocalTime, fDelta );

	// Check if opponent left while Playing
	if( bIsPlaying() )
	{
		OnCheckFinishRound( PvPCommon::FinishReason::OpponentTeamAllGone );
	}
}

void CPvPZombieMode::OnFinishProcessDie( DnActorHandle hActor )
{
	if( !hActor )
		return;

	if( bIsPlaying() != true )
		return;

	if( bIsPlayingUser( hActor) == false )
		return;

	// Set respawn position
	if( bIsZombie( hActor ) )
		_SetRespawnPosition( hActor );

	// Resurrect
	UINT uiRespawnHPPercent = GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnHPPercent			: 100;
	UINT uiRespawnMPPercent = GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnMPPercent			: 100;
	UINT uiImmortalTime		= GetPvPGameModeTable() ? GetPvPGameModeTable()->uiRespawnNoDamageTimeSec	: 5;

	char szParam[32];
	sprintf_s( szParam, "Coin/%d/%d/%d", uiRespawnHPPercent, uiRespawnMPPercent, uiImmortalTime );
	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_057, uiImmortalTime*1000, szParam );
}

void CPvPZombieMode::OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason )
{
	UINT uiWinTeam = PvPCommon::Team::Max;

	switch( Reason )
	{
		// End match if either all zombies or all users have left
		case PvPCommon::FinishReason::OpponentTeamAllGone:
		{
			if( m_bInitializeZombie == false )
				return;

			int iPlayerCount = 0;
			int iZombieCount = 0;
			for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
			{
				CDNUserSession* pSession	= GetGameRoom()->GetUserData(i);
				DnActorHandle	hActor		= pSession->GetActorHandle();
				if( bIsPlayingUser( hActor ) == false )
					continue;

				if( hActor->GetTeam() == PvPCommon::Team::A )
					++iPlayerCount;
				else if( hActor->GetTeam() == PvPCommon::Team::Zombie )
					++iZombieCount;
			}

			// Both user and zombie are alive!
			if( iPlayerCount > 0 && iZombieCount > 0 )
				return;

			if( iPlayerCount == 0 && iZombieCount > 0 )
				uiWinTeam = PvPCommon::Team::Zombie;
			else if( iZombieCount == 0 && iPlayerCount > 0 )
				uiWinTeam = PvPCommon::Team::A;

			CPvPRoundMode::_ProcessFinishRound( uiWinTeam, Reason );
			return;
		}
		// In Zombie mode, users unconditionally win on TimeOver.
		case PvPCommon::FinishReason::TimeOver:
		{
			uiWinTeam = PvPCommon::Team::A;
			CPvPRoundMode::_ProcessFinishRound( uiWinTeam, Reason );
			return;
		}
		// In Zombie mode, only check whether all zombies are dead.
		case PvPCommon::FinishReason::OpponentTeamAllDead:
		{
			if( m_bInitializeZombie == false )
				return;

			int iZombieCount = 0;
			int iZombieDeadCount = 0;
			for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
			{
				CDNUserSession* pSession	= GetGameRoom()->GetUserData(i);
				DnActorHandle	hActor		= pSession->GetActorHandle();
				if( bIsPlayingUser( hActor ) == false )
					continue;

				if( hActor->GetTeam() == PvPCommon::Team::Zombie )
				{
					++iZombieCount;
					if( hActor->IsDie() )
						++iZombieDeadCount;
				}
			}

			if( iZombieCount <= 0 )
				return;

			if( iZombieCount == iZombieDeadCount )
				uiWinTeam = PvPCommon::Team::Human;
			else
				return;

			CPvPRoundMode::_ProcessFinishRound( uiWinTeam, Reason );
			return;
		}
	}

	CPvPRoundMode::OnCheckFinishRound( Reason );
}

void CPvPZombieMode::OnFinishRound()
{
	
}

void CPvPZombieMode::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	CPvPGameMode::OnDie( hActor, hHitter );

	// If Human->Zombie, transform immediately
	if( hActor && hActor->IsPlayerActor() && static_cast<CDnPlayerActor*>(hActor.GetPointer())->IsTransformMode() == false )
		hActor->SetDieDelta( 0.f );
}

void CPvPZombieMode::OnLeaveUser( DnActorHandle hActor )
{
	if( hActor )
		m_mZombieActor.erase( hActor->GetUniqueID() );
	CPvPRoundMode::OnLeaveUser( hActor );
	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( pPlayer )
	{		
		CDNUserSession* pSession = pPlayer->GetUserSession();
		if( pSession->GetAddGhoulScore(GhoulMode::PointType::GhoulKill) || pSession->GetAddGhoulScore(GhoulMode::PointType::HumanKill) || pSession->GetAddGhoulScore(GhoulMode::PointType::HolyWaterUse) )
		{
			pSession->GetDBConnection()->QueryAddPVPGhoulScores(pSession);
			pSession->ClearAddGhoulScore();
		}		
	}
}

void CPvPZombieMode::OnSuccessBreakInto( CDNUserSession* pGameSession )
{
	CPvPRoundMode::OnSuccessBreakInto( pGameSession );

	// Send zombie info to drop-in user.
	pGameSession->SendPvPSelectZombie( m_mZombieActor );
}

void CPvPZombieMode::OnRebirth( DnActorHandle hActor, bool bForce/*=false*/ )
{
	TMonsterMutationData Data;
	memset(&Data, 0, sizeof(TMonsterMutationData));
	if( GetZombieActorData(Data) == false )
		_ASSERT(0);
	if( bIsZombie(hActor) )
	{
		_AddZombieBlow( hActor, Data );
	}
	else
	{
		_ChangeZombie( hActor, Data );
	}
}

void CPvPZombieMode::_ProcessSelectZombie()
{
	if( m_bInitializeZombie )
		return;

	if( bIsPlaying() == false )
		return;
	
	// Marked as Zombie after 10 seconds.
	if( GetElapsedTimeRound() < static_cast<float>(PvPCommon::Common::RoundStartMutationTimeSec) )
		return;

	// No need to set if zombie is already assigned.
	if( m_mZombieActor.empty() == false )
		return;

	InitializeZombie();
	m_bInitializeZombie = true;
}

void CPvPZombieMode::_OnStartRound()
{
	CPvPRoundMode::_OnStartRound();
	AllReturnHuman();
	m_bInitializeZombie = false;
	m_mZombieActor.clear();
}

void CPvPZombieMode::_ProcessFinishRoundMode( CDNUserSession* pSession, bool bIsWin, UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason )
{
	if( bIsWin )
	{					
		switch(Reason)
		{
			case PvPCommon::FinishReason::TimeOver : // Time over 
			{
				pSession->AddGhoulScore(GhoulMode::PointType::TimeOver);	
				pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 4, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Zombie_Survival,
													   EventSystem::PvPTeam, PvPCommon::Team::Human, EventSystem::PvPWinCondition, PvPCommon::FinishReason::TimeOver,
													   EventSystem::GhoulModeWin, pSession->GetTotalGhoulScore(GhoulMode::PointType::TimeOver));
				break;
			}
			case PvPCommon::FinishReason::OpponentTeamAllDead :
			{
				if( uiWinTeam == PvPCommon::Team::Human ) // All zombies are dead.
				{
					pSession->AddGhoulScore(GhoulMode::PointType::HumanWin);
					pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 4, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Zombie_Survival,
														   EventSystem::PvPTeam, PvPCommon::Team::Human, EventSystem::PvPWinCondition, PvPCommon::FinishReason::OpponentTeamAllDead,
														   EventSystem::GhoulModeWin, pSession->GetTotalGhoulScore(GhoulMode::PointType::HumanWin));
				}
				break;
			}
			case PvPCommon::FinishReason::OpponentTeamAllGone :
			{
				if( uiWinTeam == PvPCommon::Team::Human )
				{
					pSession->AddGhoulScore(GhoulMode::PointType::HumanWin);
					pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 4, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Zombie_Survival,
														   EventSystem::PvPTeam, PvPCommon::Team::Human, EventSystem::PvPWinCondition, PvPCommon::FinishReason::OpponentTeamAllGone,
														   EventSystem::GhoulModeWin, pSession->GetTotalGhoulScore(GhoulMode::PointType::HumanWin));
				}
				else if( uiWinTeam == PvPCommon::Team::Zombie )
				{
					pSession->AddGhoulScore(GhoulMode::PointType::GhoulWin);
					pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 4, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Zombie_Survival,
														   EventSystem::PvPTeam, PvPCommon::Team::Zombie, EventSystem::PvPWinCondition, PvPCommon::FinishReason::OpponentTeamAllGone,
														   EventSystem::GhoulModeWin, pSession->GetTotalGhoulScore(GhoulMode::PointType::GhoulWin));
				}
				break;
			}						
		}
		
		pSession->AddGhoulScore(GhoulMode::PointType::PlayCount);	
		// Save round match count to DB here.
		pSession->GetDBConnection()->QueryAddPVPGhoulScores( pSession );
		pSession->ClearAddGhoulScore();

		// Ghoul mode round count
		pSession->GetEventSystem()->OnEvent( EventSystem::OnRoundFinished, 2, EventSystem::PvPGameMode, PvPCommon::GameMode::PvP_Zombie_Survival,
											   EventSystem::PvPRoundCount, pSession->GetTotalGhoulScore(GhoulMode::PointType::PlayCount));
	}		
}

void CPvPZombieMode::_ChangeZombie( DnActorHandle hActor, const TMonsterMutationData &Data )
{
	if( bIsZombie(hActor) )
		return;

	if( !hActor || !hActor->IsPlayerActor() )
		return;	
	
	// Set zombie info
	if (AddZombie(hActor, Data ))
	{
		_AddZombieBlow( hActor, Data );

		_UpdateSelectZombieCount( hActor->GetSessionID() );
		return;
	}
	
	_DANGER_POINT();
}

void CPvPZombieMode::_AddHumanBlow( DnActorHandle hActor )
{
	if( !hActor )
		return;

	// Skill restriction when changing to Human
	std::string strSkillVec;
	std::vector< std::pair<int,int> > vData;
	bool bRet = g_pDataManager->GetPvPGameModeSkillSetting( GetPvPGameModeTable()->iSkillSetting, hActor->GetClassID(), vData );
	if( bRet )
	{
		hActor->CmdRemoveStateEffect(STATE_BLOW::BLOW_176); // Initialize the skill-restriction blow.
		hActor->GetStateBlow()->Process( 0, 0.f );

		for(DWORD i=0; i<vData.size();i++)
		{
			char szSkillIndex[256];
			sprintf(szSkillIndex, "%d;", vData[i].first);
			strSkillVec += szSkillIndex;
		}
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_176, -1, strSkillVec.c_str(), true );				// Disable all skills except the configured human skills.
	}

	if(!(hActor->GetStateBlow()->IsApplied(STATE_BLOW::BLOW_183)))
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_183, -1, NULL, true );
}

void CPvPZombieMode::_AddZombieBlow( DnActorHandle hActor, const TMonsterMutationData &Data )
{
	{
		USES_CONVERSION;
		
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_058, -1, I2A(Data.nMutationHP), true );				// 58) Max HP ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_002, -1, I2A(Data.nMutationStrPMax), true );			// 2) Physical attack power ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_029, -1, I2A(Data.nMutationStrMMax), true );			// 29) Magic attack ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_004, -1, I2A(Data.nMutationDefP), true );				// 4) Physical defense ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_094, -1, I2A(Data.nMutationDefM), true );				// 94) Magic defense ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_123, -1, I2A(Data.nMutationStiff), true );			// 123) Change flinch power ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_124, -1, I2A(Data.nMutationStiffResis), true );		// 124) Change flinch resistance ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_126, -1, I2A(Data.nMutationCriticalResis), true );	// 126) Change critical resistance ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_128, -1, I2A(Data.nMutationStunResis), true );		// 128) Change stun resistance ratio
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_061, -1, I2A(Data.nMutationSuperArmor), true );		// 61) Grant Super Armor 1
		if(!(hActor->GetStateBlow()->IsApplied(STATE_BLOW::BLOW_183)))
			hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_183, -1, NULL, true );								// 183) State effect that prevents prefix state effects in Ghoul mode
	}

	hActor->GetStateBlow()->Process( 0, 0.f );
	hActor->CmdRefreshHPSP( hActor->GetMaxHP(), hActor->GetMaxSP() );
}

bool CPvPZombieMode::bIsZombie( DnActorHandle hActor )
{
	if( !hActor || !hActor->IsPlayerActor() )
		return false;

	std::map<DWORD,DnActorHandle>::iterator itor = m_mZombieActor.find( hActor->GetUniqueID() );
	if( itor == m_mZombieActor.end() )
		return false;

	return true;
}

void CPvPZombieMode::_UpdateSelectZombieCount( UINT uiSessionID )
{
	std::map<UINT,UINT>::iterator itor = m_mSelectZombieCount.find( uiSessionID );
	if( itor != m_mSelectZombieCount.end() )
	{
		++(*itor).second;
	}
	else
	{
		m_mSelectZombieCount.insert( std::make_pair(uiSessionID,1) );
	}
}

UINT CPvPZombieMode::_GetSelectZombieCount( UINT uiSessionID )
{
	std::map<UINT,UINT>::iterator itor = m_mSelectZombieCount.find( uiSessionID );
	if( itor != m_mSelectZombieCount.end() )
		return (*itor).second;

	return 0;
}

void CPvPZombieMode::_GetZombieCandidate( std::vector<DnActorHandle>& vResult )
{
	UINT uiMinValue = UINT_MAX;

	std::vector<DnActorHandle> vCandidate;
	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( !pSession || !pSession->GetActorHandle() )
			continue;
		if( !bIsPlayingUser( pSession->GetActorHandle() ) )
			continue;
		UINT uiCount = _GetSelectZombieCount( pSession->GetSessionID() );
		if(  uiCount < uiMinValue )	
			uiMinValue = uiCount;

		vCandidate.push_back( pSession->GetActorHandle() );
	}

	for( UINT i=0 ; i<vCandidate.size() ; ++i )
	{
		if( _GetSelectZombieCount( vCandidate[i]->GetSessionID() ) == uiMinValue )
			vResult.push_back( vCandidate[i] );
	}
}

void CPvPZombieMode::InitZombieGroupData()
{
	g_pDataManager->GetGhoulGroup(GetGameRoom()->GetUserCountWithoutGM(), m_MutationGroup, m_nGhoulCount);
}

bool CPvPZombieMode::GetZombieActorData(TMonsterMutationData &Data)
{
	int nRandomOffset = _roomrand(GetGameRoom())%1000000000;
	int nTotalProb = 0;
	for (int i = 0; i < PvPCommon::Common::MonsterMutationColCount; i++)
	{
		if( nRandomOffset < m_MutationGroup.nProbability[i] + nTotalProb )
			return g_pDataManager->GetMonsterMutationData(m_MutationGroup.nMonsterID[i], Data);
		nTotalProb += m_MutationGroup.nProbability[i];
	}
	return false;
}

bool CPvPZombieMode::AddZombie(DnActorHandle hActor, const TMonsterMutationData &Data )
{
	if( !hActor || !hActor->IsPlayerActor() )
		return false;

	std::map<DWORD,DnActorHandle>::iterator itor = m_mZombieActor.find( hActor->GetUniqueID() );
	if( itor == m_mZombieActor.end() )
	{
		m_mZombieActor.insert(std::make_pair(hActor->GetUniqueID(), hActor));

		// Change to zombie team
		hActor->CmdChangeTeam( PvPCommon::Team::Zombie );
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->GetUserSession()->SetTeam( PvPCommon::Team::Zombie );
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->ToggleTransformMode(true, Data.nMutationID ); // Set actor zombie state

		// Random size
		_fpreset();
		int nScale = 100;
		int nMin = Data.nSizeMin;
		int nMax = Data.nSizeMax;
		if( nMin <= nMax ) {
			nScale = ( nMin + ( _rand(hActor->GetRoom())%( ( nMax + 1 ) - nMin ) ) );
		}
		float fScale = nScale/100.f;
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->SetScale( EtVector3( fScale, fScale, fScale ) );

		_ASSERT( GetGameRoom()->bIsPvPRoom() );
		static_cast<CDNPvPGameRoom*>(GetGameRoom())->SendSelectZombie( hActor, Data.nMutationID, true, false, nScale );
		return true;
	}
	return false;
}

bool CPvPZombieMode::DelZombie(DnActorHandle hActor, bool bRemoveStateBlow/*=false*/ )
{
	if( !hActor || !hActor->IsPlayerActor() )
		return false;

	std::map<DWORD,DnActorHandle>::iterator itor = m_mZombieActor.find( hActor->GetUniqueID() );
	if( itor != m_mZombieActor.end() )
	{
		// Change to Human team
		hActor->CmdChangeTeam( PvPCommon::Team::Human );
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->GetUserSession()->SetTeam( PvPCommon::Team::Human );
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->ToggleTransformMode(false);

		// Restore size
		static_cast<CDnPlayerActor*>(hActor.GetPointer())->SetScale( EtVector3( 1.f, 1.f, 1.f ) );

		_ASSERT( GetGameRoom()->bIsPvPRoom() );
		static_cast<CDNPvPGameRoom*>(GetGameRoom())->SendSelectZombie( hActor, -1, false, bRemoveStateBlow );
		m_mZombieActor.erase(itor);

		if( bRemoveStateBlow )
		{
			// Remove all remaining state effects applied by other players or mobs.
			hActor->RemoveAllBlowExpectPassiveSkill();
			hActor->GetStateBlow()->Process( 0, 0.f );
			hActor->CmdRefreshHPSP( hActor->GetMaxHP(), hActor->GetMaxSP() );
		}

		// Human state effect
		_AddHumanBlow( hActor );
		return true;
	}
	return false;
}

void CPvPZombieMode::InitializeZombie()
{
	InitZombieGroupData();

	if (GetZombieCount() >= m_nGhoulCount)
		return;

	_srand( GetGameRoom(), timeGetTime() );

	for (int i = 0; i < m_nGhoulCount; i++)
	{
		TMonsterMutationData Data;
		memset(&Data, 0, sizeof(TMonsterMutationData));
		if (GetZombieActorData(Data) == false)
		{
			_DANGER_POINT();
			continue;
		}

		std::vector<DnActorHandle> vResult;
		_GetZombieCandidate( vResult );
		if( vResult.empty() )
			return;

		DnActorHandle hZombie = vResult[_roomrand(GetGameRoom())%vResult.size()];
		_ChangeZombie( hZombie, Data );
	}
}

void CPvPZombieMode::AllReturnHuman()
{
	CDNUserSession * pSession = NULL;
	for (int i = 0; i < (int)GetGameRoom()->GetUserCount(); i++)
	{
		pSession = GetGameRoom()->GetUserData(i);
		if (pSession )
		{
			if( pSession->GetTeam() == PvPCommon::Team::Observer )
				continue;

			if( DelZombie(pSession->GetActorHandle()) == false )
			{
				_AddHumanBlow( pSession->GetActorHandle() );
			}
		}
		else
			_DANGER_POINT();
	}
}

