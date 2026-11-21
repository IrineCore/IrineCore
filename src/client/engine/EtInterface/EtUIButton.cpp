#include "StdAfx.h"
#include "EtUIButton.h"
#include "EtUIDialog.h"
#include "EtUITemplate.h"
#include "EtUIXML.h"
#include "EtSoundEngine.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtUIButton::CEtUIButton( CEtUIDialog *pParent )
: CEtUIControl( pParent )
, m_nButtonID(0)
, m_bBlink(false)
, m_fBlinkTime(0.0f)
, m_nBlinkAlpha(0)
{
	m_Property.UIType = UI_CONTROL_BUTTON;	
	m_nToggleButtonIndex = 0;
	//m_emSoundIndex = CEtUISound::UI_SOUND_NONE;
	m_nSoundIndex = -1;

	IsDisable = false;             //Whether disabled  
	m_ElapsedDisableTime = 0.0f;	// Elapsed time since disabled
	IsEnableBackUp = false;
	m_DisableTime = 0.0f; //Must be able to set this value in the tool		
	m_bEnableKeyboard = true;
	m_bRightButton = false;

	m_nBlendTime = 1000;
}

CEtUIButton::~CEtUIButton(void)
{
	if( m_nSoundIndex != -1 )
	{
		if( CEtSoundEngine::IsActive() )
		{
			CEtSoundEngine::GetInstance().RemoveSound( m_nSoundIndex );
		}
		m_nSoundIndex = -1;
	}
}

void CEtUIButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	if( m_Property.ButtonProperty.nStringIndex > 0 )
	{
		SetText( m_pParent->GetUIString( CEtUIXML::idCategory1, m_Property.ButtonProperty.nStringIndex ) );
	}

	if( m_Property.ButtonProperty.szSoundFileName != '\0' )
	{
		m_nSoundIndex = CEtSoundEngine::GetInstance().LoadSound( m_Property.ButtonProperty.szSoundFileName, false, false );
	}
	m_nButtonID = m_Property.ButtonProperty.nButtonID;
}

bool CEtUIButton::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() || !m_bEnableKeyboard )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			switch( wParam )
			{
			case VK_SPACE:
				m_bPressed = true;
				return true;
			}
		}

	case WM_KEYUP:
		{
			switch( wParam )
			{
			case VK_SPACE:
				if( m_bPressed == true )
				{
					m_bPressed = false;
					m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, WM_KEYUP );
					//GetUISound().Play( m_emSoundIndex );
					PlayButtonSound();

					//blondy
					if( m_DisableTime )
					{
						IsDisable = true;
						IsEnableBackUp = IsEnable();
						CEtUIControl::Enable(!IsDisable); 
					}						
					//blondy end
				}
				return true;
			}
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------
bool CEtUIButton::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}
	// Block all right-click handling on EtUIButton by default.	

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		m_MouseCoord.fX = fX;
		m_MouseCoord.fY = fY;
		break;
	case WM_LBUTTONDBLCLK:
	//case WM_RBUTTONDBLCLK:
		if( IsInside( fX, fY ) )
		{
			m_pParent->ProcessCommand( EVENT_BUTTON_DOUBLE_CLICKED, true, this, uMsg );
		}
		break;
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		m_MouseCoord.fX = fX;
		m_MouseCoord.fY = fY;
		if( IsInside( fX, fY ) )
		{
			m_bRightPressed = true;
			SetCapture( m_pParent->GetHWnd() );
			if( !m_bFocus )
			{
				m_pParent->RequestFocus( this );
			}
			return true;
		}		
		break;
	case WM_LBUTTONDOWN:
		{
			m_MouseCoord.fX = fX;
			m_MouseCoord.fY = fY;
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
		}
		break;
		
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		if( m_bRightPressed )
		{
			m_bRightPressed = false;
			ReleaseCapture();
			m_MouseCoord.fX = 0;
			m_MouseCoord.fY = 0;
		}
		break;
	case WM_LBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();
				m_MouseCoord.fX = 0;
				m_MouseCoord.fY = 0;
				if( IsInside( fX, fY ) )
				{
					if( uMsg == WM_LBUTTONUP )
					{
						if( wParam & MK_SHIFT )
						{
							uMsg |= 0x0010;
						}
					}

					m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, uMsg );
					
					// Note : For toggle buttons, cycle the button appearance through the number of elements on click.
					//		The size button of the chat dialog is a toggle button. It would be better to make a separate toggle button.
					//		Like CEtUIToggleButton... or make it a custom button...
					//
					if( m_Property.ButtonProperty.ButtonType == UI_BUTTON_TOGGLE )
					{
						m_nToggleButtonIndex++;

						if( m_nToggleButtonIndex >= (int)m_Template.m_vecElement.size() )
						{
							m_nToggleButtonIndex = 0;
						}
					}

					//blondy
					if( m_DisableTime )
					{
						IsDisable = true;
						IsEnableBackUp = IsEnable();
						CEtUIControl::Enable(!IsDisable); 
					}						
					//blondy end

					//GetUISound().Play( m_emSoundIndex );
					PlayButtonSound();
				}

				return true;
			}
		}
		break;
	}

	return false;
}

