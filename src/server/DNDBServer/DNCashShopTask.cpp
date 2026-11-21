#include "stdafx.h"
#include "DNCashShopTask.h"
#include "DNConnection.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "DNExtManager.h"
#include "Log.h"
#include "Util.h"

CDNCashShopTask::CDNCashShopTask(CDNConnection* pConnection)
: CDNMessageTask(pConnection)
{

}

CDNCashShopTask::~CDNCashShopTask(void)
{
}

void CDNCashShopTask::OnRecvMessage(int nThreadID, int nMainCmd, int nSubCmd, char* pData)
{
	CDNSQLMembership *pMembershipDB = NULL;
	CDNSQLWorld *pWorldDB = NULL;
	int nRet = ERROR_DB;

	switch (nSubCmd)
	{
	case QUERY_MODGIVEFAILFLAG:
		{
			TQModGiveFailFlag *pCash = (TQModGiveFailFlag*)pData;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (!pMembershipDB){
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[QUERY_MODGIVEFAILFLAG] pMembershipDB not found\r\n");
				return;
			}

			TAModGiveFailFlag Cash;
			memset(&Cash, 0, sizeof(TAModGiveFailFlag));
			Cash.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pCash->cWorldSetID);
			if (!pWorldDB){
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[QUERY_MODGIVEFAILFLAG:%d] pWorldDB not found\r\n", pCash->cWorldSetID);
				return;
			}

			int nCount = 0, nRet = 0;
			for (int i = 0; i < pCash->nCount; i++){
				int nPrice = g_pExtManager->GetCashCommodityPrice(pCash->CashItem[i].nItemSN);
				if (m_pConnection->CashItemDBProcess(pMembershipDB, pWorldDB, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, pCash->nMapID, pCash->nChannelID, false, pCash->CashItem[i], 
					nPrice, pCash->CashItem[i].biPurchaseOrderID, DBDNWorldDef::AddMaterializedItem::CashBuy, pCash->wszIP,
					pCash->CashItem[i].biSenderCharacterDBID, pCash->CashItem[i].bGift) != ERROR_NONE) 
					continue;
				// biPurchaseOrderID is actually biPurchaseOrderDetailID.
				if (pMembershipDB->QueryModGiveFailFlag2(pCash->CashItem[i].biPurchaseOrderID, false) != ERROR_NONE) continue;

				Cash.CashItem[nCount].nItemSN = pCash->CashItem[i].nItemSN;
				Cash.CashItem[nCount].CashItem = pCash->CashItem[i].CashItem;
				nCount++;
			}

			if (nCount > 0){
				Cash.nAccountDBID = pCash->nAccountDBID;
				Cash.nCount = nCount;
				Cash.nRetCode = ERROR_NONE;

				int nLen = sizeof(TAModGiveFailFlag) - sizeof(Cash.CashItem) + (sizeof(TCashBuyItem) * nCount);
				m_pConnection->AddSendData(MAINCMD_CASH, QUERY_MODGIVEFAILFLAG, (char*)&Cash, nLen);
			}
			//OnModGiveFailFlag(nThreadID, pCash);
		}
		break;

	case QUERY_CHECKGIFTRECEIVER:
		{
			TQCheckGiftReceiver *pCash = (TQCheckGiftReceiver*)pData;

			TACheckGiftReceiver Cash;
			memset(&Cash, 0, sizeof(TACheckGiftReceiver));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pCash->cWorldSetID);
			if (pWorldDB){
				Cash.nRetCode = pWorldDB->QueryGetCharacterPartialy6(pCash->wszCharacterName, Cash.cJob, Cash.cLevel);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_CHECKGIFTRECEIVER:%d] pWorldDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			switch (Cash.nRetCode)
			{
			case ERROR_NONE:
			case 103102: // Character does not exist.
				break;

			default:
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_CHECKGIFTRECEIVER:%d] DB Error\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
				break;
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TACheckGiftReceiver));
		}
		break;

	case QUERY_GETLISTGIFTBOX:
		{
			TQGetListGiftBox *pCash = (TQGetListGiftBox*)pData;

			TAGetListGiftBox Cash;
			memset(&Cash, 0, sizeof(TAGetListGiftBox));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryGetListGiftBox(pCash->biCharacterDBID, Cash.cCount, Cash.GiftBox);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GETLISTGIFTBOX:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GETLISTGIFTBOX:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			int nLen = sizeof(TAGetListGiftBox) - sizeof(Cash.GiftBox) + (sizeof(TGiftInfo) * Cash.cCount);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, nLen);
		}
		break;

	case QUERY_MODGIFTRECEIVEFLAG:
		{
			TQModGiftReceiveFlag *pCash = (TQModGiftReceiveFlag*)pData;

			TAModGiftReceiveFlag Cash;
			memset(&Cash, 0, sizeof(TAModGiftReceiveFlag));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;
			Cash.cCount = pCash->cCount;
#if defined(PRE_RECEIVEGIFTALL)
			Cash.bReceiveAll = pCash->bReceiveAll;
#endif	// #if defined(PRE_RECEIVEGIFTALL)

			for (int i = 0; i < pCash->cCount; i++){
				Cash.ReceiveGift[i].GiftData = pCash->GiftData[i];

				pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
				if (pMembershipDB){
					int nPrice = 0;
					std::vector<DBPacket::TItemIDOption> VecItemIDList;
					VecItemIDList.clear();
					Cash.nRetCode = pMembershipDB->QueryGetListItemOfGift(pCash->GiftData[i].nGiftDBID, Cash.ReceiveGift[i].nItemSN, nPrice, VecItemIDList);

					if (Cash.nRetCode == ERROR_NONE){
						char cCount = (int)VecItemIDList.size();
						for (int j = 0; j < cCount; j++){
							Cash.ReceiveGift[i].ItemIDOptions[j] = VecItemIDList[j];

							switch (g_pExtManager->GetItemMainType(Cash.ReceiveGift[i].ItemIDOptions[j].nItemID))
							{
							case ITEMTYPE_CHARACTER_SLOT:
								{
									char cCharacterCount = 0;
#if defined(PRE_MOD_SELECT_CHAR)
									nRet = pMembershipDB->QueryGetCharacterSlotCount(pCash->nAccountDBID, 0, 0, cCharacterCount);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
									nRet = pMembershipDB->QueryGetCharacterSlotCount(0, 0, pCash->biCharacterDBID, cCharacterCount);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

									if (nRet == ERROR_NONE){
#if defined(PRE_MOD_SELECT_CHAR)
										if (cCharacterCount + g_pExtManager->GetGlobalWeightValue(Login_Base_CreateCharCount) >= g_pExtManager->GetGlobalWeightValue(Login_Max_CreateCharCount))
#else	// #if defined(PRE_MOD_SELECT_CHAR)
										if (cCharacterCount >= (CreateCharacterDefaultCountMax + g_pExtManager->GetGlobalWeightValue(CharacterSlotMax)))
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
											Cash.nRetCode =	101190;	// 101190 = Exceeds the maximum number of character slots you can own.
									}
								}
								break;

							default:
								break;
							}
						}
					}
				}
				else{
					g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MODGIFTRECEIVEFLAG:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
				}

				if (Cash.nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MODGIFTRECEIVEFLAG:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);
			}

			int nLen = sizeof(TAModGiftReceiveFlag) - sizeof(Cash.ReceiveGift) + (sizeof(DBPacket::TReceiveGift) * Cash.cCount);
			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, nLen);
		}
		break;

	case QUERY_RECEIVEGIFT:
		{
			TQReceiveGift *pCash = (TQReceiveGift*)pData;

			TAReceiveGift Cash;
			memset(&Cash, 0, sizeof(TAReceiveGift));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.biPurchaseOrderID = pCash->GiftData.nGiftDBID;
#if defined(PRE_ADD_VIP)
			Cash.cPayMethodCode = pCash->GiftData.cPayMethodCode;
#endif	// #if defined(PRE_ADD_VIP)
			Cash.nRetCode = ERROR_DB;

#if defined(PRE_RECEIVEGIFTALL)
			Cash.bReceiveAll = pCash->bReceiveAll;
			Cash.cTotalCount = pCash->cTotalCount;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
			Cash.nItemSN = pCash->nItemSN;
			Cash.cCount = pCash->cCount;
			memcpy(Cash.GiftItem, pCash->GiftItem, sizeof(TGiftItem) * Cash.cCount);

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (!pMembershipDB){
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_RECEIVEGIFT:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}
			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pCash->cWorldSetID);
			if (!pWorldDB){
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_RECEIVEGIFT:%d] WorldDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (pMembershipDB && pWorldDB){
				INT64 biGiftSenderCharacterDBID = 0;
				INT64 biReplySenderCharacterDBID = 0;
				WCHAR wszReplyReceiverCharacterName[NAMELENMAX] = { 0, };
				int nPaidCashAmount = 0;

				Cash.nRetCode = pMembershipDB->QueryModGiftReceiveFlag(pCash->GiftData.nGiftDBID, biReplySenderCharacterDBID, wszReplyReceiverCharacterName, biGiftSenderCharacterDBID, nPaidCashAmount);

				if (Cash.nRetCode == ERROR_NONE){
					int nPrice = g_pExtManager->GetCashCommodityPrice(pCash->nItemSN);

					if (pCash->cCount > 1){	// Package
						int nTotalPrice = g_pExtManager->GetCashCommodityPrice(pCash->nItemSN);
						int nUnitPrice = 0;
						if (nTotalPrice > 0)
							nUnitPrice = nTotalPrice / pCash->cCount;
						int nTempPrice = 0;
						for (int i = 0; i < pCash->cCount; i++){
							if (i == (pCash->cCount - 1)) nUnitPrice = nTotalPrice - nTempPrice;

							TCashItemBase Item = { 0, };
							Item.nItemSN = pCash->GiftItem[i].nItemSN;
							Item.CashItem = pCash->GiftItem[i].AddItem;
							Item.dwPartsColor1 = pCash->GiftItem[i].dwPartsColor1;
							Item.dwPartsColor2 = pCash->GiftItem[i].dwPartsColor2;
							Item.VehiclePart1 = pCash->GiftItem[i].VehiclePart1;
							Item.VehiclePart2 = pCash->GiftItem[i].VehiclePart2;

							nRet = m_pConnection->CashItemDBProcess(pMembershipDB, pWorldDB, pCash->cWorldSetID, pCash->nAccountDBID, biReplySenderCharacterDBID, pCash->nMapID, pCash->nChannelID, false, Item, 
								nPrice, pCash->GiftData.nGiftDBID, DBDNWorldDef::AddMaterializedItem::Present, pCash->wszIP, biGiftSenderCharacterDBID, true, pCash->GiftData.cPayMethodCode);

							if (nRet != ERROR_NONE){ //Gifts use the old version. QueryModGiveFailFlag
								pMembershipDB->QueryModGiveFailFlag(pCash->GiftData.nGiftDBID, pCash->nItemSN, pCash->GiftItem[i].AddItem.nItemID, true);
							}
							nTempPrice += nUnitPrice;
						}
					}
					else{
						TCashItemBase Item = { 0, };
						Item.nItemSN = pCash->nItemSN;
						Item.CashItem = pCash->GiftItem[0].AddItem;
						Item.dwPartsColor1 = pCash->GiftItem[0].dwPartsColor1;
						Item.dwPartsColor2 = pCash->GiftItem[0].dwPartsColor2;
						Item.VehiclePart1 = pCash->GiftItem[0].VehiclePart1;
						Item.VehiclePart2 = pCash->GiftItem[0].VehiclePart2;

						nRet = m_pConnection->CashItemDBProcess(pMembershipDB, pWorldDB, pCash->cWorldSetID, pCash->nAccountDBID, biReplySenderCharacterDBID, pCash->nMapID, pCash->nChannelID, false, Item, 
							nPrice, pCash->GiftData.nGiftDBID, DBDNWorldDef::AddMaterializedItem::Present, pCash->wszIP, biGiftSenderCharacterDBID, true, pCash->GiftData.cPayMethodCode);

						if (nRet != ERROR_NONE){	//Gifts use the old version. QueryModGiveFailFlag
							pMembershipDB->QueryModGiveFailFlag(pCash->GiftData.nGiftDBID, pCash->nItemSN, pCash->GiftItem[0].AddItem.nItemID, true);
						}
					}

					Cash.nRetCode = nRet;

					if ((biGiftSenderCharacterDBID > 0) && (biGiftSenderCharacterDBID != biReplySenderCharacterDBID)){
						TQSendMail SendMail;
						memset(&SendMail, 0, sizeof(TQSendMail));
						SendMail.cWorldSetID = pCash->cWorldSetID;
						SendMail.nAccountDBID = pCash->nAccountDBID;
						SendMail.biSenderCharacterDBID = biReplySenderCharacterDBID;
						_wcscpy(SendMail.wszReceiverCharacterName, NAMELENMAX, wszReplyReceiverCharacterName, NAMELENMAX);
						SendMail.Code = MailType::ReplyGift;
						SendMail.iChannelID = pCash->nChannelID;
						SendMail.iMapID = pCash->nMapID;
						_wcscpy(SendMail.wszSubject, MAILTITLELENMAX, pCash->GiftData.wszEmoticonTitle, MAILTITLELENMAX);
						_wcscpy(SendMail.wszContent, MAILTEXTLENMAX, pCash->GiftData.wszReplyMessage, GIFTMESSAGEMAX);

						TASendMail ASendMail;
						memset(&ASendMail, 0, sizeof(TASendMail));

						pWorldDB->QuerySendMail(&SendMail, &ASendMail);

						Cash.nReceiverAccountDBID = ASendMail.nReceiverAccountDBID;
						Cash.biReceiverCharacterDBID = ASendMail.biReceiverCharacterDBID;
						Cash.nReceiverTotalMailCount = ASendMail.nReceiverTotalMailCount;
						Cash.nReceiverNotReadMailCount = ASendMail.nReceiverNotReadMailCount;
						Cash.nReceiver7DaysLeftMailCount = ASendMail.nReceiver7DaysLeftMailCount;
					}
				}
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_RECEIVEGIFT:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAReceiveGift));
		}
		break;

	case QUERY_NOTIFYGIFT:
		{
			TQNotifyGift *pCash = (TQNotifyGift*)pData;

			TANotifyGift Cash;
			memset(&Cash, 0, sizeof(TANotifyGift));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.bNew = pCash->bNew;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryGetCountNotReceivedGift(pCash->biCharacterDBID, Cash.nGiftCount);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_NOTIFYGIFT:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_NOTIFYGIFT:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TANotifyGift));
		}
		break;

	case QUERY_MAKEGIFTBYQUEST:
		{
			TQMakeGiftByQuest *pCash = (TQMakeGiftByQuest*)pData;

			TAMakeGiftByQuest Cash;
			memset(&Cash, 0, sizeof(TAMakeGiftByQuest));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				BYTE cItemOption = -1;
				string ItemIDStr, ItemOptionStr;
				INT64 biPurchaseOrderID = -1;
				TCashCommodityData CashData;
				TCashPackageData PackageData;
				for (int i = 0; i < pCash->cRewardCount; i++)
				{
					if (pCash->nRewardItemSN[i] <= 0) continue;

					memset(&PackageData, 0, sizeof(TCashPackageData));
					bool bPackage = g_pExtManager->GetCashPackageData(pCash->nRewardItemSN[i], PackageData);
					if (bPackage)
					{
						for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++){
							memset(&CashData, 0, sizeof(TCashCommodityData));

							bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
							if (!bRet) continue;

							if (!ItemIDStr.empty()){
								ItemIDStr.append("|");
								ItemOptionStr.append("|");
							}
							ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
							ItemOptionStr.append(boost::lexical_cast<std::string>(static_cast<int>(cItemOption)));
						}

						Cash.nRetCode = pMembershipDB->QueryMakeGiftByQuest(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->nQuestID, pCash->wszMemo, pCash->nLifeSpan, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
					}
					else{
						memset(&CashData, 0, sizeof(TCashCommodityData));
						bool bRet = g_pExtManager->GetCashCommodityData(pCash->nRewardItemSN[i], CashData);
						if (!bRet) continue;

						ItemIDStr = FormatA("%d", CashData.nItemID[0]);
						ItemOptionStr = FormatA("%d", static_cast<int>(cItemOption));
						Cash.nRetCode = pMembershipDB->QueryMakeGiftByQuest(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->nQuestID, pCash->wszMemo, pCash->nLifeSpan, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
					}
				}

				if (Cash.nRetCode == ERROR_NONE) 
					pMembershipDB->QueryGetCountNotReceivedGift(pCash->biCharacterDBID, Cash.nGiftCount);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFTBYQUEST:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFTBYQUEST:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAMakeGiftByQuest));
		}
		break;

	case QUERY_MAKEGIFTBYMISSION:
		{
			TQMakeGiftByMission *pCash = (TQMakeGiftByMission*)pData;

			TAMakeGiftByMission Cash;
			memset(&Cash, 0, sizeof(TAMakeGiftByMission));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				string ItemIDStr, ItemOptionStr;
				BYTE cItemOption = -1;

				INT64 biPurchaseOrderID = 0;
				TCashCommodityData CashData;
				TCashPackageData PackageData;
				for (int i = 0; i < pCash->cRewardCount; i++){
					if (pCash->nRewardItemSN[i] <= 0) continue;

					memset(&PackageData, 0, sizeof(TCashPackageData));
					bool bPackage = g_pExtManager->GetCashPackageData(pCash->nRewardItemSN[i], PackageData);
					if (bPackage){
						for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++)
						{
							memset(&CashData, 0, sizeof(TCashCommodityData));
							bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
							if (!bRet) continue;

							if (!ItemIDStr.empty()){
								ItemIDStr.append("|");
								ItemOptionStr.append("|");
							}
							ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
							ItemOptionStr.append(boost::lexical_cast<std::string>(static_cast<int>(cItemOption)));
						}

						Cash.nRetCode = pMembershipDB->QueryMakeGiftByMission(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->nMissionID, pCash->wszMemo, pCash->nLifeSpan, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
					}
					else
					{
						memset(&CashData, 0, sizeof(TCashCommodityData));
						bool bRet = g_pExtManager->GetCashCommodityData(pCash->nRewardItemSN[i], CashData);
						if (!bRet) continue;

						ItemIDStr = FormatA("%d", CashData.nItemID[0]);
						ItemOptionStr = FormatA("%d", static_cast<int>(cItemOption));
						Cash.nRetCode = pMembershipDB->QueryMakeGiftByMission(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->nMissionID, pCash->wszMemo, pCash->nLifeSpan, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
					}
				}

				if (Cash.nRetCode == ERROR_NONE) 
					pMembershipDB->QueryGetCountNotReceivedGift(pCash->biCharacterDBID, Cash.nGiftCount);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFTBYMISSION:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFTBYMISSION:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAMakeGiftByMission));
		}
		break;

	case QUERY_MAKEGIFT:
		{
			TQMakeGift *pCash = (TQMakeGift*)pData;

			TAMakeGift Cash;
			memset(&Cash, 0, sizeof(TAMakeGift));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				BYTE cItemOption = -1;
				string ItemIDStr, ItemOptionStr;
#ifdef PRE_ADD_LIMITED_CASHITEM
				string LimitedQuantityStr;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

				INT64 biPurchaseOrderID = 0;
				TCashCommodityData CashData;
				TCashPackageData PackageData;
				for (int i = 0; i < pCash->cRewardCount; i++){
					if (pCash->nRewardItemSN[i] <= 0) continue;

					memset(&PackageData, 0, sizeof(TCashPackageData));
					bool bPackage = g_pExtManager->GetCashPackageData(pCash->nRewardItemSN[i], PackageData);
					if (bPackage){
						for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++){
							memset(&CashData, 0, sizeof(TCashCommodityData));
							bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
							if (!bRet) continue;

							if (!ItemIDStr.empty()){
								ItemIDStr.append("|");
								ItemOptionStr.append("|");
							}
							ItemIDStr.append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
							ItemOptionStr.append(boost::lexical_cast<std::string>(static_cast<int>(cItemOption)));
						}
#ifdef PRE_ADD_LIMITED_CASHITEM
						LimitedQuantityStr.append(boost::lexical_cast<std::string>(0));
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#ifdef PRE_ADD_LIMITED_CASHITEM
						Cash.nRetCode = pMembershipDB->QueryMakeGift(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), LimitedQuantityStr.c_str(), pCash->wszMemo, pCash->nLifeSpan, pCash->cPayMethodCode, pCash->iOrderKey, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
						Cash.nRetCode = pMembershipDB->QueryMakeGift(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->wszMemo, pCash->nLifeSpan, pCash->cPayMethodCode, pCash->iOrderKey, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
					}
					else{
						memset(&CashData, 0, sizeof(TCashCommodityData));
						bool bRet = g_pExtManager->GetCashCommodityData(pCash->nRewardItemSN[i], CashData);
						if (!bRet) continue;

						ItemIDStr = FormatA("%d", CashData.nItemID[0]);
						ItemOptionStr = FormatA("%d", static_cast<int>(cItemOption));
#ifdef PRE_ADD_LIMITED_CASHITEM
						LimitedQuantityStr = "0";
						Cash.nRetCode = pMembershipDB->QueryMakeGift(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), LimitedQuantityStr.c_str(),pCash->wszMemo, pCash->nLifeSpan, pCash->cPayMethodCode, pCash->iOrderKey, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
						Cash.nRetCode = pMembershipDB->QueryMakeGift(pCash->biCharacterDBID, pCash->bPCBang, pCash->nRewardItemSN[i], ItemIDStr.c_str(), ItemOptionStr.c_str(), pCash->wszMemo, pCash->nLifeSpan, pCash->cPayMethodCode, pCash->iOrderKey, pCash->szIp, pCash->bNewFlag, biPurchaseOrderID);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
					}

					// Added detailed log to detect cases where level-up reward is not received
					if( DBDNWorldDef::PayMethodCode::LevelupEvent == pCash->cPayMethodCode )
						g_Log.Log(LogType::_NORMAL, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[CDBID:%I64d] [QUERY_MAKEGIFT:%d] LevelupEvent(RewardItemSN:%d, RetCode:%d)\r\n", pCash->biCharacterDBID, pCash->cWorldSetID, pCash->nRewardItemSN[i], Cash.nRetCode );
				}

				if (Cash.nRetCode == ERROR_NONE) 
					pMembershipDB->QueryGetCountNotReceivedGift(pCash->biCharacterDBID, Cash.nGiftCount);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFT:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MAKEGIFT:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAMakeGift));
		}
		break;

	case QUERY_INCREASEVIPPOINT:
		{
			TQIncreaseVIPPoint *pCash = (TQIncreaseVIPPoint*)pData;

			TAIncreaseVIPPoint Cash;
			memset(&Cash, 0, sizeof(TAIncreaseVIPPoint));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryIncreaseVIPBasicPoint(pCash->biCharacterDBID, pCash->nBasicPoint, pCash->biPurchaseOrderID, pCash->wVIPPeriod, pCash->bAutoPay, Cash.nVIPTotalPoint, Cash.tVIPEndDate);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_INCREASEVIPPOINT:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_INCREASEVIPPOINT:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TQIncreaseVIPPoint));
		}
		break;

	case QUERY_GETVIPPOINT:
		{
			TQGetVIPPoint *pCash = (TQGetVIPPoint*)pData;

			TAGetVIPPoint Cash;
			memset(&Cash, 0, sizeof(TAGetVIPPoint));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryGetVIPPoint(pCash->biCharacterDBID, Cash.nVIPTotalPoint, Cash.tVIPEndDate, Cash.bAutoPay);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GETVIPPOINT:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GETVIPPOINT:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TQGetVIPPoint));
		}
		break;

	case QUERY_MODVIPAUTOPAYFLAG:
		{
			TQModVIPAutoPayFlag *pCash = (TQModVIPAutoPayFlag*)pData;

			TAModVIPAutoPayFlag Cash;
			memset(&Cash, 0, sizeof(TAModVIPAutoPayFlag));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.bAutoPay = pCash->bAutoPay;
			Cash.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryModVIPAutoPayFlag(pCash->biCharacterDBID, pCash->bAutoPay);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MODVIPAUTOPAYFLAG:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_MODVIPAUTOPAYFLAG:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TQModVIPAutoPayFlag));
		}
		break;

	case QUERY_GIFTBYCHEAT:
		{
			TQGiftByCheat *pCash = (TQGiftByCheat*)pData;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				INT64 biOrderID = 0;				

				std::vector<string> VecItemString;
				VecItemString.resize(Append_Max);

				BYTE cItemOption = -1;
				TCashPackageData PackageData;
				bool bPackage= g_pExtManager->GetCashPackageData(pCash->nItemSN, PackageData);
				if (bPackage){
					VecItemString[Append_ItemSN] = FormatA("%d", pCash->nItemSN);
					VecItemString[Append_Price] = FormatA("%d", g_pExtManager->GetCashCommodityPrice(pCash->nItemSN));
					VecItemString[Append_Limit] = FormatA("%d", g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN));

					for (int j = 0; j < (int)PackageData.nVecCommoditySN.size(); j++){
						TCashCommodityData CashData;
						bool bRet = g_pExtManager->GetCashCommodityData(PackageData.nVecCommoditySN[j], CashData);
						if (!bRet) continue;

						if (!VecItemString[Append_ItemID].empty()){
							VecItemString[Append_ItemID].append("|");
							VecItemString[Append_Option].append("|");
						}
						VecItemString[Append_ItemID].append(boost::lexical_cast<std::string>(CashData.nItemID[0]));
						VecItemString[Append_Option].append(boost::lexical_cast<std::string>(static_cast<int>(cItemOption)));
					}

					pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, false, VecItemString, g_pExtManager->GetCashCommodityPrice(pCash->nItemSN),
						pCash->szIp, true, pCash->wszCharacterName, L"", biOrderID, NULL, true);
				}
				else{
					int nItemID = g_pExtManager->GetCashCommodityItem0(pCash->nItemSN);
					BYTE nItemOption = -1;

					TItemData *pItemData = g_pExtManager->GetItemData(nItemID);
					if (!pItemData)
					{
						g_Log.Log(LogType::_ERROR, L"[QUERY_GIFTBYCHEAT] item data not found. [item id:%d]\r\n", nItemID);
						break;
					}

					switch(pItemData->nType)
					{
					case ITEMTYPE_WEAPON:
					case ITEMTYPE_PARTS:
						{
							int nPotentialID = pItemData->nTypeParam[0];
							if (nPotentialID > 0){
								cItemOption = 0;
							}
						}
						break;
					}

#ifdef PRE_ADD_LIMITED_CASHITEM
					pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, false, pCash->nItemSN, g_pExtManager->GetCashCommodityPrice(pCash->nItemSN), g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
						nItemID, cItemOption, 0, g_pExtManager->GetCashCommodityPrice(pCash->nItemSN), pCash->szIp, true, pCash->wszCharacterName, L"", biOrderID, NULL, NULL, true);
