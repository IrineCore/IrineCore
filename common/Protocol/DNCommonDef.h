#pragma once

#include "DNItemTypeDef.h"

#include "Version.h"
namespace SecondPassword
{
	static char* GetVersion()
	{
		return szVersion;
	}
}

/*--------------------------------------------------------------------------
Common defines shared by Client and Server
--------------------------------------------------------------------------*/
const int CLIENTPACKETMAX = 4096;

const char WORLDCOUNTMAX		= 110;	// Total number of worlds
const char VILLAGECOUNTMAX		= 12;	// Total number of village servers
const short VILLAGECHANNELMAX	= 255;	// Village channel max count
const short NETLAUNCHERLISTMAX	= 100;	// Max count for launcher reporting
const short MERITINFOMAX		= 100;	// Merit table max
const short STATECOUNTMAX		= 100;	// Max state for monitor reporting
const char GAMECOUNTMAX			= 64;	// Total number of game servers
const short REPORTGAMEINFOMAX = 300;	//Max count for sending game server population info

const unsigned char CHANNELNAMEMAX		= 128;		//Max channel name length
const unsigned char SERVERINFOCOUNTMAX = 30;
const int SERVERCONTROLMAX = 200;		// Max service control items in server monitor

const int EXCUTELENMAX = 64;			//
const int EXCUTECMDMAX = 2048 * 2;

const char SERVERVERSIONMAX		= 16;	// Server version length

const BYTE IPLENMAX				= 32;
const int MACHINEIDMAX			= 16;
const BYTE NHNNETCAFECODEMAX	= 12;
const BYTE MACADDRLENMAX		= 20;

const BYTE IDLENMAX				= 32;	// Account name

const int ONEDAYSEC	= 60*60*24;

const int SNDAOAPASSPORTMAX		= 1024;		// SNDA OA authentication passport max length (SNDA OA auth)

const int SECONDAUTHIGNOREPERIOD = 3;		//day
const BYTE PASSWORDLENMAX		= 31;
const BYTE DOMAINLENMAX			= 9;		// Asiasoft domain size (including null terminator) (which account?) THPP : Thai PlayparkID, THAS : Thai AsiaSoftID, THTC : Thai TCGID
const BYTE KREONKEYMAX			= 40;		// Kreon PC Cafe Key Max
const BYTE MAILRUOTPMAX			= 65;		// Russian auth OTP length (including NULL)

const BYTE CHARNAMEMIN			= 2;	// Minimum character name length
const BYTE CHARSIZEMAX			= 1;
const BYTE NAMELENMAX			= 17;	// Character name including null terminator
const BYTE WORLDNAMELENMAX		= 64;	// World name
const BYTE EXTDATANAMELENMAX	= 128;	// Length of names used in EXT DATA
const BYTE CHERRYLOGINIDMAX		= 81;	// Max Singapore/Malaysia login ID length (email allowed), including null terminator

const USHORT CHATLENMAX			= 512;	// chat max
const BYTE MAILNAMELENMAX		= 33;	// Character name for mail (including null terminator)

const int RESERVEDPERIOD_CHARNAME = 7; // Duration for which the previous character name is reserved
const int NEXTPERIOD_CHARNAME	= 7; // Cooldown period from last name change date until next allowed name change. Unit : day
const BYTE CHANGECODE_CHARNAME	= 2; // Character rename item usage code

const short PARTYNAMELENMAX		= 21;	// Party name max length 20
const int MAXPARTYPASSWORD		= 9999;
#if defined( PRE_PARTY_DB )
#else
const BYTE PARTYPASSWORDMAX		= 5;
#endif // #if defined( PRE_PARTY_DB )
//const char PARTYMAX    = 32; // Max party members (including PvP, Raid) Should rename this now? lol
const char PARTYMAX				= 48;	// Max party members (including PvP and raid) Maybe we should rename this now?

#if defined (PRE_ADD_BESTFRIEND)
const short BESTFRIENDMEMOMAX  =  40;
#endif // #if defined (PRE_ADD_BESTFRIEND)

const char PARTYCOUNTMAX		= 8;	// Use PARTYMAX for max member info; use PARTYCOUNTMAX for in-game Party MemberMax
const char NORMPARTYCOUNTMAX	= 4;	// Max normal party members
const char NORMPARTYCOUNTMIN	= 2;	// Min normal party members
const char RAIDPARTYCOUNTMAX	= 8;	// Max raid party members
const char RAIDPARTYCOUNTMIN	= 5;	// Min raid party members

enum ePartyType
{
	_NORMAL_PARTY		= 0,	//Normal 4-player party
	_RAID_PARTY_8		= 1,	//8-player party for raid
#if defined( PRE_WORLDCOMBINE_PARTY )
	_WORLDCOMBINE_PARTY	= 2,	// Cross-world party
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	_PARTY_TYPE_MAX
};

#if defined(PRE_EXPANDGATE)
const char WORLDMAP_GATECOUNT = 15;
#else
const char WORLDMAP_GATECOUNT = 10;
#endif
const char PARTYKICKMAX = 10;			//List carried over when transferring party info
const BYTE ISOLATELISTMAX = 100;		//Max number of blocked users
#ifdef PRE_PARTY_DB
#else
const int PARTYSORTCONDITIONMAX = 3;	// Max search filter arguments when filtering party list
#endif
const int PERMITMAPPASSCLASSMAX = 10;	//Max length of class ID list that bypasses map entry condition checks

const LONG GAMEINVITEWAITTIME = 1000 * 60 * 2;

const int PARTYLISTOFFSET		= 7;	//Party list page offset
const int QUICKPARTYCOUNTMAX	= 20;	//Maximum collection count for quick join

const char CHATROOMMAX			= 20;	// Max chat room capacity

const INT64 COINMAX				= 50000000000;	// Max coin value 4.2 billion

const INT64 GUILDWARE_COINMAX   = COINMAX / 2;

const BYTE UNSEQUENCEPACKET		= 0;	//Not a sequential packet
const BYTE COMPRESSPACKET		= 2;	//Compressed packet
const BYTE COMPRESSMINSIZE		= 30;	//Minimum size for compression; threshold to gain benefit from compressing
//Not an absolute figure; if all 10,000 bytes are non-sequential random numbers, it is GG

const short RESTRAINTREASONMAX	= 300;	//Max size for user sanction reason
const short RESTRAINTREASONFORDOLISMAX	= 100;	//Max size for user sanction reason (DOLIS display value)
const BYTE RESTRAINTMAX			= 10;	//Max sanction value

const BYTE PROFILEGREETINGLEN	= 24;	// Max length of profile introduction (greeting)
// item
const BYTE INVENTORYMAX			= 150;	// For normal item inventory
const BYTE WAREHOUSEMAX			= 150;	// For normal item storage
const BYTE QUESTINVENTORYMAX	= 60;	// For quest inventory

const BYTE DEFAULTINVENTORYMAX	= 30;	// Default inventory slot count
const BYTE DEFAULTWAREHOUSEMAX	= 10;	// Default storage slot count

const BYTE PERIODINVENTORYMAX	= 30;
const BYTE PERIODWAREHOUSEMAX	= 30;

const BYTE INVENTORYTOTALMAX	= INVENTORYMAX + PERIODINVENTORYMAX;
const BYTE WAREHOUSETOTALMAX	= WAREHOUSEMAX + PERIODWAREHOUSEMAX;

const BYTE CASHINVENTORYMAX		= 150;	// For cash item inventory
const BYTE CASHINVENTORYDBMAX	= 200;	// For cash item inventory
const BYTE CASHINVENTORYPAGEMAX = 100;	// Max value per page of cash inventory

const BYTE MAXDELEXPIREITEM = 150;

const BYTE VEHICLEINVENTORYPAGEMAX	= 50;	// Max slots per page in mount inventory

const BYTE MISSINGITEMMAX		= 50;	// 

#if defined( PRE_ADD_SKILLSLOT_EXTEND )
const BYTE QUICKSLOTMAX			= 60;	// Quick slot bar: 2 rows of 12 slots x 2 (base + extended skills) + 1 row (for gesture/social actions)
const BYTE QUICKSLOTSEPARATE	= 48;	// Gesture
#else // #if defined( PRE_ADD_SKILLSLOT_EXTEND )
const BYTE QUICKSLOTMAX			= 50;	// Quick slot bar: 2 rows of 10 slots + 1 row (for gesture/social actions)
const BYTE QUICKSLOTSEPARATE	= 40;	// Gesture
#endif	// #if defined( PRE_ADD_SKILLSLOT_EXTEND )

const BYTE SKILLMAX				= 100;	// Maximum number of skills
const BYTE GESTUREMAX			= 100;	// Max gesture count

const BYTE SKILLPRESETMAX		= 3;	// Max saved skill presets
const BYTE SKILLPRESETNAMEMAX	= 13;	// Max saved skill preset name length

const BYTE CASHGLYPHSLOTMAX		= 3;	//Max number of cash crest slots

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
const BYTE EFFECTSKILLMAX		= 50;	// Max count of effect skills
#endif
#if defined(PRE_ADD_KALI) && defined(PRE_ADD_ACADEMIC)
const BYTE CLASSKINDMAX			= 6;	// Warrior, Archer, Sorceress, Cleric, Academic, Kali
#elif defined(PRE_ADD_ACADEMIC)
const BYTE CLASSKINDMAX			= 5;	// Warrior, Archer, Sorceress, Cleric, Academic
#else
const BYTE CLASSKINDMAX			= 4;	// Warrior, Archer, Sorceress, Cleric
#endif // #ifdef PRE_ADD_ACADEMIC
const BYTE JOBMAX				= 8;	// Max job change count(?) // This naming is quite confusing.. should probably rename it later...
const BYTE JOB_KIND_MAX			= 100;	// Max job class count
const BYTE CHARLEVELMAX			= 100;	// Max character level

const BYTE DailyCreateCharacterMax = 7;	// Number of characters that can be created per day

#if !defined(PRE_MOD_SELECT_CHAR)
const BYTE CreateCharacterDefaultCountMax = 4;	// Max character creation limit
#endif	// #if !defined(PRE_MOD_SELECT_CHAR)

#if defined(PRE_MOD_SELECT_CHAR)
const BYTE CHARCOUNTMAX			= 50;	// Max number of characters that can be created
#else	// #if defined(PRE_MOD_SELECT_CHAR)
const BYTE CHARCOUNTMAX			= 30;	// Max number of characters that can be created
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

#if defined(PRE_ADD_CHANNELNAME)
const BYTE CHANNELLANGUAGEMAX	= 4;	// Channel language name
#endif //#if defined(PRE_ADD_CHANNELNAME)
const BYTE MAPNAMELENMAX		= 64;	// Maximum number of map names

const BYTE DEFAULTITEMMAX		= 25;	// Max number of items given upon character creation
const BYTE DEFAULTPARTSMAX		= 10;	// Max number of default parts to compare upon character creation
const BYTE DEFAULTPOSITIONMAX	= 10;	// Max number of start positions upon character creation
const BYTE DEFAULTSKILLMAX		= 10;	// Skills granted on character creation
const BYTE DEFAULTUNLOCKSKILLMAX	= 20;	// Unlock skills given upon character creation
#if defined(PRE_ADD_SKILLSLOT_EXTEND)
const BYTE DEFAULTQUICKSLOTMAX	= 60;	// Max number of quickslots upon character creation
#else	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
const BYTE DEFAULTQUICKSLOTMAX	= 50;	// Max number of quickslots upon character creation
#endif	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
const BYTE WEAPONMAX			= 2;	// Maximum number of weapons
const BYTE DEFAULTGESTUREMAX	= 10;	// Number of default gestures (social actions) granted on character creation
const BYTE DEFAULTHAIRCOLORMAX	= 5;
const BYTE DEFAULTSKINCOLORMAX	= 4;
const BYTE DEFAULTEYECOLORMAX	= 5;

const BYTE SHOPTABMAX			= 6;	// Max number of shop tabs
const BYTE SHOPITEMMAX			= 60;	// Max number of shop items
//const BYTE REBUYITEMMAX   = 25; // Max rebuy item count
const BYTE SKILLSHOPITEMMAX		= 10;	// Maximum number of skill shop items

const BYTE SENDDROPITEMLISTMAX	= 30;	// Max DropItem count sent at once on join-in-progress

const BYTE SENDCHATROOMVIEWMAX  = 10;	// Max ChatRoomView count sent at once on join-in-progress

const BYTE MAILTITLELENMAX		= 30;	// Mail subject
const BYTE MAILDATELENMAX		= 20;	// Mail date
const BYTE MAILTEXTLENMAX		= 198;	// Mail body
const BYTE MAILBOXMAX			= 30;	// Mailbox max is 100 for now
const BYTE MAILATTACHITEMMAX	= 5;	// Max 5 attached items when composing mail
const char MAILPAGEMAX = 6;

const BYTE MARKETPAGEMAX		= 5;			// Max count per page
const BYTE MARKETMAX			= 5 * 10;		// Market list max (5 * 10 pages)
const int MYMARKETMAX			= 25;			// My market list

const int MARKETREGISTERMAX		= 100;			// Max market listings
const int MARKETSELLSUCCESSMAX	= 1000000000;	// Successful sale count
const int MARKETSELLFAILUREMAX	= 1000000000;	// Failed sale count
const int MARKETREGISTERTAXMAX	= 1000000000;	// Accumulated registration fee
const int MARKETSELLTAXMAX		= 1000000000;	// Accumulated sales fee

const int SALEABORTLISTMAX		= 30;		// Max size of discontinued sales list

const int CARTLISTMAX			= 10;		// Max shopping cart items
const int PREVIEWCARTLISTMAX	= 20;		// Max preview cart items
const int GIFTMESSAGEMAX		= 51;		// Gift window memo (unified to 50 because splitting by country is tedious)
const int GIFTBOXLISTMAX		= 100;		// Gift window
const int GIFTPAGEMAX			= 8;
const int COUPONMAX				= 31;		// Coupon
const int PACKAGEITEMMAX		= 20;
const int COUPONITEMMAX			= 10;		// Maximum number of coupon items
const int PUBLISHERORDERKEYMAX	= 32 + 1;		// Max length of publisher-issued order number

const BYTE SEARCHWORDMAX		= 3;	// Max number of search keywords
const BYTE SEARCHLENMAX			= 50;	// Search market
const BYTE EXCHANGEMAX			= 10;	// Private trade max value

const BYTE COMPOUNDITEMMAX = 10;	// Max number of jewels when synthesizing heraldry jewels

const BYTE DUNGEONCLEARMAX = 120;		// Dungeon clear count
const BYTE NESTCLEARMAX = 50;			// nest clear

const int MAXPARTYDICEGAMENUMBER = 100;	// Max dice roll value for party item distribution
const int NPCPRESENTMAX = 50;			// Max number of NPC gifts

const int MISSIONMAX = 10000;
const int APPELLATIONMAX = 10000;
const BYTE PERIODAPPELLATIONMAX = 50;	//Max count of timed titles
const int LASTMISSIONACHIEVEMAX = 3;

const int MISSIONMAX_BITSIZE = MISSIONMAX / 8;
const int TIMEEVENTMAX = 20;

const int PCBangMissionMax = 20;
const int DAILYMISSIONMAX = 15;		// Temporarily increase to 15.
const int WEEKLYMISSIONMAX = 15;	// Temporarily increase to 15.
const int GUILDWARMISSIONMAX = 15;	
const int GUILDCOMMONMISSIONMAX = 5;		
const int WEEKENDEVENTMISSIONMAX = 15;
const int WEEKENDREPEATMISSIONMAX = 15;
#if defined(PRE_ADD_MONTHLY_MISSION)
const int MONTHLYMISSIONMAX = 5;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

const int VIEWCASHEQUIPMAX_BITSIZE = 4;
const int APPELLATIONMAX_BITSIZE = APPELLATIONMAX / 8;

const int MISSIONREWARDITEMMAX = 3;

const int MAXDICERANGE = 1000;
const int VOICEUPDATETIME = 1000;

const int LIFESPANMAX = 5 * 365 * 24 * 60;	// 5 years (in minutes)
const int REMOVECASHWAITINGTIME = 7 * 24 * 60;	// 7 days (calculated in minutes)

const int ITEMDROP_DEPTH = 8;
const int MAXITEMDROPFIELD	= 20;	// Max number of result fields that can drop during costume synthesis
const int MAXCOSMIXSTUFF	= 2;	// Max number of material items for costume synthesis

#if defined (PRE_ADD_BESTFRIEND)
const int BESTFRIENDMEMO_MAX = 40;	// Max size of best friend memo
const int BESTFRIEND_CLOSETIME = 4 * 24 * 60 * 60;	// 4 days (in seconds)
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_ALTEIAWORLD_EXPLORE )
const int MAXALTEIADICENUMBER = 6;	// Altea max dice size
#endif

enum eCostumeDesignMixStuffType
{
	eCDMST_ABILITY,
	eCDMST_DESIGN,
	eCDMST_MAX
};
#ifdef PRE_ADD_COSRANDMIX
const int MAX_COSTUME_RANDOMMIX_STUFF = 2;	// Max number of material items for costume random synthesis
#endif

#if defined (PRE_ADD_CHAOSCUBE)
const int MAX_CHAOSCUBE_STUFF = 10;
#endif

// Gachapon coin item ID sold as a cash item.
const int JP_GACHACOIN_ITEM_ID = 1107345408;

// Elixir item ID used as a permanently applied item.
const int ETERNITY_ITEM_STR_ITEM_ID = 302006280;
const int ETERNITY_ITEM_DEX_ITEM_ID = 302007304;
const int ETERNITY_ITEM_INT_ITEM_ID = 302008328;
const int ETERNITY_ITEM_CON_ITEM_ID = 302009352;
const int ETERNITY_ITEM_ALL_ITEM_ID = 302050312;
const int ETERNITY_ITEM_SP_ITEM_ID = 302051336;

const BYTE ITEMCATAGORY_MAX = 50;
const BYTE ITEMEXCHANGEGROUP_MAX = 50;
const BYTE ITEMJOB_MAX = 10;

const BYTE PVPMISSIONROOMGROUP_MAX = 50;

//community
#ifdef PRE_ADD_SHOWOPTION_PVPMARK
const BYTE DISPLAYOPTIONCOUNT = 15;
#else
const BYTE DISPLAYOPTIONCOUNT = 14;
#endif
const BYTE COMMDISPLAYOPTIONCOUNT = 3;
const BYTE COMMOPTIONCOUNT = 8;

//Friend
const int FRIEND_MAXCOUNT = 100;
const int FRIEND_GROUP_MAX = 10;
const int FRIEND_GROUP_NAMELENMAX = 50;
const int FRIEND_MEMO_LEN_MAX = 64;

//Chat
const int CHAT_PAPERING_CHECKTIME = 3;			// Within 3 seconds
const int CHAT_PAPERING_CHECKCOUNT = 3;			// If spoken 3 times,
const int CHAT_PAPERING_RESTRICTIONTIME = 30;	// 30-second penalty

//Gesture
const int GESTURE_PAPERING_CHECKTIME = 3;		// Within 3 seconds
const int GESTURE_PAPERING_CHECKCOUNT = 3;		// If used 3 times,
const int GESTURE_PAPERING_RESTRICTIONTIME = 10;// 10-second penalty
const int GESTURE_UNLOCKLV_CASH = -1;				// Unlock gesture via cash item

//Guild
const int GUILDDBID_DEF = 0;				// Guild DBID (None)
const int GUILDUID_DEF = 0;					// Guild UID (None)
const int GUILDSIZE_DEF = 20;				// Default guild size
const int SENDGUILDMEMBER_MAX = 100;		// Max guild members sent at once
const int GUILDSIZE_MAX = 300;				// Max guild size (= BaseSize + ExtraSize)
const int GUILDNAME_MAX = 20+1;	// 20 chars

const int GUILDNOTICE_MAX = 40;	// Max guild notice length
const int GUILDMONEY_MAX = 1000000000;		// Max guild funds
const int GUILDCREATECHARLEVEL_DEF = 9;		// Default minimum level to create guild
const int GUILDPARAMTEXT_MAX = 40;			// Max guild parameter text length
const int GUILDHISTORYTEXT_MAX = 40;		// Max guild history text size (also used by guild homepage, check when changing)
const int GUILDHISTORYLIST_MAX = 20;		// Max guild history list size
const int GUILDROLENAME_MAX = 6;			// Max guild role name length
const int GUILDMEMBINTRODUCE_MAX = 40;		// Max guild member self-introduction size
const int GUILDMNGRSIZE_DEF = 100;			// Default resource reserve size for guild manager
const int GUILDCMMPTR_MAX = 1000000000;		// Max guild normal points
const int GUILDLDDPTR_MAX = 1000000000;		// Max guild ladder points
const BYTE GUILD_WAREHOUSE_MAX = 150;		// Max guild storage size
const BYTE GUILD_WAREHOUSE_HISTORYLIST_MAX = 20; // Max guild storage history size
const int GUILDRECRUITINTRODUCE = 190;	// Guild recruitment board introduction
const int GUILDWARPOINT_DEF = 2500;			// Guild war base points
const short GUILDWARSCHEDULE_DEF = -1;
const int GUILDHOMEPAGE = 40;	// Guild recruitment board homepage URL

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
const int RANDOMGIVE_CHARMITEM_MAX = 100;
#endif		//#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE

//event
#if defined(PRE_ADD_WORLD_EVENT)
const short EVENTLISTMAX = 100;			//Max event setting value per world
#else
const short EVENTLISTMAX = 20;			//Max event setting value per world
#endif //#if defined(PRE_ADD_WORLD_EVENT)
const short EVENTCLEARBONUSMAX = 5;			//Max event stack count on stage clear

const int CHATROOMNAME_MAX		= 14;	// Max chat room name length
const BYTE CHATROOMPASSWORDMAX	= 4;		// Max chat room password length

const int ITEMSKILLMAXCOOLTIME = 7 * 24 * 60 * 60;// Maximum skill cooldown

enum eGuildMedalType						// Guild medal type
{
	GUILDMEDAL_TYPE_GOLD = 0,				// Gold / Fri
	GUILDMEDAL_TYPE_SILVER,					// is
	GUILDMEDAL_TYPE_BRONZE,					// East
	GUILDMEDAL_TYPE_CNT,
};

enum eGuildRoleType							// Guild rank type (P.S. New members must be appended at the end)
{
	GUILDROLE_TYPE_MASTER = 0,				// Guild master
	GUILDROLE_TYPE_SUBMASTER,				// Guild sub-master
	GUILDROLE_TYPE_SENIOR,					// Appoint
	GUILDROLE_TYPE_REGULAR,					// Official
	GUILDROLE_TYPE_JUNIOR,					// Recruit
	GUILDROLE_TYPE_CNT,
};

enum eGuildAuthType							// Guild authority type (P.S. New members must be appended at the end; stored as bit flags in int, so note total count limit)
{
	GUILDAUTH_TYPE_EXILE		= 0,				// Expel
	GUILDAUTH_TYPE_NOTICE		= 1,				// Notice
	GUILDAUTH_TYPE_INVITE		= 2,				// Invite
	GUILDAUTH_TYPE_BATTLE		= 3,				// Combat
	GUILDAUTH_TYPE_WAREHOUSE	= 4,				// Storage <Unused>
	GUILDAUTH_TYPE_SKILL		= 5,				// Skill

	GUILDAUTH_TYPE_STOREITEM	= 6,				// Store item
	GUILDAUTH_TYPE_TAKEITEM		= 7,				// Withdraw item
	GUILDAUTH_TYPE_WITHDRAW		= 8,				// Withdraw
	GUILDAUTH_TYPE_HOMEPAGE		= 9,				// Homepage changed 
	GUILDAUTH_TYPE_CNT,
};

enum eGuildUpdateType						// Guild info change type | INT_1 | INT_2 | INT_3 | INT_4 | INT64_1 | TEXT(40)		
{
	GUILDUPDATE_TYPE_NOTICE = 0,			// Change notice - - - - - Notice content
	GUILDUPDATE_TYPE_ROLENAME,				// Change rank name | RankNo | - | - | - | - | RankName
	GUILDUPDATE_TYPE_ROLEAUTH,				// Change rank permission | RankNo | AuthFlag | ItemLimit | CoinLimit | - | -
	GUILDUPDATE_TYPE_GUILDWAR,				// Guild war participation, win count increase | Participation count increase | Win count increase | - | - | - | -
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	GUILDUPDATE_TYPE_HOMEPAGE,				// Homepage changed - - - - - Homepage
#endif
	GUILDUPDATE_TYPE_CNT,
};

enum eGuildMemberUpdateType					// Guild member info change type | INT_1 | INT_2 | INT64_1 | TEXT(40)
{
	GUILDMEMBUPDATE_TYPE_INTRODUCE = 0,		// Change self-introduction (guild member self) - - - Introduction content
	GUILDMEMBUPDATE_TYPE_ROLE,				// Change rank (GuildMaster -> Member ?) | RankNo (excl. GuildMaster) | CharLevel | - | -
	GUILDMEMBUPDATE_TYPE_GUILDMASTER,		// Delegate guild master (Guild master -> Guild member ?) | - | - | - | -
	GUILDMEMBUPDATE_TYPE_CMMPOINT,			// Normal point inc/dec (contrib/use) Delta/Use (IN/OUT) Cumulative (OUT) - -
	GUILDMEMBUPDATE_TYPE_JOBCODE,			// Change guild member job | Job code
	GUILDMEMBUPDATE_TYPE_LOGINOUT,			// Guild member login/logout | Logout: _LOCATION_NONE
											//										Login :_LOCATION_MOVE
	GUILDMEMBUPDATE_TYPE_CNT,
};

enum eGuildHistoryType						// Guild history type | INT_1 | INT_2 | INT64_1 | TEXT(40)
{
	GUILDHISTORY_TYPE_BEGIN		=0,
	GUILDHISTORY_TYPE_CREATE	=1,				// Create guild | - | - | - | -
	GUILDHISTORY_TYPE_LEVELUP	=2,				// Guild level up | GuildLevel | - | - | -
	GUILDHISTORY_TYPE_JOIN		=3,				// Guild member joined | - | - | - | Character name
	GUILDHISTORY_TYPE_LEAVE		=4,				// Leave guild | - | - | - | Character name
	GUILDHISTORY_TYPE_EXILE		=5,				// Kick guild member | - | - | - | Character name
	GUILDHISTORY_TYPE_ROLECHNG	=6,				// Change guild member rank | Rank number | - | - | Character name
	GUILDHISTORY_TYPE_MEMBLEVELUP=7,			// Guild member level up (level 10) | Member level | - | - | Character name
	GUILDHISTORY_TYPE_ITEMENCT	=8,				// Guild member high enhancement success | Enhance level | Item ID | Member character ID | Character name
	GUILDHISTORY_TYPE_CLOSE		=9,				// Close guild - - -					
	GUILDHISTORY_TYPE_WARESIZE	=10,			// Change guild storage size | FinalSize | - | - | CharacterName	
	GUILDHISTORY_TYPE_RENAME	=11,			// Change guild name | - | - | - | OldName,NewName
	GUILDHISTORY_TYPE_RECRUITSTART	=12,		// Start guild recruitment | - | - | - | -
	GUILDHISTORY_TYPE_RECRUITEND	=13,		// End guild recruitment | - | - | - | -
	GUILDHISTORY_TYPE_CNT,

	// The value below is not used.
	GUILDHISTORY_TYPE_SKILLGAIN,			// Acquire guild skill | - | - | - | SkillName
	GUILDHISTORY_TYPE_SKILLENFC,			// Enhance guild skill | - | - | - | SkillName
	GUILDHISTORY_TYPE_MARKGAIN,				// Acquire guild mark | ForegroundID | BackgroundID | ClientImage | -
	GUILDHISTORY_TYPE_MARKCHNG,				// Change guild mark | ForegroundID | BackgroundID | ClientImage | -	
	GUILDHISTORY_TYPE_MISSION,				// Guild mission succeeded | MissionID | - | - | MissionName	
};

const char g_GuildHistoryLevel[GUILDHISTORY_TYPE_CNT] = {
	0,		// Guild start
	1,		// Create guild
	2,		// Guild level up
	3,		// Guild member joined
	4,		// Leave guild
	5,		// Kick guild member
	6,		// Change guild member rank
	7,		// Guild member level up
	8,		// Guild member high enhancement success
	9,		// Close guild member
	10,		// Change guild member storage size
	11,		// Change guild name
	12,		// Start guild recruitment
	13,		// End guild recruitment
};

enum eGuildRewardEffect				//Guild applied effect type
{
	GUILDREWARDEFFECT_TYPE_BEGIN					= 0,
	GUILDREWARDEFFECT_TYPE_GUILDITEM				= 1,		// Cosmetic item
	GUILDREWARDEFFECT_TYPE_MAKEGUILDMARK			= 2,		// Create guild mark
	GUILDREWARDEFFECT_TYPE_MAXGUILDWARE				= 3,		// Increase guild storage slots
	GUILDREWARDEFFECT_TYPE_MAXMEMBER				= 4,		// Increase guild member limit
	GUILDREWARDEFFECT_TYPE_EXTRAEXP					= 5,		// Additional EXP
	GUILDREWARDEFFECT_TYPE_EXTRAPVPEXP				= 6,		// Additional Colosseum EXP
	GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT			= 7,		// Additional guild festival points
	GUILDREWARDEFFECT_TYPE_EXTRAWAREGOLD			= 8,		// Deposit gold into guild storage after hunting ends--
	GUILDREWARDEFFECT_TYPE_EXTRAUNIONPOINT			= 9,		// Additional union points
	GUILDREWARDEFFECT_TYPE_MOVESPEED				= 10,		// Safe zone movement speed increase--
	GUILDREWARDEFFECT_TYPE_VEHICLESPEED				= 11,		// Increase mount movement speed--
	GUILDREWARDEFFECT_TYPE_REDUCEDURABILITYRATIO	= 12,		// Reduce durability consumption rate
	GUILDREWARDEFFECT_TYPE_WAREREPAIRCOST			= 13,		// Use repair cost from guild storage --
#if defined( PRE_ADD_GUILDREWARDITEM )
	GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE1			= 14,		// Order is STR, AGI, INT, VIT, but the server manages them as data without separate types.
	GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE2			= 15,		
	GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE3			= 16,		
	GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE4			= 17,		
#endif
	GUILDREWARDEFFECT_TYPE_CNT,	
};
enum eGuildRewardUpdateType
{
	GUILDREWARDEFFECT_ADDTYPE_NEW					= 0,		// Create
	GUILDREWARDEFFECT_ADDTYPE_Add					= 1,		// Add
	GUILDREWARDEFFECT_ADDTYPE_EXCHANGE				= 2,		// Replace
};
enum eGuildCheckType
{
	GUILD_REWARDITEM_CHECKTYPE_NONE					= 0,		// No condition
	GUILD_REWARDITEM_CHECKTYPE_PREVITEM				= 1,		// Previous item (guild effect item check)
	GUILD_REWARDITEM_CHECKTYPE_GUILDMARK			= 2,		// Guild mark
};
//date
const int DATELENMAX = 32;


//serverlauncher nickname max
const BYTE SRVLAUNCHERNICKNAMEMAX = 50;

//serverprocess authorize string max for servermonitorcenter connection
const int SRVPROCESSAUTHORSTRMAX = 100;

//announce message max size
const int ANNOUNCEMSG_MAX = 200;

const int MONITORSERVERNODESEND_MAX = 10;

const int MONITOR_PATCHPATH_MAX = 128;


// Maximum number of quest reward items
const int QUESTREWARD_INVENTORYITEMMAX = 6;

const int MAX_QUEST_LEVEL_CAP_REWARD = 2;
const int MAX_PERIODQUEST_LIST = 100;
const int MAX_PERIODQUEST_DISTRIBUTION = 10;
const int PERIODQUEST_WAITTIME = 6000;
const int MAX_PERIODQUEST_PROCESSMIN = 2; // 2 minutes
const int MAX_NOTICE_WORLDEVENT = 10;

const int PLAYER_MAX_LEVEL = CHARLEVELMAX;	// Same as CHARLEVELMAX... 

#if defined(PRE_ADD_SALE_COUPON)
const int MAX_SALE_USEITEM = 20;
#endif

#if defined(_CLIENT)
#ifdef PRE_MEMORY_SHUFFLE
#if !defined(_FINAL_BUILD)
#define SW_MSDT_ASSERT	// assert() occurs
#define SW_MSDT_NOTINCLUDE_COMPILESAFETY
#define SW_MSDT_ERRORCHECK_ORIGINVALUE
#endif	// _FINAL_BUILD
#include "MemoryShuffleDataType.h"
static MSDT_DECL(float) s_fDefaultFps = 60.f;
#else	// PRE_MEMORY_SHUFFLE
static float s_fDefaultFps = 60.f;
#endif	// PRE_MEMORY_SHUFFLE
#else	// _CLIENT
static float s_fDefaultFps = 60.f;
#endif	// _CLIENT

const int QUESTMEMOMAX = 10;
const int EVENTQUESTMAX = 20;

const int QUESTMEMOREWARDCHECK = QUESTMEMOMAX;		// Allocate specific quest memo index to record quest reward grant count and prevent infinite reward claims

typedef	int		TP_QUESTID;
typedef	char	TP_QUESTMEMO;

typedef	char	TP_JOB;

