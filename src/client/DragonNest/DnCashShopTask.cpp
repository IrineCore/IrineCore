#include "StdAfx.h"
#include "EtPrimitiveDraw.h"
#include "DnCashShopTask.h"
#include "DnInterfaceCamera.h"
#include "DnInterface.h"
#include "DnTableDB.h"
#include "CashShopSendPacket.h"
#include "DnCashShopDlg.h"
#include "StringUtil.h"
#include "DnCommonUtil.h"
#include "DnWorld.h"
#include "DnItemTask.h"
#include "DnPartyTask.h"
#include "DnTradeTask.h"
#include "DnTradePrivateMarket.h"
#include "DnMainFrame.h"
#include "GameOption.h"
#include "DnLocalPlayerActor.h"
#include "DnInterfaceString.h"
#include "TimeSet.h"
#include "DnMainDlg.h"
#include "DnNPCActor.h"
#include "DnBridgeTask.h"
#include "DnMasterTask.h"
#include "DnCashShopPreviewDlg.h"

#ifdef PRE_ADD_SALE_COUPON
#include "SyncTimer.h"
#endif // PRE_ADD_SALE_COUPON

#ifdef PRE_MOD_SYSTEM_STATE
#include "DnSystemStateManager.h"
#endif

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
#include "TaskManager.h"
#include "DnCommonTask.h"
#endif

#ifdef PRE_ADD_INSTANT_CASH_BUY
#include "DnInstantCashShopBuyDlg.h"
#endif // PRE_ADD_INSTANT_CASH_BUY
#include "DnSecretCashShopDlg.h"


#ifdef PRE_ADD_CASHSHOP_RENEWAL
#include "DnMainMenuDlg.h"
#include "DnCashShopIngameBanner.h"
#endif // PRE_ADD_CASHSHOP_RENEWAL

#ifdef PRE_MOD_BROWSER
#else // PRE_MOD_BROWSER
CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

CComQIPtr<IWebBrowser2>			pChargeWebBrowser;
CComObject<CDnCashShopIESink>*	pIESink;
HWND							g_hwndChargeBrowser;
#ifdef PRE_FIX_NOMOVE_BROWSER
WNDPROC							g_ChildWndProc;
#endif
#endif // PRE_MOD_BROWSER

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCashShopTask::CDnCashShopTask()
	: CTaskListener(true)
{
	m_bRequestingGiftBasicInfoes	= false;
	m_pCashShopDlg					= NULL;
	m_CartIndexGen					= 0;
	m_PreviewCartIndexGen			= 0;
#ifdef PRE_ADD_CASHSHOP_CREDIT
	m_UserCredit					= 0;
	m_UserPrepaid					= 0;
	m_eBuyMode						= eBuyMode::CASHSHOP_BUY_NONE;
#endif // PRE_ADD_CASHSHOP_CREDIT
	m_UserCash						= 0;
	m_UserReserve					= 0;
	m_bOpened						= false;
	m_PreviewCartClassId			= 0;
	m_CashShopBGMIndex				= -1;
	m_bSoundOnOpen					= false;
	m_fSoundDelta					= 0.f;
	m_bSoundOnClose					= false;
	int i = 1;
	for (; i <= CLASSKINDMAX; ++i)
		SetPackagePreview(i, ePreview_Normal, 0);
	m_CashCoinRestriction			= 0;
	m_bChargeBrowserOpened			= false;
	m_bGeneralBrowserOpened			= false;
	m_GiftListCount					= 0;

	m_bRequestingCashShopOpen		= false;

	m_bBuying = false;
#if defined(_WORK)
	m_bCashTradable = true;
#else
	m_bCashTradable = false;
#endif

	m_bCashTradeCountRestrict = true;
	m_CashShopMode = eCSMODE_NONE;

#if defined _WORK
	m_CashShopMode &= ~eCSMODE_REFUND;
	m_CashShopMode |= eCSMODE_CHARGE_REQ_SERVERTIME;
#endif

#if defined _WORK
	m_ChargeBrowserType = eCCBT_FULL_BROWSER;
	m_ChargeBrowserWidth = 0;
	m_ChargeBrowserHeight = 0;
#endif


	m_ChargeTime = 0;
#ifdef PRE_ADD_SALE_COUPON
	m_nApplyCouponSN = 0;
#endif // PRE_ADD_SALE_COUPON

	m_bFullScreenMode = false;
#ifdef PRE_MOD_BROWSER
#else
	m_GeneralBrowserOpenedType = eGBT_GENERAL;
#endif

#ifdef PRE_RECEIVEGIFTALL
	m_nSendItemSize = 0;
#endif

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	m_SortJob = 0;
	m_SortEtc = eCashShopEtcSortType::SORT_BASIC; // Sort criterion.	
#endif // PRE_ADD_CASHSHOP_RENEWAL

}

CDnCashShopTask::~CDnCashShopTask()
{
	SAFE_RELEASE_SPTR( m_hCashShopBGM );
	SAFE_DELETE_PMAP( CS_BASIC_INFO_LIST, m_BasicInfoList );
}

#define MAX_DATE_STR 128

bool CDnCashShopTask::Initialize()
{

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	
	//std::string
	char * strStateName[32] = { "CLOSE", "HOT", "NEW", "EVENT", "DISCOUNT", "LIMITED", "SALE", "COUNTLIMIT" };
	std::map< std::string, int > mapState;
	for( int i=0; i<8; ++i )
		mapState.insert( std::make_pair( std::string(strStateName[i]), i ) );
#endif // PRE_ADD_CASHSHOP_RENEWAL

	m_BasicInfoList.clear();

	DNTableFileFormat* pItemSox = GetDNTable( CDnTableDB::TITEM );
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TCASHLIMIT);
	int iItemID = 0;
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		WCHAR wszStartDate[MAX_DATE_STR];
		WCHAR wszEndDate[MAX_DATE_STR];
		int nTableID		= pSox->GetItemID(iItemID);
		CASHITEM_SN sn		= (CASHITEM_SN)pSox->GetFieldFromLablePtr(nTableID, "_SN")->GetInteger();
		char* szStartDate	= pSox->GetFieldFromLablePtr(nTableID, "_StartDate")->GetString();
		char* szEndDate		= pSox->GetFieldFromLablePtr(nTableID, "_EndDate")->GetString();
		int nBuyAbleCount = pSox->GetFieldFromLablePtr(nTableID, "_BuyAbleCount")->GetInteger();

		MultiByteToWideChar(CP_ACP, 0, szStartDate, -1, wszStartDate, MAX_DATE_STR);
		MultiByteToWideChar(CP_ACP, 0, szEndDate, -1, wszEndDate, MAX_DATE_STR);

		SCashItemLimitInfo limitInfo;
		limitInfo.start = wszStartDate;
		limitInfo.end	= wszEndDate;
		limitInfo.nBuyAbleCount = nBuyAbleCount;

#ifdef PRE_ADD_LIMITED_CASHITEM
		limitInfo.nMaxCount = pSox->GetFieldFromLablePtr( nTableID, "_Sellcount" )->GetInteger();
#endif // PRE_ADD_LIMITED_CASHITEM

		m_CashItemLimitInfo.insert( std::make_pair( sn, limitInfo ) );
	}

	pSox = GetDNTable(CDnTableDB::TCASHPRESENTREACT);
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID	= pSox->GetItemID(iItemID);
		int msgType		= pSox->GetFieldFromLablePtr(nTableID, "_MsgType")->GetInteger();
		int msgId		= pSox->GetFieldFromLablePtr(nTableID, "_MsgID")->GetInteger();

		std::map<int, std::vector<int> >::iterator iter = m_GiftReactInfo.find(msgType);
		if (iter != m_GiftReactInfo.end())
		{
			std::vector<int>& reacts = (*iter).second;
			reacts.push_back(msgId);
		}
		else
		{
			std::vector<int> reacts;
			reacts.push_back(msgId);
			m_GiftReactInfo.insert(std::make_pair(msgType, reacts));
		}
	}

	pSox = GetDNTable( CDnTableDB::TCASHCOMMODITY );
	if (pSox == NULL) return false;
	std::vector<CDnSecretCashShopBuyDlg::SSecretShopItem> vecSecretItems;
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID = pSox->GetItemID(iItemID);

		SCashShopItemInfo* pInfo = new SCashShopItemInfo;

		pInfo->id				= nTableID;
		pInfo->sn				= pSox->GetFieldFromLablePtr(nTableID, "_SN")->GetInteger();
		pInfo->category			= pSox->GetFieldFromLablePtr(nTableID, "_Category")->GetInteger();
		pInfo->subCategory		= pSox->GetFieldFromLablePtr(nTableID, "_SubCategory")->GetInteger();
		pInfo->nameString		= GetEtUIXML().GetUIString(CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr(nTableID, "_NameID")->GetInteger());
		pInfo->descId			= pSox->GetFieldFromLablePtr(nTableID, "_DescriptionID")->GetInteger();

		std::string fieldName;
		int i = 1;

		for (; i <= MAX_ABILITY_ID; ++i)
		{
			fieldName = FormatA("_ItemID%02d", i);
			DNTableCell* pField = pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str());
			if (pField == NULL)
				continue;

			ITEMCLSID id = pField->GetInteger();
			if (id != ITEMCLSID_NONE)
			{
				if (i == 1)
				{
					pInfo->presentItemId = id;
					if (pItemSox->IsExistItem( id ))
					{
						pInfo->levelLimit = pItemSox->GetFieldFromLablePtr( id, "_LevelLimit" )->GetInteger();

						char* szJob = pItemSox->GetFieldFromLablePtr( id, "_NeedJobClass" )->GetString();

						std::vector<std::string> tokens;
						TokenizeA(szJob, tokens, ";");

						bool bProcessed = false;
						if (tokens.size() == 1 && tokens[0] == "0")
						{
							int j = 0;
							for (; j <= CLASSKINDMAX; ++j)
								pInfo->bEnableJob[j] = true;
							bProcessed = true;
						}

						if (bProcessed == false)
						{
							std::vector<std::string>::const_iterator jobIter = tokens.begin();
							for (; jobIter != tokens.end(); ++jobIter)
							{
								const std::string& curJob = (*jobIter);
								int idx = atoi(curJob.c_str());
								if (idx < 0 || idx > CLASSKINDMAX)
									continue;

								pInfo->bEnableJob[idx] = true;
							}

							pInfo->bOnlyOneClassEnable = ((int)tokens.size() == 1 && pInfo->bEnableJob[0] == false);
						}
					}
				}
				pInfo->abilityList.push_back(id);
			}
		}

		for (i = 1; i <= MAX_LINK_SN; ++i)
		{
			fieldName = FormatA("_LinkSN%02d", i);
			CASHITEM_SN linkId = pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str())->GetInteger();
			if (linkId != 0)
				pInfo->linkIdList.push_back(linkId);
		}

		pInfo->period		= pSox->GetFieldFromLablePtr(nTableID, "_Period")->GetInteger();
		pInfo->price		= pSox->GetFieldFromLablePtr(nTableID, "_Price")->GetInteger();
		pInfo->priceFix		= pSox->GetFieldFromLablePtr(nTableID, "_PriceFix")->GetInteger();
		pInfo->count		= pSox->GetFieldFromLablePtr(nTableID, "_Count")->GetInteger();
		pInfo->priority		= pSox->GetFieldFromLablePtr(nTableID, "_Priority")->GetInteger();
		pInfo->bOnSale		= (pSox->GetFieldFromLablePtr(nTableID, "_OnSale")->GetInteger()) ? true : false;
		
#ifdef PRE_ADD_CASHSHOP_RENEWAL
		DNTableCell * pCellState = pSox->GetFieldFromLablePtr(nTableID, "_StateDuplicate");
		if( pCellState )
		{
			std::string strState( pCellState->GetString() );
			if( !strState.empty() )
			{
				std::map< std::string, int >::iterator it;
				std::vector<std::string> tokens;
				TokenizeA( strState, tokens, std::string(";") );
				for( int k=0; k<(int)tokens.size(); ++k )	
				{
					it = mapState.find( tokens[k] );
					if( it != mapState.end() )
						pInfo->arrState.push_back( it->second );
				}
			}
		}	
#else
		pInfo->state		= pSox->GetFieldFromLablePtr(nTableID, "_State")->GetInteger();
#endif // PRE_ADD_CASHSHOP_RENEWAL

		pInfo->bLimit		= (pSox->GetFieldFromLablePtr(nTableID, "_Limit")->GetInteger()) ? true : false;
		{
			DNTableCell *pUseCouponCell = pSox->GetFieldFromLablePtr(nTableID, "_UseCoupon");
			pInfo->bUseCoupon = (pUseCouponCell != NULL) ? (pUseCouponCell->GetInteger() != 0) : true;
		}

		pInfo->bReserveOffer	= (pSox->GetFieldFromLablePtr(nTableID, "_ReserveGive")->GetInteger()) ? true : false;
		pInfo->ReserveAmount	= pSox->GetFieldFromLablePtr(nTableID, "_Reserve")->GetInteger();
		pInfo->bReserveUsable	= (pSox->GetFieldFromLablePtr(nTableID, "_ReserveAble")->GetInteger()) ? true : false;
		pInfo->bGiftUsable		= (pSox->GetFieldFromLablePtr(nTableID, "_PresentAble")->GetInteger()) ? true : false;
		pInfo->type				= (eCashShopItemType)(pSox->GetFieldFromLablePtr(nTableID, "_ItemSort")->GetInteger());
		pInfo->bCartAble		= (pSox->GetFieldFromLablePtr(nTableID, "_CartAble")->GetInteger()) ? true : false;

#ifdef PRE_ADD_VIP
		pInfo->bVIPSell			= (pSox->GetFieldFromLablePtr(nTableID, "_VIPSell")->GetInteger()) ? true : false;
		pInfo->neededVIPLevel	= pSox->GetFieldFromLablePtr(nTableID, "_VIPLevel")->GetInteger();
#endif
		pInfo->nOverlapBuy		= pSox->GetFieldFromLablePtr(nTableID, "_OverlapBuy")->GetInteger();

#ifdef PRE_ADD_CASHSHOP_CREDIT
		pInfo->bCreditAble		= (pSox->GetFieldFromLablePtr(nTableID, "_CreditAble")->GetInteger()) ? true : false;
#endif // PRE_ADD_CASHSHOP_CREDIT

		{
			DNTableCell *pHiddenCell = pSox->GetFieldFromLablePtr(nTableID, "_IsHiddenCashShop");
			if (pHiddenCell && pHiddenCell->GetInteger() > 0 && !pInfo->linkIdList.empty())
			{
				CDnSecretCashShopBuyDlg::SSecretShopItem secretItem;
				secretItem.nSN = pInfo->sn;
				secretItem.nItemID = pInfo->linkIdList[0];
				secretItem.nCount = (pInfo->count > 0) ? pInfo->count : 1;
				DNTableCell *pPeriodTypeCell = pSox->GetFieldFromLablePtr(nTableID, "_PeriodType");
				secretItem.nPeriod = (pPeriodTypeCell && pPeriodTypeCell->GetInteger() > 0) ? pInfo->period : pInfo->period;
				secretItem.nPrice = pInfo->price;
				secretItem.nDiscountRate = (pInfo->priceFix > pInfo->price && pInfo->priceFix > 0) ? ((pInfo->priceFix - pInfo->price) * 100 / pInfo->priceFix) : 0;
				vecSecretItems.push_back(secretItem);
			}
		}

		m_BasicInfoList.insert(std::make_pair(pInfo->sn, pInfo));

		CS_CATEGORY_LIST::iterator iter = m_ItemListByCategory.find((eCashShopSubCatType)pInfo->subCategory);
		if (iter != m_ItemListByCategory.end())
		{
			CS_INFO_LIST& infoList = (*iter).second;
			infoList.push_back(pInfo);
		}
		else
		{
			CS_INFO_LIST infoList;
			infoList.push_back(pInfo);
			m_ItemListByCategory.insert(std::make_pair((eCashShopSubCatType)pInfo->subCategory, infoList));
		}		
	}

	{
		DNTableFileFormat *pHiddenSox = GetDNTable( CDnTableDB::THIDDENCASHSHOPREAL );
		if( pHiddenSox )
		{
			for( int h = 0; h < pHiddenSox->GetItemCount(); ++h )
			{
				int nHiddenID = pHiddenSox->GetItemID( h );
				DNTableCell *pSnCell = pHiddenSox->GetFieldFromLablePtr( nHiddenID, "_SN" );
				if( !pSnCell ) continue;
				int nSN = pSnCell->GetInteger();
				std::map<CASHITEM_SN, SCashShopItemInfo*>::iterator itFound = m_BasicInfoList.find( nSN );
				if( itFound != m_BasicInfoList.end() && itFound->second && !itFound->second->linkIdList.empty() )
				{
					CDnSecretCashShopBuyDlg::SSecretShopItem secretItem;
					secretItem.nSN = nSN;
					secretItem.nItemID = itFound->second->linkIdList[0];
					secretItem.nCount = (itFound->second->count > 0) ? itFound->second->count : 1;
					secretItem.nPeriod = itFound->second->period;
					secretItem.nPrice = itFound->second->price;
					vecSecretItems.push_back( secretItem );
				}
			}
		}
		if( !vecSecretItems.empty() )
		{
			if( GetInterface().GetSecretCashShopBuyDlg() )
				GetInterface().GetSecretCashShopBuyDlg()->SetSecretItems( vecSecretItems );
			if( GetInterface().GetSecretCashShopAlarmBtnDlg() )
				GetInterface().GetSecretCashShopAlarmBtnDlg()->SetAlarmInfo( L"Secret Shop", NULL );
		}
	}

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	// Add search category - add searched items to category 9999.
	CS_INFO_LIST searchList;	
	m_ItemListByCategory.insert(std::make_pair((eCashShopSubCatType)SUBCATEGORY_SERACH, searchList));
#endif // PRE_ADD_CASHSHOP_RENEWAL

	pSox = GetDNTable(CDnTableDB::TCASHGOODSINFO);
	if (pSox == NULL) return false;
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID	= pSox->GetItemID(iItemID);
		ITEMCLSID id	= pSox->GetFieldFromLablePtr(nTableID, "_ItemID")->GetInteger();

		int	UIStringId	= pSox->GetFieldFromLablePtr(nTableID, "_StringID")->GetInteger();

		m_AbilityDescInfoList.insert(std::make_pair(id, UIStringId));
	}

	pSox = GetDNTable(CDnTableDB::TCASHPACKAGE);
	if (pSox == NULL) return false;

#ifdef _FINAL_BUILD
#else
	std::vector<CASHITEM_SN> dupCheckList;
	std::vector<CASHITEM_SN> noContentsPackageCheckList;
#endif
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID = pSox->GetItemID(iItemID);

		std::vector<CASHITEM_SN> inPackageItems;
		CASHITEM_SN packageSN = pSox->GetFieldFromLablePtr(nTableID, "_SN")->GetInteger();

#ifdef _FINAL_BUILD
#else
		std::map<CASHITEM_SN, std::vector<CASHITEM_SN> >::const_iterator dupIter = m_CashItemPackageInfo.find(packageSN);
		if (dupIter != m_CashItemPackageInfo.end())
			dupCheckList.push_back(packageSN);
#endif

		std::string fieldName;
		int i = 0;
		for (; i < MAX_PACKAGE_GOODS_ID; ++i)
		{
			fieldName = FormatA("_CommodityID%02d", i);
			CASHITEM_SN sn = pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str())->GetInteger();
			if (sn > 0)
				inPackageItems.push_back(sn);
		}

#ifdef _FINAL_BUILD
#else
		if (packageSN > 0 && inPackageItems.empty())
			noContentsPackageCheckList.push_back(packageSN);
#endif

		m_CashItemPackageInfo.insert(std::make_pair(packageSN, inPackageItems));
	}

#ifdef _FINAL_BUILD
#else
	if (dupCheckList.empty() == false)
	{
		std::wstring str;
		str = L"[TABLE DATA ERROR] CashPackageTable.ext : Duplicate SN\n\n";

		std::vector<CASHITEM_SN>::const_iterator dupIter = dupCheckList.begin();
		for (; dupIter != dupCheckList.end(); ++dupIter)
		{
			const CASHITEM_SN& sn = *dupIter;
			const SCashShopItemInfo* pInfo = GetItemInfo(sn);
			if (pInfo)
				str += FormatW(L"%s SN:%d\n", pInfo->nameString.c_str(), sn);
			else
				str += FormatW(L"SN:%d\n", sn);
		}
		::MessageBox( CDnMainFrame::GetInstance().GetHWnd(), str.c_str(), _T("Error"), MB_OK );
		return false;
	}

	if (noContentsPackageCheckList.empty() == false)
	{
		std::wstring str;
		str = L"[TABLE DATA ERROR] CashPackageTable.ext : No contents of the package item\n\n";

		std::vector<CASHITEM_SN>::const_iterator contentIter = noContentsPackageCheckList.begin();
		for (; contentIter != noContentsPackageCheckList.end(); ++contentIter)
		{
			const CASHITEM_SN& sn = *contentIter;
			const SCashShopItemInfo* pInfo = GetItemInfo(sn);
			if (pInfo)
				str += FormatW(L"%s SN:%d\n", pInfo->nameString.c_str(), sn);
			else
				str += FormatW(L"SN:%d\n", sn);
		}

		::MessageBox( CDnMainFrame::GetInstance().GetHWnd(), str.c_str(), _T("Error"), MB_OK );
		return false;
	}
#endif

	pSox = GetDNTable(CDnTableDB::TREBIRTHCOIN);
	if (pSox == NULL) return false;
	m_CashCoinRestriction = pSox->GetFieldFromLablePtr(1, "_CashCoinLimit")->GetInteger(); // References itemid 1 because designers decided cash coin possession limits don't need to vary by level. Update to loop over itemid if level-based limits are added later.

	pSox = GetDNTable(CDnTableDB::TCASHTAB);
	if (pSox == NULL) return false;
	for (iItemID = 0; iItemID < pSox->GetItemCount(); ++iItemID)
	{
		int nTableID = pSox->GetItemID(iItemID);

		std::map<int, SCashShopTabShowInfo> subTabShowList;
		SCashShopMainTabShowInfo mainTabInfo;
		mainTabInfo.showType = (eCashShopTabShowType)pSox->GetFieldFromLablePtr(nTableID, "_MTShow")->GetInteger();
		if (mainTabInfo.showType != eCST_HIDE)
		{
			int mainTab				= pSox->GetFieldFromLablePtr(nTableID, "_MTNum")->GetInteger();
			mainTabInfo.uiStringNum = pSox->GetFieldFromLablePtr(nTableID, "_MTID")->GetInteger();
			mainTabInfo.option		= pSox->GetFieldFromLablePtr(nTableID, "_MTType")->GetInteger();
			m_CashMainTabShowInfo.insert(std::make_pair(mainTab, mainTabInfo));

			// #54411 
#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
			std::map<int, SCashShopSubTabShowInfo>	subTabInfo;

			subTabInfo[ mainTab ].showType				= mainTabInfo.showType;
			subTabInfo[ mainTab ].uiStringNum			= mainTabInfo.uiStringNum ;
			CString		szColorInfo						= pSox->GetFieldFromLablePtr( nTableID, "_STColorSelect")->GetString();
			CString		szDeColorInfo					= pSox->GetFieldFromLablePtr( nTableID, "_STColorDeselect")->GetString();
			CString		szColorElement;

			int nPos = 0;
			if( ! szColorInfo.IsEmpty() ) {
				// The issue with ComposeStringSetChar is when there are duplicate indices
				// Color value is always applied to the trailing index. (No duplicate handling) 
				// If both 0:r and 0:g exist, 0:g is applied 
				while( ComposeStringSetChar( szColorElement, szColorInfo, ';' ) )
				{	
					if( szColorElement.IsEmpty() )
						continue;

					subTabInfo[nPos].szSelectedColor = szColorElement;
					nPos++;
				}
			}

			nPos = 0;

			if( ! szDeColorInfo.IsEmpty() )	{
				while( ComposeStringSetChar( szColorElement, szDeColorInfo, ';' ) )
				{
					if( szColorElement.IsEmpty() )
						continue;

					subTabInfo[nPos].szDeSelectedColor = szColorElement;
					nPos++;
				}
			}

			m_CashSubTabCatShowInfo.insert( std::make_pair( mainTab, subTabInfo) );		

#endif 

			int i = 0;
			std::string fieldName;
			for (; i < MAX_SUB_CATEGORY_PER_PAGE; ++i)
			{
				fieldName = FormatA("_STShow%d", i);
				SCashShopTabShowInfo subTabInfo;
				subTabInfo.showType = (eCashShopTabShowType)pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str())->GetInteger();
				if (subTabInfo.showType != eCST_HIDE)
				{
					fieldName = FormatA("_STNum%d", i);
					int subTab				= pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str())->GetInteger();

					fieldName = FormatA("_ST%dID", i);
					subTabInfo.uiStringNum	= pSox->GetFieldFromLablePtr(nTableID, fieldName.c_str())->GetInteger();
					subTabShowList.insert(std::make_pair(subTab, subTabInfo));
				}
			}
		

			if (subTabShowList.empty() == false)
				m_CashSubTabShowInfo.insert(std::make_pair(mainTab, subTabShowList));
		}
	}

