#pragma once

#include "DnLuaAPIDefine.h"
// GameServer Script API
//  [4/1/2008 nextome]

class CEtTriggerElement;
class CDNGameRoom;
class CDNUserBase;

int CheckInvenForQuestReward(CDNUserBase* pUser, const TQuestReward& QuestReward, const bool bSelectedArray[]);

// Register all functions to Lua.
void DefAllAPIFunc(lua_State* pLuaState);

/*
desc : Link to the next dialogue.
param  : user index, NPC index, talk index, talk file name
return : void
*/
void api_npc_NextTalk(CDNGameRoom *pRoom, UINT nUserSessionID, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);

/*
desc : Link to the next script.
param  : user index, NPC index, talk index, talk file name
return : void
*/
void api_npc_NextScript(CDNGameRoom *pRoom, UINT nUserSessionID, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);


/*
desc : Set dialogue parameter (string).
param  : parameter key, parameter string index
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
sample : 
When building a sentence like 'I like the color {%color}.'
Make it like: api_npc_SetParamString("{%color}, math.random(2) );
1 must be recorded separately in TalkParamTable.xls as 1: Red, 2: Blue.

Note: Parameters set with api_npc_SetParamString are cleared after calling api_npc_NextTalk, so set them before calling api_npc_NextTalk.
*/
int api_npc_SetParamString(CDNGameRoom *pRoom, UINT nUserObjectID, const char* szParamKey, int nParamID);

/*
desc : Set dialogue parameter (integer).
param  : parameter key, integer value
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
sample : 
When building a sentence like 'I want to die around age {%age}.'
Make it like: api_npc_SetParamInt("{%age}, math.random(30)+40 );

Note: Parameters set with api_npc_SetParamInt are cleared after calling api_npc_NextTalk, so set them before calling api_npc_NextTalk.
*/
int api_npc_SetParamInt(CDNGameRoom *pRoom, UINT nUserObjectID, const char* szParamKey, int nValue);


/*
desc : Write a log.
param  : Include \n for line breaks in log string.
return : void
*/
void api_log_AddLog(CDNGameRoom *pRoom, const char* szLog);

/*
desc : Log a message to the user. Sent as a user chat message.
param  : Include \n for line breaks in log string.
return : void
*/
void api_log_UserLog(CDNGameRoom *pRoom, UINT nUserObjectID, const char* szLog);


/*
desc : Send quest-related information to the user via chat.
param  : 
return : void
*/
void api_quest_DumpQuest(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Assign a quest to the user.
param  : user index, quest index, quest type (QuestType_SubQuest = 1, QuestType_MainQuest = 2)
return : 1 : success, -1 : user not found, -2 : user does not have quest
*/
int api_quest_AddHuntingQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt);
int api_quest_AddQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, int nQuestType);

/*
desc : Complete a quest for the user.
param  : user index, quest index, whether to remove from current quests (if not removed, only increments play count)
return : -1 => user not found, -2 => quest is not in progress
*/
int api_quest_CompleteQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, bool bDelPlayList);

/*
desc : Add quest to user's completion list. (Recorded in DB!!)
param  : user index, quest index
return : -1 => user not found, -2 => quest is not in progress
*/
int api_quest_MarkingCompleteQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);


