#include "stdafx.h"
#include "DNScriptAPI.h"
#include "DNUserSession.h"
#include "DNUserSessionManager.h"
#include "DNFieldManager.h"
#include "DNNpcObject.h"
#include "DNNpcObjectManager.h"
#include "DNUserItem.h"
#include "DNGameDataManager.h"

#include "DNLogConnection.h"
#include "DnLuaAPIDefine.h"
#include "DNReplaceString.h"
#include "EtUIXML.h"

#include "DNMasterConnection.h"
#include "DNPartyManager.h"
#include "DNQuestManager.h"
#include "TimeSet.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "ReputationSystemRepository.h"
#include "NpcReputationProcessor.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRepository.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#include "DNGuildSystem.h"
#if !defined(_FINAL_BUILD)
extern CLog g_ScriptLog;
#endif	// #if !defined(_FINAL_BUILD)
#include "DNGuildWarManager.h"
#include "DNGuildVillage.h"
#include "DNPeriodQuestSystem.h"
#include "DNScriptAPICommon.h"

int CheckInvenForQuestReward(CDNUserBase* pUser, const TQuestReward& QuestReward, const bool bSelectedArray[])
{
	return DNScriptAPI::CheckInvenForQuestReward(pUser, QuestReward, bSelectedArray);
}

void DefAllAPIFunc(lua_State* pLuaState)
{
	DefineLuaAPI(pLuaState);

	LUA_TINKER_DEF(pLuaState, api_npc_NextTalk);
	LUA_TINKER_DEF(pLuaState, api_npc_NextScript);
	
	LUA_TINKER_DEF(pLuaState, api_npc_SetParamString);
	LUA_TINKER_DEF(pLuaState, api_npc_SetParamInt);
	LUA_TINKER_DEF(pLuaState, api_log_AddLog);
	LUA_TINKER_DEF(pLuaState, api_log_UserLog);
	LUA_TINKER_DEF(pLuaState, api_quest_DumpQuest);
	
	LUA_TINKER_DEF(pLuaState, api_quest_AddHuntingQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_AddQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_UserHasQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_GetPlayingQuestCnt);
	
	LUA_TINKER_DEF(pLuaState, api_quest_CompleteQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_MarkingCompleteQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_IsMarkingCompleteQuest);
	
	
	LUA_TINKER_DEF(pLuaState, api_npc_GetNpcIndex);
	LUA_TINKER_DEF(pLuaState, api_quest_SetQuestStepAndJournalStep);
	LUA_TINKER_DEF(pLuaState, api_quest_SetQuestStep);
	LUA_TINKER_DEF(pLuaState, api_quest_GetQuestStep);
	LUA_TINKER_DEF(pLuaState, api_quest_SetJournalStep);
	LUA_TINKER_DEF(pLuaState, api_quest_GetJournalStep);
	LUA_TINKER_DEF(pLuaState, api_quest_SetQuestMemo);
	LUA_TINKER_DEF(pLuaState, api_quest_GetQuestMemo);
	LUA_TINKER_DEF(pLuaState, api_quest_CancelQuest);
	

	LUA_TINKER_DEF(pLuaState, api_quest_SetCountingInfo);
	LUA_TINKER_DEF(pLuaState, api_quest_ClearCountingInfo);
	LUA_TINKER_DEF(pLuaState, api_quest_IsAllCompleteCounting);
	LUA_TINKER_DEF(pLuaState, api_user_CheckInvenForAddItem);
	LUA_TINKER_DEF(pLuaState, api_user_CheckInvenForAddItemList);
	LUA_TINKER_DEF(pLuaState, api_user_AddItem);
	LUA_TINKER_DEF(pLuaState, api_user_DelItem);
	LUA_TINKER_DEF(pLuaState, api_user_AllDelItem);
	LUA_TINKER_DEF(pLuaState, api_user_HasItem);

	LUA_TINKER_DEF(pLuaState, api_user_GetUserClassID);
	LUA_TINKER_DEF(pLuaState, api_user_GetUserJobID);
	LUA_TINKER_DEF(pLuaState, api_user_GetUserLevel);
	LUA_TINKER_DEF(pLuaState, api_user_GetUserInvenBlankCount);
	LUA_TINKER_DEF(pLuaState, api_quest_GetUserQuestInvenBlankCount);
	
	LUA_TINKER_DEF(pLuaState, api_user_AddExp);
	LUA_TINKER_DEF(pLuaState, api_user_AddCoin);
	LUA_TINKER_DEF(pLuaState, api_user_PlayCutScene);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenWareHouse);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenShop);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenSkillShop);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenMarket);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenCompoundEmblem);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenUpgradeJewel);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenMailBox);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenUpgradeItem);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenDisjointItem);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenCompoundItem);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenCashShop);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenGuildMgrBox);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenGacha_JP);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenGiveNpcPresent);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenDarkLairRankBoard);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenPvPLadderRankBoard);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenGlyphLift);
#ifdef PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG
	LUA_TINKER_DEF(pLuaState, api_ui_OpenTextureDialog);
	LUA_TINKER_DEF(pLuaState, api_ui_CloseTextureDialog);
#endif
	LUA_TINKER_DEF(pLuaState, api_user_UserMessage);
	LUA_TINKER_DEF(pLuaState, api_quest_AddSymbolItem);
	LUA_TINKER_DEF(pLuaState, api_quest_DelSymbolItem);
	LUA_TINKER_DEF(pLuaState, api_quest_HasSymbolItem);
	
	LUA_TINKER_DEF(pLuaState, api_quest_CheckQuestInvenForAddItem);
	LUA_TINKER_DEF(pLuaState, api_quest_CheckQuestInvenForAddItemList);
	LUA_TINKER_DEF(pLuaState, api_quest_AddQuestItem);
	LUA_TINKER_DEF(pLuaState, api_quest_DelQuestItem);
	LUA_TINKER_DEF(pLuaState, api_quest_AllDelQuestItem);
	LUA_TINKER_DEF(pLuaState, api_quest_HasQuestItem);

	LUA_TINKER_DEF(pLuaState, api_ui_OpenGuildCreate);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenQuestReward);
	LUA_TINKER_DEF(pLuaState, api_quest_RewardQuestUser);

	LUA_TINKER_DEF(pLuaState, api_user_ChangeChannel);
	LUA_TINKER_DEF(pLuaState, api_user_ChangeMap);
	LUA_TINKER_DEF(pLuaState, api_user_GetPartymemberCount);
	LUA_TINKER_DEF(pLuaState, api_user_IsPartymember);

	LUA_TINKER_DEF(pLuaState, api_user_RequestEnterPVP);

	LUA_TINKER_DEF(pLuaState, api_user_GetStageConstructionLevel);
	LUA_TINKER_DEF(pLuaState, api_user_GetMapIndex);
	LUA_TINKER_DEF(pLuaState, api_user_GetLastStageClearRank);
	LUA_TINKER_DEF(pLuaState, api_user_EnoughCoin );
	LUA_TINKER_DEF(pLuaState, api_user_GetCoin);
	LUA_TINKER_DEF(pLuaState, api_user_DelCoin);
	LUA_TINKER_DEF(pLuaState, api_user_IsMissionGained);
	LUA_TINKER_DEF(pLuaState, api_user_IsMissionAchieved);
	LUA_TINKER_DEF(pLuaState, api_user_HasItemWarehouse);
	LUA_TINKER_DEF(pLuaState, api_user_HasItemEquip);
	LUA_TINKER_DEF(pLuaState, api_guild_GetGuildMemberRole);
	LUA_TINKER_DEF(pLuaState, api_quest_IncQuestMemo);
	LUA_TINKER_DEF(pLuaState, api_quest_DecQuestMemo);
	LUA_TINKER_DEF(pLuaState, api_user_SetUserJobID);
	LUA_TINKER_DEF(pLuaState, api_user_IsJobInSameLine);
	LUA_TINKER_DEF(pLuaState, api_user_HasCashItem);
	LUA_TINKER_DEF(pLuaState, api_user_HasCashItemEquip);
	LUA_TINKER_DEF(pLuaState, api_quest_IncCounting);
	LUA_TINKER_DEF(pLuaState, api_quest_IsPlayingQuestMaximum);
	LUA_TINKER_DEF(pLuaState, api_quest_ForceCompleteQuest);
	LUA_TINKER_DEF(pLuaState, api_quest_ResetSkill);

	LUA_TINKER_DEF(pLuaState, api_quest_GetWorldEventStep);
	LUA_TINKER_DEF(pLuaState, api_quest_GetWorldEventCount);
	LUA_TINKER_DEF(pLuaState, api_quest_ReturnItemToNpc);
	LUA_TINKER_DEF(pLuaState, api_quest_OpenScoreWorldEvent);
	LUA_TINKER_DEF(pLuaState, api_quest_OpenFarmAreaList);

	LUA_TINKER_DEF(pLuaState, api_user_ChangePos);

	LUA_TINKER_DEF(pLuaState, api_npc_GetFavorPoint);
	LUA_TINKER_DEF(pLuaState, api_npc_GetMalicePoint);
	LUA_TINKER_DEF(pLuaState, api_npc_GetFavorPercent);
	LUA_TINKER_DEF(pLuaState, api_npc_GetMalicePercent);
	LUA_TINKER_DEF(pLuaState, api_npc_AddFavorPoint);
	LUA_TINKER_DEF(pLuaState, api_npc_AddMalicePoint);
	LUA_TINKER_DEF(pLuaState, api_npc_SendSelectedPresent);
	LUA_TINKER_DEF(pLuaState, api_npc_Rage);
	LUA_TINKER_DEF(pLuaState, api_npc_Disappoint);

	LUA_TINKER_DEF(pLuaState, api_user_ResetSkill);
	LUA_TINKER_DEF(pLuaState, api_npc_CreateSecondarySkill );
	LUA_TINKER_DEF(pLuaState, api_npc_CheckSecondarySkill );
	LUA_TINKER_DEF(pLuaState, api_ui_OpenGuildWareHouse );
	LUA_TINKER_DEF(pLuaState, api_guild_HasGuildWare );

	LUA_TINKER_DEF(pLuaState, api_ui_OpenMovieBrowser);

	LUA_TINKER_DEF(pLuaState, api_guildwar_IsPreparation );
	LUA_TINKER_DEF(pLuaState, api_guildwar_GetStep );
	LUA_TINKER_DEF(pLuaState, api_guildwar_GetTrialCurrentState );
	LUA_TINKER_DEF(pLuaState, api_guildWar_GetTrialStats );
	LUA_TINKER_DEF(pLuaState, api_guildWar_GetMatchList );
	LUA_TINKER_DEF(pLuaState, api_guildWar_GetPopularityVote );	
	LUA_TINKER_DEF(pLuaState, api_guildWar_GetFinalProgress );	

	LUA_TINKER_DEF(pLuaState, api_ui_OpenCompound2Item);
	LUA_TINKER_DEF(pLuaState, api_ui_OpenBrowser);

