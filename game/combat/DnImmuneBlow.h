#pragma once
#include "DnBlow.h"

// Immune to the configured state effect. Used only on the game server.. 
// Immune state effects are checked in DnActor::CmdAddStateEffect so no packet is sent...
class CDnImmuneBlow : public CDnBlow, public TBoostMemoryPool< CDnImmuneBlow >
{
private:
	float m_fResistPercent;

	float m_fReduceTimeValue;	//Duration reduction ratio value (0 ~ 1)

public:
	CDnImmuneBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnImmuneBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

	bool IsImmuned( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX BlowIndex );

	bool IsImmuned( STATE_BLOW::emBLOW_INDEX BlowIndex );

	float GetReduceTimeValue() { return m_fReduceTimeValue; }

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
