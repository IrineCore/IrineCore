#pragma once
#include "DnBlow.h"


// During the duration, change the target actor's weight value to the specified value.
class CDnChangeWeightBlow : public CDnBlow, public TBoostMemoryPool< CDnChangeWeightBlow >
{
private:
	float m_fWeightDelta;

public:
	CDnChangeWeightBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeWeightBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
