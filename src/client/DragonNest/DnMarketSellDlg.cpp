#include "StdAfx.h"
#include "DnMarketSellDlg.h"
#include "DnTradeTask.h"
#include "DnMarketPriceDlg.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnMarketPriceDlg.h"
#include "DnMarketListDlg.h"
#include "DnUIString.h"
#include "DnTableDB.h"
#include "DnMainMenuDlg.h"
#include "DnMarketTabDlg.h"
#include "DnMarketListDlg.h"
#include "DnMainFrame.h"
#include "DnItemTask.h"
#include "SyncTimer.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
CDnMarketSellDlg::CDnMarketSellDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pMarketPriceDlg(NULL)
	, m_wSellingCount( 0 )
	, m_wClosingCount( 0 )
	, m_wWeeklyRegisterCount(0)
	, m_wRemainRegisterItemCount(0)
	, m_bRequestListNeeded( true )
	, m_lastSortType( ST_NONE )
	, m_lastAscending( true )
	, m_nPage( 1 )
	, m_nRecallDBID( -1 )
	, m_bPremium( false )
	, m_bRefresh( false )
	, m_bAllRecall( false )
	, m_bEnableRecallRequest( false )
	, m_nRecallCount( 0 )
	, m_pButtonAllRecall( NULL )
{
}

CDnMarketSellDlg::~CDnMarketSellDlg(void)
{
	SAFE_DELETE( m_pMarketPriceDlg );
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();
}

void CDnMarketSellDlg::Initialize( bool bShow )
{
#if defined(PRE_ADD_PETALTRADE)
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "MarketSellDlg_wPetal.ui" ).c_str(), bShow );
#else
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "MarketSellDlg.ui" ).c_str(), bShow );
#endif	//#if defined(PRE_ADD_PETALTRADE)
}

void CDnMarketSellDlg::InitCustomControl( CEtUIControl *pControl )
{

}

void CDnMarketSellDlg::InitialUpdate()
{
	m_pMarketPriceDlg = new CDnMarketPriceDlg( UI_TYPE_CHILD, this, -1, this );
	m_pMarketPriceDlg->Initialize( false );

	CONTROL( Static, ID_TEXT_SELL )->SetText(L"");				// %d on sale
	CONTROL( Static, ID_TEXT_LIMITTIME )->SetText(L"");		// %d expired
	CONTROL( Static, ID_TEXT_MAXCOUNT )->SetText(L"");		// Max registrable count: %d
	CONTROL( Static, ID_TEXT_ITEMCOUNT )->SetText(L"");	// Registrable item count : %d

	m_pButtonAllRecall = GetControl<CEtUIButton>("ID_BUTTON_ALLRECALL");

	RefreshCount();
	RefreshPageControls();
}

void CDnMarketSellDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		bool bSortNeeded = false;
		bool bChangePageNeeded = false;
		if( IsCmdControl( "ID_BUTTON_SETTING" ) )	// Register
		{
			int maxRegisterEnableSlotCount = 0;

			if( m_bPremium )
			{
				for( int itr = 0; itr < (int)m_vPremiumEnableNumList.size(); ++itr )
					maxRegisterEnableSlotCount += m_vPremiumEnableNumList[itr];
				maxRegisterEnableSlotCount += (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketItemRegisterCount_Basic);
			}
			else
			{
				maxRegisterEnableSlotCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketItemRegisterCount_Basic);
			}

			if( maxRegisterEnableSlotCount - m_wRemainRegisterItemCount == 0 )
			{
				GetInterface().MessageBox( 3621, MB_OK );
				return;
			}

			m_pMarketPriceDlg->SetPremium( m_bPremium );
			ShowChildDialog( m_pMarketPriceDlg, true );

			CDnMarketTabDlg* pMarketTabDlg = (CDnMarketTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::MARKET_DIALOG);
			if( pMarketTabDlg ) {
				pMarketTabDlg->EnableAllControl( false );
				pMarketTabDlg->EnableChildDlg( false );
			}

			CDnSlotButton *pDragButton = ( CDnSlotButton * )drag::GetControl();
			if( pDragButton )
			{
				pDragButton->DisableSplitMode(false);
				drag::ReleaseControl();
				pDragButton->DisableSplitMode(true);
			}

			return;
		}
		else if( IsCmdControl( "ID_BUTTON_NEW" ) )		// Refresh
		{
			m_bRefresh = true;
			m_IsNewPage.clear();
			GetTradeTask().GetTradeMarket().RequestMarketSellList();
		}		
		else if( IsCmdControl( "ID_BUTTON_RECALL" ) )		// Retrieve
		{
			ProcessRecall();
			return;
		}
		else if( IsCmdControl( "ID_BUTTON_BACK") ) {
			if( m_nPage > 1 ) {
				m_nPage--;
				bChangePageNeeded = true;				
			}
			else {
				ASSERT( false );
			}
		}
		else if( IsCmdControl( "ID_BUTTON_NEXT") ) {
			if( m_nPage < GetTotalPage() ) {
				m_nPage++;
				bChangePageNeeded = true;
			}
			else {
				ASSERT( false );
			}
		}		
		else if( IsCmdControl( "ID_BUTTON_FIRST") ) {
			if( m_nPage > PAGE_COUNT_FOR_SCREEN ) {
				m_nPage = GetStartPage() - PAGE_COUNT_FOR_SCREEN;
				bChangePageNeeded = true;
			}
			else {
				ASSERT( false );
			}
		}
		else if( IsCmdControl( "ID_BUTTON_LAST") ) {
			if( (m_nPage + PAGE_COUNT_FOR_SCREEN - 1) / PAGE_COUNT_FOR_SCREEN  
				< (GetTotalPage() + PAGE_COUNT_FOR_SCREEN - 1 )  / PAGE_COUNT_FOR_SCREEN  ) {
					m_nPage = GetStartPage() + PAGE_COUNT_FOR_SCREEN;
					bChangePageNeeded = true;
			}
			else {
				ASSERT( false );
			}
		}
		else if( strstr( pControl->GetControlName(), "ID_STATIC_PAGE" ) != NULL ) {
			int nIndex = -1;
			if( sscanf_s( pControl->GetControlName() + strlen("ID_STATIC_PAGE"), "%d", &nIndex) == 1 ) {
				m_nPage = GetStartPage() + nIndex - 1;
				bChangePageNeeded = true;
			}
		}
		else if( IsCmdControl( "ID_SORT_NAME" ) )		// Sort by name
		{
			m_lastAscending = (m_lastSortType == ST_NAME) ? !m_lastAscending : true;
			m_lastSortType = ST_NAME;
			bSortNeeded = true;
		}
		else if( IsCmdControl( "ID_SORT_CLOSETIME" ) )	// Sort by expiration time
		{
			m_lastAscending = (m_lastSortType == ST_REMAINTIME) ? !m_lastAscending : true;
			m_lastSortType = ST_REMAINTIME;
			bSortNeeded = true;
		}
		else if( IsCmdControl( "ID_SORT_PRICE" ) )		// Sort by price
		{
			m_lastAscending = (m_lastSortType == ST_PRICE) ? !m_lastAscending : true;
			m_lastSortType = ST_PRICE;
			bSortNeeded = true;
		}
		else if( IsCmdControl( "ID_BUTTON_PosPETAL" ) )
		{
			CDnMarketTabDlg * pMarketTab = (CDnMarketTabDlg *)m_pParentDialog;
			pMarketTab->ShowPosPetal( true );
		}
		else if( IsCmdControl( "ID_BUTTON_ALLRECALL" ) )
		{
			m_bAllRecall = true;
			m_nRecallCount = 0;
			ProcessAllRecall();
			return;
		}

		if( bChangePageNeeded ) {
			UpdateListItems();
			RefreshPageControls();
		}
		if( bSortNeeded ) {
			std::sort( m_vecSellList.begin(), m_vecSellList.end(), SellListSorter( m_lastSortType, m_lastAscending ) );	
			UpdateListItems();
		}
	}
	else if ( nCommand == EVENT_LISTBOX_ITEM_DBLCLK ) {
		if( IsCmdControl("ID_LISTBOXEX")) {
			ProcessRecall();
		}
	}

	BaseClass::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMarketSellDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_bRefresh = true;
		GetTradeTask().GetTradeMarket().RequestMarketSellList();

		if( m_bRequestListNeeded ) 
		{			// Request only once initially.... (Think it won't be needed afterward.)
			m_IsNewPage.clear();
			m_bRequestListNeeded = false;
		}
	}
	else
	{
		m_pMarketPriceDlg->Show( false );
		((CDnMarketTabDlg *)m_pParentDialog)->ShowPremium( false );
	}

	BaseClass::Show( bShow );
}

