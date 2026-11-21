#include "StdAfx.h"
#include "DnPrivateChatDlg.h"
#include "VillageSendPacket.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnPrivateChatDlg::CDnPrivateChatDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_pStaticChat(NULL)
	, m_pEditBoxChat(NULL)
	, m_eChatType(CHATTYPE_NORMAL)
{
	m_strName.reserve( NAMELENMAX );
}

CDnPrivateChatDlg::~CDnPrivateChatDlg(void)
{
}

void CDnPrivateChatDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PrivateChatDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChatDlg::InitialUpdate()
{
	m_pStaticChat = GetControl<CEtUIStatic>("ID_STATIC_CHAT");
	m_pEditBoxChat = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_CHAT");
}

void CDnPrivateChatDlg::DoSendChat()
{
	std::wstring strMsg;
	strMsg = m_pEditBoxChat->GetText();
	strMsg.reserve( CHATLENMAX );
	if( !strMsg.empty() )
	{
		if( m_eChatType == CHATTYPE_GUILD )
		{
			SendChatMsg( CHATTYPE_GUILD, strMsg.c_str() );
		}
		else
		{
			//SendChatPrivateMsg( m_strName.c_str(), strMsg.c_str() );
			GetInterface().SendChatMessage( CHATTYPE_PRIVATE, m_strName, strMsg );
		}
	}
}

void CDnPrivateChatDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK" ) )
		{
			DoSendChat();
		}

		Show( false );
	}
}

void CDnPrivateChatDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		ASSERT( !m_strName.empty()&&"CDnPrivateChatDlg::Show" );

		m_pEditBoxChat->ClearText();
		RequestFocus( m_pEditBoxChat );
	}
}

bool CDnPrivateChatDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hWnd;

	if( !IsShow() )
		return false;

	if( uMsg == WM_KEYDOWN )
	{
		if( wParam == VK_ESCAPE )
		{
			Show(false);
			return true;
		}
		else if (wParam == VK_RETURN)
		{
			DoSendChat();
			Show(false);
			return true;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnPrivateChatDlg::SetPrivateName( LPCWSTR szPrivateName )
{
	m_strName = szPrivateName;

	wchar_t szTemp[256]={0};
	swprintf_s( szTemp, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1345/*"Send a whisper to \"%s\"."*/ ), szPrivateName );

	m_pStaticChat->SetText( szTemp );
}

void CDnPrivateChatDlg::SetGuildName( LPCWSTR szGuildName )
{
	m_strName = szGuildName;

	wchar_t szTemp[256]={0};
	swprintf_s( szTemp, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1358 ), szGuildName );

	m_pStaticChat->SetText( szTemp );
	m_eChatType = CHATTYPE_GUILD;
}