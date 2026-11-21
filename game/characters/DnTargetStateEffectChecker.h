#pragma once
#include "IDnSkillUsableChecker.h"
#include "DnBlowDef.h"

class CDnTargetStateEffectChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnTargetStateEffectChecker >
{
protected:
	STATE_BLOW::emBLOW_INDEX			m_targetStateIndex;
	float								m_fRadiusDistToSearch;
	bool								m_bCheckSkillUser;	//Check if the skill user is the same.

public:
	CDnTargetStateEffectChecker( DnActorHandle hActor, STATE_BLOW::emBLOW_INDEX targetStateIndex, float fRadiusDistToSearch, bool bCheckSkillUser );
	virtual ~CDnTargetStateEffectChecker(void);

	// from IDnSkillUsableChecker
	bool CanUse( void );
	int GetNumArgument( void ) { return 2; };

	virtual IDnSkillUsableChecker* Clone();
};
