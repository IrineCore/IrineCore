#include "StdAfx.h"
#include "DnChatRoomDlg.h"
#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnInvenTabDlg.h"
#include "InputWrapper.h"
#include "ChatRoomSendPacket.h"
#include "DnLocalPlayerActor.h"
#include "DnRestraintTask.h"
#include "DnChatOption.h"
#include "DnInterfaceString.h"
#include "VillageSendPacket.h"
#include "DnNameLinkMng.h"
#include "DnChatRoomMemberListDlg.h"
#include "DnChatRoomConfigDlg.h"
#include "DnChatRoomPopupDlg.h"
#include "DnChatRoomTask.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnPrivateMarketDlg.h"
#include "DnPartyTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChatRoomDlg::CDnChatRoomDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pStaticTitle(NULL)
, m_nChatMode(CHAT_NORMAL)
, m_pButtonModeUp(NULL)
, m_pButtonModeDown(NULL)
, m_pStaticMode(NULL)
, m_pIMEEditBox(NULL)
, m_pButtonReport(NULL)
, m_pChatTextBox(NULL)
, m_pButtonDummy(NULL)
, m_pSelectMaster(NULL)
, m_pStaticMasterLevel(NULL)
, m_pStaticMasterJob(NULL)
, m_pStaticMasterName(NULL)
, m_pListBoxExPlayer(NULL)
, m_pChatRoomConfigDlg(NULL)
, m_pChatRoomPopupDlg(NULL)
, m_fPaperingRemainTime(0.0f)
, m_bLinked(false)
, m_bMasterIsMe( false )
, m_bPrivateModeChecker( false )
, m_nMasterSessionID( 0 )
, m_nRoomID( 0 )
{
	memset( m_pRenderPassChildDlg, 0, sizeof( m_pRenderPassChildDlg ) );
	memset( m_pTempChildDialog, 0, sizeof( m_pTempChildDialog ) );
}

CDnChatRoomDlg::~CDnChatRoomDlg(void)
{
	SAFE_DELETE( m_pChatRoomConfigDlg );
	SAFE_DELETE( m_pChatRoomPopupDlg );
	m_pListBoxExPlayer->RemoveAllItems();
}

void CDnChatRoomDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChatRoomDlg.ui" ).c_str(), bShow );
}

void CDnChatRoomDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>("ID_STATIC_TITLE");

	m_pButtonModeUp = GetControl<CEtUIButton>("ID_BUTTON_MODE_UP");
	m_pButtonModeDown = GetControl<CEtUIButton>("ID_BUTTON_MODE_DOWN");
	m_pStaticMode = GetControl<CEtUIStatic>("ID_STATIC_MODE");
	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>("ID_EDITBOX_CHAT");
	m_pButtonReport = GetControl<CEtUIButton>("ID_BUTTON_REPORT");

	m_pChatTextBox = GetControl<CEtUITextBox>("ID_TEXTBOX_CHAT");
	m_pButtonDummy = GetControl<CEtUIButton>("ID_BUTTON_DUMMY");

	m_pSelectMaster = GetControl<CEtUIStatic>("ID_STATIC_SELECT");
	m_pStaticMasterLevel = GetControl<CEtUIStatic>("ID_TEXT_LV");
	m_pStaticMasterJob = GetControl<CEtUIStatic>("ID_TEXT_JOB");
	m_pStaticMasterName = GetControl<CEtUIStatic>("ID_TEXT_NAME");
	m_pListBoxExPlayer = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_USERLIST");

	m_pButtonReport->Show( false );
	m_pSelectMaster->Show( false );

	m_pButtonDummy->Show( false );	// Seems unnecessary.

	m_pChatRoomConfigDlg = new CDnChatRoomConfigDlg( UI_TYPE_CHILD_MODAL, this );	// ChatRoomCreateDlg is inside the main menu, so it inevitably has to be kept separately.
	m_pChatRoomConfigDlg->Initialize( false );

	m_pChatRoomPopupDlg = new CDnChatRoomPopupDlg( UI_TYPE_CHILD, this );
	m_pChatRoomPopupDlg->Initialize( false );

	m_bLinked = false;


	m_mapCommandMode.insert( make_pair(std::wstring(L"/s "), CHAT_NORMAL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/S "), CHAT_NORMAL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/1 "), CHAT_NORMAL) );

	m_mapCommandMode.insert( make_pair(std::wstring(L"/p "), CHAT_PARTY) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/P "), CHAT_PARTY) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/2 "), CHAT_PARTY) );

	m_mapCommandMode.insert( make_pair(std::wstring(L"/g "), CHAT_GUILD) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/G "), CHAT_GUILD) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/3 "), CHAT_GUILD) );

	m_mapCommandMode.insert( make_pair(std::wstring(L"/w "), CHAT_PRIVATE) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/W "), CHAT_PRIVATE) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/4 "), CHAT_PRIVATE) );

#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/c "), CHAT_PRIVATE_CHANNEL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/C "), CHAT_PRIVATE_CHANNEL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/5 "), CHAT_PRIVATE_CHANNEL) );
#else // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/c "), CHAT_CHANNEL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/C "), CHAT_CHANNEL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/5 "), CHAT_CHANNEL) );

	m_mapCommandMode.insert( make_pair(std::wstring(L"/t "), CHAT_WORLD) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/T "), CHAT_WORLD) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/6 "), CHAT_WORLD) );
#endif // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/r "), CHAT_PRIVATE_REPLY) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/R "), CHAT_PRIVATE_REPLY) );

#if defined(_RDEBUG)
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u3134 "), CHAT_NORMAL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u3154 "), CHAT_PARTY) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u314E "), CHAT_GUILD) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u3148 "), CHAT_PRIVATE) );
#ifdef PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u314A "), CHAT_PRIVATE_CHANNEL) );
#else // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u314A "), CHAT_CHANNEL) );
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u3145 "), CHAT_WORLD) );
#endif // PRE_PRIVATECHAT_CHANNEL
	m_mapCommandMode.insert( make_pair(std::wstring(L"/\u3131 "), CHAT_PRIVATE_REPLY) );
