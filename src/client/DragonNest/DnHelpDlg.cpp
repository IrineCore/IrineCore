#include "StdAfx.h"
#include "DnHelpDlg.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#include "DnHelpListItemDlg.h"
#include "InputWrapper.h"
#include "DnInterface.h"
#include "DnMainDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnHelpDlg::CDnHelpDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
{
	m_pTreeCategory = NULL;
	m_pHtmlTextHelp = NULL;
	m_pListboxKeyword = NULL;
	m_nLastSelectTableID = -1;

	m_pStaticKeyword = NULL;
	m_pStaticTitle = NULL;
}

CDnHelpDlg::~CDnHelpDlg(void)
{
	SAFE_DELETE_VEC( m_pMapCategorySearch );
	SAFE_DELETE_PVEC( m_pVecCategoryList );
	SAFE_DELETE_PVEC( m_pVecKeywordList );
}

void CDnHelpDlg::InitialUpdate()
{
	m_pTreeCategory = GetControl<CEtUITreeCtl>( "ID_TREE_LIST" );
	m_pHtmlTextHelp = GetControl<CEtUIHtmlTextBox>( "ID_HTMLTEXTBOX0" );
	m_pListboxKeyword = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_KEYWORD" );
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_KEYWORD0" );
	m_pStaticKeyword = GetControl<CEtUIStatic>( "ID_TEXT_KEYWORD1" );

	LoadHelpTable();
	LoadKeywordTable();
}

void CDnHelpDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "HelpDlg.ui" ).c_str(), bShow );
}

void CDnHelpDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_TREECONTROL_SELECTION ) {
		if( IsCmdControl("ID_TREE_LIST" ) )
		{
			CTreeItem *pItem = m_pTreeCategory->GetSelectedItem();
			RefreshHelpHtml( pItem->GetItemValueInt() );
			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_SELECTION ) {
		if( IsCmdControl("ID_LISTBOXEX_KEYWORD" ) )
		{
			CDnHelpListItemDlg *pItem = m_pListboxKeyword->GetItem<CDnHelpListItemDlg>( m_pListboxKeyword->GetSelectedIndex() );
			if( pItem ) {
				CTreeItem *pTreeItem = m_pTreeCategory->FindItemInt( pItem->GetHelpTableID() );
				if( pTreeItem ) {
//					pTreeItem->Expand();
					m_pTreeCategory->SetSelectItem( pTreeItem );
					RefreshHelpHtml( pItem->GetHelpTableID() );
					ExpandRoot( pTreeItem );
//					m_pTreeCategory->ExpandAllChildren( pTreeItem->GetParentControl( );
				}
			}
			return;
		}
	}
	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BT_CLOSE" ) ) {
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnHelpDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			Show(false);
			return true;
		}

		if( GetKeyState(_ToVK(g_UIWrappingKeyData[IW_UI_USERHELP]))&0x80 )	// Must check like this so it works properly even when switched to another language.
		{
			Show(false);
			return true;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnHelpDlg::ExpandRoot( CTreeItem *pItem )
{
	if( pItem->GetParentItem() ) {
		m_pTreeCategory->ExpandAllChildren( pItem->GetParentItem() );
		ExpandRoot( pItem->GetParentItem() );
	}
}

void CDnHelpDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;
	if( bShow ) {
		RefreshCategory();
		RefreshKeyword();
	}
	else {
		m_pTreeCategory->DeleteAllItems();
		m_pListboxKeyword->RemoveAllItems();

		if( GetInterface().GetMainBarDialog() )
			GetInterface().GetMainBarDialog()->SetHotKeyState( 0 );
	}

	CEtUIDialog::Show( bShow );
}

