#include "stdafx.h"
#include "DnWorldMultiDurabilityProp.h"
#include "DnStateBlow.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnProjectile.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnPropState.h"
#include "DnPropCondition.h"
#include "DnPropStateDoAction.h"
#include "DnPropStateTrigger.h"
#include "DnPropCondiDurability.h"
#include "DnPropActionCondition.h"
#include "DnPropCondiSubDurability.h"
#include "DnMonsterActor.h"


CDnWorldMultiDurabilityProp::CDnWorldMultiDurabilityProp( CMultiRoom* pRoom ) : CDnWorldBrokenProp( pRoom ), m_iNowStateIndex( 0 ), m_pActivateState( NULL )
{

}

CDnWorldMultiDurabilityProp::~CDnWorldMultiDurabilityProp(void)
{
	ReleasePostCustomParam();
}

bool CDnWorldMultiDurabilityProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	// Do not call default BrokenProp Initialize; configure FSM for each phase.
	if( m_hMonster )
		*m_hMonster->GetMatEx() = *GetMatEx();

	bool bResult = CDnWorldActProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale );
	if( !bResult )
		return false;

	// Test for now
	SetActionQueue( "Activate", 0, 3.0f, 0.0f );

	return true;
}


void CDnWorldMultiDurabilityProp::_InitializeFSM( void )
{
	// Initially in idle state. Considered triggered when it enters Activate state.
	CDnPropState* pIdleState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::NORMAL );
	m_pFSM->AddState( pIdleState );

	// When operation started by executing Activate action from external trigger 
	CDnPropState* pActivateState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::NORMAL );
	m_pFSM->AddState( pActivateState );
	m_pActivateState = pActivateState;

	// When main durability runs out and breaks, execute the designated trigger and perform the Broken action.
	CDnPropState* pTriggerState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::TRIGGER );
	m_pFSM->AddState( pTriggerState );
	CDnPropState* pBrokenActionState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::DO_ACTION );
	m_pFSM->AddState( pBrokenActionState );
	
	m_pBrokenActionState = static_cast<CDnPropStateDoAction*>( pBrokenActionState );
	m_pTriggerActionState = static_cast<CDnPropStateTrigger*>( pTriggerState );
	m_pTriggerActionState->SetFuncName( "CDnWorldProp::OnBrokenProp" );
	
	// Create condition
	CDnPropCondition* pToActivate = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::ACTION_CHECK );	// Start operation
	static_cast<CDnPropActionCondition*>(pToActivate)->SetActionName( "Activate" );
	m_pFSM->AddCondition( pToActivate );
	CDnPropCondition* pToIdle = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::ACTION_CHECK );		// If idle action, transition to idle state.
	static_cast<CDnPropActionCondition*>(pToIdle)->SetActionName( "Idle" );
	m_pFSM->AddCondition( pToIdle );

	// When operation started by executing Activate action from external trigger, allow transition to second state.
	pIdleState->AddTransitState( pActivateState, pToActivate );

	// Object that checks if main durability has run out; transitions to trigger active state.
	CDnPropCondition* pDurabilityCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::COMPARE_DURABILITY );
	m_pFSM->AddCondition( pDurabilityCondition );
	static_cast<CDnPropCondiDurability*>(pDurabilityCondition)->Initialize( 0, CDnPropCondiDurability::LESS_EQUAL );

	// Transition from Trigger state to state executing Broken action. NULL condition object with no conditions.
	CDnPropCondition* pNULLCondition = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::NULL_CONDITION );
	m_pFSM->AddCondition( pNULLCondition );

	// In Trigger state, immediately transition to brokenstate with no conditions.
	m_pTriggerActionState->AddTransitState( m_pBrokenActionState, pNULLCondition );

	for( int i = 0; i < (int)m_vlStateInfos.size(); ++i )
	{
		S_STATE_INFO& StateInfo = m_vlStateInfos.at( i );

		//// Durability check index is a transition condition from the previous state, so it is based on the previous state.
		//int iSubDurCheckIndex = i - 1;
		//if( iSubDurCheckIndex < 0 )
		//	iSubDurCheckIndex = (int)m_vlStateInfos.size() - 1;

		// Create state and link with conditions.
		CDnPropState* pState =  CDnPropState::Create( GetMySmartPtr(), CDnPropState::DO_ACTION );
		m_pFSM->AddState( pState );
		static_cast<CDnPropStateDoAction*>(pState)->AddActionName( StateInfo.pActionNameWhenSubDurabilityZero );

		// Set to transition to inactive state condition.
		pState->AddTransitState( pIdleState, pToIdle );

		// When main durability runs out and breaks, immediately transition to the trigger state for destruction handling.
		pState->AddTransitState( m_pTriggerActionState, pDurabilityCondition );;
		
		if( 1 == i )
		{
			// When operation started by executing Activate action from external trigger, since it is already in Activate state 
			// Allow transitioning to the second state, which is the next transition target.
			// Once the prop's state completes one cycle, it keeps rotating through the states in this vector until main durability runs out.
			// Create durability condition.
			CDnPropCondiSubDurability* pSubDurabilityCondition = 
				static_cast<CDnPropCondiSubDurability*>(CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::COMPARE_SUB_DURABILITY ));
			m_pFSM->AddCondition( pSubDurabilityCondition );
			pSubDurabilityCondition->Initialize( 0, 0, CDnPropCondiSubDurability::LESS_EQUAL );

			pActivateState->AddTransitState( pState, pSubDurabilityCondition );
		}
		
		if( 0 < i )
		{
			// Add transition from previous state to current state.
			CDnPropState* pPrevState = m_vlStateInfos.at( i-1 ).pState;

			// Create durability condition.
			CDnPropCondiSubDurability* pSubDurabilityCondition = 
				static_cast<CDnPropCondiSubDurability*>(CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::COMPARE_SUB_DURABILITY ));
			m_pFSM->AddCondition( pSubDurabilityCondition );
			pSubDurabilityCondition->Initialize( 0, i-1, CDnPropCondiSubDurability::LESS_EQUAL );

			pPrevState->AddTransitState( pState, pSubDurabilityCondition );
		}

		// Added so it can transition back to state 1 when at the very end.
		if( i == (int)m_vlStateInfos.size()-1 )
		{
			CDnPropState* pFirstState = m_vlStateInfos.front().pState;
			
			CDnPropCondiSubDurability* pSubDurabilityCondition = 
				static_cast<CDnPropCondiSubDurability*>(CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::COMPARE_SUB_DURABILITY ));
			m_pFSM->AddCondition( pSubDurabilityCondition );
			pSubDurabilityCondition->Initialize( 0, i, CDnPropCondiSubDurability::LESS_EQUAL );

			pState->AddTransitState( pFirstState, pSubDurabilityCondition );
		}

		StateInfo.pState = pState;
	}

	m_pFSM->SetEntryState( pIdleState );
}


bool CDnWorldMultiDurabilityProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable(nTableID) == false ) return false;

	bool bResult = false;

	if( GetData() )
	{
		MultiDurabilityBrokenPropStruct* pStruct = (MultiDurabilityBrokenPropStruct*)GetData();
		int nMainDurability = pStruct->nMainDurability;
		int nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
		int nMonsterTableID = pStruct->nMonsterTableID;
		int nSkillTableID = pStruct->nSkillTableID;
		int nSkillLevel = pStruct->nSkillLevel;
		bResult = InitializeMonsterActorProp( nMonsterTableID );

		if( bResult )
		{
			m_SkillComponent.Initialize( nSkillTableID, nSkillLevel );

			m_nDurability = nMainDurability;

			// Do not take action yet. Shooter prop needs its start time synced with the client..
			SetAction( "Idle", 0.0f, 0.0f );

			if( -1 != m_nDurability )
			{
				if( 0 == m_nDurability ) m_nDurability = 1;
				m_nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				if( m_nItemDropGroupTableID > 0 ) 
				{
					CDnDropItem::CalcDropItemList( GetRoom(), Dungeon::Difficulty::Max, m_nItemDropGroupTableID, m_VecDropItemList );

					if (m_VecDropItemList.empty()){
						// Prop log
						// g_pLogConnection->QueryLogStage(GetRoom()->GetRoomID(), pTask->GetMapTableID(), 0, 0, nTableID); // Stage log 090226
					}
					else {
						for( DWORD i=0; i<m_VecDropItemList.size(); i++ ) {
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
							if( CDnDropItem::PreInitializeItem( GetRoom(), m_VecDropItemList[i].nItemID, m_VecDropItemList[i].nEnchantID ) == false )
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
							if( CDnDropItem::PreInitializeItem( GetRoom(), m_VecDropItemList[i].nItemID ) == false )
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
							{
								m_VecDropItemList.erase( m_VecDropItemList.begin() + i );
								i--;
								continue;
							}

							// Prop log
							// g_pLogConnection->QueryLogStage(GetRoom()->GetRoomID(), pTask->GetMapTableID(), 0, m_VecDropItemList[i].nItemID, nTableID); // Stage log 090226
						}
					}
				}
			}

			S_STATE_INFO StateInfo;
			StateInfo.pActionNameWhenSubDurabilityZero = pStruct->szActionName_SubDurabilityOne;
			StateInfo.iOriginalDurability = StateInfo.iNowDurability = pStruct->nSubDurability_One;
			m_vlStateInfos.push_back( StateInfo );

			StateInfo.pActionNameWhenSubDurabilityZero = pStruct->szActionName_SubDurabilityTwo;
			StateInfo.iOriginalDurability = StateInfo.iNowDurability = pStruct->nSubDurability_Two;
			m_vlStateInfos.push_back( StateInfo );

			StateInfo.pActionNameWhenSubDurabilityZero = pStruct->szActionName_SubDurabilityThree;
			StateInfo.iOriginalDurability = StateInfo.iNowDurability = pStruct->nSubDurability_Three;
			m_vlStateInfos.push_back( StateInfo );

			StateInfo.pActionNameWhenSubDurabilityZero = pStruct->szActionName_SubDurabilityFour;
			StateInfo.iOriginalDurability = StateInfo.iNowDurability = pStruct->nSubDurability_Four;
			m_vlStateInfos.push_back( StateInfo );

			// Do not call default BrokenProp Initialize; configure FSM for each state.
			_InitializeFSM();
		}
	}

	return bResult;
}

