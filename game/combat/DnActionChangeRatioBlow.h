#pragma once
#include "DnBlow.h"


class CDnActionChangeRatioBlow : public CDnBlow, public TBoostMemoryPool< CDnActionChangeRatioBlow >
{
public:
	CDnActionChangeRatioBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnActionChangeRatioBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	//float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

	void OnChangeAction();
#endif // _GAMESERVER

protected:
	float m_fRate;	//Probability
	std::string m_strActionName; //Action

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
