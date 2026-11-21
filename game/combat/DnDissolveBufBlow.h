#pragma once
#include "DnBlow.h"


// State effect that dispels state effects applied by buff skills. 
class CDnDissolveBufBlow : public CDnBlow, public TBoostMemoryPool< CDnDissolveBufBlow >
{
protected:


public:
	CDnDissolveBufBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDissolveBufBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
