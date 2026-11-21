
#pragma once

#include "DnParts.h"
#include "MAPartsBody.h"
#include "DNDefine.h"

#define MAX_LINK_SN				5
#define MAX_ABILITY_ID			5
#define MAX_PACKAGE_GOODS_ID	20
#define MAX_SUB_CATEGORY_PER_PAGE		9

#define _MAX_COORDI_SLOT				4
#define _MAX_INVEN_SLOT_PER_LINE		10
#define _MAX_INVEN_LINE					4
#define _MAX_PAGE_COUNT_PER_GROUP		5

#ifdef PRE_ADD_CASHSHOP_RENEWAL
		#define _MAX_GOODS_SLOT_NUM_PER_PAGE 10
		#define _MAX_GOODS_SLOT_NUM_PER_MAINPAGE 6 // On MainPage, 4 items are covered by the banner.
#else
#define _MAX_GOODS_SLOT_NUM_PER_PAGE		8
#endif // PRE_ADD_CASHSHOP_RENEWAL

#define _MAX_RECV_GIFT_SLOT_NUM_PER_PAGE	2
#define _MAX_PAY_SLOT_NUM_PER_PAGE			3

//	send gift
#define MAX_SEND_GIFT_MEMO_LENGTH		100
#define SEND_GIFT_CHECK_USER_COOLTIME	3

#define MAX_GIFT_IMPRESSION_RADIO_BTN	3
#define MAX_GIFT_RECV_INFO				8
#define COUPON_NUMBER_SEPARATE_UNIT		5

enum eCashShopItemType
{
	eCSType_None = -1,
	eCSType_Costume,
	eCSType_Function,
	eCSType_ePackage,
#ifdef PRE_ADD_VIP
	eCSType_Term,
#endif // PRE_ADD_VIP
};

enum eCashShopMainTabOption
{
	eCSMTOPT_WISHLIST	= 0x0001,
	eCSMTOPT_JOBSORT	= 0x0002,
};

typedef int	eCashShopSubCatType;

enum eCashUnitType
{
	eCashUnit_None,

	eCashUnit_Cart,
	eCashUnit_BuyItemNow,
	eCashUnit_PreviewCart,
	eCashUnit_Package,

	eCashUnit_Gift_Min,
	eCashUnit_Gift_Cart = eCashUnit_Gift_Min,
	eCashUnit_GiftItemNow,
	eCashUnit_Gift_PreviewCart,
	eCashUnit_Gift_Package,
	eCashUnit_Gift_Max,
#ifdef PRE_ADD_INSTANT_CASH_BUY
	eCashUnit_BuyInstant = 50,
#endif // PRE_ADD_INSTANT_CASH_BUY
};

#ifdef PRE_ADD_ACADEMIC
#else
enum eCashClassType
{
	CASHCLASS_NONE,
	CASHCLASS_WARRIOR,
	CASHCLASS_ARCHER,
	CASHCLASS_SOCERESS,
	CASHCLASS_CLERIC,
	CASHCLASS_MAX,
	CASHCLASS_COUNT = CASHCLASS_MAX - 1
};
#endif

enum eCashShopEtcSortType
{
	SORT_BASIC,
	SORT_LEVEL_DESCENDING,
	SORT_LEVEL_ASCENDING,
	SORT_NAME_DESCENDING,
	SORT_NAME_ASCENDING,
	//SORT_RELEASE_NEW,
	SORT_MYCLASS,
	SORT_MAX,
};

enum eRetCartAction
{
	eRETCART_UPDATE,
	eRETCART_ADD,
	eRETCART_REMOVE,
	eRETCART_CLEAR,
};

enum eRetPreviewChange
{
	eRETPREVIEW_ERROR,
	eRETPREVIEW_CHANGE,
	eRETPREVIEW_NOCHANGE,
};

