// =============================================================================
// IrineCore — modules/Hero-Ally/DnHeroAllyController.cpp
// Hero Ally (Geraint, Argenta, Velskud) stage accompaniment implementation.
// Extracted from main codebase (DnGameTask.cpp & DnMonsterActor.cpp) into module.
// =============================================================================
#include "stdafx.h"
#include "DnHeroAllyController.h"
#include "DnGameTask.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "DnWorld.h"
#include "DnActor.h"
#include "DnMonsterActor.h"
#include "DnNPCActor.h"
#include "DnPlayerActor.h"
#include "DNAggroSystem.h"
#include "DnStateBlow.h"
#include "DNGameDataManager.h"

void CDnMonsterActor::ScaleHelperCombatStats( int nTargetLevel, int nRefAttack )
{
	if( nTargetLevel < 1 ) nTargetLevel = 1;
	int nScaledAtk = ( nRefAttack > 0 ) ? (int)( nRefAttack * 1.25f ) : ( nTargetLevel * 120 );
	int nMinFloor = nTargetLevel * 90 + 120;
	if( nScaledAtk < nMinFloor ) nScaledAtk = nMinFloor;
	int nAtkMin = (int)( nScaledAtk * 0.88f );
	if( nAtkMin < 1 ) nAtkMin = 1;

	m_nLevel = nTargetLevel;
	m_BaseState.SetAttackPMin( nAtkMin );
	m_BaseState.SetAttackPMax( nScaledAtk );
	m_BaseState.SetAttackMMin( nAtkMin );
	m_BaseState.SetAttackMMax( nScaledAtk );
	m_BaseState.SetCritical( max( m_BaseState.GetCritical(), nTargetLevel * 200 ) );
#ifdef PRE_ADD_SKILLBUF_RENEW
	m_bCopiedFromSummoner = true;
#endif
	RefreshState( RefreshAll, ST_All );
	SetHP( GetMaxHP() );
}

void CDnGameTask::ResetHeroAllyAreaState()
{
	m_eHeroAllyType = HERO_ALLY_NONE;
	m_hHeroAllyActor.Identity();
	m_bHeroAllyAreaSpawned = false;
	m_fHeroAllySpawnDelay = 0.8f;
	m_fHeroAllyActionTimer = 0.f;
	m_fHeroAllyBuffTimer = 0.f;
	m_fHeroAllyMoveTimer = 0.f;
	m_nHeroAllyLineIndex = 0;
	m_bHeroAllyHasNaviTarget = false;
	m_HeroAllyDestOBB.Center = EtVector3( 0.f, 0.f, 0.f );
	m_HeroAllyDestOBB.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
	m_HeroAllyDestOBB.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
	m_HeroAllyDestOBB.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
	m_HeroAllyDestOBB.Extent[0] = 160.f;
	m_HeroAllyDestOBB.Extent[1] = 300.f;
	m_HeroAllyDestOBB.Extent[2] = 160.f;
}

void CDnGameTask::ResetHeroAllyRunState()
{
	m_bHeroAllyRunRolled = false;
	m_eRunHeroAllyType = HERO_ALLY_NONE;
	ResetHeroAllyAreaState();
}