/*
desc : Check if quest is marked completed in completion list.
param  : user index, quest index
return : -1 => user not found, 1 => quest marked complete, 0 => not marked complete
*/
int api_quest_IsMarkingCompleteQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Check if the user has the quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => has quest (returns quest step)
*/
int api_quest_UserHasQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Check how many quests the user currently has in progress.
param  : user index, quest index
return : -1 => user not found, otherwise returns count of quests currently in progress
*/
int api_quest_GetPlayingQuestCnt(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Return the NPC index.
param  : user index, NpcObjId (npc index and npcobject index are different)
return : -1 => user not found, -2 => npc not found, otherwise npc index
*/
int api_npc_GetNpcIndex(CDNGameRoom *pRoom, UINT nUserObjectID, UINT nNpcObjID);

/*
desc : Set quest step and journal step of user's quest.
param  : user index, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
*/
int api_quest_SetQuestStepAndJournalStep(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, short nQuestStep, int nJournalStep);

/*
desc : Set quest step of user's quest.
param  : user index, quest index, quest step
return : -1 => user not found, -2 => user does not have quest, returns 1 on success
*/
int api_quest_SetQuestStep(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, short nQuestStep);

/*
desc : Get quest step of user's quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current quest step
*/
int api_quest_GetQuestStep(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Set journal step of user's quest.
param  : user index, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
*/
int api_quest_SetJournalStep(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, int nJournalStep);

/*
desc : Get journal step of user's quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current journal step
*/
int api_quest_GetJournalStep(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Set value at a specific index of user's quest memo.
param  : user index, memo index, memo value
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, 1 => success
*/
int api_quest_SetQuestMemo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, char nMemoIndex, int iVal);

/*
desc : Get value at a specific index of user's quest memo.
param  : user index, memo index
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, > -1 => value of specific memo index
*/
int api_quest_GetQuestMemo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Function to set counting info for killing monsters or acquiring items.
param  : user index,
Quest index,
Counting info setting slot (5 setting slots from 0 to 4),
Type to count (CountingType_MyMonsterKill = 1, CountingType_AllMonsterKill = 2, CountingType_Item = 3, CountingType_SymbolItem = 4),
Index of item to count,
Count amount
return : -1 => user not found, -2 => user does not have quest, -3 => counting info setting slot out of range, -4 => counting type or target count out of range

example: 

api_quest_SetCountingInfo(userID, questID, 0, CountingType_MyMonsterKill, 10001, 10);
api_quest_SetCountingInfo(userID, questID, 1, CountingType_MyMonsterKill, 10002,  5);
api_quest_SetCountingInfo(userID, questID, 2, CountingType_MyMonsterKill, 10003, 20);
api_quest_SetCountingInfo(userID, questID, 3, Item,    1001,  5);
api_quest_SetCountingInfo(userID, questID, 4, Item,     1002,  1);

When called from script as above, 5 counting conditions are set.
The top line sets slot 0's condition to be satisfied upon killing 10 of monster 10001.

As above, the 5 slots are set to 10 of monster 10001, 5 of monster 10002, 20 of monster 10003, 5 of item 1001, and 1 of item 1002
When the above 5 conditions are met, OnComplete() of the quest script is called.

*** Max counting quantity is 30000.
*/
int api_quest_SetCountingInfo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID,  int nSlot, int nCountingType, int nCountingIndex, int nTargetCnt); 


/*
desc : Clear the configured counting info.
param  : user index, quest index
return : -1 => user not found, -2 => quest not found
*/
int api_quest_ClearCountingInfo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Check if all configured countings are complete.
param  : user index, item index, item count
return : -1 => user not found, -2 => quest not found, -3 => not completed, 1 => all completed
*/
int api_quest_IsAllCompleteCounting(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID);

/*
desc : Check if the user has enough inventory space for the item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_user_CheckInvenForAddItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has enough inventory space for the quest item list.
param  : user index, quest item list (Lua table, [<max count>]{quest item index, quest item count} pairs up to <max count (currently 10)>)
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space

example :

function grandmother_OnTalk(userObjID, npcObjID, npc_talk_index, npc_talk_target, questID)

	local TableItem =
	{
{ 1001, 10 }, -- item index (1001), item count (10)
{ 1002, 1 }, -- item index (1002), item count (1)
{ 1003, 5 }, -- Item index (1003), item count (5)
	};

	if (api_user_CheckInvenForAddItemList(userObjID, TableParam) == 1) then
-- Success
	else
-- Failure
	end
end
*/
int api_user_CheckInvenForAddItemList(CDNGameRoom *pRoom, UINT nUserObjectID, lua_tinker::table ItemTable);

/*
desc : Grant an item to the user.
param  : user index, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_user_AddItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID);

/*
desc : Remove the item from the user.
param  : user index, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient item count
*/
int api_user_DelItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID);

/*
desc : Remove all of the item from the user.
param  : user index, item index
return : -1 => user not found, -2 => item not found
*/
int api_user_AllDelItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex);

/*
desc : Check if the user has the item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);


/*
desc : Get the user's class ID.
param  : user index
return : -1 => user not found, otherwise ClasssID
*/
int api_user_GetUserClassID(CDNGameRoom *pRoom, UINT nUserObjectID);


/*
desc : Get the user's Job ID.
param  : user index
return : -1 => user not found, otherwise Job ID
*/
int api_user_GetUserJobID(CDNGameRoom *pRoom, UINT nUserObjectID);


