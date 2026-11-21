#include "StdAfx.h"
#include "DnQuestTree.h"
#include "DnQuestDlg.h"
#include "DnQuestTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnQuestTree::CDnQuestTree( CEtUIDialog *pParent )
	: CEtUITreeCtl( pParent )
{
	m_Property.UIType = UI_CONTROL_QUESTTREECONTROL;
	m_bIsMainQuest = false;
	m_fMouseX = m_fMouseY = -10000.0f;
	m_nSubImageIndex = 0;
}

CDnQuestTree::~CDnQuestTree(void)
{
	DeleteAllItems();

	for( int i = 0; i < SUB_SUB_IMAGE_COUNT; i++) {
		SAFE_RELEASE_SPTR( m_SubSubTextureList[ i ] );
	}
}

void CDnQuestTree::DeleteAllItems()
{
	BaseClass::DeleteAllItems();
	ASSERT( m_vecCheckBoxList.empty() );
#ifdef PRE_ADD_REMOTE_QUEST
	ASSERT(	m_vecRemoteButtonList.empty() );
#endif // PRE_ADD_REMOTE_QUEST
}

void CDnQuestTree::DeleteCheckBox( CEtUICheckBox *pCheckBox )
{
	int i, nSize;
	nSize = (int)m_vecCheckBoxList.size();
	for( i = 0; i < nSize; i++ )
	{
		if( m_vecCheckBoxList[ i ] == pCheckBox ) 
		{
			m_pParent->DeleteControl( pCheckBox );
			m_vecCheckBoxList.erase( m_vecCheckBoxList.begin() + i );
			break;
		}
	}
	ASSERT( i != nSize );
}

void CDnQuestTree::Initialize( SUIControlProperty *pProperty )
{
	BaseClass::Initialize( pProperty );

	m_ScrollBar.Initialize( NULL );
	m_ScrollBar.SetParentControl( this );

	if( m_Property.QuestTreeControlProperty.nScrollBarTemplate != -1 )
	{
		m_ScrollBar.SetTemplate( m_Property.QuestTreeControlProperty.nScrollBarTemplate );
		m_ScrollBar.UpdateRects();
	}

	m_ScrollBar.SetTrackRange( 0, 0 );

	m_nSubImageIndex = 0;
	for( int i = 0; i < SUB_SUB_IMAGE_COUNT; i++) {
		EtTextureHandle hTexture = LoadResource( CEtResourceMng::GetInstance().GetFullName( FormatA("Sub_SubBar%02d.dds", i+1) ).c_str(), RT_TEXTURE );		
		m_SubSubTextureList[ i ] = hTexture;		
	}
	std::random_shuffle( m_SubSubTextureList, m_SubSubTextureList + _countof(m_SubSubTextureList)  );
}

void CDnQuestTree::RenderTree( TREEITEM_LIST &VecTree )
{
#ifdef PRE_ADD_REMOTE_QUEST
	for( int i=0; i<static_cast<int>( m_vecCheckBoxList.size() ); i++ )
	{
		if( m_vecCheckBoxList[i] )
			m_vecCheckBoxList[i]->Show( false );
	}

	for( int i=0; i<static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton )
			m_vecRemoteButtonList[i].m_pRemoteQuestButton->Show( false );
	}
#else // PRE_ADD_REMOTE_QUEST
	for each (  CEtUICheckBox *pCheckBox  in m_vecCheckBoxList ) {
		pCheckBox->Show( false );
	}
#endif // PRE_ADD_REMOTE_QUEST
	TREEITEM_LIST_ITER iter = VecTree.begin();
	TREEITEM_LIST_ITER iter_end = VecTree.end();

	m_nSubImageIndex = 0;

	for( ; iter != iter_end; ++iter )
	{
		RenderFrame( *iter );
		RenderBackImg( *iter );
		RenderSelFrame( *iter );
		RenderButton( *iter );
		RenderState( *iter );
		RenderText( *iter );
	}
}

