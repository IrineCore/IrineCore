#include "StdAfx.h"
#include "EtUIComboBox.h"
#include "EtUIDialog.h"
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CEtUIComboBox::CEtUIComboBox( CEtUIDialog *pParent )
	:CEtUIButton( pParent)
	, m_ScrollBar( pParent )
{
	m_Property.UIType = UI_CONTROL_COMBOBOX;
	m_bOpened = false;
	m_bEditMode = false;
	m_iSelected = -1;
	m_iFocused = -1;
	m_fLineSpace = 0.0f;
}

CEtUIComboBox::~CEtUIComboBox(void)
{
	SAFE_DELETE_PVEC( m_vecItems );
}

void CEtUIComboBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( pProperty && pProperty->ComboBoxProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( pProperty->ComboBoxProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_fLineSpace = m_Property.ComboBoxProperty.fLineSpace;
	UpdateRects();
}

void CEtUIComboBox::SetTextColor( DWORD dwColor )
{
	SUIElement *pElement;
	pElement = GetElement(0);

	if( pElement )
	{
		pElement->FontColor.dwColor[ UI_STATE_NORMAL ] = dwColor;
		pElement->FontColor.dwColor[ UI_STATE_FOCUS ] = dwColor;
		pElement->FontColor.dwColor[ UI_STATE_MOUSEENTER ] = dwColor;
		pElement->FontColor.dwColor[ UI_STATE_PRESSED ] = dwColor;
	}
}

void CEtUIComboBox::Focus( bool bFocus )
{
	CEtUIButton::Focus( bFocus );

	if( !bFocus )
	{
		if( m_bOpened )
		{
			m_pParent->ProcessCommand( EVENT_COMBOBOX_DROPDOWN_CLOSED, true, this );
		}
		m_bOpened = false;
	}

}

void CEtUIComboBox::AddItem( const WCHAR* strText, void* pData, int nValue, bool bEllipsis )
{
	SComboBoxItem *pNewItem;
	pNewItem = new SComboBoxItem;

	std::wstring tempStr = strText;
	std::wstring ellipsis = L"...";
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	bool bHasEllipsis = false;
	if (bEllipsis)
	{
		bHasEllipsis = m_pParent->MakeStringWithEllipsis(GetElement(2), m_DropDownTextCoord.fWidth - m_ScrollBar.GetUICoord().fWidth, tempStr, ellipsis);
	}
#else
	if (bEllipsis)
		m_pParent->MakeStringWithEllipsis(GetElement(2), m_DropDownTextCoord.fWidth - (m_ScrollBar.GetUICoord().fWidth * 0.7f), tempStr, ellipsis);
#endif

	//wcscpy_s( pNewItem->strText, 256, strText );
	wcscpy_s( pNewItem->strText, 256, tempStr.c_str());
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	pNewItem->strOriginal = strText;
#endif
	pNewItem->pData = pData;
	pNewItem->nValue = nValue;
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
	pNewItem->bEllipsis = bHasEllipsis;
#else
	pNewItem->bEllipsis = bEllipsis;
#endif

	m_vecItems.push_back( pNewItem );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
	if( m_vecItems.size() == 1 )
	{
		m_iSelected = 0;
		m_iFocused = 0;
		m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
	}
}

void CEtUIComboBox::RemoveAllItems()
{
	SAFE_DELETE_PVEC( m_vecItems );
	m_ScrollBar.SetTrackRange( 0, 1 );
	m_iFocused = -1;
	m_iSelected = -1;
}

void CEtUIComboBox::RemoveItem( int nIndex )
{
	SAFE_DELETE( m_vecItems[ nIndex ] );
	m_vecItems.erase( m_vecItems.begin() + nIndex );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
	if( m_iSelected >= ( int )m_vecItems.size() )
	{
		m_iSelected = ( int )m_vecItems.size() - 1;
	}
}

bool CEtUIComboBox::ContainsItem( const WCHAR* strText, UINT iStart )
{
	return ( -1 != FindItem( strText, iStart ) );
}

int CEtUIComboBox::FindItem( const WCHAR* strText, UINT iStart )
{
	if( strText == NULL )
	{
		return -1;
	}

	for( int i = iStart; i < ( int )m_vecItems.size(); i++ )
	{
		if( wcscmp( m_vecItems[ i ]->strText, strText ) == 0 )
		{
			return i;
		}
	}

	return -1;
}

int CEtUIComboBox::FindItemByValue( int nValue, UINT iStart )
{
	for( int i = iStart; i < ( int )m_vecItems.size(); i++ )
	{
		if( m_vecItems[ i ]->nValue == nValue )
		{
			return i;
		}
	}

	return -1;
}

void *CEtUIComboBox::GetItemData( const WCHAR* strText )
{
	int nIndex = FindItem( strText );
	if( nIndex == -1 )
	{
		return NULL;
	}

	return m_vecItems[ nIndex ]->pData;
}

void *CEtUIComboBox::GetItemData( int nIndex )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return NULL;
	}

	return m_vecItems[ nIndex ]->pData;
}

