#include "StdAfx.h"
#include "EtUIListBox.h"
#include "EtUIDialog.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

SListBoxItem & SListBoxItem::operator=( const SListBoxItem & nRight )
{
	bSelected	= nRight.bSelected;
	dwColor		= nRight.dwColor;
	nValue		= nRight.nValue;
	pData		= nRight.pData;
	rcActive	= nRight.rcActive;
	wcscpy_s(strText, nRight.strText);

	return *this;
}

CEtUIListBox::CEtUIListBox( CEtUIDialog *pParent )
	: CEtUIControl( pParent ),
	m_ScrollBar( pParent )
{
	m_Property.UIType = UI_CONTROL_LISTBOX;

	m_nSelected = -1;
	m_nSelStart = 0;
	m_bDrag = false;
	m_fTextHeight = 0.0f;
	m_fLineSpace = 0.0f;
	m_nSelected = -1;	
}

CEtUIListBox::~CEtUIListBox(void)
{
	SAFE_DELETE_PVEC( m_vecItems );
}

void CEtUIListBox::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.ListBoxProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.ListBoxProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_fLineSpace = m_Property.ListBoxProperty.fLineSpace;
	UpdateRects();
}

#ifdef PRE_ADD_LISTBOX_ELLIPSIS
void CEtUIListBox::AddItem( const WCHAR *wszText, void *pData, int nValue, DWORD dwColor, bool bEllipsis )
#else // PRE_ADD_LISTBOX_ELLIPSIS
void CEtUIListBox::AddItem( const WCHAR *wszText, void *pData, int nValue, DWORD dwColor )
#endif // PRE_ADD_LISTBOX_ELLIPSIS
{
	SListBoxItem *pNewItem;

	pNewItem = new SListBoxItem;
	if( pNewItem == NULL ) return;

#ifdef PRE_ADD_LISTBOX_ELLIPSIS
	if( bEllipsis )
	{
		std::wstring tempStr = wszText;
		std::wstring ellipsis = L"...";
		pNewItem->bHasEllipsis = m_pParent->MakeStringWithEllipsis( GetElement(1), m_TextCoord.fWidth, tempStr, ellipsis );
		wcscpy_s( pNewItem->strEllipsisText, 256, tempStr.c_str() );
	}
#endif // PRE_ADD_LISTBOX_ELLIPSIS
	wcscpy_s( pNewItem->strText, 256, wszText );
	pNewItem->pData = pData;
	pNewItem->nValue = nValue;
	SetRect( &pNewItem->rcActive, 0, 0, 0, 0 );
	pNewItem->bSelected = false;
	pNewItem->dwColor = dwColor;

	m_vecItems.push_back( pNewItem );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
}

#ifdef PRE_ADD_LISTBOX_ELLIPSIS
void CEtUIListBox::InsertItem( int nIndex, const WCHAR *wszText, void *pData, int nValue, bool bEllipsis )
#else // PRE_ADD_LISTBOX_ELLIPSIS
void CEtUIListBox::InsertItem( int nIndex, const WCHAR *wszText, void *pData, int nValue )
#endif // PRE_ADD_LISTBOX_ELLIPSIS
{
	SListBoxItem *pNewItem;

	pNewItem = new SListBoxItem;
	if( pNewItem == NULL ) return;
#ifdef PRE_ADD_LISTBOX_ELLIPSIS
	if( bEllipsis )
	{
		std::wstring tempStr = wszText;
		std::wstring ellipsis = L"...";
		pNewItem->bHasEllipsis = m_pParent->MakeStringWithEllipsis( GetElement(1), m_TextCoord.fWidth, tempStr, ellipsis );
		wcscpy_s( pNewItem->strEllipsisText, 256, tempStr.c_str() );
	}
#endif // PRE_ADD_LISTBOX_ELLIPSIS
	wcscpy_s( pNewItem->strText, 256, wszText );
	pNewItem->pData = pData;
	pNewItem->nValue = nValue;
	SetRect( &pNewItem->rcActive, 0, 0, 0, 0 );
	pNewItem->bSelected = false;
	pNewItem->dwColor = 0xFFFFFFFF;

	m_vecItems.insert( m_vecItems.begin() + nIndex, pNewItem );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
}

