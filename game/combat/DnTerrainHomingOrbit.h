#pragma once
#include "IDnOrbitProcessor.h"
#include "DnCalcCatmullrom.h"
#include "SignalHeader.h"

// Class handling trajectory for OribtType:TerrainHoming VelocityType:Constant
// #22423
class CDnTerrainHomingOrbit : public IDnOrbitProcessor
{
private:
	float m_fSpeed;
	DnActorHandle m_hTargetActor;
	EtVector3 m_vTargetPosition;
	//CDnHoming m_Homing;

public:
	CDnTerrainHomingOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnTerrainHomingOrbit( void );

	void SetTargetActor( DnActorHandle hActor ) { m_hTargetActor = hActor; };
	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
};