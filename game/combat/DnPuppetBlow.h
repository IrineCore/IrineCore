#pragma once
#include "DnBlow.h"

//Puppet state effect...
class CDnPuppetBlow : public CDnBlow, 
								 public TBoostMemoryPool< CDnPuppetBlow >
{
public:
	CDnPuppetBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnPuppetBlow(void);

#ifdef _GAMESERVER
	virtual bool CanBegin( void );
#endif

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:
#if defined(_GAMESERVER)
	EtVector3 m_vDeltaPos;
	DnMonsterActorHandle m_hSummonMonster;
#ifdef PRE_ADD_DECREASE_EFFECT
	bool m_bShowReduce;
public:
	void SetShowReduce( bool bShowReduce ) { m_bShowReduce = bShowReduce; }
#endif // PRE_ADD_DECREASE_EFFECT
#endif // _GAMESERVER
	std::string m_DamageChangeRateArg;	//Damage ratio increase value (state effect setting; applies to both physical (134) and magic (135))
	float m_fLimitRange;				//Distance limit between the summon actor and puppet summoned monster (state effect is removed if exceeded)

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
