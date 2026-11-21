#include "StdAfx.h"
#include "EtUIListBoxEx.h"
#include "EtUIDialog.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUIListBoxEx::CEtUIListBoxEx( CEtUIDialog *pParent )
: CEtUIListBox( pParent )
{
	m_Property.UIType = UI_CONTROL_LISTBOXEX;
	m_bRenderScrollBar = true;
	m_bRenderSelectBar = true;
	m_bReturnHandleMouseProcessed = false;
}

CEtUIListBoxEx::~CEtUIListBoxEx(void)
{
}

void CEtUIListBoxEx::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.ListBoxExProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.ListBoxExProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_fLineSpace = m_Property.ListBoxExProperty.fLineSpace;
	UpdateRects();
}

void CEtUIListBoxEx::RemoveItem( int nIndex )
{
	if( ( nIndex < 0 ) || ( nIndex >= ( int )m_vecItems.size() ) )
	{
		return;
	}

	// Cast before delete to invoke virtual destructor
	CEtUIDialog *pBase = (CEtUIDialog *)m_vecItems[ nIndex ]->pData;
#ifdef _LISTITEM_CHILD
	m_pParent->DelChildDialog( pBase );
#endif
	delete pBase;
	delete m_vecItems[ nIndex ];
	m_vecItems.erase( m_vecItems.begin() + nIndex );
	m_ScrollBar.SetTrackRange( 0, ( int )m_vecItems.size() );
	if( m_nSelected >= ( int )m_vecItems.size() )
	{
		m_nSelected = ( int )m_vecItems.size() - 1;
	}
	m_pParent->ProcessCommand( EVENT_LISTBOX_SELECTION, true, this );
}

void CEtUIListBoxEx::RemoveAllItems()
{
	for( DWORD i = 0; i<m_vecItems.size(); i++ )
	{
		if( m_vecItems[i] )
		{
			// Cast before delete to invoke virtual destructor
			CEtUIDialog *pBase = (CEtUIDialog *)m_vecItems[i]->pData;
#ifdef _LISTITEM_CHILD
			m_pParent->DelChildDialog( pBase );
#endif
			delete pBase;
			delete m_vecItems[i];
		}
	};
	SAFE_DELETE_VEC(m_vecItems);

	m_nSelected = -1;
	m_ScrollBar.SetTrackRange( 0, 1 );
}

void CEtUIListBoxEx::CheckListBoxItem()
{
	// If a UI file to use as an item is specified in the UI tool, check the dialog name.
	if( m_Property.ListBoxExProperty.szListBoxItemUIFileName[0] != '\0' )
	{
		CEtUIDialog *pDialog = (CEtUIDialog *)m_vecItems[0]->pData;
		char szDialogFileName[128];
		_GetFullFileName(szDialogFileName, _countof(szDialogFileName), pDialog->GetDialogFileName());
		if( stricmp( m_Property.ListBoxExProperty.szListBoxItemUIFileName, szDialogFileName ) != 0 )
		{
			ASSERT(0&&"A UI file different from the one set for the list box item was added as an item.");
		}
	}

	// Due to item addition 
	if( m_vecItems.size() == 0 )
	{

	}
	else if( m_vecItems.size() == 1 )
	{
		CEtUIDialog *pDialog = (CEtUIDialog *)m_vecItems[0]->pData;
		m_fTextHeight = pDialog->Height();
		m_ScrollBar.SetPageSize( (int)( m_TextCoord.fHeight / (m_fTextHeight+m_fLineSpace) ) );
		m_ScrollBar.ShowItem( m_nSelected );
		UpdateRects();
	}
	else
	{
		// From the second one onward,
		CEtUIDialog *pFirstDialog = (CEtUIDialog *)m_vecItems[0]->pData;
		CEtUIDialog *pLastDialog = (CEtUIDialog *)m_vecItems[m_vecItems.size()-1]->pData;

		// Need to check if it is the same type of dialog as the first one that came in,
		// Just check width and height.
		// As long as the size fits into an item, it should be fine for now..
		if( (pFirstDialog->Width() != pLastDialog->Width()) || (pFirstDialog->Height() != pLastDialog->Height()) )
		{
			ASSERT(0&&"A different type of dialog was added to an extended list box item.");
		}
	}
}

