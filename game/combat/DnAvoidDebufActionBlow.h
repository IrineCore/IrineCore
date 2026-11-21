#pragma once
#include "DnBlow.h"

// Issue #51048
// State effect that prevents action interruption during freeze, shock, etc. state effects.
// Currently handled that way for Nest Named grade or higher monsters, but this state effect 
// Owned actors are processed in the same way.
class CDnAvoidDebufActionBlow : public CDnBlow,
						 public TBoostMemoryPool< CDnAvoidDebufActionBlow >
{
private:

public:
	CDnAvoidDebufActionBlow( DnActorHandle hActor, const char* szValue );
	virtual ~CDnAvoidDebufActionBlow(void);

	virtual void OnBegin( LOCAL_TIME LocalTime, float fDelta );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
