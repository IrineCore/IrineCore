#include "StdAfx.h"
#include "DnPrivateChannelDlg.h"
#include "DnInterface.h"
#include "DnFriendTask.h"
#include "DnChatTabDlg.h"
#include "DnChannelChatTask.h"
#include "DNCountryUnicodeSet.h"
#include "DnInterfaceString.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif


#ifdef PRE_PRIVATECHAT_CHANNEL

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelMessageBoxDlg

CDnPrivateChannelCreatePWDlg::CDnPrivateChannelCreatePWDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pIMEEditBox( NULL )
, m_pEditBox( NULL )
{
}

CDnPrivateChannelCreatePWDlg::~CDnPrivateChannelCreatePWDlg()
{
}

void CDnPrivateChannelCreatePWDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelCreatePWDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelCreatePWDlg::InitialUpdate()
{
	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
	m_pEditBox = GetControl<CEtUIEditBox>( "ID_EDITBOX_PASSWORD" );
}

void CDnPrivateChannelCreatePWDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pIMEEditBox )
			m_pIMEEditBox->ClearText();

		if( m_pEditBox )
			m_pEditBox->ClearText();

		RequestFocus( m_pIMEEditBox );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelCreatePWDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			std::wstring strChannelName;
			std::wstring strPassword;

			if( m_pIMEEditBox )
				strChannelName = m_pIMEEditBox->GetText();

			if( m_pEditBox )
				strPassword = m_pEditBox->GetText();

			if( strChannelName.size() < CDnChatTabDlg::CHANNEL_NAME_MIN || strChannelName.size() > CDnChatTabDlg::CHANNEL_NAME_MAX )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7812 ), MB_OK );	// UISTRING : Please enter a channel title between 2 and 10 characters.
			}
			else if( !g_CountryUnicodeSet.Check( strChannelName.c_str() ) || DN_INTERFACE::UTIL::CheckAccount( strChannelName ) || DN_INTERFACE::UTIL::CheckChat( strChannelName ) )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK );	// UISTRING : Contains prohibited words.
			}
			else
			{
				if( static_cast<int>( strPassword.size() ) == 0 )
					GetChannelChatTask().SendChannelAdd( strChannelName.c_str(), -1 );
				else if( static_cast<int>( strPassword.size() ) < 4 )
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7948 ), MB_OK );	// UISTRING : Please set a 4-digit password.
				else if( static_cast<int>( strPassword.size() ) == 4 )
					GetChannelChatTask().SendChannelAdd( strChannelName.c_str(), _wtoi( m_pEditBox->GetText() ) );
			}
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) 
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelJoinDlg

CDnPrivateChannelJoinDlg::CDnPrivateChannelJoinDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pIMEEditBox( NULL )
{
}

CDnPrivateChannelJoinDlg::~CDnPrivateChannelJoinDlg()
{
}

void CDnPrivateChannelJoinDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelCreateMessageDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelJoinDlg::InitialUpdate()
{
	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
}

void CDnPrivateChannelJoinDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pIMEEditBox )
			m_pIMEEditBox->ClearText();

		RequestFocus( m_pIMEEditBox );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelJoinDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			if( m_pIMEEditBox )
				GetChannelChatTask().SendChannelJoin( m_pIMEEditBox->GetText(), -1 );
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) 
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelPasswordDlg

CDnPrivateChannelPasswordDlg::CDnPrivateChannelPasswordDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pEditBox( NULL )
{
}

CDnPrivateChannelPasswordDlg::~CDnPrivateChannelPasswordDlg()
{
}

void CDnPrivateChannelPasswordDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelPassDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelPasswordDlg::InitialUpdate()
{
	m_pEditBox = GetControl<CEtUIEditBox>( "ID_EDITBOX_PASS" );
}

void CDnPrivateChannelPasswordDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		RequestFocus( m_pEditBox );
	}
	else
	{
		m_strChannelName.clear();
		if( m_pEditBox )
			m_pEditBox->ClearText();
	}
}

void CDnPrivateChannelPasswordDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( m_pEditBox == NULL )
			return;

		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			if( m_strChannelName.length() > 0 )
			{
				std::wstring strPassword;
				strPassword = m_pEditBox->GetText();

				if( static_cast<int>( strPassword.size() ) == 0 )
				{
					GetChannelChatTask().SendChannelJoin( m_strChannelName.c_str(), -1 );
				}
				else if( static_cast<int>( strPassword.size() ) < 4 )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7812 ), MB_OK );	// UISTRING : Please enter a channel title between 2 and 10 characters.
				}
				else if( static_cast<int>( strPassword.size() ) == 4 )
				{
					GetChannelChatTask().SendChannelJoin( m_strChannelName.c_str(), _wtoi( m_pEditBox->GetText() ) );
				}
			}
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelPasswordChangeDlg

