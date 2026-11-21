#include "stdafx.h"
#include "DnSkillPropComponent.h"
#include "DnTableDB.h"
#include "EtMatrixEx.h"
#include "DnWorldProp.h"
#include "DnProjectile.h"
#include "DnMonsterActor.h"


CDnSkillPropComponent::CDnSkillPropComponent(void)
{
}

CDnSkillPropComponent::~CDnSkillPropComponent(void)
{
	SAFE_RELEASE_SPTR( m_hSkill );
}


void CDnSkillPropComponent::Initialize( int iSkillID, int iSkillLevel )
{
	if( iSkillID == 0 || iSkillLevel == 0 )
		return;

	// Pre-create the state effect to be used.
	DNTableFileFormat* pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat* pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	vector<int> vlSkillLevelList;
	if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", iSkillID, vlSkillLevelList ) <= 0 ) 
	{
		_ASSERT( !"Prop skill index not found in SkillLevelTable." );
		//return false;
	}

	int iSkillLevelTableID = -1;
	for( int i = 0; i < (int)vlSkillLevelList.size(); ++i )
	{
		int iNowLevel = pSkillLevelTable->GetFieldFromLablePtr( vlSkillLevelList.at(i), "_SkillLevel" )->GetInteger();
		if( iNowLevel == iSkillLevel )
		{
			iSkillLevelTableID = vlSkillLevelList.at( i );
			break;
		}
	}

	_ASSERT( -1 != iSkillLevelTableID && "Requested skill level not found for prop skill." );

	// Load state effect info
	CDnSkill::StateEffectStruct StateEffect;
	char caLable[ 64 ];
	ZeroMemory( caLable, sizeof(caLable) );
	for( int i = 0; i < CDnSkill::MAX_STATE_EFFECT_COUNT; ++i )
	{
		sprintf_s( caLable, "_EffectClass%d", i + 1 );
		StateEffect.nID = pSkillTable->GetFieldFromLablePtr( iSkillID, caLable )->GetInteger();

		if( StateEffect.nID < 1 ) 
			continue;

		sprintf_s( caLable, "_EffectClass%dApplyType", i + 1 );
		StateEffect.ApplyType = (CDnSkill::StateEffectApplyType)pSkillTable->GetFieldFromLablePtr( iSkillID, caLable )->GetInteger();

		sprintf_s( caLable, "_EffectClassValue%d", i + 1 );
		StateEffect.szValue = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetString();

		sprintf_s( caLable, "_EffectClassValue%dDuration", i + 1 );
		StateEffect.nDurationTime = pSkillLevelTable->GetFieldFromLablePtr( iSkillLevelTableID, caLable )->GetInteger();

		m_vlStateEffectList.push_back( StateEffect );
	}

	m_SkillInfo.iSkillID = iSkillID;
	m_SkillInfo.iSkillLevelID = iSkillLevelTableID;
	m_SkillInfo.iLevel = iSkillLevel;
	m_SkillInfo.eDurationType = (CDnSkill::DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillID, "_DurationType" )->GetInteger();
	m_SkillInfo.eTargetType = (CDnSkill::TargetTypeEnum)pSkillTable->GetFieldFromLablePtr( iSkillID, "_TargetType" )->GetInteger();
	m_SkillInfo.iSkillDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iSkillID, "_SkillDuplicate" )->GetInteger();
	m_SkillInfo.iDuplicateCount = pSkillTable->GetFieldFromLablePtr( iSkillID, "_EffectAmassCount" )->GetInteger();
	m_SkillInfo.iDissolvable = pSkillTable->GetFieldFromLablePtr( iSkillID, "_Dissolvable" )->GetInteger();

	// Since this module is used by a buff prop, set info indicating the skill originated from a prop.
	m_SkillInfo.bFromBuffProp = true;
}	


void CDnSkillPropComponent::OnSignal( DnActorHandle hActor, DnPropHandle hProp, SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, 
									  LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type )
	{
		case STE_Projectile:
			{
				ProjectileStruct *pStruct = (ProjectileStruct *)pPtr;

				MatrixEx LocalCross = *hProp->GetMatEx();
				CDnProjectile *pProjectile = CDnProjectile::CreateProjectile( hProp->GetRoom(), hActor, LocalCross, pStruct );
				if( pProjectile == NULL ) break;

				pProjectile->SetShooterProp( hProp );

				if( m_SkillInfo.iSkillID != 0 )
				{
					int iNumStateEffect = (int)m_vlStateEffectList.size();
					for( int iStateEffect = 0; iStateEffect < iNumStateEffect; ++iStateEffect )
						pProjectile->AddStateEffect( m_vlStateEffectList.at(iStateEffect) );

					if( !m_hSkill )
						m_hSkill = CDnSkill::CreateSkill( hActor, m_SkillInfo.iSkillID, m_SkillInfo.iLevel );

					pProjectile->SetParentSkill( m_hSkill );
					pProjectile->SetParentSkillInfo( m_SkillInfo );

					//bool bActorAttachWeapon = false;
					//if( pStruct->nWeaponTableID == 0 && GetWeapon(1) ) bActorAttachWeapon = true;

					//OnProjectile( pProjectile, bActorAttachWeapon );
					//OnSkillProjectile( pProjectile );
				}
			}
			break;

	default:
		hActor->OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
		break;
	}
}

