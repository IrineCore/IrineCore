#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"
#ifdef PRE_ADD_SALE_COUPON
#include "DnItemTask.h"
#endif // PRE_ADD_SALE_COUPON

// CashShop
inline void SendCashShopOpen()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_SHOPOPEN, NULL, 0);
}

inline void SendCashShopClose()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_SHOPCLOSE, NULL, 0);
}

#ifdef PRE_ADD_CASH_REFUND
inline void SendCashShopBuy(char cartType, char cPaymentRules, const std::vector<TCashShopInfo>& buyItemList, bool bMoveCashInven)
#else
inline void SendCashShopBuy(char cartType, char cPaymentRules, const std::vector<TCashShopInfo>& buyItemList)
#endif
{
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	eBuyMode CashShopBuyMode = GetCashShopTask().GetBuyMode();
	if( CashShopBuyMode == eBuyMode::CASHSHOP_BUY_NONE )
		return;
#endif

	CSCashShopBuy CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopBuy));
	CashShop.cType = cartType;	
#ifdef PRE_ADD_CASH_REFUND
	CashShop.bMoveCashInven = bMoveCashInven;
#endif
	CashShop.cPaymentRules = cPaymentRules;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	switch( CashShopBuyMode )
	{
		case eBuyMode::CASHSHOP_BUY_CASH:
			CashShop.cPaymentRules = Cash::PaymentRules::Prepaid;
			break;
		case eBuyMode::CASHSHOP_BUY_RESERVE:
			CashShop.cPaymentRules = Cash::PaymentRules::Petal;
			break;
#if defined ( PRE_ADD_CASHSHOP_CREDIT )
		case eBuyMode::CASHSHOP_BUY_CREDIT:
			CashShop.cPaymentRules = Cash::PaymentRules::Credit;
			break;
#endif // NONE
	}
#endif
#ifdef PRE_ADD_SALE_COUPON
	CashShop.biSaleCouponSerial = GetCashShopTask().GetApplyCouponSN();

	CDnItem *pItem = GetItemTask().GetCashInventory().FindItemFromSerialID( CashShop.biSaleCouponSerial );
	if( pItem )
	{
		const SCashShopItemInfo* pItemInfo = GetCashShopTask().GetSubCatItemByPresentID( pItem->GetType(), pItem->GetClassID() );
		if( pItemInfo )
			CashShop.nSaleCouponSN = pItemInfo->sn;
	}
#endif // PRE_ADD_SALE_COUPON

	switch (CashShop.cType)
	{
	case eCashUnit_PreviewCart:
	case eCashUnit_Gift_PreviewCart:
		{
			if (buyItemList.size() > PREVIEWCARTLISTMAX)
			{
				_ASSERT(0);
				return;
			}
		}
		break;

	default:
		{
			if (buyItemList.size() > CARTLISTMAX)
			{
				_ASSERT(0);
				return;
			}
		}
		break;
	}

	int i = 0;
	for (; i < int(buyItemList.size()); ++i)
	{
		CashShop.BuyList[i] = buyItemList[i];
	}
	CashShop.cCount = int(buyItemList.size());

	int nLen = sizeof(CSCashShopBuy) - sizeof(CashShop.BuyList) + (sizeof(TCashShopInfo) * CashShop.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_BUY, (char*)&CashShop, nLen);
}

#ifdef PRE_ADD_CASH_REFUND
inline void SendCashShopPackageBuy(char cartType, char cPaymentRules, int nPackageSN, const std::vector<TCashShopPackageInfo>& buyItemList, bool bMoveCashInven)
#else
inline void SendCashShopPackageBuy(char cartType, char cPaymentRules, int nPackageSN, const std::vector<TCashShopPackageInfo>& buyItemList)
#endif
{
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	eBuyMode CashShopBuyMode = GetCashShopTask().GetBuyMode();
	if( CashShopBuyMode == eBuyMode::CASHSHOP_BUY_NONE )
		return;
#endif

	CSCashShopPackageBuy CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopPackageBuy));

	// CashShop.cType = cartType;	
	CashShop.nPackageSN = nPackageSN;
#ifdef PRE_ADD_CASH_REFUND
	CashShop.bMoveCashInven = bMoveCashInven;