void CDnHelpDlg::LoadHelpTable()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::THELP );

	int nCount = pSox->GetItemCount();
	char szLabel[32];
	for( int i=0; i<nCount; i++ ) {
		int nItemID = pSox->GetItemID(i);
		HelpStruct *pStruct = new HelpStruct;

		pStruct->nTableID = nItemID;
		pStruct->szCategory = pSox->GetFieldFromLablePtr( nItemID, "_Category" )->GetString();
		pStruct->szXmlFileName = pSox->GetFieldFromLablePtr( nItemID, "_HtmlName" )->GetString();
		for( int j=0; j<5; j++ ) {
			sprintf_s( szLabel, "_keyWordID%d", j + 1 );
			int nValue = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetInteger();
			if( nValue > 0 ) pStruct->nVecKeyword.push_back( nValue );
		}

		m_pVecCategoryList.push_back( pStruct );
	}

	/*
	HelpStruct *pStruct = new HelpStruct;
	pStruct->nTableID = 1;
	pStruct->szCategory = "9705/9709";
	pStruct->szXmlFileName = "Help_Test.html";
	pStruct->nVecKeyword.push_back( 9705 );
	pStruct->nVecKeyword.push_back( 9706 );
	m_pVecCategoryList.push_back( pStruct );

	pStruct = new HelpStruct;
	pStruct->nTableID = 2;
	pStruct->szCategory = "9705/9712";
	pStruct->szXmlFileName = "Help_Test.html";
	m_pVecCategoryList.push_back( pStruct );

	pStruct = new HelpStruct;
	pStruct->nTableID = 3;
	pStruct->szCategory = "9705/9713";
	pStruct->szXmlFileName = "Help_Test.html";
	m_pVecCategoryList.push_back( pStruct );

	pStruct = new HelpStruct;
	pStruct->nTableID = 4;
	pStruct->szCategory = "9706/9718";
	pStruct->szXmlFileName = "Help_Test.html";
	m_pVecCategoryList.push_back( pStruct );
	*/

	for( DWORD i=0; i<m_pVecCategoryList.size(); i++ ) {
		m_pMapCategorySearch.insert( make_pair( m_pVecCategoryList[i]->nTableID, m_pVecCategoryList[i] ) );
	}
}

void CDnHelpDlg::LoadKeywordTable()
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::THELPKEYWORD );

	int nCount = pSox->GetItemCount();

	for( int i=0; i<nCount; i++ ) {
		int nItemID = pSox->GetItemID(i);
		KeywordStruct *pStruct = new KeywordStruct;

		pStruct->nKeywordStringID = pSox->GetFieldFromLablePtr( nItemID, "_VitalkeywordID" )->GetInteger();
		pStruct->nLinkHelpTableID = pSox->GetFieldFromLablePtr( nItemID, "_HelpID" )->GetInteger();
		pStruct->nOrder = pSox->GetFieldFromLablePtr( nItemID, "_Order" )->GetInteger();

		m_pVecKeywordList.push_back( pStruct );
	}
}

