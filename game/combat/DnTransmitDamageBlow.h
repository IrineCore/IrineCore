#pragma once
#include "DnBlow.h"

//Forward damage to the owner actor
class CDnTransmitDamageBlow: public CDnBlow, 
								 public TBoostMemoryPool< CDnTransmitDamageBlow >
{
public:
	CDnTransmitDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnTransmitDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	//When a damage-modifying state effect such as Freezing is applied together
	//Final damage may vary depending on the calculation order.
	//If state effect #51 is present, forward final damage to the owner actor.
#if defined(_GAMESERVER)
#if defined(PRE_FIX_61382)
	void TransmitDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#else
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif // PRE_FIX_61382
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
