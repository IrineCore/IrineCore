#include "StdAfx.h"
#include "EtObject.h"
#include "EtConvexVolume.h"
#include "EtFindCollision.h"
#include "EtLoader.h"
#include "EtCustomParam.h"
#include "EtGenerateCollisionMesh.h"
#include "EtOptionController.h"
#include "EtCollisionMng.h"
#include "EtWater.h"
#include "EtEngine.h"
#include "EtFrustum.h"
#include "EtSkyBoxRTT.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

using namespace EternityEngine;

DECL_SMART_PTR_STATIC( CEtObject, 3000 )
CEtOctree< CEtObject * > *CEtObject::s_pDynamicOctree = NULL;
float CEtOctreeNode< CEtObject * >::s_fMinRadius = 1000.0f;
bool CEtObject::m_bSkipDrawCollision = false;

CEtObject::CEtObject(void)
: CSmartPtrBase< CEtObject >()
{
	SetChild( false );
	ShowObject( true );
	EnableObject( true );
	EnableCull( true );
	EnableShadowCast( false );
	m_bShadowReceive = false;
	m_bLightMapInfluence = false;
	EnableLightMapCast( false );
	EnableWaterCast( false );
	ShowBoundingBox( false );
	ShowCollisionPrimitive( false );
	EtMatrixIdentity( &m_WorldMat );
	m_WorldMat._41 = m_WorldMat._42 = m_WorldMat._43 = FLT_MAX;
	m_PrevWorldMat._11 = FLT_MAX;

	EtMatrixIdentity( &m_LinkOffsetMat );
	m_nLinkBoneIndex = -1;
	m_nSaveMatIndex = -1;
	m_bValidSaveMatIndex = false;
	m_pSkinInstance = NULL;

	m_fObjectAlpha = 1.0f;

	m_pCurOctreeNode = NULL;
	m_BoundingBox.Min = EtVector3( 0.f, 0.f, 0.f );
	m_BoundingBox.Max = EtVector3( 0.f, 0.f, 0.f );
	m_OriginalBoundingBox.Min = EtVector3( 0.f, 0.f, 0.f );
	m_OriginalBoundingBox.Max = EtVector3( 0.f, 0.f, 0.f );

	m_RenderType = RT_SCREEN;
	m_bAniObject = false;
	m_LinkType = LT_BONE;
	m_nFrustumMask = 0;
}

CEtObject::~CEtObject(void)
{
	if( m_hParent )
	{
		m_hParent->RemoveChild( GetMySmartPtr() );
	}
	if( m_pCurOctreeNode )
	{
		s_pDynamicOctree->Remove( this, m_pCurOctreeNode );
	}
	SAFE_DELETE( m_pSkinInstance );

	Clear();
}

void CEtObject::Clear()
{
	SAFE_RELEASE_SPTR( m_hSkin );
}

void CEtObject::InitializeClass()
{
	s_pDynamicOctree = new CEtOctree< CEtObject * >;
}

void CEtObject::FinalizeClass()
{
	SAFE_DELETE( s_pDynamicOctree );
}

int CEtObject::Initialize( EtSkinHandle hSkin )
{
	Clear();
	m_hSkin = hSkin;
	m_pSkinInstance = new CEtSkinInstance();

	if( !m_hSkin->AddCallback( this ) )
	{
		CommonInitialize();
	}

	return ET_OK;
}

void CEtObject::CommonInitialize()
{	CreateSkinInstance();

	if( m_hSkin->GetMeshHandle() )
		CEtCollisionEntity::Initialize( *m_hSkin->GetMeshHandle()->GetCollisionPrimitive(), *m_hSkin->GetMeshHandle()->GetCollisionPrimitiveParentIndex() );

	UpdateCollisionPrimitive( m_WorldMat );
	GetMeshBoundingBox( m_OriginalBoundingBox );
	GetMeshBoundingSphere( m_OriginalBoundingSphere );
}

void CEtObject::CreateSkinInstance()
{
	if( m_hSkin )
	{
		if( m_pSkinInstance == NULL )
		{
			m_pSkinInstance = new CEtSkinInstance();
		}
		m_pSkinInstance->CopySkinInfo( m_hSkin );
		if( m_pSkinInstance->IsSkyBox() )
		{
			CEtSkyBoxRTT::GetInstance().AddObject( m_MySmartPtr );
			CEtSkyBoxRTT::GetInstance().SetRenderFrameCount( -1 );
		}
	}
}

