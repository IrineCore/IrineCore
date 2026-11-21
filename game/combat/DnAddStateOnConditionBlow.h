#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


class CDnAddStateOnConditionBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnConditionBlow >
{
private:
	float m_fRate;				//Probability
	int m_nHitStateBlowIndex;	//State effect Index on Hit
	int m_nHitAttackAttribute;	//Attack element on Hit
	int m_nDestStateBlowIndex;	//State effect index to add when condition is met

	std::string m_strStateAttribute;
	int m_nStateDurationTime;

	void SetInfo(const char* szValue);
public:
	CDnAddStateOnConditionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnConditionBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual void OnTargetHit( DnActorHandle hTargetActor );
#else
	virtual bool OnCustomIntervalProcess( void );
	void ShowGraphicEffect(bool bShow);
#endif

#if defined(_GAMESERVER)
protected:
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
