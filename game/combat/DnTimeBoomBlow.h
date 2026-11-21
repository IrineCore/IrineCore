#pragma once
#include "DnBlow.h"


class CDnTimeBoomBlow : public CDnBlow, public TBoostMemoryPool< CDnTimeBoomBlow >
{
private:
	float m_fLimitRadius;
	int m_nBoomDamage;

public:
	CDnTimeBoomBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnTimeBoomBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