enum eStoreExternalType
{
	EXTERNALTYPE_MASTER = 1,
	EXTERNALTYPE_DATABASE,
	EXTERNALTYPE_CASH,
};


struct GameTaskType
{
	enum eType
	{
		Normal = 0,
		PvP,
		DarkLair,
		Farm,
		Max,
	};
};

enum eReqInfoType
{
	REQINFO_TYPE_SINGLE = 0,		// Single
	REQINFO_TYPE_SINGLE_SAMEDEST,	// When moving to the same village
	REQINFO_TYPE_PARTY,			// Party
	REQINFO_TYPE_PVP,				// PvP
	REQINFO_TYPE_PVP_BREAKINTO,	// PvP break-in (drop-in)
	REQINFO_TYPE_PARTY_BREAKINTO,	//Party invite (will drop-in join be added later?) 
	REQINFO_TYPE_FARM,				//Farm
	REQINFO_TYPE_LADDER,			// Ladder	
	REQINFO_TYPE_LADDER_OBSERVER,	// Ladder observer (drop-in)
};

enum eBridgeState {
	BridgeUnknown,
	LoginToVillage,
	VillageToGame,
	VillageToLogin,
	VillageToVillage,
	GameToVillage,
	GameToLogin,
	GameToGame,
	GameToGame_Direct,
	LoginToGame,		
	Exit,
};

enum eCommunityAcceptableOptionType
{
	_CA_OPTIONTYPE_ACCEPT = 0,
	_CA_OPTIONTYPE_DENYALL,
	_CA_OPTIONTYPE_FRIEND_GUILD_ACCEPT,
};

enum eGameOptionAcceptableCheckType
{
	_ACCEPTABLE_CHECKTYPE_PARTYINVITE,
	_ACCEPTABLE_CHECKTYPE_GUILDINVITE,
	_ACCEPTABLE_CHECKTYPE_TRADEREQUEST,
	_ACCEPTABLE_CHECKTYPE_DUELREQUEST,
	_ACCEPTABLE_CHECKTYPE_LADDERINVITE,
	_ACCEPTABEL_CHECKTYPE_QUICKPVPINVITE,
};

enum eError
{
	ERROR_UNKNOWN_HEADER = -3,		// Unhandled packet received (returning this disconnects the connection if it is a user)
	ERROR_INVALIDPACKET = -2,		// Abnormal packet (returning this disconnects the user connection)
	ERROR_DB = -1,					// General DB error
	ERROR_NONE = 0,					// ok	
	ERROR_NONE_ROWCOUNT	= 1,		// Leave 1 empty (DB ROWCOUNT check value)

	//Inter-server error messages up to roughly 100
	ERROR_GENERIC_INVALIDREQUEST = 2,		//Basic error, mostly for invalid requests
	ERROR_GENERIC_UNKNOWNERROR = 3,			//Unknown error.
	ERROR_GENERIC_DUPLICATEUSER = 4,		//Duplicate login
	ERROR_GENERIC_DUPLICATESESSIONID = 5,	//Duplicate session ID
	ERROR_GENERIC_USER_NOT_FOUND = 6,		//User does not exist.
	ERROR_GENERIC_DBCON_NOT_FOUND = 7,		//Database connection is unstable.
	ERROR_GENERIC_VILLAGECON_NOT_FOUND = 8,	//Server connection is unstable.
	ERROR_GENERIC_MASTERCON_NOT_FOUND = 9,	// Server connection is unstable.
	ERROR_GENERIC_GAMECON_NOT_FOUND = 10,	//Server connection is unstable.
	ERROR_GENERIC_WORLDFULLY_CAPACITY = 11, //World is full
	ERROR_GENERIC_HACKSHIELD = 12,			// HackShield error
	ERROR_GENERIC_TIME_WAIT = 13,			// Please try again in a moment.
	ERROR_GENERIC_LEVELLIMIT = 14,			// Level restriction 

	ERROR_DB_SYSTEM = 99,					// A database system error occurred.

	//UI String Match Enumulation
	//Login Message Starting at 100
	ERROR_LOGIN_FAIL = 100,									// "Login error."
	ERROR_LOGIN_VERSION_UNMATCH = 101,						// "Version mismatch."
	ERROR_LOGIN_ID_PASS_UNMATCH = 102,						// "Invalid ID or password."
	ERROR_LOGIN_SELECT_SERVER = 103,						// "Please select a server."
	ERROR_LOGIN_SELECT_CHARACTER = 104,						// "Please select a character."
	ERROR_LOGIN_SELECT_CHANNEL = 105,						// "Please select a channel."
	ERROR_LOGIN_CHARACTERNAME_ALREADY_EXIST = 106,			// "Character name already exists."
	ERROR_LOGIN_CHARACTER_DELETED = 107,					// "Character deleted."
	ERROR_LOGIN_ALREADY_LOGEDIN = 108,						// "Duplicate login."
	ERROR_LOGIN_CHARACTERLIST_ERROR = 109,					// "Failed to load character list."
	ERROR_LOGIN_CANNOT_CREATE_CHARACTER_ANYMORE = 110,		// "Cannot create any more characters."
	ERROR_LOGIN_PARTS_NOT_FOUND = 111,						// "Character parts do not exist."
	ERROR_LOGIN_CHARACTER_INDEX_UNMATCH = 112,				// "Character index mismatch."
	ERROR_LOGIN_CREATE_CHARACTER_FAIL = 113,				// "Failed to create character."
	ERROR_LOGIN_PROHIBITWORD = 114,							// Name cannot be created. (Forbidden word)
	ERROR_LOGIN_FCM_PROTECTED = 115,						// China fatigue warning (registration incomplete)
	ERROR_LOGIN_FCM_LIMITED = 116,							// China fatigue warning (minor)
	ERROR_117 = 117,										// Not within login hours.
	ERROR_LOGIN_ERASE_CHARACTER_FAIL_GUILDMASTER = 118,		// Cannot delete character because it is currently the guild master.
	ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY = 119,		// "Cannot create any more characters today."
	ERROR_LOGIN_HAIRCOLOR_NOT_FOUND = 120,
	ERROR_LOGIN_SKINCOLOR_NOT_FOUND = 121,
	ERROR_LOGIN_EYECOLOR_NOT_FOUND = 122,
	ERROR_LOGIN_SDOUA_NEED_INFO = 123,						// Chinese detailed info input required
	ERROR_LOGIN_AUTHCHECK_INVALID_ARGUMENT = 124,			//	1: Argument is not valid
	ERROR_LOGIN_AUTHCHECK_INVALID_PASSPORT = 125,			//	2: NexonPassport is not valid
	//ERROR_LOGIN_AUTHCHECK_ERROR_TIMEOUT = 126,			//	3: [obsolete] same as AUTHCHECK_ERROR_EXPIRED
	ERROR_LOGIN_AUTHCHECK_EXPIRED = 126,					//	3: NexonPassport is valid, but arleady expired
	ERROR_LOGIN_AUTHCHECK_AUTH_SERVER_SHUTDOWN = 127,		//	4: Authentication server is shutdown
	ERROR_LOGIN_AUTHCHECK_UNSUPPORTED_LOCALE = 128,			//	5: Unsupported Locale ID
	ERROR_LOGIN_AUTHCHECK_SET_LOCALE_NEEDED = 129,			//	6: call AuthCheck_SetLocale first
	ERROR_LOGIN_AUTHCHECK_NO_SERVER_INFORMATION = 130,		//	7: no server information in config file
	ERROR_LOGIN_AUTHCHECK_USER_IP_MISMATCHED = 131,			//	8: client ip address is mismatched with sso servers data
	ERROR_LOGIN_AUTHCHECK_NO_DECRYPTION_KEY = 132,			//	9: There is no passport decryption key. Check AuthCheck_LoadKey function or key file.
	ERROR_LOGIN_AUTHCHECK_LOW_LEVEL_PASSPORT = 133,			//	10: passport level is lower than minimum passport level
	ERROR_LOGIN_AUTHCHECK_GAMECODE_MISMATCHED = 134,		//	11: Mismatch between the caller's gamecode and the passport's gamecode.
	ERROR_LOGIN_AUTHCHECK_SESSION_NOT_EXIST = 135,			//	12: User session data not exists. ( Maybe timeout or something. )
	ERROR_LOGIN_AUTHCHECK_DISCONNECTED = 136,				//	13: User IP or session key missmatched with current login session. Maybe logged out by another session.
	ERROR_LOGIN_AUTHCHECK_SERVER_FAILED = 137,				//	100: Internal error.
	ERROR_LOGIN_CANNOT_CREATE_DWCCHARACTER_EXIST = 138,		// Already have a DWC character.
	ERROR_LOGIN_CANNOT_FIND_DWCCHARACTER = 139,				// Created a DWC character, but cannot find it in the character list.
	ERROR_LOGIN_DWC_WRONG_DATE = 140,						// Not during DWC period.
	ERROR_LOGIN_CANNOT_CREATE_DWCCHARACTER_LIMITLEVEL = 141,// Character level is insufficient to create a DWC character.

	//Character Message Starting at 200
	ERROR_CHARACTER_QUICKSLOT_NOTFOUND = 200,				// "Quick slot does not exist."
	ERROR_FCMSTATE = 201,									// Not allowed due to fatigue time (?)
	ERROR_CHARACTER_DATA_NOT_LOADED_SELF = 202,				// Your character data is still loading.
	ERROR_CHARACTER_DATA_NOT_LOADED_OPPOSITE = 203,			// Opponent character data is still loading.

	//Party Message Starting at 300
	ERROR_PARTY_CREATE_FAIL = 300,							//Failed to create party
	ERROR_PARTY_DELETE_FAIL = 301,							//Failed to delete party.
	ERROR_PARTY_JOINFAIL = 302,								//Cannot join the party.
	ERROR_PARTY_JOINFAIL_ZONLEVEL_SHORTAGE = 303,			//World zone level too low
	ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_SHORTAGE = 304,		//Character level insufficient
	ERROR_PARTY_JOINFAIL_PASSWORDINCORRECT = 305,			//Incorrect password
	ERROR_PARTY_LEAVEFAIL = 306,							//Failed to leave party
	ERROR_PARTY_STARTSTAGE_FAIL = 307,						//Cannot start the game.	
	ERROR_PARTY_INVITEFAIL = 308,							//Failed to invite.
	ERROR_PARTY_INVITEFAIL_ALREADY_HAS_PARTY = 309,			//Target is already in a party and cannot be invited.
	ERROR_PARTY_INVITEFAIL_ZONLEVEL_SHORTAGE = 310,			//Cannot invite because zone level is too low.
	ERROR_PARTY_INVITEFAIL_CHARACTERLEVEL_SHORTAGE = 311,	//Cannot invite because character level is too low.
	ERROR_PARTY_INVITEFAIL_ALREADY_INVITED = 312,			//Target has already been invited.
	ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND = 313,			//Target does not exist.
	ERROR_PARTY_KICKFAIL = 314,								//Kick failed
	ERROR_PARTY_LEADER_APPOINTMENTFAIL = 315,				//Failed to appoint party leader
	ERROR_PARTY_MOVECHANNELFAIL = 316,						//Failed to move channel
	ERROR_PARTY_JOINFAIL_INREADYSTATE = 317,				//Cannot join party while in ready state
	ERROR_PARTY_KICKOK = 318,								// Kick succeeded(?)
	ERROR_PARTY_JOINFAIL_PARTYMEMBER_ALREADY_MAX = 319,		//Cannot join party because it is full.
	ERROR_PARTY_JOINFAIL_YOU_ARE_KICKEDUSER = 320,			//Cannot rejoin a party you were kicked from.
	ERROR_PARTY_QUICKJOINFAIL_THERE_IS_NOMATCH_PARTY = 321,	//No suitable party to join~
	ERROR_PARTY_CREATE_FAIL_NEED_PARTYNAME = 322,			//Party name is required to create a party
	ERROR_PARTY_CREATE_FAIL_PASSWORD_ERR = 323,				//This game also has password restrictions, enter it properly
	ERROR_PARTY_CREATE_FAIL_RESTRICT_LEVEL_ERROR = 324,		//Enter level limit properly
	ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_OVER = 325,			//Character level limit
	ERROR_PARTY_KICKFAIL_DEST_IS_YOU = 326,					//Cannot kick self
	ERROR_PARTY_MODIFYINFO_FAIL = 327,						//Failed to change info
	ERROR_PARTY_MODIFYINFO_FAIL_NEED_NAME = 328,			//Party name is required to change
	ERROR_PARTY_MODIFYINFO_FAIL_PASSWORD_ERR = 329,			//Enter the password properly
	ERROR_PARTY_MODIFYINFO_FAIL_RESTRICT_LEVEL_ERROR = 330, //Enter restrictions properly
	ERROR_PARTY_LIST_ERROR = 331,							//Chati(Party) list error
	ERROR_PARTY_STARTSTAGE_FAIL_ACTLEVEL_SHORTAGE = 332,	//Cannot enter dungeon because world zone level is too low.
	ERROR_PARTY_STARTSTAGE_FAIL_FATIGUE_SHORTAGE = 333,		//Insufficient fatigue to enter the dungeon.
	ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED = 334,	//Target is not in a state to be invited
	ERROR_PARTY_STARTSTAGE_FAIL_DUNGEONNEEDITEM_SHORTAGE = 335,	//Insufficient dungeon entry item
	ERROR_PARTY_DUNGEONWARPOUT_FAIL_ITEMSHARING = 336,			//Warp failed after dungeon clear: party item distribution in progress
	ERROR_PARTY_DUNGEONWARPOUT_FAIL_QUEST = 337,				//Warp failed after dungeon clear: quest incomplete for a party member
	ERROR_PARTY_NOTFOUND_MEMBERINFO = 338,						//No party info found.
	ERROR_PARTY_CONDITION_MATCHED_LIST_NOT_FOUND = 339,			//No party matches the sort criteria.
	ERROR_PARTY_INVITEFAIL_DIFFCHANNEL = 340,					//Cannot invite to party if on a different channel.
	ERROR_PARTY_INVITEFAIL_DEST_USER_NOWREADY = 341,			//Cannot invite a character in ready state.
	ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME = 342,			//Invite target is in another village
	ERROR_PARTY_INVITEFAIL_DESTINVITE_DENIED = 343,				//Target declined the invitation.
	ERROR_PARTY_INVITEFAIL_ALREADYINVITED = 344,				//Invite in progress. Cannot re-invite
	ERROR_PARTY_INVITEAGREEFAIL = 345,							//Accepted the invite but failed to join the party
	ERROR_PARTY_ASKJOIN_FAIL = 346,								//General failure of participation application
	ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND = 347,					//Applicant is not in a party
	ERROR_PARTY_ASKJOIN_DEST_DIFFMAP = 348,						//Target is in another town.
	ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL = 349,				//Failed to accept party request
	ERROR_PARTY_ASKJOIN_RESULT_PARTYLEADER_DENIED = 350,		//Reject party request
	ERROR_PARTY_SWAPINDEX_FAIL = 351,							//Party location move failed
	ERROR_PARTY_REFRESHGATEFAIL_REQRETREAT = 352,				//Request step back on dungeon gate request during world zone party invite
	ERROR_PARTY_CANT_ENTERGATE = 353,							//Warp due to an issue entering the gate.
	ERROR_PARTY_ASKJOIN_DESTUSER_SITUATION_NOTALLOWED = 354,	//Target cannot currently receive party requests.
	ERROR_PARTY_PARTYLIST_OVERFLOW = 355,						// Requested page does not exist.
	ERROR_PARTY_GM_JOINFAIL_LADDERROOM = 356,					// Cannot follow user in ladder room. Please use ladder spectate. 
	ERROR_PARTY_REFRESHLIST_TIMERESTRAINT = 357,				// Party list request cooldown
	ERROR_PARTY_STARTSTAGE_FAIL_EXCEED_TRY_COUNT	= 358,		// Entry count exceeded
	ERROR_PARTY_STARTSTAGE_FAIL_NOT_AVAILABLE_TIME	= 359,		// Entry time limit


	//Item&shop Message Starting at 400
	ERROR_ITEM_FAIL = 400,							//Item error
	ERROR_ITEM_NOTFOUND = 401,						//Non-existent item
	ERROR_ITEM_INDEX_UNMATCH = 402,					//Index mismatch
	ERROR_ITEM_EQUIPMISMATCH = 403,					//Equip slot does not match.
	ERROR_ITEM_OVERFLOW = 404,						//Item count exceeded.
	ERROR_ITEM_INVENTORY_NOTENOUGH = 405,			//Insufficient inventory space.
	ERROR_ITEM_UNCOUNTABLE = 406,					//Not a stackable item.
	ERROR_ITEM_ALREADY_EXIST = 407,					//Item with the same index exists
	ERROR_ITEM_INSUFFICIENCY_MONEY = 408,			//Insufficient money
	ERROR_ITEM_SOULBOUND = 409,						//This is a bound item.
	ERROR_ITEM_EMBLEM_COMPOUND_FAIL = 410,			//Heraldry combination failed
	ERROR_ITEM_ITEM_COMPOUND_FAIL = 411,			//Item combination failed
	ERROR_ITEM_REPAIR = 412,						// Nothing to repair
	ERROR_ITEM_ENCHANTFAIL = 413,					// (This item cannot be enhanced)
	ERROR_ITEM_INSUFFICIENCYCOUNT = 414,			// Insufficient item count
	ERROR_ITEM_INVENTOWARE01 = 415,					// Exceeds the amount you have
	ERROR_ITEM_INVENTOWARE02 = 416,					// Exceeded maximum storable gold in storage
	ERROR_ITEM_WARETOINVEN01 = 417,					// Attempting to withdraw more gold than is in storage
	ERROR_ITEM_WARETOINVEN02 = 418,					// Cannot withdraw exceeding the maximum holdable gold
	ERROR_ITEM_CAN_NOT_DISJOINT = 419,				// Item cannot be disassembled!
	ERROR_ITEM_OVERFLOWMONEY = 420,					// Money overflow
	ERROR_ITEM_REBIRTH_CASHCOIN_SHORT_FAIL = 421,	// Cannot revive due to insufficient cash coins
	ERROR_ITEM_EMBLEM_COMPOUND_POSSIBILITY_FAIL = 422,	// Heraldry jewel combination attempt failed by probability.
	ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_MONEY = 423,	// Insufficient gold for heraldry jewel combination.
	ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_ITEM = 424,	// Insufficient material items for heraldry jewel combination
	ERROR_ITEM_EMBLEM_COMPOUND_NOT_MATCH_SLOT = 425,	// Slot mismatch in heraldry jewel combination.
	ERROR_ITEM_EMBLEM_COMPOUND_NOT_ENOUGH_ITEM_COUNT = 426, // Slot matches in heraldry jewel combination, but item count is insufficient.
	ERROR_ITEM_EMBLEM_COMPOUND_NOT_MATCH_PLATE_WITH_ITEM_COMPOUND_INDEX = 427,	// Input combination index not found in the plate's item combination list for heraldry jewel combination.
	ERROR_ITEM_ITEM_COMPOUND_POSSIBILITY_FAIL = 428,	// Item combination failed by probability.
	ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_MONEY = 429,	// Insufficient gold for item combination.
	ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM = 430,	// Insufficient material items for item combination
	ERROR_ITEM_ITEM_COMPOUND_NOT_MATCH_SLOT = 431,	// Slot mismatch in item combination.
	ERROR_ITEM_ITEM_COMPOUND_NOT_ENOUGH_ITEM_COUNT = 432, // Slot matches in heraldry jewel combination, but item count is insufficient.
	ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_BREAKITEM = 433,		// Enhancement rate failure: item destroyed
	ERROR_ITEM_REMOVE_DISALLOWANCE = 434,					// Indestructible
	ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_DOWNLEVEL = 435,		// Enhancement rate failure: enhancement level downgrade
	ERROR_ITEM_NEED_PARTY_SHARING_SOULBOUND = 436,			// Bound item requiring party distribution
	ERROR_ITEM_INSUFFICIENCY_ITEM = 437,					// Insufficient items
	ERROR_ITEM_PVPLEVEL = 438,								// Cannot equip due to insufficient PvP level
	ERROR_ITEM_UNABLE_EQUIP_NO_SOUL_BOUND = 439,			// Cannot equip because it is not unsealed
	ERROR_ITEM_UNSEAL_ALREADY = 440,						// Item already opened
	ERROR_ITEM_ENCHANT_POSSIBILITYFAIL_DOWNLEVELZERO = 441,	// Enhancement rate failure: enhancement level downgrade amount is 0
	ERROR_ITEM_REBIRTHCOIN_50 = 442,						// Up to 50 resurrection scrolls can be owned
	ERROR_ITEM_DONT_HAVE_SKILL_TO_RESET = 443,				// No skill to reset.
	ERROR_ITEM_ALREADY_HAVE_HIGHER_JOB_SKILL = 444,			// Already learned a skill from a higher job advancement than the job of the skill being reset.
	ERROR_ITEM_MISMATCH_RESET_SKILL_JOB = 445,				// The job tier of the skill to reset specified in the skill reset cash item does not match the current character's job.
	ERROR_ITEM_DONTMOVE = 446,								// Cannot move(?)
	ERROR_ITEM_SEAL_ALREADY = 447,							// Item already sealed
	ERROR_ITEM_SEALCOUNT_ZERO = 448,						// Reseal count is 0.
	ERROR_ITEM_COSMIX_FAIL = 449,							// Costume synthesis failed (general)
	ERROR_ITEM_COSMIX_FAIL_STUFF = 450,						// Costume synthesis failed (invalid material item)
	ERROR_ITEM_COSMIX_STUFF_COUNT = 451,					// Insufficient or excessive material count for costume synthesis
	ERROR_ITEM_COSMIX_OPEN_FAIL = 452,						// Failed to open costume synthesis window
	ERROR_ITEM_COSMIX_CLOSE_FAIL = 453,						// Failed to close costume synthesis window
	ERROR_ITEM_COSMIX_NO_RESULT = 454,						// Costume synthesis failed (invalid result item info)
	ERROR_ITEM_COSMIX_OPTION = 455,							// Costume synthesis failed (invalid option info)
	ERROR_GLYPH_SLOTLIMITLEVEL = 456,						// Character level is too low to place in slot
	ERROR_GLYPH_LIMITLEVEL = 457,							// Character level is too low to equip(?) the crest
	ERROR_ITEM_POTENTIAL_INVALIDTYPE = 458,					// Type cannot have potential applied.
	ERROR_GLYPH_SAMETYPEPARAM = 459,						// Same-series crest
	ERROR_ITEM_NEEDKEY = 460,								// Key required
	ERROR_ITEM_APPELLATIONGAIN_EXIST = 461,					// Title already exists.
	ERROR_ITEM_EQUIP_CRC_FAIL = 462,						// Failed to equip due to item data error
	ERROR_ITEM_CANNOT_USE_INPARTY = 463,					// This item cannot be used while in a party.
	ERROR_ITEM_CANNOT_CHANGE_SAMENAME = 464,				// Cannot change to the same name as the existing character name.
	ERROR_ITEM_ENCHANTJEWEL_INVALIDTYPE = 465,				// Item type cannot use direct enchant item.
	ERROR_ITEM_INSUFFICIENCY_LADDERPOINT = 466,				// Insufficient ladder points
	ERROR_ITEM_INSUFFICIENCY_UNIONPOINT = 467,				// Insufficient union points
	ERROR_ITEM_SEAL_FAIL_BY_RANDOM = 468,					// Seal failed after random roll.
	ERROR_ITEM_CANNOT_CHANGE_GUILDMARK = 469,				// Failed to change guild mark
	ERROR_ITEM_INSUFFICIENCY_GUILDWARPOINT = 470,			// Insufficient guild festival points
	ERROR_ITEM_INSUFFICIENCY_ITEMTYPE_REMOTEENCHANT = 471,	// No remote enhancement item
	ERROR_ITEM_DESIGNMIX_FAIL = 472,						// Costume design synthesis failed (general)
	ERROR_ITEM_DESIGNMIX_FAIL_STUFF = 473,					// Costume design synthesis failed (invalid material item)
	ERROR_ITEM_DESIGNMIX_STUFF_COUNT = 474,					// Insufficient or excessive material count for costume design synthesis
	ERROR_ITEM_DESIGNMIX_OPEN_FAIL = 475,					// Failed to open costume design synthesis window
	ERROR_ITEM_DESIGNMIX_CLOSE_FAIL = 476,					// Failed to close costume design synthesis window
	ERROR_ITEM_DESIGNMIX_NO_RESULT = 477,					// Costume design synthesis failed (invalid result item info)
	ERROR_ITEM_CHANGEJOB_ILLEGAL_STATE = 478,				// When not in a state where a job change item can be used	
	ERROR_ITEM_INSUFFICIENTY_JOBID = 479,					// Job restriction
	ERROR_ITEM_INSUFFICIENTY_LEVEL = 480,					// Level restriction
	ERROR_ITEM_INSUFFICIENTY_PVPRANK = 481,					// PvP rank restriction
	ERROR_ITEM_INSUFFICIENTY_GUILDLEVEL = 482,				// Guild level limit
	ERROR_ITEM_INSUFFICIENTY_LADDERGRADEPOINT = 483,		// Ladder rating restriction
	ERROR_ITEM_BUY_OVERFLOW_COUNT = 484,					// Exceeded purchasable quantity.
	ERROR_ITEM_CANNOT_CHANGE_SAME_PET_NAME = 485,			// Cannot change to the same name as the existing character name.
	ERROR_ITEM_ALREADY_EXPIRED = 486,						// Already destroyed.
	ERROR_ITEM_PREITEM_REQUEST = 487,						// Previous item is required.
	ERROR_ITEM_BETTEREFFECT_ITEM = 488,						// A better effect is currently applied.
	ERROR_ITEM_GUILDMARK_REQUIRE	= 489,					// Guild mark is required.			
	ERROR_ITEM_GUILDMARK_EXIST	= 490,						// Guild mark already exists.	
	ERROR_ITEM_RANDOMMIX_FAIL = 491,						// Costume random synthesis failed (general)
	ERROR_ITEM_RANDOMMIX_FAIL_STUFF = 492,					// Costume random synthesis failed (invalid material item)
	ERROR_ITEM_RANDOMMIX_STUFF_COUNT = 493,					// Insufficient or excessive material count for costume random synthesis
	ERROR_ITEM_RANDOMMIX_OPEN_FAIL = 494,					// Failed to open costume random synthesis window
	ERROR_ITEM_RANDOMMIX_CLOSE_FAIL = 495,					// Failed to close costume random synthesis window
	ERROR_ITEM_ALREADY_GUILDITEM = 496,						// You already own this item.
	ERROR_ITEM_RANDOMMIX_NORANKITEM_FAIL = 497,				// No item matching the costume random synthesis rank
	ERROR_ITEM_OVERFLOW_CHARMCOIN = 498,					// Please reduce the gold amount and try again.
	ERROR_ITEM_INSUFFICIENTY_PERIOD_RECOVERYCASH = 499,		// Cannot restore because the period has passed.

	//Trade & Mail & Market Message Starting at 500
	ERROR_SHOP_NOTFOUND = 500,						//The shop does not exist.
	ERROR_SHOP_SLOTITEM_NOTFOUND = 501,				//The shop item does not exist.
	ERROR_SHOP_DATANOTFOUND = 502,					//No shop data
	ERROR_MAIL_INSUFFICIENCY_MONEY = 503,			//Insufficient gold
	ERROR_MAIL_ATTACHITEMFAIL = 504,				//Failed to attach item
	ERROR_MAIL_CHARACTERNOTFOUND = 505,				//No recipient
	ERROR_MAIL_ATTACHEDTHING_EXIST = 506,			//Something is attached
	ERROR_MAIL_DOESNOTEXIST_ATTACHED_THING = 507,	//Nothing is attached.
	ERROR_MARKET_COUNTOVER = 508,					//Cannot register any more items on the market
	ERROR_MARKET_DOESNOTEXIST_REGISTITEM = 509,		//No registered market items.
	ERROR_MARKET_SOLDITEM = 510,					//Item already sold
	ERROR_MARKET_ISNOT_SOLDITEM = 511,				//This item has not been sold.
	ERROR_EXCHANGE_USERNOTFOUND = 512,				//No target to trade with
	ERROR_EXCHANGE_FAIL = 513,						//Trade failed
	ERROR_MAIL_DONTSENDYOURSELF = 514,				// Cannot send to yourself.
	ERROR_SHOP_NOTFORSALE = 515,					// This item is not for sale
	ERROR_MARKET_REGISTITEMFAIL = 516,				// Not an item to register
	ERROR_EXCHANGE_ITEMFAIL = 517,					// Not an item to trade
	ERROR_EXCHANGE_SENDERCANCEL = 518,				// When the trade requester cancels again
	ERROR_EXCHANGE_OVERFLOWMONEY = 519,				// Trade failed: exceeds maximum holdable gold
	ERROR_SHOP_OVERFLOWMONEY = 520,					// Sale failed: exceeds maximum holdable gold
	ERROR_MAIL_ATTACHITEM_OVERFLOW = 521,			// No empty slot in inventory.
	ERROR_MAIL_ATTACHMONEY_OVERFLOW = 522,			// Cannot withdraw exceeding the maximum amount of gold you can hold.
	ERROR_MAIL_CANT_RECVMAIL	= 523,				// Target cannot receive mail.
	ERROR_MAIL_CANT_RECVCASHPRESENT = 524,			// Target cannot receive gifts.
	ERROR_EXCHANGE_CANTEXCHANGE = 525,				// Target cannot receive trade requests.
	ERROR_ITEM_INVENTOEQUIP_FAIL = 526,				// Cannot equip items right now
	ERROR_ITEM_EQUIPTOINVEN_FAIL = 527,				// Cannot unequip items right now
	ERROR_CASHSHOP_RESERVE_NOTENOUGH = 528,			// Insufficient reward points.
	ERROR_CASHSHOP_RESERVE_DISABLE = 529,			// Item cannot be purchased with reward points.
	ERROR_CASHSHOP_OPEN = 530,						// Cannot exit while Cash Shop is open?
	ERROR_CASHSHOP_COUNTOVER = 531,					// Cannot buy anymore
	ERROR_CASHSHOP_CASH_NOTENOUGH = 532,			// Insufficient cash.
	ERROR_CASHSHOP_CALLGM = 533,					// Failed to receive the purchased item. Please contact customer support.
	ERROR_CASHSHOP_DONTSENDYOURSELF = 534,			// Cannot send a gift to yourself.
	ERROR_GIFT_COUNTOVER_INVEN = 535,				// (Inventory) Cannot receive any more
	ERROR_GIFT_COUNTOVER_WARE = 536,				// (Storage) Cannot receive any more
	ERROR_GIFT_COUNTOVER_REBIRTHCOIN = 537,			// (Resurrection scroll) Cannot receive any more
	ERROR_GIFT_NOTFORSALE = 538,					// Item cannot be gifted
	ERROR_GACHAPON_JP_MISMATCH_JOBCLASS = 539,		// Japan Gachapon. Result item does not match requested job. Likely packet hack.
	ERROR_GACHAPON_JP_NOT_ENOUGH_COIN = 540,		// Japan Gachapon. Failed due to insufficient coins.
	ERROR_GACHAPON_JP_INVALID_SHOP_ID = 541,		// Japan Gachapon. Invalid Gachapon index
	ERROR_CASHSHOP_PROHIBITSALE = 542,				// This item is no longer for sale
	ERROR_GACHAPON_JP_INVALID_RESULT_ITEM = 543,	// Japan Gachapon. Result item info not found on server.
	ERROR_VIP_FAIL = 544,							// Only VIP can buy?
	ERROR_MARKET_INVALIDPERIOD = 545,				// Not within an available sale period.
	ERROR_MARKET_CANNOTUSE_PREMIUM = 546,			// Premium service is unavailable.
	ERROR_CASHSHOP_DUPLICATE_GESTURE = 547,			// Cannot purchase a gesture you already own.
	ERROR_CASHSHOP_USE_AGREEMENT = 548,				// You must agree to the Cash Shop terms before using it
	ERROR_MARKET_REGISTPREMIUM = 549,				// Registration allowed only for market pass holders
	ERROR_MARKET_REGISTVIP = 550,					// VIP only can register
	// GuildWare
	ERROR_ITEM_NOT_MOVE_TO_GUILDWARE = 551,			// Cannot move item to guild storage.
	ERROR_ITEM_NOT_TAKE_FROM_GUILDWARE = 552,		// Cannot retrieve item from guild storage.
	ERROR_GUILDWARE_NOT_READY = 553,				// Guild storage is being prepared.
	ERROR_MAX_WITHDRAW_GUILDCOIN = 554,				// Exceeded maximum withdrawal amount per day.
	ERROR_MAX_TAKE_GUILDITEM = 555,					// Exceeded maximum number of items that can be withdrawn per day.
	ERROR_FULL_GUILDWARE = 556,						// Guild storage is full, cannot put item in
	ERROR_CASHSHOP_NXACREDITGIFT = 557,				// Cannot send gifts using NX Credit
	ERROR_CASHSHOP_NXACREDIT = 558,					// 
	ERROR_ALREADY_MAXGUILDWARE = 559,				// Cannot expand the guild storage any further.
	// Cash
	ERROR_CASHSHOP_GIFTCOUNTOVER = 560,				// "Cannot receive this gift because duplicate ownership of this item is not allowed."
	ERROR_ITEM_NOT_MOVE_TO_WARE = 561,				// This item cannot be placed in storage.
	
	
	//Quest Message Starting at 600
	ERROR_QUEST_CANCELFAIL = 600,					//Failed to delete quest
	ERROR_QUEST_SETACTIVEFAIL = 601,				//Failed to set quest
	ERROR_QUEST_NOTFOUND = 602,						//No quest
	ERROR_QUEST_REWARD_FAIL = 603,					//Quest reward request failed
	ERROR_QUEST_FCM = 604,							// Quest accept/complete/reward is restricted after fatigue time. Take a rest... -_;
	ERROR_QUEST_INVENTORY_NOTENOUGH = 605,			//Insufficient quest inventory space.
	ERROR_QUEST_INVENTORY_ALMOSTFULL = 606,			//Quest inventory space is almost full.
	ERROR_QUEST_FULLOFCOUNT_PLAYING = 607,			//Cannot accept more quests because quest limit is exceeded.
	ERROR_QUEST_ALREADY_PLAYING = 608,				//Quest already in progress
	ERROR_QUEST_NEST_HELPER = 609,					//Quest accept/complete/reward is restricted while acting as a Nest helper.
	ERROR_QUEST_CANT_ADD_QUEST_FROM_ITEM = 610,		//The quest obtainable from this item has already been completed.


