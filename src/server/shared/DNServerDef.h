#pragma once

#include "DNPacket.h"
#include "MemPool.h"

/*--------------------------------------------------------------------------
Everything used on the Server side (?)
--------------------------------------------------------------------------*/

// const ----------------------------------------------------------------------------------------------

// SQL credentials and DSN-less connection settings are configured in Config/odbc.ini.


const char DBSERVERMAX			= 16;
const char THREADMAX			= 8;	// DB THREAD
const char LOGINTIMELENMAX		= 20;	// last login time length max
const char TIMELENMAX = 20;				// DateTime type

const int LOGINCOUNTMAX				= 64;			// Total Login server count
const int LOGINCONCOUNTINFOTERM		= (1*30*1000);	// Throttle interval for reporting login server concurrent user info to master server (unit: ms)
const int LOGINSERVERLISTSORTTERM	= (1*3*1000);	// Throttle interval for sorting login server sessions (unit: ms)
const int MASTERCOUNTMAX			= 10;			// Total Master server count

const int  DELETECHAR_WAITTIME_MINUTE	= 7*24*60;	// 7 days
const BYTE PROCESSCOUNT = 40;	// Objects processed per second

const int INTERNALBUFFERLENMAX = 51200;

const UINT PROCESSBUFFERLENMAX = 1024 * 64;
const UINT WORKERBUFFERLENMAX = 1024 * 64;

const int QUESTSIZEINFO = 2775;

const int QUERYOVERTIME = 1000;

const int QUERYNAMESIZE = 200; // Max query size
const int GAMEDELAYSIZE = 200; // Max size of game delay log

const int SERVERDETACHPACKETSIZE = 1024*40;	// Buffer size when sending packet
//const variable.
#ifdef _FINAL_BUILD
const unsigned long RUDP_CONNECT_TIME_LIMIT_FOR_PARTY = 120 * 1000;								//Initial game server connect wait time
const unsigned long RUDP_CONNECT_TIME_LIMIT_FOR_SINGLE = 120 * 1000;
const unsigned long WAIT_FOR_ANOTHER_USER_TIME_LIMIT = 15 * 1000;						//Wait time for other players after the first player connects
const unsigned long WAIT_FOR_LOAD_TIME_LIMIT = 60 * 1000;								//Time to wait for database loading
const unsigned long GOGO_SING_TO_PLAY_FOR_WAIT_TIME_LIMIT = 2 * 60 * 1000;					//Wait time for client loading and sync
const unsigned long GOGO_SING_TO_FARMPLAY_FOR_WAIT_TIME_LIMIT = 5 * 60 * 1000;			//Farm loading time is handled slightly separately
#else
const unsigned long RUDP_CONNECT_TIME_LIMIT_FOR_PARTY = 60 * 1000;								//Initial game server connect wait time
const unsigned long RUDP_CONNECT_TIME_LIMIT_FOR_SINGLE = 120 * 1000;
const unsigned long WAIT_FOR_ANOTHER_USER_TIME_LIMIT = 60 * 1000;						//
const unsigned long WAIT_FOR_LOAD_TIME_LIMIT = 150 * 1000;								//
const unsigned long GOGO_SING_TO_PLAY_FOR_WAIT_TIME_LIMIT = 5 * 60 * 1000;				//
const unsigned long GOGO_SING_TO_FARMPLAY_FOR_WAIT_TIME_LIMIT = 5 * 60 * 1000;			//Farm loading time is handled slightly separately
#endif

const int EACH_GAMESERVER_MAX_ROOM_COUNT = 20;
const int EACH_GAMESERVER_MAX_USER_COUNT = 100;

const int INTERVALDELETE_TIMINGTICK = 1000 * 60;		//About 1 minute?
const int CHECK_ZOMBIE_DISCONNECT_TICK = 1000 * 60 * 5;
const int DISTRIBUTE_IDLE_TICK = 1000 * 5;

const int DELAYED_PRCESS_CHECKTICK = 1000 * 10;
const int DELAYED_PRCESS_LOG_COUNT = 10;
const int FRAMEDEADLINE = 17;

const int EVENTMAPSTRMAX = 50;

const int CHECKLIVETICK = 60000 * 2;//  * 5; // HackShield check every 5 min -> changed to 1 min as requested -> increased to 2 min
const int CHECKRESPONSETICK = 60000 * 3;		// HackShield response timeout (about 3 minutes?)

const int CHECKGPKFIRSTTICK = 1000 * 60 * 2;	// Initial check timing
const int CHECKGPKTICK = 60000;
const int MANAGEDMAX = 20;

#if !defined(_FINAL_BUILD)
const DWORD CHECK_TCP_PING_TICK	= 1000*30*1;	// 1 minute
const DWORD CHECK_UDP_PING_TICK = 1000*30*1;	// 1 minute
#else		//#if !defined(_FINAL_BUILD)
const DWORD CHECK_TCP_PING_TICK	= 1000*60*1;	// 1 minute
const DWORD CHECK_UDP_PING_TICK = 1000*60*1;	// 1 minute
#endif		//#if !defined(_FINAL_BUILD)

const int CHECKLOGINUSERKICKTICK = 15 * 60 * 1000;		// Kick if staying on login server for 15 minutes!
const int CHECKNOTAUTHLOGINUSERKICKTICK = 5 * 60 * 1000;  // Kick if staying unauthenticated on login server for 5 minutes!

const unsigned int DEFAULTUSERSESSIONID = 10000;

const unsigned int CASHDELUSERDELAY_LIMITTICK = (4*30*1000);

// Counts related to DB batch jobs
const int DBQUERYSTREAMDATAMAX			= (36*1024);	// Max DB batch job data size : Set based on the largest data struct queued for batch processing instead of immediate send to DB server -> may need ongoing readjustment if insufficient
const unsigned int DBQUERYAUTOCHKSUM	= 0xE41C723F;	// Check value when serializing DB queries

const int QUESTINVENBLANKCHECKMIN	= 2;				// Minimum count at which a message is sent during quest inventory check
const float MAX_ITEMPRICE_RATE = 0.5f;

#if defined(_FINAL_BUILD)
const int BEGINAUTHLIMITSEC			= (9*10);			// Initial auth timeout (unit:sec, <= 0 means wait indefinitely?)
const int CHECKAUTHLIMITSEC			= (6*10);			// Normal auth timeout (unit: sec)
const int BEGINAUTHLIMITTERM		= (BEGINAUTHLIMITSEC*1000);		// Initial auth limit interval (unit:ms)
//const int CHECKAUTHLIMITTERM = (CHECKAUTHLIMITSEC*1000); // Normal auth limit interval (unit: ms)
const int CHECKAUTHLIMITTERM		= (1*30*1000);		// Normal auth limit interval (unit: ms) // P.S.> Set shorter than before considering inconvenience for dungeon/PVP participants
#else	// _FINAL_BUILD
const int BEGINAUTHLIMITSEC			= (6*10);			// Initial auth timeout (unit:sec, <= 0 means wait indefinitely?)
const int CHECKAUTHLIMITSEC			= (4*10);			// Normal auth timeout (unit: sec)
const int BEGINAUTHLIMITTERM		= (BEGINAUTHLIMITSEC*1000);		// Initial auth limit interval (unit:ms)
//const int CHECKAUTHLIMITTERM = (CHECKAUTHLIMITSEC*1000); // Normal auth limit interval (unit: ms)
const int CHECKAUTHLIMITTERM		= (1*20*1000);		// Normal auth limit interval (unit: ms) // P.S.> Set shorter than before considering inconvenience for dungeon/PVP participants
#endif	// _FINAL_BUILD

const int RESETAUTHLISTMAX			= (1000);			// Auth info list reset size (WARNING! Packets using this global constant must not exceed send/recv packet size)
const int RESETAUTHLISTTERM			= (1*1000);			// Auth info list reset interval (unit: ms)
const int RESETAUTHSERVERTERM		= (5*1000);			// Auth info server reset interval (unit: ms)
const int RESETAUTHWORLDCHECKTERM	= (5*1000);			// Auth info world reset interval (unit: ms)
const int RESETAUTHWORLDLIMITTERM	= (2*30*1000);		// Auth info world reset timeout (unit: ms)

const int GETAUTHUSERCOUNT = 1000 * 10;					// Auth world user count fetch interval

const int GUILDMGRCHKTERM = (5*1000);					// Guild manager check interval
const int GUILDMGRUPDTERM = (10*1000);					// Guild manager update interval
const int GUILDCHKMEMBTERM = (30*1000);					// Guild member info check interval
const int GUILDCHKRESCTERM = (30*1000);					// Guild resource return check interval
const int GUILDREQINFOTERM = (60*1000);					// Guild info request check interval
const int GUILDRFSVIEWTERM = (10*1000);					// Guild visual info update interval
const int GUILDAPLPOINTTERM = (10*1000);				// Guild normal/ladder point DB apply interval
const int GUILDCHKRESCLIMIT = (2*30*1000);				// Guild resource return check limit

const int PERIODQUESTTERM = (60*1000);					// Timed quest renewal cycle

const int PERIODGUILDWARTERM = (60*1000);				// Guild War refresh interval
const int GUILDWARMISSION_MAXGROUP = 4;

const int SERVICEMANAGER_SERVERDEALY_SIZE = 1000;
const int SERVICEMANAGER_REPORT_SIZE = 4096;

#define SWAP16(x) (((((x) & 0xff) << 8) | (((x) >> 8) & 0xff)))
#define SWAP32(x) (((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24)))
#define SWAP64(x) (((((x) & 0xFF00000000000000ULL) >> 56) | (((x) & 0x00FF000000000000ULL) >> 40) | (((x) & 0x0000FF0000000000ULL) >> 24) | (((x) & 0x000000FF00000000ULL) >>  8) | \
	(((x) & 0x00000000FF000000ULL) <<  8) | (((x) & 0x0000000000FF0000ULL) << 24) | (((x) & 0x000000000000FF00ULL) << 40) | (((x) & 0x00000000000000FFULL) << 56)))

struct DNProcessCheck
{
	enum eDNProcessCheck
	{
		Key					= 0x00000001,
		DisconnectInterval	= 60*1000*2,
		UpdateInterval		= 10*1000,
		HeartbeatKey		= 0x00000002,
		SQLHeartbeatInterval = 60*1000*10,
		SelectIPMagicInterval = 60*1000*60,
	};
};

// enum ----------------------------------------------------------------------------------------------
enum eServerType
{
	SERVERTYPE_LOGIN,		// Login server
	SERVERTYPE_DB,			// DB server
	SERVERTYPE_VILLAGE,		// Village server
	SERVERTYPE_GAME,		// Game server
	SERVERTYPE_MASTER,		// Master server
	SERVERTYPE_LOG,			// Log server
	SERVERTYPE_MAX,
};

enum ePatchType
{
	PATCHTYPE_LOGIN,		// Login server
	PATCHTYPE_DB,			// DB server
	PATCHTYPE_VILLAGE,		// Village server
	PATCHTYPE_GAME,			// Game server
	PATCHTYPE_MASTER,		// Master server
	PATCHTYPE_LOG,			// Log server
	PATCHTYPE_PATCHER,		// ESM
	PATCHTYPE_CASH,			// Cash server
};

enum eConnectionKey
{
	CONNECTIONKEY_DEFAULT = 0,
	CONNECTIONKEY_USER,
	CONNECTIONKEY_LOGIN,
	CONNECTIONKEY_DB,
	CONNECTIONKEY_VILLAGE,
	CONNECTIONKEY_GAME,
	CONNECTIONKEY_MASTER,
	CONNECTIONKEY_LOG,
	CONNECTIONKEY_AUTH,
	CONNECTIONKEY_CASH,
	CONNECTIONKEY_BILLING,
	CONNECTIONKEY_DOORS,
};

enum eAcceptOpenVerifyType
{
	ACCEPTOPEN_VERIFY_TYPE_APPINITCOMPLETE = 0,
	ACCEPTOPEN_VERIFY_TYPE_MASTERCONNECTION,
	ACCEPTOPEN_VERIFY_TYPE_RESETAUTH,
	ACCEPTOPEN_VERIFY_TYPE_MAX,
};

enum eServerLogLevel
{
	ServerLogLevel_1 = 1,
	ServerLogLevel_2 = 2,
	ServerLogLevel_3 = 3,
};

enum eUserState		// State value used in login
{
	STATE_NONE,

	// login
	STATE_CHECKVERSION,
	STATE_CHECKLOGIN,		// ID/password verification state
	STATE_SERVERLIST,
	STATE_WAITUSER,			// Waiting user
	STATE_CHARLIST,			// Character list state
	STATE_CHANNELLIST,
	STATE_CONNECTVILLAGE,	// Ready to connect to village server
	STATE_CONNECTGAME,		// Ready to connect to game server
	STATE_RECONNECTLOGIN,	// Reconnect state

	// master
	STATE_CHECKVILLAGE,		//State before moving to village. Managed via this state since user becomes orphaned if disconnected.
	STATE_CHECKGAME,		//State before transitioning to game. Same as above
	STATE_CHECKRECONNECTLOGIN,	//State before moving to character selection screen. Same as above
	STATE_VILLAGE,			// Whether in village state
	STATE_GAME,				// Whether in game state
	STATE_LOGIN,			// Is in login state

	// village
	STATE_LOADUSERDATA,		// Whether user data was loaded from DB
	STATE_READY,			// Ready to leave field
	STATE_READYTOVILLAGE,	// When transferring to another village server (preparation step)
	STATE_READYTOGAME,		// When transitioning to game (preparation step)
	STATE_READYTOLOGIN,		// When transitioning to login (preparation stage)
	STATE_MOVETOVILLAGE,	// When transferring to another village server
	STATE_MOVETOGAME,		// When transitioning to game
	STATE_MOVETOLOGIN,		// When transitioning to login
	STATE_MOVESAMECHANNEL,	// Move to same channel


};

//enumulation
//GameRoom
enum eGameRoomState
{
	_GAME_STATE_NONE,					//Create!
	_GAME_STATE_READY2CONNECT,			//Initialization complete, waiting for connection state
	_GAME_STATE_CONNECT2CHECKAUTH,		//Check authentication info using DB
	_GAME_STATE_CONNECT2LOAD,			//Connection and verification complete! Transition to load state
	_GAME_STATE_LOAD2SYNC,				//Client connection complete ready state
	_GAME_STATE_SYNC2SYNC,
	_GAME_STATE_SYNC2PLAY,				//Waiting to play
	_GAME_STATE_PLAY,					//In game!

	_GAME_STATE_CANCEL_LOADING,			//Sync with background loader
	_GAME_STATE_DESTROYED,				//Room is destroyed!
	_GAME_STATE_FARM_NONE,				//State for farm. Creation routine differs from normal; identical from _GAME_STATE_PLAY onward.
	_GAME_STATE_FARM_READY2LOAD,		//State where farm-related data has been requested from DB
	_GAME_STATE_FARM_LOAD2PLAY,			//Received farm-related data from DB. Set it and notify master
	_GAME_STATE_FARM_PAUSE,				//Farm pause
	_GAME_STATE_PVP_SYNC2GAMEMODE,		//In PvP, a start point exists without users (Guild War); intermediate state that skips the connect state, receives the mode, and transitions to load
};

