#pragma once

#include "Connection.h"

class CDNUser;
//class CDNParty;
class CDNPvP;

class CDNGameConnection: public CConnection
{
private:	
	std::vector <TGameInfo*> m_GameServerInfoList;
	USHORT m_wGameID;
	int m_nRoomCnt;
	int m_nUserCnt;

	volatile bool m_bZeroPopulation;
	BYTE m_cAffinityType;	//eGameServerAffinityType
	int m_nManagedID;
	bool m_bConnectComplete;

	UINT m_uiLastReqEnterGameTick;
	UINT m_uiLastEnterGameTick ;
#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	bool m_bWorldCombineGameServer;
#endif

public:

	CDNGameConnection(void);
	virtual ~CDNGameConnection(void);

	bool GetConnectionCompleted() { return m_bConnectComplete; }

	int MessageProcess(int iMainCmd, int iSubCmd, char * pData, int iLen);

	inline void SetGameID(int id) { m_wGameID = id; }
	inline USHORT GetGameID() { return m_wGameID; }
	inline void SetZeroPopulation(bool bPopulation) { m_bZeroPopulation = bPopulation; }
	inline bool GetZeroPopulation() { return m_bZeroPopulation; }
	inline void SetAffinityType(BYTE cAffinity) { m_cAffinityType = cAffinity; }
	inline BYTE GetAffinityType() { return m_cAffinityType; }
	inline void SetManagedID(int nManagedID) { m_nManagedID = nManagedID; }
	inline int GetManagedID() { return m_nManagedID; }


	//GameConnection Game Registration
	void SendRegistWorldID();
	void SendReqUserList(short nRetCode);
	void SendGameRegistComplete();

	void SetLastReqEnterGameTick();

	//UserState
	void SendAddUserState(const WCHAR * pName, INT64 biChracterDBID, int nLocationState = -1, int nCommunityState = -1, int nChannelID = -1, int nMapIdx = -1);
	void SendDelUserState(const WCHAR * pName, INT64 biChracterDBID);
	void SendUpdateUserState(const WCHAR * pName, INT64 biChracterDBID, int nLocationState, int nCommunityState, int nChannelID, int nMapIdx);
	void SendChangeCharacterName(MAChangeCharacterName* pPacket);

	//GameRoom
	void SendReqTutorialRoomID(CDNUser * pUser, int nLoginServerID);
	bool SendReqRoomID( GameTaskType::eType GameTaskType, CDNUser* pUser, TDUNGEONDIFFICULTY StageDifficulty, int nMeritBonusID, bool bDirectConnect = false );				// Single Room
	void SendReqRoomID(VIMAReqGameID * pPacket, int nMeritBonusID);			// Party Room
	void SendLadderReqRoomID( VIMALadderReqGameID* pPacket );				// Ladder Room
	bool SendReqRoomID( CDNPvP* pPvPRoom, bool bDirectConnect = false );	// PvP Room
#if defined( PRE_ADD_FARM_DOWNSCALE )
	bool SendReqFarmRoomID(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart, int iAttr );
#elif defined( PRE_ADD_VIP_FARM )
	bool SendReqFarmRoomID(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart, Farm::Attr::eType Attr );
#else
	bool SendReqFarmRoomID(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart);
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	bool SendForceStopPvP(int nGameRoomID, UINT nForceWinGuildDBID);
	void SendGuildWarAllStop();
#if defined( PRE_WORLDCOMBINE_PVP )
	void SendWorldPvPReqRoomID( VIMACreateWorldPvPRoom* pPacket, BYTE cVillageID );
#endif
	//Party
	void SendInivitePartyMemberResult(UINT nAccountDBID, const WCHAR * pwszInvitedName, int nRetCode);
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	void SendGetPartyID( UINT nAccountDBID, UINT nSenderAccountDBID );
	void SendReqPartyAskJoin( UINT nAccountDBID, const VIMAReqPartyAskJoin* pPacket );	
	void SendAskJoinAgreeInfo( UINT uiAccountDBID , TPARTYID PartyID, int iPassword );
#endif

	//Each GameServer
	bool GetHasMargin();
	int GetIdleGameServer(int nCnt);
	int GetIdleGameServerByAtt(int Att);
	bool GetGameServerInfoByID(int nGameServerID, USHORT * pPort, char * pIP, USHORT * pTcpPort);

	void SendNextVillageServerInfo(UINT nAccountDBID, int nMapIdx, int nNextMapIdx, int nNextGateIdx, const char * pIP, USHORT nPort, short nRet, INT64 nItemSerial = 0);
	void SendRebirthVillageInfo(UINT nAccountDBID, int nVillageID, int nChannelID, const char *pIp, USHORT nPort, short nRet);
	void SendMovePvPGameToPvPLobby( const UINT uiAccountDBID, const char* pszIP, const USHORT unPort, const short nRet );

