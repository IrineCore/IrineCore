#pragma once

#include "Connection.h"

//----------------------------------------------------------------------------------
//								CDNMasterConnection
//
//	User-related packets sent to Master must use AccountDBID instead of ConnectionUID.
//  Finding user by AccountDBID to prevent duplicate check.
//----------------------------------------------------------------------------------

class CDNUserSession;
class CDNFarmGameRoom;
class CDNPeriodQuest;
class CDNMasterConnection: public CConnection
{
private:
	bool m_bConnectCompleted;
	// Squeeze game server in here... sorry.
	char m_cStepIndex;				// Current guild war step in progress
	short m_wScheduleID;			// Schedule ID
	int m_nBlueTeamPoint;			// Blue team score
	int m_nRedTeamPoint;			// Red team score
	int m_cSecretTeam;				// Team receiving secret bonus
	int m_nSecretRandomSeed;		// Secret mission creation key
	int m_vSecretMissionID[GUILDWARMISSION_MAXGROUP];
	char m_cGuildWarFinalPart;		// Main tournament schedule for the current round in progress.
	TGuildUID m_PreWinGuildUID;		// Previous round winning guild UID	
	// Full schedule
	TGuildWarEventInfo m_sGuildWarSchedule[GUILDWAR_STEP_END-1];
	// Finals schedule
	TGuildWarEventInfo m_sGuildWarFinalSchedule[GUILDWAR_FINALPART_MAX-1];

public:
	BYTE m_cWorldSetID;
	BYTE m_cGameID;

public:
	CDNMasterConnection(void);
	~CDNMasterConnection(void);

	void SetInComplete() { m_bConnectCompleted = false; }
	bool GetActive();
	void Reconnect();

	bool PreMessageProcess(int nMainCmd, int nSubCmd, char * pData, int nLen);
	int MessageProcess(int nMainCmd, int nSubCmd, char * pData, int nLen);

	//Server Operator
	bool SendGameServerInfoList(unsigned long * iIP, unsigned short * iPort, unsigned char * cIdx, bool * margin, int nManageID);
	void SendGameFrame(int nServerIdx, int nWholeRoonCnt, int nRoomCnt, int nUserCnt, int nMinFrame, int nMaxFrame, int nAvrFrame, bool bHasMargin);

	//User Sync
	void SendAddUserList(CDNUserSession *pSession);
	void SendEndofVillageInfo(UINT nServerID);
	void SendConnectComplete();
	void SendDelUser(UINT nAccountDBID, bool bIsIntended, UINT nSessionID);
	void SendCheckUser(UINT nSessionID);
	void SendEnterGame(UINT nAccountDBID, int nRoomID, int nServerIdx);
	void SendReconnectLogin(UINT nAccountDBID);

	//WorldUserState
	void SendUpdateWorldUserState(const WCHAR * pName, int nMapIdx);

	//GameRoom
#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
	int SendSetRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int iRoomID, BYTE cVillageID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto, ePartyType Type = _NORMAL_PARTY, WorldPvPMissionRoom::Common::eReqType eWorldReqType = WorldPvPMissionRoom::Common::NoneType );
#else
	int SendSetRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int iRoomID, BYTE cVillageID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto, ePartyType Type = _NORMAL_PARTY );
#endif
#else
	int SendSetRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int iRoomID, BYTE cVillageID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto );
#endif
	void SendSetTutorialRoomID(UINT nAccountDBID, int iServerIdx, int nRoomID, int nLoginServerID);

	//ChangeServer
	void SendRequestNextVillageInfo( UINT uiAccountDBID, int nMapIndex, int nGateIndex, bool bRetrunVillage, INT64 nItemSerial = 0 );
	void SendRequestNextVillageInfo(int nMapIndex, int nEnteredGateIndex, bool bReturnVillage, CDNGameRoom * pRoom);
	bool SendRebirthVillageInfo(UINT nAccountDBID, int nLastVillageMapIdx);
	void SendPvPGameToPvPLobby( const UINT uiAccountDBID, const UINT uiPvPIndex, const int iLastVillageMapIndex, bool bIsLadderRoom );
	void SendLoginState(UINT nAccountDBID);
	void SendVillageState(UINT nAccountDBID, int nMapIndex, int nRoomID);

	//Friend
	void SendFriendAddNotify(UINT nAddedAccountDBID, const WCHAR * pAddName);

	// chat
#ifdef PRE_ADD_DOORS
	void SendPrivateChat(UINT nFromAccountDBID, WCHAR *pwszToCharacterName, char cType, WCHAR *pwszChatMsg, short wChatLen, INT64 biDestCharacterDBID = 0);
#else		//#ifdef PRE_ADD_DOORS
	void SendPrivateChat(UINT nFromAccountDBID, WCHAR *pwszToCharacterName, char cType, WCHAR *pwszChatMsg, short wChatLen);
