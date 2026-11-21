#pragma once
#include "DnBlow.h"


class CDnDamageTransitionBlow : public CDnBlow, public TBoostMemoryPool< CDnDamageTransitionBlow >
{
public:
	CDnDamageTransitionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDamageTransitionBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

	void DamageTransition(int nDamage);
#endif // _GAMESERVER

protected:
	float m_fRate;	//Probability
	int m_nDamage;	//Damage to return

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
