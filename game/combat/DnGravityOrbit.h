#pragma once
#include "IDnOrbitProcessor.h"
#include "SignalHeader.h"

// Class handling gravity-affected projectile trajectory for OrbitType:Projectile VelocityType:Accell
class CDnGravityOrbit : public IDnOrbitProcessor
{
private:
	float m_fProjectileOrbitRotateZDegree;
	
	EtVector3 m_vStartPos;
	EtVector3 m_vDirectionWhenCreate;
	
	EtVector3 m_vTargetPosition;
	EtVector3 m_vInitialSpeed;
	
	bool m_bFirstProcess;

	float m_fElapsedTime;

public:
	CDnGravityOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnGravityOrbit( void );
	
	void Initialize( const MatrixEx& Cross, const MatrixEx& OffsetCross );
	void SetTargetPosition( EtVector3 vTarget ) { m_vTargetPosition = vTarget; };
	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
};

// Helper for quadratic curve calculations.. currently used for cannon trajectory in CDnCannonMonsterActor.
class CDnGravityOrbitCalculator
{
public:
	static const EtVector3 GRAVITY;

	// Calculate landing position and time given initial velocity and launch direction.. 
	// Since it's unknown where it will land depending on terrain, step with a 1/20 sec delta to directly calculate terrain collision..
	// Externally, ensure this is called only once when the condition value changes.
	// vInitialDir is a normalized direction vector in local coordinates. (x value is 0)
#ifndef _GAMESERVER
	static bool CalcHitGroundPos( /*IN*/ MatrixEx Cross, /*IN*/ const EtVector3& vShootPos, /*IN*/ const float fInitialSpeed, 
								  /*IN*/ const EtVector3& vInitialDir, /*OUT*/ EtVector3& vGroundHitPosition );
#endif
};