void *CEtUIComboBox::GetSelectedData()
{
	return GetItemData( m_iSelected );
}

bool CEtUIComboBox::GetItemValue( const WCHAR* strText, int &nValue )
{
	int nIndex = FindItem( strText );
	if( nIndex == -1 )
	{
		return false;
	}

	nValue = m_vecItems[nIndex]->nValue;
	return true;
}

bool CEtUIComboBox::GetItemValue( int nIndex, int &nValue )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return false;
	}

	nValue = m_vecItems[nIndex]->nValue;
	return true;
}

bool CEtUIComboBox::GetSelectedValue( int &nValue )
{
	return GetItemValue( m_iSelected, nValue );
}

SComboBoxItem *CEtUIComboBox::GetSelectedItem()
{
	if( ( m_iSelected < 0 ) || ( m_iSelected >= ( int )m_vecItems.size() ) )
	{
		return NULL;
	}

	return m_vecItems[ m_iSelected ];
}

void CEtUIComboBox::SetSelectedByIndex( int nIndex )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return;
	}

	m_iFocused = nIndex;
	m_iSelected = nIndex;
	m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
}

void CEtUIComboBox::SetSelectedByText( const WCHAR* strText )
{
	if( strText == NULL )
	{
		return;
	}

	int nIndex;

	nIndex = FindItem( strText );
	if( nIndex == -1 )
	{
		return;
	}

	m_iFocused = nIndex;
	m_iSelected = nIndex;
	m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
}

void CEtUIComboBox::SetSelectedByData( void* pData )
{
	for( int i = 0; i < ( int )m_vecItems.size(); i++ )
	{
		if( m_vecItems[ i ]->pData == pData )
		{
			m_iFocused = i;
			m_iSelected = i;
			m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
		}
	}
}

void CEtUIComboBox::SetSelectedByValue( int nValue )
{
	for( int i = 0; i < ( int )m_vecItems.size(); i++ )
	{
		if( m_vecItems[ i ]->nValue == nValue )
		{
			m_iFocused = i;
			m_iSelected = i;
			m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, false, this );
		}
	}
}


