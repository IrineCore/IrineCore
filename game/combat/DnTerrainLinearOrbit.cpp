#include "stdafx.h"
#include "DnTerrainLinearOrbit.h"
#include "DnWorld.h"
#include "DnProjectile.h"


CDnTerrainLinearOrbit::CDnTerrainLinearOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_fSpeed( 0.0f ), m_bFirstProcess( false )
{
	m_fSpeed = pProjectileInfo->fSpeed;
	m_bFirstProcess = true;
}

CDnTerrainLinearOrbit::~CDnTerrainLinearOrbit( void )
{

}

void CDnTerrainLinearOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
	// 2009.7.30 hanki
	// If on a prop like a bridge, also account for the y position on top of the prop
	//EtVector3 vPrevPos = Cross.m_vPosition;

	if( m_bFirstProcess )
	{
		Cross.m_vZAxis.y = 0.0f;
		EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis );
		Cross.MakeUpCartesianByZAxis();

		m_bFirstProcess = false;
	}

	Cross.m_vPosition += Cross.m_vZAxis * ( fDelta * m_fSpeed );

#ifdef _GAMESERVER
	Cross.m_vPosition.y = CDnWorld::GetInstance(m_pRoom).GetHeightWithProp( Cross.m_vPosition );
#else
	Cross.m_vPosition.y = CDnWorld::GetInstance().GetHeightWithProp( Cross.m_vPosition );
#endif

	Cross.m_vPosition.y += 20.0f;
	Cross.m_vZAxis = Cross.m_vPosition - vPrevPos;
	EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis );
	Cross.MakeUpCartesianByZAxis();
}
