#include "StdAfx.h"
#include "DnMarketSellDlg.h"
#include "DnTradeTask.h"
#include "DnMarketPriceDlg.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnMarketAccountDlg.h"
#include "DnMarketListDlg.h"
#include "DnCommonUtil.h"
#include "DnInterfaceString.h"
#include "DnMarketTabDlg.h"
#include "DnMainMenuDlg.h"
#include "DnMainFrame.h"
#include "DnMarketAboutTaxDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMarketAccountDlg::CDnMarketAccountDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_bRequestListNeeded( true )
	, m_lastSortType( ST_NONE )
	, m_lastAscending( true )
	, m_nPage( 1 )
	, m_pMarketAboutDlg( NULL )
{
}

CDnMarketAccountDlg::~CDnMarketAccountDlg(void)
{
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();
	SAFE_DELETE( m_pMarketAboutDlg );
}

void CDnMarketAccountDlg::Initialize( bool bShow )
{
#if defined(PRE_ADD_PETALTRADE)
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "MarketAccountDlg_wPetal.ui" ).c_str(), bShow );
#else
	BaseClass::Initialize( CEtResourceMng::GetInstance().GetFullName( "MarketAccountDlg.ui" ).c_str(), bShow );
#endif	//#if defined(PRE_ADD_PETALTRADE)
}

void CDnMarketAccountDlg::InitCustomControl( CEtUIControl *pControl )
{
}

void CDnMarketAccountDlg::InitialUpdate()
{
#if defined(PRE_ADD_PETALTRADE)
	m_pMarketAboutDlg = new CDnMarketAboutDlg( UI_TYPE_CHILD, this, -1, this );
	m_pMarketAboutDlg->Initialize( false );
#endif	//#if defined(PRE_ADD_PETALTRADE)

	CONTROL( Button, ID_SORT_NAME );
	CONTROL( Button, ID_SORT_TIME );
	CONTROL( Button, ID_SORT_PRICE );

	CONTROL( Static, ID_STATIC_TAX)->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4060 ) );

	RefreshPageControls();
}

void CDnMarketAccountDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		bool bSortNeeded = false;
		bool bChangePageNeeded = false;
		if( IsCmdControl( "ID_BUTTON_ACCOUNT" ) )	
		{
			ProcessAccount();
			return;
		}
		else if( IsCmdControl( "ID_BUTTON_ALLACCOUNT" ) )	
		{
			m_IsNewPage.clear();
			GetTradeTask().GetTradeMarket().RequestMarketCalculationAll();
			return;
		}
		else if( IsCmdControl( "ID_BUTTON_NEW" ) )	
		{
			m_IsNewPage.clear();
			GetTradeTask().GetTradeMarket().RequestMarketCalculationList();
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
			if( sscanf( pControl->GetControlName() + strlen("ID_STATIC_PAGE"), "%d", &nIndex) == 1 ) {
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
		else if( IsCmdControl( "ID_SORT_TIME" ) )	// Sort by expiration time
		{
			m_lastAscending = (m_lastSortType == ST_SELLTIME) ? !m_lastAscending : true;
			m_lastSortType = ST_SELLTIME;
			bSortNeeded = true;
		}
		else if( IsCmdControl( "ID_SORT_PRICE" ) )		// Sort by price
		{
			m_lastAscending = (m_lastSortType == ST_PRICE) ? !m_lastAscending : true;
			m_lastSortType = ST_PRICE;
			bSortNeeded = true;
		}
#if defined(PRE_ADD_PETALTRADE)
		else if( IsCmdControl( "ID_BUTTON_TAX" ) )
		{
			m_pMarketAboutDlg->Show( true );		
		}
#endif	//#if defined(PRE_ADD_PETALTRADE)
		else if( IsCmdControl( "ID_BUTTON_PosPETAL" ) )
		{
			CDnMarketTabDlg * pMarketTab = (CDnMarketTabDlg *)m_pParentDialog;
			pMarketTab->ShowPosPetal( true );
		}

		if( bChangePageNeeded ) {
			UpdateListItems();
			RefreshPageControls();
		}
		if( bSortNeeded ) {
			std::sort( m_vecAccountList.begin(), m_vecAccountList.end(), SellListSorter( m_lastSortType, m_lastAscending ) );	
			UpdateListItems();
		}
	}
	else if ( nCommand == EVENT_LISTBOX_ITEM_DBLCLK ) {
		if( IsCmdControl("ID_LISTBOXEX")) {
			ProcessAccount();
		}
	}
#ifndef PRE_FIX_11027
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl("ID_LISTBOXEX" ) )
		{
			SListBoxItem *pSelectedItem = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSelectedItem();
			if( pSelectedItem ) {
				CDnMarketListDlg *pItemDlg = (CDnMarketListDlg*)pSelectedItem->pData;
				if( pItemDlg && pItemDlg->GetMarketDBID() != -1 && CDnActor::s_hLocalActor ) {

					MONEY_TYPE nTotalMoney = pItemDlg->GetMoney();
					TAX_TYPE nAccountTax = EtMax(1, CommonUtil::CalcTax(TAX_TRADESELLING, CDnActor::s_hLocalActor->GetLevel(), nTotalMoney ));
					MONEY_TYPE nAccountMoney = nTotalMoney - nAccountTax;
					SetMoneyControl(nTotalMoney, nAccountTax, nAccountMoney);
				}
			}
			return;
		}
	}
