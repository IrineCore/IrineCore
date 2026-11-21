#include "StdAfx.h"
#include "EtUITextureControl.h"
#include "EtUIDialog.h"
#include "EtUIXML.h"

// These two are files from another project so I tried not to use them, but
// It turns out EtUIDialog also uses this when loading dialog textures.
// So decided to just use it.
#include "EtLoader.h"
#include "EtResourceMng.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CEtUITextureControl::CEtUITextureControl( CEtUIDialog *pParent )
	: CEtUIControl( pParent )
{
	m_fBlendRate = 0.8f;
	m_Scale = 1.f;
	m_bManualControlColor = false;
	m_bCreatedFromPropertyTextureFile = false;

	m_fAddX = 0.0f;
	m_fAddY = 0.0f;
}

CEtUITextureControl::~CEtUITextureControl(void)
{
	//DeleteTexture();
	if( m_bCreatedFromPropertyTextureFile )
		SAFE_RELEASE_SPTR( m_hTexture );
}

void CEtUITextureControl::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	if( ( pProperty ) && ( pProperty->StaticProperty.nStringIndex > 0 ) )
	{
		SetText( m_pParent->GetUIString( CEtUIXML::idCategory1, pProperty->StaticProperty.nStringIndex ) );
	}

	m_Scale = 1.f;

	if( ( pProperty ) && ( pProperty->TextureControlProperty.szTextureFileName[0] != '\0' ))
	{
		if( m_bCreatedFromPropertyTextureFile )
			SAFE_RELEASE_SPTR( m_hTexture );
		m_hTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( pProperty->TextureControlProperty.szTextureFileName ).c_str(), RT_TEXTURE );
		
		m_TextureUV.SetCoord( 0.0f, 0.0f, 1.0f, 1.0f );
		m_fAddX = m_fAddY = 0.0f;
		UpdateRects();

		m_bCreatedFromPropertyTextureFile = true;
	}
}

void CEtUITextureControl::Process( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;

	if( !IsShow() )
	{
		m_CurrentState = UI_STATE_HIDDEN;
	}
	else if( !IsEnable() )
	{
		m_CurrentState = UI_STATE_DISABLED;
	}

	if (m_bManualControlColor == false)
	{
		SUIElement *pElement = GetElement(0);
		if( !pElement ) return;

		pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	}
}

DWORD CEtUITextureControl::GetTextureColor()
{
	SUIElement *pElement = GetElement(0);
	if( !pElement ) return 0;

	return pElement->TextureColor.dwColor[UI_STATE_NORMAL];
}

void CEtUITextureControl::SetTextureColor(DWORD dwColor)
{
	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	pElement->TextureColor.dwCurrentColor = dwColor;
}

void CEtUITextureControl::Render( float fElapsedTime )
{
	if( !m_hTexture )
		return;

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	SUICoord renderCoord(m_ControlCoord);
	renderCoord.fWidth *= m_Scale;
	renderCoord.fHeight *= m_Scale;

	m_pParent->DrawSprite( m_hTexture, m_TextureUV, pElement->TextureColor.dwCurrentColor, renderCoord );
	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, renderCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, renderCoord );
	m_pParent->DrawDlgText( m_szText.c_str(), pElement, pElement->FontColor.dwCurrentColor, m_NewUICoord, -1, m_Property.TextureControlProperty.dwFontFormat );
}

void CEtUITextureControl::SetTexturePosition(float fX, float fY)
{
	m_ControlCoord.SetPosition(fX, fY);
}

void CEtUITextureControl::UpdateRects()
{
	m_NewUICoord = m_Property.UICoord;
	m_NewUICoord.fX += 0.5f * m_fAddX * m_Property.UICoord.fWidth;
	m_NewUICoord.fY += 0.5f * m_fAddY * m_Property.UICoord.fHeight;
	m_NewUICoord.fWidth *= (1.0f - m_fAddX);
	m_NewUICoord.fHeight *= (1.0f - m_fAddY);
	//m_ControlCoord.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, m_TextureCoord.fWidth / m_pParent->GetScreenWidth(), m_Property.UICoord.fHeight );
	m_ControlCoord.SetCoord( m_NewUICoord.fX, m_NewUICoord.fY, m_NewUICoord.fWidth, m_NewUICoord.fHeight );
}

void CEtUITextureControl::SetTexture( EtTextureHandle hTextureHandle )
{
	if( !hTextureHandle ) return;

	int nWidth = hTextureHandle->Width();
	int nHeight = hTextureHandle->Height();
	return SetTexture( hTextureHandle, 0, 0, nWidth, nHeight);
}

