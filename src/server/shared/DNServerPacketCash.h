#pragma once

#pragma pack(push, 1)

//-------------------------------------------------
//	Cash Packet (Nexon)
//-------------------------------------------------
struct TBillingHeader
{
	BYTE cPacketID;		// Fixed to 0xAF
	UINT nLength;		// 
};


/*-------------------------------------------------
	Cash Packet

Must put AccountDBID in front. User is looked up by that.
-------------------------------------------------*/
struct TCashHeader
{
	UINT nAccountDBID;
};

// CASH_BALANCEINQUIRY
struct TQCashBalanceInquiry : public TCashHeader
{
	char szAccountName[IDLENMAX];
	bool bOpen;
	bool bServer;
	UINT nIp;
};

struct TACashBalanceInquiry : public TCashHeader
{
	int nResult;
	int nCashBalance;
	int nPetal;
	bool bOpen;
	bool bServer;
};

struct TCashItemBase
{
	int nItemSN;
	TItem CashItem;
	DWORD dwPartsColor1;		// Parts color
	DWORD dwPartsColor2;	// Parts color 2
	TVehicleItem VehiclePart1;
	TVehicleItem VehiclePart2;
	int nPaidCashAmount;
};

struct TCashBuyItem: TCashItemBase
{
	char cCartIndex;
	bool bFail;
};

// CASH_BUY
struct TQCashBuyBase : public TCashHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	char szCharacterName[NAMELENMAX];
	char cAge;
	bool bPCBang;
	int nMapID;
	int nChannelID;
#if defined(PRE_ADD_CASH_REFUND)
	bool bMoveCashInven;	// Flag to move directly to cash inventory
#endif
	char cPaymentRules;	// Cash::PaymentRules
};

struct TQCashBuy: TQCashBuyBase
{
	char cCartType;		// Purchase cart type (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX, 50 + eInstantCashShopBuyType: quick purchase)
	char cProductCount;
#if defined(PRE_ADD_SALE_COUPON)
	INT64 biSaleCouponSerial; // Use discount coupon
#endif
	TCashBuyItem BuyList[PREVIEWCARTLISTMAX];
};

struct TACashBuy : public TCashHeader
{
	int nResult;
	char cPaymentRules;	// Cash::PaymentRules
	int nPetalBalance;
	char cCartType;		// Purchase cart type (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX, 50 + eInstantCashShopBuyType: quick purchase)
#if defined(PRE_ADD_CASH_REFUND)
	TPaymentItemInfoEx ItemList[PREVIEWCARTLISTMAX];	
#endif
#if defined(PRE_ADD_SALE_COUPON)
	INT64 biSaleCouponSerial;
#endif
	char cProductCount;
	TCashBuyItem BuyList[PREVIEWCARTLISTMAX];
};

// CASH_PACKAGEBUY
struct TQCashPackageBuy: TQCashBuyBase
{
	int nPackageSN;		// Package representative SN	
#if defined(PRE_ADD_SALE_COUPON)
	INT64 biSaleCouponSerial; // Use discount coupon
#endif
	char cPackageCount;	// Items inside the package
	TCashItemBase BuyList[PACKAGEITEMMAX];
};

struct TACashPackageBuy : public TCashHeader
{
	int nResult;
	char cPaymentRules;	// Cash::PaymentRules	
	int nPetalBalance;
	int nPackageSN;		// Package representative SN
#if defined(PRE_ADD_CASH_REFUND)
	TPaymentPackageItemInfoEx PaymentItem;	
#endif
#if defined(PRE_ADD_SALE_COUPON)
	INT64 biSaleCouponSerial;
#endif
	char cPackageCount;
	TCashItemBase BuyList[PACKAGEITEMMAX];
};

struct TCashGiftItem
{
	char cCartIndex;
	int nItemSN;
	int nItemID;
	char cItemOption;
	bool bFail;
};

struct TCashPackageGiftItem
{
	int nItemSN;
	int nItemID;
	char cItemOption;
};

struct TQCashGiftBase : public TCashHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	char szCharacterName[NAMELENMAX];
	char cAge;
	bool bPCBang;
	int nMapID;
	int nChannelID;
	WCHAR wszReceiverCharacterName[NAMELENMAX];
	WCHAR wszMessage[GIFTMESSAGEMAX];
#if defined(PRE_ADD_CADGE_CASH)
	int nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
};

// CASH_GIFT
struct TQCashGift: TQCashGiftBase
{
	char cCartType;		// Purchase cart type (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX, 50 + eInstantCashShopBuyType: quick purchase)
	char cProductCount;
	TCashGiftItem GiftList[PREVIEWCARTLISTMAX];
};

