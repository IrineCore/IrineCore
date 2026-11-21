#pragma once
#include "DnBlow.h"

class CDnInvincibleTypeBlow : public CDnBlow, public TBoostMemoryPool< CDnInvincibleTypeBlow >
{
protected:
	

public:
	CDnInvincibleTypeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnInvincibleTypeBlow(void);

private:
	int m_nInvinvibleType;

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	int GetInvincibleTypeValue() { return m_nInvinvibleType; }

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
