#include "stdafx.h"
#include "DNUserRepository.h"
#include "DNConnection.h"
#include "DNExtManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "DNSQLManager.h"
#include "Log.h"
#include "Util.h"
#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#include "DNBillingConnection.h"

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
void ReleaseExchange(USER_REQUEST volatile* pDestination)
{
#if defined (_WIN64)
	InterlockedExchange64(pDestination, CASH_NO_REQUESTING);
#else
	InterlockedExchange(pDestination, CASH_NO_REQUESTING);
#endif // #if defined (_WIN64)
}
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

static int DeductCashAndMapError(CDNSQLMembership *pMembershipDB, int nAccountDBID, INT64 biOrderID, int nPrice, int &nCashBalance)
{
	if (nPrice <= 0)
		return ERROR_NONE;

	int nCashOutgoID = 0;
	int nRet = pMembershipDB->QueryDeductCash(nAccountDBID, biOrderID, nPrice, nCashOutgoID, nCashBalance);
	if (nRet == 101198)
		return ERROR_CASHSHOP_CASH_NOTENOUGH;
	return nRet;
}

static int CheckCashBalanceSufficient(CDNSQLMembership *pMembershipDB, int nAccountDBID, int nRequiredPrice)
{
	if (!pMembershipDB)
		return ERROR_GENERIC_INVALIDREQUEST;
	int nCashBalance = 0;
	int nRet = pMembershipDB->QueryGetCashBalance(nAccountDBID, NULL, nCashBalance);
	if (nRet != ERROR_NONE)
		return nRet;
	if (nCashBalance < nRequiredPrice)
		return ERROR_CASHSHOP_CASH_NOTENOUGH;
	return ERROR_NONE;
}

CDNUserRepository::CDNUserRepository(void)
{
	m_cPaymentRules = 0;

	m_nAccountDBID = 0;
	memset(&m_szAccountName, 0, sizeof(m_szAccountName));
	memset(&m_wszAccountName, 0, sizeof(m_wszAccountName));
	memset(&m_szCharacterName, 0, sizeof(m_szCharacterName));
	memset(&m_wszCharacterName, 0, sizeof(m_wszCharacterName));
	m_biCharacterDBID = 0;

	m_nIp = 0;
	memset(&m_szIp, 0, sizeof(m_szIp));
	memset(&m_wszIp, 0, sizeof(m_wszIp));

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
	m_Requesting = CASH_NO_REQUESTING;
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

	Clear();
}

CDNUserRepository::~CDNUserRepository(void)
{
	Clear();
}

void CDNUserRepository::Clear()
{
	m_pConnection = NULL;
	m_nThreadID = 0;

	m_cWorldSetID = 0;
	m_nMapID = 0;
	m_nChannelID = 0;

	m_bOpen = false;	
	m_cAge = 0;
	m_bPCBang = false;
	m_cBuyCartType = m_cGiftCartType = 0;		// Purchase cart type
	m_nPackageSN = 0;

	m_biPurchaseOrderID = 0;
	m_nPickupCount = 0;

	m_nTotalPrice = 0;
	m_nUsedPaidCash = 0;

	m_nSubCommand = 0;
	m_cProductCount = 0;
	memset(&m_ProductPackageList, 0, sizeof(m_ProductPackageList));

	m_VecBuyItemList.clear();

	memset(&m_GiftPackageList, 0, sizeof(m_GiftPackageList));
	m_VecGiftItemList.clear();

	m_biCouponOrderID = 0;
	m_nCouponSN = 0;

	memset(&m_wszReceiverCharacterName, 0, sizeof(m_wszReceiverCharacterName));
	memset(&m_wszMessage, 0, sizeof(m_wszMessage));

	memset(&m_wszCoupon, 0, sizeof(m_wszCoupon));
	memset(&m_szCoupon, 0, sizeof(m_szCoupon));

	memset(&m_VIP, 0, sizeof(TVIPItem));
	m_bAutoPay = false;
	m_bServer = false;

#if defined(PRE_ADD_CASH_REFUND)
	m_biPurchaseOrderDetailID = 0;
	memset(&m_PaymentItem, 0, sizeof(m_PaymentItem));	
	memset(&m_PaymentPackageItem, 0, sizeof(TPaymentPackageItemInfoEx));
	m_bCashMoveInven = false;
	m_cRefundType = 0;
#endif

#if defined(PRE_ADD_SALE_COUPON)
	m_biSaleCouponSerial = 0;
#endif
#if defined(PRE_ADD_CADGE_CASH)
	m_nMailDBID = 0;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

	m_cCurProductCount = 0;
}

void CDNUserRepository::SetPurchaseOrderDetailID(const char* pPurchaseOrderDetailIDs)
{
	switch (m_nSubCommand)
	{
	case CASH_BUY:
		{
			if (m_VecBuyItemList.empty())
				return;

			// Parsing
			string strOrderDetails = pPurchaseOrderDetailIDs;
			
			std::vector<std::string> tokens;
			TokenizeA(strOrderDetails.c_str(), tokens, ",");

			std::vector<std::string>::iterator ii;
			int nCount = 0;
			for (ii = tokens.begin(); ii != tokens.end(); ii++)
			{
				m_VecBuyItemList[nCount].biPurchaseOrderDetailID = _atoi64((*ii).c_str());
				++nCount;		
			}
		}
		break;

	case CASH_PACKAGEBUY:
		{
#if defined(PRE_ADD_CASH_REFUND)
			m_biPurchaseOrderDetailID = _atoi64(pPurchaseOrderDetailIDs);
#endif
		}
		break;
	}
}

int CDNUserRepository::GetPetalBalance()
{
	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB) return 0;

	int nPetalBalance = 0;
	pMembershipDB->QueryGetPetalBalance(m_nAccountDBID, nPetalBalance);

	return nPetalBalance;
}

CDNSQLMembership *CDNUserRepository::GetMembershipDB()
{
	CDNSQLMembership *pMembershipDB = g_pSQLManager->FindMembershipDB(m_nThreadID);
	if (!pMembershipDB)
		g_Log.Log(LogType::_ERROR, this, L"[%d] pMembershipDB not found\r\n", m_nSubCommand);

	return pMembershipDB;
}

CDNSQLWorld *CDNUserRepository::GetWorldDB()
{
	CDNSQLWorld *pWorldDB = g_pSQLManager->FindWorldDB(m_nThreadID, m_cWorldSetID);
	if (!pWorldDB)
		g_Log.Log(LogType::_ERROR, this, L"[%d] pWorldDB not found\r\n", m_nSubCommand);

	return pWorldDB;
}

bool CDNUserRepository::SetBuyItem(TCashBuyItem &Item)
{
	TBuyItem BuyItem;
	memset(&BuyItem, 0, sizeof(TBuyItem));

	BuyItem.BuyItem = Item;
	BuyItem.nReservePetal = g_pExtManager->GetCashCommodityReserve(Item.nItemSN);
	BuyItem.nPrice = g_pExtManager->GetCashCommodityPrice(Item.nItemSN);
	BuyItem.nLimitCount = g_pExtManager->GetCashBuyAbleCount(Item.nItemSN);
	BuyItem.BuyItem.bFail = true;	// Set default to fail... set to true only when properly acquired.


#if defined(PRE_ADD_SALE_COUPON)
	if( BuyItem.nPrice <= 0 && !g_pExtManager->GetCashCommodityUseCoupon(Item.nItemSN))
#else
	if( BuyItem.nPrice <= 0 )
#endif
		return false;
	m_nTotalPrice += BuyItem.nPrice;

	m_VecBuyItemList.push_back(BuyItem);
	return true;
}

CDNUserRepository::TBuyItem *CDNUserRepository::GetBuyItem(int nIndex)
{
	if (m_VecBuyItemList.empty()) return NULL;
	if (m_cProductCount <= nIndex) return NULL;
	return &(m_VecBuyItemList[nIndex]);
}

bool CDNUserRepository::SetGiftItem(TCashGiftItem &Item)
{
	TGiftItem GiftItem;
	memset(&GiftItem, 0, sizeof(TGiftItem));

	GiftItem.GiftItem = Item;
	GiftItem.nReservePetal = g_pExtManager->GetCashCommodityReserve(Item.nItemSN);
	GiftItem.nPrice = g_pExtManager->GetCashCommodityPrice(Item.nItemSN);
	GiftItem.nLimitCount = g_pExtManager->GetCashBuyAbleCount(Item.nItemSN);
	GiftItem.GiftItem.bFail = true;	// Set default to fail... set to true only when gift is properly acquired.
	if( GiftItem.nPrice <= 0)
		return false;

	m_nTotalPrice += GiftItem.nPrice;

	m_VecGiftItemList.push_back(GiftItem);
	return true;
}

void CDNUserRepository::SetGiftItemPurchaseOrderID(int nIndex, INT64 biPurchaseOrderID)
{
	if (m_VecGiftItemList.empty()) return;
	if (m_cProductCount <= nIndex) return;

	m_VecGiftItemList[nIndex].biPurchaseOrderID = biPurchaseOrderID;
}

CDNUserRepository::TGiftItem *CDNUserRepository::GetGiftItem(int nIndex)
{
	if (m_VecGiftItemList.empty()) return NULL;
	if (m_cProductCount <= nIndex) return NULL;
	return &(m_VecGiftItemList[nIndex]);
}

void CDNUserRepository::SetBalanceInquiry(int nThreadID, CDNConnection *pConnection, TQCashBalanceInquiry *pCash)
{
	Clear();

	m_nSubCommand = CASH_BALANCEINQUIRY;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;

	m_nAccountDBID = pCash->nAccountDBID;
	_strcpy(m_szAccountName, _countof(m_szAccountName), pCash->szAccountName, (int)strlen(pCash->szAccountName));
	MultiByteToWideChar(CP_ACP, 0, m_szAccountName, -1, m_wszAccountName, IDLENMAX);

	m_bOpen = pCash->bOpen;

	m_nIp = pCash->nIp;
	_inet_addr(pCash->nIp, m_szIp);
	MultiByteToWideChar(CP_ACP, 0, m_szIp, -1, m_wszIp, IPLENMAX);

	m_bServer = pCash->bServer;
}

