#pragma once
#include "DnBlow.h"

class CDnVarianceDamgeBlow : public CDnBlow, public TBoostMemoryPool< CDnVarianceDamgeBlow >
{
public:
	CDnVarianceDamgeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnVarianceDamgeBlow(void);

	enum VarianceType
	{
		NONE = 0,
		DIVIDE_DAMAGE,
		ZERO_DAMAGE,
		INC_DAMAGE
	};

private:
	int m_MaxDamage;		//Damage
	int m_nLimitMaxCount;	//Hit limit target count?
	int m_nApplyType;		//Damage change type (split/0/increase)
	
	int m_nHitActorCount;	//Number of actors actually hit

#if defined(_GAMESERVER)
	void ApplyDamage(int nHitCount);
protected:
	void OnSetParentSkillInfo();
#endif // _GAMESERVER

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