bool CEtUIListBoxEx::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
#ifdef _LISTITEM_CHILD
#else
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_XBUTTONDBLCLK:
		if( !m_bDrag )
		{
			// Iterate only over existing list items.
			if( m_vecItems.size() > 0 )
			{
				SUICoord TextCoord(m_TextCoord);
				SUICoord SelectionCoord(m_SelectionCoord);
				SUICoord backCoord(TextCoord);

				TextCoord.fY += m_fLineSpace;
				TextCoord.fHeight = m_fTextHeight;

				for( int i = m_ScrollBar.GetTrackPos(); i < ( int )m_vecItems.size(); i++ )
				{
					if( TextCoord.Bottom() > m_TextCoord.Bottom() )
					{
						break;
					}

					CEtUIDialog *pItemDlg = (CEtUIDialog *)m_vecItems[i]->pData;
					if( pItemDlg )
					{
						// Initially, it was in this form.
						// Calls message procedures in order and finishes once handled...
						//pItemDlg->Show( true );
						//if( pItemDlg->MsgProc( m_pParent->GetHWnd(), uMsg, wParam, lParam ) )
						//{
						//	pItemDlg->Show( false );
						//	break;
						//}
						//pItemDlg->Show( false );

						// However, simply calling the message procedure like above does too many things inside.
						// A typical issue is that the scrollbar should light up on mouseover, but...
						// Because of this procedure, the item dialog's MsgProc->OnMouseMove gets called, so
						// NULL gets assigned to the mouse-over static variable.
						// So to the child controls of the dialog that owns this extended listbox, including the scrollbar,
						// Mouse-over will not be triggered.
						// So handle the control directly as shown below.
						POINT MousePoint;
						float fMouseX, fMouseY;
						MousePoint.x = short( LOWORD( lParam ) );
						MousePoint.y = short( HIWORD( lParam ) );
						pItemDlg->PointToFloat( MousePoint, fMouseX, fMouseY );

						if( pItemDlg->IsUsableFocusControl() )
						{
							if( focus::HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
							{
								return true;
							}
						}

						CEtUIControl* pControl = pItemDlg->GetControlAtPoint( fMouseX, fMouseY );
						if( pControl )
						{
							if( pControl != pItemDlg->s_pMouseEnterControl )
								pItemDlg->SetMouseEnterControl( pControl );

							// When a control is processed within a list item
							if( pControl->HandleMouse( uMsg, fMouseX, fMouseY, wParam, lParam ) )
							{
								// Returning prevents the list item selection message from being sent, so it won't be selected (default false)
								if( m_bReturnHandleMouseProcessed )
									return true;

								// Breaking will perform selection along with button check.
								break;
							}
						}
						// If an EditBox is added to the listbox item dialog in the future, this will probably be needed.
						//else
						//{
						//	if( uMsg == WM_LBUTTONDOWN )
						//	{
						//		if( focus::IsSameParent(pItemDlg) )
						//		{
						//			focus::ReleaseControl();
						//		}
						//	}
						//}
					}

					TextCoord.fY += m_fLineSpace;
					TextCoord.fY += TextCoord.fHeight;
				}
			}
		}
		break;
	}
#endif
	return CEtUIListBox::HandleMouse( uMsg, fX, fY, wParam, lParam);
}

