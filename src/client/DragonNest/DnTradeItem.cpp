#include "StdAfx.h"
#include "DnTradeItem.h"
#include "DnStoreTabDlg.h"
#include "DnInterface.h"
#include "DnItemTask.h"
#include "DnWeapon.h"
#include "DnParts.h"
#include "TradeSendPacket.h"
#include "DnInvenTabDlg.h"
#include "DnMainMenuDlg.h"
#include "DnLocalPlayerActor.h"
#include "DnNPCActor.h"
#include "DnTableDB.h"
#include "DnSlotButton.h"
#include "DnPartyTask.h"
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "DnQuestTask.h"
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
#include "DnGuildWarTask.h"
#include "TaskManager.h"
#include "DnCommonUtil.h"
#ifdef PRE_FIX_LADDERSHOP_SKILL
#include "DnBlow.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnTradeItem::CDnTradeItem(void)
	: m_bRequestWait(false)
	, m_nRequestSrcItemSlot(-1)
	, m_nRequestDestItemSlot(-1)
	, m_pStoreDialog(NULL)
	, m_pTradeSellItem(NULL)
	, m_pTradeBuyItem(NULL)
	, m_nSellSoundIndex(-1)
{
}

CDnTradeItem::~CDnTradeItem(void)
{
	DeleteStoreData();
}

bool CDnTradeItem::Initialize()
{
	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10003 );
	if( strlen( szFileName ) > 0 )
		m_nSellSoundIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );

	return true;
}

void CDnTradeItem::Finalize()
{
	CEtSoundEngine::GetInstance().RemoveSound( m_nSellSoundIndex );
}

void CDnTradeItem::DeleteStoreData()
{
	for( int i = 0; i < (int)m_vecStoreData.size(); ++i ) {
		SStoreTabData &TabData = m_vecStoreData[i];
		for( int j = 0; j < (int)TabData.vecStoreItem.size(); ++j ) {
			SAFE_DELETE( TabData.vecStoreItem[j] );
		}
		TabData.vecStoreItem.clear();
	}
	m_vecStoreData.clear();

	for( DWORD itr = 0; itr < m_vecCombinedStoreData.size(); ++itr )
		m_vecCombinedStoreData[itr].vecCombineStoreItem.clear();

	m_vecCombinedStoreData.clear();
}

void CDnTradeItem::OnRecvShopOpen( SCShopOpen *pPacket )
{
	if( Shop::Type::Combined <= pPacket->Type )	// If it is the revamped shop
	{
		CombinedShopOpen( pPacket->nShopID );
	}
	else
	{
		int nOpenShopID = pPacket->nShopID;
		if( nOpenShopID <= 0 ) return;
		bool bInvalidShopID = true;

		DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TSHOP );
		DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
		if( pSox ) {
			for( int i=0; i<pSox->GetItemCount(); i++ ) {
				int nTableID = pSox->GetItemID(i);
				int nShopID = pSox->GetFieldFromLablePtr( nTableID, "_ShopID" )->GetInteger();

				if( nOpenShopID == nShopID ) {
					bInvalidShopID = false;
					break;
				}
			}
		}
		if( bInvalidShopID ) return;

		OpenShopSuccess();

		CDnItem *pItem(NULL);

		// Who should own the CDnItem objects for items going into the shop.
		// Like typical UI handling, the UIs owning ItemSlotButton could hold it instead, though,
		// Doing this means each store slot button must hold a linked CDnItem.
		// And when deleting, must find each one and clear it when ItemSlotButton->ResetSlot is called.
		//
		// Whereas ItemSlotButton in other UIs are few and handled individually,
		// Given that the shop item list can all be received in one load,
		// Just have TradeItem hold all of them.

		for( int iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID ) {
			int nTableID = pSox->GetItemID(iItemID);
			int nShopID = pSox->GetFieldFromLablePtr( nTableID, "_ShopID" )->GetInteger();
			if( nOpenShopID != nShopID ) continue;

			int nTabID = pSox->GetFieldFromLablePtr( nTableID, "_TabID" )->GetInteger();
			if( nTabID < 1 || nTabID > SHOPTABMAX )	continue; 

			int nTabUIStringID = pSox->GetFieldFromLablePtr( nTableID, "_TabNameID" )->GetInteger();
			m_pStoreDialog->SetTabUIStringID( nTabID, nTabUIStringID );

			char szTemp[128];
			int nItemID;
			short wCount;
			int nMaxOverlapCount;
			for( int i = 0; i < SHOPITEMMAX; ++i ) {
				TItemInfo itemInfo;
				sprintf_s( szTemp, _countof(szTemp), "_itemindex%d", i+1 );
				nItemID = pSox->GetFieldFromLablePtr( nTableID, szTemp )->GetInteger();
				if( nItemID <= 0 ) continue;
				sprintf_s( szTemp, _countof(szTemp), "_Quantity%d", i+1 );
				wCount = pSox->GetFieldFromLablePtr( nTableID, szTemp )->GetInteger();
				nMaxOverlapCount = pItemSox->GetFieldFromLablePtr( nItemID, "_OverlapCount" )->GetInteger();
				if( wCount > nMaxOverlapCount ) wCount = nMaxOverlapCount;

				CDnItem::MakeItemInfo( nItemID, wCount, itemInfo );
				itemInfo.cSlotIndex = (BYTE)i;
				pItem = CDnItemTask::GetInstance().CreateItem( itemInfo );
				if( pItem == NULL ) continue;
				SetItemToDlgAndStoreDataVector(nTabID, pItem);
			}
		}
	}
