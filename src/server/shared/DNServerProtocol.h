#pragma once

//----------------------------------------------------------------------------------------------------------------------------
//                     protocol    
//----------------------------------------------------------------------------------------------------------------------------

enum eServerMainCmd
{
	MAINCMD_LOGIN,
	MAINCMD_AUTH,
	MAINCMD_STATUS,		// Mainly the attribute ones?
	MAINCMD_ETC,		// Etc.;
	MAINCMD_QUEST,
	MAINCMD_MISSION,
	MAINCMD_APPELLATION,
	MAINCMD_SKILL,
	MAINCMD_FRIEND,
	MAINCMD_ISOLATE,
	MAINCMD_PARTY,
	MAINCMD_PVP,
	MAINCMD_DARKLAIR,
	MAINCMD_GUILD,
	MAINCMD_MAIL,
	MAINCMD_MARKET,
	MAINCMD_ITEM,
	MAINCMD_CASH,
	MAINCMD_MSGADJUST,
	MAINCMD_LOG,
	MAINCMD_REPUTATION,
	MAINCMD_MASTERSYSTEM,
	MAINCMD_SECONDARYSKILL,
	MAINCMD_FARM,
	MAINCMD_JOBSYSTEM,
	MAINCMD_GUILDRECRUIT,
	MAINCMD_DONATION,
	MAINCMD_BESTFRIEND,
	MAINCMD_PRIVATECHATCHANNEL,
	MAINCMD_ALTEIAWORLD,
	MAINCMD_STAMPSYSTEM,
	MAINCMD_DWC,
};


//----------------------------------------------------------------------------------------------------------------------------
//	Login
//----------------------------------------------------------------------------------------------------------------------------

// Login -> Master
// MainCmd: MAINCMD_LOGIN
enum eLoginToMaster
{
	LOMA_NONE,
	LOMA_REGIST,				// Register to Master
	LOMA_ADDUSER,				// Changed AddUser Flow
#if !defined(PRE_MOD_SELECT_CHAR)
	LOMA_USERINFO,				// Send user info to Master
#endif		// #if defined(PRE_MOD_SELECT_CHAR)
	LOMA_DETACHUSER,			// Send user to disconnect
	LOMA_REQTUTORIALGAMEID,		// Request game server ID
	LOMA_CONCOUNTINFO,			// Send current login server user count info
	LOMA_ADDWAITUSER,			// Add to waitlist.
	LOMA_DELWAITUSER,			// Remove from waitlist
	LOMA_DETACHUSEROTHER,		// Disconnect user on another login server
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	LOMA_DETACHUSER_BY_IP,		// Disconnect by IP
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
#ifdef PRE_ADD_BEGINNERGUILD
	LOMA_ADDGUILDMEMB,			// Notify beginner guild member added
#endif		//#ifdef PRE_ADD_BEGINNERGUILD	
};


//----------------------------------------------------------------------------------------------------------------------------
//	Village
//----------------------------------------------------------------------------------------------------------------------------

// Village -> Master
enum eVillageToMaster
{
	VIMA_NONE,
	VIMA_REGIST,				// Register to master
	VIMA_ENDOFREGIST,			// End of village info when registering to master		

	VIMA_CHECKUSER,				// Check if user came properly through login or directly... (user must exist on master)
	VIMA_ADDUSERLIST,			// Send users in village when master connects
	VIMA_DELUSER,				// Remove user

	VIMA_ENTERVILLAGE,			//Successfully entered the field!
	VIMA_VILLAGETOVILLAGE,		// Move from village to village
	VIMA_LOGINSTATE,			// Check whether login is alive or dead
	VIMA_RECONNECTLOGIN,		// Move to character selection screen

	VIMA_PROCESSDELAYED,
	VIMA_VILLAGEUSERREPORT,			// Channel user count

	// party
	VIMA_REQGAMEID,				// Notify connectable game server ID
#if defined( PRE_PARTY_DB )
	VIMA_REQPARTYINVITE,
	VIMA_RESPARTYINVITE,
	VIMA_PARTYINVITEDENIED,
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	VIMA_REQPARTYASKJOIN,
	VIMA_RESPARTYASKJOIN,
	VIMA_ASKJOINAGREEINFO,
#endif
#else
	VIMA_REQPARTYID,				//Get party ID
#endif // #if defined( PRE_PARTY_DB )
	VIMA_RETINVITEPARTYMEMBER,	// Party invite result
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	VIMA_GETPARTYID,			// Search party ID by character name
	VIMA_GETPARTYID_RESULT,
#endif

	//Friend
	VIMA_FRIENDADD,				//Notify when someone adds as friend

	// chat
	VIMA_PRIVATECHAT,			// Find whisper user and send
	VIMA_CHAT,					// Party chat, guild chat, etc.
	VIMA_WORLDSYSTEMMSG,		// Handle world-wide system(?) message (e.g. enhance level x+ success, mission x success, etc.)
#if defined( PRE_PRIVATECHAT_CHANNEL )
	VIMA_PRIVATECHANNELCHAT,	// Private chat channel message
#endif

	//PvP
	VIMA_PVP_MOVELOBBYTOVILLAGE,
	VIMA_PVP_CREATEROOM,
	VIMA_PVP_MODIFYROOM,
	VIMA_PVP_LEAVEROOM,
	VIMA_PVP_CHANGECAPTAIN,
	VIMA_PVP_ROOMLIST,
	VIMA_PVP_JOINROOM,
	VIMA_PVP_READY,
	VIMA_PVP_START,
	VIMA_PVP_RANDOMJOINROOM,
	VIMA_PVP_CHANGETEAM,
	VIMA_PVP_BAN,
	VIMA_PVP_ENTERLOBBY,
	VIMA_PVP_FATIGUE_OPTION,
	VIMA_PVP_SWAPTMEMBER_INDEX,
	VIMA_PVP_CHANGEMEMBER_GRADE,

	VIMA_NOTIFYMAIL,			// Notification that mail arrived
	VIMA_NOTIFYMARKET,			// Notification of entering market
	VIMA_NOTIFYGIFT,			// Notification that a gift arrived

	// Cheat Notice
	VIMA_NOTICE,
	VIMA_ZONENOTICE,

	// Cheat-related
	VIMA_TRACEBREAKINTO,
	VIMA_REQRECALL,
	VIMA_BANUSER,
	VIMA_DESTROYPVP,
	// VoiceChat
	VIMA_REQVOICECHANNELID,		//Request voice channel ID

	// Guild
	VIMA_INVAITEGUILDMEMB,		// Guild member invite
	VIMA_RETINVITEGUILDMEMBER,	// Guild invite result

	VIMA_GUILDWAREINFO,			// Request guild storage info
	VIMA_RETGUILDWAREINFO,		// Guild storage info request result

	VIMA_DISMISSGUILD,			// Guild disband notification
	VIMA_ADDGUILDMEMB,			// Notify add guild member
	VIMA_DELGUILDMEMB,			// Guild member removal (leave/kick) notification
	VIMA_CHANGEGUILDINFO,		// Guild info change notification
	VIMA_CHANGEGUILDMEMBINFO,	// Guild member info update notification
	VIMA_GUILDCHAT,				// Guild chat notification

	VIMA_GUILDMEMBER_LEVELUP,	// Guild member level-up notification
	VIMA_GUILDCHANGENAME,		// Change guild name
	VIMA_GUILDCHANGEMARK,		// Change guild mark 

	// GuildWar	
	VIMA_ENROLLGUILDWAR,		// Apply for Guild War
	VIMA_SETGUILDWAR_SCHEDULE,		// Set Guild War schedule
	VIMA_SETGUILDWAR_FINALSCHEDULE, // Set Guild War finals schedule
	VIMA_SETGUILDWAR_POINT,			// Query Guild War blue/red team points
	VIMA_ADDGUILDWAR_POINT,			// Acquire Guild War points	
	VIMA_SETGUILDWAR_FINAL,			// Set finals-qualifying guild info		
	VIMA_SETGUILDWAR_TOURNAMENTINFO, // Set tournament (bracket) info
	VIMA_SETGUILDWAR_PRE_WIN_GUILD,		// Previous round winning guild
	VIMA_SETGUILDWAR_PREWIN_SKILLCOOLTIME,	// Guild War winner skill cooldown
	VIMA_ADD_DBJOBSYSTEM_RESERVE,		// DB Job registration result
	VIMA_GET_DBJOBSYSTEM_RESERVE,		// DB Job execution result
	VIMA_MATCHLIST_SAVE_RESULT,			// Whether tournament bracket info is saved
	VIMA_SETGUILDWAR_POINT_RUNNING_TOTAL,	// Guild War preliminary tally status
	VIMA_SETGUILDWAR_PRE_WIN_REWARD,	// Check if rewards were granted to winning guild.. (farm, gesture)
	VIMA_GUILDWAR_REFRESH_GUILD_POINT,		// Refresh guild points for Guild War reward
	VIMA_GUILDWAR_SCHEDULE_RELOAD,			// Reload Guild War schedule.

	VIMA_UPDATEGUILDEXP,			// Update guild EXP
	VIMA_GUILDRECRUIT_MEBMER,			// Notify character of guild board join/reject
	VIMA_GUILD_ADD_REWARDITEM,			// Apply guild reward item
	VIMA_GUILD_EXTEND_GUILDSIZE,		// Max guild member count
	
	// Channel
	VIMA_UPDATECHANNELSHOWINFO,
	VIMA_PCBANGRESULT,			// Get PC bang result values

	VIMA_PVP_WAITUSERLIST,
	VIMA_MASTERSYSTEM_CHECKMASTERAPPLICATION,
	VIMA_MASTERSYSTEM_SYNC_SIMPLEINFO,
	VIMA_MASTERSYSTEM_SYNC_JOIN,
	VIMA_MASTERSYSTEM_SYNC_LEAVE,
	VIMA_MASTERSYSTEM_SYNC_GRADUATE,
	VIMA_MASTERSYSTEM_SYNC_CONNECT,
	VIMA_MASTERSYSTEM_CHECK_LEAVE,
	VIMA_MASTERSYSTEM_RECALL_MASTER,
	VIMA_MASTERSYSTEM_BREAKINTO,
	VIMA_MASTERSYSTEM_JOIN_CONFIRM,			// Master-apprentice approval request
	VIMA_MASTERSYSTEM_JOIN_CONFIRM_RESULT,	// Master-apprentice approval result
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	VIMA_SYNC_SYSTEMMAIL,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	VIMA_REFRESH_GUILDITEM,			// Update guild storage item
	VIMA_REFRESH_GUILDCOIN,			// Update guild warehouse coin
	VIMA_EXTEND_GUILDWARE,			// Expand guild storage size

	VIMA_FARMLIST,					//Send list so master can manage it. Sending multiple times is fine.
	VIMA_CHANGECHARACTERNAME,		// Change character name
	VIMA_LADDER_ENTERCHANNEL,
	VIMA_LADDER_REQGAMEID,			// Notify connectable game server ID
	VIMA_LADDER_OBSERVER,
	VIMA_LADDER_INVITEUSER,
	VIMA_LADDER_INVITECONFIRM,

