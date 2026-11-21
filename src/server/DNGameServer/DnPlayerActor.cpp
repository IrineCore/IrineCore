#include "stdafx.h"
#include "DnPlayerActor.h"
#include "EtMatrixEx.h"
#include "DnWorld.h"
#include "MAMovementBase.h"
#include "DnWeapon.h"
#include "DnProjectile.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DnSkill.h"
#include "PerfCheck.h"
#include "DnDropItem.h"
#include "GameSendPacket.h"
#include "DnMonsterActor.h"
#include "DnStateBlow.h"
#include "MAAiScript.h"
#include "DnBlow.h"
#include "DnItemTask.h"
#include "DNLogConnection.h"
#include "DNGameDataManager.h"
#include "ScoreSystem.h"
#include "DNMissionSystem.h"
#include "DNDBConnectionManager.h"
#include "DnPlayerSpeedHackChecker.h"
#include "DnPlayerDoNotEnterChecker.h"
#include "DnPlayerPickupChecker.h"
#include "DnPlayerSkillChecker.h"
#include "DnPlayerActionChecker.h"
#include "DnPlayAniProcess.h"
#include "DNPvPPlayerAggroSystem.h"
#include "DNMonsterAggroSystem.h"
#include "DnChangeActionSetBlow.h"
#include "DnChangeActionStrProcessor.h"
#include "DNDBConnection.h"
#include "DnChangeStandActionBlow.h"
#include "DnActionSpecificInfo.h"
#include "MasterRewardSystem.h"
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRepositoryServer.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#include "DnCannonMonsterActor.h"
#include "DNPvPGameRoom.h"
#include "DnApplySEWhenTargetNormalHitProcessor.h"

#include "DnBubbleSystem.h"
#include "DnObserverEventMessage.h"
#include "DnBlockBlow.h"
#include "DnParryBlow.h"
#include "DnCooltimeParryBlow.h"
#include "PvPZombieMode.h"
#include "DNGameServerScriptAPI.h"
#include "DnOrderMySummonedMonsterBlow.h"
#include "IDnSkillUsableChecker.h"
#include "DnTransformBlow.h"
#ifdef IRINE_MOD_PLAYER_BOTS
#include "DnAIBotController.h"
#endif
#include "DnBasicBlow.h"
#include "DnCreateBlow.h"

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
#include "DnDPSReporter.h"
#endif

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
#include "DnSkillTask.h"
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "TotalLevelSkillSystem.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

float CDnPlayerActor::s_fRecoverySPTime = 5.f;

CDnPlayerActor::CDnPlayerActor( CMultiRoom *pRoom, int nClassID )
: CDnActor( pRoom, nClassID )
{
	m_pSession = NULL;

	CDnPlayerState::Initialize( m_nClassID );
	CDnActionBase::Initialize( this );
	MACP::Initialize( this );

	m_cMovePushKeyFlag = 0;
	m_bBattleMode = true;

	m_nWorldLevel = 0;

	m_nComboDelay = 0;
	m_nComboCount = 0;
	m_nTotalComboCount = 0;
	m_fRecoverySPDelta = 0.f;
	m_bCompleteCutScene = false;
	m_bCheckCompleteCutScene = false;
	m_bSkipCutScene = false;

	m_bGhost = false;
	m_uiStateBlowProcessAfterBit = 0;

	m_afLastEquipItemSkillDelayTime = 0.0f;
	m_afLastEquipItemSkillRemainTime = 0.0f;

	m_dwSyncDatumTick			= 0;
	m_dwSyncDatumSendTick		= 0;
	m_nVoiceChannelID = 0;
	m_pPlayerSpeedHackChecker	= new CDnPlayerSpeedHackChecker( this );
	m_pPlayerDoNotEnterChecker = new CDnPlayerDoNotEnterChecker( this );
	m_pPlayerPickupChecker = new CDnPlayerPickupChecker( this );
	m_pPlayerSkillChecker = new CDnPlayerSkillChecker( this );
	m_pPlayerActionChecker = new CDnPlayerActionChecker( this );

	memset( m_bCashSelfDeleteWeapon, 0, sizeof(m_bCashSelfDeleteWeapon) );
	memset( m_bWeaponViewOrder, 0, sizeof(m_bWeaponViewOrder) );

	m_iNowMaxProjectileCount = 0;
	m_iReservedProjectileCount = 0;

	m_LastEscapeTime = 0;

	m_dwLastBasicShootActionTime = 0;
	m_dwLastBasicShootCoolTime = 0;
	m_dwLastChargeShootTime = 0;
	m_bCheckProjectileSignalTerm = true;

	m_bUpdatedProjectileInfoFromCmdAction = false;
	m_fFrameSpeed = 1.0f;

	m_bPlayerCannonMode = false;

	m_nSwapSingleSkinActorID = -1;
	m_nMonsterMutationTableID = -1;

	m_hSwapOriginalHandle.Identity();
	m_pSwapOriginalAction = NULL;
	m_nDeathCount = 0;

	m_pBubbleSystem = new BubbleSystem::CDnBubbleSystem;
	this->RegisterObserver( m_pBubbleSystem );

	m_bTempSkillAdded = false;
	m_iTempChangedJob = 0;
	m_nInvalidPlayerCheckCounter = 0;

	m_bForceEnableRideByTrigger = true;

	m_nVehicleEffectIndex = 0;
	m_bVehicleMode = false;

	m_vecEventEffectList.clear();

	for( int i=0; i<Pet::Skill::Max; i++ )
		m_bDeletedPetSkill[i] = false;
	
	m_nAllowedSkill = 0;
	m_bRefreshTransformMode = false;
	m_bTransformMode = false;
	m_bShootMode = false;
	m_pAIBotController = NULL;

#ifdef _USE_VOICECHAT
	//	m_nVoiceUpdateCount = 0;
	m_nVoiceRotate = 0;
#endif

	m_MixedActionTimeStamp = 0;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelSkillSystem = NULL;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#ifdef PRE_ADD_COSTUME_SKILL
	m_nCostumeSkillID = 0;
#endif
}

CDnPlayerActor::~CDnPlayerActor()
{
	// If in cannon mode, release cannon ownership.
	if( IsCannonMode() )
	{
		static_cast<CDnCannonMonsterActor*>( m_hCannonMonsterActor.GetPointer() )->OnMasterPlayerActorDie();
	}

	//Reset crest skill effect
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( pSox )
	{
		for( int itr = 0; itr < CDnGlyph::GlyphSlotEnum_Amount; ++itr )
		{
			if( m_hGlyph[itr] )
			{
				if( CDnGlyph::PassiveSkill == pSox->GetFieldFromLablePtr( m_hGlyph[itr]->GetClassID(), "_GlyphType" )->GetInteger() )
				{
					int nSkillID = pSox->GetFieldFromLablePtr( m_hGlyph[itr]->GetClassID(), "_SkillID" )->GetInteger();
					DnSkillHandle hSkill = FindSkill( nSkillID );

					if( hSkill )
						hSkill->DelGlyphStateEffect( m_hGlyph[itr]->GetClassID() );
				}
			}
		}
	}

	// Clean up aura skills. Unlike other buffs, when the caster dies, aura effects applied to other characters must be removed.
	// Turn off aura skill or toggle skill if enabled.
	if( IsEnabledToggleSkill() )
		OnSkillToggle( m_hToggleSkill, false );

	if( IsEnabledAuraSkill() )
		OnSkillAura( m_hAuraSkill, false );

	for( int i=0; i<2; i++ ) SAFE_RELEASE_SPTR( m_hCashWeapon[i] );

	SAFE_RELEASE_SPTR( m_hSwapOriginalHandle );
	SAFE_DELETE( m_pSwapOriginalAction );

	SAFE_DELETE( m_pPlayerSkillChecker );
	SAFE_DELETE( m_pPlayerActionChecker );
	SAFE_DELETE( m_pPlayerDoNotEnterChecker ) ;
	SAFE_DELETE( m_pPlayerPickupChecker );
	SAFE_DELETE( m_pPlayerSpeedHackChecker );

	SAFE_DELETE( m_pBubbleSystem );

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	SAFE_DELETE(m_pTotalLevelSkillSystem);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#ifdef IRINE_MOD_PLAYER_BOTS
	SAFE_DELETE(m_pAIBotController);
#else
	m_pAIBotController = NULL;
#endif
}

MAMovementBase* CDnPlayerActor::CreateMovement()
{
	MAMovementBase* pMovement = new IBoostPoolMAWalkMovement();
	return pMovement;
}

bool CDnPlayerActor::Initialize()
{
	bool bResult = CDnActor::Initialize();

	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
	m_hObject->SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) | COLLISION_GROUP_DYNAMIC( 2 ) | COLLISION_GROUP_DYNAMIC( 3 ) );

	_ASSERT( m_pAggroSystem == NULL );
	if( GetGameRoom()->bIsPvPRoom() )
	{
		m_pAggroSystem = new CDNPvPPlayerAggroSystem( GetActorHandle() );
		_ASSERT( m_pAggroSystem != NULL );
	}

	m_mapTumbleHelper.insert( make_pair(GetElementIndex("Tumble_Front"), GetElementIndex("Move_Front")) );
	m_mapTumbleHelper.insert( make_pair(GetElementIndex("Tumble_Back"), GetElementIndex("Move_Back")) );
	m_mapTumbleHelper.insert( make_pair(GetElementIndex("Tumble_Left"), GetElementIndex("Move_Left")) );
	m_mapTumbleHelper.insert( make_pair(GetElementIndex("Tumble_Right"), GetElementIndex("Move_Right")) );

	// Sorceress Charge Missile is not implemented solely with the action tool and action system and has client-side handling, so the server has to match it.
	// Since it runs slightly differently from the system, chain the action likewise and set the action cooldown to the 1.5s charge time.
	// Setting only Move_Front as representative is fine. It just needs to be recognized as an exception in the hack check..
	m_mapTumbleHelper.insert( make_pair(GetElementIndex("ChargeShoot_Stand"), GetElementIndex("Charge_Front")) );

	m_pBubbleSystem->Initialize( GetActorHandle() );

	m_mapIcyFractionHitted.clear();

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	m_FrameSkipCallSkillProcess.SetFramePerSec( 10.0f );
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	m_pTotalLevelSkillSystem = new CDnTotalLevelSkillSystem(GetMySmartPtr());
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	return bResult;
}


void CDnPlayerActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	EtVector3 vPrevPos = m_Cross.m_vPosition;

	CDnActor::Process( LocalTime, fDelta );
	PROFILE_TIME_TEST_BLOCK_START( "CDnPlayerActor::Process" );

	m_pBubbleSystem->Process( LocalTime, fDelta );

	for( int i=0; i<2; i++ ) 
	{
		if( m_hCashWeapon[i] ) 
			m_hCashWeapon[i]->Process( LocalTime, fDelta );
	}

	MAPartsBody::PreProcess( LocalTime, fDelta );

	if( 0 < GetCantXZMoveSEReferenceCount() )
		m_vAniDistance.x = m_vAniDistance.z = 0.0f;

#ifdef PRE_ADD_MONSTER_CATCH
	if(!IsVehicleMode() && !m_hCatcherMonster )
#else
	if(!IsVehicleMode())
#endif // #ifdef PRE_ADD_MONSTER_CATCH
		m_pMovement->Process( LocalTime, fDelta );

	MAPartsBody::Process( m_Cross, LocalTime, fDelta );

	ProcessCombo( LocalTime, fDelta );
	ProcessRecoverySP( LocalTime, fDelta );

	m_pPlayerDoNotEnterChecker->Process( LocalTime, fDelta );
	if( IsAIBotActor() )
	{
#ifdef IRINE_MOD_PLAYER_BOTS
		if( !m_pAIBotController )
			m_pAIBotController = new CDnAIBotController( this );
		m_pAIBotController->Process( LocalTime, fDelta );
#endif
	}
	else
	{
		m_pPlayerSpeedHackChecker->Process( LocalTime, fDelta );
		m_pPlayerPickupChecker->Process( LocalTime, fDelta );
		m_pPlayerSkillChecker->Process( LocalTime, fDelta );
		m_pPlayerActionChecker->Process( LocalTime, fDelta );
	}

	ProcessCompanion( LocalTime, fDelta );
	ProcessNonLocalShootModeAction();
	if(m_bRefreshTransformMode)
		RefreshTransformMode();

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	if (m_pTotalLevelSkillSystem)
		m_pTotalLevelSkillSystem->Process(LocalTime, fDelta);
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	PROFILE_TIME_TEST_BLOCK_END();
}

void CDnPlayerActor::CmdPickupItem( PickupItemStruct* pStruct, DnDropItemHandle hDropItem/*=CDnDropItem::Identity()*/ )
{

	// Observers cannot PickUp
	if( GetActorHandle() && GetActorHandle()->bIsObserver() )
		return;
	// GM intrusion cannot PickUp either
	if( m_pSession && (m_pSession->bIsGMTrace() || m_pSession->bIsAIBot()) )
		return;

	if( !CDnDropItem::IsActive(GetRoom()) ) 
		return;

	DnDropItemHandle hResult;
	if( hDropItem )
		hResult = hDropItem;
	else
	{
		_DANGER_POINT();
		return;
	}

	if( hResult ) {
		if (hResult->IsReversionItem() && hResult->IsReversionLocked() == false)
		{
			DnActorHandle hBeneficiery;
			if (hResult->GetOwnerUniqueID() != -1)
			{
				hBeneficiery = CDnActor::FindActorFromUniqueID( GetRoom(), hResult->GetOwnerUniqueID() );
				if (hBeneficiery)
					CDnItemTask::GetInstance(GetRoom()).PickUpItem( hBeneficiery, hResult, ITEMLOOTRULE_NONE );
				return;
			}
			CDnItemTask::GetInstance(GetRoom()).PickUpItem( GetMySmartPtr(), hResult, ITEMLOOTRULE_NONE );
			return;
		}
		if( hResult->GetOwnerUniqueID() != -1 ) {
			DnActorHandle hOwner = CDnActor::FindActorFromUniqueID( GetRoom(), hResult->GetOwnerUniqueID() );
			if( hOwner ) {
				CDnItemTask::GetInstance(GetRoom()).PickUpItem( hOwner, hResult, ITEMLOOTRULE_NONE );
			}
			return;
		}
		else {
			eItemRank Rank = CDnItem::GetItemRank( hResult->GetItemID() );
			eItemTypeEnum Type = CDnItem::GetItemType( hResult->GetItemID() );
			TPARTYITEMLOOTRULE LootType = ITEMLOOTRULE_NONE;
			TITEMRANK LootRank = m_pRoom->GetPartyItemLootRank();

			switch( Type ) 
			{
				case ITEMTYPE_INSTANT:
					LootType = ITEMLOOTRULE_NONE;
					break;
				default:
					if (LootRank == ITEMRANK_NONE )
						LootType = m_pRoom->GetPartyItemLootRule();
					else
						LootType = ( Rank >= LootRank ) ? ITEMLOOTRULE_RANDOM : m_pRoom->GetPartyItemLootRule();
					break;
			}
			// Even if it is money, acquire it as normal.
			if( hDropItem->GetItemID() == 0 ) 
				LootType = ITEMLOOTRULE_NONE;

			TItemData *pItemData = g_pDataManager->GetItemData(hDropItem->GetItemID());
			if (pItemData)
			{
				if (pItemData->cReversion == ITEMREVERSION_BELONG)
					LootType = ITEMLOOTRULE_NONE;
			}

			// Force LootType setting in Ghoul mode
			if( GetRoom() && static_cast<CDNGameRoom*>(GetRoom())->bIsZombieMode() )
				LootType = ITEMLOOTRULE_NONE;

			switch( LootType ) 
			{
				case ITEMLOOTRULE_NONE:
				case ITEMLOOTRULE_OWNER:
					CDnItemTask::GetInstance(GetRoom()).PickUpItem( GetMySmartPtr(), hResult, LootType );
					break;
				case ITEMLOOTRULE_RANDOM:
				{
					int nLiveCount = CDnPartyTask::GetInstance(GetRoom()).GetPartyUserCount(CDNGameRoom::ePICKUPITEM);
					if( nLiveCount == 0 ) return;
					int nResultOffset = _rand(GetRoom())%nLiveCount;
					int nOffset = 0;
					DnActorHandle hResultActor;
					int nPartyCount = CDnPartyTask::GetInstance(GetRoom()).GetUserCount();
					for( int i=0; i<nPartyCount; i++ ) {
						CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(i);
						if( !pSession ) continue;
						if( pSession->bIsGMTrace() || pSession->bIsAIBot() ) continue;
						DnActorHandle hActor = pSession->GetActorHandle();
						if( !hActor || hActor->IsDie() ) continue;
						if( nOffset == nResultOffset ) {
							hResultActor = hActor;
							break;
						}
						nOffset++;
					}
					if( hResultActor ) 
						CDnItemTask::GetInstance(GetRoom()).PickUpItem( hResultActor, hResult, LootType );
				}
				break;
				case ITEMLOOTRULE_LEADER:
				{
					int nPartyCount = CDnPartyTask::GetInstance(GetRoom()).GetUserCount();
					DnActorHandle hResultActor;
					for( int i=0; i<nPartyCount; i++ ) {
						CDNGameRoom::PartyStruct *pPartyStruct = CDnPartyTask::GetInstance(GetRoom()).GetPartyData(i);
						if( !pPartyStruct ) continue;
						if( pPartyStruct->bLeader ) {
								hResultActor = ( pPartyStruct->pSession && !pPartyStruct->pSession->bIsAIBot() ) ? pPartyStruct->pSession->GetActorHandle() : CDnActor::Identity();
						}
					}
					if( hResultActor ) 
						CDnItemTask::GetInstance(GetRoom()).PickUpItem( hResultActor, hResult, LootType );
				}
				break;
				case ITEMLOOTRULE_INORDER:
				{
					CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(m_pRoom->GetCurrentItemLooterIdx());
					if (pSession && !pSession->bIsAIBot())
					{
						DnActorHandle hActor = pSession->GetActorHandle();
						CDnItemTask::GetInstance(GetRoom()).PickUpItem( hActor, hResult, LootType );
					}
				}
				break;
			}
		}
	}
}

void CDnPlayerActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Hit:
			{
			}
			break;
		case STE_Jump:
			{
				JumpStruct *pStruct = (JumpStruct *)pPtr;
				EtVector2 vVec( 0.f, 0.f );

				if( 0 == GetCantXZMoveSEReferenceCount() )
				{
					if( !pStruct->bIgnoreJumpDir ) {
						if( m_cMovePushKeyFlag & 0x01 ) vVec.x -= 1.f;
						if( m_cMovePushKeyFlag & 0x02 ) vVec.x += 1.f;
						if( m_cMovePushKeyFlag & 0x04 ) vVec.y += 1.f;
						if( m_cMovePushKeyFlag & 0x08 ) vVec.y -= 1.f;
						EtVec2Normalize( &vVec, &vVec );
					}
				}

				// Jump synchronization [2010/11/09 semozz]
				// When existing Velocity value needs reset on re-jump during jump
				// Must reset together with client to keep jump time synchronized.
				if( pStruct->bResetPrevVelocity ) {
 					SetVelocityY( 0.f );
					SetResistanceY( 0.f );
				}
				
				Jump( pStruct->fJumpVelocity, vVec );
				SetResistanceY( pStruct->fJumpResistance );
			}
			return;
		case STE_CustomAction:
			{
				CustomActionStruct *pStruct = (CustomActionStruct *)pPtr;

				ResetCustomAction();
				SetCustomAction( pStruct->szChangeAction, (float)pStruct->nChangeActionFrame );
			}
			return;
		case STE_PickupItem:
			{
			}
			return;
		case STE_RebirthAnyPlayer:
			{

				// [2011/02/11 semozz]
				// Prevent party revive on Sea Dragon map..
				CMultiRoom *pRoom = GetRoom();
				if (!pRoom)
					break;

				eDragonNestType _dragonNestType = CDnWorld::GetInstance(pRoom).GetDragonNestType();

				RebirthAnyPlayerStruct *pStruct = (RebirthAnyPlayerStruct*)pPtr;
				CDNUserSession *pSession = NULL;
				float fMinDist = FLT_MAX;

				for( DWORD i=0; i<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pParty = CDnPartyTask::GetInstance(GetRoom()).GetPartyData(i);
					if( !pParty || !pParty->pSession ) continue;
					if( pParty->pSession == m_pSession ) continue;
					if(  pParty->pSession->GetState() != SESSION_STATE_GAME_PLAY)
						continue;
					DnActorHandle hActor = pParty->pSession->GetActorHandle();
					if( !hActor ) continue;
					CDnPlayerActor *pPlayer = (CDnPlayerActor *)hActor.GetPointer();
					if( !pPlayer->IsDie() || !pPlayer->IsGhost() ) continue;					
					if( pParty->nUsableRebirthCoin == 0 ) continue;

					EtVector3 vVec = *GetPosition() - *hActor->GetPosition();
					float fDist = EtVec3LengthSq( &vVec );
					if( fDist < fMinDist ) {
						fMinDist = fDist;
						pSession = pParty->pSession;
					}
				}
				if( !pSession || fMinDist > (float)( pStruct->nRadius * pStruct->nRadius ) ) break;

				if( m_pSession->GetStatusData()->wCashRebirthCoin <= 0 )
				{
					GetUserSession()->SendRebirthCoin(ERROR_ITEM_REBIRTH_CASHCOIN_SHORT_FAIL, m_pPartyData->nUsableRebirthCoin, _REBIRTH_SELF, GetUserSession()->GetSessionID());
					break;
				}

				CDnItemTask::GetInstance(GetRoom()).RequestRebirthCoinUseAnyPlayer( m_pSession, pSession );
				UpdateRebirthPlayer();
			}
			break;

		case STE_CancelChangeStandActionSE:
			{
				CDnChangeStandActionBlow::ReleaseStandChangeSkill( GetActorHandle(), false );
			}
			break;
		case STE_CancelChangeStandEndActionSE:
			{
				CancelChangeStandEndActionSEStruct* pStruct = (CancelChangeStandEndActionSEStruct*)pPtr;
				CDnChangeStandActionBlow::ReleaseStandChangeSkill( GetActorHandle(), false, pStruct->szEndAction );
			}
			break;
		case STE_TriggerEvent:
			{
				TriggerEventStruct *pStruct = (TriggerEventStruct *)pPtr;
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventArea", -1 );
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "ActorHandle", GetSessionID() );
				CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventID", pStruct->nEventID );
				CDnWorld::GetInstance(GetRoom()).OnTriggerEventCallback( "CDnActor::TriggerEvent", LocalTime, 0.f );
			}
			break;

		case STE_ShootCannon:
			{
				ShootCannonStruct* pStruct = (ShootCannonStruct*)pPtr;
				if( IsCannonMode() )
				{
					// Set projectile to TargetPosition sent by client in CS_CANNONROTATESYNC packet.
					m_hCannonMonsterActor->UseSkill( pStruct->CannonMonsterSkillID );
				}
			}
			break;

			// #29925 Execute the specified action if the state effect specified in this signal is absent.
		case STE_ChangeActionSECheck:
			{
				ChangeActionSECheckStruct* pStruct = (ChangeActionSECheckStruct*)pPtr;
				if( false == IsAppliedThisStateBlow( (STATE_BLOW::emBLOW_INDEX)pStruct->nStateEffectID ) )
				{
					SetActionQueue( pStruct->szChangeAction );
				}
			}
			break;

			/*
			// Note: Since signals can be processed later than packets and cause skills in client-triggered actions to be skipped 
			// Modify to send as packet from client side.
		case STE_CanUseSkill:
			{
				CanUseSkillStruct* pStruct = (CanUseSkillStruct*)pPtr;
				m_bUseSignalSkillCheck = (pStruct->bUseSignalSkillCheck ? TRUE : FALSE);
				SetSignalSkillCheck( pStruct->CheckType, m_bUseSignalSkillCheck );
			}
			break;
			*/

		case STE_OrderMySummonedMonster:
			{
				OrderMySummonedMonsterStruct* pStruct = (OrderMySummonedMonsterStruct*)pPtr;
				OrderUseSkillToMySummonedMonster(pStruct);

			}
			break;
	}
	CDnActor::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}

void CDnPlayerActor::OnDrop( float fCurVelocity )
{
	if( IsAir() ) {
		if( !IsHit() ) {

			//////////////////////////////////////////////////////////////////////////
			// #32977 - After starting Top Spinning skill, before the Top Spinning action signal is processed
			// When OnDrop is called, STE_Jump in Top Spinning is not processed, preventing the motion from continuing.
			// So for now..
			// If current Frame is 0 and STE_Jump signal is at frame 0, prevent switching to _Landing action
			// Keep maintaining the current action.
			//////////////////////////////////////////////////////////////////////////
			float fCurrentFrame = CDnActionBase::GetCurFrame();
			float fPrevFrame = CDnActionBase::m_fPrevFrame;
			ActionElementStruct *pStruct = GetElement(GetCurrentAction());
			if (pStruct)
			{
				bool hasJumpSignal = false;
				CEtActionSignal *pSignal = NULL;
				for (int i = 0; i < (int)pStruct->pVecSignalList.size(); ++i)
				{
					pSignal = pStruct->pVecSignalList[i];
					if (pSignal && pSignal->GetSignalIndex() == STE_Jump)
					{
						hasJumpSignal = true;
						break;
					}
				}

				if (hasJumpSignal)
				{
					//Skip if STE_Jump and actionQueue exist.. (Probably switches to the next action automatically?..)
					if (false == m_szActionQueue.empty())
					{
						return;
					}
					//If the current action has STE_Jump and is at frame 0, allow STE_Jump to be called..
					else if (false == m_szAction.empty() && fPrevFrame == 0.0f)
					{
						return;
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////

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
			else {// Put this in because otherwise it bugs out in the air. Just matching and filling it in for now
				if( GetVelocity()->y != 0.f )
					SetActionQueue( "Stand", 0, 0.f, 0.f, true, false );
			}

			SetMovable( false );
		}
		else {
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

void CDnPlayerActor::OnStop( EtVector3 &vPosition )
{
	if( IsProcessSkill() ) return;

	CmdStop( "Stand" );
}

void CDnPlayerActor::OnFall( float fCurVelocity )
{
	// #31056 If Move and Air, it is an aerial movement action so do not process Fall (using Top Spinning when falling from high places).
	if( !(IsMove() && IsAir()) &&
		(IsStay() || IsMove()) &&
		! IsFloorCollision() )
	{
		// Also check movement angle to compensate for looking like falling when going down stairs, etc.
		EtVector3 vDir = *GetPosition() - *GetPrevPosition();
		EtVec3Normalize( &vDir, &vDir );
		float fDot = EtVec3Dot( &EtVector3( 0.f, 1.f, 0.f ), &vDir );
		float fAngle = EtToDegree( EtAcos( fDot ) );

		if( fCurVelocity < -5.f && fAngle > 155.f ) {
			ActionElementStruct *pStruct = GetElement( "Jump" );
			if( pStruct ) {
				SetActionQueue( "Jump", 0, 6.f, (float)pStruct->dwLength / 2.f );
			}
		}
	}
}

void CDnPlayerActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{

#if defined( PRE_ADD_SECONDARY_SKILL )
	if( GetUserSession() && GetUserSession()->GetSecondarySkillRepository() )
		static_cast<CSecondarySkillRepositoryServer*>(GetUserSession()->GetSecondarySkillRepository())->CancelManufacture();
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	if( IsCannonMode() )
	{
		HitParam.szActionName.clear();
		HitParam.vViewVec = *GetLookDir();
		HitParam.vResistance = EtVector3( 0.0f, 0.0f, 0.0f );
		HitParam.vVelocity = EtVector3( 0.0f, 0.0f, 0.0f );
	}

	int nSeed = CRandom::Seed(GetRoom());
	_srand( GetRoom(), nSeed );
	
	INT64 nTemp = GetHP();
	CDnActor::OnDamage( pHitter, HitParam, pHitStruct );
	INT64 nDamage = nTemp - GetHP();

	INT64 biKillerCharDBID = 0;
	if( GetGameRoom() )
	{
		DnActorHandle hHitter = pHitter ? pHitter->GetActorHandle() : CDnActor::Identity();
		GetGameRoom()->OnDamage( GetActorHandle(), hHitter, nDamage );	

		if( GetGameRoom()->GetGameTask() )
			GetGameRoom()->GetGameTask()->OnDamage( GetActorHandle(), hHitter, nDamage );

#ifdef IRINE_MOD_PLAYER_BOTS
		if( m_pAIBotController )
			m_pAIBotController->OnDamaged( hHitter, nDamage );
		if( GetGameRoom()->HasAIBots() && hHitter && hHitter->IsMonsterActor() && nDamage > 0 )
			CDnAIBotController::ObserveBossHitOnActor( hHitter.GetPointer(), this, nDamage );
#endif

		if (IsDie() && hHitter && hHitter->IsPlayerActor()){
			CDnPlayerActor *pPlayer = (CDnPlayerActor *)hHitter.GetPointer();
			biKillerCharDBID = pPlayer->GetUserSession()->GetCharacterDBID();
		}
	}

	if( m_HitParam.bSuccessNormalDamage ) 
	{
		m_nComboDelay = 0;
		m_nComboCount = 0;
	}

	UpdateAttackedCPPoint( pHitter, m_HitParam.HitType );

	ResetCustomAction();
	RequestDamage( pHitter, nSeed, nDamage );

	switch( pHitter->GetDamageObjectType() ) 
	{
		case DamageObjectTypeEnum::Actor:
			{
				DnActorHandle hActor = pHitter->GetActorHandle();

				if( m_pAggroSystem )
					m_pAggroSystem->OnDamageAggro( hActor, HitParam, (int)nDamage );
			}
			break;
	}
}

void CDnPlayerActor::CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	MovePos( vPos, false );

	// #31940 Parrying triggers the parry action while the state effect ticks, so when hit while moving and parrying triggers
	// If client holds direction key and sends move packets, CS_CMDMOVE arrives and changes action to Move_ series here, so handle as exception.
	if( m_szAction == "Skill_Parrying" ||
		m_szActionQueue == "Skill_Parrying" )
		return;

	SetActionQueue( szActionName, nLoopCount, fBlendFrame );
}

void CDnPlayerActor::CmdStop( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame )
{
	MAMovementBase *pMovement = GetMovement();
	if( !pMovement ) return;

	pMovement->ResetMove();

	// #31940 Parrying triggers the parry action while the state effect ticks, so when hit while moving and parrying triggers
	// Exception handling because CS_CMDSTOP packet arrives as character stops immediately and changes action to Stand here.
	if( m_szAction == "Skill_Parrying" ||
		m_szActionQueue == "Skill_Parrying" )
		return;

	SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame );
}

void CDnPlayerActor::CmdPassiveSkillAction( int nSkillID, const char *szActionName, int nLoopCount /*= 0*/, float fBlendFrame /*= 3.f*/, float fStartFrame /*= 0.0f*/, bool bChargeKey /*= false*/, bool bCheckOverlapAction /*= true */, bool bOnlyCheck/* = false*/ )
{
	// Note: Passive skill cannot activate when incapacitated.
	// Even though the action is blocked in SetAction() and won't go out anyway, skill usage triggers cooldown, so block signal processing.
	if( GetCantActionSEReferenceCount() > 0 ) return;

	if( false == bOnlyCheck )
	{
		DnSkillHandle hSkill = FindSkill( nSkillID );
		if( !hSkill ) return;

		// Note: For skills like kick, the action tool is configured so the same skill triggers again near the end while already active 
		//		 Even if it is the same skill, interrupt and exit.
		if( m_hProcessSkill /*&& m_hProcessSkill != hSkill*/ ) 
		{
			if( false == (IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn()) )
			{
				m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
			}

			m_hProcessSkill.Identity();
		}

		// Since the key is held on the server side too, treat the skill as continuing to execute until the action changes.
		if( bChargeKey )
		{
			hSkill->SetPassiveSkillLength( -1.0f );
		}
		else
		{
			// Account for cases where a passive skill action also has a next action.
			// Since the client only tracks one next action as reference, the server also tracks only one.
			DWORD dwActionLength = 0;
			ActionElementStruct* pElement = GetElement( szActionName );
			if( pElement )
			{
				if( pElement->dwLength <= (DWORD)fStartFrame )
					fStartFrame = pElement->dwLength*1.f;

				dwActionLength = pElement->dwLength - (DWORD)fStartFrame;
			}
			else
			{
				fStartFrame = 0.0f;
			}

			// #25042 If Stand is the next action, it is not a passive skill action, so exclude its duration.
			if( pElement->szNextActionName != "Stand" &&
				0 == strstr( pElement->szNextActionName.c_str(), "_Landing") )
			{
				ActionElementStruct* pNextElement = GetElement( pElement->szNextActionName.c_str() );
				if( pNextElement )
					dwActionLength += pNextElement->dwLength;
			}
			
			hSkill->SetPassiveSkillLength( (float)dwActionLength / s_fDefaultFps );

			// Active Type skills (like Eagle's Descent) may also be used as Passive Type.
			// Later, if _CheckActionWithProcessPassiveActionSkill checks whether ActionLength is in progress
			// If Passive Type, ChaningPassiveSkill is set to true, but it is false because this is Active Type.
			// So set ChaningPassiveKill to true here.
			if( hSkill->GetSkillType() == CDnSkill::Active ) // Set only when Active.
				hSkill->SetChaningPassiveSkill( true );
		}

		// Ignore UsableCheck when an active skill packet arrives via InputHasPassiveSkill.
		if( CDnSkill::Active == hSkill->GetSkillType() )
		{
			m_bUseSignalSkillCheck = true;
			for( int i = 0; i < 3; ++i )
				SetSignalSkillCheck( i, true );
		}

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
		// Allows a 0.5s margin to account for server cooldown error, though
		// Because zone transitions or other new cases can cause a drift of 0.5 seconds or more from the client
		// Record the last skill usage timestamp; if interval is greater than the skill cooltime specified in data 
		// Reset the cooldown stored in the server's skill object. (#19737)
		hSkill->UpdateSkillCoolTimeExactly();
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

		CDnSkill::UsingResult eResult = CanExecuteSkill( hSkill );
		if( eResult == CDnSkill::UsingResult::Success )
		{
			ExecuteSkill( hSkill, CDnActionBase::m_LocalTime, 0.f );
			hSkill->FromInputHasPassive();
			SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame, bCheckOverlapAction );
		}
		else {
			((CDnPlayerSkillChecker*)m_pPlayerSkillChecker)->OnInvalidUseSkill( nSkillID, eResult );
		}

		// Restore UsableCheck after ignoring it when an active skill packet arrives via InputHasPassiveSkill.
		if( CDnSkill::Active == hSkill->GetSkillType() )
		{
			m_bUseSignalSkillCheck = false;
			for( int i = 0; i < 3; ++i )
				SetSignalSkillCheck( i, false );
		}

		// Fill in the action name for passive/instant skills used by the player.
		hSkill->SetPassiveSkillActionName( szActionName );
	}
	else
		SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame, bCheckOverlapAction );

	m_fDownDelta = 0.f;
}

// If structs in this packet change, PvP drop-in code also needs updating, so notify me (Kimbap).
int CDnPlayerActor::CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit/* = false */, bool bCheckCanBegin/* = true*/ , bool bEternity /*= false*/ )
{
	if( emBlowIndex < STATE_BLOW::BLOW_NONE || emBlowIndex >= STATE_BLOW::BLOW_MAX )
	{
		if( pParentSkill )
			g_Log.Log(LogType::_ERROR, L"[CDnActor::CmdAddStateEffect] SkillID:%d, STATE_BLOW:%d\r\n", pParentSkill->iSkillID, emBlowIndex );
		else
			g_Log.Log(LogType::_ERROR, L"[CDnActor::CmdAddStateEffect] STATE_BLOW:%d\r\n", emBlowIndex);
		return -1;
	}

	int iID = CDnActor::CmdAddStateEffect( pParentSkill, emBlowIndex, nDurationTime, szParam, bOnPlayerInit, bCheckCanBegin , bEternity );
	if( -1 == iID ) 
		return -1;

	DnBlowHandle hAddedBlow = m_pStateBlow->GetStateBlowFromID( iID );

	const CPacketCompressStream* pPacket = hAddedBlow->GetPacketStream( szParam, bOnPlayerInit );
	Send( eActor::SC_CMDADDSTATEEFFECT, const_cast<CPacketCompressStream*>(pPacket) );

	if( GetGameRoom() )
		GetGameRoom()->OnCmdAddStateEffect( hAddedBlow->GetParentSkillInfo() );

	CheckAndRegisterObserverStateBlow( hAddedBlow );

	return iID;
}

void CDnPlayerActor::CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo /*= true */ )
{	
	if( m_pStateBlow->IsApplied( emBlowIndex ) )
	{
		if( bRemoveFromServerToo )
			CDnActor::CmdRemoveStateEffect( emBlowIndex );	

		SendRemoveStateEffect( emBlowIndex );
	}
}

