#pragma once

#pragma pack(push, 1)

//--------------------------------------------------------------
//	Master Packet
//--------------------------------------------------------------

// Common (Master -> any server...)
struct MARegist				// MAVI_REGISTWORLDINFO, MAGA_REGIST
{
	char cWorldSetID;
	BYTE cGameID;
};

struct MAReqUserList		// MAVI_REQUSERLIST
{
	int nRetCode;
};

struct MAPrivateChat			// MAVI_PRIVATECHAT, MAGA_PRIVATECHAT
{
	UINT nAccountDBID;
	WCHAR wszFromCharacterName[NAMELENMAX];
	char cType;		// Whether GM or not.. (Do we really have to go this far? -_-;)
	short wChatLen;
	int nRet;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct MAChat					// MAVI_CHAT, MAGA_CHAT
{
	UINT nAccountDBID;
	char cType;
	WCHAR wszFromCharacterName[NAMELENMAX];
	short wChatLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct MAZoneChat			//MAVI_ZONECHAT
{
	int nMapIdx;
	WCHAR wszFromCharacterName[NAMELENMAX];
	short wChatLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct MAWorldSystemMsg			// MAVI_WORLDSYSTEMMSG,	MAGA_WORLDSYSTEMMSG
{
	WCHAR wszFromCharacterName[NAMELENMAX];
	char cType;
	int nID;
	int nValue;
	WCHAR wszToCharacterName[NAMELENMAX];
};

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct MAPrivateChannelChat					// MAVI_PRIVATECHNNELCHAT, MAGA_PRIVATECHNNELCHAT
{	
	char cWorldSetID;
	WCHAR wszFromCharacterName[NAMELENMAX];
	short wChatLen;
	INT64 nChannelID;
	WCHAR wszChatMsg[CHATLENMAX];	
};
#endif

struct TNoticeTypeInfo
{
	int nNoticeType;
	int nSlideShowSec;					//If 0, normal notice; otherwise ticker/marquee notice..
	int nManagedID;
	int nChannelID;
	int nMapIdx;
};

struct MANotice				//MAGA_NOTICE
{
	TNoticeTypeInfo Info;
	USHORT nLen;
	WCHAR wszNoticeMsg[CHATLENMAX];
};

struct MAUserState			//MAVI_USERSTATE / MAGA_USERSTATE
{
	WorldUserState::eCode Type;
	INT64	biCharacterDBID;
	BYTE	cLocationState;
	BYTE	cCommunityState;
	int		nChannelID;
	int		nMapIdx;
	BYTE	cNameLen;
	WCHAR	wszBuf[NAMELENMAX];
};

struct MADetachUser			// MAVI_DETACHUSER / MAGA_DETACHUSER
{
	UINT nAccountDBID;
	UINT nSessionID;
	bool bIsDuplicate;	
};

struct MAResetAuthNode
{
public:
	MAResetAuthNode()
	{
		uiAccountDBID = 0;
		nSessionID = 0;
	}
	void Set(UINT pAccountDBID, UINT pSessionID)
	{
		uiAccountDBID = pAccountDBID;
		nSessionID = pSessionID;
	}
public:
	UINT uiAccountDBID;		// Account DBID
	UINT nSessionID;		// Session ID
};

struct MAResetAuthList		// MAVI_RESETAUTHLIST / MAGA_RESETAUTHLIST / VIMA_RESETAUTHLIST / GAMA_RESETAUTHLIST / LOMA_RESETAUTHLIST / MALO_RESETAUTHLIST
{
public:
	MAResetAuthList()
	{
		cWorldSetID = 0;
		nCount = 0;
	}
	int GetSize() const
	{ 
		return(static_cast<int>((sizeof(*this) - (sizeof(this->List[0]) * (_countof(List) - nCount)))&ULONG_MAX));
	}
	bool IsFull() const
	{
		return(RESETAUTHLISTMAX <= nCount);
	}
	bool IsEmpty() const
	{
		return(0 >= nCount);
	}
	bool Add(UINT pAccountDBID, UINT pSessionID)
	{
		if (RESETAUTHLISTMAX <= nCount) {
			return false;
		}

		List[nCount].Set(pAccountDBID, pSessionID);
		++nCount;

		return true;
	}
	const MAResetAuthNode* At(int iIndex) const
	{
		if (!CHECK_LIMIT(iIndex, _countof(List))) {
			return NULL;
		}

		return(&List[iIndex]);
	}
public:
	char cWorldSetID;			// World ID
	int nCount;				// List count
	MAResetAuthNode List[RESETAUTHLISTMAX];		// Account DBID reset request list
};

struct MAVIVoiceChannelID		//MAVI_VOICECHANNELID
{
	TINSTANCEID InstanceID;
	BYTE cType;
	int nPvPLobbyChannelID;
	UINT nVoiceChannelD[PvPCommon::TeamIndex::Max];
};

typedef MAGAFarmSync MAVIFarmSync;

struct MAResetAuthAccWld	// MALO_RESETAUTHLIST
{
public:
	MAResetAuthAccWld()
	{
		cWorldSetID = 0;
		nCount = 0;
	}
	int GetSize() const
	{ 
		return(static_cast<int>((sizeof(*this) - (sizeof(this->List[0]) * (_countof(List) - nCount)))&ULONG_MAX));
	}
	bool IsFull() const
	{
		return(RESETAUTHLISTMAX <= nCount);
	}
	bool IsEmpty() const
	{
		return(0 >= nCount);
	}
	bool Add(UINT pAccountDBID, UINT pSessionID)
	{
		if (RESETAUTHLISTMAX <= nCount) {
			return false;
		}

		List[nCount].Set(pAccountDBID, pSessionID);
		++nCount;

		return true;
	}
	const MAResetAuthNode* At(int iIndex) const
	{
		if (!CHECK_LIMIT(iIndex, _countof(List))) {
			return NULL;
		}

		return(&List[iIndex]);
	}
public:
	char cWorldSetID;			// World ID (0 if LO)
	int nCount;				// List count
	MAResetAuthNode List[RESETAUTHLISTMAX];		// Account DBID reset request list
};

struct MAResetAuthOnly		// MAGA_RESETAUTHONLY / GAMA_RESETAUTHONLY
{
public:
	MAResetAuthOnly()
	{
		cWorldSetID = 0;
		nAccountDBID = 0;
		nSessionID = 0;
	}
public:
	char cWorldSetID;						// World ID
	UINT nAccountDBID;					// Account DBID
	int nSessionID;
};

struct MAFCMState			// MAVI_FCMSTATE / MAGA_FCMSTATE
{
	UINT nAccountDBID;
	int nOnlineMin;	// Online time (min)
	bool bSend;
};

struct MANotifyMail			// MAVI_NOTIFYMAIL / MAGA_NOTIFYMAIL
{
	UINT nToAccountDBID;	// Recipient AccountDBID
	INT64 biToCharacterDBID;
	short wTotalMailCount;
	short wNotReadMailCount;
	short w7DaysLeftCount;
	bool bNewMail;
};

struct MANotifyMarket		// MAVI_NOTIFYMARKET / MAGA_NOTIFYMARKET
{
	UINT nSellerAccountDBID;			// AccountDBID
	INT64 biSellerCharacterDBID;
	int nItemID;				// Sold item
	short wCalculationCount;	// Settlement count
};

struct MANotifyGift			// MAVI_NOTIFYGIFT / MAGA_NOTIFYGIFT
{
	UINT nToAccountDBID;	// Recipient AccountDBID
	INT64 biToCharacterDBID;
	bool bNew;
	int nGiftCount;
};

struct MARestraint		//MAGA_RESTRAINT / MAVI_RESTRAINT
{
	UINT nAccountDBID;
};

// Guild
// VIMA_DISMISSGUILD / MAVI_DISMISSGUILD / MAGA_DISMISSGUILD
struct MADismissGuild
{
	int nManagedID;		// Server management ID
	TGuildUID GuildUID;	// Guild UID
};

struct MAInviteGuildMember
{	
	// Inviter
	TGuildUID	GuildUID;							// Guild ID
	UINT		nAccountDBID;						// Guild inviter DBID
	UINT		nSessionID;							// Session ID
	INT64		nCharacterDBID;						// Character ID
	WCHAR		wszFromCharacterName[NAMELENMAX];	// Character name
	WCHAR		wszGuildName[GUILDNAME_MAX];		// Guild name
	// Invitee
	WCHAR		wszToCharacterName[NAMELENMAX];	
};

struct VIMAInviteGuildMemberResult		//VIMA_RETINVITEGUILDMEMBER
{
	int		nRetCode;					// Result code
	UINT	nInviterDBID;				// User to receive the result
	bool	bAck;						// Request/Result
	WCHAR	wszInvitedName[NAMELENMAX];	// Invitee name
};

struct MAVIGuildMemberInvitedResult
{
	UINT		nAccountDBID;
	int			nRet;
	bool		bAck;						// Request/Result
	WCHAR		wszInvitedName[NAMELENMAX];	// Invitee name
};

struct MAGuildWareInfo
{
	int				nManagedID;		// Server management ID
	TGuildUID		GuildUID;		// Guild ID
};

struct MAGuildWareInfoResult
{
	int				nFromManagedID;				// Server management ID

	TGuildUID		GuildUID;					// Guild ID
	__time64_t		VersionDate;				// Version

	BYTE			cSlotListCount;				// Slot count
	TGuildWareSlot	WareSlotList[GUILD_WAREHOUSE_MAX];	// Slot list
	
};

struct MAGuildMemberLevelUp
{
	int				nManagedID;		// Server management ID
	TGuildUID		GuildUID;		// Guild DBID
	INT64			nCharacterDBID;	// Character DBID
	char			cLevel;			// Character final level
};

// VIMA_ADDGUILDMEMB / MAVI_ADDGUILDMEMB / MAGA_ADDGUILDMEMBER
struct MAAddGuildMember
{
	int nManagedID;						// Server management ID
	TGuildUID GuildUID;					// Guild UID
	UINT nAccountDBID;					// Joined user account DBID
	INT64 nCharacterDBID;					// Joined character DBID
	TP_JOB nJob;							// Character advancement job
	char cLevel;							// Character level
	__time64_t JoinDate;				// Guild join date
	TCommunityLocation Location;			// Current location (login status is also checked via this member's state)
	WCHAR wszCharacterName[NAMELENMAX];	// Joined character name
};

// VIMA_DELGUILDMEMB / MAVI_DELGUILDMEMB / MAGA_DELGUILDMEMBER
struct MADelGuildMember
{
	int nManagedID;			// Server management ID
	TGuildUID GuildUID;		// Guild UID
	UINT nAccountDBID;		// Removed user account DBID
	INT64 nCharacterDBID;		// Removed character DBID
	bool bIsExiled;			// Whether expelled
};

// VIMA_CHANGEGUILDINFO / MAVI_CHANGEGUILDINFO / MAGA_CHANGEGUILDINFO
struct MAChangeGuildInfo
{
	int nManagedID;				// Server management ID
	TGuildUID GuildUID;			// Guild UID
	UINT nAccountDBID;			// Requesting account DBID (usually not critical, used for reference only)
	INT64 nCharacterDBID;			// Requesting character DBID (usually not critical, used for reference only)
	BYTE btGuildUpdate;	// Guild info change type (eGuildUpdateType)
	int Int1;
	int Int2;
	INT64 Int64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
	int Int3;
	int Int4;
};

// VIMA_CHANGEGUILDMEMBINFO / MAVI_CHANGEGUILDMEMBINFO / MAGA_CHANGEGUILDMEMBERINFO
struct MAChangeGuildMemberInfo	
{
	int nManagedID;			// Server management ID
	TGuildUID GuildUID;		// Guild UID
	UINT nReqAccountDBID;	// Requesting account DBID (differs from target when guild master modifies guild member info)
	INT64 nReqCharacterDBID;	// Requesting character DBID (differs from target when guild master modifies guild member info)
	UINT nChgAccountDBID;	// Account DBID to be changed 
	INT64 nChgCharacterDBID;	// Character DBID to be changed
	BYTE btGuildMemberUpdate;		// Guild member info update type (eGuildMemberUpdateType)
	int Int1;
	int Int2;
	INT64 Int64;
	WCHAR Text[GUILDPARAMTEXT_MAX+1];
};

// MAGA_REFRESHGUILDSELFVIEW
struct MARefreshGuildSelfView
{
	TGuildUID GuildUID;		// Guild UID
	UINT nAccountDBID;		// Account DBID
	INT64 nCharacterDBID;		// Character DBID
};

// VIMA_GUILDCHAT / GAMA_GUILDCHAT / MAVI_GUILDCHAT / MAGA_GUILDCHAT
struct MAGuildChat
{
	int nManagedID;				// Server management ID
	TGuildUID GuildUID;			// Guild UID
	UINT nAccountDBID;			// Account DBID
	INT64 nCharacterDBID;			// Character DBID
	short nLen;					// Chat length
	WCHAR wszChatMsg[CHATLENMAX];	// Chat message
};

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
struct MADoorsGuildChat
{
	int nManagedID;				// Server management ID
	TGuildUID GuildUID;			// Guild UID
	INT64 nCharacterDBID;			// Character DBID
	short nLen;					// Chat length
	WCHAR wszChatMsg[CHATLENMAX];	// Chat message
};
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

// MAGA_GUILDMEMBLOGINLIST
struct MAGuildMemberLoginList
{
	TGuildUID GuildUID;
	int nCount;
	INT64 List[GUILDSIZE_MAX];
};

//VIMA_GUILDCHANGENAME , MAVI_GUILDCHANGE_NAME, MAGA_CHANGEGUILDNAME
struct MAGuildChangeName
{
	TGuildUID	GuildUID;
	WCHAR		wszGuildName[GUILDNAME_MAX];  // Changed guild name
};

struct MAGuildChangeMark
{
	TGuildUID	GuildUID;
	short		wMark;
	short		wMarkBG;
	short		wMarkBorder;
};

struct MAGuildLevelUp
{
	TGuildUID	GuildUID;
	int nLevel;
};

struct MAGuildExp
{
	TGuildUID	GuildUID;
	BYTE cPointType;
	int nPointValue;
	int nGuildExp;
	INT64 biCharacterDBID;
	int nMissionID;
};

struct MAEnrollGuildWar
{
	int			nManagedID;
	TGuildUID	GuildUID;
	short		wScheduleID;
	BYTE		cTeamColorCode;
};
//MAVI_GETGUILDWAR_FINALSCHEDULE
struct MAGetGuildWarFinalSchedule
{
	short		wScheduleID;
};
//MAVI_GETGUILDWAR_TOURNAMENTINFO
struct MAGetGuildWarTournamentInfo
{
	short		wScheduleID;
};
//MAVI_GETGUILDWAR_PREWIN_GUILD
struct MAGetGuildWarPreWinGuild
{
	short		wScheduleID;
};
//MAVI_SETGUILDWAR_FINAL_TEAM,
struct MAVISetGuildWarFinalTeam
{
	UINT	GuildDBID[GUILDWAR_FINALS_TEAM_MAX];	
};
//MAVI_SETGUILDWAR_FINALPROCESS, 
struct MASetGuildWarFinalProcess
{
	char cGuildFinalPart;			// Current main tournament round in progress
	__time64_t tBeginTime;			// Start time? (if none, it is in progress)
};
//MAVI_SETGUILDWAR_TOURNAMENTINFO, VIMA_SETGUILDWAR_TOURNAMENTINFO
struct MAGuildWarTournamentInfo
{
	SGuildTournamentInfo		sGuildWarFinalInfo[GUILDWAR_FINALS_TEAM_MAX]; // Sort guilds by bracket order
};
//MAVI_SETGUILDWAR_PREWIN_GUILD, VIMA_SETGUILDWAR_PRE_WIN_GUILD
struct MAGuildWarPreWinGuild
{
	TGuildUID GuildUID;		// Winning guild UID
	bool bPreWin;			
	short	wScheduleID;
};

//MAVI_SETGUILDWAR_PREWIN_SKILLCOOLTIME, VIMA_SETGUILDWAR_PREWIN_SKILLCOOLTIME
struct MAGuildWarPreWinSkillCoolTime
{
	int nManagedID;			// Server management ID
	DWORD dwSkillCoolTime;		// Cooldown
};

//MAVI_SETGUILDWAR_FINAL_RESULT
struct MAVISetGuildWarFinalResult
{
	char	cMatchTypeCode;
	char	cWinGuildIndex;
	char	cLoseGuildIndex;
};
//MAVI_SETGUILDWAR_FINAL_REWARD, MAVI_SETGUILDWAR_FINAL_RESULT_DB
struct MAVISetGuildWarFinalReward
{
	char	cMatchTypeCode;		// What enhancement level?
	UINT	GuildDBID;
	bool	bWin;	
};

struct MAVISetGuildWarFinalDB
{
	short	wScheduleID;
	char	cMatchSequence;	
	char	cMatchTypeCode;		// What enhancement level?
	UINT	GuildDBID;
	bool	bWin;
};

// MAVI_SETGUILDWAR_EVENT_TIME, MAGA_SETGUILDWAR_EVENT_TIME
struct MASetGuildWarEventTime
{
	// Whether finals are in progress
	bool bFinalProgress;		
	// Full guild war schedule
	TGuildWarEventInfo sGuildWarTime[GUILDWAR_STEP_END]; 
	// Finals round number
	TGuildWarEventInfo sFinalPartTime[GUILDWAR_FINALPART_MAX];
};

//MAVI_SETGUILDWAR_TOURNAMENTWIN, MAGA_SETGUILDWAR_TOURNAMENTWIN
struct MASetGuildWarTournamentWin
{	
	// Win round count
	char cMatchTypeCode;					// Current round in progress: 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	// Winning guild name
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
};

// VIMA_UPDATEGUILDEXP / MAVI_UPDATEGUILDEXP / MAGA_UPDATEGUILDEXP / GAMA_UPDATEGUILDEXP
struct MAUpdateGuildExp
{
	int nManagedID;				// Server management ID
	TGuildUID GuildUID;			// Guild UID
	char cPointType;			// Point type
	int nPointValue;			// Point value
	int nTotalGuildExp;			// Guild EXP
	int nDailyPointValue;		// Daily point value
	int nLevel;					// Level up
	INT64 biCharacterDBID;		// Character ID
	int nMissionID;				// Mission ID
};

struct MAGuildRecruitMember
{
	int			nManagedID;				// Server management ID
	BYTE		cAcceptType;
	UINT		uiAccountDBID;
	WCHAR		wszCharacterName[NAMELENMAX];
	INT64		biCharacterDBID;
	TGuildUID	GuildUID;
	TGuildView	GuildView;
};
struct MAGuildRewardItem
{
	int			nManagedID;				// Server management ID
	TGuildUID	GuildUID;
	int			nItemID;	
	BYTE		cItemBuyType;
	__time64_t	m_tExpireDate;
};
struct MAExtendGuildSize
{
	int			nManagedID;				// Server management ID
	TGuildUID	GuildUID;
	short		nGuildSize;	
};

//MAVI_ADD_DBJOBSYSTEM_RESERVE
struct MAVIAddDBJobSystemReserve
{
	char cJobType;
};
struct MAVIGetDBJobSystemReserve
{
	int nJobSeq;
};

struct MAChangeCharacterName
{
	int			nManagedID;
	TGuildUID	GuildUID;
	char		cWorldSetID;
	UINT		uiAccountDBID;
	INT64		nCharacterDBID;
	WCHAR		wszOriginName[NAMELENMAX];
	WCHAR		wszCharacterName[NAMELENMAX];
#if defined (PRE_ADD_BESTFRIEND)
	UINT		uiBFAccountDBID;
	INT64		biBFCharacterDBID;
#endif
};

struct MAUpdateWorldEventCounter
{
	char		cWorldSetID;
	int			nScheduleID;
	int			nCount;
};

struct MAPCBangResult
{
	UINT nAccountDBID;

	char cPCBangGrade;

};

struct MAGAZeroPopulation		//MAGA_ZEROPOPULATION
{
	bool bZeroPopulation; //true: remove user, false: user may enter
};

// Master -> Login
struct MALORegist				//MALO_REGIST
{
	char cWorldSetID;
	UINT nWorldMaxUser;
};

struct MALOVillageInfo			//MALO_VILLAGEINFO
{
	int nVillageID;
	UINT nWorldUserCount;
	UINT nWorldMaxUserCount;
	BYTE cCount;
	sChannelInfo Info[VILLAGECHANNELMAX];
};

struct MALOVillageInfoDelete				//MALO_VILLAGEINFODEL		
{
	int nVillageID;
};

struct MALOAddUser			//MALO_ADDUSER
{
	UINT nAccountDBID;
	UINT nSessionID;
	char cWorldID;
	int nRet;
};

#if !defined(PRE_MOD_SELECT_CHAR)
struct MALOUserInfo			// MALO_USERINFO
{
	UINT nAccountDBID;
	UINT nSessionID;
	int nRet;
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	char szIP[IPLENMAX];
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
};
#endif	// #if !defined(PRE_MOD_SELECT_CHAR)

struct MALODetachUser		//MALO_DETACHUSER
{
	UINT nAccountDBID;
};

struct MALOSetTutorialGameID	// MALO_SETTUTORIALGAMEID
{
	UINT nSessionID;
	ULONG nIP;
	USHORT nPort;
	USHORT nTcpPort;
	BYTE cServerIdx;
	USHORT wGameID;
	int nRoomID;
	int nRet;
};

struct MALOResetAuthServer		// MALO_RESETAUTHSERVER
{
	int nServerID;
};

struct MALOWaitUserProcess		// MALO_WAITUSERPROCESS
{
	USHORT nCurCount;
	short nCount;
	UINT nAccountArr[WAITPROCESSMAX];
};

struct MALOAddWaitUser			// MALO_ADDWAITUSER
{
	UINT nAccountDBID;
	USHORT nTicketNum;
	short nRetCode;
};

struct MALOUpdateChannelShowInfo
{
	USHORT	unChannelID;
	bool	bShow;
};

struct TUpdateTicket
{
	short nIndex;
	short nCount;
};

struct MALODuplicateLogin
{
	UINT nAccountDBID;
	UINT nSessionID;
};

// Master -> Village
//struct MAVIVillageServerInfo		//MAVI_VILLAGESERVERINFO
//{
//	USHORT nInfoCount;
//	TChannelInfo ChannelList[VILLAGECHANNELMAX];
//};

#define MAVIVillageInfo MALOVillageInfo		//MAVI_VILLAGEINFO

#define MAVIVillageInfoDelete MALOVillageInfoDelete //MAVI_VILLAGEINFODEL

struct MAVILoginState		// MAVI_LOGINSTATE
{
	UINT nSessionID;
	int nRet;
};

struct MAVICheckUser			// MAVI_CHECKUSER
{
	UINT	nSessionID;
	UINT	nAccountDBID;
	WCHAR	wszAccountName[IDLENMAX];
	INT64	biCharacterDBID;
#if !defined( PRE_PARTY_DB )
	TPARTYID	PartyID;
#endif
	USHORT	nChannelID;
	BYTE	cPvPVillageID;
	USHORT	unPvPVillageChannelID;
	bool	bAdult;			// Whether adult or not (fatigue)
	char	cPCBangGrade;	// pcbang
	short	nRet;
#ifdef PRE_ADD_COMEBACK
	bool bComebackUser;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	bool bReConnectNewbieReward;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
};

struct MAVITargetVillageInfo		//MAVI_TARGETVILLAGEINFO
{
	UINT nAccountDBID;
	BYTE cVillageID;
	int nTargetChannelID;				//maybe
	int nTargetMapIdx;
	char cTargetGateNo;
	char szIP[IPLENMAX];
	USHORT nPort;
	int nRet;
	BYTE cPartyInto;
	INT64 nItemSerial;
};

struct MAVISetGameID			// MAVI_SETGAMEID
{
	GameTaskType::eType GameTaskType;
	BYTE	cReqGameIDType;
	TINSTANCEID InstanceID;
	ULONG	nIP;
	USHORT	nPort;
	USHORT	nTcpPort;
	BYTE	cServerIdx;
	USHORT	wGameID;
	int		iRoomID;
	USHORT	unVillageChannelID;
	BYTE cVillageID;
	short	nRet;
	BYTE cUseVoice;
#if defined( PRE_WORLDCOMBINE_PARTY )
	ePartyType Type;
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
	WorldPvPMissionRoom::Common::eReqType eWorldReqType;
#endif
};

struct MAVILadderSetGameID
{
	ULONG	nIP;
	USHORT	nPort;
	USHORT	nTcpPort;
	BYTE	cServerIdx;
	USHORT	wGameID;
	int		iRoomID;
	USHORT	unVillageChannelID;
	BYTE	cVillageID;
	short	nRet;
	int		nGameModeTableID;
	BYTE	cUserCount;
	UINT	uiAccountDBIDArr[PARTYMAX];
};

struct MAVIPushParty				// MAVI_PUSHPARTY
{
	UINT nLeaderAccountDBID;	
#if defined( PRE_PARTY_DB )
	Party::Data PartyData;	
#else
	TPARTYID PartyID;
	int nChannelID;
	BYTE cMemberMax;
	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemLootRank;	

	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;
#endif
	int nRandomSeed;
	BYTE cPushType;
	UINT nKickedMemberList[PARTYKICKMAX];
	WCHAR wszPartyName[PARTYNAMELENMAX];
#if defined( PRE_PARTY_DB )
#else
	USHORT nUserLvLimitMin;
	USHORT nUserLvLimitMax;
	BYTE cIsPartyJobDice;
	WCHAR wszPartyPass[PARTYPASSWORDMAX];
	BYTE cUpkeepCount;
	BYTE cMemberCount;
#endif // #if defined( PRE_PARTY_DB )
	UINT nVoiceChannelID;	
	TMemberInfo MemberInfo[PARTYMAX];
};

struct MAVIInvitePartyMember : public TInvitePartyInfo		//MAVI_INVITEPARTYMEMBER
{
	int nGameServerID;				//Just checking which game server sent the invitation
	int nVillageMapIdx;				//Verified once on master, but to re-check where the party went

	UINT nInviterAccountDBID;
	INT64 biInviterCharacterDBID;
	WCHAR wszInviterName[NAMELENMAX];	//Inviter's name
	UINT nInvitedAccountDBID;			//Invitee's DB ID	
	WCHAR wszInvitedName[NAMELENMAX];	//Invitee's name
	BYTE cPermitLevel;
	char cPassClassIds[PERMITMAPPASSCLASSMAX];
};

#if defined( PRE_PARTY_DB )

struct MAVIResPartyInvite
{
	int iRet;
	UINT uiReqAccountDBID;
};

#else
struct MAVIReqPartyID			// MAVI_REQPARTYID
{
	UINT nAccountDBID;
	UINT nSessionID;
	TPARTYID PartyID;
	BYTE cMemberMax;
	int nChannelID;	
	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemRank;
	BYTE cUserLvLimitMin;
	BYTE cUserLvLimitMax;

	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;

	BYTE cUseVoice;
	UINT nAfterInviteAccountDBID;

	WCHAR wszPartyName[PARTYNAMELENMAX];
	BYTE cPartyJobDice;
	WCHAR wszPartyPass[PARTYPASSWORDMAX];
};
#endif // #if defined( PRE_PARTY_DB )

struct MAVIDelParty			// MAVI_DELPARTY
{
	UINT nAccountDBID;
	int nPartyIndex;
	bool bSend;
};

struct MAVIAddPartyMember		// MAVI_ADDPARTYMEMBER
{
	UINT nAccountDBID;
	UINT nSessionID;
	int nPartyIndex;
	int nMemberIndex;
};

struct MAVIDelPartyMember		// MAVI_DELPARTYMEMBER
{
	UINT nAccountDBID;
	int nPartyIndex;
	int nMemberIndex;
	UINT nNewLeaderAccountDBID;
	UINT nNewLeaderSeessionID;
	bool bSend;
	char cKickKind;
};

struct MAVISwapPartyLeader		//MAVI_SWAPPARTYLEADER
{
	int nPartyIdx;
	int nNewLeaderMemberIdx;
	UINT nAccountDBID;
	UINT nSessionID;
};

struct MAVIReservePartyMember		//MAVI_RESERVEPARTYMEMBER
{
	UINT nAccountDBID;
	int nPartyIdx;
	int nChannelID;
	int nRet;
};

struct MAVIDelVoiceChannel	 //MAVI_DELVOICECHANNEL
{
	int nPartyIdx;
};

struct MAVIResult				// MAVI_ADDPARTYFAIL, MAVI_DELPARTYFAIL, MAVI_ADDPARTYMEMBERFAIL, MAVI_DELPARTYMEMBERFAIL
{
	UINT nAccountDBID;
	int nMainCmd;
	int nRet;
};

struct MAVIFriendAddNotice			//MAVI_FRIENDADDNOTICE
{
	UINT nAddedAccountDBID;
	WCHAR wszAddName[NAMELENMAX];
};

// PvP
struct MAVIPVP_CREATEROOM
{
	int					nRetCode;
	UINT				uiCreateAccountDBID;
	UINT				uiPvPIndex;
	USHORT				unVillageChannelID;
	int					nEventID;
	TEvent				EventData;
	BYTE				cGameMode;
	UINT nGuildWarDBID[PvPCommon::TeamIndex::Max];		//Referenced only when cGameMode is GuildWar
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE				cGambleType;
	int					nPrice;
#endif
	CSPVP_CREATEROOM	sCSPVP_CREATEROOM;
#if defined( PRE_WORLDCOMBINE_PVP )
	BYTE cWorldPvPRoomCurMemberCount;
#endif
};

struct MAVIPVP_MODIFYROOM
{
	int					nRetCode;
	UINT				uiAccountDBID;
	UINT				uiPvPIndex;
	USHORT				unVillageChannelID;
	BYTE				cGameMode;
	CSPVP_MODIFYROOM	sCSPVP_MODIFYROOM;
};

struct MAVIPVP_DESTROYROOM
{
	USHORT				unVillageChannelID;
	UINT				uiPvPIndex;
};

struct MAVIPVP_LEAVEROOM
{
	short								nRetCode;
	USHORT								unVillageChannelID;
	UINT								uiPvPIndex;
	UINT								uiLeaveAccountDBID;
	PvPCommon::LeaveType::eLeaveType	eType;
};

struct MAVIPVP_ROOMLIST
{
	UINT			uiAccountDBID;
	USHORT			unVillageChannelID;
	CSPVP_ROOMLIST	sCSPVP_ROOMLIST;
};

#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
struct MAVIPVP_ROOMLIST_RELAY
{
	BYTE			 cVillageID;
	UINT			 uiAccountDBID;
	MAVIPVP_ROOMLIST sRoomList;	
};
#endif //#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
struct MAVIPVP_CHANGECHANNEL
{
	UINT nAccountDBID;
	BYTE cType;
	int nRetCode;
};
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

struct MAVIPVP_WAITUSERLIST
{
	UINT				uiAccountDBID;
	USHORT				unVillageChannelID;
	CSPVP_WAITUSERLIST	sCSPVP_WAITUSERLIST;
};

struct MAVIPVP_JOINROOM
{
	short	nRetCode;
	UINT	uiAccountDBID;
	USHORT	unVillageChannelID;
	UINT32	uiPvPIndex;
	UINT16	unPvPTeam;
	UINT	uiUserState;
	BYTE cIndex;
};


struct MAVIPVP_ROOMSTATE
{
	USHORT	unVillageChannelID;
	UINT	uiPvPIndex;
	UINT	uiRoomState;
};

struct MAVIPVP_CHANGETEAM
{
	int					nRetCode;
	USHORT				unVillageChannelID;
	UINT32				uiPvPIndex;
	VIMAPVP_CHANGETEAM	sVIMAPVP_CHANGETEAM;
	char cTeamSlotIndex;
};

struct MAVIPVP_CHANGEUSERSTATE
{
	USHORT	unVillageChannelID;
	UINT	uiPvPIndex;
	UINT	uiAccountDBID;
	UINT	uiUserState;
};

struct MAVIPVP_START
{
	short	nRetCode;
	USHORT	unVillageChannelID;
	UINT	uiPvPIndex;
	UINT	uiAccountDBID;
};

struct MAVIPVP_STARTMSG
{
	USHORT	unVillageChannelID;
	UINT	uiPvPIndex;
	BYTE	cSec;
};

struct MAVIPVP_ROOMINFO
{
	UINT	uiAccountDBID;
	UINT	uiIndex;
	UINT	uiWinCondition;
	UINT	uiGameModeTableID;
	UINT	uiPlayTimeSec;
	BYTE	cMaxUser;
	bool bIsGuildWar;		//If this is true, only exit is enabled and everything else is blocked
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE cGambleType;
	int nGamblePrice;
#endif
};

struct MAVIPvPMemberIndex		//MAVI_PVP_MEMBERINDEX
{
	UINT nAccountDBID;
	USHORT nTeam;
	int nRetCode;
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct MAVIPvPMemberGrade		//MAVI_PVP_MEMBERGRADE
{
	UINT nAccountDBID;
	USHORT nTeam;
	UINT uiUserState;
	UINT nChangedSessionID;
	int nRetCode;
};

struct MAVIResRecall		// MAVI_RESRECALL
{
	UINT	uiAccountDBID;			// Requester AccountDBID
	UINT	uiRecallAccountDBID;	// AccountDBID of user to be Recalled
	UINT	uiRecallChannelID;		// Target ChannelID for Recall
};

struct MAVIVillageTrace		// MAVI_VILLAGETRACE
{
	UINT	uiAccountDBID;
	UINT	uiTargetAccountDBID;
	int		iTargetVillageID;
	int		iTargetChannelID;
	bool	bIsGMCall;
};

struct MAVIUpdateChannelShowInfo
{
	USHORT	unChannelID;
	bool	bShow;
};

struct MAVIUserTempDataResult	//MAVI_USERTEMPDATA_RESULT
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

// Master -> Game
struct MAGAReqMemberInfo
{
	UINT MemberAccountDBID;
	UINT MemberSessionID;
	INT64 biMemberCharacterDBID;
	UINT MemberTeam;
	WCHAR wszAccountName[IDLENMAX];
	bool bAdult;			// Adult or not (fatigue)	
	char cPCBangGrade;		// PCBang
	BYTE cMemberIndex;
	bool bPvPFatigue;
	UINT uiPvPUserState;
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComebackAppellation;
#endif
	TMemberVoiceInfo VoiceInfo;
};

// Because this packet is shared by GameTaskType and ReqGameIDType
// Not very efficient.. will need to clean this up someday.
struct MAGAReqRoomID			// MAGA_REQROOMID
{
	GameTaskType::eType GameTaskType;
	BYTE cReqGameIDType;
	UINT nLeaderAccountDBID;
	union
	{
		LadderSystem::MatchType::eCode MatchType;
		TINSTANCEID InstanceID;
	};
	int nRandomSeed;
#if defined( PRE_PARTY_DB )
	Party::Data PartyData;
#else
	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemLootRank;	
	BYTE cUserLvLimitMin;
	TDUNGEONDIFFICULTY PartyDifficulty;
	BYTE cUpkeepCount;
	BYTE cMemberMax;	
#endif	
	BYTE cMemberCount;
	int	nMapIndex;
	BYTE cGateNo;
	BYTE cGateSelect;
	TDUNGEONDIFFICULTY StageDifficulty;
	char cWorldSetID;			//They must be transferring from the same world!
	BYTE cVillageID;		//This should also come from the same village! (Please expand if that might not be the case)
	USHORT nMeritBonusID;			//Can receive a specific bonus when entering a game from a specific channel 	
#if defined( PRE_PARTY_DB )
#else
	BYTE cUserLvLimitMax;
#endif // #if defined( PRE_PARTY_DB )
#if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )
	union
	{
#if defined( PRE_ADD_FARM_DOWNSCALE )
		int iFarmAttr;
#else
		Farm::Attr::eType FarmAttr;
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
		int nTargetMapIdx;
	};
#else
	int nTargetMapIdx;
#endif // #if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )
	
	int nChannelID;
	UINT nKickedMemberList[PARTYKICKMAX];

	UINT nGuildDBID[PvPCommon::Common::DefaultGuildCount];
	int nGuildQualifyingScore[PvPCommon::Common::DefaultGuildCount];

	bool bStart;

	int nEventRoomIndex;

	bool bDirectConnect;

	INT64 biLadderRoomIndexArr[2];
	WCHAR wszPartyName[PARTYNAMELENMAX];

#if defined( PRE_PARTY_DB )	
#else
	WCHAR wszPartyPass[PARTYPASSWORDMAX];
	BYTE cIsJobDice;
#endif // #if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PVP )
	WorldPvPMissionRoom::Common::eReqType eWorldReqType;
	TWorldPvPMissionRoom MissonRoomData;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE cGambleType;
	int nGamblePrice;
#endif
	INT64 biCreateRoomCharacterDBID;
	UINT nVoiceChannelID[PvPCommon::TeamIndex::Max];					//Voice channel ID to create on game server (currently PvP teams have the most, so max is team max)	
	MAGAReqMemberInfo arrMemberInfo[PvPCommon::Common::MaxPlayer];	// Since PvP rooms currently have the highest user count per room, set to the PvP room max.
};

struct MAGAInvitePartyMemberResult		//MAGA_INVITEPARTYMEMBERRESULT
{
	UINT nAccountDBID;
	WCHAR wszInvitedName[NAMELENMAX];
	int nRetCode;
};

struct MAGAForceStopPvP		//MAGA_PVP_FORCESTOP
{
	UINT nGameRoomID;
	UINT nForceWinGuildDBID;
};

struct MAGAReqTutorialRoomID	// MAGA_REQTUTORIALROOMID
{
	UINT	nAccountDBID;
	UINT	nSessionID;
	INT64	biCharacterDBID;
	int		nRandomSeed;
	int		nTutorialMapIndex;
	BYTE	cTutorialGateNo;
	char	cWorldSetID;
	bool	bAdult;			// Adult or not (fatigue)
	WCHAR	wszAccountName[IDLENMAX];
	int		nLoginServerID;
};

struct MAGALoginState // MAGA_LOGINSTATE
{
	UINT nAccountDBID;
	int nRet;
};

struct MAGAVillageState	// MAGA_VILLAGESTATE
{
	UINT nAccountDBID;
	int nRoomID;
	int nMapIndex;
	int nRet;
};

struct MAGAFriendAddNotice		//MAGA_FRIENDADDNOTICE
{
	UINT nAddedAccountDBID;
	WCHAR wszAddName[NAMELENMAX];
};

struct MAGAPVP_GAMEMODE
{
	int		iGameRoomID;
	UINT	uiPvPIndex;
	UINT	uiPvPGameModeTableID;
	UINT	uiWinCondition;
	UINT	uiPlayTimeSec;
	USHORT	unGameModeCheck;
	INT64	biSNMain;
	int		iSNSub;
	BYTE	cIsPWRoom;
	USHORT	unRoomOptionBit;
	BYTE	cMaxUser;
	int		nEventID;
	bool	bIsLadderRoom;
	bool bIsGuildWar;		//Whether created by system
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	char cPvPChannelType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined( PRE_WORLDCOMBINE_PVP )
	int nStartPlayer;
#endif
};
#if defined(PRE_ADD_PVP_TOURNAMENT)
struct STournamentUserInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	BYTE	cJob;
	UINT	uiSessionID;
	WCHAR	wszCharName[NAMELENMAX];
};
struct STournamentMatchInfo
{
	bool bStartMatch;			// Whether the game is in progress.
	char cTournamentStep;		// Tournament step
	bool bWin;					// Whether won or lost.		
	bool bAbuse;				// Abuse flag
	bool bLeave;				// Whether exited
	STournamentUserInfo sTournamentUserInfo;	
};
struct MAGAPVP_TOURNAMENT_INFO
{
	int nGameRoomID;
	int nCount;
	STournamentUserInfo sTournamentUserInfo[PvPCommon::Common::PvPTournamentUserMax];
};
#endif

struct MAGABreakIntoRoom
{
	int		iGameRoomID;
	//
	char	cWorldSetID;
	BYTE	cVillageID;
	UINT	uiAccountDBID;
	UINT	uiSessionID;
	INT64	biCharacterDBID;
	UINT	uiTeam;
	bool	bAdult;			// Adult or not (fatigue)
	char	cPCBangGrade;		// PCBang
	BreakInto::Type::eCode		BreakIntoType;
	bool	bPvPFatigue;	// Gain EXP by consuming fatigue in PvP
	char cPvPTeamIndex;
	UINT uiPvPUserState;
	WCHAR	wszAccountName[IDLENMAX];
	WCHAR wszCharacterName[NAMELENMAX];
};

#define MAGAPvPMemberIndex MAVIPvPMemberIndex		//GAVI_PVP_MEMBERINDEX
#define MAGAPvPMemberGrade MAVIPvPMemberGrade		//MAGA_PVP_MEMBERGRADE

struct MAGAPVPChangeUserState		//MAGA_PVP_MEMBERUSERSTATE
{
	UINT nPvPRoomID;
	UINT nAccountDBID;
	UINT uiUserState;
};

struct MAGADelPartyMember		// MAGA_DELPARTYMEMBER
{
	UINT nNewLeaderAccountDBID;	// New leader ID
	int nRoomID;				// Room number
	UINT nMemberSessionID;		// Member ID to be deleted
	UINT nNewLeaderSessionID;
	//bool bIsUnintendMember;
	char cKickKind;
};

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
struct MAGAGetPartyID
{	
	UINT nAccountDBID;
	UINT nSenderAccountDBID;	//Requester of the info
};

struct GAMAGetPartyIDResult
{
	UINT nSenderAccountDBID;	//Requester of the info
	TPARTYID PartyID;
};

struct MAGAReqPartyAskJoin
{
	UINT nAccountDBID;
	UINT nReqAccountDBID;
	INT64 biReqCharacterDBID;
	char cReqUserJob;
	BYTE cReqUserLevel;
	WCHAR wszReqChracterName[NAMELENMAX];	
};

struct GAMAResPartyAskJoin
{
	int iRet;
	UINT uiReqAccountDBID;
	WCHAR wszReqChracterName[NAMELENMAX];
};

struct MAGAResPartyAskJoin
{
	int iRet;
	UINT uiReqAccountDBID;	
};

struct GAMAAskJoinAgreeInfo
{
	TPARTYID PartyID;
	int iPassword;
	WCHAR wszAskerCharName[NAMELENMAX];	
};
#endif

struct MAGANextVillageInfo		// MAGA_NEXTVILLAGEINFO
{
	UINT nAccountDBID;
	int nChannelID;				//maybe
	int nMapIdx;
	int nNextMapIdx;
	int nNextGateIdx;
	char szIP[IPLENMAX];
	USHORT nPort;
	int nRet;
	INT64 nItemSerial;
};

struct MAGARebirthVillageInfo	// MAGA_REBIRTHVILLAGEINFO
{
	UINT nAccountDBID;
	int nVillageID;
	int nChannelID;				//maybe
	char szIP[IPLENMAX];
	USHORT nPort;
	int nRet;
};

struct MAGAMovePvPGameToPvPLobby	// MAGA_MOVEPVPGAMETOPVPLOBBY
{
	UINT	uiAccountDBID;
	char	szIP[IPLENMAX];
	USHORT	unPort;
	short	nRet;
};

struct MAGAResRecall		// MAGA_RESRECALL
{
	UINT	uiAccountDBID;			// Requester AccountDBID
	UINT	uiRecallAccountDBID;	// AccountDBID of user to be Recalled
	UINT	uiRecallChannelID;		// Target ChannelID for Recall
	int		iRecallMapIndex;
};

namespace LadderSystem
{
	struct MAVIDelUser
	{
		WCHAR	wszCharName[NAMELENMAX];
		INT64	biCharDBID;
	};

	struct MAVIRoomSync
	{
		INT64	biRoomIndexArr[2];
		LadderSystem::RoomState::eCode State;
	};
};

namespace MasterSystem
{
	struct MAVICheckMasterApplication
	{
		short	nRet;
		UINT	uiPupilAccountDBID;
		INT64	biMasterCharacterDBID;
	};

	struct MAVISyncSimpleInfo
	{
		UINT							uiAccountDBID;
		INT64							biCharacterDBID;
		MasterSystem::EventType::eCode	EventCode;
	};

	typedef MAVISyncSimpleInfo MAGASyncSimpleInfo;

	struct MAVISyncJoin
	{
		UINT	uiAccountDBID;
		INT64	biCharacterDBID;
		bool	bIsAddPupil;
	};

	typedef MAVISyncJoin MAGASyncJoin;

	struct MAVISyncLeave
	{
		UINT	uiAccountDBID;
		INT64	biCharacterDBID;
		bool	bIsDelPupil;
	};

	typedef MAVISyncLeave MAGASyncLeave;

	struct MAVISyncGraduate
	{
		UINT	uiAccountDBID;
		WCHAR	wszCharName[NAMELENMAX];
	};

	typedef MAVISyncGraduate MAGASyncGraduate;

	struct MAVISyncConnect
	{
		UINT	uiAccountDBID;
		bool	bIsConnect;
		WCHAR	wszCharName[NAMELENMAX];
	};

	typedef MAVISyncConnect	 MAGASyncConnect;

	struct MAVICheckLeave
	{
		int		iRet;
		UINT	uiAccountDBID;
		INT64	biDestCharacterDBID;
		bool	bIsMaster;
	};

	struct MAVIRecallMaster
	{
		int		iRet;
		UINT	uiAccountDBID;
		WCHAR	wszCharName[NAMELENMAX];
		bool	bIsConfirm;
	};
	struct MAVIJoinConfirm
	{		
		UINT uiMasterAccountDBID;
		BYTE cLevel;
		BYTE cJob;
		WCHAR wszPupilCharName[NAMELENMAX];
	};
	struct MAVIJoinResult
	{
		int iRet;
		bool bIsAccept;
		INT64 biMasterCharacterDBID;
		WCHAR wszPupilCharName[NAMELENMAX];
	};
}

// VIMA_REFRESH_GUILDITEM / MAVI_REFRESH_GUILDITEM
struct MARefreshGuildItem
{
	int			nManagedID;
	TGuildUID	GuildUID;

	char cMoveType;

	INT64	biSrcSerial;
	INT64   biDestSerial;

	TItemInfo SrcInfo;
	TItemInfo DestInfo;

	__time64_t VersionDate;
};

// VIMA_REFRESH_GUILDCOIN / MAVI_REFRESH_GUILDCOIN
struct MARefreshGuildCoin
{
	int			nManagedID;
	TGuildUID	GuildUID;

	INT64		biTotalCoin;
};

//VIMA_EXTEND_GUILDWARE,  MAVI_EXTEND_GUILDWARE
struct MAExtendGuildWare
{
	int			nManagedID;
	TGuildUID	GuildUID;

	short		wWareSize;
};

struct TFarmInfo
{
	UINT nFarmDBID;
	int nFarmCurUserCount;
	bool bActivate;
};

struct MAVIFarmInfoUpdate	
{
	BYTE cFarmCount;
	TFarmInfo FarmInfo[Farm::Max::FARMCOUNT];
};

struct MAGAAssginPeriodQuest	// MAGA_ASSIGN_PERIODQUEST
{
	UINT nQuestID;
	bool bFlag;
};

struct MAGANoticePeriodQuest	// MAGA_NOTICE_PERIODQUEST
{
	char cWorldSetID;
	int nItemID;
	int nNoticeCount;
};

struct MAGAUserTempDataResult	//MAGA_USERTEMPDATA_RESULT
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

struct VIMACheckLastDungeonInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	int		iManagedID;
	int		iRoomID;
};

struct MAVICheckLastDungeonInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	bool	bIsCheck;
	WCHAR	wszPartyName[PARTYNAMELENMAX];
};

struct MAGACheckLastDungeonInfo
{
	char	cWorldSetID;
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	int		iRoomID;
};

struct GAMACheckLastDungeonInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	bool	bIsCheck;
	WCHAR	wszPartyName[PARTYNAMELENMAX];
};

struct VIMAConfirmLastDungeonInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	int		iManagedID;
	int		iRoomID;
	bool	bIsOK;
#if defined( PRE_PARTY_DB )
	BreakInto::Type::eCode BreakIntoType;
#endif // #if defined( PRE_PARTY_DB )
};

struct MAVIConfirmLastDungeonInfo
{
	int		iRet;
	UINT	uiAccountDBID;
#if defined( PRE_PARTY_DB )
	BreakInto::Type::eCode BreakIntoType;
#endif // #if defined( PRE_PARTY_DB )
};

struct GAMAConfirmLastDungeonInfo
{
	int		iRet;
	UINT	uiAccountDBID;
#if defined( PRE_PARTY_DB )
	BreakInto::Type::eCode BreakIntoType;
#endif // #if defined( PRE_PARTY_DB )
};

struct MAGADeleteBackupDungeonInfo
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	int		iRoomID;
};

#if defined(PRE_ADD_QUICK_PVP)
//VIMA_MAKEQUICKPVP_ROOM
struct VIMAMakeQuickPvPRoom
{
	UINT	uiMasterAccountDBID;
	UINT	uiSlaveAccountDBID;
};
//MAVI_MAKEQUICKPVP_ROOM
struct MAVIMakeQuickPvPRoom
{
	int		nRet;
	UINT	uiMasterAccountDBID;
	UINT	uiSlaveAccountDBID;
};
#endif