CDnPrivateChannelPasswordChangeDlg::CDnPrivateChannelPasswordChangeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pEditBox( NULL )
{
}

CDnPrivateChannelPasswordChangeDlg::~CDnPrivateChannelPasswordChangeDlg()
{
}

void CDnPrivateChannelPasswordChangeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelPassChangeDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelPasswordChangeDlg::InitialUpdate()
{
	m_pEditBox = GetControl<CEtUIEditBox>( "ID_EDITBOX_PASS" );
}

void CDnPrivateChannelPasswordChangeDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		RequestFocus( m_pEditBox );
	}
	else
	{
		if( m_pEditBox )
			m_pEditBox->ClearText();
	}
}

void CDnPrivateChannelPasswordChangeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( m_pEditBox == NULL )
			return;

		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			std::wstring strPassword;
			strPassword = m_pEditBox->GetText();

			if( static_cast<int>( strPassword.size() ) == 0 )	// Clear password
			{
				GetChannelChatTask().SendChannelMod( PrivateChatChannel::Common::ChangePassWord, -1 );
			}
			else if( static_cast<int>( strPassword.size() ) < 4 )
			{
				GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7812 ), MB_OK );	// UISTRING : Please enter a channel title between 2 and 10 characters.
			}
			else if( static_cast<int>( strPassword.size() ) == 4 )	// Set password
			{
				GetChannelChatTask().SendChannelMod( PrivateChatChannel::Common::ChangePassWord, _wtoi( strPassword.c_str() ) );
			}
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#else // PRE_ADD_PRIVATECHAT_CHANNEL

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelMessageBoxDlg

CDnPrivateChannelMessageBoxDlg::CDnPrivateChannelMessageBoxDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pIMEEditBox( NULL )
, m_eChannelJoinType( TYPE_CHANNEL_NONE	)
{
}

CDnPrivateChannelMessageBoxDlg::~CDnPrivateChannelMessageBoxDlg()
{
}

void CDnPrivateChannelMessageBoxDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelCreateMessageDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelMessageBoxDlg::InitialUpdate()
{
	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
}

void CDnPrivateChannelMessageBoxDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_pIMEEditBox )
			m_pIMEEditBox->ClearText();

		RequestFocus( m_pIMEEditBox );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelMessageBoxDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			if( m_eChannelJoinType == TYPE_CHANNEL_CREATE )
			{
				if( m_pIMEEditBox )
				{
					std::wstring strChannelName = m_pIMEEditBox->GetText();

					if( !g_CountryUnicodeSet.Check( strChannelName.c_str() ) || DN_INTERFACE::UTIL::CheckAccount( strChannelName ) || DN_INTERFACE::UTIL::CheckChat( strChannelName ) )
					{
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3772 ), MB_OK );
					}
					else
					{
						GetChannelChatTask().SendChannelAdd( strChannelName.c_str() );
						Show( false );
					}
				}
			}
			else if( m_eChannelJoinType == TYPE_CHANNEL_JOIN )
			{
				if( m_pIMEEditBox )
					GetChannelChatTask().SendChannelJoin( m_pIMEEditBox->GetText(), -1 );
				Show( false );
			}
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) 
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelPasswordDlg

CDnPrivateChannelPasswordDlg::CDnPrivateChannelPasswordDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pEditBox( NULL )
, m_eChannelPasswordType( TYPE_PASSWORD_NONE )
{
}

CDnPrivateChannelPasswordDlg::~CDnPrivateChannelPasswordDlg()
{
}

void CDnPrivateChannelPasswordDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelPassDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelPasswordDlg::InitialUpdate()
{
	m_pEditBox = GetControl<CEtUIEditBox>( "ID_EDITBOX_PASS" );
}

void CDnPrivateChannelPasswordDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		RequestFocus( m_pEditBox );
	}
	else
	{
		m_strChannelName.clear();
		if( m_pEditBox )
			m_pEditBox->ClearText();
	}
}

void CDnPrivateChannelPasswordDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( m_pEditBox == NULL )
			return;

		if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_OK" ) ) 
		{
			if( m_eChannelPasswordType == TYPE_PASSWORD_JOIN )
			{
				if( m_strChannelName.length() > 0 )
				{
					std::wstring strPassword;
					strPassword = m_pEditBox->GetText();

					if( static_cast<int>( strPassword.size() ) == 0 )
					{
						GetChannelChatTask().SendChannelJoin( m_strChannelName.c_str(), -1 );
#ifndef PRE_ADD_PRIVATECHAT_CHANNEL
						Show( false );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
					}
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
					else if( static_cast<int>( strPassword.size() ) < 4 )
					{
						GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7812 ), MB_OK );	// UISTRING : Please enter a channel title between 2 and 10 characters.
					}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
					else if( static_cast<int>( strPassword.size() ) == 4 )
					{
						GetChannelChatTask().SendChannelJoin( m_strChannelName.c_str(), _wtoi( m_pEditBox->GetText() ) );
#ifndef PRE_ADD_PRIVATECHAT_CHANNEL
						Show( false );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
					}
				}
			}
			else if( m_eChannelPasswordType == TYPE_PASSWORD_CHANGE )
			{
				std::wstring strPassword;
				strPassword = m_pEditBox->GetText();

				if( static_cast<int>( strPassword.size() ) == 0 )	// Clear password
				{
					GetChannelChatTask().SendChannelMod( PrivateChatChannel::Common::ChangePassWord, -1 );
#ifndef PRE_ADD_PRIVATECHAT_CHANNEL
					Show( false );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
				}
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
				else if( static_cast<int>( strPassword.size() ) < 4 )
				{
					GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7812 ), MB_OK );	// UISTRING : Please enter a channel title between 2 and 10 characters.
				}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
				else if( static_cast<int>( strPassword.size() ) == 4 )	// Set password
				{
					GetChannelChatTask().SendChannelMod( PrivateChatChannel::Common::ChangePassWord, _wtoi( strPassword.c_str() ) );
#ifndef PRE_ADD_PRIVATECHAT_CHANNEL
					Show( false );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
				}
			}	
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#endif // PRE_ADD_PRIVATECHAT_CHANNEL

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelCreateDlg

CDnPrivateChannelCreateDlg::CDnPrivateChannelCreateDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
, m_pDnPrivateChannelCreatePWDlg( NULL )
, m_pDnPrivateChannelJoinDlg( NULL )
#else // PRE_ADD_PRIVATECHAT_CHANNEL
, m_pDnPrivateChannelMessageBoxDlg( NULL )
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
{
}

CDnPrivateChannelCreateDlg::~CDnPrivateChannelCreateDlg()
{
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	SAFE_DELETE( m_pDnPrivateChannelCreatePWDlg );
	SAFE_DELETE( m_pDnPrivateChannelJoinDlg );
#else // PRE_ADD_PRIVATECHAT_CHANNEL
	SAFE_DELETE( m_pDnPrivateChannelMessageBoxDlg );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
}

void CDnPrivateChannelCreateDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelCreateDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelCreateDlg::InitialUpdate()
{
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	m_pDnPrivateChannelCreatePWDlg = new CDnPrivateChannelCreatePWDlg( UI_TYPE_MODAL );
	m_pDnPrivateChannelCreatePWDlg->Initialize( false );
	m_pDnPrivateChannelJoinDlg = new CDnPrivateChannelJoinDlg( UI_TYPE_MODAL );
	m_pDnPrivateChannelJoinDlg->Initialize( false );
#else // PRE_ADD_PRIVATECHAT_CHANNEL
	m_pDnPrivateChannelMessageBoxDlg = new CDnPrivateChannelMessageBoxDlg( UI_TYPE_MODAL );
	m_pDnPrivateChannelMessageBoxDlg->Initialize( false );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
}

void CDnPrivateChannelCreateDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		CEtUITextBox* pTextBoxHelp = GetControl<CEtUITextBox>( "ID_TEXTBOX_LIST" );
		pTextBoxHelp->ClearText();

		if( pTextBoxHelp )
		{
			std::vector<std::wstring> vecHelps;
			TokenizeW_NewLine( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7836 ), vecHelps, L"\\n" );

			for( int i=0; i<static_cast<int>( vecHelps.size() ); i++ )
			{
				pTextBoxHelp->AddText( vecHelps[i].c_str() );
			}
		}
	}
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	else
	{
		if( m_pDnPrivateChannelCreatePWDlg )
			m_pDnPrivateChannelCreatePWDlg->Show( false );
		if( m_pDnPrivateChannelJoinDlg )
			m_pDnPrivateChannelJoinDlg->Show( false );
	}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelCreateDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
		if( IsCmdControl( "ID_BT_CREATE" ) ) 
		{
			if( m_pDnPrivateChannelCreatePWDlg )
			{
				m_pDnPrivateChannelCreatePWDlg->Show( true );
			}
		}
		else if( IsCmdControl( "ID_BT_ADD" ) ) 
		{
			if( m_pDnPrivateChannelJoinDlg )
			{
				m_pDnPrivateChannelJoinDlg->Show( true );
			}
		}
