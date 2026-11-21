#pragma once

class CDNUserSession;

namespace EventSystem
{
	enum EventTypeEnum {	
		OnNone,
		OnLevelUp,				// V(o) G(o)
		OnDungeonEnter,			// V(o) G(o)
		OnDungeonClear,			// V(x) G(o)
		OnItemGain,				// V(o) G(o)
		OnItemUse,				// V(o) G(o)
		OnItemEquip,			// V(o) G(o)
		OnItemCompound,			// V(o) G(x)
		OnItemDisjoint,			// V(x) G(o)
		OnItemEnchantSuccess,	// V(o) G(o)
		OnItemEnchantFailed,	// V(o)	G(o)	// 10
		OnItemRepair,			// V(o) G(o)
		OnNpcTalk,				// V(o) G(o)
		OnMissionClear,			// V(o) G(o)
		OnQuestClear,			// V(o) G(o)
		OnAppelationGain,		// Unused
		OnRebirth,				// V(x) G(o)
		OnDie,					// V(x) G(o)
		OnRebirthAny,			// V(x) G(o)
		OnKillMonster,			// V(x) G(o)
		OnGuildCreate,			// V(o) G(x)	// 20
		OnGuildJoin,			// V(o) G(x)
		OnJobChange,			//
		OnAirshipRiding,		// Unused
		OnSocialAction,			// V(o) G(o)
		OnMarketBuy,			// V(o) G(x)
		OnMarketSell,			// V(o) G(x)
		OnExchange,				// V(o) G(o)
		OnPvPVictory,			// V(x) G(o)
		OnPvPLoss,				// V(x) G(o)
		OnCoopClear,			// Unused // 30
		OnCoopFailed,			// Unused
		OnSkillUse,				// V(x) G(o)
		OnSkillLevelUp,			// V(o) G(o)
		OnSkillAdd,				// V(o) G(x)
		OnWarehouseKeep,		// V(o) G(x)
		OnPvPFinished,			// V(x) G(o)
		OnKillPlayer,			// V(x) G(o)
		OnQuestGain,			// V(o) G(o)
		OnDungeonGiveup,		// V(x) G(o) // When abandoning stage by pressing F9
		OnFriendFull,			// V(o) G(o)	// 40
		OnDarklairClearRound,	// V(x) G(o)
		OnPvPKillCaptain,		// V(x) G(o)
		OnNpcReputaionChange,	// V(o) G(o)
		OnCashShopOpen,			// V(x) G(o) // When opening Cash Shop
		OnMasterSystemGraduate, // V(o) G(o) // 45) Apprentice graduation
		OnMasterSystemGraduated,// V(o) G(o) // 46) Graduate an apprentice
		OnMasterSystemJoin,		// V(o) G(o) // 47) Form master-apprentice relationship
		OnPvPEventFinished,		// V(x) G(o) // 48) Clear PvP event room
		OnKillMonster2,			// V(x) G(o) // 49) This KillMonster does not check all conditions such as whether the killing blow was by a user.
		OnDailyFatigue,							// 50) On daily FTG consumption
		OnWeeklyFatigue,						// 51) On weekly FTG consumption
		OnPCBangFatigue,						// 52) On PC bang FTG consumption
		OnVIPFatigue,							// 53) On VIP FTG consumption 
		OnCountingDungeonClear,					// 54) On consecutive dungeon clear
		OnDailyMissionClear,					// 55) On daily mission clear
		OnWeeklyMissionClear,					// 56) On weekly mission clear
		OnPCBang_Item = 57,						// 57) If PC bang - grant item
		OnPCBang_Fatigue = 58,					// 58) If PC bang - grant FTG
		OnPCBang_RebirthCoin = 59,				// 59) If PC bang - grant revival stone
		OnRoundFinished = 60,	// V(x) G(o) // 60) When PvP round ends
		OnOccupyArea = 61,		// V(x) G(o) // 61) Capture stronghold
		OnStealArea = 62,		// V(x) G(o) // 62) Contest stronghold
		OnTrigger = 63,			// V(x) G(o) // 63) When trigger activates
		OnLadderFinished = 64,	// V(x) G(o) // 64) Clear ladder
		OnPetLevelUp = 65,		// V(o) G(o) // 65) When pet levels up
		OnGiftToCadger = 66,					// 66) On gift request
		OnSetRevengeTarget = 67,				// 67) When a revenge target is added
		OnPvPLevelChange = 68,					// 68) On PvP rank change
		OnHPChanged = 69,		// V(x) G(o) // 69) When HP changes
		OnMPChanged = 70,		// V(x) G(o) // 70) When MP changes
		OnPetExpired = 71,		// V(o) G(o) // When equipped pet expires
		OnActiveMissionClear = 72, // V(x) G(o) // 72) When active mission is cleared
		OnChatMission = 73,						// 73) On chat mission clear
		EventTypeEnum_Amount,
	};