struct DBGetGuildRewardItem
{
	TGuildUID GuildUID;				// Guild UID
};

#if defined (PRE_ADD_BESTFRIEND)
struct MASearchBestFriend
{
	int nRet;
	bool bAck;
	UINT nAccountDBID;
	WCHAR wszName[NAMELENMAX];

	char cJob;
	char cLevel;
};

struct MARegistBestFriend
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	WCHAR wszFromName[NAMELENMAX];

	UINT nToAccountDBID;
	WCHAR wszToName[NAMELENMAX];

	int nRegistItemID;
};

struct MARegistBestFriendResult
{
	UINT nAccountDBID;
	int	 nRet;
	bool bAck;
	INT64 biFromCharacterDBID;
	UINT nToAccountDBID;
	INT64 biToCharacterDBID;
	WCHAR wszFromName[NAMELENMAX];
	WCHAR wszToName[NAMELENMAX];
};

struct MACompleteBestFriend
{
	UINT nFromAccountDBID;
	
	// Applicant
	UINT nToAccountDBID;
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];

	int nItemID;
};

struct MAEditBestFriendMemo
{
	UINT nFromAccountDBID;
	UINT nToAccountDBID;
	INT64 biToCharacterDBID;
	WCHAR wszMemo[BESTFRIENDMEMOMAX];
};

