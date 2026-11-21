#include "stdafx.h"
#include "DnWorldBrokenDamageProp.h"
#include "TaskManager.h"
#include "DnGameTask.h"


CDnWorldBrokenDamageProp::CDnWorldBrokenDamageProp( CMultiRoom* pRoom ) : CDnWorldBrokenProp( pRoom ),
																		  CDnDamageBase( DamageObjectTypeEnum::Prop ),
																		  m_iSkillApplyType( -1 ),
																		  m_bEnableHitSignal( true )
{
}

CDnWorldBrokenDamageProp::~CDnWorldBrokenDamageProp(void)
{
	ReleasePostCustomParam();
}

bool CDnWorldBrokenDamageProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	if( CDnWorldBrokenProp::Initialize( pParentSector, szPropName, vPos, vRotate, vScale ) == false ) return false;
	if( m_hMonster ) *m_hMonster->GetMatEx() = *GetMatEx();
	return true;
}

bool CDnWorldBrokenDamageProp::InitializeTable( int nTableID )
{
	if( CDnWorldActProp::InitializeTable( nTableID ) == false ) return false;

	if( GetData() ) {
		BrokenDamageStruct*	pStruct = (BrokenDamageStruct *)GetData();
		bool				bResult = false;

//#61146 Structure allowing Trap-type prop attack power to be set differently by difficulty.
#ifdef PER_ADD_PROP_TRAP_DIFFICULT_SET
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID, pStruct->MonsterWeight );
#else
		bResult = InitializeMonsterActorProp( pStruct->nMonsterTableID );
#endif	
		
		if( bResult )
		{
//			m_hMonster->SetUniqueID( STATIC_INSTANCE(CDnActor::s_dwUniqueCount)++ );

			m_nDurability = pStruct->nDurability;

			if( -1 != m_nDurability )
			{
				if( 0 == m_nDurability ) m_nDurability = 1;
				m_nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				if( m_nItemDropGroupTableID > 0 ) {
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

			m_iSkillApplyType = pStruct->SkillApplyType;
			if( 0 != pStruct->nSkillTableID )
			{
				m_SkillComponent.Initialize( pStruct->nSkillTableID, pStruct->nSkillLevel );
			}

			m_HitComponent.Initialize( m_hMonster, &m_Cross, GetMySmartPtr(), this );
		}
		else
			return false;
	}

	return true;
}


void CDnWorldBrokenDamageProp::OnDamage( CDnDamageBase *pHitter, CDnDamageBase::SHitParam &HitParam )
{
	CDnWorldBrokenProp::OnDamage( pHitter, HitParam );

	if( m_nDurability <= 0 )
	{
		// If the skill apply type is last-hit user, check if durability is 0 and apply the skill's state effect.
		if( APPLY_SKILL_LASTHITTER == m_iSkillApplyType )
		{
			DnActorHandle hLastHitter = pHitter->GetActorHandle();
			if( hLastHitter )
			{
				const CDnSkill::SkillInfo& SkillInfo = m_SkillComponent.GetSkillInfo();
				map<int, bool> mapDuplicateResult;
				bool bAllowAddStateEffect = (CDnSkill::Fail != CDnSkill::CanApplySkillStateEffect( &SkillInfo, hLastHitter, SkillInfo.iSkillID, SkillInfo.iLevel, 
																								   SkillInfo.iSkillDuplicateMethod, SkillInfo.eDurationType, mapDuplicateResult ));

				if( bAllowAddStateEffect )
				{
					int iNumStateEffect = m_SkillComponent.GetNumStateEffect();
					for( int iStateEffect = 0; iStateEffect < iNumStateEffect; ++iStateEffect )
					{
						const CDnSkill::StateEffectStruct* pStateEffectInfo = m_SkillComponent.GetStateEffectInfo( iStateEffect );
						int iID = hLastHitter->CmdAddStateEffect( &SkillInfo, 
																  (STATE_BLOW::emBLOW_INDEX)pStateEffectInfo->nID, 
																  pStateEffectInfo->nDurationTime, 
																  pStateEffectInfo->szValue.c_str() );
					}
				}
				else
				{
					OutputDebug( "CDnWorldBrokenDamageProp::OnDamage() : APPLY_SKILL_LASTHITTER -> failed to apply state effect!\n" );
				}
			}
		}
	}
}

void CDnWorldBrokenDamageProp::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	if( m_bEnableHitSignal )
	{
#ifdef PRE_ADD_HIT_PROP_ADD_SE
		if( m_hMonster )
		{
			DNVector(DnActorHandle) vlHittedActors;
			m_HitComponent.OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex, &vlHittedActors );
			if( false == vlHittedActors.empty() )
			{
				if( APPLY_SKILL_HITSIGNAL_RANGE == m_iSkillApplyType )
				{
					// When applied to a prop, ignore skill target types etc. to simplify the process if it passed the hit signal
					// Do not separately check the target team specified on the skill.
					m_SkillComponent.ApplyStateBlowsToTheseTargets( m_hMonster, vlHittedActors );
				}
			}
		}
#else
		m_HitComponent.OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
#endif // #ifdef PRE_ADD_HIT_PROP_ADD_SE
	}

	CDnWorldBrokenProp::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}


void CDnWorldBrokenDamageProp::_OnLifeTimeEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_bEnableHitSignal = true;

	CDnWorldBrokenProp::_OnLifeTimeEnd( LocalTime, fDelta );
}