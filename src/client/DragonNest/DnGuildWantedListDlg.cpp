#include "StdAfx.h"
#include "DnGuildWantedListDlg.h"
#include "DnGuildTask.h"
#include "DnInterface.h"
#include "DnGuildWantedListItemDlg.h"
#include "DnGuildWarTask.h"
#include "DnGuildWantedListPopupDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnGuildWantedListDlg::CDnGuildWantedListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
, m_pWantedTabButton(NULL)
, m_pJoinTabButton(NULL)
#endif
, m_pListBoxEx(NULL)
, m_pButtonPagePrev(NULL)
, m_pButtonPageNext(NULL)
, m_pStaticPage(NULL)
, m_nPage(0)
, m_nPrevPage(0)
, m_pStaticCount(NULL)
, m_pButtonOK(NULL)
#ifdef PRE_ADD_GUILD_EASYSYSTEM
, m_pComboCategory(NULL)
, m_pComboSort(NULL)
, m_pEditBoxGuildName(NULL)
, m_pStaticGuildName(NULL)
, m_pButtonSearch(NULL)
, m_pButtonReset(NULL)
, m_pButtonHomepage(NULL)
#else
, m_pButtonCancel(NULL)
#endif
, m_pGuildWantedListPopupDlg(NULL)
, m_nRequestCount(0)
, m_nMaxRequestCount(0)
{
}

CDnGuildWantedListDlg::~CDnGuildWantedListDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
	SAFE_DELETE( m_pGuildWantedListPopupDlg );
}

void CDnGuildWantedListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GuildWantedListDlg.ui" ).c_str(), bShow );
}

void CDnGuildWantedListDlg::InitialUpdate()
{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
	m_pWantedTabButton = GetControl<CEtUIRadioButton>("ID_RBT_WANTEDLIST");
	m_pJoinTabButton = GetControl<CEtUIRadioButton>("ID_RBT_JOINLIST");
#endif
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");
	m_pButtonPagePrev = GetControl<CEtUIButton>("ID_BUTTON_PREV");
	m_pButtonPageNext = GetControl<CEtUIButton>("ID_BUTTON_NEXT");
	m_pStaticPage = GetControl<CEtUIStatic>("ID_TEXT_PAGE");
	m_pStaticCount = GetControl<CEtUIStatic>("ID_TEXT_COUNT");
	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_OK");
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_pComboCategory = GetControl<CEtUIComboBox>("ID_COMBOBOX_PURPOSE");
	m_pComboSort = GetControl<CEtUIComboBox>("ID_COMBOBOX_SORT");
	m_pEditBoxGuildName = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_GUILDNAME");
	m_pStaticGuildName = GetControl<CEtUIStatic>("ID_TEXT_GUILDNAME");
	m_pButtonSearch = GetControl<CEtUIButton>("ID_BT_SEARCH");
	m_pButtonReset = GetControl<CEtUIButton>("ID_BT_RESET");
	m_pButtonHomepage = GetControl<CEtUIButton>("ID_BT_HOMEPAGE");
#else
	m_pButtonCancel = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
#endif

#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3870 ), NULL, GuildRecruitSystem::PuposeCodeType::AllCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3857 ), NULL, GuildRecruitSystem::PuposeCodeType::FriendlyCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3858 ), NULL, GuildRecruitSystem::PuposeCodeType::WarCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3859 ), NULL, GuildRecruitSystem::PuposeCodeType::NestCode );
	m_pComboCategory->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3860 ), NULL, GuildRecruitSystem::PuposeCodeType::PvPCode );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3845 ), NULL, GuildRecruitSystem::SortType::GuildLevelDesc );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3846 ), NULL, GuildRecruitSystem::SortType::GuildLevelAsc );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3847 ), NULL, GuildRecruitSystem::SortType::GuildMemberDesc );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3848 ), NULL, GuildRecruitSystem::SortType::GuildMemberAsc );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3849 ), NULL, GuildRecruitSystem::SortType::GuildNameDesc );
	m_pComboSort->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3850 ), NULL, GuildRecruitSystem::SortType::GuildNameAsc );
	m_pButtonHomepage->Enable( false );
#endif
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
	m_pButtonHomepage->Show( false );
#endif
	m_pButtonPagePrev->SetDisableTime( 1.0f );
	m_pButtonPageNext->SetDisableTime( 1.0f );

	m_pGuildWantedListPopupDlg = new CDnGuildWantedListPopupDlg( UI_TYPE_CHILD, this );
	m_pGuildWantedListPopupDlg->Initialize( false );

