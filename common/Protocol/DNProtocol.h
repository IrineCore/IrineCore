
//#define ACTIVATE_PROTOCOLSHUFFLE // Always keep commented out so only DNProtocol.h is used until shuffle handling in CCNET is complete.

#if !defined (_FINAL_BUILD) || !defined (ACTIVATE_PROTOCOLSHUFFLE)

// PROTOCOL_ORIGIN_START -- Token required for shuffle, please do not delete.

#pragma once
const int PROCESS_DELAY_CHECK_TICK = 3000;

//-----------------------------------------------
//					Command
// - MainCmd
// - SubCmd
//-----------------------------------------------

const USHORT IN_DISCONNECT = 200;
const USHORT IN_DESTROY = 201;
const USHORT IN_SC_ITEM = 202;

// MainCmd
enum eCSMainCmd
{
	CS_LOGIN = 1,			// Login server side
	CS_SYSTEM,				// IP, port, and inter-server connection related, etc...
	CS_CHAR,				// Everything such as Enter, Leave, etc.
	CS_ACTOR,				// Actor related
	CS_PROP,				// Prop-related
	CS_PARTY,				// Party-related
	CS_ITEM,				// Item-related
	CS_ITEMGOODS,			// Move everything related to item products(?) here
	CS_NPC,					// NPC related
	CS_QUEST,				// Quest-related
	CS_CHAT,				// Chat
	CS_ROOM,				// Everything related to rooms
	CS_SKILL,				// Skill-related
	CS_TRADE,				// Shop, mail, unmanned shop, etc.
	CS_FRIEND,				// Friend-related
	CS_TRIGGER,				// Trigger
	CS_ACTORBUNDLE,			// Actor 
	CS_GUILD,				// guild
	CS_PVP,					// PvP
	CS_ISOLATE,				// Block!
	CS_MISSION,				// Mission
	CS_APPELLATION,			// Title
	CS_GAMEOPTION,			// Game options
	CS_RADIO,				// Radio message
	CS_GESTURE,				// Gesture
	CS_VOICECHAT,			// Voice chat
	CS_CASHSHOP,
	CS_CHATROOM,			// Chat room
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CS_REPUTATION,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	CS_MASTERSYSTEM,
#if defined( PRE_ADD_SECONDARY_SKILL )
	CS_SECONDARYSKILL,		// Secondary skill
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	CS_ETC,					// Junk
	CS_GUILDRECRUIT,		// Guild recruitment board
	CS_CUSTOMEVENTUI,
#if defined (PRE_ADD_DONATION)
	CS_DONATION,
#endif // #if defined (PRE_ADD_DONATION)

#if defined (PRE_ADD_BESTFRIEND)
	CS_BESTFRIEND,
#endif
#ifdef PRE_ADD_DOORS
	CS_DOORS,
#endif		//#ifdef PRE_ADD_DOORS
#if defined( PRE_PRIVATECHAT_CHANNEL )
	CS_PRIVATECHAT_CHANNEL,
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	CS_ALTEIAWORLD,
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( PRE_ADD_DWC )
	CS_DWC,
#endif // #if defined( PRE_ADD_DWC )
	//#####################################################
	// Please add above here!
	//#####################################################

	CS_FARM,
	CS_FISHING,
	CS_TIMEEVENT,

};

// MainCmd
enum eSCMainCmd
{
	SC_LOGIN = 1,			// Login server side
	SC_SYSTEM,				// IP, port, and inter-server connection related, etc...
	SC_CHAR,				// Character
	SC_ACTOR,				// Actor
	SC_PROP,				// Prop
	SC_PARTY,				// Party
	SC_ITEM,				// Item
	SC_ITEMGOODS,			// Move everything related to item products(?) here
	SC_NPC,					// NPC
	SC_QUEST,				// Quest
	SC_CHAT,				// Chat
	SC_ROOM,				// Everything related to rooms
	SC_SKILL,				// Skill-related
	SC_TRADE,				// Shop, mail, unmanned shop, etc.
	SC_FRIEND,				// Friend-related
	SC_TRIGGER,				// Trigger
	SC_ACTORBUNDLE,			// Actor
	SC_GUILD,				// guild
	SC_PVP,					// PvP
	SC_ISOLATE,				// Block!
	SC_MISSION,				// Mission
	SC_APPELLATION,			// Title
	SC_GAMEOPTION,			// Game options
	SC_RADIO,				// Radio!
	SC_GESTURE,				// Gesture
	SC_VOICECHAT,			// Voice chat
	SC_RESTRAINT,			// User sanction
	SC_CASHSHOP,
	SC_CHATROOM,			// Chat room
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	SC_REPUTATION,			// Reputation system
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	SC_MASTERSYSTEM,		// Master-apprentice system
	SC_VEHICLE,				// Mount
#if defined( PRE_ADD_SECONDARY_SKILL )
	SC_SECONDARYSKILL,		// Secondary skill
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
	SC_ETC,					// Junk
	SC_GUILDRECRUIT,		
	SC_CUSTOMEVENTUI,
#if defined (PRE_ADD_DONATION)
	SC_DONATION,
#endif // #if defined (PRE_ADD_DONATION)

#if defined (PRE_ADD_BESTFRIEND)
	SC_BESTFRIEND,
#endif
#ifdef PRE_ADD_DOORS
	SC_DOORS,
#endif		//#ifdef PRE_ADD_DOORS
#if defined( PRE_PRIVATECHAT_CHANNEL )
	SC_PRIVATECHAT_CHANNEL,
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	SC_ALTEIAWORLD,
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )
#if defined( PRE_ADD_STAMPSYSTEM )
	SC_STAMPSYSTEM,
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_ADD_DWC )
	SC_DWC,
#endif // #if defined( PRE_ADD_DWC )

	//#####################################################
	// Please add above here!
	//#####################################################

	SC_FARM,
	SC_FISHING,
	SC_TIMEEVENT,

};

// LOGIN -----------------------------------------------------------------------------

namespace eLogin {
	// Main: CS_LOGIN
	enum eCSLogin
	{
		CS_CHECKVERSION = 1,			// Check client version
		CS_CHECKLOGIN,					// (Just default)
		CS_SERVERLIST,					// Server list 
		CS_SELECTSERVER,				// Display character list
		CS_SELECTCHAR,					// Select character
		CS_CREATECHAR,					// Create character
		CS_DELETECHAR,					// Delete character
		CS_SELECTCHANNEL,				// Select channel
		CS_BACKBUTTON,					// Press back button
		CS_CHANNELLIST,					//Re-request channel list
		CS_BACKBUTTON_LOGIN,			// Server list -> Login screen
		CS_REVIVECHAR,
		CS_CHARACTERSORTCODE,
		CS_CREATEDWCCHAR,				// Create DWC character
		CS_CHECKLOGIN_US = 34,			// NA/US v353 Passport/Token login (0x22)
	};

	// Main: SC_LOGIN
	enum eSCLogin
	{
		SC_CHECKVERSION = 1,			// Version check
		SC_CHECKLOGIN,					// (Just default) Check id, pass
		SC_SERVERLIST,					// Server list
		SC_CHARLIST,					// Send character list
		SC_CHANNELLIST,					// Send channel list
		SC_SELECTCHAR,					// Select character
		SC_CREATECHAR,					// Create character
		SC_DELETECHAR,					// Delete character
		SC_TUTORIALMAPINFO,				// Tutorial map info
		SC_WAITUSER,					// You're in the waiting queue, what can you do hohoho
		SC_BACKBUTTON_LOGIN,			// Server list -> Login screen
		SC_CHECKBLOCK,					// Account suspended
		SC_REVIVECHAR,
		SC_CREATEDWCCHAR,				// DWC character creation result		
	};
};

// SYSTEM ---------------------------------------------------------------------------

namespace eSystem {
	// CS_SYSTEM
	enum eCSSystem
	{
		CS_CONNECTVILLAGE = 1,
		CS_VILLAGEREADY,				// Whether ready to go to the village server
		CS_CONNECT_REQUEST,
		CS_CONNECTGAME,					//gameserver connect with tcp
		CS_READY_2_RECIEVE,
		CS_INTENDED_DISCONNECT,			//Intentional disconnect
		CS_PEER_DISCONNECT,				//Peer connection lost between Clients
		CS_RECONNECTLOGIN,				// Reconnect to login (character select)
		CS_ABANDONSTAGE,				// Abandon stage
		CS_MOVE_PVPVILLAGETOLOBBY,		// Request to move from PvP town to PvP lobby
		CS_MOVE_PVPLOBBYTOPVPVILLAGE,	// Request to move from PvP lobby to PvP town
		CS_MOVE_PVPGAMETOPVPLOBBY,		// Request move from PvP game to PvP lobby

		CS_VIPAUTOMATICPAY,				// VIP auto-billing 

		CS_TCP_PING,
#ifdef PRE_MOD_INDUCE_TCPCONNECT
		CS_TCP_CONNECT_REQ,
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
		CS_CLIENT_PING,

		CC_PEER_CONNECT_REQUEST = 30,	//Below peer-related messages are intentionally spaced out a bit. Parsing splits here and these are peer-to-peer messages..
		CC_PEER_PING,
		CC_PEER_PONG,
	};

	// SC_SYSTEM
	enum eSCSystem
	{
		SC_VILLAGEINFO = 1,				// Village server info (send IP, PORT)
		SC_CONNECTVILLAGE,
		SC_GAMEINFO,					// Game server info (send IP, PORT)
		SC_PEER_CONNECT_REQUEST,		//GameServer -> Client Peer connection request
		SC_CONNECTED_RESULT,			//Message indicating Client -> GameServer RUDP connection is complete
		SC_TCP_CONNECT_REQ,				//Message instructing Client -> GameServer to establish TCP connection
		SC_RECONNECT_REQ,				//Request RUDP reconnection to game server
		SC_RECONNECTLOGIN,				// Reconnect to login
		SC_COUNTDOWNMSG,				// If there is something to count on client, just use this.

		SC_MOVE_PVPVILLAGETOLOBBY,		// Command to move from PvP town to PvP lobby

		SC_CHAR_COMM_OPTION,			// Receive opponent's community-related options

		SC_DETACHCONNECTION_MSG,		// Detaching message
		//ServerControl
		SC_ZEROPOPULATION,				// User control
		SC_VIPAUTOMATICPAY,				// VIP auto-billing 
		SC_SERVICECLOSE,

		SC_TCP_PING,
		SC_CLIENT_PING,
	};
};

// CHAR -----------------------------------------------------------------------------

namespace eChar {
	// Main: CS_CHAR
	enum eCSChar
	{
		CS_ENTER = 1,					// When character enters
		CS_ADDQUICKSLOT,				// Add to quick slot
		CS_DELQUICKSLOT,				// Remove from quick slot
		CS_LOOKUSER,					// Inspect user
		CS_COMPLETELOADING,				// Loading complete
		CS_CLOSE_UIWINDOW,				// Close UI window
#if defined( PRE_ADD_NOTIFY_ITEM_COMPOUND )
		CS_COMPOUND_NOTIFY,				// Save suffix notifier
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
		CS_STAGECLEAR_BONUSREWARD_SELECT, //Request bonus reward after stage clear
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	};

	// Main: SC_CHAR
	enum eSCChar
	{
		SC_MAPINFO = 1,					// Send map information
		SC_ENTER,						// My info
		SC_ENTERUSER,					// Nearby user info excluding self
		SC_LEAVEUSER,					// User exit
		SC_ENTERNPC,					// Nearby NPC info
		SC_LEAVENPC,					// Exit npc

		SC_QUICKSLOTLIST,				// Send quick slot bar (unclear where to categorize, so putting it here? -_-;)
		//SC_ADDQUICKSLOT, // Add to quick slot
		//SC_DELQUICKSLOT, // Remove from quick slot

		SC_FATIGUE,						// Send fatigue
		SC_LOOKUSER,					//Inspect user

		SC_PVPDATA,						// PvP info
		SC_HIDE,						// Transparency level

		SC_ENTERPROPNPC,
		SC_LEAVEPROPNPC,

		SC_PCBANG,
		SC_NESTCLEAR,					// Nest clear
		SC_UPDATENESTCLEAR,				// Update
		SC_MAXLEVEL_CHARACTER_COUNT,	// Max-level character

		SC_COMMONVARIABLE_LIST,
		SC_MOD_COMMONVARIABLE,
		SC_ATTENDANCEEVENT,
#if defined(PRE_ADD_REBIRTH_EVENT)
		SC_REBIRTH_MAXCOIN,				// For resurrection count event.
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
		SC_STAGECLEAR_BONUSREWARD_SELECT, //Send additional reward result
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)


	};
};

// ACTOR ----------------------------------------------------------------------------

