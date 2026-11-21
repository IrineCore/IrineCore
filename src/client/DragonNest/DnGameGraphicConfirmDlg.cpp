#include "StdAfx.h"
#include "DnGameGraphicConfirmDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnGameGraphicConfirmDlg::CDnGameGraphicConfirmDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
, m_fElapsedTime( 0.0f )
{
}

CDnGameGraphicConfirmDlg::~CDnGameGraphicConfirmDlg(void)
{
}

void CDnGameGraphicConfirmDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "GameGraphicConfirmDlg.ui" ).c_str(), bShow );
}

void CDnGameGraphicConfirmDlg::InitialUpdate()
{
	GetControl<CEtUIButton>("ID_BUTTON_CLOSE")->Enable( false );
	m_SmartMove.SetControl( GetControl<CEtUIButton>("ID_YES") );
}

void CDnGameGraphicConfirmDlg::Show( bool bShow ) 
{ 
	CEtUIDialog::Show( bShow );

	if( bShow )
	{
		m_fElapsedTime = (float)CONFIRM_TOTAL_TIME;
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
	}
}

void CDnGameGraphicConfirmDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( m_fElapsedTime <= 0.0f )
	{
		ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_NO"), 0 );
	}
	else
	{
		// Unlike AcceptRequest dialog, there is no overlap handling at all, so it updates elapsed time on its own.
		m_fElapsedTime -= fElapsedTime;

		int nTime = (int)m_fElapsedTime;
		nTime += 1;	// To hide when 0 seconds remain.

		WCHAR wszTemp[256];
		swprintf_s( wszTemp, _countof(wszTemp), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 3059 ), nTime );
		GetControl<CEtUIStatic>("ID_MESSAGE")->SetText( wszTemp );
	}
}

void CDnGameGraphicConfirmDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		Show(false);
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}