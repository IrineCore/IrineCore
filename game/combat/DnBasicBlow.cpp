#include "stdafx.h"
#include "DnBasicBlow.h"
#include "DnActor.h"
#include "DnSkillTask.h"
#include "DnMonsterActor.h"
#include "DnPlayerActor.h"
#ifdef _GAMESERVER
#include "DNUserSession.h"
#else
#include "DnInterface.h"
#endif
#include "DnStateBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnBasicBlow::CDnBasicBlow(DnActorHandle hActor, const char *szValue)
: CDnBlow(hActor), m_bNeedRefreshState( true ), m_bRefreshAll( false ),
  m_eRefreshState( CDnActorState::ST_All ), m_eAddBlowStateType( CDnActorState::Equip_Buff_Level )
{
	SetValue(szValue);
	m_fValue = (float)atof(szValue);
}

CDnBasicBlow::~CDnBasicBlow(void)
{
}

void CDnBasicBlow::SetBlowState()
{
	switch( m_StateBlow.emBlowIndex )
	{
		case STATE_BLOW::BLOW_001: m_eRefreshState = SetBlowState_001();	break;
		case STATE_BLOW::BLOW_002: m_eRefreshState = SetBlowState_002();	break;
		case STATE_BLOW::BLOW_003: m_eRefreshState = SetBlowState_003();	break;
		case STATE_BLOW::BLOW_004: m_eRefreshState = SetBlowState_004();	break;
		case STATE_BLOW::BLOW_005: m_eRefreshState = SetBlowState_005();	break;
		case STATE_BLOW::BLOW_006: m_eRefreshState = SetBlowState_006();	break;
		case STATE_BLOW::BLOW_007: m_eRefreshState = SetBlowState_007();	break;
		case STATE_BLOW::BLOW_008: m_eRefreshState = SetBlowState_008();	break;
		case STATE_BLOW::BLOW_009: m_eRefreshState = SetBlowState_009();	break;
		case STATE_BLOW::BLOW_010: m_eRefreshState = SetBlowState_010();	break;
		case STATE_BLOW::BLOW_015: m_eRefreshState = SetBlowState_015();	break;
		case STATE_BLOW::BLOW_017: m_eRefreshState = SetBlowState_017();	break;
		case STATE_BLOW::BLOW_019: m_eRefreshState = SetBlowState_019();	break;
		case STATE_BLOW::BLOW_020: m_eRefreshState = SetBlowState_020();	break;
		case STATE_BLOW::BLOW_021: m_eRefreshState = SetBlowState_021();	break;
		case STATE_BLOW::BLOW_022: m_eRefreshState = SetBlowState_022();	break;
		case STATE_BLOW::BLOW_023: m_eRefreshState = SetBlowState_023();	break;
		case STATE_BLOW::BLOW_024: m_eRefreshState = SetBlowState_024();	break;
		case STATE_BLOW::BLOW_026: m_eRefreshState = SetBlowState_026();	break;
		case STATE_BLOW::BLOW_028: m_eRefreshState = SetBlowState_028();	break;	// Special attack power absolute
		case STATE_BLOW::BLOW_029: m_eRefreshState = SetBlowState_029();	break;	// Special attack power ratio
		case STATE_BLOW::BLOW_032: m_eRefreshState = SetBlowState_032();	break;
		case STATE_BLOW::BLOW_033: m_eRefreshState = SetBlowState_033();	break;
		case STATE_BLOW::BLOW_034: m_eRefreshState = SetBlowState_034();	break;
		case STATE_BLOW::BLOW_035: m_eRefreshState = SetBlowState_035();	break;
		case STATE_BLOW::BLOW_036: m_eRefreshState = SetBlowState_036();	break;
		case STATE_BLOW::BLOW_037: m_eRefreshState = SetBlowState_037();	break;
		case STATE_BLOW::BLOW_038: m_eRefreshState = SetBlowState_038();	break;
		case STATE_BLOW::BLOW_039: m_eRefreshState = SetBlowState_039();	break;
		case STATE_BLOW::BLOW_048: m_eRefreshState = SetBlowState_048();	break;
		case STATE_BLOW::BLOW_049: m_eRefreshState = SetBlowState_049();	break;
		case STATE_BLOW::BLOW_058: m_eRefreshState = SetBlowState_058();	break;
		case STATE_BLOW::BLOW_059: m_eRefreshState = SetBlowState_059();	break;
		case STATE_BLOW::BLOW_061: m_eRefreshState = SetBlowState_061();	break;
		case STATE_BLOW::BLOW_064: m_eRefreshState = SetBlowState_064();    break;
		case STATE_BLOW::BLOW_075: m_eRefreshState = SetBlowState_075();	break;
		case STATE_BLOW::BLOW_076: m_eRefreshState = SetBlowState_076();	break;
		case STATE_BLOW::BLOW_082: m_eRefreshState = SetBlowState_082();    break;
		case STATE_BLOW::BLOW_087: m_eRefreshState = SetBlowState_087();	break;
		case STATE_BLOW::BLOW_088: m_eRefreshState = SetBlowState_088();	break;
		case STATE_BLOW::BLOW_089: m_eRefreshState = SetBlowState_089();	break;
		case STATE_BLOW::BLOW_090: m_eRefreshState = SetBlowState_090();	break;
		case STATE_BLOW::BLOW_093: m_eRefreshState = SetBlowState_093();	break;
		case STATE_BLOW::BLOW_094: m_eRefreshState = SetBlowState_094();	break;
		case STATE_BLOW::BLOW_123: m_eRefreshState = SetBlowState_123();	break;
		case STATE_BLOW::BLOW_124: m_eRefreshState = SetBlowState_124();	break;
		case STATE_BLOW::BLOW_125: m_eRefreshState = SetBlowState_125();	break;
		case STATE_BLOW::BLOW_126: m_eRefreshState = SetBlowState_126();	break;
		case STATE_BLOW::BLOW_127: m_eRefreshState = SetBlowState_127();	break;
		case STATE_BLOW::BLOW_128: m_eRefreshState = SetBlowState_128();	break;
		case STATE_BLOW::BLOW_200: m_eRefreshState = SetBlowState_200();	break;
		case STATE_BLOW::BLOW_201: m_eRefreshState = SetBlowState_201();	break;
		case STATE_BLOW::BLOW_202: m_eRefreshState = SetBlowState_202();	break;
		case STATE_BLOW::BLOW_203: m_eRefreshState = SetBlowState_203();	break;
		case STATE_BLOW::BLOW_234: m_eRefreshState = SetBlowState_234();	break;
		case STATE_BLOW::BLOW_306: m_eRefreshState = SetBlowState_306();	break;
		case STATE_BLOW::BLOW_307: m_eRefreshState = SetBlowState_307();	break;
		case STATE_BLOW::BLOW_309: m_eRefreshState = SetBlowState_309();	break;
		case STATE_BLOW::BLOW_310: m_eRefreshState = SetBlowState_310();	break;
		case STATE_BLOW::BLOW_313: m_eRefreshState = SetBlowState_313();	break;

		default:
			_ASSERT( "Missing BasicStateBlow enumeration case." );
			break;
	}

	m_State.CalcValueType();
}

void CDnBasicBlow::AddBlowState()
{
	m_eAddBlowStateType = GetAddBlowStateType();

#ifdef PRE_ADD_SKILLBUF_RENEW
	// 2012.05.03 ------------------------------------------------------------------------------------------------------------------------------------------//
	// Due to the skill revamp, buff/debuff state effects are calculated at the same stage as equipment, and subsequent state effects attached to active skills etc. are applied as PostStateEffect
	// This maximizes buff/debuff effects. This applies to monsters as well as players, so when distinguishing buff state effects 
	// Calls AddBlowState() with Equip_Buff_Level; m_eAddBlowStateType is called that way, but for monsters, bIsBuff is turned off when calling.
	// The reason is, for players, to distinguish buff/debuff BlowState so buff/debuff stats are applied after PvP scaling adjustments 
	// Internally split into buff/debuff lists, but monsters do not apply PvP adjustments in stat updates, so splitting the list is unnecessary.
	// (Rather, if split up, CDnMonsterState::CalcState() does not reference that buff/debuff list itself, so it won't be applied.)
	// Ultimately, the formula multiplies the corrected result by the buff/debuff ratio on the player side, making player-side variance slightly larger
	// This changes, but it was agreed upon with the design team and will be applied to code after future discussion on correction adjustments.
	// 2012.05.03 ------------------------------------------------------------------------------------------------------------------------------------------//

	// PlayerState is used only by players, and monsters do not go through complex calculations like CalcEquip, so apply only for players.
	bool bIsPlayerActor = m_hActor->IsPlayerActor();
	bool bIsBuff = (m_eAddBlowStateType == CDnActorState::Equip_Buff_Level);
	m_hActor->AddBlowState( &m_State, m_eAddBlowStateType, (bIsPlayerActor && bIsBuff) );
#else
	m_hActor->AddBlowState( &m_State, m_eAddBlowStateType );
#endif
}

