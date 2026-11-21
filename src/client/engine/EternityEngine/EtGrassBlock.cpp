#include "StdAfx.h"
#include "EtGrassBlock.h"
#include "EtRenderStack.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtGrassBlock::CEtGrassBlock(void)
{
	m_InteractivePos = EtVector4(0,0,0,0);
	m_InteractiveVelocity = EtVector2(0,0);
	m_nRenderUniqueID = -1;
	m_nGrassCount = 0;
}

CEtGrassBlock::~CEtGrassBlock(void)
{
	Clear();
}

void CEtGrassBlock::Clear()
{
	SAFE_RELEASE_SPTR( m_hMaterial );
	SAFE_RELEASE_SPTR( m_hTexture );
	m_MeshStream.Clear();
}

void CEtGrassBlock::Initialize( CEtTerrainArea *pTerrainArea )
{
//	Clear();

	SAFE_RELEASE_SPTR( m_hMaterial );
	m_MeshStream.Clear();
	CreateMaterial();
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	int i, j, k;

	m_nGrassCount = 0;
	for( i = 0; i < m_GrassBlockInfo.nSizeY; i++ )
	{
		for( j = 0; j < m_GrassBlockInfo.nSizeX; j++ )
		{
			char cValue;

			cValue = GetGrassValue( j, i );
			m_nGrassCount += cValue & 0x03;
			m_nGrassCount += ( cValue >> 2 ) & 0x03;
			m_nGrassCount += ( cValue >> 4 ) & 0x03;
			m_nGrassCount += ( cValue >> 6 ) & 0x03;
		}
	}
	if( m_nGrassCount <= 0 )
	{
		return;
	}

	int nVertexCount, nIndexCount, nCurGrass;
	EtVector3 *pPosition;
	EtVector2 *pTexCoord;
	float *pShake;
	DWORD *pColor;
	WORD *pIndex;

	nVertexCount = m_nGrassCount * 3 * 4;
	nIndexCount = m_nGrassCount * 18;
	pPosition = new EtVector3[ nVertexCount ];
	pTexCoord = new EtVector2[ nVertexCount ];
	pColor = new DWORD[ nVertexCount ];
	pShake = new float[ nVertexCount ];
	memset( pColor, 0xff, sizeof( DWORD ) * nVertexCount );
	pIndex = new WORD[ nIndexCount ];
	nCurGrass = 0;
	m_BoundingBox.Reset();
	for( i = 0; i < m_GrassBlockInfo.nSizeY; i++ )
	{
		for( j = 0; j < m_GrassBlockInfo.nSizeX; j++ )
		{
			char cGrassValue;

			cGrassValue = GetGrassValue( j, i );
			for( k = 0; k < ( cGrassValue & 0x03 ); k++ )
			{
				CreateGrass( pTerrainArea, j, i, nCurGrass, 0, pPosition + nCurGrass * 12, pTexCoord + nCurGrass * 12, pShake + nCurGrass * 12, pIndex + nCurGrass * 18 );
				nCurGrass++;
			}
			for( k = 0; k < ( ( cGrassValue >> 2 ) & 0x03 ); k++ )
			{
				CreateGrass( pTerrainArea, j, i, nCurGrass, 1, pPosition + nCurGrass * 12, pTexCoord + nCurGrass * 12, pShake + nCurGrass * 12, pIndex + nCurGrass * 18 );
				nCurGrass++;
			}
			for( k = 0; k < ( ( cGrassValue >> 4 ) & 0x03 ); k++ )
			{
				CreateGrass( pTerrainArea, j, i, nCurGrass, 2, pPosition + nCurGrass * 12, pTexCoord + nCurGrass * 12, pShake + nCurGrass * 12, pIndex + nCurGrass * 18 );
				nCurGrass++;
			}
			for( k = 0; k < ( ( cGrassValue >> 6 ) & 0x03 ); k++ )
			{
				CreateGrass( pTerrainArea, j, i, nCurGrass, 3, pPosition + nCurGrass * 12, pTexCoord + nCurGrass * 12, pShake + nCurGrass * 12, pIndex + nCurGrass * 18 );
				nCurGrass++;
			}
		}
	}

	CMemoryStream Stream;
	Stream.Initialize( pPosition, sizeof( EtVector3 ) * nVertexCount );
	m_MeshStream.LoadVertexStream( &Stream, MST_POSITION, 0, nVertexCount );
	Stream.Initialize( pTexCoord, sizeof( EtVector2 ) * nVertexCount );
	m_MeshStream.LoadVertexStream( &Stream, MST_TEXCOORD, 0, nVertexCount );
	Stream.Initialize( pShake, sizeof( float ) * nVertexCount );
	m_MeshStream.LoadVertexStream( &Stream, MST_DEPTH, 0, nVertexCount );
	Stream.Initialize( pColor, sizeof( DWORD ) * nVertexCount );
	m_MeshStream.LoadVertexStream( &Stream, MST_COLOR, 0, nVertexCount );
	Stream.Initialize( pIndex, sizeof( WORD ) * nIndexCount );
	m_MeshStream.LoadIndexStream( &Stream, false, nIndexCount );

	delete [] pIndex;
	delete [] pColor;
	delete [] pTexCoord;
	delete [] pPosition;
	delete [] pShake;
}

