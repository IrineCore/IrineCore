#include "StdAfx.h"
#include "EtRenderStack.h"
#include "EtShadowMap.h"
#include "EtMRTMng.h"
#include "EtLight.h"
#include "EtBackBufferMng.h"
#include "EtLensFlare.h"
#include "EtCustomRender.h"
#include "EtOptionController.h"
#include "EtWater.h"
#include "D3DDevice9/EtStateManager.h"
#include "D3DDevice9/EtCallBack.h"
#include "StringUtil.h"
#include "EtSkyBoxRTT.h"
#include "EtEngine.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define _USE_INSTANCING			1

extern float g_fTotalElapsedTime;

int GetDepthTechnique( int nTechniqueIndex, int nBakeDepthIndex )
{
	int nDepthIndex = nBakeDepthIndex;
	int nResult = nDepthIndex + ( nTechniqueIndex & 1 );
	return nResult;
}

void CEtRenderStack::RadixSort( std::vector< SRenderStackElement* > &vecSortNeed )
{
	int nBufferIndex = 0;
	for( std::vector< SRenderStackElement* >::iterator it = vecSortNeed.begin(); it != vecSortNeed.end(); ++it) {
		SRenderStackElement *pElem = *it;
		float fDistVal = EtMax(pElem->fDist, 0);
		int nIndex = ((int)(fDistVal)) % 10;
		m_vecRadixSort[ nBufferIndex ][ nIndex ].push_back( pElem );
	}

	float fDivide = 10.0f;
	bool bContinue = false;
	do {		
		bContinue = false;		
		for( int i = 0; i < 10; i++) 
		{
			int nSize = (int)m_vecRadixSort[ nBufferIndex ][ i ].size();
			for( int j = 0; j < nSize; j++) {

				SRenderStackElement *pElem = m_vecRadixSort[ nBufferIndex ][ i ][ j ];

				float fDistVal = EtMax(pElem->fDist , 0.f);
				int nIndex = ((int)(fDistVal / fDivide )) % 10;
				if( nIndex != 0) bContinue = true;

				assert( 1 - nBufferIndex < 2 && nIndex < 10 );
				m_vecRadixSort[ 1 - nBufferIndex ][ nIndex ].push_back( pElem );			
			}
			m_vecRadixSort[ nBufferIndex ][ i ].clear();
		}
		fDivide *= 10;
		nBufferIndex = 1 - nBufferIndex;
	}while( bContinue );


	vecSortNeed.clear();

	for( int i = 0; i < 10; i++) 
	{
		int nSize = (int)m_vecRadixSort[ nBufferIndex ][ i ].size();
		for( int j = 0; j < nSize; j++) {
			vecSortNeed.push_back( m_vecRadixSort[ nBufferIndex ][ i ][ j ] );
		}
		m_vecRadixSort[ nBufferIndex ][ i ].clear();
		ASSERT( m_vecRadixSort[ 1 - nBufferIndex ][ i ].empty() );
	}
}

