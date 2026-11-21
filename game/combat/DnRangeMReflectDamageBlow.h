#pragma once
#include "DnBlow.h"

//Value Type : float
//
//Effect Index 
//- Magic Ranged Damage Reflect : 114
//
//Setting: Attack concept using the physical or magic attack power of <target attacking the caster> instead of the caster's physical or magic attack power.
//	 - Magic Ranged Damage Reflect : For <duration>, an attacker hitting the buffed target with a Magic&Ranged attack takes Magic damage equal to <attacker Magic Attack> * <effect value>.

class CDnRangeMReflectDamageBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnRangeMReflectDamageBlow >
{
private:
#ifdef _GAMESERVER
	float m_fReflectRatio;
	float m_fReflectRatioMax;
#endif // #ifdef _GAMESERVER

public:
	CDnRangeMReflectDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnRangeMReflectDamageBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual bool OnDefenseAttack( DnActorHandle hActor, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