bool CDnMarketSellDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() ) {
		return false;
	}

	if( m_pMarketPriceDlg && !m_pMarketPriceDlg->IsShow() )
	{
		switch( uMsg )
		{
		case WM_MOUSEWHEEL:
			{
				POINT MousePoint;
				MousePoint.x = short( LOWORD( lParam ) );
				MousePoint.y = short( HIWORD( lParam ) );

				ScreenToClient( hWnd, &MousePoint );
				lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

				if( IsMouseInDlg() )
				{
					UINT uLines;
					SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
					int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
					if( nScrollAmount > 0 )
					{
						if( m_nPage > 1 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BUTTON_BACK"), 0 );
					}
					else if( nScrollAmount < 0 )
					{
						if( m_nPage < GetTotalPage() )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BUTTON_NEXT"), 0 );
					}

					CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );
					return true;
				}
			}
			break;
		}
	}

	return BaseClass::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnMarketSellDlg::Process( float fElapsedTime ) 
{
	int nSize = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize();
	for( int i = 0; i < nSize; i++) {
		CDnMarketListDlg *pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetItem<CDnMarketListDlg>( i );
		pItemDlg->ProcessAlpha( fElapsedTime );
	}

	if( m_bAllRecall )
	{
		m_pButtonAllRecall->Enable( false );

		if( m_bEnableRecallRequest )
			ProcessAllRecall();
	}
	else
		m_pButtonAllRecall->Enable( true );

	BaseClass::Process( fElapsedTime );
}

void CDnMarketSellDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) {
		if( IsCmdControl( "ID_BUTTON_CLOSE" ) ) {
			ShowChildDialog( m_pMarketPriceDlg, false );
			CDnMarketTabDlg* pMarketTabDlg = (CDnMarketTabDlg*)GetInterface().GetMainMenuDialog(CDnMainMenuDlg::MARKET_DIALOG);
			if( pMarketTabDlg ) {
				pMarketTabDlg->EnableAllControl( true );
				pMarketTabDlg->EnableChildDlg( true );
			}
		}
		else {
			switch( nID ) 
			{
			case MESSAGEBOX_RECALL_ITEM:
				if( IsCmdControl( "ID_YES" ) ) {
					if( m_nRecallDBID != -1 ) {
						GetTradeTask().GetTradeMarket().RequestMarketInterrupt( m_nRecallDBID );
						m_nRecallDBID = -1;
					}
				}
				break;
			}
		}		
	}
}

void CDnMarketSellDlg::OnRecvUpdateList( std::vector< TMyMarketInfo > &vecSellList )
{
	m_vecSellList.clear();
	m_nPage = 1;
	for each( TMyMarketInfo myMarket in vecSellList ) {
		MySellItem myItem;
		myItem.wszItemName = CDnItem::GetItemFullName( myMarket.nItemID );
		myItem.MyMarket = myMarket;
		m_vecSellList.push_back( myItem );
	}

	UpdateListItems();
	RefreshCount();
	RefreshPageControls();

	if( !m_bRefresh )
	{
		int  iValue = (int)m_vecSellList.size() / MAX_ITEM_COUNT_FOR_PAGE;
		float fRemain = (float)( (int)m_vecSellList.size() % MAX_ITEM_COUNT_FOR_PAGE );
		m_nPage = fRemain == 0.0f ? iValue : iValue+1;
		UpdateListItems();
		RefreshPageControls();
	}
	else
		m_bRefresh = false;
}

void CDnMarketSellDlg::OnRecvRemoveItem( int nMarketDBID, int nRegisterItemCount )
{
	if( nMarketDBID == -1 ) {
		// Retrieval failed...
		m_bAllRecall = false;
		return;
	}
	int nSize = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize();
	for( int i = 0; i < nSize; i++) {
		CDnMarketListDlg *pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetItem<CDnMarketListDlg>( i );
		if( pItemDlg->GetMarketDBID() == nMarketDBID ) {
			CEtSoundEngine::GetInstance().PlaySound( "2D", pItemDlg->GetItemSound() );
			CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveItem( i );						
			break;
		}
	}

	std::vector< MySellItem >::iterator it = m_vecSellList.begin();
	while( it != m_vecSellList.end() ) {
		if( it->MyMarket.nMarketDBID == nMarketDBID ) {
			if( it->MyMarket.cSellType == 0 ) {
				if( it->MyMarket.nRemainTime > 0 )
					m_wSellingCount--;
				else
					m_wClosingCount--;
			}
			it = m_vecSellList.erase( it );
		}
		else {
			++it;
		}
	}	

	m_wRemainRegisterItemCount = nRegisterItemCount;
	RefreshCount();
	UpdateListItems();
	RefreshPageControls();

	m_bEnableRecallRequest = true;
}