#else		//#ifdef PRE_ADD_LIMITED_CASHITEM
					pMembershipDB->QueryAddPurchaseOrderByCash(pCash->biCharacterDBID, false, pCash->nItemSN, g_pExtManager->GetCashCommodityPrice(pCash->nItemSN), g_pExtManager->GetCashBuyAbleCount(pCash->nItemSN), 
						nItemID, cItemOption, g_pExtManager->GetCashCommodityPrice(pCash->nItemSN), pCash->szIp, true, pCash->wszCharacterName, L"", biOrderID, NULL, NULL, true);
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
				}
				pMembershipDB->QuerySetPurchaseOrderResult(biOrderID, DBDNMembership::OrderStatusCode::Success, "", 0, 0, 0);
			}			
		}
		break;

	case QUERY_PETAL:
		{
			TQPetal *pCash = (TQPetal*)pData;

			TAPetal Cash;
			memset(&Cash, 0, sizeof(Cash));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;
			Cash.nUseItemID = pCash->nUseItemID;
			Cash.cInvenType = pCash->cInvenType;
			Cash.cInvenIndex = pCash->cInvenIndex;
			Cash.biInvenSerial = pCash->biInvenSerial;

			pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, pCash->cWorldSetID);
			if (pWorldDB){
#if defined(PRE_FIX_PETALGIFTCARD)
				pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
				if (pMembershipDB){
					Cash.nRetCode = pMembershipDB->QueryAddPetalIncome(pCash->biCharacterDBID, pCash->biInvenSerial, pCash->nAddPetal, Cash.nTotalPetal);
					if (Cash.nRetCode != ERROR_NONE)
						g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_PETAL:%d] Result:%d\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);
					else
					{
						// Delete item
						TQUseItem UseItem;
						memset( &UseItem, 0, sizeof(UseItem) );

						UseItem.cThreadID		= pCash->cThreadID;
						UseItem.cWorldSetID		= pCash->cWorldSetID;
						UseItem.nAccountDBID	= pCash->nAccountDBID;
						UseItem.Code			= DBDNWorldDef::UseItem::Use;
						UseItem.biItemSerial	= pCash->biInvenSerial;
						UseItem.nUsedItemCount	= 1;
						UseItem.iMapID			= pCash->iMapID;
						_wcscpy( UseItem.wszIP, _countof(UseItem.wszIP), pCash->wszIP, (int)wcslen(pCash->wszIP) );

						CDNMessageTask* pTask = m_pConnection->GetMessageTask(MAINCMD_ITEM);
						if (pTask)
							pTask->OnRecvMessage(pCash->cThreadID, MAINCMD_ITEM, QUERY_USEITEM, reinterpret_cast<char*>(&UseItem));
					}
				}
				else{
					g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_PETAL] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
				}

