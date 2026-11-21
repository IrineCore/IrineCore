#include "StdAfx.h"
#include "EtAniObject.h"
#include "EtSaveMat.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace EternityEngine;

CSyncLock CEtAniObject::s_CalcAniLock;

CEtAniObject::CEtAniObject(void)
{
	m_bShadowCast = true;
	m_bShadowReceive = false;
	m_nCalcPositionFlag = CALC_POSITION_Y;
	m_nSaveMatIndex = -1;
	EnableLightMapInfluence( true );
	m_bSkipPhysics = false;
	m_bForceSkipSimulateAni = false;

	m_nExtraAniIndex = -1;
	m_fExtraAniFrame = 0.0f;

	m_bAniObject = true;
}

CEtAniObject::~CEtAniObject(void)
{
	SAFE_RELEASE_SPTR( m_hAni );
}

int CEtAniObject::Initialize( EtSkinHandle hSkin, EtAniHandle hAni )
{
	bool bSkinReady, bAniReady;

	m_hSkin = hSkin;
	m_hAni = hAni;
	m_pSkinInstance = new CEtSkinInstance();

	bSkinReady = !m_hSkin->AddCallback( this );
	bAniReady = false;

	if( m_hAni )
	{
		bAniReady = !m_hAni->AddCallback( this );
		if( ( bSkinReady ) && ( bAniReady ) )
		{
			if( m_hSkin->GetMeshHandle() ) {
				m_hSkin->GetMeshHandle()->LinkToAni( m_hAni );
			}
		}
	}

	if( bSkinReady )
	{
		CommonInitialize();
	}

	SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 1 ) );
	SetTargetCollisionGroup( COLLISION_GROUP_STATIC( 1 ) );

	return ET_OK;
}

EtMatrix *CEtAniObject::GetBoneTransMat( int nBoneIndex )
{
	EtMatrix BoneWorldMat;
	static EtMatrix BoneMat;

	nBoneIndex = m_hAni->ConvertBoneIndex( nBoneIndex );
	if( !GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex ) )
	{
		CalcAni();
	}

	std::vector< EtMatrix > &vecInvWordlList = GetMesh()->GetInvWorldMatList();
	// Temp siva - changing to error handling.
	if( nBoneIndex < 0 || nBoneIndex >= (int)vecInvWordlList.size() ) {
		_ASSERT(0&&"Invalid bone index.");
	}
	else 
	{
		EtMatrixInverse( &BoneWorldMat, NULL, &vecInvWordlList[ nBoneIndex ] );

		if( GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex + nBoneIndex ) )
		{
			return EtMatrixMultiply( &BoneMat, &BoneWorldMat, GetEtSaveMat()->GetMatrix( m_nSaveMatIndex + nBoneIndex ) );
		}
	}
	EtMatrixIdentity( &BoneMat );

	return &BoneMat;
}

EtMatrix *CEtAniObject::GetDummyTransMat( int nDummyIndex )
{
	EtMeshHandle hMesh = m_hSkin->GetMeshHandle();
	if( hMesh )
	{
		std::vector< EtMatrix > &vecDummyMat = hMesh->GetDummyMatrixList();
		std::vector< std::string > &vecParentName = hMesh->GetDummyParentNameList();
		if( nDummyIndex < ( int )vecDummyMat.size() )
		{
			static EtMatrix ReturnMat;
			int nParentIndex = -1;
			if( m_hAni )
			{
				nParentIndex = m_hAni->GetBoneIndex( vecParentName[ nDummyIndex ].c_str() );
			}
			if( nParentIndex == -1 )
			{
				ReturnMat = vecDummyMat[ nDummyIndex ];
			}
			else
			{
				EtMatrixMultiply( &ReturnMat, &vecDummyMat[ nDummyIndex ], GetBoneTransMat( nParentIndex ) );
			}
			return &ReturnMat;
		}
	}

	return NULL;
}

