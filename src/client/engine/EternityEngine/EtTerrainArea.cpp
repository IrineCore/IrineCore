#include "StdAfx.h"
#include "EtConvexVolume.h"
#include "EtCollisionFunc.h"
#include "EtLoader.h"
#include "EtTerrainArea.h"
#include "EtLayeredMultiUVTerrain.h"
#include "EtOptionController.h"
#include "EtGrassBlock.h"
#include "EtDrawQuad.h"
#include "EtWater.h"
#include "EtShadowMap.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

using namespace EternityEngine;

DECL_SMART_PTR_STATIC( CEtTerrainArea, 20 )

extern CSyncLock *g_pEtRenderLock;

bool CEtTerrainArea::s_bOptimize = false;
bool CEtTerrainArea::s_bEnableFrustumCull = true;
CEtTerrainArea::CEtTerrainArea(void)
{
	m_BoundingBox.Reset();
	m_pCommonIndexBuffer = NULL;
	m_pCommonLowIndexBuffer = NULL;

	m_pLightMapInfluenceBuffer = NULL;
	m_nLightMapInfluenceStride = 0;

	m_bShow = true;
	m_nBlockCountX = 0;
	m_nBlockCountY = 0;
	m_nBlockSizeX = 0;
	m_nBlockSizeY = 0;
	m_nGrassBlockCountX = 0;
	m_nGrassBlockCountY = 0;
}

CEtTerrainArea::~CEtTerrainArea(void)
{
	Clear();
}

void CEtTerrainArea::Clear()
{
	SAFE_DELETE_PVEC( m_vecTerrain );
	SAFE_DELETE_PVEC( m_vecGrassBlock );
	SAFE_RELEASE( m_pCommonIndexBuffer );
	SAFE_RELEASE( m_pCommonLowIndexBuffer );

	SAFE_DELETEA( m_pLightMapInfluenceBuffer );
}

void CEtTerrainArea::CalcSelectCount( int nBlockIndex, int &nStart, int &nEnd )
{
	if( nBlockIndex == -1 )
	{
		nStart = 0;
		nEnd = ( int )m_vecTerrain.size();
	}
	else
	{
		nStart = nBlockIndex;
		nEnd = nStart + 1;
	}
}

void CEtTerrainArea::DrawGrid( bool bDraw, int nBlockIndex )
{
	int i, nStart, nEnd;

	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		m_vecTerrain[ i ]->DrawGrid( bDraw );
	}
}

void CEtTerrainArea::SetGridColor( DWORD dwColor, int nBlockIndex )
{
	int i, nStart, nEnd;

	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		m_vecTerrain[ i ]->SetGridColor( dwColor );
	}
}

void CEtTerrainArea::SetTextureDistance( int nTexLayer, float fDistance, int nBlockIndex )
{
	int i, nStart, nEnd;

	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		m_vecTerrain[ i ]->SetTextureDistance( nTexLayer, fDistance );
	}
}

void CEtTerrainArea::SetTextureRotation( int nTexLayer, float fRotation, int nBlockIndex )
{
	int i, nStart, nEnd;

	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		m_vecTerrain[ i ]->SetTextureRotation( nTexLayer, fRotation );
	}
}

void CEtTerrainArea::Initialize( STerrainInfo *pInfo )
{
	CEtTerrain *pTerrain;
	int i, j;

	Clear();

	m_TerrainInfo = *pInfo;
	// When optimize is on, alpha layer must be manipulated internally, so create a separate buffer.
	if( s_bOptimize )
	{
		int nSize = ( m_TerrainInfo.nSizeX + 1 ) * ( m_TerrainInfo.nSizeY + 1 );
		DWORD *pAlphaLayer = new DWORD[ nSize ];
		memcpy( pAlphaLayer, m_TerrainInfo.pLayerDensity, nSize * sizeof( DWORD ) );
		m_TerrainInfo.pLayerDensity = pAlphaLayer;
	}

	m_nBlockCountX = pInfo->nSizeX / DEFAULT_TERRAIN_SIZE;
	if( pInfo->nSizeX % DEFAULT_TERRAIN_SIZE )
	{
		m_nBlockCountX++;
	}
	m_nBlockSizeX = DEFAULT_TERRAIN_SIZE;

	m_nBlockCountY = pInfo->nSizeY / DEFAULT_TERRAIN_SIZE;
	if( pInfo->nSizeY % DEFAULT_TERRAIN_SIZE )
	{
		m_nBlockCountY++;
	}
	m_nBlockSizeY = DEFAULT_TERRAIN_SIZE;

	STerrainInfo TerrainInfo;

	memcpy( &TerrainInfo, &m_TerrainInfo, sizeof( STerrainInfo ) );
	TerrainInfo.nSizeX = m_nBlockSizeX;
	TerrainInfo.nSizeY = m_nBlockSizeY;
	for( i = 0; i < m_nBlockCountY; i++ )
	{
		for( j = 0; j < m_nBlockCountX; j++ )
		{
			TerrainInfo.pHeight = pInfo->pHeight + ( i * m_nBlockSizeY ) * ( pInfo->nSizeX + 1 ) + j * m_nBlockSizeX;
			TerrainInfo.pLayerDensity = pInfo->pLayerDensity + ( i * m_nBlockSizeY ) * ( pInfo->nSizeX + 1 ) + j * m_nBlockSizeX;

			TerrainInfo.TerrainOffset = pInfo->TerrainOffset 
				+ EtVector3( pInfo->fTileSize * j * m_nBlockSizeX, 0.0f, pInfo->fTileSize * i * m_nBlockSizeY );
			pTerrain = CreateBlock( pInfo->Type );
			pTerrain->SetBlockOffset( j * m_nBlockSizeX, i * m_nBlockSizeY );
			pTerrain->SetStride( pInfo->nSizeX + 1, pInfo->nSizeY + 1 );
			pTerrain->SetTerrainInfo( &TerrainInfo );
			m_vecTerrain.push_back( pTerrain );
		}
	}

	float fWorldSizeX, fWorldSizeY, fWorldSizeZ;
	fWorldSizeX = m_TerrainInfo.nSizeX * m_TerrainInfo.fTileSize;
	fWorldSizeY = m_TerrainInfo.nSizeY * m_TerrainInfo.fTileSize;
	fWorldSizeZ = pInfo->fHeightMultiply * 65535.f;
	CEtObject::SetWorldSize( EtVector3( fWorldSizeX * 0.5f, 0.0f, fWorldSizeY * 0.5f ), max( max( fWorldSizeX, fWorldSizeY ), fWorldSizeZ ) );

	CalcBoundingBox();

	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	if( Option.bDrawGrass )
	{
		CreateGrassBlock();
	}

	if( s_bOptimize )
	{
		delete [] m_TerrainInfo.pLayerDensity;
	}
}

void CEtTerrainArea::CreateCommonIndexBuffer()
{
	// Create only the needed index buffer depending on whether low splatting is used for low-spec option or not.
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	
	// #40097 Use legacy method for high-spec option, or low-spec with GPUs that do not support 2-layer splatting shaders
	// Terrain rendering. m_pCommonIndexBuffer is also needed for this.
	if( true == Option.bUseSplatting ||
		(false == Option.bUseSplatting && true == Option.bIsOnlyLowShaderAvailable) )
	{
		if( m_pCommonIndexBuffer == NULL )
		{
			int i, j, nIndexCount;
			WORD *pIndexBuffer;
			int nFaceCount;

			nFaceCount = m_nBlockSizeX * m_nBlockSizeY * 2;
			nIndexCount = 0;
			m_pCommonIndexBuffer = GetEtDevice()->CreateIndexBuffer( nFaceCount * 3 * sizeof( WORD ) );
			m_pCommonIndexBuffer->Lock( 0, sizeof( WORD ) * nFaceCount * 3, ( void ** )&pIndexBuffer, 0 );
			for( i = 0; i < m_nBlockSizeY; i++ )
			{
				for( j = 0; j < m_nBlockSizeX; j++ )
				{
					int nOffset;

					nOffset = ( m_nBlockSizeX + 1 ) * i + j;
					pIndexBuffer[ nIndexCount ] = nOffset;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX + 1;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX + 2;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX + 2;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + 1;
					nIndexCount++;
				}
			}
			m_pCommonIndexBuffer->Unlock();
		}
	}
	
	if( false == Option.bUseSplatting )
	{
		if( m_pCommonLowIndexBuffer == NULL )
		{
			int i, j, nIndexCount;
			WORD *pIndexBuffer;
			int nFaceCount;

			nFaceCount = (m_nBlockSizeX / 2) * (m_nBlockSizeY / 2) * 2;
			nIndexCount = 0;
			m_pCommonLowIndexBuffer = GetEtDevice()->CreateIndexBuffer( nFaceCount * 3 * sizeof( WORD ) );
			m_pCommonLowIndexBuffer->Lock( 0, sizeof( WORD ) * nFaceCount * 3, ( void ** )&pIndexBuffer, 0 );
			for( i = 0; i < m_nBlockSizeY / 2; i++ )
			{
				for( j = 0; j < m_nBlockSizeX / 2; j++ )
				{
					int nOffset;

					nOffset = ( m_nBlockSizeX / 2 + 1 ) * i + j;
					pIndexBuffer[ nIndexCount ] = nOffset;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX / 2 + 1;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX / 2 + 2;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + m_nBlockSizeX / 2 + 2;
					nIndexCount++;
					pIndexBuffer[ nIndexCount ] = nOffset + 1;
					nIndexCount++;
				}
			}
			m_pCommonLowIndexBuffer->Unlock();
		}
	}
}

