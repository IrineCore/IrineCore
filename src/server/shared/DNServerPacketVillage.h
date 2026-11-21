#pragma once

#pragma pack(push, 1)

//--------------------------------------------------------------
//	Village Packet
//--------------------------------------------------------------

// Village -> Master
struct VIMARegist				// VIMA_REGIST
{
	BYTE cVillageID;
	int nManagedID;
	char szIP[IPLENMAX];
	short nPort;
};

struct VIMACheckUser			// VIMA_CHECKUSER
{
	UINT nSessionID;
};

struct VIMAAddUserList			// VIMA_ADDUSERLIST
{
	UINT nAccountDBID;
	UINT nSessionID;
	INT64 biCharacterDBID;
	WCHAR wszAccountName[IDLENMAX];
	WCHAR wszCharacterName[NAMELENMAX];
	char cPCBangGrade;	// PC bang grade (0: not PC bang, others: ePCBangGrade)
	char szIP[IPLENMAX];	// Added for fatigue
	char szVirtualIp[IPLENMAX];
	USHORT wChannelID;
	bool bAdult;			// Whether adult or not (fatigue)

	BYTE	cPvPVillageID;
	USHORT	unPvPVillageChannelID;
};

struct VIMADelUser				// VIMA_DELUSER
{
	UINT nAccountDBID;
	UINT nSessionID;
};

struct TMemberVoiceInfo
{
	BYTE cVoiceAvailable;
	UINT nMutedList[PARTYCOUNTMAX];
};

struct TMemberInfo
{
	UINT nAccountDBID;
	BYTE cMemberIndex;
#if defined( PRE_PARTY_DB )
	INT64 biCharacterDBID;
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComebackAppellation;
#endif
	TMemberVoiceInfo VoiceInfo;
};

struct TLadderMemberInfo
{
	UINT	uiAccountDBID;
	UINT	uiTeam;
};

struct TPartyData
{
	UINT nLeaderAccountDBID;
	UINT nLeaderSessionID;
	USHORT nChannelID;
	
#if defined( PRE_PARTY_DB )
	Party::Data PartyData;	
#else
	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemLootRank;
	BYTE cUserLvLimitMin;
	WCHAR wszPartyName[PARTYNAMELENMAX];
	BYTE cUserLvLimitMax;
	BYTE cJobDice;
	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;
	WCHAR wszPartyPass[PARTYPASSWORDMAX];	
	BYTE cUpkeepCount;
	BYTE cMemberMax;
	BYTE cMemberCount;
#endif // #if defined( PRE_PARTY_DB )
	UINT nVoiceChannelID;
	UINT nKickedMemberDBID[PARTYKICKMAX];
	TMemberInfo MemberInfo[PARTYMAX];
};

struct TLadderData
{
	BYTE				cMemberCount;
	TLadderMemberInfo	MemberInfoArr[PvPCommon::Common::MaxPlayer];
};

struct VIMAVillageToVillage		//VIMA_VILLAGETOVILLAGE
{
	BYTE cReqType;
#if defined( PRE_PARTY_DB )
	INT64 biID;
#else
	UINT nID;
#endif // #if defined( PRE_PARTY_DB )
	int nMapIndex;
	int nEnteredGateIndex;
	int nMoveChannel;
	INT64 nItemSerial;

	TPartyData PartyData;
};

struct VIMAEnterVillage			//VIMA_ENTERVILLAGE
{
	UINT nAccountDBID;
	USHORT nChannelID;
};

struct VIMAReconnectLogin		//VIMA_RECONNECTLOGIN
{
	UINT nAccountDBID;
};

struct VIMAProcessDelayed		//VIMA_PROCESSDELAYED
{
	UINT nDelayedTick;
};

struct TVillageUserReport
{
	short nChannelID;
	USHORT nChannelUserCount;
};

struct VIMAUserReport	//VIMA_VILLAGEUSERREPORT
{
	UINT nUserCount;
	USHORT nCount;
	TVillageUserReport Info[VILLAGECHANNELMAX];
};

struct VIMALoginState			// VIMA_LOGINSTATE
{
	UINT nSessionID;
};

struct VIMAReqGameID			// VIMA_REQGAMEID
{
	GameTaskType::eType	GameTaskType;
	BYTE cReqGameIDType;
	TINSTANCEID InstanceID;
	USHORT unVillageChannelID;
	int	nRandomSeed;
	int	nMapIndex;
	char cGateNo;
	char cGateSelect;
	TDUNGEONDIFFICULTY StageDifficulty;
	bool bDirectConnect;
	UINT nFarmDBID;

