#pragma once
#include "DnBlow.h"


//Change MP usage (UseMpRatio)
//- Skill Effect ID : 139
//- Value unit : float
//- Description 
//1) When active for <duration>, adjusts skill MP cost for the affected user to <base cost> * <effect ratio>.

class CDnChangeMPConsumeRatioBlow : public CDnBlow, 
								    public TBoostMemoryPool< CDnChangeMPConsumeRatioBlow >
{
private:
	

public:
	CDnChangeMPConsumeRatioBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnChangeMPConsumeRatioBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	virtual int OnUseMP( int iMPDelta );
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
