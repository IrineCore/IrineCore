
#include "stdafx.h"
#include "DnMonsterActor.h"
#include "DnWorld.h"
#include "DnWeapon.h"
#include "MAAiBase.h"
#include "DnTableDB.h"
#include "DnDropItem.h"
#include "DnPartyTask.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#include "DnBlow.h"
#include "DNGameRoom.h"
#include "DNUserSession.h"
#include "DnWorldTrapProp.h"
#include "DnStateBlow.h"
#include "navigationcell.h"
#include "navigationmesh.h"
#include "navigationpath.h"
#include "DnPlayerActor.h"
#include "DNLogConnection.h"
#include "MAWalkMovementNav.h"
#include "DNMissionSystem.h"
#include "DnProjectile.h"
#include "MAAiScript.h"
#include "DNMonsterAggroSystem.h"
#include "GameSendPacket.h"
#include "MAScanner.h"
#include "MasterRewardSystem.h"
#include "DnBlockBlow.h"
#include "DnParryBlow.h"
#include "DnCannonMonsterActor.h"
#if defined(PRE_ADD_WEEKLYEVENT)
#include "DNGameDataManager.h"
#endif

int CDnMonsterActor::s_nPositionRevisionTime = 3000;

CDnMonsterActor::CDnMonsterActor( CMultiRoom *pRoom, int nClassID )
:CDnActor( pRoom, nClassID )
,m_nMonsterClassID(0)
,m_AIDifficult( Dungeon::Difficulty::Easy )
,m_pszCanBumpActionName(NULL)
{
	CDnActionBase::Initialize( this );
	m_nDestroyTime						= 0;
	m_bTimeMonster						= false;
	m_fScale							= 1.f;
	m_nMonsterWeightTableID				= -1;
	m_eElementType						= ElementEnum_Amount;
	m_bNoDamage							= false;
	m_LastSendMoveMsg					= 0;
	m_fRotateResistance					= 1.f;
	m_nPartyComboCount					= 0;
	m_nPartyComboDelay					= 0;
	m_bIsTriggerMonster					= false;
	m_iTriggerRandomSeed				= 0;
	m_hProp								= CDnWorldProp::Identity();
	m_uiForcePositionRevisionTick		= 0;
	m_uiPrevForcePositionRevisionTick	= 0;
	m_bEnableDropItem = true;
	m_nBirthAreaHandle = -1;
	m_dwSummonerActorID = 0;
	m_bSuicideWhenSummonerDie =	false;
	m_bFollowSummonerStage = false;
	m_iSummonGroupID = 0;
	m_fLimitSummonerDistanceSQ = 0.0f;
	m_bSummoned = false;
	m_bReCreatedFollowStageMonster = false;

#ifdef PRE_ADD_MONSTER_CATCH
	m_iCatchedActorActionIndex = 0;
#endif // #ifdef PRE_ADD_MONSTER_CATCH

#if defined( PRE_FIX_MOVEBACK )
	m_bNearMoveBack = false;
	m_bPrevMoveBack = false;
#endif

	m_nAutoRecallRange = 0;

#if defined(PRE_FIX_51048)
	m_EnablePassiveStateEffect = false;
#endif // PRE_FIX_51048

	m_bChangeAxisOnFinishAction = false;
	m_isPuppetSummonMonster = false;
	m_bIsHelperAlly = false;
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	m_nEventAreaUniqueID = -1;
#endif // PRE_MOD_DARKLAIR_RECONNECT
#ifdef PRE_ADD_TRANSFORM_MONSTER_ACTOR
	m_nSwapActorID = -1;
#endif
}

CDnMonsterActor::~CDnMonsterActor()
{
	//Cancel monster aura skill.
	if( IsEnabledAuraSkill() )
		OnSkillAura( m_hAuraSkill, false );

	SAFE_DELETE_VEC( m_VecDropItemList );
}

bool CDnMonsterActor::Initialize()
{
	CalcMonsterWeightIndex();

	CDnMonsterState::Initialize( m_nClassID );
	MAAiReceiver::Initialize( m_nMonsterClassID, GetAIFileName().c_str() );
	CDnActor::Initialize();

	if( m_fWeight == 0.f ) m_fRevisionWeight = 0.f;
	else m_fRevisionWeight = m_fWeight + ( ( m_fScale - 1.f ) * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ScaleWeightValue ) );

	GenerationDropItem();

	if( m_hObject ) {
		m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
		m_hObject->SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) | COLLISION_GROUP_DYNAMIC( 2 ) );
	}

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	int nStrElementType = pSox->GetFieldFromLablePtr( m_nMonsterClassID, "_Element_Str_Type" )->GetInteger();
	if( nStrElementType != -1 )
		m_eElementType = (ElementEnum)nStrElementType;

	_ASSERT( m_pAggroSystem == NULL );
	m_pAggroSystem = new CDNMonsterAggroSystem( GetActorHandle() );
	_ASSERT( m_pAggroSystem != NULL );

	return true;
}

void CDnMonsterActor::ProcessLook( LOCAL_TIME LocalTime, float fDelta )
{
	if( bIsAILook() && bIsTurnOnAILook() )
	{
		Look( *m_pAi->GetAILook() );
	}
	else
	{
		bool bNaviMode		= IsNaviMode();
//		bool bLockTarget	= IsSignalRange( STE_LockTargetLook );

#if defined( PRE_MOD_LOCK_TARGET_LOOK )
		if( false == bNaviMode && 0 != m_nLockLookEventArea && true == m_bLockLookTarget )
		{
			if( GetLookTarget() )
				ResetLook();
			if( NULL != GetGameRoom() && NULL != GetGameRoom()->GetWorld() )
			{
				std::vector<CEtWorldEventArea *> vecArea;
				GetGameRoom()->GetWorld()->FindEventAreaFromCreateUniqueID( m_nLockLookEventArea, &vecArea );

				if( false == vecArea.empty() )
				{
					EtVector3 vAreaPosition = vecArea[0]->GetOBB()->Center;
					EtVector3 * vMonsterPosition = GetPosition();

					EtVector2 vDir;
					vDir.x = vAreaPosition.x - vMonsterPosition->x;
					vDir.y = vAreaPosition.z - vMonsterPosition->z;
					EtVec2Normalize( &vDir, &vDir );

					CmdLook( vDir );
				}
			}
		}
		else 
#endif	// #if defined( PRE_MOD_LOCK_TARGET_LOOK )
		if( !bNaviMode && m_bLockLookTarget && GetAggroTarget() ) 
		{
			if( !GetLookTarget() && GetLookTarget() != GetAggroTarget() ) 
				LookTarget( GetAggroTarget() );
		}
		else 
		{
			if( GetLookTarget() )
				ResetLook();
		}
	}
	if( !IsSignalRange( STE_RotateResistance ) ) m_fRotateResistance = 1.f;
}

void CDnMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_bLockLookTarget					= false;
	m_pszCanBumpActionName				= NULL;
	m_uiPrevForcePositionRevisionTick	= m_uiForcePositionRevisionTick;
	m_uiForcePositionRevisionTick		= 0;

#if defined( PRE_MOD_LOCK_TARGET_LOOK )
	m_nLockLookEventArea = 0;
#endif	// #if defined( PRE_MOD_LOCK_TARGET_LOOK )

	CDnActor::Process( LocalTime, fDelta );
	PROFILE_TIME_TEST_BLOCK_START( "CDnMonsterActor::Process" );
	ProcessLook( LocalTime, fDelta );
	SetMoveVectorX( m_Cross.m_vXAxis );
	SetMoveVectorZ( m_Cross.m_vZAxis );

	if ( m_bTimeMonster && !IsDie() )
	{
		m_nDestroyTime -= (LOCAL_TIME)(fDelta*1000.0f);

		if( m_nDestroyTime <= 0 )
			CmdSuicide( false, false );
	}

	// Check if set to die together when summoner dies, and die together if so.
	if( m_bSuicideWhenSummonerDie )
	{
		// When the summoner's actor object has disappeared or died.
		if( !m_hSummonerPlayerActor || 
			m_hSummonerPlayerActor->IsDie() )
		{
			CmdSuicide( false, false );
		}
	}

	// If configured to die when separated beyond a certain distance, handle it accordingly.
	if( 0.0f < m_fLimitSummonerDistanceSQ )
	{
		if( m_hSummonerPlayerActor && false == m_hSummonerPlayerActor->IsDie() )
		{
			EtVector3 vSummonerPos = *m_hSummonerPlayerActor->GetPosition();
			float fNowDistanceSQ = EtVec3LengthSq( &(vSummonerPos - *GetPosition()) );
			if( m_fLimitSummonerDistanceSQ < fNowDistanceSQ )
			{
				CmdSuicide( false, false );
			}
		}
	}

	ProcessPositionRevision( fDelta );
	ProcessPartyCombo( LocalTime, fDelta );

#ifdef PRE_ADD_MONSTER_CATCH
	ProcessCatchActor( LocalTime, fDelta );
#endif // #ifdef PRE_ADD_MONSTER_CATCH

	Process_AutoRecallRange();

	PROFILE_TIME_TEST_BLOCK_END();
}

void CDnMonsterActor::ProcessAI(LOCAL_TIME LocalTime, float fDelta)
{
	if( MATransAction::GetGameRoom() ) 
	{
		MAAiReceiver::Process( LocalTime, fDelta );
	}
}

void CDnMonsterActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{
	int nSeed = CRandom::Seed(GetRoom());
	_srand( GetRoom(), nSeed );
	INT64 nTemp = GetHP();

	CDnActor::OnDamage( pHitter, HitParam, pHitStruct );
	INT64 nDamage = nTemp - GetHP();

	switch( pHitter->GetDamageObjectType() ) 
	{
		case DamageObjectTypeEnum::Actor:
			{
				// Aggro Process
				//DnActorHandle hActor = dynamic_cast<CDnActor*>(pHitter)->GetMySmartPtr();
				DnActorHandle hActor = pHitter->GetActorHandle();

				if( m_pAggroSystem )
					m_pAggroSystem->OnDamageAggro( hActor, HitParam, (int)nDamage );

				ResetCustomAction();
				// For weapons like arrows that fire while playing, set to Idle on Hit.
				if( GetWeapon() ) {
					if( GetWeapon()->GetElementIndex( "Idle" ) != -1 )
						GetWeapon()->SetActionQueue( "Idle" );
				}
				if( HitParam.HitType != CDnWeapon::Defense )
					OnPartyCombo( hActor, pHitStruct->nPartyComboDelay );
			}
			break;
		case DamageObjectTypeEnum::Prop:
			break;
	}
	RequestDamage( pHitter, nSeed, nDamage );
}

void CDnMonsterActor::OnDie( DnActorHandle hHitter )
{
	CDnActor::OnDie( hHitter );
	CDnPartyTask *pTask = (CDnPartyTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "PartyTask" );
	if( !pTask ) return;

#ifdef PRE_FIX_REMOVE_AURA_ONDIE
	if (IsEnabledAuraSkill())
		OnSkillAura(m_hAuraSkill, false);
#endif

	if( m_hSummonerPlayerActor )
	{
		_ASSERT( m_hSummonerPlayerActor->IsPlayerActor() );
		if( m_hSummonerPlayerActor->IsPlayerActor() )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hSummonerPlayerActor.GetPointer());
			pPlayerActor->OnDieSummonedMonster( GetMySmartPtr() );
		}
	}

	if( m_bIsHelperAlly )
	{
#ifdef PRE_ADD_MONSTER_CATCH
		if( false == m_vlCatchedActors.empty() )
			ReleaseAllActor();
#endif
		return;
	}

	CDnPlayerActor* pMasterPlayerActor = NULL;
	if (hHitter)
	{
		if (hHitter->IsPlayerActor())
			pMasterPlayerActor = static_cast<CDnPlayerActor*>(hHitter.GetPointer());
		else if (hHitter->IsMonsterActor())
		{
			CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hHitter.GetPointer());
			if (pMonster && pMonster->IsCannonMonsterActor())
			{
				CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>(hHitter.GetPointer());
				if (pCannonMonster && pCannonMonster->GetMasterPlayerActor() && pCannonMonster->GetMasterPlayerActor()->IsPlayerActor())
					pMasterPlayerActor = static_cast<CDnPlayerActor*>(pCannonMonster->GetMasterPlayerActor().GetPointer());
			}
		}
	}

	int nTotalCount = (int)pTask->GetRoom()->GetUserCount();
	int nPartyCount = (int)pTask->GetRoom()->GetRealUserCountWithoutGM();
	if( nPartyCount < 1 ) nPartyCount = 1;

	// Distribute EXP and add money..
