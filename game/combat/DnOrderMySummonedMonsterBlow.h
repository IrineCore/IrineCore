#pragma once
#include "DnBlow.h"

// Related to #32426. Instructs monsters summoned by me to use a specific skill 
// State effect linked with the OrderMySummonedMonster signal.
// Holds skill and skill level information for the monster to use.
// Not everything was put into the signal for convenience, since separate actions would have to be made per skill level
// Because the design team requested it.
// Used only on game server.
class CDnOrderMySummonedMonsterBlow : public CDnBlow, public TBoostMemoryPool< CDnOrderMySummonedMonsterBlow >
{
private:
	int m_iSkillID;

public:
	CDnOrderMySummonedMonsterBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnOrderMySummonedMonsterBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	int GetSkillID( void ) { return m_iSkillID; };

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif

};
