#include "stdafx.h"
#include "DnLinearOrbit.h"
#include "DnProjectile.h"
#include "VelocityFunc.h"


CDnLinearOrbit::CDnLinearOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_fSpeed( 0.0f ), 
																				 m_vFirstZVector( 0.0f, 0.0f, 0.0f ),
																				 m_bFirstProcess( true )
{
	m_fSpeed = pProjectileInfo->fSpeed;
}

CDnLinearOrbit::~CDnLinearOrbit( void )
{

}

void CDnLinearOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	Cross.m_vPosition += Cross.m_vZAxis * ( fDelta * m_fSpeed );
#else
	if( true == m_bFirstProcess )
	{
		m_vFirstZVector = Cross.m_vZAxis;
	}

	Cross.m_vPosition += m_vFirstZVector * ( fDelta * m_fSpeed );
	
	if( false == m_bFirstProcess )
	{
		Cross.m_vZAxis = Cross.m_vPosition - vPrevPos;
		EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis );
		Cross.MakeUpCartesianByZAxis();
	}

	m_bFirstProcess = false;
#endif
}