bool CEtUIComboBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	const DWORD REPEAT_MASK = (0x40000000);

	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	//if( ( WM_LBUTTONDOWN == uMsg ) && ( !m_bFocus ) )
	//{
	//	m_pParent->RequestFocus( this );
	//}

	if( m_ScrollBar.HandleKeyboard( uMsg, wParam, lParam ) )
	{
		return true;
	}

	if( uMsg == WM_KEYDOWN )
	{
		switch( wParam )
		{
		case VK_RETURN:
			if( m_bOpened )
			{
				if( m_iSelected != m_iFocused )
				{
					m_iSelected = m_iFocused;
				}
				m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this ); // moved by kalliste
				m_bOpened = false;
				return true;
			}
			break;
		case VK_F4:
			if( lParam & REPEAT_MASK )
			{
				return true;
			}
			m_bOpened = !m_bOpened;
			if( !m_bOpened )
			{
				m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
			}
			return true;
		case VK_LEFT:
		case VK_UP:
			if( m_iFocused > 0 )
			{
				m_iFocused--;
				m_iSelected = m_iFocused;
				// PRE_ADD_PREDICTIVE_TEXT - elkain03
				m_ScrollBar.SetTrackPos( m_iSelected );
				if( !m_bOpened )
				{
					m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
				}
			}
			return true;
		case VK_RIGHT:
		case VK_DOWN:
			if( ( m_iFocused + 1 ) < ( int )m_vecItems.size() )
			{
				m_iFocused++;
				m_iSelected = m_iFocused;
				// PRE_ADD_PREDICTIVE_TEXT - elkain03
				m_ScrollBar.SetTrackPos( m_iSelected );
				if( !m_bOpened )
				{
					m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
				}
			}
			return true;
		}
	}

	return false;
}

bool CEtUIComboBox::ToggleDropDownList()
{
	m_bPressed = true;
	SetCapture( m_pParent->GetHWnd() );
	if( !m_bFocus )
	{
		m_pParent->RequestFocus( this );
	}
	if( m_bFocus )
	{
		m_bOpened = !m_bOpened;

		if( m_bOpened )
		{
			m_pParent->ProcessCommand( EVENT_COMBOBOX_DROPDOWN_OPENED, true, this );
		}
		else
		{
			m_pParent->ProcessCommand( EVENT_COMBOBOX_DROPDOWN_CLOSED, true, this );
		}
	}

	return true;
}

bool CEtUIComboBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}
	// Originally there was no m_bOpened check, fixed due to a bug (not checked in CustomUI either).
	// When combo boxes are placed vertically and the upper combo box's dropdown area overlaps the lower one
	// (e.g., Graphics Resolution and Graphics Quality combo boxes in System Options -> Graphics Options)
	// Opening and closing the upper combobox once breaks the dropdown button on the lower combobox (hover highlight still shows).
	// The cause was in this scrollbar's HandleMouse.
	// So fixed it by only sending messages to the scrollbar when Opened.
	// Will need to fix again if any weird issue occurs.
	if( m_bOpened && m_ScrollBar.HandleMouse( uMsg, fX, fY, wParam, lParam ) )
	{
		return true;
	}

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		if( ( m_bOpened ) && ( m_DropDownCoord.IsInside( fX, fY ) ) )
		{
			for( int i = 0; i < ( int )m_vecItems.size(); i++ )
			{
				if( ( m_vecItems[ i ]->bVisible ) && ( m_vecItems[ i ]->ItemCoord.IsInside( fX, fY ) ) )
				{
					m_iFocused = i;

#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
					m_pParent->ShowTooltipDlg(this, m_vecItems[i]->bEllipsis, m_vecItems[i]->strOriginal);
#endif
				}
			}

			return true;
		}

#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
		if (m_DropDownCoord.IsInside( fX, fY ) == false)
		{
			const SComboBoxItem* pSelected = GetSelectedItem();
			if (pSelected != NULL)
				m_pParent->ShowTooltipDlg(this, pSelected->bEllipsis, pSelected->strOriginal);

			if (IsInside(fX, fY) == false && m_pParent->IsShowTooltipDlg())
				m_pParent->ShowTooltipDlg(this, false, pSelected->strOriginal);
		}
#endif
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
			if (m_pParent->IsShowTooltipDlg())
				m_pParent->ShowTooltipDlg(this, false, L"");
