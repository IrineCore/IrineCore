#include "StdAfx.h"
#include "DnActor.h"
#include "MAHeadLookController.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

MAHeadLookController::MAHeadLookController()
{
	m_vHeadLook = EtVector3( 0.f, 0.f, 0.f );
	m_vCurHeadLook = EtVector3( 0.f, 0.f, 0.f );
	m_vHeadLookTargetPos = EtVector3( 0.f, 0.f, 0.f );

	m_nHeadLookResopneLength = 0;
	m_nHeadLookRotateSpeed = 0;
	m_fHeadLookHeadRatio = 0.f;
	m_fHeadLookYAxisMaxAngle = 0.f;
	m_fHeadLookXAxisMaxAngle = 0.f;
	m_nHeadLookBoneIndex[0] = m_nHeadLookBoneIndex[1] = -1;
	m_bHeadLookYAxisReverse = m_bHeadLookXAxisReverse = false;
	SetHeadLookTarget( CDnActor::s_hLocalActor );

	m_fHeadScale = 1.0f;
}

MAHeadLookController::~MAHeadLookController()
{
}

void MAHeadLookController::InitializeHeadLook( EtAniObjectHandle hObject )
{
	if( !hObject ) return ;
	if( !hObject->GetAniHandle() ) return ;

	m_hHeadLookObject = hObject;
	m_nHeadLookBoneIndex[ 0 ] = hObject->GetAniHandle()->GetBoneIndex( "Bip01 Spine1" );
	m_nHeadLookBoneIndex[ 1 ] = hObject->GetAniHandle()->GetBoneIndex( "Bip01 Head" );
}

void MAHeadLookController::OnSignalHeadLook( HeadLookStruct *pStruct )
{
	m_nHeadLookResopneLength = pStruct->nResponeLenth;
	m_nHeadLookRotateSpeed = pStruct->nRotateSpeed;
	m_fHeadLookHeadRatio = pStruct->fHeadRatio;
	m_fHeadLookYAxisMaxAngle = pStruct->fYAxisMaxAngle;
	m_fHeadLookXAxisMaxAngle = pStruct->fXAxisMaxAngle;
	m_bHeadLookYAxisReverse = ( pStruct->bReverseYAxis == TRUE );
	m_bHeadLookXAxisReverse = ( pStruct->bReverseXAxis == TRUE );

	m_nHeadLookResopneLength = pStruct->nResponeLenth;
	m_nHeadLookRotateSpeed = pStruct->nRotateSpeed;
	m_fHeadLookHeadRatio = pStruct->fHeadRatio;
	m_fHeadLookYAxisMaxAngle = pStruct->fYAxisMaxAngle;
	m_fHeadLookXAxisMaxAngle = pStruct->fXAxisMaxAngle;
	m_bHeadLookYAxisReverse = ( pStruct->bReverseYAxis == TRUE );
	m_bHeadLookXAxisReverse = ( pStruct->bReverseXAxis == TRUE );

}