void CEtUIListBox::SwapItem(int nLeft, int nRight)
{
	/*std::vector< SListBoxItem * >::iterator leftItor =  m_vecItems.begin() + nLeft;
	std::vector< SListBoxItem * >::iterator rightItor =  m_vecItems.begin() + nRight;

	if( m_vecItems.end() != leftItor && m_vecItems.end() != rightItor )
		std::swap(leftItor, rightItor);*/

	/*if( (int)m_vecItems.size() <= nLeft && (int)m_vecItems.size() <= nRight )
		std::swap(m_vecItems[nLeft], m_vecItems[nRight]);*/

	std::vector< SListBoxItem * >::iterator leftItor =  m_vecItems.begin() + nLeft;
	std::vector< SListBoxItem * >::iterator rightItor =  m_vecItems.begin() + nRight;

	SListBoxItem temp = *(*leftItor);

	*(*leftItor) = *(*rightItor);
	*(*rightItor) = temp;
}

void CEtUIListBox::RemoveItem( int nIndex )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return;
	}

	delete m_vecItems[ nIndex ];
	m_vecItems.erase( m_vecItems.begin() + nIndex );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
	if( m_nSelected >= ( int )m_vecItems.size() )
	{
		m_nSelected = ( int )m_vecItems.size() - 1;
	}
	m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
}

void CEtUIListBox::RemoveItemByText( WCHAR *wszText )
{
}

void CEtUIListBox::RemoveItemByData( void *pData )
{
}

void CEtUIListBox::RemoveAllItems()
{
	SAFE_DELETE_PVEC( m_vecItems );
	m_nSelected = -1;
	m_ScrollBar.SetTrackRange( 0, 1 );
}

bool CEtUIListBox::GetItemValue( int nIndex, int &nValue )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return false;
	}

	nValue = m_vecItems[nIndex]->nValue;
	return true;
}

bool CEtUIListBox::GetSelectedValue( int &nValue )
{
	return GetItemValue( m_nSelected, nValue );
}

SListBoxItem *CEtUIListBox::GetItem( int nIndex )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return NULL;
	}

	return m_vecItems[ nIndex ];
}

int CEtUIListBox::GetSelectedIndex( int nPreviousSelected )
{
	if( nPreviousSelected < -1 )
	{
		return -1;
	}

	if( m_Property.ListBoxProperty.bMultiSelect )
	{
		int i;

		for( i = nPreviousSelected + 1; i < ( int )m_vecItems.size(); i++ )
		{
			if( m_vecItems[ i ]->bSelected )
			{
				return i;
			}
		}
		return -1;
	}
	else
	{
		return m_nSelected;
	}
}

void CEtUIListBox::SelectItem( int nNewIndex )
{
	if( m_vecItems.empty() )
	{
		return;
	}

	int nOldSelected;

	nOldSelected = m_nSelected;
	m_nSelected = nNewIndex;

	if( m_nSelected < 0 )
	{
		m_nSelected = 0;
	}
	if( m_nSelected >= ( int )m_vecItems.size() )
	{
		m_nSelected = ( int )m_vecItems.size() - 1;
	}

	if( nOldSelected != m_nSelected )
	{
		if( m_Property.ListBoxProperty.bMultiSelect )
		{
			m_vecItems[ m_nSelected ]->bSelected = true;
		}
		m_nSelStart = m_nSelected;
		m_ScrollBar.ShowItem( m_nSelected );
	}
	m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
}