void CEtUITextureControl::SetTexture( EtTextureHandle hTextureHandle, int nX, int nY, int nWidth, int nHeight )
{
	if( !hTextureHandle )
	{
		//CDebugSet::ToLogFile( "CEtUITextureControl::SetTexture, hTextureHandle is NULL!" );
		return;
	}

	// If currently using the texture passed in Property,
	// When a new texture setting arrives, clear the previously used texture and turn off the flag.
	if( m_bCreatedFromPropertyTextureFile )
	{
		SAFE_RELEASE_SPTR( m_hTexture );
		m_bCreatedFromPropertyTextureFile = false;
	}

	//DeleteTexture();

	// It is just a pointer copy. There is no reference counter increment or anything.
	m_hTexture = hTextureHandle;
	/*m_TextureCoord.SetCoord( (float)nX, (float)nY, (float)nWidth, (float)nHeight );*/

	m_TextureUV.SetCoord( nX / ( float )hTextureHandle->Width(), 
						 nY / ( float )hTextureHandle->Height(), 
						 nWidth / ( float )hTextureHandle->Width(),
						 nHeight / ( float )hTextureHandle->Height());


	m_fAddX = 0.f;
	m_fAddY = 0.f;
	if( m_TextureUV.fX < 0.f ) {
		m_fAddX = -m_TextureUV.fX;
		m_TextureUV.fX = 0.f;
	}
	if( m_TextureUV.fY < 0.f ) {
		m_fAddY = -m_TextureUV.fY;
		m_TextureUV.fY = 0.f;
	}

	if( m_TextureUV.fX+m_TextureUV.fWidth > 1.f ) {
		m_fAddX += m_TextureUV.fWidth - (1 - m_TextureUV.fX);
		m_TextureUV.fWidth = 1 - m_TextureUV.fX;
	}
	if( m_TextureUV.fY+m_TextureUV.fHeight > 1.f ) {
		m_fAddY += m_TextureUV.fHeight - (1 - m_TextureUV.fY);
		m_TextureUV.fHeight = 1 - m_TextureUV.fY;
	}
	
	UpdateRects();
}

void CEtUITextureControl::SetFlipHTexture( EtTextureHandle hTextureHandle, int nX, int nY, int nWidth, int nHeight, bool bFlipH /*= false*/)
{
	if( !hTextureHandle )
	{
		//CDebugSet::ToLogFile( "CEtUITextureControl::SetTexture, hTextureHandle is NULL!" );
		return;
	}

	// If currently using the texture passed in Property,
	// When a new texture setting arrives, clear the previously used texture and turn off the flag.
	if( m_bCreatedFromPropertyTextureFile )
	{
		SAFE_RELEASE_SPTR( m_hTexture );
		m_bCreatedFromPropertyTextureFile = false;
	}

	//DeleteTexture();

	// It is just a pointer copy. There is no reference counter increment or anything.
	m_hTexture = hTextureHandle;
	/*m_TextureCoord.SetCoord( (float)nX, (float)nY, (float)nWidth, (float)nHeight );*/

	m_TextureUV.SetCoord(	nX / ( float )hTextureHandle->Width(), 
							nY / ( float )hTextureHandle->Height(), 
							nWidth / ( float )hTextureHandle->Width(),
							nHeight / ( float )hTextureHandle->Height() );

	if(bFlipH)
		m_TextureUV.fWidth = 3.0f;

	m_fAddX = 0.f;
	m_fAddY = 0.f;
	if( m_TextureUV.fX < 0.f ) {
		m_fAddX = -m_TextureUV.fX;
		m_TextureUV.fX = 0.f;
	}
	if( m_TextureUV.fY < 0.f ) {
		m_fAddY = -m_TextureUV.fY;
		m_TextureUV.fY = 0.f;
	}

	if( m_TextureUV.fX+m_TextureUV.fWidth > 1.f ) {
		m_fAddX += m_TextureUV.fWidth - (1 - m_TextureUV.fX);
		m_TextureUV.fWidth = 1 - m_TextureUV.fX;
	}
	if( m_TextureUV.fY+m_TextureUV.fHeight > 1.f ) {
		m_fAddY += m_TextureUV.fHeight - (1 - m_TextureUV.fY);
		m_TextureUV.fHeight = 1 - m_TextureUV.fY;
	}

	UpdateRects();
}

void CEtUITextureControl::DeleteTexture()
{
	SAFE_RELEASE_SPTR( m_hTexture );
}

void CEtUITextureControl::Show( bool bShow )
{
	if (bShow == false && m_bManualControlColor)
		m_bManualControlColor = false;

	CEtUIControl::Show(bShow);
}