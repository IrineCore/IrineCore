#pragma once
#include "IDnSkillUsableChecker.h"

class CDnSummonChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnSummonChecker >
{
private:
	int	m_iRange;
	DNVector( int ) m_vlCheckActorIDs;

	string m_strValue;

public:
	CDnSummonChecker( DnActorHandle hActor , const char* pArg, int nRange);
	virtual ~CDnSummonChecker(void);

	virtual bool CanUse( void );
	virtual int GetNumArgument( void ) { return 2; }; // Takes range and monster ID arguments.

	virtual IDnSkillUsableChecker* Clone();
};
