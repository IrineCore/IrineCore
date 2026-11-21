#pragma once
#include "DnBlow.h"

// Issue #19619
// For skills that apply state effects using a zero-damage hit signal, like Time Restriction
// May or may not apply depending on target's CanHit state. 
// This state effect checks CanHit state in CDnActor::IsHittable() of the subject having this state effect 
// Ignore and unconditionally treat as true.
class CDnIgnoreCanHitBlow : public CDnBlow,
							public TBoostMemoryPool< CDnIgnoreCanHitBlow >
{
private:

public:
	CDnIgnoreCanHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnIgnoreCanHitBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

};