	enum EventValueTypeEnum {		
		None,
		PlayerLevel = 1,	// Player level
		PartyCount = 2,		// Number of party members
		MapID = 3,			// Map ID
		MapLevel = 4,		// Map Level
		InvenCoin = 5,		// Inventory gold
		WarehouseCoin = 6,	// Money in warehouse
		JobID = 7,			// Current job ID
		ClassID = 8,		// Current class ID
		MonsterID = 9,		// Monster ID
		NpcID = 10,			// NPC ID
		SkillID = 11,		// Skill ID
		SkillLevel = 12,	// Skill Level
		ItemID = 13,		// Item ID
		StageClearRank = 14,	// Stage clear rank
		StageClearTime = 15,	// Stage clear time
		MaxComboCount = 16,		// Max combo count on stage clear
		UseCoinCount = 17,		// Money spent on trading, repairs, etc.
		EnchantLevel = 18,		// Enhancement level on enhance
		TotalDamageCount = 19,	// Hit count taken on stage clear
		MissionID = 20,			// Achieved mission ID
		DieCount = 21,			// Death count
		GenocideCount = 22,		// How many monsters were hunted simultaneously.
		PartyMemberDieCount = 23,// Party member death count
		KOClassIDinPvP = 24,		// PvP info. Kill class ID
		KOClassCountinPvP = 25,	// PvP info. Kill class count
		KObyClassIDinPvP = 26,	// PvP info. Death class ID
		KObyClassCountinPvP = 27,// PvP info. Death class count
		PvPWin = 28,
		PvPLose = 29,
		AllPartsMinLevel = 30,	// Minimum enhancement level across 5 equipment pieces
		QuestID = 31,			// Quest ID
		DarkLairRoundCount = 32,
		SocialActionID = 33,		// Gesture ID
		KOCountinPvP = 34,		// PvP info. Kill count
		KObyCountinPvP = 35,		// PvP info. Death count
		StartPartyCount = 36,	// Party member count at stage start
		NpcReputaionFavor = 37,
		NpcReputaionMalice = 38,		
		MasterCount = 39,		// Master count
		PupilCount = 40,			// Number of pupils
		GraduateCount = 41,		// Number of graduates
		PartyMasterCount = 42,	// Number of masters in party
		PartyPupilCount = 43,	// Number of apprentices in party
		PartyClassmateCount = 44,// Number of peers in party
		MapSubType = 45,			// Map SubType
		DungeonClearCount = 46,	// Dungeon clear count
		DailyFatigue = 47,		// Daily fatigue
		WeeklyFatigue = 48,		// Weekly fatigue 
		PCBangFatigue = 49,		// PC bang fatigue
		VIPFatigue = 50,			// VIP fatigue
		ItemType = 51,			// Item Type
		PCBangGrade = 52,	// PC bang grade (1-Normal 2-Red 3-Silver 4-Gold) PCBang::Grade
		PvPGameMode = 53,	// PvP game mode (0-Respawn, 1-Round, 2-Captain, 3-Individual, 11-Zombie, 12-GuildWar, 13-Occupation)
		PvPTeam		= 54,	// Check PvP Team (A = 1000, B = 1001, Human = 1000, Zombie = 1001)
		PvPRoundCount = 55,	// PvP round count 
		PvPWinCondition=56,	// PvP win condition (1-Timeout, 2-All opponents left, 3-All opponents dead, 4-Opponent captain left, 5-Opponent captain dead..etc.
		HolyWaterUse=57,	// Ghoul holy water usage count
		GhoulModeWin=58,	// Ghoul mode win count
		BossKillCount=59,	// Boss kill count
		PvPModePlayCount = 60,	// Play count per PvP game mode
		PvPModeWinCount = 61,	// Win count per PvP game mode
		PvPModeOccupyCount = 62,// Occupation count per PvP game mode
		PvPModeStealCount = 63,// Capture count per PvP game mode
		PvPModeKillCount = 64,// Kill count per PvP game mode
		MonsterGrade = 65,	// Monster grade
		MonsterRaceID = 66,	// Monster race ID
		DiffMapToUserLevel= 67,	// Difference between map recommended level and user level (absolute value)
		GuildPartyCount= 68,	// Number of guild members in party
		PetLevel=69,			// Highest level among owned pets
		PartyMasterAndPupilCount = 70, // Number of masters + apprentices in party
		PartyType = 71,		//Party type (see ePartyType values)
		PvPRevengeSuccess = 72, //Whether revenge succeeded on kill or die in PvP mode
		PvPLevel = 73, //PvP rank
		PvPTournamentWin = 74,	// PvP tournament win (win, lose)
		PvPTournamentType = 75, // PvP tournament type (Round of 32, Round of 16, Quarterfinals, Semifinals)
		PvPTournamentAdvance = 76, // PvP tournament advance (Round of 16, Quarterfinals, Semifinals, Finals)		
		ItemCount = 77,	// Item count (processed with this item ID only in OnItemGain event)
		PlayerHpPercent = 78,	// Player HP
		PlayerMpPercent = 79,	// Player MP
		TotalRebirthCount = 80, // Remaining resurrection count
		SkillPoint = 81, // Remaining skill points in the currently selected tree
		PvPWorldCombine = 82, // Cross-world battlefield
		ActiveMissionID = 83, // Active mission ID (Event Variable)
		UIStringMID = 84,	//MID in UIString.xml
		EventValueTypeEnum_Amount,
	};
};

class CDNEventSystem:public TBoostMemoryPool<CDNEventSystem>
{

public:
	CDNEventSystem( CDNUserSession *pUser );
	virtual ~CDNEventSystem();

public:	

	struct EventValueFuncStruct {
		EventSystem::EventValueTypeEnum Type;
		int(_cdecl *pFunc)( CDNUserSession *pSession );
	};
	static EventValueFuncStruct s_EventValueFuncList[];

protected:
	CDNUserSession *m_pUserSession;
	
	int m_nEventParamValue[EventSystem::EventValueTypeEnum_Amount];

public:
	void RegisterEventParam( EventSystem::EventValueTypeEnum Index, int nValue );
	int GetEventParam( EventSystem::EventValueTypeEnum Index );

	bool OnEvent( EventSystem::EventTypeEnum Event );
	bool OnEvent( EventSystem::EventTypeEnum Event, int nParamCount, ... );

	bool CheckEventCondition( const TEventCondition *pEventCondition );
};
