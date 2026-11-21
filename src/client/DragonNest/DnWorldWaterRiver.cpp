#include "StdAfx.h"
#include "DnWorldWaterRiver.h"
#include "DnWorldWater.h"
#include "DnWorldSector.h"
#include "EtWater.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldWaterRiver::CDnWorldWaterRiver( CEtWorldWater *pWater )
: CEtWorldWaterRiver( pWater )
{
	m_pMesh = NULL;
}

CDnWorldWaterRiver::~CDnWorldWaterRiver()
{
	SAFE_DELETE( m_pMesh );
	SAFE_RELEASE_SPTR( m_hAlphaTexture );
	SAFE_RELEASE_SPTR( m_hMaterial );
	std::map<std::string, EtTextureHandle>::iterator it;
	for( it = m_MapUsingTexture.begin(); it!=m_MapUsingTexture.end(); it++ ) {
		SAFE_RELEASE_SPTR( it->second );
	}
	m_MapUsingTexture.clear();
}

bool CDnWorldWaterRiver::Load( CStream *pStream )
{
	bool bResult = CEtWorldWaterRiver::Load( pStream );
	if( !bResult ) return false;

	CreateRiver();

	// Well, since it is not needed after creation, let's make sure it gets deleted.
	SAFE_DELETEA( m_pAlphaTable );
	SAFE_DELETE_PVEC( m_pVecCustomParamList );
	return bResult;
}

void CDnWorldWaterRiver::Render( LOCAL_TIME LocalTime )
{
	SRenderStackElement RenderElement;
	EtMatrix WorldMat;

	EtMatrixIdentity( &WorldMat );
	RenderElement.hMaterial = m_hMaterial;
	RenderElement.nTechniqueIndex = 0;
	RenderElement.WorldMat = WorldMat;
	RenderElement.PrevWorldMat = WorldMat;
	RenderElement.nSaveMatIndex = -1;
	RenderElement.pvecCustomParam = &m_VecCustomParam;
	RenderElement.pRenderMeshStream = m_pMesh;
	RenderElement.renderPriority = RP_HIGH;
	RenderElement.nBakeDepthIndex = DT_OPAQUE;

	/*SRenderStateBlock RenderStateBlock;
	RenderStateBlock.AddRenderState( D3DRS_ZWRITEENABLE, FALSE);
	RenderStateBlock.AddRenderState( D3DRS_ALPHABLENDENABLE, TRUE);
	RenderStateBlock.AddRenderState( D3DRS_ALPHAREF, 0x1 );
	RenderElement.nStateBlockIndex = CEtStateBlockManager::GetInstance().CreateStateBlock( &RenderStateBlock );*/

	GetCurRenderStack()->AddAlphaRenderElement( RenderElement );
}

void CDnWorldWaterRiver::CreateRiver()
{
	m_pMesh = new CEtMeshStream;
	GenerationVertex();
	GenerationIndex();
	GenerationNormal();
	GenerationTexCoord();
	GenerationShader();
	GenerationTexture();
}

void CDnWorldWaterRiver::GenerationVertex()
{
	if( !m_pMesh ) return;

	int nSize = (int)m_vVecPointList.size() * 2;
	EtVector3 *pVertex = new EtVector3[ nSize ];
	/*
	EtVector3 vDir, vCross;
	for( DWORD i=0; i<m_vVecPointList.size(); i++ ) {
		if( i == m_vVecPointList.size() - 1 ) vDir = m_vVecPointList[i] - m_vVecPointList[i-1];
		else vDir = m_vVecPointList[i+1] - m_vVecPointList[i];

		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		pVertex[ i*2 ] = m_vVecPointList[i] + ( vCross * ( m_fWidth / 2.f ) );
		pVertex[ (i*2) + 1 ] = m_vVecPointList[i] - ( vCross * ( m_fWidth / 2.f ) );
	}
	*/
	EtVector3 vOffset = GetSectorOffset();
	for( DWORD i=0; i<m_vVecPrimitiveList.size(); i++ )
		pVertex[i] = m_vVecPrimitiveList[i] + vOffset;

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_POSITION, 0, nSize );
	SAFE_DELETEA( pVertex );
}