void CEtUIButton::OnHotkey()
{
	m_pParent->ProcessCommand( EVENT_BUTTON_CLICKED, true, this, WM_KEYDOWN );
	//blondy
	if( m_DisableTime )
	{
		IsDisable = true;
		IsEnableBackUp = IsEnable();
		CEtUIControl::Enable(!IsDisable); 
	}						
	//blondy end

	//GetUISound().Play( m_emSoundIndex );
	PlayButtonSound();
}

void CEtUIButton::SetBlink( bool bBlink )
{
	m_bBlink = bBlink;

	if( !m_bBlink )
		m_fBlinkTime = 0.0f;
}

void CEtUIButton::UpdateBlink( float fElapsedTime )
{
	m_nBlinkAlpha = 255;

	if( m_bBlink )
	{
		m_fBlinkTime += fElapsedTime * 1000;
		float BLEND_RANGE  = 1.0f;
		int nTemp = (int)m_fBlinkTime%(m_nBlendTime+m_nBlendTime);
		if( nTemp < m_nBlendTime ) 
		{
			m_nBlinkAlpha = int(( BLEND_RANGE / (float)m_nBlendTime ) * nTemp * 255.0f);
		}
		else 
		{
			m_nBlinkAlpha = int(( BLEND_RANGE / (float)m_nBlendTime ) * ( (m_nBlendTime+m_nBlendTime) - nTemp ) * 255.0f);
		}

		m_nBlinkAlpha = min(m_nBlinkAlpha, 255);
	}
	else
	{
		m_fBlinkTime = 0.0f;
	}
}

