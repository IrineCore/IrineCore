#pragma once

#include "DNCommonDef.h"
#include "DNNotifierCommon.h"

/* -----------------------------------------------------------------------------
Struct here is for DB storage
Please create non-saved variables in each class as needed~
----------------------------------------------------------------------------- */

#pragma pack(push, 1)

struct TQuickSlot
{
	BYTE cType;		// See eQuickSlotType
	INT64 nID; 
};

#if !defined(PRE_DELETE_DUNGEONCLEAR)
struct TDungeonClearData
{
	int nMapIndex;	// Map ID
	char cType;		// Clear type? (eDungeonClearType)
};
#endif	// #if defined(PRE_DELETE_DUNGEONCLEAR)

struct TNestClearData
{
	int nMapIndex;
	char cClearCount;
	char cPCBangClearCount;
};

struct TCharacterStatus
{
	__time64_t tLastFatigueDate;
	__time64_t tLastEventFatigueDate;
	__time64_t tLastRebirthCoinDate;
	__time64_t tLastConnectDate;
	__time64_t tLastTimeEventDate;
	__time64_t tLastPeriodQuestDate[PERIODQUEST_RESET_MAX];

	char cAccountLevel;						// Account level for GM, developer, etc.
	WCHAR wszCharacterName[NAMELENMAX];		// Character name
	char cClass;							// Class (Warrior, Archer, Sorceress, Cleric)

	bool IsCheckFirstVillage;				// Check first village login after character creation

	int nDefaultBody;						// Equip set on initial creation (EQUIP_BODY, EQUIP_LEG, EQUIP_HAND, EQUIP_FOOT)
	int nDefaultLeg;
	int nDefaultHand;
	int nDefaultFoot;

	BYTE cLevel;							// Level
	int nExp;								// EXP
	BYTE cJob;								// Job

	DWORD dwHairColor;		// Hair color
	DWORD dwEyeColor;		// Eye color
	DWORD dwSkinColor;		// Skin color

	int nMapIndex;							// Current map ID
	int nLastVillageMapIndex;				// Last village map number
	int nLastSubVillageMapIndex;			// Last sub-village map number
	char cLastVillageGateNo;				// Last village gate number
	int nPosX;								// Current X coordinate
	int nPosY;								// Current Y coordinate
	int nPosZ;								// Current Z coordinate
	float fRotate;							// Rotate

	INT64 nCoin;							// Money
	INT64 nWarehouseCoin;					// Storage money

	BYTE cRebirthCoin;						// Coin count (different from Coin; coin used to revive upon death)
	BYTE cPCBangRebirthCoin;				// PC cafe coin count (different from Coin; coin used to revive on death)
	USHORT wCashRebirthCoin;				// Coin count purchased with cash (different from Coin; coin used to revive upon death)

	short wFatigue;							// Fatigue
	short wWeeklyFatigue;					// Weekly fatigue
	short wPCBangFatigue;					// PC cafe fatigue
	short wEventFatigue;
	short wVIPFatigue;

	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];	// 4 * 8 = 32 items

	BYTE cJobArray[JOBMAX];					// Class (0: starting class, rest sequentially including job advancements)
	DNNotifier::Data NotifierData[DNNotifier::RegisterCount::Total];
	TQuickSlot QuickSlot[QUICKSLOTMAX];		// Quick slot window (2 rows of 10)
#if !defined(PRE_DELETE_DUNGEONCLEAR)
	TDungeonClearData DungeonClear[DUNGEONCLEARMAX];	// Dungeon clear (120)
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)
	TNestClearData NestClear[NESTCLEARMAX];	// Nest clear
	int nDungeonClearCount;					// Dungeon clear count
	int nGlyphDelayTime;
	int nGlyphRemainTime;

	short wMarketRegisterCount;				// Remaining market listing count
	int nMarketSellSuccess;					// Market sale success count
	int nMarketSellFailure;					// Market sale failure count
	int nMarketRegisterTax;					// Market cumulative registration fee
	int nMarketSellTax;						// Market cumulative sales fee

	int nPetal;								// Reward points (Petal)

	char cSkillPage;						// Currently active skill page