//GameSession
enum eGameSessionState
{
	SESSION_STATE_NONE,
	SESSION_STATE_READY,				//Object is only created! Not connected yet
	SESSION_STATE_RUDP_CONNECTED,	
	SESSION_STATE_CONNECTED,			//Connected!
	SESSION_STATE_LOAD,					//Request from DB once connected.
	SESSION_STATE_LOADED,				//Game loading complete!
	SESSION_STATE_WAIT_TO_READY,		//Wait for Ready state
	SESSION_STATE_READY_TO_SYNC,		//Sync ready!
	SESSION_STATE_SYNC_READY_2_DELAY,	//Server received a message before it was ready to process messages.
	SESSION_STATE_READY_TO_PLAY,		//Sync 2 Play
	SESSION_STATE_GAME_PLAY,			//Playing game!
	SESSION_STATE_READY_TO_VILLAGE,		//game -> village (preparation step)
	SESSION_STATE_READY_TO_LOGIN,		//game -> login char select (preparation step)
	SESSION_STATE_RECONNECTLOGIN,		//game -> login char select
	SESSION_STATE_SERVER_CHANGE,		//Moving server! Disconnects once transfer completes
	SESSION_STATE_DISCONNECTED,			//Cut off state! (Even TCP is completely disconnected)
	//SESSION_STATE_CRASH, // Crash!
};

// GameServer Parse Type
enum eGameMsgType
{
	MSG_TYPE_MASTER,
	MSG_TYPE_DATABASE,
};

enum eFieldMessage
{
	FM_CREATE,
	FM_DESTROY,
	FM_SHOW,
	FM_HIDE,
	FM_MOVE,
	FM_ACTORMSG,
	FM_CHAT,
	FM_CHANGEEQUIP,
	FM_ADDEXP,
	FM_LEVELUP,
	FM_CHANGEAPPELLATION,
	FM_CHANGEHIDE,
	FM_CHANGECASHEQUIP,
	FM_CHANGEGUILDSELFVIEW,
	FM_CHANGEJOB,
	FM_EFFECT,
	FM_CHANGECOLOR,
	FM_CHANGEGLYPH,
	FM_CHATROOMVIEW,
	FM_CHATROOMUSER,
	FM_CHANGEVEHICLE,
	FM_CHANGEVEHICLECOLOR,
	FM_CHANGEPET,
	FM_CHANGEPETBODY,
	FM_RENAME_PET,
	FM_ENTERPARTY,
	FM_LEAVEPARTY,
	FM_CHARACTER_RENAME,
	FM_GUILDWARWINSKILL,
	FM_USE_SOURCE,
	FM_USE_EFFECTSKILL,
	FM_HAVE_NAMEDITEM,
	FM_CHANGE_TRANSFORM,
	FM_CHANGETALISMAN,
	FM_CHANGEDWCTEAM,
};

enum eClass
{
	CLASS_NONE = 0,
	CLASS_WARRIER = 1,
	CLASS_ARCHER,
	CLASS_SOCERESS,
	CLASS_CLERIC,
#if defined( PRE_ADD_ACADEMIC )
	CLASS_ACADEMIC,
#endif // #if defined( PRE_ADD_ACADEMIC )
#if defined( PRE_ADD_KALI )
	CLASS_KALI,
#endif // #if defined( PRE_ADD_KALI )
	CLASS_MAX,
};

enum eJob
{
	JOB_NONE,

	JOB_WARRIER = 1,
	JOB_ARCHER,
	JOB_SOCERESS,
	JOB_CLERIC,

	JOB_BLAZEWARRIER = 11,
	JOB_SWORDMAN,
	JOB_CHAINSOLDIER,

	JOB_RANGER = 21,
	JOB_HUNTER,
	JOB_KEEPER,

	JOB_WIZARD = 31,
	JOB_MAGICIAN,
	JOB_WITCH,

	JOB_PRIEST = 41,
	JOB_PALADIN,
	JOB_MONK,

	JOB_MAX = 47,
};

enum eAppendItemString
{
	Append_ItemSN,
	Append_Price,
	Append_Limit,
	Append_ItemID,
	Append_Option,
	Append_Refundable,
#ifdef PRE_ADD_LIMITED_CASHITEM
	Append_QuantityLimited,
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	Append_Max,
};

enum eModItemBitmap
{
	ModItem_Durability = 1,
	ModItem_Level = (1<<1),
	ModItem_Potential = (1<<2),
	ModItem_SoulBound = (1<<3),
	ModItem_SealCount = (1<<4),
	ModItem_Option = (1<<5),
	ModItemAll = ModItem_Durability|ModItem_Level|ModItem_Potential|ModItem_SoulBound|ModItem_SealCount|ModItem_Option,
};

enum eWeaponType
{
	WEAPON_SWORD,			// Greatsword
	WEAPON_GAUNTLET,		// Gauntlet
	WEAPON_AXE,				// Axe
	WEAPON_HAMMER,			// Hammer
	WEAPON_SMALLBOW,		// Shortbow
	WEAPON_BIGBOW,			// Longbow
	WEAPON_CROSSBOW,		// Crossbow
	WEAPON_STAFF,			// Staff
	WEAPON_BOOK,			// Spellbook
	WEAPON_ORB,				// Crystal orb
	WEAPON_PUPPET,			// Voodoo doll
	WEAPON_MACE,			// Mace
	WEAPON_FLAIL,			// Flail
	WEAPON_WAND,			// Wand
	WEAPON_SHIELD,			// Shield
	WEAPON_ARROW,			// Arrow
	WEAPON_CANNON,			// Cannon
	WEAPON_BUBBLEGUN,		// Bubble Gun
	WEAPON_GLOVE,			// Gloves
	WEAPON_FAN,             // Fan
	WEAPON_CHAKRAM,         // Chakram
	WEAPON_CHARM,           // Charm
	WEAPON_SCIMITER,		// Scimitar
	WEAPON_DAGGER,			// Dagger
	WEAPON_CROOK,			// Crook
};

enum eParts
{
	PARTS_FACE,				// 
	PARTS_HAIR,
	PARTS_HELMET,			// Helmet
	PARTS_BODY,				// Upper body
	PARTS_LEG,				// Lower body
	PARTS_HAND,				// Hand
	PARTS_FOOT,				// Foot
	PARTS_NECKLACE,
	PARTS_EARRING,
	PARTS_RING,
	PARTS_RING2,
	PARTS_CREST,
	PARTS_FULLSET,			// Full set
};

enum eQuestDataType
{
	QUESTDATA_STATE = 1,	// Quest state
	QUESTDATA_STEP,			// Current quest step
	QUESTDATA_JOURNAL,		// Current quest journal
};

enum eEtcType	//  DNEtc table Type
{
	ETC_NONE,
	ETC_QUICKSLOT,			// TQuickSlot QuickSlot[QUICKSLOTMAX]; // Quick slot bar (2 rows of 10)
	ETC_JOB,				// USHORT wJobArray[JOBMAX]; // Class (0: initial class, remaining include subsequent class advancements)
	ETC_NOTIFIER,			// int Notifier[Notifier_MaxCount]; // Notifier (quest, mission)
	ETC_DUNGEONCLEAR,		// TDungeonClearData DungeonClear[DUNGEONCLEARMAX]; // Dungeon clear (120)
	ETC_ETERNITYITEM,		// USHORT wEternityItem[ETERNITYITEMMAX]; // Permanent items
	ETC_EQUIPDELAYTIME,		// int nEquipDelayTime[EQUIPMAX]; // Cooldown related to equip index
	ETC_EQUIPREMAINTIME,	// int nEquipRemainTime[EQUIPMAX];
};

enum eAuthType
{
	AUTHTYPE_BEGINAUTH,		// Start auth
	AUTHTYPE_STOREAUTH,		// Save auth
	AUTHTYPE_CHECKAUTH,		// Check auth
	AUTHTYPE_RESETAUTH,		// Reset auth
	AUTHTYPE_CNT,
};

enum eAbuseCode
{
	ABUSE_NONE = 0,
	ABUSE_MOVE_SERVERS,		// Village <-> Dungeon warp
	ABUSE_MOVE_INSKY,		// Air movement
	ABUSE_HACKSHIELD,		// Cheater detected by HackShield
	ABUSE_TWN_EXTENDLOG,	// Additional log value for Taiwan, related to #48714
#if defined( PRE_ADD_CHARACTERCHECKSUM )
	ABUSE_CHARACTER_CHECKSUM, // #80315 Character checksum anomaly detected
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )
};

struct WorldUserState
{
	enum eCode
	{
		Add = 0,
		Delete,
		Modify,
	};
};

namespace DBDNMembership
{
	struct OrderStatusCode
	{
		enum eOrderStatusCode
		{
			SuccessPayment = 1,		// Payment succeeded & waiting for pickup
			Success = 2,			// Purchase complete
			FailPayment = 3,		// Payment failed
		};
	};
};

namespace DBDNWorldDef
{
	struct CharacterExpChangeCode
	{
		enum eCharacterExpChangeCode 
		{
			Quest = 1,			// : Quest reward -> FKey = QuestID
			Dungeon = 2,		// : Dungeon reward -> FKey = PartyID or NULL
			Cheat = 3,			// : Cheat -> FKey = NULL
			Admin = 4,			// : Calibration change
			Eternity = 5,		// Permanent item -> FKey = ItemSerial
			PvP	= 6,			// : PvP FTG reward -> FKey = NULL
			DungeonMonster = 7,	// : Dungeon monster hunt EXP -> FKey = PartyID or NULL (used as a server delimiter; converted to Dungeon when passed to DB)
			Item = 8,			// : EXP boost item -> FKey = ItemID
		};
	};

	struct CharacterLevelChangeCode
	{
		enum eCharacterLevelChangeCode
		{
			Normal = 1,		// : Normal
			Cheat = 2,		// : Cheat
			Admin = 3,		// : Admin modification
		};
	};

	struct CoinChangeCode
	{
		enum eCoinChangeCode	// Note: Dungeon reward, mail attachment, storage deposit/withdraw, item sellback, trade deposit/withdraw, trade fee, enhance fee, skill unlock, shop purchase, and mail fee are included in their respective SPs.
		{
			DoNotDBSave = 0,
			QuestReward = 1,					// : Quest reward (FKey: QuestID)
			PickUp = 2,							// : Pickup (FKey: PartyID or NULL)
			Cheat = 3,							// : Cheat (FKey: NULL)
			Admin = 4,							// : Admin modification (FKey: AuditLogID)
			Use = 5,							// : Use (FKey: NULL)
												// 6: Mail send attachment (FKey: MailID)
												// 7: Mail receive attachment (FKey: MailID)
			InvenToWare = 8,					// : Storage deposit (FKey: NULL)
			WareToInven = 9,					// : Storage withdraw (FKey: NULL)
												// 10: Sell item back (FKey: NpcID)
												// 11: Trade deposit (FKey: CharacterID)
												// 12: Trade withdrawal (FKey: CharacterID)
			QuestSub = 13,						// Deduct quest (FKey: QuestID)
			RepairItem = 14,					// Repair item (FKey: ItemSerial)
												// 15: Trade fee (FKey: NULL)
												// 16: Enchant fee (FKey: ItemEnchantLogID)
												// 17: Skill unlock (FKey: SkillID)
												// 18: Shop purchase (FKey: ItemSerial)
			DisjointTax = 19,					// Disassemble fee (FKey: NULL)
												// 20: Mail fee (FKey: MailID)
												// 21: Trade House registration fee (FKey: TradeID)
												// 22: Trade House item purchase (FKey: TradeID)
												// 23: Trade House item sale proceeds (FKey: TradeID)
												// 24: Trade House transaction fee (FKey: TradeID)
			CompoundTax = 25,					// Combination fee (FKey: NULL)
												// 26: Delete character (FKey: NULL)
			MaxLevelExperienceTransGold = 27,	// Max-level EXP gold reward (FKey: NULL)
			Present = 28,						// Gift (FKey: NpcID)
												// 29: Create guild (FKey: GuildID)
			GuildLevelup = 30,					// 30: Guild level up (FKey: GuildID)
			InvenToGuildWare = 31,				// 31: Guild storage deposit (FKey: GuildID)
			GuildWareToInven = 32,				// 32: Guild storage withdraw (FKey: GuildID)
			GuildRewardBuy = 33,				// 33: Guild reward purchase (FKey: GuildItemID)
			Donation = 34,						// Donation
												// 35: (DOORS) Registration fee
												// 36: (DOORS) Bidding fee
												// 37: (DOORS) Winning bid fee
												// 38: (DOORS) Winning bid payment
												// 39: (DOORS) Winning bid settlement
												// 40: (DOORS) Cancel registration fee
												// 41: (DOORS) Cancel bid fee
												// 42: (DOORS) Cancel winning bid fee
												// 43: (DOORS) Cancel winning bid payment
												// 44: (DOORS) Cancel winning bid settlement
			CharmItem = 45,						// Treasure chest (FKey: ItemID)
			ChaosItem = 46,						// Chaos Cube (FKey: ItemID)
			SpecialBox = 47,					// Special Storage (FKey: EventRewardID)
			// 48: World storage usage fee
			// 49: C2C storage
			// 50: C2C retrieval
			// 51: C2C trade
			TalismanSlotOpen = 52,				// 52: Talisman slot open (FKey: Slot Index)
			TalismanSlotChange = 53,			// 53: Move talisman slot equip position

			GamblePrice=55,						// 55: Gamble mode entry fee
			GambleFirstWinner=56,				// 56: Gamble mode 1st place gold
			GambleSecondWinner=57,				// 56: Gamble mode 2nd place gold
			DWC = 58,							// 58: DWC character default starting gold
		};
		enum ePetalChangeCode
		{
			GambleEnterPetal= 9,				// 9: Gamble mode Petal entry fee
			GambleFirstWinPetal= 12,			// 12: Gamble mode 1st place Petal prize
			GambleSecondWinPetal= 13,			// 13: Gamble mode 2nd place Petal prize
		};
	};

	struct CoinTypeCode
	{
		enum eCoinTypeCode
		{
			Coin = 1,				// : Coin
			WarehouseCoin = 2,		// : Storage coin
			RebirthCoin = 3,		// : Revival coin
			PCBangRebirthCoin = 4,	// : PC bang revival coin
		};
	};

	struct EquipmentAttributeCode
	{
		enum eEquipmentAttributeCode 
		{
			DelayTime = 1,	// : Delay time
			RemainTime = 2,	// : Remain time
		};
	};

	struct FatigueTypeCode
	{
		enum eFatigueTypeCode
		{
			PCBang = 1,		// : PC bang FTG
			Daily = 2,		// : Daily FTG
			Weekly = 3,		// : Weekly FTG
			Event = 4,		// Event fatigue
			VIP = 5,		// VIP fatigue
		};
	};

	struct PublisherCode
	{
		enum ePublisherCode
		{
			Studio = 0,
			KR = 1,
			CH = 2,
			JP = 3,
			TW = 4,
			US = 5,
			SG = 6,
			TH = 7,
			ID = 8,
			RU = 9,
			EU = 10,
		};
	};

	struct QuestStatusCode
	{
		enum eQuestStatusCode
		{
			Gain = 0,		// : Quest acquired
			Progress = 1,	// : Quest progress
		};
	};

	struct SkillPointCode
	{
		enum eSkillPointCode
		{
			LevelUp = 1,		// Skill level up
			EternityItem = 2,	// Permanent item
			Repair = 10,		// Just force restore settings
		};
	};

