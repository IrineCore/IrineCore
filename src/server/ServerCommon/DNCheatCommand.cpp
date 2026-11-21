#include "stdafx.h"
#include "DNCheatCommand.h"
#include "DNUserSendManager.h"
#include "DNUserQuest.h"
#include "DNGameDataManager.h"
#include "DNLogConnection.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#include "DNMissionSystem.h"
#include "DNFriend.h"
#include "DNIsolate.h"
#include <boost/format.hpp>
#include "DNQuestManager.h"
#include "DnScriptManager.h"
#include "DNGameDataManager.h"
#include "DNUserSession.h"
#include "DNMasterConnection.h"
#include "IniFile.h"
#include <boost/algorithm/string.hpp>
#include "DNGuildSystem.h"
#include "TimeSet.h"
#include "DNAppellation.h"
#include "DNMailSender.h"

#if defined(_VILLAGESERVER)

#if defined( PRE_PVP_GAMBLEROOM )
#include "DNPvPRoomManager.h"
#endif

#include "DNFieldDataManager.h"
#include "DNScriptAPI.h"
#include "DNPartyManager.h"
#include "DNUserSessionManager.h"
#include "DNField.h"
#include "DnSkillTreeSystem.h"
#include "DNAuthManager.h"
#include "DNCountryUnicodeSet.h"
#include "DNGuildVillage.h"
#include "DNLadderSystemManager.h"
#include "DNLadderRoom.h"
#include "DNGuildWarManager.h"
extern TVillageConfig g_Config;
#if defined(PRE_ADD_QUICK_PVP)
#include "DNUserSessionManager.h"
#endif //#if defined(PRE_ADD_QUICK_PVP)
#if defined(PRE_ADD_LIMITED_CASHITEM)
#include "DNCashConnection.h"
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM)
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriend.h"
#endif
#elif defined(_GAMESERVER)

#include "DNPvPGameRoom.h"
#include "PvPGameMode.h"
#include "DnActor.h"
#include "TaskManager.h"
#include "DnItemTask.h"
#include "DnDropItem.h"
#include "EtActionCoreMng.h"
#include "DNGameServerScriptAPI.h"
#include "DnGameTask.h"
#include "DnPvPGameTask.h"
#include "DNMasterConnectionManager.h"
#include "PvPRespawnLogic.h"
#include "DnPlayerActor.h"
#include "DnFarmGameTask.h"
#include "GrowingArea.h"
#include "MAAiScript.h"
#include "DNFarmGameRoom.h"
#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
#include "DnDPSReporter.h"
#endif

#endif

#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
#include "NpcReputationProcessor.h"
#include "ReputationSystemRepository.h"
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRepository.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#include "DNPeriodQuestSystem.h"

#if defined( PRE_ADD_STAMPSYSTEM )
#include "DNStampSystem.h"
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

extern bool LoadNpcQuest();

enum _CHEAT_CMT_TYPE
{
	HELP = 0,
	SET_USER_LEVEL,
	CLEAR_ALL_QUEST,
	MAKEITEM,
	RELOAD_ACTION,
	ADD_SKILL,
	REMOVE_SKILL,
	ADD_SKILL_LEVEL,
	MAKECOIN,
	CHANGEMAP,
	SET_COMPLETE_QUEST,
	SETQUEST,
	DEL_QUEST,
	SHOW_QUEST_INFO,
	ALL_KILL_MONSTER,
	GEN_MONSTER,
	ADD_STATEEFFECT,
	NOTICE,
	ADDINVENCOUNT,
	ADDWARECOUNT,
	REBIRTH,
	GEN_NPC,
	ENABLESHADOW,
	FATIGUE,
	IGNORECOOLTIME,
	FCM,
	SETFCM,
	RESPAWNLOGIC,
	GETPVPSCORE,
	SETCULLING,
	GOTOMAP,
	SET_HP,
	SET_MP,
	SET_HPMPFULL,
	GET_HP,
	GAINMISSION,
	ACHIEVEMISSION,
	FORCERANK,
	ADD_XP,
	GAINDAILYMISSION,
	ACHIEVEDAILYMISSION,
	STAGECLEAR,
	SET_ENCHANT,
	LUCKY,
	GEN_MONSTER_COMPOUND,
	MAKECITEM,
	SETCP,
	MAILREMAIN,
	FULLFRIEND,
	FULLBLOCK,
	RELOAD_SCRIPT,
	TRACE,
	RECALL,
	CHEATSET,
	SETSP,
	SET_USER_JOB,
	REPAIRALL,
	BUILDUPALL,
	POTENTIALALL,
	GUILD_CREATE,
	GUILD_DISMISS,
	GUILD_CHANGELEVEL,
	GUILD_MAKECOIN,
	GUILD_CHANGECMMPOINT,
	GUILD_CHANGEMISSIONPOINT, 
	GUILD_CHANGEWARPOINT,
	ACHIEVEGUILDCOMMONMISSION,
	RESET_SECONDAUTH,
	GAINWEEKLYMISSION,
	ACHIEVEWEEKLYMISSION,
	ACHIEVEGUILDMISSION,
	MAKE_GUILDFESTIVAL_POINT,
	MAKE_GUILDWAR_REWARD,
	FARM_START,
	IGNOREHIT,
	INVENCLEAR,
	GIFT,
	GETPOS,
	SETPOS,
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
	SETREPUTE,
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
	MASTERSYSTEM_MOD_GRADUATECOUNT,
	MASTERSYSTEM_SKIP_DATE,
	MASTERSYSTEM_MOD_FAVORPOINT,
	MASTERSYSTEM_MOD_RESPECTPOINT,
	FARM_PLANTSEED,
	FARM_DESTROYSEED,
	FARM_SKIPSEED,
#if defined( PRE_ADD_FARM_DOWNSCALE )
	FARM_ADDATTR,
	FARM_DELATTR,
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	RELOAD_MONSTER_AI,
	NESTCLEAR,
#if defined( PRE_ADD_SECONDARY_SKILL )
	ADD_SECONDARYSKILL,
	DEL_SECONDARYSKILL,
	ADD_SECONDARYSKILL_EXP,
	ADD_SECONDARYSKILL_RECIPE,
	ADD_SECONDARYSKILL_RECIPE_EXP,
	DEL_SECONDARYSKILL_RECIPE,
	EXTRACT_SECONDARYSKILL_RECIPE,
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	SET_PERIODQUEST,
	SET_WORLDQUEST,
	GUILD_CHANGEMEMBERCOUNT,
	SET_GUILDWAREVENT,
	SET_PREWINGUILDUID,
	SET_GUILDWAR_FINALTIME,
	SET_GUILDWARSCHEDULE_RELOAD,
	SET_TITLE,
	SET_LADDER_POINT,
	SET_LADDER_GRADEPOINT,
	SET_LADDER_HIDDENGRADEPOINT,
	GET_LADDER_POINT,
	FORCE_LADDER_MATCHING,
	SET_SECOND_JOB_SKILL,
	CLEAR_SECOND_JOB_SKILL,
	MAKE_UNION_POINT,
	CMDEQUIPTRIGGER,
	CMDUNRIDE,
	ITEMEXPIREDATE,
	UPDATE_PET_EXP,
	SET_PET_COLOR,
	PERIODRESET,
	MAKEGITEM,
	FARM_GUILDWAR_FINAL,
	DESTROYPVP,
	CLEAR_MISSION,
	CLEAR_PRESENT,
	SHOP_GETLIST_REPURCHASE,
	SHOP_REPURCHASE,
#if defined( PRE_ADD_QUICK_PVP)
	QUICKPVP,
#endif // #if defined( PRE_ADD_QUICK_PVP)
	PVPTIMEOVER,
	RESETNESTCOUNT,
	DEL_GUILDEFFECT,
	DEL_GUILDMARK,
	DENY_GUILDRECRUIT,
	CANCEL_GUILDAPPLY,
	RESET_GUILDPOINTLIMIT,
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	PVP_VILLAGE_JOIN,
	PVP_VILLAGE_CREATE,
	PVP_VILLAGE_LADDER,
#endif
#if defined(PRE_ADD_EXPORT_DPS_INFORMATION)
	ENABLE_DPS_REPORT,
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	GOLDBOX_NAMEDITEM,
	NAMEDITEM_COUNT,
#endif
#if defined(PRE_ADD_LIMITED_CASHITEM)
	CHANGE_LIMITQUANTITY,
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM)
#if defined (PRE_ADD_BESTFRIEND)
	CLOSE_BESTFRIEND,
#endif
#if defined( PRE_ADD_RACING_MODE)
	RACING_START,
	RACING_END,
#endif //#if defined( PRE_ADD_RACING_MODE)
#if defined( PRE_ADD_PCBANG_RENTAL_ITEM)
	SETPCCAFERENTITEM,
#endif
#ifdef PRE_MOD_PVPRANK
	RANK,
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
	PVP_RANK,
	PVP_LADDER_RANK,
#endif
#if defined(PRE_SPECIALBOX)
	SPECIALBOX,
#endif	// #if defined(PRE_SPECIALBOX)
#if defined(PRE_ADD_REMOTE_QUEST)
	GAIN_REMOTE_QUEST,
	ACCEPT_REMOTE_QUEST,
	COMPLETE_REMOTE_QUEST,
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
	SET_STAGE_CLEAR_BOXNUM,
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
	ISGAINMISSION,
#if defined( PRE_ADD_STAMPSYSTEM )
	ADD_STAMP,
	CLEAR_STAMP,
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_ADD_ACTIVEMISSION)
	SET_ACTIVEMISSION,
#endif
#if defined(PRE_ADD_MONTHLY_MISSION)
	GAIN_MONTHLY_MISSION,
	ACHIEVE_MONTHLY_MISSION,
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
#if defined(PRE_ADD_CP_RANK)
	INIT_STAGECP,
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	CREATE_GAMBLEROOM,
#endif
	CMD_END,
	
};

