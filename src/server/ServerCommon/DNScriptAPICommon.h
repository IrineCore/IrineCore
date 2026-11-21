#pragma once

class CDNUserBase;
class CDnNpc;

namespace DNScriptAPI
{
	int CheckInvenForQuestReward(CDNUserBase* pUser, const TQuestReward& QuestReward, const bool bSelectedArray[]);

	/*
desc : Link to the next dialogue.
param  : user (must be CDNUserSession*), NPC index, talk index, talk filename
	return : void
	*/
	void api_npc_NextTalk(CDNUserBase* pUser, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);

	/*
desc : Link to the next script.
param  : user (must be CDNUserSession*), NPC, talk index, talk filename
	return : void
	*/
	void api_npc_NextScript(CDNUserBase* pUser, CDnNpc* pNpc, UINT nNpcObjectID, const char* szTalkIndex, const char* szTargetFile);

	/*
desc : Set dialogue parameter (string).
param  : user, parameter key, parameter string index
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
	sample : 
When building a sentence like 'I like the color {%color}.'
Make it like: api_npc_SetParamString("{%color}, math.random(2) );
1 must be recorded separately in TalkParamTable.xls as 1: Red, 2: Blue.

Note: Parameters set with api_npc_SetParamString are cleared after calling api_npc_NextTalk, so set them before calling api_npc_NextTalk.
	*/
	int api_npc_SetParamString(CDNUserBase* pUser, const char* szParamKey, int nParamID);

	/*
desc : Set dialogue parameter (integer).
param  : user, parameter key, integer value
return : -1 => user not found, -2 => parameter count exceeded, cannot set more, 1 => success
	sample : 
When building a sentence like 'I want to die around age {%age}.'
Make it like: api_npc_SetParamInt("{%age}, math.random(30)+40 );

Note: Parameters set with api_npc_SetParamInt are cleared after calling api_npc_NextTalk, so set them before calling api_npc_NextTalk.
	*/
	int api_npc_SetParamInt(CDNUserBase* pUser, const char* szParamKey, int nValue);

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
	void api_log_UserLog(CDNUserBase* pUser, const char* szLog);

	/*
desc : Send quest-related information to the user via chat.
	param  : 
	return : void
	*/
	void api_quest_DumpQuest(CDNUserBase* pUser);

	/*
desc : Assign a quest to the user.
param  : user, quest index, quest type (QuestType_SubQuest = 1, QuestType_MainQuest = 2)
return : 1 : success, -1 : user not found, -2 : user does not have quest
	*/
	int api_quest_AddHuntingQuest(CDNUserBase* pUser, int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt);
	int api_quest_AddQuest(CDNUserBase* pUser, int nQuestID, int nQuestType);

	/*
desc : Complete a quest for the user.
param  : user, quest index, whether to remove from current quests (if not removed, only increments play count), repeatable flag
return : -1 => user not found, -2 => quest is not in progress
	*/
	int api_quest_CompleteQuest(CDNUserBase* pUser, int nQuestID, bool bDelPlayList);

	/*
desc : Check if quest is marked completed in completion list.
param  : user, quest index
return : -1 => user not found, 1 => quest marked complete, 0 => not marked complete
	*/
	int api_quest_IsMarkingCompleteQuest(CDNUserBase* pUser, int nQuestID);

	/*
desc : Check if the user has the quest.
param  : user, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => has quest (returns quest step)
	*/
	int api_quest_UserHasQuest(CDNUserBase* pUser, int nQuestID);

	/*
desc : Check how many quests the user currently has in progress.
param  : user
return : -1 => user not found, otherwise returns count of quests currently in progress
	*/
	int api_quest_GetPlayingQuestCnt(CDNUserBase* pUser);
	
	/*
desc : Return the NPC index.
	param  : Npc
return : -2 => npc not found, otherwise npc index
	*/
	int api_npc_GetNpcIndex(CDnNpc* pNpc);

	/*
desc : Set quest step and journal step of user's quest.
param  : user, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
	*/
	int api_quest_SetQuestStepAndJournalStep(CDNUserBase* pUser, int nQuestID, short nQuestStep, int nJournalStep);
	
	/*
desc : Set quest step of user's quest.
param  : user, quest index, quest step
return : -1 => user not found, -2 => user does not have quest, returns 1 on success
	*/
	int api_quest_SetQuestStep(CDNUserBase* pUser, int nQuestID, short nQuestStep);
	
	/*
desc : Get quest step of user's quest.
param  : user, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current quest step
	*/
	int api_quest_GetQuestStep(CDNUserBase* pUser, int nQuestID);