#endif
}

void CDnChatRoomDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	GetPartyTask().ClearInviteInfoList(true);
#else
	GetPartyTask().ClearInviteInfoList();
#endif
	GetInterface().CloseGuildInviteReqDlg( true );

	if( bShow )
	{
		// To non-combat mode.
		CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
		if( pActor->IsBattleMode() )
		{
			if( !pActor->IsDie() && ( pActor->IsStay() || pActor->IsMove() ) )
			{
				if( pActor->IsMove() ) pActor->CmdStop( "Stand" );
				pActor->CmdToggleBattle( false );
			}
		}

		CDnLocalPlayerActor::LockInput( true );

		LinkChildDialog( true );

		GetControl<CEtUIButton>("ID_BUTTON_INVENTORY")->SetHotKey( _ToVK( g_UIWrappingKeyData[IW_UI_INVEN] ) );
		m_nChatMode = CHAT_NORMAL;
		SetChatModeText();
		m_pIMEEditBox->ClearText();
		RequestFocus( m_pIMEEditBox );
	}
	else
	{
		CDnLocalPlayerActor::LockInput( false );

		LinkChildDialog( false );

		m_pChatTextBox->ClearText();
		m_pListBoxExPlayer->RemoveAllItems();
		m_pSelectMaster->Show( false );
		m_bMasterIsMe = false;
		m_vecSessionID.clear();

		if( m_pChatRoomPopupDlg && m_pChatRoomPopupDlg->IsShow() )
			ShowChildDialog( m_pChatRoomPopupDlg, false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnChatRoomDlg::LinkChildDialog( bool bLink )
{
	if( m_bLinked == bLink )
		return;

	if( bLink )
	{
		// Calling Render forcibly complicates MsgProc handling, so we chose to temporarily register it as a Child.
		// When shown, just change AllignType and DlgCoord, attach as child, and detach again.
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
		{
			AddChildDialog( pInvenDlg );
			pInvenDlg->ShowRadioButton( false );
		}
		m_pTempChildDialog[0] = (CEtUIDialog *)pInvenDlg;
		m_pTempChildDialog[1] = (CEtUIDialog *)GetInterface().GetNameLinkToolTipDlg();
		m_pTempChildDialog[2] = GetInterface().GetAcceptRequestDialog();
		m_pTempChildDialog[3] = GetInterface().GetGuildInviteReqDlg();
		for( int i = 1; i < NUM_TEMP_CHILD; ++i )
			AddChildDialog( m_pTempChildDialog[i] );

		m_pRenderPassChildDlg[0] = m_pTempChildDialog[2];
		m_pRenderPassChildDlg[1] = m_pTempChildDialog[3];

		// Tried temporarily attaching as a child so messages are processed while excluding from rendering, but
		// ChatRoomDlg is Modal, so it cannot be handled normally (to prevent drawing twice... should make sure designs like this don't happen next time).
		// So,
		// InvenTabDlg hides the main menu and renders directly as a child,
		// Exclude AcceptRequestDialog and GuildInviteReqDlg when drawing.
		GetInterface().GetMainMenuDialog()->Show( false );

		// In the case of namelink tooltip
		// Rendered once as a child and once in UI_TYPE_BOTTOM_MSG as usual, so it gets rendered twice.
		// In tooltip rendering, could skip when it's a name-link tooltip and chat room is shown, but
		// Rendering twice overlapping is not a major issue here, so skip.

		// Finally 
		SetRenderPriority( m_pChatRoomConfigDlg, true );
		SetRenderPriority( m_pChatRoomPopupDlg, true );
	}
	else
	{
		CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
		{
			pInvenDlg->ShowRadioButton( true );
			if( pInvenDlg->IsShow() )
				ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BUTTON_INVENTORY"), 0 );
			DelChildDialog( pInvenDlg );
			m_pTempChildDialog[0] = NULL;
		}
		for( int i = 1; i < NUM_TEMP_CHILD; ++i ) {
			DelChildDialog( m_pTempChildDialog[i] );
			m_pTempChildDialog[i] = NULL;
		}

		GetInterface().GetMainMenuDialog()->Show( true );
	}
	m_bLinked = bLink;

	// Note: forcing a child link like this causes Process to be called twice so it processes a bit faster,
	// Trade request is fine because the task handles time calculation, but
	// Guild invite uses a dialog so the timeout is only half.
	// So force multiplying the time value by 2 where it is passed in.
}

void CDnChatRoomDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_EDITBOX_KEYUP )
	{
		SetNextChatHistory();
		return;
	}

	if( nCommand == EVENT_EDITBOX_KEYDOWN )
	{
		SetPrevChatHistory();
		return;
	}

	if( nCommand == EVENT_EDITBOX_SHIFT_KEYUP )
	{
		SetChatNextMode();
		return;
	}

	if( nCommand == EVENT_EDITBOX_SHIFT_KEYDOWN )
	{
		SetChatPrevMode();
		return;
	}

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_MODE_UP" ) )
		{
			SetChatNextMode();
			return;
		}

		if( IsCmdControl("ID_BUTTON_MODE_DOWN" ) )
		{
			SetChatPrevMode();
			return;
		}

		if( IsCmdControl("ID_BUTTON_OPTION") )
		{
			if( !GetTradeTask().GetTradePrivateMarket().GetPrivateMarketDlg()->IsShow() )
			{
				m_pChatRoomConfigDlg->SetInfo( GetChatRoomTask().GetChatRoomView(), GetChatRoomTask().GetChatRoomAllow(), GetChatRoomTask().GetPassword() );
				ShowChildDialog( m_pChatRoomConfigDlg, true );
			}
		}
		else if( IsCmdControl("ID_BUTTON_INVENTORY") )
		{
			if( !GetTradeTask().GetTradePrivateMarket().GetPrivateMarketDlg()->IsShow() )
			{
				CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
				if( pInvenDlg )
					GetInterface().GetMainMenuDialog()->ToggleShowDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			}
		}
		if( IsCmdControl("ID_BUTTON_CLOSE") )
		{
			// If the chat room host, the message changes.
			if( m_bMasterIsMe && m_pListBoxExPlayer->GetSize() > 0 )
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8137 ), MB_YESNO, MESSAGEBOX_EXITROOM, this, true, true );
			else
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8134 ), MB_YESNO, MESSAGEBOX_EXITROOM, this );
		}
	}
	else if( nCommand == EVENT_TEXTBOX_SELECTION )
	{
		if( IsCmdControl("ID_TEXTBOX_CHAT" ) )
		{
			CWord selectedWord = m_pChatTextBox->GetSelectedWordData();
			if (selectedWord.m_strWordWithTag.empty() == false)
				EtInterface::GetNameLinkMgr().TranslateText(std::wstring(), selectedWord.m_strWordWithTag.c_str(), this);
		}
	}
	else if( nCommand == EVENT_EDITBOX_STRING )
	{
		if( AddEditBoxString() )
		{
			focus::ReleaseControl();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChatRoomDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	std::list<float>::iterator iter = m_listRecentChatTime.begin();
	while( iter != m_listRecentChatTime.end() )
	{
		*iter -= fElapsedTime;
		if( *iter <= 0.0f )
		{
			iter = m_listRecentChatTime.erase( iter );
			continue;
		}
		++iter;
	}

	if( m_fPaperingRemainTime > 0.0f )
		m_fPaperingRemainTime -= fElapsedTime;

	if( !IsShow() )
		return;

	if( m_pChatRoomPopupDlg->IsShow() || m_pChatRoomConfigDlg->IsShow() )
	{
		bool bTradeDlg = false;
		if( GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting() )
			bTradeDlg = true;
		if( GetTradeTask().GetTradePrivateMarket().GetPrivateMarketDlg()->IsShow() )
			bTradeDlg = true;

		if( bTradeDlg )
		{
			if( m_pChatRoomConfigDlg->IsShow() )
				ShowChildDialog( m_pChatRoomConfigDlg, false );
			if( m_pChatRoomPopupDlg->IsShow() )
				ShowChildDialog( m_pChatRoomPopupDlg, false );
		}
	}

	// LockInput may be released while in Show state (e.g., trade ended). Force-lock it again.
	if( !CDnLocalPlayerActor::IsLockInput() )
		CDnLocalPlayerActor::LockInput( true );
}

void CDnChatRoomDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == MESSAGEBOX_EXITROOM )
		{
			if( IsCmdControl( "ID_YES" ) )
			{
				SendLeaveChatRoom();
			}
		}
	}
}