#endif

	BaseClass::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMarketAccountDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{		
		GetTradeTask().GetTradeMarket().RequestMarketCalculationList();
		if( m_bRequestListNeeded ) 
		{			// Request only once initially.... (Think it won't be needed afterward.)
			m_IsNewPage.clear();
			m_bRequestListNeeded = false;
		}
	}
	else
	{
	}
	BaseClass::Show( bShow );
}

void CDnMarketAccountDlg::Process( float fElapsedTime )
{
	int nSize = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize();
	for( int i = 0; i < nSize; i++) {
		CDnMarketListDlg *pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetItem<CDnMarketListDlg>( i );
		pItemDlg->ProcessAlpha( fElapsedTime);
	}
	BaseClass::Process( fElapsedTime );
}

bool CDnMarketAccountDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

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

	return BaseClass::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnMarketAccountDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );
}

void CDnMarketAccountDlg::OnRecvUpdateList( std::vector< TMarketCalculationInfo > &vecCalculationList )
{
	m_vecAccountList.clear();
	m_nPage = 1;

	for each( TMarketCalculationInfo Market in vecCalculationList ) {
		MyAccountItem myItem;
		myItem.wszItemName = CDnItem::GetItemFullName( Market.nItemID );
		myItem.Market = Market;
		m_vecAccountList.push_back( myItem );
	}
	UpdateListItems();
	RefreshPageControls();
}

void CDnMarketAccountDlg::OnRecvRemoveItem( int nMarketDBID )
{
	if( nMarketDBID == -1 ) {
		// Settlement failed...
		return;
	}
	int nSize = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize();

	for( int i = 0; i < nSize; i++) {
		CDnMarketListDlg *pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetItem<CDnMarketListDlg>( i );
		if( pItemDlg->GetMarketDBID() == nMarketDBID ) {
			CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveItem( i );
			GetItemTask().PlayMoneySound();
			break;
		}
	}

	std::vector< MyAccountItem >::iterator it = m_vecAccountList.begin();
	while( it != m_vecAccountList.end() ) {
		if( it->Market.nMarketDBID == nMarketDBID ) {
			it = m_vecAccountList.erase( it );
		}
		else {
			++it;
		}
	}
#ifdef PRE_FIX_11027
	RefreshAccount();
#endif
	RefreshPageControls();
#ifndef PRE_FIX_11027
	bool bLostFocus = false;
	SListBoxItem *pSelectedItem = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSelectedItem();
	if( pSelectedItem ) {
		CDnMarketListDlg *pItemDlg = (CDnMarketListDlg*)pSelectedItem->pData;
		if( pItemDlg && pItemDlg->GetMarketDBID() == -1 ) {
			bLostFocus = true;
		}
	}
	else {
		bLostFocus = true;
	}
	if( bLostFocus ) {
		SetMoneyControl( 0, 0, 0);
	}
#endif
}

