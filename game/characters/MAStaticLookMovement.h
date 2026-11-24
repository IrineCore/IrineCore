
#pragma once

#include "MAStaticMovement.h"

class MAStaticLookMovement : public MAStaticMovement
{
public:

	MAStaticLookMovement();

	virtual void Look( const EtVector2 &vVec, bool bForceRotate = true );
	virtual EtVector3 *GetLookDir();
	virtual EtVector2 *GetLookDirTarget() { return &m_vTargetLookVec; }
	virtual void LookTarget( const DnActorHandle &hActor );
	virtual DnActorHandle GetLookTarget();
	virtual void ResetLook();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

protected:

	EtVector2 m_vTargetLookVec;
	DnActorHandle m_hLookTarget;
	LOCAL_TIME m_LastLookTargetTime;
};