RenderType CEtObject::SetRenderType( RenderType Type )
{
	RenderType Ret = m_RenderType;
	m_RenderType = Type;
	return Ret;
}

void CEtObject::SetParent( EtObjectHandle hObject, const char *pLinkBoneName, EtMatrix *pOffsetMat )
{
	EtMeshHandle hMesh = hObject->GetMesh();
	int nLinkBoneIndex = -1;
	if( hMesh )
	{
		nLinkBoneIndex = hMesh->FindDummy( pLinkBoneName );
		if( nLinkBoneIndex != -1 )
		{
			SetParent( hObject, nLinkBoneIndex, pOffsetMat, LT_MESH_DUMMY );
			return;
		}
		nLinkBoneIndex = hMesh->FindBone( pLinkBoneName );
		if( nLinkBoneIndex != -1 )
		{
			SetParent( hObject, nLinkBoneIndex, pOffsetMat, LT_BONE );
			return;
		}
	}
}

void CEtObject::SetParent( EtObjectHandle hObject, int nBoneIndex, EtMatrix *pOffsetMat, LinkType Type )
{
	if ( hObject == GetMySmartPtr() )
	{
		ASSERT( false && "hObject == GetMySmartPtr()" );
		return;
	}

	// Dangerous if object is deleted while function is running!!!
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	if( m_hParent )
	{
		m_hParent->RemoveChild( GetMySmartPtr() );
	}
	if( hObject )
	{
		m_hParent = hObject;
		m_nLinkBoneIndex = nBoneIndex;
		m_LinkType = Type;
		if( pOffsetMat )
		{
			m_LinkOffsetMat = *pOffsetMat;
		}
		m_hParent->AddChild( GetMySmartPtr() );
		EnableShadowCast( m_hParent->IsShadowCast() );
		EnableShadowReceive( m_hParent->IsShadowReceive() );
		EnableLightMapInfluence( m_hParent->IsLightMapInfluence() );
		SetObjectAlpha( m_hParent->GetObjectAlpha() );
		SetChild( true );

		int nItemIndex, nParentItemIndex;

		nItemIndex = GetMyItemIndex();
		nParentItemIndex = m_hParent->GetMyItemIndex();
		if(  nItemIndex < nParentItemIndex )
		{
			SwapItemIndex( nItemIndex, nParentItemIndex );
		}
	}
	else
	{
		m_hParent.Identity();
		m_nLinkBoneIndex = -1;
		EtMatrixIdentity( &m_LinkOffsetMat );
		SetChild( false );
	}

}

void CEtObject::AddChild( EtObjectHandle hObject )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	m_vecChild.push_back( hObject );
	if( m_hSkin->IsReady() )
	{
		RecalcBoundingBox();
		RecalcBoundingSphere();
	}
}

void CEtObject::RemoveChild( EtObjectHandle hObject )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i;

	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		if( m_vecChild[ i ] == hObject )
		{
			m_vecChild.erase( m_vecChild.begin() + i );
			RecalcBoundingBox();
			RecalcBoundingSphere();
			return;
		}
	}
	ASSERT( 0 && "No child to delete" );
}

bool CEtObject::IsShow() 
{ 
	if( !m_hSkin ) return false;
	return m_bShow && m_hSkin->IsReady(); 
}

void CEtObject::EnableLightMapInfluence( bool bEnable ) 
{ 
	m_bLightMapInfluence = bEnable; 
	CalcLightMapInfluence();
}

void CEtObject::ShowSubmesh( int nSubmeshIndex, bool bShow )
{
	if( m_pSkinInstance )
	{
		m_pSkinInstance->GetSkinRenderInfo( nSubmeshIndex )->bShowSubmesh = bShow;
	}
}

bool CEtObject::IsShowSubmesh( int nSubmeshIndex ) 
{ 
	if( m_pSkinInstance )
	{
		return m_pSkinInstance->GetSkinRenderInfo( nSubmeshIndex )->bShowSubmesh;
	}
	return true;
}

void CEtObject::SetFrustumMask( int nMask ) 
{ 
	m_nFrustumMask = nMask; 
}