namespace eActor {
	// CS_ACTOR
	enum eCSActor
	{
		CS_CMDMOVE = 1,
		CS_CMDSTOP,
		CS_CMDACTION,
		CS_CMDMIXEDACTION,
		CS_CMDMOVETARGET,
		CS_CMDMOVETARGET_NAVI,
		CS_CMDLOOKTARGET,
		CS_ONDAMAGE,
		CS_PROJECTILE,
		CS_INVALID_PROJECTILE,	// When the projectile reaches its max hit count, the server sends a command to remove the projectile. Not sent by the client.
		CS_CMDLOOK,
		CS_PLAYERPITCH,
		CS_CMDENTEREDGATE,
		CS_USESKILL,
		CS_AGGROTARGET,
		CS_VIEWSYNC,
		CS_CMDTOGGLEBATTLE,
		CS_CMDPASSIVESKILLACTION,
		CS_CMDADDSTATEEFFECT,
		CS_CMDREMOVESTATEEFFECT,
		CS_CMDREMOVESTATEEFFECTFROMID,
		CS_CMDMODIFYSTATEEFFECT,
		CS_ADDEXP,
		CS_LEVELUP,
		CS_ADDCOIN,
		CS_PARTYCHAIN,
		CS_CMDSUICIDE,
		CS_SETHPMP_DELTA,
		CS_SHADOW,
		CS_POSREV,
		CS_RECOVERYSP,
		CS_CMDENABLEOPERATOR,
		CS_REFRESHHPSP,
		//		CS_POSITION,
		CS_PARTSDAMAGE,
		CS_ADD_STATE_EFFECT_DENIED,
		CS_CMDWARP,
		CS_PLAYERDIE,
		CS_CP,
		CS_ONDROP,
		CS_CMDCHANGETEAM,
		CS_CMDFORCEVELOCITY,
		CS_CMDCHATBALLOON,
		CS_SHOWEXPOSUREINFO,
		CS_CMDSHOW,
		CS_CMDHIDE,
		CS_CMDTOGGLEWEAPONORDER,
		CS_CMDTOGGLEPARTSORDER,
		CS_CHANGEJOB,
		CS_RESET_SKILL,
		CS_CMDMOVEPOS,
		CS_CMDPICKUPITEM,
		CS_SLAVE_OF,				// Server sends the master actor unique ID indicating this is a summoned monster. Not sent by the client so this protocol is unused. Kept to align indices..
		CS_SKILLUSING_FAILED,		// Skill use failed; returned from server.
		CS_CMDESCAPE,
		CS_STATEEFFECT_PROB_SUCCESS,	// Notify the client that a probability-triggered state effect was activated by the server. Currently used only for server-to-client notifications.
		CS_MONSTER_PROJECTILE_FROM_PROJECTILE,		// // Related to #15557. Case where a monster fires a projectile from another projectile.
		CS_FINISH_PROCESSSKILL,
		CS_COOLTIME_RESET,
		CS_CANNONPOSSESS_REQ,		// Client requests game server to take ownership of cannon. Sent only from client.
		CS_CANNONPOSSESS_RES,		// Server response to client's cannon occupy request. Sent only from game server.
		CS_CANNONRELEASE,			// User released the cannon on client.
		CS_CANNONROTATESYNC,		// Send packet every 100ms with the actor using the cannon and the cannon's rotation.
		CS_CANNONTARGETING,			// Client operating the cannon calculates the impact point from current cannon direction and sends it to the server.
		CS_SLAVE_RELEASE,			// Opposite packet of SC_SLAVE_OF. Sent only by the server.
		CS_COOLTIMEPARRY_SUCCESS,	// Client does not SEND.
		CS_FREEZINGPRISON_DURABILITY,	//Display durability of FREEZINGPRISON.
		CS_FISHINGROD_CAST,
		CS_FISHINGROD_LIFT,
		CS_FISHINGROD_HIDE,
		CS_SYNCPRESSEDPOS,
		CS_ADDBUBBLE,				// Used only on game server. Notify client that a bubble was added.
		CS_REMOVEBUBBLE,			// Used only on game server. Notify client that a bubble was removed.
		CS_REFRESH_BUBBLE_DURATIONTIME, // Used only on game server. Notify client to reset only the timer since max bubble stack count was reached.
		CS_SHOCK_INFECTION,			// From game server to client only..(packet to notify projectile creation in ShockInfectionBlow..)
		CS_PASSIVESKILL_COOLTIME,	// Request from game server to client to display passive skill cooldown (only works when Actor is LocalPlayer..)
		CS_SHOW_STATE_EFFECT,		// Packet to display effect only when a probabilistic passive state effect triggers
		CS_DO_TEMP_JOBCHANGE,		// Temporary job change. Sent from server to client only.
		CS_RESTORE_TEMP_JOBCHANGE,	// Restore temporary job change. Sent from server to client only.
		CS_ADD_TEMP_SKILL,			// Temporarily add skill. Sent from server to client only.
		CS_REMOVE_TEMP_SKILL,		// Remove temporarily added skill. Sent from server to client only.
		CS_CMDUPDATESTATEBLOW,		// Add prefix state effect and send packet (S->C only) to force update state effects
		CS_FINISH_AURASKILL,		// Send S->C packet when aura skill stops due to MP check mid-skill.
		CS_CMDSHOOTMODE,
		CS_BLOW_GRAPHIC_ERASE,
		CS_CATCH_ACTOR,				// Monster grabs character. Sent only from server to client. #28532
		CS_CATCH_ACTOR_FAILED,		// Client does not SEND.
		CS_RELEASE_ACTOR,			// Monster releases character. Sent only from server to client. #28532
		CS_MONSTERPARTS_STATE,
		CS_VEHICLE_RIDE_COMPLETE,   // Notify the server that the player has mounted normally.
		CS_GHOST_TYPE,				// Client does not SEND.
		CS_UDP_PING,
		CS_ACTION_CHANGE,			// Behavior changed for Academic Wax skill (Server->Client only)
		CS_CHAINATTACK_PROJECTILE,	// For creating ping-pong bomb/chain projectiles (server -> client only)
		CS_SUMMONOFF,				// Unsummon summoned monster (Client -> Server only)
		CS_PROBINVINCIBLE_SUCCESS,	// #41440 Damage nullification succeeded via state effect 226. Effect used by Origin items.
		CS_FORCE_ADD_SKILL,         
		CS_FORCE_REMOVE_SKILL, 
		CS_MAILBOX_OPEN,			//Open mailbox via shortcut key in town (client -> server, server only broadcasts..)
		CS_MAILBOX_CLOSE,			//When closing mailbox opened via shortcut key in town (client -> server, server only broadcasts..)
		CS_APPLY_SUMMON_MONSTER_EX_SKILL,		//For applying summoned monster EX skill (Server -> Client)
		CS_TOTAL_LEVEL,					//Character's combined level info (packet sent only from server -> client)
		CS_ADD_TOTAL_LEVEL_SKILL,		//Request packet from Client -> Server when applying unified level skill to slot
		CS_REMOVE_TOTAL_LEVEL_SKILL,	//Request packet from Client -> Server when removing unified level skill from slot
		CS_TOTAL_LEVEL_SKILL_ACTIVE_LIST,	//Unified level skill registration list info (sent Server -> Client only)
		CS_TOTAL_LEVEL_SKILL_CASHSLOT_ACTIVATE,	//Unified level skill cash slot activation info (Server -> Client..)
		CS_SKIP_END_ACTION,	// #68898 Send info added to Input signal (skip EndAction of the skill's partialPalyAniProcessor when Input signal is processed).
		CS_ACTOR_STATE_MAX,  // Please leave at the end nextome
	};

	// Must be declared in both eCSActor / eSCActor.
	// SC_ACTOR
	enum eSCActor
	{
		SC_CMDMOVE = 1,
		SC_CMDSTOP,
		SC_CMDACTION,
		SC_CMDMIXEDACTION,
		SC_CMDMOVETARGET,
		SC_CMDMOVETARGET_NAVI,
		SC_CMDLOOKTARGET,
		SC_ONDAMAGE,
		SC_PROJECTILE,
		SC_INVALID_PROJECTILE,	// When the projectile reaches its max hit count, the server sends a command to remove the projectile. Not sent by the client.
		SC_CMDLOOK,
		SC_PLAYERPITCH,
		SC_CMDENTEREDGATE,
		SC_USESKILL,
		SC_AGGROTARGET,
		SC_VIEWSYNC,
		SC_CMDTOGGLEBATTLE,
		SC_CMDPASSIVESKILLACTION,
		SC_CMDADDSTATEEFFECT,
		SC_CMDREMOVESTATEEFFECT,
		SC_CMDREMOVESTATEEFFECTFROMID,
		SC_CMDMODIFYSTATEEFFECT,
		SC_ADDEXP,
		SC_LEVELUP,
		SC_ADDCOIN,
		SC_PARTYCHAIN,
		SC_CMDSUICIDE,
		SC_SETHPMP_DELTA,
		SC_SHADOW,
		SC_POSREV,
		SC_RECOVERYSP,
		SC_CMDENABLEOPERATOR,
		SC_REFRESHHPSP,
		SC_PARTSDAMAGE,
		SC_ADD_STATE_EFFECT_DENIED,
		SC_CMDWARP,
		SC_PLAYERDIE,				// Currently used only in PvP.
		SC_CP,
		SC_ONDROP,
		SC_CMDCHANGETEAM,
		SC_CMDFORCEVELOCITY,
		SC_CMDCHATBALLOON,
		SC_SHOWEXPOSUREINFO,
		SC_CMDSHOW,
		SC_CMDHIDE,
		SC_CMDTOGGLEWEAPONORDER,
		SC_CMDTOGGLEPARTSORDER,
		SC_CHANGEJOB,
		SC_RESET_SKILL,
		SC_CMDMOVEPOS,
		SC_CMDPICKUPITEM,
		SC_SLAVE_OF,				// Server sends the master actor unique ID indicating this is a summoned monster.
		SC_SKILLUSING_FAILED,		// Skill use failed; returned from server.
		SC_CMDESCAPE,
		SC_STATEEFFECT_PROB_SUCCESS,	// Notify the client that a probability-triggered state effect was activated by the server. Currently used only for server-to-client notifications.
		SC_MONSTER_PROJECTILE_FROM_PROJECTILE,		// // Related to #15557. Case where a monster fires a projectile from another projectile.
		SC_FINISH_PROCESSSKILL,
		SC_COOLTIME_RESET,
		SC_CANNONPOSSESS_REQ,		// Client requests game server to take ownership of cannon. Sent only from client.
		SC_CANNONPOSSESS_RES,		// Server response to client's cannon occupy request. Sent only from game server.
		SC_CANNONRELEASE,			// Client released the cannon.
		SC_CANNONROTATESYNC,		// Send packet every 100ms with the actor using the cannon and the cannon's rotation.
		SC_CANNONTARGETING,			// Client calculates landing point based on current cannon direction and sends to server.
		SC_SLAVE_RELEASE,			// Opposite packet of SC_SLAVE_OF. Sent only by the server.
		SC_COOLTIMEPARRY_SUCCESS,	// Notify client when parry succeeds on server.
		SC_FREEZINGPRISON_DURABILITY,	//Display durability of FREEZINGPRISON. (S -> C only)
		SC_FISHINGROD_CAST,
		SC_FISHINGROD_LIFT,
		SC_FISHINGROD_HIDE,
		SC_SYNCPRESSEDPOS,
		SC_ADDBUBBLE,				// Used only on game server. Notify client that a bubble was added.
		SC_REMOVEBUBBLE,			// Used only on game server. Notify client that a bubble was removed.
		SC_REFRESH_BUBBLE_DURATIONTIME, // Used only on game server. Notify client to reset only the timer since max bubble stack count was reached.
		SC_SHOCK_INFECTION,			// From game server to client only..(packet to notify projectile creation in ShockInfectionBlow..)
		SC_PASSIVESKILL_COOLTIME,	// Request from game server to client to display passive skill cooldown (only works when Actor is LocalPlayer..)
		SC_SHOW_STATE_EFFECT,		// Packet to display effect only when a probabilistic passive state effect triggers
		SC_DO_TEMP_JOBCHANGE,		// Temporary job change. Sent from server to client only.
		SC_RESTORE_TEMP_JOBCHANGE,	// Restore temporary job change. Sent from server to client only.
		SC_ADD_TEMP_SKILL,			// Temporarily add skill. Sent from server to client only.
		SC_REMOVE_TEMP_SKILL,		// Remove temporarily added skill. Sent from server to client only.
		SC_CMDUPDATESTATEBLOW,		// Add prefix state effect and send packet (S->C only) to force update state effects
		SC_FINISH_AURASKILL,		// Send S->C packet when aura skill stops due to MP check mid-skill.
		SC_CMDSHOOTMODE,
		SC_BLOW_GRAPHIC_ERASE,
		SC_CATCH_ACTOR,				// Monster grabs character. Sent only from server to client. #28532
		SC_CATCH_ACTOR_FAILED,		// Monster failed to grab character. Currently handled by executing an action due to parry or block. #28532
		SC_RELEASE_ACTOR,			// Monster releases character. Sent only from server to client. #28532
		SC_MONSTERPARTS_STATE,
		SC_VEHICLE_RIDE_COMPLETE,   // Server does not SEND.
		SC_GHOST_TYPE,				// Client does not SEND.
		SC_UDP_PING,
		SC_ACTION_CHANGE,			// Behavior changed for Academic Wax skill (Server->Client only)
		SC_CHAINATTACK_PROJECTILE,	// For creating ping-pong bomb/chain projectiles (server -> client only)
		SC_SUMMONOFF,				// Unsummon summoned monster (Client -> Server only)
		SC_PROBINVINCIBLE_SUCCESS,	// #41440 Damage nullification succeeded via state effect 226. Effect used by Origin items.
		SC_FORCE_ADD_SKILL,         // Add or remove Actor's skill / sent only from server.
		SC_FORCE_REMOVE_SKILL,      // 
		SC_MAILBOX_OPEN,			//Open mailbox via shortcut key in town
		SC_MAILBOX_CLOSE,			//When closing mailbox after opening via shortcut key in town
		SC_APPLY_SUMMON_MONSTER_EX_SKILL,		//For applying summoned monster EX skill (Server -> Client)
		SC_TOTAL_LEVEL,					//Character's combined level info (packet sent only from server -> client)
		SC_ADD_TOTAL_LEVEL_SKILL,		//Result packet from Server -> Client when applying unified level skill to slot
		SC_REMOVE_TOTAL_LEVEL_SKILL,	//Result packet from Server -> Client when removing unified level skill from slot
		SC_TOTAL_LEVEL_SKILL_ACTIVE_LIST,	//Unified level skill registration list info (sent Server -> Client only)
		SC_TOTAL_LEVEL_SKILL_CASHSLOT_ACTIVATE,	//Unified level skill cash slot activation info (Server -> Client..)
		SC_SKIP_END_ACTION,	// #68898 Send info added to Input signal (skip EndAction of the skill's partialPalyAniProcessor when Input signal is processed).
		SC_ACTOR_STATE_MAX, // Please leave at the end nextome
	};
};