int CDnWorldMultiDurabilityProp::GetSubDurability( int iIndex )
{
	int iResult = 0;

	if( iIndex < (int)m_vlStateInfos.size() )
	{
		iResult = m_vlStateInfos.at( iIndex ).iNowDurability;
	}

	return iResult;
}

void CDnWorldMultiDurabilityProp::OnFSMStateEntry( const TDnFSMState<DnPropHandle>* pState )
{
	for( int i = 0; i < (int)m_vlStateInfos.size(); ++i )
	{
		const S_STATE_INFO& StateInfo = m_vlStateInfos.at( i );
		if( StateInfo.pState == pState )
		{
			m_iNowStateIndex = i;
			break;
		}
	}
}

void CDnWorldMultiDurabilityProp::CalcDamage( CDnDamageBase *pHitter, CDnDamageBase::SHitParam &HitParam )
{
	if( m_bDestroy == true ) return;
	if( m_bBroken ) return;

	m_pLastHitObject = pHitter;
	m_LastDamageTime = HitParam.RemainTime;
	m_nLastHitUniqueID = HitParam.iUniqueID;

	// Deduct both main durability and sub durability together.
	int iDurabilityAttack = (int)( HitParam.fDurability * 100.f );
	_ASSERT( 0 <= m_iNowStateIndex && m_iNowStateIndex < (int)m_vlStateInfos.size() );
	S_STATE_INFO& StateInfo = m_vlStateInfos.at( m_iNowStateIndex );
	StateInfo.iNowDurability -= iDurabilityAttack;
	m_nDurability -= iDurabilityAttack;

	// NOTE: Trigger uses IsBroken().. Should probably check other functions too.
	// For triggers, run transition checks immediately as soon as the situation changes.
	if( m_nDurability <= 0 )
	{
		m_bBroken = true;

		//If summon monster, must find and set the master actor.
		DnActorHandle hHitterActor;

		if (pHitter)
			hHitterActor = pHitter->GetActorHandle();

		//In case of monster actor 
		if (hHitterActor && hHitterActor->IsMonsterActor())
		{
			DnActorHandle hMasterActor;
			CDnMonsterActor* pMonsterActor = NULL;

			pMonsterActor = static_cast<CDnMonsterActor*>(hHitterActor.GetPointer());

			if (pMonsterActor)
				hMasterActor = pMonsterActor->GetSummonerPlayerActor();

			//If a valid owner actor is found, change hHitterActor to the owner actor.
			if (hMasterActor)
				hHitterActor = hMasterActor;
		}

		m_pTriggerActionState->AddFuncParam( "LastBrokenPropActor", hHitterActor ? hHitterActor->GetUniqueID() : -1 );

		m_pFSM->Process( 0, 0.0f );

		UpdatePropBreakToHitter( pHitter );
	}
	else
	if( StateInfo.iNowDurability <= 0 )
	{
		// If sub-durability is depleted, advance the FSM and reset the broken sub-durability.
		m_pFSM->Process( 0, 0.0f );
		StateInfo.RestoreDurability();
	}

	if( HitParam.hWeapon ) {
		if( HitParam.hWeapon->GetWeaponType() & CDnWeapon::Projectile ) {
			CDnProjectile *pProjectile = static_cast<CDnProjectile *>(HitParam.hWeapon.GetPointer());
			if( pProjectile ) pProjectile->OnDamageSuccess( CDnActor::Identity(), HitParam );
			else HitParam.hWeapon->SetDestroy();
		}
	}
}