bool CDNUserRepository::SetBuy(int nThreadID, CDNConnection *pConnection, TQCashBuy *pCash)
{
	Clear();

	m_nSubCommand = CASH_BUY;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;
	m_nAccountDBID = pCash->nAccountDBID;
	_strcpy(m_szCharacterName, _countof(m_szCharacterName), pCash->szCharacterName, (int)strlen(pCash->szCharacterName));
	MultiByteToWideChar(CP_ACP, 0, m_szCharacterName, -1, m_wszCharacterName, NAMELENMAX);
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;	
	m_cAge = pCash->cAge;
	m_bPCBang = pCash->bPCBang;
	m_cBuyCartType = pCash->cCartType;
	m_cProductCount = pCash->cProductCount;
#if defined(PRE_ADD_CASH_REFUND)
	m_bCashMoveInven = pCash->bMoveCashInven;
#endif
	m_cPaymentRules = pCash->cPaymentRules;
#if defined(PRE_ADD_SALE_COUPON)
	m_biSaleCouponSerial = pCash->biSaleCouponSerial;
#endif

	for (int i = 0; i < m_cProductCount; i++)
	{
		if( !SetBuyItem(pCash->BuyList[i]) )
			return false;
	}
	return true;
}

void CDNUserRepository::SetPackageBuy(int nThreadID, CDNConnection *pConnection, TQCashPackageBuy *pCash)
{
	Clear();

	m_nThreadID = nThreadID;
	m_nSubCommand = CASH_PACKAGEBUY;
	m_nAccountDBID = pCash->nAccountDBID;
	_strcpy(m_szCharacterName, _countof(m_szCharacterName), pCash->szCharacterName, (int)strlen(pCash->szCharacterName));
	MultiByteToWideChar(CP_ACP, 0, m_szCharacterName, -1, m_wszCharacterName, NAMELENMAX);
	m_pConnection = pConnection;
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;	
	m_cProductCount = pCash->cPackageCount;
	memcpy(m_ProductPackageList, pCash->BuyList, sizeof(TCashItemBase) * pCash->cPackageCount);
	m_nPackageSN = pCash->nPackageSN;
	m_nTotalPrice = g_pExtManager->GetCashCommodityPrice(m_nPackageSN);
	m_cPaymentRules = pCash->cPaymentRules;
#if defined(PRE_ADD_CASH_REFUND)
	m_bCashMoveInven = pCash->bMoveCashInven;
#endif	// #if defined(PRE_ADD_CASH_REFUND)
#if defined(PRE_ADD_SALE_COUPON)
	m_biSaleCouponSerial = pCash->biSaleCouponSerial;
#endif
}

bool CDNUserRepository::SetGift(int nThreadID, CDNConnection *pConnection, TQCashGift *pCash)
{
	Clear();

	m_nSubCommand = CASH_GIFT;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;

	m_nAccountDBID = pCash->nAccountDBID;
	_strcpy(m_szCharacterName, _countof(m_szCharacterName), pCash->szCharacterName, (int)strlen(pCash->szCharacterName));
	MultiByteToWideChar(CP_ACP, 0, m_szCharacterName, -1, m_wszCharacterName, NAMELENMAX);
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;

	_wcscpy(m_wszReceiverCharacterName, NAMELENMAX, pCash->wszReceiverCharacterName, NAMELENMAX);
	_wcscpy(m_wszMessage, GIFTMESSAGEMAX, pCash->wszMessage, GIFTMESSAGEMAX);

	m_cAge = pCash->cAge;
	m_bPCBang = pCash->bPCBang;
	m_cGiftCartType = pCash->cCartType;
#if defined(PRE_ADD_CADGE_CASH)
	m_nMailDBID = pCash->nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	m_cProductCount = pCash->cProductCount;

	for (int i = 0; i < m_cProductCount; i++)
	{
		if( !SetGiftItem(pCash->GiftList[i]) )		
			return false;	
	}
	return true;
}

void CDNUserRepository::SetPackageGift(int nThreadID, CDNConnection *pConnection, TQCashPackageGift *pCash)
{
	Clear();

	m_nSubCommand = CASH_PACKAGEGIFT;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;

	m_nAccountDBID = pCash->nAccountDBID;
	_strcpy(m_szCharacterName, _countof(m_szCharacterName), pCash->szCharacterName, (int)strlen(pCash->szCharacterName));
	MultiByteToWideChar(CP_ACP, 0, m_szCharacterName, -1, m_wszCharacterName, NAMELENMAX);
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;
	m_cProductCount = pCash->cPackageCount;
	memcpy(m_GiftPackageList, pCash->GiftList, sizeof(TCashPackageGiftItem) * m_cProductCount);
	m_nPackageSN = pCash->nPackageSN;
	m_nTotalPrice = g_pExtManager->GetCashCommodityPrice(m_nPackageSN);

	_wcscpy(m_wszReceiverCharacterName, NAMELENMAX, pCash->wszReceiverCharacterName, NAMELENMAX);
	_wcscpy(m_wszMessage, GIFTMESSAGEMAX, pCash->wszMessage, GIFTMESSAGEMAX);

	m_cAge = pCash->cAge;
	m_bPCBang = pCash->bPCBang;
#if defined(PRE_ADD_CADGE_CASH)
	m_nMailDBID = pCash->nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
}

void CDNUserRepository::SetCoupon(int nThreadID, CDNConnection *pConnection, TQCashCoupon *pCash)
{
	Clear();

	m_nSubCommand = CASH_COUPON;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_bPCBang = pCash->bPCBang;
	_strcpy(m_szCharacterName, NAMELENMAX, pCash->szCharacterName, NAMELENMAX);
	MultiByteToWideChar(CP_ACP, 0, m_szCharacterName, -1, m_wszCharacterName, NAMELENMAX);
	_wcscpy(m_wszCoupon, COUPONMAX, pCash->wszCoupon, COUPONMAX);
	WideCharToMultiByte(CP_ACP, NULL, pCash->wszCoupon, -1, m_szCoupon, sizeof(m_szCoupon), NULL, NULL);
}

void CDNUserRepository::SetVIPBuy(int nThreadID, CDNConnection *pConnection, TQCashVIPBuy *pCash)
{
	Clear();

	m_nSubCommand = CASH_VIPBUY;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;
	m_nAccountDBID = pCash->nAccountDBID;
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;
	m_bPCBang = pCash->bPCBang;
	m_bAutoPay = pCash->bAutoPay;
	m_bServer = pCash->bServer;

	m_VIP.nItemSN = pCash->nItemSN;
	m_VIP.nPrice = g_pExtManager->GetCashCommodityPrice(pCash->nItemSN);
	if (m_bAutoPay)	// Use discounted price if auto-payment 
		m_VIP.nPrice = g_pExtManager->GetCashCommodityVIPAutomaticPaySalePrice(pCash->nItemSN);

	m_VIP.nLimitCount = g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN);
}

void CDNUserRepository::SetVIPGift(int nThreadID, CDNConnection *pConnection, TQCashVIPGift *pCash)
{
	Clear();

	m_nSubCommand = CASH_VIPGIFT;
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;
	m_nAccountDBID = pCash->nAccountDBID;
	m_biCharacterDBID = pCash->biCharacterDBID;
	m_cWorldSetID = pCash->cWorldSetID;
	m_nMapID = pCash->nMapID;
	m_nChannelID = pCash->nChannelID;
	m_bPCBang = pCash->bPCBang;
	_wcscpy(m_wszReceiverCharacterName, NAMELENMAX, pCash->wszReceiverCharacterName, NAMELENMAX);
	_wcscpy(m_wszMessage, GIFTMESSAGEMAX, pCash->wszMessage, GIFTMESSAGEMAX);

	m_VIP.nItemSN = pCash->nItemSN;
	m_VIP.nPrice = g_pExtManager->GetCashCommodityPrice(pCash->nItemSN);
	m_VIP.nLimitCount = g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN);
}

void CDNUserRepository::SetRefund(int nThreadID, CDNConnection *pConnection, TQCashRefund* pCash)
{
	Clear();
	m_nThreadID = nThreadID;
	m_pConnection = pConnection;
	m_nAccountDBID = pCash->nAccountDBID;
	m_cWorldSetID = pCash->cWorldSetID;
#if defined(PRE_ADD_CASH_REFUND)
	m_cRefundType = pCash->cItemType;
	m_biPurchaseOrderDetailID = pCash->biPurchaseOrderDetailID;
#endif	// #if defined(PRE_ADD_CASH_REFUND)
}

int CDNUserRepository::OnBalanceInquiry(int nThreadID, CDNConnection *pConnection, TQCashBalanceInquiry *pCash)
{
	SetBalanceInquiry(nThreadID, pConnection, pCash);
	return g_pBillingConnection->OnBalanceInquiry(this);
}

