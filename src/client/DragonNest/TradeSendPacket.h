#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"

// TRADE ----------------------------------------------------------------------------
// Shop
#if defined(PRE_ADD_REMOTE_OPENSHOP)
inline void SendShopRemoteOpen(Shop::Type::eCode eType)
{
	CSShopRemoteOpen Shop;
	Shop.Type = eType;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SHOP_REMOTEOPEN, (char*)&Shop, sizeof(CSShopRemoteOpen));
}
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)

inline void SendShopBuy(int nTabID, BYTE cShopIndex, int nCount)
{
	CSShopBuy Buy;
	memset(&Buy, 0, sizeof(CSShopBuy));

	Buy.cTapIndex = nTabID;
	Buy.cShopIndex = cShopIndex;
	Buy.wCount = nCount;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SHOP_BUY, (char*)&Buy, sizeof(CSShopBuy));
}

inline void SendShopSell(BYTE cInvenIndex, int nCount, INT64 biSerial)
{
	CSShopSell Sell;
	memset(&Sell, 0, sizeof(CSShopSell));

	Sell.cInvenIndex = cInvenIndex;
	Sell.biItemSerial = biSerial;
	Sell.wCount = nCount;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SHOP_SELL, (char*)&Sell, sizeof(CSShopSell));
}

inline void SendShopGetRepurchaseList()
{
	CClientSessionManager::GetInstance().SendPacket( CS_TRADE, eTrade::CS_SHOP_GETLIST_REPURCHASE, NULL, 0 );
}

inline void SendShopRepurchase(int nRepurchaseID)
{
	CSShopRepurchase packet;
	memset(&packet, 0, sizeof(CSShopRepurchase));

	packet.iRepurchaseID = nRepurchaseID;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SHOP_REPURCHASE, (char*)&packet, sizeof(CSShopRepurchase));
}

inline void SendRepairEquip()
{
	CClientSessionManager::GetInstance().SendPacket( CS_TRADE, eTrade::CS_REPAIR_EQUIP, NULL, 0 );
}

inline void SendRepairAll()
{
	CClientSessionManager::GetInstance().SendPacket( CS_TRADE, eTrade::CS_REPAIR_ALL, NULL, 0 );
}

//// SkillShop
//inline void SendSkillShopBuy(BYTE cShopIndex)
//{
//	CSSkillShopBuy Buy;
//	memset(&Buy, 0, sizeof(CSSkillShopBuy));
//	Buy.cShopIndex = cShopIndex;
//	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SKILLSHOP_BUY, (char*)&Buy, sizeof(CSSkillShopBuy));
//}

// Mail
inline void SendMailBox(char cPageNum)	// Request mailbox list
{
	CSMailBox MailBox;
	memset(&MailBox, 0, sizeof(CSMailBox));
	MailBox.cPageNum = cPageNum;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAILBOX, (char*)&MailBox, sizeof(CSMailBox));
}

inline void SendMailSend(const WCHAR *pToCharName, const WCHAR *pTitle, const WCHAR *pText, int nAttachCoin, char cDeliveryType, int nAttachItemTotalCount, TAttachMailItem *AttachItemList)		// Send mail
{
	CSSendMail Send;
	memset(&Send, 0, sizeof(CSSendMail));

	_wcscpy(Send.wszToCharacterName, _countof(Send.wszToCharacterName), pToCharName, (int)wcslen(pToCharName));
	_wcscpy(Send.wszTitle, _countof(Send.wszTitle), pTitle, (int)wcslen(pTitle));
	_wcscpy(Send.wszText, _countof(Send.wszText), pText, (int)wcslen(pText));
	Send.nAttachCoin = nAttachCoin;
	Send.cDeliveryType = cDeliveryType;
	memcpy(Send.AttachMailItem, AttachItemList, sizeof(TAttachMailItem) * nAttachItemTotalCount);
	Send.cAttachItemTotalCount = nAttachItemTotalCount;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_SEND, (char*)&Send, sizeof(CSSendMail) - sizeof(Send.AttachMailItem) + (sizeof(TAttachMailItem) * Send.cAttachItemTotalCount));
}

inline void SendMailRead(int nMailDBID)		// Read mail
{
	CSReadMail Read;
	memset(&Read, 0, sizeof(CSReadMail));
	Read.nMailDBID = nMailDBID;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_READ, (char*)&Read, sizeof(CSReadMail));
}

#ifdef PRE_ADD_CADGE_CASH
inline void SendCadgeMailRead(int nMailDBID)		// Read beg/request gift mail
{
	CSReadMail Read;
	memset(&Read, 0, sizeof(CSReadMail));
	Read.nMailDBID = nMailDBID;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_READCADGE, (char*)&Read, sizeof(CSReadMail));
}
#endif // PRE_ADD_CADGE_CASH

inline void SendMailDelete(int *nMailDBIDArray)	// Delete mail
{
	CSMailDBIDArray Delete;
	memset(&Delete, 0, sizeof(CSMailDBIDArray));
	memcpy(Delete.nMailDBID, nMailDBIDArray, sizeof(Delete.nMailDBID));
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_DELETE, (char*)&Delete, sizeof(CSMailDBIDArray));
}

