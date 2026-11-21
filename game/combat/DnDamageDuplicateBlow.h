#pragma once
#include "DnBlow.h"


class CDnDamageDuplicateBlow : public CDnBlow, public TBoostMemoryPool< CDnDamageDuplicateBlow >
{
public:
	CDnDamageDuplicateBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDamageDuplicateBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	//virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );

	virtual bool CalcDuplicateValue( const char* szValue );

protected:
	float m_DuplicateDamage;
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
