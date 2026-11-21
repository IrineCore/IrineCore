#pragma once

// VillageServer Script API
//  [4/1/2008 nextome]

class CDNUserBase;

int CheckInvenForQuestReward(CDNUserBase* pUser, const TQuestReward& QuestReward, const bool bSelectedArray[]);

// Register all functions to Lua.
void DefAllAPIFunc(lua_State* pLuaState);

/*
desc : Link to the next dialogue.
param  : user index, NPC index, talk index, talk file name
return : void
*/
void api_npc_NextTalk(UINT nUserObjectID, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);

/*
desc : Link to the next script.
param  : user index, NPC index, talk index, talk file name
return : void
*/
void api_npc_NextScript(UINT nUserObjectID, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);

/*
desc : Set dialogue parameter (string).
param  : parameter key, parameter string index
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
sample : 
When building a sentence like 'I like the color {%color}.'
Make it like: api_SetParamString("{%color}, math.random(2) );
1 must be recorded separately in TalkParamTable.xls as 1: Red, 2: Blue.

Note: Parameters set with api_SetParamString are cleared after calling api_NextTalk, so set them before calling api_NextTalk.
*/
int api_npc_SetParamString(UINT nUserObjectID, const char* szParamKey, int nParamID);

/*
desc : Set dialogue parameter (integer).
param  : parameter key, integer value
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
sample : 
When building a sentence like 'I want to die around age {%age}.'
Make it like: api_SetParamInt("{%age}, math.random(30)+40 );

Note: Parameters set with api_SetParamInt are cleared after calling api_NextTalk, so set them before calling api_NextTalk.
*/
int api_npc_SetParamInt(UINT nUserObjectID, const char* szParamKey, int nValue);


/*
desc : Write a log.
param  : Include \n for line breaks in log string.
return : void
*/
void api_log_AddLog(const char* szLog);

/*
desc : Log a message to the user. Sent as a user chat message.
param  : Include \n for line breaks in log string.
return : void
*/
void api_log_UserLog(UINT nUserObjectID, const char* szLog);

/*
desc : Send quest-related information to the user via chat.
param  : 
return : void
*/
void api_quest_DumpQuest(UINT nUserObjectID);


/*
desc : Assign a quest to the user.
param  : user index, quest index, quest type (QuestType_SubQuest = 1, QuestType_MainQuest = 2)
return : 1 : success, -1 : user not found, -2 : user does not have quest
*/
int api_quest_AddHuntingQuest(UINT nUserObjectID, int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt);
int api_quest_AddQuest(UINT nUserObjectID, int nQuestID, int nQuestType);

/*
desc : Complete a quest for the user.
param  : user index, quest index, whether to remove from current quests (if not removed, only increments play count), repeatable flag
return : 1 : success, -1 : user not found, -2 : user does not have quest
*/
int api_quest_CompleteQuest(UINT nUserObjectID, int nQuestID, bool bDelPlayList, bool bRepeat);

/*
desc : Add quest to user's completion list. (Recorded in DB!!)
param  : user index, quest index
return : -1 => user not found, -2 => quest is not in progress
*/
int api_quest_MarkingCompleteQuest(UINT nUserObjectID, int nQuestID);

/*
desc : Check if quest is marked completed in completion list.
param  : user index, quest index
return : -1 => user not found, 1 => quest marked complete, 0 => not marked complete
*/
int api_quest_IsMarkingCompleteQuest(UINT nUserObjectID, int nQuestID);

/*
desc : Check if the user has the quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => has quest (returns quest step)
*/
int api_quest_UserHasQuest(UINT nUserObjectID, int nQuestID);

/*
desc : Check how many quests the user currently has in progress.
param  : user index, quest index
return : -1 => user not found, otherwise returns count of quests currently in progress
*/
int api_quest_GetPlayingQuestCnt(UINT nUserObjectID);