#ifdef PRE_MOD_BROWSER
#else
	InitializeChargeBrowser();
	m_bChargeBrowserOpened = false;
	m_bGeneralBrowserOpened = false;
#endif

#ifdef PRE_ADD_CASHSHOP_RENEWAL
	mapState.clear();
#endif // PRE_ADD_CASHSHOP_RENEWAL

	return true;
}

#ifdef PRE_MOD_BROWSER
#else
	#ifdef PRE_FIX_NOMOVE_BROWSER
	LRESULT CALLBACK ChildWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_WINDOWPOSCHANGING:
			{
				((WINDOWPOS*)lParam)->flags |= SWP_NOMOVE;
			}
			break;
		}

		return CallWindowProc(g_ChildWndProc, hWnd, message, wParam, lParam);
	}
	#endif

LRESULT CDnCashShopTask::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch(message)
	{
	case WM_KEYUP:
		{
			if (m_bChargeBrowserOpened && pChargeWebBrowser)
			{
				CComQIPtr<IOleInPlaceActiveObject, &IID_IOleInPlaceActiveObject> pIOIPAO(pChargeWebBrowser) ;

				if(pIOIPAO)
				{
					MSG msg;
					msg.message = message;
					msg.wParam = wParam;
					msg.lParam = lParam;

					pIOIPAO->TranslateAccelerator(&msg);
				}
			}
		}
		break;
	}

#ifdef PRE_FIX_CASHSHOP_CHARGE_WNDPROC
	if (m_bChargeBrowserOpened && pChargeWebBrowser && IsWindow(g_hwndChargeBrowser))
	{
		READYSTATE state;
		pChargeWebBrowser->get_ReadyState(&state);
		if (state == READYSTATE_LOADED && GetForegroundWindow() == CDnMainFrame::GetInstance().GetHWnd())
			return DefWindowProc( g_hwndChargeBrowser, message, wParam, lParam );
		else
			return 0;
	}
	else
		return 0;
#else
	return DefWindowProc( g_hwndChargeBrowser, message, wParam, lParam );
#endif
}
#endif // PRE_MOD_BROWSER

#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
bool CDnCashShopTask::ComposeStringSetChar( CString& pString, CString& InStr, char ch )
{
	if( InStr.IsEmpty() || ch == 0 || InStr.Find( ch ) < 0 ) 
		return FALSE;

	int	nPos = InStr.Find( ch );

	pString = InStr;  
	pString.GetBufferSetLength( nPos );

	// nPos + 1 to also remove up to ';'  
	InStr.Delete( 0, nPos + 1 );

	return TRUE;
}	
#endif

#ifdef PRE_MOD_BROWSER
#else
void CDnCashShopTask::InitializeChargeBrowser()
{
	_Module.Init(ObjectMap, CDnMainFrame::GetInstance().GetHInst());
}
#endif

void CDnCashShopTask::GetChargeAddress(std::string& addressString)
{
	addressString.clear();

#if defined(_WORK)

	addressString = "http://cash.happyoz.com";

#else

	_ASSERT(0);
	m_pCashShopDlg->HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER, false);
	return;

#endif
}

#ifdef PRE_MOD_BROWSER
#else
void CDnCashShopTask::OnExceptionalCloseChargeBrowser(bool onInitOLE)
{
	if (pChargeWebBrowser)
	{
		pChargeWebBrowser->Quit();
		pChargeWebBrowser.Release();
	}

	if (onInitOLE)
		OleUninitialize();
}
#endif

void CDnCashShopTask::OpenChargeBrowser()
{
	if (IsCashShopMode(eCSMODE_CHARGE_REQ_SERVERTIME))
	{
		m_ChargeTime = 0;
		RequestCashShopGetChargeTime();
		return;
	}

	DoOpenChargeBrowser();
}


//	todo by kalliste : Consider commander pattern if charge types diversify and service regions diverge further.

#ifdef PRE_MOD_BROWSER
#else
void CDnCashShopTask::DoOpenChargeBrowser()
{
	if (m_ChargeBrowserType != eCCBT_FULL_BROWSER && m_ChargeBrowserType != eCCBT_INGAME_BROWSER)
	{
		HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER, false);
		return;
	}

	if (m_bChargeBrowserOpened || IsWindow(g_hwndChargeBrowser))
		return;

	if (SUCCEEDED(OleInitialize(NULL)))
	{
		HRESULT hr;

		if (pIESink)
			pIESink = NULL;

		if (pChargeWebBrowser == NULL)
		{
			if (m_ChargeBrowserType == eCCBT_FULL_BROWSER)
			{
				hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_IWebBrowser2, (LPVOID*)&pChargeWebBrowser);
				if(FAILED(hr) || pChargeWebBrowser == NULL)
				{
					_ASSERT(0);
					HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_COCREATE_FAIL, true);
					OnExceptionalCloseChargeBrowser(true);
					return;
				}

				SHANDLE_PTR hBrowserWnd = 0;
				pChargeWebBrowser->get_HWND(&hBrowserWnd);
				g_hwndChargeBrowser = reinterpret_cast<HWND>(hBrowserWnd);
			}
			else if (m_ChargeBrowserType == eCCBT_INGAME_BROWSER)
			{
				AtlAxWinInit();

				RECT mainRect;
				if (CGameOption::GetInstance().m_bWindow)
				{
					GetWindowRect(CDnMainFrame::GetInstance().GetHWnd(), &mainRect);
					mainRect.left += 300;
					mainRect.top += 50;
				}
				else
				{
					mainRect.left = 300;
					mainRect.top = 50;
				}

				if (m_ChargeBrowserWidth <= 0 || m_ChargeBrowserHeight <= 0)
				{
					_ASSERT(0);
					HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_SIZE_FAIL, true);
					OnExceptionalCloseChargeBrowser(true);
					return;
				}

				g_hwndChargeBrowser = CreateWindow(L"AtlAxWin80", L"Shell.Explorer.2", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER, mainRect.left, mainRect.top, m_ChargeBrowserWidth, m_ChargeBrowserHeight, 
					CDnMainFrame::GetInstance().GetHWnd(), (HMENU)0, CDnMainFrame::GetInstance().GetHInst(), NULL);
#ifdef PRE_FIX_NOMOVE_BROWSER
				g_ChildWndProc = (WNDPROC)SetWindowLongPtr(g_hwndChargeBrowser, GWLP_WNDPROC, (LONG_PTR)ChildWndProc);
#endif

				OutputDebug("[CASHSHOP] charge browser OPEN - left:%ld top:%ld width:%d height:%d\n", mainRect.left, mainRect.top, m_ChargeBrowserWidth, m_ChargeBrowserHeight);

				CComPtr<IUnknown> punkIE;
				if (AtlAxGetControl(g_hwndChargeBrowser, &punkIE) == S_OK)
				{
					pChargeWebBrowser = punkIE;

					if (pIESink == NULL)
					{
						hr = CComObject<CDnCashShopIESink>::CreateInstance(&pIESink);
						if(FAILED(hr))
						{
							_ASSERT(0);
							HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_ATTACH_IE_EVENT_FAIL, true);
							OnExceptionalCloseChargeBrowser(true);
							return;
						}
						pIESink->SetTask(this);
					}

					AtlGetObjectSourceInterface(punkIE, &(pIESink->m_libid), &(pIESink->m_iid), &(pIESink->m_wMajorVerNum), &(pIESink->m_wMinorVerNum));
					hr = pIESink->DispEventAdvise(pChargeWebBrowser, &(pIESink->m_iid));

					if (FAILED(hr))
					{
						_ASSERT(0);
						HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_ATTACH_IE_EVENT_FAIL, true);
						OnExceptionalCloseChargeBrowser(true);
						return;
					}
				}
			}
			else
			{
				_ASSERT(0);
				HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_TYPE_FAIL, true);
				OnExceptionalCloseChargeBrowser(true);
				return;
			}
		}
		else
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER, true);
			OnExceptionalCloseChargeBrowser(true);
			return;
		}

		if (m_ChargeBrowserType == eCCBT_INGAME_BROWSER)
		{
			if (CGameOption::IsActive() && CGameOption::GetInstance().m_bWindow == false && GetEtDevice())
			{
				LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr();
				if (pDevice == NULL)
				{
					m_pCashShopDlg->HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_D3D, true);
					OnExceptionalCloseChargeBrowser(true);
					return;
				}

				pDevice->SetDialogBoxMode(TRUE);
				m_bFullScreenMode = true;
			}
			else
			{
				m_bFullScreenMode = false;
			}
		}

		std::string url;
		GetChargeAddress(url);
		if (url.empty())
		{
			OnExceptionalCloseChargeBrowser(true);
			return;
		}

		CComVariant vUrl(url.c_str()), vEmpty;

		VARIANT vFlags;
		V_VT(&vFlags) = VT_I4;
		V_I4(&vFlags) = navNoReadFromCache;

		hr = pChargeWebBrowser->Navigate2(&vUrl, &vFlags, &vEmpty, &vEmpty, &vEmpty);
		if (SUCCEEDED(hr))
		{
			pChargeWebBrowser->put_Visible(VARIANT_TRUE);
			ShowWindow((HWND)g_hwndChargeBrowser, SW_SHOW);
			if (m_ChargeBrowserType == eCCBT_FULL_BROWSER)
			{
				SetForegroundWindow((HWND)g_hwndChargeBrowser);
				SetActiveWindow((HWND)g_hwndChargeBrowser);
			}

			InvalidateRect(g_hwndChargeBrowser, NULL, true);

			m_bChargeBrowserOpened = true;
			m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4773 ));	// UISTRING : Processing Cash charge.
		}
		else
		{
			HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_NAVIGATE_FAIL, true);
			OnExceptionalCloseChargeBrowser(true);
			return;
		}

		OleUninitialize();
	}
}

void CDnCashShopTask::OpenBrowser(const std::string& url, const SUICoord& coord, eGeneralBrowserType type, const std::wstring& msgBoxString)
{
	if (m_bChargeBrowserOpened || IsWindow(g_hwndChargeBrowser) || m_bGeneralBrowserOpened)
		return;

	if (SUCCEEDED(OleInitialize(NULL)))
	{
		HRESULT hr;

		if (pIESink)
			pIESink = NULL;

		if (pChargeWebBrowser == NULL)
		{
			AtlAxWinInit();

			RECT mainRect;
			SUICoord correctCoord = coord;
			if (CGameOption::GetInstance().m_bWindow)
			{
				GetWindowRect(CDnMainFrame::GetInstance().GetHWnd(), &mainRect);
				correctCoord.fX = mainRect.left + coord.fX;
				correctCoord.fY = mainRect.top + coord.fY;
			}

			g_hwndChargeBrowser = CreateWindow(L"AtlAxWin80", L"Shell.Explorer.2", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_BORDER, (int)correctCoord.fX, (int)correctCoord.fY, (int)correctCoord.fWidth, (int)correctCoord.fHeight, 
				CDnMainFrame::GetInstance().GetHWnd(), (HMENU)0, CDnMainFrame::GetInstance().GetHInst(), NULL);
#ifdef PRE_FIX_NOMOVE_BROWSER
			g_ChildWndProc = (WNDPROC)SetWindowLongPtr(g_hwndChargeBrowser, GWLP_WNDPROC, (LONG_PTR)ChildWndProc);
#endif

			CComPtr<IUnknown> punkIE;
			if (AtlAxGetControl(g_hwndChargeBrowser, &punkIE) == S_OK)
			{
				pChargeWebBrowser = punkIE;

				if (pIESink == NULL)
				{
					hr = CComObject<CDnCashShopIESink>::CreateInstance(&pIESink);
					if(FAILED(hr))
						_ASSERT(0);
					pIESink->SetTask(this);
				}

				AtlGetObjectSourceInterface(punkIE, &(pIESink->m_libid), &(pIESink->m_iid), &(pIESink->m_wMajorVerNum), &(pIESink->m_wMinorVerNum));
				hr = pIESink->DispEventAdvise(pChargeWebBrowser, &(pIESink->m_iid));
				pIESink->SetCashShopSinkType(CDnCashShopIESink::eGENERAL);

				if (FAILED(hr))
					_ASSERT(0);
			}
		}
		else
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER, true);
			return;
		}

		if (CGameOption::IsActive() && CGameOption::GetInstance().m_bWindow == false && GetEtDevice())
		{
			LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr();
			if (pDevice == NULL)
			{
				m_pCashShopDlg->HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_D3D, true);
				return;
			}

			pDevice->SetDialogBoxMode(TRUE);
			m_bFullScreenMode = true;
		}
		else
		{
			m_bFullScreenMode = false;
		}

		CComVariant vUrl(url.c_str()), vEmpty;
		hr = pChargeWebBrowser->Navigate2(&vUrl, &vEmpty, &vEmpty, &vEmpty, &vEmpty);
		if (SUCCEEDED(hr))
		{
			pChargeWebBrowser->put_Visible(VARIANT_TRUE);
			ShowWindow((HWND)g_hwndChargeBrowser, SW_SHOW);
			InvalidateRect(g_hwndChargeBrowser, NULL, true);

			m_bGeneralBrowserOpened = true;
			m_GeneralBrowserOpenedType = type;

			m_pCashShopDlg->DisableAllDlgs(true, msgBoxString);
		}

		OleUninitialize();
	}
}
#endif // PRE_MOD_BROWSER

void CDnCashShopTask::CloseBrowser()
{
	DestroyWindow(g_hwndChargeBrowser);
}

void CDnCashShopTask::CloseChargeBrowser()
{
	DestroyWindow(g_hwndChargeBrowser);
}

void CDnCashShopTask::Finalize()
{

}

#ifdef PRE_MOD_BROWSER
#else
void CDnCashShopTask::ProcessGeneralBrowser()
{
	if (m_bGeneralBrowserOpened)
	{
		BOOL bIsActivate = IsWindow(g_hwndChargeBrowser);

		if (bIsActivate == false)
		{
			m_bGeneralBrowserOpened = false;
			m_pCashShopDlg->DisableAllDlgs(false, L"");

			pChargeWebBrowser->Quit();
			pChargeWebBrowser.Release();

			CDnInterface::GetInstance().OnCloseBrowser(m_GeneralBrowserOpenedType);

			if (CGameOption::IsActive() && CGameOption::GetInstance().m_bWindow == false && GetEtDevice())
			{
				LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr();
				if (pDevice == NULL)
					return;

				pDevice->SetDialogBoxMode(FALSE);
			}
		}
	}
}
#endif // PRE_MOD_BROWSER

void CDnCashShopTask::Process(LOCAL_TIME LocalTime, float fDelta)
{
	if (m_bSoundOnOpen)
	{
		m_fSoundDelta += fDelta;
		if (m_fSoundDelta > CGlobalInfo::GetInstance().m_fFadeDelta)
		{
			if (CDnWorld::GetInstance().IsActive())
				CDnWorld::GetInstance().StopBGM();

			SAFE_RELEASE_SPTR( m_hCashShopBGM );
			if( m_CashShopBGMIndex > -1 ) {
				CEtSoundEngine::GetInstance().RemoveSound( m_CashShopBGMIndex );
				m_CashShopBGMIndex = -1;
			}

			if (m_CashShopBGMIndex == -1)
				m_CashShopBGMIndex = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( "DRAGON NEST_BGM_Cashshop.ogg" ).c_str(), false, true );
			m_hCashShopBGM = CEtSoundEngine::GetInstance().PlaySound__("BGM", m_CashShopBGMIndex, true, true);

			if (m_hCashShopBGM)
			{
				m_hCashShopBGM->SetVolume( CEtSoundEngine::GetInstance().GetMasterVolume( "BGM" ) );
				m_hCashShopBGM->Resume();
			}

			CEtSoundEngine::GetInstance().FadeVolume("BGM", 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false);
			CEtSoundEngine::GetInstance().FadeVolume("2D", 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false);

			m_bSoundOnOpen = false;
			m_fSoundDelta = 0.f;
		}
	}
	else if (m_bSoundOnClose)
	{
		m_fSoundDelta += fDelta;
		if (m_fSoundDelta > CGlobalInfo::GetInstance().m_fFadeDelta)
		{
			if( m_CashShopBGMIndex > -1 )
			{
				SAFE_RELEASE_SPTR(m_hCashShopBGM);
				CEtSoundEngine::GetInstance().RemoveSound( m_CashShopBGMIndex );
				m_CashShopBGMIndex = -1;
			}

			m_bSoundOnClose = false;
			m_fSoundDelta = 0.f;
		}
	}

	if (m_ChargeBrowserType == eCCBT_FULL_BROWSER)
	{
		if (m_bChargeBrowserOpened && pChargeWebBrowser)
		{
			BOOL bIsActivate = IsWindow(g_hwndChargeBrowser);

			READYSTATE state;
			pChargeWebBrowser->get_ReadyState(&state);
			if (bIsActivate == false || (state == READYSTATE_COMPLETE && GetForegroundWindow() == CDnMainFrame::GetInstance().GetHWnd()))
			{
				if (CGameOption::GetInstance().m_bWindow == false)
					ShowWindow(CDnMainFrame::GetInstance().GetHWnd(), SW_MAXIMIZE);
				SendCashShopBalanceInquiry();

				m_bChargeBrowserOpened = false;
				m_pCashShopDlg->DisableAllDlgs(false, L"");

				pChargeWebBrowser->Quit();
				pChargeWebBrowser.Release();

				CloseChargeBrowser();
			}
		}
	}
	else if (m_ChargeBrowserType == eCCBT_INGAME_BROWSER)
	{
		if (m_bChargeBrowserOpened && g_hwndChargeBrowser)
		{
			BOOL bIsActivate = IsWindow(g_hwndChargeBrowser);

			if (bIsActivate == false)
			{
				SendCashShopBalanceInquiry();

				m_bChargeBrowserOpened = false;
				m_pCashShopDlg->DisableAllDlgs(false, L"");

				pChargeWebBrowser->Quit();
				pChargeWebBrowser.Release();

				if (CGameOption::IsActive() && CGameOption::GetInstance().m_bWindow == false && GetEtDevice())
				{
					LPDIRECT3DDEVICE9 pDevice = (LPDIRECT3DDEVICE9)GetEtDevice()->GetDevicePtr();
					if (pDevice == NULL)
						return;

					pDevice->SetDialogBoxMode(FALSE);
				}

				AtlAxWinTerm();
			}
		}
	}


	ProcessGeneralBrowser();

	if (m_bBuying)
	{
		m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4774 )); // UISTRING : Processing purchase
		m_bBuying = false;
	}
}

const CS_INFO_LIST* CDnCashShopTask::GetSubCatItemList(eCashShopSubCatType type) const
{
	CS_CATEGORY_LIST::const_iterator iter = m_ItemListByCategory.find(type);
	if (iter != m_ItemListByCategory.end())
	{
		const CS_INFO_LIST& infoList = (*iter).second;
		return &infoList;
	}

	return NULL;
}

CS_INFO_LIST* CDnCashShopTask::GetSubCatItemList(eCashShopSubCatType type)
{
	CS_CATEGORY_LIST::iterator iter = m_ItemListByCategory.find(type);
	if (iter != m_ItemListByCategory.end())
	{
		CS_INFO_LIST& infoList = (*iter).second;
		return &infoList;
	}

	return NULL;
}

const SCashShopItemInfo* CDnCashShopTask::GetSubCatItem(eCashShopSubCatType type, int index) const
{
	CS_CATEGORY_LIST::const_iterator iter = m_ItemListByCategory.find(type);
	if (iter != m_ItemListByCategory.end())
	{
		const CS_INFO_LIST& infoList = (*iter).second;
		CS_INFO_LIST::const_iterator iter = infoList.begin();
		for (; iter != infoList.end(); ++iter)
		{
			SCashShopItemInfo* pInfo = *iter;
			if (pInfo)
			{
				if (pInfo->id == index)
					return pInfo;
			}
		}
	}

	return NULL;
}

int	CDnCashShopTask::GetSubCatItemListSize(eCashShopSubCatType type) const
{
	CS_CATEGORY_LIST::const_iterator iter = m_ItemListByCategory.find(type);
	if (iter != m_ItemListByCategory.end())
	{
		const CS_INFO_LIST& infoList = (*iter).second;
		return int(infoList.size());
	}

	return -1;
}

void CDnCashShopTask::OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize )
{
	bool bProcessDispatch = false;
	switch (nMainCmd)
	{
	case SC_CASHSHOP:
		{
			OnRecvCashShopMessage(nSubCmd, pData, nSize);
			bProcessDispatch = true;
			break;
		}
	}

	if (bProcessDispatch)
	{
		WriteLog( 1, ", Info, CDnCashShopTask::OnDispatchMessage End : nMainCmd(%d), nSubCmd(%d), nSize(%d)\n", nMainCmd, nSubCmd, nSize );
	}
}