inline void SendAttachAllMail(int *nMailDBIDArray)	// Find attached coin and items
{
	CSMailDBIDArray Attach;
	memset(&Attach, 0, sizeof(CSMailDBIDArray));
	memcpy(Attach.nMailDBID, nMailDBIDArray, sizeof(Attach.nMailDBID));
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_ATTACHALL, (char*)&Attach, sizeof(CSMailDBIDArray));
}

inline void SendAttachMail(int nMailDBID, char cAttachSlotIndex)	// Find attached coin and items
{
	CSAttachMail Attach;
	memset(&Attach, 0, sizeof(CSAttachMail));
	Attach.nMailDBID = nMailDBID;
	Attach.cAttachSlotIndex = cAttachSlotIndex;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MAIL_ATTACH, (char*)&Attach, sizeof(CSAttachMail));
}

// Personal trade
inline void SendExchangeRequest(UINT nSessionID, bool bCancel)
{
	CSExchangeRequest Request;
	memset(&Request, 0, sizeof(CSExchangeRequest));
	Request.nReceiverSessionID = nSessionID;
	Request.bCancel = bCancel;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_REQUEST, (char*)&Request, sizeof(CSExchangeRequest));
}

inline void SendExchangeAccept(bool bAccept, UINT nSenderSessionID)
{
	CSExchangeAccept Accept;
	memset(&Accept, 0, sizeof(CSExchangeAccept));
	Accept.nSenderSessionID = nSenderSessionID;
	Accept.bAccept = bAccept;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_ACCEPT, (char*)&Accept, sizeof(CSExchangeAccept));
}

inline void SendExchangeAddItem(char cExchangeIndex, char cInvenIndex, short wCount, INT64 biSerial)
{
	CSExchangeAddItem AddItem;
	memset(&AddItem, 0, sizeof(CSExchangeAddItem));
	AddItem.cExchangeIndex = cExchangeIndex;
	AddItem.cInvenIndex = cInvenIndex;
	AddItem.wCount = wCount;
	AddItem.biItemSerial = biSerial;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_ADDITEM, (char*)&AddItem, sizeof(CSExchangeAddItem));
}

inline void SendExchangeDeleteItem(char cIndex)
{
	CSExchangeDeleteItem DeleteItem;
	memset(&DeleteItem, 0, sizeof(CSExchangeDeleteItem));
	DeleteItem.cExchangeIndex = cIndex;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_DELETEITEM, (char*)&DeleteItem, sizeof(CSExchangeDeleteItem));	
}

inline void SendExchangeAddCoin(INT64 nCoin)
{
	CSExchangeAddCoin AddCoin;
	memset(&AddCoin, 0, sizeof(CSExchangeAddCoin));
	AddCoin.nCoin = nCoin;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_ADDCOIN, (char*)&AddCoin, sizeof(CSExchangeAddCoin));
}

inline void SendExchangeConfirm(char cType)
{
	CSExchangeConfirm Confirm;
	memset(&Confirm, 0, sizeof(CSExchangeConfirm));
	Confirm.cType = cType;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_CONFIRM, (char*)&Confirm, sizeof(CSExchangeConfirm));
}

inline void SendExchangeCancel()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_EXCHANGE_CANCEL, NULL, 0);
}

// Trading house
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
#include "DnNameAutoComplete.h"
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
inline void SendMarketList(int nPageNum, char cMinLevel, char cMaxLevel, char *cItemGradeArray, char cJob, short cMainType, char cDetailType, WCHAR *pwszSearch1, WCHAR *pwszSearch2, WCHAR *pwszSearch3, char cSortType, int nExchangeItemID, bool bLowJobGroup, WCHAR *pwszSearch, char cPayMethodCode )
{
	CSMarketList List;
	memset(&List, 0, sizeof(CSMarketList));

	List.wPageNum = nPageNum;
	List.cMinLevel = cMinLevel;
	List.cMaxLevel = cMaxLevel;
	memcpy_s(List.cItemGrade, sizeof(List.cItemGrade), cItemGradeArray, sizeof(List.cItemGrade) );
	List.cJob = cJob;
	List.cMainType = cMainType;
	List.cDetailType = cDetailType;
	if (pwszSearch1) _wcscpy(List.wszSearchWord1, _countof(List.wszSearchWord1), pwszSearch1, (int)wcslen(pwszSearch1));
	if (pwszSearch2) _wcscpy(List.wszSearchWord2, _countof(List.wszSearchWord2), pwszSearch2, (int)wcslen(pwszSearch2));
	if (pwszSearch3) _wcscpy(List.wszSearchWord3, _countof(List.wszSearchWord3), pwszSearch3, (int)wcslen(pwszSearch3));
	List.cSortType = cSortType;
	_wcscpy( List.wszSearchItemName, _countof(List.wszSearchItemName), pwszSearch, (int)wcslen(pwszSearch) );
	List.nExchangeItemID = nExchangeItemID;
	List.bLowJobGroup = bLowJobGroup;

#if defined(PRE_ADD_PETALTRADE)
	List.cPayMethodCode = cPayMethodCode;
#endif	//#if defined(PRE_ADD_PETALTRADE)
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	std::vector<CNameAutoComplete::TNameAutoItem*> vList;
	const CNameAutoComplete::TNameAutoItem * pAutoName = CNameAutoComplete::GetInstance().GetAutoNameData(List.wszSearchItemName, vList);
	if (pAutoName || vList.size() > 0)
	{
		if (pAutoName)
		{
			List.cCount = 1;
			List.SearchList[0].nSearchNameID = pAutoName->nNameID;
			List.SearchList[0].nSearchItemID = pAutoName->nItemID;
		}
		else
		{
			List.cCount = static_cast<BYTE>(vList.size());
			for (int i = 0; i < List.cCount; i++)
			{
				List.SearchList[i].nSearchNameID = vList[i]->nNameID;
				List.SearchList[i].nSearchItemID = vList[i]->nItemID;
			}
		}
	}
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKETLIST, (char*)&List, sizeof(CSMarketList));
}