struct MACancelBestFriend
{
	bool bCancel;
	UINT nAccountDBID;			// Target's account code (used on game server only)
	WCHAR wszFromName[NAMELENMAX];
	WCHAR wszToName[NAMELENMAX];
};

struct MACloseBestFriend
{
	UINT nAccountDBID;			// Target's account code (used on game server only)
	WCHAR wszFromName[NAMELENMAX];
	WCHAR wszToName[NAMELENMAX];
};

struct MALevelUpBestFriend
{
	UINT nAccountDBID;			// Target's account code (used on game server only)	
	BYTE cLevel;
	WCHAR wszName[NAMELENMAX];
};
#endif


#if defined( PRE_WORLDCOMBINE_PARTY )
struct MADelWorldParty
{
	int nPrimaryIndex;	// Deleted world party room key
};

struct MAGetWorldPartyMember
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	int iServerID;
	TPARTYID PartyID;
	int iRoomID;	
};

struct GAMASendWorldPartyMember
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	int nRetCode;
	TPARTYID PartyID;
	int nCount;
	Party::MemberData MemberData[PARTYCOUNTMAX];
};

#if defined( PRE_FIX_WORLDCOMBINEPARTY )
struct GAMAWorldCombineGameserver
{
	bool bFlag;
};
#endif
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct MAAddPrivateChannel
{
	char cWorldSetID;
	int nManagedID;				// Server management ID
	TPrivateChatChannelInfo tPrivateChatChannel;
};