/*
desc : Return the NPC index.
param  : user index, NpcObjId (npc index and npcobject index are different)
return : -1 => user not found, -2 => npc not found, otherwise npc index
*/
int api_npc_GetNpcIndex(UINT nUserObjectID, UINT nNpcObjID);

/*
desc : Set quest step and journal step of user's quest.
param  : user index, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
*/
int api_quest_SetQuestStepAndJournalStep(UINT nUserObjectID, int nQuestID, short nQuestStep, int nJournalStep);

/*
desc : Set quest step of user's quest.
param  : user index, quest index, quest step
return : -1 => user not found, -2 => user does not have quest, returns 1 on success
*/
int api_quest_SetQuestStep(UINT nUserObjectID, int nQuestID, short nQuestStep);

/*
desc : Get quest step of user's quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current quest step
*/
int api_quest_GetQuestStep(UINT nUserObjectID, int nQuestID);

/*
desc : Set journal step of user's quest.
param  : user index, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
*/
int api_quest_SetJournalStep(UINT nUserObjectID, int nQuestID, int nJournalStep);

/*
desc : Get journal step of user's quest.
param  : user index, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current journal step
*/
int api_quest_GetJournalStep(UINT nUserObjectID, int nQuestID);

/*
desc : Set value at a specific index of user's quest memo.
param  : user index, quest number, memo index, memo value
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, 1 => success
*/
int api_quest_SetQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex, TP_QUESTMEMO nVal);

/*
desc : Get value at a specific index of user's quest memo.
param  : user index, quest number, memo index
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, > -1 => value of specific memo index
*/
int api_quest_GetQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Function to set counting info for killing monsters or acquiring items.
param  : user index,
Quest index,
Counting info setting slot (50 setting slots from 0 to 49),
Type to count (CountingType_MyMonsterKill = 1, CountingType_AllMonsterKill = 2, CountingType_Item = 3),
Index of item to count, range 0 ~ 60000,
Count amount
return : -1 => user not found, -2 => user does not have quest, -3 => counting info setting slot out of range, -4 => counting type or target count out of range

example: 

api_SetCountingInfo(userID, questID, 0, CountingType_MyMonsterKill, 10001, 10);
api_SetCountingInfo(userID, questID, 1, CountingType_MyMonsterKill, 10002,  5);
api_SetCountingInfo(userID, questID, 2, CountingType_MyMonsterKill, 10003, 20);
api_SetCountingInfo(userID, questID, 3, Item,    1001,  5);
api_SetCountingInfo(userID, questID, 4, Item,     1002,  1);

When called from script as above, 5 counting conditions are set.
The top line sets slot 0's condition to be satisfied upon killing 10 of monster 10001.

As above, the 5 slots are set to 10 of monster 10001, 5 of monster 10002, 20 of monster 10003, 5 of item 1001, and 1 of item 1002
When the above 5 conditions are met, OnComplete() of the quest script is called.

*** Max counting quantity is 30000.
*/
int api_quest_SetCountingInfo(UINT nUserObjectID, int nQuestID,  int nSlot, int nCountingType, int nCountingIndex, int nTargetCnt); 


/*
desc : Clear the configured counting info.
param  : user index, quest index
return : -1 => user not found, -2 => quest not found
*/
int api_quest_ClearCountingInfo(UINT nUserObjectID, int nQuestID);

/*
desc : Check if all configured countings are complete.
param  : user index, item index, item count
return : -1 => user not found, -2 => quest not found, -3 => not completed, 1 => all completed
*/
int api_quest_IsAllCompleteCounting(UINT nUserObjectID, int nQuestID);

/*
desc : Cancel the user's quest.
param  : user index, quest ID
return : -1 => user not found, -2 => fail, 1 => success
 */
int api_quest_CancelQuest(UINT nUserObjectID, int nQuestID);