//	float fValue = ( ( 1.f + ( 0.9f * ( nPartyCount - 1 ) ) ) / nPartyCount );
	float fExpPenalty;
	float fExp;
	int nValue;
	float fCompleteExp;
	float fItemExp = 0.f;
	float fGuildRewardExp = 0.f;

	float fDeadDurabilityRevision = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MonsterDeadDurabilityRevision );
	int nDeadDurability = (int)( ( GetDeadDurability() * ( 1.f + ( fDeadDurabilityRevision * ( nPartyCount - 1 ) ) ) ) / nPartyCount );
	int nFinalDeadDurability = nDeadDurability;
	for( int i=0; i<nTotalCount; i++ ) 
	{
		CDNUserSession *pSession = pTask->GetRoom()->GetUserData(i);
		if( !pSession )
			continue;
		// Prevent GM intrusion from gaining EXP.
		if( pSession->bIsGMTrace() || pSession->bIsAIBot() )
			continue;

		DnActorHandle hActor = pSession->GetActorHandle();
		if( !hActor ) continue;

		pSession->GetEventSystem()->OnEvent( EventSystem::OnKillMonster2, 1, EventSystem::MonsterID, GetMonsterClassID() );

		// EXP on death
		nValue = hActor->GetLevel() - GetLevel();
		fExpPenalty = 1.f - ( ( pow( max( nValue - 1, 0.f ), CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ExpPenaltyValue1 ) ) ) *
						CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ExpPenaltyValue2 ) );
		fExpPenalty = max( fExpPenalty, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ExpPenaltyMin ) );

		float fExpPartyBonus = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ExpPartyBonus );
		fExp = GetDeadExperience() * ( 1.f + ( fExpPartyBonus * ( nPartyCount - 1 ) ) ) / nPartyCount * fExpPenalty;
		float fEventBonusExp = pTask->GetRoom()->GetEventExpWhenMonsterDie(fExp, pSession->GetFriendBonus(), pSession->GetClassID(), pSession->GetUserJob());
#ifdef PRE_ADD_BEGINNERGUILD
		fEventBonusExp += (pTask->GetRoom()->GetPartyStructData().bPartyBeginnerGuild == true && pSession->CheckBegginerGuild()) ? (float)((fExp * (float)((float)(BeginnerGuild::Common::PartyBonusRate)/100)) + 0.5f) : 0;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
		float fIncExpRate = 0.0f;
		if (hActor && hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_255))
		{
			DNVector(DnBlowHandle) vlBlows;
			hActor->GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_255, vlBlows);
			{
				int nCount = (int)vlBlows.size();
				for (int i = 0; i < nCount; ++i)
				{
					DnBlowHandle hBlow = vlBlows[i];
					if (hBlow && hBlow->IsEnd() == false)
					{
						fIncExpRate += hBlow->GetFloatValue();
					}
				}
			}
		}
		
		float fAddBlowExp = fExp * fIncExpRate;
		fEventBonusExp += fAddBlowExp;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined(PRE_ADD_WEEKLYEVENT)
		if (CDnWorld::GetInstance(GetRoom()).GetMapSubType() != EWorldEnum::MapSubTypeNest){
			int nThreadID = GetGameRoom()->GetServerID();

			if (pTask->GetRoom()->GetPartyUpkeepCount() > 0){
				float fEventValue = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, pSession->GetClassID(), WeeklyEvent::Event_6, nThreadID);
				if (fEventValue != 0)
					fEventBonusExp += fEventValue;
			}

			if (pSession->GetFriendBonus()){
				float fEventValue = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, pSession->GetClassID(), WeeklyEvent::Event_7, nThreadID);
				if (fEventValue != 0)
					fEventBonusExp += fEventValue;
			}
		}
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)

		// On completion, process EXP and then check death.
		fCompleteExp = GetCompleteExperience() * ( 1.f + ( fExpPartyBonus * ( nPartyCount - 1 ) ) ) / nPartyCount * fExpPenalty;

		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());

		if( pPlayer->GetAddExp() > 0.f )
			fItemExp = GetDeadExperience()*pPlayer->GetAddExp();
		float fGuildCompleteExp = 0.f;
		int nGuildRewardValue = pSession->GetGuildRewardItemValue(GUILDREWARDEFFECT_TYPE_EXTRAEXP);
		if( nGuildRewardValue > 0 )
		{
			fGuildRewardExp = (float)(GetDeadExperience() * (nGuildRewardValue * 0.01));
			fGuildCompleteExp = (float)(fCompleteExp * (nGuildRewardValue * 0.01));
		}
		else
			fGuildRewardExp = 0;

		fCompleteExp += ( (fCompleteExp*pPlayer->GetAddExp()) + fGuildCompleteExp );
		pPlayer->AddCompleteExperience((int)fCompleteExp);

		if (hActor->IsDie()) 
			continue;

		MasterSystem::CRewardSystem* pMasterRewardSystem = pSession->GetGameRoom()->GetMasterRewardSystem();
		if( pMasterRewardSystem )
		{
			float fBonusRate = pMasterRewardSystem->GetExpRewardRate( pSession );
			if( fBonusRate > 0.f )
			{
				fEventBonusExp += (fExp*fBonusRate);
				int iMasterAddExp = 0;
				iMasterAddExp = pMasterRewardSystem->GetMasterSystemAddExp(pSession, fExp, false);
				fEventBonusExp += iMasterAddExp;
			}
		}
		if( pSession->GetPeriodExpItemRate() > 0 )
		{
			fEventBonusExp += (fExp* (float)(pSession->GetPeriodExpItemRate()/100.0f));
		}
		float fPcBangExp = 0.0f;
		if (pSession->IsPCBang() && pSession->GetPcBangBonusExp() > 0 && fExp > 0.f )
			fPcBangExp = (float)((fExp * (float)((float)(pSession->GetPcBangBonusExp())/100)) + 0.5f);

		float fPromotionBonusExp = 0.0f;
		if( fExp > 0.f )
			fPromotionBonusExp = (float)((fExp * (float)((float)(pSession->GetPromotionValue(PROMOTIONTYPE_MONSTERKILL))/100)) + 0.5f);

		float fVIPExp = 0.0f;
#if defined(PRE_ADD_VIP)
		if (pSession->IsVIP() && pSession->GetVIPBonusExp() > 0 && fExp > 0.f )
			fVIPExp = (float)((fExp * (float)((float)(pSession->GetVIPBonusExp())/100)) + 0.5f);
#endif	// #if defined(PRE_ADD_VIP)

		TExpData ExpData;		
		ExpData.set( fExp, fEventBonusExp, fPcBangExp, fVIPExp, fPromotionBonusExp, fItemExp, fGuildRewardExp );
		pPlayer->CmdAddExperience( ExpData, DBDNWorldDef::CharacterExpChangeCode::Dungeon, pPlayer->GetUserSession()->GetPartyID() );

		// Max-level bonus EXP. Be sure to add it here as well whenever added above.
		pPlayer->UpdateMaxLevelGainExperience( (int)( fExp + fEventBonusExp + fPcBangExp ) ); 

		int nGuildRewardItemValue = pSession->GetGuildRewardItemValue(GUILDREWARDEFFECT_TYPE_REDUCEDURABILITYRATIO);
		if( nGuildRewardItemValue > 0 )
			nFinalDeadDurability -= (int)(nFinalDeadDurability * nGuildRewardItemValue * 0.01);
		// Reduce party member durability
		pPlayer->OnDecreaseEquipDurability( nFinalDeadDurability, false );

		if (pMasterPlayerActor)
		{
			pSession->GetEventSystem()->OnEvent( EventSystem::OnKillMonster, 3,
				EventSystem::MonsterID, GetMonsterClassID(),
				EventSystem::MonsterGrade, GetGrade(),
				EventSystem::MonsterRaceID, GetRaceID());
		}
	}
//	pTask->GetRoom()->AddCompleteExperience( GetCompleteExperience() );

	if( m_bEnableDropItem )
		DropItems();

	if( IsSlowByDie() ) {
		CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
		if( pGameTask ) pGameTask->RequestChangeGameSpeed( 0.2f, 3000 );
	}

	if (m_pAggroSystem)
	{
		int wholeAggro = m_pAggroSystem->GetAggroSum();

		std::list<CDNAggroSystem::AggroStruct>& aggroList = m_pAggroSystem->GetAggroList();
		std::list<CDNAggroSystem::AggroStruct>::iterator iter = aggroList.begin();
		for( ; iter!=aggroList.end() ; ++iter)
		{
			CDNAggroSystem::AggroStruct& curAggro = *iter;
			if (curAggro.hActor && curAggro.hActor->IsPlayerActor())
			{
				CDnPlayerActor* pActor = static_cast<CDnPlayerActor*>(curAggro.hActor.GetPointer());
				pActor->UpdateAssistScore(curAggro.iAggro, wholeAggro);
			}
		}
	}
	if (GetGameRoom())
	{
		GetGameRoom()->OnDie(GetActorHandle(), hHitter);
	}

#ifdef PRE_ADD_MONSTER_CATCH
	if( false == m_vlCatchedActors.empty() )
	{
		ReleaseAllActor();
	}
#endif // #ifdef PRE_ADD_MONSTER_CATCH
}

void CDnMonsterActor::ProcessDie( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActor::ProcessDie( LocalTime, fDelta );
	if( !IsDie() || IsDestroy() )
		return;

	if( CDnActionBase::GetFPS() <= 0.f )
	{
		ResetPlaySpeed();
		if( m_iFrameStopRefCount > 0 )
			m_iFrameStopRefCount = 0;
		UpdateFPS();
	}
}

void CDnMonsterActor::DropItems()
{
	DNVector(CDnItem::RealDropItemStruct) vRealDropItemList;
	vRealDropItemList.reserve( m_VecDropItemList.size() );

	int iPercent = HackPenanty::Common::MaxRate - GetGameRoom()->GetHackPenalty();
	for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) 
	{
		CDnItem::DropItemStruct *pStruct = &m_VecDropItemList[i];
		int nRotate = (int)( ( ( pStruct->nSeed % 360 ) / (float)m_VecDropItemList.size() ) * i );

		if( _rand(GetRoom())%HackPenanty::Common::MaxRate < iPercent )
		{
			vRealDropItemList.push_back( CDnItem::RealDropItemStruct(*pStruct, nRotate) );
		}
		else
		{
			g_Log.Log(LogType::_DROPITEMPENALTY, L"[%d] RoomID=%d HackPenalty=%d ItemID=%d Miss!!!\n", g_Config.nManagedID, GetGameRoom()->GetRoomID(), GetGameRoom()->GetHackPenalty(), pStruct->nItemID );
		}
	}

#if defined( PRE_FATIGUE_DROPITEM_PENALTY )
	if( CDnWorld::GetInstance(GetRoom()).GetMapSubType() == EWorldEnum::MapSubTypeNone)	
	{
		int iFatigueDropPercent = GetGameRoom()->GetFatigueDropRate();
		if( iFatigueDropPercent < 100 )
		{
			for( UINT i=0 ; i<vRealDropItemList.size() ; ++i )
			{
				if( _roomrand(GetRoom())%100 >= iFatigueDropPercent )
				{
					vRealDropItemList.erase( vRealDropItemList.begin()+i );
					--i;
				}
			}
		}
	}	