void CDnCashShopTask::OnRecvCashShopMessage( int nSubCmd, char *pData, int nSize )
{
	switch (nSubCmd)
	{
	case eCashShop::eSCCashShop::SC_SHOPOPEN:		OnRecvCashShopOpen((SCCashShopOpen*)pData); break;
	case eCashShop::eSCCashShop::SC_SHOPCLOSE:		OnRecvCashShopClose((SCCashShopClose*)pData); break;
	case eCashShop::eSCCashShop::SC_BUY:			OnRecvCashShopBuy((SCCashShopBuy*)pData); break;	
	case eCashShop::eSCCashShop::SC_PACKAGEBUY:		OnRecvCashShopPackageBuy((SCCashShopPackageBuy*)pData); break;
	case eCashShop::eSCCashShop::SC_BALANCEINQUIRY:	OnRecvCashShopBalanceInquiry((SCCashShopBalanceInquiry*)pData); break;
	case eCashShop::eSCCashShop::SC_CHECKRECEIVER:	OnRecvCashShopGiftReceiverInfo((SCCashShopCheckReceiver*)pData); break;
	case eCashShop::eSCCashShop::SC_GIFTLIST:		OnRecvCashShopRecvGiftBasicInfo((SCCashShopGiftList*)pData); break;
	case eCashShop::eSCCashShop::SC_RECEIVEGIFT:	OnRecvCashShopRecvGift((SCCashShopReceiveGift*) pData); break;
	case eCashShop::eSCCashShop::SC_RECEIVEGIFTALL:	OnRecvCashShopRecvGiftAll((SCCashShopReceiveGiftAll*) pData); break;
	case eCashShop::eSCCashShop::SC_COUPON:			OnRecvCashShopRecvItemByCoupon((SCCashShopCoupon*)pData); break;
	case eCashShop::eSCCashShop::SC_GIFT:			OnRecvCashShopSendGift((SCCashShopGift*)pData); break;
	case eCashShop::eSCCashShop::SC_PACKAGEGIFT:	OnRecvCashShopSendPackageGift((SCCashShopPackageGift*) pData); break;
	case eCashShop::eSCCashShop::SC_NOTIFYGIFT:		OnRecvGiftNotify((SCNotifyGift*)pData); break;
#ifdef PRE_ADD_VIP
	case eCashShop::eSCCashShop::SC_VIPBUY:			OnRecvCashShopVIPBuy((SCVIPBuy*)pData); break;
	case eCashShop::eSCCashShop::SC_VIPGIFT:		OnRecvCashShopVIPGift((SCVIPGift*)pData); break;
	case eCashShop::eSCCashShop::SC_VIPINFO:		OnRecvCashShopVIPInfo((SCVIPInfo*)pData); break;
#endif // PRE_ADD_VIP
	case eCashShop::eSCCashShop::SC_SALEABORTLIST:	OnRecvSaleAbortList((SCSaleAbortList*)pData); break;
#if defined(PRE_ADD_GIFT_RETURN)
	case eCashShop::eSCCashShop::SC_GIFTRETURN :	OnRecvCashShopGiftReturn((SCCashShopGiftReturn*)pData); break;
#endif
	case eCashShop::eSCCashShop::SC_CHARGE_TIME:			OnRecvCashShopServerTimeForCharge((SCCashShopChargeTime*)pData); break;
#ifdef PRE_ADD_LIMITED_CASHITEM
	case eCashShop::eSCCashShop::SC_LIMITEDCASHITEM:		OnRecvCashShopLimitedItemInfo((SCQuantityLimitedItem*)pData); break;
	case eCashShop::eSCCashShop::SC_CHANGEDLIMITEDCASHITEM: OnRecvCashShopChangedLimitedItemInfo((SCChangedQuantityLimitedItem*)pData); break;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
#ifdef PRE_ADD_CADGE_CASH
	case eCashShop::eSCCashShop::SC_CADGE:					OnRecvCashShopCadge( (SCCashShopCadge*)pData ); break;
#endif // PRE_ADD_CADGE_CASH
	case eCashShop::eSCCashShop::SC_SECRETCASHSHOP_NOTIFY:	OnRecvSecretCashShopNotify( (SCSecretCashShopNotify*)pData ); break;
	case eCashShop::eSCCashShop::SC_SECRETCASHSHOP_OPEN:	OnRecvSecretCashShopOpen( (SCSecretCashShopOpen*)pData ); break;
	default:
		CDebugSet::ToLogFile( "CDnCashShopTask::OnRecvCashShopMessage, Invalid Message(%d)!", nSubCmd );
	}
}

void CDnCashShopTask::TerminateCashShop()
{
	m_bOpened = false;
}

void CDnCashShopTask::OnRecvSecretCashShopNotify(SCSecretCashShopNotify* pData)
{
	if( !pData || pData->nHiddenShopID <= 0 )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::THIDDENCASHSHOPREAL );
	if( !pSox || !pSox->IsExistItem( pData->nHiddenShopID ) )
		return;

	if( CDnInterface::IsActive() && GetInterface().GetSecretCashShopAlarmBtnDlg() )
	{
		GetInterface().GetSecretCashShopAlarmBtnDlg()->SetHiddenShop( pData->nHiddenShopID );
		GetInterface().GetSecretCashShopAlarmBtnDlg()->Show( true );
	}
}

void CDnCashShopTask::OnRecvSecretCashShopOpen(SCSecretCashShopOpen* pData)
{
	if( !pData )
		return;

	if( pData->nRet == ERROR_NONE )
	{
		if( CDnInterface::IsActive() && GetInterface().GetSecretCashShopBuyDlg() )
		{
			GetInterface().GetSecretCashShopBuyDlg()->SetHiddenShop( pData->nHiddenShopID );
			GetInterface().GetSecretCashShopBuyDlg()->Show( true );
		}
	}
	else if( CDnInterface::IsActive() )
	{
		GetInterface().ServerMessageBox( pData->nRet );
	}
}

void CDnCashShopTask::RequestCashShopOpen()
{
#ifdef PRE_REMOVE_CASHSHOP
	return;
#else
	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());
	if (CDnPartyTask::GetInstance().IsLocalActorEnterGateReady() || (pPlayer && pPlayer->IsLockInput()) || m_bRequestingCashShopOpen)
		return;
	if( CDnBridgeTask::IsActive() ) {
		switch( CDnBridgeTask::GetInstance().GetBridgeState() ) {
			case eBridgeState::LoginToVillage:
			case eBridgeState::GameToVillage:
			case eBridgeState::VillageToVillage:
				break;
			default: return;
		}
	}

#ifdef PRE_MOD_SYSTEM_STATE
	if (GetInterface().GetSystemStateMgr().StartState(eSTATE_CASHSHOP) == false)
	{
		//	todo by kalliste : Error Handling
		return;
	}
#endif

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
	CDnCommonTask *pCommonTask = static_cast<CDnCommonTask *>(CTaskManager::GetInstance().GetTask("CommonTask"));
	if (pCommonTask)
	{
		if (pCommonTask->IsRequestingQuickPvP())
			return;
	}
#endif

	if( CDnTradeTask::IsActive() ) GetTradeTask().GetTradePrivateMarket().ClearTradeUserInfoList();
#ifdef PRE_MOD_INTEG_SYSTEM_STATE
	if( CDnPartyTask::IsActive() ) GetPartyTask().ClearInviteInfoList(true);
#else
	if( CDnPartyTask::IsActive() ) GetPartyTask().ClearInviteInfoList();
#endif
	if( CDnMasterTask::IsActive() ) GetMasterTask().RejectRecall();
	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );


	m_bOpened = m_bRequestingCashShopOpen = true;

#ifdef _TEST_CASHSHOP_CLIENT_OPEN
	SCCashShopOpen openPacket;
	openPacket.nRet = ERROR_NONE;
	openPacket.nCashAmount = 100000;
	openPacket.nReserveAmount = 1000000;
	wcscpy_s(openPacket.wszAccountName, IDLENMAX, L"ABCDEFG");
	OnRecvCashShopOpen(&openPacket);
#else
	SendCashShopOpen();
#endif // _TEST_CASHSHOP_CLIENT_OPEN
#endif
}

void CDnCashShopTask::RequestCashShopClose()
{
	CEtSoundEngine::GetInstance().FadeVolume( NULL, 0.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );
	m_bSoundOnClose = true;

	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4775 ), 2.f);	// UISTRING : Closing Cash Shop.

#ifdef _TEST_CASHSHOP_CLIENT_OPEN
	SCCashShopClose closePacket;
	closePacket.nRet = ERROR_NONE;
	OnRecvCashShopClose(&closePacket);
#else
	SendCashShopClose();
#endif
}

void CDnCashShopTask::OnOpenCashShop()
{
	GetMasterTask().CloseAcceptDlg();

#ifdef PRE_FIX_QUICKPVP_EXCLUSIVEREQ
	GetInterface().ClearAcceptReqDlg(true);
#endif
	GetInterface().ShowCashShopDlg(true);

	m_PreviewCartClassId = CDnActor::s_hLocalActor->GetClassID();
	memset( &m_PackagePreviewUnit, 0, sizeof(m_PackagePreviewUnit) );
	ClearCartItems(true);
#ifdef PRE_ADD_CADGE_CASH
	// Hack upon hack...
	SetCadgeListToCartList();
#endif // PRE_ADD_CADGE_CASH
	CDnLocalPlayerActor::HideAnotherPlayers( true, 0.f );
}

void CDnCashShopTask::OnRecvCashShopOpen(SCCashShopOpen* pPacket)
{

	if (pPacket->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_CASHSHOP_CREDIT
		m_UserCredit = pPacket->nNxACredit;
		m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
		m_UserCash = pPacket->nCashAmount;
		m_UserReserve = pPacket->nReserveAmount;

		GetInterface().ShowCashShopDlg(true);

		m_bSoundOnOpen = true;

		if (pPacket->wszAccountName && pPacket->wszAccountName[0] != '\0')
		{
			char szName[IDLENMAX] = { 0, };
			WideCharToMultiByte(CP_ACP, 0, pPacket->wszAccountName, -1, szName, sizeof(szName), NULL, NULL);

			m_AccountName = szName;
		}

		Sort(0, SORT_BASIC);

	}
	else
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4703 ));	// UISTRING : Cannot open the Cash Shop.
		CEtSoundEngine::GetInstance().FadeVolume( NULL, 1.f, CGlobalInfo::GetInstance().m_fFadeDelta, false );

		m_bOpened = false;

#ifdef PRE_MOD_SYSTEM_STATE
		GetInterface().GetSystemStateMgr().EndState(eSTATE_CASHSHOP);
#endif
	}

	m_bRequestingCashShopOpen = false;
}

void CDnCashShopTask::OnRecvCashShopClose(SCCashShopClose* pPacket)
{
	if (pPacket->nRet == ERROR_NONE)
	{
		m_bOpened = false;

		CloseChargeBrowser();

		m_pCashShopDlg->DisableAllDlgs(false, L"");
		m_pCashShopDlg->EnableDummyModalDlg(true);
		GetInterface().ShowCashShopDlg(false);

#ifdef PRE_MOD_SYSTEM_STATE
		GetInterface().GetSystemStateMgr().EndState(eSTATE_CASHSHOP);
#endif
	}
	else
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4704 ));	// UISTRING : An error occurred while closing the Cash Shop.
	}

	CDnLocalPlayerActor::HideAnotherPlayers( false, 0.f );
}

void CDnCashShopTask::RequestCashShopGiftReceiverInfo(const wchar_t *pReceiverName)
{
	SendCashShopCheckReceiver(pReceiverName);
}

void CDnCashShopTask::OnRecvCashShopGiftReceiverInfo(SCCashShopCheckReceiver* pReceiverInfo)
{
	if (pReceiverInfo != NULL)
		m_pCashShopDlg->UpdateGiftReceiverInfo(*pReceiverInfo);
}

void CDnCashShopTask::RequestCashShopRecvGiftBasicInfo()
{
	SendCashShopGiftList();
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4783 ), 2.f); // UISTRING : Receiving gift information. Please wait a moment.
	m_bRequestingGiftBasicInfoes = true;
}

void CDnCashShopTask::OnRecvCashShopRecvGiftBasicInfo(SCCashShopGiftList* pPacket)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");
	m_bRequestingGiftBasicInfoes = false;
	if (pPacket->nRet == ERROR_NONE)
	{
		int i = 0;
		m_GiftListCount = pPacket->cCount;

		m_RecvGiftInfoList.clear();
		if (pPacket->cCount <= 0)
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4784 ), MB_OK); // UISTRING : No received gifts.
			return;
		}

		for (; i < pPacket->cCount; ++i)
		{
			TGiftInfo& info = pPacket->GiftInfo[i];
			ConvertRecvGiftInfo(info);
		}

		m_pCashShopDlg->ShowRecvGiftDlg(true);
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnCashShopTask::RequestCashShopRecvItemByCoupon(const WCHAR* couponNum)
{
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4785 )); // UISTRING : Processing coupon.
	SendCashShopCoupon(couponNum);
}

void CDnCashShopTask::OnRecvCashShopRecvItemByCoupon(SCCashShopCoupon* pData)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");
	if (pData->nRet == ERROR_NONE)
	{
		m_pCashShopDlg->ShowCouponDlg(false);
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4570 ), MB_OK); // UISTRING : Successfully received the coupon item.
	}
	else
	{
		GetInterface().ServerMessageBox(pData->nRet);
	}
}

void CDnCashShopTask::RequestCashShopRecvGift(INT64 giftDBID, char cEmoticon, const std::wstring& memo, char cPayMethodCode)
{
	std::wstring str;
	std::map<int, std::vector<int> >::const_iterator iter = m_GiftReactInfo.find(cEmoticon + 1);
	if (iter != m_GiftReactInfo.end() && CDnActor::s_hLocalActor)
	{
		const std::vector<int>& reacts = (*iter).second;
		int index = _rand() % (int)reacts.size();
		std::wstring formatStr = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, reacts[index]);
		str = FormatW(formatStr.c_str(), CDnActor::s_hLocalActor->GetName());
	}

	SendCashShopRecvGift(giftDBID, str.c_str(), memo.c_str(), cPayMethodCode);
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4786 ));	// UISTRING : Retrieving gift.
}

void CDnCashShopTask::RequestCashShopGetChargeTime()
{
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4773 ), 1.f);	// UISTRING : Processing Cash charge.

	SendCashShopGetChargeTime();
}

#if defined(PRE_ADD_GIFT_RETURN)
void CDnCashShopTask::RequestCashShopGiftReturn(INT64 giftDBID)
{	
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4598 )); // UISTRING Returning gift.
	SendCashShopGiftReturn(giftDBID);
}
void CDnCashShopTask::OnRecvCashShopGiftReturn(SCCashShopGiftReturn* pData)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");
	if (pData)
	{
		if (pData->nRet != ERROR_NONE)
		{
			m_pCashShopDlg->ShowRecvGiftDlg(false);
			GetInterface().ServerMessageBox(pData->nRet);
		}
		else
		{
			RECV_GIFT_LIST::iterator iter = m_RecvGiftInfoList.begin();
			for (; iter != m_RecvGiftInfoList.end(); ++iter)
			{
				SCashShopRecvGiftBasicInfo& info = *iter;
				if (info.giftDBID == pData->nGiftDBID)
				{
					m_RecvGiftInfoList.erase(iter);
					break;
				}
			}

			m_GiftListCount = (int)m_RecvGiftInfoList.size();

			if (m_RecvGiftInfoList.empty() == false)
				m_pCashShopDlg->UpdateRecvGiftList();
			else
				m_pCashShopDlg->ShowRecvGiftDlg(false);
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4599 )); // UISTRING : Gift returned successfully.
		}
	}	
}
#endif

void CDnCashShopTask::OnRecvGiftNotify(SCNotifyGift* pPacket)
{
	m_GiftListCount = pPacket->nGiftCount;

	if (pPacket->bNew)
	{
		CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
		if( pMainBarDlg )
			pMainBarDlg->OnCashShopGiftNotify();
	}
}

void CDnCashShopTask::OnRecvCashShopRecvGift(SCCashShopReceiveGift* pData)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");
	if (pData)
	{
		if (pData->nRet != ERROR_NONE)
		{
			m_pCashShopDlg->ShowRecvGiftDlg(false);
			GetInterface().ServerMessageBox(pData->nRet);
		}
		else
		{
			RECV_GIFT_LIST::iterator iter = m_RecvGiftInfoList.begin();
			for (; iter != m_RecvGiftInfoList.end(); ++iter)
			{
				SCashShopRecvGiftBasicInfo& info = *iter;
				if (info.giftDBID == pData->nGiftDBID)
				{
					m_RecvGiftInfoList.erase(iter);
					break;
				}
			}

			m_GiftListCount = (int)m_RecvGiftInfoList.size();

			if (m_RecvGiftInfoList.empty() == false)
				m_pCashShopDlg->UpdateRecvGiftList();
			else
				m_pCashShopDlg->ShowRecvGiftDlg(false);
			GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4765)); // UISTRING : Received gift

#ifdef PRE_ADD_SALE_COUPON
			m_pCashShopDlg->CheckShowSaleTabButton( true );	// Check to activate discount tab immediately upon receiving coupon.
#endif // PRE_ADD_SALE_COUPON
		}
	}
	else
	{
		m_pCashShopDlg->ShowRecvGiftDlg(false);
		m_pCashShopDlg->HandleCashShopError(eERRCS_RECV_GIFT_PACKET, true);
	}
}

void CDnCashShopTask::OnRecvCashShopRecvGiftAll(SCCashShopReceiveGiftAll* pData)
{
#ifdef PRE_RECEIVEGIFTALL
	m_pCashShopDlg->DisableAllDlgs(false, L"");
	
	if(pData)
	{
		if (pData->nRet != ERROR_NONE)
		{
			m_pCashShopDlg->ShowRecvGiftDlg(false);
			if (pData->cCount == 0)
				GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4966));	// There are no gifts available to receive in the gift box
			else
				GetInterface().ServerMessageBox(pData->nRet);
		}
		else // nGiftDBID[GIFTPAGEMAX];
		{
			RECV_GIFT_LIST::iterator iter = m_RecvGiftInfoList.begin();
			for(int i = 0 ; i < (int)pData->cCount ; ++i)
			{
				for(; iter != m_RecvGiftInfoList.end(); ++iter)
				{
					SCashShopRecvGiftBasicInfo& info = *iter;
					if(info.giftDBID == pData->nGiftDBID[i])
					{
						m_RecvGiftInfoList.erase(iter);
						break;
					}
				}
			}
			m_GiftListCount = (int)m_RecvGiftInfoList.size();
			
			if(m_RecvGiftInfoList.empty() == false)
				m_pCashShopDlg->UpdateRecvGiftList();
			else
				m_pCashShopDlg->ShowRecvGiftDlg(false);
			
			int nStringID = -1;
			if(m_nSendItemSize > pData->cCount)
				nStringID = 4954; // UISTRING : Received gifts excluding those that cannot be claimed. Please check your Cash Inventory.  4954
			else
				nStringID = 4953; // UISTRING : All gifts on the current page have been received. Check your Cash Inventory. 4953
			GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nStringID)); 

#ifdef PRE_ADD_SALE_COUPON
			m_pCashShopDlg->CheckShowSaleTabButton( true );	// Check to activate discount tab immediately upon receiving coupon.
#endif // PRE_ADD_SALE_COUPON
		}
	}
	else
	{
		m_pCashShopDlg->ShowRecvGiftDlg(false);
		m_pCashShopDlg->HandleCashShopError(eERRCS_RECV_GIFT_PACKET, true);
	}
#endif // PRE_RECEIVEGIFTALL
}

#ifdef PRE_ADD_VIP
void CDnCashShopTask::OnRecvCashShopVIPBuy(SCVIPBuy* pPacket)
{
	ClearCartItems(false);

	m_pCashShopDlg->DisableAllDlgs(false, L"");

	if (pPacket->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_CASHSHOP_CREDIT
		m_UserCredit = pPacket->nNxACredit;
		m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
		m_UserCash = pPacket->nCashAmount;
		m_UserReserve = pPacket->nReserveAmount;

		if (m_pCashShopDlg)
			m_pCashShopDlg->UpdateCashReserveAmount();

		SCVIPInfo info;
		info.nVIPPoint = pPacket->nVIPPoint;
		info.tVIPExpirationDate = pPacket->tVIPExpirationDate;
		info.bAutoPay = pPacket->bAutoPay;
		info.bVIP = true;

		if (OnRecvCashShopVIPInfo(&info) == false)
			return;

		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4700 )); // UISTRING : Purchase succeeded
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnCashShopTask::OnRecvCashShopVIPGift(SCVIPGift* pPacket)
{
	ClearCartItems(false);

	m_pCashShopDlg->DisableAllDlgs(false, L"");

	if (pPacket->nRet == ERROR_NONE)
	{
		m_UserCash = pPacket->nCashAmount;
		m_UserReserve = pPacket->nReserveAmount;

		if (m_pCashShopDlg)
			m_pCashShopDlg->UpdateCashReserveAmount();

		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4787 )); // UISTRING : Gift sent
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet, MB_OK);
	}
}

bool CDnCashShopTask::OnRecvCashShopVIPInfo(SCVIPInfo* pPacket)
{
	CDnVIPDataMgr* pDataMgr = GetInterface().GetLocalPlayerVIPDataMgr();
	if (pDataMgr)
	{
		pDataMgr->SetData(pPacket->tVIPExpirationDate, pPacket->bAutoPay, pPacket->nVIPPoint, pPacket->bVIP);
	}
	else
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_VIP_NO_DATAMGR, true);
		return false;
	}

	return true;
}
#endif // PRE_ADD_VIP

void CDnCashShopTask::OnRecvSaleAbortList(SCSaleAbortList* pPacket)
{
	if (pPacket)
		SetNotOnSaleItem(pPacket->cCount, pPacket->nAbortList);
}

void CDnCashShopTask::ConvertRecvGiftInfo(const TGiftInfo& src)
{
	SCashShopRecvGiftBasicInfo info;
	info.payMethodCode = src.cPayMethodCode;
	info.giftDBID	= src.nGiftDBID;
	info.name		= src.wszSenderName;
	info.sn			= src.nItemSN;
	info.itemId		= src.nItemID;
#if defined(PRE_ADD_GIFT_RETURN)
	info.giftReturn = src.bGiftReturn;
#endif
	info.bNewFlag = src.bNewFlag;

	DBTIMESTAMP DbTime;
	std::wstring dateStr;
	if (src.tOrderDate <= 0)
	{
		info.recvDate = L"DATE ERROR";
	}
	else
	{
		CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( src.tOrderDate, &DbTime );
		dateStr = FormatW(L"%d/%d/%d %02d:%02d", DbTime.year, DbTime.month, DbTime.day, DbTime.hour, DbTime.minute);
		info.recvDate = dateStr;
	}

	if (src.tGiftExpireDate <= 0)
	{
		info.expireDate.clear();
	}
	else
	{
		CTimeSet::ConvertTimeT64ToDbTimeStamp_LC( src.tGiftExpireDate, &DbTime );
		dateStr = FormatW(L"%d/%d/%d %02d:%02d", DbTime.year, DbTime.month, DbTime.day, DbTime.hour, DbTime.minute);
		info.expireDate	= dateStr;
	}

	if (info.bNewFlag){
		int nMemoUIStringID = _wtoi(src.wszMessage);
		info.memo = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, nMemoUIStringID);
	}
	else
		info.memo = src.wszMessage;

	m_RecvGiftInfoList.push_back(info);
}

#define MAX_COSTUME_CART_ITEM	20

bool CDnCashShopTask::SetItemAbility(CASHITEM_SN sn, ITEMCLSID& ability)
{
	SCashShopItemInfo* pInfo = GetItemInfo(sn);
	if (pInfo == NULL)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_CART_NOT_EXIST, false);
		return false;
	}

	int count = GetValidAbilityCount(pInfo->abilityList);
	if (count <= 0)
	{
		_ASSERT(0);
		m_pCashShopDlg->HandleCashShopError(eERRCS_CART_NOT_EXIST, false);
		return false;
	}

	if (count == 1)
		ability = pInfo->abilityList[0];
	else
		ability = ITEMCLSID_NONE;

	return true;
}