__CmdLineDefineStruct g_CmdList[] = 
{
	{ HELP, _T("/help"), _T("Help"), _T("Help") },
	{ SET_USER_LEVEL, _T("/level"), _T("Set user level /level(1~255)"),  _T("Set User Level /level(1~255)")},
	{ CLEAR_ALL_QUEST, _T("/clearquest"), _T("Clear all quest info.( including completed )"), _T("Clear all quest.( Including info completed )") },
	{ MAKEITEM, _T("/makeitem"), _T("/makeitem(ItemIndex)(ItemCount)(Enchant)(Potential)(Duration if Cash Item)"), _T("/makeitem(ItemIndex)(Count)(Enchant)(Potential)(Cash Period)") },
	{ RELOAD_ACTION, _T("/reloadaction"), _T("Reload action files"), _T("Reload action file")},
	{ ADD_SKILL, _T("/addskill"), _T("Add Skill"), _T("Add Skill") },
	{ REMOVE_SKILL, _T("/removeskill"), _T("Remove Skill"), _T("Remove Skill") },
	{ ADD_SKILL_LEVEL, _T("/addskilllevel"), _T("add Skill level"), _T("Add Skill level") },
	{ MAKECOIN, _T("/makecoin"), _T("/makecoin(Amount)"), _T("/makecoin(Amount)") },
	{ CHANGEMAP, _T("/changemap"), _T("/changemap(TargetMapID)(Difficulty:0~4:Default(0))(StartGateIndex:Default(1)"), _T("Change your position by map /changemap(MapId)(Difficulty:0~4:Default(0))(StartGate Index:Default(1)") },
	{ SET_COMPLETE_QUEST, _T("/completequest"), _T("Set quest completion info /completequest QuestID MarkFlag(1:Mark/Other:No) DeleteFlag(0:No/Other:Delete) RepeatFlag(0:No/Other:Repeat)"), _T("Set questinfo completed /completequest QuestNumber Marking?(1:Marking/Else:No) Delete?(0:No/Else:Delete) Repeat?(0:No/Else:Repeat)") },
	{ SETQUEST, _T("/setquest"), _T("/setquest(QuestID)(QuestType)(step)(journal)"), _T("/setquest(QuestNumber)(Type)(Step)(Journal)") },
	{ DEL_QUEST, _T("/delquest"), _T("/delquest(QuestID)"), _T("/delquest(QuestNumber)") },
	{ SHOW_QUEST_INFO, _T("/showquest"), _T("/showquest Display quest info"), _T("/showquest Show all information about quest") },
	{ ALL_KILL_MONSTER, _T("/killmon"), _T("/killmon Kill all monsters."), _T("/killmon Kill all monsters on your map.") },
	{ GEN_MONSTER, _T("/genmon"), _T("/genmon(MonsterID)(Count) Spawn monsters "), _T("/genmon(MonsterNumber)(Count) Create Monsters ") },
	{ ADD_STATEEFFECT, _T("/addse"), _T("/addse(StateEffectIndex)(Duration(ms))(Arg)"), _T("/addse(StateEffect Index)(Duration(ms))(Factor)") },
	{ NOTICE, _T("/notice"), _T("/notice(NoticeText)"), _T("/notice(Sentance)") },
	{ ADDINVENCOUNT, _T("/invencount"), _T("/invencount(MaxInventoryCountToSet max:50)"), _T("/invencount(Max InvenCount  max:50)") },
	{ ADDWARECOUNT, _T("/warecount"), _T("/warecount(MaxWarehouseCountToSet max:50)"), _T("/warecount(Max WareCount max:50)") },
	{ REBIRTH, _T("/rebirth"), _T("/rebirth(ResurrectionStone)(PCBangResurrectionStone)(cash ResurrectionStone)"), _T("/rebirth(Normal)(PCRoom)(Cash)") },
	{ GEN_NPC, _T("/gennpc"), _T("/gennpc(npcIndex)"), _T("/gennpc(NpcIndex)") },
	{ ENABLESHADOW, _T("/enableshadow"), _T("Shadow test On/Off"), _T("Testing Shadow On/Off") },
	{ FATIGUE, _T("/fatigue"), _T("/fatigue(Daily max)(Weekly max)(PCBang max)"), _T("/fatigue(Max Daily Value)(Max Weekly Value)(Max PCRoom Value)") },
	{ IGNORECOOLTIME, _T("/ignorect"), _T("(Ignore skill cooldown)"), _T("(Ignore Skill CoolTime)") },
	{ FCM, _T("/fcm"), _T("/fcm"), _T("/fcm") },
	{ SETFCM, _T("/setfcm"), _T("/setfcm(onlinemin)"), _T("/setfcm(onlinemin)") },
	{ RESPAWNLOGIC, _T("/respawnlogic"), _T("/respawnlogic(Enable/Disable respawn point cheat.)"), _T("/respawnlogic(Activate cheat for respawn point or not)") },
	{ GETPVPSCORE, _T("/getpvpscore"), _T("/getpvpscore(View your own PvP record.)"), _T("/getpvpscore(Show my PvP's score)") },
	{ SETCULLING, _T("/setculling"), _T("/setculling(Adjust user sync time, distance)"), _T("/setculling(Sync time between users, distance)") },
	{ GOTOMAP, _T("/go"), _T("/go(TargetMapID)(Difficulty:0~4:Default(0))(StartGateIndex:Default(1)"), _T("/go(MapId)(Difficulty:0~4:Default(0))(StartGate Index:Default(1)") },
	{ SET_HP, _T("/hp"), _T("Change HP"), _T("Change HP") },
	{ SET_MP, _T("/mp"), _T("Change MP"), _T("Change MP") },
	{ SET_HPMPFULL, _T("/full"), _T("HP,MP Full"), _T("HP,MP Full") },
	{ GET_HP, _T("/gethp"), _T("Check HP"), _T("Check HP") },
	{ GAINMISSION, _T("/missiongain"), _T("Acquire mission"), _T("Acquire Mission") },
	{ ACHIEVEMISSION, _T("/missionachieve"), _T("Complete mission"), _T("Achieve Mission") },
	{ FORCERANK, _T("/forcerank"), _T("Force change dungeon clear rank"), _T("Force to change rank of dungeon clear") },
	{ ADD_XP, _T("/add_xp"), _T("PVP XP Cheat"), _T("Cheat PvP's xp") },
	{ GAINDAILYMISSION, _T("/dmissiongain"), _T("Acquire daily mission"), _T("Acquire Daily Mission") },
	{ ACHIEVEDAILYMISSION, _T("/dmissionachieve"), _T("Complete daily mission"), _T("Achieve Daily Mission") },
	{ STAGECLEAR, _T("/clear"), _T("Force stage clear test"), _T("Force to clear Stage") },
	{ SET_ENCHANT, _T("/enchant"), _T("Enchant item /enchant(InvenIndex)(EnchantLevel)(PotentialValue)"), _T("Item Enchant /enchant(InvenIndex)(Level)(Potential Value)") },
	{ LUCKY, _T("/lucky"), _T("100% rate for enchant, craft, crest, etc."), _T("Success to enchance any items") },
	{ GEN_MONSTER_COMPOUND, _T("/gencmon"), _T("Spawn monster(Refer to CompoundTable)"), _T("Create Monster(Reference:CompoundTable") },
	{ MAKECITEM, _T("/makecitem"), _T("/makecitem(ItemCompoundIndex)(ItemCount)"), _T("/makecitem(ItemTableIndex)(Count)") },
	{ SETCP, _T("/cp"), _T("Set CP value."), _T("Set cp value") },
	{ MAILREMAIN, _T("/mailremain"), _T("/mailremain(RemainingTime)"), _T("/mailremain(left time)") },
	{ FULLFRIEND, _T("/fullfriend"), _T("Fill friend list"), _T("Fullfill friend list") },
	{ FULLBLOCK, _T("/fullblock"), _T("Fill block list"), _T("Fullfill block list") },
	{ RELOAD_SCRIPT, _T("/reloadscript"), _T("Reload script files"), _T("Reload script file") },
	{ TRACE, _T("/trace"), _T("Usage(/trace CharName), Desc(Intrudes into the dungeon of the specified character.)"), _T("Usage(/trace CharacterName), Description(Trace character's dungeon)") },
	{ RECALL, _T("/recall"), _T("Usage(/recall CharName), Desc(Summons the specified character to GM Village.)"), _T("Usage(/recall CharacterName), Description(Recall character to GM Village") },
	{ CHEATSET, _T("/cheatset"), _T("Usage(/cheatset SetName), Desc(Executes the cheat set with the specified name.)"), _T("Usage(/cheatset SetName),Description(Excute CheatSet related to SetName you typed") },
	{ SETSP, _T("/setsp"), _T("Usage(/setsp SkillPoints), Desc(Set current character's skill points to the entered value."), _T("Usage(/setsp SkillPoint), Description(Set your character's skillpoint") },
	{ SET_USER_JOB, _T("/setjob"), _T("Usage(/setjob JobIndex), Desc(Changes current character's job to the specified job index.)"), _T("Usage(/setjob JobIndex), Description(Change your character's job") },
	{ REPAIRALL, _T("/repairall"), _T("Usage(/repairall), Desc(Repairs all items.)"), _T("Usage(/repairall), Description(Repaire all items your character has)") },
	{ BUILDUPALL, _T("/buildupall"), _T("Usage(/buildupall(EnhanceLevel 0-15)), Desc(Enhances all equipped and inventory items)"), _T("Usage(/buildupall(Enchant Level 0-15)), Description(Upgrade all Items' enchant:Equip, Inven)") },
	{ POTENTIALALL, _T("/potentialall"), _T("Usage(/potentialall), Desc(Applies potential to equipped items)"), _T("Usage(/potentialall), Description(Apply potential of EquipItem)") },
	{ GUILD_CREATE, _T("/gcreate"), _T("Usage(/gcreate GuildName), Desc(Creates a guild)"), _T("Usage(/gcreate GuildName), Description(Create Guild)") },
	{ GUILD_DISMISS, _T("/gdismiss"), _T("Usage(/gdismiss), Desc(Disbands the guild)"), _T("Usage(/gdismiss), Description(Dismiss Guild)") },
	{ GUILD_CHANGELEVEL, _T("/glevel"), _T("Usage(/glevel GuildLevel(1~MaxLevel)), Desc(Sets guild level)"), _T("Usage(/glevel GuildLevel(1~20)), Description(Set Guild Level)") },
	{ GUILD_MAKECOIN, _T("/gcoin"), _T("Usage(/gcoin GuildFunds(0~MaxFunds)), Desc(Increases guild funds)"), _T("Usage(/gcoin GuildCoin(0~MaxCoin)), Description(Increase Guild Coin)") },
	{ GUILD_CHANGECMMPOINT, _T("/gcommon"), _T("Usage(/gcommon GuildCommonPoint(0~MaxCommonPoint)), Desc(Increases guild common points)"), _T("Usage(/gcommon GuildPoint(0~MaxPoint)), Description(Increase Guild Point)") },
	{ GUILD_CHANGEMISSIONPOINT, _T("/gmission"), _T("Usage(/gmission GuildMissionPoint(0~MaxCommonPoint) MissionID), Desc(Increases guild mission points)"), _T("Usage(/gmission GuildPoint(0~MaxPoint), MissionID), Description(Increase Guild Point)") },
	{ GUILD_CHANGEWARPOINT, _T("/gwar"), _T("Usage(/gwar GuildWarPoint(0~MaxCommonPoint)), Desc(Increases guild war points)"), _T("Usage(/gwar GuildPoint(0~MaxPoint)), Description(Increase Guild Point)") },
	{ ACHIEVEGUILDCOMMONMISSION, _T("/gcommonachieve"), _T("Complete guild normal mission"), _T("Achieve Guild Common Mission") },
	{ RESET_SECONDAUTH, _T("/resetsa"), _T("Usage(/resetsa), Desc(Resets secondary auth info)"), _T("Usage(/resetsa), Description(Reset second authentication's info)") },
	{ GAINWEEKLYMISSION, _T("/wmissiongain"), _T("Acquire weekly mission"), _T("Acquire Weekly Mission") },
	{ ACHIEVEWEEKLYMISSION, _T("/wmissionachieve"), _T("Complete weekly mission"), _T("Achieve Weekly Mission") },
	{ ACHIEVEGUILDMISSION, _T("/gmissionachieve"), _T("Complete guild war mission"), _T("Achieve GuildWar Mission") },
	{ MAKE_GUILDFESTIVAL_POINT, _T("/makefestivalpoint"), _T("Usage(/makefestivalpoint(FestivalPoint))"), _T("Usage(/makefestivalpoint(Value))") },
	{ MAKE_GUILDWAR_REWARD, _T("/guildwarreward"), _T("Usage(/guildwarreward(Finals round? Winner:1, 2nd:2, Top4:3, Top8:4, Top16:5))"), _T("Usage(/gulldwarreward(GuildWar Reward)") },
	{ FARM_START, _T("/gofarm"), _T("Usage(/gofarm), Desc(Only usable in Village)"), _T("Usage(/gofarm), Description(Only use in village)") },
	{ IGNOREHIT, _T("/ignorehit"), _T("(Ignore hit)"), _T("(Ingore hit)") },
	{ INVENCLEAR, _T("/invenclear"), _T("Usage(/invenclear(0:NormalInven 1:CashInven 2:QuestInven))"), _T("Usage(/invenclear(0:NormalInven 1:CashInven 2:QuestInven)") },
	{ GIFT, _T("/gift"), _T("Usage(/gift(ItemSN:ProductSN))"), _T("Usage(/gift(ItemSN))") },
	{ GETPOS, _T("/getpos"), _T("Usage(/getpos)"), _T("Usage(/getpos)") },
	{ SETPOS, _T("/setpos"), _T("Usage(/setpos x, y, z)"), _T("Usage(/setpos x, y, z)") },
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
	{ SETREPUTE, _T("/setrepute"), _T("Usage(/setrepute(NpcID), (FavorValue), (DisfavorValue))"), _T("Usage(/setrepute(NpcID), (ReputationValue:Like), (Dislike)") },
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
	{ MASTERSYSTEM_MOD_GRADUATECOUNT, _T("/modgraduatecount"), _T("Usage(/modgraduatecount(Value))"), _T("Usage(/modgraduatecount(Value)") },
	{ MASTERSYSTEM_SKIP_DATE, _T("/skipmasterdate"), _T("Usage(/skipmasterdate(1or0))"), _T("Usage(/skipmasterdate(1or0)") },
	{ MASTERSYSTEM_MOD_FAVORPOINT, _T("/modmasterfavorpoint"), _T("Usage(/modmasterfavorpoint(PupilName)(Value))"), _T("Usage(/modmasterfavorpoint(ChildName)(Value)") },
	{ MASTERSYSTEM_MOD_RESPECTPOINT, _T("/modmasterrespectpoint"), _T("Usage(/modmasterrespectpoint(Value))"), _T("Usage(/modmasterrespectpoint(Value)") },
	{ FARM_PLANTSEED, _T("/plantseed"), _T("Usage(/plantseed(AreaIndex)(ActionType)(SeedInvenIndex))"), _T("Usage(/plantseed(AreaIndex)(ActionType)(SeedInvenIndex)") },
	{ FARM_DESTROYSEED, _T("/destroyseed"), _T("Usage(/destroyseed(AreaIndex))"), _T("Usage(/destroyseed(AreaIndex)") },
	{ FARM_SKIPSEED, _T("/skipseed"), _T("Usage(/skipseed(AreaIndex))"), _T("Usage(/skipseed(AreaIndex)") },
#if defined( PRE_ADD_FARM_DOWNSCALE )
	{ FARM_ADDATTR, _T("/farmaddattr"), _T("Usage(/farmaddattr(attr))"), _T("Usage(/farmaddattr(attr)") },
	{ FARM_DELATTR, _T("/farmdelattr"), _T("Usage(/farmdelattr(attr))"), _T("Usage(/farmdelattr(attr)") },
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	{ RELOAD_MONSTER_AI, _T("/reloadmonsterai"), _T("Reload monster AI"), _T("Reload MonsterAI") },
	{ NESTCLEAR, _T("/nest"), _T("Usage(/nest(NestMapID))"), _T("Usage(/nest(NestMapID)") },
#if defined( PRE_ADD_SECONDARY_SKILL )
	{ ADD_SECONDARYSKILL, _T("/add2ndskill"), _T("Usage(/add2ndskill(SkillID))"), _T("Usage(/add2ndskill(SkillID)") },
	{ DEL_SECONDARYSKILL, _T("/del2ndskill"), _T("Usage(/del2ndskill(SkillID))"), _T("Usage(/del2ndskill(SkillID)") },
	{ ADD_SECONDARYSKILL_EXP, _T("/add2ndskillexp"), _T("Usage(/add2ndskillexp(SkillID)(addexp))"), _T("Usage(/add2ndskillexp(SkillID)(addexp)") },
	{ ADD_SECONDARYSKILL_RECIPE, _T("/add2ndskillrecipe"), _T("Usage(/add2ndskillrecipe(SkillID)(recipeinvenindex))"), _T("Usage(/add2ndskillrecipe(SkillID)(recipeinvenindex)") },
	{ ADD_SECONDARYSKILL_RECIPE_EXP, _T("/add2ndskillrecipeexp"), _T("Usage(/add2ndskillrecipeexp(SkillID)(recipeItemID)(addexp))"), _T("Usage(/add2ndskillrecipeexp(SkillID)(recipeItemID)(addexp)") },
	{ DEL_SECONDARYSKILL_RECIPE, _T("/del2ndskillrecipe"), _T("Usage(/del2ndskillrecipe(SkillID)(recipeItemID))"), _T("Usage(/del2ndskillrecipe(SkillID)(recipeItemID)") },
	{ EXTRACT_SECONDARYSKILL_RECIPE, _T("/extract2ndskillrecipe"), _T("Usage(/extract2ndskillrecipe(SkillID)(recipeItemID))"), _T("Usage(/extract2ndskillrecipe(SkillID)(recipeItemID)") },
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	{ SET_PERIODQUEST, _T("/setperiodquest"), _T("Usage(/setperiodquest(nScheduleID)(1:Active 0:Inactive)(EventDate yyyymmddhhmm:yyyymmddhhmm))"), _T("Usage(/setperiodquest(ScheduleID)(Activate:1 or 0)(EventDate yyyymmddhhmm:yyyymmddhhmm)") },
	{ SET_WORLDQUEST, _T("/setworldquest"), _T("Usage(/setworldquest(nScheduleID)(CollectStart:yyyymmddhhmm)(CollectEnd:yyyymmddhhmm)(RewardStart:yyyymmddhhmm))"), _T("Usage(/setworldquest(ScheduleID)(CollectStartDay:yyyymmddhhmm)(CollectEndDay:yyyymmddhhmm)(RewardStartDay:yyyymmddhhmm)") },
	{ GUILD_CHANGEMEMBERCOUNT, _T("/gmember"), _T("Usage(/gmember(MemberCount))"), _T("Usage(/gmember(GuildMemberCount)") },
	{ SET_GUILDWAREVENT, _T("/setguildwarevent"), _T("Usage(/setguildwarevent(GuildWarRound)(StartTime yyyymmddhhmm)(EventTime:min)(PrepTime:min))"), _T("Usage(/setguildwarevent(ScheduleID)(StartDate yyyymmddhhmm)(EventDuration:Minute)(ReadyDuration:Minute)")},
	{ SET_PREWINGUILDUID, _T("/setprewinguild"), _T("Usage(/setprewinguild)"), _T("Usage(/setprewinguild") },
	{ SET_GUILDWAR_FINALTIME, _T("/setguildwarfinaltime"), _T("Usage(/setguildwarfinaltime(+sec))"), _T("Usage(/setguildwarfinaltime(+sec)") },
	{ SET_GUILDWARSCHEDULE_RELOAD, _T("/setguildwarreload"), _T("Reload guild war schedule"), _T("GuildWar Reload")},
	{ SET_TITLE, _T("/settitle"), _T("Usage(/settitle(Index))"), _T("Usage(/settitle(Index)") },
	{ SET_LADDER_POINT, _T("/setladderpoint"), _T("Usage(/setladderpoint(1:1=1,2:2=2...etc)(LadderPoint))"), _T("Usage(/setladderpoint(1:1=1,2:2=2...Etc)(LadderPoint)") },
	{ SET_LADDER_GRADEPOINT, _T("/setladdergradepoint"), _T("Usage(/setladdergradepoint(1:1=1,2:2=2...etc)(GradePoint))"), _T("Usage(/setladdergradepoint(1:1=1,2:2=2...Etc)(Rating)") },
	{ SET_LADDER_HIDDENGRADEPOINT, _T("/setladderhiddengradepoint"), _T("Usage(/setladderhiddengradepoint(1:1=1,2:2=2...etc)(HiddenGradePoint))"), _T("Usage(/setladderhiddengradepoint(1:1=1,2:2=2...Etc)(Pre Rating)") },
	{ GET_LADDER_POINT, _T("/getladderpoint"), _T("Usage(/getladderpoint(1:1=1,2:2=2...etc))"), _T("Usage(/getladderpoint(1:1=1,2:2=2...Etc)") },
	{ FORCE_LADDER_MATCHING, _T("/forcematching"), _T("Usage(/forcematching)"), _T("Usage(/forcematching") },
	{ SET_SECOND_JOB_SKILL, _T("/setsecondjobskill"), _T("Usage(/setsecondjobskill(2ndJob Job ID))"), _T("Usage(/setsecondjobskill(Second Ex-Job ID)") },
	{ CLEAR_SECOND_JOB_SKILL, _T("/clearsecondjobskill"), _T("Usage(/clearsecondjobskill)"), _T("Usage(/clearsecondjobskill") },
	{ MAKE_UNION_POINT, _T("/makeunionpoint"), _T("Usage(/makeunionpoint(UnionType)(UnionPoint))"), _T("Usage(/makeunionpoint(UnionType)(Point)") },
	{ CMDEQUIPTRIGGER, _T("/cmdtriggerequip"), _T("Usage(/cmdtriggerequip(ItemID)(shootmode))"), _T("Usage(/cmdtriggerequip(ItemID)(shootmode)") },
	{ CMDUNRIDE, _T("/cmdunride"), _T("Usage(/cmdunride)") },
	{ ITEMEXPIREDATE, _T("/itemexpiredate"), _T("Usage(/itemexpiredate(ItemSerial))"), _T("Usage(/itemexpiredate(ItemSerial))") },
	{ UPDATE_PET_EXP, _T("/updatepetexp"), _T("Usage(/UpdatePetExp(Exp))"), _T("Usage(/UpdatePetExp(Exp)") },
	{ SET_PET_COLOR, _T("/setpetcolor"), _T("Usage(/SetPetColor(Part 1:body 2:tatoo)(ColorValue))"), _T("Usage(/SetPetColor(type 1 or 2)(color)") },
	{ PERIODRESET, _T("/periodreset"), _T("Usage(/periodreset(Type))"), _T("Usage(/periodreset(Type)(Value)") },
	{ MAKEGITEM, _T("/makegitem"), _T("Usage(/makegitem(itemgrouptableid))"), _T("Usage(/makegitem(itemgrouptableid)") },
	{ FARM_GUILDWAR_FINAL, _T("/farmguildwarfinal"), _T("Usage(/farmguildwarfinal)"), _T("Usage(/farmguildwarfinal") },
	{ DESTROYPVP, _T("/destroypvp"), _T("Usage(/destroypvp(PvPIndex)(ForceWinGuildName))"), _T("Usage(/destroypvp(PvP RoomNumber)(Win GuildName)") },	
	{ CLEAR_MISSION, _T("/clearmission"), _T("Usage(/clearmission)"), _T("Usage(/clearmission)") },	
	{ CLEAR_PRESENT, _T("/cleargift"), _T("Usage(/cleargift)"), _T("Usage(/cleargift)") },	
	{ SHOP_GETLIST_REPURCHASE, _T("/getlistrepurchase"), _T("Usage(/getlistrepurchase)"), _T("Usage(/getlistrepurchase)") },	
	{ SHOP_REPURCHASE, _T("/repurchase"), _T("Usage(/repurchase repurchaseid)"), _T("Usage(/repurchase repurchaseid)") },	
#if defined( PRE_ADD_QUICK_PVP)
	{ QUICKPVP, _T("/quickpvp"), _T("Usage(/quickpvp (OpponentCharName))"), _T("Usage(/quickpvp (Other CharacterName))") },
#endif
	{ PVPTIMEOVER, _T("/pvptimeover"), _T("Usage(/pvptimeover)"), _T("Usage(/pvptimeover)") },
	{ RESETNESTCOUNT, _T("/resetnestcount"), _T("Usage(/resetnestcount)"), _T("Usage(/resetnestcount)") },
	{ DEL_GUILDEFFECT, _T("/grewardreset"), _T("Usage(/grewardreset(GuildRewardID))"), _T("Usage(/grewardreset)(guildrewarditemID)") },	// Remove guild reward effect (only for timed effects)
	{ DEL_GUILDMARK, _T("/gmarkreset"), _T("Usage(/gmarkreset)"), _T("Usage(/gmarkreset)") },												// Reset guild mark
	{ DENY_GUILDRECRUIT, _T("/gwantedreset"), _T("Usage(/gwantedreset)"), _T("Usage(/gwantedreset)") },									// Delete info of characters who applied to join own guild
	{ CANCEL_GUILDAPPLY, _T("/gapplicationreset"), _T("Usage(/gapplicationreset)"), _T("Usage(/gapplicationreset)") },						// Delete own guild join application info
	{ RESET_GUILDPOINTLIMIT, _T("/resetglimit"), _T("Usage(/resetglimit), Desc(Resets guild EXP daily limit)"), _T("Usage(/resetglimit), Description(Reset Guild DailyExp Limit)") },
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	{ PVP_VILLAGE_JOIN, _T("/pvpjoin"), _T("Usage(/pvpjoin), Desc(Join a PvP room from village)"), _T("Usage(/pvpjoin), Description(Go pvp Room)") },
	{ PVP_VILLAGE_CREATE, _T("/pvpmake"), _T("Usage(/pvpmake), Desc(Enter a PvP room from village)"), _T("Usage(/pvpmake), Description(Make pvp Room)") },
	{ PVP_VILLAGE_LADDER, _T("/pvpladder"), _T("Usage(/pvpladder), Desc(Connect to ladder from village)"), _T("Usage(/pvpladder), Description(Go pvp ladder)") },	
#endif
#if defined(PRE_ADD_EXPORT_DPS_INFORMATION)
	{ ENABLE_DPS_REPORT, _T("/reportdps"), _T("Usage(/reportdps), Desc( Record DPS measurement )"), _T("Usage(/reportdps), Description( USE DPS Report )") },
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	{ GOLDBOX_NAMEDITEM, _T("/goldboxitem"), _T("Usage(/goldboxitem (ItemID)), Desc( Specify gold box item )"), _T("Usage(/goldboxitem), Description( goldboxitem change )") },
	{ NAMEDITEM_COUNT, _T("/nameditemcount"), _T("Usage(/nameditemcount (ItemID)), Desc( Check named item count )"), _T("Usage(/nameditemcount), Description( nameditem count )") },
#endif
#if defined(PRE_ADD_LIMITED_CASHITEM)
	{ CHANGE_LIMITQUANTITY, _T("/limititemmax"), _T("Usage(/limititemmax (ItemSerialNumber)(NewCount)), Desc( Change limited item count )"), _T("Usage(/limititemmax), Description( limitation quantity change )") },
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM)
#if defined (PRE_ADD_BESTFRIEND)
	{ CLOSE_BESTFRIEND, _T("/closebf"), _T("Usage(/closebf), Desc( Delete best friend immediately )"), _T("Usage(/closebf), Description( remove bestfriend quickly )") },
#endif
#if defined(PRE_ADD_RACING_MODE)
	{ RACING_START, _T("/startracing"), _T("Usage(/startracing), Desc(PVP Racing Start)"), _T("Usage(/startracing), Description( PVP racing start )") },
	{ RACING_END, _T("/endracing"), _T("Usage(/startracing(index), Desc(PVP Racing End)"), _T("Usage(/endracing(index), Description(  PVP racing end )") },
#endif //#if defined(PRE_ADD_RACING_MODE)	
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
	{ SETPCCAFERENTITEM, _T("/setpccafeitem"), _T("Usage(/setpccafeitem, Desc(Give PCBang item)"), _T("Usage(/setpccafeitem), Description( PCCafe Rent Item )") },	
#endif
#ifdef PRE_MOD_PVPRANK
	{ RANK, _T("/pvprank"), _T("Recalculate PvPRank"), _T("Cheat ReCalc PvPRank") },
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
	{ PVP_RANK, _T("/rank"), _T("Re-aggregate PvPRank"), _T("Cheat Aggregate PvPRank") },
	{ PVP_LADDER_RANK, _T("/ladderrank"), _T("Re-aggregate PvPLadderRank"), _T("Cheat Aggregate PvPLadderRank") },
#endif
#if defined(PRE_SPECIALBOX)
	{ SPECIALBOX, _T("/specialbox"), _T("specialbox (mailid)"), _T("specialbox (mailid)") },
#endif	// #if defined(PRE_SPECIALBOX)
#if defined(PRE_ADD_REMOTE_QUEST)
	{ GAIN_REMOTE_QUEST, _T("/gain_rq"), _T("Acquire remote quest"), _T("Cheat Aggregate PvPRank") },
	{ ACCEPT_REMOTE_QUEST, _T("/accept_rq"), _T("Accept remote quest"), _T("Cheat Aggregate PvPRank") },	
	{ COMPLETE_REMOTE_QUEST, _T("/complete_rq"), _T("Complete remote quest"), _T("Cheat Aggregate PvPRank") },
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
	{ SET_STAGE_CLEAR_BOXNUM, _T("/setboxnum"), _T("Set stage clear reward number"), _T("setboxnum groupnum")},
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
	{ ISGAINMISSION, _T("/missiongain?"), _T("Mission acquired?"), _T("Acquire Mission?") },
#if defined( PRE_ADD_STAMPSYSTEM )
	{ ADD_STAMP, _T("/addstamp"), _T("Usage(/addstamp index(1~8) day(1~7))"), _T("Usage(/addstamp index(1~8) day(1~7))") },
	{ CLEAR_STAMP, _T("/clearstamp"), _T("Usage(/clearstamp)"), _T("Usage(/clearstamp)") },
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_ADD_ACTIVEMISSION)
	{ SET_ACTIVEMISSION, _T("/activemission"), _T("Usage(/activemission (missionid)"), _T("Usage(/activemission (missionid))") },
#endif
#if defined(PRE_ADD_MONTHLY_MISSION)
	{ GAIN_MONTHLY_MISSION, _T("/mmissiongain"), _T("Acquire monthly mission"), _T("Acquire Monthly Mission)") },
	{ ACHIEVE_MONTHLY_MISSION, _T("/mmissionachieve"), _T("Achieve monthly mission"), _T("Achieve Monthly Mission") },
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
#if defined(PRE_ADD_CP_RANK)
	{ INIT_STAGECP, _T("/initcprank"), _T("Usage(/initcprank inittype(0-all,1-Legend,2-Monthly,3-Personal))"), _T("Usage(/initcprank inittype(0-all,1-Legend,2-Monthly,3-Personal))") },
