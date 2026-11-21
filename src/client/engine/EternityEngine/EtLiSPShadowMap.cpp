#include "StdAfx.h"
#include "EtLiSPShadowMap.h"
#include "EtCollisionFunc.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtLiSPShadowMap::CEtLiSPShadowMap(void)
{
	m_fShadowRange = 2000.0f;
}

CEtLiSPShadowMap::~CEtLiSPShadowMap(void)
{
	Clear();
}

void CEtLiSPShadowMap::Clear()
{
	SAFE_RELEASE_SPTR( m_hShadowMap );
	SAFE_RELEASE_SPTR( m_hShadowMapDepth );
}

void CEtLiSPShadowMap::Initialize( ShadowQuality Quality )
{
	switch( Quality )
	{
	case SQ_HIGH:
		m_nShadowMapSize = 2048;
		break;
	case SQ_NORMAL:
		m_nShadowMapSize = 1024;
		break;
	case SQ_NONE:
		m_nShadowMapSize = 512;
		break;
	default:
		m_nShadowMapSize = 2048;
		break;
	}
	if( ( !m_hShadowMap ) || ( m_hShadowMap->Width() != m_nShadowMapSize ) )
	{
		SAFE_RELEASE_SPTR( m_hShadowMap );
		SAFE_RELEASE_SPTR( m_hShadowMapDepth );
		m_hShadowMap = CEtTexture::CreateRenderTargetTexture( m_nShadowMapSize, m_nShadowMapSize, FMT_R32F );
		m_hShadowMap->SetFileName( "EtLiSPShadowMap" );
		m_hShadowMapDepth = CEtDepth::CreateDepthStencil( m_nShadowMapSize, m_nShadowMapSize );
		m_hShadowMapDepth->SetFileName( "EtLiSPShadowMapDepth" );
	}
}

void CEtLiSPShadowMap::CalcFrustumPoly( std::vector< SPolygon > &vecPolygon )
{
	unsigned i;
	EtMatrix InvViewProjMat;
	SAABox Box( &EtVector3( -1.0f, -1.0f, -1.0f ), &EtVector3( 1.0f, 1.0f, 1.0f ) );

	DNVector(EtVector3) vecVertex;
	Box.GetVertices( vecVertex );
	EtMatrixInverse( &InvViewProjMat, NULL, CEtCamera::GetActiveCamera()->GetViewProjMatForCull() );

	for( i = 0; i < vecVertex.size(); i++ )
	{
		EtVec3TransformCoord( &vecVertex[ i ], &vecVertex[ i ], &InvViewProjMat );
	}

	vecPolygon.resize( 6 );
	for( i = 0; i < 6; i++ )
	{
		vecPolygon[ i ].Polygon.resize( 4 );
	}
	//near poly ccw
	for( i = 0; i < 4; i++ )
	{
		vecPolygon[ 0 ].Polygon[i] = vecVertex[ i ];
	}
	//far poly ccw
	for( i = 4; i < 8; i++ )
	{
		vecPolygon[ 1 ].Polygon[ i - 4 ] = vecVertex[ 11 - i ];
	}
	//left poly ccw
	vecPolygon[ 2 ].Polygon[ 0 ] = vecVertex[ 0 ];
	vecPolygon[ 2 ].Polygon[ 1 ] = vecVertex[ 3 ];
	vecPolygon[ 2 ].Polygon[ 2 ] = vecVertex[ 7 ];
	vecPolygon[ 2 ].Polygon[ 3 ] = vecVertex[ 4 ];
	//right poly ccw
	vecPolygon[ 3 ].Polygon[ 0 ] = vecVertex[ 1 ];
	vecPolygon[ 3 ].Polygon[ 1 ] = vecVertex[ 5 ];
	vecPolygon[ 3 ].Polygon[ 2 ] = vecVertex[ 6 ];
	vecPolygon[ 3 ].Polygon[ 3 ] = vecVertex[ 2 ];
	//bottom poly ccw
	vecPolygon[ 4 ].Polygon[ 0 ] = vecVertex[ 4 ];
	vecPolygon[ 4 ].Polygon[ 1 ] = vecVertex[ 5 ];
	vecPolygon[ 4 ].Polygon[ 2 ] = vecVertex[ 1 ];
	vecPolygon[ 4 ].Polygon[ 3 ] = vecVertex[ 0 ];
	//top poly ccw
	vecPolygon[ 5 ].Polygon[ 0 ] = vecVertex[ 6 ];
	vecPolygon[ 5 ].Polygon[ 1 ] = vecVertex[ 7 ];
	vecPolygon[ 5 ].Polygon[ 2 ] = vecVertex[ 3 ];
	vecPolygon[ 5 ].Polygon[ 3 ] = vecVertex[ 2 ];
}