#if defined( PRE_PRIVATECHAT_CHANNEL )
	INT64 nPrivateChatChannel;				// Current private chat room number
#endif
#if defined( PRE_ADD_STAMPSYSTEM )
	__time64_t tLastStampDate;				// Last time a record was logged to the stamp system
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	int nTalismanOpenFlag;
#endif
};

struct TDailyMission 
{
	int nMissionID;
	bool bAchieve;
	int nCounter;
};

struct TMissionGroup
{
	int nMissionScore;						// Mission score
	short wLastMissionAchieve[LASTMISSIONACHIEVEMAX];	// Final objective achieved(?)

	//Flags below are processed in 8-bit chunks from the leftmost bit (e.g. mission start value 0: 1000 0000, 1: 0100 0000)
	char MissionGain[MISSIONMAX_BITSIZE];		// Acquisition status
	char MissionAchieve[MISSIONMAX_BITSIZE];	// Goal achieved (on/off)

	TDailyMission DailyMission[DAILYMISSIONMAX];
	TDailyMission WeeklyMission[WEEKLYMISSIONMAX];
	TDailyMission WeekendEventMission[WEEKENDEVENTMISSIONMAX];
	TDailyMission GuildWarMission[GUILDWARMISSIONMAX];
	TDailyMission PCBangMission[PCBangMissionMax];
	TDailyMission GuildCommonMission[GUILDCOMMONMISSIONMAX];
	TDailyMission WeekendRepeatMission[WEEKENDREPEATMISSIONMAX];
#if defined(PRE_ADD_MONTHLY_MISSION)
	TDailyMission MonthlyMission[MONTHLYMISSIONMAX];
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

	__time64_t tDailyMissionDate;
	__time64_t tWeeklyMissionDate;
#if defined(PRE_ADD_MONTHLY_MISSION)
	__time64_t tMonthlyMissionDate;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
};

struct TAppellationGroup
{
	int nSelectAppellation;							// Selected title
	int nCoverAppellation;							// Cover title
	char Appellation[APPELLATIONMAX_BITSIZE];		// Title
};

struct TPvPGroup
{
	BYTE	cLevel;												// Level
	UINT	uiXP;												// XP
	UINT	uiPlayTimeSec;										// Playtime (seconds)
	UINT	uiWin;												// Victory
	UINT	uiLose;												// Defeat
	UINT	uiDraw;												// Draw
	UINT	uiGiveUpCount;										// Mid-game leave count
	UINT	uiKOClassCount[PvPCommon::Common::MaxClass];		// Number of times each class was KO'd
	UINT	uiKObyClassCount[PvPCommon::Common::MaxClass];		// Number of times KO'd by each class
	INT64	biTotalKillPoint;
	INT64	biTotalAssistPoint;
	INT64	biTotalSupportPoint;
	int nExpAbsoluteRank;		//Only top 101 rank values are sent. Separate from the actual rank displayed on the client.
	float fExpRateRank;			//Returns what top percentage users above a certain exp fall into
};

struct TItem
{
	int nItemID;
	INT64 nSerial;		// Serial
	int nRandomSeed;
	USHORT wDur;			// Durability
	short wCount;		// Count
	int nCoolTime;		// Remaining cooldown
	char cLevel;		// Level
	char cPotential;	// Potential
	bool bSoulbound;	// Whether unsealed (bind on equip)
	char cOption;
	char cSealCount;	// Seal count
	bool bEternity;		// Permanent item
	__time64_t tExpireDate;
 	int nLookItemID;
	int nLifespan;
	bool bExpireComplete;
	char cPotentialMoveCount;		//Move count: 0 if never moved, >= 0 is number of moves
	char cDragonJewelType;
};

struct TFarmWareHouseItem:public TItem
{
	INT64		biUniqueID;
	__time64_t	tMaterializeDate;
};

struct TQuestItem
{
	int nItemID;
	INT64 nSerial;	// Serial
	short wCount;	// Count
};

// 6 bytes of required item/monster counting info for quest
const int QUEST_EXTRACOUNT_MAX = 10;