	struct SkillChangeCode
	{
		enum eSkillChangeCode
		{
			Gain = 1,		// : Skill point acquired
			Use = 2,		// : Skill level-up & SP usage
			Reset = 3,		// : Skill point reset
			GainByQuest = 4,	// : Skill acquire (quest)
			GainByDungeon = 5,	// : Skill acquire (dungeon reward)
			GainByBook = 6,		// : Skill acquire (skill book)
			GainByEvent = 7,	// : Skill acquire (event)
			GainByAdmin = 8,	// : Skill acquire (by admin)
			DelByDrop = 9,		// : Skill delete (drop)
			DelByQuest = 10,	// : Skill delete (quest)
			DelByEvent = 11,	// : Skill delete (event expired)
			DelByAdmin = 12,	// : Skill delete (by admin)
			ModSkillLevelByAdmin = 13,	// : Skill level adjustment (by admin)
			GainByBuy = 14,		// 14 Acquire skill (purchase)
		};
	};

	struct TradeStatusCode
	{
		enum eTradeStatusCode
		{
			Sell = 0,		// : On sale
			CompleteSellByItem = 1,	// : Sold (item claimed)
			CompleteSellByCoin = 2,	// : Sold (proceeds claimed)
			CancelSell = 3,	// : Sale canceled
			Expire = 4,		// : Expired
		};
	};

	struct AddMaterializedItem
	{
		enum eCode
		{
			Pick			= 1,	// 1=Loot -> FKey: PartyID or NULL
			Mission			= 2,	// 2=Mission -> FKey: MissionID
			QuestReward		= 3,	// 3=Quest reward -> FKey: QuestID
			Present			= 4,	// 4=Gift -> FKey: PurchaseID
			PointBuy		= 5,	// 5=In-game point purchase -> FKey: NPCID
			CashBuy			= 6,	// 6=Cash purchase -> FKey: PurchaseID
			ReserveMoneyBuy	= 7,	// 7=Petal purchase -> FKey: PurchaseID
			GetGachaResultCashItem_JP = 9,	// 9=Cash item obtained from Japan gacha.
			GMGive			= 10,	// 10=Admin grant -> FKey: AuditLogID
			ItemDecompose	= 11,	// 11=Disassemble item -> FKey: NPCID
			// ItemEnchantFail = 12, // 12=Item enchant failed -> FKey: ItemEnchantID
			ItemCompound	= 13,	// Combination
			RandomItem		= 14,	// Random item
			DungeonReward	= 15,	// Dungeon reward
			PvPReward		= 16,	// PvP reward
			CharacterCreate = 17,	// Default grant
			SystemMail = 18,		// System mail (mission reward)
			FixedItem = 19,			// Cash item (face, hair) - erase existing face/hair and overwrite, FKey: serial of used cash item (in inventory)
			CostumeMix = 21,		// Costume synthesis
			FishingReward = 22,		// Fishing reward
			Harvest = 23,			// Harvest
			Cheat = 24,				// Cheat
			PvPLadderPoint = 25,	// Ladder points
			UnionPoint = 26,		// Union points
			Trigger = 27,			// Trigger
			GuildWarFestivalPoint = 28, // Guild festival points
			Repurchase = 29,
			GuildReversionItem = 30,
			CostumeRandomMixItem = 31,
			BestFriendItem = 34,
			COMEBACKREWARD = 35,	// Returning user reward
			SpecialBoxReward = 36,
			NewbieGameQuitReward = 37, //Reward when new user disconnects
			NewbiewReConnectReward = 38, //Reward on new user reconnect
			ComebackGameQuitReward = 39, //Reward when returnee logs out
			SeedPointBuy = 40, //Purchase with Seed
		};
	};

	struct MailTypeCode
	{
		enum eCode
		{
			Normal					= MailType::NormalMail,			// Normal mail
			Mission					= MailType::MissionMail,		// Mission reward
			Admin					= MailType::AdminMail,			// Admin mail
			EventMail				= MailType::EventMail,			// Event mail
			VIP						= MailType::VIPMail,			// VIP
			MasterSystem			= MailType::MasterSystemMail,	// Master-apprentice system
			NpcReputaion			= MailType::NpcReputationMail,	// NPC favorability system
			Quest					= MailType::Quest,				// Quest
			GuildWarReward  		= MailType::GuildWarReward,		// Guild War reward
			SpecialBox_Account		= MailType::SpecialBox_Account,	// Special Storage - Account Target 
			SpecialBox_Character	= MailType::SpecialBox_Character,	// Special Storage - Current Character Target
			Cadge					= MailType::Cadge,				// Wish request
			GuildMaxLevelReward		= MailType::GuildMaxLevelReward,// Max level guild reward for first guild join
			AppellationBookReward	= MailType::AppellationBookReward, // Title collection book completion reward
		};
	};

	struct UseItem
	{
		enum eUseItemCode
		{
			DoNotDBSave = 0,
			Use = 1,		// 1=Use consumable item
			Destroy = 2,	// 2=Discard
			DeCompose = 4,	// 4=Disassemble
			Present = 5,	// 5=Gift
			GuildReversionItem = 6,	// 6=Delete guild-bound item due to guild leave or kick
		};
	};

	struct ItemLocation
	{
		enum eCode
		{
			Equip = 1,
			Inventory = 2,
			Warehouse = 3,
			QuestInventory = 4,
			CashInventory = 5,
			GiftRepository = 6,
			CashEquip = 7,
			Glyph = 8,
			VehicleInventory = 9,
			PetInventory = 10,
			HarvestDepot = 11,	// Harvest warehouse
			ServerWare = 17,	// Server normal item storage
			ServerWareCash = 18,	// Server cash item storage
			Talisman = 19,	// Talisman
			PeriodInventory = 20,
			PeriodWarehouse = 21,
		};
	};

	struct PetType
	{
		enum eCode
		{
			Vehicle = 1,
			Pet = 2,
			SATIETY_PET = 3,
		};
	};

	struct PayMethodCode
	{
		enum eCode
		{
			Coin = 1,				// 1: Coin
			Cash = 2,				// 2: Cash
			Petal = 3,				// 3: Reward points
			Coupon = 4,				// 4: Coupon
			Admin = 5,				// 5: Admin grant
			Quest = 6,				// 6: Quest reward
			Mission = 7,			// 7: Mission reward
			LevelupEvent = 8,		// 8=Character level-up reward
			VIP = 9,				// 9 : VIP gift
			PvP = 10,				// 10 : PVP Ladder Points
			Union_Commerical = 11,	// 11 : Merchant Alliance Points
			Union_Royal = 12,		// 12 : Royal Castle Alliance Points
			Union_Liberty = 13,		// 13 : Free Alliance Points
			Cash_NexonUSA = 14,		// 14 : Nexon America Cash
			Event = 15,				// 15 : Event
			GuildWar_Festival = 16,	// 16 : Guild War Festival Points
			GuildWar_Reward = 17,	// 17 : Guild War Reward
			Comeback = 19,			// 18 : Returning User Reward
			BestFriend = 20,		// 20: Best friend
			BeginnerGuild = 21,		// 21: Beginner guild gift
			GuildSupport = 22,		// 22: Guild support reward
			ActiveMission = 23,		// 23: Active mission reward
#if defined( PRE_ADD_STAMPSYSTEM )
			Stamp = 24,				// 24: Stamp reward
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
		};
	};

	struct CashRebirthCode
	{
		enum eCode
		{
			Cash = 1,
			Petal = 2,
			Admin = 3,
			Item = 4,
			Seed = 5,
		};
	};

	struct EffectItemGetCode
	{
		enum eCode
		{
			Cash = 1,
			Petal = 2,
			Cooper = 3,
			Admin = 4,
			Union = 5,
			Item = 6,
			Guild = 7,
			BestFriend = 8,
			Seed = 9,
		};
	};

	struct EffectItemLifeSpanType
	{
		enum eCode
		{
			New = 0,
			Extend = 1,
			Update = 2,
		};
	};

	struct DifficultyCode
	{
		enum eCode
		{
			Easy = 1,
			Normal,
			Hard,
			Master,
			Abyss,
		};
	};

	struct ClearGradeCode
	{
		enum eCode
		{
			SSS = 10, 
			SS,
			S,
			A,
			B,
			C,
			D,
		};
	};

	struct RewardBoxCode
	{
		enum eCode
		{
			Wood = 1,
			Bronze,
			Silver,
			Gold,
		};
	};

	struct WhereToGoCode
	{
		enum eCode
		{
			StageEntrance = 1,
			Village,
		};
	};

	struct PvPModeCode
	{
		enum eCode
		{
			Round = 1,
			Respawn,
			Captain,
			IndividualRespawn,
			Zombie,
			Ladder1vs1,
			Ladder2vs2,
			Ladder3vs3,
			Ladder4vs4,
			GuildWar,
			Occupation,
			AllKill,
			Racing,
			Tournament,
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
			ComboExercise,
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
		};
	};

	struct PvPTeamCode
	{
		enum eCode
		{
			A = 1,
			B,
		};
	};

	struct PvPResultCode
	{
		enum eCode
		{
			Win = 1,
			Lose,
			Draw,
			GiveUp,
		};
	};

	struct RestraintTargetCode
	{
		enum eCode
		{
			Account = 1,	// Account
			Character,		// Character
		};
	};

	struct RestraintTypeCode
	{
		enum eCode
		{
			ConnectRestraint = 1,
			ChatRestraint,
			TradeRestraint,
		};
	};

	struct RestraintDolisReasonCode
	{
		enum eCode
		{
			DefaultCode = 110,
			AbuseRestraintCode = 111,
			AbuseTradeRestraintCode = 112,
		};
	};

	struct TransactorCode
	{
		enum eCode
		{
			Master = 1,
			Pupil,
		};
	};

	struct GetMasterCharacterCode
	{
		enum eCode
		{
			MyInfo = 1,				// Own master info
			MyMasterInfo,			// My master info
			OptionalMasterInfo,		// Arbitrary specific master info
		};
	};
#if defined( PRE_ADD_SECONDARY_SKILL )

	struct SecondarySkillTypeCode
	{
		enum eCode
		{
			ProductionSkill		= SecondarySkill::Type::ProductionSkill,		// Production skill
			ManufactureSkill	= SecondarySkill::Type::ManufactureSkill,		// Crafting skill
			CommonSkill			= SecondarySkill::Type::CommonSkill,			// Common skill
		};
	};
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	struct AppellationKind
	{
		enum eAppellationKind
		{
			Select = 1,
			Cover = 2
		};
	};

	struct EtcPointCode
	{
		enum eCode
		{
			None = 0,
			PvP = 1,
			Union_Commercial = 2,
			Union_Royal = 3,
			Union_Liberty = 4,
			GuildWar_Festival = 5,	// Guild War festival points			
			SeedPoint = 6,			// Seed money
			Max,
		};
	};
	struct GuildWarRewardType
	{
		enum eGuildWarRewardType
		{
			GuildWarFestivalWin = 1,	// Guild festival victory
			GuildWarFestivalLose,		// Guild festival defeat
			GuildWarFinalUnder4,		// Finals below semifinals
			GuildWarFinal4,				// Finals semifinalist reward
			GuildWarFinal2,				// Finals 2nd place reward
			GuildWarFinalWin,			// Finals winner reward
			GuildWarFinalAbnormalLose,	// Finals abnormal loss (preliminary score on tie?)
		};
	};

	struct EventRewardType
	{
		enum eEventRewardType
		{
			Admin = 1,
			System = 2,
		};
	};
};

//Exception Report
enum eExceptionRepert
{
	_EXCEPTIONTYPE_NONE = 0,

	//for game
	_EXCEPTIONTYPE_ROOMCRASH,
	_EXCEPTIONTYPE_SESSIONCRASH,
	_EXCEPTIONTYPE_ROOMDESTRUCTOR,
	_EXCEPTIONTYPE_GAME_DELAYED,

	//for db
	_EXCEPTIONTYPE_DBMW_DELAYED,
};

//GameServerAffinity Type
enum eGameServerAffinityType
{
	_GAMESERVER_AFFINITYTYPE_NORMAL = 0,		//Just normal dungeon or whatever like that
	_GAMESERVER_AFFINITYTYPE_HYBRYD,			//Receive both PvP and everything else
	_GAMESERVER_AFFINITYTYPE_PVP,				//Receive only PvP
	_GAMESERVER_AFFINITYTYPE_FARM,				//Dedicated farm game server for unified game server world processing!
};

// Cash
enum eCashEtc
{
	Reason_GameClient = 1,
	Reason_Web = 2,
	Reason_PCBangGameClient = 3,
	Reason_PCBangWeb = 4,

	Category_Main = 1,
	Category_Costume = 2,
	Category_Item = 3,
	Category_Package = 4,

	CashItemState_None = 1,
	CashItemState_Hot = 2,
	CashItemState_New = 3,
	CashItemState_Normal = 4,
	CashItemState_Discount = 5,
};


// structure -----------------------------------------------------------------------------------------------

struct TChannelInfo
{
	USHORT nChannelID;			//Assigned channel (field) ID
	USHORT nChannelIdx;
	USHORT nMapIdx;				//ID of the assigned map
	USHORT nChannelMaxUser;
	USHORT nCurrentUser;
	int nAttribute;				//DirtyBit
	int nMeritBonusID;			//Merit bonus
	bool bVisibility;
	WCHAR wszMapName[MAPNAMELENMAX];
	int	nLimitLevel;
	bool bShow;
	USHORT nDependentMapID;
	int	nServerID;
	BYTE cThreadID;
#if defined(PRE_ADD_CHANNELNAME)
	WCHAR wszLanguageName[CHANNELLANGUAGEMAX];
#endif //#if defined(PRE_ADD_CHANNELNAME)
	int GetUserPercentage() const
	{
		if( nChannelMaxUser > 0 )
			return nCurrentUser*100/nChannelMaxUser;
		return -1;
	}
};

struct TVillageInfo
{
	BYTE cVillageID;
	int nManagedID;
	char szIP[IPLENMAX];
	USHORT nPort;

	std::vector <TChannelInfo> vOwnedChannelList;
	TVillageInfo()
	{
		cVillageID = 0;
		nManagedID = 0;
		memset(szIP, 0, sizeof(szIP));
		nPort = 0;
	}
};

struct TGameInfo
{
	int nServerID;
	int nServerAttribute;
	ULONG nIP;
	USHORT nPort;
	USHORT nTcpPort;

	int nRoomCnt;
	int nUserCnt;
	bool bHasMargin;
	UINT nFrame;

	bool bComplete;
	UINT uiLastDelayReportTick;
};

// Server Info
struct TServerInfo
{
	bool bMergedWorld;
	int nWorldSetID;							// Merged world ID
	char cWorldID;
	WCHAR wszWorldName[WORLDNAMELENMAX];
	UINT nWorldMaxUser;				//For waiting user
	USHORT nDefaultMaxUser;
	bool bOnline;				// For displaying online/offline status
	bool bOnTop;
	std::vector <TVillageInfo> vOwnedVillageList;

	TServerInfo() 
	{
		bMergedWorld = false;
		nWorldSetID = 0;
		cWorldID = 0;
		memset(wszWorldName, 0, sizeof(wszWorldName));
		nWorldMaxUser = 0;
		nDefaultMaxUser = 0;
		bOnline = false;
		bOnTop = false;
	}
};

// 
struct TChannelInfoEx
{
	TChannelInfo Channel;
	BYTE cVillageID;
	char szIP[IPLENMAX];
	USHORT nPort;
};
typedef std::map<int, TChannelInfoEx> TMapChannel;

// Do not use float directly on the server. Multiply coordinates received from client by *1000.
struct TPosition
{
	int nX;
	int nY;
	int nZ;
};

struct TLocation
{
	UINT nMapIndex;
	TPosition Pos;
};