void CDnMarketAccountDlg::RemoveAllItem()	// CalculationAll
{
	if( CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSize() > 0 ) {
		GetItemTask().PlayMoneySound();	// Do not play sound if there is none...
	}
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();		
	SetMoneyControl( 0, 0, 0, 0);
	m_vecAccountList.clear();
	RefreshPageControls();
}

void CDnMarketAccountDlg::ProcessAccount()
{
	SListBoxItem *pSelectedItem = CONTROL( ListBoxEx, ID_LISTBOXEX )->GetSelectedItem();
	if( pSelectedItem ) {
		CDnMarketListDlg *pItemDlg = (CDnMarketListDlg*)pSelectedItem->pData;
		if( pItemDlg && pItemDlg->GetMarketDBID() != -1 ) {
			m_IsNewPage.clear();
			GetTradeTask().GetTradeMarket().RequestMarketCalculation( pItemDlg->GetMarketDBID() );
		}
	}
}

void CDnMarketAccountDlg::RefreshPageControls()
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

		bool bEnable = ((int)m_vecAccountList.size() + MAX_ITEM_COUNT_FOR_PAGE - 1 ) / MAX_ITEM_COUNT_FOR_PAGE >= nPage;

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

void CDnMarketAccountDlg::UpdateListItems()
{
	CONTROL( ListBoxEx, ID_LISTBOXEX )->RemoveAllItems();
	SetMoneyControl( 0, 0, 0, 0);

	bool bNewItems = (m_IsNewPage.count( m_nPage ) == 0 );
	if( bNewItems ) {
		m_IsNewPage.insert( m_nPage );
	}

	float fAlpha = -0.5f;
	static float fAlphaTerm = 0.3f;

	int nSize = (int)m_vecAccountList.size();
	int nStartIndex = (m_nPage - 1) * MAX_ITEM_COUNT_FOR_PAGE;
	int nRenderItemCount = EtMin( (int)m_vecAccountList.size() - nStartIndex, MAX_ITEM_COUNT_FOR_PAGE);
	for ( int i = 0; i <  nRenderItemCount; i++) {		
		CDnMarketListDlg* pItemDlg = CONTROL( ListBoxEx, ID_LISTBOXEX )->AddItem<CDnMarketListDlg>();
		if( bNewItems ) {
			pItemDlg->SetProperty( m_vecAccountList[ nStartIndex + i ].Market, fAlpha );
			fAlpha -= fAlphaTerm;
		}
		else {
			pItemDlg->SetProperty( m_vecAccountList[ nStartIndex + i ].Market , 1.0f);
		}
	}
#ifdef PRE_FIX_11027
	RefreshAccount();
#endif
}

int CDnMarketAccountDlg::GetTotalPage()
{
	int nTotalPage = ( ( (int)m_vecAccountList.size() + MAX_ITEM_COUNT_FOR_PAGE - 1 ) / MAX_ITEM_COUNT_FOR_PAGE );
	return nTotalPage;
}

int CDnMarketAccountDlg::GetStartPage()
{
	int nStartPage = (m_nPage - 1) / PAGE_COUNT_FOR_SCREEN * PAGE_COUNT_FOR_SCREEN + 1;
	return nStartPage;
}

void CDnMarketAccountDlg::RefreshRequestNeed()
{
	m_vecAccountList.clear();
	m_lastSortType = ST_NONE;
	m_lastAscending = true;
	m_nPage = 1;
	m_IsNewPage.clear();;
	m_bRequestListNeeded = true;
	SetMoneyControl( 0 , 0, 0, 0 );
}