void CEtTerrainArea::InitializeBlock( int nBlockIndex )
{
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );

	int i, nStart, nEnd;

	CreateCommonIndexBuffer();
	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		if( true == Option.bUseSplatting ||
			(false == Option.bUseSplatting && true == Option.bIsOnlyLowShaderAvailable) )
		{
			m_vecTerrain[ i ]->SetCommonIndexBuffer( m_pCommonIndexBuffer, m_nBlockSizeX * m_nBlockSizeY * 2 );
		}
		else
		{
			m_vecTerrain[ i ]->SetCommonIndexBuffer( m_pCommonLowIndexBuffer, (m_nBlockSizeX/2) * (m_nBlockSizeY/2) * 2 );
		}
		m_vecTerrain[ i ]->Initialize();
		m_vecTerrain[ i ]->Enable( true );		

		if( !m_vecGrassBlock.empty() )
		{
			int nGrassBlockIndex;

			nGrassBlockIndex = ( i % m_nBlockCountX ) * 2 + ( i / m_nBlockCountX ) * m_nBlockCountX * 4;
			if( m_vecGrassBlock[ nGrassBlockIndex ] )
			{
				m_vecGrassBlock[ nGrassBlockIndex ]->Initialize( this );
			}
			nGrassBlockIndex++;
			if( m_vecGrassBlock[ nGrassBlockIndex ] )
			{
				m_vecGrassBlock[ nGrassBlockIndex ]->Initialize( this );
			}
			nGrassBlockIndex += m_nBlockCountX * 2;
			if( m_vecGrassBlock[ nGrassBlockIndex ] )
			{
				m_vecGrassBlock[ nGrassBlockIndex ]->Initialize( this );
			}
			nGrassBlockIndex--;
			if( m_vecGrassBlock[ nGrassBlockIndex ] )
			{
				m_vecGrassBlock[ nGrassBlockIndex ]->Initialize( this );
			}
		}
	}
	CalcBoundingBox();
}

CEtTerrain *CEtTerrainArea::CreateBlock( TerrainType Type )
{
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	if( !Option.bUseSplatting )
	{
		if( Option.bIsOnlyLowShaderAvailable )
		{
			return new CEtLowDetailTerrain();
		}
		else
		{
			return new CEtLowSplatTerrain();
		}
	}
	if( s_bOptimize )
	{
		return new CEtTerrainOpti();
	}
	else
	{
		switch( Type )
		{
			case TT_NORMAL:
				return new CEtTerrain();
			case TT_DETAILNORMAL:
				return new CEtDetailTerrain();
			case TT_CLIFF:
				return new CEtCliffTerrain();
			case TT_DETAILCLIFF:
				return new CEtDetailCliffTerrain();
		}
	}

	return NULL;
}

void CEtTerrainArea::ChangeBlockType( TerrainType Type, int nBlockIndex )
{
	// When optimize is on, there is only one terrain type, so no need to change block type.
	if( s_bOptimize )
	{
		return;
	}

	bool bEnable;
	int i, j, nStart, nEnd, nTexCount;
	STerrainInfo TerrainInfo;
	std::vector< std::string > vecTexName;
	std::vector< float > vecTexDist;
	std::vector< float > vecTexRotation;

	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		bEnable = m_vecTerrain[ i ]->IsEnable();
		memcpy( &TerrainInfo, m_vecTerrain[ i ]->GetTerrainInfo(), sizeof( STerrainInfo ) );
		TerrainInfo.Type = Type;
		vecTexName.clear();
		vecTexDist.clear();
		const char *szTextureName;
		for( j = 0; j < m_vecTerrain[ i ]->GetTextureCount(); j++ )
		{
			szTextureName = m_vecTerrain[ i ]->GetTextureName( j );
			if( szTextureName )
				vecTexName.push_back( szTextureName );
			else 
				vecTexName.push_back( "" );
			vecTexDist.push_back( m_vecTerrain[ i ]->GetTextureDistance( j ) );
			vecTexRotation.push_back( m_vecTerrain[ i ]->GetTextureRotation( j ) );
		}
		delete m_vecTerrain[ i ];
		m_vecTerrain[ i ] = CreateBlock( Type );
		m_vecTerrain[ i ]->SetStride( m_TerrainInfo.nSizeX + 1, m_TerrainInfo.nSizeY + 1 );
		m_vecTerrain[ i ]->SetBlockOffset( i % m_nBlockCountX * m_nBlockSizeX, i / m_nBlockCountX * m_nBlockSizeY );
		m_vecTerrain[ i ]->SetTerrainInfo( &TerrainInfo );
		m_vecTerrain[ i ]->SetCommonIndexBuffer( m_pCommonIndexBuffer, m_nBlockSizeX * m_nBlockSizeY * 2 );
		if( bEnable )
		{
			m_vecTerrain[ i ]->Initialize();
			m_vecTerrain[ i ]->Enable( true );
		}
		nTexCount = min( m_vecTerrain[ i ]->GetTextureCount(), ( int )vecTexName.size() );
		for( j = 0; j < nTexCount; j++ )
		{
			if( vecTexName[ j ].size() )
			{
				m_vecTerrain[ i ]->SetTexture( j, vecTexName[ j ].c_str() );
			}
			m_vecTerrain[ i ]->SetTextureDistance( j, vecTexDist[ j ] );
			m_vecTerrain[ i ]->SetTextureRotation( j, vecTexRotation[ j ] );
		}
	}
}

TerrainType CEtTerrainArea::GetBlockType( int nBlockIndex )
{
	return m_vecTerrain[nBlockIndex]->GetTerrainInfo()->Type;
}

void CEtTerrainArea::Render( CEtConvexVolume *pFrustum )
{
	int i, nSize;
	EtVector3 Origin, Extent;
	static std::vector< bool > vecFrustumCull;

	nSize = (int)m_vecTerrain.size();
	vecFrustumCull.clear();
	vecFrustumCull.resize( nSize );
	for( i = 0; i < nSize; i++ )
	{
		if( m_vecTerrain[ i ]->IsEnable() == false )
		{
			vecFrustumCull[ i ] = false;
			continue;
		}
		if( s_bEnableFrustumCull ) {
			m_vecTerrain[ i ]->GetExtent( Origin, Extent );
			if( pFrustum->TesToBox( Origin, Extent ) == false )
			{
				vecFrustumCull[ i ] = false;
				continue;
			}
		}
		vecFrustumCull[ i ] = true;
	}

	nSize = ( int )m_vecGrassBlock.size();
	for( i = 0; i < nSize; i++ )
	{
		if( ( m_vecGrassBlock[ i ] ) && ( m_vecGrassBlock[ i ]->GetGrassCount() > 0 ) )
		{
			int nTerrainIndex;

			nTerrainIndex = i  / ( m_nGrassBlockCountX * 2 ) * m_nBlockCountX + i % ( m_nGrassBlockCountX ) / 2;
			if( !vecFrustumCull[ nTerrainIndex ] )
			{
				continue;
			}
			m_vecGrassBlock[ i ]->GetExtent( Origin, Extent );
			if( pFrustum->TesToBox( Origin, Extent ) == true || !s_bEnableFrustumCull )
			{
				m_vecGrassBlock[ i ]->Render();
			}
		}
	}

	nSize = ( int )m_vecTerrain.size();
	for( i = 0; i < nSize; i++ )
	{
		if( !vecFrustumCull[ i ] )
		{
			continue;
		}
		m_vecTerrain[ i ]->Render();
	}
}

void CEtTerrainArea::RenderWater( int index, CEtConvexVolume *pFrustum )
{
	int i;
	EtVector3 Origin, Extent;
	static std::vector< bool > vecFrustumCull;

	int nSize = (int)m_vecTerrain.size();
	vecFrustumCull.clear();
	vecFrustumCull.resize( nSize );
	for( i = 0; i < nSize; i++ )
	{
		if( m_vecTerrain[ i ]->IsEnable() == false )
		{
			vecFrustumCull[ i ] = false;
			continue;
		}
		if( s_bEnableFrustumCull ) {
			m_vecTerrain[ i ]->GetExtent( Origin, Extent );
			if( pFrustum->TesToBox( Origin, Extent ) == false )
			{
				vecFrustumCull[ i ] = false;
				continue;
			}
		}
		vecFrustumCull[ i ] = true;
	}

	for( i = 0; i < nSize; i++ )
	{
		if( !vecFrustumCull[ i ] )
		{
			continue;
		}
		m_vecTerrain[ i ]->RenderWater( index );
	}
}

void CEtTerrainArea::GetExtent( EtVector3 &Origin, EtVector3 &Extent )
{
	Origin = ( m_BoundingBox.Max + m_BoundingBox.Min ) / 2;
	Extent = ( m_BoundingBox.Max - m_BoundingBox.Min ) / 2;
}

void CEtTerrainArea::CalcBoundingBox()
{
	int i;
	SAABox *pBoundingBox;

	for( i = 0; i < ( int )m_vecTerrain.size(); i++ )
	{
		pBoundingBox = m_vecTerrain[ i ]->GetBoundingBox();
		if( m_BoundingBox.Min.y > pBoundingBox->Min.y )
		{
			m_BoundingBox.Min.y = pBoundingBox->Min.y;
		}
		if( m_BoundingBox.Max.y < pBoundingBox->Max.y )
		{
			m_BoundingBox.Max.y = pBoundingBox->Max.y;
		}
	}
	m_BoundingBox.Max.x = m_TerrainInfo.TerrainOffset.x + m_TerrainInfo.nSizeX * m_TerrainInfo.fTileSize;
	m_BoundingBox.Max.z = m_TerrainInfo.TerrainOffset.z + m_TerrainInfo.nSizeY * m_TerrainInfo.fTileSize;

	m_BoundingBox.Min.x = m_TerrainInfo.TerrainOffset.x;
	m_BoundingBox.Min.z = m_TerrainInfo.TerrainOffset.z;
}

