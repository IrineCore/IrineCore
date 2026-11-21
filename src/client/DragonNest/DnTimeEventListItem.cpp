#include "StdAfx.h"
#include "DnTimeEventListItem.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnCashShopTask.h"
#include "DnTimeEventDlg.h"


CDnTimeEventListItem::CDnTimeEventListItem( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	memset( m_pStaticTitle, 0, sizeof(m_pStaticTitle) );
	memset( m_pStaticRemainTime, 0, sizeof(m_pStaticRemainTime) );
	m_pStaticPeriod = NULL;
	for( int i=0; i<3; i++ ) m_pStaticIcon[i] = NULL;
	m_pStaticStatus = NULL;
	for( int i=0; i<3; i++ ) {
		m_pItemSlotReward[i] = NULL;
		m_pStaticItemBoard[i] = NULL;
//		m_pRewardItem[i] = NULL;
	}
	m_pStaticCoin = NULL;
	m_nEventID = 0;
	m_pTimeEventDlg = NULL;
	m_pCheckBox = NULL;
	m_pStaticDailyOnly = NULL;
}

CDnTimeEventListItem::~CDnTimeEventListItem()
{
	for( int i=0; i<3; i++ ) {
		if( m_pItemSlotReward[i] ) m_pItemSlotReward[i]->ResetSlot();
//		SAFE_DELETE( m_pRewardItem[i] );
	}
}

void CDnTimeEventListItem::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "EventTimeListDlg.ui" ).c_str(), bShow );
}

void CDnTimeEventListItem::InitialUpdate()
{
	char szStr[32];
	char *szIconName[3] = { "ID_STATIC_END", "ID_STATIC_WILLEND", "ID_STATIC_NOTEND" };

	m_pStaticTitle[0] = GetControl<CEtUIStatic>("ID_TEXT_TITLE");
	m_pStaticTitle[1] = GetControl<CEtUIStatic>("ID_TEXT_DAILYTITLE");

	m_pStaticRemainTime[0] = GetControl<CEtUIStatic>("ID_TEXT_REMAIN");
	m_pStaticRemainTime[1] = GetControl<CEtUIStatic>("ID_TEXT_DAILYREMAIN");

	m_pStaticDailyOnly = GetControl<CEtUIStatic>("ID_TEXT_DAILY");

	m_pStaticPeriod = GetControl<CEtUIStatic>("ID_TEXT_TIME");

	for( int i=0; i<3; i++ ) {
		m_pStaticIcon[i] = GetControl<CEtUIStatic>(szIconName[i]);
		m_pStaticIcon[i]->Show( false );
	}

	m_pStaticStatus = GetControl<CEtUIStatic>("ID_TEXT_NOTEND");
	m_pCheckBox = GetControl<CEtUICheckBox>("ID_CHECKBOX_SHOW");

	for( int i=0; i<3; i++ ) {
		sprintf_s( szStr, "ID_LIST_SLOT%d", i );
		m_pItemSlotReward[i] = GetControl<CDnItemSlotButton>(szStr);
		m_pItemSlotReward[i]->Show( false );
		sprintf_s( szStr, "ID_STATIC_SLOT%d", i );
		m_pStaticItemBoard[i] = GetControl<CEtUIStatic>(szStr);
		m_pStaticItemBoard[i]->Show( false );
	}
	m_pStaticCoin = GetControl<CEtUIStatic>("ID_STATIC_COIN");
	m_pStaticCoin->Show( false );
}

void CDnTimeEventListItem::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_CHECKBOX_CHANGED ) {
		if( IsCmdControl( "ID_CHECKBOX_SHOW" ) ) {
			if( ((CEtUICheckBox*)pControl)->IsChecked() ) {
				if( m_pTimeEventDlg ) m_pTimeEventDlg->InvertCheckOthers( this );
				CDnTimeEventTask::GetInstance().SetTimeEventAlarm( m_nEventID );
			}
			else {
				CDnTimeEventTask::GetInstance().SetTimeEventAlarm( 0 );
			}
		}
	}

	CDnCustomDlg::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnTimeEventListItem::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );

	if( !IsShow() ) return;
	if( !CDnTimeEventTask::IsActive() ) return;

	CDnTimeEventTask *pTask = &CDnTimeEventTask::GetInstance();
	if( pTask == NULL ) return;
	CDnTimeEventTask::TimeEventInfoStruct *pStruct = pTask->FindTimeEventInfo( m_nEventID );
	if( pStruct == NULL ) return;

	UpdateRemainTime( pStruct->nRemainTime, ( pStruct->RepeatType == CDnTimeEventTask::Daily ) ? true : false );
}

