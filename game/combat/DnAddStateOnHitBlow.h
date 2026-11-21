#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"


class CDnAddStateOnHitBlow : public CDnBlow, public TBoostMemoryPool< CDnAddStateOnHitBlow >
{
private:
	float m_fRate;				//Probability value
	int m_nStateDurationTime;	//Duration of state effect to be added
	int m_nDestStateBlowIndex;	//State effect index to add when condition is met

	std::string m_strStateAttribute;
	
	void SetInfo(const char* szValue);
public:
	CDnAddStateOnHitBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAddStateOnHitBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

#if defined(_GAMESERVER)
protected:
	//////////////////////////////////////////////////////////////////////////
	// Handling to prevent per-actor probability calculation.
	bool m_bCalcProb;	//Whether to calculate probability
	bool m_bOnDamaged;	//Check if hit.

	bool CalcProb();
	//////////////////////////////////////////////////////////////////////////
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