int CDNUserRepository::OnBuy(int nThreadID, CDNConnection *pConnection, TQCashBuy *pCash)
{
	if( !SetBuy(nThreadID, pConnection, pCash) )
	{
		SendBuyGift(ERROR_GENERIC_INVALIDREQUEST);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	int nAdd = 0;
	for (int i = 0; i < pCash->cProductCount; i++){
		if (!CheckSaleItemSN(pCash->BuyList[i].nItemSN)){
			g_Log.Log(LogType::_ERROR, this, L"[CASH_BUY] CheckSaleItemSN Error (ItemSN:%d)\r\n", pCash->BuyList[i].nItemSN);
			SendBuyGift(ERROR_CASHSHOP_PROHIBITSALE);
			return ERROR_CASHSHOP_PROHIBITSALE;
		}

		if (g_pExtManager->GetItemMainType(pCash->BuyList[i].CashItem.nItemID) == ITEMTYPE_CHARACTER_SLOT){
			nAdd++;
		}
	}

#if defined(PRE_MOD_SELECT_CHAR)
	int nRet = CheckCharacterSlotCount(pCash->nAccountDBID, nAdd);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int nRet = CheckCharacterSlotCount(pCash->biCharacterDBID, nAdd);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	if (pCash->cPaymentRules == Cash::PaymentRules::Petal){		// Buy item with Petal
		nRet = PurchaseItemByPetal();
	}
	else {	// Buy item with cash
		nRet = CheckCashBalanceSufficient(GetMembershipDB(), GetAccountDBID(), m_nTotalPrice);
		if (nRet != ERROR_NONE){
			SendBuyGift(nRet);
			return nRet;
		}
		nRet = PurchaseBuyItem();
	}

	return nRet;
}

int CDNUserRepository::OnPackageBuy(int nThreadID, CDNConnection *pConnection, TQCashPackageBuy *pCash)
{
	SetPackageBuy(nThreadID, pConnection, pCash);

#if defined(PRE_ADD_SALE_COUPON)
	if( m_nTotalPrice <= 0 && !g_pExtManager->GetCashCommodityUseCoupon(pCash->nPackageSN))
#else
	if( m_nTotalPrice <=0 )
#endif
	{
		SendBuyGift(ERROR_ITEM_FAIL);
		return ERROR_ITEM_FAIL;
	}

	if (!CheckSaleItemSN(pCash->nPackageSN)){
		g_Log.Log(LogType::_ERROR, this, L"[CASH_PACKAGEBUY] CheckSaleItemSN Error (ItemSN:%d)\r\n", pCash->nPackageSN);
		SendBuyGift(ERROR_CASHSHOP_PROHIBITSALE);
		return ERROR_CASHSHOP_PROHIBITSALE;
	}

	int nRet = ERROR_DB;
	int nAdd = 0;
	for (int i = 0; i < pCash->cPackageCount; i++){
		if (g_pExtManager->GetItemMainType(pCash->BuyList[i].CashItem.nItemID) == ITEMTYPE_CHARACTER_SLOT){
			nAdd++;
		}
	}
#if defined(PRE_MOD_SELECT_CHAR)
	nRet = CheckCharacterSlotCount(pCash->nAccountDBID, nAdd);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	nRet = CheckCharacterSlotCount(pCash->biCharacterDBID, nAdd);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	if (pCash->cPaymentRules == Cash::PaymentRules::Petal){		// Buy item with Petal
		return PurchaseItemByPetal();
	}
	else {	// Buy item with cash
		if (m_nTotalPrice > 0){
			nRet = CheckCashBalanceSufficient(GetMembershipDB(), GetAccountDBID(), m_nTotalPrice);
			if (nRet != ERROR_NONE){
				SendBuyGift(nRet);
				return nRet;
			}
		}

		nRet = AddPurchaseOrderByCash(0);
		if (nRet != ERROR_NONE){
			SendBuyGift(nRet);
			return nRet;
		}

#if defined(PRE_ADD_SALE_COUPON)
		if( m_nTotalPrice == 0 && g_pExtManager->GetCashCommodityUseCoupon(pCash->nPackageSN) )
		{
			OnRecvBillingBuyItem(ERROR_NONE);
			return ERROR_NONE;
		}
#endif	//	#if defined(PRE_ADD_SALE_COUPON)

		nRet = g_pBillingConnection->OnPackageBuy(this);
		if (nRet != ERROR_NONE){
			SendBuyGift(nRet);
			return nRet;
		}
	}

	return ERROR_NONE;
}

int CDNUserRepository::OnGift(int nThreadID, CDNConnection *pConnection, TQCashGift *pCash)
{
	if( !SetGift(nThreadID, pConnection, pCash) )
	{
		SendBuyGift(ERROR_ITEM_FAIL);
		return ERROR_ITEM_FAIL;
	}

	// Return an error value if trying to buy something not on sale
	for (int i = 0; i < pCash->cProductCount; i++){
		if (!CheckSaleItemSN(pCash->GiftList[i].nItemSN)){
			g_Log.Log(LogType::_ERROR, this, L"[CASH_GIFT] CheckSaleItemSN Error (ItemSN:%d)\r\n", pCash->GiftList[i].nItemSN);
			SendBuyGift(ERROR_CASHSHOP_PROHIBITSALE);
			return ERROR_CASHSHOP_PROHIBITSALE;
		}
	}

	int nRet = CheckCashBalanceSufficient(GetMembershipDB(), GetAccountDBID(), m_nTotalPrice);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	return PurchaseGiftItem();
}

int CDNUserRepository::OnPackageGift(int nThreadID, CDNConnection *pConnection, TQCashPackageGift *pCash)
{
	SetPackageGift(nThreadID, pConnection, pCash);

	if( m_nTotalPrice <=0 )
	{
		SendBuyGift(ERROR_ITEM_FAIL);
		return ERROR_ITEM_FAIL;
	}

	if (!CheckSaleItemSN(pCash->nPackageSN)){
		g_Log.Log(LogType::_ERROR, this, L"[CASH_PACKAGEGIFT] CheckSaleItemSN Error (ItemSN:%d)\r\n", pCash->nPackageSN);
		SendBuyGift(ERROR_CASHSHOP_PROHIBITSALE);
		return ERROR_CASHSHOP_PROHIBITSALE;
	}

	int nRet = CheckCashBalanceSufficient(GetMembershipDB(), GetAccountDBID(), m_nTotalPrice);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	nRet = AddPurchaseOrderByCash(0);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	nRet = g_pBillingConnection->OnPackageGift(this);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	return ERROR_NONE;
}

int CDNUserRepository::OnCoupon(int nThreadID, CDNConnection *pConnection, TQCashCoupon *pCash)
{
	SetCoupon(nThreadID, pConnection, pCash);

	int nRet = g_pBillingConnection->OnCoupon(this);
	SendCoupon(nRet);
	return nRet;
}

int CDNUserRepository::OnVIPBuy(int nThreadID, CDNConnection *pConnection, TQCashVIPBuy *pCash)
{
	SetVIPBuy(nThreadID, pConnection, pCash);

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		SendVIPBuyGift(ERROR_GENERIC_INVALIDREQUEST, 0);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if (!CheckSaleItemSN(pCash->nItemSN)){
		SendVIPBuyGift(ERROR_CASHSHOP_PROHIBITSALE, 0);
		return ERROR_CASHSHOP_PROHIBITSALE;
	}

	TCashCommodityData CashData;
	bool bRet= g_pExtManager->GetCashCommodityData(pCash->nItemSN, CashData);
	if (!bRet){
		SendVIPBuyGift(ERROR_ITEM_FAIL, 0);
		return ERROR_ITEM_FAIL;
	}

	INT64 biOrderID = 0;

	int nVIPPrice = CashData.nPrice;
	if (pCash->bAutoPay)	// Use discounted price if auto-payment 
		nVIPPrice = (int)((CashData.nPrice * (100 - CashData.nAutomaticPaySale)) / 100);
	m_VIP.nPrice = nVIPPrice;
	m_nTotalPrice = nVIPPrice;

	int nRet = CheckCashBalanceSufficient(pMembershipDB, GetAccountDBID(), nVIPPrice);
	if (nRet != ERROR_NONE){
		SendVIPBuyGift(nRet, 0);
		return nRet;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	nRet = pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, pCash->bPCBang, pCash->nItemSN, nVIPPrice, g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
		CashData.nItemID[0], -1, g_pExtManager->GetCashLimitedItemCount(pCash->nItemSN), nVIPPrice, m_szIp, false, NULL, NULL, biOrderID);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	nRet = pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, pCash->bPCBang, pCash->nItemSN, nVIPPrice, g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
		CashData.nItemID[0], -1, nVIPPrice, m_szIp, false, NULL, NULL, biOrderID);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	if (nRet != ERROR_NONE){
		SendVIPBuyGift(nRet, 0);
		return nRet;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	if (g_pLimitedCashItemRepository && g_pExtManager->GetCashLimitedItemCount(pCash->nItemSN) > 0)
	{
		std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
		if (g_pLimitedCashItemRepository->GetLimitedItemList(vList))
			m_pConnection->SendLimiteItemInfoList(vList);
	}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	m_VIP.biPurchaseOrderID = biOrderID;

	nRet = g_pBillingConnection->OnVIPBuy(this);
	if (nRet != ERROR_NONE){
		SendVIPBuyGift(nRet, 0);
		return nRet;
	}

	return ERROR_NONE;
}

int CDNUserRepository::OnVIPGift(int nThreadID, CDNConnection *pConnection, TQCashVIPGift *pCash)
{
	SendVIPBuyGift(ERROR_GENERIC_INVALIDREQUEST, 0);
	return ERROR_GENERIC_INVALIDREQUEST;

/*
	SetVIPGift(nThreadID, pConnection, pCash);

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		SendVIPBuyGift(ERROR_GENERIC_INVALIDREQUEST, 0);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if (!CheckSaleItemSN(pCash->nItemSN)){
		SendVIPBuyGift(ERROR_CASHSHOP_PROHIBITSALE, 0);
		return ERROR_CASHSHOP_PROHIBITSALE;
	}

	TCashCommodityData CashData;
	bool bRet = g_pExtManager->GetCashCommodityData(pCash->nItemSN, CashData);
	if (!bRet){
		SendVIPBuyGift(ERROR_ITEM_FAIL, 0);
		return ERROR_ITEM_FAIL;
	}

	INT64 biOrderID = 0;
#ifdef PRE_ADD_LIMITED_CASHITEM
	int nRet = pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, pCash->bPCBang, pCash->nItemSN, CashData.nPrice, g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
		CashData.nItemID[0], -1, g_pExtManager->GetCashLimitedItemCount(pCash->nItemSN), CashData.nPrice, m_szIp, true, pCash->wszReceiverCharacterName, pCash->wszMessage, biOrderID);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	int nRet = pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, pCash->bPCBang, pCash->nItemSN, CashData.nPrice, g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
		CashData.nItemID[0], -1, CashData.nPrice, m_szIp, true, pCash->wszReceiverCharacterName, pCash->wszMessage, biOrderID);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	if (nRet != ERROR_NONE){
		SendVIPBuyGift(nRet, 0);
		return nRet;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	if (g_pLimitedCashItemRepository && g_pExtManager->GetCashLimitedItemCount(pCash->nItemSN) > 0)
	{
		std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
		if (g_pLimitedCashItemRepository->GetLimitedItemList(vList))
			m_pConnection->SendLimiteItemInfoList(vList);
	}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	m_VIP.biPurchaseOrderID = biOrderID;

	return ERROR_NONE;
	*/
}

void CDNUserRepository::OnRefund(int nThreadID, CDNConnection *pConnection, TQCashRefund* pCash)
{
#if defined(PRE_ADD_CASH_REFUND)
	/*
	SetRefund(nThreadID, pConnection, pCash);

	int nProductNo = g_pBillingConnection->GetProductNoBySN(pCash->nItemSN);
	g_pBillingConnection->SendRefund(this, pCash->uiOrderNo, nProductNo, pCash->biPurchaseOrderDetailID);
	*/
#endif	// #if defined(PRE_ADD_CASH_REFUND)
}

void CDNUserRepository::OnMoveCashInven(int nThreadID, CDNConnection *pConnection, TQCashMoveCashInven* pCash)
{
#if defined(PRE_ADD_CASH_REFUND)
	m_cWorldSetID = pCash->cWorldSetID;

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		m_pConnection->SendMoveCashInven(pCash->nAccountDBID, ERROR_GENERIC_INVALIDREQUEST, 0, 0, NULL, 0);
		return;
	}

	// Turn off billing inventory flag
	int nRet = pMembershipDB->QueryModPurchaseOrderDetail(pCash->PaymentItemInfo.PaymentItemInfo.biDBID);	
	if (nRet != ERROR_NONE)
	{
		m_pConnection->SendMoveCashInven(pCash->nAccountDBID, nRet, 0, 0, NULL, 0);
		return;
	}
	// Insert into product DB
	nRet = CashItemDBProcess(pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, pCash->nMapID, pCash->nChannelID, 0, pCash->CashItem, 
		g_pExtManager->GetCashCommodityPrice(pCash->PaymentItemInfo.PaymentItemInfo.ItemInfo.nItemSN), pCash->PaymentItemInfo.PaymentItemInfo.biDBID, DBDNWorldDef::AddMaterializedItem::CashBuy);

	if (nRet != ERROR_NONE)			// If it fails, turn on the fail flag..
		pMembershipDB->QueryModGiveFailFlag2(pCash->PaymentItemInfo.PaymentItemInfo.biDBID, true);			

	// Check if Petal points should be credited
	int nReservePetal = g_pExtManager->GetCashCommodityReserve(pCash->PaymentItemInfo.PaymentItemInfo.ItemInfo.nItemSN);
	int nTotalPetal = 0;
	if( nReservePetal > 0 )
	{
		nRet = pMembershipDB->QueryAddPetalIncomeCashRefund(pCash->biCharacterDBID, pCash->PaymentItemInfo.PaymentItemInfo.biDBID, nReservePetal, nTotalPetal);
		if( nRet != ERROR_NONE ) // Should I really return an error this far in...
		{
			m_pConnection->SendMoveCashInven(pCash->nAccountDBID, nRet, 0, 0, NULL, 0);
			return;
		}
	}
	else
	{
		nTotalPetal = GetPetalBalance();
	}
	/*
	// Send SendPickUp to billing
	int nProductNo = g_pBillingConnection->GetProductNoBySN(pCash->PaymentItemInfo.PaymentItemInfo.ItemInfo.nItemSN);
g_pBillingConnection->SendInventoryPickup(pCash->nAccountDBID, pCash->PaymentItemInfo.uiOrderNo, nProductNo, 1); // Quantity is always 1.
	*/

	INT64 nSeedPoint = 0;
	
	m_pConnection->SendMoveCashInven(pCash->nAccountDBID, ERROR_NONE, nTotalPetal, pCash->PaymentItemInfo.PaymentItemInfo.biDBID, &pCash->CashItem, nSeedPoint);
#endif	// #if defined(PRE_ADD_CASH_REFUND)
}

