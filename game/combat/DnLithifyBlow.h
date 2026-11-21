#pragma once
#include "DnBlow.h"


// Petrify state effect!
class CDnLithifyBlow : public CDnBlow
{
private:
	float m_fStepTime;
	int m_iNowStep;
	float m_fOriDurationTime;

public:
	CDnLithifyBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnLithifyBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