#else	// #if defined(PRE_FIX_PETALGIFTCARD)
				BYTE cItemCode = 0;
				INT64 biFKey = 0;
				nRet = pWorldDB->QueryItemMaterializeFKey(pCash->biInvenSerial, cItemCode, biFKey);
				if (nRet == ERROR_NONE){
					pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
					if (pMembershipDB){
						Cash.nRetCode = pMembershipDB->QueryAddPetalIncome(pCash->biCharacterDBID, biFKey, pCash->nAddPetal, Cash.nTotalPetal);
						if (Cash.nRetCode != ERROR_NONE)
							g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_PETAL:%d] Result:%d\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);
						else
						{
							// Delete item
							TQUseItem UseItem;
							memset( &UseItem, 0, sizeof(UseItem) );

							UseItem.cThreadID		= pCash->cThreadID;
							UseItem.cWorldSetID		= pCash->cWorldSetID;
							UseItem.nAccountDBID	= pCash->nAccountDBID;
							UseItem.Code			= DBDNWorldDef::UseItem::Use;
							UseItem.biItemSerial	= pCash->biInvenSerial;
							UseItem.nUsedItemCount	= 1;
							UseItem.iMapID			= pCash->iMapID;
							_wcscpy( UseItem.wszIP, _countof(UseItem.wszIP), pCash->wszIP, (int)wcslen(pCash->wszIP) );

							CDNMessageTask* pTask = m_pConnection->GetMessageTask(MAINCMD_ITEM);
							if (pTask)
								pTask->OnRecvMessage(pCash->cThreadID, MAINCMD_ITEM, QUERY_USEITEM, reinterpret_cast<char*>(&UseItem));
						}
					}
					else{
						g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_PETAL] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
					}
				}