// Applies to NPC and user object creation. Common parts only 
struct TBaseData
{
	UINT nObjectID;
	WCHAR wszName[NAMELENMAX];
	int nChannelID;
	TPosition CurPos, TargetPos;
	TPosition MoveDir;
	float fRotate;
	bool bBattleMode;
};

struct TParamData
{
	eChatType eChatType;
	int nChatSize;
	WCHAR wszChatMsg[CHATLENMAX];
	char Buffer[128];
	int nBufSize;
	USHORT wActorProtocol;
	TItemInfo ItemInfo;
	TItemInfo CashItemInfo;
	BYTE cLevel;
	UINT nExp;
	int nJob;
	int nSelectAppellation;
	int nCoverAppellation;
	bool bHide;
	bool bEncrypt;
	DNEncryptPacketSeq	sEncrypt;
	char cType;
	char cState;
	DWORD dwColor;
	int nChatRoomParam;
	int nIndex;
	DWORD dwSendSessionID; // SendSessionID
	TItem Vehicle;
	TVehicleCompact PetInfo;
	char cPetPartsType;
};

struct TDefaultCreateData
{
	BYTE cClassID;										// Class ID
	int nCreateMapIndex;								// Starting map index on initial creation
	int nCreateMapStartPositionX[DEFAULTPOSITIONMAX];	// Starting X position on initial creation
	int nCreateMapStartPositionY[DEFAULTPOSITIONMAX];	// Starting Y position on initial creation
	int nCreateMapStartRadius[DEFAULTPOSITIONMAX];		// Radius

	int nCreateTutorialMapIndex;			// Tutorial map index on initial creation (GameServer side)
	int nCreateTutorialGateNo;				// Tutorial gate on initial creation (GameServer side)

	int nDefaultBody;						// Default body
	int nDefaultLeg;						// Default legs
	int nDefaultHand;						// Default hands
	int nDefaultFoot;						// Default feet
	int nFace[DEFAULTPARTSMAX];				// Face
	int nHair[DEFAULTPARTSMAX];				// Head
	int nHelmet[DEFAULTPARTSMAX];			// Helmet
	int nBody[DEFAULTPARTSMAX];				// Upper body
	int nLeg[DEFAULTPARTSMAX];				// Bottoms
	int nHand[DEFAULTPARTSMAX];				// Hand
	int nFoot[DEFAULTPARTSMAX];				// Foot
	int nWeapon[WEAPONMAX];					// Weapon

	DWORD dwHairColor[DEFAULTHAIRCOLORMAX];
	DWORD dwSkinColor[DEFAULTSKINCOLORMAX];
	DWORD dwEyeColor[DEFAULTEYECOLORMAX];

	int nDefaultItemID[DEFAULTITEMMAX];		// Owned item
	int nDefaultItemCount[DEFAULTITEMMAX];	// Owned item count

	int nDefaultSkillID[DEFAULTSKILLMAX];	// Owned skill

	TQuickSlot DefaultQuickSlot[DEFAULTQUICKSLOTMAX];	// Quick slot count
	int nDefaultGestureID[DEFAULTGESTUREMAX];	// Owned gesture
};


struct TMapInfo
{
	int nMapID;
	char szMapName[10][256];
	BYTE GateMapCount[WORLDMAP_GATECOUNT];
	USHORT GateMapIndexs[WORLDMAP_GATECOUNT][WORLDMAP_GATECOUNT];
	BYTE GateStartIndexs[WORLDMAP_GATECOUNT][WORLDMAP_GATECOUNT];
	int nWorldLevel;
	GlobalEnum::eMapTypeEnum MapType;
	GlobalEnum::eMapSubTypeEnum MapSubType;
	int nMaxClearCount;
	int nAllowMapType;
	std::vector<int> vMapAreaIndex;
	bool bExpandable;
	bool bUserReturnSystem;	// User return system
	bool bVehicleMode;
	bool bPetMode;
	bool bAllowFreePass;
#if defined( PRE_PARTY_DB )
	std::wstring wstrMapName;
	int iEnterConditionTable;
#endif // #if defined( PRE_PARTY_DB )
#if defined( PRE_NORMALSTAGE_REGULATION )
	char cRevisionNum;	//Correction number
#endif
#if defined(PRE_ADD_VEHICLE_SPECIAL_ACTION)
	bool bVehicleSpecalActionMode;
#endif
};

struct TCoinCountData
{
	BYTE cLevel;
	int nRebirthCoin[WORLDCOUNTMAX];		// Resurrection coin
	int nCashRebirthCoin[WORLDCOUNTMAX];	// Resurrection cash coin
	int nRebirthCoinLimit;					// Max free coin possession limit
	int nCashRebirthCoinLimit;				// Max held paid coins
	int nVIPRebirthCoin;					// VIP rebirth coin
};

struct TActorData
{
	int nActorID;
	BYTE cClass;					// Unit type kinds
};

const int QUEST_MAX_CNT = 20;
struct TNpcData
{
	int nNpcID;
	int nParam[2];
	WCHAR wszName[EXTDATANAMELENMAX];
	char szTalkFile[256];				// Dialogue file name
	char szScriptFile[256];				// script filename
	int QuestIndexArr[QUEST_MAX_CNT];	// Quest index
	TActorData ActorData;
};

struct TItemData
{
	int nItemID;
	WCHAR wszItemName[EXTDATANAMELENMAX];
	int nType;							// Type
	int nTypeParam[3];					// Type parameter
	BYTE cLevelLimit;					// Level restriction
	BYTE cRank;							// Grade
	BYTE cReversion;					// Whether bound
	bool IsCash;						// Is it a cash item
	bool IsDestruction;					// Whether destroyed
	bool IsAuthentication;				// Auth status
	int nAmount;						// Amount
	int nSellAmount;					// Sale amount
	int nOverlapCount;					// Stack count
	std::vector<int> nNeedJobClassList;	// Job restriction

	int nSkillID;						// Skill index
	char cSkillLevel;					// Skill level
	char cSkillUsingType;				// Skill use type
	int nMaxCoolTime;					// Max cooltime (immutable value)
	int nEnchantID;						// Enchant ID
	int	nAllowMapType;					// Usable map type
	char cSealCount;					// Seal count
	int nSealID;						// SealCountTableID
	int nNeedBuyItemID;					// Required item ID when purchasing from shop
	int nNeedBuyItemCount;				// Required item count when purchasing from shop
	int nNeedPvPRank;					// Required PvP rank to equip
	bool IsSealed;						// Whether sealed on pickup
	int nCashTradeCount;				// Cash item exchange count
	bool IsEnableCostumeMix;
	int nApplicableValue;
	bool IsCollectingEvent;			// Whether to call item collection API
	int nExchangeCode;				// Item category ID
	int iNeedBuyLadderPoint;		
	int iUseLevelLimit;
	int iNeedBuyUnionPoint;
	int iNeedBuyGuildWarPoint;
	bool bRebuyable;
	int nPeriod;
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	bool IsWStorage;					// Exception for items allowed in server storage
#endif

	void Reset() 
	{
		memset( wszItemName, 0, sizeof(wszItemName) );
		memset( nTypeParam, 0, sizeof(nTypeParam) );
		nNeedJobClassList.clear();
		std::vector<int>().swap(nNeedJobClassList);

		nItemID			= 0;
		nType			= 0;
		cLevelLimit		= 0;
		cRank			= 0;
		cReversion		= 0;
		IsCash			= 0;
		IsDestruction	= 0;
		IsAuthentication= 0;
		nAmount			= 0;
		nSellAmount		= 0;
		nOverlapCount	= 0;
		nSkillID		= 0;
		cSkillLevel		= 0;
		cSkillUsingType = 0;
		nMaxCoolTime	= 0;
		nAllowMapType	= 0;
		cSealCount		= 0;
		nNeedBuyItemID		= 0;
		nNeedBuyItemCount	= 0;
		nNeedPvPRank		= 0;
		IsSealed			= 0;
		nCashTradeCount = 0;
		IsEnableCostumeMix	= 0;
		nApplicableValue = 0;
		IsCollectingEvent = 0;
		nExchangeCode = 0;
		iNeedBuyUnionPoint = 0;
		iNeedBuyGuildWarPoint = 0;
		bRebuyable = false;
		nPeriod = 0;
	}
};

struct TWeaponData
{
	int nWeaponIndex;
	char cEquipType;					// Equip type
	int nLength;						// Weapon length
	int nDurability;					// Durability
	int nDurabilityRepairCoin;			// Repair cost
	int nPermitJob[10];
	bool bOneType;
};

struct TPartData
{
	int nPartIndex;
	int nParts;							// Parts
	int nDurability;					// Durability
	int nDurabilityRepairCoin;			// Repair cost
	int nPermitJob[10];
	std::vector<int> nSubPartsList;		// Sub-parts (one-piece outfit)

	TPartData()
	{
		nPartIndex = 0;
		nParts = 0;
		nDurability = 0;
		nDurabilityRepairCoin = 0;
		memset(nPermitJob, 0, sizeof(nPermitJob));
	}
};

struct TVehiclePartsData
{
	int nItemID;
	int nVehicleClassID;
	int nVehiclePartsType;
	std::string strSkinName;
	std::string strAniName;
	std::string strActName;
	int nSkinColor;
	TVehiclePartsData()
	{
		nItemID = 0;
		nVehicleClassID = 0;
		nVehiclePartsType = 0;
		nSkinColor = 0;
	}
};

struct TVehicleData
{
	int nItemID;
	int nVehicleClassID;
	int nVehicleActorID;
	int nDefaultBody;
	int nDefaultParts1;
	int nDefaultParts2;
	int nVehicleSummonTime;
	int nVehicleDefaultSpeed;
	int nPetLevelTypeID;	// Pet grade(?) value; key value to reference pets of different grades
	int nPetSkillID1;		// 
	int nPetSkillID2;
	int nRange;				// Scan range value, e.g. for pickup 
	int nFoodID;
};

const BYTE PetStateMax = 10;
struct TPetLevelDetail
{
	int nTableIndex;
	int nPetLevel;
	int nPetExp;
	int nState[PetStateMax];
	char *szStateMin[PetStateMax];
	char *szStateMax[PetStateMax];
	int nVipExp;
};

struct TPetLevelData
{
	int nPetLevelType;
	std::vector<TPetLevelDetail> VecPetLevel;
};

struct TPetFoodData
{
	int nItemID;
	int nTickTime; // Time consumed (second)
	int nFullTic;  // Consumption per tick
	BYTE cFullTimeLogOut; // Existing satiety divisor on logout
	int nFullMaxCount;		// Maximum satiety value
	int nUseExp[Pet::MAX_SATIETY_SELECTION];
	int nUseState[Pet::MAX_SATIETY_SELECTION];
};

struct TPotentialJewelData
{
	int nItemID;
	int nApplyApplicableValue;
	int nPotentialNo;
	bool bErasable;
	bool bCanApplySealedItem;

#if defined(PRE_ADD_REMOVE_PREFIX)
	bool bSuffix;
#endif // PRE_ADD_REMOVE_PREFIX
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	int nRollbackAmount; // Potential application rollback fee
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
};

struct TEnchantJewelData
{
	int nItemID;
	int nApplyApplicableValue;
	int nRequireMaxItemLevel;
	int nRequireEnchantLevel;
	int nEnchantLevel;
	bool bCanApplySealedItem;
};

struct TItemDropData
{
	int nItemDropID;
	bool bIsGroup[20];
	int nIndex[20];
	int nProb[20];
	int nInfo[20];
};

struct TGlobalWeightData
{
	int nID;
	float fValue;
};

struct TGlobalWeightIntData
{
	int nID;
	int nValue;
};

struct TLevelData
{
	int nIndex;
	short wStrength;					// Strength
	short wAgility;						// Agility
	short wIntelligence;				// Intelligence
	short wStamina;						// Health
	int nExperience;					// EXP
	int nFatigue;						// Fatigue
};

struct TStat
{
	USHORT wStr;
	USHORT wDex;
	USHORT wInt;
	USHORT wHealth;
	USHORT wLuck;
};

struct TSkillLevelData
{
	char cSkillLevel;					// Skill level
	char cLevelLimit;					// Level limit
	//int nNeedJobClass; // Job restriction -> Moved to skill table.
	int nDelayTime;						// Skill delay (ms)
	int nAddRange;						// Additional range(cm)
	int nNeedItemID;					// Consumable item
	int nNeedItemDecreaseCount;			// Consumed item count
	int nDecreaseHP;					// HP cost
	int nDecreaseSP;					// SP cost
	int nNeedSkillPoint;
	int nEffectClassValue1Duration;
};

struct TSkillData
{
	int nSkillID;						// Skill ID
	char cNeedWeaponType[ 2 ];			// Required weapon
	int nMaxLevel;						// Max level
	char cSkillType;					// Skill type
	char cDurationType;					// Duration type
	char cTargetType;					// Target type
	char cDissolvable;					// Whether it can be removed
	int nNeedJobID;						// Job restriction
	bool bDefaultLocked;				// Whether locked by default
	int nUnlockSkillBookItemID;			// Item ID of the unlocking skill book
	int nUnlockPrice;					// Cost when unlocking with money.
	int nExclusiveID;					// Skills with the same ID cannot be learned together.
	int nGlobalCoolTime;				// Global skill cooldown (#36858)
	int nGlobalSkillGroup;				// Global skill group (#36858)
	std::vector<TSkillLevelData> vLevelDataList;

	TSkillData()
	{
		nSkillID =0;
		memset(cNeedWeaponType, 0, sizeof(cNeedWeaponType));		
		nMaxLevel = 0;
		cSkillType = 0;
		cDurationType = 0;
		cTargetType = 0;
		cDissolvable = 0;
		nNeedJobID = 0;
		bDefaultLocked = false;
		nUnlockSkillBookItemID = 0;
		nUnlockPrice = 0;
		nExclusiveID = 0;
		nGlobalCoolTime = 0;
		nGlobalSkillGroup = 0;
	}
};

struct TSkillTreeData
{
	int nSkillID;
	int nTreeSlotIndex;
	int nParentSkillID[ 3 ];
	int nNeedParentSkillLevel[ 3 ];
#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
	int nNeedBasicSP;
	int nNeedFirstSP;
	int nNeedSecondSP;
#endif	//	#if defined(PRE_ADD_SKILL_LEVELUP_LIMIT_BY_SP)
#if defined( PRE_ADD_ONLY_SKILLBOOK )
	bool bNeedSkillBook;
#endif	// #if defined( PRE_ADD_ONLY_SKILLBOOK )
};

// NPC dialog related struct 
struct TALK_QUESTION
{
	std::wstring szQuestion;
};

struct TALK_ANSWER 
{
	std::wstring szLinkTarget;
	std::wstring szLinkIndex;
	std::wstring szAnswer;
	bool bOtherTargetLink;	// true if linking to another talk file
};

struct TALK_PARAGRAPH
{
	std::wstring				szIndex;
	TALK_QUESTION				Question;
	std::vector<TALK_ANSWER>	Answers;
};

// key - paragraph string index, second - paragraph
typedef std::map<std::wstring, TALK_PARAGRAPH> TALK_PARAGRAPH_MAP;
typedef TALK_PARAGRAPH_MAP::iterator TALK_PARAGRAPH_MAP_IT;

// key - dialog xml file name, second - full dialog
typedef std::map<std::wstring, TALK_PARAGRAPH_MAP> TALK_MAP;
typedef TALK_MAP::iterator TALK_MAP_IT;

#include <set>

// key - paragraph response string index (only used to prevent client dialog request integrity check error messages)
typedef std::set<std::wstring> TALK_ANSWER_SET;
typedef TALK_ANSWER_SET::iterator TALK_ANSWER_SET_IT;