void CDnQuestTree::RenderButton( CTreeItem *pItem )
{
	ASSERT(pItem&&"CDnQuestTree::RenderButton");

	SUIElement *pElement(NULL);

	bool bMouseOver = false;
	if( pItem->m_ButtonCoord.IsInside(m_fMouseX, m_fMouseY) ) {
		bMouseOver = true;				
	}

	if( !pItem->IsOpened() )
	{
		// Note : x image
		pElement = GetElement(9);
	}
	else
	{
		if( pItem->HasChild() )
		{
			if( pItem->GetDepth() == 1 || pItem->GetDepth() == 2 )
			{
				if( !pItem->IsExpanded() )
				{
					// Note : + image
					pElement = GetElement( bMouseOver ? 12 : 0 );
				}
				else
				{
					// Note : - image
					pElement = GetElement( bMouseOver ? 13 : 1 );
				}
			}
			//else if( pItem->GetDepth() == 2 )
			//{
			//	// Note: . image (SubButton)
			//	pElement = GetElement(8);
			//}
		}
	}

	if( pElement )
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, 0xffffffff, pItem->m_ButtonCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, 0xffffffff, pItem->m_ButtonCoord );
		//m_pParent->DrawRect( VecTree[i]->m_ButtonCoord, debug::BLUE );
	}
}

void CDnQuestTree::RenderState( CTreeItem *pItem )
{
	if( pItem == NULL )
		return;

	ASSERT(pItem&&"CDnQuestTree::RenderState");
	CDnQuestTreeItem *pQuestItem = static_cast<CDnQuestTreeItem*>(pItem);

	if( !pItem->IsOpened() )
		return;

	SUIElement *pElement(NULL);

	if( pQuestItem->IsCompleted() )
		pElement = GetElement(8);
	else
		pElement = GetElement(9);

	int nQuestIndex = (int)pItem->GetItemValueInt();
	if ( nQuestIndex > 0 ) 
	{
		CEtUICheckBox *pCheckBox = pQuestItem->GetCheckBox();
		if( pCheckBox ) 
		{
			pCheckBox->SetPosition( pQuestItem->m_NotifierCoord.fX, pQuestItem->m_NotifierCoord.fY );
			pCheckBox->SetSize( pQuestItem->m_NotifierCoord.fWidth, pQuestItem->m_NotifierCoord.fHeight );
			pCheckBox->Show( true );
		}
#ifdef PRE_ADD_REMOTE_QUEST
		CEtUIButton* pRemoteButton = pQuestItem->GetRemoteButton();
		if( pRemoteButton )
		{
			SUICoord uiCoord;
			pRemoteButton->GetUICoord( uiCoord );
			pRemoteButton->SetPosition( pQuestItem->m_TextureCoord.fX + pQuestItem->m_TextureCoord.fWidth - uiCoord.fWidth, pQuestItem->m_NotifierCoord.fY );
			pRemoteButton->SetSize( uiCoord.fWidth, uiCoord.fHeight );
			pRemoteButton->Show( true );
		}
#endif // PRE_ADD_REMOTE_QUEST
	}
}

void CDnQuestTree::RenderText( CTreeItem *pItem )
{
	ASSERT(pItem&&"CDnQuestTree::RenderText");

	if( !pItem->IsOpened() )
		return;

	SUIElement *pElement = GetElement(0);
	if( !pElement ) return;

	//m_pParent->DrawRect( pItem->m_strCoord, debug::YELLOW );
	DWORD nOriginalClr = pItem->GetTextColor();

	std::wstring szText = pItem->GetText();
	std::wstring szOriginText = pItem->GetText();

	SUICoord Coord;
	m_pParent->CalcTextRect( szText.c_str(), pElement, Coord );
	while( Coord.fWidth > pItem->m_strCoord.fWidth ) {
		if( !szOriginText.empty() )
			szOriginText.erase( szOriginText.end()-1,  szOriginText.end() );
		szText = szOriginText + L"...";
		m_pParent->CalcTextRect( szText.c_str(), pElement, Coord );
	}

	WCHAR wszString[512];
	if( pItem->GetSubCount() > 0 )
	{
		swprintf_s( wszString, _countof(wszString), L" (%d)", pItem->GetSubCount() );

		szText += std::wstring( wszString );
	}

	m_pParent->DrawDlgText( szText.c_str(), pElement, pItem->GetTextColor(), pItem->m_strCoord, -1, -1, true );
	pItem->SetTextColor(nOriginalClr);
}