bool CEtUIListBox::HandleKeyboard( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}
	if( m_ScrollBar.HandleKeyboard( uMsg, wParam, lParam ) )
	{
		return true;
	}

	if( uMsg == WM_KEYDOWN )
	{
		switch( wParam )
		{
		case VK_UP:
		case VK_DOWN:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_HOME:
		case VK_END:
		{
			if( m_vecItems.size() == 0 )
			{
				return true;
			}

			int nOldSelected;
			nOldSelected = m_nSelected;

			switch( wParam )
			{
				case VK_UP: 
					m_nSelected--; 
					break;
				case VK_DOWN: 
					m_nSelected++; 
					break;
				case VK_NEXT: 
					m_nSelected += m_ScrollBar.GetPageSize() - 1; 
					break;
				case VK_PRIOR: 
					m_nSelected -= m_ScrollBar.GetPageSize() - 1; 
					break;
				case VK_HOME: 
					m_nSelected = 0; 
					break;
				case VK_END: 
					m_nSelected = ( int )m_vecItems.size() - 1; 
					break;
			}

			if( m_nSelected < 0 )
			{
				m_nSelected = 0;
			}
			else if( m_nSelected >= ( int )m_vecItems.size() )
			{
				m_nSelected = ( int )m_vecItems.size() - 1;
			}

			if( nOldSelected != m_nSelected )
			{
				if( m_Property.ListBoxProperty.bMultiSelect )
				{
					int i;

					for( i = 0; i < ( int )m_vecItems.size(); i++ )
					{
						SListBoxItem *pItem;
						pItem = m_vecItems[i];
						pItem->bSelected = false;
					}
					if( GetKeyState( VK_SHIFT ) < 0 )
					{
						int nEnd;
						nEnd = max( m_nSelStart, m_nSelected );

						for( i = min( m_nSelStart, m_nSelected ); i <= nEnd; i++ )
						{
							m_vecItems[ i ]->bSelected = true;
						}
					}
					else
					{
						m_vecItems[ m_nSelected ]->bSelected = true;
						m_nSelStart = m_nSelected;
					}
				} 
				else
				{
					m_nSelStart = m_nSelected;
				}
				m_ScrollBar.ShowItem( m_nSelected );
				m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
			}
			return true;
		}

		case VK_SPACE:
			m_pParent->ProcessCommand( EVENT_LISTBOX_ITEM_DBLCLK, true, this );
			return true;
		}
	}

	return false;
}