void CDnPlayerActor::SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BYTE pBuffer[32];
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &emBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );

	Send( eActor::SC_CMDREMOVESTATEEFFECT, &Stream );
}

void CDnPlayerActor::SendRemoveStateEffectGraphic( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &emBlowIndex, sizeof(int) );

	Send( eActor::SC_BLOW_GRAPHIC_ERASE, &Stream );
}

void CDnPlayerActor::CmdToggleBattle( bool bBattleMode )
{
	if( bBattleMode == true ) 
	{
		if( !m_hWeapon[0] ) {
			m_bBattleMode = false;
			return;
		}
	}
	if( bBattleMode != m_bBattleMode ) 
	{
		SetBattleMode( bBattleMode );

		bool bSkipAction = false;

#if defined(PRE_ADD_50907)
		bool bSkipChangeAction = IsSkipChangeWeaponAction();
		if (bSkipChangeAction == true)
			return;
#endif // PRE_ADD_50907
		if(m_bShootMode)
		{
			if( m_bBattleMode == true ) SetActionQueue( "MOD_PullOut_Weapon" );
			else SetActionQueue( "MOD_PutIn_Weapon" ); 
			
			// ShootMode is only used in combat state, so the action of drawing a weapon in Normal state cannot be set in GetchangeShootaction and is placed here instead.
			bSkipAction = true;
		}
		if(!bSkipAction)
		{
			if( !IsDie() ) 
			{
				if( m_bBattleMode == true ) SetActionQueue( "PullOut_Weapon" );
				else SetActionQueue( "PutIn_Weapon" );
			}
		}
	}
}

void CDnPlayerActor::CmdAddExperience( TExpData &ExpData, int nLogCode, INT64 biFKey )
{
	if( IsAIBotActor() )
		return;

	int nExp = ExpData.nExperience;
	int nEventExp = ExpData.nEventExperience;
	int nPCBangExp = ExpData.nPcBangExperience;
	int nVIPExp = ExpData.nVIPExperience;
	int nPromoExp = ExpData.nPromotionExperience;

	TPlayerCommonLevelTableInfo* pPlayerCommonLevelTableInfo = g_pDataManager->GetPlayerCommonLevelTable(GetLevel());
	if( pPlayerCommonLevelTableInfo )
	{
		//Increase by 1.5x here (doing it in one place should be easier to manage... dyss)
		nExp = (int)(nExp * pPlayerCommonLevelTableInfo->fAddGainExp);
		nEventExp = (int)(nEventExp * pPlayerCommonLevelTableInfo->fAddGainExp);
		nPCBangExp = (int)(nPCBangExp * pPlayerCommonLevelTableInfo->fAddGainExp);
		nVIPExp = (int)(nVIPExp * pPlayerCommonLevelTableInfo->fAddGainExp);
		nPromoExp = (int)(nPromoExp * pPlayerCommonLevelTableInfo->fAddGainExp);
	}

	nExp += ExpData.nItemExperience;
	nExp += ExpData.nGuildExp;

#if defined(PRE_ADD_WEEKLYEVENT)
	if (CDnWorld::GetInstance(GetRoom()).GetMapType() != EWorldEnum::MapSubTypeNest){
		int nThreadID = GetGameRoom()->GetServerID();

		float fEventValue = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, GetClassID(), WeeklyEvent::Event_5, nThreadID);
		if (fEventValue != 0)
			nExp += (int)(nExp * fEventValue);
	}
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)

	int nPrevLevel = GetLevel();
	int nTotalExp = nExp + nEventExp + nPCBangExp + nVIPExp + nPromoExp;
	AddExperience( nTotalExp, nLogCode, biFKey );		//If not _KR, pcbangexp is always 0.

	// No need to send it once leveled up.
	if( GetLevel() == nPrevLevel ) {
		BYTE pBuffer[128];
		CPacketCompressStream Stream( pBuffer, 128 );

		Stream.Write( &m_nExperience, sizeof(int) );
		Stream.Write( &nExp, sizeof(int) );		
		Stream.Write( &nEventExp, sizeof(int) );
		Stream.Write( &nPCBangExp, sizeof(int) );
#if defined(PRE_ADD_VIP)
		Stream.Write( &nVIPExp, sizeof(int) );
#endif	// #if defined(PRE_ADD_VIP)
		Stream.Write( &nPromoExp, sizeof(int) );

		Send( eActor::SC_ADDEXP, GetMySmartPtr(), &Stream );
	}
}

void CDnPlayerActor::CmdAddCoin( INT64 nCoin, int nLogCode, int nFKey, bool bSync )
{
	INT64 nChangeCoin = nCoin;
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	if( nLogCode == DBDNWorldDef::CoinChangeCode::PickUp )
	{
		float fIncGoldRate = 0.0f;
		if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_266))
		{
			DNVector(DnBlowHandle) vlBlows;
			GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_266, vlBlows);
			{
				int nCount = (int)vlBlows.size();
				for (int i = 0; i < nCount; ++i)
				{
					DnBlowHandle hBlow = vlBlows[i];
					if (hBlow && hBlow->IsEnd() == false)
					{
						fIncGoldRate += hBlow->GetFloatValue();
					}
				}
			}
			nChangeCoin += (INT64)(nChangeCoin * fIncGoldRate);
			if( !m_pSession->CheckMaxCoin(nChangeCoin) )
				return;
		}
	}	
#endif

#if defined( _GAMESERVER )
	if( nLogCode != DBDNWorldDef::CoinChangeCode::PickUp )
		g_Log.Log(LogType::_ERROR, m_pSession, L"CmdAddCoin PrevCoin=%I64d  ChangeCoin=%I64d PickUpCoint=%I64d LogType=%d\r\n", m_pSession->GetCoin(), nChangeCoin, m_pSession->GetPickUpCoin(), nLogCode);
#endif

	if( nLogCode == DBDNWorldDef::CoinChangeCode::PickUp )
	{
		m_pSession->AddPickUpCoin( nChangeCoin );
		nLogCode = 0;	// Re-initialize so it is not saved to DB
	}

	// Caution!! bSync and AddCoin's bWarehouse are different, but since it's just whether to send a packet or not, I'll just use it -> Changed to Send...
	if( m_pSession && m_pSession->GetItem() ){
		if (nChangeCoin > 0)
			m_pSession->AddCoin( nChangeCoin, nLogCode, nFKey, bSync );
		else if (nChangeCoin < 0)
			m_pSession->DelCoin( -nChangeCoin, nLogCode, nFKey, bSync );
	}
}

void CDnPlayerActor::OnDispatchMessage( CDNUserSession *pSession, DWORD dwActorProtocol, BYTE *pPacket )
{

	switch( dwActorProtocol ) {
		case eActor::CS_CMDMOVE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nActionIndex;
				EtVector3 vPos, vXVec;
				EtVector2 vZVec, vLook;
				char cFlag;
				DWORD dwGap;
				int nMoveSpeed;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &cFlag, sizeof(char) );
				Stream.Read( &nMoveSpeed, sizeof(int) );

				ActionElementStruct *pStruct = GetElement( nActionIndex );
				if( pStruct == NULL ) break;

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );
				m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );
#ifdef PRE_ADD_CHECK_MOVESPEED_HACK
				m_pPlayerSpeedHackChecker->OnSyncMoveSpeed( nMoveSpeed );
#endif
				if( CheckSkillAction(pStruct->szName.c_str() ) == true )
				{
					((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
					break;
				}

				Look( vLook, false );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );

				SetPosition( vPos );
//				SetMagnetPosition( vPos );

#ifdef _USE_VOICECHAT
				m_nVoiceRotate = (int)EtToDegree( acos( EtVec2Dot( &EtVector2( 0.f, 1.f ), &vZVec ) ) );
				if( vZVec.x > 0.0f )
					m_nVoiceRotate = 360 - m_nVoiceRotate;
				m_nVoiceRotate = (m_nVoiceRotate + 90) % 360;
				m_pSession->SetVoicePos((int)vPos.x, (int)vPos.y, (int)vPos.z, m_nVoiceRotate);
#endif

				m_cMovePushKeyFlag = cFlag;
				float fXSpeed = 0.f, fZSpeed = 0.f;
				if( cFlag & 0x01 ) fXSpeed = -100000.f;
				if( cFlag & 0x02 ) fXSpeed = 100000.f;
				if( cFlag & 0x04 ) fZSpeed = 100000.f;
				if( cFlag & 0x08 ) fZSpeed = -100000.f;
				vPos += ( vXVec * fXSpeed );
				vPos += ( EtVec2toVec3( vZVec ) * fZSpeed );

				CmdMove( vPos, pStruct ? pStruct->szName.c_str() : "", -1, 8.f );
			}
			break;
		case eActor::CS_CMDSTOP:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos;
				DWORD dwGap;
				bool bReset, bForce;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &bReset, sizeof(bool) );
				Stream.Read( &bForce, sizeof(bool) );

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );
				m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );

				/*
				std::string szActionName = GetCurrentAction();
				if( EtVec2Length( &( EtVector2( vPos.x, vPos.z ) - EtVector2( GetPosition()->x, GetPosition()->z ) ) ) > 100.f ) {
					if( GetState() != ActorStateEnum::Move ) szActionName = "Move_Front";
				}

				CmdMove( vPos, szActionName.c_str(), -1, CDnActionBase::m_fQueueBlendFrame );
				*/
				if( IsProcessSkill() && bForce ) CancelUsingSkill();
				SetPosition( vPos );
				OnStop( vPos );

#ifdef _USE_VOICECHAT
				m_pSession->SetVoicePos((int)vPos.x, (int)vPos.y, (int)vPos.z, m_nVoiceRotate);
#endif
			}
			break;
		case eActor::CS_CMDACTION:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nActionIndex, nLoopCount;
				float fBlendFrame;
 				EtVector3 vXVec, vPos;
				EtVector2 vLook, vZVec;
				DWORD dwGap;
				bool bFromStateBlow = false;
				bool bSkillChain = false;
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &m_cMovePushKeyFlag, sizeof(char) );
				Stream.Read( &bFromStateBlow, sizeof(bool) );
				Stream.Read( &bSkillChain, sizeof(bool) );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );

				ResetMove();

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
				Look( vLook );
				SetMagnetPosition( vPos );

				ActionElementStruct *pStruct = GetElement( nActionIndex );
				if( pStruct == NULL ) break;

				if( bSkillChain )
				{
					if( false == IsValidSkillChain( GetCurrentActionIndex(), nActionIndex ) )
						return;
				}
				else
				{
					bool bCheckStandAction = CDnChangeStandActionBlow::CheckUsableAction( GetActorHandle(), true, pStruct->szName.c_str() );
		
					if( IsProcessSkill() == true && CheckSkillAction( pStruct->szName.c_str() ) == true || bCheckStandAction == false ) 
					{
						ActionElementStruct *pCurrentActionElement = GetElement( GetCurrentActionIndex() );
						if( pCurrentActionElement )
						{
							bool bCorrectInputAction = false;

							for( DWORD iSignal = 0; iSignal < pCurrentActionElement->pVecSignalList.size(); ++iSignal )
							{
								CEtActionSignal* pSignal = pCurrentActionElement->pVecSignalList.at( iSignal );
								if( STE_Input == pSignal->GetSignalIndex() )
								{
									InputStruct* pInputStruct = (InputStruct*)(pSignal->GetData());
									if( strcmp( pInputStruct->szChangeAction, pStruct->szName.c_str() ) == NULL )
									{
										bCorrectInputAction = true;
										break;
									}
								}
							}

							if( bCorrectInputAction == false )
							{
								((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
								return;
							}
						}
					}
				}

				if( m_pBasicAttackInfo && strstr(pStruct->szName.c_str(), "Attack") )
				{
					DNVector(int) nVecCheckActionList;
					nVecCheckActionList.push_back( GetCurrentActionIndex() );
					if( IsCustomAction() ) 
						nVecCheckActionList.push_back( GetCustomActionIndex() );

					// If there is a next action, add it to the verification list with a margin of about 10 frames.
					// Even if the server frame is slow and stand is set as next action to accept input, on the server 
					// Sometimes treated as a hack because next action is not set.
					ActionElementStruct* pCurrentActionElement = GetElement( GetCurrentActionIndex() );
					if( pCurrentActionElement )
					{
						if( 0 < pCurrentActionElement->szNextActionName.length() )
						{
							// Frame gap with server can grow depending on client manipulation, so remove server-side frame check for now.
							//if( pCurrentActionElement->dwLength - (DWORD)CDnActionBase::m_fFrame < 10 )
							{
								int iNextActionIndex = GetElementIndex( pCurrentActionElement->szNextActionName.c_str() );
								if( -1 < iNextActionIndex )
									nVecCheckActionList.push_back( iNextActionIndex );
							}
						}
					}

					// When spamming normal attack packets via cmdaction, checking all possible transition actions can basically block it, and
					// Since the actual hit frame of the action occurs slightly later and cmdaction packet cannot start an action from an arbitrary frame
					// This method works. However, mixedaction below can set an arbitrary action start frame, so block that case separately.
					bool bValid = false;
					for( DWORD k=0; k<nVecCheckActionList.size(); k++ ) 
					{
						map<int, vector<CDnActionSpecificInfo::S_BASIC_ATTACK_INPUT_SIGNAL_INFO> >::const_iterator iter = m_pBasicAttackInfo->mapBasicAttackInfo.find( nVecCheckActionList[k] );
						if( iter != m_pBasicAttackInfo->mapBasicAttackInfo.end() )
						{
							const vector<CDnActionSpecificInfo::S_BASIC_ATTACK_INPUT_SIGNAL_INFO>& vlInputSignals = iter->second;
							for( int i = 0; i < (int)vlInputSignals.size(); ++i )
							{
								const CDnActionSpecificInfo::S_BASIC_ATTACK_INPUT_SIGNAL_INFO& InputSignalInfo = vlInputSignals.at( i );
								if( strstr( pStruct->szName.c_str(), InputSignalInfo.strChangeActionName.c_str() ) )
								{
									bValid = true;
									break;
								}
							}
						}
					}
					if( !bValid ) 
					{
						((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
						return;
					}
				}

				// If it's a normal shoot action, check against input signal interval time to see if it's currently valid.
				if( m_pProjectileCountInfo ) {
					map<int, DWORD>::const_iterator iterCooltime = m_pProjectileCountInfo->mapBasicShootActionCoolTime.find( nActionIndex );
					if( m_pProjectileCountInfo->mapBasicShootActionCoolTime.end() != iterCooltime )
					{
						DWORD dwTerm = timeGetTime() - m_dwLastBasicShootActionTime;
						DWORD dwCoolTime = DWORD((float)m_dwLastBasicShootCoolTime/m_fFrameSpeed);
						if( dwTerm < dwCoolTime )
						{
							// Normal attack packet arrived within less than the set time interval; this is a hack.
	#ifndef _FINAL_BUILD
							OutputDebug( "CDnPlayerActor-CS_CMDACTION: normal shoot action arrived faster than allowed interval; suspected hack.\n" );
	#endif // #ifndef _FINAL_BUILD
							return;
						}

						m_dwLastBasicShootActionTime = timeGetTime();
						m_dwLastBasicShootCoolTime = iterCooltime->second;
					}
				}

				if( IsCustomAction() ) ResetCustomAction();

				// If not a skill chain, check whether the current cmdaction packet is an action used by a skill. #26467
				// Flow reaches here only after signal data validation for skill chain completes above, so it is safe to check.
				if( false == bSkillChain )
					_CheckProcessSkillActioncChange( pStruct->szName.c_str() );

				CmdAction( pStruct->szName.c_str(), nLoopCount, fBlendFrame, false, false, bSkillChain );

				// Changed to update immediately when CS_CMDACTION packet arrives.
				_UpdateMaxProjectileCount( nActionIndex );
				m_bUpdatedProjectileInfoFromCmdAction = true;

				// Action triggered by a state effect on either the server or client.
				if( bFromStateBlow )
					m_pStateBlow->OnCmdActionFromPacket( pStruct->szName.c_str() );
				//
#ifdef _USE_VOICECHAT
				m_nVoiceRotate = (int)EtToDegree( acos( EtVec2Dot( &EtVector2( 0.f, 1.f ), &vZVec ) ) );
				if( vZVec.x > 0.0f )
					m_nVoiceRotate = 360 - m_nVoiceRotate;
				m_nVoiceRotate = (m_nVoiceRotate + 90) % 360;
				m_pSession->SetVoicePos((int)vPos.x, (int)vPos.y, (int)vPos.z, m_nVoiceRotate);
#endif
			}
			break;
		case eActor::CS_CMDMIXEDACTION:
			{
				// Note: If a normal action is performed while using an active-passive skill, check and terminate the active skill.
				// To prevent remaining state effects from affecting normal attacks. 
				if( IsProcessSkill() ) 
				{
					if( (m_hProcessSkill->GetSkillType() == CDnSkill::Passive || m_hProcessSkill->GetSkillType() == CDnSkill::AutoPassive) &&
						 m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
					{
						m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
						m_hProcessSkill.Identity();
					}
				}

				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos;

				int nActionIndex, nMaintenanceBone, nActionBone;
				float fFrame, fBlendFrame;
				EtVector3 vLook;
				DWORD dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &nActionBone, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &nMaintenanceBone, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &fFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
				Stream.Read( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
				Stream.Read( &m_cMovePushKeyFlag, sizeof(char) );

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );
				ActionElementStruct *pStruct = GetElement( nActionIndex );
				if( pStruct == NULL ) break;

				// Basically, Skill_-related packets arriving via cmdaction are invalid (except skill chain input).
				// Ignore if not in the currently running exception action set.
				if( CheckSkillAction(pStruct->szName.c_str()) == true )
				{
					((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
					break;
				}

				bool bCheckStandAction = CDnChangeStandActionBlow::CheckUsableAction( GetActorHandle(), true, pStruct->szName.c_str() );
				if( bCheckStandAction == false )
				{
					((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
					return;
				}
	
				// If it's a normal shoot action, check against input signal interval time to see if it's currently valid.
				// Check normal attack action cooldown only when performing a new action (packet arrives with frame 0).
				// If hacking is used, sending 0 frames triggers action cooldown, and if sending more than 0 frames 
				// Do not update projectile count or other anti-hack verification info. Any projectiles fired afterward become invalid.
				// Since cmdaction always starts at frame 0, only MIXEDACTION needs to check the frame sent by the client.
				bool bActionCoolTimeChecked = false;

				if( m_pProjectileCountInfo )
				{
					map<int, DWORD>::const_iterator iterCooltime = m_pProjectileCountInfo->mapBasicShootActionCoolTime.find( nActionIndex );
					if( m_pProjectileCountInfo->mapBasicShootActionCoolTime.end() != iterCooltime )
					{
						if( 0.0f == fFrame )
						{
							DWORD dwTerm = timeGetTime() - m_dwLastBasicShootActionTime;
							DWORD dwCoolTime = DWORD((float)m_dwLastBasicShootCoolTime/m_fFrameSpeed);
							if( dwTerm < dwCoolTime )
							{
								// Normal attack packet arrived within less than the set time interval; this is a hack.
#ifndef _FINAL_BUILD
								OutputDebug( "CDnPlayerActor-CS_CMDMIXEDACTION: normal shoot action arrived faster than allowed interval; suspected hack.\n" );
#endif // #ifndef _FINAL_BUILD
								return;
							}

							m_dwLastBasicShootActionTime = timeGetTime();
							m_dwLastBasicShootCoolTime = iterCooltime->second;

							bActionCoolTimeChecked = true;
						}
					}
				}

				if( false == bActionCoolTimeChecked )
				{
					// When frame value is present, it is when Warrior, Cleric, etc. move after one normal attack. Spamming this packet with an arbitrary normal attack action at a frame with a hit signal 
					// Defend against hacks that keep sending hits by alternating Stand actions (via CmdStop or CmdMove) with MixedAction
					// Since the server has no way to verify, check using cooldown.
					LOCAL_TIME MixedActionCoolTime = 500;
					if( CDnActionBase::m_LocalTime - m_MixedActionTimeStamp < MixedActionCoolTime )
					{
						if( CDnActionBase::m_LocalTime - m_MixedActionTimeStamp < 0 )
							m_MixedActionTimeStamp = CDnActionBase::m_LocalTime;

						// If received beyond a certain time interval, treat as hack.
						return;
					}
					m_MixedActionTimeStamp = CDnActionBase::m_LocalTime;
				}


				std::string szAction = pStruct->szName;
				m_szActionBoneName = GetBoneName(nActionBone);
				m_szMaintenanceBoneName = GetBoneName(nMaintenanceBone);

				if( IsCustomAction() ) ResetCustomAction();
				SetCustomAction( szAction.c_str(), fFrame );

				// MixedAction does not trigger OnChangeAction(), so it must be updated manually.
				//_UpdateMaxProjectileCount( nActionIndex, true );

				// If the frame value is non-zero, only change the player's direction.
				if( 0.0f == fFrame )
				{
					_UpdateMaxProjectileCount( nActionIndex );
				}
			}
			break;
		case eActor::CS_PROJECTILE:
			{
				// If there is a next action and it is almost at the end frame, it is right before the action changes, so on the client 
				// Even if the action changed to fire a projectile, the server might not have caught up. Allow one extra shot.
				// Rapidly looping actions like Pinpoint Shot can cause issues.
#ifdef PRE_ADD_LOOP_PROJECTILE
				ActionElementStruct* pActionElement = GetElement( m_szAction.c_str() );
				if( pActionElement && false == pActionElement->szNextActionName.empty() )
				{
					// TODO: Additionally check mixed animation with IsCustomAction() 
					// Accurately update projectile verification info. Leaving it like this while moving backward... 
					// When shooting an arrow, only Move_Back is updated so the projectile count becomes 0.
					int iNextActionElementIndex = GetElementIndex( pActionElement->szNextActionName.c_str() );
					if( GetCurrentActionIndex() == iNextActionElementIndex  )
						_UpdateMaxProjectileCount( iNextActionElementIndex );
				}
#endif // #ifdef PRE_ADD_LOOP_PROJECTILE

				CDnProjectile* pProjectile = CDnProjectile::CreatePlayerProjectileFromClientPacket( GetMySmartPtr(), pPacket );
			}
			break;
		case eActor::CS_CMDLOOK:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				EtVector2 vLook, vZVec;
				EtVector3 vXVec;
				bool bForce;

				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &bForce, sizeof(bool) );

				Look( vLook, bForce );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
			}
			break;
		case eActor::CS_USESKILL:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nSkillTableID = 0;
				int nEnchantSkillID = 0;		// #38294 Not used on game server. Used on village server to notify info when using EX skill.
				BYTE cLevel;
				EtVector2 vLook, vZVec;
				EtVector3 vXVec;
#ifdef PRE_ADD_POSITION_SYNC_BY_SKILL_USAGE
				EtVector3 vPos;
#endif
				bool bUseApplySkillItem = false;
				bool bAutoUseFromServer = false;		// Data not used on game server.


				Stream.Read( &nSkillTableID, sizeof(int) );
				Stream.Read( &cLevel, sizeof(char) );
				Stream.Read( &bUseApplySkillItem, sizeof(bool) );
				Stream.Read( m_abSignalSkillCheck, sizeof(m_abSignalSkillCheck) );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &bAutoUseFromServer, sizeof(bool) );
				Stream.Read( &nEnchantSkillID, sizeof(int) );
#ifdef PRE_ADD_POSITION_SYNC_BY_SKILL_USAGE
				Stream.Read(&vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				SetPosition( vPos );
#endif
	
				Look( vLook, true );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );
				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );

				// When skills are chained consecutively, CanMove can be false,
				// In this case, skills with a Movable activation condition could be dropped, so set it to true.
				// Packet arrived because it is already usable on the client (under normal conditions). #14127
				if( false == m_bMovable )
					m_bMovable = true;
				////////////////////////////////////////////////////////////////////////////////////////////////////

				// Note: On game server, since the client sends item usage with ApplySkill attached, it is already handled at that time..
				// Therefore ignore here. This flag is for applying to other clients.
				if( false == bUseApplySkillItem )
				{
					bool bSkipAirCondition = false;

					if(CheckSkipAirCondition(nSkillTableID))
					{
						SetSignalSkillCheck(2,true);
						bSkipAirCondition = true;
					}
					// When state is Air and the next action has GroundMovable condition, client-server gap can cause it to fail the Air check and be canceled.

					m_bUseSignalSkillCheck = (m_abSignalSkillCheck[ 0 ] || m_abSignalSkillCheck[ 1 ] || m_abSignalSkillCheck[ 2 ]);

					if( m_hProcessSkill ) 
					{
						// #25154 Aura skills trigger OnEnd when the aura is turned off.
						if( false == m_hProcessSkill->IsAuraOn() )
						{
							m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
						}
						else
						{
							// #26002 If the aura skill has ended, clear the list of state effects applied to self.
							// Otherwise it affects other skills when they are used. 
							// Since the original intent of ApplyStateEffect signal is not to apply if the skill action is interrupted by getting hit,
							// If the user immediately uses another skill and the action changes before the self-applied state effect signal runs
							// Naturally state effects will not apply, so handle it this way assuming this was considered in action design.
							if( IsEnabledAuraSkill() )
								ClearSelfStateSignalBlowQueue();
						}

						m_hProcessSkill.Identity();
					}
					

					if (IsExistSkill(nSkillTableID, cLevel) == false) {
						((CDnPlayerSkillChecker*)m_pPlayerSkillChecker)->OnInvalidUseSkill(nSkillTableID, CDnSkill::UsingResult::NoExistSkill);
						// Hack!!!
						break;
					}
#if defined( PRE_FIX_CHANGESTAND_HACK )
					CDnChangeStandActionBlow::ReleaseStandChangeSkill( GetActorHandle(), true  );
#endif
		
					CDnSkill::UsingResult eResult = UseSkill( nSkillTableID );
					if( eResult != CDnSkill::UsingResult::Success ) 
					{
						// If skill use fails due to remaining cooldown, treat it as a hack.
						// For the rest, the server already considers it hit the moment the client uses the skill and notifies the client of the result.
						// Reset cooldown on client side.
						((CDnPlayerSkillChecker*)m_pPlayerSkillChecker)->OnInvalidUseSkill( nSkillTableID, eResult );
						switch( eResult )
						{
							case CDnSkill::FailedByCooltime:
								break;

							case CDnSkill::FailedByUsableChecker:
								{
#ifdef PRE_FIX_69469
									if( GetProcessSkill() ) CancelUsingSkill();
									SetAction( "Stand", 0.f, 3.f );
#endif
									const char* pCurrentAction = GetCurrentAction();
									int iCurrentActionIndex = GetElementIndex( pCurrentAction );

									BYTE pBuffer[ 32 ] = { 0 };
									CPacketCompressStream LocalStream( pBuffer, 32 );

									LocalStream.Write( &nSkillTableID, sizeof(int) );
									LocalStream.Write( &eResult, sizeof(CDnSkill::UsingResult) );
									LocalStream.Write( &iCurrentActionIndex, sizeof(int) );

									Send( eActor::SC_SKILLUSING_FAILED, &LocalStream );
								}
								break;
						}

					}

					if(bSkipAirCondition) 
						SetSignalSkillCheck(2,false); // Release again.


					// If it is an equipment item skill,
					if( m_hProcessSkill && m_hProcessSkill->IsEquipItemSkill() )
					{
						m_afLastEquipItemSkillDelayTime = m_hProcessSkill->GetDelayTime();
						m_afLastEquipItemSkillRemainTime = m_hProcessSkill->GetElapsedDelayTime();
					}


					m_bUseSignalSkillCheck = false;
					ZeroMemory( m_abSignalSkillCheck, sizeof(m_abSignalSkillCheck) );
				}

			}
			break;
		case eActor::CS_VIEWSYNC:
			{
				if( GetGameRoom() && GetGameRoom()->GetGameTask() && !GetGameRoom()->GetGameTask()->IsSyncComplete() ) break;
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos, vXVec;
				EtVector2 vZVec, vLook;
				DWORD dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );

				Look( vLook, false );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );

				m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );

				SetPosition( vPos );
				// SetMagnetPosition( vPos );
			}
			break;
		case eActor::CS_CMDTOGGLEBATTLE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				bool bBattle;
				Stream.Read( &bBattle, sizeof(bool) );

				CmdToggleBattle( bBattle );
			}
			break;
		case eActor::CS_CMDPASSIVESKILLACTION:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nActionIndex, nLoopCount, nSkillID;
				BYTE cLevel;
				float fBlendFrame;
				float fStartFrame;
				EtVector3 vXVec, vPos;
				EtVector2 vLook, vZVec;
				bool bChargeKey = false;
				Stream.Read( &nSkillID, sizeof(int));
				Stream.Read( &cLevel, sizeof(char) );
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
				Stream.Read( &fStartFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.0f );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Read( &m_cMovePushKeyFlag, sizeof(char) );
				Stream.Read( &bChargeKey, sizeof(bool) );

				bool bOnlyCheck = false;
				Stream.Read( &bOnlyCheck, sizeof(bool) );

				EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

				ResetMove();

				SetMoveVectorX( vXVec );
				SetMoveVectorZ( EtVec2toVec3( vZVec ) );
				Look( vLook );
				//SetMagnetPosition( vPos );
				SetPosition( vPos );

				ActionElementStruct *pStruct = GetElement( nActionIndex );
				if( pStruct == NULL )
				{
					// Hack!!
					break;
				}

				// Custom action info is also needed for hack checks, so extract the info a few lines below and reset it.
				//if( IsCustomAction() ) ResetCustomAction();

				// Tumble-type actions are not in Move action on the server, so they get caught by the hack check below and fail to execute.
				// The reason the server is in Stand action state when a Tumble action request arrives here is before this packet arrives 
				// Because CMDSTOP packet arrives first, and the signal to transition to Tumble is in the Move action.
				// Would trigger the hack check. Since the server cannot determine this currently, let Tumble-type actions pass through.
				// However, since the skillid from the client cannot be trusted, replace it with the related passive skill id when a Tumble-type action arrives 
				// Let it pass through. Then CmdInputHasPassiveSkill will also check skill cooldown and ownership, catching unauthorized use.
				int iCurrentActionIndex = GetCurrentActionIndex();
				vector<int> vlCheckActionIndices;
				vlCheckActionIndices.push_back( iCurrentActionIndex );

				// #23245 When state effects directly manipulate actions like parrying series, server only enqueues action and frame update... 
				// Since a passive skill use request for an action already changed on the client may arrive beforehand, consider queued actions as well.
				int iQueuedActionIndex = -1;
				if( false == m_szActionQueue.empty() )
				{
					iQueuedActionIndex = GetElementIndex( m_szActionQueue.c_str() );
					vlCheckActionIndices.push_back( iQueuedActionIndex );
				}

				// If in a mixed action, include that action as well.
				if( IsCustomAction() )
				{
					int iCustomActionIndex = GetCustomActionIndex();
					vlCheckActionIndices.push_back( iCustomActionIndex );
					
					ResetCustomAction();
				}

				// Use tumblehelper only when the current action is stand due to cmdstop.
				bool bNowStand = (0 != strstr( m_szAction.c_str(), "Stand" ));

				// Use tunble helper even in Move state so any tumbling action can be performed regardless of direction while moving.
				// Account for cases where client moves sideways and immediately presses shift+forward to dash in another direction, sending packet here.
				// Otherwise it will not match the move action signal in the current direction, be treated as a hack, and not be used.
				bool bNowMove = IsMove();

				bool bValid = false;
				for( int k = 0; k < (int)vlCheckActionIndices.size(); ++k )
				{
					int iCheckActionIndex = vlCheckActionIndices.at( k );

					if( bNowStand || bNowMove )
					{
						map<int, int>::iterator iterTumble = m_mapTumbleHelper.find( nActionIndex );
						if( m_mapTumbleHelper.end() != iterTumble )
						{
							// Since Tumble is allowed from any move action, set current action to this so Tumble passes the hack check as valid.
							iCheckActionIndex = iterTumble->second;
						}
					}

					if( !m_pPassiveSkillInfo || ( m_pPassiveSkillInfo && m_pPassiveSkillInfo->mapPassiveSkillInfo.empty() ) )
						continue;

					// First, check if the action from the packet can transition from the current action to an inputhaspassive skill.
					map<int, vector<CDnActionSpecificInfo::S_PASSIVESKILL_SIGNAL_INFO> >::const_iterator iter = m_pPassiveSkillInfo->mapPassiveSkillInfo.find( iCheckActionIndex );
					if( m_pPassiveSkillInfo->mapPassiveSkillInfo.end() != iter )
					{
						const vector<CDnActionSpecificInfo::S_PASSIVESKILL_SIGNAL_INFO>& vlPassiveSkillSignalInfos = iter->second;

						// If there is an exact action name match, that signal has priority; substring match is next in priority.
						for( int i = 0; i < (int)vlPassiveSkillSignalInfos.size(); ++i )
						{
							const CDnActionSpecificInfo::S_PASSIVESKILL_SIGNAL_INFO& PassiveSkillInfo = vlPassiveSkillSignalInfos.at( i );
							if( PassiveSkillInfo.strChangeActionName == pStruct->szName ||
								PassiveSkillInfo.strEXSkillChangeActionName == pStruct->szName )
							{
								// Action name in request from client is a passive skill action that can transition from current action, however 
								// Double-check whether it is an owned passive skill.
								nSkillID = PassiveSkillInfo.iSkillID;
								if( IsExistSkill( PassiveSkillInfo.iSkillID ) )
								{
									// Case where a passive skill is enhanced by an enhancement passive skill. 
									// Normally, the action of the enhancement passive skill should arrive in the packet.
									DnSkillHandle hSkill = FindSkill( PassiveSkillInfo.iSkillID );
									if( hSkill->IsEnchantedSkill() )
									{
										if( PassiveSkillInfo.strEXSkillChangeActionName == pStruct->szName )
											bValid = true;
									}
									else
										bValid = true;

									// In the case of Charge Shot, measure action cooldown separately.
									if( strstr( pStruct->szName.c_str(), "ChargeShoot_" ) )
									{
										if( timeGetTime() - m_dwLastChargeShootTime < 1500 )
										{
											// Hack that keeps sending Charge Shot action via passive skill packets.
											bValid = false;
										}
										else
											m_dwLastChargeShootTime = timeGetTime();
									}
								}
								break;
							}
						}

						// When no matching action name is found.
						if( false == bValid )
						{
							for( int i = 0; i < (int)vlPassiveSkillSignalInfos.size(); ++i )
							{
								const CDnActionSpecificInfo::S_PASSIVESKILL_SIGNAL_INFO& PassiveSkillInfo = vlPassiveSkillSignalInfos.at( i );
								if( strstr( pStruct->szName.c_str(), PassiveSkillInfo.strChangeActionName.c_str() ) )		// Sorceress uses suffixes like _Book and _Orb, so check by substring containment.
								{
									// Action name in request from client is a passive skill action that can transition from current action, however 
									// Double-check whether it is an owned passive skill.
									nSkillID = PassiveSkillInfo.iSkillID;
									if( IsExistSkill( PassiveSkillInfo.iSkillID ) )
									{
										// Case where a passive skill is enhanced by an enhancement passive skill. 
										// Normally, the action of the enhancement passive skill should arrive in the packet.
										DnSkillHandle hSkill = FindSkill( PassiveSkillInfo.iSkillID );
										if( hSkill->IsEnchantedSkill() )
										{
											if( PassiveSkillInfo.strEXSkillChangeActionName == pStruct->szName )
												bValid = true;
										}
										else
											bValid = true;

										// In the case of Charge Shot, measure action cooldown separately.
										if( strstr( pStruct->szName.c_str(), "ChargeShoot_" ) )
										{
											if( timeGetTime() - m_dwLastChargeShootTime < 1500 )
											{
												// Hack that keeps sending Charge Shot action via passive skill packets.
												bValid = false;
											}
											else
												m_dwLastChargeShootTime = timeGetTime();
										}
									}
								}

								if( bValid )
									break;
							}
						}

						if( bValid )
							break;
					}
				}