void CDnQuestTree::RenderBackImg( CTreeItem *pItem )
{
	ASSERT(pItem&&"CDnQuestTree::RenderBackImg");
	CDnQuestTreeItem *pQuestItem = static_cast<CDnQuestTreeItem*>(pItem);

	if ( pQuestItem->GetTexture() )
	{
		static SUICoord texUV(0.0f,0.0f, 1.0f, 1.0f);
		SUIElement *pElement(NULL);
		pElement = GetElement(pQuestItem->GetDepth()+1);
		m_pParent->DrawSprite( pQuestItem->GetTexture(), texUV, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
	}
}

void CDnQuestTree::RenderFrame( CTreeItem *pItem )
{
	ASSERT( pItem && "CDnQuestTree::RenderFrame" );
	CDnQuestTreeItem *pQuestItem = static_cast<CDnQuestTreeItem*>(pItem);

	SUIElement *pElement(NULL);
	pElement = GetElement( pQuestItem->GetDepth() + 1 );
	if( pElement == NULL )
		return;

	if( pQuestItem->GetDepth() == 3 )
	{
		SUICoord UVCoord(0,0,1,1);
		m_pParent->DrawSprite( m_SubSubTextureList[ m_nSubImageIndex ], UVCoord, 0xffffffff, pQuestItem->m_FrameCoord );
		m_nSubImageIndex++;
		m_nSubImageIndex %= SUB_SUB_IMAGE_COUNT;
	}
	else
	{
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
	}
	//m_pParent->DrawSprite( pQuestItem->GetTexture(), pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameCoord );
	//m_pParent->DrawRect( pQuestItem->m_FrameCoord, debug::YELLOW );
}

void CDnQuestTree::RenderSelFrame( CTreeItem *pItem )
{
	ASSERT(pItem&&"CDnQuestTree::RenderSelFrame");
	CDnQuestTreeItem *pQuestItem = static_cast<CDnQuestTreeItem*>(pItem);

	if( pItem->IsSelected() )
	{
		SUIElement *pElement(NULL);
		pElement = GetElement(pQuestItem->GetDepth()+4);
		if( m_bExistTemplateTexture )
			m_pParent->DrawSprite( m_Template.m_hTemplateTexture, pElement->TemplateUVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameSelectCoord );
		else
			m_pParent->DrawSprite( pElement->UVCoord, pElement->TextureColor.dwCurrentColor, pQuestItem->m_FrameSelectCoord );

		//m_pParent->DrawRect( pQuestItem->m_FrameSelectCoord, debug::YELLOW );
	}
}

CDnQuestTreeItem* CDnQuestTree::AddQuestItem( DWORD dwType, LPCWSTR szText, DWORD dwColor )
{
	SUICoord uiCoord(m_Property.UICoord);

	SUIElement *pElement(NULL);
	pElement = GetElement(2);
	if( !pElement ) return NULL;

	uiCoord.fHeight = pElement->fTextureHeight;
	uiCoord.fWidth = pElement->fTextureWidth;
	//uiCoord.fWidth -= m_Property.QuestTreeControlProperty.fScrollBarSize;

	if( m_pRootItem->HasChild() )
	{
		CTreeItem *pEndItem = m_pRootItem->GetEndItem();
		uiCoord.fY = pEndItem->m_uiCoord.Bottom();
	}
	else
	{
		m_nCurLine = 0;
	}

	CDnQuestTreeItem *pItem = new CDnQuestTreeItem();
	pItem->Initialize( this );
	pItem->SetUICoord(uiCoord);
	pItem->SetText( szText, dwColor );
	pItem->AddItemType( dwType );	

	// for SubQuest
	CDnQuestDlg *pQuestDlg = dynamic_cast<CDnQuestDlg*>(GetParent());
	if( pQuestDlg ) {
		pQuestDlg->RefreshEnableNotifierCheck();
	}

	m_pRootItem->Add( pItem );

	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
	UpdateRectsTree();	

	return pItem;
}

CDnQuestTreeItem* CDnQuestTree::AddQuestChildItem( CEtUIQuestTreeItem *pTreeItem, DWORD dwType, LPCWSTR szText, DWORD dwColor )
{
	ASSERT( pTreeItem&&"CDnQuestTree::AddQuestChildItem, pTreeItem is NULL!" );

	SUICoord uiCoord(pTreeItem->GetCoord());

	SUIElement *pElement(NULL);
	pElement = GetElement( pTreeItem->GetDepth() + 2 );
	if( !pElement ) return NULL;

	uiCoord.fX += m_Property.QuestTreeControlProperty.fIndentSize;
	uiCoord.fWidth = pElement->fTextureWidth;
	uiCoord.fHeight = pElement->fTextureHeight;

	CDnQuestTreeItem *pItem = new CDnQuestTreeItem();
	pItem->Initialize( this, pTreeItem->GetDepth()+1 );
	pItem->SetUICoord( uiCoord );
	pItem->SetText( szText, dwColor );
	pItem->AddItemType( dwType );

	pTreeItem->Add( pItem );

	m_ScrollBar.SetTrackRange( 0, GetExpandedItemCount() );
	UpdateRectsTree();

	return pItem;
}

// Select the last tree node.
void CDnQuestTree::SelectLastTree()
{
	UpdateRectsTree();

	if ( m_listTreeTravelItem.size() ) {
		TREEITEM_LIST::reverse_iterator r_iter = m_listTreeTravelItem.rbegin();

		CDnQuestTreeItem* pItem = static_cast<CDnQuestTreeItem*>(*r_iter);
		if ( pItem )
			SetSelectItem( pItem );
	}
}

CEtUICheckBox* CDnQuestTree::CreateCheckBox( int nQuestIndex, CDnQuestTreeItem *pItem )
{
	bool bAttachCheckBox = false;
	if( m_bIsMainQuest ) {	// For main quests, remove previously created checkboxes so that only the lastly added main quest is displayed.
		TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
		TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
		for( ; iter != iter_end; ++iter )
		{
			((CDnQuestTreeItem*)(*iter))->DeleteCheckBox();
		}
		ASSERT( m_vecCheckBoxList.empty() );
	}

	CEtUICheckBox *pCheckBox = (CEtUICheckBox *)GetParent()->CreateControl( &m_CheckBoxProperty );
	if( pCheckBox )
	{
		pCheckBox->SetBlendRate( 0.f );
		pCheckBox->SetButtonID( nQuestIndex );	
		m_vecCheckBoxList.push_back( pCheckBox );
	}

	return pCheckBox;
}

#ifdef PRE_ADD_REMOTE_QUEST
void CDnQuestTree::SetRemoteQuestState( int nQuestIndex, eRemoteQuestState remoteQuestState )
{
	for( int i=0; i <static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton )
		{
			if( m_vecRemoteButtonList[i].m_pRemoteQuestButton->GetButtonID() == nQuestIndex )
			{
				m_vecRemoteButtonList[i].m_eRemoteQuestState = remoteQuestState;
				switch( remoteQuestState )
				{
				case REMOTEQUEST_ASK:
					{
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1413 ) );
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->Show( true );
					}
					break;
				case REMOTEQUEST_COMPLETE:
					{
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7117 ) );
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->Show( true );
					}
					break;
				case REMOTEQUEST_PROGRESSING:
					{
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->ClearText();
						m_vecRemoteButtonList[i].m_pRemoteQuestButton->Show( false );
					}
					break;
				}
				break;
			}
		}
	}
}