#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
	m_pWantedTabButton->SetChecked( true );
	m_pButtonCancel->Show( false );
#endif
}

void CDnGuildWantedListDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
		// Since it moves inside a tab dialog within GuildBase, need to check the parent's parent.
		// Calling TabDialog::SetCheckedTab during InitialUpdate automatically Shows that child tab Dialog,
		// Because of this, packets end up being sent during loading.
		// Checking by hWnd because there is currently no way to tell if loading is finished; a structural change seems necessary.
		if( m_pParentDialog && m_pParentDialog->GetParentDialog() && m_pParentDialog->GetParentDialog()->GetHWnd() )
		{
			// Always send 0 when opened for the first time.
			m_nPrevPage = m_nPage = 0;
			RequestSearch();
			GetGuildTask().RequestGetGuildRecruitRequestCount();
		}
#else
		// Always send 0 when opened for the first time.
		if( m_pWantedTabButton->IsChecked() )
		{
			m_nPrevPage = m_nPage = 0;
			GetGuildTask().RequestGetGuildRecruitList( m_nPage );
			GetGuildTask().RequestGetGuildRecruitRequestCount();
		}
		else if( m_pJoinTabButton->IsChecked() )
		{
			m_pStaticPage->SetIntToText( 1 );
			GetGuildTask().RequestGetGuildRecruitMyList();
		}
#endif
	}
	else
	{
		InitControl();
		ShowChildDialog( m_pGuildWantedListPopupDlg, false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnGuildWantedListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_OK") )
		{
			int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
			if( nSelectIndex != -1 )
			{
				CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
				if( pItem )
				{
					WCHAR wszMsg[256] = {0,};
					bool bNormalMsg = true;
					eGuildWarStepType eWarStep = CDnGuildWarTask::GetInstance().GetCurrentGuildWarEventTimeType();
					if( eWarStep >= GUILDWAR_STEP_PREPARATION && eWarStep <= GUILDWAR_STEP_REWARD ) bNormalMsg = false;

					if( bNormalMsg ) swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3991 ), pItem->GetGuildName() );
					else swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3397 ), pItem->GetGuildName() );
					GetInterface().MessageBox( wszMsg, MB_YESNO, REQUEST_JOIN_DIALOG, this, false );
				}
			}
			return;
		}
#ifdef PRE_ADD_GUILD_EASYSYSTEM
		else if( IsCmdControl("ID_BT_SEARCH") )
		{
			m_nPrevPage = m_nPage = 0;
			RequestSearch();
			return;
		}
		else if( IsCmdControl("ID_BT_RESET") )
		{
			// Requested to show the initial list displayed when the window first opened upon pressing Reset.
			m_pComboCategory->SetSelectedByIndex( 0 );
			m_pComboSort->SetSelectedByIndex( 0 );
			m_pEditBoxGuildName->ClearText();
			m_pStaticGuildName->Show( true );
			m_nPrevPage = m_nPage = 0;
			RequestSearch();
			return;
		}
#else
		else if( IsCmdControl("ID_BUTTON_CANCEL") )
		{
			int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
			if( nSelectIndex != -1 )
			{
				CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
				if( pItem )
				{
					WCHAR wszMsg[256] = {0,};
					swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3998 ), pItem->GetGuildName() );
					GetInterface().MessageBox( wszMsg, MB_YESNO, REQUEST_CANCEL_DIALOG, this, false );
				}
			}
			return;
		}
#endif
		else if( IsCmdControl("ID_BUTTON_PREV") )
		{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
			if( m_pJoinTabButton->IsChecked() )
				return;
#endif

			if( m_nPage == 0 )
				return;

			m_nPrevPage = m_nPage;
			m_nPage -= 1;
#ifdef PRE_ADD_GUILD_EASYSYSTEM
			RequestSearch();
#else
			GetGuildTask().RequestGetGuildRecruitList( m_nPage );
#endif
			return;
		}
		else if( IsCmdControl("ID_BUTTON_NEXT") )
		{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
			if( m_pJoinTabButton->IsChecked() )
				return;
#endif

			m_nPrevPage = m_nPage;
			m_nPage += 1;
#ifdef PRE_ADD_GUILD_EASYSYSTEM
			RequestSearch();
#else
			GetGuildTask().RequestGetGuildRecruitList( m_nPage );
#endif
			return;
		}
