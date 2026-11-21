#pragma once
#include "DnBlow.h"


// Related to #33316: physical attack final damage ratio state effect. 0.3 means +30%.
class CDnFinalPDamageBlow : public CDnBlow, public TBoostMemoryPool< CDnFinalPDamageBlow >
{
private:
	

public:
	CDnFinalPDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFinalPDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
