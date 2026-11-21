#include "stdafx.h"
#include "DnAlteaCheckDlg.h"
#include "DnTableDB.h"
#include "DnMissionTask.h"
#include "DnCashShopTask.h"
#include "DnItemTask.h"
#include "DnAlteaTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#if defined( PRE_ALTEIAWORLD_EXPLORE )

#define REWARD_ITEM_INDEX 0

CDnAlteaCheckDlg::CDnAlteaCheckDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg(dialogType, pParentDialog, nID, pCallback, true)
, m_pNotice_Static( NULL )
{

}

CDnAlteaCheckDlg::~CDnAlteaCheckDlg(void)
{
	for( int itr = 0; itr < E_Clear_Count; ++itr )
		m_pClearUI[itr].ResetSlot();
}

void CDnAlteaCheckDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "AlteaCheckDlg.ui" ).c_str(), bShow );
}

void CDnAlteaCheckDlg::InitialUpdate()
{
	char szName[64] = {0,};
	for( int itr = 0; itr < E_Clear_Count; ++itr )
	{
		sprintf_s( szName, "ID_BT_ITEM%d", itr + 1 );
		m_pClearUI[itr].m_pSlotButton = GetControl<CDnItemSlotButton>( szName );

		sprintf_s( szName, "ID_STATIC_ITEMSLOT%d", itr + 1 );
		m_pClearUI[itr].m_pSlot_Static = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, "ID_TEXT_COUNT%d", itr + 1 );
		m_pClearUI[itr].m_pCount_Static = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, "ID_STATIC_STEMPSLOT%d", itr + 1 );
		m_pClearUI[itr].m_pBoard_Static = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, "ID_STATIC_CHECK%d", itr + 1 );
		m_pClearUI[itr].m_pCheck_Static = GetControl<CEtUIStatic>( szName );

		m_pClearUI[itr].ResetSlot();
	}

	m_pNotice_Static = GetControl<CEtUIStatic>("ID_TEXT_NOTICE");

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSCHEDULE );
	if( NULL == pSox )
		return;

	int nHour = (char)pSox->GetFieldFromLablePtr( 4, "_Hour" )->GetInteger();	// 4 is daily mission reset

	WCHAR wszString[512] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7995 ), nHour );
	m_pNotice_Static->SetText( wszString );
}

void CDnAlteaCheckDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		if( m_vAlteaMission.empty() )
			InitializeTable();

		GetAlteaTask().RefreshCheckDlg();
	}

	CDnCustomDlg::Show( bShow );
}

void CDnAlteaCheckDlg::SetNotice( const int nTime )
{
	WCHAR wszString[32] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7995 ), nTime );	// UISTRING : You can receive the above rewards based on weekly clear count.............
	m_pNotice_Static->SetText( wszString );
}

void CDnAlteaCheckDlg::SetWeeklyClearCount( const int nCount )
{
	for( int itr = 0; itr < E_Clear_Count; ++itr )
		m_pClearUI[itr].m_pCheck_Static->Show( itr < nCount ? true : false );
}

void CDnAlteaCheckDlg::InitializeTable()
{
	DNTableFileFormat * pSox = GetDNTable( CDnTableDB::TALTEIAWEEKREWARD );
	if( NULL == pSox )
		return;

	int nCount = pSox->GetItemCount();
	for( int itr = 0; itr < nCount; ++itr )
	{
		int nItemID = pSox->GetItemID( itr );
		int nOrder = pSox->GetFieldFromLablePtr( nItemID, "_Order" )->GetInteger() - 1;		// Subtract 1 because Table Index starts from 0.
		int nMissionID = pSox->GetFieldFromLablePtr( nItemID, "_MissionID" )->GetInteger();

		m_vAlteaMission.push_back( std::pair<int, int>( nMissionID, nOrder ) );
	}

	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();
	std::vector<CDnMissionTask::MissionInfoStruct *> vecList;
	DWORD dwCount = pTask->GetDailyMissionCount( CDnMissionTask::WeekendEvent );
	for( DWORD itr = 0; itr < dwCount; ++itr )
	{
		CDnMissionTask::MissionInfoStruct *pInfo = pTask->GetDailyMissionInfo( CDnMissionTask::WeekendEvent, itr );
		if( NULL == pInfo )
			continue;

		const int nOderID = GetAlteaMissionByOderID( pInfo->nNotifierID );
		if( -1 == nOderID )
			continue;

		vecList.push_back( pInfo );
	}

	for( DWORD itr = 0; itr < m_vAlteaMission.size(); ++itr )
	{
		CDnMissionTask::MissionInfoStruct * pInfo = NULL;
		for( DWORD jtr = 0; jtr < vecList.size(); ++jtr )
		{
			if( m_vAlteaMission[itr].first != vecList[jtr]->nNotifierID )
				continue;

			pInfo = vecList[jtr];
		}
		SetSlot( itr, pInfo );
	}
}

