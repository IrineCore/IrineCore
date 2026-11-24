#include "stdafx.h"
#include "DnComboDamageLimitBlow.h"
#include "DnProjectile.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

/*
This state effect is added at the first Hit signal of the corresponding skill action, and
In subsequent hit signals, set szSkipStateBlows to skip this state blow so it is not added again.
*/
CDnComboDamageLimitBlow::CDnComboDamageLimitBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_242;
	SetValue( szValue );
	m_fValue = 0.0f;

	std::string str = szValue;//"HitLimitCount;DamageLimitValue(0.1 => 10%)";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_nHitLimitCount = atoi( tokens[0].c_str() );
		m_fDamageRate = (float)atof(tokens[1].c_str());

		//Setting value error? Correction..
		if (m_nHitLimitCount == 0)
		{
			m_nHitLimitCount = -1;
			m_fDamageRate = -1.0f;
		}
	}
	else
	{
		//Prevent application if the state effect configuration is invalid.
		m_nHitLimitCount = -1;
		m_fDamageRate = -1.0f;

		OutputDebug("%s Invalid Value!!!!\n", __FUNCTION__);
	}

	m_bActivated = false;

	m_SkillStartTime = 0;

#if defined(_GAMESERVER)
	AddCallBackType(SB_ONCALCDAMAGE);
#endif // _GAMESERVER
}

CDnComboDamageLimitBlow::~CDnComboDamageLimitBlow(void)
{
}

void CDnComboDamageLimitBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}


void CDnComboDamageLimitBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s state effect start time %d\n", __FUNCTION__, m_SkillStartTime);
}

void CDnComboDamageLimitBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);

#if defined(_GAMESERVER)
	if (m_ParentSkillInfo.hSkillUser)
	{
		//End the state effect once skill use finishes and all projectiles used by the skill are gone.
		DnSkillHandle hSkill = m_ParentSkillInfo.hSkillUser->FindSkill(m_ParentSkillInfo.iSkillID);
		if (hSkill && hSkill->IsFinished() && hSkill->GetProjectileCount(m_SkillStartTime) == 0)
		{
#ifdef PRE_FIX_REMOVE_STATE_EFFECT_PACKET
#else
			m_hActor->SendRemoveStateEffectFromID(GetBlowID());
#endif
			m_StateBlow.fDurationTime = 0.0f;
			SetState(STATE_BLOW::STATE_END);
		}
	}
#endif // _GAMESERVER
}

#if defined(_GAMESERVER)
float CDnComboDamageLimitBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	float fDamage = 0.0f;

	//Check if hit is from a projectile...
	LOCAL_TIME projectileSkillStartTime = 0;
	CDnProjectile* pProjectile = NULL;
	if (HitParam.bFromProjectile && HitParam.hWeapon)
		pProjectile = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() );

	//The projectile's skill start time and the state effect's skill start time must match..
	if (pProjectile)
		projectileSkillStartTime = pProjectile->GetSkillStartTime();

	//Check if the skill user and current Hitter are the same.
	bool bSameHitter = false;
	if (m_ParentSkillInfo.hSkillUser && HitParam.hHitter &&
		m_ParentSkillInfo.hSkillUser == HitParam.hHitter &&
		m_SkillStartTime == projectileSkillStartTime)
		bSameHitter = true;

	// Calculate damage adjustment when m_bActivated is enabled and it is the same Hitter
	if (m_bActivated == true && bSameHitter)
	{
		//Case where original damage is 100 and damage limit is 0.3 (30%)
		//100 - (1.0 - 0.3) = 70 
		//Apply -70 damage adjustment so only 30 damage is dealt.
		fDamage = -(fOriginalDamage * (1.0f - m_fDamageRate));

		//Do not apply if setting value is invalid.
		if (m_nHitLimitCount == -1 && m_fDamageRate == -1.0f)
			fDamage = 0.0f;
	}

	return fDamage;
}

void CDnComboDamageLimitBlow::UpdateHitCount(CDnSkill::SkillInfo* pSkillInfo, CDnDamageBase::SHitParam& HitParam)
{
	if (pSkillInfo)
	{
		//Check if hit is from a projectile...
		LOCAL_TIME projectileSkillStartTime = 0;
		CDnProjectile* pProjectile = NULL;
		if (HitParam.bFromProjectile && HitParam.hWeapon)
			pProjectile = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() );
		
		//The projectile's skill start time and the state effect's skill start time must match..
		if (pProjectile)
			projectileSkillStartTime = pProjectile->GetSkillStartTime();

		//Decrease HitCount only when both skill user and skill are the same
		if (pSkillInfo->hSkillUser && pSkillInfo->hSkillUser == m_ParentSkillInfo.hSkillUser &&
			pSkillInfo->iSkillID == m_ParentSkillInfo.iSkillID &&
			pSkillInfo->iLevel == m_ParentSkillInfo.iLevel &&
			m_SkillStartTime == projectileSkillStartTime)
		{
			if (m_bActivated == false && m_nHitLimitCount >= 0)
			{
				m_nHitLimitCount--;

				if (m_nHitLimitCount <= 0)
				{
					m_bActivated = true;
					m_nHitLimitCount = 0;
				}
			}
		}
	}
}

#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnComboDamageLimitBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnComboDamageLimitBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
