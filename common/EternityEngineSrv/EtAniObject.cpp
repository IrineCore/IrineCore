#include "stdafx.h"
#include "EtAniObject.h"

using namespace EternityEngine;

CEtAniObject::CEtAniObject( CMultiRoom *pRoom )
: CEtObject( pRoom )
{
	m_bCalcAni = false;	
	m_nCalcPositionFlag = CALC_POSITION_Y;
	memset(&m_AniInfo, 0, sizeof(m_AniInfo));
}

CEtAniObject::~CEtAniObject(void)
{
	if( m_hAni )
	{
		m_hAni->Release();
	}
}

int CEtAniObject::Initialize( EtSkinHandle hSkin, EtAniHandle hAni )
{
	m_hSkin = hSkin;
	m_hAni = hAni;
	if( m_hAni )
	{
		if( m_hSkin )
		{
			hSkin->GetMeshHandle()->LinkToAni( m_hAni );
			if( hSkin->GetMeshHandle()->HasCollisionBone() )
			{
				m_bCalcAni = true;
			}
		}
	}

	if( m_hSkin ) {
		CommonInitialize();
		SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
		SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) );
	}

	return ET_OK;
}

EtMatrix *CEtAniObject::GetBoneTransMat( int nBoneIndex )
{
	if( !m_bCalcAni )
	{
		ASSERT( 0 && "Invalid Function Call" );
		return NULL;
	}

	EtMatrix BoneWorldMat;
	static EtMatrix BoneMat;

	if( m_vecTransMat.empty() )
	{
		CalcAni();
	}

	std::vector< EtMatrix > &vecInvWordlList = GetMesh()->GetInvWorldMatList();
	EtMatrixInverse( &BoneWorldMat, NULL, &vecInvWordlList[ nBoneIndex ] );
	if( nBoneIndex >= m_vecTransMat.size() )		
		return NULL;
	return EtMatrixMultiply( &BoneMat, &BoneWorldMat, &m_vecTransMat[ nBoneIndex ] );
}

EtMatrix *CEtAniObject::GetBoneMat( int nBoneIndex )
{
	if( !m_bCalcAni )
	{
		ASSERT( 0 && "Invalid Function Call" );
		return NULL;
	}
	if( m_vecTransMat.empty() )
	{
		CalcAni();
	}

	return &m_vecTransMat[ nBoneIndex ];
}

void CEtAniObject::SetAniFrame( int nAni, float fFrame )
{
	ASSERT( fFrame >= 0.0f );

	m_AniInfo.nAni = nAni;
	m_AniInfo.fFrame = fFrame;

	if( m_bCalcAni )
	{
		CalcAni();
	}
}

void CEtAniObject::CalcAni()
{
	if( !m_hAni )
	{
		return;
	}

	m_hAni->Reset();
	m_hAni->SetCalcPositionFlag( m_nCalcPositionFlag );
	m_hAni->SetAni( &m_AniInfo );
	m_hAni->CalcAni( m_hSkin->GetMeshHandle()->GetInvWorldMatList(), m_vecTransMat );
}
