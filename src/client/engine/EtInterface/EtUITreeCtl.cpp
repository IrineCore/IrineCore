#include "StdAfx.h"
#include "EtUITreeCtl.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUITreeCtl::CEtUITreeCtl( CEtUIDialog *pParent )
	: CEtUIControl( pParent )
	, m_ScrollBar( pParent )
	, m_bPressed(false)
	, m_nCurLine(-1)
	, m_nVisibleCount(0)
	, m_pSelectedItem(NULL)
{
	m_Property.UIType = UI_CONTROL_TREECONTROL;

	m_pRootItem = new CTreeItem();
	m_pRootItem->Expand();
}

CEtUITreeCtl::~CEtUITreeCtl(void)
{
	DeleteAllItems();
	SAFE_DELETE(m_pRootItem);
}

void CEtUITreeCtl::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.TreeControlProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.TreeControlProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_ScrollBar.SetTrackRange( 0, 0 );
}

void CEtUITreeCtl::Render( float fElapsedTime )
{
	// Note : Added for debugging
	//m_pParent->DrawRect( m_Property.UICoord, EtInterface::debug::BLUE );

	RenderTree( m_listTreeTravelItem );

	if( m_Property.TreeControlProperty.bVerticalScrollBar )
	{
		static int nOldPosition, nCurPosition;

		if( !m_ScrollBar.IsArrowStateClear() )
		{
			nOldPosition = m_ScrollBar.GetTrackPos();
		}

		m_ScrollBar.Render( fElapsedTime );

		if( !m_ScrollBar.IsArrowStateClear() )
		{
			nCurPosition = m_ScrollBar.GetTrackPos();
			Scroll( nCurPosition-nOldPosition );
		}
	}
}

void CEtUITreeCtl::RenderTree( TREEITEM_LIST &VecTree )
{	
	TREEITEM_LIST_ITER iter = VecTree.begin();
	TREEITEM_LIST_ITER iter_end = VecTree.end();
	for( ; iter != iter_end; ++iter )
	{
		RenderBase( *iter );
		RenderSelect( *iter );
		RenderButton( *iter );
		RenderClassIcon( *iter );
		RenderText( *iter );
		//m_pParent->DrawRect( (*iter)->m_uiCoord, debug::BLUE );
	}
}

void CEtUITreeCtl::RenderButton( CTreeItem *pItem )
{
	ASSERT(pItem&&"CEtUITreeCtl::RenderButton");

	SUIElement *pElement(NULL);

	if( pItem->HasChild() )
	{
		if( !pItem->IsExpanded() )
		{
			// Note : + image
			pElement = GetElement(0);
		}
		else
		{
			// Note : - image
			pElement = GetElement(1);
		}
	}

	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, pItem->m_ButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_ButtonCoord );
		//m_pParent->DrawRect( pItem->m_ButtonCoord, debug::RED );
	}
}

void CEtUITreeCtl::RenderText( CTreeItem *pItem )
{
	ASSERT(pItem&&"CEtUITreeCtl::RenderText");

	if( !pItem->IsOpened() )
		return;

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	if( pItem->IsSelected() )
	{
		if( !(pItem->GetItemType() & CTreeItem::typeMain) )
		{
			m_pParent->DrawDlgText( pItem->GetText(), pElement, 0xffffffff, pItem->m_strCoord, -1, -1, true );
			return;
		}	
	}

	m_pParent->DrawDlgText( pItem->GetText(), pElement, pItem->GetTextColor(), pItem->m_strCoord, -1, -1, true );
	//m_pParent->DrawRect( pItem->m_strCoord, debug::YELLOW );
}

void CEtUITreeCtl::RenderBase( CTreeItem *pItem )
{
	ASSERT(pItem&&"CEtUITreeCtl::RenderBase");

	SUIElement *pElement = GetElement(2);
	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, pItem->m_BaseCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_BaseCoord );
	}
}

void CEtUITreeCtl::RenderSelect( CTreeItem *pItem )
{
	ASSERT(pItem&&"CEtUITreeCtl::RenderSelect");

	if( !pItem->IsSelected() )
		return;

	SUIElement *pElement = GetElement(3);
	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, pItem->m_SelectCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_SelectCoord );
	}
}

void CEtUITreeCtl::RenderClassIcon( CTreeItem *pItem )
{
	ASSERT(pItem&&"CEtUITreeCtl::RenderClassIcon");

	CTreeItem::eClassIconType iconType = pItem->GetClassIconType();
	if( pItem->GetItemType() & CTreeItem::typeMain || iconType == CTreeItem::eNONE )
		return;

	SUIElement *pElement = GetElement(iconType);
	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, pItem->m_ClassIconCoord[0] );
		else	
			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_ClassIconCoord[0] );
	}
}

