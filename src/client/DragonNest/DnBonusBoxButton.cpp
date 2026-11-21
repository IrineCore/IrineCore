#include "StdAfx.h"
#include "DnBonusBoxButton.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBonusBoxButton::CDnBonusBoxButton( CEtUIDialog *pParent )
	: CEtUIRadioButton( pParent )
	, m_emBonusBoxType(typeQuestion)
	, m_bItemLight(false)
	, m_bHandle(true)
	, m_bRPressed( false )
{
}

CDnBonusBoxButton::~CDnBonusBoxButton(void)
{
}

void CDnBonusBoxButton::Render( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;

	if( m_bHandle )
	{
		if( !IsShow() )
		{
			m_CurrentState = UI_STATE_HIDDEN;
		}
		else if( !IsEnable() )
		{
			m_CurrentState = UI_STATE_DISABLED;
		}
		else if( m_bPressed )
		{
			m_CurrentState = UI_STATE_PRESSED;
		}
		else if( m_bMouseEnter )
		{
			m_CurrentState = UI_STATE_MOUSEENTER;
		}
		else if( m_bFocus )
		{
			m_CurrentState = UI_STATE_FOCUS;
		}
	}
	else
	{
		if( !IsShow() )
		{
			m_CurrentState = UI_STATE_HIDDEN;
		}
		else if( !IsEnable() )
		{
			m_CurrentState = UI_STATE_DISABLED;
		}
	}

	UpdateBlendRate();

	SUIElement *pElement(NULL);

	if( !m_bChecked )
	{
		switch( m_emBonusBoxType )
		{
		case typeQuestion:
			pElement = GetElement(BonusBox);
			if( m_CurrentState == UI_STATE_MOUSEENTER )
			{
				pElement = GetElement(BonusSelect);
			}
			break;
		case typeBronze:
			pElement = GetElement(BronzeBox);
			break;
		case typeBronzeOpen:
			pElement = GetElement(BronzeBoxOpen);
			break;
		case typeSilver:
			pElement = GetElement(SilverBox);
			break;
		case typeSilverOpen:
			pElement = GetElement(SilverBoxOpen);
			break;
		case typeGold:
			pElement = GetElement(GoldBox);
			break;
		case typeGoldOpen:
			pElement = GetElement(GoldBoxOpen);
			break;
		case typePlatinum:
			pElement = GetElement(PlatinumBox);
			break;
		case typePlatinumOpen:
			pElement = GetElement(PlatinumBoxOpen);
			break;
		}
	}
	else
	{
		switch( m_emBonusBoxType )
		{
		case typeQuestion:
			pElement = GetElement(BonusSelect);
			break;
		case typeBronze:
			pElement = GetElement(BronzeBoxSelect);
			break;
		case typeBronzeOpen:
			pElement = GetElement(BronzeBoxOpenSelect);
			break;
		case typeSilver:
			pElement = GetElement(SilverBoxSelect);
			break;
		case typeSilverOpen:
			pElement = GetElement(SilverBoxOpenSelect);
			break;
		case typeGold:
			pElement = GetElement(GoldBoxSelect);
			break;
		case typeGoldOpen:
			pElement = GetElement(GoldBoxOpenSelect);
			break;
		case typePlatinum:
			pElement = GetElement(PlatinumBoxSelect);
			break;
		case typePlatinumOpen:
			pElement = GetElement(PlatinumBoxOpenSelect);
			break;
		}
	}

	if( !pElement )
		return;

	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_ButtonCoord );

	if( m_bItemLight )
	{
		pElement = GetElement( ItemLight );
		if( pElement )
		{
			LOCAL_TIME time = CGlobalInfo::GetInstance().m_LocalTime;

			int nAlpha = 255;
			int BLEND_TIME = 500;
			float BLEND_RANGE  = 1.0f;
			int nTemp = (int)time%(BLEND_TIME+BLEND_TIME);
			if( nTemp < BLEND_TIME ) 
			{
				nAlpha = int(( BLEND_RANGE / (float)BLEND_TIME ) * nTemp * 255.0f);
			}
			else 
			{
				nAlpha = int(( BLEND_RANGE / (float)BLEND_TIME ) * ( (BLEND_TIME+BLEND_TIME) - nTemp ) *  255.0f);
			}

			nAlpha = min(nAlpha, 255);
			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, D3DCOLOR_ARGB(nAlpha,255,255,255), m_ButtonCoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, D3DCOLOR_ARGB(nAlpha,255,255,255), m_ButtonCoord );
		}
	}
}

bool CDnBonusBoxButton::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !m_bHandle )
		return false;

	return CEtUIRadioButton::HandleKeyboard( uMsg, wParam, lParam );
}

bool CDnBonusBoxButton::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !m_bHandle )
		return false;

	switch( uMsg )
	{
	case WM_RBUTTONDOWN:
	//case WM_RBUTTONDBLCLK: // No immediate need to handle right double-click and there are SetCapture issues, so block for now.
		{
			if( IsInside( fX, fY ) )
			{
				m_bRPressed = true;
				SetCapture( m_pParent->GetHWnd() );
				if( !m_bFocus )
				{
					m_pParent->RequestFocus( this );
				}
				return true;
			}

			break;
		}

	case WM_RBUTTONUP:
		{
			if( m_bRPressed )
			{
				m_bRPressed = false;
				ReleaseCapture();
				if( IsInside( fX, fY ) )
				{
					m_pParent->ProcessCommand( EVENT_BUTTON_RCLICKED, true, this );
					if( m_nSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndex );
				}
				return true;
			}
			break;
		}
	}

	return CEtUIRadioButton::HandleMouse( uMsg, fX, fY, wParam, lParam );
}

void CDnBonusBoxButton::SetUseHandle( bool bHandle )
{
	m_bHandle = bHandle;

	// Added to fix the issue where tooltips do not appear while holding down on a reward chest in stage rewards.
	if( !bHandle )
	{
		if( m_bPressed )
		{
			m_bPressed = false;
			ReleaseCapture();
		}
		if( m_bRPressed )
		{
			m_bRPressed = false;
			ReleaseCapture();
		}
	}
}