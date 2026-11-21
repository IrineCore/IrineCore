#pragma once
#include "DnBlow.h"


// Rotha
// Increases critical rate when hit by a fire attack while burned <ratio and flat rate increase>.

class CDnBurnCriticalBlow : public CDnBlow, public TBoostMemoryPool< CDnBurnCriticalBlow >
{
public:
	CDnBurnCriticalBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnBurnCriticalBlow(void);
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
	void SetBlowInfo();

	int m_nMultiplyRatio;   // Product of existing probabilities.
	int m_nAddValueRatio; // Add absolute critical value

	int GetMultiplyRatio(){return m_nMultiplyRatio;}
	int GetAddValueRatio(){return m_nAddValueRatio;}
#endif

protected:

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
