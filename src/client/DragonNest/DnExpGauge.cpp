#include "StdAfx.h"
#include "DnExpGauge.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnExpGauge::CDnExpGauge(CEtUIDialog *pParent)
	: CEtUIProgressBar( pParent )
{
	m_fProgress = 0.0f;
	m_bFlipH = false;
}

CDnExpGauge::~CDnExpGauge(void)
{
}

void CDnExpGauge::Render( float fElapsedTime )
{
	if( !IsShow() ) {
		return;
	}

	if( m_fProgress <= 0.0f )
		return;

	SUIElement *pElement(NULL);
	SUICoord ProgressCoord, UVCoord;

	EtTextureHandle hTexture = m_pParent->GetUITexture();
	if( m_bExistTemplateTexture )
		hTexture = m_Template.m_hTemplateTexture;

	if( m_bFlipH == false )
	{
		pElement = GetElement(0);
		UVCoord = pElement->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement->TemplateUVCoord;
		UVCoord.fWidth *= (m_fProgress * 0.01f);
		ProgressCoord = m_ProgressCoord;
		ProgressCoord.fWidth *= (m_fProgress * 0.01f);
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord );

		// EXP gauge reportedly needs shading too.
		if( m_nLightShadeElementIndex > 0 ) {
			pElement = GetElement( m_nLightShadeElementIndex );
			if( pElement ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fWidth *= (m_fProgress * 0.01f);
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
			}
		}
		if( m_nDarkShadeElementIndex > 0 ) {
			pElement = GetElement( m_nDarkShadeElementIndex );
			if( pElement ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fX += (ProgressCoord.fWidth * (m_fProgress * 0.01f));
				ProgressCoord.fWidth *= ((100.0f-m_fProgress) * 0.01f);
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
			}
		}

		if( m_nGaugeEndElementIndex > 0 ) {
			pElement = GetElement( m_nGaugeEndElementIndex );
			if( pElement && m_fProgress < 100.0f ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				SUICoord StarCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fWidth *= (m_fProgress * 0.01f);
				StarCoord.SetPosition( ProgressCoord.Right()-(pElement->fTextureWidth/2.0f), ProgressCoord.fY-((pElement->fTextureHeight-ProgressCoord.fHeight)/2.0f) );
				StarCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, StarCoord );
			}
		}
	}
	else
	{
		// Draw flipped horizontally.
		pElement = GetElement(0);
		UVCoord = pElement->UVCoord;
		if( m_bExistTemplateTexture )
			UVCoord = pElement->TemplateUVCoord;
		UVCoord.fWidth *= (m_fProgress * 0.01f);
		if( hTexture )
			UVCoord.fWidth = (int)(UVCoord.fWidth * hTexture->Width()) / (float)hTexture->Width();
		if( m_bExistTemplateTexture )
			UVCoord.fX = UVCoord.fX + pElement->TemplateUVCoord.fWidth - UVCoord.fWidth;
		else
			UVCoord.fX = UVCoord.fX + pElement->UVCoord.fWidth - UVCoord.fWidth;
		ProgressCoord = m_ProgressCoord;
		ProgressCoord.fWidth *= (m_fProgress * 0.01f);
		ProgressCoord.fX = ProgressCoord.fX + m_ProgressCoord.fWidth - ProgressCoord.fWidth;
		m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord );

		// EXP gauge reportedly needs shading too.
		if( m_nLightShadeElementIndex > 0 ) {
			pElement = GetElement( m_nLightShadeElementIndex );
			if( pElement ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fWidth *= (m_fProgress * 0.01f);
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
			}
		}
		if( m_nDarkShadeElementIndex > 0 ) {
			pElement = GetElement( m_nDarkShadeElementIndex );
			if( pElement ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fX += (ProgressCoord.fWidth * (m_fProgress * 0.01f));
				ProgressCoord.fWidth *= ((100.0f-m_fProgress) * 0.01f);
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, ProgressCoord, 0.f, m_fZValue );
			}
		}

		if( m_nGaugeEndElementIndex > 0 ) {
			pElement = GetElement( m_nGaugeEndElementIndex );
			if( pElement && m_fProgress < 100.0f ) {
				UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;
				SUICoord StarCoord;
				ProgressCoord = m_ProgressCoord;
				ProgressCoord.fWidth *= (m_fProgress * 0.01f);
				ProgressCoord.fX = ProgressCoord.fX + m_ProgressCoord.fWidth - ProgressCoord.fWidth;
				StarCoord.SetPosition( ProgressCoord.fX-(pElement->fTextureWidth/2.0f), ProgressCoord.fY-((pElement->fTextureHeight-ProgressCoord.fHeight)/2.0f) );
				StarCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
				m_pParent->DrawSprite( hTexture, UVCoord, 0xffffffff, StarCoord );
			}
		}
	}
}