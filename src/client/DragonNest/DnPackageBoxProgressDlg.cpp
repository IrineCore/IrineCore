#include "stdafx.h"
#include "DnPackageBoxProgressDlg.h"
#include "DnWorld.h"
#include "ItemSendPacket.h"
#include "DnInterface.h"
#include "DnTradeTask.h"
#include "DnTradeMail.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ADD_EASYGAMECASH )

CDnPackageBoxProgressDlg::CDnPackageBoxProgressDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pCancelButton(NULL)
, m_pProgressBarTime(NULL)
, m_pString( NULL )
, m_fTimer(0.f)
{
	m_cFlag = 0;
	m_cInvenType = ITEMPOSITION_INVEN;
	m_cInvenIndex = -1;
	m_biInvenSerial = 0;
}

CDnPackageBoxProgressDlg::~CDnPackageBoxProgressDlg(void)
{
}

void CDnPackageBoxProgressDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "RandomItemDlg.ui" ).c_str(), bShow );
}

void CDnPackageBoxProgressDlg::InitialUpdate()
{
	m_pCancelButton = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	m_pProgressBarTime = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR_TIME");
	m_pString = GetControl<CEtUIStatic>( "ID_TEXT" );

	m_pString->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4928 ) );	// UISTRING : Opening item . . .
}

void CDnPackageBoxProgressDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() ) return;
	switch( m_cFlag ) {
		case 1:
			{
				m_fTimer -= fElapsedTime;
				if( m_fTimer <= 0.f ) {
					m_fTimer = 0.f;
					m_pCancelButton->Enable( false );
					m_pProgressBarTime->Enable( false );
					m_cFlag = 0;
					SendCompleteCharmItem( m_cInvenType, m_cInvenIndex, m_biInvenSerial, 0, 0, 0 );

					Show( false );
				}
				m_pProgressBarTime->SetProgress( 100.f - ( 100.f / 2.f * m_fTimer ) );
			}
			break;
	}
}

void CDnPackageBoxProgressDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CANCEL" ) ) {
			SendCancelCharmItem();
		}
		Show(false);

		CDnTradeMail& tradeMail = GetTradeTask().GetTradeMail();
		if (tradeMail.IsOnMailMode())
			tradeMail.LockMailDlg(false);

		GetInterface().CloseMovieDlg();
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPackageBoxProgressDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnPackageBoxProgressDlg::ShowEx( bool bShow, char cInvenType, BYTE cInvenIndex/* = -1*/, INT64 biInvenSerial/* = 0*/, float fTimer/* = 0.f*/ )
{
	Show( bShow );

	if( bShow ) {
		m_cFlag = 1;
		m_fTimer = fTimer;

		m_pCancelButton->Show( true );
		m_pProgressBarTime->Show( true );

		m_pCancelButton->Enable( true );
		m_cInvenType = cInvenType;
		m_biInvenSerial = biInvenSerial;
		m_cInvenIndex = cInvenIndex;
	}
}

#endif	// #if defined( PRE_ADD_EASYGAMECASH )