#ifdef PRE_ADD_GUILD_EASYSYSTEM
		else if( IsCmdControl("ID_BT_HOMEPAGE") )
		{
			int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
			if( nSelectIndex != -1 )
			{
				CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
				if( pItem )
				{
					std::wstring wszHomepage = pItem->GetHomepage();
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
					std::string url = "";
					url.assign(wszHomepage.begin(), wszHomepage.end());
					GetInterface().OpenBrowser(url, (float)CEtDevice::GetInstance().Width(), (float)CEtDevice::GetInstance().Height(), CDnInterface::eBPT_CENTER, eGBT_SIMPLE);
#endif
				}
			}
		}
#endif
	}
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED && bTriggeredByUser )
	{
		if( IsCmdControl("ID_RBT_WANTEDLIST") )
		{
			m_pButtonOK->Show( true );
			m_pButtonCancel->Show( false );
			m_pButtonPageNext->Enable( true );
			m_pButtonPagePrev->Enable( true );

			InitControl();
			if( bTriggeredByUser ) GetGuildTask().RequestGetGuildRecruitList( m_nPage );
		}
		else if( IsCmdControl("ID_RBT_JOINLIST") )
		{
			m_pButtonOK->Show( false );
			m_pButtonCancel->Show( true );
			m_pButtonPageNext->Enable( false );
			m_pButtonPagePrev->Enable( false );

			InitControl();
			m_pStaticPage->SetIntToText( 1 );
			if( bTriggeredByUser ) GetGuildTask().RequestGetGuildRecruitMyList();
		}
	}
#endif
	else if( nCommand == EVENT_LISTBOX_SELECTION )
	{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
		if( m_nRequestCount < m_nMaxRequestCount )
			m_pButtonOK->Enable( true );

		bool bHomepage = false;
		int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
		if( nSelectIndex != -1 )
		{
			CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
			if( pItem )
			{
#ifdef PRE_MOD_GUILD_EASYSYSTEM_NO_HOMEPAGE
#else
				std::wstring wszHomepage = pItem->GetHomepage();
				if( !wszHomepage.empty() ) bHomepage = true;
#endif
			}
		}
		m_pButtonHomepage->Enable( bHomepage );
#else
		if( m_pWantedTabButton->IsChecked() )
		{
			if( m_nRequestCount < m_nMaxRequestCount )
				m_pButtonOK->Enable( true );
		}
		else if( m_pJoinTabButton->IsChecked() )
		{
			m_pButtonCancel->Enable( true );
		}
#endif
	}
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	else if( nCommand == EVENT_EDITBOX_FOCUS )
	{
		m_pStaticGuildName->Show( false );
	}
	else if( nCommand == EVENT_EDITBOX_RELEASEFOCUS )
	{
		if( m_pEditBoxGuildName->GetTextLength() == 0 )
			m_pStaticGuildName->Show( true );
	}
#endif

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnGuildWantedListDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hWnd;

	if( !m_bShow )
	{
		return false;
	}

	if( m_pGuildWantedListPopupDlg && m_pGuildWantedListPopupDlg->IsShow() )
	{
		if( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE )
			ShowChildDialog( m_pGuildWantedListPopupDlg, false );
	}

	bool bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	if( bRet && !drag::IsValid() && !m_pComboCategory->IsOpenedDropDownBox() && !m_pComboSort->IsOpenedDropDownBox() )
#else
	if( bRet && !drag::IsValid() )
#endif
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		fMouseX = MousePoint.x / GetScreenWidth();
		fMouseY = MousePoint.y / GetScreenHeight();

		switch( uMsg )
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_MOUSEMOVE:
			{
				bool bIsInsideItem = false;
				for( int i = m_pListBoxEx->GetScrollBar()->GetTrackPos(); i < m_pListBoxEx->GetSize(); ++i ) {
					CDnGuildWantedListItemDlg *pDlg = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(i);
					if( !pDlg ) continue;
					SUICoord uiCoord;
					pDlg->GetDlgCoord( uiCoord );
					if( uiCoord.IsInside( fMouseX, fMouseY ) ) {
						bIsInsideItem = true;
						m_pGuildWantedListPopupDlg->SetPosition( fMouseX, fMouseY );
						static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH);
						SUICoord sDlgCoord;
						m_pGuildWantedListPopupDlg->GetDlgCoord( sDlgCoord );
						if( (sDlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
							sDlgCoord.fX -= (sDlgCoord.Right()+fXRGap - GetScreenWidthRatio());
						m_pGuildWantedListPopupDlg->SetDlgCoord( sDlgCoord );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
						m_pGuildWantedListPopupDlg->SetInfo( pDlg->GetGuildName(), pDlg->GetLevel(), pDlg->GetCount(), pDlg->GetCountMax(), pDlg->GetNotice(), pDlg->GetHomepage(), pDlg->GetGuildMasterName() );
#else
						m_pGuildWantedListPopupDlg->SetInfo( pDlg->GetGuildName(), pDlg->GetLevel(), pDlg->GetCount(), pDlg->GetCountMax(), pDlg->GetNotice() );
#endif
						break;
					}
				}
				ShowChildDialog( m_pGuildWantedListPopupDlg, bIsInsideItem );
			}
			break;
		}
	}
	return bRet;
}

void CDnGuildWantedListDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl("ID_YES") )
		{
			switch( nID )
			{
			case REQUEST_JOIN_DIALOG:
				{
					int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
					if( nSelectIndex != -1 )
					{
						CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
						if( pItem )
							GetGuildTask().RequestJoinGuild( pItem->GetGuildUID(), false );
						m_RequestGuildUID = pItem->GetGuildUID();
					}
				}
				break;
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
			case REQUEST_CANCEL_DIALOG:
				{
					int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
					if( nSelectIndex != -1 )
					{
						CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
						if( pItem )
							GetGuildTask().RequestJoinGuild( pItem->GetGuildUID(), true );
					}
				}
				break;
#endif
			}
		}
	}
}


void CDnGuildWantedListDlg::InitControl()
{
	m_pStaticPage->SetText(L"");
	m_pListBoxEx->RemoveAllItems();
	m_pButtonOK->Enable( false );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
	m_pButtonCancel->Enable( false );
#endif
}

#ifdef PRE_ADD_GUILD_EASYSYSTEM
BYTE CDnGuildWantedListDlg::GetSearchPurpose()
{
	BYTE cPurpose = 0;
	if( m_pComboCategory )
	{
		int nPurpose = 0;
		if( m_pComboCategory->GetSelectedValue( nPurpose ) )
			cPurpose = nPurpose;
	}
	return cPurpose;
}

const WCHAR *CDnGuildWantedListDlg::GetSearchGuildName()
{
	if( !m_pEditBoxGuildName ) return L"";
	return m_pEditBoxGuildName->GetText();
}

BYTE CDnGuildWantedListDlg::GetSearchSort()
{
	BYTE cSort = GuildRecruitSystem::SortType::GuildLevelDesc;	// Requested to use this as the default value.
	if( m_pComboSort )
	{
		int nSort = 0;
		if( m_pComboSort->GetSelectedValue( nSort ) )
			cSort = nSort;
	}
	return cSort;
}

void CDnGuildWantedListDlg::RequestSearch()
{
	GetGuildTask().RequestGetGuildRecruitList( m_nPage, GetSearchPurpose(), GetSearchGuildName(), GetSearchSort() );
}
#endif

void CDnGuildWantedListDlg::OnRecvGetGuildRecruitList( GuildRecruitSystem::SCGuildRecruitList *pPacket )
{
	if( pPacket->cCount == 0 )
	{
		m_nPage = m_nPrevPage;
		m_pStaticPage->SetIntToText( m_nPage+1 );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
		GetInterface().ShowCaptionDialog( CDnInterface::typeCaption3, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3872 ), textcolor::YELLOW, 4.0f );
		m_pListBoxEx->RemoveAllItems();
#endif
		return;
	}

	m_pListBoxEx->RemoveAllItems();

	for( int i = 0; i < pPacket->cCount; ++i ) {
		TGuildRecruitInfo *pRecruit = &pPacket->GuildRecruitList[i];
		CDnGuildWantedListItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnGuildWantedListItemDlg>();

		EtTextureHandle hTexture;
		TGuildView GuildView;
		GuildView.GuildUID.Set(1, 1);	// Just put any dummy value and call Set.
		GuildView.wGuildMark = pRecruit->wGuildMark;
		GuildView.wGuildMarkBG = pRecruit->wGuildMarkBG;
		GuildView.wGuildMarkBorder = pRecruit->wGuildMarkBorder;
		if( GetGuildTask().IsShowGuildMark( GuildView ) )
			hTexture = GetGuildTask().GetGuildMarkTexture( GuildView );

		bool bWaitRequest = false;
		for( int i = 0; i < (int)m_vecMyRecruitGuildUID.size(); ++i ) {
			if( m_vecMyRecruitGuildUID[i] == pRecruit->GuildUID ) {
				bWaitRequest = true;
				break;
			}
		}

#ifdef PRE_ADD_GUILD_EASYSYSTEM
		pItemDlg->SetInfo( pRecruit->GuildUID, hTexture, pRecruit->wszGuildName, pRecruit->wszGuildRecruitNotice, pRecruit->wGuildLevel, pRecruit->wGuildMemberPresentSize, pRecruit->wGuildMemberMaxSize, bWaitRequest, pRecruit->cPurposeCode, pRecruit->wszGuildHomePage, pRecruit->wszGuildMasterName );
#else
		pItemDlg->SetInfo( pRecruit->GuildUID, hTexture, pRecruit->wszGuildName, pRecruit->wszGuildRecruitNotice, pRecruit->wGuildLevel, pRecruit->wGuildMemberPresentSize, pRecruit->wGuildMemberMaxSize, bWaitRequest );
#endif
	}

	// Reset using data from packet.
	m_nPage = pPacket->uiPage;
	m_pStaticPage->SetIntToText( m_nPage+1 );
	m_pButtonOK->Enable( false );
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_pButtonHomepage->Enable( false );
#endif
}

