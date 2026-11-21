#pragma once

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
#if defined (_WIN64)
typedef LONGLONG USER_REQUEST;
#else
typedef LONG USER_REQUEST;
#endif // #if defined (_WIN64)

const USER_REQUEST CASH_NO_REQUESTING = 0;
const USER_REQUEST CASH_REQUESTING = 1;

void ReleaseExchange(USER_REQUEST volatile* pDestination);

class CRequestReleaser
{
public:
	CRequestReleaser(USER_REQUEST& requesting)
		: m_Requesting(requesting)
	{
		_ASSERT(m_Requesting == CASH_REQUESTING);
		m_Lock = false;
	}

	~CRequestReleaser()
	{
		if (!m_Lock)
			ReleaseExchange(&m_Requesting);
	}

public:
	void Lock() { m_Lock = true; }

private:
	USER_REQUEST& m_Requesting;
	bool m_Lock;
};
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

// Temporary storage for data to send to village/game while waiting for billing round-trip
class CDNSQLWorld;
class CDNSQLMembership;
class CDNConnection;
class CDNUserRepository
{
public:
	struct TBuyItem
	{
		TCashBuyItem BuyItem;

		int nReservePetal;
		int nPrice;
		int nLimitCount;

		INT64 biPurchaseOrderDetailID;	// Our item ID per product
	};

	struct TGiftItem
	{
		TCashGiftItem GiftItem;
		INT64 biPurchaseOrderID;	// Ours
		int nReservePetal;
		int nPrice;
		int nLimitCount;
	};

	struct TVIPItem
	{
		int nItemSN;
		INT64 biPurchaseOrderID;	// Ours
		int nPrice;
		int nLimitCount;
	};

private:
	UINT m_nAccountDBID;
	char m_szAccountName[IDLENMAX];
	WCHAR m_wszAccountName[IDLENMAX];
	char m_szCharacterName[NAMELENMAX];
	WCHAR m_wszCharacterName[NAMELENMAX];
	INT64 m_biCharacterDBID;

	CDNConnection *m_pConnection;
	int m_nThreadID;
	char m_cWorldSetID;
	int m_nMapID;
	int m_nChannelID;

	int m_nSubCommand;

	char m_cPaymentRules;	// Cash::PaymentRules

	int m_nPickupCount;

	bool m_bOpen;	
	bool m_bPCBang;
	char m_cBuyCartType;	// Purchase cart type
	char m_cGiftCartType;	// Gift cart type

	int m_nPackageSN;

	INT64 m_biPurchaseOrderID;			// Ours
#if defined(PRE_ADD_CASH_REFUND)
	INT64 m_biPurchaseOrderDetailID;	// Ours
#endif
	INT64 m_biCouponOrderID;	// Ours
	int m_nCouponSN;

	TCashItemBase m_ProductPackageList[PACKAGEITEMMAX];
	TCashPackageGiftItem m_GiftPackageList[PACKAGEITEMMAX];

#if defined(PRE_ADD_CASH_REFUND)
	bool m_bCashMoveInven;
#endif
#if defined(PRE_ADD_SALE_COUPON)
	INT64 m_biSaleCouponSerial;
#endif
#if defined(PRE_ADD_CADGE_CASH)
	int m_nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)

public:
	char m_cCurProductCount;

	char m_cProductCount;
	std::vector<TBuyItem> m_VecBuyItemList;
	std::vector<TGiftItem> m_VecGiftItemList;

#if defined(PRE_ADD_CASH_REFUND)
	TPaymentItemInfoEx m_PaymentItem[PREVIEWCARTLISTMAX];	
	TPaymentPackageItemInfoEx m_PaymentPackageItem;
	char m_cRefundType;
#endif

	char m_cAge;
	UINT m_nIp;
	char m_szIp[IPLENMAX];
	WCHAR m_wszIp[IPLENMAX];

	WCHAR m_wszReceiverCharacterName[NAMELENMAX];
	WCHAR m_wszMessage[GIFTMESSAGEMAX];

	WCHAR m_wszCoupon[COUPONMAX];
	char m_szCoupon[COUPONMAX]; // For Taiwan and Singapore, convert to char type and store

	int m_nTotalPrice;
	int m_nUsedPaidCash;

	TVIPItem m_VIP;
	bool m_bAutoPay;
	bool m_bServer;

#if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)
	USER_REQUEST m_Requesting;
#endif // #if defined (PRE_ADD_REJECT_DUPLICATED_CASH_REQUEST)

private:
	void Clear();

