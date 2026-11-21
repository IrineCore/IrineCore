#pragma once
#include "DnBlow.h"


class CDnConfuseBlow : public CDnBlow, public TBoostMemoryPool< CDnConfuseBlow >
{
public:
	CDnConfuseBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnConfuseBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:
	int m_OrigInverseInputMode;	//Save the original InverseMode value.

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
