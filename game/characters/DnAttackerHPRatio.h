#pragma once
#include "DnBlow.h"

//Attacker HP ratio
//Effect index: 92
//Effect value: Float
//Effect details:
//During the effect duration (or while the action runs if duration is 0), increases/decreases the current HP of any target attacking the caster by the specified value.
//Effect calculation: attacker max HP * effect value
//Increases if the effect value is [+], decreases if [-].
//e.g. If the effect value is 0.1, each successful attack increases the attacker's current HP by 10% of max HP.
//If the effect value is -0.1, the attacker's current HP decreases by 10% of max HP on each successful hit.
//Applies only the input value regardless of the attacker's attack power, damage, etc.
class CDnAttackerHPRatio : public CDnBlow
{
public:
	CDnAttackerHPRatio( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAttackerHPRatio(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

protected:
	virtual void OnSetParentSkillInfo();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