#if defined( PRE_FIX_CHANGESTAND_HACK )
				CDnChangeStandActionBlow::ReleaseStandChangeSkill( GetActorHandle(), true  );
#endif

				if( false == bValid )
				{
					// Hack!! Attempting to execute an action that cannot transition from current action and is not in owned passive skill signals.
					// Or trying to execute an action of a passive skill not owned.. 
					// It's a hack.
#ifdef PRE_FIX_SKILL_FAILED_BY_ACTION
#ifdef PRE_FIX_69469
					if( GetProcessSkill() ) CancelUsingSkill();
					SetAction( "Stand", 0.f, 3.f );
#endif
					const char* pCurrentAction = GetCurrentAction();
					int iCurrentActionIndex = GetElementIndex( pCurrentAction );

					BYTE pBuffer[ 32 ] = { 0 };
					CPacketCompressStream LocalStream( pBuffer, 32 );

					CDnSkill::UsingResult eResult = CDnSkill::FailedByInvailedAction;

					LocalStream.Write( &nSkillID, sizeof(int) );
					LocalStream.Write( &eResult, sizeof(CDnSkill::UsingResult) );
					LocalStream.Write( &iCurrentActionIndex, sizeof(int) );
					
					Send( eActor::SC_SKILLUSING_FAILED, &LocalStream );
#endif

					return;
				}
		
				// If it passes the hack test above, the remaining nLoopCount, fBlendFrame, fStartFrame, etc. are used as is
				// If this causes an issue, use the data in the signal as-is.
				CmdPassiveSkillAction( nSkillID, pStruct->szName.c_str(), nLoopCount, fBlendFrame, fStartFrame, bChargeKey, false, bOnlyCheck );
				m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );
			}
			break;

		case eActor::CS_POSREV:
			{
				if( GetGameRoom() && GetGameRoom()->GetGameTask() && !GetGameRoom()->GetGameTask()->IsSyncComplete() ) break;
				CPacketCompressStream Stream( pPacket, 128 );
				EtVector3 vPos;
				bool bMove;
				DWORD dwGap;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &bMove, sizeof(bool) );

				_ASSERT( m_pPlayerSpeedHackChecker );
				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );
				m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );

				SetPosition( vPos );
				if( bMove ) {
					EtVector3 vXVec;
					EtVector2 vZVec;
					Stream.Read( &vZVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
					EtVec3Cross( &vXVec, &EtVector3( 0.f, 1.f, 0.f ), &EtVec2toVec3( vZVec ) );

					SetMoveVectorX( vXVec );
					SetMoveVectorZ( EtVec2toVec3( vZVec ) );

					if( EtVec3LengthSq( GetMovePos() ) > 0.f ) {
						float fXSpeed = 0.f, fZSpeed = 0.f;
						if( m_cMovePushKeyFlag & 0x01 ) fXSpeed = -100000.f;
						if( m_cMovePushKeyFlag & 0x02 ) fXSpeed = 100000.f;
						if( m_cMovePushKeyFlag & 0x04 ) fZSpeed = 100000.f;
						if( m_cMovePushKeyFlag & 0x08 ) fZSpeed = -100000.f;
						vPos += ( vXVec * fXSpeed );
						vPos += ( EtVec2toVec3( vZVec ) * fZSpeed );
						MovePos( vPos, false );
					}
				}
			}
			break;
		case eActor::CS_ONDROP:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				EtVector3 vPos;
				DWORD dwGap;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &dwGap, sizeof(DWORD) );

				_ASSERT( m_pPlayerSpeedHackChecker );
				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );

				SetPosition( vPos );
				SetJumpMovement( EtVector2( 0.f, 0.f ) );
			}
			break;

		case eActor::CS_CMDREMOVESTATEEFFECT:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				STATE_BLOW::emBLOW_INDEX emBlowIndex;
				Stream.Read( &emBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );

				// Because other clients also need to know about the state effect removal packet requested by the client
				// Re-broadcast. It is already removed on the client that requested state effect removal.
				CmdRemoveStateEffect( emBlowIndex );
				break;
			}
			break;
#if defined(PRE_FIX_57706)
		case eActor::CS_CMDREMOVESTATEEFFECTFROMID:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				int nServerBlowID = 0;
				Stream.Read( &nServerBlowID, sizeof(int) );

				//Only client knows the removal time change, so client sends packet to server, and server
				//Broadcast so other clients also remove the state effect.
				CmdRemoveStateEffectFromID(nServerBlowID);
				break;
			}
			break;
#endif // PRE_FIX_57706

		case eActor::CS_CMDTOGGLEWEAPONORDER:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				int nEquipIndex;
				bool bViewCash;

				Stream.Read( &nEquipIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &bViewCash, sizeof(bool) );

				CmdToggleWeaponViewOrder( nEquipIndex, bViewCash );
			}
			break;

		case eActor::CS_CMDTOGGLEPARTSORDER:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				int nEquipIndex;
				bool bViewCash;

				Stream.Read( &nEquipIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &bViewCash, sizeof(bool) );

				if( nEquipIndex == HIDEHELMET_BITINDEX ) CmdToggleHideHelmet( bViewCash );
				else CmdTogglePartsViewOrder( nEquipIndex, bViewCash );

				if (GetUserSession() && GetUserSession()->GetPartyID() > 0) {
					if( nEquipIndex == CASHEQUIP_HELMET || nEquipIndex == CASHEQUIP_EARRING || nEquipIndex == HIDEHELMET_BITINDEX ) {
						for (DWORD i = 0; i < m_pSession->GetGameRoom()->GetUserCount(); i++) {
							CDNGameRoom::PartyStruct *pStruct = m_pSession->GetGameRoom()->GetPartyData(i);
							if (pStruct == NULL) continue;
							pStruct->pSession->SendPartyMemberPart(m_pSession);
						}
					}
				}
			}
			break;

		case eActor::CS_CMDPICKUPITEM:
		{
			if( IsDie() )
				break;
			if( IsInvalidPlayerChecker() ) break;

			CPacketCompressStream Stream( pPacket, 128 );

			DWORD dwUniqueID;
			int nSignalIndex;
			EtVector3 vPos;
			int	nRange = 100;

			Stream.Read( &dwUniqueID, sizeof(DWORD) );
			Stream.Read( &nSignalIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
			Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

			CEtActionSignal *pSignal = GetSignal( m_nActionIndex, nSignalIndex );
			if( pSignal && pSignal->GetSignalIndex() == STE_PickupItem ) {
				PickupItemStruct *pSignalStruct = (PickupItemStruct *)pSignal->GetData();
				nRange = pSignalStruct->nRange;
			}

			m_pPlayerSpeedHackChecker->OnSyncPosition( vPos );
			SetPosition( vPos );

			if( GetGameRoom() )
			{
				DnDropItemHandle hDropItem = GetGameRoom()->FindDropItem( dwUniqueID );
				if( hDropItem )
				{
					((CDnPlayerPickupChecker*)m_pPlayerPickupChecker)->OnPickupDist( vPos, hDropItem );
					if( m_pSession && m_pSession->GetItem() && m_pSession->GetItem()->GetPetEquip() )//If a pet is summoned among the player's equipped items
					{
						const TVehicle* pPet = m_pSession->GetItem()->GetPetEquip();

						// Measure distance (allow 2x tolerance)
						float fDist = EtVec2Length( &(EtVector2(hDropItem->GetPosition()->x,hDropItem->GetPosition()->z)-EtVector2(vPos.x,vPos.z) ) );
						if( fDist > pPet->nRange*2.f )
							return;
					}
					else
					{
						// Measure distance (allow 2x tolerance)
						float fDist = EtVec2Length( &(EtVector2(hDropItem->GetPosition()->x,hDropItem->GetPosition()->z)-EtVector2(vPos.x,vPos.z) ) );
						if( fDist > nRange*2.f )
							return;
					}

					PickupItemStruct Struct;
					Struct.nRange = nRange;
					CmdPickupItem( &Struct, hDropItem );
				}
			}
			break;
		}

		case eActor::CS_CMDESCAPE:
			{
				// Verify here whether it should actually take effect.
				if( m_LastEscapeTime > 0 && CDnActionBase::m_LocalTime - m_LastEscapeTime < 1000 * 60 * 1 ) break;

				CPacketCompressStream Stream( pPacket, 128 );
				EtVector3 vPos;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

				SetPosition( vPos );
				SetStaticPosition( vPos );
				SetPrevPosition( vPos );
				if( m_pPlayerSpeedHackChecker ) m_pPlayerSpeedHackChecker->ResetInvalid();
				if( m_pPlayerDoNotEnterChecker ) m_pPlayerDoNotEnterChecker->ResetInvalid();

				CmdEscape( vPos );
			}
			break;

		case eActor::CS_CANNONPOSSESS_REQ:
			{
				// Request from client to occupy a cannon. 
				// Also send the result of the request.

				CPacketCompressStream Stream( pPacket, 128 );
				DWORD dwCannonMonsterActorID = 0;
				MatrixEx Cross;

				Stream.Read( &dwCannonMonsterActorID, sizeof(DWORD) );
				Stream.Read( &Cross, sizeof(MatrixEx) );

				bool bSuccess = false;

				// Assign to cannon monster.
				CDnActor *pActor = CDnActor::FindActorFromUniqueID( GetRoom(), dwCannonMonsterActorID );
				if( pActor && pActor->GetActorType() == ActorTypeEnum::Cannon )
				{
					CDnCannonMonsterActor* pCannonActor = static_cast<CDnCannonMonsterActor*>( pActor );

					// Check if this character is near the cannon monster sent by client.
					// If distance is farther than Press Circle, it might be a hack.. Could give a slight tolerance value.
					bool bValid = false;
					float fDistanceSQ = EtVec3LengthSq( &EtVector3(*pCannonActor->GetPosition() - *GetPosition()) );
					float fPressCircleDist = float(GetUnitSize() + pCannonActor->GetUnitSize());
					float fPressCircleDistSQ = fPressCircleDist * fPressCircleDist;
					if( fPressCircleDistSQ <= fDistanceSQ + 1000.0f  || fPressCircleDistSQ - 1000.f <= fDistanceSQ) 
					{
						// Whether already occupied.
						if( false == pCannonActor->IsPossessed() )
						{
							m_bPlayerCannonMode = true;
						
							pCannonActor->SetMasterPlayerActor( GetMySmartPtr() );
							m_hCannonMonsterActor = pCannonActor->GetMySmartPtr();
							m_Cross = Cross;
							bSuccess = true;

							// Cannon occupation succeeded.. send response with required info..
							char acBuffer[ 64 ] = { 0 };
							CPacketCompressStream Result( acBuffer, sizeof(acBuffer) );
							Result.Write( &bSuccess, sizeof(bool) );
							Result.Write( &dwCannonMonsterActorID, sizeof(DWORD) );
							Result.Write( &m_Cross.m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
							Result.Write( &m_Cross.m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
							Result.Write( &m_Cross.m_vYAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
							Result.Write( &m_Cross.m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
							Send( eActor::SC_CANNONPOSSESS_RES, &Result );

							// Set not to be pushed back while in cannon state. Must be reset when dismounting the cannon.
							SetWeight(0.f);
							SetPressLevel(-1);

#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
							DNTableFileFormat* pTableCannon = GetDNTable( CDnTableDB::TCANNON );

							if( pTableCannon->IsExistItem( pCannonActor->GetClassID() ) )
							{
								const char* pCannonActionName = pTableCannon->GetFieldFromLablePtr( pCannonActor->GetClassID(), "_StandName" )->GetString();
								CmdAction(pCannonActionName);
							}
							else
							{
								CmdAction( "Stand_Cannon" );
							}
#else
							// Change to cannon action.
							CmdAction( "Stand_Cannon" );
#endif
						}
					}
				}

				// Cannon occupation failed.. send response.
				if( false == bSuccess )
				{
					char acBuffer[ 64 ] = { 0 };
					CPacketCompressStream Result( acBuffer, sizeof(acBuffer) );
					Result.Write( &bSuccess, sizeof(bool) );
					Send( eActor::SC_CANNONPOSSESS_RES, &Result );
				}
			}
			break;

		case eActor::CS_CANNONRELEASE:
			{
				DWORD dwCannonMonsterID = 0;
				MatrixEx Cross;

				CPacketCompressStream Result( pPacket, 64 );
				Result.Read( &dwCannonMonsterID, sizeof(DWORD) );
				Result.Read( &Cross.m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Result.Read( &Cross.m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				Result.Read( &Cross.m_vYAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				Result.Read( &Cross.m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				
				CDnActor* pActor = CDnActor::FindActorFromUniqueID( GetRoom(), dwCannonMonsterID );
				if( pActor && pActor->GetActorType() == ActorTypeEnum::Cannon )
				{
					CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>(pActor);
					pCannonMonster->ClearMasterPlayerActor();
					m_Cross = Cross;
					
					EndCannonMode();
				}
			}
			break;

		case eActor::CS_CANNONROTATESYNC:
			{
				_ASSERT( IsCannonMode() );
				if( false == IsCannonMode() )
					break;

				CPacketCompressStream Stream( pPacket, 64 );
				DWORD dwCannonMonsterID = 0;
				EtVector3 vDirection( 0.0f, 0.0f, 0.0f );

				Stream.Read( &dwCannonMonsterID, sizeof(DWORD) );
				Stream.Read( &vDirection, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

				CDnActor* pActor = CDnActor::FindActorFromUniqueID( GetRoom(), dwCannonMonsterID );
				if( pActor && pActor->GetActorType() == ActorTypeEnum::Cannon )
				{
					CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>(pActor);
					EtVec3Normalize( &vDirection, &vDirection );		// Normalize once since the data was transmitted compressed.
					
					pCannonMonster->SetCannonLookDirection(&vDirection); // Rotha - Configured to rotate the cannon barrel instead of the cannon itself. The barrel only manages the viewpoint.


					m_Cross.m_vZAxis = pCannonMonster->GetMatEx()->m_vZAxis;
					m_Cross.MakeUpCartesianByZAxis();

					// Push by the press circle value.
					m_Cross.m_vPosition = pCannonMonster->GetMatEx()->m_vPosition - (pCannonMonster->GetMatEx()->m_vZAxis*20.0f);
					EtVector2 vDir;
					float vDist = 0.0f;
					_ASSERT( GetPress() == CDnActorState::Press_Circle && pCannonMonster->GetPress() == CDnActorState::Press_Circle );
					if( GetPress() == CDnActorState::Press_Circle && pCannonMonster->GetPress() == CDnActorState::Press_Circle )
					{
						if( CheckPressCircle2Clrcle2( pCannonMonster->GetMySmartPtr(), GetMySmartPtr() , vDir, vDist ) ) 
						{
							m_Cross.m_vPosition.x += vDir.x*vDist;
							m_Cross.m_vPosition.z += vDir.y*vDist;
						}
					}

					// If the character is in the "Stand" action at this timing for some other reason
					// Change to Stand_Cannon.
					// Case where an electrocution action ends or is released.
					const char* pCurrentAction = GetCurrentAction();
					if( strcmp(pCurrentAction, "Stand") == 0 )
					{
#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
						DNTableFileFormat* pTableCannon = GetDNTable( CDnTableDB::TCANNON );

						if( pTableCannon->IsExistItem( pCannonMonster->GetClassID() ) )
						{
							const char* pCannonActionName = pTableCannon->GetFieldFromLablePtr( pCannonMonster->GetClassID(), "_StandName" )->GetString();
							CmdAction(pCannonActionName);
						}
						else
						{
							CmdAction( "Stand_Cannon" );
						}
#else
						// Change to cannon action.
						CmdAction( "Stand_Cannon" );
#endif
					}
				}
			}
			break;
		
		case eActor::CS_CANNONTARGETING:
			{
				if( false == IsCannonMode() )
					break;

				DWORD dwCannonMonsterID = 0;
				EtVector3 vCannonDir;			// Direction camera is facing on client; effectively the cannon direction.
				EtVector3 vShootDir;
				EtVector3 vCannonGroundHitPos;

				CPacketCompressStream Stream( pPacket, 64 );
				Stream.Read( &dwCannonMonsterID, sizeof(DWORD) );
				Stream.Read( &vCannonDir, sizeof(EtVector3) );
				Stream.Read( &vShootDir, sizeof(EtVector3) );
				Stream.Read( &vCannonGroundHitPos, sizeof(EtVector3) );

				CDnActor* pActor = CDnActor::FindActorFromUniqueID( GetRoom(), dwCannonMonsterID );
				if( pActor && pActor->GetActorType() == ActorTypeEnum::Cannon )
				{
					CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>(pActor);
					pCannonMonster->SetCannonProjectileInfo( vCannonDir, vShootDir, vCannonGroundHitPos );
				}
			}
			break;

		case eActor::CS_VEHICLE_RIDE_COMPLETE:
			{
				if(!IsVehicleMode() || !GetMyVehicleActor())
					return;

				CPacketCompressStream Stream( pPacket, 128 );
				bool bComplete = false;
				Stream.Read( &bComplete, sizeof(bool) );

				if(bComplete)
				{
					DNVector( DnBlowHandle ) vlhFrameBlows;
					DNVector( DnBlowHandle ) vlhMoveSpeedBlows;
					m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_025, vlhFrameBlows );
					m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_076, vlhMoveSpeedBlows );

					if(vlhFrameBlows.size() > 0)
					{
						for(DWORD i=0 ; i<vlhFrameBlows.size() ; i++)
						{
							if(vlhFrameBlows[i])
								GetMyVehicleActor()->CmdAddStateEffect( vlhFrameBlows[i]->GetParentSkillInfo() , vlhFrameBlows[i]->GetBlowIndex() , (int)(vlhFrameBlows[i]->GetDurationTime() * 1000) , 
								vlhFrameBlows[i]->GetValue() );
						}
					}

					if(vlhMoveSpeedBlows.size() > 0)
					{
						for(DWORD i=0 ; i<vlhMoveSpeedBlows.size() ; i++)
						{
							if(vlhMoveSpeedBlows[i])
								GetMyVehicleActor()->CmdAddStateEffect( vlhMoveSpeedBlows[i]->GetParentSkillInfo() , vlhMoveSpeedBlows[i]->GetBlowIndex() , (int)(vlhMoveSpeedBlows[i]->GetDurationTime() * 1000) , 
								vlhMoveSpeedBlows[i]->GetValue() );
						}
					}
				}

			}
			break;

		case eActor::CS_SYNCPRESSEDPOS:
			{
				if( GetGameRoom() && GetGameRoom()->GetGameTask() && !GetGameRoom()->GetGameTask()->IsSyncComplete() ) break;
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos;
				DWORD dwGap;

				Stream.Read( &dwGap, sizeof(DWORD) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

				m_pPlayerSpeedHackChecker->OnSyncDatumGap( dwGap );

				SetPosition( vPos );
				// SetMagnetPosition( vPos );
			}
			break;
		case eActor::CS_UDP_PING:
		{
			CPacketCompressStream Stream( pPacket, 128 );

			DWORD dwTick = 0;
			Stream.Read( &dwTick, sizeof(dwTick) );

			m_pSession->RecvUdpPing( dwTick );
			break;
		}

		case eActor::CS_SUMMONOFF:
			{
				CPacketCompressStream Stream( pPacket, 64 );

				DWORD dwSummonMonsterUniqueID = 0;
				Stream.Read( &dwSummonMonsterUniqueID, sizeof(DWORD) );

				CDnActor* pSummonMonster = CDnActor::FindActorFromUniqueID( GetRoom(), dwSummonMonsterUniqueID );
				if (pSummonMonster)
					pSummonMonster->CmdSuicide(false, false);
			}
			break;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
// 		case eActor::CS_TOTAL_LEVEL:
// 			{
// 				int nTotalLevel = 0;
// 
// 				CPacketCompressStream Stream( pPacket, 128 );
// 				Stream.Read( &nTotalLevel, sizeof(int) );
// 
// 				UpdateTotalLevel(nTotalLevel);
// 			}
// 			break;
		case eActor::CS_ADD_TOTAL_LEVEL_SKILL:
			{
				int nSlotIndex = -1;
				int nSkillID = 0;
				bool isInitialize = false;

				CPacketCompressStream Stream( pPacket, 128 );
				Stream.Read( &nSlotIndex, sizeof(int) );
				Stream.Read( &nSkillID, sizeof(int) );		
				Stream.Read( &isInitialize, sizeof(bool) );
				
				if( nSlotIndex >= TotalLevelSkill::Common::MAXSLOTCOUNT )
					break;

				if(!m_pSession->AddTotalLevelSkillData(nSlotIndex, nSkillID, isInitialize))
				{
					break;
				}
				else
					AddTotalLevelSkill(nSlotIndex, nSkillID, isInitialize);
				
				m_pSession->SendAddTotalLevelSkill(m_pSession->GetSessionID(), nSlotIndex, nSkillID, isInitialize);

			}
			break;
		case eActor::CS_REMOVE_TOTAL_LEVEL_SKILL:
			{
				int nSlotIndex = -1;

				CPacketCompressStream Stream( pPacket, 128 );
				Stream.Read( &nSlotIndex, sizeof(int) );

				if( nSlotIndex < 0 || nSlotIndex >= TotalLevelSkill::Common::MAXSLOTCOUNT )
					break;

				RemoveTotalLevelSkill(nSlotIndex);
				m_pSession->AddTotalLevelSkillData(nSlotIndex, 0);
				m_pSession->SendDelTotalLevelSkill(m_pSession->GetSessionID(), nSlotIndex);
			}
			break;
// 		case eActor::CS_TOTAL_LEVEL_SKILL_ACTIVE_LIST:
// 			{
// 				int nCount = 0;
// 				int nSlotIndex = -1;
// 				int nSkillID = 0;
// 
// 				CPacketCompressStream Stream( pPacket, 128 );
// 				Stream.Read( &nCount, sizeof(int) );
// 
// 				for (int i = 0; i < nCount; ++i)
// 				{
// 					Stream.Read( &nSlotIndex, sizeof(int) );
// 					Stream.Read( &nSkillID, sizeof(int) );
// 
// 					ActivateTotalLevelSkillSlot(nSlotIndex, true);
// 					AddTotalLevelSkill(nSlotIndex, nSkillID);
// 				}
// 			}
// 			break;
// 		case eActor::CS_TOTAL_LEVEL_SKILL_CASHSLOT_ACTIVATE:
// 			{
// 				int nSlotIndex = -1;
// 				bool bActivate = false;
// 
// 				CPacketCompressStream Stream( pPacket, 128 );
// 				Stream.Read(&nSlotIndex, sizeof(int));
// 				Stream.Read(&bActivate, sizeof(bool));
// 
// 				ActivateTotalLevelSkillCashSlot(bActivate);
// 			}
// 			break;
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined(PRE_FIX_68898)
		case eActor::CS_SKIP_END_ACTION:
			{
				bool isSkipEndAction = false;

				CPacketCompressStream Stream( pPacket, 128 );
				Stream.Read(&isSkipEndAction, sizeof(bool));

				SetSkipEndAction(isSkipEndAction);
			}
			break;
#endif // PRE_FIX_68898
	}
}

void CDnPlayerActor::SyncClassTime( LOCAL_TIME LocalTime )
{
	MAActorRenderBase::m_LocalTime = LocalTime;
	CDnActor::SyncClassTime( LocalTime );
}

void CDnPlayerActor::ProcessDie( LOCAL_TIME LocalTime, float fDelta )
{
	// Cannot die~
	if( !IsDie() ) {
		ToggleGhostMode( false );
		return;
	}
	if( m_fDieDelta > 0.f ) {
		m_fDieDelta -= fDelta;
		if( m_fDieDelta < 0.f ) m_fDieDelta = 0.f;
	}
	if( !m_bGhost && m_fDieDelta < m_fMaxDieDelta - 4.f ) {
		ToggleGhostMode( true );
	}

	if( m_fDieDelta <= 0.f ) {
		OnFinishProcessDie();
	}

	// Cash items can contain skills, and heraldry plates can also contain skills. For now, let's loop through accessories too.
	if( m_afLastEquipItemSkillRemainTime != 0.0f )
	{
		m_afLastEquipItemSkillRemainTime -= fDelta;
		if( m_afLastEquipItemSkillRemainTime < 0.0f )
			m_afLastEquipItemSkillRemainTime = 0.0f;
	}
}

void CDnPlayerActor::OnKillMonster(DnActorHandle hMonster)
{
	if ( !m_pSession ) return;

	if ( m_pSession->GetQuest() == NULL ) return;

	if( hMonster && hMonster->IsMonsterActor() )
	{
		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hMonster.GetPointer());
		if ( pMonster ) 
		{
			m_pSession->GetQuest()->OnKillMonster(pMonster->GetMonsterClassID());
			UpdateKillMonster();

			if (pMonster->IsBossKillCheck())
				UpdateKillBoss();
		}
	}
}

void CDnPlayerActor::OnStateBlowProcessAfter()
{
	if( m_uiStateBlowProcessAfterBit&eStateBlowAfterProcessType::eRebirth )
	{
		m_uiStateBlowProcessAfterBit &= ~eStateBlowAfterProcessType::eRebirth;

		if( !GetGameRoom() )
		{
			_DANGER_POINT();
			return;
		}

		GetGameRoom()->OnRebirth( GetActorHandle() );
	}
	if( m_uiStateBlowProcessAfterBit & eStateBlowAfterProcessType::eDelZombie )
	{
		m_uiStateBlowProcessAfterBit &= ~eStateBlowAfterProcessType::eDelZombie;

		if( GetGameRoom() && GetGameRoom()->bIsZombieMode() )
			static_cast<CPvPZombieMode*>(static_cast<CDNPvPGameRoom*>(GetGameRoom())->GetPvPGameMode())->DelZombie( GetActorHandle(), true );
	}
}

void CDnPlayerActor::OnAddStateBlowProcessAfterType( eStateBlowAfterProcessType Type )
{
	m_uiStateBlowProcessAfterBit |= Type;
}

void CDnPlayerActor::OnDie( DnActorHandle hHitter )
{
#if defined( PRE_ADD_SECONDARY_SKILL )
	if( GetUserSession() && GetUserSession()->GetSecondarySkillRepository() )
		static_cast<CSecondarySkillRepositoryServer*>(GetUserSession()->GetSecondarySkillRepository())->CancelManufacture();
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	CDnActor::OnDie( hHitter );

	// Notify the hitter's bubble system that the hit target died. /////////////////////////////////////////////
	if(hHitter && hHitter->IsPlayerActor())
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hHitter.GetPointer());
		boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_PLAYER_KILL_TARGET ) );
		pPlayer->Notify( pEvent );
	}
	//////////////////////////////////////////////////////////////////////////

	UpdateDead();
	AddStageDeathCount();

	if(m_pRoom && !m_pRoom->bIsPvPRoom()) {
		for( DWORD i=0; i<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); i++ ) {
			CDNGameRoom::PartyStruct *pParty = CDnPartyTask::GetInstance(GetRoom()).GetPartyData(i);
			if( !pParty || !pParty->pSession ) continue;
			if( pParty->pSession == m_pSession ) continue;
			DnActorHandle hActor = pParty->pSession->GetActorHandle();
			if( !hActor ) continue;
			CDnPlayerActor *pPlayer = (CDnPlayerActor *)hActor.GetPointer();
			pPlayer->UpdatePartyMemberDead();
		}
	}

	// Terminate all currently active skills,
	// Must reapply passive skills after resurrection..
	if( m_hProcessSkill )
		m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.0f );

	// Terminate any active auto-passive skill if running
	EndAutoPassiveSkill( CDnActionBase::m_LocalTime, 0.0f );

	// End prefix skill
	EndPrefixSystemSkill(CDnActionBase::m_LocalTime, 0.0f);

	// #14340 Fixed not to remove passive skills on death.

	// Turn off aura skill or toggle skill if enabled.
	if( IsEnabledToggleSkill() )
		OnSkillToggle( m_hToggleSkill, false );

	if( IsEnabledAuraSkill() )
		OnSkillAura( m_hAuraSkill, false );

	// Remove all remaining state effects applied by other players or mobs.
	RemoveAllBlowExpectPassiveSkill();

	// Reset inventory item cooldowns.
	if( m_pSession && m_pSession->GetItem() )
	{
		//m_pSession->GetItem()->ResetCoolTime();
	}

	SetSP(0);

	bool bIgnoreDuration = false;

	if (bIgnoreDuration == false)
	{
		// Decrease durability.
		bool bDecreaseDurability = true;
		if( m_pRoom ) {
			if( m_pRoom->bIsPvPRoom() ) bDecreaseDurability = false;
			else if( m_pRoom->bIsDLRoom() ) bDecreaseDurability = false;
			else if( m_pRoom->bIsFarmRoom() ) bDecreaseDurability = false;
		}
		if( bDecreaseDurability ) OnDecreaseEquipDurability( GetDeadDurabilityRatio(), true );
	}

	if( GetGameRoom() ) 
	{
		GetGameRoom()->OnDie( GetActorHandle(), hHitter );
		if( GetGameRoom()->GetGameTask() )
			GetGameRoom()->GetGameTask()->OnDie( GetActorHandle(), hHitter );
	}

	if( IsCannonMode() )
	{
		static_cast<CDnCannonMonsterActor*>(m_hCannonMonsterActor.GetPointer())->OnMasterPlayerActorDie();
		EndCannonMode();
	}

	m_pBubbleSystem->Clear();

	if( CDnWorld::IsActive(GetRoom()) )
	{
		CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "DieActionPlayer", GetUniqueID() );
		CDnWorld::GetInstance(GetRoom()).OnTriggerEventCallback( "CDnPlayerActor::OnDie", CDnActionBase::m_LocalTime, 0.f );
	}
}