	/*
desc : Set journal step of user's quest.
param  : user, quest index, journal step
return : -1 => user not found, -2 => user does not have quest, -3 => journal step out of range, returns 1 on success
	*/
	int api_quest_SetJournalStep(CDNUserBase* pUser, int nQuestID, int nJournalStep);

	/*
desc : Get journal step of user's quest.
param  : user, quest index
return : -1 => user not found, -2 => user does not have quest, > -1 => current journal step
	*/
	int api_quest_GetJournalStep(CDNUserBase* pUser, int nQuestID);
	
	/*
desc : Set value at a specific index of user's quest memo.
param  : user, memo index, memo value
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, 1 => success
	*/
	int api_quest_SetQuestMemo(CDNUserBase* pUser, int nQuestID, char nMemoIndex, int iVal);

	/*
desc : Get value at a specific index of user's quest memo.
param  : user, memo index
return : -1 => user not found, -2 => user does not have quest, -3 => memo index out of range, > -1 => value of specific memo index
	*/
	int api_quest_GetQuestMemo(CDNUserBase* pUser, int nQuestID, char nMemoIndex);
	
	/*
desc : Function to set counting info for killing monsters or acquiring items.
param  : user,
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
	int api_quest_SetCountingInfo(CDNUserBase* pUser, int nQuestID,  int nSlot, int nCountingType, int nCountingIndex, int nTargetCnt); 


	/*
desc : Clear the configured counting info.
param  : user, quest index
return : -1 => user not found, -2 => quest not found
	*/
	int api_quest_ClearCountingInfo(CDNUserBase* pUser, int nQuestID);
	
	
	/*
desc : Check if all configured countings are complete.
param  : user, item index, item count
return : -1 => user not found, -2 => quest not found, -3 => not completed, 1 => all completed
	*/
	int api_quest_IsAllCompleteCounting(CDNUserBase* pUser, int nQuestID);

	/*
desc : Check if the user has enough inventory space for the item.
param  : user, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
	*/
	int api_user_CheckInvenForAddItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);
	
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
	int api_user_CheckInvenForAddItemList(CDNUserBase* pUser, lua_tinker::table ItemTable);
	
	/*
desc : Grant an item to the user.
param  : user, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
	*/
	int api_user_AddItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt, int nQuestID);

	/*
desc : Remove the item from the user.
param  : user, item index, item count, quest ID
return : -1 => user not found, -2 => item not found, -3 => insufficient item count
	*/
	int api_user_DelItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt, int nQuestID);

	/*
desc : Remove all of the item from the user.
param  : user, item index, item count, quest ID
return : -1 => user not found, -2 => item not found
	*/
	int api_user_AllDelItem(CDNUserBase* pUser, int nItemIndex);

	/*
desc : Check if the user has the item.
param  : user, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
	*/
	int api_user_HasItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);

	/*
desc : Get the user's class ID.
param  : user
return : -1 => user not found, otherwise ClasssID
	*/
	int api_user_GetUserClassID(CDNUserBase* pUser);

	/*
desc : Get the user's Job ID.
param  : user
return : -1 => user not found, otherwise Job ID
	*/
	int api_user_GetUserJobID(CDNUserBase* pUser);

	/*
desc : Get the user's level.
param  : user
return : -1 => user not found, otherwise Level
	*/
	int api_user_GetUserLevel(CDNUserBase* pUser);

	/*
desc : Get number of empty slots in user's inventory.
param  : user
return : -1 => user not found, otherwise empty slot count
	*/
	int api_user_GetUserInvenBlankCount(CDNUserBase* pUser);

	/*
desc : Get number of empty slots in user's quest inventory.
param  : user
return : -1 => user not found, otherwise empty slot count
	*/
	int api_quest_GetUserQuestInvenBlankCount(CDNUserBase* pUser);
	
	/*
desc : Play a video for the user.
param  : user, movie filename
return : -1 => user not found, 1 => success
	*/
	int api_user_PlayCutScene(CDNUserBase* pUser, UINT nNpcObjectID, int nCutSceneTableID, bool bIgnoreFadeIn);

	/*
desc : Notify the user to open the storage.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenWareHouse(CDNUserBase* pUser, int iItemID=0 );

#if defined (PRE_MOD_GAMESERVERSHOP)
	/*
desc : Notify the user to open the shop.
param  : user index, shop ID
return : -1 => user not found, -2 => no shop, 1 => success
	*/
	int api_ui_OpenShop(CDNUserBase* pUser, int nShopID, Shop::Type::eCode Type);

	/*
desc : Notify the user to open the skill shop.
param  : user index, skill shop ID
return : -1 => user not found, -2 => no shop, 1 => success
	*/
	int api_ui_OpenSkillShop(CDNUserBase* pUser);
