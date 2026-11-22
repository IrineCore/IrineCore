#include "StdAfx.h"
#include "DnZoneNPCButton.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnZoneNPCButton::CDnZoneNPCButton( CEtUIDialog *pParent )
	: CEtUIButton( pParent )
	, m_emQuestState(buttonQuestNone)
	, m_emMailState(buttonMailNone)
	, m_bPushPin(false)
	, m_vWorldPos( 0.f, 0.f )
	, m_nUnitSize(0)
	, m_vDirection( 0.f, 0.f )
	, m_nNpcID(0)
{
}

CDnZoneNPCButton::~CDnZoneNPCButton(void)
{
}

bool CDnZoneNPCButton::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_RBUTTONDOWN:
		{
			if( IsInside( fX, fY ) )
			{
				m_bPressed = true;
				SetCapture( m_pParent->GetHWnd() );
				if( !m_bFocus )
				{
					m_pParent->RequestFocus( this );
				}
				return true;
			}
			m_MouseCoord.fX = fX;
			m_MouseCoord.fY = fY;
		}
		break;
	case WM_RBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();

				if( IsInside( fX, fY ) )
				{
					m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, uMsg );
				}
				return true;
			}
		}
		break;
	}

	return CEtUIButton::HandleMouse( uMsg, fX, fY, wParam, lParam );
}

void CDnZoneNPCButton::Render( float fElapsedTime )
{
	/*if( !IsMouseEnter() && m_bPressed )
	{
		m_bPressed = false;
		ReleaseCapture();
	}*/

	m_CurrentState = UI_STATE_NORMAL;
	m_MoveButtonCoord = m_Property.UICoord;

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

		m_MoveButtonCoord.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveButtonCoord.fY += m_Property.ButtonProperty.fMoveOffsetVert;
	}
	else if( m_bMouseEnter )
	{
		m_CurrentState = UI_STATE_MOUSEENTER;
	}
	else if( m_bFocus )
	{
		m_CurrentState = UI_STATE_FOCUS;
	}

	UpdateBlendRate();

	SUIElement *pElement(NULL);

	if( m_Property.ButtonProperty.ButtonType == UI_BUTTON_CLICK )
	{
		pElement = GetElement(0);

		if( ( m_CurrentState == UI_STATE_MOUSEENTER ) && ( m_Template.m_vecElement.size() >= 2 ) )
		{
			pElement = GetElement(1);
		}
	}

	if( !pElement )
		return;

	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
	m_pParent->DrawDlgText( m_szText.c_str(), pElement, pElement->FontColor.dwCurrentColor, m_MoveButtonCoord );

	bool bMarkEnable = false;
	if( m_emQuestState >= buttonQuestAvailable )
	{
		pElement = GetElement(m_emQuestState);
		bMarkEnable = true;
	}

	if (m_emMailState >= buttonMailNew)
	{
		pElement = GetElement(m_emMailState);
		bMarkEnable = true;
	}

	if (bMarkEnable)
	{
		if( pElement ) {
			SUICoord questCoord;
			questCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
			questCoord.fX = (m_MoveButtonCoord.fX + m_MoveButtonCoord.fWidth/2.0f) - ( questCoord.fWidth/2.0f );
			questCoord.fY = m_MoveButtonCoord.fY - questCoord.fHeight;

			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, questCoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, questCoord );
		}
	}

	if( m_bPushPin )
	{
		pElement = GetElement(5);

		if( pElement ) {
			SUICoord pinCoord;
			pinCoord.SetSize( pElement->fTextureWidth, pElement->fTextureHeight );
			pinCoord.fX = m_MoveButtonCoord.fX + m_MoveButtonCoord.fWidth/2.0f;
			pinCoord.fY = (m_MoveButtonCoord.fY + m_MoveButtonCoord.fHeight/2.0f) - pinCoord.fHeight;

			if( m_bExistTemplateTexture )
				m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, pinCoord );
			else
				m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pinCoord );
		}
	}
}