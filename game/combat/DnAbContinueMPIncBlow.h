#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

// Continuous SP increase flat value - #12
class CDnAbContinueMPIncBlow : public CDnBlow, public TBoostMemoryPool< CDnAbContinueMPIncBlow >
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;

	bool m_bShowValue;


public:
	CDnAbContinueMPIncBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAbContinueMPIncBlow(void);

	virtual float GetStatDelta( void ) { return m_fValue; };

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
