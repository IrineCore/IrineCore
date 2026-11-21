#include "StdAfx.h"
#include "DnJobInformationDlg.h"
#include "DnJobIconStatic.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnInterfaceString.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnJobInformationMessageBox
//-----------------------------------------------------------------------------
CDnJobInformationMessageBox::CDnJobInformationMessageBox( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticClass( NULL )
	, m_pStaticAsk( NULL )
{
}

CDnJobInformationMessageBox::~CDnJobInformationMessageBox()
{
}

void CDnJobInformationMessageBox::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassInfoMessageDlg.ui" ).c_str(), bShow );
}

void CDnJobInformationMessageBox::InitialUpdate()
{
	m_pStaticClass = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
	m_pStaticAsk = GetControl<CEtUIStatic>( "ID_TEXT_ASK" );
}

void CDnJobInformationMessageBox::SetJobMessage( int nJobID, const WCHAR *wszMessage )
{
	if( m_pStaticClass && nJobID > 0 )
		m_pStaticClass->SetIconID( nJobID );
	if( m_pStaticAsk && wszMessage )
		m_pStaticAsk->SetText( wszMessage );
}

void CDnJobInformationMessageBox::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnJobInformationPopupDlg
//-----------------------------------------------------------------------------
CDnJobInformationPopupDlg::CDnJobInformationPopupDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pProgressExp( NULL )
	, m_pStaticLv( NULL )
	, m_pStaticNotice( NULL )
{
}

CDnJobInformationPopupDlg::~CDnJobInformationPopupDlg()
{
}

void CDnJobInformationPopupDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassGaugePopUpDlg.ui" ).c_str(), bShow );
}

void CDnJobInformationPopupDlg::InitialUpdate()
{
	m_pProgressExp = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR_EXP" );
	m_pStaticLv = GetControl<CEtUIStatic>( "ID_TEXT_LV" );
	m_pStaticNotice = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
}

void CDnJobInformationPopupDlg::SetPopupInfo( int nLevel, float fExpPercent, const WCHAR *wszNotice )
{
	if( m_pProgressExp )
		m_pProgressExp->SetProgress( fExpPercent );
	if( m_pStaticLv )
	{
		WCHAR wszBuf[32] = { 0 };
		swprintf_s( wszBuf, _countof( wszBuf ), L"Lv.%d", nLevel );
		m_pStaticLv->SetText( wszBuf );
	}
	if( m_pStaticNotice && wszNotice )
		m_pStaticNotice->SetText( wszNotice );
}

void CDnJobInformationPopupDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnJobInformationMainDlg
//-----------------------------------------------------------------------------
CDnJobInformationMainDlg::CDnJobInformationMainDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnAddClass( NULL )
	, m_pBtnSkill( NULL )
	, m_pStaticClassIcon( NULL )
	, m_pTextureBg( NULL )
	, m_pTextureClass( NULL )
	, m_pStaticClassName( NULL )
	, m_pStaticInfo( NULL )
	, m_pStaticSkillInfo( NULL )
	, m_pMessageBox( NULL )
	, m_pPopupDlg( NULL )
	, m_nSelectedJobID( 0 )
{
	for( int i = 0; i < MAX_JOB_RADIO; ++i )
		m_pRadioJobs[i] = NULL;
}

CDnJobInformationMainDlg::~CDnJobInformationMainDlg()
{
	SAFE_DELETE( m_pMessageBox );
	SAFE_DELETE( m_pPopupDlg );
}

void CDnJobInformationMainDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ClassInfoDlg.ui" ).c_str(), bShow );

	m_pMessageBox = new CDnJobInformationMessageBox( UI_TYPE_MODAL );
	if( m_pMessageBox )
		m_pMessageBox->Initialize( false );

	m_pPopupDlg = new CDnJobInformationPopupDlg( UI_TYPE_FOCUS );
	if( m_pPopupDlg )
		m_pPopupDlg->Initialize( false );
}

void CDnJobInformationMainDlg::InitialUpdate()
{
	m_pBtnAddClass = GetControl<CEtUIButton>( "ID_BT_ADDCLASS" );
	m_pBtnSkill = GetControl<CEtUIButton>( "ID_BT_SKILL" );
	m_pStaticClassIcon = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
	m_pTextureBg = GetControl<CEtUITextureControl>( "ID_TEXTUREL_BG" );
	m_pTextureClass = GetControl<CEtUITextureControl>( "ID_TEXTUREL_CLASS" );
	m_pStaticClassName = GetControl<CEtUIStatic>( "ID_TEXT_CLASS" );
	m_pStaticInfo = GetControl<CEtUIStatic>( "ID_TEXT_INFO" );
	m_pStaticSkillInfo = GetControl<CEtUIStatic>( "ID_TEXT_SKILLINFO" );

	char szName[32] = { 0 };
	for( int i = 0; i < MAX_JOB_RADIO; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_RBT_%d", i );
		m_pRadioJobs[i] = GetControl<CEtUIRadioButton>( szName );
	}
}

void CDnJobInformationMainDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
}

void CDnJobInformationMainDlg::SetJobInfo( int nJobID )
{
	m_nSelectedJobID = nJobID;
	if( m_pStaticClassIcon && nJobID > 0 )
		m_pStaticClassIcon->SetIconID( nJobID );
	if( m_pStaticClassName && nJobID > 0 )
		m_pStaticClassName->SetText( DN_INTERFACE::STRING::GetJobString( nJobID ) );
}

void CDnJobInformationMainDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