void CDnMarketAccountDlg::SetMoneyControl( MONEY_TYPE nTotalMoney, TAX_TYPE nAccountTax, MONEY_TYPE nAccountMoney, MONEY_TYPE nTotalPetal )
{
	INT64 nMoney = 0;
	WCHAR wszBuffer[32]={0};
	std::wstring strString;
	//////////////////////////////////////////////////////////////////////////
	CDnMoneyControl::GetStrMoneyG( nTotalMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_GOLD )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_GOLD )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyS( nTotalMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_SILVER )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_SILVER )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyC( nTotalMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_BRONZE )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_BRONZE )->SetTooltipText( strString.c_str() );
	//////////////////////////////////////////////////////////////////////////
	CDnMoneyControl::GetStrMoneyG( nAccountTax, wszBuffer, &nMoney );
	CONTROL( Static, ID_GOLD_FEE )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_GOLD_FEE )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyS( nAccountTax, wszBuffer, &nMoney );
	CONTROL( Static, ID_SILVER_FEE )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_SILVER_FEE )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyC( nAccountTax, wszBuffer, &nMoney );
	CONTROL( Static, ID_BRONZE_FEE )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_BRONZE_FEE )->SetTooltipText( strString.c_str() );
	//////////////////////////////////////////////////////////////////////////
	CDnMoneyControl::GetStrMoneyG( nAccountMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_GOLD_ACCOUNT )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_GOLD_ACCOUNT )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyS( nAccountMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_SILVER_ACCOUNT )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_SILVER_ACCOUNT )->SetTooltipText( strString.c_str() );

	CDnMoneyControl::GetStrMoneyC( nAccountMoney, wszBuffer, &nMoney );
	CONTROL( Static, ID_BRONZE_ACCOUNT )->SetText( wszBuffer );
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nMoney, strString );
	CONTROL( Static, ID_BRONZE_ACCOUNT )->SetTooltipText( strString.c_str() );

#ifdef PRE_MOD_PETAL_WRITE
	CONTROL( Static, ID_PETAL_ACCOUNT )->SetText( DN_INTERFACE::UTIL::GetAddCommaString( (int)nTotalPetal ).c_str() );
#else // PRE_MOD_PETAL_WRITE
	CONTROL( Static, ID_PETAL_ACCOUNT )->SetIntToText( (int)nTotalPetal );
#endif // PRE_MOD_PETAL_WRITE
	DN_INTERFACE::UTIL::GetValue_2_String( (int)nTotalPetal, strString );
	CONTROL( Static, ID_PETAL_ACCOUNT )->SetTooltipText( strString.c_str() );
}

#ifdef PRE_FIX_11027
void CDnMarketAccountDlg::RefreshAccount()
{
	MONEY_TYPE sumTotalMoney = 0;
	TAX_TYPE sumAccountTax = 0;
	MONEY_TYPE sumAccountMoney = 0;
	MONEY_TYPE sumTotalPetal = 0;

	int nSize = (int)m_vecAccountList.size();
	for( int itr = 0; itr < nSize; ++itr )
	{
		if( m_vecAccountList[itr].Market.nMarketDBID != -1 && CDnActor::s_hLocalActor )
		{
#if defined(PRE_ADD_PETALTRADE)
			if( 3 == m_vecAccountList[itr].Market.cPayMethodCode )	// Petal
			{
				MONEY_TYPE nTotalMoney = m_vecAccountList[itr].Market.nPrice;
				TAX_TYPE nAccountTax = EtMax(1, CommonUtil::CalcTax( TAX_TRADEPETALSELLING, CDnActor::s_hLocalActor->GetLevel(), nTotalMoney ) );
				MONEY_TYPE nAccountMoney = nTotalMoney - nAccountTax;

				sumTotalPetal += nAccountMoney;
			}
			else
#endif	//#if defined(PRE_ADD_PETALTRADE)
			{
				MONEY_TYPE nTotalMoney = m_vecAccountList[itr].Market.nPrice;
				TAX_TYPE nAccountTax = EtMax(1, CommonUtil::CalcTax( TAX_TRADESELLING, CDnActor::s_hLocalActor->GetLevel(), nTotalMoney ) );
				MONEY_TYPE nAccountMoney = nTotalMoney - nAccountTax;

				sumTotalMoney += nTotalMoney;
				sumAccountTax += nAccountTax;
				sumAccountMoney += nAccountMoney;
			}
		}
	}
	SetMoneyControl(sumTotalMoney, sumAccountTax, sumAccountMoney, sumTotalPetal);
}
#endif
