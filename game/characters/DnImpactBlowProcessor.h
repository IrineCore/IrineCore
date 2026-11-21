#pragma once
#include "IDnSkillProcessor.h"


class CDnState;

// Dedicated activation type for Cleric's Impact Blow. Checks if Energy Charge skill buff is active and increases attack power by that value
class CDnImpactBlowProcessor : public IDnSkillProcessor, public TBoostMemoryPool< CDnImpactBlowProcessor >
{
private:
	CDnState*					m_pState;
	int							m_iNumChargeBlow;

public:
	CDnImpactBlowProcessor( DnActorHandle hActor );
	virtual ~CDnImpactBlowProcessor(void);

	void OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );

	int GetNumArgument( void ) { return 0; };
};
