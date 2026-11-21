#include "StdAfx.h"
#include "DnDungeonEnterNotifierDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnDungeonEnterNotifierDlg::CDnDungeonEnterNotifierDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticTitle( NULL )
	, m_pStaticDesc( NULL )
{
}

CDnDungeonEnterNotifierDlg::~CDnDungeonEnterNotifierDlg()
{
}

void CDnDungeonEnterNotifierDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterNoticeDlg.ui" ).c_str(), bShow );
}

void CDnDungeonEnterNotifierDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_TEXT_TITLE" );
	m_pStaticDesc = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
}

void CDnDungeonEnterNotifierDlg::SetDungeonNotice( const WCHAR *wszTitle, const WCHAR *wszDesc )
{
	if( m_pStaticTitle && wszTitle )
		m_pStaticTitle->SetText( wszTitle );
	if( m_pStaticDesc && wszDesc )
		m_pStaticDesc->SetText( wszDesc );
}

void CDnDungeonEnterNotifierDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CANCEL" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