struct MAModPrivateChannel
{
	char cWorldSetID;
	int nManagedID;				// Server management ID
	INT64 nPrivateChatChannelID;
	PrivateChatChannel::Common::eModType eType;
	int nPassWord;
	INT64 biCharacterDBID;
};

struct MAAddPrivateChannelMember
{
	char cWorldSetID;
	int nManagedID;				// Server management ID
	INT64 nPrivateChatChannelID;
	TPrivateChatChannelMember Member;
};

struct MAInvitePrivateChannelMember
{
	char cWorldSetID;
	INT64 nPrivateChatChannelID;
	UINT nMasterAccountDBID;	
	UINT nInviteAccountDBID;
	WCHAR wszInviteName[NAMELENMAX];
};

struct MAInvitePrivateChannelMemberResult
{
	char cWorldSetID;
	int nRet;
	UINT nMasterAccountDBID;	
};

struct MADelPrivateChannelMember
{
	char cWorldSetID;
	int nManagedID;				// Server management ID
	PrivateChatChannel::Common::eModType eType;
	INT64 nPrivateChatChannelID;	
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];
};

struct MAKickPrivateChannelMemberResult
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
};

struct MAModPrivateChannelMemberName
{	
	char cWorldSetID;
	int nManagedID;	
	INT64 nPrivateChatChannelID;	
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];
};
#endif