void CEtObject::AddFrustumMask( int nMask ) 
{ 
	m_nFrustumMask |= nMask; 
	int i, nCount;
	nCount = ( int )m_vecChild.size();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hObject = m_vecChild[ i ];
		hObject->AddFrustumMask( nMask );
	}
}

void CEtObject::EnableShadowCast( bool bEnable ) 
{ 
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	m_bShadowCast = bEnable; 

	int i, nCount;
	nCount = ( int )m_vecChild.size();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hObject = m_vecChild[ i ];
		hObject->EnableShadowCast( bEnable );
	}
}

void CEtObject::EnableShadowReceive( bool bEnable ) 
{ 
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	m_bShadowReceive = bEnable;

	int i, nCount;
	nCount = ( int )m_vecChild.size();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hObject = m_vecChild[ i ];
		hObject->EnableShadowReceive( bEnable );
	}
}

void CEtObject::SetObjectAlpha( float fAlpha ) 
{ 
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	m_fObjectAlpha = fAlpha; 

	int i, nCount;
	nCount = ( int )m_vecChild.size();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hObject = m_vecChild[ i ];
		hObject->SetObjectAlpha( fAlpha );
	}
}

void CEtObject::Update( EtMatrix *pWorldMat ) 
{ 
	SSphere Sphere;
	
	if( m_PrevWorldMat._11 == FLT_MAX )
	{
		m_PrevWorldMat = *pWorldMat;
		m_bUpdateNeeded = true;
	}
	else
	{
		m_PrevWorldMat = m_WorldMat;
		if( memcmp ( &m_PrevWorldMat, pWorldMat , sizeof(EtMatrix) ) != 0 ) {
			m_bUpdateNeeded = true;
		}
	}
	m_WorldMat = *pWorldMat;

	UpdateBoundingPrimitive();
	CalcLightInfluence();
	CalcLightMapInfluence();
	if( ( m_hSkin ) && ( !IsChild() ) )
	{
		UpdateCollisionPrimitive( m_WorldMat, m_bUpdateNeeded );
		GetBoundingSphere( Sphere );
		if( GetCollisionGroup() >= COLLISION_GROUP_DYNAMIC( 2 ) )	// Only dynamic collision types are put into the octree for checking; static ones use KdTree.
		{			
			if( m_pCurOctreeNode )
			{
				m_pCurOctreeNode = s_pDynamicOctree->Update( this, Sphere, m_pCurOctreeNode );
			}
			else
			{
				m_pCurOctreeNode = s_pDynamicOctree->Insert( this, Sphere );
			}
		}
	}
	m_bUpdateNeeded = false;
}

void CEtObject::CalcParentBoneMat()
{
	EtMatrix LinkMat, *pBoneMat;

	EtMatrixMultiply( &LinkMat, m_hParent->GetWorldMat(), &m_LinkOffsetMat );
	if( m_nLinkBoneIndex != -1 )
	{
		if( m_LinkType == LT_BONE )
		{
			pBoneMat = m_hParent->GetBoneTransMat( m_nLinkBoneIndex );
		}
		else
		{
			pBoneMat = m_hParent->GetDummyTransMat( m_nLinkBoneIndex );
		}
		if( pBoneMat )
		{
			EtMatrixMultiply( &LinkMat, pBoneMat, &LinkMat );
		}
	}
	Update( &LinkMat );
}

void CEtObject::InitRender( int nSaveMatIndex )
{	
	if( !IsShow() )
	{
		return;
	}

	if( m_hParent )
	{
		CalcParentBoneMat();
	}

	int i;

	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		m_vecChild[ i ]->InitRender( nSaveMatIndex );
	}
}

void CEtObject::Render()
{
	if( !IsShow() )
	{
		return;
	}
	if( IsShowBoundingBox() )
	{
		DrawBoundingBox();
	}

	if( m_pSkinInstance )
	{
		m_pSkinInstance->Render( m_WorldMat, m_PrevWorldMat, m_fObjectAlpha, m_vecInfluenceLight, m_bShadowReceive, m_nSaveMatIndex );
	}
}

