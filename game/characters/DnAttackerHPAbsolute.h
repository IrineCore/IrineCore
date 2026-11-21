#pragma once
#include "DnBlow.h"

// Attacker HP absolute
// Effect index: 91
// Effect value: Int
// Effect details:
// During the effect duration (or while the action runs if duration is 0), increases/decreases the current HP of any target attacking the caster by the specified value.
// Increases if the effect value is [+], decreases if [-].
// Applies only the input value regardless of the attacker's attack power, damage, etc.
class CDnAttackerHPAbsolute : public CDnBlow
{
private:
	

public:
	CDnAttackerHPAbsolute( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAttackerHPAbsolute(void);

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