#endif // #if defined( PRE_FATIGUE_DROPITEM_PENALTY )

	EtVector3 vPos = *GetPosition();

	for( DWORD i=0 ; i<vRealDropItemList.size() ; ++i )
	{
		CDnItem::RealDropItemStruct* pStruct = &vRealDropItemList[i];
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		CDnDropItem::DropItem( GetRoom(), vPos, pStruct->dwUniqueID, pStruct->nItemID, pStruct->nSeed, pStruct->cOption, pStruct->nCount, pStruct->nRotate, -1, pStruct->nEnchantID );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		CDnDropItem::DropItem( GetRoom(), vPos, pStruct->dwUniqueID, pStruct->nItemID, pStruct->nSeed, pStruct->cOption, pStruct->nCount, pStruct->nRotate );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	}

	if( !vRealDropItemList.empty() )
	{
		for( DWORD i=0; i<GetGameRoom()->GetUserCount(); i++ ) 
		{
			CDNUserSession* pGameSession = GetGameRoom()->GetUserData(i);
			if( pGameSession && pGameSession->GetState() == SESSION_STATE_GAME_PLAY )
				SendGameDropItemList( pGameSession, vPos, vRealDropItemList );
		}
	}

	SAFE_DELETE_VEC( m_VecDropItemList );
}

void CDnMonsterActor::OnDrop( float fCurVelocity )
{
	if( IsAir() ) {
		if( !IsHit() ) {
			char szStr[64];
			sprintf_s( szStr, "%s_Landing", GetCurrentAction() );
			if( IsExistAction( szStr ) )
			{
				SetActionQueue( szStr, 0, 2.f, 0.f, true, false );

				// Set chain action when currently using a skill.
				if( m_hProcessSkill )
				{
					m_hProcessSkill->AddUseActionName( szStr );
					m_hProcessSkill->OnChainInput( szStr );
				}
			}

			SetMovable( false );
		}
		else if( !IsDie() ) {
			std::string szAction;
			float fBlendFrame = 2.f;
			// If falling speed is 10 or more, bounce up once more with bigBounce.
			if( fCurVelocity < -6.f && m_HitParam.vVelocity.y != 0.f && abs(m_HitParam.vVelocity.y) > 0.1f ) {
				if( m_HitParam.vVelocity.y > 0.f ) {
					m_HitParam.vVelocity.y *= 0.6f;
					SetVelocityY( m_HitParam.vVelocity.y );
				}
				else { // If the acceleration points toward the ground from the start, invert it.
					m_HitParam.vVelocity.y *= -0.6f;
					if( m_HitParam.vResistance.y > 0.f )
						m_HitParam.vResistance.y *= -1.f;
					SetVelocityY( m_HitParam.vVelocity.y );

					if( m_HitParam.vVelocity.y > 0 && m_HitParam.vResistance.y <= 0 )
						SetResistanceY( -15.0f );
					else
						SetResistanceY( m_HitParam.vResistance.y );

				}
				szAction = "Hit_AirBounce";
			}
			else {
				szAction = "Down_SmallBounce";
				fBlendFrame = 0.f;
			}
			SetActionQueue( szAction.c_str(), 0, fBlendFrame, 0.f, true, false );
		}
	}
}

void CDnMonsterActor::OnStop( EtVector3 &vPosition )
{
	if( IsDie() ) return;
	if( IsMove() ) CmdStop( "Stand" );
	
	if( m_pAi ) m_pAi->OnStop(vPosition);
}

void CDnMonsterActor::OnBeginNaviMode()
{
	if( IsDie() ) return;
	std::string strPrevAction = m_hActor->GetCurrentAction();

	bool bPrevIsMove = false;
	if( IsMove() && !IsNaviMode() )
	{
		bPrevIsMove = true;
		CmdStop( "Stand" );
	}

	if( m_pAi )
		m_pAi->OnBeginNaviMode( strPrevAction.c_str(), bPrevIsMove );
}

void CDnMonsterActor::CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	if( !IsMovable() ) return;

	MovePos( vPos, true );
	if( SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, true ) == false ) return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	int nActionIndex = GetElementIndex( szActionName );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	Send( eActor::SC_CMDMOVE, &Stream );

	ResetPositionRevision();
}

void CDnMonsterActor::CmdMove( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	if( !IsMovable() ) 
		return;

	bool bIsNaviMode = m_hActor->IsNaviMode();
	MoveTarget( hActor, fMinDistance );
	if( SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, !bIsNaviMode ) == false ) 
		return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	int nActionIndex = GetElementIndex( szActionName );
	DWORD dwUniqueID = hActor->GetUniqueID();
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &dwUniqueID, sizeof(DWORD) );
	Stream.Write( &fMinDistance, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	Send( eActor::SC_CMDMOVETARGET, &Stream );

	ResetPositionRevision();
}

bool CDnMonsterActor::_bIsCheckVaildPosition( EtVector3& vTargetPos )
{
	// Check target navigation range
	CEtWorldGrid* pGrid = INSTANCE(CDnWorld).GetGrid();
	if( IsMovable() && pGrid )
	{
		NavigationMesh* pNavMesh = pGrid->GetNavMesh( vTargetPos );
		if( pNavMesh )
		{
			NavigationCell* pCurCell = pNavMesh->FindClosestCell( vTargetPos );
			if( pCurCell && pCurCell->IsPointInCellCollumn( vTargetPos ) )
				return true;
		}
	}

	if( !IsDie() ) CmdStop( "Stand" );
	return false;
}

void CDnMonsterActor::_SendCmdMoveNavi( EtVector3& vTargetPos, float fMinDistance, const char* szActionName, int nLoopCount )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	int nActionIndex = GetElementIndex( szActionName );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &vTargetPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &fMinDistance, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	Send( eActor::SC_CMDMOVETARGET_NAVI, &Stream );
}

void CDnMonsterActor::CmdMoveNavi( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	// Check if target is outside navigation
	if( !_bIsCheckVaildPosition( *hActor->GetPosition() ) )
		return;
	//
	bool bAlreadyNaviMode = m_hActor->IsNaviMode();
	MoveTargetNavi( hActor, fMinDistance, szActionName );	
	if( !m_hActor->IsNaviMode() )
		return CmdStop( "Stand" );
	if( (bAlreadyNaviMode && GetWayPointSize() <= 2) )
		return CmdStop( "Stand" );
	if( SetActionQueue( szActionName, -1, fBlendFrame, 0.f, !bAlreadyNaviMode ) == false ) 
		return;
	// Packet
	_SendCmdMoveNavi( *hActor->GetPosition(), fMinDistance, szActionName, nLoopCount );

	ResetPositionRevision();
}

void CDnMonsterActor::CmdMoveNavi( EtVector3& vTargetPos, float fMinDistance, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	// Check if target is outside navigation
	if( !_bIsCheckVaildPosition( vTargetPos ) )
		return;
	//
	bool bAlreadyNaviMode = m_hActor->IsNaviMode();
	MoveTargetNavi( vTargetPos, fMinDistance, szActionName );
	if( !m_hActor->IsNaviMode() )
		return CmdStop( "Stand" );
	if( SetActionQueue( szActionName, -1, fBlendFrame, 0.f, !bAlreadyNaviMode ) == false ) 
		return;
	// Packet
	_SendCmdMoveNavi( vTargetPos, fMinDistance, szActionName, nLoopCount );

	ResetPositionRevision();
}

void CDnMonsterActor::CmdStop( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame )
{
	if( SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame, true ) == false ) return;

	ResetMove();
	ResetLook();

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	int nActionIndex = GetElementIndex( szActionName );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

	Send( eActor::SC_CMDSTOP, &Stream );

	ResetPositionRevision();
}

void CDnMonsterActor::CmdAction( const char *szActionName, int nLoopCount /*= 0*/, float fBlendFrame /*= 3.f*/, 
								 bool bCheckOverlapAction /*= true*/, bool bFromStateBlow /*= false */, bool bSkillChain/* = false*/ )
{
	// Status effect disabling MP-consuming skills prevents monsters from using actions with attack state. (#13032)
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_078 ) )
	{
		if( IsAttack( szActionName ) )
			return;
	}

	if( SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, bCheckOverlapAction ) == false ) 
		return;

	int		nActionIndex	= GetElementIndex( szActionName );
	bool	bAILook			= bIsAILook();

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );
	Stream.Write( &bAILook, sizeof(bool) );
	if( bAILook )
		Stream.Write( m_pAi->GetAILook(), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	else
		Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &bFromStateBlow, sizeof(bool) );	

#if defined( PRE_FIX_MOVEBACK )
	bool bNearMoveBack = IsPrevMoveBack();
	Stream.Write( &bNearMoveBack, sizeof(bool));
#endif

	Send( eActor::SC_CMDACTION, &Stream );

	ResetPositionRevision();
}

void CDnMonsterActor::CmdLook( EtVector2 &vVec, bool bForce )
{
	Look( vVec, bForce );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &vVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &bForce, sizeof(bool) );

	Send( eActor::SC_CMDLOOK, &Stream );
}

void CDnMonsterActor::CmdLook( DnActorHandle hActor, bool bLock )
{
	if( bLock ) LookTarget( hActor );
	else {
		if( !hActor ) ResetLook();
		else {
			EtVector2 vVec;
			vVec.x = hActor->GetPosition()->x - GetPosition()->x;
			vVec.y = hActor->GetPosition()->z - GetPosition()->z;
			EtVec2Normalize( &vVec, &vVec );
			Look( vVec );
		}
	}

	BYTE pBuffer[128];
	CMemoryStream Stream( pBuffer, 128 );

	DWORD dwUniqueID = ( hActor ) ? hActor->GetUniqueID() : -1;
	Stream.Write( &dwUniqueID, sizeof(DWORD) );
	Stream.Write( &bLock, sizeof(bool) );

	Send( eActor::SC_CMDLOOKTARGET, &Stream );
}

int CDnMonsterActor::CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, 
										const char *szParam, bool bOnPlayerInit/* = false*/, bool bCheckCanBegin/* = true*/ , bool bEternity /* = false */  )
{
	int iID = CDnActor::CmdAddStateEffect( pParentSkill, emBlowIndex, nDurationTime, szParam, bOnPlayerInit, bCheckCanBegin , bEternity );
	if( -1 == iID ) 
		return -1;

	DnBlowHandle hAddedBlow = m_pStateBlow->GetStateBlowFromID( iID );

	const CPacketCompressStream* pPacketStream = hAddedBlow->GetPacketStream( szParam, false );
	Send( eActor::SC_CMDADDSTATEEFFECT, const_cast<CPacketCompressStream*>(pPacketStream) );

	return iID;
}

void CDnMonsterActor::CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo/* = true*/ )
{
	if( m_pStateBlow->IsApplied( emBlowIndex ) )
	{
		if( bRemoveFromServerToo )
			CDnActor::CmdRemoveStateEffect( emBlowIndex );

		SendRemoveStateEffect( emBlowIndex );
	}
}


void CDnMonsterActor::SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &emBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );		

	Send( eActor::SC_CMDREMOVESTATEEFFECT, &Stream );
}


void CDnMonsterActor::CmdSuicide( bool bDropItem, bool bDropExp )
{
	if( IsDie() ) return;
	SetDestroy();

	if( m_hSummonerPlayerActor )
	{
		_ASSERT( m_hSummonerPlayerActor->IsPlayerActor() );
		if( m_hSummonerPlayerActor->IsPlayerActor() )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hSummonerPlayerActor.GetPointer());
			pPlayerActor->OnDieSummonedMonster( GetMySmartPtr() );
		}
	}

	if( m_bIsHelperAlly )
	{
#ifdef PRE_ADD_MONSTER_CATCH
		if( false == m_vlCatchedActors.empty() )
			ReleaseAllActor();
#endif
		bDropItem = false;
		bDropExp = false;
	}

	if( bDropExp ) {
		EnableDropItem( bDropItem );
		OnDie( DnActorHandle() );
	}
	else {
		if( bDropItem ) DropItems();
	}

	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &bDropItem, sizeof(bool) );
	Stream.Write( &bDropExp, sizeof(bool) );
	Send( eActor::SC_CMDSUICIDE, &Stream );
}