CDnQuestTree::eRemoteQuestState CDnQuestTree::GetRemoteQuestState( int nQuestIndex )
{
	eRemoteQuestState remoteQuestState = REMOTEQUEST_NONE;

	for( int i=0; i <static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton )
		{
			if( m_vecRemoteButtonList[i].m_pRemoteQuestButton->GetButtonID() == nQuestIndex )
			{
				remoteQuestState = m_vecRemoteButtonList[i].m_eRemoteQuestState;
				break;
			}
		}
	}

	return remoteQuestState;
}

bool CDnQuestTree::IsRemoteQuest( int nQuestIndex )
{
	bool bRemoteQuest = false;

	for( int i=0; i <static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton )
		{
			if( m_vecRemoteButtonList[i].m_pRemoteQuestButton->GetButtonID() == nQuestIndex )
			{
				bRemoteQuest = true;
				break;
			}
		}
	}

	return bRemoteQuest;
}

CEtUIButton* CDnQuestTree::CreateRemoteButton( int nQuestIndex )
{
	CEtUIButton* pRemoteButton = NULL;
	bool bExist = false;
	for( int i=0; i<static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton && m_vecRemoteButtonList[i].m_pRemoteQuestButton->GetButtonID() == nQuestIndex )
		{
			bExist = true;
			pRemoteButton = m_vecRemoteButtonList[i].m_pRemoteQuestButton;
			break;
		}
	}

	if( !bExist )
	{
		pRemoteButton = (CEtUIButton*)GetParent()->CreateControl( &m_RemoteButtonProperty );
		if( pRemoteButton )
		{
			pRemoteButton->SetBlendRate( 0.f );
			pRemoteButton->SetButtonID( nQuestIndex );

			stRemoteQuestStateButton stStateButton;
			stStateButton.m_eRemoteQuestState = REMOTEQUEST_NONE;
			stStateButton.m_pRemoteQuestButton = pRemoteButton;
			m_vecRemoteButtonList.push_back( stStateButton );
		}
	}

	return pRemoteButton;
}