	//If in a party
	TPartyData PartyData;
};

struct VIMALadderEnterChannel
{
	UINT uiAccountDBID;
	LadderSystem::MatchType::eCode MatchType;
};

struct VIMALadderReqGameID
{
	LadderSystem::MatchType::eCode MatchType;
	INT64		biRoomIndexArr[2];
	USHORT		unVillageChannelID;
	int			iRandomSeed;
	int			iMapIndex;
	TLadderData	LadderData;
};

struct VIMALadderObserver
{
	INT64	biCharacterDBID;
	int		iGameServerID;
	int		iGameServerThreadID;
	int		iGameRoomID;
};

struct VIMALadderInviteUser
{
	UINT	uiAccountDBID;
	WCHAR	wszCharName[NAMELENMAX];
	int nRetCode;
};

struct VIMALadderInviteConfirm
{
	UINT uiAccountDBID;
	WCHAR wszConfirmCharName[NAMELENMAX];
	LadderSystem::CS_INVITE_CONFIRM sInviteComfirm;
};

#if defined( PRE_PARTY_DB )

struct VIMAReqPartyInvite
{
	TPARTYID PartyID;
	int iPartyMinLevel;
	WCHAR wszPartyName[PARTYNAMELENMAX];
	int iPartyPassword;
	int iPartyMemberMax;
	int iPartyMemberCount;
	int iPartyAvrLV;
	UINT uiReqAccountDBID;
	INT64 biReqCharacterDBID;
	WCHAR wszReqCharName[NAMELENMAX];
	int iReqMapIndex;
	WCHAR wszInviteCharName[NAMELENMAX];
};

typedef VIMAReqPartyInvite MAVIReqPartyInvite;

struct VIMAResPartyInvite
{
	int iRet;
	UINT uiReqAccountDBID;
};

struct VIMAPartyInviteDenied
{
	WCHAR wszReqCharName[NAMELENMAX];
	TPARTYID PartyID;
	WCHAR wszInviteCharName[NAMELENMAX];
	BYTE cIsOpenBlind;
};

typedef VIMAPartyInviteDenied MAVIPartyInviteDenided;

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
struct VIMAReqPartyAskJoin
{
	UINT uiReqAccountDBID;
	INT64 biReqCharacterDBID;
	char cReqUserJob;
	BYTE cReqUserLevel;
	WCHAR wszReqChracterName[NAMELENMAX];
	WCHAR wszTargetChracterName[NAMELENMAX];
};
typedef VIMAReqPartyAskJoin MAVIReqPartyAskJoin;

struct VIMAResPartyAskJoin
{
	int iRet;
	UINT uiReqAccountDBID;
	WCHAR wszReqChracterName[NAMELENMAX];
};

struct MAVIResPartyAskJoin
{
	int iRet;
	UINT uiReqAccountDBID;	
};

struct VIMAAskJoinAgreeInfo
{
	TPARTYID PartyID;
	int iPassword;
	WCHAR wszAskerCharName[NAMELENMAX];	
};

struct MAVIAskJoinAgreeInfo
{
	UINT uiAskerAccountDBID;
	TPARTYID PartyID;
	int iPassword;	
};

#endif

#else
struct VIMAReqPartyID			// VIMA_REQPARTYID
{
	UINT nAccountDBID;
	BYTE cMemberMax;
	USHORT nChannelID;

	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemLootRank;
	BYTE cUserLvLimitMin;
	BYTE cUserLvLimitMax;

	UINT nCreateAfterInvite;		//If created by invite, create and invite immediately
	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;
	BYTE cUseVoice;

	BYTE cPartyJobDice;
	BYTE cNameLen;
	BYTE cPassWordLen;
	WCHAR wszBuf[PARTYNAMELENMAX + PARTYPASSWORDMAX];
};
#endif // #if defined( PRE_PARTY_DB )

struct VIMAInvitePartyMemberResult		//VIMA_RETINVITEPARTYMEMBER
{
	int nGameServerID;
	WCHAR wszInviterName[NAMELENMAX];	//Target to receive the result
	WCHAR wszInvitedName[NAMELENMAX];
	int nRetCode;
};

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
struct VIMAGetPartyID
{
	UINT nSenderAccountDBID;	//Requester of the info
	WCHAR wszCharName[NAMELENMAX];
};

