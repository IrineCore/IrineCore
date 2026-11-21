#pragma once
#include "IDnOrbitProcessor.h"
#include "SignalHeader.h"


// Class handling trajectory for OribtType:TerrainLinear
class CDnTerrainLinearOrbit : public IDnOrbitProcessor
{
private:
	bool m_bFirstProcess;
	float m_fSpeed;


public:
	CDnTerrainLinearOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnTerrainLinearOrbit( void );

	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
};