// PROP -----------------------------------------------------------------------------

// prop
namespace eProp {
	// CS_PROP
	enum eCSProp
	{
		CS_ONDAMAGE,
		CS_CMDACTION,
		CS_CMDSHOW,
		CS_CMDOPERATION,
		CS_CMDENABLEOPERATOR,
		CS_CMDKEEPCLICK,
		CS_CMDKEEPCLICKSTART,
		CS_CMDKEEPCLICKCANCLE,
		CS_QUERY_RANDOM_CREATED, // #42473 When _VisibleProb is not 100%, client queries whether it was spawned by the server's random roll.
		CS_CMDCHATBALLOON_AS_INDEX,
		CS_SEND_SHOWINFO,
		CS_PROP_STATE_MAX,		// Please leave at the end
	};

	// SC_PROP
	enum eSCProp
	{
		SC_ONDAMAGE,
		SC_CMDACTION,
		SC_CMDSHOW,
		SC_CMDOPERATION,
		SC_CMDENABLEOPERATOR,
		SC_CMDKEEPCLICK,
		SC_CMDKEEPCLICKSTART,
		SC_CMDKEEPCLICKCANCLE,
		SC_QUERY_RANDOM_CREATED, // #42473 When _VisibleProb is not 100%, client queries whether it was spawned by the server's random roll.
		SC_CMDCHATBALLOON_AS_INDEX,
		SC_SEND_SHOWINFO,
		SC_PROP_STATE_MAX,		// Please leave at the end
	};
};

// PARTY ----------------------------------------------------------------------------

namespace eParty {
	// Main: CS_PARTY
	enum eCSParty
	{
		CS_CREATEPARTY = 1,			// Create party
		CS_JOINPARTY,				// Join party
		CS_PARTYOUT,				// Leave party
		CS_PARTYLISTINFO,			// Show party list
		CS_REFRESHGATEINFO,			// Update info when all party members gather in front of the gate
		CS_STARTSTAGE,				// Go to field
		CS_CANCELSTAGE,				// Cancel going to field
		CS_PARTYINVITE,				//Party invite
		CS_PARTYINVITE_DENIED,		//Send when rejecting an invite
		CS_PARTYINVITE_OVERSERVER_RESULT,		//Handling when receiving an invite request on game server (in village)
		CS_PARTYMEMBER_INFO,		//Request party member info
		CS_PARTYMEMBER_KICK,		//Kick member
		CS_PARTYLEADER_SWAP,		//Delegate party leader
		CS_CHANNELLIST,				//For changing channels in village during game
		CS_SELECTCHANNEL,			//Request channel to move to
		CS_PARTYMODIFY,				//Modify party info
		CS_SELECTDUNGEON,			//Send the window selection by the party leader when starting a dungeon
		CS_JOINGETREVERSEITEM,		//Whether to participate in rolling dice for bound items
		CS_READYREQUEST,			//Request party members to gather at the portal
		CS_ABSENT,					//When AFK on game server
		CS_VERIFYTARGETDUNGEON,		//Validate target dungeon setting when creating party or changing info
		CS_PARTYASKJOIN,			//Request to join party using leader (or member) name as key
		CS_PARTYASKJOINDECISION,		//Result value when party leader receives a join request
		CS_SWAPMEMBERINDEX,			//Change party member index
		CS_CONFIRM_LASTDUNGEONINFO,	// Response for rejoining previous party
		CS_SELECTSTAGE,
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
		CS_PARTYINFO,
#endif
	};

	// Main: SC_PARTY
	enum eSCParty
	{
		SC_CREATEPARTY = 1,				// Create party
		SC_JOINPARTY,					// Join party
		SC_REFRESHPARTY,				// Refresh party
		SC_PARTYOUT,					// Party ended
		SC_PARTYLISTINFO,				// Show party list
		SC_REFRESHGATEINFO,				// Update gate for party members ready to exit to field
		SC_GATEINFO,					// Party members gate info
		SC_EQUIPDATA,					//Equipped item data
		SC_STARTSTAGE,					// Go to field
		SC_STARTVILLAGE,				// Village server is ready, come on in~
		SC_STARTSTAGEDIRECT,			// From dungeon to world map.
		SC_CANCELSTAGE,					// Cancel going to field
		SC_CHANGEPARTYLEADER,			// Party leader changed
		SC_PARTYINVITE,					//Send party invitation message to user
		SC_PARTYINVITEFAIL,				//Party invite failed
		SC_PARTYMEMBER_INFO,			//Party member info
		SC_PARTYINFO_ERROR,				//Party info error
		SC_PARTYMEMBERKICKED,
		SC_PARTYLEADERSWAP,
		SC_SKILLDATA,					// Passive skill data triggered when matching equipped item data
		SC_CHANNELLIST,					//For changing channels in village during game
		SC_MOVECHANNELFAIL,
		SC_DEFAULTPARTSDATA,			// Parts index data on initial creation
		SC_SELECTDUNGEON,				//Dungeon info (selected upon dungeon entry)
		SC_PARTYMODIFY,					//When party info changes		
		SC_VERIFYJOINPARTY,				//For pre-verification
		SC_PARTYMEMBER_MOVE,			// Related to party member movement
		SC_ETCDATA,						// Send other minor party info that needs syncing here.
		SC_PARTYINVITEDENIED,			// Someone invited has rejected
		SC_PARTYMEMBER_PART,			// Partial update of party member info
		SC_GUILDDATA,					// Collect and send each party member's guild info
		SC_EQUIPCASHDATA,
		SC_WEAPONORDERDATA,
		SC_REQUEST_JOINGETREVERSIONITEM,	// Ask party members whether to join dice roll for bound item
		SC_NOTIFY_ROLLGETREVERSIONITEM,		// Notify that a party member rolled dice for a bound item
		SC_NOTIFY_RESULTGETREVERSIONITEM,	// Notify party members of bound item roll result
		SC_PARTYREQUEST,					// Request ready status from party member
		SC_ABSENT,							// Notify party members of AFK status
		SC_VERIFYTARGETDUNGEON,				// Check dungeon for party member or individual (when creating party)
		SC_REFRESHREBIRTHCOIN,				// For client UI update from game server
		SC_PARTYBONUSVALUE,					// For updating when a specific bonus is applied to the party
		SC_GLYPHDATA,
		SC_PARTYINVITENOTICE,				// Handle notification when party invite is on game server
		SC_VEHICLEEQUIPDATA,
		SC_ASKJOINPARTYTOLEADER,			// Request party join to party leader
		SC_ASKJOINPARTYRESULT,				// Party request result message
		SC_ASKJOINPARTYDECISIONINFO,		// Party leader's decision
		SC_PARTYUPDATEFIELDUI,				// For refreshing party UI in town
		SC_SWAPMEMBERINDEX,					// Index swap update (could send via refresh, but separated since it may be frequent)
		SC_PETEQUIPDATA,
		SC_CHECK_LASTDUNGEONINFO,			// Check for rejoining previous dungeon
		SC_CONFIRM_LASTDUNGEONINFO,			// Response for rejoining previous dungeon
		SC_SELECTSTAGE,
#if defined ( PRE_ADD_BESTFRIEND)
		SC_BESTFRIENDDATA,
#endif
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
		SC_PARTYINFO,
#endif
	};
};

// ITEM -----------------------------------------------------------------------------
// Since CS and SC numbers cannot overlap on the item side, adding goods too would exceed byte (cmd value). Moved out to ItemGoods

namespace eItem {
	// Main: CS_ITEM
	enum eCSItem
	{
		CS_MOVEITEM = 1,				// inven <-> ware
		CS_REMOVEITEM,					// Move to trash
		CS_PICKUP,						// Since SC_PICKUP is used on the server, indices overlap, so keep this as a dummy.
		CS_USEITEM,						// Items used like potions
		CS_EMBLEM_COMPOUND_OPEN_REQ,	// Request to open heraldry jewel window
		CS_ITEM_COMPOUND_OPEN_REQ,		// Request to open item combination window
		CS_EMBLEM_COMPOUND_REQ,			// Request glyph jewel
		CS_ITEM_COMPOUND_REQ,			// Item synthesis request
		CS_EMBLEM_COMPOUND_CANCEL_REQ,	// Request to cancel heraldry jewel
		CS_ITEM_COMPOUND_CANCEL_REQ,	// Item synthesis cancel request
		CS_MODITEMEXPIREDATE,			// Modify item attributes. Currently used for extending paid pet expiration. by supertj@20110524 
		CS_REBIRTH,						// Resurrect (to town)
		CS_REBIRTH_COIN,				// Resurrect, use coin

		CS_MOVECOIN,					// Storage money

		// Round-trip related to random item.
		CS_COMPLETE_RANDOMITEM,			// Complete random item presentation
		CS_CANCEL_RANDOMITEM,			// Cancel random item

		// Item disassembly. Handled only on GameServer since it only happens in world zones.
		CS_DISJOINT_REQ,

		// Enhance
		CS_ENCHANT,						// Enhance button
		CS_ENCHANTCOMPLETE,				// Enhancement complete
		CS_ENCHANTCANCEL,				// Cancel enhancement button

		CS_SORTINVENTORY,				// Sort items
		CS_SORTCASHINVENTORY,			// Sort cash items

		CS_MOVECASHITEM,

		CS_MOVEGUILDITEM,				// Move guild item
		CS_MOVEGUILDCOIN,				// Move guild coin
		CS_DELETE_PET_SKILL,			// Delete pet skill..
		CS_SORTWAREHOUSE,				// Sort personal storage
		CS_REMOVECASH,

		CS_USE_AUTOUNPACK_CASHITEM_REQ, // Request to use auto-open item.
		CS_AUTOUNPACK_CLOSE,
#if defined(PRE_ADD_SERVER_WAREHOUSE)
		CS_MOVE_SERVERWARE,				// Move normal item in server storage
		CS_MOVE_CASHSERVERWARE,			// Move cash item in server storage
#endif
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		CS_TALISMAN_OPENSLOT,
#endif
#if defined(PRE_ADD_EQUIPLOCK)
		CS_ITEM_LOCK_REQ,				// Item lock request
		CS_ITEM_UNLOCK_REQ,				// Item unlock request
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_PERIOD_INVENTORY)
		CS_SORTPERIODINVENTORY,
		CS_SORTPERIODWAREHOUSE,
#endif	// #if defined(PRE_PERIOD_INVENTORY)
	};

	// Main: SC_ITEM
	enum eSCItem
	{
		SC_INVENLIST = 1,				// Send all inventory info
		SC_CASHINVENLIST,				// Send full cash inven info
		SC_EQUIPLIST,
		SC_MOVEITEM,					// Item move: INVEN <-> EQUIP, EQUIP <-> EQUIP, INVEN <-> INVEN
		SC_PICKUP,						// Pick up item from ground
		SC_REMOVEQUESTITEM,				// Quest item
		SC_REMOVEITEM,					// Discard item into trash
		SC_CHANGEEQUIP,					// Broadcast equip info to nearby players
		SC_CHANGECASHEQUIP,				// Broadcast cash equip info to nearby players
		SC_REFRESHINVEN,				// Add and remove items in inventory
		SC_REFRESHCASHINVEN,			// Fix cash inven
		SC_CREATE_DROPITEM,
		SC_DROPITEM_MSG,
		SC_WAREHOUSELIST,				// Warehouse
		SC_USEITEM,						// Using potion-like items
		SC_EMBLEM_COMPOUND_OPEN_RES,	// Request to open heraldry jewel window
		SC_ITEM_COMPOUND_OPEN_RES,		// Request to open item combination window
		SC_EMBLEM_COMPOUND_RES,			// Heraldry jewel result response
		SC_ITEM_COMPOUND_RES,			// Item combination result response
		SC_EMBLEM_COMPOUND_CANCEL_RES,	// Request to cancel heraldry jewel
		SC_ITEM_COMPOUND_CANCEL_RES,	// Item synthesis cancel request
		SC_INVENTORYMAXCOUNT,			// Max inventory count
		SC_WAREHOUSEMAXCOUNT,			// Max storage count
		SC_QUESTINVENLIST,				// Clear quest inventory
		SC_REFRESHQUESTINVEN,			// Update quest inventory (create, delete)

		SC_REBIRTH,						// Resurrect (to town)
		SC_REBIRTH_COIN,				// Resurrect, use coin
		SC_REFRESHEQUIP,				// Equipment refresh
		SC_REPAIR_EQUIP,				// Repair equipped items
		SC_REPAIR_ALL,					// Repair all

		SC_MOVECOIN,					// Inventory <-> Storage coin

		SC_MOVEGUILDITEM,				// Move guild item
		SC_MOVEGUILDCOIN,				// Move guild coin
		SC_REFRESH_GUILDITEM,			// Update guild item info
		SC_REFRESH_GUILDCOIN,			// Update guild coin info

		// Round-trip related to random item.
		SC_REQUEST_RANDOMITEM,			// Start random item presentation
		SC_COMPLETE_RANDOMITEM,			// Random item result value

		// Item disassembly. Handled only on GameServer since it only happens in world zones.
		SC_DISJOINT_RES,

		// Enhance
		SC_ENCHANT,						// Enhance button
		SC_ENCHANTCOMPLETE,				// Enhancement complete
		SC_ENCHANTCANCEL,				// Cancel enhancement button

		SC_GLYPHTIMEINFO,				// Heraldry Dealy, Remain info
		SC_SORTINVENTORY,				// Sort items
		SC_SORTCASHINVENTORY,

		SC_DECREASE_DURABILITY_INVENTORY,
		SC_BROADCASTINGEFFECT,			// Item effect presentation

		SC_CREATE_DROPITEMLIST,

		SC_MOVECASHITEM,
		SC_CHANGEGLYPH,					// Broadcast glyph info to nearby players

		SC_VEHICLEEQUIPLIST,			// Send mount equipment info
		SC_VEHICLEINVENLIST,			// Send pet & mount inventory info.
		SC_CHANGEVEHICLEPARTS,			// Broadcast Vehicle info to nearby players
		SC_CHANGEVEHICLECOLOR,			// 
		SC_REFRESHVEHICLEINVEN,			// 
		SC_CHANGEPETPARTS,				// Broadcast Pet info to nearby players
		SC_CHANGEPETBODY,				// Send full pet info to nearby players (when sending as Body)
		SC_PETEQUIPLIST,				// Send pet summon info.
		SC_ADDPETEXP,					// Pet EXP

#if defined( PRE_ADD_VIP_FARM )
		SC_EFFECTITEM_INFO,
#endif // #if defined( PRE_ADD_VIP_FARM )
		SC_GLYPHEXTENDCOUNT,			// Number of additional heraldry slots
		SC_GLYPHEXPIREDATA,				// Send timed heraldry slot info
		SC_INCREASELIFE,
		SC_USE_SOURCE,
		SC_DELETE_PET_SKILL,
		SC_REFRESHWAREHOUSE,
		SC_SORTWAREHOUSE,
		SC_REMOVECASH,

		SC_USE_AUTOUNPACK_CASHITEM, // Result of auto-open item use request.

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		SC_EFFECTSKILLITEM,
		SC_DELEFFECTSKILLITEM,
		SC_SHOWEFFECT,
		SC_SENDNAMEDITEMID,
#endif
		SC_PET_CURRENT_SATIETY,		// Change pet satiety value
		SC_PET_FOOD_EAT,			// Pet feeding result
#if defined(PRE_ADD_EXPUP_ITEM)
		SC_DAILY_LIMIT_ITEM,		// For error message of item with daily usage limit
#endif
		SC_MODITEMEXPIREDATE,		// Validity period extension request response packet - stupidfox #60598
#if defined( PRE_ADD_LIMITED_SHOP )
		SC_LIMITEDSHOPITEMDATA,		// Purchase-count-limited item list
#endif
#if defined( PRE_ADD_TRANSFORM_POTION)
		SC_CHANGE_TRANSFORM,		// Transform
#endif
#if defined( PRE_ADD_SERVER_WAREHOUSE)
		SC_SERVERWARE_LIST,				// Server storage list
		SC_CASHSERVERWARE_LIST,			// Server storage cash list
		SC_MOVE_SERVERWARE,				// Move normal item in server storage
		SC_MOVE_CASHSERVERWARE,			// Move cash item in server storage
#endif // #if defined( PRE_ADD_SERVER_WAREHOUSE)
#if defined(PRE_ADD_PVP_EXPUP_ITEM)
		SC_USE_PVPEXPUP,			// Colosseum EXP boost item usage result
#endif // #if defined(PRE_ADD_PVP_EXPUP_ITEM)
#if defined(PRE_ADD_EQUIPLOCK)
		SC_ITEM_LOCK_RES,				// Return item lock result
		SC_ITEM_UNLOCK_RES,				// Return item unlock result
		SC_EQUIPITEM_LOCKLIST,			// Send locked item list
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_PERIOD_INVENTORY)
		SC_PERIODINVENTORY,
		SC_PERIODWAREHOUSE,
		SC_SORTPERIODINVENTORY,
		SC_SORTPERIODWAREHOUSE,
