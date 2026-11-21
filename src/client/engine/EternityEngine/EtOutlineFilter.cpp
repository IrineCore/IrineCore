#include "StdAfx.h"
#include "EtOutlineFilter.h"
#include "EtBackBufferMng.h"
#include "EtMRTMng.h"
#include "EtSaveMat.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_SMART_PTR_STATIC( CEtOutlineObject, 5 )

bool CEtOutlineObject::ProcessIntensity( float fElapsedTime )
{
	m_fIntensity += m_fIntensitySpeed * fElapsedTime * 2.f;
	m_fIntensity = EtClamp( m_fIntensity, 0.f, 1.f );
	if( m_fIntensity > 0.f ) return true;
	return false;
}

void CEtOutlineObject::ClassifyChild( int nIndex, std::vector< std::pair<EtObjectHandle, int> > &vecObjects )
{
}

void CEtOutlineObject::Show( bool bShow, float fSpeed )
{
	m_fIntensitySpeed = bShow ? fSpeed : -fSpeed;		
	m_bShow = bShow;
}

EtOutlineHandle CEtOutlineObject::Create( EtObjectHandle hObject )
{
	return (new CEtOutlineObject(hObject))->GetMySmartPtr();
}

CEtOutlineObject::CEtOutlineObject( EtObjectHandle object )
{
	m_hObject = object;
	m_fIntensity = 0.0f;
	m_fIntensitySpeed = -1.0f;
	m_vColor = EtColor(1,1,1,1);
	m_fWidth = 0.7f;
	m_bShow = false;
}
//////////////////////////////////////////////////////////////////////////
 
CEtOutlineFilter::CEtOutlineFilter()
{
	m_fIntensityParam = 3.f;
	m_BorderColorParam = EtColor( 0.8f, 1.0f, 0.5f, 1.0f );

	m_Type = SF_OUTLINE;
}

CEtOutlineFilter::~CEtOutlineFilter()
{
	Clear();
}

void CEtOutlineFilter::Clear()
{
	SAFE_RELEASE_SPTR( m_hOutlineSource );
	SAFE_RELEASE_SPTR( m_hBlurTarget );
	CEtPostProcessFilter::Clear();
}

void CEtOutlineFilter::Initialize()
{
	Clear();

	m_hMaterial = LoadResource( "OutlineFilter.fx", RT_SHADER );
	if( !m_hMaterial ) return;

	int nTexIndex = -1;
	int nRet = -1;
	nRet = AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_DiffuseTex", &nTexIndex );
	ASSERT( nRet != -1 );
	nRet = AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fOutlineStrength", &m_fIntensityParam );
	ASSERT( nRet != -1 );
	nRet = AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_OutlineColor", &m_BorderColorParam );
	ASSERT( nRet != -1 );

	int nWidth, nHeight;

	m_PixelSize.x = 1.0f / ( GetEtDevice()->Width() * m_fDrawScale );
	m_PixelSize.y = 1.0f / ( GetEtDevice()->Height() * m_fDrawScale );
	AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_fPixelSize", &m_PixelSize );

	nWidth = ( int )( GetEtDevice()->Width() * m_fDrawScale );
	nHeight = ( int )( GetEtDevice()->Height() * m_fDrawScale );
	m_hOutlineSource = AddRenderTarget( nWidth, nHeight, "g_OutlineSource" , FMT_A8R8G8B8 );
	m_hBlurTarget = AddRenderTarget( nWidth, nHeight, "g_BlurBuffer" );
}