EtMatrix *CEtAniObject::GetBoneMat( int nBoneIndex )
{
	nBoneIndex = m_hAni->ConvertBoneIndex( nBoneIndex );
	if( !GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex ) )
	{
		CalcAni();
	}

	if( GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex ) )
	{
		return GetEtSaveMat()->GetMatrix( m_nSaveMatIndex + nBoneIndex );
	}

	return NULL;
}

void CEtAniObject::ResetAniFrame()
{
	m_vecAniInfo.clear();
	m_vecBlendAniInfo.clear();
	m_vecDisableBone.clear();
	m_vecBoneRotation.clear();
	m_vecBoneScale.clear();
	m_nSaveMatIndex = -1;
}

void CEtAniObject::SetAniFrame( int nAni, float fFrame, int nBoneIndex )
{
	ASSERT( fFrame >= 0.0f );

	if( nBoneIndex == 0 )	// If the bone being set is the root bone, initialize all animations.
	{
		ResetAniFrame();
	}

	SAniInfo AniInfo;

	if( nAni == -1 ) return;
	AniInfo.BoneAniInfo.nAni = nAni;
	AniInfo.BoneAniInfo.fFrame = fFrame;
	AniInfo.nBoneIndex = nBoneIndex;
	m_vecAniInfo.push_back( AniInfo );
}

void CEtAniObject::BlendAniFrame( int nBlendAni, float fBlendFrame, float fBlendWeight, int nBlendBoneIndex )
{
	SAniInfo AniInfo;

	if( nBlendAni == -1 ) return;
	if( fBlendWeight < 0.0f || fBlendWeight > 10.0f )
	{
		ASSERT( 0 && "Invalid blend weight value" );
		return;
	}
	AniInfo.BoneAniInfo.nAni = nBlendAni;
	AniInfo.BoneAniInfo.fFrame = fBlendFrame;
	AniInfo.BoneAniInfo.fWeight = fBlendWeight;
	AniInfo.nBoneIndex = nBlendBoneIndex;
	m_vecBlendAniInfo.push_back( AniInfo );
	m_vecDisableBone.resize( m_vecBlendAniInfo.size() );
}

void CEtAniObject::BlendAniFrame( int nBlendAni, float fBlendFrame, float fBlendWeight, int nBlendBoneIndex, std::vector< int > &vecDisableBone )
{
	BlendAniFrame( nBlendAni, fBlendFrame, fBlendWeight, nBlendBoneIndex );
	int nBlendCount = ( int )m_vecBlendAniInfo.size();
	if( nBlendCount == 0 )
	{
		return;
	}
	if( !vecDisableBone.empty() )
	{
		m_vecDisableBone[ nBlendCount - 1 ].resize( vecDisableBone.size() );
		std::copy( vecDisableBone.begin(), vecDisableBone.end(), m_vecDisableBone[ nBlendCount - 1 ].begin() );
	}
}

void CEtAniObject::BlendAniFrame( int nBlendAni, float fBlendFrame, float fBlendWeight, int nBlendBoneIndex, int nDisableBoneIndex )
{
	BlendAniFrame( nBlendAni, fBlendFrame, fBlendWeight, nBlendBoneIndex );
	int nBlendCount = ( int )m_vecBlendAniInfo.size();
	if( nBlendCount == 0 )
	{
		return;
	}
	if( nDisableBoneIndex != -1 )
	{
		m_vecDisableBone[ nBlendCount - 1 ].insert( m_vecDisableBone[ nBlendCount - 1 ].begin(), nDisableBoneIndex );
	}
	// Even if CalcAni() was called earlier and m_nSaveMatIndex is valid, if BlendAni or SetBoneRotation is called in between 
	// Indicates m_nSaveMatIndex is invalid; reason for doing this is if m_nSaveMatIndex is changed to -1, during animation calculation
	// Memory may continue to be allocated.
	m_bValidSaveMatIndex = false;
}