void CEtObject::RenderImmediate( bool bRenderChild )
{
	if( !IsShow() )
	{
		return;
	}
	if( m_pSkinInstance )
	{
		m_pSkinInstance->RenderImmediate( m_WorldMat, m_WorldMat, m_fObjectAlpha, m_vecInfluenceLight, m_bShadowReceive, false, m_nSaveMatIndex );
	}

	if( bRenderChild )
	{
		int i;
		for( i = 0; i < ( int )m_vecChild.size(); i++ )
		{
			if( m_vecChild[ i ] )
			{
				m_vecChild[ i ]->RenderImmediate( bRenderChild );
			}
		}
	}
}

void CEtObject::RenderAlphaImmediate( bool bRenderChild )
{
	if( !IsShow() )
	{
		return;
	}
	if( m_pSkinInstance )
	{
		m_pSkinInstance->RenderImmediate( m_WorldMat, m_WorldMat, m_fObjectAlpha, m_vecInfluenceLight, m_bShadowReceive, true, m_nSaveMatIndex );
	}

	if( bRenderChild )
	{
		int i;
		for( i = 0; i < ( int )m_vecChild.size(); i++ )
		{
			if( m_vecChild[ i ] )
			{
				m_vecChild[ i ]->RenderAlphaImmediate( bRenderChild );
			}
		}
	}
}

void CEtObject::RenderShadow()
{
	if( !IsShow() )
	{
		return;
	}

	if( m_pSkinInstance )
	{
		m_pSkinInstance->RenderShadow( m_WorldMat, m_nSaveMatIndex );
	}
}

void CEtObject::RenderWater( int index )
{
	if( !IsShow() )
	{
		return;
	}

	if( m_pSkinInstance )
	{
		if( !m_hParent ) 
		{
			InitRender();
			m_pSkinInstance->RenderWater( index, m_WorldMat, m_PrevWorldMat, m_fObjectAlpha, m_vecInfluenceLight, m_bShadowReceive, m_nSaveMatIndex );
		}
	}
}

void CEtObject::SetSaveMatIndex( int nIndex ) 
{ 
	m_nSaveMatIndex = nIndex; 
	int i;
	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		m_vecChild[ i ]->SetSaveMatIndex( nIndex );
	}
}

void CEtObject::GetMeshBoundingBox( SAABox &AABox )
{
	if( m_hSkin )
	{
		EtMeshHandle hMesh = m_hSkin->GetMeshHandle();
		if( hMesh )
		{
			hMesh->GetBoundingBox( AABox ); 
		}
		return;
	}

	AABox.Min = EtVector3( -1.0f, -1.0f, -1.0f );
	AABox.Max = EtVector3( 1.0f, 1.0f, 1.0f );
}

void CEtObject::GetMeshBoundingSphere( SSphere &Sphere )
{ 
	if( m_hSkin )
	{
		EtMeshHandle hMesh = m_hSkin->GetMeshHandle();
		if( hMesh )
		{
			hMesh->GetBoundingSphere( Sphere ); 
		}
		return;
	}

	Sphere.Center = EtVector3( 0.0f, 0.0f, 0.0f );
	Sphere.fRadius = 1.0f;
}

void CEtObject::RecalcBoundingBox()
{
	SAABox BoundingBox;

	GetMeshBoundingBox( m_OriginalBoundingBox );
	// Server calculates without children, so delete to match sync with server. Might cause issues..
	/*	for( int i = 0; i < ( int )m_vecChild.size(); i++ )
	{
	m_vecChild[ i ]->GetMeshBoundingBox( BoundingBox );
	m_OriginalBoundingBox.AddPoint( BoundingBox.Max );
	m_OriginalBoundingBox.AddPoint( BoundingBox.Min );
	}*/
}

void CEtObject::RecalcBoundingSphere()
{
	int i;
	EtVector3 vDirection, vOriginalEnd, vChildEnd;
	SSphere BoundingSphere;

	GetMeshBoundingSphere( m_OriginalBoundingSphere );
	for( i = 0; i < ( int )m_vecChild.size(); i++ )
	{
		m_vecChild[ i ]->GetMeshBoundingSphere( BoundingSphere );
		if( m_OriginalBoundingSphere.IsInside( BoundingSphere ) )
		{
			continue;
		}
		vDirection = BoundingSphere.Center - m_OriginalBoundingSphere.Center;
		EtVec3Normalize( &vDirection, &vDirection );
		vOriginalEnd = m_OriginalBoundingSphere.Center - vDirection * m_OriginalBoundingSphere.fRadius;
		vChildEnd = BoundingSphere.Center + vDirection * BoundingSphere.fRadius;
		m_OriginalBoundingSphere.Center = ( vOriginalEnd + vChildEnd ) * 0.5f;
		m_OriginalBoundingSphere.fRadius = EtVec3Length( &( vOriginalEnd - vChildEnd ) ) * 0.5f;
	}
}

