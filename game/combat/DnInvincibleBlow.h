#pragma once
#include "DnBlow.h"


// Invincible state effect. Immune to all attacks, skills, and state effects
class CDnInvincibleBlow : public CDnBlow, public TBoostMemoryPool< CDnInvincibleBlow >
{
private:
	

public:
	CDnInvincibleBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnInvincibleBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
