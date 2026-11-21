#pragma once
#include "DnBlow.h"

// Immune to the configured state effect. Used only on the game server.. 
// Immune state effects are checked in DnActor::CmdAddStateEffect so no packet is sent...
class CDnAllImmuneBlow : public CDnBlow, public TBoostMemoryPool< CDnAllImmuneBlow >
{
private:
	float m_fResistPercent;
	set<int> m_setExceptionBlowIndices;			// Immune to all except indices listed here.

public:
	CDnAllImmuneBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAllImmuneBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDuplicate( const STATE_BLOW& StateBlowInfo );

	bool IsImmuned( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX BlowIndex );

	bool IsImmuned( STATE_BLOW::emBLOW_INDEX BlowIndex );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