#else // PRE_ADD_PRIVATECHAT_CHANNEL
		if( IsCmdControl( "ID_BT_CREATE" ) ) 
		{
			if( m_pDnPrivateChannelMessageBoxDlg )
			{
				m_pDnPrivateChannelMessageBoxDlg->SetChannelJoinType( CDnPrivateChannelMessageBoxDlg::TYPE_CHANNEL_CREATE );
				m_pDnPrivateChannelMessageBoxDlg->Show( true );
			}
		}
		else if( IsCmdControl( "ID_BT_ADD" ) ) 
		{
			if( m_pDnPrivateChannelMessageBoxDlg )
			{
				m_pDnPrivateChannelMessageBoxDlg->SetChannelJoinType( CDnPrivateChannelMessageBoxDlg::TYPE_CHANNEL_JOIN );
				m_pDnPrivateChannelMessageBoxDlg->Show( true );
			}
		}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
bool CDnPrivateChannelCreateDlg::IsShowChannelCreateJoinDlg()
{
	bool bShowChannelCreateJoinDlg = false;
	if( m_pDnPrivateChannelCreatePWDlg->IsShow() || m_pDnPrivateChannelJoinDlg->IsShow() )
		bShowChannelCreateJoinDlg = true;
	
	return bShowChannelCreateJoinDlg;
}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

//////////////////////////////////////////////////////////////////////////
// CDnChannelEntryInfoDlg

CDnChannelEntryPopupDlg::CDnChannelEntryPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_nCharacterDBID( 0 )
{
}

CDnChannelEntryPopupDlg::~CDnChannelEntryPopupDlg()
{
}

void CDnChannelEntryPopupDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelInfoDlg.ui" ).c_str(), bShow );
}

void CDnChannelEntryPopupDlg::InitialUpdate()
{
}

void CDnChannelEntryPopupDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( GetChannelChatTask().IsActive() && GetChannelChatTask().IsChannelMaster() )
		{
			CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BUTTON_MASTER" );
			if( pButton )
				pButton->Enable( true );
			
			pButton = GetControl<CEtUIButton>( "ID_BUTTON_BAN" );
			if( pButton )
				pButton->Enable( true );
		}
		else
		{
			CEtUIButton* pButton = GetControl<CEtUIButton>( "ID_BUTTON_MASTER" );
			if( pButton )
				pButton->Enable( false );

			pButton = GetControl<CEtUIButton>( "ID_BUTTON_BAN" );
			if( pButton )
				pButton->Enable( false );
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnChannelEntryPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	PrivateChatChannel::TMemberInfo* pChannelMemberInfo = GetChannelChatTask().GetChannelMemberInfoByDBID( m_nCharacterDBID );
	if( pChannelMemberInfo == NULL ) return;

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BUTTON_WHISPER" ) )
		{
			GetInterface().GetChatDialog()->ShowEx( true );
			GetInterface().GetChatDialog()->SetPrivateName( pChannelMemberInfo->wszCharacterName );
		}
		else if( IsCmdControl( "ID_BUTTON_FRIEND" ) )
		{
			wchar_t wszQuery[255];
			swprintf_s(wszQuery, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1418 ), pChannelMemberInfo->wszCharacterName );	// UISTRING : Add %s to your friend list?
			GetInterface().MessageBox( wszQuery, MB_YESNO, MESSAGEBOX_FRIEND_QUERY, this );
		}
		else if( IsCmdControl( "ID_BUTTON_MASTER" ) )
		{
			GetChannelChatTask().SendChannelMod( PrivateChatChannel::Common::ChangeMaster, 0, m_nCharacterDBID );
		}
		else if( IsCmdControl( "ID_BUTTON_BAN" ) )
		{
			GetChannelChatTask().SendChannelKick( pChannelMemberInfo->wszCharacterName );
		}

		Show( false );
		return;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChannelEntryPopupDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	PrivateChatChannel::TMemberInfo* pChannelMemberInfo = GetChannelChatTask().GetChannelMemberInfoByDBID( m_nCharacterDBID );
	if( pChannelMemberInfo == NULL ) return;

	switch( nID ) 
	{
	case MESSAGEBOX_FRIEND_QUERY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED )
			{
				if( IsCmdControl( "ID_YES" ) )
				{
					GetFriendTask().RequestFriendAdd( 0, pChannelMemberInfo->wszCharacterName );
				}
			}
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// CDnChannelEntryInfoDlg

