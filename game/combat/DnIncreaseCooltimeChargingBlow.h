#pragma once
#include "DnBlow.h"

// Skill effect ID: 304
// Increases max cooldown charge stack count on target skill ID
class CDnIncreaseCooltimeChargingBlow : public CDnBlow, public TBoostMemoryPool< CDnIncreaseCooltimeChargingBlow >
{
protected:
	int m_nSkillID;
	int m_nAddChargeCount;

public:
	CDnIncreaseCooltimeChargingBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnIncreaseCooltimeChargingBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
