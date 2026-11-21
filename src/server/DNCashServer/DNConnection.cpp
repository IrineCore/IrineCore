#include "stdafx.h"
#include "DNConnection.h"
#include "DNSQLWorld.h"
#include "DNSQLMembership.h"
#include "DNSQLManager.h"
#include "DNExtManager.h"
#include "DNUserRepository.h"
#include "DNManager.h"
#include "DNManager.h"
#include "Util.h"
#include "Log.h"
#include "DNUserDeleteManager.h"
#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
USER_REQUEST UserRequestCompareExchange(USER_REQUEST volatile* pDestination, USER_REQUEST exchange, USER_REQUEST comperand)
{
#if defined (_WIN64)
	return InterlockedCompareExchange64(pDestination, exchange, comperand);
#else
	return InterlockedCompareExchange(pDestination, exchange, comperand);
#endif // #if defined (_WIN64)
}
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

CDNConnection::CDNConnection(void)
{
#if defined(_FINAL_BUILD)
	Init(1024 * 1024, 1024 * 1024);
#else	// #if defined(_FINAL_BUILD)
	Init(1024 * 100, 1024 * 100);
#endif	// #if defined(_FINAL_BUILD)
}

CDNConnection::~CDNConnection(void)
{
}

int CDNConnection::MessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen)
{
	return 0;
}

void CDNConnection::DBMessageProcess(char *pData, int nThreadID)
{
	DNTPacket *pPacket = (DNTPacket*)pData;
	switch(pPacket->cMainCmd)
	{
	case MAINCMD_CASH: OnRecvCashShop(nThreadID, pPacket->cSubCmd, pPacket->buf); break;
	}
}

