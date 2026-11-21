#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnAddDamageOnCriticalBlow : public CDnBlow, public TBoostMemoryPool< CDnAddDamageOnCriticalBlow >
{
public:
	CDnAddDamageOnCriticalBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddDamageOnCriticalBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#else
	virtual bool OnCustomIntervalProcess( void );
	void ShowGraphicEffect(bool bShow);
#endif

protected:
	float m_fAddDamageRate;
	float m_fRate;

#if defined(_GAMESERVER)
	//////////////////////////////////////////////////////////////////////////
	// Handling to prevent per-actor probability calculation.
	bool m_bCalcProb;	//Whether to calculate probability
	bool m_bOnDamaged;	//Check if hit.
	
	bool CalcProb();
	//////////////////////////////////////////////////////////////////////////
#else
	CDnActorStatIntervalManipulator	m_IntervalChecker;
	bool m_bGraphicEffectShow;
	bool m_bTrigger;
#endif // _GAMESERVER
	
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
