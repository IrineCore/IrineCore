#include "StdAfx.h"
#include "DnPartyMenuDlg.h"
#include "DnPartyTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPartyMenuDlg::CDnPartyMenuDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
	, m_pButtonGive(NULL)
	, m_pButtonOut(NULL)
	, m_pButtonPrivate(NULL)
	, m_pButtonFriend(NULL)
	, m_pButtonGuild(NULL)
{
}

CDnPartyMenuDlg::~CDnPartyMenuDlg(void)
{
}

void CDnPartyMenuDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyMenuDlg.ui" ).c_str(), bShow );
}

void CDnPartyMenuDlg::InitialUpdate()
{
	m_pButtonGive = GetControl<CEtUIButton>("ID_GIVE");
	m_pButtonOut = GetControl<CEtUIButton>("ID_OUT");
	m_pButtonPrivate = GetControl<CEtUIButton>("ID_PRIVATE");
	m_pButtonFriend = GetControl<CEtUIButton>("ID_FRIEND");
	m_pButtonGuild = GetControl<CEtUIButton>("ID_GUILD");
	m_pButtonGive->Enable(false);
	m_pButtonOut->Enable(false);
	m_pButtonGuild->Enable(false);
}

void CDnPartyMenuDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( GetPartyTask().GetPartyRole() == CDnPartyTask::LEADER )
		{
			m_pButtonGive->Enable(true);
			m_pButtonOut->Enable(true);
		}
		else
		{
			m_pButtonGive->Enable(false);
			m_pButtonOut->Enable(false);
		}

		if( false/*If joined a guild*/ )
		{
			m_pButtonGuild->Enable(true);
		}
		else
		{
			m_pButtonGuild->Enable(false);
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnPartyMenuDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CLOSE" ) )
		{
			Show(false);
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}