void CDnMarketSellDlg::SearchPremium()
{
	m_vPremiumEnableNumList.clear();

	//Find premium item in cash inventory.
	std::vector< CDnItem *> pList;
	CDnItemTask::GetInstance().GetCashInventory().FindItemFromItemType( ITEMTYPE_PREMIUM_TRADE, pList );

	const time_t pNowTime = CSyncTimer::GetInstance().GetCurTime();
	for( int itr = 0; itr < (int)pList.size(); ++itr )
	{
		if( !pList[itr]->IsEternityItem() && pNowTime < *pList[itr]->GetExpireDate() )
		{
			m_vPremiumEnableNumList.push_back( pList[itr]->GetTypeParam( 1 ) );
			break;
		}
	}
}

void CDnMarketSellDlg::SetCount( short wSellingCount, short wClosingCount, short wWeeklyRegisterCount, short wRemainRegisterItemCount, bool bPremium )
{
	m_wSellingCount = wSellingCount;
	m_wClosingCount = wClosingCount;
	m_wWeeklyRegisterCount = wWeeklyRegisterCount;
	m_wRemainRegisterItemCount = wRemainRegisterItemCount;
	m_bPremium = bPremium;
}

void CDnMarketSellDlg::RefreshCount()
{	
	SearchPremium();

	CONTROL( Static, ID_TEXT_SELL )->SetText( FormatW( CONTROL( Static, ID_TEXT_SELL )->GetPropertyString(), m_wSellingCount ) );			// %d on sale
	CONTROL( Static, ID_TEXT_LIMITTIME )->SetText( FormatW( CONTROL( Static, ID_TEXT_LIMITTIME )->GetPropertyString(), m_wClosingCount ) );			// %d expired
	
	if( m_bPremium )
	{
		CONTROL( Static, ID_TEXT_ITEMCOUNT )->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4076) );	//"Weekly item registration limit : Unlimited"

		int maxRegisterEnableSlotCount = 0;
										
		for( int itr = 0; itr < (int)m_vPremiumEnableNumList.size(); ++itr )
			maxRegisterEnableSlotCount += m_vPremiumEnableNumList[itr];
		maxRegisterEnableSlotCount += (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketItemRegisterCount_Basic);

		int currentRegisterEnableSlotCnt = maxRegisterEnableSlotCount - m_wRemainRegisterItemCount;
		if( currentRegisterEnableSlotCnt < 0 )
			currentRegisterEnableSlotCnt = 0;
		CONTROL( Static, ID_TEXT_MAXCOUNT )->SetText( FormatW( CONTROL( Static, ID_TEXT_MAXCOUNT )->GetPropertyString(), currentRegisterEnableSlotCnt) );			// Max registrable count: %d
		CONTROL( Button, ID_BUTTON_SETTING )->Enable( maxRegisterEnableSlotCount >= currentRegisterEnableSlotCnt ? true : false );

		if( IsShow() )
			((CDnMarketTabDlg *)m_pParentDialog)->ShowPremium( true );
	}
	else
	{
		int weeklyRegisterCount;
		int nRemainCount = m_wWeeklyRegisterCount;
		int maxWeeklyRegisterCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketRegisterCount_Basic);
		weeklyRegisterCount = maxWeeklyRegisterCount - nRemainCount;
		CONTROL( Static, ID_TEXT_ITEMCOUNT )->SetText( FormatW( CONTROL( Static, ID_TEXT_ITEMCOUNT )->GetPropertyString(), weeklyRegisterCount ) );

		int maxRegisterEnableSlotCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketItemRegisterCount_Basic);
		int currentRegisterEnableSlotCnt;
		currentRegisterEnableSlotCnt = maxRegisterEnableSlotCount - m_wRemainRegisterItemCount;
		if( weeklyRegisterCount < maxRegisterEnableSlotCount && weeklyRegisterCount < currentRegisterEnableSlotCnt )
			currentRegisterEnableSlotCnt = weeklyRegisterCount;
		if (currentRegisterEnableSlotCnt <= 0)
			currentRegisterEnableSlotCnt = 0;
		CONTROL( Static, ID_TEXT_MAXCOUNT )->SetText( FormatW( CONTROL( Static, ID_TEXT_MAXCOUNT )->GetPropertyString(), currentRegisterEnableSlotCnt) );			// Max registrable count: %d

		CONTROL( Button, ID_BUTTON_SETTING )->Enable( (nRemainCount >= 0 && nRemainCount < (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::MarketRegisterCount_Basic) ) ? true : false );
		
		((CDnMarketTabDlg *)m_pParentDialog)->ShowPremium( false );
	}
}