bool NormalRenderSort( SRenderStackElement *Element1, SRenderStackElement *Element2 )
{
	if( Element1->renderPriority != Element2->renderPriority ) 
	{
		if( Element1->renderPriority < Element2->renderPriority )
			return true;
		else
			return false;
	}
	else
	{
		if( Element1->fDist < Element2->fDist )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

bool AlphaRenderSort( const SRenderStackElement *Element1, const SRenderStackElement *Element2 )
{
	if( Element1->renderPriority != Element2->renderPriority ) 
	{
		if( Element1->renderPriority < Element2->renderPriority )
			return true;
		else
			return false;
	}
	else
	{
		if( Element1->fDist > Element2->fDist )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

CEtRenderStack *g_pRenderStack[ 2 ];
bool CEtRenderStack::s_bUseOcclusion = false;
bool CEtRenderStack::s_bRenderDepth = false;

CEtRenderStack::CEtRenderStack(void)
{
	m_fComputeAniTime = 0.0f;
	m_nCurentUniqueID = 0;
	m_nCurOcclusionMngIndex = 0;
	m_nInteractivePosIndex = -1;
	m_nDiffuseTexParamIndex = -1;
	m_nVolumeTexParamIndex = -1;
	m_vInteractivePos = EtVector4( 0.0f, 0.0f, 0.0f, 0.0f );
}

CEtRenderStack::~CEtRenderStack(void)
{
	SAFE_RELEASE_SPTR( m_hBakeDepthMaterial );
	SAFE_DELETE_PVEC( m_RenderBlockPoolFree );
	SAFE_DELETE_PVEC( m_RenderBlockPoolUsed );
}

SRenderStackElement* CEtRenderStack::AllocBlock( SRenderStackElement &RenderElement ) 
{
	SRenderStackElement *pBlock = NULL;
	if( m_RenderBlockPoolFree.empty() ) {
		pBlock = new SRenderStackElement;		
	}
	else {
		pBlock = m_RenderBlockPoolFree.back();
		m_RenderBlockPoolFree.pop_back();
	}
	*pBlock = RenderElement;
	m_RenderBlockPoolUsed.push_back( pBlock );
	return pBlock;
}

void CEtRenderStack::RefreshAllocBlock()
{
	m_RenderBlockPoolFree.insert( m_RenderBlockPoolFree.end(),  m_RenderBlockPoolUsed.begin(), m_RenderBlockPoolUsed.end() );
	m_RenderBlockPoolUsed.clear();
}

void CEtRenderStack::ClassifyAlphaDepth()
{
	for( std::vector< SRenderStackElement* >::iterator it = m_vecRenderElement[ PASS_ALPHA ].begin(); it != m_vecRenderElement[ PASS_ALPHA ].end();  )
	{
		if( ( *it )->nBakeDepthIndex == DT_NONE )
		{
			++it;
		}
		else
		{
			m_vecRenderElement[ PASS_ALPHADEPTH ].push_back( *it );	
			it = m_vecRenderElement[ PASS_ALPHA ].erase( it );
		}
	}
}

void CEtRenderStack::RenderDepthBlock( PassType passType, bool bAutoDeleteList )
{
	if( !s_bRenderDepth )
	{
		if( bAutoDeleteList )
		{
			m_vecRenderElement[ passType ].clear();
		}
		return;
	}

	if( m_vecRenderElement[ passType ].empty() )
	{
		return;
	}
	if( !m_hBakeDepthMaterial )
	{
		int nTexIndex = -1;
		m_vInteractivePos = EtVector4( 0.0f, 0.0f, 0.0f, 0.0f );
		m_hBakeDepthMaterial = ::LoadResource( "BakeDepth.fx", RT_SHADER );
		m_nDiffuseTexParamIndex = ::AddCustomParam( m_vecBakeDepthParam, EPT_TEX, m_hBakeDepthMaterial, "g_DiffuseTex", &nTexIndex );
		m_nVolumeTexParamIndex = ::AddCustomParam( m_vecBakeDepthParam, EPT_TEX, m_hBakeDepthMaterial, "g_DiffuseVolumeTex", &nTexIndex );
		::AddCustomParam( m_vecBakeDepthParam, EPT_VECTOR_PTR, m_hBakeDepthMaterial, "g_InteractivePos", &m_vInteractivePos );
		::AddCustomParam( m_vecBakeDepthParam, EPT_FLOAT_PTR, m_hBakeDepthMaterial, "g_ComputeAniTimeForDepth", &m_fComputeAniTime );
	}

	CEtMRTMng::GetInstance().BeginDepthTarget();

	CEtMaterial *pBakeDepthMaterial = m_hBakeDepthMaterial.GetPointer();
	int nPasses = 0;
	std::vector< SRenderStackElement* >::iterator it;
	for( it = m_vecRenderElement[ passType ].begin(); it != m_vecRenderElement[ passType ].end();  )
	{
		SRenderStackElement *pCurBlock = *it;
		if( pCurBlock->nBakeDepthIndex == DT_NONE )
		{
			++it;
			continue;
		}

		int nDepthTechnique = GetDepthTechnique( pCurBlock->nTechniqueIndex, pCurBlock->nBakeDepthIndex );
		if( pCurBlock->nBakeDepthIndex == DT_TERRAIN )
		{
			nDepthTechnique = DT_OPAQUE;
		}

		switch( nDepthTechnique ) 
		{		
			case DT_GRASS:
				{
					if( m_nInteractivePosIndex == -1 )
					{
						EtParameterHandle hFindHandle = pCurBlock->hMaterial->GetParameterByName( "g_InteractivePos" );
						int i;
						std::vector< SCustomParam > &vecCustomParam = *pCurBlock->pvecCustomParam;
						for( i = 0; i < ( int )vecCustomParam.size(); i++ )
						{
							if( vecCustomParam[ i ].hParamHandle == hFindHandle )
							{
								m_nInteractivePosIndex = i;
								break;
							}
						}
					}
					if( ( m_nInteractivePosIndex >= 0 ) && ( m_nInteractivePosIndex < ( int )pCurBlock->pvecCustomParam->size() ) )
					{
						memcpy( &m_vInteractivePos, ( *pCurBlock->pvecCustomParam )[ m_nInteractivePosIndex ].pPointer, sizeof( EtVector4 ) );
					}
				}
				break;
			case DT_VOLUME:
			case DT_VOLUME + 1:
				{
					if( m_nVolumeTexParamIndex != -1 )
					{
						m_vecBakeDepthParam[ m_nVolumeTexParamIndex ].nTextureIndex = pCurBlock->nDiffuseTexIndex;
					}
					if( pCurBlock->pvecCustomParam )
					{
						int i;
						std::vector< SCustomParam > &vecCustomParam = *pCurBlock->pvecCustomParam;
						for( i = 0; i < ( int )vecCustomParam.size(); i++ )
						{
							if( ( vecCustomParam[ i ].nVariableCount == 1 ) && ( vecCustomParam[ i ].Type == EPT_FLOAT ) )
							{
								if( pCurBlock->hMaterial->IsParameterName( vecCustomParam[ i ].hParamHandle , "g_AniTime" ) )
								{
									m_fComputeAniTime = fmodf( g_fTotalElapsedTime, vecCustomParam[ i ].fFloat ) / vecCustomParam[ i ].fFloat;
									break;
								}
							}
						}
					}
				}
				break;
		}
		++it;

		pBakeDepthMaterial->SetTechnique( nDepthTechnique );
		pBakeDepthMaterial->BeginEffect( nPasses );
		pBakeDepthMaterial->BeginPass( 0 );
		pBakeDepthMaterial->SetGlobalParams();

		if( pCurBlock->nSaveMatIndex != -1 )
		{
			if( ( pCurBlock->pRenderSubMesh ) && ( pCurBlock->pRenderSubMesh->GetLinkCount() ) )
			{
				pBakeDepthMaterial->SetWorldMatArray( &pCurBlock->WorldMat, pCurBlock->nSaveMatIndex, 
					pCurBlock->pRenderSubMesh->GetLinkCount(), pCurBlock->pRenderSubMesh->GetLinkIndex() );
			}
		}
		pBakeDepthMaterial->SetWorldMatParams( &pCurBlock->WorldMat, &pCurBlock->PrevWorldMat );
		if( m_nDiffuseTexParamIndex != -1 )
		{
			m_vecBakeDepthParam[ m_nDiffuseTexParamIndex ].nTextureIndex = pCurBlock->nDiffuseTexIndex;	
		}
		pBakeDepthMaterial->SetCustomParamList( m_vecBakeDepthParam );

		// Set RenderState, SamplerState
		bool bZWrite = true;
		if( pCurBlock->nStateBlockIndex != -1 )
		{
			const SStateBlock *pStateBlock = GetEtStateManager()->GetStateBlock( pCurBlock->nStateBlockIndex );
			pStateBlock->SetState();
			bZWrite = GetEtDevice()->EnableZWrite( true );
		}		

		pBakeDepthMaterial->CommitChanges();

		int nOcclusionIndex;
		if( ( s_bUseOcclusion ) && ( passType == PASS_OPAQUE ) )
		{			
			nOcclusionIndex = m_OcclusionMng[ m_nCurOcclusionMngIndex ].AllocOcclusion();
			m_mapOcclusionCheck[ m_nCurOcclusionMngIndex ].insert( std::make_pair( pCurBlock->nRenderUniqueID, nOcclusionIndex ) );
			m_OcclusionMng[ m_nCurOcclusionMngIndex ].BeginOcclusion( nOcclusionIndex );
		}

		int nVertexDeclIndex = pBakeDepthMaterial->GetVertexDeclIndex( nDepthTechnique, 0 );
		if( nVertexDeclIndex != -1 )
		{
			if( pCurBlock->pRenderSubMesh )
			{
				pCurBlock->pRenderSubMesh->Draw( nVertexDeclIndex );
			}
			else
			{
				pCurBlock->pRenderMeshStream->Draw( nVertexDeclIndex, pCurBlock->nDrawStart, pCurBlock->nDrawCount );	
			}
		}

		if( ( s_bUseOcclusion ) && ( passType == PASS_OPAQUE ) )
		{
			m_OcclusionMng[ m_nCurOcclusionMngIndex ].EndOcclusion( nOcclusionIndex );
		}

		// Restore RenderState, SamplerSate
		if( pCurBlock->nStateBlockIndex != -1 )
		{
			GetEtDevice()->EnableZWrite( bZWrite );
			const SStateBlock *pStateBlock = GetEtStateManager()->GetStateBlock( pCurBlock->nStateBlockIndex );
			pStateBlock->RestoreState();
		}
		pBakeDepthMaterial->EndPass();
		pBakeDepthMaterial->EndEffect();
	}

	if( bAutoDeleteList ) 
	{
		m_vecRenderElement[ passType ].clear();
	}
	CEtMRTMng::GetInstance().EndDepthTarget();
}

void CEtRenderStack::RenderBlock( PassType passType, bool bAutoDeleteList )
{
	if( m_vecRenderElement[ passType ].empty() ) {
		return;
	}

	RenderPriorityEnum prevBackBufferPriority = RP_NONE;

	bool bLightSettingNeeded = ( passType != PASS_SHADOW && passType != PASS_WATER );

	int nElementSize = ( int )m_vecRenderElement[ passType ].size();
	for( int i = 0; i <nElementSize; i++ )
	{	
		SRenderStackElement *pBlock = m_vecRenderElement[ passType ][ i ];

		if(passType == PASS_USEBACKBUFFER ) {
			if( prevBackBufferPriority != pBlock->renderPriority ) {
				GetEtBackBufferMng()->DumpBackBuffer();
				prevBackBufferPriority = pBlock->renderPriority;
			}
		}

		if( bLightSettingNeeded ) {
			CEtLight::SetInfluenceLight( pBlock->pInfluenceLight );
			CEtLight::SetDirLightAttenuation( pBlock->fDirLightAttenuation );
		}

		if( pBlock->bIsAlphaSky )
		{
			GetEtDevice()->EnableAlphaBlend( true );
			GetEtDevice()->EnableZWrite( false );
		}

		CEtMaterial *pCurMaterial = pBlock->hMaterial.GetPointer();
#ifdef PRE_FIX_MATERIAL_DUMP
		if( pCurMaterial == NULL ) 
			continue;
#endif
		int nCurTechnique = pBlock->nTechniqueIndex;
		pCurMaterial->SetTechnique( nCurTechnique );
		int nPasses=0;
		pCurMaterial->BeginEffect( nPasses );
		pCurMaterial->BeginPass( 0 );
		pCurMaterial->SetGlobalParams();

		if( ( pBlock->nSaveMatIndex != -1 ) )
		{
			if( ( pBlock->pRenderSubMesh ) && ( pBlock->pRenderSubMesh->GetLinkCount() ) )
			{
				pCurMaterial->SetWorldMatArray( &pBlock->WorldMat, pBlock->nSaveMatIndex, 
					pBlock->pRenderSubMesh->GetLinkCount(), pBlock->pRenderSubMesh->GetLinkIndex() );
			}
		}
		pCurMaterial->SetWorldMatParams( &pBlock->WorldMat, &pBlock->PrevWorldMat );
		if( pBlock->pvecCustomParam )
		{
			pCurMaterial->SetCustomParamList( *pBlock->pvecCustomParam );
		}		

		// Set RenderState, SamplerState
		if( pBlock->nStateBlockIndex != -1 ) {
			const SStateBlock *pStateBlock = GetEtStateManager()->GetStateBlock( pBlock->nStateBlockIndex );
			pStateBlock->SetState();
		}

		pCurMaterial->CommitChanges();

		EtCmpFunc ZFunc;
		DWORD dwAlphaRef;
		if( pBlock->bIsTwoPassAlpha )
		{
			DWORD dwColorWrite = GetEtDevice()->SetColorWriteEnable( 0 );
			bool bZWrite = GetEtDevice()->EnableZWrite( true );
			bool bEnableAlpha = GetEtDevice()->EnableAlphaBlend( false );
			dwAlphaRef = GetEtDevice()->SetAlphaRef( 1 );

			if( pBlock->pRenderSubMesh )			
				pBlock->pRenderSubMesh->Draw( pCurMaterial->GetVertexDeclIndex( pBlock->nTechniqueIndex, 0 ), pBlock->nDrawStart, pBlock->nDrawCount );			
			else			
				pBlock->pRenderMeshStream->Draw( pCurMaterial->GetVertexDeclIndex( pBlock->nTechniqueIndex, 0 ), pBlock->nDrawStart, pBlock->nDrawCount );			

			GetEtDevice()->SetAlphaRef( dwAlphaRef );
			GetEtDevice()->EnableAlphaBlend( bEnableAlpha );
			GetEtDevice()->EnableZWrite( bZWrite );
			GetEtDevice()->SetColorWriteEnable( dwColorWrite );
			ZFunc = GetEtDevice()->SetZFunc( CF_EQUAL );
		}

		if( pBlock->pRenderSubMesh )
			pBlock->pRenderSubMesh->Draw( pCurMaterial->GetVertexDeclIndex( pBlock->nTechniqueIndex, 0 ), pBlock->nDrawStart, pBlock->nDrawCount );			
		else			
			pBlock->pRenderMeshStream->Draw( pCurMaterial->GetVertexDeclIndex( pBlock->nTechniqueIndex, 0 ), pBlock->nDrawStart, pBlock->nDrawCount );			

		if( pBlock->bIsTwoPassAlpha )
		{
			GetEtDevice()->SetZFunc( ZFunc );
		}

		// Restore RenderState, SamplerSate
		if( pBlock->nStateBlockIndex != -1 ) {
			const SStateBlock *pStateBlock = GetEtStateManager()->GetStateBlock( pBlock->nStateBlockIndex );
			pStateBlock->RestoreState();			
		}
		pCurMaterial->EndPass();
		pCurMaterial->EndEffect();

		if( pBlock->bIsAlphaSky )
		{
			GetEtDevice()->EnableAlphaBlend( false );
			GetEtDevice()->EnableZWrite( true );
		}
	}

	if( bAutoDeleteList )
	{
		m_vecRenderElement[ passType ].clear();
	}
}

void CEtRenderStack::UpdateSkyBoxRTT()
{
	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	CEtSkyBoxRTT::GetInstance().UpdateCamera( *hCamera->GetInvViewMat() );
}

void CEtRenderStack::FlushRender( bool bExtraRender )
{	
	UpdateSkyBoxRTT();

	// Bake Shadow
	if( GetEtShadowMap() )
	{
		GetEtShadowMap()->Bake( this );
	}
	// Bake Water
	if( GetEtWater() )
	{
		GetEtWater()->Bake( this, m_fElapsedTime);	
	}

	// Bake Depth
	CEtMRTMng::GetInstance().ClearDepthTarget();
	if( !s_bUseOcclusion )
	{
		m_InstancingManager.ClassifyBlock( m_vecRenderElement[ PASS_OPAQUE ], m_vecRenderElement[ PASS_INSTANCING ] );
		m_InstancingManager.RenderDepth( m_vecRenderElement[ PASS_INSTANCING ] );
	}
	std::sort( m_vecRenderElement[ PASS_OPAQUE ].begin(), m_vecRenderElement[ PASS_OPAQUE ].end(), NormalRenderSort );
	RenderDepthBlock( PASS_OPAQUE );

	CalcOcclusion();		// Occlusion calculation
	m_nCurOcclusionMngIndex = 1 - m_nCurOcclusionMngIndex;

	// Render Opaque Color Pass 
	GetEtDevice()->ClearBuffer( 0, 1.0f, 0, false, true, true );	// Z Buffer Clear
	if( s_bUseOcclusion )
	{
		m_InstancingManager.ClassifyBlock( m_vecRenderElement[ PASS_OPAQUE ], m_vecRenderElement[ PASS_INSTANCING ] );
	}
	m_InstancingManager.Render( m_vecRenderElement[ PASS_INSTANCING ] );

	GetEtDevice()->EnableAlphaBlend( true );
	GetEtDevice()->EnableZWrite( false );
	RenderBlock( PASS_ALPHASKYBOX, false );
	GetEtDevice()->EnableAlphaBlend( false );
	GetEtDevice()->EnableZWrite( true );
	RenderDepthBlock( PASS_ALPHASKYBOX, true );

	RenderBlock( PASS_OPAQUE );

	GetEtDevice()->EnableAlphaBlend( true );
	GetEtDevice()->EnableZWrite( false );
	std::stable_sort( m_vecRenderElement[ PASS_ALPHA ].begin(), m_vecRenderElement[ PASS_ALPHA ].end(), AlphaRenderSort );	
	RenderBlock( PASS_ALPHA, false );

	GetEtDevice()->EnableAlphaBlend( false );
	RenderDepthBlock( PASS_ALPHA, true );

	std::stable_sort( m_vecRenderElement[ PASS_USEBACKBUFFER ].begin(), m_vecRenderElement[ PASS_USEBACKBUFFER ].end(), AlphaRenderSort );
	GetEtDevice()->EnableAlphaBlend( true );
	RenderBlock( PASS_USEBACKBUFFER );
	GetEtDevice()->EnableAlphaBlend( false );

	if( bExtraRender )
	{
		CEtLensFlare::RenderFlareList( m_fElapsedTime );
	}

	GetEtDevice()->EnableAlphaBlend( false );
	GetEtPostProcessMng()->Render( m_fElapsedTime );
	GetEtDevice()->EnableAlphaBlend( true );

	// Shouldn't be here, but just draw here for debugging..
	CEtObject::DrawCollisionList( g_nCurFrustumMask );

	if( bExtraRender )
	{
		CEtCustomRender::RenderCustomList( m_fElapsedTime );
	}

	GetEtDevice()->EnableZWrite( true );
	GetEtDevice()->EnableAlphaBlend( false );

	if( CEtCamera::GetActiveCamera() )
	{
		CEtCamera::GetActiveCamera()->EndCamera();
	}

	RefreshAllocBlock();

#if defined(_DEBUG) || defined(_RDEBUG)
	for( int i = 0; i < PASS_COUNT; i++)
	{
		ASSERT( m_vecRenderElement[i].empty() );
	}
#endif
}


void CEtRenderStack::CalcOcclusion()
{	
	if( !s_bUseOcclusion ) return;

	PROFILE_TIME_TEST_BLOCK_START( "Occlusion Culling" );
	int i;
	int nSkipCount = 0;
	int nPrevOcclusionMng = 1 - m_nCurOcclusionMngIndex;
	CEtOcclusionMng *pPrevOcclusionMng = &m_OcclusionMng[ nPrevOcclusionMng ];

	int nCurRenderElementCount = ( int )m_vecRenderElement[ PASS_OPAQUE ].size();
	for( i = 0; i < nCurRenderElementCount; i++ )
	{
		if( i >= nCurRenderElementCount - nSkipCount )
		{
			break;
		}

		std::map< int, int >::iterator it = m_mapOcclusionCheck[ nPrevOcclusionMng ].find( m_vecRenderElement[ PASS_OPAQUE ][ i ]->nRenderUniqueID );
		if( it != m_mapOcclusionCheck[ nPrevOcclusionMng ].end() )
		{
			DWORD dwRet;
			dwRet = pPrevOcclusionMng->ResultOcclusion( it->second );
			if( dwRet == 0 )
			{
				nSkipCount++;
				std::swap( m_vecRenderElement[ PASS_OPAQUE ][ i ], m_vecRenderElement[ PASS_OPAQUE ][ nCurRenderElementCount - nSkipCount ] );
				i--;
			}
		}
	}

	if( nSkipCount )
	{
		m_vecRenderElement[ PASS_OPAQUE ].erase( m_vecRenderElement[ PASS_OPAQUE ].begin() + ( nCurRenderElementCount - nSkipCount ), m_vecRenderElement[ PASS_OPAQUE ].end() );
	}
	m_mapOcclusionCheck[ nPrevOcclusionMng ].clear();
	pPrevOcclusionMng->ClearOcclusion();	
	PROFILE_TIME_TEST_BLOCK_END();
}

CEtRenderStack *GetCurRenderStack()
{
	return g_pRenderStack[ 0 ];
}

int CEtRenderStack::AddNormalRenderElement( SRenderStackElement &RenderElement, int nRenderUniqueID )
{
	if( nRenderUniqueID == -1 )
	{
		nRenderUniqueID = m_nCurentUniqueID;
		m_nCurentUniqueID++;
	}
	RenderElement.nRenderUniqueID = nRenderUniqueID;
	m_vecRenderElement[ PASS_OPAQUE ].push_back( AllocBlock( RenderElement ) ); 

	return nRenderUniqueID;
}

void CEtRenderStack::EmptyRenderElement()
{
	for( int i = 0; i < PASS_COUNT; i++) {
		m_vecRenderElement[i].clear();
	}
	RefreshAllocBlock();
}

void CEtRenderStack::Sort( int passType, bool bOpaque )
{
	if( bOpaque ) {
		std::sort( m_vecRenderElement[ passType ].begin(), m_vecRenderElement[ passType ].end(), NormalRenderSort );
	}
	else {
		std::sort( m_vecRenderElement[ passType ].begin(), m_vecRenderElement[ passType ].end(), AlphaRenderSort );
	}
}

void CEtRenderStack::FlushBuffer()
{
	for( int i=0; i<PASS_COUNT; i++ ) SAFE_DELETE_VEC( m_vecRenderElement[i] );

	for( int i=0; i<2; i++ ) {
		m_OcclusionMng[i].FlushBuffer();
		SAFE_DELETE_VEC( m_vecOcclusionCheck[i] );
		SAFE_DELETE_MAP( m_mapOcclusionCheck[i] );

		for( int j=0; j<10; j++ )
			SAFE_DELETE_VEC( m_vecRadixSort[i][j] );
	}

	SAFE_DELETE_PVEC( m_RenderBlockPoolFree );
	SAFE_DELETE_PVEC( m_RenderBlockPoolUsed );
}