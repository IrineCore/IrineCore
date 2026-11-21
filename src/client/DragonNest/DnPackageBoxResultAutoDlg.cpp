#include "StdAfx.h"
#include "DnPackageBoxResultAutoDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPackageBoxResultAutoDlg::CDnPackageBoxResultAutoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticTitle( NULL )
	, m_pProgressBar( NULL )
	, m_fDuration( 1.5f )
	, m_fElapsed( 0.0f )
{
}

CDnPackageBoxResultAutoDlg::~CDnPackageBoxResultAutoDlg()
{
}

void CDnPackageBoxResultAutoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PackageAutoBox.ui" ).c_str(), bShow );
}

void CDnPackageBoxResultAutoDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
	m_pProgressBar = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR1" );
}

void CDnPackageBoxResultAutoDlg::StartAutoOpen( const WCHAR *wszTitle, float fDuration )
{
	m_fDuration = ( fDuration > 0.1f ) ? fDuration : 1.5f;
	m_fElapsed = 0.0f;
	if( m_pStaticTitle && wszTitle )
		m_pStaticTitle->SetText( wszTitle );
	if( m_pProgressBar )
		m_pProgressBar->SetProgress( 0.0f );
	Show( true );
}

void CDnPackageBoxResultAutoDlg::Show( bool bShow )
{
	if( !bShow )
		m_fElapsed = 0.0f;
	CEtUIDialog::Show( bShow );
}

void CDnPackageBoxResultAutoDlg::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( !IsShow() )
		return;

	m_fElapsed += fElapsedTime;
	float fPct = ( m_fDuration > 0.0f ) ? ( m_fElapsed / m_fDuration ) * 100.0f : 100.0f;
	if( fPct >= 100.0f )
	{
		if( m_pProgressBar ) m_pProgressBar->SetProgress( 100.0f );
		Show( false );
		return;
	}
	if( m_pProgressBar )
		m_pProgressBar->SetProgress( fPct );
}

void CDnPackageBoxResultAutoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
