#pragma once
#include "DnBlow.h"


class CDnMPBurstBlow : public CDnBlow, public TBoostMemoryPool< CDnMPBurstBlow >
{
public:
	CDnMPBurstBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnMPBurstBlow(void);
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:

#if defined(_GAMESERVER)
	float m_fBurstRatio;
	float m_fBurstConditionLimit;
	void SetBlowInfo();
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

};