void CDnMonsterActor::CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, float fFrame, float fBlendFrame )
{
	int nActionIndex = GetElementIndex( szActionName );
	int nMaintenanceBoneIndex = GetBoneIndex( szMaintenanceBone );
	int nActionBoneIndex = GetBoneIndex( szActionBone );

	int nBlendAniIndex = m_nAniIndex;
	
	if( nActionIndex == -1 ) {
		assert(0);
	}

	if( nActionBoneIndex == -1 || nMaintenanceBoneIndex == -1 ) 
		return;

	BYTE pBuffer[128] = {0,};
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nActionBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nMaintenanceBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );

	Send( eActor::SC_CMDMIXEDACTION, &Stream );


}

void CDnMonsterActor::CmdWarp( EtVector3 &vPos, EtVector2 &vLook, CDNUserSession* pGameSession, bool bCheckPlayerFollowSummonedMonster/*=false*/ )
{
	CDnActor::CmdWarp( vPos, vLook, pGameSession, bCheckPlayerFollowSummonedMonster );

	MAWalkMovementNav *pMovement = dynamic_cast<MAWalkMovementNav *>(GetMovement());
	if( pMovement ) pMovement->ValidateCurCell();
}


void CDnMonsterActor::GenerationDropItem()
{
	m_VecDropItemList.clear();

	if( m_nItemDropGroupTableID < 1 ) return;

	int nExtendDropRate = 0;
	GetGameRoom()->GetExtendDropRateIgnoreTime(nExtendDropRate);
#if defined(PRE_ADD_WORLD_EVENT)
#else
#if defined(PRE_ADD_WEEKLYEVENT)
	if (CDnWorld::GetInstance(GetRoom()).GetMapSubType() != EWorldEnum::MapSubTypeNest){
		int nThreadID = GetGameRoom()->GetServerID();

		int nEventValue = g_pDataManager->GetWeeklyEventValue(0, 0, WeeklyEvent::Event_10, nThreadID);
		if (nEventValue != 0)
			nExtendDropRate += nEventValue;
	}
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)
#endif //#if defined(PRE_ADD_WORLD_EVENT)
#if defined( PRE_ADD_NEWCOMEBACK )
	if( GetGameRoom() )
	{
		if( GetGameRoom()->GetTaskMng() )
		{
			CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetGameRoom()->GetTaskMng()->GetTask("PartyTask"));
			if( pPartyTask )
			{
				float fBlowValue = pPartyTask->GetPlayerDropUpBlowValue();
				if( fBlowValue > 0 )
				{
					fBlowValue = fBlowValue * 100;
					nExtendDropRate += (int)fBlowValue;
				}
			}			
		}
	}
#endif
#if defined( PRE_ADD_STAGE_WEIGHT )
	if( GetGameRoom() && GetGameRoom()->GetTaskMng() )
	{
		CDnGameTask* pGameTask = static_cast<CDnGameTask*>(GetGameRoom()->GetTaskMng()->GetTask("GameTask"));
		if( pGameTask )
		{
			const TStageWeightData * pStageWeightData = pGameTask->GetStageWeightData();
			if( pStageWeightData )
			{
				float fStageWeightBonus = pStageWeightData->fItemDropRate;
				fStageWeightBonus = fStageWeightBonus * 100;
				nExtendDropRate += (int)fStageWeightBonus;
			}
		}			
	}
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )

	float fCalcDropCount = ((float)((float)nExtendDropRate/100) + 1.0f);
	for (int h = 0; 0 < fCalcDropCount; h++)
	{
		CDnDropItem::CalcDropItemList( GetRoom(), m_AIDifficult, m_nItemDropGroupTableID, m_VecDropItemList );

		for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) {
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			if( CDnDropItem::PreInitializeItem( GetRoom(), m_VecDropItemList[i].nItemID, m_VecDropItemList[i].nEnchantID ) == false )
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			if( CDnDropItem::PreInitializeItem( GetRoom(), m_VecDropItemList[i].nItemID ) == false )
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
			{
				m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
				i--;
			}
		}
		
		fCalcDropCount -= 1.0f;		
		if (h >= 4 || (fCalcDropCount < 1.0f && ((float)(_rand(GetRoom())%100)/100) > fCalcDropCount))
			break;			//Restrict to less than 4 (3 for event); if remaining value is less than 1, calculate probability to decide whether to run once more
	}
}

void CDnMonsterActor::SetGenerationArea( SOBB &Box )
{
	m_GenerationArea = Box;
}

SOBB *CDnMonsterActor::GetGenerationArea()
{
	return &m_GenerationArea;
}

void CDnMonsterActor::SyncClassTime( LOCAL_TIME LocalTime )
{
	MAActorRenderBase::m_LocalTime = LocalTime;

	CDnActor::SyncClassTime( LocalTime );
}

void CDnMonsterActor::OnChangeAction( const char *szPrevAction )
{
	if( m_pAi )
		m_pAi->OnChangeAction( szPrevAction );

#ifdef PRE_ADD_AURA_FOR_MONSTER
	_CheckActionWithProcessPassiveActionSkill(szPrevAction);
#endif

}

void CDnMonsterActor::_CheckActionWithProcessPassiveActionSkill( const char* szPrevAction )
{
	// Skip if repeating the same action
	if( szPrevAction && m_nPrevActionIndex == m_nActionIndex ) 
		return;

	// if instantly passive skill, then cancel skill. ( ex) archer's spinkick)
	// because state effect must deactivate when change to another attack action.
	if( m_hProcessSkill )
	{
		m_setUseActionName.clear();
		m_setUseActionName.insert( szPrevAction );

		// If the current action is the next action of the prev action, consider the skill to be chaining.
		// If previous action was used by the currently active skill and current action is not the next action of previous action
		// Consider the passive skill finished.
		ActionElementStruct* pElement = GetElement( szPrevAction );
		bool bIsNextAction = false;
		if( pElement )
		{
			// #25154 Default Stand action is not treated as continuing the next action specified by the skill.
			// When this function is called after aura skill action ends, current action is Stand at that point
			// m_hProcessSkill should be considered finished and set to NULL. 
			// m_hProcessSkill must not remain, as it could be forcibly ended (onend) when another skill is used.
			// Therefore bIsNextAction becomes false, and inside m_hProcessSkill->IsUseSkillActionNames()
			// The skill action should be considered finished.
			bIsNextAction = ((pElement->szNextActionName != "Stand") && (pElement->szNextActionName == GetCurrentAction()));
		}

		if( false == bIsNextAction &&
			m_hProcessSkill->IsUseActionNames( m_setUseActionName ) )
		{
			if( (m_hProcessSkill->GetSkillType() == CDnSkill::Passive || m_hProcessSkill->GetSkillType() == CDnSkill::AutoPassive) &&
				m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
			{
				// Check when chain input arrives for a passive skill. The chain input flag is reset once checked.
				// When chain input occurs, passive skill duration extends by the action length.
				// Both flag and time must be used together like this to allow continuous passive skill chaining.
				if( false == m_hProcessSkill->CheckChainingPassiveSkill() )
				{
					m_hProcessSkill->OnEnd( MAActorRenderBase::m_LocalTime, 0.0f );
					m_hProcessSkill.Identity();
				}
			}
			else
				// Note Hanki: m_hProcessSkill smart pointer must remain valid while the aura skill action is playing
				// Filtered in CDnPlayerActor::CmdStop() on the game server, so on the game server the action signal to the end 
				// Processed. Therefore, make ProcessAction an identity in CDnActor::OnChangeAction.
				if( IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn() )
				{
					m_hProcessSkill.Identity();
					ClearSelfStateSignalBlowQueue(); // Clear queue sent to state effect timing signal applied to self for aura skill; otherwise it affects other skills.
				}
		}
	}
}

void CDnMonsterActor::OnFinishAction(const char* szPrevAction, LOCAL_TIME time)
{
	if( m_bChangeAxisOnFinishAction )
	{		
		EtVector2 vView = EtVec3toVec2( *GetLookDir() );
		vView *= -1.f;
		Look( vView, true );
		m_bChangeAxisOnFinishAction = false;
	}

	if( m_pAi ) 
		m_pAi->OnFinishAction(szPrevAction, time);
}

void CDnMonsterActor::ResetActor()
{
	if( m_pAi ) 
		m_pAi->ResetDelay();
}

void CDnMonsterActor::OnBeginStateBlow( DnBlowHandle hBlow )
{
	CDnActor::OnBeginStateBlow( hBlow );

	if( m_pAggroSystem )
		m_pAggroSystem->OnStateBlowAggro( hBlow );
}

// MASkillUser
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
void CDnMonsterActor::OnAddSkill( DnSkillHandle hSkill, bool isInitialize/* = false*/ )
{
	// Immediately apply skills that are passive, buffs, and have Self state effects.
	bool bPassiveBuff = false;

	bPassiveBuff = ApplyPassiveSkill( hSkill, isInitialize );

	if( bPassiveBuff )
		m_vlhSelfPassiveBlowSkill.push_back( hSkill );
}

bool CDnMonsterActor::AddSkill( int nSkillTableID, int nLevel/* = 1*/, int iSkillLevelApplyType/* = CDnSkill::PVE*/ )
{
	if( IsExistSkill( nSkillTableID, nLevel ) ) return false;
	if( !MASkillUser::IsValidActor() ) return false;
	if( GetRoom() == NULL ) return false;

	DnSkillHandle hSkill = CDnSkill::CreateMonsterSkill( GetMySmartPtr(), nSkillTableID, nLevel );
	if( !hSkill ) return false;

#ifdef PRE_ADD_AURA_FOR_MONSTER
	CDnSkill::DurationTypeEnum eDurationType = hSkill->GetDurationType();
	switch(eDurationType)
	{
		case CDnSkill::Aura:
			m_vlhAuraSkills.push_back(hSkill);
	}
#endif

#ifndef PRE_FIX_SKILLLIST
	m_vlhSkillList.push_back( hSkill );
	m_vbSelfAllocList.push_back( true );
#else
	AddSkillObject( S_SKILL_OBJECT(hSkill, true) );
#endif // #ifndef PRE_FIX_SKILLLIST

	if( hSkill->GetActor() )
		hSkill->SetHasActor( GetMySmartPtr() );

	OnAddSkill( hSkill );
	
	return true;
}
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE

bool CDnMonsterActor::ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta )
{
	if ( CDnActor::ExecuteSkill(hSkill, LocalTime, fDelta) == false )
		return false;

	BYTE pBuffer[128] = {0,};
	CPacketCompressStream Stream( pBuffer, 128 );

	int nSkillID = hSkill->GetClassID();
	char cLevel = hSkill->GetLevel();

	Stream.Write( &nSkillID, sizeof(int) );
	Stream.Write( &cLevel, sizeof(char) );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
#if defined( PRE_ADD_ACADEMIC )
	int iSummonerSkillID = 0;
	DnSkillHandle hSummonerSkill = FindSkill( nSkillID );
	if( hSummonerSkill )
		iSummonerSkillID = hSummonerSkill->GetSummonerDecreaseSPSkillID();
	Stream.Write( &iSummonerSkillID, sizeof(int) );
#endif // #if defined( PRE_ADD_ACADEMIC )

	Send( eActor::SC_USESKILL, &Stream );

	ResetPositionRevision();

	return true;
}

CDnSkill::UsingResult CDnMonsterActor::UseSkill( int nSkillTableID, bool bCheckValid/* = true*/, bool bAutoUseFromServer/* = false*/, int nLuaSkillIndex/*=-1*/ )
{
	CDnSkill::UsingResult Result = MASkillUser::UseSkill( nSkillTableID, bCheckValid, bAutoUseFromServer );
	if( Result == CDnSkill::UsingResult::Success && nLuaSkillIndex >= 0 )
	{
		std::map<int,int>::iterator itor = m_mUseSkillCount.find( nLuaSkillIndex );
		if( itor == m_mUseSkillCount.end() )
		{
			m_mUseSkillCount.insert( std::make_pair(nLuaSkillIndex,1) );
		}
		else
		{
			++(*itor).second;
		}
	}
	
	return Result;
}

int CDnMonsterActor::GetUseSkillCount( int iLuaSkillIndex )
{
	std::map<int,int>::iterator itor = m_mUseSkillCount.find( iLuaSkillIndex );
	if( itor != m_mUseSkillCount.end() )
		return (*itor).second;

	return 0;
}