void CEtLiSPShadowMap::ClipPolygonByPlane( std::vector< EtVector3 > &Polygon, EtVector4 &Plane, 
									  std::vector< EtVector3 > &PolyOut, std::vector< EtVector3 > &PolyIntersect )
{
	if( Polygon.size() < 3 )
	{
		return;
	}

	unsigned i;
	std::vector< bool > vecOutside;

	vecOutside.resize( Polygon.size() );
	for( i = 0; i < Polygon.size(); i++ )
	{
		vecOutside[ i ] = IsBehindPoint( Plane, Polygon[ i ] );
	}
	for( i = 0; i < Polygon.size(); i++ )
	{
		unsigned nNext;
		EtVector3 Intersect;

		nNext = ( i + 1 ) % Polygon.size();
		if( ( vecOutside[ i ] ) && ( vecOutside[ nNext ] ) )
		{
			continue;
		}
		if( vecOutside[ i ] )
		{
			if( TestEdgeToPlane( Polygon[ i ], Polygon[ nNext ], Plane, Intersect ) )
			{
				PolyOut.push_back( Intersect );
				PolyIntersect.push_back( Intersect );
			}
			PolyOut.push_back( Polygon[ nNext ] );
			continue;
		}
		if( vecOutside[ nNext ] )
		{
			if( TestEdgeToPlane( Polygon[ i ], Polygon[ nNext ], Plane, Intersect ) )
			{
				PolyOut.push_back( Intersect );
				PolyIntersect.push_back( Intersect );
			}
			continue;
		}
		PolyOut.push_back( Polygon[ nNext ] );
	}
}

void CEtLiSPShadowMap::ClosePolygon( std::vector< EtVector3 > &PolyOut, std::vector< SPolygon > &IntersectEdges, EtVector4 &Plane )
{
	std::vector< SPolygon >::iterator iter;

	iter = IntersectEdges.begin();
	while( iter != IntersectEdges.end() )
	{
		if( ( *iter ).Polygon.size() != 2 )
		{
			iter = IntersectEdges.erase( iter );
		}
		else
		{
			iter++;
		}
	}
	if( IntersectEdges.size() < 3 )
	{
		return;
	}

	PolyOut.push_back( IntersectEdges.back().Polygon[ 0 ] );
	PolyOut.push_back( IntersectEdges.back().Polygon[ 1 ] );
	IntersectEdges.pop_back();

	while( IntersectEdges.size() > 0 )
	{
		unsigned i, nLastPointIndex, nFindIndex;

		nFindIndex = -1;
		nLastPointIndex = ( unsigned )PolyOut.size() - 1;
		for( i = 0; i < IntersectEdges.size(); i++ )
		{
			if( IsNearPoint( IntersectEdges[ i ].Polygon[ 0 ], PolyOut[ nLastPointIndex ] ) )
			{
				PolyOut.push_back( IntersectEdges[ i ].Polygon[ 1 ] );
				nFindIndex = i;
				break;
			}
			if( IsNearPoint( IntersectEdges[ i ].Polygon[ 1 ], PolyOut[ nLastPointIndex ] ) )
			{
				PolyOut.push_back( IntersectEdges[ i ].Polygon[ 0 ] );
				nFindIndex = i;
				break;
			}
		}
		if( nFindIndex != -1 )
		{
			IntersectEdges.erase( IntersectEdges.begin() + nFindIndex );
		}
		else
		{
			PolyOut.clear();
			return;
		}
	}
	PolyOut.pop_back();

	EtVector3 PolyNormal;

	EtVec3Cross( &PolyNormal, &( PolyOut[ 1 ] - PolyOut[ 0 ] ), &( PolyOut[ 2 ] - PolyOut[ 1 ] ) );
	if( EtVec3Dot( &PolyNormal, ( EtVector3 * )&Plane ) <= 0 )
	{
		std::reverse( PolyOut.begin(), PolyOut.end() );
	}
}