	//Chat Message Starting at 700
	ERROR_CHAT = 700,
	ERROR_CHAT_USERNOTFOUND = 701,
	ERROR_CHAT_DONTSAYYOURSELF = 702,
	ERROR_CHAT_MOBILESENDED = 703,

	//ChatRoom Starting at 750
	ERROR_CHATROOM = 750,
	ERROR_CHATROOM_UNKNOWNUSER = 751,				// User is not in the room.
	ERROR_CHATROOM_NOMORESPACE = 752,				// Room is full
	ERROR_CHATROOM_ALREADYINROOM = 753,				// Already participating in a room.
	ERROR_CHATROOM_CANNOTENTER = 754,				// Cannot enter the room.
	ERROR_CHATROOM_WRONGPASSWORD = 755,				// Incorrect password.
	ERROR_CHATROOM_FAILEDMAKEROOM = 756,			// Failed to create room
	ERROR_CHATROOM_INVALIDCHATROOM = 757,			// Room error! - No such room exists.
	ERROR_CHATROOM_ALREADYLEADER = 758,				// Already the room master.
	ERROR_CHATROOM_ANOTHERROOM = 759,				// Already participating in another chat room.
	ERROR_CHATROOM_CANNOTLEAVELEADER = 760,			// Room master cannot leave while other players remain in the room.
	ERROR_CHATROOM_ONLYLEADER = 761,				// This function is only available to the room master.
	ERROR_CHATROOM_KICKEDFOREVER = 762,				// Cannot enter because permanently banned from the room.

	//Skill Message Starting at 800
	ERROR_SKILL_ACQUIREFAIL = 800,								//Failed to acquire skill
	ERROR_SKILL_DELETEFAIL = 801,								//Failed to delete skill
	ERROR_SKILL_ISNOT_YOURS = 802,								//You do not own this skill.
	ERROR_SKILL_LEVELUPFAIL_ISNOT_YOURS = 803,					//Failed to level up skill because you do not own it
	ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_SKILLPOINT = 804,		//Insufficient skill points
	ERROR_SKILL_LEVELUPFAIL_INSUFFICIENCY_CHARACTERLEVEL = 805,	//Character level too low
	ERROR_SKILL_LEVELUPFAIL_ALREADY_MAX = 806,					//Already at max level.
	ERROR_SKILL_LEVELUPFAIL_INVALID_SKILLPOINT = 807,			// Invalid skill points (owned points less than actually used skill points, or differs from table skill point requirement for skill level-up)
	ERROR_SKILL_LEVELUPFAIL_EQUIPITEMSKILL = 808,				// Equipment item skills cannot be leveled up.
	ERROR_SKILL_LEVELUPFAIL_TOGGLE_AURA_ON = 809,				// Skills that are toggled on or have an active aura cannot be leveled up.
	ERROR_SKILL_UNLOCK_ALREADY_OPEN = 810,						// Skill already unlocked.
	ERROR_SKILL_UNLOCK_FAIL = 811,		// Unlock failed... WIP. Will refine later.
	ERROR_SKILL_UNLOCK_MISMATCH_JOB = 812,						// Unlock failed because class does not match.
	ERROR_SKILL_ACQUIRE_FAIL_ISNOT_YOURS = 820,					// Attempting to acquire a skill not currently owned.
	ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_SKILLPOINT = 821,		// Insufficient points required to acquire the skill (required SP for skill level 1 defined in the table).
	ERROR_SKILL_ACQUIRE_FAIL_DONT_HAVE_PARENT_SKILL = 822,		// Failed to acquire skill because prerequisite skill is not owned.
	ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_PARENT_SKILL_LEVEL = 823, // Prerequisite skill level not met.
	ERROR_SKILL_ACQUIRE_FAIL_NOT_ENOUGH_CHAR_LEVEL = 824,		// Failed to acquire skill because required character level is not met.
	ERROR_SKILL_ACQUIRE_FAIL_MISMATCH_JOB = 825,
	ERROR_SKILL_UNLOCK_NOT_ENOUGH_MONEY = 826,					// Tried to unlock skill with money, but failed due to insufficient money.
	ERROR_SKILL_ACQUIRE_FAIL_EXCLUSIVE = 827,					// When a request arrives to acquire an ultimate skill that cannot be learned simultaneously.

	//Friend Message Starting at 900
	ERROR_FRIEND_GETLISTFAIL = 900,						//Failed to get friend list
	ERROR_FRIEND_GROUP_ALREADYMAX = 901,				//Cannot add more groups
	ERROR_FRIEND_ALREADYMAX = 902,						//Cannot add more friends
	ERROR_FRIEND_DESTUSER_ALREADYFRIEND = 903,			//Already friends with target
	ERROR_FRIEND_USERNOTFOUND = 904,					//No target
	ERROR_FRIEND_FRIEND_NORFOUND = 905,					//Target is not a friend
	ERROR_FRIEND_GROUP_NOTFOUND = 906,					//No group.
	ERROR_FRIEND_ADDFRIENDFAIL = 907,					//Failed to add friend
	ERROR_FRIEND_ADDGROUPFAIL = 908,					//Failed to add friend group
	ERROR_FRIEND_DELETEFRIENDFAIL = 909,				//Failed to delete friend
	ERROR_FRIEND_DELETEGROUPFAIL = 910,					//Failed to delete friend group
	ERROR_FRIEND_CANT_ADDFRIEND = 911,					//Cannot add target as a friend.

	//Isolate Message Starting at 950
	ERROR_ISOLATE_REQUESTFAIL = 950,					//Sent request to blocked user
	ERROR_ISOLATE_ADDFAIL = 951,						//Block failed
	ERROR_ISOLATE_ALREADYADDED = 952,					//Already blocked
	ERROR_ISOLATE_DELFAIL = 953,						//Failed to delete block	

	//Guild Message Starting at 1000
	ERROR_GUILD_YOUR_NOT_BELONGANYGUILD = 1000,			// Not in any guild
	ERROR_GUILD_YOUR_ALREADY_BELONGTOGUILD = 1001,		// Belongs to a guild
	ERROR_GUILD_NAME_ALREADYEXIST = 1002,				// Guild name already exists
	ERROR_GUILD_CREATEFAIL = 1003,						// Failed to create guild
	ERROR_GUILD_YOURNOT_INVITED = 1004,					// Not invited // Unused ?
	ERROR_GUILD_LEAVEFAIL = 1005,						// Failed to leave guild // Unused ?
	ERROR_GUILD_HAS_NO_AUTHORITY = 1006,				// No permission
	ERROR_GUILD_INFOUPDATEFAIL = 1007,					// Failed to change guild info
	ERROR_GUILD_GRADEINFO_UPDATEFAIL = 1008,			// Failed to change guild rank info // Unused ?
	ERROR_GUILD_USERINFO_UPDATEFAIL = 1009,				// Failed to change guild member info
	ERROR_GUILD_NOTEXIST_GUILD = 1010,					// Non-existent guild
	ERROR_GUILD_NOTEXIST_GUILDMEMBER = 1011,			// Non-existent guild member
	ERROR_GUILD_NOTEXIST_GUILDMASTER = 1012,			// Non-existent guild master
	ERROR_GUILD_ONLYAVAILABLE_GUILDMASTER = 1013,		// Guild master only
	ERROR_GUILD_NOTAVAILABLE_GUILDMASTER = 1014,		// Not allowed for guild master
	ERROR_GUILD_NOTEXIST_OPPOSITEUSER = 1015,			// Target does not exist	
	ERROR_GUILD_ALREADYINGUILD_OPPOSITEUSER = 1016,		// Target is already in a guild
	ERROR_GUILD_REFUESED_GUILDINVITATION = 1017,		// Target declined the invitation
	ERROR_GUILD_NOTOURGUILD_OPPOSITEUSER = 1018,		// Target is not a guild member
	ERROR_GUILD_CANTINVITE_YOURSELF = 1019,				// Cannot invite yourself
	ERROR_GUILD_CANTEXILE_YOURSELF = 1020,				// Cannot kick yourself
	ERROR_GUILD_ALREADYINGUILD_ACCOUNT = 1021,			// Account already joined a guild
	ERROR_GUILD_LACKOFCOUNT_CMMPOINT = 1022,			// Insufficient guild normal points
	ERROR_GUILD_FULLOFCOUNT_CMMPOINT = 1023,			// Guild normal points exceeded
	ERROR_GUILD_LACKOFCOUNT_LDDPOINT = 1024,			// Insufficient guild ladder points
	ERROR_GUILD_FULLOFCOUNT_LDDPOINT = 1025,			// Guild ladder points exceeded
	ERROR_GUILD_LACKOFCOUNT_MONEY = 1026,				// Insufficient guild funds
	ERROR_GUILD_FULLOFCOUNT_MONEY = 1027,				// Guild funds exceeded
	ERROR_GUILD_STILLEXIST_MONEY = 1028,				// Guild resources still exist
	ERROR_GUILD_LACKOFCOUNT_LEVEL = 1029,				// Reached min guild level
	ERROR_GUILD_FULLOFCOUNT_LEVEL = 1030,				// Reached max guild level
	ERROR_GUILD_LACKOFCOUNT_ADDSIZE = 1031,				// Reached min extra guild capacity
	ERROR_GUILD_FULLOFCOUNT_ADDSIZE = 1032,				// Reached max extra guild capacity
	ERROR_GUILD_CANTADD_GUILDMEMBER = 1033,				// Exceeded total guild capacity (insufficient capacity)
	ERROR_GUILD_CANTDISMISS_MONEY = 1034,				// Cannot disband guild (funds exist)
	ERROR_GUILD_CANTDISMISS_ITEM = 1035,				// Cannot disband guild (items exist)
	ERROR_GUILD_CANTDISMISS_GUILDMEMBER = 1036,			// Cannot disband guild (members exist)
	ERROR_GUILD_FAILADD_GUILDMEMBER = 1037,				// Failed to add guild member
	ERROR_GUILD_ALREADYDISMISS_GUILD = 1038,			// Guild already disbanded
	ERROR_GUILD_INVALIDCHANGE_GUILDINFO = 1039,			// Invalid guild info change request
	ERROR_GUILD_LACKOFCOND_GUILDINFO = 1040,			// Insufficient conditions to change guild info
	ERROR_GUILD_INVALIDCHANGE_GUILDMEMBINFO = 1041,		// Invalid guild member info change request
	ERROR_GUILD_LACKOFCOND_GUILDMEMBINFO = 1042,		// Insufficient conditions to change guild member info
	ERROR_GUILD_NOTEXIST_GUILDINFO = 1043,				// Non-existent guild info (whether it exists on server)
	ERROR_GUILD_NOTEXIST_GUILDMEMBERINFO = 1044,		// Non-existent guild member info (whether it exists on server)
	ERROR_GUILD_ALREADYINGUILD_CHARACTER = 1045,		// Character already joined a specific guild
	ERROR_GUILD_LACKOFCOUNT_GUILDMEMBER = 1046,			// Insufficient guild members
	ERROR_GUILD_LACKOFCOUNT_SELFMONEY = 1047,			// Insufficient funds for guild activity (personal funds)
	ERROR_GUILD_LACKOFCOUNT_GUILDMONEY = 1048,			// Insufficient funds for guild activity (guild funds)
	ERROR_GUILD_LACKOFCOUNT_CREATEMONEY = 1049,			// Insufficient funds to create guild
	ERROR_GUILD_FAILADD_LEVEL = 1050,					// Failed to increase guild level
	ERROR_GUILD_LACKOFCOUNT_CREATELEVEL = 1051,			// Insufficient level to create guild
	ERROR_GUILD_FAILADD_SIZE = 1052,					// Failed to increase extra guild member capacity
	ERROR_GUILD_STILLEXIST_MEMBER = 1053,				// Guild members still exist
	ERROR_GUILD_CANTRENAME_SAME = 1054,					// Cannot use the name of your current guild.
	ERROR_GUILDWAR_NOT_REWARD_STEP = 1055,				// Not guild war reward period.
	ERROR_GUILD_DAILYLIMIT = 1056,						// Reached daily limit; cannot earn more guild points
	ERROR_GUILD_INVALIDWARE = 1057,					// Guild storage is unavailable.
	ERROR_GUILD_LACKOFCOUNT_GUILDLEVEL = 1058,			// Failed to purchase guild reward item due to insufficient guild level
	ERROR_GUILD_CANTACCEPT_GUILDINVITATION = 1059,		// Target cannot currently receive guild invitations

	// PvP Message Starting at 1100
	ERROR_PVP_CREATEROOM_FAILED			= 1100,			// Failed to create PvP room
	ERROR_PVP_MODIFYROOM_FAILED = 1101,					// Failed to change PvP room
	ERROR_PVP_ALREADY_PVPROOM = 1102,					// Already in PvP
	ERROR_PVP_LEAVE_FAILED = 1103,						// Failed to leave PvP room
	ERROR_PVP_JOINROOM_NOTFOUNDROOM = 1104,				// Cannot find PvP room to join
	ERROR_PVP_JOINROOM_INVALIDPW = 1105,				// Wrong password for PvP room to join
	ERROR_PVP_JOINROOM_MAXPLAYER = 1106,				// Max players reached for PvP room to join
	ERROR_PVP_JOINROOM_BANUSER = 1107,					// Kicked user in PvP room to join
	ERROR_PVP_JOINROOM_FAILED = 1108,					// Failed to join PvP room
	ERROR_PVP_CANTMOVECHANNEL_PARTYSTATE = 1109,		// Cannot change channels while in a party
	ERROR_PVP_FORCELEAVEROOM_MASTERSERVER_DOWN = 1110,	// Forced out of the room due to master server shutdown
	ERROR_PVP_CANTSTART_LESS_MINTEAMPLAYER = 1111,		// Fewer than minimum starting team members
	ERROR_PVP_CANTSTART_MORE_MAXDIFFTEAMPLAYER = 1112,	// Team member count difference exceeds limit
	ERROR_PVP_STARTPVP_FAILED = 1113,					// Failed to start PvP
	ERROR_PVP_CANTCHANGETEAM_NOSLOT = 1114,					// Cannot change team because there is no slot
	ERROR_PVP_JOINROOM_LEVELLIMIT = 1115,					// Cannot enter due to PvP level restriction
	ERROR_PVP_JOINROOM_MAXOBSERVER = 1116,					// Max observers reached for PvP room to join
	ERROR_PVP_CANTCHANGETEAM_CHANGECAPTAIN_FAILED = 1117,	// Cannot transfer: no one available to be room master
	ERROR_PVP_CANTCHANGETEAM_LEVELLIMIT = 1118,				// Cannot switch teams due to game entry level restriction
	ERROR_PVP_JOINROOM_DONTALLOW_EVENTROOM_OBSERVER = 1119,	// Spectating is not allowed in event rooms.
	ERROR_PVP_SWAPTEAM_MEMBERINDEX_FAIL = 1120,			// Failed to change PvP team member index
	ERROR_PVP_CHANGEMEMBERGRADE_FAIL = 1121,			// PvP GuildWar leader/sub-leader appointment failed
	ERROR_PVP_TRYACQUIREPOINT_FAIL = 1122,				// Acquisition request failed
	ERROR_PVP_TRYACQUIRESKILL_FAIL = 1123,				// Failed to acquire skill
	ERROR_PVP_TRYACQUIRESKILL_FAIL_AUTH = 1124,			// No permission to request skill acquisition.	
	ERROR_PVP_INITSKILL_FAIL = 1125,					// Failed to reset occupation war skills
	ERROR_PVP_USESKILL_FAIL = 1126,						// Failed to use occupation war skill
	ERROR_PVP_SWAPSKILLINDEX_FAIL = 1127,				// Failed to move occupation war quickslot index	
	ERROR_PVP_STARTPVP_FAIL_READYMASTER = 1128,			// In occupation war, masters of all teams must be ready
	ERROR_PVP_GUILDWAR_SECONDARYMASTER_ASIGNFAIL = 1129,// Appointment count limit exceeded
	ERROR_PVP_QUICK_USERNOTFOUND = 1131,				// No target available for 1:1 duel request
	ERROR_PVP_QUICK_FAIL = 1132,						// 1:1 duel request failed
	ERROR_PVP_QUICK_CANTINVITE = 1133,					// Target cannot receive duel requests.
	ERROR_PVP_QUICK_DENY = 1134,						// The opponent declined the duel request.
	ERROR_PVP_QUICK_MY_LEVEL = 1135,					// Your level is too low to duel.
	ERROR_PVP_QUICK_OTHER_LEVEL = 1136,					// Cannot duel because the opponent's level is too low.
	ERROR_PVP_LIST_LEVEL_LIMIT = 1137,					// Level too low to enter PvP..
	ERROR_PVP_CHANGELEVEL_FAIL = 1138,					// Channel change failed!
	ERROR_PVP_JOINFAIL_MISMATCHCHANNEL = 1139,			// Failed to enter room: channel mismatch.
	ERROR_PVP_SWAP_TOURNAMENT_INDEX_FAIL = 1140,		// Failed to swap tournament slot	
	ERROR_PVP_GAMBLEROOM_NOTCOIN = 1141,				// Cannot enter due to insufficient gold
	ERROR_PVP_GAMBLEROOM_NOTPETAL = 1142,				// Cannot enter due to insufficient petal

	// HAN auth
	ERROR_HAN_AUTHSVR_0					= 1200,			//	OK
	ERROR_HAN_AUTHSVR_OK = 1201,						//	Success
	ERROR_HAN_AUTHSVR_ARGUMENT_INVALID = 1202,			//	Invalid argument passed.
	ERROR_HAN_AUTHSVR_INITED_ALREADY = 1203,			//	Module is already initialized.
	ERROR_HAN_AUTHSVR_INITED_NOT = 1204,				//	Module is not initialized.
	ERROR_HAN_AUTHSVR_INITED_FAIL = 1205,				//	Failed to initialize module.
	ERROR_HAN_AUTHSVR_CONNPOOL_CREATEFAIL = 1206,		//	Failed to create connection pool.
	ERROR_HAN_AUTHSVR_GETCONNECTION_FAIL = 1207,		//	Failed to get connection
	ERROR_HAN_AUTHSVR_OPENCONNECTION_FAIL = 1208,		//	Failed to open connection.
	ERROR_HAN_AUTHSVR_SENDCONNECTION_FAIL = 1209,		//	Failed to send.
	ERROR_HAN_AUTHSVR_RECVCONNECTION_FAIL = 1210,		//	receive failed.
	ERROR_HAN_AUTHSVR_RETURNVALUE_INVALID = 1211,		//	Invalid return value from auth server.
	ERROR_HAN_AUTHSVR_AUTHSTRING_INVALID = 1212,		//	AuthString is invalid.
	ERROR_HAN_AUTHSVR_DLL_UNEXPECTED = 1213,			//	An unexpected error occurred.
	ERROR_HAN_AUTHSVR_PARAM_INVALID = 1214,				//	Invalid argument passed to auth server.
	ERROR_HAN_AUTHSVR_IP_INVALID = 1215,				//	Invalid IP address.
	ERROR_HAN_AUTHSVR_MEMBERID_INVALID = 1216,			//	Invalid user ID.
	ERROR_HAN_AUTHSVR_PASSWORD_INCORRECT = 1217,		//	Invalid user password.
	ERROR_HAN_AUTHSVR_PASSWORD_MISMATCHOVER = 1218,		//	Exceeded max password entry attempts
	ERROR_HAN_AUTHSVR_MEMBERID_NOTEXIST = 1219,			//	User does not exist
	ERROR_HAN_AUTHSVR_SYSTEM_ERROR = 1220,				//	System error occurred on auth server.
	ERROR_HAN_AUTHSVR_COOKIE_SETTINGERR = 1221,			//	Invalid cookie.
	ERROR_HAN_AUTHSVR_COOKIE_NOTEXIST = 1222,			//	Cookie does not exist. 
	ERROR_HAN_AUTHSVR_COOKIE_TIMEOUT = 1223,			//	Cookie has expired.

	// Han Billing
	ERROR_HAN_BILLING_OK				= 1230,			// Normal
	// +) fault from Billing Server
	ERROR_HAN_BILLING_GAMECODE_NOTEXIST = 1231,				// GAMEID is missing.
	ERROR_HAN_BILLING_USERNO_NOTEXIST = 1232,					// User Number is missing.
	ERROR_HAN_BILLING_CMD_NOTEXIST = 1233,						// No billing command.
	ERROR_HAN_BILLING_ORDNO_NOTEXIST = 1234,					// Order number is missing.
	ERROR_HAN_BILLING_SCD_NOTEXIST = 1235,						// No product code.
	ERROR_HAN_BILLING_SNAME_NOTEXIST = 1236,					// No product name.
	ERROR_HAN_BILLING_QTY_NOTEXIST = 1237,						// No product quantity.
	ERROR_HAN_BILLING_AMT_NOTEXIST = 1238,						// No purchase price.
	ERROR_HAN_BILLING_IP_NOTEXIST = 1239,						// No client IP address.
	ERROR_HAN_BILLING_USERNO_INVALID = 1240,					// User Number is invalid.
	ERROR_HAN_BILLING_BUY_FAIL = 1241,							// Payment was not made with this order number.
	ERROR_HAN_BILLING_GAMECODE_INVALID = 1242,					// Game ID is invalid.
	ERROR_HAN_BILLING_BALANCE_INSUFFICIENT = 1243,				// Insufficient balance.
	ERROR_HAN_BILLING_CMD_UNEXPECTED = 1244, 					// Received an unexpected command.
	ERROR_HAN_BILLING_DB_CONNECTFAIL = 1245,					// Failed to connect to billing DB.
	ERROR_HAN_BILLING_SVR_UNEXPECTED = 1246,					// Unexpected error occurred in billing system.
	// -) fault from function / this system
	ERROR_HAN_BILLING_ARGUMENT_INVALID = 1247,					// Invalid argument passed.
	ERROR_HAN_BILLING_INITED_ALREADY = 1248,					// Module is already initialized.
	ERROR_HAN_BILLING_INITED_NOT = 1249,						// Module is not initialized.
	ERROR_HAN_BILLING_INITED_FAIL = 1250,						// Failed to initialize module.
	ERROR_HAN_BILLING_INITED_FAILBALANCE = 1256,				// Failed to initialize balance inquiry connection.
	ERROR_HAN_BILLING_INITED_FAILBUY = 1257,					// Failed to initialize purchase connection.
	ERROR_HAN_BILLING_INITED_FAILCONFIRM = 1258,				// Failed to initialize payment verification connection.
	ERROR_HAN_BILLING_GETCONN_FAILBALANCE = 1259,				// Failed to get balance inquiry connection.
	ERROR_HAN_BILLING_GETCONN_FAILBUY = 1260,					// Failed to obtain purchase connection.
	ERROR_HAN_BILLING_GETCONN_FAILCONFIRM = 1261,				// Failed to obtain payment verification connection.
	ERROR_HAN_BILLING_OPENREQ_FAIL = 1262,						// Failed to open Request.
	ERROR_HAN_BILLING_SENDREQ_FAIL = 1263,						// Failed to Send billing info.
	ERROR_HAN_BILLING_RECVRESULT_FAIL = 1264,					// Failed to Recv billing info.
	ERROR_HAN_BILLING_RECVRESULT_INVALID = 1265,				// Billing info received from server is invalid.
	ERROR_HAN_BILLING_INITED_FAILBILLNO = 1266,
	ERROR_HAN_BILLING_GETCONN_FAILBILLNO = 1267,
	ERROR_HAN_BILLING_INITED_FAILBILLNOACC = 1268,
	ERROR_HAN_BILLING_GETCONN_FAILBILLNOACC = 1269,
	ERROR_HAN_BILLING_DLL_UNEXPECTED = 1270,					// An unexpected error occurred within the module.
	// 4.2 HanBillingGetBillNoAcc() specific Return Value
	// +) fault from TGS
	ERROR_HAN_BILLING_GETBILLNO_NEED_PARAM = 1271,				// Required parameter is missing.
	ERROR_HAN_BILLING_GETBILLNO_IP_MISMATCH = 1272,			// Invalid IP.
	ERROR_HAN_BILLING_GETBILLNO_USERID_MISMATCH = 1273,		// Incorrect user ID.
	ERROR_HAN_BILLING_GETBILLNO_DB_CONN_ERROR = 1274,			// Failed to connect to DB.
	ERROR_HAN_BILLING_GETBILLNO_USER_NOTEXIST = 1275,			// Unregistered user.
	ERROR_HAN_BILLING_GETBILLNO_COOKIE_ERROR = 1276,			// cookie error
	ERROR_HAN_BILLING_GETBILLNO_COOKIE_IS_EMPTY = 1277,		// cookie is empty.
	ERROR_HAN_BILLING_GETBILLNO_COOKIE_TIMEOUT = 1278,			// Expired cookie.

	//Auth Message Starting at 1300
	ERROR_AUTH_CERTIFYINGSTEP1_CHECK = 1300,			// Error occurred while checking auth info.
	ERROR_AUTH_SERVERID_CHECK = 1301,							// Error occurred while checking auth info.
	ERROR_AUTH_CERTIFYINGKEY_CHECK = 1302,						// Error occurred while checking auth info.
	ERROR_AUTH_CERTIFYINGKEY_SET = 1303,						// Error occurred while setting auth info.
	ERROR_AUTH_AUTHLIMITSEC_EXCEED = 1304,						// Error occurred while checking auth info.
	ERROR_AUTH_ACCOUNTID_NOTEXIST = 1305,						// Error occurred while checking auth info.
	ERROR_AUTH_WORLDID_SET = 1306,								// Error occurred while setting auth info.
	ERROR_AUTH_SESSIONID_SET = 1307,							// Error occurred while setting auth info.
	ERROR_AUTH_CERTIFYINGSTEP2_CHECK = 1308,					// Error occurred while checking auth info.
	ERROR_AUTH_WORLDID_CHECK  = 1309,							// Error occurred while checking auth info.
	ERROR_AUTH_SESSIONID_CHECK = 1310,							// Error occurred while checking auth info.

	// NexonPCBang
	ERROR_NEXONAUTH_I0			= 1400,							// Not a premium-enabled PC cafe. Please contact the PC cafe manager.
	ERROR_NEXONAUTH_M17 = 1401,									// Premium benefits and event details (display as text if event image cannot be rendered)
	ERROR_NEXONAUTH_I3 = 1402,									// PC Bang flat-rate period has expired and the game will exit. Please contact the PC Bang staff.
	ERROR_NEXONAUTH_I19 = 1403,									// PC Bang metered time has expired and the game will exit. Please contact the PC Bang staff.
	ERROR_NEXONAUTH_I2 = 1404,									// Allowed concurrent connections per IP exceeded. Please contact your PC bang administrator.
	ERROR_NEXONAUTH_I27 = 1405,									// Not eligible for PC Bang premium. Please contact Nexon PC Bang Customer Support.
	ERROR_NEXONAUTH = 1406,										// General error values related to PC Bang
	ERROR_NEXONAUTH_I28 = 1407,									// Block machine ID of illegal user
	// ERROR_NEXONAUTH_I17, // Remaining PC bang flat-rate time is OO hours OO minutes
	ERROR_NEXONAUTH_SHUTDOWNED_3 = 1408,						// Under the Youth Protection Act, users under 16 cannot play the game from 00:00 to 06:00.

	// Nexon Billing 
	ERROR_NEXONBILLING_0		= 1410,					// Error
	ERROR_NEXONBILLING_99 = 1411,								// DB error
	ERROR_NEXONBILLING_255 = 1412,								// maintenance

	ERROR_NEXONBILLING_2 = 1413,								// Duplicate payment error
	ERROR_NEXONBILLING_3 = 1414,								// Exceeded total product purchase quantity
	ERROR_NEXONBILLING_4 = 1415,								// Exceeded product purchase quantity
	ERROR_NEXONBILLING_5 = 1416,								// Less than repurchase days
	ERROR_NEXONBILLING_10 = 1417,								// Invalid event for lottery/raffle item
	ERROR_NEXONBILLING_11 = 1418,								// Invalid winning algorithm for lottery/raffle item
	ERROR_NEXONBILLING_21 = 1419,								// No cart
	ERROR_NEXONBILLING_22 = 1420,								// Restricted game
	ERROR_NEXONBILLING_23 = 1421,								// Total sales quantity limited
	ERROR_NEXONBILLING_24 = 1422,								// Order quantity limited (buyer)
	ERROR_NEXONBILLING_25 = 1423,								// Not within reorder period (buyer)
	ERROR_NEXONBILLING_26 = 1424,								// Product cannot be gifted
	ERROR_NEXONBILLING_27 = 1425,								// Order quantity limited (recipient)
	ERROR_NEXONBILLING_28 = 1426,								// Not within reorder period (recipient)
	ERROR_NEXONBILLING_35 = 1427,								// Payment method error
	ERROR_NEXONBILLING_51 = 1428,								// Used coupon
	ERROR_NEXONBILLING_52 = 1429,								// Expired (arbitrary coupon expiration by admin)
	ERROR_NEXONBILLING_54 = 1430,								// Coupon canceled (expired from user side)
	ERROR_NEXONBILLING_61 = 1431,								// Coupon info error (on coupon service code mismatch, missing item info, invalid contract state, missing contract info, contract type mismatch, etc.)
	ERROR_NEXONBILLING_62 = 1432,								// Invalid coupon number
	ERROR_NEXONBILLING_63 = 1433,								// Game mismatch
	ERROR_NEXONBILLING_65 = 1434,								// Coupon expired.
	ERROR_NEXONBILLING_66 = 1435,								// Another user used a coupon intended for a specific user.
	ERROR_NEXONBILLING_81 = 1436,								// Coupon usage limit check error
	ERROR_NEXONBILLING_82 = 1437,								// Coupon usage limit input error
	ERROR_NEXONBILLING_100 = 1438,								// Entered a PC Bang coupon while not at a PC Bang, though it is a valid coupon.
	ERROR_NEXONBILLING_101 = 1439,								// Entered a PC Bang coupon while not at a PC Bang, and it has already been used.
	ERROR_NEXONBILLING_102 = 1440,								// Entered a PC Bang coupon while not at a PC Bang, and it is expired.
	ERROR_NEXONBILLING_103 = 1441,								// Entered a PC Bang coupon while not at a PC Bang, and its issuance was canceled.
	ERROR_NEXONBILLING_149 = 1442,								// Coupon processing error (coupon DB, etc.)

	ERROR_NEXONBILLING_1000 = 1443,							// Exceeded gift limit
	ERROR_NEXONBILLING_1001 = 1444,							// Gift age restriction
	ERROR_NEXONBILLING_1002 = 1445,							// User restricted from sending gifts
	ERROR_NEXONBILLING_1003 = 1446,							// User restricted from receiving gifts
	ERROR_NEXONBILLING_11001 = 1447,							// Purchase limit exceeded for this item
	ERROR_NEXONBILLING_11002 = 1448,							// Purchase unavailable due to user cash lock enabled for this game
	ERROR_NEXONBILLING_12001 = 1449,							// Blocked user
	ERROR_NEXONBILLING_12002 = 1450,							// Member information not found
	ERROR_NEXONBILLING_12040 = 1451,							// Insufficient balance
	ERROR_NEXONBILLING_12044 = 1452,							// When the valid period to use charged cash has expired
	ERROR_NEXONBILLING_120043 = 1453,							// Not a purchasable item.
	ERROR_NEXONBILLING_120062 = 1454,							// User is in a state unable to purchase.

	ERROR_NEXONBILLING_REFUND_2 = 1455,						// Requested quantity exceeds refundable quantity
	ERROR_NEXONBILLING_REFUND_3 = 1456,						// Item purchased more than 7 days ago
	ERROR_NEXONBILLING_REFUND_4 = 1457,						// Claimed part of the package
	ERROR_NEXONBILLING_REFUND_5 = 1458,						// Item priced at 0
	ERROR_NEXONBILLING_REFUND_10 = 1459,						// Duplicate RequestID
	ERROR_NEXONBILLING_REFUND_11 = 1460,						// Non-existent OrderNo
	ERROR_NEXONBILLING_REFUND_12 = 1461,						// Non-existent ProductNo	

