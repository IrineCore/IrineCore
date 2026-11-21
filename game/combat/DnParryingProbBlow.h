#pragma once
#include "DnBlow.h"


//Effect Index : 56
//
//Value Type : float
//
//Setting: Increase parry rate by <effect value> for <duration>
//
//	 Usage: Increases parry rate via switching to a specific stance, buff, or passive skill


class CDnParryingProbBlow : public CDnBlow, public TBoostMemoryPool< CDnParryingProbBlow >
{
private:
	

public:
	CDnParryingProbBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnParryingProbBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