void CEtTerrainArea::RenderTerrainBlockList( CEtConvexVolume *pFrustum)
{
	int i, nCount, nWaterCount;
	EtVector3 Origin, Extent;
	EtTerrainHandle hHandle;

	nWaterCount = GetEtWater()->GetWaterCount();

	// Applying Water culling distance to terrain caused terrain at an intermediate distance to flicker in water..
	float fWaterRatio = CEtCamera::GetActiveCamera()->GetWaterFarRatio();
	CEtCamera::GetActiveCamera()->SetWaterFarRatio( 1.0f );
	std::vector< CEtConvexVolume > vecWaterFrustum;
	vecWaterFrustum.resize( nWaterCount );
	for( int j = 0; j < nWaterCount; j++) 
	{
		vecWaterFrustum[j].Initialize( GetEtWater()->GetViewProjMat( j ) );
	}
	CEtCamera::GetActiveCamera()->SetWaterFarRatio( fWaterRatio );

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		hHandle = GetItem( i );
		if( ( hHandle ) && ( hHandle->IsShow() ) )
		{
			hHandle->GetExtent( Origin, Extent );
			if( pFrustum->TesToBox( Origin, Extent ) || !s_bEnableFrustumCull )
			{
				hHandle->Render( pFrustum );
			}
			if( GetEtOptionController()->GetWaterQuality() != WQ_LOW )
			{
				for( int j = 0; j < nWaterCount; j++) 
				{
					if( GetEtWater()->GetWaterIgnoreBake( j ) )
						continue;

					if( vecWaterFrustum[j].TesToBox( Origin, Extent) || !s_bEnableFrustumCull ) 
					{
						hHandle->RenderWater( j, &vecWaterFrustum[j]);
					}
				}
			}
		}
	}
}

void CEtTerrainArea::UpdateHeight( int nStartX, int nStartY, int nEndX, int nEndY )
{
	int i, j;
	int nStartBlockX, nStartBlockY, nEndBlockX, nEndBlockY;

	nStartBlockX = nStartX / m_nBlockSizeX;
	nStartBlockY = nStartY / m_nBlockSizeY;
	nEndBlockX = nEndX / m_nBlockSizeX;
	nEndBlockY = nEndY / m_nBlockSizeY;

	if( nStartBlockX < 0 ) nStartBlockX = 0;
	else if( nStartBlockX >= m_nBlockCountX ) nStartBlockX = m_nBlockCountX - 1;
	if( nStartBlockY < 0 ) nStartBlockY = 0;
	else if( nStartBlockY >= m_nBlockCountY ) nStartBlockY = m_nBlockCountY - 1;

	if( nEndBlockX < 0 ) nEndBlockX = 0;
	else if( nEndBlockX >= m_nBlockCountX ) nEndBlockX = m_nBlockCountX - 1;
	if( nEndBlockY < 0 ) nStartBlockY = 0;
	else if( nEndBlockY >= m_nBlockCountY ) nEndBlockY = m_nBlockCountY - 1;


	for( i = nStartBlockY; i <= nEndBlockY; i++ )
	{
		for( j = nStartBlockX; j <= nEndBlockX; j++ )
		{
			if( m_vecTerrain[ i * m_nBlockCountX + j ]->IsEnable() == false )
			{
				continue;
			}
			m_vecTerrain[ i * m_nBlockCountX + j ]->UpdateHeight();
		}
	}
	CalcBoundingBox();
}

void CEtTerrainArea::UpdateLayer( int nStartX, int nStartY, int nEndX, int nEndY )
{
	int i, j;
	int nStartBlockX, nStartBlockY, nEndBlockX, nEndBlockY;

	nStartBlockX = nStartX / m_nBlockSizeX;
	nStartBlockY = nStartY / m_nBlockSizeY;
	nEndBlockX = nEndX / m_nBlockSizeX;
	nEndBlockY = nEndY / m_nBlockSizeY;

	if( nStartBlockX < 0 ) nStartBlockX = 0;
	else if( nStartBlockX >= m_nBlockCountX ) nStartBlockX = m_nBlockCountX - 1;
	if( nStartBlockY < 0 ) nStartBlockY = 0;
	else if( nStartBlockY >= m_nBlockCountY ) nStartBlockY = m_nBlockCountY - 1;

	if( nEndBlockX < 0 ) nEndBlockX = 0;
	else if( nEndBlockX >= m_nBlockCountX ) nEndBlockX = m_nBlockCountX - 1;
	if( nEndBlockY < 0 ) nStartBlockY = 0;
	else if( nEndBlockY >= m_nBlockCountY ) nEndBlockY = m_nBlockCountY - 1;

	for( i = nStartBlockY; i <= nEndBlockY; i++ )
	{
		for( j = nStartBlockX; j <= nEndBlockX; j++ )
		{
			if( m_vecTerrain[ i * m_nBlockCountX + j ]->IsEnable() == false )
			{
				continue;
			}
			m_vecTerrain[ i * m_nBlockCountX + j ]->UpdateLayer();
		}
	}
}

void CEtTerrainArea::UpdateNormal( int nStartX, int nStartY, int nEndX, int nEndY )
{
	int i, j;
	int nStartBlockX, nStartBlockY, nEndBlockX, nEndBlockY;

	nStartBlockX = nStartX / m_nBlockSizeX;
	nStartBlockY = nStartY / m_nBlockSizeY;
	nEndBlockX = nEndX / m_nBlockSizeX;
	nEndBlockY = nEndY / m_nBlockSizeY;

	if( nStartBlockX < 0 ) nStartBlockX = 0;
	else if( nStartBlockX >= m_nBlockCountX ) nStartBlockX = m_nBlockCountX - 1;
	if( nStartBlockY < 0 ) nStartBlockY = 0;
	else if( nStartBlockY >= m_nBlockCountY ) nStartBlockY = m_nBlockCountY - 1;

	if( nEndBlockX < 0 ) nEndBlockX = 0;
	else if( nEndBlockX >= m_nBlockCountX ) nEndBlockX = m_nBlockCountX - 1;
	if( nEndBlockY < 0 ) nStartBlockY = 0;
	else if( nEndBlockY >= m_nBlockCountY ) nEndBlockY = m_nBlockCountY - 1;


	for( i = nStartBlockY; i <= nEndBlockY; i++ )
	{
		for( j = nStartBlockX; j <= nEndBlockX; j++ )
		{
			if( m_vecTerrain[ i * m_nBlockCountX + j ]->IsEnable() == false )
			{
				continue;
			}
			m_vecTerrain[ i * m_nBlockCountX + j ]->UpdateNormal();
		}
	}
}

void CEtTerrainArea::UpdateTextureCoord( int nStartX, int nStartY, int nEndX, int nEndY )
{
	int i, j;
	int nStartBlockX, nStartBlockY, nEndBlockX, nEndBlockY;

	nStartBlockX = nStartX / m_nBlockSizeX;
	nStartBlockY = nStartY / m_nBlockSizeY;
	nEndBlockX = nEndX / m_nBlockSizeX;
	nEndBlockY = nEndY / m_nBlockSizeY;

	if( nStartBlockX < 0 ) nStartBlockX = 0;
	else if( nStartBlockX >= m_nBlockCountX ) nStartBlockX = m_nBlockCountX - 1;
	if( nStartBlockY < 0 ) nStartBlockY = 0;
	else if( nStartBlockY >= m_nBlockCountY ) nStartBlockY = m_nBlockCountY - 1;

	if( nEndBlockX < 0 ) nEndBlockX = 0;
	else if( nEndBlockX >= m_nBlockCountX ) nEndBlockX = m_nBlockCountX - 1;
	if( nEndBlockY < 0 ) nStartBlockY = 0;
	else if( nEndBlockY >= m_nBlockCountY ) nEndBlockY = m_nBlockCountY - 1;


	for( i = nStartBlockY; i <= nEndBlockY; i++ )
	{
		for( j = nStartBlockX; j <= nEndBlockX; j++ )
		{
			if( m_vecTerrain[ i * m_nBlockCountX + j ]->IsEnable() == false )
			{
				continue;
			}
			m_vecTerrain[ i * m_nBlockCountX + j ]->GenerateTexureCoord();
		}
	}
}

void CEtTerrainArea::UpdateGrassBlock( int nStartX, int nStartY, int nEndX, int nEndY )
{
	int i, j;
	int nStartBlockX, nStartBlockY, nEndBlockX, nEndBlockY;

	nStartBlockX = nStartX / ( m_nBlockSizeX / 2 );
	nStartBlockY = nStartY / ( m_nBlockSizeY / 2 );
	nEndBlockX = nEndX / ( m_nBlockSizeX / 2 );
	nEndBlockY = nEndY / ( m_nBlockSizeY / 2 );

	if( nStartBlockX < 0 ) nStartBlockX = 0;
	else if( nStartBlockX >= m_nGrassBlockCountX ) nStartBlockX = m_nGrassBlockCountX - 1;
	if( nStartBlockY < 0 ) nStartBlockY = 0;
	else if( nStartBlockY >= m_nGrassBlockCountY ) nStartBlockY = m_nGrassBlockCountY - 1;

	if( nEndBlockX < 0 ) nEndBlockX = 0;
	else if( nEndBlockX >= m_nGrassBlockCountX ) nEndBlockX = m_nGrassBlockCountX - 1;
	if( nEndBlockY < 0 ) nStartBlockY = 0;
	else if( nEndBlockY >= m_nGrassBlockCountY ) nEndBlockY = m_nGrassBlockCountY - 1;


	for( i = nStartBlockY; i <= nEndBlockY; i++ )
	{
		for( j = nStartBlockX; j <= nEndBlockX; j++ )
		{
			if( !m_vecGrassBlock[ i * m_nGrassBlockCountX + j ] )
			{
				continue;
			}
			m_vecGrassBlock[ i * m_nGrassBlockCountX + j ]->Initialize( this );
		}
	}
}