	VIMA_SAVE_USERTEMPDATA,			// Save temporary data
	VIMA_LOAD_USERTEMPDATA,			// Load temporary data
	VIMA_UPPDATE_WORLDEVENTCOUNTER,
	VIMA_DUPLICATE_LOGIN,			// Duplicate login
	VIMA_CHECK_LASTDUNGEONINFO,
	VIMA_CONFIRM_LASTDUNGEONINFO,

#if defined( PRE_ADD_QUICK_PVP)
	VIMA_MAKEQUICKPVP_ROOM,
#endif
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	VIMA_PVP_ROOMLIST_RELAY,	// Receive room list from outside Colosseum village
#endif
#if defined (PRE_ADD_BESTFRIEND)
	VIMA_SEARCH_BESTFRIEND,		// Search user
	VIMA_REGIST_BESTFRIEND,		// Best friend request
	VIMA_RETREGIST_BESTFRIEND,	// Best friend request result
	VIMA_COMPLETE_BESTFRIEND,	// Best friend registration complete
	VIMA_EDIT_BESTFRIENDMEMO,	// Modify best friend memo
	VIMA_CANCEL_BESTFRIEND,		// Break off best friend
	VIMA_CLOSE_BESTFRIEND,		// Delete best friend
	VIMA_LEVELUP_BESTFRIEND,	// Best friend level up
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	VIMA_PVP_CHANGECHANNEL,
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined( PRE_WORLDCOMBINE_PARTY )
	VIMA_GET_WORLDPARTYMEMBER,
#endif	//	#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_PRIVATECHAT_CHANNEL )
	VIMA_PRIVATECHATCHANNEL_ADD,	
	VIMA_PRIVATECHATCHANNEL_MEMBERADD,
	VIMA_PRIVATECHATCHANNEL_MEMBERINVITE,
	VIMA_PRIVATECHATCHANNEL_MEMBERINVITERESULT,
	VIMA_PRIVATECHATCHANNEL_MEMBERDEL,
	VIMA_PRIVATECHATCHANNEL_MEMBERKICKRESULT,
	VIMA_PRIVATECHATCHANNEL_MOD,
	VIMA_PRIVATECHATCHANNEL_MODMEMBERNAME,
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
	VIMA_PVP_SWAP_TOURNAMENT_INDEX,
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_WORLDCOMBINE_PVP )
	VIMA_WORLDPVP_REQGAMEID,
	VIMA_WORLDPVP_BREAKINTO,
#endif
#if defined( PRE_ADD_MUTE_USERCHATTING )
	VIMA_MUTE_USERFIND, //GM command: block chat for a specific user
#endif	// #if defined( PRE_ADD_MUTE_USERCHATTING )
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	VIMA_ALTEIAWORLD_SENDTICKET,
	VIMA_ALTEIAWORLD_SENDTICKET_RESULT,
#endif
#if defined( PRE_DRAGONBUFF )
	VIMA_APPLY_WORLDBUFF,
#endif
	//DWC
#if defined( PRE_PVP_GAMBLEROOM )
	VIMA_GAMBLE_CREATEROOMFLAG,
	VIMA_GAMBLE_DELETEROOM,
#endif
};


//----------------------------------------------------------------------------------------------------------------------------
//	Game
//----------------------------------------------------------------------------------------------------------------------------

// Game -> Master
enum eGameToMaster
{
	GAMA_NONE,
	GAMA_REGIST,				// Register to master
	GAMA_ENDOFREGIST,
	GAMA_CONNECTCOMPLETE,
	GAMA_DELAYREPORT,
	
	GAMA_CHECKUSER,				// Check if user came properly through login or directly... (user must exist on master)
	GAMA_ADDUSERLIST,			// Send users in game when master connects
	GAMA_DELUSER,				// Remove user
	GAMA_ENTERGAME,

	GAMA_CHANGESERVER,			// Swap server info (game and village)
	GAMA_SETROOMID,				// Get room ID to create
	GAMA_SETTUTORIALROOMID,		// Get room ID to create
	GAMA_REQNEXTVILLAGEINFO,	// Get next village info
	GAMA_REBIRTHVILLAGEINFO,	// Get village info for resurrection
	GAMA_MOVEPVPGAMETOPVPLOBBY,	// Move from PvP game -> PvP lobby
	GAMA_LOGINSTATE,			// Check whether login is alive or dead
	GAMA_VILLAGESTATE,			// Whether village server is alive or dead
	GAMA_RECONNECTLOGIN,		// Move to login

	// party
	//GAMA_DELPARTYMEMBER, // Remove party member
	//GAMA_PARTY,

	//Friend
	GAMA_FRIENDADD,

	// chat
	GAMA_PRIVATECHAT,			// Find whisper user and send
	GAMA_CHAT,					// Party chat, guild chat, etc.
	GAMA_WORLDSYSTEMMSG,		// Handle world-wide system(?) message (e.g. enhance level x+ success, mission x success, etc.)
#if defined( PRE_PRIVATECHAT_CHANNEL )
	GAMA_PRIVATECHANNELCHAT,	// Private chat channel
#endif

	// CheatNotice
	GAMA_NOTICE,
	GAMA_BANUSER,

	// PvP
	GAMA_PVP_ROOMSYNCOK,
	GAMA_PVP_BREAKINTOOK,
	GAMA_PVP_COMMAND,

	GAMA_PVP_SWAPTMEMBER_INDEX,
	GAMA_PVP_CHANGEMEMBER_GRADE,
	GAMA_PVP_GUILDWARSCORE,
	GAMA_PVP_GUILDWARRESULT,
	GAMA_PVP_GUILDWAREXCEPTION,

	// Ladder
	GAMA_PVP_LADDERROOMSYNC,

	//UserState
	GAMA_UPDATEWORLDUSERSTATE,		//For updating current position

	// Guild
	GAMA_CHANGEGUILDINFO,			// Guild info change notification
	GAMA_CHANGEGUILDMEMBINFO,		// Guild member info update notification
	GAMA_GUILDCHAT,					// Guild chat notification
	GAMA_GUILDCHANGENAME,			// Change guild name
	GAMA_ADDGUILDWAR_POINT,			// Acquire Guild War points	
	GAMA_UPDATEGUILDEXP,			// Update guild EXP

	//etc
	GAMA_PCBANGRESULT,				// Get PC bang result values
	GAMA_NOTIFYGIFT,				// Notification that a gift arrived

	//Party
	GAMA_INVITEPARTYMEMBER,			// Send invite message from game server to user in village
	GAMA_INVITEPARTYMEMBER_RETURNMSG,	// Error message when invited user enters
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	GAMA_GETPARTYID_RESULT,
	GAMA_RESPARTYASKJOIN,
	GAMA_ASKJOINAGREEINFO,
#endif

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	GAMA_SYNC_SYSTEMMAIL,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	GAMA_MASTERSYSTEM_SYNC_SIMPLEINFO,
	GAMA_MASTERSYSTEM_SYNC_GRADUATE,
	GAMA_MASTERSYSTEM_SYNC_CONNECT,
	GAMA_FARM_DATALOADED,
	GAMA_FARM_USERCOUNT,
	GAMA_FARM_INTENDEDDESTROY,
	GAMA_FARM_SYNC,
	GAMA_FARM_SYNC_ADDWATER,

	GAMA_GUILDMEMBER_LELVEUP,
	GAMA_CHANGECHARACTERNAME,
	GAMA_LOAD_USERTEMPDATA,		
	GAMA_SAVE_USERTEMPDATA,			
	GAMA_DUPLICATE_LOGIN,
	GAMA_CHECK_LASTDUNGEONINFO,
	GAMA_CONFIRM_LASTDUNGEONINFO,

#if defined( PRE_WORLDCOMBINE_PARTY )
	GAMA_DELWORLDPARTY,
	GAMA_WORLDPARTYMEMBER,
#endif
#if defined( PRE_ADD_BESTFRIEND )
	GAMA_LEVELUP_BESTFRIEND,	// Best friend level up
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
	GAMA_PRIVATECHATCHANNEL_ADD,	
	GAMA_PRIVATECHATCHANNEL_MEMBERADD,
	GAMA_PRIVATECHATCHANNEL_MEMBERINVITE,
	GAMA_PRIVATECHATCHANNEL_MEMBERINVITERESULT,
	GAMA_PRIVATECHATCHANNEL_MEMBERDEL,
	GAMA_PRIVATECHATCHANNEL_MEMBERKICKRESULT,
	GAMA_PRIVATECHATCHANNEL_MOD,
#endif
#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	GAMA_WORLDCOMBINEGAMESERVER,
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
	GAMA_DEL_WORLDPVPROOM,
	GAMA_WORLDPVPROOM_JOINRESULT,
	GAMA_WORLDPVPROOM_GMCREATERESULT,
#endif
#if defined( PRE_ADD_MUTE_USERCHATTING )
	GAMA_MUTE_USERFIND, //GM command: block chat for a specific user
#endif	// #if defined( PRE_ADD_MUTE_USERCHATTING )
#if defined( PRE_MOD_71820 )
	GAMA_NOTIFYMAIL,
#endif // #if defined( PRE_MOD_71820 )
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	GAMA_ALTEIAWORLD_SENDTICKET_RESULT,
#endif
};

//----------------------------------------------------------------------------------------------------------------------------
//	Master
//----------------------------------------------------------------------------------------------------------------------------

// Master -> Login
enum MasterToLogin
{
	MALO_NONE,
	MALO_REGIST,					// Register to Login
	MALO_VILLAGEINFO,				// Village channel info
	MALO_VILLAGEINFODEL,			// Clear village channel info
	MALO_CHECKUSER,					// Checking on master (for drop-in) -- unused.. safe to delete~
#if !defined(PRE_MOD_SELECT_CHAR)
	MALO_USERINFO,					// Result of whether user data sent from Login was registered properly
#endif		// #if defined(PRE_MOD_SELECT_CHAR)
	MALO_SETTUTORIALGAMEID,			// Send game ID to connect to
	MALO_DETACHUSER,				// Disconnect user
	MALO_ADDUSER,					// Result of adding user to master on normal entry through login
	MALO_ADDWAITUSER,				// Queue add result value
	MALO_WAITUSERPROCESS,			// Queue process
	MALO_UPDATECHANNELSHOWINFO,
	MALO_DUPLICATE_LOGIN,			// Duplicate login result
};

// Master -> Village
enum eMasterToVillage
{
	MAVI_NONE,

	MAVI_REGISTWORLDINFO,			// Notify world info
	MAVI_REQUSERLIST,				// Request user list
	MAVI_REGISTCOMPLETE,			// Village registration complete

	//MAVI_REGIST, // Register to village
	MAVI_SETGAMEID,					// Send game ID to connect to
	//MAVI_VILLAGESERVERINFO, // Village server info composed and sent by Master
	MAVI_VILLAGEINFO,				// Channel info of another physical process or another server (Village)
	MAVI_VILLAGEINFODEL,			// Clear channel info
	MAVI_LOGINSTATE,				// Check login state

	MAVI_CHECKUSER,					// Check if user actually exists and pass data
	MAVI_TARGETVILLAGEINFO,
	MAVI_DETACHUSER,				// Send forced disconnect to user
	MAVI_USERSTATE,					// User State
	MAVI_FCMSTATE,					// Send fatigue status (Shanda)

	// party
#if defined( PRE_PARTY_DB )
	MAVI_RESPARTYINVITE,
	MAVI_REQPARTYINVITE,
	MAVI_PARTYINVITEDENIED,
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	MAVI_REQPARTYASKJOIN,
	MAVI_RESPARTYASKJOIN,
	MAVI_ASKJOINAGREEINFO,
#endif
#else
	MAVI_REQPARTYID,
	MAVI_REQPARTYIDFAIL,
#endif // #if defined( PRE_PARTY_DB )
	MAVI_PUSHPARTY,					// Send party info
	MAVI_INVITEPARTYMEMBER,			// Please invite
	MAVI_INVITEPARTYMEMBER_RETMSG,	// Result of response to invite result

	//friend
	MAVI_FRIENDADDNOTICE,

	// chat
	MAVI_PRIVATECHAT,				// Send whisper
	MAVI_CHAT,						// Guild, party, etc.
	MAVI_ZONECHAT,					// All in the same village regardless of channel (newly created due to different format)
	MAVI_WORLDSYSTEMMSG,			// Handle world-wide system(?) message (e.g. enhance level x+ success, mission x success, etc.)
#if defined( PRE_PRIVATECHAT_CHANNEL )
	MAVI_PRIVATECHNNELCHAT,			// Private chat channel message
#endif

	//	PvP
	MAVI_PVP_CREATEROOM,			// Create PvP room
	MAVI_PVP_MODIFYROOM,			// Modify PvP room
	MAVI_PVP_DESTROYROOM,			// Destroy PvP room
	MAVI_PVP_LEAVEROOM,				// User left PvP room
	MAVI_PVP_ROOMLIST,				// PvP room list
	MAVI_PVP_JOINROOM,				// Join PvP room
	MAVI_PVP_ROOMSTATE,				// PvP room state
	MAVI_PVP_CHANGETEAM,			// Change PvP team
	MAVI_PVP_CHANGEUSERSTATE,		// Change PvP user state
	MAVI_PVP_START,					// Start PvP
	MAVI_PVP_STARTMSG,				// PvP start message
	MAVI_PVP_ROOMINFO,				// PvP room info