void CDnHelpDlg::RefreshCategory()
{
	m_pTreeCategory->DeleteAllItems();

	struct TreeItemStruct {
		CTreeItem *pItem;
		std::map<int, TreeItemStruct *> nMapTreeList;
	};
	std::map<int, TreeItemStruct *> nMapTreeList;
	std::vector<TreeItemStruct *> pVecTemp;

	CTreeItem *pSelectItem = NULL;
	for( DWORD i=0; i<m_pVecCategoryList.size(); i++ ) {
		HelpStruct *pInfo = m_pVecCategoryList[i];

		std::map<int, TreeItemStruct *> *pTreeList = &nMapTreeList;
		CTreeItem *pParentItem = NULL;
		std::string szCodeStr;
		for( int j=0;; j++ ) {
			std::string strValue = _GetSubStrByCountSafe( j, (char*)pInfo->szCategory.c_str(), '/' );
			if( strValue.size() == 0 ) break;
			int nCategoryID = atoi( strValue.c_str() );

			std::map<int, TreeItemStruct *>::iterator it = pTreeList->find( nCategoryID );
			if( it == pTreeList->end() ) {
				TreeItemStruct *pStruct = new TreeItemStruct;
				pVecTemp.push_back( pStruct );
				if( j == 0 ) pStruct->pItem = m_pTreeCategory->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nCategoryID ) );
				else pStruct->pItem = m_pTreeCategory->AddChildItem( pParentItem, CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nCategoryID ) );

				pParentItem = pStruct->pItem;
				if( j == 0 ) pStruct->pItem->SetItemValueInt( 0 );
				else pStruct->pItem->SetItemValueInt( pInfo->nTableID );
				pTreeList->insert( make_pair( nCategoryID, pStruct ) );
				pTreeList = &pStruct->nMapTreeList;

				if( pSelectItem == NULL ) {
					if( j > 0 ) {
						if( m_nLastSelectTableID == -1 ) {
							m_nLastSelectTableID = pInfo->nTableID;
							pSelectItem = pStruct->pItem;
						}
						else {
							if( pInfo->nTableID == m_nLastSelectTableID ) {
								pSelectItem = pStruct->pItem;
							}
						}
					}
				}
			}
			else {
				pParentItem = it->second->pItem;
				pTreeList = &it->second->nMapTreeList;
			}
		}

	}
	if( pSelectItem ) {
		m_pTreeCategory->ExpandAllChildren( pSelectItem->GetParentItem() );
		m_pTreeCategory->SetSelectItem( pSelectItem );
		RefreshHelpHtml( m_nLastSelectTableID );

	}
	SAFE_DELETE_PVEC( pVecTemp );
}

void CDnHelpDlg::RefreshHelpHtml( int nTableID )
{
	HelpStruct *pStruct = FindHelpStructFromTableID( nTableID );
	if( !pStruct ) {
		m_pHtmlTextHelp->ClearText();
		m_pStaticKeyword->SetText( L"" );
		m_pStaticTitle->SetText( L"" );
		return;
	}

	CResMngStream Stream( pStruct->szXmlFileName.c_str() );
	if( Stream.IsValid() ) {
		m_pHtmlTextHelp->ReadHtmlStream( &Stream );
	}

	std::vector<std::string> szVecTokens;
	TokenizeA( pStruct->szCategory, szVecTokens, std::string("/") );

	if( !szVecTokens.empty() ) 
		m_pStaticTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, atoi( szVecTokens[ szVecTokens.size()-1 ].c_str() ) ) );
	else m_pStaticTitle->SetText( L"" );

	std::wstring wszKeyword;
	for( DWORD i=0; i<pStruct->nVecKeyword.size(); i++ ) {
		wszKeyword += GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pStruct->nVecKeyword[i] );
		if( i != pStruct->nVecKeyword.size() - 1 ) wszKeyword += L", ";
	}
	m_pStaticKeyword->SetText( wszKeyword );

	m_nLastSelectTableID = nTableID;
}

void CDnHelpDlg::RefreshKeyword()
{
	m_pListboxKeyword->RemoveAllItems();

	for( DWORD i=0; i<m_pVecKeywordList.size(); i++ ) {
		KeywordStruct *pStruct = m_pVecKeywordList[i];
		CDnHelpListItemDlg *pItem = m_pListboxKeyword->AddItem<CDnHelpListItemDlg>();
		pItem->SetInfo( pStruct );
	}
}

CDnHelpDlg::HelpStruct *CDnHelpDlg::FindHelpStructFromTableID( int nTableID )
{
	std::map<int, HelpStruct*>::iterator it = m_pMapCategorySearch.find( nTableID );
	if( it == m_pMapCategorySearch.end() ) return NULL;
	return it->second;
}

void CDnHelpDlg::ShowCategory( int nIndex )
{
	m_nLastSelectTableID = nIndex;
	//RefreshHelpHtml( nIndex );
	//Only set m_nLastSelectTableID here; Show(true) will call RefreshCategory().
}
