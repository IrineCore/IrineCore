#include "StdAfx.h"
#include "DnAniCamera.h"
#include "EtResourceMng.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAniCamera::CDnAniCamera()
: m_fTotalFrame( 0.0f )
, m_StartTime( 0 )
, m_nCurrentActionIndex( -1 )
, m_nNextActionIndex( -1 )
, m_fFrame( 0.0f )
{
}

CDnAniCamera::~CDnAniCamera()
{
	SAFE_DELETE_PVEC( m_pVecAniList );
}

CDnAniCamera::AniCameraStruct *CDnAniCamera::LoadCameraData( const char *szFileName )
{
	CResMngStream Stream( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str() );
	if( !Stream.IsValid() ) return NULL;

	AniCameraStruct *pStruct = new AniCameraStruct;

	Stream.Read( &pStruct->Header, sizeof(SCameraFileHeader) );
	Stream.Seek( CAMERA_HEADER_RESERVED, SEEK_CUR );

	pStruct->VecFOV.resize( pStruct->Header.nFOVKeyCount );
	pStruct->VecPosition.resize( pStruct->Header.nPositionKeyCount );
	pStruct->VecRotation.resize( pStruct->Header.nRotationKeyCount );

	if( pStruct->Header.nFOVKeyCount )
		Stream.Read( &pStruct->VecFOV[0], pStruct->Header.nFOVKeyCount * sizeof(SCameraFOVKey) );
	if( pStruct->Header.nPositionKeyCount )
		Stream.Read( &pStruct->VecPosition[0], pStruct->Header.nPositionKeyCount * sizeof(SCameraPositionKey) );
	if( pStruct->Header.nRotationKeyCount )
		Stream.Read( &pStruct->VecRotation[0], pStruct->Header.nRotationKeyCount * sizeof(SCameraRotationKey) );

	return pStruct;
}

bool CDnAniCamera::AddCameraData( const char *szFileName )
{
	AniCameraStruct *pStruct = LoadCameraData( szFileName );
	if( !pStruct ) return false;

	char szTemp[256] = { 0, };
	_GetFileName( szTemp, _countof(szTemp), szFileName );
	pStruct->szName = szTemp;

	m_pVecAniList.push_back( pStruct );
	return true;
}

int CDnAniCamera::GetCameraIndex( const char *szName )
{
	for( DWORD i=0; i<m_pVecAniList.size(); i++ ) {
		if( strcmp( m_pVecAniList[i]->szName.c_str(), szName ) == NULL ) {
			return i;
		}
	}
	return -1;
}

bool CDnAniCamera::Activate()
{
	m_hCamera = EternityEngine::CreateCamera( &m_CameraInfo );
	m_hCamera->Activate();

	return ( m_hCamera ) ? true : false;
}

bool CDnAniCamera::DeActivate()
{
	CDnCamera::DeActivate();
	SAFE_RELEASE_SPTR( m_hCamera );
	return true;
}

void CDnAniCamera::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nCurrentActionIndex < 0 || m_nCurrentActionIndex >= (int)m_pVecAniList.size() ) return;

	AniCameraStruct *pStruct = m_pVecAniList[m_nCurrentActionIndex];
	if( !pStruct ) return;

	if( m_StartTime == 0 ) m_StartTime = LocalTime;

	float fFrame = ( ( LocalTime - m_StartTime ) / 1000.f ) * 60.f;

	if( fFrame > m_fTotalFrame ) 
	{
		if( m_nNextActionIndex != -1 )
		{
			PlayCamera( m_szNextAction.c_str(), NULL );
			if( m_nCurrentActionIndex < 0 || m_nCurrentActionIndex >= (int)m_pVecAniList.size() ) return;
			fFrame = fFrame - m_fTotalFrame;
			m_StartTime = LocalTime - (LOCAL_TIME)( ( fFrame - m_fTotalFrame ) / 60.f );
			pStruct = m_pVecAniList[m_nCurrentActionIndex];
		}
		else
		{
			fFrame = m_fTotalFrame;
		}
	}
	m_fFrame = fFrame;

	// Pos, Rot
	EtVector3 vPos;
	EtQuat qQuat;
	float fFov;
	pStruct->CalcPosition( vPos, fFrame );
	pStruct->CalcRotation( qQuat, fFrame );
	pStruct->CalcFov( fFov, fFrame );

	EtMatrix matTemp;
	EtMatrixIdentity( &matTemp );
	EtMatrixTransformation( &matTemp, NULL, NULL, NULL, NULL, &qQuat, &vPos );

	memcpy( &m_matExWorld.m_vXAxis, &matTemp._11, sizeof(EtVector3) );
	memcpy( &m_matExWorld.m_vYAxis, &matTemp._21, sizeof(EtVector3) );
	memcpy( &m_matExWorld.m_vZAxis, &matTemp._31, sizeof(EtVector3) );
	memcpy( &m_matExWorld.m_vPosition, &matTemp._41, sizeof(EtVector3) );

