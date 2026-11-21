#include "StdAfx.h"
#include "DnGuildStorageHistoryDlg.h"
#include "DnGuildStorageHistoryItemDlg.h"
#include "DnGuildTask.h"
#include "DnInterfaceString.h"
#include "DnTableDB.h"
#include "DnUIString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildStorageHistoryDlg::CDnGuildStorageHistoryDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pListBoxEx(NULL)
, m_pButtonPagePrev(NULL)
, m_pButtonPageNext(NULL)
, m_pStaticPage(NULL)
, m_nPage(0)
{
}

CDnGuildStorageHistoryDlg::~CDnGuildStorageHistoryDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnGuildStorageHistoryDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildStorageDescDlg.ui" ).c_str(), bShow );
}

void CDnGuildStorageHistoryDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_HISTORY");
	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_BUTTON_PREV");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_BUTTON_NEXT");
	m_pStaticPage = GetControl<CEtUIStatic>("ID_STATIC_PAGE");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);
}

void CDnGuildStorageHistoryDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		// Always send (0, 0) when opened for the first time.
		GetGuildTask().RequestGetGuildStorageHistoryList( 1 );

		m_nPage = 1;
	}
	else
	{
		InitControl();
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildStorageHistoryDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_CLOSE") || IsCmdControl("ID_BUTTON_CLOSE") )
		{
			Show(false);
			return;
		}
		else if( IsCmdControl("ID_BUTTON_PREV") )
		{
			InitControl();
			GetGuildTask().RequestGetGuildStorageHistoryList( (m_nPage-2)*GUILD_WAREHOUSE_HISTORYLIST_MAX+1 );
			m_nPage -= 1;
			return;
		}
		else if( IsCmdControl("ID_BUTTON_NEXT") )
		{
			InitControl();
			GetGuildTask().RequestGetGuildStorageHistoryList( m_nPage*GUILD_WAREHOUSE_HISTORYLIST_MAX+1 );
			m_nPage += 1;
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnGuildStorageHistoryDlg::InitControl()
{
	m_pStaticPage->SetText(L"");
	m_pButtonPagePrev->Enable(false);
	m_pButtonPageNext->Enable(false);
	m_pListBoxEx->RemoveAllItems();
}

static bool CompareGuildStorageHistory( TGuildWareHistory *s1, TGuildWareHistory *s2 )
{
	// Higher index is later, so put it lower
	if( s1->EventDate > s2->EventDate ) return true;
	else if( s1->EventDate < s2->EventDate ) return false;
	return false;
}

void CDnGuildStorageHistoryDlg::OnRecvGetGuildStorageHistoryList( SCGetGuildWareHistory *pPacket )
{
	m_pListBoxEx->RemoveAllItems();

	// Set NextPrev button Enable
	if( pPacket->nCurrCount > GUILD_WAREHOUSE_HISTORYLIST_MAX )
		m_pButtonPageNext->Enable(true);
	else
		m_pButtonPageNext->Enable(false);

	if( m_nPage <= 1 )
		m_pButtonPagePrev->Enable(false);
	else
		m_pButtonPagePrev->Enable(true);

	std::wstring wszDate;
	std::wstring wszItemName;
	WCHAR wszTemp[256] = {0,};
	WCHAR wszRole[32] = {0,};
	WCHAR wszType[32] = {0,};
	WCHAR wszCount[32] = {0,};

	// Sort by history index ascending
	std::vector<TGuildWareHistory *> vecGuildHistory;
	for( int i = 0; i < pPacket->nCount; ++i )
		vecGuildHistory.push_back(&pPacket->HistoryList[i]);
	std::sort( vecGuildHistory.begin(), vecGuildHistory.end(), CompareGuildStorageHistory );

	// Set history
	for( int i = 0; i < pPacket->nCount; ++i ) {
		TGuildWareHistory *pHistory = vecGuildHistory[i];
		swprintf_s( wszType, _countof(wszType), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pHistory->cInOut ? 3831 : 3830 ) );
		if( pHistory->wCount ) {
			wszItemName = CDnItem::GetItemFullName( pHistory->nContent );
			swprintf_s( wszCount, _countof(wszCount), L"%d", pHistory->wCount );
		}
		else {
			DN_INTERFACE::UTIL::GetMoneyFormatUseStr( pHistory->nContent, wszItemName );
			swprintf_s( wszCount, _countof(wszCount), L"" );
		}
		DN_INTERFACE::STRING::GetDayText( wszDate, pHistory->EventDate );
		CDnGuildStorageHistoryItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnGuildStorageHistoryItemDlg>();
		pItemDlg->SetInfo( pHistory->wszCharacterName, wszType, wszItemName.c_str(), wszCount, wszDate.c_str() );
	}
	m_pStaticPage->SetIntToText( m_nPage );
}