void MAHeadLookController::ProcessHeadLook( MatrixEx &Cross, float fDelta, bool bSignalRange )
{
	if( m_nHeadLookBoneIndex[0] == -1 && m_nHeadLookBoneIndex[1] == -1 ) return;
	if( m_vHeadLook.x != m_vCurHeadLook.x ) {
		if( m_vCurHeadLook.x < m_vHeadLook.x ) {
			m_vCurHeadLook.x += ( m_nHeadLookRotateSpeed * fDelta ) * ( ( 1.f / 30.f ) * abs( m_vHeadLook.x - m_vCurHeadLook.x ) );
			if( m_vCurHeadLook.x >= m_vHeadLook.x ) m_vCurHeadLook.x = m_vHeadLook.x;
		}
		else {
			m_vCurHeadLook.x -= ( m_nHeadLookRotateSpeed * fDelta ) * ( ( 1.f / 30.f ) * abs( m_vHeadLook.x - m_vCurHeadLook.x ) );
			if( m_vCurHeadLook.x <= m_vHeadLook.x ) m_vCurHeadLook.x = m_vHeadLook.x;
		}
	}

	if( m_vHeadLook.y != m_vCurHeadLook.y ) {
		if( m_vCurHeadLook.y < m_vHeadLook.y ) {
			m_vCurHeadLook.y += ( m_nHeadLookRotateSpeed * fDelta ) * ( ( 1.f / 30.f ) * abs( m_vHeadLook.y - m_vCurHeadLook.y ) );
			if( m_vCurHeadLook.y >= m_vHeadLook.y ) m_vCurHeadLook.y = m_vHeadLook.y;
		}
		else {
			m_vCurHeadLook.y -= ( m_nHeadLookRotateSpeed * fDelta ) * ( ( 1.f / 30.f ) * abs( m_vHeadLook.y - m_vCurHeadLook.y ) );
			if( m_vCurHeadLook.y <= m_vHeadLook.y ) m_vCurHeadLook.y = m_vHeadLook.y;
		}
	}

	if( m_hHeadLookObject ) {
		if( m_vCurHeadLook.x != m_vCurHeadLook.x ) m_vCurHeadLook.x = 0.f;
		if( m_vCurHeadLook.y != m_vCurHeadLook.y ) m_vCurHeadLook.y = 0.f;
		EtVector3 vTemp = m_vCurHeadLook;
		if( m_bHeadLookYAxisReverse ) vTemp.y = -vTemp.y;
		if( m_bHeadLookXAxisReverse ) vTemp.x = -vTemp.x;
		float fRatio = m_fHeadLookHeadRatio;
		if( m_nHeadLookBoneIndex[0] != -1 )
			m_hHeadLookObject->SetBoneRotation( m_nHeadLookBoneIndex[0], vTemp * ( 1.f - m_fHeadLookHeadRatio ) );
		else fRatio = 1.f;
		if( m_nHeadLookBoneIndex[1] != -1 ) 
			m_hHeadLookObject->SetBoneRotation( m_nHeadLookBoneIndex[1], vTemp * fRatio );

		// Set HeadScale regardless of signals.
		if( m_fHeadScale != 1.0f )
			m_hHeadLookObject->SetBoneScale( m_nHeadLookBoneIndex[1], m_fHeadScale );
	}
	else {
		if( m_hHeadLookObject != CEtObject::Identity() )
		{
			CDnActor *pActor = dynamic_cast<CDnActor*>(this);
			if( pActor && pActor->GetObjectHandle() )
				m_hHeadLookObject = pActor->GetObjectHandle();
		}
	}

	/////////////////
	if( bSignalRange ) {

		EtVector3 vTargetPos;
		if( m_hHeadLookTargetActor ) vTargetPos = *m_hHeadLookTargetActor->GetPosition();
		else if( EtVec3LengthSq( &m_vHeadLookTargetPos ) > 0.f ) vTargetPos = m_vHeadLookTargetPos;
		else {
			m_vHeadLook = EtVector3( 0.f, 0.f, 0.f );
			return;
		}

		EtVector3 vTemp = vTargetPos - Cross.m_vPosition;
		float fLength = EtVec3LengthSq( &vTemp );
		if( fLength > (float)( m_nHeadLookResopneLength * m_nHeadLookResopneLength ) ) {
			m_vHeadLook = EtVector3( 0.f, 0.f, 0.f );
		}
		else {
			EtVector2 vVec2[2];
			EtVector3 vYVec;
			float fDot;
			EtVec3Normalize( &vTemp, &vTemp );

			if( m_fHeadLookYAxisMaxAngle != 0.f ) {
				vVec2[0] = EtVec3toVec2( Cross.m_vZAxis );
				vVec2[1] = EtVec3toVec2( vTemp );
				EtVec2Normalize( &vVec2[0], &vVec2[0] );
				EtVec2Normalize( &vVec2[1], &vVec2[1] );
				fDot = EtVec2Dot( &vVec2[0], &vVec2[1] );
				if( fDot >= 1.f ) m_vHeadLook.y = 0.f;
				else {
					m_vHeadLook.y = EtToDegree( EtAcos( fDot ) );
					EtVec3Cross( &vYVec, &Cross.m_vZAxis, &vTemp );
					if( m_vHeadLook.y > m_fHeadLookYAxisMaxAngle ) m_vHeadLook.y = m_fHeadLookYAxisMaxAngle;
					if( vYVec.y < 0.f ) m_vHeadLook.y = -m_vHeadLook.y;
				}
			}

			if( m_fHeadLookXAxisMaxAngle != 0.f ) {
				//				vVec2[0] = EtVector2( vTemp.y, abs(vTemp.z) );
				//				vVec2[1] = EtVector2( m_matExWorld.m_vZAxis.y, abs(m_matExWorld.m_vZAxis.z) );
				//				vVec2[0] = EtVector2( m_matExWorld.m_vYAxis.y, m_matExWorld.m_vYAxis.z );
				EtVector3 vTemp2, vTemp3;
				EtVec3Cross( &vTemp3, &vTemp, &EtVector3( 0.f, 1.f, 0.f ) );
				EtVec3Cross( &vTemp2, &vTemp3, &vTemp );
				//				vVec2[1] = EtVector2( vTemp2.y, vTemp2.z );
				//				vVec2[1] = EtVector2( 0.f, vTemp.y );
				//				EtVec2Normalize( &vVec2[0], &vVec2[0] );
				//				EtVec2Normalize( &vVec2[1], &vVec2[1] );
				//				fDot = EtVec2Dot( &vVec2[0], &vVec2[1] );
				fDot = EtVec3Dot( &Cross.m_vYAxis, &vTemp2 );
				if( fDot >= 1.f ) m_vHeadLook.x = 0.f;
				else {
					m_vHeadLook.x = EtToDegree( EtAcos( fDot ) );
					if( m_vHeadLook.x > m_fHeadLookXAxisMaxAngle ) m_vHeadLook.x = m_fHeadLookXAxisMaxAngle;
					//					if( abs(m_vHeadLook.y) == m_fHeadLookYAxisMaxAngle ) m_vHeadLook.x = 0.f;

					if( vTargetPos.y > Cross.m_vPosition.y ) 
						m_vHeadLook.x = -m_vHeadLook.x;
				}
			}
		}
	}
	else m_vHeadLook = EtVector3( 0.f, 0.f, 0.f );
}

void MAHeadLookController::SetHeadLookTarget( EtVector3 &vPos )
{
	m_vHeadLookTargetPos = vPos;
	m_hHeadLookTargetActor.Identity();
}

void MAHeadLookController::SetHeadLookTarget( DnActorHandle hActor )
{
	m_hHeadLookTargetActor = hActor;
	m_vHeadLookTargetPos = EtVector3( 0.f, 0.f, 0.f );
}

void MAHeadLookController::SetHeadScale( float fScale )
{
	m_fHeadScale = fScale;
}