void CEtUITreeCtl::UpdateRects()
{
	UpdateRectsTree();

	if( m_Property.TreeControlProperty.bVerticalScrollBar )
	{
		m_ScrollBar.SetPosition( m_Property.UICoord.Right()-m_Property.TreeControlProperty.fScrollBarSize, m_Property.UICoord.fY );
		m_ScrollBar.SetSize( m_Property.TreeControlProperty.fScrollBarSize, m_Property.UICoord.fHeight );
	}
}

void CEtUITreeCtl::TravelTree()
{
	int nCount(-1);
	m_listTreeTravelItem.clear();
	m_pRootItem->TravelTree( m_listTreeTravelItem, nCount, m_nCurLine, m_nVisibleCount );
}

void CEtUITreeCtl::UpdateRectsTree()
{
	UpdateVisibleCount();
	TravelTree();

	float fX, fY;
	fX = m_Property.UICoord.fX;
	fY = m_Property.UICoord.fY;

	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		(*iter)->UpdatePos( fX, fY );
		fY += (*iter)->m_uiCoord.fHeight;
	}

	// Do not draw the scrollbar when it does not need to be visible.
	if ( m_ScrollBar.IsShowThumb() )
	{
		m_ScrollBar.Show(true);
	}
	else
	{
		m_ScrollBar.Show(false);
	}
}

CTreeItem* CEtUITreeCtl::AddItem( DWORD dwType, LPCWSTR szText, DWORD dwColor )
{
	SUICoord uiCoord;
	uiCoord = m_Property.UICoord;

	SUIElement *pElement(NULL);
	pElement = GetElement(0);

	uiCoord.fHeight = pElement->fTextureHeight;
	uiCoord.fWidth -= m_Property.TreeControlProperty.fScrollBarSize;

	if( m_pRootItem->HasChild() )
	{
		CTreeItem *pEndItem = m_pRootItem->GetEndItem();
		uiCoord.fY = pEndItem->m_uiCoord.Bottom();
	}
	else
	{
		m_nCurLine = 0;
	}

	CTreeItem *pItem = new CTreeItem();
	pItem->Initialize( this );
	pItem->SetUICoord(uiCoord);
	pItem->SetText( szText, dwColor );
	pItem->AddItemType( dwType );
	
	m_pRootItem->Add( pItem );
	pItem->SetParentItem( m_pRootItem );

	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
	UpdateRectsTree();

	return pItem;
}

CTreeItem* CEtUITreeCtl::AddChildItem( CTreeItem *pTreeItem, DWORD dwType, LPCWSTR szText, DWORD dwColor )
{
	ASSERT( pTreeItem&&"CEtUITreeCtl::AddChildItem, pTreeItem is NULL!" );

	SUICoord uiCoord(pTreeItem->GetCoord());
	uiCoord.fX += m_Property.TreeControlProperty.fIndentSize;
	uiCoord.fWidth -= m_Property.TreeControlProperty.fIndentSize;

	CTreeItem *pItem = new CTreeItem();
	pItem->Initialize( this, pTreeItem->GetDepth()+1 );
	pItem->SetUICoord( uiCoord );
	pItem->SetText( szText, dwColor );
	pItem->AddItemType( dwType );

	pTreeItem->Add( pItem );
	pItem->SetParentItem( pTreeItem );
		
	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
	UpdateRectsTree();

	return pItem;
}

bool CEtUITreeCtl::DeleteItem( CTreeItem *pTreeItem )
{
	bool bRet = m_pRootItem->Delete( pTreeItem );
	m_pSelectedItem = NULL;
	UpdateRectsTree();
	return bRet;
}

bool CEtUITreeCtl::DeleteItem( void *pData )
{
	bool bRet = m_pRootItem->Delete( pData );
	m_pSelectedItem = NULL;
	UpdateRectsTree();
	return bRet;
}

void CEtUITreeCtl::DeleteAllItems()
{
	m_pRootItem->DeleteAll();
	m_bPressed = false;
	m_nCurLine = -1;
	m_nVisibleCount = 0;
	m_pSelectedItem = NULL;
	m_ScrollBar.SetTrackRange( 0, 0 );
	UpdateRectsTree();
}

CTreeItem* CEtUITreeCtl::FindItemData( void *pData, bool bRecursive )
{
	return m_pRootItem->FindItemData( pData, bRecursive );
}

CTreeItem* CEtUITreeCtl::FindItemInt( int nValue, bool bRecursive )
{
	return m_pRootItem->FindItemInt( nValue, bRecursive );
}

CTreeItem* CEtUITreeCtl::FindItemInt64( INT64 nValue, bool bRecursive )
{
	return m_pRootItem->FindItemInt64( nValue, bRecursive );
}

CTreeItem* CEtUITreeCtl::FindItemFloat( float fValue, bool bRecursive )
{
	return m_pRootItem->FindItemFloat( fValue, bRecursive );
}