void CDNUserRepository::OnPackageMoveCashInven(int nThreadID, CDNConnection *pConnection, TQCashPackageMoveCashInven* pCash)
{
#if defined(PRE_ADD_CASH_REFUND)
	m_cWorldSetID = pCash->cWorldSetID;

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		m_pConnection->SendPackageMoveCashInven(pCash->nAccountDBID, ERROR_GENERIC_INVALIDREQUEST, 0, 0, NULL, 0);
		return;
	}

	// Turn off billing inventory flag
	int nRet = pMembershipDB->QueryModPurchaseOrderDetail(pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.biDBID);	
	if (nRet != ERROR_NONE)
	{
		m_pConnection->SendPackageMoveCashInven(pCash->nAccountDBID, nRet, 0, 0, NULL, 0);
		return;
	}	

	int nProductCount = 0;
	for( int i=0; i<PACKAGEITEMMAX; ++i)
	{
		if( pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.ItemInfoList[i].nItemID == 0)
			break;
		++nProductCount;
	}

	if (nProductCount == 0)
	{
		m_pConnection->SendPackageMoveCashInven(pCash->nAccountDBID, ERROR_GENERIC_UNKNOWNERROR, 0, 0, NULL, 0);
		return;
	}

	int nTotalPrice = g_pExtManager->GetCashCommodityPrice(pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.nPackageSN);
	int nUnitPrice = nTotalPrice / nProductCount;
	int nTempPrice = 0;
	for (int i = 0; i < nProductCount; i++)
	{
		if (i == (nProductCount - 1)) nUnitPrice = nTotalPrice - nTempPrice;					
		// Insert into product DB
		nRet = CashItemDBProcess(pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, pCash->nMapID, pCash->nChannelID, 0, pCash->CashItemList[i], 
			nUnitPrice, pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.biDBID, DBDNWorldDef::AddMaterializedItem::CashBuy);

		if (nRet != ERROR_NONE){
			pMembershipDB->QueryModGiveFailFlag2(pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.biDBID, true);						
			// Even if it fails, don't return early; process everything for now
		}
		nTempPrice += nUnitPrice;
	}

	// Check if Petal points should be credited
	int nReservePetal = g_pExtManager->GetCashCommodityReserve(pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.nPackageSN);
	int nTotalPetal = 0;
	if( nReservePetal > 0 )
	{
		nRet = pMembershipDB->QueryAddPetalIncomeCashRefund(pCash->biCharacterDBID, pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.biDBID, nReservePetal, nTotalPetal);
		if( nRet != ERROR_NONE ) // Should I really return an error this far in...
		{
			m_pConnection->SendPackageMoveCashInven(pCash->nAccountDBID, nRet, 0, 0, NULL, 0);
			return;
		}
	}
	else
	{
		nTotalPetal = GetPetalBalance();
	}

	

	INT64 nSeedPoint = 0;
	/*
	// Send SendPickUp to billing
	int nProductNo = g_pBillingConnection->GetProductNoBySN(pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.nPackageSN);
g_pBillingConnection->SendInventoryPickup(pCash->nAccountDBID, pCash->PaymentPackageItemInfo.uiOrderNo, nProductNo, 1); // Quantity is always 1.
	*/
	m_pConnection->SendPackageMoveCashInven(pCash->nAccountDBID, ERROR_NONE, nTotalPetal, pCash->PaymentPackageItemInfo.PaymentPackageItemInfo.biDBID, pCash->CashItemList, nSeedPoint);
#endif	// #if defined(PRE_ADD_CASH_REFUND)
}

int CDNUserRepository::CalcPackageProcess()
{
	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB)
		return ERROR_GENERIC_INVALIDREQUEST;

	if (0 == m_cProductCount)
	{
		printf("[err] CDNUserRepository::CalcPackageProcess m_cProductCount=0\n");  //
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	int nRet = 0;

	int nTotalPrice = g_pExtManager->GetCashCommodityPrice(m_nPackageSN);
	int nUnitPrice = nTotalPrice / m_cProductCount;
	int nTempPrice = 0;
	for (int i = 0; i < m_cProductCount; i++){
		if (i == (m_cProductCount - 1)) nUnitPrice = nTotalPrice - nTempPrice;

		int nCode = DBDNWorldDef::AddMaterializedItem::CashBuy;

		nRet = CashItemDBProcess(m_cWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nMapID, m_nChannelID, m_cPaymentRules, m_ProductPackageList[i], nUnitPrice, m_biPurchaseOrderID, nCode);

		if (nRet != ERROR_NONE){
			pMembershipDB->QueryModGiveFailFlag(m_biPurchaseOrderID, m_nPackageSN, m_ProductPackageList[i].CashItem.nItemID, true);
			g_Log.Log(LogType::_ERROR, this, L"[SetBillingNo(Package)] CashItemDBQuery Fail(%d) Order(%I64d)\r\n", nRet, m_biPurchaseOrderID);
			// Even if it fails, don't return early; process everything for now
		}
		nTempPrice += nUnitPrice;
	}

	return nRet;
}

#if defined(PRE_ADD_CASH_REFUND)
void CDNUserRepository::OnRecvBillingRefund(int nResult)
{
	if( nResult != ERROR_NONE)
	{
		SendCashRefund(nResult);
		return;
	}

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		SendCashRefund(ERROR_DB);  // Already refunded, this should not happen..
		return;
	}

	// Refund processing.
	int nRet = pMembershipDB->QueryCashRefund(m_biPurchaseOrderDetailID);
	if (nRet != ERROR_NONE){
		SendCashRefund(nRet);
		return;
	}

	SendCashRefund(ERROR_NONE);	
}
#endif

