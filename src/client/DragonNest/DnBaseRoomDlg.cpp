#include "stdafx.h"
#include "DnBaseRoomDlg.h"
#include "DnInterface.h"


bool CDnBaseRoomDlg::IsMaster( UINT uiUserState )
{
	if(!uiUserState) 
		return false; 

	bool bMaster = false;

	if( uiUserState & PvPCommon::UserState::Captain ) 
		bMaster = true;

	return bMaster;
}

bool CDnBaseRoomDlg::IsGroupCaptain( UINT uiUserState )
{
	if(!uiUserState) 
		return false; 

	bool bGroupCaptain = false;

	if( uiUserState & PvPCommon::UserState::GroupCaptain )
		bGroupCaptain = true;

	return bGroupCaptain;
}


bool CDnBaseRoomDlg::IsReady( UINT uiUserState )
{ 
	if(!uiUserState) 
		return false; 

	bool bReady = false;

	if(uiUserState & PvPCommon::UserState::Ready)
		bReady = true;
	
	return bReady;
}


bool CDnBaseRoomDlg::IsStartingGame( UINT uiUserState )
{ 
	if(!uiUserState) 
		return false; 

	bool bStartingGame = false;

	if(uiUserState & PvPCommon::UserState::Starting)
		bStartingGame = true;

	return bStartingGame;
}


bool CDnBaseRoomDlg::IsInGame( UINT uiUserState )
{ 
	if(!uiUserState) 
		return false;

	bool bIsInGame = false;

	if( uiUserState & PvPCommon::UserState::Syncing )
		bIsInGame = true;

	if( uiUserState & PvPCommon::UserState::Playing )
		bIsInGame = true;

	return bIsInGame;
}


void CDnBaseRoomDlg::ErrorHandler(UINT ErrorCode)
{
	int UIErrorString = 0;

	switch( ErrorCode )
	{
	case ERROR_PVP_ALREADY_PVPROOM:
		return;
	case ERROR_PVP_JOINROOM_NOTFOUNDROOM:
		UIErrorString = PVPBaseRoom::JOINROOM_NOTFOUNDROOM;
		break;
	case ERROR_PVP_JOINROOM_BANUSER:
		UIErrorString = PVPBaseRoom::JOINROOM_BANUSER;
		break;
	case ERROR_PVP_JOINROOM_FAILED:
		UIErrorString = PVPBaseRoom::JOINROOM_FAILED;
		break;
	case ERROR_PVP_JOINROOM_MAXPLAYER:
		UIErrorString = PVPBaseRoom::JOINROOM_MAXPLAYER;
		break;	
	case ERROR_PVP_JOINROOM_INVALIDPW:
		UIErrorString = PVPBaseRoom::JOINROOM_INVALIDPW;
		break;
	case ERROR_PVP_CANTCHANGETEAM_NOSLOT:
		UIErrorString = PVPBaseRoom::JOINROOM_CHANGE_TEAM_SLOT_NOT_ENOUGH;
		break;
	case ERROR_PVP_JOINROOM_LEVELLIMIT:
		UIErrorString = PVPBaseRoom::JOINROOM_LEVELLIMIT;
		break;
	case ERROR_GENERIC_MASTERCON_NOT_FOUND :// Master server down
		UIErrorString = PVPBaseRoom::JOINROOM_MASTERSERVERERROR;
		break;
	case ERROR_PVP_CANTSTART_LESS_MINTEAMPLAYER: //Fewer than minimum starting team members
		UIErrorString = PVPBaseRoom::JOINROOM_NOTENOUGHPLAYER;
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, UIErrorString ));
		break;
	case ERROR_PVP_CANTSTART_MORE_MAXDIFFTEAMPLAYER :// Team member count difference exceeds limit
		UIErrorString = PVPBaseRoom::JOINROOM_TEAMBALANCE;
		GetInterface().AddChatMessage(CHATTYPE_SYSTEM, L"", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, UIErrorString ));
		break;
	case ERROR_PVP_STARTPVP_FAILED:// Game start failed (common error)
		UIErrorString = PVPBaseRoom::JOINROOM_UNKOWNFAILED;
		break;
	case ERROR_GENERIC_GAMECON_NOT_FOUND://Game server down 
		UIErrorString = PVPBaseRoom::JOINROOM_UNKOWNFAILED;
		break;
	case ERROR_PVP_MODIFYROOM_FAILED:     // On room edit failure
		UIErrorString = PVPBaseRoom::JOINROOM_MODIFY_FAILED;     // "Room information was not changed due to a temporary error."
		break;
	case ERROR_PVP_CREATEROOM_FAILED:     // On room creation failure
		UIErrorString = PVPBaseRoom::JOINROOM_MASTERSERVERERROR; // "Moving to lobby due to an error"
		break;
	case ERROR_PVP_JOINROOM_MAXOBSERVER:
		UIErrorString = PVPBaseRoom::JOINROOM_OBSERVERFAILED; 
		break;
	case ERROR_PVP_CANTCHANGETEAM_CHANGECAPTAIN_FAILED:      
		UIErrorString = PVPBaseRoom::JOINROOM_CHANGECAPTAINFAILDED; 
		break;
	case ERROR_PVP_CANTCHANGETEAM_LEVELLIMIT:
		UIErrorString = PVPBaseRoom::JOINROOM_MOVETOPLAYERFAILED;
		break;
	case ERROR_PVP_JOINROOM_DONTALLOW_EVENTROOM_OBSERVER:
		UIErrorString = PVPBaseRoom::JOINROOM_DONTALLOW_EVENTROOM_OBSERVER;
		break;
	case ERROR_PVP_STARTPVP_FAIL_READYMASTER:
		UIErrorString = PVPBaseRoom::JOINROOM_START_FAIL_READYMASTER;
		break;
#ifdef PRE_PVP_GAMBLEROOM
	case ERROR_PVP_GAMBLEROOM_NOTCOIN:
		UIErrorString = PVPBaseRoom::JOINROOM_START_FAIL_GAMBLEROOM_NOTCOIN;
		break;
	case ERROR_PVP_GAMBLEROOM_NOTPETAL:
		UIErrorString = PVPBaseRoom::JOINROOM_START_FAIL_GAMBLEROOM_NOTPETAL;
		break;
#endif // PRE_PVP_GAMBLEROOM
	};

	GetInterface().MessageBox( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, UIErrorString ), MB_OK );
}