#include "StdAfx.h"
#include "DnFishingProgressBar.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnFishingProgressBar::CDnFishingProgressBar( CEtUIDialog* pParent )
: CEtUIProgressBar( pParent )
, m_fSpeedUpStart( 0.0f )
, m_fSpeedUpEnd( 0.0f )
{
	m_fProgress = 0.0f;
}

CDnFishingProgressBar::~CDnFishingProgressBar()
{
}

void CDnFishingProgressBar::SetSpeedUpSection( float fSpeedUpStart, float fSpeedUpEnd )
{
	m_fSpeedUpStart = fSpeedUpStart;
	m_fSpeedUpEnd = fSpeedUpEnd;
}

void CDnFishingProgressBar::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	SUIElement *pElement( NULL );
	SUICoord ProgressCoord, UVCoord;

	EtTextureHandle hTexture = m_pParent->GetUITexture();
	if( m_bExistTemplateTexture )
		hTexture = m_Template.m_hTemplateTexture;

	pElement = GetElement( 0 );
	UVCoord = pElement->UVCoord;
	if( m_bExistTemplateTexture )
		UVCoord = pElement->TemplateUVCoord;
	m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, m_Property.UICoord, 0.f, m_fZValue );

	if( m_bSmooth )
	{
		if( !m_bUseLazy || GetTickCount() - m_dwTick > 1000 )
			m_fProgress += (m_fProgressSmooth - m_fProgress ) * min(1.0f, ( fElapsedTime * 7.f ));
	}

	if( m_fProgress <= 0.0f )
		return;

	if( m_bUseLayer && m_nProgressIndex > 0 )
	{
		pElement = GetElement( 1 + m_nProgressIndex - 1);
		UVCoord = pElement->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement->TemplateUVCoord;
		ProgressCoord = m_ProgressCoord;
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
	}

	SUIElement* pElement1;
	SUIElement* pElement2;
	pElement1 = GetElement( 1 );
	pElement2 = GetElement( 2 );

	if( pElement1 == NULL || pElement2 == NULL )
		return;

	if( m_fSpeedUpStart > m_fSpeedUpEnd )
		return;

	bool bRenderLeft, bRenderMiddle, bRenderRight;
	bRenderLeft = bRenderMiddle = bRenderRight = false;

	if( m_fProgress < m_fSpeedUpStart )
		bRenderLeft = true;
	else if( m_fProgress >= m_fSpeedUpStart && m_fProgress < m_fSpeedUpEnd )
		bRenderLeft = bRenderMiddle = true;
	else if( m_fProgress >= m_fSpeedUpEnd )
		bRenderLeft = bRenderMiddle = bRenderRight = true;

	if( bRenderLeft )
	{
		UVCoord = pElement1->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement1->TemplateUVCoord;
		UVCoord.fWidth *= (m_fProgress * 0.01f);
		ProgressCoord = m_ProgressCoord;
		ProgressCoord.fWidth *= (m_fProgress * 0.01f);
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
	}

	if( bRenderMiddle )
	{
		UVCoord = pElement2->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement2->TemplateUVCoord;
		UVCoord.fX = UVCoord.fX + ( UVCoord.fWidth * m_fSpeedUpStart / 100.0f );
		UVCoord.fWidth *= ((m_fProgress - m_fSpeedUpStart) * 0.01f);
		ProgressCoord = m_ProgressCoord;
		ProgressCoord.fWidth *= ((m_fProgress - m_fSpeedUpStart) * 0.01f);
		ProgressCoord.fX = m_ProgressCoord.fX + ( m_ProgressCoord.fWidth * m_fSpeedUpStart / 100.0f );
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
	}

	if( bRenderRight )
	{
		UVCoord = pElement1->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement1->TemplateUVCoord;
		UVCoord.fX = UVCoord.fX + ( UVCoord.fWidth * m_fSpeedUpEnd / 100.0f );
		UVCoord.fWidth *= ((m_fProgress - m_fSpeedUpEnd) * 0.01f);
		ProgressCoord = m_ProgressCoord;
		ProgressCoord.fWidth *= ((m_fProgress - m_fSpeedUpEnd) * 0.01f);
		ProgressCoord.fX = m_ProgressCoord.fX + ( m_ProgressCoord.fWidth * m_fSpeedUpEnd / 100.0f );
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
	}
}
