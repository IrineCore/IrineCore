#include "StdAfx.h"
#include "DnPvPKilledMeDlg.h"
#include "DnActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPvPKilledMeDlg::CDnPvPKilledMeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
{
	m_pStaticEnemy = NULL;
	m_pStaticFriendly = NULL;
	m_pStaticEnemyBG = NULL;
	m_pStaticFriendlyBG = NULL;
	m_pStaticKilledBy = NULL;
	m_pStaticName = NULL;
	m_pStaticCount = NULL;

	m_fDelta = 0.f;
}

CDnPvPKilledMeDlg::~CDnPvPKilledMeDlg()
{
}

void CDnPvPKilledMeDlg::InitialUpdate()
{
	m_pStaticEnemy = GetControl<CEtUIStatic>("ID_TEXT_RED");
	m_pStaticFriendly = GetControl<CEtUIStatic>("ID_TEXT_BLUE");
	m_pStaticEnemyBG = GetControl<CEtUIStatic>("ID_STATIC_RED");
	m_pStaticFriendlyBG = GetControl<CEtUIStatic>("ID_STATIC_BLUE");
	m_pStaticKilledBy = GetControl<CEtUIStatic>("ID_TEST_KILL");
	m_pStaticName = GetControl<CEtUIStatic>("ID_UserID");
	m_pStaticCount = GetControl<CEtUIStatic>("ID_TEST_KILLCOUMT");

	// Hide since there is no plan to view enemy camera for now
	m_pStaticFriendlyBG->Show( false );
	m_pStaticFriendly->Show( false );
}

void CDnPvPKilledMeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvPViewmode.ui" ).c_str(), bShow );
}

void CDnPvPKilledMeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPvPKilledMeDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	if( bShow )
		m_fDelta = 3.f;
	else m_fDelta = 0.f;

	CEtUIDialog::Show( bShow );

}

void CDnPvPKilledMeDlg::Process( float fElapsedTime )
{
	if( m_fDelta > 0.f ) {
		m_fDelta -= fElapsedTime;
		if( m_fDelta <= 0.f ) {
			m_pStaticName->SetText( L"" );
			m_pStaticCount->SetText( L"" );
			Show( false );
		}
	}
	CEtUIDialog::Process( fElapsedTime );
}

void CDnPvPKilledMeDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );
}

void CDnPvPKilledMeDlg::Refresh( WCHAR *wszName, int nKillCount )
{
	WCHAR wszStr[128];
	m_pStaticName->SetText( wszName );
	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121056 ), nKillCount );
	m_pStaticCount->SetText( wszStr );

}