#endif	// #if defined(PRE_PERIOD_INVENTORY)
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		SC_TALISMAN_OPENSLOT,
		SC_CHANGETALISMAN,
		SC_TALISMANEXPIREDATA,
#endif
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
		SC_STAGE_USELIMITITEM,
#endif
		SC_SPECIAL_ITEM_LIST,			// Special/daily item check list (up to 10 item IDs)
		SC_AUTO_UNPACK_RESULT,			// Auto-unpack / charm progress completion result
		SC_DELETE_EXPIRE_ITEM_LIST,		// Batch delete expired items by serial
	};
};


// ITEMGOODS -----------------------------------------------------------------------------
// Moved here because too many cash items in the item section might overflow byte (cmd value).

namespace eItemGoods {
	// Main: CS_ITEMGOODS
	enum eCSItemGoods
	{
		CS_UNSEAL,						// Unseal item
		CS_SEAL,						// Seal item
#ifdef PRE_ADD_GACHA_JAPAN
		CS_JP_GACHAPON_REQ,				// Japan Gachapon item request.
#endif //PRE_ADD_GACHA_JAPAN

		// Costume synthesis
		CS_COSMIXOPEN,
		CS_COSMIXCLOSE,
		CS_COSMIXCOMPLETE,

		CS_POTENTIALJEWEL,

		// Charm
		CS_CHARMITEMREQUEST,			// Start Charm item sequence
		CS_CHARMITEMCOMPLETE,			// Charm item sequence complete
		CS_CHARMITEMCANCEL,				// Cancel Charm item


#if defined (PRE_ADD_CHAOSCUBE)
		// Chaos Cube
		CS_CHAOSCUBEREQUEST,			// Start Chaos Cube sequence
		CS_CHAOSCUBECOMPLETE,			// Chaos Cube sequence complete
		CS_CHAOSCUBECANCEL,				// Cancel Chaos Cube
#endif

		// GuildReName
		CS_GUILDRENAME,					// Guild name change cash item
		CS_CHARACTERRENAME,				// Character rename cash item
		CS_ENCHANTJEWEL,
		CS_GUILDMARK,					// Change guild mark
		CS_CHANGEPETNAME,				// Rename pet
		CS_USE_CHANGEJOB_CASHITEM_REQ,	// Request to use instant job change item.

		// Costume design synthesis
		CS_DESIGNMIXCLOSE,
		CS_DESIGNMIXCOMPLETECLOSE,
		CS_DESIGNMIXCOMPLETE,

		CS_REMOVE_PREFIX,				//Delete suffix(?) info
		CS_PETSKILLITEM,				// Equip pet skill item
		CS_PETSKILLEXPAND,				// Pet skill slot expansion item		

		CS_WARP_VILLAGE_LIST,
		CS_WARP_VILLAGE,

		CS_RANDOMMIXCLOSE,
		CS_RANDOMMIXCOMPLETECLOSE,
		CS_RANDOMMIXCOMPLETE,

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		CS_EXCHANGE_POTENTIAL,			// Potential transfer
#endif			//#ifdef PRE_ADD_EXCHANGE_POTENTIAL

#if defined (PRE_ADD_BESTFRIEND)
		CS_BESTFRIENDREQUEST,
		CS_BESTFRIENDCOMPLETE,
		CS_BESTFRIENDCANCEL,		// CS_CHARMITEMCANCEL, CS_CHAOSCUBECANCEL -> Merge into CS_RANDOMITEMCANCEL later
#endif
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
		CS_EXCHANGE_ENCHANT,			// Enhancement transfer
#endif
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
		CS_POTENTIALJEWEL_ROLLBACK,			//Cancel potential grant
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
#if defined(PRE_ADD_CHAOSCUBE)
		CS_CHAOSGAUGECUBE_ADDSTUFF,			// Feed material into Chaos Gauge Cube
		CS_CHAOSGAUGECUBE_COMPLETE,			// Claim result from full Chaos Gauge Cube
#endif
	};

	// Main: SC_ITEMGOODS
	enum eSCItemGoods
	{
		SC_UNSEAL,						// Unseal item
		SC_SEAL,						// Seal item
		SC_REQUEST_CASHWORLDMSG,		// Cash item world chat
		SC_CHANGECOLOR,					// HAIR, EYE, SKIN COLOR value changes

#ifdef PRE_ADD_GACHA_JAPAN
		SC_JP_GACHAPON_OPEN_SHOP_RES,	// Open Japan Gachapon shop.
		SC_JP_GACHAPON_RES,				// Japan Gachapon item request response.
#endif //PRE_ADD_GACHA_JAPAN

		// Costume synthesis
		SC_COSMIXOPEN,
		SC_COSMIXCLOSE,
		SC_COSMIXCOMPLETECLOSE,
		SC_COSMIXCOMPLETE,

		// Attach potential
		SC_POTENTIALJEWEL,

		// Charm
		SC_CHARMITEMREQUEST,			// Start Charm item sequence
		SC_CHARMITEMCOMPLETE,			// Charm item result value

#if defined (PRE_ADD_CHAOSCUBE)
		// Chaos Cube
		SC_CHAOSCUBEREQUEST,			// Start Chaos Cube sequence
		SC_CHAOSCUBECOMPLETE,			// Chaos Cube result value
#endif

		SC_PETALTOKENRESULT,			// Show purchased Petal voucher in chat window..
		SC_APPELLATIONGAINRESULT,
		SC_GUILDRENAME,					// Guild name change result
		SC_CHARACTERRENAME,				// Character rename result
		SC_ENCHANTJEWEL,
		SC_GUILDMARK,					// Guild mark change result
		SC_USE_CHANGEJOB_CASHITEM_RES,	// Response to client for instant job change item use.
		SC_CHANGEPETNAME,				// Rename pet

		// Costume design synthesis
		SC_DESIGNMIXOPEN,
		SC_DESIGNMIXCLOSE,
		SC_DESIGNMIXCOMPLETECLOSE,
		SC_DESIGNMIXCOMPLETE,

		SC_REMOVE_PREFIX,				//Delete suffix(?) info

		SC_EXPANDSKILLPAGERESULT,		// Dual skill tree item usage result
		SC_PETSKILLITEM,				// Result of using pet skill item
		SC_PETSKILLEXPAND,				// Result of using pet skill slot expansion item

		SC_WARP_VILLAGE_LIST,
		SC_WARP_VILLAGE,

		SC_RANDOMMIXOPEN,
		SC_RANDOMMIXCLOSE,
		SC_RANDOMMIXCOMPLETECLOSE,
		SC_RANDOMMIXCOMPLETE,

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
		SC_EXCHANGE_POTENTIAL,			// Potential transfer result
#endif			//#ifdef PRE_ADD_EXCHANGE_POTENTIAL

#if defined (PRE_ADD_BESTFRIEND)
		SC_BESTFRIENDREQUEST,
		SC_BESTFRIENDCOMPLETE,
#endif
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
		SC_EXCHANGE_ENCHANT,			// Enhancement transfer
#endif
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
		SC_PCCAFE_RENT_ITEM,			// PC Bang rental item..
#endif // #if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
		SC_POTENTIALJEWEL_ROLLBACK,			//Cancel potential grant
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
#if defined(PRE_ADD_CHAOSCUBE)
		SC_CHAOSGAUGECUBE_ADDSTUFF,
		SC_CHAOSGAUGECUBE_COMPLETE,
		SC_CHAOSGAUGECUBE_LIST,
#endif
	};
};

#if defined( PRE_ADD_DWC )
namespace eDWC
{
	enum eCSDWC
	{
		CS_DWC_CREATETEAM = 0,
		CS_DWC_LEAVETEAM = 1,
		CS_DWC_INVITE_MEMBER = 2,
		CS_DWC_INVITE_MEMBER_ACK = 3,
		CS_DWC_GET_TEAMINFO = 4,
		CS_DWC_GET_RANKPAGE = 5,
		CS_DWC_FIND_RANK = 6,
	};

	enum eSCDWC
	{
		SC_DWC_CREATETEAM = 0,
		SC_DWC_LEAVETEAM = 1,
		SC_DWC_DISMISS_TEAM = 2,
		SC_DWC_INVITE_MEMBER_REQ = 3,
		SC_DWC_INVITE_MEMBER_ACK = 4,
		SC_DWC_GET_TEAMINFO = 5,
		SC_DWC_GET_TEAMMEMBER = 6,
		SC_DWC_GET_RANKPAGE = 7,
		SC_DWC_FIND_RANK = 8,
		SC_DWC_UPDATE_MEMBERSTATE = 9,
		SC_DWC_CHANNELINFO = 10,
		SC_DWC_TEAMNAME_INFO = 11,
		SC_DWC_SCORE_INFO = 12,
		SC_DWC_GRADE_INFO = 13,
	};
};
#endif // #if defined( PRE_ADD_DWC )

// NPC ------------------------------------------------------------------------------

namespace eNpc {
	// Main: CS_NPC
	enum eCSNpc
	{
		CS_NPCTALK = 1,
		CS_NPCTALKEND,
	};

	// Main: SC_NPC
	enum eSCNpc
	{
		SC_NPCTALK = 1,
		SC_SHOWWAREHOUSE,			// Open storage window
		SC_OPENCOMPOUND_EMBLEM,		// Open heraldry jewel synthesis window
		SC_OPENUPGRADE_JEWEL,		// Open jewel synthesis window
		SC_OPENMAILBOX,				// Open mailbox
		SC_OPENDISJOINT_ITEM,		// Open disassembly window
		SC_OPENUPGRADE_ITEM,		// Open item enhancement window
		SC_OPENCOMPOUND_ITEM,		// Open item synthesis window
		SC_OPENCASHSHOP,			// Open cash item shop
		SC_OPENGUILDMGRBOX,			// Open guild management dialog
		SC_OPEN_DARKLAIRRANKBOARD,	// Open Dark Lair ranking board
		SC_OPEN_PVPLADDERRANKBOARD,	// Open PvP ladder ranking board
		SC_OPEN_FARMWAREHOUSE,		// Open farm warehouse
		SC_OPEN_MOVIEBROWSER,
		SC_OPENCOMPOUND2_ITEM,		// Open item synthesis window 2
		SC_OPEN_BROWSER,
		SC_OPEN_GLYPH_LIFT,			// Unequip heraldry
		SC_OPEN_INVENTORY,			// Open inventory window
		SC_OPEN_EXCHANGE_ENCHANT,	// Open enhancement transfer
#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
		SC_OPEN_TEXTURE_DIALOG,		// Open texture dialog
		SC_CLOSE_TEXTURE_DIALOG,	// Close texture dialog
#endif
	};
};

