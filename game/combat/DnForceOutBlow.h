#pragma once
#include "DnBlow.h"


class CDnForceOutBlow : public CDnBlow, public TBoostMemoryPool< CDnForceOutBlow >
{
public:
	CDnForceOutBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnForceOutBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
