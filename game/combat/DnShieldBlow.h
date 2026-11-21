#pragma once
#include "DnBlow.h"

class CDnShieldBlow : public CDnBlow, public TBoostMemoryPool< CDnShieldBlow >
{
private:
	int m_iDurability;				// Durability. State effect is removed when depleted.
	bool m_bCalcDamage;				// Flag for whether to calculate damage correction in OnCalcDamage
public:
	CDnShieldBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnShieldBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
