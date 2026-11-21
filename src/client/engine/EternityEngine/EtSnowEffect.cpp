#include "StdAfx.h"
#include "EtSnowEffect.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtSnowEffect::CEtSnowEffect(void)
{
	m_fSnowTimer = 0.f;
	m_bUsePointSprite = false;	// Point sprites can no longer express texture animation using volume textures, so we decided not to use them anymore.
	m_nSnowCount = 1500;
	m_nSnowTimeOut = 7000;
	m_fSnowHeight = 1500.f;
	m_isUp = false;

	m_fSnowSize = 0.0f;
	m_fSnowAniSpeed = 1.0f;
}

CEtSnowEffect::~CEtSnowEffect(void)
{
	SAFE_DELETE_PVEC( m_MeshStreamList );
}

void CEtSnowEffect::Initialize( const char *pSnowTexture )
{
	int i, j, nVertexCount;
	EtVector3 *pvPosition, *pvNormal;
	float *pPointSize;

	nVertexCount = m_nSnowCount;
	m_hTexture = LoadResource( pSnowTexture, RT_TEXTURE );

	if( m_bUsePointSprite ) {

		pvPosition = new EtVector3[ nVertexCount ];
		pvNormal = new EtVector3[ nVertexCount ];
		pPointSize = new float[ nVertexCount ];
		
		EtTerrainHandle Terrain = CEtTerrainArea::GetItem( 0 );
		int nCountX, nCountY;
		Terrain->GetBlockCount(nCountX, nCountY);
		for( j = 0; j < nCountX*nCountY; j++)
		{
			SAABox Box = *Terrain->GetBoundingBox( j );
			for( i = 0; i < nVertexCount; i ++ )
			{
				pvPosition[ i ] = EtVector3( Box.Min.x + (Box.Max.x-Box.Min.x) * (rand() / (float)RAND_MAX),
														0.f,
													 Box.Min.z + (Box.Max.z-Box.Min.z) * (rand() / (float)RAND_MAX));
				if (m_isUp)
					pvPosition[i].y = Terrain->GetLandHeight( pvPosition[i].x, pvPosition[i].z ) - 5.f;
				else
					pvPosition[i].y = Terrain->GetLandHeight( pvPosition[i].x, pvPosition[i].z ) + 5.f;

				pvNormal[i].x = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
				pvNormal[i].y = 1.f;
				pvNormal[i].z = -1.0f + 2.0f * (rand() / (float)RAND_MAX);
				EtVec3Normalize( &pvNormal[i], &pvNormal[i] );
				pvNormal[i] *= 1.0f + 0.3f * (rand() / (float)RAND_MAX);

				pPointSize[i] = 8.f + 5.f * (rand() / (float)RAND_MAX);
			}
			CMemoryStream Stream;
			CEtMeshStream *MeshStream = new CEtMeshStream;

			Stream.Initialize( pvPosition, nVertexCount * sizeof( EtVector3 ) );
			MeshStream->LoadVertexStream( &Stream, MST_POSITION, 0, nVertexCount );
			Stream.Initialize( pvNormal, nVertexCount * sizeof( EtVector3 ) );
			MeshStream->LoadVertexStream( &Stream, MST_NORMAL, 0, nVertexCount );
			Stream.Initialize( pPointSize, nVertexCount * sizeof( float ) );
			MeshStream->LoadVertexStream( &Stream, MST_PSIZE, 0, nVertexCount );
			if( m_hTexture ) {
				MeshStream->SetPointSpriteTexture( m_hTexture->GetTexturePtr() );	
			}
			m_MeshStreamList.push_back( MeshStream );
		}
		delete [] pvPosition;
		delete [] pvNormal;
		delete [] pPointSize;
	}
	else {
		
		pvPosition = new EtVector3[ nVertexCount * 4 ];
		pvNormal = new EtVector3[ nVertexCount * 4 ];
		EtVector2 *pTexCoord = new EtVector2[ nVertexCount * 4];
		float *pRadomVector = new float[nVertexCount * 4];	//For random start value of texture animation
		WORD *pwIndex = new WORD[ nVertexCount * 6];

		EtTerrainHandle Terrain = CEtTerrainArea::GetItem( 0 );
		int nCountX, nCountY;
		Terrain->GetBlockCount(nCountX, nCountY);
		for( j = 0; j < nCountX*nCountY; j++)
		{
			SAABox Box = *Terrain->GetBoundingBox( j );
			for( i = 0; i < nVertexCount; i ++ )
			{
				pvPosition[ i*4 ] = EtVector3( Box.Min.x + (Box.Max.x-Box.Min.x) * (rand() % 4097 ) / 4097.f,
														0.f,
														Box.Min.z + (Box.Max.z-Box.Min.z) * (rand() % 4097 ) / 4097.f);
				
				if (m_isUp)
					pvPosition[i*4].y = Terrain->GetLandHeight( pvPosition[i*4].x, pvPosition[i*4].z ) - 5.f;
				else
					pvPosition[i*4].y = Terrain->GetLandHeight( pvPosition[i*4].x, pvPosition[i*4].z ) + 5.f;

				pvNormal[i*4].x = -1.0f + 2.0f * (rand() % 255 ) / 255.f;
				pvNormal[i*4].y = 1.f;
				pvNormal[i*4].z = -1.0f + 2.0f * (rand() % 255 ) / 255.f;
				EtVec3Normalize( &pvNormal[i*4], &pvNormal[i*4] );

				pvPosition[ i*4+1 ] = pvPosition[ i*4];
				pvPosition[ i*4+2 ] = pvPosition[ i*4];
				pvPosition[ i*4+3 ] = pvPosition[ i*4];

				pvNormal[ i*4+1 ] = pvNormal[ i*4];
				pvNormal[ i*4+2 ] = pvNormal[ i*4];
				pvNormal[ i*4+3 ] = pvNormal[ i*4];

				pTexCoord[ i*4+0 ] = EtVector2(0, 1);
				pTexCoord[ i*4+1 ] = EtVector2(0, 0);
				pTexCoord[ i*4+2 ] = EtVector2(1, 1);
				pTexCoord[ i*4+3 ] = EtVector2(1, 0);

				float fRandom = ((float)(rand() % 100));// * 0.01f;
				pRadomVector[ i*4+0 ] = fRandom;
				pRadomVector[ i*4+1 ] = pRadomVector[ i*4+0 ];
				pRadomVector[ i*4+2 ] = pRadomVector[ i*4+0 ];
				pRadomVector[ i*4+3 ] = pRadomVector[ i*4+0 ];
				
				pwIndex[ i*6+0 ] = i*4+0;
				pwIndex[ i*6+1 ] = i*4+1;
				pwIndex[ i*6+2 ] = i*4+2;
				pwIndex[ i*6+3 ] = i*4+2;
				pwIndex[ i*6+4 ] = i*4+1;
				pwIndex[ i*6+5 ] = i*4+3;

			}
			CMemoryStream Stream;
			CEtMeshStream *MeshStream = new CEtMeshStream;

			Stream.Initialize( pvPosition, nVertexCount * 4 * sizeof( EtVector3 ) );
			MeshStream->LoadVertexStream( &Stream, MST_POSITION, 0, nVertexCount*4 );
			Stream.Initialize( pvNormal, nVertexCount * 4 * sizeof( EtVector3 ) );
			MeshStream->LoadVertexStream( &Stream, MST_NORMAL, 0, nVertexCount*4 );
			Stream.Initialize( pTexCoord, nVertexCount * 4 * sizeof( EtVector2 ) );
			MeshStream->LoadVertexStream( &Stream, MST_TEXCOORD, 0, nVertexCount*4 );

			Stream.Initialize( pRadomVector, nVertexCount * 4 * sizeof( float ) );
			MeshStream->LoadVertexStream( &Stream, MST_PSIZE, 0, nVertexCount*4 );

			Stream.Initialize( pwIndex, nVertexCount * 6 * sizeof( WORD ) );
			MeshStream->LoadIndexStream( &Stream, false, nVertexCount*6 );

			m_MeshStreamList.push_back( MeshStream );
		}
		delete [] pvPosition;
		delete [] pvNormal;
		delete [] pTexCoord;
		delete [] pRadomVector;
		delete [] pwIndex;
	}

	m_hMaterial = LoadResource( "Snowfall.fx", RT_SHADER );
	AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fSnowHeight", &m_fSnowHeight);
	AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fSnowTimer", &m_fSnowTimer);
	AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_CamPos", &m_CamPos);
	if( !m_bUsePointSprite ) {
		AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_CamXVector", &m_CamXVector);
		AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_CamYVector", &m_CamYVector);
		AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fSnowSize", &m_fSnowSize);
		AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fAniSpeed", &m_fSnowAniSpeed);
		if( m_hTexture ) {
			int nTexIndex = m_hTexture->GetMyIndex();
			//AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_SnowTex", &nTexIndex );
			// On GTX 460 GPUs, texture types are strictly checked, so texture2D vs texture3D must be distinguished.
			if( m_hTexture->GetTextureType() == ETTEXTURE_NORMAL )
				AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_SnowTex", &nTexIndex );
			else
				AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_SnowTex3D", &nTexIndex );
		}
	}
}


