#pragma once
#include "DnBlow.h"

//
//Value Type : float
//
//Effect Index
//- Apply Target Magic Attack Damage : 110
//
//Setting: Attack concept using the physical or magic attack power of the actual <target receiving the effect> instead of the caster's physical or magic attack power
//	 - Apply Target Magic Attack Damage : Deals Magic damage with Magic Attack equal to <affected target's Magic Attack> * <effect value> + <caster's Magic Attack>
//
//	 Usage
//	 - Increases damage based on the Physical/Magic Attack of the target hit by this skill

class CDnUseHitterMAttackBlow : public CDnBlow, 
								public TBoostMemoryPool< CDnUseHitterMAttackBlow >
{
private:
	

public:
	CDnUseHitterMAttackBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnUseHitterMAttackBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