#endif
	
	/*
desc : Instruct the user to open the crest jewel synthesis window.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenCompoundEmblem(CDNUserBase* pUser);
	
	/*
desc : Notify the user to open the jewel upgrade window.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenUpgradeJewel(CDNUserBase* pUser);

	/*
desc : Instruct the user to open the mailbox.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenMailBox(CDNUserBase* pUser);

	/*
desc : Instruct the user to open the crest window for crest removal.
param  : user index
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenGlyphLift(CDNUserBase* pUser);
	
	/*
desc : Instruct the user to open the disassembly window.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenDisjointItem(CDNUserBase* pUser);
	
	/*
desc : Instruct the user to open the item synthesis window.
param  : user, item synthesis shop ID
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenCompoundItem(CDNUserBase* pUser, int nCompoundShopID);
	
	/*
desc : Instruct the user to open the cash item shop.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenCashShop(CDNUserBase* pUser);

	/*
desc : Instruct the user to open the guild management UI dialog.
param  : user, guild management ID
return : -1 => user not found, 1 => success

Guild management ID>
0 : Create guild
1 : Disband guild
2 : Leave guild
3: Guild points
4: Guild level up
	*/
	int api_ui_OpenGuildMgrBox(CDNUserBase* pUser, int nGuildMgrNo);
	
	/*
desc : Open the Japan gachapon dialog.
param  : gachapon index to be used
return : -1 => user not found, -2 => no shop, 1 => success, -3 => filtered by define
	*/
	int api_ui_OpenGacha_JP(CDNUserBase* pUser, int nGachaShopID);
	
	/*
desc : Open the NPC gift dialog.
param  : ID of NPC in conversation
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenGiveNpcPresent(CDNUserBase* pUser, int nNpcID);

	/*
desc : Open the donation window.
	param	: 
return  : -1 => user not found, 1 => success.
	*/
	int api_ui_OpenDonation(CDNUserBase* pUser);


	/*
desc : Open the inventory window.
	param	: 
return  : -1 => user not found, 1 => success.
	*/
	int api_ui_OpenInventory(CDNUserBase* pUser);

#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
	/*
desc : Open the texture dialog
param  : user index, dialog data
return : -1 => user not found, 1 => success
	*/
	int api_Open_Texture_Dialog(CDNUserBase* pUser, SCOpenTextureDialog data);


	/*
desc : Close the texture dialog
param  : user index, dialog data
return : -1 => user not found, 1 => success
	*/
	int api_Close_Texture_Dialog(CDNUserBase* pUser, int nDlgID);