#endif

			if( IsInside( fX, fY ) )
			{
				ToggleDropDownList();
				return true;
			}

			if( ( m_bOpened ) && ( m_DropDownCoord.IsInside( fX, fY ) ) )
			{
				int i;

				for( i = m_ScrollBar.GetTrackPos(); i < ( int)m_vecItems.size(); i++ )
				{
					if( ( m_vecItems[ i ]->bVisible ) && ( m_vecItems[ i ]->ItemCoord.IsInside( fX, fY ) ) )
					{
						m_iFocused = m_iSelected = i;
						m_bOpened = false;	//	moved by kalliste
						m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						break;
					}
				}
				return true;
			}

			if( m_bOpened )
			{
				m_iFocused = m_iSelected;
				m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
				m_bOpened = false;
			}
			m_bPressed = false;
			break;
		}
	case WM_LBUTTONUP:
		if( m_bPressed && IsInside( fX, fY ) )
		{
			m_bPressed = false;
			ReleaseCapture();
			return true;
		}
		break;
	case WM_MOUSEWHEEL:
		{
			int zDelta;

#ifdef PRE_FIX_COMBOBOX_ELLIPSIS
			if (m_pParent->IsShowTooltipDlg())
				m_pParent->ShowTooltipDlg(this, false, L"");
#endif

			zDelta = ( short )HIWORD( wParam ) / WHEEL_DELTA;
			if( m_bOpened )
			{
				UINT uLines;

				SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
				m_ScrollBar.Scroll( -zDelta * uLines );
			} 
			else
			{
				if( zDelta > 0 )
				{
					if( m_iFocused > 0 )
					{
						m_iFocused--;
						m_iSelected = m_iFocused;     
						if( !m_bOpened )
						{
							m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						}
					}          
				}
				else
				{
					if( m_iFocused + 1 < ( int )m_vecItems.size() )
					{
						m_iFocused++;
						m_iSelected = m_iFocused;   
						if( !m_bOpened )
						{
							m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
						}
					}
				}
			}
			return true;
		}
	}

	return false;
}

void CEtUIComboBox::OnHotkey()
{
	if( m_bOpened )
	{
		return;
	}
	if( m_iSelected == -1 )
	{
		return;
	}

	m_pParent->RequestFocus( this );
	m_iSelected++;
	if( m_iSelected >= ( int )m_vecItems.size() )
	{
		m_iSelected = 0;
	}
	m_iFocused = m_iSelected;
	m_pParent->ProcessCommand( EVENT_COMBOBOX_SELECTION_CHANGED, true, this );
}

void CEtUIComboBox::UpdateRects()
{
	SUIElement *pElement = GetElement(1);
	if( !pElement ) return;

	m_ButtonCoord = m_Property.UICoord;
	m_ButtonCoord.fX = m_Property.UICoord.Right() - pElement->fTextureWidth;
	m_ButtonCoord.fWidth = pElement->fTextureWidth;
	m_ButtonCoord.fHeight = pElement->fTextureHeight;

	m_DropDownCoord = m_Property.UICoord;
	m_DropDownCoord.fY += m_Property.UICoord.fHeight * 0.9f;
	m_DropDownCoord.fWidth = m_Property.UICoord.fWidth;

	m_TextCoord = m_Property.UICoord;
	m_TextCoord.fWidth -= pElement->fTextureWidth;
//	m_TextCoord.fX += 14.0f / DEFAULT_UI_SCREEN_WIDTH; // Unavoidable magic number. Set left position when font is left-aligned in combo box.
//	m_TextCoord.fWidth -= 24.0f / DEFAULT_UI_SCREEN_WIDTH; // Set right position when font is right-aligned in combo box.

	pElement = GetElement(2);

	if( m_Property.ComboBoxProperty.fDropdownSize )
	{
		m_DropDownCoord.fHeight = m_Property.ComboBoxProperty.fDropdownSize;
	}
	else
	{
		m_DropDownCoord.fHeight = pElement->fTextureHeight;
	}

	m_DropDownTextCoord = m_DropDownCoord;
	m_DropDownTextCoord.fX += 14.0f / DEFAULT_UI_SCREEN_WIDTH;
	m_DropDownTextCoord.fWidth -= 24.0f / DEFAULT_UI_SCREEN_WIDTH;
	m_DropDownTextCoord.fY += m_DropDownCoord.fHeight * 0.1f;
	m_DropDownTextCoord.fHeight -= m_DropDownCoord.fHeight * 0.16f;

	m_ScrollBar.SetPosition( m_DropDownCoord.Right()-m_Property.ComboBoxProperty.fScrollBarSize, m_DropDownCoord.fY+6.0f/DEFAULT_UI_SCREEN_HEIGHT );
	m_ScrollBar.SetSize( m_Property.ComboBoxProperty.fScrollBarSize, m_DropDownCoord.fHeight-0.012f );

	pElement = GetElement(2);

	float fFontHeight = m_pParent->GetFontHeight( pElement->nFontIndex, pElement->nFontHeight );
	fFontHeight += m_fLineSpace;
	if( fFontHeight > 0.0f )
	{
		m_ScrollBar.SetPageSize( ( int )( m_DropDownTextCoord.fHeight / fFontHeight ) );
		m_ScrollBar.ShowItem( m_iSelected );
	}
}

