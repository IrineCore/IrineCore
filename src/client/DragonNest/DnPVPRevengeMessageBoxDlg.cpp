#include "stdafx.h"
#include "DnPVPRevengeMessageBoxDlg.h"
#include "PvPSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPRevengeMessageBoxDlg::CDnPVPRevengeMessageBoxDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pInviteStatic( NULL )
, m_pMessageStatic( NULL )
{

}

CDnPVPRevengeMessageBoxDlg::~CDnPVPRevengeMessageBoxDlg()
{

}

void CDnPVPRevengeMessageBoxDlg::InitialUpdate()
{
	m_pInviteStatic = GetControl<CEtUIStatic>( "ID_TEXT0" );
	m_pMessageStatic = GetControl<CEtUIStatic>( "ID_TEXT1" );
}

void CDnPVPRevengeMessageBoxDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpRevengeMessageBoxDlg.ui" ).c_str(), bShow );
}

void CDnPVPRevengeMessageBoxDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_OK" ) )
		{
		
		}
		else if( IsCmdControl("ID_CANCEL" ) )
		{
			//SendRequestPvP_RevengeInviteDenied( 
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPVPRevengeMessageBoxDlg::SetMessageBox( const std::wstring szRevengeRequestUser )
{
	WCHAR wszString[512] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7845 ), szRevengeRequestUser.c_str() );	// UISTRING : %s invited you to the room.
	m_pInviteStatic->SetText( wszString );

	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7846 ), szRevengeRequestUser.c_str() );	// UISTRING : %s says, \"Let's fight!!
	m_pMessageStatic->SetText( wszString );
}