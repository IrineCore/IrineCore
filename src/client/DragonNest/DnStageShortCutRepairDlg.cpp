#include "StdAfx.h"
#include "DnStageShortCutRepairDlg.h"
#include "DnItemTask.h"
#include "TradeSendPacket.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnStageShortCutRepairDlg::CDnStageShortCutRepairDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pIconRepute( NULL )
{
	for( int i = 0; i < COST_LINE_MAX; ++i )
	{
		m_pStaticGold[i] = NULL;
		m_pStaticSilver[i] = NULL;
		m_pStaticBronze[i] = NULL;
	}
}

CDnStageShortCutRepairDlg::~CDnStageShortCutRepairDlg()
{
}

void CDnStageShortCutRepairDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonRepairConfirmDlg.ui" ).c_str(), bShow );
}

void CDnStageShortCutRepairDlg::InitialUpdate()
{
	char szName[32] = { 0 };
	for( int i = 0; i < COST_LINE_MAX; ++i )
	{
		sprintf_s( szName, sizeof( szName ), "ID_GOLD_%d", i );
		m_pStaticGold[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_SILVER_%d", i );
		m_pStaticSilver[i] = GetControl<CEtUIStatic>( szName );
		sprintf_s( szName, sizeof( szName ), "ID_BRONZE_%d", i );
		m_pStaticBronze[i] = GetControl<CEtUIStatic>( szName );
	}
	m_pIconRepute = GetControl<CEtUIStatic>( "ID_ICON_REPUTE" );
}

void CDnStageShortCutRepairDlg::SetRepairCost( INT64 nEquipCost, INT64 nAllCost )
{
	INT64 costs[COST_LINE_MAX] = { nEquipCost, nAllCost };
	for( int i = 0; i < COST_LINE_MAX; ++i )
	{
		if( m_pStaticGold[i] ) m_pStaticGold[i]->SetIntToText( (int)( costs[i] / 10000 ) );
		if( m_pStaticSilver[i] ) m_pStaticSilver[i]->SetIntToText( (int)( ( costs[i] % 10000 ) / 100 ) );
		if( m_pStaticBronze[i] ) m_pStaticBronze[i]->SetIntToText( (int)( costs[i] % 100 ) );
	}
}

void CDnStageShortCutRepairDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) || IsCmdControl( "ID_OK" ) )
		{
			SendRepairAll();
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