#endif
	CashShop.cPaymentRules = cPaymentRules;
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	switch( CashShopBuyMode )
	{
	case eBuyMode::CASHSHOP_BUY_CASH:
		CashShop.cPaymentRules = Cash::PaymentRules::Prepaid;
		break;
	case eBuyMode::CASHSHOP_BUY_RESERVE:
		CashShop.cPaymentRules = Cash::PaymentRules::None;
		break;
#if defined( PRE_ADD_CASHSHOP_CREDIT )
	case eBuyMode::CASHSHOP_BUY_CREDIT:
		CashShop.cPaymentRules = Cash::PaymentRules::Credit;
		break;
#endif // NONE
	}
#endif
#ifdef PRE_ADD_SALE_COUPON
	CashShop.biSaleCouponSerial = GetCashShopTask().GetApplyCouponSN();

	CDnItem *pItem = GetItemTask().GetCashInventory().FindItemFromSerialID( CashShop.biSaleCouponSerial );
	if( pItem )
	{
		const SCashShopItemInfo* pItemInfo = GetCashShopTask().GetSubCatItemByPresentID( pItem->GetType(), pItem->GetClassID() );
		if( pItemInfo )
			CashShop.nSaleCouponSN = pItemInfo->sn;
	}
#endif // PRE_ADD_SALE_COUPON
	if (buyItemList.size() > PACKAGEITEMMAX)
	{
		_ASSERT(0);
		return;
	}

	int i = 0;
	for (; i < int(buyItemList.size()); ++i)
	{
		CashShop.BuyList[i] = buyItemList[i];
	}
	CashShop.cCount = int(buyItemList.size());

	int nLen = sizeof(CSCashShopPackageBuy) - sizeof(CashShop.BuyList) + (sizeof(TCashShopPackageInfo) * CashShop.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_PACKAGEBUY, (char*)&CashShop, nLen);
}

inline void SendCashShopCheckReceiver(const WCHAR *pCharName)
{
	CSCashShopCheckReceiver CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopCheckReceiver));
	_wcscpy(CashShop.wszToCharacterName, _countof(CashShop.wszToCharacterName), pCharName, (int)wcslen(pCharName));
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_CHECKRECEIVER, (char*)&CashShop, sizeof(CSCashShopCheckReceiver));
}

inline void SendCashShopGift(char cartType, const WCHAR *pToCharName, const WCHAR *pMemo, const std::vector<TCashShopInfo>& giftItemList, int nMailDBID)
{
#if defined(PRE_ADD_CASHSHOP_CREDIT)
	eBuyMode CashShopBuyMode = GetCashShopTask().GetBuyMode();
	if( CashShopBuyMode == eBuyMode::CASHSHOP_BUY_NONE )
		return;
#endif

	CSCashShopGift CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopGift));

	CashShop.cType = cartType;
	_wcscpy(CashShop.wszToCharacterName, _countof(CashShop.wszToCharacterName), pToCharName, (int)wcslen(pToCharName));
	_wcscpy(CashShop.wszMessage, _countof(CashShop.wszMessage), pMemo, (int)wcslen(pMemo));

#if defined(PRE_ADD_CADGE_CASH)
	CashShop.nMailDBID = nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

	switch (CashShop.cType)
	{
	case eCashUnit_PreviewCart:
	case eCashUnit_Gift_PreviewCart:
		{
			if (giftItemList.size() > PREVIEWCARTLISTMAX)
			{
				_ASSERT(0);
				return;
			}
		}
		break;

	default:
		{
			if (giftItemList.size() > CARTLISTMAX)
			{
				_ASSERT(0);
				return;
			}
		}
		break;
	}

#if defined(PRE_ADD_CASHSHOP_CREDIT)
	switch( CashShopBuyMode )
	{
	case eBuyMode::CASHSHOP_BUY_CASH:
		CashShop.cPaymentRules = Cash::PaymentRules::Prepaid;
		break;
	case eBuyMode::CASHSHOP_BUY_RESERVE:
		CashShop.cPaymentRules = Cash::PaymentRules::None;
		break;
#if defined( PRE_ADD_CASHSHOP_CREDIT )
	case eBuyMode::CASHSHOP_BUY_CREDIT:
		CashShop.cPaymentRules = Cash::PaymentRules::Credit;
		break;
#endif // NONE
	}
#endif

	int i = 0;
	for (; i < int(giftItemList.size()); ++i)
	{
		CashShop.GiftList[i] = giftItemList[i];
	}
	CashShop.cCount = int(giftItemList.size());

	int nLen = sizeof(CSCashShopGift) - sizeof(CashShop.GiftList) + (sizeof(TCashShopInfo) * CashShop.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_GIFT, (char*)&CashShop, nLen);
}

