#include "StdAfx.h"

#ifdef PRE_ADD_CHAOSCUBE

#include "DnChaosGaugeCubeDlg.h"
#include "DnChaosGaugeCubeListItemDlg.h"
#include "DnChaosGaugeCubeStuffConfirmDlg.h"
#include "DnChaosGaugeCubeResultDlg.h"
#include "DnChaosGaugeCubeCompleteDlg.h"
#include "DnChaosCubeProgressDlg.h"
#include "DnStoreConfirmExDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnInterface.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnChaosGaugeCubeDlg::CDnChaosGaugeCubeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
	, m_pStaticTitle( NULL )
	, m_pStaticDesc( NULL )
	, m_pProgressGauge( NULL )
	, m_pListBoxStuff( NULL )
	, m_pBtnOK( NULL )
	, m_pBtnCancel( NULL )
	, m_pBtnPreview( NULL )
	, m_pStaticEffect( NULL )
	, m_pStaticGaugeCount( NULL )
	, m_pSplitDlg( NULL )
	, m_pStuffConfirmDlg( NULL )
	, m_pResultPreviewDlg( NULL )
	, m_pProgressDlg( NULL )
	, m_pCompleteDlg( NULL )
	, m_biCubeSerial( 0 )
	, m_cCubeInvenType( 0 )
	, m_sCubeSlotIndex( -1 )
	, m_nChaosNum( 0 )
	, m_nMaxGauge( 100 )
	, m_nCurrentGauge( 0 )
	, m_nTargetGauge( 0 )
	, m_fGaugeAnimProgress( 0.0f )
{
}

CDnChaosGaugeCubeDlg::~CDnChaosGaugeCubeDlg()
{
	if( m_pListBoxStuff )
		m_pListBoxStuff->RemoveAllItems();
	SAFE_DELETE( m_pSplitDlg );
	SAFE_DELETE( m_pStuffConfirmDlg );
	SAFE_DELETE( m_pResultPreviewDlg );
	SAFE_DELETE( m_pProgressDlg );
	SAFE_DELETE( m_pCompleteDlg );
}

void CDnChaosGaugeCubeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MixGaugDlg.ui" ).c_str(), bShow );

	m_pSplitDlg = new CDnStoreConfirmExDlg( UI_TYPE_MODAL, NULL, DIALOG_SPLIT_ITEM, this );
	if( m_pSplitDlg )
		m_pSplitDlg->Initialize( false );

	m_pStuffConfirmDlg = new CDnChaosGaugeCubeStuffConfirmDlg( UI_TYPE_MODAL, NULL, DIALOG_STUFF_CONFIRM, this );
	if( m_pStuffConfirmDlg )
		m_pStuffConfirmDlg->Initialize( false );

	m_pResultPreviewDlg = new CDnChaosGaugeCubeResultDlg( UI_TYPE_MODAL, NULL, DIALOG_RESULT_PREVIEW, this );
	if( m_pResultPreviewDlg )
		m_pResultPreviewDlg->Initialize( false );

	m_pProgressDlg = new CDnChaosCubeProgressDlg( UI_TYPE_MODAL, NULL, DIALOG_PROGRESS, this );
	if( m_pProgressDlg )
	{
		m_pProgressDlg->Initialize( false );
		m_pProgressDlg->SetCubeType( CDnChaosCubeProgressDlg::ECUBETYPE_CHAOSGAUGE );
	}

	m_pCompleteDlg = new CDnChaosGaugeCubeCompleteDlg( UI_TYPE_MODAL, NULL, DIALOG_COMPLETE, this );
	if( m_pCompleteDlg )
		m_pCompleteDlg->Initialize( false );
}

void CDnChaosGaugeCubeDlg::InitialUpdate()
{
	m_pStaticTitle = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pStaticDesc = GetControl<CEtUIStatic>( "ID_TEXT" );
	m_pProgressGauge = GetControl<CEtUIProgressBar>( "ID_PROGRESSBAR_COUNT" );
	m_pListBoxStuff = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );
	m_pBtnOK = GetControl<CEtUIButton>( "ID_BT_OK" );
	m_pBtnCancel = GetControl<CEtUIButton>( "ID_BT_CANCEL" );
	m_pBtnPreview = GetControl<CEtUIButton>( "ID_BT_PREVIEW" );
	m_pStaticEffect = GetControl<CEtUIStatic>( "ID_STATIC_EFFECT" );
	m_pStaticGaugeCount = GetControl<CEtUIStatic>( "ID_TEXT_GAUGECOUNT" );
}