int CEtTerrainArea::GetGrassBlockCount()
{
	return (int)m_vecGrassBlock.size();
}

void CEtTerrainArea::SetGrassInfo( int nBlockIndex, SGrassBlockInfo &Info )
{
	if( nBlockIndex == -1 ) 
	{
		for( DWORD i=0; i<m_vecGrassBlock.size(); i++ ) 
		{
			m_vecGrassBlock[i]->SetGrassBlockInfo( &Info );
			m_vecGrassBlock[i]->Initialize( this );
		}
	}
	else 
	{
		m_vecGrassBlock[nBlockIndex]->SetGrassBlockInfo( &Info );
		m_vecGrassBlock[nBlockIndex]->Initialize( this );
	}
}

SGrassBlockInfo *CEtTerrainArea::GetGrassInfo( int nBlockIndex )
{
	if( nBlockIndex < 0 || nBlockIndex >= (int)m_vecGrassBlock.size() ) return NULL;
	return m_vecGrassBlock[nBlockIndex]->GetGrassBlockInfo();
}

bool CEtTerrainArea::Pick( EtVector3 &Origin, EtVector3 &Direction, EtVector3 &PickPos )
{
	int i;
	float fMinDist, fFindDist, fDistToBox;
	EtVector3 FindPos, ModifyOrigin;

	fMinDist = FLT_MAX;
	for( i = 0; i < ( int )m_vecTerrain.size(); i++ )
	{
		SAABox *pBoundingBox;

		pBoundingBox = m_vecTerrain[ i ]->GetBoundingBox();
		if( !TestLineToBox( Origin, Direction, *pBoundingBox, fDistToBox ) )
		{
			continue;
		}
		if( fDistToBox == 0.0f )
		{
			ModifyOrigin = Origin;
		}
		else
		{
			ModifyOrigin = Origin + Direction * fDistToBox;
		}
		fFindDist = m_vecTerrain[ i ]->Pick( ModifyOrigin, Direction, FindPos ) + fDistToBox;
		if(fFindDist < fMinDist )
		{
			fMinDist = fFindDist;
			PickPos = FindPos;
		}
	}

	if( fMinDist != FLT_MAX )
	{
		return true;
	}

	return false;
}

bool CEtTerrainArea::Pick( int nX, int nY, EtVector3 &PickPos, int nCameraIndex )
{
	EtVector3 Direction, Origin;

	CEtCamera::GetItem( nCameraIndex )->CalcPositionAndDir( nX, nY, Origin, Direction );
	return Pick( Origin, Direction, PickPos );
}

bool CEtTerrainArea::SetTexture( int nTexIndex, const char *pTexName, int nBlockIndex )
{
	int i, nStart, nEnd;
	bool bRet;

	bRet = true;
	CalcSelectCount( nBlockIndex, nStart, nEnd );
	for( i = nStart; i < nEnd; i++ )
	{
		if( m_vecTerrain[ i ]->SetTexture( nTexIndex, pTexName ) == false )
		{
			bRet = false;
		}
	}

	return bRet;
}

void CEtTerrainArea::SetGrassTexture( const char *pFileName )
{
	int i;

	for( i = 0; i < ( int )m_vecGrassBlock.size(); i++ )
	{
		if( m_vecGrassBlock[ i ] )
		{
			m_vecGrassBlock[ i ]->SetTexture( pFileName );
		}
	}
}

const char *CEtTerrainArea::GetGrassTexture()
{
	if( m_vecGrassBlock.empty() ) return NULL;
	return m_vecGrassBlock[0]->GetTextureName();
}

bool CEtTerrainArea::IsInside( float fX, float fZ )
{
	if( fX < m_TerrainInfo.TerrainOffset.x )
	{
		return false;
	}
	if( fX > m_TerrainInfo.TerrainOffset.x + m_TerrainInfo.nSizeX * m_TerrainInfo.fTileSize )
	{
		return false;
	}
	if( fZ < m_TerrainInfo.TerrainOffset.z )
	{
		return false;
	}
	if( fZ > m_TerrainInfo.TerrainOffset.z + m_TerrainInfo.nSizeY * m_TerrainInfo.fTileSize )
	{
		return false;
	}

	return true;
}

void CEtTerrainArea::BakeLightMap( int nBlockIndex, int nWidth, int nHeight, float fBlurSize, int nBakeRange, float fSlopeBias )
{
	SBakeLightMapParam Param;
	Param.nBlockIndex = nBlockIndex;
	Param.nWidth = nWidth;
	Param.nHeight = nHeight;
	Param.fBlurSize = fBlurSize;
	Param.nBakeRange = nBakeRange;
	Param.fSlopeBias = fSlopeBias;
	m_vecBakeLightMapParam.push_back( Param );
}