	// Cheat-related
	MAVI_RESRECALL,
	MAVI_VILLAGETRACE,
	MAVI_RESTRAINT,

	//notice
	MAVI_NOTICE,
	MAVI_CANCELNOTICE,
	//Consolidated as types increased
	//MAVI_ZONENOTICE, // Notice; ultimately sent as a chat message to avoid sending multiple times..

	MAVI_NOTIFYMAIL,				// Notification that mail arrived
	MAVI_NOTIFYMARKET,				// Notification of entering market
	MAVI_NOTIFYGIFT,				// Notification that a gift arrived

	//VoiceChannel
	MAVI_VOICECHANNELID,

	// Guild
	MAVI_INVITEGUILDMEMB,			// Guild invite notification
	MAVI_INVITEGUILDMEMBER_RETMSG,	// Result of response to guild invite result
	MAVI_GUILDWAREINFO,				// Request guild storage info
	MAVI_RETGUILDWAREINFO,			// Guild storage info request result
	MAVI_DISMISSGUILD,				// Guild disband notification
	MAVI_ADDGUILDMEMB,				// Notify add guild member
	MAVI_DELGUILDMEMB,				// Guild member removal (leave/kick) notification
	MAVI_CHANGEGUILDINFO,			// Guild info change notification
	MAVI_CHANGEGUILDMEMBINFO,		// Guild member info update notification
	MAVI_GUILDCHAT,					// Guild chat notification
	MAVI_GUILDMEMBER_LEVELUP,		// Guild member level-up notification
	MAVI_GUILDCHANGE_NAME,			// Change guild name
	MAVI_GUILDCHANGEMARK,			// Change guild mark
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	MAVI_GUILDCHAT_MOBILE,			// Mobile guild chat
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

	// GuildWar
	MAVI_ENROLL_GUILDWAR,			// Notify Guild War application
	MAVI_CHANGE_GUILDWAR_STEP,		// Notify Guild War state change (preliminaries, finals, etc.)
	MAVI_GETGUILDWAR_SCHEDULE,		// Request Guild War schedule	
	MAVI_GETGUILDWAR_FINALSCHEDULE, // Request Guild War finals schedule
	MAVI_SETGUILDWAR_FINALPROCESS,	// Guild War finals progress status
	MAVI_GETGUILDWAR_POINT,			// Request Guild War points query (blue/red team)
	MAVI_SETGUILDWAR_POINT,			// Set Guild War points.
	MAVI_ADDGUILDWAR_POINT,			// Acquire Guild War points
	MAVI_GETGUILDWAR_FINAL_TEAM,	// Finals-advancing guild info (before finals start)
	MAVI_SETGUILDWAR_FINAL_TEAM,	// Save finals-qualifying guild info (save bracket)
	MAVI_GETGUILDWAR_TOURNAMENTINFO, // Get tournament info for already started finals
	MAVI_SETGUILDWAR_TOURNAMENTINFO, // Send tournament info for already started finals
	MAVI_GETGUILDWAR_TRIAL_STATS,	// Preliminary statistics info	
	MAVI_SETGUILDWAR_SECRETMISSION,	// Guild War secret mission
	MAVI_SETGUILDWAR_FINAL_RESULT,	// Guild War finals result (win/lose)
	MAVI_SETGUILDWAR_FINAL_REWARD,	// Reward when Guild War room is destroyed.
	MAVI_ADDGUILD_POINT,			// Just acquire guild points..
	MAVI_SETGUILDWAR_FINAL_RESULT_DB,	// Save the main tournament results to DB.
	MAVI_GETGUILDWAR_PREWIN_GUILD,		// Get previous Guild War season winning team
	MAVI_SETGUILDWAR_PREWIN_GUILD,		// Set previous Guild War season winning team
	MAVI_SETGUILDWAR_PREWIN_SKILLCOOLTIME,	// Guild War winner skill cooldown
	MAVI_SETGUILDWAR_EVENT_TIME,		// Notify Guild War schedule
	MAVI_SETGUILDWAR_TOURNAMENTWIN,		// Notify Guild War finals champion guild
	MAVI_ADD_DBJOBSYSTEM_RESERVE,		// Register DB Job
	MAVI_GET_DBJOBSYSTEM_RESERVE,		// DB Job result
	MAVI_GETGUILDWAR_POINT_RUNNING,		// Get Guild War preliminary guild ranking (approx. every 10 min)..
	MAVI_SETGUILDWAR_POINT_RUNNING,		// Set Guild War preliminary guild ranking
	MAVI_GETGUILDWAR_PREWIN_REWARD,		// Previous Guild War season winning team reward status (farm, gesture)
	MAVI_GUILDWAR_REFRESH_GUILD_POINT,	// Refresh guild points
	MAVI_GUILDWAR_RESET_BUYED_ITEM_COUNT, // Reset count of items purchased with Guild War festival points.
	MAVI_UPDATEGUILDEXP,				// Update guild EXP
	MAVI_GUILDRECRUIT_MEMBER,			//Send guild recruitment board join approval/rejection to user
	MAVI_GUILD_ADD_REWARDITEM,			// Add guild reward item effect
	MAVI_GUILD_EXTEND_GUILDSIZE,				// Increase guild max size

	MAVI_PCBANGRESULT,				// Related to PC bang results (Nexon)

	// Channel
	MAVI_UPDATECHANNELSHOWINFO,

	MAVI_PVP_WAITUSERLIST,
	MAVI_PVP_MEMBERINDEX,
	MAVI_PVP_MEMBERGRADE,
	MAVI_MASTERSYSTEM_CHECK_MASTERAPPLICATION,
	MAVI_MASTERSYSTEM_SYNC_SIMPLEINFO,
	MAVI_MASTERSYSTEM_SYNC_JOIN,
	MAVI_MASTERSYSTEM_SYNC_LEAVE,
	MAVI_MASTERSYSTEM_SYNC_GRADUATE,
	MAVI_MASTERSYSTEM_SYNC_CONNECT,
	MAVI_MASTERSYSTEM_CHECK_LEAVE,
	MAVI_MASTERSYSTEM_RECALL_MASTER,
	MAVI_MASTERSYSTEM_JOIN_CONFIRM,				// Master-apprentice approval request
	MAVI_MASTERSYSTEM_JOIN_CONFIRM_RESULT,		// Master-apprentice approval request result
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	MAVI_SYNC_SYSTEMMAIL,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	MAVI_REFRESH_GUILDITEM,		// Update guild storage item
	MAVI_REFRESH_GUILDCOIN,		// Update guild warehouse coin
	MAVI_EXTEND_GUILDWARE,		// Expand guild warehouse

	MAVI_FARMINFOUPDATE,		// Farm-related items to synchronize
	MAVI_FARM_SYNC,

	MAVI_CHANGE_CHARACTERNAME,	// Change character name
	MAVI_LADDERSYSTEM_ENTERCHANNEL,
	MAVI_LADDERSYSTEM_DELUSER,
	MAVI_LADDERSYSTEM_SETGAMEID,	// Send game ID to connect to
	MAVI_LADDERSYSTEM_ROOMSYNC,
	MAVI_LADDERSYSTEM_INVITEUSER,
	MAVI_LADDERSYSTEM_INVITECONFIRM,
	MAVI_USERTEMPDATA_RESULT,
	MAVI_UPPDATE_WORLDEVENTCOUNTER,
	MAVI_CHECK_LASTDUNGEONINFO,
	MAVI_CONFIRM_LASTDUNGEONINFO,
#if defined(PRE_ADD_QUICK_PVP)
	MAVI_MAKEQUICKPVP_ROOM,
#endif
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	MAVI_PVP_ROOMLIST_RELAY,			// Relay PVP room list
	MAVI_PVP_ROOMLIST_RELAY_ACK,		// PVP room list relay response (deliver actual room list)
#endif
#if defined (PRE_ADD_BESTFRIEND)
	MAVI_SEARCH_BESTFRIEND,
	MAVI_REGIST_BESTFRIEND,
	MAVI_REGIST_BESTFRIEND_RETMSG,
	MAVI_COMPLETE_BESTFRIEND,
	MAVI_EDIT_BESTFRIENDMEMO,
	MAVI_CANCEL_BESTFRIEND,
	MAVI_CLOSE_BESTFRIEND,
	MAVI_LEVELUP_BESTFRIEND,
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	MAVI_PVP_CHANGECHANNEL,
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined( PRE_WORLDCOMBINE_PARTY )
	MAVI_DELWORLDPARTY,
	MAVI_WORLDPARTYMEMBER,
#endif
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	MAVI_GETPARTYID,			// Search party ID by character name
	MAVI_GETPARTYID_RESULT,
#endif
	MAVI_UPDATEGUILDWARE,
#if defined( PRE_PRIVATECHAT_CHANNEL )
	MAVI_PRIVATECHATCHANNEL_ADD,
	MAVI_PRIVATECHATCHANNEL_MEMBERADD,
	MAVI_PRIVATECHATCHANNEL_MEMBERINVITE,
	MAVI_PRIVATECHATCHANNEL_MEMBERINVITERESULT,
	MAVI_PRIVATECHATCHANNEL_MEMBERDEL,
	MAVI_PRIVATECHATCHANNEL_MEMBERKICKRESULT,
	MAVI_PRIVATECHATCHANNEL_MOD,
	MAVI_PRIVATECHATCHANNEL_MODMEMBERNAME,
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
	MAVI_PVP_SWAP_TOURNAMENT_INDEX,
	MAVI_PVP_SHUFFLE_TOURNAMENT_INDEX,
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_WORLDCOMBINE_PVP )
	MAVI_DEL_WORLDPVPROOM,
	MAVI_WORLDPVPROOM_JOINRESULT,	
	MAVI_WORLDPVPROOM_GMCRETERESULT,
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
	MAVI_MUTE_USERCHAT,	// GM command: block chat for a specific user
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	MAVI_ALTEIAWORLD_SENDTICKET,
	MAVI_ALTEIAWORLD_SENDTICKET_RESULT,
#endif
#if defined( PRE_DRAGONBUFF )
	MAVI_APPLYWORLDBUFF,
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	MAVI_GAMBLEROOM_CREATE,
#endif
};

// Master -> Game
enum eMasterToGame
{
	MAGA_NONE,

	MAGA_REGISTWORLDINFO,			// Notify world info
	MAGA_REQUSERLIST,				// Request user list
	MAGA_REGISTCOMPLETE,			// Village registration complete

	//MAGA_REGIST, // Register to game

	MAGA_REQROOMID,					// Request room id
	//MAGA_REQFARMROOMID, // Request farm room ID
	MAGA_REQTUTORIALROOMID,			// Request tutorial room id
	MAGA_CHECKUSER,					// Check if user actually exists and pass data
	MAGA_DETACHUSER,				// Send forced disconnect to user
	MAGA_USERSTATE,					// User State
	MAGA_FCMSTATE,					// Send fatigue status (Shanda)
	MAGA_LOGINSTATE,				// Login state
	MAGA_VILLAGESTATE,				// Village status

	//channel
	MAGA_NEXTVILLAGEINFO,
	MAGA_REBIRTHVILLAGEINFO,		// Send previous village info when resurrection is clicked
	MAGA_MOVEPVPGAMETOPVPLOBBY,		// Move from game to PvP lobby

	//Friend
	MAGA_FRIENDADDNOTICE,
 
	// party
	//MAGA_DELPARTYMEMBER, // Remove party member
	MAGA_INVITEPARTYMEMBERRESULT,	// Party member invite result on game server (not sent if successful)
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	MAGA_GETPARTYID,
	MAGA_REQPARTYASKJOIN,
#endif

