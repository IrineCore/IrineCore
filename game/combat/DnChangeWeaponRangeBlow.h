#pragma once
#include "DnBlow.h"

class CDnChangeWeaponRangeBlow : public CDnBlow, public TBoostMemoryPool< CDnChangeWeaponRangeBlow >
{
private:
	float m_fIncreaseRange;
	DnWeaponHandle m_hAppliedWeapon;

public:
	CDnChangeWeaponRangeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeWeaponRangeBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
