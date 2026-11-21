#pragma once
#include "IDnSkillUsableChecker.h"


// Checks whether a specific toggle skill is running
class CDnToggleChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnToggleChecker >
{
private:
	int					m_iRequireToggleSkillID;	

public:
	CDnToggleChecker( DnActorHandle hActor, int iToggleSkillID );
	virtual ~CDnToggleChecker(void);

	// from IDnSkillUsableChecker
	bool CanUse( void );
	int GetNumArgument( void ) { return 1; };

	virtual IDnSkillUsableChecker* Clone();
};
