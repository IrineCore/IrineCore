#pragma once
#include "DnBlow.h"

// Skill effect ID: 287
// Issues an AI/order index to a monster actor
class CDnMonsterOrderBlow : public CDnBlow, public TBoostMemoryPool< CDnMonsterOrderBlow >
{
protected:
	int m_nOrderID;

public:
	CDnMonsterOrderBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnMonsterOrderBlow( void );

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	int GetOrderID( void ) const { return m_nOrderID; }
};