void CEtSnowEffect::Render( float fElapsedTime )
{
	if( !m_hTexture ) {
		return;
	}
	
	if (m_isUp)
	{
		m_fSnowTimer += fElapsedTime*1000.f/m_nSnowTimeOut;
		while( m_fSnowTimer > 1 ) m_fSnowTimer-=1.f;
	}
	else
	{
		m_fSnowTimer -= fElapsedTime*1000.f/m_nSnowTimeOut;
		while( m_fSnowTimer < 0 ) m_fSnowTimer+=1.f;
	}

	EtMatrix WorldMat;
	EtMatrixIdentity( &WorldMat );

	SRenderStackElement RenderElement;

	RenderElement.hMaterial = m_hMaterial;
	RenderElement.nTechniqueIndex = m_bUsePointSprite ? 1 : 0;
	RenderElement.WorldMat = WorldMat;
	RenderElement.nSaveMatIndex = -1;
	RenderElement.pvecCustomParam = &m_vecCustomParam;
	RenderElement.nBakeDepthIndex = DT_NONE;

	// Select technique depending on texture2D or texture3D.
	if( !m_bUsePointSprite ) {
		if( m_hTexture && m_hTexture->GetTextureType() == ETTEXTURE_NORMAL )
			RenderElement.nTechniqueIndex = 2;
	}
	
	EtMatrix *CamMat = CEtCamera::GetActiveCamera()->GetInvViewMat();

	m_CamXVector = *(EtVector3*)&CamMat->_11;
	m_CamYVector = *(EtVector3*)&CamMat->_21;

	m_CamPos = *(EtVector3*)&CamMat->_41;
	m_CamPos.y = 0.f;

	const int POS_COUNT = 7;
	EtVector3 camPos[ POS_COUNT ];
	camPos[0] = *(EtVector3*)&CamMat->_41;
	camPos[1] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 4000.f;
	camPos[2] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 2000.f;
	camPos[3] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 3000.f + (*(EtVector3*)&CamMat->_11) * 1500.f;
	camPos[4] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 3000.f - (*(EtVector3*)&CamMat->_11) * 1500.f;
	camPos[5] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 2000.f + (*(EtVector3*)&CamMat->_11) * 1000.f;
	camPos[6] = camPos[0] + (*(EtVector3*)&CamMat->_31) * 2000.f - (*(EtVector3*)&CamMat->_11) * 1000.f;
	EtTerrainHandle Terrain = CEtTerrainArea::GetItem( 0 );
	int i, j, nCountX, nCountY;
	Terrain->GetBlockCount(nCountX, nCountY);
	for( j = 0; j < nCountX*nCountY; j++)
	{
		SAABox Box = *Terrain->GetBoundingBox( j );
		for( i = 0; i < POS_COUNT; i++) {
			if( Box.IsInside( EtVector3( camPos[i].x, (Box.Max.y+Box.Min.y)*0.5f , camPos[i].z ))) {
				break;
			}
		}
		if( i != POS_COUNT ) {
			RenderElement.pRenderMeshStream = m_MeshStreamList[ j ];
			GetCurRenderStack()->AddAlphaRenderElement( RenderElement );
		}
	}
}

EtSnowHandle CEtSnowEffect::CreateSnowEffect( const char *pSnowTexture, int nSnowCount, float fSnowSize )
{
	CEtSnowEffect *pSnowEffect;

	pSnowEffect = new CEtSnowEffect();
	pSnowEffect->SetSnowCount( nSnowCount );
	pSnowEffect->SetSnowSize(fSnowSize);
	pSnowEffect->Initialize( pSnowTexture );	

	return pSnowEffect->GetMySmartPtr();
}