bool CDnCashShopTask::HandleItemPurchasable(CASHITEM_SN sn, ITEMCLSID itemId)
{
	eItemTypeEnum itemType = CDnItem::GetItemType(itemId);
	if (itemType == ITEMTYPE_REBIRTH_COIN)
	{
		if (CDnItemTask::IsActive())
		{
			int nCashCoin(0);
			nCashCoin	= CDnItemTask::GetInstance().GetRebirthCoin(CDnItemTask::eCASHCOIN);

			SCashShopItemInfo* pInfo = GetItemInfo(sn);
			if (pInfo == NULL)
				return false;

			if (pInfo->count + nCashCoin > m_CashCoinRestriction)
			{
				std::wstring str;
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4739), m_CashCoinRestriction);	// UISTRING : You can own up to %d Resurrection Scrolls
				GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str(), true);
				return false;
			}
		}
		else
		{
			return false;
		}
	}

#if defined PRE_ADD_VIP
	SCashShopItemInfo* pItemInfo = GetCashShopTask().GetItemInfo(sn);
	const CDnVIPDataMgr* pDataMgr = GetInterface().GetLocalPlayerVIPDataMgr();
	if (pItemInfo && pDataMgr)
	{
		if (pItemInfo->bVIPSell)
		{
			bool bCannotBuy = false;
			const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(pDataMgr->GetCurrentPts());
			if (pUnit == NULL || (pItemInfo->neededVIPLevel > pUnit->level))
				bCannotBuy = true;

			if (bCannotBuy)
			{
				GetInterface().MessageBox(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4585));	// UISTRING : VIP level is too low to purchase
				return false;
			}
		}
	}
	else
	{
		_ASSERT(0);
		return false;
	}
#endif	// #if defined PRE_ADD_VIP

	return true;
}

bool CDnCashShopTask::IsValidSN(CASHITEM_SN sn) const
{
	return (sn > 0);
}

bool CDnCashShopTask::IsPeriodSelectableItem(CASHITEM_SN presentSN) const
{
	const SCashShopItemInfo* pItemInfo = GetItemInfo(presentSN);
	return (pItemInfo->linkIdList.size() >= 1);
}

void CDnCashShopTask::PutItemIntoCart(CASHITEM_SN sn)
{
	SCashShopCartItemInfo info;
	if (m_CartList.size() >= CARTLISTMAX)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_CART_MAX, false);
		return;
	}

	if (SetItemAbility(sn, info.ability) == false)
		return;

	info.presentSN = sn;
	if (IsPeriodSelectableItem(sn) == false)
		info.selectedSN = sn;
	info.id		= m_CartIndexGen;
	m_CartIndexGen++;
	m_CartList.push_back(info);

	SCartActionParam param;
	param.presentSN = info.presentSN;
	param.cartItemId = info.id;
	m_pCashShopDlg->UpdateCart(eRETCART_ADD, param);
}

bool CDnCashShopTask::PutItemIntoPreviewCart(int classId, CASHITEM_SN sn, ITEMCLSID itemId, bool *pCheckPackageCashRing2)
{
	SCashShopCartItemInfo info;
	const int index = GetClassArrayIndex(classId);
	if (index < 0)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_CHR_ID, true);
		return false;
	}

	CART_ITEM_LIST& curPreviewList = m_PreviewCartList[index];
	if (curPreviewList.size() > CASHEQUIPMAX)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_MAX, false);
		return false;
	}

	CART_ITEM_LIST::const_iterator iter = curPreviewList.begin();
	for (; iter != curPreviewList.end(); ++iter)
	{
		const SCashShopCartItemInfo& info = *iter;
		if (info.presentSN == sn)
		{
			// #58961 Exception handling when a package contains two rings.
			if (pCheckPackageCashRing2 != NULL)
			{
				bool bCheckRing2 = false;
				eItemTypeEnum itemType = CDnItem::GetItemType(itemId);
				if (itemType == ITEMTYPE_PARTS)
				{
					CDnParts::PartsTypeEnum partType = CDnParts::GetPartsType(itemId);
					if (partType == CDnParts::CashRing)
					{
						int nCount = 0;
						CART_ITEM_LIST::const_iterator iterCheckSN = curPreviewList.begin();
						for (; iterCheckSN != curPreviewList.end(); ++iterCheckSN)
						{
							const SCashShopCartItemInfo& infoCheckSN = *iterCheckSN;
							if (infoCheckSN.presentSN == sn) ++nCount;
						}
						if (nCount == 1) bCheckRing2 = true;
					}
				}
				if (bCheckRing2)
				{
					*pCheckPackageCashRing2 = true;
					continue;
				}
			}

			m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_ALREADY_EXIST, false);
			return false;
		}
	}

#define EQUIP_TYPE_OFFSET 1000

	eItemTypeEnum type = CDnItem::GetItemType(itemId);
	int prefix = type * EQUIP_TYPE_OFFSET;
	int postfix = 0;
	if (type == ITEMTYPE_PARTS)
	{
		CDnParts::PartsTypeEnum partsIdx = CDnParts::GetPartsType(itemId);
		if (partsIdx == -1)
		{
			_ASSERT(0);
			return false;
		}
		postfix += partsIdx;
	}
	else if (type == ITEMTYPE_WEAPON)
	{
		CDnWeapon::EquipTypeEnum weaponIdx = CDnWeapon::GetEquipType(itemId);
		if (weaponIdx == -1)
		{
			_ASSERT(0);
			return false;
		}
		postfix += weaponIdx;
	}

	if (SetItemAbility(sn, info.ability) == false)
		return false;

	info.presentSN = sn;
	if (IsPeriodSelectableItem(sn) == false)
		info.selectedSN = sn;
	info.id		= m_PreviewCartIndexGen;
	m_PreviewCartIndexGen++;
	info.equipIdx = prefix + postfix;

	CART_ITEM_LIST::iterator replaceIter = curPreviewList.begin();
	for (; replaceIter != curPreviewList.end(); ++replaceIter)
	{
		SCashShopCartItemInfo& curInfo = *replaceIter;
		if (curInfo.equipIdx == info.equipIdx)
		{
			curPreviewList.erase(replaceIter);
			curPreviewList.push_back(info);
			return true;
		}
	}

	curPreviewList.push_back(info);

	return true;
}

void CDnCashShopTask::DoPutItemIntoItemNow(CASHITEM_SN sn, ITEMCLSID ability, SCashShopCartItemInfo& result, CART_ITEM_LIST& packageItemList)
{
	result.Clear();
	if (IsPackageItem(sn))
	{
		packageItemList.clear();

		const std::vector<CASHITEM_SN>* pPackageItemList = GetInPackageItemList(sn);
		if (pPackageItemList != NULL)
		{
			std::vector<CASHITEM_SN>::const_iterator iter = pPackageItemList->begin();
			for (;iter != pPackageItemList->end(); ++iter)
			{
				CASHITEM_SN curSn = *iter;
				const SCashShopItemInfo* pCurInfo = GetCashShopTask().GetItemInfo(curSn);
				if (pCurInfo)
				{
					SCashShopCartItemInfo cartInfo;
					cartInfo.presentSN = cartInfo.selectedSN = pCurInfo->sn;
					cartInfo.isPackage = true;
					SetItemAbility(pCurInfo->sn, cartInfo.ability);

					packageItemList.push_back(cartInfo);
				}
			}
		}

		result.isPackage = true;
		result.selectedSN = sn;
	}
	else
	{
		if (IsPeriodSelectableItem(sn) == false)
			result.selectedSN = sn;
	}

	result.presentSN = sn;
	result.ability = ability;
}

void CDnCashShopTask::ShowGiftPayDlg(bool bShow, eCashUnitType type, const std::wstring& receiver, const std::wstring& memo)
{
	SetGiftBasicInfo(receiver, memo);
	if (m_pCashShopDlg)
		m_pCashShopDlg->ShowGiftPayDlg(bShow, type);
}

void CDnCashShopTask::SetGiftBasicInfo(const std::wstring& receiver, const std::wstring& memo)
{
	m_GiftBasicInfo.receiverName = receiver;
	m_GiftBasicInfo.memo = memo;
}

bool CDnCashShopTask::HandleItemLevelLimit(eCashUnitType type, int userLevel, int callBackDlgID /* = -1 */, CEtUICallback* pCall /* = NULL */)
{
	eCashShopError ret = eERRCS_NONE;
	const CART_ITEM_LIST* pCartList = NULL;
	if (type == eCashUnit_Gift_Cart)
	{
		pCartList = &(GetCartList());
	}
	else if (type == eCashUnit_Gift_PreviewCart)
	{
		pCartList = GetPreviewCartList(m_PreviewCartClassId);
	}
	else if (type == eCashUnit_Gift_Package || type == eCashUnit_GiftItemNow)
	{
		return HandleItemLevelLimit(m_BuyItemNow, userLevel, callBackDlgID, pCall);
	}

	if (pCartList == NULL)
		return false;

	return HandleItemLevelLimit(*pCartList, userLevel, callBackDlgID, pCall);
}

bool CDnCashShopTask::HandleItemLevelLimit(const CART_ITEM_LIST& itemList, int userLevel, int callBackDlgID, CEtUICallback* pCall) const
{
	if (m_pCashShopDlg == NULL)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
		return false;
	}

	std::vector<CASHITEM_SN> limitItemList;
	CART_ITEM_LIST::const_iterator iter = itemList.begin();
	for (; iter != itemList.end(); ++iter)
	{
		const SCashShopCartItemInfo& info = *iter;
		const SCashShopItemInfo* pItemInfo = GetItemInfo(info.presentSN);
		if (pItemInfo == NULL)
		{
			_ASSERT(0);
			m_pCashShopDlg->HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
			return false;
		}

		if (pItemInfo->levelLimit > userLevel)
			limitItemList.push_back(pItemInfo->sn);
	}

	if (limitItemList.empty() == false)
	{
		std::wstring msg;
		msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4679);	// UISTRING : There is an item you cannot use due to level mismatch. Do you really want to purchase it?
		m_pCashShopDlg->ShowGiftFailDlg(true, msg.c_str(), limitItemList, MB_YESNO, callBackDlgID, pCall);
		return false;
	}

	return true;
}

bool CDnCashShopTask::HandleItemLevelLimit(const SCashShopCartItemInfo& itemInfo, int userLevel, int callBackDlgID /* = -1 */, CEtUICallback* pCall /* = NULL */)
{
	const SCashShopItemInfo* pItemInfo = GetItemInfo(itemInfo.presentSN);
	if (pItemInfo == NULL)
	{
		_ASSERT(0);
		m_pCashShopDlg->HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
		return false;
	}

	if (pItemInfo->levelLimit > userLevel)
	{
		std::wstring msg;
		msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4679);	// UISTRING : There is an item you cannot use due to level mismatch. Do you really want to purchase it?
		std::vector<CASHITEM_SN> limitItemList;
		limitItemList.push_back(pItemInfo->sn);
		m_pCashShopDlg->ShowGiftFailDlg(true, msg.c_str(), limitItemList, MB_YESNO, callBackDlgID, pCall);
		return false;
	}

	return true;
}

bool CDnCashShopTask::HandleItemOverlapBuy(const CART_ITEM_LIST& itemList, eCashUnitType type, int callBackDlgID, CEtUICallback* pCall)
{
	if (m_pCashShopDlg == NULL)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
		return false;
	}

	std::vector<CASHITEM_SN> canoverlapItemList;
	std::vector<CASHITEM_SN> cannotoverlapItemList;
	bool IsWaningOverlapItem = false;
	CART_ITEM_LIST::const_iterator iter = itemList.begin();
	for (; iter != itemList.end(); ++iter)
	{
		const SCashShopCartItemInfo& info = *iter;
		const SCashShopItemInfo* pItemInfo = GetItemInfo(info.presentSN);
		if (pItemInfo == NULL)
		{
			_ASSERT(0);
			m_pCashShopDlg->HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
			return false;
		}

		if( pItemInfo->nOverlapBuy > 0 && GetValidAbilityCount(pItemInfo->abilityList) == 1 )
		{
			CDnItem * pFindItemStatus = GetItemTask().FindItem( pItemInfo->presentItemId, ITEM_SLOT_TYPE::ST_CHARSTATUS );
			CDnItem * pFindItemInvCash = GetItemTask().FindItem( pItemInfo->presentItemId, ITEM_SLOT_TYPE::ST_INVENTORY_CASH );
			if( pFindItemStatus || pFindItemInvCash )
			{
				if( pItemInfo->nOverlapBuy == 1 )		canoverlapItemList.push_back(pItemInfo->sn);
				else if( pItemInfo->nOverlapBuy == 2 )	cannotoverlapItemList.push_back(pItemInfo->sn);
			}
		}
		if( CDnItem::GetItemType(pItemInfo->presentItemId) == ITEMTYPE_PERIODEXPITEM)
		{			
			if( GetItemTask().FindItemCountFromItemType(ITEMTYPE_PERIODEXPITEM) > 0 )
				IsWaningOverlapItem = true;
		}
	}

	if (canoverlapItemList.empty() == false )
	{
		// Duplicate purchase allowed
		std::wstring msg;
		msg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4799 );	// UISTRING : You have already purchased this product. Are you sure you want to buy it?
		UINT nButtonType = MB_YESNO;
		if( type == eCashUnit_BuyItemNow)	nButtonType = MB_YESNO;
		else if( type == eCashUnit_Cart )	nButtonType = MB_RETRYCANCEL;
		m_pCashShopDlg->ShowGiftFailDlg( true, msg.c_str(), canoverlapItemList, nButtonType, callBackDlgID, pCall );

		if( cannotoverlapItemList.empty() )	// Because the duplicate-purchase-disallowed window also needs to be shown...
			return false;
	}

	if( cannotoverlapItemList.empty() == false )
	{
		// Duplicate purchase not allowed
		std::wstring msg;
		msg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4518 );	// UISTRING : You have already purchased this product.
		m_pCashShopDlg->ShowGiftFailDlg( true, msg.c_str(), cannotoverlapItemList, MB_OK );
		return false;
	}
	if( IsWaningOverlapItem )
	{
		// Item of the same type already exists.. warning
		std::wstring firstmsg, secondmsg;		
		// UISTRING : You already own an EXP boost item. Purchase anyway?
		firstmsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 130256 );	
		// UISTRING : EXP boost items do not stack; only the highest-value item applies.
		secondmsg = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 130255 );		

		m_pCashShopDlg->ShowCashShopMsgBox(firstmsg,secondmsg, callBackDlgID, pCall);
		return false;
	}
	return true;
}

bool CDnCashShopTask::PutItemIntoBuyItemNow(CASHITEM_SN sn)
{
	ITEMCLSID ability = ITEMCLSID_NONE;
	if (SetItemAbility(sn, ability) == false)
		return false;

	if (HandleItemPurchasable(sn, ability) == false)
		return false;

	if (IsNotOnSaleItem(sn))
	{
		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4586 )); // UISTRING : Contains an item that cannot be purchased.
		return false;
	}

	DoPutItemIntoItemNow(sn, ability, m_BuyItemNow, m_BuyPackageItemList);

	return true;
}

eCashShopError CDnCashShopTask::IsAllCartItemPeriodSelected(eCashUnitType type, bool bHandleError) const
{
	eCashShopError ret = eERRCS_NONE;
	if (type == eCashUnit_Cart || type == eCashUnit_Gift_Cart)
	{
		if (m_CartList.empty())
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
		for (; iter != m_CartList.end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidSN(info.selectedSN) == false)
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
		}
	}
	else if (type == eCashUnit_BuyItemNow || type == eCashUnit_Package)
	{
		if (IsValidSN(m_BuyItemNow.selectedSN) == false)
			ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
	}
	else if (type == eCashUnit_PreviewCart || type == eCashUnit_Gift_PreviewCart)
	{
		const CART_ITEM_LIST* pPreviewList = GetPreviewCartList(m_PreviewCartClassId);
		if (pPreviewList == NULL)
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = pPreviewList->begin();
		for (; iter != pPreviewList->end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidSN(info.selectedSN) == false)
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
		}
	}
	else if (type == eCashUnit_GiftItemNow || type == eCashUnit_Gift_Package)
	{
		if (IsValidSN(m_BuyItemNow.selectedSN) == false)
			ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
	}
	else
	{
		_ASSERT(0);
		ret = eERRCS_CART_NOT_EXIST;
	}

	if (ret != eERRCS_NONE && bHandleError)
		m_pCashShopDlg->HandleCashShopError(ret, false);

	return ret;
}

bool CDnCashShopTask::IsValidAbility(ITEMCLSID ability) const
{
	return (ability > ITEMCLSID_NONE);
}


eCashShopError CDnCashShopTask::IsAllCartItemAbilitySelected(eCashUnitType type, bool bHandleError) const
{
	eCashShopError ret = eERRCS_NONE;
	if (type == eCashUnit_Cart || type == eCashUnit_Gift_Cart)
	{
		if (m_CartList.empty())
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
		for (; iter != m_CartList.end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidAbility(info.ability) == false)
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
		}
	}
	else if (type == eCashUnit_BuyItemNow || type == eCashUnit_Package)
	{
		if (IsValidAbility(m_BuyItemNow.ability) == false)
			ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
	}
	else if (type == eCashUnit_PreviewCart || type == eCashUnit_Gift_PreviewCart)
	{
		const CART_ITEM_LIST* pPreviewList = GetPreviewCartList(m_PreviewCartClassId);
		if (pPreviewList == NULL)
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = pPreviewList->begin();
		for (; iter != pPreviewList->end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidAbility(info.ability) == false)
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
		}
	}
	else if (type == eCashUnit_GiftItemNow || type == eCashUnit_Gift_Package)
	{
		if (IsValidAbility(m_BuyItemNow.ability) == false)
			ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
	}
	else
	{
		_ASSERT(0);
		ret = eERRCS_CART_NOT_EXIST;
	}

	if (ret != eERRCS_NONE && bHandleError)
		m_pCashShopDlg->HandleCashShopError(ret, false);

	return ret;
}

eCashShopError CDnCashShopTask::IsAllCartItemSelected(eCashUnitType type, bool bHandleError) const
{
	eCashShopError ret = eERRCS_NONE;
	if (type == eCashUnit_Cart || type == eCashUnit_Gift_Cart)
	{
		if (m_CartList.empty())
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
		for (; iter != m_CartList.end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidSN(info.selectedSN) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
				break;
			}
			if (IsValidAbility(info.ability) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
				break;
			}
			if( IsValidSkill( info.ability, info.nOptionIndex ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;
				break;
			}
		}
	}
	else if (type == eCashUnit_PreviewCart || type == eCashUnit_Gift_PreviewCart)
	{
		const CART_ITEM_LIST* pPreviewList = GetPreviewCartList(m_PreviewCartClassId);
		if (pPreviewList == NULL)
			ret = eERRCS_CART_EMPTY;

		CART_ITEM_LIST::const_iterator iter = pPreviewList->begin();
		for (; iter != pPreviewList->end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidSN(info.selectedSN) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
				break;
			}
			if (IsValidAbility(info.ability) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
				break;
			}
			if( IsValidSkill( info.ability, info.nOptionIndex ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;
				break;
			}
		}
	}
	else if (type == eCashUnit_BuyItemNow || type == eCashUnit_GiftItemNow)
	{
		if( IsValidSN( m_BuyItemNow.selectedSN ) == false )
			ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
		else if( IsValidAbility( m_BuyItemNow.ability ) == false )
			ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
		else if( IsValidSkill( m_BuyItemNow.ability, m_BuyItemNow.nOptionIndex ) == false )
			ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;
	}
	else if (type == eCashUnit_Package || type == eCashUnit_Gift_Package)
	{
		const CART_ITEM_LIST* pList = NULL;
		if (type == eCashUnit_Package)
			pList = &m_BuyPackageItemList;
		else if (type == eCashUnit_Gift_Package)
			pList = &m_GiftPackageItemList;


		CART_ITEM_LIST::const_iterator iter = pList->begin();
		for (; iter != pList->end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			if (IsValidSN(info.selectedSN) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
				break;
			}
			if (IsValidAbility(info.ability) == false)
			{
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
				break;
			}
			if( IsValidSkill( info.ability, info.nOptionIndex ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;
				break;
			}
		}
	}
	else
	{
		_ASSERT(0);
		ret = eERRCS_CART_NOT_EXIST;
	}

	if (ret != eERRCS_NONE && bHandleError)
		m_pCashShopDlg->HandleCashShopError(ret, false);

	return ret;
}

bool CDnCashShopTask::PutItemIntoGiftItemNow(CASHITEM_SN sn)
{
	ITEMCLSID ability = ITEMCLSID_NONE;
	if (SetItemAbility(sn, ability) == false)
		return false;

	DoPutItemIntoItemNow(sn, ability, m_BuyItemNow, m_GiftPackageItemList);

	return true;
}

bool CDnCashShopTask::PutItemIntoCartItemNow(CASHITEM_SN sn)
{
	ITEMCLSID ability = ITEMCLSID_NONE;
	if (SetItemAbility(sn, ability) == false)
		return false;

	DoPutItemIntoItemNow(sn, ability, m_BuyItemNow, m_GiftPackageItemList);

	return true;
}

const SCashShopItemInfo* CDnCashShopTask::GetItemInfo(CASHITEM_SN sn) const
{
	const CS_BASIC_INFO_LIST::const_iterator iter = m_BasicInfoList.find(sn);
	if (iter != m_BasicInfoList.end())
	{
		const SCashShopItemInfo* pInfo = (*iter).second;
		if (pInfo)
			return pInfo;
	}

	return NULL;
}

SCashShopItemInfo* CDnCashShopTask::GetItemInfo(CASHITEM_SN sn)
{
	const CS_BASIC_INFO_LIST::iterator iter = m_BasicInfoList.find(sn);
	if (iter != m_BasicInfoList.end())
	{
		SCashShopItemInfo* pInfo = (*iter).second;
		if (pInfo)
			return pInfo;
	}

	return NULL;
}

SCashShopCartItemInfo* CDnCashShopTask::GetCartItemInfo(int cartId)
{
	if (cartId < 0)
		return NULL;

	CART_ITEM_LIST::iterator iter = m_CartList.begin();
	for (; iter != m_CartList.end(); ++iter)
	{
		SCashShopCartItemInfo& info = *iter;
		if (info.id == cartId)
			return &info;
	}

	return NULL;
}

const SCashShopCartItemInfo* CDnCashShopTask::GetCartListItemInfo(const CART_ITEM_LIST& list, int index) const
{
	if (index > (int)list.size() || index < 0)
		return NULL;

	int i = 0;
	CART_ITEM_LIST::const_iterator iter = list.begin();
	for (; iter != list.end(); ++iter, ++i)
	{
		if (i == index)
			return &(*iter);
	}

	return NULL;
}

void CDnCashShopTask::BuyCart()
{
	m_pCashShopDlg->OpenBuyCart(true);
}

void CDnCashShopTask::BuyPreviewCart(int classId)
{
	m_pCashShopDlg->OpenBuyPreviewCart(true);
}

void CDnCashShopTask::GiftCart(bool bClear)
{
	m_pCashShopDlg->GiftCart(bClear);
}

void CDnCashShopTask::GiftPreviewCart()
{
	m_pCashShopDlg->GiftPreviewCart(true);
}

std::wstring CDnCashShopTask::GetAbilityString( ITEMCLSID itemId )
{
	std::wstring strAbility;
	// If item does not exist in CashGoodsInfo, output empty value
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TCASHGOODSINFO );
	if( pSox == NULL ) return strAbility;

	bool bFindItem = false;
	for( int iItemID=0; iItemID<pSox->GetItemCount(); ++iItemID )
	{
		int nTableID = pSox->GetItemID( iItemID );
		ITEMCLSID id = pSox->GetFieldFromLablePtr( nTableID, "_ItemID" )->GetInteger();
		if( id == itemId )
		{
			bFindItem = true;
			break;
		}
	}

	if( !bFindItem )
		return strAbility;

	int nStringNumber = GetAbilityStringNumber( itemId );

	if( nStringNumber > 0 )
		return GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringNumber );

	pSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox || !pSox->IsExistItem( itemId ) )
		return strAbility;

	char szLabel[32];
	WCHAR wszTemp[128] = { 0, };
	for( int i=0; i<10; i++ ) 
	{
		sprintf_s( szLabel, "_State%d", i + 1 );
		int nStateType = pSox->GetFieldFromLablePtr( itemId, szLabel )->GetInteger();
		if( nStateType >= 0 )
		{
			sprintf_s( szLabel, "_State%d_Min", i + 1 );
			char *szStateValue = pSox->GetFieldFromLablePtr( itemId, szLabel )->GetString();
			
			if( (nStateType < 50 && !(16 <= nStateType && nStateType <= 23)) || nStateType == 100 )		// + State
			{
				if( nStateType == 100 ) nStateType = 30;		// Tenacity
				swprintf_s( wszTemp, L"%s %c%d", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, CDnItem::GetStateValueMatchUIStringIndex(nStateType) ), ( atoi(szStateValue) >= 0 ) ? '+' : '-', abs(atoi(szStateValue)) );
			}
			else	// % State
			{
				if( nStateType >= 50 ) nStateType -= 50;
				float fValue = (float)atof(szStateValue);
				swprintf_s( wszTemp, L"%s %c%.2f%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, CDnItem::GetStateValueMatchUIStringIndex(nStateType) ), ( fValue >= 0.f ) ? '+' : '-', abs( fValue ) * 100.f );
			}

			if( i > 0 )
				strAbility += L" ";

			strAbility += wszTemp;
		}
	}

	return strAbility;
}