/*
desc : Check if the user has enough inventory space for the item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_user_CheckInvenForAddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);

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
int api_user_CheckInvenForAddItemList(UINT nUserObjectID, lua_tinker::table ItemTable);

/*
desc : Grant an item to the user.
param  : user index, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_user_AddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID);

/*
desc : Remove the item from the user.
param  : user index, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient item count
*/
int api_user_DelItem(UINT nUserObjectID, int nItemIndex, int nItemCnt, int nQuestID);

/*
desc : Remove all of the item from the user.
param  : user index, item index
return : -1 => user not found, -2 => item not found
*/
int api_user_AllDelItem(UINT nUserObjectID, int nItemIndex);

/*
desc : Check if the user has the item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);


/*
desc : Get the user's class ID.
param  : user index
return : -1 => user not found, otherwise ClasssID
*/
int api_user_GetUserClassID(UINT nUserObjectID);


/*
desc : Get the user's Job ID.
param  : user index
return : -1 => user not found, otherwise Job ID
*/
int api_user_GetUserJobID(UINT nUserObjectID);


/*
desc : Get the user's level.
param  : user index
return : -1 => user not found, otherwise Level
*/
int api_user_GetUserLevel(UINT nUserObjectID);

/*
desc : Get number of empty slots in user's inventory.
param  : user index
return : -1 => user not found, otherwise empty slot count
*/
int api_user_GetUserInvenBlankCount(UINT nUserObjectID);

/*
desc : Get number of empty slots in user's quest inventory.
param  : user index
return : -1 => user not found, otherwise empty slot count
*/
int api_quest_GetUserQuestInvenBlankCount(UINT nUserObjectID);

/*
desc : Add EXP to the user.
param  : user index, EXP
return : -1 => user not found, 1 => success
*/
int api_user_AddExp(UINT nUserObjectID, int nQuestID, int nAddExp);

/*
desc : Add money to the user.
param  : user index, money
return : -1 => user not found, 1 => success
*/
int api_user_AddCoin(UINT nUserObjectID, int nAddCoin, int nQuestID);

/*
desc : Play a video for the user.
param  : user index, movie file name
return : -1 => user not found, 1 => success
*/
int api_user_PlayCutScene(UINT nUserObjectID, UINT nNpcObjectID, int nCutSceneTableID, bool bIgnoreFadeIn);

/*
desc : Notify the user to open the storage.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenWareHouse(UINT nUserObjectID, int iItemID/*=0*/ );

/*
desc : Notify the user to open the shop.
param  : user index, shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenShop(UINT nUserObjectID, int nShopID, Shop::Type::eCode Type=Shop::Type::Normal );

/*
desc : Notify the user to open the skill shop.
param  : user index, skill shop ID
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenSkillShop(UINT nUserObjectID);

/*
desc : Notify the user to open the unmanned shop.
param  : user index
return : -1 => user not found, -2 => no shop, 1 => success
*/
int api_ui_OpenMarket(UINT nUserObjectID);

/*
desc : Instruct the user to open the crest jewel synthesis window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCompoundEmblem(UINT nUserObjectID);


/*
desc : Notify the user to open the jewel upgrade window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenUpgradeJewel(UINT nUserObjectID);


/*
desc : Instruct the user to open the mailbox.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenMailBox(UINT nUserObjectID);

/*
desc : Instruct the user to open the crest window for crest removal.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenGlyphLift(UINT nUserObjectID);

/*
desc : Instruct the user to open the item enhancement window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenUpgradeItem(UINT nUserObjectID,int iItemID=0);

#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
/*
desc : Open the texture dialog.
param  : user index, file index, screen position, display duration, image center point,
return : -1 => user not found, 1 => success
*/
int api_ui_OpenTextureDialog(UINT nUserObjectID, int nFileIndex, float fX, float fY, int nPos, int nTime, int nDialogIndex, bool bAutoCloseDialog);