#endif		//#ifdef PRE_ADD_DOORS
	void SendChat(char cType, UINT nFromAccountDBID, const WCHAR *pwszChatMsg, short wChatLen);
	void SendWorldSystemMsg(UINT nFromAccountDBID, char cType, int nID, int nValue, const WCHAR* pwszToCharacterName = NULL);
#if defined( PRE_PRIVATECHAT_CHANNEL )
	void SendPrivateChannelChat(char cType, UINT nFromAccountDBID, const WCHAR *pwszChatMsg, short wChatLen, INT64 nChannelID);
#endif

	// Should symbol item be discarded?
	//void SendLocalSymbolData(UINT nAccountDBID, TSymbolItem *SymbolDataArray);

	//Cheat Notice
	void SendNoticeFromClient(const WCHAR * pMsg, const int nLen);
	void SendBanUser(UINT nAccountDBID, const WCHAR * pCharacterName);

	// PvP
	void SendPvPRoomSyncOK( const UINT uiPvPIndex );
	void SendPvPLadderRoomSync( INT64 biRoomIndex, INT64 biRoomIndex2, LadderSystem::RoomState::eCode State );
	void SendPvPBreakIntoOK( const UINT uiPvPIndex, const UINT uiAccountDBID );
	void SendPvPCommand( const UINT uiCommand, const UINT uiPvPIndex, const UINT uiRoomIndex );
	void SendPvPChangeMemberGrade(UINT nAccountDBID, USHORT nType, UINT nSessionID, bool bAsign);
	void SendPvPSwapMemberIndex(UINT nAccountDBID, BYTE cCount, const TSwapMemberIndex * pIndex);
	void SendPvPRealTimeScore(UINT nGuildDBID, int nScore);
	void SendPvPGuildWarResult(UINT nWinGuildDBID, UINT nLoseGuildDBID, bool bIsDraw);
	void SendPvPDetectCrash(UINT nRoomID, UINT nPvPIndex);

	//Guild
	void SendChangeGuildInfo(const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iInt1 = 0, int iInt2 = 0, int iInt3 = 0, int iInt4 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL);
	void SendChangeGuildMemberInfo(const TGuildUID pGuildUID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iInt1 = 0, int iInt2 = 0, INT64 biInt64 = 0, LPCWSTR pText = NULL, bool bReturn = false);
	void SendGuildChat(const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen);
	void SendGuildChangeName(const TGuildUID pGuildUID, LPCWSTR lpwszGuildName);
	void SendUpdateGuildExp(const TGuildUID pGuildUID, const TAModGuildExp* pPacket);
	void SendAddGuildWarPoint(char cTeamType, TGuildUID GuildUID, int nAddPoint);

	//Party
	bool SendInvitePartyMember(CDNGameRoom * pGameRoom, int nMapIdx, int nChannelID, UINT nAccountDBID, const WCHAR * pwszInviterName, const WCHAR * pwszInvitedName, int * pPassClassIds, int nPermitLevel);
	void SendInvitedMemberReturnMsg(UINT nAccountDBID, int nRetCode);
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	void SendGetPartyIDResult( UINT nSenderAccountDBID , TPARTYID PartyID );
	void SendResPartyAskJoin(int iRet, UINT uiReqAccountDBID, const WCHAR* pwszReqChracterName = NULL );
	void SendAskJoinAgreeInfo(const WCHAR* pwszAskerCharName, TPARTYID PartyID, int iPassword);