bool CDnChatRoomDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
	{
		return false;
	}

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			if( wParam == VK_TAB )
			{
				if( GetChatMode() == CHAT_PRIVATE )
				{
					ChangePrivateName();
				}
				RequestFocus(m_pIMEEditBox);
				return true;
			}
		}
		break;
	}

	if( !drag::IsValid() )
	{
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );

		float fMouseX, fMouseY;
		fMouseX = MousePoint.x / GetScreenWidth();
		fMouseY = MousePoint.y / GetScreenHeight();

		switch( uMsg )
		{
		case WM_RBUTTONDOWN:
			{
				std::wstring wszName;

				bool bIsInsideMasterSelect = false;
				SUICoord uiCoordsBase;
				float fMouseXinDlg, fMouseYinDlg;
				m_pSelectMaster->GetUICoord(uiCoordsBase);
				PointToFloat( MousePoint, fMouseXinDlg, fMouseYinDlg );
				if( uiCoordsBase.IsInside( fMouseXinDlg, fMouseYinDlg ) ) {
					bIsInsideMasterSelect = true;
					wszName = m_pStaticMasterName->GetText();
				}
				if( bIsInsideMasterSelect ) {
					m_pSelectMaster->Show( true );
					SListBoxItem *pSelectedItem = m_pListBoxExPlayer->GetSelectedItem();
					if( pSelectedItem ) {
						pSelectedItem->bSelected = false;
						m_pListBoxExPlayer->DeselectItem();
					}
				}
				else {
					m_pSelectMaster->Show( false );
				}

				bool bIsInsideListBoxItem = false;
				for( int i = m_pListBoxExPlayer->GetScrollBar()->GetTrackPos(); i < m_pListBoxExPlayer->GetSize(); ++i ) {
					CDnChatRoomMemberListDlg *pDlg = m_pListBoxExPlayer->GetItem<CDnChatRoomMemberListDlg>(i);
					if( !pDlg ) continue;
					SUICoord uiCoord;
					pDlg->GetDlgCoord( uiCoord );
					if( uiCoord.IsInside( fMouseX, fMouseY ) ) {
						bIsInsideListBoxItem = true;
						wszName = pDlg->GetName();
						break;
					}
				}

				if( GetTradeTask().GetTradePrivateMarket().IsRequesting() || GetTradeTask().GetTradePrivateMarket().IsAccepting() )
					break;
				if( GetTradeTask().GetTradePrivateMarket().GetPrivateMarketDlg()->IsShow() )
					break;
				if( m_pChatRoomConfigDlg->IsShow() )
					break;

				if( bIsInsideMasterSelect || bIsInsideListBoxItem ) {
					ShowChildDialog( m_pChatRoomPopupDlg, false );
					m_pChatRoomPopupDlg->SetPosition( fMouseX, fMouseY );
					static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH), fYBGap(4.0f/DEFAULT_UI_SCREEN_HEIGHT);
					SUICoord sDlgCoord;
					m_pChatRoomPopupDlg->GetDlgCoord( sDlgCoord );
					if( (sDlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
						sDlgCoord.fX -= (sDlgCoord.Right()+fXRGap - GetScreenWidthRatio());
					m_pChatRoomPopupDlg->SetDlgCoord( sDlgCoord );
					m_pChatRoomPopupDlg->SetInfo( m_bMasterIsMe, wszName.c_str() );
					ShowChildDialog( m_pChatRoomPopupDlg, true );
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
				if( IsMouseInDlg() ) {
					if( m_pChatRoomPopupDlg->IsShow() ) {
						SUICoord uiCoord;
						m_pChatRoomPopupDlg->GetDlgCoord( uiCoord );
						if( !uiCoord.IsInside( fMouseX, fMouseY ) ) {
							ShowChildDialog( m_pChatRoomPopupDlg, false );
							return true;
						}
					}
				}

				SUICoord uiCoordsBase;
				float fMouseXinDlg, fMouseYinDlg;
				m_pSelectMaster->GetUICoord(uiCoordsBase);
				PointToFloat( MousePoint, fMouseXinDlg, fMouseYinDlg );
				if( uiCoordsBase.IsInside( fMouseXinDlg, fMouseYinDlg ) ) {
					m_pSelectMaster->Show( true );
					SListBoxItem *pSelectedItem = m_pListBoxExPlayer->GetSelectedItem();
					if( pSelectedItem ) {
						pSelectedItem->bSelected = false;
						m_pListBoxExPlayer->DeselectItem();
					}
				}
				else {
					m_pSelectMaster->Show( false );
				}
			}
			break;
		}
	}

	bool bRet;
	bRet = CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );

	switch( uMsg )
	{
	case WM_KEYDOWN:
		{
			if( wParam == VK_RETURN )
			{
				if( !m_pIMEEditBox->IsFocus() )
					RequestFocus( m_pIMEEditBox );
			}
			else if( wParam == VK_TAB )
			{
				if( GetChatMode() == CHAT_PRIVATE )
				{
					ChangePrivateName();
				}

				RequestFocus(m_pIMEEditBox);
				return true;
			}
		}
		break;
	}

	return bRet;
}