void CEtTerrainArea::_BakeLightMap( int nBlockIndex, int nWidth, int nHeight, float fBlurSize, int nBakeRange, float fSlopeBias )
{
	if( !CEtLight::GetShadowCastDirLightInfo() ) return;

	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	if( !Option.bUseTerrainLightMap )
	{
		Option.bUseTerrainLightMap = true;
		GetEtOptionController()->SetGraphicOption( Option );
	}

#ifdef PRE_FIX_MATERIAL_DUMP
	EtMaterialHandle hBakeLightMapMaterial = LoadResource( "BakeTerrainLightMap.fx", RT_SHADER );
	EtMaterialHandle hGaussianFilter = LoadResource( "GaussianFilter.fx", RT_SHADER );
	if( !hBakeLightMapMaterial ) return;
	if( !hGaussianFilter ) return;
#endif

	// Halve lightmap size on low-spec.
	bool bBakeLightMap = true;
	if( false == Option.bUseSplatting )
	{
		if( false == Option.bIsOnlyLowShaderAvailable )
		{
			nWidth /= 2;
			nHeight /= 2;
		}
		else
		{
			// Only splat 2 layers due to low-spec option, but
			// For low-end graphics cards that cannot run the splatting shader, fall back to the legacy terrain rendering method.
			BakeLowDetailMap();
			bBakeLightMap = false;
		}
	}

	if( bBakeLightMap )
	{
		int i, nStart, nEnd;
		static EtMatrix LightMapViewProjMat;

		EtTextureHandle hBlurLightMap = CEtTexture::CreateRenderTargetTexture( nWidth * 2, nHeight * 2, FMT_A8R8G8B8 );
		EtTextureHandle hBlurMoreLightMap = CEtTexture::CreateRenderTargetTexture( nWidth * 2, nHeight * 2, FMT_A8R8G8B8 );	
		EtTextureHandle hRoughLightMap = CEtTexture::CreateRenderTargetTexture( nWidth * 2, nHeight * 2, FMT_A8R8G8B8 );	

		EtTextureHandle hFinalLightMap = CEtTexture::CreateRenderTargetTexture( nWidth, nHeight, FMT_A8R8G8B8 );
		
		EtTextureHandle hDepthLightMap = CEtTexture::CreateRenderTargetTexture( nWidth * 2, nHeight * 2, FMT_R32F );

		LPD3DXRENDERTOSURFACE   pBlurSurfaceMng = NULL, pFinalSurfaceMng = NULL, pDepthSurfaceMng = NULL;
		GetEtDevice()->CreateRenderToSurface( nWidth * 2, nHeight * 2, FMT_A8R8G8B8, FALSE, FMT_UNKNOWN, &pBlurSurfaceMng );
		GetEtDevice()->CreateRenderToSurface( nWidth * 2, nHeight * 2, FMT_R32F, TRUE, FMT_D24S8, &pDepthSurfaceMng );
		GetEtDevice()->CreateRenderToSurface( nWidth, nHeight, FMT_A8R8G8B8, FALSE, FMT_UNKNOWN, &pFinalSurfaceMng );

		EtTextureHandle hInfluenceMap = CEtTexture::CreateNormalTexture( nWidth, nHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_SYSTEMMEM );
		//EtDepthHandle hDepth = CEtDepth::CreateDepthStencil( nWidth * 2, nHeight * 2 );

#ifdef PRE_FIX_MATERIAL_DUMP
#else
	EtMaterialHandle hBakeLightMapMaterial = LoadResource( "BakeTerrainLightMap.fx", RT_SHADER );
	EtMaterialHandle hGaussianFilter = LoadResource( "GaussianFilter.fx", RT_SHADER );
#endif

		SCameraInfo CameraInfo;
		EtCameraHandle hCamera;
		CameraInfo.fViewWidth = m_TerrainInfo.fTileSize * m_nBlockSizeX;
		CameraInfo.fViewHeight = m_TerrainInfo.fTileSize * m_nBlockSizeY;
		CameraInfo.fWidth = ( float )nWidth;
		CameraInfo.fHeight = ( float )nHeight;
		CameraInfo.Type = CT_ORTHOGONAL;
		CameraInfo.Target = CT_RENDERTARGET_NO_GENERATE_BACKBUFFER;
		hCamera = CreateCamera( &CameraInfo );

		EtViewPort viewPort;
		GetEtDevice()->GetViewport( &viewPort );

		CalcSelectCount( nBlockIndex, nStart, nEnd );
		for( i = nStart; i < nEnd; i++ )
		{
			std::vector< SCustomParam > vecCustomParam;
			SAABox BoundingBox;
			int nTexIndex;
			EtVector3 Eye, At, TerrainOffset;

			BakeDepthLightMap( i, hDepthLightMap, pDepthSurfaceMng, hBakeLightMapMaterial, LightMapViewProjMat, nBakeRange, fSlopeBias );
			BakeRoughLightMap( i, hRoughLightMap, hDepthLightMap, pBlurSurfaceMng, hBakeLightMapMaterial, hCamera, nWidth * 2, nHeight * 2, LightMapViewProjMat );

			float fStart, fEnd;
			EtVector4 FilterRadius;

			GetEtDevice()->EnableZ( false );
			GetEtDevice()->SetCullMode( CULL_NONE );
			//GetEtDevice()->SetDepthStencilSurface( NULL );
			vecCustomParam.clear();
			FilterRadius.x = fBlurSize * 2 / nWidth;
			FilterRadius.y = fBlurSize * 2 / nHeight;
			AddCustomParam( vecCustomParam, EPT_VECTOR, hGaussianFilter, "g_fBlurRadius", &FilterRadius );
			nTexIndex = hRoughLightMap->GetMyIndex();
			AddCustomParam( vecCustomParam, EPT_TEX, hGaussianFilter, "g_GaussianSource", &nTexIndex );

			if( SUCCEEDED(pBlurSurfaceMng->BeginScene( hBlurLightMap->GetSurfaceLevel(), NULL ) ) ) {
				DrawQuadIm( hGaussianFilter, vecCustomParam, 0, EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ), EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ) );
				pBlurSurfaceMng->EndScene( 0 );
			}

			nTexIndex = hBlurLightMap->GetMyIndex();
			AddCustomParam( vecCustomParam, EPT_TEX, hGaussianFilter, "g_GaussianSource", &nTexIndex );
			if( SUCCEEDED(pBlurSurfaceMng->BeginScene( hBlurMoreLightMap->GetSurfaceLevel(), NULL ) ) ) {
				DrawQuadIm( hGaussianFilter, vecCustomParam, 1, EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ), EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ) );
				pBlurSurfaceMng->EndScene( 0 );
			}

			vecCustomParam.clear();
			nTexIndex = hBlurMoreLightMap->GetMyIndex();
			AddCustomParam( vecCustomParam, EPT_TEX, hBakeLightMapMaterial, "g_LightMapTex", &nTexIndex );
			if( SUCCEEDED(pFinalSurfaceMng->BeginScene( hFinalLightMap->GetSurfaceLevel(), NULL ) ) ) {
				fStart = ( nWidth / 2.0f - 1.0f ) / ( nWidth * 2.0f );
				fEnd = ( nWidth * 3.0f / 2.0f + 1.0f ) / ( nWidth * 2.0f );
				DrawQuadIm( hBakeLightMapMaterial, vecCustomParam, 2, EtVector2( 0.0f, 0.0f ), EtVector2( 1.0f, 1.0f ), EtVector2( fStart, fStart ), EtVector2( fEnd, fEnd ) );
				pFinalSurfaceMng->EndScene( 0 );
			}

			GetEtDevice()->SetCullMode( CULL_CCW );
			GetEtDevice()->EnableZ( true );

			HRESULT hResult = ( ( IDirect3DDevice9 * )GetEtDevice()->GetDevicePtr() )->GetRenderTargetData( hFinalLightMap->GetSurfaceLevel(), hInfluenceMap->GetSurfaceLevel() );
			
			if( SUCCEEDED( hResult ) )  {
				int nTexStride;
				char *pTexBuffer;
				pTexBuffer = ( char * )hInfluenceMap->Lock( nTexStride );
				SetLightMapInfluence( i, pTexBuffer, nTexStride, hFinalLightMap->Width(), hFinalLightMap->Height() );
				hInfluenceMap->Unlock();
				EtTextureHandle hLightMap = CEtTexture::CreateNormalTexture( nWidth, nHeight, FMT_DXT1, USAGE_DEFAULT, POOL_MANAGED );
				hLightMap->Copy( hInfluenceMap );
				m_vecTerrain[ i ]->SetLightMap( hLightMap );
			}
		}

		GetEtDevice()->SetViewport( &viewPort );

		SAFE_RELEASE_SPTR( hCamera );
		SAFE_RELEASE_SPTR( hGaussianFilter );

		SAFE_RELEASE_SPTR( hInfluenceMap );
		SAFE_RELEASE_SPTR( hFinalLightMap );
		SAFE_RELEASE_SPTR( hRoughLightMap );
		SAFE_RELEASE_SPTR( hBakeLightMapMaterial );
		SAFE_RELEASE_SPTR( hDepthLightMap );
		SAFE_RELEASE_SPTR( hBlurLightMap );
		SAFE_RELEASE_SPTR( hBlurMoreLightMap );

		SAFE_RELEASE( pBlurSurfaceMng );
		SAFE_RELEASE( pFinalSurfaceMng );
		SAFE_RELEASE( pDepthSurfaceMng );
	}

	CEtObject::RecalcLightMapInfluence();
}

bool CEtTerrainArea::_BakeLightMapFromFile( int nBlockIndex, EtTextureHandle hTexture )
{
	EtTextureHandle hInfluenceMap = CEtTexture::CreateNormalTexture( hTexture->Width(), hTexture->Height(), FMT_A8R8G8B8, USAGE_DEFAULT, POOL_SYSTEMMEM );
	if( !hInfluenceMap ) return false;
	hInfluenceMap->Copy( hTexture );
	int nTexStride;
	char *pTexBuffer;
	pTexBuffer = ( char * )hInfluenceMap->Lock( nTexStride );
	SetLightMapInfluence( nBlockIndex, pTexBuffer, nTexStride, hTexture->Width(), hTexture->Height() );
	hInfluenceMap->Unlock();
	m_vecTerrain[ nBlockIndex ]->SetLightMap( hTexture );
	SAFE_RELEASE_SPTR( hInfluenceMap );
	return true;
}

void CEtTerrainArea::BakeLowDetailMap()
{
	GetEtShadowMap()->ResetShadowMap();

	int i, nSize;
	nSize = (int)m_vecTerrain.size();
	for( i = 0; i < nSize; i++) {		
		m_vecTerrain[ i ]->BakeLowDetailMap();
		m_vecTerrain[ i ]->SetCommonIndexBuffer( m_pCommonLowIndexBuffer, (m_nBlockSizeX/2) * (m_nBlockSizeY/2) * 2 );
	}
}

void CEtTerrainArea::CreateLightCamera( int nBlockIndex, EtCameraHandle &hCamera, int nWidth, int nHeight, EtMatrix &LightMapViewProjMat )
{
	SAABox BoundingBox;
	SCameraInfo CameraInfo;
	EtVector3 Eye, At, LightDir, TerrainOffset;
	EtMatrix ScaleMat, TransMat, *pProjMat, *pViewMat;

	LightDir = -CEtLight::GetShadowCastDirLightInfo()->Direction;

	TerrainOffset = m_vecTerrain[ nBlockIndex ]->GetTerrainInfo()->TerrainOffset;
	BoundingBox = *GetBoundingBox( nBlockIndex );

	BoundingBox.Max.y = m_BoundingBox.Max.y;
	BoundingBox.Min.y = m_BoundingBox.Min.y;

	BoundingBox.Max.x -= TerrainOffset.x;
	BoundingBox.Max.z -= TerrainOffset.z;
	BoundingBox.Min.x -= TerrainOffset.x;
	BoundingBox.Min.z -= TerrainOffset.z;
	if( BoundingBox.Max.y - BoundingBox.Min.y < 3000.0f )
	{
		BoundingBox.Max.y = BoundingBox.Min.y + 3000.0f;
	}
	BoundingBox.Max.y += ( BoundingBox.Max.y - BoundingBox.Min.y ) * 0.05f;
	BoundingBox.Min.y -= ( BoundingBox.Max.y - BoundingBox.Min.y ) * 0.05f;

	CameraInfo.fViewWidth = m_TerrainInfo.fTileSize * m_nBlockSizeX;
	CameraInfo.fViewHeight = m_TerrainInfo.fTileSize * m_nBlockSizeY;
	CameraInfo.fWidth = ( float )nWidth;
	CameraInfo.fHeight = ( float )nHeight;
	CameraInfo.Target = CT_RENDERTARGET_NO_GENERATE_BACKBUFFER;
	CameraInfo.Type = CT_ORTHOGONAL;
	CameraInfo.fFar = ( BoundingBox.Max.y - BoundingBox.Min.y );
	hCamera = CreateCamera( &CameraInfo );
	At = ( BoundingBox.Max + BoundingBox.Min ) / 2.0f;
	At.y = BoundingBox.Min.y;
	Eye = At + LightDir * ( BoundingBox.Max.y - BoundingBox.Min.y );
	hCamera->LookAt( Eye, At, EtVector3( 0.0f, 1.0f, 0.0f ) );

	pViewMat = hCamera->GetViewMat();
	pViewMat->_13 = 0.0f;
	pViewMat->_23 = -1.0f;
	pViewMat->_33 = 0.0f;
	BoundingBox.Transform( *hCamera->GetViewProjMat() );
	EtMatrixScaling( &ScaleMat, 1.0f / ( BoundingBox.Max.x - BoundingBox.Min.x ), 1.0f / ( BoundingBox.Max.y - BoundingBox.Min.y ), 
		1.0f / ( BoundingBox.Max.z - BoundingBox.Min.z ) );
	EtMatrixTranslation( &TransMat, -( BoundingBox.Max.x + BoundingBox.Min.x ) / ( BoundingBox.Max.x - BoundingBox.Min.x ), 
		-( BoundingBox.Max.y + BoundingBox.Min.y ) / ( BoundingBox.Max.y - BoundingBox.Min.y ), -BoundingBox.Min.z );
	pProjMat = hCamera->GetProjMat();
	EtMatrixMultiply( pProjMat, pProjMat, &ScaleMat );
	EtMatrixMultiply( pProjMat, pProjMat, &TransMat );
	LightMapViewProjMat = *hCamera->GetViewProjMat();
}