void CDnPlayerActor::OnFinishProcessDie()
{
	if( GetGameRoom() )
		GetGameRoom()->OnFinishProcessDie( GetActorHandle() );
}

void CDnPlayerActor::ResetActor()
{
	CDnActor::ResetActor();

	m_cMovePushKeyFlag = 0;
	m_LastEscapeTime = 0;
	if( IsProcessSkill() ) CancelUsingSkill();

	// #26902 Remove any temporarily added skills. Packet is sent to client.
	if( IsTempSkillAdded() )
	{
		RemoveAllTempSkill();
		EndAddTempSkillAndSendRestoreTempJobChange();
	}
	if( m_pPlayerSpeedHackChecker ) m_pPlayerSpeedHackChecker->ResetInvalid();

	map<int, DnSkillHandle>::iterator iter = m_mapEnchantSkillFromBubble.begin();
	for( iter; iter != m_mapEnchantSkillFromBubble.end();  )
	{
		SAFE_RELEASE_SPTR( iter->second );
		iter = m_mapEnchantSkillFromBubble.erase( iter );
	}

	m_MixedActionTimeStamp = 0;
}

void CDnPlayerActor::GetBoundingSphere( SSphere &Sphere, bool bActorSize/* = false*/ )
{
	Sphere.Center = m_Cross.m_vPosition;
	Sphere.Center.y += 50.f;
	Sphere.fRadius = 50.f;
}

void CDnPlayerActor::SetBattleMode( bool bEnable )
{
	m_bBattleMode = bEnable;
}

bool CDnPlayerActor::IsCanBattleMode()
{
	if( IsSwapSingleSkin() ) return false;
	if( m_bBattleMode && !m_hWeapon[0] ) return false;
	if( IsSpectatorMode() ) return false;

	return m_bBattleMode;
}

int CDnPlayerActor::GetLevelUpSkillPoint( int nPrevLevel, int nCurLevel )
{
	if( nPrevLevel == nCurLevel ) return 0;

	int nSkillPoint = 0;
	for( int i=nPrevLevel+1; i<=nCurLevel; i++ ) {
		int nItemID = ( ( GetClassID() - 1 ) * PLAYER_MAX_LEVEL ) + i;
		nSkillPoint += CPlayerLevelTable::GetInstance().GetValue( GetJobClassID(), i, CPlayerLevelTable::SkillPoint );
	}
	return nSkillPoint;
}

void CDnPlayerActor::OnBattleToggle( bool bBattle )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &bBattle, sizeof(bool) );

	Send( eActor::SC_CMDTOGGLEBATTLE, &Stream );
}

void CDnPlayerActor::OnLevelUp( int nLevel, int nLevelUpAmount )
{
	if( !IsDie() ) {
		SetHP( GetMaxHP() );
		SetSP( GetMaxSP() );
	}
	int nSkillPoint = GetLevelUpSkillPoint( nLevel - nLevelUpAmount, nLevel );
	//Increase skill points in both places on level up
	m_pSession->ChangeSkillPoint(nSkillPoint, 0, true, DBDNWorldDef::SkillPointCode::LevelUp, DualSkill::Type::Primary);
	m_pSession->ChangeSkillPoint(nSkillPoint, 0, true, DBDNWorldDef::SkillPointCode::LevelUp, DualSkill::Type::Secondary);

	m_pSession->SetLevel(m_nLevel, DBDNWorldDef::CharacterLevelChangeCode::Normal, true);
	m_pSession->SetExp(m_nExperience, DBDNWorldDef::CharacterExpChangeCode::Dungeon, 0, true);

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nLevel, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &m_nExperience, sizeof(int) );

	Send( eActor::SC_LEVELUP, &Stream );

#if !defined(PRE_DELETE_DUNGEONCLEAR)
	// Refresh dungeon save data here once to remove unnecessary items
	m_pSession->RefreshDungeonEnterLevel();
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

	// Refresh and send gate info if there are any gate changes.
	if( CDnPartyTask::IsActive(GetRoom()) ) CDnPartyTask::GetInstance(GetRoom()).UpdateGateInfo();
	
	// Always refresh because skill SP cost varies by level.
	for( DWORD i=0; i<GetSkillCount(); i++ ) {
		DnSkillHandle hSkill = GetSkillFromIndex(i);
		if( hSkill ) hSkill->RefreshDecreaseMP();
	}

	m_pSession->NotifyGuildMemberLevelUp(nLevel);

	m_pSession->GetEventSystem()->OnEvent( EventSystem::OnLevelUp );

	// Master-apprentice graduation
	if( m_pSession->GetMasterSystemData()->SimpleInfo.iMasterCount > 0 && nLevel >= static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MasterSystem_GraduateLevel )) )
	{
		if( m_pSession->CheckDBConnection() )
			m_pSession->GetDBConnection()->QueryMasterSystemGraduate( m_pSession );
	}
	
	if( CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeDungeon && m_pSession->GetPeriodExpItemRate() > 0 )
		m_pSession->SetPeriodExpItemRate();

#if defined( PRE_ADD_BESTFRIEND )
	m_pSession->BestFriendChangeLevel(nLevel, true);
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	UpdateTotalLevelByCharLevel();
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
}

void CDnPlayerActor::OnAddExperience( int nAddExperience, int nLogCode, INT64 biFKey )
{
	if( IsAIBotActor() )
		return;

	m_pSession->ChangeExp(nAddExperience, nLogCode, biFKey);	// Included in DB save
	// No need to match EXP.
}

void CDnPlayerActor::OnBeginStateBlow( DnBlowHandle hBlow )
{
	// Fix this later..
	DNVector(DnActorHandle) hVecList;
	ScanActor( GetRoom(), m_Cross.m_vPosition, 2000.f, hVecList );
	
	for( DWORD i=0; i<hVecList.size(); i++ ) 
	{
		DnActorHandle hActor = hVecList[i];
		if( !hActor ) 
			continue;

		CDNAggroSystem* pAggroSystem = hActor->GetAggroSystem();
		// PlayerActor has no AggroSystem when playing a normal dungeon.
		if( !pAggroSystem )
			continue;

		if( !pAggroSystem->bOnCheckPlayerBeginStateBlow( this ) )
			continue;

		pAggroSystem->OnStateBlowAggro( hBlow );
	}
}


void CDnPlayerActor::UnLockSkill( int nSkillID, INT64 nUnlockPrice/*=0*/ )
{
	// An already unlocked skill must not come in again..
	vector<int>::iterator iter = find( m_vlUnlockZeroLevelSkills.begin(), m_vlUnlockZeroLevelSkills.end(), nSkillID );
	_ASSERT( m_vlUnlockZeroLevelSkills.end() == iter );

	if( m_vlUnlockZeroLevelSkills.end() == iter )
	{
		m_vlUnlockZeroLevelSkills.push_back( nSkillID );

		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
		SkillInfo.iSkillID = nSkillID;
		SkillInfo.iSkillLevel = 0;
		SkillInfo.bCurrentLock = false;
		m_vlPossessedSkill.push_back( SkillInfo );

		INT64 biCurrentCoin = 0;
		INT64 biPickUpCoin = 0;
		
		if( nUnlockPrice > 0 )
		{
			biCurrentCoin = m_pSession->GetCoin();
			biPickUpCoin = m_pSession->GetPickUpCoin();
			m_pSession->SelPickUpCoin(0);
		}

		m_pSession->GetDBConnection()->QueryAddSkill(m_pSession, nSkillID, SkillInfo.iSkillLevel, 0, DBDNWorldDef::SkillChangeCode::GainByBuy, nUnlockPrice, biCurrentCoin, biPickUpCoin);		
	}
}

int CDnPlayerActor::CanAcquireSkillIfUnlock( int nSkillID )
{
	int nRetCode = ERROR_NONE;

	CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
	CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( GetPossessedSkillInfo() );
	CDnSkillTreeSystem::S_OUTPUT Output;

	TryAcquire.iCurrentCharLevel = GetLevel();
	TryAcquire.iTryAcquireSkillID = nSkillID;
	TryAcquire.iHasSkillPoint = GetAvailSkillPointByJob( nSkillID );

	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int iNeedJob = pSkillTable->GetFieldFromLablePtr( nSkillID, "_NeedJob" )->GetInteger();
	if( IsPassJob( iNeedJob ) )
	{
		pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

		// #36858 If linked as global skills, learning any one in the group allows learning without parent skill prerequisites.
		bool bAlreadyGlobalSkillAcquired = HasSameGlobalIDSkill( nSkillID );
		bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
											 (CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
											 (CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
											 true == bAlreadyGlobalSkillAcquired;

		if( CDnSkillTreeSystem::R_SUCCESS != Output.eResult &&
			false == bIgnoreParentSkillCondition )
		{
			switch( Output.eResult )
			{
					// Character level requirement not met.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
					break;

					// No prerequisite (parent) skill.
				case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
					break;

					// Parent skill level requirement not met.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
					break;

					// Cannot acquire skill due to insufficient skill points.
				case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
					nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
					break;
			}
		}
	}

	return nRetCode;
}

int CDnPlayerActor::AcquireSkill( int nSkillID )
{
	int nRetCode = ERROR_NONE;

	vector<int>::iterator iter = find( m_vlUnlockZeroLevelSkills.begin(), m_vlUnlockZeroLevelSkills.end(), nSkillID );
	_ASSERT( m_vlUnlockZeroLevelSkills.end() != iter );

	if( m_vlUnlockZeroLevelSkills.end() != iter )
	{
		// Prevent learning mutually exclusive skills.
		TSkillData* pSkillData = g_pDataManager->GetSkillData( nSkillID );
		if( IsExclusiveSkill( nSkillID, pSkillData->nExclusiveID ) )
		{
			// Blocked by client by default, so reaching here indicates a hack.
			return ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE;
		}

		CDnSkillTreeSystem* pSkillTreeSystem = g_pDataManager->GetSkillTreeSystem();
		CDnSkillTreeSystem::S_TRY_ACQUIRE TryAcquire( GetPossessedSkillInfo() );
		CDnSkillTreeSystem::S_OUTPUT Output;

		TryAcquire.iCurrentCharLevel = GetLevel();
		TryAcquire.iTryAcquireSkillID = nSkillID;
		TryAcquire.iHasSkillPoint = GetAvailSkillPointByJob( nSkillID );

		DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
		int iNeedJob = pSkillTable->GetFieldFromLablePtr( nSkillID, "_NeedJob" )->GetInteger();
		if( IsPassJob( iNeedJob ) )
		{
			//TryAcquire.iJobID = iNeedJob;
			pSkillTreeSystem->TryAcquireSkill( TryAcquire, &Output );

#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
			std::vector<int> nNeedSPValues;
			CDnSkillTask* pSkillTask = static_cast<CDnSkillTask*>(CTaskManager::GetInstance( m_pSession->GetGameRoom() ).GetTask( "SkillTask" ));
			if (pSkillTask)
				pSkillTask->GetNeedSPValuesByJob(nSkillID, nNeedSPValues);

			std::vector<int> jobHistory;
			GetJobHistory(jobHistory);

			bool bAvailableSPByJob = false;
			if (pSkillTask)
				bAvailableSPByJob = pSkillTask->IsAvailableSPByJob(jobHistory, nNeedSPValues, this);

			if (bAvailableSPByJob == false)
				Output.eResult = CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE;
#endif // PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP

			// #36858 If linked as global skills, learning any one in the group allows learning without parent skill prerequisites.
			bool bAlreadyGlobalSkillAcquired = HasSameGlobalIDSkill( nSkillID );
			bool bIgnoreParentSkillCondition = ( (CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL == Output.eResult) ||
				(CDnSkillTreeSystem::R_LOCKED_PARENTSKILL == Output.eResult) ||
				(CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL == Output.eResult) ) &&
				true == bAlreadyGlobalSkillAcquired;

			if( CDnSkillTreeSystem::R_SUCCESS == Output.eResult ||
				true == bIgnoreParentSkillCondition )
			{			
				// Directly change level to 1 internally.
				//vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill.begin();
				//for( iter; m_vlPossessedSkill.end() != iter; ++iter )
				//{
				//	if( iter->iSkillID == nSkillID )
				//	{
				//		iter->iSkillLevel = 1;
				//		break;
				//	}
				//}

				bool bSuccess = AddSkill( nSkillID, 1 );
				_ASSERT( bSuccess );
				DnSkillHandle hAcquiredSkill = FindSkill( nSkillID );
				m_pSession->ChangeSkillPoint( -hAcquiredSkill->GetNowLevelSkillPoint(), nSkillID, false, 0 );

				m_pSession->GetDBConnection()->QueryModSkillLevel(m_pSession, nSkillID, 1, 0, -hAcquiredSkill->GetNowLevelSkillPoint(), DBDNWorldDef::SkillChangeCode::GainByBuy);	// Save to DB: also update skill points
			}
			else
			{
				switch( Output.eResult )
				{
						// Character level requirement not met.
					case CDnSkillTreeSystem::R_NOT_ENOUGH_CHAR_LEVEL:
						nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL;
						break;

						// No prerequisite (parent) skill.
					case CDnSkillTreeSystem::R_DONT_HAVE_PARENT_SKILL:
						nRetCode = ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL;
						break;

						// Parent skill level requirement not met.
					case CDnSkillTreeSystem::R_NOT_ENOUGH_PARENT_SKILL_LEVEL:
						nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL;
						break;

						// Cannot acquire skill due to insufficient skill points.
					case CDnSkillTreeSystem::R_NOT_ENOUGH_SKILLPOINT_TO_ACQUIRE:
						nRetCode = ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT;
						break;
				}

			}
		}
	}

	return nRetCode;
}

void CDnPlayerActor::CheckAndRegisterObserverStateBlow( DnBlowHandle hBlow )
{
	switch( hBlow->GetBlowIndex() )
	{
			// Block state effect
		case STATE_BLOW::BLOW_030:
			{
				CDnBlockBlow* pObservable = static_cast<CDnBlockBlow*>( hBlow.GetPointer() );
				pObservable->RegisterObserver( m_pBubbleSystem );
			}
			break;

			// Parrying state effect
		case STATE_BLOW::BLOW_031:
			{
				CDnParryBlow* pObservable = static_cast<CDnParryBlow*>( hBlow.GetPointer() );
				pObservable->RegisterObserver( m_pBubbleSystem );
			}
			break;

			// Cooldown parry state effect.
		case STATE_BLOW::BLOW_153:
			{
				CDnCooltimeParryBlow* pObservable = static_cast<CDnCooltimeParryBlow*>( hBlow.GetPointer() );
				pObservable->RegisterObserver( m_pBubbleSystem );
			}
			break;
	}
}

void CDnPlayerActor::OnApplyPassiveSkillBlow( int iBlowID )
{
	DnBlowHandle hBlow = m_pStateBlow->GetStateBlowFromID( iBlowID );

	if( hBlow )
		CheckAndRegisterObserverStateBlow( hBlow );
}

bool CDnPlayerActor::CanAddSkill( int nSkillTableID, int nLevel /*= 1*/ )
{
	TSkillData* pSkillData = g_pDataManager->GetSkillData( nSkillTableID );
	if( pSkillData == NULL )
		return false;
	if( GetGameRoom() == NULL )
		return false;
	if( GetGameRoom()->bIsZombieMode() == true )
	{
		if( CDnSkill::Passive == pSkillData->cSkillType && CDnSkill::DurationTypeEnum::Buff == pSkillData->cDurationType )
			return false;
	}
	return true;
}

bool CDnPlayerActor::AddSkill( int nSkillTableID, int nLevel /* = 1 */, int iSkillLevelApplyType/* = CDnSkill::PVE*/ )
{
	bool bSuccess = false;

	if( 0 < nLevel )
	{
		// Set skill level data separately for PvE and PvP.
		int iSkillLevelDataType = CDnSkill::PVE;
		if( GetGameRoom()->bIsPvPRoom() )
			iSkillLevelDataType = CDnSkill::PVP;

		bSuccess = MASkillUser::AddSkill( nSkillTableID, nLevel, iSkillLevelDataType );

		vector<int>::iterator iter = find( m_vlUnlockZeroLevelSkills.begin(), m_vlUnlockZeroLevelSkills.end(), nSkillTableID );
		if( m_vlUnlockZeroLevelSkills.end() != iter )
			m_vlUnlockZeroLevelSkills.erase( iter );
	}
	else
	{
		// Enters here if skill level is 0 when receiving initial skill list from DB.
		// A skill that is only unlocked and not actually owned.
		m_vlUnlockZeroLevelSkills.push_back( nSkillTableID );

		bSuccess = true;
	}

	//if( bSuccess )
	//{
	//	bool bFinded = false;
	//	int iNumPossessed = (int)m_vlPossessedSkill.size();
	//	for( int i = 0; i < iNumPossessed; ++i )
	//	{
	//		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO& PossessedSkill = m_vlPossessedSkill.at( i );
	//		if( PossessedSkill.iSkillID == nSkillTableID )
	//		{
	//			PossessedSkill.iSkillLevel = nLevel;
	//			bFinded = true;
	//		}
	//	}

	//	if( false == bFinded )
	//	{
	//		// Include level 0 skills that are only unlocked in the list as well.
	//		CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
	//		SkillInfo.iSkillID = nSkillTableID;
	//		SkillInfo.iSkillLevel = nLevel;
	//		SkillInfo.bCurrentLock = false;
	//		m_vlPossessedSkill.push_back( SkillInfo );
	//	}
	//}

	return bSuccess;
}


bool CDnPlayerActor::ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta )
{
	ResetCustomAction();

	// When toggled off, m_hAuraSkill is cleared for aura skill.
	// CDnActor::OnSkillExecute( LocalTime, fDelta ); 
	DnSkillHandle hNowSkill;
	if( m_hAuraSkill )
		hNowSkill = m_hAuraSkill;

	bool bResult = CDnActor::ExecuteSkill( hSkill, LocalTime, fDelta );

	if( m_hProcessSkill )
		hNowSkill = m_hProcessSkill;
	else
	if( m_hAuraSkill )
		hNowSkill = m_hAuraSkill;

	if( bResult && hNowSkill ) 
	{
		UpdateUseSkill( hNowSkill );

		// Fire the event unconditionally unless it's instant for now. In case aura or time toggle isn't needed later
		// Filter again here.
		switch( hNowSkill->GetDurationType() ) {
			case CDnSkill::Instantly:
				break;
			default:
				{
					if( m_pSession && m_pSession->GetMissionSystem() )
					{
						m_pSession->GetEventSystem()->OnEvent( EventSystem::OnSkillUse, 2,
																 EventSystem::SkillID, hNowSkill->GetClassID(),
																 EventSystem::SkillLevel, hNowSkill->GetLevel() );
					}
				}
				break;
		}
	}


#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
	if( bResult && CDnDPSReporter::IsActive() )
	{
		if(CDnDPSReporter::GetInstance().IsEnabledUser( GetCharacterDBID() ))
		{
			DNVector(DnActorHandle) hVecList;
			ScanActor( GetRoom(), *GetPosition() , 500.f , hVecList );
			CDnDPSReporter::GetInstance().ReportSkillInfo( hSkill , (int)hVecList.size());
		}
	}
#endif

	return bResult;
}


void CDnPlayerActor::OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct )
{
	// #33265 by kalliste
	// #if defined( PRE_ADD_LOTUSGOLEM )
	// 	if( hActor->GetHitParam()->bIgnoreShowDamage == true )
	// 	{
	// 		m_nComboDelay = -1;
	// 		return;
	// 	}
	// #endif // #if defined( PRE_ADD_LOTUSGOLEM )

	// #12170 Hits with combo delay 0 do not affect combo evaluation.
	if( m_bAllowCalcCombo && 
		0 < pStruct->nComboDelay )		
	{
		if( m_nComboDelay > 0 ) {
#if defined( PRE_ADD_LOTUSGOLEM )
			if( hActor->GetHitParam()->bIgnoreShowDamage == false )
				m_nComboCount++;
#else
			m_nComboCount++;
#endif // #if defined( PRE_ADD_LOTUSGOLEM )

			if( m_nComboCount > m_nTotalComboCount )
				m_nTotalComboCount = m_nComboCount;

			UpdateMaxCombo( m_nComboCount );
		}
		else {
			m_nComboCount = 1;
		}
		m_nComboDelay = pStruct->nComboDelay;
		if( m_nComboDelay == 0 ) {
			m_nComboCount = 0;
		}
	}

	switch( hActor->GetHitParam()->HitType ) {
		case CDnWeapon::Critical: UpdateCriticalHit(); break;
		case CDnWeapon::Stun: UpdateStunHit(); break;
	}

	CheckNormalHitSEProcessor( hActor, *hActor->GetHitParam() );

#ifdef IRINE_MOD_PLAYER_BOTS
	if( m_pAIBotController && hActor )
		m_pAIBotController->OnAttackHitTarget( hActor, false );
#endif

	if( hActor && hActor->IsHit() && pStruct->szTargetHitAction != NULL )
	{
		if( IsAppliedThisStateBlow(STATE_BLOW::BLOW_246) )
		{
			DNVector(DnBlowHandle) vlBlows;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_246, vlBlows );
			for( DWORD i=0; i<vlBlows.size(); i++ )
			{
				if( vlBlows[i] )
				{
					const CDnSkill::SkillInfo* pSkillInfo = vlBlows[i]->GetParentSkillInfo();
					if( pSkillInfo )
					{
						DnActorHandle hHitter = pSkillInfo->hSkillUser;
						if( hHitter == hActor )
						{
							vlBlows[i]->SetState(STATE_BLOW::STATE_END);
							SendRemoveStateEffectFromID( vlBlows[i]->GetBlowID() );
						}
					}
				}
			}
		}
	}
}

// When a projectile fired by this actor hits.
void CDnPlayerActor::OnHitProjectile( LOCAL_TIME LocalTime, DnActorHandle hHittedTarget, const CDnDamageBase::SHitParam& HitParam )
{
	CheckNormalHitSEProcessor( hHittedTarget, HitParam );
#ifdef IRINE_MOD_PLAYER_BOTS
	if( m_pAIBotController && hHittedTarget )
		m_pAIBotController->OnAttackHitTarget( hHittedTarget, true );
#endif
}