bool CDNUserRepository::OnRecvBillingCoupon(int nBillingResult, int nResult, int nItemSN, UINT nOrderNo)
{
	// Whether serial package exists.
	if (!g_pExtManager->IsOnSaleCommodity(nItemSN))
	{
		SendCoupon(ERROR_CASHSHOP_PROHIBITSALE);
		return false;
	}

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		SendCoupon(ERROR_GENERIC_INVALIDREQUEST);
		return false;
	}

	string SNStr, ItemIDStr, ItemOptionStr;
	BYTE cOption = -1;

	TCashCommodityData CashData;
	memset(&CashData, 0, sizeof(CashData));

	TCashPackageData PackageData;
	memset(&PackageData, 0, sizeof(PackageData));
	bool bPackage = g_pExtManager->GetCashPackageData(nItemSN, PackageData);

	if (bPackage){	// package
		SNStr = FormatA("%d", nItemSN);
		for (int i = 0; i < (int)PackageData.nVecCommoditySN.size(); i++){
			if (PackageData.nVecCommoditySN[i] <= 0) continue;

			if (!ItemIDStr.empty()){
				ItemIDStr.append("|");
				ItemOptionStr.append("|");
			}

			memset(&CashData, 0, sizeof(TCashCommodityData));
			bool bCash = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[i], CashData);
			if (!bCash) continue;

			ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
			ItemOptionStr.append(boost::lexical_cast<std::string>(static_cast<int>(cOption)));			
		}
	}
	else{
		memset(&CashData, 0, sizeof(TCashCommodityData));
		SNStr = FormatA("%d", nItemSN);
		bool bCash = g_pExtManager->GetCashCommodityData(nItemSN, CashData);
		if (bCash){
			ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
			ItemOptionStr.append(boost::lexical_cast<std::string>(static_cast<int>(cOption)));
		}
	}

	m_nCouponSN = nItemSN;

	string OrderStr = FormatA("%d", nOrderNo);
	int nRet = pMembershipDB->QueryMakeGiftByCoupon(m_biCharacterDBID, m_bPCBang, SNStr.c_str(), ItemIDStr.c_str(), ItemOptionStr.c_str(), m_wszCoupon, OrderStr.c_str(), m_szIp, m_biCouponOrderID);
	if (nRet != ERROR_NONE){
		SendCoupon(nRet);
		return false;
	}

	char szOrderNo[PUBLISHERORDERKEYMAX] = {0, };
	sprintf_s(szOrderNo, "%d", nOrderNo);
	char cOrderCode = DBDNMembership::OrderStatusCode::FailPayment;
	if (nResult == ERROR_NONE)
		cOrderCode = DBDNMembership::OrderStatusCode::Success;

	nRet = pMembershipDB->QuerySetPurchaseOrderResult(m_biCouponOrderID, cOrderCode, szOrderNo, nBillingResult, g_pExtManager->GetCashCommodityReserve(m_nCouponSN), m_nUsedPaidCash);
	if (nRet != ERROR_NONE){
		g_Log.Log(LogType::_ERROR, this, L"[SetCoupon] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d Billing:%u)\r\n", 
			nResult, m_biCouponOrderID, nOrderNo);
		SendCoupon(nRet);
		return false;
	}

	SendCoupon(nResult);
	return true;
}


// Send via Connection.
void CDNUserRepository::SendBalanceInquiry(int nResult, int nCashBalance, int nNotRefundableBalance/* = 0*/)
{
	if (!m_pConnection) return;
	m_pConnection->SendBalanceInquiry(nResult, m_nAccountDBID, nCashBalance, GetPetalBalance(), m_bOpen, m_bServer, nNotRefundableBalance);
}

#ifdef PRE_ADD_LIMITED_CASHITEM
void CDNUserRepository::SendBuyGift(int nResult, bool bUpdateLimitedCount)
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
void CDNUserRepository::SendBuyGift(int nResult)
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
{
	//if (ERROR_NONE != nResult)  return;

	if (!m_pConnection) return;

#ifdef PRE_ADD_LIMITED_CASHITEM
	if (g_pLimitedCashItemRepository && bUpdateLimitedCount)
	{
		std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
		if (g_pLimitedCashItemRepository->GetLimitedItemList(vList))
			m_pConnection->SendLimiteItemInfoList(vList);
	}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	INT64 nSeedPoint = 0;

	switch (m_nSubCommand)
	{
	case CASH_PACKAGEBUY:
		{
#if defined(PRE_ADD_CASH_REFUND)
#if defined(PRE_ADD_SALE_COUPON)
			m_pConnection->SendPackageBuy(m_nAccountDBID, nResult, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_ProductPackageList, &m_PaymentPackageItem, m_cPaymentRules, m_biSaleCouponSerial, nSeedPoint);
#else
			m_pConnection->SendPackageBuy(m_nAccountDBID, nResult, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_ProductPackageList, &m_PaymentPackageItem, m_cPaymentRules, nSeedPoint);
#endif // #if defined(PRE_ADD_SALE_COUPON)
#else // #if defined(PRE_ADD_CASH_REFUND)
#if defined(PRE_ADD_SALE_COUPON)
			m_pConnection->SendPackageBuy(m_nAccountDBID, nResult, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_ProductPackageList, NULL, m_cPaymentRules, m_biSaleCouponSerial, nSeedPoint);
#else
			m_pConnection->SendPackageBuy(m_nAccountDBID, nResult, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_ProductPackageList, NULL, m_cPaymentRules, nSeedPoint);
#endif // #if defined(PRE_ADD_SALE_COUPON)
#endif //#if defined(PRE_ADD_CASH_REFUND)
		}
		break;

	case CASH_BUY:
		{
			TCashBuyItem CashBuyItem[PREVIEWCARTLISTMAX];
			memset(&CashBuyItem, 0, sizeof(CashBuyItem));
			for (int i = 0; i < (int)m_VecBuyItemList.size(); i++)
			{
				CashBuyItem[i] = m_VecBuyItemList[i].BuyItem;
			}
#if defined(PRE_ADD_CASH_REFUND)
#if defined(PRE_ADD_SALE_COUPON)
			m_pConnection->SendBuy(m_nAccountDBID, nResult, m_cBuyCartType, GetPetalBalance(), m_cProductCount, CashBuyItem, m_PaymentItem, m_cPaymentRules, m_biSaleCouponSerial, nSeedPoint);
#else
			m_pConnection->SendBuy(m_nAccountDBID, nResult, m_cBuyCartType, GetPetalBalance(), m_cProductCount, CashBuyItem, m_PaymentItem, m_cPaymentRules, nSeedPoint);
#endif //#if defined(PRE_ADD_SALE_COUPON)
#else //#if defined(PRE_ADD_CASH_REFUND)
#if defined(PRE_ADD_SALE_COUPON)
			m_pConnection->SendBuy(m_nAccountDBID, nResult, m_cBuyCartType, GetPetalBalance(), m_cProductCount, CashBuyItem, NULL, m_cPaymentRules, m_biSaleCouponSerial, nSeedPoint);
#else
			m_pConnection->SendBuy(m_nAccountDBID, nResult, m_cBuyCartType, GetPetalBalance(), m_cProductCount, CashBuyItem, NULL, m_cPaymentRules, nSeedPoint);
#endif //#if defined(PRE_ADD_SALE_COUPON)
#endif //#if defined(PRE_ADD_CASH_REFUND)
		}
		break;

	case CASH_PACKAGEGIFT:
		{
			if (nResult != ERROR_NONE){
#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
			}
			else{
				CDNSQLMembership *pMembershipDB = GetMembershipDB();
				if (!pMembershipDB){
#if defined(PRE_ADD_CADGE_CASH)
					m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
					m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
					return;
				}
				CDNSQLWorld *pWorldDB = GetWorldDB();
				if (!pWorldDB){
#if defined(PRE_ADD_CADGE_CASH)
					m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
					m_pConnection->SendPackageGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, 0, m_nPackageSN, m_cProductCount, m_GiftPackageList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
					return;
				}

				UINT nReceiverAccountDBID = 0;
				INT64 biReceiverCharacterDBID = 0;
				pWorldDB->QueryGetCharacterPartialy7(m_wszReceiverCharacterName, nReceiverAccountDBID, biReceiverCharacterDBID);

				int nGiftCount = 0;
				pMembershipDB->QueryGetCountNotReceivedGift(biReceiverCharacterDBID, nGiftCount);

#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendPackageGift(nResult, m_nAccountDBID, nReceiverAccountDBID, biReceiverCharacterDBID, nGiftCount, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_GiftPackageList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendPackageGift(nResult, m_nAccountDBID, nReceiverAccountDBID, biReceiverCharacterDBID, nGiftCount, GetPetalBalance(), m_nPackageSN, m_cProductCount, m_GiftPackageList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
			}
		}
		break;

	case CASH_GIFT:
		{
			TCashGiftItem CashGiftList[PREVIEWCARTLISTMAX] = {0,};
			CDNSQLMembership *pMembershipDB = GetMembershipDB();
			if (!pMembershipDB){
#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(ERROR_DB, m_nAccountDBID, 0, (INT64)0, 0, 0, m_cGiftCartType, m_cProductCount, CashGiftList, m_nMailDBID, 0);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(ERROR_DB, m_nAccountDBID, 0, (INT64)0, 0, 0, m_cGiftCartType, m_cProductCount, CashGiftList, 0, 0);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
				return;
			}
			CDNSQLWorld *pWorldDB = GetWorldDB();
			if (!pWorldDB){
#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(ERROR_DB, m_nAccountDBID, 0, (INT64)0, 0, 0, m_cGiftCartType, m_cProductCount, CashGiftList, m_nMailDBID, 0);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(ERROR_DB, m_nAccountDBID, 0, (INT64)0, 0, 0, m_cGiftCartType, m_cProductCount, CashGiftList, 0, 0);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
				return;
			}

			for (int i = 0; i < (int)m_VecGiftItemList.size(); i++){
				CashGiftList[i] = m_VecGiftItemList[i].GiftItem;
			}

			if (nResult != ERROR_NONE){
#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, GetPetalBalance(), m_cGiftCartType, m_cProductCount, CashGiftList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(nResult, m_nAccountDBID, 0, (INT64)0, 0, GetPetalBalance(), m_cGiftCartType, m_cProductCount, CashGiftList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
			}
			else{
				UINT nReceiverAccountDBID = 0;
				INT64 biReceiverCharacterDBID = 0;
				pWorldDB->QueryGetCharacterPartialy7(m_wszReceiverCharacterName, nReceiverAccountDBID, biReceiverCharacterDBID);

				int nGiftCount = 0;
				pMembershipDB->QueryGetCountNotReceivedGift(biReceiverCharacterDBID, nGiftCount);

#if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(nResult, m_nAccountDBID, nReceiverAccountDBID, biReceiverCharacterDBID, nGiftCount, GetPetalBalance(), m_cGiftCartType, m_cProductCount, CashGiftList, m_nMailDBID, nSeedPoint);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				m_pConnection->SendGift(nResult, m_nAccountDBID, nReceiverAccountDBID, biReceiverCharacterDBID, nGiftCount, GetPetalBalance(), m_cGiftCartType, m_cProductCount, CashGiftList, 0, nSeedPoint);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
			}
		}
		break;
	}
}

void CDNUserRepository::SendCoupon(int nResult)
{

	if (!m_pConnection) return;
	m_pConnection->SendCoupon(m_nAccountDBID, nResult);
}

#if defined(PRE_ADD_CASH_REFUND)
void CDNUserRepository::SendCashRefund(int nResult)
{
	if (!m_pConnection) return;
	m_pConnection->SendCashRefund(m_nAccountDBID, nResult, m_cRefundType, m_biPurchaseOrderDetailID);
}
#endif

