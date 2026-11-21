#pragma once
#include "DnBlow.h"
#ifndef _GAMESERVER
#include "DnComboCalculator.h"
#endif

class CDnCantActionBlow;

// Skill effect ID: 305
// Fear debuff: wraps CDnCantActionBlow with A/D key mash duration reduction
class CDnFearBlow : public CDnBlow, public TBoostMemoryPool< CDnFearBlow >
{
protected:
	CDnCantActionBlow* m_pCantActionBlow;
	int m_nParam0;
	float m_fReduceTimePerMash;
#ifndef _GAMESERVER
	CDnComboCalculator m_ComboCalc;
#endif

public:
	CDnFearBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnFearBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