void CEtUIButton::Render( float fElapsedTime )
{
	//if( !IsMouseEnter() && m_bPressed )
	//{
	//	m_bPressed = false;
	//	ReleaseCapture();
	//}

	m_CurrentState = UI_STATE_NORMAL;

	m_MoveButtonCoord = m_Property.UICoord;
	m_MoveBaseCoordLeft = m_BaseCoordLeft;
	m_MoveBaseCoordMid = m_BaseCoordMid;
	m_MoveBaseCoordRight = m_BaseCoordRight;

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

		m_MoveBaseCoordLeft.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordLeft.fY += m_Property.ButtonProperty.fMoveOffsetVert;

		m_MoveBaseCoordMid.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordMid.fY += m_Property.ButtonProperty.fMoveOffsetVert;

		m_MoveBaseCoordRight.fX += m_Property.ButtonProperty.fMoveOffsetHori;
		m_MoveBaseCoordRight.fY += m_Property.ButtonProperty.fMoveOffsetVert;
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
	else if( m_Property.ButtonProperty.ButtonType == UI_BUTTON_TOGGLE )
	{
		pElement = GetElement(m_nToggleButtonIndex);
	}

	if( !pElement )
		return;
	
	UpdateBlink( fElapsedTime );

	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

	D3DXCOLOR colorMouseEnter( pElement->TextureColor.dwColor[UI_STATE_MOUSEENTER] );
	colorMouseEnter.a = m_nBlinkAlpha / 255.0f;

	if( (pElement->fTextureWidth < m_Property.UICoord.fWidth) && m_Property.ButtonProperty.bVariableWidth )
	{
		// Note : If larger than the texture size, split into 3 parts to render.
		//		For buttons, only horizontal size is variable. Additional work is needed to make vertical size variable too.
		//
		m_MoveBaseCoordLeft.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, m_Property.ButtonProperty.fVariableWidthLength, m_Property.UICoord.fHeight );
		m_MoveBaseCoordMid.SetCoord( m_MoveBaseCoordLeft.Right(), m_Property.UICoord.fY, m_Property.UICoord.fWidth-(m_Property.ButtonProperty.fVariableWidthLength*2.0f), m_Property.UICoord.fHeight );
		m_MoveBaseCoordRight.SetCoord( m_MoveBaseCoordMid.Right(), m_Property.UICoord.fY, m_Property.ButtonProperty.fVariableWidthLength, m_Property.UICoord.fHeight );

		EtTextureHandle hTexture = m_pParent->GetUITexture();
		if( m_bExistTemplateTexture )
			hTexture = m_Template.m_hTemplateTexture;
		if( hTexture )
		{
			int nTextureWidth = hTexture->Width();
			float fBaseWidth = m_Property.ButtonProperty.fVariableWidthLength * DEFAULT_UI_SCREEN_WIDTH / nTextureWidth;

			SUICoord UVCoord = pElement->UVCoord;
			if( m_bExistTemplateTexture )
				UVCoord = pElement->TemplateUVCoord;

			m_uvCoordLeft = UVCoord;
			m_uvCoordLeft.fWidth = fBaseWidth;

			m_uvCoordRight = UVCoord;
			m_uvCoordRight.fX = UVCoord.Right() - fBaseWidth;
			m_uvCoordRight.fWidth = fBaseWidth;

			m_uvCoordMid = UVCoord;
			m_uvCoordMid.fX = UVCoord.fX + fBaseWidth;
			m_uvCoordMid.fWidth = m_uvCoordMid.fWidth - fBaseWidth*2.0f;
		}

		m_pParent->DrawSprite( hTexture, m_uvCoordLeft, pElement->TextureColor.dwCurrentColor, m_MoveBaseCoordLeft );
		m_pParent->DrawSprite( hTexture, m_uvCoordMid, pElement->TextureColor.dwCurrentColor, m_MoveBaseCoordMid );
		m_pParent->DrawSprite( hTexture, m_uvCoordRight, pElement->TextureColor.dwCurrentColor, m_MoveBaseCoordRight );

		if( m_bBlink && m_CurrentState == UI_STATE_NORMAL )
		{
			SUIElement *pElementOver = GetElement(1);
			SUICoord uvCoordLeft;
			SUICoord uvCoordMid;
			SUICoord uvCoordRight;
			if( hTexture )
			{
				int nTextureWidth = hTexture->Width();
				float fBaseWidth = m_Property.ButtonProperty.fVariableWidthLength * DEFAULT_UI_SCREEN_WIDTH / nTextureWidth;

				SUICoord UVCoord = pElementOver->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElementOver->TemplateUVCoord;

				uvCoordLeft = UVCoord;
				uvCoordLeft.fWidth = fBaseWidth;

				uvCoordRight = UVCoord;
				uvCoordRight.fX = UVCoord.Right() - fBaseWidth;
				uvCoordRight.fWidth = fBaseWidth;

				uvCoordMid = UVCoord;
				uvCoordMid.fX = UVCoord.fX + fBaseWidth;
				uvCoordMid.fWidth = uvCoordMid.fWidth - fBaseWidth*2.0f;
			}
			m_pParent->DrawSprite( hTexture, uvCoordLeft, colorMouseEnter, m_MoveBaseCoordLeft );
			m_pParent->DrawSprite( hTexture, uvCoordMid, colorMouseEnter, m_MoveBaseCoordMid );
			m_pParent->DrawSprite( hTexture, uvCoordRight, colorMouseEnter, m_MoveBaseCoordRight );
		}
	}
	else
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );

		if( m_bBlink && m_CurrentState == UI_STATE_NORMAL )
		{
			SUIElement *pElementOver = GetElement(1);
			if( pElementOver )
			{
				if( m_bExistTemplateTexture )
					m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElementOver->TemplateUVCoord, colorMouseEnter, m_MoveButtonCoord );
				else
					m_pParent->DrawSprite( pElementOver->UVCoord, colorMouseEnter, m_MoveButtonCoord );
			}
		}
	}

	m_MoveButtonCoord.fX += pElement->fFontHoriOffset;
	m_MoveButtonCoord.fY += pElement->fFontVertOffset;

	m_pParent->DrawDlgText( m_strRenderText.c_str(), pElement, pElement->FontColor.dwCurrentColor, m_MoveButtonCoord );
}

void CEtUIButton::MouseEnter( bool bEnter )
{
	CEtUIControl::MouseEnter( bEnter );

	if( bEnter ) 
	{
		//GetUISound().Play( CEtUISound::UI_CUSOR_ON );
	}
}

