#include "stdafx.h"
#include "DnWorldBrokenBuffProp.h"
#include "DnPropCondition.h"
#include "DnPropStateScanActor.h"
#include "DnPropActionCondition.h"
#include "TaskManager.h"
#include "DnGameTask.h"

CDnWorldBrokenBuffProp::CDnWorldBrokenBuffProp( CMultiRoom* pRoom ) : CDnWorldBrokenProp( pRoom ), 
																	   m_pBuffFSM( new TDnFiniteStateMachine<DnPropHandle>(GetMySmartPtr()) )
{

}

CDnWorldBrokenBuffProp::~CDnWorldBrokenBuffProp(void)
{
	SAFE_DELETE( m_pBuffFSM );
	ReleasePostCustomParam();
}

bool CDnWorldBrokenBuffProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( CDnWorldBrokenProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale ) == false ) return false;
	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

bool CDnWorldBrokenBuffProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) 
		return false;

	bool bResult = false;

	if( GetData() )
	{
		BuffBrokenStruct* pStruct = (BuffBrokenStruct*)(GetData());
		int nSkillTableID = pStruct->nSkillTableID;
		int nSkillLevel = pStruct->nSkillLevel;
		float fCheckRange = pStruct->fCheckRange;			// Check range for actors
		//m_iAffectTeam = pStruct->nTeam; // Target team to apply to.
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID );

		// Invalid skill info
#if !defined( PRE_TRIGGER_TEST )
		_ASSERT( nSkillTableID != 0 && nSkillLevel != 0 );
#endif // #if defined( PRE_TRIGGER_TEST )
		if( nSkillTableID == 0 || nSkillLevel == 0 )
			return false;

		if( bResult )
		{
			m_BuffPropComponent.Initialize( GetRoom(), pStruct->nTeam, nSkillTableID, nSkillLevel );

			m_nDurability = pStruct->nDurability;

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

			// Initialize states.
			// Do nothing in Idle action; when Activate action runs, search nearby targets and apply state effects attached to the skill.
			CDnPropState* pNormalState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::NORMAL );
			CDnPropState* pScanState = CDnPropState::Create( GetMySmartPtr(), CDnPropState::SCAN_ACTOR );

			CDnPropCondition* pNormalToScan = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::ACTION_CHECK );
			CDnPropCondition* pScanToNormal = CDnPropCondition::Create( GetMySmartPtr(), CDnPropCondition::ACTION_CHECK );

			static_cast<CDnPropStateScanActor*>(pScanState)->SetRange( fCheckRange );

			static_cast<CDnPropActionCondition*>(pNormalToScan)->SetActionName( "Activate" );
			static_cast<CDnPropActionCondition*>(pScanToNormal)->SetActionName( "Idle" );

			m_pBuffFSM->AddCondition( pNormalToScan );
			m_pBuffFSM->AddCondition( pScanToNormal );

			m_pBuffFSM->AddState( pNormalState );
			m_pBuffFSM->AddState( pScanState );

			pNormalState->AddTransitState( pScanState, pNormalToScan );
			pScanState->AddTransitState( pNormalState, pScanToNormal );

			m_pBuffFSM->SetEntryState( pNormalState );

		}
	}

	return bResult;
}

void CDnWorldBrokenBuffProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorldBrokenProp::Process( LocalTime, fDelta );

	/*
	if( false == m_bProcessAllowed )
		return;
	*/

	m_pBuffFSM->Process( LocalTime, fDelta );
}


void CDnWorldBrokenBuffProp::OnMessage( const boost::shared_ptr<IStateMessage>& pMessage )
{
	m_BuffPropComponent.OnMessage( pMessage );
}

void CDnWorldBrokenBuffProp::OnChangeAction( const char *szPrevAction )
{
	if( (0 == strcmp( szPrevAction, "Activate" ) && 0 == strcmp( GetCurrentAction(), "Idle" )) ||
		(0 == strcmp( szPrevAction, "Off" ) && 0 == strcmp(GetCurrentAction(), "Idle")) )	// Some props transition to idle after the off action.
	{
		// Remove the currently applied effect.
		m_BuffPropComponent.RemoveAffectedStateEffects();
	}
}