void CEtObject::DrawBoundingBox( DWORD dwColor )
{
	int i;
	static short s_LineIndices[] =
	{
		0, 1,  1, 3,  3, 2,  2, 0,
		4, 5,  5, 7,  7, 6,  6, 4,
		0, 4,  1, 5,  3, 7,  2, 6,
	};

	SOBB OBBox;

	OBBox.Init( m_OriginalBoundingBox, m_WorldMat );

	for( i = 0; i < sizeof( s_LineIndices ) / sizeof( short ) / 2; i++ )
	{
		DrawLine3D( OBBox.Vertices[ s_LineIndices[ i * 2 ] ], OBBox.Vertices[ s_LineIndices[ i * 2 + 1 ] ], dwColor );
	}
}

void CEtObject::UpdateBoundingPrimitive()
{
	if( !m_hSkin )
	{
		return;
	}

	if( !m_bUpdateNeeded )  {			// For Static Object, no need to recalculate every time.
		return;
	}

	if( !m_OriginalBoundingBox.IsValid() ) {
		return;
	}

	m_BoundingBox.Reset();
	SOBB OBBox;
	OBBox.Init( m_OriginalBoundingBox, m_WorldMat );
	for( int i = 0; i < 8; i++) {
		m_BoundingBox.AddPoint( OBBox.Vertices[i] );
	}

	m_BoundingSphere = m_OriginalBoundingSphere;
	EtVec3TransformCoord( &m_BoundingSphere.Center, &m_OriginalBoundingSphere.Center, &m_WorldMat );
	m_BoundingSphere.fRadius *= max( max( m_vScale.x, m_vScale.y ), m_vScale.z );
}

void CEtObject::UpdateCollisionPrimitive( EtMatrix &WorldMat, bool bUpdate )
{
	if( !IsEnableCollision() )
	{
		return;
	}

	int i;
	EtMatrix *pBoneMat, AniMat;

	for( i = 0; i < ( int )m_vecPrimitive.size(); i++ )
	{
		pBoneMat = &WorldMat;
		if( m_vecPrimitiveParentIndex[ i ] != -1 )
		{
			pBoneMat = GetBoneMat( m_vecPrimitiveParentIndex[ i ] );
			if( pBoneMat )
			{
				pBoneMat = EtMatrixMultiply( &AniMat, pBoneMat, &WorldMat );
			}
			else
			{
				pBoneMat = &WorldMat;
			}			

			CEtCollisionEntity::UpdateCollisionPrimitive( i, *pBoneMat );
		}
		else {
			CEtCollisionEntity::UpdateCollisionPrimitive( i, *pBoneMat, bUpdate );
		}
	}
}

bool CEtObject::FindDynamicCollision( EtMatrix &WorldMat, DNVector(SCollisionResponse) &vecResponse, bool bCalcContactTime, bool bUpdaetPrimitive)
{
	SAABox BoundingBox;
	EtVector3 vMove;

	vMove.x = WorldMat._41 - m_WorldMat._41;
	vMove.y = WorldMat._42 - m_WorldMat._42;
	vMove.z = WorldMat._43 - m_WorldMat._43;
	GetBoundingBox( BoundingBox );
	BoundingBox.Max += vMove;
	BoundingBox.Min += vMove;
	if( bUpdaetPrimitive )
	{
		UpdateCollisionPrimitive( WorldMat );
		m_bUpdateNeeded = true;
	}
	return _FindDynamicCollision( BoundingBox, vMove, vecResponse, bCalcContactTime );
}

