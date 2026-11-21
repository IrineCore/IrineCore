#include "StdAfx.h"
#include "HighResolutionCapture.h"
#include <shellapi.h>
#include <Shlwapi.h>
#include "shlobj.h"
#include <time.h>
#include <Direct.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CHighResolutionCapture::CHighResolutionCapture()
: m_bCapture( false )
, m_pColorAdjFilter( NULL )
, m_dwClearColor( 0xFF000000 )
, m_fTempAspectRatio( 0.0f )
{
}

CHighResolutionCapture::~CHighResolutionCapture()
{
	SAFE_DELETE( m_pColorAdjFilter );
}

void CHighResolutionCapture::Capture( const char *szFilePrefix, int nWidth, int nHeight )
{
	if( !m_pColorAdjFilter )
	{
		// Reuse the one from Saint Haven.
		m_pColorAdjFilter = (CEtColorAdjustTexFilter*)EternityEngine::CreateFilter( SF_COLORADJUSTTEX );
		m_pColorAdjFilter->SetParam( EtVector4(-0.071f, -0.107f, -0.107f, 0.0f), EtVector4(0.25f, 0.286f, 0.357f, 0.0f), EtVector4(0.357f, 0.357f, 0.536f, 0.0f), 0.9f );
		m_pColorAdjFilter->Enable( false );
	}

	if( m_bCapture )
		return;

	if( nWidth <= 0 || nWidth == INT_MAX ) nWidth = GetEtDevice()->GetMaxTextureWidth();
	if( nHeight <= 0 || nHeight == INT_MAX ) nHeight = GetEtDevice()->GetMaxTextureHeight();

	// Might be a video card memory issue.. texture creation is fine, but Save throws out-of-memory, so set a max limit.
	if( nWidth > 7200 ) nWidth = 7200;
	if( nHeight > 7200 ) nHeight = 7200;

	m_hRenderTarget = CEtTexture::CreateRenderTargetTexture( nWidth, nHeight, FMT_X8R8G8B8 );
	m_hDepthTarget = CEtDepth::CreateDepthStencil( nWidth, nHeight, FMT_D24S8 );
	m_hTempTarget = CEtTexture::CreateRenderTargetTexture( nWidth, nHeight, FMT_X8R8G8B8 );
	m_szFilePrefix = szFilePrefix;

	// There was a request to clear the background color to 0x0 and render onto an alpha plane.
	// So when setting the target to FMT_A8R8G8B8 and letting it run the default render routine as is,
	// During alpha rendering, alpha was written strangely to the buffer (later-drawn alpha overwrites buffer directly).
	// So an issue occurred where the alpha value of the alpha area drawn over opaque pixels became lower than 255.
	// Probably because the render routine is written assuming drawing onto X8R8G8B8..
	// Yet we cannot just tear apart the shader either,
	// Even with D3DRS_SEPARATEALPHABLENDENABLE, there is no clean way to render cumulative alpha blending, so set to draw only onto an opaque surface.
	// Actually, trying to draw to the alpha backbuffer while using the engine's render routine as-is seemed wrong in the first place.

	m_bCapture = true;
}

void CHighResolutionCapture::BeginCapture()
{
	if( !m_bCapture ) return;

	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	if( hCamera )
	{
		m_fTempAspectRatio = hCamera->GetAspectRatio();
		hCamera->SetAspectRatio( (float)m_hRenderTarget->Width() / (float)m_hRenderTarget->Height() );
	}

	GetEtDevice()->SetRenderTarget( m_hRenderTarget->GetSurfaceLevel() );
	GetEtDevice()->SetDepthStencilSurface( m_hDepthTarget->GetDepthBuffer() );
	GetEtDevice()->ClearBuffer( m_dwClearColor, 1.0f, 0 );
}

void CHighResolutionCapture::EndCapture()
{
	if( !m_bCapture ) return;

	GetEtDevice()->RestoreDepthStencil();
	GetEtDevice()->RestoreRenderTarget();

	EtCameraHandle hCamera = CEtCamera::GetActiveCamera();
	if( hCamera )
	{
		hCamera->SetAspectRatio( m_fTempAspectRatio );
	}

	if( m_pColorAdjFilter && 0 )
	{
		// After copying current RTT contents to temp texture
		m_hTempTarget->Copy( m_hRenderTarget );

		// Unify to use the one used in Prairie.
		float fSat = 0.f;
		fSat = m_pColorAdjFilter->GetSceneSaturation();
		m_pColorAdjFilter->SetSceneSaturation( 1.0f );
		bool bEnableZ = GetEtDevice()->EnableZ( false );	
		bool bAlphaEnable = GetEtDevice()->EnableAlphaBlend( true );
		GetEtDevice()->SetDepthStencilSurface( NULL );
		m_pColorAdjFilter->SetTexture( m_hTempTarget->GetMyIndex() );
		m_pColorAdjFilter->DrawFilter( m_hRenderTarget, 1, EtVector2(0, 0), EtVector2(1, 1) , true );
		GetEtDevice()->RestoreRenderTarget();
		GetEtDevice()->RestoreDepthStencil();
		GetEtDevice()->EnableZ( bEnableZ );
		GetEtDevice()->EnableAlphaBlend( bAlphaEnable );
		m_pColorAdjFilter->SetSceneSaturation( fSat );
	}

	char szPath[MAX_PATH]={0,};
	SHGetSpecialFolderPath(GetEtDevice()->GetHWnd(), szPath, CSIDL_PERSONAL, 0);

	__time64_t ltime;
	char szBuf[256] = { 0, }, szFile[256] = { 0, };
	struct	tm *tm_ptr;
	time(&ltime);
	tm_ptr = localtime(&ltime);

	strftime(szBuf, 256, "%Y-%m-%d %H-%M-%S %a", tm_ptr);
	sprintf(szFile, "%s %s.tga", m_szFilePrefix.c_str(), szBuf);

	char szResultName[MAX_PATH] = { 0, };

	sprintf( szResultName, "%s\\ToolScreenshot", szPath );
	if( !PathFileExists( szResultName ) ) _mkdir( szResultName );

	sprintf( szResultName, "%s\\ToolScreenshot\\%s", szPath, szFile );

	HRESULT hr = D3DXSaveSurfaceToFile( szResultName, D3DXIFF_TGA, m_hRenderTarget->GetSurfaceLevel(), NULL, NULL );

	SAFE_RELEASE_SPTR( m_hRenderTarget );
	SAFE_RELEASE_SPTR( m_hDepthTarget );
	SAFE_RELEASE_SPTR( m_hTempTarget );

	m_bCapture = false;
}