// key - dialog xml file name, second - paragraph response string set (only used to suppress client dialog request integrity check error messages)
typedef std::map<std::wstring, TALK_ANSWER_SET>	TALK_ANSWER_MAP;
typedef TALK_ANSWER_MAP::iterator TALK_ANSWER_MAP_IT;

// Quest reward item table info
// Can set money and up to 6 item types. nItemID and nItemCount are a pair.

struct TQuestRewardBase
{
	struct _ITEMSET
	{
		int	nItemID;
		int	nItemCount;
		int	nItemOptionTableID;
	};
};

struct TQuestReward : public TQuestRewardBase
{
	UINT nIndex;			// Reward table index
	UINT nStringIndex;	// Reward table string
	char cType;			// Reward type: 1 = all rewards / 2 = selective reward
	char cSelectMax;		// Max number of items to choose for selectable reward.
	UINT nCoin;			// Reward amount
	int	nExp;			// Reward EXP
	int	nQuestID;		// Quest ID
	int	nQuestStep;		// Step on quest completion
	BYTE nClass;			// Reward class
	_ITEMSET ItemArray[QUESTREWARD_INVENTORYITEMMAX];
	_ITEMSET LevelCapItemArray[MAX_QUEST_LEVEL_CAP_REWARD];
	int nMailID;
	int nFatigue;
};

struct TQuestLevelCapReward : public TQuestRewardBase
{
	UINT uIndex;
	char cType;
	char cClassID;
	_ITEMSET Items[MAX_QUEST_LEVEL_CAP_REWARD];
};

struct TCompleteEventQuest
{
	short wCompleteCount;
	int nScheduleID;
};

// Shop
struct TShopItem
{
	int nItemID;		// Shop item ID
	int nCount;			// Shop item quantity
	int nMaxCount;		// Max item count
	int nPrice;			// Item price
	int buyLimitCount;
#if defined (PRE_ADD_COMBINEDSHOP_PERIOD)
	int nPeriod;
#endif
#if defined( PRE_ADD_LIMITED_SHOP )
	int nShopLimitReset;
#endif
};

struct TShopTabData
{
	int nTabID;
	std::vector<TShopItem> vShopItem;	// SHOPTABMAX
	TShopTabData()
	{
		Clear();
	}
	void Clear()
	{
		nTabID = 0;
		vShopItem.clear();
	}
};

struct TShopData
{
	int nShopID;		// Shop ID (compare with the ID held by the NPC to link)
	std::vector<TShopTabData> vShopTabData;	// SHOPITEMMAX
	TShopData()
	{
		nShopID = 0;
	}
};

// Skill Shop
struct TSkillShopItem
{
	int nSkillID;		// Skill ID
	int nPrice;			// Skill price
};

struct TSkillShopData
{
	int nShopID;		// Shop ID (compare with the ID held by the NPC to link)
	TSkillShopItem SkillShopItem[SKILLSHOPITEMMAX];	// Maximum is only 10
};

// Enchant
const int ENCHANTITEMMAX = 5;
struct TEnchantLevelData
{
	int nItemID;
	char cLevel;
#ifdef PRE_FIX_MEMOPT_ENCHANT
	int nNeedItemTableID;
	int nStateTableID;
	int nNeedCoin;
#else
	char cEnchantProb;
	char cBreakRatio;
	char cMinDownLevel;
	char cMaxDownLevel;
	int nNeedItemID[ENCHANTITEMMAX];
	int nNeedItemCount[ENCHANTITEMMAX];
	int nNeedCoin;
	int nFailResultItemID;
	int nPotentialID;
	char cPotentialProb;
	int nProtectItemCount;
#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
	int nShieldItemCount;	// For Superior Item Protection Jelly (no downgrade)
#endif
#endif // PRE_FIX_MEMOPT_ENCHANT
};

struct TEnchantData
{
	int nEnchantID;
	char cCount;
	std::map<char, TEnchantLevelData*> pMapLevelData;
};

#ifdef PRE_FIX_MEMOPT_ENCHANT
struct TEnchantNeedItemData
{
	char cEnchantProb;
	char cBreakRatio;
	char cMinDownLevel;
	char cMaxDownLevel;
	int nNeedItemID[ENCHANTITEMMAX];
	int nNeedItemCount[ENCHANTITEMMAX];
	float fUpStateRatio;
	int nFailResultItemID;
	int nProtectItemCount;
#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
	int nShieldItemCount;	// For Superior Item Protection Jelly (no downgrade)
#endif
};
#endif // PRE_FIX_MEMOPT_ENCHANT

struct TPotentialDataItem
{
	int nItemID;
	int nPotentailOffset;
	int nPotentialNo;
	int nProb;

	int nSkillID;
	int nSkillLevel;
	int nSkillUsingType;
};

struct TPotentialData
{
	int nPotentialID;
	int nTotalProb;
	std::vector<TPotentialDataItem*> pVecItemData;
};

// Exchange
struct TExchangeData
{
	int nInvenIndex;	// Inven index
	short wCount;		// How many were inserted
	INT64 biSerial;
};

#ifdef PRE_ADD_GACHA_JAPAN
// gachapon japan
struct TGachaponShopInfoByJob_JP
{
	int nGachaponTableID;		// Gachapon table ID
	int nGachaponIndex;			// Gachapon shop index.
	int nJobClassID;			// Job class of items dropped by this gachapon
	int nPartsLinkDropTableID[ CASHEQUIPMAX ];
};

struct TGachaponData_JP
{
	int nGachaponIndex;			// Gachapon shop index. Used to look up the index in the npc table. Not the table index.
	TGachaponShopInfoByJob_JP GachaponShopInfo[ CLASSKINDMAX ];
};
#endif // PRE_ADD_GACHA_JAPAN

struct TCostumeMix
{
	int nMinScore;
	int nDropID;
};

//community
//getsimple characterdata patialy enum type
enum eReqCharacterPartialy
{
	_REQCHARACTERPARTIALY_FRIEND, 
};

//Friend
struct TFriendGroup:public TBoostMemoryPool<TFriendGroup>
{
	UINT nGroupDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
};

struct TFriend:public TBoostMemoryPool<TFriend>
{
	UINT nFriendAccountDBID;
	INT64 biFriendCharacterDBID;
	UINT nGroupDBID;

	TCommunityLocation Location;
	
	WCHAR wszCharacterName[NAMELENMAX];
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
	WCHAR wszFriendMemo[FRIEND_MEMO_LEN_MAX];
#ifdef PRE_ADD_DOORS
	bool bMobileAuthentication;
#endif		//#ifdef PRE_ADD_DOORS
};

//UserState
enum eCummunityState		//Accept/decline invites, accept/decline notes, etc.
{
	_COMMUNITY_NONE = 0,
	_COMMUNITY_MSG_PERMIT,		//Accepting all messages~
	_COMMUNITY_MSG_NOTPERMIT,//Not accepting messages~ Leave me alone!
};

struct sWorldUserState
{
	WCHAR wszCharacterName[NAMELENMAX];
	INT64 biCharacterDBID;			//Just in case for now
	int	nChannelID;				//
	int	nMapIdx;
	int	nLocationState;			//eLocationState
	int	nCummunityState;		//eCummunityState
};

//ServiceManager NoticeType
enum eSMNoticeType
{
	_NOTICETYPE_ALL = 0,
	_NOTICETYPE_WORLD,
	_NOTICETYPE_EACHSERVER,
	_NOTICETYPE_CHANNEL,
	_NOTICETYPE_ZONE,
};

//VoiceChannelReqType
enum eVoiceChannelReqType
{
	_VOICECHANNEL_REQTYPE_PARTY,
	_VOICECHANNEL_REQTYPE_PVP,
	//Wouldn't it look something like this..
	//_VOICECHANNELTYPE_GUILD,
	//_VOICECHANNELTYPE_PRIVATE,
};

//Invite PartyInfo
struct TInvitePartyInfo
{
	TPARTYID PartyID;
	int nMapIdx;
	BYTE cLimitLevelMin;
#if defined( PRE_PARTY_DB )
#else
	BYTE cLimitLevelMax;
#endif // #if defined( PRE_PARTY_DB )
	BYTE cPartyMemberMax;
	BYTE cPartyMemberCount;
	BYTE cPartyAvrLevel;
	WCHAR wszPartyName[PARTYNAMELENMAX];
};

// PvP
struct TPvPMapTable
{
	UINT uiGameType;				// PvPCommon::GameType
	std::vector<UINT> vGameModeTableID;		// PvPGameModeTableID
	std::vector<UINT> vNumOfPlayerOption;		// Player count option
	bool bIsBreakInto;			// Drop-in allowed flag
	UINT uiItemUsageType;		// PvPCommon::ItemUsageType
	bool bIsAllowItemDrop;		// Item drop Flag
	bool bIsGuildBattleGround;	// Flag indicating whether usable in Guild War
	bool bIsReleaseShow;			// Expose in release build
	LadderSystem::MatchType::eCode	MatchType;

	// Function
	bool CheckMaxUser( const BYTE cMaxUser ) const
	{
		for( UINT i=0 ; i<vNumOfPlayerOption.size() ; ++i )
		{
			if( vNumOfPlayerOption[i] == cMaxUser )
				return true;
		}
		return false;
	}

	bool CheckGameModeTableID( const UINT uiTableID ) const
	{
		for( UINT i=0 ; i<vGameModeTableID.size() ; ++i )
		{
			if( vGameModeTableID[i] == uiTableID )
				return true;
		}
		return false;
	}

	bool CheckBreakIntoOption( const BYTE cBreakIntoFlag ) const
	{
		if( cBreakIntoFlag )
		{
			if( !bIsBreakInto )
				return false;
		}
		return true;
	}
};

struct TPvPGameModeTable
{
	int	nItemID;
	UINT uiGameMode;		// PvPCommon::GameMode
	std::vector<UINT> vWinCondition;
	std::vector<UINT> vPlayTimeSec;
	UINT uiRespawnTimeSec;
	UINT uiRespawnNoDamageTimeSec;
	UINT uiRespawnHPPercent;
	UINT uiRespawnMPPercent;
	UINT uiWinXPPerRound;
	UINT uiLoseXPPerRound;
	UINT uiItemDropTableID;
	bool bIsLevelRegulation;
	bool bIsReleaseShow;			// Expose in release build
	UINT uiMedalExp;
	UINT uiNumOfPlayersMin;
	UINT uiNumOfPlayersMax;
	std::vector<UINT> vVictoryExp;
	std::vector<UINT> vVictoryBonusRate;
	std::vector<UINT> vDefeatExp;
	std::vector<UINT> vDefeatBonusRate;
	LadderSystem::MatchType::eCode	LadderMatchType;
	int	iRewardItemID;
	int	nFatigueConTime;
	int	nFatigueConValue;
	int	nFatigueConExp;
	int	nFatigueConMedal;
	int	iNeedInven;
	int	iSkillSetting;
	bool bIsSelectableRegulation;
	bool bIsDefaultRegulation;
	int nBattleGroundTableID;
	bool bAllowedUserCreateMode;
#if defined( PRE_ADD_RACING_MODE ) || defined( PRE_ADD_PVP_TOURNAMENT )
	int nDefaultRewardItem;
	int nDefaultRewardValue;
	std::vector<int> vRankRewardItem;
	std::vector<int> vRankRewardValue;
#endif // #if defined( PRE_ADD_RACING_MODE ) || defined( PRE_ADD_PVP_TOURNAMENT )

	// Function
	bool CheckWinCondition( const UINT uiWinCondition ) const
	{
		if( vWinCondition.empty() )
			return true;

		for( UINT i=0 ; i<vWinCondition.size() ; ++i )
		{
			if( vWinCondition[i] == uiWinCondition )
				return true;
		}
		return false;
	}

	bool CheckPlayTimeSec( const UINT uiPlayTime ) const
	{
		if( vPlayTimeSec.empty() )
			return true;

		for( UINT i=0 ; i<vPlayTimeSec.size() ; ++i )
		{
			if( vPlayTimeSec[i] == uiPlayTime )
				return true;
		}
		return false;
	}

	// Return -1 on failure
	int GetWinconditionIndex( const UINT uiWinCondition ) const
	{
		for( UINT i=0 ; i<vWinCondition.size() ; ++i )
		{
			if( vWinCondition[i] == uiWinCondition )
				return i;
		}
		return -1;
	}

	bool CheckMaxUser( const UINT uiMaxUser ) const 
	{
		return (uiMaxUser >= uiNumOfPlayersMin && uiMaxUser <= uiNumOfPlayersMax);
	}
};

struct TPvPGameStartConditionTable
{
	UINT uiMinTeamPlayerNum;		// Min team members to start
	UINT uiMaxTeamPlayerDiff;	// Max allowed member count for both teams

	short Check( const UINT uiATeam, const UINT uiBTeam, const bool bRandomTeam ) const
	{
		// Check min team members to start
		if( uiATeam < uiMinTeamPlayerNum || uiBTeam < uiMinTeamPlayerNum )
			return ERROR_PVP_CANTSTART_LESS_MINTEAMPLAYER;

		if (!bRandomTeam)	// Handle when not a random team.
		{
			// Check max allowed member count for both teams
			UINT uiDiff = ::abs( static_cast<int>(uiATeam-uiBTeam) );
			if( uiDiff > uiMaxTeamPlayerDiff )
				return ERROR_PVP_CANTSTART_MORE_MAXDIFFTEAMPLAYER;
		}
		

		return ERROR_NONE;
	}
};

struct TPvPRankTable
{
	UINT uiXP;
#ifdef PRE_MOD_PVPRANK
	BYTE cType;			//See PvPCommon::RankTable
	BYTE cMinRange;		//rate or rank
	BYTE cMaxRange;		//rate or rank

	TPvPRankTable()
	{
		uiXP = 0;
		cType = PvPCommon::RankTable::InitValue;
		cMinRange = 0;
		cMaxRange = 0;
	}
#endif		//#ifdef PRE_MOD_PVPRANK
};

struct TAppellationData
{
	int nAppellationID;
	int nArrayIndex;
	int nColor;
	int nLevelLimit;
	int nType;
#if defined( PRE_ADD_NEWCOMEBACK )
	int nSkillItemID;	
#endif
#if defined( PRE_MOD_APPELLATIONBOOK_RENEWAL)
	std::vector<int> nUseCollectionBookIndex;
#endif	// #if defined( PRE_MOD_APPELLATIONBOOK_RENEWAL)
};

// Mission system related
struct TEventCondition
{
	char cEvent;
	char cType[5];
	int  nParam[5];
	char cOperator[5];
	bool bCheckType;
};

struct TMissionData
{
	bool bActivate;
	int nType;			// Mission type (eMissionType)
	int nMailID;
	int nRewardAppellation;
	int nRewardPoint;
	bool bIsAchieveNotice;
	TEventCondition GainCondition;
	TEventCondition AchieveCondition;
};

struct TDailyMissionData
{
	int nMailID;
	int nRewardAppellation;
	int nRewardPoint;
	TEventCondition AchieveCondition;
	int nCounterParam;
	char cCounterOperator;
	int nItemID;
	int nType;
	int nTypeParam;
	__time64_t tStartDate;
#if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	__time64_t tEndDate;
#endif	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
};

#if defined(PRE_ADD_REMOTE_QUEST)
struct TRemoteQuestData
{
	bool bActivate;
	int nIndex;
	int nQuestID;
	bool bPreQuestCheckType;
	std::vector<int> nVecPreQuestCondition;
	TEventCondition GainCondition;
};
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
struct TActiveMissionData
{
	int nIndex;
	bool bActivate;
	int nMailID;
	TEventCondition AchieveCondition;
};
#endif

