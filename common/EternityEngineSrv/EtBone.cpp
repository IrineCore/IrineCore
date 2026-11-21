#include "stdafx.h"
#include "EtBone.h"
#include "EtAniKey.h"

int CEtBone::s_CalcPositionFlag = CALC_POSITION_Y;

CEtBone::CEtBone(void)
{
	m_bRootBone = false;
	m_pParent = NULL;
}

CEtBone::~CEtBone(void)
{
	Clear();
}

void CEtBone::Clear()
{
	SAFE_DELETE_PVEC( m_vecAniKey );
}

void CEtBone::LoadBone( CStream *pStream, int nAniCount, int nAniVersion )
{
	int i;
	CEtAniKey *pAniKey;

	Clear();
	pStream->Read( &m_BoneHeader, sizeof( SBoneHeader ) );
	pStream->Seek( BONE_HEADER_RESERVED, SEEK_CUR );

	for( i = 0; i < nAniCount; i++ )
	{
		pAniKey = new CEtAniKey();
		pAniKey->LoadKey( pStream, nAniVersion );
		m_vecAniKey.push_back( pAniKey );
	}
}

void CEtBone::Reset()
{
	m_AniInfo.fFrame = 0.0f;
	m_AniInfo.nAni = 0;
}

void CEtBone::SetAni( SBoneAniInfo *pInfo )
{
	int i;

	m_AniInfo = *pInfo;
	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		m_vecChild[ i ]->SetAni( pInfo );
	}
}

void CEtBone::CalcAni()
{
	if( m_pParent )
	{
		EtMatrix AniMat;

		GetAniMatrix( &AniMat );
		EtMatrixMultiply( &m_TransMat, &AniMat, m_pParent->GetTransMat() );
	}
	else
	{
		GetAniMatrix( &m_TransMat );
	}
}

void CEtBone::GetAniMatrix( EtMatrix *pOutMat )
{
	EtVector3 Position1, Position2;
	EtQuat Rotation1;
	EtVector3 Scale;

	if( m_bRootBone )
	{
		m_vecAniKey[ m_AniInfo.nAni ]->CalcPosition( Position1, 0.0f );
		m_vecAniKey[ m_AniInfo.nAni ]->CalcPosition( Position2, m_AniInfo.fFrame );
		if( s_CalcPositionFlag & CALC_POSITION_X )
		{
			Position1.x = Position2.x;
		}
		if( s_CalcPositionFlag & CALC_POSITION_Y )
		{
			Position1.y = Position2.y;
		}
		if( s_CalcPositionFlag & CALC_POSITION_Z )
		{
			Position1.z = Position2.z;
		}
	}
	else
	{
		m_vecAniKey[ m_AniInfo.nAni ]->CalcPosition( Position1, m_AniInfo.fFrame );
	}
	m_vecAniKey[ m_AniInfo.nAni ]->CalcRotation( Rotation1, m_AniInfo.fFrame );
	m_vecAniKey[ m_AniInfo.nAni ]->CalcScale( Scale, m_AniInfo.fFrame );
	EtMatrixTransformation( pOutMat, NULL, NULL, &Scale, NULL, &Rotation1, &Position1 );
}

void CEtBone::GetAniRotationMatrix( int nAni, float fFrame, EtMatrix &AniMat )
{
	EtQuat Rotation;

	m_vecAniKey[ nAni ]->CalcRotation( Rotation, fFrame );
	EtMatrixRotationQuaternion( &AniMat, &Rotation );
}

void CEtBone::CalcAniDistance( int nAni, float fCurFrame, float fPrevFrame, EtVector3 &DistVec )
{
	EtVector3 Position1, Position2;

	m_vecAniKey[ nAni ]->CalcPosition( Position1, fPrevFrame );
	m_vecAniKey[ nAni ]->CalcPosition( Position2, fCurFrame );
	DistVec = Position2 - Position1;
}

void CEtBone::MergeBone( CEtBone *pMergeBone, int nAniCount )
{
	int i;

	for( i = 0; i < nAniCount; i++ )
	{
		CEtAniKey *pAniKey, *pMergeKey;

		pAniKey = new CEtAniKey();
		pMergeKey = pMergeBone->GetAniKey( i );
		pAniKey->CopyKey( pMergeKey );
		m_vecAniKey.push_back( pAniKey );
	}
}
