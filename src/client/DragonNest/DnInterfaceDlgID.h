#pragma once

// Note : Global ID of top-level window without a parent
//
enum DIALOG_ID
{
	NONE_DIALOG = 0,
	REBIRTH_DIALOG,
	GATEQUESTION_DIALOG,
	NPC_DIALOG,
	FADE_DIALOG,
	ACCEPT_DIALOG,
	PRIVATE_MARKET_REQ_DIALOG,
	DUNGEON_ENTER_DIALOG,
	DUNGEON_MOVE_DIALOG,
	DUNGEON_CLEAR_MOVE_DIALOG,
	STAGEGIVEUP_MOVE_DIALOG,
	ACCEPT_REQUEST_DIALOG,
	DAKRLAIR_ENTER_DIALOG,
	PARTY_JOIN_FAIL_DIALOG,
	ACCEPT_REQUEST_PARTYASK_DIALOG,
	UI_DISABLE_DIALOG,
	GATE_MAP_INDEX_SELECT_DIALOG,
	MAINQUEST_DIALOG,
	
	// Note: Play info dialog ID is 
	//		Base ID increments according to character index.
	//
	PLAYERINFO_DIALOG = 1000,
	PLAYERDELETEWAIT_DIALOG = 1100,

	// Note : MessageBox text and ID must match.
	//		Refer to UIString.xls for text.
	//
	MESSAGEBOX_1 = 100001,	// Enter ID.
	MESSAGEBOX_2,			// Please enter password.
	MESSAGEBOX_3,			// Unregistered ID.
	MESSAGEBOX_4,			// Passwords do not match.
	MESSAGEBOX_5 = 100005,	// Unable to retrieve character list.
	MESSAGEBOX_6,			// Deleted characters cannot be restored.\nAre you sure you want to delete?
	MESSAGEBOX_7,			// Character creation failed.
	MESSAGEBOX_8,			// Cannot enter village.
	MESSAGEBOX_9,			// Cannot connect to login server.
	MESSAGEBOX_10,			// Disconnected from the server.
	MESSAGEBOX_11,			// Please select a character.
	MESSAGEBOX_12,			// Character deletion failed.
	MESSAGEBOX_13,			// Enter name
	MESSAGEBOX_14,			// Duplicate name exists.
	MESSAGEBOX_15,			// Enter party name.
	MESSAGEBOX_16,			// Failed to create party.
	MESSAGEBOX_17,			// Cannot join party.
	MESSAGEBOX_18,			// Cannot leave party.
	MESSAGEBOX_19,			// Failed to disband party.
	MESSAGEBOX_20,			// Kicked
	MESSAGEBOX_21,			// Do you want to exit the game?
	MESSAGEBOX_22,			// "Version mismatch. Please copy all executables and resources again from \\\\eye-ad\\ToolData\\Build\\2_Quarter."
	MESSAGEBOX_23,			// "Destroy the item.\\nDestroyed items cannot be permanently restored."
	//MESSAGEBOX_32 = 100032, // Duplicate login.
	MESSAGEBOX_33 = 100033,	// Please select a server.
	MESSAGEBOX_34,			// All party members must be gathered at the exit portal.
	MESSAGEBOX_35,			// Enter password.
	MESSAGEBOX_36,			// Select a channel.
//blondymarry start
    MESSAGEBOX_47 = 100047,			// You must leave the party to change channels.
//blondymarry end
	MESSAGEBOX_62 = 100062,	// Would you like to play the tutorial?
	MESSAGEBOX_FRIEND_QUERY = 1418,
	MESSAGEBOX_FRIEND_ADDED = 1419,
	MESSAGEBOX_GUILD_CREATE				= 1504,		//	Creating guild '%s'.
	MESSAGEBOX_FRIEND_GROUP_DUPNAME,				//	Cannot add because the name already exists in the current group names
	MESSAGEBOX_FRIEND_GROUP_FILTERWORD,				//	Cannot add because it is a forbidden word
	MESSAGEBOX_FRIEND_DELETE,						//	Are you sure you want to delete?
	MESSAGEBOX_BLOCK_ADD_USER_FAIL,					//	Please enter the name of the user to add
	MESSAGEBOX_BLOCK_FRIEND_DELETE,					//	%s has been blocked and removed from the friend list.
	MESSAGEBOX_BLOCK_DELETE,						//	%s has been removed from the block list
	MESSAGEBOX_BLOCK_SERVER_ERROR_MSG,				//	Handle ServerMessageBox
	MESSAGEBOX_PARTY_INVITE_NONE,					//	Enter the name of the character to invite
	MESSAGEBOX_MIN_MAX_LEVEL_ERROR,					//	Max level cannot be lower than min level. (Used in both party creation and unmanned shop item search level input)
	MESSAGEBOX_PARTY_CREATE_FAIL_MAX_LEVEL,			//	Max level cannot be lower than your own level.
	MESSAGEBOX_PARTY_RECONN_LAST_DUNGEON,			//	Prompt whether to reconnect on abnormal raid termination
	MESSAGEBOX_PARTY_RECONN_ON_DGNCLEAR,			//  When joining mid-dungeon-clear after abnormal raid termination
	MESSAGEBOX_DISABLE_ALL,							//	Message box that disables input
	MESSAGEBOX_REQEUST_SPECIALBOX,                  //  Approve special storage

	//
};
