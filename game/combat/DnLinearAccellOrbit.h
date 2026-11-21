#pragma once
#include "IDnOrbitProcessor.h"
#include "SignalHeader.h"


// Class handling trajectory for OrbitType:Accell
class CDnLinearAccellOrbit : public IDnOrbitProcessor
{
private:
	float m_fSpeed;
	float m_fResistance;

public:
	CDnLinearAccellOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnLinearAccellOrbit( void );

	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
};