#endif //INIT_STAGECP
#if defined( PRE_PVP_GAMBLEROOM )
	{ CREATE_GAMBLEROOM, _T("/creategambleroom"), _T("Usage(/creategambleroom RoomIndex Regulation GambleType Gambleprice "), _T("Usage(/creategambleroom RoomIndex Regulation GambleType Gambleprice") },
#endif

};

CDNCheatCommand::CDNCheatCommand(CDNUserSession *pUserSession)
: m_pSession(pUserSession)
{
	m_bLucky = false;
	BOOST_STATIC_ASSERT( CMD_END == _countof(g_CmdList) );
}

#if defined(_GAMESERVER) && defined(_SHADOW_TEST)
extern bool g_bEnableShadow[MAX_SESSION_COUNT];
#endif

CDNCheatCommand::~CDNCheatCommand()
{

}

std::wstring CDNCheatCommand::_GetUserName()
{
	return(m_pSession->GetCharacterName());
}

CDNUserQuest* CDNCheatCommand::_GetQuest()
{
	return m_pSession->GetQuest();
}

int CDNCheatCommand::Command(std::wstring& szCheat)
{
	if( szCheat.find(L"/") != 0 )
	{
		return -1;
	}

	int nResult = -1;
	std::vector<std::wstring> tokens;
	TokenizeW(szCheat, tokens, L" ");
	ToLowerW(tokens[0]);
	int nCount = sizeof(g_CmdList) / sizeof(__CmdLineDefineStruct);
	for( int i = 0 ; i < nCount; i++ )
	{
		//		if( szCheat.find(g_CmdList[i].szString) < szCheat.size() )
		if( wcscmp( tokens[0].c_str(), g_CmdList[i].szString ) == NULL )
		{
			m_pSession->SendChat(CHATTYPE_SYSTEM, (int)szCheat.size()*sizeof(WCHAR), (WCHAR*)_GetUserName().c_str(), (WCHAR*)szCheat.c_str() );
			bool bResult = OnCommand(i, tokens);
			if( bResult == false ) 
			{
				nResult = 2;
				break;
			}
			else 
			{
				nResult = 1;
				break;
			}
		}
	}

	std::wstring szResultMsg;
	switch( nResult )
	{
	case 2:
		{
			szResultMsg = L"Command param Error";
			m_pSession->SendChat(CHATTYPE_SYSTEM, (int)szResultMsg.size()*sizeof(WCHAR), (WCHAR*)_GetUserName().c_str(), (WCHAR*)szResultMsg.c_str() );

		}
		break;
	case -1:
		{
			//szResultMsg = L"Can't found Command.";
			//m_pSession->SendChat(CHATTYPE_SYSTEM, (int)szResultMsg.size()*sizeof(WCHAR), (WCHAR*)_GetUserName().c_str(), (WCHAR*)szResultMsg.c_str() );
		}
		break;
	case 1:
		{
			szResultMsg = L"OK.";
			m_pSession->SendChat(CHATTYPE_SYSTEM, (int)szResultMsg.size()*sizeof(WCHAR), (WCHAR*)_GetUserName().c_str(), (WCHAR*)szResultMsg.c_str() );
		}
		break;
	}

	return nResult;
}

bool CDNCheatCommand::OnCommand(int nCmdID, std::vector<std::wstring>& tokens)
{
	switch( nCmdID )
	{
	case HELP:
		{
			for( int i = 0 ; i < CMD_END ; i++ )
			{
				std::wstring szMsg;
				szMsg = FormatW( L"[%s] - %s", g_CmdList[i].szString, g_CmdList[i].szCommentEn);
				m_pSession->SendChat(CHATTYPE_SYSTEM, (int)szMsg.size()*sizeof(WCHAR), (WCHAR*)_GetUserName().c_str(), (WCHAR*)szMsg.c_str() );
			}
		}
		break;

	case SET_USER_LEVEL:
		{
			wstring wszString;

			if( tokens.size() < 2 ){
				wszString = FormatW(L"Please enter the level properly!!!\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

			int nLevel = _wtoi(tokens[1].c_str());
			float fPercent = ( tokens.size() >= 3 ) ?(float)( _wtof( tokens[2].c_str() )*0.01f) : 0.f;
			if( nLevel < 1 || nLevel > CHARLEVELMAX ){
				wszString = FormatW(L"Please enter the level properly!!!\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

			DWORD nExp = g_pDataManager->GetExp(m_pSession->GetUserJob(), nLevel);
			if( nLevel < CHARLEVELMAX ) {
				int nNextExp = g_pDataManager->GetExp( m_pSession->GetUserJob(), nLevel+1 );
				nExp += (int)(( nNextExp - nExp ) * fPercent );
			}

#if defined(_VILLAGESERVER)
			// Changed to just lower it first and then raise it back up.
			// Note: Doing this causes skill points to differ from client when using level cheat.
			// Client increases SP by the exact number of levels gained, but
			// On the server, cumulative skill points from level 1 up to the target level are added.
			m_pSession->SetExp(1, DBDNWorldDef::CharacterExpChangeCode::Cheat, 0, false);
			m_pSession->SetLevel(1, DBDNWorldDef::CharacterLevelChangeCode::Cheat, false);
			m_pSession->SetExp(nExp, DBDNWorldDef::CharacterExpChangeCode::Cheat, 0, true);	// Setting exp automatically updates the level as well

#elif defined(_GAMESERVER)
			if( m_pSession == NULL || m_pSession->GetPlayerActor() == NULL || !m_pSession->GetActorHandle() )
				break;
			if( m_pSession->GetLevel() > nLevel){
				wszString = FormatW(L"Cannot lower level outside of town (level up is possible)!!!\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			m_pSession->GetPlayerActor()->SetExperience( 1 );
			m_pSession->GetActorHandle()->SetLevel( 1 );
			m_pSession->GetPlayerActor()->SetExperience(nExp);		// Setting exp updates it automatically.
#endif
		}
		break;

	case CLEAR_ALL_QUEST:
		{
			CDNUserQuest* pQuest = _GetQuest();
			if(!pQuest)
				return false;

			pQuest->ClearAllQuestInfo();
			pQuest->SendAllQuestRefresh();
		}
		break;

	case MAKEITEM:
		{
			return m_pSession->GetGMCommand()->CmdMakeItem(tokens);
		}
		break;


	case ADD_SKILL:
		{
#if defined(_VILLAGESERVER)
			if( tokens.size() != 2 )	return false;

			// Add only skills that exist in the table data.
			int iSkillID = _ttoi(tokens[1].c_str());
			TSkillData* pSkillData = g_pDataManager->GetSkillData( iSkillID );
			if( NULL == pSkillData )
			{
				wstring wszString = FormatW(L"[SkillID %d] Skill ID does not exist!!\r\n", iSkillID );
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

			// Skip skills already owned.
			if( false == m_pSession->GetSkill()->FindSkill( iSkillID ) )
			{
				m_pSession->GetSkill()->AddSkill( iSkillID );

			}
			else
			{
				bool bUnLockLevelZeroSkill = false;
				for( int i = 0; i < SKILLMAX; ++i )
				{
					TSkill* pSkillData = &(m_pSession->GetSkillData()->SkillList[i]);
					if( 0 == pSkillData->nSkillID )
						break;

					if( pSkillData->nSkillID == iSkillID )
					{
						if( 0 < pSkillData->cSkillLevel )
						{
							wstring wszString = FormatW(L"[SkillID %d] Skill is already owned.!!\r\n", iSkillID );
							m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
						}
						else
						{
							m_pSession->GetSkill()->CheatAcquireSkill( iSkillID );
							bUnLockLevelZeroSkill = true;
						}
 
						break;
					}
				}
			}
#elif defined(_GAMESERVER)
			wstring wszString = FormatW(L"Please use this in town!!\r\n");
			m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
#endif
		}
		break;

	case REMOVE_SKILL:
		{
#if defined(_VILLAGESERVER)
			if( tokens.size() != 2 )	return false;
			m_pSession->GetSkill()->DelSkill( _ttoi(tokens[1].c_str()) );

#elif defined(_GAMESERVER)
#endif
		}
		break;

	case ADD_SKILL_LEVEL:
#if defined(_VILLAGESERVER)
		if( tokens.size() != 3 )	return false;
		m_pSession->GetSkill()->SetSkillLevel( _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), DBDNWorldDef::SkillChangeCode::ModSkillLevelByAdmin );	// Deducted within skill points

#elif defined(_GAMESERVER)
#endif
		break;

	case MAKECOIN:
		if( tokens.size() != 2 ) return false;
		if(!m_pSession->AddCoin(_ttoi64(tokens[1].c_str()), DBDNWorldDef::CoinChangeCode::Cheat, 0)) return false;
		break;

	case CHANGEMAP:
	case GOTOMAP:
		{
			wstring wszString;

			if( tokens.size() < 2 ){
				wszString = FormatW(L"Please check the map ID. Current Map ID : [%d]\r\n", m_pSession->GetStatusData()->nMapIndex);
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

			if(_ttoi(tokens[1].c_str()) < 1){
				wszString = FormatW(L"Cannot use this in the tutorial map!!!!!\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

#if defined(_VILLAGESERVER)
			CDNField* pField = m_pSession->GetField();
			if( pField )
			{
				if( pField->bIsPvPVillage() || pField->bIsPvPLobby() || pField->bIsDarkLairVillage() )
				{
					wszString = FormatW(L"Cannot use /changemap cheat in PvP Town, PvP Lobby, or Dark Lair Town.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					return false;
				}
			}

#elif defined(_GAMESERVER)
			if( m_pSession->GetGameRoom() )
			{
				if( m_pSession->GetGameRoom()->bIsPvPRoom() )
				{
					wszString = FormatW(L"Cannot use /changemap cheat during PvP.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					return false;
				}

				if( m_pSession->bIsGMTrace() )
				{
					wszString = FormatW(L"Cannot use /changemap cheat during Trace.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					return false;
				}

				if(m_pSession->GetGameRoom()->bIsFarmRoom())
				{
					wszString = FormatW(L"Cannot use /changemap cheat in Farm.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					return false;
				}

				if( m_pSession->GetGameRoom()->GetRoomState() != _GAME_STATE_PLAY ) return false;
			}
#endif

			int nMapIndex = _ttoi(tokens[1].c_str());	// Target mapindex to move to
			TDUNGEONDIFFICULTY StageConstrucionLevel = ( tokens.size() <= 2 ) ? Dungeon::Difficulty::Easy : static_cast<TDUNGEONDIFFICULTY>(_ttoi(tokens[2].c_str()));
			if(StageConstrucionLevel >= Dungeon::Difficulty::Max) 
				return false;

			int nGateNo = ( tokens.size() <= 3 ) ? 1 : _ttoi(tokens[3].c_str());

			if( g_pDataManager->GetMapType( nMapIndex ) == GlobalEnum::MAP_UNKNOWN )
			{
				std::wstring wszLocalString;
				wszLocalString = FormatW( L"Cannot found map index %d ",  nMapIndex );
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszLocalString.size()*sizeof(WCHAR), L"", (WCHAR*)wszLocalString.c_str());
				return false;
			}

			if( g_pDataManager->GetMapSubType( nMapIndex ) == GlobalEnum::MAPSUB_FARM )
			{
				wszString = FormatW(L"Cannot use /changemap cheat to enter Farm.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
				return false;
			}

#if defined(_VILLAGESERVER)
			if(g_pDataManager->GetMapType(nMapIndex) != GlobalEnum::MAP_VILLAGE){		// In case of game server
				char cGateNo = g_pFieldDataManager->GetRandomGateNo(nMapIndex);

				if( m_pSession->GetPartyID() > 0 ) {
					CDNParty *pParty = g_pPartyManager->GetParty(m_pSession->GetPartyID());
					if(!pParty) return false;
					pParty->SetRandomSeed( timeGetTime() );
					pParty->SetAllMemberGate(nGateNo);

					if(g_pMasterConnection && g_pMasterConnection->GetActive())
						g_pMasterConnection->SendReqGameID( GameTaskType::Normal, REQINFO_TYPE_PARTY, pParty->GetPartyID(), m_pSession->GetChannelID(), pParty->GetRandomSeed(), nMapIndex, nGateNo, StageConstrucionLevel, true, pParty );		// Request GameID
					else
						return false;
				}
				else {
					m_pSession->m_cGateNo = nGateNo;
					m_pSession->m_nSingleRandomSeed = timeGetTime();

					if(g_pMasterConnection && g_pMasterConnection->GetActive())
						g_pMasterConnection->SendReqGameID( GameTaskType::Normal, REQINFO_TYPE_SINGLE, m_pSession->GetAccountDBID(), m_pSession->GetChannelID(), m_pSession->m_nSingleRandomSeed, nMapIndex, nGateNo, StageConstrucionLevel, true );	// Request GameID
					else
						return false;
				}
			}
			else
			{	// If village server
				char cGateNo = ( tokens.size() == 2 ) ? -1 : _ttoi(tokens[2].c_str());
				if( m_pSession->GetPartyID() > 0 ) {
					CDNParty *pParty = g_pPartyManager->GetParty(m_pSession->GetPartyID());
					if(!pParty) return false;
					if(g_pMasterConnection && g_pMasterConnection->GetActive())
						g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_PARTY, pParty->GetPartyID(), nMapIndex, cGateNo, -1, pParty);
					else
						return false;
				}
				else
				{
					if(g_pMasterConnection && g_pMasterConnection->GetActive())
						g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_SINGLE, m_pSession->GetAccountDBID(), nMapIndex, cGateNo);
					else
						return false;
				}
			}
#elif defined(_GAMESERVER)
			CDNGameRoom *pRoom = m_pSession->GetGameRoom();
			if (!pRoom) return false;
			if( pRoom->bIsPvPRoom() )	// Block changemap cheat in PvP rooms.
				return false;

			CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
			if( g_pDataManager->GetMapType( nMapIndex ) == GlobalEnum::eMapTypeEnum::MAP_VILLAGE ) {

				int nAttribute = g_pDataManager->GetChannelMapAtt( m_pSession->GetWorldSetID(), nMapIndex );
				
				if(nAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN)
				{
					wszString = FormatW(L"Cannot use /changemap cheat to enter Farm.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					return false;
				}							
						
				CDNUserSession *pLeaderSession = NULL;
				for( DWORD i=0; i<pRoom->GetUserCount(); i++ ) {
					CDNUserSession *pSession = pRoom->GetUserData(i);
					pSession->SetNextVillageData( NULL, 0, pTask->GetMapTableID(), nMapIndex, nGateNo );

					if( pRoom->GetPartyData(i)->bLeader ) pLeaderSession = pSession;
				}

				if( pLeaderSession ) {
					g_pMasterConnectionManager->SendRequestNextVillageInfo(pLeaderSession->GetWorldSetID(), nMapIndex, nGateNo, true, pRoom);
				}
			}
			else {
				for( DWORD i=0; i<pRoom->GetUserCount(); i++ )
				{
					CDNUserSession *pSession = pRoom->GetUserData(i);
					if( !pSession ) continue;
					DnActorHandle hActor = pSession->GetActorHandle();
					if( !hActor ) continue;
					CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
					pPlayer->CmdRemoveStateEffect(STATE_BLOW::BLOW_099);
				}

#if defined(PRE_FIX_INITSTATEANDSYNC)
				pRoom->ReserveInitStateAndSync( nMapIndex, nGateNo, CRandom::Seed(pRoom), StageConstrucionLevel, true );
#else
				pRoom->InitStateAndSync( nMapIndex, nGateNo, CRandom::Seed(pRoom), StageConstrucionLevel, true );
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
			}
#endif
		}
		break;

	case SET_COMPLETE_QUEST:
		{
			CDNUserQuest* pQuest = _GetQuest();
			if(!pQuest) return false;
			if( tokens.size() != 5 ) return false;
			int nQuestIndex = _wtoi(tokens[1].c_str());

			if(pQuest->HasQuest(nQuestIndex) == -2){	// If no quest exists
				int iRetVal = m_pSession->GetQuest()->AddQuest(nQuestIndex);
				if(1 != iRetVal) {
					g_Log.Log(LogType::_ERROR, m_pSession, L"SET_COMPLETE_QUEST Error : %d  AddQuest return %d" , nQuestIndex , iRetVal);
					break;
				}
			}

			int nFlag = _wtoi(tokens[2].c_str());
			if( nFlag < 0 || nFlag > 1 )
				return false;
			int bDelPlayList = _wtoi(tokens[3].c_str());
			int bRepeat = _wtoi(tokens[4].c_str());

			pQuest->RewardAfterCompletingQuest(nQuestIndex, ((bDelPlayList)?(true):(false)));
		}
		break;

	case SETQUEST:
		{
			if(tokens.size() <= 3) return false;

			int nQuestID = _ttoi(tokens[1].c_str());
			int nQuestType = _ttoi(tokens[2].c_str());

			int nStep = (tokens.size() >= 4) ? _ttoi(tokens[3].c_str()) : 1;
			if(nStep <= 0) nStep = 1;
			int nJournal = (tokens.size() >= 5) ? _ttoi(tokens[4].c_str()) : 1;
			if(nJournal <= 0) nJournal = 1;

			int iRetVal = m_pSession->GetQuest()->FindPlayingQuest(nQuestID);
			if(-1 >= iRetVal) {	// Index, not Quest ID, starts from 0
				iRetVal = m_pSession->GetQuest()->AddQuest(nQuestID);
				if(1 != iRetVal) {
					g_Log.Log(LogType::_ERROR, m_pSession, L"SETQUEST Error : %d  AddQuest return %d" , nQuestID , iRetVal);
					break;
				}
			}
			else {
#ifndef _FINAL_BUILD
				std::wstring wszMsg;
				wszMsg = FormatW( L"Only step and journal info will be changed for an already active quest. : %d" , nQuestID );
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
			}
			iRetVal = m_pSession->GetQuest()->SetQuestStep(nQuestID, nStep);
			if(1 != iRetVal) {
				g_Log.Log(LogType::_ERROR, m_pSession, L"SETQUEST Error : %d  SetQuestStep return %d" , nQuestID , iRetVal);
				break;
			}
			iRetVal = m_pSession->GetQuest()->SetJournalStep(nQuestID, nJournal);
			if(1 != iRetVal) {
				g_Log.Log(LogType::_ERROR, m_pSession, L"SETQUEST Error : %d  SetJournalStep return %d" , nQuestID , iRetVal);
				break;
			}
		}
		break;

	case DEL_QUEST:
		{
			return false;
		}
		break;

	case SHOW_QUEST_INFO:
		{
#if defined(_VILLAGESERVER)
			UINT nUserID =  m_pSession->GetObjectID();
			api_quest_DumpQuest(nUserID);
#elif defined(_GAMESERVER)
			UINT nUserID = m_pSession->GetSessionID();
			api_quest_DumpQuest(m_pSession->GetGameRoom(), nUserID);
#endif
		}
		break;

	case ALL_KILL_MONSTER:
		{
#if defined(_VILLAGESERVER)
			return false;
#elif defined(_GAMESERVER)

			CDNGameRoom *pRoom = m_pSession->GetGameRoom();
			if(!pRoom) return false;
			if(!pRoom->GetTaskMng()) return false;

			CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
			if(!pTask) return false;

			pTask->RequestDestroyAllMonster(false);
#endif
		}
		break;

	case GEN_MONSTER:
		{
#if defined(_VILLAGESERVER)
			return false;
#elif defined(_GAMESERVER)

			if( tokens.size() < 2 ) return false;

			int nMonsterID = _ttoi(tokens[1].c_str());
			int nCount = ( tokens.size() >= 3 ) ? _ttoi(tokens[2].c_str()) : 1;

			CDNGameRoom *pRoom = m_pSession->GetGameRoom();
			if(!pRoom) return false;
			if(!pRoom->GetTaskMng()) return false;

			CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
			if(!pTask) return false;

			EtVector3* pvPos = m_pSession->GetActorHandle()->GetPosition();

			for( int i  = 0 ; i < nCount ; i++ )
			{
				pTask->RequestGenerationMonsterFromMonsterID( nMonsterID, *pvPos, EtVector3( 0.f, 0.f, 0.f ), EtVector3( 0.f, 0.f, 0.f ));
			}
#endif
		}
		break;
	case GEN_MONSTER_COMPOUND:
		{
#if defined(_VILLAGESERVER)
			return false;
#elif defined(_GAMESERVER)

			if( tokens.size() != 3 )
				return false;

			int nMonsterID = _ttoi(tokens[1].c_str());
			int nCount = _ttoi(tokens[2].c_str());

			CDNGameRoom *pRoom = m_pSession->GetGameRoom();
			if(!pRoom) return false;
			if(!pRoom->GetTaskMng()) return false;

			CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
			if(!pTask) return false;

			SOBB Obb;
			Obb.Center = *m_pSession->GetActorHandle()->GetPosition();
			Obb.Axis[0] = EtVector3( 1.f, 0.f, 0.f );
			Obb.Axis[1] = EtVector3( 0.f, 1.f, 0.f );
			Obb.Axis[2] = EtVector3( 0.f, 0.f, 1.f );
			Obb.Extent[0] = Obb.Extent[1] = Obb.Extent[2] = 100.f;


			pTask->RequestGenerationMonsterFromMonsterGroupID( nMonsterID, nCount, Obb );
#endif
		}
		break;
	case GEN_NPC:
		{
			if( tokens.size() < 2 ) return false;

			int nNpcID = _ttoi(tokens[1].c_str());

#if defined(_VILLAGESERVER)
			m_pSession->GetField()->RequestGenerationNpc(nNpcID, &m_pSession->GetCurrentPos(), m_pSession->GetRotate());
#elif defined(_GAMESERVER)

			CDNGameRoom *pRoom = m_pSession->GetGameRoom();
			if(!pRoom) return false;
			if(!pRoom->GetTaskMng()) return false;

			CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
			if(!pTask) return false;

			EtVector3* pvPos = m_pSession->GetActorHandle()->GetPosition();
			pTask->RequestGenerationNpc(nNpcID, pvPos->x, pvPos->y, pvPos->z, 0);
#endif
		}
		break;

	case ADD_STATEEFFECT:
		{
#ifdef _GAMESERVER
			if( tokens.size() != 4 )
				return false;

			int iStateEffectIndex = _ttoi(tokens[1].c_str());
			int iDurationTime = _ttoi(tokens[2].c_str());
			
			if((int)tokens.size() > 2 )
			{
				const TCHAR* pParameter = tokens[3].c_str();
				int iCodePage = CEtResourceMng::GetInstance().GetCodePage();
				char acBuf[ 256 ];
				ZeroMemory( acBuf, sizeof(acBuf) );
				WideCharToMultiByte( iCodePage, 0, pParameter, -1, acBuf, sizeof(acBuf), NULL, NULL );

				CDNGameRoom *pRoom = m_pSession->GetGameRoom();
				if(!pRoom) return false;
				if(!pRoom->GetTaskMng()) return false;

				DnActorHandle hActor = m_pSession->GetActorHandle();
				hActor->CmdAddStateEffect( /*hActor,*/ NULL, (STATE_BLOW::emBLOW_INDEX)iStateEffectIndex, iDurationTime, acBuf );
			}
#endif
		}
		break;

	case NOTICE:
		{
			if(tokens.size() < 2)
				return false;

			WCHAR szMsg[CHATLENMAX];
			int nLen = 0;
			for (int i = 1; i <(int)tokens.size(); i++)
			{
				int nTempLen = (int)wcslen(tokens[i].c_str());
				if(nTempLen + nLen >= CHATLENMAX)
					return false;
				_wcscpy(szMsg+nLen, CHATLENMAX-nLen, tokens[i].c_str(), nTempLen);
				nLen += nTempLen;
				szMsg[nLen++] = ' ';
			}
			if( nLen < CHATLENMAX )
				szMsg[nLen] = '\0';
			else
				return false;

#if defined(_VILLAGESERVER)
			g_pMasterConnection->SendNoticeFromClient(szMsg, nLen);
#elif defined(_GAMESERVER)
			g_pMasterConnectionManager->SendNoticeFromClinet(szMsg, nLen);
#endif
		}
		break;

	case ADDINVENCOUNT:
		{
			if(tokens.size() < 2) return false;
			int nCount = _ttoi(tokens[1].c_str());
			if(nCount > INVENTORYMAX) return false;

			m_pSession->GetItem()->SetInventoryCount(nCount);
			m_pSession->SendInventoryMaxCount(nCount);
		}
		break;

	case ADDWARECOUNT:
		{
			if(tokens.size() < 2) return false;
			int nCount = _ttoi(tokens[1].c_str());
			if(nCount > WAREHOUSEMAX) return false;

			m_pSession->GetItem()->SetWarehouseCount(nCount);
			m_pSession->SendWarehouseMaxCount(nCount);
		}
		break;

	case REBIRTH:
		{
			if(tokens.size() < 4) return false;
			int nRebirth = (tokens.size() <= 1) ? 1 : _ttoi(tokens[1].c_str());
			if(nRebirth <= 0) nRebirth = 1;
			int nPCBang = (tokens.size() <= 2) ? 1 : _ttoi(tokens[2].c_str());
			if(nPCBang <= 0) nPCBang = 1;
			int nCash = (tokens.size() <= 3) ? 1 : _ttoi(tokens[3].c_str());
			if(nCash <= 0) nCash = 1;

			m_pSession->SetRebirthCoin(nRebirth, nPCBang, nCash);
			m_pSession->GetDBConnection()->QueryRebirthCoin(m_pSession, nRebirth, nPCBang, 0);
			m_pSession->SendRebirthCoin(ERROR_NONE, 0, _REBIRTH_SELF, m_pSession->GetSessionID());
		}
		break;

#if defined(_GAMESERVER )
		case RESPAWNLOGIC:
		{

			if( tokens.size() != 1 ) 
				return false;

			if( !m_pSession || !m_pSession->GetGameRoom() )
				return false;

			CDnGameTask* pGameTask = m_pSession->GetGameRoom()->GetGameTask();
			if( !pGameTask )
				return false;

			if( pGameTask->GetGameTaskType() != GameTaskType::PvP ) 
				return false;

			CPvPRespawnLogic* pRespawnLogic = ((CDnPvPGameTask*)pGameTask)->GetRespawnLogic();
			if( !pRespawnLogic )
				return false;

			pRespawnLogic->ToggleCheat();

			break;
		}

		case SETCULLING:
			{
				if( tokens.size() < 2 ) 
					return false;

				if( !m_pSession || !m_pSession->GetGameRoom() )
					return false;

				m_pSession->GetGameRoom()->SetActorTick(_ttoi(tokens[1].c_str()));
				m_pSession->GetGameRoom()->SetCullingDistance(_ttoi(tokens[2].c_str()));
			}
			break;
#endif
		
#if defined( _VILLAGESERVER )
		case GETPVPSCORE:
		{
			/*
BYTE cLevel; // Level
			UINT	uiXP;												// XP
UINT uiPlayTimeSec; // Play time (sec)
UINT uiWin; // Wins
UINT uiLose; // Losses
UINT uiDraw; // Draw
UINT uiGiviUpCount; // Mid-game quit count
UINT uiKOClassCount[PvPCommon::Common::MaxClass]; // KO count against each class
UINT uiKObyClassCount[PvPCommon::Common::MaxClass]; // Times KO'd by each class
			INT64	biTotalKillPoint;
			INT64	biTotalAssistPoint;
			INT64	biTotalSupportPoint;
			*/

			TPvPGroup* pPvP = m_pSession->GetPvPData();

			wchar_t wszBuf[MAX_PATH];
			wsprintf( wszBuf, L"[Level:%d,XP:%d] %dW %dL %dD %dF PlayTime:%ds", pPvP->cLevel, pPvP->uiXP, pPvP->uiWin, pPvP->uiLose, pPvP->uiDraw, pPvP->uiGiveUpCount, pPvP->uiPlayTimeSec );
			m_pSession->SendDebugChat( wszBuf );

			wsprintf( wszBuf, L"[KO] Warrior:%d Archer:%d Sorceress:%d Cleric:%d", pPvP->uiKOClassCount[0], pPvP->uiKOClassCount[1], pPvP->uiKOClassCount[2], pPvP->uiKOClassCount[3] );
			m_pSession->SendDebugChat( wszBuf );

			wsprintf( wszBuf, L"[KO_By] Warrior:%d Archer:%d Sorceress:%d Cleric:%d", pPvP->uiKObyClassCount[0], pPvP->uiKObyClassCount[1], pPvP->uiKObyClassCount[2], pPvP->uiKObyClassCount[3] );
			m_pSession->SendDebugChat( wszBuf );

			wsprintf( wszBuf, L"TotalKillPoint=%I64d TotalAssistPoint=%I64d TotalSupportPoint=%I64d", pPvP->biTotalKillPoint, pPvP->biTotalAssistPoint, pPvP->biTotalSupportPoint );
			m_pSession->SendDebugChat( wszBuf );

			break;
		}
#endif
		case ENABLESHADOW:
		{
#ifdef _GAMESERVER
			#ifdef _SHADOW_TEST
			if(tokens.size() < 1) return false;
			bool bEnable = true;
			if( tokens.size() >= 2 )
				bEnable = ( _ttoi(tokens[1].c_str()) == 0 ) ? false : true;
			g_bEnableShadow[m_pSession->GetRoomID()] = bEnable;
			#endif
#endif
		}
		break;

	case FATIGUE:
		{
			if(tokens.size() < 4) return false;
			int nFatigue = (tokens.size() <= 1) ? 1 : _ttoi(tokens[1].c_str());
			if(nFatigue <= 0) nFatigue = 1;
			int nWeekly = (tokens.size() <= 2) ? 1 : _ttoi(tokens[2].c_str());
			if(nWeekly <= 0) nWeekly = 1;
			int nPCBang = (tokens.size() <= 3) ? 1 : _ttoi(tokens[3].c_str());
			if(nPCBang <= 0) nPCBang = 1;
			int nEvent = (tokens.size() <= 4) ? 1 : _ttoi(tokens[4].c_str());
			if(nEvent <= 0) nEvent = 1;
			
#if defined(PRE_ADD_VIP)
			int nVIP = (tokens.size() <= 5) ? 1 : _ttoi(tokens[5].c_str());
			if(nVIP <= 0) nEvent = 1;

			m_pSession->SetCheatFatigue(nFatigue, nWeekly, nPCBang, nEvent, nVIP);

#else	// #if defined(PRE_ADD_VIP)
			m_pSession->SetCheatFatigue(nFatigue, nWeekly, nPCBang, nEvent, 0);
#endif	// #if defined(PRE_ADD_VIP)
		}
		break;

	case IGNORECOOLTIME:
		{
#if defined(_GAMESERVER)
			DnActorHandle hActor = m_pSession->GetActorHandle();
			//hActor->ResetSkillCoolTime();
			hActor->IgnoreSkillCoolTime();
#endif
		}
		break;

	case FCM:
		{
		}
		break;

	case SETFCM:
		{
		}
		break;

	case SET_HP:
		{
#ifdef _GAMESERVER
			wstring wszString;

			if( tokens.size() < 2 ){
				wszString = FormatW(L"Please enter HP value (ratio 0~100).\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDnPlayerActor *pPlayer = m_pSession->GetPlayerActor();
			if( !pPlayer ) break;
			INT64 nHP = (INT64)( pPlayer->GetMaxHP() *( _ttoi( tokens[1].c_str() ) / 100.f ) );
			if( nHP > pPlayer->GetMaxHP() ) nHP = pPlayer->GetMaxHP();
			else if( nHP < 1 ) nHP = 1;
			pPlayer->CmdRefreshHPSP( nHP, pPlayer->GetSP() );
#endif
		}
		break;
	case SET_MP:
		{
#ifdef _GAMESERVER
			wstring wszString;

			if( tokens.size() < 2 ){
				wszString = FormatW(L"Please enter SP value (ratio 0~100).\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDnPlayerActor *pPlayer = m_pSession->GetPlayerActor();
			if( !pPlayer ) break;
			int nSP = (int)( pPlayer->GetMaxSP() *( _ttoi( tokens[1].c_str() ) / 100.f ) );
			if( nSP > pPlayer->GetMaxSP() ) nSP = pPlayer->GetMaxSP();
			else if( nSP < 1 ) nSP = 1;
			pPlayer->CmdRefreshHPSP( pPlayer->GetHP(), nSP );
#endif
		}
		break;
	case SET_HPMPFULL:
		{
#if defined( _GAMESERVER )
			CDnPlayerActor *pPlayer = m_pSession->GetPlayerActor();
			if( !pPlayer ) 
				break;
			pPlayer->CmdRefreshHPSP( pPlayer->GetMaxHP(), pPlayer->GetMaxSP() );
#endif // #if defined( _GAMESERVER )
			break;
		}
	case GET_HP:
		{
#ifdef _GAMESERVER

			CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
			if( !pGameRoom )
				return false;

			std::wstring wstrCharName = m_pSession->GetCharacterName();

			if( tokens.size() == 2 )
				wstrCharName = tokens[1];

			CDNGameRoom::PartyStruct* pStruct = pGameRoom->GetPartyData( const_cast<WCHAR*>(wstrCharName.c_str()) );
			if( !pStruct || !pStruct->pSession )
				return false;

			DnActorHandle hActor = pStruct->pSession->GetActorHandle();
			if( !hActor )
				return false;

			INT64 iHP = hActor->GetHP();
			INT64 iMaxHP = hActor->GetMaxHP();

			WCHAR wszBuf[MAX_PATH];
			wsprintf( wszBuf, L"[%s] HP:%I64d/%I64d", wstrCharName.c_str(), iHP, iMaxHP );
			m_pSession->SendDebugChat( wszBuf );
#endif
			break;
		}
	case GAINMISSION:
	case ACHIEVEMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			if( nCmdID  == GAINMISSION ) pMission->RequestForceGainMission( _ttoi(tokens[1].c_str()) );
			else pMission->RequestForceAchieveMission( _ttoi(tokens[1].c_str()) );
		}
		break;
	case GAINDAILYMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the daily mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			switch( tokens.size() ) {
			case 2: pMission->RequestForceGainDailyMission( 1, _ttoi(tokens[1].c_str()) ); break;
			case 3: pMission->RequestForceGainDailyMission( 2, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()) ); break;
			case 4: pMission->RequestForceGainDailyMission( 3, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()) ); break;
			case 5: pMission->RequestForceGainDailyMission( 4, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()) ); break;
			case 6: pMission->RequestForceGainDailyMission( 5, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()) ); break;
			case 7: pMission->RequestForceGainDailyMission( 6, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()) ); break;
			case 8: pMission->RequestForceGainDailyMission( 7, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()) ); break;
			case 9: pMission->RequestForceGainDailyMission( 8, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()) ); break;
			case 10: pMission->RequestForceGainDailyMission( 9, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()) ); break;
			case 11: pMission->RequestForceGainDailyMission( 10, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()), _ttoi(tokens[10].c_str()) ); break;
			}

			m_pSession->GetDBConnection()->QuerySetDailyMission( m_pSession );
		}
		break;
	case ACHIEVEDAILYMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the daily mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			pMission->RequestForceAchieveDailyMission( _ttoi(tokens[1].c_str()) );
		}
		break;
	case GAINWEEKLYMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the weekly mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			switch( tokens.size() ) {
			case 2: pMission->RequestForceGainWeeklyMission( 1, _ttoi(tokens[1].c_str()) ); break;
			case 3: pMission->RequestForceGainWeeklyMission( 2, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()) ); break;
			case 4: pMission->RequestForceGainWeeklyMission( 3, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()) ); break;
			case 5: pMission->RequestForceGainWeeklyMission( 4, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()) ); break;
			case 6: pMission->RequestForceGainWeeklyMission( 5, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()) ); break;
			case 7: pMission->RequestForceGainWeeklyMission( 6, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()) ); break;
			case 8: pMission->RequestForceGainWeeklyMission( 7, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()) ); break;
			case 9: pMission->RequestForceGainWeeklyMission( 8, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()) ); break;
			case 10: pMission->RequestForceGainWeeklyMission( 9, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()) ); break;
			case 11: pMission->RequestForceGainWeeklyMission( 10, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()), _ttoi(tokens[10].c_str()) ); break;
			}
			
			m_pSession->GetDBConnection()->QuerySetWeeklyMission( m_pSession );
		}
		break;
	case ACHIEVEWEEKLYMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the weekly mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			pMission->RequestForceAchieveWeeklyMission( _ttoi(tokens[1].c_str()) );
		}
		break;