int	CDnCashShopTask::GetAbilityStringNumber(ITEMCLSID id) const
{
	std::map<ITEMCLSID, int>::const_iterator iter = m_AbilityDescInfoList.find(id);
	if (iter != m_AbilityDescInfoList.end())
		return (*iter).second;

	return -1;
}

int	CDnCashShopTask::GetOnSaleCount(const CS_INFO_LIST& list) const
{
	int count = 0;
	CS_INFO_LIST::const_iterator iter = list.begin();
	for (;iter != list.end(); ++iter)
	{
		const SCashShopItemInfo* pInfo = *iter;
		if (pInfo && pInfo->bOnSale && IsNotOnSaleItem(pInfo->sn) == false)
			++count;
	}

	return count;
}

void CDnCashShopTask::SetCartItemAbility(int index, ITEMCLSID abilityID)
{
	SCashShopCartItemInfo* pCartItem = GetCartItemInfo(index);
	if (pCartItem != NULL)
	{
		pCartItem->ability = abilityID;
	}
}

void CDnCashShopTask::SelectCartItemSN(eCashUnitType type, int index, CASHITEM_SN sn)
{
	if (type == eCashUnit_Cart || type == eCashUnit_Gift_Cart)
	{
		SCashShopCartItemInfo* pCartItem = GetCartItemInfo(index);
		if (pCartItem != NULL)
			pCartItem->selectedSN = sn;
		else
			_ASSERT(0);
	}
	else if (type == eCashUnit_BuyItemNow || type == eCashUnit_Package)
	{
		m_BuyItemNow.selectedSN = sn;
	}
	else if (type == eCashUnit_PreviewCart || type == eCashUnit_Gift_PreviewCart)
	{
		CART_ITEM_LIST* pInfo = GetBuyPreviewCartList();
		if (pInfo == NULL)
		{
			_ASSERT(0);
			return;
		}

		CART_ITEM_LIST::iterator iter = pInfo->begin();
		for (; iter != pInfo->end(); ++iter)
		{
			SCashShopCartItemInfo& info = *iter;
			if (info.id == index)
			{
				info.selectedSN = sn;
				return;
			}
		}
	}
	else if (type == eCashUnit_GiftItemNow || type == eCashUnit_Gift_Package)
	{
		m_BuyItemNow.selectedSN = sn;
	}
	else
	{
		_ASSERT(0);
		m_pCashShopDlg->HandleCashShopError(eERRCS_CART_NOT_EXIST, false);
	}
	
}

void CDnCashShopTask::SetPreviewCartItemAbility(int index, ITEMCLSID abilityID)
{
	CART_ITEM_LIST* pInfo = GetBuyPreviewCartList();
	if (pInfo == NULL)
	{
		_ASSERT(0);
		return;
	}

	CART_ITEM_LIST::iterator iter = pInfo->begin();
	for (; iter != pInfo->end(); ++iter)
	{
		SCashShopCartItemInfo& info = *iter;
		if (info.id == index)
		{
			info.ability = abilityID;
			return;
		}
	}
}

void CDnCashShopTask::RemoveCartItem(int cartItemId)
{
	if (cartItemId < 0)
		return;

	int i = 0;
	CART_ITEM_LIST::iterator iter = m_CartList.begin();
	for (; iter != m_CartList.end(); ++iter, ++i)
	{
		SCashShopCartItemInfo& info = *iter;
		if (info.id == cartItemId)
		{
			SCartActionParam param;
			param.presentSN = info.presentSN;
			param.cartItemId = cartItemId;
			m_CartList.erase(iter);
			m_pCashShopDlg->UpdateCart(eRETCART_REMOVE, param);
			break;
		}
	}
}

void CDnCashShopTask::RemovePreviewCartItem(CASHITEM_SN sn)
{
	int i = 0;
	bool bErased = false;
	for (; i < CLASSKINDMAX && bErased == false; ++i)
	{
		CART_ITEM_LIST& itemList = m_PreviewCartList[i];
		CART_ITEM_LIST::iterator iter = itemList.begin();
		for (; iter != itemList.end(); ++iter)
		{
			SCashShopCartItemInfo& info = *iter;
			if (info.presentSN == sn)
			{
				m_pCashShopDlg->DetachPartsToPreview(i + 1, sn);
				itemList.erase(iter);
				bErased = true;
				break;
			}
		}
	}
}

void CDnCashShopTask::RemovePreviewCartItemById(int id)
{
	int i = 0;
	bool bErased = false;

	for (; i < CLASSKINDMAX && bErased == false; ++i)
	{
		CART_ITEM_LIST& itemList = m_PreviewCartList[i];
		CART_ITEM_LIST::iterator iter = itemList.begin();
		for (; iter != itemList.end(); ++iter)
		{
			SCashShopCartItemInfo& info = *iter;
			if (info.id == id)
			{
				m_pCashShopDlg->DetachPartsToPreview(i + 1, info.presentSN);
				itemList.erase(iter);
				bErased = true;
				break;
			}
		}
	}

	if( !bErased )
	{
		CART_ITEM_LIST& itemList = m_PetPreviewCartList;
		CART_ITEM_LIST::iterator iter = itemList.begin();
		bool bErasePetBody = false;
		for( ; iter != itemList.end(); ++iter )
		{
			SCashShopCartItemInfo& info = *iter;
			if( info.id == id )
			{
				if( info.equipIdx == Pet::Slot::Body )
				{
					bErasePetBody = true;
					break;
				}
				else
					m_pCashShopDlg->DetachPartsToPetPreview( info.equipIdx );

				itemList.erase( iter );
				bErased = true;
				break;
			}
		}

		if( !bErased && bErasePetBody )
		{
			if( m_pCashShopDlg->GetCurrentTabID() == 3 )
				m_pCashShopDlg->GetPetPreviewDlg()->ResetBasePet();
			else
				m_pCashShopDlg->SwapPreview( true );

			m_PetPreviewCartList.clear();
		}
	}
}

void CDnCashShopTask::ClearCartItems(bool bWithPreviewCart)
{
	m_CartList.clear();

	SCartActionParam param;
	m_pCashShopDlg->UpdateCart(eRETCART_CLEAR, param);

	if (bWithPreviewCart)
	{
		int i = 0;
		for (; i < CLASSKINDMAX; ++i)
		{
			CART_ITEM_LIST& cartItemList = m_PreviewCartList[i];
			cartItemList.clear();
		}

		m_pCashShopDlg->ClearPreviewCart(m_PreviewCartClassId);
		m_PreviewCartIndexGen = 0;
		m_PetPreviewCartList.clear();
	}
}

void CDnCashShopTask::SetBuyItemNowAbility(ITEMCLSID abilityID)
{
	m_BuyItemNow.ability = abilityID;
}

bool CDnCashShopTask::IsInWishList(CASHITEM_SN sn) const
{
	CS_INFO_LIST::const_iterator iter = m_WishList.begin();
	for (; iter != m_WishList.end(); ++iter)
	{
		const SCashShopItemInfo* pInfo = (*iter);
		if (pInfo->sn == sn)
			return true;
	}

	return false;
}

void CDnCashShopTask::AddWishListItem(CASHITEM_SN sn)
{
	std::wstring str;
	if (IsInWishList(sn) == false)
	{
		SCashShopItemInfo* pItem = GetItemInfo(sn);
		m_WishList.push_back(pItem);

		str = FormatW(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4702 ), pItem->nameString.c_str());	// UISTRING : Added [%s] to your wishlist
		//GetInterface().MessageBox(str.c_str());
	}
	else
	{
		str = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4701 );	// UISTRING : Item is already in your wishlist.
	}
	GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", str.c_str());
}

void CDnCashShopTask::DeleteWishListItem(CASHITEM_SN sn)
{
	CS_INFO_LIST::iterator iter = m_WishList.begin();
	for (; iter != m_WishList.end(); ++iter)
	{
		const SCashShopItemInfo* pCurInfo = (*iter);
		if (pCurInfo && pCurInfo->sn == sn)
		{
			m_WishList.erase(iter);
			return;
		}
	}
}

int CDnCashShopTask::GetCashLimitBuyAbleCount(CASHITEM_SN sn) const
{
	std::map<CASHITEM_SN, SCashItemLimitInfo>::const_iterator iter = m_CashItemLimitInfo.find(sn);
	if (iter != m_CashItemLimitInfo.end())
	{
		const SCashItemLimitInfo& info = (*iter).second;
		return info.nBuyAbleCount;
	}

	return 0;
}

const WCHAR* CDnCashShopTask::GetCashLimitItemStartDate(CASHITEM_SN sn) const
{
	std::map<CASHITEM_SN, SCashItemLimitInfo>::const_iterator iter = m_CashItemLimitInfo.find(sn);
	if (iter != m_CashItemLimitInfo.end())
	{
		const SCashItemLimitInfo& info = (*iter).second;
		return info.start.c_str();
	}

	return NULL;
}

const WCHAR* CDnCashShopTask::GetCashLimitItemEndDate(CASHITEM_SN sn) const
{
	std::map<CASHITEM_SN, SCashItemLimitInfo>::const_iterator iter = m_CashItemLimitInfo.find(sn);
	if (iter != m_CashItemLimitInfo.end())
	{
		const SCashItemLimitInfo& info = (*iter).second;
		return info.end.c_str();
	}

	return NULL;
}

#ifdef PRE_ADD_LIMITED_CASHITEM
int CDnCashShopTask::GetCashLimitItemMaxCount(CASHITEM_SN sn) const
{
	std::map<CASHITEM_SN, SCashItemLimitInfo>::const_iterator iter = m_CashItemLimitInfo.find(sn);
	if (iter != m_CashItemLimitInfo.end())
	{
		const SCashItemLimitInfo& info = (*iter).second;
		return info.nMaxCount;
	}

	return 0;
}

int CDnCashShopTask::GetCashLimitItemRemainCount(CASHITEM_SN sn) const
{
	int nRemainCount = 0;
	std::map<CASHITEM_SN, SCashItemLimitInfo>::const_iterator iter = m_CashItemLimitInfo.find(sn);
	if (iter != m_CashItemLimitInfo.end())
	{
		const SCashItemLimitInfo& info = (*iter).second;
		nRemainCount = info.nMaxCount - info.nSaleCount;
		if( nRemainCount < 0 )
			nRemainCount = 0;
	}

	return nRemainCount;
}
#endif // PRE_ADD_LIMITED_CASHITEM

const std::vector<CASHITEM_SN>* CDnCashShopTask::GetInPackageItemList(CASHITEM_SN packageSN) const
{
	std::map<CASHITEM_SN, std::vector<CASHITEM_SN> >::const_iterator iter = m_CashItemPackageInfo.find(packageSN);
	if (iter != m_CashItemPackageInfo.end())
		return &((*iter).second);

	return NULL;
}
#ifdef PRE_ADD_CADGE_CASH
void CDnCashShopTask::RequestCashShopSendGiftItemNow(const std::wstring& receiverName, const std::wstring& memo, int nMailID)
#else // PRE_ADD_CADGE_CASH
void CDnCashShopTask::RequestCashShopSendGiftItemNow(const std::wstring& receiverName, const std::wstring& memo)
#endif // PRE_ADD_CADGE_CASH
{
#ifdef PRE_ADD_VIP
	if (IsVIPItem(m_BuyItemNow.ability))
	{
		m_pCashShopDlg->OpenBuyCart(false);
		m_bBuying = true;

		SendCashShopVIPGift(m_BuyItemNow.selectedSN, receiverName.c_str(), memo.c_str());
	}
	else
#endif // PRE_ADD_VIP
	if (IsPackageItem(m_BuyItemNow.selectedSN))
	{
		std::vector<TCashShopPackageInfo> infoList;

		const CART_ITEM_LIST& packageItemList = m_GiftPackageItemList;
		CART_ITEM_LIST::const_iterator iter = packageItemList.begin();
		for (;iter != packageItemList.end(); ++iter)
		{
			const SCashShopCartItemInfo& cartInfo = *iter;

			TCashShopPackageInfo info;
			info.nItemSN = cartInfo.selectedSN;
			info.nItemID = cartInfo.ability;
			info.nOptionIndex = cartInfo.nOptionIndex;
			infoList.push_back(info);
		}

		m_pCashShopDlg->OpenBuyCart(false);
		m_bBuying = true;
#ifdef PRE_ADD_CADGE_CASH
		SendCashShopPackageGift(receiverName.c_str(), memo.c_str(), m_BuyItemNow.selectedSN, infoList, nMailID);
#else // PRE_ADD_CADGE_CASH
		SendCashShopPackageGift(receiverName.c_str(), memo.c_str(), m_BuyItemNow.selectedSN, infoList, 0);
#endif // PRE_ADD_CADGE_CASH
	}
	else
	{
		std::vector<TCashShopInfo> infoList;
		TCashShopInfo info;
		info.nItemID = m_BuyItemNow.ability;
		info.nItemSN = m_BuyItemNow.selectedSN;
		info.cSlotIndex = m_BuyItemNow.id;
		info.nOptionIndex = m_BuyItemNow.nOptionIndex;

		infoList.push_back(info);

		m_bBuying = true;
		m_pCashShopDlg->ShowGiftPayDlg(false, eCashUnit_GiftItemNow);
#ifdef PRE_ADD_CADGE_CASH
		SendCashShopGift(eCashUnit_GiftItemNow, receiverName.c_str(), memo.c_str(), infoList, nMailID);
#else // PRE_ADD_CADGE_CASH
		SendCashShopGift(eCashUnit_GiftItemNow, receiverName.c_str(), memo.c_str(), infoList, 0);
#endif // PRE_ADD_CADGE_CASH
	}
}

#ifdef PRE_ADD_CADGE_CASH

eCashShopError CDnCashShopTask::IsCheckEnableCadge( eCashUnitType type )
{
	eCashShopError ret = eERRCS_NONE;

	if( type == eCashUnit_BuyItemNow )
	{
		if( IsValidSN( m_BuyItemNow.selectedSN ) == false )
			ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
		else if( IsValidAbility( m_BuyItemNow.ability ) == false )
			ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
		else if( IsValidSkill( m_BuyItemNow.ability, m_BuyItemNow.nOptionIndex ) == false )
			ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;

		const SCashShopItemInfo* pItemInfo = GetItemInfo( m_BuyItemNow.presentSN );
		if( pItemInfo )
		{
			if( pItemInfo->bGiftUsable == false || IsNotOnSaleItem( pItemInfo->sn ) )
				ret = eERRCS_GIFT_UNABLE_NOGIFT_OPTION;
		}
	}
	else if( type == eCashUnit_Package || type == eCashUnit_PreviewCart || type == eCashUnit_Cart )
	{
		const CART_ITEM_LIST* pList = NULL;
		if( type == eCashUnit_Package )
			pList = &m_BuyPackageItemList;
		else if( type == eCashUnit_PreviewCart )
			pList = GetPreviewCartList( m_PreviewCartClassId );
		else if( type == eCashUnit_Cart )
			pList = &m_CartList;

		CART_ITEM_LIST::const_iterator iter = pList->begin();
		for( ; iter != pList->end(); ++iter )
		{
			const SCashShopCartItemInfo& info = *iter;
			if( IsValidSN( info.selectedSN ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_PERIOD_SELECTED;
				break;
			}
			if( IsValidAbility( info.ability ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_ABILITY_SELECTED;
				break;
			}
			if( IsValidSkill( info.ability, info.nOptionIndex ) == false )
			{
				ret = eERRCS_NO_ALL_ITEM_SKILL_SELECTED;
				break;
			}

			const SCashShopItemInfo* pItemInfo = GetItemInfo( info.presentSN );
			if( pItemInfo )
			{
				if( pItemInfo->bGiftUsable == false || IsNotOnSaleItem( pItemInfo->sn ) )
				{
					ret = eERRCS_GIFT_UNABLE_NOGIFT_OPTION;
					break;
				}
			}
		}
	}

	return ret;
}

void CDnCashShopTask::RequestCadge( eCashUnitType type, const std::wstring& receiver, const std::wstring& memo )
{
	std::vector<TCashShopPackageInfo> infoList;

	if( IsPackageItem( m_BuyItemNow.selectedSN ) )
	{
		const CART_ITEM_LIST& packageItemList = m_BuyPackageItemList;
		CART_ITEM_LIST::const_iterator iter = packageItemList.begin();
		for( ;iter != packageItemList.end(); ++iter )
		{
			const SCashShopCartItemInfo& cartInfo = *iter;

			TCashShopPackageInfo info;
			info.nItemSN = cartInfo.selectedSN;
			info.nItemID = cartInfo.ability;
			info.nOptionIndex = cartInfo.nOptionIndex;
			infoList.push_back( info );
		}

		SendCashShopCadge( receiver.c_str(), memo.c_str(), m_BuyItemNow.selectedSN, infoList, 0 );
	}
	else
	{
		if( type == eCashUnit_BuyItemNow )
		{
			TCashShopPackageInfo info;
			info.nItemSN = m_BuyItemNow.selectedSN;
			info.nItemID = m_BuyItemNow.ability;
			info.nOptionIndex = m_BuyItemNow.nOptionIndex;
			infoList.push_back( info );
		}
		else if( type == eCashUnit_PreviewCart || type == eCashUnit_Cart )
		{
			const CART_ITEM_LIST* pList = NULL;
			if( type == eCashUnit_PreviewCart )
				pList = GetPreviewCartList( m_PreviewCartClassId );
			else if( type == eCashUnit_Cart )
				pList = &m_CartList;

			CART_ITEM_LIST::const_iterator iter = pList->begin();
			for( ; iter != pList->end(); ++iter )
			{
				const SCashShopCartItemInfo& ItemInfo = *iter;
				TCashShopPackageInfo info;
				info.nItemSN = ItemInfo.selectedSN;
				info.nItemID = ItemInfo.ability;
				info.nOptionIndex = ItemInfo.nOptionIndex;
				infoList.push_back( info );
			}
		}

		SendCashShopCadge( receiver.c_str(), memo.c_str(), 0, infoList, 0 );
	}
}

void CDnCashShopTask::OnRecvCashShopCadge( SCCashShopCadge* pPacket )
{
	if( pPacket == NULL )
		return;

	if( pPacket->nRet == ERROR_NONE )
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4929 ) ); // UISTRING : Gift request sent successfully.
	}
	else if( pPacket->nRet == 103339 )
	{
		int nCadgeMaxCountOneDay = (int)CGlobalWeightIntTable::GetInstance().GetValue( CGlobalWeightIntTable::CadgeMaxCountOneDay );
		std::wstring str = FormatW( GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4913 ),  nCadgeMaxCountOneDay );
		GetInterface().MessageBox( str.c_str() );
	}
	else
	{
		GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4930 ) ); // UISTRING : Failed to send gift request. Please try again later.
	}
}

bool CDnCashShopTask::SetCadgeItemList( int nPackageSN, CART_ITEM_LIST& itemList )
{
	if( static_cast<int>( itemList.size() ) == 0 )
		return false;

	m_BuyItemNow.Clear();
	if( IsPackageItem( nPackageSN ) )
	{
		m_GiftPackageItemList.clear();

		const std::vector<CASHITEM_SN>* pPackageItemList = GetInPackageItemList( nPackageSN );
		if( pPackageItemList != NULL )
		{
			std::vector<CASHITEM_SN>::const_iterator iter = pPackageItemList->begin();
			for( ;iter != pPackageItemList->end(); ++iter )
			{
				CASHITEM_SN curSn = *iter;
				const SCashShopItemInfo* pCurInfo = GetCashShopTask().GetItemInfo( curSn );
				if( pCurInfo )
				{
					SCashShopCartItemInfo cartInfo;
					cartInfo.presentSN = cartInfo.selectedSN = pCurInfo->sn;
					cartInfo.isPackage = true;
					std::list<SCashShopCartItemInfo>::iterator iter2 = itemList.begin();
					for( ; iter2 != itemList.end(); iter2++ )
					{
						if( pCurInfo->sn == (*iter2).presentSN )
						{
							cartInfo.ability = (*iter2).ability;
							cartInfo.nOptionIndex = (*iter2).nOptionIndex;
							break;
						}
					}

					m_GiftPackageItemList.push_back( cartInfo );
				}
			}
		}

		m_BuyItemNow.isPackage = true;
		m_BuyItemNow.selectedSN = nPackageSN;
		m_BuyItemNow.presentSN = nPackageSN;
	}
	else
	{
		m_CadgeList.clear();
		m_CartList.clear();
		std::list<SCashShopCartItemInfo>::iterator iter = itemList.begin();
		for( ; iter != itemList.end(); iter++ )
		{
			m_CadgeList.push_back( (*iter) );
			m_CartList.push_back( (*iter) );
		}
	}

	return true;
}

void CDnCashShopTask::SetCadgeListToCartList()
{
	if( static_cast<int>( m_CadgeList.size() ) == 0 )
		return;

	std::list<SCashShopCartItemInfo>::iterator iter = m_CadgeList.begin();
	for( ; iter != m_CadgeList.end(); iter++ )
	{
		m_CartList.push_back( (*iter) );
	}

	m_CadgeList.clear();
}

#endif // PRE_ADD_CADGE_CASH

bool CDnCashShopTask::HasPermanentItem(eCashUnitType type) const
{
	if (type == eCashUnit_Cart)
	{
		CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
		for (;iter != m_CartList.end(); ++iter)
		{
			const SCashShopCartItemInfo& info = *iter;
			const SCashShopItemInfo* pItemInfo = GetItemInfo(info.selectedSN);
			if (pItemInfo)
			{
				if (pItemInfo->period == -1)
					return true;
			}
		}
	}
	else if (type == eCashUnit_BuyItemNow || type == eCashUnit_Package)
	{
		const SCashShopItemInfo* pItemInfo = GetItemInfo(m_BuyItemNow.selectedSN);
		if (pItemInfo)
		{
			if (pItemInfo->period == -1)
				return true;
		}
	}
	else if (type == eCashUnit_PreviewCart)
	{
		const CART_ITEM_LIST* pItemList = GetPreviewCartList(m_PreviewCartClassId);//m_PreviewCartList[m_PreviewCartClassId - 1];
		if (pItemList)
		{
			CART_ITEM_LIST::const_iterator iter = pItemList->begin();
			for (;iter != pItemList->end(); ++iter)
			{
				const SCashShopCartItemInfo& info = *iter;
				const SCashShopItemInfo* pItemInfo = GetItemInfo(info.selectedSN);
				if (pItemInfo)
				{
					if (pItemInfo->period == -1)
						return true;
				}
			}
		}
	}

	return false;
}