#ifdef PRE_ADD_PVPRANK_INFORM
	GetInterface().OpenStoreDialog(pPacket->Type, pPacket->nShopID);
#else
	GetInterface().OpenStoreDialog(pPacket->Type);
#endif
}

void CDnTradeItem::SetItemToDlgAndStoreDataVector(int nTabID, CDnItem* pItem)
{
	int vecStoreDataSize = (int)m_vecStoreData.size();
	if (m_pStoreDialog == NULL || pItem == NULL || (nTabID-1) >= vecStoreDataSize || vecStoreDataSize < 0)
		return;

	m_vecStoreData[nTabID-1].vecStoreItem.push_back(pItem);
	m_pStoreDialog->SetStoreItem(nTabID, pItem);	// slotIndex is stored in pItem.
}

void CDnTradeItem::OnRecvShopBuy( SCShopBuyResult *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		if( ERROR_ITEM_INSUFFICIENTY_JOBID == pPacket->nRet )
			GetInterface().MessageBox( 1759 );	// Cannot purchase because job requirement is not met
		else if( ERROR_ITEM_INSUFFICIENTY_LEVEL == pPacket->nRet )
			GetInterface().MessageBox( 1760 );	// Cannot purchase because level requirement is not met
		else if( ERROR_ITEM_INSUFFICIENTY_PVPRANK == pPacket->nRet )
			GetInterface().MessageBox( 1761 );	// Cannot purchase because Colosseum rank requirement is not met
		else if( ERROR_ITEM_INSUFFICIENTY_GUILDLEVEL == pPacket->nRet )
			GetInterface().MessageBox( 1762 );	// Cannot purchase because guild level requirement is not met
		else if( ERROR_ITEM_INSUFFICIENTY_LADDERGRADEPOINT == pPacket->nRet )
			GetInterface().MessageBox( 1763 );	// Cannot purchase because ladder rating requirement is not met
		else
			GetInterface().ServerMessageBox(pPacket->nRet);
	}

	m_nRequestSrcItemSlot = -1;
	m_bRequestWait = false;
}

void CDnTradeItem::OnRecvShopSell( SCShopSellResult *pPacket )
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		if( m_nSellSoundIndex != -1 )
			CEtSoundEngine::GetInstance().PlaySound( "2D", m_nSellSoundIndex );
	}

	CDnStoreTabDlg *pStoreDlg = static_cast<CDnStoreTabDlg*>(GetInterface().GetMainMenuDialog(CDnMainMenuDlg::STORE_DIALOG));
	if (pStoreDlg && pStoreDlg->IsShow())
		pStoreDlg->OnStoreSell();

	m_nRequestSrcItemSlot = -1;
	m_bRequestWait = false;
}