void CEtGrassBlock::CreateGrass( CEtTerrainArea *pTerrainArea, int nTileX, int nTileY, int nGrassIndex, char cKind, EtVector3 *pPosition, 
								EtVector2 *pTexCoord, float *pShake, WORD *pIndex )
{
	int nLoop;
	float fGrassWidth, fGrassHeight, fRotateAngle, fShake;
	EtVector3 BasePos[ 4 ], GrassPos, UpVec, XVec, ZVec;
	EtMatrix RotateMat, LandMat;

	GrassPos.x = m_GrassBlockInfo.GrassOffset.x + ( nTileX + rand() / ( float )RAND_MAX ) * m_GrassBlockInfo.fTileSize;
	GrassPos.z = m_GrassBlockInfo.GrassOffset.z + ( nTileY + rand() / ( float )RAND_MAX ) * m_GrassBlockInfo.fTileSize;
	GrassPos.y = pTerrainArea->GetLandHeight( GrassPos.x, GrassPos.z, &UpVec );
	XVec = EtVector3( 1.0f, 0.0f, 0.0f );
	EtVec3Cross( &ZVec, &XVec, &UpVec );
	EtVec3Normalize( &ZVec, &ZVec );
	EtVec3Cross( &XVec, &UpVec, &ZVec );
	EtVec3Normalize( &XVec, &XVec );
	EtMatrixTranslation( &LandMat, GrassPos.x, GrassPos.y, GrassPos.z );
	memcpy( &LandMat._11, &XVec, sizeof( EtVector3 ) );
	memcpy( &LandMat._21, &UpVec, sizeof( EtVector3 ) );
	memcpy( &LandMat._31, &ZVec, sizeof( EtVector3 ) );

	fGrassWidth = m_GrassBlockInfo.fGrassWidth[ cKind ] * 0.5f; 
	fGrassHeight = ( rand() / ( float )RAND_MAX ) * ( m_GrassBlockInfo.fMaxGrassHeight[ cKind ] - m_GrassBlockInfo.fMinGrassHeight[ cKind ] ) + 
		m_GrassBlockInfo.fMinGrassHeight[ cKind ];
	BasePos[ 0 ] = EtVector3( -fGrassWidth, 0.0f, 0.0f );
	BasePos[ 1 ] = EtVector3( fGrassWidth, 0.0f, 0.0f );
	BasePos[ 2 ] = EtVector3( fGrassWidth, fGrassHeight, 0.0f );
	BasePos[ 3 ] = EtVector3( -fGrassWidth, fGrassHeight, 0.0f );
	fRotateAngle = rand() / ( float )RAND_MAX * 360.0f;
	for( nLoop = 0; nLoop < 12; nLoop++ )
	{
		EtVector3 *pCurPos;

		if( nLoop % 4 == 0 )
		{
			EtMatrixRotationY( &RotateMat, EtToRadian( fRotateAngle ) );
			EtMatrixMultiply( &RotateMat, &RotateMat, &LandMat );
			fRotateAngle += 120.0f;
		}
		pCurPos = pPosition  + nLoop;
		EtVec3TransformCoord( pCurPos, BasePos + nLoop % 4, &RotateMat );

		if( pCurPos->x > m_BoundingBox.Max.x ) m_BoundingBox.Max.x = pCurPos->x;
		else if( pCurPos->x < m_BoundingBox.Min.x ) m_BoundingBox.Min.x = pCurPos->x;
		if( pCurPos->y > m_BoundingBox.Max.y ) m_BoundingBox.Max.y = pCurPos->y;
		else if( pCurPos->y < m_BoundingBox.Min.y ) m_BoundingBox.Min.y = pCurPos->y;
		if( pCurPos->z > m_BoundingBox.Max.z ) m_BoundingBox.Max.z = pCurPos->z;
		else if( pCurPos->z < m_BoundingBox.Min.z ) m_BoundingBox.Min.z = pCurPos->z;
	}

	fShake = ( rand() / ( float )RAND_MAX ) * ( m_GrassBlockInfo.fMaxShake - m_GrassBlockInfo.fMinShake ) + m_GrassBlockInfo.fMinShake;
	for( nLoop = 0; nLoop < 3; nLoop++ )
	{
		pTexCoord[ nLoop * 4 ] = EtVector2( cKind * 0.25f, 1.0f );
		pTexCoord[ nLoop * 4 + 1 ] = EtVector2( cKind * 0.25f + 0.25f, 1.0f );
		pTexCoord[ nLoop * 4 + 2 ] = EtVector2( cKind * 0.25f + 0.25f, 0.0f );
		pTexCoord[ nLoop * 4 + 3 ] = EtVector2( cKind * 0.25f, 0.0f );

		pShake[ nLoop * 4 ] = 0.0f;
		pShake[ nLoop * 4 + 1 ] = 0.0f;
		pShake[ nLoop * 4 + 2 ] = fShake;
		pShake[ nLoop * 4 + 3 ] = fShake;
	}

	pIndex[ 0 ] = nGrassIndex * 12;
	pIndex[ 1 ] = nGrassIndex * 12 + 1;
	pIndex[ 2 ] = nGrassIndex * 12 + 3;
	pIndex[ 3 ] = nGrassIndex * 12 + 1;
	pIndex[ 4 ] = nGrassIndex * 12 + 2;
	pIndex[ 5 ] = nGrassIndex * 12 + 3;

	pIndex[ 6 ] = nGrassIndex * 12 + 4;
	pIndex[ 7 ] = nGrassIndex * 12 + 5;
	pIndex[ 8 ] = nGrassIndex * 12 + 7;
	pIndex[ 9 ] = nGrassIndex * 12 + 5;
	pIndex[ 10 ] = nGrassIndex * 12 + 6;
	pIndex[ 11 ] = nGrassIndex * 12 + 7;

	pIndex[ 12 ] = nGrassIndex * 12 + 8;
	pIndex[ 13 ] = nGrassIndex * 12 + 9;
	pIndex[ 14 ] = nGrassIndex * 12 + 11;
	pIndex[ 15 ] = nGrassIndex * 12 + 9;
	pIndex[ 16 ] = nGrassIndex * 12 + 10;
	pIndex[ 17 ] = nGrassIndex * 12 + 11;
}