void CEtLiSPShadowMap::ClipFrustumByPlane( std::vector< SPolygon > &vecPolygon, EtVector4 &Plane, std::vector< SPolygon > &vecOutput )
{
	if( vecPolygon.size() <= 0 )
	{
		return;
	}

	unsigned i, nSize;
	std::vector< SPolygon > vecIntersectEdge;
	std::vector< SPolygon > vecInputPoly( vecPolygon );

	vecOutput.clear();
	nSize = 0;
	for( i = 0; i < vecInputPoly.size(); i++ )
	{
		vecOutput.resize( nSize + 1 );
		vecIntersectEdge.resize( nSize + 1 );
		ClipPolygonByPlane( vecInputPoly[ i ].Polygon, Plane, vecOutput[ nSize ].Polygon, vecIntersectEdge[ nSize ].Polygon );
		if( vecOutput[ nSize ].Polygon.size() != 0 )
		{
			nSize++;
		}
	}
	if( vecOutput.back().Polygon.size() == 0 )
	{
		vecOutput.pop_back();
	}
	if( vecIntersectEdge.size() > 0 )
	{ 
		vecOutput.resize( vecOutput.size() + 1 );
		ClosePolygon( vecOutput.back().Polygon, vecIntersectEdge, Plane );
		if( vecOutput.back().Polygon.size() == 0 )
		{
			vecOutput.pop_back();
		}
	}

}

void CEtLiSPShadowMap::ClipFrustumBySceneBox(  std::vector< SPolygon > &vecPolygon )
{
	unsigned i;
	DNVector(EtVector4) vecPlane;
	m_SceneBox.GetPlanes( vecPlane );

	for( i = 0; i < vecPlane.size(); i++ )
	{
		ClipFrustumByPlane( vecPolygon, vecPlane[ i ], vecPolygon );
	}
}

void CEtLiSPShadowMap::CalcNearCameraPoint()
{
	int i;
	float fMinDist;

	fMinDist = FLT_MAX;
	for( i = 0; i < ( int )m_vecFocusRegion.size(); i++ )
	{
		float fDist;

		fDist = EtVec3LengthSq( &( m_vecFocusRegion[ i ] - m_CamPos ) );
		if( fDist < fMinDist )
		{
			fMinDist = fDist;
			m_NearCameraPoint = m_vecFocusRegion[ i ];
		}
	}
}

void CEtLiSPShadowMap::GetSCameraInfo()
{
	EtMatrix InvViewMat, *pViewMat;

	pViewMat = CEtCamera::GetActiveCamera()->GetViewMat();
	EtMatrixInverse( &InvViewMat, NULL, pViewMat );
	memcpy( &m_CamPos, &InvViewMat._41, sizeof( EtVector3 ) );
	m_CamDir.x = pViewMat->_13;
	m_CamDir.y = pViewMat->_23;
	m_CamDir.z = pViewMat->_33;

	m_LightDir = CEtLight::GetShadowCastDirLightInfo()->Direction;

	m_SceneBox.Min = m_CamPos + EtVector3( -m_fShadowRange, -m_fShadowRange / 4, -m_fShadowRange );
	m_SceneBox.Max = m_CamPos + EtVector3( m_fShadowRange, m_fShadowRange / 4, m_fShadowRange );
}

void CEtLiSPShadowMap::CalcFocusRegion()
{
	std::vector< SPolygon > vecPolygon;

	GetSCameraInfo();
	CalcFrustumPoly( vecPolygon );
	ClipFrustumBySceneBox( vecPolygon );

	unsigned i, j;

	m_vecFocusRegion.clear();
	for( i = 0; i < vecPolygon.size(); i++ )
	{
		const std::vector< EtVector3 > &Polygon = vecPolygon[ i ].Polygon;
		for( j = 0; j < Polygon.size(); j++ )
		{
			m_vecFocusRegion.push_back( Polygon[ j ] );
		}
	}

	CalcNearCameraPoint();

	unsigned nSize;
	EtVector3 LightDir;
	float fDist;

	nSize = ( unsigned )m_vecFocusRegion.size();
	LightDir = -m_LightDir;
	for( i = 0; i < nSize; i++ )
	{
		// by mapping   When m_vecFocusRegion[ i ] is inside the box, intersection is not found.. Fix later..
		if( TestLineToBox( m_vecFocusRegion[ i ], LightDir, m_SceneBox, fDist ) )
		{
			if( fDist > 10e-5 )
			{
				m_vecFocusRegion.push_back( m_vecFocusRegion[ i ] + LightDir * fDist );
			}
		}
	}
}

