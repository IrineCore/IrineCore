#include "stdafx.h"
#include "DnMonsterState.h"
#include "DnTableDB.h"
#include "DnMonsterActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMonsterState::CDnMonsterState()
{
	m_nDeadExperience = 0;
	m_nCompleteExperience = 0;
	m_nDeadDurability = 0;
	m_nItemDropGroupTableID = -1;

	m_Grade = Normal;
	m_bSlowByDie = false;
	m_bSummonRandomFrame = false;
	m_bApplyEffectScale = false;
	m_nRaceID = -1;
	m_bBossKillCheck = false;
}

CDnMonsterState::~CDnMonsterState()
{
}

void CDnMonsterState::Initialize( int nClassID )
{
	CDnActorState::Initialize( nClassID );

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	DNTableFileFormat* pActorSox = GetDNTable( CDnTableDB::TACTOR );
	int nItemID = 0;
	CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(m_pActor);
	if( pMonster ) nItemID = pMonster->GetMonsterClassID();
	if( nItemID < 1 ) return;

	m_Grade = (GradeEnum)pSox->GetFieldFromLablePtr( nItemID, "_Grade" )->GetInteger();
	m_nItemDropGroupTableID = pSox->GetFieldFromLablePtr( nItemID, "_ItemDropGroupID" )->GetInteger();
	m_bSlowByDie = ( pSox->GetFieldFromLablePtr( nItemID, "_SlowEffectByDeath" )->GetInteger() ) ? true : false;

	m_bSummonRandomFrame = pActorSox->GetFieldFromLablePtr( nClassID, "_RandomFrameSummon" )->GetInteger() ? true : false;
	m_bApplyEffectScale = ( pSox->GetFieldFromLablePtr( nItemID, "_EffectScale" )->GetInteger() ) ? true : false;
	m_nRaceID = pSox->GetFieldFromLablePtr( nItemID, "_MonsterRaceID" )->GetInteger();
	m_bBossKillCheck = ( pSox->GetFieldFromLablePtr( nItemID, "_BossKillCheck" )->GetInteger() ) ? true : false;
}