bool CEtUIListBox::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	if( ( WM_LBUTTONDOWN == uMsg ) && ( !m_bFocus ) )
	{
		m_pParent->RequestFocus( this );
	}

	if( m_ScrollBar.HandleMouse( uMsg, fX, fY, wParam, lParam ) )
	{
		return true;
	}

	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
		{
#ifdef PRE_ADD_LISTBOX_ELLIPSIS
			if( m_pParent->IsShowTooltipDlg() )
				m_pParent->ShowTooltipDlg( this, false, L"" );
#endif // PRE_ADD_LISTBOX_ELLIPSIS
			if( ( m_vecItems.size() > 0 ) && ( m_SelectionCoord.IsInside( fX, fY ) ) )
			{
				int nClicked(-1);

				if( m_fTextHeight > 0.0f )
				{
					nClicked = m_ScrollBar.GetTrackPos() + ( int )( ( fY - m_TextCoord.fY ) / (m_fTextHeight+m_fLineSpace) );
				}

				if( ( nClicked >= m_ScrollBar.GetTrackPos() ) && ( nClicked < ( int )m_vecItems.size() ) &&
					( nClicked < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() ) )
				{
					SetCapture( m_pParent->GetHWnd() );
					m_bDrag = true;
					if( uMsg == WM_LBUTTONDBLCLK )
					{
						// If ReleaseCapture below is not called,
						// When performing an action by double-clicking a listbox item (e.g., channel selection at login or channel switch in-game)
						// Afterwards, GetCapture returns that handle.
						//
						// A representative problematic part was inside the CDnSlotButton::HandleMouse function.
						// After entering by double-clicking a channel in the listbox, until Alt-Tab or clicking any UI,
						// No mouse-over effects (shown when hovering over UI controls) appear on any UI.
						// Anyway, forcing ReleaseCapture like this resolved the issue above.
						ReleaseCapture();
						m_pParent->ProcessCommand( EVENT_LISTBOX_ITEM_DBLCLK, true, this );
						return true;
					}

					m_nSelected = nClicked;
					if( !( wParam & MK_SHIFT ) )
					{
						m_nSelStart = m_nSelected;
					}

					if( m_Property.ListBoxProperty.bMultiSelect )
					{
						SListBoxItem *pSelItem;

						pSelItem = m_vecItems[ m_nSelected ];
						if( ( wParam & ( MK_SHIFT|MK_CONTROL ) ) == MK_CONTROL )
						{
							pSelItem->bSelected = !pSelItem->bSelected;
						} 
						else
						{
							if( ( wParam & (MK_SHIFT|MK_CONTROL) ) == MK_SHIFT )
							{
								int nBegin = min( m_nSelStart, m_nSelected );
								int nEnd = max( m_nSelStart, m_nSelected );

								for( int i = 0; i < nBegin; i++ )
								{
									m_vecItems[ i ]->bSelected = false;
								}

								for( int i = nEnd + 1; i < ( int )m_vecItems.size(); i++ )
								{
									m_vecItems[ i ]->bSelected = false;
								}

								for( int i = nBegin; i <= nEnd; i++ )
								{
									m_vecItems[ i ]->bSelected = true;
								}
							} 
							else
							{
								if( ( wParam & ( MK_SHIFT|MK_CONTROL ) ) == ( MK_SHIFT|MK_CONTROL ) )
								{
									int nBegin, nEnd;
									bool bLastSelected;

									nBegin = min( m_nSelStart, m_nSelected );
									nEnd = max( m_nSelStart, m_nSelected );

									bLastSelected = m_vecItems[ m_nSelStart ]->bSelected;
									for( int i = nBegin + 1; i < nEnd; i++ )
									{
										m_vecItems[ i ]->bSelected = bLastSelected;
									}
									pSelItem->bSelected = true;
									m_nSelected = m_nSelStart;
								} 
								else
								{
									for( int i = 0; i < ( int )m_vecItems.size(); i++ )
									{
										m_vecItems[ i ]->bSelected = false;
									}
									pSelItem->bSelected = true;
								}
							}
						}
					}
					m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
				}
				return true;
			}
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			ReleaseCapture();
			m_bDrag = false;
			if( m_nSelected != -1 )
			{
				int nItemSize = (int)m_vecItems.size();
				int nEnd = max( m_nSelStart, m_nSelected );
				for( int i = min( m_nSelStart, m_nSelected ) + 1; i < nEnd; i++ )
				{
					if( nItemSize > i && nItemSize > m_nSelStart ) {
						m_vecItems[ i ]->bSelected = m_vecItems[ m_nSelStart ]->bSelected;
					}
				}

				if( nItemSize > m_nSelected && nItemSize > m_nSelStart ) {
					m_vecItems[ m_nSelected ]->bSelected = m_vecItems[ m_nSelStart ]->bSelected;
				}

				if( m_nSelStart != m_nSelected )
				{
					m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
				}

				m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION_END, true, this );
			}
			return false;
		}
		break;
	case WM_MOUSEMOVE:
		{
			if( m_bDrag )
			{
				int nItem;
				nItem = -1;
				if( m_fTextHeight > 0.0f )
				{
					nItem = m_ScrollBar.GetTrackPos() + ( int )( ( fY - m_TextCoord.fY ) / (m_fTextHeight+m_fLineSpace) );
				}

				if( ( nItem >= ( int )m_ScrollBar.GetTrackPos() ) && ( nItem < (int)m_vecItems.size() ) &&
					( nItem < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() ) )
				{
					m_nSelected = nItem;
					m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
				} 
				else
				{
					if( nItem < ( int )m_ScrollBar.GetTrackPos() )
					{
						m_ScrollBar.Scroll( -1 );
						m_nSelected = m_ScrollBar.GetTrackPos();
						m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
					} 
					else
					{
						if( nItem >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() )
						{
							m_ScrollBar.Scroll( 1 );
							m_nSelected = min( ( int )m_vecItems.size(), m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() ) - 1;
							m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
						}
					}
				}
			}
#ifdef PRE_ADD_LISTBOX_ELLIPSIS
			else
			{
				int nFocusedItem(-1);

				if( m_fTextHeight > 0.0f )
				{
					nFocusedItem = m_ScrollBar.GetTrackPos() + ( int )( ( fY - m_TextCoord.fY ) / (m_fTextHeight+m_fLineSpace) );
				}

				if( ( nFocusedItem >= m_ScrollBar.GetTrackPos() ) && ( nFocusedItem < ( int )m_vecItems.size() ) &&
					( nFocusedItem < m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() ) )
				{
					if( m_vecItems[nFocusedItem]->bHasEllipsis )
						m_pParent->ShowTooltipDlg( this, true, m_vecItems[nFocusedItem]->strText );
				}

				if( IsInside(fX, fY) == false && m_pParent->IsShowTooltipDlg() )
					m_pParent->ShowTooltipDlg( this, false, L"" );
			}
#endif // PRE_ADD_LISTBOX_ELLIPSIS
		}
		break;
	case WM_MOUSEWHEEL:
		{
			UINT uLines;
			int nScrollAmount;

			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			// Using the default line count scrolls 3 lines at a time. Scrolling one line at a time looks better for listboxes.
			//m_ScrollBar.Scroll( -nScrollAmount );
			if( nScrollAmount > 0 ) m_ScrollBar.Scroll( -1 );
			else m_ScrollBar.Scroll( 1 );

#ifdef PRE_ADD_LISTBOX_ELLIPSIS
			if( m_pParent->IsShowTooltipDlg() )
				m_pParent->ShowTooltipDlg( this, false, L"" );
#endif // PRE_ADD_LISTBOX_ELLIPSIS

			return true;
		}
		break;
	}

	return false;
}