void CDnMarketSellDlg::RefreshPageControls()
{
	int nStartPage = GetStartPage();

	for( int i = 0; i < PAGE_COUNT_FOR_SCREEN; i++ ) {
		CEtUIStatic *pStaticPage = _GC<CEtUIStatic>( FormatA("ID_STATIC_PAGE%d", i + 1 ).c_str() );
		int nPage = i + nStartPage;
		if( nPage >= 100 ) {
			pStaticPage->SetText( FormatW(L"%d", nPage).c_str() );
		}
		else if( nPage >= 10 ) {
			pStaticPage->SetText( FormatW(L" %d ", nPage).c_str() );
		}
		else {
			pStaticPage->SetText( FormatW(L"  %d  ", nPage).c_str() );
		}

		if( nPage == m_nPage ) {
			pStaticPage->SetBgTextColor( D3DCOLOR_ARGB(90, 164, 32, 48), UI_STATE_MOUSEENTER );
			pStaticPage->SetBgTextColor(  D3DCOLOR_ARGB(90, 164, 32, 48) , UI_STATE_NORMAL );
		}
		else {
			pStaticPage->SetBgTextColor( D3DCOLOR_ARGB(128, 64, 86, 128), UI_STATE_MOUSEENTER );			
			pStaticPage->SetBgTextColor( D3DCOLOR_ARGB(90, 0, 0, 0), UI_STATE_NORMAL );		
		}

		pStaticPage->SetButton( true );

		bool bEnable = ((int)m_vecSellList.size() + MAX_ITEM_COUNT_FOR_PAGE - 1 ) / MAX_ITEM_COUNT_FOR_PAGE >= nPage;

		pStaticPage->Enable( bEnable );
		pStaticPage->Show( bEnable );
	}

	if( GetTotalPage() == 0 ) {
		CONTROL( Button, ID_BUTTON_FIRST )->Enable( false );
		CONTROL( Button, ID_BUTTON_LAST )->Enable( false );

		CONTROL( Button, ID_BUTTON_BACK )->Enable( false );
		CONTROL( Button, ID_BUTTON_NEXT )->Enable( false );
	}
	else {
		CONTROL( Button, ID_BUTTON_FIRST )->Enable( GetStartPage() != 1 ? true : false );
		CONTROL( Button, ID_BUTTON_LAST )->Enable( GetStartPage() + PAGE_COUNT_FOR_SCREEN < GetTotalPage() ? true : false );

		CONTROL( Button, ID_BUTTON_BACK )->Enable( m_nPage != 1 ? true : false );
		CONTROL( Button, ID_BUTTON_NEXT )->Enable( m_nPage != GetTotalPage() ? true : false );
	}
}