/*
desc : Close the texture dialog.
param  : user index, dialog ID
return : -1 => user not found, 1 => success
*/
int api_ui_CloseTextureDialog(UINT nUserObjectID, int nDlgID);
#endif


int api_ui_OpenDisjointItem(UINT nUserObjectID);


/*
desc : Instruct the user to open the item synthesis window.
param  : user index, item synthesis shop ID
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCompoundItem(UINT nUserObjectID, int nCompoundShopID);

/*
desc : Instruct the user to open item synthesis window 2.
param  : user index, item synthesis shop ID
return : -1 => user not found, 1 => success
*/

int api_ui_OpenCompound2Item(UINT nUserObjectID, int nCompoundGroupID, int iItemID/*=0*/ );

/*
desc : Instruct the user to open the cash item shop.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCashShop(UINT nUserObjectID);


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
int api_ui_OpenGuildMgrBox(UINT nUserObjectID, int nGuildMgrNo);

/*
desc : Open the Japan gachapon dialog.
param  : gachapon index to be used
return : -1 => user not found, -2 => no shop, 1 => success, -3 => filtered by define
*/
int api_ui_OpenGacha_JP( UINT nUserObjectID, int nGachaShopID );

/*
desc : Open the NPC gift dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenGiveNpcPresent( UINT nUserObjectID, int nNpcID );

/*
desc : Open the Dark Lair ranking board dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenDarkLairRankBoard( UINT nUserObjectID, int nNpcID, int iMapIndex, int iPlayerCount );


/*
desc : Open the PvP ladder ranking board dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
*/
int api_ui_OpenPvPLadderRankBoard( UINT nUserObjectID, int nNpcID, int iPvPLadderCode );
/*
desc : Display a message to the user.
param  : nType is message output type,
CHATTYPE_NORMAL   => 1, // Normal chat
CHATTYPE_PARTY    => 2, // Party chat
CHATTYPE_PRIVATE  => 3, // Whisper
CHATTYPE_GUILD    => 4, // Guild chat
CHATTYPE_CHANNEL  => 5, // Trade/channel chat
CHATTYPE_SYSTEM   => 6, // System message
nBaseStringIdx is the base string
ParamTable is a Lua table containing replacement info for each keyword
return : -1 => user not found, -2 invalid type, -3 message too long

example :

{%Hitter} hit {%Target} while equipping {item:2}. <-- uistring #6000
"Hitter_Human" <-- uistring #6001
"Target_Mantis" <-- uistring #6002

function grandmother_OnTalk(userObjID, npcObjID, npc_talk_index, npc_talk_target, questID)

    
	local TableParam =
	{
		// As below, the {%Hitter} keyword is replaced with ID 6001 in uistring.xml. 
		{ "{%Hitter}", 1, 6001 },
		{ "{%Target}", 1, 6002 },
	};

	// If { "{%Hitter}", 2, 6001 }, the integer 6001 is converted to string "6001" and substituted.

	api_UserMessage(userObjID, 1, 6000, TableParam);
end
*/

int api_user_UserMessage(UINT nUserObjectID, int nType, int nBaseStringIdx, lua_tinker::table ParamTable);

/*
desc : Add symbol item to the user.
param  : user index, symbol item ID, count
return : -1 => user not found, -2 => fail, 1 => success
*/
int api_quest_AddSymbolItem(UINT nUserObjectID, int nItemID, short wCount);

/*
desc : Delete symbol item from the user.
param  : user index, symbol item ID, count
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_DelSymbolItem(UINT nUserObjectID, int nItemID, short wCount);

/*
desc : Check if the user has the symbol item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_quest_HasSymbolItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has enough inventory space for the quest item.
param  : user index, quest item index, quest item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
*/
int api_quest_CheckQuestInvenForAddItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);

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
int api_quest_CheckQuestInvenForAddItemList(UINT nUserObjectID, lua_tinker::table ItemTable);

