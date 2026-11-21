#include "StdAfx.h"
#include "DnMovieControl.h"
#include "EtBVLEngine.h"
#include "DnMainFrame.h"
#include "EtDrawQuad.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMovieControl::CDnMovieControl(CEtUIDialog *pParent)
: CEtUIControl( pParent )
{
	m_pTexture = NULL;
	m_fAlphaBorderHoriRate = 0.0f;
	m_fAlphaBorderVertRate = 0.0f;
}

CDnMovieControl::~CDnMovieControl(void)
{
	SAFE_RELEASE_SPTR( m_hBorderAlphaTexture );
}

void CDnMovieControl::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	if( pProperty->UICoord.fWidth > 0.0f )
		m_fAlphaBorderHoriRate = pProperty->MovieControlPropery.fAlphaBorderHori / pProperty->UICoord.fWidth;
	if( pProperty->UICoord.fHeight > 0.0f )
		m_fAlphaBorderVertRate = pProperty->MovieControlPropery.fAlphaBorderVert / pProperty->UICoord.fHeight;
}

void CDnMovieControl::Render( float fElapsedTime )
{
	CEtUIControl::Render( fElapsedTime );

	if (!m_pMovieEngine)
		return;

	if (m_pMovieEngine->IsOnLostDevice())
	{
		Stop();
		return;
	}

	if (m_pMovieEngine->GetVideoTexture( &m_pTexture ) == true || m_pTexture)
	{
		if (m_fAlphaBorderHoriRate != 0.0f || m_fAlphaBorderVertRate != 0.0f)
		{
			if (m_hBorderAlphaTexture)
				ProcessBorderAlpha();
		}

		SUICoord uiCoord, uiCoordTemp;
		GetUICoord( uiCoordTemp );

		uiCoord = m_pParent->DlgCoordToScreenCoord( uiCoordTemp );

		if (GetSafeTexture())
		{
			D3DSURFACE_DESC Desc;
			SUICoord UVCoord;
			if (m_pTexture)
				m_pTexture->GetLevelDesc( 0, &Desc );
			UVCoord.SetCoord( 0.f, 0.f, 1.f, 1.f );
			CEtSprite::GetInstance().DrawSprite( GetSafeTexture(), Desc.Width, Desc.Height, UVCoord, 0xFFFFFFFF, uiCoord, 0.f );
		}
	}
}

bool CDnMovieControl::Play( const char *szFileName, bool bRepeat )
{
	Stop();

	if (!m_pMovieEngine)
		return false;

	OutputDebug("[BVL] Before Open\n");

	if (m_pMovieEngine->Open( szFileName, bRepeat ? CEtBVLEngine::BVL_OPEN_OPT_AUTOPLAY_REPEAT : CEtBVLEngine::BVL_OPEN_OPT_AUTOPLAY ) == false)
	{
		OutputDebug("[BVL] Open returns FALSE\n");
		return false;
	}

	OutputDebug("[BVL] OPEN : %s\n", szFileName);

	// If either is not 0.0f, border alpha processing must be performed.
	if (m_fAlphaBorderHoriRate != 0.0f || m_fAlphaBorderVertRate != 0.0f)
	{
		// Since videos are rarely shown, do not create during Initialize; create just before playing.
		// Don't delete immediately on Stop, otherwise it would have to be recreated every time it's shown again.
		BVL_VIDEO_INFO VideoInfo;
		if (m_pMovieEngine->GetVideoInfo(VideoInfo) != E_FAIL)
		{
			bool bCreateAlphaTexture = true;
			if (m_hBorderAlphaTexture)
			{
				if (m_hBorderAlphaTexture->Width() == VideoInfo.width && m_hBorderAlphaTexture->Height() == VideoInfo.height)
					bCreateAlphaTexture = false;
			}

			if (bCreateAlphaTexture)
			{
				SAFE_RELEASE_SPTR(m_hBorderAlphaTexture);
				m_hBorderAlphaTexture = CEtTexture::CreateRenderTargetTexture( VideoInfo.width, VideoInfo.height, FMT_A8R8G8B8 );
			}
		}
	}

	return true;
}

void CDnMovieControl::Stop()
{
	if (!m_pMovieEngine)
		return;

	if (m_pTexture) {
		m_pTexture = NULL;
		OutputDebug("[BVL] CLOSE\n");
	}

	m_pMovieEngine->Stop();
}


void CDnMovieControl::Show( bool bShow )
{
	CEtUIControl::Show( bShow );

	if( !bShow )
	{
		Stop();
		m_pMovieEngine.reset();
	}
	else
	{
		m_pMovieEngine.reset(new CEtBVLEngine);
		if (m_pMovieEngine->Initialize( CDnMainFrame::GetInstance().GetHWnd(), &CEtDevice::GetInstance() ) == false)
		{
			m_pMovieEngine.reset();
			return;
		}
		m_pMovieEngine->Stop();
	}
}

