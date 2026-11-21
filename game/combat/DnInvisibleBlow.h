#pragma once
#include "DnBlow.h"

// Invisibility state effect
// Effect index: 73
// Effect value: Not numeric
// Effect details:
// Makes the caster invisible (alpha 100%) during the effect duration.
// The invisibility effect disappears when the duration expires or when hit by a buff-removing effect.
// While invisible, performing any action other than moving or Stand (attacking, using skills, using items) makes the character appear semi-transparent (about 50% alpha) for the duration of that action, then turns invisible again once the action ends.
// When hit while invisible, the character becomes semi-transparent just like when attacking, and becomes invisible again after the hitstun motion ends.
// As an exception while invisible, if affected by a [Debuff], the [Status Effect VFX] for that debuff does not turn invisible and remains visible. (All other status effect VFX become invisible together.)
// While invisible, [Shadow] is not rendered.
// Even while invisible, all related sounds are still played.
// When the caster dies, all buffs/debuffs are removed, so invisibility also disappears.
class CDnInvisibleBlow : public CDnBlow,
						 public TBoostMemoryPool<CDnInvisibleBlow>
{
private:
#ifndef _GAMESERVER
	float m_fDestAlpha;
	float m_fNowAlpha;
	bool m_bEnded;
#endif

public:
	CDnInvisibleBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnInvisibleBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
	virtual bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