#if defined( PRE_WORLDCOMBINE_PVP )

struct MADelWorldPvPRoom
{	
	int nRoomIndex;
};

struct MAWorldPvPRoomJoinResult
{		
	UINT uiAccountDBID;
	int nRet;		
};

struct MAWorldPvPRoomGMCreateResult
{		
	UINT uiAccountDBID;
	UINT uiPvPIndex;
	int nRet;		
	TWorldPvPRoomDBData Data;
};

struct VIMAWorldPvPRoomBreakInto
{
	UINT	uiAccountDBID;
	INT64	biCharacterDBID;
	int		nManagedID;
	int		nRoomIndex;
	int		nRoomID;
	bool    bPvPLobby;
	UINT	uiTeam;
	BreakInto::Type::eCode BreakIntoType;
};

#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
struct MAGAChangeRoomMaster
{
	//UINT	uiAccountDBID;
	UINT	uiRoomID;
	INT64	biRoomMasterCharacterDBID;
	UINT	uiRoomMasterSessionID;
};
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

#if defined(PRE_ADD_MUTE_USERCHATTING)
struct MAMuteUserFind	//GAMA_MUTE_USERFIND, VIMA_MUTE_USERFIND
{
	UINT uiGmAccountID; //GM AccountID that imposed the sanction
	WCHAR wszMuteUserName[NAMELENMAX]; // Target character name
	int nMuteMinute; //Sanction duration (minutes)
};
struct MAMuteUserChat	//MAGA_MUTE_USERCHAT, MAVI_MUTE_USERCHAT
{
	UINT uiGmAccountID; //GM AccountID that imposed the sanction
	UINT uiMuteUserAccountID; //Sanctioned target AccountID
	int nMuteMinute; //Sanction duration (minutes)
};
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)