/*
desc : Add quest item to the user.
param  : user index, quest item ID, count, quest ID
return : -1 => user not found, -2 => fail, 1 => success
*/
int api_quest_AddQuestItem(UINT nUserObjectID, int nItemID, short wCount, int nQuestID);

/*
desc : Delete quest item from the user.
param  : user index, quest item ID, count, quest ID
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_DelQuestItem(UINT nUserObjectID, int nItemID, short wCount, int nQuestID);

/*
desc : Delete all quest items from the user.
param  : user index, quest item ID
return : -1 => user not found, -2 => failure, 1 => success
*/
int api_quest_AllDelQuestItem(UINT nUserObjectID, int nItemID);

/*
desc : Check if the user has the quest item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_quest_HasQuestItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Notify the user to open the guild creation window.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenGuildCreate(UINT nUserObjectID);

/*
desc : Notify the user to open the quest reward window.
param  : user index, reward table, bActivate (true if selectable, false for view-only)
return : -1 => user not found, 1 => success
*/
int api_ui_OpenQuestReward(UINT nUserObjectID, int nRewardTableIndex, bool bActivate);

/*
desc : Give quest reward to the user.
param  : user index, reward table, quest ID, reward check number
return : -1 => user not found, -2 user item object error, -3 insufficient inventory, -4 table not found, -5 reward condition check error, 1 => success
*/
int api_quest_RewardQuestUser(UINT nUserObjectID, int nRewardTableIndext, int nQuestID, int nRewardCheck);

/*
desc : Move the user to another channel.
param  : user index, channel type
return : -1 => user not found, -2 invalid channel type, -3 invalid channel
*/
int api_user_ChangeChannel(UINT nUserObjectID, int ChannelType );

/*
desc : Move the user.
param  : user index, target map index, target gate index
return : -1 => user not found, -2 => map does not exist
*/
int api_user_ChangeMap(UINT nUserObjectID, int nMapIndex, int nGateNo);

/*
desc   : Return the number of party members for the user.
param  : user index
return : -1 => user not found, otherwise party member count (Note: returns 1 even if user is not in a party)
*/
int api_user_GetPartymemberCount(UINT nUserObjectID);

/*
desc   : Check if the user is in a party.
param  : user index
return : -1 => user not found, 0 => not in party, 1 => in party
*/
int api_user_IsPartymember(UINT nUserObjectID);

//blondy
/*
desc   : Direct the user to the PVP lobby.
param  : user index
return : 1 => success, -1 => user not found
*/
int api_user_RequestEnterPVP( UINT nUserObjectID );

/*
desc   : Get the user's stage difficulty
param  : user index
return : 0 ~ 4 => dungeon difficulty (0:Easy / 1:Normal / 2:Hard / 3:Master / 4:Abyss), otherwise fail (default -1)
*/
int api_user_GetStageConstructionLevel(UINT nUserObjectID);

/*
desc   : Get the user's current map ID
param  : user index
return : map number (>= 1), otherwise fail (<= 0)
*/
int api_user_GetMapIndex(UINT nUserObjectID);

/*
desc   : Get the user's latest stage clear rank
param  : user index
return : stage clear rank (SSS : 0 / SS : 1 / S : 2 / A : 3 / B : 4 / C : 5 / D : 6), otherwise fail (default -1)
*/
int api_user_GetLastStageClearRank(UINT nUserObjectID);

/*
desc : Check the user's current money.
param  : user index, value to check
return : -1 => user not found, -2 => insufficient gold, 1 => sufficient gold
*/
int api_user_EnoughCoin(UINT nUserObjectID, int nCoin );

/*
desc : Check the user's current money.
param  : user index
return : -1 => user not found, >= 0 => current money
*/
int api_user_GetCoin(UINT nUserObjectID);

/*
desc : Deduct money from the user.
param  : user index, money
return : -1 => user not found, 1 => success
*/
int api_user_DelCoin(UINT nUserObjectID, int nDelCoin);

