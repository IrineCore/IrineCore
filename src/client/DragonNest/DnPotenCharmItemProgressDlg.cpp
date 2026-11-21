#include "StdAfx.h"
#include "DnPotenCharmItemProgressDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPotenCharmItemProgressDlg::CDnPotenCharmItemProgressDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pBtnNext( NULL )
	, m_pBtnStop( NULL )
	, m_pProgressPoten( NULL )
	, m_pStatic5( NULL )
	, m_pStaticDouble( NULL )
	, m_pStaticLight( NULL )
	, m_pStaticPotenBase( NULL )
	, m_pStaticCashBase( NULL )
	, m_pStaticText( NULL )
	, m_pTextBoxPotenCont( NULL )
	, m_pTextBoxPoten( NULL )
	, m_nSoundFeverStart( -1 )
	, m_nSoundFeverLoop( -1 )
	, m_nSoundDoubleStart( -1 )
	, m_nSoundDoubleLoop( -1 )
	, m_bDoubleFever( false )
	, m_fGaugePercent( 0.0f )
{
}

CDnPotenCharmItemProgressDlg::~CDnPotenCharmItemProgressDlg()
{
}

void CDnPotenCharmItemProgressDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "InvenPotenBoxOpenDlg.ui" ).c_str(), bShow );
}

void CDnPotenCharmItemProgressDlg::InitialUpdate()
{
	m_pBtnNext = GetControl<CEtUIButton>( "ID_BT_NEXT" );
	m_pBtnStop = GetControl<CEtUIButton>( "ID_BT_STOP" );
	m_pProgressPoten = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR_POTEN" );
	m_pStatic5 = GetControl<CEtUIStatic>( "ID_STATIC5" );
	m_pStaticDouble = GetControl<CEtUIStatic>( "ID_STATIC_DOUBLE" );
	m_pStaticLight = GetControl<CEtUIStatic>( "ID_STATIC_LIGHT" );
	m_pStaticPotenBase = GetControl<CEtUIStatic>( "ID_STATIC_POTENBASE" );
	m_pStaticCashBase = GetControl<CEtUIStatic>( "ID_STATIC_CASHBASE" );
	m_pStaticText = GetControl<CEtUIStatic>( "ID_TEXT" );
	m_pTextBoxPotenCont = GetControl<CEtUITextBox>( "ID_TEXTBOX_POTENCONT" );
	m_pTextBoxPoten = GetControl<CEtUITextBox>( "ID_TEXTBOX_POTEN" );

	m_nSoundFeverStart = CEtSoundEngine::GetInstance().LoadSound( "UI_Gatcha_Fever_Start.ogg", false, false );
	m_nSoundFeverLoop = CEtSoundEngine::GetInstance().LoadSound( "UI_Gatcha_Fever_Loop.ogg", false, false );
	m_nSoundDoubleStart = CEtSoundEngine::GetInstance().LoadSound( "UI_Gatcha_DoubleFever_Start.ogg", false, false );
	m_nSoundDoubleLoop = CEtSoundEngine::GetInstance().LoadSound( "UI_Gatcha_DoubleFever_Loop.ogg", false, false );
}

void CDnPotenCharmItemProgressDlg::Show( bool bShow )
{
	if( bShow )
	{
		if( m_bDoubleFever && m_nSoundDoubleStart != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundDoubleStart );
		else if( m_nSoundFeverStart != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSoundFeverStart );
	}
	CEtUIDialog::Show( bShow );
}

void CDnPotenCharmItemProgressDlg::SetPotenGauge( float fPercent, bool bDoubleFever, const WCHAR *wszDesc )
{
	m_fGaugePercent = fPercent;
	m_bDoubleFever = bDoubleFever;
	if( m_pProgressPoten ) m_pProgressPoten->SetProgress( fPercent );
	if( m_pStaticDouble ) m_pStaticDouble->Show( bDoubleFever );
	if( m_pStaticLight ) m_pStaticLight->Show( fPercent >= 100.0f );
	if( m_pTextBoxPoten && wszDesc )
	{
		m_pTextBoxPoten->ClearText();
		m_pTextBoxPoten->AddText( wszDesc );
	}
}

void CDnPotenCharmItemProgressDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_STOP" ) || IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
