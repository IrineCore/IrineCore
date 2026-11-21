#include "StdAfx.h"
#include "DnMovieDlg.h"
#include "DnWorld.h"
#include "ItemSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnMovieDlg::CDnMovieDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
, m_pMovieControl(NULL)
{
	m_fTimer = 0.f;
}

CDnMovieDlg::~CDnMovieDlg(void)
{
}

void CDnMovieDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CustomMovie.ui" ).c_str(), bShow );
}

void CDnMovieDlg::InitialUpdate()
{
}

void CDnMovieDlg::InitCustomControl( CEtUIControl *pControl )
{
	// If only doing static_cast<classtype*>(GetControl like this, doing it in InitialUpdate is enough.
	m_pMovieControl = GetControl<CDnMovieControl>("ID_CUSTOM_MOVIE");
}

void CDnMovieDlg::Process( float fElapsedTime )
{
	if( m_fTimer != -1.f ) {
		m_fTimer -= fElapsedTime;
		if( m_fTimer <= 0.f ) Show( false );
	}
	CDnCustomDlg::Process( fElapsedTime );
}

void CDnMovieDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMovieDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnMovieDlg::PlayMovie( const char *szFileName, float fTimer )
{
	m_pMovieControl->Play( szFileName );
	m_fTimer = fTimer;
}