void CEtUIListBoxEx::Render( float fElapsedTime )
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

		for( int i = 0; i < (int)m_vecItems.size(); ++i )
		{
			CEtUIDialog *pItemDlg = (CEtUIDialog *)m_vecItems[i]->pData;
			if( pItemDlg )
				pItemDlg->SetElementDialogShowState( false );
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

			CEtUIDialog *pItemDlg = (CEtUIDialog *)m_vecItems[i]->pData;
			if( pItemDlg )
			{
				SUICoord DlgCoord;
				pItemDlg->GetDlgCoord(DlgCoord);
				// Align to the center like the select bar rather than to the left.
				//DlgCoord.fX = m_pParent->GetXCoord() + TextCoord.fX;
				float fWidth = DlgCoord.fWidth - TextCoord.fWidth;
				DlgCoord.fX = m_pParent->GetXCoord() + TextCoord.fX - (fWidth/2.0f);
				DlgCoord.fY = m_pParent->GetYCoord() + TextCoord.fY;
				pItemDlg->SetDlgCoord(DlgCoord);
				// Managing via normal Show() puts it in dialog list, making management hard (due to auto batch handling).
				// So UI_TYPE_SELF came to be used as the dialog creation type.
				// If set to SELF, it is not included in EtUIDialogBase groups, so even with Show(true), it will not go through batch processing.
				// Manage by directly calling message processing and render in this state.

#ifdef _LISTITEM_CHILD
				m_pParent->ShowChildDialog( pItemDlg, true );
#else
				pItemDlg->SetElementDialogShowState( true );
				pItemDlg->Process( fElapsedTime );	// Call process only for visible dialogs
				pItemDlg->Render( fElapsedTime );
#endif
			}

			if( bSelectedStyle && m_bRenderSelectBar )
			{
				// If the selected item texture differs from the dialog size, scale it.
				if( (m_Property.ListBoxExProperty.nSelectFrameLeft > 0) || (m_Property.ListBoxExProperty.nSelectFrameTop > 0) ||
					(m_Property.ListBoxExProperty.nSelectFrameRight > 0) || (m_Property.ListBoxExProperty.nSelectFrameBottom > 0) )
				{
					SUICoord DlgCoord;
					if( pItemDlg ) pItemDlg->GetDlgCoord(DlgCoord);

					float fWidth = DlgCoord.fWidth - TextCoord.fWidth;
					SelectionCoord.fX = TextCoord.fX - (fWidth/2.0f);

					float fHeight = DlgCoord.fHeight - TextCoord.fHeight;
					SelectionCoord.fY = TextCoord.fY - (fHeight/2.0f);

					for( int j=0; j<9; j++ ) {
						SUICoord SplitAddUICoord(SelectionCoord);
						SplitAddUICoord.fWidth = m_SplitSelectCoord[j].fWidth;
						SplitAddUICoord.fHeight = m_SplitSelectCoord[j].fHeight;
						SplitAddUICoord.fX += m_SplitSelectCoord[j].fX;
						SplitAddUICoord.fY += m_SplitSelectCoord[j].fY;
						if( m_bExistTemplateTexture )
							m_pParent->DrawSprite( m_Template.m_hTemplateTexture, m_SplitSelectUVCoord[j], pElement->TextureColor.dwCurrentColor, SplitAddUICoord );
						else
							m_pParent->DrawSprite( m_SplitSelectUVCoord[j], pElement->TextureColor.dwCurrentColor, SplitAddUICoord );
					}
				}
				else
				{
					float fWidth = SelectionCoord.fWidth - TextCoord.fWidth;
					SelectionCoord.fX = TextCoord.fX - (fWidth/2.0f);

					float fHeight = SelectionCoord.fHeight - TextCoord.fHeight;
					SelectionCoord.fY = TextCoord.fY - (fHeight/2.0f);

					if( m_bExistTemplateTexture )
						m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pSelElement->TemplateUVCoord, pSelElement->TextureColor.dwCurrentColor, SelectionCoord );
					else
						m_pParent->DrawSprite( pSelElement->UVCoord, pSelElement->TextureColor.dwCurrentColor, SelectionCoord );
				}

				EtColor Color;
				EtColorModulate(&Color, &EtColor(pSelElement->FontColor.dwCurrentColor), &EtColor(m_vecItems[i]->dwColor));				
				DrawText( m_vecItems[i]->strText, pSelElement, Color, TextCoord );
				//m_pParent->DrawDlgText( m_vecItems[i]->strText, pSelElement, pSelElement->FontColor.dwCurrentColor, TextCoord );
				//m_pParent->DrawRect( SelectionCoord, EtInterface::debug::BLUE );
				//m_pParent->DrawRect( TextCoord, EtInterface::debug::YELLOW );
			}
			else
			{			
				EtColor Color;
				EtColorModulate(&Color, &EtColor(pElement->FontColor.dwCurrentColor), &EtColor(m_vecItems[i]->dwColor));				
				DrawText( m_vecItems[i]->strText, pElement, Color, TextCoord );
				//m_pParent->DrawDlgText( m_vecItems[i]->strText, pElement, pElement->FontColor.dwCurrentColor, TextCoord );
				//m_pParent->DrawRect( TextCoord, EtInterface::debug::RED );
			}

			TextCoord.fY += m_fLineSpace;
			TextCoord.fY += TextCoord.fHeight;
		}
	}

	if( m_bRenderScrollBar )
		m_ScrollBar.Render( fElapsedTime );
}