#if defined( PRE_MOD_71820 )
struct GAMANotifyMail
{
	UINT nToAccountDBID;		// Recipient AccountDBID
	INT64 biToCharacterDBID;
	short wTotalMailCount;		// Total mail count
	short wNotReadMailCount;	// Unread mail
	short w7DaysLeftMailCount;	// Expiration warning mail
	bool bNewMail;
};
#endif // #if defined( PRE_MOD_71820 )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
struct MAAlteiaWorldSendTicket
{	
	char cWorldSetID;
	UINT nAccountDBID;
	WCHAR wszRecvCharacterName[NAMELENMAX];
	INT64 biSendCharacterDBID;
	WCHAR wszSendCharacterName[NAMELENMAX];
};

struct MAAlteiaWorldSendTicketResult
{	
	int nRetCode;
	UINT nSendAccountDBID;
	INT64 biSendCharacterDBID;
};
#endif


#if defined(PRE_DRAGONBUFF )
struct MAApplyWorldBuff
{
	WCHAR wszCharacterName[NAMELENMAX];
	int	nManagedID;
	int nItemID;
	int nMapIdx;
};
#endif


#if defined( PRE_PVP_GAMBLEROOM )
struct MACreateGambleRoom
{
	bool bFlag;
	int nTotalGambleRoomCount;
};
struct MADelGambleRoom
{
	bool bFlag;
};
#endif
#pragma pack(pop)
