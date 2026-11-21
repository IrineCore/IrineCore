#include "StdAfx.h"
#include "EtLensFlare.h"
#include "EtCamera.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

DECL_SMART_PTR_STATIC( CEtLensFlare, 2 )

CEtLensFlare::CEtLensFlare(void)
{
	m_fSunRadius = 0.1f;
	m_fFlareRadius = 0.15f;
	m_dwMaxVisiblePoint = 0;
	m_fFlareAlphaSmooth = 0.f;
	m_pLightShaftFilter = NULL;
}

CEtLensFlare::~CEtLensFlare(void)
{
	SAFE_RELEASE_SPTR( m_hSunTexture );
	SAFE_RELEASE_SPTR( m_hFlareTexture );
	SAFE_DELETE( m_pLightShaftFilter );
}

void CEtLensFlare::Initialize( EtVector3 &LightDir, const char *pSunTextureName, const char *pFlareTextureName )
{
	m_LightDir = LightDir;
	m_hSunTexture = LoadResource( pSunTextureName, RT_TEXTURE );
	m_hFlareTexture = LoadResource( pFlareTextureName, RT_TEXTURE );

	m_fAspectRatio = GetEtDevice()->Height() / ( float )GetEtDevice()->Width();

	EtMatrixOrthoLH( &m_FlareProjMat, 2.0f, 2.0f * m_fAspectRatio, 0.0f, 1.0f );

//	m_pLightShaftFilter = ( CEtLightShaftFilter * )EternityEngine::CreateFilter( SF_LIGHTSHAFT );
}

void CEtLensFlare::Render( float fElpasedTime )
{
	if( m_dwMaxVisiblePoint == 0 )
	{
		CalcMaxVisiblePoint();
	}
	DrawSun( fElpasedTime );
}

void CEtLensFlare::DrawSun( float fElpasedTime )
{
	if( m_dwMaxVisiblePoint == 0 ) {
		return;
	}

	EtCameraHandle hCamera;
	EtVector3 ViewLightDir, ViewLightPos;
	float fHalfSunHeight, fHalfFlareHeight, fFlareAlpha;
	DWORD dwColor, dwResult;

	m_Query.GetResult( dwResult );
	fFlareAlpha = dwResult / ( float )m_dwMaxVisiblePoint;
	fFlareAlpha = min( 1.0f, fFlareAlpha );

	float fAdd = (fFlareAlpha - m_fFlareAlphaSmooth ) * EtMin(1.0f, fElpasedTime * 20.0f );
	fAdd = EtClamp( fAdd, -0.03f, 0.03f);
	m_fFlareAlphaSmooth += fAdd;
	//m_fFlareAlphaSmooth = max( min( m_fFlareAlphaSmooth, 1.0f ), 0.0f ); // Added in case values like 1.000001 occur due to floating-point error..
	int nAlpha = EtClamp( (int)(m_fFlareAlphaSmooth * 255), 0, 255 );
	dwColor = 0xffffff | ( ((DWORD)nAlpha) << 24 );
	if( m_pLightShaftFilter )
	{
		if( fFlareAlpha <= 0.0f )
		{
			m_pLightShaftFilter->Enable( false );
		}
		else
		{
			m_pLightShaftFilter->Enable( true );
		}
	}

	hCamera = CEtCamera::GetActiveCamera();
	EtVec3TransformNormal( &ViewLightDir, &m_LightDir, hCamera->GetViewMat() );
	ViewLightPos = -ViewLightDir * sqrtf( 3 );
	EtVec3TransformCoord( &ViewLightPos, &ViewLightPos, &m_FlareProjMat );
	if( ViewLightPos.z <= 0.0f )
	{
		return;
	}
	ViewLightPos.x = ( ViewLightPos.x + 1.0f ) * 0.5f;
	ViewLightPos.y = 1.0f - ( ViewLightPos.y + 1.0f ) * 0.5f;
	fHalfSunHeight = m_fSunRadius / m_fAspectRatio;
	fHalfFlareHeight = m_fFlareRadius / m_fAspectRatio;

	SUICoord UVCoord, ScreenCoord;

	UVCoord.SetCoord( 0.0f, 0.0f, 1.0f, 1.0f );
	ScreenCoord.SetCoord( ViewLightPos.x - m_fSunRadius, ViewLightPos.y - fHalfSunHeight, m_fSunRadius * 2.0f, fHalfSunHeight * 2.0f );
	
	m_Query.Begin();
	
	GetEtDevice()->SetColorWriteEnable( 0 );
	DrawImage( m_hSunTexture, UVCoord, 0xffffffff, ScreenCoord, 0.999f );
	
	GetEtDevice()->SetColorWriteEnable( CW_ALL );
	m_Query.End();

	if( m_pLightShaftFilter )
	{
		m_pLightShaftFilter->SetSunPosition( EtVector2( ScreenCoord.fX, ScreenCoord.fY ) );
	}

	if( fFlareAlpha == 0.0f ) {
		return;
	}

	ScreenCoord.SetCoord( ViewLightPos.x - m_fFlareRadius, ViewLightPos.y - fHalfFlareHeight, m_fFlareRadius * 2.0f, fHalfFlareHeight * 2.0f );

	if( m_hFlareTexture ) 
	{
		bool bWriteZ = GetEtDevice()->EnableZWrite( false );
		bool bEnableAlpha = GetEtDevice()->EnableAlphaBlend( true );
		DrawImage( m_hFlareTexture, UVCoord, dwColor, ScreenCoord, 0.0f );
		GetEtDevice()->EnableAlphaBlend( bEnableAlpha );
		GetEtDevice()->EnableZWrite( bWriteZ );
	}
}