void CDnQuestTree::DeleteRemoteButton( CEtUIButton* pRemoteButton )
{
	for( int i=0; i <static_cast<int>( m_vecRemoteButtonList.size() ); i++ )
	{
		if( m_vecRemoteButtonList[i].m_pRemoteQuestButton == pRemoteButton ) 
		{
			m_pParent->DeleteControl( pRemoteButton );
			m_vecRemoteButtonList.erase( m_vecRemoteButtonList.begin() + i );
			break;
		}
	}
}

void CDnQuestTree::SetSelectRemoteQuest( int nQuestIndex )
{
	TREEITEM_LIST_ITER iter = m_listTreeTravelItem.begin();
	TREEITEM_LIST_ITER iter_end = m_listTreeTravelItem.end();
	for( ; iter != iter_end; ++iter )
	{
		CTreeItem* pItem = *iter;
		CDnQuestTreeItem* pDnQuestTreeItem = static_cast<CDnQuestTreeItem*>(pItem);
		if( pDnQuestTreeItem && pDnQuestTreeItem->GetRemoteButton() )
		{
			if( pDnQuestTreeItem->GetRemoteButton()->GetButtonID() == nQuestIndex )
			{
				SetSelectItem( pDnQuestTreeItem );
				break;
			}
		}
	}
}
#endif // PRE_ADD_REMOTE_QUEST

bool CDnQuestTree::HandleMouse( UINT uMsg, float fX, float fY, WPARAM wParam, LPARAM lParam )
{
	bool bResult = CEtUITreeCtl::HandleMouse( uMsg, fX, fY, wParam, lParam);
	if( uMsg == WM_MOUSEMOVE ) {
		m_fMouseX = fX;
		m_fMouseY = fY;
	}
	return bResult;
}

void CDnQuestTree::FindInputPos( std::vector<EtVector2> &vecPos )
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
		CDnQuestTreeItem *pQuestItem = static_cast<CDnQuestTreeItem*>(pItem);

		fPosX = pDialog->GetXCoord() + pQuestItem->m_FrameCoord.fX + pQuestItem->m_FrameCoord.fWidth / 2.0f;
		fPosY = pDialog->GetYCoord() + pQuestItem->m_FrameCoord.fY + pQuestItem->m_FrameCoord.fHeight / 2.0f;

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

		// Quest tree checkboxes are handled separately in the parent dialog processing, so no need to do it here.
	}

	if( m_listTreeTravelItem.size() )
	{
		m_ScrollBar.FindInputPos( vecPos );
	}
}

void CDnQuestTree::SetExceptLastSort( int nGoalID )
{
	m_pRootItem->SetExceptLastSort( nGoalID );
}