/*
desc : Return whether the user acquired a specific mission
param  : user index, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
*/
int api_user_IsMissionGained(UINT nUserObjectID, int nMissionIndex);

/*
desc : Return whether the user completed a specific mission
param  : user index, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
*/
int api_user_IsMissionAchieved(UINT nUserObjectID, int nMissionIndex);

/*
desc : Check if the user has the item in storage.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasItemWarehouse(UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has the item equipped.
param  : user index, item index
return : -1 => user not found, -2 => item not found, 1 or other => true
*/
int api_user_HasItemEquip(UINT nUserObjectID, int nItemIndex);

/*
desc : Return the user's current guild rank
param  : user index
return : -1 : user not found / 0 : no guild / 1 : guild master / 2 : vice guild master / 3 : senior guild member / 4 : regular guild member / 5 : recruit guild member
*/
int api_guild_GetGuildMemberRole(UINT nUserObjectID);

/*
desc : Increment value at a specific index of user's quest memo by 1
param  : user index, quest number, memo index
return : incremented quest memo value (returns LONG_MIN on error)
*/
int api_quest_IncQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Decrement value at a specific index of user's quest memo by 1
param  : user index, quest number, memo index
return : decremented quest memo value (returns LONG_MAX on error)
*/
int api_quest_DecQuestMemo(UINT nUserObjectID, int nQuestID, char nMemoIndex);

/*
desc : Assign a specific job advancement class to the user
param  : user index, target job class
return : -1 : user not found / -2 : invalid job class ID / -3 : job change condition not met / 0 : already the same job class / 1 : job class changed successfully
*/
int api_user_SetUserJobID(UINT nUserObjectID, int nJobID);

/*
desc : Check if the user's job belongs to a specific job branch
param  : user index, prerequisite job class
return : -1 : user not found / 0 : not same class line / 1 : same class line
*/
int api_user_IsJobInSameLine(UINT nUserObjectID, int nBaseJobID);

/*
desc : Check if the user has the cash item.
param  : user index, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
*/
int api_user_HasCashItem(UINT nUserObjectID, int nItemIndex, int nItemCnt);

/*
desc : Check if the user has the cash item equipped.
param  : user index, item index
return : -1 => user not found, -2 => item not found, 1 or other => true
*/
int api_user_HasCashItemEquip(UINT nUserObjectID, int nItemIndex);

/*
desc   : Arbitrarily increment the count for a specific counting type and ID.
param  : user index,
Type to count (CountingType_MyMonsterKill = 1, CountingType_AllMonsterKill = 2, CountingType_Item = 3),
Index of item to count (Item ID, Monster ID, etc.)
return : -1 => user not found, 1 => success
*/
int api_quest_IncCounting(UINT nUserObjectID, int nCountingType, int nCountingIndex);

/*
desc : Return whether the user has reached the maximum number of active quests.
param  : user index, quest index
return : -1 : user not found, 1 : max quest count reached, otherwise : no
*/
int api_quest_IsPlayingQuestMaximum(UINT nUserObjectID);

/*
desc   : Force-complete a specific quest. (If not in progress, add and complete it)
param  : user index, quest index, quest code (0: normal / 1: timed), mark flag (1: mark / 0: do not mark), delete flag (1: delete / 0: do not delete), repeat flag (1: repeat / 0: do not repeat)
return : -1 : user not found, 1 : max quest count reached, otherwise : no
*/
int api_quest_ForceCompleteQuest(UINT nUserObjectID, int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat);


/*
desc : Reset skills
param  : user index, quest index,
return : -1 : user not found, -2 : quest not found, -3 : quest already completed, 1 : skill reset succeeded
*/
int api_quest_ResetSkill(UINT nUserObjectID, int nQuestID);