#if defined (PRE_ADD_DONATION)
	LUA_TINKER_DEF(pLuaState, api_ui_OpenDonation);
#endif // #if defined (PRE_ADD_DONATION)
	LUA_TINKER_DEF(pLuaState, api_ui_OpenInventory);
	LUA_TINKER_DEF(pLuaState, api_user_GetPCCafe);
	LUA_TINKER_DEF(pLuaState, api_user_SetPCCafeItem);
#if defined(PRE_ADD_REPAIR_NPC)
	LUA_TINKER_DEF(pLuaState, api_user_RepairItem);
#endif
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	LUA_TINKER_DEF(pLuaState, api_ui_OpenExchangeEnchant);
#endif
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	LUA_TINKER_DEF(pLuaState, api_env_CheckCloseGateByTime);
#endif
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	LUA_TINKER_DEF(pLuaState, api_ui_OpenCostumeRandomMix);
#endif
#if defined( PRE_DRAGONBUFF )
	LUA_TINKER_DEF(pLuaState, api_quest_WorldBuffCheck);
	LUA_TINKER_DEF(pLuaState, api_quest_ApplyWorldBuff);
#endif
	LUA_TINKER_DEF(pLuaState, api_quest_ForceSkipQuest);
	LUA_TINKER_DEF(pLuaState, api_user_IsDNGirl);

}

CDNUserSession* __GetUserObj(UINT nUserObjectID)
{
	return g_pUserSessionManager->FindUserSessionByObjectID(nUserObjectID);
}

// desc : Set an item into a specific empty inventory slot (P.S.> Used in api_user_CheckInvenForAddItemList)
bool __SetBlankInventorySlot(TInvenItemCnt pTempInven[INVENTORYMAX], const TItem& Item, int iIndex)
{
	DN_ASSERT(NULL != pTempInven,	"Invalid!");
	DN_ASSERT(0 != Item.nItemID,			"Invalid!");
	DN_ASSERT(0 < Item.wCount,			"Invalid!");
	DN_ASSERT(-1 < iIndex,			"Invalid!");

	if (pTempInven[iIndex].nItemID != 0) {
		return false;
	}

	pTempInven[iIndex].Set( Item.nItemID, Item.wCount, Item.bSoulbound, Item.cSealCount );

	return true;
}

// desc : Set a specified count of items into empty inventory slots (P.S.> Used in api_user_CheckInvenForAddItemList)
bool __SetBlankInventorySlotCount(TInvenItemCnt pTempInven[INVENTORYMAX], const TItem& Item, int nOverlapCount, int nCount /* Number of slots to fill */)
{
	DN_ASSERT(NULL != pTempInven,	"Invalid!");
	DN_ASSERT(0 != Item.nItemID,			"Invalid!");
	DN_ASSERT(0 < Item.wCount,			"Invalid!");
	DN_ASSERT(0 < nOverlapCount,	"Invalid!");
	DN_ASSERT(0 < nCount,			"Invalid!");

	// !!! Caution - Assumes checks were already verified before calling this function; only performs setup

	int nBundle = Item.wCount / nOverlapCount;
	int nRemain = Item.wCount % nOverlapCount;

	for (int iIndex = 0 ; INVENTORYMAX > iIndex ; ++iIndex) 
	{
		if (0 < nBundle) {
			if (pTempInven[iIndex].nItemID == 0) 
			{
				pTempInven[iIndex].Set( Item.nItemID, nOverlapCount, Item.bSoulbound, Item.cSealCount );
				--nBundle;
				continue;
			}
		}

		if (0 < nRemain) 
		{
			if( CDNUserItem::bIsSameItem( &Item, &pTempInven[iIndex] ) && nOverlapCount >= pTempInven[iIndex].wCount + nRemain )
			{
				pTempInven[iIndex].wCount += nRemain;
				nRemain = 0;
				continue;
			}
		}
	}

	if (0 < nBundle) 
	{
		return false;
	}
	if (0 < nRemain) 
	{
		for (int iIndex = 0 ; INVENTORYMAX > iIndex ; ++iIndex) 
		{
			if (pTempInven[iIndex].nItemID == 0) {
				pTempInven[iIndex].Set( Item.nItemID, nRemain, Item.bSoulbound, Item.cSealCount );
				nRemain = 0;
				break;
			}
		}
	}
	if (0 < nRemain) 
	{
		return false;
	}

	return true;
}

// desc : Stack-set an item into a specific inventory slot (P.S.> Used in api_user_CheckInvenForAddItemList)
bool __SetOverlapInventorySlot(TInvenItemCnt pTempInven[INVENTORYMAX], const TItem& Item, int nOverlapCount, int iIndex)
{
	DN_ASSERT(NULL != pTempInven,	"Invalid!");
	DN_ASSERT(0 != Item.nItemID,			"Invalid!");
	DN_ASSERT(0 < Item.wCount,			"Invalid!");
	DN_ASSERT(0 < nOverlapCount,	"Invalid!");
	DN_ASSERT(-1 < iIndex,			"Invalid!");

	if ( CDNUserItem::bIsDifferentItem( &Item, &pTempInven[iIndex] ) || nOverlapCount < pTempInven[iIndex].wCount + Item.wCount )
	{
		return false;
	}

	pTempInven[iIndex].wCount += Item.wCount;

	return true;
}

// desc : Find an empty slot in inventory (P.S.> Used in api_user_CheckInvenForAddItemList)
int __FindBlankInventorySlot(TInvenItemCnt pTempInven[INVENTORYMAX], int nTempInvenCount)
{
	DN_ASSERT(NULL != pTempInven,								"Invalid!");
	DN_ASSERT(CHECK_RANGE(nTempInvenCount, 0, INVENTORYMAX),	"Invalid!");

	for (int iIndex = 0 ; nTempInvenCount > iIndex ; ++iIndex) {
		if (pTempInven[iIndex].nItemID == 0) {
			return iIndex;
		}
	}

	return -1;
}

// desc : Count how many empty slots are in inventory (P.S.> Used in api_user_CheckInvenForAddItemList)
int __FindBlankInventorySlotCount(TInvenItemCnt pTempInven[INVENTORYMAX], int nTempInvenCount)
{
	DN_ASSERT(NULL != pTempInven,								"Invalid!");
	DN_ASSERT(CHECK_RANGE(nTempInvenCount, 0, INVENTORYMAX),	"Invalid!");

	int nCount = 0;
	for (int iIndex = 0 ; nTempInvenCount > iIndex ; ++iIndex) {
		if (pTempInven[iIndex].nItemID == 0) {
			++nCount;
		}
	}

	return nCount;
}