	void SendLoginState(UINT nAccountDBID, short nRet);
	void SendVillageState(UINT nAccountDBID, int nRoomID, int nMapIndex, short nRet);

	void SendBreakintoRoom( int iGameRoomID, CDNUser* pJoinUser, BreakInto::Type::eCode BreakIntoType, char cTeamSlotIndex = -1 );

	void SendLadderObserver( int iGameRoomID, CDNUser* pUser );
	void SendPopulationZero(bool bZero);

	//Friend
	void SendFriendAddNotice(UINT nAddedAccountDBID, const WCHAR * pName);

	// party
	//void SendDelPartyMember(int nRoomID, UINT nLeaderAccountDBID, UINT nMemberSessionID, UINT nLeaderSessionID, char cKickKind);

	// chat
	void SendPrivateChat(UINT nToAccountDBID, const WCHAR *pwszFromCharacterName, char cType, const WCHAR *pwszChatMsg, short wChatLen, int nRet = ERROR_NONE);
	void SendChat(char cType, UINT nToAccountDBID, const WCHAR *pwszFromCharacterName, const WCHAR *pwszChatMsg, short wChatLen);
	void SendWorldSystemMsg(const WCHAR *pwszFromCharacterName, char cType, int nID, int nValue, WCHAR* pwszToCharacterName);
#if defined( PRE_PRIVATECHAT_CHANNEL )
	void SendPrivateChannelChat( const WCHAR *pwszFromCharacterName, const WCHAR *pwszChatMsg, short wChatLen, INT64 nChannelID);
#endif

	//notice
	void SendNotice(const WCHAR * pMsg, const int nLen, int nShowSec);
	void SendNoticeServer(int nManagedID, const WCHAR * pMsg, int nLen, int nShowSec);
	void SendNoticeCancel();

	// Cheat related
	void	SendResRecall( CDNUser* pRecallUser, const VIMAReqRecall* pPacket, const int iRecallMapIndex );
	void SendUserRestraint(UINT nAccountDBID);

	// PvP
	void SendPvPGameMode( const CDNPvP* pPvPRoom );
	void SendPvPLadderGameMode( GAMASetRoomID* pPacket, const TPvPGameModeTable* pGameModeTable );
	void SendPvPMemberGrade(UINT nAccountDBID, USHORT nTeam, UINT uiUserState, UINT nChangedSessionID, int nRetCode);
	void SendPvPMemberIndex(USHORT nTeam, const VIMAPVPSwapMemberIndex * pIndex, int nRetCode);
	void SendPvPChangeUserState( const UINT nRoomID, const UINT uiAccountDBID, const UINT uiUserState );
#if defined(PRE_ADD_PVP_TOURNAMENT)
	void SendPvPTournamentUserInfo( CDNPvP* pPvPRoom );
#endif

	void SendDetachUser(UINT nAccountDBID, bool bIsDuplicate=false, UINT nSessionID=0);		// Force disconnect

	void SendNotifyMail(UINT nToAccountDBID, INT64 biToCharacterDBID, short wTotalMailCount, short wNotReadMailCount, short w7DaysLeftCount, bool bNewMail);
	void SendNotifyMarket(UINT nSellerAccountDBID, INT64 biSellerCharacterDBID, int nItemID, short wCalculationCount);
	void SendNotifyGift(UINT nToAccountDBID, INT64 biToCharacterDBID, bool bNew, int nGiftCount);

	//Guild
	void SendDismissGuild(MADismissGuild* pPacket);
	void SendAddGuildMember(MAAddGuildMember* pPacket);
	void SendDelGuildMember(MADelGuildMember* pPacket);
	void SendChangeGuildInfo(MAChangeGuildInfo* pPacket);
	void SendChangeGuildMemberInfo(MAChangeGuildMemberInfo* pPacket);
	void SendGuildChat(MAGuildChat* pPacket);
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void SendDoorsGuildChat(MADoorsGuildChat* pPacket);
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	void SendGuildMemberLevelUp(MAGuildMemberLevelUp* pPacket);
	void SendChangeGuildName(MAGuildChangeName* pPacket);
	void SendChangeGuildMark(MAGuildChangeMark* pPacket);
	void SendUpdateGuildExp(MAUpdateGuildExp* pPacket);
	void SendEnrollGuildWar(MAEnrollGuildWar* pPacket);
	void SendChangeGuildWarStep(MAChangeGuildWarStep* pPacket);
	void SendSetGuildWarPoint(MASetGuildWarPoint* pPacket);
	void SendAddGuildWarPoint(MAAddGuildWarPoint* pPacket);
	void SendSetGuildWarSecretMission(MASetGuildWarSecretMission* pPacket);
	void SendSetGuildwarFinalProcess(char cGuildFinalPart, __time64_t tBeginTime);
	void SendSetGuildWarPreWinGuild(MAGuildWarPreWinGuild* pPacket);
	void SendSetGuildWarTournamentWin(MASetGuildWarTournamentWin* pPacket);
	void SendSetGuildWarSchedule(MASetGuildWarEventTime* pPacket);
	void SendGuildRecruitMember(MAGuildRecruitMember* pPacket);
	void SendAddGuildRewardItem(MAGuildRewardItem* pPacket);
	void SendExtendGuildSize(MAExtendGuildSize* pPacket);
	void SendUpdateGuildWare(int nGuildID);

