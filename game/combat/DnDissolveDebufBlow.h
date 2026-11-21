#pragma once
#include "DnBlow.h"


// State effect that dispels state effects applied by debuff skills. 
class CDnDissolveDebufBlow : public CDnBlow, public TBoostMemoryPool< CDnDissolveDebufBlow >
{
protected:
	

public:
	CDnDissolveDebufBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDissolveDebufBlow(void);

private:
	bool _IsBadStateBlow( DnBlowHandle hBlow );

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
