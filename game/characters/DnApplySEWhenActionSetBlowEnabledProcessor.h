#pragma once
#include "IDnSkillProcessor.h"
#include "DnSkill.h"


class CDnState;

// Added due to the Sacred Hammering skill. (#21052)
// Since ChangeActionSet is also a state effect, the state effect that lowers magic attack during Sacred Hammering
// It gets added, so only call ChangeActionSet when actually replaced with the Sacred Hammering action
// Add the remaining state effects except excluded ones; applies only to state effects added to self.
// Currently has no separate arguments.
class CDnApplySEWhenActionSetBlowEnabledProcessor : public IDnSkillProcessor, 
													public TBoostMemoryPool< CDnApplySEWhenActionSetBlowEnabledProcessor >
{
private:
	vector<const CDnSkill::StateEffectStruct*> m_vlpStateEffect;
	vector<int> m_vlAppliedStateEffectID;

public:
	CDnApplySEWhenActionSetBlowEnabledProcessor( DnActorHandle hActor );
	virtual ~CDnApplySEWhenActionSetBlowEnabledProcessor(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

#ifdef _GAMESERVER
	void AddStateEffect( const CDnSkill::StateEffectStruct* pStateEffect );

	void OnChangeActionBegin( void );
	void OnChangeActionSetBlowEnd( void );
#endif

	int GetNumArgument( void ) { return 0; };
};
