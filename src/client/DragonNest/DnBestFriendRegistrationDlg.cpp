#include "Stdafx.h"

#ifdef PRE_ADD_BESTFRIEND

#include "DnItemTask.h"

#include "DnBestFriendRegistrationDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnBFRegistrationDlg::CDnBFRegistrationDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
 : CEtUIDialog( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
 , m_IMEBfFriend( NULL )
{
	m_nSerial = 0;
}

void CDnBFRegistrationDlg::ReleaseDlg()
{

}


void CDnBFRegistrationDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("BFAddDlg.ui").c_str(), bShow );
}


void CDnBFRegistrationDlg::InitialUpdate()
{
	m_IMEBfFriend = GetControl<CEtUIIMEEditBox>( "ID_IMEEDITBOX_NAME" );
	
}


void CDnBFRegistrationDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_CANCEL") || IsCmdControl("ID_BT_CANCEL") )
		{
			Show( false );
		}

		else if( IsCmdControl("ID_BT_OK") )
		{
			RequestBF(); // Best friend registration request.
			Show( false );
		}
	}



	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}


void CDnBFRegistrationDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		RequestFocus( m_IMEBfFriend );
	}
	else
	{		
		m_IMEBfFriend->ClearText();
	}
}


// Best friend registration request.
void CDnBFRegistrationDlg::RequestBF()
{
	std::wstring str = m_IMEBfFriend->GetText();
	if( !str.empty() || 
		!(str.size() < 1) )
	{
		GetItemTask().RequestSearchBF( str );
	}
	
}


// Best friend certificate serial.
void CDnBFRegistrationDlg::SetSerial( INT64 serial )
{
	m_nSerial = serial;
}


#endif