// desc : Find index of a countable item in inventory with enough remaining space to fit wCount (P.S.> Used in api_user_CheckInvenForAddItemList)
int __FindOverlapInventorySlot(TInvenItemCnt pTempInven[INVENTORYMAX], int nTempInvenCount, const TItem& Item)
{
	DN_ASSERT(NULL != pTempInven,								"Invalid!");
	DN_ASSERT(CHECK_RANGE(nTempInvenCount, 0, INVENTORYMAX),	"Invalid!");

	if ((Item.nItemID <= 0) || (Item.wCount <= 0)) DN_RETURN(-1);

	TItemData *pItemData = NULL;
	for (int iIndex = 0 ; nTempInvenCount > iIndex ; ++iIndex) {
		if (pTempInven[iIndex].nItemID == 0) {
			// Pass if empty slot
			continue;
		}

		pItemData = g_pDataManager->GetItemData(pTempInven[iIndex].nItemID);
		if (!pItemData) continue;

		if (pItemData->nOverlapCount == 1) continue;			// Skip if not overlapping
		if( CDNUserItem::bIsDifferentItem( &Item, &pTempInven[iIndex] ) ) continue;	// Not the same item

		if (pItemData->nOverlapCount >= pTempInven[iIndex].wCount + Item.wCount) {
			return iIndex;
		}
	}

	return -1;
}

// desc : Check if inventory has enough space to add items (P.S.> Used in api_user_CheckInvenForAddItemList)
bool __IsValidSpaceInventorySlotAndSet(TInvenItemCnt pTempInven[INVENTORYMAX], int nTempInvenCount, const TItem& Item )
{
	DN_ASSERT(NULL != pTempInven,								"Invalid!");
	DN_ASSERT(CHECK_RANGE(nTempInvenCount, 0, INVENTORYMAX),	"Invalid!");

	if ((Item.nItemID <= 0) || (Item.wCount <= 0)) 
		DN_RETURN(false);

	TItemData *pItem = g_pDataManager->GetItemData(Item.nItemID);
	if (!pItem) return false;

	if (ITEMTYPE_QUEST == pItem->nType) {
		return false;
	}

	int nCount = 0;
	if (pItem->nOverlapCount == 1){		// Non-overlapping item
		nCount = __FindBlankInventorySlotCount(pTempInven, nTempInvenCount);
		if (Item.wCount > nCount) {
			// Compare with empty slot
			return false;
		}

		return(__SetBlankInventorySlotCount(pTempInven, Item, 1, nCount));
	}
	else {
		// Stackable item (only consider clean insertion without worrying about remaining stacks of the same item in inventory)
		if (pItem->nOverlapCount < Item.wCount){		// When exceeding the stackable amount at once
			int nBundle = Item.wCount / pItem->nOverlapCount;
			if ((Item.wCount % pItem->nOverlapCount) > 0) nBundle++;

			nCount = __FindBlankInventorySlotCount(pTempInven, nTempInvenCount);
			if (nBundle > nCount) {
				// Compare bundle count with empty slots
				return false;
			}

			return(__SetBlankInventorySlotCount(pTempInven, Item, pItem->nOverlapCount, nCount));
		}
		else {
			int nBlank = __FindOverlapInventorySlot(pTempInven, nTempInvenCount, Item );	// Check if any existing item can fit all at once 
			if (nBlank < 0){
				nBlank = __FindBlankInventorySlot(pTempInven, nTempInvenCount);
				if (nBlank < 0) {
					return false;
				}

				return(__SetBlankInventorySlot(pTempInven, Item, nBlank));
			}

			return(__SetOverlapInventorySlot(pTempInven, Item, pItem->nOverlapCount, nBlank));
		}
	}

	return true;
}

// desc : Set an item into a specific empty quest inventory slot (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
bool __SetBlankQuestInventorySlot(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX], int nItemID, short wCount, int iIndex)
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");
	DN_ASSERT(0 != nItemID,				"Invalid!");
	DN_ASSERT(0 < wCount,				"Invalid!");
	DN_ASSERT(-1 < iIndex,				"Invalid!");

	if (pTempQuestInven[iIndex].nItemID != 0) {
		return false;
	}

	pTempQuestInven[iIndex].Set(nItemID, wCount);

	return true;
}

// desc : Set a specified count of items into empty quest inventory slots (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
bool __SetBlankQuestInventorySlotCount(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX], int nItemID, short wCount, int nOverlapCount, int nCount /* Number of slots to fill */)
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");
	DN_ASSERT(0 != nItemID,				"Invalid!");
	DN_ASSERT(0 < wCount,				"Invalid!");
	DN_ASSERT(0 < nOverlapCount,		"Invalid!");
	DN_ASSERT(0 < nCount,				"Invalid!");

	// !!! Caution - Assumes checks were already verified before calling this function; only performs setup

	int nBundle = wCount / nOverlapCount;
	int nRemain = wCount % nOverlapCount;

	for (int iIndex = 0 ; QUESTINVENTORYMAX > iIndex ; ++iIndex) {
		if (0 < nBundle) {
			if (pTempQuestInven[iIndex].nItemID == 0) {
				pTempQuestInven[iIndex].Set(nItemID, nOverlapCount);
				--nBundle;
				continue;
			}
		}

		if (0 < nRemain) {
			if (pTempQuestInven[iIndex].nItemID == nItemID &&
				nOverlapCount >= pTempQuestInven[iIndex].wCount + nRemain
				)
			{
				pTempQuestInven[iIndex].wCount += nRemain;
				nRemain = 0;
				continue;
			}
		}
	}

	if (0 < nBundle) {
		return false;
	}
	if (0 < nRemain) {
		for (int iIndex = 0 ; QUESTINVENTORYMAX > iIndex ; ++iIndex) {
			if (pTempQuestInven[iIndex].nItemID == 0) {
				pTempQuestInven[iIndex].Set(nItemID, nRemain);
				nRemain = 0;
				break;
			}
		}
	}
	if (0 < nRemain) {
		return false;
	}

	return true;
}

// desc : Stack-set an item into a specific quest inventory slot (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
bool __SetOverlapQuestInventorySlot(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX], int nItemID, short wCount, int nOverlapCount, int iIndex)
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");
	DN_ASSERT(0 != nItemID,				"Invalid!");
	DN_ASSERT(0 < wCount,				"Invalid!");
	DN_ASSERT(0 < nOverlapCount,		"Invalid!");
	DN_ASSERT(-1 < iIndex,				"Invalid!");

	if (pTempQuestInven[iIndex].nItemID != nItemID || 
		nOverlapCount < pTempQuestInven[iIndex].wCount + wCount
		)
	{
		return false;
	}

	pTempQuestInven[iIndex].wCount += wCount;

	return true;
}

// desc : Count how many empty slots are in quest inventory (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
int __FindBlankQuestInventorySlotCount(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX])
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");

	int nCount = 0;
	for (int iIndex = 0 ; QUESTINVENTORYMAX > iIndex ; ++iIndex) {
		if (pTempQuestInven[iIndex].nItemID == 0) {
			++nCount;
		}
	}

	return nCount;
}

// desc : Find an empty slot in quest inventory (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
int __FindBlankQuestInventorySlot(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX])
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");

	for (int iIndex = 0 ; QUESTINVENTORYMAX > iIndex ; ++iIndex) {
		if (pTempQuestInven[iIndex].nItemID == 0) {
			return iIndex;
		}
	}

	return -1;
}

// desc : Find index of a countable item in quest inventory with enough remaining space to fit wCount (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
int __FindOverlapQuestInventorySlot(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX], int nItemID, short wCount)
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");

	if ((nItemID <= 0) || (wCount <= 0)) DN_RETURN(-1);

	bool boFlag = false;
	TItemData *pQuestItem = NULL;
	for (int iIndex = 0 ; QUESTINVENTORYMAX > iIndex ; ++iIndex) {
		if (pTempQuestInven[iIndex].nItemID == 0) continue;	// Pass if empty slot

		pQuestItem = g_pDataManager->GetItemData(pTempQuestInven[iIndex].nItemID);
		if (!pQuestItem) continue;

		if (pQuestItem->nOverlapCount == 1) continue;				// Skip if not overlapping
		if (pTempQuestInven[iIndex].nItemID != nItemID) continue;	// Not the same item

		if (pQuestItem->nOverlapCount >= pTempQuestInven[iIndex].wCount + wCount){
			return iIndex;
		}
	}

	return -1;
}

