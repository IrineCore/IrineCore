#pragma once
#include "DnBlow.h"


//Super armor attack power ratio
//- Skill Effect ID : 142
//- Value unit : float
//- Description 
//1) For <duration>, increases the target's attack power ratio against Super Armor 1 by <effect ratio>.
class CDnSuperArmorAttackRatioBlow : public CDnBlow, public TBoostMemoryPool< CDnSuperArmorAttackRatioBlow >
{
private:
	

public:
	CDnSuperArmorAttackRatioBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSuperArmorAttackRatioBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
