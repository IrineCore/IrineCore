#include "StdAfx.h"
#include "EtDOFFilter.h"
#include "EtOptionController.h"
#include "EtBackBufferMng.h"
#include "EtMRTMng.h"
#include "EtDrawQuad.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtDOFFilter::CEtDOFFilter()
{
	m_fNearDOFStart = 30.0f;
	m_fNearDOFEnd = 50.0f;
	m_fFarDOFStart = 500.0f;
	m_fFarDOFEnd = 2000.0f;
	m_fFocusDistance = 150.0f;
	
	m_fNearBlurSize = 16;
	m_fFarBlurSize = 4;

	m_Type = SF_DOF;

	m_vDOFValue = EtVector4(0,0,0,0);
	m_vDOFBlurSize = EtVector4(0,0,0,0);
	m_bDOFEnable = false;
	m_fCamFar = 3000.0f;
	m_blurRectSize = 0;
}

CEtDOFFilter::~CEtDOFFilter()
{
	Clear();
}

void CEtDOFFilter::Clear()
{
	SAFE_RELEASE_SPTR( m_hBlurSizeBuffer );
	SAFE_RELEASE_SPTR( m_hDOFHoriBuffer );
	SAFE_RELEASE_SPTR( m_hDOFVertBuffer );
	SAFE_RELEASE_SPTR( m_hBlurSizeBlurBuffer );
	CEtPostProcessFilter::Clear();
}

void CEtDOFFilter::Initialize()
{
	Clear();

	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	m_bDOFEnable = Option.bEnableDOF;

	if( m_bDOFEnable ) {
		m_hMaterial = LoadResource( "DOFFilter.fx", RT_SHADER );
		if( !m_hMaterial ) return;

		AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_DOFValue", &m_vDOFValue );
		AddCustomParam( m_vecCustomParam, EPT_VECTOR_PTR, m_hMaterial, "g_DOFBlurSize", &m_vDOFBlurSize );
		AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fFocusDistance", &m_fFocusDistance);
		AddCustomParam( m_vecCustomParam, EPT_FLOAT_PTR, m_hMaterial, "g_fBlurRect", &m_blurRectSize );

		int nTexIndex = GetEtBackBufferMng()->GetBackBufferIndex();
		AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_BackBuffer", &nTexIndex );
		nTexIndex = CEtMRTMng::GetInstance().GetDepthTarget()->GetMyIndex();
		AddCustomParam( m_vecCustomParam, EPT_TEX, m_hMaterial, "g_DepthTex", &nTexIndex );

		// Check whether R16F is supported.
		HRESULT resultFullScreen = GetEtDevice()->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_R16F, D3DFMT_X8R8G8B8, TRUE);
		HRESULT resultWindowed = GetEtDevice()->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_R16F, D3DFMT_X8R8G8B8, FALSE);	
		EtFormat BlurSizeFormat = FMT_R32F;		
		if( resultFullScreen == D3D_OK && resultWindowed == D3D_OK ) {
			BlurSizeFormat = FMT_R16F;
		}		
		m_hBlurSizeBuffer = AddRenderTarget( 160, 100, "g_BlurSizeBuffer", BlurSizeFormat );		//  Use a fixed size regardless of screen resolution (to reduce Shader Constant setup and Device Reset effort)
		m_hBlurSizeBlurBuffer = AddRenderTarget( 160, 100, "g_BlurSizeBlurBuffer", BlurSizeFormat );		//  Use a fixed size regardless of screen resolution (to reduce Shader Constant setup and Device Reset effort)
		m_hDOFHoriBuffer = AddRenderTarget( -1, -1, "g_DOFHoriBuffer" );
	}
}

void CEtDOFFilter::AddBlurRect( SUICoord Coord, float fBlurWidth )
{
	m_blurRectList.push_back( std::make_pair(Coord, fBlurWidth ) );
}

void CEtDOFFilter::DrawBlurRect()
{
	if( m_blurRectList.empty()  ) 
		return;
	int nPasses = 0;
	if( !m_hMaterial )  return;
	m_hMaterial->SetTechnique( 7 );
	m_hMaterial->BeginEffect( nPasses );
	m_hMaterial->BeginPass( 0 );	
	GetEtDevice()->SetVertexShader( NULL );	// Use Fixed Pipeline

	//g_fBlurRect
	for each( std::pair<SUICoord, float> e in m_blurRectList ) {
		m_blurRectSize = e.second;
		m_hMaterial->SetCustomParamList( m_vecCustomParam );
		m_hMaterial->CommitChanges();
		EtVector2 vStart(e.first.fX, e.first.fY);
		EtVector2 vEnd( e.first.fX + e.first.fWidth, e.first.fY + e.first.fHeight );
		DrawQuad( &vStart, &vEnd, &EtVector2(0,0), &EtVector2(1,1));
	}
	m_blurRectList.clear();

	m_hMaterial->EndPass();
	m_hMaterial->EndEffect();
}

void CEtDOFFilter::Render( float fElapsedTime )
{
	if( !m_bDOFEnable ) {
		return;
	}

	m_vDOFValue.x = m_fFarDOFStart;
	m_vDOFValue.y = 1.0f / ( m_fFarDOFEnd - m_fFarDOFStart );
	m_vDOFValue.z = m_fNearDOFStart;
	m_vDOFValue.w = 1.0f / ( m_fNearDOFEnd - m_fNearDOFStart );

	m_vDOFBlurSize.x = m_fFarBlurSize;
	m_vDOFBlurSize.y = m_fNearBlurSize;

	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();

	m_fCamFar = hCamera->GetCameraFar();
		
	DrawFilter( m_hBlurSizeBuffer, 0 );	
	DrawBlurRect();
	DrawFilter( m_hBlurSizeBlurBuffer, 6 );	//	- Down Filter
	DrawFilter( m_hDOFHoriBuffer, 4 );
	DrawFilter( CEtTexture::Identity(), 5 );

}