void CEtOutlineFilter::DrawOutline( EtObjectHandle hObject, EtOutlineHandle hOutline, SAABox &Box )
{	
	if( ( !hObject ) || ( !hObject->IsShow() ) )
	{
		return;
	}
	if( !m_hMaterial ) return;

	SAABox ObjBox;
	int i, nSubmeshCount;

	hObject->GetBoundingBox( ObjBox );
	Box.AddPoint( ObjBox.Min );
	Box.AddPoint( ObjBox.Max );
	nSubmeshCount = hObject->GetSkin()->GetMeshHandle()->GetSubMeshCount();
	for( i = 0; i < nSubmeshCount; i++)
	{
		CEtSubMesh *pSubMesh = hObject->GetSkin()->GetMeshHandle()->GetSubMesh( i );
		if( ( !hObject->IsShowSubmesh( i ) ) || ( pSubMesh == NULL ) )
		{
			continue;
		}

		int nCurTechnique, nPasses, nSaveMatIndex, nLinkCount;

		nLinkCount = pSubMesh->GetLinkCount();
		nCurTechnique = ( nLinkCount == 0 ) ? 0 : 1;
		nSaveMatIndex = -1;
		if( nCurTechnique != 0 )
		{ 
			nSaveMatIndex = hObject->GetSaveMatIndex();
			if( !GetEtSaveMat()->IsValidIndex( nSaveMatIndex ) ) 
			{
				continue;
			}
		}
		m_hMaterial->SetTechnique( nCurTechnique );
		nPasses = 0;
		m_hMaterial->BeginEffect( nPasses );

		EtMatrix WorldMat = *hObject->GetWorldMat();
		WorldMat._41 += 1.0f;

		m_hMaterial->BeginPass( 0 );
		m_hMaterial->SetGlobalParams();
		if( nSaveMatIndex != -1 )
		{
			m_hMaterial->SetWorldMatArray( &WorldMat, nSaveMatIndex, nLinkCount, pSubMesh->GetLinkIndex() );
		}
		m_hMaterial->SetWorldMatParams( &WorldMat, &WorldMat );

		static float fWidthCorrectionValue = 5.0f;
		static float fIntensityCorrectionValue = 1.5f;
		m_PixelSize.x = ( hOutline->GetBorderWidth() * fWidthCorrectionValue ) / ( GetEtDevice()->Width() * m_fDrawScale );
		m_PixelSize.y = ( hOutline->GetBorderWidth() * fWidthCorrectionValue ) / ( GetEtDevice()->Height() * m_fDrawScale );
		m_fIntensityParam = hOutline->GetIntensity() * fIntensityCorrectionValue;
		m_BorderColorParam = hOutline->GetColor();
		m_vecCustomParam[ 0 ].nTextureIndex = hObject->GetSkin()->GetDiffuseTexIndex( i );

		m_hMaterial->SetCustomParamList( m_vecCustomParam );
		m_hMaterial->CommitChanges();
		pSubMesh->GetMeshStream()->Draw( m_hMaterial->GetVertexDeclIndex( nCurTechnique, 0 ) );
		m_hMaterial->EndPass();
		m_hMaterial->EndEffect();
	}
	
}

void CEtOutlineFilter::Render( float fElapsedTime )
{
#ifdef PRE_FIX_MATERIAL_DUMP
	if( !m_hMaterial ) return;
#endif

	bool bSkip = true;
	int i, nSize=0;
	SAABox Box;
	int nCount = CEtOutlineObject::GetItemCount();
	{
		ScopeLock<CSyncLock> Lock( CEtOutlineObject::s_SmartPtrLock );
		for( i = 0; i < nCount; i++) {
			EtOutlineHandle hHandle = CEtOutlineObject::GetItem( i );
			if( hHandle->ProcessIntensity( fElapsedTime ) ) {
				bSkip = false;
			}
		}

		if( bSkip ) {
			return;
		}

		GetEtDevice()->SetRenderTarget( m_hOutlineSource->GetSurfaceLevel() );
		GetEtDevice()->ClearBuffer( 0, 1.0f, 0, true, false, false );
		Box.Reset();
		nCount = CEtOutlineObject::GetItemCount();

//		Rotha - Adjust depth value because objects were visible through other objects
		float fDepthBias = GetEtDevice()->SetDepthBias( -0.00025f );

		for( i = 0; i < nCount; i++)
		{
			EtOutlineHandle hHandle = CEtOutlineObject::GetItem( i );
			if( ( !hHandle ) || ( hHandle->GetIntensity() <= 0.0f ) )
			{
				continue;
			}
			EtObjectHandle hObject = hHandle->GetObject();
			if( !hObject  ) 
			{
				continue;		
			}
			DrawOutline( hObject, hHandle, Box );

			int j;
			std::vector<EtObjectHandle> &vecChild = hObject->GetChild();
			for( j = 0; j <  ( int )vecChild.size(); j++ )
			{
				DrawOutline( hObject->GetChild()[ j ], hHandle, Box );
			}
		}
		GetEtDevice()->SetDepthBias( fDepthBias );
	}


	GetEtDevice()->RestoreDepthStencil();

	SAABox ProjBox;
	ProjBox.Min = EtVector3(-1,-1,0);
	ProjBox.Max = EtVector3(1, 1, 0);
	if( CEtCamera::GetActiveCamera() && nSize != 0 ) {		
		Box.GetVertices( m_Bounds );
		for( i = 0; i < 8; i++) {
			EtVec3TransformCoord(&m_Bounds[i], &m_Bounds[i], CEtCamera::GetActiveCamera()->GetViewProjMat());
		}		
		ProjBox.Reset();
		for( i = 0; i < 8; i++) {
			ProjBox.AddPoint( m_Bounds[i] );
		}
	}

	EtVector2 vStart = EtVector2( (ProjBox.Min.x + 1.0f ) * 0.5f, 1.0f - (ProjBox.Max.y + 1.0f ) * 0.5f);
	EtVector2 vEnd = EtVector2( (ProjBox.Max.x + 1.0f ) * 0.5f, 1.0f - (ProjBox.Min.y + 1.0f ) * 0.5f);

	vStart -= EtVector2(0.025f, 0.07f);
	vEnd += EtVector2(0.025f, 0.07f);
	if(vStart.x<0)vStart.x = 0;
	if(vStart.y<0)vStart.y = 0;
	if(vEnd.x>1)vEnd.x = 1;
	if(vEnd.y>1)vEnd.y = 1;
	
	GetEtDevice()->EnableAlphaTest( false );
	DrawFilter( m_hBlurTarget, 2 , vStart, vEnd, true );
	GetEtDevice()->EnableAlphaTest( true );
	GetEtDevice()->EnableAlphaBlend( true );
	GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	DrawFilter( CEtTexture::Identity(), 3 , vStart, vEnd, false );
	GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	GetEtDevice()->EnableAlphaBlend( false );
}