// QUEST ----------------------------------------------------------------------------

namespace eQuest {
	// Main: CS_QUEST
	enum eCSQuest
	{
		CS_ALL_QUEST_INFO = 1,
		CS_CANCEL_QUEST,			// Request quest abandonment
		CS_SELECT_QUEST_REWARD,		// Request quest reward
		CS_COMPLETE_CUTSCENE,
		CS_SKIP_CUTSCENE,
		CS_SHORTCUT_QUEST,			// Quest shortcut processing
#if defined(PRE_ADD_REMOTE_QUEST)
		CS_ACCEPT_REMOTE_QEUST,
		CS_COMPLETE_REMOTE_QUEST,
		CS_CANCEL_REMOTE_QUEST,
#endif
	};

	// Main: SC_QUEST
	enum eSCQuest
	{
		// SC_ALL_QUEST_INFO = 1,
		SC_QUESTINFO,
		SC_QUESTCOMPLETEINFO,
		SC_REFRESH_QUEST,
		SC_COMPLETE_QUEST,
		SC_MARKING_COMPLETE_QUEST,
		SC_REMOVE_QUEST,
		SC_CANCEL_QUEST,
		SC_PLAYCUTSCENE,		// Play video
		SC_QUEST_REWARD,		// Open quest reward window
		SC_QUESTRESULT,
		SC_COMPLETE_CUTSCENE,
		SC_SKIP_CUTSCENE,
		SC_SKIPALL_CUTSCENE,
		SC_ASSIGN_PERIODQUEST,	// Assign timed quest
		SC_SCORE_PERIODQUEST,	// Check timed quest score
		SC_NOTICE_PERIODQUEST,	// Timed quest notice
#if defined(PRE_ADD_REMOTE_QUEST)
		SC_GAIN_REMOTE_QUEST,	// Acquire remote quest
		SC_REMOVE_REMOTE_QEUST,	// Delete remote quest
		SC_COMPLETE_REMOTE_QEUST,//Notify remote quest completion
#endif

	};
};

// CHAT -----------------------------------------------------------------------------

namespace eChat 
{
	// CS_CHAT, // Everything related to chat
	enum eCSChat
	{
		CS_CHATMSG = 1,				// Chat
		CS_CHAT_PRIVATE,			// Whisper
		CS_DICE,
		CS_CHAT_CHATROOM,			// Chat room
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
		CS_CHAT_SPAMMER,			// I am a spammer.
#endif
	};

	// SC_CHAT
	enum eSCChat
	{
		SC_CHATMSG = 1,				// Chat-related
		SC_WORLDSYSTEMMSG = 2,		// Handle world-wide system(?) message (e.g. enhance level x+ success, mission x success, etc.)
		SC_NOTICE,					// Removing from chat-related since the number of items grew, hmm
		SC_GUILDCHAT,				// Guild chat
		SC_CANCELNOTICE,			// Cancel slide notice
		SC_SERVERMSG,				// Message sent from server (sends message number from UIString.xml, UIString_Server.xml)
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
		SC_DOORS_GUILDCHAT,				// Guild chat
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	};
};

// ROOM -----------------------------------------------------------------------------

namespace eRoom {
	// CS_ROOM
	enum eCSRoom
	{
		CS_SYNC_WAIT = 1,
		CS_REQUEST_DUNGEONFAILED,
		CS_WARP_DUNGEON,			//The packet name used during dungeon move was 'fail', so just transform into warp
		CS_SELECT_REWARDITEM,
		CS_IDENTIFY_REWARDITEM,
		CS_DLCHALLENGE_RESPONSE,
	};

	// SC_ROOM
	enum eSCRoom
	{
		SC_SYNC_WAIT = 1,
		SC_SYNC_START,
		SC_GENERATION_MONSTER_MSG,
		SC_GATEINFO_MSG,
		SC_DUNGEONCLEAR_MSG,
		SC_CHANGE_GATESTATE_MSG,
		SC_GENERATION_PROP_MSG,
		SC_CHANGE_GAMESPEED,
		SC_DUNGEONHISTORY,
		SC_WARP_DUNGEONCLEAR, // After dungeon clear, when gathered party members press OK, ask the leader where to warp
		SC_GATEEJECTINFO_MSG,
		//
		SC_SYNC_MEMBERINFO,
		SC_SYNC_MEMBERTEAM,
		SC_SYNC_MEMBERDEFAULTPARTS,
		SC_SYNC_MEMBEREQUIP,
		SC_SYNC_MEMBERSKILL,
		SC_SYNC_MEMBER_BREAKINTO,
		SC_SYNC_MEMBERHPSP,
		SC_SYNC_MEMBERBATTLEMODE,
		SC_SYNC_DROPITEMLIST,
		SC_DUNGEONFAILED_MSG,
		SC_OPENDUNGEONLEVEL_MSG,
		SC_SELECT_REWARDITEM,
		SC_DUNGEONCLEAR_SELECTREWARDITEM,
		SC_DUNGEONCLEAR_REWARDITEM,
		SC_DUNGEONCLEAR_REWARDITEMRESULT,
		SC_REFRESH_MEMBER,
		SC_SYNC_DATUMTICK,
		SC_DLDUNGEONCLEAR_MSG,
		SC_DLDUNGEONCLEAR_RANKINFO,
		SC_DUNGEONCLEAR_CANNOTWARP,
		SC_DUNGEONCLEAR_CANNOTWARP_QUEST,
		SC_DUNGEONCLEAR_ENABLE_LEADERWARP,
		SC_DUNGEONCLEAR_IDENTIFYREWARDITEM,
		//Room SeqLevel
		SC_ROOMSYNC_SEQLEVEL,
		SC_SYNC_CHATROOMINFO,
		SC_START_DRAGONNEST,
		SC_DLCHALLENGE_REQUEST,
		SC_DLCHALLENGE_RESPONSE,
		SC_SYNC_DUNGEONTIMEATTACK,
		SC_STOP_DUNGEONTIMEATTACK,
		SC_SPECIAL_REBIRTHITEM,
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		SC_DUNGEONCLEAR_REWARDBOXTYPE,
#endif
#if defined(PRE_MOD_DARKLAIR_RECONNECT)
		SC_DLROUND_INFO,
#endif
#if defined(PRE_ADD_CP_RANK)
		SC_ABYSS_STAGE_CLEAR_BEST,
#endif
	};
};

// SKILL ----------------------------------------------------------------------------

namespace eSkill{
	// CS_SKILL
	enum eCSSkill
	{
		CS_ADDSKILL,
		CS_DELSKILL,
		CS_SKILLLEVELUP_REQ,
		CS_ACQUIRESKILL_REQ,
		CS_UNLOCKSKILL_BY_MONEY_REQ,	// When requesting skill unlock by paying at a skill trainer in town.
		CS_USE_SKILLRESET_CASHITEM_REQ,		// Final request to use skill reset item
		CS_CHANGE_SKILLPAGE,			// Change dual skill tree page
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION)
		CS_RESERVATION_SKILL_LIST_REQ,		// Skill acquisition and level up
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
		CS_LIST_SKILLSET,					// Skill set list
		CS_SAVE_SKILLSET,					// Save skill set
		CS_DELETE_SKILLSET,					// Delete skill set

#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
	};

	// SC_SKILL
	enum eSCSkill
	{
		SC_SKILLLIST,
		SC_ADDSKILL,
		SC_DELSKILL,
		SC_SKILLLEVELUP,
		SC_OTHERPLAYERSKILLLEVELUP,
		SC_PUSHSKILLPOINT,	// Used when sending to client for setsp cheat and skill point correction. #29463
		SC_UNLOCKSKILL,		// Skill unlocked by using skill book.
		SC_ACQUIRESKILL,	// Server granted permission to acquire the skill.
		SC_SKILL_RESET,		// Skill reset.
		SC_UNLOCKSKILL_BY_MONEY,	// Response from server when requesting skill unlock by paying at a skill trainer in town.
		SC_CANUSE_SKILLRESET_CASH_ITEM,	// Query server whether the skill reset item can be used
		SC_USE_SKILLRESET_CASHITEM_RES,	// Skill reset cash item usage result response
		SC_SKILLPAGE_COUNT,				// Number of skill tree pages
		SC_CHANGE_SKILLPAGE_RES,		// Dual skill tree page change response
#if defined( PRE_ADD_SKILL_LEVELUP_RESERVATION)
		SC_RESERVATION_SKILL_LIST_ACK,		// Skill acquisition and level up
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
		SC_LIST_SKILLSET,					// Skill set list
		SC_SAVE_SKILLSET,					// Save skill set
		SC_DELETE_SKILLSET,					// Delete skill set
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
	};
};

// TRADE ----------------------------------------------------------------------------

namespace eTrade{
	// CS_TRADE
	enum eCSTrade
	{
		// Shop
		CS_SHOP_BUY,			// Purchase
		CS_SHOP_SELL,			// Sell
		CS_SHOP_GETLIST_REPURCHASE,
		CS_SHOP_REPURCHASE,
		CS_REPAIR_EQUIP,		// Repair equipped items
		CS_REPAIR_ALL,			// Repair all

		// Mail
		CS_MAILBOX,				// Request mail list
		CS_MAIL_SEND,			// Send mail
		CS_MAIL_READ,			// Read mail
		CS_MAIL_DELETE,			// Delete mail
		CS_MAIL_ATTACH,			// Retrieve money and items attached to mail
		CS_MAIL_ATTACHALL,		// Retrieve money and items attached to mail
		CS_MAIL_READCADGE,		// Read gift request mail

		// Private trade (item exchange)
		CS_EXCHANGE_REQUEST,	// Request trade
		CS_EXCHANGE_ACCEPT,		// Accept trade
		CS_EXCHANGE_ADDITEM,	// Register item
		CS_EXCHANGE_DELETEITEM,	// Unregister item
		CS_EXCHANGE_ADDCOIN,	// Register coin
		CS_EXCHANGE_CONFIRM,	// Confirm
		CS_EXCHANGE_CANCEL,		// Cancel

		// Trading house
		CS_MARKETLIST,				// Request market list
		CS_MYMARKETLIST,			// Request list of items I listed
		CS_MARKET_REGISTER,			// Register market item
		CS_MARKET_INTERRUPT,		// Cancel market listing
		CS_MARKET_BUY,				// Market purchase
		CS_MARKET_CALCULATIONLIST,	// Market settlement list
		CS_MARKET_CALCULATION,		// Market settlement (1 item)
		CS_MARKET_CALCULATIONALL,	// Settle all market items
		CS_MARKET_PETALBALANCE,		// Query Petal balance
		CS_MARKET_PRICE,			// Query market price
		CS_MARKETMINILIST,			// Request specific item from Trade House (Mini Trade House)

		// specialbox
		CS_SPECIALBOX_LIST,			// Special storage request
		CS_SPECIALBOX_ITEMLIST,		// Select/Receive All button clicked (request list of items to receive)
		CS_SPECIALBOX_RECEIVEITEM,	// Receive item

		CS_SHOP_REMOTEOPEN,
	};

	// SC_TRADE
	enum eSCTrade
	{
		// Shop
		SC_SHOP_OPEN,			// Open shop
		SC_SHOP_BUY,			// Purchase
		SC_SHOP_SELL,			// Sell
		SC_SHOP_REBUY,			// Buyback
		SC_SHOP_REPAIR,			// Repair
		SC_SHOP_GETLIST_REPURCHASE,
		SC_SHOP_REPURCHASE,

		// SkillShop
		SC_SKILLSHOP_OPEN,		// Allowed to open skill window by talking to a trainer NPC that unlocks with money.
		//SC_SKILLSHOPLIST, // Send list
		//SC_SKILLSHOP_BUY, // Buy

		// Mail
		SC_MAILBOX,				// Send mail list
		SC_MAIL_SEND,			// Send mail
		SC_MAIL_READ,			// Read mail
		SC_MAIL_DELETE,			// Delete mail
		SC_MAIL_ATTACH,			// Retrieve money and items attached to mail
		SC_MAIL_ATTACHALL,		// Retrieve money and items attached to mail
		SC_MAIL_NOTIFY,			// Notify of incoming mail
		SC_MAIL_READCADGE,		// Read gift request mail

		// Private trade (item exchange)
		SC_EXCHANGE_REQUEST,	// Request trade
		SC_EXCHANGE_REJECT,		// Trade declined
		SC_EXCHANGE_START,		// Start trade
		SC_EXCHANGE_ADDITEM,	// Register item
		SC_EXCHANGE_DELETEITEM,	// Unregister item
		SC_EXCHANGE_ADDCOIN,	// Register coin
		SC_EXCHANGE_CONFIRM,	// Confirm
		SC_EXCHANGE_CANCEL,		// Cancel
		SC_EXCHANGE_COMPLETE,	// Complete

		// Trading house
		SC_MARKETLIST,			// Send market list
		SC_MYMARKETLIST,		// Send my market list
		SC_MARKET_REGISTER,		// Item registration result
		SC_MARKET_INTERRUPT,	// Cancel market listing
		SC_MARKET_BUY,			// Market purchase
		SC_MARKET_CALCULATIONLIST,	// Market settlement list
		SC_MARKET_CALCULATION,		// Market settlement (1 item)
		SC_MARKET_CALCULATIONALL,	// Settle all market items
		SC_MARKET_NOTIFY,		// Market notification
		SC_MARKET_PETALBALANCE,		// Petal notification
		SC_MARKET_PRICE,			// Market price notification
		SC_MARKETMINILIST,			// Request specific item from Trade House (Mini Trade House)

		// specialbox
		SC_SPECIALBOX_LIST,			// Special storage request
		SC_SPECIALBOX_ITEMLIST,		// Select/Receive All button clicked (request list of items to receive)
		SC_SPECIALBOX_RECEIVEITEM,	// Receive item
		SC_SPECIALBOX_NOTIFY,		// Storage notifier
	};
};