struct TACashGift : public TCashHeader
{
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverGiftCount;
	int nResult;
	int nPetalBalance;
	char cCartType;		// Purchase cart type (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX, 50 + eInstantCashShopBuyType: quick purchase)
#if defined(PRE_ADD_CADGE_CASH)
	int nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	char cGiftCount;
	TCashGiftItem GiftList[PREVIEWCARTLISTMAX];
};

// CASH_PACKAGEGIFT
struct TQCashPackageGift: TQCashGiftBase
{
	int nPackageSN;		// Package representative SN
	char cPackageCount;	// Items inside the package
	TCashPackageGiftItem GiftList[PACKAGEITEMMAX];
};

struct TACashPackageGift : public TCashHeader
{
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverGiftCount;
	int nResult;
	int nPetalBalance;
	int nPackageSN;		// Package representative SN
#if defined(PRE_ADD_CADGE_CASH)
	int nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	char cPackageCount;
	TCashPackageGiftItem GiftList[PACKAGEITEMMAX];
};

// CASH_COUPON
struct TQCashCoupon : public TCashHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	char szCharacterName[NAMELENMAX];
	WCHAR wszCoupon[COUPONMAX];
	bool bPCBang;
};

struct TACashCoupon : public TCashHeader
{
	int nResult;
};

// CASH_DELUSER
struct TQCashDelUser : public TCashHeader
{
};


// CASH_VIPBUY,
struct TQCashVIPBuy : public TCashHeader
{
	INT64 biCharacterDBID;
	int nItemSN;
	char cWorldSetID;
	bool bPCBang;
	int nMapID;
	int nChannelID;
	bool bAutoPay;			// 0=Manual billing, 1=Auto billing
	bool bServer;			// Whether bought by user or sent by server
};

struct TACashVIPBuy : public TCashHeader
{
	int nResult;
	int nPetal;
	int nCash;
	int nItemSN;
	int nVIPTotalPoint;
	__time64_t tVIPEndDate;
	bool bAutoPay;			// 0=Manual billing, 1=Auto billing
	bool bServer;			// Whether bought by user or sent by server
};

// CASH_VIPGIFT,
struct TQCashVIPGift : public TCashHeader
{
	INT64 biCharacterDBID;
	int nItemSN;
	char cWorldSetID;
	bool bPCBang;
	int nMapID;
	int nChannelID;
	WCHAR wszReceiverCharacterName[NAMELENMAX];
	WCHAR wszMessage[GIFTMESSAGEMAX];
};

struct TACashVIPGift : public TCashHeader
{
	int nResult;
	int nPetal;
	int nCash;
	int nItemSN;
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverGiftCount;
};

#ifdef PRE_ADD_LIMITED_CASHITEM
struct TQGetLimitedItemList
{
};

struct TAGetLimitedItemList
{
	BYTE cIsLast;
	BYTE cCount;
	LimitedCashItem::TLimitedQuantityCashItem Limited[LimitedCashItem::Common::DefaultPacketCount];
};

struct TAGetChangedLimitedItemList
{
	BYTE cCount;
	LimitedCashItem::TChangedLimitedQuantity Limited[LimitedCashItem::Common::DefaultPacketCount];
};

struct TQChangeLimitMax
{
	int nSN;
	int nLimitMax;
};
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

// CASH_SALEABORTLIST
struct TASaleAbortList
{
	BYTE cCount;
	int nAbortList[SALEABORTLISTMAX];
};

// CASH_MOVE_CASHINVEN
struct TQCashMoveCashInven : public TCashHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	int nMapID;
	int nChannelID;
	TCashItemBase CashItem;
	TPaymentItemInfoEx PaymentItemInfo;
};

struct TACashMoveCashInven : public TCashHeader
{
	int nResult;
	int nTotalPetal;			// Petal can be accumulated. (Total Petal)
	TCashItemBase CashItem;
	INT64 biPurchaseOrderDetailID;
};

// CASH_PACKAGE_MOVE_CASHINVEN
struct TQCashPackageMoveCashInven : public TCashHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	int nMapID;
	int nChannelID;
	TCashItemBase CashItemList[PACKAGEITEMMAX];
	TPaymentPackageItemInfoEx PaymentPackageItemInfo;
};

struct TACashPackageMoveCashInven : public TCashHeader
{
	int nResult;
	int nTotalPetal;			// Petal can be accumulated. (Total Petal)	
	TCashItemBase CashItemList[PACKAGEITEMMAX];
	INT64 biPurchaseOrderDetailID;
};

// CASH_REFUNDCASH
struct TQCashRefund : public TCashHeader
{
	char cItemType;				// 1..Normal, 2..Package
	char cWorldSetID;
	int nItemSN;	
	UINT uiOrderNo;				// Nexon
	INT64 biPurchaseOrderDetailID;	// Ours
};

struct TACashRefund : public TCashHeader
{
	int nResult;
	char cItemType;
	INT64 biPurchaseOrderDetailID;	// Ours
};

#pragma pack(pop)