CDnChannelEntryInfoDlg::CDnChannelEntryInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_biMemberDBID( 0 )
, m_bMaster( false )
{
}

CDnChannelEntryInfoDlg::~CDnChannelEntryInfoDlg()
{
}

void CDnChannelEntryInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelNameList_ListDlg.ui" ).c_str(), bShow );
}

void CDnChannelEntryInfoDlg::InitialUpdate()
{
}

void CDnChannelEntryInfoDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnChannelEntryInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChannelEntryInfoDlg::SetChannelEntryInfo( PrivateChatChannel::TMemberInfo& channelMemeberInfo )
{
	CEtUIStatic* pMasterIcon = GetControl<CEtUIStatic>( "ID_STATIC1" );
	CEtUIStatic* pMasterName = GetControl<CEtUIStatic>( "ID_TEXT_MARSTERNAME" );
	CEtUIStatic* pNormalName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );

	m_biMemberDBID = channelMemeberInfo.biCharacterDBID;
	m_bMaster = channelMemeberInfo.bMaster;
	m_strMemberName = channelMemeberInfo.wszCharacterName;

	if( m_bMaster )
	{
		pMasterIcon->Show( true );
		pMasterName->Show( true );
		pMasterName->SetText( m_strMemberName );
		pNormalName->Show( false );
		pNormalName->ClearText();
	}
	else
	{
		pMasterIcon->Show( false );
		pMasterName->Show( false );
		pMasterName->ClearText();
		pNormalName->Show( true );
		pNormalName->SetText( m_strMemberName );
	}
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelInfoDlg

CDnPrivateChannelInfoDlg::CDnPrivateChannelInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pDnChannelEntryPopupDlg( NULL )
, m_pChannelEntryListBox( NULL )
{
}

CDnPrivateChannelInfoDlg::~CDnPrivateChannelInfoDlg()
{
	SAFE_DELETE( m_pDnChannelEntryPopupDlg );
}

void CDnPrivateChannelInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelNameListDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelInfoDlg::InitialUpdate()
{
	m_pDnChannelEntryPopupDlg = new CDnChannelEntryPopupDlg( UI_TYPE_CHILD, this );
	m_pDnChannelEntryPopupDlg->Initialize( false );

	m_pChannelEntryListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX0" );
}

void CDnPrivateChannelInfoDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		RefreshChannelEntryList();
	}
	else
	{
		if( m_pDnChannelEntryPopupDlg && m_pDnChannelEntryPopupDlg->IsShow() )
			ShowChildDialog( m_pDnChannelEntryPopupDlg, false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

bool CDnPrivateChannelInfoDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	m_hWnd = hWnd;

	if( !m_bShow )
	{
		return false;
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
				CDnChannelEntryInfoDlg* pSelectEntryInfoDlg = NULL;
				bool bIsInsideItem = false;
				for( int i = m_pChannelEntryListBox->GetScrollBar()->GetTrackPos(); i < m_pChannelEntryListBox->GetSize(); ++i ) 
				{
					pSelectEntryInfoDlg = m_pChannelEntryListBox->GetItem<CDnChannelEntryInfoDlg>(i);
					if( !pSelectEntryInfoDlg ) continue;
					SUICoord uiCoord;
					pSelectEntryInfoDlg->GetDlgCoord( uiCoord );

					bool bChannelMaster = false;
					if( __wcsicmp_l( pSelectEntryInfoDlg->GetMemberName().c_str(), CDnActor::s_hLocalActor->GetName() ) == 0 ) 
					{
						bChannelMaster = true;
					}

					if( uiCoord.IsInside( fMouseX, fMouseY ) && !bChannelMaster ) 
					{
						bIsInsideItem = true;
						break;
					}
				}

				if( bIsInsideItem && pSelectEntryInfoDlg ) 
				{
					ShowChildDialog( m_pDnChannelEntryPopupDlg, false );
					m_pDnChannelEntryPopupDlg->SetPosition( fMouseX, fMouseY );
					static float fXRGap(4.0f/DEFAULT_UI_SCREEN_WIDTH), fYBGap(4.0f/DEFAULT_UI_SCREEN_HEIGHT);
					SUICoord sDlgCoord;
					m_pDnChannelEntryPopupDlg->GetDlgCoord( sDlgCoord );
					if( (sDlgCoord.Right()+fXRGap) > GetScreenWidthRatio() )
						sDlgCoord.fX -= (sDlgCoord.Right()+fXRGap - GetScreenWidthRatio());
					m_pDnChannelEntryPopupDlg->SetDlgCoord( sDlgCoord );
					m_pDnChannelEntryPopupDlg->SetChannelMemberDBID( pSelectEntryInfoDlg->GetMemberDBID() );
					ShowChildDialog( m_pDnChannelEntryPopupDlg, true );
				}
			}
			break;
		case WM_LBUTTONDOWN:
			{
				if( IsMouseInDlg() ) 
				{
					if( m_pDnChannelEntryPopupDlg->IsShow() ) 
					{
						SUICoord uiCoord;
						m_pDnChannelEntryPopupDlg->GetDlgCoord( uiCoord );
						if( !uiCoord.IsInside( fMouseX, fMouseY ) ) 
						{
							ShowChildDialog( m_pDnChannelEntryPopupDlg, false );
							return true;
						}
					}
				}
			}
			break;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnPrivateChannelInfoDlg::RefreshChannelEntryList()
{
	if( m_pChannelEntryListBox == NULL )
		return;

#ifndef PRE_ADD_PRIVATECHAT_CHANNEL
	CEtUIStatic* pChannelName = GetControl<CEtUIStatic>( "ID_STATIC6" );
	if( pChannelName )
		pChannelName->SetText( GetChannelChatTask().GetChannelName() );
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

	m_pChannelEntryListBox->RemoveAllItems();

	std::vector<PrivateChatChannel::TMemberInfo>& vecChannelMemberInfoList = GetChannelChatTask().GetChannelMemeberInfoList();

	for( int i=0; i<static_cast<int>( vecChannelMemberInfoList.size() ); i++ )
	{
		CDnChannelEntryInfoDlg* pDnChannelEntryInfoDlg = m_pChannelEntryListBox->AddItem<CDnChannelEntryInfoDlg>();
		if( pDnChannelEntryInfoDlg )
			pDnChannelEntryInfoDlg->SetChannelEntryInfo( vecChannelMemberInfoList[i] );
	}

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
	CEtUIStatic* pChannelName = GetControl<CEtUIStatic>( "ID_STATIC6" );
	if( pChannelName )
	{
		std::wstring strChannelInfoTitle;
		strChannelInfoTitle = FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7944 ), GetChannelChatTask().GetChannelName().c_str(), 
										static_cast<int>( vecChannelMemberInfoList.size() ), PrivateChatChannel::Common::GetDBMaxChannel );
		pChannelName->SetText( strChannelInfoTitle.c_str() );
	}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelInviteDlg

CDnPrivateChannelInviteDlg::CDnPrivateChannelInviteDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pEditBoxName( NULL )
, m_pFriendComboBox( NULL )
{
}

CDnPrivateChannelInviteDlg::~CDnPrivateChannelInviteDlg()
{
}

void CDnPrivateChannelInviteDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelAddDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelInviteDlg::InitialUpdate()
{
	m_pEditBoxName = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
	m_pFriendComboBox = GetControl<CEtUIComboBox>( "ID_COMBOBOX_NAME" );
	m_pFriendComboBox->SetEditMode( true );
}

void CDnPrivateChannelInviteDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_pEditBoxName->ClearText();
		SetFriendComboBox();
		RequestFocus( m_pEditBoxName );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelInviteDlg::SetFriendComboBox()
{
	int i = 0;
	const CDnFriendTask::FRIEND_MAP& friendList = GetFriendTask().GetFriendList();
	m_pFriendComboBox->RemoveAllItems();

	CDnFriendTask::FRIEND_MAP::const_iterator iter = friendList.begin();
	for( ; iter != friendList.end(); ++iter )
	{
		CDnFriendTask::FRIENDINFO_MAP *pFriendInfo = iter->second;
		if( pFriendInfo == NULL )
			continue;

		CDnFriendTask::FRIENDINFO_MAP::const_iterator fmIter = pFriendInfo->begin();
		for( ; fmIter != pFriendInfo->end(); ++fmIter )
		{
			const CDnFriendTask::SFriendInfo& info = fmIter->second;
			m_pFriendComboBox->AddItem( info.wszFriendName.c_str(), NULL, i );
			m_FriendNameList.insert( make_pair( info.wszFriendName[0], info.wszFriendName ) );
			++i;
		}
	}

	m_pFriendComboBox->ClearSelectedItem();
}

void CDnPrivateChannelInviteDlg::OnAddEditBoxString( const std::wstring& strName )
{
	int curEditBoxCaretPos = m_pEditBoxName->GetCaretPos();
	if( strName.size() < 1 ) return;

	std::pair<std::multimap<wchar_t, std::wstring>::iterator, std::multimap<wchar_t, std::wstring>::iterator> range;
	range = m_FriendNameList.equal_range( strName[0] );

	std::wstring almost = _T("");
	std::wstring::size_type longest = 0;
	std::multimap<wchar_t, std::wstring>::iterator it = range.first;
	for( ; it != range.second; ++it )
	{
		std::wstring& friendName = (*it).second;
		if( friendName == strName )
			return;
		std::wstring::size_type size = friendName.find( strName );
		if( size != std::wstring::npos && size >= longest )
			almost = friendName;
	}

	if( almost != _T("") )
	{
		m_pEditBoxName->ClearText();
		m_pEditBoxName->SetText( almost.c_str() );
		m_pEditBoxName->SetSelection( m_pEditBoxName->GetTextLength(), curEditBoxCaretPos );
		m_AutoCompleteTextCache = almost;
	}
	else
	{
		m_AutoCompleteTextCache = _T("");
	}
}

void CDnPrivateChannelInviteDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( IsCmdControl( "ID_IMEEDITBOX_NAME" ) )
	{
		if( nCommand == EVENT_EDITBOX_IME_CHANGE )
		{
			std::wstring curText = m_pEditBoxName->GetText();
			std::wstring composeStr;
			m_pEditBoxName->GetIMECompositionString( composeStr );
			curText += composeStr;
			OnAddEditBoxString( curText );
		}
		else if( nCommand == EVENT_EDITBOX_CHANGE )
		{
			WCHAR ch = (WCHAR)uMsg;
			if( CommonUtil::IsCtrlChar(ch) == false )
			{
				std::wstring curText = m_pEditBoxName->GetText();
				OnAddEditBoxString( curText );
			}
		}
		else if( nCommand != EVENT_EDITBOX_CHANGE )
		{
			if( nCommand == EVENT_EDITBOX_FOCUS || nCommand == EVENT_EDITBOX_RELEASEFOCUS )
			{
				CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
				return;
			}

			if( m_AutoCompleteTextCache != _T("") )
			{
				m_pEditBoxName->ClearText();
				m_pEditBoxName->SetText( m_AutoCompleteTextCache.c_str() );
				m_AutoCompleteTextCache = _T("");
			}
		}
		return;
	}
	else if( nCommand == EVENT_COMBOBOX_SELECTION_CHANGED && bTriggeredByUser )
	{
		if( IsCmdControl( "ID_COMBOBOX_NAME" ) && m_pFriendComboBox->IsOpenedDropDownBox() == false )
		{
			SComboBoxItem* pItem = m_pFriendComboBox->GetSelectedItem();
			if( pItem )
			{
				m_pEditBoxName->ClearText();
				m_pEditBoxName->SetText( pItem->strText );
			}
		}
	}
	else if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl( "ID_BT_OK" ) ) 
		{
			GetChannelChatTask().SendChannelInvite( m_pEditBoxName->GetText() );
			Show( false );
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) 
		{
			Show( false );
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//////////////////////////////////////////////////////////////////////////
// CDnPrivateChannelDlg

CDnPrivateChannelDlg::CDnPrivateChannelDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_pDnPrivateChannelCreateDlg( NULL )
, m_pDnPrivateChannelInfoDlg( NULL )
, m_pButtonChannelInvite( NULL )
, m_pButtonChannelPassword( NULL )
, m_pButtonChannelLeave( NULL )
{
}

CDnPrivateChannelDlg::~CDnPrivateChannelDlg()
{
	SAFE_DELETE( m_pDnPrivateChannelCreateDlg );
	SAFE_DELETE( m_pDnPrivateChannelInfoDlg );
}

void CDnPrivateChannelDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ChannelDlg.ui" ).c_str(), bShow );
}

