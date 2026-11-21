#pragma once
#include "DnBlow.h"


class CDnBoneCrushBlow : public CDnBlow, public TBoostMemoryPool< CDnBoneCrushBlow >
{
public:
	CDnBoneCrushBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnBoneCrushBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	//virtual void OnTargetHit( DnActorHandle hTargetActor );
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

protected:
	int m_nAddDamage;	//Additional damage
	bool m_bOnDamaged;		//For checking if hit

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