void CDnChatRoomDlg::Render( float fElapsedTime )
{
	if( !IsAllowRender() ) return;

	// #41573 When creating a chat room and opening inventory right after closing the inventory dialog,
	// When the texture optimization routine of the parent main menu dialog runs, it also deletes the inventory dialog's textures.
	// So force restoring it like this and rendering again.
	if( m_pTempChildDialog[0] && m_pTempChildDialog[0]->IsShow() )
		m_pTempChildDialog[0]->LoadDialogTexture();

	CheckCommandMode();

	std::list<CEtUIDialog*>::iterator iterTemp[NUM_TEMP_CHILD_RENDER_PASS];
	int nIndexTemp[NUM_TEMP_CHILD_RENDER_PASS];
	
	for(int n=0; n<NUM_TEMP_CHILD_RENDER_PASS; n++)
		nIndexTemp[n] = 0;

	if( m_bLinked )
	{
		// Since inventory is forcibly attached as a child window, it renders by default in MainMenu and renders here again,
		// To prevent this, temporarily set the linked child dialog to NULL.
		// If the order were always fixed, we would just put it at a specific index, but
		// Originally child dialogs change list order whenever Shown by default, so handled as below.
		std::list<CEtUIDialog*>::iterator iter = m_listChildDialog.begin();
		int nIndex = 0;
		if( iter != m_listChildDialog.end() )
		{
			for( ; iter != m_listChildDialog.end(); ++iter )
			{
				for( int j = 0; j < NUM_TEMP_CHILD_RENDER_PASS; ++j )
				{
					if( *iter == m_pRenderPassChildDlg[j] )
					{
						iterTemp[nIndex] = iter;
						nIndexTemp[nIndex] = j;
						++nIndex;
						break;
					}
				}
			}
		}

		for( int i = 0; i < NUM_TEMP_CHILD_RENDER_PASS; ++i )
		{
			*(iterTemp[i]) = NULL;
		}
	}

	CEtUIDialog::Render( fElapsedTime );

	if( m_bLinked )
	{
		for( int i = 0; i < NUM_TEMP_CHILD_RENDER_PASS; ++i )
		{
			*(iterTemp[i]) = m_pRenderPassChildDlg[nIndexTemp[i]];
		}
	}
}

void CDnChatRoomDlg::SetRoomInfo( int nRoomID, UINT nSessionID, LPCWSTR pwszRoomName, bool bMasterIsMe )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor ) return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pPlayer ) return;

	m_nRoomID = nRoomID;
	m_nMasterSessionID = nSessionID;

	m_pStaticTitle->SetText( pwszRoomName );
	m_pStaticMasterLevel->SetIntToText( hActor->GetLevel() );
	m_pStaticMasterJob->SetText( pPlayer->GetJobName() );
	m_pStaticMasterName->SetText( hActor->GetName() );

	GetControl<CEtUIButton>("ID_BUTTON_OPTION")->Show( bMasterIsMe );

	m_bMasterIsMe = bMasterIsMe;
}

int CDnChatRoomDlg::GetRoomID()
{
	if( !IsShow() ) return 0;
	return m_nRoomID;
}

bool CDnChatRoomDlg::IsMyChatRoomMember( UINT nSessionID )
{
	if( m_nMasterSessionID == nSessionID ) return true;
	if( m_vecSessionID.empty() ) return false;
	for( int i = 0; i < (int)m_vecSessionID.size(); ++i )
	{
		if( m_vecSessionID[i] == nSessionID )
			return true;
	}
	return false;
}

void CDnChatRoomDlg::AddUser( UINT nSessionID, bool bNotify )
{
	DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
	if( !hActor ) return;

	AddUser( hActor, bNotify );
}