void CDnPlayerActor::CheckNormalHitSEProcessor( DnActorHandle hHittedTarget, const CDnDamageBase::SHitParam& HitParam )
{
	// #23818 Code to distinguish "normal attack" for processing item prefix skills attached to weapons.
	// For normal attacks, check if there is a trigger type that applies a state effect to the target on normal attack and process it.
	if( hHittedTarget )
	{
		// Only when I am not using a skill.
		if( ( false == IsProcessSkill() || false == HitParam.bFromProjectileSkill) )
		{
			int iNumApplySEProcessor = (int)m_vlpApplySEWhenNormalHitProcessor.size();
			for( int i = 0; i < iNumApplySEProcessor; ++i )
			{
				CDnApplySEWhenTargetNormalHitProcessor* pProcessor = static_cast<CDnApplySEWhenTargetNormalHitProcessor*>( m_vlpApplySEWhenNormalHitProcessor.at( i ) );
				pProcessor->OnNormalHitSuccess( hHittedTarget );
			}
		}
	}
}

void CDnPlayerActor::OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct )
{
	CDnActor::OnHitFinish( LocalTime, pStruct );
	/*
	if( m_nComboDelay > 0 && pStruct->bFinish ) {
		for( DWORD i=0; i<m_hVecLastHitList.size(); i++ ) {
			if( m_hVecLastHitList[i] && m_hVecLastHitList[i]->IsAir() ) {
				m_nChainCount++;
				UpdateChainCombo();

				char pBuffer[2];
				CPacketCompressStream Stream( pBuffer, sizeof(pBuffer) );
				Stream.Write( &m_nChainCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Send( eActor::SC_CHAIN, &Stream );
				break;
			}
		}
	}
	*/

	int nDieCount = 0;
	for( DWORD i=0; i<m_hVecLastHitList.size(); i++ ) {
		if( m_hVecLastHitList[i] && m_hVecLastHitList[i]->IsDie() ) nDieCount++;
	}
	
	UpdateMissionByMonsterKillCount(nDieCount);
}

void CDnPlayerActor::UpdateMissionByMonsterKillCount(int nCount)
{
	if( nCount >= 3 ) UpdateGenocide();

	if( nCount >= 2 && m_pSession )
	{
		m_pSession->GetEventSystem()->OnEvent( EventSystem::OnKillMonster, 1,
			EventSystem::GenocideCount, nCount );
	}
}

void CDnPlayerActor::SaveUserData(TUserData &UserData)
{
#ifndef PRE_FIX_SKILLLIST
	memset( UserData.Skill.SkillList, 0, sizeof(UserData.Skill.SkillList) );

	if( static_cast<CDNGameRoom*>(GetRoom())->bIsPvPRoom() == false )
	{
		int iIndex = 0;
		for( UINT i=0 ; i<m_vlhSkillList.size() ; ++i )
		{
			DnSkillHandle hSkill = m_vlhSkillList[i];
			if( !hSkill )
				continue;
			UserData.Skill.SkillList[iIndex].nSkillID		= hSkill->GetClassID();
			UserData.Skill.SkillList[iIndex].cSkillLevel	= hSkill->GetLevel();
			UserData.Skill.SkillList[iIndex].nCoolTime		= static_cast<int>(hSkill->GetElapsedDelayTime()*1000);
			++iIndex;
		}
	}
#else
	memset( UserData.Skill[0].SkillList, 0, sizeof(UserData.Skill[0].SkillList) );

	if( static_cast<CDNGameRoom*>(GetRoom())->bIsPvPRoom() == false )
	{
		int iIndex = 0;
		DWORD dwNumSkill = GetSkillCount();
		for( DWORD i = 0; i < dwNumSkill; ++i )
		{
			DnSkillHandle hSkill = GetSkillFromIndex( i );
			if( !hSkill )
				continue;
			UserData.Skill[0].SkillList[iIndex].nSkillID		= hSkill->GetClassID();
			UserData.Skill[0].SkillList[iIndex].cSkillLevel	= hSkill->GetLevel();
			UserData.Skill[0].SkillList[iIndex].nCoolTime		= static_cast<int>(hSkill->GetElapsedDelayTime()*1000);
			++iIndex;
		}
	}	
#endif // #ifndef PRE_FIX_SKILLLIST

	UserData.Status.nGlyphDelayTime = (int)(m_afLastEquipItemSkillDelayTime * 1000.f);
	UserData.Status.nGlyphRemainTime = (int)(m_afLastEquipItemSkillRemainTime * 1000.f);
	if( GetUserSession() && m_nInvalidPlayerCheckCounter > 0 ) 
	{
		g_Log.Log( LogType::_HACK, GetUserSession(), L"HackChecker(DB) : CharName=%s Counter=%d\n", GetUserSession()->GetCharacterName(), m_nInvalidPlayerCheckCounter );
		m_pSession->GetDBConnection()->QueryAddAbuseMonitor( m_pSession, m_nInvalidPlayerCheckCounter, 0 );
		m_nInvalidPlayerCheckCounter = 0;
	}
}

void CDnPlayerActor::SetUserSession(CDNUserSession * pSession) 
{ 
	m_pSession = pSession; 
	m_pPartyData = pSession->GetGameRoom()->GetPartyData( m_pSession );

	// Just set EquipDelayTime and EquipRemainTime here;;
	m_afLastEquipItemSkillDelayTime = (float)m_pSession->GetGlyphDelayTime() / 1000.f;
	m_afLastEquipItemSkillRemainTime = (float)m_pSession->GetGlyphRemainTime() / 1000.f;

	// If there is a matching skill in the current equipment slot, set its cooldown.
	if( m_ahEquipSkill )
		m_ahEquipSkill->SetOnceCoolTime( m_afLastEquipItemSkillDelayTime, m_afLastEquipItemSkillRemainTime );

	SetAccountLevel(m_pSession->GetAccountLevel());
}

void CDnPlayerActor::ProcessCombo( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nComboDelay > 0 ) m_nComboDelay -= (int)( fDelta * 1000 );
	if( m_nComboDelay < 0 ) {
		OnComboFinish( m_nComboCount );
		m_nComboCount = 0;
		m_nComboDelay = 0;
	}
}

void CDnPlayerActor::OnComboFinish( int nCombo )
{
	if( nCombo > 1 ) {
		UpdateCombo( nCombo );
	}
}

bool CDnPlayerActor::IsGMTrace() const
{
	if( m_pSession && m_pSession->bIsGMTrace() )
		return true;

	return false;
}

UINT CDnPlayerActor::GetSessionID()
{
	return m_pSession ? m_pSession->GetSessionID() : 0;	
}

int CDnPlayerActor::GetMoveSpeed()
{
	int nMoveSpeed = CDnActor::GetMoveSpeed();

	if( CDnWorld::IsActive(GetRoom()) && ( CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeVillage || CDnWorld::GetInstance(GetRoom()).GetMapType() == EWorldEnum::MapTypeWorldMap ) )
		nMoveSpeed += GetSafeZoneMoveSpeed();

	if( IsTransformMode() )
	{
		nMoveSpeed = g_pDataManager->GetMonsterMutationMoveSpeed( m_nMonsterMutationTableID );
		if( m_cMovePushKeyFlag & 0x08 )
			 nMoveSpeed /= 2;
		return nMoveSpeed;
	}

	if( m_cMovePushKeyFlag & 0x08 ) nMoveSpeed /= 2;
	// #62481 Change world zone movement speed to match village
	if( !IsGhost() && !IsBattleMode() ) nMoveSpeed = (int)( nMoveSpeed * 1.4f );
	if( IsDie() && IsGhost() ) nMoveSpeed = (int)( nMoveSpeed * 1.5f );
	return nMoveSpeed;
}

void CDnPlayerActor::OnAddSkill( DnSkillHandle hSkill, bool isInitialize/* = false*/ )
{
	CDnActor::OnAddSkill( hSkill, isInitialize );

	if( hSkill )
	{
		bool bFinded = false;
		int iNumPossessed = (int)m_vlPossessedSkill.size();
		for( int i = 0; i < iNumPossessed; ++i )
		{
			CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO& PossessedSkill = m_vlPossessedSkill.at( i );
			if( PossessedSkill.iSkillID == hSkill->GetClassID() )
			{
				PossessedSkill.iSkillLevel = hSkill->GetLevel();
				bFinded = true;
			}
		}

		if( false == bFinded )
		{
			// Include level 0 skills that are only unlocked in the list as well.
			CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO SkillInfo;
			SkillInfo.iSkillID = hSkill->GetClassID();
			SkillInfo.iSkillLevel = hSkill->GetLevel();
			SkillInfo.bCurrentLock = false;
			m_vlPossessedSkill.push_back( SkillInfo );
		}
	}

	// When the added skill is an enhanced passive skill (acquired an enhanced passive skill or another player leveled up)
	if( hSkill->GetSkillType() == CDnSkill::EnchantPassive )
	{
		int iBaseSkillID = hSkill->GetBaseSkillID();
		DnSkillHandle hBaseSkill = FindSkill( iBaseSkillID );
		if( hBaseSkill )
		{
#if defined(PRE_FIX_64312)
			//For summon monster skills, store instead of applying immediately, and apply at UseSkill time in MAAiSkill.
			bool isSummonMonsterSkill = false;

			isSummonMonsterSkill = hBaseSkill->IsSummonMonsterSkill();

			if (isSummonMonsterSkill == false)
				hBaseSkill->ApplyEnchantSkill( hSkill );
			else
				hBaseSkill->AddSummonMonsterEnchantSkill(hSkill);
#else
			hBaseSkill->ApplyEnchantSkill( hSkill );
#endif // PRE_FIX_64312
		}
	}
	else
	{
		// When the added skill is the base skill of an owned enhanced passive skill (when another player's enhanced skill leveled up)
		CheckAndApplyEnchantPassiveSkill( hSkill );
	}
}


void CDnPlayerActor::OnRemoveSkill( DnSkillHandle hSkill )
{
	CDnActor::OnRemoveSkill( hSkill );

	if( hSkill )
	{
		vector<CDnSkillTreeSystem::S_POSSESSED_SKILL_INFO>::iterator iter = m_vlPossessedSkill.begin();
		for( iter; iter != m_vlPossessedSkill.end(); ++iter )
		{
			if( iter->iSkillID == hSkill->GetClassID() )
			{
				m_vlPossessedSkill.erase( iter );
				break;
			}
		}
	}

	// Case where another player's passive enhancement skill levels up and the previous level skill object enters this deletion routine.
	// Reset the enhancement state of the currently applied base skill.
	if( CDnSkill::EnchantPassive == hSkill->GetSkillType() &&
		0 < hSkill->GetBaseSkillID() )
	{
		DnSkillHandle hBaseSkill = FindSkill( hSkill->GetBaseSkillID() );
		if( hBaseSkill )
		{
#if defined(PRE_FIX_64312)
			//For summon monster skills, store instead of applying immediately, and apply at UseSkill time in MAAiSkill.
			bool isSummonMonsterSkill = false;

			isSummonMonsterSkill = hBaseSkill->IsSummonMonsterSkill();

			if (isSummonMonsterSkill == false)
				hBaseSkill->ReleaseEnchantSkill();
			else
				hBaseSkill->RemoveSummonMonsterEnchantSkill();
#else
			hBaseSkill->ReleaseEnchantSkill();
#endif // PRE_FIX_64312
		}
	}

	// Case where a base skill targeted by another player's passive enhancement skill levels up and enters this removal routine.
	// In this case, just delete the base skill object; no extra handling needed.
}

void CDnPlayerActor::ProcessRecoverySP( LOCAL_TIME LocalTime, float fDelta )
{
	if( IsDie() ) {
		m_fRecoverySPDelta = 0.f;
		return;
	}
	m_fRecoverySPDelta += fDelta;

	int nValue = GetSP();
	if( m_fRecoverySPDelta >= s_fRecoverySPTime ) {
		m_fRecoverySPDelta -= s_fRecoverySPTime;
		nValue += GetRecoverySP();
	}
	else return;

	if( GetSP() >= GetMaxSP() ) return;

	nValue = min( nValue, GetMaxSP() );
	SetSP( nValue );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nValue, sizeof(int) );

	Send( eActor::SC_RECOVERYSP, &Stream );
}

void CDnPlayerActor::CmdToggleWeaponViewOrder( int nEquipIndex, bool bShowCash )
{
	SetWeaponViewOrder( nEquipIndex, bShowCash );
	RefreshWeaponViewOrder( nEquipIndex );

	if( m_pSession )
	{
		m_pSession->SetViewCashEquipBitmap(CASHEQUIP_WEAPON1 + nEquipIndex, bShowCash);
	}
}

void CDnPlayerActor::CmdTogglePartsViewOrder( int nEquipIndex, bool bShowCash )
{
	if ((nEquipIndex < 0)  || (nEquipIndex >= _countof(m_bPartsViewOrder))) 
		return;

	SetPartsViewOrder( nEquipIndex, bShowCash );
	RefreshPartsViewOrder( nEquipIndex );

	if( m_pSession )
		m_pSession->SetViewCashEquipBitmap(nEquipIndex, bShowCash);
}

void CDnPlayerActor::CmdToggleHideHelmet( bool bHideHelmet )
{
	if( m_pSession )
		m_pSession->SetViewCashEquipBitmap(HIDEHELMET_BITINDEX, bHideHelmet);
}

void CDnPlayerActor::CmdChangeJob( int nJobID )
{
	SetJobHistory( nJobID );
	OnChangeJob( nJobID );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nJobID, sizeof(int) );

	Send( eActor::SC_CHANGEJOB, m_pSession->GetSessionID(), &Stream );
}

void CDnPlayerActor::CmdEscape( EtVector3 &vPos )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

	Send( eActor::SC_CMDESCAPE, &Stream );
}

void CDnPlayerActor::ToggleGhostMode( bool bGhost )
{
	if( bGhost == m_bGhost ) return;
	m_bGhost = bGhost;

	if( bGhost == true && GetSP() > 0 )
		SetSP(0);

#ifdef PRE_FIX_GAMESERVER_USE_GHOST_MODE
	bool bGhostMode = true;
	CDnGameTask* pGameTask = m_pSession->GetGameRoom()->GetGameTask();
	if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP ) 
	{
		bGhostMode = false;
		CDNGameRoom* pGameRoom = GetGameRoom();
#if defined(PRE_ADD_PVP_TOURNAMENT)
		if( pGameRoom && pGameRoom->GetPvPGameMode() && (pGameRoom->GetPvPGameMode()->bIsAllKillMode() || pGameRoom->GetPvPGameMode()->bIsTournamentMode()))
#else
		if( pGameRoom && pGameRoom->GetPvPGameMode() && pGameRoom->GetPvPGameMode()->bIsAllKillMode() )
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)
			bGhostMode= true;
	}

	if( !bGhostMode ) 
	{
		if( bGhost ) 
		{
			if( IsMovable() || IsStay() ) 
			{
				SetAction( "Die", 0.f, 0.f, false );
			}
		}
		else 
		{
			std::string szActionName = "Stand";
			if(m_bShootMode)
				szActionName = "MOD_Stand";

			if(IsTransformMode())
			{
				if(IsExistAction( "Summon_On" ))
					szActionName = "Summon_On";
			}
			SetAction( szActionName.c_str(), 0.f, 0.f );
		}
	}
	else 
	{
		if( bGhost ) 
		{
			SwapSingleSkin( 499 + m_nClassID );
			SetAction( "Stand", 0.f, 0.f );
		}
		else
		{
			SwapSingleSkin( -1 );
			SetAction( "Stand", 0.f, 0.f );
		}
	}
#endif

	if( m_bGhost && GetGameRoom() && GetGameRoom()->GetGameTask() )
		GetGameRoom()->GetGameTask()->OnGhost( GetActorHandle() );
}

void CDnPlayerActor::_CheckProcessSkillActioncChange( const char* pAction )
{
	// #26467 Action is updated on the next frame after being set, so check immediately if a skill is in use and end the skill if it is not a skill action.
	if( m_hProcessSkill )
	{
		m_setUseActionName.clear();
		m_setUseActionName.insert( pAction );

		if( false == m_hProcessSkill->IsUseActionNames( m_setUseActionName ) )
		{
			if( false == (IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn()) )
			{
				m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
			}

			m_hProcessSkill.Identity();
		}
	}
}

void CDnPlayerActor::_CheckActionWithProcessPassiveActionSkill( const char* szPrevAction )
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
			// When an active skill is registered and used as a passive, it can be checked with GetPassiveSkillLengh().
			if( ( (m_hProcessSkill->GetSkillType() == CDnSkill::Passive || m_hProcessSkill->GetPassiveSkillLength() != 0.f ) || m_hProcessSkill->GetSkillType() == CDnSkill::AutoPassive) &&
				m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
			{
				// Check when chain input arrives for a passive skill. The chain input flag is reset once checked.
				// When chain input occurs, passive skill duration extends by the action length.
				// Both flag and time must be used together like this to allow continuous passive skill chaining.
				if( false == m_hProcessSkill->CheckChainingPassiveSkill() )
				{
					if( false == m_hProcessSkill->IsChainInputAction( GetCurrentAction() ) )
					{
						m_hProcessSkill->OnEnd( MAActorRenderBase::m_LocalTime, 0.0f );
						m_hProcessSkill.Identity();
					}
				}
			}
			else if( IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn() )
			{
				// Note Hanki: m_hProcessSkill smart pointer must remain valid while the aura skill action is playing
				// Filtered in CDnPlayerActor::CmdStop() on the game server, so on the game server the action signal to the end 
				// Processed. Therefore, make ProcessAction an identity in CDnActor::OnChangeAction.

				m_hProcessSkill.Identity();
				ClearSelfStateSignalBlowQueue(); // Clear queue sent to state effect timing signal applied to self for aura skill; otherwise it affects other skills.
			}
		}
	}
}


void CDnPlayerActor::OnChangeAction( const char* szPrevAction )
{
	// Enable/disable handling for state effect #129 (action name replacement).
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_129 ) )
	{
		DNVector(DnBlowHandle) vlhChangeActionSetBlow;
		m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_129, vlhChangeActionSetBlow );

		// There can be multiple action set change state effects.
		int iNumBlow = (int)vlhChangeActionSetBlow.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			if( !vlhChangeActionSetBlow[i] )
				continue;
			CDnChangeActionSetBlow* pChangeActionSetBlow = static_cast<CDnChangeActionSetBlow*>( vlhChangeActionSetBlow.at(i).GetPointer() );
			pChangeActionSetBlow->UpdateEnable( szPrevAction, GetCurrentAction() );
			
			// Call the function if there is a state effect trigger type on action change.
			// Since action already comes converted from client...
			CDnChangeActionStrProcessor* pProcessor = pChangeActionSetBlow->GetChangeActionStrProcessor();	// Returns NULL when the action change trigger type is inactive.
			if( pProcessor && pProcessor->IsChangedActionName( GetCurrentAction() ) )
			{	
				pChangeActionSetBlow->OnChangeAction();
			}
			else
			{
				// Must also be called in setaction where actions are handled internally on the server.
				pChangeActionSetBlow->OnNotChangeAction();
			}
		}
	}

	_CheckActionWithProcessPassiveActionSkill( szPrevAction );
	//CDnActor::OnChangeAction( szPrevAction );

	if( !( szPrevAction && m_nPrevActionIndex == m_nActionIndex ) ) {
		if( CDnWorld::IsActive(GetRoom()) ) {
			CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "ChangeActionPlayer", GetUniqueID() );
			CDnWorld::GetInstance(GetRoom()).OnTriggerEventCallback( "CDnPlayerActor::OnChangeAction", CDnActionBase::m_LocalTime, 0.f );
		}
	}

	m_bUseSignalSkillCheck = false;
	ZeroMemory( m_abSignalSkillCheck, sizeof(m_abSignalSkillCheck) );

	// During a mixed action, it is already updated when the mixed action packet arrives.
	if( false == m_bUpdatedProjectileInfoFromCmdAction )
	{
		if( false == IsCustomAction() )
			_UpdateMaxProjectileCount( m_nActionIndex );
	}
	else
		m_bUpdatedProjectileInfoFromCmdAction = false;	// Clear flag so it updates when OnChangeAction() is called next.

	// Restore term checking when the action changes.
	m_bCheckProjectileSignalTerm = true;

	if( false == m_mapIcyFractionHitted.empty() )
		m_mapIcyFractionHitted.clear();

	// Notify the bubble system (used by Academic classes, etc.).
	boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_ONCHANGEACTION ) );
	Notify( pEvent );
}

void CDnPlayerActor::OnBreakSkillSuperAmmor( int nIndex, int nOriginalSupperAmmor, int nDescreaseSupperAmmor )
{
	if( nOriginalSupperAmmor >= 200 ) {
		UpdateSuperAmmorBreak();
	}
}

void CDnPlayerActor::OnAirCombo( int nComboCount )
{
	if( nComboCount >= 2 ) {
		UpdateAirCombo();
	}
}


void CDnPlayerActor::PushSummonMonster( DnMonsterActorHandle hMonster, const SummonMonsterStruct* pSummonMonsterStruct, bool bReCreateFollowStageMonster/* = false*/ )
{
	CDnActor::PushSummonMonster( hMonster, pSummonMonsterStruct, bReCreateFollowStageMonster );

	// Regardless of whether it is a summoned mob respawned on stage transition (bReCreateFollowStageMonster)
	// Insert under the group ID. Invalid in CDnActor::bIsCanSummonMonster() during stage transition
	// The monster actor object handle is cleaned up.
	if( 0 < pSummonMonsterStruct->nGroupID )
	{
		m_mapSummonMonsterByGroup[ pSummonMonsterStruct->nGroupID ].push_back( hMonster );
		hMonster->SetSummonGroupID( pSummonMonsterStruct->nGroupID );
	}

	if( bReCreateFollowStageMonster )
	{
		// Replace handle when a managed stage-following monster is recreated in a new stage.
		DWORD dwMonsterClassID = hMonster->GetMonsterClassID();
		std::list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO>::iterator iter = m_listSummonedMonstersFollowStageInfos.begin();
		for( iter; iter != m_listSummonedMonstersFollowStageInfos.end(); ++iter )
		{
			if( false == iter->bReCreatedFollowStageMonster && 
				dwMonsterClassID == iter->dwMonsterClassID )
			{
				iter->hMonster = hMonster;
				iter->hMonster->SetActionQueue( "Stand" );		// Since it is not newly summoned, immediately play the stand action.
				iter->hMonster->CmdWarp( *GetPosition(), EtVec3toVec2( *GetLookDir() ) );
				iter->bReCreatedFollowStageMonster = true;
				break;
			}
		}
	}
	else
	{
		// Additionally, for players, summons configured to follow on stage move or zone move (CmdWarp)
		// Also store it separately.
		if( pSummonMonsterStruct->bFollowSummonerStage )
		{
			S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO Info;
			Info.hMonster = hMonster;
			Info.dwMonsterClassID = hMonster->GetMonsterClassID();
#ifdef PRE_FIX_MEMOPT_SIGNALH
			CopyShallow_SummonMonsterStruct(Info.SummonMonsterSignalData, pSummonMonsterStruct);
#else
			Info.SummonMonsterSignalData = *pSummonMonsterStruct;
#endif
			Info.iRemainDestroyTime = pSummonMonsterStruct->nLifeTime;

			m_listSummonedMonstersFollowStageInfos.push_back( Info );
		}
	}
}


// Called when a summoned monster object expires or dies with 0 HP.
// For these two cases, remove from the management list of stage-following summoned monsters.
// Not called when the monster object is destroyed upon stage transition.
void CDnPlayerActor::OnDieSummonedMonster( DnMonsterActorHandle hSummonedMonster )
{
	std::list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO>::iterator iter = m_listSummonedMonstersFollowStageInfos.begin();
	for( iter; iter != m_listSummonedMonstersFollowStageInfos.end(); ++iter )
	{
		if( hSummonedMonster == iter->hMonster )
		{
			m_listSummonedMonstersFollowStageInfos.erase( iter );
			break;
		}
	}

	int iGroupID = hSummonedMonster->GetSummonGroupID();
	if( 0 < m_mapSummonMonsterByGroup.count( iGroupID ) )
	{
		list<DnMonsterActorHandle>& listSummonedMonsters = m_mapSummonMonsterByGroup[ iGroupID ];
		list<DnMonsterActorHandle>::iterator iter = find( listSummonedMonsters.begin(), listSummonedMonsters.end(), hSummonedMonster );
		if( listSummonedMonsters.end() != iter )
			listSummonedMonsters.erase( iter );

		// Remove from map if the group's list is empty.
		if( listSummonedMonsters.empty() )
			m_mapSummonMonsterByGroup.erase( iGroupID );
	}

#if defined(PRE_ADD_65808)
	if (hSummonedMonster)
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hSummonedMonster.GetPointer());
		if (pMonsterActor)
		{
			RemoveSummonMonsterGlyphStateEffects(pMonsterActor->GetMonsterClassID());
		}
	}
#endif // PRE_ADD_65808
}

// Called right before monsters in the stage are deleted.
// Extract and hold the required info here.
void CDnPlayerActor::OnBeforeDestroyStageMonsters( void )
{
	std::list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO>::iterator iter = m_listSummonedMonstersFollowStageInfos.begin();
	for( iter; iter != m_listSummonedMonstersFollowStageInfos.end(); )
	{
		// If a monster handle in the list is invalid, discard it on stage transition.
		if( iter->hMonster )
		{
			iter->iRemainDestroyTime = iter->hMonster->GetRemainDestroyTime();
			iter->bReCreatedFollowStageMonster = false;

			// TODO: Receive any additional info if needed.
			
			++iter;
		}
		else
		{
			iter = m_listSummonedMonstersFollowStageInfos.erase( iter );
		}
	}
}

void CDnPlayerActor::OnInitializeNextStageFinished( void )
{
	// Recreate removed summoned monsters. Re-run STE_SummonMonster signal as-is.
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	std::list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO>::iterator iter = m_listSummonedMonstersFollowStageInfos.begin();
	for( iter; iter != m_listSummonedMonstersFollowStageInfos.end(); ++iter )
	{
		// Since this monster is respawned upon stage transition, only actor handle update is done in PushMonster().
		// Set the duration of the re-summoned monster to the remaining duration.
		iter->SummonMonsterSignalData.nLifeTime = (int)iter->iRemainDestroyTime;
		pTask->RequestSummonMonster( GetMySmartPtr(), &(iter->SummonMonsterSignalData), true );
	}

#ifdef PRE_ADD_WEEKLYEVENT
	RemoveEventStateBlow();
	ApplyEventStateBlow();
#endif

#if defined( PRE_FIX_70618 )
	if( IsAppliedThisStateBlow(STATE_BLOW::BLOW_078) )
		CmdRemoveStateEffect(STATE_BLOW::BLOW_078);
#endif	// #if defined( PRE_FIX_70618 )
}

bool CDnPlayerActor::_bIsMasterSystemDurabilityReward()
{
	if( GetGameRoom() && GetGameRoom()->GetMasterRewardSystem() )
	{
		if( GetGameRoom()->GetMasterRewardSystem()->bIsDurabilityReward( m_pSession ) )
		{
			return true;
		}
	}
	return false;
}

void CDnPlayerActor::OnRepairEquipDurability( bool bDBSave, INT64 nPriceCoin )
{
	bool bRefreshStatus = false;

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();

	for( int i=CDnParts::Helmet; i<=CDnParts::Ring2; i++ ) {
		DnPartsHandle hParts = GetParts( (CDnParts::PartsTypeEnum)i );
		if( !hParts ) continue;
		if( hParts->IsInfinityDurability() ) continue;
		if( hParts->GetDurability() == hParts->GetMaxDurability() ) continue;

		int nTemp = hParts->GetMaxDurability();
		hParts->SetDurability( nTemp );
		bRefreshStatus = true;

		m_pSession->GetItem()->SetEquipItemDurability( i, nTemp, true );
		if (bDBSave && m_pSession->GetItem()->GetEquip(i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(i)->wDur);
		}
	}
	for( int i=0; i<2; i++ ) {
		DnWeaponHandle hWeapon = m_hWeapon[i];
		if( !hWeapon ) continue;
		if( hWeapon->IsInfinityDurability() ) continue;
		if( hWeapon->GetDurability() == hWeapon->GetMaxDurability() ) continue;

		int nTemp = hWeapon->GetMaxDurability();
		hWeapon->SetDurability( nTemp );
		bRefreshStatus = true;

		m_pSession->GetItem()->SetEquipItemDurability( EQUIP_WEAPON1 + i, nTemp, true );
		if (bDBSave && m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->wDur);
		}
	}
	if( bRefreshStatus ) {
		RefreshState( RefreshEquip, ST_All );
	}

	if (bDBSave)
	{
		INT64 biCurrentCoin = 0;
		INT64 biPickUpCoin = 0;

		if( nPriceCoin > 0 )
		{
			biCurrentCoin = m_pSession->GetCoin();
			biPickUpCoin = m_pSession->GetPickUpCoin();
			m_pSession->SelPickUpCoin(0);
		}
		
		m_pSession->GetDBConnection()->QueryModItemDurability(m_pSession, nPriceCoin, VecSerialList, VecDurList, biCurrentCoin, biPickUpCoin);
	}
}

void CDnPlayerActor::OnDecreaseEquipDurability( int nValue, bool bDBSave )
{
	if( _bIsMasterSystemDurabilityReward() )
		return;

	bool bRefreshStatus = false;

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();

	for( int i=CDnParts::Helmet; i<=CDnParts::Ring2; i++ ) {
		DnPartsHandle hParts = GetParts( (CDnParts::PartsTypeEnum)i );
		if( !hParts ) continue;
		if( hParts->IsInfinityDurability() ) continue;
		if( hParts->GetDurability() == 0 ) continue;

		int nTemp = hParts->GetDurability() - nValue;
		if( nTemp <= 0 ) {
			nTemp = 0;
			bRefreshStatus = true;
		}
		hParts->SetDurability( nTemp );

		m_pSession->GetItem()->SetEquipItemDurability( i, nTemp );
		if (bDBSave && m_pSession->GetItem()->GetEquip(i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(i)->wDur);
		}
	}
	for( int i=0; i<2; i++ ) {
		DnWeaponHandle hWeapon = m_hWeapon[i];
		if( !hWeapon ) continue;
		if( hWeapon->IsInfinityDurability() ) continue;
		if( hWeapon->GetDurability() == 0 ) continue;

		int nTemp = hWeapon->GetDurability() - nValue;
		if( nTemp <= 0 ) {
			nTemp = 0;
			bRefreshStatus = true;
		}
		hWeapon->SetDurability( nTemp );

		m_pSession->GetItem()->SetEquipItemDurability( EQUIP_WEAPON1 + i, nTemp );
		if (bDBSave && m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->wDur);
		}
	}
	if( bRefreshStatus ) {
		RefreshState( RefreshEquip, ST_All );
	}

	if (bDBSave)
		m_pSession->GetDBConnection()->QueryModItemDurability(m_pSession, 0, VecSerialList, VecDurList);
}

