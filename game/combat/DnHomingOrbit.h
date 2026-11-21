#pragma once
#include "IDnOrbitProcessor.h"
#include "DnCalcCatmullrom.h"
#include "SignalHeader.h"

// Class handling homing projectile for OrbitType:Homing, VelocityType:Constant
class CDnHomingOrbit : public IDnOrbitProcessor
{
private:
	float m_fSpeed;
	DnActorHandle m_hTargetActor;
	EtVector3 m_vTargetPosition;
	CDnHoming m_Homing;
	bool m_bFallGravity;

public:
	CDnHomingOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnHomingOrbit( void );

	void SetTargetActor( DnActorHandle hActor ) { m_hTargetActor = hActor; };
	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
	
	// Due to #29199, align the world transform matrix of projectiles that become invalid and enter fallgravity to their travel direction.
	void ProcessFallGravityOrbit( MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta, float& fGravity, float fFallGravityAccell );
};