/*
desc : Return world event completion step.
param  : user index, schedule ID
return : -1 : error, 0 : complete, 1 : start notice, 2 : collection period, 3 : reward notice, 4 : reward period
*/
int api_quest_GetWorldEventStep(UINT nUserObjectID, int nScheduleID);

/*
desc : Return world event completion count.
param  : user index, schedule ID
return : -1 : error, otherwise current count
*/
int api_quest_GetWorldEventCount (UINT nUserObjectID, int nScheduleID);


/*
desc : Turn in item to NPC and accumulate count in quest.
param  : user index, schedule ID
return : -1 : error, otherwise returned count
*/
int api_quest_ReturnItemToNpc (UINT nUserObjectID, int nScheduleID);

/*
desc : Return current count of items turned in to NPC.
param  : user index, schedule ID
*/
void api_quest_OpenScoreWorldEvent (UINT nUserObjectID, int nScheduleID);

/*
desc   : Show the farm area list to a specific user
param  : user index
return : -1 : user not found, 1 : success
*/
int api_quest_OpenFarmAreaList (UINT nUserObjectID);

/*
desc   : Move a specific user's position.
param  : user index, X coord, Y coord, Z coord
return : -1 : user not found, 1 : success
*/
int api_user_ChangePos(UINT nUserObjectID, int nX, int nY, int nZ , int nLookX = 0, int nLookY = 0);

/*
desc : Get NPC favor points.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetFavorPoint( UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC malice points.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetMalicePoint( UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC favor percentage.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetFavorPercent( UINT uiUserObjectID, int iNpcID );

/*
desc : Get NPC malice percentage.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_GetMalicePercent( UINT uiUserObjectID, int iNpcID );

/*
desc : Add or subtract NPC favor points.
param  : user index, NpcID, delta value (negative value decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_AddFavorPoint( UINT uiUserObjectID, int iNpcID, int val );

/*
desc : Add or subtract NPC malice points.
param  : user index, NpcID, delta value (negative value decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_AddMalicePoint( UINT uiUserObjectID, int iNpcID, int val );

/*
desc : Request client to send selected NPC gift info to server.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_SendSelectedPresent( UINT uiUserObjectID, int iNpcID );

/*
desc : Request client to play anger effect on NPC.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_Rage( UINT uiUserObjectID, int iNpcID );

/*
desc : Request client to play disappointment effect on NPC.
param  : user index, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
*/
int api_npc_Disappoint( UINT uiUserObjectID, int iNpcID );


/*
desc   : Reset all skills of a specific user.
param  : user index
return : -1 : user not found, 1 : success
*/
int api_user_ResetSkill(UINT nUserObjectID);

/*
desc : Have a specific user newly learn a sub-skill.
param  : user index, sub-skill ID
return : -1 : user not found, 1 : success, 0 : failure
*/
int api_npc_CreateSecondarySkill( UINT uiObjectUserID, int iSkillID );

/*
desc : Check if a specific user has learned a sub-skill
param  : user index, sub-skill ID
return : -1 : user not found, 1 : success, 0 : failure
*/
int api_npc_CheckSecondarySkill( UINT uiObjectUserID, int iSkillID );

/*
desc : Notify the user to open the guild storage.
param : user index
return  : -1 => user not found, -2 => not a guild member, 1 => success
*/
int api_ui_OpenGuildWareHouse (UINT nUserObjectID);

/*
desc : Return whether the user's current guild owns a guild storage.
param  : user index
return : -1 : user not found / 0 : no guild storage / 1 : has guild storage
*/
int api_guild_HasGuildWare(UINT nUserObjectID);