void CDnChatRoomDlg::AddUser( DnActorHandle hActor, bool bNotify )
{
	if( !hActor ) return;

	CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( !pPlayer ) return;

	int nSessionID = hActor->GetUniqueID();
	int nIndex = -1;
	for( int i = 0; i < (int)m_vecSessionID.size(); ++i )
	{
		if( m_vecSessionID[i] == nSessionID )
		{
			nIndex = i;
			break;
		}
	}
	if( nIndex != -1 )
	{
		// Why is it trying to add an already-added user again? Did something get tangled up?
		return;
	}

	CDnChatRoomMemberListDlg *pItemDlg = m_pListBoxExPlayer->AddItem<CDnChatRoomMemberListDlg>();
	pItemDlg->SetInfo( pPlayer->GetName(), pPlayer->GetJobName(), pPlayer->GetLevel() );

	if( bNotify )
	{
		WCHAR wszTemp[256];
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8131 ), pPlayer->GetName() );
		AddChat( L"", wszTemp, CHAT_SYSTEM );
	}

	m_vecSessionID.push_back( nSessionID );

	if( m_pChatRoomPopupDlg && m_pChatRoomPopupDlg->IsShow() )
		ShowChildDialog( m_pChatRoomPopupDlg, false );
}

void CDnChatRoomDlg::DelUser( UINT nSessionID, BYTE cLeaveReason )
{
	int nIndex = -1;
	for( int i = 0; i < (int)m_vecSessionID.size(); ++i )
	{
		if( m_vecSessionID[i] == nSessionID )
		{
			nIndex = i;
			break;
		}
	}

	if( nIndex != -1 )
	{
		m_pListBoxExPlayer->RemoveItem( nIndex );
		m_vecSessionID.erase( m_vecSessionID.begin() + nIndex );

		DnActorHandle hActor = CDnActor::FindActorFromUniqueID( nSessionID );
		if( hActor )
		{
			CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor *>(hActor.GetPointer());
			if( pPlayer )
			{
				int nMsgIndex = 0;
				switch( cLeaveReason )
				{
				case CHATROOMLEAVE_LEAVE:			nMsgIndex = 8132; break;
				case CHATROOMLEAVE_KICKED:			nMsgIndex = 121061; break;
				case CHATROOMLEAVE_FOREVERKICKED:	nMsgIndex = 121062; break;
				}
				if( nMsgIndex )
				{
					WCHAR wszTemp[256];
					swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nMsgIndex ), pPlayer->GetName() );
					AddChat( L"", wszTemp, CHAT_SYSTEM );
				}
			}
		}

		if( m_pChatRoomPopupDlg && m_pChatRoomPopupDlg->IsShow() )
			ShowChildDialog( m_pChatRoomPopupDlg, false );
	}
}

void CDnChatRoomDlg::ResizeIMECCtl( LPCWSTR wszHeader )
{
	DWORD dwTextColor(chatcolor::NORMAL);

	switch( GetChatMode() )
	{
	case CHAT_NORMAL:	dwTextColor = chatcolor::NORMAL;	break;
	case CHAT_PRIVATE:	dwTextColor = chatcolor::PRIVATE;	break;
	case CHAT_PARTY:	dwTextColor = chatcolor::PARTY;		break;
	case CHAT_GUILD:	dwTextColor = chatcolor::GUILD;		break;
	case CHAT_CHANNEL:	dwTextColor = chatcolor::CHANNEL;	break;
	case CHAT_WORLD:	dwTextColor = chatcolor::WORLD;		break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:	dwTextColor = chatcolor::PRIVATECHANNEL;	break;
#endif // PRE_PRIVATECHAT_CHANNEL
	default:
		ASSERT( 0&&"CDnChatRoomDlg::ResizeIMECCtl" );
		break;
	}

	m_pIMEEditBox->SetTextColor( dwTextColor );
	m_pIMEEditBox->SetCompTextColor( dwTextColor );
	m_pStaticMode->SetTextColor( dwTextColor );
	m_pStaticMode->SetText( wszHeader );

	SUICoord sTextCoord;
	CalcTextRect( wszHeader, m_pStaticMode->GetElement(0), sTextCoord );
	sTextCoord.fWidth += 0.001f;

	SUICoord sCtlCoord;
	m_pStaticMode->GetUICoord( sCtlCoord );
	float fTemp = sTextCoord.fWidth - sCtlCoord.fWidth;
	sCtlCoord.fWidth = sTextCoord.fWidth;
	m_pStaticMode->SetUICoord( sCtlCoord );

	m_pIMEEditBox->GetUICoord( sCtlCoord );
	sCtlCoord.fX += fTemp;
	sCtlCoord.fWidth -= fTemp;
	m_pIMEEditBox->SetUICoord( sCtlCoord );
}

void CDnChatRoomDlg::SetChatPrevMode()
{
	int nCurChatMode = GetChatMode();
	while(true)
	{
		nCurChatMode >>= 1;

		if( nCurChatMode == 0 )
			nCurChatMode = CHAT_WORLD;

		if( SetChatMode( nCurChatMode ) )
		{
			RequestFocus(m_pIMEEditBox);
			break;
		}
	}
}

void CDnChatRoomDlg::SetChatNextMode()
{
	int nCurChatMode = GetChatMode();
	while(true)
	{
		nCurChatMode <<= 1;

		if( nCurChatMode >= CHAT_MODE_MAX )
			nCurChatMode = CHAT_NORMAL;

		if( SetChatMode( nCurChatMode ) )
		{
			RequestFocus(m_pIMEEditBox);
			break;
		}
	}
}

void CDnChatRoomDlg::SetChatModeText()
{
	if( !m_pStaticMode )
		return;

	wchar_t wszMsg[256]={0};

	switch( m_nChatMode )
	{
	case CHAT_NORMAL:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 501 ) );
		break;
	case CHAT_PARTY:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 632 ) );
		break;
	case CHAT_GUILD:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5007 ) );
		break;
	case CHAT_PRIVATE:
		{
			swprintf_s( wszMsg, 256, L"[%s][%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 648), m_strPrivateUserName.c_str() );

			if( !m_strPrivateUserName.empty() )
			{
				AddPrivateName( m_strPrivateUserName );
			}
		}
		break;
	case CHAT_CHANNEL:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 605 ) );
		break;
	case CHAT_WORLD:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 606 ));
		break;