inline void SendMyMarketList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MYMARKETLIST, NULL, 0);
}

inline void SendMarketRegister(INT64 biInvenSerial, short wCount, int nPrice, char cPeriodIndex, char cInvenType, BYTE cInvenIndex, bool bPremium, char cPayMethodCode )
{
	CSMarketRegister Register;
	memset(&Register, 0, sizeof(CSMarketRegister));

	Register.cInvenType = cInvenType;
	Register.cInvenIndex = cInvenIndex;
	Register.biInvenSerial = biInvenSerial;
	Register.wCount = wCount;
	Register.nPrice = nPrice;
	Register.cPeriodIndex = cPeriodIndex;
	Register.bPremiumTrade = bPremium;
#if defined(PRE_ADD_PETALTRADE)
	Register.cPayMethodCode = cPayMethodCode;
#endif	//#if defined(PRE_ADD_PETALTRADE)

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_REGISTER, (char*)&Register, sizeof(CSMarketRegister));
}

inline void SendMarketInterrupt(int nMarketDBID)
{
	CSMarketDBID Market;
	memset(&Market, 0, sizeof(CSMarketDBID));
	Market.nMarketDBID = nMarketDBID;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_INTERRUPT, (char*)&Market, sizeof(CSMarketDBID));
}

inline void SendMarketBuy(int nMarketDBID, bool bMini)
{
	CSMarketBuy Market;
	memset(&Market, 0, sizeof(CSMarketBuy));

	Market.nMarketDBID = nMarketDBID;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	Market.bMini = bMini;
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_BUY, (char*)&Market, sizeof(CSMarketBuy));
}

inline void SendMarketCalculationList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_CALCULATIONLIST, NULL, 0);
}

inline void SendMarketCalculation(int nMarketDBID)
{
	CSMarketDBID Market;
	memset(&Market, 0, sizeof(CSMarketDBID));
	Market.nMarketDBID = nMarketDBID;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_CALCULATION, (char*)&Market, sizeof(CSMarketDBID));
}

inline void SendMarketCalculationAll()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_CALCULATIONALL, NULL, 0);
}

inline void SendMarketPetalBalance()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_PETALBALANCE, NULL, 0);
}

inline void SendMarketPrice( int nMarketDBID, int nItemID, BYTE cLevel, BYTE cOption )
{
	CSMarketPrice packet;
	memset( &packet, 0, sizeof(CSMarketPrice) );

	packet.nMarketDBID = nMarketDBID;
	packet.nItemID = nItemID;
	packet.cLevel = cLevel;
	packet.cOption = cOption;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKET_PRICE, (char*)&packet, sizeof(CSMarketPrice));
}

#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
inline void SendMarketMiniList(int nItemID, char cPayMethodCode)
{
	CSMarketMiniList Market;
	Market.nItemID = nItemID;
	Market.cPayMethodCode = cPayMethodCode;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_MARKETMINILIST, (char*)&Market, sizeof(CSMarketMiniList));
}
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)

#if defined(PRE_SPECIALBOX)
inline void SendSpecialBoxList()
{
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SPECIALBOX_LIST, NULL, 0);
}

inline void SendSpecialBoxItemList(int nEventRewardID)
{
	CSSpecialBoxItemList Packet = {0,};
	Packet.nEventRewardID = nEventRewardID;
	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SPECIALBOX_ITEMLIST, (char*)&Packet, sizeof(CSSpecialBoxItemList));
}

inline void SendReceiveSpecialBoxItem(int nEventRewardID, int nItemID)
{
	CSReceiveSpecialBoxItem Packet = {0,};

	Packet.nEventRewardID = nEventRewardID;
	Packet.nItemID = nItemID;

	CClientSessionManager::GetInstance().SendPacket(CS_TRADE, eTrade::CS_SPECIALBOX_RECEIVEITEM, (char*)&Packet, sizeof(CSReceiveSpecialBoxItem));
}

#endif	// #if defined(PRE_SPECIALBOX)