void CDnTimeEventListItem::SetInfo( CDnTimeEventDlg *pTimeEventDlg, CDnTimeEventTask::TimeEventInfoStruct *pStruct )
{
	m_pTimeEventDlg = pTimeEventDlg;
	m_nEventID = pStruct->nEventID;

	for( int i=0; i<2; i++ ) {
		m_pStaticTitle[i]->Show( false );
		m_pStaticRemainTime[i]->Show( false );
	}
	m_pStaticDailyOnly->Show( false );

	CEtUIStatic *pStaticTitle = ( pStruct->RepeatType == CDnTimeEventTask::Daily ) ? m_pStaticTitle[1] : m_pStaticTitle[0];
	switch( pStruct->RepeatType ) {
		case CDnTimeEventTask::None:
		case CDnTimeEventTask::Repeat:
			m_pStaticTitle[0]->Show( true );
			m_pStaticRemainTime[0]->Show( true );
			break;
		case CDnTimeEventTask::Daily:
			m_pStaticTitle[1]->Show( true );
			m_pStaticRemainTime[1]->Show( true );
			m_pStaticDailyOnly->Show( true );
			break;
	}

	pStaticTitle->SetText( pStruct->szTitle );
	pStaticTitle->SetTooltipText( pStruct->szDescription.c_str() );
	UpdateRemainTime( pStruct->nRemainTime, ( pStruct->RepeatType == CDnTimeEventTask::Daily ) ? true : false );

	WCHAR wszStr[256];
	_snwprintf_s( wszStr, _countof(wszStr), _TRUNCATE, L"%04d/%02d/%02d %02d:%02d - %04d/%02d/%02d %02d:%02d", pStruct->BeginTime.GetYear(), pStruct->BeginTime.GetMonth(), pStruct->BeginTime.GetDay(), pStruct->BeginTime.GetHour(), pStruct->BeginTime.GetMinute()
		, pStruct->EndTime.GetYear(), pStruct->EndTime.GetMonth(), pStruct->EndTime.GetDay(), pStruct->EndTime.GetHour(), pStruct->EndTime.GetMinute() );
	m_pStaticPeriod->SetText( wszStr );

	int nRewardOffset = 0;
	if( pStruct->nRewardCoin > 0 ) {
		m_pStaticCoin->Show( true );
		m_pStaticItemBoard[0]->Show( true );
		m_pItemSlotReward[0]->Show( true );
		nRewardOffset = 1;
	}
	/*
	for( DWORD i=0; i<pStruct->nVecRewardItemList.size(); i++ ) {
		int nRewardItemID = pStruct->nVecRewardItemList[i];
		if( nRewardItemID < 1 ) continue;

		TItemInfo Info;
		if( CDnItem::MakeItemInfo( nRewardItemID, 1, Info ) == false ) continue;

		m_pRewardItem[nRewardOffset] = CDnItemTask::GetInstance().CreateItem( Info );

		if( !m_pRewardItem[nRewardOffset] ) continue;

		m_pItemSlotReward[nRewardOffset]->SetItem( m_pRewardItem[nRewardOffset], CDnSlotButton::ITEM_ORIGINAL_COUNT );
		m_pItemSlotReward[nRewardOffset]->Show( true );
		m_pItemSlotReward[nRewardOffset]->SetRegist( true );
		m_pStaticItemBoard[nRewardOffset]->Show( true );
		nRewardOffset++;
		if( nRewardOffset >= 3 ) break;
	}
	for( DWORD i=0; i<pStruct->nVecRewardCashItemList.size(); i++ ) {
		int nRewardItemID = pStruct->nVecRewardCashItemList[i];
		if( nRewardItemID < 1 ) continue;

		const SCashShopItemInfo* pCashInfo = GetCashShopTask().GetItemInfo( nRewardItemID );
		if( !pCashInfo ) continue;

		TItemInfo Info;
		if( !CDnItem::MakeItemInfo( pCashInfo->presentItemId, pCashInfo->count, Info ) ) continue;

		m_pRewardItem[nRewardOffset] = CDnItemTask::GetInstance().CreateItem( Info );

		if( !m_pRewardItem[nRewardOffset] ) continue;

		m_pRewardItem[nRewardOffset]->SetCashItemSN( nRewardItemID );
		m_pItemSlotReward[nRewardOffset]->SetItem( m_pRewardItem[nRewardOffset], CDnSlotButton::ITEM_ORIGINAL_COUNT );
		m_pItemSlotReward[nRewardOffset]->Show( true );
		m_pItemSlotReward[nRewardOffset]->SetRegist( true );
		m_pStaticItemBoard[nRewardOffset]->Show( true );
		nRewardOffset++;
		if( nRewardOffset >= 3 ) break;
	}
	*/
	for( DWORD i=0; i<pStruct->pVecRewardItemList.size(); i++ ) {
		m_pItemSlotReward[nRewardOffset]->SetItem( pStruct->pVecRewardItemList[i], pStruct->VecRewardItemCountList[i] );
		m_pItemSlotReward[nRewardOffset]->Show( true );
		m_pItemSlotReward[nRewardOffset]->SetRegist( true );
		m_pStaticItemBoard[nRewardOffset]->Show( true );

		nRewardOffset++;
		if( nRewardOffset >= 3 ) break;
	}
}

void CDnTimeEventListItem::UpdateRemainTime( INT64 nRemainTime, bool bDailyOnly )
{
	WCHAR wszStr[256];
	CEtUIStatic *pStaticRemain = ( bDailyOnly ) ? m_pStaticRemainTime[1] : m_pStaticRemainTime[0];

	INT64 nValue = nRemainTime / 1000;
	if( nRemainTime == -1 ) {
		pStaticRemain->SetText( L"-" );
		m_pStaticStatus->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7713 ) );
	}
	else {
		int nHour = (int)( nValue / 3600 );
		int nMin = (int)( ( nValue % 3600 ) / 60 );
		int nSec = (int)( nValue % 60 );
		swprintf_s( wszStr, L"%02d:%02d:%02d", nHour, nMin, nSec );
		pStaticRemain->SetText( wszStr );
		m_pStaticStatus->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7715 ) );
	}

	m_pCheckBox->Enable( true );
	for( int i=0; i<3; i++ ) m_pStaticIcon[i]->Show( false );
	if( nRemainTime > 5 * 60 * 1000 ) {
		m_pStaticIcon[2]->Show( true );
	}
	else if( nRemainTime <= 0 ) {
		m_pStaticIcon[0]->Show( true );
		m_pCheckBox->Enable( false );
	}
	else m_pStaticIcon[1]->Show( true );
}

void CDnTimeEventListItem::SetCheckBox( bool bCheck )
{
	if( m_pCheckBox ) {
		m_pCheckBox->SetChecked( bCheck );
	}
}