void CEtLensFlare::DrawImage( EtTextureHandle hTexture, SUICoord UVCoord, DWORD dwColor, SUICoord ScreenCoord,  float fZValue )
{
	STextureDiffuseVertex2D Vertices[ 4 ];

	GetEtDevice()->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
	GetEtDevice()->SetPixelShader( NULL );
	GetEtDevice()->SetVertexShader( NULL );

	DWORD dwWidth = GetEtDevice()->Width();
	DWORD dwHeight = GetEtDevice()->Height();

	Vertices[ 0 ].Position = EtVector4( ScreenCoord.fX , ScreenCoord.fY, fZValue, 1.0f );
	Vertices[ 1 ].Position = EtVector4( (ScreenCoord.fX+ScreenCoord.fWidth), ScreenCoord.fY , fZValue, 1.0f );
	Vertices[ 2 ].Position = EtVector4( (ScreenCoord.fX+ScreenCoord.fWidth) , (ScreenCoord.fY+ScreenCoord.fHeight), fZValue, 1.0f );
	Vertices[ 3 ].Position = EtVector4( ScreenCoord.fX , (ScreenCoord.fY+ScreenCoord.fHeight) , fZValue, 1.0f );

	for( int i = 0; i < 4; i++ )  {
		Vertices[ i ].Position.x *= dwWidth;
		Vertices[ i ].Position.y *= dwHeight;
	}

	Vertices[ 0 ].TexCoordinate = EtVector2( UVCoord.fX, UVCoord.fY );
	Vertices[ 1 ].TexCoordinate = EtVector2( UVCoord.fX+UVCoord.fWidth, UVCoord.fY );
	Vertices[ 2 ].TexCoordinate = EtVector2( UVCoord.fX+UVCoord.fWidth, UVCoord.fY + UVCoord.fHeight );
	Vertices[ 3 ].TexCoordinate = EtVector2( UVCoord.fX, UVCoord.fY + UVCoord.fHeight );
	Vertices[ 0 ].Color = dwColor;
	Vertices[ 1 ].Color = dwColor;
	Vertices[ 2 ].Color = dwColor;
	Vertices[ 3 ].Color = dwColor;

	GetEtDevice()->SetTexture( 0, hTexture->GetTexturePtr() );

	GetEtDevice()->DrawPrimitiveUP( PT_TRIANGLEFAN, 2, Vertices, sizeof( STextureDiffuseVertex2D ) );
}

void CEtLensFlare::CalcMaxVisiblePoint()
{
	SUICoord UVCoord, ScreenCoord;
	float fHalfSunHeight;

	fHalfSunHeight = m_fSunRadius / m_fAspectRatio;
	UVCoord.SetCoord( 0.0f, 0.0f, 1.0f, 1.0f );
	ScreenCoord.SetCoord( 0.5f - m_fSunRadius, 0.5f - fHalfSunHeight, m_fSunRadius * 2.0f, fHalfSunHeight * 2.0f );
	
	if( m_hSunTexture ) {
		m_StartQuery.Begin();
		GetEtDevice()->SetColorWriteEnable( 0 );
		DrawImage( m_hSunTexture, UVCoord, 0xffffffff, ScreenCoord, 0.0f );
		GetEtDevice()->SetColorWriteEnable( CW_ALL );
		m_StartQuery.End();
	}

	m_StartQuery.GetResult( m_dwMaxVisiblePoint );
}

void CEtLensFlare::SetFlareRadius( float fSunRadius, float fFlareRadius )
{
	m_fSunRadius = fSunRadius;
	m_fFlareRadius = fFlareRadius;
	m_dwMaxVisiblePoint = 0;
}

void CEtLensFlare::SetDirection( EtVector3 &LightDir )
{
	m_LightDir = LightDir;
}

void CEtLensFlare::SetSunTexture( const char *pSunTextureName )
{
	EtTextureHandle hTemp = m_hSunTexture;
	m_hSunTexture = LoadResource( pSunTextureName, RT_TEXTURE );
	SAFE_RELEASE_SPTR( hTemp );
}

void CEtLensFlare::SetFlareTexture( const char *pFlareTextureName )
{
	EtTextureHandle hTemp = m_hFlareTexture;
	m_hFlareTexture = LoadResource( pFlareTextureName, RT_TEXTURE );
	SAFE_RELEASE_SPTR( hTemp );
}

EtLensFlareHandle CEtLensFlare::CreateLensFlare( EtVector3 &LightDir, const char *pSunTextureName, const char *pFlareTextureName )
{
	CEtLensFlare *pFlare;

	pFlare = new CEtLensFlare();
	pFlare->Initialize( LightDir, pSunTextureName, pFlareTextureName );

	return pFlare->GetMySmartPtr();
}

void CEtLensFlare::RenderFlareList( float fElpasedTime )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	int i, nCount;

	nCount = GetItemCount();
	for( i = 0; i < nCount; i++ )
	{
		GetItem( i )->Render( fElpasedTime );
	}
}

void CEtLensFlare::OnLostDevice()
{
	m_dwMaxVisiblePoint = 0;
}