void CDnPlayerActor::OnDecreaseEquipDurability( float fValue, bool bDBSave )
{
	if( _bIsMasterSystemDurabilityReward() )
		return;

	bool bRefreshStatus = false;

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	float fTotalLevelValue = 0.0f;
	if ( m_pRoom->bIsPvPRoom() == false && IsAppliedThisStateBlow(STATE_BLOW::BLOW_259))
	{
		DNVector(DnBlowHandle) vlBlows;
		GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_259, vlBlows);
		{
			int nCount = (int)vlBlows.size();
			for (int i = 0; i < nCount; ++i)
			{
				DnBlowHandle hBlow = vlBlows[i];
				if (hBlow && hBlow->IsEnd() == false)
				{
					fTotalLevelValue += hBlow->GetFloatValue();
				}
			}
		}
	}
#endif

	for( int i=CDnParts::Helmet; i<=CDnParts::Ring2; i++ ) {
		DnPartsHandle hParts = GetParts( (CDnParts::PartsTypeEnum)i );
		if( !hParts ) continue;
		if( hParts->IsInfinityDurability() ) continue;
		if( hParts->GetDurability() == 0 ) continue;

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
		int nTemp = (int)( hParts->GetDurability() - (( hParts->GetMaxDurability() * fValue ) * (1 - fTotalLevelValue)) );
#else
		int nTemp = (int)( hParts->GetDurability() - ( hParts->GetMaxDurability() * fValue ) );
#endif		
		if( nTemp <= 0 ) {
			nTemp = 0;
			bRefreshStatus = true;
		}
		hParts->SetDurability( nTemp );
		m_pSession->GetItem()->SetEquipItemDurability( i, nTemp );
		if (bDBSave && m_pSession->GetItem()->GetEquip(i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(i)->wDur);
		}
	}
	for( int i=0; i<2; i++ ) {
		DnWeaponHandle hWeapon = m_hWeapon[i];
		if( !hWeapon ) continue;
		if( hWeapon->IsInfinityDurability() ) continue;
		if( hWeapon->GetDurability() == 0 ) continue;

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
		int nTemp = (int)( hWeapon->GetDurability() - (( hWeapon->GetMaxDurability() * fValue ) * (1 - fTotalLevelValue)) );
#else
		int nTemp = (int)( hWeapon->GetDurability() - ( hWeapon->GetMaxDurability() * fValue ) );
#endif
		if( nTemp <= 0 ) {
			nTemp = 0;
			bRefreshStatus = true;
		}

		hWeapon->SetDurability( nTemp );
		m_pSession->GetItem()->SetEquipItemDurability( EQUIP_WEAPON1 + i, nTemp );
		if (bDBSave && m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)){
			VecSerialList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->nSerial);
			VecDurList.push_back(m_pSession->GetItem()->GetEquip(EQUIP_WEAPON1 + i)->wDur);
		}
	}
	if( bRefreshStatus ) {
		RefreshState( RefreshEquip, ST_All );
	}

	if (bDBSave)
		m_pSession->GetDBConnection()->QueryModItemDurability(m_pSession, 0, VecSerialList, VecDurList);
}

void CDnPlayerActor::OnDecreaseInvenDurability( int nValue, bool bDBSave )
{
	if( _bIsMasterSystemDurabilityReward() )
		return;

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();

	for( int i=0; i<INVENTORYMAX; i++ ) {
		const TItem *pItem = m_pSession->GetItem()->GetInventory(i);
		if( !pItem ) continue;
		switch( g_pDataManager->GetItemMainType( pItem->nItemID ) ) {
			case ITEMTYPE_WEAPON:
				{
					CDnWeapon *pWeapon = static_cast<CDnWeapon*>(m_pPartyData->pInventory[i]);
					if( !pWeapon ) continue;
					if( pWeapon->IsInfinityDurability() ) continue;
					if( pWeapon->GetDurability() == 0 ) continue;

					int nTemp = pWeapon->GetDurability() - nValue;
					if( nTemp <= 0 ) nTemp = 0;

					pWeapon->SetDurability( nTemp );

					m_pSession->GetItem()->SetInvenItemDurability( i, nTemp );
					if (bDBSave && m_pSession->GetItem()->GetInventory(i)){
						VecSerialList.push_back(m_pSession->GetItem()->GetInventory(i)->nSerial);
						VecDurList.push_back(m_pSession->GetItem()->GetInventory(i)->wDur);
					}
				}
				break;
			case ITEMTYPE_PARTS:
				{
					CDnParts *pParts = static_cast<CDnParts*>(m_pPartyData->pInventory[i]);
					if( pParts->IsInfinityDurability() ) continue;
					if( pParts->GetDurability() == 0 ) continue;

					int nTemp = pParts->GetDurability() - nValue;
					if( nTemp <= 0 ) nTemp = 0;
					pParts->SetDurability( nTemp );

					m_pSession->GetItem()->SetInvenItemDurability( i, nTemp );
					if (bDBSave && m_pSession->GetItem()->GetInventory(i)){
						VecSerialList.push_back(m_pSession->GetItem()->GetInventory(i)->nSerial);
						VecDurList.push_back(m_pSession->GetItem()->GetInventory(i)->wDur);
					}
				}
				break;
			default:
				continue;
		}
	}

	if (bDBSave)
		m_pSession->GetDBConnection()->QueryModItemDurability(m_pSession, 0, VecSerialList, VecDurList);
}

void CDnPlayerActor::OnDecreaseInvenDurability( float fValue, bool bDBSave )
{
	if( _bIsMasterSystemDurabilityReward() )
		return;

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();

	for( int i=0; i<INVENTORYMAX; i++ ) {
		const TItem *pItem = m_pSession->GetItem()->GetInventory(i);
		if( !pItem ) continue;
		switch( g_pDataManager->GetItemMainType( pItem->nItemID ) ) {
			case ITEMTYPE_WEAPON:
				{
					CDnWeapon *pWeapon = static_cast<CDnWeapon*>(m_pPartyData->pInventory[i]);
					if( !pWeapon ) continue;
					if( pWeapon->IsInfinityDurability() ) continue;
					if( pWeapon->GetDurability() == 0 ) continue;

					int nTemp = (int)( pWeapon->GetDurability() - ( pWeapon->GetMaxDurability() * fValue ) );
					if( nTemp <= 0 ) nTemp = 0;

					pWeapon->SetDurability( nTemp );

					m_pSession->GetItem()->SetInvenItemDurability( i, nTemp );
					if (m_pSession->GetItem()->GetInventory(i)){
						VecSerialList.push_back(m_pSession->GetItem()->GetInventory(i)->nSerial);
						VecDurList.push_back(m_pSession->GetItem()->GetInventory(i)->wDur);
					}
				}
				break;
			case ITEMTYPE_PARTS:
				{
					CDnParts *pParts = static_cast<CDnParts*>(m_pPartyData->pInventory[i]);
					if( !pParts ) continue;
					if( pParts->IsInfinityDurability() ) continue;
					if( pParts->GetDurability() == 0 ) continue;

					int nTemp = (int)( pParts->GetDurability() - ( pParts->GetMaxDurability() * fValue ) );
					if( nTemp <= 0 ) nTemp = 0;
					pParts->SetDurability( nTemp );

					m_pSession->GetItem()->SetInvenItemDurability( i, nTemp );
					if (m_pSession->GetItem()->GetInventory(i)){
						VecSerialList.push_back(m_pSession->GetItem()->GetInventory(i)->nSerial);
						VecDurList.push_back(m_pSession->GetItem()->GetInventory(i)->wDur);
					}
				}
				break;
			default:
				continue;
		}
	}

	m_pSession->GetDBConnection()->QueryModItemDurability(m_pSession, 0, VecSerialList, VecDurList);
}

bool CDnPlayerActor::IsPenaltyStageGiveUp()
{
	if (!m_pRoom) return false;
	switch( m_pRoom->GetGameTaskType() ) {
		case GameTaskType::PvP: return false;
		case GameTaskType::DarkLair: return false;
		case GameTaskType::Farm: return false;
		default: break;
	}

	if (IsDie())
		return false;

	if (CDnWorld::IsActive(GetRoom()))
	{
		EWorldEnum::MapTypeEnum mapType = CDnWorld::GetInstance(GetRoom()).GetMapType();
		if (mapType == EWorldEnum::MapTypeDungeon)
		{
			CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
			if (pTask->GetDungeonClearState() == CDnGameTask::DCS_WarpStandBy)
				return false;
		}
		else
		{
			return false;
		}
	}

	return true;
}

// If durability decreased, return true; otherwise return false;
bool CDnPlayerActor::OnStageGiveUp()
{
	if (IsPenaltyStageGiveUp() == false)
		return false;
	DNTableFileFormat *pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( pDungeonSox->GetFieldFromLablePtr( GetGameRoom()->GetGameTask()->GetDungeonEnterTableID(), "_StageOutDurability" )->GetInteger() == 0)	
		return false;
	OnDecreaseEquipDurability( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StageGiveupDurabilityPenalty ), true );
	OnDecreaseInvenDurability( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StageGiveupDurabilityPenalty ), true );

	// If you give up and leave, Equip updates automatically, but Inventory does not, so we need to notify it.
	if( m_pSession ) {
		float fRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StageGiveupDurabilityPenalty );
		m_pSession->SendDecreaseDurabilityInventory( 1, (void*)&fRatio );
	}

	return true;
}

void CDnPlayerActor::RecvPartyRefreshGateInfo( const EtVector3& Pos )
{
	SetPosition( Pos );
	SetStaticPosition( Pos );
}

bool CDnPlayerActor::AttachParts( DnPartsHandle hParts, CDnParts::PartsTypeEnum Index, bool bDelete )
{
	if( !hParts )
		return false;

	bool bResult = MAPartsBody::AttachParts( hParts, Index, bDelete );

#if defined(_GAMESERVER)
	int nSkillID = -1;
	int nSkillLevel = 0;

	if (hParts->HasPrefixSkill(nSkillID, nSkillLevel))
	{
		DnSkillHandle hSkill = CDnSkill::CreateSkill(GetMySmartPtr(), nSkillID, nSkillLevel);
		if (!hSkill)
		{
			OutputDebug("%s ==> SkillID %d, SkillLevel %d...  CreateSkill Failed!!!!\n", __FUNCTION__, nSkillID, nSkillLevel);
		}
		else 
		{
#if defined(PRE_ADD_PREFIXSKILL_PVP)
			// Set skill level data separately for PvE and PvP.
			int iSkillLevelDataType = CDnSkill::PVE;
			if( GetGameRoom()->bIsPvPRoom() )
				iSkillLevelDataType = CDnSkill::PVP;

			hSkill->SelectLevelDataType( iSkillLevelDataType );
#endif // PRE_ADD_PREFIXSKILL_PVP

			if (!MASkillUser::AddPreFixSystemDefenceSkill(Index, hSkill))
			{
				SAFE_RELEASE_SPTR(hSkill);
				OutputDebug("MASkillUser::AddPreFixSystemDeffenceSkill ===> slotIndex %d, ItemID %d, SkillID %d, SkillLevel %d AddPreFixSystem Deffence skill Failed !!!\n", 
					Index, hParts->GetClassID(), nSkillID, nSkillLevel);
			}
		}
	}
#endif // _GAMESERVER

	int nLevelUpSkillID = -1;
	int nLevelUpSkillLevelValue = 0;
	int nLevelUpItemSkillUsingType = 0;
	if (hParts->HasLevelUpInfo(nLevelUpSkillID, nLevelUpSkillLevelValue, nLevelUpItemSkillUsingType))
	{
		if (CDnItem::ItemSkillApplyType::SkillLevelUp == nLevelUpItemSkillUsingType)
			AddSkillLevelUpInfo(Index, nLevelUpSkillID, nLevelUpSkillLevelValue);
	}

	return bResult;
}

bool CDnPlayerActor::DetachParts( CDnParts::PartsTypeEnum Index )
{

#if defined(_GAMESERVER)
	MASkillUser::RemovePreFixSystemDefenceSkill(Index);
#endif // _GAMESERVER

	RemoveSkillLevelUpInfo(Index);

	// Note: Skill may disappear even if parts separation fails.
	bool bResult = MAPartsBody::DetachParts( Index );
	return bResult;
}

bool CDnPlayerActor::AttachCashParts( DnPartsHandle hParts, CDnParts::PartsTypeEnum Index, bool bDelete )
{
	if( !hParts ) return false;

	bool bResult = MAPartsBody::AttachCashParts( hParts, Index, bDelete );

	int nLevelUpSkillID = -1;
	int nLevelUpSkillLevelValue = 0;
	int nLevelUpItemSkillUsingType = 0;
	if (hParts->HasLevelUpInfo(nLevelUpSkillID, nLevelUpSkillLevelValue, nLevelUpItemSkillUsingType))
	{
		if (CDnItem::ItemSkillApplyType::SkillLevelUp == nLevelUpItemSkillUsingType)
			AddSkillLevelUpInfoByCashItem(Index, nLevelUpSkillID, nLevelUpSkillLevelValue);
	}

	return bResult;
}

bool CDnPlayerActor::DetachCashParts( CDnParts::PartsTypeEnum Index )
{
	RemoveSkillLevelUpInfoByCashItem(Index);

	// Note: Skill may disappear even if parts separation fails.
	bool bResult = MAPartsBody::DetachCashParts( Index );
	return bResult;
}

void CDnPlayerActor::ReplacementGlyph( DnSkillHandle hNewSkill )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"GlyphSkillTable.ext failed\r\n");
		return;
	}

	int iSkillID = hNewSkill->GetClassID();

	for( int itr = 0; itr < CDnGlyph::GlyphSlotEnum_Amount; ++itr )
	{
		if( m_hGlyph[itr] )
		{
			int eType = pSox->GetFieldFromLablePtr( m_hGlyph[itr]->GetClassID(), "_GlyphType" )->GetInteger();
			int iGlyphSkillID = pSox->GetFieldFromLablePtr( m_hGlyph[itr]->GetClassID(), "_SkillID" )->GetInteger();

			if( CDnGlyph::PassiveSkill == eType && iSkillID == iGlyphSkillID )
				hNewSkill->AddGlyphStateEffect( m_hGlyph[itr]->GetClassID() );
		}
	}
}

bool CDnPlayerActor::AttachGlyph( DnGlyphHandle hGlyph, CDnGlyph::GlyphSlotEnum Index, bool bDelete /* = false */ )
{
	if( !MAPlateUser::AttachGlyph( hGlyph, Index, bDelete ) )
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"GlyphSkillTable.ext failed\r\n");
		return false;
	}

	int eType = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_GlyphType" )->GetInteger();

	// If the crest has a skill, determine whether it adds a skill or adds a skill effect.
	if( CDnItem::TemperedSkill == eType )
	{
		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
		if( !pSox ) 
		{
			g_Log.Log( LogType::_FILELOG, L"GlyphSkillTable.ext failed\r\n");
			return false;
		}

		int nSkillID = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_SkillID" )->GetInteger();

		DnSkillHandle hSkill = FindSkill( nSkillID );
		if( !hSkill ) return false;

		//Need to add SkillEffect to CDnSkill
		hSkill->AddGlyphStateEffect( hGlyph->GetClassID() );
	}
	else if( CDnItem::AddSKill == eType && 0 != hGlyph->GetSkillID() &&  0 != hGlyph->GetSkillLevel() )
	{
		AddSkill( hGlyph->GetSkillID(), hGlyph->GetSkillLevel() );
		DnSkillHandle hSkill = FindSkill( hGlyph->GetSkillID() );

		if( !hSkill )	return false;

		hSkill->AsEquipItemSkill();
		
		hSkill->SetEquipIndex( Index );

		m_ahEquipSkill = hSkill;

		if( 0.0f != m_afLastEquipItemSkillDelayTime )
		{
			hSkill->SetOnceCoolTime( m_afLastEquipItemSkillDelayTime, m_afLastEquipItemSkillRemainTime );
		}
	}

	return true;
}

bool CDnPlayerActor::DetachGlyph( CDnGlyph::GlyphSlotEnum Index )
{
	DnGlyphHandle hGlyph = m_hGlyph[Index];
	if( !hGlyph )	return false;

	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TGLYPHSKILL );
	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"GlyphSkillTable.ext failed\r\n");
		return false;
	}

	int eType = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_GlyphType")->GetInteger();

	if( CDnItem::TemperedSkill == eType )
	{
		int nSkillID = pSox->GetFieldFromLablePtr( hGlyph->GetClassID(), "_SkillID")->GetInteger();

		DnSkillHandle hSkill = FindSkill( nSkillID );
		if( !hSkill ) return false;

		hSkill->DelGlyphStateEffect( hGlyph->GetClassID() );
	}
	else if( CDnItem::AddSKill == eType && 0 != hGlyph->GetSkillID() && 0 != hGlyph->GetSkillLevel() )
	{
		DnSkillHandle hSkill = FindSkill( hGlyph->GetSkillID() );
		m_afLastEquipItemSkillDelayTime = hSkill->GetDelayTime();
		m_afLastEquipItemSkillRemainTime = hSkill->GetElapsedDelayTime();

		RemoveSkill( hGlyph->GetSkillID() );
	}

	return true;
}

void CDnPlayerActor::AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex /*= 0*/, bool bDelete/* = false */)
{
	CDnActor::AttachWeapon( hWeapon, nEquipIndex, bDelete );

#if defined(PRE_ADD_50907)
	if (IsSkipOnAttatchDetachWeapon() == true)
		return;
#endif // PRE_ADD_50907

	RefreshWeaponViewOrder( nEquipIndex );

	if( m_hCashWeapon[nEquipIndex] ) {
		m_hCashWeapon[nEquipIndex]->RecreateCashWeapon( GetMySmartPtr(), nEquipIndex );
		LinkCashWeapon( nEquipIndex );
	}

#if defined(_GAMESERVER)
	int nSkillID = -1;
	int nSkillLevel = 0;

	if (hWeapon->HasPrefixSkill(nSkillID, nSkillLevel))
	{
		DnSkillHandle hSkill = CDnSkill::CreateSkill(GetMySmartPtr(), nSkillID, nSkillLevel);
		if (!hSkill)
		{
			OutputDebug("%s ==> SkillID %d, SkillLevel %d...  CreateSkill Failed!!!!\n", __FUNCTION__, nSkillID, nSkillLevel);
		}
		else 
		{
#if defined(PRE_ADD_PREFIXSKILL_PVP)
			// Set skill level data separately for PvE and PvP.
			int iSkillLevelDataType = CDnSkill::PVE;
			if( GetGameRoom()->bIsPvPRoom() )
				iSkillLevelDataType = CDnSkill::PVP;

			hSkill->SelectLevelDataType( iSkillLevelDataType );
#endif // PRE_ADD_PREFIXSKILL_PVP

			if (!MASkillUser::AddPreFixSystemOffenceSkill(nEquipIndex, hSkill))
			{
				SAFE_RELEASE_SPTR(hSkill);
				OutputDebug("MASkillUser::AddPreFixSystemOffenceSkill ===> slotIndex %d, ItemID %d, SkillID %d, SkillLevel %d AddPreFixSystem Deffence skill Failed !!!\n", 
					nEquipIndex, hWeapon->GetClassID(), nSkillID, nSkillLevel);
			}
		}
	}
#endif // _GAMESERVER

	int nLevelUpSkillID = -1;
	int nLevelUpSkillLevelValue = 0;
	int nLevelUpItemSkillUsingType = 0;
	if (hWeapon->HasLevelUpInfo(nLevelUpSkillID, nLevelUpSkillLevelValue, nLevelUpItemSkillUsingType))
	{
		if (CDnItem::ItemSkillApplyType::SkillLevelUp == nLevelUpItemSkillUsingType)
			AddSkillLevelUpInfo(CDnParts::PartsTypeEnum::PartsTypeEnum_Amount+nEquipIndex, nLevelUpSkillID, nLevelUpSkillLevelValue);
	}

}

void CDnPlayerActor::DetachWeapon( int nEquipIndex/* = 0*/ )
{
	CDnActor::DetachWeapon( nEquipIndex );

#if defined(PRE_ADD_50907)
	if (IsSkipOnAttatchDetachWeapon() == true)
		return;
#endif // PRE_ADD_50907

	RefreshWeaponViewOrder( nEquipIndex );

#if defined(_GAMESERVER)
	MASkillUser::RemovePreFixSystemOffenceSkill(nEquipIndex);
#endif // _GAMESERVER

	RemoveSkillLevelUpInfo(CDnParts::PartsTypeEnum::PartsTypeEnum_Amount+nEquipIndex);
}

void CDnPlayerActor::AttachCashWeapon( DnWeaponHandle hWeapon, int nEquipIndex, bool bDelete )
{
	if( m_hCashWeapon[nEquipIndex] ) {
		DetachCashWeapon( nEquipIndex );
	}
	m_hCashWeapon[nEquipIndex] = hWeapon;
	m_bCashSelfDeleteWeapon[nEquipIndex] = bDelete;
	if( !m_hCashWeapon[nEquipIndex] ) return;
	m_hCashWeapon[nEquipIndex]->CreateObject();

	RefreshWeaponViewOrder( nEquipIndex );
	m_hCashWeapon[nEquipIndex]->RecreateCashWeapon( GetMySmartPtr(), nEquipIndex );
	LinkCashWeapon( nEquipIndex );

	int nLevelUpSkillID = -1;
	int nLevelUpSkillLevelValue = 0;
	int nLevelUpItemSkillUsingType = 0;
	if (hWeapon->HasLevelUpInfo(nLevelUpSkillID, nLevelUpSkillLevelValue, nLevelUpItemSkillUsingType))
	{
		if (CDnItem::ItemSkillApplyType::SkillLevelUp == nLevelUpItemSkillUsingType)
			AddSkillLevelUpInfoByCashItem(CASHEQUIP_WEAPON1+nEquipIndex, nLevelUpSkillID, nLevelUpSkillLevelValue);
	}
}

void CDnPlayerActor::LinkCashWeapon( int nEquipIndex )
{
	switch( m_hCashWeapon[nEquipIndex]->GetEquipType() ) {
		case CDnWeapon::Sword:
		case CDnWeapon::Axe:
		case CDnWeapon::Hammer:
		case CDnWeapon::SmallBow:
		case CDnWeapon::BigBow:
		case CDnWeapon::CrossBow:
		case CDnWeapon::Staff:
		case CDnWeapon::Book:
		case CDnWeapon::Orb:
		case CDnWeapon::Puppet:
		case CDnWeapon::Mace:
		case CDnWeapon::Flail:
		case CDnWeapon::Wand:
		case CDnWeapon::Shield:
		case CDnWeapon::Gauntlet:
			m_hCashWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex );
			break;
		case CDnWeapon::Arrow:
			m_hCashWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), m_hCashWeapon[0] );
			break;
	}
}

void CDnPlayerActor::DetachCashWeapon( int nEquipIndex )
{
	if( !m_hCashWeapon[nEquipIndex] ) return;

	m_hCashWeapon[nEquipIndex]->FreeObject();
	m_hCashWeapon[nEquipIndex]->UnlinkWeapon();

	if( m_bCashSelfDeleteWeapon[nEquipIndex] ) {
		SAFE_RELEASE_SPTR( m_hCashWeapon[nEquipIndex] );
		m_bCashSelfDeleteWeapon[nEquipIndex] = false;
	}
	m_hCashWeapon[nEquipIndex].Identity();

	RefreshWeaponViewOrder( nEquipIndex );

	RemoveSkillLevelUpInfoByCashItem(CASHEQUIP_WEAPON1+nEquipIndex);
}

void CDnPlayerActor::ShowCashWeapon( int nEquipIndex, bool bShow )
{
	if( m_hCashWeapon[nEquipIndex] )
		m_hCashWeapon[nEquipIndex]->ShowWeapon( bShow );
}

void CDnPlayerActor::SetWeaponViewOrder( int nEquipIndex, bool bShowCash )
{
	if( nEquipIndex < 0 || nEquipIndex >= 2 ) return;
	m_bWeaponViewOrder[nEquipIndex] = bShowCash;
}

void CDnPlayerActor::RefreshWeaponViewOrder( int nEquipIndex )
{
	if( nEquipIndex < 0 || nEquipIndex >= 2 ) return;
	if( m_hWeapon[nEquipIndex] && m_hCashWeapon[nEquipIndex] ) {
		if( m_bWeaponViewOrder[nEquipIndex] ) {
			if( !m_hCashWeapon[nEquipIndex]->IsCreateObject() ) {
				m_hCashWeapon[nEquipIndex]->CreateObject();
				m_hCashWeapon[nEquipIndex]->RecreateCashWeapon( GetMySmartPtr(), nEquipIndex );
				LinkCashWeapon( nEquipIndex );
				m_hCashWeapon[nEquipIndex]->ShowWeapon( true );
			}

			if( m_hWeapon[nEquipIndex]->IsCreateObject() ) {
				m_hWeapon[nEquipIndex]->FreeObject();
				m_hWeapon[nEquipIndex]->ShowWeapon( false );
			}
		}
		else {
			if( !m_hWeapon[nEquipIndex]->IsCreateObject() ) {
				m_hWeapon[nEquipIndex]->CreateObject();
				LinkWeapon( nEquipIndex );
				m_hWeapon[nEquipIndex]->ShowWeapon( true );
			}

			if( m_hCashWeapon[nEquipIndex]->IsCreateObject() ) {
				m_hCashWeapon[nEquipIndex]->FreeObject();
				m_hCashWeapon[nEquipIndex]->ShowWeapon( false );
			}
		}
	}
	else if( m_hWeapon[nEquipIndex] && !m_hCashWeapon[nEquipIndex] ) {
		if( !m_hWeapon[nEquipIndex]->IsCreateObject() ) {
			m_hWeapon[nEquipIndex]->CreateObject();
			LinkWeapon( nEquipIndex );
			m_hWeapon[nEquipIndex]->ShowWeapon( true );
		}
	}
	else {
		if( nEquipIndex == 0 ) {
			if( m_hCashWeapon[nEquipIndex] && m_hCashWeapon[nEquipIndex]->IsCreateObject() ) {
				m_hCashWeapon[nEquipIndex]->FreeObject();
				m_hCashWeapon[nEquipIndex]->ShowWeapon( false );
			}
		}
	}
	SetBattleMode( IsBattleMode() );
}

void CDnPlayerActor::OnEventCP( CPTypeEnum Type, int nResult )
{
#if defined( PRE_ADD_CP_RENEWAL )
	MACP_Renewal::OnEventCP( Type, nResult );
#else	// #if defined( PRE_ADD_CP_RENEWAL )
	MACP::OnEventCP( Type, nResult );
#endif	// #if defined( PRE_ADD_CP_RENEWAL )

	switch( Type ) {
		case MACP::MaxComboCount:
		case MACP::KillBossCount:
		case MACP::SuperAmmorBreakScore:
		case MACP::GenocideScore:
		case MACP::AirComboScore:
		case MACP::RebirthPlayerScore:
		case MACP::ComboScore:
		case MACP::PartyComboScore:
		case MACP::AssistMonsterScore:
			{
				BYTE pBuffer[32];
				CPacketCompressStream Stream( pBuffer, 32 );

				Stream.Write( &Type, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Write( &nResult, sizeof(int) );

				Send( eActor::SC_CP, &Stream );
			}
			break;
	}
}

void CDnPlayerActor::UpdateAttackedCPPoint( CDnDamageBase *pHitter , CDnWeapon::HitTypeEnum eHitType )
{
	bool bIsSameTeam = false;
	if( pHitter && pHitter->GetActorHandle() )
	{
		if( GetTeam() == pHitter->GetActorHandle()->GetTeam() )
		{
			bIsSameTeam = true;
		}
	}

	if( bIsSameTeam == false )
	{
		switch( eHitType ) 
		{
		case CDnWeapon::Normal: 
		case CDnWeapon::CriticalRes: 
			{
				UpdateAttackedHit();
			}
			break;

		case CDnWeapon::Critical: 
			{
				UpdateAttackedCriticalHit(); 
			}
			break;
		case CDnWeapon::Stun: 
			{
				UpdateAttackedStunHit();
			}
			break;
		}
	}
}

DnWeaponHandle CDnPlayerActor::GetActiveWeapon( int nEquipIndex )
{
	if( m_bWeaponViewOrder[nEquipIndex] && m_hCashWeapon[nEquipIndex] ) return m_hCashWeapon[nEquipIndex];
	return CDnActor::GetActiveWeapon( nEquipIndex );
}

float CDnPlayerActor::PreCalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam, const float fDefenseRate, float fStateEffectAttackM )
{
	float fResult = CDnActor::PreCalcDamage( pHitter, HitParam, fDefenseRate, fStateEffectAttackM );
	return fResult;
}

bool CDnPlayerActor::IsDie()
{
	bool bResult = CDnPlayerState::IsDie();
	if( bResult ) return true;

	// No need to return true in PvP.
	if( IsGMTrace() && GetGameRoom() && !GetGameRoom()->bIsPvPRoom() )
		return true;

	return false;
}

bool CDnPlayerActor::SetActionQueue( const char *szActionName, int nLoopCount, float fBlendFrame , float fStartFrame , bool bCheck , bool bCheckStateEffect )
{
	if(m_bShootMode && !m_bTransformMode && m_bBattleMode)
		szActionName = GetChangeShootActionName(szActionName);

#if defined(PRE_FIX_63219)
	//#63219
	//When block triggers while pressing left/right move keys, OnStop is called in MAWalkMovement and
	//At this point, CmdStop sets the "Stand" action.
	//If block triggers while moving on client and passive skill is used via special attack key
	//Changes to Stand action on the server, so the skill does not execute on the server.
	//Ignore transition to Stand during Block action for now
	if ((strstr(m_szAction.c_str(), "Block") != NULL ||
		strstr(m_szActionQueue.c_str(), "Block") != NULL)&&
		strstr(szActionName, "Stand") != NULL)
	{
		//#68376 Stance of Faith bug
		//Must transition to Stand from actions like Skill_StandOfFaith_EX_Block...
		//Modified to transition if "StandOfFaith" exists since there may be other Blocks.
		bool isSkillAction = (strstr(m_szAction.c_str(), "StandOfFaith") != NULL || strstr(m_szActionQueue.c_str(), "StandOfFaith"));
		
		if (isSkillAction == false)
			return false;
	}
#endif // PRE_FIX_63219

	return CDnActor::SetActionQueue( szActionName , nLoopCount, fBlendFrame, fStartFrame, bCheck, bCheckStateEffect );
}