/*
desc : Get the user's level.
param  : user index
return : -1 => user not found, otherwise Level
*/
int api_user_GetUserLevel(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Get number of empty slots in user's inventory.
param  : user index
return : -1 => user not found, otherwise empty slot count
*/
int api_user_GetUserInvenBlankCount(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Get number of empty slots in user's quest inventory.
param  : user index
return : -1 => user not found, otherwise empty slot count
*/
int api_quest_GetUserQuestInvenBlankCount(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Add EXP to the user.
param  : user index, EXP
return : -1 => user not found, 1 => success
*/
int api_user_AddExp(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, int nAddExp);

/*
desc : Add money to the user.
param  : user index, money
return : -1 => user not found, 1 => success
*/
int api_user_AddCoin(CDNGameRoom *pRoom, UINT nUserObjectID, int nAddCoin, int nQuestID);


/*
desc : Play a video for the user.
param  : user index, cutscene table ID
return : -1 => user not found, 1 => success
*/
int api_user_PlayCutScene(CDNGameRoom *pRoom, UINT nUserObjectID, UINT nNpcObjectID, int nCutSceneTableID, bool bIgnoreFadeIn);


/*
desc : Notify the user to open the storage.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenWareHouse(CDNGameRoom *pRoom, UINT nUserObjectID, int iItemID=0 );

/*
desc : Notify the user to open the shop.
param  : user index, shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenShop(CDNGameRoom *pRoom, UINT nUserObjectID, int nShopID, Shop::Type::eCode Type = Shop::Type::Normal);

/*
desc : Notify the user to open the skill shop.
param  : user index, skill shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenSkillShop(CDNGameRoom *pRoom, UINT nUserObjectID, int nSkillShopID);


/*
desc : Notify the user to open the unmanned shop.
param  : user index
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenMarket(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Instruct the user to open the crest jewel synthesis window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCompoundEmblem(CDNGameRoom *pRoom, UINT nUserObjectID);


/*
desc : Notify the user to open the jewel upgrade window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenUpgradeJewel(CDNGameRoom *pRoom, UINT nUserObjectID);


/*
desc : Instruct the user to open the mailbox.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenMailBox(CDNGameRoom *pRoom, UINT nUserObjectID);


/*
desc : Instruct the user to open the disassembly window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenDisjointItem(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Instruct the user to open the item synthesis window.
param  : user index, item synthesis shop ID
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCompoundItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nCompoundShopID);

int api_ui_OpenCompound2Item(CDNGameRoom *pRoom, UINT nUserObjectID, int nCompoundShopID, int iItemID/*=0*/ );

/*
desc : Instruct the user to open the cash item shop.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCashShop(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Instruct the user to open the guild management UI dialog.
param  : user index, guild management ID
return : -1 => user not found, 1 => success

Guild management ID>
0 : Create guild
1 : Disband guild
2 : Leave guild
3: Guild points
4: Guild level up
*/
int api_ui_OpenGuildMgrBox(CDNGameRoom *pRoom, UINT nUserObjectID, int nGuildMgrNo);

/*
desc : Open the Japan gachapon dialog.
param  : gachapon index to be used
return : -1 => user not found, -2 => no shop, 1 => success, -3 => filtered by define
*/
int api_ui_OpenGacha_JP(CDNGameRoom *pRoom,  UINT nUserObjectID, int nGachaShopID);

/*
desc : Open the NPC gift dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenGiveNpcPresent(CDNGameRoom* pRoom, UINT nUserObjectID, int nNpcID);


/*
desc : Notify the user to open the farm storage.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenFarmWareHouse(CDNGameRoom *pRoom, UINT nUserObjectID);


#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
/*
desc : Open the texture dialog.
param  : user index, file index, screen position, display duration, image center point,
return : -1 => user not found, 1 => success
*/
int api_ui_OpenTextureDialog(CDNGameRoom* pRoom, UINT nUserObjectID, int nFileIndex, float fX, float fY, int nPos, int nTime, int nDialogIndex, bool bAutoCloseDialog);


/*
desc : Close the texture dialog.
param  : user index, dialog ID
return : -1 => user not found, 1 => success
*/
int api_ui_CloseTextureDialog(CDNGameRoom* pRoom, UINT nUserObjectID, int nDlgID);
#endif


/*
desc : Display a message to the user.
param  : nType is message output type,
CHATTYPE_NORMAL   => 1, // Normal chat
CHATTYPE_PARTY    => 2, // Party chat
CHATTYPE_PRIVATE  => 3, // Whisper
CHATTYPE_GUILD    => 4, // Guild chat
CHATTYPE_CHANNEL  => 5, // Trade/channel chat
CHATTYPE_SYSTEM   => 6, // System message

Include \n for line breaks in log string.
return : -1 => user not found, -2 invalid type, -3 message too long
*/
int api_user_UserMessage(CDNGameRoom *pRoom, UINT nUserObjectID, int nType, int nBaseStringIdx, lua_tinker::table ParamTable);

/*
desc : Add symbol item to the user.
param  : user index, symbol item ID, count
return : -1 => user not found, -2 => fail, 1 => success
*/
int api_quest_AddSymbolItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID, short wCount);

/*
desc : Delete symbol item from the user.
param  : user index, symbol item ID, count
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_DelSymbolItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID, short wCount);

/*
desc : Check if the user has the symbol item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_quest_HasSymbolItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has enough inventory space for the quest item.
param  : user index, quest item index, quest item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_quest_CheckQuestInvenForAddItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has enough inventory space for the quest item list.
param  : user index, quest item list (Lua table, [<max count>]{quest item index, quest item count} pairs up to <max count (currently 10)>)
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space

example :

function grandmother_OnTalk(userObjID, npcObjID, npc_talk_index, npc_talk_target, questID)

	local TableItem =
	{
{ 1001, 10 }, -- item index (1001), item count (10)
{ 1002, 1 }, -- item index (1002), item count (1)
{ 1003, 5 }, -- Item index (1003), item count (5)
	};

	if (api_quest_CheckQuestInvenForAddItemList(userObjID, TableParam) == 1) then
-- Success
	else
-- Failure
	end
end
*/
int api_quest_CheckQuestInvenForAddItemList(CDNGameRoom *pRoom, UINT nUserObjectID, lua_tinker::table ItemTable);

/*
desc : Add quest item to the user.
param  : user index, quest item ID, count, quest ID
return : -1 => user not found, -2 => fail, 1 => success
*/
int api_quest_AddQuestItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID, short wCount, int nQuestID);

/*
desc : Delete quest item from the user.
param  : user index, quest item ID, count, quest ID
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_DelQuestItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID, short wCount, int nQuestID);

/*
desc : Delete all quest items from the user.
param  : user index, quest item ID
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_AllDelQuestItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID );

/*
desc : Check if the user has the quest item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_quest_HasQuestItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Notify the user to open the guild creation window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenGuildCreate(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Notify the user to open the quest reward window.
param  : user index, reward table, bActivate (true if selectable, false for view-only)
return : -1 => user not found, 1 => success
*/
int api_ui_OpenQuestReward(CDNGameRoom *pRoom, UINT nUserObjectID, int nRewardTableIndex, bool bActivate);

/*
desc : Give quest reward to the user.
param  : user index, reward table, quest ID, reward check number
return : -1 => user not found, -2 user item object error, -3 insufficient inventory, -4 table not found, -5 reward condition check error, 1 => success
*/
int api_quest_RewardQuestUser(CDNGameRoom *pRoom, UINT nUserObjectID, int nRewardTableIndext, int nQuestID, int nRewardCheck);

