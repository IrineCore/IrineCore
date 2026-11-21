#pragma once
#include "IDnSkillUsableChecker.h"
#include "DnActorState.h"

// Check the actor's state
class CDnActorStateChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnActorStateChecker >
{
private:
	int									m_iOperator;
	CDnActorState::ActorStateEnum		m_eRequiredState;
	

public:
	CDnActorStateChecker( DnActorHandle hActor, CDnActorState::ActorStateEnum iRequiredState, int iOperator );
	virtual ~CDnActorStateChecker(void);

	bool CanUse( void );
	int GetNumArgument( void ) { return 2; };

	virtual IDnSkillUsableChecker* Clone();
};