	// chat
	MAGA_PRIVATECHAT,				// Send whisper
	MAGA_CHAT,						// Guild, party, etc.
	MAGA_WORLDSYSTEMMSG,			// Handle world-wide system(?) message (e.g. enhance level x+ success, mission x success, etc.)
#if defined( PRE_PRIVATECHAT_CHANNEL )
	MAGA_PRIVATECHANNELCHAT,		// Private chat channel message
#endif

	//notice
	MAGA_NOTICE,					// Notice is ultimately sent as a chat message to avoid sending multiple times..
	MAGA_CANCELNOTICE,				// Cancel ongoing slide notice
	MAGA_NOTIFYMAIL,				// Notification that mail arrived
	MAGA_NOTIFYMARKET,				// Notification of entering market
	MAGA_NOTIFYGIFT,				// Notification that a gift arrived

	// Cheat
	MAGA_RESRECALL,		
	MAGA_RESTRAINT,					// Add sanction

	// PvP
	MAGA_PVP_GAMEMODE,				// Game mode info
	MAGA_BREAKINTOROOM,				// Drop-in join room
	MAGA_PVP_MEMBERINDEX,
	MAGA_PVP_MEMBERGRADE,
	MAGA_PVP_MEMBERUSERSTATE,
	MAGA_PVP_FORCESTOP,
	MAGA_PVP_ALLSTOPGUILDWAR,

	// Auth
	MAGA_RESETAUTHLIST,				// Disconnect target user list and request report
	MAGA_RESETAUTHONLY,				// Disconnect single target user and request report

	// Guild
	MAGA_DISMISSGUILD,				// Guild disband notification
	MAGA_ADDGUILDMEMBER,				// Notify add guild member
	MAGA_DELGUILDMEMBER,				// Guild member removal (leave/kick) notification
	MAGA_CHANGEGUILDINFO,			// Guild info change notification
	MAGA_CHANGEGUILDMEMBERINFO,		// Guild member info update notification
	MAGA_REFRESHGUILDSELFVIEW,		// Guild visual info (personal/group) change notification
	MAGA_GUILDCHAT,					// Guild chat notification
	MAGA_GUILDMEMBLOGINLIST,		// Guild member login list notification
	MAGA_GUILDMEMBER_LEVELUP,		// Guild member level-up notification
	MAGA_CHANGEGUILDNAME,			// Change guild name
	MAGA_CHANGEGUILDMARK,			// Change guild mark
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	MAGA_GUILDCHAT_MOBILE,			// Mobile guild chat
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

	// GuildWar
	MAGA_ENROLLGUILDWAR,			// Notify Guild War application
	MAGA_CHANGE_GUILDWAR_STEP,		// Notify Guild War state change (preliminaries, finals, etc.)
	MAGA_SETGUILDWAR_POINT,			// Set Guild War points.
	MAGA_ADDGUILDWAR_POINT,			// Acquire Guild War points
	MAGA_SETGUILDWAR_SECRETMISSION,	// Guild War secret mission
	MAGA_SETGUILDWAR_FINALPROCESS,	// Current Guild War season
	MAGA_SETGUILDWAR_PRE_WIN_GUILD,	// Previous round winning guild
	MAGA_SETGUILDWAR_TOURNAMENTWIN,	// Notify Guild War finals winning guild.
	MAGA_SETGUILDWAR_EVENT_TIME,	// Notify Guild War schedule
	MAGA_UPDATEGUILDEXP,			// Update guild EXP
	MAGA_GUILDLEVELUP,				// Guild level up
	MAGA_GUILDRECRUIT_MEMBER,		// Send guild recruitment board join result to user
	MAGA_GUILD_ADD_REWARDITEM,		// Add guild reward item effect
	MAGA_GUILD_EXTEND_GUILDSIZE,			// Increase guild max member count

	//pcbang
	MAGA_PCBANGRESULT,				// Related to PC bang results (Nexon)

	//ServerControl
	MAGA_ZEROPOPULATION,

	MAGA_MASTERSYSTEM_SYNC_SIMPLEINFO,
	MAGA_MASTERSYSTEM_SYNC_JOIN,
	MAGA_MASTERSYSTEM_SYNC_LEAVE,
	MAGA_MASTERSYSTEM_SYNC_GRADUATE,
	MAGA_MASTERSYSTEM_SYNC_CONNECT,
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	MAGA_SYNC_SYSTEMMAIL,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	MAGA_FARM_SYNC,
	MAGA_FARM_SYNC_ADDWATER,
	MAGA_CHANGE_CHARACTERNAME,	// Change character name
	MAGA_ASSIGN_PERIODQUEST,
	MAGA_UPPDATE_WORLDEVENTCOUNTER,
	MAGA_NOTICE_PERIODQUEST,
	MAGA_USERTEMPDATA_RESULT,
	MAGA_CHECK_LASTDUNGEONINFO,
	MAGA_DELETE_BACKUPDUNGEONINFO,
#if defined( PRE_WORLDCOMBINE_PARTY )
	MAGA_GET_WORLDPARTYMEMBER,
#endif
#if defined( PRE_ADD_BESTFRIEND )
	MAGA_CANCEL_BESTFRIEND,
	MAGA_CLOSE_BESTFRIEND,
	MAGA_LEVELUP_BESTFRIEND,
#endif
	MAGI_UPDATEGUILDWARE,
#if defined( PRE_PRIVATECHAT_CHANNEL )
	MAGA_PRIVATECHATCHANNEL_ADD,
	MAGA_PRIVATECHATCHANNEL_MEMBERADD,
	MAGA_PRIVATECHATCHANNEL_MEMBERINVITE,
	MAGA_PRIVATECHATCHANNEL_MEMBERINVITERESULT,
	MAGA_PRIVATECHATCHANNEL_MEMBERDEL,
	MAGA_PRIVATECHATCHANNEL_MEMBERKICKRESULT,
	MAGA_PRIVATECHATCHANNEL_MOD,
	MAGA_PRIVATECHATCHANNEL_MODMEMBERNAME,
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
	MAGA_PVP_TOURNAMENT_INFO,		// Tournament bracket info
#endif


#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	MAGA_PVP_CHANGEROOMMASTER, // Change room master
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
#if defined(PRE_ADD_MUTE_USERCHATTING)
	MAGA_MUTE_USERCHAT,	//GM command: block chat for a specific user
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	MAGA_ALTEIAWORLD_SENDTICKET,
	MAGA_ALTEIAWORLD_SENDTICKET_RESULT,
#endif
};

//----------------------------------------------------------------------------------------------------------------------------
//	DB
//----------------------------------------------------------------------------------------------------------------------------

namespace DB{
	// MAINCMD_AUTH
	enum eDB_MainCmdAuth
	{
		// Auth
		QUERY_BEGINAUTH,			// Set auth info (initial login) (used by GA, VI) // PRE_MODIFY_AUTH_PROCESS_04 // Must be removed!
		QUERY_STOREAUTH,			// Set auth info (right before server transfer) (used by GA, VI)
		QUERY_CHECKAUTH,			// Check auth info (right after server transfer) (used by GA, VI)
		QUERY_RESETAUTH,			// Reset auth info (specific account) (used by GA, VI)
		QUERY_RESETAUTHSERVER,		// Reset auth info (specific server)
		QUERY_RESETERRORCHECK,		// Initialize DB error check
	};

	// MAINCMD_ETC
	enum eDB_MainCmdEtc
	{
		QUERY_UPDATEQUERYQUEUE,			// Enqueue multiple DB work units into the query queue and execute sequentially in one batch.
		QUERY_EVENTLIST,				//Get event list
		QUERY_GETGAMEOPTION,
		QUERY_SETGAMEOPTION,
		QUERY_UPDATE_SECONDAUTH_PASSWORD,
		QUERY_UPDATE_SECONDAUTH_LOCK,
		QUERY_INIT_SECONDAUTH,
		QUERY_VALIDATE_SECONDAUTH,
		QUERY_RESTRAINT,
		QUERY_LOGOUT,
		QUERY_GET_KEYSETTING_OPTION,	// Load key setting info
		QUERY_MOD_KEYSETTING_OPTION,	// Modify key setting info
		QUERY_DBRESULTERROR,			// Packet to notify server to disconnect when DB returns non-ERROR_NONE for a one-way save packet without response
		QUERY_LOGINCHARACTER,			// Character login 
		QUERY_LOGOUTCHARACTER,			// Character logout
		QUERY_GET_PADSETTING_OPTION,	// Load pad setting info
		QUERY_MOD_PADSETTING_OPTION,	// Modify pad setting info
		QUERY_SETRESTRAINT,				// When a sanction occurs on the server
		QUERY_GET_PROFILE,
		QUERY_SET_PROFILE,
		QUERY_UPDATECONNECTINGTIME,		// Save connection time
		QUERY_ADD_ABUSELOG,				// Add log for abusing user.
		QUERY_ADD_ABUSEMONITOR,
		QUERY_DEL_ABUSEMONITOR,
		QUERY_GET_ABUSEMONITOR,
		QUERY_GET_WHOLE_ABUSEMONITOR,		//Query all characters in world
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
		QUERY_GET_SIMPLECONFIG,				//Load default setting values
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
		QUERY_GET_DBSID,
		QUERY_MODLASTCONNECTDATE,
#if defined( PRE_FIX_67546 )
		QUERY_ADD_CHANNELCOUNT,				// Channel count
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
		QUERY_MOD_NEWBIE_REWARDFLAG,	// For checking new user reward after specified time
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#if defined(PRE_ADD_CP_RANK)
		QUERY_ADD_STAGE_CLEAR_BEST,			// Save CP record
		QUERY_GET_STAGE_CLEAR_BEST,			// Query CP record
		QUERY_GET_STAGE_PERSONAL_BEST,		// Query personal CP record
		QUERY_INIT_STAGE_BEST,				// Reset CP record (cheat key)
#endif
		QUERY_MODCHARACTERSLOTCOUNT,
		QUERY_MAINTENANCEFLAG,
	};

	// MAINCMD_STATUS
	enum eDB_MainCmdStatus
	{
		QUERY_SELECTCHARACTER,			// Select character
		QUERY_UPDATEUSERDATA,			// Save user data
		QUERY_CHANGESERVERUSERDATA,		// Save user data when switching servers (Village-Game, Village-Village, Game-Game)
		QUERY_LASTUPDATEUSERDATA,		// Save user data on disconnect
		QUERY_CHANGESTAGEUSERDATA,		// Save info whenever leaving a stage
		QUERY_GETCHARACTERPARTIALYBYNAME,	//  1: CharacterID/bigint, CharacterClassCode/tinyint, CharacterLevel/tinyint, JobCode/tinyint
		QUERY_GETCHARACTERPARTIALYBYDBID,
		QUERY_LEVEL,					// Level
		QUERY_EXP,						// exp
		QUERY_COIN,						// Money
		QUERY_WAREHOUSECOIN,			// Storage coin
		QUERY_MAPINFO,					// Map info (lastgate, lastmap, map)
		QUERY_CASHREBIRTHCOIN,			// Update RebirthCashCoin
		QUERY_FATIGUE,					// Fatigue, 
		QUERY_JOB,
		QUERY_GLYPHDELAYTIME,
		QUERY_GLYPHREMAINTIME,
		QUEST_NOTIFIER,					// Notifier
#if !defined(PRE_DELETE_DUNGEONCLEAR)
		QUERY_DUNGEONCLEAR,				// Dungeon clear
#endif		// #if !defined(PRE_DELETE_DUNGEONCLEAR)
		QUERY_ETERNITYITEM,				// Permanent item
		QUERY_CHECKFIRSTVILLAGE,		// Initial village check
		QUERY_ADDCASHREBIRTHCOIN,		// Update RebirthCashCoin
		QUERY_COLOR,					// hair, eye, skin color
		QUERY_REBIRTHCOIN,				// Resurrection coin
		QUERY_GETLISTEFFECTITEM,		// 
		QUERY_EVENTFATIGUE,				// Event fatigue
		QUERY_OWNCHRACTERLEVEL,			// Level list of my owned characters
		QUERY_NESTCLEARCOUNT,			// UPSERT NEST clear count 
		QUERY_INITNESTCLEARCOUNT,		// Reset NEST weekly clear count 
		QUERY_CHANGECHARACTERNAME,		// Change character name
		QUERY_GETLIST_ETCPOINT,			// Get points.
		QUERY_ADD_ETCPOINT,
		QUERY_USE_ETCPOINT,
		QUERY_MOD_TIMEEVENT_DATE,
		QUERY_CHANGEJOBCODE,			// Use job change item.
		QUERY_GETLIST_VARIABLERESET,
		QUERY_MOD_VARIABLERESET,
		QUERY_CHANGE_SKILLPAGE,
#if defined(PRE_ADD_DOORS_PROJECT)
		QUERY_SAVE_CHARACTER_ABILITY,	// Save character stats..
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
		QUERY_GET_TOTALSKILLLEVEL,		// Combined skill level
		QUERY_ADD_TOTALSKILLLEVEL,		// Equip combined skill level		
#endif
#ifdef PRE_ADD_PRESET_SKILLTREE
		QUERY_GET_SKILLPRESET_LIST,
		QUERY_ADD_SKILLPRESET,
		QUERY_DEL_SKILLPRESET,
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE
#if defined( PRE_ADD_NEWCOMEBACK )
		QUERY_MOD_COMEBACKFLAG,
#endif
#if defined( PRE_ADD_TALISMAN_SYSTEM )
		QUERY_MOD_TALISMANSLOTBITMAP,
#endif
	};