bool CEtUIListBox::MsgProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( WM_CAPTURECHANGED == uMsg )
	{
		if( ( HWND )lParam != m_pParent->GetHWnd() )
		{
			m_bDrag = false;
		}
	}

	return false;
}

void CEtUIListBox::UpdateRects()
{
	m_SelectionCoord = m_Property.UICoord;
	m_SelectionCoord.fWidth -= m_Property.ListBoxProperty.fScrollBarSize;
	m_SelectionCoord.fX += m_Property.ListBoxProperty.fBorder;
	m_SelectionCoord.fY += m_Property.ListBoxProperty.fBorder;
	m_SelectionCoord.fWidth -= m_Property.ListBoxProperty.fBorder*2.0f;
	m_SelectionCoord.fHeight -= m_Property.ListBoxProperty.fBorder*2.0f;

	m_TextCoord = m_SelectionCoord;
	m_TextCoord.fX += m_Property.ListBoxProperty.fMargin;
	m_TextCoord.fWidth -= m_Property.ListBoxProperty.fMargin;

	m_ScrollBar.SetPosition( m_Property.UICoord.Right() - m_Property.ListBoxProperty.fScrollBarSize, m_Property.UICoord.fY );
	m_ScrollBar.SetSize( m_Property.ListBoxProperty.fScrollBarSize, m_Property.UICoord.fHeight );

	SUIElement *pElement(NULL);
	pElement = GetElement(0);
	if( pElement == NULL )
		return;

	m_fTextHeight = m_pParent->GetFontHeight( pElement->nFontIndex, pElement->nFontHeight );
	m_ScrollBar.SetPageSize( (int)( m_TextCoord.fHeight / (m_fTextHeight+m_fLineSpace) ) );
	m_ScrollBar.ShowItem( m_nSelected );
}