void CDnTradeItem::OnRecvShopGetRepurchaseList(SCShopRepurchaseList* pPacket)
{
	if (pPacket->nRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
	else
	{
		m_StoreRepurchaseSystem.SetRepurchaseInfo(pPacket->cCount, pPacket->List);
	}
}

void CDnTradeItem::OnRecvShopRepurchase(SCShopRepurchase* pPacket)
{
	if (pPacket->iRet != ERROR_NONE)
	{
		GetInterface().ServerMessageBox(pPacket->iRet);
	}
	else
	{
		m_StoreRepurchaseSystem.OnRepurchaseSuccess(pPacket->iRepurchaseID);
	}
}

void CDnTradeItem::SetStoreDialog( CDnStoreTabDlg *pDialog)
{
	m_pStoreDialog = pDialog;
	m_StoreRepurchaseSystem.SetStoreDialog(pDialog);
}

CDnItem* CDnTradeItem::CreateItem( int nItemID, short wCount, BYTE cSlotIndex )
{
	eItemTypeEnum Type = CDnItem::GetItemType( nItemID );
	CDnItem *pItem = NULL;

	if( nItemID == 0 ) 
		return NULL;

	switch( Type ) 
	{
	case ITEMTYPE_WEAPON:
		{
			pItem = CDnWeapon::CreateWeapon( nItemID, 0, 0, 0, 0, 0, false, false, false );
			//((CDnWeapon*)pItem)->SetDurability( (float)Info.wDur );
		}
		break;
	case ITEMTYPE_PARTS:
		{
			pItem = CDnParts::CreateParts( nItemID, 0, 0 );
			//((CDnParts*)pItem)->SetDurability( (float)Info.wDur );
		}
		break;
	case ITEMTYPE_NORMAL:
	case ITEMTYPE_COOKING:
	case ITEMTYPE_SKILL:
	case ITEMTYPE_INSTANT:
	case ITEMTYPE_JEWEL:
	case ITEMTYPE_PLATE:
	case ITEMTYPE_RANDOM:
	case ITEMTYPE_SKILLBOOK:
		pItem = CDnItem::CreateItem( nItemID, 0 );
		break;
	}

	if( pItem == NULL )
	{
		assert( 0&&"Failed to create item!!" );
		return NULL;
	}

	pItem->SetSerialID( 0 );
	pItem->SetOverlapCount( wCount );
	pItem->SetSlotIndex( cSlotIndex );

	if( pItem->GetReversion() > CDnItem::NoReversion )
		pItem->SetSoulBound( true );

	return pItem;
}

void CDnTradeItem::RequestShopBuy( int nTabID, BYTE cSlotIndex, int nCount )
{
	CDnItem *pItem = GetStoreItem( nTabID, cSlotIndex );
	if( !pItem ) return;

	CDnStoreTabDlg *pStoreDlg = (CDnStoreTabDlg*)GetInterface().GetMainMenuDialog( CDnMainMenuDlg::STORE_DIALOG );
	if( pStoreDlg && pStoreDlg->IsShow() && Shop::Type::Combined <= pStoreDlg->GetStoreType() )
	{
		if( false == CheckCombinedShopBuy( nTabID, cSlotIndex, nCount ) )
			return;
#if defined( PRE_ADD_LIMITED_SHOP )
		CDnTradeItem::TCombinedShopTableData * pData = GetCombinedStoreItem( nTabID, cSlotIndex );
		int nLimitCount = pData->nBuyLimitCount;
		if( nLimitCount > 0 && pData->nShopLimitReset > 0 )
		{
#if defined( PRE_FIX_74404 )
			int nCurrentItemBuyCount = GetItemTask().GetLimitedShopItemCount( pData->nShopID, pItem->GetClassID() );
#else// #if defined( PRE_FIX_74404 )
			int nCurrentItemBuyCount = GetItemTask().GetLimitedShopItemCount( pItem->GetClassID() );
#endif// #if defined( PRE_FIX_74404 )
			if( nLimitCount - nCurrentItemBuyCount <= 0 )
			{
				GetInterface().MessageBox( 7618, MB_OK );	// UISTRING : Purchase limit exceeded; cannot buy any more.
				return;
			}
		}
#endif	// #if defined( PRE_ADD_LIMITED_SHOP )
	}
	else
	{
#ifdef PRE_ADD_NPC_REPUTATION_SYSTEM
		// Note : Check amount
		//
		int iNeedCoin = pItem->GetItemAmount() * nCount;
		GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::BuyingPriceDiscount, iNeedCoin );

		if( iNeedCoin > GetItemTask().GetCoin() )
		{
			GetInterface().MessageBox( 1706, MB_OK );
			return;
		}
#else
		// Note : Check amount
		//
		if( (INT64)pItem->GetItemAmount() * nCount > GetItemTask().GetCoin() )
		{
			GetInterface().MessageBox( 1706, MB_OK );
			return;
		}
#endif // #ifdef PRE_ADD_NPC_REPUTATION_SYSTEM

		// Check required items
		if( pItem->GetNeedBuyItem() && pItem->GetNeedBuyItemCount() )
		{
			int nCurItemCount = GetItemTask().GetCharInventory().GetItemCount( pItem->GetNeedBuyItem() );
			if( nCurItemCount < pItem->GetNeedBuyItemCount() * nCount / pItem->GetOverlapCount() )
			{
				GetInterface().MessageBox( 1745, MB_OK );
				return;
			}
		}

		if( pItem->GetNeedBuyLadderPoint() )
		{
#ifdef PRE_FIX_LADDERSHOP_SKILL
			int iNeedLadderPoint = CalcPurchaseLadderPoint(pItem->GetNeedBuyLadderPoint()) * nCount;
#else
			int iNeedLadderPoint = pItem->GetNeedBuyLadderPoint() * nCount;
#endif
			int nCurLadderPoint = 0;

			// Ladder shop points 
			if(CDnPartyTask::IsActive())
				nCurLadderPoint = CDnPartyTask::GetInstance().GetPVPLadderInfo()->Data.iPvPLadderPoint;
			if( nCurLadderPoint < iNeedLadderPoint )
			{
				GetInterface().MessageBox( 126192, MB_OK );
				return;
			}
		}

		if( pItem->GetNeedBuyGuildWarPoint() )
		{
			int nNeedGuildWarPoint = pItem->GetNeedBuyGuildWarPoint() * nCount;
			int nCurGuildWarPoint = 0;

			if(CDnGuildWarTask::IsActive())
				nCurGuildWarPoint = (int)CDnGuildWarTask::GetInstance().GetGuildWarFestivalPoint();
			if( nCurGuildWarPoint < nNeedGuildWarPoint )
			{
				GetInterface().MessageBox( 126192, MB_OK );
				return;
			}
		}
	}

	m_nRequestSrcItemSlot = cSlotIndex;
	m_bRequestWait = true;

	SendShopBuy(nTabID, cSlotIndex, nCount);
}

