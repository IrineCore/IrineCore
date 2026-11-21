#include "StdAfx.h"
#include "DnComboCount.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnComboCount::CDnComboCount(CEtUIDialog *pParent)
	: CDnCountCtl( pParent )
{
	m_fFontSize = 1.0f;
	m_nRedIndex = 0;
}

CDnComboCount::~CDnComboCount(void)
{
}

void CDnComboCount::Render( float fElapsedTime )
{
	SUIElement *pElement(NULL);
	SUICoord uiCoord;
	float fWidth(0.0f);
	int nIndex(-1);

	UI_CONTROL_STATE CurrentState = UI_STATE_NORMAL;
	if( !IsShow() ) CurrentState = UI_STATE_HIDDEN;
	m_Color.Blend( CurrentState, fElapsedTime, 0.8f );

	for( int i=(int)m_vecValueIndex.size()-1; i>=0 ; i-- )
	{
		nIndex = m_vecValueIndex[i];
		pElement = GetElement(nIndex +m_nRedIndex );
		if( pElement == NULL ) continue;

		uiCoord = m_Property.UICoord;
		uiCoord.fX += fWidth;
		
		//if( m_nRedIndex == 11 )
		//{
			uiCoord.SetSize( pElement->fTextureWidth * m_fFontSize, pElement->fTextureHeight * m_fFontSize );
			fWidth += pElement->fTextureWidth * m_fFontSize;
		//}

//		temp by kalliste
// 		memcpy(&m_Color, &pElement->TextureColor.dwColor, sizeof(SUIColor));
// 		m_Color.Blend( CurrentState, fElapsedTime, 0.8f );

		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, m_Color.dwCurrentColor, uiCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, m_Color.dwCurrentColor, uiCoord );
	}

	pElement = GetElement(10);
	if( pElement == NULL ) return;

	uiCoord = m_Property.UICoord;
	uiCoord.fX += fWidth;

	//if( m_nRedIndex == 11 )
	//{
		uiCoord.SetSize( pElement->fTextureWidth * m_fFontSize, pElement->fTextureHeight * m_fFontSize );
	//}
	//else
	//{
		//uiCoord.SetSize( pElement->fTextureWidth * COUNT_YELLOW_RATIO, pElement->fTextureHeight * COUNT_YELLOW_RATIO );
	//}

//	temp by kalliste
// 	memcpy(&m_Color, &pElement->TextureColor.dwColor, sizeof(SUIColor));
// 	m_Color.Blend( CurrentState, fElapsedTime, 0.8f );

	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, m_Color.dwCurrentColor, uiCoord );//m_Color.dwColor[UI_STATE_NORMAL], uiCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, m_Color.dwCurrentColor, uiCoord );//m_Color.dwColor[UI_STATE_NORMAL], uiCoord );
}

float CDnComboCount::GetControlWidth()
{
	float fWidth = CDnCountCtl::GetControlWidth();

	SUIElement *pElement = GetElement(10);
	fWidth += pElement->fTextureWidth;

	return fWidth;
}

float CDnComboCount::GetControlHeight()
{
	float fHeight = CDnCountCtl::GetControlHeight();

	SUIElement *pElement = GetElement(10);
	if( fHeight < pElement->fTextureHeight )
	{
		fHeight = pElement->fTextureHeight;
	}

	return fHeight;
}

void CDnComboCount::SetValue( int nValue )
{
	CDnCountCtl::SetValue( nValue );

	// Combo counter is a screen-coordinate UI; numbers >= 8 are displayed in red.
	/*if( nValue >= 8 )
	{
		m_nRedIndex = 11;
	}
	else {
		m_nRedIndex = 0;
	}
	*/
}