// desc : Check if quest inventory has enough space to add items (P.S.> Used in api_quest_CheckQuestInvenForAddItemList)
bool __IsValidSpaceQuestInventorySlotAndSet(TQuestItemCnt pTempQuestInven[QUESTINVENTORYMAX], int nItemID, short wCount)
{
	DN_ASSERT(NULL != pTempQuestInven,	"Invalid!");

	if ((nItemID <= 0) || (wCount <= 0)) DN_RETURN(false);

	TItemData *pItem = g_pDataManager->GetItemData(nItemID);
	if (!pItem) return false;

	if (ITEMTYPE_QUEST != pItem->nType) {
		return false;
	}

	int nCount = 0;
	if (pItem->nOverlapCount == 1){		// Non-overlapping item
		nCount = __FindBlankQuestInventorySlotCount(pTempQuestInven);
		if (wCount > nCount) {
			// Compare with empty slot
			return false;
		}

		return(__SetBlankQuestInventorySlotCount(pTempQuestInven, nItemID, wCount, 1, nCount));
	}
	else {	// Overlapping item
		int nRemain = 0, nBlank = 0, nBundle = 0;

		int nCount = __FindBlankQuestInventorySlotCount(pTempQuestInven);	// Empty slot count

		if (pItem->nOverlapCount >= wCount){
			nRemain = wCount;
		}
		else {
			nBundle = wCount / pItem->nOverlapCount;
			nRemain = wCount % pItem->nOverlapCount;

			if (nBundle > nCount) return false;	// Insufficient space
		}

		if (nRemain > 0){
			nBlank = __FindOverlapQuestInventorySlot(pTempQuestInven, nItemID, nRemain);	// Whether there is space for leftover items
			if (nBlank < 0){	// No leftover space
				if ((nBundle + 1) > nCount) return false;	// Check if there is one more free slot...
			}
		}

		return(__SetBlankQuestInventorySlotCount(pTempQuestInven, nItemID, wCount, pItem->nOverlapCount, nCount));
	}

	return true;
}

void api_log_AddLog(const char* szLog)
{
	DNScriptAPI::api_log_AddLog(szLog);
}

void api_log_UserLog(UINT nUserObjectID, const char* szLog)
{
#ifndef _FINAL_BUILD
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return;

	DNScriptAPI::api_log_UserLog(pUser, szLog);
#endif // _FINAL_BUILD
}

void api_quest_DumpQuest(UINT nUserObjectID)
{
#ifndef _FINAL_BUILD
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return;

	DNScriptAPI::api_quest_DumpQuest(pUser);
#endif // _FINAL_BUILD
}

void api_npc_NextTalk(UINT nUserObjectID, UINT nNpcObjectID , const char* szTalkIndex, const char* szTargetFile)
{
	CDNUserBase* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return;

	DNScriptAPI::api_npc_NextTalk(pUser, nNpcObjectID, szTalkIndex, szTargetFile);
}

void api_npc_NextScript(UINT nUserObjectID, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser)
	{
		g_Log.Log(LogType::_ERROR, pUser, L"[CS_NPCTALK] (5-1) UID:%d, PAK:%d/%s/%s\n", nUserObjectID, nNpcObjectID, szTalkIndex,szTargetFile);
		return;
	}
	
	CDNNpcObject* pNpc = g_pFieldManager->GetNpcObjectByNpcObjID(pUser->GetChannelID(), nNpcObjectID);
	if ( !pNpc ) return;

	DNScriptAPI::api_npc_NextScript(pUser, pNpc, nNpcObjectID, szTalkIndex, szTargetFile);
}

int api_npc_SetParamString(UINT nUserObjectID, const char* szParamKey, int nParamID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_npc_SetParamString(pUser, szParamKey, nParamID);
}

int api_npc_SetParamInt(UINT nUserObjectID, const char* szParamKey, int nValue)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_npc_SetParamInt(pUser, szParamKey, nValue);
}

int api_quest_AddHuntingQuest(UINT nUserObjectID, int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_quest_AddHuntingQuest(pUser, nQuestID, nQuestStep, nJournalStep, nCountingSlot, nCountingType, nCountingIndex, nTargetCnt);
}

// nQuestType : unused parameter.
int api_quest_AddQuest(UINT nUserObjectID, int nQuestID, int nQuestType)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_quest_AddQuest(pUser, nQuestID, nQuestType);
}

int api_quest_CompleteQuest(UINT nUserObjectID, int nQuestID, bool bDelPlayList, bool bRepeat)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_quest_CompleteQuest(pUser, nQuestID, bDelPlayList);
}

int api_quest_MarkingCompleteQuest(UINT nUserObjectID, int nQuestID)
{
	// This API is no longer used in order to unify reward quest marking into api_quest_CompleteQuest.
	return 1;
}

int api_quest_IsMarkingCompleteQuest(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_IsMarkingCompleteQuest(pUser, nQuestID);
}

int api_quest_UserHasQuest(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_UserHasQuest(pUser, nQuestID);
}

int api_quest_GetPlayingQuestCnt(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_GetPlayingQuestCnt(pUser);
}

int api_npc_GetNpcIndex(UINT nUserObjectID, UINT nNpcObjID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	CDNNpcObject* pNpc = g_pFieldManager->GetNpcObjectByNpcObjID(pUser->GetChannelID(), nNpcObjID);
	if ( !pNpc ) 
		return -2;

	return DNScriptAPI::api_npc_GetNpcIndex(pNpc);
}

int api_quest_SetQuestStepAndJournalStep(UINT nUserObjectID, int nQuestID, short nQuestStep, int nJournalStep)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_quest_SetQuestStepAndJournalStep(pUser, nQuestID, nQuestStep, nJournalStep);
}

int api_quest_SetQuestStep(UINT nUserObjectID, int nQuestID, short nQuestStep)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_SetQuestStep(pUser, nQuestID, nQuestStep);
}

int api_quest_GetQuestStep(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_GetQuestStep(pUser, nQuestID);
}

int api_quest_SetJournalStep(UINT nUserObjectID, int nQuestID, int nJournalStep)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_SetJournalStep(pUser, nQuestID, nJournalStep);
}

int api_quest_GetJournalStep(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_GetJournalStep(pUser, nQuestID);
}

int api_quest_SetQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex, TP_QUESTMEMO nVal)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return -1;
	}

	return DNScriptAPI::api_quest_SetQuestMemo(pUser, nQuestID, nMemoIndex, nVal);
}

int api_quest_GetQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return -1;
	}

	return DNScriptAPI::api_quest_GetQuestMemo(pUser, nQuestID, nMemoIndex);
}

int api_quest_SetCountingInfo(UINT nUserObjectID, int nQuestID, int nSlot, int nCountingType, int nCountingIndex, int nTargetCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_SetCountingInfo(pUser, nQuestID, nSlot, nCountingType, nCountingIndex, nTargetCnt);
}

int api_quest_ClearCountingInfo(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_ClearCountingInfo(pUser, nQuestID);
}

int api_quest_IsAllCompleteCounting(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_IsAllCompleteCounting(pUser, nQuestID);
}

int api_quest_CancelQuest(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	pUser->GetQuest()->OnCancelQuest(nQuestID);

	return 1;
}

int api_user_CheckInvenForAddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_CheckInvenForAddItem(pUser, nItemIndex, nItemCnt);
}

int api_user_CheckInvenForAddItemList(UINT nUserObjectID, lua_tinker::table ItemTable)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_CheckInvenForAddItemList(pUser, ItemTable);
}

int api_user_AddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_AddItem(pUser, nItemIndex, nItemCnt, nQuestID);
}

int api_user_DelItem(UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_DelItem(pUser, nItemIndex, nItemCnt, nQuestID);
}

int api_user_AllDelItem(UINT nUserObjectID, int nItemIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_AllDelItem(pUser, nItemIndex);
}

int api_user_HasItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_HasItem(pUser, nItemIndex, nItemCnt);
}

int api_user_GetUserClassID(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_user_GetUserClassID(pUser);
}

int api_user_GetUserJobID(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_user_GetUserJobID(pUser);
}

int api_user_GetUserLevel(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_user_GetUserLevel(pUser);
}

int api_user_GetUserInvenBlankCount(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_GetUserInvenBlankCount(pUser);
}

int api_quest_GetUserQuestInvenBlankCount(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_GetUserQuestInvenBlankCount(pUser);
}

/*
desc : Add EXP to the user.
param  : user index, EXP
return : -1 => user not found,
*/
int api_user_AddExp(UINT nUserObjectID, int nQuestID, int nAddExp)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;


	pUser->ChangeExp(nAddExp, DBDNWorldDef::CharacterExpChangeCode::Quest, nQuestID);

	int nExp = pUser->GetExp();

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Add EXP : %d total : %d" , nAddExp, nExp);
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif
	
	return 1;
}

/*
desc : Add money to the user.
param  : user index, money
return : -1 => user not found,
*/
int api_user_AddCoin(UINT nUserObjectID, int nAddCoin, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;


	if (!pUser->AddCoin(nAddCoin, DBDNWorldDef::CoinChangeCode::QuestReward, nQuestID))
		return -1;

	INT64 nCoin = pUser->GetCoin();

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Add Coin : %d total : %lld" , nAddCoin, nCoin);
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif

	return 1;
}

int api_user_PlayCutScene(UINT nUserObjectID, UINT nNpcObjectID, int nCutSceneTableID, bool bIgnoreFadeIn)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_PlayCutScene(pUser, nNpcObjectID, nCutSceneTableID, bIgnoreFadeIn);
}

