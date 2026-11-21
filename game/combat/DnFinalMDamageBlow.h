#pragma once
#include "DnBlow.h"


// Related to #33316: magic attack final damage ratio state effect. 0.3 means +30%.
class CDnFinalMDamageBlow : public CDnBlow, public TBoostMemoryPool< CDnFinalMDamageBlow >
{
private:


public:
	CDnFinalMDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFinalMDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
