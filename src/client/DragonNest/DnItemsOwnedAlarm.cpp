#include "StdAfx.h"
#include "DnItemsOwnedAlarm.h"
#include "DnTableDB.h"
#include "DNTableFile.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnAlarmMarkButton
//-----------------------------------------------------------------------------
CDnAlarmMarkButton::CDnAlarmMarkButton( CEtUIDialog *pParent )
	: CEtUIButton( pParent )
	, m_bBlink( false )
	, m_fBlinkTime( 0.0f )
{
}

CDnAlarmMarkButton::~CDnAlarmMarkButton()
{
}

void CDnAlarmMarkButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );
}

void CDnAlarmMarkButton::Process( float fElapsedTime )
{
	CEtUIButton::Process( fElapsedTime );
	if( m_bBlink )
		m_fBlinkTime += fElapsedTime;
}

//-----------------------------------------------------------------------------
// CDnItemsOwnedAlarm
//-----------------------------------------------------------------------------
CDnItemsOwnedAlarm::CDnItemsOwnedAlarm( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStatic0( NULL )
	, m_pTextBoxTitle( NULL )
	, m_pTextBoxSubTitle( NULL )
	, m_fAutoHideTime( 0.0f )
{
}

CDnItemsOwnedAlarm::~CDnItemsOwnedAlarm()
{
}

void CDnItemsOwnedAlarm::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "VIPAlarmDlg.ui" ).c_str(), bShow );
}

void CDnItemsOwnedAlarm::InitialUpdate()
{
	m_pStatic0 = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pTextBoxTitle = GetControl<CEtUITextBox>( "ID_TEXTBOX_TITLE" );
	m_pTextBoxSubTitle = GetControl<CEtUITextBox>( "ID_TEXTBOX_SUBTITLE" );
}

void CDnItemsOwnedAlarm::ShowAlarm( const WCHAR *wszTitle, const WCHAR *wszSubTitle, float fDuration )
{
	if( m_pTextBoxTitle )
	{
		m_pTextBoxTitle->ClearText();
		if( wszTitle ) m_pTextBoxTitle->AddText( wszTitle );
	}
	if( m_pTextBoxSubTitle )
	{
		m_pTextBoxSubTitle->ClearText();
		if( wszSubTitle ) m_pTextBoxSubTitle->AddText( wszSubTitle );
	}
	m_fAutoHideTime = fDuration;
	Show( true );
}

void CDnItemsOwnedAlarm::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( IsShow() && m_fAutoHideTime > 0.0f )
	{
		m_fAutoHideTime -= fElapsedTime;
		if( m_fAutoHideTime <= 0.0f )
			Show( false );
	}
}

//-----------------------------------------------------------------------------
// CDnItemsOwnedAlarmMng
//-----------------------------------------------------------------------------
CDnItemsOwnedAlarmMng::CDnItemsOwnedAlarmMng()
	: m_pAlarmDlg( NULL )
{
}

CDnItemsOwnedAlarmMng::~CDnItemsOwnedAlarmMng()
{
	Finalize();
}

void CDnItemsOwnedAlarmMng::Initialize()
{
	if( !m_pAlarmDlg )
	{
		m_pAlarmDlg = new CDnItemsOwnedAlarm( UI_TYPE_TOP );
		m_pAlarmDlg->Initialize( false );
	}
}

void CDnItemsOwnedAlarmMng::Finalize()
{
	SAFE_DELETE( m_pAlarmDlg );
}

void CDnItemsOwnedAlarmMng::CheckOwnedItemNotice( int nItemID )
{
	if( !m_pAlarmDlg || nItemID <= 0 )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TITEMNOTICE );
	if( !pSox || !pSox->IsExistItem( nItemID ) )
		return;

	m_pAlarmDlg->ShowAlarm( L"", L"", 5.0f );
}