	//Nexon GioSecure
	ERROR_NEXONAUTH_GIO_TERMINATE = 1470,				// Location-based game access block (in-game)
	ERROR_NEXONAUTH_GIO_BAN = 1471,						// Location-based game access block (at login)	

	ERROR_NEXONAUTH_SHUTDOWNED_1 = 1472,					// An error occurred while checking age info. Please log in again later.
	ERROR_NEXONAUTH_SHUTDOWNED_2 = 1473,					// Not a real-name verified member or not a Nexon.com member.
	// ERROR_NEXONAUTH_SHUTDOWNED_3 = 1408, already defined above
	ERROR_NEXONAUTH_SHUTDOWNED_4 = 1474,					// Game access is blocked by the selective shutdown policy.
	ERROR_NEXONAUTH_SHUTDOWNED_99 = 1475,					// Game access is blocked.

	//ServiceManager
	ERROR_SM_UNKNOWN_OPERATING_TYPE = 1500,				// Unknown message.
	ERROR_SM_TERMINATE_BUSY = 1501,								// Killing the service.
	ERROR_SM_NETLAUNCHER_INSUFFISIENCY = 1502,					// Not all required launchers are ready.
	ERROR_SM_STARTEACH_FAILED = 1503,								// Failed to individually start the service.
	ERROR_SM_SELECTED_NETLAUNCHER_NOTFOUND = 1504,				// No launcher exists to individually start the service launcher.
	ERROR_SM_NETLAUNCHER_NOTFOUND = 1505,						// No running NetLauncher.
	ERROR_SM_TERMINATE_SERVICE_NOTFOUND = 1506,				// No running service to stop.
	ERROR_SM_MAKEDUMP_FAIL =  1507,
	ERROR_SM_CHANNELCONTROL_FAIL = 1508,
	ERROR_SM_GAMECONTROL_FAIL = 1509,
	ERROR_SM_UNMATCHED_REQ = 1510,								// Request with mismatched container version.
	ERROR_SM_LOGINFAIL = 1511,
	ERROR_SM_LOGINFAIL_VERSIONUNMATCHED = 1512,				// Version mismatch.
	ERROR_SM_YOU_ARE_NOT_ALLOWED_LEVEL = 1513,					// Unauthorized request for this command.
	ERROR_SM_LOGINFAIL_LEVEL_DUPLICATED = 1514,				// Duplicate login is not allowed for this ID.

	// MasterSystem
	ERROR_MASTERSYSTEM_MASTERAPPLICATION_PENALTY = 1600,		// Cannot join due to master participation penalty period
	ERROR_MASTERSYSTEM_MASTERUSER_OFFLINE = 1601,			
	ERROR_MASTERSYSTEM_LEAVE_DESTUSER_OFFLINE = 1602,					// Target user is offline when breaking master-apprentice relationship
	ERROR_MASTERSYSTEM_LEAVE_DESTUSER_CANTSTATE = 1603,				// Target user is in a state where the master-apprentice relationship cannot be broken
	ERROR_MASTERSYSTEM_LEAVE_FAILED = 1604,							// Failed to break master-apprentice relationship (common error)
	ERROR_MASTERSYSTEM_RECALL_SAMECHANNEL = 1605,
	ERROR_MASTERSYSTEM_RECALL_CANTSTATUS = 1606,						// State where summoning is not possible
	ERROR_MASTERSYSTEM_RECALL_FAILED = 1607,
	ERROR_MASTERSYSTEM_RECALL_REJECT = 1608,							// Summon rejected
	ERROR_MASTERSYSTEM_JOIN_DENY = 1609,								// Reject master-apprentice
	ERROR_MASTERSYSTEM_CANT_JOINSTATE = 1610,							// State where master-apprentice relationship cannot be formed

	//ERROR_VEHICLE_DONOTHAVE_VEHICLE = 1700, // You do not own a vehicle.
	//ERROR_VEHICLE_DONOTRIDE_VEHICLE = 1701, // You are not riding a vehicle.

	// Gamania - AUTH, BILLING
	ERROR_GAMANIA_INVALID_ID_OR_PASSWORD = 1800,		// Invalid ID or password
	ERROR_GAMANIA_NO_SUCH_ACCOUNT = 1801,						// Account does not exist.
	ERROR_GAMANIA_NO_SUCH_SERVICE = 1802,						// Non-existent service
	ERROR_GAMANIA_NO_ENOUGH_POINT = 1803,						// Insufficient points.
	ERROR_GAMANIA_WRONG_EVENTOTP = 1804,						// EVENT OTP is incorrect.
	ERROR_GAMANIA_WRONG_OTP = 1805,							// OTP is incorrect.
	ERROR_GAMANIA_ACCOUNT_LOCK = 1806,							// Account is locked.
	ERROR_GAMANIA_SERVICE_NOT_AVAILABLE = 1807,				// Game service is not active.
	ERROR_GAMANIA_TIME_OUT = 1808,								// Timeout
	ERROR_GAMANIA_CONTRACT_BLOCK = 1809,						// Contract expired
	ERROR_GAMANIA_ALREADY_LOGIN = 1810,						// Already logged in.
	ERROR_GAMANIA_ALREADY_WAITING = 1811,						// Authentication request already in progress
	//Coupon-related
	ERROR_GAMANIA_COUPON_UNAVAILABE = 1812,					// This coupon has been suspended.
	ERROR_GAMANIA_COUPON_EXPIRED = 1813,						// Coupon expired
	ERROR_GAMANIA_COUPON_WRONG = 1814,							// Coupon input error
	ERROR_GAMANIA_COUPON_WRONG_WORLD = 1815,					// This coupon cannot be used on this server.

	ERROR_GAMANIA_COUPON_AVAILABE = 1816,						// Coupon has not been used.
	ERROR_GAMANIA_COUPON_USED = 1817,							// Coupon has already been used.
	ERROR_GAMANIA_COUPON_EXCHANGING = 1818,					// Exchanging coupon.
	ERROR_GAMANIA_COUPON_USED_DATE = 1819,						// Already exchanged during the specified period. See the homepage for details
	ERROR_GAMANIA_COUPON_NOT_START_USE = 1820,					// Coupon exchange has not started yet. Check the website for details.
	ERROR_GAMANIA_COUPON_REINPUT = 1821,						// Item grant error. Please re-enter the coupon number.
	ERROR_GAMANIA_COUPON_ACCOUNT = 1822,						// This coupon cannot be used on the account that already used it.
	ERROR_GAMANIA_COUPON_WORD = 1823,							// Coupon usage limit exceeded
	ERROR_GAMANIA_COUPON_COUNT = 1824,							// Coupon usage limit exceeded
	ERROR_GAMANIA_SPECIAL_CHAR = 1825,							// Special characters cannot be used.

	// Secondary skill
	ERROR_SECONDARYSKILL_ADDRECIPE_FAILED	= 1900,			// Failed to add secondary skill recipe
	ERROR_SECONDARYSKILL_DELETE_FAILED = 1901,						// Failed to remove secondary skill
	ERROR_SECONDARYSKILL_DELETERECIPE_FAILED = 1902,				// Failed to remove secondary skill recipe
	ERROR_SECONDARYSKILL_EXTRACTRECIPE_FAILED = 1903,				// Failed to extract secondary skill recipe
	ERROR_SECONDARYSKILL_MANUFACTURE_FAILED = 1904,				// Failed to craft secondary skill (general error)
	ERROR_SECONDARYSKILL_MANUFACTURE_PROBABILITY_FAILED = 1905,	// Failed to craft secondary skill due to probability
	ERROR_SECONDARYSKILL_MANUFACTURE_DONTALLOWMAP = 1906,			// Map does not allow crafting secondary skills
	ERROR_SECONDARYSKILL_ADDRECIPE_MAX = 1907,						// Max recipes
	ERROR_SECONDARYSKILL_ADDRECIPE_DUPLICATE = 1908,				// Duplicate recipe
	ERROR_SECONDARYSKILL_DELETE_FAILED_DONTALLOWMAP = 1909,		// Map does not allow deleting secondary skills.

	// Farm
	ERROR_FARM_CANT_START_PLANTSTATE = 2000,				// State where planting is not possible
	ERROR_FARM_CANT_START_PLANTSTATE_MAXFIELDCOUNT = 2001,			// State where planting is not possible (max limit)
	ERROR_FARM_CANT_CANCEL_PLANTSTATE = 2002,						// State where planting cannot be canceled
	ERROR_FARM_CANT_COMPLETE_PLANTSTATE = 2003,					// State where planting cannot be completed
	ERROR_FARM_CANT_COMPLETE_PLANTSTATE_MAXFIELDCOUNT = 2004,		// State where planting cannot be completed (max limit)
	ERROR_FARM_CANT_START_HARVESTSTATE = 2005,						// State where the plant cannot be harvested
	ERROR_FARM_CANT_CANCEL_HARVESTSTATE = 2006,					// State where harvesting the plant cannot be canceled
	ERROR_FARM_CANT_COMPLETE_HARVESTSTATE = 2007,					// State where harvesting the plant cannot be completed
	ERROR_FARM_CANT_START_ADDWATERSTATE = 2008,					// State where the plant cannot be watered
	ERROR_FARM_CANT_CANCEL_ADDWATERSTATE = 2009,					// State where watering the plant cannot be canceled
	ERROR_FARM_CANT_COMPLETE_ADDWATERSTATE = 2010,					// State where watering the plant cannot be completed
	ERROR_FARM_CANT_PLANT_OVERLAPCOUNT = 2011,						// Exceeded the max number of a specific item that can be planted at once
	ERROR_FARM_INVALID_VIP = 2012,									// Not VIP
	ERROR_FARM_CANT_START_PLANTSTATE_MAXPRIVATEFIELDCOUNT = 2013,	// State where planting cannot be completed (personal area max limit)
	ERROR_FARM_CANT_START_PLANTSTATE_DOWNSCALEFARM = 2014,			// Farm currently closing
	ERROR_FARM_INVALUD_CHAMPIONGUILD = 2015,						// Not the winning guild
	ERROR_FARM_CANT_START_PLANTSTATE_GUILDCHAMPION_TIMELIMIT = 2016,// Winning guild time limit

	//Fishing
	ERROR_FISHING_FAIL = 2050,								//General error for fishing state
	ERROR_FISHING_INSUFFICIENCY_FISHINGSKILL_LEVEL = 2051,  //Insufficient fishing skill level
	ERROR_FISHING_INSUFFICIENCY_FISHINGSKILL_GRADE = 2052,  //Insufficient fishing skill grade
	ERROR_FISHING_INSUFFICIENCY_FISHING_ROD = 2053,			//No fishing rod
	ERROR_FISHING_INSUFFICIENCY_FISHING_BAIT = 2054,		//No fishing bait
	ERROR_FISHING_INSUFFICIENCY_INVENTORY = 2055,			//No inventory space to hold the result
	ERROR_FISHING_CASTBAIT_FAIL = 2056,						//Basic error for starting fishing
	ERROR_FISHING_STOPFISHING_FAIL = 2057,					//Failed to end fishing
	ERROR_FISHING_FAIL_ISNOT_FISHINGAREA = 2058,			//Not a fishing area

	// Shanda Cash
	// Author
	ERROR_SHANDAAUTHOR_100201 = 2100,						// -100201 = PT account error (ptId format error)
	ERROR_SHANDAAUTHOR_100202 = 2101,								// -100202 Game ID error (appId error)
	ERROR_SHANDAAUTHOR_100203 = 2102,								// -100203 Partition ID error (areaId error)
	ERROR_SHANDAAUTHOR_100206 = 2103,								// -100206 Account does not exist (Account not exist)
	ERROR_SHANDAAUTHOR_100207 = 2104,								// -100207 Partition ID does not exist (areaId not exist)
	ERROR_SHANDAAUTHOR_100208 = 2105,								// -100208 Game ID does not exist (appId not exist)
	ERROR_SHANDAAUTHOR_100210 = 2106,								// -100210 Other record not found error (other error)
	ERROR_SHANDAAUTHOR_100213 = 2107,								// -100213 sdId too long or contains alphabetic characters (sdId format error)
	ERROR_SHANDAAUTHOR_100299 = 2108,								// -100299 DB error
	ERROR_SHANDAAUTHOR_120101 = 2109,								// -120101 Numeric account too long or contains alphabetic characters (sndaID format error)
	ERROR_SHANDAAUTHOR_120102 = 2110,								// -120102 PT account too long or all numeric (ptId format error)
	ERROR_SHANDAAUTHOR_120103 = 2111,								// -120103 accountid mapping error for user account (accountID error)
	ERROR_SHANDAAUTHOR_120104 = 2112,								// -120104 Balance error (balance error)
	ERROR_SHANDAAUTHOR_120105 = 2113,								// -120105 Billing type error (paytypeId error)
	ERROR_SHANDAAUTHOR_120108 = 2114,								// -120108 No matching data in LBS local DB (no data in LBS DB)
	ERROR_SHANDAAUTHOR_120109 = 2115,								// -120109 Other error (other errror)
	ERROR_SHANDAAUTHOR_120111 = 2116,								// -120111 Game ID error (appId error)
	ERROR_SHANDAAUTHOR_120112 = 2117,								// -120112 Network status error (Net status error)
	ERROR_SHANDAAUTHOR_120113 = 2118,								// -120113 Message error (message content error) (data body error)
	ERROR_SHANDAAUTHOR_129999 = 2119,								// -129999 DB error
	// Account
	ERROR_SHANDAACCOUNT_102601 = 2120,								// -102601 Billing message status error (Status error)
	ERROR_SHANDAACCOUNT_102602 = 2121,								// -102602 Billing type error (Paytype error)
	ERROR_SHANDAACCOUNT_102603 = 2122,								// -102603 Billing amount less than 0 (Amount error)
	ERROR_SHANDAACCOUNT_102613 = 2123,								// -102613 Packet log time is 5 days old (Calltime error)
	ERROR_SHANDAACCOUNT_102604 = 2124,								// -102604 PT account does not exist (Account not exist)
	ERROR_SHANDAACCOUNT_102605 = 2125,								// -102605 Game partition account does not exist (balance not exist)
	ERROR_SHANDAACCOUNT_102609 = 2126,								// -102609 sdId too long or contains alphabetic characters (sdId format error)
	ERROR_SHANDAACCOUNT_102610 = 2127,								// -102610 Consumption type error (appType error)
	ERROR_SHANDAACCOUNT_102611 = 2128,								// -102611 Coupling type error, must be 1 or 2 (coupletype error)
	ERROR_SHANDAACCOUNT_102612 = 2129,								// -102612 begintime must be less than or equal to endtime (begintime must samll to endtime)
	ERROR_SHANDAACCOUNT_102615 = 2130,								// -102615	amount = 0	Amount is 0
	ERROR_SHANDAACCOUNT_102617 = 2131,								// -102617 Other error (other error)
	ERROR_SHANDAACCOUNT_102699 = 2132,								// -102699 DB error
	ERROR_SHANDAACCOUNT_120201 = 2133,								// -120201 Numeric account too long or contains alphabetic characters (sndaID format error)
	ERROR_SHANDAACCOUNT_120202 = 2134,								// -120202 PT account too long or all numeric (ptId format error)
	ERROR_SHANDAACCOUNT_120203 = 2135,								// -120203 Billing amount format error (Amount error)
	ERROR_SHANDAACCOUNT_120204 = 2136,								// -120204 Packet upload interval error (period error)
	ERROR_SHANDAACCOUNT_120205 = 2137,								// -120205 Account does not exist (ID not exist)
	ERROR_SHANDAACCOUNT_120206 = 2138,								// -120206 No union record (no union data)
	ERROR_SHANDAACCOUNT_120207 = 2139,								// -120207 Payment type error (paytypeId error)
	ERROR_SHANDAACCOUNT_120208 = 2140,								// -120208 Numeric account does not exist (sndaID not exist)
	ERROR_SHANDAACCOUNT_120209 = 2141,								// -120209 Other parameter format error (other param format error)
	ERROR_SHANDAACCOUNT_120210 = 2142,								// -120210 Duplicate orderid (orderid error)
	ERROR_SHANDAACCOUNT_120211 = 2143,								// -120211 serviceid error (appId error)
	ERROR_SHANDAACCOUNT_120212 = 2144,								// -120212 Network status error (net status error)
	ERROR_SHANDAACCOUNT_120213 = 2145,								// -120213 Cache flag error (flag error)
	ERROR_SHANDAACCOUNT_120214 = 2146,								// -120214 Status parameter error (status error)
	ERROR_SHANDAACCOUNT_120215 = 2147,								// -120215 Multiple records exist for one sessionid (Data repeat)
	ERROR_SHANDAACCOUNT_120216 = 2148,								// -120216 ptid and sessionid do not match (ptId and sessid error)
	ERROR_SHANDAACCOUNT_120217 = 2149,								// -120217 PT account does not exist (ptID not exist)
	// ERROR_SHANDAACCOUNT_129999, // -129999 System error + sqlcode (other error)
	// User numeric account			
	ERROR_SHANDABILLING_200101 = 2150,								// SDIDLONGER -200101 Input length exceeded long
	ERROR_SHANDABILLING_200102 = 2151,								// SDIDNULL -200102 Empty null
	ERROR_SHANDABILLING_200103 = 2152,								// SDIDILLEGAL -200103 Contains illegal string format error
	ERROR_SHANDABILLING_200199 = 2153,								// SDIDOTHER -200199 Other error other error
	// User PT account			
	ERROR_SHANDABILLING_200201 = 2154,								// PTIDLONGER -200201 Input length exceeded long
	ERROR_SHANDABILLING_200202 = 2155,								// PTIDNULL -200202 Empty null
	ERROR_SHANDABILLING_200203 = 2156,								// PTIDILLEGAL -200203 Contains illegal string format error
	ERROR_SHANDABILLING_200204 = 2157,								// PTIDALLNUM -200204 All digits or starts with a digit format error
	ERROR_SHANDABILLING_200299 = 2158,								// PTIDOTHER -200299 Other error other error
	// Service type ID			
	ERROR_SHANDABILLING_200301 = 2159,								// SERVIDLONGER -200301 Input length exceeded long
	ERROR_SHANDABILLING_200302 = 2160,								// SERVIDNULL -200302 Empty null
	ERROR_SHANDABILLING_200303 = 2161,								// SERVIDILLEGAL -200303 Contains illegal string format error
	ERROR_SHANDABILLING_200399 = 2162,								// SERVIDOTHER -200399 Other error other error
	// Game partition ID			
	ERROR_SHANDABILLING_200401 = 2163,								// AREAIDLONGER -200401 Input length exceeded (long)
	ERROR_SHANDABILLING_200402 = 2164,								// AREAIDNULL -200402 Empty (null)
	ERROR_SHANDABILLING_200403 = 2165,								// AREAIDILLEGAL -200403 Illegal characters included (format error)
	ERROR_SHANDABILLING_200499 = 2166,								// AREAIDOTHER -200499 Other error
	// Session ID			
	ERROR_SHANDABILLING_200501 = 2167,								// SESSIDLONGER -200501 Input length exceeded long
	ERROR_SHANDABILLING_200502 = 2168,								// SESSIDNULL -200502 Empty null
	ERROR_SHANDABILLING_200503 = 2169,								// SESSIDILLEGAL -200503 Contains illegal character format error
	ERROR_SHANDABILLING_200504 = 2170,								// SESSIDERRFORMAT -200504 Format error (illegal combination) format error
	ERROR_SHANDABILLING_200599 = 2171,								// SESSIDOTHER -200599 Other error other error
	// Data packet id		
	ERROR_SHANDABILLING_200601 = 2172,								// UNIQUEIDLONGER -200601 Input length exceeded long
	ERROR_SHANDABILLING_200602 = 2173,								// UNIQUEIDNULL -200602 Empty null
	ERROR_SHANDABILLING_200603 = 2174,								// UNIQUEIDILLEGAL -200603 Contains illegal characters format error
	ERROR_SHANDABILLING_200604 = 2175,								// UNIQUEIDERRFORMAT -200604 Format error (invalid combination) format error
	ERROR_SHANDABILLING_200605 = 2176,								// UNIQUEIDREPEAT -200605 Duplicate uniqueid id repeat
	ERROR_SHANDABILLING_200699 = 2177,								// UNIQUEIDOTHER -200699 Other error null
	// World ID			
	ERROR_SHANDABILLING_200801 = 2178,								// GROUPIDLONGER -200801 input length exceeded - long
	ERROR_SHANDABILLING_200802 = 2179,								// GROUPIDNULL -200802 empty - null
	ERROR_SHANDABILLING_200803 = 2180,								// GROUPIDILLEGAL -200803 illegal character included - format error
	ERROR_SHANDABILLING_200899 = 2181,								// GROUPIDOTHER -200899 other error
	// Start time			
	ERROR_SHANDABILLING_201301 = 2182,								// BEGINTIMELONGER -201301 Input length exceeded (long)
	ERROR_SHANDABILLING_201302 = 2183,								// BEGINTIMENULL -201302 Empty (null)
	ERROR_SHANDABILLING_201303 = 2184,								// BEGINTIMEILLEGAL -201303 Illegal characters included (format error)
	ERROR_SHANDABILLING_201304 = 2185,								// BEGINTIMEERRLONGFOMAT -201304 Format error, must be YYYY-MM-DD hh:mi:ss format (format error)
	ERROR_SHANDABILLING_201305 = 2186,								// BEGINTIMEERRSHORTFOMAT -201305 Illegal characters included, must be YYYY-MM-DD format (format error)
	ERROR_SHANDABILLING_201399 = 2187,								// BEGINTIMEOTHER -201399 Other error
	// End time			
	ERROR_SHANDABILLING_201401 = 2188,								// ENDTIMELONGER -201401 Input length exceeded (long)
	ERROR_SHANDABILLING_201402 = 2189,								// ENDTIMENULL -201402 Empty (null)
	ERROR_SHANDABILLING_201403 = 2190,								// ENDTIMEILLEGAL -201403 Contains illegal characters (format error)
	ERROR_SHANDABILLING_201404 = 2191,								// ENDTIMEERRLONGFOMAT -201404 Format error, must be YYYY-MM-DD hh:mi:ss format (format error)
	ERROR_SHANDABILLING_201405 = 2192,								// ENDTIMEERRSHORTFOMAT -201405 Contains illegal characters, must be YYYY-MM-DD format (format error)
	ERROR_SHANDABILLING_201406 = 2193,								// ENDTIMEERRVALUE -201406 End time is greater than start time (error value)
	ERROR_SHANDABILLING_201499 = 2194,								// ENDTIMEOTHER -201499 Other error (other error)
	// Data packet id			
	ERROR_SHANDABILLING_201501 = 2195,								// ORDERIDLONGER -201501 input length exceeded long
	ERROR_SHANDABILLING_201502 = 2196,								// ORDERIDNULL -201502 empty null
	ERROR_SHANDABILLING_201503 = 2197,								// ORDERIDILLEGAL -201503 illegal character included format error
	ERROR_SHANDABILLING_201504 = 2198,								// ORDERIDERRFORMAT -201504 format error (invalid combination) format error
	ERROR_SHANDABILLING_201505 = 2199,								// ORDERIDREPEAT -201505 duplicate orderid ID repeat
	ERROR_SHANDABILLING_201599 = 2200,								// ORDERIDOTHER -201599 other error
	// Billing type			
	ERROR_SHANDABILLING_201901 = 2202,								// PAYTYPELONGER -201901 Input length exceeded long
	ERROR_SHANDABILLING_201902 = 2203,								// PAYTYPENULL -201902 Empty null
	ERROR_SHANDABILLING_201903 = 2204,								// PAYTYPEILLEGAL -201903 Contains illegal characters format error
	ERROR_SHANDABILLING_201904 = 2205,								// PAYTYPEERRVALUE -201904 Input error (does not have a specific value) error value
	ERROR_SHANDABILLING_201999 = 2206,								// PAYTYPEOTHER -201999 Other error other error
	// Consume type			
	ERROR_SHANDABILLING_202001 = 2207,								// APPTYPELONGER -202001 Input length exceeded (long)
	ERROR_SHANDABILLING_202002 = 2208,								// APPTYPENULL -202002 Empty (null)
	ERROR_SHANDABILLING_202003 = 2209,								// APPTYPEILLEGAL -202003 Illegal characters included (format error)
	ERROR_SHANDABILLING_202004 = 2210,								// APPTYPEERRVALUE -202004 Input error (does not have a specific value) (error value)
	ERROR_SHANDABILLING_202099 = 2211,								// APPTYPEOTHER -202099 Other error
	// Coupling type			
	ERROR_SHANDABILLING_202101 = 2212,								// COUPLETYPELONGER -202101 Input length exceeded (long)
	ERROR_SHANDABILLING_202102 = 2213,								// COUPLETYPENULL -202102 Empty (null)
	ERROR_SHANDABILLING_202103 = 2214,								// COUPLETYPEILLEGAL -202103 Illegal characters included (format error)
	ERROR_SHANDABILLING_202104 = 2215,								// COUPLETYPEERRVALUE -202104 Input error (does not have a specific value) (error value)
	ERROR_SHANDABILLING_202199 = 2216,								// COUPLETYPEOTHER -202199 Other error
	// Amount			
	ERROR_SHANDABILLING_202201 = 2217,								// AMOUNTLONGER -202201 Input length exceeded (long)
	ERROR_SHANDABILLING_202202 = 2218,								// AMOUNTNULL -202202 Empty (null)
	ERROR_SHANDABILLING_202203 = 2219,								// AMOUNTILLEGAL -202203 Illegal characters included (format error)
	ERROR_SHANDABILLING_202204 = 2220,								// AMOUNTNEGATIVE -202204 Negative number entered (error value)
	ERROR_SHANDABILLING_202205 = 2221,								// AMOUNTZERO -202205 Entered 0 (error value)
	ERROR_SHANDABILLING_202206 = 2222,								// AMOUNTERRVALUE -202206 Input number is too large (error value)
	ERROR_SHANDABILLING_202299 = 2223,								// AMOUNTOTHER -202299 Other error
	// Billing discount rate			
	ERROR_SHANDABILLING_202301 = 2224,								// DISCOUNTLONGER -202301 Input length exceeded (long)
	ERROR_SHANDABILLING_202302 = 2225,								// DISCOUNTNULL -202302 Empty (null)
	ERROR_SHANDABILLING_202303 = 2226,								// DISCOUNTILLEGAL -202303 Contains illegal characters (format error)
	ERROR_SHANDABILLING_202399 = 2227,								// DISCOUNTOTHER -202399 Other error (other error)
	// Client IP address			
	ERROR_SHANDABILLING_202501 = 2228,								// IPLONGER -202501 input length exceeded - long
	ERROR_SHANDABILLING_202502 = 2229,								// IPNULL -202502 empty - null
	ERROR_SHANDABILLING_202503 = 2230,								// IPILLEGAL -202503 illegal character included - format error
	ERROR_SHANDABILLING_202504 = 2231,								// IPERRVALUE -202504 input error (out of specified range) - error value
	ERROR_SHANDABILLING_202599 = 2232,								// IPOTHER -202599 other error
	// State			
	ERROR_SHANDABILLING_204401 = 2233,								// STATUSLONGER -204401 Input length exceeded long
	ERROR_SHANDABILLING_204402 = 2234,								// STATUSNULL -204402 Empty null
	ERROR_SHANDABILLING_204403 = 2235,								// STATUSILLEGAL -204403 Contains illegal character format error
	ERROR_SHANDABILLING_204404 = 2236,								// STATUSERRVALUE -204404 Input error (does not match specified value) error value
	ERROR_SHANDABILLING_204499 = 2237,								// STATUSOTHER -204499 Other error other error
	// User uid			
	ERROR_SHANDABILLING_200211 = 2238,								// UIDLONGER -200211 Input length exceeded long
	// CBS service processing			
	ERROR_SHANDABILLING_300001 = 2239,								// CBSDB_IS_DISCONNECT -300001 cbs DB connection error (sql connect error)
	ERROR_SHANDABILLING_300102 = 2240,								// CBSDB_PROC_ERROR -300002 cbs sql execution error (sql error)
	ERROR_SHANDABILLING_300103 = 2241,								// UNAUTHOR_OPERATION -300003 Unauthorized request packet type unauthor
	ERROR_SHANDABILLING_300201 = 2242,								// AMOUNT_NOT_MATCHING -300201 Amount mismatch (amount error)
	ERROR_SHANDABILLING_390001 = 2243,								// RES_TIMEOUT -390001 response timeout
	// Communication return - API		
	ERROR_SHANDABILLING_401001 = 2244,								// SOCKDISCONNECT -401001 Socket disconnected socket disconnect
	ERROR_SHANDABILLING_401002 = 2245,								// SOCKIP_OR_PORTERR -401002 Server IP/port error IP Port error
	ERROR_SHANDABILLING_401003 = 2246,								// SOCKCONNECTFAIL -401003 Socket connection failed socket connect fail
	ERROR_SHANDABILLING_401004 = 2247,								// SOCKLOCALFAILED -401004 Local SOCKET initialization failed initial fail
	ERROR_SHANDABILLING_401005 = 2248,								// SNDBUF_OVERFLOW -401005 Send buffer overflow send buffer overflow
	ERROR_SHANDABILLING_401006 = 2249,								// RCVBUF_OVERFLOW -401006 receive buffer overflow
	ERROR_SHANDABILLING_401007 = 2250,								// TMOBUF_OVERFLOW	-401007	timeout buffer overflow	timeout buffer overflow
	ERROR_SHANDABILLING_401008 = 2251,								// LSOCKDISCONNECT -401008 communication error - socket error
	ERROR_SHANDABILLING_401011 = 2252,								// NOINITCLIENT -401011 Client not initialized / client without initial
	// QUEUE processing			
	ERROR_SHANDABILLING_402001 = 2253,								// QUEUECACHEFULL -402001 queue cache full
	ERROR_SHANDABILLING_402002 = 2254,								// QUEUELOCKERROR -402002 queue lock error
	ERROR_SHANDABILLING_402003 = 2255,								// QUEUEOPERATEERROR -402003 queue Push/Pop parameter error (null pointer) - queue param error
	ERROR_SHANDABILLING_402004 = 2256,								// QUEUEEMPTY -402004 queue is empty, no Pop data - queue null
	ERROR_SHANDABILLING_402005 = 2257,								// QUEUEDATAERR -402005 cache content error, length exceeds cacheable range, etc. - queue data error
	// Send data			
	ERROR_SHANDABILLING_403001 = 2258,								// USERDATALENERROR -403001 Mismatch between user data Push length and service struct length data length error
	ERROR_SHANDABILLING_403002 = 2259,								// COMMUDATAERROR -403002 Received communication data packet encryption error (encrypt error)
	// POINTER insert			
	ERROR_SHANDABILLING_404001 = 2260,								// POINTERNULL -404001 Empty pointer inserted (cannot accept null pointer where a pointer is required) point null
	// Encode/decode data			
	ERROR_SHANDABILLING_405001 = 2261,								// ENCRYPTERROR -405001 Encryption error (encrypt error)
	ERROR_SHANDABILLING_405002 = 2262,								// DECRYPTERROR -405002 decrypt error
	// Setting info			
	ERROR_SHANDABILLING_406001 = 2263,								// PARAMPATHERROR -406001 cache or annular file path error path error
	ERROR_SHANDABILLING_406002 = 2264,								// CONFLOADFAILED -406002 Failed to load config file (fail to load ini)
	ERROR_SHANDABILLING_406003 = 2265,								// CONFGETFAILED -406003 Failed to get config from config server (fail to get ini)
	ERROR_SHANDABILLING_406004 = 2266,								// RELOADFAILED -406004 loading failed - fail to reload
	// Config server connection info			
	ERROR_SHANDABILLING_407001 = 2267,								// ERR_MSG_TYPE -407001 Message type error in request message (message type error)
	ERROR_SHANDABILLING_407002 = 2268,								// IP_NOT_EXIST -407002 hostIP missing in request message - hostip not exist
	ERROR_SHANDABILLING_407003 = 2269,								// ID_NOT_EXIST -407003 hostId missing in request message - hostid not exist
	ERROR_SHANDABILLING_407004 = 2270,								// SYS_FLAG_INVALID -407004 OS flag error system flag error
	ERROR_SHANDABILLING_407005 = 2271,								// MSG_LEN_ERR -407005 request message length error
	ERROR_SHANDABILLING_407006 = 2272,								// CANNOTCONNECTDB -407006 DB connection failed (db connect error)
	ERROR_SHANDABILLING_407099 = 2273,								// UNKNOWNED_ERR -407099 Other error other error
	// Etc			
	ERROR_SHANDABILLING_499999 = 2280,								// UNDECLARE_ERROR -499999 Other error other error
	
