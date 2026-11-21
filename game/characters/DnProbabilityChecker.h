#pragma once
#include "IDnSkillUsableChecker.h"


// Check with a certain probability.
class CDnProbabilityChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnProbabilityChecker >
{
private:
	float			m_fProbability;

public:
	CDnProbabilityChecker( DnActorHandle hActor, float fProbability );		// Probability is from 0 to 100
	virtual ~CDnProbabilityChecker(void);

	// from IDnSkillUsableChecker
	bool CanUse( void );
	int GetNumArgument( void ) { return 1; };

	float GetProbability() { return m_fProbability; }

	virtual IDnSkillUsableChecker* Clone();
};