void CEtAniObject::SetBoneRotation( int nBoneIndex, EtVector3 &BoneRotation )
{
	SBoneRotationInfo BoneRotationInfo;

	if( nBoneIndex == -1 ) {
		ASSERT( 0 && "Rotate Bone Index -1" );
		return;
	}
	BoneRotationInfo.nBoneIndex = nBoneIndex;
	BoneRotationInfo.BoneRotation = BoneRotation;
	m_vecBoneRotation.push_back( BoneRotationInfo );
	// Even if CalcAni() was called earlier and m_nSaveMatIndex is valid, if BlendAni or SetBoneRotation is called in between 
	// Indicates m_nSaveMatIndex is invalid; reason for doing this is if m_nSaveMatIndex is changed to -1, during animation calculation
	// Memory may continue to be allocated.
	m_bValidSaveMatIndex = false;
}

void CEtAniObject::SetBoneScale( int nBoneIndex, float fScale )
{
	SBoneScaleInfo BoneScaleInfo;

	if( nBoneIndex == -1 ) {
		ASSERT( 0 && "Scale Bone Index -1" );
		return;
	}
	BoneScaleInfo.nBoneIndex = nBoneIndex;
	BoneScaleInfo.fScale = fScale;
	m_vecBoneScale.push_back( BoneScaleInfo );
	m_bValidSaveMatIndex = false;
}

void CEtAniObject::ReBuildSkin( EtSkinHandle hSkin )
{	
	if( ! hSkin )	return;

	SAFE_RELEASE_SPTR( m_hSkin );
	SAFE_DELETE( m_pSkinInstance );

	bool bSkinReady, bAniReady;
	m_hSkin = hSkin;
	m_pSkinInstance = new CEtSkinInstance();

	bSkinReady = !m_hSkin->AddCallback( this );
	bAniReady = false;

	if( m_hAni )
	{
		bAniReady = !m_hAni->AddCallback( this );
		if( ( bSkinReady ) && ( bAniReady ) )
		{
			if( m_hSkin->GetMeshHandle() ) {
				m_hSkin->GetMeshHandle()->LinkToAni( m_hAni );
			}
		}
	}

	if( bSkinReady )
	{
		CommonInitialize();
	}
}

void CEtAniObject::CalcAni()
{
	// Sometimes called concurrently from another thread.. Must prevent this..
	ScopeLock< CSyncLock > Lock( s_CalcAniLock );

	if( !m_hAni )
	{
		return;
	}

	int i, j;

	m_hAni->Reset();
	m_hAni->SetCalcPositionFlag( m_nCalcPositionFlag );
	if( m_nExtraAniIndex != -1 )
	{
		SBoneAniInfo BoneInfo;
		BoneInfo.nAni = m_nExtraAniIndex;
		BoneInfo.fFrame = m_fExtraAniFrame;
		BoneInfo.fWeight = 0.0f;
		m_hAni->SetAni( &BoneInfo, 0 );
	}
	else
	{
		for( i = 0; i < ( int )m_vecAniInfo.size(); i++ )
		{
			m_hAni->SetAni( &m_vecAniInfo[ i ].BoneAniInfo, m_vecAniInfo[ i ].nBoneIndex );
		}
		for( i = 0; i < ( int )m_vecBlendAniInfo.size(); i++ )
		{
			for( j = 0; j < ( int )m_vecDisableBone[ i ].size(); j++ )
			{
				m_hAni->EnableBlend( m_vecDisableBone[ i ][ j ], false );
			}
			m_hAni->BlendAni( &m_vecBlendAniInfo[ i ].BoneAniInfo, m_vecBlendAniInfo[ i ].nBoneIndex );
			for( j = 0; j < ( int )m_vecDisableBone[ i ].size(); j++ )
			{
				m_hAni->EnableBlend( m_vecDisableBone[ i ][ j ], true );
			}
		}
		for( i = 0; i < ( int )m_vecBoneRotation.size(); i++ )
		{
			m_hAni->SetBoneRotation( m_vecBoneRotation[ i ].nBoneIndex, &m_vecBoneRotation[ i ].BoneRotation );
		}
		for( i = 0; i < ( int )m_vecBoneScale.size(); i++ )
		{
			m_hAni->SetBoneScale( m_vecBoneScale[ i ].nBoneIndex, m_vecBoneScale[ i ].fScale );
		}
	}

	m_nSaveMatIndex = m_hAni->CalcAni( GetMesh()->GetInvWorldMatList() );
	m_bValidSaveMatIndex = true;
}

