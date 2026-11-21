#pragma once
#include "DnBlow.h"

// Skill effect ID: 291
// Decreases remaining cooldown of actor skills by m_fValue seconds (excluding skill IDs 2415 and 1412)
class CDnDecreaseCooltimeBlow : public CDnBlow, public TBoostMemoryPool< CDnDecreaseCooltimeBlow >
{
public:
	CDnDecreaseCooltimeBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnDecreaseCooltimeBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