void CDNConnection::OnRecvCashShop(int nThreadID, int nSubCmd, char *pData)
{
	switch (nSubCmd)
	{
	case CASH_SALEABORTLIST:
		{
			std::vector<int> VecProhibitSaleList;
			VecProhibitSaleList.clear();

			UINT nAccountDBID;
			memcpy(&nAccountDBID, pData, sizeof(UINT));

			CDNSQLMembership *pMembershipDB = g_pSQLManager->FindMembershipDB(nThreadID);
			if (!pMembershipDB){
				g_Log.Log(LogType::_ERROR, 0, nAccountDBID, 0, 0, L"[M:%d S:%d] pMembershipDB not found\r\n", MAINCMD_CASH, nSubCmd);
				return;
			}
			pMembershipDB->QueryGetListBanOfSale(VecProhibitSaleList);

			SendSaleAbortList(VecProhibitSaleList);
		}
		break;

#ifdef PRE_ADD_LIMITED_CASHITEM
	case CASH_LIMITEDITEM_INFOLIST:
		{
			if (g_pLimitedCashItemRepository)
			{
				std::vector<LimitedCashItem::TLimitedQuantityCashItem> vList;
				if (g_pLimitedCashItemRepository->GetLimitedItemList(vList))
					SendLimiteItemInfoList(vList);

				std::vector<LimitedCashItem::TChangedLimitedQuantity> vChangedList;
				if (g_pLimitedCashItemRepository->GetChangedLimitedItemList(vChangedList))
					SendChangedLimitedItemInfoList(vChangedList);
			}
		}
		break;

	case CASH_CHANGEDLIMITEDITEM_INFOLIST:
		{
			TQChangeLimitMax * pPacket = (TQChangeLimitMax*)pData;
			if (g_pLimitedCashItemRepository)
			{
				if (g_pLimitedCashItemRepository->UpdateChangedLimitedItemMax(pPacket->nSN, pPacket->nLimitMax))
				{
					std::vector<LimitedCashItem::TChangedLimitedQuantity> vChangedList;
					if (g_pLimitedCashItemRepository->GetChangedLimitedItemList(vChangedList))
						SendChangedLimitedItemInfoList(vChangedList);
				}
				else
				{
					_DANGER_POINT_MSG(L"g_pLimitedCashItemRepository->UpdateChangedLimitedItemMax Fail");
				}
			}
		}
		break;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	case CASH_DELUSER:
		{
			TQCashDelUser *pCash = (TQCashDelUser*)pData;

			g_pUserDeleteManager->Add(pCash->nAccountDBID, true);
		}
		break;

	case CASH_BALANCEINQUIRY:
		{
			TQCashBalanceInquiry *pCash = (TQCashBalanceInquiry *)pData;

			g_pUserDeleteManager->Remove(pCash->nAccountDBID, true);

			// Initial add (fetch object if registered under user account)
			CDNUserRepository *pUser = g_pManager->AddUser(pCash->nAccountDBID);
			if (!pUser){
				SendBalanceInquiry(ERROR_GENERIC_USER_NOT_FOUND, pCash->nAccountDBID, 0, 0, pCash->bOpen, pCash->bServer);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnBalanceInquiry(nThreadID, this, pCash);
		}
		break;

	case CASH_BUY:
		{
			TQCashBuy *pCash = (TQCashBuy *)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendBuy(pCash->nAccountDBID, ERROR_ITEM_FAIL, pCash->cCartType, 0, pCash->cProductCount, pCash->BuyList);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnBuy(nThreadID, this, pCash);
		}
		break;

	case CASH_PACKAGEBUY:
		{
            //return;

			TQCashPackageBuy *pCash = (TQCashPackageBuy *)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendPackageBuy(pCash->nAccountDBID, ERROR_ITEM_NOTFOUND, 0, pCash->nPackageSN, pCash->cPackageCount, pCash->BuyList);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnPackageBuy(nThreadID, this, pCash);

		}
		break;

	case CASH_GIFT:
		{

			TQCashGift *pCash = (TQCashGift *)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
#if defined(PRE_ADD_CADGE_CASH)
				SendGift(ERROR_ITEM_NOTFOUND, pCash->nAccountDBID, 0, (INT64)0, 0, 0, pCash->cCartType, pCash->cProductCount, pCash->GiftList, pCash->nMailDBID, 0);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				SendGift(ERROR_ITEM_NOTFOUND, pCash->nAccountDBID, 0, (INT64)0, 0, 0, pCash->cCartType, pCash->cProductCount, pCash->GiftList, 0,0);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnGift(nThreadID, this, pCash);

		}
		break;

	case CASH_PACKAGEGIFT:
		{

			TQCashPackageGift *pCash = (TQCashPackageGift *)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
#if defined(PRE_ADD_CADGE_CASH)
				SendPackageGift(ERROR_ITEM_NOTFOUND, pCash->nAccountDBID, 0, (INT64)0, 0, 0, pCash->nPackageSN, pCash->cPackageCount, pCash->GiftList, pCash->nMailDBID, 0);
#else	// #if defined(PRE_ADD_CADGE_CASH)
				SendPackageGift(ERROR_ITEM_NOTFOUND, pCash->nAccountDBID, 0, (INT64)0, 0, 0, pCash->nPackageSN, pCash->cPackageCount, pCash->GiftList, 0, 0);
#endif	// #if defined(PRE_ADD_CADGE_CASH)
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnPackageGift(nThreadID, this, pCash);

		}
		break;

	case CASH_COUPON:
		{
            return;
/*
			TQCashCoupon *pCash = (TQCashCoupon*)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendCoupon(pCash->nAccountDBID, ERROR_ITEM_NOTFOUND);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			int nRet = pUser->OnCoupon(nThreadID, this, pCash);
			if (nRet != ERROR_NONE)
				return;

*/
		}
		break;

	case CASH_VIPBUY:
		{
			TQCashVIPBuy *pCash = (TQCashVIPBuy*)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendVIPBuy(pCash->nAccountDBID, ERROR_ITEM_FAIL, 0, 0, 0, 0, 0, pCash->bAutoPay, pCash->bServer, 0 );
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			
			pUser->OnVIPBuy(nThreadID, this, pCash);

		}
		break;

	case CASH_VIPGIFT:
		{
			return;
/*
			TQCashVIPGift *pCash = (TQCashVIPGift*)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendVIPGift(pCash->nAccountDBID, ERROR_ITEM_FAIL, 0, 0, 0, 0, 0, 0, 0);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			int nRet = pUser->OnVIPGift(nThreadID, this, pCash);
			if (nRet != ERROR_NONE)
				return;

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			//releaser.Lock();
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

*/

		}
		break;

#if defined(PRE_ADD_CASH_REFUND)
	case CASH_MOVE_CASHINVEN:
		{
			TQCashMoveCashInven* pCash = (TQCashMoveCashInven*)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendMoveCashInven(pCash->nAccountDBID, ERROR_ITEM_NOTFOUND, 0, 0, NULL, 0);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnMoveCashInven(nThreadID, this, pCash);

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			releaser.Lock();
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
		}
		break;

	case CASH_PACKAGE_MOVE_CASHINVEN :
		{
			TQCashPackageMoveCashInven* pCash = (TQCashPackageMoveCashInven*)pData;

			CDNUserRepository *pUser = g_pManager->GetUser(pCash->nAccountDBID);
			if (!pUser){
				SendPackageMoveCashInven(pCash->nAccountDBID, ERROR_ITEM_NOTFOUND, 0, 0, NULL, 0);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnPackageMoveCashInven(nThreadID, this, pCash);

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			releaser.Lock();
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
		}
		break;
	
	case CASH_REFUNDCASH :
		{
			TQCashRefund* pCashRefund = (TQCashRefund*)pData;
			CDNUserRepository *pUser = g_pManager->GetUser(pCashRefund->nAccountDBID);
			if (!pUser){
				SendCoupon(pCashRefund->nAccountDBID, ERROR_ITEM_NOTFOUND);
				return;
			}

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			if (UserRequestCompareExchange(&pUser->m_Requesting, CASH_REQUESTING, CASH_NO_REQUESTING) == CASH_REQUESTING)
				return;

			CRequestReleaser releaser(pUser->m_Requesting);
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

			pUser->OnRefund(nThreadID, this, pCashRefund);
#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
			releaser.Lock();
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
		}
		break;
#endif	// #if defined(PRE_ADD_CASH_REFUND)
	}
}

void CDNConnection::SendSaleAbortList(std::vector<int> &VecProhibitSaleList)
{
	TASaleAbortList Cash = { 0, };

	if (!VecProhibitSaleList.empty()){
		Cash.cCount = (BYTE)VecProhibitSaleList.size();
		for (int i = 0; i < Cash.cCount; i++){
			Cash.nAbortList[i] = VecProhibitSaleList[i];
		}
	}

	AddSendData(MAINCMD_CASH, CASH_SALEABORTLIST, (char*)&Cash, sizeof(TASaleAbortList) - sizeof(Cash.nAbortList) + (sizeof(int) * Cash.cCount));
}

#ifdef PRE_ADD_LIMITED_CASHITEM
void CDNConnection::SendLimiteItemInfoList(std::vector<LimitedCashItem::TLimitedQuantityCashItem> &vList)
{
	TAGetLimitedItemList packet;
	memset(&packet, 0, sizeof(TAGetLimitedItemList));

	for (int i = 0; i < (int)vList.size(); i++)
	{
		packet.Limited[packet.cCount++] = vList[i];
		if (packet.cCount >= LimitedCashItem::Common::DefaultPacketCount)
		{
			AddSendData(MAINCMD_CASH, CASH_LIMITEDITEM_INFOLIST, (char*)&packet, sizeof(TAGetLimitedItemList) - sizeof(packet.Limited) + \
				(sizeof(LimitedCashItem::TLimitedQuantityCashItem) * packet.cCount));
			memset(&packet, 0, sizeof(TAGetLimitedItemList));
		}
	}

	packet.cIsLast = 1;
	AddSendData(MAINCMD_CASH, CASH_LIMITEDITEM_INFOLIST, (char*)&packet, sizeof(TAGetLimitedItemList) - sizeof(packet.Limited) + \
		(sizeof(LimitedCashItem::TLimitedQuantityCashItem) * packet.cCount));
}

void CDNConnection::SendChangedLimitedItemInfoList(std::vector<LimitedCashItem::TChangedLimitedQuantity> &vList)
{
	TAGetChangedLimitedItemList packet;
	memset(&packet, 0, sizeof(TAGetChangedLimitedItemList));

	for (int i = 0; i < (int)vList.size(); i++)
	{
		packet.Limited[packet.cCount++] = vList[i];
		if (packet.cCount >= LimitedCashItem::Common::DefaultPacketCount)
			break;
	}

	AddSendData(MAINCMD_CASH, CASH_CHANGEDLIMITEDITEM_INFOLIST, (char*)&packet, sizeof(TAGetChangedLimitedItemList) - sizeof(packet.Limited) + \
		(sizeof(LimitedCashItem::TLimitedQuantityCashItem) * packet.cCount));
}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

void CDNConnection::SendBalanceInquiry(UINT nResult, UINT nAccountDBID, int nCashBalance, int nPetal, bool bOpen, bool bServer, int nNotRefundableBalance/* = 0*/)
{
	TACashBalanceInquiry Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nCashBalance = nCashBalance;
	Cash.nPetal = nPetal;
	Cash.nResult = nResult;
	Cash.bOpen = bOpen;
	Cash.bServer = bServer;

	AddSendData(MAINCMD_CASH, CASH_BALANCEINQUIRY, (char*)&Cash, sizeof(TACashBalanceInquiry));
}

#if defined(PRE_ADD_SALE_COUPON)
void CDNConnection::SendBuy(UINT nAccountDBID, int nResult, char cCartType, int nPetalBalance, char cProductCount, TCashBuyItem *BuyList, TPaymentItemInfoEx* pPaymentItem, char cPaymentRules, INT64 biSaleCouponSerial, INT64 nSeedPoint/*=0*/)
#else
void CDNConnection::SendBuy(UINT nAccountDBID, int nResult, char cCartType, int nPetalBalance, char cProductCount, TCashBuyItem *BuyList, TPaymentItemInfoEx* pPaymentItem, char cPaymentRules/* = 0*/, INT64 nSeedPoint/*=0*/)
#endif
{
	TACashBuy Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.cPaymentRules = cPaymentRules;	// Cash::PaymentRules	
	Cash.nPetalBalance = nPetalBalance;
	Cash.cCartType = cCartType;
	Cash.cProductCount = cProductCount;
	if ((cProductCount > 0) && BuyList)
		memcpy(Cash.BuyList, BuyList, sizeof(TCashBuyItem) * cProductCount);
#if defined(PRE_ADD_CASH_REFUND)
	if( pPaymentItem )
		memcpy(Cash.ItemList, pPaymentItem, sizeof(Cash.ItemList));
#endif
#if defined(PRE_ADD_SALE_COUPON)
	Cash.biSaleCouponSerial = biSaleCouponSerial;
#endif

	int nLen = sizeof(TACashBuy) - sizeof(Cash.BuyList) + (sizeof(TCashBuyItem) * cProductCount);
	AddSendData(MAINCMD_CASH, CASH_BUY, (char*)&Cash, nLen);
}
#if defined(PRE_ADD_SALE_COUPON)
void CDNConnection::SendPackageBuy(UINT nAccountDBID, int nResult, int nPetalBalance, int nPackageSN, char cPackageCount, TCashItemBase *PackageList, TPaymentPackageItemInfoEx* PaymentPackageItem, char cPaymentRules, INT64 biSaleCouponSerial, INT64 nSeedPoint/*=0*/)
#else
void CDNConnection::SendPackageBuy(UINT nAccountDBID, int nResult, int nPetalBalance, int nPackageSN, char cPackageCount, TCashItemBase *PackageList, TPaymentPackageItemInfoEx* PaymentPackageItem, char cPaymentRules/* = 0*/, INT64 nSeedPoint/*=0*/)
#endif
{
	TACashPackageBuy Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.cPaymentRules = cPaymentRules;	// Cash::PaymentRules	
	Cash.nPackageSN = nPackageSN;
	Cash.cPackageCount = cPackageCount;
	if (PackageList) memcpy(Cash.BuyList, PackageList, sizeof(TCashItemBase) * cPackageCount);
#if defined(PRE_ADD_CASH_REFUND)
	if( PaymentPackageItem )
		memcpy(&Cash.PaymentItem, PaymentPackageItem, sizeof(Cash.PaymentItem));
#endif
#if defined(PRE_ADD_SALE_COUPON)
	Cash.biSaleCouponSerial = biSaleCouponSerial;
#endif
	int nLen = sizeof(TACashPackageBuy) - sizeof(Cash.BuyList) + (sizeof(TCashItemBase) * cPackageCount);
	AddSendData(MAINCMD_CASH, CASH_PACKAGEBUY, (char*)&Cash, nLen);
}
void CDNConnection::SendGift(int nResult, UINT nAccountDBID, UINT nReceiverAccountDBID, INT64 biReceiverCharacterDBID, int nReceiverGiftCount, int nPetal, char cCartType, char cGiftCount, TCashGiftItem *GiftList, int nMailDBID, INT64 nSeedPoint)
{
	TACashGift Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nResult = nResult;
	Cash.nAccountDBID = nAccountDBID;
	Cash.nReceiverAccountDBID = nReceiverAccountDBID;
	Cash.biReceiverCharacterDBID = biReceiverCharacterDBID;
	Cash.nReceiverGiftCount = nReceiverGiftCount;
	Cash.nPetalBalance = nPetal;
	Cash.cCartType = cCartType;
#if defined(PRE_ADD_CADGE_CASH)
	Cash.nMailDBID = nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	Cash.cGiftCount = cGiftCount;
	if ((cGiftCount > 0) && GiftList)
		memcpy(Cash.GiftList, GiftList, sizeof(TCashGiftItem) * cGiftCount);

	int nLen = sizeof(TACashGift) - sizeof(Cash.GiftList) + (sizeof(TCashGiftItem) * cGiftCount);
	AddSendData(MAINCMD_CASH, CASH_GIFT, (char*)&Cash, nLen);
}

void CDNConnection::SendPackageGift(int nResult, UINT nAccountDBID, UINT nReceiverAccountDBID, INT64 biReceiverCharacterDBID, int nReceiverGiftCount, int nPetalBalance, int nPackageSN, char cPackageCount, TCashPackageGiftItem *PackageList, int nMailDBID, INT64 nSeedPoint)
{
	TACashPackageGift Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nReceiverAccountDBID = nReceiverAccountDBID;
	Cash.biReceiverCharacterDBID = biReceiverCharacterDBID;
	Cash.nReceiverGiftCount = nReceiverGiftCount;
	Cash.nResult = nResult;
	Cash.nPetalBalance = nPetalBalance;
	Cash.nPackageSN = nPackageSN;
#if defined(PRE_ADD_CADGE_CASH)
	Cash.nMailDBID = nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	Cash.cPackageCount = cPackageCount;
	if (PackageList) memcpy(Cash.GiftList, PackageList, sizeof(TCashPackageGiftItem) * cPackageCount);

	int nLen = sizeof(TACashPackageGift) - sizeof(Cash.GiftList) + (sizeof(TCashPackageGiftItem) * cPackageCount);
	AddSendData(MAINCMD_CASH, CASH_PACKAGEGIFT, (char*)&Cash, nLen);
}

void CDNConnection::SendCoupon(UINT nAccountDBID, int nResult)
{
	TACashCoupon Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	AddSendData(MAINCMD_CASH, CASH_COUPON, (char*)&Cash, sizeof(TACashCoupon));
}

void CDNConnection::SendVIPBuy(UINT nAccountDBID, int nResult, int nPetal, int nCash, int nItemSN, int nVIPTotalPoint, __time64_t tVIPEndDate, bool bAutoPay, bool bServerBuy, INT64 nSeedPoint)
{
	TACashVIPBuy Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.nPetal = nPetal;
	Cash.nCash = nCash;
	Cash.nItemSN = nItemSN;
	Cash.nVIPTotalPoint = nVIPTotalPoint;
	Cash.tVIPEndDate = tVIPEndDate;
	Cash.bAutoPay = bAutoPay;
	Cash.bServer = bServerBuy;

	AddSendData(MAINCMD_CASH, CASH_VIPBUY, (char*)&Cash, sizeof(TACashVIPBuy));
}

void CDNConnection::SendVIPGift(UINT nAccountDBID, int nResult, int nPetal, int nCash, int nItemSN, UINT nReceiverAccountDBID, INT64 biReceiverCharacterDBID, int nReceiverGiftCount, INT64 nSeedPoint)
{
	TACashVIPGift Cash;
	memset(&Cash, 0, sizeof(Cash));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.nPetal = nPetal;
	Cash.nCash = nCash;
	Cash.nItemSN = nItemSN;
	Cash.nReceiverAccountDBID = nReceiverAccountDBID;
	Cash.biReceiverCharacterDBID = biReceiverCharacterDBID;
	Cash.nReceiverGiftCount = nReceiverGiftCount;

	AddSendData(MAINCMD_CASH, CASH_VIPGIFT, (char*)&Cash, sizeof(TACashVIPGift));
}

#if defined(PRE_ADD_CASH_REFUND)
void CDNConnection::SendMoveCashInven(UINT nAccountDBID, int nResult, int nTotalPetal, INT64 biPurchaseOrderDetailID, TCashItemBase* pCashItem, INT64 nSeedPoint)
{
	TACashMoveCashInven Cash;
	memset(&Cash, 0, sizeof(TACashMoveCashInven));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.nTotalPetal = nTotalPetal;
	Cash.biPurchaseOrderDetailID = biPurchaseOrderDetailID;
	if( pCashItem )
		memcpy( &Cash.CashItem, pCashItem, sizeof(TCashItemBase));

	AddSendData(MAINCMD_CASH, CASH_MOVE_CASHINVEN, (char*)&Cash, sizeof(TACashMoveCashInven));
}

void CDNConnection::SendPackageMoveCashInven(UINT nAccountDBID, int nResult, int nTotalPetal, INT64 biPurchaseOrderDetailID, TCashItemBase* pCashItemList, INT64 nSeedPoint)
{
	TACashPackageMoveCashInven Cash;
	memset(&Cash, 0, sizeof(TACashPackageMoveCashInven));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.nTotalPetal = nTotalPetal;
	Cash.biPurchaseOrderDetailID = biPurchaseOrderDetailID;
	if( pCashItemList )
		memcpy( &Cash.CashItemList, pCashItemList, sizeof(Cash.CashItemList));

	AddSendData(MAINCMD_CASH, CASH_PACKAGE_MOVE_CASHINVEN, (char*)&Cash, sizeof(TACashPackageMoveCashInven));
}

void CDNConnection::SendCashRefund(UINT nAccountDBID, int nResult, char cItemType, INT64 biPurchaseOrderDetailID)
{
	TACashRefund Cash;
	memset(&Cash, 0, sizeof(TACashRefund));

	Cash.nAccountDBID = nAccountDBID;
	Cash.nResult = nResult;
	Cash.cItemType = cItemType;
	Cash.biPurchaseOrderDetailID = biPurchaseOrderDetailID;

	AddSendData(MAINCMD_CASH, CASH_REFUNDCASH, (char*)&Cash, sizeof(TACashRefund));
}
#endif