void CDnWorldMultiDurabilityProp::SetActionQueue( const char *szActionName, int nLoopCount , float fBlendFrame , float fStartFrame )
{
	//// If it is a per-state action, fill the sub durability.
	//for( int i = 0; i < (int)m_vlStateInfos.size(); ++i )
	//{
	//	S_STATE_INFO& StateInfo = m_vlStateInfos.at( i );

	//	if( 0 == strcmp( StateInfo.pActionNameWhenSubDurabilityZero, szActionName ) )
	//	{
	//		StateInfo.RestoreDurability();

	//		// Set current state index. Since the action name is the action to perform when sub-durability breaks
	//		// The next state after the current index becomes the current state.
	//		m_iNowStateIndex = (i+1) % (int)m_vlStateInfos.size();
	//	}
	//}

	CDnWorldActProp::SetActionQueue( szActionName, nLoopCount, fBlendFrame, fStartFrame );
}


void CDnWorldMultiDurabilityProp::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	if( m_hMonster ) 
	{
		*(m_hMonster->GetMatEx()) = m_Cross;
		m_SkillComponent.OnSignal( m_hMonster, GetMySmartPtr(), Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
	}

	CDnWorldBrokenProp::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );

	//// Remember the last hit action and restore it when returning to Normal state.
	//if( STE_Projectile == Type )
	//{
	//	const char* pCurrentAttackAction = GetCurrentAction();
	//	if( strlen( pCurrentAttackAction ) )
	//		m_strLastActionName.assign( pCurrentAttackAction );
	//}
}


void CDnWorldMultiDurabilityProp::OnSyncComplete( CDNUserSession* pBreakIntoGameSession/*=NULL*/ )
{
	if( pBreakIntoGameSession )
		CmdAction( GetCurrentAction(), pBreakIntoGameSession );
	else
		CmdAction( m_szDefaultActionName.c_str(), pBreakIntoGameSession );
}


void CDnWorldMultiDurabilityProp::OnChangeAction( const char *szPrevAction )
{
	//// Because idle repeats anyway unless it changes to another action 
	//// For idle action, send only once when changed to idle. 
	//// Fixed issue where packets kept firing when Idle action is short. (#17409)
	//const char* pCurrentAction = GetCurrentAction();
	//if( strcmp(pCurrentAction, "Idle") == 0 )
	//{
	//	if( strcmp(szPrevAction, "Idle") != 0 )
	//	{
	//		ActionSync( pCurrentAction );
	//	}
	//}
	//else
	//	ActionSync( pCurrentAction );

	////if( strcmp( szPrevAction, "Hit" ) )
	////{
	////	if( !m_strLastActionName.empty() )
	////		SetActionQueue( m_strLastActionName.c_str() );
	////}

	// Send packet only when the action changes.
	if( m_nActionIndex != m_nPrevActionIndex )
		ActionSync( GetCurrentAction() );
}