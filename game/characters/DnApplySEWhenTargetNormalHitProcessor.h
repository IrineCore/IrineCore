#pragma once
#include "IDnSkillProcessor.h"
#include "DnSkill.h"

#ifdef _GAMESERVER

// Trigger type used by skills in the item prefix system. (#23818)
// Apply skill Target state effects with a probability to targets hit by normal attack.
// A cooldown concept may be included later.
class CDnApplySEWhenTargetNormalHitProcessor : public IDnSkillProcessor, 
											   public TBoostMemoryPool< CDnApplySEWhenTargetNormalHitProcessor >
{
private:
	float m_fProbability;

public:
	CDnApplySEWhenTargetNormalHitProcessor( DnActorHandle hActor, float fProbability );
	virtual ~CDnApplySEWhenTargetNormalHitProcessor( void );

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	void OnNormalHitSuccess( DnActorHandle hHittedTarget );		// When normal basic attack succeeds.

	int GetNumArgument( void ) { return 0; };
};
#endif // #ifdef _GAMESERVER