	// MAINCMD_ITEM
	enum eDB_MainCmdItem
	{
		QUERY_ADDITEM,				// Acquire item - P_AddMaterializedItem
		QUERY_USEITEM,				// Consume/destroy item (use/discard/disassemble consumable item) - P_UseItem
		QUERY_RESELLITEM,			// Resell item - P_ResellItem
		QUERY_ENCHANTITEM,			// Item enchant - P_EnchantItem
		QUERY_CHANGEITEMLOCATION,	// Change item location - P_ChangeItemLocation
		QUERY_SWITCHITEMLOCATION,	// Swap positions of two items - P_SwitchItemLocation
		QUERY_MODITEMDURABILITY,	// Batch modify durability - P_ModItemDurability
		QUERY_MODITEM,				// Modify item properties - P_ModMaterializedItem
		QUERY_EXCHANGEPROPERTY,		// Item/coin exchange - P_ExchangeProperty
		QUERY_UPSEQUIP,				// Change equipped item (equip / cashequip)
		QUERY_PUTONEPIECEON,		// Equip one-piece
		QUERY_TAKECASHEQUIPMENTOFF,	// Unequip all at once (cashequip -> cashinven)
		QUERY_GETPAGEMATERIALIZEDITEM,	// Call cash item list by page
		QUERY_GETPAGEVEHICLE,		// Request mount inventory list by page
		QUERY_MODPETSKINCOLOR,		// P_ModPetSkinColor
		QUERY_DELPETEQUIPMENT,		// P_DelPetEquipment
		QUERY_MISSINGITEMLIST,
		QUERY_RECOVERMISSINGITEM,
		QUERY_ADDEFFECTITEMS,		// Add effect item
		QUERY_MWTEST,
		QUERY_CHANGEPETNAME,
		QUERY_MODITEMEXPIREDATE,
		QUERY_MODPETEXP,		
		QUERY_ITEMEXPIREBYCHEAT,
		QUERY_MODADDITIVEITEM,		// P_ModAdditiveItemID
		QUERY_SAVEITEMLOCATIONINDEX,	// P_SaveItemLocationIndex
		QUERY_MODPETSKILL,			// Change pet skill
		QUERY_MODPETSKILLEXPAND,	// Expand pet skill slot
		QUERY_GETLIST_REPURCHASEITEM,
		QUERY_DELEXPIREITEM,
#if defined (PRE_ADD_COSRANDMIX)
		QUERY_MODRANDOMITEM,
#endif
		QUERY_DELCASHITEM,
		QUERY_RECOVERCASHITEM,
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		QUERY_CHECK_NAMEDITEMCOUNT,
		QUERY_CHEATCHECK_NAMEDITEMCOUNT,
#endif
#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
		QUERY_MOVEPOTENTIAL,
#endif		//#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
		QUERY_MODDEGREEOFHUNGER,
#if defined( PRE_ADD_LIMITED_SHOP )
		QUERY_GETLIMITEDSHOPITEM,
		QUERY_ADDLIMITEDSHOPITEM,
		QUERY_RESETLIMITEDSHOPITEM,
#endif
		QUERY_DEL_EFFECTITEM,
#if defined(PRE_ADD_EQUIPLOCK)
		QUERY_ADDLOCK_ITEM,
		QUERY_REQUEST_ITEMUNLOCK,
		QUERY_UNLOCK_ITEM,
		QUERY_GET_LIST_LOCKEDITEMS,
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
#if defined(PRE_ADD_CHAOSCUBE)
		QUERY_GETLIST_CHAOSGAUGEITEMS,
		QUERY_UPS_CHAOSGAUGEITEM,
#endif	// #if defined(PRE_ADD_CHAOSCUBE)
	};

	// MAINCMD_QUEST
	enum eDB_MainCmdQuest
	{
		QUERY_ADDQUEST,						// Insert quest
		QUERY_DELQUEST,						// Erase quest
		QUERY_MODQUEST,						// Modify quest
		QUERY_CLEARQUEST,					// Reset quest
		QUERY_FORCECOMPLETEQUEST,			// Force complete quest
		QUERY_DELETE_CHARACTER_PERIODQUEST,	// Delete assigned timed quest
		QUERY_GET_CHARACTER_PERIODQUESTDATE,// Get the date when the timed quest was assigned
		QUERY_GET_LISTCOMPLETE_EVENTQUEST,	// Get completed event quests
		QUERY_COMPLETE_EVENTQUEST,			// Complete event quest
		QUERY_GET_WORLDEVENTQUESTCOUNTER,	// World quest completion count
		QUERY_MOD_WORLDEVENTQUESTCOUNTER,	// Update world quest count
	};

	// MAINCMD_SKILL
	enum eDB_MainCmdSkill
	{
		QUERY_ADDSKILL,					// Add skill
		QUERY_MODSKILLLEVEL,			// Change skill level
		QUERY_INCREASESKILLPOINT,		// Increase skill points
		QUERY_DELSKILL,					// del skill
		QUERY_RESETSKILL,				// Whether skills have been reset
		QUERY_RESETSKILLBYSKILLIDS,		//
		QUERY_DECREASESKILLPOINT,		// Increase skill points
		QUERY_USEEXPANDSKILLPAGE,		// Use dual skill tree expansion item
		QUERY_SETSKILLPOINT,			// Save skill points (for calibration)
	};

	// MAINCMD_MISSION
	enum eDB_MainCmdMission
	{
		QUERY_MISSIONGAIN,				// Save acquirable mission bitmap
		QUERY_MISSIONACHIEVE,			// Mission ON/OFF
		QUERY_SETDAILYMISSION,			// Save daily mission info (when initially populating all)
		QUERY_SETWEEKLYMISSION,			// Save weekly mission info (when inserting all initially)
		QUERY_SETWEEKENDEVENTMISSION,	// Save event mission info
		QUERY_SETGUILDWARMISSION,		// Save guild festival mission info
		QUERY_MODDAILYWEEKLYMISSION,	// Save daily/weekly mission info
		QUERY_SETPCBANGMISSION,			// Save PC bang mission info
		QUERY_CLEARMISSION,				// Mission clear cheat key
		QUERY_SETGUILDCOMMONMISSION,	// Save guild normal mission info
		QUERY_SETWEEKENDREPEATMISSION,	// Save weekly repeatable mission info
#if defined(PRE_ADD_MONTHLY_MISSION)
		QUERY_SETMONTHLYMISSION,	// Save monthly mission info (when initially received and populating all)
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	};

	// MAINCMD_APPELLATION
	enum eDB_MainCmdAppellation
	{
		QUERY_ADDAPPELLATION,				// Title
		QUERY_SELECTAPPELLATION,		// Selected title
		QUERY_DELETEAPPELLATION,		//Delete title
	};

	// MAINCMD_MAIL
	enum eDB_MainCmdMail
	{
		QUERY_GETCOUNTRECEIVEMAIL,		// Received mail count
		QUERY_GETPAGERECEIVEMAIL,		// Received mail list paging
		QUERY_SENDMAIL,					// Send mail
		QUERY_SENDSYSTEMMAIL,			// System mail (mission reward)
		QUERY_READMAIL,					// Read mail
		QUERY_TAKEATTACHMAILLIST,		// Take attachments (as a batch list)
		QUERY_TAKEATTACHMAIL,			// Take attachment
		QUERY_DELMAIL,					// Delete mail
		QUERY_GETLISTMAILATTACHMENT,	// Query mail-attached item count and attached coins
		QUERY_SENDWISHMAIL,					// Send wish mail
		QUERY_READWISHMAIL,					// Read wish mail
#ifdef PRE_ADD_BEGINNERGUILD
		// Simple system to process rewards via mail in village when an event occurs on login server
		QUERY_GETWILLSENDMAIL,			// Get MailID to grant
		QUERY_DELWILLSENDMAIL,			// Clear ID
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

		// SpecialBox
		QUERY_ADDEVENTREWARD,
		QUERY_GETCOUNTEVENTREWARD,
		QUERY_GETLISTEVENTREWARD,
		QUERY_GETLISTEVENTREWARDITEM,
		QUERY_ADDEVENTREWARDRECEIVER,

#ifdef PRE_ADD_JOINGUILD_SUPPORT
		QUERY_GUILDSUPPORT_REWARDINFO,
		QUERY_SENDGUILDMAIL,			// Mail sent to user that goes to guild storage upon receipt
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	};

	// MAINCMD_MARKET
	enum eDB_MainCmdMarket
	{
		QUERY_GETPAGETRADE,					// Paging of registered item list - P_GetPageTrade
		QUERY_GETCOUNTTRADE,				// Registered item count - P_GetCountTrade
		QUERY_GETLISTMYTRADE,				// List of my registered items for sale - P_GetListMyTrade
		QUERY_GETCOUNTMYTRADE,				// Count of my registered items for sale - P_GetCountMyTrade
		QUERY_ADDTRADE,						// Register item - P_AddTrade
		QUERY_CANCELTRADE,					// Cancel item registration - P_CancelTrade
		QUERY_BUYTRADEITEM,					// Buy item - P_BuyTradeItem
		QUERY_GETLISTTRADEFORCALCULATION,	// Item settlement list - P_GetListTradeForCalculation
		QUERY_GETCOUNTTRADEFORCALCULATION,	// Item settlement count - P_GetCountTradeForCalculation
		QUERY_TRADECALCULATE,				// Individual settlement - P_TradeCalculate
		QUERY_TRADECALCULATEALL,				// Full settlement - P_TradeCalculateAll
#if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
		QUERY_GETTRADEITEMID,				// Find ItemID by MarketDBID
#endif		// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
		QUERY_GETPETALBALANCE,				// Get Petal amount.
		QUERY_GETTRADEPRICE,				// Get market price quote
		QUERY_GETLISTMINITRADE,				// Return list of a specific item registered on the trading house
	};

	// MAINCMD_FRIEND
	enum eDB_MainCmdFriend
	{
		QUERY_FRIENDLIST,				// Receive group and friend list
		QUERY_ADDGROUP,					// Add group
		QUERY_MODGROUPNAME,				// Modify group name
		QUERY_DELGROUP,					// Delete group
		QUERY_MODFRIENDANDGROUPMAPPING,	// Modify friend and friend group mapping
		QUERY_ADDFRIEND,				// Add friend
		QUERY_MODFRIENDMEMO,			// Modify friend memo
		QUERY_DELFRIEND,				// Delete friend
	};

