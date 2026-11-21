#pragma once
#include "DnBlow.h"

class CDnChangePAttackDamageBlow : public CDnBlow,
								   public TBoostMemoryPool< CDnChangePAttackDamageBlow >
{
public:
	CDnChangePAttackDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangePAttackDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	// When physical damage modification state effects overlap, since the value is stored in a format like 1.2
	// From the second one onward, subtract 1 before adding.
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