void CDnMonsterState::CalcBaseState( StateTypeEnum Type )
{
#ifdef PRE_ADD_SKILLBUF_RENEW
	// For monsters copied from the summoner, do not fetch BaseState from the table; keep the copied stats as is.
	if( m_bCopiedFromSummoner )
		return;
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
	DNTableFileFormat* pWeight = GetDNTable( CDnTableDB::TMONSTERWEIGHT );
	int nItemID = 0;
	CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(m_pActor);
	if( pMonster ) nItemID = pMonster->GetMonsterClassID();
	if( nItemID < 1 ) return;
	int nWeightID = pMonster->GetMonsterWeightTableID();

	int nValue;
	float fValue;
	INT64 nValueHP = 0;

	if( Type & ST_HP ) {
		nValueHP = pSox->GetFieldFromLablePtr( nItemID, "_HP" )->GetInteger();
		if( nWeightID != -1 ) nValueHP = (INT64)( nValueHP * pWeight->GetFieldFromLablePtr( nWeightID, "_HPWeight" )->GetFloat() );
		m_BaseState.SetMaxHP( nValueHP );
	}

	if( Type & ST_SP ) {
		m_BaseState.SetMaxSP( 0 );
	}

	if( Type & ST_AttackP ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StrPMin" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_AttackPMinWeight" )->GetFloat() );
		m_BaseState.SetAttackPMin( nValue );

		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StrPMax" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_AttackPMaxWeight" )->GetFloat() );
		m_BaseState.SetAttackPMax( nValue );
	}
	if( Type & ST_AttackM ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StrMMin" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_AttackMMinWeight" )->GetFloat() );
		m_BaseState.SetAttackMMin( nValue );

		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StrMMax" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_AttackMMaxWeight" )->GetFloat() );
		m_BaseState.SetAttackMMax( nValue );
	}

	if( Type & ST_DefenseP ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_DefP" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_DefensePWeight" )->GetFloat() );
		m_BaseState.SetDefenseP( nValue );
	}
	if( Type & ST_DefenseM ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_DefM" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_DefenseMWeight" )->GetFloat() );
		m_BaseState.SetDefenseM( nValue );
	}

	if( Type & ST_MoveSpeed ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_MoveSpeed" )->GetInteger();
		if( nWeightID != -1 ) nValue += pWeight->GetFieldFromLablePtr( nWeightID, "_MoveSpeedWeight" )->GetInteger();
		m_BaseState.SetMoveSpeed( nValue );
	}

	if( Type & ST_DownDelay ) {
		fValue = pSox->GetFieldFromLablePtr( nItemID, "_DownDelay" )->GetFloat();
		if( nWeightID != -1 ) fValue = (float)( fValue * pWeight->GetFieldFromLablePtr( nWeightID, "_DownDelayWeight" )->GetFloat() );
		m_BaseState.SetDownDelayProb( fValue );
	}

	if( Type & ST_Stiff ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_Stiff" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_StiffWeight" )->GetFloat() );
		m_BaseState.SetStiff( nValue );
	}

	if( Type & ST_StiffResistance ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StiffResistance" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_StiffResistanceWeight" )->GetFloat() );
		m_BaseState.SetStiffResistance( nValue );
	}

	if( Type & ST_Critical ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_Critical" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_CriticalWeight" )->GetFloat() );
		m_BaseState.SetCritical( nValue );
	}

	if( Type & ST_CriticalResistance ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_CriticalResistance" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_CriticalResistanceWeight" )->GetFloat() );
		m_BaseState.SetCriticalResistance( nValue );
	}

	if( Type & ST_Stun ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_Stun" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_StunWeight" )->GetFloat() );
		m_BaseState.SetStun( nValue );
	}

	if( Type & ST_StunResistance ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_StunResistance" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_StunResistanceWeight" )->GetFloat() );
		m_BaseState.SetStunResistance( nValue );
	}

	if( Type & ST_SuperAmmor ) {
		nValue = pSox->GetFieldFromLablePtr( nItemID, "_SuperAmmor" )->GetInteger();
		if( nWeightID != -1 ) nValue = (int)( nValue * pWeight->GetFieldFromLablePtr( nWeightID, "_SuperAmmorWeight" )->GetFloat() );
		m_BaseState.SetSuperAmmor( nValue );
	}

	m_nDeadExperience = pSox->GetFieldFromLablePtr( nItemID, "_DeadExperience" )->GetInteger();
	if( nWeightID != -1 ) m_nDeadExperience = (int)( m_nDeadExperience * pWeight->GetFieldFromLablePtr( nWeightID, "_DeadExperienceWeight" )->GetFloat() );

	m_nCompleteExperience = pSox->GetFieldFromLablePtr( nItemID, "_CompleteExperience" )->GetInteger();
	if( nWeightID != -1 ) m_nCompleteExperience = (int)( m_nCompleteExperience * pWeight->GetFieldFromLablePtr( nWeightID, "_CompleteExperienceWeight" )->GetFloat() );

	m_nDeadDurability = pSox->GetFieldFromLablePtr( nItemID, "_DeadDurability" )->GetInteger();
	if( nWeightID != -1 ) m_nDeadDurability = (int)( m_nDeadDurability * pWeight->GetFieldFromLablePtr( nWeightID, "_DeadDurabilityWeight" )->GetFloat() );

	if( Type & ST_ElementDefense ) {
		char szLable[64];
		for( int i=0; i<4; i++ ) {
			sprintf_s( szLable, "%sDef", CDnState::s_szElementStr[i] );
			fValue = pSox->GetFieldFromLablePtr( nItemID, szLable )->GetFloat();
			if( nWeightID != -1 ) {
				sprintf_s( szLable, "%sDef_Weight", CDnState::s_szElementStr[i] );
				fValue *= pWeight->GetFieldFromLablePtr( nWeightID, szLable )->GetFloat();
			}
			m_BaseState.SetElementDefense( (ElementEnum)i, fValue );
		}
	}
	// Attack is changed to a single configured value and fires downward.
	if( Type & ST_ElementAttack ) {
		for( int i=0; i<ElementEnum::ElementEnum_Amount; i++ )
			m_BaseState.SetElementAttack( (ElementEnum)i, 0.f );

		int nStrElementType = pSox->GetFieldFromLablePtr( nItemID, "_Element_Str_Type" )->GetInteger();
		if( nStrElementType != -1 ) {
			fValue = pSox->GetFieldFromLablePtr( nItemID, "_Element_Str" )->GetFloat();
			if( nWeightID != -1 ) {
				fValue *= pWeight->GetFieldFromLablePtr( nWeightID, "_Element_Str_Weight" )->GetFloat();
			}
			m_BaseState.SetElementAttack( (ElementEnum)nStrElementType, fValue );
		}
	}
	m_BaseState.CalcValueType();
}