int api_ui_OpenWareHouse(UINT nUserObjectID, int iItemID/*=0*/ )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenWareHouse(pUser,iItemID);
}

/*
desc : Notify the user to open the shop.
param  : user index, shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenShop(UINT nUserObjectID, int nShopID, Shop::Type::eCode Type/*=Shop::Type::Normal*/ )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;
#if defined (PRE_MOD_GAMESERVERSHOP)
	return DNScriptAPI::api_ui_OpenShop(pUser, nShopID, Type);
#else

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open shop");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	TShopData *pShopData = g_pDataManager->GetShopData(nShopID);
	if ( !pShopData )
	{
		if( false == g_pDataManager->IsCombinedShop( nShopID ) )
			return -2;
	}

#if defined(PRE_ADD_REMOTE_OPENSHOP)
	pUser->m_bRemoteShopOpen = false;
#endif	// #if defined(PRE_ADD_REMOTE_OPENSHOP)
	pUser->m_nShopID = nShopID;
	pUser->SendShopOpen(nShopID,Type);
	pUser->SetShopType( Type );
	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
#endif
}

/*
desc : Notify the user to open the skill shop.
param  : user index, skill shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenSkillShop( UINT nUserObjectID )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#if defined (PRE_MOD_GAMESERVERSHOP)
	return DNScriptAPI::api_ui_OpenSkillShop(pUser);
#else
	pUser->SendSkillShopOpen();
	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
#endif
}

/*
desc : Notify the user to open the unmanned shop.
param  : user index
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenMarket(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open trading house");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->SendMarketList(NULL, 0, 0, ERROR_NONE);
	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
}

int api_ui_OpenCompoundEmblem(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenCompoundEmblem(pUser);
}

int api_ui_OpenUpgradeJewel(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenUpgradeJewel(pUser);
}

int api_ui_OpenGlyphLift(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenGlyphLift(pUser);
}

int api_ui_OpenMailBox(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenMailBox(pUser);
}

/*
desc : Instruct the user to open the item enhancement window.
param  : user index
return : -1 => user not found, 1 => success
*/

int api_ui_OpenUpgradeItem(UINT nUserObjectID,int iItemID/*=0*/ )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open item enhancement UI");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->SendOpenUpgradeItem( iItemID );

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	pUser->SetRemoteEnchantItemID(iItemID);
	return 1;
}

int api_ui_OpenDisjointItem(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenDisjointItem(pUser);
}

int api_ui_OpenCompoundItem(UINT nUserObjectID, int nCompoundShopID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenCompoundItem(pUser, nCompoundShopID);
}

int api_ui_OpenCompound2Item(UINT nUserObjectID, int nCompoundGroupID, int iItemID/*=0*/ )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open item compound UI");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->SendOpenCompound2Item(nCompoundGroupID, iItemID );

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	pUser->SetRemoteItemCompoundItemID( iItemID );

	return 1;
}

int api_ui_OpenCashShop(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenCashShop(pUser);
}

int api_ui_OpenGuildMgrBox(UINT nUserObjectID, int nGuildMgrNo)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenGuildMgrBox(pUser, nGuildMgrNo);
}

int api_ui_OpenGacha_JP( UINT nUserObjectID, int nGachaShopID )
{
#ifdef PRE_ADD_GACHA_JAPAN
	CDNUserSession* pUser = __GetUserObj( nUserObjectID );
	if( !pUser )
		return -1;

	return DNScriptAPI::api_ui_OpenGacha_JP(pUser, nGachaShopID);
#else
	return -3;
#endif // PRE_ADD_GACHA_JAPAN
}

int api_ui_OpenGiveNpcPresent( UINT nUserObjectID, int nNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj( nUserObjectID );
	if( !pUser )
		return -1;

	return DNScriptAPI::api_ui_OpenGiveNpcPresent(pUser, nNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

/*
desc : Open the Dark Lair ranking board dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenDarkLairRankBoard( UINT nUserObjectID, int nNpcID, int iMapIndex, int iPlayerCount )
{
	CDNUserSession* pUser = __GetUserObj( nUserObjectID );
	if( !pUser )
		return -1;

	_ASSERT( iPlayerCount > 0 && iPlayerCount <= PARTYCOUNTMAX );

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open Dark Lair ranking board UI");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	// Send packet to client to open NPC gift UI.
	pUser->SendOpenDarkLairRankBoard( nNpcID, iMapIndex, iPlayerCount );

	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check
	// However, for NPC gifting, the conversation continues, so leave it as is. Calling this closes the talk dialog immediately.
	pUser->SetCalledNpcResponse( true, false );

	return 1;
}


/*
desc : Open the PvP ladder ranking board dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenPvPLadderRankBoard( UINT nUserObjectID, int nNpcID, int iPvPLadderCode )
{
	CDNUserSession* pUser = __GetUserObj( nUserObjectID );
	if( !pUser )
		return -1;

	_ASSERT( LadderSystem::bIsServiceMatchType( static_cast<LadderSystem::MatchType::eCode>(iPvPLadderCode) ) == true );

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open PvP Ladder ranking board UI");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	// Send packet to client to open NPC gift UI.
	pUser->SendOpenPvPLadderRankBoard( nNpcID, static_cast<LadderSystem::MatchType::eCode>(iPvPLadderCode) );

	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check
	// However, for NPC gifting, the conversation continues, so leave it as is. Calling this closes the talk dialog immediately.
	pUser->SetCalledNpcResponse( true, false );

	return 1;
}

#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
int api_ui_OpenTextureDialog(UINT nUserObjectID, int nFileIndex, float fX, float fY, int nPos, int nTime, int nDialogIndex, bool bAutoCloseDialog)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if(!pUser)
	{
		return -1;
	}
	
	SCOpenTextureDialog sDlgData;
	sDlgData.fX = fX;
	sDlgData.fY = fY;
	sDlgData.nTime = nTime;
	sDlgData.nPosition = nPos;
	sDlgData.nFileIndex = nFileIndex;
	sDlgData.nDialogIndex = nDialogIndex;
	sDlgData.bAutoCloseDialog = bAutoCloseDialog;

	return DNScriptAPI::api_Open_Texture_Dialog(pUser, sDlgData);
}

int api_ui_CloseTextureDialog(UINT nUserObjectID, int nDlgID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if(!pUser)
	{
		return -1;
	}

	return DNScriptAPI::api_Close_Texture_Dialog(pUser, nDlgID);
}
#endif

int api_user_UserMessage(UINT nUserObjectID, int nType, int nBaseStringIdx, lua_tinker::table ParamTable)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser )
	{
		return -1;
	}

	return DNScriptAPI::api_user_UserMessage(pUser, nType, nBaseStringIdx, ParamTable);
}

int api_quest_AddSymbolItem(UINT nUserObjectID, int nItemID, short wCount)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_AddSymbolItem(pUser, nItemID, wCount);
}

int api_quest_DelSymbolItem(UINT nUserObjectID, int nItemID, short wCount)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_DelSymbolItem(pUser, nItemID, wCount);
}

int api_quest_HasSymbolItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_HasSymbolItem(pUser, nItemIndex, nItemCnt);
}

int api_quest_CheckQuestInvenForAddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{

	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_CheckQuestInvenForAddItem(pUser, nItemIndex, nItemCnt);
}

int api_quest_CheckQuestInvenForAddItemList(UINT nUserObjectID, lua_tinker::table ItemTable)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_CheckQuestInvenForAddItemList(pUser, ItemTable);
}

int api_quest_AddQuestItem(UINT nUserObjectID, int nItemID, short wCount, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_AddQuestItem(pUser, nItemID, wCount, nQuestID);
}

int api_quest_DelQuestItem(UINT nUserObjectID, int nItemID, short wCount, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_DelQuestItem(pUser, nItemID, wCount, nQuestID);
}

int api_quest_AllDelQuestItem(UINT nUserObjectID, int nItemID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_AllDelQuestItem(pUser, nItemID);
}

int api_quest_HasQuestItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_quest_HasQuestItem(pUser, nItemIndex, nItemCnt);
}

int api_ui_OpenGuildCreate(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenGuildCreate(pUser);
}

int api_ui_OpenQuestReward(UINT nUserObjectID, int nRewardTableIndex, bool bActivate)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenQuestReward(pUser, nRewardTableIndex, bActivate);
}

int api_quest_RewardQuestUser(UINT nUserObjectID, int nRewardTableIndext, int nQuestID, int nRewardCheck)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_RewardQuestUser(pUser, nRewardTableIndext, nQuestID, nRewardCheck);
}

/*
desc : Move the user to another channel.
param  : user index, channel type
return : -1 => user not found, -2 invalid channel type, -3 invalid channel
*/

int api_user_ChangeChannel(UINT nUserObjectID, int ChannelType )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if( !pUser ) 
		return -1;

	int iAttr = 0;
	switch( ChannelType )
	{
		// PvP lobby
		case 1:
		{
			iAttr = GlobalEnum::CHANNEL_ATT_PVPLOBBY;
			break;
		}
		default:
		{
			return -2;
		}
	}
	const sChannelInfo* pChannelInfo = g_pMasterConnection->GetChannelInfoFromAttr( iAttr );
	if( pChannelInfo == NULL )
		return -3;
	pUser->CmdSelectChannel( pChannelInfo->nChannelID, true );

	return 0;
}

int api_user_ChangeMap(UINT nUserObjectID, int nMapIndex, int nGateNo)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	if (pUser->GetPartyID() > 0)
	{
		CDNParty * pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
		if (pParty)
		{
			int nPermitCnt = g_pDataManager->GetMapPermitPartyCount(nMapIndex);
			if (nPermitCnt > 0 && pParty->GetMemberCount() > nPermitCnt)
				return -1;
		}
	}

	if( pUser->IsWindowState( WINDOW_BLIND ) == false )
	{
		if (!pUser->IsNoneWindowState()) 
			return -1;
	}

	int nMapType = g_pDataManager->GetMapType( nMapIndex );
	switch( nMapType ) {
		case GlobalEnum::MAP_UNKNOWN:
			return -2;
		case GlobalEnum::MAP_VILLAGE:
			if( pUser->GetPartyID() > 0 ) {
				CDNParty *pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
				if (!pParty) return -1;
				if (g_pMasterConnection && g_pMasterConnection->GetActive())
					g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_PARTY, pParty->GetPartyID(), nMapIndex, -1, -1, pParty);	//pUser->GetLevel()
				else
					return -1;
			}
			else
			{
				if (g_pMasterConnection && g_pMasterConnection->GetActive())
					g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_SINGLE, pUser->GetAccountDBID(), nMapIndex, -1, -1);
				else
					return -1;
			}
			break;
		case GlobalEnum::MAP_WORLDMAP:
		case GlobalEnum::MAP_DUNGEON:
			{
				// If in a party, there is an issue of whether others should follow on script forced movement.
				// Enable movement for now, though it will surely break in some cases, so let's discuss and handle properly later.
				if( pUser->GetPartyID() > 0 ) {
					CDNParty *pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
					if (!pParty) return -1;
					pParty->SetRandomSeed( timeGetTime() );
					if (g_pMasterConnection && g_pMasterConnection->GetActive())
					{
						g_pMasterConnection->SendReqGameID( GameTaskType::Normal, REQINFO_TYPE_PARTY, pParty->GetPartyID(), pUser->GetChannelID(), pParty->GetRandomSeed(), nMapIndex, nGateNo, CONVERT_TO_DUNGEONDIFFICULTY(0), true, pParty );		// Request GameID
					}
					else
						return -1;
				}
				else {
					pUser->m_cGateNo = nGateNo;
					pUser->m_nSingleRandomSeed = timeGetTime();
					if (g_pMasterConnection && g_pMasterConnection->GetActive())
						g_pMasterConnection->SendReqGameID( GameTaskType::Normal, REQINFO_TYPE_SINGLE, pUser->GetAccountDBID(), pUser->GetChannelID(), pUser->m_nSingleRandomSeed, nMapIndex, 1, CONVERT_TO_DUNGEONDIFFICULTY(0), true );	// Request GameID
					else
						return -1;
				}
			}
			break;
	}
	return 0;
}