void CDnBasicBlow::RefreshActorState()
{
	// If ST_All, SetBlowState() was not called, so do not update.
	if( m_bNeedRefreshState )
	{
		if( m_bRefreshAll )
		{
			m_hActor->RefreshState( CDnActorState::RefreshAll, m_eRefreshState );
		}
		else
		{
			m_hActor->RefreshState( CDnActorState::RefreshSkill, m_eRefreshState );
		}
	}
}


void CDnBasicBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	SetBlowState();
	AddBlowState();
	CalcRefreshTypes();
	RefreshActorState();
#ifdef _GAMESERVER
	CheckHPMPFullWhenBegin();
#endif
}

#ifdef _GAMESERVER
void CDnBasicBlow::CheckHPMPFullWhenBegin()
{
	if( m_bHPMPFullWhenBegin )
	{
		m_hActor->CmdRefreshHPSP( m_hActor->GetMaxHP(), m_hActor->GetMaxSP() );
		m_bHPMPFullWhenBegin = false;
	}
}
#endif

void CDnBasicBlow::SetModifyFloatValue(float fValue)
{
	m_fValue = fValue;

	SetBlowState();
	RefreshActorState();
}

void CDnBasicBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	DelBlowState();
	RefreshActorState();
}


void CDnBasicBlow::CalcRefreshTypes( void )
{
	m_bNeedRefreshState = (CDnActorState::ST_All != m_eRefreshState);

	// Arbitrarily set state effects have no parent skill. Account for this as base state change, not post state change.
	// If there is a parent skill, check if it is passive; if passive, treat as basic stat change with All Refresh, otherwise call RefreshSkill..
	m_bRefreshAll = false;

	if( false == m_bNeedRefreshState )
		return;
#ifdef PRE_ADD_SKILLBUF_RENEW
	switch( m_eAddBlowStateType )
	{
		// State blow that affects base stats.
		case CDnActorState::Equip_Buff_Level:
		case CDnActorState::Equip_Skill_Level:
			m_bRefreshAll = true;
			break;

		// State blow applied based on the base stat calculation result.
		case CDnActorState::Skill_Level:
			m_bRefreshAll = false;
			break;
	}
#else
	m_bRefreshAll = ( false == m_bHasParentSkill || (CDnSkill::Passive == m_ParentSkillInfo.eSkillType && CDnSkill::Buff == m_ParentSkillInfo.eDurationType) );
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
	
	// #32220 Specific state effects used in cooking are classified here so their calculation order is handled separately.
	if( STATE_BLOW::BLOW_200 == m_StateBlow.emBlowIndex || 
		STATE_BLOW::BLOW_201 == m_StateBlow.emBlowIndex ||
		STATE_BLOW::BLOW_202 == m_StateBlow.emBlowIndex ||
		STATE_BLOW::BLOW_203 == m_StateBlow.emBlowIndex )
	{
		m_bRefreshAll = true;
	}

	//The added critical increase state effect, like the state effect above, before skill state effects are calculated
	if (STATE_BLOW::BLOW_234 == m_StateBlow.emBlowIndex)
		m_bRefreshAll = true;

#ifdef PRE_ADD_SKILLBUF_RENEW
	// Due to the skill revamp, all stats must be updated even for summoned monsters.
	// In CDnMonsterState::CalcaBaseState(), m_BaseState from monster table
	// Regarding stats copied from the player being overwritten after fetching
	// Leave it as is without updating. 
#else
	if( m_hActor && m_hActor->IsMonsterActor() )
	{
		// For a summoned monster actor created by copying the summoner's stats, if all base stats are refreshed
		// Since it changes to the table's stats, only update the skill state effects.
		CDnMonsterActor* pMonstreActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
		if( pMonstreActor->GetSummonerPlayerActor() )
		{
			// Return bNeedRefreshAll -> false.
			m_bRefreshAll = false;
		}
	}
#endif // #ifdef PRE_ADD_SKILLBUF_RENEW
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnBasicBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	float fOrigValue = (float)atof(szOrigValue);
	float fAddValue = (float)atof(szAddValue);

	float fResultValue = fOrigValue + fAddValue;

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnBasicBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	static char szBuff[128] = {0, };

	float fOrigValue = (float)atof(szOrigValue);
	float fAddValue = (float)atof(szAddValue);

	float fResultValue = fOrigValue - fAddValue;

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_001()
{
	m_State.SetAttackPMin( (int)m_fValue );
	m_State.SetAttackPMax( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_AttackP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_002()
{
	m_State.SetAttackPMinRatio(m_fValue);
	m_State.SetAttackPMaxRatio(m_fValue);

	return CDnActorState::StateTypeEnum::ST_AttackP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_003()
{
	m_State.SetDefenseP( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_DefenseP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_004()
{
	m_State.SetDefensePRatio(m_fValue);

	return CDnActorState::StateTypeEnum::ST_DefenseP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_005()
{
	m_State.SetStrength( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_Strength;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_006()
{
	m_State.SetAgility( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_Agility;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_007()
{
	m_State.SetIntelligence( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_Intelligence;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_008()
{
	m_State.SetStamina( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_Stamina;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_009()
{
	INT64 iAddToMaxHP = INT64(m_fValue);

	m_State.SetMaxHP( iAddToMaxHP );

	return CDnActorState::StateTypeEnum::ST_HP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_010()
{
	m_State.SetMaxSP((int)m_fValue);

	return CDnActorState::StateTypeEnum::ST_SP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_015()
{
#ifdef _GAMESERVER
	// Just process everything at once
	INT64 iHP = m_hActor->GetHP();
	INT64 iMaxHP = m_hActor->GetMaxHP();

	if( GetParentSkillInfo() && GetParentSkillInfo()->bIsItemSkill == true && m_hActor && m_hActor->GetStateBlow() )
	{
		if( m_fValue > 0 )
		{
			float fResultValue = 1.f;
			DNVector(DnBlowHandle) vBlows;
			m_hActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_272, vBlows );
			for( int i = 0; i < (int)vBlows.size(); ++i )
			{
				fResultValue += vBlows[i]->GetFloatValue();
			}
			m_fValue *= fResultValue;
		}
	}

#if defined( _GAMESERVER )
	CalcHealValueLimit( m_StateBlow.emBlowIndex, m_fValue );
#endif

#if defined(PRE_ADD_DARKLAIR_HEAL_REGULATION) && defined(_GAMESERVER) 
	m_fValue = CalcModifiedHealValue( m_fValue );
#endif

	INT64 iDelta = INT64(m_fValue);
	INT64 iResult = iHP + iDelta;

	if( iMaxHP < iResult )
	{
		iResult = iMaxHP;
		iDelta = iMaxHP - iHP;
	}
	else
	if( iResult < 0 )
	{
		iResult = 1;
		iDelta = 1 - iHP;
	}

	m_hActor->SetHP( iResult );
	m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, m_hActor->GetUniqueID() );
#endif
	
	return CDnActorState::StateTypeEnum::ST_HP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_017()
{
	int nSP = m_hActor->GetSP();
	int nPrevSP = nSP;
	nSP += (int)m_fValue;

	if( nSP > m_hActor->GetMaxSP() )
		nSP = m_hActor->GetMaxSP();

	m_hActor->SetSP( nSP );

#ifndef _GAMESERVER
	int nAddSP = nSP - nPrevSP;
	if( nAddSP > 0 ) {
		GetInterface().SetRecovery( m_hActor->GetHeadPosition(), 0, nAddSP, false, (m_hActor == CDnActor::s_hLocalActor)  );
	}
#endif
	
	return CDnActorState::StateTypeEnum::ST_SP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_019()
{
	m_State.SetStiff( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_Stiff;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_020()
{
	m_State.SetStiffResistance( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_StiffResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_021()
{
	m_State.SetCritical( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_Critical;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_022()
{
	m_State.SetCriticalResistance( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_CriticalResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_023()
{
	m_State.SetStun( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_Stun;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_024()
{
	m_State.SetStunResistance( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_StunResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_026()
{
	m_State.SetDownDelayProb( m_fValue );
	return CDnActorState::StateTypeEnum::ST_DownDelay;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_028()
{
	m_State.SetAttackMMin( (int)m_fValue );
	m_State.SetAttackMMax( (int)m_fValue );

	return CDnActorState::StateTypeEnum::ST_AttackM;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_029()
{
	m_State.SetAttackMMinRatio( m_fValue );
	m_State.SetAttackMMaxRatio( m_fValue );

	return CDnActorState::StateTypeEnum::ST_AttackM;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_032()
{
	m_State.SetElementAttack( CDnState::Fire, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementAttack;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_033()
{
	m_State.SetElementAttack( CDnState::Ice, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementAttack;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_034()
{
	m_State.SetElementAttack( CDnState::Light, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementAttack;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_035()
{
	m_State.SetElementAttack( CDnState::Dark, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementAttack;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_036()
{
	m_State.SetElementDefense( CDnState::Fire, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementDefense;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_037()
{
	m_State.SetElementDefense( CDnState::Ice, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementDefense;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_038()
{
	m_State.SetElementDefense( CDnState::Light, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementDefense;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_039()
{
	m_State.SetElementDefense( CDnState::Dark, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementDefense;
}


// Increase skill points (used only by items with permanent effects)
CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_048()
{
#ifndef _GAMESERVER
	GetSkillTask().SetSkillPoint( GetSkillTask().GetSkillPoint()+(int)m_fValue );
#else
	// Player actor only.
	_ASSERT( m_hActor->GetActorType() <= CDnActor::ActorTypeEnum::Reserved6 );
	if( !m_hActor || m_hActor->GetActorType() > CDnActor::ActorTypeEnum::Reserved6 )
		return CDnActorState::StateTypeEnum(-1);
	
	static_cast<CDnPlayerActor*>( m_hActor.GetPointer() )->GetUserSession()->ChangeSkillPoint( (int)m_fValue, 0, false, 0 );
#endif

	return CDnActorState::StateTypeEnum(-1);
}


// EXP increase (used only for items with permanent effect)
CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_049()
{
	// Not sure what log code is, set to -1 for now
	if( m_hActor && m_hActor->IsPlayerActor() ) {
		((CDnPlayerActor*)m_hActor.GetPointer())->AddExperience( (int)m_fValue, -1, (INT64)0 );
	}

	return CDnActorState::StateTypeEnum(-1);
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_061()
{
	m_State.SetSuperAmmor( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_SuperAmmor;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_064()
{
	m_State.SetSuperAmmorRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_SuperAmmor;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_058()
{
	m_State.SetMaxHPRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_HP;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_059()
{
	m_State.SetMaxSPRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_SP;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_075()
{
	m_State.SetMoveSpeed( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_MoveSpeed;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_076()
{
	m_State.SetMoveSpeedRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_MoveSpeed;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_082()
{
	m_State.SetFinalDamageRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_FinalDamage;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_087()
{
	m_State.SetStrengthRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Strength;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_088()
{
	m_State.SetAgilityRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Agility;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_089()
{
	m_State.SetIntelligenceRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Intelligence;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_090()
{
	m_State.SetStaminaRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Stamina;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_093()
{
	m_State.SetDefenseM( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_DefenseM;
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_094()
{
	m_State.SetDefenseMRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_DefenseM;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_123()
{
	m_State.SetStiffRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Stiff;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_124()
{
	m_State.SetStiffResistanceRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_StiffResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_125()
{
	m_State.SetCriticalRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Critical;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_126()
{
	m_State.SetCriticalResistanceRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_CriticalResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_127()
{
	m_State.SetStunRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_Stun;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_128()
{
	m_State.SetStunResistanceRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_StunResistance;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_200()
{
	m_State.SetAttackPMin( (int)m_fValue );
	m_State.SetAttackPMax( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_AttackP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_201()
{
	m_State.SetAttackPMinRatio(m_fValue);
	m_State.SetAttackPMaxRatio(m_fValue);
	return CDnActorState::StateTypeEnum::ST_AttackP;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_202()
{
	m_State.SetAttackMMin( (int)m_fValue );
	m_State.SetAttackMMax( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_AttackM;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_203()
{
	m_State.SetAttackMMinRatio( m_fValue );
	m_State.SetAttackMMaxRatio( m_fValue );
	return CDnActorState::StateTypeEnum::ST_AttackM;
}

void CDnBasicBlow::DelBlowState()
{
	m_hActor->DelBlowState( &m_State );
}


CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_234()
{
	m_State.SetCritical((int)m_fValue);
	return CDnActorState::StateTypeEnum::ST_Critical;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_306()
{
	m_State.SetElementDefense( CDnState::Fire, m_fValue );
	m_State.SetElementDefense( CDnState::Ice, m_fValue );
	m_State.SetElementDefense( CDnState::Light, m_fValue );
	m_State.SetElementDefense( CDnState::Dark, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementDefense;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_307()
{
	m_State.SetElementAttack( CDnState::Fire, m_fValue );
	m_State.SetElementAttack( CDnState::Ice, m_fValue );
	m_State.SetElementAttack( CDnState::Light, m_fValue );
	m_State.SetElementAttack( CDnState::Dark, m_fValue );
	return CDnActorState::StateTypeEnum::ST_ElementAttack;
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_309()
{
	m_State.SetAttackPMin( (int)m_fValue );
	m_State.SetAttackPMax( (int)m_fValue );
	m_State.SetAttackMMin( (int)m_fValue );
	m_State.SetAttackMMax( (int)m_fValue );
	return (CDnActorState::StateTypeEnum)(CDnActorState::StateTypeEnum::ST_AttackP | CDnActorState::StateTypeEnum::ST_AttackM);
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_310()
{
	m_State.SetAttackPMinRatio( m_fValue );
	m_State.SetAttackPMaxRatio( m_fValue );
	m_State.SetAttackMMinRatio( m_fValue );
	m_State.SetAttackMMaxRatio( m_fValue );
	return (CDnActorState::StateTypeEnum)(CDnActorState::StateTypeEnum::ST_AttackP | CDnActorState::StateTypeEnum::ST_AttackM);
}

CDnActorState::StateTypeEnum CDnBasicBlow::SetBlowState_313()
{
	m_State.SetFinalDamage( (int)m_fValue );
	return CDnActorState::StateTypeEnum::ST_FinalDamage;
}