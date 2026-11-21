#pragma once
#include "DnBlow.h"

class CDnChangeMAttackDamageBlow : public CDnBlow,
								   public TBoostMemoryPool< CDnChangeMAttackDamageBlow >
{
private:

public:
	CDnChangeMAttackDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeMAttackDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	// When magic damage modification state effects stack, values are stored in 1.2 format, so
	// From the second one onward, subtract 1 before adding.
	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