void CEtGrassBlock::CreateMaterial()
{
	m_hMaterial = LoadResource( "Grass.fx", RT_SHADER );
}

void CEtGrassBlock::SetTexture( const char *pFileName )
{
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	SAFE_RELEASE_SPTR( m_hTexture );

	int nTexIndex = -1;
	m_hTexture = LoadResource( pFileName, RT_TEXTURE );
	if( m_hTexture ) 
	{
		nTexIndex = m_hTexture->GetMyIndex();
	}
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_DiffuseTex", &nTexIndex );

	// Set CustomParam here together!!
	AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_InteractivePos", &m_InteractivePos );
}

const char *CEtGrassBlock::GetTextureName()
{
	if( !m_hTexture ) return NULL;
	return m_hTexture->GetFileName();
}

void CEtGrassBlock::Render()
{
	if( m_nGrassCount <= 0 )
	{
		return;
	}
	if( !m_hTexture )
	{
		return;
	}

	SRenderStackElement RenderElement;
	EtMatrix WorldMat;

	EtMatrixIdentity( &WorldMat );
	RenderElement.hMaterial = m_hMaterial;
	RenderElement.nTechniqueIndex = 0;
	RenderElement.WorldMat = WorldMat;
	RenderElement.PrevWorldMat = WorldMat;
	RenderElement.nSaveMatIndex = -1;
	RenderElement.pvecCustomParam = &m_vecCustomParam;
	RenderElement.pRenderMeshStream = &m_MeshStream;

	SStateBlock RenderState;

	RenderState.AddRenderState( D3DRS_CULLMODE, CULL_NONE );
	RenderElement.nStateBlockIndex = CEtStateBlockManager::GetInstance().CreateStateBlock(&RenderState);
	RenderElement.nDiffuseTexIndex = m_hTexture ? m_hTexture->GetMyIndex() : -1;
	RenderElement.nBakeDepthIndex = DT_GRASS;
	
	m_nRenderUniqueID = GetCurRenderStack()->AddNormalRenderElement( RenderElement, m_nRenderUniqueID );
}