void CDnGuildWantedListDlg::OnRecvGetGuildRecruitRequestCount( GuildRecruitSystem::SCGuildRecruitRequestCount *pPacket )
{
	m_nRequestCount = pPacket->cRequestCount;
	m_nMaxRequestCount = pPacket->cMaxRequestCount;

	WCHAR wszMsg[256] = {0,};
	swprintf_s( wszMsg, _countof(wszMsg), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3345 ), pPacket->cRequestCount, pPacket->cMaxRequestCount );
	m_pStaticCount->SetText( wszMsg );

	if( m_nRequestCount == m_nMaxRequestCount )
		m_pButtonOK->Enable( false );
}

void CDnGuildWantedListDlg::OnRecvGetGuildRecruitMyList( GuildRecruitSystem::SCMyGuildRecruitList *pPacket )
{
#ifdef PRE_ADD_GUILD_EASYSYSTEM
	m_vecMyRecruitGuildUID.clear();

	for( int i = 0; i < pPacket->cCount; ++i ) {
		TGuildRecruitInfo *pRecruit = &pPacket->MyGuildRecruitList[i];
		m_vecMyRecruitGuildUID.push_back( pRecruit->GuildUID );
	}
#else
	m_pListBoxEx->RemoveAllItems();
	m_vecMyRecruitGuildUID.clear();

	for( int i = 0; i < pPacket->cCount; ++i ) {
		TGuildRecruitInfo *pRecruit = &pPacket->MyGuildRecruitList[i];
		CDnGuildWantedListItemDlg *pItemDlg = m_pListBoxEx->AddItem<CDnGuildWantedListItemDlg>();
		m_vecMyRecruitGuildUID.push_back( pRecruit->GuildUID );

		EtTextureHandle hTexture;
		TGuildView GuildView;
		GuildView.GuildUID.Set(1, 1);
		GuildView.wGuildMark = pRecruit->wGuildMark;
		GuildView.wGuildMarkBG = pRecruit->wGuildMarkBG;
		GuildView.wGuildMarkBorder = pRecruit->wGuildMarkBorder;
		if( GetGuildTask().IsShowGuildMark( GuildView ) )
			hTexture = GetGuildTask().GetGuildMarkTexture( GuildView );

		pItemDlg->SetInfo( pRecruit->GuildUID, hTexture, pRecruit->wszGuildName, pRecruit->wszGuildRecruitNotice, pRecruit->wGuildLevel, pRecruit->wGuildMemberPresentSize, pRecruit->wGuildMemberMaxSize, true );
	}
#endif
}

void CDnGuildWantedListDlg::OnRecvGuildRecruitRequest( GuildRecruitSystem::SCGuildRecruitRequest *pPacket )
{
	// Remember the applied TGuildUID and update the info of the selected slot.
	if( !m_pListBoxEx ) return;
#ifdef PRE_ADD_GUILD_EASYSYSTEM
#else
	if( !m_pWantedTabButton->IsChecked() ) return;
#endif
	m_vecMyRecruitGuildUID.push_back( m_RequestGuildUID );

	int nSelectIndex = m_pListBoxEx->GetSelectedIndex();
	if( nSelectIndex != -1 )
	{
		CDnGuildWantedListItemDlg *pItem = m_pListBoxEx->GetItem<CDnGuildWantedListItemDlg>(nSelectIndex);
		if( pItem && pItem->GetGuildUID() == m_RequestGuildUID )
			pItem->SetOnWaitRequest();
	}
}