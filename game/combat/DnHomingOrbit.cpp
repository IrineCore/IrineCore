#include "stdafx.h"
#include "DnHomingOrbit.h"
#include "DnActor.h"
#include "DnProjectile.h"


CDnHomingOrbit::CDnHomingOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_fSpeed( 0.0f ),
																				 m_vTargetPosition( 0.0f, 0.0f, 0.0f ),
																				 m_bFallGravity( false )
{
	m_fSpeed = pProjectileInfo->fSpeed;
}

CDnHomingOrbit::~CDnHomingOrbit( void )
{

}

void CDnHomingOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hTargetActor )
	{
		m_vTargetPosition = IDnOrbitProcessor::FindTargetPosition(m_hTargetActor, m_nTargetPartsIndex, m_nTargetPartsBoneIndex);
	}

	EtVector3 vDir = m_vTargetPosition - Cross.m_vPosition;
	float fDistanceSQ = EtVec3LengthSq( &vDir );
	EtVec3Normalize( &vDir, &vDir );
	Cross.m_vPosition += vDir * m_fSpeed * fDelta;

	// In fallgravity state, only update the position.
	if( false == m_bFallGravity )
	{
		Cross.m_vZAxis = vDir;
		Cross.MakeUpCartesianByZAxis();
	}

	// If nearly at the destination, set to that position
#ifdef _GAMESERVER
	// Game server runs at 20 frames, so widen the check interval.
	if( fDistanceSQ < 1000.0f )
		Cross.m_vPosition = m_vTargetPosition;
#else
	if( fDistanceSQ < 100.0f )
		Cross.m_vPosition = m_vTargetPosition;
#endif // #ifdef _GAMESERVER
}

void CDnHomingOrbit::ProcessFallGravityOrbit( MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta, float& fGravity, float fFallGravityAccell )
{
	IDnOrbitProcessor::ProcessFallGravityOrbit( Cross, vPrevPos, LocalTime, fDelta, fGravity, fFallGravityAccell );

	m_bFallGravity = true;

	// Related to #29199. Align the projectile's world matrix to its travel direction when entering fallgravity state.
	// In fallgravity state, separately store prevpos based on fallgravity-processed position + current homing-updated position 
	// Process direction with this vector. The projectile's vPrevPos passed as argument is before fallgravity is applied in the current frame.
	//if( m_bFallGravity )
	//{
		Cross.m_vZAxis = Cross.m_vPosition - vPrevPos;
		EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis);
	//}
}