enum eCashShopError
{
	eERRCS_NONE = 0,
	eERRCS_CART_MAX = 10000,
	eERRCS_PREVIEWCART_MAX,
	eERRCS_PREVIEWCART_CHR_ID,
	eERRCS_PREVIEWCART_CLEAR_INVALID_CLASS,
	eERRCS_PREVIEWCART_ALREADY_EXIST,
	eERRCS_CART_NOT_EXIST,
	eERRCS_CART_EMPTY,
	eERRCS_REBIRTHCOIN_MAX,
	eERRCS_CANT_OPEN_CHARGEBROWSER,
	eERRCS_CANT_OPEN_CHARGEBROWSER_D3D,
	eERRCS_PREVIEWCART_ALREADY_EQUIP,	// 10010
	eERRCS_NO_ALL_ITEM_PERIOD_SELECTED,
	eERRCS_RECV_GIFT_PACKET,
	eERRCS_GIFT_UNABLE_INVALID_CART,
	eERRCS_GIFT_UNABLE_NOGIFT_OPTION,
	eERRCS_GIFT_UNABLE_CLASS,
	eERRCS_GIFT_WARNING_LEVEL,
	eERRCS_CART_UNABLE_BUY_INVALID_CART,
	eERRCS_CART_UNABLE_BUY_DLG_INFO_FAIL,
	eERRCS_NO_ALL_ITEM_ABILITY_SELECTED,
	eERRCS_CART_UNABLE_BUY_COUNT,		// 10020
	eERRCS_PREVIEWCART_UNABLE_BUY_COUNT,
	eERRCS_VIP_NO_DATAMGR,
	eERRCS_BUY_NO_ITEM_INFO,
	eERRCS_NO_DIALOG,
	eERRCS_NO_PACKAGE_ITEM_INFO,
	eERRCS_REFUND_ITEM_ERROR,
	eERRCS_CANT_OPEN_CHARGEBROWSER_COCREATE_FAIL,
	eERRCS_CANT_OPEN_CHARGEBROWSER_SIZE_FAIL,
	eERRCS_CANT_OPEN_CHARGEBROWSER_TYPE_FAIL,
	eERRCS_CANT_OPEN_CHARGEBROWSER_GET_ACCOUNT_FAIL,	// 10030
	eERRCS_CANT_OPEN_CHARGEBROWSER_NAVIGATE_FAIL,
	eERRCS_CANT_OPEN_CHARGEBROWSER_ATTACH_IE_EVENT_FAIL,
	eERRCS_CANT_OPEN_CHARGEBROWSER_SERVERTIME_FAIL,
	eERRCS_NO_ALL_ITEM_SKILL_SELECTED,
};

enum eCashShopMode
{
	eCSMODE_NONE					= 0x00000000,
	eCSMODE_REFUND					= 0x00000001,
	eCSMODE_WARN_PACKAGEWITHINVEN	= 0x00000002,
	eCSMODE_CHARGE_REQ_SERVERTIME	= 0x00000004,
};

enum eCashChargeBrowserType
{
	eCCBT_INGAME_BROWSER,
	eCCBT_FULL_BROWSER,
};

enum eGeneralBrowserType
{
	eGBT_GENERAL,
	eGBT_NPCBROWSER,
	eGBT_SIMPLE,
};

enum eCashShopTabShowType
{
	eCST_HIDE,
	eCST_SHOW,
	eCST_DISABLE
};

#if defined(PRE_ADD_CASHSHOP_CREDIT)
enum eBuyMode
{
	CASHSHOP_BUY_NONE = 0, 
	CASHSHOP_BUY_CASH, 
	CASHSHOP_BUY_RESERVE,
#if defined( PRE_ADD_CASHSHOP_CREDIT )
	CASHSHOP_BUY_CREDIT,
#endif // NONE
};
#endif

struct SCashShopTabShowInfo
{
	eCashShopTabShowType	showType;
	int						uiStringNum;

	SCashShopTabShowInfo()
	{
		showType = eCST_HIDE;
		uiStringNum = -1;
	}
};

#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
struct SCashShopSubTabShowInfo : public SCashShopTabShowInfo
{
	CString						szSelectedColor;
	CString						szDeSelectedColor;

	SCashShopSubTabShowInfo() : szSelectedColor(""),
								szDeSelectedColor("") 
	{}
};

#endif

struct SCashShopMainTabShowInfo : public SCashShopTabShowInfo
{
	int	option;

	SCashShopMainTabShowInfo() : option(0) {}
};





struct SCartActionParam
{
	CASHITEM_SN presentSN;
	int			cartItemId;

	SCartActionParam()
	{
		presentSN				= -1;
		cartItemId	= -1;
	}

	bool IsEmpty() { return (presentSN == -1); }
};

struct SCashShopItemInfo
{
	int				id;
	CASHITEM_SN		sn;
	int				category;
	int				subCategory;
	std::wstring	nameString;
	int				descId;
	ITEMCLSID		presentItemId;
	std::vector<ITEMCLSID>		abilityList;
	std::vector<CASHITEM_SN>	linkIdList;
	int				levelLimit;

	int				period;
	int				price;
	int				priceFix;
	int				count;
	int				priority;
	bool			bOnSale;
	
#ifdef PRE_ADD_CASHSHOP_RENEWAL
	std::vector<int> arrState;
#else
	int				state;
#endif // PRE_ADD_CASHSHOP_RENEWAL

	bool			bLimit;

	bool			bReserveOffer;
	int				ReserveAmount;
	bool			bReserveUsable;
	bool				bGiftUsable;
	eCashShopItemType	type;

	bool				bCartAble;

	bool				bEnableJob[CLASSKINDMAX + 1];
	bool				bOnlyOneClassEnable;	// For use during compare
	
#ifdef PRE_ADD_VIP
	bool			bVIPSell;
	int				neededVIPLevel;
#endif

	int				nOverlapBuy;
#ifdef PRE_ADD_CASHSHOP_CREDIT
	bool			bCreditAble;
#endif // PRE_ADD_CASHSHOP_CREDIT

#ifdef PRE_MOD_SALE_COUPON_LEVEL_LIMIT
	bool			bLimitUseCouponLevel;
	int				nLimitUseCouponLevel;
#endif // PRE_MOD_SALE_COUPON_LEVEL_LIMIT