#endif	// #if defined(PRE_FIX_PETALGIFTCARD)
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_PETAL] WorldDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(Cash));
		}
		break;

#if defined(PRE_ADD_GIFT_RETURN)
	case QUERY_GIFTRETURN:
		{
			TQGiftReturn *pCash = (TQGiftReturn*)pData;
			TAGiftReturn Cash;
			memset(&Cash, 0, sizeof(TAGiftReturn));

			Cash.nAccountDBID = pCash->nAccountDBID;
			Cash.nRetCode = ERROR_DB;			
			Cash.biPurchaseOrderID = pCash->biPurchaseOrderID;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB){
				Cash.nRetCode = pMembershipDB->QueryModGiftRejectFlag(pCash->biPurchaseOrderID, &Cash);
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GIFTRETURN:%d] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}

			if (Cash.nRetCode != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_GIFTRETURN:%d] (Ret:%d)\r\n", pCash->nAccountDBID, pCash->cWorldSetID, Cash.nRetCode);

			m_pConnection->AddSendData(nMainCmd, nSubCmd, (char*)&Cash, sizeof(TAGiftReturn));
		}
		break;
#endif //#if defined(PRE_ADD_GIFT_RETURN)

#if defined(PRE_ADD_CASH_REFUND)
	case  QUERY_PAYMENTINVEN_LIST : // Request billing inventory
		{
			TQPaymentItemList* pItemList = (TQPaymentItemList*)pData;
			TAPaymentItemList PaymentItemList;
			TAPaymentPackageItemList PaymentPackageItemList;
			memset(&PaymentItemList, 0, sizeof(PaymentItemList));
			memset(&PaymentPackageItemList, 0, sizeof(PaymentPackageItemList));

			PaymentItemList.nAccountDBID = pItemList->nAccountDBID;
			PaymentItemList.nRetCode = ERROR_DB;
			PaymentPackageItemList.nAccountDBID = pItemList->nAccountDBID;
			PaymentPackageItemList.nRetCode = ERROR_DB;

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				int nRetCode = pMembershipDB->QueryGetListRefundableProducts(pItemList->biCharacterDBID, &PaymentItemList, &PaymentPackageItemList);
				PaymentItemList.nRetCode = nRetCode;
				PaymentPackageItemList.nRetCode = nRetCode;
			}
			else{
				g_Log.Log(LogType::_ERROR, pItemList->cWorldSetID, pItemList->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_PAYMENTINVEN_LIST:%d] MembershipDB not found\r\n", pItemList->nAccountDBID, pItemList->cWorldSetID);
			}

			int nLen = 0;
			// Single item
			if (PaymentItemList.nPaymentCount > 0){
				nLen = sizeof(TAPaymentItemList) - sizeof(PaymentItemList.PaymentItemList) + (sizeof(TPaymentItemInfoEx) * PaymentItemList.nPaymentCount);
				m_pConnection->AddSendData(nMainCmd, QUERY_PAYMENTINVEN_LIST, (char*)&PaymentItemList, nLen);
			}
			if (PaymentPackageItemList.nPaymentPackegeCount > 0){
				nLen = sizeof(TAPaymentPackageItemList) - sizeof(PaymentPackageItemList.PaymentPackageItemList) + (sizeof(TPaymentPackageItemInfoEx) * PaymentPackageItemList.nPaymentPackegeCount);
				m_pConnection->AddSendData(nMainCmd, QUERY_PAYMENTINVEN_PAKAGELIST, (char*)&PaymentPackageItemList, nLen);
			}
		}
		break;
