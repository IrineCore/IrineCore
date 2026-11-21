#pragma once
#include "DnBlow.h"

// Value Type : float
//
// Effect Index 
// - Physical Ranged Damage Reflect : 112
//
// - Physical Ranged Damage Reflect : For <duration>, an attacker hitting the buffed target with a Physical&Ranged attack takes Physical damage equal to <attacker Physical Attack> * <effect value>. 

class CDnRangePReflectDamageBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnRangePReflectDamageBlow >
{
private:
#ifdef _GAMESERVER
	float m_fReflectRatio;
	float m_fReflectRatioMax;
#endif // #ifdef _GAMESERVER

public:
	CDnRangePReflectDamageBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnRangePReflectDamageBlow(void);

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
