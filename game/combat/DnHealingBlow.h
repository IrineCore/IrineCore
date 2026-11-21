#pragma once
#include "DnBlow.h"


//Healing
//- Skill Effect ID : 140
//- Value unit : float
//- Description 
//1) Restores target's HP by caster's <magic attack power> * <effect ratio>.

class CDnHealingBlow : public CDnBlow, 
					   public TBoostMemoryPool< CDnHealingBlow >
{
private:
	

public:
	CDnHealingBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnHealingBlow(void);

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