struct VIMAGetPartyIDResult
{
	UINT nSenderAccountDBID;	//Requester of the info
	TPARTYID PartyID;
};

struct MAVIGetPartyID
{	
	UINT nAccountDBID;
	UINT nSenderAccountDBID;	//Requester of the info
};

struct MAVIGetPartyIDResult
{
	UINT nSenderAccountDBID;	//Requester of the info
	TPARTYID PartyID;
};
#endif

struct VIMAFriendAddNotice
{
	UINT nAddedAccountDBID;
	WCHAR wszAddName[NAMELENMAX];
};

struct VIMAPrivateChat			// VIMA_PRIVATECHAT
{
	UINT nAccountDBID;
	WCHAR wszToCharacterName[NAMELENMAX];
	char cType;		// Whether GM or not.. (Do we really have to go this far? -_-;)
#ifdef PRE_ADD_DOORS
	INT64 biDestCharacterDBID;
#endif		//#ifdef PRE_ADD_DOORS
	short wChatLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct VIMAChat				// VIMA_CHAT
{
	UINT nAccountDBID;
	char cType;
	short wChatLen;
	int nMapIdx;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct VIMAWorldSystemMsg		// VIMA_WORLDSYSTEMMSG
{
	UINT nAccountDBID;
	char cType;
	int nID;
	int nValue;
	WCHAR wszToCharacterName[NAMELENMAX];
};

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct VIMAPrivateChannelChatMsg		// VIMA_PRIVATECHANNELCHAT
{
	UINT nAccountDBID;
	char cType;	
	short wChatLen;
	INT64 nChannelID;
	WCHAR wszChatMsg[CHATLENMAX];
};
#endif

// VoiceChat
struct VIMAReqVoiceChannelID		//VIMA_REQVOICECHANNELID
{
	TINSTANCEID InstanceID;
	int nPvPLobbyChannelID;
	BYTE cVoiceChannelType;		//eVoiceChannelReqType
};

// PvP
struct VIMAPVP_MOVELOBBYTOVILLAGE
{
	UINT				uiAccountDBID;
};

struct VIMAPVP_CREATEROOM
{
	UINT				uiCreateAccountDBID;
	USHORT				unVillageChannelID;
	UINT				uiVillageMapIndex;
	BYTE				cGameMode;
	int					nEventID;
	TEvent				EventData;
	UINT nGuildDBID[PvPCommon::Common::DefaultGuildCount];
	int nGuildQualifyingScore[PvPCommon::Common::DefaultGuildCount];
#if defined(PRE_ADD_PVP_TOURNAMENT)
	BYTE				cCreateUserJob;
#endif
#if defined( PRE_PVP_GAMBLEROOM )	
	BYTE				cGambleType;
	int					nPrice;
#endif
	CSPVP_CREATEROOM	sCSPVP_CREATEROOM;
};


struct VIMAPVP_CREATEEVENTROOM
{
	BYTE	cGameMode;
};


struct VIMAPVP_MODIFYROOM
{
	UINT				uiAccountDBID;
	BYTE				cGameMode;
	CSPVP_MODIFYROOM	sCSPVP_MODIFYROOM;
};

struct VIMAPVP_LEAVEROOM
{
	UINT	uiLeaveAccountDBID;
};

struct VIMAPVP_CHANGECAPTAIN
{
	UINT				uiAccountDBID;
	CSPVP_CHANGECAPTAIN	sCSPVP_CHANGECAPTAIN;
};

struct VIMAPVP_ROOMLIST
{
	UINT			uiAccountDBID;
	USHORT			unVillageChannelID;
	CSPVP_ROOMLIST	sCSPVP_ROOMLIST;
};

struct VIMAPVP_WAITUSERLIST
{
	UINT				uiAccountDBID;
	USHORT				unVillageChannelID;
	CSPVP_WAITUSERLIST	sCSPVP_WAITUSERLIST;
};

struct VIMAPVP_JOINROOM
{
	UINT			uiAccountDBID;
	BYTE			cUserLevel;
#if defined(PRE_ADD_PVP_TOURNAMENT)
	BYTE			cUserJob;
#endif
	CSPVP_JOINROOM	sCSPVP_JOINROOM;
};

struct VIMAPVP_READY
{
	UINT			uiAccountDBID;
	CSPVP_READY		sCSPVP_READY;
};

struct VIMAPVP_START
{
	UINT		uiAccountDBID;
	CSPVP_START	sCSPVP_START;
};

struct VIMAPVP_RANDOMJOINROOM
{
	UINT	uiAccountDBID;
	BYTE	cUserLevel;
};

struct VIMAPVP_CHANGETEAM
{
	UINT				uiAccountDBID;
	BYTE				cUserLevel;
	CSPVP_CHANGETEAM	sCSPVP_CHANGETEAM;
};

struct VIMAPVP_BAN
{
	UINT				uiAccountDBID;
	CSPVP_BAN			sCSPvPBan;
};

struct VIMAPVP_ENTERLOBBY
{
	UINT	uiAccountDBID;
	USHORT	unVillageChannelID;
};
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
struct PVP_ROOMLIST_RELAY
{
	BYTE	cVillageID;
	UINT	uiAccountDBID;
	int		nSize;
	SCPVP_ROOMLIST		sSCPVP_ROOMLIST;
};
#endif

struct PVP_CHANGECHANNEL
{
	UINT nAccountDBID;
	BYTE cType;
	BYTE cPvPLevel;
	BYTE cSeperateLevel;
	bool bSend;
};

struct VIMAPVP_FATIGUE_OPTION
{
	UINT	uiAccountDBID;
	bool	bFatigueOption;
};
struct VIMAPVPSwapMemberIndex		//VIMA_PVP_SWAPTMEMBER_INDEX
{
	UINT nAccountDBID;
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct VIMAPVPChangeMemberGrade		//VIMA_PVP_CHANGEMEMBER_GRADE
{
	UINT nAccountDBID;
	USHORT nType;
	bool bAsign;		//true to appoint, false to dismiss
	UINT nTargetSessionID;
};

#if defined(PRE_ADD_PVP_TOURNAMENT) 
struct VIMAPVPSwapTournamentIndex //VIMA_PVP_SWAP_TOURNAMENT_INDEX
{
	UINT nAccountDBID;
	char cSourceIndex;
	char cDestIndex;
};

struct MAVIPVPSwapTournamentIndex // MAVI_PVP_SWAP_TOURNAMENT_INDEX
{
	short nRetCode;
	UINT nAccountDBID;	
	char cSourceIndex;
	char cDestIndex;
};

struct MAVIPVPShuffleTournamentIndex // MAVI_PVP_SHUFFLE_TOURNAMENT_INDEX
{
	UINT uiPvPIndex;
	USHORT unVillageChannelID;
	int nCount;
	UINT uiAccountDBID[PvPCommon::Common::PvPTournamentUserMax];
};
#endif

struct VIMANotifyMail		// VIMA_NOTIFYMAIL
{
	UINT nToAccountDBID;	// Recipient AccountDBID
	INT64 biToCharacterDBID;
	short wTotalMailCount;			// Total mail count
	short wNotReadMailCount;			// Unread mail
	short w7DaysLeftMailCount;			// Expiration warning mail
	bool bNewMail;
};

struct VIMANotifyMarket		// VIMA_NOTIFYMARKET
{
	UINT nSellerAccountDBID;			// AccountDBID
	INT64 biSellerCharacterDBID;
	int nItemID;				// Sold item
	short wCalculationCount;	// Settlement count
};

struct VIMANotifyGift		// VIMA_NOTIFYGIFT
{
	UINT nToAccountDBID;	// Recipient AccountDBID
	INT64 biToCharacterDBID;
	bool bNew;
	int nGiftCount;
};

struct VIMANotice			// VIMA_NOTICE
{
	int nLen;
	WCHAR szMsg[CHATLENMAX];
};

struct VIMAZoneNotice		// VIMA_ZONENOTICE
{
	int nMapIndex;
	int nLen;
	WCHAR szMsg[CHATLENMAX];
};

struct VIMATraceBreakInto	// VIMA_TRACEBREAKINTO
{
	int nType; //eTrace
	UINT	uiAccountDBID;				//Tracer
	WCHAR	wszParam[SCHAR_MAX];
};

struct VIMAReqRecall		// VIMA_REQRECALL
{
	UINT	uiAccountDBID;
	UINT	uiRecallChannelID;
	WCHAR	wszCharacterName[NAMELENMAX];
};

struct VIMABanUser			//VIMA_BANUSER
{
	UINT nAccountDBID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct VIMADestroyPvP		//VIMA_DESTROYPVP
{
	UINT nAccountDBID;
	int nPvPIndex;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct VIMAUpdateChannelShowInfo
{
	USHORT	unChannelID;
	bool	bShow;
	int		nServerID;
	CHAR	cThreadID;
};

struct VIMAPCBangResult			// VIMA_PCBANGRESULT, // Receive PC cafe related result value
{
	UINT nAccountDBID;
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct VIMASyncSystemMail
{
	INT64 biReceiverCharacterDBID;
	int nMailDBID;
	int	iTotalMailCount;
	int	iNotReadMailCount;
	int	i7DaysLeftMailCount;
};

struct MAVISyncSystemMail
{
	UINT	uiAccountDBID;
	int		nMailDBID;
	int		iTotalMailCount;
	int		iNotReadMailCount;
	int		i7DaysLeftMailCount;
};

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

namespace MasterSystem
{
	struct VIMACheckMasterApplication
	{
		INT64	biMasterCharacterDBID;
		INT64	biPupilCharacterDBID;	
	};

	struct VIMASyncSimpleInfo
	{
		INT64							biCharacterDBID;
		MasterSystem::EventType::eCode	EventCode;
	};

	struct VIMASyncJoin
	{
		INT64	biCharacterDBID;
		bool	bIsAddPupil;
	};

	struct VIMASyncLeave
	{
		INT64	biCharacterDBID;
		bool	bIsDelPupil;
	};

	struct VIMASyncGraduate
	{
		INT64	biCharacterDBID;
		WCHAR	wszCharName[NAMELENMAX];
	};

	struct VIMASyncConnect
	{
		bool	bIsConnect;
		WCHAR	wszCharName[NAMELENMAX];
		BYTE	cCharacterDBIDCount;
		INT64	CharacterDBIDList[MasterSystem::Max::MasterCount+MasterSystem::Max::PupilCount];
	};

	struct VIMACheckLeave
	{
		UINT	uiAccountDBID;
		INT64	biDestCharacterDBID;
		bool	bIsMaster;
	};

	struct VIMARecallMaster
	{
		UINT	uiPupilAccountDBID;
		WCHAR	wszPupilCharName[NAMELENMAX];
		WCHAR	wszMasterCharName[NAMELENMAX];
		BYTE	cCharacterDBIDCount;
		INT64	CharacterDBIDList[MasterSystem::Max::MasterCount+MasterSystem::Max::PupilCount];
	};

	struct VIMABreakInto
	{
		int		iRet;
		UINT	uiMasterAccountDBID;
		WCHAR	wszPupilCharName[NAMELENMAX];
		BYTE	cCharacterDBIDCount;
		INT64	CharacterDBIDList[MasterSystem::Max::MasterCount+MasterSystem::Max::PupilCount];
	};
	struct VIMAJoinConfirm
	{
		INT64 biMasterCharacterDBID;		
		BYTE cLevel;
		BYTE cJob;
		WCHAR wszPupilCharName[NAMELENMAX];
	};
	struct VIMAJoinConfirmResult
	{
		int iRet;
		bool bIsAccept;
		INT64 biMasterCharacterDBID;
		WCHAR wszPupilCharName[NAMELENMAX];
	};
}

//farm

struct VIMAFarmList			//VIMA_FARMLIST
{
	BYTE		cCount;
	TFarmItemFromDB	Farms[Farm::Max::FARMCOUNT];
};


struct VIMASaveUserTempData			// VIMA_SAVE_USERTEMPDATA
{
	UINT uiAccountDBID;
	int nDungeonClearCount;
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	GameQuitReward::RewardType::eType eUserGameQuitRewardType;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#if defined(PRE_ADD_REMOTE_QUEST)
	int nAcceptWaitRemoteQuestCount;
	int AcceptWaitRemoteQuestList[MAX_PLAY_QUEST];
#endif
};

struct VIMALoadUserTempData			// VIMA_LOAD_USERTEMPDATA
{
	UINT uiAccountDBID;
};

//VIMA_SETGUILDWAR_SCHEDULE
struct VIMASetGuildWarSchedule
{
	int nResultCode;				// Result
	short wScheduleID;				// Guild War season
	short wWinersWeightRate;		// Winning team weight
	bool bForce;
	bool bFinalProgress;			// Whether finals are in progress
	TGuildWarEventInfo EventInfo[GUILDWAR_STEP_END];	// Period info	
	__time64_t tRewardExpireDate;	// Reward (farm, gesture) expiration period 
};
//VIMA_SETGUILDWAR_FINALSCHEDULE
struct MASetGuildWarFinalSchedule
{	
	TGuildWarEventInfo GuildWarFinalSchedule[GUILDWAR_FINALPART_MAX];
};
//VIMA_SETGUILDWAR_POINT, MAVI_SETGUILDWAR_POINT
struct MASetGuildWarPoint
{
	int nResultCode;				// Result
	int nBlueTeamPoint;				// Blue team
	int nRedTeamPoint;				// Red team
};
//VIMA_ADDGUILDWAR_POINT, MAVI_ADDGUILDWAR_POINT
struct MAAddGuildWarPoint
{	
	char cTeamType;					// 1-Blue, 2-Red
	TGuildUID GuildUID;				// Accumulated guild ID
	int nAddPoint;					// Accumulated points
};

// MAVI_CHANGE_GUILDWAR_STEP, MAGA_CHANGE_GUILDWAR_STEP
struct MAChangeGuildWarStep
{
	short wScheduleID;		// Tier
	char cEventStep;		// Event step: GUILDWAR_STEP_NONE ~ GUILDWAR_STEP_END
	char cEventType;		// GUILDWAR_EVENT_START (start) / GUILDWAR_EVENT_END (end)
	short wWinersWeightRate;	// Winning team weight (used in prelim results)
	bool bCheatFlag;		// Cheat key flag
};

// VIMA_SETGUILDWAR_FINAL
struct VIMASetGuildWarFinalTeam
{
	int nCount;
	TGuildUID	GuidUID[GUILDWAR_FINALS_TEAM_MAX];
	WCHAR wszGuildName[GUILDWAR_FINALS_TEAM_MAX][GUILDNAME_MAX];	// Guild name
	int nTotalPoint[GUILDWAR_FINALS_TEAM_MAX];
};

// VIMA_SETGUILDWAR_POINT_RUNNING_TOTAL, MAVI_SETGUILDWAR_POINT_RUNNING
struct MASetGuildWarPointRunningTotal
{	
	SGuildWarRankingInfo sGuildWarPointRunningTotal[GUILDWAR_TRIAL_POINT_TEAM_MAX];
};

// MAVI_GETGUILDWAR_PREWIN_REWARD
struct MAVIGetGuildWarPrewinReward
{
	short wScheduleID;		// Previous winning round
	UINT nGuildDBID;		// DBID of winning guild		
};

// VIMA_GUILDWAR_REFRESH_GUILD_POINT, MAVI_GUILDWAR_REFRESH_GUILD_POINT
struct MAGuildWarRefreshGuildPoint
{
	TGuildUID GuildUID[GUILDWAR_FINALS_TEAM_MAX]; // GuildUID to update
	int nGuildPoint[GUILDWAR_FINALS_TEAM_MAX]; // GuildPoint to update
};

//MAVI_SETGUILDWAR_SECRETMISSION / MAGA_SETGUILDWAR_SECRETMISSION, // Guild war secret mission
struct MASetGuildWarSecretMission 
{
	BYTE cTeamCode;
	int nRandomSeed;
};

struct MAUpdateGuildWare	//MAVI_UPDATEGUILDWARE	/	MAGI_UPDATEGUILDWARE
{
	TGuildUID GUildID;
};

//VIMA_ADD_DBJOBSYSTEM_RESERVE
struct VIMAAddDBJobSystemReserve
{
	int nResultCode;
	char cJobType;
	int nJobSeq;
};
struct VIMAGetDBJobSystemReserve
{
	int nResultCode;
	int nJobSeq;
	char cJobStatus;
};
//VIMA_MATCHLIST_SAVE_RESULT
struct VIMAMatchListSaveResult
{
	int nResultCode;
};

struct VIMADuplicateLogin
{
	UINT nAccountDBID;
	UINT nSessionID;
	bool bIsDetach;	
};

#if defined( PRE_WORLDCOMBINE_PVP )

struct VIMACreateWorldPvPRoom
{
	int nRoomIndex;						// Since PvP room indices up to PvPCommon::Common::MaxRoomCount are in use, add +1	
	TWorldPvPMissionRoom Data;
	UINT nGMAccountDBID;				// Creator GM ID
};

#endif

#pragma pack(pop)