void CDnMarketSellDlg::UpdateListItems()
{
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();

	bool bNewItems = (m_IsNewPage.count( m_nPage ) == 0 );
	if( bNewItems ) {
		m_IsNewPage.insert( m_nPage );
	}

	float fAlpha = -0.5f;
	static float fAlphaTerm = 0.3f;
	int nSize = (int)m_vecSellList.size();
	int nStartIndex = (m_nPage - 1) * MAX_ITEM_COUNT_FOR_PAGE;
	int nRenderItemCount = EtMin( (int)m_vecSellList.size() - nStartIndex, MAX_ITEM_COUNT_FOR_PAGE);
	for ( int i = 0; i <  nRenderItemCount; i++) 
	{
		DWORD nIndex = nStartIndex + i;
		if( nIndex < 0 || nIndex >= m_vecSellList.size() )
			break;
		
		if( m_vecSellList[nIndex].MyMarket.bPremiumTrade )
#if defined(PRE_ADD_PETALTRADE)
			CDnMarketListDlg::SetUIFileName( std::string("MarketPRMListDlg_wPetal.ui") );
#else
			CDnMarketListDlg::SetUIFileName( std::string("MarketPRMListDlg.ui") );
#endif	//#if defined(PRE_ADD_PETALTRADE)
		else
#if defined(PRE_ADD_PETALTRADE)
			CDnMarketListDlg::SetUIFileName( std::string("MarketListDlg_wPetal.ui") );
#else
			CDnMarketListDlg::SetUIFileName( std::string("MarketListDlg.ui") );
#endif	//#if defined(PRE_ADD_PETALTRADE)

		CDnMarketListDlg * pItemDlg = (CDnMarketListDlg *)CONTROL( ListBoxEx, ID_LISTBOXEX )->AddItem<CDnMarketListDlg>();

		if( bNewItems ) {
			pItemDlg->SetProperty( m_vecSellList[ nIndex ].MyMarket, fAlpha );
			fAlpha -= fAlphaTerm;
		}
		else {
			pItemDlg->SetProperty( m_vecSellList[ nIndex ].MyMarket, 1.0f );
		}
	}
}

int CDnMarketSellDlg::GetTotalPage()
{
	int nTotalPage = ( ( (int)m_vecSellList.size() + MAX_ITEM_COUNT_FOR_PAGE - 1 ) / MAX_ITEM_COUNT_FOR_PAGE );
	return nTotalPage;
}

int CDnMarketSellDlg::GetStartPage()
{
	int nStartPage = (m_nPage - 1) / PAGE_COUNT_FOR_SCREEN * PAGE_COUNT_FOR_SCREEN + 1;
	return nStartPage;
}

void CDnMarketSellDlg::ProcessRecall()
{
	SListBoxItem *pSelectedItem = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSelectedItem();
	if( pSelectedItem ) {
		CDnMarketListDlg *pItemDlg = (CDnMarketListDlg*)pSelectedItem->pData;
		if( pItemDlg && pItemDlg->GetMarketDBID() != -1 ) {
			m_nRecallDBID = pItemDlg->GetMarketDBID();
			std::wstring wszMsg = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, MESSAGEBOX_RECALL_ITEM ) , pItemDlg->GetItemName() );			

			GetInterface().MessageBox( wszMsg.c_str(), MB_YESNO, MESSAGEBOX_RECALL_ITEM, this );
		}
	}
}

void CDnMarketSellDlg::ProcessAllRecall()
{
	CEtUIListBoxEx * pListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX" );
	bool bRecall = false;

	for( int itr = 0; itr < pListBox->GetSize(); ++itr )
	{
		CDnMarketListDlg *pItemDlg = pListBox->GetItem<CDnMarketListDlg>( itr );

		if( pItemDlg && !pItemDlg->GetRemainTime() && pItemDlg->GetMarketDBID() != -1 )
		{
			GetTradeTask().GetTradeMarket().RequestMarketInterrupt( pItemDlg->GetMarketDBID() );			
			++m_nRecallCount;
			m_bEnableRecallRequest = false;
			bRecall = true;
			break;
		}
	}

	if( 0 == m_nRecallCount )
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4164 ) );	//No expired items.

	if( !bRecall )
		m_bAllRecall = false;
}

void CDnMarketSellDlg::EnableListBoxControl( bool bEnable )
{
	int nSize = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize();
	for( int i = 0; i < nSize; i++) {
		CDnMarketListDlg *pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetItem<CDnMarketListDlg>( i );
		pItemDlg->EnableAllControl( bEnable );	
	}	
}

void CDnMarketSellDlg::RefreshRequestNeed()
{
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();
	m_IsNewPage.clear();
	m_vecSellList.clear();
	m_bRequestListNeeded = true;
	m_nRecallDBID = -1;
	m_nPage = 1;
	m_wSellingCount = 0;
	m_wClosingCount = 0;
	m_wWeeklyRegisterCount = 0;
	m_wRemainRegisterItemCount = 0;
	m_lastSortType = ST_NONE;
	m_lastAscending = true;
}

void CDnMarketSellDlg::OnRecvMarketPrice( int nMarketDBID, std::vector< TMarketPrice > & vecPrice )
{
	m_pMarketPriceDlg->OnRecvMarketPrice( nMarketDBID, vecPrice );
}