	//Ladder system
	ERROR_ALREADY_LADDERROOM = 2300,						// Already in a ladder room.
	ERROR_INVALIDUSER_LADDERROOM = 2301,							// Invalid user not in the ladder room.
	ERROR_LADDERSYSTEM_CANT_MATCHINGCANCLE_ROOMSTATE = 2302,		// State does not allow canceling matching.
	ERROR_LADDERSYSTEM_CANT_MATCHING_ROOMSTATE = 2303,				// State does not allow requesting matching.
	ERROR_LADDERSYSTEM_NOLEADER = 2304,							// Not the ladder room leader.
	ERROR_LADDERSYSTEM_CANT_ENTER_LEVELLIMIT = 2305,				// Cannot enter ladder due to level restriction
	ERROR_LADDERSYSTEM_CANT_EXIST_ROOMLIST_PAGE = 2306,			// Requested non-existent room list page
	ERROR_LADDERSYSTEM_CANT_EXIST_ROOM = 2307,						// Room does not exist.
	ERROR_LADDERSYSTEM_CANT_OBSERVER_ROOMSTATE = 2308,				// Room state does not allow joining as an observer.
	ERROR_LADDERSYSTEM_LADDERPOINT_INVALID = 2309,
	ERROR_LADDERSYSTEM_EXHIBITION_DURATION = 2310,					// Currently in exhibition match period
	ERROR_LADDERSYSTEM_NOTENOUGH_USER = 2311,						// Not enough users
	ERROR_LADDERSYSTEM_CANT_INVITESTATE = 2312,					// Cannot invite to ladder room in current state.
	ERROR_LADDERSYSTEM_CANT_RECVINVITESTATE = 2313,				// Target cannot be invited to a ladder room.
	ERROR_LADDERSYSTEM_ROOM_FULLUSER = 2314,						// Ladder room is full
	ERROR_LADDERSYSTEM_INVALID_INVITEUSER = 2315,					// User was not invited
	ERROR_LADDERSYSTEM_CANT_JOIN_ROOMSTATE = 2316,					// State unable to join via invite
	ERROR_LADDERSYSTEM_INVITE_DENY = 2317,							// Reject invite
	ERROR_LADDERSYSTEM_CANT_INVITE_LEVELLIMIT = 2318,				// Cannot invite to ladder due to level restriction
	ERROR_LADDERSYSTEM_CANT_KICKOUT_ROOMSTATE = 2319,				// Room state cannot kick user
	ERROR_LADDERSYSTEM_KICKOUT = 2320,								// Kicked by the room master
	ERROR_LADDERSYSTEM_INVITING = 2321,							// Currently inviting

	// Guild war
	ERROR_GUILDWAR_CANTADD_GUILDMEMBER = 2400,				// Cannot join during guild war period.
	ERROR_GUILDWAR_CANTLEAVE_GUILDMEMBER = 2401,			// Cannot leave during guild war period.
	ERROR_GUILDWAR_CANTEXILE_GUILDMEMBER = 2402,			// Cannot kick during guild war period.
	ERROR_GUILDWAR_CANTDISMISS = 2403,						// Cannot disband guild during guild war period.
	ERROR_GUILDWAR_CANTENROLL = 2404,						// Not guild war application period.
	ERROR_GUILDWAR_ALREADY_ENROLL = 2405,					// Already applied for Guild War.	
	ERROR_GUILDWAR_ALREADY_VOTE = 2406,						// You have already voted in the popularity poll.
	ERROR_GUILDWAR_CANT_VOTE = 2407,						// Cannot cast popularity vote.
	ERROR_GUILDWAR_CANT_COMPENSATION = 2408,				// Cannot receive guild war preliminary reward.
	ERROR_GUILDWAR_NOT_PRE_WIN = 2409,						// Not the guild war winning team.
	ERROR_GUILDWAR_SKILL_COOLTIME = 2410,					// Cannot use the winner skill yet.

	//Cashshop
	ERROR_CASHSHOP_FAIL_PREVIEWEQUIP_GENERAL = 2500,		// Cannot equip costume. Please restart.
	ERROR_CASHSHOP_FAIL_PREVIEWEQUIP_CRC	 = 2501,
	ERROR_CASHSHOP_FARMVIP_DUPLICATED = 2502,						// Duplicate Farm VIP

	//Cherry Auth (SG)
	ERROR_CHERRY_AUTHFAIL = 2600,							// Singapore authentication failed
	ERROR_CHERRY_AUTHSUCCESS = 2600,								// Success
	ERROR_CHERRY_AUTHBLOCKED = 2600,								// Locked?
	ERROR_CHERRY_AUTHSUSPENDED = 2600,								// Dormant?
	ERROR_CHERRY_AUTHOTHER = 2600,									// ?????
	ERROR_CHERRY_NOTALLOWED_SPECIALCHAR = 2600,					// Special characters not allowed?
	ERROR_CHERRY_NO_SUCH_ACCOUNT = 2600,							// Account does not exist

	//Cherry Billing (SG)
	ERROR_CHERRY_COUPON_ALREADY = 2700,						// Coupon has already been used.
	ERROR_CHERRY_COUPON_EXPRIED = 2700,							// Coupon expired	
	ERROR_CHERRY_COUPON_INVALID = 2700,							// Coupon input error
	ERROR_CHERRY_COUPON_AND_CHARACTERID_MISMATCH = 2700,			// Coupon character ID mismatch

	// Recover disconnected(?) party 
	ERROR_CANT_RESTORE_PARTY_STATE = 2800,					// Cannot rejoin the party right now(?).
	ERROR_CANT_EXSIT_BACKUPPARTYINFO = 2801,						// No party info to restore

	// Pet
	ERROR_CANT_RECALL_PET_STATUS = 2900,					// Cannot summon pet in current state
	ERROR_PETNAME_SHORTAGE_2 = 2901,						// Cannot create pet because name is too short (min 2 chars)
	ERROR_PETNAME_SHORTAGE_4 = 2902,						// Cannot create pet because name is too short (min 4 chars)
	ERROR_PETNAME_PROHIBIT = 2903,						// Disallowed characters exist.
	ERROR_PETFOOD_FAIL_MAX = 2904,						// Cannot eat because satiety is full
	ERROR_PETFOOD_FAIL_NORMAL = 2905,						// Normal feed cannot be consumed above 90%.

	// Gamania - AUTH, BILLING
	ERROR_ASIASOFT_INVALID_REQUEST = 3000,				// Invalid request
	ERROR_ASIASOFT_INVALID_USERID = 3001,						// Account does not exist.
	ERROR_ASIASOFT_USER_ID_NOT_FOUND = 3002,					// User ID not found (what's the difference from above? lol keeping it since the return code exists)
	ERROR_ASIASOFT_WORNG_PASSWORD = 3003,						// Incorrect password reported
	ERROR_ASIASOFT_DISABLE_USER = 3004,						// Is this block? Marked as (Optional)
	ERROR_ASIASOFT_DATABASE_ERROR = 3005,						// Asiasoft DB error
	ERROR_ASIASOFT_UNEXPECTED_ERROR = 3006,					// Asiasoft unexpected error
	ERROR_ASIASOFT_OTP_FAILED = 3007,							// When OTP fails	

	ERROR_ASIASOFT_NO_SUCH_SERVICE = 3008,						// Non-existent service
	ERROR_ASIASOFT_NO_ENOUGH_POINT = 3009,						// Insufficient points.
	ERROR_ASIASOFT_WRONG_EVENTOTP = 3010,						// EVENT OTP is incorrect.
	ERROR_ASIASOFT_WRONG_OTP = 3011,							// OTP is incorrect.
	ERROR_ASIASOFT_ACCOUNT_LOCK = 3012,							// Account is locked.
	ERROR_ASIASOFT_SERVICE_NOT_AVAILABLE = 3013,				// Game service is not active.
	ERROR_ASIASOFT_TIME_OUT = 3014,								// Timeout
	ERROR_ASIASOFT_CONTRACT_BLOCK = 3015,						// Contract expired
	ERROR_ASIASOFT_ALREADY_LOGIN = 3016,						// Already logged in.
	ERROR_ASIASOFT_ALREADY_WAITING = 3017,						// Authentication request already in progress
	ERROR_ASIASOFT_SPECIAL_CHAR = 3018,							// Special characters cannot be used.

	//Donation
	ERROR_NOT_AVAILABLE_DONATION_TIME = 3100,			// Donation is not available at this time.

	//Potential
	ERROR_EXCHANGEPOTENTIAL_OK = 3200,									// Success
	ERROR_EXCHANGEPOTENTIAL_FAIL = 3201,								// Potential exchange failed with general error!
	ERROR_EXCHANGEPOTENTIAL_EXTRACT_ITEM_VALIDATION_FAIL = 3202,		// Failed to verify source item to extract potential
	ERROR_EXCHANGEPOTENTIAL_INJECT_ITEM_VALIDATION_FAIL = 3203,			// Failed to verify target item to insert potential
	ERROR_EXCHANGEPOTENTIAL_ITEM_COMPARE_VALIDATION_FAIL = 3204,		// Item comparison check failed (rank, parts, type, etc.)
	ERROR_EXCHANGEPOTENTIAL_MOVEABLECOUNT_EXPIRED = 3205,				// Potential transfer count limit

	// ExchangeEnchant
	ERROR_EXCHANGE_ENCHANT_FAIL = 3206,									// Enhancement transfer failed
	ERROR_EXCHANGE_ENCHANT_REQ_MONEY = 3207,							// Insufficient required gold
	ERROR_EXCHANGE_ENCHANT_REQ_ITEM = 3208,								// Insufficient required items

	// BestFriend
	ERROR_BESTFRIEND_REGISTFAIL_DESTUSER_SITUATION_NOTALLOWED = 3300,	// Not in a state to register a best friend
	ERROR_BESTFRIEND_NOTEXIST_OPPOSITEUSER = 3301,						// Target does not exist	
	ERROR_BESTFRIEND_REFUESED_REGISTRATION = 3302,						// Registration rejected
	ERROR_BESTFRIEND_ALREADY_REGISTERED = 3303,							// Already in best friend status.
	ERROR_BESTFRIEND_NOT_REGISTERED = 3304,								// Not in best friend state.
	ERROR_BESTFRIEND_NOTSEARCH = 3305,									// User not found
	ERROR_BESTFRIEND_NOTHAVE_REGISTITEM = 3306,							// You do not have a best friend registration scroll.
	ERROR_BESTFRIEND_HAVE_REWARDITEM = 3307,							// Already owns the best friend reward item.
	ERROR_BESTFRIEND_SELFSEARCH = 3308,									// Cannot search for yourself as a best friend.
	ERROR_BESTFRIEND_SELFREGIST = 3309,									// Cannot register yourself as a best friend.
	
	// Kreon - AUTH, BILLING
	ERROR_KREON_INVALID_REQUEST = 3400,							// Invalid request
	ERROR_KREON_INVALID_USERID = 3401,							// Account does not exist.	 
	ERROR_KREON_WRONG_PASSWORD = 3402,						// Incorrect password reported
	ERROR_KREON_BLOCK_GEMSCOOL = 3403,						// GEMSCOOL block (block all games)
	ERROR_KREON_BLOCK_DN	   = 3404,						// DN block (block Dragon Nest only)
	ERROR_KREON_ACCOUNT_LOCKED = 3405,						// Account locked (on 5 or more failed logins)
	ERROR_KREON_INTERNAL_ERR   = 3406,						// Kreon internal error
	ERROR_KREON_INTERNAL_DB_ERR  = 3407,						// Kreon internal DB error

	ERROR_KREON_NOT_ALLOWD_IP = 3408,						// Disallowed IP
	ERROR_KREON_NO_ENOUGH_POINT = 3409,						// Insufficient balance
	ERROR_KREON_LIMIT_AMOUNT = 3410,						// When the monthly payment limit set for the member's tier is exceeded
	ERROR_KREON_PG_LIMIT_AMOUNT = 3411,						// When the PG payment limit amount is exceeded	
	ERROR_KREON_BLOCK_PCCAFE = 3412,						// This PC bang is blocked from connecting. Please contact the Gameschool Premium Warnet administrator for inquiries.

	// Duplicate login message
	ERROR_DUPLICATEUSER_INGAME = 3413,						// Disconnected because logged in from another location.

	// MailRU - Auth
	ERROR_MAILRU_INVALID_REQUEST = 3500,					// Invalid request
	ERROR_MAILRU_WORNG_PASSWORD = 3501,						// Invalid ID/OTP hash (authentication failed)
	ERROR_MAILRU_BLOCK = 3502,								// Banned user
	ERROR_MAILRU_ERROR = 3503,								// AUth Error	

	// EU - Auth
	ERROR_EUAUTH_SPECIALCHAR = ERROR_ASIASOFT_SPECIAL_CHAR,	// ERROR_ASIASOFT_SPECIAL_CHAR = 3018, // Special characters cannot be used. - Using the same UISTRING from Asiasoft anyway.

	// Altea Continent Exploration message
	ERROR_ALTEIAWORLD_ENTERLIMITTIME = 3600,				// Entry prohibited time
	ERROR_ALTEIAWORLD_NOTEXISTUSER = 3601,					// No user found.

	// Talisman message
	ERROR_TALISMAN_FAIL = 3700,								// Talisman error
	ERROR_TALISMAN_FAIL_LOACTE = 3701,						// Talisman error, cannot use in this location
	ERROR_TALISMAN_DUPLICATE = 3702,						// Error: duplicate equipping of the same talisman type
	ERROR_TALISMAN_EQUIP_FAIL = 3703,						// Failed to equip talisman
	ERROR_TALISMAN_SLOT_ALERADY_OPEN = 3704,				// Slot is already open
	ERROR_TALISMAN_SLOT_NOT_OPEND = 3705,					// Talisman slot is not open
	ERROR_TALISMAN_SLOT_NOT_SERVICE = 3706,					// Slot not in service
	ERROR_TALISMAN_SLOT_LIMITLEVEL = 3707,					// Character level is too low to place in slot
	ERROR_TALISMAN_LIMITLEVEL = 3708,						// Character level is too low to equip

	// DWC message
	ERROR_DWC_FAIL = 3800,									// DWC error
	ERROR_DWC_PERMIT_FAIL = 3801,							// No DWC permission
	ERROR_DWC_ALREADY_HASTEAM = 3802,						// Already joined a team
	ERROR_DWC_PROHIBITWORD = 3803,							// Unusable name
	ERROR_DWC_CANT_INVITE_YOURESELF = 3804,					// Cannot invite yourself
	ERROR_DWC_DUPLICATION_JOB = 3805,						// The same class already exists in the team	
	ERROR_DWC_CANTADD_TEAMMEMBER = 3806,					// Cannot invite anymore	
	ERROR_DWC_CANTACCEPT_TEAMINVITATION = 3807,				// Target cannot currently receive team invitations
	ERROR_DWC_CANTACCEPT_JOINTEAM = 3808,					// Team leader is unable to accept new members
	ERROR_DWC_REFUESED_TEAMINVITATION = 3809,				// Declined team invitation
	ERROR_DWC_NOT_DWC_CHARACTER = 3810,						// Target does not exist or is not a DWC character.
	ERROR_DWC_HAVE_NOT_TEAM = 3811,							// Not belonging to a team
	ERROR_DWC_HAS_NO_AUTHORITY = 3812,						// No command permission
	ERROR_DWC_NOTEXIST_TEAMINFO = 3813,						// Team info does not exist
	ERROR_DWC_NOT_SAME_TEAM = 3814,							// Not on the same team.	
	ERROR_DWC_LADDER_MATCH_CLOSED = 3815,					// Not within matching hours.
	ERROR_DWC_CLOSED = 3816,								// It is not the DWC period.
	ERROR_DWC_LIMIT = 3817,									// DWC characters cannot use this.
	ERROR_DWC_CANT_LEAVE_TEAM = 3818,						// Cannot leave or disband the team while a team member is playing

	// Seed message
	ERROR_SEED_MAXPOINT = 3900,								// Cannot hold any more seeds.
	ERROR_SEED_INSUFFICIENCY_SEEDPOINT = 3901,				// Insufficient Seed points	
	ERROR_CASHSHOP_SEED_DISABLE = 3902,						// This item cannot be purchased with Seed.

	//etc
	// Error message
	ERROR_NOT_AVAILABLE_CHECK_RANKING_TIME = 7500,			// Cannot check ranking info during reset time (Dark Lair)
	ERROR_FARM_ENTERANCE_CONDITION = 7531,					// Farm area entry requirements not met. Please check the entry requirements again.		

	//PrivateChatChannel 
	ERROR_ALREADY_CHANNELNAME = 7810,						// Channel name already exists.
	ERROR_INVALID_CHANNELNAME = 7811,						// Invalid channel name format.
	ERROR_INVALID_CHANNELNAMELENGTH = 7812,					// Please enter a channel title within 2-10 characters
	ERROR_DUPLICATE_CHANNELJOIN = 7813,						// Cannot join more than 1 channel.
	ERROR_NOTEXIST_CHANNELNAME = 7815,						// Channel name does not exist.
	ERROR_INVALID_PRIVATECHATCHANNELPASSWORD = 7819,		// Entered an incorrect password.
	ERROR_NOTEXIST_INVITEUSER = 7820,						// User to invite does not exist.
	ERROR_ALREADY_JOINCHANNEL = 7821,						// Already participating in the channel.
	ERROR_INVALID_PIRVATECHATMEMBERNAME = 7824,				// User to kick does not exist.
	ERROR_NOTEXIST_CHANNELMEMBER = 7825,					// No user exists to delegate room master to.
	ERROR_MAX_CHANNELMEMBER = 7828,							// Exceeded the channel's user limit.

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	// PvP - ComboExerciseMode
	ERROR_CANT_USE_RECALLMONSTER = 7938,					// Cannot be used while moving.
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
#if defined(PRE_ADD_EQUIPLOCK)
	ERROR_ITEM_LOCKITEM_NOTUPGRADE = 8388,				// Enhancement is not possible while locked.
	ERROR_ITEM_LOCKITEM_NOTMOVE = 8389,					// Cannot change equipment while locked. Please unlock and try again.
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

	// Awkward to insert because it's full...
	ERROR_GIFT_COUNTOVER_GLYPH	= 10000,				// (Heraldry) Cannot receive any more
	ERROR_GIFT_COUNTOVER_FARMEXTEND = 10001,			// (Farm expansion) Cannot receive any more.
	ERROR_CANT_USE_ITEM_DAYILY = 10002,						// Check fatigue item usage count
	ERROR_ITEM_RANDOMMIX_FAIL_MONEY = 10003,			// Insufficient fee to synthesize.
	ERROR_ITEM_RANDOMMIX_FAIL_MIXER = 10004,			// Cannot synthesize because a valid synthesizer was not used.
#if defined( PRE_ADD_PVP_EXPUP_ITEM )
	ERROR_CANT_USE_PVPEXPUPITEM_DAILY = 10005,			// Daily usage limit exceeded for Colosseum EXP boost item
	ERROR_CANT_USE_PVPEXPUPITEM_LEVEL = 10006,			// Rank too low for Colosseum EXP boost item
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )
	ERROR_GIFT_COUNTOVER_TALISMAN	= 10007,			// (Talisman) Cannot receive any more

	// DB error value range 100000 ~ 199999 (Do not use this)
	ERROR_SECONDAUTH_CHECK_MAXFAILED	= 101125,		// Error due to consecutive failures exceeding the max configured count.
	ERROR_SECONDAUTH_CHECK_OLDPWINVALID = 101126,		// Existing 2nd-auth password does not match.
	ERROR_SECONDAUTH_CHECK_FAILED		= 101127,		// 2nd auth password does not match.
	ERROR_SECONDAUTH_NOTEXIST			= 101128,		// 2nd auth password is not set.
	ERROR_DB_CHANGENAME_NEXTDAY = 101191,				// You must wait 7 days before changing your character name again.
	ERROR_DB_RESTRICTED_IP = 101194,					// Exceeded max connections per IP
	ERROR_IS_NOT_DOORS_ACCOUNT = 101115,				// Not a DoAs-authenticated account
	ERROR_ALREADY_EVENTREWARDRECEIVER = 101203,			// Character has already received the event reward.
	ERROR_ALTEIAWORLD_AGAINSENDTICKET = 103369,			// Already sent a treasure map piece to that character
	ERROR_ALTEIAWORLD_NOTEXIST_SENDCHARACTER = 103370,	// Sending character does not exist.
	ERROR_ALTEIAWORLD_NOTEXIST_RECVCHARACTER = 103371,	// Target character to receive does not exist.
	ERROR_ALTEIAWORLD_SENDTICKET_MAXCOUNT = 103372,	// Exceeded the maximum quantity of treasure maps that can be sent.

	ERROR_DB_DWC_PERMIT_FAIL = 103357,					// Not a DWC character or does not exist
	ERROR_DB_DWC_NAME_ALREADYEXIST = 103358,			// DWC team name already exists
	ERROR_DB_DWC_ALREADY_HASTEAM = 103359,				// Already joined a team
	ERROR_DB_DWC_USER_NOT_FOUND = 103360,				// Non-existent character
	ERROR_DB_DWC_TEAM_NOT_FOUND = 103361,				// Non-existent team
	ERROR_DB_DWC_DEL_AFTER_LEAVE_DWCTEAM = 103376,		// Cannot delete a character that belongs to a DWC team
};

enum eServerMsgIndex
{
	SERVERMSG_INDEX125	=	125,							// Guild festival mission can no longer be completed as all fatigue is used
	SERVERMSG_INDEX626  =   626,							// Acquired Level Up Reward Box. Check your Cash Gift Box.
	SERVERMSG_INDEX6211 =   6211,							// PC Bang rental item has been removed. If equipped, the equipment item will be gone, so please be sure to check.	
	SERVERMSG_INDEX7884 =   7884,							// Welcome back, hero! Press 'I' right now to open the inventory window and check the Cash tab!
};

enum eAccountLevel
{
	AccountLevel_New = 10,			// Recruit
	AccountLevel_Intern = 15,		// Intern
	AccountLevel_Monitoring = 20,	// Monitoring
	AccountLevel_Master = 30,		// Master	
	AccountLevel_QA = 99,			// QA
	AccountLevel_Developer = 100,	// Developer
	AccountLevel_DWC = 110,			// DWC tournament account
};

enum eAccountCmdAuth //int type.. bit operation
{	
	CmdAuth_New		   = 1,
	CmdAuth_Intern	   = 2,
	CmdAuth_Monitoring = 4,
	CmdAuth_Master	   = 8,
	CmdAuth_QA		   = 16,
	CmdAuth_Developer  = 32,	
};

//party
enum ePartyRefreshSubject
{
	PARTYREFRESH_NONE,						//Cases not needing display or notification (e.g., re-entering village from game server, creating a party, etc.)
	PARTYREFRESH_CHANGE_LEADER,
	PARTYREFRESH_MODIFY_INFO,
	PARTYREFRESH_RETURNVILLAGE,				//When returning to village
};

enum eMapType
{
	MAPTYPE_VILLAGE,
	MAPTYPE_FIELD,
};

enum eDungeonClearType
{
	DUNGEONCLEAR_F = 0,
	DUNGEONCLEAR_D,
	DUNGEONCLEAR_C,
	DUNGEONCLEAR_B,
	DUNGEONCLEAR_A,
	DUNGEONCLEAR_S,
	DUNGEONCLEAR_SS,
	DUNGEONCLEAR_SSS,
};

enum eItemMoveType
{
	MoveType_Equip = 1,					// EQUIP <-> EQUIP
	MoveType_Inven = 2,					// INVEN <-> INVEN
	MoveType_Ware = 3,					// WARE <-> WARE
	MoveType_QuestInven = 4,			// QUEST INVEN <-> QUEST INVEN
	MoveType_EquipToInven = 5,			// EQUIP -> INVEN
	MoveType_InvenToEquip = 6,			// INVEN -> EQUIP
	MoveType_InvenToWare = 7,			// INVEN -> WARE
	MoveType_WareToInven = 8,			// WARE -> INVEN
	MoveType_Pickup = 9,				// Pick up
	MoveType_Drop = 10,					// Move to trash
	MoveType_Glyph = 11,				// GLYPH <-> GLYPH
	MoveType_GlyphToInven = 12,			// GLYPH -> INVEN
	MoveType_InvenToGlyph = 13,			// INVEN -> GLYPH

	// Guild storage
	MoveType_GuildWare = 14,			// GUILDWARE <-> GUILDWARE
	MoveType_InvenToGuildWare = 15,		// INVEN -> GUILDWARE
	MoveType_GuildWareToInven = 16,		// GUILDWARE -> INVEN
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	MoveType_MailToGuildWare = 17,		// GuildMail -> GuildWare
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	// Server storage
	MoveType_InvenToServerWare = 18,	// Inven -> ServerWare
	MoveType_ServerWareToInven = 19,	// ServerWare -> Inven
	MoveType_CashToServerWare = 20,		// Cash -> ServerWare
	MoveType_ServerWareToCash = 21,		// ServerWare -> Cash

#endif //#if defined(PRE_ADD_SERVER_WAREHOUSE)

	// Cash side
	MoveType_CashEquip = 30,			// EQUIP <-> EQUIP
	MoveType_CashInven = 31,			// INVEN <-> INVEN
	MoveType_CashEquipToCashInven = 32,	// EQUIP -> INVEN
	MoveType_CashInvenToCashEquip = 33,	// INVEN -> EQUIP
	MoveType_CashGlyph = 34,			// GLYPH <-> GLYPH
	MoveType_CashGlyphToCashInven = 35,	// GLYPH -> INVEN
	MoveType_CashInvenToCashGlyph = 36,	// INVEN -> GLYPH

	// Mount
	MoveType_VehicleBodyToVehicleInven = 40,	// VehicleBody <-> VehicleInven
	MoveType_VehicleInvenToVehicleBody = 41,	// VehicleInven <-> VehicleBody
	MoveType_VehiclePartsToCashInven = 42,		// VehicleParts <-> CashInven
	MoveType_CashInvenToVehicleParts = 43,		// CashInven <-> VehicleParts

	// Pet
	MoveType_PetBodyToPetInven = 50,	// PetBody <-> PetInven
	MoveType_PetInvenToPetBody = 51,	// PetInven <-> PetBody
	MoveType_PetPartsToCashInven = 52,		// PetParts <-> CashInven
	MoveType_CashInvenToPetParts = 53,		// CashInven <-> PetParts

	// Talisman
	MoveType_Talisman = 60,				// TALISMAN <-> TALISMAN
	MoveType_TalismanToInven = 61,		// TALISMAN -> INVEN
	MoveType_InvenToTalisman = 62,		// INVEN -> TALISMAN

	// Type of newly added item to inventory (tacked on here..)
	CREATEINVEN_PICKUP = 1,			// When acquired by picking up
	CREATEINVEN_QUEST,				// When entering through a quest script
	CREATEINVEN_ETC,				// All remaining cases other than the above
	CREATEINVEN_PICKUP_DICE,		// When obtaining a bound item by rolling dice
	CREATEINVEN_REPURCHASE,			// Repurchase
};

enum eEquipType
{
	EQUIP_FACE,				// Face
	EQUIP_HAIR,				// Head
	EQUIP_HELMET,			// Helmet
	EQUIP_BODY,				// Upper body
	EQUIP_LEG,				// Bottoms
	EQUIP_HAND,				// Hand
	EQUIP_FOOT,				// Foot
	EQUIP_NECKLACE,
	EQUIP_EARRING,
	EQUIP_RING1,
	EQUIP_RING2,
	EQUIP_WEAPON1,			// Weapon
	EQUIP_WEAPON2,

	EQUIPMAX,			// For normal item equipment
};

enum eCashEquipType
{
	CASHEQUIPMIN,
	CASHEQUIP_HELMET = CASHEQUIPMIN, // Helmet
	CASHEQUIP_BODY,				// Upper body
	CASHEQUIP_LEG,				// Bottoms
	CASHEQUIP_HAND,				// Hands (Gloves)
	CASHEQUIP_FOOT,				// Feet (Shoes)
	CASHEQUIP_NECKLACE,			// Necklace
	CASHEQUIP_EARRING,			// Earring
	CASHEQUIP_RING1,			// Ring
	CASHEQUIP_RING2,			// Ring
	CASHEQUIP_WING,				// Wing/
	CASHEQUIP_TAIL,				// Tail
	CASHEQUIP_FACEDECO,			// Face decoration
	CASHEQUIP_FAIRY,			// Spirit
	CASHEQUIP_WEAPON1,			// Weapon
	CASHEQUIP_WEAPON2,
	CASHEQUIP_EFFECT,			// Common effects (mounts, pets, etc...)
	CASHEQUIPMAX,

	HIDEHELMET_BITINDEX = 31,	// Last bit flag for cash equipment toggle (VIEWCASHEQUIPMAX_BITSIZE * 8 - 1)
};

enum eItemRank
{
	ITEMRANK_D,		// NORMAL
	ITEMRANK_C,		// MAGIC
	ITEMRANK_B,		// RARE
	ITEMRANK_A,		// EPIC
	ITEMRANK_S,		// UNIQUE
	ITEMRANK_SS,	// HEROIC
	ITEMRANK_SSS,	// NAMED
	ITEMRANK_MAX,
	ITEMRANK_NONE,	// Note : Used only to distinguish party looting.
};

enum eItemPositionType
{
	ITEMPOSITION_NONE,
	ITEMPOSITION_EQUIP = 1,			// Equipment window
	ITEMPOSITION_INVEN,				// Inventory window
	ITEMPOSITION_WARE,				// Storage window
	ITEMPOSITION_QUESTINVEN,		// Quest inventory window
	// Cash-related
	ITEMPOSITION_CASHEQUIP,			// Equipment window (cash)
	ITEMPOSITION_CASHINVEN,			// Inventory window (Cash)

	ITEMPOSITION_CASHREMOVE,        // Inventory window (Delete Cash)

	ITEMPOSITION_GLYPH,				// Crest
	ITEMPOSITION_VEHICLE,			// Mount inventory
	ITEMPOSITION_SERVERWARE,		// Server storage
	ITEMPOSITION_SERVERWARECASH,	// Server cash storage

	ITEMPOSITION_TALISMAN,			// Talisman

	ITEMPOSITION_GUILD = 1,			// Guild storage window
};

enum eGlyphType
{
	GLYPETYPE_NONE = 0,
	GLYPETYPE_ENCHANT = 1,
	GLYPETYPE_SKILL = 2,
	GLYPETYPE_SPECIALSKILL = 3,
	GLYPHTYPE_ALL = 4,
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	GLYPHTYPE_DRAGON = 5,			// Dragon Expedition
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
};

enum eGlyph
{
	GLYPH_ENCHANT1,			// Heraldry - Enhancement
	GLYPH_ENCHANT2,
	GLYPH_ENCHANT3,
	GLYPH_ENCHANT4,
	GLYPH_ENCHANT5,
	GLYPH_ENCHANT6,
	GLYPH_ENCHANT7,
	GLYPH_ENCHANT8,
	GLYPH_SKILL1,			// Heraldry - Skill
	GLYPH_SKILL2,
	GLYPH_SKILL3,
	GLYPH_SKILL4,
	GLYPH_SPECIALSKILL,		// Heraldry - Special Skill
	GLYPH_CASH1,			// Heraldry - Cash
	GLYPH_CASH2,
	GLYPH_CASH3,
	// Nothing else may be inserted between GLYPHMAX and CASH. -- kimbap --	
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	GLYPH_DRAGON1,
	GLYPH_DRAGON2,
	GLYPH_DRAGON3,
	GLYPH_DRAGON4,
#endif //#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	GLYPHMAX,
};

#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
const int GLYPH_CASH_TOTAL_MAX = 3;
const int GLYPH_DRAGON_TOTAL_MAX = 4;
#endif //#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)

enum eTalisman
{
	TALISMAN_CASH1 = 0,
	TALISMAN_CASH2 = 1,
	TALISMAN_CASH3 = 2,
	TALISMAN_CASH4 = 3,
	TALISMAN_CASH_MAX = TALISMAN_CASH4,

	TALISMAN_BASIC_START = 4,
	TALISMAN_BASIC_END = 11,

	TALISMAN_EXPANSION_START = 12,

	TALISMAN_MAX = 28,

	// To expand beyond 32 extra slots, the data type of CDNUserItem::m_nTalismanOpenFlag must be changed.
	TALISMAN_EXPANSION_SLOT_LIMIT = 32,
};

enum eTalismanSlotType
{
	TALISMAN_BASIC = 0, 
	TALISMAN_EXPANSION = 1,
	TALISMAN_CASH_EXPANSION = 2,
};

enum ePartyItemLootRule
{
	ITEMLOOTRULE_NONE = 0,
	ITEMLOOTRULE_RANDOM,
	ITEMLOOTRULE_LEADER,
	ITEMLOOTRULE_INORDER,
	ITEMLOOTRULE_OWNER,
	ITEMLOOTRULE_MAX,
};

enum eChatType
{
	CHATTYPE_NORMAL = 1,	// Normal conversation
	CHATTYPE_PARTY,			// Party chat
	CHATTYPE_PRIVATE,		// Whisper
	CHATTYPE_GUILD,			// Guild chat - guild packet has different structure so instead of SCChat struct, S
	CHATTYPE_CHANNEL,		// Channel chat (will trade channels etc. be added?)
	CHATTYPE_SYSTEM,		// System
	CHATTYPE_WORLD,			// World chat
	CHATTYPE_NOTICE,		// Notice
	CHATTYPE_TEAMCHAT,		// Team chat

	//CHATTYPE_NOTICEZONE, // Map notice
	CHATTYPE_GM,			// GM chat
	CHATTYPE_PARTY_GM,		// GM party chat
	CHATTYPE_PRIVATE_GM,	// GM whisper
	CHATTYPE_DICE,
	CHATTYPE_CHATROOM,		// Chat room message
	CHATTYPE_RAIDNOTICE,	// Raid notice
	CHATTYPE_BLIND,			// Message sent for blind
	CHATTYPE_GUILDWAR,		// Messages used in guild war mode
	CHATTYPE_PRIVATE_MOBILE,// Mobile whisper	
	CHATTYPE_PRIVATECHANNEL,// Private channel chat
	CHATTYPE_GUILD_MOBILE,	// Mobile guild tournament