// FRIEND ---------------------------------------------------------------------
namespace eFriend
{
	enum eCSFriend
	{
		CS_FRIEND_GROUP_ADD,			//Create friend group
		CS_FRIEND_GROUP_DELETE,			//Delete friend group
		CS_FRIEND_GROUP_STATEUPDATE,	//Change friend group state (currently probably just rename)
		CS_FRIEND_ADD,					//Add friend
		CS_FRIEND_DELETE,				//Delete friend
		CS_FRIEND_STATEUPDATE,			//Change friend state (currently probably group change)
		CS_FRIEND_INFO,					//Request details of selected friend
		CS_FRIEND_REQ_LIST,				//Request friend list (with groups) (check related parts if expanded only once initially)
		CS_FRIEND_REQ_LOCATION,			//Request friend location update
	};

	enum eSCFriend
	{
		SC_FRIEND_GROUP_LIST,			//Group list goes out first.
		SC_FRIEND_LIST,					//Changed to client request timing
		SC_FRIEND_GROUP_ADDED,
		SC_FRIEND_ADDED,
		SC_FRIEND_GROUP_DELETED,
		SC_FRIEND_ADDED_DELETED,
		SC_FRIEND_GROUP_UPDATED,
		SC_FRIEND_UPDATED,
		SC_FRIEND_INFO,					//Friend's detailed info
		SC_FRIEND_RESULT,
		SC_FRIEND_ADDNOTICE,
		SC_FRIEND_LOCATIONLIST,
	};
};


// ISOLATE -----------------------------------------------------------------------------
namespace eIsolate
{
	enum eCSIsolate
	{
		CS_ISOLATE_GET,
		CS_ISOLATE_ADD,
		CS_ISOLATE_DELETE,
	};

	enum eSCIsolate
	{
		SC_ISOLATE_LIST,
		SC_ISOLATE_ADD,
		SC_ISOLATE_DELETE,
		SC_ISOLATE_RESULT,	//message
	};
};

// GUILD -----------------------------------------------------------------------------
namespace eGuild
{
	enum eCSGuild			// CS_GUILD
	{
		CS_CREATEGUILD,				// Request guild creation
		CS_DISMISSGUILD,			// Guild disband request
		CS_INVITEGUILDMEMBREQ,		// Guild member invite request
		CS_INVITEGUILDMEMBACK,		// Guild member invite response
		CS_LEAVEGUILDMEMB,			// Request to leave guild
		CS_EXILEGUILDMEMB,			// Request to kick guild member
		CS_CHANGEGUILDINFO,			// Request guild info change
		CS_CHANGEGUILDMEMBINFO,		// Guild member info update request
		CS_GETGUILDHISTORYLIST,		// Guild history list request
		CS_GETGUILDINFO,			// Request guild info
		CS_OPEN_GUILDWARE,			// Open guild storage
		CS_GET_GUILDWARE_HISTORY,	// Guild warehouse history
		CS_CLOSE_GUILDWARE,			// Close guild storage
		CS_ENROLL_GUILDWAR,			// Apply to join Guild War
		CS_GUILDWAR_VOTE,			// Guild war popularity vote
		CS_GUILDWAR_COMPENSATION,	// Claim Guild War preliminary reward
		CS_GUILDWAR_WIN_SKILL,		// Use (cast) guild war winner skill
		CS_GUILD_BUY_REWARDITEM,		// Purchase guild reward item
		CS_PLAYER_REQUEST_GUILDINFO,	// Guild info for inspection
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
		CS_GUILD_CONTRIBUTION_RANK,	//Request guild contribution ranking lookup
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	};
	enum eSCGuild			// SC_GUILD
	{
		SC_CREATEGUILD,				// Guild creation result
		SC_DISMISSGUILD,			// Guild disband result
		SC_INVITEGUILDMEMBREQ,		// Guild member invite request
		SC_INVITEGUILDMEMBACK,		// Guild member invite result
		SC_LEAVEGUILDMEMB,			// Leave guild result
		SC_EXILEGUILDMEMB,			// Kick guild member result
		SC_CHANGEGUILDINFO,			// Guild info change result
		SC_CHANGEGUILDMEMBINFO,		// Guild member info update result
		SC_GETGUILDHISTORYLIST,		// Guild history list response
		SC_GETGUILDINFO,			// Guild info response
		SC_CHANGEGUILDSELFVIEW,		// Guild visual info (personal/group) change notification
		SC_GUILDMEMBLOGINLIST,		// Guild member login list notification
		SC_OPEN_GUILDWARE,			// Open guild storage result
		SC_GET_GUILDWARE_HISTORY,	// Guild warehouse history result
		SC_CHANGEGUILDNAME,			// Change guild name
		SC_EXTEND_GUILDWARESIZE,	// Expand guild storage size
		SC_CHANGEGUILDMARK,			// Change guild mark
		SC_CHANGE_WAREVENT,			// Guild war event notice
		SC_ENROLL_GUILDWAR,			// Guild war participation application result		
		// Receive data from server and open the window
		SC_GUILDWAR_STATUS_OPEN,			// Guild war preliminary progress
		SC_GUILDWAR_TRIAL_RESULT_OPEN,		// Guild war preliminary result
		SC_GUILDWAR_TOURNAMENT_INFO_OPEN,	// Guild war tournament (bracket) info
		SC_GUILDWAR_VOTE,					// Guild war popularity vote result		
		SC_GUILDWAR_COMPENSATION,			// Guild war preliminary reward claim result
		SC_GUILDWAR_FESTIVAL_POINT,			// Guild War festival points
		SC_GUILDWAR_VOTE_TOP,				// 1st place in the previous guild war popularity vote
		SC_GUILDWAR_PRE_WIN,				// Winning guild of the previous guild war (true or false)
		SC_GUILDWAR_WIN_SKILL,				// Guild war winner skill use result (for caster)
		SC_GUILDWAR_USER_WINSKILL,			// Winning guild uses winner skill (for nearby players)
		SC_GUILDWAR_EVENT_TIME,				// Guild war schedule notice.
		SC_GUILDWAR_TOURNAMENT_WIN,			// Notify Guild War finals winning guild.
		SC_GUILDWAR_TRIAL_RESULT_UIOPEN,		// Guild war preliminary result
		SC_GUILDWAR_TOURNAMENT_INFO_UIOPEN,	// Guild war tournament (bracket) info
		SC_GUILDWAR_BUYED_ITEM_COUNT,		// Item count purchased during guild war reward period.
		SC_GUILDWAR_BUYED_ITEM_COUNT_LIST,	// List of item counts purchased during guild war reward period.
		SC_GUILDLEVELUP,			// Guild level up
		SC_UPDATEGUILDEXP,			// Guild update EXP
		SC_GUILD_GET_REWARDITEM,	// Send applied guild reward item list
		SC_GUILD_BUY_REWARDITEM,	// Purchase guild reward item		
		SC_GUILD_ADD_REWARDITEM,	// Add guild reward item effect		
		SC_GUILD_EXTEND_GUILDSIZE,	// Guild: increase max member count		
		SC_PLAYER_REQUEST_GUILDINFO,	// Guild info for inspection
		SC_GET_GUILDMEMBER,			// Send guild member info
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
		SC_GUILD_CONTRIBUTION_POINT, //Guild contribution (personal)
		SC_GUILD_CONTRIBUTION_RANK,	//Send guild contribution ranking
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	};
};

// PvP --------------------------------------------------------------------------
namespace ePvP
{
	enum eCSPvP
	{
		CS_CREATEROOM,				// Create PvP room
		CS_MODIFYROOM,				// Modify PvP room
		CS_LEAVEROOM,				// Leave PvP room
		CS_CHANGECAPTAIN,			// Change PVP room master
		CS_ROOMLIST,				// PVP room list
		CS_JOINROOM,				// Enter PVP room
		CS_READY,					// PVP room ready
		CS_START,					// Start PVP room
		CS_RANDOMJOINROOM,			// Random join PvP room
		CS_CHANGETEAM,				// Change PvP team
		CS_BAN,						// Kick from PvP room
		CS_WAITUSERLIST,			// PvP lobby user list

		// Ladder system related protocols
		CS_ENTER_LADDERCHANNEL,
		CS_LEAVE_LADDERCHANNEL,
		CS_LADDER_MATCHING,
		CS_LADDER_PLAYING_ROOMLIST,
		CS_LADDER_OBSERVER,
		CS_LADDER_INVITE,
		CS_LADDER_INVITE_CONFIRM,
		CS_LADDER_KICKOUT,
		CS_FATIGUE_OPTION,			// PvP fatigue consumption On/Off
		CS_SWAPMEMBERINDEX,
		CS_GUILDWAR_CHANGEMEMBER_GRADE,
		CS_TRYCAPTURE,				// Attempt to capture occupation war territory
		CS_CANCELCAPTURE,			// Cancel attempt
		CS_TRYACQUIRE_SKILL,		// Attempt to acquire occupation war skill
		CS_INITSKILL,				// Reset occupation war skills
		CS_USESKILL,				// Use occupation war skill
		CS_SWAPSKILLINDEX,			// Change occupation war skill slot position
		CS_CONCENTRATE_ORDER,		// Guild master rally command
#if defined(PRE_ADD_QUICK_PVP)
		CS_QUICKPVP_INVITE,			// Duel request		
		CS_QUICKPVP_RESULT,			// Duel request response
#endif
		CS_PVP_ALLKILL_SELECTPLAYER,		
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
		CS_PVP_LIST_OPEN_UI,		// Request to open PVP room list (level check)
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		CS_PVP_CHANGECHANNEL,
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined(PRE_ADD_PVP_TOURNAMENT)
		CS_PVP_SWAP_TOURNAMENT_INDEX,	// Swap PvP tournament slots (not allowed when Random)
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
		CS_PVP_WORLDPVPROOM_STARTMSG,	// Cross-world battlefield start message
#endif
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
		CS_PVP_COMBOEXERCISE_RECALLMONSTER,	// Summon monster
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
	};

	enum eSCPvP
	{
		SC_CREATEROOM,				// Create PvP room
		SC_MODIFYROOM,				// Modify PvP room
		SC_LEAVEROOM,				// Leave PVP room
		SC_ROOMLIST,				// PVP room list
		SC_JOINROOM,				// Enter PVP room
		SC_JOINUSER,				// User joined PVP room (send JOIN user info to users already in room)
		SC_CHANGETEAM,				// Change PvP team
		SC_USERSTATE,				// User state
		SC_START,					// Start PvP
		SC_STARTMSG,				// PvP start message
		SC_ROOMSTATE,				// Room state
		SC_MODE_STARTTICK,			// PvPMode start tick
		SC_FINISH_PVPMODE,			// PvP end
		SC_START_PVPROUND,			// PvP round start
		SC_FINISH_PVPROUND,			// PvP round end
		SC_MODE_SCORE,				// PvPMode score
		SC_RESPAWN_POINT,			// PvP respawn point
		SC_XPSCORE,					// PvP XP score
		SC_BREAKINTO_SUCCESS,		// Drop-in succeeded
		SC_ROOMINFO,
		SC_ADDPOINT,
		SC_LEVEL,					// PvP rank
		SC_SELECTCAPTAIN,			// Captain selected in PvP Captain mode
		SC_CAPTAINKILL_COUNT,		// Send captain kill info
		SC_WAITUSERLIST,				// PvP user list
		SC_SELECTZOMBIE,			// Zombie selected in PvP Zombie mode
		SC_ZOMBIEKILL_COUNT,		// Send zombie kill info
		SC_OCCUPATIONSCORE,

		// Ladder system related protocols
		SC_ENTER_LADDERCHANNEL,
		SC_LEAVE_LADDERCHANNEL,
		SC_LADDER_NOTIFY_LEAVEUSER,
		SC_LADDER_NOTIFY_LEADER,
		SC_LADDER_NOTIFY_ROOMSTATE,
		SC_LADDER_MATCHING,
		SC_LADDER_NOTIFY_GAMEMODE_TABLEID,
		SC_LADDER_PLAYING_ROOMLIST,
		SC_LADDER_OBSERVER,
		SC_LADDER_SCOREINFO,
		SC_LADDER_SCOREINFO_BYJOB,
		SC_LADDER_MATCHING_AVGSEC,
		SC_LADDER_POINT_REFRESH,
		SC_LADDER_INVITE,
		SC_LADDER_INVITE_CONFIRM_REQ,
		SC_LADDER_INVITE_CONFIRM,
		SC_LADDER_NOTIFY_JOINUSER,
		SC_LADDER_REFRESH_USERINFO,
		SC_LADDER_KICKOUT,
		SC_FATIGUE_REWARD,			// Reward through PvP fatigue consumption
		SC_PVP_MEMBERINDEX,
		SC_PVP_MEMBERGRADE,
		SC_PVP_OCCUPATION_MODESTATE,
		SC_PVP_OCCUPATION_POINTSTATE,
		SC_PVP_OCCUPATION_TEAMSTATE,
		SC_PVP_OCCUPATION_SKILLSTATE,
		SC_PVP_TRYACQUIREPOINT,
		SC_PVP_TRYACQUIRESKILL,
		SC_PVP_INITSKILL,
		SC_PVP_USESKILL,
		SC_PVP_SWAPSKILLINDEX,
		SC_CONCENTRATE_ORDER,
		SC_PVP_GHOULSCORES,
		SC_PVP_HOLYWATER_KILLCOUNT, //Add KILL count due to holy water usage.
		SC_PVP_CLIMAXMODE,

#if defined(PRE_ADD_QUICK_PVP)
		SC_QUICKPVP_INVITE,			// Duel request		
		SC_QUICKPVP_RESULT,			// Duel request response
#endif
		SC_PVP_ALLKILL_SHOW_SELECTPLAYER,
		SC_PVP_ALLKILL_GROUPCAPTAIN,
		SC_PVP_ALLKILL_ACTIVEPLAYER,
		SC_PVP_ALLKILL_CONTINUOUSWIN,
		SC_PVP_ALLKILL_FINISH_DETAILREASON,
		SC_PVP_ALLKILL_BATTLEPLAYER,
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
		SC_PVP_LIST_OPEN_UI,		// Request to open PVP room list (level check)
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
		SC_PVP_CHANGE_CHANNEL,
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined(PRE_ADD_RACING_MODE)
		SC_PVP_RACING_RAPTIME,					// PvP Racing lap time
		SC_PVP_RACING_FIRST_END,				// PvP 1st place has entered.
#endif
#if defined(PRE_ADD_REVENGE)
		SC_PVP_SET_REVENGE_TARGET,
		SC_PVP_SUCCESS_REVENGE,
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
		SC_PVP_SWAP_TOURNAMENT_INDEX,	// Swap PvP tournament slots (not allowed when Random)
		SC_PVP_TOURNAMENT_MATCHLIST,	// PvP tournament bracket info
		SC_PVP_TOURNAMENT_DEFAULTWIN,	// PvP tournament bye
		SC_PVP_TOURNAMENT_IDLE_TICK,	// PvP tournament wait tick
		SC_PVP_TOURNAMENT_TOP4,			// PvP tournament up to 4th place.
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
		SC_PVP_WORLDPVPROOM_STARTMSG,	// Cross-world battlefield start message
		SC_PVP_WORLDPVPROOM_START_RESULT,	// Cross-world battlefield start result
		SC_PVP_WORLDPVPROOM_JOIN_RESULT,	// Cross-world battlefield join result (sent on failure)
		SC_PVP_WORLDPVPROOM_ALLKILLTEAMINFO,	// Cross-world battlefield all-kill match team info
		SC_PVP_WORLDPVPROOM_TOURNAMENTUSERINFO,	// Cross-world battlefield tournament user info
#endif
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
		SC_PVP_COMBOEXERCISE_ROOMMASTERINFO, // Room master info (on start)
		SC_PVP_COMBOEXERCISE_CHANGEROOMMASTER, // Change room master
		SC_PVP_COMBOEXERCISE_RECALLMONSTERRESULT,	// Monster summon result
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
	};
};