#ifdef PRE_ADD_HIT_PROP_ADD_SE
//void CDnSkillPropComponent::ApplyStateBlowsToTheseTargets( DnActorHandle hPropActor, DNVector(DnActorHandle)& vlhActors )
void CDnSkillPropComponent::ApplyStateBlowsToTheseTargets( DnActorHandle hPropActor, DNVector(DnActorHandle)& vlhActors, HitStruct * pStruct )
{
	if( !m_hSkill )
		m_hSkill = CDnSkill::CreateSkill( hPropActor, m_SkillInfo.iSkillID, m_SkillInfo.iLevel );
	
	if( !m_hSkill )
		return;

	int iNumActorApplyStateEffect = (int)vlhActors.size();
	for( int i = 0; i < iNumActorApplyStateEffect; ++i )
	{
		// Ignore applying state effects when the target is in Ice Prison state
		if (vlhActors[ i ]->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
			continue;

		// For props, to targets that passed the hit signal
		// Process adding the state effect.
		// Later, if complex features like skill stacking (as in monsters or players) are required 
		// Must follow the same format as the actor's hit signal handling.

		//// Distinguish state effect application based on whether the skill target is set to ally or target.
		//switch( m_hSkill->GetTargetType() )
		//{
		//	case CDnSkill::Enemy:
		//	case CDnSkill::Self:
		//		if( hPropActor->GetTeam() == vlhActors[ i ]->GetTeam() )
		//			continue;
		//		break;

		//	case CDnSkill::Friend:
		//	case CDnSkill::Party:
		//		if( hPropActor->GetTeam() != vlhActors[ i ]->GetTeam() )
		//			continue;
		//		break;

		//		// If the skill target is both allies and enemies, the state effect application side must distinguish whether to apply.
		//	case CDnSkill::All:
		//		break;
		//}

//		// Need to handle persistent distinguish index when adding a state effect.
//		// Distinguish the actor's persistent effect index and add the state effect on success.
//		map<int, bool> mapDuplicateResult;
//		CDnSkill::CanApply eResult = CDnSkill::CanApplySkillStateEffect( vlhActors[i], m_hSkill, mapDuplicateResult );
//		if( CDnSkill::CanApply::Fail != eResult )
//		{
			for( DWORD k = 0; k < m_hSkill->GetStateEffectCount(); k++ ) 
			{
				CDnSkill::StateEffectStruct *pLocalStruct = m_hSkill->GetStateEffectFromIndex(k);
//
//#if defined(PRE_ADD_SKIP_STATEBLOW)				
				// #74389 [Desert Dragon] HitStateEffect attribute prop bug.
				if( pStruct && pStruct->szSkipStateBlows && CDnSkill::IsSkipStateBlow(pStruct->szSkipStateBlows, (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID) )
					continue;
//#endif // PRE_ADD_SKIP_STATEBLOW
//
//				// For skills that also heal allies, both Self heal and Target heal state effects are declared.
//				// Self state effects were already applied to self, and this is where Hit signal is evaluated, so only Target is applied.
//				switch( pLocalStruct->ApplyType )
//				{
//					case CDnSkill::ApplySelf:
//						continue;
//						break;
//
//					case CDnSkill::ApplyTarget:
//						break;
//
//					case CDnSkill::ApplyEnemy:
//						if( hPropActor->GetTeam() == vlhActors[ i ]->GetTeam() )
//							continue;
//						break;
//
//					case CDnSkill::ApplyFriend:
//						if( hPropActor->GetTeam() != vlhActors[ i ]->GetTeam() )
//							continue;
//						break;
//				}
//
//				// For same-skill stacking, probability checks in skill effects are already checked in CanApplySkillStateEffect
//				// Already passed. So check here if probability was already rolled, and if so, call the function without re-rolling.
//				bool bAllowAddThisSE = true;
//				bool bCheckCanBegin = true;
//				if( CDnSkill::ApplyDuplicateSameSkill == eResult )
//				{
//					map<int, bool>::iterator iter = mapDuplicateResult.find( pLocalStruct->nID );
//					// If not in the map, the actor does not have the state effect, so run the normal add routine.
//					if( mapDuplicateResult.end() != iter )
//					{
//						// A probabilistic state effect from the same skill is currently active, so CanAdd was called but failed.
//						// Do not add state effect in this case.
//						if( false == (iter->second) )
//							bAllowAddThisSE = false;
//						else
//							// Already passed CanAdd, so skip checking again when calling CmdAddStateEffect.
//							bCheckCanBegin = false;
//					}
//				}

				//if( bAllowAddThisSE )
				//{
					//// Remove registered state effects marked for removal here. [2010/12/08 semozz]
					//vlhActors[i]->RemoveResetStateBlow();

					vlhActors[i]->CmdAddStateEffect( m_hSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
													 pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str()/*, false, bCheckCanBegin*/ );
				//}
			}
		//}
		// Clear reset list when skill usage ends [2010/12/09 semozz]
		//vlhActors[i]->InitStateBlowIDToRemove();
	}
}
#endif // #ifdef PRE_ADD_HIT_PROP_ADD_SE   