void CDnMonsterState::CalcState( RefreshStateExtentEnum Extent, StateTypeEnum Type )
{
	if( m_nLevel == 0 ) return;

	std::vector<CDnState *> pVecAbsoluteList;
	std::vector<CDnState *> pVecRatioList;
	int nAddValue = 0;
	float fAddValue = 0.0f;
	float fRatioValue = 0.0f;
	int nTemp;
	float fTemp;
	INT64 nAddHPValue = 0, nTempHP = 0;

	if( Extent & RefreshEquip ) {
		GetStateList( BaseList | WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList, ValueTypeAbsolute, pVecAbsoluteList );
		GetStateList( BaseList | WeaponList | DefenseList | AccessoryList | StateEffectList | SetItemList | AppellationList, ValueTypeRatio, pVecRatioList ); 

		if( Type & ST_HP ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddHPValue, GetMaxHP() );
			nTempHP = nAddHPValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTempHP, fRatioValue, GetMaxHPRatio() );
			nTempHP += (INT64)fRatioValue;
			m_StateStep[0].SetMaxHP( nTempHP );
		}

		if( Type & ST_SP ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetMaxSP() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetMaxSPRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetMaxSP( nTemp );
		}

		if( Type & ST_AttackP ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackPMin() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetAttackPMinRatio() )
				nTemp += (int)fRatioValue;
			m_StateStep[0].SetAttackPMin( nTemp );

			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackPMax() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetAttackPMaxRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetAttackPMax( nTemp );
		}
		if( Type & ST_AttackM ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackMMin() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetAttackMMinRatio() )
				nTemp += (int)fRatioValue;
			m_StateStep[0].SetAttackMMin( nTemp );

			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackMMax() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetAttackMMaxRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetAttackMMax( nTemp );
		}

		if( Type & ST_DefenseP ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetDefenseP() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetDefensePRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetDefenseP( nTemp );
		}
		if( Type & ST_DefenseM ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetDefenseM() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetDefenseMRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetDefenseM( nTemp );
		}

		if( Type & ST_MoveSpeed ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetMoveSpeed() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetMoveSpeedRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetMoveSpeed( nTemp );
		}

		if( Type & ST_DownDelay ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, fAddValue, GetDownDelayProb() );
			fTemp = fAddValue;
			CALC_STATE_LIMIT( fTemp, 0.f, 1.f );
			m_StateStep[0].SetDownDelayProb( fTemp );
		}

		if( Type & ST_Stiff ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStiff() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetStiffRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetStiff( nTemp );
		}

		if( Type & ST_StiffResistance ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStiffResistance() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetStiffResistance() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetStiffResistance( nTemp );
		}

		if( Type & ST_Critical ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetCritical() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetCriticalRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetCritical( nTemp );
		}

		if( Type & ST_CriticalResistance ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetCriticalResistance() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetCriticalResistanceRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetCriticalResistance( nTemp );
		}

		if( Type & ST_Stun ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStun() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetStunRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetStun( nTemp );
		}

		if( Type & ST_StunResistance ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStunResistance() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetStunResistanceRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetStunResistance( nTemp );
		}

		if( Type & ST_SuperAmmor ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetSuperAmmor() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetSuperAmmorRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetSuperAmmor( nTemp );
		}
		if( Type & ST_FinalDamage ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetFinalDamage() );
			nTemp = nAddValue;
			CALC_STATE_VALUE_RATIO( pVecRatioList, nTemp, fRatioValue, GetFinalDamageRatio() );
			nTemp += (int)fRatioValue;
			m_StateStep[0].SetFinalDamage( nTemp );
		}
		if( Type & ST_ElementAttack ) {
			for( int i=0; i<CDnState::ElementEnum_Amount; i++ ) {
				CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, fAddValue, GetElementAttack((ElementEnum)i) );
				fTemp = fAddValue;
				m_StateStep[0].SetElementAttack( (ElementEnum)i, fTemp );
			}
		}
		if( Type & ST_ElementDefense ) {
			for( int i=0; i<CDnState::ElementEnum_Amount; i++ ) {
				CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, fAddValue, GetElementDefense((ElementEnum)i) );
				fTemp = fAddValue;
				m_StateStep[0].SetElementDefense( (ElementEnum)i, fTemp );
			}
		}
	}

	// Post State 
	GetStateList( PostStateEffectList, ValueTypeAbsolute, pVecAbsoluteList );
	GetStateList( PostStateEffectList, ValueTypeRatio, pVecRatioList ); 

	if( Type & ST_HP ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetMaxHP(), fRatioValue, GetMaxHPRatio() );
		nTempHP = (INT64)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddHPValue, GetMaxHP() );
		nTempHP += nAddHPValue;
		m_StateStep[1].SetMaxHP( nTempHP );
	}

	if( Type & ST_SP ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetMaxSP(), fRatioValue, GetMaxSPRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetMaxSP() );
		nTemp += nAddValue;
		m_StateStep[1].SetMaxSP( nTemp );
	}

	if( Type & ST_AttackP ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetAttackPMin(), fRatioValue, GetAttackPMinRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackPMin() );
		nTemp += nAddValue;
		m_StateStep[1].SetAttackPMin( nTemp );

		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetAttackPMax(), fRatioValue, GetAttackPMaxRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackPMax() );
		nTemp += nAddValue;
		m_StateStep[1].SetAttackPMax( nTemp );
	}

	if( Type & ST_AttackM ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetAttackMMin(), fRatioValue, GetAttackMMinRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackMMin() );
		nTemp += nAddValue;
		m_StateStep[1].SetAttackMMin( nTemp );

		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetAttackMMax(), fRatioValue, GetAttackMMaxRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetAttackMMax() );
		nTemp += nAddValue;
		m_StateStep[1].SetAttackMMax( nTemp );
	}

	if( Type & ST_DefenseP ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetDefenseP(), fRatioValue, GetDefensePRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetDefenseP() );
		nTemp += nAddValue;
		m_StateStep[1].SetDefenseP( nTemp );
	}

	if( Type & ST_DefenseM ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetDefenseM(), fRatioValue, GetDefenseMRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetDefenseM() );
		nTemp += nAddValue;
		m_StateStep[1].SetDefenseM( nTemp );
	}

	if( Type & ST_MoveSpeed ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetMoveSpeed(), fRatioValue, GetMoveSpeedRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetMoveSpeed() );
		nTemp += nAddValue;
		m_StateStep[1].SetMoveSpeed( nTemp );
	}

	if( Type & ST_DownDelay ) {
		CALC_STATE_VALUE_ABSOLUTE( pVecRatioList, fAddValue, GetDownDelayProb() );
		fTemp = fAddValue;
		CALC_STATE_LIMIT( fTemp, 0.f, 1.f );
		m_StateStep[1].SetDownDelayProb( fTemp );
	}

	if( Type & ST_Stiff ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetStiff(), fRatioValue, GetStiffRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStiff() );
		nTemp += nAddValue;
		m_StateStep[1].SetStiff( nTemp );
	}

	if( Type & ST_StiffResistance ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetStiffResistance(), fRatioValue, GetStiffResistanceRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStiffResistance() );
		nTemp += nAddValue;
		m_StateStep[1].SetStiffResistance( nTemp );
	}

	if( Type & ST_Critical ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetCritical(), fRatioValue, GetCriticalRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetCritical() );
		nTemp += nAddValue;
		m_StateStep[1].SetCritical( nTemp );
	}

	if( Type & ST_CriticalResistance ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetCriticalResistance(), fRatioValue, GetCriticalResistanceRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetCriticalResistance() );
		nTemp += nAddValue;
		m_StateStep[1].SetCriticalResistance( nTemp );
	}

	if( Type & ST_Stun ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetStun(), fRatioValue, GetStunRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStun() );
		nTemp += nAddValue;
		m_StateStep[1].SetStun( nTemp );
	}

	if( Type & ST_StunResistance ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetStunResistance(), fRatioValue, GetStunResistanceRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetStunResistance() );
		nTemp += nAddValue;
		m_StateStep[1].SetStunResistance( nTemp );
	}

	if( Type & ST_SuperAmmor ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetSuperAmmor(), fRatioValue, GetSuperAmmorRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetSuperAmmor() );
		nTemp += nAddValue;
		m_StateStep[1].SetSuperAmmor( nTemp );
	}
	if( Type & ST_FinalDamage ) {
		CALC_STATE_VALUE_RATIO( pVecRatioList, m_StateStep[0].GetFinalDamage(), fRatioValue, GetFinalDamageRatio() );
		nTemp = (int)fRatioValue;
		CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, nAddValue, GetFinalDamage() );
		nTemp += nAddValue;
		m_StateStep[1].SetFinalDamage( nTemp );
	}

	if( Type & ST_ElementAttack ) {
		for( int i=0; i<CDnState::ElementEnum_Amount; i++ ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, fAddValue, GetElementAttack((ElementEnum)i) );
			fTemp = fAddValue;
			m_StateStep[1].SetElementAttack( (ElementEnum)i, fTemp );
		}
	}

	if( Type & ST_ElementDefense ) {
		for( int i=0; i<CDnState::ElementEnum_Amount; i++ ) {
			CALC_STATE_VALUE_ABSOLUTE( pVecAbsoluteList, fAddValue, GetElementDefense((ElementEnum)i) );
			fTemp = fAddValue;
			m_StateStep[1].SetElementDefense( (ElementEnum)i, fTemp );
		}
	}
	if( Type & ST_HP ) 
	{
		m_nMaxHP = m_StateStep[0].GetMaxHP() + m_StateStep[1].GetMaxHP();
	}
	
	if( Type & ST_SP ) 
	{
		m_nMaxSP = m_StateStep[0].GetMaxSP() + m_StateStep[1].GetMaxSP();
	}

	if( Type & ST_AttackP ) {
		m_nAttackP[0] = m_StateStep[0].GetAttackPMin() + m_StateStep[1].GetAttackPMin();
		m_nAttackP[1] = m_StateStep[0].GetAttackPMax() + m_StateStep[1].GetAttackPMax();

		// Related to #31367. Adjust minimum attack power if it falls below a certain ratio of the original value.
		float fAttackPowerClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AttackPowerClampLowestRatio );
		int nAttackPowerLowest = int((float)m_StateStep[0].GetAttackPMin() * fAttackPowerClampLowestRatio);
		if( m_nAttackP[ 0 ] < nAttackPowerLowest )
		{
			m_nAttackP[ 0 ] = nAttackPowerLowest;
			m_nAttackP[ 1 ] = int((float)m_StateStep[0].GetAttackPMax() * fAttackPowerClampLowestRatio);
		}
	}
	if( Type & ST_AttackM ) {
		m_nAttackM[0] = m_StateStep[0].GetAttackMMin() + m_StateStep[1].GetAttackMMin();
		m_nAttackM[1] = m_StateStep[0].GetAttackMMax() + m_StateStep[1].GetAttackMMax();

		// Related to #31367. Adjust minimum attack power if it falls below a certain ratio of the original value.
		float fAttackPowerClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AttackPowerClampLowestRatio );
		int nAttackPowerLowest = int((float)m_StateStep[0].GetAttackMMin() * fAttackPowerClampLowestRatio);
		if( m_nAttackM[ 0 ] < nAttackPowerLowest )
		{
			m_nAttackM[ 0 ] = nAttackPowerLowest;
			m_nAttackM[ 1 ] = int((float)m_StateStep[0].GetAttackMMax() * fAttackPowerClampLowestRatio);
		}
	}
	if( Type & ST_DefenseP ) m_nDefenseP = m_StateStep[0].GetDefenseP() + m_StateStep[1].GetDefenseP();
	if( Type & ST_DefenseM ) m_nDefenseM = m_StateStep[0].GetDefenseM() + m_StateStep[1].GetDefenseM();

	if( Type & ST_ElementAttack ) 
	{
		for( int i=0; i<4; i++ ) 
			m_fElementAttack[i] = m_StateStep[0].GetElementAttack( (ElementEnum)i ) + m_StateStep[1].GetElementAttack( (ElementEnum)i );
	}
	if( Type & ST_ElementDefense ) 
	{
#if defined(PRE_FIX_50470)
		for( int i=0; i<4; i++ ) 
		{
#ifdef PRE_ADD_BUFF_STATE_LIMIT
			float fDefenceValue = m_StateStep[0].GetElementDefense( (ElementEnum)i ) + m_StateStep[1].GetElementDefense( (ElementEnum)i );
			CALC_STATE_LIMIT(  
				fDefenceValue , 
				CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Min ),
				CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Element_Defense_Max ) );

			m_fElementDefense[i] = fDefenceValue;