	// MasterSystem
	void SendMasterSystemSyncSimpleInfo( UINT uiAccountDBID, INT64 biCharacterDBID, MasterSystem::EventType::eCode EventCode );
	void SendMasterSystemSyncJoin( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsAddPupil );
	void SendMasterSystemSyncLeave( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsDelPupil );
	void SendMasterSystemSyncGraduate( UINT uiAccountDBID, MasterSystem::VIMASyncGraduate* pPacket );
	void SendMasterSystemSyncConnect( UINT uiAccountDBID, bool bIsConnect, WCHAR* pwszCharName );

	void SendFarmSync( UINT uiAccountDBID, INT64 biCharacterDBID, Farm::ServerSyncType::eType Type );
	void SendFarmSyncAddWater( UINT uiAccountDBID, WCHAR* pwszCharName, int iAddWaterPoint );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendSyncSystemMail( UINT uiAccountDBID, VIMASyncSystemMail* pMail );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

	void SendPCBangResult(UINT nAccountDBID, CDNUser *pUser);
#if defined(PRE_ADD_REMOTE_QUEST)
	void SendUserTempDataResult(UINT uiAccountDBID, CDNUser *pUser);
#else
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	void SendUserTempDataResult(UINT uiAccountDBID, int nDungeonClearCount, GameQuitReward::RewardType::eType eUserGameQuitRewardType);
#else	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	void SendUserTempDataResult(UINT uiAccountDBID, int nDungeonClearCount);
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#endif	//#if defined(PRE_ADD_REMOTE_QUEST)

	void SendCheckLastDungeonInfo( char cWorldSetID, const VIMACheckLastDungeonInfo* pPacket );
	void SendDeleteBackupDungeonInfo( UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID );

#if defined( PRE_WORLDCOMBINE_PARTY )
	void SendGetWorldPartyMember( MAGetWorldPartyMember *Packet);	
#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	void SetWorldCombineGameServer( bool bFlag ) { m_bWorldCombineGameServer = bFlag; }
	bool GetWorldCombineGameServer() { return m_bWorldCombineGameServer; }
#endif
#endif
#if defined( PRE_ADD_BESTFRIEND )
	void SendCancelBestFriend(MACancelBestFriend* pPacket);	
	void SendCloseBestFriend(MACloseBestFriend* pPacket);
	void SendLevelBestFriend(MALevelUpBestFriend* pPacket);
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
	void SendPrivateChatChannelAdd(MAAddPrivateChannel* pPacket);
	void SendPrivateChatChannelMemberAdd(MAAddPrivateChannelMember* pPacket);
	void SendPrivateChatChannelMemberInvite(MAInvitePrivateChannelMember* pPacket);
	void SendPrivateChatChannelMemberInviteResult(MAInvitePrivateChannelMemberResult* pPacket);
	void SendPrivateChatChannelMemberDel(MADelPrivateChannelMember* pPacket);
	void SendPrivateChatChannelMemberKickResult(MAKickPrivateChannelMemberResult* pPacket);
	void SendPrivateChatChannelMod(MAModPrivateChannel* pPacket);
	void SendPrivateChatChannelModMemberName(MAModPrivateChannelMemberName* pPacket);
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
	void SendMuteUserChatting(MAMuteUserChat* pData);
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	void SendChangeRoomMaster( UINT uiRoomID, INT64 biRoomMasterCharacterDBID, UINT uiRoomMasterSessionID );
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void SendAlteiaWorldSendTicket( MAAlteiaWorldSendTicket *pPacket );
	void SendAlteiaWorldSendTicketResult( MAAlteiaWorldSendTicketResult *pPacket );
#endif

	// Common
private:
	int	_ProcessRebirthVillage( CDNUser* pUser, const int iLastVillageMapIndex );
};