/*
desc   : Return the number of party members for the user.
param  : user index
return : -1 => user not found, otherwise party member count (Note: returns 1 even if user is not in a party)
*/
int api_user_GetPartymemberCount(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;
	if (pUser->GetPartyID() > 0)
	{
		//If it is a party, member count
		CDNParty* pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
		if (!pParty) return -1;

		return pParty->GetMemberCount();
	}
	
	// No party means solo, so 1
	return 1;
}

/*
desc   : Check if the user is in a party.
param  : user index
return : -1 => user not found, 0 => not in party, 1 => in party
*/
int api_user_IsPartymember(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;
	if (pUser->GetPartyID() > 0)
	{
		//If it is a party, member count
		CDNParty* pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
		if (!pParty) return -1;

		return 1;
	}

	// Return 0 if not in a party
	return 0;
}

//blondy
/*
desc   : Direct the user to the PVP lobby.
param  : user index
return : 1 => success, -1 => user not found
*/
int api_user_RequestEnterPVP( UINT nUserObjectID )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	pUser->SendPvPVillageToLobby();
	return 1;

}
//blondy end

/*
desc   : Get the user's stage difficulty (P.S.> Always fails on VillageServer)
param  : user index
return : 0 ~ 4 => dungeon difficulty (0:Easy / 1:Normal / 2:Hard / 3:Master / 4:Abyss), otherwise fail (default -1)
*/
int api_user_GetStageConstructionLevel(UINT nUserObjectID)
{
//	DN_ASSERT(0,	"Can't be Called!");

	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return -1;
}

int api_user_GetMapIndex(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_GetMapIndex(pUser);
}

int api_user_GetLastStageClearRank(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_GetLastStageClearRank(pUser);
}

int api_user_EnoughCoin(UINT nUserObjectID, int nCoin )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_EnoughCoin(pUser, nCoin);
}

/*
desc : Check the user's current money.
param  : user index
return : -1 => user not found, >= 0 => current money
*/
int api_user_GetCoin(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}


	return (int)(pUser->GetCoin());
}

/*
desc : Deduct money from the user.
param  : user index, money (> 0)
return : -1 => user not found, 1 => success
*/
int api_user_DelCoin(UINT nUserObjectID, int nDelCoin)
{
	DN_ASSERT(0 < nDelCoin,	"Invalid!");

	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}


	pUser->DelCoin(nDelCoin, DBDNWorldDef::CoinChangeCode::Use, 0);

	INT64 nCoin = pUser->GetCoin();

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Remove Coin : %d total : %lld" , nDelCoin, nCoin);
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif

	return 1;
}

int api_user_IsMissionGained(UINT nUserObjectID, int nMissionIndex)
{
	DN_ASSERT(CHECK_LIMIT(nMissionIndex, MISSIONMAX),	"Invalid!");

	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_IsMissionGained(pUser, nMissionIndex);
}

int api_user_IsMissionAchieved(UINT nUserObjectID, int nMissionIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_IsMissionAchieved(pUser, nMissionIndex);
}

int api_user_HasItemWarehouse(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_HasItemWarehouse(pUser, nItemIndex, nItemCnt);
}

int api_user_HasItemEquip(UINT nUserObjectID, int nItemIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_HasItemEquip(pUser, nItemIndex);
}

int api_guild_GetGuildMemberRole(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_guild_GetGuildMemberRole(pUser);
}

/*
desc : Return whether the user's current guild owns a guild storage.
param  : user index
return : -1 : user not found / 0 : no guild storage / 1 : has guild storage
*/
int api_guild_HasGuildWare(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	const TGuildUID GuildUID = pUser->GetGuildUID ();
	if (!GuildUID.IsSet())
		return -1;

	CDNGuildBase* pGuild = g_pGuildManager->At (GuildUID);
	if (!pGuild)
		return 0;


#if !defined( PRE_ADD_NODELETEGUILD )
	// Guild exists.
	CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
	if (FALSE == pGuild->IsEnable()) return 0;
#endif

	if (pGuild->GetWareSize() > 0)
		return 1;

	return 0;
}

int api_quest_IncQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return LONG_MIN;
	}

	return DNScriptAPI::api_quest_IncQuestMemo(pUser, nQuestID, nMemoIndex);
}

int api_quest_DecQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return LONG_MAX;
	}

	return DNScriptAPI::api_quest_DecQuestMemo(pUser, nQuestID, nMemoIndex);
}

int api_user_SetUserJobID(UINT nUserObjectID, int nJobID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return -1;
	}

	return DNScriptAPI::api_user_SetUserJobID(pUser, nJobID);
}

int api_user_IsJobInSameLine(UINT nUserObjectID, int nBaseJobID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) {
		return -1;
	}

	return DNScriptAPI::api_user_IsJobInSameLine(pUser, nBaseJobID);
}

int api_user_HasCashItem(UINT nUserObjectID, int nItemIndex, int nItemCnt)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_user_HasCashItem(pUser, nItemIndex, nItemCnt);
}

int api_user_HasCashItemEquip(UINT nUserObjectID, int nItemIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_user_HasCashItemEquip(pUser, nItemIndex);
}

int api_quest_IncCounting(UINT nUserObjectID, int nCountingType, int nCountingIndex)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_IncCounting(pUser, nCountingType, nCountingIndex);
}

int api_quest_IsPlayingQuestMaximum(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_IsPlayingQuestMaximum(pUser);
}

int api_quest_ForceCompleteQuest(UINT nUserObjectID, int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	return DNScriptAPI::api_quest_ForceCompleteQuest(pUser, nQuestID, nQuestCode, bDoMark, bDoDelete, bDoRepeat);
}