void CEtUIListBox::DrawText( LPCWSTR szText, SUIElement *pElement, DWORD dwFontColor, SUICoord &Coord  )
{
	SUICoord ScreenCoord;
	ScreenCoord.fX = ( Coord.fX + m_pParent->GetXCoord() ) / m_pParent->GetScreenWidthRatio();
	ScreenCoord.fY = ( Coord.fY + m_pParent->GetYCoord() ) / m_pParent->GetScreenHeightRatio();
	ScreenCoord.fWidth = Coord.fWidth / m_pParent->GetScreenWidthRatio();
	ScreenCoord.fHeight = Coord.fHeight / m_pParent->GetScreenHeightRatio();

	float fStartX = ScreenCoord.fX;

	DWORD dwFontFormat = pElement->dwFontFormat;
	dwFontFormat |= DT_WORDBREAK;

	SFontDrawEffectInfo Info;
	Info.nDrawType = pElement->nDrawType;
	Info.nWeight = pElement->nWeight;
	Info.fAlphaWeight = pElement->fAlphaWeight;	
	Info.dwEffectColor = pElement->ShadowFontColor.dwCurrentColor;

	std::wstring szStr = szText;

	std::deque< DWORD > ColorStack;

	ColorStack.push_front( dwFontColor );
	bool bFirst = true;

	while( !szStr.empty() ) {
		std::wstring::size_type nPos = szStr.find_first_of( L'@' );
		Info.dwFontColor = ColorStack.front();
		std::wstring szCurStr = szStr.substr(0, nPos);
		
		CEtFontMng::GetInstance().DrawTextW( pElement->nFontIndex, pElement->nFontHeight, szCurStr.c_str(), dwFontFormat, ScreenCoord, -1, Info, true, 0.f );
		SUICoord FontCoord;
		CEtFontMng::GetInstance().CalcTextRect( pElement->nFontIndex, pElement->nFontHeight, szCurStr.c_str(), dwFontFormat, FontCoord, -1 );
		
		ScreenCoord.fX += FontCoord.fWidth;
		
		bFirst = false;

		if( nPos == std::wstring::npos ) break;
		szStr = szStr.substr( nPos+1 );
		if( szStr.compare(0, 4, L"MOVE") == 0 ) {
			szStr = szStr.substr( 5 );

			float fPos = 0.f;
			const wchar_t *pString =  szStr.c_str();
			swscanf_s(pString, L"%f", &fPos);
			const wchar_t *pEnd = wcschr(pString, L')' );

			szStr = szStr.substr( pEnd-pString+1 );
			fPos /= m_pParent->GetScreenWidthRatio();
			ScreenCoord.fX = fStartX + fPos;
		}
		else if( szStr.compare(0, 3, L"RGB") == 0 ) {
			szStr = szStr.substr( 4 );

			wchar_t szNum[16]={0,};
			int nRed = -1, nGreen = -1, nBlue = -1;
			swscanf_s(szStr.c_str(), L"%d", &nRed);
			wsprintfW(szNum, L"%d", nRed);
			szStr = szStr.substr( wcslen(szNum)+1 );
			swscanf_s(szStr.c_str(), L"%d", &nGreen);
			wsprintfW(szNum, L"%d", nGreen);
			szStr = szStr.substr( wcslen(szNum)+1 );
			swscanf_s(szStr.c_str(), L"%d", &nBlue);
			wsprintfW(szNum, L"%d", nBlue);
			szStr = szStr.substr( wcslen(szNum)+1 );
			assert(nRed>=0&&nRed<=255);
			assert(nGreen>=0&&nGreen<=255);
			assert(nBlue>=0&&nBlue<=255);
			ColorStack.push_front( D3DCOLOR_XRGB(nRed, nGreen, nBlue) );
		}
		else if( szStr.compare(0, 4, L"/RGB") == 0 ) {
			szStr = szStr.substr( 4 );
			ColorStack.pop_front();
		}
	}
}

