#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


// Continuous HP increase flat value - #11
class CDnAbContinueHPIncBlow : public CDnBlow, public TBoostMemoryPool< CDnAbContinueHPIncBlow >
{
private:
	CDnActorStatIntervalManipulator m_IntervalChecker;

	bool m_bShowValue;

#if defined(PRE_ADD_56253)
	float m_fLimitMinValue;		//Minimum HP limit (terminates this state effect skill if HP falls outside this limit.)
#endif // PRE_ADD_56253

public:
	CDnAbContinueHPIncBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAbContinueHPIncBlow(void);

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

protected:
	virtual void OnSetParentSkillInfo();
};