void CDnCashShopTask::RequestCashShopBuyItemNow(bool bBuyReserveMoney)
{
#ifdef PRE_ADD_VIP
	if (IsVIPItem(m_BuyItemNow.ability))
	{
		m_pCashShopDlg->OpenBuyCart(false);
		m_bBuying = true;

		SendCashShopVIPBuy(m_BuyItemNow.selectedSN);
	}
	else
#endif // PRE_ADD_VIP
	if (IsPackageItem(m_BuyItemNow.selectedSN))
	{
		std::vector<TCashShopPackageInfo> infoList;

		const CART_ITEM_LIST& packageItemList = m_BuyPackageItemList;
		CART_ITEM_LIST::const_iterator iter = packageItemList.begin();
		for (;iter != packageItemList.end(); ++iter)
		{
			const SCashShopCartItemInfo& cartInfo = *iter;

			TCashShopPackageInfo info;
			info.nItemSN = cartInfo.selectedSN;
			info.nItemID = cartInfo.ability;
			info.nOptionIndex = cartInfo.nOptionIndex;
			infoList.push_back(info);
		}

		m_pCashShopDlg->OpenBuyCart(false);
		m_bBuying = true;

		char cPaymentRules = Cash::PaymentRules::None;

		if( bBuyReserveMoney )
			cPaymentRules = Cash::PaymentRules::Petal;

#ifdef PRE_ADD_CASH_REFUND
		SendCashShopPackageBuy(eCashUnit_Package, cPaymentRules, m_BuyItemNow.selectedSN, infoList, bDirectMoveCashInven);
#else
		SendCashShopPackageBuy(eCashUnit_Package, cPaymentRules, m_BuyItemNow.selectedSN, infoList);
#endif
	}
	else
	{
		std::vector<TCashShopInfo> infoList;

		TCashShopInfo info;
		info.nItemID = m_BuyItemNow.ability;
		info.nItemSN = m_BuyItemNow.selectedSN;
		info.cSlotIndex = m_BuyItemNow.id;
		info.nOptionIndex = m_BuyItemNow.nOptionIndex;
		infoList.push_back(info);

		m_pCashShopDlg->OpenBuyCart(false);
		m_bBuying = true;

		char cPaymentRules = Cash::PaymentRules::None;

		if( bBuyReserveMoney )
			cPaymentRules = Cash::PaymentRules::Petal;

#ifdef PRE_ADD_CASH_REFUND
		SendCashShopBuy(eCashUnit_BuyItemNow, cPaymentRules, infoList, bDirectMoveCashInven);
#else
		SendCashShopBuy(eCashUnit_BuyItemNow, cPaymentRules, infoList);
#endif
	}
#ifdef PRE_ADD_SALE_COUPON
	m_nApplyCouponSN = 0;
#endif // PRE_ADD_SALE_COUPON
}

bool CDnCashShopTask::IsCartPurchasble(eCashUnitType type, int classIdx) const
{
	if (type == eCashUnit_Cart)
	{
		if (m_CartList.size() > CARTLISTMAX)
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_CART_UNABLE_BUY_COUNT, false);
			return false;
		}
	}
	else if (type == eCashUnit_PreviewCart)
	{
		if( classIdx == PET_CLASS_ID )
		{
			if( (int)m_PetPreviewCartList.size() > PREVIEWCARTLISTMAX )
			{
				m_pCashShopDlg->HandleCashShopError( eERRCS_PREVIEWCART_UNABLE_BUY_COUNT, false );
				return false;
			}
			return true;
		}
		const int index = GetClassArrayIndex(classIdx);
		if (index < 0)
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_CART_NOT_EXIST, true);
			return false;
		}

		if ((int)m_PreviewCartList[index].size() > PREVIEWCARTLISTMAX)
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_UNABLE_BUY_COUNT, false);
			return false;
		}
	}

	return true;
}

void CDnCashShopTask::RequestCashShopBuyCart(bool bBuyReserveMoney)
{
	std::vector<TCashShopInfo> infoList;

	int nRebirthCount = 0;
	CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
	for (; iter != m_CartList.end(); ++iter)
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		TCashShopInfo info;
		info.nItemID = cartItem.ability;

		if (HandleItemPurchasable(cartItem.selectedSN, cartItem.ability) == false)
			return;

		info.nItemSN = cartItem.selectedSN;
		info.cSlotIndex = cartItem.id;
		info.nOptionIndex = cartItem.nOptionIndex;
		infoList.push_back(info);
	}

	m_pCashShopDlg->OpenBuyCart(false);
	m_bBuying = true;
	char cPaymentRules = Cash::PaymentRules::None;

	if( bBuyReserveMoney )
		cPaymentRules = Cash::PaymentRules::Petal;
#ifdef PRE_ADD_CASH_REFUND
	SendCashShopBuy(eCashUnit_Cart, cPaymentRules, infoList, bDirectMoveCashInven);
#else
	SendCashShopBuy(eCashUnit_Cart, cPaymentRules, infoList);
#endif
}

void CDnCashShopTask::RequestCashShopBuyPreviewCart(bool bBuyReserveMoney)
{
	std::vector<TCashShopInfo> infoList;

	CART_ITEM_LIST* pItemList = NULL;
	if( m_PreviewCartClassId == PET_CLASS_ID )
	{
		pItemList = &m_PetPreviewCartList;
	}
	else
	{
		const int index = GetClassArrayIndex( m_PreviewCartClassId );
		if( index < 0 )
		{
			_ASSERT(0);
			return;
		}

		pItemList = &m_PreviewCartList[index];
	}

	if( pItemList == NULL || pItemList->size() == 0 ) return;

	CART_ITEM_LIST::const_iterator iter = pItemList->begin();
	for( ; iter != pItemList->end(); ++iter )
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		TCashShopInfo info;
		info.nItemID = cartItem.ability;
		info.nItemSN = cartItem.selectedSN;
		info.cSlotIndex = cartItem.id;
		info.nOptionIndex = cartItem.nOptionIndex;
		infoList.push_back( info );
	}

	m_pCashShopDlg->OpenBuyCart(false);
	m_bBuying = true;

	char cPaymentRules = Cash::PaymentRules::None;

	if( bBuyReserveMoney )
		cPaymentRules = Cash::PaymentRules::Petal;

#ifdef PRE_ADD_CASH_REFUND
	SendCashShopBuy(eCashUnit_PreviewCart, cPaymentRules, infoList, bDirectMoveCashInven);
#else
	SendCashShopBuy(eCashUnit_PreviewCart, cPaymentRules, infoList);
#endif
}


void CDnCashShopTask::OnRecvCashShopBuy(SCCashShopBuy* pPacket)
{
	if(!m_pCashShopDlg)
		return;

#ifdef PRE_ADD_INSTANT_CASH_BUY
	CDnInstantCashShopBuyDlg* pDnInstantCashShopBuyDlg = GetInterface().GetInstantCashShopBuyDlg();
	if( !m_pCashShopDlg->IsShow() && pDnInstantCashShopBuyDlg && pDnInstantCashShopBuyDlg->IsShow() )
	{
		pDnInstantCashShopBuyDlg->OnRecvInstantBuyResult( pPacket );
		return;
	}
#endif // PRE_ADD_INSTANT_CASH_BUY

	if (pPacket->cType == eCashUnit_Cart || pPacket->cType == eCashUnit_BuyItemNow)
		ClearCartItems(false);
	else if (pPacket->cType == eCashUnit_PreviewCart)
	{
		ClearPreviewCart(m_PreviewCartClassId);
		if( m_PreviewCartClassId != PET_CLASS_ID )
			m_pCashShopDlg->ClearPreviewCart(m_PreviewCartClassId);
	}

	m_pCashShopDlg->DisableAllDlgs(false, L"");

	switch (pPacket->nRet){
	case ERROR_NONE:
		{
#ifdef PRE_ADD_CASHSHOP_CREDIT
			m_UserCredit = pPacket->nNxACredit;
			m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
			m_UserCash = pPacket->nCashAmount;
			m_UserReserve = pPacket->nReserveAmount;
			m_pCashShopDlg->UpdateCashReserveAmount();

#ifdef PRE_ADD_CASH_REFUND
			if (IsCashShopMode(eCSMODE_REFUND))
			{
				int i = 0, itemCount = 0;
				int totalInvenItemCount = m_RefundInvenItemCount[eRITYPE_NORMAL] + m_RefundInvenItemCount[eRITYPE_PACKAGE];

				for (; i < PREVIEWCARTLISTMAX; ++i)
				{
					TPaymentItemInfo& packetInfo = pPacket->ItemList[i];
					if (AddItemToRefundInven(packetInfo, totalInvenItemCount + i))
						itemCount++;
				}
				m_RefundInvenItemCount[eRITYPE_NORMAL] += itemCount;
			}
#endif
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4700 )); // UISTRING : Purchase succeeded

#ifdef PRE_ADD_LIMITED_CASHITEM
			SendCashShopLimitedItemList();
#endif // PRE_ADD_LIMITED_CASHITEM
		}
		break;
	case 558:
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4839 )); // UISTRING : Your cart contains an item that cannot be purchased with Nx Credit
		}
		break;
	default:
		{
			std::wstring str;
			GetInterface().GetServerMessage(str, pPacket->nRet);

			if (pPacket->cCount > 0)
			{
				int i = 0;
				for (; i < pPacket->cCount; ++i)
				{
					if (i == 0)
						str.append(L"\n");

					const SCashShopItemInfo* pItemInfo = GetItemInfo(pPacket->BuyList[i].nItemSN);
					if (pItemInfo)
					{
						std::wstring temp;
						temp = FormatW(L"%s / ", pItemInfo->nameString.c_str());
						str.append(temp.c_str());
					}
				}
			}

			GetInterface().MessageBox(str.c_str(), MB_OK);
		}
		break;
	}

#ifdef PRE_ADD_SALE_COUPON
	m_pCashShopDlg->CheckShowSaleTabButton();	// Check the discount tab.
#endif // PRE_ADD_SALE_COUPON
}

void CDnCashShopTask::OnRecvCashShopSendPackageGift(SCCashShopPackageGift* pPacket)
{
	if (pPacket->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_CASHSHOP_CREDIT
		m_UserCredit = pPacket->nNxACredit;
		m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
		m_UserCash = pPacket->nCashAmount;
		m_UserReserve = pPacket->nReserveAmount;

		const SPackagePreviewUnit* pUnit = GetClassPackagePreviewUnit(m_PreviewCartClassId);
		if (pUnit && pUnit->previewType == ePreview_Package && pUnit->packageSN == pPacket->nPackageSN)
		{
			ClearPreviewCart(m_PreviewCartClassId);
			m_pCashShopDlg->ClearPreviewCart(m_PreviewCartClassId);
		}

		if (m_pCashShopDlg)
			m_pCashShopDlg->UpdateCashReserveAmount();

		m_pCashShopDlg->ShowGiftPayDlg(false, eCashUnit_Gift_Package);

#ifdef PRE_ADD_LIMITED_CASHITEM
		SendCashShopLimitedItemList();
#endif // PRE_ADD_LIMITED_CASHITEM

		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4787 )); // UISTRING : Gift sent
	}
	else
	{
		GetInterface().ServerMessageBox(pPacket->nRet);
	}
}

void CDnCashShopTask::OnRecvCashShopSendGift(SCCashShopGift* pPacket)
{
	if (pPacket->cType == eCashUnit_Gift_Cart || pPacket->cType == eCashUnit_GiftItemNow)
		ClearCartItems(false);
	else if (pPacket->cType == eCashUnit_Gift_PreviewCart)
	{
		ClearPreviewCart(m_PreviewCartClassId);
		m_pCashShopDlg->ClearPreviewCart(m_PreviewCartClassId);
	}

	m_pCashShopDlg->DisableAllDlgs(false, L"");

	if (pPacket->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_CASHSHOP_CREDIT
		m_UserCredit = pPacket->nNxACredit;
		m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
		m_UserCash = pPacket->nCashAmount;
		m_UserReserve = pPacket->nReserveAmount;

		if (m_pCashShopDlg)
			m_pCashShopDlg->UpdateCashReserveAmount();

#ifdef PRE_ADD_LIMITED_CASHITEM
		SendCashShopLimitedItemList();
#endif // PRE_ADD_LIMITED_CASHITEM

		GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4787 )); // UISTRING : Gift sent
	}
	else
	{
		std::wstring str;
		GetInterface().GetServerMessage(str, pPacket->nRet);

		if (pPacket->cCount > 0)
		{
			int i = 0;
			for (; i < pPacket->cCount; ++i)
			{
				if (i == 0)
					str.append(L"\n");

				const SCashShopItemInfo* pItemInfo = GetItemInfo(pPacket->GiftList[i].nItemSN);
				if (pItemInfo)
				{
					std::wstring temp;
					temp = FormatW(L"%s / ", pItemInfo->nameString.c_str());
					str.append(temp.c_str());
				}
			}
		}

		GetInterface().MessageBox(str.c_str(), MB_OK);
	}
}

const CDnCashShopTask::SPackagePreviewUnit* CDnCashShopTask::GetClassPackagePreviewUnit(int classId) const
{
	const int index = GetClassArrayIndex(classId);
	if (index < 0)
		return NULL;

	return &(m_PackagePreviewUnit[index]);
}

void CDnCashShopTask::OnRecvCashShopPackageBuy(SCCashShopPackageBuy* pPacket)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");

	switch(pPacket->nRet)
	{
	case ERROR_NONE:
		{
#ifdef PRE_ADD_CASHSHOP_CREDIT
			m_UserCredit = pPacket->nNxACredit;
			m_UserPrepaid = pPacket->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
			m_UserCash = pPacket->nCashAmount;
			m_UserReserve = pPacket->nReserveAmount;
			const SPackagePreviewUnit* pUnit = GetClassPackagePreviewUnit(m_PreviewCartClassId);
			if (pUnit && pUnit->previewType == ePreview_Package && pUnit->packageSN == pPacket->nPackageSN)
			{
				ClearPreviewCart(m_PreviewCartClassId);
				m_pCashShopDlg->ClearPreviewCart(m_PreviewCartClassId);
			}

			if (m_pCashShopDlg)
				m_pCashShopDlg->UpdateCashReserveAmount();

#ifdef PRE_ADD_CASH_REFUND
			if (IsCashShopMode(eCSMODE_REFUND))
			{
				int totalInvenItemCount = m_RefundInvenItemCount[eRITYPE_NORMAL] + m_RefundInvenItemCount[eRITYPE_PACKAGE];
				if (AddPackageItemToRefundInven(pPacket->PackageItem, totalInvenItemCount))
					++m_RefundInvenItemCount[eRITYPE_PACKAGE];
			}
#endif

			m_pCashShopDlg->OpenBuyCart(false);
#ifdef PRE_ADD_LIMITED_CASHITEM
			SendCashShopLimitedItemList();
#endif // PRE_ADD_LIMITED_CASHITEM

			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4700 )); // UISTRING : Purchase succeeded
		}
		break;
	case 558:
		{
			GetInterface().MessageBox(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4839 )); // UISTRING : Your cart contains an item that cannot be purchased with Nx Credit
		}
		break;
	default:
		{
			GetInterface().ServerMessageBox(pPacket->nRet);
		}
		break;
	}
#ifdef PRE_ADD_SALE_COUPON
	m_pCashShopDlg->CheckShowSaleTabButton();	// Check the discount tab.
#endif // PRE_ADD_SALE_COUPON
}

void CDnCashShopTask::OnRecvCashShopBalanceInquiry(SCCashShopBalanceInquiry* pData)
{
	if (pData->nRet == ERROR_NONE)
	{
#ifdef PRE_ADD_CASHSHOP_CREDIT
		m_UserCredit = pData->nNxACredit;
		m_UserPrepaid = pData->nNxAPrepaid;
#endif // PRE_ADD_CASHSHOP_CREDIT
		m_UserCash = pData->nCash;
		m_UserReserve = pData->nPetal;

		if (m_pCashShopDlg)
			m_pCashShopDlg->UpdateCashReserveAmount();
	}
	else
	{
		GetInterface().ServerMessageBox(pData->nRet);
	}
}

#ifdef PRE_ADD_CADGE_CASH
void CDnCashShopTask::RequestCashShopSendGiftCart(const std::wstring& receiverName, const std::wstring& memo, int nCadgeMailID)
#else // PRE_ADD_CADGE_CASH
void CDnCashShopTask::RequestCashShopSendGiftCart(const std::wstring& receiverName, const std::wstring& memo)
#endif // PRE_ADD_CADGE_CASH
{
	std::vector<TCashShopInfo> infoList;

	CART_ITEM_LIST::const_iterator iter = m_CartList.begin();
	for (; iter != m_CartList.end(); ++iter)
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		TCashShopInfo info;
		info.nItemID = cartItem.ability;
		info.nItemSN = cartItem.selectedSN;
		info.nOptionIndex = cartItem.nOptionIndex;
		infoList.push_back(info);
	}

	m_bBuying = true;
	m_pCashShopDlg->ShowGiftPayDlg(false, eCashUnit_Gift_Cart);
#ifdef PRE_ADD_CADGE_CASH
	SendCashShopGift(eCashUnit_Gift_Cart, receiverName.c_str(), memo.c_str(), infoList, nCadgeMailID);
#else // PRE_ADD_CADGE_CASH
	SendCashShopGift(eCashUnit_Gift_Cart, receiverName.c_str(), memo.c_str(), infoList, 0);
#endif // PRE_ADD_CADGE_CASH
}

void CDnCashShopTask::RequestCashShopSendGiftPreviewCart(const std::wstring& receiverName, const std::wstring& memo)
{
	std::vector<TCashShopInfo> infoList;

	CART_ITEM_LIST* pItemList = NULL;
	if( m_PreviewCartClassId == PET_CLASS_ID )
	{
		pItemList = &m_PetPreviewCartList;
	}
	else
	{
		const int index = GetClassArrayIndex( m_PreviewCartClassId );
		if( index < 0 )
		{
			_ASSERT(0);
			return;
		}

		pItemList = &m_PreviewCartList[index];
	}

	if( pItemList == NULL || pItemList->size() == 0 ) return;

	CART_ITEM_LIST::const_iterator iter = pItemList->begin();
	for( ; iter != pItemList->end(); ++iter )
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		TCashShopInfo info;
		info.nItemID = cartItem.ability;
		info.nItemSN = cartItem.selectedSN;
		info.nOptionIndex = cartItem.nOptionIndex;
		infoList.push_back( info );
	}

	m_bBuying = true;
	m_pCashShopDlg->ShowGiftPayDlg(false, eCashUnit_Gift_PreviewCart);

	SendCashShopGift(eCashUnit_Gift_PreviewCart, receiverName.c_str(), memo.c_str(), infoList, 0);
}

static bool CompareCashShopItemBasicDescending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
#define JOB_WEIGHT		1000000
#define JOB_ONLY_WEIGHT 9000000
	int s1Priority = s1->priority;
	int s2Priority = s2->priority;
	if (CDnActor::s_hLocalActor)
	{
		int classId = CDnActor::s_hLocalActor->GetClassID();
		if (CommonUtil::IsValidCharacterClassId(classId) == false)
			return false;

		if (s1->bEnableJob[classId])
			s1Priority += (s1->bOnlyOneClassEnable) ? JOB_ONLY_WEIGHT : JOB_WEIGHT;
		if (s2->bEnableJob[classId])
			s2Priority += (s2->bOnlyOneClassEnable) ? JOB_ONLY_WEIGHT : JOB_WEIGHT;
	}

	if( s1Priority > s2Priority ) return true;
	else if( s1Priority < s2Priority ) return false;

	return false;
}

static bool CompareCashShopItemLevelDescending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	if( s1->levelLimit > s2->levelLimit ) return true;
	else if( s1->levelLimit < s2->levelLimit ) return false;
	return false;
}
static bool CompareCashShopItemLevelAscending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	if( s1->levelLimit < s2->levelLimit ) return true;
	else if( s1->levelLimit > s2->levelLimit ) return false;
	return false;
}

static bool CompareCashShopItemNameAscending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	if( __wcsicmp_l( s1->nameString.c_str(), s2->nameString.c_str() ) > 0 ) return true;
	else if( __wcsicmp_l( s1->nameString.c_str(), s2->nameString.c_str() ) < 0 ) return false;
	return false;
}

static bool CompareCashShopItemNameDescending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	if( __wcsicmp_l( s1->nameString.c_str(), s2->nameString.c_str() ) < 0 ) return true;
	else if( __wcsicmp_l( s1->nameString.c_str(), s2->nameString.c_str() ) > 0 ) return false;
	return false;
}

static bool CompareCashShopReleaseNewDescending( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	if( s1->sn > s2->sn ) return true;
	else if( s1->sn < s1->sn ) return false;
	return false;
}

static bool CompareCashShopMyClass( SCashShopItemInfo* s1, SCashShopItemInfo* s2 )
{
	int classId = CDnActor::s_hLocalActor->GetClassID();
	if (CommonUtil::IsValidCharacterClassId(classId) == false)
		return false;

	if( s1->bEnableJob[classId] && s2->bEnableJob[classId] == false )
		return true;
	else if( s1->bEnableJob[classId] && s2->bEnableJob[classId] )
		return (s1->bOnlyOneClassEnable && s2->bOnlyOneClassEnable == false);

	return false;
}

bool CDnCashShopTask::Sort(eCashShopSubCatType catType, eCashShopEtcSortType sortType)
{
	static bool (*fp[SORT_MAX])( SCashShopItemInfo* s1, SCashShopItemInfo* s2 ) = {
		CompareCashShopItemBasicDescending,
		CompareCashShopItemLevelDescending,
		CompareCashShopItemLevelAscending,
		CompareCashShopItemNameDescending,
		CompareCashShopItemNameAscending,
		//CompareCashShopReleaseNewDescending,
		CompareCashShopMyClass
	};

	CS_INFO_LIST* pList = GetSubCatItemList(catType);
	if (pList)
	{
		std::sort(pList->begin(), pList->end(), fp[sortType]);
		return true;
	}

	return false;
}

bool CDnCashShopTask::IsAttachablePackageItem(CASHITEM_SN sn) const
{
	const std::vector<CASHITEM_SN>* pList = GetInPackageItemList(sn);
	if (pList)
	{
		std::vector<CASHITEM_SN>::const_iterator iter = pList->begin();
		for (; iter != pList->end(); ++iter)
		{
			CASHITEM_SN curSn = *iter;
			const SCashShopItemInfo* pItemInfo = GetItemInfo(curSn);
			if (pItemInfo == NULL)
			{
#ifdef _RDEBUG
				std::wstring str;
				str = FormatW(L"There is NO package contents item in CashcommodityTable(sn:%d). CHECK Package(sn:%d)", curSn, sn);
				GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2, str.c_str(), textcolor::RED );
#endif
				return false;
			}

			eItemTypeEnum curType = CDnItem::GetItemType(pItemInfo->presentItemId);
			if (pItemInfo && (curType == ITEMTYPE_WEAPON || curType == ITEMTYPE_PARTS || curType == ITEMTYPE_HAIRDYE || 
							  curType == ITEMTYPE_SKINDYE || curType == ITEMTYPE_HAIRDRESS || curType == ITEMTYPE_EYEDYE ||
							  curType == ITEMTYPE_FACIAL))
				return true;
		}
	}

	return false;
}

