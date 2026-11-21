#pragma once
#include "IDnSkillUsableChecker.h"


// Check if Cleric's Energy Charge skill is active
class CDnEnergyChargeChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnEnergyChargeChecker >
{
private:


public:
	CDnEnergyChargeChecker( DnActorHandle hActor );
	virtual ~CDnEnergyChargeChecker(void);

	// from IDnSkillUsableChecker
	bool CanUse( void );
	int GetNumArgument( void ) { return 0; };

	virtual IDnSkillUsableChecker* Clone();
};