void CDnGameTask::DespawnHeroAlly( bool bSendFarewell )
{
	if( m_eHeroAllyType == HERO_ALLY_NONE )
		return;

	if( bSendFarewell && m_hHeroAllyActor )
	{
		const WCHAR* wszName = L"Hero";
		const WCHAR* wszBye = L"Our paths cross again soon.";
		if( m_eHeroAllyType == HERO_ALLY_GERAINT )
		{
			wszName = L"Geraint";
			wszBye = L"Well fought! The stage is clear. Until we meet again.";
		}
		else if( m_eHeroAllyType == HERO_ALLY_VELSKUD )
		{
			wszName = L"Velskud";
			wszBye = L"Hmph. The stage is done. I have other matters to attend to.";
		}
		else if( m_eHeroAllyType == HERO_ALLY_ARGENTA )
		{
			wszName = L"Argenta";
			wszBye = L"Adequate work, human. Do not grow reliant on my power.";
		}

		m_hHeroAllyActor->CmdChatBalloon( wszBye );
		for( DWORD i = 0; i < GetUserCount(); ++i )
		{
			CDNUserSession* pSession = GetUserData( i );
			if( pSession && !pSession->bIsAIBot() )
				pSession->SendChat( CHATTYPE_PARTY, (int)wcslen( wszBye ), wszName, wszBye );
		}
	}

	if( m_hHeroAllyActor )
	{
		if( m_eHeroAllyType == HERO_ALLY_ARGENTA )
		{
			if( m_hHeroAllyActor->IsNpcActor() )
				static_cast<CDnNPCActor*>( m_hHeroAllyActor.GetPointer() )->StopNpcMove();

			UINT nUniqueID = m_hHeroAllyActor->GetUniqueID();
			for( DWORD i = 0; i < GetUserCount(); ++i )
			{
				CDNUserSession* pSession = GetUserData( i );
				if( pSession && !pSession->bIsAIBot() )
					pSession->SendLeaveNpc( nUniqueID );
			}
			DestroyNpc( nUniqueID );
			m_hHeroAllyActor->SetDestroy();
		}
		else if( m_hHeroAllyActor->IsMonsterActor() )
		{
			CDnMonsterActor* pHeroMon = static_cast<CDnMonsterActor*>( m_hHeroAllyActor.GetPointer() );
			pHeroMon->SetNaviDestination( NULL, 0 );
			if( !pHeroMon->IsDie() )
				pHeroMon->CmdSuicide( false, false );
		}
	}

	m_bHeroAllyHasNaviTarget = false;
	m_hHeroAllyActor.Identity();
	m_eHeroAllyType = HERO_ALLY_NONE;
}