void CDnMonsterActor::SetScale( float fValue )
{
	m_fScale = fValue;
	if( m_hObject ) {
		m_hObject->SetCollisionScale( m_fScale );
	}
	MAActorRenderBase::SetScale( EtVector3( m_fScale, m_fScale, m_fScale ) );

	if( m_fWeight == 0.f ) m_fRevisionWeight = 0.f;
	else m_fRevisionWeight = m_fWeight + ( ( m_fScale - 1.f ) * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ScaleWeightValue ) );
}

float CDnMonsterActor::GetWeight()
{
	return m_fRevisionWeight;
}

float CDnMonsterActor::GetThreatRange()
{
	_ASSERT( dynamic_cast<CDNMonsterAggroSystem*>(m_pAggroSystem) );
	return m_pAggroSystem ? static_cast<CDNMonsterAggroSystem*>(m_pAggroSystem)->GetThreatRange() : 0.f; 
}

float CDnMonsterActor::GetCognizanceThreatRange()
{
	_ASSERT( dynamic_cast<CDNMonsterAggroSystem*>(m_pAggroSystem) );
	return m_pAggroSystem ? static_cast<CDNMonsterAggroSystem*>(m_pAggroSystem)->GetCognizanceThreatRange() : 0.f; 
}

float CDnMonsterActor::GetCognizanceThreatRangeSq()
{
	_ASSERT( dynamic_cast<CDNMonsterAggroSystem*>(m_pAggroSystem) );
	return m_pAggroSystem ? static_cast<CDNMonsterAggroSystem*>(m_pAggroSystem)->GetCognizanceThreatRangeSq() : 0.f; 
}

std::string CDnMonsterActor::GetAIFileName()
{
	DNTableFileFormat*	pSox	= NULL;
	int			nItemID = 0;

	if( m_nMonsterWeightTableID == -1 ) 
	{
		pSox	= GetDNTable( CDnTableDB::TMONSTER );
		nItemID	= m_nMonsterClassID;
	}
	else
	{
		pSox	= GetDNTable( CDnTableDB::TMONSTERWEIGHT );
		nItemID	= m_nMonsterWeightTableID;
	}

	if( !pSox || !pSox->IsExistItem( nItemID ) )
		return std::string("");
	std::string szName = pSox->GetFieldFromLablePtr( nItemID, "_CustomAI" )->GetString();

	return szName;
}

void CDnMonsterActor::CalcMonsterWeightIndex()
{
	if( m_nMonsterClassID == 0 ) return;
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTERWEIGHT );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"MonsterWeightTable.ext failed\r\n");
		return;
	}

	std::vector<int> nVecList;
	pSox->GetItemIDListFromField( "_MonsterTableIndex", m_nMonsterClassID, nVecList );

	int nDifficulty;
	for( DWORD i=0; i<nVecList.size(); i++ ) {
		nDifficulty = pSox->GetFieldFromLablePtr( nVecList[i], "_Difficulty" )->GetInteger();
		if( nDifficulty == m_AIDifficult ) {
			m_nMonsterWeightTableID = nVecList[i];
			break;
		}
	}
}

void CDnMonsterActor::SetNaviDestination( SOBB* pOBB, UINT uiMoveFrontRate )
{
	if( m_pAi )
		m_pAi->OnInitNaviDestination( pOBB, uiMoveFrontRate );
	else
		_DANGER_POINT();
}

int CDnMonsterActor::GetWaitOrderCount( int iSkillID )
{
	std::map<int,int>::iterator itor = m_mOrderCount.find( iSkillID );
	if( itor == m_mOrderCount.end() )
		return 0;
	return (*itor).second;
}

void CDnMonsterActor::AddWaitOrderCount( int iSkillID )
{
	std::map<int,int>::iterator itor = m_mOrderCount.find( iSkillID );
	if( itor == m_mOrderCount.end() )
	{
		m_mOrderCount.insert( std::make_pair(iSkillID,1) );
		return;
	}
	++(*itor).second;
}

void CDnMonsterActor::DelWaitOrderCount( int iSkillID )
{
	std::map<int,int>::iterator itor = m_mOrderCount.find( iSkillID );
	if( itor == m_mOrderCount.end() )
		return;
	if( (*itor).second > 0 )
		--(*itor).second;
}

bool CDnMonsterActor::OnAINonTarget()
{
	// Check Walk_Font action
	if( !IsExistAction( "Walk_Front") )
		return true;

	if ( !IsHit() && IsMove() ) 	
		CmdStop( "Stand", 0, g_fBendFrame );

	if ( !IsMovable() ) 
		return false;
	
	// Prowling around.
	EtVector3 *pvPos = m_hActor->GetPosition();
	EtVector3 vTemp;
	vTemp = *pvPos;
	vTemp.y = 0.f;
	vTemp.x += cos( EtToRadian( _rand(m_hActor->GetRoom())%360 ) ) * ( 300 + _rand(m_hActor->GetRoom())%200 );
	vTemp.z += sin( EtToRadian( _rand(m_hActor->GetRoom())%360 ) ) * ( 300 + _rand(m_hActor->GetRoom())%200 );

	SOBB Box = *GetGenerationArea();
	Box.Extent[1] = 1000000.f;
	if( !Box.IsInside( vTemp ) ) 
	{
		vTemp = Box.Center;
		vTemp -= Box.Axis[0] * Box.Extent[0];
		vTemp -= Box.Axis[2] * Box.Extent[2];
		
		int iModValue = static_cast<int>(Box.Extent[0] * 2.f);
		if( iModValue > 0 )	vTemp += Box.Axis[0] * (float)( _rand(m_hActor->GetRoom())%iModValue );
		else				vTemp += Box.Axis[0];
		
		iModValue = static_cast<int>(Box.Extent[2] * 2.f);
		if( iModValue )		vTemp += Box.Axis[2] * (float)( _rand(m_hActor->GetRoom())%iModValue );
		else				vTemp += Box.Axis[2];
		
		vTemp.y = CDnWorld::GetInstance(m_hActor->GetRoom()).GetHeight( vTemp );
		vTemp += Box.Axis[1] * Box.Extent[1];
	}

	CmdMove( vTemp, "Walk_Front", -1, g_fBendFrame );

	return true;
}

void CDnMonsterActor::OnBumpWall()
{
	if( !m_pszCanBumpActionName )
		return;

	CmdAction( m_pszCanBumpActionName, 0, g_fBendFrame );
}

// Send packet
// This part sends packets by action index and signal index instead of CDnProjectile::GetPacketStream(), so
// Client finds projectile signal info to create it; leave as is for now. 
// Currently there is no issue, but this must be modified later if data needs to be decided and sent by the server. - Hanki.
void CDnMonsterActor::SendProjectile( CDnProjectile *pProjectile, ProjectileStruct* pStruct, MatrixEx& LocalCross, int iSignalIndex ) 
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwVal = pProjectile->GetUniqueID();
	Stream.Write( &dwVal,					sizeof(DWORD)												);
	Stream.Write( &LocalCross.m_vPosition,	sizeof(EtVector3),	CPacketCompressStream::VECTOR3_BIT		);
	Stream.Write( &LocalCross.m_vXAxis,	sizeof(EtVector3),	CPacketCompressStream::VECTOR3_SHORT	);
	Stream.Write( &LocalCross.m_vYAxis,	sizeof(EtVector3),	CPacketCompressStream::VECTOR3_SHORT	);
	Stream.Write( &LocalCross.m_vZAxis,	sizeof(EtVector3),	CPacketCompressStream::VECTOR3_SHORT	);
	
#if defined(PRE_FIX_55378)
	int nActionIndex = m_nActionIndex;

	std::string szChargerAction = GetChargerAction();
	if (szChargerAction.empty() == false)
		nActionIndex = GetElementIndex(szChargerAction.c_str());

	Stream.Write( &nActionIndex,			sizeof(int)													);
#else
	Stream.Write( &m_nActionIndex,			sizeof(int)													);
#endif // PRE_FIX_55378

	Stream.Write( &iSignalIndex,			sizeof(int)													);
	
	bool bUsedForceDir = pProjectile->IsUsedForceDir();
	Stream.Write( &bUsedForceDir, sizeof(bool) );
	if( bUsedForceDir )
	{
		const EtVector3& vForceDir = pProjectile->GetForceDir();
		Stream.Write( &vForceDir, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	}

	switch( pStruct->nTargetType )
	{
		case 2:	// TargetPosition
		{
			Stream.Write( pProjectile->GetTargetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
			break;
		}
		case 3:	// Target
		{
			DnActorHandle hTarget = pProjectile->GetTargetActor();
			if( !hTarget )
				hTarget = GetActorHandle();

			dwVal = hTarget->GetUniqueID();
			Stream.Write( &dwVal, sizeof(DWORD) );
			break;
		}
	}

	Send( eActor::SC_PROJECTILE, &Stream );
}


// Related to issue #15557. Case where a monster fires a projectile from a projectile.
// Basically the same as what the client's LocalPlayerActor sends.
// On client side, process packets identically to projectiles fired by party members.
// Therefore CDnLocalPlayerActor::OnProjectile() must also be modified together. 
void CDnMonsterActor::SendProjectileFromProjectile( CDnProjectile* pProjectile, int nSignalIndex )
{
	CPacketCompressStream* pPacketStream = pProjectile->GetPacketStream();
	_ASSERT( pPacketStream );

	if( pPacketStream )
		Send( eActor::SC_MONSTER_PROJECTILE_FROM_PROJECTILE, pPacketStream );
}


void CDnMonsterActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )	
{
	switch( Type ) {
		case STE_LockTargetLook:
			{
#if defined( PRE_MOD_LOCK_TARGET_LOOK )
				LockTargetLookStruct * pStruct = static_cast<LockTargetLookStruct*>(pPtr);
				m_nLockLookEventArea = pStruct->LookEeventAreaID;
#endif	// PRE_MOD_LOCK_TARGET_LOOK

#if defined( PRE_FIX_MOVEBACK )
					if( !IsNearMoveBack() )
						m_bLockLookTarget = true;
#else
					m_bLockLookTarget = true;
#endif
				break;
			}
		case STE_CanBumpWall:
			{
				CanBumpWallStruct* pStruct = static_cast<CanBumpWallStruct*>(pPtr);
				m_pszCanBumpActionName = pStruct->szActionName;
				break;
			}
		case STE_Projectile:
			{
				ProjectileStruct* pStruct = (ProjectileStruct *)pPtr;
				// If Multiple target, additionally create and fire Projectile on server
#if defined (PRE_MOD_AIMULTITARGET)
				if( static_cast<MAAiScript*>(GetAIBase())->m_cMultipleTarget.GetMultipleTarget() > 0 )
#else
				if( static_cast<MAAiScript*>(GetAIBase())->m_cMultipleTarget.bIsMultipleTarget() )
#endif
				{
					static_cast<MAAiScript*>(GetAIBase())->m_cMultipleTarget.CreateProjectile( this, static_cast<ProjectileStruct*>(pPtr), nSignalIndex );
				}
				else
				{
					DNVector(DnActorHandle) StigmaActorList;
					if (pStruct->nTargetStateIndex != 0)
					{
						ScanActorByStateIndex(StigmaActorList, STATE_BLOW::BLOW_246);
					}

					//Process only when there is a brand target
					if (StigmaActorList.empty() == false)
					{
						int nStigmaActorCount = (int)StigmaActorList.size();
						for (int i = 0; i < nStigmaActorCount; ++i)
						{
							//Target actor.
							DnActorHandle hTargetActor = StigmaActorList[i];

							CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( GetRoom(), GetMySmartPtr(), m_Cross, pStruct, NULL, hTargetActor );
							if( pProjectile == NULL ) 
								return;
							pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

							SendProjectile( pProjectile, pStruct, m_Cross, nSignalIndex );
							OnProjectile( pProjectile, pStruct, m_Cross, nSignalIndex );
							OnSkillProjectile( pProjectile );
						}
					}
					else
					{
						//#52808 - Additional request (if brand projectile setting is 2, do not create projectile when there is no branded target).
						if (pStruct->nTargetStateIndex == 2)
							return;

						CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( GetRoom(), GetMySmartPtr(), m_Cross, pStruct );

						if( pProjectile == NULL ) 
							return;
						pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

						SendProjectile( pProjectile, pStruct, m_Cross, nSignalIndex );
						OnProjectile( pProjectile, pStruct, m_Cross, nSignalIndex );
						OnSkillProjectile( pProjectile );
					}
				}
				return;
			}
		case STE_ProjectileTargetPosition:
			{
				if( m_pAi )
					m_pAi->SetProjectileTarget();
				break;
			}
	
		case STE_ReserveProjectileTarget:
			{
				if( m_pAi )
					m_pAi->ReservedProjectileTarget();
				
				break;
			}
		
		case STE_RotateResistance:
			{
				RotateResistanceStruct *pStruct = (RotateResistanceStruct *)pPtr;
				m_fRotateResistance = pStruct->fResistanceRatio;
				break;
			}
		case STE_PositionRevision:
			{
				PositionRevisionStruct* pStruct = reinterpret_cast<PositionRevisionStruct*>(pPtr);
				m_uiForcePositionRevisionTick = pStruct->nRevisionTick;
				return;
			}

		case STE_SuicideMonster:
			{
				SuicideMonsterStruct* pStruct = (SuicideMonsterStruct*)pPtr;
				// Whether to drop the item. Also sends packet to client here.
				CmdSuicide( pStruct->bDropItem ? true : false, pStruct->bGetExp ? true : false );
			}
			break;
		case STE_TriggerEvent:
			{
				TriggerEventStruct *pStruct = (TriggerEventStruct *)pPtr;
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventArea", GetBirthAreaHandle() );
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "ActorHandle", GetUniqueID() );
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventID", pStruct->nEventID );
				CDnWorld::GetInstance(GetRoom()).OnTriggerEventCallback( "CDnActor::TriggerEvent", LocalTime, 0.f );
			}
			break;
		case STE_Announce:
		{
			AnnounceStruct* pStruct = reinterpret_cast<AnnounceStruct*>(pPtr);
			
			DNVector(DnActorHandle) hScanList;
			GetMAScanner().Scan( MAScanner::eType::MonsterSkillSameTeamExpectMe, m_hActor, 0.f, static_cast<float>(pStruct->nRange), hScanList );

			for( UINT i=0 ; i<hScanList.size() ; ++i )
			{
				DnActorHandle hActor = hScanList[i];
				if( hActor && hActor->IsMonsterActor() )
				{
					CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());
					if( pMonster->GetMonsterClassID() == pStruct->nTargetMonsterID )
					{
						pMonster->GetAIBase()->NotifyDieAnnounce();
					}
				}
			}
			break;
		}
		case STE_NextCustomAction:
		{
			if( !GetAIBase() )
				return;

			int			state	= 0;
			float		fLength = 0.0f;
			EtVector3*	pvPos	= GetPosition();

			MAAiScript* pScript = static_cast<MAAiScript*>(GetAIBase());
			if( pScript->GetTarget() ) 
			{
				state	= pScript->GetTargetDistanceState( pScript->GetTarget() );
				fLength = EtVec3Length( &( *pvPos - *pScript->GetTarget()->GetPosition() ) );
			}

			if( static_cast<MAAiScript*>(GetAIBase())->GetAIState() == MAAiScript::AT_CustomAction )
				static_cast<MAAiScript*>(GetAIBase())->OnCustomAction( state, fLength, 0 );
			if( static_cast<MAAiScript*>(GetAIBase())->GetAIState() == MAAiScript::AT_UseSkill )
			{
				CancelUsingSkill();
				static_cast<MAAiScript*>(GetAIBase())->OnUseSkill( state, fLength, 0 );
			}
			return;
		}

