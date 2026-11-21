#pragma once
#include "DnBlow.h"

class CDnDisarmamentBlow : public CDnBlow, public TBoostMemoryPool< CDnDisarmamentBlow >
{
public:
	CDnDisarmamentBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDisarmamentBlow(void);

private:

	static DnWeaponHandle ms_hOrigWeapon;	// Existing weapon
	static bool ms_bSelefDelete;

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
