#include "stdafx.h"
#include "DnTerrainHomingOrbit.h"
#include "DnActor.h"
#include "DnProjectile.h"
#include "DnWorld.h"


CDnTerrainHomingOrbit::CDnTerrainHomingOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_fSpeed( 0.0f ),
																				 m_vTargetPosition( 0.0f, 0.0f, 0.0f )
{
	m_fSpeed = pProjectileInfo->fSpeed;
}

CDnTerrainHomingOrbit::~CDnTerrainHomingOrbit( void )
{

}

void CDnTerrainHomingOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
	// Homing processing.
	if( m_hTargetActor )
	{
		m_vTargetPosition = IDnOrbitProcessor::FindTargetPosition(m_hTargetActor, m_nTargetPartsIndex, m_nTargetPartsBoneIndex);
	}

	EtVector3 vDir = m_vTargetPosition - Cross.m_vPosition;
	float fDistanceSQ = EtVec3LengthSq( &vDir );
	EtVec3Normalize( &vDir, &vDir );
	Cross.m_vPosition += vDir * m_fSpeed * fDelta;

	// If nearly at the destination, set to that position
#ifdef _GAMESERVER
	// Game server runs at 20 frames, so widen the check interval.
	if( fDistanceSQ < 1000.0f )
		Cross.m_vPosition = m_vTargetPosition;
#else
	if( fDistanceSQ < 100.0f )
		Cross.m_vPosition = m_vTargetPosition;
#endif // #ifdef _GAMESERVER

	// Handle flying along the terrain. If above a prop like a bridge, also account for the y position on the prop.
#ifdef _GAMESERVER
	Cross.m_vPosition.y = CDnWorld::GetInstance(m_pRoom).GetHeightWithProp( Cross.m_vPosition );
#else
	Cross.m_vPosition.y = CDnWorld::GetInstance().GetHeightWithProp( Cross.m_vPosition );
#endif

	Cross.m_vPosition.y += 20.0f;
}