bool CDnTradeItem::IsShopSellableItem(const CDnItem& item, int count) const
{
	if (item.GetItemType() == ITEMTYPE_QUEST ||
		item.GetItemSellAmount() <= 0 ||
		count <= 0 )
	{
		return false;
	}

	return true;
}

bool CDnTradeItem::RequestShopSell(const CDnItem& sellItem, int count)//BYTE cInvenIndex, int nCount )
{
	BYTE slotIdx = (BYTE)sellItem.GetSlotIndex();
	if (IsShopSellableItem(sellItem, count) == false)
	{
		GetInterface().ServerMessageBox(ERROR_SHOP_NOTFORSALE);
		return false;
	}

#ifdef PRE_MOD_SELL_SEALEDITEM
#else
	if( sellItem.GetReversion() > CDnItem::NoReversion && !sellItem.IsSoulbBound() )
	{
		GetInterface().MessageBox( 1751, MB_OK );
		return false;
	}
#endif	// #ifdef PRE_MOD_SELL_SEALEDITEM

	m_pStoreDialog->OnStoreSell();

	m_nRequestSrcItemSlot = slotIdx;
	m_bRequestWait = true;

	SendShopSell(slotIdx, count, sellItem.GetSerialID());

	return true;
}

CDnItem* CDnTradeItem::GetStoreItem( int nTabID, int nSlotIndex )
{
	// TabID is a value between 1 and 6.
	int nIndex = nTabID-1;
	if( nIndex < 0 || nIndex >= (int)m_vecStoreData.size() )
		return NULL;

	SStoreTabData &TabData = m_vecStoreData[nIndex];		
	for( int j = 0; j < (int)TabData.vecStoreItem.size(); ++j ) {
		if( nSlotIndex == TabData.vecStoreItem[j]->GetSlotIndex() )
			return TabData.vecStoreItem[j];
	}

	return NULL;
}