void CDnCashShopTask::GetJobUsable(std::vector<int>& usableClassIds, ITEMCLSID itemId) const
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
	if( !pSox || !pSox->IsExistItem( itemId ) )
		return;

	char *szPermitStr = pSox->GetFieldFromLablePtr( itemId, "_NeedJobClass" )->GetString();
	int nJobIndex;
	for( int i=0;; i++ ) {
		std::string strValue = _GetSubStrByCountSafe( i, szPermitStr, ';' );
		if( strValue.size() == 0 ) break;
		nJobIndex = atoi(strValue.c_str());
		if (nJobIndex == 0)
		{
			usableClassIds.push_back(nJobIndex);
			return;
		}
		else if (nJobIndex < 0)
		{
			continue;
		}

		usableClassIds.push_back(nJobIndex);
	}
}

int CDnCashShopTask::GetAttachableClass(int classId, ITEMCLSID itemId) const
{
	std::vector<int> usableClassIds;
	GetCashShopTask().GetJobUsable(usableClassIds, itemId);
	if (usableClassIds.size() != 0)
	{
		if (usableClassIds.size() == 1 && usableClassIds.front() == 0)
			return classId;

		if (std::find(usableClassIds.begin(), usableClassIds.end(), classId) == usableClassIds.end())
			return usableClassIds.front();

		return classId;
	}

	return 0;
}

#ifdef PRE_ADD_LIMITED_CASHITEM
void CDnCashShopTask::RequestCashShopRefresh()
{
	SendCashShopLimitedItemList();
}

void CDnCashShopTask::OnRecvCashShopLimitedItemInfo(SCQuantityLimitedItem * pPacket)
{
	bool bUpdate = false;
	int nCount = 0;

	if( pPacket->cIsLast )
		nCount = pPacket->cCount;
	else
		nCount = LimitedCashItem::Common::DefaultPacketCount;

	for( int i=0; i<nCount; i++ )
	{
		std::map<CASHITEM_SN, SCashItemLimitInfo>::iterator iter = m_CashItemLimitInfo.find( pPacket->Limited[i].nProductID );
		if( iter != m_CashItemLimitInfo.end() )
		{
			SCashItemLimitInfo& info = (*iter).second;
			info.nSaleCount = pPacket->Limited[i].nSaleCount;
			if( info.nSaleCount < 0)
				info.nSaleCount = 0;
			bUpdate = true;
		}
	}

	if( bUpdate && m_pCashShopDlg )
		m_pCashShopDlg->UpdateCurrentGoodsDlg();

}

void CDnCashShopTask::OnRecvCashShopChangedLimitedItemInfo(SCChangedQuantityLimitedItem * pPacket)
{
	if( pPacket->cCount >= LimitedCashItem::Common::DefaultPacketCount )
		return;

	bool bUpdate = false;

	for( int i=0; i<pPacket->cCount; i++ )
	{
		std::map<CASHITEM_SN, SCashItemLimitInfo>::iterator iter = m_CashItemLimitInfo.find( pPacket->Changed[i].nProductID );
		if( iter != m_CashItemLimitInfo.end() )
		{
			SCashItemLimitInfo& info = (*iter).second;
			info.nMaxCount = pPacket->Changed[i].nMaxCount;
			bUpdate = true;
		}
	}

	if( bUpdate && m_pCashShopDlg )
	{
		m_pCashShopDlg->UpdateCurrentGoodsDlg();

#ifdef PRE_ADD_CASHSHOP_RENEWAL
		//UpdateIngameBannerItem();
#endif // PRE_ADD_CASHSHOP_RENEWAL

	}
}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

void CDnCashShopTask::AttachPartsToPreview(CASHITEM_SN sn, ITEMCLSID itemId)
{
	m_pCashShopDlg->SwapPreview( true );
	if (IsPackageItem(sn))
	{
		int attachableClass = GetAttachableClass(m_PreviewCartClassId, itemId);
		if (attachableClass < 0)
			return;

		if( attachableClass != m_PreviewCartClassId && !CommonUtil::IsValidCharacterClassId(m_PreviewCartClassId) )
			ChangePreviewAvatar(attachableClass);

		const int idx = GetClassArrayIndex(attachableClass);
		if (idx < 0)
			return;

		if (m_PackagePreviewUnit[idx].packageSN == sn && m_PreviewCartClassId == attachableClass)
		{
			m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_ALREADY_EXIST, false);
			return;
		}

		if (IsAttachablePackageItem(sn) == false)
			return;

		const std::vector<CASHITEM_SN>* pList = GetInPackageItemList(sn);
		if (pList)
		{
			if (ChangePreviewAvatarWhenAttachPackageItem(m_PreviewCartClassId, sn) == false)
				return;

			CASHITEM_SN realPreviewParts[CDnParts::CashPartsTypeEnum_Amount] = {0,};
			CASHITEM_SN realPreviewWeapon[CDnWeapon::EquipTypeEnum_Amount] = {0,};
			std::vector<CASHITEM_SN>::const_iterator iter = pList->begin();
			for (; iter != pList->end(); ++iter)
			{
				CASHITEM_SN curSn = *iter;
				SCashShopItemInfo* pItemInfo = GetItemInfo(curSn);
				if (pItemInfo)
				{
					const ITEMCLSID& itemId = pItemInfo->presentItemId;
					eItemTypeEnum type = CDnItem::GetItemType(itemId);
					if (type == ITEMTYPE_PARTS)
					{
						CDnParts::PartsTypeEnum partsIdx = CDnParts::GetPartsType(itemId);
						if (partsIdx < 0 || partsIdx >= CDnParts::CashPartsTypeEnum_Amount)
						{
							_ASSERT(0);
							continue;
						}

						if ((realPreviewParts[partsIdx] > 0) && (realPreviewParts[partsIdx] < curSn))
							continue;

						m_pCashShopDlg->AttachPartsToPreview(curSn, pItemInfo->presentItemId, true);
						realPreviewParts[partsIdx] = curSn;
					}
					else if (type == ITEMTYPE_WEAPON)
					{
						CDnWeapon::EquipTypeEnum weaponIdx = CDnWeapon::GetEquipType(itemId);
						if (weaponIdx < 0 || weaponIdx >= CDnWeapon::EquipTypeEnum_Amount)
						{
							_ASSERT(0);
							continue;
						}

						if ((realPreviewWeapon[weaponIdx] > 0) && (realPreviewWeapon[weaponIdx] < curSn))
							continue;

						m_pCashShopDlg->AttachPartsToPreview(curSn, pItemInfo->presentItemId);
						realPreviewWeapon[weaponIdx] = curSn;
					}
					else if (type == ITEMTYPE_HAIRDRESS)
					{
						m_pCashShopDlg->AttachPartsToPreview(curSn, pItemInfo->presentItemId);
					}
				}
			}

			SetPackagePreview(m_PreviewCartClassId, ePreview_Package, sn);
		}
	}
	else
	{
		if (CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetClassID() == m_PreviewCartClassId)
		{
			MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(CDnActor::s_hLocalActor.GetPointer());
			// Because Hair needs to be checked in NormalParts..
			/* 
			// No check for whether dye or hairstyle is currently applied to the character. Commented out for now since handling only hairstyle here is awkward.
			// It seems better to do this and have the relevant part checked in appropriate code.
			if( pPartsBody->GetParts( CDnParts::Hair ) ) {
				DnPartsHandle hParts = pPartsBody->GetParts( CDnParts::Hair );
				if( hParts ) {
					CDnItem *pItem = static_cast<CDnItem*>(hParts.GetPointer());
					if( pItem ) {
						ITEMCLSID curItemId = pItem->GetClassID();
						DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TITEM );
						if (curItemId == pSox->GetFieldFromLablePtr( itemId, "_TypeParam1" )->GetInteger() )
						{
							m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_ALREADY_EQUIP, false);
							return;
						}
					}
				}
			}
			*/
			//////////////////////////////
			for( int nIndex = CDnParts::CashHelmet; nIndex < CDnParts::CashPartsTypeEnum_Amount; nIndex++)
			{
				DnPartsHandle hParts = pPartsBody->GetCashParts( (CDnParts::PartsTypeEnum)nIndex );
				if( !hParts )
					continue;

				CDnItem *pItem = static_cast<CDnItem*>(hParts.GetPointer());
				if (pItem)
				{
					ITEMCLSID curItemId = pItem->GetClassID();
					if (curItemId == itemId)
					{
						m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_ALREADY_EQUIP, false);
						return;
					}
				}
			}

		}

		int attachableClassId = GetAttachableClass(m_PreviewCartClassId, itemId);
		if (CommonUtil::IsValidCharacterClassId(attachableClassId))
		{
			const SPackagePreviewUnit* pUnit = GetClassPackagePreviewUnit(attachableClassId);
			if (pUnit && pUnit->previewType == ePreview_Package)
			{
				m_pCashShopDlg->ClearPreviewCart(attachableClassId);
				ClearPreviewCart(attachableClassId);
			}
		}

		m_pCashShopDlg->AttachPartsToPreview(sn, itemId);
	}
}

const CART_ITEM_LIST* CDnCashShopTask::GetPreviewCartList(int classId) const
{
	if( classId == 100 )
		return &m_PetPreviewCartList;
	const int index = GetClassArrayIndex(classId);
	if (index < 0)
		return NULL;

	return &(m_PreviewCartList[index]);
}

CART_ITEM_LIST* CDnCashShopTask::GetPreviewCartList(int classId)
{
	if( classId == 100 )
		return &m_PetPreviewCartList;
	const int index = GetClassArrayIndex(classId);
	if (index < 0)
		return NULL;

	return &(m_PreviewCartList[index]);
}

void CDnCashShopTask::ClearPreviewCart(int classId)
{
	if( classId == PET_CLASS_ID )
	{
		m_PetPreviewCartList.clear();
		return;
	}
	const int index = GetClassArrayIndex(classId);
	if (index < 0)
	{
		m_pCashShopDlg->HandleCashShopError(eERRCS_PREVIEWCART_CLEAR_INVALID_CLASS, true);
		return;
	}

	CART_ITEM_LIST& previewList = m_PreviewCartList[index];
	previewList.clear();
	SetPackagePreview(classId, ePreview_Normal, 0);
}

const CART_ITEM_LIST* CDnCashShopTask::GetBuyPreviewCartList() const
{
	return GetPreviewCartList(m_PreviewCartClassId);
}

CART_ITEM_LIST* CDnCashShopTask::GetBuyPreviewCartList()
{
	return GetPreviewCartList(m_PreviewCartClassId);
}

bool CDnCashShopTask::IsCartEmpty(eCashUnitType type, int classId) const
{
	if (type == eCashUnit_Cart || type == eCashUnit_Gift_Cart)
	{
		return m_CartList.empty();
	}
	else if (type == eCashUnit_PreviewCart || type == eCashUnit_Gift_PreviewCart)
	{
		if( classId == PET_CLASS_ID )
			return m_PetPreviewCartList.empty();
		const int index = GetClassArrayIndex(classId);
		if (index >= 0)
			return m_PreviewCartList[index].empty();
	}

	m_pCashShopDlg->HandleCashShopError(eERRCS_CART_NOT_EXIST, true);

	return false;
}

#ifdef PRE_MOD_CASHSHOP_NOETERNITY_TOOLTIP
bool CDnCashShopTask::IsOpenCashShopRecvGiftDlg() const
{
	return m_pCashShopDlg->IsOpenCashShopRecvGiftDlg();
}
#endif

#ifdef PRE_MOD_CASHSHOP_NOETERNITY_TOOLTIP
bool CDnCashShopTask::IsUsingCashShopTooltip(CASHITEM_SN sn, ITEM_SLOT_TYPE type) const
#else
bool CDnCashShopTask::IsUsingCashShopTooltip(CASHITEM_SN sn) const
#endif
{
	if( IsPackageItem( sn ) )	// Pass packages only to view package item tooltips outside the Cash Shop
		return true;

	if (m_bOpened == false)
	{
#ifdef PRE_MOD_CASHSHOP_NOETERNITY_TOOLTIP
		if (type != ST_CASHINSTANCE_ITEM)
			return false;
#else
		return false;
#endif
	}

	bool bNeed = true;

	if (m_pCashShopDlg->IsOpenCashShopInvenDlg())
		bNeed = false;

#ifdef PRE_MOD_CASHSHOP_NOETERNITY_TOOLTIP
	if (IsOpenCashShopRecvGiftDlg())
#else
	if (m_pCashShopDlg->IsOpenCashShopRecvGiftDlg())
#endif
		bNeed = IsPackageItem(sn);

	return bNeed;
}

bool CDnCashShopTask::IsPackageItem(CASHITEM_SN sn) const
{
	const std::vector<CASHITEM_SN>* pPackageList = GetInPackageItemList(sn);
	return (pPackageList != NULL);
}

#ifdef PRE_ADD_VIP
bool CDnCashShopTask::IsVIPItem(ITEMCLSID id) const
{
	eItemTypeEnum type = CDnItem::GetItemType(id);
	return (type == ITEMTYPE_VIP);
}
#endif

void CDnCashShopTask::SetPackagePreview(int classId, ePreviewType type, CASHITEM_SN sn)
{
	int idx = GetClassArrayIndex(classId);
	if (idx < 0)
	{
		_ASSERT(0);
		return;
	}

	m_PackagePreviewUnit[idx].previewType = type;
	m_PackagePreviewUnit[idx].packageSN = sn;
}

int	CDnCashShopTask::GetCashShopDiscountRate(float priceDiscounted, float fixedPrice, float reserveAmount) const
{
	float raw = (1.f - ((priceDiscounted - reserveAmount) / fixedPrice)) * 100.f;
	int convert = (int)raw;
	if (raw - (float)convert >= 0.5f)
		convert += 1;

	return convert;
}

void CDnCashShopTask::DoSetPackageItemAbility(CART_ITEM_LIST& itemList, CASHITEM_SN sn, ITEMCLSID ability, int itemListIndex)
{
	if (itemList.empty())
	{
		_ASSERT(0);
		return;
	}

	// Check with index included because there are duplicate SNs in the package.
	int nIndex = 0;
	CART_ITEM_LIST::iterator iter = itemList.begin();
	for (; iter != itemList.end(); ++iter, ++nIndex)
	{
		SCashShopCartItemInfo& info = *iter;
		if (info.presentSN == sn && itemListIndex == nIndex)
		{
			info.ability = ability;
			break;
		}
	}

	if (iter == itemList.end())
		_ASSERT(0);
}

void CDnCashShopTask::SetPackageItemAbility(bool bGift, CASHITEM_SN sn, ITEMCLSID ability, int itemListIndex)
{
	DoSetPackageItemAbility(bGift ? m_GiftPackageItemList : m_BuyPackageItemList, sn, ability, itemListIndex);
}

bool CDnCashShopTask::IsCostumeItem(ITEMCLSID id) const
{
	eItemTypeEnum type = CDnItem::GetItemType(id);
	return (type == ITEMTYPE_PARTS || type == ITEMTYPE_WEAPON);
}

eRetPreviewChange CDnCashShopTask::ChangePreviewAvatar(int classId,bool bForce)
{
	if (m_pCashShopDlg)
		return m_pCashShopDlg->ChangePreviewAvatar(classId,bForce);

	return eRETPREVIEW_ERROR;
}

void CDnCashShopTask::ChangeVehicleParts(int nClassID,int nPartClassID, bool bDefault)
{
	if (m_pCashShopDlg)
		return m_pCashShopDlg->ChangeVehicleParts(nClassID,nPartClassID,bDefault);
}

bool CDnCashShopTask::ChangePreviewAvatarWhenAttachPackageItem(int classId, CASHITEM_SN sn)
{
	if (CommonUtil::IsValidCharacterClassId(classId) == false || IsValidSN(sn) == false)
		return false;

	const SCashShopItemInfo* pPackageItemInfo = GetItemInfo(sn);
	if (pPackageItemInfo)
	{
		int attachableClassId = GetAttachableClass(classId, pPackageItemInfo->presentItemId);
		if (attachableClassId > 0)
		{
			if (ChangePreviewAvatar(attachableClassId) == eRETPREVIEW_ERROR)
				return false;

			ClearPreviewCart(attachableClassId);
			m_pCashShopDlg->ClearPreviewCart(attachableClassId);

			return true;
		}
	}

	return false;
}

const int CDnCashShopTask::GetClassArrayIndex(int classId) const
{
	if (CommonUtil::IsValidCharacterClassId(classId) == false)
		return -1;

	return classId - 1;
}

CDnCashShopTask::ePreviewType CDnCashShopTask::GetPreviewType(int classId) const
{
	if (classId == PET_CLASS_ID)
		return ePreview_Normal;

	const int index = GetClassArrayIndex(classId);
	if (index < 0)
	{
		_ASSERT(0);
		return ePreview_Normal;
	}

	return m_PackagePreviewUnit[index].previewType;
}

const SCashShopRecvGiftBasicInfo* CDnCashShopTask::GetRecvGiftInfo(INT64 giftDBID) const
{
	RECV_GIFT_LIST::const_iterator iter = m_RecvGiftInfoList.begin();
	for (; iter != m_RecvGiftInfoList.end(); ++iter)
	{
		const SCashShopRecvGiftBasicInfo& info = *iter;
		if (info.giftDBID == giftDBID)
			return &info;
	}

	return NULL;
}

eCashShopError CDnCashShopTask::IsItemGiftable(eCashUnitType type, std::vector<CASHITEM_SN>& unableItemList) const
{
	eCashShopError ret = eERRCS_NONE;
	const CART_ITEM_LIST* pCartList = NULL;
	if (type == eCashUnit_Gift_Cart)
	{
		pCartList = &(GetCartList());
	}
	else if (type == eCashUnit_Gift_PreviewCart)
	{
		pCartList = GetPreviewCartList(m_PreviewCartClassId);
	}
	else if (type == eCashUnit_Gift_Package || type == eCashUnit_GiftItemNow)
	{
		const SCashShopItemInfo* pItemInfo = GetItemInfo(m_BuyItemNow.presentSN);
		if (pItemInfo)
		{
			if (pItemInfo->bGiftUsable == false ||
				IsNotOnSaleItem(pItemInfo->sn))
			{
				unableItemList.push_back(pItemInfo->sn);
				ret = eERRCS_GIFT_UNABLE_NOGIFT_OPTION;
			}
		}

		return ret;
	}

	if (pCartList == NULL)
		return eERRCS_GIFT_UNABLE_INVALID_CART;

	CART_ITEM_LIST::const_iterator iter = pCartList->begin();
	for (; iter != pCartList->end(); ++iter)
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		const SCashShopItemInfo* pItemInfo = GetItemInfo(cartItem.presentSN);
		if (pItemInfo)
		{
			if (pItemInfo->bGiftUsable == false ||
				IsNotOnSaleItem(pItemInfo->sn))
			{
				unableItemList.push_back(pItemInfo->sn);
				ret = eERRCS_GIFT_UNABLE_NOGIFT_OPTION;
			}
		}
	}

	return ret;
}

eCashShopError CDnCashShopTask::IsItemGiftable(eCashUnitType type, int receiverClassId, int receiverLevel, std::vector<CASHITEM_SN>& unableItemList) const
{
	if (CommonUtil::IsValidCharacterClassId(receiverClassId) == false)
		return eERRCS_GIFT_UNABLE_CLASS;

	eCashShopError ret = eERRCS_NONE;

	const CART_ITEM_LIST* pCartList = NULL;
	if (type == eCashUnit_Gift_Cart)
	{
		pCartList = &(GetCartList());
	}
	else if (type == eCashUnit_Gift_PreviewCart)
	{
		pCartList = GetPreviewCartList(m_PreviewCartClassId);
	}
	else if (type == eCashUnit_Gift_Package || type == eCashUnit_GiftItemNow)
	{
		const SCashShopItemInfo* pItemInfo = GetItemInfo(m_BuyItemNow.presentSN);
		if (pItemInfo)
		{
			if ((pItemInfo->bEnableJob[0] == false && pItemInfo->bEnableJob[receiverClassId] == false) ||
				IsNotOnSaleItem(pItemInfo->sn))
			{
				unableItemList.push_back(pItemInfo->sn);
				ret = eERRCS_GIFT_UNABLE_CLASS;
			}
		}

		return ret;
	}

	if (pCartList == NULL)
		return eERRCS_GIFT_UNABLE_INVALID_CART;

	CART_ITEM_LIST::const_iterator iter = pCartList->begin();
	for (; iter != pCartList->end(); ++iter)
	{
		const SCashShopCartItemInfo& cartItem = *iter;
		const SCashShopItemInfo* pItemInfo = GetItemInfo(cartItem.presentSN);
		if (pItemInfo)
		{
			if ((pItemInfo->bEnableJob[0] == false && pItemInfo->bEnableJob[receiverClassId] == false) ||
				IsNotOnSaleItem(pItemInfo->sn))
			{
				unableItemList.push_back(pItemInfo->sn);
				ret = eERRCS_GIFT_UNABLE_CLASS;
			}
		}
	}

	return ret;
}

bool CDnCashShopTask::HandleItemGiftable(eCashUnitType type, int receiverJobId, int receiverLevel)
{
	std::vector<CASHITEM_SN> unableGiftItemList;
	eCashShopError err = IsItemGiftable(type, CDnPlayerState::GetJobToBaseClassID(receiverJobId), receiverLevel, unableGiftItemList);
	if (err != eERRCS_NONE)
	{
		if (unableGiftItemList.empty())
		{
			HandleCashShopError(err, false);
		}
		else
		{
			if (m_pCashShopDlg)
			{
				std::wstring msg;
				if (err == eERRCS_GIFT_UNABLE_CLASS)
				{
					msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4660); // UISTRING : Item cannot be used by your class.
				}
				else
				{
					if (type == eCashUnit_Cart)
						msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4779);	// UISTRING : There is an unpurchasable item in your cart.
					else if (type == eCashUnit_Gift_Cart)
						msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4728);	// UISTRING : There is a non-giftable item in your cart.
					else if (type == eCashUnit_Gift_Package || type == eCashUnit_GiftItemNow)
						msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4574);	// UISTRING : This item cannot be gifted.
					else
						msg = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4575);	// UISTRING : This item cannot be purchased.
				}

				m_pCashShopDlg->ShowGiftFailDlg(true, msg, unableGiftItemList, MB_OK);
			}
			else
			{
				HandleCashShopError(err, false);
			}
			unableGiftItemList.clear();
		}
		return false;
	}

	return true;
}

