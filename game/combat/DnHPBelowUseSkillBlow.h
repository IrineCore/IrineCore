#pragma once
#include "DnBlow.h"


// Skill effect ID: 281
// Activates a designated skill when HP drops below a specific value

#ifdef PRE_ADD_HPBELOWDOSKILLBLOW

class CDnHPBelowUseSkillBlow : public CDnBlow, public TBoostMemoryPool< CDnHPBelowUseSkillBlow >
{
public:
	CDnHPBelowUseSkillBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnHPBelowUseSkillBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	float CalcDamage( float fOriginalDamage );
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

private:
	INT64 m_nHP;
	int m_nSkillIndex;
};

#endif // PRE_ADD_HPBELOWDOSKILLBLOW