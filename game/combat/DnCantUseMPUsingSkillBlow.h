#pragma once
#include "DnBlow.h"


// Effect Index : 78
//
// Value Type : int
//
// Setting: Disable MP-consuming skills for <duration>
//
// Usage: An opponent hit by a skill with this attribute receives a debuff preventing MP-consuming skills for <duration>.

class CDnCantUseMPUsingSkillBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnCantUseMPUsingSkillBlow >
{
private:
	

public:
	CDnCantUseMPUsingSkillBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCantUseMPUsingSkillBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
