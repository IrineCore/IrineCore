#pragma once
#include "DnBlowDef.h"

class CDnCreateBlow
{
public:
	CDnCreateBlow(void);
	virtual ~CDnCreateBlow(void);

static	bool IsBasicBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex );

public:
	DnBlowHandle CreateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex, DnActorHandle hActor, const char *szParam );

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(STATE_BLOW::emBLOW_INDEX emBlowIndex, const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(STATE_BLOW::emBLOW_INDEX emBlowIndex, const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
	