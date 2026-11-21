#include "StdAfx.h"
#include "DnMiniPlayerGuildWarPopupDlg.h"
#include "DnInterface.h"
#include "DnWorld.h"
#include "PvPSendPacket.h"
#include "DnPartyTask.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnMiniPlayerGuildWarPopupDlg::CDnMiniPlayerGuildWarPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pButtonMaster(NULL)
, m_pButtonSubMaster(NULL)
, m_pButtonSubMasterOff(NULL)
, m_uiSessionID( 0 )
{
}

CDnMiniPlayerGuildWarPopupDlg::~CDnMiniPlayerGuildWarPopupDlg(void)
{
}

void CDnMiniPlayerGuildWarPopupDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MiniPlayerGuildWarPopupDlg.ui" ).c_str(), bShow );
}

void CDnMiniPlayerGuildWarPopupDlg::InitialUpdate()
{
	m_pButtonMaster = GetControl<CEtUIButton>("ID_BUTTON_MARSTER");
	m_pButtonMaster->Show(true);

	m_pButtonSubMaster = GetControl<CEtUIButton>("ID_BUTTON_SUBMARSTER");
	m_pButtonSubMaster->Show(true);

	m_pButtonSubMasterOff = GetControl<CEtUIButton>("ID_BUTTON_SUBMARSTEROFF");
	m_pButtonSubMasterOff->Show(true);
}


void CDnMiniPlayerGuildWarPopupDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_pButtonMaster->Enable(false);
		m_pButtonSubMaster->Enable(false);
		m_pButtonSubMasterOff->Enable(false);
	}

	CEtUIDialog::Show( bShow );
}

void CDnMiniPlayerGuildWarPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );


	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show(false);

		if( !IsExistActor() )
			return;

		if( IsCmdControl("ID_BUTTON_MARSTER")) 
		{
			SendChangeMemberGrade( PvPCommon::UserState::GuildWarCaptain, m_uiSessionID, true );
		}
		else if( IsCmdControl("ID_BUTTON_SUBMARSTER") )
		{
			SendChangeMemberGrade( PvPCommon::UserState::GuildWarSedcondCaptain, m_uiSessionID, true );
		}
		else if( IsCmdControl("ID_BUTTON_SUBMARSTEROFF") )
		{
			SendChangeMemberGrade( PvPCommon::UserState::GuildWarSedcondCaptain, m_uiSessionID, false );
		}
	}
}


void CDnMiniPlayerGuildWarPopupDlg::SetControl( UINT eMyState, UINT eTargetState, UINT nSessionID, bool bSecondary )
{
	//Disable all if not master or sub-master

	m_uiSessionID = nSessionID;

	if( PvPCommon::UserState::GuildWarCaptain & eMyState )	
	{
		//Target is Master
		if( PvPCommon::UserState::GuildWarCaptain & eTargetState )
		{
			m_pButtonMaster->Enable(false);
			m_pButtonSubMaster->Enable(false);
			m_pButtonSubMasterOff->Enable(false);
			m_pButtonSubMasterOff->Show( false );	// Disable all
		}
		else if( PvPCommon::UserState::GuildWarSedcondCaptain & eTargetState )	//If the target is Sub-Master
		{
			m_pButtonMaster->Enable(true);
			m_pButtonSubMasterOff->Show( true );
			m_pButtonSubMasterOff->Enable( true );	
			m_pButtonSubMaster->Show( false );	//Dismiss sub-master
		}
		else
		{
			m_pButtonMaster->Enable(true);
			m_pButtonSubMaster->Show( true );
			m_pButtonSubMaster->Enable(true);
			m_pButtonSubMasterOff->Show( false );	// Appoint master, sub-master

			if( !bSecondary )
				m_pButtonSubMaster->Enable(false);
		}
	}
	else
	{
		m_pButtonMaster->Enable(false);
		m_pButtonSubMaster->Enable(false);
		m_pButtonSubMasterOff->Enable(false);
		m_pButtonSubMasterOff->Show( false );	// Disable all
	}
}

void CDnMiniPlayerGuildWarPopupDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	switch( nID ) 
	{
	case MESSAGEBOX_FRIEND_QUERY:
		{
			if( nCommand == EVENT_BUTTON_CLICKED ) 
			{
				if (IsCmdControl("ID_YES"))
				{
					
				}
			}
		}
		break;
	}
}

void CDnMiniPlayerGuildWarPopupDlg::Process( float fElapsedTime )
{
	if( IsShow() && !IsExistActor() )
		Show( false );

	CEtUIDialog::Process( fElapsedTime );
}

bool CDnMiniPlayerGuildWarPopupDlg::IsExistActor()
{
	CDnPartyTask::PartyStruct * pInfo = GetPartyTask().GetPartyDataFromSessionID( m_uiSessionID, true );

	if( pInfo )
		return true;

	return false;
}