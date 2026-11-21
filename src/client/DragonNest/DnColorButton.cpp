#include "StdAfx.h"
#include "DnColorButton.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnColorButton::CDnColorButton( CEtUIDialog *pParent )
: CEtUIButton( pParent )
{
}

CDnColorButton::~CDnColorButton(void)
{
}

void CDnColorButton::Render( float fElapsedTime )
{
	CEtUIButton::Render( fElapsedTime );

	// Rendering like this will draw over the button text, but
	// Color buttons will not have text, and overriding everything would just be code duplication so skipping.
	SUIElement *pElement = GetElement( 1 );
	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
}

void CDnColorButton::MouseEnter( bool bEnter )
{
	// Do not change to Enter state.
	return;
}