void CDnWorldWaterRiver::GenerationIndex()
{
	if( !m_pMesh ) return;

	int nSize = ( (int)m_vVecPointList.size() - 1 ) * 6;
	WORD *pIndex = new WORD[ nSize ];

	int nOffset;
	for( int i=0; i<(int)m_vVecPointList.size()-1; i++ ) {
		nOffset = (i*6);
		pIndex[nOffset] = (i*2);
		pIndex[nOffset+1] = ((i+1)*2);
		pIndex[nOffset+2] = (i*2)+1;
		pIndex[nOffset+3] = (i*2)+1;
		pIndex[nOffset+4] = ((i+1)*2);
		pIndex[nOffset+5] = ((i+1)*2)+1;
	}

	CMemoryStream Stream( pIndex, sizeof(WORD) * nSize );
	m_pMesh->LoadIndexStream( &Stream, false, nSize );

	SAFE_DELETEA( pIndex );
}

void CDnWorldWaterRiver::GenerationNormal()
{
	if( !m_pMesh ) return;

	int nSize = (int)m_vVecPointList.size() * 2;
	EtVector3 *pVertex = new EtVector3[ nSize ];
	/*
	EtVector3 vDir, vCross;
	for( DWORD i=0; i<m_vVecPointList.size(); i++ ) {
		if( i == m_vVecPointList.size() - 1 ) vDir = m_vVecPointList[i] - m_vVecPointList[i-1];
		else vDir = m_vVecPointList[i+1] - m_vVecPointList[i];

		EtVec3Normalize( &vDir, &vDir );
		EtVec3Cross( &vCross, &vDir, &EtVector3( 0.f, 1.f, 0.f ) );

		pVertex[ i*2 ] = EtVector3( 0.f, 1.f, 0.f );
		pVertex[ (i*2) + 1 ] = EtVector3( 0.f, 1.f, 0.f );
	}
	*/
	for( int i=0; i<nSize; i++ )
		pVertex[i] = EtVector3( 0.f, 1.f, 0.f );

	CMemoryStream Stream( pVertex, sizeof(EtVector3) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_NORMAL, 0, nSize );
	SAFE_DELETEA( pVertex );
}

void CDnWorldWaterRiver::GenerationTexCoord()
{
	if( !m_pMesh ) return;

	float fWidth = (float)GetWater()->GetSector()->GetParentGrid()->GetGridWidth() * 100.f;
	float fHeight = (float)GetWater()->GetSector()->GetParentGrid()->GetGridHeight() * 100.f;

	int nSize = (int)m_vVecPointList.size() * 2;
	EtVector2 *pUV = new EtVector2[ nSize ];

	EtVector3 vVertex;

	for( DWORD i=0; i<m_vVecPrimitiveList.size(); i++ ) {
		vVertex = m_vVecPrimitiveList[i];
		pUV[i].x = ( m_vUVRatio.x / fWidth ) * vVertex.x;
		pUV[i].y = ( m_vUVRatio.y / fHeight ) * vVertex.z;
	}

	CMemoryStream Stream( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 0, nSize );

	for( DWORD i=0; i<m_vVecPrimitiveList.size(); i++ ) {
		vVertex = m_vVecPrimitiveList[i];
		pUV[i].x = ( 1.f / m_vSize.x ) * ( vVertex.x - m_vMin.x );
		pUV[i].y = ( 1.f / m_vSize.y ) * ( vVertex.z - m_vMin.y );
	}

	Stream.Initialize( pUV, sizeof(EtVector2) * nSize );
	m_pMesh->LoadVertexStream( &Stream, MST_TEXCOORD, 1, nSize );


	SAFE_DELETEA( pUV );
}