#endif
	case QUERY_CHEAT_GIFTCLEAR :
		{
			TQCheatGiftClear* pCheatGiftClear = (TQCheatGiftClear*)pData;		
			
			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				pMembershipDB->QueryCheatGiftClear(pCheatGiftClear->biCharacterDBID);		
			}
			else{
				g_Log.Log(LogType::_ERROR, pCheatGiftClear->cWorldSetID, pCheatGiftClear->nAccountDBID, 0, 0, L"[ADBID:%d] [QUERY_CHEAT_GIFTCLEAR:%d] MembershipDB not found\r\n", pCheatGiftClear->nAccountDBID, pCheatGiftClear->cWorldSetID);
			}			
		}
		break;
#if defined( PRE_PVP_GAMBLEROOM )
	case QUERY_LOGCODE_PETAL :
		{
			TQAddPetalLogCode* pCash = (TQAddPetalLogCode*)pData;		

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				int nTotalPetal = 0;
				int nRetCode = pMembershipDB->QueryAddPetalIncome(pCash->biCharacterDBID, pCash->nUseItemID, pCash->nAddPetal, nTotalPetal, pCash->cLogCode, pCash->nGambleDBID);
				if (nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_LOGCODE_PETAL:%d] Result:%d\r\n", pCash->nAccountDBID, pCash->cWorldSetID, nRetCode);
				
			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_LOGCODE_PETAL] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}
		}
		break;
	case QUERY_USE_PETAL :
		{
			TQUsePetal* pCash = (TQUsePetal*)pData;		

			pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				int nTotalPetal = 0;
				int nRetCode = pMembershipDB->QueryUsePetal(pCash->biCharacterDBID, pCash->nDelPetal, pCash->cLogCode, pCash->nGambleDBID);
				if (nRetCode != ERROR_NONE)
					g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_USE_PETAL:%d] Result:%d\r\n", pCash->nAccountDBID, pCash->cWorldSetID, nRetCode);

			}
			else{
				g_Log.Log(LogType::_ERROR, pCash->cWorldSetID, pCash->nAccountDBID, pCash->biCharacterDBID, 0, L"[ADBID:%u] [QUERY_USE_PETAL] MembershipDB not found\r\n", pCash->nAccountDBID, pCash->cWorldSetID);
			}
		}
		break;
#endif
	}
}