void CEtAniObject::CreateSimulation( const char *szFileName )
{
	std::map<std::string, int > boneIndexMap;
	int nBoneCount = m_hAni->GetBoneCount();
	for( int i = 0; i < nBoneCount; i++) {
		boneIndexMap[ m_hAni->GetBone(i)->GetName() ] = i;
	}
	m_Spring.Create( szFileName, GetMesh()->GetInvWorldMatList(), boneIndexMap );
}

void CEtAniObject::SimulateAni()
{
	// If animation is linked incorrectly, getting InvWorldMatList below will crash.
	if( m_hAni->GetBoneCount() > (int)(GetMesh()->GetInvWorldMatList().size()) )
		return;

	int i;
	m_Spring.Simulate( m_WorldMat, GetMesh()->GetInvWorldMatList() );

	int nBoneCount = m_hAni->GetBoneCount();

	EtMatrix *pLockedMatrices = NULL;
	m_nSaveMatIndex = GetEtSaveMat()->LockMatrix( nBoneCount, &pLockedMatrices);	
	for( i = 0; i < nBoneCount; i++) {
		EtMatrix TransMat, VertexTransMat;
		if( i == 0 ) {
			EtMatrixIdentity( &VertexTransMat );
			pLockedMatrices[ i ] = VertexTransMat;
		}
		else {
			EtMatrix InvWorldMat;
			EtMatrixInverse(&InvWorldMat, NULL, &m_WorldMat);
			int nChildOffset = 0;
			if( m_hAni->GetBone( i )->GetChildBoneCount() > 0 ) {
				nChildOffset = m_hAni->GetBone( i )->GetChild( 0 )->GetBoneIndex() - m_hAni->GetBone( i )->GetBoneIndex();
			}
			else {
				nChildOffset = m_hAni->GetBone( i )->GetParent()->GetBoneIndex() - m_hAni->GetBone( i )->GetBoneIndex();
			}			
			TransMat = m_Spring.GetMatrix( i, nChildOffset );
			EtMatrixMultiply( &TransMat, &TransMat, &InvWorldMat);
			EtMatrixMultiply( &VertexTransMat, &GetMesh()->GetInvWorldMatList()[i], &TransMat );
			pLockedMatrices[ i ] = VertexTransMat;
		}
	}
}

void CEtAniObject::ForceSkipSimulateAni( bool bForceSkip, bool bApplyChild )
{
	if( m_Spring.IsEnable() )
	{
		m_bForceSkipSimulateAni = bForceSkip;
	}

	if( bApplyChild )
	{
		for( int i = 0; i < ( int )m_vecChild.size(); i++ )
		{
			if( m_vecChild[ i ]->IsAniObject() )
				( ( EtAniObjectHandle )m_vecChild[ i ] )->ForceSkipSimulateAni( bForceSkip );
		}
	}
}