void CEtUITreeCtl::ExpandAll()
{
	m_pRootItem->ExpandAll();
	UpdateRectsTree();
}

void CEtUITreeCtl::ExpandAllChildren( const CTreeItem *pItem )
{
	if( pItem == NULL )
		return;

	if( m_pRootItem->ExpandAllChildren( pItem ) )
	{
		UpdateRectsTree();
	}
}

void CEtUITreeCtl::CollapseAll()
{
	m_pRootItem->CollapseAll();
	UpdateRectsTree();
}

CTreeItem* CEtUITreeCtl::GetBeginItem()
{
	return m_pRootItem->GetBeginItem();
}

CTreeItem* CEtUITreeCtl::GetEndItem()
{
	return m_pRootItem->GetEndItem();
}

bool CEtUITreeCtl::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	if( !IsEnable() || !IsShow() )
	{
		return false;
	}

	if( ( WM_LBUTTONDOWN == uMsg ) && ( !m_bFocus ) )
	{
		m_pParent->RequestFocus( this );
	}

	int nOldPosition = m_ScrollBar.GetTrackPos();

	if( m_ScrollBar.HandleMouse( uMsg, fX, fY, wParam, lParam ) )
	{
		int nCurPosition = m_ScrollBar.GetTrackPos();
		Scroll( nCurPosition-nOldPosition );
		return true;
	}

	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
			if( IsInside( fX, fY ) )
			{
				if( OverItem( fX, fY ) )
					return true;
				else
					CEtUIDialog::ShowTooltipDlg( NULL, false );
			}
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		{
			if( IsInside( fX, fY ) )
			{
				m_bPressed = true;
				SetCapture( m_pParent->GetHWnd() );

				if( !m_bFocus )
				{
					m_pParent->RequestFocus( this );
				}

				if( WM_LBUTTONDBLCLK == uMsg && SelectItem( fX, fY ) )
					DoubleClickExpand( fX, fY );

				return true;
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			if( m_bPressed )
			{
				m_bPressed = false;
				ReleaseCapture();

				if( IsInside( fX, fY ) )
				{
					if( !Expand( fX, fY ) )
					{
						if( SelectItem( fX, fY ) )
						{
							m_pParent->ProcessCommand( EVENT_TREECONTROL_SELECTION, true, this );
						}
						else
						{
							m_pParent->ProcessCommand( EVENT_TREECONTROL_SELECTION_END, true, this );
						}
					}
				}

				return true;
			}
		}
		break;
	case WM_MOUSEWHEEL:
		{
			UINT uLines;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
			int nScrollPos = Scroll( -nScrollAmount );
			m_ScrollBar.ShowItem( nScrollPos );

			ClearTooltipText();
			CEtUIDialog::ShowTooltipDlg( NULL, false );

			return true;
		}
		break;
	}

	return false;
}

void CEtUITreeCtl::DoubleClickExpand( float fX, float fY )
{
	if( NULL == m_pSelectedItem || m_pSelectedItem->GetButtonClickItem( fX, fY ) )
		return;

	bool bExpanded = m_pSelectedItem->IsExpanded();
	if( bExpanded )
		m_pSelectedItem->Collapse();
	else
		m_pSelectedItem->Expand();

	UpdateRectsTree();
}

bool CEtUITreeCtl::Expand( float fX, float fY )
{
	CTreeItem *pItem(NULL);

	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( !(*iter)->IsOpened() ) continue;
		if( !(*iter)->HasChild() ) continue;

		pItem = (*iter)->GetButtonClickItem( fX, fY );

		if( pItem )
		{
			bool bExpanded = pItem->IsExpanded();
			if( bExpanded )
				pItem->Collapse();
			else
				pItem->Expand();

			// When expanding, adjust only the scrollbar size without scrolling
			// When collapsing an expanded item, scroll first and then adjust the scrollbar size.
			// Does not strictly replicate Windows tree control, so modify later if needed.
			if( bExpanded )
			{
				int nCount(-1);
				pItem->GetChildCount(nCount);
				int nScrollPos = Scroll(-nCount);
				m_ScrollBar.ShowItem(nScrollPos);
			}
			UpdateRectsTree();
			return true;
		}
	}

	return false;
}
//#ifdef PRE_ADD_MISSION_NEST_TAB
void CEtUITreeCtl::Sort( bool bSortStandard )
{	
	m_pRootItem->SortBySubCategory( bSortStandard );
}
//#endif 
void CEtUITreeCtl::SetSelectItem( CTreeItem *pItem )
{
	if( !pItem )
		return;

	ResetSelectedItem();
	pItem->Select();
	m_pSelectedItem = pItem;
}

void CEtUITreeCtl::ResetSelectedItem()
{
	if( m_pSelectedItem )
	{
		m_pSelectedItem->Unselect();
		m_pSelectedItem = NULL;
	}
}