EtTextureHandle CEtTerrainArea::BakeDepthLightMap( int nBlockIndex, EtTextureHandle hLightMap, LPD3DXRENDERTOSURFACE pDepthSurface, EtMaterialHandle hMaterial,
												  EtMatrix &LightMapViewProjMat, int nBakeRange, float fSlopeBias )
{
	EtCameraHandle hCamera;

	CreateLightCamera( nBlockIndex, hCamera, hLightMap->Width(), hLightMap->Height(), LightMapViewProjMat );

	hCamera->Activate();
	if( SUCCEEDED( pDepthSurface->BeginScene( hLightMap->GetSurfaceLevel(), NULL ) ) )
	{
		GetEtDevice()->EnableZ( true );
		GetEtDevice()->ClearBuffer( 0xffffffff, 1.0f, 0 );
		GetEtDevice()->SetCullMode( CULL_CW );

		int i, j, nCount;
		EtMatrix WorldMat;
		EtVector3 TerrainOffset;

		float fBlockWidth, fBlockHeight;

		fBlockWidth = m_nBlockSizeX * m_TerrainInfo.fTileSize;
		fBlockHeight = m_nBlockSizeY * m_TerrainInfo.fTileSize;
		TerrainOffset = m_vecTerrain[ nBlockIndex ]->GetTerrainInfo()->TerrainOffset;

		{
			ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
			nCount = CEtObject::GetItemCount();
			for( i = 0; i < nCount; i++ )
			{
				EtObjectHandle hObject;
				EtSkinHandle hSkin;

				hObject = CEtObject::GetItem( i );
				if( !hObject )
				{
					continue;
				}
				if( !hObject->IsLightMapCast() )
				{
					continue;
				}


				WorldMat = *hObject->GetWorldMat();
				WorldMat._41 -= TerrainOffset.x;
				WorldMat._43 -= TerrainOffset.z;
				if( ( WorldMat._41 > -fBlockWidth ) && ( WorldMat._41 < fBlockWidth * 2 ) && ( WorldMat._43 > -fBlockHeight ) && ( WorldMat._43 < fBlockHeight * 2 ) )
				{
					int nTexIndex;
					hSkin = hObject->GetSkin();
					if( hSkin && hSkin->GetMeshHandle() )
					{
						std::vector< std::vector< SCustomParam > > vecCustomParam;
						vecCustomParam.resize( hSkin->GetMeshHandle()->GetSubMeshCount() );

						for( int j = 0; j < hSkin->GetMeshHandle()->GetSubMeshCount(); j++) {
							AddCustomParam( vecCustomParam[j], EPT_FLOAT, hMaterial, "g_fSlopeBias", &fSlopeBias );
							nTexIndex = hSkin->GetDiffuseTexIndex( j );
							AddCustomParam( vecCustomParam[j], EPT_TEX, hMaterial, "g_DiffuseTex", &nTexIndex );
						}
						hSkin->SetExternalMaterial( hMaterial, NULL, &vecCustomParam );
						hSkin->Render( WorldMat, WorldMat, 1.0f, false );
						hSkin->ClearExternalMaterial();
					}
				}
			}
		}

		std::vector< SCustomParam > vecCustomParam;

		AddCustomParam( vecCustomParam, EPT_FLOAT, hMaterial, "g_fSlopeBias", &fSlopeBias );
		int nTexIndex = -1;
		AddCustomParam( vecCustomParam, EPT_TEX, hMaterial, "g_DiffuseTex", &nTexIndex );

		for( i = -nBakeRange; i <= nBakeRange; i++ )
		{
			for( j = -nBakeRange; j <= nBakeRange; j++ )
			{
				int nCurTerrainIndex;

				EtMatrixIdentity( &WorldMat );
				nCurTerrainIndex = nBlockIndex + i * m_nBlockCountX + j;
				if( ( nCurTerrainIndex < 0 ) || ( nCurTerrainIndex >= ( int )m_vecTerrain.size() ) )
				{
					continue;
				}
				EtMatrixTranslation( &WorldMat, j * m_nBlockSizeX * m_TerrainInfo.fTileSize, 0.0f, i * m_nBlockSizeY * m_TerrainInfo.fTileSize );
				hMaterial->SetTechnique( 0 );
				hMaterial->SetWorldMatParams( &WorldMat, &WorldMat );			
				hMaterial->SetCustomParamList( vecCustomParam );			
				int nPasses;
				hMaterial->BeginEffect( nPasses );
				hMaterial->BeginPass( 0 );
				hMaterial->CommitChanges();
				m_vecTerrain[ nCurTerrainIndex ]->GetMeshStream().Draw( hMaterial->GetVertexDeclIndex( 0, 0 ) );
				hMaterial->EndPass();
				hMaterial->EndEffect();
			}
		}

		pDepthSurface->EndScene( 0 );
	}
	GetEtDevice()->SetCullMode( CULL_NONE );

	SAFE_RELEASE_SPTR( hCamera );

	return hLightMap;
}

void CEtTerrainArea::BakeRoughLightMap( int nBlockIndex, EtTextureHandle hRoughLightMap, EtTextureHandle hDepthLightMap, LPD3DXRENDERTOSURFACE pRoughSurface, EtMaterialHandle hMaterial, EtCameraHandle hCamera, int nWidth, int nHeight, EtMatrix &LightMapViewProjMat )
{
	std::vector< SCustomParam > vecCustomParam;
	SAABox BoundingBox;
	int i, j, nTexIndex;
	EtVector3 Eye, At, TerrainOffset;
	EtMatrix WorldLightViewProjMat;

	SCameraInfo CameraInfo;
	CameraInfo.fViewWidth = m_TerrainInfo.fTileSize * m_nBlockSizeX * 2.0f;
	CameraInfo.fViewHeight = m_TerrainInfo.fTileSize * m_nBlockSizeY * 2.0f;
	CameraInfo.fWidth = ( float )nWidth;
	CameraInfo.fHeight = ( float )nHeight;
	CameraInfo.Type = CT_ORTHOGONAL;
	CameraInfo.Target = CT_RENDERTARGET_NO_GENERATE_BACKBUFFER;

	TerrainOffset = m_vecTerrain[ nBlockIndex ]->GetTerrainInfo()->TerrainOffset;
	BoundingBox = *GetBoundingBox( nBlockIndex );

	BoundingBox.Max.y = m_BoundingBox.Max.y;
	BoundingBox.Min.y = m_BoundingBox.Min.y;

	BoundingBox.Max.x -= TerrainOffset.x;
	BoundingBox.Max.z -= TerrainOffset.z;
	BoundingBox.Min.x -= TerrainOffset.x;
	BoundingBox.Min.z -= TerrainOffset.z;
	if( BoundingBox.Max.y - BoundingBox.Min.y < 1000.0f )
	{
		BoundingBox.Max.y = BoundingBox.Min.y + 1000.0f;
	}
	CameraInfo.fFar = ( BoundingBox.Max.y - BoundingBox.Min.y ) * 1.1f;
	hCamera->Initialize( &CameraInfo );
	At = ( BoundingBox.Max + BoundingBox.Min ) / 2.0f;
	At.y = BoundingBox.Min.y;
	Eye = At;
	Eye.y += ( BoundingBox.Max.y - BoundingBox.Min.y ) * 1.05f;
	hCamera->LookAt( Eye, At, EtVector3( 0.0f, 0.0f, 1.0f ) );

	nTexIndex = hDepthLightMap->GetMyIndex();
	AddCustomParam( vecCustomParam, EPT_TEX, hMaterial, "g_LightMapTex", &nTexIndex );
	AddCustomParam( vecCustomParam, EPT_MATRIX_PTR, hMaterial, "g_BakeLightViewProjMat", &WorldLightViewProjMat );

	EtMatrix WorldMat;
	int nPasses;
	GetEtDevice()->EnableZ( false );
	GetEtDevice()->SetCullMode( CULL_NONE );	
	hCamera->Activate();

	if( SUCCEEDED( pRoughSurface->BeginScene( hRoughLightMap->GetSurfaceLevel() , NULL) ) )
	{
		GetEtDevice()->ClearBuffer( 0xffffffff, 1.0f, 0 );
		CEtLight::SetDirLightAttenuation( 1.0f );
		for( i = -1; i <= 1; i++ )
		{
			for( j = -1; j <= 1; j++ )
			{
				int nCurTerrainIndex;

				EtMatrixIdentity( &WorldMat );
				nCurTerrainIndex = nBlockIndex + i * m_nBlockCountX + j;
				if( ( nCurTerrainIndex < 0 ) || ( nCurTerrainIndex >= ( int )m_vecTerrain.size() ) )
				{
					continue;
				}
				EtMatrixTranslation( &WorldMat, j * m_nBlockSizeX * m_TerrainInfo.fTileSize, 0.0f, i * m_nBlockSizeY * m_TerrainInfo.fTileSize );
				EtMatrixMultiply( &WorldLightViewProjMat, &WorldMat, &LightMapViewProjMat );
				hMaterial->SetTechnique( 1 );
				hMaterial->SetGlobalParams();			
				hMaterial->SetWorldMatParams( &WorldMat, &WorldMat );
				hMaterial->SetCustomParamList( vecCustomParam );			
				hMaterial->BeginEffect( nPasses );
				hMaterial->BeginPass( 0 );
				hMaterial->CommitChanges();
				m_vecTerrain[ nCurTerrainIndex ]->GetMeshStream().Draw( hMaterial->GetVertexDeclIndex( 1, 0 ) );
				hMaterial->EndPass();
				hMaterial->EndEffect();
			}
		}
		GetEtDevice()->SetCullMode( CULL_CCW );
		GetEtDevice()->EnableZ( true );
		pRoughSurface->EndScene( 0 );
	}
	
}

