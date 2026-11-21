#pragma once
#include "MAWalkMovement.h"

class MAWalkMovementNav : public MAWalkMovement, public TBoostMemoryPool<MAWalkMovementNav>
{
public:

	enum
	{
		NAV_WALLATTR_RED	= 1,
		NAV_WALLATTR_BLUE	= 2,
		NAV_WALLATTR_GREEN	= 4,
		NAV_WALLATTR_YELLOW	= 8,
	};

	MAWalkMovementNav(void);
	virtual ~MAWalkMovementNav(void);

protected:
	NavigationCell *m_pCurCell;
	DNVector(SCollisionResponse) m_vecResult;

public:

	virtual void ValidateCurCell();
	void ProcessNormal( LOCAL_TIME LocalTime, float fDelta );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void ProcessWallCollision( CDnActor *pActor, EtObjectHandle hObject, EtVector3 &vPrevPos );


};
