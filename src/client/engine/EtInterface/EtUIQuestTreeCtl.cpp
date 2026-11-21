#include "StdAfx.h"
#include "EtUIQuestTreeCtl.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtUIQuestTreeCtl::CEtUIQuestTreeCtl( CEtUIDialog *pParent )
	: CEtUITreeCtl( pParent )
{
	m_Property.UIType = UI_CONTROL_QUESTTREECONTROL;
}

CEtUIQuestTreeCtl::~CEtUIQuestTreeCtl(void)
{
}

void CEtUIQuestTreeCtl::Initialize( SUIControlProperty *pProperty )
{
	CEtUIControl::Initialize( pProperty );

	/*m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.QuestTreeControlProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.QuestTreeControlProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_ScrollBar.SetTrackRange( 0, 0 );*/
}
//
//void CEtUIQuestTreeCtl::RenderTree( TREEITEM_LIST &VecTree )
//{	
//	TREEITEM_LIST_ITER iter = VecTree.begin();
//	TREEITEM_LIST_ITER iter_end = VecTree.end();
//	for( ; iter != iter_end; ++iter )
//	{
//		RenderFrame( *iter );
//		RenderBackImg( *iter );
//		RenderSelFrame( *iter );
//		RenderButton( *iter );
//		RenderState( *iter );
//		RenderText( *iter );
//		//m_pParent->DrawRect( (*iter)->m_uiCoord, debug::BLUE );
//	}
//}
//
////void CEtUIQuestTreeCtl::RenderButton( CTreeItem *pItem )
////{
////	ASSERT(pItem&&"CDnQuestTree::RenderButton");
////
////	SUIElement *pElement(NULL);
////
////	if( !pItem->IsOpened() )
////	{
////		// Note: x image
////		pElement = GetElement(9);
////	}
////	else
////	{
////		if( pItem->HasChild() )
////		{
////			if( pItem->GetDepth() == 1 )
////			{
////				if( !pItem->IsExpanded() )
////				{
////					// Note: + image
////					pElement = GetElement(0);
////				}
////				else
////				{
////					// Note: - image
////					pElement = GetElement(1);
////				}
////			}
////			else if( pItem->GetDepth() == 2 )
////			{
////				// Note: . image (SubButton)
////				pElement = GetElement(8);
////			}
////		}
////	}
////
////	if( pElement )
////	{
////		m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_ButtonCoord );
////		//m_pParent->DrawRect( VecTree[i]->m_ButtonCoord, debug::BLUE );
////	}
////}
//
//void CEtUIQuestTreeCtl::RenderState( CTreeItem *pItem )
//{
//	ASSERT(pItem&&"CDnQuestTree::RenderState");
//	CEtUIQuestTreeItem *pQuestItem = static_cast<CEtUIQuestTreeItem*>(pItem);
//
//	if( !pItem->IsOpened() )
//		return;
//
//	SUIElement *pElement(NULL);
//
//	if( pQuestItem->IsCompleted() )
//	{
//		pElement = GetElement(8);
//	}
//	else
//	{
//		pElement = GetElement(9);
//	}
//
//	if( pElement )
//	{
//		m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pQuestItem->m_StateCoord );
//		//m_pParent->DrawRect( pQuestItem->m_StateCoord, debug::RED );
//	}
//
///*
//
//	int nQuestIndex = (int)pItem->GetItemValueFloat();
//	if ( nQuestIndex > 0 )
//	{
//		pElement = NULL;
//
//		int nType = GetQuestTask().GetNotifierType(nQuestIndex);
//
//		switch( nType )
//		{
//		case QuestNotifier_Dynamic:
//			pElement = GetElement(10);
//			break;
//		case QuestNotifier_Active:
//			pElement = GetElement(11);
//			break;
//		default:
//			pElement = NULL;
//			break;
//		}
//
//		if( pElement )
//		{
//			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pQuestItem->m_NotifierCoord );
//			//m_pParent->DrawRect( pQuestItem->m_StateCoord, debug::RED );
//		}
//	}
//
//
//	if( pQuestItem->IsDynamicNotifier() )
//	{
//		pElement = GetElement(10);
//	}
//	else if( pQuestItem->IsActiveNotifier() )
//	{
//		pElement = GetElement(11);
//	}
//
//	if( pElement )
//	{
//		m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pQuestItem->m_NotifierCoord );
//		//m_pParent->DrawRect( pQuestItem->m_StateCoord, debug::RED );
//	}*/
//
//
//}
//
//void CEtUIQuestTreeCtl::RenderText( CTreeItem *pItem )
//{
//	ASSERT(pItem&&"CDnQuestTree::RenderText");
//
//	if( !pItem->IsOpened() )
//		return;
//
//	SUIElement *pElement = GetElement(0);
//	if( !pElement ) return;
//
//	//m_pParent->DrawRect( pItem->m_strCoord, debug::YELLOW );
//	DWORD nOriginalClr = pItem->GetTextColor();
//	if ( pItem->IsSelected() )
//	{
//		pItem->SetTextColor(EtInterface::textcolor::YELLOW);
//	}
//	else if ( pItem->IsCompleted() )
//	{
//		pItem->SetTextColor(EtInterface::textcolor::DARKGRAY);
//	}
//
//	m_pParent->DrawDlgText( pItem->GetText(), pElement, pItem->GetTextColor(), pItem->m_strCoord, -1, -1, true );
//	pItem->SetTextColor(nOriginalClr);
//}
//
//void CEtUIQuestTreeCtl::RenderBackImg( CTreeItem *pItem )
//{
//	ASSERT(pItem&&"CDnQuestTree::RenderBackImg");
//	CEtUIQuestTreeItem *pQuestItem = static_cast<CEtUIQuestTreeItem*>(pItem);
//
//	if ( pQuestItem->GetTexture() )
//	{
//		static SUICoord texUV(0.0f,0.0f, 1.0f, 1.0f);
//		SUIElement *pElement(NULL);
//		pElement = GetElement(pQuestItem->GetDepth()+1);
//		m_pParent->DrawSprite( pQuestItem->GetTexture(), texUV, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
//	}
//}
//
//void CEtUIQuestTreeCtl::RenderFrame( CTreeItem *pItem )
//{
//	ASSERT(pItem&&"CDnQuestTree::RenderFrame");
//	CEtUIQuestTreeItem *pQuestItem = static_cast<CEtUIQuestTreeItem*>(pItem);
//
//	SUIElement *pElement(NULL);
//	pElement = GetElement(pQuestItem->GetDepth()+1);
//	m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
//	//m_pParent->DrawSprite( pQuestItem->GetTexture(), pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
//	//m_pParent->DrawRect( pQuestItem->m_FrameCoord, debug::YELLOW );
//}
//
//void CEtUIQuestTreeCtl::RenderSelFrame( CTreeItem *pItem )
//{
//	ASSERT(pItem&&"CDnQuestTree::RenderSelFrame");
//	CEtUIQuestTreeItem *pQuestItem = static_cast<CEtUIQuestTreeItem*>(pItem);
//
//	if( pItem->IsSelected() )
//	{
//		SUIElement *pElement(NULL);
//		pElement = GetElement(pQuestItem->GetDepth()+4);
//		m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameSelectCoord );
//		
//		//m_pParent->DrawRect( pQuestItem->m_FrameSelectCoord, debug::YELLOW );
//	}
//}
//
////CEtUIQuestTreeItem* CEtUIQuestTreeCtl::AddQuestItem( DWORD dwType, LPCWSTR szText, DWORD dwColor )
////{
////	SUICoord uiCoord(m_Property.UICoord);
////
////	SUIElement *pElement(NULL);
////	pElement = GetElement(2);
////	if( !pElement ) return NULL;
////
////	uiCoord.fHeight = pElement->fTextureHeight;
////	uiCoord.fWidth = pElement->fTextureWidth;
////	//uiCoord.fWidth -= m_Property.QuestTreeControlProperty.fScrollBarSize;
////
////	if( m_pRootItem->HasChild() )
////	{
////		CTreeItem *pEndItem = m_pRootItem->GetEndItem();
////		uiCoord.fY = pEndItem->m_uiCoord.Bottom();
////	}
////	else
////	{
////		m_nCurLine = 0;
////	}
////
////	CEtUIQuestTreeItem *pItem = new CEtUIQuestTreeItem();
////	pItem->Initialize( this );
////	pItem->SetUICoord(uiCoord);
////	pItem->SetText( szText, dwColor );
////	pItem->AddItemType( dwType );
////
////	m_pRootItem->Add( pItem );
////
////	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
////	UpdateRectsTree();
////
////	return pItem;
////}
////
////CEtUIQuestTreeItem* CEtUIQuestTreeCtl::AddQuestChildItem( CEtUIQuestTreeItem *pTreeItem, DWORD dwType, LPCWSTR szText, DWORD dwColor )
////{
////	ASSERT( pTreeItem&&"CEtUIQuestTreeCtl::AddQuestChildItem, pTreeItem is NULL!" );
////
////	SUICoord uiCoord(pTreeItem->GetCoord());
////
////	SUIElement *pElement(NULL);
////	pElement = GetElement(pTreeItem->GetDepth()+2);
////	if( !pElement ) return NULL;
////
////	uiCoord.fX += m_Property.QuestTreeControlProperty.fIndentSize;
////	uiCoord.fWidth = pElement->fTextureWidth;
////	// Setting only fTextureHeight does not apply LineSpace, so add it directly here.
////	uiCoord.fHeight = pElement->fTextureHeight + m_Property.QuestTreeControlProperty.fLineSpace;
////
////	CEtUIQuestTreeItem *pItem = new CEtUIQuestTreeItem();
////	pItem->Initialize( this, pTreeItem->GetDepth()+1 );
////	pItem->SetUICoord( uiCoord );
////	pItem->SetText( szText, dwColor );
////	pItem->AddItemType( dwType );
////
////	pTreeItem->Add( pItem );
////
////	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
////	UpdateRectsTree();
////
////	return pItem;
////}
//
//// Select the last tree node.
//void CEtUIQuestTreeCtl::SelectLastTree()
//{
//	UpdateRectsTree();
//
//	if ( m_listTreeTravelItem.size() )
//	{
//		TREEITEM_LIST::reverse_iterator r_iter = m_listTreeTravelItem.rbegin();
//
//		CEtUIQuestTreeItem* pItem = static_cast<CEtUIQuestTreeItem*>(*r_iter);
//		if ( pItem )
//			SetSelectItem( pItem );
//
//	}
//
//
//}