public:
	CDNUserRepository(void);
	~CDNUserRepository(void);

	void SetAccountDBID(UINT nAccountDBID) { m_nAccountDBID = nAccountDBID; }
	UINT GetAccountDBID() { return m_nAccountDBID; }
	void SetAccountName(char *pName) { _strcpy(m_szAccountName, _countof(m_szAccountName), pName, (int)strlen(pName)); }
	char *GetAccountName() { return m_szAccountName; }
	WCHAR *GetAccountNameW() { return m_wszAccountName; }
	char *GetCharacterName() { return m_szCharacterName; }
	WCHAR *GetCharacterNameW() { return m_wszCharacterName; }
	INT64 GetCharacterDBID() { return m_biCharacterDBID; }
	char GetWorldID() { return m_cWorldSetID; }
	int GetMapID() { return m_nMapID; }
	int GetChannelID() { return m_nChannelID; }
	WCHAR *GetReceiverCharacterNameW() { return m_wszReceiverCharacterName; }

	char *GetCoupon() { return m_szCoupon; }
	WCHAR *GetCouponW() { return m_wszCoupon; }

	CDNConnection * GetConnection() { return m_pConnection; }
	int GetThreadID() { return m_nThreadID; }

	char GetPaymentRules() { return m_cPaymentRules; }

	bool IsOpen() { return m_bOpen; }	
	bool IsPCBang() { return m_bPCBang; }
	char GetBuyCartType() { return m_cBuyCartType; }	// Purchase cart type
	int GetPackageSN() { return m_nPackageSN; }

	void SetPurchaseOrderID(INT64 biPurchaseOrderID) { m_biPurchaseOrderID = biPurchaseOrderID; }
	INT64 GetPurchaseOrderID() const { return m_biPurchaseOrderID; }

	int GetSubCommand() { return m_nSubCommand; }
	void SetPurchaseOrderDetailID(const char* pPurchaseOrderDetailIDs);

	int GetPetalBalance();

	CDNSQLMembership *GetMembershipDB();
	CDNSQLWorld *GetWorldDB();

	bool SetBuyItem(TCashBuyItem &Item);
	CDNUserRepository::TBuyItem *GetBuyItem(int nIndex);
	bool SetGiftItem(TCashGiftItem &Item);
	void SetGiftItemPurchaseOrderID(int nIndex, INT64 biPurchaseOrderID);
	CDNUserRepository::TGiftItem *GetGiftItem(int nIndex);

	void SetBalanceInquiry(int nThreadID, CDNConnection *pConnection, TQCashBalanceInquiry *pCash);
	bool SetBuy(int nThreadID, CDNConnection *pConnection, TQCashBuy *pCash);
	void SetPackageBuy(int nThreadID, CDNConnection *pConnection, TQCashPackageBuy *pCash);
	bool SetGift(int nThreadID, CDNConnection *pConnection, TQCashGift *pCash);
	void SetPackageGift(int nThreadID, CDNConnection *pConnection, TQCashPackageGift *pCash);
	void SetCoupon(int nThreadID, CDNConnection *pConnection, TQCashCoupon *pCash);
	void SetVIPBuy(int nThreadID, CDNConnection *pConnection, TQCashVIPBuy *pCash);
	void SetVIPGift(int nThreadID, CDNConnection *pConnection, TQCashVIPGift *pCash);
	void SetRefund(int nThreadID, CDNConnection *pConnection, TQCashRefund* pCash);

	int OnBalanceInquiry(int nThreadID, CDNConnection *pConnection, TQCashBalanceInquiry *pCash);
	int OnBuy(int nThreadID, CDNConnection *pConnection, TQCashBuy *pCash);
	int OnPackageBuy(int nThreadID, CDNConnection *pConnection, TQCashPackageBuy *pCash);
	int OnGift(int nThreadID, CDNConnection *pConnection, TQCashGift *pCash);
	int OnPackageGift(int nThreadID, CDNConnection *pConnection, TQCashPackageGift *pCash);
	int OnCoupon(int nThreadID, CDNConnection *pConnection, TQCashCoupon *pCash);
	int OnVIPBuy(int nThreadID, CDNConnection *pConnection, TQCashVIPBuy *pCash);
	int OnVIPGift(int nThreadID, CDNConnection *pConnection, TQCashVIPGift *pCash);
	void OnRefund(int nThreadID, CDNConnection *pConnection, TQCashRefund* pCash);
	void OnMoveCashInven(int nThreadID, CDNConnection *pConnection, TQCashMoveCashInven* pCash);
	void OnPackageMoveCashInven(int nThreadID, CDNConnection *pConnection, TQCashPackageMoveCashInven* pCash);

	int CalcPackageProcess();

	bool OnRecvBillingCoupon(int nBillingResult, int nResult, int nItemSN, UINT nOrderNo);

	void OnRecvBillingBuyItem(int nBillingResult, UINT nOrderNo=0, bool bCart=false);

	// Send via Connection.
	void SendBalanceInquiry(int nResult, int nCashBalance, int nNotRefundableBalance = 0);
#ifdef PRE_ADD_LIMITED_CASHITEM
	void SendBuyGift(int nResult, bool bUpdateLimitedCount = false);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
	void SendBuyGift(int nResult);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
	void SendCoupon(int nResult);
	void SendVIPBuyGift(int nResult, int nCash);

#if defined(PRE_ADD_CASH_REFUND)
	void SendCashRefund(int nResult);
	void OnRecvBillingRefund(int nResult);
	inline bool GetCashMoveInven() { return m_bCashMoveInven; };
	inline void SetCashMoveInven(bool bCashMoveInven) { m_bCashMoveInven = bCashMoveInven; };		
#endif

	int PurchaseItemByPetal();
	int AddPurchaseOrderByCash(int nIndex);
	int PurchaseBuyItem();
	int PurchaseGiftItem();

	// Handle CashItemDBQuery inside.. wrapped one more time
	int CashItemDBProcess(int nWorldSetID, int nAccountDBID, INT64 biCharacterDBID, int nMapID, int nChannelID, char cPaymentRules, TCashItemBase &BuyItem, int nPrice, 
		INT64 biPurchaseOrderID, int nAddMaterializedItemCode, INT64 biSenderCharacterDBID = 0, bool bGift = false, char cPayMethodCode = DBDNWorldDef::PayMethodCode::Cash);

	bool CheckSaleItemSN(int nItemSN);
#if defined(PRE_MOD_SELECT_CHAR)
	int CheckCharacterSlotCount(int nAccountDBID, int nAddCount);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int CheckCharacterSlotCount(INT64 biCharacterDBID, int nAddCount);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	void AppendItemString(int nStartIndex, int nEndIndex, std::vector<std::string> &VecItemString);
	void SetUsedPaidCash(LPCWSTR pReservedFields);

};