bool CEtObject::_FindDynamicCollision( SAABox &BoundingBox, EtVector3 &vMove, DNVector(SCollisionResponse) &vecResponse, bool bCalcContactTime)
{
	static DNVector(CEtObject *) vecPickObject;

	bool bRet = false;
	vecPickObject.clear();
	s_pDynamicOctree->Pick( BoundingBox, vecPickObject );
	if( !vecPickObject.empty() )
	{
		int i;

		for( i = 0; i < ( int )vecPickObject.size(); i++ )
		{
			std::vector< SCollisionPrimitive * > *pTargetPrimitive;

			if( !vecPickObject[ i ]->IsEnableCollision() )
			{
				continue;
			}
			if( ( vecPickObject[ i ]->GetCollisionGroup() & GetTargetCollisionGroup() ) == 0 )
			{
				continue;
			}
			pTargetPrimitive = vecPickObject[ i ]->GetCollisionPrimitive();
			if( !pTargetPrimitive )
			{
				continue;
			}
			if( CEtCollisionEntity::FindCollision( *pTargetPrimitive, vMove, vecResponse, bCalcContactTime ) )
			{
				bRet = true;
			}
		}
	}

	return bRet;
}

bool CEtObject::FindCollision( EtMatrix &WorldMat, DNVector(SCollisionResponse) &vecResponse, bool bCalcContactTime, bool bUpdaetPrimitive)
{
	if( !IsEnableCollision() )
	{
		return false;
	}

	SAABox BoundingBox;
	EtVector3 vMove;
	bool bResult;

	vMove.x = WorldMat._41 - m_WorldMat._41;
	vMove.y = WorldMat._42 - m_WorldMat._42;
	vMove.z = WorldMat._43 - m_WorldMat._43;
	GetBoundingBox( BoundingBox );
	BoundingBox.Max += vMove;
	BoundingBox.Min += vMove;

	if( bUpdaetPrimitive )
	{
		UpdateCollisionPrimitive( WorldMat );
		m_bUpdateNeeded = true;
	}

	bResult = CEtCollisionMng::GetInstance().FindCollision( m_vecPrimitive, BoundingBox, vMove, vecResponse, bCalcContactTime );

	if( _FindDynamicCollision( BoundingBox, vMove, vecResponse, bCalcContactTime ) )
	{
		bResult = true;
	}

	return bResult;
}

bool CEtObject::FindCollision( EtObjectHandle hObject, std::vector< SCollisionResponse > &vecResponse, bool bCalcContactTime )
{
	int i, j;
	SCollisionResponse Response;
	bool bResult = false;
	std::vector< SCollisionPrimitive * > &vecTragetPrimitive = *hObject->GetCollisionPrimitive();

	for( i = 0; i < ( int )m_vecPrimitive.size(); i++ )
	{
		if( !m_vecPrimitiveEnable[ i ] )
		{
			continue;
		}
		for( j = 0; j < ( int )vecTragetPrimitive.size(); j++ )
		{
			if( !hObject->IsEnableCollision( vecTragetPrimitive[ j ] ) )
			{
				continue;
			}
			if( CEtCollisionFinder::GetInstance().FindCollision( *m_vecPrimitive[ i ], *vecTragetPrimitive[ j ], Response, bCalcContactTime ) )
			{
				Response.pCollisionPrimitive = vecTragetPrimitive[ j ];
				vecResponse.push_back( Response );
				if( Response.vExtraNormal.x != FLT_MAX )
				{
					Response.vNormal = Response.vExtraNormal;
					vecResponse.push_back( Response );
				}
				bResult = true;
			}
		}
	}

	return bResult;
}

int CEtObject::AddCustomParam( const char *pParamName )
{
	if( m_pSkinInstance )
	{
		return m_pSkinInstance->AddCustomParam( pParamName );
	}
	return -1;
}

bool CEtObject::SetCustomParam( int nParamIndex, void *pValue, int nSubMeshIndex )
{
	if( m_pSkinInstance )
	{
		return m_pSkinInstance->SetCustomParam( nParamIndex, pValue, nSubMeshIndex );
	}
	return false;
}

void CEtObject::RestoreCustomParam( int nParamIndex, int nSubMeshIndex )
{
	if( m_pSkinInstance )
	{
		m_pSkinInstance->RestoreCustomParam( nParamIndex, nSubMeshIndex );
	}
}

void CEtObject::GetExtent( EtVector3 &Origin, EtVector3 &Extent )
{
	Origin = ( m_BoundingBox.Max + m_BoundingBox.Min ) / 2;
	Extent = ( m_BoundingBox.Max - m_BoundingBox.Min ) / 2;
}

void CEtObject::ClearFrustumMask()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		GetItem( i )->SetFrustumMask( 0 );
	}
}

