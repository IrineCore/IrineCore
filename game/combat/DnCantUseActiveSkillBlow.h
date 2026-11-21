#pragma once
#include "DnBlow.h"


// Effect Index : 67

class CDnCantUseActiveSkillBlow : public CDnBlow, public TBoostMemoryPool< CDnCantUseActiveSkillBlow >
{
private:
	

public:
	CDnCantUseActiveSkillBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCantUseActiveSkillBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