void CEtTerrainArea::CreateLightMapInfluenceBuffer()
{
	if( m_pLightMapInfluenceBuffer )
	{
		return;
	}

	m_nLightMapInfluenceStride = m_TerrainInfo.nSizeX / 8;
	if( m_TerrainInfo.nSizeX % 8 )
	{
		m_nLightMapInfluenceStride++;
	}
	m_pLightMapInfluenceBuffer = new char[ m_nLightMapInfluenceStride * (m_TerrainInfo.nSizeY+1) ];
	memset( m_pLightMapInfluenceBuffer, 0, m_nLightMapInfluenceStride * (m_TerrainInfo.nSizeY+1) );
}

void CEtTerrainArea::SetLightMapInfluence( int nBlockIndex, char *pTexBuffer, int nTexStride, int nTexWidth, int nTexHeight )
{
	CreateLightMapInfluenceBuffer();

	int i, j, nBlockStartX, nBlockStartY;

	nBlockStartX = nBlockIndex % m_nBlockCountX * m_nBlockSizeX;
	nBlockStartY = nBlockIndex / m_nBlockCountX * m_nBlockSizeY;
	for( i = 0; i < m_nBlockSizeY; i++ )
	{
		for( j = 0; j < m_nBlockSizeX; j++ )
		{
			int nHori, nVert;
			int nOffset, nTexOffset;
			char cShift;

			nHori = ( int )( j / ( m_nBlockSizeX - 1.0f ) * ( nTexWidth - 1 ) );
			nVert = ( int )( i / ( m_nBlockSizeY - 1.0f ) * ( nTexHeight - 1 ) );
			nTexOffset = nVert * nTexStride + nHori * 4;
			if( pTexBuffer[ nTexOffset + 1 ] == 0 )
			{
				nOffset = ( i + nBlockStartY ) * m_nLightMapInfluenceStride + ( j + nBlockStartX ) / 8;
				cShift = ( ( char )( 0x1 ) ) << ( ( j + nBlockStartX ) % 8 );
				m_pLightMapInfluenceBuffer[ nOffset ] |= cShift;
			}
		}
	}

	if( !m_vecGrassBlock.empty() )
	{
		int nGrassBlockIndex;

		nGrassBlockIndex = ( nBlockIndex % m_nBlockCountX ) * 2 + ( nBlockIndex / m_nBlockCountX ) * m_nBlockCountX * 4;
		if( m_vecGrassBlock[ nGrassBlockIndex ] )
		{
			m_vecGrassBlock[ nGrassBlockIndex ]->SetLightMapInfluence( pTexBuffer, nTexStride, nTexWidth, nTexHeight, 0.0f, 0.0f );
		}
		nGrassBlockIndex++;
		if( m_vecGrassBlock[ nGrassBlockIndex ] )
		{
			m_vecGrassBlock[ nGrassBlockIndex ]->SetLightMapInfluence( pTexBuffer, nTexStride, nTexWidth, nTexHeight, 0.5f, 0.0f );
		}
		nGrassBlockIndex += m_nBlockCountX * 2;
		if( m_vecGrassBlock[ nGrassBlockIndex ] )
		{
			m_vecGrassBlock[ nGrassBlockIndex ]->SetLightMapInfluence( pTexBuffer, nTexStride, nTexWidth, nTexHeight, 0.5f, 0.5f );
		}
		nGrassBlockIndex--;
		if( m_vecGrassBlock[ nGrassBlockIndex ] )
		{
			m_vecGrassBlock[ nGrassBlockIndex ]->SetLightMapInfluence( pTexBuffer, nTexStride, nTexWidth, nTexHeight, 0.0f, 0.5f );
		}
	}
}

float CEtTerrainArea::GetLightMapInfluence( float fX, float fZ )
{
	if( m_pLightMapInfluenceBuffer == NULL )
	{
		return 1.0f;
	}

	int nCellX = 0 , nCellZ = 0;
	float fWeight[ 4 ], fValue;
	char cShift;

	CalcCellPosition( fX, fZ, nCellX, nCellZ, fWeight );

	if ( nCellX < 0 || nCellZ < 0 )
		return 1.0f;

	cShift = ( ( char )( 0x1 ) ) << ( nCellX % 8 );
	fValue = m_pLightMapInfluenceBuffer[ nCellZ * m_nLightMapInfluenceStride + nCellX / 8 ] & cShift ? fWeight[ 0 ] : 0.0f;
	nCellX++;
	cShift = ( ( char )( 0x1 ) ) << ( nCellX % 8 );
	fValue += m_pLightMapInfluenceBuffer[ nCellZ * m_nLightMapInfluenceStride + nCellX / 8 ] & cShift ? fWeight[ 1 ] : 0.0f;
	nCellZ++;
	cShift = ( ( char )( 0x1 ) ) << ( nCellX % 8 );
	fValue += m_pLightMapInfluenceBuffer[ nCellZ * m_nLightMapInfluenceStride + nCellX / 8 ] & cShift ? fWeight[ 3 ] : 0.0f;
	nCellX--;
	cShift = ( ( char )( 0x1 ) ) << ( nCellX % 8 );
	fValue += m_pLightMapInfluenceBuffer[ nCellZ * m_nLightMapInfluenceStride + nCellX / 8 ] & cShift ? fWeight[ 2 ] : 0.0f;

	return 1.0f - fValue;
}

float CEtTerrainArea::CalcLightMapInfluence( float fX, float fZ )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i;

	for( i = 0; i < GetItemCount(); i++ )
	{
		EtTerrainHandle hTerrainArea;

		hTerrainArea = GetItem( i );
		if( hTerrainArea->IsInside( fX, fZ ) )
		{
			return hTerrainArea->GetLightMapInfluence( fX, fZ );
		}
	}

	return 1.0f;
}

void CEtTerrainArea::CalcCellPosition( float fX, float fZ, int &nCellX, int &nCellZ, float *pWeight, int nScale )
{
	float fModX, fModZ;

	fX -= m_TerrainInfo.TerrainOffset.x;
	fZ -= m_TerrainInfo.TerrainOffset.z;
	if( fX < 0.0f )
	{
		fX = 0.f;
	}
	else if( fX >= m_TerrainInfo.nSizeX * m_TerrainInfo.fTileSize ) 
	{
		fX = m_TerrainInfo.nSizeX * m_TerrainInfo.fTileSize - 0.1f;
	}

	if( fZ < 0.f )
	{
		fZ = 0.f;
	}
	else if( fZ >= m_TerrainInfo.nSizeY * m_TerrainInfo.fTileSize )
	{
		fZ = m_TerrainInfo.nSizeY * m_TerrainInfo.fTileSize - 0.1f;
	}

	nCellX = ( int )( fX / (m_TerrainInfo.fTileSize*nScale) );
	nCellZ = ( int )( fZ / (m_TerrainInfo.fTileSize*nScale) );
	fModX = fX / (m_TerrainInfo.fTileSize*nScale) - nCellX;
	fModZ = fZ / (m_TerrainInfo.fTileSize*nScale) - nCellZ;
	

	if( fModX > fModZ ) 
	{
		pWeight[ 0 ] = 1.0f - fModX;
		pWeight[ 1 ] = fModX - fModZ;
		pWeight[ 2 ] = 0.0f;
		pWeight[ 3 ] = fModZ;
	}
	else if( fModX < fModZ ) 
	{
		pWeight[ 0 ] = 1.0f - fModZ;
		pWeight[ 1 ] = 0.0f;
		pWeight[ 2 ] = fModZ - fModX;
		pWeight[ 3 ] = fModX;
	}
	else
	{
		pWeight[ 0 ] = 1.0f - fModZ;
		pWeight[ 1 ] = 0.0f;
		pWeight[ 2 ] = 0.0f;
		pWeight[ 3 ] = fModZ;
	}
}

