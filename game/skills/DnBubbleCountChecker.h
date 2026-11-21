#pragma once
#include "IDnSkillUsableChecker.h"


// Skill can only be used if a specific bubble ID has at least a certain count.
class CDnBubbleCountChecker : public IDnSkillUsableChecker, public TBoostMemoryPool< CDnBubbleCountChecker >
{
private:
	int m_iBubbleTypeID;
	int	m_iNeedBubbleCount;

public:
	CDnBubbleCountChecker( DnActorHandle hActor, int iBubbleTypeID, int iNeedBubbleCount );
	virtual ~CDnBubbleCountChecker(void);

	void SetBubbleTypeID( int iBubbleTypeID ) { m_iBubbleTypeID = iBubbleTypeID; }
	int GetBubbleTypeID( void ) { return m_iBubbleTypeID; };
	void SetNeedBubbleCount( int iBubbleCount ) { m_iNeedBubbleCount = iBubbleCount; };
	int GetNeedBubbleCount( void ) { return m_iNeedBubbleCount; };

	virtual bool CanUse( void );
	virtual int GetNumArgument( void ) { return 2; };

	virtual IDnSkillUsableChecker* Clone();
};