inline void SendCashShopPackageGift(const WCHAR *pToCharName, const WCHAR *pMemo, int nPackageSN, const std::vector<TCashShopPackageInfo>& giftItemList, int nMailDBID)
{
#ifdef PRE_ADD_CASHSHOP_CREDIT
	eBuyMode CashShopBuyMode = GetCashShopTask().GetBuyMode();
	if( CashShopBuyMode == eBuyMode::CASHSHOP_BUY_NONE )
		return;
#endif // PRE_ADD_CASHSHOP_CREDIT

	CSCashShopPackageGift CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopPackageGift));

	_wcscpy(CashShop.wszToCharacterName, _countof(CashShop.wszToCharacterName), pToCharName, (int)wcslen(pToCharName));
	_wcscpy(CashShop.wszMessage, _countof(CashShop.wszMessage), pMemo, (int)wcslen(pMemo));

#if defined(PRE_ADD_CADGE_CASH)
	CashShop.nMailDBID = nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

#ifdef PRE_ADD_CASHSHOP_CREDIT
	switch( CashShopBuyMode )
	{
	case eBuyMode::CASHSHOP_BUY_CASH:
		CashShop.cPaymentRules = Cash::PaymentRules::Prepaid;
		break;
	case eBuyMode::CASHSHOP_BUY_RESERVE:
		CashShop.cPaymentRules = Cash::PaymentRules::None;
		break;
	case eBuyMode::CASHSHOP_BUY_CREDIT:
		CashShop.cPaymentRules = Cash::PaymentRules::Credit;
		break;
	}
#endif // PRE_ADD_CASHSHOP_CREDIT

	CashShop.nPackageSN = nPackageSN;
	if (giftItemList.size() > PACKAGEITEMMAX)
	{
		_ASSERT(0);
		return;
	}

	int i = 0;
	for (; i < int(giftItemList.size()); ++i)
	{
		CashShop.PackageGiftList[i] = giftItemList[i];
	}
	CashShop.cCount = int(giftItemList.size());

	int nLen = sizeof(CSCashShopPackageGift) - sizeof(CashShop.PackageGiftList) + (sizeof(TCashShopPackageInfo) * CashShop.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_PACKAGEGIFT, (char*)&CashShop, nLen);
}

#ifdef PRE_ADD_CADGE_CASH

inline void SendCashShopCadge(const WCHAR *pToCharName, const WCHAR *pMemo, int nPackageSN, const std::vector<TCashShopPackageInfo>& cadgeItemList, int nMailDBID)
{
	CSCashShopCadge CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopCadge));

	_wcscpy(CashShop.wszToCharacterName, _countof(CashShop.wszToCharacterName), pToCharName, (int)wcslen(pToCharName));
	_wcscpy(CashShop.wszMessage, _countof(CashShop.wszMessage), pMemo, (int)wcslen(pMemo));

	CashShop.nMailDBID = nMailDBID;
	CashShop.nPackageSN = nPackageSN;

	if (cadgeItemList.size() > PACKAGEITEMMAX)
	{
		_ASSERT(0);
		return;
	}

	int i = 0;
	for (; i < int(cadgeItemList.size()); ++i)
	{
		CashShop.PackageGiftList[i] = cadgeItemList[i];
	}
	CashShop.cCount = int(cadgeItemList.size());

	int nLen = sizeof(CSCashShopCadge) - sizeof(CashShop.PackageGiftList) + (sizeof(TCashShopPackageInfo) * CashShop.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_CADGE, (char*)&CashShop, nLen);
}

#endif // PRE_ADD_CADGE_CASH

inline void SendCashShopGiftList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_GIFTLIST, NULL, 0);
}

inline void SendCashShopRecvGift(INT64 giftDBID, const WCHAR* pEmoticonMsg, const WCHAR *pMemo, char cPayMethodCode)
{
	CSCashShopReceiveGift gift;
	memset(&gift, 0, sizeof(CSCashShopReceiveGift));

	gift.GiftData.nGiftDBID = giftDBID;
	gift.GiftData.cPayMethodCode = cPayMethodCode;
	_wcscpy(gift.GiftData.wszEmoticonTitle, _countof(gift.GiftData.wszEmoticonTitle), pEmoticonMsg, (int)wcslen(pEmoticonMsg));
	_wcscpy(gift.GiftData.wszReplyMessage, _countof(gift.GiftData.wszReplyMessage), pMemo, (int)wcslen(pMemo));

	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_RECEIVEGIFT, (char*)&gift, sizeof(CSCashShopReceiveGift));
}

