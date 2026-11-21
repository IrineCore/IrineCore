#pragma once
#include "IDnSkillUsableChecker.h"


// Object that checks whether an enemy is within range using weapon + skill range. 
// Used only on client.
class CDnRangeChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnRangeChecker >
{
private:
	int	m_iRange;

public:
	CDnRangeChecker( DnActorHandle hActor );
	virtual ~CDnRangeChecker(void);

	void SetRange( int iRange ) { m_iRange = iRange; };
	int GetRange( void ) { return m_iRange; };

	virtual bool CanUse( void );
	virtual int GetNumArgument( void ) { return 0; };

	virtual IDnSkillUsableChecker* Clone();
};
