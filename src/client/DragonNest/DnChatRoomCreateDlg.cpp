#include "StdAfx.h"
#include "DnChatRoomCreateDlg.h"
#include "DnInterface.h"
#include "DnPlayerActor.h"
#include "DnInterfaceString.h"
#include "DnMainDlg.h"
#include "DnChatRoomTask.h"
#include "DnPartyTask.h"
#include "DnGameTask.h"
#include "TaskManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChatRoomCreateDlg::CDnChatRoomCreateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pEditBoxName(NULL)
, m_pButtonOK(NULL)
, m_bShowedJustNow(false)
, m_pComboRoomType(NULL)
, m_pComboCondition(NULL)
, m_pEditBoxPR0(NULL)
, m_pEditBoxPR1(NULL)
, m_pEditBoxPR2(NULL)
, m_pCheckBoxSecret(NULL)
, m_pEditBoxPassword(NULL)
{
}

CDnChatRoomCreateDlg::~CDnChatRoomCreateDlg(void)
{
}

void CDnChatRoomCreateDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChatCreateDlg.ui" ).c_str(), bShow );
}

void CDnChatRoomCreateDlg::InitialUpdate()
{
	m_pEditBoxName = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_CHATNAME");
	m_pButtonOK = GetControl<CEtUIButton>("ID_BUTTON_CREATE");

	m_pComboRoomType = GetControl<CEtUIComboBox>("ID_COMBOBOX_COUNT");
	m_pComboCondition = GetControl<CEtUIComboBox>("ID_COMBOBOX_TSTAGE");

	m_pEditBoxPR0 = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_PR0");
	m_pEditBoxPR1 = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_PR1");
	m_pEditBoxPR2 = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_PR2");

	m_pCheckBoxSecret = GetControl<CEtUICheckBox>("ID_CHECKBOX_SECRET");
	m_pEditBoxPassword = GetControl<CEtUIEditBox>("ID_EDITBOX_PASSWORD");

	m_pComboRoomType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8120 ), NULL, CHATROOMTYPE_NORMAL );
	m_pComboRoomType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8121 ), NULL, CHATROOMTYPE_TRADE );
	m_pComboRoomType->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8122 ), NULL, CHATROOMTYPE_PR );

	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8123 ), NULL, CHATROOMALLOW_ALL );
	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8124 ), NULL, CHATROOMALLOW_FRIEND );
	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8125 ), NULL, CHATROOMALLOW_GUILD );
	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8126 ), NULL, CHATROOMALLOW_FRIENDANDGUILD );

	m_pEditBoxPassword->Enable( false );
}

void CDnChatRoomCreateDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_pEditBoxName->ClearText();
		RequestFocus( m_pEditBoxName );
		m_pButtonOK->Enable( false );
	}
	else
	{
		m_pCheckBoxSecret->SetChecked( false );
		m_pComboRoomType->SetSelectedByIndex( 0 );
		m_pComboCondition->SetSelectedByIndex( 0 );
	}

	CEtUIDialog::Show( bShow );

	// Due to a hotkey bug, we have no choice but to handle it as below.
	// (Forcibly resets the hotkey state of the dialog that has the hotkey to open this create-chatroom window.)
	// Otherwise,
	// After opening window with c (default create-chatroom shortcut), pressing c again, closing with Esc, pressing c does nothing and it only reopens on the next c press.
	if( GetInterface().GetMainBarDialog() )
		GetInterface().GetMainBarDialog()->SetHotKeyState( 0 );

	if( m_pCallback )
	{
		m_pCallback->OnUICallbackProc( GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL );
	}
}

void CDnChatRoomCreateDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CREATE") )
		{
			std::wstring szName = m_pEditBoxName->GetText();
			if( DN_INTERFACE::UTIL::CheckChat( szName ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK, 0, this );
				return;
			}

			std::wstring wszPR0 = m_pEditBoxPR0->GetText();
			if( DN_INTERFACE::UTIL::CheckChat( wszPR0 ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK, 0, this );
				return;
			}

			std::wstring wszPR1 = m_pEditBoxPR1->GetText();
			if( DN_INTERFACE::UTIL::CheckChat( wszPR1 ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK, 0, this );
				return;
			}

			std::wstring wszPR2 = m_pEditBoxPR2->GetText();
			if( DN_INTERFACE::UTIL::CheckChat( wszPR2 ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK, 0, this );
				return;
			}

			if( m_pCheckBoxSecret->IsChecked() )
			{
				std::wstring wszPassword = m_pEditBoxPassword->GetText();
				if( wszPassword.empty() || ((int)wszPassword.size() < CHATROOMPASSWORDMAX) )
				{
					GetInterface().MessageBox( MESSAGEBOX_35, MB_OK, 0, this );
					return;
				}
			}

			if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage )
			{
				if( GetPartyTask().GetPartyRole() != CDnPartyTask::SINGLE )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 8129 ) );
					return;
				}
			}
			else
			{
				CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
				bool bFarmGameTask = false;
				if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
					bFarmGameTask = true;
 
				if( !bFarmGameTask )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7463 ) );
					return;
				}
			}

			int nType = 0, nAllow = 0;
			m_pComboRoomType->GetSelectedValue( nType );
			m_pComboCondition->GetSelectedValue( nAllow );
			GetChatRoomTask().RequestCreateChatRoom( szName.c_str(), m_pEditBoxPassword->GetText(), nType, nAllow, wszPR0.c_str(), wszPR1.c_str(), wszPR2.c_str() );
			m_pButtonOK->Enable( false );
			return;
		}

		if( IsCmdControl("ID_BUTTON_CLOSE") )
		{
			Show(false);
		}
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED )
	{
		if( IsCmdControl("ID_COMBOBOX_COUNT") )
		{
			SComboBoxItem *pItem = m_pComboRoomType->GetSelectedItem();
			if( pItem )
			{
				// Check if it is a PR chat room
				bool bPR = (pItem->nValue == CHATROOMTYPE_PR);
				m_pEditBoxPR0->Enable( bPR );
				m_pEditBoxPR1->Enable( bPR );
				m_pEditBoxPR2->Enable( bPR );
				if( !bPR )
				{
					m_pEditBoxPR0->ClearText();
					m_pEditBoxPR1->ClearText();
					m_pEditBoxPR2->ClearText();
				}
			}
		}
	}
	else if( nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if( IsCmdControl("ID_CHECKBOX_SECRET") )
		{
			bool bChecked = m_pCheckBoxSecret->IsChecked();
			m_pEditBoxPassword->Enable( bChecked );
			GetControl<CEtUIStatic>("ID_STATIC12")->Show( bChecked );
			GetControl<CEtUIStatic>("ID_STATIC13")->Show( bChecked );
			if( !bChecked )
				m_pEditBoxPassword->ClearText();
		}
	}
	else if( nCommand == EVENT_EDITBOX_CHANGE || nCommand == EVENT_EDITBOX_IME_CHANGE )
	{
		// Since this dialog must focus the edit box when shown via hotkey, there was an issue.
		// WM_CHAR arriving after the hotkey's WM_KEYDOWN entered the focused edit box and got typed as a character.
		// No better workaround for now, so clear it again in EVENT_EDITBOX_CHANGE triggered right after Show.
		if( m_bShowedJustNow )
		{
			m_pEditBoxName->ClearText();
			m_bShowedJustNow = false;
		}

		if( IsCmdControl("ID_IMEEDITBOX_CHATNAME") )
		{
			bool bCreatable = true;
			std::wstring szName = m_pEditBoxName->GetText();
			szName = boost::algorithm::trim_copy(szName);
			if (szName.empty())
				bCreatable = false;
			if( m_pEditBoxName->GetTextLength() + CEtUIIME::GetCompStringLength() > 0 && bCreatable )
				m_pButtonOK->Enable( true );
			else
				m_pButtonOK->Enable( false );
		}
	}
	else if( nCommand == EVENT_EDITBOX_STRING )
	{
		focus::ReleaseControl();
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChatRoomCreateDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_OK" ) )
		{
			RequestFocus( m_pEditBoxName );
			m_pButtonOK->Enable( true );
		}
	}
}