void CEtObject::MaskFrustumObjectList( CEtConvexVolume *pFrustum, int nFrustumMask )
{
	int i, j, nCount;

	int nWaterCount = GetEtWater()->GetWaterCount();
	std::vector<CEtConvexVolume> ReflectFrustumList;
	ReflectFrustumList.resize( nWaterCount );
	
	for( i = 0; i < nWaterCount; i++) {
		ReflectFrustumList[i].Initialize( GetEtWater()->GetViewProjMat( i ) );					
	}
	
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtVector3 Origin, Extent;
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		CEtObject *pObject = hHandle.GetPointer();
		if( !pObject->IsEnable() )
		{
			continue;
		}
		if( pObject->IsChild() )
		{
			continue;
		}
		if( !pObject->IsShow() )
		{
			continue;
		}
		pObject->GetExtent( Origin, Extent );

		if( ( !pObject->IsEnableCull() ) || pFrustum->TesToBox( Origin, Extent ) )
		{				
			pObject->AddFrustumMask( nFrustumMask );
		}
		if( pObject->IsWaterCast() )
		{
			for( j = 0; j < nWaterCount; j++)
			{		
				if( GetEtWater()->GetWaterIgnoreBake( j ) )
					continue;

				if( ( !pObject->IsEnableCull() ) || ( ReflectFrustumList[j].TesToBox( Origin, Extent ) ) )
				{
					pObject->RenderWater( j );
				}		
			}
		}
	}
}

void CEtObject::InitRenderObjectList()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		if( hHandle->IsChild() )
		{
			continue;
		}
		if( hHandle->GetFrustumMask() )
		{
			hHandle->InitRender();
		}
	}
}

void CEtObject::RenderObjectList( int nMask )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		if( hHandle->GetRenderType() != CT_SCREEN )
		{
			continue;
		}
		if( hHandle->GetFrustumMask() & nMask )
		{
			hHandle->Render();
		}
	}
}

void CEtObject::RenderShadowObjectList( int nMask, float fShadowRange )
{
	int i, nCount;
	CEtConvexVolume Frustum;
	EtMatrix ProjMat;
	EtCameraHandle hCamera;

	hCamera = CEtCamera::GetActiveCamera();
	EtMatrixPerspectiveFovLH( &ProjMat, hCamera->GetFOV(), hCamera->GetAspectRatio(), hCamera->GetCameraNear(), fShadowRange );
	EtMatrixMultiply( &ProjMat, hCamera->GetViewMat(), &ProjMat );
	Frustum.Initialize( ProjMat );

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle = GetItem( i );		
		if( !hHandle )
		{
			continue;
		}
		if( hHandle->GetRenderType() != CT_SCREEN )
		{
			continue;
		}
		if( ( hHandle->GetFrustumMask() & nMask ) && ( hHandle->IsShadowCast() ) && ( hHandle->GetObjectAlpha() > 0.5f ) )
		{
			EtVector3 Origin, Extent;

			hHandle->GetExtent( Origin, Extent );
			if( ( !hHandle->IsEnableCull() ) || Frustum.TesToBox( Origin, Extent ) )
			{
				hHandle->RenderShadow();
			}
		}
	}
}

void CEtObject::DrawCollisionList( int nMask )
{
	if( m_bSkipDrawCollision ) {	// If nothing is drawn, avoid looping unnecessarily.
		return;
	}

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;
	nCount = GetItemCount();
	bool bDraw = false;
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		if( hHandle->GetRenderType() != CT_SCREEN )
		{
			continue;
		}
		if( hHandle->IsShowCollisionPrimitive() ) bDraw = true;
		if( hHandle->GetFrustumMask() & nMask )
		{
			hHandle->DrawCollisionPrimitive();
		}
	}

	if( !bDraw ) {
		m_bSkipDrawCollision = true;
	}
}

