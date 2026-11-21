#pragma once
#include "DnBlow.h"

//Value Type : float
//
//Effect Index 
//- Magic Melee Damage Reflect : 113
//
//Setting: Attack concept using the physical or magic attack power of <target attacking the caster> instead of the caster's physical or magic attack power.
//	 - Magic Melee Damage Reflect : For <duration>, an attacker hitting the buffed target with a Magic&Melee attack takes Magic damage equal to <attacker Magic Attack> * <effect value>. 

class CDnMeleeMReflectDamageBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnMeleeMReflectDamageBlow >
{
private:
#ifdef _GAMESERVER
	float m_fReflectRatio;
	float m_fReflectRatioMax;
#endif // #ifdef _GAMESERVER

public:
	CDnMeleeMReflectDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnMeleeMReflectDamageBlow(void);

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