float CEtTerrainArea::GetLandHeight( float fX, float fZ, EtVector3 *pNormal, int nScale )
{
	int i;
	float fRet;
	int nCellX, nCellZ;
	float fHeight[ 4 ], fWeight[ 4 ];
	DWORD dwSectorWidth = m_TerrainInfo.nSizeX + 1;

	CalcCellPosition( fX, fZ, nCellX, nCellZ, fWeight, nScale );
	fHeight[ 0 ] = m_TerrainInfo.pHeight[ nCellZ * dwSectorWidth + nCellX ];
	fHeight[ 1 ] = m_TerrainInfo.pHeight[ nCellZ * dwSectorWidth + nCellX + nScale ];
	fHeight[ 2 ] = m_TerrainInfo.pHeight[ ( nCellZ + nScale ) * dwSectorWidth + nCellX ];
	fHeight[ 3 ] = m_TerrainInfo.pHeight[ ( nCellZ + nScale ) * dwSectorWidth + nCellX + nScale ];
	fRet = 0.0f;
	for( i = 0; i < 4; i++ )
	{
		fRet += fHeight[ i ] * fWeight[ i ];
	}

	if( pNormal )
	{
		GetVertexNormal( *pNormal, nCellX, nCellZ );
	}

	return fRet * m_TerrainInfo.fHeightMultiply;
}

void CEtTerrainArea::GetLandNormal( EtVector3 &Normal, float fX, float fZ )
{
	int i;
	EtVector3 ReturnVec, VertexNormal[ 4 ];
	int nCellX, nCellZ;
	float fWeight[ 4 ];

	CalcCellPosition( fX, fZ, nCellX, nCellZ, fWeight );
	GetVertexNormal( VertexNormal[ 0 ], nCellX, nCellZ );
	GetVertexNormal( VertexNormal[ 1 ], nCellX + 1, nCellZ );
	GetVertexNormal( VertexNormal[ 2 ], nCellX, nCellZ + 1 );
	GetVertexNormal( VertexNormal[ 3 ], nCellX + 1, nCellZ + 1 );
	ReturnVec = EtVector3( 0.0f, 0.0f, 0.0f );;
	for( i = 0; i < 4; i++ )
	{
		ReturnVec += VertexNormal[ i ] * fWeight[ i ];
	}
	EtVec3Normalize( &Normal, &ReturnVec );
}

void CEtTerrainArea::GetVertexNormal( EtVector3 &Normal, int nCellX, int nCellZ )
{
	int nHeightIndex;
	float fDX, fDZ;
	EtVector3 Return;

	DWORD dwTileWidth = m_TerrainInfo.nSizeX + 1;
	DWORD dwTileHeight = m_TerrainInfo.nSizeY + 1;
	nHeightIndex = dwTileWidth * nCellZ + nCellX;
	if( nCellX == 0 )
	{
		fDX = ( m_TerrainInfo.pHeight[ nHeightIndex ] - m_TerrainInfo.pHeight[ nHeightIndex + 1 ] ) / m_TerrainInfo.fTileSize;
	}
	else if( nCellX == dwTileWidth - 1 ) 
	{
		fDX = ( m_TerrainInfo.pHeight[ nHeightIndex - 1 ] - m_TerrainInfo.pHeight[ nHeightIndex ] ) / m_TerrainInfo.fTileSize;
	}
	else
	{
		fDX = ( m_TerrainInfo.pHeight[ nHeightIndex - 1 ] - m_TerrainInfo.pHeight[ nHeightIndex + 1 ] ) / ( m_TerrainInfo.fTileSize * 2 );
	}

	if( nCellZ == 0 ) 
	{
		fDZ = ( m_TerrainInfo.pHeight[ nHeightIndex ] - m_TerrainInfo.pHeight[ nHeightIndex + dwTileWidth ] ) / m_TerrainInfo.fTileSize;
	}
	else if( nCellZ == dwTileHeight - 1 )
	{
		fDZ = ( m_TerrainInfo.pHeight[ nHeightIndex - dwTileWidth ] - m_TerrainInfo.pHeight[ nHeightIndex ] ) / m_TerrainInfo.fTileSize;
	}
	else
	{
		fDZ = ( m_TerrainInfo.pHeight[ nHeightIndex - dwTileWidth ] - m_TerrainInfo.pHeight[ nHeightIndex + dwTileWidth ] ) / ( m_TerrainInfo.fTileSize * 2 );
	}

	Return.x = fDX;
	Return.y = 1.414f;
	Return.z = fDZ;
	EtVec3Normalize( &Normal, &Return );
}

void CEtTerrainArea::CreateGrassBlock()
{
	if( m_TerrainInfo.pGrassBuffer == NULL )
	{
		return;
	}

	int i, j;
	int nGrassBlockSize;

	m_nGrassBlockCountX = m_nBlockCountX * 2;
	m_nGrassBlockCountY = m_nBlockCountY * 2;

	nGrassBlockSize = DEFAULT_TERRAIN_SIZE / 2;

	for( i = 0; i < m_nGrassBlockCountY; i++ )
	{
		for( j = 0; j < m_nGrassBlockCountX; j++ )
		{
			CEtGrassBlock *pGrassBlock;
			SGrassBlockInfo BlockInfo;

			if( ( j + 1 ) * nGrassBlockSize <= m_TerrainInfo.nSizeX )
			{
				BlockInfo.nSizeX = nGrassBlockSize;
			}
			else
			{
				BlockInfo.nSizeX = m_TerrainInfo.nSizeX - j * nGrassBlockSize;
			}
			if( ( i + 1 ) * nGrassBlockSize <= m_TerrainInfo.nSizeY )
			{
				BlockInfo.nSizeY = nGrassBlockSize;
			}
			else
			{
				BlockInfo.nSizeY = m_TerrainInfo.nSizeY - i * nGrassBlockSize;
			}
			if( ( BlockInfo.nSizeX > 0 ) && ( BlockInfo.nSizeY > 0 ) )
			{
				pGrassBlock = new CEtGrassBlock();
				//				int nDivide = ( m_TerrainInfo.nSizeX / ( nGrassBlockSize * i ) ) ;
				BlockInfo.pGrassBuffer = m_TerrainInfo.pGrassBuffer + ( i * m_TerrainInfo.nSizeX * nGrassBlockSize ) + ( j * nGrassBlockSize );
				BlockInfo.nStride = m_TerrainInfo.nSizeX;
				BlockInfo.fTileSize = m_TerrainInfo.fTileSize;
				BlockInfo.GrassOffset.x = m_TerrainInfo.TerrainOffset.x + nGrassBlockSize * j * m_TerrainInfo.fTileSize;
				BlockInfo.GrassOffset.y = m_TerrainInfo.TerrainOffset.y;
				BlockInfo.GrassOffset.z = m_TerrainInfo.TerrainOffset.z + nGrassBlockSize * i * m_TerrainInfo.fTileSize;
				memcpy( BlockInfo.fGrassWidth, m_TerrainInfo.fGrassWidth, sizeof( float ) * 4 );
				memcpy( BlockInfo.fMaxGrassHeight, m_TerrainInfo.fMaxGrassHeight, sizeof( float ) * 4 );;
				memcpy( BlockInfo.fMinGrassHeight, m_TerrainInfo.fMinGrassHeight, sizeof( float ) * 4 );;;
				BlockInfo.fMaxShake = m_TerrainInfo.fMaxShake;
				BlockInfo.fMinShake = m_TerrainInfo.fMinShake;
				pGrassBlock->SetGrassBlockInfo( &BlockInfo );
			}
			else
			{
				pGrassBlock = NULL;
			}
			m_vecGrassBlock.push_back( pGrassBlock );
		}
	}
}

void CEtTerrainArea::CheckBakeLightMap()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, j, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		EtTerrainHandle hTerrainArea;

		hTerrainArea = GetItem( i );
		if( hTerrainArea )
		{
			int nBakeLightMapCount = hTerrainArea->GetBakeLightMapParamCount();
			SBakeLightMapParam Param;
			for( j = 0; j < nBakeLightMapCount; j++ )
			{
				hTerrainArea->GetBakeLightMapParam( j, Param );
				hTerrainArea->_BakeLightMap( Param.nBlockIndex, Param.nWidth, Param.nHeight, Param.fBlurSize, Param.nBakeRange, Param.fSlopeBias );
			}
			hTerrainArea->m_vecBakeLightMapParam.clear();
		}
	}
}


void CEtTerrainArea::ReloadMaterial()
{
	CEtResource::FlushWaitDelete();		// Delete all Materials with RefCount 0.
	EtMaterialHandle hMaterial;
	hMaterial = CEtResource::GetResource( "LayeredTerrain.fx" );	
	if( hMaterial )
	{
		hMaterial->Reload();
//		SAFE_RELEASE_SPTR( hMaterial );
		//ASSERT( hMaterial->GetRefCount() > 0 );
	}
	hMaterial = CEtResource::GetResource( "LayeredCliffTerrain.fx" );	
	if( hMaterial )
	{
		hMaterial->Reload();
//		SAFE_RELEASE_SPTR( hMaterial );
		//ASSERT( hMaterial->GetRefCount() > 0 );b
	}
}

EtTerrainHandle CEtTerrainArea::GetTerrainArea( float fX, float fZ )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i;

	for( i = 0; i < GetItemCount(); i++ )
	{
		EtTerrainHandle hTerrainArea;

		hTerrainArea = GetItem( i );
		if( hTerrainArea->IsInside( fX, fZ ) )
		{
			return hTerrainArea;
		}
	}

	return CEtTerrainArea::Identity();
}

void CEtTerrainArea::SetInteractivePos( EtVector3 *vPos, float fDelta )
{
	int i, nSize;
	nSize = ( int )m_vecGrassBlock.size();
	for( i = 0; i < nSize; i++ )
	{
		if( ( m_vecGrassBlock[ i ] ) && ( m_vecGrassBlock[ i ]->GetGrassCount() > 0 ) )
		{
			m_vecGrassBlock[ i ]->SetInteractivePos( vPos, fDelta );
		}
	}	
}