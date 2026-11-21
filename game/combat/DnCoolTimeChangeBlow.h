#pragma once
#include "DnBlow.h"

class CDnCoolTimeChangeBlow : public CDnBlow, public TBoostMemoryPool< CDnCoolTimeChangeBlow >
{
private:
	int m_nSkillType;		//Active/Passive
	float m_fCoolTimeRate;	//Cooldown ratio

	std::map<int, float> m_OrigSkillCoolTimeAdjustValues;

public:
	CDnCoolTimeChangeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnCoolTimeChangeBlow(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void ResetCoolTime();

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