void CEtOutlineFilter::OnResetDevice()
{
	int nWidth, nHeight;

	nWidth = ( int )( GetEtDevice()->Width() * m_fDrawScale );
	nHeight = ( int )( GetEtDevice()->Height() * m_fDrawScale );
	if( m_hOutlineSource )
		m_hOutlineSource->SetSize( nWidth, nHeight );
	if( m_hBlurTarget )
		m_hBlurTarget->SetSize( nWidth, nHeight );
}

/*#include "StdAfx.h"
#include "EtOutlineFilter.h"
#include "EtBackBufferMng.h"
#include "EtMRTMng.h"
#include "EtSaveMat.h"

DECL_SMART_PTR_STATIC( CEtOutlineObject, 5 )

bool CEtOutlineObject::ProcessIntensity( float fElapsedTime )
{
	m_fIntensity += m_fIntensitySpeed * fElapsedTime * 2.f;
	m_fIntensity = EtClamp( m_fIntensity, 0.f, 1.f );
	if( m_fIntensity > 0.f ) return true;
	return false;
}

void CEtOutlineObject::ClassifyChild( int nIndex, std::vector< std::pair<EtObjectHandle, int> > &vecObjects )
{
}

void CEtOutlineObject::Show( bool bShow, float fSpeed )
{
	m_fIntensitySpeed = bShow ? fSpeed : -fSpeed;		
	m_bShow = bShow;
}

EtOutlineHandle CEtOutlineObject::Create( EtObjectHandle hObject )
{
	return (new CEtOutlineObject(hObject))->GetMySmartPtr();
}

CEtOutlineObject::CEtOutlineObject( EtObjectHandle object )
{
	m_hObject = object;
	m_fIntensity = 0.0f;
	m_fIntensitySpeed = -1.0f;
	m_vColor = EtColor(1,1,1,1);
	m_fWidth = 0.7f;
	m_bSolid = false;
	m_bSkipStaticObject = false;
	m_bShow = false;
	m_Type = NORMAL;
}
//////////////////////////////////////////////////////////////////////////

CEtOutlineFilter::CEtOutlineFilter()
{
	SetBloomScale( 0.8f );
	SetDrawSacle( 1.0f / 1.5f );

	m_fIntensityParam = 1.f;
	m_fBorderWidthParam = 1.f;
	m_BorderColorParam = EtColor(0.8f, 1.0f, 0.5f, 1.0f);

	m_Type = SF_OUTLINE;
}

CEtOutlineFilter::~CEtOutlineFilter()
{
	Clear();
}

void CEtOutlineFilter::Clear()
{
	SAFE_RELEASE_SPTR( m_hBrightTarget );
	SAFE_RELEASE_SPTR( m_hHoriBlurTarget );
	SAFE_RELEASE_SPTR( m_hVertBlurTarget );
	SAFE_RELEASE_SPTR( m_hDepth );
	CEtPostProcessFilter::Clear();
}

void CEtOutlineFilter::Initialize()
{
	Clear();

	EtVector4 PixelSize;
	m_hMaterial = LoadResource( "OutlineFilter.fx", RT_SHADER );
	if (!m_hMaterial) return;

	int nTexIndex = -1;
	int nRet = -1;
	nRet = AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_DiffuseTex", &nTexIndex );
	ASSERT( nRet != -1 );
	nRet = AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fBloomScale", &m_fBloomScale );
	ASSERT( nRet != -1 );
	nRet = AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fIntensity", &m_fIntensityParam );
	ASSERT( nRet != -1 );
	nRet = AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fBorderWidth", &m_fBorderWidthParam );
	ASSERT( nRet != -1 );	
	nRet = AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_BorderColor", &m_BorderColorParam );
	ASSERT( nRet != -1 );

	PixelSize.x = 1.0f / ( GetEtDevice()->Width() * m_fDrawScale );
	PixelSize.y = 1.0f / ( GetEtDevice()->Height() * m_fDrawScale );
	AddCustomParam( m_vecCustomParam, EPT_VECTOR, m_hMaterial, "g_fPixelSize", &PixelSize );
	int nBackBufferIndex = GetEtBackBufferMng()->GetBackBufferIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_BackBuffer", &nBackBufferIndex );

	int nWidth, nHeight;

	nWidth = ( int )( GetEtDevice()->Width() * m_fDrawScale );
	nHeight = ( int )( GetEtDevice()->Height() * m_fDrawScale );
	m_hBrightTarget = AddRenderTarget( nWidth, nHeight, "g_BrightPass" , FMT_X8R8G8B8 );
	m_hHoriBlurTarget = AddRenderTarget( nWidth, nHeight, "g_BloomHori" );
	m_hVertBlurTarget = AddRenderTarget( nWidth, nHeight, "g_BloomVert" );

	m_hDepth = CEtDepth::CreateDepthStencil( nWidth, nHeight );

	int nDepthTexIndex = CEtMRTMng::GetInstance().GetDepthTarget()->GetMyIndex();
	AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_DepthTex", &nDepthTexIndex );	
}

void CEtOutlineFilter::DrawOutline( EtObjectHandle hObject, EtOutlineHandle hOutline, SAABox &Box )
{	
	SAABox ObjBox;
	hObject->GetBoundingBox( ObjBox );
	Box.AddPoint( ObjBox.Min );
	Box.AddPoint( ObjBox.Max );
	for( int i = 0; i < hObject->GetSkin()->GetMeshHandle()->GetSubMeshCount(); i++) {
		if( !hObject->IsShowSubmesh( i ) ) continue;
		int nTechniqueAni = (hObject->GetSkin()->GetMeshHandle()->GetSubMesh(i)->GetLinkCount() == 0 ) ? 0 : 1;
		if( nTechniqueAni == 0 && hOutline->IsSkipStaticObject() ) continue;

		int nOutlineTech = hOutline->GetType();
		int nTech = nTechniqueAni + nOutlineTech;

		m_hMaterial->SetTechnique( nTech );
		int nPasses = 0;
		m_hMaterial->BeginEffect( nPasses );

		for( int nPass = 0; nPass < nPasses; nPass++) {
			if( hOutline->IsSolid() && nPass == 0) continue;
			int nSaveMatIndex = -1;
			if( nTechniqueAni != 0 ) { 
				nSaveMatIndex = hObject->GetSaveMatIndex();
				if( !GetEtSaveMat()->IsValidIndex(nSaveMatIndex) ) break;
			}
			m_hMaterial->BeginPass( nPass );
			m_hMaterial->SetGlobalParams();

			if( nSaveMatIndex != -1 ) {
				m_hMaterial->SetWorldMatArray( hObject->GetWorldMat(), nSaveMatIndex, 
					hObject->GetSkin()->GetMeshHandle()->GetSubMesh(i)->GetLinkCount(), hObject->GetSkin()->GetMeshHandle()->GetSubMesh(i)->GetLinkIndex() );
			}

			m_hMaterial->SetWorldMatParams( hObject->GetWorldMat(), hObject->GetWorldMat());

			m_fIntensityParam = hOutline->GetIntensity();
			m_fBorderWidthParam = hOutline->GetBorderWidth();
			m_BorderColorParam = hOutline->GetColor();
			m_vecCustomParam[ 0 ].nTextureIndex = hObject->GetSkin()->GetDiffuseTexIndex( i );
			m_hMaterial->SetCustomParamList( m_vecCustomParam );
			m_hMaterial->CommitChanges();

			if( nPass == 0 )
				GetEtDevice()->SetRenderState( D3DRS_CULLMODE, CULL_CW);
			else 
				GetEtDevice()->SetRenderState( D3DRS_CULLMODE, CULL_CCW);


			hObject->GetSkin()->GetMeshHandle()->GetSubMesh(i)->GetMeshStream()->Draw( m_hMaterial->GetVertexDeclIndex( nTechniqueAni, 0 ) );
			m_hMaterial->EndPass();
		}

		GetEtDevice()->SetRenderState( D3DRS_CULLMODE, CULL_CCW  );

		m_hMaterial->EndEffect();
	}

}

void CEtOutlineFilter::Render( float fElapsedTime )
{
	bool bSkip = true, bEnableZ;
	int i, nSize=0;
	SAABox Box;
	int nCount = CEtOutlineObject::GetItemCount();
	{
		ScopeLock<CSyncLock> Lock( CEtOutlineObject::s_SmartPtrLock );
		for( i = 0; i < nCount; i++) {
			EtOutlineHandle hHandle = CEtOutlineObject::GetItem( i );
			if( hHandle->ProcessIntensity( fElapsedTime ) ) {
				bSkip = false;
			}
		}

		if( bSkip ) {
			return;
		}

		GetEtDevice()->SetRenderTarget( m_hBrightTarget->GetSurfaceLevel() );
		GetEtDevice()->SetDepthStencilSurface( m_hDepth->GetDepthBuffer() );
		bEnableZ = GetEtDevice()->EnableZ( true );
		GetEtDevice()->ClearBuffer( 0x0 );

		Box.Reset();

		nCount = CEtOutlineObject::GetItemCount();
		for( i = 0; i < nCount; i++) {
			EtOutlineHandle hHandle = CEtOutlineObject::GetItem( i );
			EtObjectHandle hObject = hHandle->GetObject();
			if( !hObject || !hObject->IsShow()  ) continue;		
			if(  hObject->GetSkin()->GetMeshHandle()->GetSubMeshCount() >  0 ) {
				DrawOutline( hObject, hHandle, Box );
			}
			for( int j = 0; j < (int)hObject->GetChild().size(); j++ ) {
				EtObjectHandle hChildObject = hObject->GetChild()[j];
				if( !hChildObject->IsShow() ) continue;
				if( hChildObject->GetSkin()->GetMeshHandle()->GetSubMeshCount() > 0 ) {
					DrawOutline( hChildObject, hHandle, Box );
				}
			}
		}
	}


	GetEtDevice()->RestoreDepthStencil();
	GetEtDevice()->EnableZ( bEnableZ );

	GetEtDevice()->EnableZWrite( false );

	SAABox ProjBox;
	ProjBox.Min = EtVector3(-1,-1,0);
	ProjBox.Max = EtVector3(1, 1, 0);
	if( CEtCamera::GetActiveCamera() && nSize != 0 ) {		
		Box.GetVertices( m_Bounds );
		for( i = 0; i < 8; i++) {
			EtVec3TransformCoord(&m_Bounds[i], &m_Bounds[i], CEtCamera::GetActiveCamera()->GetViewProjMat());
		}		
		ProjBox.Reset();
		for( i = 0; i < 8; i++) {
			ProjBox.AddPoint( m_Bounds[i] );
		}
	}

	EtVector2 vStart = EtVector2( (ProjBox.Min.x + 1.0f ) * 0.5f, 1.0f - (ProjBox.Max.y + 1.0f ) * 0.5f);
	EtVector2 vEnd = EtVector2( (ProjBox.Max.x + 1.0f ) * 0.5f, 1.0f - (ProjBox.Min.y + 1.0f ) * 0.5f);

	vStart -= EtVector2(0.025f, 0.07f);
	vEnd += EtVector2(0.025f, 0.07f);
	if(vStart.x<0)vStart.x = 0;
	if(vStart.y<0)vStart.y = 0;
	if(vEnd.x>1)vEnd.x = 1;
	if(vEnd.y>1)vEnd.y = 1;

	DrawFilter( m_hHoriBlurTarget, 2 , vStart, vEnd, true );
	DrawFilter( m_hVertBlurTarget, 3 , vStart, vEnd, true );
	DrawFilter( CEtTexture::Identity(), 4 , vStart, vEnd, false);
}

void CEtOutlineFilter::OnResetDevice()
{
	int nWidth, nHeight;

	nWidth = ( int )( GetEtDevice()->Width() * m_fDrawScale );
	nHeight = ( int )( GetEtDevice()->Height() * m_fDrawScale );
	if( m_hBrightTarget )
		m_hBrightTarget->SetSize( nWidth, nHeight );
	if( m_hHoriBlurTarget )
		m_hHoriBlurTarget->SetSize( nWidth, nHeight );
	if( m_hVertBlurTarget )
		m_hVertBlurTarget->SetSize( nWidth, nHeight );
	if( m_hDepth )
		m_hDepth->SetSize( nWidth, nHeight );
}
*/