// Trigger --------------------------------------------------------------------------
namespace eTrigger
{
	enum eCSTrigger
	{
	};
	enum eSCTrigger
	{
		SC_TRIGGER_CALLACTION,
		SC_CHANGE_MY_BGM,
		SC_RADIO_IMAGE,
		SC_FILE_TABLE_BGM_OFF,
		SC_TRIGGER_FORCE_ENABLE_RIDE,
	};
};

//-----------------------------------------------------------------------------------

// Mission --------------------------------------------------------------------------
namespace eMission
{
	enum eCSMission
	{
#if defined(PRE_ADD_CHAT_MISSION)
		CS_CHAT_MISSION,			// Chat mission completion packet
#endif
	};

	enum eSCMission
	{
		SC_MISSION_LIST,
		SC_GAIN_MISSION,
		SC_ACHIEVE_MISSION,

		SC_DAILYMISSION_LIST,
		SC_COUNT_DAILYMISSION,
		SC_ACHIEVE_DAILYMISSION,

		SC_ALARM_DAILYMISSION,
		SC_EVENT_POPUP,
		SC_CONNECTING_TIME,
		SC_HELP_ALARM,
#if defined(PRE_ADD_ACTIVEMISSION)
		SC_GAIN_ACTIVE_MISSION,
		SC_ACHIEVE_ACTIVE_MISSION,
#endif
	};
};
//-----------------------------------------------------------------------------------


// Appellation ----------------------------------------------------------------------
namespace eAppellation
{
	enum eCSAppellation
	{
		CS_SELECT_APPELLATION,
		CS_COLLECTIONBOOK,
	};
	enum eSCAppellation
	{
		SC_APPELLATION_LIST = 128,
		SC_GAIN_APPELLATION,
		SC_SELECT_APPELLATION, 	// Select local title
		SC_CHANGE_APPELLATION,	// When another player's title changes
		SC_PERIOD_APPELLATION,	// Timed title duration
		SC_RESET_PCBANGAPPELLATION,	//Notify PC bang title reset
	};
};

//-----------------------------------------------------------------------------------

// GameOption -----------------------------------------------------------------------
// CS_GAMEOPTION

namespace eGameOption
{
	enum eCSGameOption
	{
		CS_GAMEOPTION_UPDATEOPTION,
		CS_GAMEOPTION_REQCOMMOPTION,
		CS_GAMEOPTION_QUEST_NOTIFIER,
		CS_GAMEOPTION_MISSION_NOTIFIER,
		CS_GAMEOPTION_UPDATE_KEYSETTING,
		CS_UPDATE_SECONDAUTH_PASSWORD,
		CS_UPDATE_SECONDAUTH_LOCK,
		CS_GAMEOPTION_VALIDATE_SECONDAUTH,
		CS_INIT_SECONDAUTH,
		CS_GAMEOPTION_UPDATE_PADSETTING,
		CS_GAMEOPTION_GET_PROFILE,
		CS_GAMEOPTION_SET_PROFILE,
	};

	enum eSCGameOption
	{
		SC_GAMEOPTION_USERCOMMOPTION,
		SC_GAMEOPTION_REFRESH_NOTIFIER,
		SC_GAMEOPTION_SELECT_KEYSETTING,
		SC_NOTIFY_SECONDAUTHINFO,
		SC_UPDATE_SECONDAUTH_PASSWORD,
		SC_UPDATE_SECONDAUTH_LOCK,
		SC_NOTIFY_SECONDAUTH_RESTRAINT,			// Notify that it is restricted due to lock state.
		SC_GAMEOPTION_VALIDATE_SECONDAUTH,
		SC_INIT_SECONDAUTH,
		SC_GAMEOPTION_SELECT_PADSETTING,
		SC_GAMEOPTION_GET_PROFILE,
		SC_GAMEOPTION_SET_PROFILE,
		SC_GAMEOPTION_DISPLAY_PROFILE,
	};
};

// RadioMessage ---------------------------------------------------------------------
// CS_RADIO
namespace eRadio
{
	enum eCSRadio
	{
		CS_USERADIO,
	};

	enum eSCRadio
	{
		SC_USERADIO,
	};
};

//Gesture ---------------------------------------------------------------------------
namespace eGesture
{
	enum eCSGesture
	{
		CS_USEGESTURE,
	};

	enum eSCGesture
	{
		SC_CASHGESTURELIST,
		SC_CASHGESTUREADD,
		SC_USEGESTURE,
		SC_EFFECTITEMGESTURELIST,
		SC_EFFECTITEMGESTUREADD,
	};
};

//VoiceChat -------------------------------------------------------------------------
namespace eVoiceChat
{	//CS_VOICECHAT
	enum eCSVoiceChat
	{
		CS_VOICEAVAILABLE,		//Availability / Whether possible
		CS_VOICEMUTE,
		CS_VOICECOMPLAINTREQ,
	};

	enum eSCVoiceChat
	{
		SC_VOICECHATINFO,					// dolbyserver
		SC_TALKINGINFO,
		SC_VOICEMEMBERINFO,
	};
};

//Restraint -------------------------------------------------------------------------
namespace eRestraint
{
	enum eCSRestraint
	{
	};

	enum eSCRestraint
	{
		SC_RESTRAINTADD,
		SC_RESTRAINTDEL,
		SC_RESTRAINTRETCODE,
		SC_RESTRAINTLIST,
	};
};

// CashShop -------------------------------------------------------------------------
namespace eCashShop
{
	// CS_CASHSHOP
	enum eCSCashShop
	{
		CS_SHOPOPEN,		// Open shop
		CS_SHOPCLOSE,		// Close shop
		CS_BUY,				// Purchase
		CS_PACKAGEBUY,		// Purchase package item
		CS_BALANCEINQUIRY,	// Query cash, petal balance
		CS_CHECKRECEIVER,	// Check recipient (needed when gifting)
		CS_GIFT,			// Send gift
		CS_PACKAGEGIFT,		// Gift package
		CS_GIFTLIST,		// Request gift box (received gifts)
		CS_RECEIVEGIFT,		// Receive gift
		CS_RECEIVEGIFTALL,	// Receive gift

		CS_COUPON,			// Enter coupon
		CS_VIPBUY,							// VIP item purchase
		CS_VIPGIFT,						// VIP item purchase

		CS_GIFT_RETURN,		// Return gift (Taiwan)

#if defined(PRE_ADD_CASH_REFUND)
		CS_MOVE_CASHINVEN,	// Request to move to cash inventory
		CS_CASH_REFUND,		// Cash refund request
#endif
		CS_CHARGE_TIME,
		CS_EQUIPCRC,
#ifdef PRE_ADD_LIMITED_CASHITEM
		CS_LIMITEDCASHITEM,	// Quantity-limited cash item info
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
		CS_CADGE,			// Wish request
		CS_GIFTLIST_BY_SHORTCUT,		// Request gift box (received gifts) (gift box indicator icon clicked)
		CS_SECRETCASHSHOP_OPEN,			// Open secret cash shop
		CS_SECRETCASHSHOP_CLOSE,		// Close secret cash shop
	};

	// SC_CASHSHOP
	enum eSCCashShop
	{
		SC_SHOPOPEN,		// Open shop
		SC_SHOPCLOSE,		// Close shop
		SC_BUY,				// Purchase
		SC_PACKAGEBUY,		// Purchase package item
		SC_BALANCEINQUIRY,	// Query cash, petal balance
		SC_CHECKRECEIVER,	// Check recipient (needed when gifting)
		SC_GIFT,			// Send gift
		SC_PACKAGEGIFT,		// Gift package
		SC_GIFTLIST,		// Gift box (received gifts)
		SC_RECEIVEGIFT,		// Receive gift
		SC_RECEIVEGIFTALL,		// Receive gift

		SC_COUPON,			// Enter coupon
		SC_NOTIFYGIFT,		// Gift notification

		SC_VIPBUY,							// Send VIP information
		SC_VIPGIFT,							// Send VIP information
		SC_VIPINFO,							// When looking for VIP in gift box

		SC_SALEABORTLIST,	// Discontinued sales list
		SC_GIFTRETURN,		// Return gift (Taiwan)

#if defined(PRE_ADD_CASH_REFUND)
		SC_PAYMENT_LIST,			// Single-item payment list
		SC_PAYMENT_PACKAGELIST,		// Package payment list
		SC_MOVE_CASHINVEN,			// Result of moving to cash inventory
		SC_CASH_REFUND,				// Cash refund result	
#endif
		SC_CHARGE_TIME,
		SC_EQUIPCRC,
#ifdef PRE_ADD_LIMITED_CASHITEM
		SC_LIMITEDCASHITEM,			// Quantity-limited cash item info
		SC_CHANGEDLIMITEDCASHITEM,	// Change quantity limit
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
		SC_CADGE,			// Wish request
		SC_GIFTLIST_BY_SHORTCUT,		// Request gift box (received gifts) (gift box indicator icon clicked) (server sends packet via existing sC_GIFTLIST..)
		SC_SECRETCASHSHOP_NOTIFY,		// Notify available secret cash shop ID
		SC_SECRETCASHSHOP_OPEN,			// Secret cash shop open response
	};
};

// ChatRoom -------------------------------------------------------------------------
namespace eChatRoom
{
	enum eCSChatRooom
	{
		CS_CREATECHATROOM,			// Create chat room
		CS_ENTERCHATROOM,			// Enter chat room
		CS_CHANGEROOMOPTION,		// Change chat room options
		CS_KICKUSER,				// Kick user
		CS_LEAVEROOM,				// Leave the chat room
	};

	enum eSCChatRoom
	{
		SC_CREATECHATROOM,			// Create chat room
		SC_ENTERCHATROOM,			// Enter chat room
		SC_ENTERUSERCHATROOM,		// Another user entered the chat room
		SC_CHANGEROOMOPTION,		// Change chat room options
		SC_KICKUSER,				// Kick user
		SC_LEAVEUSER,				// User left the chat room (left, kicked, etc.)

		SC_CHATROOMVIEW,			// Create/delete chat room (delete if ChatRoomID == 0)
	};
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

namespace eReputation
{
	enum eCSReputation
	{
		CS_GIVENPCPRESENT,
	};

	enum eSCReputation
	{
		SC_REPUTATIONLIST,
		SC_MODREPUTATION,
		SC_OPENGIVENPCPRESENT,
		SC_REQUESTSENDSELECTEDPRESENT,
		SC_SHOWNPCEFFECT,
		SC_UNIONPOINT,
		SC_UNIONMEMBERSHIPLIST,
		SC_BUYUNIONMEMBERSHIPRESULT,
	};
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

namespace eMasterSystem
{
	enum eCSMasterSystem
	{
		CS_MASTERLIST,			// Master list window info
		CS_MASTERCHARACTERINFO,	// Master info
		CS_INTRODUCTION_ONOFF,	// Register/unregister introduction
		CS_MASTER_APPLICATION,	// Master application
		CS_JOIN,				// Join master-apprentice
		CS_MASTERANDCLASSMATE,	// Master and alumni list
		CS_CLASSMATEINFO,		// Alumni info
		CS_MYMASTERINFO,		// My master info (apprentice)
		CS_LEAVE,				// Break master-apprentice relationship
		CS_INVITE_PUPIL,		// Invite apprentice
		CS_INVITE_PUPIL_CONFIRM,// Apprentice invite response
		CS_JOIN_DIRECT,			// Join as master
		CS_JOIN_DIRECT_CONFIRM,
		CS_RECALL_MASTER,		// Summon master
		CS_BREAKINTO_PUPIL,		// Move to apprentice channel
		CS_JOIN_CONFIRM,		// Whether master-apprentice join is accepted
	};

