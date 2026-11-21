#pragma once
#include "DnBlow.h"

//Value Type : float
//
//Effect Index 
//- Physical Melee Damage Reflect : 111
//
//Setting: Attack concept using the physical or magic attack power of <target attacking the caster> instead of the caster's physical or magic attack power.
//	 - Physical Melee Damage Reflect : For <duration>, an attacker hitting the buffed target with a Physical&Melee attack takes Physical damage equal to <attacker Physical Attack> * <effect value>. 

class CDnMeleePReflectDamageBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnMeleePReflectDamageBlow >
{
private:
#ifdef _GAMESERVER
	float m_fReflectRatio;
	float m_fReflectRatioMax;
#endif // #ifdef _GAMESERVER	

public:
	CDnMeleePReflectDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnMeleePReflectDamageBlow(void);

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
