#pragma once
#include "DnBlow.h"


//Highlander (HighLander)
//- Skill Effect ID : 143
//- Value unit : float
//- Description 
//1) When applied, grants the "Highlander" buff effect for <duration 1>.
//2) While the buff is active, if the buffed player's HP drops to 0 or below, removes the "Highlander" buff, forces HP to 1, and prevents HP from dropping below 1 for <duration 2>.

class CDnHighlanderBlow : public CDnBlow, 
						  public TBoostMemoryPool< CDnHighlanderBlow >
{
private:
	enum
	{
		NORMAL,
		HIGHLANDER,
	};

	int m_iState;
	

public:
	CDnHighlanderBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnHighlanderBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	float CalcDamage( float fOriginalDamage );
	float OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
