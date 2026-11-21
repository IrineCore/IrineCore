#pragma once
#include "DnBlow.h"


//Modify healing ratio (HealingAdd)
//- Skill Effect ID : 141
//- Value unit : float
//- Description 
//1) When recovering HP via healing actions (potions, healing skills, etc.), increases heal amount by <effect ratio>.

class CDnChangeHealingRatioBlow : public CDnBlow, 
								   public TBoostMemoryPool< CDnChangeHealingRatioBlow >
{
private:
	
public:
	CDnChangeHealingRatioBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeHealingRatioBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:
	virtual void OnSetParentSkillInfo();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