void CDnTradeItem::DeleteAllStore()
{
	DeleteStoreData();
}

bool CDnTradeItem::IsExpensiveItem( CDnItem *pItem )
{
	// Note: Sale price is 1,000,000 or more
	//
	if( pItem->GetItemAmount() >= 1000000 )
		return true;

	// Note : Item grade is B or higher
	//
	if( pItem->GetItemRank() >= ITEMRANK_B )
		return true;

	// Note : Item with heraldry enchantment
	//

	return false;	
}

void CDnTradeItem::OpenShopSuccess()
{
	DWORD nUID = CDnLocalPlayerActor::GetTakeNpcUID();
	DnActorHandle hNpc = CDnActor::FindActorFromUniqueID(nUID);

	if ( hNpc ) {
		CDnNPCActor* pActor = dynamic_cast<CDnNPCActor*>(hNpc.GetPointer());
		if ( pActor ) {
			pActor->OnSoundPlay( CDnNPCActor::OpenShop );
		}
	}

	m_pStoreDialog->ResetAllListSlot();
	DeleteStoreData();
	m_vecStoreData.resize(SHOPTABMAX);	// Allocate space per tab.

	m_vecCombinedStoreData.resize(SHOPTABMAX);
}

void CDnTradeItem::CombinedShopOpen( int nShopID )
{
	int nOpenShopID = nShopID;
	if( nOpenShopID <= 0 ) return;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCOMBINEDSHOP );
	DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox )
		return;

	bool bInvalidShopID = true;
	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		int nTableID = pSox->GetItemID( itr );
		int nShopID = pSox->GetFieldFromLablePtr( nTableID, "_ShopID" )->GetInteger();

		if( nOpenShopID == nShopID )
		{
			bInvalidShopID = false;
			break;
		}
	}
	if( bInvalidShopID ) return;

	OpenShopSuccess();

	CDnItem *pItem(NULL);
	for( int itr = 0; itr < pSox->GetItemCount(); ++itr )
	{
		int nTableID = pSox->GetItemID( itr );
		int nShopID = pSox->GetFieldFromLablePtr( nTableID, "_ShopID" )->GetInteger();

		if( nOpenShopID != nShopID )
			continue;

		int nTabID = pSox->GetFieldFromLablePtr( nTableID, "_TabID" )->GetInteger();
		if( nTabID < 1 || nTabID > SHOPTABMAX )	
			continue; 

		int nTabUIStringID = pSox->GetFieldFromLablePtr( nTableID, "_TabNameID" )->GetInteger();
#if !defined(PRE_ADD_TALISMAN_SYSTEM)
		if( nTabUIStringID == 1000054149 )
			continue;
#endif
		m_pStoreDialog->SetTabUIStringID( nTabID, nTabUIStringID );

		int nListID = pSox->GetFieldFromLablePtr( nTableID, "_ListID" )->GetInteger();
		int nItemID = pSox->GetFieldFromLablePtr( nTableID, "_itemindex" )->GetInteger();
		int nCount = pSox->GetFieldFromLablePtr( nTableID, "_Quantity" )->GetInteger();

		if( nItemID <= 0 ) 
			continue;

		TItemInfo itemInfo;
		CDnItem::MakeItemInfo( nItemID, nCount, itemInfo );
		itemInfo.cSlotIndex = static_cast<BYTE>(nListID);
		pItem = CDnItemTask::GetInstance().CreateItem( itemInfo );
		if( pItem == NULL ) 
			continue;

		m_vecStoreData[nTabID-1].vecStoreItem.push_back(pItem);
		m_pStoreDialog->SetStoreItem( nTabID, pItem );	// slotIndex is stored in pItem.

		TCombinedShopTableData Data;
		Data.PurchaseLimitType = static_cast<Shop::PurchaseLimitType::eCode>( pSox->GetFieldFromLablePtr( nTableID, "_PurchaseLimitType" )->GetInteger() );
		Data.iPuschaseLimitValue = pSox->GetFieldFromLablePtr( nTableID, "_PurchaseLimitValue" )->GetInteger();
#if defined( PRE_ADD_LIMITED_SHOP )
		Data.nBuyLimitCount = pSox->GetFieldFromLablePtr( nTableID, "_buyLimitCount" )->GetInteger();
		Data.nShopLimitReset = pSox->GetFieldFromLablePtr( nTableID, "_ShopLimitReset" )->GetInteger();
#if defined( PRE_FIX_74404 )
		Data.nShopID = nOpenShopID;
#endif// #if defined( PRE_FIX_74404 )
#endif

		char szFieldName[256];
		for( int jtr = 0; jtr < Shop::Max::PurchaseType; ++jtr )
		{
			sprintf_s( szFieldName, _countof(szFieldName), "_PurchaseType%d", jtr + 1 );
			Data.PurchaseType[jtr].PurchaseType = static_cast<Shop::PurchaseType::eCode>( pSox->GetFieldFromLablePtr( nTableID, szFieldName )->GetInteger() );

			sprintf_s( szFieldName, _countof(szFieldName), "_PurchaseItem%d", jtr + 1 );
			Data.PurchaseType[jtr].iPurchaseItemID = pSox->GetFieldFromLablePtr( nTableID, szFieldName )->GetInteger();

			sprintf_s( szFieldName, _countof(szFieldName), "_PurchaseItemValue%d", jtr + 1 );
			Data.PurchaseType[jtr].iPurchaseItemValue = pSox->GetFieldFromLablePtr( nTableID, szFieldName )->GetInteger();
		}

		m_vecCombinedStoreData[nTabID - 1].vecCombineStoreItem.push_back( Data );	
	}
}