	enum eSCMasterSystem
	{
		SC_SIMPLEINFO,			// Summary info 
		SC_MASTERLIST,			// Master list page
		SC_MASTERCHARACTERINFO,	// Master info
		SC_PUPILLIST,			// Apprentice list
		SC_INTRODUCTION_ONOFF,	// Register/unregister introduction
		SC_MASTER_APPLICATION,	// Master application
		SC_JOIN,				// Join master-apprentice
		SC_MASTERANDCLASSMATE,	// Master and alumni list
		SC_CLASSMATEINFO,		// Alumni info
		SC_MYMASTERINFO,		// My master info (apprentice)	
		SC_LEAVE,				// Break master-apprentice relationship
		SC_GRADUATE,			// Graduation
		SC_COUNTINFO,			// 
		SC_RESPECTPOINT,
		SC_FAVORPOINT,
		SC_INVITE_PUPIL,
		SC_INVITE_PUPIL_CONFIRM,// Confirm apprentice invite
		SC_JOIN_DIRECT,
		SC_JOIN_DIRECT_CONFIRM,
		SC_RECALL_MASTER,
		SC_CONNECT,
		SC_JOIN_COMFIRM,		// Whether master-apprentice join is accepted
	};
}

#if defined( PRE_ADD_SECONDARY_SKILL )

namespace eSecondarySkill
{
	enum eCSSecondarySkill
	{
		CS_DELETE,
		CS_ADD_RECIPE,
		CS_DELETE_RECIPE,
		CS_EXTRACT_RECIPE,
		CS_MANUFACTURE,
		CS_CANCEL_MANUFACTURE,
	};

	enum eSCSecondarySkill
	{
		SC_CREATE,
		SC_DELETE,
		SC_UPDATE_EXP,
		SC_LIST,
		SC_ADD_RECIPE,
		SC_DELETE_RECIPE,
		SC_EXTRACT_RECIPE,
		SC_UPDATE_RECIPE_EXP,
		SC_RECIPE_LIST,
		SC_MANUFACTURE,
		SC_CANCEL_MANUFACTURE,
	};
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

namespace eEtc
{
	enum eCSEtc
	{
		CS_DARKLAIR_RANK_BOARD,
		CS_PVPLADDER_RANK_BOARD,
		CS_WINDOW_STATE,
#if defined(PRE_ADD_PVP_RANKING)
		CS_PVP_RANK_BOARD, // My info
		CS_PVP_RANK_LIST,  // Page info (guild search)
		CS_PVP_RANK_INFO,  // Search by character name.
		CS_PVP_LADDER_RANK_BOARD,
		CS_PVP_LADDER_RANK_LIST,
		CS_PVP_LADDER_RANK_INFO,
#endif
#if defined( PRE_ADD_GAMEQUIT_REWARD )
		CS_GAMEQUIT_REWARDCHECK_REQ,	//Request to check eligibility for exit reward
		CS_GAMEQUIT_REWARD_REQ,			//Exit reward request
#endif	//	#if defined( PRE_ADD_GAMEQUIT_REWARD )
	};

	enum eSCEtc
	{
		SC_DARKLAIR_RANK_BOARD,
		SC_PVPLADDER_RANK_BOARD,
#if defined(PRE_ADD_PVP_RANKING)
		SC_PVP_RANK_BOARD,
		SC_PVP_RANK_LIST,
		SC_PVP_RANK_INFO,
		SC_PVP_LADDER_RANK_BOARD,
		SC_PVP_LADDER_RANK_LIST,
		SC_PVP_LADDER_RANK_INFO,
#endif
#if defined( PRE_ADD_GAMEQUIT_REWARD )
		SC_GAMEQUIT_REWARDCHECK_RES,	//Response on whether eligible for exit reward
		SC_GAMEQUIT_REWARD_RES,			//Exit reward response
#endif	//	#if defined( PRE_ADD_GAMEQUIT_REWARD )
#if defined( PRE_DRAGONBUFF )
		SC_WORLDBUFFMSG,
#endif
	};
}

namespace eGuildRecruit
{
	enum eCSGuildRecruit
	{
		CS_GUILDRECRUIT_LIST,			// Request guild list registered on guild board (default) ---		
		CS_GUILDRECRUIT_CHARACTER,		// Request list of join applications to own guild (default) -----
		CS_GUILDRECRUIT_MYLIST,			// Request list of guilds applied to join (default) ----
		CS_GUILDRECRUIT_REQUESTCOUNT,	// Number of times applied to join guilds
		CS_GUILDRECRUIT_REGISTERINFO,	// Check content registered on guild recruitment board --
		CS_GUILDRECRUIT_REGISTER,		// Register/unregister guild on recruitment board ----
		CS_GUILDRECRUIT_REQUEST,		// Guild join application/cancellation via recruitment board --
		CS_GUILDRECRUIT_ACCEPT,			// Approve join request
	};
	enum eSCGuildRecruit
	{
		SC_GUILDRECRUIT_LIST,			// Send guild list registered on guild board --
		SC_GUILDREQUEST_CHARACTER,		// Send list of join applications to own guild --
		SC_GUILDRECRUIT_MYLIST,			// Request list of guilds applied to join --
		SC_GUILDRECRUIT_REQUESTCOUNT,	// Number of times applied to join guilds
		SC_GUILDRECRUIT_REGISTERINFO,	// Check content registered on guild recruitment board --
		SC_GUILDRECRUIT_REGISTER,		// Result of registering/unregistering guild on recruitment board --
		SC_GUILDRECRUIT_REQUEST,		// Result of guild join application/cancellation via recruitment board --
		SC_GUILDRECRUIT_ACCEPRESULT,	// Join approval result (sent to the guild member who requested approval/rejection)
		SC_GUILDRECRUIT_MEMBERRESULT,	// Join approval result (sent to the user who applied to join)
	};
}

#if defined (PRE_ADD_DONATION)
namespace eDonation
{
	enum eCSDonation
	{
		CS_DONATE,
		CS_DONATION_RANKING,
	};

	enum eSCDonation
	{
		SC_DONATION_OPEN,
		SC_DONATE,
		SC_DONATION_RANKING,
		SC_DONATION_TOP_RANKER,
	};
}
#endif // #if defined (PRE_ADD_DONATION)

#if defined (PRE_ADD_BESTFRIEND)


namespace eBestFriend
{
	enum eCSBestFriend
	{
		CS_GETINFO,
		CS_SEARCH,
		CS_REGIST,
		CS_ACCEPT,
		CS_EDITMEMO,
		CS_CANCELBF,
	};

	enum eSCBestFriend
	{
		SC_GETINFO,
		SC_SEARCH,
		SC_REGISTREQ,
		SC_REGISTACK,
		SC_COMPLETE,
		SC_EDITMEMO,
		SC_CANCELBF,
		SC_CLOSEBF,
	};
}
#endif // #if defined (PRE_ADD_BESTFRIEND)

#ifdef PRE_ADD_DOORS
namespace eDoors
{
	enum eCSDoors
	{
		CS_GET_AUTHKEY,
		CS_CANCEL_AUTH,
		CS_GET_MOBILEAUTHFLAG,
	};

	enum eSCDoors
	{
		SC_GET_AUTHKEY,
		SC_CANCEL_AUTH,
		SC_GET_MOBILEAUTHFLAG,
	};
};
#endif		//#ifdef PRE_ADD_DOORS

#if defined( PRE_PRIVATECHAT_CHANNEL )
// CS_PRIVATECHAT_CHANNEL
namespace ePrivateChatChannel
{
	enum eCSPrivateChatChannel
	{
		CS_PRIVATECHAT_CHANNEL_ADD,	    // Create private chat channel
		CS_PRIVATECHAT_CHANNEL_JOIN,	// Join private chat channel
		CS_PRIVATECHAT_CHANNEL_INVITE,	// Invite to private chat channel
		CS_PRIVATECHAT_CHANNEL_INVITEREQ,// Private chat channel invite result
		CS_PRIVATECHAT_CHANNEL_OUT,		// Leave private chat channel
		CS_PRIVATECHAT_CHANNEL_MOD,		// Modify private chat channel info
		CS_PRIVATECHAT_CHANNEL_KICK,	// Kick from private chat channel
	};

	enum eSCPrivateChatChannel
	{
		SC_PRIVATECHAT_CHANNEL_INFO,			// Private chat channel info		
		SC_PRIVATECHAT_CHANNEL_JOINRESULT,		// Join private chat channel result
		SC_PRIVATECHAT_CHANNEL_INVITERESULT,	// Private chat channel invite result
		SC_PRIVATECHAT_CHANNEL_OUTRESULT,		// Leave private chat channel result
		SC_PRIVATECHAT_CHANNEL_KICKRESULT,		// Private chat channel kick result
		SC_PRIVATECHAT_CHANNEL_ADD,				// Private chat channel joined member info
		SC_PRIVATECHAT_CHANNEL_DEL,				// Private chat channel departed member info
		SC_PRIVATECHAT_CHANNEL_MOD,				// Change private chat channel master
		SC_PRIVATECHAT_CHANNEL_KICK,			// Private chat channel kicked member info
		SC_PRIVATECHAT_CHANNEL_RESULT,			// Private channel chat request result (result for CS message)
	};
};
#endif //#if defined( PRE_PRIVATECHAT_CHANNEL )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
namespace eAlteiaWorld
{
	enum eCSAlteiaWorld
	{
		CS_ALTEIAWORLD_INFO,		// Altea info
		CS_ALTEIAWORLD_JOIN,	    // Altea join
		CS_ALTEIAWORLD_SENDTICKET,  // Gift Altea entry ticket
		CS_ALTEIAWORLD_DICE,		// Altea dice roll
		CS_ALTEIAWORLD_NEXTMAP,		// Move to next Altea map
		CS_ALTEIAWORLD_QUITINFO,	// Exit Altea info window
	};

	enum eSCAlteiaWorld
	{
		SC_ALTEIAWORLD_JOIN_INFO,	// Altea entry tab info
		SC_ALTEIAWORLD_PRIVATEGOLDKEYRANKING_INFO,	// Altea personal Golden Key ranking info
		SC_ALTEIAWORLD_PRIVATEPLAYTIMERANKING_INFO,	// Altea personal clear time ranking info
		SC_ALTEIAWORLD_GUILDGOLDKEYRANKING_INFO,	// Altea guild Golden Key ranking info
		SC_ALTEIAWORLD_SENDTICKET_INFO,	// Altea entry ticket sending info
		SC_ALTEIAWORLD_JOIN_RESULT,	// Altea join result
		SC_ALTEIAWORLD_SENDTICKET,	// Received Altea entry ticket
		SC_ALTEIAWORLD_SENDTICKET_RESULT,	// Altea entry ticket send result (to sender)
		SC_ALTEIAWORLD_DICE_RESULT,		// Altea dice roll result
	};
};
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined( PRE_ADD_STAMPSYSTEM )
namespace eStampSystem
{
	enum eSCStampSystem
	{
		SC_STAMPSYSTEM_INIT,
		SC_STAMPSYSTEM_ADD_COMPLETE,
		SC_STAMPSYSTEM_CLEAR_COMPLETE,
		SC_STAMPSYSTEM_CHANGE_WEEKDAY,
		SC_STAMPSYSTEM_CHANGE_TABLE,
	};
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )


//#####################################################
// Please add above here!
//#####################################################


// Farm -------------------------------------------------------------------------
namespace eFarm
{
	enum eCSFarm
	{
		CS_START,
		CS_PLANT,
		CS_HARVEST,
		CS_ADD_WATER,
		CS_WAREHOUSE_LIST,
		CS_TAKE_WAREHOUSE_ITEM,
		CS_FARMINFO,
	};

	enum eSCFarm
	{
		SC_FARMINFO,
		SC_FARMPLANTEDINFO,
		SC_BEGIN_AREASTATE,
		SC_PLANT,
		SC_HARVEST,
		SC_ADDWATER,
		SC_AREAINFO,
		SC_AREAINFO_LIST,
		SC_WAREHOUSE_LIST,
		SC_TAKE_WAREHOUSE_ITEM,
		SC_FIELDCOUNTIFNO,
		SC_WAREHOUSE_ITEMCOUNT,
		SC_ADDWATER_ANOTHERUSER,
#if defined( PRE_ADD_VIP_FARM )
		SC_START,
#endif // #if defined( PRE_ADD_VIP_FARM )
		SC_CHANGE_OWNERNAME,
		SC_OPEN_FARMLIST,
	};
};

//Fishing
namespace eFishing
{
	enum eCSFishing
	{
		CS_READYFISHING,		//Start if in a fishable area (must receive and check) 
		CS_CASTBAIT,			//Turn start (start point of each turn)
		CS_STOPFISHING,			//End fishing
		CS_PULLINGROD,			//Process user messages intermittently
		CS_FISHINGREWARD,		//Request fishing result
	};

	enum eSCFishing
	{
		SC_READYFISHING,		//Response to start
		SC_FISHINGPATTERN,		//Pattern info
		SC_CASTBAITRESULT,		//Result of start (on failure)
		SC_STOPFISHINGRESULT,	//End fishing
		SC_FISHINGREWARD,		//Fishing result~
		SC_FISHINGSYNC,
		SC_FISHINGPLAYER,
		SC_FISHINGEND,
	};
};

namespace eTimeEvent
{
	enum eCSTimeEvent
	{
	};
	enum eSCTimeEvent
	{
		SC_TIMEEVENTLIST,
		SC_ACHIEVE_EVENT,
		SC_EXPIRE_EVENT,
	};
};

namespace ePlayerCustomEventUI
{
	enum eCSPlayerCustomEventUI
	{
		CS_GETPLAYERUI,
		CS_DISMISSGUILD
	};

	enum eSCPlayerCustomEventUI
	{
		SC_GETPLAYERUI,
	};
};


// PROTOCOL_ORIGIN_END -- Token required for shuffle, please do not delete.

#endif	// #if !defined (_FINAL_BUILD) || !defined (ACTIVATE_PROTOCOLSHUFFLE)
