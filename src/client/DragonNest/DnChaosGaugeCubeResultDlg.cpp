#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosGaugeCubeResultDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnMainFrame.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChaosGaugeCubeResultDlg::CDnChaosGaugeCubeResultDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticPage( NULL )
	, m_pBtnPrior( NULL )
	, m_pBtnNext( NULL )
	, m_nMaxPage( 0 )
	, m_nCurPage( 0 )
{
	for( int i = 0; i < ITEM_PER_PAGE; ++i )
	{
		m_pSlotButtons[i] = NULL;
		m_pStaticNames[i] = NULL;
	}
}

CDnChaosGaugeCubeResultDlg::~CDnChaosGaugeCubeResultDlg()
{
	ClearSlots();
	ClearItems();
}

void CDnChaosGaugeCubeResultDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MixBoxListDlg.ui" ).c_str(), bShow );
}

void CDnChaosGaugeCubeResultDlg::InitialUpdate()
{
	char szName[32] = { 0 };
	for( int i = 0; i < ITEM_PER_PAGE; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_TEXT_NAME%d", i );
		m_pStaticNames[i] = GetControl<CEtUIStatic>( szName );

		if( !m_pSlotButtons[i] )
		{
			sprintf_s( szName, sizeof( szName ), "ID_ITEM%d", i );
			m_pSlotButtons[i] = GetControl<CDnItemSlotButton>( szName );
		}
	}

	CEtUIStatic *pTitle = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
	if( pTitle )
		pTitle->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7060 ) );

	CEtUIStatic *pAsk = GetControl<CEtUIStatic>( "ID_TEXT_ASK" );
	if( pAsk )
		pAsk->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7025 ) );

	m_pStaticPage = GetControl<CEtUIStatic>( "ID_TEXT_PAGE" );
	m_pBtnPrior = GetControl<CEtUIButton>( "ID_BT_PRIOR" );
	m_pBtnNext = GetControl<CEtUIButton>( "ID_BT_NEXT" );
}

void CDnChaosGaugeCubeResultDlg::InitCustomControl( CEtUIControl *pControl )
{
	if( !pControl )
		return;

	int nIdx = -1;
	if( sscanf_s( pControl->GetControlName(), "ID_ITEM%d", &nIdx ) == 1 && nIdx >= 0 && nIdx < ITEM_PER_PAGE )
		m_pSlotButtons[nIdx] = static_cast<CDnItemSlotButton*>( pControl );
}

void CDnChaosGaugeCubeResultDlg::ClearSlots()
{
	for( int i = 0; i < ITEM_PER_PAGE; ++i )
	{
		if( m_pSlotButtons[i] )
			m_pSlotButtons[i]->ResetSlot();
		if( m_pStaticNames[i] )
			m_pStaticNames[i]->SetText( L"" );
	}
}

void CDnChaosGaugeCubeResultDlg::ClearItems()
{
	for( size_t i = 0; i < m_vecItems.size(); ++i )
		SAFE_DELETE( m_vecItems[i].pItem );
	m_vecItems.clear();
	m_mapGold.clear();
}