int api_quest_ResetSkill(UINT nUserObjectID, int nQuestID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	CDNUserQuest* pQuest = pUser->GetQuest();
	if (!pQuest) {
		return -2;
	}

	bool bResult = pQuest->IsClearQuest(nQuestID);

	if ( bResult )
		return -3;

#if defined(_VILLAGESERVER)
	pUser->GetSkill()->ResetSkill( pUser->GetSkillPage() );
#elif defined(_GAMESERVER)
	return -1;
#endif // #if defined(_VILLAGESERVER)

	return 1;
}

int api_quest_GetWorldEventStep(UINT nUserObjectID, int nScheduleID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if (!g_pPeriodQuestSystem)
		return -1;

	return g_pPeriodQuestSystem->GetWorldEventStep(pUser->GetWorldSetID(), nScheduleID);
}

int api_quest_GetWorldEventCount (UINT nUserObjectID, int nScheduleID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if (!g_pPeriodQuestSystem)
		return -1;

	return g_pPeriodQuestSystem->GetWorldEventCount(pUser->GetWorldSetID(), nScheduleID);
}

int api_quest_ReturnItemToNpc (UINT nUserObjectID, int nScheduleID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	int nCount = pUser->GetItem()->ReturnItemToNpc(nScheduleID);
	if (nCount <= 0)
		return -1;

	return nCount;
}

void api_quest_OpenScoreWorldEvent (UINT nUserObjectID, int nScheduleID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return;

	TGlobalEventData* pEvent = g_pDataManager->GetGlobalEventData(nScheduleID);
	if (!pEvent) 
		return;

	int nMaxCount = pEvent->nCollectTotalCount;
	int nCurrentCount = g_pPeriodQuestSystem->GetWorldEventCount(pUser->GetWorldSetID(), nScheduleID);
	
	if (nCurrentCount >= nMaxCount)
		nCurrentCount = nMaxCount;
	
	pUser->SendScorePeriodQuest(nCurrentCount, nMaxCount);
}

/*
desc   : Show the farm area list to a specific user
param  : user index
return : -1 : user not found, 1 : success
*/
int api_quest_OpenFarmAreaList (UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	pUser->SendFarmOpenList();

	pUser->ReqFarmInfo(false);

	return 1;
}

/*
desc   : Move a specific user's position.
param  : user index, X coord, Y coord, Z coord
return : -1 : user not found, 1 : success
*/
int api_user_ChangePos(UINT nUserObjectID, int nX, int nY, int nZ, int nLookX, int nLookY )
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	pUser->ChangePos(nX, nY, nZ, float(nLookX), float(nLookY));

	return 1;
}


