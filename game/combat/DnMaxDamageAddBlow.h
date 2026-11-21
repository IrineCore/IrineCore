#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnMaxDamageAddBlow : public CDnBlow, public TBoostMemoryPool< CDnMaxDamageAddBlow >
{
public:
	CDnMaxDamageAddBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnMaxDamageAddBlow(void);

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
	float m_fRate;	//Probability value

#if defined(_GAMESERVER)
	//////////////////////////////////////////////////////////////////////////
	// OnTargetHit is called inside OnDamage(), causing bonus damage to appear applied twice.
	// So added handling to process additional damage on the next frame.
	// Includes handling to prevent per-actor probability calculation.
	bool m_bCalcProb;	//Whether to calculate probability
	bool m_bOnDamaged;	//Check if hit.
	//int m_nAddDamag; // Additional damage
	
	bool CalcProb();
	void AddDamage();
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