void CDnChaosGaugeCubeDlg::ResetCubeState()
{
	m_biCubeSerial = 0;
	m_cCubeInvenType = 0;
	m_sCubeSlotIndex = -1;
	m_nChaosNum = 0;
	m_nMaxGauge = 100;
	m_nCurrentGauge = 0;
	m_nTargetGauge = 0;
	m_fGaugeAnimProgress = 0.0f;
	m_mapStuffTable.clear();
	if( m_pListBoxStuff )
		m_pListBoxStuff->RemoveAllItems();
}

void CDnChaosGaugeCubeDlg::SetChaosGaugeCubeItem( CDnItem *pItem )
{
	ResetCubeState();
	if( !pItem || pItem->GetClassID() <= 0 )
		return;

	m_biCubeSerial = pItem->GetSerialID();
	m_cCubeInvenType = pItem->IsCashItem() ? ITEMPOSITION_CASHINVEN : ITEMPOSITION_INVEN;
	m_sCubeSlotIndex = (short)pItem->GetSlotIndex();
	m_nChaosNum = pItem->GetTypeParam( 0 );
	m_nMaxGauge = pItem->GetTypeParam( 2 );
	if( m_nMaxGauge <= 0 )
	{
		DNTableFileFormat *pItemSox = GetDNTable( CDnTableDB::TITEM );
		if( pItemSox )
		{
			DNTableCell *pCell = pItemSox->GetFieldFromLablePtr( pItem->GetClassID(), "_TypeParam3" );
			if( pCell && pCell->GetInteger() > 0 )
				m_nMaxGauge = pCell->GetInteger();
		}
	}
	if( m_nMaxGauge <= 0 )
		m_nMaxGauge = 100;

	if( m_pStaticTitle )
		m_pStaticTitle->SetText( pItem->GetName() );

	m_nCurrentGauge = GetItemTask().GetChaosGauge( m_nChaosNum );
	m_nTargetGauge = m_nCurrentGauge;
	m_fGaugeAnimProgress = ( m_nMaxGauge > 0 ) ? ( (float)m_nCurrentGauge / (float)m_nMaxGauge ) * 100.0f : 0.0f;

	RefreshStuffList();
	RefreshGaugeUI();
}

CDnItem *CDnChaosGaugeCubeDlg::FindInventoryItem( int nItemID ) const
{
	CDnItem *pItem = GetItemTask().GetCharInventory().FindItem( nItemID );
	if( pItem )
		return pItem;
	pItem = GetItemTask().GetCashInventory().FindItem( nItemID );
	if( pItem )
		return pItem;
	return GetItemTask().GetQuestInventory().FindItem( nItemID );
}

int CDnChaosGaugeCubeDlg::GetOwnedItemCount( int nItemID ) const
{
	int nCount = 0;
	nCount += GetItemTask().GetCharInventory().GetItemCount( nItemID );
	nCount += GetItemTask().GetCashInventory().GetItemCount( nItemID );
	nCount += GetItemTask().GetQuestInventory().GetItemCount( nItemID );
	return nCount;
}

void CDnChaosGaugeCubeDlg::RefreshStuffList()
{
	if( !m_pListBoxStuff || m_nChaosNum <= 0 )
		return;

	m_pListBoxStuff->RemoveAllItems();
	m_mapStuffTable.clear();

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TCHAOSCUBESTUFF );
	if( !pSox )
		return;

	std::vector<SStuffInfo> vecOwned;
	std::vector<SStuffInfo> vecUnowned;

	int nCount = pSox->GetItemCount();
	for( int i = 0; i < nCount; ++i )
	{
		int nRowID = pSox->GetItemID( i );
		DNTableCell *pNumCell = pSox->GetFieldFromLablePtr( nRowID, "_ChaoscubeNum" );
		if( !pNumCell || pNumCell->GetInteger() != m_nChaosNum )
			continue;

		DNTableCell *pItemCell = pSox->GetFieldFromLablePtr( nRowID, "_ItemID" );
		if( !pItemCell || pItemCell->GetInteger() <= 0 )
			continue;

		SStuffInfo info;
		info.nRowID = nRowID;
		info.nItemID = pItemCell->GetInteger();
		info.nGaugeValue = 1;

		if( pSox->GetFieldNum( "_GaugeValue" ) >= 0 )
		{
			DNTableCell *pGaugeCell = pSox->GetFieldFromLablePtr( nRowID, "_GaugeValue" );
			if( pGaugeCell && pGaugeCell->GetInteger() > 0 )
				info.nGaugeValue = pGaugeCell->GetInteger();
		}

		m_mapStuffTable[info.nItemID] = info;
		if( GetOwnedItemCount( info.nItemID ) > 0 )
			vecOwned.push_back( info );
		else
			vecUnowned.push_back( info );
	}

	for( size_t i = 0; i < vecOwned.size(); ++i )
	{
		CDnChaosGaugeCubeListItemDlg *pItemDlg = m_pListBoxStuff->AddItem<CDnChaosGaugeCubeListItemDlg>();
		if( pItemDlg )
			pItemDlg->SetInfo( vecOwned[i].nItemID, GetOwnedItemCount( vecOwned[i].nItemID ), vecOwned[i].nGaugeValue );
	}

	for( size_t i = 0; i < vecUnowned.size(); ++i )
	{
		CDnChaosGaugeCubeListItemDlg *pItemDlg = m_pListBoxStuff->AddItem<CDnChaosGaugeCubeListItemDlg>();
		if( pItemDlg )
			pItemDlg->SetInfo( vecUnowned[i].nItemID, 0, vecUnowned[i].nGaugeValue );
	}
}