void CEtUIListBox::Render( float fElapsedTime )
{
	if( !IsShow() )
		return;

	UpdateBlendRate();

	SUIElement *pElement(NULL);
	SUIElement *pSelElement(NULL);
	SUIElement *pBackElement(NULL);

	pElement = GetElement(0);
	pElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	pElement->FontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	pElement->ShadowFontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );

	pSelElement = GetElement(1);
	pSelElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	pSelElement->FontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	pSelElement->ShadowFontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );

	if( GetElementCount() > 2 )
	{
		pBackElement = GetElement(2);
		pBackElement->TextureColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
		pBackElement->FontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
		pBackElement->ShadowFontColor.Blend( UI_STATE_NORMAL, fElapsedTime, m_fBlendRate );
	}

	if( m_bExistTemplateTexture )
		m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
	else
		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, m_Property.UICoord );
	//m_pParent->DrawRect( m_Property.UICoord, EtInterface::debug::GREEN );

	if( m_vecItems.size() > 0 )
	{
		SUICoord TextCoord(m_TextCoord);
		SUICoord SelectionCoord(m_SelectionCoord);
		SUICoord backCoord(TextCoord);

		TextCoord.fY += m_fLineSpace;
		TextCoord.fHeight = m_fTextHeight;
		if( pSelElement ) SelectionCoord.SetSize( pSelElement->fTextureWidth, pSelElement->fTextureHeight );
		if( pBackElement ) backCoord.SetSize( pBackElement->fTextureWidth, pBackElement->fTextureHeight );

		static bool bSBInit = false;
		if( !bSBInit )
		{
			m_ScrollBar.SetPageSize( ( int )( m_TextCoord.fHeight / (TextCoord.fHeight+m_fLineSpace) ) );
			bSBInit = true;
		}

		for( int i = m_ScrollBar.GetTrackPos(); i < ( int )m_vecItems.size(); i++ )
		{
			if( TextCoord.Bottom() > m_TextCoord.Bottom() )
			{
				break;
			}

			bool bSelectedStyle = false;

			if( !( m_Property.ListBoxProperty.bMultiSelect ) && i == m_nSelected )
			{
				bSelectedStyle = true;
			}
			else
			{
				if( m_Property.ListBoxProperty.bMultiSelect )
				{
					if( ( m_bDrag ) && ( ( i >= m_nSelected && i < m_nSelStart ) || ( i <= m_nSelected && i > m_nSelStart ) ) )
					{
						bSelectedStyle = m_vecItems[ m_nSelStart ]->bSelected;
					}
					else
					{
						if( m_vecItems[ i ]->bSelected )
						{
							bSelectedStyle = true;
						}
					}
				}
			}

			if( pBackElement )
			{
				float fWidth = backCoord.fWidth - TextCoord.fWidth;
				backCoord.fX = TextCoord.fX - (fWidth/2.0f);

				float fHeight = backCoord.fHeight - TextCoord.fHeight;
				backCoord.fY = TextCoord.fY - (fHeight/2.0f);
				
				if( m_bExistTemplateTexture )
					m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pBackElement->TemplateUVCoord, pBackElement->TextureColor.dwCurrentColor, backCoord );
				else
					m_pParent->DrawSprite( pBackElement->UVCoord, pBackElement->TextureColor.dwCurrentColor, backCoord );
				//m_pParent->DrawRect( backCoord, EtInterface::debug::RED );
			}


			if( bSelectedStyle )
			{
				float fWidth = SelectionCoord.fWidth - TextCoord.fWidth;
				SelectionCoord.fX = TextCoord.fX - (fWidth/2.0f);

				float fHeight = SelectionCoord.fHeight - TextCoord.fHeight;
				SelectionCoord.fY = TextCoord.fY - (fHeight/2.0f);
				
				if( m_bExistTemplateTexture )
					m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pSelElement->TemplateUVCoord, pSelElement->TextureColor.dwCurrentColor, SelectionCoord );
				else
					m_pParent->DrawSprite( pSelElement->UVCoord, pSelElement->TextureColor.dwCurrentColor, SelectionCoord );

				EtColor Color;
				EtColorModulate(&Color, &EtColor(pSelElement->FontColor.dwCurrentColor), &EtColor(m_vecItems[i]->dwColor));

#ifdef PRE_ADD_LISTBOX_ELLIPSIS
				if( m_vecItems[i]->bHasEllipsis )
					DrawText( m_vecItems[i]->strEllipsisText, pSelElement, Color, TextCoord );
				else
					DrawText( m_vecItems[i]->strText, pSelElement, Color, TextCoord );
#else // PRE_ADD_LISTBOX_ELLIPSIS
				DrawText( m_vecItems[i]->strText, pSelElement, Color, TextCoord );
#endif // PRE_ADD_LISTBOX_ELLIPSIS
				//m_pParent->DrawDlgText( m_vecItems[i]->strText, pSelElement, pSelElement->FontColor.dwCurrentColor, TextCoord );
				//m_pParent->DrawRect( SelectionCoord, EtInterface::debug::BLUE );
				//m_pParent->DrawRect( TextCoord, EtInterface::debug::YELLOW );
			}
			else
			{			
				EtColor Color;
				EtColorModulate(&Color, &EtColor(pElement->FontColor.dwCurrentColor), &EtColor(m_vecItems[i]->dwColor));				
#ifdef PRE_ADD_LISTBOX_ELLIPSIS
				if( m_vecItems[i]->bHasEllipsis )
					DrawText( m_vecItems[i]->strEllipsisText, pElement, Color, TextCoord );
				else
					DrawText( m_vecItems[i]->strText, pElement, Color, TextCoord );
#else // PRE_ADD_LISTBOX_ELLIPSIS
				DrawText( m_vecItems[i]->strText, pElement, Color, TextCoord );
#endif // PRE_ADD_LISTBOX_ELLIPSIS
				//m_pParent->DrawDlgText( m_vecItems[i]->strText, pElement, pElement->FontColor.dwCurrentColor, TextCoord );
				//m_pParent->DrawRect( TextCoord, EtInterface::debug::RED );
			}

			TextCoord.fY += m_fLineSpace;
			TextCoord.fY += TextCoord.fHeight;
		}
	}

	m_ScrollBar.Render( fElapsedTime );
}

void CEtUIListBox::FindInputPos( std::vector<EtVector2> &vecPos )
{
	CEtUIDialog *pDialog = GetParent();
	if( !pDialog ) return;

	float fPosX = 0.0f;
	float fPosY = 0.0f;

	if( m_vecItems.size() > 0 )
	{
		SUICoord TextCoord(m_TextCoord);
		TextCoord.fY += m_fLineSpace;
		TextCoord.fHeight = m_fTextHeight;

		for( int i = m_ScrollBar.GetTrackPos(); i < ( int )m_vecItems.size(); i++ )
		{
			if( i >= m_ScrollBar.GetTrackPos() + m_ScrollBar.GetPageSize() )
			{
				break;
			}

			fPosX = pDialog->GetXCoord() + TextCoord.fX + TextCoord.fWidth / 2.0f;
			fPosY = pDialog->GetYCoord() + TextCoord.fY + TextCoord.fHeight / 2.0f;

			TextCoord.fY += m_fLineSpace;
			TextCoord.fY += TextCoord.fHeight;

			vecPos.push_back( EtVector2(fPosX, fPosY) );
		}

		m_ScrollBar.FindInputPos( vecPos );
	}
}