#include "stdafx.h"
#include "MAActorRenderBase.h"
#include "PerfCheck.h"

MAActorRenderBase::MAActorRenderBase()
: m_vAniDistance( 0.f, 0.f, 0.f )
, m_vAddAniDistance( 0.f, 0.f, 0.f )
, m_vScale( 1.f, 1.f, 1.f )
{
	ResetActorRenderBase();
}

MAActorRenderBase::~MAActorRenderBase()
{
}

void MAActorRenderBase::ResetActorRenderBase()
{
	m_LocalTime = 0;
	m_AniTime = 0;
	m_nAniIndex = -1;

	m_nBlendAniIndex = -1;
	m_fBlendAniFrame = 0.f;
	m_fBlendFrame = 0.f;
	m_fBlendStartFrame = 0.f;
	m_fBlendCurFrame = 0.f;
	m_fFrame = m_fPrevFrame = 0.f;

	m_vAniDistance = EtVector3( 0.f, 0.f, 0.f );
	m_vAddAniDistance = EtVector3( 0.f, 0.f, 0.f );
	m_vScale = EtVector3( 1.f, 1.f, 1.f );

	m_fFPS = 60.f;
}

void MAActorRenderBase::ChangeAnimation( int nAniIndex, float fFrame, float fBlendFrame )
{
	m_nBlendAniIndex = m_nAniIndex;
	m_fBlendStartFrame = fFrame;
	m_fBlendFrame = fBlendFrame;
	if( m_fBlendFrame == 0.f ) m_nBlendAniIndex = -1;

	if( m_nBlendAniIndex != -1 ) {
		m_fBlendAniFrame = m_fFrame;

		float fAniLength = (float)m_hObject->GetLastFrame( m_nBlendAniIndex );
		if( m_fBlendAniFrame >= fAniLength ) {
			m_fBlendAniFrame = fAniLength;
		}
	}

	m_nAniIndex = nAniIndex;
	m_AniTime = m_LocalTime - (LOCAL_TIME)( fFrame / m_fFPS * 1000.f );
	if( m_AniTime < 0 ) m_AniTime = 0;
	m_fFrame = fFrame;
	m_fPrevFrame = fFrame;
}

int MAActorRenderBase::GetBoneIndex( const char *szBoneName )
{
	return m_hObject->GetAniHandle()->GetBoneIndex( szBoneName );
}


const char *MAActorRenderBase::GetBoneName( int nBoneIndex )
{
	if( nBoneIndex < 0 || nBoneIndex >= m_hObject->GetAniHandle()->GetBoneCount() ) return "";
	return m_hObject->GetAniHandle()->GetBone( nBoneIndex )->GetName();
}

void MAActorRenderBase::Process( EtMatrix *pmatWorld, LOCAL_TIME LocalTime, float fDelta )
{
	m_LocalTime = LocalTime;

	if( !m_hObject ) return;

	EtMatrix matTemp, matScale;
	EtMatrixScaling( &matScale, m_vScale.x, m_vScale.y, m_vScale.z );
	EtMatrixMultiply( &matTemp, &matScale, pmatWorld );

	m_hObject->Update( &matTemp );
}

void MAActorRenderBase::CalcAniDistance( int nAniIndex, float fFrame, float fPrevFrame, EtVector3 &vDist )
{
	vDist = EtVector3( 0.f, 0.f, 0.f );
	if( m_hObject ) m_hObject->CalcAniDistance( nAniIndex, fFrame, fPrevFrame, vDist );
}

void MAActorRenderBase::PreProcess( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nAniIndex == -1 ) return;
	if( !m_hObject || !m_hObject->GetAniHandle() ) return;
	if( m_nAniIndex >= m_hObject->GetAniCount()  ) return;

	// Calc Ani
	if( 0.0f < m_fFPS )
		m_fFrame = ( ( m_LocalTime - m_AniTime ) / 1000.f ) * m_fFPS;

	float fAniLength = (float)m_hObject->GetLastFrame( m_nAniIndex );

	if( m_fFrame < 0.f ) m_fFrame = fAniLength;

	if( m_fFrame >= fAniLength ) {
		m_fFrame = fAniLength;
	}
	m_hObject->SetAniFrame( m_nAniIndex, m_fFrame );

	// Calc Ani Distance
	CalcAniDistance( m_nAniIndex, m_fFrame, m_fPrevFrame, m_vAniDistance );
	m_vAniDistance += m_vAddAniDistance;
	m_vAddAniDistance = EtVector3( 0.f, 0.f, 0.f );
	if( m_hObject->GetCalcPositionFlag() & CALC_POSITION_X ) m_vAniDistance.x = 0.f;
	if( m_hObject->GetCalcPositionFlag() & CALC_POSITION_Y ) m_vAniDistance.y = 0.f;
	if( m_hObject->GetCalcPositionFlag() & CALC_POSITION_Z ) m_vAniDistance.z = 0.f;

	m_fPrevFrame = m_fFrame;
}

int MAActorRenderBase::GetAniIndex( const char *szAniName )
{
	if( !m_hObject || szAniName == NULL) return -1;
	for( int i=0; i<m_hObject->GetAniCount(); i++ ) {
		if( strcmp( m_hObject->GetAniName(i), szAniName ) == NULL ) return i;
	}
	return -1;
}

EtMatrix MAActorRenderBase::GetBoneMatrix( const char *szBoneName )
{
	EtMatrix mat;

	if( !m_hObject || szBoneName == NULL )
	{
		EtMatrixIdentity( &mat );
		return mat;
	}

	int nBoneIndex = m_hObject->GetAniHandle()->GetBoneIndex( szBoneName );
	
	if( nBoneIndex == -1 ) 
		return *m_hObject->GetWorldMat();;

	EtMatrix *pmatBoneTransMat = m_hObject->GetBoneTransMat( nBoneIndex );
	
	if( pmatBoneTransMat == NULL ) 
		return *m_hObject->GetWorldMat();

	EtMatrixMultiply( &mat, m_hObject->GetBoneTransMat( nBoneIndex ), m_hObject->GetWorldMat() );
	return mat;
}

void MAActorRenderBase::AddAniDistance( EtVector3 &vVec )
{
	m_vAddAniDistance += vVec;
}

EtVector3 *MAActorRenderBase::GetAniDistance()
{
	return &m_vAniDistance;
}

bool MAActorRenderBase::GetBoundingSphere( SSphere &Sphere )
{
	if( !m_hObject ) return false;
	m_hObject->GetBoundingSphere( Sphere );
	return true;
}

bool MAActorRenderBase::GetBoundingBox( SAABox &Box )
{
	if( !m_hObject ) return false;
	m_hObject->GetBoundingBox( Box );
	return true;
}

bool MAActorRenderBase::GetOriginalBoundingBox( SAABox &Box )
{
	if( !m_hObject ) return false;
	m_hObject->GetOriginalBoundingBox( Box );
	return true;
}

void MAActorRenderBase::SetFPS( float fValue )
{
	m_AniTime = m_LocalTime - (LOCAL_TIME)( m_fFrame / fValue * 1000.f );
	m_fFPS = fValue;
}

float MAActorRenderBase::GetFPS()
{
	return m_fFPS;
}

void MAActorRenderBase::SetFrame( float fValue )
{
	m_fFrame = fValue;
	m_AniTime = m_LocalTime - (LOCAL_TIME)( m_fFrame / m_fFPS * 1000.f );
	if( m_AniTime < 0 ) m_AniTime = 0;
}

void MAActorRenderBase::SetPrevFrame( float fValue )
{
	m_fPrevFrame = fValue;
}