// Collection
const char CollectionAppellationMax = 10;
struct TCollectionBookData
{
	int nID;
	std::vector<int> nVecAppellationArrayIndex;
	int nRewardAppellationArrayIndex;
#if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
	int nMailID[2];			// 0 - Mail ID sent when exceeding 50%, 1 - Reward MailID on completing title collection 
#endif  //#if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
};

// TaxTable
struct TTaxData
{
	char cTaxType;			// Tax type (TaxType_PostTax)
	int nLevelStart;		// Start level
	int nLevelEnd;			// End level
	int nAmountStart;		// Starting amount (in gold)
	int nAmountEnd;			// End amount
	float fRate;			// Tax rate
	int nTaxAmount;			// Tax amount
};

// Seal
const BYTE SEALENCHANTMAX = 16;	// From +0 to +15 enhancement.
struct TSealCountData
{
	int nType1;
	int nType2;
	int nLevel[SEALENCHANTMAX];
	char cSuccessRate[SEALENCHANTMAX];
};

// PCBang
const BYTE PCParamMax = 2;
struct TPCBangParam
{
	int nParam[PCParamMax];
};

struct TPCBangData
{
	char cType;		// PCBang benefit type (ePCBangType)
	std::vector<TPCBangParam> VecPCBangParam;
	std::vector<TPCBangParam> VecPremiumPCBangParam;
	std::vector<TPCBangParam> VecGoldPCBangParam;
	std::vector<TPCBangParam> VecSilverPCBangParam;
	std::vector<TPCBangParam> VecRedPCBangParam;
};

// CashShop
const BYTE COMMODITYITEMMAX = 5;
const BYTE COMMODITYLINKMAX = 5;
struct TCashCommodityData
{
	int nSN;
	BYTE cCategory;
	int nSubCategory;
	int nItemID[COMMODITYITEMMAX];	// ItemTable item ID
	int nLinkSN[COMMODITYLINKMAX];	// Linked product
	std::string strName;
	short wPeriod;					// Duration
	int nPrice;						// Price
	int nCount;						// Quantity granted on purchase
	int nPriority;					// Display priority in the cash shop
	bool bOnSale;					// Sale status
	bool bOnSaleReal;				// Actual(?) sale state
	char cState;					// For displaying status such as HOT, NEW, etc.
	bool bLimit;					// Limited sale
	bool bReserveGive;				// Whether reward points are granted
	int nReserve;					// Reward points granted on purchase
	int nValidity;					// Expiration period
	bool bReserveAble;				// Whether it can be purchased with reward points
	bool bPresentAble;				// Whether gifting is allowed
	char cItemSort;					// Item type
	int nPriceFix;					// Fixed price
	bool bCartAble;					// Whether it can be added to the cart
	bool bVIPSell;					// Whether VIP-exclusive sale
	int nVIPLevel;					// VIP tier
	bool bAutomaticPay;				// Whether auto-payment is enabled
	int nAutomaticPaySale;			// Apply discount after auto-payment
	int nVIPPoint;					// VIPPoint granted on purchase
	int nOverlapBuy;				// Prevent duplicate purchase
#if defined(PRE_ADD_CASH_REFUND)
	bool bNoRefund;					// Whether withdrawal is disallowed (true: disallowed)
#endif
#if defined(PRE_ADD_SALE_COUPON)
	bool bUseCoupon;
#endif //#if defined(PRE_ADD_SALE_COUPON)

	TCashCommodityData()
	{
		nSN=0; cCategory=0; nSubCategory =0; 
		memset(nItemID, 0, sizeof(nItemID));
		memset(nLinkSN, 0, sizeof(nLinkSN));
		wPeriod=0; nPrice=0; nCount=0; nPriority=0;
		bOnSale=false; bOnSaleReal=false; cState=0;
		bLimit=false; bReserveGive=false; nReserve=0;
		nValidity=0; bReserveAble=false; bPresentAble=false;
		cItemSort=0; nPriceFix=0; bCartAble=false;
		bVIPSell=false; nVIPLevel=0; bAutomaticPay=false;
		nAutomaticPaySale=0; nVIPPoint=0;
		nOverlapBuy =0;

#if defined(PRE_ADD_CASH_REFUND)
		bNoRefund = true;		// Disabled by default
#endif
#if defined(PRE_ADD_SALE_COUPON)
		bUseCoupon = false;
#endif // #if defined(PRE_ADD_SALE_COUPON)
	}
};

struct TCashPackageData
{
	int nSN;		// Package sale SN
	std::vector<int> nVecCommoditySN;

	TCashPackageData()
	{
		nSN = 0;
		nVecCommoditySN.clear();
	}
};

struct TCashLimitData
{
	int nSN;			// Product SN for limited sale
	time_t tStartDate;  // _StartDate Start date
	time_t tEndDate;	// _EndDate End date	
	int nBuyAbleCount;	// Purchase limit per character
#if defined(PRE_ADD_LIMITED_CASHITEM)
	int nLimitedSellCount;		//Limited-quantity item!
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM)
};

struct TVIPData
{
	int nVipID;				// ID in VIPTable
	int nMonthItemSN;		// Cash item product number (grant once on 30-day payment)
	int nMailID[4];			// 0: 7 days before service expiration, 1: 1 day before service expiration, 2: On service expiration, 3: Insufficient cash on auto-billing
	int nAutoPayItemSN;		// Item SN to pay with during auto-payment	
};

// DL Table
struct TDLMapData
{
	int nTotalRound;
	int nFloor;

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	bool bChallengeDarkLair;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
};

struct TDLClearDataItem
{
	int nLevelMin;
	int nLevelMax;
	int nRoundMin;
	int nRoundMax;
	int nRewardExperience;
	int nShowBoxCount;
	int nSelectBoxCount;
	char cTreasureBoxRatio[4];
	int nRewardItemID[4];
	int nReturnWorldSetID;
	int nReturnWorldGateID;
	int nRewardGP;
};

struct TDLClearData
{
	std::vector<TDLClearDataItem*> pVecClearItem;
};

// Levelup Event 
struct TLevelupEvent
{
	enum eEventType
	{
		Levelup = 1,			// Normal level-up event
		ShandaPromotion = 2,	// Shanda promotion event
		CombackUser = 3,		// Returning user event
		LevelupNew = 4,			// New level-up event (one-time claim version)
		ComeBackUserInven = 5,		// Item type given directly to returnee
		NewbieQuitReward = 6, //New user disconnect reward
		NewbieReconnectReward = 7, //New user reconnect reward
		ComeBackUserQuitReward = 8, //Returnee logout reward
	};

	int nLevel;			// Player character's level
	char cClassID;		// Class
	BYTE cJob;
	int nMailID;
	int nCashMailID;
	int nEventType;		// eEventType

	TLevelupEvent()
	{
		nLevel = 0;
		cClassID = 0;
		nMailID = 0;
		nCashMailID = 0;
		cJob = 0;
	}
};

struct TItemOptionTableData
{
	int iEnchant;
	int iOption;
	int iPotential;
	TItemOptionTableData()
	{
		iEnchant = 0;
		iOption = 0;
		iPotential = 0;
	}
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TReputeTableData
{
#if defined( PRE_ADD_REPUTATION_EXPOSURE )
	int iMissionID;
	int iNpcPresentRepute1; // Favorability condition to grant Gift 1
	int iNpcPresentRepute2; // Favorability condition to grant Gift 2
	int iNpcPresentID1; // Gift 1 ItemID given by NPC upon reaching NpcPresentRepute1
	int iNpcPresentID2; // Gift 2 ItemID given by NPC upon reaching NpcPresentRepute1
#else
	std::vector<int>	iQuestIDs;
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )
	
	int	iMaxFavor;			// Max favor
	int	iTakeFavorPerDay;	// Favor value decreased per day
	int	iMaxMalice;			// Max malice
	int	iAddMalicePerDay;	// Hostility value increased per day
	int	iPresentIDArr[6];	// List of receivable gifts
	std::vector<int> vFavorNpcID;		// Favor NPC list
	std::vector<int> vMaliceNpcID;		// Malice NPC list
	int	iFavorGroupBomb;
	int	iMaliceGroupBomb;
	int	iMailID[NpcReputation::Common::MaxMailCount];
	int	iMailRand[NpcReputation::Common::MaxMailCount];
	int	iPlusItemID;
	int	iPlusProb;
	int	iUnionID;
	int	iPresentPointArr[6]; // Alliance points obtainable per gift

	TReputeTableData()
	{
#if defined( PRE_ADD_REPUTATION_EXPOSURE )
		iMissionID			= 0;
		iNpcPresentRepute1	= 0;
		iNpcPresentRepute2	= 0;
		iNpcPresentID1		= 0;
		iNpcPresentID2		= 0;
#else
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )
		iMaxFavor			= 0;
		iTakeFavorPerDay	= 0;
		iMaxMalice			= 0;
		iAddMalicePerDay	= 0;
		iFavorGroupBomb		= 0;
		iMaliceGroupBomb	= 0;
		for( UINT i=0 ; i<_countof(iPresentIDArr) ; ++i )
			iPresentIDArr[i] = 0;
		for( UINT i=0 ; i<NpcReputation::Common::MaxMailCount ; ++i )
		{
			iMailID[i]		= 0;
			iMailRand[i]	= 0;
		}
		vFavorNpcID.clear();
		vMaliceNpcID.clear();

		iPlusItemID			= 0;
		iPlusProb			= 0;
		iUnionID			= 0;
		for( UINT i=0 ; i<_countof(iPresentPointArr) ; ++i )
			iPresentPointArr[i] = 0;
	}

	int GetUnionPointByPresent (const int iPresentID)
	{
		for( UINT i=0 ; i<_countof(iPresentIDArr) ; ++i )
		{
			if( iPresentIDArr[i] == iPresentID )
			{
				// Just in case, check the size one more time!
				if (0 <= i && i < _countof(iPresentPointArr))
					return iPresentPointArr[i];
			}
		}
		return 0;
	}

	bool CheckPresentID( const int iPresentID )
	{
		for( UINT i=0 ; i<_countof(iPresentIDArr) ; ++i )
		{
			if( iPresentIDArr[i] == iPresentID )
				return true;
		}
		return false;
	}

	bool bIsFavorNpc( const int iNpcID )
	{
		for( UINT i=0 ; i<vFavorNpcID.size() ; ++i )
		{
			if( vFavorNpcID[i] == iNpcID )
				return true;
		}
		return false;
	}

	bool bIsMaliceNpc( const int iNpcID )
	{
		for( UINT i=0 ; i<vMaliceNpcID.size() ; ++i )
		{
			if( vMaliceNpcID[i] == iNpcID )
				return true;
		}
		return false;
	}
};

struct TPresentTableData
{
	enum eType
	{
		Normal	= 0,
		Cash	= 1,
		Coin	= 2,
		Max,
	};

	eType Type;
	int	iTypeID;
	int	iCount;
	int	iAddFavorPoint;
	int	iTakeMalicePoint;
	int	iFavorGroupPoint;
	int	iMaliceGroupPoint;

	TPresentTableData()
	{
		Type = Max;
		iTypeID = 0;
		iCount = 0;
		iAddFavorPoint = 0;
		iTakeMalicePoint = 0;
		iFavorGroupPoint = 0;
		iMaliceGroupPoint = 0;
	}
};

struct TMailTableData
{
	DBDNWorldDef::MailTypeCode::eCode	Code;
	bool IsCash;
	int	nSenderUIStringIndex;
	int	nTitleUIStringIndex;
	int	nTextUIStringIndex;
	int	ItemIDArr[MAILATTACHITEMMAX];
	int	ItemCountArr[MAILATTACHITEMMAX];
	int	ItemSNArr[MAILATTACHITEMMAX];
	int	nPresentMoney;
#if defined(PRE_SPECIALBOX)
	int nKeepBoxReceive;
	int nKeepBoxType;
	int nKeepBoxLevelMin;
	int nKeepBoxLevelMax;
	int nKeepBoxClass;
#endif	// #if defined(PRE_SPECIALBOX)
	
	TMailTableData()
	{
		Code = static_cast<DBDNWorldDef::MailTypeCode::eCode>(0);
		IsCash = false;
		nSenderUIStringIndex = 0;
		nTitleUIStringIndex = 0;
		nTextUIStringIndex = 0;
		nPresentMoney = 0;
		memset( ItemIDArr, 0, sizeof(ItemIDArr) );
		memset( ItemCountArr, 0, sizeof(ItemCountArr) );
		memset( ItemSNArr, 0, sizeof(ItemSNArr) );
#if defined(PRE_SPECIALBOX)
		nKeepBoxReceive = 0;
		nKeepBoxType = 0;
		nKeepBoxLevelMin = 0;
		nKeepBoxLevelMax = 0;
		nKeepBoxClass = 0;
#endif	// #if defined(PRE_SPECIALBOX)
	}
};

struct TStoreBenefitData
{
	// The order below matches the StoreBenefit table definition, so it must NEVER be changed.
	enum eType
	{
		EnchantFeeDiscount = NpcReputation::StoreBenefit::EnchantFeeDiscount,
		RepairFeeDiscount = NpcReputation::StoreBenefit::RepairFeeDiscount,
		SellingPriceUp = NpcReputation::StoreBenefit::SellingPriceUp,
		BuyingPriceDiscount = NpcReputation::StoreBenefit::BuyingPriceDiscount,
		FishProficiencyUp = NpcReputation::StoreBenefit::FishProficiencyUp,
		CookProficiencyUp = NpcReputation::StoreBenefit::CookProficiencyUp,
		CultivateProficiencyUp = NpcReputation::StoreBenefit::CultivateProficiencyUp,
		Max = NpcReputation::StoreBenefit::Max, 
	};

	int iNpcID;
	eType Type;
	int aiFavorThreshold[ STORE_BENEFIT_MAX ];
	int aiAdjustPercent[ STORE_BENEFIT_MAX ];

	TStoreBenefitData( void ) : iNpcID( 0 ), Type( Max )
	{
		memset( aiFavorThreshold, 0, sizeof(aiFavorThreshold) );
		memset( aiAdjustPercent, 0, sizeof(aiAdjustPercent) );
	}
};

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TShopPurchaseType
{
	Shop::PurchaseType::eCode PurchaseType;
	int iPurchaseItemID;
	int iPurchaseItemValue;
};

struct TCombinedShopTableData
{
	TShopItem ShopItem;
	TShopPurchaseType PurchaseType[Shop::Max::PurchaseType];
	Shop::PurchaseLimitType::eCode PurchaseLimitType;
	int iPurchaseLimitValue;
#if defined(PRE_SAMPLEITEMNPC)
	int nSampleVersion;	// Sample item round
#endif	//#if defined(PRE_SAMPLEITEMNPC)
};

struct TMasterSystemDecreaseTableData
{
	int iPupilLevel;
	int iPupilFavor;
	int iFavorUpDecreaseRespectPoint;
	int iFavorDownDecreaseRespectPoint;
	int iMasterSuccessMailID;
	int iMasterFailureMailID;
	int iPupilSuccessMailID;
	int iPupilFailureMailID;

	int GetDecreaseRespectPoint( int iFavorPoint )
	{
		if( iFavorPoint >= iPupilFavor)
			return iFavorUpDecreaseRespectPoint;
		return iFavorDownDecreaseRespectPoint;
	}
};

struct TMasterSystemGainTableData
{
	int	iFavorPoint;
	int	iRepectPoint;
};