	//CHATTYPE_DWCZONE
	CHATTYPE_DWC_PRIVATE,	// Whisper between DWC characters -> Used only on server side. Sent to client as CHATTYPE_PRIVATE
	CHATTYPE_DWC_TEAM,		// DWC team chat

	CHATTYPE_WORLD_POPMSG,	// World chat (Red Messenger Bird)

	// Global system message
	WORLDCHATTYPE_ENCHANTITEM = 1,	// Enhanced item
	WORLDCHATTYPE_MISSION,			// Mission
	WORLDCHATTYPE_CHARMITEM,		// Charm item
	WORLDCHATTYPE_DONATION,
	WORLDCHATTYPE_NAMEDITEM,		// Named item acquisition message
	WORLDCHATTYPE_CHAOSITEM,		// Chaos item
	WORLDCHATTYPE_BESTFRIEND,		// Best friend notification
	WORLDCHATTYPE_CHOICEITEM,		// Choice item
	WORLDCHATTYPE_DIRECTPARTYBUFF,	// Party member buff item (sent only to party members)
	WORLDCHATTYPE_COSMIX,			// Costume synthesis
};

enum eQuickSlotType
{
	QUICKSLOTTYPE_NONE,	// none
	QUICKSLOTTYPE_ITEM,	// item
	QUICKSLOTTYPE_SKILL,	// skill
	QUICKSLOTTYPE_GESTURE,	// Gesture
	QUICKSLOTTYPE_SECONDARYSKILL,	// SecondarySkill (fishing, cooking)
	QUICKSLOTTYPE_VEHICLE_PET,
};

struct MailType
{
	enum ReadState
	{
		NoRead = 0,		// Unread state
		Read = 1,		// Read state
	};

	enum Attach
	{
		AttachItem = 1,	// Item attachment state
	};

	enum Delivery
	{
		Normal = 1,		// Normal mail
		Premium = 2,	// Premium
		ReplyGift = 3,	// Gift reply
	};
	
	enum Kind
	{
		NormalMail				= 1,	// Normal mail
		MissionMail				= 2,	// Mission mail
		AdminMail				= 3,	// Admin mail
		EventMail				= 4,	// Event mail
		VIPMail					= 5,	// VIP
		MasterSystemMail		= 6,	// Master-apprentice system
		NpcReputationMail		= 7,	// NPC favorability system
		Quest					= 8,	// Quest
		GuildWarReward			= 9,	// Guild war reward mail
		SpecialBox_Account		= 10,	// Special Storage - Account Target 
		SpecialBox_Character	= 11,	// Special Storage - Current Character Target
		Cadge					= 18,	// Wish request
		GuildMaxLevelReward		= 19,	// Max level guild reward for first guild join
		AppellationBookReward	= 20,	// Title collection book completion reward
	};
};

enum eSortType
{
	MARKETSORT_ITEM_DESC = 0,	// Item descending order
	MARKETSORT_ITEM_ASC,		// Item ascending order
	MARKETSORT_LEVEL_DESC,		// Level descending
	MARKETSORT_LEVEL_ASC,		// Level ascending
	MARKETSORT_PRICE_DESC,		// Price descending
	MARKETSORT_PRICE_ASC,		// Price ascending
	MARKETSORT_ONEPRICE_DESC,	// Unit price descending
	MARKETSORT_ONEPRICE_ASC,	// Unit price ascending
	MARKETSORT_ITEMGRADE_DESC,	// Enhancement descending
	MARKETSORT_ITEMGRADE_ASC,	// Enhancement ascending
};

// Quest-related
// TCount
enum EnumCountingType
{
	CountingType_None	= 0,
	CountingType_MyMonsterKill = 1,
	CountingType_AllMonsterKill = 2,
	CountingType_Item	= 3,
	CountingType_SymbolItem = 4,
	CountingType_StageClear = 5,
	CountingType_Max
};

// TQuest
enum EnumQuestType
{
	QuestType_Min = 0,
	QuestType_SubQuest = 1,
	QuestType_MainQuest = 2,
	QuestType_PeriodQuest = 3,
	QuestType_RemoteQuest = 4,
	QuestType_Max,
};

enum EnumRemoteQuestType
{
	RemoteQuestType_Normal = 0,
	RemoteQuestType_Meta = 1,
};

enum EnumQuestState
{
	QuestState_None = 0,
	QuestState_Playing = 1,
	QuestState_Recompense = 2	//Used only as completion flag for remote quests - 2012-09-11 by stupidfox.
};

enum EnumPeriodQuestResetType
{
	PERIODQUEST_RESET_EVENT = 0,	// Event
	PERIODQUEST_RESET_DAILY = 1,	// Daily
	PERIODQUEST_RESET_WORLD = 2,	// World
	PERIODQUEST_RESET_WEEKLY = 7,	// Weekly
	PERIODQUEST_RESET_MAX,
};

enum EnumPeriodQuestTime
{
	PERIODQUEST_TIME_START = 0,
	PERIODQUEST_TIME_END = 1,
	PERIODQUEST_TIME_MAX = 2,
};

enum EnumQuestRecompenseType
{
	QuestRecompense_None = 0,
	QuestRecompense_All = 1,
	QuestRecompense_Select = 2
};

enum EnumExtraType
{
	ExtraType_None = 0,
	ExtraType_Counting = 1,
};

enum eDailyMissionType		// Daily Mission
{
	DAILYMISSION_DAY = 1,		// Daily mission
	DAILYMISSION_WEEK = 2,		// Weekly mission
	DAILYMISSION_GUILDWAR = 3,	// Guild war mission
	DAILYMISSION_PCBANG = 4,	// PC cafe mission
	DAILYMISSION_GUILDCOMMON = 5, // Normal guild mission
	DAILYMISSION_WEEKENDEVENT = 6,	// Weekend event mission
	DAILYMISSION_WEEKENDREPEAT = 7, // Repeatable mission
#if defined( PRE_ADD_MONTHLY_MISSION )
	DAILYMISSION_MONTH = 8,	//Monthly mission
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )
};

// China fatigue
enum eFCMState
{
	// Fatigue state value
	FCMSTATE_NONE = 0,
	FCMSTATE_HALF = 1,		// 50% (3 - 5 hours, notify every 30 minutes)
	FCMSTATE_ZERO = 2,		// 0% (5+ hours, notify every 15 min)
};


enum eExchangeButton
{
	EXCHANGE_REGIST = 1,	// Register
	EXCHANGE_CONFIRM,		// Confirm
};

enum eLeaveType
{
	LEAVE_LOGOUT,		// Logout
	LEAVE_MOVE,				// Move
};

// 2nd auth
namespace SecondAuth
{
	struct Common
	{
		enum
		{
			LimitCount			= 10,		// 2nd auth consecutive failure threshold
			PWMaxLength			= 4,		// Max length of secondary password
			NotifyDate			= 3,		// 2nd auth notification threshold date
			RestraintReasonID	= 5,
			RestraintTimeMin	= 10,
		};
	};

	struct DBMsg
	{
		enum
		{
			ShowDialog		= 101139,	// Show message because 2nd auth re-notification cooldown has passed.
		};
	};
}

// PvP
namespace PvPCommon
{
	struct CaptainType
	{
		enum eCode
		{
			Captain = 0,
			GroupCaptain,
			Max,
		};
	};

	struct RoomOption
	{
		enum eCode
		{
			None				= 0,
			BreakInto			= 1,
			DropItem			= (1<<1),
			ShowHP				= (1<<2),
			RandomTeam			= (1<<3),
			NoRegulation		= (1<<4),
			AllKill_RandomOrder = (1<<5),
#ifdef PRE_MOD_PVPOBSERVER
			AllowObserver = (1<<6),
#endif		//#ifdef PRE_MOD_PVPOBSERVER
		};
	};

	struct RoomType
	{
		enum eRoomType
		{
			beginner,
			regular,
			max,
		};
	};

	struct Common
	{
		enum
		{
			MaxObserverPlayer				= 4,								// Max observer count
			ExtendMaxObserverPlayer = 16,										// Max observer count
			//MaxPlayer = PARTYMAX+MaxObserverPlayer, // Max players (including observers)
			MaxPlayer						= PARTYMAX,		// Max players (including observers)
			MaxPVPPlayer					= PARTYMAX,							// Max players for PVP game mode
			MaxTeamPVPPlayer				= MaxPVPPlayer/4,					// Max players per team (may change if multi-team battle mode is added) 	
			MaxRoomCount					= 1000,								// Max room count
			RoomPerPage						= 10,								// Number of rooms per page
			PvPVillageStartPositionGateNo	= 1,								// Start position gate number when entering PvP town
			PvPVillageToLobbyGateNo			= 2,								// Gate number when moving from PvP town -> lobby
			RoomListRefreshGapTime			= 5000,								// PvP room list refresh time
			GameModeStartDelta				= 3,								// Countdown time (seconds) at PvP game mode start
			RoundModeFinishDelta			= 10,								// Time interval (seconds) until next round after PvP round 1 ends
#if defined(PRE_ADD_KALI) && defined(PRE_ADD_ACADEMIC)
			MaxClass						= 6,								// Max count of PlayerActor class
#elif defined(PRE_ADD_ACADEMIC)
			MaxClass						= 5,								// Max count of PlayerActor class
#else
			MaxClass						= 4,								// Max count of PlayerActor class
#endif // #if defined( PRE_ADD_ACADEMIC )
			MaxRank							= 26,								// Max PvP Rank grade
			RegulationLevel					= 99,								// Adjusted level
			NormalXPMinPlayerCount			= 6,								// Minimum player count to receive normal XP
			PvPFinishAutoClose				= 18,								// Auto-close button time (seconds) after PvP mode ends
			PvPFinishScoreOpenDelay			= 3,								// Delay (seconds) before Score board opens after PvP mode ends
			MonsterMutationColCount			= 20,								// Monster transformation group column count
#if defined( PRE_ADD_ACADEMIC )
			MonsterMutationSkillColCount	= 120,
#else
			MonsterMutationSkillColCount	= 30,
#endif // #if defined( PRE_ADD_ACADEMIC )
			GhouConditionColCount			= 6,
			RoundStartMutationTimeSec		= 10,
			DefaultGuildCount				= 2,								// Minimum team count until multi-team matches (2+ teams) introduce variables
			MaximumGuildWarSecondCaptain	= 3,								// Guild war sub-leader max count
			MaximumCapturePosition			= 4,								// Max occupation (guild war) positions
			MaximumVitoryCondition			= 5,
			ReservedGuildSkillJobID			= 2000,								// Reserve guild-only skill job ID to reuse existing skill table; existing job IDs do not use >= 999 (confirmed with Wonseon)
			MaximumEffectCount				= 7,
			DoorCount						= 3,
			DamagePrintTime					= 60000,
			PvPBeginnerMedalMultipleFactor = 2,
			PvPStateAdjustmentTerm = (60 * 1000 * 10),
			PvPAbsoluteRankCountMax = 101,
#if defined(PRE_ADD_PVP_TOURNAMENT)
			PvPTournamentUserMax = 32,	// +32 is max enhancement
#endif
		};
	};
#if defined(PRE_ADD_PVP_TOURNAMENT)
	struct Tournament
	{
		enum eStep
		{
			TOURNAMENT_NONE = 0,
			TOURNAMENT_FINAL = 2,
			TOURNAMENT_4 = 4,
			TOURNAMENT_8 = 8,
			TOURNAMENT_16 = 16,
			TOURNAMENT_32 = 32,
		};
	};
#endif

	struct BanType
	{
		enum eBanType
		{
			Normal = 0,				// Normal kick
			Permanence,				// Permanent ban
		};
	};

	struct LeaveType
	{
		enum eLeaveType
		{
			Normal = 0,				// Normal
			Ban,					// Expel
			PermanenceBan,			// Permanent ban
			DestroyRoom,			// Destroy room
		};
	};

	struct GameType
	{
		enum
		{
			PvP = 0,
			COOP,
			Max,
		};
	};

	struct GameMode
	{
		enum
		{
			PvP_Respawn				= 0,
			PvP_Round			    = 1,
			PvP_Captain				= 2,
			PvP_IndividualRespawn	= 3,
			COOP_Defense			= 6,
			COOP_Survival			= 7,
			PvP_Zombie_Survival		= 11,
			PvP_GuildWar			= 12,
			PvP_Occupation			= 13,
			PvP_AllKill             = 14,
			PvP_Racing				= 15,
#ifdef PRE_ADD_PVP_TOURNAMENT
			PvP_Tournament = 16,
#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
			PvP_ComboExercise		= 17,
#endif
			Max,
		};
	};
	
	struct RoomSortType
	{
		enum
		{
			Index		= 0,		// Sort by room number
			GameMode,				// Sort game modes
			MapIndex,				// Sort map indices
			NumOfPlayer,			// Sort by member count
			Max,
		};
	};

	struct WaitUserList
	{
		struct Common
		{
			enum
			{
				MaxPerPage = 12,
			};
		};

		struct SortType
		{
			enum eSortType
			{
				Null		= 0,
				PvPLevel,
				Job,
				Level,
				Name,
				MAX,
			};
		};

		struct WaitUserInfo
		{
			BYTE	cPvPLevel;					// PvP rank
			BYTE	cJob;						// Job
			BYTE	cLevel;						// Level
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
			BYTE cType;							//Channel type
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
			WCHAR	wszCharName[NAMELENMAX];	// Name
		};

		struct Repository
		{
			WaitUserInfo	WaitUserList[Common::MaxPerPage];
		};
	};

	struct ItemUsageType
	{
		enum
		{
			PvPItemOnly = 0,
			Max,
		};
	};

	struct RoomState
	{
		enum
		{
			None				= 0,				// No state
			Password			= 1,				// Private room
			Starting			= (1<<1),			// Starting
			Syncing				= (1<<2),			// Synchronizing
			Playing				= (1<<3),			// Playing
			Finished			= (1<<4),			// End game mode
			NoMoreBreakInto		= (1<<5),			// Cannot intrude anymore
			// Mask
			CantBanMask			= (Starting|Syncing|Playing|Finished|NoMoreBreakInto),	// RoomState where kick is not allowed
			CantModifyRoom		= (Starting|Syncing|Playing|Finished|NoMoreBreakInto),	// RoomState that cannot edit room
		};
	};

	struct UserState
	{
		enum
		{
			None		= 0,		// No state
			Captain		= 1,		// Room master
			Ready		= (1<<1),	// Ready
			Starting	= (1<<2),	// Starting
			Syncing		= (1<<3),	// Synchronizing
			Playing		= (1<<4),	// In game.
			GuildWarCaptain			= (1<<5),	// Captain
			GuildWarSedcondCaptain	= (1<<6),	// Senior member.
			GroupCaptain = (1<<7),	// Team leader
			// Mask
			CantBanMask	= (Starting|Syncing|Playing),	// UserState where kick is not allowed
			CantLeaveMask	= (Ready|Starting|Syncing|Playing),	// UserState that cannot exit
		};
	};

	struct TxtMax
	{
		enum
		{
			RoomName	= 32,
			RoomPW		= 4,
		};
	};

	struct Team
	{
		enum eTeam
		{
			StartIndex	= 1000,
			A			= 1000,
			B			= 1001,
			EndIndex	= 1001,
			Others,
			Observer	= StartIndex+PARTYMAX+100,
			Human		= A,
			Zombie		= B,
			Max,
		};

		struct TPvPTeamIndex
		{
			UINT nTeam;
			UINT nAccountDBID[PARTYMAX];
		};

		struct  TPvPGuildWarInfo
		{
			USHORT nTeam;
			UINT nGuildDBID;
			int nQualifyingScore;
		};
	};

	struct GAMA_Command
	{
		enum
		{
			DeleteGameRoom = 0,
			FinishGameMode,
			NoMoreBreakInto,
			Max,
		};
	};

	// PvPCommon::TeamIndex is an array index for accessing PvPCommon::Team when stored as an array. Be careful not to confuse it with PvPCommon::Team.
	struct TeamIndex
	{
		enum eCode
		{
			A	= 0,
			B,
			Max,
		};
	};

	struct Check
	{
		enum
		{
			None						= 0,			
			CheckStartConditionTable	= 1,			// Check PvPStartConditonTable
			CheckTimeOver				= (1<<1),		// Check TimeOver
			CheckTeamMemberZero			= (1<<2),		// Check if an entire team has left
			CheckScore					= (1<<3),		// Score check
			AllCheck					= 0xFFFF,		// Check all
		};
	};

	struct FinishReason
	{
		enum eCode
		{
			AchieveWinCondition			= 0,			// Victory condition achieved
			TimeOver,									// Time expired
			OpponentTeamAllGone,						// All opposing team members left
			OpponentTeamAllDead,						// All opposing team members died
			OpponentCaptainGone,						// Opposing team captain left
			OpponentCaptainDead,						// Opposing team captain died
			LadderDraw,									// Ladder draw
			LadderNoGame,
			OpponentTeamBossDead,						// Opposing team boss monster died
			OpponentTeamScoreLimit,						// Opposing team reached score limit
			OpponentTeamResourceLimit,					// Opposing team reached resource limit
			OpponentTeamQualifierScore,					// ???????????
			OpponentTeamForceWin,
#if defined( PRE_ADD_RACING_MODE )			
			RacingAllFinish,							// All users crossed the finish line.
#endif // #if defined( PRE_ADD_RACING_MODE )
			Max,
		};
	};

	struct FinishDetailReason
	{
		enum eCode
		{
			None = 0,
			SuperiorHPWin,			// Win by higher HP
			SuperiorContinuousWin,	// Win streak / advantage win
			HPnContinuousWinDraw,	// Draw on tied HP and win streak
		};
	};

	struct BattleGroundVictoryState
	{
		enum eBattleGroundVictoryState
		{
			None = 0,					//Aah~
			BossKill = 1,				//Boss monster kill
			Score = (1<<1),				//Score (kill count, etc.)
			Resource = (1<<2),			//Acquire resources
			QualifierScore = (1<<3),	//With preliminaries score ???
		};
	};

	struct OccupationSystemState
	{
		enum eOccupationSystemState
		{
			None,
			Wait,
			Play,
			End,
		};
	};

	struct OccupationState
	{
		enum eOccupationState
		{
			None = 0,			//No owner
			Own = 1,			//Owned
			Try = (1<<1),		//Attempting ownership
			MoveWait = (1<<2),	//Waiting for ownership transfer
			CantTryMask	= (Try),	// UserState that cannot exit
		};
	};

	struct OccupantType
	{
		enum eOccupantType
		{
			Actor = 0,
			Team,
		};
	};

	struct OccupationVictoryCondition
	{
		enum eOccupationVictoryCondition
		{
			None,
			BossKill = 1,
			ScoreLimit,
			TeamQualifierScore,
			ResourceLimit,
		};
	};

	struct OccupationScoreType
	{
		enum eOccupationScoreType
		{
			None,
			Kill,
			Acquire,
			StealAcquire,
		};
	};	

	//Same formula as in CDNSkill, but separated since the table was separated.
	struct OccupationSkillTargetType
	{
		enum eOccupationSkillTargetType
		{
			Team = 5,
		};
	};

	struct OccuaptionSkillEffectTartgetType
	{
		enum eOccuaptionSkillEffectTartgetType
		{
			Team = 5,
		};
	};

	struct OccupationLogWinType
	{
		enum eOccupationLogWinType
		{
			TimeOut = 0,
			LimitResource = 1,
		};
	};

	struct QueryUpdatePvPDataType
	{
		enum eCode
		{
			FinishGameMode = 0,
			GoPvPLobby,
			GoSelectChar,
			Disconnect,
#if defined(PRE_MOD_PVP_LADDER_XP)
			FinishLadder,	//Add XP when ladder game ends
#endif
#if defined( PRE_ADD_PVP_EXPUP_ITEM )
			UseExpupItem,
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )
			Max,
		};
	};

#ifdef PRE_MOD_PVPRANK
	struct RankTable
	{
		enum eType
		{
			ExpValue = 0, 
			RateValue,
			AbsoluteRankValue,
			InitValue = 100,
		};
	};
#endif		//#ifdef PRE_MOD_PVPRANK

#pragma pack(push, 1)
	struct UserInfo
	{
		UINT	uiSessionID;
		int		iJobID;
		BYTE	cLevel;
		BYTE	cPvPLevel;
		USHORT	usTeam;										// PvPCommon::Team
		UINT	uiUserState;									// PvPCommon::UserState
		char cSlotIndex;
		BYTE	cCharacterNameLen;
		WCHAR	wszCharacterName[NAMELENMAX];
	};

	struct UserInfoList
	{
		UINT	uiSessionID[Common::MaxPlayer];
		int		iJobID[Common::MaxPlayer];
		BYTE	cLevel[Common::MaxPlayer];
		BYTE	cPvPLevel[Common::MaxPlayer];
		USHORT	usTeam[Common::MaxPlayer];					// PvPCommon::Team
		char cTeamIndex[Common::MaxPlayer];
		UINT	uiUserState[Common::MaxPlayer];				// PvPCommon::UserState
		BYTE	cCharacterNameLen[Common::MaxPlayer];
		WCHAR	wszBuffer[NAMELENMAX*Common::MaxPlayer];
	};

	struct RoomInfo
	{
		UINT	uiIndex;									// Room index
		UINT	uiMapIndex;									// Map index
		UINT	uiGameModeTableID;							// Game mode table ID
		UINT	uiWinCondition;
		UINT	uiPlayTimeSec;
		BYTE	cMaxUserCount;								// Max players
		BYTE	cRoomState;									// PvPCommon::RoomState
		BYTE	cRoomNameLen;
		BYTE	cMinLevel;
		BYTE	cMaxLevel;
		USHORT	unRoomOptionBit;
		int		nEventID;
		BYTE	cMinUserCount;
		bool bIsGuildWarSystem;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		BYTE cRoomType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPOBSERVER
		bool bExtendObserver;
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_WORLDCOMBINE_PVP )
		int nWorldPvPRoomType;
#endif
#ifdef PRE_PVP_GAMBLEROOM
		BYTE cGambleType;
		int nGamblePrice;
#endif // PRE_PVP_GAMBLEROOM
		WCHAR	wszBuffer[TxtMax::RoomName];
	};

	struct RoomInfoList
	{
		UINT	uiPvPIndex[Common::RoomPerPage];			// PvPIndex
		UINT	uiMapIndex[Common::RoomPerPage];			// MapIndex
		UINT	uiGameModeTableID[Common::RoomPerPage];
		BYTE	cRoomState[Common::RoomPerPage];			// PvPCommon::RoomState
		BYTE	cRoomNameLen[Common::RoomPerPage];
		BYTE	cCurUserCount[Common::RoomPerPage];			// Current user count
		BYTE	cMaxUserCount[Common::RoomPerPage];			// Max user count
		USHORT	unWinCondition[Common::RoomPerPage];		// Win condition
		BYTE	cMinLevel[Common::RoomPerPage];
		BYTE	cMaxLevel[Common::RoomPerPage];
		USHORT	unRoomOptionBit[Common::RoomPerPage];
		int		nEventID[Common::RoomPerPage];
		UINT nGuildDBID[Common::RoomPerPage][PvPCommon::TeamIndex::Max];			//Ignored if 0; otherwise dedicated room for that guild
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		BYTE cRoomType[Common::RoomPerPage];		//PvPCommon::RoomType
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPOBSERVER
		bool bExtendObserver[Common::RoomPerPage];
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_WORLDCOMBINE_PVP )
		int nWorldPvPRoomType[Common::RoomPerPage];
#endif
#if defined( PRE_PVP_GAMBLEROOM )
		BYTE cGambleType[Common::RoomPerPage];
		int nGamblePrice[Common::RoomPerPage];
#endif // #if defined( PRE_PVP_GAMBLEROOM )
		WCHAR	wszBuffer[Common::RoomPerPage*TxtMax::RoomName];
	};

	struct OccupationStateInfo
	{
		int nAreaID;
		int nOccupationState;		//PvPCommon::OccupationState

		UINT nOwnedUniqueID;				//User who succeeded at ownership time
		short nOwnedTemID;					//Current owning team

		UINT nTryUniqueID;					//When the attempter's PvPCommon::OccupationState is in the try state
		short nTryTeamID;
		
		UINT nMoveWaitUniqueID;				//User who succeeded in move-wait state
		short nMoveWaitTeamID;				//Move-wait state team
	};

	struct OccupationSkillSlotInfo
	{
		int nTeam;
		char cSlotIndex[QUICKSLOTMAX];
		int nSkillID[QUICKSLOTMAX];
		ULONG nSKillUseElapsedTick[QUICKSLOTMAX];			//Elapsed time after use; 0 means never used
	};

	struct AllKillMode
	{
		struct Time
		{
			enum eCode
			{
				SelectPlayerTick	= 10000,	// Time to select participating players
			};
		};

		struct WinBonus
		{
			enum eCode
			{
				HPPercent		= 10,		// Bonus HP percentage on victory
				SPPercent		= 10,		// Bonus SP percentage on victory

			};
		};
	};

#pragma pack(pop)

	//=============================================================================================
	// Function
	//=============================================================================================

	static bool CheckTeam( const USHORT usTeam )
	{
		if( usTeam >= Team::A && usTeam <= Team::B )
			return true;
		if( usTeam == PvPCommon::Team::Observer )
			return true;
		return false;
	}

	static TeamIndex::eCode Team2Index( Team::eTeam Team )
	{
		switch( Team )
		{
			case Team::A:	return TeamIndex::A;
			case Team::B:	return TeamIndex::B;
		}
		return TeamIndex::Max;
	}

	static Team::eTeam TeamIndex2Team( TeamIndex::eCode TeamIdx )
	{
		switch( TeamIdx )
		{
			case TeamIndex::A:	return Team::A;
			case TeamIndex::B:	return Team::B;
		}
		return Team::Max;
	}

	//blondy
	const int s_iMaxRoundNum = 7; 
	//blondy end
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

// NPC favorability
namespace NpcReputation
{
	struct Common
	{
		enum
		{
			MaxFavorEffectNpcCount	= 5,												// Maximum amount that can affect NPC favorability
			MaxMaliceEffectNpcCount = 5,												// Maximum amount that can affect NPC favorability
			MaxEffectNpcCount		= MaxFavorEffectNpcCount+MaxMaliceEffectNpcCount+1,

			MaxMailCount			= 12,
			MaxMailRandValue		= 1000000000,
			MaxBenefitCount			= 6,
		};
	};

	struct StoreBenefit
	{
		// The order below matches the StoreBenefit table definition, so it must NEVER be changed.
		enum
		{
			EnchantFeeDiscount = 0,
			RepairFeeDiscount,
			SellingPriceUp,
			BuyingPriceDiscount,
			FishProficiencyUp,		// Increase fishing proficiency
			CookProficiencyUp,		// Increase cooking proficiency
			CultivateProficiencyUp,	// Increase farming proficiency
			Max,
		};
	};
	
	struct UnionType
	{
		enum
		{
			Commercial = 0,
			Liberty,
			Royal,
			Etc,
			Max,
		};
	};
};

// Max number of item types an npc can receive as gifts. Currently up to 6 types can be specified in the table.
const int MAX_ACCEPT_NPC_PRESENT_VARIATION_COUNT = 6;

// Misc item ID used for UI display when quest reward increases favorability 
// Prevent it from actually entering the inventory.
const int NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID = 400204;		// Reward that increases affinity. 
const int NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID = 400250;		// Bromide reward.

// Max count of affinity shop benefit % data pairs
const BYTE STORE_BENEFIT_MAX = 10;

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

// Master-apprentice system
namespace MasterSystem
{
	struct Common
	{
		enum
		{
			RefreshCacheTimeSec			= 30,	// Caching time between Server<->DB M/W (sec)
		};
	};

	struct Max
	{
		enum
		{
			MasterCount				= 3,		// Max master count
			PupilCount				= 3,		// Max apprentice count
			OppositeCount			= (MasterCount > PupilCount) ? MasterCount : PupilCount,
			MasterPageRowCount		= 7,		// Number of rows in master list
			SelfIntrotuctionLen		= (30+1),	// Max character length of self-introduction when registering as master
			MasterAndClassmateCount	= MasterCount+((PupilCount-1)*MasterCount),
			RequestLocationCount	= ((MasterAndClassmateCount > PupilCount) ? MasterAndClassmateCount : PupilCount),
		};
	};

	struct Penalty
	{
		enum
		{
			PupilLeavePenaltyDay		= 1,									// Penalty duration when apprentice leaves master (days)
			PupilLeavePenaltySec		= PupilLeavePenaltyDay*24*60*60,		// Penalty duration when apprentice leaves master (seconds)
			MasterLeavePenaltyDay		= 3,									// Penalty duration when a master banishes a pupil (days)
			PupilLeaveSkipPenaltyDay	= 3,									// Grace period without penalty when a master banishes a pupil (days)
			PupilLeaveSkipPenaltySec	= PupilLeaveSkipPenaltyDay*24*60*60,	// Grace period without penalty when a master banishes a pupil (seconds)
		};
	};

	struct MasterClassmateKind
	{
		enum eCode
		{
			Master = 1,
			Classmate,
			Max,
		};
	};

	struct RequestLocation
	{
		enum eCode
		{
			Pupil = 1,
			MasterAndClassmate,
			ClassmateInfo,
			MyMasterInfo,
		};
	};

	struct Reward
	{
		enum
		{
			PupilCountExpBonusRate		= 10,
		};
	};

	struct EventType
	{
		enum eCode
		{
			Init	= 0,
			VillageFirstConnect,
			Join,
			Graduate,
			Graduated,
			Leave,
			DungeonClear,
		};
	};
};

namespace LadderSystem
{
	struct Common
	{
		enum eCode
		{
			StartMsgCount			= 5,
			RoomListPerPage			= 12,
			MaxObserver				= 4,
			RankingTop				= 10,
			InviteValidTick			= 5000, // Invite valid duration
		};
	};
	
	struct Stats
	{
		enum eCode
		{
			InitGradePoint	= 1500,	// Initial rating
			MinGradePoint	= 600,	// Minimum rating
		};
	};

	struct Exchange
	{
		enum eCode
		{
			MinGamePlay		= 10,	// Minimum number of games
		};
	};

	struct MatchResult
	{
		enum eCode
		{
			None = 0,
			Win,
			Lose,
			Draw,
		};
	};

	struct MatchType
	{
		enum eCode
		{
			None = 0,
			_1vs1 = 1,
			_2vs2,
			_3vs3,
			_4vs4,
			MaxCount = _3vs3,
			MaxMatchType = _4vs4,	// Max player match type
		};
	};

	static bool bIsServiceMatchType( LadderSystem::MatchType::eCode Type )
	{
		switch( Type )
		{
			case MatchType::_1vs1:
			case MatchType::_2vs2:
			case MatchType::_3vs3:
			case MatchType::_4vs4:
				return true;
		}
		return false;
	}

	static int GetNeedTeamCount( LadderSystem::MatchType::eCode Type )
	{
		switch( Type )
		{
		case LadderSystem::MatchType::_1vs1:
		case LadderSystem::MatchType::_2vs2:
		case LadderSystem::MatchType::_3vs3:
		case LadderSystem::MatchType::_4vs4:
		{
			return static_cast<int>(Type);
		}
		default: return 0;
		}
	}

	struct RoomState
	{
		enum eCode
		{
			None			= 0,
			WaitUser		= 1,		// Waiting for users for team setup (considered a normal room from this point)
			WaitMatching	= (1<<1),	// Waiting for match
			Matched			= (1<<2),	// Matched
			Starting		= (1<<3),	// Preparing to start
			Syncing			= (1<<4),	// Synchronizing
			Playing			= (1<<5),	// Playing
			GameFinished	= (1<<6),	// Game ended
			GameRoomDestroy	= (1<<7),	// Destroy game room
			Observer		= (1<<8),	// Observer
		};
	};

	struct Reason
	{
		enum eCode
		{
			Disconnect,		// Disconnect
			LeaveRoom,		// Left on their own(?)
			KickOut,		// Kicked out
		};
	};

	struct RoomStateReason
	{
		enum eCode
		{
			ERROR_NONE = 0,
			CANT_CREATE_GAMEROOM = 1,
			MAX_OBSERVER,
		};
	};
};

namespace RankingSystem
{
	const BYTE RANKINGPAGEMAX			= 10;					// Max number of Items to display per page.
	const BYTE RANKINGPAGECOUNT			= 5;					// Max number of list pages.
	const BYTE RANKINGMAX				= RANKINGPAGEMAX * RANKINGPAGECOUNT; // Max item count in ranking list.
};

// DarkLair
namespace DarkLair
{
	struct Common
	{
		enum eCommon
		{
			MaxPlayer = 4,	// Max participants for Dark Lair
			DarkLairPositionGateNo = 1,
		};
	};

	struct Rank
	{
		enum eRank
		{
			SelectTop			= 5,		// Number of top ranks to display
			SelectRankBoardTop	= 10,		// Count to display on Dark Lair ranking board
			MaxRank			= 10000,	// Max ranking
		};
	};
	enum
	{
		TopFloorAbstractMapIndex = 1000000000
	};
};

