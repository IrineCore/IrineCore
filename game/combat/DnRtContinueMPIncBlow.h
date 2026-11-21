#pragma once
#include "DnBlow.h"
#ifdef _GAMESERVER
#include "DnActorStatIntervalManipulator.h"
#endif

//Continuous MP increase ratio
//Increases current MP by <effect value * max MP> every 2 seconds during the effect duration.
//Does not change above max MP.
class CDnRtContinueMPIncBlow : public CDnBlow, public TBoostMemoryPool< CDnRtContinueMPIncBlow >
{
private:
#ifdef _GAMESERVER
	CDnActorStatIntervalManipulator m_IntervalChecker;
#endif
	bool m_bShowValue;


public:
	CDnRtContinueMPIncBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnRtContinueMPIncBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual bool OnCustomIntervalProcess( void );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