void CDnPrivateChannelDlg::InitialUpdate()
{
	m_pDnPrivateChannelCreateDlg = new CDnPrivateChannelCreateDlg( UI_TYPE_CHILD, this );
	m_pDnPrivateChannelCreateDlg->Initialize( false );
	m_pDnPrivateChannelInfoDlg = new CDnPrivateChannelInfoDlg( UI_TYPE_CHILD, this );
	m_pDnPrivateChannelInfoDlg->Initialize( false );

	m_pButtonChannelInvite = GetControl<CEtUIButton>( "ID_BT_INVITE" );
	m_pButtonChannelPassword = GetControl<CEtUIButton>( "ID_BT_PASSWORD" );
	m_pButtonChannelLeave = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
}

void CDnPrivateChannelDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( GetChannelChatTask().IsActive() && GetChannelChatTask().IsJoinChannel() )
		{
			ShowChannelJoinMode();
		}
		else
		{
			ShowChannelCreateMode();
		}
	}
	else
	{
		if( m_pDnPrivateChannelCreateDlg )
			m_pDnPrivateChannelCreateDlg->Show( false );
		if( m_pDnPrivateChannelInfoDlg )
			m_pDnPrivateChannelInfoDlg->Show( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnPrivateChannelDlg::ShowChannelCreateMode()
{
	m_pDnPrivateChannelCreateDlg->Show( true );
	m_pDnPrivateChannelInfoDlg->Show( false );
	m_pButtonChannelInvite->Enable( false );
	m_pButtonChannelPassword->Enable( false );
	m_pButtonChannelLeave->Enable( false );
}

void CDnPrivateChannelDlg::ShowChannelJoinMode()
{
	m_pDnPrivateChannelCreateDlg->Show( false );
	m_pDnPrivateChannelInfoDlg->Show( true );
	m_pButtonChannelInvite->Enable( true );
	m_pButtonChannelPassword->Enable( true );
	m_pButtonChannelLeave->Enable( true );
	RefreshChannelInfo();
}

void CDnPrivateChannelDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED ) 
	{
		if( IsCmdControl( "ID_BT_INVITE" ) ) 
		{
			CDnPrivateChannelInviteDlg* pDnPrivateChannelInviteDlg = GetInterface().GetPrivateChannelInviteDlg();
			if( pDnPrivateChannelInviteDlg )
				pDnPrivateChannelInviteDlg->Show( true );
		}
		else if( IsCmdControl( "ID_BT_PASSWORD" ) )
		{
#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
			CDnPrivateChannelPasswordChangeDlg* pDnPrivateChannelPasswordChangeDlg = GetInterface().GetPrivateChannelPasswordChangeDlg();
			if( pDnPrivateChannelPasswordChangeDlg )
			{
				pDnPrivateChannelPasswordChangeDlg->Show( true );
			}
#else // PRE_ADD_PRIVATECHAT_CHANNEL
			CDnPrivateChannelPasswordDlg* pDnPrivateChannelPasswordDlg = GetInterface().GetPrivateChannelPasswordDlg();
			if( pDnPrivateChannelPasswordDlg )
			{
				pDnPrivateChannelPasswordDlg->SetChannelPasswordType( CDnPrivateChannelPasswordDlg::TYPE_PASSWORD_CHANGE );
				pDnPrivateChannelPasswordDlg->Show( true );
			}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL
		}
		else if( GetChannelChatTask().IsActive() && IsCmdControl( "ID_BT_CANCEL" ) )
		{
			GetChannelChatTask().SendChannelOut();
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			if(pMainMenuDlg)
			{
				CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
				if(pSystemDlg)
					pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_PRIVATE_CHAT);
			}
			
		}
#endif // PRE_ADD_SHORTCUT_HELP_DIALOG
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPrivateChannelDlg::RefreshChannelInfo()
{
	if( m_pDnPrivateChannelInfoDlg )
	{
		m_pDnPrivateChannelInfoDlg->RefreshChannelEntryList();
		
		if( GetChannelChatTask().IsActive() && GetChannelChatTask().IsChannelMaster() )
		{
			m_pButtonChannelInvite->Enable( true );
			m_pButtonChannelPassword->Enable( true );
		}
		else
		{
			m_pButtonChannelInvite->Enable( false );
			m_pButtonChannelPassword->Enable( false );
		}
	}
}

#ifdef PRE_ADD_PRIVATECHAT_CHANNEL
bool CDnPrivateChannelDlg::IsShowChannelInputDlg()
{
	bool bInputDlg = false;
	if( m_pDnPrivateChannelCreateDlg )
		bInputDlg = m_pDnPrivateChannelCreateDlg->IsShowChannelCreateJoinDlg();

	if( !bInputDlg )
	{
		CDnPrivateChannelPasswordChangeDlg* pDnPrivateChannelPasswordChangeDlg = GetInterface().GetPrivateChannelPasswordChangeDlg();
		if( pDnPrivateChannelPasswordChangeDlg )
		{
			bInputDlg = pDnPrivateChannelPasswordChangeDlg->IsShow();
		}
	}

	return bInputDlg;
}
#endif // PRE_ADD_PRIVATECHAT_CHANNEL

#endif // PRE_PRIVATECHAT_CHANNEL