#if defined( PRE_ADD_SECONDARY_SKILL )

struct TSecondarySkillTableData
{
	int	iSkillID;
	SecondarySkill::Type::eType Type;
	SecondarySkill::SubType::eType SubType;
	SecondarySkill::ExecuteType::eType ExecuteType;
};

struct TSecondarySkillLevelTableData
{
	int	iSkillID;
	SecondarySkill::Grade::eType Grade;
	int	iLevel;
	int	iExp;
	int	iNextID;
	int	iParam;
};

struct TSecondarySkillRecipeTableData
{
public:
	int iManufactureItemID[SecondarySkill::ManufactureSkill::MaxManufactureItemTypeCount];
	int	iMinExp[SecondarySkill::ManufactureSkill::MaxManufactureItemTypeCount];
	int	iMaxExp[SecondarySkill::ManufactureSkill::MaxManufactureItemTypeCount];
	SecondarySkill::Grade::eType	RequireGrade;
	int	iRequireSkillLevel;
	int	iToolItemID[SecondarySkill::ManufactureSkill::MaxToolItemCount];
	int	iMasterExp;
	int	iStartExp;
	int	iMaxTime;	// Unit (ms)
	int	iSuccessProbability;
	int	iSuccessUpExp;
	int	iRecipeUpExp;
	int	iMaterialItemID[SecondarySkill::ManufactureSkill::MaxMaterialItemCount];
	int	iMaterialItemCount[SecondarySkill::ManufactureSkill::MaxMaterialItemCount];
	int	iSuccessCount;

	// Get ItemID to be produced from EXP
	int GetManufactureItemID( int iExp, int iMaxValue )
	{
		int iExpRate = static_cast<int>(iExp/static_cast<float>(iMaxValue)*100);

		for( int i=0 ; i<SecondarySkill::ManufactureSkill::MaxManufactureItemTypeCount ; ++i )
		{
			if( iExpRate >= iMinExp[i] && iExpRate <= iMaxExp[i] )
				return iManufactureItemID[i];
		}

		_ASSERT_EXPR(0, L"[GetManufactureItemID] can not find ItemID");
		return -1;
	}
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )


#ifdef PRE_ADD_CASHFISHINGITEM
struct TFishingMeritInfo
{
	int nRodItemID;

	bool bUseCashBait;
	INT64 biBaitSerial;
	int nBaitIndex;

	int nMeritReduceFishingTime;
	int nMeritSuccessRate;
};
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

struct TFarmCultivateTableData
{
	int	iCultivateMaxTimeSec;
	int	iMaxWater;
	int iConsumeWater;
	int iHarvestNeedItemID;
	int iHarvestDropRate[Farm::Max::HARVESTITEM_COUNT];
	int iHarvestDropTableID;
	int iSkillPointPlant;			// EXP when planting seed
	int iSkillPointWater;			// EXP when watering
	int iCatalystCount;				// Required growth accelerator count
	SecondarySkill::Grade::eType	RequiredSkillGrade;
	int								iRequiredSkillLevel;
	int	iOverlap;
};

struct TFishingTableData
{
	int nMaxTime;
#ifdef PRE_ADD_CASHFISHINGITEM
	int nAutoMaxTime;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
	int nPullingTargetMin;
	int nPullingTargetMax;
	int nPullingTargetSpeed;
	int nPullingTargetUpTime;
	int nSkillPointSuccess;
	int nSkillPointFailure;
	int nSuccessProbability;
	int nSuccessDropTableID;
};

struct TFishingPattern
{
	int nPatternID;
	int nProbabillity;
};

struct TFishingPointTableData
{
	int nSecondarySkillClass;			//Fishing assist skill grade
	int nSecondarySkillLevel;			//Fishing assist skill level
	int nRequireItemType1;
	int nRequireItemType2;
	TFishingPattern Pattern[Fishing::Max::FISHINGPATTERNMAX];
};

struct TFishingAreaTableData
{
	int nMatchedMapID;
	int nMatchedFishingAreaID;
	int nFishingPointID;
};

struct TJobTableData
{
	char	cJobNumber;
	char	cBaseClass;
	char	cParentJob;
	float	afMaxUsingSP[ 5 ];
	char	cClass;
};

struct TGlyphSkillData
{
	int nGlyphID;
	int nGlyphType;
	int nSkillID;
	int nSkillLevelID;
};

struct TGlyphSlotData
{
	int nID;
	char cSlotID;
	char cLevelLimit;
	char cSlotType;
	bool IsCash;
};

struct TGlyphChargeData
{
	char cGlyphType;		// 1: Enhancement Heraldry, 2: Skill Heraldry, 3: Special Skill Heraldry (eGlyphType)
	char cGlyphRank;		// 1: Magic, 2: Rare, 3: Epic, 4: Unique (eItemRank)
	int nCharge;
};

struct TCharmItem
{
#if defined(PRE_FIX_68828)
	int nCharmID;	// Charm table ID
#endif
	int nItemID;	// Item
	int nCount;		// Count for user to acquire
	int nProb;		// Probability
	int nPeriod;	// Duration
	int nAccumulationProb;
	int nGold;
	bool bMsg;
};

struct TCharmItemData
{
	int nCharmNo;	// Charm item number
	std::vector<TCharmItem> CharmItemList;
};

const BYTE CharmItemKeyMax = 5;
struct TCharmItemKeyData
{
	int nCharmItemID;	// Box item ID
	std::vector<int> nKeyList;
};

struct TCharmCountData
{
	int nID;
	int nMin;
	int nMax;
};

#if defined(PRE_ADD_TALISMAN_SYSTEM)
struct TTalismanSlotData
{
	int nID;
	bool bService;
	char cLevelLimit;
	int nAmount;
	int nItemID;
	int nItemCount;
	float fEfficiency;
	char cType;
};

struct TTalismanData
{
	int nItemID;
	char cType;
	int nIndex;
	float fExpectation;
};
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

#if defined (PRE_ADD_CHAOSCUBE)
struct TChaosStuffItem
{
	int nItemID;
	int nCount;
	int nChaosNo;
};

struct TChaosResultItem
{
	int nItemID;	// Item
	int nCount;		// Count for user to acquire
	int nProb;		// Probability
	int nPeriod;	// Duration
	int nAccumulationProb;
	int nGold;
	bool bMsg;
};

struct TChaosItemData
{
	int nChaosNo;
	std::vector<TChaosResultItem> ChaosItemList;

};
#endif // #if defined (PRE_ADD_CHAOSCUBE)

//ServerMonitor
enum eServerMonitorLevel
{
	SERVERMONITOR_LEVEL_NONE = 0,
	SERVERMONITOR_LEVEL_MONITOR,				//CCU info and server status
	SERVERMONITOR_LEVEL_ADMIN,					//CCU info, server status, and process start only allowed
	SERVERMONITOR_LEVEL_SUPERADMIN,				//CCU info, server status, and full control allowed
};

enum eServerMonitorNotice
{
	SERVERMONITOR_UPDATENOTICE_CHANNELINFO,
	SERVERMONITOR_UPDATENOTICE_SERVERINFO,
	SERVERMONITOR_UPDATENOTICE_MERITINFO,
	SERVERMONITOR_UPDATENOTICE_WOLRDINFO,
};

enum eSIDConnectionType
{
	SID_CONNECTION_TYPE_LOG = 1,
	SID_CONNECTION_TYPE_DB,
};

struct TPromotionData
{
	int nID;
	int nConditionType;				//ePromotionConditionType
	int nConditionValue;
	int nPromotionType;				//ePromotionType
	int nRewardValue;				//Percent
};

struct TUserPromotionData
{
	int nType;		//ePromotionType
	int nValue;		//Percent
};

struct TSchedule
{
	char cHour;
	char cMinute;
	int nCycle;
};


struct TGuildLevel
{
	int nLevel;
	int nReqGold;
	int nReqGuildPoint;
	int nDailyGuildPointLimit;
	int nDailyMissionPointLimit;
	bool bApplicable;
};

struct TGuildWarPoint
{
	int nRank;
	int nRewardGuildPoint;
};

struct TGuildWarMapInfo
{
	UINT uiMapTableID;
	char cGuildWarUseMonth;
};

struct TGuildWarEventInfo
{
	//char cEventStep;
	__time64_t tBeginTime;
	__time64_t tEndTime;
};
// Guild War reward mail related
struct TGuildWarRewardData
{
	char cType;			// Reward type (eGuildWarRewardType)
	char cClass;		// Class (Warrior, Archer, Sorceress, Cleric)
	int nMailID;		// Mail ID
	int nPresentID;		// Gift box ID
	int nGuildFestivalPoint;	// Guild festival points to award in finals
	int nGuildPoint;			// Guild points to award in finals
};


struct TPeriodQuestDate
{
	int nPeriodType;
	__time64_t tAssignDate;
};

struct TExchangeTrade
{
	int nItemID;
	int nCategoryType;
	int nGroupID;
	int ExchangeType;
};

struct TPvPMissionRoom
{
	int nItemID;
	int nGroupID;
	int nMapID;
	int nModeID;
	int nWinCondition;
	int nPlayTime;
	int nMaxPlayers;
	int nStartPlayers;
	int nRoomNameIndex;
	bool bDropItem;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	int nChannelType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
};

typedef std::vector<TPvPMissionRoom> TVecPvPMissionRoom;

struct TGhoulConditionData
{
	int nItemID;
	int nItemIdx;
	int nPlayerMaxNum[PvPCommon::Common::GhouConditionColCount];
	int nGhoulCount[PvPCommon::Common::GhouConditionColCount];
	int nMatchingMutationGroupID[PvPCommon::Common::GhouConditionColCount];
};

struct TMonsterMutationGroup
{
	int nGroupID;
	int nGourpIdx;
	int nMonsterID[PvPCommon::Common::MonsterMutationColCount];
	int nProbability[PvPCommon::Common::MonsterMutationColCount];
};

struct TMonsterMutationData
{
	int nMutationID;
	int nMutationIdx;
	int nMutationActorID;

	int nMutationHP;
	int nMutationStrPMax;
	int nMutationStrMMax;
	int nMutationDefP;
	int nMutationDefM;
	int nMutationStiff;

	int nMutationStiffResis;
	int nMutationCriticalResis;
	int nMutationStunResis;

	int nMutationWeaponIdx1;
	int nMutationWeaponIdx2;
	int nMutationSkillTableID;
	int nMutationSuperArmor;

	int nSizeMin;
	int nSizeMax;
	int nMoveSpeed;
};

struct TMonsterMutationSkill
{
	int nMonsterID;
	int nMonsterIdx;
	int nMonsterSkillIndex[PvPCommon::Common::MonsterMutationSkillColCount];
	int nMonsterSkillLevel[PvPCommon::Common::MonsterMutationSkillColCount];
};

struct TOccupationModeInfo
{
	int nVictoryCondition[PvPCommon::Common::MaximumVitoryCondition];		//PvPCommon::BattleGroundVictoryState
	int nVictoryParam[PvPCommon::Common::MaximumVitoryCondition];		//PvPCommon::BattleGroundVictoryState
	int nBossID[PvPCommon::TeamIndex::Max];									//Spawn boss ID
	int nBossSpawnAreaID[PvPCommon::TeamIndex::Max];						//Spawn position when creating boss
};

struct TBattleGourndModeInfo
{
	int nID;
	int nWaitingMin;
	int nFinishingMin;
	int nPvPSkillPoint;	
	int nMaximumResourceLimit;
	int nAllOcuupationBonusGain;
	int nOccupationBonusGain[PvPCommon::Common::MaximumCapturePosition];

	int nCaptureScore;
	int nStealScore;
	int nKillScore;
	int nClimaxTime;
	int nClimaxRespawnTime;

	TOccupationModeInfo ModeInfo;
};

struct TPositionAreaInfo
{
	int nID;
	int nMapID;
	int nPvPModeID;
	int nAreaID;
	int nGainResourceTermTick;			//Acquisition interval on capture
	int nGainResource;					//Acquisition amount per interval
	int nRequireTryTick;				//Required click hold duration when attempting acquisition
	int nCompleteOccupationTick;		//Wait time after click completes before occupation ownership transfers
	int nBonusBuffID;
	//Climax~
	int nClimaxGainTermTick;
	int nClimaxGainVal;
	int nClimaxTryTick;
	int nClimaxCompleteOccupationTick;
};

struct TBattleGroundEffectValue
{
	int nSkillID;
	int nSkillLevel;
	int nNeedSkillPoint;			//Points required for acquisition
	int nUseResPoint;			//Occupation war points consumed when using skill
	std::string strEffectValue[PvPCommon::Common::MaximumEffectCount];
	int nEffectValueDuration[PvPCommon::Common::MaximumEffectCount];
	//int nMonsterID;
	//int nSummonDist;
	std::string strActionName;
	int nCoolTime;

	void Reset()
	{
		nSkillID = 0;
		nSkillLevel = 0;
		nNeedSkillPoint = 0;			//Points required for acquisition
		nUseResPoint = 0;			//Occupation war points consumed when using skill		
		for (int i = 0; i < PvPCommon::Common::MaximumEffectCount; i++)
		{
			strEffectValue[i].clear();;
			nEffectValueDuration[i] = 0;
		}

		//nMonsterID = 0;
		//nSummonDist = 0;
		strActionName.clear();
		nCoolTime = 0;
	}
};

struct TBattleGroundSkillInfo
{
	int nSkillID;
	int nSkillMaxLevel;
	int nSkillType;				//CDnSkill::SkillTypeEnum // Conquest skills are not much affected by skill type, but read in case needed for future expansion
	int nSkillDurationType;		//CDnSkill::DurationTypeEnum
	int nSkillTargetType;		//CDnSkill::TargetTypeEnum
	int nEffectType[PvPCommon::Common::MaximumEffectCount];
	int nEffectApplyType[PvPCommon::Common::MaximumEffectCount];
	int nProcess;				//Use only one process type
	int nCanDuplicate;			//Stackable?
};

struct TBattleGroundSkill
{
};

struct TUnionReputeBenefitData
{
	int nItemID;
	int nPeriod;
	int nBenefitType[NpcReputation::Common::MaxBenefitCount];
	int nBenefitNum[NpcReputation::Common::MaxBenefitCount];
};

struct TGuildMarkData
{
	int nItemID;
	int nType;
	bool bCash;
	bool bMarkOnly;
	bool bMarkView;
};

struct TPlayerCommonLevelTableInfo
{
	int iFatigue10000Ratio;	// 10000x multiplier
	float fMasterGainExp;
	//int nMasterGainItemID;
	//int nMasterGainItemCount;
	float fAddGainExp;
	float fDefense;
	float fCritical;
	float fFinalDamage;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	int nTalismanCost;
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)
};

struct TMasterSysFeelTableInfo
{
	int nMinFeel;
	int nMaxFeel;
	int nAddExp; //(% value)
};

struct TGlobalEventData
{
	int nItemID;
	int nCollectItemID;
	int nScheduleID;
	int nCollectTotalCount;
	int nNoticeCount[MAX_NOTICE_WORLDEVENT];
	bool bCheckNotice[MAX_NOTICE_WORLDEVENT];

	time_t tCollectStartDate;
	time_t tCollectEndDate;
	time_t tRewardDate;
};

struct TEveryDayEventData
{
	int nEventID;
	int nEventOnOff;
	int nSquenceDateCount;
	int nMailID;
	int nCashMailID;
	int nNumber;
	time_t tStartDate;
	time_t tEndDate;
};