struct TCount
{
	char cType;		// If cType is monster, monster index; if item, item index (EnumCountingType)
	int nIndex;
	unsigned short nCnt;
	unsigned short nTargetCnt;
};

const int ExtraSize = sizeof(TCount) * QUEST_EXTRACOUNT_MAX;

struct TQuest
{
	int				nQuestID;				// Quest index
	char			cQuestState;			// Quest state (EnumQuestState)
	short			nQuestStep;				// Current quest step
	char			cQuestJournal;			// Current quest journal
	__time64_t		tRegistDate;			// Registration time
	char			Extra[ExtraSize];		// TCount cnt[QUEST_EXTRACOUNT_MAX] // 210 bytes spare
	TP_QUESTMEMO	nMemo[QUESTMEMOMAX];	 // Memo (index 1~10, requested by design team)
};

const int MAX_QUEST_INDEX = 10000;
const int COMPLETEQUEST_BITSIZE = (MAX_QUEST_INDEX / CHAR_BIT);
#if defined(PRE_FIX_QUESTCOUNT)
const int MAX_PLAY_QUEST = 40;
#else	// #if defined(PRE_FIX_QUESTCOUNT)
const int MAX_PLAY_QUEST = 25;
#endif	// #if defined(PRE_FIX_QUESTCOUNT)
const int MAX_BROWSER_URL = 64;

struct TQuestGroup
{
	TQuest Quest[MAX_PLAY_QUEST];
	char CompleteQuest[COMPLETEQUEST_BITSIZE];
};

struct TSkill
{
	int nSkillID;		// Skill ID
	BYTE cSkillLevel;	// Skill level
	int nCoolTime;		// Cooldown
	bool bLock;			// Default is true.. likely not needed later.
};

struct TSkillGroup
{
	bool bResetSkill;						// Whether skill reset has been done (default false, true once done)
	USHORT wSkillPoint;						// Skill points
	TSkill SkillList[SKILLMAX];				// Skill
};

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
struct TEffectSkillData
{
	INT64 nItemSerial;
	int nItemID;
	int nSkillID;
	int nSkillLevel;
	__time64_t tExpireDate;	
	int nRemainTime;
	bool bEternity;	// false: timed, true: permanent
	bool bApplySkill;
};
#endif

struct TGesture
{
	int nGestureID;		//Gesture ID
};

struct TTimeEvent 
{
	int nItemID;
	INT64 nRemainTime;
	bool bCheckFlag;
};

struct TTimeEventGroup
{
	TTimeEvent Event[TIMEEVENTMAX];
};

struct TUserData
{
	TCharacterStatus Status;
	TPvPGroup PvP;	
	TSkillGroup Skill[DualSkill::Type::MAX];	
	TQuestGroup	Quest;
	TAppellationGroup Appellation;
	TMissionGroup Mission;
	TTimeEventGroup TimeEvent;
};

struct TMasterSystemOppositeInfo
{
	INT64 CharacterDBID;
	WCHAR wszCharName[NAMELENMAX];
	int	nFavorPoint;
};

struct TMasterSystemSimpleInfo
{
	int	iMasterCount;
	int	iPupilCount;
	int	iGraduateCount;
	__time64_t BlockDate;
	BYTE cCharacterDBIDCount;
	TMasterSystemOppositeInfo OppositeInfo[MasterSystem::Max::OppositeCount];
};

struct TMasterSystemData
{
	TMasterSystemSimpleInfo SimpleInfo;
};

struct TRestraint
{
	UINT nRestraintKey;
	int nRestraintType;		//eRestraintType
	__time64_t _tBegineTime;
	__time64_t _tEndTime;
	WCHAR wszRestraintReason[RESTRAINTREASONMAX];
};

struct TRestraintData
{
	TRestraint Restraint[RESTRAINTMAX];
};

struct TChracterLevel
{
	INT64 biCharacterDBID;
	BYTE cLevel;
};

struct TUnionMembership
{
	BYTE cType;
	int nItemID;
	__time64_t tExpireDate;
};

struct ItemCount
{
	int id;
	int count;
};

#pragma pack(pop)