#endif

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendSyncSystemMail( TASendSystemMail* pMail );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendMasterSystemSyncSimpleInfo( INT64 biCharacterDBID, MasterSystem::EventType::eCode Type );
	void SendMasterSystemSyncGraduate( INT64 biCharacterDBID, WCHAR* pwszCharName );
	void SendMasterSystemSyncConnect( bool bIsConnect, WCHAR* pwszCharName, const TMasterSystemData* pMasterSystemData );

	bool SendFarmDataLoaded(int nGameServerIdx, UINT nRoomID, UINT nFarmDBID, int nMapID, int nFarmMaxUser);
	bool SendFarmUserCount(UINT nFarmDBID, UINT nFarmRoomID, int nCurUserCount, bool bStarted);
	bool SendFarmIntendedDestroy(UINT nFarmDBID);
	void SendFarmSync( INT64 biCharacterDBID, Farm::ServerSyncType::eType Type );
	void SendFarmSyncAddWater( INT64 biCharacterDBID, WCHAR* pwszCharName, int iAddWaterPoint );

	void SendLoadUserTempData(UINT uiAccountDBID);
	void SendSaveUserTempData(CDNUserSession* pSession);
	void SendNotifyGift(UINT nToAccountDBID, INT64 biToCharacterDBID, bool bNew, int nGiftCount);
	void SendGuildMemberLevelUp (TGuildUID GuildUID, INT64 nCharacterDBID, char cLevel);

	void GuildWarReset();
	inline void SetGuildWarStepIndex(char cStepIndex) { m_cStepIndex=cStepIndex;};
	inline char GetGuildWarStepIndex() { return m_cStepIndex;};
	inline void SetGuildWarScheduleID(short wScheduleID) { m_wScheduleID=wScheduleID;};
	inline short GetGuildWarScheduleID() { return m_wScheduleID;};	
	inline void SetGuildWarFinalPart(char cFinalPart ) { m_cGuildWarFinalPart = cFinalPart;};
	inline char GetGuildWarFinalPart() { return m_cGuildWarFinalPart;};

	inline void SetTeamPoint(int nBlueTeamPoint, int nRedTeamPoint) { m_nBlueTeamPoint=nBlueTeamPoint; m_nRedTeamPoint=nRedTeamPoint; CalcTeamSecret();};
	inline int GetBlueTeamPoint() { return m_nBlueTeamPoint; };
	inline int GetRedTeamPoint() { return m_nRedTeamPoint; };
	inline void AddBlueTeamPoint(int nAddPoint) { m_nBlueTeamPoint += nAddPoint; CalcTeamSecret();};
	inline void AddRedTeamPoint(int nAddPoint) { m_nRedTeamPoint += nAddPoint; CalcTeamSecret();};

	bool IsSecretMission(BYTE cTeamCode, int nMissionID);
	void CalcTeamSecret();
	void SetGuildSecretMission(MASetGuildWarSecretMission* pSecret);

	__time64_t GetGuildWarFinalStatTime(){ return m_sGuildWarSchedule[GUILDWAR_STEP_REWARD-1].tBeginTime; }
	void SetGuildWarFinalStartTimeForCheat( __time64_t _tTime ){ m_sGuildWarSchedule[GUILDWAR_STEP_REWARD-1].tBeginTime = _tTime; }

	// Previous round winning guild
	inline TGuildUID GetPreWinGuildUID() { return m_PreWinGuildUID; };
	inline void SetPreWinGuildUID(TGuildUID GuildUID) { m_PreWinGuildUID = GuildUID;};	

	void SetGuildWarSechdule(MASetGuildWarEventTime* SetGuildWarEventTime);
	void SendDuplicateLogin(UINT nAccountDBID, bool bIsDetach, UINT nSessionID);
	void SendCheckLastDungeonInfo( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsCheck, const WCHAR* pwszPartyName );
#if defined( PRE_PARTY_DB )
	void SendConfirmLastDungeonInfo( int iRet, UINT uiAccountID, BreakInto::Type::eCode BreakIntoType );
#else
	void SendConfirmLastDungeonInfo( int iRet, UINT uiAccountID );
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_WORLDCOMBINE_PARTY )
	void SendDelWorldParty( int nPrimaryIndex );
	void SendWorldPartyMember( UINT nAccountDBID, INT64 biCharacterDBID, TPARTYID PartyID, short nCount, Party::MemberData *MemberData, int nRet );
#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	void SendWorldCombineGameServer();
#endif
#endif
#if defined( PRE_ADD_BESTFRIEND )
	void SendLevelUpBestFriend(BYTE cLevel, LPCWSTR lpwszCharacterName);
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
	void SendAddPrivateChatChannel(TPrivateChatChannelInfo tPrivateChatChannel);
	void SendAddPrivateChatChannelMember(INT64 nChannelID, TPrivateChatChannelMember Member);
	void SendInvitePrivateChatChannel(INT64 nChannelID, UINT nAccountDBID, WCHAR* wszInviteCharacterName);
	void SendInvitePrivateChatChannelResult(int nRet, UINT nAccountDBID);
	void SendDelPrivateChatChannelMember(PrivateChatChannel::Common::eModType eType, INT64 nChannelID, INT64 biCharacterDBID, const WCHAR* wszKickName);
	void SendKickPrivateChatChannelMemberResult(INT64 biCharacterDBID);
	void SendModPrivateChatChannelInfo(INT64 nChannelID, PrivateChatChannel::Common::eModType eModType, int nPassWord, INT64 biCharacterDBID);
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
	void SendDelWorldPvPRoom( char cWorldSetID, int nRoomIndex );
	void SendWorldPvPRoomJoinResult( char cWorldSetID, int nRet, UINT uiAccountDBID );
	void SendWorldPvPRoomGMCreateResult( char cWorldSetID, int nRet, UINT uiAccountDBID, TWorldPvPRoomDBData* Data, UINT uiPvPIndex );
#endif

#if defined( PRE_ADD_MUTE_USERCHATTING)
	void SendMuteUserFind(UINT uiGmAccountDBID, const WCHAR * pMuteCharacterName, int nMuteTime);
#endif	// #if defined( PRE_ADD_MUTE_USERCHATTING)

#if defined( PRE_MOD_71820 )
	void SendNotifyMail(UINT nToAccountDBID, INT64 biToCharacterDBID, short wTotalMailCount, short wNotReadMailCount, short w7DaysLeftCount, bool bNewMail);
#endif // #if defined( PRE_MOD_71820 )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void SendAddAlteiaWorldSendTicketResult( int nRetCode, INT64 biSendCharacterDBID );
#endif
};