void CEtAniObject::InitRender( int nSaveMatIndex )
{
	if( !IsShow() )
	{
		return;
	}

	// #25479
	// Not when doing /go 404 in town, but when doing /go 404 again while already in-game
	// When EtInterface::Process( fDelta ); in GameTask Process finishes fade-out, initialize the next stage.
	// At this point, it creates both the monster and the flail object the monster is holding...
	// Because the EternityEngine::RenderFrame( fDelta ); line below executes before the Process call, the position value remains at its initial FLT_MAX state.
	// Matrix calculation in this state doesn't matter since it will be recalculated on the next call after the next Process call anyway, but
	// Since the simulation accumulates values, calculating even once with FLT_MAX leaves invalid values permanently.
	// So check if it is FLT_MAX as shown below, and if invalid, do not call SimulateAni.
	bool bInvalidPos = ( m_PrevWorldMat._11 == FLT_MAX );

	// Skip simulation if the valid top-level parent's world matrix value is abnormal.
	// Due to setting parent on a thread while mounting and forced InitRender calls in gauge Face, it seems to enter before Process runs.
	if( m_hParent )
	{
		EtObjectHandle hParent = m_hParent;
		while( 1 )
		{
			if( hParent->GetParent() )
				hParent = hParent->GetParent();
			else
				break;
		}

		if( hParent->GetWorldMat()->_41 == FLT_MAX )
			bInvalidPos = true;
	}

	int i;

	if( m_hParent )
	{
		CalcParentBoneMat();
	}

	if( m_hAni )
	{
		if( m_Spring.IsEnable() ) {
			if( m_bSkipPhysics ) {
				CalcAni();
				m_Spring.InputKeyframe( m_WorldMat, m_nSaveMatIndex );
			}
			else {
				if( m_bForceSkipSimulateAni == false && bInvalidPos == false )
					SimulateAni();
			}
		}
		else {
			if( m_hParent ) {
				if( m_hAni->GetMyIndex() != m_hParent->GetAniFileIndex() )
				{
					nSaveMatIndex = -1;
				}
				else if( m_nLinkBoneIndex != -1 )
				{
					// Briefly explaining SaveMatIndex.
					// The main purpose is to assign the same SaveMatIndex to parts sharing the same ani file so animation calculations are not repeated multiple times,
					// (Usually the same character's head, hair, merge object, warrior gauntlets, etc. are children under one parent, so they share the same SaveMatIndex.)
					// When calling SetWorldMatArray in RenderStack::RenderBlock
					// Multiplies the Object's WorldMat by the configured SaveMat value and stores it in EtSaveMat::m_vecTransMat for use.
					//
					// However, checking only the Ani file above can cause problems.
					// If among Objects using the same SaveMatIndex
					// If one has m_nLinkBoneIndex == -1 and another has a specific bone index, the WorldMat of the two Objects will inevitably differ.
					// At this time, while entering the rendering routine, TransMat is computed once where RenderBlock is first called, and then
					// Afterward that value is shared, making it appear wrongly linked to a specific object.
					// So like this, if LinkBoneIndex is not -1, invalidate SaveMatIndex passed from the parent and
					// Use the self-calculated m_nSaveMatIndex.
					nSaveMatIndex = -1;
				}
			}
			bool bMySaveMatInvalid = false;
			if( ( !GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex ) ) || ( !m_bValidSaveMatIndex ) )
			{
				bMySaveMatInvalid = true;
			}
			if( ( !GetEtSaveMat()->IsValidIndex( nSaveMatIndex ) ) && ( bMySaveMatInvalid ) )
			{
				CalcAni();
			}
			else if( !GetEtSaveMat()->IsValidIndex( m_nSaveMatIndex ) )
			{
				m_nSaveMatIndex = nSaveMatIndex;
			}
		}
	}

	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		m_vecChild[ i ]->InitRender( m_nSaveMatIndex );
	}
}

void CEtAniObject::OnLoadComplete( CBackgroundLoader *pLoader )
{
	if( m_hAni )
	{
		if( ( m_hSkin->IsReady() ) && ( m_hAni->IsReady() ) )
		{
			if( m_hSkin->GetMeshHandle() ) {
				m_hSkin->GetMeshHandle()->LinkToAni( m_hAni );
			}
		}
	}
	if( m_hSkin->IsReady() )
	{
		return CEtObject::OnLoadComplete( pLoader );
	}

	CBackgroundLoaderCallback::OnLoadComplete( pLoader );
}