	// MAINCMD_ISOLATE
	enum eDB_MainCmdIsolate
	{
		QUERY_GETISOLATELIST,			//Get block list
		QUERY_ADDISOLATE,				//Add block
		QUERY_DELISOLATE,				//Remove block
	};

#if defined( PRE_PARTY_DB )
	// MAINCMD_PARTY
	enum eDB_MainCmdParty
	{
		QUERY_DELPARTY_FORSERVER,
		QUERY_ADDPARTY,
		QUERY_DELPARTY,
		QUERY_JOINPARTY,
		QUERY_OUTPARTY,
		QUERY_GETLISTPARTY,
		QUERY_ADDPARTYANDMEMBERGAME,
		QUERY_ADDPARTYANDMEMBERVILLAGE,
		QUERY_MODPARTY,
		QUERY_MODPARTYLEADER,
		QUERY_GETPARTY_JOINMEMBERS,
		QUERY_DELPARTY_FORGAMESERVER,
		QUERY_ADDWORLDPARTY,
		QUERY_GETLISTWORLDPARTY,
#if defined( PRE_ADD_NEWCOMEBACK )
		QUERY_MOD_COMEBACKINFO,
#endif
	};
#endif // #if defined( PRE_PARTY_DB )

	// MAINCMD_GUILD
	enum eDB_MainCmdGuild
	{
		// Guild
		QUERY_CREATEGUILD,				// Guild creation request/result
		QUERY_DISMISSGUILD,				// Guild disband request/result
		QUERY_ADDGUILDMEMBER,			// Add guild member request/result
		QUERY_DELGUILDMEMBER,			// Guild member removal (leave/kick) request/result
		QUERY_CHANGEGUILDINFO,			// Guild info change request/result
		QUERY_CHANGEGUILDMEMBERINFO,	// Guild member info update request/result
		QUERY_ADDGUILDHISTORY,			// Add guild history request
		QUERY_GETGUILDHISTORYLIST,		// Guild history list request/result
		QUERY_GETGUILDINFO,				// Guild info request/result
		QUERY_INVITEGUILDMEMBER,		// Guild member invite request/result
		QUERY_MOVE_ITEM_IN_GUILDWARE,	// Move item between guild warehouses
		QUERY_MOVE_INVEN_TO_GUILDWARE,	// Inven -> Guild warehouse
		QUERY_MOVE_GUILDWARE_TO_INVEN,	// Guild storage -> inventory
		QUERY_GUILDWARE_COIN,			// Guild warehouse coin
		QUERY_GET_GUILDWAREINFO,		// Request guild warehouse info
		QUERY_GET_GUILDWAREHISTORY,		// Request guild warehouse history
		QUERY_SET_GUILDWAREINFO,		// Save guild warehouse
		QUERY_CHANGEGUILDNAME,			// Change guild name
		QUERY_CHANGEGUILDWARESIZE,		// Guild storage size change request/result
		QUERY_CHANGEGUILDMARK,			// Change guild mark

		QUERY_ENROLL_GUILDWAR,			// Apply for Guild War
		QUERY_GET_GUILDWARSCHEDULE,		// Get Guild War schedule
		QUERY_GET_GUILDWARFINALSCHEDULE,	// Fetch finals schedule
		QUERY_ADD_GUILDWARPOINT,			// Earn points when clearing preliminary mission
		QUERY_GET_GUILDWARPOINT,			// Query preliminary points (character, guild, team)
		QUERY_GET_GUILDWARPOINT_PARTTOTAL,			// Query 1st place by category (individual)
		QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL,	// Guild ranking by category
		QUERY_GET_GUILDWARPOINT_DAILY,				// Daily 1st place in preliminaries		
		QUERY_GET_GUILDWARPOINT_GUILD_TOTAL,		// Aggregated point rankings of each guild (main tournament qualifiers can also be queried here)
		QUERY_ADD_GUILDWAR_FINAL_MATCHLIST,			// Save finals bracket
		QUERY_ADD_GUILDWARPOPULARITYVOTE,	// Save popularity vote
		QUERY_GET_GUILDWARPOPULARITYVOTE,	// Popularity vote result
		QUERY_ADD_GUILDWARFINALRESULTS,		// Save Guild War finals result
		QUERY_GET_GUILDWARFINALRESULTS,		// Get Guild War finals result
		QUERY_ADD_GUILDWAR_REWARD_CHARACTER,	// Save Guild War preliminary reward result
		QUERY_GET_GUILDWAR_REWARD_CHARACTER,	// Get Guild War preliminary reward result
		QUERY_ADD_GUILDWAR_REWARD_GUILD,		// Save Guild War guild reward result
		QUERY_GET_GUILDWAR_REWARD_GUILD,		// Get Guild War guild reward result
		QUERY_ADD_GUILDWAR_POINT_QUEUE,			// Grant Guild War finals reward points (personal festival points)
		QUERY_ADD_GUILDWAR_SYSTEM_MAIL_QUEUE,	// Send Guild War finals reward mail
		QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE,		// Send Guild War finals reward gift
		QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO,	// Send Guild War finals reward gift (when guild info is missing)
		QUERY_GET_GUILDWAR_PRE_WIN_GUILD,				// Get previous Guild War season winning team
		QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL,		// Guild War preliminary score standings (top 24)
		QUERY_GET_GUILDWARPOINT_FINAL_REWARDS,		// For refreshing guild points in finals reward
		QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD,
		QUERY_ADD_GUILDWAR_ITEM_TRADE_RECORD,

		QUERY_MOD_GUILDEXP,						// Change guild EXP
		QUERY_MOD_GUILDCHEAT,					// Guild cheat
		QUERY_GET_GUILDREWARDITEM,				// Applied guild reward item
		QUERY_ADD_GUILDREWARDITEM,				// Add guild reward item
		QUERY_DEL_GUILDREWARDITEM,				// Delete guild reward item (for cheat)
		QUERY_CHANGE_GUILDSIZE,					// Increase max guild members
		QUERY_GET_GUILDMEMBER,					// Fetch guild members
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
		QUERY_GET_GUILDCONTRIBUTION_POINT,		// Query guild contribution (personal)
		QUERY_GET_GUILDCONTRIBUTION_WEEKLYRANKING,	// Guild contribution weekly ranking
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	};

	// MAINCMD_PVP
	enum eDB_MainCmdPvP
	{
		QUERY_UPDATEPVPDATA,			// Modify PvP data
		QUERY_ADD_PVP_LADDERRESULT,
		QUERY_GETLIST_PVP_LADDERSCORE,
		QUERY_GETLIST_PVP_LADDERSCORE_BYJOB,
		QUERY_INIT_PVP_LADDERGRADEPOINT,
		QUERY_USE_PVP_LADDERPOINT,
		QUERY_ADD_PVP_LADDERCUCOUNT,
		QUERY_MOD_PVP_LADDERSCORE_FORCHEAT,
		QUERY_GETLIST_PVPLADDERRANKING,
		QUERY_GET_PVP_GHOULSCORES,
		QUERY_ADD_PVP_GHOULSCORES,
#ifdef PRE_MOD_PVPRANK
		QUERY_UPDATE_PVPRANKCRITERIA,
		QUERY_FORCE_CALCPVPRANK,
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined(PRE_ADD_PVP_RANKING)
		QUERY_GET_PVP_RANKBOARD,
		QUERY_GET_PVP_RANKLIST,
		QUERY_GET_PVP_RANKINFO,
		QUERY_GET_PVP_LADDER_RANKBOARD,
		QUERY_GET_PVP_LADDER_RANKLIST,
		QUERY_GET_PVP_LADDER_RANKINFO,
		QUERY_FORCE_AGGREGATE_PVP_RANKING,
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
		QUERY_ADD_WORLDPVPROOM,
		QUERY_ADD_WORLDPVPROOMMEMBER,
		QUERY_DEL_WORLDPVPROOM,
		QUERY_DEL_WORLDPVPROOMMEMBER,
		QUERY_DEL_WORLDPVPROOMFORSERVER,
		QUERY_GETLIST_WORLDPVPROOM,
		QUERY_UPDATE_WORLDPVPROOM,
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
		QUERY_ADD_PVP_TOURNAMENTRESULT,
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_FIX_76282 )
		SYNC_GOPVPLOBBY,
#endif // #if defined( PRE_FIX_76282 )
#if defined( PRE_PVP_GAMBLEROOM )
		QUERY_ADD_GAMBLEROOM,
		QUERY_ADD_GAMBLEROOMMEMBER,
		QUERY_END_GAMBLEROOM,
#endif
	};

	// MAINCMD_DARKLAIR
	enum eDB_MainCmdDarkLair
	{
		QUERY_UPDATE_DARKLAIR_RESULT,	// Save Dark Lair result
		QUERY_GET_DARKLAIR_RANK_BOARD,	// Get Dark Lair ranking board info
	};

	// MAINCMD_CASH
	enum eDB_MainCmdCashShop
	{
		QUERY_GETLISTGIVEFAILITEM,			// Get list of failed cash items
		QUERY_MODGIVEFAILFLAG,				// Update failed cash items (put back into inventory)

		QUERY_CHECKGIFTRECEIVER,			// Check cash shop gift recipient
		QUERY_GETLISTGIFTBOX,				// Query gift box list
		QUERY_MODGIFTRECEIVEFLAG,			// Check flag in PurchaseOrder table before placing gift into inventory
		QUERY_RECEIVEGIFT,					// Receive gift
		QUERY_NOTIFYGIFT,					// Gift notifier

		QUERY_MAKEGIFTBYQUEST,				// Quest completion reward
		QUERY_MAKEGIFTBYMISSION,			// Mission completion reward
		QUERY_MAKEGIFT,						// Etc.. completion reward
		QUERY_INCREASEVIPPOINT,				// Add VIP base points
		QUERY_GETVIPPOINT,					// Query VIP period and points
		QUERY_MODVIPAUTOPAYFLAG,			// Change VIP auto-billing status
		QUERY_GIFTBYCHEAT,					// Cheat key
		QUERY_PETAL,						// Petal
		QUERY_GIFTRETURN,					// Gift reject, return
#if defined(PRE_ADD_CASH_REFUND)
		QUERY_PAYMENTINVEN_LIST,			// Request billing inventory (single request, response sent split into single items and packages)
		QUERY_PAYMENTINVEN_PAKAGELIST,		// Billing inventory result (package)
#endif
		QUERY_CHEAT_GIFTCLEAR,				// Cheat to empty gift box
#if defined( PRE_PVP_GAMBLEROOM )
		QUERY_LOGCODE_PETAL,				// Add log code when adding Petal
		QUERY_USE_PETAL,					// Use Petal
#endif
	};

	// MAINCMD_MSGADJUST
	enum eDB_MessageAdjust
	{
		QUERY_MSGADJUST,					// No actual DB work. Packet to check if everything is finished
#ifdef PRE_FIX_63822
		QUERY_MSGADJUST_SEQ_CHANGEPARTS,	// No actual DB work. Packet to prevent duplicate execution
#endif		//#ifdef PRE_FIX_63822
	};

	enum eDB_MainCmdLog
	{
		QUERY_ADDPARTY_STARTLOG,
		QUERY_ADDPARTY_ENDLOG,
		QUERY_ADDSTAGE_STARTLOG,
		QUERY_ADDSTAGE_CLEARLOG,
		QUERY_ADDSTAGE_REWARDLOG,
		QUERY_ADDSTAGE_ENDLOG,
		QUERY_ADDPVP_STARTLOG,
		QUERY_ADDPVP_RESULTLOG,
		QUERY_ADDPVP_ENDLOG,
		QUERY_ADDNESTGATE_STARTLOG,
		QUERY_ADDNESTGATE_ENDLOG,
		QUERY_ADDNESTGATE_CLEARLOG,
		QUERY_ADDNESTDEATHLOG,
	};

	enum eDB_MainCmdReputation
	{
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
		QUERY_MOD_NPCLOCATION,
		QUERY_MOD_NPCFAVOR,
		QUERT_GET_LISTNPCFAVOR,
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	};

	enum eDB_MainCmdMasterSystem
	{
		QUERY_GET_SIMPLEINFO,
		QUERY_GET_PAGEMASTERCHARACTER,
		QUERY_GET_MASTERCHARACTER_TYPE1,
		QUERY_GET_MASTERCHARACTER_TYPE2,
		QUERY_GET_PUPILLIST,
		QUERY_REGISTER_MASTER,
		QUERY_REGISTERCANCEL_MASTER,
		QUERY_JOIN_MASTERSYSTEM,
		QUERY_GET_MASTERANDCLASSMATE,
		QUERY_GET_CLASSMATEINFO,
		QUERY_LEAVE_MASTERSYSTEM,
		QUERY_GET_MASTERANDFAVORPOINT,
		QUERY_GRADUATE,
		QUERY_GET_COUNTINFO,
		QUERY_MOD_RESPECTPOINT,
		QUERY_MOD_FAVORPOINT,
		QUERY_MOD_GRADUATECOUNT,
	};