	SCashShopItemInfo()
	{
		id				= -1;
		sn				= -1;
		category		= 0;
		subCategory		= 0;
		descId			= 0;
		presentItemId	= ITEMCLSID_NONE;

		period			= 0;		// -1 : permanent
		price			= 0;
		priceFix		= 0;
		count			= 0;
		priority		= 0;
		bOnSale			= false;
		
#ifdef PRE_ADD_CASHSHOP_RENEWAL
		// ...
#else
		state			= 0;
#endif // PRE_ADD_CASHSHOP_RENEWAL

		bLimit			= false;

		bReserveOffer	= false;
		ReserveAmount	= 0;
		bReserveUsable	= false;
		bGiftUsable		= false;
		type			= eCSType_None;
		levelLimit		= 0;
		bCartAble		= false;

		memset(bEnableJob, 0, _countof(bEnableJob));
		bOnlyOneClassEnable = false;

#ifdef PRE_ADD_VIP
		bVIPSell		= false;
		neededVIPLevel	= 0;
#endif
		nOverlapBuy		= 0;
#ifdef PRE_ADD_CASHSHOP_CREDIT
		bCreditAble		= false;
#endif // PRE_ADD_CASHSHOP_CREDIT

#ifdef PRE_MOD_SALE_COUPON_LEVEL_LIMIT
		bLimitUseCouponLevel = false;
		nLimitUseCouponLevel = 0;
#endif // PRE_MOD_SALE_COUPON_LEVEL_LIMIT
	}
};

typedef std::map<CASHITEM_SN, SCashShopItemInfo*>					CS_BASIC_INFO_LIST;
typedef std::map<CASHITEM_SN, std::vector<ITEMCLSID> >				CS_ABILITY_INFO_LIST;
typedef std::map<CASHITEM_SN, std::vector<CASHITEM_SN> >			CS_PERIOD_INFO_LIST;

typedef std::vector<SCashShopItemInfo*>								CS_INFO_LIST;
typedef std::map<eCashShopSubCatType, CS_INFO_LIST>					CS_CATEGORY_LIST;

struct SCashShopRecvGiftBasicInfo
{
	int				payMethodCode;
	INT64			giftDBID;
	CASHITEM_SN		sn;
	ITEMCLSID		itemId;
	std::wstring	name;
	std::wstring	recvDate;
	std::wstring	expireDate;
	std::wstring	memo;
#if defined(PRE_ADD_GIFT_RETURN)
	bool			giftReturn;
#endif
	bool			bNewFlag;

	void Clear()
	{
		payMethodCode	= 0;
		giftDBID	= -1;
		sn			= 0;
		itemId		= ITEMCLSID_NONE;
		name		= L"";
		recvDate	= L"";
		expireDate	= L"";
		memo		= L"";
#if defined(PRE_ADD_GIFT_RETURN)
		giftReturn	= false;
#endif
		bNewFlag	= false;
	}

	SCashShopRecvGiftBasicInfo()
	{
		Clear();
	}
};

typedef std::vector<SCashShopRecvGiftBasicInfo>		RECV_GIFT_LIST;

struct SCashShopCartItemInfo
{
	int				id;
	CASHITEM_SN		presentSN;
	CASHITEM_SN		selectedSN;
	int				equipIdx;
	ITEMCLSID		ability;
	bool			isPackage;
	int				nOptionIndex;

	SCashShopCartItemInfo()
	{
		Clear();
	}

	void Clear()
	{
		id		= -1;
		presentSN = -1;
		selectedSN = -1;
		equipIdx = -1;
		ability = ITEMCLSID_NONE;
		isPackage = false;
		nOptionIndex = -1;
	}

	bool IsEmpty() const { return (presentSN == -1); }
};

typedef std::list<SCashShopCartItemInfo>			CART_ITEM_LIST;

struct SCashItemLimitInfo
{
	int nBuyAbleCount;
#ifdef PRE_ADD_LIMITED_CASHITEM
	int	nSaleCount;
	int	nMaxCount;
#endif // PRE_ADD_LIMITED_CASHITEM
	std::wstring start;
	std::wstring end;

	SCashItemLimitInfo()
	{
		nBuyAbleCount	= 0;
#ifdef PRE_ADD_LIMITED_CASHITEM
		nSaleCount	= 0;
		nMaxCount		= 0;
#endif // PRE_ADD_LIMITED_CASHITEM
	}
};

//	todo by kalliste : eliminate duplication with DefaultPartsStruct in LoginTask
struct DefaultPartsStruct
{
	ITEMCLSID				nSelectPartsIndex[EQUIPMAX];
	ITEMCLSID				nSelectCashPartsIndex[EQUIPMAX];
	DWORD					dwColor[MAPartsBody::PartsColorAmount];
	ITEMCLSID				nWeapon[2];
	ITEMCLSID				nCashWeapon[2];
	ITEMCLSID				nSelectCashPartsLookIndex[EQUIPMAX];
	ITEMCLSID				nCashWeaponLookIndex[2];
};

struct SGiftSendBasicInfo
{
	std::wstring	receiverName;
	std::wstring	memo;

	void Clear()			{ receiverName.clear(); memo.clear(); }
	bool IsEmpty() const	{ return receiverName.empty(); }
};