void CDnChaosGaugeCubeResultDlg::LoadResultItems( int nChaosNum )
{
	ClearSlots();
	ClearItems();

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TCHAOSCUBERESULT );
	if( !pSox )
		return;

	int nCount = pSox->GetItemCount();
	for( int i = 0; i < nCount; ++i )
	{
		int nRowID = pSox->GetItemID( i );
		DNTableCell *pNumCell = pSox->GetFieldFromLablePtr( nRowID, "_ChaoscubeNum" );
		if( pNumCell && pNumCell->GetInteger() != nChaosNum )
			continue;

		DNTableCell *pItemCell = pSox->GetFieldFromLablePtr( nRowID, "_ItemID" );
		if( !pItemCell || pItemCell->GetInteger() <= 0 )
			continue;

		int nItemID = pItemCell->GetInteger();
		TItemInfo itemInfo;
		if( !CDnItem::MakeItemInfo( nItemID, 1, itemInfo ) )
			continue;

		CDnItem *pItem = GetItemTask().CreateItem( itemInfo );
		if( !pItem )
			continue;

		DNTableCell *pCountCell = pSox->GetFieldFromLablePtr( nRowID, "_Count" );
		if( pCountCell )
			pItem->SetOverlapCount( pCountCell->GetInteger() );

		SResultItemEntry entry;
		entry.nRowID = nRowID;
		entry.pItem = pItem;
		DNTableCell *pPeriodCell = pSox->GetFieldFromLablePtr( nRowID, "_Period" );
		if( pPeriodCell )
			entry.nPeriod = pPeriodCell->GetInteger();

		int nEntryIdx = (int)m_vecItems.size();
		m_vecItems.push_back( entry );

		DNTableCell *pGoldCell = pSox->GetFieldFromLablePtr( nRowID, "_Gold" );
		if( pGoldCell && pGoldCell->GetInteger() > 0 )
			m_mapGold[nEntryIdx] = (INT64)pGoldCell->GetInteger();
	}

	m_nCurPage = 0;
	if( m_vecItems.empty() )
		m_nMaxPage = 0;
	else
		m_nMaxPage = ( (int)m_vecItems.size() - 1 ) / ITEM_PER_PAGE;

	RefreshPage();
}

void CDnChaosGaugeCubeResultDlg::RefreshPage()
{
	ClearSlots();

	int nStart = m_nCurPage * ITEM_PER_PAGE;
	for( int i = 0; i < ITEM_PER_PAGE; ++i )
	{
		int nIdx = nStart + i;
		if( nIdx < 0 || nIdx >= (int)m_vecItems.size() )
			break;

		CDnItem *pItem = m_vecItems[nIdx].pItem;
		if( !pItem )
			continue;

		if( m_pSlotButtons[i] )
			m_pSlotButtons[i]->SetItem( pItem, pItem->GetOverlapCount() );

		if( m_pStaticNames[i] )
		{
			if( pItem->GetClassID() == 1073750029 )
			{
				std::map<int, INT64>::iterator it = m_mapGold.find( nIdx );
				if( it != m_mapGold.end() )
				{
					std::wstring strGold;
					GoldToString( strGold, it->second,
						GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1766 ),
						GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1781 ),
						GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1783 ) );
					m_pStaticNames[i]->SetText( strGold );
					continue;
				}
			}
			m_pStaticNames[i]->SetText( pItem->GetName() );
		}
	}

	if( m_pStaticPage )
	{
		WCHAR wszPage[32] = { 0 };
		swprintf_s( wszPage, _countof( wszPage ), L"%d / %d", m_nCurPage + 1, m_nMaxPage + 1 );
		m_pStaticPage->SetText( wszPage );
	}
}

void CDnChaosGaugeCubeResultDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	RefreshPage();
	CEtUIDialog::Show( bShow );
}

bool CDnChaosGaugeCubeResultDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( uMsg == WM_MOUSEWHEEL )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		ScreenToClient( hWnd, &MousePoint );
		lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

		if( IsMouseInDlg() )
		{
			UINT uLines = 3;
			SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int nScrollAmount = int( (short)HIWORD( wParam ) ) / WHEEL_DELTA * (int)uLines;
			if( nScrollAmount > 0 && m_nCurPage > 0 && m_pBtnPrior )
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pBtnPrior, 0 );
			else if( nScrollAmount < 0 && m_nCurPage < m_nMaxPage && m_pBtnNext )
				ProcessCommand( EVENT_BUTTON_CLICKED, false, m_pBtnNext, 0 );

			CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );
			return true;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnChaosGaugeCubeResultDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_PRIOR" ) )
		{
			if( m_nCurPage > 0 )
				--m_nCurPage;
			else
				m_nCurPage = m_nMaxPage;
			RefreshPage();
			return;
		}
		else if( IsCmdControl( "ID_BT_NEXT" ) )
		{
			if( m_nCurPage < m_nMaxPage )
				++m_nCurPage;
			else
				m_nCurPage = 0;
			RefreshPage();
			return;
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#endif // PRE_ADD_CHAOSCUBE