void CEtLiSPShadowMap::CalcLightSpaceViewProjDir( EtVector3 &LightSpaceViewProjDir )
{
	EtMatrix LightSpaceMat;
	EtVector3 NearLS, EdgeLS;

	EtMatrixMultiply( &LightSpaceMat, &m_LightViewMat, &m_LightProjMat );

	EtVec3TransformCoord( &NearLS, &m_NearCameraPoint, &LightSpaceMat );
	EtVec3TransformCoord( &EdgeLS, &( m_NearCameraPoint + m_CamDir ), &LightSpaceMat );
	LightSpaceViewProjDir = EdgeLS - NearLS;
	LightSpaceViewProjDir.y = 0.0f;
}

void CEtLiSPShadowMap::CalcZ0LS( EtVector3 &Output, EtMatrix &LightSpaceMat, float fBoxMaxZ )
{
	EtMatrix InvLightSpaceMat;
	EtVector3 NearLS;
	EtVector4 Plane;
	float fLength;

	EtMatrixInverse( &InvLightSpaceMat, NULL, &LightSpaceMat );
	EtMatrixTranspose( &InvLightSpaceMat, &InvLightSpaceMat );
	Plane.x = m_CamDir.x;
	Plane.y = m_CamDir.y;
	Plane.z = m_CamDir.z;
	Plane.w = -EtVec3Dot( &m_CamDir, &m_NearCameraPoint );
	EtVec4Transform( &Plane, &Plane, &InvLightSpaceMat );
	Plane.w = -Plane.w;
	fLength = EtVec3Length( ( EtVector3 * )&Plane );
	Plane /= fLength;

	EtVec3TransformCoord( &NearLS, &m_NearCameraPoint, &LightSpaceMat );
	Output = EtVector3( NearLS.x, ( Plane.w - Plane.z * fBoxMaxZ - Plane.x * NearLS.x ) / Plane.y, fBoxMaxZ );
}

float CEtLiSPShadowMap::CalcNParam( EtMatrix &LightSpaceMat, SAABox &Box )
{
	EtVector3 Z0, Z1;
	EtMatrix InvLightSpaceMat, ViewMat;

	CalcZ0LS( Z0, LightSpaceMat, Box.Min.z );
	Z1 = EtVector3( Z0.x, Z0.y, Box.Max.z );

	EtMatrixInverse( &InvLightSpaceMat, NULL, &LightSpaceMat );
	EtVec3TransformCoord( &Z0, &Z0, &InvLightSpaceMat );
	EtVec3TransformCoord( &Z1, &Z1, &InvLightSpaceMat );

	ViewMat = *CEtCamera::GetActiveCamera()->GetViewMat();
	EtVec3TransformCoord( &Z0, &Z0, &ViewMat );
	EtVec3TransformCoord( &Z1, &Z1, &ViewMat );

	return fabs( Box.Max.z - Box.Min.z ) / ( sqrtf( Z1.z / Z0.z ) - 1.0f );
}

void CEtLiSPShadowMap::CalcLiSPSMat()
{
	unsigned i;
	float fNParam, fDepth;
	EtMatrix LightSpaceMat, ProjCenterMat, Frustum;
	EtVector3 NearPointLS, Center;
	SAABox Box;

	EtMatrixMultiply( &LightSpaceMat, &m_LightViewMat, &m_LightProjMat );
	Box.Reset();
	for( i = 0; i < m_vecFocusRegion.size(); i++ )
	{
		EtVector3 PointLS;
		EtVec3TransformCoord( &PointLS, &m_vecFocusRegion[ i ], &LightSpaceMat );
		Box.AddPoint( PointLS );
	}
	EtVec3TransformCoord( &NearPointLS, &m_NearCameraPoint, &LightSpaceMat );
	fNParam = CalcNParam( LightSpaceMat, Box );

	EtMatrixIdentity( &ProjCenterMat );
	ProjCenterMat._41 = -NearPointLS.x;
	ProjCenterMat._42 = -NearPointLS.y;	
	ProjCenterMat._43 = ( fNParam - Box.Min.z );

	fDepth = fabs( Box.Max.z - Box.Min.z );
	EtMatrixPerspectiveOffCenterLH( &Frustum, -1.0f, 1.0f, -1.0f, 1.0f, fNParam, fNParam + fDepth );
	Frustum._33 = ( fNParam + fNParam + fDepth ) / fDepth;
	Frustum._43 = 2.0f * Frustum._43;
	EtMatrixMultiply( &Frustum, &ProjCenterMat, &Frustum );
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &Frustum );
}

