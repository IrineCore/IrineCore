#include "stdafx.h"
#include "DnHomingAccellOrbit.h"
#include "DnActor.h"
#include "DnProjectile.h"


#if defined(PRE_FIX_HOMINGTARGET)
bool CDnHomingAccellOrbit::ms_ShowHomingTarget = false;
#endif // PRE_FIX_HOMINGTARGET

CDnHomingAccellOrbit::CDnHomingAccellOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo ) : m_fSpeed( 0.0f ),
																							 m_vTargetPosition( 0.0f, 0.0f, 0.0f ),
																							 m_bFirstProcess( true ),
																							 m_piValidTime( NULL )
{
	m_fSpeed = pProjectileInfo->fSpeed;
}

CDnHomingAccellOrbit::~CDnHomingAccellOrbit( void )
{

}

void CDnHomingAccellOrbit::Initialize( const MatrixEx& Cross, const MatrixEx& OffsetCross )
{
	m_OffsetCross = OffsetCross;
}

void CDnHomingAccellOrbit::ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta )
{
	if( m_hTargetActor )
	{
		m_vTargetPosition = __super::FindTargetPosition(m_hTargetActor, m_nTargetPartsIndex, m_nTargetPartsBoneIndex);
	}

	if( m_bFirstProcess )
	{
		EtVector3 vDir = m_vTargetPosition - Cross.m_vPosition;
		float fLength = EtVec3Length( &vDir );

		m_Homing.Init( (LOCAL_TIME)( ( fLength / m_fSpeed ) * 1000.f ), LocalTime, m_OffsetCross.m_vPosition, m_vTargetPosition, m_OffsetCross.m_vZAxis );
		_ASSERT( m_piValidTime );
		*m_piValidTime = (int)( ( fLength / m_fSpeed ) * 1000.f );

		m_bFirstProcess = false;
	}

	m_Homing.SetEndPos( m_vTargetPosition );
	m_Homing.Process( LocalTime );

	Cross.m_vZAxis = m_Homing.GetVVec();
	Cross.m_vPosition = m_Homing.GetPos();
	Cross.MakeUpCartesianByZAxis();


#if defined(PRE_FIX_HOMINGTARGET)
#if defined(_GAMESERVER)
#else
	if (ms_ShowHomingTarget == true)
		EternityEngine::DrawLine3D(Cross.m_vPosition, m_vTargetPosition);
#endif // _GAMESERVER
#endif // PRE_FIX_HOMINGTARGET
}