enum eDBJobSystem  //g_szDBJobSystem
{
	DBJOB_GUILDWAR_INIT = 0,		// Initialize Guild War
	DBJOB_GUILDWAR_STATS,		// Guild War statistics
	DBJOB_SYSTEM_MAX,
};

enum eDBJobSystemStatus
{
	JOB_STATUS_RESERVE,		// Reserved
	JOB_STATUS_COMPLETED,	// Complete
	JOB_STATUS_DOING,		// Running
	JOB_STATUS_FAIL,		// Failed
};

struct TGuildRewardItemData
{
	int		nItemID;			// ID in guild reward table
	short	nNeedGuildLevel;	// Guild level required to purchase
	int		nNeedGold;			// Gold required to purchase
	int		nItemType;			// Guild reward item type
	int		nTypeParam1;
	int		nTypeParam2;
	bool	bCheckInven;		// Check if already purchased
	bool	bEternity;			// 0: Timed, 1: Permanent
	int		nPeriod;			// Duration
	bool	bCheckMaster;		// Whether to check master
	int		nCheckType;			// Check required condition (0:NONE 1:ITEMID 2:GuildMark
	int		nCheckID;			// Required previous item ID
};
struct TExpData 
{
	int nExperience;
	int nEventExperience;
	int nPcBangExperience;
	int nVIPExperience;
	int nPromotionExperience;
	int nItemExperience;
	int nGuildExp;
	TExpData()
	{
		nExperience = 0;
		nEventExperience = 0;
		nPcBangExperience = 0;
		nVIPExperience = 0;
		nPromotionExperience = 0;
		nItemExperience = 0;
		nGuildExp = 0;
	}
	void set(float Exp, float EventBonusExp = 0, float PcBangExp = 0, float VIPExp = 0, float PromotionBonusExp = 0, float ItemExp = 0, float GuildExp = 0)
	{
		nExperience = (int)Exp;
		nEventExperience = (int)EventBonusExp;
		nPcBangExperience = (int)PcBangExp;
		nVIPExperience = (int)VIPExp;
		nPromotionExperience = (int)PromotionBonusExp;
		nItemExperience = (int)ItemExp;
		nGuildExp = (int)GuildExp;
	}
};
#if defined(PRE_ADD_SALE_COUPON)
struct TSaleCouponData
{
	int nItemID;							// Coupon ID
	int nUseItemSN[MAX_SALE_USEITEM];		// Item SN to which the coupon applies
};
#endif

#if defined( PRE_PARTY_DB )

namespace Party
{
	struct QueryOutPartyType
	{
		enum eCode
		{
			Normal = 0,
			Disconnect,
			Kick,
			EternityKick,
		};
	};

	struct AddPartyAndMemberGame
	{
		Party::Data PartyData;		
		UINT nKickedMemberList[PARTYKICKMAX];
		INT64 biCharacterDBID[PARTYMAX];
		UINT nSessionID[PARTYMAX];	
#if defined( PRE_ADD_NEWCOMEBACK )
		bool bCheckComebackAppellation[PARTYMAX];
#endif
	};

	struct PartyMemberVillageData
	{
		UINT nAccountDBID;
		INT64 biCharacterDBID;
		BYTE cMemberIndex;
		BYTE cVoiceAvailable;
#if defined( PRE_ADD_NEWCOMEBACK )
		bool bCheckComebackAppellation;
#endif
		UINT nMutedList[PARTYCOUNTMAX];
	};
	struct AddPartyAndMemberVillage
	{
		Party::Data PartyData;
		UINT nVoiceChannelID;
		WCHAR wszPartyName[PARTYNAMELENMAX];
		UINT nKickedMemberList[PARTYKICKMAX];
		PartyMemberVillageData MemberData[PARTYMAX];
	};
};

#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
struct TNamedItemData
{
	int nItemID;		// Named item ID
	int nMaxCount;		// Max count
	int nSwapItemID;	// Replacement item ID
};
namespace EffectSkillNameSpace
{
	struct SearchType
	{
		enum eType
		{
			ItemType = 0,
			ItemID,
			SkillID,			
		};
	};	
	struct BuffType
	{
		enum eType
		{
			Partybuff = 0,
			SelfBuff,
		};
	};
	struct ShowEffectType
	{
		enum eType
		{
			NONEEFFECT = 0,
			BUFFEFFECT,
			SPELLEFFECT,
		};
	};
};
#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
struct TPotentialTransferData
{
	int nItemID;
	int nExtractItemLevel;
	int nExtractItemRank;
	int nInjectItemLevel;
	int nInjectItemRank;
	int nConsumptionCount;
	int nMainType;
	int nSubType;
};
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL

#if defined(PRE_ADD_INSTANT_CASH_BUY)
const int CashBuyShortcutMax = 10;
struct TCashBuyShortcutData
{
	int nType;
	int nAllowMapTypes;
	int nShow;
	std::vector<int> nSNList;
};
#endif	// #if defined(PRE_ADD_INSTANT_CASH_BUY)

#if defined(PRE_ADD_EXCHANGE_ENCHANT)
struct TExchangeEnchantData
{
	BYTE cRank;	// Item grade
	BYTE cLevelLimit;	// Level limit
	BYTE cEnchantLevel;	// Enhancement level
	int nNeedCoin;	// Fee
	int nNeedItemID1;	// Required item 1
	short wNeedItemCount1;	// Required item count 1
	int nNeedItemID2;	// Required item 2
	short wNeedItemCount2;	// Required item count 2
};
#endif //#if defined(PRE_ADD_EXCHANGE_ENCHANT)

#if defined( PRE_WORLDCOMBINE_PARTY )
namespace WorldCombineParty
{
	enum eType
	{
		MAXSKILLCOUNT = 2,
	};

	struct WrldCombinePartyData
	{
		BYTE cIndex;				// Index
		BYTE cGroupIndex;			// Group index
		WCHAR wszPartyName[PARTYNAMELENMAX];
		int nWorldMap;			// World zone
		int nTargetMap;			// Target map (reference value)
		ePartyType PartyType;	// Party type
		BYTE cPartyMemberMax;
		int nItemID;			// Entry ticket item
		TDUNGEONDIFFICULTY Difficulty;
		TPARTYITEMLOOTRULE ItemLootRule;			//Refer to ePartyItemLootRule
		TITEMRANK ItemRank;				//Item rank (refer to loot rule design)
		BYTE cUserLvLimitMin;		//It says there is a min/max value.
		int iBitFlag;
		int nSkillID[MAXSKILLCOUNT];				// Skill ID		
	};		
}
#endif

#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
struct TPcBangRentItem
{
	int nItemID;
	BYTE cLevelStart;
	BYTE cLevelEnd;
	BYTE cClassID;
	BYTE cJob;
	BYTE cGrade;
	int nItemOption;
};
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct TPrivateChatChannelInfo
{	
	INT64 nPrivateChatChannelID;
	WCHAR wszName[PrivateChatChannel::Common::MaxNameLen];
	INT64 biMasterCharacterDBID;
	int nMemberCount;
	int nPassWord;	
};

struct TPrivateChatChannelMember
{
	time_t tJoinDate;
	UINT nAccountDBID;
	INT64 biCharacterDBID;	
	bool bMaster;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TPrivateMemberDelServer
{
	INT64 nPrivateChatChannelID;	
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
};
#endif

#if defined(PRE_ADD_WEEKLYEVENT)
namespace WeeklyEvent
{
	struct TWeeklyEventData
	{
		int nStartTime;
		int nEndTime;
		BYTE cRaceType;		// eRaceType
		BYTE cClassType;	// eClass
		int nEventType;
		int nValue;			// % value
	};

	struct TWeeklyEvent
	{
		int nDayOfWeek;		// eDayType
		std::vector<TWeeklyEventData> VecEventData;
	};

	enum eDayType
	{
		Monday = 1,
		Tuesday = 2,
		Wednesday = 3,
		Thursday = 4,
		Friday = 5,
		Saturday = 6,
		Sunday = 7,
	};

	enum eRaceType
	{
		Player = 1,
		Monster = 2,
	};

	enum eEventType
	{
		Event_1 = 1,			// 1. HP increase and decrease
		Event_2 = 2,			// 2. Buff and debuff attack power
		Event_3 = 3,			// 3. Defense buff and debuff
		Event_4 = 4,			// 4. Increase enhancement rate
		Event_5 = 5,			// 5. EXP boost
		Event_6 = 6,			// 6. Party maintenance EXP boost
		Event_7 = 7,			// 7. Friend EXP boost (mutual friends, not best friends)
		Event_8 = 8,			// 8. Affinity increase
		Event_9 = 9,			// 9. Alliance point gain boost
		Event_10 = 10,			// 10. Increase item drop rate
	};
}
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
namespace TotalLevelSkill
{
	struct TTotalSkillLevelTable
	{
		int nSkillID;
		int nTotalLevelLimit;
		int nUseLevelLimit;
		int nSkillType;		
	};
	struct TTotalSkillBlowTable
	{
		int nSkillID;
		int nBlowID;
		float fBlowValue;
		int nBlowValue;		// Used for farm item ID
	};
	struct TTotalSkillSlotTable
	{
		int nSlotIndex;
		int nLevelLimit;
		bool bCash;
	};
}
#endif

#if defined(PRE_SPECIALBOX)
struct TProvideItemData
{
	int nCashSN;
	int nItemID;
	int nCount;
	int nSealCount;
	int nOption;
};

struct TKeepBoxProvideItemData
{
	int nType;
	std::vector<TProvideItemData> VecProvideItem;
};
#endif	// #if defined(PRE_SPECIALBOX)

#ifdef PRE_ADD_JOINGUILD_SUPPORT
struct TGuildSupportData
{
	int nID;
	int nJoinMinLevel;			//Min level for first guild join
	int nJoinMaxLevel;			//Max level for first guild join
	int nFirstJoinMailID;		//Reward mail ID on initial join
	int nMaxLevelGuildSupportMailID;	//Reward ID on reaching max level in first-joined guild
};
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT

#if defined( PRE_FIX_67546 )

struct TChannelDBInfo
{
	int nDBConnectionID;
	BYTE cThreadID;
};

struct TDBConnectionInfo
{	
	int nThreadCount[THREADMAX];
};

#endif
#if defined( PRE_WORLDCOMBINE_PVP )

struct TWorldPvPMissionRoom
{
	int nItemID;	
	int nMapID;
	BYTE cModeID;
	int nWinCondition;
	int nPlayTime;
	BYTE cMaxPlayers;
	int nStartPlayers;
	int nRoomNameIndex;
	bool bDropItem;
	USHORT unRoomOptionBit;
	WorldPvPMissionRoom::Common::eReqType eWorldReqType;
	int nMinLevel;
	int nMaxLevel;
	int nChannelType;
	WCHAR wszRoomName[PARTYNAMELENMAX];
	int nRoomPW;
};

struct TWorldPvPRoomDBData
{
	int nDBRoomID;
	int nWorldID;
	int nServerID;
	int nRoomID;
	BYTE cModeID;
	WCHAR wszRoomName[PARTYNAMELENMAX];
	BYTE cMaxPlayers;
	int nMapID;
	int nWinCondition;
	UINT nPlayTimeSec;
	bool bBreakIntoFlag;
	bool bDropItem;
	bool bRegulationFlag;
	bool bObserverFlag;
	bool bRandomFlag;
	BYTE cMinLevel;
	BYTE cMaxLevel;
	int nPassWord;
	int nRoomMemberCount;
	BYTE cChannelType;
};

typedef std::vector<TWorldPvPMissionRoom> TVecWorldPvPMissionRoom;

#endif

#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
struct TBonusDropTable
{
	int nIndex;
	int nMapID; //Map ID
	int nDropType;
	int nNeedItemID[3]; //Required key item ID
	int nDropItemGroupID[3]; //Drop item group ID to be dropped
};
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)


#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
struct TItemDropEnchantData
{
	int nIndex; //IDX
	int nDropID; // Value matching ID in ItemDropTable
	int nEnchantCount; //Count how many enchants there are
	int nEnchantOption[20]; //Enchant info
	int nEnchantProb[20]; //Probability for an enchant to be selected
};
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)

#if defined( PRE_ALTEIAWORLD_EXPLORE )
struct TAlteiaWorldMapInfo
{
	int nIndex;
	int nMapID;
	TDUNGEONDIFFICULTY Difficulty;
};
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined( PRE_ADD_STAMPSYSTEM )
namespace StampSystem
{
	struct ChallengeType
	{
		enum eType
		{
			DailyQuest	 = 1,
			DailyMission = 2,
		};
	};

	struct RewardCount
	{
		enum eCount
		{
			Day3 = 3,
			Day5 = 5,
		};
	};

	struct TStampChallenge // Stamp achievement
	{
		BYTE cType;
		std::set<int> setAssignmentID; // Achievement list
		int nMailID4; // 3-day reward
		int nMailID7; // Day 5 reward
	};

	struct TStampTableData // Stamp table
	{
		INT64 biStartTime;
		std::vector<TStampChallenge> vChallengeList;
	};
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
struct TPrevPotentialData
{
	int nItemID; //Item ID
	INT64 nSerial; //Serial
	char cPotential; //Previous potential data
	char cNowPotential; //Newly granted potential
	char cPotentialMoveCount; //(Previous) Potential transfer count
	INT64 nPotentialItemSerial; //Code serial used to apply potential
	int nPotentialItemID; //Item ID used to apply potential
};
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)

struct TEffectItemData
{
	int nItemSN;
	int nItemID;
	int nCount;
	bool bFail;
	INT64 nGiftDBID;
};


#if defined( PRE_PVP_GAMBLEROOM )

struct TPvPGambleRoomData
{
	int nRoomIndex;	
	bool bRegulation;
	BYTE cGambleType;
	int nPrice;	
};

struct TPvPGambleRoomWinData
{
	INT64 biCharacterDBID;
	int nPrice;
};

#endif // #if defined( PRE_PVP_GAMBLEROOM )

#if defined(PRE_ADD_EQUIPLOCK)
struct TDBLockItemInfo
{
	BYTE cItemCode;
	BYTE cItemSlotIndex;
	INT64 biItemSerial;
	EquipItemLock::TLockItem ItemData;
};
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined( PRE_ADD_STAGE_WEIGHT )
struct TStageWeightData
{
	float fHPRate;
	float fAttackRate;
	float fSuperArmorRate;
	float fItemDropRate;
};
#endif // #if defined( PRE_ADD_STAGE_WEIGHT )

#if defined( PRE_ADD_CHARACTERCHECKSUM )
struct CheckSumReason
{
	enum eReason
	{
		LogoutCharacter			= 1,
		C2C_KeepGameMoney		= 2,
		C2C_TransferGameMoney	= 3,
	};
};
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )

#if defined(PRE_ADD_STAGECLEAR_TIMECHECK)
namespace StageClearCheckTime
{
	enum eCheckMinTime	// Min clear time
	{
		CheckMinTime = 60 * 1000, // 1 minute
	};
	enum eAbuseCount // AbuseCount value to increment
	{
		AbuseCount = 10,
	};
	// Maps not using CheckTime (determined by boss map ID
	struct IgnoreMapIndex
	{
		enum eIgnoreMapIndex
		{
			MAP_CHAOSE_FIELD_BossA = 15157,	//Chaos Rift Parma
			MAP_CHAOSE_FIELD_32Lv_BossA = 15167,	//Chaos Rift Baira
			MAP_CHAOSE_FIELD_24Lv_BossA = 15174,	// Chaos Rift Kamala
		};
	};
};
#endif	// #if defined(PRE_ADD_STAGECLEAR_TIMECHECK)