void CEtGrassBlock::GetExtent( EtVector3 &Origin, EtVector3 &Extent )
{
	Origin = ( m_BoundingBox.Max + m_BoundingBox.Min ) / 2;
	Extent = ( m_BoundingBox.Max - m_BoundingBox.Min ) / 2;
}

void CEtGrassBlock::SetLightMapInfluence( char *pTexBuffer, int nTexStride, int nTexWidth, int nTexHeight, float fOffsetU, float fOffsetV )
{
	if( m_nGrassCount <= 0 )
	{
		return;
	}
	
	int i, j, nVertexCount;
	EtVector3 *pPosition;
	DWORD *pColor;
	EtColor Ambient;

	Ambient = *CEtLight::GetGlobalAmbient();
	Ambient.r *= 0.381f;
	Ambient.g *= 0.381f;
	Ambient.b *= 0.381f;
	nVertexCount = m_MeshStream.GetVertexCount();
	pPosition = new EtVector3[ nVertexCount ];
	pColor = new DWORD[ nVertexCount ];
	m_MeshStream.GetVertexStream( MST_POSITION, pPosition, 0 );
	for( i = 0; i < m_nGrassCount; i++ )
	{
		EtVector3 *pCurPosition, CenterPos;
		float fU, fV;
		int nTexOffset;
		DWORD dwGrassColor;

		pCurPosition = pPosition + i * 12;
		CenterPos = ( pCurPosition[ 0 ] + pCurPosition[ 1 ] ) * 0.5f - m_GrassBlockInfo.GrassOffset;
		fU = CenterPos.x / ( m_GrassBlockInfo.nSizeX * m_GrassBlockInfo.fTileSize ) * 0.5f + fOffsetU;
		fV = CenterPos.z / ( m_GrassBlockInfo.nSizeY * m_GrassBlockInfo.fTileSize ) * 0.5f + fOffsetV;
		nTexOffset = ( ( ( int )( fV * ( nTexHeight - 1 ) ) ) * nTexStride ) + ( ( int )( fU * ( nTexWidth - 1 ) ) ) * 4;
		if( pTexBuffer[ nTexOffset + 1 ] == 0 )
		{
			dwGrassColor = Ambient;
		}
		else
		{
			EtColor Color;

			Color = *( ( DWORD * )( pTexBuffer + nTexOffset ) );
			Color += Ambient;
			dwGrassColor = Color;
		}
		for( j = 0; j < 12; j++ )
		{
			pColor[ i * 12 + j ] = dwGrassColor;
		}
	}

	CMemoryStream Stream( pColor, nVertexCount * sizeof( DWORD ) );
	m_MeshStream.LoadVertexStream( &Stream, MST_COLOR, 0, nVertexCount );

	delete [] pPosition;
	delete [] pColor;
}

void CEtGrassBlock::SetInteractivePos( EtVector3 *vPos, float fDelta )
{
	EtVector2 vCurrVelocity = EtVector2(vPos->x - m_InteractivePos.x, vPos->z - m_InteractivePos.y);
	if( fDelta != 0.0f ) {
		vCurrVelocity /= fDelta;
	}
	m_InteractiveVelocity += (vCurrVelocity - m_InteractiveVelocity) * min( 1.0f, ( fDelta * 5.0f ));

	static float fVelocityScale = 0.01f;
	static float fMaxVelocity  = 5.f;

	m_InteractivePos.z = (float)(m_InteractiveVelocity.x * fVelocityScale + 0.001f );
	m_InteractivePos.w = (float)(m_InteractiveVelocity.y * fVelocityScale + 0.001f );

	float fVelLength = sqrtf( m_InteractivePos.z*m_InteractivePos.z+m_InteractivePos.w*m_InteractivePos.w);
	if( fVelLength > fMaxVelocity ) {
		m_InteractivePos.z /= fVelLength;
		m_InteractivePos.w /= fVelLength;
		m_InteractivePos.z *= fMaxVelocity;
		m_InteractivePos.w *= fMaxVelocity;
	}

	m_InteractivePos.x = vPos->x;
	m_InteractivePos.y = vPos->z;

}
