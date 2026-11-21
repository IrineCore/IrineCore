#pragma once
#include "DnBlow.h"

class CDnForceAggroTargetBlow : public CDnBlow, public TBoostMemoryPool< CDnForceAggroTargetBlow >
{
private:

public:
	CDnForceAggroTargetBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnForceAggroTargetBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
protected:
	void ForceAggroTarget();
	void FindAggroTargetWhenProcessSkill();

	DnActorHandle FindRandomAggroTarget();
#endif

protected:

#ifdef _GAMESERVER
	DnActorHandle m_hTargetActor;
	LOCAL_TIME m_tLastRefreshTime;
	LOCAL_TIME m_tLastFindTargetTime;

	bool m_bFindRandomAggroTarget;
	bool m_bFindPlayerActorOnly;
	float m_fForceIncreaseAggroValue;
	int m_nServerGraphicBlowID;
	int m_nTargetEffectIndex;
#endif


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
public:
	static void AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
	static void RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
};