#ifdef PRE_PRIVATECHAT_CHANNEL
	case CHAT_PRIVATE_CHANNEL:
		swprintf_s( wszMsg, 256, L"[%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1202 ));
		break;
#endif // PRE_PRIVATECHAT_CHANNEL
	}
	ResizeIMECCtl(wszMsg);
}

bool CDnChatRoomDlg::SetChatMode( int nChatMode )
{
	if( nChatMode == CHAT_PARTY )
	{
		return false;
	}
	else if( nChatMode == CHAT_GUILD )
	{
		// If not joined a guild,
		if( CDnActor::s_hLocalActor ) {
			CDnPlayerActor* pPlayer = dynamic_cast<CDnPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
			if( !pPlayer->IsJoinGuild() ) {
				return false;
			}
		}
	}
	else if( nChatMode == CHAT_PRIVATE )
	{
		// If there is no whisper target
		if( GetPrivateName() == L"" )
			return false;
	}
	else if( nChatMode == CHAT_CHANNEL )
	{
		// Disable feature for CBT first.
		return false;
	}
	else if( nChatMode == CHAT_WORLD )
	{
		return false;
	}

	m_nChatMode = nChatMode;
	SetChatModeText();
	return true;
}

bool CDnChatRoomDlg::AddEditBoxString()
{
	std::wstring strChat = m_pIMEEditBox->GetText();
	strChat.reserve( CHATLENMAX );

	std::wstring chatTemp;
	chatTemp = boost::algorithm::trim_copy(strChat);
	if (chatTemp.empty())
		return true;

	if( CheckCommandMode( strChat.c_str() ) )
		return false;

	// Must store the command before processing so it can be re-entered using history.
	AddChatHistotry( strChat.c_str() );

	// Check chat restriction here.
	if( GetRestraintTask().CheckRestraint( _RESTRAINTTYPE_CHAT, true ) )
		return true;

	// Gestures also have chat commands.
	// Unlike other commands, gestures do not cause focus loss since the user is just chatting.
	//GetGestureTask().UseGestureByChat( strChat.c_str() );

	// Cannot use cheats while chat room is open; character is locked.
	if( strChat[0] == L'/' )
	{
		m_pIMEEditBox->ClearText();
		return false;
	}

	SendChat( m_nChatMode, strChat, m_strPrivateUserName );

	m_pIMEEditBox->ClearText();

	return false;
}

void CDnChatRoomDlg::ChangePrivateName()
{
	if( !m_listPrivateName.empty() )
	{
		m_strPrivateUserName = m_listPrivateName.front();
		m_listPrivateName.pop_front();
		m_listPrivateName.push_back( m_strPrivateUserName );

		wchar_t wszMsg[256]={0};
		swprintf_s( wszMsg, 256, L"[%s][%s]:", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 648), m_strPrivateUserName.c_str() );
		ResizeIMECCtl(wszMsg);
	}
}

void CDnChatRoomDlg::AddPrivateName( const std::wstring strPrivateName )
{
	PRIVATENAME_LIST_ITER iter = m_listPrivateName.begin();
	for( ; iter != m_listPrivateName.end(); )
	{
		if( (*iter) == strPrivateName )
		{
			iter = m_listPrivateName.erase(iter);
		}
		else ++iter;
	}

	m_listPrivateName.push_back( strPrivateName );
}

void CDnChatRoomDlg::SetPrivateName( const std::wstring &strPrivateName )
{
	m_strPrivateUserName = strPrivateName;

	SetChatMode( CHAT_PRIVATE );
	m_pIMEEditBox->ClearText();
	RequestFocus( m_pIMEEditBox );
}

bool CDnChatRoomDlg::GetLastPrivateName( std::wstring &strLastPrivateName )
{
	if( !m_strLastPrivateUserName.empty() )
	{
		strLastPrivateName = m_strLastPrivateUserName;
		return true;
	}

	return false;
}

void CDnChatRoomDlg::SetLastPrivateName( LPCWSTR wszLastPrivateName )
{
	ASSERT( wszLastPrivateName&&"CDnChatTabDlg::SetLastPrivateName" );
	m_strLastPrivateUserName = wszLastPrivateName;
}

void CDnChatRoomDlg::AddChatHistotry( LPCWSTR wszChat )
{
	if( m_listChatHistory.size() > 20 )
	{
		m_listChatHistory.pop_back();
		m_listChatHistory.push_front( wszChat );
	}
	else
	{
		m_listChatHistory.push_front( wszChat );
	}
}

void CDnChatRoomDlg::SetPrevChatHistory()
{
	if( !m_listChatHistory.empty() )
	{
		std::wstring strChat = m_listChatHistory.back();
		m_listChatHistory.pop_back();
		m_listChatHistory.push_front( strChat );

		m_pIMEEditBox->SetText( strChat.c_str() );
	}
}

void CDnChatRoomDlg::SetNextChatHistory()
{
	if( !m_listChatHistory.empty() )
	{
		std::wstring strChat = m_listChatHistory.front();
		m_listChatHistory.pop_front();
		m_listChatHistory.push_back( strChat );

		m_pIMEEditBox->SetText( strChat.c_str() );
	}

	return;
}

