#include "StdAfx.h"
#include "DnSkillClassInfoDlg.h"
#include "DnJobInformationDlg.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSkillClassInfoDlg::CDnSkillClassInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pBtnInfo( NULL )
	, m_pTextBox( NULL )
{
}

CDnSkillClassInfoDlg::~CDnSkillClassInfoDlg()
{
}

void CDnSkillClassInfoDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "SkillTreeClassInfoDlg.ui" ).c_str(), bShow );
}

void CDnSkillClassInfoDlg::InitialUpdate()
{
	m_pBtnInfo = GetControl<CEtUIButton>( "ID_BT_INFO" );
	m_pTextBox = GetControl<CEtUITextBox>( "ID_TEXTBOX" );
}

void CDnSkillClassInfoDlg::SetClassInfoText( const WCHAR *wszText )
{
	if( m_pTextBox )
	{
		m_pTextBox->ClearText();
		if( wszText )
			m_pTextBox->AddText( wszText );
	}
}

void CDnSkillClassInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_INFO" ) )
		{
			if( GetInterface().GetJobInformationMainDlg() )
				GetInterface().GetJobInformationMainDlg()->Show( true );
			return;
		}
		else if( IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