void CDNUserRepository::SendVIPBuyGift(int nResult, int nCash)
{
	if (!m_pConnection) return;

	switch (m_nSubCommand)
	{
	case CASH_VIPBUY:
		{
			if (nResult != ERROR_NONE){
				m_pConnection->SendVIPBuy(m_nAccountDBID, nResult, 0, nCash, m_VIP.nItemSN, 0, 0, m_bAutoPay, m_bServer, 0);
			}
			else{
				CDNSQLMembership *pMembershipDB = GetMembershipDB();
				if (!pMembershipDB){
					m_pConnection->SendVIPBuy(m_nAccountDBID, ERROR_DB, 0, nCash, m_VIP.nItemSN, 0, 0, m_bAutoPay, m_bServer, 0);
					return;
				}

				int nPetal = 0;
				int nVIPPoint = 0;
				__time64_t tEndDate = 0;
				if (!m_bAutoPay){
					m_bAutoPay = g_pExtManager->GetCashCommodityPay(m_VIP.nItemSN);
				}
				int nRet = pMembershipDB->QueryIncreaseVIPBasicPoint(GetCharacterDBID(), g_pExtManager->GetCashCommodityVIPPoint(m_VIP.nItemSN), m_VIP.biPurchaseOrderID, 
					g_pExtManager->GetCashCommodityPeriod(m_VIP.nItemSN), m_bAutoPay, nVIPPoint, tEndDate);

				if (nRet != ERROR_NONE){
					pMembershipDB->QueryModGiveFailFlag(m_VIP.biPurchaseOrderID, m_VIP.nItemSN, g_pExtManager->GetCashCommodityItem0(m_VIP.nItemSN), true);
					nResult = nRet;
				}

				INT64 nSeedPoint = 0;
				m_pConnection->SendVIPBuy(m_nAccountDBID, nResult, GetPetalBalance(), nCash, m_VIP.nItemSN, nVIPPoint, tEndDate, m_bAutoPay, m_bServer, nSeedPoint);
			}
		}
		break;

	case CASH_VIPGIFT:
		{
			if (nResult != ERROR_NONE){
				m_pConnection->SendVIPGift(m_nAccountDBID, nResult, 0, nCash, m_VIP.nItemSN, 0, 0, 0, 0);
			}
			else{
				CDNSQLMembership *pMembershipDB = GetMembershipDB();
				if (!pMembershipDB){
					m_pConnection->SendVIPGift(m_nAccountDBID, ERROR_DB, 0, nCash, m_VIP.nItemSN, 0, 0, 0, 0);
					return;
				}
				CDNSQLWorld *pWorldDB = GetWorldDB();
				if (!pWorldDB){
					m_pConnection->SendVIPGift(m_nAccountDBID, ERROR_DB, 0, nCash, m_VIP.nItemSN, 0, 0, 0, 0);
					return;
				}

				UINT nReceiverAccountDBID = 0;
				INT64 biReceiverCharacterDBID = 0;
				pWorldDB->QueryGetCharacterPartialy7(m_wszReceiverCharacterName, nReceiverAccountDBID, biReceiverCharacterDBID);

				int nGiftCount = 0;
				pMembershipDB->QueryGetCountNotReceivedGift(biReceiverCharacterDBID, nGiftCount);

				INT64 nSeedPoint = 0;

				m_pConnection->SendVIPGift(m_nAccountDBID, nResult, GetPetalBalance(), nCash, m_VIP.nItemSN, nReceiverAccountDBID, biReceiverCharacterDBID, nGiftCount, nSeedPoint);
			}
		}
		break;
	}
}

void CDNUserRepository::OnRecvBillingBuyItem(int nBillingResult, UINT nOrderNo, bool bCart)
{
	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		if (m_nSubCommand == CASH_VIPBUY || m_nSubCommand == CASH_VIPGIFT)
			SendVIPBuyGift(ERROR_GENERIC_INVALIDREQUEST, 0);
		else
			SendBuyGift(ERROR_GENERIC_INVALIDREQUEST);
		return;
	}

	char szOrderNo[PUBLISHERORDERKEYMAX] = {0, };
	sprintf_s(szOrderNo, "%u", nOrderNo);

	switch (m_nSubCommand)
	{
	case CASH_BUY:
		{
			if (m_VecBuyItemList.empty()) return;

			int nCashBalance = 0;
			if (nBillingResult == ERROR_NONE && m_nTotalPrice > 0)
			{
				int nRetDeduct = DeductCashAndMapError(pMembershipDB, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice, nCashBalance);
				if (nRetDeduct != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(Buy)] QueryDeductCash Fail(%d) Account(%d) Order(%I64d) Price(%d)\r\n",
						nRetDeduct, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice);
					nBillingResult = nRetDeduct;
				}
			}

			char cOrderCode = (nBillingResult == ERROR_NONE) ? DBDNMembership::OrderStatusCode::Success : DBDNMembership::OrderStatusCode::FailPayment;
			int nTotalReservePetal = 0;
			if (nBillingResult == ERROR_NONE)
			{
				for (int i = 0; i < (int)m_VecBuyItemList.size(); i++)
					nTotalReservePetal += m_VecBuyItemList[i].nReservePetal;
			}

			int nRet = pMembershipDB->QuerySetPurchaseOrderResult(m_biPurchaseOrderID, cOrderCode, szOrderNo, nBillingResult, nTotalReservePetal, (nBillingResult == ERROR_NONE) ? m_nTotalPrice : 0);
			if (nRet != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d)\r\n", nRet, m_biPurchaseOrderID);
				SendBuyGift(nRet);
				return;
			}

			if (nBillingResult == ERROR_NONE)
			{
				for (int i = 0; i < (int)m_VecBuyItemList.size(); i++)
				{
					m_VecBuyItemList[i].BuyItem.bFail = false;
					int nCode = DBDNWorldDef::AddMaterializedItem::CashBuy;

					nRet = CashItemDBProcess(m_cWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nMapID, m_nChannelID, m_cPaymentRules, m_VecBuyItemList[i].BuyItem, m_VecBuyItemList[i].nPrice, m_biPurchaseOrderID, nCode);
					if (nRet != ERROR_NONE)
					{
						pMembershipDB->QueryModGiveFailFlag2(m_VecBuyItemList[i].biPurchaseOrderDetailID, true);
						g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem] CashItemDBQuery Fail(%d) Order(%I64d) Item(SN:%d ID:%d)\r\n", nRet, m_biPurchaseOrderID, m_VecBuyItemList[i].BuyItem.nItemSN, m_VecBuyItemList[i].BuyItem.CashItem.nItemID);
					}
				}
			}

			SendBuyGift(nBillingResult);
		}
		break;

	case CASH_PACKAGEBUY:
		{
			int nCashBalance = 0;
			if (nBillingResult == ERROR_NONE && m_nTotalPrice > 0)
			{
				int nRetDeduct = DeductCashAndMapError(pMembershipDB, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice, nCashBalance);
				if (nRetDeduct != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(Package)] QueryDeductCash Fail(%d) Account(%d) Order(%I64d) Price(%d)\r\n",
						nRetDeduct, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice);
					nBillingResult = nRetDeduct;
				}
			}

			char cOrderCode = (nBillingResult == ERROR_NONE) ? DBDNMembership::OrderStatusCode::Success : DBDNMembership::OrderStatusCode::FailPayment;
			int nPetalAmount = (nBillingResult == ERROR_NONE) ? g_pExtManager->GetCashCommodityReserve(m_nPackageSN) : 0;

			int nRet = pMembershipDB->QuerySetPurchaseOrderResult(m_biPurchaseOrderID, cOrderCode, szOrderNo, nBillingResult, nPetalAmount, (nBillingResult == ERROR_NONE) ? m_nTotalPrice : 0);
			if (nRet != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(Package)] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d)\r\n", nRet, m_biPurchaseOrderID);
				SendBuyGift(nRet);
				return;
			}

			if (nBillingResult == ERROR_NONE)
				CalcPackageProcess();

			SendBuyGift(nBillingResult);
		}
		break;

	case CASH_GIFT:
		{
			if (m_VecGiftItemList.empty())
				return;

			int nOverallResult = nBillingResult;
			for (int i = 0; i < (int)m_VecGiftItemList.size(); i++)
			{
				int nItemResult = nOverallResult;
				if (nItemResult == ERROR_NONE && m_VecGiftItemList[i].nPrice > 0)
				{
					int nCashBalance = 0;
					int nRetDeduct = DeductCashAndMapError(pMembershipDB, m_nAccountDBID, m_VecGiftItemList[i].biPurchaseOrderID, m_VecGiftItemList[i].nPrice, nCashBalance);
					if (nRetDeduct != ERROR_NONE)
					{
						g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(Gift)] QueryDeductCash Fail(%d) Account(%d) Order(%I64d) Price(%d)\r\n",
							nRetDeduct, m_nAccountDBID, m_VecGiftItemList[i].biPurchaseOrderID, m_VecGiftItemList[i].nPrice);
						nItemResult = nRetDeduct;
						nOverallResult = nRetDeduct;
					}
				}

				char cOrderCode = DBDNMembership::OrderStatusCode::FailPayment;
				if (nItemResult == ERROR_NONE)
				{
					m_VecGiftItemList[i].GiftItem.bFail = false;
					cOrderCode = DBDNMembership::OrderStatusCode::Success;
				}

				int nRet = pMembershipDB->QuerySetPurchaseOrderResult(
					m_VecGiftItemList[i].biPurchaseOrderID, cOrderCode, szOrderNo, nItemResult,
					(nItemResult == ERROR_NONE) ? m_VecGiftItemList[i].nReservePetal : 0,
					(nItemResult == ERROR_NONE) ? m_VecGiftItemList[i].nPrice : 0);
				if (nRet != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(Gift)] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d)\r\n", nRet, m_VecGiftItemList[i].biPurchaseOrderID);
					SendBuyGift(nRet);
					return;
				}
			}

			SendBuyGift(nOverallResult);
		}
		break;

	case CASH_PACKAGEGIFT:
		{
			int nCashBalance = 0;
			if (nBillingResult == ERROR_NONE && m_nTotalPrice > 0)
			{
				int nRetDeduct = DeductCashAndMapError(pMembershipDB, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice, nCashBalance);
				if (nRetDeduct != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(GiftPackage)] QueryDeductCash Fail(%d) Account(%d) Order(%I64d) Price(%d)\r\n",
						nRetDeduct, m_nAccountDBID, m_biPurchaseOrderID, m_nTotalPrice);
					nBillingResult = nRetDeduct;
				}
			}

			char cOrderCode = (nBillingResult == ERROR_NONE) ? DBDNMembership::OrderStatusCode::Success : DBDNMembership::OrderStatusCode::FailPayment;
			int nPetalAmount = (nBillingResult == ERROR_NONE) ? g_pExtManager->GetCashCommodityReserve(m_nPackageSN) : 0;

			int nRet = pMembershipDB->QuerySetPurchaseOrderResult(m_biPurchaseOrderID, cOrderCode, szOrderNo, nBillingResult, nPetalAmount, (nBillingResult == ERROR_NONE) ? m_nTotalPrice : 0);
			if (nRet != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(GiftPackage)] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d)\r\n", nRet, m_biPurchaseOrderID);
				SendBuyGift(nRet);
				return;
			}

			SendBuyGift(nBillingResult);
		}
		break;

	case CASH_VIPBUY:
	case CASH_VIPGIFT:
		{
			int nCashBalance = 0;
			if (nBillingResult == ERROR_NONE && m_VIP.nPrice > 0)
			{
				int nRetDeduct = DeductCashAndMapError(pMembershipDB, m_nAccountDBID, m_VIP.biPurchaseOrderID, m_VIP.nPrice, nCashBalance);
				if (nRetDeduct != ERROR_NONE)
				{
					g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(VIP)] QueryDeductCash Fail(%d) Account(%d) Order(%I64d) Price(%d)\r\n",
						nRetDeduct, m_nAccountDBID, m_VIP.biPurchaseOrderID, m_VIP.nPrice);
					nBillingResult = nRetDeduct;
				}
			}

			char cOrderCode = (nBillingResult == ERROR_NONE) ? DBDNMembership::OrderStatusCode::Success : DBDNMembership::OrderStatusCode::FailPayment;
			int nPetalAmount = (nBillingResult == ERROR_NONE) ? g_pExtManager->GetCashCommodityReserve(m_VIP.nItemSN) : 0;

			int nRet = pMembershipDB->QuerySetPurchaseOrderResult(m_VIP.biPurchaseOrderID, cOrderCode, szOrderNo, nBillingResult, nPetalAmount, (nBillingResult == ERROR_NONE) ? m_VIP.nPrice : 0);
			if (nRet != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, this, L"[BillingBuyItem(CASH_VIPBUY)] QuerySetPurchaseOrderResult Fail(%d) Order(Eye:%I64d)\r\n", nRet, m_VIP.biPurchaseOrderID);
				SendVIPBuyGift(nRet, nCashBalance);
				return;
			}

			SendVIPBuyGift(nBillingResult, nCashBalance);
		}
		break;
	}
}

