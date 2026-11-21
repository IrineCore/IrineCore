#pragma once
#include "DnBlow.h"

//If the target being hit has this state effect, increases the attacker's critical rate.
class CDnFinalCriticalBlow : public CDnBlow, public TBoostMemoryPool< CDnFinalCriticalBlow >
{
public:
	CDnFinalCriticalBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFinalCriticalBlow(void);
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