CDnTradeItem::TCombinedShopTableData * CDnTradeItem::GetCombinedStoreItem( int nTabID, int nSlotIndex )
{
	// TabID is a value between 1 and 6.
	int nIndex = nTabID-1;
	if( nIndex < 0 || ( nIndex > (int)m_vecStoreData.size() && nIndex > (int)m_vecCombinedStoreData.size() ))
		return NULL;

	SStoreTabData &TabData = m_vecStoreData[nIndex];		
	for( DWORD itr = 0; itr < TabData.vecStoreItem.size(); ++itr ) 
	{
		if( nSlotIndex == TabData.vecStoreItem[itr]->GetSlotIndex() )
		{
			if( itr >= m_vecCombinedStoreData[nIndex].vecCombineStoreItem.size() )
				return NULL;

			return &m_vecCombinedStoreData[nIndex].vecCombineStoreItem[itr];
		}
	}

	return NULL;
}

bool CDnTradeItem::CheckCombinedShopBuy( int nTabID, BYTE cSlotIndex, int nCount )
{
	CDnTradeItem::TCombinedShopTableData * pData = GetCombinedStoreItem( nTabID, cSlotIndex );

	if( NULL == pData )
	{
		DN_ASSERT( NULL != pData, "Combined Shop Item Invalid!!" );
		return false;
	}

	for( int itr = 0; itr < Shop::Max::PurchaseType; ++itr )
	{
		if( Shop::PurchaseType::Gold == pData->PurchaseType[itr].PurchaseType )
		{
			int storeBenefitAmount = pData->PurchaseType[itr].iPurchaseItemValue;
			GetQuestTask().CheckAndCalcStoreBenefit( NpcReputation::StoreBenefit::BuyingPriceDiscount, storeBenefitAmount );

			if( GetItemTask().GetCoin() < storeBenefitAmount )
			{
				GetInterface().MessageBox( 1706, MB_OK );	// UISTRING : Not enough money to purchase.
				return false;
			}
		}
		else if( Shop::PurchaseType::ItemID == pData->PurchaseType[itr].PurchaseType )
		{
			const int nInvenCount = GetItemTask().GetCharInventory().GetItemCount( pData->PurchaseType[itr].iPurchaseItemID );

			if( nInvenCount < pData->PurchaseType[itr].iPurchaseItemValue * nCount )
			{
				GetInterface().MessageBox( 1745, MB_OK );	// UISTRING : Insufficient items required for purchase.
				return false;
			}
		}
		else if( Shop::PurchaseType::LadderPoint == pData->PurchaseType[itr].PurchaseType )
		{
			const int nMyPoint = CDnPartyTask::GetInstance().GetPVPLadderInfo()->Data.iPvPLadderPoint;

	#ifdef PRE_FIX_LADDERSHOP_SKILL
			int nPurchaseItemValue = CalcPurchaseLadderPoint(pData->PurchaseType[itr].iPurchaseItemValue);
			if (nMyPoint < nPurchaseItemValue)
	#else
			if( nMyPoint < pData->PurchaseType[itr].iPurchaseItemValue )
	#endif
			{
				GetInterface().MessageBox( 126192, MB_OK );	// UISTRING : Insufficient points required for purchase.
				return false;
			}
		}
		else if( Shop::PurchaseType::GuildPoint == pData->PurchaseType[itr].PurchaseType )
		{
			const int nMyPoint = static_cast<int>( CDnGuildWarTask::GetInstance().GetGuildWarFestivalPoint() );

			if( nMyPoint < pData->PurchaseType[itr].iPurchaseItemValue )
			{
				GetInterface().MessageBox( 126192, MB_OK );	// UISTRING : Insufficient points required for purchase.
				return false;
			}
		}
		else if( Shop::PurchaseType::UnionPoint== pData->PurchaseType[itr].PurchaseType )
		{
#if defined(PRE_UNIONSHOP_RENEWAL)
			const int eType = pData->PurchaseType[itr].iPurchaseItemID;
			if( NpcReputation::UnionType::Etc <= eType || eType < NpcReputation::UnionType::Commercial )
				return false;

			CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
			if( !pQuestTask ) 
				return false;

			CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
			if (pReputationRepos == NULL)
				return false;

			const int nMyPoint = (const int)pReputationRepos->GetUnionPoint( eType );

			if( nMyPoint < pData->PurchaseType[itr].iPurchaseItemValue )
			{
				GetInterface().MessageBox( 3275, MB_OK );	// UISTRING : Insufficient Union Points to purchase.
				return false;
			}
#endif	//	#if defined(PRE_UNIONSHOP_RENEWAL)
		}
	}

	return true;
}

#ifdef PRE_FIX_LADDERSHOP_SKILL
int CDnTradeItem::CalcPurchaseLadderPoint(int nPoint) const
{
	//Ladder points n% discount...
	float nLadderRate = 0.0f;
	if (CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_263))
	{
		DNVector(DnBlowHandle) vlhBlows;
		CDnActor::s_hLocalActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_263, vlhBlows );
		int iNumBlow = (int)vlhBlows.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			nLadderRate += vlhBlows[i]->GetFloatValue();
		}

		nPoint = (int)(nPoint - nPoint * nLadderRate);
	}

	return nPoint;
}
#endif