int CDNUserRepository::PurchaseItemByPetal()
{
	INT64 biOrderID = 0;
#ifdef PRE_ADD_LIMITED_CASHITEM
	bool bUpdateLimitedCount = false;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB){
		SendBuyGift(ERROR_GENERIC_INVALIDREQUEST);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	switch (m_nSubCommand)
	{
	case CASH_BUY:
		{
			std::vector<string> VecItemString;
			AppendItemString(0, m_cProductCount, VecItemString);

			int nRet = pMembershipDB->QueryPurchaseItemByPetal(m_biCharacterDBID, m_bPCBang, VecItemString, m_nTotalPrice, m_szIp, biOrderID);
			if (nRet != ERROR_NONE){
				SendBuyGift(nRet);
				return nRet;
			}

#ifdef PRE_ADD_LIMITED_CASHITEM
			if (atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
				bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

			for (int i = 0; i < m_cProductCount; i++){
				nRet = CashItemDBProcess(m_cWorldSetID, m_nAccountDBID, m_biCharacterDBID, m_nMapID, m_nChannelID, m_cPaymentRules, m_VecBuyItemList[i].BuyItem, 
					g_pExtManager->GetCashCommodityPrice(m_ProductPackageList[i].nItemSN), biOrderID, DBDNWorldDef::AddMaterializedItem::ReserveMoneyBuy);

				if (nRet != ERROR_NONE){
					SendBuyGift(nRet);
					return nRet;
				}

				m_VecBuyItemList[i].BuyItem.bFail = false;
			}

		}
		break;

	case CASH_PACKAGEBUY:
		{
			std::vector<string> VecItemString;
			AppendItemString(0, 0, VecItemString);

			int nRet = pMembershipDB->QueryPurchaseItemByPetal(m_biCharacterDBID, m_bPCBang, VecItemString, m_nTotalPrice, m_szIp, biOrderID);
			if (nRet != ERROR_NONE){
				SendBuyGift(nRet);
				return nRet;
			}

#ifdef PRE_ADD_LIMITED_CASHITEM
			if (atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
				bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

			CalcPackageProcess();
		}
		break;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	SendBuyGift(ERROR_NONE, bUpdateLimitedCount);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	SendBuyGift(ERROR_NONE);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	return ERROR_NONE;
}

int CDNUserRepository::AddPurchaseOrderByCash(int nIndex)
{
	INT64 biOrderID = 0;
	int nRet = ERROR_DB;
#ifdef PRE_ADD_LIMITED_CASHITEM
	bool bUpdateLimitedCount = false;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB)
		return ERROR_GENERIC_INVALIDREQUEST;

	switch (m_nSubCommand)
	{
	case CASH_BUY:
		{
			std::vector<string> VecItemString;
			AppendItemString(0, m_cProductCount, VecItemString);

			char szTempOrderDetails[1024] = {0, };
			nRet = pMembershipDB->QueryAddPurchaseOrderByCash(m_biCharacterDBID, m_bPCBang, VecItemString, m_nTotalPrice, m_szIp, false, NULL, NULL, biOrderID, szTempOrderDetails);
			if (nRet != ERROR_NONE)
				return nRet;

#ifdef PRE_ADD_LIMITED_CASHITEM
			if (atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
				bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

			SetPurchaseOrderDetailID(szTempOrderDetails);
			SetPurchaseOrderID(biOrderID);
		}
		break;

	case CASH_PACKAGEBUY:
		{
			std::vector<string> VecItemString;
			AppendItemString(0, 0, VecItemString);

			char szTempOrderDetails[64] = {0, };
			nRet = pMembershipDB->QueryAddPurchaseOrderByCash(m_biCharacterDBID, m_bPCBang, VecItemString, m_nTotalPrice, m_szIp, false, NULL, NULL, biOrderID, szTempOrderDetails);
			if (nRet != ERROR_NONE)
				return nRet;

#ifdef PRE_ADD_LIMITED_CASHITEM
			if (atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
				bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

			SetPurchaseOrderID(biOrderID);
			SetPurchaseOrderDetailID(szTempOrderDetails);
		}
		break;

	case CASH_GIFT:
		{
			for (int i = 0; i < m_cProductCount; i++){
#ifdef PRE_ADD_LIMITED_CASHITEM
				nRet = pMembershipDB->QueryAddPurchaseOrderByCash(m_biCharacterDBID, m_bPCBang, m_VecGiftItemList[i].GiftItem.nItemSN, m_VecGiftItemList[i].nPrice, 
					m_VecGiftItemList[i].nLimitCount, m_VecGiftItemList[i].GiftItem.nItemID, m_VecGiftItemList[i].GiftItem.cItemOption, g_pExtManager->GetCashLimitedItemCount(m_VecGiftItemList[i].GiftItem.nItemSN),
					m_VecGiftItemList[i].nPrice, m_szIp, true, m_wszReceiverCharacterName, m_wszMessage, biOrderID);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
				nRet = pMembershipDB->QueryAddPurchaseOrderByCash(m_biCharacterDBID, m_bPCBang, m_VecGiftItemList[i].GiftItem.nItemSN, m_VecGiftItemList[i].nPrice, 
					m_VecGiftItemList[i].nLimitCount, m_VecGiftItemList[i].GiftItem.nItemID, m_VecGiftItemList[i].GiftItem.cItemOption,
					m_VecGiftItemList[i].nPrice, m_szIp, true, m_wszReceiverCharacterName, m_wszMessage, biOrderID);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

				if (nRet != ERROR_NONE)
					return nRet;

#ifdef PRE_ADD_LIMITED_CASHITEM
				if (g_pExtManager->GetCashLimitedItemCount(m_VecGiftItemList[i].GiftItem.nItemSN) > 0)
					bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

				SetGiftItemPurchaseOrderID(i, biOrderID);
#if defined(PRE_ADD_CADGE_CASH)
				if (m_nMailDBID > 0){
					CDNSQLWorld *pWorldDB = GetWorldDB();
					if (pWorldDB)
						pWorldDB->QueryModWishProducts(m_nMailDBID, biOrderID);
				}
#endif	// #if defined(PRE_ADD_CADGE_CASH)
			}
		}
		break;

	case CASH_PACKAGEGIFT:
		{
			std::vector<string> VecItemString;
			AppendItemString(0, 0, VecItemString);

			INT64 biOrderID = 0;
			nRet = pMembershipDB->QueryAddPurchaseOrderByCash(m_biCharacterDBID, m_bPCBang, VecItemString, m_nTotalPrice, m_szIp, true, m_wszReceiverCharacterName, m_wszMessage, biOrderID);
			if (nRet != ERROR_NONE)
				return nRet;

#ifdef PRE_ADD_LIMITED_CASHITEM
			if (atoi(VecItemString[Append_QuantityLimited].c_str()) > 0)
				bUpdateLimitedCount = true;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

			SetPurchaseOrderID(biOrderID);
#if defined(PRE_ADD_CADGE_CASH)
			if (m_nMailDBID > 0){
				CDNSQLWorld *pWorldDB = GetWorldDB();
				if (pWorldDB)
					pWorldDB->QueryModWishProducts(m_nMailDBID, biOrderID);
			}
#endif	// #if defined(PRE_ADD_CADGE_CASH)
		}
		break;

	case CASH_VIPBUY:
		{

		}
		break;

	case CASH_VIPGIFT:
		{

		}
		break;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	if (g_pLimitedCashItemRepository && bUpdateLimitedCount)
	{
		std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
		if (g_pLimitedCashItemRepository->GetLimitedItemList(vList))
			m_pConnection->SendLimiteItemInfoList(vList);
	}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	return ERROR_NONE;
}

int CDNUserRepository::PurchaseBuyItem()
{
	int nRet = AddPurchaseOrderByCash(m_cCurProductCount);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	nRet = g_pBillingConnection->OnBuy(this);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
	}

	return nRet;
}

int CDNUserRepository::PurchaseGiftItem()
{
	int nRet = AddPurchaseOrderByCash(m_cCurProductCount);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
		return nRet;
	}

	nRet = g_pBillingConnection->OnGift(this);
	if (nRet != ERROR_NONE){
		SendBuyGift(nRet);
	}

	return nRet;
}

int CDNUserRepository::CashItemDBProcess(int nWorldSetID, int nAccountDBID, INT64 biCharacterDBID, int nMapID, int nChannelID, char cPaymentRules, TCashItemBase &BuyItem, int nPrice, INT64 biPurchaseOrderID,
										 int nAddMaterializedItemCode, INT64 biSenderCharacterDBID/* = 0*/, bool bGift/* = false*/, char cPayMethodCode/* = DBDNWorldDef::PayMethodCode::Cash*/)
{
	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB)
		return ERROR_GENERIC_INVALIDREQUEST;

	CDNSQLWorld *pWorldDB = GetWorldDB();
	if (!pWorldDB)
		return ERROR_GENERIC_INVALIDREQUEST;

	int nRet = ERROR_DB;

	if (m_cPaymentRules == Cash::PaymentRules::Petal) 
		nAddMaterializedItemCode = DBDNWorldDef::AddMaterializedItem::ReserveMoneyBuy;

	switch (g_pExtManager->GetItemMainType(BuyItem.CashItem.nItemID))
	{
	case ITEMTYPE_CHARACTER_SLOT:
		{
			char cMaxCount = 0;
			nRet = pMembershipDB->QueryModCharacterSlotCount(biCharacterDBID, 1, cMaxCount);	// 34: Character slot count limit
		}
		break;

	default:
		{
			nRet = pWorldDB->CashItemDBQuery(nAccountDBID, biCharacterDBID, nMapID, nChannelID, cPaymentRules, BuyItem, nPrice, biPurchaseOrderID, nAddMaterializedItemCode, m_wszIp, biSenderCharacterDBID, bGift, cPayMethodCode);
		}
		break;
	}

	return nRet;
}

bool CDNUserRepository::CheckSaleItemSN(int nItemSN)
{
	if (!g_pExtManager->IsOnSaleCommodity(nItemSN)){
		g_Log.Log(LogType::_ERROR, this, L"[CheckSaleItemSN] IsOnSaleCommodity Error(ItemSN:%d)\r\n", nItemSN);
		return false;
	}
	if (!g_pExtManager->IsOnSaleDate(nItemSN)){
		g_Log.Log(LogType::_ERROR, this, L"[CheckSaleItemSN] IsOnSaleDate Error(ItemSN:%d)\r\n", nItemSN);
	    return false;  //
	}

	return true;
}

#if defined(PRE_MOD_SELECT_CHAR)
int CDNUserRepository::CheckCharacterSlotCount(int nAccountDBID, int nAddCount)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
int CDNUserRepository::CheckCharacterSlotCount(INT64 biCharacterDBID, int nAddCount)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	CDNSQLMembership *pMembershipDB = GetMembershipDB();
	if (!pMembershipDB)
		return ERROR_GENERIC_INVALIDREQUEST;

	int nRet = ERROR_DB;
	if (nAddCount > 0){
		char cCharacterCount = 0;
#if defined(PRE_MOD_SELECT_CHAR)
		nRet = pMembershipDB->QueryGetCharacterSlotCount(nAccountDBID, 0, 0, cCharacterCount);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
		nRet = pMembershipDB->QueryGetCharacterSlotCount(0, 0, biCharacterDBID, cCharacterCount);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

		if (nRet == ERROR_NONE){
#if defined(PRE_MOD_SELECT_CHAR)
			if (cCharacterCount + g_pExtManager->GetGlobalWeightValue(1023) >= g_pExtManager->GetGlobalWeightValue(1022)){
#else	// #if defined(PRE_MOD_SELECT_CHAR)
			if (cCharacterCount + nAddCount > (CreateCharacterDefaultCountMax + g_pExtManager->GetGlobalWeightValue(34))){
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
				return ERROR_CASHSHOP_COUNTOVER;
			}
		}
		else
		{
			return ERROR_ITEM_FAIL;
		}
	}

	return ERROR_NONE;
}

void CDNUserRepository::AppendItemString(int nStartIndex, int nEndIndex, std::vector<std::string> &VecItemString)
{
	VecItemString.resize(Append_Max);

	switch (m_nSubCommand)
	{
	case CASH_BUY:
		{
			for (int i = nStartIndex; i < nEndIndex; i++){
				if (m_VecBuyItemList[i].BuyItem.nItemSN > 0){
					if (!VecItemString[Append_ItemSN].empty()){
						VecItemString[Append_ItemSN].append(",");
						VecItemString[Append_Price].append(",");
						VecItemString[Append_Limit].append(",");
						VecItemString[Append_ItemID].append(",");
						VecItemString[Append_Option].append(",");
#if defined(PRE_ADD_CASH_REFUND)
						VecItemString[Append_Refundable].append(",");
#endif
#ifdef PRE_ADD_LIMITED_CASHITEM
						VecItemString[Append_QuantityLimited].append(",");
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
					}
					VecItemString[Append_ItemSN].append(boost::lexical_cast<std::string>(m_VecBuyItemList[i].BuyItem.nItemSN));
					VecItemString[Append_Price].append(boost::lexical_cast<std::string>(g_pExtManager->GetCashCommodityPrice(m_VecBuyItemList[i].BuyItem.nItemSN)));
					VecItemString[Append_Limit].append(boost::lexical_cast<std::string>(g_pExtManager->GetCashBuyAbleCount(m_VecBuyItemList[i].BuyItem.nItemSN)));
					VecItemString[Append_ItemID].append(boost::lexical_cast<std::string>(m_VecBuyItemList[i].BuyItem.CashItem.nItemID));
					BYTE cOption = m_VecBuyItemList[i].BuyItem.CashItem.cOption;
					VecItemString[Append_Option].append(boost::lexical_cast<std::string>(static_cast<int>(cOption)));
#if defined(PRE_ADD_CASH_REFUND)					
					if( m_bCashMoveInven || g_pExtManager->GetCashCommodityNoRefund(m_VecBuyItemList[i].BuyItem.CashItem.nItemID, m_VecBuyItemList[i].BuyItem.nItemSN) )
						VecItemString[Append_Refundable].append("0");	// Go directly into cash inventory
					else
						VecItemString[Append_Refundable].append("1");  // Enter billing inventory
#endif
#ifdef PRE_ADD_LIMITED_CASHITEM
					VecItemString[Append_QuantityLimited].append(boost::lexical_cast<std::string>(g_pExtManager->GetCashLimitedItemCount(m_VecBuyItemList[i].BuyItem.nItemSN)));
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
				}
			}
		}
		break;

	case CASH_PACKAGEBUY:
		{
			VecItemString[Append_ItemSN] = FormatA("%d", m_nPackageSN);
			VecItemString[Append_Price] = FormatA("%d", g_pExtManager->GetCashCommodityPrice(m_nPackageSN));
			VecItemString[Append_Limit] = FormatA("%d", g_pExtManager->GetCashBuyAbleCount(m_nPackageSN));
			for (int i = 0; i < m_cProductCount; i++){
				if (m_ProductPackageList[i].nItemSN > 0){
					if (!VecItemString[Append_ItemID].empty()){
						VecItemString[Append_ItemID].append("|");
						VecItemString[Append_Option].append("|");
					}
					VecItemString[Append_ItemID].append(boost::lexical_cast<std::string>(m_ProductPackageList[i].CashItem.nItemID));
					BYTE cOption = m_ProductPackageList[i].CashItem.cOption;
					VecItemString[Append_Option].append(boost::lexical_cast<std::string>(static_cast<int>(cOption)));
				}
			}

#ifdef PRE_ADD_LIMITED_CASHITEM
			VecItemString[Append_QuantityLimited].append(boost::lexical_cast<std::string>(g_pExtManager->GetCashLimitedItemCount(m_nPackageSN)));
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#if defined(PRE_ADD_CASH_REFUND)
			if( m_bCashMoveInven || g_pExtManager->GetCashCommodityNoRefund(0, m_nPackageSN))	
			{
				VecItemString[Append_Refundable] = "0";		// Put directly into cash inventory
				m_bCashMoveInven = true;
			}
			else 				
				VecItemString[Append_Refundable] = "1";		// To billing inventory
#endif	// #if defined(PRE_ADD_CASH_REFUND)
		}
		break;

	case CASH_PACKAGEGIFT:
		{
			VecItemString[Append_ItemSN] = FormatA("%d", m_nPackageSN);
			VecItemString[Append_Price] = FormatA("%d", g_pExtManager->GetCashCommodityPrice(m_nPackageSN));
			VecItemString[Append_Limit] = FormatA("%d", g_pExtManager->GetCashBuyAbleCount(m_nPackageSN));
			for (int i = 0; i < m_cProductCount; i++){
				if (m_GiftPackageList[i].nItemSN > 0){
					if (!VecItemString[Append_ItemID].empty()){
						VecItemString[Append_ItemID].append("|");
						VecItemString[Append_Option].append("|");
					}	
					VecItemString[Append_ItemID].append(boost::lexical_cast<std::string>(m_GiftPackageList[i].nItemID));
					BYTE cOption = m_GiftPackageList[i].cItemOption;
					VecItemString[Append_Option].append(boost::lexical_cast<std::string>(static_cast<int>(cOption)));
				}
			}

#ifdef PRE_ADD_LIMITED_CASHITEM
			VecItemString[Append_QuantityLimited].append(boost::lexical_cast<std::string>(g_pExtManager->GetCashLimitedItemCount(m_nPackageSN)));
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM		
		}
		break;
	}
}

void CDNUserRepository::SetUsedPaidCash(LPCWSTR pReservedFields)
{
	if (!pReservedFields) return;

	std::vector<std::wstring> UPCTokens;
	TokenizeW(pReservedFields, UPCTokens, L"=");
	if (UPCTokens.size() != 2) return;

	m_nUsedPaidCash = _wtoi(UPCTokens[1].c_str());
}