// Vehicle
namespace Vehicle
{
	struct Slot
	{
		enum eVehicleSlot
		{
			Body = 0,
			Saddle = 1,	// Mount: Saddle
			Hair = 2,	// Mount: Mane (useitem)
			Max,

			Sundries = 255,
		};
	};

	struct Common
	{
		enum eCommon
		{
			VehicleEquipBody = 255,
		};

		enum eVehicleValue
		{
			ROTATE_ANGLEPOWER = 50,
			ROTATE_YAWDELTA = 2,
		};
	};

	struct Parts
	{
		enum eVehicleParts
		{
			Default = -1,
			Saddle = 0,
			Hair = 1,
		};

		enum eVehiclePartsColorType
		{
			BodyColor = 0,
		};
	};
};

// Pet
namespace Pet
{
	struct Slot
	{
		enum ePetSlot
		{
			Body = Vehicle::Slot::Body,
			Accessory1 = Vehicle::Slot::Saddle, //Accessory 1
			Accessory2 = Vehicle::Slot::Hair, //Accessory 2
			Max = Vehicle::Slot::Max,

			Sundries = Vehicle::Slot::Sundries,
		};	
	};

	struct Skill
	{
		enum ePetSkill
		{
			Primary = 0,
			Secondary  = 1,
			Max = 2,
		};
	};

	struct Type
	{
		enum ePetType
		{
			ePETTYPE_EXPIREDATE		= 0x00000001,
			ePETTYPE_SATIETY		= 0x00000010,
		};
	};
	struct Satiety
	{
		enum eSatietySelection
		{
			SatietySelection0 = 0,
			SatietySelection1 = 30,
			SatietySelection2 = 50,
			SatietySelection3 = 80,
			SatietySelection4 = 100
		};
	};	
	const int MAX_SATIETY_SELECTION	 = 5;

	struct Common
	{
		enum eCommon
		{
			PetEquipBody = 254,
		};
	};

	struct Parts
	{
		enum ePetParts
		{
			Default = Vehicle::Parts::Default,
			PetAccessory1 = Vehicle::Parts::Saddle,
			PetAccessory2 = Vehicle::Parts::Hair,
		};

		enum ePetPartsColorType
		{
			BodyColor = 0,
			NoseColor = 1,
		};
	};	

	const float PetExpRate = 0.1f;	
};

#if defined( PRE_ADD_SECONDARY_SKILL )

namespace SecondarySkill
{
	struct Max
	{
		enum eMax
		{
			LearnProductionSkill	= 1,
			LearnManufactureSkill	= 1,
			LearnCommonSkill		= 1,
			LearnSecondarySkill		= LearnProductionSkill+LearnManufactureSkill+LearnCommonSkill,	// Max number of sub skills that can be learned simultaneously
			LearnCookingRecipe		= 20,															// Cooking recipe count
			LearnManufactureRecipe	= LearnCookingRecipe,											// Max recipe skill count among production skills
		};
	};

	struct Type
	{
		enum eType
		{
			ManufactureSkill	= 1,	// Crafting skill
			ProductionSkill		= 2,	// Production skill
			CommonSkill			= 3,	// Common skill
		};
	};

	struct SubType
	{
		enum eType
		{
			CookingSkill		= 1,	// Cooking skill
			CultivationSkill	= 2,	// Farming skill
			FishingSkill		= 3,	// Fishing skill
		};
	};

	struct ExecuteType
	{
		enum eType
		{
			Passive				= 1,	// Active
			Active				= 2,	// Passive
		};
	};

	struct Grade
	{
		enum eType
		{
			Beginning			= 1,	// Beginner
			Intermediate,				// Intermediate
			Advanced,					// Advanced
			Max,
		};
	};

	struct ManufactureSkill
	{
		enum eType
		{
			MaxManufactureItemTypeCount	= 5,	// Max crafted item result type count
			MaxToolItemCount			= 5,	// Max tool item count
			MaxMaterialItemCount		= 5,	// Max material item count
		};
	};
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

namespace Farm
{
	struct Common
	{
		enum eCommon
		{
#if defined( _FINAL_BUILD )
			SEED_UPDATE_SEC			= (5*60),		// Seed update interval
#else
			SEED_UPDATE_SEC			= (1*60),		// Seed update interval
#endif // #if defined( _FINAL_BUILD )
#if defined( PRE_ADD_VIP_FARM )
			VIP_GROWING_BOOST_ITEMID	= 348334082,			// Growth accelerator for VIP farm
			VIP_FARM_ITEMID				= 1107396608,			// VIP farm entry item
#endif // #if defined( PRE_ADD_VIP_FARM )
#ifdef PRE_MOD_OPERATINGFARM
			OPERATINGFARM_LIMIT = (1000 * 60 * 5),
#endif		//#ifdef PRE_MOD_OPERATINGFARM
		};
	};

	struct AreaType
	{
		enum eType
		{
			Public = 0,				// Common area
			Private = 1,			// Private area
		};
	};

#if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )
	struct Attr
	{
		enum eType
		{
			None			= 0,
			Vip				= 1,
			GuildChampion	= 2,
			DownScale		= 4,
		};
	};
#endif // #if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )

	struct Max
	{
		enum eMax
		{
			HARVESTITEM_COUNT				= 3,			// Maximum yield from one seed
			HARVESTDEPOT_COUNT				= 35,			// Max count fetched for harvest warehouse page
			PLANTING_TICK					= 3000,			// Seed planting time
			HARVESTING_TICK					= 3000,			// Seed harvesting time
			ADDWATERING_TICK				= 3000,			// AddWatering time
			EXTRA_SERVER_TICK				= 1500,			// Additional time used on the server
			ATTACHITEM_KIND					= 20,			// Max variety count of additional items
			PLANTING_ATTACHITEM				= 30,			// Max quantity of extra items when planting
			FARMCOUNT						= 100,			// Max farm count
			FIELDCOUNT						= 200,			// Max field count
			FIELDCOUNT_PERCHARACTER			= 30,			// Max number of fields a character can own
			FARMNAMELEN						= (24+1),		// Max farm name length
			WAREHOUSE_DEFAULT_KEEPING_DAY	= 15,			// Default storage days for farm warehouse
			PRIVATEFIELDCOUNT				= 5,			// Personal max area
			PRIVATEPLANTCOUNT				= 1,			// Max personal areas for normal user
		};
	};

	struct AreaState
	{
		enum eState
		{
			NONE,			// No state
			PLANTING,		// Planting seed
			GROWING,		// Seed planted and growing
			COMPLETED,		// Fruiting (?) state
			HARVESTING,		// Harvesting state
			HARVESTED,		// Harvested state
			// SubState
			ADDWATER,		// Watering
		};
	};

	struct ActionType
	{
		enum eType
		{
			START = 0,		// Start
			CANCEL,			// Cancel
			COMPLETE,		// Complete
		};
	};

	struct ServerSyncType
	{
		enum eType
		{
			FIELDCOUNT = 0,
			FIELDLIST,
			WAREHOUSE_ITEMCOUNT,
		};
	};

	const std::string	StartPositionStr	= "StartPosition";
	const float			ElapsedSyncPer		= 0.9f;
};

namespace Fishing
{
	struct Status
	{
		enum eStatus
		{
			FISHING_STATUS_NONE,
			FISHING_STATUS_IDLE,
			FISHING_STATUS_READY,		//State ready to start immediately
			FISHING_STATUS_FISHING,
			FISHING_STATUS_REWARD,
		};
	};

	struct Max
	{
		enum eMax
		{
			FISHINGPATTERNMAX = 10,			// Max fishing point table patterns
		};
	};

	struct Cast
	{
		enum eCast
		{
			CASTNONE,						//
			CASTAUTO,						//Auto fishing
			CASTMANUAL,						//Manual fishing
		};
	};

	struct Control
	{
		enum eControl
		{
			CONTROL_NONE,					//When released
			CONTROL_PULLING,				//When pressed
		};
	};

	const int FISHINGCHECKPERIODTICK = 200;
	const int FISHINGMAXGAUGE = 1000 * 100;
	const int FISHINGSYNCTERM = 1 * 1000;
};

namespace PCBang
{
	struct Type
	{
		enum ePCBangType			// PCBang benefit type
		{
			Exp = 0,				// PCBang EXP
			Appellation = 1,		// PCBang title
			Fatigue = 2,			// Fatigue
			RebirthCoin = 3,		// 
			MissionReward = 4,
			NestClearCount = 5,
			ClearBox = 6,			// Number of pcbang boxes
			MissionFatigue = 7,		// Fatigue mission
			MissionRebirthCoin = 8,	// Resurrection stone mission
			MissionRewardCash = 9,
		};
	};

	struct Grade
	{
		enum ePCBangGrade
		{
			None = 0,
			Normal = 1,	// Normal
			Red = 2,
			Silver = 3,
			Gold = 4,
			Premium = 5,
		};
	};
};

namespace Cash
{
	struct PaymentRules
	{
		enum ePaymentRules
		{
			None = 0,
			Total = 1,
			Credit = 2,
			Prepaid = 3,
			Petal = 4,
			Seed = 5,
		};
	};
};

#ifdef PRE_ADD_LIMITED_CASHITEM
namespace LimitedCashItem
{
	struct Common
	{
		enum eCommon
		{
			DitributingTerm = 5000,
			DefaultPacketCount = 200,
		};
	};

	struct TLimitedQuantityCashItem
	{
		int nProductID;
		int nSaleCount;				//Sold item count
	};

	struct TChangedLimitedQuantity
	{
		int nProductID;
		short nMaxCount;
	};
};
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

// Move GlobalEnum.h
namespace GlobalEnum 
{
	enum TileTypeEnum 
	{
		None,
		DryGround,
		ShortGrass,
		LongGrass,
		Stone,
		WetGround,
		Sand,
		Snow,
		Water,
		Temp1,
		Temp2,
		Temp3,
		Temp4,
		Temp5,
		TileTypeEnum_Amount = 14,
	};

	enum ClientSessionTypeEnum 
	{
		Unknown,
		Tcp,
		Udp,
		Auto,
	};

	//	Village Channel Info
	enum eChannelAttribute
	{
		CHANNEL_ATT_NORMAL			= 0x00,
		CHANNEL_ATT_PVP				= 0x01,
		CHANNEL_ATT_PVPLOBBY		= 0x02,
		CHANNEL_ATT_MERITBONUS		= 0x04,
		CHANNEL_ATT_GM				= 0x08,
		CHANNEL_ATT_DARKLAIR		= 0x10,
		CHANNEL_ATT_FARMTOWN		= 0x20,
		CHANNEL_ATT_DWC				= 0X40,
		CHANNEL_ATT_USELASTMAPINDEX = (CHANNEL_ATT_PVP|CHANNEL_ATT_GM|CHANNEL_ATT_DARKLAIR|CHANNEL_ATT_PVPLOBBY|CHANNEL_ATT_FARMTOWN),
	};

	enum eMeritBonus
	{
		MERIT_BONUS_NONE = 0,
		MERIT_BONUS_COMPLETEEXP = 1,
	};

	enum eMapTypeEnum
	{
		MAP_UNKNOWN,
		MAP_VILLAGE,
		MAP_WORLDMAP,
		MAP_DUNGEON,
	};

	enum eMapSubTypeEnum 
	{
		MAPSUB_NONE,
		MAPSUB_NEST,
		MAPSUB_PVPVILLAGE,
		MAPSUB_PVPLOBBY,
		MAPSUB_EVENT,
		MAPSUB_FARM,
		MAPSUB_CHAOSFIELD,
		MAPSUB_DARKLAIR,
		MAPSUB_NESTNORMAL,		// #47865 Map subtype for nest practice mode.
		MAPSUB_DARKLAIRWORLDZONE,
		MAPSUB_TREASURESTAGE,
		MAPSUB_FELLOWSHIP,
	};

	struct AllowItemMapType
	{
		enum
		{
			Village		= 0x0001,
			WorldMap	= 0x0002,
			Dungeon		= 0x0004,
			PvP			= 0x0008,
			DarkLair	= 0x0010,
			Nest		= 0x0020,
		};
	};

	enum eDefaultStartGate
	{
		DEFAULT_STARTPOSITION_GATENO = 1,
	};
};

//UserState
enum eLocationState
{
	_LOCATION_NONE = 0,		//This is the Ophu (offline?) line
	_LOCATION_MOVE,			//Wondering if we should check server-transferring state too, so making it for now
	_LOCATION_LOGIN,
	_LOCATION_VILLAGE,
	_LOCATION_GAME,
};

#if defined(PRE_ADD_WORLD_EVENT)
namespace WorldEvent
{	
	enum eJobClassType
	{
		All = 0,
		Warrior = 1,
		Archer = 2,
		Soceress = 3,
		Cleric = 4,
		Academic = 5,
		Kali = 6,
	};

	enum eDetailEventType
	{
		EVENT_NONE = 0, //0  
		EVENT1 = 1, //1. EXP on monster death (party)			
		EVENT2 = 2, //2. EXP on monster death						
		EVENT3 = 3, //3. Stage clear EXP			
		EVENT4 = 4, //4. Increase union point gain
		EVENT5 = 5, //5. Increase item drop rate
		EVENT6 = 6, //6. Grant fatigue
		EVENT7 = 7, //7. Increase enhancement success rate		
		EVENT8 = 8, //8. Secondary authentication
		EVENT9 = 9, //9. Party maintenance
		EVENT10 = 10, //10. Party with friends
	};	
};
#else //#endif //#if defined(PRE_ADD_WORLD_EVENT)
//Event Type
enum eEventType1
{
	_EVENT_1_NONE = 0,
	_EVENT_1_EXP = 1,
	_EVENT_1_FATIGUE = 2,
	_EVENT_1_DROPITEM = 3,
	_EVENT_1_QUEST = 4,
	_EVENT_1_PVPEVENTROOM = 5,
#if defined( PRE_ADD_REBIRTH_EVENT )
	_EVENT_1_REBIRTH = 6, // Resurrection limit removal event
#endif // 
};

enum eEventType2
{
	_EVENT_2_NONE = 0,
	_EVENT_2_MONSTERDIE = 1,	//exp
	_EVENT_2_CLEAR = 2,		//exp
	_EVENT_2_EXTEND_FATIGUE = 3, //Bonus fatigue for event
};

enum eEventType3
{
	_EVENT_3_NONE = 0,
	_EVENT_3_PARTY = 1,
	_EVENT_3_SECONDAUTH = 2,
	_EVENT_3_UPKEEPPARTY = 3,			//For keeping the party alive
	_EVENT_3_FRIENDPARTY = 4,			//For best friend(?)
	_EVENT_3_EKEYANDECARD = 5,			//For China ekey and ecard
	_EVENT_3_CLASSEXP = 6,				//Set differentiated bonus EXP per class
};
#endif //#if defined(PRE_ADD_WORLD_EVENT)

enum eRebirthType
{
	_REBIRTH_SELF,
	_REBIRTH_REBIRTHER,
	_REBIRTH_REBIRTHEE
};

enum eTaxType
{
	TAX_NONE,
	TAX_POST,			// Mail tax
	TAX_TRADEREGISTER,	// Market registration fee
	TAX_TRADESELLING,	// Market sales fee
	TAX_POSTDEFAULT,	// Base fee for sending mail (stamp cost per mail)
	TAX_DEAL,			// User-to-user trade tax
	TAX_RESERVE,			// [Unused]
	TAX_TRADEREGISTER_CASH,	// Market registration fee when using a cash item
	TAX_TRADEPETALSELLING,	// Market Petal sales fee
	TAX_WSTORAGE,			// Server storage fee
};

enum eMissionType
{
	MissionType_Normal,		// Normal mission
	MissionType_MetaShow,	// Meta achievement and visible to client
	MissionType_MetaHide,	// Meta achievement and hidden from client
	MissionType_Quest,		// Mission linked to a quest; hidden on client
	MissionType_EventPopUp, // Event popup (Uses Type value instead of a popup field in MissionTable)
	MissionType_HelpAlarm,	// Help alarm
	MissionType_TimeEvent,	// Time event mission
};

enum eRestraintType
{
	_RESTRAINTTYPE_NONE = 0,		//none
	_RESTRAINTTYPE_BLOCK,		//account block
	_RESTRAINTTYPE_CHAT,		//chat block
	_RESTRAINTTYPE_TRADE,		//trade blcok(include market)
};

enum eVoiceChannelType
{
	_VOICECHANNEL_TYPE_SPARTIAL,			//with 3d sound
	_VOICECHANNEL_TYPE_NONESPARTIAL,		//none 3d sound
};

enum eVoiceJoinType
{
	_VOICEJOINTYPE_NONE,
	_VOICEJOINTYPE_TALKONLY,
	_VOICEJOINTYPE_LISTENONLY,
	_VOICEJOINTYPE_TALK_N_LISTEN,
};

enum eBroadcastingEffect
{
	EffectType_None = 0,
	EffectType_Enchant = 1,
	EffectType_Compound = 2,
	EffectType_Emblem = 3,
	EffectType_Random = 4,
#ifdef PRE_ADD_SECONDARY_SKILL
	EffectType_Plant = 5,
	EffectType_Water = 6,
	EffectType_Harvest = 7,
	EffectType_Cooking_Making = 8,
	EffectType_Cooking_Finish = 9,
	EffectType_Fishing = 10,
#endif // PRE_ADD_SECONDARY_SKILL

	EffectState_None = 0,
	EffectState_Start = 1,		// Attempt
	EffectState_Success = 2,	// Success
	EffectState_Fail = 3,		// Enhancement destroyed, crafting failed (changed due to resource name)
	EffectState_SoftFail = 4,	// Enhancement partial failure (level downgrade)
	EffectState_Open = 5,		// Attempt to open random item
	EffectState_VeryGood = 6,	// Random item 
	EffectState_Good = 7,		// Random item
	EffectState_Normal = 8,		// Random item
	EffectState_Bad = 9,		// Random item
	EffectState_Cancel = 10,	// Cancel
};

enum eChatRoomType		// Chat room type
{
	CHATROOMTYPE_NONE = 0,			// Initial value (meaningless)
	CHATROOMTYPE_NORMAL = 1,		// Normal
	CHATROOMTYPE_TRADE = 2,			// Trade
	CHATROOMTYPE_PR = 3,			// PR
};

enum eChatRoomAllow		// Enterable type
{
	CHATROOMALLOW_ALL,				// All 
	CHATROOMALLOW_FRIEND,			// Friend
	CHATROOMALLOW_GUILD,			// Guild
	CHATROOMALLOW_FRIENDANDGUILD,	// Friends & Guild
};

enum eChatRoomLeave
{
	CHATROOMLEAVE_NONE			= 0,	// Meaningless initial value
	CHATROOMLEAVE_KICKED,
	CHATROOMLEAVE_LEAVE,
	CHATROOMLEAVE_DESTROYROOM,
	CHATROOMLEAVE_FOREVERKICKED,
};

typedef INT64	MONEY_TYPE;
typedef int		TAX_TYPE;

typedef int		ITEMCLSID;
typedef INT64	ITEMSERIALID;

typedef INT64	CASH_TYPE;
typedef int		CASHITEM_SN;

#define ITEMCLSID_NONE	0


inline int GetCashRemainTime(INT64 biSerial)
{
	return (int)(biSerial & 0x00000000ffffffff);
}

enum eItemReversion
{
	ITEMREVERSION_NONE = 0,
	ITEMREVERSION_BELONG = 1,	//Bound
	ITEMREVERSION_TRADE = 2,
	ITEMREVERSION_GUILD = 3,	//Guild-bound item
	//ITEMREVERSION_GAIN = 2, // Unified system to bound or unbound; commented by kalliste 100113
};

//ServiceManager
enum eServiceState
{
	_SERVICE_STATE_OFFLINE = 0,
	_SERVICE_STATE_CREATE,
	_SERVICE_STATE_ONLINE,
	_SERVICE_STATE_TERMINATED,
};

enum eServiceStartType
{
	_SERVICE_START_ALL,
	_SERVICE_START_EACH,
	_SERVICE_START_EACH_BY_LAUNCHERID,
};

enum eServiceStopType
{
	_SERVICE_STOP_ALL,
	_SERVICE_STOP_ALL_FORCE,
	_SERVICE_STOP_EACH,
	_SERVICE_STOP_EACH_FORCE,
	_SERVICE_STOP_EACH_BY_LAUNCHERID,
	_SERVICE_STOP_EACH_FORCE_BY_LAUNCHERID,
};

enum eBanFileType
{
	_BANTYPE_ACCOUNTNAME,
	_BANTYPE_ACCOUNTID,
	_BANTYPE_CHARACTERNAME,
	_BANTYPE_MAX,
};

const int SMIDLENMAX	= 16;		// Service Manager / Monitor auth ID max length
const int SMPASSLENMAX	= 16;		// Service Manager / Monitor auth password max length

#define	MAKE_SM_SVRCHN(pSVRID, pCHNID)	MAKELONGLONG(pCHNID, pSVRID)	// Combine server/channel ID used as key for managing channel info
#define	GET_SM_SVR(pSVRCHN)				HILONG(pSVRCHN)					// Extract server ID from key when managing channel info
#define	GET_SM_CHN(pSVRCHN)				LOLONG(pSVRCHN)					// Extract channel ID from key when managing channel info

const int WAITPROCESSMAX = 500;			//Queue max processes

enum ePromotionType
{
	PROMOTIONTYPE_NONE = 0,
	PROMOTIONTYPE_MONSTERKILL,
	PROMOTIONTYPE_STAGECLEAR,
	PROMOTIONTYPE_DECREASEFATIGUE,
	PROMOTIONTYPE_MAX,
};

enum ePromotionConditionType
{
	PROMOTION_CONDTIONTYPE_NONE = 0,
	PROMOTION_CONDTIONTYPE_MAXLEVEL,
};

enum eNpcTalkString 
{
	NPCTALK_START = 0,			// Please always keep NPCTALK_START as 0.
	NPCTALK_COMPLETECUTSCENE,
	NPCTALK_END,
};

static std::wstring NpcTalkStringGroup[NPCTALK_END]= 
{
	L"start",					// Please always keep string 0 as "start".
	L"completecutscene",
};

#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
enum eEnchantGuardType
{
	ENCHANT_ITEM_NONE = 0,
	ENCHANT_ITEM_GUARD,				// Item Protection Jelly (no destroy)
	ENCHANT_ITEM_SHIELD,			// Superior enhancement protection jelly (no destroy, no downgrade)	
};
#endif

namespace Partner
{
	struct Type
	{
		enum eType
		{
			None = 0,
			MS,
			Max,
		};
	};
}

const int GUILDWAR_FINALS_TEAM_MAX = 16;	// 16 teams based on Round of 16
const int GUILDWAR_DAILY_AWARD_MAX = 7;		// Daily reward history
const int REQ_GUILDWARINFO_TICK_MAX=30*1000; // Response retry request Tick
const int GUILDWAR_DB_JOB_SYSTEM_RESULT_REQ_MAX=60*1000;		// Fetch DBJob results every 1 minute after the registration period and main round start
const int GUILDWAR_TOURNAMENT_GROUP_MAX = GUILDWAR_FINALS_TEAM_MAX/2; // Tournament group
const int GUILDWAR_TRIAL_POINT_TEAM_MAX = 24;	// 24 teams to display preliminaries scores.

#if defined(_FINAL_BUILD)
const int GUILDWAR_FINAL_ROOMSTRAT_TICK_MAX=5*60*1000;			// Time until room starts after room creation (5 minutes)
const int GUILDWAR_PRE_WIN_SKILL_COOLTIME=60*2*1000;			// Guild war winner skill cooldown 2 min
const int GUILDWAR_TRIAL_POINT_REFRESH_TICK_MAX=10*60*1000;		// Fetch once every 10 minutes
const int GUILDWAR_SECRET_TERM_SECOND=24*60*60;					// Secret activation and deactivation time (seconds)
#else
const int GUILDWAR_FINAL_ROOMSTRAT_TICK_MAX=60*1000;			// Time until room starts after room creation.
const int GUILDWAR_PRE_WIN_SKILL_COOLTIME=30*1000;				// Guild War winner skill cooldown
const int GUILDWAR_TRIAL_POINT_REFRESH_TICK_MAX=60*1000;		// Fetch once per minute
const int GUILDWAR_SECRET_TERM_SECOND=5*60;						// Secret activation and deactivation time (seconds)
#endif // #if defined(_FINAL_BUILD)

const float GUILDWAR_SECRET_SATRT_RATE = 0.1f;	// Secret activation
const float GUILDWAR_SECRET_END_RATE = 0.02f;	// Secret deactivation

struct sGroup
{
	int nTeamNum;			// Since it is a Number, subtract 1 to get the index
	int nOpeningPoint;		// Preliminaries score
	int nTournamentPoint;	// Interim score
};

struct sTournamentGroup
{
	sGroup ATeam;
	sGroup BTeam;
	UINT  unPvPIndex;
	DWORD dwStartTick;
};

enum eGuildWarStepType
{
	GUILDWAR_STEP_NONE = 0,
	GUILDWAR_STEP_PREPARATION,	// Application period
	GUILDWAR_STEP_TRIAL,		// Preliminary period
	GUILDWAR_STEP_REWARD,		// Reward period (including main tournament)
	GUILDWAR_STEP_END,
};

enum eGuildWarEventType
{
	GUILDWAR_EVENT_START = 0,	// Start
	GUILDWAR_EVENT_END = 1,		// End
	GUILDWAR_EVENT_MAX = 2,		
};

//Round: 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
enum eGuildFinalPart
{	
	GUILDWAR_FINALPART_NONE =0,
	GUILDWAR_FINALPART_FINAL,
	GUILDWAR_FINALPART_4,
	GUILDWAR_FINALPART_8,
	GUILDWAR_FINALPART_16,	
	GUILDWAR_FINALPART_MAX,
};

enum eGuildWarRankingType
{
	GUILDWAR_RANKINGTYPE_TOTAL = 0,	// Target
	GUILDWAR_RANKINGTYPE_SUPPRESSION,	// Subjugation
	GUILDWAR_RANKINGTYPE_CAPTURE,		// Capture
	GUILDWAR_RANKINGTYPE_HARVEST,		// Capture
	GUILDWAR_RANKINGTYPE_COOKING,		// Cooking
	GUILDWAR_RANKINGTYPE_PVP,			// Combat
	GUILDWAR_RANKINGTYPE_MAX,
};

enum eGuildWarSettingStep
{
	REQ_NONE,				// No state
	REQ_SCHEDULE,			// Full schedule request step	
	REQ_FINAL_SCHEDULE,		// Finals schedule request stage	
	REQ_PRE_WIN,			// Request winning guild of the previous round
	REQ_TEAM_POINT,			// Request blue team and red team scores	
	REQ_ALL_COMPLETE,		// All information setup complete
};

enum eGuildWarTeamType
{
	GUILDWAR_TEAM_BLUE = 1,	// Blue team
	GUILDWAR_TEAM_RED,		// Red team
};

enum eGuildWarRewardForCharacter
{
	GUILDWAR_REWARD_CHAR_PRESENT = 0,   // Preliminaries gift
	GUILDWAR_REWARD_CHAR_MAIL,			// Preliminaries mail
	GUILDWAR_REWARD_CHAR_POINT,			// Preliminaries festival points
	GUILDWAR_REWARD_CHAR_MAX,		
};

enum eGuildWarRewardForGuild
{
	GUILDWAR_REWARD_GUILD_SKILL = 0,	// Finals skill?
	GUILDWAR_REWARD_GUILD_PRESENT,		// Finals gift (Queue)
	GUILDWAR_REWARD_GUILD_MAIL,			// Finals mail (Queue)
	GUILDWAR_REWARD_GUILD_TRIAL_POINT,	// Preliminaries guild points 
	GUILDWAR_REWARD_GUILD_FINAL_POINT,	// Finals guild points
	GUILDWAR_REWARD_GUILD_MAX,
};

enum eGuildPointType					// Guild point type
{
	GUILDPOINTTYPE_STAGE = 1,			// Stage 
	GUILDPOINTTYPE_MISSION = 2,			// Mission
	GUILDPOINTTYPE_WAR = 3,				// Guild war
	GUILDPOINTTYPE_MAX = 4,
};

struct TAppellationTime
{
	int nArrayID;									//Title
	__time64_t tExpireDate;							//Duration
};

struct TCashGlyphData
{
	BYTE cActiveGlyph;				// Heraldry slot activation state
	__time64_t	tGlyphExpireDate;	// Heraldry slot duration
};

#if defined( PRE_ADD_SHORTCUT_HELP_DIALOG )
enum eShortCutHelpDlgType
{
	HELP_SHORTCUT_CONTROL_KEY = 1,
	HELP_SHORTCUT_CHAR_SET_PLATE,
	HELP_SHORTCUT_CREATE_PLATE,
	HELP_SHORTCUT_DESTROY_PLATE,
	HELP_SHORTCUT_SET_APPELLATION,
	HELP_SHORTCUT_SKILL,
	HELP_SHORTCUT_POTENTIAL,
	HELP_SHORTCUT_ITEM_UPGRADE,
	HELP_SHORTCUT_ITEM_CREATE,
	HELP_SHORTCUT_ITEM_DISJOINT,
	HELP_SHORTCUT_PARTY_PLAY,
	HELP_SHORTCUT_FRIEND,
	HELP_SHORTCUT_PUPILLIST,
	HELP_SHORTCUT_GUILD,
	HELP_SHORTCUT_QUEST,
	HELP_SHORTCUT_STAGE,
	HELP_SHORTCUT_MARKET,
	HELP_SHORTCUT_MAIL,
	HELP_SHORTCUT_VEHICLE,
	HELP_SHORTCUT_PET,
	HELP_SHORTCUT_REPUTATION,
	HELP_SHORTCUT_FARM,
	HELP_SHORTCUT_FISHING,
	HELP_SHORTCUT_COOK,
	HELP_SHORTCUT_MASTER,
	HELP_SHORTCUT_PVP,
	HELP_SHORTCUT_PRIVATE_CHAT,
};
#endif // PRE_ADD_SHORTCUT_HELP_DIALOG

#if defined( PRE_ADD_INSTANT_CASH_BUY )
enum eInstantCashShopBuyType
{
	INSTANT_BUY_NONE = -1,
	INSTANT_BUY_SEAL = 0,
	INSTANT_BUY_ITEMUPGRADE,
	INSTANT_BUY_DEATH,
	INSTANT_BUY_INVEN,
	INSTANT_BUY_STORAGE,
	INSTANT_BUY_POTENTRANS,
	INSTANT_BUY_PET,
	INSTANT_BUY_TRADE,
	INSTANT_BUY_SKILL,
	INSTANT_BUY_FARM,
	INSTANT_BUY_AUTOUNPACK,
	INSTANT_BUY_PLATE,
	INSTANT_BUY_PETEXTEND,
};

enum eInstantShowType
{
	INSTANT_BUY_HIDE = 0,
	INSTANT_BUY_SHOW,
	INSTANT_BUY_DISABLE
};

const int InstantCashShopTypeMin = 50;

#endif // PRE_ADD_INSTANT_CASH_BUY

enum eDragonNestType
{
	None = 0,
	SeaDragon = 1,
	GreenDragon = 2,
	DesertDragon = 3,
	Max,
};

enum eTrace
{
	TRACE_CHARACTERNAME = 0,
	TRACE_ACCOUNTDBID,
	TRACE_CHRACTERDBID,
};

enum eWindowState
{
	WINDOW_NONE					= 0,
	WINDOW_EXCHANGE				= 1, 		// Trade	
	WINDOW_CASHSHOP				= (1<<1),	// Cash shop
	WINDOW_COSMIX				= (1<<2),	// Costume synthesis
	WINDOW_DESIGNMIX			= (1<<3),	// Costume design synthesis
	WINDOW_BLIND				= (1<<4),	// Blind state (NPC talk, normal enhance, item combine, mail, shop, unmanned shop)
	WINDOW_PROGRESS				= (1<<5),	// Progress bar state (random pouch, pet, etc.)
	WINDOW_ISACCEPT				= (1<<6),	// Accept dialog is open..
	WINDOW_RANDOMMIX			= (1<<7),	// Costume random synthesis
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	WINDOW_ALTEIAWORLD			= (1<<8),	// Altea Continent Exploration window open state
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )
};

struct TSourceData
{
	int nItemID;
	int nRemainTime;
};

namespace PartsMonster
{
	struct Common
	{
		enum UI // Max number of parts displayed in UI only; unrelated to the monster's actual part count.
		{
			Max_PartsSize = 4,
		};
	};
}

enum ePartyModifyResult
{
	_PARTY_MOD_RET_NONE = 0,
	_PARTY_MOD_RET_ADD,
	_PARTY_MOD_RET_DEL,
};