int api_npc_GetFavorPoint( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if (!pUser)
		return -1;

	return DNScriptAPI::api_npc_GetFavorPoint(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_GetMalicePoint( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_GetMalicePoint(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_GetFavorPercent( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_GetFavorPercent(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_GetMalicePercent( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_GetMalicePercent(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_AddFavorPoint( UINT uiUserObjectID, int iNpcID, int val )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_AddFavorPoint(pUser, iNpcID, val);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_AddMalicePoint( UINT uiUserObjectID, int iNpcID, int val )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_AddMalicePoint(pUser, iNpcID, val);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_SendSelectedPresent( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_SendSelectedPresent(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_Rage( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_Rage(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_npc_Disappoint( UINT uiUserObjectID, int iNpcID )
{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if( !pUser )
		return -1;

	return DNScriptAPI::api_npc_Disappoint(pUser, iNpcID);
#else
	return -1;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
}

int api_user_ResetSkill(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser) {
		return -1;
	}

	pUser->GetItem()->ResetSkillFromQuestScript();

	return 1;
}

int api_npc_CreateSecondarySkill( UINT uiUserObjectID, int iSkillID )
{
#if defined( PRE_ADD_SECONDARY_SKILL )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if (!pUser) 
		return -1;

	CSecondarySkillRepository* pRepository = pUser->GetSecondarySkillRepository();
	if( pRepository == NULL )
		return 0;

	if( pRepository->Create( iSkillID ) == false )
		return 0;

	return 1;
#else
	return -1;
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
}

int api_npc_CheckSecondarySkill( UINT uiUserObjectID, int iSkillID )
{
#if defined( PRE_ADD_SECONDARY_SKILL )
	CDNUserSession* pUser = __GetUserObj(uiUserObjectID);
	if (!pUser) 
		return -1;

	CSecondarySkillRepository* pRepository = pUser->GetSecondarySkillRepository();
	if( pRepository == NULL )
		return 0;

	if( pRepository->Get( iSkillID ) == NULL )
		return 0;

	return 1;
#else
	return -1;
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
}

/*
desc : Notify the user to open the guild storage.
param : user index
return  : -1 => user not found, -2 => not a guild member, 1 => success
*/
int api_ui_OpenGuildWareHouse (UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);

	if ( !pUser ) 
		return -1;

	if (g_pGuildManager->OnRecvApiOpenGuildWare (pUser))
		pUser->SetCalledNpcResponse(true, false);	
	else
		return -2;

	return 1;
}


int api_ui_OpenMovieBrowser(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open movie clip browser");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->SendOpenMovieBrowser();

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
}

int api_ui_OpenBrowser(UINT nUserObjectID, const char* szUrl, int nX, int nY, int nWidth, int nHeight)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open NPC browser (%s)", szUrl);
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->SendOpenBrowser(szUrl, nX, nY, nWidth, nHeight);

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
}

/*
desc : Return whether registered for guild war. (Allowed during registration period only)
param  : user index
return : -1 : user not found / 0 : cannot apply for guild war / 1 : can apply for guild war
*/
int api_guildwar_IsPreparation(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if (g_pGuildWarManager)
	{
		if (g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_PREPARATION && pUser->GetGuildSelfView().IsSet())
		{
			if (pUser->GetGuildSelfView().btGuildRole == GUILDROLE_TYPE_MASTER && 
				pUser->GetGuildSelfView().wWarSchduleID != g_pGuildWarManager->GetScheduleID())
				return 1;
		}
	}
	return 0;
}
/*
desc : Check guild war period.
param  : user index
return : -1 : user not found / eGuildWarStepType / 0 : not guild war period / 1 : guild war registration period / 2 : guild war preliminaries period / 3 : finals and reward period
*/
int api_guildwar_GetStep(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if (g_pGuildWarManager)
		return g_pGuildWarManager->GetStepIndex();	
	return 0;
}
/*
desc : View guild war preliminary progress
param  : user index
return : -1 => user not found, -2 => not in preliminary period, 0 : success
*/
int api_guildwar_GetTrialCurrentState(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if( g_pGuildWarManager->GetStepIndex() != GUILDWAR_STEP_TRIAL)
		return -2;	

	int nGuildWarPoint = 0;
	CDNGuildBase* pGuild = g_pGuildManager->At(pUser->GetGuildUID());

	if (pGuild)
	{
#if !defined( PRE_ADD_NODELETEGUILD )
		CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
		if (FALSE == pGuild->IsEnable()) return -1;
#endif

		if( pGuild->IsEnrollGuildWar() )
		{
			CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
			if( pGuildVillage->GetGuildWarScheduleID() != g_pGuildWarManager->GetScheduleID() )
			{
				pGuildVillage->GuildWarReset();
				pGuildVillage->SetGuildWarScheduleID(g_pGuildWarManager->GetScheduleID());
			}
			nGuildWarPoint = pGuildVillage->GetGuildWarPoint();
		}
	}
	// Send open first (must send this since it shares UI with the prelim results window)
	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check
	pUser->SendGuildWarTrialResultUIOpen();
	pUser->SendGuildWarStatusOpen( g_pGuildWarManager->GetBlueTeamPoint(), g_pGuildWarManager->GetRedTeamPoint(), pUser->GetGuildWarPoint(), nGuildWarPoint);
	return 0;
}
/*
desc : Guild war preliminary results
param  : user index
return : -1 => user not found, -2 => not in finals or reward period, -3 => preliminary results not yet tallied, 0 : success
*/
int api_guildWar_GetTrialStats(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if( g_pGuildWarManager->GetStepIndex() != GUILDWAR_STEP_REWARD)
		return -2;
	if( !g_pGuildWarManager->GetGuildWarStats() )
		return -3;
	// Problem from here... send non-participants right away, and look up guild and own info for participants.

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check
	pUser->SendGuildWarTrialResultUIOpen();

	CDNGuildBase* pGuild = g_pGuildManager->At(pUser->GetGuildUID());
	if (pGuild)
	{
#if !defined( PRE_ADD_NODELETEGUILD )
		CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
		if (FALSE == pGuild->IsEnable()) return -1;
#endif

		if( pGuild->IsEnrollGuildWar() )
		{
			CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
			if (!pGuildVillage->GetGuildWarStats() )
			{
				// Get my guild summary info				
				CDNDBConnection* pDBCon = pGuild->GetDBCon();
				if (pDBCon)				
					pDBCon->QueryGetGuildWarPointGuildPartTotal(pGuild->GetDBThreadID(), pUser->GetWorldSetID(), pUser->GetAccountDBID(), 'G', pUser->GetGuildUID().nDBID);				
				return 0;
			}
			else if( !pUser->GetGuildWarStats() )
			{
				CDNDBConnection* pDBCon = pUser->GetDBConnection();
				if( pDBCon )				
					pDBCon->QueryGetGuildWarPointPartTotal(pUser->GetDBThreadID(), pUser->GetWorldSetID(), pUser->GetAccountDBID(), 'C', pUser->GetCharacterDBID());
				return 0;
			}			
		}
	}
	// If neither in a guild nor participating in guild war, send immediately.
	pUser->SendGuildWarTrialResultOpen();	
	return 0;
}
/*
desc : View guild war finals bracket
param  : user index
return : -1 => user not found, -2 => not in finals or reward period, -3 => bracket not set, 0: success
*/
int api_guildWar_GetMatchList(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if( g_pGuildWarManager->GetStepIndex() != GUILDWAR_STEP_REWARD)
		return -2;
	if( !g_pGuildWarManager->GetGuildWarStats() )
		return -3;

	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check
	pUser->SendGuildWarTournamentInfoUIOpen();

	bool bPopularityVote = false;
	CDNGuildBase* pGuild = g_pGuildManager->At(pUser->GetGuildUID());
	if (pGuild)
	{
#if !defined( PRE_ADD_NODELETEGUILD )
		CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
		if (FALSE == pGuild->IsEnable()) return -1;
#endif

		// If participating in Guild War, not in a finals team, and this character entered preliminaries.
		if( pGuild->IsEnrollGuildWar() && !g_pGuildWarManager->IsGuildWarFinal(pUser->GetGuildUID()) && pUser->GetGuildWarPoint() )
		{
			// If never voted and never fetched from DB
			if( !pUser->GetGuildWarVote() && !pUser->GetGuildWarVoteLoading() )
			{					
				CDNDBConnection* pDBCon = pUser->GetDBConnection();
				if (pDBCon)			
					pDBCon->QueryGetGuildWarPopularityVote(pUser->GetDBThreadID(), pUser->GetWorldSetID(), pUser->GetAccountDBID(), 'C', pUser->GetCharacterDBID());								
				return 0;
			}
			// If something was fetched from DB..
			else if( pUser->GetGuildWarVoteLoading() )
			{
				bPopularityVote = !pUser->GetGuildWarVote();
			}
		}
	}
	pUser->SendGuildWarTournamentInfoOpen(bPopularityVote);	
	return 0;
}
/*
desc : Guild war popularity vote results
param  : user index
return : -1 => user not found, -2 => no previous popularity vote results, -3 => guild war in progress, 0: success
*/
int api_guildWar_GetPopularityVote(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if( g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_REWARD)
		return -3;

	if( g_pGuildWarManager->GetGuildWarVote() && g_pGuildWarManager->GetGuildWarVoteCount() == 0)
		return -2;

	// If nothing has been fetched from DB
	if( !g_pGuildWarManager->GetGuildWarVote() )
	{	
		CDNDBConnection* pDBCon = pUser->GetDBConnection();
		if (pDBCon)			
			pDBCon->QueryGetGuildWarPopularityVote(pUser->GetDBThreadID(), pUser->GetWorldSetID(), pUser->GetAccountDBID(), 'G', 0);					
		return 0;		
	}
	pUser->SendGuildWarVoteTop(g_pGuildWarManager->GetGuildWarVoteCount(), g_pGuildWarManager->GetGuildWarVoteGuildName());
	return 0;	
}

/*
desc : Check whether guild war main tournament exists.
param  : user index
return : -1 : user not found / 0 : finals not in progress / 1 : finals in progress
*/
int api_guildWar_GetFinalProgress(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	if (g_pGuildWarManager)
		return (int)g_pGuildWarManager->GetFinalProgress();	
	return 0;
}

#if defined (PRE_ADD_DONATION)
int api_ui_OpenDonation(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);

	return DNScriptAPI::api_ui_OpenDonation(pUser);
}
#endif // #if defined (PRE_ADD_DONATION)

int api_ui_OpenInventory(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);

	return DNScriptAPI::api_ui_OpenInventory(pUser);
}

/*
desc : User's PC Cafe grade
param	:
return  : -1 => user not found, 0 => not a PC bang, otherwise PC bang grade.
*/
int api_user_GetPCCafe(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if( !pUser)
		return -1;	
	return pUser->GetPCBangGrade();
}

/*
desc : Create and give rental items such as weapons and mounts to PC cafe users.
param	:
return  : -1 => user not found, 0 => success, 1 => not a PC bang user, 2 => no item to grant, 3 => insufficient inventory space, 4 => already has rental item, 5 => item error.
*/
int api_user_SetPCCafeItem(UINT nUserObjectID)
{
#if defined( PRE_ADD_PCBANG_RENTAL_ITEM )
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if( !pUser )
		return -1;
	if( pUser->GetPCBangGrade() == 0)
		return 1;

	// Check here if already holding a rental item.
	if( pUser->GetItem()->IsHavePcCafeRentItem())
		return 4;

	std::map<int,int> mapItemIDList;
	int nRequireInven = g_pDataManager->GetPcCafeRentItemID(pUser->GetLevel(), pUser->GetClassID(), pUser->GetUserJob(), pUser->GetPCBangGrade(),mapItemIDList);
	if( mapItemIDList.empty())
		return 2;

	// Check inven..
	if( nRequireInven > pUser->GetItem()->FindBlankInventorySlotCount() ) 
	{	
		pUser->SendPcCafeRentItem(ERROR_ITEM_INVENTORY_NOTENOUGH, nRequireInven);
		return 3;
	}

	std::map<int,int>::iterator itor;
	for( itor=mapItemIDList.begin(); itor !=mapItemIDList.end(); ++itor )
	{
		if( !pUser->GetItem()->CreatePCRentalItem(itor->first, itor->second) )
			return 5;
	}
	pUser->SendPcCafeRentItem(ERROR_NONE, 0);	
	return 0;
#else
	return -1;
#endif		
}

/*
desc : Repair user's equipped items and deduct repair cost.
param  : user, repair all flag (1: repair all, 0: equipped items)
return : -1 : user not found / 0 : no items to repair / 1 : insufficient gold / 2 : item repair complete
*/
int api_user_RepairItem(UINT nUserObjectID, int iAllRepair)
{
#if defined(PRE_ADD_REPAIR_NPC)
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) return -1;

	return DNScriptAPI::api_user_RepairItem(pUser, iAllRepair);
#else
	return -1;
#endif
}


int api_ui_OpenExchangeEnchant(UINT nUserObjectID)
{
#ifdef PRE_ADD_EXCHANGE_ENCHANT
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

	return DNScriptAPI::api_ui_OpenExchangeEnchant(pUser);
#else
	return -1;
#endif
}

int api_env_CheckCloseGateByTime(int iMapIndex)
{
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	return DNScriptAPI::api_env_CheckCloseGateByTime(iMapIndex);
#else
	return 0;
#endif
}

int api_ui_OpenCostumeRandomMix(UINT nUserObjectID)
{
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;

#ifndef _FINAL_BUILD
	std::wstring wszLog;
	wszLog = FormatW( L"Open NPC random compound UI");
	pUser->SendChat(CHATTYPE_NORMAL, (int)wszLog.size()*sizeof(WCHAR), L"", (WCHAR*)wszLog.c_str());
#endif

	pUser->StartCostumeRandomMix(CostumeMix::RandomMix::OpenByNpc, 0, 0);
	pUser->SetCalledNpcResponse(true, false);	// Since a UI open request sent to the client usually does not continue into dialogue, mark it as talked so it does not unintentionally fail the NPC response check

	return 1;
#else
	return -1;
#endif
}

int api_quest_WorldBuffCheck(int nItemID)
{
#if defined(PRE_DRAGONBUFF)
	return DNScriptAPI::api_quest_WorldBuffCheck(nItemID);
#else
	return -1;
#endif
}

int api_quest_ApplyWorldBuff(UINT nUserObjectID, int nItemID, int nMapIdx)
{
#if defined(PRE_DRAGONBUFF)
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if ( !pUser ) 
		return -1;
	return DNScriptAPI::api_quest_ApplyWorldBuff(pUser, nItemID, nMapIdx);
#else
	return -1;
#endif
}

int api_quest_ForceSkipQuest(UINT nUserObjectID, const char* szQuestList, int nSkipFlag)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser)
		return -1;
	return DNScriptAPI::api_quest_ForceSkipQuest(pUser, szQuestList, nSkipFlag);
}

int api_user_IsDNGirl(UINT nUserObjectID)
{
	CDNUserSession* pUser = __GetUserObj(nUserObjectID);
	if (!pUser)
		return -1;
	return DNScriptAPI::api_user_IsDNGirl(pUser);
}