inline void SendCashShopRecvGiftAll(char cCount, TReceiveGiftData *GiftData)
{
	CSCashShopReceiveGiftAll Gift;
	memset(&Gift, 0, sizeof(CSCashShopReceiveGiftAll));

	if (GiftData) {
		Gift.cCount = cCount;
		memcpy(Gift.GiftData, GiftData, sizeof(TReceiveGiftData) * Gift.cCount);
	}
	_wcscpy(Gift.GiftData->wszEmoticonTitle, _countof(Gift.GiftData->wszEmoticonTitle), GiftData->wszEmoticonTitle, _countof(GiftData->wszEmoticonTitle));
	_wcscpy(Gift.GiftData->wszReplyMessage,  _countof(Gift.GiftData->wszReplyMessage),  GiftData->wszReplyMessage, _countof(GiftData->wszReplyMessage));

	int nLen = sizeof(CSCashShopReceiveGiftAll) - sizeof(Gift.GiftData) + (sizeof(TReceiveGiftData) * Gift.cCount);
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_RECEIVEGIFTALL, (char*)&Gift, nLen);
}

inline void SendCashShopCoupon(const WCHAR* couponNum)
{
	CSCashShopCoupon CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopCoupon));
	_wcscpy(CashShop.wszCoupon, _countof(CashShop.wszCoupon), couponNum, (int)wcslen(couponNum));
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_COUPON, (char*)&CashShop, sizeof(CSCashShopCoupon));
}

#if defined(PRE_ADD_GIFT_RETURN)
inline void SendCashShopGiftReturn(INT64 giftDBID)
{
	CSCashShopGiftReturn GiftReturn;
	memset(&GiftReturn, 0, sizeof(CSCashShopGiftReturn));

	GiftReturn.nGiftDBID = giftDBID;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_GIFT_RETURN, (char*)&GiftReturn, sizeof(CSCashShopGiftReturn));
}
#endif

/*
inline void SendCashShopAddWish(int nItemSN)
{
	CSCashShopWish CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopWish));
	CashShop.nItemSN = nItemSN;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_ADDWISH, (char*)&CashShop, sizeof(CSCashShopWish));
}

inline void SendCashShopDeleteWish(int nItemSN)
{
	CSCashShopWish CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopWish));
	CashShop.nItemSN = nItemSN;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_DELETEWISH, (char*)&CashShop, sizeof(CSCashShopWish));
}

inline void SendCashShopList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_WISHLIST, NULL, 0);
}

inline void SendCashShopCoupon(int nCouponNo)
{
	CSCashShopCoupon CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopCoupon));
	CashShop.nCouponNo = nCouponNo;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_COUPON, (char*)&CashShop, sizeof(CSCashShopCoupon));
}

inline void SendCashShopReserveList(int nPageNum)
{
	CSCashShopReserveList CashShop;
	memset(&CashShop, 0, sizeof(CSCashShopReserveList));
	CashShop.wPage = nPageNum;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_RESERVELIST, (char*)&CashShop, sizeof(CSCashShopReserveList));
}
*/

inline void SendCashShopBalanceInquiry()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_BALANCEINQUIRY, NULL, 0);
}

#if defined(PRE_ADD_VIP)
inline void SendCashShopVIPBuy(int nItemSN)
{
	CSVIPBuy CashShop;
	CashShop.nItemSN = nItemSN;
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_VIPBUY, (char*)&CashShop, sizeof(CSVIPBuy));
}

inline void SendCashShopVIPGift(int nItemSN, const WCHAR *pToCharName, const WCHAR *pMemo)
{
	CSVIPGift CashShop;
	memset(&CashShop, 0, sizeof(CSVIPGift));

	_wcscpy(CashShop.wszToCharacterName, _countof(CashShop.wszToCharacterName), pToCharName, (int)wcslen(pToCharName));
	_wcscpy(CashShop.wszMessage, _countof(CashShop.wszMessage), pMemo, (int)wcslen(pMemo));
	CashShop.nItemSN = nItemSN;

	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_VIPGIFT, (char*)&CashShop, sizeof(CSVIPGift));
}
#endif	// #if defined(PRE_ADD_VIP)


inline void SendCashShopGetChargeTime()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_CHARGE_TIME, NULL, 0);
}

#ifdef PRE_ADD_LIMITED_CASHITEM
inline void SendCashShopLimitedItemList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_LIMITEDCASHITEM, NULL, 0);
}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

inline void SendCashShopGiftListByShortCut()
{
	CClientSessionManager::GetInstance().SendPacket(CS_CASHSHOP, eCashShop::CS_GIFTLIST_BY_SHORTCUT, NULL, 0);
}