/*
desc : Instruct the user to open the movie clip browser.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenMovieBrowser(UINT nUserObjectID);

/*
desc : Return whether registered for guild war.
param  : user index
return : -1 : user not found / 0 : cannot apply for guild war / 1 : can apply for guild war
*/
int api_guildwar_IsPreparation(UINT nUserObjectID);
/*
desc : Check guild war period.
param  : user index
return : -1 : user not found / eGuildWarStepType / 0 : not guild war period / 1 : guild war registration period / 2 : guild war preliminaries period / 3 : finals and reward period
*/
int api_guildwar_GetStep(UINT nUserObjectID);
/*
desc : View guild war preliminary progress
param  : user index
return : -1 => user not found, -2 => not in preliminary period, 0 : success
*/
int api_guildwar_GetTrialCurrentState(UINT nUserObjectID);
/*
desc : Guild war preliminary results
param  : user index
return : -1 => user not found, -2 => not in finals or reward period, -3 => preliminary results not yet tallied, 0 : success
*/
int api_guildWar_GetTrialStats(UINT nUserObjectID);
/*
desc : View guild war finals bracket
param  : user index
return : -1 => user not found, -2 => not in finals or reward period, -3 => bracket not set, 0: success
*/
int api_guildWar_GetMatchList(UINT nUserObjectID);
/*
desc : Guild war popularity vote results
param  : user index
return : -1 => user not found, -2 => no previous popularity vote results, -3 => guild war in progress, 0: success
*/
int api_guildWar_GetPopularityVote(UINT nUserObjectID);

/*
desc : Check whether guild war main tournament exists.
param  : user index
return : -1 : user not found / 0 : finals not in progress / 1 : finals in progress
*/
int api_guildWar_GetFinalProgress(UINT nUserObjectID);

/*
desc : Instruct the user to open the browser.
param  : user index
return : -1 => user not found, 1 => success
*/
int api_ui_OpenBrowser(UINT nUserObjectID, const char* szUrl, int nX, int nY, int nWidth, int nHeight);

/*
desc : Open the donation window.
param	: 
return  : -1 => user not found, 1 => success.
*/
int api_ui_OpenDonation(UINT nUserObjectID);


/*
desc : Open the inventory window.
param	: 
return  : -1 => user not found, 1 => success.
*/
int api_ui_OpenInventory(UINT nUserObjectID);

/*
desc : User's PC Cafe grade
param	:
return  : -1 => user not found, 0 => not a PC bang, otherwise PC bang grade.
*/
int api_user_GetPCCafe(UINT nUserObjectID);

/*
desc : Create and give rental items such as weapons and mounts to PC cafe users.
param	:
return  : -1 => user not found, 0 => success, 1 => insufficient inventory space, 2 => already has rental item, 3 =>
*/
int api_user_SetPCCafeItem(UINT nUserObjectID);

/*
desc : Repair user's equipped items and deduct repair cost.
param  : user, repair all flag (1: repair all, 0: equipped items)
return : -1 : user not found / 0 : no items to repair / 1 : insufficient gold / 2 : item repair complete
*/
int api_user_RepairItem(UINT nUserObjectID, int iAllRepair);


/*
desc : Instruct the user to open the enhancement transfer window.
param  : user
return : -1 => user not found, 1 => success
*/
int api_ui_OpenExchangeEnchant(UINT nUserObjectID);

/*
desc   : Check if the map is closed.
param  : user, map index
return : 1 => open, 0 => closed
*/
int api_env_CheckCloseGateByTime(int iMapIndex);

/*
desc : Open the costume random synthesis window
param  : user
return : -1 => user not found, 1 => success
*/
int api_ui_OpenCostumeRandomMix(UINT nUserObjectID);

/*
desc : Check if world buff can be given
param  : buff item ID
return : -1 => failure, 1 => success
*/
int api_quest_WorldBuffCheck(int nItemID);

/*
desc : Give world buff
param  : buff item ID
return : 
*/
int api_quest_ApplyWorldBuff(UINT nUserObjectID, int nItemID, int nMapIdx);
int api_quest_ForceSkipQuest(UINT nUserObjectID, const char* szQuestList, int nSkipFlag);
int api_user_IsDNGirl(UINT nUserObjectID);
