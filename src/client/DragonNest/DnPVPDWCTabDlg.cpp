#include "StdAfx.h"

#if defined( PRE_ADD_DWC )

#include "DnPVPDWCTabDlg.h"
#include "DnJobIconStatic.h"
#include "DnDWCTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnPVPDWCTabDlg
//-----------------------------------------------------------------------------
CDnPVPDWCTabDlg::CDnPVPDWCTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
	, m_nSelectedTab( TAB_COLOSSEUM )
{
	for( int i = 0; i < TAB_MAX; ++i )
		m_pTabButtons[i] = NULL;
}

CDnPVPDWCTabDlg::~CDnPVPDWCTabDlg()
{
}

void CDnPVPDWCTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpDWCTabDlg.ui" ).c_str(), bShow );
}

void CDnPVPDWCTabDlg::InitialUpdate()
{
	const char *szControlNames[TAB_MAX] = { "ID_RBT_COLO", "ID_RBT_DWC", "ID_RBT_DWCEX" };
	for( int i = 0; i < TAB_MAX; ++i )
		m_pTabButtons[i] = GetControl<CEtUIRadioButton>( szControlNames[i] );

	if( m_pTabButtons[TAB_COLOSSEUM] )
		m_pTabButtons[TAB_COLOSSEUM]->SetChecked( true, true, false );
}

void CDnPVPDWCTabDlg::SelectTab( int nTab )
{
	if( nTab < 0 || nTab >= TAB_MAX )
		return;
	m_nSelectedTab = nTab;
	if( m_pTabButtons[nTab] )
		m_pTabButtons[nTab]->SetChecked( true, false, false );
}

void CDnPVPDWCTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsCmdControl( "ID_RBT_COLO" ) )
			m_nSelectedTab = TAB_COLOSSEUM;
		else if( IsCmdControl( "ID_RBT_DWC" ) )
			m_nSelectedTab = TAB_DWC;
		else if( IsCmdControl( "ID_RBT_DWCEX" ) )
			m_nSelectedTab = TAB_DWC_EX;
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

//-----------------------------------------------------------------------------
// CDnPvPRankListItemDwcDlg
//-----------------------------------------------------------------------------
CDnPvPRankListItemDwcDlg::CDnPvPRankListItemDwcDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticRank( NULL )
	, m_pStaticTeamName( NULL )
	, m_pStaticTeamCaptain( NULL )
	, m_pStaticResult( NULL )
	, m_pStaticCount( NULL )
	, m_pStaticClass( NULL )
{
}

CDnPvPRankListItemDwcDlg::~CDnPvPRankListItemDwcDlg()
{
}

void CDnPvPRankListItemDwcDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "RankDWCListDlg.ui" ).c_str(), bShow );
}

void CDnPvPRankListItemDwcDlg::InitialUpdate()
{
	m_pStaticRank = GetControl<CEtUIStatic>( "ID_TEXT_RANK" );
	m_pStaticTeamName = GetControl<CEtUIStatic>( "ID_TEXT_TEAMNAME" );
	m_pStaticTeamCaptain = GetControl<CEtUIStatic>( "ID_TEXT_TEAMCAPTAIN" );
	m_pStaticResult = GetControl<CEtUIStatic>( "ID_TEXT_RESULT" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
	m_pStaticClass = GetControl<CDnJobIconStatic>( "ID_STATIC_CLASS" );
}

void CDnPvPRankListItemDwcDlg::SetRankInfo( const TDWCRankInfo &info )
{
	WCHAR wszBuf[128] = { 0 };
	if( m_pStaticRank )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", info.nRank );
		m_pStaticRank->SetText( wszBuf );
	}
	if( m_pStaticTeamName )
		m_pStaticTeamName->SetText( info.wszTeamName );
	if( m_pStaticTeamCaptain )
		m_pStaticTeamCaptain->SetText( info.wszLeaderName );
	if( m_pStaticResult )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d / %d / %d", info.nTotalWin, info.nTotalLose, info.nTotalDraw );
		m_pStaticResult->SetText( wszBuf );
	}
	if( m_pStaticCount )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", info.nDWCPoint );
		m_pStaticCount->SetText( wszBuf );
	}
	if( m_pStaticClass )
		m_pStaticClass->SetIconID( info.cLeaderJobCode, true );
}

#endif // PRE_ADD_DWC