void CEtUIListBoxEx::UpdateRects()
{
	CEtUIListBox::UpdateRects();

	if( m_vecItems.size() > 0 )
	{
		CEtUIDialog *pDialog = (CEtUIDialog *)m_vecItems[0]->pData;
		m_fTextHeight = pDialog->Height();
		m_ScrollBar.SetPageSize( (int)( m_TextCoord.fHeight / (m_fTextHeight+m_fLineSpace) ) );
		m_ScrollBar.ShowItem( m_nSelected );
	}

	SUIElement *pElement = GetElement(1);
	if( !pElement ) return;

	// If split rendering is needed for the select texture,
	if( (m_Property.ListBoxExProperty.nSelectFrameLeft > 0) || (m_Property.ListBoxExProperty.nSelectFrameTop > 0) ||
		(m_Property.ListBoxExProperty.nSelectFrameRight > 0) || (m_Property.ListBoxExProperty.nSelectFrameBottom > 0))
	{
		float fFrameLeft(0.0f), fFrameTop(0.0f), fFrameRight(0.0f), fFrameBottom(0.0f);
		float fModWidth(0.0f), fModHeight(0.0f);

		EtTextureHandle hTexture = m_pParent->GetUITexture();
		if( m_bExistTemplateTexture )
			hTexture = m_Template.m_hTemplateTexture;
		if( hTexture )
		{
			int nTextureWidth = hTexture->Width();
			int nTextureHeight = hTexture->Height();

			fFrameLeft = m_Property.ListBoxExProperty.nSelectFrameLeft / ( float )nTextureWidth;
			fFrameTop = m_Property.ListBoxExProperty.nSelectFrameTop / ( float )nTextureHeight;
			fFrameRight = m_Property.ListBoxExProperty.nSelectFrameRight / ( float )nTextureWidth;
			fFrameBottom = m_Property.ListBoxExProperty.nSelectFrameBottom / ( float )nTextureHeight;

			if( pElement )
			{
				if( pElement->fTextureWidth > 0.0f && pElement->fTextureHeight > 0.0f )
				{
					fModWidth = ((pElement->fTextureWidth * DEFAULT_UI_SCREEN_WIDTH) / nTextureWidth) - (fFrameLeft+fFrameRight);
					fModHeight = ((pElement->fTextureHeight * DEFAULT_UI_SCREEN_HEIGHT) / nTextureHeight) - (fFrameTop+fFrameBottom);
				}
				else
				{
					fModWidth = (m_Property.UICoord.fWidth / nTextureWidth) - (fFrameLeft+fFrameRight);
					fModHeight = (m_Property.UICoord.fHeight / nTextureHeight) - (fFrameTop+fFrameBottom);
				}

				SUICoord UVCoord = pElement->UVCoord;
				if( m_bExistTemplateTexture )
					UVCoord = pElement->TemplateUVCoord;

				m_SplitSelectUVCoord[0].SetCoord( UVCoord.fX,						UVCoord.fY,							fFrameLeft,		fFrameTop );
				m_SplitSelectUVCoord[1].SetCoord( UVCoord.fX+fFrameLeft,			UVCoord.fY,							fModWidth,		fFrameTop );
				m_SplitSelectUVCoord[2].SetCoord( UVCoord.fX+fFrameLeft+fModWidth,	UVCoord.fY,							fFrameRight,	fFrameTop );
				m_SplitSelectUVCoord[3].SetCoord( UVCoord.fX,						UVCoord.fY+fFrameTop,				fFrameLeft,		fModHeight );
				m_SplitSelectUVCoord[4].SetCoord( UVCoord.fX+fFrameLeft,			UVCoord.fY+fFrameTop,				fModWidth,		fModHeight );
				m_SplitSelectUVCoord[5].SetCoord( UVCoord.fX+fFrameLeft+fModWidth,	UVCoord.fY+fFrameTop,				fFrameRight,	fModHeight );
				m_SplitSelectUVCoord[6].SetCoord( UVCoord.fX,						UVCoord.fY+fFrameTop+fModHeight,	fFrameLeft,		fFrameBottom );
				m_SplitSelectUVCoord[7].SetCoord( UVCoord.fX+fFrameLeft,			UVCoord.fY+fFrameTop+fModHeight,	fModWidth,		fFrameBottom );
				m_SplitSelectUVCoord[8].SetCoord( UVCoord.fX+fFrameLeft+fModWidth,	UVCoord.fY+fFrameTop+fModHeight,	fFrameRight,	fFrameBottom );
			}
		}

		fFrameLeft = m_Property.ListBoxExProperty.nSelectFrameLeft / (float)DEFAULT_UI_SCREEN_WIDTH;
		fFrameTop = m_Property.ListBoxExProperty.nSelectFrameTop / (float)DEFAULT_UI_SCREEN_HEIGHT;
		fFrameRight = m_Property.ListBoxExProperty.nSelectFrameRight / (float)DEFAULT_UI_SCREEN_WIDTH;
		fFrameBottom = m_Property.ListBoxExProperty.nSelectFrameBottom / (float)DEFAULT_UI_SCREEN_HEIGHT;

		if( m_vecItems.size() > 0 )
		{
			CEtUIDialog *pDialog = (CEtUIDialog *)m_vecItems[0]->pData;
			if( pDialog ) {
				fModWidth = pDialog->Width() - (fFrameLeft+fFrameRight);
				fModHeight = pDialog->Height() - (fFrameTop+fFrameBottom);
			}
		}
		else if( pElement )
		{
			fModWidth = pElement->fTextureWidth - (fFrameLeft+fFrameRight);
			fModHeight = pElement->fTextureHeight - (fFrameTop+fFrameBottom);
		}

		m_SplitSelectCoord[0].SetCoord( 0.0f,					0.0f,					fFrameLeft,		fFrameTop );
		m_SplitSelectCoord[1].SetCoord( fFrameLeft,				0.0f,					fModWidth,		fFrameTop );
		m_SplitSelectCoord[2].SetCoord( fFrameLeft+fModWidth,	0.0f,					fFrameRight,	fFrameTop );
		m_SplitSelectCoord[3].SetCoord( 0.0f,					fFrameTop,				fFrameLeft,		fModHeight );
		m_SplitSelectCoord[4].SetCoord( fFrameLeft,				fFrameTop,				fModWidth,		fModHeight );
		m_SplitSelectCoord[5].SetCoord( fFrameLeft+fModWidth,	fFrameTop,				fFrameRight,	fModHeight );
		m_SplitSelectCoord[6].SetCoord( 0.0f,					fFrameTop+fModHeight,	fFrameLeft,		fFrameBottom );
		m_SplitSelectCoord[7].SetCoord( fFrameLeft,				fFrameTop+fModHeight,	fModWidth,		fFrameBottom );
		m_SplitSelectCoord[8].SetCoord( fFrameLeft+fModWidth,	fFrameTop+fModHeight,	fFrameRight,	fFrameBottom );
	}
}