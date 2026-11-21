#include "stdafx.h"
#include "DnGravityOrbit.h"
#include "DnProjectile.h"
#include "DnWorld.h"


const EtVector3 CDnGravityOrbitCalculator::GRAVITY( 0.0f, -980.0f, 0.0f );

CDnGravityOrbit::CDnGravityOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_vStartPos( 0.0f, 0.0f, 0.0f ),
																				   m_vDirectionWhenCreate( 0.0f, 0.0f, 0.0f ),
																				   m_vInitialSpeed( 0.0f, 0.0f, 0.0f ),
																				   m_vTargetPosition( 0.0f, 0.0f, 0.0f ), 
																				   m_fProjectileOrbitRotateZDegree( 0.0f ),
																				   m_bFirstProcess( true ),
																				   m_fElapsedTime( 0.0f )
{
	m_fProjectileOrbitRotateZDegree = pProjectileInfo->fProjectileOrbitRotateZ;
}

CDnGravityOrbit::~CDnGravityOrbit( void )
{

}

void CDnGravityOrbit::Initialize( const MatrixEx& Cross, const MatrixEx& OffsetCross )
{
	m_vStartPos = Cross.m_vPosition;
	
	m_vDirectionWhenCreate = Cross.m_vZAxis;
	EtVec3Normalize( &m_vDirectionWhenCreate, &Cross.m_vZAxis );
}

void CDnGravityOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
	// Hanki 2009.7.22
	// Related to projectile trajectory.. (#4851)
	// Changed to apply gravitational acceleration instead of using Catmull-Rom interpolation
	// Process with quadratic curve..
	if( m_bFirstProcess )
	{
		// Note: If distance is the value when the projectile is created on client and direction is the rotation calculated on server 
		// Direction may differ due to slight time lag, so use the direction at projectile creation time.
		// Server projectiles are created from client input, so server must do this as well to spawn in the client-specified direction without mismatch.

		// #37334 When fired weakly, the target point's height may exceed the parabola's maximum height.
		// Otherwise the result value comes out wrong, so remove the correction value..
		//if( m_vTargetPosition.y - m_vStartPos.y <= 0.0f )
		//	m_vTargetPosition.y += 50.0f; // Raise slightly to account for character height.

		EtVector3 vDistance = m_vTargetPosition - m_vStartPos;

		// The xz signs of the direction and Direction must match. 
		bool bXValid = (vDistance.x < 0.0f && m_vDirectionWhenCreate.x < 0.0f || vDistance.x >= 0.0f && m_vDirectionWhenCreate.x >= 0.0f);
		bool bZValid = (vDistance.z < 0.0f && m_vDirectionWhenCreate.z < 0.0f || vDistance.z >= 0.0f && m_vDirectionWhenCreate.z >= 0.0f);
		bool bValidDirection =  bXValid && bZValid;

		if( 0.0f == m_vDirectionWhenCreate.y )
			m_vDirectionWhenCreate.y = 0.1f;

		// Sign can subtly flip when direction is near origin, so follow the direction.
		if( false == bXValid )
		{
			if( m_vDirectionWhenCreate.x < 0.0f && vDistance.x >= 0.0f ||
				m_vDirectionWhenCreate.x >= 0.0f && vDistance.x < 0.0f )
				vDistance.x *= -1.0f;
		}
		else
		if( false == bZValid )
		{
			if( m_vDirectionWhenCreate.z < 0.0f && vDistance.z >= 0.0f ||
				m_vDirectionWhenCreate.z >= 0.0f && vDistance.z < 0.0f )
				vDistance.z *= -1.0f;
		}

		// Substitute the trailing term by the slope of the direction vector and solve for time using the quadratic formula in 0.5*Gt^2 + c form
		float fSlope = 0.0f;
		float fC = -vDistance.y;

		// Can a character's direction ever be a zero vector? OTL
		_ASSERT( m_vDirectionWhenCreate.x != 0.0f || m_vDirectionWhenCreate.z != 0.0f );
		if( m_vDirectionWhenCreate.x == 0.0f && m_vDirectionWhenCreate.z == 0.0f )
			m_vDirectionWhenCreate.x = m_vDirectionWhenCreate.z = 0.1f;

		bool bUseX = true;
		if( 0.0f != m_vDirectionWhenCreate.x )
		{
			fSlope = m_vDirectionWhenCreate.y / m_vDirectionWhenCreate.x;
			fC += vDistance.x * fSlope;
		}
		else
		{
			fSlope = m_vDirectionWhenCreate.y / m_vDirectionWhenCreate.z;
			fC += vDistance.z * fSlope;
			bUseX = false;
		}

		float fA = 0.5f * CDnGravityOrbitCalculator::GRAVITY.y;
		float fD = -4.0f * fA * fC;

		// Let's check when this becomes negative. -_-
		if( fD < 0.0f )
			fD *= -1.0f;
		//_ASSERT( fD >= 0.0f );
		float fEstimateElapseTime = sqrtf( fD ) / (2.0f * fA);

		// Time must always be positive
		if( fEstimateElapseTime < 0.0f )
			fEstimateElapseTime *= -1.0f;

		float fPower = 0.0f;
		if( bUseX )
			fPower = vDistance.x / (m_vDirectionWhenCreate.x*fEstimateElapseTime);
		else
			fPower = vDistance.z / (m_vDirectionWhenCreate.z*fEstimateElapseTime);

		m_vInitialSpeed = m_vDirectionWhenCreate * fPower;

		m_bFirstProcess = false;
	}

	// Current direction equals the normalized current velocity vector.
	EtVector3 vNowDir = m_vInitialSpeed + m_fElapsedTime*CDnGravityOrbitCalculator::GRAVITY; 
	EtVector3 vDelta = 0.5f*CDnGravityOrbitCalculator::GRAVITY*(m_fElapsedTime*m_fElapsedTime) + m_vInitialSpeed*m_fElapsedTime;

	// Added option for the trajectory itself to rotate. (#15056)
	// If destination is set to DestPosition with random drop, trajectory rotation is not applied and x, z are used as-is.
	// For example, even if tilted 90 degrees, the trajectory does not scatter along the 90-degree axis, since the destination remains based on x, z...
	if( 0.0f != m_fProjectileOrbitRotateZDegree )
	{
		EtMatrix matRotZ;
		EtVector3 vCharViewXZ( m_vDirectionWhenCreate.x, 0.0f, m_vDirectionWhenCreate.z );
		EtVec3Normalize( &vCharViewXZ, &vCharViewXZ );
		EtMatrixRotationAxis( &matRotZ, &vCharViewXZ, D3DXToRadian(m_fProjectileOrbitRotateZDegree) );
		EtVec3TransformNormal( &vDelta, &vDelta, &matRotZ );
		EtVec3TransformNormal( &vNowDir, &vNowDir, &matRotZ );
	}
	// 

	EtVector3 vNowPos = m_vStartPos + vDelta;
	
	EtVec3Normalize( &vNowDir, &vNowDir );
	if( EtVec3LengthSq( &vNowDir ) != 0.0f ) 
		Cross.m_vZAxis = vNowDir;

	Cross.m_vPosition = vNowPos;
	Cross.MakeUpCartesianByZAxis();

	m_fElapsedTime += fDelta;
}