#if defined(PRE_ADD_MONTHLY_MISSION)
	case GAIN_MONTHLY_MISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the monthly mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			switch( tokens.size() ) {
				case 2: pMission->RequestForceGainMonthlyMission( 1, _ttoi(tokens[1].c_str()) ); break;
				case 3: pMission->RequestForceGainMonthlyMission( 2, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()) ); break;
				case 4: pMission->RequestForceGainMonthlyMission( 3, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()) ); break;
				case 5: pMission->RequestForceGainMonthlyMission( 4, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()) ); break;
				case 6: pMission->RequestForceGainMonthlyMission( 5, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()) ); break;
				case 7: pMission->RequestForceGainMonthlyMission( 6, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()) ); break;
				case 8: pMission->RequestForceGainMonthlyMission( 7, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()) ); break;
				case 9: pMission->RequestForceGainMonthlyMission( 8, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()) ); break;
				case 10: pMission->RequestForceGainMonthlyMission( 9, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()) ); break;
				case 11: pMission->RequestForceGainMonthlyMission( 10, _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()), _ttoi(tokens[3].c_str()), _ttoi(tokens[4].c_str()), _ttoi(tokens[5].c_str()), _ttoi(tokens[6].c_str()), _ttoi(tokens[7].c_str()), _ttoi(tokens[8].c_str()), _ttoi(tokens[9].c_str()), _ttoi(tokens[10].c_str()) ); break;
			}

			m_pSession->GetDBConnection()->QuerySetMonthlyMission( m_pSession );
		}
		break;

	case ACHIEVE_MONTHLY_MISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the monthly mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			pMission->RequestForceAchieveMonthlyMission( _ttoi(tokens[1].c_str()) );
		}
		break;