#ifdef PRE_ADD_MONSTER_CATCH
		case STE_CatchActor:
			{
				CatchActor( reinterpret_cast<CatchActorStruct*>(pPtr), nSignalIndex );
			}
			break;

		case STE_ReleaseActor:
			{
				ReleaseAllActor( /*reinterpret_cast<ReleaseActorStruct*>(pPtr)*/ );
			}
			break;
#endif // #ifdef PRE_ADD_MONSTER_CATCH
		case STE_ChangeAxis:
			{
				m_bChangeAxisOnFinishAction = true;
			}
			break;
	}
	CDnActor::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}

bool CDnMonsterActor::IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal, int iHitUniqueID )
{
#if defined(PRE_FIX_61382)
#else
	//#59347
	//For puppet-summoned monsters, check the owner actor's Hittable status to determine if hittable.
	if ( IsPuppetSummonMonster() && m_hSummonerPlayerActor )
	{
		bool bSummonerPlayerActorHittable = m_hSummonerPlayerActor->IsHittable(hHitter, LocalTime, pHitSignal, iHitUniqueID);
		if (bSummonerPlayerActorHittable == false)
			return false;
	}
#endif // PRE_FIX_61382

	if( m_bNoDamage ) return false;
	return CDnActor::IsHittable( hHitter, LocalTime, pHitSignal, iHitUniqueID );
}

void CDnMonsterActor::ProcessPositionRevision( float fDelta )
{
	if( CheckSignalPositionRevision( fDelta ) || CheckPositionRevision()  ) 
	{
		BYTE pBuffer[128] = { 0, };
		CPacketCompressStream Stream( pBuffer, 128 );

		Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
		Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

		Send( eActor::SC_POSREV, &Stream );

		ResetPositionRevision();
	}
}

bool CDnMonsterActor::CheckPositionRevision()
{
	if( m_LastSendMoveMsg == 0 )		
		return false;
	
	if( !IsMove() ) 
	{
		m_LastSendMoveMsg = CDnActionBase::m_LocalTime;
		return false;
	}

	if( CDnActionBase::m_LocalTime - m_LastSendMoveMsg > s_nPositionRevisionTime ) 
		return true;

	return false;
}

bool CDnMonsterActor::CheckSignalPositionRevision( float fDelta )
{
	if( m_uiForcePositionRevisionTick == 0 )
		return false;

	if( m_uiPrevForcePositionRevisionTick == 0 )
	{
		//std::cout << GetTickCount() << ":First forced position correction" << std::endl;
		m_fForcePositionRevisionDelta = m_uiForcePositionRevisionTick/1000.f;
		return true;
	}

	m_fForcePositionRevisionDelta -= fDelta;
	if( m_fForcePositionRevisionDelta <= 0.f )
	{
		m_fForcePositionRevisionDelta = m_uiForcePositionRevisionTick/1000.f;
		//std::cout << GetTickCount() << ":Forced position correction" << std::endl;
		return true;
	}

	return false;
}

void CDnMonsterActor::ResetPositionRevision()
{
	m_LastSendMoveMsg = CDnActionBase::m_LocalTime;
}


float CDnMonsterActor::GetRotateAngleSpeed()
{
	return CDnMonsterState::GetRotateAngleSpeed() * m_fRotateResistance;
}

void CDnMonsterActor::AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex, bool bDelete )
{
	CDnActor::AttachWeapon( hWeapon, nEquipIndex, bDelete );
	// Reset weapon durability
	if( m_nMonsterWeightTableID == -1 ) return;
	if( hWeapon->IsInfinityDurability() || hWeapon->GetDurability() <= 0 ) return;

	DNTableFileFormat* pWeight = GetDNTable( CDnTableDB::TMONSTERWEIGHT );
	int nDurability = (int)( hWeapon->GetDurability() * pWeight->GetFieldFromLablePtr( m_nMonsterWeightTableID, "_DurabilityWeight" )->GetFloat() );
	hWeapon->SetDurability( nDurability );
}


void CDnMonsterActor::SetTeam( int nValue )
{
	if( GetTeam() != nValue ) {
		if( m_pAi ) m_pAi->ResetAggro();
	}
	CDnActor::SetTeam( nValue );
}

void CDnMonsterActor::ProcessPartyCombo( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nPartyComboDelay > 0 ) m_nPartyComboDelay -= (int)( fDelta * 1000 );
	if( m_nPartyComboDelay < 0 ) {
		m_nPartyComboCount = 0;
		m_nPartyComboDelay = 0;
		m_hPartyHitActor.Identity();
	}
}

void CDnMonsterActor::OnPartyCombo( DnActorHandle hHitter, int nComboDelay )
{
	// #12170 Hits with combo delay 0 do not affect combo evaluation.
	if( 0 == nComboDelay )
		return;

	if( m_hPartyHitActor == hHitter ) {
		m_nPartyComboDelay = nComboDelay;
		return;
	}
	m_hPartyHitActor = hHitter;
	if( m_nPartyComboDelay > 0 ) {
		m_nPartyComboCount++;
		if( hHitter && hHitter->IsPlayerActor() ) {
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hHitter.GetPointer());
			pPlayer->UpdatePartyCombo( m_nPartyComboCount );
		}
	}
	else {
		m_nPartyComboCount = 0;
	}
	m_nPartyComboDelay = nComboDelay;
}

void CDnMonsterActor::Process_AutoRecallRange()
{
	if( NULL == m_hSummonerPlayerActor )
		return;

	if( 0 == m_nAutoRecallRange )
		return;

	//#55707 Forced summon disabled if currently in attack state
	bool isAttackState = IsAttack();
	if (isAttackState == true)
		return;

	EtVector2 vVec;
	vVec.x = m_hSummonerPlayerActor->GetPosition()->x - m_Cross.m_vPosition.x;
	vVec.y = m_hSummonerPlayerActor->GetPosition()->z - m_Cross.m_vPosition.z;

	float fLength = EtVec2Length(&vVec);

	if( m_nAutoRecallRange < fLength )
	{
		//#55707 Reset aggro on forced summon.
		ResetAggro();

		CmdWarp( *m_hSummonerPlayerActor->GetPosition(), EtVec3toVec2( *m_hSummonerPlayerActor->GetLookDir() ) );
	}
}

void CDnMonsterActor::ReTransmitSlaveMsg(CDNUserSession* pBreakIntoSession) // Used when sending to drop-in user for synchronization.
{
	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &m_dwSummonerActorID, sizeof(DWORD) );
	Stream.Write( &m_bSummoned , sizeof(bool) );
	Stream.Write( &m_nMaxHP, sizeof(INT64) );
	Stream.Write( &m_nHP, sizeof(INT64) );
	Stream.Write( &m_bFollowSummonerStage, sizeof(bool) );
	Stream.Write( &m_bReCreatedFollowStageMonster, sizeof(bool) );

#if defined(PRE_FIX_55618)
	//Send BaseState MaxHP/HP/move speed info to client, and client saves the info upon receiving this packet
	//Set m_bCopiedFromSummoner so it is no longer updated..
	INT64 nBaseMaxHP = m_BaseState.GetMaxHP();
	float nBaseMaxHPRate = m_BaseState.GetMaxHPRatio();

	Stream.Write( &nBaseMaxHP, sizeof(INT64) );
	Stream.Write( &nBaseMaxHPRate, sizeof(float) );

	Stream.Write( &m_nMoveSpeed, sizeof(int) );
#endif // PRE_FIX_55618

	Send( pBreakIntoSession , eActor::SC_SLAVE_OF, GetUniqueID() , &Stream );
}