#ifndef _GAMESERVER
//////////////////////////////////////////////////////////////////////////
bool CDnGravityOrbitCalculator::CalcHitGroundPos( MatrixEx Cross, const EtVector3& vShootPos, const float fInitialSpeed, 
												  const EtVector3& vInitialDir, EtVector3& vGroundHitPosition )
{
	bool bResult = false;

	// Calculate starting from when falling to save computation load. 
	// Point where velocity becomes 0.
	EtVector3 vFallingStartPos( 0.0f, 0.0f, 0.0f );
	
	// Calculate the time when y-axis velocity becomes 0.
	float fFallingStartTime = (-fInitialSpeed*vInitialDir.y) / GRAVITY.y;

	// Set projectile position to the drop point..
	vFallingStartPos = (fInitialSpeed*vInitialDir*fFallingStartTime + ((GRAVITY*fFallingStartTime*fFallingStartTime) / 2.0f));
	Cross.m_vPosition = vShootPos;					// vShootPos is in world coordinates.
	Cross.MoveLocalYAxis( vFallingStartPos.y );			// vFallingStartPos is in local coordinates. Therefore, position is set by axis translation.
	Cross.MoveLocalZAxis( vFallingStartPos.z );
	Cross.MoveLocalXAxis( vFallingStartPos.x );
	EtVector3 vPrevPos = Cross.m_vPosition;

	// Check if it collides with terrain when falling from that point..
	float fTimeDelta = 1.0f / 20.0f;
	float fElapsedTime = fFallingStartTime;
	while( true )
	{
		fElapsedTime += fTimeDelta;

		EtVector3 vNowPos = (fInitialSpeed*vInitialDir*fElapsedTime + ((GRAVITY*fElapsedTime*fElapsedTime) / 2.0f));
		Cross.m_vPosition = vShootPos;
		Cross.MoveLocalYAxis( vNowPos.y );
		Cross.MoveLocalZAxis( vNowPos.z );
		Cross.MoveLocalXAxis( vNowPos.x );

		EtVector3 vDir = Cross.m_vPosition - vPrevPos;
		EtVec3Normalize( &vDir, &vDir );
		EtVector3 vPickPos;
		float fHeight = CDnWorld::GetInstance().GetHeight( Cross.m_vPosition );
		if( Cross.m_vPosition.y <= fHeight )
		{
			if( CDnWorld::GetInstance().Pick( vPrevPos, vDir, vPickPos ) == true ) 
			{
				vGroundHitPosition = vPickPos;
				bResult = true;
				break;
			}
		}

		// If time exceeds limit, assume no landing point and exit loop.
		if( 5.0f < fElapsedTime )
		{
			// If there is no terrain collision point, set to the position that reaches the y value of the original firing position.
			float fYPosZeroTime = -2.0f*(fInitialSpeed*vInitialDir.y) / GRAVITY.y;
			EtVector3 vYPosZero = (fInitialSpeed*vInitialDir*fYPosZeroTime + ((GRAVITY*fYPosZeroTime*fYPosZeroTime) / 2.0f));
			Cross.m_vPosition = vShootPos;
			Cross.MoveLocalYAxis( vYPosZero.y );
			Cross.MoveLocalZAxis( vYPosZero.z );
			Cross.MoveLocalXAxis( vYPosZero.x );
			vGroundHitPosition = Cross.m_vPosition;
			break;
		}

		vPrevPos = Cross.m_vPosition;
	}

	return bResult;
}
#endif