void CEtUIComboBox::Render( float fElapsedTime )
{
	m_CurrentState = UI_STATE_NORMAL;
	if( !m_bOpened )
	{
		m_CurrentState = UI_STATE_HIDDEN;
	}

	SUIElement *pElement(NULL), *pSelectionElement(NULL);
	float fFontHeight;

	pElement = GetElement(2);
	pSelectionElement = GetElement(3);
	fFontHeight = m_pParent->GetFontHeight( pElement->nFontIndex, pElement->nFontHeight );
	fFontHeight += m_fLineSpace;

	UpdateBlendRate();

	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

	// Combo box will always be resizable, so draw it split into 3 parts.
	int nTextureWidth = 0;
	float fBaseWidth = 0.0f;
	EtTextureHandle hTexture = m_pParent->GetUITexture();
	if( m_bExistTemplateTexture )
		hTexture = m_Template.m_hTemplateTexture;
	if( hTexture ) {
		nTextureWidth = hTexture->Width();
		fBaseWidth = m_Property.ComboBoxProperty.fVariableWidthLength * DEFAULT_UI_SCREEN_WIDTH / nTextureWidth;
	}

	SUICoord uvLeft, uvCenter, uvRight;
	SUICoord uiLeft, uiCenter, uiRight;
	SUICoord UVCoord = pElement->UVCoord;
	if( m_bExistTemplateTexture )
		UVCoord = pElement->TemplateUVCoord;

	uiLeft.SetCoord( m_DropDownCoord.fX, m_DropDownCoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, m_DropDownCoord.fHeight );
	uiCenter.SetCoord( uiLeft.Right(), m_DropDownCoord.fY, m_DropDownCoord.fWidth-(m_Property.ComboBoxProperty.fVariableWidthLength*2.0f), m_DropDownCoord.fHeight );
	uiRight.SetCoord( uiCenter.Right(), m_DropDownCoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, m_DropDownCoord.fHeight );
	uvLeft = uvCenter = uvRight = UVCoord;
	uvLeft.fWidth = fBaseWidth;
	uvCenter.fX = UVCoord.fX + fBaseWidth;
	uvCenter.fWidth = uvCenter.fWidth-fBaseWidth*2.0f;
	uvRight.fX = UVCoord.Right() - fBaseWidth;
	uvRight.fWidth = fBaseWidth;
	m_pParent->DrawSprite( hTexture, uvLeft, pElement->TextureColor.dwCurrentColor, uiLeft );
	m_pParent->DrawSprite( hTexture, uvCenter, pElement->TextureColor.dwCurrentColor, uiCenter );
	m_pParent->DrawSprite( hTexture, uvRight, pElement->TextureColor.dwCurrentColor, uiRight );

	pSelectionElement->TextureColor.dwCurrentColor = pElement->TextureColor.dwCurrentColor;
	pSelectionElement->FontColor.dwCurrentColor = pSelectionElement->FontColor.dwColor[ UI_STATE_NORMAL ];

	float fCurY, fRemainingHeight;
	fCurY = m_DropDownTextCoord.fY;
	fRemainingHeight = m_DropDownTextCoord.fHeight;

	for( int i = m_ScrollBar.GetTrackPos(); i < ( int )m_vecItems.size(); i++ )
	{
		SComboBoxItem *pItem;

		pItem = m_vecItems[ i ];
		fRemainingHeight -= fFontHeight;
		if( fRemainingHeight < 0.0f )
		{
			pItem->bVisible = false;
			continue;
		}
		pItem->ItemCoord.SetCoord( m_DropDownTextCoord.fX, fCurY, m_DropDownTextCoord.fWidth, fFontHeight );
		fCurY += fFontHeight;
		pItem->bVisible = true;

		SUICoord uiItemFont = pItem->ItemCoord;
		uiItemFont.fX += pSelectionElement->fFontHoriOffset;
		uiItemFont.fY += pSelectionElement->fFontVertOffset;

		SUICoord SelectionUVCoord = pSelectionElement->UVCoord;
		if( m_bExistTemplateTexture )
			SelectionUVCoord = pSelectionElement->TemplateUVCoord;

		if( m_bOpened )
		{
			if( i == m_iFocused )
			{
				SUICoord FocusCoord;
				FocusCoord.SetCoord( m_DropDownCoord.fX, pItem->ItemCoord.fY-2.0f/DEFAULT_UI_SCREEN_HEIGHT, m_DropDownCoord.fWidth, GetElement(3)->fTextureHeight );

				uiLeft.SetCoord( FocusCoord.fX, FocusCoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, FocusCoord.fHeight );
				uiCenter.SetCoord( uiLeft.Right(), FocusCoord.fY, FocusCoord.fWidth-(m_Property.ComboBoxProperty.fVariableWidthLength*2.0f), FocusCoord.fHeight );
				uiRight.SetCoord( uiCenter.Right(), FocusCoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, FocusCoord.fHeight );
				uvLeft = uvCenter = uvRight = SelectionUVCoord;
				uvLeft.fWidth = fBaseWidth;
				uvCenter.fX = SelectionUVCoord.fX + fBaseWidth;
				uvCenter.fWidth = uvCenter.fWidth-fBaseWidth*2.0f;
				uvRight.fX = SelectionUVCoord.Right() - fBaseWidth;
				uvRight.fWidth = fBaseWidth;
				m_pParent->DrawSprite( hTexture, uvLeft, pElement->TextureColor.dwCurrentColor, uiLeft );
				m_pParent->DrawSprite( hTexture, uvCenter, pElement->TextureColor.dwCurrentColor, uiCenter );
				m_pParent->DrawSprite( hTexture, uvRight, pElement->TextureColor.dwCurrentColor, uiRight );
				m_pParent->DrawDlgText( pItem->strText, pSelectionElement, pSelectionElement->FontColor.dwColor[UI_STATE_MOUSEENTER], uiItemFont );
			}
			else
			{
				m_pParent->DrawDlgText( pItem->strText, pSelectionElement, pSelectionElement->FontColor.dwCurrentColor, uiItemFont );
			}
		}
	}

	m_CurrentState = UI_STATE_NORMAL;
	m_MoveButtonCoord = m_ButtonCoord;

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
		m_MoveButtonCoord.fX += m_Property.ComboBoxProperty.fMoveOffsetHori;
		m_MoveButtonCoord.fY += m_Property.ComboBoxProperty.fMoveOffsetVert;
	}
	else if( m_bMouseEnter )
	{
		m_CurrentState = UI_STATE_MOUSEENTER;
	}
	else if( m_bFocus )
	{
		m_CurrentState = UI_STATE_FOCUS;
	}

	if( m_bOpened )
	{
		m_CurrentState = UI_STATE_PRESSED;
	}
	
	pElement = GetElement(0);
	pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->FontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );

	UVCoord = pElement->UVCoord;
	if( m_bExistTemplateTexture )
		UVCoord = pElement->TemplateUVCoord;

	uiLeft.SetCoord( m_Property.UICoord.fX, m_Property.UICoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, m_Property.UICoord.fHeight );
	uiCenter.SetCoord( uiLeft.Right(), m_Property.UICoord.fY, m_Property.UICoord.fWidth-(m_Property.ComboBoxProperty.fVariableWidthLength*2.0f), m_Property.UICoord.fHeight );
	uiRight.SetCoord( uiCenter.Right(), m_Property.UICoord.fY, m_Property.ComboBoxProperty.fVariableWidthLength, m_Property.UICoord.fHeight );
	uvLeft = uvCenter = uvRight = UVCoord;
	uvLeft.fWidth = fBaseWidth;
	uvCenter.fX = UVCoord.fX + fBaseWidth;
	uvCenter.fWidth = uvCenter.fWidth-fBaseWidth*2.0f;
	uvRight.fX = UVCoord.Right() - fBaseWidth;
	uvRight.fWidth = fBaseWidth;
	m_pParent->DrawSprite( hTexture, uvLeft, pElement->TextureColor.dwCurrentColor, uiLeft );
	m_pParent->DrawSprite( hTexture, uvCenter, pElement->TextureColor.dwCurrentColor, uiCenter );
	m_pParent->DrawSprite( hTexture, uvRight, pElement->TextureColor.dwCurrentColor, uiRight );

	SUICoord uiMovedTextCoord = m_TextCoord;
	uiMovedTextCoord.fX += pElement->fFontHoriOffset;
	uiMovedTextCoord.fY += pElement->fFontVertOffset;
	if (m_bEditMode == false)
	{
		if( ( m_iSelected >= 0 ) && ( m_iSelected < ( int ) m_vecItems.size() ) )
		{
			m_pParent->DrawDlgText( m_vecItems[ m_iSelected ]->strText, pElement, pElement->FontColor.dwCurrentColor, uiMovedTextCoord );
		}
	}

	// Note : Combo button
	//
	{
		SUIElement *pElement(NULL);
		pElement = GetElement(1);

		if( ( m_CurrentState == UI_STATE_MOUSEENTER ) && (m_Template.m_vecElement.size() > 4) )
		{
			pElement = GetElement(4);
		}

		pElement->TextureColor.Blend( m_CurrentState, fElapsedTime, m_fBlendRate );
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_MoveButtonCoord );
	}

	static bool bSBInit = false;
	if( !bSBInit )
	{
		m_ScrollBar.SetPageSize( ( int )( m_DropDownTextCoord.fHeight / fFontHeight ) );
		bSBInit = true;
	}

	if( m_bOpened )
	{
		m_ScrollBar.Render( fElapsedTime );
	}
}

