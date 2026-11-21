#pragma once
#include "IDnOrbitProcessor.h"
#include "SignalHeader.h"


// Class handling trajectory for OribtType:Linear
class CDnLinearOrbit : public IDnOrbitProcessor
{
private:
	float m_fSpeed;
	EtVector3 m_vFirstZVector;		// Direction vector used to move the actual projectile. Cross.ZVector is updated in real time as the transform matrix for elongated objects.
	bool m_bFirstProcess;

public:
	CDnLinearOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnLinearOrbit( void );

	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );
};