#endif

	int api_quest_ForceSkipQuest(CDNUserBase* pUser, const char* szQuestList, int nSkipFlag);
	int api_user_IsDNGirl(CDNUserBase* pUser);

	

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
	int api_user_UserMessage(CDNUserBase* pUser, int nType, int nBaseStringIdx, lua_tinker::table ParamTable);
	
	/*
desc : Add symbol item to the user.
param  : user, symbol item ID, count
return : -1 => user not found, -2 => fail, 1 => success
	*/
	int api_quest_AddSymbolItem(CDNUserBase* pUser, int nItemID, short wCount);

	/*
desc : Delete symbol item from the user.
param  : user, symbol item ID, count
return : -1 => user not found, -2 => failure, 1 => success
	*/
	int api_quest_DelSymbolItem(CDNUserBase* pUser, int nItemID, short wCount);

	/*
desc : Check if the user has the symbol item.
param  : user, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
	*/
	int api_quest_HasSymbolItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);
	
	/*
desc : Check if the user has enough inventory space for the quest item.
param  : user, quest item index, quest item count
return : -1 => user not found, -2 => item not found, -3 => insufficient inventory space
	*/
	int api_quest_CheckQuestInvenForAddItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);
	
	/*
desc : Check if the user has enough inventory space for the quest item list.
param  : user, quest item list (Lua table, [<max count>]{quest item index, quest item count} pairs up to <max count (currently 10)>)
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
	int api_quest_CheckQuestInvenForAddItemList(CDNUserBase* pUser, lua_tinker::table ItemTable);

	/*
desc : Add quest item to the user.
param  : user, quest item ID, count, quest ID
return : -1 => user not found, -2 => fail, 1 => success
	*/
	int api_quest_AddQuestItem(CDNUserBase* pUser, int nItemID, short wCount, int nQuestID);

	/*
desc : Delete quest item from the user.
param  : user, quest item ID, count, quest ID
return : -1 => user not found, -2 => failure, 1 => success
	*/
	int api_quest_DelQuestItem(CDNUserBase* pUser, int nItemID, short wCount, int nQuestID);

	/*
desc : Delete all quest items from the user.
param  : user, quest item ID
return : -1 => user not found, -2 => failure, 1 => success
	*/
	int api_quest_AllDelQuestItem(CDNUserBase* pUser, int nItemID);

	/*
desc : Check if the user has the quest item.
param  : user, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
	*/
	int api_quest_HasQuestItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);
	
	/*
desc : Notify the user to open the guild creation window.
param  : user index
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenGuildCreate(CDNUserBase* pUser);

	/*
desc : Notify the user to open the quest reward window.
param  : user index, reward table, bActivate (true if selectable, false for view-only)
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenQuestReward(CDNUserBase* pUser, int nRewardTableIndex, bool bActivate);
	
	/*
desc : Give quest reward to the user.
param  : user, reward table, quest ID, reward check number
return : -1 => user not found, -2 user item object error, -3 insufficient inventory, -4 table not found, -5 reward condition check error, 1 => success
	*/
	int api_quest_RewardQuestUser(CDNUserBase* pUser, int nRewardTableIndext, int nQuestID, int nRewardCheck);

	/*
desc   : Get the user's current map ID
param  : user
return : map number (>= 1), otherwise fail (<= 0)
	*/
	int api_user_GetMapIndex(CDNUserBase* pUser);
	
	/*
desc   : Get the user's latest stage clear rank
param  : user index
return : stage clear rank (SSS : 0 / SS : 1 / S : 2 / A : 3 / B : 4 / C : 5 / D : 6), otherwise fail (default -1)
	*/
	int api_user_GetLastStageClearRank(CDNUserBase* pUser);
	
	/*
desc : Check the user's current money.
param  : user, value to check
return : -1 => user not found, -2 => insufficient gold, 1 => sufficient gold
	*/
	int api_user_EnoughCoin(CDNUserBase* pUser, int nCoin);
	
	/*
desc : Check the user's current money.
param  : user
return : -1 => user not found, >= 0 => current money
	*/
	int api_user_GetCoin(CDNUserBase* pUser);
	
	/*
desc : Return whether the user acquired a specific mission
param  : user, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
	*/
	int api_user_IsMissionGained(CDNUserBase* pUser, int nMissionIndex);
	
	/*
desc : Return whether the user completed a specific mission
param  : user, mission index (MISSIONMAX (currently 5000 -> 0 ~ 4999))
return : -1 => user not found, 0 => mission does not exist, 1 or other => mission exists
	*/
	int api_user_IsMissionAchieved(CDNUserBase* pUser, int nMissionIndex);
	
	/*
desc : Check if the user has the item in storage.
param  : user, item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
	*/
	int api_user_HasItemWarehouse(CDNUserBase* pUser, int nItemIndex, int nItemCnt);

	/*
desc : Check if the user has the item equipped.
param  : user, item index
return : -1 => user not found, -2 => item not found, 1 or other => true
	*/
	int api_user_HasItemEquip(CDNUserBase* pUser, int nItemIndex);
	
	/*
desc : Return the user's current guild rank
param  : user
return : 0 : no guild / 1 : guild master / 2 : vice guild master / 3 : senior guild member / 4 : regular guild member / 5 : junior guild member
	*/
	int api_guild_GetGuildMemberRole(CDNUserBase* pUser);
	
	/*
desc : Increment value at a specific index of user's quest memo by 1
param  : user, quest number, memo index
return : incremented quest memo value (returns LONG_MIN on error)
	*/
	int api_quest_IncQuestMemo(CDNUserBase* pUser, int nQuestID, char nMemoIndex);

	/*
desc : Decrement value at a specific index of user's quest memo by 1
param  : user, quest number, memo index
return : incremented quest memo value (returns LONG_MAX on error)
	*/
	int api_quest_DecQuestMemo(CDNUserBase* pUser, int nQuestID, char nMemoIndex);
	
	/*
desc : Assign a specific job advancement class to the user
param  : user, target advancement job
return : -1 : user not found / -2 : invalid job class ID / -3 : job change condition not met / 0 : already the same job class / 1 : job class changed successfully
	*/
	int api_user_SetUserJobID(CDNUserBase* pUser, int nJobID);
	
	/*
desc : Check if the user's job belongs to a specific job branch
param  : user, prerequisite advancement job
return : -1 : user not found / 0 : not same class line / 1 : same class line
	*/
	int api_user_IsJobInSameLine(CDNUserBase* pUser, int nBaseJobID);
	
	/*
desc : Check if the user has the cash item.
param  : user, cash item index, item count
return : -1 => user not found, -2 => item not found, -3 => insufficient item count, > 0 => true (returns count)
	*/
	int api_user_HasCashItem(CDNUserBase* pUser, int nItemIndex, int nItemCnt);

	/*
desc : Check if the user has the cash item equipped.
param  : user, cash item index
return : -1 => user not found, -2 => item not found, 1 or other => true
	*/
	int api_user_HasCashItemEquip(CDNUserBase* pUser, int nItemIndex);
	
	/*
desc   : Arbitrarily increment the count for a specific counting type and ID.
param  : user,
Type to count (CountingType_MyMonsterKill = 1, CountingType_AllMonsterKill = 2, CountingType_Item = 3),
Index of item to count (Item ID, Monster ID, etc.)
return : -1 => user not found, 1 => success
	*/
	int api_quest_IncCounting(CDNUserBase* pUser, int nCountingType, int nCountingIndex);
	
	/*
desc : Return whether the user has reached the maximum number of active quests.
param  : user, quest index
return : -1 : user not found, 1 : max quest count reached, otherwise : no
	*/
	int api_quest_IsPlayingQuestMaximum(CDNUserBase* pUser);
	
	/*
desc   : Force-complete a specific quest. (If not in progress, add and complete it)
param  : user, quest index, quest code (0: normal / 1: timed), mark flag (1: mark / 0: do not mark), delete flag (1: delete / 0: do not delete), repeat flag (1: repeat / 0: do not repeat)
return : -1 : user not found, 1 : max quest count reached, otherwise : no
	*/
	int api_quest_ForceCompleteQuest(CDNUserBase* pUser, int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat);
	
	/*
desc : Get NPC favor points.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_GetFavorPoint(CDNUserBase* pUser, int iNpcID);
	
	/*
desc : Get NPC malice points.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_GetMalicePoint(CDNUserBase* pUser, int iNpcID);

	/*
desc : Get NPC favor percentage.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_GetFavorPercent(CDNUserBase* pUser, int iNpcID);

	/*
desc : Get NPC malice percentage.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_GetMalicePercent(CDNUserBase* pUser, int iNpcID);

	/*
desc : Add or subtract NPC favor points.
param  : user, NpcID, delta value (negative decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_AddFavorPoint(CDNUserBase* pUser, int iNpcID, int val);

	/*
desc : Add or subtract NPC malice points.
param  : user, NpcID, delta value (negative decreases)
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_AddMalicePoint(CDNUserBase* pUser, int iNpcID, int val);

	/*
desc : Request client to send selected NPC gift info to server.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_SendSelectedPresent(CDNUserBase* pUser, int iNpcID);
	
	/*
desc : Request client to send selected NPC gift info to server.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_Rage(CDNUserBase* pUser, int iNpcID);

	/*
desc : Request client to send selected NPC gift info to server.
param  : user, NpcID
return : -1 => user not found, -2 => system error, otherwise => return value
	*/
	int api_npc_Disappoint(CDNUserBase* pUser, int iNpcID);

	/*
desc   : Reset all skills of a specific user.
param  : user
return : -1 : user not found, 1 : success
	*/
	int api_user_ResetSkill(CDNUserBase* pUser);

	/*
desc : Repair user's equipped items and deduct repair cost.
param  : user, repair all flag (1: repair all, 0: equipped items)
return : -1 : user not found / 0 : no items to repair / 1 : insufficient gold / 2 : item repair complete
	*/
	int api_user_RepairItem(CDNUserBase* pUser, int iAllRepair);

	/*
desc : Instruct the user to open the enhancement transfer window.
param  : user
return : -1 => user not found, 1 => success
	*/
	int api_ui_OpenExchangeEnchant(CDNUserBase* pUser);

	/*
desc   : Check if the map is closed.
param  : user, map index
return : 1 => open, 0 => closed
	*/
	int api_env_CheckCloseGateByTime(int iMapID);

	/*
desc : Register remote quest completion step
param  : quest index, step index
	*/
	void api_quest_SetRemoteComplete (int nQuestID, int nStep);

	/*
desc : Check if world buff can be given
param  : buff item ID
	*/
	int api_quest_WorldBuffCheck(int nItemID);

	/*
desc : Give world buff
param  : buff item ID
	*/
	int api_quest_ApplyWorldBuff(CDNUserBase* pUser, int nItemID, int nMapIdx);

};
