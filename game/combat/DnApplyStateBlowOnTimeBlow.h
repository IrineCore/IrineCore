#pragma once
#include "DnBlow.h"
#include "DnActorStatIntervalManipulator.h"

class CDnApplyStateBlowOnTimeBlow : public CDnBlow, public TBoostMemoryPool< CDnApplyStateBlowOnTimeBlow >
{
public:
	CDnApplyStateBlowOnTimeBlow(DnActorHandle hActor, const char *szValue);
	virtual ~CDnApplyStateBlowOnTimeBlow(void);

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#if defined(_GAMESERVER)
protected:
	CDnActorStatIntervalManipulator	m_IntervalChecker;

	DnSkillHandle m_hSkill;	//Skill handle of state effect to apply

	bool OnCustomIntervalProcess( void );
	void ApplyStateEffect();
#endif // _GAMESERVER

protected:
	int m_nTime;	//Time interval (1000 => 1 sec)
	float m_fRate;	//Probability
	int m_nSkillID;	//Skill ID of state effect to apply
	int m_nSkillLevel;	//Skill level of state effect to apply

	void SetInfo(const char* szValue);

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);

#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
