#include "StdAfx.h"
#include "DnQuestNotifierListItemDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnQuestNotifierListItemDlg::CDnQuestNotifierListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pBtnClose( NULL )
	, m_pStaticMainIcon( NULL )
	, m_pStaticMainTitle( NULL )
	, m_pTextBoxMain( NULL )
	, m_pStaticArrowBase( NULL )
	, m_pStaticBoard( NULL )
	, m_pStaticOver( NULL )
	, m_pStaticSubIcon( NULL )
	, m_pStaticSubTitle( NULL )
	, m_nQuestIndex( -1 )
{
}

CDnQuestNotifierListItemDlg::~CDnQuestNotifierListItemDlg()
{
}

void CDnQuestNotifierListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSumInfoListDlg.ui" ).c_str(), bShow );
}

void CDnQuestNotifierListItemDlg::InitialUpdate()
{
	m_pBtnClose = GetControl<CEtUIButton>( "ID_BT_MAIN_CLOSE" );
	m_pStaticMainIcon = GetControl<CEtUIStatic>( "ID_MAIN_ICON" );
	m_pStaticMainTitle = GetControl<CEtUIStatic>( "ID_MAIN_TITLE" );
	m_pTextBoxMain = GetControl<CEtUITextBox>( "ID_MAIN_TEXTBOX" );
	m_pStaticArrowBase = GetControl<CEtUIStatic>( "ID_MAIN_ARROW_BASE" );
	m_pStaticBoard = GetControl<CEtUIStatic>( "ID_STATIC_BOARD0" );
	m_pStaticOver = GetControl<CEtUIStatic>( "ID_STATIC_OVER0" );
	m_pStaticSubIcon = GetControl<CEtUIStatic>( "ID_SUB0_ICON" );
	m_pStaticSubTitle = GetControl<CEtUIStatic>( "ID_SUB0_TITLE" );
}

void CDnQuestNotifierListItemDlg::SetQuestSummary( int nQuestIndex, const WCHAR *wszTitle, const WCHAR *wszDesc, bool bMainQuest )
{
	m_nQuestIndex = nQuestIndex;
	if( m_pStaticMainIcon ) m_pStaticMainIcon->Show( bMainQuest );
	if( m_pStaticSubIcon ) m_pStaticSubIcon->Show( !bMainQuest );

	if( bMainQuest )
	{
		if( m_pStaticMainTitle ) { m_pStaticMainTitle->SetText( wszTitle ? wszTitle : L"" ); m_pStaticMainTitle->Show( true ); }
		if( m_pStaticSubTitle ) m_pStaticSubTitle->Show( false );
	}
	else
	{
		if( m_pStaticSubTitle ) { m_pStaticSubTitle->SetText( wszTitle ? wszTitle : L"" ); m_pStaticSubTitle->Show( true ); }
		if( m_pStaticMainTitle ) m_pStaticMainTitle->Show( false );
	}

	if( m_pTextBoxMain )
	{
		m_pTextBoxMain->ClearText();
		if( wszDesc )
			m_pTextBoxMain->AddText( wszDesc );
	}
}

void CDnQuestNotifierListItemDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && IsCmdControl( "ID_BT_MAIN_CLOSE" ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
