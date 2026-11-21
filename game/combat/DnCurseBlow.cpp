#include "stdafx.h"
#include "DnCurseBlow.h"
#include "DnSkill.h"

#if defined(_GAMESERVER)
#include "DnMonsterActor.h"
#endif // _GAMESERVER


#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnCurseBlow::CDnCurseBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_244;
	SetValue( szValue );
	m_fValue = 0.0f;
	
	m_fDamageRate = 0.0f;
	m_fDamage = 0.0f;
	m_fDelayTime = 0.0f;
	
	m_fCoolTime = 0.0f;
	m_bAtivate = false;
	m_nDamage = 0;

	std::string str = szValue;//"Damage%;Damage;DelayTime";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if( 3 == tokens.size() )
	{
		m_fDamageRate = (float)atof(tokens[0].c_str());
		m_fDamage = (float)atof( tokens[1].c_str() );
		m_fDelayTime = (float)atof(tokens[2].c_str()) * 0.001f;
	}
#if defined(_GAMESERVER)
	else if( 4 == tokens.size() )
	{
		m_fDamageRate = (float)atof(tokens[0].c_str());
		m_fDamage = (float)atof( tokens[1].c_str() );
		m_fDelayTime = (float)atof(tokens[2].c_str()) * 0.001f;
		m_fCoolTime = (float)atof( tokens[3].c_str() );

		std::string::size_type delimiterindex = str.rfind( delimiters.c_str() );
		if( delimiterindex != std::string::npos )
		{
			str.erase( delimiterindex, str.length() - delimiterindex );
			SetValue( str.c_str() );
		}
	}
#endif // _GAMESERVER
	else
	{
		OutputDebug("%s InvalidValue %s\n", __FUNCTION__, szValue);
	}
}

CDnCurseBlow::~CDnCurseBlow(void)
{
}

void CDnCurseBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug("%s \n", __FUNCTION__);
}

void CDnCurseBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
	if (m_fCoolTime > 0.0f)
	{
		m_fCoolTime -= fDelta;

		if (m_fCoolTime < 0.0f)
			m_fCoolTime = 0.0f;
	}
}

void CDnCurseBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug("%s \n", __FUNCTION__);
}

#if defined(_GAMESERVER)
bool CDnCurseBlow::CalcDuplicateValue( const char* szValue )
{
	float fValue = 0.0;
	
	std::string str = szValue;//"Damage%;Damage;DelayTime";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	float fDamageRate = 0.0f;
	float fDamage = 0.0f;
	float fDelayTime = 0.0f;

	TokenizeA(str, tokens, delimiters);
	if (3 == tokens.size())
	{
		fDamageRate = (float)atof(tokens[0].c_str());
		fDamage = (float)atof( tokens[1].c_str() );
		fDelayTime = (float)atof(tokens[2].c_str());
	}
	else
	{
		OutputDebug("%s InvalidValue %s\n", __FUNCTION__, szValue);
	}
	
	//Add damage only??
	m_fDamageRate += fDamageRate;
	m_fDamage += fDamage;

	return true;
}

void CDnCurseBlow::OnAttackChange()
{
	if (m_fCoolTime <= 0.0f)
	{
		m_bAtivate = true;
		m_fCoolTime = m_fDelayTime;

		m_nDamage = CalcDamage();
		RequestDamage();
	}
	else
	{
		m_bAtivate = false;
		m_nDamage = 0;
	}
}

void CDnCurseBlow::RequestDamage()
{
	if (m_nDamage > 0)
	{
#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if (hActor && hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), m_nDamage);

			if (hActor->IsDie())
				hActor->Die(m_hSkillUser);
		}
#else
		if (m_hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), m_nDamage);

			if( m_hActor->IsDie())
				m_hActor->Die( m_hSkillUser );
		}
#endif // PRE_FIX_61382
	}
}

int CDnCurseBlow::CalcDamage()
{
	int nDamage = 0;
	
	//Skill caster's magic attack power
	if (m_hSkillUser)
	{
		//Min/Max magic attack power
		float fAttackPower = 0.0f;
		float fAttackMaxPower = (float)m_hSkillUser->GetAttackMMaxWithoutSkill();
		float fAttackMinPower = (float)m_hSkillUser->GetAttackMMinWithoutSkill();
		fAttackPower = (fAttackMinPower + fAttackMaxPower) * 0.5f;

		nDamage = (int)((fAttackPower * m_fDamageRate) + m_fDamage);
	}

	return nDamage;
}

//#59938
//For Soul Gate, the skill user in projectile firing is set to Soul Gate itself.
//Even if damage is set in the curse state effect after Soul Gate disappears, Soul Gate is gone so
//Issue where damage is not displayed.
//Fixed to change skill user to owner actor if it is a summoned monster when setting curse state effect.
void CDnCurseBlow::OnSetParentSkillInfo()
{
	//Skill caster's magic attack power
	if (m_ParentSkillInfo.hSkillUser)
	{
		m_hSkillUser = m_ParentSkillInfo.hSkillUser;

		if (m_hSkillUser->IsMonsterActor())
		{
			CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>(m_hSkillUser.GetPointer());
			if (pMonsterActor && pMonsterActor->IsSummonedMonster())
				m_hSkillUser = pMonsterActor->GetSummonerPlayerActor();
		}

		m_ParentSkillInfo.hSkillUser = m_hSkillUser;
	}
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnCurseBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnCurseBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;	
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