#if defined( PRE_ADD_SECONDARY_SKILL )
	enum eDB_MainCmdSecondarySkill
	{
		QUERY_ADD_SECONDARYSKILL,
		QUERY_DELETE_SECONDARYSKILL,
		QUERY_GETLIST_SECONDARYSKILL,
		QUERY_MOD_SECONDARYSKILL_EXP,
		QUERY_GETLIST_SECONDARYSKILL_RECIPE,
		QUERY_ADD_SECONDARYSKILL_RECIPE,
		QUERY_DELETE_SECONDARYSKILL_RECIPE,
		QUERY_EXTRACT_SECONDARYSKILL_RECIPE,
		QUERY_MOD_SECONDARYSKILL_RECIPE_EXP,
	};
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	enum eDB_MainCmdFarm
	{
		QUERY_GETLIST_FARM,
		QUERY_GETLIST_FIELD,
		QUERY_GETLIST_FIELD_BYCHARACTER,
		QUERY_ADD_FIELD,
		QUERY_DEL_FIELD,
		QUERY_ADD_FIELD_ATTACHMENT,
		QUERY_MOD_FIELD_ELAPSEDTIME,
		QUERY_HARVEST,
		QUERY_GETLIST_HARVESTDEPOTITEM,
		QUERY_GETFIELDCOUNT_BYCHARACTER,
		QUERY_GETCOUNT_HARVESTDEPOTITEM,
		QUERY_UPDATEFARM_ACTIVATION,
		QUERY_GET_FIELDITEMCOUNT,
		QUERY_GETLIST_FIELD_FORCHARACTER,
		QUERY_ADD_FIELD_FORCHARACTER,
		QUERY_DEL_FIELD_FORCHARACTER,
		QUERY_ADD_FIELD_FORCHARACTER_ATTACHMENT,
		QUERY_MOD_FIELD_FORCHARACTER_ELAPSEDTIME,
		QUERY_HARVEST_FORCHARACTER,
	};
	//Group under guild war for now. Can reuse later if this kind of job procedure is needed.
	enum eDB_MainCmdJobSystem
	{
		QUERY_ADD_JOB_RESERVE,		// For registering job query
		QUERY_GET_JOB_RESERVE,		// For checking whether the job ran.
	};
	enum eDB_MainCmdGuildRecruit
	{
		QUERY_GET_PAGEGUILDRECRUIT,				//Guild recruitment list
		QUERY_GET_PAGEGUILDRECRUITCHARACTER,	//Guild join applicant user
		QUERY_GET_MYGUILDRECRUIT,				//List of guilds the user requested to join		
		QUERY_GET_GUILDRECRUIT_REQUESTCOUNT,	//Number of join requests by user
		QUERY_REGISTERINFO_GUILDRECRUIT,		//Check guild recruitment info
		QUERY_REGISTERON_GUILDRECRUIT,			//Register guild recruitment
		QUERY_REGISTERMOD_GUILDRECRUIT,			//Modify guild recruitment
		QUERY_REGISTEROFF_GUILDRECRUIT,			//Delete guild recruitment		
		QUERY_REQUESTON_GUILDRECRUIT,			//Apply for guild recruitment
		QUERY_REQUESTOFF_GUILDRECRUIT,			//Cancel guild recruitment application
		QUERY_ACCEPTON_GUILDRECRUIT,			//Approve guild recruitment application
		QUERY_ACCEPTOFF_GUILDRECRUIT,			//Reject guild recruitment application
		QUERY_DELETE_GUILDRECRUIT,				//Delete guild and personal recruitment entries
	};

#if defined (PRE_ADD_DONATION)
	enum eDB_MainCmdDonation
	{
		QUERY_DONATE,
		QUERY_DONATION_RANKING,
		QUERY_DONATION_TOP_RANKER,
	};
#endif // #if defined (PRE_ADD_DONATION)

#if defined (PRE_ADD_BESTFRIEND)
	enum eDB_MainCmdBestFriend
	{
		QUERY_GET_BESTFRIEND,
		QUERY_REGIST_BESTFRIEND,
		QUERY_CANCEL_BESTFRIEND,
		QUERY_CLOSE_BESTFRIEND,
		QUERY_EDIT_BESTFRIENDMEMO,
		QUERY_MOD_BESTFRIEND_REWARDITEM,
	};
#endif

#if defined (PRE_PRIVATECHAT_CHANNEL)
	enum eDB_MainCmdPrivateChatChannel
	{
		QUERY_GET_PRIVATECHATCHANNEL,
		QUERY_GET_PRIVATECHATCHANNELMEMBER,
		QUERY_CREATE_PRIVATECHATCHANNEL,
		QUERY_ADD_PRIVATECHATMEMBER,		
		QUERY_DEL_PRIVATECHATMEMBER,
		QUERY_KICK_PRIVATECHATMEMBER,
		QUERY_MOD_PRIVATECHATCHANNELINFO,		
		QUERY_INVITE_PRIVATECHATMEMBER,
		QUERY_MOD_PRIVATEMEMBERSERVERID,
		QUERY_DEL_PRIVATEMEMBERSERVERID,
	};
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	// MAINCMD_ALTEIAWORLD
	enum eDB_MainCmdAlteiaWorld
	{
		QUERY_RESET_ALTEIAWORLDEVENT,
		QUERY_ADD_ALTEIAWORLDPLAYRESULT,
		QUERY_GET_ALTEIAWORLDINFO,
		QUERY_GET_ALTEIAWORLDPRIVATEGOLDKEYRANK,
		QUERY_GET_ALTEIAWORLDPRIVATEPLAYTIMERANK,
		QUERY_GET_ALTEIAWORLDGUILDGOLDKEYRANK,
		QUERY_GET_ALTEIAWORLDSENDTICKETLIST,
		QUERY_ADD_ALTEIAWORLDSENDTICKETLIST,
		QUERY_RESET_ALTEIAWORLDPLAYALTEIA,
		QUERY_ADD_ALTEIAWORLDEVENT,
	};
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined( PRE_ADD_STAMPSYSTEM )
	// MAINCMD_STAMPSYSTEM
	enum eDB_MainCmdStampSystem
	{
		QUERY_GETLIST_COMPLETECHALLENGES,
		QUERY_INIT_COMPLETECHALLENGE,
		QUERY_ADD_COMPLETECHALLENGE,
	};
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined( PRE_ADD_DWC )
	// MAINCMD_DWC
	enum eDB_MainCmdDWC
	{
		QUERY_ADD_DWCTEAM,
		QUERY_ADD_DWCTEAMMEMBER,
		QUERY_DEL_DWCTEAMMEMBER,
		QUERY_GET_DWCTEAM_BY_CHAR,
		QUERY_GETLIST_DWCTEAMMEMBER,
		QUERY_GETPAGE_DWCRANKING,
		QUERY_GET_DWCRANKING,
		QUERY_GET_DWCCHANNEL,
	};
#endif // #if defined( PRE_ADD_DWC )

};

//----------------------------------------------------------------------------------------------------------------------------
//	ServiceManager <-> NetLauncher & ServiceServer
//----------------------------------------------------------------------------------------------------------------------------
enum eServiceManager
{
	NETSERVICE_RESULT = 1,
	NETSERVICE_RUN_PROCESS,
	NETSERVICE_TERMINATE_ALLPROCESS,
	NETSERVICE_TERMINATE_PROCESS,
	NETSERVICE_PATCHSTART,
	NETSERVICE_PATCHEND,
	NETSERVICE_PATCHAPPLY,
	NETSERVICE_INFOCOPY,
	NETSERVICE_PATCH,
	NETSERVICE_PATCH_BYURL,
	NETSERVICE_STARTOF_MANAGING,
	NETSERVICE_MANAGING,
	NETSERVICE_ENDOF_MANAGING,
	NETSERVICE_RETURN_PING,
	NETSERVICE_PATCHBATCH,
	NETSERVICE_RUNBATCH,
	NETSERVICE_STOPBATCH,
	NETSERVICE_LIVEEXTDEL,
	NETSERVICE_LIVEEXTCOPY,
	NETSERVICE_STARTESM,
	NETSERVICE_STOPESM,

	//to server
	NETSERVICE_CONNECTED,
	NETSERVICE_NOTICE,
	NETSERVICE_MAKE_DUMP,
	NETSERVICE_CHANNEL_CONTROL,
	NETSERVICE_POPULATION_CONTROL,
	NETSERVICE_EVENTUPDATE,
	NETSERVICE_AFFINITY_CONTROL,
	NETSERVICE_DETACHALLUSER,
	NETSERVICE_USERRESTRAINT,
	NETSERVICE_USERBAN,
	NETSERVICE_WORLDMAXUSER,
	NETSERVICE_PING,
	NETSERVICE_CANCELNOTICE,
	NETSERVICE_SELECTJOIN,
	NETSERVICE_CLEAR_SELECTJOIN,
    NETSERVICE_RELOADDYNCODE,
	NETSERVICE_SCHEDULE_SERVERCLOSE,
	NETSERVICE_UPDATESALEDATA,
	NETSERVICE_UPDATEFARM,
	NETSERVICE_UPDATEGUILDWAR_SCHEDULE,
	NETSERVICE_EXTRELOAD,
	NETSERVICE_CREATEWORLDDB,
	NETSERVICE_RELOADGUILDWARSCHEDULE,
	NETSERVICE_SIMPLECONFIGUPDATE,
	NETSERVICE_LIMITEITEM_CHANGEQUANTITY,
	NETSERVICE_UPDATEGUILDWARE,
	NETSERVICE_UPDATEWORLDPVPROOM,
	NETSERVICE_CREATEGAMBLEROOM,
	NETSERVICE_STOPGAMBLEROOM,
	NETSERVICE_DELGAMBLEROOM,
	NETSERVICE_UPDATE_DWCSTATUS,
};

//----------------------------------------------------------------------------------------------------------------------------
//	ServiceManager <-> NetLauncher
//----------------------------------------------------------------------------------------------------------------------------
enum ePatcherService
{
	PATCHERSERVICE_RESULT = 1,
	PATCHERSERVICE_STARTLAUNCHER,
	PATCHERSERVICE_STOPLAUNCHER,
	PATCHERSERVICE_PATCHLAUNCHER,

	//to server
	PATCHERSERVICE_PATCHSTATE,
	PATCHERSERVICE_PING,
};

enum eManagedType
{
	MANAGED_TYPE_NONE = 0,
	MANAGED_TYPE_LOGIN = 1,
	MANAGED_TYPE_DB,
	MANAGED_TYPE_MASTER,
	MANAGED_TYPE_VILLAGE,
	MANAGED_TYPE_GAME,
	MANAGED_TYPE_LOG,
	MANAGED_TYPE_CASH,
	MANAGED_TYPE_MAX,
};

enum eNetService
{
	SERVICE_LAUNCHER_CREATED_PROCESS = 1,
	SERVICE_LAUNCHER_PROCESS_TERMINATED,	
	SERVICE_LAUNCHER_PROCESS_STATE,
	SERVICE_LAUNCHER_PATCH_STATE,
	SERVICE_LAUNCHER_VERSION,
	SERVICE_LAUNCHER_PING,
	SERVICE_LAUNCHER_RETURN_PING,
	SERVICE_LAUNCHER_RUNBATCH_STATE,
	SERVICE_LAUNCHER_STOPBATCH_STATE,
	SERVICE_LAUNCHER_LIVEEXTCOPY,
	SERVICE_LAUNCHER_PATCH_PROGRESS,
	SERVICE_LAUNCHER_UNZIP_PROGRESS,
	SERVICE_LAUNCHER_ESM_STATE,	
};