bool CDnCashShopTask::IsCartItemBuyable(const CART_ITEM_LIST& cartItemList, std::vector<CASHITEM_SN>& unableItemList) const
{
	if (cartItemList.empty() || !CDnActor::s_hLocalActor)
		return false;

	int myClassId = CDnActor::s_hLocalActor->GetClassID();
	if (CommonUtil::IsValidCharacterClassId(myClassId) == false)
		return false;

#ifdef PRE_ADD_VIP
	const CDnVIPDataMgr* pDataMgr = GetInterface().GetLocalPlayerVIPDataMgr();
	if (pDataMgr == NULL)
		return false;
#endif

	bool bRet = true;
	CART_ITEM_LIST::const_iterator iter = cartItemList.begin();
	for (; iter != cartItemList.end(); ++iter)
	{
		const SCashShopCartItemInfo& info = *iter;
		const SCashShopItemInfo* pItemInfo = GetItemInfo(info.presentSN);
		if (pItemInfo)
		{
			if (pItemInfo->bEnableJob[myClassId] == false || IsNotOnSaleItem(pItemInfo->sn))
			{
				unableItemList.push_back(pItemInfo->sn);
				bRet = false;
			}
#ifdef PRE_ADD_VIP
			else if (pItemInfo->bVIPSell)
			{
				const CVIPGradeTable::SVIPGradeUnit* pUnit = CVIPGradeTable::GetInstance().GetValue(pDataMgr->GetCurrentPts());
				if (pUnit == NULL || (pItemInfo->neededVIPLevel > pUnit->level))
				{
					unableItemList.push_back(pItemInfo->sn);
					bRet = false;
				}
			}
#endif
		}
	}

	return bRet;
}

void CDnCashShopTask::HandleCashShopError(eCashShopError code, bool bShowCode, CEtUIDialog* pCallbackDlg)
{
	if (m_pCashShopDlg)
		m_pCashShopDlg->HandleCashShopError(code, bShowCode, pCallbackDlg);
}

bool CDnCashShopTask::IsGiftNeedWarning(eCashUnitType type) const
{
	const CART_ITEM_LIST* pCartList = NULL;
	if (type == eCashUnit_Gift_Cart)
		pCartList = &(GetCartList());
	else if (type == eCashUnit_Gift_PreviewCart)
		pCartList = GetPreviewCartList(m_PreviewCartClassId);

	if (pCartList == NULL)
	{
		if (type == eCashUnit_Gift_Package || type == eCashUnit_GiftItemNow)
		{
			const SCashShopItemInfo* pItemInfo = GetItemInfo(m_BuyItemNow.selectedSN);
			if (pItemInfo)
			{
				eItemTypeEnum itemType = CDnItem::GetItemType(pItemInfo->presentItemId);
				if (itemType == ITEMTYPE_INVENTORY_SLOT || itemType == ITEMTYPE_WAREHOUSE_SLOT)
					return true;
			}
		}
	}
	else
	{
		CART_ITEM_LIST::const_iterator iter = pCartList->begin();
		for (; iter != pCartList->end(); ++iter)
		{
			const SCashShopCartItemInfo& cartItemInfo = *iter;
			const SCashShopItemInfo* pItemInfo = GetItemInfo(cartItemInfo.selectedSN);
			if (pItemInfo)
			{
				eItemTypeEnum itemType = CDnItem::GetItemType(pItemInfo->presentItemId);
				if (itemType == ITEMTYPE_INVENTORY_SLOT || itemType == ITEMTYPE_WAREHOUSE_SLOT)
					return true;
			}
		}
	}

	return false;
}

int CDnCashShopTask::GetValidAbilityCount(const std::vector<ITEMCLSID>& abilityList) const
{
	return (int)abilityList.size();
}

bool CDnCashShopTask::IsRecvGiftReplyAble(int payMethodCode) const
{
	switch (payMethodCode)
	{
	case eRGT_COUPON:
	case eRGT_GM:
	case eRGT_QUEST:
	case eRGT_MISSION:
	case eRGT_LEVELUP:
	case eRGT_VIP:
	case eRGT_TIMEEVENT:
		return false;
	}

	return true;
}

const SCashShopMainTabShowInfo* CDnCashShopTask::GetShowMainTabInfo(int tabNum) const
{
	std::map<int, SCashShopMainTabShowInfo>::const_iterator iter = m_CashMainTabShowInfo.find(tabNum);
	if (iter != m_CashMainTabShowInfo.end())
		return &((*iter).second);

	return NULL;
}

#ifdef PRE_ADD_CASHSHOP_SUBCAT_COLOR
const std::map<int, SCashShopSubTabShowInfo>*CDnCashShopTask::GetShowSubTabCatList( int mainTabNum ) const 
{
	if (GetShowMainTabInfo(mainTabNum) != NULL)
	{
		std::map<int, std::map<int , SCashShopSubTabShowInfo> >::const_iterator  iter = m_CashSubTabCatShowInfo.find( mainTabNum );
		if( iter != m_CashSubTabCatShowInfo.end() )
		{
			const std::map<int, SCashShopSubTabShowInfo>& SubCatList = ( *iter ).second;
			return &SubCatList;
		}
	}
	return NULL;
}
#endif

const SCashShopTabShowInfo* CDnCashShopTask::GetShowSubTab(int mainTabNum, int subTabNum) const
{
	if (GetShowMainTabInfo(mainTabNum) != NULL)
	{
		std::map<int, std::map<int, SCashShopTabShowInfo> >::const_iterator subIter = m_CashSubTabShowInfo.find(mainTabNum);
		if (subIter != m_CashSubTabShowInfo.end())
		{
			const std::map<int, SCashShopTabShowInfo>& subList = (*subIter).second;
			std::map<int, SCashShopTabShowInfo>::const_iterator subSubIter = subList.find(subTabNum);
			if (subSubIter != subList.end())
				return &((*subSubIter).second);
		}
	}

	return NULL;
}

const std::map<int, SCashShopTabShowInfo>* CDnCashShopTask::GetShowSubTabList(int mainTabNum) const
{
	if (GetShowMainTabInfo(mainTabNum) != NULL)
	{
		std::map<int, std::map<int, SCashShopTabShowInfo> >::const_iterator subIter = m_CashSubTabShowInfo.find(mainTabNum);
		if (subIter != m_CashSubTabShowInfo.end())
		{
			const std::map<int, SCashShopTabShowInfo>& subList = (*subIter).second;
			return &subList;
		}
	}

	return NULL;
}

bool CDnCashShopTask::IsItemPermanent(CASHITEM_SN sn) const
{
	const SCashShopItemInfo* pItemInfo = GetItemInfo(sn);
	if (pItemInfo)
	{
		if (IsCostumeItem(pItemInfo->presentItemId) == false)
			return (pItemInfo->period <= 0);
		else
			return (pItemInfo->period < 0);
	}

	return false;
}

bool CDnCashShopTask::HandleWarning(CASHITEM_SN itemSN, int callBackDlgID, CEtUICallback* pCall)
{
	const SCashShopItemInfo* pInfo = GetItemInfo(itemSN);
	if (pInfo == NULL)
	{
		HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
		return false;
	}

	if (IsPackageItem(itemSN))
	{
		const std::vector<CASHITEM_SN>* pList = GetInPackageItemList(itemSN);
		if (pList == NULL || pList->empty())
		{
			HandleCashShopError(eERRCS_NO_PACKAGE_ITEM_INFO, true);
			return false;
		}

		std::vector<CASHITEM_SN>::const_iterator iter = pList->begin();
		for (; iter != pList->end(); ++iter)
		{
			const CASHITEM_SN& curSN = *iter;
			const SCashShopItemInfo* pInfo = GetItemInfo(curSN);
			if (pInfo == NULL)
			{
				HandleCashShopError(eERRCS_BUY_NO_ITEM_INFO, true);
				return false;
			}

			bool bWarning = false;
			eItemTypeEnum itemType = CDnItem::GetItemType(pInfo->presentItemId);
			if (itemType == ITEMTYPE_INVENTORY_SLOT)
			{
				int usableInvenCount = CDnItemTask::GetInstance().GetCharInventory().GetUsableSlotCount();
				if (usableInvenCount >= INVENTORYMAX)
					bWarning = true;
			}
			else if (itemType == ITEMTYPE_WAREHOUSE_SLOT)
			{
				int usableWarehouseCount = CDnItemTask::GetInstance().GetStorageInventory().GetUsableSlotCount();
				if (usableWarehouseCount >= WAREHOUSEMAX)
					bWarning = true;
			}

			if (bWarning)
			{
				std::wstring str;
				str = FormatW(GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 4519), pInfo->nameString.c_str());	// UISTRING : Already expanded to the maximum limit, so %s cannot expand it further. Are you sure you want to buy it?
				CDnInterface::GetInstance().MessageBox(str.c_str(), MB_YESNO, callBackDlgID, pCall);
				return false;
			}
		}
	}

	return true;
}

void CDnCashShopTask::RequestCashShopBalanceInquiry()
{
	SendCashShopBalanceInquiry();
}

void CDnCashShopTask::SetNotOnSaleItem(int count, const CASHITEM_SN* itemList)
{
	if (count <= 0)
	{
		m_NotOnSaleList.clear();
	}
	else
	{
		int i = 0;
		for (; i < count; ++i)
		{
			const CASHITEM_SN& notOnSaleItemSN = itemList[i];
			if (IsValidSN(notOnSaleItemSN))
				m_NotOnSaleList.insert(notOnSaleItemSN);
		}
	}
}

bool CDnCashShopTask::IsNotOnSaleItem(CASHITEM_SN sn) const
{
	std::set<CASHITEM_SN>::const_iterator iter = m_NotOnSaleList.find(sn);
	return (iter != m_NotOnSaleList.end());
}

bool CDnCashShopTask::IsCashShopMode(eCashShopMode mode) const
{
	return (m_CashShopMode & mode) ? true : false;
}


void CDnCashShopTask::OnRecvCashShopServerTimeForCharge(SCCashShopChargeTime* pData)
{
	m_pCashShopDlg->DisableAllDlgs(false, L"");

	if (pData != NULL)
	{
		m_ChargeTime = pData->tTime;
		DoOpenChargeBrowser();
	}
	else
	{
		HandleCashShopError(eERRCS_CANT_OPEN_CHARGEBROWSER_SERVERTIME_FAIL, true);
	}
}

int	CDnCashShopTask::GetBuyPreviewCartCount() const
{
	int nBuyPreviewCartCount = 0;
	if( m_PreviewCartClassId == PET_CLASS_ID )
		nBuyPreviewCartCount = (int)m_PetPreviewCartList.size();
	else
		nBuyPreviewCartCount = static_cast<int>( m_PreviewCartList[m_PreviewCartClassId - 1].size() );

	return nBuyPreviewCartCount;
}

void CDnCashShopTask::ClearPetPreviewCart()
{
	m_PetPreviewCartList.clear();
}

bool CDnCashShopTask::PutItemIntoPetPreviewCart( CASHITEM_SN sn, ITEMCLSID itemId, int nPetPartsType )
{
	CART_ITEM_LIST::const_iterator iter = m_PetPreviewCartList.begin();
	for( ; iter!=m_PetPreviewCartList.end(); ++iter )
	{
		const SCashShopCartItemInfo& info = *iter;
		if( info.presentSN == sn )
		{
			m_pCashShopDlg->HandleCashShopError( eERRCS_PREVIEWCART_ALREADY_EXIST, false );
			return false;
		}
	}

	eItemTypeEnum type = CDnItem::GetItemType( itemId );
	if( type != ITEMTYPE_PET && type != ITEMTYPE_PETPARTS && type != ITEMTYPE_PETCOLOR_BODY && type != ITEMTYPE_PETCOLOR_TATOO )
		return false;

	SCashShopCartItemInfo info;
	if( SetItemAbility( sn, info.ability ) == false )
		return false;

	info.presentSN = sn;
	if( IsPeriodSelectableItem( sn ) == false )
		info.selectedSN = sn;

	if( nPetPartsType == 2 )
		nPetPartsType = Pet::Slot::Body;
	else if( nPetPartsType == 0 )
		nPetPartsType = Pet::Slot::Accessory1;
	else if( nPetPartsType == 1 )
		nPetPartsType = Pet::Slot::Accessory2;

	if( type == ITEMTYPE_PETCOLOR_BODY || type == ITEMTYPE_PETCOLOR_TATOO )
		info.equipIdx = type;
	else
		info.equipIdx = nPetPartsType;

	info.id	= m_PreviewCartIndexGen;
	m_PreviewCartIndexGen++;

	CART_ITEM_LIST::iterator replaceIter = m_PetPreviewCartList.begin();
	for( ; replaceIter != m_PetPreviewCartList.end(); ++replaceIter )
	{
		SCashShopCartItemInfo& curInfo = *replaceIter;
		if( curInfo.equipIdx == info.equipIdx )
		{
			m_PetPreviewCartList.erase( replaceIter );
			break;
		}
	}

	m_PetPreviewCartList.push_back( info );

	return true;
}

bool CDnCashShopTask::IsValidSkill( ITEMCLSID ability, int nOptionIndex ) const
{
	CDnItem* pItem = NULL;
	TItemInfo itemInfo;
	if( CDnItem::MakeItemInfo( ability, 1, itemInfo ) )
	{
		pItem = GetItemTask().CreateItem( itemInfo );
	}

	if( pItem == NULL )
		return false;

	int nPotentialIndex = pItem->GetTypeParam( 0 );
	SAFE_DELETE( pItem );

	if( nPotentialIndex == 0 )
		return true;

	DNTableFileFormat*  pPotenSox = GetDNTable( CDnTableDB::TPOTENTIAL );
	if( pPotenSox == NULL )
	{
		_ASSERT(0);
		return false;
	}
	DNTableFileFormat*  pSkillTable = GetTableDB().GetTable( CDnTableDB::TSKILL );
	if( pSkillTable == NULL )
	{
		_ASSERT(0);
		return false;
	}

	std::vector<int> nVecItemID;
	pPotenSox->GetItemIDListFromField( "_PotentialID", nPotentialIndex, nVecItemID );

	if( nVecItemID.size() == 0 )
		return true;

	CDnItem::ItemSkillApplyType SkillApplyType = (CDnItem::ItemSkillApplyType)pPotenSox->GetFieldFromLablePtr( nVecItemID[0], "_SkillUsingType" )->GetInteger();
	if( SkillApplyType !=  CDnItem::ItemSkillApplyType::SkillLevelUp )
		return true;

	bool bRtnValue = false;
	if( nOptionIndex > 0 )
		bRtnValue = true;

	return bRtnValue;
}

void CDnCashShopTask::SetCartItemSkill( int index, int nOptionIndex )
{
	SCashShopCartItemInfo* pCartItem = GetCartItemInfo( index );
	if( pCartItem != NULL )
		pCartItem->nOptionIndex = nOptionIndex;
}

void CDnCashShopTask::SetPackageItemSkill( bool bGift, CASHITEM_SN sn, int nOptionIndex )
{
	DoSetPackageItemSkill( bGift ? m_GiftPackageItemList : m_BuyPackageItemList, sn, nOptionIndex );
}

void CDnCashShopTask::DoSetPackageItemSkill( CART_ITEM_LIST& itemList, CASHITEM_SN sn, int nOptionIndex )
{
	if( itemList.empty() )
	{
		_ASSERT(0);
		return;
	}

	CART_ITEM_LIST::iterator iter = itemList.begin();
	for( ; iter != itemList.end(); ++iter )
	{
		SCashShopCartItemInfo& info = *iter;
		if( info.presentSN == sn )
		{
			info.nOptionIndex = nOptionIndex;
			break;
		}
	}

	if( iter == itemList.end() )
		_ASSERT(0);
}

void CDnCashShopTask::SetBuyItemNowSkill( int nOptionIndex )
{
	m_BuyItemNow.nOptionIndex = nOptionIndex;
}

void CDnCashShopTask::SetPreviewCartItemSkill( int index, int nOptionIndex )
{
	CART_ITEM_LIST* pInfo = GetBuyPreviewCartList();
	if( pInfo == NULL )
	{
		_ASSERT(0);
		return;
	}

	CART_ITEM_LIST::iterator iter = pInfo->begin();
	for( ; iter != pInfo->end(); ++iter )
	{
		SCashShopCartItemInfo& info = *iter;
		if( info.id == index )
		{
			info.nOptionIndex = nOptionIndex;
			break;
		}
	}
}

#ifdef PRE_ADD_SALE_COUPON

bool CDnCashShopTask::CheckCoupon()
{
	std::vector<CDnItem *> vecCouponItem;
	GetItemTask().FindItemFromItemType( ITEMTYPE_SALE_COUPON, ITEM_SLOT_TYPE::ST_INVENTORY_CASH, vecCouponItem );
	if( vecCouponItem.empty() )
		return false;

	bool bExistUsableCoupon = false;
	const time_t pNowTime = CSyncTimer::GetInstance().GetCurTime();

	for( int i=0; i<static_cast<int>( vecCouponItem.size() ); i++ )
	{
		if( vecCouponItem[i] == NULL )
			continue;

		if( vecCouponItem[i]->IsEternityItem() ||
			pNowTime < *( vecCouponItem[i]->GetExpireDate() ) )	// Check if expiration period has passed
		{
			bExistUsableCoupon = true;
			break;
		}
	}

	vecCouponItem.clear();

	return bExistUsableCoupon;
}

INT64 CDnCashShopTask::GetItemApplyCouponSN( CASHITEM_SN sn )
{
	std::vector<CDnItem *> vecCouponItem;
	GetItemTask().FindItemFromItemType( ITEMTYPE_SALE_COUPON, ITEM_SLOT_TYPE::ST_INVENTORY_CASH, vecCouponItem );
	if( vecCouponItem.empty() )
		return false;

	DNTableFileFormat* pCouponSox = GetDNTable( CDnTableDB::TSALECOUPON );
	if( pCouponSox == NULL )
		return false;

	bool bExistUsableCoupon = false;
	const time_t pNowTime = CSyncTimer::GetInstance().GetCurTime();

	char szLabel[32]={0};
	wchar_t szTemp[256]={0};
	bool bFindUsableCoupon = false;
	INT64 UsableCouponSN = 0;

	std::sort( vecCouponItem.begin(), vecCouponItem.end(), CompareCouponExpire );	// Show coupons with shortest expiration date first

	for( int i=0; i<static_cast<int>( vecCouponItem.size() ); i++ )
	{
		if( vecCouponItem[i] == NULL )
			continue;

#ifdef PRE_MOD_SALE_COUPON_LEVEL_LIMIT
		if( vecCouponItem[i]->GetLevelLimit() > CDnActor::s_hLocalActor->GetLevel() )
			continue;
#endif // PRE_MOD_SALE_COUPON_LEVEL_LIMIT

		if( vecCouponItem[i]->IsEternityItem() || pNowTime < *( vecCouponItem[i]->GetExpireDate() ) )	// Check if expiration period has passed
		{
			int nCouponID = vecCouponItem[i]->GetTypeParam( 0 );
			for( int j=0; j<MAX_COUPON_SALEITEM; j++ )
			{
				if( j+1 < 10 )
					sprintf_s( szLabel, "_UseItem0%d", j+1 );
				else
					sprintf_s( szLabel, "_UseItem%d", j+1 );

				int nSaleItemID = pCouponSox->GetFieldFromLablePtr( nCouponID, szLabel )->GetInteger();

				if( nSaleItemID == 0 )
					continue;

				const SCashShopItemInfo* pInfo = GetCashShopTask().GetItemInfo( nSaleItemID );
				if( pInfo && pInfo->sn == sn && pInfo->bUseCoupon )
				{
					bFindUsableCoupon = true;
					break;
				}
			}

			if( bFindUsableCoupon )
			{
				UsableCouponSN = vecCouponItem[i]->GetSerialID();
				break;
			}
		}
	}

	vecCouponItem.clear();

	return UsableCouponSN;
}

const SCashShopItemInfo* CDnCashShopTask::GetSubCatItemByPresentID( eCashShopSubCatType type, int index ) const
{
	CS_CATEGORY_LIST::const_iterator iter = m_ItemListByCategory.find( type );
	if( iter != m_ItemListByCategory.end() )
	{
		const CS_INFO_LIST& infoList = (*iter).second;
		CS_INFO_LIST::const_iterator iter = infoList.begin();
		for( ; iter != infoList.end(); ++iter )
		{
			SCashShopItemInfo* pInfo = *iter;
			if( pInfo )
			{
				if( pInfo->presentItemId == index )
					return pInfo;
			}
		}
	}

	return NULL;
}

#endif // PRE_ADD_SALE_COUPON

#ifdef PRE_ADD_INSTANT_CASH_BUY

int CDnCashShopTask::GetInstantCashBuyEnableType( eInstantCashShopBuyType eBuyType )
{
	DNTableFileFormat* pCashBuyShortcut = GetDNTable( CDnTableDB::TCASHBUYSHORTCUT );
	if( pCashBuyShortcut == NULL )
		return false;

	int nEnabeType = 0;	// 0 : hide, 1 : Show, 2 : Disable
	for( int i=0; i<pCashBuyShortcut->GetItemCount(); i++ )
	{
		int nItemIndex = pCashBuyShortcut->GetItemID( i );
		int nType = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_Type" )->GetInteger();

		if( nType == eBuyType )
		{
			int nAllowedGameType = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_AllowMapTypes" )->GetInteger();
			int nGameMode = CDnWorld::GetInstance().GetAllowMapType();
			int nShow = pCashBuyShortcut->GetFieldFromLablePtr( nItemIndex, "_Show" )->GetInteger();
			if( nAllowedGameType & nGameMode )
			{
				if( nShow == 0 )
					nEnabeType = 0;
				else if( nShow == 1 || nShow == 2 )
					nEnabeType = 1;
			}
			else
			{
				if( nShow == 0 || nShow == 2 )
					nEnabeType = 0;
				else if( nShow == 1 )
					nEnabeType = 2;
			}
			break;
		}
	}

	return nEnabeType;
}

#endif // PRE_ADD_INSTANT_CASH_BUY

#if defined(PRE_ADD_68286)
void CDnCashShopTask::RequestCashShopRecvGiftBasicInfoByShortCut()
{
	SendCashShopGiftListByShortCut();
	m_pCashShopDlg->DisableAllDlgs(true, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 4783 ), 2.f); // UISTRING : Receiving gift information. Please wait a moment.
	m_bRequestingGiftBasicInfoes = true;
}
#endif // PRE_ADD_68286


#ifdef PRE_ADD_CASHSHOP_RENEWAL

// Product search - find items whose product name contains argument str and add them as a category.
void CDnCashShopTask::SearchItems( const std::wstring & str )
{		
	if( m_strSearch == str )
		return;

	// Clear previous search results.
	CS_CATEGORY_LIST::iterator it = m_ItemListByCategory.find( SUBCATEGORY_SERACH );
	if( it == m_ItemListByCategory.end() )
		return;
	CS_INFO_LIST & rCategory = it->second;
	rCategory.clear();

	if( !str.empty() && str.size() > 0 )
	{
		// Product search.
		int size = 0;
		it = m_ItemListByCategory.begin();
		for( ; it != m_ItemListByCategory.end(); it++ )
		{
			CS_INFO_LIST & vItems = it->second;
			size = (int)vItems.size();
			for( int i=0; i<size; ++i )
			{
				SCashShopItemInfo * pItemInfo = vItems[i];
				if( std::wstring::npos != pItemInfo->nameString.find( str ) )
				{
					// Add searched item to search results.
					rCategory.push_back( pItemInfo );
				}	
			}
		}

	}

	// Sort by job.
	Sort( SUBCATEGORY_SERACH, SORT_MYCLASS );

	m_strSearch.assign( str );	
}


void CDnCashShopTask::UpdateIngameBannerItem()
{
#ifdef PRE_ADD_CASHSHOPBANNER_RENEWAL
	CEtUIDialog * pDlg = GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CASHSHOP_INGAMEBANNER_DIALOG );
	if( pDlg )
		static_cast< CDnCashShopIngameBanner * >( pDlg )->UpdateIngameBannerItem();
#endif // PRE_ADD_CASHSHOPBANNER_RENEWAL
}

#endif // PRE_ADD_CASHSHOP_RENEWAL