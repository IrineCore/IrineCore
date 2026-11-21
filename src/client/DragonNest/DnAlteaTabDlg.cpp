#include "stdafx.h"
#include "DnAlteaTabDlg.h"
#include "DnAlteaCheckDlg.h"
#include "DnAlteaInfoDlg.h"
#include "DnAlteaRankDlg.h"
#include "DnAlteaInviteDlg.h"
#include "DnAlteaTask.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )

CDnAlteaTabDlg::CDnAlteaTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback)
: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pEnter_Button( NULL )
, m_pCancle_Button( NULL )
, m_pEnable_Static( NULL )
, m_pHold_Static( NULL )
, m_pEnter_TabButton( NULL )
, m_pIntro_TabButton( NULL )
, m_pPersonalRank_TabButton( NULL )
, m_pGuildRank_TabButton( NULL )
, m_pSend_TabButton( NULL )
, m_pAlteaCheckDlg( NULL )
, m_pAlteaInfoDlg( NULL )
, m_pAlteaPersonalRankDlg( NULL )
, m_pAlteaGuildRankDlg( NULL )
, m_pAlteaInviteDlg( NULL )
{
}

CDnAlteaTabDlg::~CDnAlteaTabDlg(void)
{
}

void CDnAlteaTabDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AlteaTabDlg.ui" ).c_str(), bShow );
}

void CDnAlteaTabDlg::InitialUpdate()
{
	m_pEnter_Button = GetControl<CEtUIButton>( "ID_BT_OK" );
	m_pCancle_Button = GetControl<CEtUIButton>( "ID_BT_CANNEL" );

	m_pEnable_Static = GetControl<CEtUIStatic>( "ID_TEXT_CLEAR" );
	m_pHold_Static = GetControl<CEtUIStatic>( "ID_TEXT_MAP" );

	m_pEnter_TabButton = GetControl<CEtUIRadioButton>("ID_TAB0");
	m_pAlteaCheckDlg = new CDnAlteaCheckDlg( UI_TYPE_CHILD, this );
	m_pAlteaCheckDlg->Initialize( false );
	AddTabDialog( m_pEnter_TabButton, m_pAlteaCheckDlg );

	m_pIntro_TabButton = GetControl<CEtUIRadioButton>("ID_TAB1");
	m_pAlteaInfoDlg = new CDnAlteaInfoDlg( UI_TYPE_CHILD, this );
	m_pAlteaInfoDlg->Initialize( false );
	AddTabDialog( m_pIntro_TabButton, m_pAlteaInfoDlg );

	m_pPersonalRank_TabButton = GetControl<CEtUIRadioButton>("ID_TAB2");
	m_pAlteaPersonalRankDlg = new CDnAlteaRankDlg( UI_TYPE_CHILD, this );
	m_pAlteaPersonalRankDlg->Initialize( false );
	AddTabDialog( m_pPersonalRank_TabButton, m_pAlteaPersonalRankDlg );

	m_pGuildRank_TabButton = GetControl<CEtUIRadioButton>("ID_TAB3");
	m_pAlteaGuildRankDlg = new CDnAlteaRankDlg( UI_TYPE_CHILD, this );
	m_pAlteaGuildRankDlg->Initialize( false );
	m_pAlteaGuildRankDlg->SetGuildRankDlg();
	AddTabDialog( m_pGuildRank_TabButton, m_pAlteaGuildRankDlg );

	m_pSend_TabButton = GetControl<CEtUIRadioButton>("ID_TAB4");
	m_pAlteaInviteDlg = new CDnAlteaInviteDlg( UI_TYPE_CHILD, this );
	m_pAlteaInviteDlg->Initialize( false );
	AddTabDialog( m_pSend_TabButton, m_pAlteaInviteDlg );
}

void CDnAlteaTabDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		SetCheckedTab( m_pEnter_TabButton->GetTabID() );
		
		GetAlteaTask().RequestAlteaInfo( AlteiaWorld::Info::JoinInfo );
		GetAlteaTask().RequestAlteaInfo( AlteiaWorld::Info::PrivateGoldKeyRankInfo );
		GetAlteaTask().RequestAlteaInfo( AlteiaWorld::Info::PrivatePlayTimeRankInfo );
		GetAlteaTask().RequestAlteaInfo( AlteiaWorld::Info::GuildGoldKeyRankInfo );
		GetAlteaTask().RequestAlteaInfo( AlteiaWorld::Info::SendTicketInfo );

		EnableEnterTime();
	}
	else
		GetAlteaTask().RequestAlteaQuitInfo();

	CEtUITabDialog::Show( bShow );
}

void CDnAlteaTabDlg::Process( float fElapsedTime )
{
	if( false == m_bShow )
		return;

	CEtUITabDialog::Process( fElapsedTime );
}

void CDnAlteaTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl("ID_BT_OK" ) )
		{
			GetAlteaTask().RequestAlteaJoin();
		}
		else if( IsCmdControl("ID_BT_CANNEL" ) 
				|| IsCmdControl("ID_BT_CLOSE" ) )
		{
			Show( false );
		}
	}

	CEtUITabDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnAlteaTabDlg::SetCount( const int nEnableCount, const int nHoldCount )
{
	WCHAR wszString[256] = {0,};
	int nTotalCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::AlteiaWorldDailyPlayCount);
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7994 ), nEnableCount, nTotalCount );	// UISTRING : Daily clear count %d/%d
	m_pEnable_Static->SetText( wszString );

	int nNeedCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::AlteiaWorldJoinTicketCount);
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7982 ), nHoldCount, nNeedCount );	// UISTRING : Currently owned / Required for entry %d/%d
	m_pHold_Static->SetText( wszString );

	bool bEnterCount = nEnableCount < nTotalCount;
	bool bNeedCount = nHoldCount >= nNeedCount;

	if( false == bEnterCount )
	{
		m_pEnter_Button->Enable( false );
		m_pEnter_Button->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7996 ) );	// UISTRING : Daily clear limit reached.
	}
	else if( false == bNeedCount )
	{
		m_pEnter_Button->Enable( false );
		m_pEnter_Button->SetTooltipText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7997 ) );	// UISTRING : Insufficient quantity [5] required for entry.
	}
	else
	{
		m_pEnter_Button->Enable( true );
		m_pEnter_Button->ClearTooltipText();
	}

	EnableEnterTime();
}

void CDnAlteaTabDlg::EnableEnterTime()
{
	DNTableFileFormat * pSox = GetDNTable( CDnTableDB::TSCHEDULE );
	if( NULL == pSox )
		return;

	time_t LocalTime;
	tm pLocalTime;
	time(&LocalTime);
	pLocalTime = *localtime(&LocalTime);

	// 5 is weekly mission reset
	int nWeek = pSox->GetFieldFromLablePtr( 5, "_Cycle" )->GetInteger() - 2;	// 0 and 1 are used for other values in the table, so subtract 2.

	if( pLocalTime.tm_wday != nWeek )
		return;

	int nHour = pSox->GetFieldFromLablePtr( 5, "_Hour" )->GetInteger();
	int nMinute = pSox->GetFieldFromLablePtr( 5, "_Minute" )->GetInteger();

	int nEnterLimitStartTime = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AlteiaWorldEnterLimitStartTime );
	int nEnterLimitEndTime = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AlteiaWorldEnterLimitEndTime );
	int nResetTime = nHour * 60 + nMinute;
	int nCurTime = pLocalTime.tm_hour * 60 + pLocalTime.tm_min;

	if( nCurTime >= nResetTime - nEnterLimitStartTime && nCurTime <= nResetTime + nEnterLimitEndTime )
		m_pEnter_Button->Enable( false );
}

#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )