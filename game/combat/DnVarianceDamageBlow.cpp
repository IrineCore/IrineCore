#include "stdafx.h"
#include "DnVarianceDamageBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnVarianceDamgeBlow::CDnVarianceDamgeBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_236;
	SetValue( szValue );
	m_fValue = 0.0f;

	m_MaxDamage = 0;
	m_nLimitMaxCount = 0;
	m_nApplyType = 0;
	
	m_nHitActorCount = 0;

	std::string str = szValue;
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);
	if (tokens.size() == 3)
	{
		m_MaxDamage = atoi(tokens[0].c_str());
		m_nLimitMaxCount = atoi(tokens[1].c_str());
		m_nApplyType = atoi(tokens[2].c_str());
	}
	else
	{
		OutputDebug("%s Invalid Value[%s]\n", __FUNCTION__, szValue);
	}
}

CDnVarianceDamgeBlow::~CDnVarianceDamgeBlow(void)
{
}

#if defined(_GAMESERVER)
void CDnVarianceDamgeBlow::OnSetParentSkillInfo()
{
#if defined(PRE_ADD_50903)
	DnSkillHandle hSkill;
	if (m_ParentSkillInfo.hSkillUser)
		hSkill = m_ParentSkillInfo.hSkillUser->FindSkill(m_ParentSkillInfo.iSkillID);

	if (hSkill)
		m_nHitActorCount = hSkill->GetHitCountForVarianceDamage();
#endif // PRE_ADD_50903
}
#endif // _GAMESERVER

void CDnVarianceDamgeBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);

#if defined(_GAMESERVER)
	//In projectiles, the hit count is not properly set at the time OnSetParentSkillInfo is called.
	//If hit count is 0, call the function again to set the hit count info.
	if (m_nHitActorCount == 0)
		OnSetParentSkillInfo();

	ApplyDamage(m_nHitActorCount);
#endif // _GAMESERVER
}

#if defined(_GAMESERVER)
void CDnVarianceDamgeBlow::ApplyDamage(int nHitCount)
{
	int nApplyDamage = m_MaxDamage;

	switch(m_nApplyType)
	{
	case DIVIDE_DAMAGE:
		{
			if (nHitCount == 0)
				nApplyDamage = (int)m_MaxDamage;
			else
				nApplyDamage = (int)((float)((float)m_MaxDamage / (float)nHitCount));
		}
		break;
	case ZERO_DAMAGE:
		{
			//Set damage to 0 if there are more people than configured
			if (nHitCount >= m_nLimitMaxCount)
				nApplyDamage = 0;
			else
				nApplyDamage = m_MaxDamage * nHitCount;
		}
		break;
	case INC_DAMAGE:
		{
			if (nHitCount > m_nLimitMaxCount)
				nApplyDamage = (nHitCount * m_MaxDamage);
			else
				nApplyDamage = 0;
		}
		break;
	}

#if defined(PRE_FIX_61382)
	DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
	if (hActor &&
		hActor->IsDie() == false)
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), nApplyDamage);

		if (hActor->IsDie())
			hActor->Die(m_ParentSkillInfo.hSkillUser);
	}
#else
	if (m_hActor->IsDie() == false)
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), nApplyDamage);

		if( m_hActor->IsDie())
			m_hActor->Die( m_ParentSkillInfo.hSkillUser	);
	}
#endif // PRE_FIX_61382
}
#endif // _GAMESERVER


void CDnVarianceDamgeBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnVarianceDamgeBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnVarianceDamgeBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