/*
desc   : Return the number of party members for the user.
param  : user index
return : -1 => user not found, otherwise party member count (Note: returns 1 even if user is not in a party)
*/
int api_user_GetPartymemberCount(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc   : Check if the user is in a party.
param  : user index
return : -1 => user not found, 0 => not in party, 1 => in party
*/
int api_user_IsPartymember(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Move the user.
param  : user index, target map index, target gate index
return : -1 => user not found, -2 => map does not exist
*/
void api_user_ChangeMap(CDNGameRoom *pRoom, UINT nUserObjectID, int nMapIndex, int nGateNo);

/*
desc   : Get the user's stage difficulty
param  : user index
return : 0 ~ 4 => dungeon difficulty (0:Easy / 1:Normal / 2:Hard / 3:Master / 4:Abyss), otherwise fail (default -1)
*/
int api_user_GetStageConstructionLevel(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc   : Get the user's current map ID
param  : user index
return : map number (>= 1), otherwise fail (<= 0)
*/
int api_user_GetMapIndex(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc   : Get the user's latest stage clear rank
param  : user index
return : stage clear rank (SSS : 0 / SS : 1 / S : 2 / A : 3 / B : 4 / C : 5 / D : 6), otherwise fail (default -1)
*/
int api_user_GetLastStageClearRank(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Check the user's current money.
param  : user index, value to check
return : -1 => user not found, -2 => insufficient gold, 1 => sufficient gold
*/
int api_user_EnoughCoin(CDNGameRoom *pRoom, UINT nUserObjectID, int nCoin );

/*
desc : Check the user's current money.
param  : user index
return : -1 => user not found, >= 0 => current money
*/
int api_user_GetCoin(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Deduct money from the user.
param  : user index, money (> 0)
return : -1 => user not found, 1 => success
*/
int api_user_DelCoin(CDNGameRoom *pRoom, UINT nUserObjectID, int nDelCoin);

/*
desc : Return whether the user acquired a specific mission
param  : user index, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
*/
int api_user_IsMissionGained(CDNGameRoom *pRoom, UINT nUserObjectID, int nMissionIndex);

/*
desc : Return whether the user completed a specific mission
param  : user index, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
*/
int api_user_IsMissionAchieved(CDNGameRoom *pRoom, UINT nUserObjectID, int nMissionIndex);

/*
desc : Check if the user has the item in storage.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasItemWarehouse(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has the item equipped.
param  : user index, item index
return : -1 => user not found, -2 => item not found, 1 or other => true
*/
int api_user_HasItemEquip(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex);

/*
desc : Return the user's current guild rank
param  : user index
return : 0 : no guild / 1 : guild master / 2 : vice guild master / 3 : senior guild member / 4 : regular guild member / 5 : junior guild member
*/
int api_guild_GetGuildMemberRole(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Increment value at a specific index of user's quest memo by 1
param  : user index, quest number, memo index
return : incremented quest memo value (returns LONG_MIN on error)
*/
int api_quest_IncQuestMemo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Decrement value at a specific index of user's quest memo by 1
param  : user index, quest number, memo index
return : incremented quest memo value (returns LONG_MAX on error)
*/
int api_quest_DecQuestMemo(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Assign a specific job advancement class to the user
param  : user index, target job class
return : -1 : user not found / -2 : invalid job class ID / -3 : job change condition not met / 0 : already the same job class / 1 : job class changed successfully
*/
int api_user_SetUserJobID(CDNGameRoom *pRoom, UINT nUserObjectID, int nJobID);

/*
desc : Check if the user's job belongs to a specific job branch
param  : user index, prerequisite job class
return : -1 : user not found / 0 : not same class line / 1 : same class line
*/
int api_user_IsJobInSameLine(CDNGameRoom *pRoom, UINT nUserObjectID, int nBaseJobID);

/*
desc : Check if the user has the cash item.
param  : user index, cash item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasCashItem(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has the cash item equipped.
param  : user index, cash item index
return : -1 => user not found, -2 => item not found, 1 or other => true
*/
int api_user_HasCashItemEquip(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemIndex);

/*
desc   : Arbitrarily increment the count for a specific counting type and ID.
param  : user index,
Quest index,
Type to count (CountingType_MyMonsterKill = 1, CountingType_AllMonsterKill = 2, CountingType_Item = 3),
Index of item to count (Item ID, Monster ID, etc.)
return : -1 => user not found, 1 => success
*/
int api_quest_IncCounting(CDNGameRoom *pRoom, UINT nUserObjectID, int nCountingType, int nCountingIndex);

#if defined(PRE_ADD_DAILYQUESTDATEAPI_01) && defined(PRE_ADD_QUEST_DAILYCHECK_01)
/*
desc : Get the current date based on daily quest reset.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_quest_GetDailyQuestDay(CDNGameRoom *pRoom, UINT nUserObjectID);
#endif	// #if defined(PRE_ADD_DAILYQUESTDATEAPI_01) && defined(PRE_ADD_QUEST_DAILYCHECK_01)

/*
desc : Return whether the user has reached the maximum number of active quests.
param  : user index, quest index
return : -1 : user not found, 1 : max quest count reached, otherwise : no
*/
int api_quest_IsPlayingQuestMaximum(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc   : Force-complete a specific quest. (If not in progress, add and complete it)
param  : user index, quest index, quest code (0: normal / 1: timed), mark flag (1: mark / 0: do not mark), delete flag (1: delete / 0: do not delete), repeat flag (1: repeat / 0: do not repeat)
return : -1 : user not found, 1 : max quest count reached, otherwise : no
*/
int api_quest_ForceCompleteQuest(CDNGameRoom *pRoom, UINT nUserObjectID, int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat);

/*
desc   : Move a specific user's position.
param  : user index, X coord, Y coord, Z coord
return : -1 : user not found, 1 : success
*/
int api_user_ChangePos(CDNGameRoom *pRoom, UINT nUserObjectID, int nX, int nY, int nZ, int nLookX = 0, int nLookY = 0);

int api_ui_OpenUpgradeItem(CDNGameRoom *pRoom, UINT nUserObjectID,int iItemID=0);

int api_user_RequestEnterPVP(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Get NPC favor points.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetFavorPoint( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC malice points.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetMalicePoint( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC favor percentage.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetFavorPercent( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC malice percentage.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetMalicePercent( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Add or subtract NPC favor points.
param  : user index, NpcID, delta value (negative value decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_AddFavorPoint( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID, int val );

/*
desc : Add or subtract NPC malice points.
param  : user index, NpcID, delta value (negative value decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_AddMalicePoint( CDNGameRoom *pRoom, UINT uiUserObjectID, int iNpcID, int val );

/*
desc : Request client to send selected NPC gift info to server.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_SendSelectedPresent( CDNGameRoom* pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Request client to send selected NPC gift info to server.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_Rage( CDNGameRoom* pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc : Request client to send selected NPC gift info to server.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_Disappoint( CDNGameRoom* pRoom, UINT uiUserObjectID, int iNpcID );

/*
desc   : Reset all skills of a specific user.
param  : user index
return : -1 : user not found, 1 : success
*/
int api_user_ResetSkill(CDNGameRoom *pRoom, UINT nUserObjectID);

/*
desc : Temporarily allow a specific user to use 2nd job advancement skills.
param  : user index
return : -1 : user not found, 1 : success
*/
int api_user_SetSecondJobSkill( CDNGameRoom *pRoom, UINT nUserObjectID, int iSecondChangeJobID );

/*
desc : Restore a user temporarily enabled for 2nd job advancement back to original state.
Resets on map change even without calling this api.
param  : user index
return : -1 : user not found, 1 : success
*/
int api_user_ClearSecondJobSkill( CDNGameRoom *pRoom, UINT nUserObjectID );

/*
desc : Repair user's equipped items and deduct repair cost.
param  : user index, repair all flag (1: repair all, 0: equipped items)
return : -1 : user not found / 0 : no items to repair / 1 : insufficient gold / 2 : item repair complete
*/
int api_user_RepairItem(CDNGameRoom *pRoom, UINT nUserObjectID, int iAllRepair);


/*
desc : Transfer an item's enhancement level to another item.
param  : user index
return : -1 : user not found, 1 : success
*/
int api_ui_OpenExchangeEnchant(CDNGameRoom *pRoom, UINT nUserObjectID);

//------------------------------------------------------------------------------------------------
// trigger

int api_trigger_GetPartyCount( CDNGameRoom *pRoom );
int api_trigger_GetPartyActor( CDNGameRoom *pRoom, int nPartyIndex );
bool api_trigger_IsInsidePlayerToEventArea( CDNGameRoom *pRoom, int nActorHandle, int nEventAreaHandle );
void api_trigger_SetActionToProp( CDNGameRoom *pRoom, int nPropHandle, const char *szAction );
void api_trigger_ShowDungeonClear( CDNGameRoom *pRoom, bool bClear = true, bool bIgnoreRewardItem = false );
void api_trigger_SetPermitGate( CDNGameRoom *pRoom, int nGateIndex, int nOpenClose );
int api_trigger_GetMonsterAreaLiveCount( CDNGameRoom *pRoom, int nEventAreaHandle );
int api_trigger_GetMonsterAreaTotalCount( CDNGameRoom *pRoom, int nEventAreaHandle );
void api_trigger_RebirthMonster( CDNGameRoom *pRoom, int nEventAreaHandle );
void api_trigger_GenerationMonster( CDNGameRoom *pRoom, int nEventAreaHandle, int nMonsterTableID, int nMin, int nMax, int nTeamSetting );
void api_trigger_GenerationMonsterGroup( CDNGameRoom *pRoom, int nEventAreaHandle, int nMonsterGroupID, int nMin, int nMax, int nTeamSetting );
void api_trigger_CmdMoveMonster( CDNGameRoom* pRoom, int nEventAreaHandle, int nTargetEventAreaHandle );
void api_trigger_CmdRandMoveMonster( CDNGameRoom* pRoom, int nEventAreaHandle, int nTargetEventAreaHandle, UINT uiMoveFrontRate );
bool api_trigger_CheckMonsterIsInsideArea( CDNGameRoom* pRoom, int nEventAreaHandle, int nTargetEventAreaHandle );
bool api_trigger_CheckMonsterLessHP( CDNGameRoom* pRoom, int iMonsterID, int iHP );
bool api_trigger_CheckInsideAreaMonsterLessHP( CDNGameRoom* pRoom, int iAreaUniqueID, int iHP );
void api_trigger_GenerationMonsterSet( CDNGameRoom *pRoom, int nEventAreaHandle, int nSetID, int nPosID, int nTeamSetting, bool bResetReference );
void api_trigger_GenerationMonsterSetInEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nChangeSetID, int nChangePosID, int nTeamSetting, bool bResetReference );
/*
desc : Spawn an NPC in EventArea.
param  : event area ID, npc ID
return : -1 => event area not found, 1 => success
*/
int api_trigger_GenerationNpc(CDNGameRoom *pRoom, int nEventAreaHandle, int nNpcTableID);

/*
desc : Remove an NPC from EventArea.
param  : event area ID
return : -1 => event area not found, 1 => success
*/
int api_trigger_DestroyNpc(CDNGameRoom *pRoom, int nEventAreaHandle);

void api_trigger_SetGameSpeed( CDNGameRoom *pRoom, float fPlaySpeed, DWORD dwDelay );
void api_trigger_ShowProp( CDNGameRoom *pRoom, int nPropHandle, bool bShow );
bool api_trigger_IsAllDieMonster( CDNGameRoom *pRoom, bool bCheckTeam = false );
int api_trigger_GetLastAimPropHandle( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
bool api_trigger_IsProp( CDNGameRoom *pRoom, int nPropHandle );
bool api_trigger_IsBrokenProp( CDNGameRoom *pRoom, int nPropHandle );
void api_trigger_DestroyMonsterArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nDropType );
void api_trigger_DestroyAllMonster( CDNGameRoom *pRoom, bool bDropItem, int nTeam );
void api_trigger_SetMonsterAIState( CDNGameRoom *pRoom, int nEventAreaHandle, int nValue );
void api_trigger_PlayCutSceneByTrigger( CDNGameRoom *pRoom, int nCutSceneTableID, bool bFadeIn, int nQuestIndex , int nQuestStep);
int api_trigger_GetGameTime( CDNGameRoom *pRoom );
bool api_trigger_Delay( CDNGameRoom *pRoom, CEtTriggerElement *pElement, int nDelay );
int api_trigger_GetBattleModeCount( CDNGameRoom *pRoom, int nEventAreaHandle );
char *api_trigger_GetActionToProp( CDNGameRoom *pRoom, int nPropHandle );
void api_trigger_ChangeMapByTrigger( CDNGameRoom *pRoom, int nMapIndex, int nGateNo );
int api_trigger_GetTalkNpcClassID( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetTalkNpcActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetTalkNpcTargetActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
void api_trigger_EnableOperatorProp( CDNGameRoom *pRoom, int nPropHandle, int nActorHandle, bool bEnable );
void api_trigger_AllUserEnableOperatorProp( CDNGameRoom *pRoom, int nPropHandle, bool bEnable );
void api_trigger_SetActionToPropActor( CDNGameRoom *pRoom, int nPropHandle, int nActorHandle, const char *szAction );
int api_trigger_GetLastAimPropActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetLastBrokenPropActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
void api_trigger_EnableTriggerElement( CDNGameRoom *pRoom, CEtTriggerElement *pElement, bool bEnable );
void api_trigger_SetActionToActor( CDNGameRoom *pRoom, int nEventAreaHandle, const char *szAction, int nLoopCount, float fBlendFrame );
void api_trigger_EnableOperatorNpc( CDNGameRoom *pRoom, int nEventAreaHandle, int nActorHandle, bool bEnable );
void api_trigger_EnableOperatorNpcFromPropNpc( CDNGameRoom *pRoom, int nPropHandle, int nActorHandle, bool bEnable );
void api_trigger_EnableTriggerObject( CDNGameRoom *pRoom, const char *szTriggerObjectName, bool bEnable );
void api_trigger_EnableMonsterNoDamage( CDNGameRoom *pRoom, int nEventAreaHandle, bool bEnable );
void api_trigger_WarpActor( CDNGameRoom *pRoom, int nActorHandle, int nTargetEventAreaHandle );
void api_trigger_WarpActorFromEventArea( CDNGameRoom *pRoom, int nMonsterEventAreaHandle, int nTargetEventAreaHandle );
bool api_trigger_IsInsidePropToEventArea( CDNGameRoom* pRoom, int nPropHandle, int nTargetEventAreaHandle );
void api_trigger_ChangeTeamFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nTeam );
bool api_trigger_IsDieActor( CDNGameRoom *pRoom, int nActorHandle );
int api_trigger_GetUserLevelByTrigger( CDNGameRoom *pRoom, int nActorHandle );
int api_trigger_GetMonsterActorFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nIndex );
void api_trigger_DropItemToEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nItemID, int nCount, DWORD dwOwnerActorHandle = 0 );
void api_trigger_DropItemToActor( CDNGameRoom *pRoom, int nActorHandle, int nItemID, int nCount, DWORD dwOwnerActorHandle = 0 );
void api_trigger_ShowChatBalloonToMonsterArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nUIStringIndex );
void api_trigger_ShowChatBalloonToActor( CDNGameRoom *pRoom, int nActorHandle, int nUIStringIndex );
void api_trigger_ShowChatBalloonToProp( CDNGameRoom *pRoom, int nPropHandle, int nUIStringIndex );
void api_trigger_HideChatBalloonToMonsterArea( CDNGameRoom *pRoom, int nEventAreaHandle );
void api_trigger_HideChatBalloonToProp( CDNGameRoom *pRoom, int nPropHandle );
void api_trigger_SetForceAggroToMonsterArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nTargetEventArea, int nValue );
void api_trigger_SetForceAggroToProp( CDNGameRoom *pRoom, int nEventAreaHandle, int nPropHandle, int nValue );
void api_trigger_ShowExposureInfo( CDNGameRoom *pRoom, int nEventAreaHandle, bool bShow );
void api_trigger_DLChangeRound( CDNGameRoom *pRoom, bool bBoss, int iNextTotalRound=0 );
int api_trigger_DLGetRound( CDNGameRoom *pRoom );
int api_trigger_DLGetTotalRound( CDNGameRoom *pRoom );
int api_trigger_DLGetStartFloor( CDNGameRoom* pRoom );
int api_trigger_DLRequestChallenge( CDNGameRoom* pRoom );
int api_trigger_DLResponseChallenge( CDNGameRoom* pRoom );
void api_trigger_DLSetStartRound( CDNGameRoom* pRoom, int iRound );
void api_trigger_EnableDungeonClearWarpQuestFlag(CDNGameRoom* pRoom);
bool api_trigger_IsEnableDungeonClearWarp(CDNGameRoom* pRoom);
bool api_trigger_IsExistNotAchieveMissionPartyActor( CDNGameRoom *pRoom, int nMissionArrayIndex );
int api_trigger_GetChangeActionPlayerActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
char *api_trigger_GetActionToPlayerActor( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
void api_trigger_EnablePropNoDamage( CDNGameRoom *pRoom, int nPropHandle, bool bEnable );
int api_trigger_GetTriggerEventSignalEventAreaHandle( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetTriggerEventSignalActorHandle( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetTriggerEventSignalEventID( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
int api_trigger_GetTriggerEventSignalTeam( CDNGameRoom *pRoom, CEtTriggerElement *pElement );
void api_trigger_HoldPartyDice(CDNGameRoom* pRoom);
void api_trigger_ReleasePartyDice(CDNGameRoom* pRoom);
void api_trigger_UpdateNestTryCount(CDNGameRoom *pRoom,int iForceMapIndex=0 );
void api_trigger_UpdateUserNestTryCount(CDNGameRoom *pRoom, int nActorHandle );
int api_trigger_GetMonsterClassID( CDNGameRoom *pRoom, int nActorHandle );
void api_trigger_EnableDungeonClearWarpAlarm( CDNGameRoom *pRoom, bool bEnable );
void api_trigger_SetHPFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nValue );
void api_trigger_SetHP( CDNGameRoom *pRoom, int nActorHandle, int nValue );
void api_trigger_AddHPAbsoluteFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nValue );
void api_trigger_AddHPRatioFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, float fValue );
void api_trigger_AddHPAbsolute( CDNGameRoom *pRoom, int nActorHandle, int nValue );
void api_trigger_AddHPRatio( CDNGameRoom *pRoom, int nActorHandle, float fValue );
int api_trigger_GetTeamFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle );
int api_trigger_GetTeam( CDNGameRoom *pRoom, int nActorHandle );
void api_trigger_SetHPRatioFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, float fValue );
void api_trigger_SetHPRatio( CDNGameRoom *pRoom, int nActorHandle, float fValue );
void api_trigger_IgnoreAggroFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nValue );
int api_trigger_GetPropDurability( CDNGameRoom *pRoom, int nPropHandle );
int api_trigger_LastAimPropActorMessage( CDNGameRoom *pRoom, int nType, int nActorHandle, int nBaseStringIdx, lua_tinker::table ParamTable );
void api_trigger_BuffFromEventArea( CDNGameRoom *pRoom, int nActorHandle, int nSkillID, int nSkillLevel, int nDelay);
bool api_trigger_NoDuplicateBuffFromEventArea( CDNGameRoom *pRoom, int nActorHandle, int nSkillID, int nSkillLevel, int nDelay);
void api_trigger_Start_DragonNest( CDNGameRoom* pRoom, int nType, int nRebirthCount );
void api_trigger_SetUsableRebirthCoin( CDNGameRoom* pRoom, int nActorHandle, int nUsableRebirthCoin );
int api_trigger_GetMonsterHPPercent_FromEventArea( CDNGameRoom* pRoom, int nEventAreaHandle );
void api_trigger_UseSkillFromEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nSkillID );
void api_trigger_UseSkill( CDNGameRoom *pRoom, int nActorHandle, int nSkillID );
void api_trigger_NotUseManaSkill( CDNGameRoom *pRoom, int nActorHandle );
void api_trigger_AllUser_AddVehicleItem(CDNGameRoom *pRoom, char* pszStr, bool bDBSave );
void api_trigger_PlayerNameShowBlindCaption( CDNGameRoom *pRoom, int nUIStringIndex, int nDelay, int nActorHandle );
void api_trigger_InvenToEquip( CDNGameRoom* pRoom, UINT nUserObjectID, int iItemID, bool bShootMode, bool bBackup=false );
void api_trigger_UnRideVehicle( CDNGameRoom* pRoom, UINT nUserObjectID );
void api_trigger_ChangeMyBGM( CDNGameRoom * pRoom, int nActorHandle, int nBGM, int nFadeDelta = 0 );
void api_trigger_FileTableBGMOff( CDNGameRoom * pRoom, int nActorHandle );
void api_trigger_RadioMsg( CDNGameRoom * pRoom, int nActorHandle, int nRadioID );
void api_trigger_RadioImage( CDNGameRoom * pRoom, int nActorHandle, int nFileID, DWORD nTime );
int api_trigger_GetPvPGameModePlayTime( CDNGameRoom * pRoom );
void api_trigger_DropGroupItemToEventArea( CDNGameRoom *pRoom, int nEventAreaHandle, int nItemID, DWORD dwOwnerActorHandle = 0 );
void api_trigger_DropGroupItemToActor( CDNGameRoom *pRoom, int nActorHandle, int nItemID, DWORD dwOwnerActorHandle = 0 );
void api_trigger_TargetActorShowNpc( CDNGameRoom * pRoom, int nActorHandle, int nEventAreaHandle, bool bShow );
void api_trigger_TargetActorShowProp( CDNGameRoom * pRoom, int nActorHandle, int nPropHandle, bool bShow );
int  api_trigger_TriggerVariableTable( CDNGameRoom * pRoom, int nItemID );
int  api_trigger_GetPvPGameMode( CDNGameRoom * pRoom );
void api_trigger_CmdKeepOperationProp( CDNGameRoom * pRoom, int nPropHandle, int nActorHandle, int nOperationTime, int nStringID, const char *szAction );
void api_trigger_ActivateMission( CDNGameRoom * pRoom, int nActorHandle, int nMissionID);
void api_trigger_SetDungeonClearRound( CDNGameRoom * pRoom, int iRound );
bool api_trigger_BreakIntoUserCheck( CDNGameRoom* pRoom );
bool api_trigger_DungeonStartTimeAttack( CDNGameRoom* pRoom, int iMin, int iSec );
int api_trigger_DungeonCheckTimeAttack( CDNGameRoom* pRoom );
void api_trigger_DungeonStopTimeAttack( CDNGameRoom* pRoom );
void api_trigger_RequestDungeonFail( CDNGameRoom* pRoom, bool bTimeOut=false );
void api_trigger_Special_RebirthItem( CDNGameRoom* pRoom, int nType, int nITemID, int nRebirthItemCount );
void api_trigger_GameWarpDungeonClearToLeader(CDNGameRoom* pRoom, bool bIgnoreClear = false);
void api_trigger_ActorBuffFromEventArea( CDNGameRoom *pRoom, int nActorHandle, int nEventAreaHandle, int nSkillID, int nSkillLevel, int nDelay, bool bSummonInclude );
void api_trigger_console_WriteLine( char * szOutputString, const char * s0, const char * s1, const char * s2, const char * s3, const char * s4, const char * s5,
								   const char * s6, const char * s7, const char * s8, const char * s9 );
int api_trigger_GetPartyCountbyRoom( CDNGameRoom* pRoom );
int api_trigger_GetPartyActorbyRoom( CDNGameRoom* pRoom, int nFirstPartyIndex );

int api_trigger_GetRacingStartTime( CDNGameRoom * pRoom );
int api_trigger_GetRacingGoalTime( CDNGameRoom * pRoom, const int nActorHandle );
int api_trigger_GetCurrentWeather( CDNGameRoom* pRoom );
void api_trigger_StartDungeonGateTime( CDNGameRoom* pRoom, int nGateNumber );
void api_trigger_EndDungeonGateTime( CDNGameRoom* pRoom, int nGateNumber  );
int api_trigger_StageRemainClearCount( CDNGameRoom* pRoom, UINT nUserObjectID, int nMapIndex );
void api_trigger_EnableRideVehicle( CDNGameRoom * pRoom, UINT nActorHandle, bool bEnableRide );
void api_trigger_AllMonsterUseSkill( CDNGameRoom * pRoom, int nSkillID, int nSkillLevel, int nDelay, bool bEternity );
bool api_trigger_RequireTotalComboCount( CDNGameRoom * pRoom, UINT nActorHandle, int nTotalComboCount );
void api_trigger_RequireTotalComboReset( CDNGameRoom * pRoom, UINT nActorHandle );

/*
desc   : Check the number of members of a specific class in the party
[Interger1] Return the count of the user class
param  : class (Interger1) 1 Warrior, 2 Archer, 3 Sorceress, 4 Cleric, 5 Academic, 6 Kali
return : number of [Interger1] class members in party
*/
int api_trigger_GetPartyInClassCount( CDNGameRoom* pRoom, int nClassID );

void api_trigger_Set_AlteiaWorld( CDNGameRoom* pRoom, bool bFlag );
void api_tirgger_Set_UserHPSP( CDNGameRoom * pRoom, UINT nActorHandle, int nHP_Ratio, int nSP_Ratio );
void api_tirgger_Set_UserHPSPNotGhost( CDNGameRoom * pRoom, UINT nActorHandle, int nHP_Ratio, int nSP_Ratio );

void api_trigger_Set_ItemUseCount( CDNGameRoom* pRoom, int nItemID, int nUseCount );	// Stage item usage count limit
int api_quest_ForceSkipQuest( CDNGameRoom *pRoom, UINT nUserObjectID, const char* szQuestList, int nSkipFlag );
int api_user_IsDNGirl( CDNGameRoom *pRoom, UINT nUserObjectID );
void api_trigger_BuffFromActor( CDNGameRoom *pRoom, int nActorHandle, int nSkillID, int nSkillLevel, int nDelay );
void api_trigger_RemoveBuffFromActor( CDNGameRoom *pRoom, int nActorHandle, int nSkillID );
void api_trigger_CancelKeepOperationProp( CDNGameRoom *pRoom, int nPropHandle, int nActorHandle );
int api_trigger_Get_KeepOperationPropClickSessionID( CDNGameRoom *pRoom, int nPropHandle );
bool api_trigger_CheckPartyAction( CDNGameRoom *pRoom, const char *szAction );
int api_trigger_CheckPlayState( CDNGameRoom *pRoom );
int api_trigger_Get_Random_PartyMemeber_From_EventArea( CDNGameRoom *pRoom, int nEventAreaHandle );
void api_trigger_ShowBalloonToPropIcon( CDNGameRoom *pRoom, int nPropHandle, int nActorHandle, bool bOnOff );
void api_trigger_WarpDieActor( CDNGameRoom *pRoom, int nActorHandle, int nTargetEventAreaHandle );