namespace CommonVariable
{
	struct Type
	{
		enum eCode
		{
			AddFatigue					= 1,	// Add fatigue
			LastPartyDungeonInfo		= 2,	// Previous dungeon server info for party state
			AttendanceEvent				= 3,	// Attendance event
			PvPOccupation_PlayCount 	= 4,	// Occupation war play count
			PvPOccupation_WinCount		= 5,	// Occupation war win count
			PvPOccupation_KillCount 	= 6,	// Occupation war kill count
			PvPOccupation_OccupyArea	= 7,	// Occupation war base capture
			PvPOccupation_StealArea		= 8,	// Occupation war base contest
			PvPAllKill_PlayCount		= 9,	// All-kill mode play count
			PvPAllKill_WinCount			= 10,	// All-kill mode win count
			PvPAllKill_KillCount		= 11,	// All-kill mode kill count
			MOVESERVER_PARTYID = 12,		// PartyID to temporarily store during server transfer
			BESTFRIEND_GIFTBOXID = 13,		// Best friend gift box ID
			EXPUPITEM_COUNT = 14,			// Absolute value EXP increase item
			LevelupEventNew = 15,			// Level-up event gift (check that it cannot be claimed again once received)
			LimitedShopWeek = 16,			// Reset weekly purchase-limited shop
			NotifyCompound = 17,
			TotalLevelSkillMedal = 18,		// Daily limit on unified skill level medals acquired
			SampleDataVersion = 19,
			SampleItem1 = 20,
			SampleItem2 = 21,
			SampleItem3 = 22,
			SampleItem4 = 23,
			SampleItem5 = 24,
			SampleItem6 = 25,
			SampleItem7 = 26,
			SampleItem8 = 27,
			SampleItem9 = 28,
			SampleItem10 = 29,
			MuteChatting = 30,				// Chat banned
			PvPExpupItem_UseCount = 31,		// Colosseum EXP boost item usage count
			Max,
		};
	};

	struct Data
	{
		Type::eCode Type;
		INT64 biValue;
		__time64_t tLastModifyDate;
	};
}

namespace GhoulMode
{
	struct PointType
	{
		enum eCode
		{
			PlayCount = 0,			// Play count
			GhoulWin,				// Ghoul victory
			HumanWin,				// Human Victory
			TimeOver,				// Human victory (Time Over)
			GhoulKill,				// Ghoul Kill
			HumanKill,				// Human Kill
			HolyWaterUse,			// Use holy water
			Max,
		};
	};
}

namespace BreakInto
{
	struct Type
	{
		enum eCode
		{
			None			= 0,
			GMTrace			= 1,	// GM tracking
			WorldZoneParty	= 2,	// World zone invite
			PartyRestore	= 3,	// Restore party(?)
#if defined( PRE_PARTY_DB )
			PartyJoin		= 4,	// Join party
#endif // #if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PVP )
			WorldPvPRoom	= 5,
#endif
		};
	};
}

namespace AppellationColor
{
	struct Type
	{
		enum eCode
		{
			White			= 0,
			Purple			= 1,
			Orange			= 2,	//Timed cash title
			Green			= 3,	//PC Bang exclusive title
		};
	};
}

namespace AppellationType
{
	struct Type
	{
		enum eCode
		{
			Normal	= 0,
			Cash	= 1,
			Period	= 2,
			PCBang	= 3,
			Guild	= 4,
			BestFriend = 5,
			ComeBack = 6,	//Returnee title
#ifdef PRE_ADD_GM_APPELLATION
			GM = 7,
#endif // PRE_ADD_GM_APPELLATION

#ifdef PRE_ADD_GM_APPELLAThongse
			hong = 88,
#endif // PRE_ADD_GM_APPELLATION

#ifdef PRE_ADD_GM_APPELLAchengse
			jin = 99,
#endif // PRE_ADD_GM_APPELLATION
		};
	};	
}

namespace IncreaseLifeType
{
	struct Type
	{
		enum eCode
		{
			LifeStone	= 0,		//Life Stone
			RebirthStone	= 1,		//Resurrection stone			
		};
	};
}

namespace Ghost
{
	struct Type
	{
		enum eCode
		{
			Normal			= 0,
			PartyRestore	= 1,
		};
	};	
}

namespace Shop
{
	struct Type
	{
		enum eCode
		{
			Normal = 0,			// Normal shop
			LadderPoint,		// Ladder point shop
			GuildPoint,			// Guild war point shop
			ReputePoint,		// Affinity point shop
			
			Combined = 100,		// Normal integrated(?) shop
			CombinedLadderPoint,// Ladder point unified(?) shop
			CombinedGuildPoint,	// Guild war point unified (?) shop
			CombinedReputePoint,// Unified(?) affinity point shop
			CombinedSeedPoint,	// Seed Shop
		};
	};

	struct Max
	{
		enum eCode
		{
			PurchaseType = 2,
		};
	};

	struct PurchaseType
	{
		enum eCode
		{
			None = 0,
			Gold,				// Gold
			ItemID,				// ItemID
			LadderPoint,		// Ladder points
			UnionPoint,			// Union points
			GuildPoint,			// Guild points
			Sample,				// Sample item
			Seed,				// Bound currency (Seed)
		};
	};

	struct PurchaseLimitType
	{
		enum eCode
		{
			None = 0,
			JobID,				// JobID
			Level,				// Level
			PvPRank,			// PvP rank
			GuildLevel,			// Guild level
			LadderGradePoint,	// Ladder rating (passing any one of 1vs1, 2vs2, etc. is sufficient)
		};
	};

	struct Repurchase
	{
		enum eCode
		{
			MaxList = 30,
		};
	};
}

namespace DualSkill
{
	struct Type
	{
		enum eSkillPage
		{
			None = -1,
			Primary = 0,
			Secondary = 1,
			MAX,
		};
	};
}

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
namespace ReservationSKillList
{
	struct Type
	{
		enum eSkillReservation
		{
			UnLock = 1,		// First acquisition.. deducts Money
			Acquire = 2,	// Learn after reset
			LevelUp = 3,	// Skill level up.
		};		
	};
	const int ReservationMax = 40;
}
#endif

namespace Item
{
	struct SkillUsingType
	{
		enum eCode 
		{
			None,
			ApplyStateBlow,
			ApplySkill,
			Eternity,				// Permanent effect item
			SkillLevelUp,			// Skill level-up item
			PrefixSkill,			// Item for prefix skill
		};
	};
}

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
namespace SimpleConfig
{
	struct Type
	{
		enum eType
		{
			None = 0,
			ConnectIPCountMax,
		};
	};

	struct DefaultValue
	{
		enum Value
		{
			None = 0,
			ConnectIPCountMax = 100,
		};
	};

	struct ConfigData
	{
		char cCode;
		int nValue;
	};
	const int ConfigDataCountMax = 100;
};
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

namespace PlayerCustomEventUI
{
	const int PlayerCustomEventUIMax = 100;
};

namespace GuildRecruitSystem
{
	struct Max
	{
		enum eType
		{
			MaxRequestCount			= 3,		// Guild recruitment board max application count
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
			GuildRecruitRowCount	= 5,		// Guild recruitment board row count
#else
			GuildRecruitRowCount	= 6,		// Guild recruitment board row count
#endif
			GuildRecruitCharacterRowCount = 50,	// Row count of users who applied to join guild via recruitment board
		};
	};
	struct RegisterType
	{
		enum eType
		{
			RegisterOn,		// Register
			RegisterMod,	// Modify
			RegisterOff,	// Delete
		};
	};
	struct RequestType
	{
		enum eType
		{
			RequestOn,		// Register
			RequestOff,		// Release / Unbind
		};
	};
	struct AcceptType
	{
		enum eType
		{
			AcceptOn,		// Approve join request
			AcceptOff,		// Reject join request
		};
	};
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	struct PuposeCodeType	//	Guild purpose
	{
		enum eType
		{
			AllCode,	    //	All
			FriendlyCode,	//	Social
			WarCode,		//	Combat
			NestCode,		//	Nest
			PvPCode,		//	Colosseum
		};
	};
	struct SortType			//	Sort method
	{
		enum eType
		{
			GuildLevelDesc = 1,	//	Guild level descending
			GuildLevelAsc,	//	Guild level ascending
			GuildMemberDesc,//	Current member count descending
			GuildMemberAsc, //  Current member count ascending
			GuildNameDesc,	//  Guild name descending
			GuildNameAsc,	//  Guild name ascending
		};
	};
#endif
};

namespace AsiaAuth
{
	struct Domain
	{
		enum eDomain
		{
			ThailandPlayParkID,
			ThailandAsiaSoftID,
			ThailandTCGID,
			ThailandDomainMax,
		};		
	};

	static std::string strDomain[Domain::ThailandDomainMax] = 
	{
		"THPP",
		"THAS",
		"THTC",
	};

	struct Common
	{
		enum eCommon
		{
			OTPRetCommond = 1002,
			OTPLenMax = 6,
			OTPTimeOut = 1000 * 10,
		};
	};

	struct Auth
	{
		enum eAuth
		{
			AuthReturnCode = 0,
			AuthDescription,
			AuthMaster_ID,
			AuthAkeyFlag,
			AuthMaster_Akey_ID,
			AuthSessionID,
			AuthFormatCountMax,
		};
	};

	struct OTP
	{
		enum eOTP
		{
			OTPCommand = 0,
			OTPLength,
			OTPStatus,
			OTPMessage,
			OTPEndPoint,
			OTPFormatCountMax,
		};
	};

	struct AuthResultCode
	{
		enum eAuthResultCode
		{
			None = -1234567,
			Success = 0,
			Invalid_Request = -1001,
			Invalid_UserID = -1002,
			User_Not_Found = -1003,
			Worng_Password = -1004,
			Disable_User = -1005,
			Database_Error = -9009,
			Unexpected_Error = -9010,
		};
	};

	struct TAsiaSoftAuthInfo
	{
		int nAuthReturnCode;		//Ref: AsiaAuth::AuthResultCode
		std::string strDescription;
		std::string strDomain;
		std::string strMasterID;
		std::string strAkeyMasterID;		//with itp
		UINT nSessionID;
		bool bAkeyOTPFlag;					//need otp

		TAsiaSoftAuthInfo()
		{
			nAuthReturnCode = AsiaAuth::AuthResultCode::None;
			nSessionID = 0;
			bAkeyOTPFlag = false;
		}
	};
};

namespace WarpVillage
{
	struct WarpVillageInfo
	{
		int nMapIndex;
		BYTE btLimitLevel;

		WarpVillageInfo() {}
		WarpVillageInfo(int nIndex, BYTE btLevel)
			: nMapIndex(nIndex), btLimitLevel(btLevel)
		{

		}
	};
};

namespace NestClear
{
	struct Type
	{
		enum eNestClearType
		{
			Normal = 1,		// Normal user
			PCBang = 2,		// When clearing in a PC cafe
		};
	};
};

namespace Gesture
{
	struct Type
	{
		enum eGestureType
		{
			Normal = 0,
			GuildWarWin = 1,
			GuildRewardItem = 2,
		};
	};
}

#if defined (PRE_ADD_DONATION)
namespace Donation
{
	const BYTE MAX_RANKING = 10;

	struct Ranking
	{
		INT64 biCharacterID;
		WCHAR wszCharacterName[NAMELENMAX];
		BYTE btJobCode;
		WCHAR wszGuildName[GUILDNAME_MAX];
		INT64 nCoin;
	};
};
#endif // #if defined (PRE_ADD_DONATION)

namespace Dungeon
{
	struct Difficulty
	{
		enum eCode
		{
			Easy = 0,
			Normal = 1,
			Hard = 2,
			Master = 3,
			Abyss = 4,
			Chaos = 5,
			Hell = 6,
			Max,
		};
	};
}

#if defined( PRE_PARTY_DB )

typedef INT64 TPARTYID;

namespace Party
{
	struct Interval
	{
		enum eCode
		{
			GetListPartyTick = (3*1000),
		};
	};

	struct Max
	{
		enum eCode
		{
			SearchWordLen = (20+1),	// Max length of search query
			WordSpacing= 5,			// Max spaces in search query
			SearchWordHistory = 5,	// Max search query history count
			DetailSearchMapCount = 1, // Max number of maps for detailed search
			WorldCombinePartyList = 2, // Max cross-world parties
			WorldCombinePartyBonus = 2 // Max cross-world party bonuses
		};
	};

	struct LocationType
	{
		enum eCode
		{
			Village = 0,
			Worldmap,
		};
	};

	struct Constants
	{
		enum eCode
		{
			INVALID_PASSWORD = 0,
			PARTY_LIST_REFRESH_SEC_CLIENT = 5,
			MIN_SEARCH_WORD = 2,
			INVALID_WORLDCOMBINE_TABLE_INDEX = -1,
			INVALID_SERVERID = 0,
#ifdef PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP
			INCORRECT_PASSWORD = 10,
#endif 
		};
	};

	struct OrderType
	{
		enum eCode
		{
			LevelDesc = 0,				// Entry level descending
			LevelAsc,					// Entry level ascending
			MemberCountDesc,			// Member count descending
			MemberCountAsc,				// Member count ascending
			TargetMapDesc,				// Target stage descending
			TargetMapAsc,				// Target stage ascending
			TargetMapDifficultyDesc,	// Target stage difficulty descending
			TargetMapDifficultyAsc,		// Target stage difficulty ascending
			BonusDesc,					// Bonus descending
			BonusAsc,					// Bonus ascending
			Max,
			Default = LevelDesc,
		};
	};

	struct StageOrderType
	{
		enum eCode
		{
			TargetMapAllDesc = 0,		// All maps descending
			TargetMapNestDesc,			// Nest map descending order
			TargetMapStageDesc,			// Stage descending order
			TargetMapEtcDesc,			// Other descending
			Max,
			Default = TargetMapAllDesc,
		};
	};

	struct KickKind
	{
		enum eCode
		{
			NormalKick = 0,
			PermanentKick = 1,
		};
	};

	struct Data
	{
		TPARTYID PartyID;
		int iServerID;
		Party::LocationType::eCode LocationType;
		int iLocationID;	// Town: ChannelID, WorldZone/Dungeon: MapID
		ePartyType Type;
		WCHAR wszPartyName[PARTYNAMELENMAX];
		short nPartyMaxCount;
		int iTargetMapIndex;
		Dungeon::Difficulty::eCode TargetMapDifficulty;
		int iBitFlag;
		int iPassword;
		BYTE cMinLevel;
		ePartyItemLootRule LootRule;
		eItemRank LootItemRank;
		INT64 biLeaderCharacterDBID;
		int iCurMemberCount;
		int iUpkeepCount;
		int iRoomID;
#if defined( PRE_WORLDCOMBINE_PARTY )
		int nWorldSetID;
		int nPrimaryIndex;	// Cross-world party reference value (target map)	
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
		bool bCheckComeBackParty;
#endif
#ifdef PRE_ADD_BEGINNERGUILD
		bool bPartyBeginnerGuild;		//Whether party members are all beginner guild members
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

		void Clear()
		{
			// Zero initial values
			PartyID = 0;
			iServerID = 0;
			iLocationID = 0;
			memset( wszPartyName, 0, sizeof(wszPartyName) );
			iTargetMapIndex = 0;
			TargetMapDifficulty = Dungeon::Difficulty::Easy;
			iBitFlag = 0;
			cMinLevel = 0;
			LootRule = ITEMLOOTRULE_NONE;
			LootItemRank = ITEMRANK_D;
			biLeaderCharacterDBID = 0;
			iCurMemberCount = 0;
			iUpkeepCount = 0;
			iRoomID = 0;
			
			// Other
			iPassword = Constants::INVALID_PASSWORD;
			Type = _NORMAL_PARTY;
			LocationType = LocationType::Village;
			nPartyMaxCount = PARTYMAX;
#if defined( PRE_WORLDCOMBINE_PARTY )
			nWorldSetID = 0;
			nPrimaryIndex = 0;
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
			bCheckComeBackParty = false;
#endif
#ifdef PRE_ADD_BEGINNERGUILD
			bPartyBeginnerGuild = false;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
		}
	};

	struct MemberData
	{
		INT64 biCharacterDBID;
		WCHAR wszCharName[NAMELENMAX];
		BYTE cLevel;
		BYTE cJob;
		bool bLeader;
#if defined( PRE_WORLDCOMBINE_PARTY )
		int nWorldSetID;
#endif
		bool bCheckComeback;
	};

	struct BitFlag
	{
		enum eCode
		{
			JobDice = 1,			// Class dice priority
			Password = (1<<1),		// Password-protected party
			WorldmapAllow = (1<<2),	// Allow world zone party
			VoiceChat = (1<<3),		// Voice chat
		};
	};

	struct SortWeightIndex
	{
		enum eCode
		{
			SearchWordTitle = 10001,
			SearchWordStage = 10002,
			SearchWordDifficulty = 10003,
			
			PublicParty = 20001,
			PrivateParty = 20002,

			DifficultyAbyss = 30001,
			DifficultyMaster = 30002,
			DifficultyHard = 30003,
			DifficultyNormal = 30004,
			DifficultyEasy = 30005,
			DifficultyNone = 30006,

			DifficultyAbyssOrderbyAsc = 31001,
			DifficultyMasterOrderbyAsc = 31002,
			DifficultyHardOrderbyAsc = 31003,
			DifficultyNormalOrderbyAsc = 31004,
			DifficultyEasyOrderbyAsc = 31005,
			DifficultyNoneOrderbyAsc = 31006,

			DifficultyAbyssOrderbyDesc = 32001,
			DifficultyMasterOrderbyDesc = 32002,
			DifficultyHardOrderbyDesc = 32003,
			DifficultyNormalOrderbyDesc = 32004,
			DifficultyEasyOrderbyDesc = 32005,
			DifficultyNoneOrderbyDesc = 32006,

			LevelLimit = 40001,

			LevelLimitOrderbyAsc = 41001,
			LevelLimitOrderbyDesc = 42001,

			SameVillage = 50001,
			ManaRidge = 50002,
			PrairieTown = 50003,
			CataractGateWay = 50004,
			SaintHaven = 50005,
			LotusMash = 50006,

			PartyMemberCount = 60001,

			PartyMemberCountOrderbyAsc = 61001,
			PartyMemberCountOrderbyDesc = 62001,

			TargetStage = 70001,

			TargetStageAsc = 71001,

			TargetStageDesc = 72001,

			NestToggle = 100001,
			StageToggle = 100002,
			EtcToggle = 100003,
		};
	};

#if defined( PRE_WORLDCOMBINE_PARTY )
	static bool bIsWorldCombineParty( ePartyType Type )
	{
		switch( Type )
		{
			case _WORLDCOMBINE_PARTY:
				return true;
		}

		return false;
	}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
};

typedef Dungeon::Difficulty::eCode TDUNGEONDIFFICULTY;
typedef ePartyItemLootRule TPARTYITEMLOOTRULE;
typedef eItemRank TITEMRANK;
typedef INT64 TINSTANCEID;	// Used for various purposes such as PartyID, AccountID, RoomID, etc.

#else

typedef UINT TPARTYID;
typedef char TDUNGEONDIFFICULTY;
typedef BYTE TPARTYITEMLOOTRULE;
typedef BYTE TITEMRANK;
typedef UINT TINSTANCEID;	// Used for various purposes such as PartyID, AccountID, RoomID, etc.

#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_ADD_REVENGE )
namespace Revenge
{
	struct RevengeTarget
	{
		enum eCode
		{
			eRevengeTarget_None = 0,
			eRevengeTarget_Me,
			eRevengeTarget_Target,
		};		
	};

	struct TargetReason
	{
		enum eCode
		{
			SetNewRevengeTarget = 0,	//Register new revenge target
			SuccessRevenge = 1,			//Revenge succeeded
			TargetLeaveGame = 2,		//Revenge target left the game
			TargetLeaveRoom = 3,		//Revenge target left the PvP room			
			NotPartedInPlay = 4,		//Target is not participating in the game (spectator)
			ResetRevengeTarget = 10,	//Reset because self left the game/room or is a spectator
		};

	};
	
	enum
	{
		RevengeRewardMedalCount = 1,	//Reward medal count on successful revenge
		RevengeConditionCount = 3,		//Kill count required to be registered for revenge
	};
};
#endif	// #if define( PRE_ADD_REVENGE )

#define CONVERT_TO_DUNGEONDIFFICULTY(n) static_cast<TDUNGEONDIFFICULTY>(n)
#define CONVERT_TO_PARTYITEMLOOTRULE(n) static_cast<TPARTYITEMLOOTRULE>(n)
#define CONVERT_TO_ITEMRANK(n) static_cast<TITEMRANK>(n)

namespace MultiLanguage
{
	struct SupportLanguage
	{
		enum eSupportLanguage
		{
			Kor = 0,		//0 is default
			NationMax,
		};
	};

	static std::string NationString[SupportLanguage::NationMax] = 
	{
		//Match the count of eSupportNation - used to build suffix string when loading
		"_KOR",
	};

	struct Common
	{
		enum eCommon
		{
			SerachCountMax = 10,
		};
	};

	static SupportLanguage::eSupportLanguage eDefaultLanguage = static_cast<SupportLanguage::eSupportLanguage>(0);
};

#ifdef PRE_ADD_48714
namespace AbuseLog
{
	struct Common
	{
		enum eCommon
		{
			AbuseLog_Reached_MailID = 442,			//Does not change! Must not change~

		};
	};
};
#endif		//#ifdef PRE_ADD_48714

#ifdef PRE_ADD_49660
// Cleric Divine Punishment skill ID
const int DVINE_PUNISHMENT_SKILL_ID = 3205;
#endif // #ifdef PRE_ADD_49660
#if defined(PRE_ADD_DOORS_PROJECT)
namespace Doors
{
	struct CharacterAbility
	{
		enum eStateType
		{
			Strength =0,		// Strength
			Agility,			// Agility
			Intelligence,		// Intelligence
			Stamina,			// Health
			AttackPMin,			// Physical Attack Min
			AttackPMax,			// Physical Attack Max
			AttackMMin,			// Min magic attack power
			AttackMMax,			// Max magic attack power
			DefenseP,			// Physical Defense
			DefenseM,			// Magic defense
			Stiff,				// Stagger power
			StiffResistance,	// Stagger resistance
			Critical,			// Critical
			CriticalResistance, // Critical resistance
			Stun,				// Stun
			StunResistance,		// Stun resistance
			FinalDamage,		// Final damage
			FireAttack,			// Fire element attack power
			IceAttack,		 	// Water element attack
			LightAttack,		// Light element attack power
			DarkAttack,			// Dark element attack power
			FireDefense,		// Fire element attack power
			IceDefense,		 	// Water element attack
			LightDefense,		// Light element attack power
			DarkDefense,		// Dark element attack power
			MoveSpeed,			// Move speed
			MaxHP,				// Max HP
			MaxMP,				// Max MP	
			MaxType,			
		};
	};
}
#endif

namespace BeginnerGuild
{
	struct Type
	{
		enum eType
		{
			Normal = 0,
			Beginner = 1,
		};
	};

	struct StringIndex
	{
		enum eStringIndex
		{
			GuildTitle = 1594,			//Beginner guild title uistring Index
			GuildNotice = 1386,			//Beginner guild notice uistring Index
			GuildRegisterContents = 1598,
		};
	};

	struct Common
	{
		enum eCommon
		{
			PartyBonusRate = 5,			//Bonus EXP when partying with beginner guild members
		};
	};
}

namespace WillSendMail
{
	struct Common
	{
		enum eCommon
		{
			WIllMailPacketMax = 20,			//Will it overwrite?
			WillSendMail_Memo_Size = 20,	//DB is set to 30.
			WillSendMail_NoRecord_RetCode = 103326,
		};
	};

	struct TWillSendInfo
	{
		int nMailID;
		WCHAR wszMemo[WillSendMail::Common::WillSendMail_Memo_Size];
	};
};

namespace Doors
{
	struct Common
	{
		enum eCommon
		{
			AuthenticationKey_LenMax = 6,
		};
	};
};

#if defined( PRE_ADD_LIMITED_SHOP )
namespace LimitedShop
{
	struct LimitedItemData
	{
#if defined( PRE_FIX_74404 )
		int nShopID;
#endif // #if defined( PRE_FIX_74404 )
		int nItemID;
		int nBuyCount;
		int nResetCycle;
	};
	struct LimitedItemType
	{
		enum eCycle
		{
			Day = 1,
			Week = 2,
		};
	};	
}
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
namespace PrivateChatChannel
{
	struct Common
	{
		enum eCommon
		{
			WaitDBResult = -1,
			MinNameLen = 2,
			MaxNameLen = 11,
			MaxMemberCount = 100,
			GetDBMaxChannel = 100,
			MAXWORLDCOUNT = 5,
		};
		enum eModType
		{
			ChangePassWord = 1,
			ChangeMaster,
			ExitMember,
			OutMember,				
			KickMember,
			JoinMember,
			EnterMember,			
		};
		enum eReturnType
		{
			NeedPassWord = 1,
		};
	};
	struct TMemberInfo
	{
		bool bMaster;
		INT64 biCharacterDBID;
		WCHAR wszCharacterName[NAMELENMAX];
	};
}
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
namespace TotalLevelSkill
{
	struct Common
	{
		enum eCommon
		{
			MAXSLOTCOUNT = 4,
			MAX_STATE_EFFECT_COUNT = 7,
		};		
		enum eVillageEffectType
		{
			RepuTationIncrease = 260,	// Affinity increase
			GrowingBoost = 262,			// Reduce farm cultivation time
			LadderPointSale = 263,		// Ladder point discount
			CookExpIncrease = 265,		// Increase cooking proficiency
		};
	};
	struct TTotalLevelSkillData
	{
		int nSlotIndex;
		int nSkillID;
	};
}
#endif

namespace SpecialBox
{
	struct Common
	{
		enum eCommon
		{
			ListMax = 40,				// List max value
			RewardAllMax = 10,			// Maximum reward value
			RewardSelectMax = 50,		// Max selectable rewards
		};

		enum eRequestDelay
		{
			DelayTime = 5,
		};
	};

	struct TargetTypeCode
	{
		enum eTargetTypeCode
		{
			AccountAll = 1,			// 1 = whole account
			AccountSelect = 2,		// 2=Specified account
			CharacterAll = 3,		// 3=All characters
			CharacterSelect = 4,	// 4=Specify character
			ConditionSelect = 5,	// 5=Specify condition
		};
	};

	struct ReceiveTypeCode
	{
		enum eReceiveTypeCode
		{
			All = 1,
			Select = 2,
		};
	};
};

#if defined( PRE_WORLDCOMBINE_PVP )

namespace WorldPvPMissionRoom
{
	struct Common
	{
		enum eCommon
		{
			MaxDBDataCount = 100,
			GetWorldPvPRoomTick = 30*1000,
			WaitStartWorldPvPRoomSecond = 10,
			WorldPvPRoomStartIndex = PvPCommon::Common::MaxRoomCount,			
			GMWorldPvPRoomStartIndex = PvPCommon::Common::MaxRoomCount + 100,
		};
		enum eReqType
		{
			NoneType = 0,
			MissionRoom,
			GMRoom,
		};		
	};
}

#endif

namespace Login
{
	struct CharacterSortCode
	{
		enum eCharacterSortCode
		{
			None = 0,
			World = 1,		// 1 = world, 
			Create = 2, 	// 2=Create, 
			Name = 3,		// 3=Name, 
			Level = 4,		// 4=Level, 
			Job = 5,		// 5=Class, 
			LastLogin = 6,	// 6=Recent login
			Max,
		};
	};
};

namespace CostumeMix
{
	struct RandomMix
	{
		enum eOpenType
		{
			OpenFail = -1,
			OpenByItem = 0,
			OpenByNpc
		};

		enum eConst
		{
			MixerCanMixPartsAll = -1,
			MixerCanMixBasicParts = 0,
		};
	};
}

#if defined(PRE_ADD_ACTIVEMISSION)
namespace ActiveMission
{
	enum ActiveMissionGainRate
	{
		NoneTypeActiveMissionGainRate = 13,
		NoneTypeActiveMissionGainBonusRate = 15,
		NestTypeActiveMissionGainRate = 5,
		NestTypeActiveMissionGainBonusRate = 8,
	};	

	enum ActiveMissionState
	{
		NOT_ASSIGNED = 0,
		GAIN_MISSION = 1,
		COMPLETE_MISSION = 2,
	};
};
#endif

#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
namespace PropDef
{
	struct Option
	{
		enum eTypeWithPriority
		{
			eMinVisibleOption,
			eRandomCreate = eMinVisibleOption,
			eTrigger,
			eMaxVisibleOption
		};

		enum eValue
		{
			NotSet = -1,
			False,
			True,
		};
	};
}
#endif

namespace Actoz
{
	struct ShutdownType
	{
		enum eShutdownType
		{
			Force = 1,		// Forced
			Selective = 2,	// Selective
		};
	};
}

namespace MapGateCondition
{
	//Set open time
	enum eOpenDateTimeType
	{
		oDailyHours = 0,	//Opens only at specific times each day
		oDayOfWeek = 1,	//Opens only on the specified day of the week (0~6: Sun~Sat)
		oDayOfWeekAndHours = 2,	//Specific day of week & time
		oDayOfWeekDuringFatigueTime = 3, //From a specific day of week until fatigue reset time
	};

	//Set close time
	enum eCloseDateTimeType
	{
		cMonthlyHours = 10, //Specific date & time
	};
}

#ifdef PRE_ADD_SERVER_WAREHOUSE
namespace Storage
{
	struct WorldServerStorage
	{
		enum eChargeType
		{
			eFeeCharge,
			eFree
		};

		enum eInvenType
		{
			eNormal,
			eCash
		};
	};
}
#endif

#if defined( PRE_ADD_GAMEQUIT_REWARD )
namespace GameQuitReward
{
	struct RewardType
	{
		enum eType		// Game end reward
		{
			None,					//No reward
			NewbieReward,			//New user reward
			ComeBackReward,			//Returnee reward
		};
	};
}
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
namespace AlteiaWorld
{
	struct Info
	{
		enum eInfoType
		{
			JoinInfo,
			PrivateGoldKeyRankInfo,
			PrivatePlayTimeRankInfo,
			GuildGoldKeyRankInfo,
			SendTicketInfo,
		};
	};	

	struct Common
	{
		enum eCommon
		{
			MaxRankCount = 10,				// Max ranking count
			MaxSendCount = 10,				// Max number of entry ticket gifts
			MoveNextMapTick = 10000,			// Time from rolling the dice to moving to the next map
		};
	};

	struct ResetType
	{
		enum eResetType
		{
			DailyPlayCount = 1,	// Daily play count
			WeeklyPlayCount,	// Weekly play count
			SendTicketList,		// Sent entry ticket list
		};
	};

	struct GoldKeyRankMemberInfo
	{
		int nRankIndex;
		BYTE cJob;
		WCHAR wszCharacterName[NAMELENMAX];
		WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
		int nGoldKeyCount;
	};

	struct PlayTimeRankMemberInfo
	{
		int nRankIndex;
		BYTE cJob;
		WCHAR wszCharacterName[NAMELENMAX];
		WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
		UINT nPlaySec;
	};

	struct GuildGoldKeyRankInfo
	{
		int nRankIndex;
		int nGuildID;
		WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
		short wGuildMark;					// Guild mark emblem
		short wGuildMarkBG;					// Guild mark background
		short wGuildMarkBorder;				// Guild mark border				
		int nGoldKeyCount;
	};
}
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined( PRE_ADD_STAMPSYSTEM )
namespace StampSystem
{
	struct Common
	{
		enum eCommon
		{
			MaxWeekDayCount = 7,	// Number of days of week 
			MaxChallengeCount = 8,  // Max achievement count
			MaxStampSlotCount = MaxChallengeCount * MaxWeekDayCount, // Number of stamp slots
		};
	};

	struct StampCompleteInfo
	{
		BYTE cChallengeIndex;	// Achievement Index
		int nWeekDay;			// Day of week (Sun=1,Mon=2,Tue=3,Wed=4,Thu=5,Fri=6,Sat=7)
	};
};
#endif // #if defined( PRE_ADD_STAMPSYSTEM )


#if defined(PRE_ADD_GUILD_CONTRIBUTION)
namespace GuildContribution
{
	struct TGuildContributionRankingData
	{
		INT64 biCharacterDBID;
		int nRank;
		WCHAR wszCharacterName[NAMELENMAX];
		BYTE cJobCode;
		int nWeeklyGuildContributionPoint;
	};
}
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)

#if defined( PRE_PVP_GAMBLEROOM )

const int SENDGAMBLEROOMWAITTICK = 5 * 60 * 1000;	// 5 minutes

namespace PvPGambleRoom
{
	enum eGambleType
	{
		NoneType = 0,
		Gold,
		Petal,
	};
	enum eType
	{
		firstWinner = 0,
		SecondWinner = 1,
		Max = 2,	// Number of users receiving rewards
	};
}
#endif // #if defined( PRE_PVP_GAMBLEROOM )

#if defined(PRE_ADD_EQUIPLOCK)
namespace EquipItemLock
{
	enum eLockStatus
	{
		None = 0,
		Lock = 1,
		RequestUnLock = 2,
	};

	struct TLockItem
	{
		EquipItemLock::eLockStatus eItemLockStatus; //0: no state, 1: locked, 2: unlocked
		__time64_t tUnLockDate;	//Use as lock date if locked, or unlock date if unlocked
		__time64_t tUnLockRequestDate;	//Lock request date
	};

	struct TLockItemInfo
	{
		BYTE cItemSlotIndex;
		TLockItem ItemData;
	};
}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined(PRE_ADD_WORLD_MSG_RED)
namespace WorldChatUseItem
{
	struct CashItemTypeParam
	{
		enum eItemTypeParam
		{
			BlueBird = 1,
			YellowBird = 2,
			RedBird = 3,
		};
	};
}
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)