bool CEtUITreeCtl::SelectItem( float fX, float fY )
{
	ResetSelectedItem();

	CTreeItem *pItem(NULL);

	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		if( !(*iter)->IsOpened() )
			continue;

		//if( (*iter)->GetItemType()&CTreeItem::typeMain )
		//	continue;

		pItem = (*iter)->GetClickItem( fX, fY );
		if( pItem )
		{
			m_pSelectedItem = pItem;
			m_pSelectedItem->Select();
			return true;
		}
	}

	return false;
}

bool CEtUITreeCtl::OverItem( float fX, float fY )
{
	CTreeItem *pItem(NULL);

	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		pItem = (*iter)->GetClickItem( fX, fY );
		if( pItem && pItem->IsWithEllipsis() )
		{
			std::wstring wszText = (*iter)->GetOrigText();
			CEtUIDialog::ShowTooltipDlg( this, true, wszText );
			return true;
		}
	}

	return false;
}

bool CEtUITreeCtl::UpdateVisibleCount()
{
	if( !m_pRootItem->HasChild() )
		return false;

	int nCount(-1);
	float fHeight(0.0f);

	m_pRootItem->GetVisibleCount( m_Property.UICoord.fHeight, fHeight, m_nCurLine, nCount );
	m_nVisibleCount = nCount - m_nCurLine;
	m_ScrollBar.SetPageSize( m_nVisibleCount );
	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );

	return true;
}

int CEtUITreeCtl::GetExpandedItemCount()
{
	int nTotalCount(-1);
	m_pRootItem->GetExpandedItemCount( nTotalCount );

	return nTotalCount;
}

bool CEtUITreeCtl::IsScrollMode()
{
	return m_nVisibleCount<GetExpandedItemCount();
}

int CEtUITreeCtl::Scroll( int nScrollAmount )
{
	int nScrollPos(0);

	if( nScrollAmount < 0 )
	{
		nScrollAmount = -nScrollAmount;

		for( int i=0; i<nScrollAmount; i++ )
		{
			if( m_nCurLine <= 0 )
				break;

			m_nCurLine--;
			UpdateRectsTree();
		}

		nScrollPos = m_nCurLine;
	}
	else if( nScrollAmount > 0 )
	{
		for( int i=0; i<nScrollAmount; i++ )
		{
			int nTemp = GetExpandedItemCount() - m_nCurLine - m_nVisibleCount;
			if( nTemp <= 0 )
				break;

			m_nCurLine++;
			UpdateRectsTree();
		}

		nScrollPos = m_nCurLine+m_nVisibleCount-1;
	}

	return nScrollPos;
}

void CEtUITreeCtl::ScrollToBegin()
{
	Scroll( -m_nCurLine );
	m_ScrollBar.ShowItem( 0 );
}

void CEtUITreeCtl::ScrollToEnd()
{
	int nExpandedItemCount = GetExpandedItemCount();
	Scroll( nExpandedItemCount-m_nVisibleCount-m_nCurLine );
	m_ScrollBar.ShowItem( nExpandedItemCount );
}

void CEtUITreeCtl::AddItemTypeChildren( CTreeItem *pTreeItem, DWORD dwType )
{
	ASSERT( pTreeItem&&"CEtUITreeCtl::AddItemType" );
	pTreeItem->AddItemTypeAllChildren( dwType );
}

void CEtUITreeCtl::FindInputPos( std::vector<EtVector2> &vecPos )
{
	CEtUIDialog *pDialog = GetParent();
	if( !pDialog ) return;

	float fPosX = 0.0f;
	float fPosY = 0.0f;

	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		CTreeItem *pItem = *iter;

		fPosX = pDialog->GetXCoord() + pItem->m_BaseCoord.fX + pItem->m_BaseCoord.fWidth / 2.0f;
		fPosY = pDialog->GetYCoord() + pItem->m_BaseCoord.fY + pItem->m_BaseCoord.fHeight / 2.0f;

		vecPos.push_back( EtVector2(fPosX, fPosY) );

		if( pItem->HasChild() )
		{
			// Must also handle input for + and - buttons.
			SUIElement *pElement(NULL);
			if( !pItem->IsExpanded() )
				pElement = GetElement(0);
			else
				pElement = GetElement(1);

			if( pElement )
			{
				fPosX = pDialog->GetXCoord() + pItem->m_ButtonCoord.fX + pItem->m_ButtonCoord.fWidth / 2.0f;
				fPosY = pDialog->GetYCoord() + pItem->m_ButtonCoord.fY + pItem->m_ButtonCoord.fHeight / 2.0f;

				vecPos.push_back( EtVector2(fPosX, fPosY) );
			}
		}
	}

	if( m_listTreeTravelItem.size() )
	{
		m_ScrollBar.FindInputPos( vecPos );
	}
}