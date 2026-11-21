#pragma once
#include "DnBlow.h"


class CDnBasisAttackDmgUpBlow : public CDnBlow, public TBoostMemoryPool< CDnBasisAttackDmgUpBlow >
{
public:
	CDnBasisAttackDmgUpBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnBasisAttackDmgUpBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

protected:
	//DWORD m_nObejctID;

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
