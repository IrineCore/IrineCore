#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosGaugeCubeStuffConfirmDlg.h"
#include "DnItem.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChaosGaugeCubeStuffConfirmDlg::CDnChaosGaugeCubeStuffConfirmDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticName( NULL )
	, m_pStaticCount( NULL )
	, m_pStaticGauge( NULL )
	, m_cCubeInvenType( 0 )
	, m_sCubeSlotIndex( -1 )
	, m_biCubeSerial( 0 )
	, m_cStuffInvenType( 0 )
	, m_sStuffSlotIndex( -1 )
	, m_biStuffSerial( 0 )
	, m_nStuffCount( 0 )
{
}

CDnChaosGaugeCubeStuffConfirmDlg::~CDnChaosGaugeCubeStuffConfirmDlg()
{
}

void CDnChaosGaugeCubeStuffConfirmDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MixGaugMessageBoxDlg.ui" ).c_str(), bShow );
}

void CDnChaosGaugeCubeStuffConfirmDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
	m_pStaticGauge = GetControl<CEtUIStatic>( "ID_TEXT_GAUGE" );

	CEtUIButton *pOkBtn = GetControl<CEtUIButton>( "ID_BT_OK" );
	if( pOkBtn )
		m_SmartMove.SetControl( pOkBtn );
}

void CDnChaosGaugeCubeStuffConfirmDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_SmartMove.MoveCursor();
	}
	else
	{
		m_SmartMove.ReturnCursor();
		m_cCubeInvenType = 0;
		m_sCubeSlotIndex = -1;
		m_biCubeSerial = 0;
		m_cStuffInvenType = 0;
		m_sStuffSlotIndex = -1;
		m_biStuffSerial = 0;
		m_nStuffCount = 0;
	}

	CEtUIDialog::Show( bShow );
}

void CDnChaosGaugeCubeStuffConfirmDlg::SetStuffConfirmInfo(
	char cCubeInvenType, short sCubeSlotIndex, INT64 biCubeSerial,
	CDnItem *pStuffItem, int nCount, int nGaugePerItem )
{
	m_cCubeInvenType = cCubeInvenType;
	m_sCubeSlotIndex = sCubeSlotIndex;
	m_biCubeSerial = biCubeSerial;

	if( !pStuffItem )
		return;

	m_cStuffInvenType = pStuffItem->IsCashItem() ? ITEMPOSITION_CASHINVEN : ITEMPOSITION_INVEN;
	m_sStuffSlotIndex = (short)pStuffItem->GetSlotIndex();
	m_biStuffSerial = pStuffItem->GetSerialID();
	m_nStuffCount = ( nCount > 0 ) ? nCount : 1;

	if( m_pStaticName )
		m_pStaticName->SetText( pStuffItem->GetName() );

	WCHAR wszBuf[64] = { 0 };
	if( m_pStaticCount )
	{
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d", m_nStuffCount );
		m_pStaticCount->SetText( wszBuf );
	}
	if( m_pStaticGauge )
	{
		int nTotalGauge = m_nStuffCount * ( ( nGaugePerItem > 0 ) ? nGaugePerItem : 1 );
		swprintf_s( wszBuf, _countof( wszBuf ), L"+%d", nTotalGauge );
		m_pStaticGauge->SetText( wszBuf );
	}
}

void CDnChaosGaugeCubeStuffConfirmDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_biCubeSerial != 0 && m_biStuffSerial != 0 && m_nStuffCount > 0 )
			{
				CSChaosGaugeCubeAddStuff packet;
				memset( &packet, 0, sizeof( packet ) );
				packet.cCubeInvenType = m_cCubeInvenType;
				packet.sCubeInvenIndex = m_sCubeSlotIndex;
				packet.biCubeInvenSerial = m_biCubeSerial;
				packet.cStuffInvenType = m_cStuffInvenType;
				packet.sStuffInvenIndex = m_sStuffSlotIndex;
				packet.biStuffInvenSerial = m_biStuffSerial;
				packet.nStuffCount = m_nStuffCount;
				GetItemTask().RequestChaosGaugeCubeAddStuff( packet );
			}
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) )
		{
			Show( false );
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#endif // PRE_ADD_CHAOSCUBE