void CDnPlayerActor::_UpdateMaxProjectileCount( int nActionIndex, bool bUpdateReservedCount/* = false*/ )
{
	if( NULL == m_pProjectileCountInfo )
		return;

	// Maximum number of projectiles that can be fired in the current action.
	m_iNowMaxProjectileCount = 0;
	map<int, int>::const_iterator iterProj = m_pProjectileCountInfo->mapMaxProjectileCountInAction.find( nActionIndex );
	if( m_pProjectileCountInfo->mapMaxProjectileCountInAction.end() != iterProj )
		m_iNowMaxProjectileCount = iterProj->second;

	// Weapon table index used in projectile signal order
	// Normally, if projectile signals and weapon SendAction are not mixed, the deck is not reset and keeps accumulating in the weapon check loop below, so
	// Clear once here.
	m_setWeaponIDUsingProjectileSignal.clear();
	map<int, multiset<int> >::const_iterator iterWeaponIDs = m_pProjectileCountInfo->mapUsingProjectileWeaponTableIDs.find( nActionIndex );
	if( m_pProjectileCountInfo->mapUsingProjectileWeaponTableIDs.end() != iterWeaponIDs )
		m_setWeaponIDUsingProjectileSignal = iterWeaponIDs->second;

	// Collection of frames in the current action where projectiles are fired.
	// Normally, if projectile signals and weapon SendAction are not mixed, the deck is not reset and keeps accumulating in the weapon check loop below, so
	// Clear once here.
	m_dqProjectileSignalOffset.clear();
	map<int, deque<int> >::const_iterator iterProjOffset = m_pProjectileCountInfo->mapProjectileSignalFrameOffset.find( nActionIndex );
	if( m_pProjectileCountInfo->mapProjectileSignalFrameOffset.end() != iterProjOffset )
		m_dqProjectileSignalOffset = iterProjOffset->second;

	sort( m_dqProjectileSignalOffset.begin(), m_dqProjectileSignalOffset.end() );

	map<int, vector<CDnActionSpecificInfo::S_WEAPONACTION_INFO> >::const_iterator iterWeapon = m_pProjectileCountInfo->mapSendActionWeapon.find( nActionIndex );
	if( m_pProjectileCountInfo->mapSendActionWeapon.end() != iterWeapon )
	{
		const vector<CDnActionSpecificInfo::S_WEAPONACTION_INFO>& vlWeaponInfo = iterWeapon->second;
		for( int i = 0; i < (int)vlWeaponInfo.size(); ++i )
		{
			const CDnActionSpecificInfo::S_WEAPONACTION_INFO& Struct = vlWeaponInfo.at( i );

			// Calling directly invokes TDnPlayer~~::GetActiveWeapon(), which retrieves the foot weapon when switching to a shooting action mid-kick and triggers the hack check, so changed to call CDnPlayerActor::GetActiveWeapon().
			DnWeaponHandle hWeapon = CDnPlayerActor::GetActiveWeapon( Struct.iWeaponIndex );		
			if( hWeapon ) 
			{
				if( false == Struct.strActionName.empty() ) 
				{
					// For player actors, add the weapon's projectile count to the number of projectiles that can be fired in the current action.
					if( hWeapon->IsExistAction( Struct.strActionName.c_str() ) )
					{
						hWeapon->SetActionQueue( Struct.strActionName.c_str() );
						if( m_ActorType <= Reserved6 )
						{
							int nWeaponActionIndex = hWeapon->GetElementIndex( Struct.strActionName.c_str() );
							int nAddCount = hWeapon->GetMaxProjectileCountInAction( nWeaponActionIndex );
							
							if( false == bUpdateReservedCount )
								m_iNowMaxProjectileCount += nAddCount;
							else
								m_iReservedProjectileCount += nAddCount;

							// Add weapon table info used by projectile fired from weapon.
							hWeapon->AddUsingProjectileWeaponTableIDs( nWeaponActionIndex, m_setWeaponIDUsingProjectileSignal );

							// Add frame info for projectile fired from weapon.
							// Putting a shoot action signal for a weapon on the last frame or similar may trigger this hack check routine.
							// There is no possibility of that happening right now, so pass for now.
							hWeapon->AddProjectileSignalOffset( nWeaponActionIndex, Struct.iFrame, m_dqProjectileSignalOffset );
							
							// Keep retrieved results sorted.
							// Keep sorted in frame order and use for comparison.
							sort( m_dqProjectileSignalOffset.begin(), m_dqProjectileSignalOffset.end() );
						}
					}
				}
			}
		}
	}
}


bool CDnPlayerActor::UseAndCheckAvailProjectileCount( void )
{
	if( m_iNowMaxProjectileCount <= 0 )
	{
		// If there is no reserved projectile count either (like firing a projectile from a projectile), judge as a hack.
		if( m_iReservedProjectileCount <= 0 )
		{
			// Firing projectiles wildly using a hack.
			OutputDebug( "CS_PROJECTILE: projectile count exceeded action maximum; suspected hack.\n" );
			
			return false;
		}

		--m_iReservedProjectileCount;
	}

	if( 0 < m_iNowMaxProjectileCount )
		--m_iNowMaxProjectileCount;

	return true;
}

bool CDnPlayerActor::IsExclusiveSkill( int iSkillID, int iExclusiveID )
{
#ifndef PRE_FIX_SKILLLIST
	// True if any learned skill has an ID that prevents learning the same skill.
	int iNumSkill = (int)m_vlhSkillList.size();
	for( int iSkill = 0; iSkill < iNumSkill; ++iSkill )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( iSkill );

		// Excluding own skills.
		if( hSkill->GetClassID() == iSkillID )
			continue;

		if( 0 == hSkill->GetExclusiveID() )
			continue;

		if( iExclusiveID == hSkill->GetExclusiveID() )
		{
			return true;
		}
	}
#else
	// True if any learned skill has an ID that prevents learning the same skill.
	DWORD dwNumSkill = GetSkillCount();
	for( DWORD i = 0; i < dwNumSkill; ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );

		// Excluding own skills.
		if( hSkill->GetClassID() == iSkillID )
			continue;

		if( 0 == hSkill->GetExclusiveID() )
			continue;

		if( iExclusiveID == hSkill->GetExclusiveID() )
		{
			return true;
		}
	}
#endif // #ifndef PRE_FIX_SKILLLIST

	return false;
}

int CDnPlayerActor::GetAvailSkillPointByJob( int iSkillID )
{
	// Current job tier
	int iJobDegree = g_pDataManager->GetJobNumber( GetJobClassID() );

	int iWholeSP = GetLevelUpSkillPoint( 1, GetLevel() );
	int iWholeAvailSPByJob = int(iWholeSP * m_pSession->GetAvailSkillPointRatioByJob( iSkillID ));

	// Sum up spent SP corresponding to the job required by the skill.
	int iUsedSPByJob = 0;
	const TSkillData* pSkillDataForNeedJobID = g_pDataManager->GetSkillData( iSkillID );

#ifndef PRE_FIX_SKILLLIST
	for( int i = 0; i < (int)m_vlhSkillList.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( i );
#else
	DWORD dwNumSkill = GetSkillCount();
	for( DWORD i = 0; i < dwNumSkill; ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
#endif // #ifndef PRE_FIX_SKILLLIST
		if( hSkill->GetNeedJobClassID() == pSkillDataForNeedJobID->nNeedJobID )
		{
			int iLevel = hSkill->GetLevel();
			for( int k = 0; k < iLevel; ++k )
			{
				const TSkillData* pSkillData = g_pDataManager->GetSkillData( hSkill->GetClassID() );
				iUsedSPByJob += pSkillData->vLevelDataList.at( k ).nNeedSkillPoint;
			}
		}
	}

	// If remaining job SP exceeds total usable SP, total usable SP is authoritative, so return that amount.
	int iAvailPoint = iWholeAvailSPByJob - iUsedSPByJob;
	if( m_pSession->GetSkillPoint() < iAvailPoint )
		iAvailPoint = m_pSession->GetSkillPoint();
	
	return iAvailPoint;
}

int CDnPlayerActor::GetUsedSkillPointInThisJob( const int nJobID )
{
	int iUsedSPByJob = 0;

#ifndef PRE_FIX_SKILLLIST
	for( int i = 0; i < (int)m_vlhSkillList.size(); ++i )
	{
		DnSkillHandle hSkill = m_vlhSkillList.at( i );
#else
	DWORD dwNumSkill = GetSkillCount();
	for( DWORD i = 0; i < dwNumSkill; ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
#endif // #ifndef PRE_FIX_SKILLLIST
		if( hSkill->GetNeedJobClassID() == nJobID )
		{
			int iLevel = hSkill->GetLevel();
			for( int k = 0; k < iLevel; ++k )
			{
				const TSkillData* pSkillData = g_pDataManager->GetSkillData( hSkill->GetClassID() );
				iUsedSPByJob += pSkillData->vLevelDataList.at( k ).nNeedSkillPoint;
			}
		}
	}

	return iUsedSPByJob;
}

void CDnPlayerActor::OnChangePlaySpeed( DWORD dwFrame, float fSpeed )
{
	// When play time increases due to super armor from getting hit, etc., the signal term frame value
	// Must update together to avoid triggering anti-hack detection.
	// Changed to store the value and apply it during checks, since an action may start while frame speed is already modified.
	//for( int i = 0; i < (int)m_dqProjectileSignalOffset.size(); ++i )
	//{
	//	int& iSignalOffsetFrame = m_dqProjectileSignalOffset.at( i );
	//	iSignalOffsetFrame = (int)((float)iSignalOffsetFrame * fSpeed );
	//}

	// Case where frames momentarily freeze for a duration due to super armor
	if( dwFrame <= 200 && fSpeed < 0.05f)
		m_bCheckProjectileSignalTerm = false;

	m_fFrameSpeed = fSpeed;
}

void CDnPlayerActor::ProcessCompanion( LOCAL_TIME LocalTime, float fDelta )
{
	CheckPetSatietyPercent();
}

void CDnPlayerActor::CheckPetSatietyPercent()
{
	if( !IsCanPetMode() )
		return;

	const TVehicle* pEquipPet = GetUserSession()->GetItem()->GetPetEquip();
	if( pEquipPet && (pEquipPet->nType & Pet::Type::ePETTYPE_SATIETY) && pEquipPet->Vehicle[Pet::Slot::Body].nItemID > 0 )
	{
		if( GetUserSession()->GetItem()->GetPetSatietyPercent() < 50.f ) // Need to check Limit here.
		{
			DnSkillHandle hFirstSkill = FindSkill(pEquipPet->nSkillID1);
			if(hFirstSkill && hFirstSkill->GetSkillType() == CDnSkill::Passive )
			{
				CmdForceRemoveSkill(pEquipPet->nSkillID1);
				m_bDeletedPetSkill[Pet::Skill::Primary] = true;
			}

			DnSkillHandle hSecondSkill = FindSkill(pEquipPet->nSkillID2);
			if(hSecondSkill && hSecondSkill->GetSkillType() == CDnSkill::Passive )
			{
				CmdForceRemoveSkill(pEquipPet->nSkillID2);
				m_bDeletedPetSkill[Pet::Skill::Secondary] = true;
			}
		}
		else
		{
			if( m_bDeletedPetSkill[Pet::Skill::Primary] == true )
			{
				if( !IsExistSkill( pEquipPet->nSkillID1 ) )
					CmdForceAddSkill( pEquipPet->nSkillID1 );

				m_bDeletedPetSkill[Pet::Skill::Primary] = false;
			}

			if( m_bDeletedPetSkill[Pet::Skill::Secondary] == true )
			{
				if( !IsExistSkill( pEquipPet->nSkillID2 ) )
					CmdForceAddSkill( pEquipPet->nSkillID2 );

				m_bDeletedPetSkill[Pet::Skill::Secondary] = false;
			}
		}
	}
}

CDnVehicleActor *CDnPlayerActor::GetMyVehicleActor()
{
	if(m_hVehicleActor)
	{
		return static_cast<CDnVehicleActor*>(m_hVehicleActor.GetPointer());
	}

	return NULL;
}

bool CDnPlayerActor::IsCanVehicleMode()
{
	if( IsDie() || IsGhost() || m_nTeam == PvPCommon::Team::Observer ) 
		return false;

	if( IsSpectatorMode() ) 
		return false;

	int nCurrentMapID = 0;

	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	if( pGameTask ) // Detection when in game task 
		nCurrentMapID = pGameTask->GetMapTableID();
	else
		nCurrentMapID = m_pSession->GetMapIndex();

	bool bIsCanVehicleMode = g_pDataManager->IsVehicleMode(nCurrentMapID);

#if defined( PRE_ADD_FORCE_RIDE_ENABLE_TRIGGER )
	bIsCanVehicleMode = bIsCanVehicleMode && m_bForceEnableRideByTrigger;
#endif	// #if defined( PRE_ADD_FORCE_RIDE_ENABLE_TRIGGER )

	return bIsCanVehicleMode;
}

bool CDnPlayerActor::IsCanPetMode()
{
	int nCurrentMapID = 0;

	CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	if( pGameTask ) // Detection when in game task 
		nCurrentMapID = pGameTask->GetMapTableID();
	else
		nCurrentMapID = m_pSession->GetMapIndex();

	return g_pDataManager->IsPetMode( nCurrentMapID );
}

void CDnPlayerActor::RideVehicle(TVehicle *pInfo)
{
	if( !IsPlayerActor() || IsVehicleMode() || !pInfo )
		return;

	if( !IsCanVehicleMode())
		return;
		
	if(IsBattleMode())
		SetBattleMode(false);

	int nVehicleActorTableID = g_pDataManager->GetVehicleActorID(pInfo->Vehicle[Vehicle::Slot::Body].nItemID);

	DnActorHandle hVehicle;
	hVehicle = CreateActor( GetRoom(), nVehicleActorTableID );
	if( !hVehicle || !hVehicle->GetObjectHandle())
		return;

	CDnVehicleActor* pVehicle = (CDnVehicleActor *)hVehicle.GetPointer();
	if(!pVehicle)
		return;

	pVehicle->SetMyPlayerActor(GetActorHandle());
	pVehicle->Show( false );

	float fLandHeight = INSTANCE(CDnWorld).GetHeight( GetMatEx()->m_vPosition );

	pVehicle->SetPosition(GetMatEx()->m_vPosition);
	pVehicle->SetPrevPosition(GetMatEx()->m_vPosition);
	pVehicle->SetAddHeight( GetMatEx()->m_vPosition.y - fLandHeight );
	pVehicle->GetMatEx()->CopyRotationFromThis(GetMatEx());

	pVehicle->SetItemID(pInfo->Vehicle[Vehicle::Slot::Body].nItemID); // Holds its own item ID.

#ifdef PRE_ADD_VEHICLE_ACTION_STRING
	DNTableFileFormat* pVehicleTable = GetDNTable( CDnTableDB::TVEHICLE );

	std::string strVehicleAction = pVehicleTable->GetFieldFromLablePtr( pInfo->Vehicle[Vehicle::Slot::Body].nItemID , "_RiderString" )->GetString();
	if(!strVehicleAction.empty() || strstr(strVehicleAction.c_str() , "Vehicle_") != NULL )
		pVehicle->SetVehicleActionString(strVehicleAction.c_str());
#endif

	// Equip by part // Equip if incoming data contains parts info.
	for (int i= Vehicle::Slot::Saddle; i<Vehicle::Slot::Max; i++)
	{
		if(pInfo->Vehicle[i].nItemID != 0 && pInfo->Vehicle[i].nSerial != 0)
		{
			pVehicle->EquipItem(pInfo->Vehicle[i]);
		}
	}

	////////////////
	if(pInfo->dwPartsColor1 != 0 && pInfo->dwPartsColor1 != -1) // If a color is configured, apply the color setting.
	{
		pVehicle->ChangeHairColor(pInfo->dwPartsColor1); // Assign color defined in the default table.
	}

	SetMyVehicleActor(pVehicle->GetActorHandle());
	pVehicle->InitializeRoom((CDNGameRoom*)GetRoom());
	pVehicle->Initialize();
	pVehicle->SetUniqueID( m_pSession->GetVehicleObjectID() );
		
	pVehicle->SetVehicleClassID(pInfo->Vehicle[Vehicle::Slot::Body].nItemID);
	pVehicle->RefreshState();
	pVehicle->Show(true);
	
	pVehicle->SetAttachToPlayer(true);
	pVehicle->SetProcess(true);

	SetVehicleMode(true);
	pVehicle->SetActionQueue( "Stand" );
	pVehicle->SetTeam(GetTeam());
}
  
void CDnPlayerActor::UnRideVehicle(bool bForce)
{
	if(!IsVehicleMode())
		return;

	if(GetMyVehicleActor() && !GetMyVehicleActor()->IsDestroy())
	{
		float fLandHeight = INSTANCE(CDnWorld).GetHeight( GetMyVehicleActor()->GetMatEx()->m_vPosition );
		SetPosition(GetMyVehicleActor()->GetMatEx()->m_vPosition);
		SetPrevPosition(GetMyVehicleActor()->GetMatEx()->m_vPosition);
		SetAddHeight( GetMyVehicleActor()->GetMatEx()->m_vPosition.y - fLandHeight );

		GetMyVehicleActor()->SetAttachToPlayer(false);
		GetMyVehicleActor()->Show(false);
		GetMyVehicleActor()->SetUniqueID(GetMyVehicleActor()->GetUniqueID() +1 );  
		// If mounting a new horse between SetDestroy being set and the actor being deleted after Process, a unique ID collision occurs
		// < Case where a new unique ID gets deleted in a subsequent process > So change the unique ID before destroying.
		GetMyVehicleActor()->SetDestroy();
	}

	SetVehicleMode(false);
	Show(true);
	SetActionQueue("Stand");

	return;
}

void CDnPlayerActor::ForceUnRideVehicle()
{
	if(!IsVehicleMode())
		return;

	CDNGameRoom* pRoom = GetGameRoom();
	if (!pRoom)
		return;

	api_trigger_UnRideVehicle( pRoom, GetSessionID() );
}

void CDnPlayerActor::RemoveVehicleStateEffectImmediately(int nBlowIndex )
{
	if(IsVehicleMode() && GetMyVehicleActor())
	{
		GetMyVehicleActor()->CmdRemoveStateEffectImmediately( (STATE_BLOW::emBLOW_INDEX)nBlowIndex );
		GetMyVehicleActor()->SendRemoveStateEffect( (STATE_BLOW::emBLOW_INDEX)nBlowIndex );
	}
}

void CDnPlayerActor::SetForceEnableRide( const bool bForceEnableRide )
{
	if( false == bForceEnableRide )
		ForceUnRideVehicle();

	m_bForceEnableRideByTrigger = bForceEnableRide;

	m_pSession->SendTriggerForceEnableRide( GetSessionID(), bForceEnableRide );
}

void CDnPlayerActor::OnCannonMonsterDie( void )
{
	// Exit cannon mode
	EndCannonMode();
	SetActionQueue( "Stand" );
}

void CDnPlayerActor::EndCannonMode()
{
	m_bPlayerCannonMode = false;
	m_hCannonMonsterActor.Identity();
	
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if( pSox && pSox->IsExistItem(GetClassID()))
	{
		float fWeight = pSox->GetFieldFromLablePtr( GetClassID(), "_Weight" )->GetFloat();
		int fPressLevel = pSox->GetFieldFromLablePtr( GetClassID(), "_PressLevel" )->GetInteger();

		SetWeight(fWeight);
		SetPressLevel(fPressLevel);
	}
}

void CDnPlayerActor::ProcessNonLocalShootModeAction()
{
	if(!IsLocalActor())
		return;

	if( strcmp( GetCurrentAction(), "MOD_Stand" ) == NULL ) {
		if( strstr( m_szCustomAction.c_str(), "MOD_Shoot" ) ) {
			float fFrame = ( ( CDnActionBase::m_LocalTime - m_CustomActionTime ) / 1000.f ) * CDnActionBase::m_fFPS;
			CmdStop( "MOD_Shoot_Stand", 0, 8.f, fFrame );
			ResetCustomAction();
		}
	}
}

const char *CDnPlayerActor::GetChangeShootActionName(const char *szActionName) // Work in progress.
{
	if(strcmp(szActionName,"Stand") == NULL)
	{
		szActionName = "MOD_Stand";
	}
	if(strcmp(szActionName,"Move_Front") == NULL)
	{
		szActionName = "MOD_Move_Front";
	}
	if(strcmp(szActionName,"Move_Back") == NULL)
	{
		szActionName = "MOD_Move_Back";
	}
	if(strcmp(szActionName,"Move_Left") == NULL)
	{
		szActionName = "MOD_Move_Left";
	}
	if(strcmp(szActionName,"Move_Right") == NULL)
	{
		szActionName = "MOD_Move_Right";
	}
	if(strcmp(szActionName,"Jump") == NULL)
	{
		szActionName = "MOD_Jump";
	}
	if(strstr(szActionName,"Attack1"))
	{
		szActionName = "MOD_Shoot_Stand";
	}

	return szActionName;
}

bool CDnPlayerActor::IsTransformSkill( int nSkillID )
{
	for( DWORD i=0; i<m_vecTransformSkillList.size(); i++ )
	{
		if( m_vecTransformSkillList[i] == nSkillID )
			return true;
	}

	return false;
}

void CDnPlayerActor::RefreshTransformMode()
{
	m_bRefreshTransformMode = false;

	DNTableFileFormat* pMonsterSox = GetDNTable( CDnTableDB::TMONSTER_TRANS );
	DNTableFileFormat* pMonsterSkillSox = GetDNTable( CDnTableDB::TMONSTERSKILL_TRANS );
	if(!pMonsterSox || !pMonsterSkillSox) return;

	if( IsDestroy() ) return;

	if( IsProcessSkill() == true )
	{
		bool bCancelSkill = true;

		if(GetStateBlow()->IsApplied(STATE_BLOW::BLOW_232))
		{
			DNVector(DnBlowHandle) vlBlows;
			GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_232, vlBlows);

			for (DWORD i = 0; i < vlBlows.size(); i++)
			{
				if( vlBlows[i] )
				{
					CDnTransformBlow *pTransformBlow = static_cast<CDnTransformBlow*>( vlBlows[i].GetPointer() );
					if( pTransformBlow && pTransformBlow->GetParentSkillInfo()->iSkillID == m_hProcessSkill->GetClassID() )
					{
						bCancelSkill = false;
						break;
					}
				}
			}
		}

		if( bCancelSkill == true )
		{
			CancelUsingSkill();
			SetAction("Stand",0.f,0.f);
		}
	}
	else
	{
		SetAction("Stand",0.f,0.f);
	}

	if(IsTransformMode() == true)
	{
		int nActorIndex = pMonsterSox->IsExistItem(m_nMonsterMutationTableID) ? pMonsterSox->GetFieldFromLablePtr( m_nMonsterMutationTableID , "_ActorTableID" )->GetInteger() : 0;
		int nSkillTableIndex = pMonsterSox->IsExistItem(m_nMonsterMutationTableID) ? pMonsterSox->GetFieldFromLablePtr( m_nMonsterMutationTableID , "_SkillTable" )->GetInteger() : 0;

		if(GetStateBlow()->IsApplied(STATE_BLOW::BLOW_176))
		{
			CmdRemoveStateEffect(STATE_BLOW::BLOW_176);
			GetStateBlow()->Process( 0, 0.f );
		}
	
		DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
		if(!pActorSox || !pActorSox->IsExistItem(nActorIndex))
			return;

		if(nActorIndex > 0)
		{
			SwapSingleSkin( nActorIndex );
			ResetCustomAction();
			SetAction( "Stand", 0.f, 0.f );
		}
		else
			return;

		if( pMonsterSkillSox && pMonsterSkillSox->IsExistItem(nSkillTableIndex))
		{
			if(!m_vecTransformSkillList.empty()) // Remove already applied skill if present. < Case of transforming again while transformed >
			{
				for (int i=0; i<(int)m_vecTransformSkillList.size(); i++)
				{
					if(m_vecTransformSkillList[i] != -1 )
						RemoveSkill(m_vecTransformSkillList[i]);
				}

				m_vecTransformSkillList.clear();
			}

			for (int i=0; i<PvPCommon::Common::MonsterMutationSkillColCount; i++)
			{
				char szStr[256];
				int nSkillIndex = -1;
				int nSkillLevel = -1;
				sprintf_s( szStr, "_SkillIndex%d", i+1 );
				nSkillIndex = pMonsterSkillSox->GetFieldFromLablePtr( nSkillTableIndex, szStr )->GetInteger();
				sprintf_s( szStr, "_SkillLevel%d", i+1 );
				nSkillLevel = pMonsterSkillSox->GetFieldFromLablePtr( nSkillTableIndex, szStr )->GetInteger();
				if(nSkillIndex != -1 && nSkillLevel != -1)
				{
					m_vecTransformSkillList.push_back(nSkillIndex);
					AddSkill(nSkillIndex,nSkillLevel);
				}
				else
					break;
			}
			std::string strSkillVec = "";
			for(DWORD i=0; i<m_vecTransformSkillList.size();i++)
			{
				char szSkillIndex[256];
				sprintf(szSkillIndex, "%d;", m_vecTransformSkillList[i]);
				strSkillVec += szSkillIndex;
			}

			m_nAllowedSkill = CmdAddStateEffect( NULL, STATE_BLOW::BLOW_176, -1, strSkillVec.c_str(), true ); // Disable all skills except the added monster skills.
		}

	}
	else if(IsTransformMode() == false)
	{
		SwapSingleSkin( -1 );
		SetActionQueue( IsDie() ? "Die" : m_strTransformEndAction.c_str() );

		for(UINT i=0; i<m_vecTransformSkillList.size(); i++)
		{
			if(m_vecTransformSkillList[i] != -1)
				RemoveSkill(m_vecTransformSkillList[i]);
		}
	
		if(m_nAllowedSkill > 0)
		{
			CmdRemoveStateEffectFromID(m_nAllowedSkill);
			m_nAllowedSkill = 0;
		}

		m_vecTransformSkillList.clear();
	}

	SetBattleMode(true);
	RefreshState();
}

void CDnPlayerActor::ToggleTransformMode( bool bTrue,int nMonsterMutatorTableID , bool bForce, const char* strEndAction )
{
	if( m_bTransformMode == bTrue && !bForce )
		return;

	m_bTransformMode = bTrue;
	m_nMonsterMutationTableID = nMonsterMutatorTableID;
	m_bRefreshTransformMode = true;
	m_strTransformEndAction = strEndAction;
}

void CDnPlayerActor::CmdShootMode(bool bTrue)
{
	m_bShootMode = bTrue;

	if(m_bShootMode)
		CmdStop("MOD_Shoot_Stand");
	else
		CmdStop("Stand");

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwUniqueID = GetUniqueID();
	Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
	Stream.Write( &m_bShootMode, sizeof(bool) );

	Send( eActor::SC_CMDSHOOTMODE, &Stream );
}

void CDnPlayerActor::CmdWarp( EtVector3 &vPos, EtVector2 &vLook, CDNUserSession* pGameSession, bool bCheckPlayerFollowSummonedMonster/*=false*/ )
{
	if(IsVehicleMode() && GetMyVehicleActor() )
		GetMyVehicleActor()->CmdWarp(vPos, vLook, pGameSession);

	CDnActor::CmdWarp( vPos, vLook, pGameSession, bCheckPlayerFollowSummonedMonster );
	if( m_pPlayerSpeedHackChecker ) m_pPlayerSpeedHackChecker->ResetInvalid();

	// #32426 Summon control feature - summons that need to follow when moving stage or zone (CmdWarp)
	// Check and process monster objects summoned by this player.
	if( bCheckPlayerFollowSummonedMonster )
	{
		list<S_FOLLOWSTAGE_SUMMONED_MONSTER_INFO>::iterator iter = m_listSummonedMonstersFollowStageInfos.begin();
		for( iter; iter != m_listSummonedMonstersFollowStageInfos.end(); )
		{
			DnMonsterActorHandle hMonsterActor = iter->hMonster;
			if( hMonsterActor && false == hMonsterActor->IsDie() )
			{
				hMonsterActor->CmdWarp( vPos, vLook, pGameSession, false );
				hMonsterActor->ResetAggro();
				hMonsterActor->CmdAction( "Stand" );
				++iter;
			}
			else
			{
				iter = m_listSummonedMonstersFollowStageInfos.erase( iter );
			}
		}
	}
}

void CDnPlayerActor::RequestCooltimeParrySuccess( int iSkillID )
{	
	BYTE pBuffer[ 32 ] = { 0 };
	CPacketCompressStream Stream( pBuffer, 32 );
	
	Stream.Write( &iSkillID, sizeof(int) );

	Send( eActor::CS_COOLTIMEPARRY_SUCCESS, &Stream );
}

bool CDnPlayerActor::IsInvalidPlayerChecker()
{
	if( IsAIBotActor() ) return false;
	if( !m_pSession ) return false;
	if( m_pSession->GetHackPlayRestraintValue() <= 0 ) return false;
	if( m_pSession->GetHackAbuseDBValue() + m_nInvalidPlayerCheckCounter >= m_pSession->GetHackPlayRestraintValue() ) return true;
	return false;
}

void CDnPlayerActor::SwapSingleSkin( int nChangeActorTableID )
{
	if( m_nSwapSingleSkinActorID == nChangeActorTableID ) return;

	m_nSwapSingleSkinActorID	= nChangeActorTableID;
//	FreeAction();
#ifdef PRE_FIX_MEMOPT_EXT
	if (g_pDataManager == NULL)
	{
		_ASSERT(0);
		return;
	}
#endif

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TACTOR );
	if( m_nSwapSingleSkinActorID == -1 ) {
#ifdef PRE_FIX_MEMOPT_EXT
		std::string szSkinName, szAniName, szActName;
		g_pDataManager->GetFileNameFromFileEXT(szSkinName, pSox, m_nClassID, "_SkinName");
		g_pDataManager->GetFileNameFromFileEXT(szAniName, pSox, m_nClassID, "_AniName");
		g_pDataManager->GetFileNameFromFileEXT(szActName, pSox, m_nClassID, "_ActName");
#else
		std::string szSkinName = pSox->GetFieldFromLablePtr( m_nClassID, "_SkinName" )->GetString();
		std::string szAniName = pSox->GetFieldFromLablePtr( m_nClassID, "_AniName" )->GetString();
		std::string szActName = pSox->GetFieldFromLablePtr( m_nClassID, "_ActName" )->GetString();
#endif

		SAFE_RELEASE_SPTR( GetObjectHandle() );
		FreeAction();

		LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
		LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );
		SetAction( "Stand", 0.f, 0.f );

		for( int i=0; i<CDnWeapon::EquipSlot_Amount; i++ ) 
		{
			DnWeaponHandle hWeapon = GetWeapon(i);
			if( hWeapon ) 
			{
				LinkWeapon( i );
			}

			hWeapon = GetCashWeapon(i);
			if( hWeapon ) 
			{
				LinkCashWeapon( i );
			}
		}

		SAFE_RELEASE_SPTR( m_hSwapOriginalHandle );
		SAFE_DELETE( m_pSwapOriginalAction );

	}
	else 
	{
#ifdef PRE_FIX_MEMOPT_EXT
		std::string szSkinName, szAniName, szActName;
		g_pDataManager->GetFileNameFromFileEXT(szSkinName, pSox, m_nSwapSingleSkinActorID, "_SkinName");
		g_pDataManager->GetFileNameFromFileEXT(szAniName, pSox, m_nSwapSingleSkinActorID, "_AniName");
		g_pDataManager->GetFileNameFromFileEXT(szActName, pSox, m_nSwapSingleSkinActorID, "_ActName");
#else
		std::string szSkinName = pSox->GetFieldFromLablePtr( m_nSwapSingleSkinActorID, "_SkinName" )->GetString();
		std::string szAniName = pSox->GetFieldFromLablePtr( m_nSwapSingleSkinActorID, "_AniName" )->GetString();
		std::string szActName = pSox->GetFieldFromLablePtr( m_nSwapSingleSkinActorID, "_ActName" )->GetString();
#endif

		if( !m_hSwapOriginalHandle && m_hObject ) 
		{
			m_hSwapOriginalHandle = EternityEngine::CreateAniObject( GetRoom(), CEtResourceMng::GetInstance().GetFullName( m_hObject->GetSkinFileName() ).c_str(), CEtResourceMng::GetInstance().GetFullName( m_hObject->GetAniHandle()->GetFileName() ).c_str() );
		}
		SAFE_RELEASE_SPTR( GetObjectHandle() );
		if( !m_pSwapOriginalAction ) 
		{
			m_pSwapOriginalAction = new CDnActionBase;
			m_pSwapOriginalAction->LoadAction( CEtResourceMng::GetInstance().GetFullName( CDnActionBase::m_szFileName ).c_str() );
		}

		SAFE_RELEASE_SPTR( GetObjectHandle() );
		FreeAction();

		LoadSkin( CEtResourceMng::GetInstance().GetFullName( szSkinName ).c_str(), CEtResourceMng::GetInstance().GetFullName( szAniName ).c_str() );
		LoadAction( CEtResourceMng::GetInstance().GetFullName( szActName ).c_str() );

		SetAction( "Stand", 0.f, 0.f );
	}

	if( m_hObject ) 
	{
		m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
		m_hObject->SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) | COLLISION_GROUP_DYNAMIC( 2 ) | COLLISION_GROUP_DYNAMIC( 3 ) );
	}

	if(m_pBubbleSystem)
		m_pBubbleSystem->Clear();
}

void CDnPlayerActor::InitializeEnchantPassiveSkills( void )
{
	// Find skills of the EnchantPassive type and apply their values to the base skill.
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i = 0; i < m_vlhSkillList.size(); i++ ) 
	{
		DnSkillHandle hSkill = m_vlhSkillList[ i ];
#else
	for( DWORD i = 0; i < GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
#endif // #ifndef PRE_FIX_SKILLLIST
		if( CDnSkill::EnchantPassive == hSkill->GetSkillType() &&
			0 < hSkill->GetBaseSkillID() )
		{
			DnSkillHandle hEnchantPassiveSkill = hSkill;
			int iBaseSkillID = hEnchantPassiveSkill->GetBaseSkillID();
			DnSkillHandle hBaseSkill = FindSkill( iBaseSkillID );

			// Added temporary defensive code. Must be removed once the skill reset cash item is properly added.
			if( hBaseSkill )
				hBaseSkill->ApplyEnchantSkill( hEnchantPassiveSkill );
		}
	}
}

void CDnPlayerActor::OnReplacementSkill( DnSkillHandle hLegacySkill, DnSkillHandle hNewSkill )
{
	MASkillUser::OnReplacementSkill( hLegacySkill, hNewSkill );

	// Case where local player's passive enhancement skill levels up and reaches here via previous-level skill object replacement routine.
	// Reset the enhancement state of the currently applied base skill and apply that of the newly leveled-up enhanced skill.
	if( CDnSkill::EnchantPassive == hLegacySkill->GetSkillType() &&
		0 < hLegacySkill->GetBaseSkillID() )
	{
		DnSkillHandle hBaseSkill = FindSkill( hLegacySkill->GetBaseSkillID() );
		if( hBaseSkill )
		{
			_ASSERT( hLegacySkill->GetBaseSkillID() == hNewSkill->GetBaseSkillID() );
			hBaseSkill->ReleaseEnchantSkill();
			hBaseSkill->ApplyEnchantSkill( hNewSkill );
		}
	}
	else
	{
		// Case where local player's base skill targeted by passive enhancement skill levels up and reaches here via replacement routine.
		// In this case, leave the base skill object as SkillTask will delete it after this routine finishes 
		// Apply the enhancement skill to the newly leveled-up base skill.
		CheckAndApplyEnchantPassiveSkill( hNewSkill );
	}
}

void CDnPlayerActor::CheckAndApplyEnchantPassiveSkill( DnSkillHandle hBaseSkill )
{
	if( hBaseSkill )
	{
		DnSkillHandle hEnchantPassiveSkill;
#ifndef PRE_FIX_SKILLLIST
		for( DWORD i = 0; i < m_vlhSkillList.size(); i++ ) 
		{
			DnSkillHandle hSkill = m_vlhSkillList[ i ];
#else
		for( DWORD i = 0; i < GetSkillCount(); ++i )
		{
			DnSkillHandle hSkill = GetSkillFromIndex( i );
#endif // #ifndef PRE_FIX_SKILLLIST
			if( hSkill->GetBaseSkillID() == hBaseSkill->GetClassID() ) 
			{
				hEnchantPassiveSkill = hSkill;
				break;
			}
		}

		if( hEnchantPassiveSkill )
			hBaseSkill->ApplyEnchantSkill( hEnchantPassiveSkill );
	}
}

// Currently only Elemental Lord's Icy Fraction exceptionally filters 2 state effects
// Want only one of the two to be hit, so prevent the first checked one from being caught in the next filtering. #28747
void CDnPlayerActor::OnHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex )
{
	if( STATE_BLOW::BLOW_041 == (STATE_BLOW::emBLOW_INDEX)iBlowIndex || 
		STATE_BLOW::BLOW_144 == (STATE_BLOW::emBLOW_INDEX)iBlowIndex )
	{
		m_mapIcyFractionHitted[ dwTargetActorUniqueID ] = true;
	}
}

bool CDnPlayerActor::CheckHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex )
{
	bool bResult = true;

	if( STATE_BLOW::BLOW_041 == (STATE_BLOW::emBLOW_INDEX)iBlowIndex || 
		STATE_BLOW::BLOW_144 == (STATE_BLOW::emBLOW_INDEX)iBlowIndex )
	{
		if( m_mapIcyFractionHitted.end() != m_mapIcyFractionHitted.find( dwTargetActorUniqueID ) )
		{
			bResult = false;
		}
	}

	return bResult;
}

bool CDnPlayerActor::IsMyRelicMonster( DnActorHandle hActor )
{
	bool bResult = false;

	if( hActor && hActor->IsMonsterActor() )
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if( pMonsterActor->IsClericRelicMonster() )
		{
			if( GetMySmartPtr() == pMonsterActor->GetSummonerPlayerActor() )
				bResult = true;
		}
	}

	return bResult;
}

INT64 CDnPlayerActor::GetCharacterDBID()
{ 
	return m_pSession ? m_pSession->GetCharacterDBID() : 0; 
}

// #26902 
// Temporarily command the client to change job to this class.
// From this point on, skill level-up and various controls are disabled.
// Also ignore packets such as skill level-up on the game server side.
// Reset immediately when moving stages.
bool CDnPlayerActor::CanChangeJob( int iJobID )
{
	// Does not actually change job on game server. 
	// Just verify whether the current job can advance to the 2nd job and send to client.
	DNTableFileFormat* pJobTable = GetDNTable( CDnTableDB::TJOB );

	// Get the tier value and root job of the current job.
	int iNowJob = m_pSession->GetUserJob();
	int iNowJobDeep = 0;
	int iNowRootJob = 0;
	for( int i = 0; i < pJobTable->GetItemCount(); ++i )
	{
		int iItemID = pJobTable->GetItemID( i );
		if( iItemID == iNowJob )
		{
			iNowJobDeep = pJobTable->GetFieldFromLablePtr( iItemID, "_JobNumber" )->GetInteger();
			iNowRootJob = pJobTable->GetFieldFromLablePtr( iItemID, "_BaseClass" )->GetInteger();
			break;
		}
	}

	int iJobIDToChange = iJobID;

	// Check if target job tier is greater than or equal.
	bool bResult = false;
	map<int, int> mapRootJob;
	for( int i = 0; i < pJobTable->GetItemCount(); ++i )
	{
		int iItemID = pJobTable->GetItemID( i );
		if( iItemID == iJobIDToChange )
		{
			int iJobRootToChange = pJobTable->GetFieldFromLablePtr( iItemID, "_BaseClass" )->GetInteger();
			if( iNowRootJob == iJobRootToChange )
			{
				int iJobDeepToChange = pJobTable->GetFieldFromLablePtr( iItemID, "_JobNumber" )->GetInteger();
				if( iNowJobDeep < iJobDeepToChange )
				{
					// Parent job must also match.
					int iParentJobID = pJobTable->GetFieldFromLablePtr( iItemID, "_ParentJob" )->GetInteger();
					if( iParentJobID == iNowJob )
					{
						bResult = true;
					}
					else
					{
						// Parent job of target job is not the current job.
						wstring wszString = FormatW(L"Cannot advance to this job from your current class.\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					}
				}
				else
				{
					// Target job is a lower tier. Cannot change.
					wstring wszString = FormatW(L"Cannot change to a job of the same or lower tier.\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				}
			}
			else
			{
				// Target job is a different class. Cannot change.
				wstring wszString = FormatW(L"Cannot change to a job belonging to another class.\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
			}
		}
	}

	if( false == bResult )
	{
		wstring wszString = FormatW(L"Invalid Job ID.\r\n");
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
	}

	return bResult;
}


void CDnPlayerActor::SendTempJobChange( int iJobID )
{
	BYTE pBuffer[ 32 ] = { 0 };
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &iJobID, sizeof(int) );

	Send( eActor::SC_DO_TEMP_JOBCHANGE, &Stream );

	m_iTempChangedJob = iJobID;
}

// Command to restore temporary job change back to original.
// From this point on, skill level-up and various controls are enabled.
void CDnPlayerActor::EndAddTempSkillAndSendRestoreTempJobChange( void )
{
	BYTE pBuffer[ 32 ] = { 0 };
	CPacketCompressStream Stream( pBuffer, 32 );
	Stream.Write( &m_iTempChangedJob, sizeof(int) );

	Send( eActor::SC_RESTORE_TEMP_JOBCHANGE, &Stream );

	m_iTempChangedJob = 0;
}


void CDnPlayerActor::AddTempSkill( int iSkillID )
{
	// If the skill being temporarily created is an enhanced passive skill 
	// Check if the base skill is owned; if not, also add the base skill as a temporary skill.
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int iNeedBaseSkillID = 0;
	if( pSkillTable->IsExistItem( iSkillID ) )
		iNeedBaseSkillID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_BaseSkillID" )->GetInteger();

	DNVector(int) vlSkillsToAdd;
	if( 0 < iNeedBaseSkillID )
	{
		// If the target skill to enhance is not owned, add it to the vector to create it as well.
		if( false == IsExistSkill( iNeedBaseSkillID ) )
			vlSkillsToAdd.push_back( iNeedBaseSkillID );
	}
	vlSkillsToAdd.push_back( iSkillID );

	for( int i = 0; i < (int)vlSkillsToAdd.size(); ++i )
	{
		int iSkillIDToAdd = vlSkillsToAdd.at( i );
		bool bSuccess = AddSkill( iSkillIDToAdd, 1, CDnSkill::PVE );
		if( bSuccess )
		{
			DnSkillHandle hSkill = FindSkill( iSkillIDToAdd );
			if( hSkill )
			{
				hSkill->AsTempSkill();

				// If the skill is currently unusable because it is a 2nd job advancement skill,
				// Update object values to match the current character level and job.
				if( GetLevel() < hSkill->GetLevelLimit() )
					hSkill->SetLevelLimit( GetLevel() );

				if( false ==  IsPassJob( hSkill->GetNeedJobClassID() ) )
					hSkill->SetNeedJobClassID( GetJobClassID() );

				// Send temporary skill addition to client.
				BYTE pBuffer[ 32 ] = { 0 };
				CPacketCompressStream Stream( pBuffer, 32 );
				Stream.Write( &iSkillID, sizeof(int) );

				Send( eActor::SC_ADD_TEMP_SKILL, &Stream );

				m_bTempSkillAdded = true;
			}
		}
	}
}

void CDnPlayerActor::RemoveAllTempSkill( void )
{
#ifndef PRE_FIX_SKILLLIST
	DNVector(DnSkillHandle)::iterator iter = m_vlhSkillList.begin();
	for( iter; iter != m_vlhSkillList.end(); )
	{
		DnSkillHandle hSkill = *iter;
		if( hSkill->IsTempSkill() )
		{
			// Since this is a temporary feature for 2nd job advancement, handle directly here instead of MASkillUser 
			// Handle skill object deletion. If there are additional requests related to this later, 
			// May need to use MASkillUser::RemoveSkill().
			int iSkillID = hSkill->GetClassID();
			OnRemoveSkill( hSkill );

			iter = m_vlhSkillList.erase( iter );

			// Send temporary skill removal to client.
			BYTE pBuffer[ 32 ] = { 0 };
			CPacketCompressStream Stream( pBuffer, 32 );
			Stream.Write( &iSkillID, sizeof(int) );

			Send( eActor::SC_REMOVE_TEMP_SKILL, &Stream );
		}
		else
			++iter;
	}
#else
	vector<int> vlTempSkills;
	for( DWORD i = 0; i < GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
		if( hSkill->IsTempSkill() )
		{
			vlTempSkills.push_back( hSkill->GetClassID() );
		}
	}

	for( int i = 0; i < (int)vlTempSkills.size(); ++i )
	{
		int iSkillID = vlTempSkills.at( i );
		RemoveSkill( iSkillID );

		// Send temporary skill removal to client.
		BYTE pBuffer[ 32 ] = { 0 };
		CPacketCompressStream Stream( pBuffer, 32 );
		Stream.Write( &iSkillID, sizeof(int) );

		Send( eActor::SC_REMOVE_TEMP_SKILL, &Stream );
	}
#endif // #ifndef PRE_FIX_SKILLLIST

	m_bTempSkillAdded = false;
}

#ifdef PRE_ADD_48714
#include "DNMailSender.h"
#endif		//#ifdef PRE_ADD_48714
void CDnPlayerActor::OnInvalidPlayerChecker( int nValue )
{
	m_nInvalidPlayerCheckCounter += nValue;
	OutputDebug( "OnInvalidPlayerChecker : %d, (%d)\n", m_nInvalidPlayerCheckCounter, nValue );

	if( m_pSession && m_pSession->GetHackPlayRestraintValue() > 0 )
	{
#if defined (PRE_ADD_ABUSE_ACCOUNT_RESTRAINT)
		//Exclude *2 from disconnect judgment when sanction is applied
		if( m_nInvalidPlayerCheckCounter >= m_pSession->GetHackPlayRestraintValue() )
		{
			//Handled only for Korea for now
			if (m_pSession->GetHackCharacterCntWithoutMe() > 0)
			{
				//Besides the current character, there is one more character and another was caught; sanction per issue 36080.
				std::wstring wszRestraintReason = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100070);
				std::wstring wszRestraintReasonForDolis = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 100071);
				m_pSession->GetDBConnection()->QueryAddRestraint(m_pSession, DBDNWorldDef::RestraintTargetCode::Account, DBDNWorldDef::RestraintTypeCode::ConnectRestraint, wszRestraintReason.c_str(), wszRestraintReasonForDolis.c_str(), 9999, DBDNWorldDef::RestraintDolisReasonCode::AbuseRestraintCode);
			}
			m_pSession->DetachConnection( L"InvalidPlayerCheckCounter" );
		}
#else
#ifdef PRE_ADD_48714
		if(m_nInvalidPlayerCheckCounter >= m_pSession->GetHackPlayRestraintValue())
		{
			if (m_pSession->GetDBConnection())
			{
				CDNMailSender::Process(m_pSession, AbuseLog::Common::AbuseLog_Reached_MailID);
			}
		}
#endif		//#ifdef PRE_ADD_48714

		if( m_nInvalidPlayerCheckCounter >= m_pSession->GetHackPlayRestraintValue()*2 )
		{
			m_pSession->DetachConnection( L"InvalidPlayerCheckCounter" );
		}
#endif		//#if defined (PRE_ADD_ABUSE_ACCOUNT_RESTRAINT)
	}
}


void CDnPlayerActor::OrderUseSkillToMySummonedMonster( OrderMySummonedMonsterStruct* pStruct )
{
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_215 ) )
	{
		DNVector( DnBlowHandle ) vlhBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_215, vlhBlows );
		_ASSERT( 1 == (int)vlhBlows.size() );
		if( false == vlhBlows.empty() )
		{
			// Check if there is a monster summoned by me within the defined radius.
			float fRangeSQ = pStruct->fOrderRange * pStruct->fOrderRange;

			DNVector( DnMonsterActorHandle ) vlhMonsters;
			for( list<DnMonsterActorHandle>::const_iterator iter = m_listSummonMonster.begin();
				iter != m_listSummonMonster.end(); ++iter )
			{
				DnMonsterActorHandle hMonster = *iter;
				if( hMonster && false == hMonster->IsDie() )
				{
					float fDistanceWithThisMonsterSQ = EtVec3LengthSq( &EtVector3(*GetPosition() - *hMonster->GetPosition()) );
					if( fDistanceWithThisMonsterSQ < fRangeSQ )
					{
						vlhMonsters.push_back( hMonster );
					}
				}
			}

			map<int, list<DnMonsterActorHandle> >::iterator iterMap = m_mapSummonMonsterByGroup.begin();
			for( iterMap; iterMap != m_mapSummonMonsterByGroup.end(); ++iterMap )
			{
				const list<DnMonsterActorHandle>& listSummonMonster = iterMap->second;
				for( list<DnMonsterActorHandle>::const_iterator iterList = listSummonMonster.begin();
					iterList != listSummonMonster.end(); ++iterList )
				{
					DnMonsterActorHandle hMonster = *iterList;
					if( hMonster && false == hMonster->IsDie() )
					{
						float fDistanceWithThisMonsterSQ = EtVec3LengthSq( &EtVector3(*GetPosition() - *hMonster->GetPosition()) );
						if( fDistanceWithThisMonsterSQ < fRangeSQ )
						{
							vlhMonsters.push_back( hMonster );
						}
					}
				}
			}

			if( false == vlhMonsters.empty() )
			{
				CDnOrderMySummonedMonsterBlow* pBlow = static_cast<CDnOrderMySummonedMonsterBlow*>( vlhBlows.front().GetPointer() );
				int iSkillID = pBlow->GetSkillID();

				for( int i = 0; i < (int)vlhMonsters.size(); ++i )
				{
					DnMonsterActorHandle hMonster = vlhMonsters.at( i );
					if( hMonster->GetMonsterClassID() == pStruct->nMonsterID )
					{
						// TODO: Need to ask AI side and reserve skill usage.
						if( hMonster->IsExistSkill( iSkillID ) )
						{
							MAAiScript* pScript = static_cast<MAAiScript*>(hMonster->GetAIBase());

							if( hMonster->GetAggroTarget() )
								pScript->GetMonsterSkillAI()->AddWaitOrderCount( hMonster, iSkillID );
						}
					}
				}
			}
		}
	}
}

bool CDnPlayerActor::CheckSkipAirCondition(int iSkill)
{
	if(IsAir())
	{
		bool bHaveGroundCheck = false;
		DnSkillHandle hSkill = FindSkill(iSkill);

		if(hSkill)
		{
			if(hSkill->IsExistUsableChecker(IDnSkillUsableChecker::GROUNDMOVABLE_CHECKER))
				bHaveGroundCheck = true;
		}

		if(bHaveGroundCheck)
		{
			return true;
		}
	}

	return false;
}


bool CDnPlayerActor::HasSameGlobalIDSkill( int iSkillID )
{
	bool bResult = false;
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	int iGlobalSkillGroupID = pSkillTable->GetFieldFromLablePtr( iSkillID, "_GlobalSkillGroup" )->GetInteger();

	if( 0 < iGlobalSkillGroupID )
	{ 
		for( DWORD i = 0; i < GetSkillCount(); ++i )
		{
		 	DnSkillHandle hExistSkill = GetSkillFromIndex( i );
			if( hExistSkill &&
				hExistSkill->GetGlobalSkillGroupID() == iGlobalSkillGroupID )
			{
				bResult = true;
				break;
			}
		}
	}

	return bResult;
}

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
bool CDnPlayerActor::IsAllowCallSkillProcess( float fDelta )
{
	return m_FrameSkipCallSkillProcess.Update( fDelta );
}
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

void CDnPlayerActor::RemoveAllBubbles( bool bRemoveEvent )
{
	if( m_pBubbleSystem )
		m_pBubbleSystem->RemoveAllBubbles( bRemoveEvent );
}

void CDnPlayerActor::ApplyEnchantSkillOnceFromBubble( int iTargetSkillID, int iEnchantSkillID )
{
	DnSkillHandle hEnchantSkill;
	map<int, DnSkillHandle>::iterator iter = m_mapEnchantSkillFromBubble.find( iEnchantSkillID );
	if( m_mapEnchantSkillFromBubble.end() != iter )
	{
		hEnchantSkill = iter->second;
	}
	else
	{
		// Both PvP and PvE data must be present to create properly.
		hEnchantSkill = CDnSkill::CreateSkill( GetMySmartPtr(), iEnchantSkillID, 1 );
		m_mapEnchantSkillFromBubble.insert( make_pair(iEnchantSkillID, hEnchantSkill) );
	}

	DnSkillHandle hTargetSkill = FindSkill( iTargetSkillID );
	_ASSERT( hTargetSkill );
	_ASSERT( hEnchantSkill );

	if( hTargetSkill && hEnchantSkill )
	{
		// An already enhanced skill cannot be targeted.
		bool bIsEnchantedAlready = hTargetSkill->IsEnchantedSkill();
		_ASSERT( false == bIsEnchantedAlready );
		if( false == bIsEnchantedAlready )
		{
			hTargetSkill->ApplyEnchantSkillOnceFromBubble( hEnchantSkill );
		}
	}
}

bool CDnPlayerActor::OnApplySpectator(bool bEnable)
{
	CDNGameRoom* pGameRoom = GetGameRoom();
	if( pGameRoom == NULL || pGameRoom->bIsPvPRoom() == false || pGameRoom->GetPvPGameMode()->bIsAllKillMode() == false )
		return false;

	if( bEnable )
	{
		if( IsProcessSkill() )
			CancelUsingSkill();
	
		if(IsBattleMode())
			SetBattleMode(false);
	}

	return true;
}

bool CDnPlayerActor::IsSpectatorMode()
{
	return IsAppliedThisStateBlow(STATE_BLOW::BLOW_230);
}

void CDnPlayerActor::ChangeSkillLevelUp(int nSkillID, int nOrigLevel)
{
	if( IsProcessSkill() )
		return;

	DnSkillHandle hSkill = FindSkill(nSkillID);
	if( hSkill )
	{
		if (hSkill->GetElapsedDelayTime() > 0.0f || hSkill->IsToggleOn())
			return;
	}
	__super::ChangeSkillLevelUp(nSkillID, nOrigLevel);

	DnSkillHandle hChangedSkill = FindSkill( nSkillID );
	if( hChangedSkill )
	{
		ReplacementGlyph( hChangedSkill );
	}
}

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
void CDnPlayerActor::UpdateTotalLevel(int nLevel)
{
	if (m_pTotalLevelSkillSystem)
	{
		m_pTotalLevelSkillSystem->SetTotalLevel(nLevel);
	}
}

void CDnPlayerActor::UpdateTotalLevelByCharLevel()
{
	if (m_pTotalLevelSkillSystem)
		m_pTotalLevelSkillSystem->UpdateTotalLevel();
}

void CDnPlayerActor::AddTotalLevelSkill(int nSlotIndex, int nSkillID, bool isInitialize/* = false*/)
{
	if (m_pTotalLevelSkillSystem)
	{
		DnSkillHandle hSkill = m_pTotalLevelSkillSystem->FindTotalLevelSkill(nSkillID);
		if (!hSkill)
		{
			RemoveTotalLevelSkill(nSlotIndex);
			return;
		}

		//Do not add if the slot is not activated..
		bool isActivateSlot = m_pTotalLevelSkillSystem->IsActivateSlot(nSlotIndex);
		if (isActivateSlot == false)
			return;

		//PVE/PVP settings..
		// Set skill level data separately for PvE and PvP.
		int iSkillLevelDataType = CDnSkill::PVE;
		if( GetGameRoom()->bIsPvPRoom() )
			iSkillLevelDataType = CDnSkill::PVP;

		hSkill->SelectLevelDataType( iSkillLevelDataType );


		m_pTotalLevelSkillSystem->AddTotalLevelSkill(nSlotIndex, hSkill, isInitialize);
	}
}
void CDnPlayerActor::RemoveTotalLevelSkill(int nSlotIndex)
{
	if (m_pTotalLevelSkillSystem)
	{
		m_pTotalLevelSkillSystem->RemoveTotallevelSkill(nSlotIndex);
	}
}

void CDnPlayerActor::ActivateTotalLevelSkillSlot(int nSlotIndex, bool bActivate)
{
	if (m_pTotalLevelSkillSystem)
		m_pTotalLevelSkillSystem->ActivateTotalLevelSkillSlot(nSlotIndex, bActivate);
}

void CDnPlayerActor::ActivateTotalLevelSkillCashSlot(int nSlotIndex, bool bActivate, __time64_t tExpireDate)
{
	if (m_pTotalLevelSkillSystem)
		m_pTotalLevelSkillSystem->ActivateTotalLevelSkillCashSlot(nSlotIndex, bActivate, tExpireDate);
}

void CDnPlayerActor::OnLevelChange()
{
	UpdateTotalLevelByCharLevel();
}

#endif // PRE_ADD_TOTAL_LEVEL_SKILL

void CDnPlayerActor::ApplyEventStateBlow()
{
	int nAddStateEffectIndex = -1;
	DnBlowHandle hBlow;

#ifdef PRE_ADD_WEEKLYEVENT
	if (CDnWorld::GetInstance(GetRoom()).GetMapSubType() != EWorldEnum::MapSubTypeNest && !GetGameRoom()->bIsPvPRoom() )
	{
		bool bRfreshHP = false;
		float fCurrentHpRatio = (float)GetHP() / (float)GetMaxHP();

		int nThreadID = GetGameRoom()->GetServerID();

		float fEventHp = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, GetClassID(), WeeklyEvent::Event_1, nThreadID);
		if (fEventHp != 0.f)
		{
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventHp));

			nAddStateEffectIndex = CmdAddStateEffect(NULL, STATE_BLOW::BLOW_058, -1, strValue.c_str(), false, true , true );
			m_vecEventEffectList.push_back( nAddStateEffectIndex );
			bRfreshHP = true;
		}
		
		float fEventAttack = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, GetClassID(), WeeklyEvent::Event_2, nThreadID);
		if (fEventAttack != 0.f)
		{
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventAttack));

			nAddStateEffectIndex = CmdAddStateEffect(NULL, STATE_BLOW::BLOW_002, -1, strValue.c_str(), false, true , true );
			m_vecEventEffectList.push_back( nAddStateEffectIndex );

			nAddStateEffectIndex = CmdAddStateEffect(NULL, STATE_BLOW::BLOW_029, -1, strValue.c_str(), false, true , true );
			m_vecEventEffectList.push_back( nAddStateEffectIndex );
		}

		float fEventDefense = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, GetClassID(), WeeklyEvent::Event_3, nThreadID);
		if (fEventDefense != 0.f)
		{
			std::string strValue;
			strValue.append(boost::lexical_cast<std::string>(fEventDefense));

			nAddStateEffectIndex = CmdAddStateEffect(NULL, STATE_BLOW::BLOW_004, -1, strValue.c_str(), false, true , true );
			m_vecEventEffectList.push_back( nAddStateEffectIndex );

			nAddStateEffectIndex = CmdAddStateEffect(NULL, STATE_BLOW::BLOW_094, -1, strValue.c_str(), false, true , true );
			m_vecEventEffectList.push_back( nAddStateEffectIndex );
		}
	
		if( bRfreshHP )
		{
			GetStateBlow()->Process( 0 , 0 );
			CmdRefreshHPSP( (INT64)(GetMaxHP() * fCurrentHpRatio) , GetSP() );
		}
	}
#endif

}

void CDnPlayerActor::RemoveEventStateBlow()
{
	for( DWORD index = 0; index < m_vecEventEffectList.size() ; ++index )
	{
		CmdRemoveStateEffectFromID( m_vecEventEffectList[index] );
	}
	m_vecEventEffectList.clear();
}

bool CDnPlayerActor::CheckSkillAction( const char *szActionName )
{
	if( strstr( szActionName, "Skill_" ) 
		|| strstr( szActionName, "ChargeShoot_" ) 
		|| strstr( szActionName, "GuildSkill_" ) 
		|| strstr( szActionName, "Throw_" ) 
		|| strstr( szActionName, "Tumble_" )
		|| strstr( szActionName, "AerialEvasion" ) )
	{
		return true;
	}

	return false;
}

void CDnPlayerActor::MakeEquipAndPassiveState( CDnState &State )
{
	State.ResetState();
	State = MakeEquipState();
	
	CDnState BUFF_STATE;
	BUFF_STATE.ResetState();

	if( m_pStateBlow )
	{
		int nSize = m_pStateBlow->GetNumStateBlow();
		for ( int i = 0 ; i < nSize ; i++ )
		{
			DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);
			if( hBlow && 
				CDnCreateBlow::IsBasicBlow( hBlow->GetBlowIndex() ) == true &&
				hBlow->GetParentSkillInfo() && 
				hBlow->GetParentSkillInfo()->eSkillType == CDnSkill::SkillTypeEnum::Passive && 
				hBlow->GetParentSkillInfo()->eDurationType == CDnSkill::DurationTypeEnum::Buff )
			{
				CDnBasicBlow *pBlow = static_cast<CDnBasicBlow*>(hBlow.GetPointer());
				if( pBlow )
				{
					CDnState *pState = const_cast<CDnState*>(pBlow->GetState());
					BUFF_STATE.MergeState( *pState, pState->GetValueType() );
				}
			}
		}
	}
	
	State.MergeState( BUFF_STATE, ValueTypeAbsolute );
	State.CalculateRatioValue( BUFF_STATE );
}

#ifdef PRE_ADD_COSTUME_SKILL

void CDnPlayerActor::RefreshCostumeSkill( int nSkillIndex, int nSkillLevel )
{
	if( m_nCostumeSkillID == nSkillIndex )
		return;

	if( m_nCostumeSkillID > 0 ) 
		RemoveSkill( m_nCostumeSkillID );

	if( nSkillIndex > 0 ) 
	{
		AddSkill( nSkillIndex, nSkillLevel );
		DnSkillHandle hSkill = FindSkill( nSkillIndex );
		if( hSkill ) 
		{
			if( hSkill->GetSkillType() != CDnSkill::SkillTypeEnum::Active )
			{
				RemoveSkill( nSkillIndex );
				return;
			}

			hSkill->OnBeginCoolTime();
		}
	}

	m_nCostumeSkillID = nSkillIndex;
}

#endif // PRE_ADD_COSTUME_SKILL

#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
void CDnPlayerActor::ReportInvalidAction()
{
	if( m_pPlayerActionChecker )
		((CDnPlayerActionChecker*)m_pPlayerActionChecker)->OnInvalidAction();
}
#endif