#else
			float fDefecneValue = m_StateStep[0].GetElementDefense( (ElementEnum)i ) + m_StateStep[1].GetElementDefense( (ElementEnum)i );
			m_fElementDefense[i] = max( fDefecneValue, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MonsterElementMinRevision ) );
#endif
		}
#else
		for( int i=0; i<4; i++ ) m_fElementDefense[i] = m_StateStep[0].GetElementDefense( (ElementEnum)i ) + m_StateStep[1].GetElementDefense( (ElementEnum)i );
#endif // PRE_FIX_50470
	}
	if( Type & ST_MoveSpeed ) m_nMoveSpeed = m_StateStep[0].GetMoveSpeed() + m_StateStep[1].GetMoveSpeed();
	if( Type & ST_DownDelay ) m_fDownDelayProb = m_StateStep[0].GetDownDelayProb() + m_StateStep[1].GetDownDelayProb();
	if( Type & ST_Stiff ) m_nStiff = m_StateStep[0].GetStiff() + m_StateStep[1].GetStiff();
	if( Type & ST_StiffResistance ) m_nStiffResistance = m_StateStep[0].GetStiffResistance() + m_StateStep[1].GetStiffResistance();
	if( Type & ST_Critical ) m_nCritical = m_StateStep[0].GetCritical() + m_StateStep[1].GetCritical(); 
	if( Type & ST_CriticalResistance ) m_nCriticalResistance = m_StateStep[0].GetCriticalResistance() + m_StateStep[1].GetCriticalResistance();
	if( Type & ST_Stun ) m_nStun = m_StateStep[0].GetStun() + m_StateStep[1].GetStun();
	if( Type & ST_StunResistance ) m_nStunResistance = m_StateStep[0].GetStunResistance() + m_StateStep[1].GetStunResistance();
	if( Type & ST_SuperAmmor ) m_nSuperAmmor = m_StateStep[0].GetSuperAmmor() + m_StateStep[1].GetSuperAmmor();
	if( Type & ST_FinalDamage ) m_nFinalDamage = m_StateStep[0].GetFinalDamage() + m_StateStep[1].GetFinalDamage();

	if( Extent & RefreshBase ) m_BaseState.CalcValueType();
	if( Extent & RefreshEquip ) m_StateStep[0].CalcValueType();
	if( Extent & RefreshSkill ) m_StateStep[1].CalcValueType();

	CalcValueType();

	if( m_nHP > m_nMaxHP ) m_nHP = m_nMaxHP;
	if( m_nSP > m_nMaxSP ) m_nSP = m_nMaxSP;
}


void CDnMonsterState::GetStateList( int ListType, CDnState::ValueType Type, std::vector<CDnState *> &VecList )
{
	VecList.clear();
	CDnActorState::GetStateList( ListType, Type, VecList );
}