void CDnChatRoomDlg::CheckCommandMode()
{
	std::wstring strCmd;
	COMMANDMODE_MAP_ITER iter;
	m_bPrivateModeChecker = false;

	strCmd = m_pIMEEditBox->GetText();

	std::wstring::size_type idx = strCmd.find_first_of(L" ");
	if (idx != std::wstring::npos)
	{
		std::wstring checkStr = strCmd.substr(0, idx + 1);

		iter = m_mapCommandMode.find(checkStr);
		if( iter != m_mapCommandMode.end() )
		{
			int chatType = iter->second;
			if( chatType & CHAT_PRIVATE_REPLY )
			{
				std::wstring strLastName;
				if( GetLastPrivateName( strLastName ) )
				{
					SetPrivateName( strLastName );
				}
				return;
			}

			if( chatType == CHAT_PRIVATE )
			{
				m_bPrivateModeChecker = true;
			}
			else
			{
				SetChatMode( chatType );
				m_pIMEEditBox->ClearText();
				RequestFocus(m_pIMEEditBox);
			}
		}

		if (m_bPrivateModeChecker)
		{
			// Note : Do not change mode until opponent name is entered.
			//
			std::wstring::size_type begIdx, endIdx;

			begIdx = strCmd.find_first_of(L" ");
			begIdx = (begIdx != std::wstring::npos) ? ++begIdx : std::wstring::npos;
			endIdx = strCmd.find_first_of(L" ", begIdx);

			if( endIdx != std::wstring::npos )
			{
				std::wstring strPrivateUserName = strCmd.substr(begIdx, endIdx-begIdx);
				EtInterface::GetNameLinkMgr().TranslateText(m_strPrivateUserName, strPrivateUserName.c_str());
				if ((int)m_strPrivateUserName.size() > CGlobalInfo::GetInstance().m_nClientCharNameLenMax)
					m_strPrivateUserName.resize(CGlobalInfo::GetInstance().m_nClientCharNameLenMax);

				SetChatMode( CHAT_PRIVATE );
				m_pIMEEditBox->ClearText();
				RequestFocus(m_pIMEEditBox);

				m_bPrivateModeChecker = false;
			}
		}
	}
}

bool CDnChatRoomDlg::CheckCommandMode( LPCWSTR wszMsg )
{
	std::wstring strCmd(wszMsg);
	if( strCmd.size() < 2 ) return false;
	if( (strCmd.size() > 2) && (strCmd[2] != L' ') ) return false;

	strCmd = strCmd.substr(0,2);
	strCmd += L" ";
	COMMANDMODE_MAP_ITER iter = m_mapCommandMode.find( strCmd );

	if( iter != m_mapCommandMode.end() )
	{
		if( iter->second&CHAT_PRIVATE_REPLY )
		{
			std::wstring strLastName;
			if( GetLastPrivateName( strLastName ) )
			{
				SetPrivateName( strLastName );
			}
			return true;
		}

		if( iter->second == CHAT_PRIVATE )
		{
			strCmd = wszMsg;
			std::wstring::size_type begIdx = strCmd.find_first_not_of(L" ", 2);
			if( begIdx != std::wstring::npos )
			{
				std::wstring strPrivateUserName = strCmd.substr(begIdx);
				EtInterface::GetNameLinkMgr().TranslateText(m_strPrivateUserName, strPrivateUserName.c_str());
				if( (int)m_strPrivateUserName.size() > CGlobalInfo::GetInstance().m_nClientCharNameLenMax )
				{
					m_strPrivateUserName.resize( CGlobalInfo::GetInstance().m_nClientCharNameLenMax );
				}
			}
		}

		SetChatMode( iter->second );
		m_pIMEEditBox->ClearText();
		RequestFocus(m_pIMEEditBox);

		return true;
	}

	return false;
}

void CDnChatRoomDlg::SendChat( int chatMode, std::wstring& chatMsg, const std::wstring& userName )
{
	DN_INTERFACE::UTIL::CheckChat( chatMsg, '*' );

	switch( chatMode )
	{
	case CHAT_NORMAL:
		{
			SendChatRoomMsg( chatMsg.c_str() );
		}
		break;
	case CHAT_PRIVATE:
		{
			if( CheckPapering() )
			{
				wchar_t wszMsg[256]={0};
				int nTime = (int)m_fPaperingRemainTime;
				swprintf_s( wszMsg, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 672 ), nTime );
				m_pChatTextBox->AddText( wszMsg, textcolor::RED );
				return;
			}

			if( userName.empty() )
				break;

			CDnInvenTabDlg *pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
			if( pInvenDlg )
			{
				// Register in my speech bubble only if wszFromCharName matches the trading partner's name.
				if( pInvenDlg->IsYourName( userName.c_str() ) )
					pInvenDlg->AddPrivateMarketChatMe( chatMsg.c_str() );
			}

			wchar_t wszMsg[1024]={0};
			swprintf_s( wszMsg, 1024, L"[%s]<<< : %s", userName.c_str(), chatMsg.c_str() );
			AddChat( userName.c_str(), wszMsg, CHAT_PRIVATE );

			SendChatPrivateMsg( userName.c_str(), chatMsg.c_str() );
		}
		break;
	case CHAT_GUILD:
		{
			SendChatMsg( CHATTYPE_GUILD, chatMsg.c_str() );
		}
		break;
	}
}

