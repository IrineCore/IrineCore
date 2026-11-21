#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnContinueBaseMPIncBlow : public CDnBlow, public TBoostMemoryPool< CDnContinueBaseMPIncBlow >
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;

	bool m_bShowValue;


public:
	CDnContinueBaseMPIncBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnContinueBaseMPIncBlow(void);

	virtual float GetStatDelta( void ) { return m_fValue; };

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	virtual bool OnCustomIntervalProcess( void );

#if defined(PRE_FIX_46381)
	float GetMPIncValue();
#endif // PRE_FIX_46381

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