void CEtUIButton::UpdateRects()
{
	//if( GetElementCount() == 0 )
	//	return;

	//SUIElement *pElement(NULL);
	//pElement = GetElement(0);

	//if( (pElement->fTextureWidth < m_Property.UICoord.fWidth) && m_Property.ButtonProperty.bVariableWidth )
	//{
	//	// Note: If larger than texture size, split into 3 parts and render.
	//	//
	//	m_BaseCoordLeft.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, m_Property.ButtonProperty.fVariableWidthLength, m_Property.UICoord.fHeight );
	//	m_BaseCoordMid.SetCoord( m_BaseCoordLeft.Right(), m_Property.UICoord.fY, m_Property.UICoord.fWidth-(m_Property.ButtonProperty.fVariableWidthLength*2.0f), m_Property.UICoord.fHeight );
	//	m_BaseCoordRight.SetCoord( m_BaseCoordMid.Right(), m_Property.UICoord.fY, m_Property.ButtonProperty.fVariableWidthLength, m_Property.UICoord.fHeight );

	//	EtTextureHandle hTexture = m_pParent->GetUITexture();
	//	if( hTexture )
	//	{
	//		int nTextureWidth = hTexture->Width();
	//		float fBaseWidth = m_Property.ButtonProperty.fVariableWidthLength * DEFAULT_UI_SCREEN_WIDTH / nTextureWidth;

	//		m_uvCoordLeft = pElement->UVCoord;
	//		m_uvCoordLeft.fWidth = fBaseWidth;

	//		m_uvCoordMid = pElement->UVCoord;
	//		m_uvCoordMid.fX = pElement->UVCoord.Right() - fBaseWidth;
	//		m_uvCoordMid.fWidth = fBaseWidth;

	//		m_uvCoordRight = pElement->UVCoord;
	//		m_uvCoordRight.fX = pElement->UVCoord.fX + fBaseWidth;
	//		m_uvCoordRight.fWidth = ((pElement->fTextureWidth-(m_Property.ButtonProperty.fVariableWidthLength*2.0f))*DEFAULT_UI_SCREEN_WIDTH) / nTextureWidth;
	//	}
	//}
}

void CEtUIButton::Process( float fElapsedTime )
{
	CEtUIControl::Process(fElapsedTime);

	if( IsDisable )
	{
		m_ElapsedDisableTime += fElapsedTime;
		if( m_ElapsedDisableTime >= m_DisableTime)
		{
			IsDisable = false;
			m_ElapsedDisableTime = 0.0f;
			Enable(IsEnableBackUp);

		}
	}
}

void CEtUIButton::Enable( bool bEnable ) 
{
	IsEnableBackUp = bEnable;

	if( !IsDisable )
	{
		CEtUIControl::Enable( bEnable );
	}	 
}

void CEtUIButton::PlayButtonSound()
{
	if( !IsEnable() ) return;
	if( m_nSoundIndex != -1 ) CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundIndex );
}

void CEtUIButton::SetText( const std::wstring &strText, bool bFit, float fScaleX, float fScaleY )
{
	if( m_Property.ButtonProperty.bAutoTooltip ) {
		SetTooltipText( L"" );
		if( SetTextWithEllipsis( strText.c_str(), L"..." ) )
			SetTooltipText( strText.c_str() );
		return;
	}
	CEtUIControl::SetText( strText );
}

bool CEtUIButton::SetTextWithEllipsis( const wchar_t *wszText, const wchar_t *wszSymbol )
{
	ASSERT( wszText&&wszSymbol);

	m_szText = wszText;
	m_strRenderText = m_szText;

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return false;

	SUICoord sSymbolCoord;
	m_pParent->CalcTextRect( wszSymbol, pElement, sSymbolCoord );

	SUICoord sTextCoord;
	m_pParent->CalcTextRect( m_szText.c_str(), pElement, sTextCoord );

	if( sTextCoord.fWidth > m_Property.UICoord.fWidth )
	{
		float fRemainWidth = m_Property.UICoord.fWidth - sSymbolCoord.fWidth - 20.0f/DEFAULT_UI_SCREEN_WIDTH;
		int nStrLen = (int)m_szText.length();
		float fTextCoordHeight = sTextCoord.fHeight;
		int i(0), nStartPos(0);

		for( ; (nStartPos+i)<nStrLen; i++)
		{
			SUICoord sSubTextCoord;
			m_pParent->CalcTextRect( m_szText.substr(nStartPos,i).c_str(), pElement, sSubTextCoord );

			// Unlike Static, buttons have plenty of vertical space, so only check horizontal size
			// Both borders as well 
			if( sSubTextCoord.fWidth > fRemainWidth )
			{
				i--;

				m_strRenderText = m_szText.substr(0,nStartPos+i);
				m_strRenderText += wszSymbol;
				return true;
			}
		}
	}

	return false;
}