void CDnGameTask::SpawnHeroAllyForCurrentArea( DnActorHandle hHuman )
{
	m_bHeroAllyAreaSpawned = true;

	if( !GetRoom() || !m_pWorld || !hHuman )
		return;
	if( GetRoom()->bIsPvPRoom() || GetRoom()->bIsFarmRoom() )
		return;
	if( m_nGameTaskType != GameTaskType::Normal )
		return;
	if( m_pWorld->GetMapType() != EWorldEnum::MapTypeDungeon )
		return;
	if( m_pWorld->GetMapSubType() != EWorldEnum::MapSubTypeNone )
		return;

	// Restrict hero allies strictly to Easy and Normal stages
	if( m_StageDifficulty != Dungeon::Difficulty::Easy && m_StageDifficulty != Dungeon::Difficulty::Normal )
		return;

	bool bIsFirstArea = false;
	if( !m_bHeroAllyRunRolled )
	{
		m_bHeroAllyRunRolled = true;
		bIsFirstArea = true;

		// Temporarily 100% trigger chance at the start of Easy/Normal stage runs
		const int kHeroSpawnChancePercent = 100;
		int nRoll = (int)( ( timeGetTime() ^ ( GetRoom()->GetRoomID() * 2654435761u ) ^ ( m_nMapTableID * 40503u ) ^ (DWORD)rand() ) % 100 );
		if( nRoll >= kHeroSpawnChancePercent )
		{
			m_eRunHeroAllyType = HERO_ALLY_NONE;
			return;
		}

		int nPick = (int)( ( ( timeGetTime() >> 3 ) ^ (DWORD)rand() ) % 3 );
		m_eRunHeroAllyType = ( nPick == 0 ) ? HERO_ALLY_GERAINT : ( ( nPick == 1 ) ? HERO_ALLY_VELSKUD : HERO_ALLY_ARGENTA );
	}

	eHeroAllyType eChosen = m_eRunHeroAllyType;
	if( eChosen == HERO_ALLY_NONE )
		return;

	EtVector3 vSpawnPos = *hHuman->GetPosition();
	EtVector3 vLook = *hHuman->GetLookDir();
	EtVector3 vRight( vLook.z, 0.f, -vLook.x );
	if( EtVec3LengthSq( &vRight ) < 0.0001f )
		vRight = EtVector3( 1.f, 0.f, 0.f );
	else
		EtVec3Normalize( &vRight, &vRight );

	vSpawnPos += vRight * 120.f - vLook * 60.f;
	vSpawnPos.y = m_pWorld->GetHeight( vSpawnPos );

	int nRefAtk = max( hHuman->GetAttackPMax(), hHuman->GetAttackMMax() );
	int nTargetLevel = max( 1, hHuman->GetLevel() );

	const WCHAR* wszNotice = NULL;
	const WCHAR* wszHeroName = NULL;
	const WCHAR* wszGreeting = NULL;
	const WCHAR* wszContinueLine = NULL;

	if( eChosen == HERO_ALLY_GERAINT || eChosen == HERO_ALLY_VELSKUD )
	{
		const int nMonsterID = ( eChosen == HERO_ALLY_GERAINT ) ? 21077 : 900329;
		EtVector3 vZero( 0.f, 0.f, 0.f );
		// AreaUniqueID = -1 & Team = 0: allies with players and never blocks stage clear gates
		DnActorHandle hHero = RequestGenerationMonsterFromMonsterID( nMonsterID, vSpawnPos, vZero, vZero, NULL, -1, 0 );
		if( !hHero || !hHero->IsMonsterActor() )
			return;

		CDnMonsterActor* pHeroMon = static_cast<CDnMonsterActor*>( hHero.GetPointer() );
		pHeroMon->SetHelperAlly( true );
		pHeroMon->EnableDropItem( false );
		pHeroMon->EnableNoDamage( true );
		pHeroMon->ScaleHelperCombatStats( nTargetLevel, nRefAtk );
		pHeroMon->SlaveOf( hHuman->GetUniqueID(), true, false, false, false );
		// High emergency unstuck range so heroes walk/run naturally instead of teleporting
		pHeroMon->SetAutoRecallRange( 2400 );

		m_hHeroAllyActor = hHero;
		m_eHeroAllyType = eChosen;
		m_fHeroAllyActionTimer = 0.6f;
		m_fHeroAllyBuffTimer = 0.f;
		m_bHeroAllyHasNaviTarget = false;

		if( eChosen == HERO_ALLY_GERAINT )
		{
			wszHeroName = L"Geraint";
			wszNotice = L"[Hero Encounter] Geraint has joined the battle to help you clear this stage!";
			wszGreeting = L"Stand firm! My Golden Sword will clear the path ahead!";
			wszContinueLine = L"Stay close! Let us clear the next area together.";
		}
		else
		{
			wszHeroName = L"Velskud";
			wszNotice = L"[Hero Encounter] Velskud has appeared to cut down your foes!";
			wszGreeting = L"Stay out of my way if you don't want to get caught in my blade.";
			wszContinueLine = L"More vermin ahead. Keep moving.";
		}
	}
	else
	{
		// Argenta (NpcID = 27, ActorID = 1026) - Silver Dragon Blessing & Silver Dragon Strike support
		const int nArgentaNpcID = 27;
		TNpcData* pNpcData = g_pDataManager ? g_pDataManager->GetNpcData( nArgentaNpcID ) : NULL;
		if( !pNpcData )
			return;

		float fRot = EtToDegree( atan2f( vLook.x, vLook.z ) );
		DnActorHandle hNpc = CreateNpc( nArgentaNpcID, pNpcData, vSpawnPos, fRot );
		if( !hNpc )
			return;

		UINT nUniqueID = hNpc->GetUniqueID();
		for( DWORD i = 0; i < GetUserCount(); ++i )
		{
			CDNUserSession* pSession = GetUserData( i );
			if( pSession && !pSession->bIsAIBot() )
				pSession->SendEnterNpc( nUniqueID, nArgentaNpcID, vSpawnPos.x, vSpawnPos.y, vSpawnPos.z, fRot );
		}

		m_hHeroAllyActor = hNpc;
		m_eHeroAllyType = HERO_ALLY_ARGENTA;
		m_fHeroAllyActionTimer = 2.0f;
		m_fHeroAllyBuffTimer = 0.2f;
		m_fHeroAllyMoveTimer = 0.35f;

		wszHeroName = L"Argenta";
		wszNotice = L"[Hero Encounter] Argenta has appeared! Silver Dragon Blessing & Strikes empower your party!";
		wszGreeting = L"Pitiful humans... I suppose I'll lend you the Silver Dragon's power just this once.";
		wszContinueLine = L"Try not to fall behind, human.";
	}

	if( bIsFirstArea )
	{
		if( m_hHeroAllyActor && wszGreeting )
			m_hHeroAllyActor->CmdChatBalloon( wszGreeting );

		for( DWORD i = 0; i < GetUserCount(); ++i )
		{
			CDNUserSession* pSession = GetUserData( i );
			if( pSession && !pSession->bIsAIBot() )
			{
				if( wszNotice )
					pSession->SendNotice( wszNotice, (int)wcslen( wszNotice ), 7 );
				if( wszHeroName && wszGreeting )
					pSession->SendChat( CHATTYPE_PARTY, (int)wcslen( wszGreeting ), wszHeroName, wszGreeting );
			}
		}
	}
	else if( m_hHeroAllyActor && wszContinueLine )
	{
		m_hHeroAllyActor->CmdChatBalloon( wszContinueLine );
	}
}