int CDnAlteaCheckDlg::GetAlteaMissionByOderID( const int nMissionID )
{
	for( DWORD itr = 0; itr < m_vAlteaMission.size(); ++itr )
	{
		if( nMissionID == m_vAlteaMission[itr].first )
			return m_vAlteaMission[itr].second;
	}

	return -1;
}

void CDnAlteaCheckDlg::SetSlot( const int nCount, void * pData )
{
	m_pClearUI[nCount].ResetSlot();

	WCHAR wszString[32] = {0,};
	swprintf_s( wszString, _countof(wszString), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7993 ), nCount + 1 );	// UISTRING : %d times
	m_pClearUI[nCount].m_pCount_Static->SetText( wszString );

	CDnMissionTask::MissionInfoStruct * pInfo = (CDnMissionTask::MissionInfoStruct *)pData;
	if( NULL == pInfo )
		return;

	bool bRewardCashItem = false;
	for( int itr = 0; itr < CDnMissionTask::RewardItem_Amount; ++itr )
	{
		if( pInfo->nRewardCashItemSN[itr] > 0 )
		{
			bRewardCashItem = true;
			break;
		}
	}

	if( true == bRewardCashItem )
	{
		if( 0 >= pInfo->nRewardCashItemSN[REWARD_ITEM_INDEX] )
			return;

		const SCashShopItemInfo* pCashInfo = GetCashShopTask().GetItemInfo( pInfo->nRewardCashItemSN[REWARD_ITEM_INDEX] );
		if( NULL == pCashInfo )
			return;

		TItemInfo Info;
		if( false == CDnItem::MakeItemInfo( pCashInfo->presentItemId, pCashInfo->count, Info ) )
			return;

		CDnItem * pItem = CDnItemTask::GetInstance().CreateItem( Info );
		if( NULL == pItem )
			return;

		pItem->SetCashItemSN( pInfo->nRewardCashItemSN[REWARD_ITEM_INDEX] );

		m_pClearUI[nCount].m_pSlotButton->SetItem( pItem, pInfo->nRewardCashItemCount[REWARD_ITEM_INDEX] );
		m_pClearUI[nCount].m_pSlotButton->Show( true );
		m_pClearUI[nCount].m_pSlot_Static->Show( true );
	}
	else
	{
		if( 0 >= pInfo->nRewardItemID[REWARD_ITEM_INDEX] )
			return;

		TItemInfo Info;
		if( false == CDnItem::MakeItemInfo( pInfo->nRewardItemID[REWARD_ITEM_INDEX], 1, Info ) )
			return;

		CDnItem * pItem = CDnItemTask::GetInstance().CreateItem( Info );
		if( NULL == pItem )
			return;

		m_pClearUI[nCount].m_pSlotButton->SetItem( pItem, pInfo->nRewardItemCount[REWARD_ITEM_INDEX] );
		m_pClearUI[nCount].m_pSlotButton->Show( true );
		m_pClearUI[nCount].m_pSlot_Static->Show( true );
	}
}

void CDnAlteaCheckDlg::SClearUI::ResetSlot()
{
	MIInventoryItem * pItem = m_pSlotButton->GetItem();
	SAFE_DELETE( pItem );
	m_pSlotButton->ResetSlot();

	m_pCheck_Static->Show( false );
	m_pSlot_Static->Show( false );
	m_pSlotButton->Show( false );
}

void CDnAlteaCheckDlg::SClearUI::SetSlot( const int nItemID, const int nCount )
{
	CDnItem * pItem = CDnItem::CreateItem( nItemID, 0 );
	if( NULL == pItem )
		return;

	m_pSlotButton->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
}

#endif	// #if defined( PRE_ALTEIAWORLD_EXPLORE )