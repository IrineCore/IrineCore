#include "StdAfx.h"
#include "DnLoginDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnMessageBox.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterfaceDlgID.h"
#include "DnInterface.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnLoginDlg::CDnLoginDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_bCallbackProcessed( false )
	, m_pEditBoxID( NULL )
	, m_pEditBoxPW( NULL )
{
}

CDnLoginDlg::~CDnLoginDlg(void)
{
}

void CDnLoginDlg::Initialize( bool bShow )
{	
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "login.ui" ).c_str(), bShow );
}

void CDnLoginDlg::InitialUpdate()
{
	m_pEditBoxID = GetControl<CEtUIEditBox>( "ID_LOGIN_USERID" );
	m_pEditBoxPW = GetControl<CEtUIEditBox>( "ID_LOGIN_PASSWORD" );
	GetControl<CEtUIButton>( "ID_BUTTON_IDSOFTKEY" )->Show( false );
	GetControl<CEtUIButton>( "ID_BUTTON_PASSSOFTKEY")->Show( false );




	m_bCallbackProcessed = false;
}

void CDnLoginDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_LOGIN_OK" ) )
		{
			CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
			if( !pTask ) {
				return;
			}


			if( m_pEditBoxID->GetTextLength() == 0 ) 
			{
				GetInterface().MessageBox( MESSAGEBOX_1, MB_OK, MESSAGEBOX_1, this );
				return;
			}

			if( m_pEditBoxPW->GetTextLength() == 0 ) 
			{
				GetInterface().MessageBox( MESSAGEBOX_2, MB_OK, MESSAGEBOX_2, this );
				return;
			}

			// Moved LoginButton above RequestLogin to match order in StandAlone mode by kwcsc
			EnableLoginButton( false );

#ifdef PRE_ADD_RELOGIN_PROCESS
			pTask->m_strConnectID = m_pEditBoxID->GetText();
			pTask->m_strConnectPassword = m_pEditBoxPW->GetText();
#endif // PRE_ADD_RELOGIN_PROCESS
			pTask->RequestLogin( m_pEditBoxID->GetText(), m_pEditBoxPW->GetText() );

			return;
		}
	
		if( IsCmdControl( "ID_LOGIN_CANCEL" ) ) 
		{
			CTask *pTask = CTaskManager::GetInstance().GetTask( "LoginTask" );
			if( pTask ) pTask->OnCloseTask();
			return;
		}
	}
	// Temporary enter handling
	else if( nCommand == EVENT_EDITBOX_STRING )
	{
		if( m_bCallbackProcessed )
		{
			// When Enter is pressed while the message box is open
			// WM_KEYDOWN message comes once, followed by WM_CHAR-VKRETURN (Notepad also receives messages like this.)
			// There is a slight issue here...
			//
			// The first WM_KEYDOWN is bound to the message box MB_OK button hotkey and treated as pressing that hotkey button,
			// Since a callback is connected here, calling the callback below will move focus to the edit box.
			// Afterward, the message box's Show becomes false and it becomes invisible.
			// (Here, smart-move causes a WM_MOUSEMOVE message to loop through the dialog list once, which ends without major impact so it doesn't matter.)
			//
			// And it should just finish after that, but an extra WM_CHAR-VKRETURN arrives here, causing the problem.
			// Since focus is on the edit box, EVENT_EDITBOX_STRING is executed due to VK_RETURN,
			// Because of this, focus is lost again and CEtUIDialog::ProcessCommand below is called
			// The default button executes, and eventually the message box pops up again.
			//
			// That is, even if Enter is pressed while the message box is open, it stays open (internally closing and reappearing).
			//
			// To prevent this, ignore the VK_RETURN immediately following the UICallback call.
			// Is there a cleaner way...
			//
			// [Resolved]
			// Since Inseon suggested changing it to ignore WM_CHAR-VKRETURN itself,
			// Commented out WM_CHAR-VKRETURN handling in EtUIEditBox::MsgProc and changed to handle it in WM_KEYDOWN.
			m_bCallbackProcessed = false;
			return;
		}
		else
		{
			// Must clear focus from the edit box so hotkeys work on the invalid ID/password message box.
			focus::ReleaseControl();
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnLoginDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );


	if( bShow )
	{
		m_pEditBoxPW->Focus(false);

		// RequestFocus must be called after Show like this so that after dialog Push/Pop completes
		// Only changes the current focus control.
		// Come to think of it, this is really inconvenient. Making something that requires a strict call order like this...
		RequestFocus(m_pEditBoxID);
	}
	else
	{
		// Completely clear the contents of CEtUIEditBox.
		m_pEditBoxID->ClearTextMemory();
		m_pEditBoxPW->ClearTextMemory();

		EnableLoginButton( true );
	}
}

void CDnLoginDlg::SetId(WCHAR *Id)
{
	if(m_pEditBoxID)
	{
		m_pEditBoxID->SetText(Id);
	}
}

void CDnLoginDlg::SetPw(WCHAR *Pw)
{
	if(m_pEditBoxPW)
	{
		m_pEditBoxPW->SetText(Pw);
	}
}

int CDnLoginDlg::GetMaxCharID()
{
	if( m_pEditBoxPW )
	{
		return m_pEditBoxID->GetMaxChar();
	}

	return 0;
}

int CDnLoginDlg::GetMaxCharPW()
{
	if( m_pEditBoxPW)
	{
		return m_pEditBoxPW->GetMaxChar();
	}
	return 0;
}

void CDnLoginDlg::EnableLoginButton( bool bEnable )
{
	GetControl<CEtUIButton>("ID_LOGIN_OK")->Enable( bEnable );
}

void CDnLoginDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case MESSAGEBOX_1:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( IsCmdControl("ID_OK") )
				{
					RequestFocus(m_pEditBoxID);
					//m_bCallbackProcessed = true;
					// No longer needed now that the solution above is in place, but leave the code as is for now.
					// Because it was another solution.
				}
			}
		}
		break;
	case MESSAGEBOX_2:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if( IsCmdControl("ID_OK") )
				{
					RequestFocus(m_pEditBoxPW);
					//m_bCallbackProcessed = true;
				}
			}
		}
		break;
	}
}

void CDnLoginDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( IsShow() )
	{
	}
}

