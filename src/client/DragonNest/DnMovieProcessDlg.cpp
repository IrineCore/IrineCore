#include "StdAfx.h"
#include "DnMovieProcessDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnMovieProcessDlg::CDnMovieProcessDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pStatic(NULL)
, m_pProcessComplete(NULL)
, m_pCancelButton(NULL)
, m_pProgressBarTime(NULL)
, m_fMaxTimer(0.0f)
, m_fTimer(0.f)
, m_cFlag(0)
{
}

CDnMovieProcessDlg::~CDnMovieProcessDlg(void)
{
}

void CDnMovieProcessDlg::Initialize( bool bShow )
{
	// Basically only has buttons and a progress bar; movie control uses DnInterface's shared dialog since it is display-only.
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MovieProcessDlg.ui" ).c_str(), bShow );
}

void CDnMovieProcessDlg::InitialUpdate()
{
	m_pStatic = GetControl<CEtUIStatic>("ID_STATIC0");
	m_pProcessComplete = GetControl<CEtUIButton>("ID_PROCESS_COMPLETE");
	m_pCancelButton = GetControl<CEtUIButton>("ID_BUTTON_CANCEL");
	m_pProgressBarTime = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR_TIME");

	m_pProcessComplete->Show( false );
}

void CDnMovieProcessDlg::Show( bool bShow )
{
	if( bShow == m_bShow )
		return;

	if( !bShow )
	{
		// Normally Show(false) is only called when cancel is pressed, so m_cFlag will be 0.
		// However, while in a party and the progress bar is filling, if the party leader changes channel or similar,
		// (i.e., when the window is forcibly closed abnormally)
		// Check the flag and call force cancel.
		if( m_cFlag == 1 )
			m_pCallback->OnUICallbackProc( m_nDialogID, EVENT_BUTTON_CLICKED, m_pCancelButton, 0 );

		SetDialogID(-1);
		SetCallback(NULL);
		m_cFlag = 0;
		m_fTimer = 0.0f;
		m_pProgressBarTime->SetProgress( 0.0f );
	}

	CEtUIDialog::Show( bShow );
}

void CDnMovieProcessDlg::Process( float fElapsedTime )
{
	switch( m_cFlag )
	{
	case 1:
		{
			m_fTimer -= fElapsedTime;
			if( m_fTimer <= 0.f )
			{
				m_pCancelButton->Enable( false );
				m_pProgressBarTime->Enable( false );
				// Notify the upper layer of process termination this way.
				m_pCallback->OnUICallbackProc( m_nDialogID, EVENT_BUTTON_CLICKED, m_pProcessComplete, 0 );
				m_cFlag = 0;
				Show( false );
			}
			m_pProgressBarTime->SetProgress( (1.0f - (m_fTimer / m_fMaxTimer)) * 100.0f );
		}
		break;
	}

	CEtUIDialog::Process( fElapsedTime );
}

void CDnMovieProcessDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	// When the cancel button is pressed, the client sends a cancel packet.
	// If the progress bar timer expires and sends completion before a response to this packet arrives, it would send the wrong packet, so
	// Turn off processing flag.
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BUTTON_CANCEL" ) )
		{
			m_cFlag = 0;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMovieProcessDlg::SetInfo( LPCWSTR wszMessage, float fTimer, int nID, CEtUICallback *pCall, bool bShowButton )
{
	// Calling SetInfo is treated as initializing and starting the process.
	m_cFlag = 1;

	m_pStatic->SetText( wszMessage );
	m_fMaxTimer = m_fTimer = fTimer;

	m_nDialogID = nID;
	m_pCallback = pCall;

	m_pCancelButton->Show( bShowButton );
	m_pCancelButton->Enable( true );
	m_pProgressBarTime->Show( true );
}