void CDnMonsterActor::SlaveOf( DWORD dwSummonerActorUniqueID, bool bSummoned/* = false*/, bool bSuicideWhenSummonerDie/* = false*/, bool bFollowSummonerStage/* = false*/, bool bReCreateFollowStageMonster/* = false*/ )
{
	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &dwSummonerActorUniqueID, sizeof(DWORD) );
	Stream.Write( &bSummoned, sizeof(bool) );
	if( bSummoned )
	{
		Stream.Write( &m_nMaxHP, sizeof(INT64) );
		Stream.Write( &m_nHP, sizeof(INT64) );
		Stream.Write( &bFollowSummonerStage, sizeof(bool) );
		Stream.Write( &bReCreateFollowStageMonster, sizeof(bool) );

#if defined(PRE_FIX_55618)
		//Send BaseState MaxHP/HP/move speed info to client, and client saves the info upon receiving this packet
		//Set m_bCopiedFromSummoner so it is no longer updated..
		INT64 nBaseMaxHP = m_BaseState.GetMaxHP();
		float nBaseMaxHPRate = m_BaseState.GetMaxHPRatio();
		
		Stream.Write( &nBaseMaxHP, sizeof(INT64) );
		Stream.Write( &nBaseMaxHPRate, sizeof(float) );

		Stream.Write( &m_nMoveSpeed, sizeof(int) );
#endif // PRE_FIX_55618

#if defined(PRE_FIX_61382)
		Stream.Write(&m_isPuppetSummonMonster, sizeof(bool));
#endif // PRE_FIX_61382
	}

	Send( eActor::SC_SLAVE_OF, &Stream );

	m_dwSummonerActorID = dwSummonerActorUniqueID;
	
	if( bSummoned )
	{
		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( GetRoom(), dwSummonerActorUniqueID );
		//#53454 Cases occur where the Puppet summon owner is not a PlayerActor.
		if( hActor/* && hActor->IsPlayerActor()*/ )
		{
			m_hSummonerPlayerActor = hActor;
			m_bSuicideWhenSummonerDie = bSuicideWhenSummonerDie;
			m_bFollowSummonerStage = bFollowSummonerStage;

			// Monster actors summoned by players use the split PvP/PvE skill level table.
			SelectSkillLevelDataType( hActor->GetSelectedSkillLevelDataType(), true );
		}
	}

	m_bSummoned = bSummoned;
	m_bReCreatedFollowStageMonster = bReCreateFollowStageMonster;
}

// Currently used only for cannons.
void CDnMonsterActor::SlaveRelease( void )
{
	if( 0 == m_dwSummonerActorID )
		return;

	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &m_dwSummonerActorID, sizeof(DWORD) );
	Send( eActor::SC_SLAVE_RELEASE, &Stream );

	if( m_hSummonerPlayerActor )
	{
		m_hSummonerPlayerActor.Identity();
	}
}


void CDnMonsterActor::OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct )
{
	// Normal mobs have nothing to do here, but mobs summoned by a relic must be counted toward the summoning player's combo count.
	if( m_hSummonerPlayerActor && m_hSummonerPlayerActor->IsPlayerActor() )
		m_hSummonerPlayerActor->OnHitSuccess( LocalTime, hActor, pStruct );
}

void CDnMonsterActor::OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct )
{
	CDnActor::OnHitFinish( LocalTime, pStruct );

	if (IsSummonedMonster())
	{
		DnActorHandle hMasterActor = GetSummonerPlayerActor();
		if (hMasterActor)
		{
			int nDieCount = 0;
			for( DWORD i=0; i<m_hVecLastHitList.size(); i++ ) {
				if( m_hVecLastHitList[i] && m_hVecLastHitList[i]->IsDie() ) nDieCount++;
			}

			hMasterActor->UpdateMissionByMonsterKillCount(nDieCount);
		}
	}

	if( GetAIBase() )
		GetAIBase()->OnHitFinish( LocalTime, pStruct );
}

void CDnMonsterActor::ResetAggro( void )
{
	if( m_pAggroSystem ) 
		m_pAggroSystem->ResetAggro(); 
}

void CDnMonsterActor::ResetAggro( DnActorHandle hActor )
{
	if( m_pAggroSystem )
		m_pAggroSystem->ResetAggro( hActor );
}

void CDnMonsterActor::RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage )
{
	if( m_bNoDamage ) return;
	CDnActor::RequestDamageFromStateBlow( hFromBlow, iDamage );
}

#ifdef PRE_ADD_MONSTER_CATCH
void CDnMonsterActor::_CatchThisActor( DnActorHandle hResultActor, int nSignalIndex )
{
	S_CATCH_ACTOR_INFO CatchActorInfo;

	CatchActorInfo.hCatchedActor = hResultActor;
	CatchActorInfo.hCatchedActor->SetActionQueue( m_strCatchedActorAction.c_str() );
	m_iCatchedActorActionIndex = CatchActorInfo.hCatchedActor->GetElementIndex( m_strCatchedActorAction.c_str() );

	// Add immobilize/incapacitate state effect.
	CatchActorInfo.iCatchCantMoveBlowID = CatchActorInfo.hCatchedActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_070, -1, "", false, false );
	CatchActorInfo.iCatchCantActionBlowID = CatchActorInfo.hCatchedActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_071, -1, "", false, false );

	if( CatchActorInfo.hCatchedActor->IsPlayerActor() )
	{
		static_cast<CDnPlayerActor*>(CatchActorInfo.hCatchedActor.GetPointer())->SetCatcherMonsterActor( GetMySmartPtr() );
	}

	m_vlCatchedActors.push_back( CatchActorInfo );

	// Send grab packet to client.
	BYTE pBuffer[ 128 ] = { 0 };
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwCatchedActorID = CatchActorInfo.hCatchedActor->GetUniqueID();
	int nCatchActionIndex = GetCurrentActionIndex();
	int nCatchSignalArrayIndex = nSignalIndex;
	Stream.Write( &dwCatchedActorID, sizeof(DWORD) );
	Stream.Write( &nCatchActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nCatchSignalArrayIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

	Send( eActor::SC_CATCH_ACTOR, &Stream );
}

void CDnMonsterActor::CatchActor( CatchActorStruct* pCatchActor, int nSignalIndex )
{
	// If multi-grab is not enabled, skip processing even if holding just one target.
	if( FALSE == pCatchActor->bMultiCatch )
		if( false == m_vlCatchedActors.empty() )
			return;

	m_strCatchBoneName = pCatchActor->szCatchBoneName;
	m_strTargetActorCatchBoneName = pCatchActor->szTargetActorCatchBoneName;

	if( pCatchActor->szCatchedActorAction != NULL &&
		0 < strlen(pCatchActor->szCatchedActorAction) )
	{
		m_strCatchedActorAction.assign( pCatchActor->szCatchedActorAction );
	}
	else
		m_strCatchedActorAction.assign( "Hold" );

	EtVector3 vCatchBonePos( 0.0f, 0.0f, 0.0f );
	GetObjectHandle()->SetCalcAni( true );
	EtMatrix matBoneWorld = GetBoneMatrix( m_strCatchBoneName.c_str() );
	GetObjectHandle()->SetCalcAni( false );
	memcpy_s( &vCatchBonePos, sizeof(EtVector3), &matBoneWorld._41, sizeof(EtVector3) );
	
	// Get bone position by bone name in use, and near the current bone position check against the range set in the signal 
	// Check if there is one with a shorter distance.
	DNVector( DnActorHandle ) vlhActors;
	ScanActor( GetGameRoom(), vCatchBonePos, pCatchActor->fCatchDistance, vlhActors );

	// Check range using height value only.

	float fMinDistanceSQ = FLT_MAX;
	DnActorHandle hResultActor;
	for( int i = 0; i < (int)vlhActors.size(); ++i )
	{
		DnActorHandle hActor = vlhActors.at( i );

		// Exclude actors already checked in the same signal.
		if( m_setCatchCheckedActorIDs.end() != m_setCatchCheckedActorIDs.find( hActor->GetUniqueID() ) )
			continue;

		// Skip self
		if( GetMySmartPtr() == hActor )
			continue;

		// Checked items are immediately added to the set.
		m_setCatchCheckedActorIDs.insert( hActor->GetUniqueID() );

		// Player actors only.
		if( false == hActor->IsPlayerActor() )
			continue;

		// Exclude same team.
		if( GetTeam() == hActor->GetTeam() )
			continue;

		// Check CanHit status.
		if( FALSE == pCatchActor->bIgnoreCanHit )
		{
			if( false == hActor->CDnActorState::IsHittable() )
				continue;

			// Cannot be grabbed if invincibility state effect is on.
			if( hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_099 ) )
				continue;
		}
 	
		// If bMultiCatch flag is off, handle catching the closest target.
		// If enabled, grab all targets within range.
		EtVector3 vTargetActorPos = *hActor->GetPosition();
		EtVector3 vDist = vCatchBonePos - vTargetActorPos;
		float fDistanceSQ = EtVec3LengthSq( &vDist );
		if( TRUE == pCatchActor->bMultiCatch ||
			fDistanceSQ < fMinDistanceSQ )
		{
			// Check if height also matches.
			bool bHeightMax =  vTargetActorPos.y < (pCatchActor->fHeightMax + vCatchBonePos.y);
			bool bHeightMin = (vCatchBonePos.y + pCatchActor->fHeightMin) < vTargetActorPos.y;		// HeightMin value is negative.
			if( bHeightMax && bHeightMin )
			{
				bool bCatchFailed = true;

				// Info to send to the client when grab fails.
				// Failure reason, 0: parried, 1: blocked, 2: endured with super armor.
				int iFailedType = -1;
				int iBlowID = 0;
				string strBlockOrParringAction;
				//////////////////////////////////////////////////////////////////////////

				// Whether to ignore parrying state effect. Since this is not a hit here, force probability check on related state effects.
				bool bPassParring = true;
				if( FALSE == pCatchActor->bIgnoreParring )
				{
					bool bAppliedBlockBlow = hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_030 );
					bool bAppliedParringBlow = hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_031 );

					if( bAppliedBlockBlow || bAppliedParringBlow )
					{
						// If parried, the parrying action must be executed.
						// Create a dummy HitParam and pass it because the state effect side uses HitParam.
						SHitParam HitParam;
						HitParam.szActionName = m_strCatchedActorAction;
						HitParam.fDamage = 1.0f;// Since damage is not dealt here anyway, any value works. If 0, it passes the block state effect check.
						HitParam.bIgnoreParring = false;

						CDnActorState* pState = static_cast<CDnActorState*>(this);
						bool bSuccess = false;

						if( bAppliedParringBlow )
						{
							DNVector(DnBlowHandle) vlhBlows;
							hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_031, vlhBlows );
							if( false == vlhBlows.empty() )
							{
								DnBlowHandle hParringBlow = vlhBlows.front();
								bSuccess = hParringBlow->OnDefenseAttack( GetMySmartPtr(), pState, HitParam, true );
								if( bSuccess )
								{
									iFailedType = 0;
									iBlowID = hParringBlow->GetBlowID();
								}
							}
						}

						if( (false == bSuccess) && bAppliedBlockBlow )
						{
							DNVector(DnBlowHandle) vlhBlows;
							hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_030, vlhBlows );
							if( false == vlhBlows.empty() )
							{
								DnBlowHandle hBlockBlow = vlhBlows.front();
								bSuccess = hBlockBlow->OnDefenseAttack( GetMySmartPtr(), pState, HitParam, true );
								if( bSuccess )
								{
									iFailedType = 1;
									iBlowID = hBlockBlow->GetBlowID();
								}
							}
						}

						if( bSuccess )
						{
							bPassParring = false;
							// If hit action was changed by parrying or block state effect, execute it and skip.
							if( HitParam.szActionName != m_strCatchedActorAction )
							{
								m_hActor->SetActionQueue( HitParam.szActionName.c_str() );
								strBlockOrParringAction = HitParam.szActionName;
							}
						}
					}
				}

				// If not parried, actually process grab. Finally check if endured with super armor.
				if( bPassParring )
				{
					if( hActor->CatchCalcSuperArmor( GetMySmartPtr(), pCatchActor->nApplySuperArmorDamage ) )
					{
						// Target actually caught after running out of super armor.
						// Handle multi-grab depending on whether the multi-grab flag is enabled.
						if( TRUE == pCatchActor->bMultiCatch )
						{
							_CatchThisActor( hActor, nSignalIndex );
						}
						else
						{
							hResultActor = hActor;
							fMinDistanceSQ = fDistanceSQ;
						}

						bCatchFailed = false;
					}
					else
					{
						iFailedType = 2;
					}
				}

				// If the monster ultimately fails to grab the character even when within range, notify the client.
				// Parrying, block, or super armor could be the reason.
				// For block, client decrements count, and both parry and block must execute the blocking action.
				if( bCatchFailed )
				{
					BYTE pBuffer[ 128 ] = { 0 };
					CPacketCompressStream Stream( pBuffer, 128 );

					// Failure reason, 0: parried, 1: blocked, 2: endured with super armor.
					int iFailedType = 0;
					DWORD dwCatchFailedActorID = hActor->GetUniqueID();
					int iParringOrBlockActionIndex = hActor->GetElementIndex( strBlockOrParringAction.c_str() );
					Stream.Write( &iFailedType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
					Stream.Write( &iBlowID, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
					Stream.Write( &dwCatchFailedActorID, sizeof(DWORD) );
					Stream.Write( &iParringOrBlockActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

					Send( eActor::SC_CATCH_ACTOR_FAILED, &Stream );
				}

			}
		}
	}

	if( hResultActor )
	{
		_CatchThisActor( hResultActor, nSignalIndex );
	}
}

