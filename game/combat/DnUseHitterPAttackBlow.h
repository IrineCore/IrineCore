#pragma once
#include "DnBlow.h"

//Value Type : float
//
//Effect Index
//- Apply Target Physical Attack Damage : 109
//
//Setting: Attack concept using the physical or magic attack power of the actual <target receiving the effect> instead of the caster's physical or magic attack power
//	 - Apply Target Physical Attack Damage : Deals Physical damage with Physical Attack equal to <affected target's Physical Attack> * <effect value> + <caster's Physical Attack> 
//
//	 Usage
//	 - Increases damage based on the Physical/Magic Attack of the target hit by this skill

class CDnUseHitterPAttackBlow : public CDnBlow, 
								public TBoostMemoryPool< CDnUseHitterPAttackBlow >
{
private:
	

public:
	CDnUseHitterPAttackBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnUseHitterPAttackBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