#ifdef PRE_MOD_SELECT_CHAR
	ProcessEffect( LocalTime, fDelta );
	m_hCamera->SetFOV( fFov );
#endif // PRE_MOD_SELECT_CHAR
	m_hCamera->Update( m_matExWorld );
}

void CDnAniCamera::PlayCamera( const char *szName, const char *szNextName, bool bLoop )
{
	m_nCurrentActionIndex = -1;
	m_nNextActionIndex = -1;

	m_szAction = szName;
	if( szNextName != NULL )
	{
		m_szNextAction = szNextName;
	}		
	else
	{
		if( bLoop )
			m_szNextAction = m_szAction;
		else
			m_szNextAction = "";
	}

	if( m_szAction.length() > 0 )
		m_nCurrentActionIndex = GetCameraIndex( m_szAction.c_str() );
	if( m_szNextAction.length() > 0 )
		m_nNextActionIndex = GetCameraIndex( m_szNextAction.c_str() );

	if( m_nCurrentActionIndex >= 0 && m_nCurrentActionIndex < (int)m_pVecAniList.size() ) 
	{
		m_fTotalFrame = (float)( m_pVecAniList[m_nCurrentActionIndex]->Header.nFrame );//* 60.f ) / 1000.f;
	}
	else 
		m_fTotalFrame = 0.f;

	m_StartTime = 0;
}

float CDnAniCamera::GetTotalFrame( const char *szName )
{
	std::string strAction = szName;

	if( strAction.length() == 0 )
		return 0.0f;

	int nActionIndex = GetCameraIndex( strAction.c_str() );
	if( nActionIndex >= 0 && nActionIndex < (int)m_pVecAniList.size() )
		return (float)( m_pVecAniList[nActionIndex]->Header.nFrame );

	return 0.0f;
}

void CDnAniCamera::AniCameraStruct::CalcPosition( EtVector3 &Position, float fFrame )
{
	if( fFrame < 0.f ) fFrame = 0.f;
	if( VecPosition.empty() )
	{
		return;
	}
	if( VecPosition.size() == 1 )
	{
		Position = VecPosition[ 0 ].vPosition;
		return;
	}

	int nFrame;
	int nRangeStart, nRangeEnd, nRangeCur;
	float fWeight;

	nRangeStart = 0;
	nRangeEnd = ( int )VecPosition.size() - 1;
	nRangeCur = ( int )( fFrame / VecPosition[ nRangeEnd ].nTime * nRangeEnd );
	nFrame = ( int )fFrame;
	while( 1 )
	{
		// Handling exception due to invalid exported data. Let's fix the exporter side later
		// by mapping
		if( nRangeCur >= ( int )VecPosition.size() - 1 )
		{
			Position = VecPosition[ VecPosition.size() - 1 ].vPosition;
			return;
		}

		if( nFrame == VecPosition[ nRangeCur ].nTime )
		{
			fWeight = ( fFrame - VecPosition[ nRangeCur ].nTime ) / ( VecPosition[ nRangeCur + 1 ].nTime - VecPosition[ nRangeCur ].nTime );
			EtVec3Lerp( &Position, &VecPosition[ nRangeCur ].vPosition, 
				&VecPosition[ nRangeCur + 1 ].vPosition, fWeight );
			return;
		}
		if( nFrame < VecPosition[ nRangeCur ].nTime )
		{
			nRangeEnd = nRangeCur - 1;
		}
		else
		{
			if( nFrame < VecPosition[ nRangeCur + 1 ].nTime )
			{
				fWeight = ( fFrame - VecPosition[ nRangeCur ].nTime ) / ( VecPosition[ nRangeCur + 1 ].nTime - VecPosition[ nRangeCur ].nTime );
				EtVec3Lerp( &Position, &VecPosition[ nRangeCur ].vPosition, 
					&VecPosition[ nRangeCur + 1 ].vPosition, fWeight );
				return;
			}
			nRangeStart = nRangeCur + 1;
		}
		nRangeCur = ( nRangeStart + nRangeEnd ) >> 1;
	}
}