void CEtObject::CalcLightInfluence()
{
	if( !m_bUpdateNeeded ) {
		return;
	}

	m_vecInfluenceLight.clear();
	CEtLight::CalcLightInfluence( m_BoundingSphere, m_vecInfluenceLight, m_MySmartPtr );

	struct CompareFunc {
		EtVector3 m_vOffset;
		CompareFunc( EtVector3 &vOffset ) : m_vOffset(vOffset) {}
		bool operator () ( CEtLight *pLight1, CEtLight *pLight2 ) const
		{
			float fLength1 = EtVec3LengthSq( &EtVector3( pLight1->GetLightInfo()->Position - m_vOffset ) );
			float fLength2 = EtVec3LengthSq( &EtVector3( pLight2->GetLightInfo()->Position - m_vOffset ) );
			return fLength1 < fLength2;
		}
	};

	std::sort( m_vecInfluenceLight.begin(), m_vecInfluenceLight.end(), CompareFunc( m_BoundingSphere.Center ) );

	int nMaxLightCount = MAX_POINT_LIGHT_COUNT + MAX_SPOT_LIGHT_COUNT - 1;
	if( ( int )m_vecInfluenceLight.size() > nMaxLightCount )
	{
		m_vecInfluenceLight.erase( m_vecInfluenceLight.begin() + nMaxLightCount, m_vecInfluenceLight.end() );
	}
}

void CEtObject::CalcLightInfluenceList( bool bForce )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		if( bForce ) hHandle->SetUpdateNeeded( true );
		hHandle->CalcLightInfluence();
	}
}

void CEtObject::CalcLightMapInfluence()
{
	if( m_pSkinInstance )
	{
		if( m_bLightMapInfluence )
		{
			if( m_bUpdateNeeded ) {
				float fInfluence = CEtTerrainArea::CalcLightMapInfluence( m_WorldMat._41, m_WorldMat._43 );
				m_pSkinInstance->SetDirLightAttenuation( fInfluence );
			}
		}
		else
		{
			m_pSkinInstance->SetDirLightAttenuation( 1.0f );
		}
	}
}

void CEtObject::RecalcLightMapInfluence()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		hHandle->SetUpdateNeeded( true );
		hHandle->CalcLightMapInfluence();
	}
}

void CEtObject::SetWorldSize( EtVector3 &WorldCenter, float fRadius ) 
{ 
	CEtLight::SetWorldSize( WorldCenter, fRadius );
	s_pDynamicOctree->Initialize( WorldCenter, fRadius );

	int i, nItemCount;

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	nItemCount = GetItemCount();
	for( i = 0; i < nItemCount; i++ )
	{
		EtObjectHandle hHandle;
		hHandle = GetItem( i );
		if( !hHandle )
		{
			continue;
		}
		hHandle->m_pCurOctreeNode = NULL;
	}
}

EtObjectHandle CEtObject::Pick( int nX, int nY, int nCameraIndex )
{
	CEtObject *pPickObject;
	EtVector3 Position, Direction;

	CEtCamera::GetItem( nCameraIndex )->CalcPositionAndDir( nX, nY, Position, Direction );
	pPickObject = NULL;
	s_pDynamicOctree->Pick( Position, Direction, pPickObject );
	if( pPickObject )
	{
		return pPickObject->GetMySmartPtr();
	}
	else
	{
		EtObjectHandle Identity;
		return Identity;
	}
}

void CEtObject::OnLoadComplete( CBackgroundLoader *pLoader )
{
	CommonInitialize();
	RecalcBoundingBox();
	RecalcBoundingSphere();
	Update( &m_WorldMat );

	CBackgroundLoaderCallback::OnLoadComplete( pLoader );
}

void CEtObject::AddCollisionMeshToKdTree()
{
	if( ( !m_hSkin ) || ( ( m_nCollisionGroup & COLLISION_TYPE_STATIC ) == 0 ) )
	{
		return;
	}

	int i;
	EtMeshHandle hMesh;

	hMesh = m_hSkin->GetMeshHandle();
	if( hMesh )
	{
		for( i = 0; i < hMesh->GetCollisionPrimitiveCount(); i++ )
		{
			CEtCollisionMng::GetInstance().AddCollisionMeshToKdTree( *( *hMesh->GetCollisionPrimitive() )[ i ], m_WorldMat, m_vScale );
		}
	}
}

void CEtObject::BuildKdTree()
{
	int i, nCount;

	CEtCollisionMng::GetInstance().Clear();

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtObjectHandle hHandle;

		hHandle = GetItem( i );
		if( hHandle )
		{
			if( !hHandle->IsEnableCollision() ) {
				continue;
			}
			hHandle->UpdateCollisionPrimitive( *hHandle->GetWorldMat() );
			hHandle->AddCollisionMeshToKdTree();
		}
	}
	CEtCollisionMng::GetInstance().BuildCollisionTree();
}