void CDnChatRoomDlg::AddChatMessage( eChatType eType, LPCWSTR wszFromCharName, LPCWSTR wszChatMsg )
{
	wchar_t wszMsg[1024]={0};

	std::wstring fromCharNameString;
	if (wcslen( wszFromCharName ) != 0)
	{
		CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
		if (pMgr)
			pMgr->MakeNameLinkString_UserName(fromCharNameString, wszFromCharName);
	}

	int nType = CHAT_NORMAL;
	DWORD dwBgColor = 0;
	CDnInvenTabDlg *pInvenDlg = NULL;
	switch( eType )
	{
	case CHATTYPE_NORMAL:
	case CHATTYPE_GM:
		// When entering a chat room, normal chat is not heard.
		return;
	case CHATTYPE_PRIVATE:
	case CHATTYPE_PRIVATE_GM:
		nType = CHAT_PRIVATE;
		pInvenDlg = (CDnInvenTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::INVENTORY_DIALOG );
		if( pInvenDlg )
		{
			// Register in my speech bubble only if wszFromCharName matches the trading partner's name.
			if( pInvenDlg->IsYourName( wszFromCharName ) )
				pInvenDlg->AddPrivateMarketChatYou( wszChatMsg );
		}

		// Save this regardless of the IsPrivate option.
		// This is required to allow replying with Shift+r.
		SetLastPrivateName( wszFromCharName );

		swprintf_s( wszMsg, 1024, L"[%s]>>> : %s", fromCharNameString.c_str(), wszChatMsg );
		AddPrivateName( wszFromCharName );

		if( eType == CHATTYPE_PRIVATE_GM ) dwBgColor = D3DCOLOR_ARGB(0x80,0x22,0x8B,0x22);
		break;
	case CHATTYPE_CHATROOM:
		// Need to check block list even after entering the chat room?
		//if (CDnIsolateTask::IsActive() && GetIsolateTask().IsBlackList(fromCharNameString.c_str()))
		//	return;
		nType = CHAT_NORMAL;
		if( fromCharNameString.empty() )
			swprintf_s( wszMsg, 1024, L"%s", wszChatMsg );
		else 
			swprintf_s( wszMsg, 1024, L"[%s] : %s", fromCharNameString.c_str(), wszChatMsg );
		break;
	case CHATTYPE_GUILD:
		nType = CHAT_GUILD;		
		swprintf_s( wszMsg, 1024, L"[%s][%s] : %s", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5007), fromCharNameString.c_str(), wszChatMsg );
		break;
	case CHATTYPE_SYSTEM:
		nType = CHAT_SYSTEM;
		swprintf_s( wszMsg, 1024, L"%s", wszChatMsg );
		break;
	case CHATTYPE_RAIDNOTICE:
		{
			nType = CHAT_RAIDNOTICE;
			swprintf_s( wszMsg, 1024, L"[%s]: %s", fromCharNameString.c_str(), wszChatMsg );

			int soundIndex = CDnPartyTask::GetInstance().GetPartySoundIndex(CDnPartyTask::REQUESTREADY);
			CEtSoundEngine::GetInstance().PlaySound("2D", soundIndex);
		}
		break;
	}

	AddChat( wszFromCharName, wszMsg, nType, dwBgColor );
}

void CDnChatRoomDlg::AppendChatEditBox(LPCWSTR wszString, bool bFocus)
{
	if (m_pIMEEditBox->GetFullTextLength() + lstrlenW(wszString) >= CHATLENMAX)
		return;

	if (bFocus == false)
		focus::ReleaseControl();

	m_pIMEEditBox->AddText(wszString, bFocus);
}

bool CDnChatRoomDlg::OnParseTextItemInfo(const std::wstring& argString)
{
	CDnNameLinkMng* pMgr = GetInterface().GetNameLinkMng();
	if (pMgr)
	{
		CDnItem* pLinkItem = pMgr->MakeItem(argString);
		if (pLinkItem)
		{
			CDnTooltipDlg* pToolTipDlg = GetInterface().GetNameLinkToolTipDlg();
			if (pToolTipDlg)
			{
				ITEM_SLOT_TYPE slotType = ST_ITEM_NONE;
				if (pLinkItem->GetType() == MIInventoryItem::Item)
					slotType = ST_INVENTORY;
				else if (pLinkItem->GetType() == MIInventoryItem::Skill)
					slotType = ST_SKILL;
				pToolTipDlg->ShowTooltip(pLinkItem, pLinkItem->GetType(), slotType, GetScreenWidthRatio() * 0.5f, GetScreenHeightRatio() * 0.83f, true);
			}
		}
	}

	return true;
}

bool CDnChatRoomDlg::OnParseTextUserName(const std::wstring& name)
{
	SetPrivateName(name);
	return true;
}

void CDnChatRoomDlg::AddChat( LPCWSTR wszName, LPCWSTR szMessage, int nType, DWORD dwBackgroundColor )
{
	DWORD dwTextColor(textcolor::WHITE);

	switch( nType )
	{
	case CHAT_NORMAL:	dwTextColor = chatcolor::NORMAL;	break;
	case CHAT_PARTY:	dwTextColor = chatcolor::PARTY;		break;
	case CHAT_GUILD:	dwTextColor = chatcolor::GUILD;		break;
	case CHAT_PRIVATE:	dwTextColor = chatcolor::PRIVATE;	break;
	case CHAT_SYSTEM:	dwTextColor = textcolor::GOLD;		break;
	case CHAT_RAIDNOTICE: dwTextColor = chatcolor::RAIDNOTICE;		break;
	}

	if( wszName != NULL && wszName[0] != '\0' )
	{
		SLineData sLineData;
		sLineData.m_strData = wszName;
		m_pChatTextBox->SetLineData( sLineData );
	}

	bool bAutoScroll = false;

	if( m_pChatTextBox->IsLastPage() || m_pChatTextBox->IsEmpty() )
		bAutoScroll = true;

	// Apply auto-scroll for own messages even if scrolled up.
	if( !bAutoScroll && wszName ) {
		bool bMyChat = false;
		if( CDnActor::s_hLocalActor ) {
			if( __wcsicmp_l( wszName, CDnActor::s_hLocalActor->GetName() ) == 0 )
				bMyChat = true;
		}
		if( bMyChat )
			bAutoScroll = true;
	}
	m_pChatTextBox->SetAutoScroll( bAutoScroll );

	m_pChatTextBox->AddText( szMessage, dwTextColor, UITEXT_NONE, dwBackgroundColor );
}

bool CDnChatRoomDlg::CheckPapering()
{
	// If currently restricted due to chat spamming,
	if( m_fPaperingRemainTime > 0.0f )
		return true;

	// If the limit count is reached when adding,
	if( (int)m_listRecentChatTime.size() + 1 >= CHAT_PAPERING_CHECKCOUNT )
		m_fPaperingRemainTime = (float)CHAT_PAPERING_RESTRICTIONTIME;

	m_listRecentChatTime.push_back((float)CHAT_PAPERING_CHECKTIME);
	return false;
}

bool CDnChatRoomDlg::IsEmptyRoom() const
{
	return (m_pListBoxExPlayer->GetSize() <= 0);
}