void CDnWorldWaterRiver::GenerationShader()
{
	m_hMaterial = LoadResource( m_szShaderName.c_str(), RT_SHADER );
	if( !m_hMaterial ) return;

	for( DWORD i=0; i<m_pVecCustomParamList.size(); i++ ) {
		CEtWorldWater::CustomParamStruct *pStruct = m_pVecCustomParamList[i];
		switch( pStruct->nType ) {
			case EPT_INT: 
				AddCustomParam( m_VecCustomParam, (EffectParamType)pStruct->nType, m_hMaterial, pStruct->szName.c_str(), &pStruct->nInt );
				break;
			case EPT_FLOAT:
				AddCustomParam( m_VecCustomParam, (EffectParamType)pStruct->nType, m_hMaterial, pStruct->szName.c_str(), &pStruct->fFloat );
				break;
			case EPT_VECTOR:
				AddCustomParam( m_VecCustomParam, (EffectParamType)pStruct->nType, m_hMaterial, pStruct->szName.c_str(), &pStruct->fFloat4 );
				break;
			case EPT_TEX:
				{
					std::map<std::string, EtTextureHandle>::iterator it = m_MapUsingTexture.find(m_pVecCustomParamList[i]->szName);
					if( it != m_MapUsingTexture.end() ) {
						SAFE_RELEASE_SPTR( it->second );
						m_MapUsingTexture.erase( it );
					}
					EtTextureHandle hTexture = LoadResource( pStruct->szTexName, RT_TEXTURE );
					int nTexIndex = hTexture.GetIndex();
					m_MapUsingTexture.insert( make_pair( pStruct->szName, hTexture ) );

					AddCustomParam( m_VecCustomParam, (EffectParamType)pStruct->nType, m_hMaterial, pStruct->szName.c_str(), &nTexIndex );
				}
				break;
		}
	}

	AddCustomParam( m_VecCustomParam, EPT_MATRIX_PTR, m_hMaterial, "g_LastViewMat", GetEtWater()->GetWaterBakeViewMat() );
}


void CDnWorldWaterRiver::GenerationTexture()
{
	m_hAlphaTexture = CEtTexture::CreateNormalTexture( m_nAlphaTableWidth, m_nAlphaTableHeight, FMT_A8R8G8B8, USAGE_DEFAULT, POOL_MANAGED );
	if( m_hAlphaTexture ) {
		int nTexIndex = m_hAlphaTexture.GetIndex();
		AddCustomParam( m_VecCustomParam, EPT_TEX, m_hMaterial, "g_TransparencyTex", &nTexIndex );
	}

	int nStride;
	float fValue;
	DWORD *pPtr = (DWORD*)m_hAlphaTexture->Lock( nStride );

	for( int j=0; j<m_nAlphaTableHeight; j++ ) {
		for( int i=0; i<m_nAlphaTableWidth; i++ ) {
			fValue = m_pAlphaTable[j*m_nAlphaTableWidth+i] * m_fRevisionAlpha;
			if( fValue < 0.f ) fValue = 0.f;
			if( fValue > 255.f ) fValue = 255.f;
			*pPtr = D3DCOLOR_ARGB( (BYTE)fValue, 0, 0, 0 );
			pPtr++;
		}
	}

	m_hAlphaTexture->Unlock();
}


EtVector3 CDnWorldWaterRiver::GetSectorOffset()
{
	EtVector3 vOffset = *GetWater()->GetSector()->GetOffset();
	vOffset.x -= ( GetWater()->GetSector()->GetTileWidthCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;
	vOffset.z -= ( GetWater()->GetSector()->GetTileHeightCount() * GetWater()->GetSector()->GetTileSize() ) / 2.f;

	return vOffset;
}


void CDnWorldWaterRiver::RefreshWaterInfo()
{
	int nIndex = ((CDnWorldWater*)m_pWater)->GetWaterIndex( this );
	int nTexIndex = -1;
	if( nIndex != -1 ) {
		if( !m_vVecPointList.empty() )
			GetEtWater()->SetWaterHeight( nIndex, m_vVecPointList[0].y );
		GetEtWater()->SetWaterIgnoreBake( nIndex, m_InterpolationType == CEtWorldWaterRiver::CatmullRom );

		EtTextureHandle hTexture = GetEtWater()->GetWaterMapTexture( nIndex );
		nTexIndex = hTexture.GetIndex();
	}
	AddCustomParam( m_VecCustomParam, EPT_TEX, m_hMaterial, "g_WaterMapTex", &nTexIndex );

}