#pragma once
#include "DnBlow.h"

// Aggro adjustment state effect. 
// Polls from the monster aggro system and adjusts aggro by the value specified in the parameter.
// Argument value is directly multiplied (1.0 means unchanged).
class CDnAggroBlow : public CDnBlow, 
					 public TBoostMemoryPool< CDnAggroBlow >
{
private:
	float m_fRange;
	float m_fRangeSQ;
	float m_fRangeAngleByRadian;
	DNVector(DnActorHandle) m_vlhApplyMonsters;

public:
	CDnAggroBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAggroBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