void CEtLiSPShadowMap::RescaleShadowMat()
{
	unsigned i;
	EtMatrix LightSpaceMat, ScaleMat, TransMat;
	SAABox Box;

	EtMatrixMultiply( &LightSpaceMat, &m_LightViewMat, &m_LightProjMat );
	Box.Reset();
	for( i = 0; i < m_vecFocusRegion.size(); i++ )
	{
		EtVector3 PointLS;
		EtVec3TransformCoord( &PointLS, &m_vecFocusRegion[ i ], &LightSpaceMat );
		Box.AddPoint( PointLS );
	}

	EtVector3 DiffBox, SumBox;

	DiffBox = Box.Max - Box.Min;
	SumBox = Box.Max + Box.Min;
	EtMatrixScaling( &ScaleMat, 2 / DiffBox.x, 2 / DiffBox.y, 2 / DiffBox.z );
	EtMatrixTranslation( &TransMat, -SumBox.x / DiffBox.x, -SumBox.y / DiffBox.y, -SumBox.z / DiffBox.z );
	EtMatrixMultiply( &ScaleMat, &ScaleMat, &TransMat );
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &ScaleMat );
}

void CEtLiSPShadowMap::CalcUpVec( EtVector3 &Output )
{
	unsigned i;
	EtVector3 ViewDir, RightDir;

	ViewDir = EtVector3( 0.0f, 0.0f, 0.0f );
	for( i = 0; i < m_vecFocusRegion.size(); i++ )
	{
		EtVector3 Diff;

		Diff = m_vecFocusRegion[ i ] - m_CamPos;
		ViewDir += Diff;
	}
	EtVec3Normalize( &ViewDir, &ViewDir );
	EtVec3Cross( &RightDir, &ViewDir, &m_LightDir );
	EtVec3Normalize( &RightDir, &RightDir );
	EtVec3Cross( &Output, &m_LightDir, &RightDir );
	EtVec3Normalize( &Output, &Output );
}

void CEtLiSPShadowMap::CalcShadowMat()
{
	CalcFocusRegion();

	EtVector3 At, UpVec;

	At = m_CamPos + m_LightDir * 100.0f;
	CalcUpVec( UpVec );
	EtMatrixLookAtLH( &m_LightViewMat, &m_CamPos, &At, &UpVec );
	EtMatrixIdentity( &m_LightProjMat );

	if( m_vecFocusRegion.size() <= 0 )
	{
		return;
	}

	EtMatrix ChangeCoordMat;

	memset( &ChangeCoordMat, 0, sizeof( EtMatrix ) );
	ChangeCoordMat._11 = 1.0f;
	ChangeCoordMat._23 = 1.0f;
	ChangeCoordMat._32 = -1.0f;
	ChangeCoordMat._44 = 1.0f;
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &ChangeCoordMat );

	EtVector3 ViewProjDirLS;
	EtMatrix ViewProjMatLS, LiSPSMat;

	CalcLightSpaceViewProjDir( ViewProjDirLS );
	EtMatrixLookAtLH( &ViewProjMatLS, &EtVector3( 0.0f, 0.0f, 0.0f ), &( ViewProjDirLS * 100.0f ), &EtVector3( 0.0f, 1.0f, 0.0f ) );
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &ViewProjMatLS );
	CalcLiSPSMat();
	RescaleShadowMat();
	ChangeCoordMat._23 = -1.0f;
	ChangeCoordMat._32 = 1.0f;
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &ChangeCoordMat );

	EtMatrixIdentity( &ChangeCoordMat );
	ChangeCoordMat._33 = 0.5f;
	ChangeCoordMat._43 = 0.5f;
	EtMatrixMultiply( &m_LightProjMat, &m_LightProjMat, &ChangeCoordMat );
}

void CEtLiSPShadowMap::BeginShadow()
{
	GetEtDevice()->SetRenderTarget( m_hShadowMap->GetSurfaceLevel() );
	GetEtDevice()->SetDepthStencilSurface( m_hShadowMapDepth->GetDepthBuffer() );
	GetEtDevice()->ClearBuffer( 0xffffffff, 1.0f, 0 );

	GetEtDevice()->SetCullMode( CULL_CW );
}

void CEtLiSPShadowMap::EndShadow()
{
	GetEtDevice()->RestoreRenderTarget();
	GetEtDevice()->RestoreDepthStencil();
	GetEtDevice()->SetCullMode( CULL_CCW );
}

/*void CreateLiSPSahdowMap( ShadowQuality Quality )
{
	SAFE_DELETE( g_pEtShadowMap ); 
	g_pEtShadowMap = new CEtLiSPShadowMap();
	g_pEtShadowMap->Initialize( Quality );
}*/