void CDnChaosGaugeCubeDlg::RefreshGaugeUI()
{
	float fPercent = ( m_nMaxGauge > 0 ) ? ( (float)m_nCurrentGauge / (float)m_nMaxGauge ) * 100.0f : 0.0f;
	if( fPercent > 100.0f )
		fPercent = 100.0f;
	if( fPercent < 0.0f )
		fPercent = 0.0f;

	if( m_pProgressGauge )
		m_pProgressGauge->SetProgress( fPercent );

	if( m_pStaticGaugeCount )
	{
		WCHAR wszText[64] = { 0 };
		swprintf_s( wszText, _countof( wszText ), L"%d / %d", m_nCurrentGauge, m_nMaxGauge );
		m_pStaticGaugeCount->SetText( wszText );
	}

	bool bReady = ( m_nCurrentGauge >= m_nMaxGauge && m_nMaxGauge > 0 );
	if( m_pBtnOK )
		m_pBtnOK->Enable( bReady );
	if( m_pStaticEffect )
		m_pStaticEffect->Show( bReady );
}

void CDnChaosGaugeCubeDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		if( m_biCubeSerial == 0 )
			return;

		RefreshStuffList();
		RefreshGaugeUI();
	}
	else
	{
		ResetCubeState();
	}

	CEtUIDialog::Show( bShow );
}

void CDnChaosGaugeCubeDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !IsShow() )
		return;

	if( m_nCurrentGauge != m_nTargetGauge )
	{
		float fTargetPct = ( m_nMaxGauge > 0 ) ? ( (float)m_nTargetGauge / (float)m_nMaxGauge ) * 100.0f : 0.0f;
		if( fTargetPct > 100.0f )
			fTargetPct = 100.0f;

		if( m_fGaugeAnimProgress < fTargetPct )
		{
			m_fGaugeAnimProgress += fElapsedTime * 80.0f;
			if( m_fGaugeAnimProgress >= fTargetPct )
			{
				m_fGaugeAnimProgress = fTargetPct;
				m_nCurrentGauge = m_nTargetGauge;
			}
			if( m_pProgressGauge )
				m_pProgressGauge->SetProgress( m_fGaugeAnimProgress );
		}
		else
		{
			m_fGaugeAnimProgress = fTargetPct;
			m_nCurrentGauge = m_nTargetGauge;
			RefreshGaugeUI();
		}
	}
}

void CDnChaosGaugeCubeDlg::OnAddStuffItem( CDnItem *pStuffItem )
{
	if( !pStuffItem || !IsShow() )
		return;

	if( m_nCurrentGauge >= m_nMaxGauge )
		return;

	std::map<int, SStuffInfo>::iterator it = m_mapStuffTable.find( pStuffItem->GetClassID() );
	if( it == m_mapStuffTable.end() )
		return;

	int nOverlap = pStuffItem->GetOverlapCount();
	if( nOverlap > 1 && m_pSplitDlg )
	{
		m_pSplitDlg->SetItem( pStuffItem, CDnInterface::PRIVATE_TRADE, nOverlap );
		m_pSplitDlg->Show( true );
	}
	else if( m_pStuffConfirmDlg )
	{
		m_pStuffConfirmDlg->SetStuffConfirmInfo(
			m_cCubeInvenType, m_sCubeSlotIndex, m_biCubeSerial,
			pStuffItem, 1, it->second.nGaugeValue );
		m_pStuffConfirmDlg->Show( true );
	}
}

void CDnChaosGaugeCubeDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_CLOSE_DIALOG" ) )
		{
			Show( false );
			return;
		}
		else if( IsCmdControl( "ID_BT_PREVIEW" ) )
		{
			if( m_pResultPreviewDlg )
			{
				m_pResultPreviewDlg->LoadResultItems( m_nChaosNum );
				m_pResultPreviewDlg->Show( true );
			}
			return;
		}
		else if( IsCmdControl( "ID_BT_OK" ) )
		{
			if( m_nCurrentGauge >= m_nMaxGauge && m_biCubeSerial != 0 && m_pProgressDlg )
			{
				m_pProgressDlg->SetCubeType( CDnChaosCubeProgressDlg::ECUBETYPE_CHAOSGAUGE );
				m_pProgressDlg->Show( true );
			}
			return;
		}
	}
	else if( nCommand == EVENT_LISTBOX_ITEM_DBLCLK || nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( IsCmdControl( "ID_LISTBOXEX_LIST" ) && nCommand == EVENT_LISTBOX_ITEM_DBLCLK && m_pListBoxStuff )
		{
			int nSel = m_pListBoxStuff->GetSelectedIndex();
			if( nSel >= 0 )
			{
				CDnChaosGaugeCubeListItemDlg *pEntry = m_pListBoxStuff->GetItem<CDnChaosGaugeCubeListItemDlg>( nSel );
				if( pEntry && pEntry->GetOwnedCount() > 0 )
				{
					CDnItem *pInvItem = FindInventoryItem( pEntry->GetItemID() );
					if( pInvItem )
						OnAddStuffItem( pInvItem );
				}
			}
			return;
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnChaosGaugeCubeDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	if( !pControl )
		return;

	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( nID == DIALOG_SPLIT_ITEM )
		{
			if( IsCmdControl( "ID_OK" ) && m_pSplitDlg && m_pStuffConfirmDlg )
			{
				CDnItem *pStuffItem = m_pSplitDlg->GetItem();
				int nCount = m_pSplitDlg->GetItemCount();
				if( pStuffItem && nCount > 0 )
				{
					int nGaugeValue = 1;
					std::map<int, SStuffInfo>::iterator it = m_mapStuffTable.find( pStuffItem->GetClassID() );
					if( it != m_mapStuffTable.end() )
						nGaugeValue = it->second.nGaugeValue;

					m_pStuffConfirmDlg->SetStuffConfirmInfo(
						m_cCubeInvenType, m_sCubeSlotIndex, m_biCubeSerial,
						pStuffItem, nCount, nGaugeValue );
					m_pSplitDlg->Show( false );
					m_pStuffConfirmDlg->Show( true );
				}
				else
				{
					m_pSplitDlg->Show( false );
				}
			}
			else if( IsCmdControl( "ID_CANCEL" ) && m_pSplitDlg )
			{
				m_pSplitDlg->Show( false );
			}
		}
		else if( nID == DIALOG_PROGRESS )
		{
			if( m_biCubeSerial != 0 )
			{
				CSChaosGaugeCubeComplete packet;
				memset( &packet, 0, sizeof( packet ) );
				packet.cCubeInvenType = m_cCubeInvenType;
				packet.sCubeInvenIndex = m_sCubeSlotIndex;
				packet.biCubeInvenSerial = m_biCubeSerial;
				GetItemTask().RequestChaosGaugeCubeComplete( packet );
			}
		}
	}
}

void CDnChaosGaugeCubeDlg::OnRecvAddStuff( int nChaosGaugeType, int nCurrentGauge )
{
	if( !IsShow() || nChaosGaugeType != m_nChaosNum )
		return;

	m_nTargetGauge = nCurrentGauge;
	m_nCurrentGauge = nCurrentGauge;
	RefreshGaugeUI();
	RefreshStuffList();
}

void CDnChaosGaugeCubeDlg::OnRecvComplete( int nChaosGaugeType, int nResultItemID, int nItemCount )
{
	if( m_pCompleteDlg && nResultItemID > 0 )
	{
		m_pCompleteDlg->SetResultItem( nResultItemID, nItemCount );
		m_pCompleteDlg->Show( true );
	}

	if( IsShow() && nChaosGaugeType == m_nChaosNum )
	{
		m_nCurrentGauge = 0;
		m_nTargetGauge = 0;
		m_fGaugeAnimProgress = 0.0f;
		RefreshGaugeUI();
		RefreshStuffList();
	}
}

void CDnChaosGaugeCubeDlg::UpdateStuffList()
{
	if( IsShow() )
		RefreshStuffList();
}

#endif // PRE_ADD_CHAOSCUBE