void CDnGameTask::ProcessHeroAlly( LOCAL_TIME LocalTime, float fDelta )
{
	if( !GetRoom() || !m_pWorld )
		return;

	// Despawn cleanly once the stage is cleared and prevent respawning during clear screen
	if( m_DungeonClearState != DCS_None )
	{
		if( m_eHeroAllyType != HERO_ALLY_NONE )
			DespawnHeroAlly( true );
		m_bHeroAllyAreaSpawned = true;
		m_eRunHeroAllyType = HERO_ALLY_NONE;
		return;
	}

	// Find primary living human player who has finished loading into gameplay
	DnActorHandle hHuman;
	bool bHumanInPlay = false;
	for( DWORD i = 0; i < GetUserCount(); ++i )
	{
		CDNUserSession* pSession = GetUserData( i );
		if( !pSession || pSession->bIsAIBot() || pSession->bIsGMTrace() )
			continue;
		DnActorHandle hActor = pSession->GetActorHandle();
		if( hActor && !hActor->IsDie() )
		{
			hHuman = hActor;
			if( pSession->GetState() == SESSION_STATE_GAME_PLAY )
				bHumanInPlay = true;
			break;
		}
	}

	// Spawn at the start of the stage once the human player has loaded into gameplay
	if( !m_bHeroAllyAreaSpawned )
	{
		if( !hHuman || !bHumanInPlay )
			return;
		m_fHeroAllySpawnDelay -= fDelta;
		if( m_fHeroAllySpawnDelay <= 0.f )
			SpawnHeroAllyForCurrentArea( hHuman );
		return;
	}

	if( m_eHeroAllyType == HERO_ALLY_NONE )
		return;

	if( !m_hHeroAllyActor || m_hHeroAllyActor->IsDie() )
	{
		m_bHeroAllyHasNaviTarget = false;
		m_hHeroAllyActor.Identity();
		m_eHeroAllyType = HERO_ALLY_NONE;
		return;
	}

	if( !hHuman )
		return;

	if( m_eHeroAllyType == HERO_ALLY_GERAINT || m_eHeroAllyType == HERO_ALLY_VELSKUD )
	{
		m_fHeroAllyActionTimer -= fDelta;
		if( m_fHeroAllyActionTimer > 0.f )
			return;
		m_fHeroAllyActionTimer = 0.6f;

		CDnMonsterActor* pHeroMon = static_cast<CDnMonsterActor*>( m_hHeroAllyActor.GetPointer() );
		if( !pHeroMon )
			return;

		// While actively fighting a target, clear navigation destination so combat AI runs smoothly
		if( pHeroMon->GetAggroTarget() )
		{
			if( m_bHeroAllyHasNaviTarget )
			{
				pHeroMon->SetNaviDestination( NULL, 0 );
				m_bHeroAllyHasNaviTarget = false;
			}
			return;
		}

		// Feed closest hostile monster within 1200 units so the hero engages immediately
		DnActorHandle hClosestEnemy;
		if( pHeroMon->GetAggroSystem() )
		{
			float fBestDistSq = 1200.f * 1200.f;
			EtVector3 vHumanPos = *hHuman->GetPosition();
			EtVector3 vHeroPos = *pHeroMon->GetPosition();

			for( DWORD i = 0; i < STATIC_INSTANCE(CDnActor::s_pVecProcessList).size(); ++i )
			{
				DnActorHandle hActor = STATIC_INSTANCE(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
				if( !hActor || !hActor->IsMonsterActor() || hActor->IsDie() || hActor->GetTeam() == 0 )
					continue;
				if( static_cast<CDnMonsterActor*>( hActor.GetPointer() )->IsHelperAlly() )
					continue;
				EtVector3 vEnemyPos = *hActor->GetPosition();
				EtVector3 d1 = vEnemyPos - vHumanPos;
				EtVector3 d2 = vEnemyPos - vHeroPos;
				float fDistSq = min( d1.x * d1.x + d1.z * d1.z, d2.x * d2.x + d2.z * d2.z );
				if( fDistSq < fBestDistSq )
				{
					fBestDistSq = fDistSq;
					hClosestEnemy = hActor;
				}
			}
		}

		if( hClosestEnemy )
		{
			if( m_bHeroAllyHasNaviTarget )
			{
				pHeroMon->SetNaviDestination( NULL, 0 );
				m_bHeroAllyHasNaviTarget = false;
			}
			pHeroMon->GetAggroSystem()->AddAggro( hClosestEnemy, 500, true );
		}
		else
		{
			// Follow player smoothly using persistent member SOBB (never a stack-local OBB!)
			EtVector3 vDiff = *hHuman->GetPosition() - *pHeroMon->GetPosition();
			float fDistSq = vDiff.x * vDiff.x + vDiff.z * vDiff.z;
			if( fDistSq > 340.f * 340.f && !pHeroMon->IsAttack() )
			{
				m_HeroAllyDestOBB.Center = *hHuman->GetPosition();
				if( !m_bHeroAllyHasNaviTarget )
				{
					pHeroMon->SetNaviDestination( &m_HeroAllyDestOBB, 100 );
					m_bHeroAllyHasNaviTarget = true;
				}
			}
			else if( fDistSq < 240.f * 240.f && m_bHeroAllyHasNaviTarget )
			{
				pHeroMon->SetNaviDestination( NULL, 0 );
				m_bHeroAllyHasNaviTarget = false;
				if( pHeroMon->IsMove() && !pHeroMon->IsAttack() )
					pHeroMon->CmdStop( "Stand" );
			}
		}
		return;
	}

	if( m_eHeroAllyType == HERO_ALLY_ARGENTA )
	{
		// Smoothly run behind the human player; only warp if separated by >= 2200 units
		m_fHeroAllyMoveTimer -= fDelta;
		if( m_fHeroAllyMoveTimer <= 0.f )
		{
			m_fHeroAllyMoveTimer = 0.45f;

			CDnNPCActor* pArgNpc = m_hHeroAllyActor->IsNpcActor() ? static_cast<CDnNPCActor*>( m_hHeroAllyActor.GetPointer() ) : NULL;
			EtVector3 vHumanPos = *hHuman->GetPosition();
			EtVector3 vArgPos = *m_hHeroAllyActor->GetPosition();
			EtVector3 vLook = *hHuman->GetLookDir();
			EtVector3 vRight( vLook.z, 0.f, -vLook.x );
			if( EtVec3LengthSq( &vRight ) < 0.0001f )
				vRight = EtVector3( 1.f, 0.f, 0.f );
			else
				EtVec3Normalize( &vRight, &vRight );

			EtVector3 vFollowPos = vHumanPos + vRight * 110.f - vLook * 70.f;
			vFollowPos.y = m_pWorld->GetHeight( vFollowPos );

			EtVector3 vDiffHuman = vHumanPos - vArgPos;
			float fHumanDistSq = vDiffHuman.x * vDiffHuman.x + vDiffHuman.z * vDiffHuman.z;

			if( fHumanDistSq >= 2200.f * 2200.f )
			{
				if( pArgNpc )
					pArgNpc->StopNpcMove();
				EtVector2 vLook2D( vLook.x, vLook.z );
				if( EtVec2LengthSq( &vLook2D ) < 0.0001f ) vLook2D = EtVector2( 0.f, 1.f );
				else EtVec2Normalize( &vLook2D, &vLook2D );
				m_hHeroAllyActor->CmdWarp( vFollowPos, vLook2D );
			}
			else if( pArgNpc )
			{
				EtVector3 vDiffFollow = vFollowPos - vArgPos;
				float fFollowDistSq = vDiffFollow.x * vDiffFollow.x + vDiffFollow.z * vDiffFollow.z;
				if( fHumanDistSq > 300.f * 300.f && fFollowDistSq > 140.f * 140.f )
				{
					const char* szMoveAction = ( pArgNpc->GetElementIndex( "Event_Run" ) != -1 ) ? "Event_Run" : "Event_10_Walk";
					pArgNpc->CmdMove( vFollowPos, szMoveAction, -1, 4.f );
				}
				else if( pArgNpc->IsNpcMoveActive() && fFollowDistSq <= 140.f * 140.f )
				{
					pArgNpc->CmdStop( "Stand" );
				}
			}
		}

		// Periodically apply Argenta's Silver Dragon Blessing to all living party members (non-overlapping 15s buff)
		m_fHeroAllyBuffTimer -= fDelta;
		if( m_fHeroAllyBuffTimer <= 0.f )
		{
			m_fHeroAllyBuffTimer = 15.5f;
			for( DWORD i = 0; i < GetUserCount(); ++i )
			{
				CDNUserSession* pSession = GetUserData( i );
				if( !pSession || pSession->bIsGMTrace() )
					continue;
				DnActorHandle hMember = pSession->GetActorHandle();
				if( !hMember || hMember->IsDie() )
					continue;

				hMember->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_002, 15000, "0.20", false, false );
				hMember->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_029, 15000, "0.20", false, false );
				hMember->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_025, 15000, "0.15", false, false );
				hMember->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_076, 15000, "0.15", false, false );
			}
		}

		// Every 4.5s: restore 15% HP/MP to party and unleash Silver Dragon Strike on up to 5 nearby monsters
		m_fHeroAllyActionTimer -= fDelta;
		if( m_fHeroAllyActionTimer <= 0.f )
		{
			m_fHeroAllyActionTimer = 4.5f;

			// Heal party HP/MP
			for( DWORD i = 0; i < GetUserCount(); ++i )
			{
				CDNUserSession* pSession = GetUserData( i );
				if( !pSession || pSession->bIsGMTrace() )
					continue;
				DnActorHandle hMember = pSession->GetActorHandle();
				if( !hMember || hMember->IsDie() )
					continue;
				INT64 nNewHP = min( hMember->GetMaxHP(), hMember->GetHP() + max( (INT64)1, hMember->GetMaxHP() * 15 / 100 ) );
				int nNewSP = min( hMember->GetMaxSP(), hMember->GetSP() + max( 1, hMember->GetMaxSP() * 15 / 100 ) );
				if( nNewHP > hMember->GetHP() || nNewSP > hMember->GetSP() )
					hMember->CmdRefreshHPSP( nNewHP, nNewSP );
			}

			// Gather up to 5 hostile monsters within 950 units of human or Argenta
			std::vector<DnActorHandle> vecTargets;
			EtVector3 vHumanPos = *hHuman->GetPosition();
			EtVector3 vArgPos = *m_hHeroAllyActor->GetPosition();
			const float kRangeSq = 950.f * 950.f;

			for( DWORD i = 0; i < STATIC_INSTANCE(CDnActor::s_pVecProcessList).size(); ++i )
			{
				DnActorHandle hActor = STATIC_INSTANCE(CDnActor::s_pVecProcessList)[i]->GetMySmartPtr();
				if( !hActor || !hActor->IsMonsterActor() || hActor->IsDie() || hActor->GetTeam() == 0 )
					continue;
				CDnMonsterActor* pMon = static_cast<CDnMonsterActor*>( hActor.GetPointer() );
				if( pMon->IsHelperAlly() || pMon->IsEnableNoDamage() || pMon->IsAppliedThisStateBlow( STATE_BLOW::BLOW_099 ) )
					continue;

				EtVector3 vEnemyPos = *hActor->GetPosition();
				EtVector3 d1 = vEnemyPos - vHumanPos;
				EtVector3 d2 = vEnemyPos - vArgPos;
				if( ( d1.x * d1.x + d1.z * d1.z ) <= kRangeSq || ( d2.x * d2.x + d2.z * d2.z ) <= kRangeSq )
				{
					vecTargets.push_back( hActor );
					if( vecTargets.size() >= 5 )
						break;
				}
			}

			if( !vecTargets.empty() )
			{
				// Face closest target and play Argenta's combat stance animation
				EtVector3 vToEnemy = *vecTargets[0]->GetPosition() - vArgPos;
				EtVector2 vFace( vToEnemy.x, vToEnemy.z );
				if( EtVec2LengthSq( &vFace ) > 0.001f )
				{
					EtVec2Normalize( &vFace, &vFace );
					m_hHeroAllyActor->Look( vFace, true );
				}
				if( m_hHeroAllyActor->GetElementIndex( "Event_Surprise_Fight_01" ) != -1 )
					m_hHeroAllyActor->CmdAction( "Event_Surprise_Fight_01" );

				static const WCHAR* s_wszArgentaLines[3] = {
					L"Silver Dragon Strike!",
					L"Perish before the Silver Dragon!",
					L"Try keeping up with my blessing, human."
				};
				if( ( m_nHeroAllyLineIndex % 2 ) == 0 )
					m_hHeroAllyActor->CmdChatBalloon( s_wszArgentaLines[ ( m_nHeroAllyLineIndex / 2 ) % 3 ] );
				++m_nHeroAllyLineIndex;

				int nBaseAtk = max( hHuman->GetAttackPMax(), hHuman->GetAttackMMax() );
				INT64 nStrikeBase = max( (INT64)( nBaseAtk * 1.45f ), (INT64)max( 1, hHuman->GetLevel() ) * 180 );

				for( size_t t = 0; t < vecTargets.size(); ++t )
				{
					DnActorHandle hEnemy = vecTargets[t];
					if( !hEnemy || hEnemy->IsDie() )
						continue;
					CDnMonsterActor* pEnemy = static_cast<CDnMonsterActor*>( hEnemy.GetPointer() );
					INT64 nDmg = nStrikeBase;
					// Cap per-strike damage against bosses/named monsters at 18% MaxHP so boss fights stay exciting
					if( pEnemy->GetGrade() >= CDnMonsterState::Boss && pEnemy->GetMaxHP() > 0 )
					{
						INT64 nBossCap = max( (INT64)1, pEnemy->GetMaxHP() * 18 / 100 );
						if( nDmg > nBossCap ) nDmg = nBossCap;
					}
					INT64 nNewHP = pEnemy->GetHP() - nDmg;
					if( nNewHP < 0 ) nNewHP = 0;
					pEnemy->SetHP( nNewHP );
					pEnemy->RequestHPMPDelta( CDnState::Light, -nDmg, hHuman->GetUniqueID(), false, true );
					if( pEnemy->GetHP() <= 0 )
						pEnemy->Die( hHuman );
				}
			}
		}
	}
}