void CDnMovieControl::ProcessBorderAlpha()
{
	// Reset RTT texture as render target.
	bool bEnableZ = GetEtDevice()->EnableZ( false );
	bool bAlphaEnable = GetEtDevice()->EnableAlphaBlend( true );
	// Since the background is 0x0, draw with One-One blending to copy Source texture pixels as-is.
	GetEtDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
	GetEtDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
	GetEtDevice()->SetDepthStencilSurface( NULL );

	GetEtDevice()->SetRenderTarget( m_hBorderAlphaTexture->GetSurfaceLevel() );
	GetEtDevice()->ClearBuffer( 0x0, 1.0f, 0, true, false, false );

	// Apply boundary alpha blending using vertex alpha.
	STextureDiffuseVertex2D Vertices[ 16 ];
	int nWidth, nHeight;
	EtViewPort Viewport;

	GetEtDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
	GetEtDevice()->SetPixelShader( NULL );
	GetEtDevice()->SetVertexShader( NULL );
	GetEtDevice()->SetTexture( 0, m_pTexture );
	GetEtDevice()->GetViewport( &Viewport );

	nWidth = Viewport.Width;
	nHeight = Viewport.Height;

	float XPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float YPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	XPos[1] = XPos[0] + (XPos[3]-XPos[0]) * m_fAlphaBorderHoriRate;
	XPos[2] = XPos[3] - (XPos[3]-XPos[0]) * m_fAlphaBorderHoriRate;
	YPos[1] = YPos[0] + (YPos[3]-YPos[0]) * m_fAlphaBorderVertRate;
	YPos[2] = YPos[3] - (YPos[3]-YPos[0]) * m_fAlphaBorderVertRate;

	float UVXPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float UVYPos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	for( int i = 0; i < 4; ++i )
	{
		UVXPos[i] = XPos[i];
		UVYPos[i] = YPos[i];
		XPos[i] = XPos[i] * nWidth - 0.5f;
		YPos[i] = YPos[i] * nHeight - 0.5f;
	}

	for( int i = 0; i < 4; i++)
	{
		Vertices[i*4+0].Position.y = Vertices[i*4+1].Position.y = Vertices[i*4+2].Position.y = Vertices[i*4+3].Position.y = YPos[i];
		Vertices[ i ].Position.x = Vertices[4 + i].Position.x = Vertices[8 + i].Position.x = Vertices[12 + i].Position.x = XPos[i];
		Vertices[ i ].Position.z = Vertices[4 + i].Position.z = Vertices[8 + i].Position.z = Vertices[12 + i].Position.z = 0.0f;
		Vertices[ i ].Position.w = Vertices[4 + i].Position.w = Vertices[8 + i].Position.w = Vertices[12 + i].Position.w = 1.0f;
		Vertices[ i ].Color = Vertices[4 + i].Color = Vertices[8 + i].Color = Vertices[12 + i].Color = D3DCOLOR_ARGB(255, 255, 255, 255);

		Vertices[i*4+0].TexCoordinate.y = Vertices[i*4+1].TexCoordinate.y = Vertices[i*4+2].TexCoordinate.y = Vertices[i*4+3].TexCoordinate.y = UVYPos[i];
		Vertices[ i ].TexCoordinate.x = Vertices[4 + i].TexCoordinate.x = Vertices[8 + i].TexCoordinate.x = Vertices[12 + i].TexCoordinate.x = UVXPos[i];
	}
	Vertices[ 0 ].Color = Vertices[ 1 ].Color = Vertices[ 2 ].Color = Vertices[ 3 ].Color = Vertices[ 4 ].Color = Vertices[ 7 ].Color = D3DCOLOR_ARGB(0, 255, 255, 255);
	Vertices[ 8 ].Color = Vertices[ 11 ].Color = Vertices[ 12 ].Color = Vertices[ 13 ].Color = Vertices[ 14 ].Color = Vertices[ 15 ].Color = D3DCOLOR_ARGB(0, 255, 255, 255);

	unsigned short Indices[54] = {
		0, 5, 4, 0, 1, 5, 1, 2, 5, 6, 5, 2, 2, 3, 6, 7, 6, 3,
		4, 5, 8, 9, 8, 5, 5, 6, 9, 10, 9, 6, 6, 7, 10, 11, 10, 7,
		8, 9, 12, 13, 12, 9, 9, 10, 13, 14, 13, 10, 10, 15, 14, 10, 11, 15
	};
	GetEtDevice()->DrawIndexedPrimitiveUP( PT_TRIANGLELIST, 0, 16, 18, Indices, FMT_INDEX16, Vertices, sizeof( STextureDiffuseVertex2D ) );

	// Restore target
	GetEtDevice()->RestoreRenderTarget();
	GetEtDevice()->RestoreDepthStencil();
	GetEtDevice()->EnableZ( bEnableZ );
	GetEtDevice()->EnableAlphaBlend( bAlphaEnable );
}

EtTexture *CDnMovieControl::GetSafeTexture()
{
	if (m_fAlphaBorderHoriRate != 0.0f || m_fAlphaBorderVertRate != 0.0f)
	{
		if (m_hBorderAlphaTexture)
			return (EtTexture *)m_hBorderAlphaTexture->GetTexturePtr();
	}
	return m_pTexture;
}