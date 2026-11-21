#pragma once
#include "EtUIDialog.h"


namespace PVPBaseRoom
{
	enum ErrorCode{
		JOINROOM_MASTERSERVERERROR = 121007,
		JOINROOM_NOTENOUGHPLAYER = 121009,
		JOINROOM_TEAMBALANCE = 121010,
		JOINROOM_NOTFOUNDROOM = 120116,
		JOINROOM_FAILED = 120116,
		JOINROOM_MAXPLAYER = 120114,
		JOINROOM_INVALIDPW = 120113,
		JOINROOM_UNKOWNFAILED = 121011,
		JOINROOM_BANUSER = 121065,
		JOINROOM_LEVELLIMIT = 121072,
		JOINROOM_MODIFY_FAILED = 120067, // "Room information was not changed due to a temporary error."
		JOINROOM_CHANGE_TEAM_SLOT_NOT_ENOUGH = 103212,

		JOINROOM_OBSERVERFAILED = 120114,
		JOINROOM_CHANGECAPTAINFAILDED = 120116,
		JOINROOM_MOVETOPLAYERFAILED = 121072,

		JOINROOM_DONTALLOW_EVENTROOM_OBSERVER = 126038,
		JOINROOM_START_FAIL_READYMASTER = 126278,
#ifdef PRE_MOD_PVPOBSERVER
		JOINROOM_START_FAIL_GAMBLEROOM_NOTCOIN = 8400,
		JOINROOM_START_FAIL_GAMBLEROOM_NOTPETAL = 8401
#endif // PRE_MOD_PVPOBSERVER
	};
}

class CDnBaseRoomDlg 
{
public :
	enum UITeam
	{
		TeamA = 0,
		TeamB = 1,
		MAXTeamNUM = 2,
	};

	enum SlotState
	{
		InPlayer = 0,
		Open = 1,
		Closed = 2,
	};
	
	static void ErrorHandler( UINT ErrorCode ); 
	static bool IsGuildMaster( UINT uiUserState ){ return uiUserState&PvPCommon::UserState::GuildWarCaptain?true:false; }
	static bool IsMaster( UINT uiUserState );
	static bool IsReady( UINT uiUserState );
	static bool IsGroupCaptain( UINT uiUserState );
	static bool IsStartingGame( UINT uiUserState );
	static bool IsInGame( UINT uiUserState );
	
	virtual void RoomSetting( PvPCommon::RoomInfo* pInfo ,bool IsModified){};
	virtual void InsertPlayer(int team  , UINT cUserState , UINT    uiSessionID , int iJobID , BYTE  cLevel , BYTE  cPVPLevel , const WCHAR * szPlayerName, char cPosition = 0 ){};
#ifdef PRE_MOD_PVPOBSERVER
	virtual void InsertObserver( UINT uiUserState, UINT uiSessionID ){}
#endif // PRE_MOD_PVPOBSERVER
	virtual void SetRoomState( UINT cRoomState ){};
	virtual void RemovePlayer( UINT uiSessionID, PvPCommon::LeaveType::eLeaveType eType = PvPCommon::LeaveType::Normal ){};
	virtual void UpdateSlot( bool ForceAll ){};
	virtual void SetUserState( UINT uiSessionID , UINT  cUserState , bool IsOuterCall = false ){};
	virtual int GetCurMapIndex(){ return 0; };
	virtual void UpdatePvPLevel( SCPVP_LEVEL* pPacket ){};
	virtual void BaseProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg ){};
	virtual void ShowPVPPopUpDlg(bool bShow){};
	virtual void PlaySound( byte cType){};

	virtual void ChangeTeam( UINT uiSessionID , int cTeam, char cTeamSlotIndex = 0 ){};

	virtual void EnableKickButton(bool bTrue){};
	virtual void ResetSelectButton(){}; 
	virtual void EnableChangeMasterButton(bool bTrue){};

	virtual UINT GetLocalUserState() const { return 0; }

#ifdef PRE_MOD_PVPOBSERVER	
	virtual bool IsEventRoom() const = 0;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_ADD_PVP_TOURNAMENT
	virtual void OnSwapPosition(const int& srcPos, const int& destPos) {}
#endif
};