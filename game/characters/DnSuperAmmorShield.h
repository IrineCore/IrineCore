#pragma once
#include "DnBlow.h"

/********************************************************************
	created:	2010/12/18
	author:		semozz
	
purpose: State effect that protects super armor from breaking
*********************************************************************/
class CDnSuperAmmorShield : public CDnBlow, public TBoostMemoryPool< CDnSuperAmmorShield >
{
protected:
	

public:
	CDnSuperAmmorShield( DnActorHandle hActor, const char* szValue );
	virtual ~CDnSuperAmmorShield(void);

public:
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
