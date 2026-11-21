#pragma once
#include "DnBlow.h"

// Issue #12200
// Due to the stun chance stat cap (35% for stun), the [Force Stun] skill cannot be set as a skill effect (which makes all current stun skills broken).
// Setting forced stun in action tool would stun boss monsters unconditionally, making the skill too strong. So handle it as below.
// Add a skill effect that stuns the target with the probability configured in the effect value, regardless of the caster's stun rate stat.
class CDnForceStunBlow : public CDnBlow,
						 public TBoostMemoryPool< CDnForceStunBlow >
{
private:

public:
	CDnForceStunBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnForceStunBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