void CEtUIComboBox::ClearSelectedItem()
{
	m_iSelected = -1;
}

bool CEtUIComboBox::IsInside( float fX, float fY )
{
	if (m_bEditMode)
	{
		return m_ButtonCoord.IsInside(fX, fY);
	}

	return CEtUIButton::IsInside(fX, fY);
}

void CEtUIComboBox::FindInputPos( std::vector<EtVector2> &vecPos )
{
	CEtUIDialog *pDialog = GetParent();
	if( !pDialog ) return;

	float fPosX = pDialog->GetXCoord() + GetUICoord().fX + GetUICoord().fWidth / 2.0f;
	float fPosY = pDialog->GetYCoord() + GetUICoord().fY + GetUICoord().fHeight / 2.0f;

	vecPos.push_back( EtVector2(fPosX, fPosY) );

	fPosX = 0.0f;
	fPosY = 0.0f;

	if( m_bOpened )
	{
		for( int i = m_ScrollBar.GetTrackPos(); i < ( int )m_vecItems.size(); i++ )
		{
			if( m_vecItems[ i ]->bVisible )
			{
				fPosX = pDialog->GetXCoord() + m_vecItems[ i ]->ItemCoord.fX + m_vecItems[ i ]->ItemCoord.fWidth / 2.0f;
				fPosY = pDialog->GetYCoord() + m_vecItems[ i ]->ItemCoord.fY + m_vecItems[ i ]->ItemCoord.fHeight / 2.0f;

				vecPos.push_back( EtVector2(fPosX, fPosY) );
			}
		}

		m_ScrollBar.FindInputPos( vecPos );
	}
}