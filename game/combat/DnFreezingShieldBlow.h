#pragma once
#include "DnBlow.h"
#include "DnObservable.h"

class CDnFreezingShieldBlow : public CDnBlow, public TBoostMemoryPool< CDnFreezingShieldBlow >
{
private:
	int m_nDuration; //Durability
	float m_fRate;			//Magic attack power ratio
	float m_fLimitRadius;	//Attack radius
	int m_nFreezingTime;	//Attack (freeze) duration

	std::string m_strStateAttribute;	//Freeze setting values.

	void SetInfo(const char* szValue);

public:
	CDnFreezingShieldBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFreezingShieldBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	bool OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
