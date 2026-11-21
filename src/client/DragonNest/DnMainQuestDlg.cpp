#include "StdAfx.h"
#include "DnMainQuestDlg.h"
#include "DnMainQuestAnswerDlg.h"
#include "DnMainQuestRewardDlg.h"
#include "DnInterface.h"
#include "TaskManager.h"
#include "DnCommonTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMainQuestDlg::CDnMainQuestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticBack( NULL )
	, m_pStatic0( NULL )
	, m_pStaticNpcName( NULL )
	, m_pTextBoxMain( NULL )
	, m_pBtnClose( NULL )
	, m_pBtnReturn( NULL )
	, m_pAnswerDlg( NULL )
	, m_pRewardDlg( NULL )
{
}

CDnMainQuestDlg::~CDnMainQuestDlg()
{
	SAFE_DELETE( m_pAnswerDlg );
	SAFE_DELETE( m_pRewardDlg );
}

void CDnMainQuestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "NpcBTDlg.ui" ).c_str(), bShow );

	m_pAnswerDlg = new CDnMainQuestAnswerDlg( UI_TYPE_CHILD, this );
	if( m_pAnswerDlg )
		m_pAnswerDlg->Initialize( false );

	m_pRewardDlg = new CDnMainQuestRewardDlg( UI_TYPE_CHILD, this );
	if( m_pRewardDlg )
		m_pRewardDlg->Initialize( false );
}

void CDnMainQuestDlg::InitialUpdate()
{
	m_pStaticBack = GetControl<CEtUIStatic>( "ID_BACK" );
	m_pStatic0 = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pStaticNpcName = GetControl<CEtUIStatic>( "ID_STATIC_NAME" );
	m_pTextBoxMain = GetControl<CEtUITextBox>( "ID_TEXTBOX_MAIN" );
	m_pBtnClose = GetControl<CEtUIButton>( "ID_BT_CLOSE" );
	m_pBtnReturn = GetControl<CEtUIButton>( "ID_BUTTON_RETURN" );
}

void CDnMainQuestDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		if( m_pAnswerDlg ) m_pAnswerDlg->Show( false );
		if( m_pRewardDlg ) m_pRewardDlg->Show( false );
	}

	CEtUIDialog::Show( bShow );
}

void CDnMainQuestDlg::SetNpcTalkText( const WCHAR *wszNpcName, const WCHAR *wszContent )
{
	if( m_pStaticNpcName )
		m_pStaticNpcName->SetText( wszNpcName ? wszNpcName : L"" );
	if( m_pTextBoxMain )
	{
		m_pTextBoxMain->ClearText();
		if( wszContent )
			m_pTextBoxMain->AddText( wszContent );
	}
}

void CDnMainQuestDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BUTTON_RETURN" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