void CDnMonsterActor::ReleaseThisActor( DnActorHandle hActor )
{
	vector<S_CATCH_ACTOR_INFO>::iterator iter = m_vlCatchedActors.begin();
	for( iter; iter != m_vlCatchedActors.end(); ++iter )
	{
		S_CATCH_ACTOR_INFO& CatchActorInfo = *iter;
		if( CatchActorInfo.hCatchedActor == hActor )
		{
			ReleaseThisActor( CatchActorInfo );
			m_vlCatchedActors.erase( iter );
			break;
		}
	}
}

void CDnMonsterActor::ReleaseThisActor( S_CATCH_ACTOR_INFO &CatchActorInfo )
{
	if( CatchActorInfo.hCatchedActor )
	{
		m_setCatchCheckedActorIDs.erase( CatchActorInfo.hCatchedActor->GetUniqueID() );

		CatchActorInfo.hCatchedActor->CmdRemoveStateEffectFromID( CatchActorInfo.iCatchCantMoveBlowID );
		CatchActorInfo.hCatchedActor->CmdRemoveStateEffectFromID( CatchActorInfo.iCatchCantActionBlowID );
	
		// Send packet to client indicating placement.
		BYTE pBuffer[ 128 ] = { 0 };
		CPacketCompressStream Stream( pBuffer, 128 );

		DWORD dwCatchActorID = CatchActorInfo.hCatchedActor->GetUniqueID();
		Stream.Write( &dwCatchActorID, sizeof(DWORD) );

		Send( eActor::SC_RELEASE_ACTOR, &Stream );

		// TODO: If we need to notify the moment it is released.. handle that separately.

		if( CatchActorInfo.hCatchedActor->IsPlayerActor() )
		{
			static_cast<CDnPlayerActor*>( CatchActorInfo.hCatchedActor.GetPointer() )->ReleaseCatcherMonsterActor();
		}
	}
}


void CDnMonsterActor::ReleaseAllActor( void )
{
	// Clear check set.
	m_setCatchCheckedActorIDs.clear();

	// Remove applied state effects
	for( int i = 0; i < (int)m_vlCatchedActors.size(); ++i )
	{
		S_CATCH_ACTOR_INFO& CatchActorInfo = m_vlCatchedActors.at( i );
		if( CatchActorInfo.hCatchedActor )
		{
			ReleaseThisActor( CatchActorInfo );
		}
	}

	m_vlCatchedActors.clear();
}

void CDnMonsterActor::ProcessCatchActor( LOCAL_TIME LocalTime, float fDelta )
{
	vector<S_CATCH_ACTOR_INFO>::iterator iter = m_vlCatchedActors.begin();
	for( iter; iter != m_vlCatchedActors.end(); )
	{
		S_CATCH_ACTOR_INFO& CatchActorInfo = *iter;
		// When the grabbing player dies, release.
		// TODO: Must also release when this grabbed monster dies.
		if( CatchActorInfo.hCatchedActor && 
			false == CatchActorInfo.hCatchedActor->IsDie() &&
			false == IsDie() )
		{
			EtVector3 vCatchBonePos( 0.0f, 0.0f, 0.0f );
			EtMatrix matBoneWorld = GetBoneMatrix( m_strCatchBoneName.c_str() );
			memcpy_s( &vCatchBonePos, sizeof(EtVector3), &matBoneWorld._41, sizeof(EtVector3) );

			// Must place character Bip01 at vCatchBonePos.
			// Calculate the distance between this mob's vCatchBonePos and the target character's Bip01 
			// Apply that amount to the position.
			EtVector3 vPlayerCatchedPointPos( 0.0f, 0.0f, 0.0f );
			CatchActorInfo.hCatchedActor->GetObjectHandle()->SetCalcAni( true );
			matBoneWorld = CatchActorInfo.hCatchedActor->GetBoneMatrix( m_strTargetActorCatchBoneName.c_str() );
			CatchActorInfo.hCatchedActor->GetObjectHandle()->SetCalcAni( false );
			memcpy_s( &vPlayerCatchedPointPos, sizeof(EtVector3), &matBoneWorld._41, sizeof(EtVector3) );

			EtVector3 vDelta = vCatchBonePos - vPlayerCatchedPointPos;
			EtVector3 vNowCatchedActorPos = *CatchActorInfo.hCatchedActor->GetPosition();
			EtVector3 vCatchedActorPos = vNowCatchedActorPos + vDelta;
			CatchActorInfo.hCatchedActor->SetPosition( vCatchedActorPos );

			//CatchActorInfo.hCatchedActor->SetActionQueue( m_strCatchedActorAction.c_str() );

			// If performing another action, change to grab action.
			int iCurrentActionIndex = CatchActorInfo.hCatchedActor->GetCurrentActionIndex();
			if( iCurrentActionIndex != m_iCatchedActorActionIndex )
				CatchActorInfo.hCatchedActor->SetActionQueue( m_strCatchedActorAction.c_str() );

			++iter;
		}
		else
		{
			ReleaseThisActor( CatchActorInfo );
			iter = m_vlCatchedActors.erase( iter );
		}
	}

	if( false == IsSignalRange( STE_CatchActor ) )
	{
		if( false == m_setCatchCheckedActorIDs.empty() )
			m_setCatchCheckedActorIDs.clear();
	}
}
#endif // #ifdef PRE_ADD_MONSTER_CATCH

#if defined(PRE_FIX_51048)
void CDnMonsterActor::AddPassiveStateEffectInfo(PassiveStateEffectInfo& stateEffectInfo)
{
	m_PassiveStateEffectInfoList.push_back(stateEffectInfo);
}

void CDnMonsterActor::InitPassiveStateEffectInfo()
{
	m_PassiveStateEffectInfoList.clear();
}

void CDnMonsterActor::ApplyPassiveStateEffect()
{
	if (m_PassiveStateEffectInfoList.empty())
		return;

	std::list<PassiveStateEffectInfo>::iterator iter = m_PassiveStateEffectInfoList.begin();
	std::list<PassiveStateEffectInfo>::iterator endIter = m_PassiveStateEffectInfoList.end();

	for (; iter != endIter; ++iter)
	{
		PassiveStateEffectInfo& passiveStateEffect = (*iter);

		DnSkillHandle hSkill = FindSkill(passiveStateEffect.pParentSkill->iSkillID);
		if (hSkill)
		{
			// Modified for sending packets from server to client
			int iBlowID = CmdAddStateEffect( passiveStateEffect.pParentSkill, passiveStateEffect.emBlowIndex, -1, passiveStateEffect.szParam.c_str(), true ); // If Duration Time is -1, it is applied infinitely
			OnApplyPassiveSkillBlow( iBlowID );

			hSkill->SetAppliedPassiveBlows( true );
		}
	}
}

bool CDnMonsterActor::ApplyPassiveSkill( DnSkillHandle hSkill, bool isInitialize/* = false*/ )
{
	bool bPassiveBuf = false;

	if( CDnSkill::Passive == hSkill->GetSkillType() )
	{
		int iNumStateEffect = hSkill->GetStateEffectCount();
		for( int i = 0; i < iNumStateEffect; ++i )
		{
			CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
			if( CDnSkill::StateEffectApplyType::ApplySelf == pSE->ApplyType )
			{
				if( CDnSkill::DurationTypeEnum::Buff == hSkill->GetDurationType() )
				{
					bPassiveBuf = true;
					if( hSkill->IsSatisfyWeapon() )
					{
						//Store state effect info in a list due to packet ordering.
						if (GetEnablePassiveStateEffectList() == true)
						{
							PassiveStateEffectInfo passiveStateEffectInfo;
							passiveStateEffectInfo.pParentSkill = hSkill->GetInfo();
							passiveStateEffectInfo.emBlowIndex = (STATE_BLOW::emBLOW_INDEX)pSE->nID;
							passiveStateEffectInfo.nDurationTime = -1;
							passiveStateEffectInfo.szParam = pSE->szValue;

							AddPassiveStateEffectInfo(passiveStateEffectInfo);
						}
						else
						{
							// Modified for sending packets from server to client
							int iBlowID = CmdAddStateEffect( hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pSE->nID, -1, pSE->szValue.c_str(), true ); // If Duration Time is -1, it is applied infinitely
							OnApplyPassiveSkillBlow( iBlowID );

							hSkill->SetAppliedPassiveBlows( true );
						}

#ifndef _FINAL_BUILD
						char szTemp[256] = { 0, };
						WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
						OutputDebug( "[Passive buff skill applied: %d] \"%s\" StateEffectIndex: %d, Value:%s\n", hSkill->GetClassID(), szTemp, pSE->nID, pSE->szValue.c_str() );
#endif // #ifndef _FINAL_BUILD
					}
#ifndef _FINAL_BUILD
					else
					{
						char szTemp[256] = { 0, };
						WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
						OutputDebug( "[Passive buff skill skipped: %d] \"%s\" required weapon type mismatch.\n", hSkill->GetClassID(), szTemp );
					}
#endif // #ifndef _FINAL_BUILD
				}
#ifndef _FINAL_BUILD
				else
				{
					char szTemp[256] = { 0, };
					WideCharToMultiByte( CP_ACP, 0, hSkill->GetName(), -1, szTemp, _countof(szTemp), NULL, NULL );
					OutputDebug( "[Check skill data: %d] \"%s\" looks like a passive buff skill but Buff flag is not set; skipped.\n", hSkill->GetClassID(), szTemp );
				}
#endif // #ifndef _FINAL_BUILD
			}
		}
	}

	return bPassiveBuf;
}
#endif // PRE_FIX_51048


#if defined(PRE_FIX_61382)
void CDnMonsterActor::RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage, CDnDamageBase::SHitParam* pHitParam/* = NULL*/ )
{
	if (IsPuppetSummonMonster() == true)
	{
		DnActorHandle hOwnerActor = GetSummonerPlayerActor();

		//Only when damage transfer state effect is set? Transfer damage.
		if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_051) && hOwnerActor)
		{
#if defined(PRE_FIX_59347) && !defined(PRE_FIX_67656)
			hOwnerActor->SetApplyPartsDamage(true);
#endif // PRE_FIX_59347

			hOwnerActor->RequestDamageFromStateBlow(hFromBlow, iDamage, pHitParam);

#if defined(PRE_FIX_59347) && !defined(PRE_FIX_67656)
			hOwnerActor->SetApplyPartsDamage(false);
#endif // PRE_FIX_59347
		}
	}
	else
	{
		CDnActor::RequestDamageFromStateBlow(hFromBlow, iDamage, pHitParam);
	}
}
#endif // PRE_FIX_61382

#if defined(PRE_FIX_64312)
void CDnMonsterActor::SendApplySummonMonsterExSkill(int nBaseSkillID, int nLevel, int nSelectedType, DWORD dwMasterUniqueID, int nMasterExSkillID)
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write(&nBaseSkillID, sizeof(int));
	Stream.Write(&nLevel, sizeof(int));
	Stream.Write(&nSelectedType, sizeof(int));
	Stream.Write(&dwMasterUniqueID, sizeof(DWORD));
	Stream.Write(&nMasterExSkillID, sizeof(int));

	Send( eActor::SC_APPLY_SUMMON_MONSTER_EX_SKILL, &Stream );
}
#endif // PRE_FIX_64312