void CDnAniCamera::AniCameraStruct::CalcRotation( EtQuat &Rotation, float fFrame )
{
	if( fFrame < 0.f ) fFrame = 0.f;
	if( VecRotation.empty() )
	{
		return;
	}
	if( VecRotation.size() == 1 )
	{
		Rotation = VecRotation[ 0 ].qRotation;
		return;
	}

	int nFrame;
	int nRangeStart, nRangeEnd, nRangeCur;
	float fWeight;

	nRangeStart = 0;
	nRangeEnd = ( int )VecRotation.size() - 1;
	nRangeCur = ( int )( fFrame / VecRotation[ nRangeEnd ].nTime * nRangeEnd );
	nFrame = ( int )fFrame;
	while( 1 )
	{
		// Handling exception due to invalid exported data. Let's fix the exporter side later
		// by mapping
		if( nRangeCur >= ( int )VecRotation.size() - 1 )
		{
			Rotation = VecRotation[ VecRotation.size() - 1 ].qRotation;
			return;
		}

		if( nFrame == VecRotation[ nRangeCur ].nTime )
		{
			fWeight = ( fFrame - VecRotation[ nRangeCur ].nTime ) / ( VecRotation[ nRangeCur + 1 ].nTime - VecRotation[ nRangeCur ].nTime );
			EtQuaternionSlerp( &Rotation, &VecRotation[ nRangeCur ].qRotation, 
				&VecRotation[ nRangeCur + 1 ].qRotation, fWeight );
			return;
		}
		if( nFrame < VecRotation[ nRangeCur ].nTime )
		{
			nRangeEnd = nRangeCur - 1;
		}
		else
		{
			if( nFrame < VecRotation[ nRangeCur + 1 ].nTime )
			{
				fWeight = ( fFrame - VecRotation[ nRangeCur ].nTime ) / ( VecRotation[ nRangeCur + 1 ].nTime - VecRotation[ nRangeCur ].nTime );
				EtQuaternionSlerp( &Rotation, &VecRotation[ nRangeCur ].qRotation, 
					&VecRotation[ nRangeCur + 1 ].qRotation, fWeight );
				return;
			}
			nRangeStart = nRangeCur + 1;
		}
		nRangeCur = ( nRangeStart + nRangeEnd ) >> 1;
	}
}

void CDnAniCamera::AniCameraStruct::CalcFov( float &fFov, float fFrame )
{
	if( fFrame < 0.f ) fFrame = 0.f;
	if( VecFOV.empty() )
	{
		fFov = 1.f;
		return;
	}
	if( VecFOV.size() == 1 )
	{
		fFov = 1.f;
		return;
	}

	int nFrame;
	int nRangeStart, nRangeEnd, nRangeCur;
	float fWeight;

	nRangeStart = 0;
	nRangeEnd = ( int )VecFOV.size() - 1;
	nRangeCur = ( int )( fFrame / VecFOV[ nRangeEnd ].nTime * nRangeEnd );
	nFrame = ( int )fFrame;
	while( 1 )
	{
		// Handling exception due to invalid exported data. Let's fix the exporter side later
		// by mapping
		if( nRangeCur >= ( int )VecFOV.size() - 1 )
		{
			fFov = VecFOV[ VecFOV.size() - 1 ].fFOV;
			return;
		}

		if( nFrame == VecFOV[ nRangeCur ].nTime )
		{
			fWeight = ( fFrame - VecFOV[ nRangeCur ].nTime ) / ( VecFOV[ nRangeCur + 1 ].nTime - VecFOV[ nRangeCur ].nTime );
			fFov = VecFOV[ nRangeCur ].fFOV + ( ( VecFOV[ nRangeCur + 1 ].fFOV - VecFOV[ nRangeCur ].fFOV ) * fWeight );
			return;
		}
		if( nFrame < VecFOV[ nRangeCur ].nTime )
		{
			nRangeEnd = nRangeCur - 1;
		}
		else
		{
			if( nFrame < VecFOV[ nRangeCur + 1 ].nTime )
			{
				fWeight = ( fFrame - VecFOV[ nRangeCur ].nTime ) / ( VecFOV[ nRangeCur + 1 ].nTime - VecFOV[ nRangeCur ].nTime );
				fFov = VecFOV[ nRangeCur ].fFOV + ( ( VecFOV[ nRangeCur + 1 ].fFOV - VecFOV[ nRangeCur ].fFOV ) * fWeight );
				return;
			}
			nRangeStart = nRangeCur + 1;
		}
		nRangeCur = ( nRangeStart + nRangeEnd ) >> 1;
	}
}

void CDnAniCamera::ResetAni()
{
	m_StartTime = 0;
}