#endif
	case ACHIEVEGUILDMISSION:
		{
			if( tokens.size() < 2 ){
				wstring wszString = FormatW(L"Please enter the guild war mission index\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}

			CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
			if(pMission->bIsGuildWarMission())
				pMission->RequestForceAchieveGuildWarMission( _ttoi(tokens[1].c_str()) );
			else
			{
				wstring wszString = FormatW(L"Not within the guild war mission period.\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				return false;
			}
		}
		break;
	case MAKE_GUILDFESTIVAL_POINT :
		{
			if( tokens.size() != 2 )
				return false;

			int nFestivalPoint = _ttoi(tokens[1].c_str());

			// Additional guild festival points
			const TGuildUID GuildUID = m_pSession->GetGuildUID();
			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
			if(pGuild)
			{
#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if ( pGuild->IsEnable() )
				{
#endif
					TGuildRewardItem* GuildRewardItem = pGuild->GetGuildRewardItem();
					if( GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nItemID > 0 &&
						GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue > 0 )
					{
						nFestivalPoint += (int)(nFestivalPoint * GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue * 0.01);
					}
#if !defined( PRE_ADD_NODELETEGUILD )
				}				
#endif
			}
			m_pSession->AddEtcPoint(DBDNWorldDef::EtcPointCode::GuildWar_Festival, nFestivalPoint);
		}
		break;
	case FORCERANK:
		{
#ifdef _GAMESERVER
#endif
		break;
		}
		case ADD_XP:
		{
#if defined( _GAMESERVER )
			if( tokens.size() != 2 )
			{
				wstring wszString = FormatW(L"Please enter the XP value to increase.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
				return false;
			}

			// Handle lexical_cast exceptions
			if( tokens[1].size() > 9)
				tokens[1] = L"999999999";

			int iAddXP = boost::lexical_cast<int>( tokens[1].c_str() );
			if( iAddXP <= 0 )
			{
				wstring wszString = FormatW(L"Please enter an XP value greater than 0.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
				return false;
			}

			TPvPGroup* pPvPData = m_pSession->GetPvPData();

			pPvPData->uiXP += iAddXP;

			// PvP_Level
			for( UINT i=pPvPData->cLevel ; i<PvPCommon::Common::MaxRank ; ++i )
			{
				const TPvPRankTable* pPvPRankTable = g_pDataManager->GetPvPRankTable( i );
				if( !pPvPRankTable )
					return false;

#ifdef PRE_MOD_PVPRANK
				//Need to handle on cheat side
				if (pPvPRankTable->cType != PvPCommon::RankTable::ExpValue)
					continue;

				if( pPvPData->uiXP >= pPvPRankTable->uiXP )
#if defined(PRE_ADD_PVPLEVEL_MISSION)
					m_pSession->SetPvPLevel(static_cast<BYTE>(i+1));
#else
					pPvPData->cLevel = static_cast<BYTE>(i+1);
#endif
				else
					break;
#else		//#ifdef PRE_MOD_PVPRANK
				if( pPvPData->uiXP >= pPvPRankTable->uiXP )
#if defined(PRE_ADD_PVPLEVEL_MISSION)
					m_pSession->SetPvPLevel(static_cast<BYTE>(i+1));
#else
					pPvPData->cLevel = static_cast<BYTE>(i+1);
#endif
				else
					break;
#endif		//#ifdef PRE_MOD_PVPRANK
			}

			CDNDBConnection* pDBCon = m_pSession->GetDBConnection();
			if( pDBCon )
				pDBCon->QueryUpdatePvPData( m_pSession->GetDBThreadID(), m_pSession, PvPCommon::QueryUpdatePvPDataType::Max );
#else
			wstring wszString = FormatW(L"This cheat cannot be used in town.\r\n");
			m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif
			break;
		}
		case STAGECLEAR:
			{
#ifdef _GAMESERVER
#ifdef _TEST_CODE_KAL
				if( tokens.size() < 2 ){
					wstring wszString = FormatW(L"Please enter rank (0:SSS ~ 6:D), -1:cheat disabled.\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				CDNGameRoom *pRoom = m_pSession->GetGameRoom();
				if(!pRoom) return false;
				if(!pRoom->GetTaskMng()) return false;

				CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
				if(!pTask) return false;

				int rank = _ttoi(tokens[1].c_str());
				int difficulty = 4;

				pTask->RequestDungeonClear_ForCheat(64, 29771, 1, -1, 1, 2, rank, difficulty);
#endif
#endif
			}
			break;
		case SET_ENCHANT:
			{
#ifdef _VILLAGESERVER
				if( tokens.size() < 2 ) {
					break;
				}
				int nInvenSlot = _wtoi( tokens[1].c_str() );
				int nLevel = ( tokens.size() >= 3 ) ? _wtoi( tokens[2].c_str() ) : -1;
//				if( nLevel < 1 ) nLevel = -1;
				int nPotential = ( tokens.size() >= 4 ) ? _wtoi( tokens[3].c_str() ) : -1;

				if( nInvenSlot < 0 || nInvenSlot >= INVENTORYMAX ) break;
				CDNUserItem *pUserItem = m_pSession->GetItem();
				const TItem *pItem = pUserItem->GetInventory( nInvenSlot );
				if( !pItem ) break;
				TItemData *pItemData = g_pDataManager->GetItemData( pItem->nItemID );
				if( !pItemData ) break;

				TEnchantData *pEnchantData = g_pDataManager->GetEnchantData( pItemData->nEnchantID );
				if( !pEnchantData ) break;

				if( nLevel == -1 ) 
					const_cast<TItem*>(pItem)->cLevel = 1 +( rand()%pEnchantData->cCount );
				else {
					if( nLevel > pEnchantData->cCount ) nLevel = pEnchantData->cCount;
					const_cast<TItem*>(pItem)->cLevel = (char)nLevel;
				}
				if( pItem->cLevel > 0 ) {
#ifdef PRE_FIX_MEMOPT_ENCHANT
					DWORD potentialID = g_pDataManager->GetItemTypeParam2(pItem->nItemID);
					TPotentialData* pPotentialData = g_pDataManager->GetPotentialData(potentialID);
#else
					TEnchantLevelData *pEnchantLevelData = g_pDataManager->GetEnchantLevelData( pItemData->nEnchantID, pItem->cLevel );
					if( !pEnchantLevelData ) break;
					TPotentialData *pPotentialData = g_pDataManager->GetPotentialData( pEnchantLevelData->nPotentialID );
#endif
					if( !pPotentialData ) 
						break;
					if( nPotential == -1 ) 
					{
						if( pPotentialData->pVecItemData.size() == 0 )
							break;
						const_cast<TItem*>(pItem)->cPotential = (char)( 1 +( rand()%pPotentialData->pVecItemData.size() ) );
					}
					else {
						if( nPotential >(int)pPotentialData->pVecItemData.size() ) nPotential = (int)pPotentialData->pVecItemData.size();
						const_cast<TItem*>(pItem)->cPotential = (char)nPotential;
					}
				}
				else 
					const_cast<TItem*>(pItem)->cPotential = 0;
				m_pSession->SendRefreshInven((BYTE)nInvenSlot, pItem, false );
#endif
			}
		case LUCKY:
			m_bLucky = true;
			if( tokens.size() >= 2 ) {
				m_bLucky = ( _wtoi( tokens[1].c_str() ) == 0 ) ? false : true;
			}
			break;

		case MAKECITEM:
			{
				wstring wszString;

				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please check the parameters properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

#if defined(_VILLAGESERVER)
				return false;
#elif defined(_GAMESERVER)


				CDNGameRoom *pSession = m_pSession->GetGameRoom();
				if(!pSession) return false;
				if(!pSession->GetTaskMng()) return false;

				CDnItemTask *pTask = (CDnItemTask *)pSession->GetTaskMng()->GetTask( "ItemTask" );
				if(!pTask) return false;

				int nItemID = _wtoi(tokens[1].c_str());
				int nCount = ( tokens.size() >= 3 ) ? _wtoi(tokens[2].c_str()) : 1;
				if(nCount <= 0) nCount = 1;

				if(g_pDataManager->GetItemDropData( nItemID ) == NULL) return false;

				CMultiRoom *pRoom = pTask->GetRoom();

				for( int i=0; i<nCount; i++ ) {
					DNVector(CDnItem::DropItemStruct) VecList;
					CDnDropItem::CalcDropItemList( pRoom, nItemID, VecList );
					for( DWORD j=0; j<VecList.size(); j++ ) {
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
						pTask->RequestDropItem( VecList[j].dwUniqueID, *(m_pSession->GetActorHandle()->GetPosition()), VecList[j].nItemID, VecList[j].nSeed, 1, (short)( _rand(pRoom)%360 ), -1, VecList[j].nEnchantID);
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
						pTask->RequestDropItem( VecList[j].dwUniqueID, *(m_pSession->GetActorHandle()->GetPosition()), VecList[j].nItemID, VecList[j].nSeed, 1, (short)( _rand(pRoom)%360 ));
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					}
				}
#endif
			}
			break;
		case SETCP:
			{
#if defined(_GAMESERVER)
				if( tokens.size() < 2 ) return false;
				int nCP = _wtoi( tokens[1].c_str() );
				m_pSession->GetPlayerActor()->SetAccumulationCP( nCP );
#endif
			}
			break;

		case MAILREMAIN:
			{
				if(tokens.size() < 2) return false;
				int nHour = _wtoi(tokens[1].c_str());
			}
			break;

		case FULLFRIEND:
			{
				WCHAR szTempName[NAMELENMAX];
				TFriendGroup * pGroup = NULL;

				for (int i = 1; i <= 100; i++)
				{
					if(i < 10)	wsprintf(szTempName, L"Sorceress0%d", i);
					else wsprintf(szTempName, L"Sorceress%d", i);
					
					if(m_pSession->GetFriend()->GetFriendCount() >= FRIEND_MAXCOUNT)				break;
					if(m_pSession->GetFriend()->HasFriend(szTempName) == true)					continue;
					
				}
			}
			break;

		case FULLBLOCK:
			{
				WCHAR szTempName[NAMELENMAX];

				for (int i = 1; i <= 100; i++)
				{
					if (i < 10) wsprintf(szTempName, L"Sorceress0%d", i);
					else wsprintf(szTempName, L"Sorceress%d", i);

					if( m_pSession->GetIsolate()->IsIsolateItem(szTempName) == true) continue;

					TFriend * pFriend = m_pSession->GetFriend()->GetFriend(szTempName);
					if (pFriend == NULL)
					{
						m_pSession->GetDBConnection()->QueryAddIsolate(m_pSession, szTempName);
					}
				}
			}
			break;

		case RELOAD_SCRIPT:
			{
#if !defined(_FINAL_BUILD)
				g_pQuestManager->LoadAllQuest("QuestTable.ext", g_pNpcQuestScriptManager);
				g_pDataManager->LoadTalkData();
				g_pNpcQuestScriptManager->CloseAllState();
				LoadNpcQuest();

				g_pDataManager->LoadNpcData();	// 2010.11.11 Robust added
				g_pDataManager->LoadQuestReward(); // 2010. 11. 25 Robust added
#if defined(PRE_ADD_REMOTE_QUEST)
				g_pDataManager->LoadRemoteQuestData();
#endif			
				wstring wszString;
				wszString = FormatW( L"All Script Reloaded");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
#endif	// _FINAL_BUILD
			}
			break;

			// Description: Reload monster AI scripts.
			// Author : Robust
			// Created : 2010.11.11
#if defined(_GAMESERVER)
		case RELOAD_MONSTER_AI:
			{
				g_AiScriptLoader.AllLoadScript();
			}
			break;
#endif // #if defined(_GAMESERVER)

		// Description: Cheat to track and intrude into a user's active dungeon
		// Author: Kimbap
		// Created : 2009.10.30
		
		case TRACE:
		{
			DN_ASSERT( m_pSession != NULL, "CDNCheatCommand::OnCommand() case TRACE: m_pSession != NULL" );
			DN_ASSERT( m_pSession->GetGMCommand() != NULL, "CDNCheatCommand::OnCommand() case TRACE: m_pSession->GetGMCommand() != NULL" );

			return m_pSession->GetGMCommand()->CmdTrace(TRACE_CHARACTERNAME, tokens );
		}

		// Description: Summon user to GM town
		// Author: Kimbap
		// Created : 2009.11.16

		case RECALL:
		{
			DN_ASSERT( m_pSession != NULL, "CDNCheatCommand::OnCommand() case RECALL: m_pSession != NULL" );
			DN_ASSERT( m_pSession->GetGMCommand() != NULL, "CDNCheatCommand::OnCommand() case TRACE: m_pSession->GetGMCommand() != NULL" );

			return m_pSession->GetGMCommand()->CmdRecall( tokens );
		}

		// Description: Execute cheat set
		// Author: Kimbap
		// Created : 2009.12.01

		case CHEATSET:
		{
			return _bCommandCheatSet( tokens );
		}

		case SETSP:
			{
				if( tokens.size() < 2 ) 
					return false;

				unsigned short usSkillPoint = (unsigned short)_wtoi( tokens[1].c_str() );
				_ASSERT( 0 <= usSkillPoint );
				m_pSession->SetCheatSkillPoint( usSkillPoint, -1 );
			}
			break;

		case SET_USER_JOB:
		{
			if( tokens.size() < 2 )
				return false;

			DNTableFileFormat* pJobTable = GetDNTable( CDnTableDB::TJOB );

			// Get the tier value and root job of the current job.
			int iNowJob = m_pSession->GetUserJob();
			int iNowJobDeep = 0;
			int iNowRootJob = 0;
			for( int i = 0; i < pJobTable->GetItemCount(); ++i )
			{
				int iItemID = pJobTable->GetItemID( i );
				if( iItemID == iNowJob )
				{
					iNowJobDeep = pJobTable->GetFieldFromLablePtr( iItemID, "_JobNumber" )->GetInteger();
					iNowRootJob = pJobTable->GetFieldFromLablePtr( iItemID, "_BaseClass" )->GetInteger();
					break;
				}
			}

			int iJobIDToChange = _wtoi( tokens[1].c_str() );

			// Check if target job tier is greater than or equal.
			bool bSuccess = false;
			map<int, int> mapRootJob;
			for( int i = 0; i < pJobTable->GetItemCount(); ++i )
			{
				int iItemID = pJobTable->GetItemID( i );
				if( iItemID == iJobIDToChange )
				{
					int iJobRootToChange = pJobTable->GetFieldFromLablePtr( iItemID, "_BaseClass" )->GetInteger();
					if( iNowRootJob == iJobRootToChange )
					{
						int iJobDeepToChange = pJobTable->GetFieldFromLablePtr( iItemID, "_JobNumber" )->GetInteger();
						if( iNowJobDeep < iJobDeepToChange )
						{
							// Parent job must also match.
							int iParentJobID = pJobTable->GetFieldFromLablePtr( iItemID, "_ParentJob" )->GetInteger();
							if( iParentJobID == iNowJob )
							{
								m_pSession->SetUserJob( iJobIDToChange );
								// In Korea, reset is not currently performed on job change. (#19141)
								// Therefore, call skill reset separately when changing job via cheat key.
#ifdef _VILLAGESERVER
								for(int nSkillPage = DualSkill::Type::Primary; nSkillPage < DualSkill::Type::MAX ; nSkillPage++ )
									m_pSession->GetSkill()->ResetSkill( nSkillPage );
#endif // #ifdef _VILLAGESERVER
								bSuccess = true;
							}
							else
							{
								// Parent job of target job is not the current job.
								wstring wszString = FormatW(L"Cannot advance to this job from your current class.!!\r\n");
								m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());

								return false;
							}
						}
						else
						{
							// Target job is a lower tier. Cannot change.
							wstring wszString = FormatW(L"Cannot change to a job of the same or lower tier!!\r\n");
							m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());

							return false;
						}
					}
					else
					{
						// Target job is a different class. Cannot change.
						wstring wszString = FormatW(L"Cannot change to a job of a different class!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());

						return false;
					}
				}
			}

			if( false == bSuccess )
			{
				wstring wszString = FormatW(L"Invalid Job ID..\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());

				return false;
			}

			break;
		}

		// Description: Repair durability of all items (usable only in town)
		// Author: Kimbap
		// Created : 2009.12.10

		case REPAIRALL:
		{
#if defined( _VILLAGESERVER )
			if( !m_pSession->GetItem() )
				return false;
			m_pSession->GetItem()->OnRecvRepairAll( true );
#else // #if defined( _VILLAGESERVER )
			wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
			m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			break;
		}

		case BUILDUPALL:
			{
				if( tokens.size() < 2 ) return false;
				int nItemLevel = _wtoi( tokens[1].c_str() );
				if(nItemLevel <= 0) nItemLevel = 1;

#if defined( _VILLAGESERVER )
				if( !m_pSession->GetItem() ) return false;
				m_pSession->GetItem()->CheatUpgradeItemLevel(nItemLevel);
#else // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			}
			break;

		case POTENTIALALL:
			{
				if( tokens.size() < 2 ) return false;
				int nItemPotential = _wtoi( tokens[1].c_str() );
				if(nItemPotential <= 0) nItemPotential = 1;

#if defined( _VILLAGESERVER )
				if( !m_pSession->GetItem() ) return false;
				m_pSession->GetItem()->CheatUpgradeItemPotential(nItemPotential);
#else // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			}
			break;

		case GUILD_CREATE:			// Create guild
			{
				if(2 > tokens.size()) {
					return false;
				}
				const std::wstring aGuildName = tokens[1].c_str();
				if(aGuildName.empty() || GUILDNAME_MAX <= aGuildName.size()) {
					return false;
				}

#if defined(_VILLAGESERVER)

				// 20100629 Added check for country-specific Unicode character ranges when creating guild name
				if(!g_CountryUnicodeSet.Check(aGuildName.c_str())) 
				{	
					wstring wszString = FormatW(L"Guild name contains invalid characters.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Already in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				int wGuildRoleAuth[GUILDROLE_TYPE_CNT] = { 0, };
				g_pGuildManager->SetAuthBase(wGuildRoleAuth);	// Apply guild authority (initial)

				m_pSession->GetDBConnection()->QueryCreateGuild(m_pSession->GetDBThreadID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), aGuildName.c_str(), m_pSession->GetMapIndex(), m_pSession->GetLevel(), 0, g_Config.nWorldSetID, wGuildRoleAuth);

				wstring wszString = FormatW(L"Guild creation requested successfully.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#else // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			}
			break;

		case GUILD_DISMISS:			// Disband guild
			{
				if(1 > tokens.size()) {
					return false;
				}

#if defined(_VILLAGESERVER)
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				// Check if guild is already disbanded
				if(g_pGuildManager->IsDismissExist(GuildUID)) {
					m_pSession->ResetGuildSelfView();
					m_pSession->SendDismissGuild(m_pSession->GetSessionID(), ERROR_GUILD_ALREADYDISMISS_GUILD, NULL);
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif


				if(GUILDROLE_TYPE_MASTER != m_pSession->GetGuildSelfView().btGuildRole) 
				{
					wstring wszString = FormatW(L"Only the guild master can do this.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				
				if(!pGuild->GetMemberInfo(m_pSession->GetCharacterDBID())) 
				{
					wstring wszString = FormatW(L"Not a member of that guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				if(pGuild->GetMemberCount() > 1) 
				{
					wstring wszString = FormatW(L"Guild members still exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				m_pSession->GetDBConnection()->QueryDismissGuild( m_pSession );

				wstring wszString = FormatW(L"Guild disband requested successfully.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#else // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			}
			break;

		case GUILD_CHANGELEVEL:		// Set guild level
			{
				if(2 > tokens.size()) {
					return false;
				}
				short aNewGuildLevel = _wtoi(tokens[1].c_str());
				{	
					TGuildLevel* GuildNextLevel = g_pDataManager->GetGuildLevelData(aNewGuildLevel);
					if (!GuildNextLevel)
						return false;

					const TGuildUID GuildUID = m_pSession->GetGuildUID();
					if(!GuildUID.IsSet()) {
						wstring wszString = FormatW(L"Not in a guild.\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
						break;
					}

					CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

					if(!pGuild) 
					{
						wstring wszString = FormatW(L"Guild info does not exist.\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
						break;
					}


#if !defined( PRE_ADD_NODELETEGUILD )
					CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
					if(FALSE == pGuild->IsEnable()) break;
#endif

					m_pSession->GetDBConnection()->QueryModGuildCheat(m_pSession->GetDBThreadID(), pGuild->GetUID().nWorldID, pGuild->GetUID().nDBID, 1, GuildNextLevel->nReqGuildPoint, (char)aNewGuildLevel);

				}
			}
			break;

	
		case GUILD_MAKECOIN:		// Increase guild funds
			{
				if(2 > tokens.size()) {
					return false;
				}
				INT64 aNewGuildMoney = _wtoi(tokens[1].c_str());
				if(!CHECK_LIMIT(aNewGuildMoney, GUILDMONEY_MAX)) {
					return false;
				}

#if defined(_VILLAGESERVER)
				// Cheat support deferred as the feature is not yet implemented (2009-12-16)
/*
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
wstring wszString = FormatW(L"Not a member of a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuild* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild || !pGuild->Attach(GuildUID)) {
wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
				CDNGuild::TP_DETACHAUTO aDetachAuto(pGuild);

				const INT64 aCurGuildMoney = pGuild->GetInfo()->ui64GuildMoney;
				DN_ASSERT(CHECK_LIMIT(aCurGuildMoney, GUILDMONEY_MAX),	"Invalid!");

				if(aCurGuildMoney == aNewGuildMoney) {
wstring wszString = FormatW(L"Same as current guild funds.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				aNewGuildMoney -= aCurGuildMoney;

				g_pDBConnectionManager->QueryChangeGuildInfo(m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), GuildUID.nDBID, g_Config.nWorldSetID, GUILDUPDATE_TYPE_MONEY, aNewGuildMoney, 0, static_cast<INT64>(0), L"", 0);

wstring wszString = FormatW(L"Guild fund change has been requested normally.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
*/
#else // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
			}
			break;

		case GUILD_CHANGECMMPOINT:	// Increase guild normal points
			{
				if(2 > tokens.size()) {
					return false;
				}
				long aNewGuildCommonPoint = _wtoi(tokens[1].c_str());
				if(!CHECK_LIMIT(aNewGuildCommonPoint, GUILDCMMPTR_MAX)) {
					return false;
				}

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				m_pSession->AddGuildPoint(GUILDPOINTTYPE_STAGE, aNewGuildCommonPoint);
			}
			break;

		case GUILD_CHANGEMISSIONPOINT:	// Increase guild mission points
			{
				if(3 > tokens.size()) {
					return false;
				}
				long aNewPoint = _wtoi(tokens[1].c_str());
				if(!CHECK_LIMIT(aNewPoint, GUILDCMMPTR_MAX)) {
					return false;
				}

				int nMissionID = _wtoi(tokens[2].c_str());

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				m_pSession->AddGuildPoint(GUILDPOINTTYPE_MISSION, aNewPoint, nMissionID);
				return true;
			}
			break;

		case GUILD_CHANGEWARPOINT:
			{
				if(2 > tokens.size())
					return false;

				long aNewPoint = _wtoi(tokens[1].c_str());
				if(!CHECK_LIMIT(aNewPoint, GUILDCMMPTR_MAX))
					return false;

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				m_pSession->AddGuildPoint(GUILDPOINTTYPE_WAR, aNewPoint);

			}
			break;

		case ACHIEVEGUILDCOMMONMISSION:
			{
				if( tokens.size() < 2 ){
					wstring wszString = FormatW(L"Please enter the normal guild mission index\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) {
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				CDNMissionSystem *pMission = m_pSession->GetMissionSystem();
				if (pMission)
					pMission->RequestForceAchieveGuildCommonMission( _ttoi(tokens[1].c_str()) );
			}
			break;

		case GUILD_CHANGEMEMBERCOUNT:
			{
				if(2 > tokens.size()) 
					return false;

				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) 
				{
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				TAGetGuildInfo packet;
				memset(&packet, 0, sizeof(packet));
				packet.nRetCode = ERROR_NONE;

				int nCount = _wtoi(tokens[1].c_str());

				if(!pGuild->IsMemberAddable())
				{
					wstring wszString = FormatW(L"Cannot add any more guild members.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				if(nCount > pGuild->GetInfo()->wGuildSize)
				{
					wstring wszString = FormatW(L"Exceeded the maximum allowed guild members.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				if(nCount > GUILDSIZE_MAX)
					nCount = GUILDSIZE_MAX;

				TGuildMember MemberInfo;
				TAGetGuildMember GuildMember;
				memset(&GuildMember, 0, sizeof(GuildMember));

				for (int i=0; i<nCount-1; i++)
				{
					wsprintf(MemberInfo.wszCharacterName, L"GuildMember_%d", i);
					MemberInfo.nCharacterDBID = i;					
					
					GuildMember.MemberList[GuildMember.nCount] = MemberInfo;
					++GuildMember.nCount;

					if( GuildMember.nCount >= SENDGUILDMEMBER_MAX )
					{
						pGuild->UpdateMember(&GuildMember);
						memset(&GuildMember, 0, sizeof(GuildMember));
					}
				}
				if(GuildMember.nCount > 0)
					pGuild->UpdateMember(&GuildMember);

				// Add self.
				MemberInfo.nAccountDBID = m_pSession->GetAccountDBID();
				MemberInfo.nCharacterDBID = m_pSession->GetCharacterDBID();
				wsprintf(MemberInfo.wszCharacterName, L"%s", m_pSession->GetCharacterName());
				pGuild->AddMember(&MemberInfo, NULL);

				// Construct packet
				m_pSession->SendGetGuildInfo(pGuild, ERROR_NONE);
				m_pSession->SendGetGuildMember(pGuild, ERROR_NONE);
			}
			break;
			
			// Description: Reset 2nd authentication info
			// Author: Kimbap
			// Created : 2010.01.27

		case RESET_SECONDAUTH:
			{
				break;
			}
		case FARM_START:
			{
#if defined( _VILLAGESERVER )
				CDNField* pField = m_pSession->GetField();
				if( pField )
				{
					if( pField->bIsPvPVillage() || pField->bIsPvPLobby() || pField->bIsDarkLairVillage() )
					{
						std::wstring wszString = FormatW(L"Cannot use this cheat in PvP Town, PvP Lobby, or Dark Lair Town.\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
						return false;
					}
				}

				CSFarmEnter packet;
				memset(&packet, 0, sizeof(CSFarmEnter));

				packet.iFarmDBID = 1;
				packet.iMapID = 15106;
				
				m_pSession->OnRecvFarmMessage( eFarm::CS_START, (char*)&packet, sizeof(packet) );
				
#else
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
				break;
			}

		case IGNOREHIT:
			{
#if defined(_GAMESERVER)
				DnActorHandle hActor = m_pSession->GetActorHandle();
				if( hActor )
					hActor->ToggleIgnoreHit();
#endif
				break;
			}

		case INVENCLEAR:
			{
				if(tokens.size() < 1) return false;
				int nInven = (tokens.size() <= 1) ? 0 : _ttoi(tokens[1].c_str());

				switch(nInven)
				{
				case 0:
					{
						m_pSession->GetItem()->CheatClearInven();

						std::wstring wszString = FormatW(L"Normal inventory has been reset..\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					}
					break;

				case 1:
					{
						m_pSession->GetItem()->CheatClearCashInven();

						std::wstring wszString = FormatW(L"Cash inventory has been reset..\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					}
					break;

				case 2:
					{
						m_pSession->GetItem()->CheatClearQuestInven();

						std::wstring wszString = FormatW(L"Quest inventory has been reset..\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					}
					break;

				case 3:
					{
						m_pSession->GetItem()->CheatClearVehicleInven();

						std::wstring wszString = FormatW(L"Mount inventory has been reset..\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					}
					break;
				}
			}
			break;

		case GIFT:
			{
				if( tokens.size() >= 2 )
				{
					int nItemSN = _ttoi(tokens[1].c_str());
					TCashCommodityData CashData;
					TCashPackageData PackageData;
					if((!g_pDataManager->GetCashCommodityData(nItemSN, CashData)) && (!g_pDataManager->GetCashPackageData(nItemSN, PackageData))) return false;

					int nItemID = g_pDataManager->GetCashCommodityItem0(nItemSN);
					if (nItemID <= 0)
					{
						std::wstring wszString = FormatW(L"No item ID exists for that serial in the cash commodity..\r\n");
						m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					}

					m_pSession->GetDBConnection()->QueryGiftByCheat(m_pSession, nItemSN);
				}
			}
			break;

		case GETPOS:
			{
#if !defined( _VILLAGESERVER )
				//TPosition aCurPos = m_pSession->GetCurrentPos();
				
				EtVector3* pvPos = m_pSession->GetActorHandle()->GetPosition();
				
				std::wstring wszString = FormatW(L"POS (X:%.2f, Y:%.2f, Z:%.2f)", pvPos->x,pvPos->y,pvPos->z);
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#else	 // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif	 // #if defined( _VILLAGESERVER )
			}
			break;

		case SETPOS:
			{
#if defined( _VILLAGESERVER )
				if(tokens.size() < 4) {
					return false;
				}
				
				int nX = _ttoi(tokens[1].c_str());
				int nY = _ttoi(tokens[2].c_str());
				int nZ = _ttoi(tokens[3].c_str());

				TPosition aCurPos = m_pSession->GetCurrentPos();

				m_pSession->ChangePos(nX, nY, nZ, 0.f, 0.f);

				std::wstring wszString = FormatW(L"Character moved, previous position(X:%d, Y:%d, Z:%d)", aCurPos.nX, aCurPos.nY, aCurPos.nZ);
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#else	 // #if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used in town.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif	 // #if defined( _VILLAGESERVER )
			}
			break;

#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
		case SETREPUTE:
			{
				if( tokens.size() < 4 )
				{
					return false;
				}

				int iNpcID = _ttoi( tokens[1].c_str() );
				int iFavor = _ttoi( tokens[2].c_str() );
				int iMalice = _ttoi( tokens[3].c_str() );

				CReputationSystemRepository* pReputationRepos = m_pSession->GetReputationSystem();
				if( pReputationRepos )
				{
					int iCurFavor	= static_cast<int>(pReputationRepos->GetNpcReputation( iNpcID, IReputationSystem::NpcFavor ));
					int iCurMalice	= static_cast<int>(pReputationRepos->GetNpcReputation( iNpcID, IReputationSystem::NpcMalice ));

					CNpcReputationProcessor::Process( m_pSession, iNpcID, IReputationSystem::NpcFavor, iFavor-iCurFavor );
					CNpcReputationProcessor::Process( m_pSession, iNpcID, IReputationSystem::NpcMalice, iMalice-iCurMalice );
				}
				else
					return false;
			}
			break;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
		case MASTERSYSTEM_MOD_GRADUATECOUNT:
			{
				if( tokens.size() < 2 )
					return false;

				int iGraduateCount = _ttoi( tokens[1].c_str() );
				if( iGraduateCount < 0 )
					return false;

				if( m_pSession == NULL || m_pSession->CheckDBConnection() == false )
					return false;

				m_pSession->GetDBConnection()->QueryModMasterSystemGraduateCount( m_pSession, iGraduateCount );
				break;
			}
		case MASTERSYSTEM_SKIP_DATE:
			{
#if defined( _VILLAGESERVER ) && !defined( _FINAL_BUILD )
				if( tokens.size() < 2 )
					return false;

				bool bIsSkip = _ttoi( tokens[1].c_str() ) ? true : false;
				m_pSession->m_bIsMasterSystemSkipDate =  bIsSkip;
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _VILLAGESERVER )
				break;
			}
		case MASTERSYSTEM_MOD_FAVORPOINT:
			{
#if defined( _GAMESERVER )
				if( tokens.size() < 3 )
					return false;
	
				int iPoint = _ttoi( tokens[2].c_str() );
				if( iPoint < 0 )
					return false;

				if( m_pSession == NULL || m_pSession->CheckDBConnection() == false )
					return false;

				// Check if there is a pupil
				if( m_pSession->GetMasterSystemData()->SimpleInfo.iPupilCount <= 0 )
					return false;

				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom == NULL )
					return false;

				INT64 biPupilCharacterDBID = 0;
				for( UINT i=0 ;i<pGameRoom->GetUserCount() ; ++i )
				{
					CDNUserSession* pSession = pGameRoom->GetUserData(i);
					if( pSession && wcscmp( pSession->GetCharacterName(), tokens[1].c_str() ) == 0 )
					{
						biPupilCharacterDBID = pSession->GetCharacterDBID();
						break;
					}
				}

				if( biPupilCharacterDBID == 0 )
					return false;

				m_pSession->GetDBConnection()->QueryModMasterSystemFavorPoint( m_pSession, m_pSession->GetCharacterDBID(), biPupilCharacterDBID, iPoint );
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Game Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _GAMESERVER )
				break;
			}
		case MASTERSYSTEM_MOD_RESPECTPOINT:
			{
#if defined( _GAMESERVER )
				if( tokens.size() < 2 )
					return false;
				
				int iPoint = _ttoi( tokens[1].c_str() );
				if( iPoint < 0 )
					return false;

				if( m_pSession == NULL || m_pSession->CheckDBConnection() == false )
					return false;

				m_pSession->GetDBConnection()->QueryModMasterSystemRespectPoint( m_pSession, iPoint );
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Game Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif // #if defined( _GAMESERVER )
				break;
			}
			case FARM_PLANTSEED:
			{
#if defined( _GAMESERVER )
				// { FARM_PLANTSEED, _T("/plantseed"), _T("Usage(/plantseed(AreaIndex)(ActionType)(SeedInvenIndex)" },
				if( tokens.size() < 3 )
					return false;

				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom->bIsFarmRoom() == false )
					return false;

				CSFarmPlant TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.iAreaIndex				= _ttoi( tokens[1].c_str() );
				TxPacket.ActionType				= static_cast<Farm::ActionType::eType>(_ttoi( tokens[2].c_str() ));
				TxPacket.cSeedInvenIndex		= _ttoi( tokens[3].c_str() );

				pGameRoom->GetGameTask()->OnDispatchMessage( m_pSession, CS_FARM, eFarm::CS_PLANT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket)-sizeof(TxPacket.AttachItems) );
#endif // #if defined( _GAMESERVER )
				break;
			}
			case FARM_DESTROYSEED:
			{
#if defined( _GAMESERVER )
				// { FARM_DESTROYSEED, _T("/destroyseed"), _T("Usage(/destroyseed(AreaIndex)") },
				if( tokens.size() < 2 )
					return false;

				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom->bIsFarmRoom() == false )
					return false;

				CGrowingArea* pArea = static_cast<CDnFarmGameTask*>(pGameRoom->GetGameTask())->GetGrowingArea( _ttoi(tokens[1].c_str() ) );
				pArea->ChangeState( Farm::AreaState::NONE );

#endif // #if defined( _GAMESERVER )
				break;
			}
			case FARM_SKIPSEED:
			{
#if defined( _GAMESERVER )
				// { FARM_SKIPSEED, _T("/skipseed"), _T("Usage(/skipseed(AreaIndex)") },
				if( tokens.size() < 2 )
					return false;

				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom->bIsFarmRoom() == false )
					return false;

				float fMinusSec = 2.f;
				if( tokens.size() >= 3 )
					fMinusSec = static_cast<float>(_ttoi(tokens[1].c_str() ));

				CGrowingArea* pArea = static_cast<CDnFarmGameTask*>(pGameRoom->GetGameTask())->GetGrowingArea( _ttoi(tokens[1].c_str() ) );
				if( pArea && pArea->GetState() == Farm::AreaState::GROWING )
					pArea->GetPlantSeed()->SkipSeed( fMinusSec );

#endif // #if defined( _GAMESERVER )
				break;
			}
#if defined( PRE_ADD_FARM_DOWNSCALE )
			case FARM_ADDATTR:
			{
#if defined( _GAMESERVER )
				if( tokens.size() < 2 )
					return false;

				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom->bIsFarmRoom() == false )
					return false;

				CDNFarmGameRoom* pFarmRoom = static_cast<CDNFarmGameRoom*>(pGameRoom);
				pFarmRoom->AddAttr( static_cast<Farm::Attr::eType>(_ttoi(tokens[1].c_str())) );

#endif // #if defined( _GAMESERVER )
				break;
			}
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )

#if defined( PRE_ADD_FARM_DOWNSCALE )
				case FARM_DELATTR:
				{
#if defined( _GAMESERVER )
					if( tokens.size() < 2 )
						return false;

					CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
					if( pGameRoom->bIsFarmRoom() == false )
						return false;

					CDNFarmGameRoom* pFarmRoom = static_cast<CDNFarmGameRoom*>(pGameRoom);
					pFarmRoom->DelAttr( static_cast<Farm::Attr::eType>(_ttoi(tokens[1].c_str())) );

#endif // #if defined( _GAMESERVER )
					break;
				}
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )

		case NESTCLEAR:
			{
				// { NESTCLEAR, _T("/nest"), _T("Usage(/nest(NestMapID)") },
				if(tokens.size() < 2)
					return false;

				int nMapID = _ttoi(tokens[1].c_str());

				const TMapInfo *pMapInfo = g_pDataManager->GetMapInfo(nMapID);
				if(!pMapInfo) return false;

#if defined(PRE_SHARE_MAP_CLEARCOUNT)
				DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMAP );				
				if( pSox->IsExistItem( nMapID ) )
				{
					int nDungeonEnterTableID = pSox->GetFieldFromLablePtr( nMapID, "_EnterConditionTableID" )->GetInteger();
					if( nDungeonEnterTableID > 0 ) 
					{
						DNTableFileFormat *pDungeonSOX = GetDNTable( CDnTableDB::TDUNGEONENTER );
						if( pDungeonSOX->IsExistItem( nDungeonEnterTableID ) ) {
							if(pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_ShareCountMap" )->GetInteger() > 0)
							{
								nMapID = pDungeonSOX->GetFieldFromLablePtr( nDungeonEnterTableID, "_ShareCountMap" )->GetInteger();
								pMapInfo = g_pDataManager->GetMapInfo(nMapID);
								if(!pMapInfo) return false;
							}
						}
					}
				}				
#endif

				if(m_pSession->GetNestClearTotalCount(nMapID) >= (pMapInfo->nMaxClearCount + m_pSession->m_nExpandNestClearCount))
					return false;

				m_pSession->AddNestClear(nMapID);
				m_pSession->SendUpdateNestClear(m_pSession->GetNestClearCount(nMapID));
			}
			break;

#if defined( PRE_ADD_SECONDARY_SKILL )
			case ADD_SECONDARYSKILL:
			{
				// { ADD_SECONDARYSKILL, _T("/add2ndskill"), _T("Usage(/add2ndskill(SkillID)") },
				if(tokens.size() < 2)
					return false;

				int iSkillID = _ttoi(tokens[1].c_str());

				m_pSession->GetSecondarySkillRepository()->Create( iSkillID );
				break;
			}

			case DEL_SECONDARYSKILL:
			{
				// { DEL_SECONDARYSKILL, _T("/del2ndskill"), _T("Usage(/del2ndskill(SkillID)") },
				if(tokens.size() < 2)
					return false;

				int iSkillID = _ttoi(tokens[1].c_str());

				SecondarySkill::CSDelete TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.iSkillID = _wtoi(tokens[1].c_str());

				m_pSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_DELETE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				break;
			}
			case ADD_SECONDARYSKILL_EXP:
			{
				// { ADD_SECONDARYSKILL_EXP, _T("/add2ndskillexp"), _T("Usage(/add2ndskillexp(SkillID)(addexp)" },
				if(tokens.size() < 3)
					return false;

				int iSkillID	= _ttoi(tokens[1].c_str());
				int iAddExp		= _ttoi(tokens[2].c_str());

				m_pSession->GetSecondarySkillRepository()->AddExp( iSkillID, iAddExp );
				break;
			}
			case ADD_SECONDARYSKILL_RECIPE:
			{
				// { ADD_SECONDARYSKILL_RECIPE, _T("/add2ndskillrecipe"), _T("Usage(/add2ndskillrecipe(SkillID)(recipeinvenindex)" },
				if(tokens.size() < 3)
					return false;

				int iSkillID	= _ttoi(tokens[1].c_str());
				int iInvenIndex	= _ttoi(tokens[2].c_str());

				const TItem* pItem = m_pSession->GetItem()->GetInventory( iInvenIndex );
				if( pItem == NULL )
					break;

				SecondarySkill::CSAddRecipe TxPacket;
				TxPacket.iSkillID		= iSkillID;
				TxPacket.cInvenType		= ITEMPOSITION_INVEN;
				TxPacket.cInvenIndex	= static_cast<BYTE>(iInvenIndex);
				TxPacket.biInvenSerial	= pItem->nSerial;

				m_pSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_ADD_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				break;
			}
			case ADD_SECONDARYSKILL_RECIPE_EXP:
			{
				// { ADD_SECONDARYSKILL_RECIPE_EXP, _T("/add2ndskillrecipeexp"), _T("Usage(/add2ndskillrecipeexp(SkillID)(recipeItemID)(addexp)" },
				if(tokens.size() < 4)
					return false;

				int iSkillID	= _ttoi(tokens[1].c_str());
				int iItemID		= _ttoi(tokens[2].c_str());
				int iAddExp		= _ttoi(tokens[3].c_str());

				m_pSession->GetSecondarySkillRepository()->AddRecipeExp( iSkillID, iItemID, iAddExp );
				break;
			}
			case DEL_SECONDARYSKILL_RECIPE:
			{
				// { DEL_SECONDARYSKILL_RECIPE, _T("/del2ndskillrecipe"), _T("Usage(/del2ndskillrecipe(SkillID)(recipeItemID)" },
				if(tokens.size() < 3)
					return false;

				SecondarySkill::CSDeleteRecipe TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.iSkillID	= _ttoi(tokens[1].c_str());
				TxPacket.iItemID	= _ttoi(tokens[2].c_str());

				m_pSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_DELETE_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				break;
			}
			case EXTRACT_SECONDARYSKILL_RECIPE:
			{
				// { EXTRACT_SECONDARYSKILL_RECIPE, _T("/extract2ndskillrecipe"), _T("Usage(/extract2ndskillrecipe(SkillID)(recipeItemID)" },
				if(tokens.size() < 3)
					return false;

				SecondarySkill::CSExtractRecipe TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.iSkillID	= _wtoi(tokens[1].c_str());
				TxPacket.iItemID	= _wtoi(tokens[2].c_str());

				m_pSession->OnRecvSecondarySkillMessage( eSecondarySkill::CS_EXTRACT_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
				break;
			}
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

			case SET_PERIODQUEST:
			{
				if( tokens.size() <= 2 ) 
					return false;

				// Enable/disable flag value
				int nFlag = _wtoi(tokens[2].c_str());
				if( nFlag < 0 || nFlag > 1 )
					return false;

				bool bFlag;
				if(nFlag == 0) bFlag = false;
				else bFlag = true;

				// Get the time.
				time_t tStartDate = 0, tEndDate = 0;
				if(tokens.size() > 3)
				{
					char szDate[512] = {0,};
					ZeroMemory(szDate, sizeof(char)*512);
					WideCharToMultiByte( CP_ACP, 0, tokens[3].c_str(), -1, szDate, 512, NULL, NULL );

					std::vector<string> vCompleteDate;
					TokenizeA(szDate, vCompleteDate, ":");

					if(vCompleteDate.size() == PERIODQUEST_TIME_MAX)
					{
						CTimeSet tStartSet(vCompleteDate[PERIODQUEST_TIME_START].c_str(), true);
						CTimeSet tEndSet(vCompleteDate[PERIODQUEST_TIME_END].c_str(), true);

						tStartDate = tStartSet.GetTimeT64_LC();
						tEndDate = tEndSet.GetTimeT64_LC();
					}
				}

				int nWorldSetID;
#if defined(_VILLAGESERVER)
				nWorldSetID = g_Config.nWorldSetID;
#elif defined(_GAMESERVER)		
				nWorldSetID = m_pSession->GetWorldSetID();
#endif
				if(!g_pPeriodQuestSystem->SetActivateQuest(_wtoi(tokens[1].c_str()), bFlag, tStartDate, tEndDate)) 
					return false;
				
				break;
			}

			case SET_WORLDQUEST:
			{
				if( tokens.size() <= 4 ) 
					return false;

				int nScheduleID = _wtoi(tokens[1].c_str());		

				char szDate[512] = {0,};

				ZeroMemory(szDate, sizeof(char)*512);
				WideCharToMultiByte( CP_ACP, 0, tokens[2].c_str(), -1, szDate, 512, NULL, NULL );
				CTimeSet tCollectStartSet(szDate, true);

				ZeroMemory(szDate, sizeof(char)*512);
				WideCharToMultiByte( CP_ACP, 0, tokens[3].c_str(), -1, szDate, 512, NULL, NULL );
				CTimeSet tCollectEndSet(szDate, true);

				ZeroMemory(szDate, sizeof(char)*512);
				WideCharToMultiByte( CP_ACP, 0, tokens[4].c_str(), -1, szDate, 512, NULL, NULL );
				CTimeSet tRewardSet(szDate, true);
				
				if(!g_pPeriodQuestSystem->SetWorldEventTime(nScheduleID, tCollectStartSet.GetTimeT64_LC(), tCollectEndSet.GetTimeT64_LC(), tRewardSet.GetTimeT64_LC()))
					return false;

				break;
			}

			case SET_GUILDWAREVENT:
				{				
#if defined(_GAMESERVER)
					wstring wszString = FormatW(L"Guild war time cheat can only be used on the Village Server.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );

					return false;
#else
					if( tokens.size() < 5 ) 
						return false;

					int nIndex = 1;
					int nScheduleID = _wtoi(tokens[nIndex++].c_str());

					char szDate[512] = {0,};
					ZeroMemory(szDate, sizeof(char)*512);
					WideCharToMultiByte( CP_ACP, 0, tokens[nIndex++].c_str(), -1, szDate, 512, NULL, NULL );

					int nEventPeriod = _wtoi(tokens[nIndex++].c_str());
					int nReadyPeriod = _wtoi(tokens[nIndex++].c_str());

					CTimeSet tEventTime(szDate, true);
					if(!tEventTime.CheckIntegrity())
						return false;

					TAGetGuildWarSchedule ScheduleData;
					memset(&ScheduleData, 0, sizeof(ScheduleData));

					ScheduleData.wScheduleID = (short)nScheduleID;					

					for (int i=GUILDWAR_STEP_PREPARATION; i<GUILDWAR_STEP_END; i++)
					{
						//ScheduleData.EventInfo[i].cEventStep = i;
						ScheduleData.EventInfo[i].tBeginTime = tEventTime.GetTimeT64_LC();
						tEventTime.AddSecond(nEventPeriod*60);

						//if(i == GUILDWAR_STEP_REWARD)
						//{
						//	ScheduleData.tMainBeginDate = tEventTime.GetTimeT64_LC();
						//	tEventTime.AddSecond(nEventPeriod*60);
						//}

						ScheduleData.EventInfo[i].tEndTime	= tEventTime.GetTimeT64_LC();
						tEventTime.AddSecond(nReadyPeriod*60);
					}

					if( g_pMasterConnection && g_pMasterConnection->GetActive() )					
						g_pMasterConnection->SendSetGuildWarSchedule(ERROR_NONE, ScheduleData.wScheduleID, ScheduleData.wWinersWeightRate, ScheduleData.EventInfo, true, true, 0);
					break;
#endif //#if defined(_GAMESERVER)
				}
		case SET_PREWINGUILDUID:
		{		
			if( !m_pSession->GetGuildUID().IsSet() )
				return false;

			TGuildUID GuildUID = m_pSession->GetGuildUID();
#if defined( _VILLAGESERVER )
			g_pGuildWarManager->SetPreWinGuildUID( GuildUID );
			CDNGuildBase* pGuild = g_pGuildManager->At (GuildUID);
			if (!pGuild)
				break;

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
			if (FALSE == pGuild->IsEnable()) break;
#endif
			CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
			pGuildVillage->SendGuildWarPreWinGuild(true);
#else
			g_pMasterConnectionManager->SetPreWinGuildUID( m_pSession->GetWorldSetID(), GuildUID );
#endif // #if defined( _VILLAGESERVER )
			break;
		}
		case SET_GUILDWAR_FINALTIME:
		{
#if defined( _GAMESERVER )
			if( tokens.size() < 2 ) 
				return false;
			CTimeSet CurTime;
			CurTime.AddSecond( _wtoi(tokens[1].c_str()) );
			g_pMasterConnectionManager->SetGuildWarFinalStartTimeForCheat( m_pSession->GetWorldSetID(), CurTime.GetTimeT64_LC() );
#endif // #if defined( _GAMESERVER )
			break;
		}
		case SET_GUILDWARSCHEDULE_RELOAD :
			{
#if defined(_VILLAGESERVER)
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
					g_pMasterConnection->SendGuildWarScheduleReload();
#endif
				break;
			}
		case SET_TITLE:
			{
				if( tokens.size() < 2 ) 
					return false;

				int nIndex = _wtoi(tokens[1].c_str());
				if( nIndex-1 >= APPELLATIONMAX_BITSIZE )
					return false;

				m_pSession->GetAppellation()->RequestAddAppellation( nIndex-1 );
				break;
			}
		// { SET_LADDER_POINT, _T("/setladderpoint"), _T("Usage(/setladderpoint(1:1=1,2:2=2...etc)(LadderPoint)") },
		case SET_LADDER_POINT:
		{
			if( tokens.size() != 3 )
				return false;

			LadderSystem::MatchType::eCode MatchType = static_cast<LadderSystem::MatchType::eCode>(_ttoi( tokens[1].c_str() ));
			if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
				return false;

			m_pSession->GetDBConnection()->QueryModPvPLadderScoresForCheat( m_pSession, MatchType, _ttoi(tokens[2].c_str()), m_pSession->GetPvPLadderScoreInfoPtr()->GetGradePoint( MatchType ), m_pSession->GetPvPLadderScoreInfoPtr()->GetHiddenGradePoint( MatchType ) );
			break;
		}
		// { SET_LADDER_GRADEPOINT, _T("/setladdergradepoint"), _T("Usage(/setladdergradepoint(1:1=1,2:2=2...etc)(GradePoint)") },
		case SET_LADDER_GRADEPOINT:
		{
			if( tokens.size() != 3 )
				return false;

			LadderSystem::MatchType::eCode MatchType = static_cast<LadderSystem::MatchType::eCode>(_ttoi( tokens[1].c_str() ));
			if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
				return false;

			m_pSession->GetDBConnection()->QueryModPvPLadderScoresForCheat( m_pSession, MatchType, m_pSession->GetPvPLadderScoreInfoPtr()->iPvPLadderPoint, _ttoi(tokens[2].c_str()), m_pSession->GetPvPLadderScoreInfoPtr()->GetHiddenGradePoint( MatchType ) );
			break;
		}
		// { SET_LADDER_HIDDENGRADEPOINT, _T("/setladderhiddengradepoint"), _T("Usage(/setladderhiddengradepoint(1:1=1,2:2=2...etc)(HiddenGradePoint)") },
		case SET_LADDER_HIDDENGRADEPOINT:
		{
			if( tokens.size() != 3 )
				return false;

			LadderSystem::MatchType::eCode MatchType = static_cast<LadderSystem::MatchType::eCode>(_ttoi( tokens[1].c_str() ));
			if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
				return false;

			m_pSession->GetDBConnection()->QueryModPvPLadderScoresForCheat( m_pSession, MatchType, m_pSession->GetPvPLadderScoreInfoPtr()->iPvPLadderPoint, m_pSession->GetPvPLadderScoreInfoPtr()->GetGradePoint( MatchType ), _ttoi(tokens[2].c_str()) );
			break;
		}
		// { GET_LADDER_POINT, _T("/getladderpoint"), _T("Usage(/getladderpoint(1:1=1,2:2=2...etc)") },
		case GET_LADDER_POINT:
		{
			if( tokens.size() != 2 )
				return false;

			LadderSystem::MatchType::eCode MatchType = static_cast<LadderSystem::MatchType::eCode>(_ttoi( tokens[1].c_str() ));
			if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
				return false;

			WCHAR wszBuf[MAX_PATH];
			wsprintf( wszBuf, L"LadderPoint:%d LadderGradePoint=%d HiddenLadderGradePoint=%d", m_pSession->GetPvPLadderScoreInfoPtr()->iPvPLadderPoint, m_pSession->GetPvPLadderScoreInfoPtr()->GetGradePoint(MatchType), m_pSession->GetPvPLadderScoreInfoPtr()->GetHiddenGradePoint(MatchType) );

			m_pSession->SendDebugChat( wszBuf );

			break;
		}
		// { FORCE_LADDER_MATCHING, _T("/forcematching"), _T("Usage(/forcematching") },
		case FORCE_LADDER_MATCHING:
		{
#if defined( _VILLAGESERVER )
			LadderSystem::CRoom* pRoom = LadderSystem::CManager::GetInstance().GetRoomPtr( m_pSession->GetCharacterName() );
			if( pRoom )
				pRoom->ToggleForceMatching();
#else
			wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
			m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );	
#endif // #if !defined( _VILLAGESERVER )
			break;
		}

		case SET_SECOND_JOB_SKILL:
			{
#ifdef _VILLAGESERVER
				wstring wszString = FormatW(L"Cheat to temporarily obtain 2nd specialization skills can only be used on the Game Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );

				return false;
#else
				// Proceed depending on whether advancement to the 2nd job class ID is possible.
				if((int)tokens.size() < 2 )
					return false;

				int iJobID = _wtoi( tokens[1].c_str() );
				CDnPlayerActor* pPlayerActor = m_pSession->GetPlayerActor();
				if( pPlayerActor )
				{
					if( pPlayerActor->CanChangeJob( iJobID ) )
					{
						// IDs of 2nd specialization skills for each class
						switch( iJobID )
						{
							// Sword Master -> Gladiator
							case 23:
								{
									// Also send packet to client for temporary skill addition..
									pPlayerActor->AddTempSkill( 301 );	// Evasion Slash
									pPlayerActor->AddTempSkill( 302 );	// Triple Slash EX
									pPlayerActor->AddTempSkill( 303 );	// Finish attack
								}
								break;

							// Sword Master -> Warlord
							case 24:
								{
									pPlayerActor->AddTempSkill( 351 );	// Moon Blade Dance
									pPlayerActor->AddTempSkill( 352 );	// Moonlight Splinter EX
									pPlayerActor->AddTempSkill( 353 );	// Flash Stance
								}
								break;

							// Mercenary -> Barbarian
							case 25:
								{
									pPlayerActor->AddTempSkill( 501 );	// Append hit
									pPlayerActor->AddTempSkill( 502 );	// Stomp EX
									pPlayerActor->AddTempSkill( 503 );	// Bone Crash
								}
								break;

							// Mercenary -> Destroyer
							case 26:
								{
									pPlayerActor->AddTempSkill( 551 );	// Breaking point
									pPlayerActor->AddTempSkill( 552 );	// Flying Swing EX
									pPlayerActor->AddTempSkill( 553 );	// Maelstrom Howl
								}
								break;

							// Bow Master -> Sniper
							case 29:
								{
									pPlayerActor->AddTempSkill( 1301 );	// Critical break
									pPlayerActor->AddTempSkill( 1302 );	// Charge Shot EX
									pPlayerActor->AddTempSkill( 1303 );	// Cheating point
								}
								break;

							// Bow Master -> Artillery
							case 30:
								{
									pPlayerActor->AddTempSkill( 1351 );	// Magical Breeze
									pPlayerActor->AddTempSkill( 1352 );	// Extension Arrow EX
									pPlayerActor->AddTempSkill( 1353 );	// Detonating Arrow
								}
								break;

							// Acrobat -> Tempest
							case 31:
								{
									pPlayerActor->AddTempSkill( 1501 );	// Evade
									pPlayerActor->AddTempSkill( 1502 );	// Kick and Shot EX
									pPlayerActor->AddTempSkill( 1503 );	// Hurricane Dance
								}
								break;

							// Acrobat -> Windwalker
							case 32:
								{
									pPlayerActor->AddTempSkill( 1551 );	// Showtime
									pPlayerActor->AddTempSkill( 1552 );	// Spiral Kick EX
									pPlayerActor->AddTempSkill( 1553 );	// Rising Storm
								}
								break;

							// Elemental Lord -> Saleana
							case 35:
								{
									pPlayerActor->AddTempSkill( 2301 );	// Explosion
									pPlayerActor->AddTempSkill( 2302 );	// Fireball EX
									pPlayerActor->AddTempSkill( 2303 );	// Rolling Lava
								}
								break;

							// Elemental Lord -> Elestra
							case 36:
								{
									pPlayerActor->AddTempSkill( 2351 );	// Ice Barrier
									pPlayerActor->AddTempSkill( 2352 );	// Ice Sword EX
									pPlayerActor->AddTempSkill( 2353 );	// Icicle Injection }
								break;

							// Force User -> Smasher
							case 37:
								{
									pPlayerActor->AddTempSkill( 2501 );	// Arcane
									pPlayerActor->AddTempSkill( 2502 );	// Linear Ray EX
									pPlayerActor->AddTempSkill( 2503 );	// Orbital laser
								}
								break;

							// Force User -> Majesty
							case 38:
								{
									pPlayerActor->AddTempSkill( 2551 );	// Steal Magic
									pPlayerActor->AddTempSkill( 2552 );	// Gravity Ball EX
									pPlayerActor->AddTempSkill( 2553 );	// Switch Gravity
								}
								break;

							// Paladin -> Guardian
							case 41:
								{
									pPlayerActor->AddTempSkill( 3301 );	// Auto Block EX
									pPlayerActor->AddTempSkill( 3302 );	// Guardian Force
									pPlayerActor->AddTempSkill( 3303 );	// Justice Crash
								}
								break;

							// Paladin -> Crusader
							case 42:
								{
									pPlayerActor->AddTempSkill( 3351 );	// Judge's Power
									pPlayerActor->AddTempSkill( 3352 );	// Holy Cross EX
									pPlayerActor->AddTempSkill( 3353 );	// Judgment Hammer
								}
								break;

							// Priest -> Saint
							case 43:
								{
									pPlayerActor->AddTempSkill( 3501 );	// Holy Shield
									pPlayerActor->AddTempSkill( 3502 );	// Relic of Lightning EX
									pPlayerActor->AddTempSkill( 3503 );	// Shock of Relic
								}
								break;

							// Priest -> Inquisitor
							case 44:
								{
									pPlayerActor->AddTempSkill( 3551 );	// Shock Transition
									pPlayerActor->AddTempSkill( 3552 );	// Lightning Bolt EX
									pPlayerActor->AddTempSkill( 3553 );	// Consecration
								}
								break;

							// Engineer -> Shooting Star
							case 47:
								{
									pPlayerActor->AddTempSkill( 4301 );	// Splash
									pPlayerActor->AddTempSkill( 4302 ); // Ping Pong Bomb EX
									pPlayerActor->AddTempSkill( 4303 ); // Alfredo Beam
								}
								break;

							// Engineer -> Gear Master
							case 48:
								{
									pPlayerActor->AddTempSkill( 4351 ); // Chainsaw Tower
									pPlayerActor->AddTempSkill( 4352 );	// Mecha Duck EX
									pPlayerActor->AddTempSkill( 4353 ); // Big Mecha Bomber
								}
								break;

							// Alchemist -> Adept
							case 50:
								{
									pPlayerActor->AddTempSkill( 4501 ); // C2H5OH
									pPlayerActor->AddTempSkill( 4502 ); // Magma Punch EX
									pPlayerActor->AddTempSkill( 4503 ); // Ice Beam
								}
								break;

							// Alchemist -> Physician
							case 51:
								{
									pPlayerActor->AddTempSkill( 4551 ); // Love Virus
									pPlayerActor->AddTempSkill( 4552 ); // Injector EX
									pPlayerActor->AddTempSkill( 4553 );	// Heal shower
								}
								break;
							}
						}

						// Must first add temporary skills and send temporary job change to client so the skill tree displays properly on client
						// Gets initialized.
						pPlayerActor->SendTempJobChange( iJobID );
					}
				}
#endif
			}
			break;

		case CLEAR_SECOND_JOB_SKILL:
			{
#ifdef _VILLAGESERVER
				wstring wszString = FormatW(L"Cheat to temporarily obtain 2nd specialization skills can only be used on the Game Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );

				return false;
#else

				CDnPlayerActor* pPlayerActor = m_pSession->GetPlayerActor();
				if( pPlayerActor )
				{
					// Packet is also sent to client side.
					pPlayerActor->RemoveAllTempSkill();
					pPlayerActor->EndAddTempSkillAndSendRestoreTempJobChange();
				}
#endif
			}
			break;
		case MAKE_UNION_POINT:
			{
				if( tokens.size() <= 2 )
					return false;

				int nUnionType = _ttoi(tokens[1].c_str());

				BYTE cPointType = DBDNWorldDef::EtcPointCode::None;
				switch(nUnionType)
				{
				case NpcReputation::UnionType::Commercial:	
					cPointType = DBDNWorldDef::EtcPointCode::Union_Commercial;
					break;
				case NpcReputation::UnionType::Liberty:		
					cPointType = DBDNWorldDef::EtcPointCode::Union_Liberty;
					break;
				case NpcReputation::UnionType::Royal:		
					cPointType = DBDNWorldDef::EtcPointCode::Union_Royal;
					break;
				default:
					return false;
				}
				
				m_pSession->AddEtcPoint(cPointType, _ttoi(tokens[2].c_str()));

			}
			break;

		case CMDEQUIPTRIGGER:
		{
#if defined( _GAMESERVER )
			if( tokens.size() <= 2 )
				return false;

			api_trigger_InvenToEquip( m_pSession->GetGameRoom(), m_pSession->GetSessionID(), _ttoi(tokens[1].c_str()), _ttoi(tokens[2].c_str()) ? true : false );
#endif // #if defined( _GAMESERVER )
			break;
		}
		case CMDUNRIDE:
		{
#if defined( _GAMESERVER )
			api_trigger_UnRideVehicle( m_pSession->GetGameRoom(), m_pSession->GetSessionID() );
#endif // #if defined( _GAMESERVER )
			break;
		}

		case ITEMEXPIREDATE:
			{
				if( tokens.size()  <= 1 ) return false;
				INT64 biSerial = _wtoi64(tokens[1].c_str());				
				
				m_pSession->GetItem()->ItemExpireByCheat( biSerial );
			}
			break;

		case UPDATE_PET_EXP:
			{
				if( tokens.size() <= 1) return false;
				int nExp = _wtoi(tokens[1].c_str());
				m_pSession->GetItem()->UpdatePetExp(nExp);

			}
			break;

		case SET_PET_COLOR:
			{
				if( tokens.size()  <= 2 ) return false;
				int nType = _wtoi(tokens[1].c_str()); // Pet color change part
				int nColor = _wtoi(tokens[2].c_str());  // Color value
				if( 1 == nType )
					m_pSession->GetItem()->ChangePetBodyColor(&nColor);	//body color
				else
					m_pSession->GetItem()->ChangePetNoseColor(&nColor);	//tatoo color
			}
			break;

		case PERIODRESET:
			{
				if( tokens.size()  <= 1 ) return false;
				int nType = _wtoi(tokens[1].c_str()); // type

				// Set usage count to 0 
				m_pSession->ModCommonVariableData((CommonVariable::Type::eCode)nType, 0);
			}
			break;

		case MAKEGITEM:
			{
#ifdef _GAMESERVER
				if( tokens.size() < 2 ) return false;
				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( !pGameRoom ) return false;
				if( !pGameRoom->GetGameTask() ) return false;
				DNVector(CDnItem::DropItemStruct) VecList;
				CDnDropItem::CalcDropItemList( pGameRoom, pGameRoom->GetGameTask()->GetStageDifficulty(), _wtoi(tokens[1].c_str()), VecList );

				CDnItemTask *pTask = (CDnItemTask *)pGameRoom->GetTaskMng()->GetTask( "ItemTask" );
				if(!pTask) return false;
				for( DWORD j=0; j<VecList.size(); j++ ) {
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[j].dwUniqueID, *(m_pSession->GetActorHandle()->GetPosition()), VecList[j].nItemID, VecList[j].nSeed, 1, (short)( _rand(pGameRoom)%360 ), -1, VecList[j].nEnchantID);
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[j].dwUniqueID, *(m_pSession->GetActorHandle()->GetPosition()), VecList[j].nItemID, VecList[j].nSeed, 1, (short)( _rand(pGameRoom)%360 ));
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
				}
#endif
			}
			break;
		case FARM_GUILDWAR_FINAL:
		{
#if defined( _GAMESERVER )
			CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
			if( pGameRoom->bIsFarmRoom() )
			{
				static_cast<CDNFarmGameRoom*>(pGameRoom)->OnStartGuildWarFinal();
			}
#endif // #if defined( _GAMESERVER )
			break;
		}
		case DESTROYPVP:
			{
				if( tokens.size() < 2 ) return false;
#if defined(_VILLAGESERVER)
				g_pMasterConnection->SendForceDestroyPvP(m_pSession->GetAccountDBID(), _wtoi(tokens[1].c_str()), tokens.size() > 2 ? tokens[2].c_str() : NULL);
#endif		//#if defined(_VILLAGESERVER)
				break;
			}
		case CLEAR_MISSION :
			{
#if defined(_VILLAGESERVER)
				m_pSession->GetDBConnection()->QueryDelAllMissionsForCheat( m_pSession );					
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );	
#endif
			}
			break;
		case CLEAR_PRESENT :
			{
#if defined(_VILLAGESERVER)
				m_pSession->GetDBConnection()->QueryCheatGiftClear(m_pSession);
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );	
#endif
			}
			break;

		case SHOP_GETLIST_REPURCHASE:
			{
#if defined( _VILLAGESERVER )
				m_pSession->OnRecvTradeMessage( eTrade::CS_SHOP_GETLIST_REPURCHASE, NULL, 0 );
#endif // #if defined( _VILLAGESERVER )
				break;
			}

		case SHOP_REPURCHASE:
			{
#if defined( _VILLAGESERVER )
				if( tokens.size() < 2 ) 
					return false;
				CSShopRepurchase TxPacket;
				memset( &TxPacket, 0, sizeof(TxPacket) );

				TxPacket.iRepurchaseID = _wtoi(tokens[1].c_str());

				m_pSession->OnRecvTradeMessage( eTrade::CS_SHOP_REPURCHASE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
#endif // #if defined( _VILLAGESERVER )
				break;
			}

#if defined(PRE_ADD_QUICK_PVP)
		case QUICKPVP :
			{
				if( tokens.size() < 2)
					return false;
#if defined(_VILLAGESERVER)
				CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(tokens[1].c_str());
				if( pUser )
				{
					g_pMasterConnection->SendMakeQuickPvPRoom(m_pSession->GetAccountDBID(), pUser->GetAccountDBID());
				}					
#endif // #if defined(_VILLAGESERVER)					
			}
			break;
#endif //#if defined(PRE_ADD_QUICK_PVP)

		case PVPTIMEOVER:
			{
#if defined( _GAMESERVER ) && !defined( _FINAL_BUILD )
				CDNGameRoom* pGameRoom = m_pSession->GetGameRoom();
				if( pGameRoom->bIsPvPRoom() == false )
					break;
				CDNPvPGameRoom* pPvPGameRoom = static_cast<CDNPvPGameRoom*>(pGameRoom);
				if( pPvPGameRoom->GetPvPGameMode() )
					pPvPGameRoom->GetPvPGameMode()->SetForceTimeOver();
#endif // #if defined( _GAMESERVER )
				break;
			}

		case RESETNESTCOUNT:
			{
				m_pSession->InitNestClear(true);

			}
			break;
		case DEL_GUILDEFFECT:
			{
				if( tokens.size() < 2 ) 
					return false;
				int nItemID = _wtoi(tokens[1].c_str());
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) 
				{
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				TGuildRewardItem *RewardItemInfo;
				RewardItemInfo = pGuild->GetGuildRewardItem();
				for(int i = GUILDREWARDEFFECT_TYPE_EXTRAEXP; i < GUILDREWARDEFFECT_TYPE_CNT; i++)
				{
					if( RewardItemInfo[i].nItemID == nItemID )
					{
						m_pSession->GetDBConnection()->QueryCheatDelGuildEffectItem( m_pSession->GetDBThreadID(), m_pSession->GetWorldSetID(), m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), GuildUID.nDBID, nItemID );
						m_pSession->GetDBConnection()->QueryGetGuildRewardItem( m_pSession->GetDBThreadID(), GuildUID.nWorldID, 0, GuildUID.nDBID );
						return true;
					}
				}
				wstring wszString = FormatW(L"Effect does not exist.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
			}
			break;
		case DEL_GUILDMARK:
			{
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) 
				{
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				m_pSession->GetDBConnection()->QueryChangeGuildMark(m_pSession->GetDBThreadID(),  m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), m_pSession->GetGuildSelfView().GuildUID.nDBID, 
					m_pSession->GetWorldSetID(), 0, 0, 0);
			}
			break;
		case DENY_GUILDRECRUIT:
			{
#if defined(_VILLAGESERVER)
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) 
				{
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
				m_pSession->GetDBConnection()->QueryCheatDelGuildRecruit(m_pSession, GuildUID.nDBID, 1 );
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );	
#endif				
			}
			break;
		case CANCEL_GUILDAPPLY:
			{
#if defined(_VILLAGESERVER)				
				m_pSession->GetDBConnection()->QueryCheatDelGuildRecruit(m_pSession, 0, 2 );
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif
			}
			break;
		case RESET_GUILDPOINTLIMIT:
			{
				const TGuildUID GuildUID = m_pSession->GetGuildUID();
				if(!GuildUID.IsSet()) 
				{
					wstring wszString = FormatW(L"Not in a guild.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

				CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
				if(!pGuild) 
				{
					wstring wszString = FormatW(L"Guild info does not exist.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}

#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif

				m_pSession->GetDBConnection()->QueryModGuildCheat(m_pSession->GetDBThreadID(), pGuild->GetUID().nWorldID, pGuild->GetUID().nDBID, 2, 0, 0);
			}
			break;
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
		case PVP_VILLAGE_JOIN :
			{


			}
			break;
		case PVP_VILLAGE_LADDER :
			{

			}
			break;
#endif
#if defined(PRE_ADD_EXPORT_DPS_INFORMATION)
		case ENABLE_DPS_REPORT :
			{
#if defined(_GAMESERVER)
				if( tokens.size() < 2 ) 
					return false;
				
				bool bEnable = (_wtoi(tokens[1].c_str()) != 0) ? true : false;

				if(CDnDPSReporter::IsActive())
				{
					if(bEnable)
					{
						if( !CDnDPSReporter::GetInstance().IsEnabledUser(m_pSession->GetCharacterDBID()) )
						{
							if( CDnDPSReporter::GetInstance().EnableUser( m_pSession->GetCharacterDBID() ) == true )
							{
								CDnDPSReporter::GetInstance().ReportUserInfo( m_pSession->GetActorHandle() );
								wstring wszString;
								wszString = FormatW(L" Starting DPS recording for [%s] (User %d / %d) )" , m_pSession->GetCharacterName() , CDnDPSReporter::GetInstance().GetEnabledUserCount() , CDnDPSReporter::eReport::MaxUserCount );
								g_pMasterConnectionManager->SendNoticeFromClinet((WCHAR*)wszString.c_str(), (int)wszString.size()*sizeof(WCHAR) );
							}
							else
							{
								wstring wszString;
								wszString = FormatW(L" Server DPSREPORT user limit exceeded. (MAX %d) \n ( /reportdps 0 : Reset existing DPS users )" , CDnDPSReporter::eReport::MaxUserCount );
								m_pSession->SendChat( CHATTYPE_NOTICE, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
								return false;
							}
						}
					}
					else
					{
						CDnDPSReporter::GetInstance().DisableAllUser();
						wstring wszString;
						wszString = FormatW(L" Resetting all DPSREPORT users." );
						g_pMasterConnectionManager->SendNoticeFromClinet((WCHAR*)wszString.c_str(), (int)wszString.size()*sizeof(WCHAR) );
					}
				}
#else
				return false;
#endif	// #if defined(_GAMESERVER)
			}
			break;
#endif
#if defined( PRE_ADD_NAMEDITEM_SYSTEM ) 
		case GOLDBOX_NAMEDITEM:
			{
#if defined( _GAMESERVER )
				wstring wszString;

				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				int nItemID = _wtoi(tokens[1].c_str());
				if( nItemID != 0 )
				{
					TItemData *pItemData = g_pDataManager->GetItemData(nItemID);
					if (!pItemData)
					{
						wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
						return false;
					}
					else if( pItemData->cRank != ITEMRANK_SSS )
					{
						wszString = FormatW(L"Item rank does not match.!!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
						return false;
					}
				}
				
				m_pSession->SetGoldBoxItemID( nItemID );

				g_Log.Log(LogType::_NORMAL, m_pSession, L"SET_GOLDBOXITEMID : %d ", nItemID);
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Game Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif
			}
			break;

		case NAMEDITEM_COUNT:
			{
				wstring wszString;

				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				int nItemID = _wtoi(tokens[1].c_str());
				if( nItemID != 0 )
				{
					TItemData *pItemData = g_pDataManager->GetItemData(nItemID);
					if (!pItemData)
					{
						wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
						return false;
					}
					else if( pItemData->cRank != ITEMRANK_SSS )
					{
						wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
						return false;
					}
					m_pSession->GetDBConnection()->QueryCheatCheckNamedItemCount( m_pSession, pItemData->nItemID, 0 );
					//m_pSession->GetDBConnection()->quergetname( nItemID );
				}
				else
				{
					wszString = FormatW(L"Please enter the item ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}				
			}
			break;
#endif

#if defined(PRE_ADD_LIMITED_CASHITEM)
		case CHANGE_LIMITQUANTITY:
			{
#if defined( _VILLAGESERVER )
				wstring wszString;

				if( tokens.size() < 3 ){
					wszString = FormatW(L"Please enter the item serial and change value properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				int nItemSN = _wtoi(tokens[1].c_str());
				int nLimitMax = _wtoi(tokens[2].c_str());
				if( nItemSN != 0 )
				{
					TCashCommodityData CashData;
					if (!g_pDataManager->GetCashCommodityData(nItemSN, CashData))
					{
						wszString = FormatW(L"Please enter the item serial properly!!!\r\n");
						m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					}
					else
					{
						if (g_pCashConnection)
							g_pCashConnection->SendCheatChangeLimitMax(nItemSN, nLimitMax);
					}
				}
#else		//#if defined( _VILLAGESERVER )
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif		//#if defined( _VILLAGESERVER )
			}
			break;
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM)
#if defined (PRE_ADD_BESTFRIEND)
		case CLOSE_BESTFRIEND:
			{
#if defined( _VILLAGESERVER )
				wstring wszString;

				if(!m_pSession->GetBestFriend()->IsRegistered())
				{
					wszString = FormatW(L"Not in Best Friend status.!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				m_pSession->GetBestFriend()->CheckStatus(true);
#else
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
#endif
			}
			break;
#endif // #if defined (PRE_ADD_BESTFRIEND)
#if defined (PRE_ADD_RACING_MODE)
		case RACING_START :
			{
#if defined(_GAMESERVER)				
				if( m_pSession->GetGameRoom()->GetPvPGameMode()->bIsRacingMode() )				
					api_trigger_GetRacingStartTime(m_pSession->GetGameRoom());				
#endif
			}
			break;
		case RACING_END :
			{
#if defined(_GAMESERVER)
				if( m_pSession->GetGameRoom()->GetPvPGameMode()->bIsRacingMode() && tokens.size() == 2 )		
				{
					int nPartyIndex = _wtoi(tokens[1].c_str());
					//api_trigger_GetRacingGoalTime(m_pSession->GetGameRoom(), nPartyIndex);
				}
#endif
			}
			break;
#endif
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
		case SETPCCAFERENTITEM :
			{
#if defined(_VILLAGESERVER)
				UINT nUserID =  m_pSession->GetObjectID();
				api_user_SetPCCafeItem(nUserID);
#endif
			}
			break;
#endif
#ifdef PRE_MOD_PVPRANK
		case RANK:
			{
				BYTE cThreadID;
				CDNDBConnection *pDBCon = static_cast<CDNDBConnection*>(g_pDBConnectionManager->GetDBConnection(cThreadID));
				if (pDBCon)
				{
					pDBCon->QueryCalcPvPRank(cThreadID);
					wstring wszString = FormatW(L"Requested PvP Rank calculation. Please re-login.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
			}
			break;
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
		case PVP_RANK:
			{
				BYTE cThreadID;
				CDNDBConnection *pDBCon = static_cast<CDNDBConnection*>(g_pDBConnectionManager->GetDBConnection(cThreadID));
				if (pDBCon)
				{
					pDBCon->QueryAggregatePvPRank(m_pSession, false);
					wstring wszString = FormatW(L"Requested PvP Rank aggregation.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
			}
			break;
		case PVP_LADDER_RANK:
			{
				BYTE cThreadID;
				CDNDBConnection *pDBCon = static_cast<CDNDBConnection*>(g_pDBConnectionManager->GetDBConnection(cThreadID));
				if (pDBCon)
				{
					pDBCon->QueryAggregatePvPRank(m_pSession, true);
					wstring wszString = FormatW(L"Requested PvP Ladder Rank aggregation.\r\n");
					m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
					break;
				}
				wstring wszString = FormatW(L"This cheat can only be used on the Village Server.\r\n");
				m_pSession->SendChat( CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str() );
			}
			break;
#endif	//#if defined(PRE_ADD_PVP_RANKING)

#if defined(PRE_SPECIALBOX)
		case SPECIALBOX:
			{
				wstring wszString;

				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the mail table ID properly!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				int nMailID = _wtoi(tokens[1].c_str());
				if (nMailID > 0){
					CDNMailSender::Process(m_pSession, nMailID, DBDNWorldDef::PayMethodCode::Admin, nMailID);
				}				
			}
			break;
#endif	// #if defined(PRE_SPECIALBOX)

#if defined(PRE_ADD_REMOTE_QUEST)
		case GAIN_REMOTE_QUEST:
			{
				wstring wszString;
				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the quest ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nQuestID = _wtoi(tokens[1].c_str());

				m_pSession->GetQuest()->AddRemoteQuestToAcceptWaitList(nQuestID);
			}
			break;
		case ACCEPT_REMOTE_QUEST:
			{
				wstring wszString;
				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the quest ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nQuestID = _wtoi(tokens[1].c_str());

				m_pSession->GetQuest()->AcceptRemoteQuest(nQuestID);
			}
			break;			
		case COMPLETE_REMOTE_QUEST:
			{
				wstring wszString;
				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the quest ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nQuestID = _wtoi(tokens[1].c_str());
				m_pSession->GetQuest()->SetQuestToRecompenseState(nQuestID);
				m_pSession->SendCompleteRemoteQuest(nQuestID);
			}
			break;			
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
		case SET_STAGE_CLEAR_BOXNUM:
			{
				wstring wszString;
				if( tokens.size() > 2){
					wszString = FormatW(L"Please enter the item drop group number properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				m_pSession->SetBoxNumber(_wtoi(tokens[1].c_str()));
			}
			break;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD) && defined(_GAMESERVER)
		case ISGAINMISSION:
			{
				wstring wszString;
				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter the mission ID properly!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nMissionID = _wtoi(tokens[1].c_str());
				DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSION );
				int nTargetArrayIndex = pSox->GetArrayIndex( nMissionID );

				if( GetBitFlag(m_pSession->GetMissionData()->MissionGain, nTargetArrayIndex) )
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wcslen(L"Acquired!\r\n"), L"", (L"Acquired!\r\n"));
				else
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wcslen(L"Not acquired\r\n"), L"", (L"Not acquired\r\n"));
				
			}
			break;
#if defined( PRE_ADD_STAMPSYSTEM )
		case ADD_STAMP:
			{
				wstring wszString;
				if( tokens.size() < 3 )
				{
					wszString = FormatW(L"Invalid usage!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nChallengeIndex = _wtoi(tokens[1].c_str());
				int nWeekDay = _wtoi(tokens[2].c_str());

				if( 0 == nChallengeIndex || 
					StampSystem::Common::MaxChallengeCount < nChallengeIndex || 
					0 == nWeekDay ||
					StampSystem::Common::MaxWeekDayCount < nWeekDay )
				{
					wszString = FormatW(L"Input value is out of range!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				m_pSession->GetStampSystem()->ForceAddCompleteSlot( nChallengeIndex, nWeekDay );
			}
			break;
		case CLEAR_STAMP:
			{
				m_pSession->GetStampSystem()->ForceClearCompleteSlot();
			}
			break;
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_ADD_ACTIVEMISSION)
		case SET_ACTIVEMISSION:
			{
#if defined(_VILLAGESERVER)
				wstring wszString = FormatW(L"Please use this in the world map!!\r\n");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
#endif
#if defined(_GAMESERVER)
				wstring wszString;
				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please enter a valid Active Mission ID!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nActiveMissionID = _wtoi(tokens[1].c_str());

				if( g_pDataManager->GetActiveMissionData(nActiveMissionID-1) == NULL )
				{
					wstring wszString = FormatW(L"Invalid Active Mission ID.\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
				}				
				m_pSession->SetActiveMissionCheatID(nActiveMissionID);
#endif
			}
			break;
#endif
#if defined(PRE_ADD_CP_RANK)
		case INIT_STAGECP :
			{
				wstring wszString;

				if( tokens.size() < 2 ){
					wszString = FormatW(L"Please specify the reset type (0-All, 1-Legend, 2-Monthly, 3-Personal)\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}

				int nInitType = _wtoi(tokens[1].c_str());
				if( nInitType >= 0 && nInitType <= 3)
				{
					m_pSession->GetDBConnection()->QueryInitStageCP(m_pSession, (char)nInitType);
				}
				else
				{
					wszString = FormatW(L"Please enter a valid reset type!!!\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
			}
			break;		
#endif //#if defined(PRE_ADD_CP_RANK)
#if defined( PRE_PVP_GAMBLEROOM )
		case CREATE_GAMBLEROOM:
			{	
#if defined( _VILLAGESERVER )
				if( tokens.size() < 5 )
				{
					wstring wszString;
					wszString = FormatW(L"CreateGambleRoom Fail\r\n");
					m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
					return false;
				}
				int nRoomIndex = _wtoi(tokens[1].c_str());
				bool bRegulation = (_wtoi(tokens[2].c_str()) != 0) ? true : false;				
				int nGambleType = _wtoi(tokens[3].c_str());
				int nGamblePrice = _wtoi(tokens[4].c_str());				

				CDNPvPRoomManager::GetInstance().SetGambleRoomData(nRoomIndex, bRegulation, nGambleType, nGamblePrice, true );				
#endif
			}
			break;					
#endif
	}

	return true;
}

bool CDNCheatCommand::_bCommandCheatSet( std::vector<std::wstring>& tokens )
{
	WCHAR wszPath[MAX_PATH];
	GetCurrentDirectoryW( MAX_PATH, wszPath );
	const std::wstring wstrFileName = boost::io::str( boost::wformat( L"%s\\CheatSet.ini" ) % wszPath );

	if( tokens.size() != 2 )
	{
		m_pSession->SendChat( CHATTYPE_GM, static_cast<int>(wcslen(g_CmdList[CHEATSET].szComment)), L"", g_CmdList[CHEATSET].szComment );
		return false;
	}

	CIniFile cFile;
	cFile.Open( wstrFileName.c_str() );

	WCHAR wszBuf[MAX_PATH];

	for( UINT i=1 ; ; ++i )
	{
		std::wstring wstrValue = boost::io::str( boost::wformat( L"cmd%d" ) % i );

		cFile.GetValue( tokens[1].c_str(), wstrValue.c_str(), wszBuf );
		if( wcslen( wszBuf ) <= 0 )
			break;

		std::wstring wstrCheat = wszBuf;
		size_t result = wstrCheat.find( L";" );
		if( result != std::string::npos )
			boost::erase_tail( wstrCheat, static_cast<int>(wstrCheat.size()-result) );
		// Per design team request, continue executing cheat set even if a step fails midway.
		//if( Command( wstrCheat ) != 1 )
		//	return false;
		Command( wstrCheat );
	}

	return true;
}