enum eServiceServer
{
	SERVICE_SERVER_REGIST_MANAGEDID,
	SERVICE_SERVER_USUAL_LOGINREPORT,
	SERVICE_SERVER_USUAL_VILLAGEREPORT,
	SERVICE_SERVER_USUAL_GAMEREPORT,
	SERVICE_SERVER_EXCEPTION_REPORT,
	SERVICE_SERVER_SERVICE_CLOSED,
	SERVICE_SERVER_DELAYEDREPORT,
	SERVICE_SERVER_PONG,
	SERVICE_SERVER_MERITINFO,
	SERVICE_SERVER_CHANNELINFO,
	SERVICE_SERVER_WORLDINFO,
	SERVICE_SERVER_UNRECOVERY,
	SERVICE_SERVER_USUAL_MASTERREPORT,
	SERVICE_SERVER_EXT_RELOADRESULT,
	SERVICE_SERVER_LOGIN_WORDDBRESULT,
	SERVICE_SERVER_QUERYDELAYREPORT,
	SERVICE_SERVER_REQUESTSERVICE,
	SERVICE_SERVER_GAMEDELAYREPORT,
	SERVICE_SERVER_DBERROR_REPORT,
	SERVICE_SERVER_VILLAGEDELAYREPORT,
	SERVICE_SERVER_HEARTBEAT,
	SERVICE_SERVER_UPDATEWORLDPVPROOM,
};

enum eMonitorToManager
{
	MONITOR2MANAGER_CHECK_LOGIN = 1,
	MONITOR2MANAGER_SERVERSTRUCT_REQ,
	MONITOR2MANAGER_REALTIME_REQ,	
	MONITOR2MANAGER_STARTSERVICE,
	MONITOR2MANAGER_STOPSERVICE,
	MONITOR2MANAGER_MAKEDUMP,
	MONITOR2MANAGER_CHANNELCONTROL,
	MONITOR2MANAGER_GAMECONTROL,
	MONITOR2MANAGER_REPORT_REQ,
	MANAGER2MONITOR_SERVER_DEALYINFO_REQ,
	MONITOR2MANAGER_CHANGEPASSWORD,
};

enum eServerToMonitor
{
	MANAGER2MONITOR_LOGIN_RESULT = 1,
	MANAGER2MONITOR_MERITLIST,
	MANAGER2MONITOR_NETLAUNCHERLIST,
	MANAGER2MONITOR_SERVERINFO_LIST,
	MANAGER2MONITOR_CHANNELINFO_LIST,
	MANAGER2MONITOR_UPDATE_NOTICE,
	MANAGER2MONITOR_UPDATE_SERVERSTATE,
	MANAGER2MONITOR_LOGIN_UPDATEINFO,
	MANAGER2MONITOR_VILLAGE_UPDATEINFO,
	MANAGER2MONITOR_GAME_UPDATEINFO,
	MANAGER2MONITOR_SERVERSTATE_LIST,
	MANAGER2MONITOR_FARM_UPDATEINFO,
	MANAGER2MONITOR_OPERATING_RESULT,
	MANAGER2MONITOR_WORLDINFO_RESULT,
	MANAGER2MONITOR_REPORT_RESULT,
	MANAGER2MONITOR_SERVER_DEALYINFO,
};

//----------------------------------------------------------------------------------------------------------------------------
//	Log
//----------------------------------------------------------------------------------------------------------------------------

enum eLog
{
	LOG_FILE,					// File log
	LOG_HEARTBEAT,				// For LiveCheck

/*
	// Master Log
MASTERLOG_LOGIN, // Account login
MASTERLOG_LOGOUT, // Account logout
MASTERLOG_CONNECT, // Character connect log

	// Detail Log
LOG_CHARCHANGE, // Character change
LOG_CHARJOB, // Character job change
LOG_CHARCREATEDELETE, // Character create/delete
LOG_CHARZONE, // Character zone enter/exit
LOG_CHARDEATHREBIRTH, // Character death/rebirth

LOG_ITEM, // Item change (acquire/use)
LOG_ITEMENCHANT, // Item enchant

LOG_PARTY, // Party
LOG_QUEST, // Quest
LOG_SKILL, // Skill
LOG_STAGECHAR, // Stage
	LOG_STAGE,
LOG_CUTSCENE, // Cutscene
	
	// PvP Log
	LOG_PvP_Combat,
	LOG_PvP_RoomStart,
	LOG_PvP_RoomEnd,
	LOG_PvP_User,

	LOG_MISSION,
LOG_SPEEDHACK, // Speedhack log

	// Auth
LOG_AUTH, // Authentication
*/
};

enum eHeartBeat
{
	SYN,
	SYN_ACK,
	ACK,
};


//----------------------------------------------------------------------------------------------------------------------------
//	Auth (Nexon)
//----------------------------------------------------------------------------------------------------------------------------
enum eAuthKor
{
	AUTH_INITIALIZE = 41,	// initialize
	AUTH_LOGIN = 42,		// login
	AUTH_LOGOUT = 43,		// logout
	AUTH_TERMINATE = 44,
	AUTH_MESSAGE = 45,
	AUTH_SYNCHRONIZE = 46,
	AUTH_ALIVE = 100,		// alive
};


//----------------------------------------------------	------------------------------------------------------------------------
//	Cash	Village, Game <-> CashServer
//----------------------------------------------------------------------------------------------------------------------------

// MAINCMD_CASH,
enum eCash
{
	/*
CASH_GETPRODUCT, // 1. Query single paid product - P_GetProduct
CASH_ADDPURCHASEORDERBYCASH, // 2. Cash purchase request - P_AddPurchaseOrderByCash
CASH_SETPURCHASEORDERRESULT, // 3. Record cash purchase response - P_SetPurchaseOrderResult
CASH_ROLLBACKPURCHASEORDERBYCASH, // 4. Cash purchase cancel request - P_RollbackPurchaseOrderByCash
CASH_SETROLLBACKPURCHASEORDERRESULT, // 5. Record cash purchase cancel response - P_SetRollbackPurchaseOrderResult
CASH_GETPETALBALANCE, // 1. Query Petal balance - P_GetPetalBalance
CASH_PURCHASEITEMBYPETAL, // 2. Purchase with Petal - P_PurchaseItemByPetal
CASH_ROLLBACKPURCHASEITEMBYPETAL, // 3. Cancel Petal purchase - P_RollbackPurchaseItemByPetal
	*/

	CASH_BALANCEINQUIRY,	// Balance inquiry
	CASH_BUY,				// Purchase product
	CASH_PACKAGEBUY,		// Purchase package item
	CASH_DELUSER,			// Disconnect user
	// CASH_MODGIVEFAILFLAG, // Update failed cash item (put back into inventory)
	CASH_GIFT,				// Gift product
	CASH_PACKAGEGIFT,		// Gift package item
	CASH_COUPON,			// Coupon  

	CASH_VIPBUY,
	CASH_VIPGIFT,

	CASH_SALEABORTLIST,		// Purchase suspended
#if defined(PRE_ADD_CASH_REFUND)	
	CASH_MOVE_CASHINVEN,	// Move to cash inventory (normal)
	CASH_PACKAGE_MOVE_CASHINVEN,	// Move to cash inventory (package)
	CASH_REFUNDCASH,		// Cash refund	
#endif
#ifdef PRE_ADD_LIMITED_CASHITEM
	CASH_LIMITEDITEM_INFOLIST,		//Get limited-quantity cash item list (get sales count)
	CASH_CHANGEDLIMITEDITEM_INFOLIST,	//Quantity-limited cash item info modified during live service
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
};

//----------------------------------------------------------------------------------------------------------------------------
//	Cash (Nexon)	CashServer <-> NISMS
//----------------------------------------------------------------------------------------------------------------------------
enum eCashKor
{
	CASH_INITIALIZE = 0x01,							// Initialize game server connection
	CASH_HEARTBEAT = 0x02,							// Check status with connected server
	CASH_CHECK_BALANCE = 0x11,						// Check user balance
	CASH_CHECK_BALANCE_WITH_NOT_REFUNDABLE = 0x12,	// Check user balance and also return non-refundable amount (NA & EU)
	CASH_CHECK_BALANCE_WITH_PAYMENT_TYPE = 0x13,	// Check user balance according to PaymentType (payment amount type)
	CASH_PURCHASE_ITEM = 0x21,						// Purchase product (bulk product purchase)
	CASH_PURCHASE_ITEM_DISCOUNT = 0x26,				// Purchase item according to PaymentType (payment amount type). When only deducting cash.
	CASH_PURCHASE_ITEM_EXTEND = 0x23,				// Item purchase by charge method (bulk item purchase) (NA & EU)
	CASH_PURCHASE_ITEM_ATTRIBUTE = 0x24,			// When applying dynamic Attribute values upon product purchase (bulk product purchase)
	CASH_PURCHASE_ITEM_REFUND = 0x27,				// Request refund (withdrawal) for purchased product
	CASH_PURCHASE_GIFT = 0x22,						// Gift product (bulk product gift)
	CASH_PURCHASE_GIFT_ATTRIBUTE = 0x25,			// When applying dynamic Attribute values upon gifting a product (bulk product gift)
	CASH_COUPON_INQUIRY = 0x31,						// Query coupon usage status
	CASH_COUPON_INQUIRY2 = 0x33,					// Query coupon usage status
	CASH_COUPON_USING = 0x32,						// Use coupon
	CASH_INVENTORY_INQUIRY = 0x41,					// Query NISMS Inventory
	CASH_INVENTORY_INQUIRY_ORDER = 0x42,			// Query NISMS Inventory (basic order info)
	CASH_INVENTORY_INQUIRY_PACKAGE = 0x43,			// Query NISMS Inventory (package item info)
	CASH_INVENTORY_INQUIRY_LOTTERY = 0x44,			// Query NISMS Inventory (lottery item info)
	CASH_INVENTORY_CHECK_READABLE = 0x45,			// Check if there are unread items in NISMS Inventory
	CASH_INVENTORY_INQUIRY_REDUCTION = 0x46,		// Query cash inventory items when only cash inventory is used without storing item info in NISMS
	CASH_INVENTORY_CHECK = 0x40,					// Check NISMS Inventory (check order info)
	CASH_INVENTORY_INQUIRY_READ = 0x49,				// Mark NISMS Inventory item as read
	CASH_INVENTORY_PICKUP = 0x4A,					// Fetch item from NISMS Inventory to Game
	CASH_INVENTORY_PICKUP_ONCE = 0x4C,				// Fetch product info after PickUp
	CASH_INVENTORY_PICKUP_FOR_PACKAGE = 0x4D,		// Process individual items within a package product
	CASH_INVENTORY_PICKUP_ROLLBACK = 0x4B,			// Rollback for Pickup
	CASH_INVENTORY_PICKUP_ROLLBACK_FOR_PACKAGE = 0x4E,	// Restore after executing PickUp For Package
	CASH_INVENTORY_CLEAR = 0x48,					// Delete NISMS Inventory (delete order info due to account withdrawal)
	CASH_PRODUCT_INQUIRY_XML = 0x51,				// Query product (returns all product info in XML format)
	CASH_PRODUCT_INQUIRY = 0x52,					// Query product
	CASH_PRODUCT_INQUIRY2 = 0x55,					// 
	CASH_HIDDEN_PRODUCT_INQUIRY_XML = 0x53,			// Query product (items not visible in the sale list)
	CASH_HIDDEN_PRODUCT_INQUIRY = 0x54,				// Query product
	CASH_CATEGORY_INQUIRY = 0x61,
	CASH_LOGIN = 0xB0,
	CASH_LOGIN_WITH_MID = 0xB3,
	CASH_LOGOUT = 0xB1,
	CASH_REFUND = 0x80,								// Cash refund
};
