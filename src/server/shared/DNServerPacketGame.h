#pragma once

#pragma pack(push, 1)

//--------------------------------------------------------------
//	Game Packet
//--------------------------------------------------------------
struct GameServerInfo
{
	BYTE cServerIdx;
	int nServerAttribute;
	ULONG nIP;
	USHORT nPort;
	BYTE cMargin;
};

struct GARegist				// GAAG_REGIST
{
	int nManagedID;						//Used when controlled by GSM via ServiceManager
	USHORT nTcpOpenPort;
	BYTE cAffinityType;							//eGameserverAffinityType
		unsigned char cInfoCount;
	GameServerInfo ServerInfo[GAMECOUNTMAX];
};

// Game -> Master
struct GAMADelayReport			//GAMA_DELAYREPORT
{
	int nServerIdx;
	int nWholeRoomCnt;
	int nRoomCnt;
	int nUserCnt;
	int nMinFrame;
	int nMaxFrame;
	int nAvrFrame;
	BYTE cHasMargin;
};

struct GAMACheckUser			// GAMA_CHECKUSER
{
	UINT nSessionID;
};

struct GAMAEnterGame			//GAME_ENTERGAME
{
	UINT nAccountDBID;
	USHORT nRoomID;
	USHORT nServerIdx;
};

struct GAMAReconnectLogin		//GAMA_RECONNECTLOGIN
{
	UINT nAccountDBID;
};

struct GAMAAddUserList			// GAMA_ADDUSERLIST
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
	USHORT nThreadIdx;
	int	nRoomID;
	TPARTYID PartyID;
};

struct GAMADelUser				// GAMA_DELUSER
{
	UINT nAccountDBID;
	BYTE cIsUnIntended;
	UINT nSessionID;
};

struct GAMAEndofRegist			//GAMA_ENDOFREGIST
{
	UINT nServerID;
};

struct GAMAChangeServer			// GAMA_CHANGESERVER
{
	UINT nAccountDBID;
};

struct GAMASetRoomID			// GAMA_SETROOMID	
{
	GameTaskType::eType GameTaskType;
	BYTE	cReqGameIDType;
	union
	{
		TINSTANCEID InstanceID;
		LadderSystem::MatchType::eCode MatchType;
	};
	BYTE	cGameID;
	BYTE	cServerIdx;
	BYTE	cVillageID;
	int		iRoomID;
	TPARTYID PartyIDForBreakInto;
#if defined( PRE_WORLDCOMBINE_PARTY )
	ePartyType Type;
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
	WorldPvPMissionRoom::Common::eReqType eWorldReqType;
#endif
	UINT nRoomMember[PARTYMAX];
};

struct GAMASetTutorialRoomID	// GAMA_SETTUTORIALROOMID	
{
	UINT nAccountDBID;
	BYTE cGameID;
	BYTE cServerIdx;
	int nRoomID;
	int	nLoginServerID;
};

struct GAMARequestNextVillageInfo	//GAMA_REQNEXTVILLAGEINFO
{
	BYTE cReqGameIDType;
	TINSTANCEID InstanceID;
	BYTE cReturnVillage;						//0false 1true;
	int nMapIndex;
	int nEnteredGateIndex;
	INT64 nItemSerial;
	TPartyData PartyData;
};

struct GAMARebirthVillageInfo	// GAMA_REBIRTHVILLAGEINFO
{
	UINT nAccountDBID;
	int nLastVillageMapIdx;
};

struct GAMAMovePvPGameToPvPLobby	// GAMA_MOVEPVPGAMETOPVPLOBBY
{
	UINT	uiAccountDBID;
	UINT	uiPvPIndex;
	int		iLastVillageMapIndex;
	bool	bIsLadderRoom;
};

struct GAMALoginState			// GAMA_LOGINSTATE
{
	UINT nAccountDBID;
};

struct GAMAVillageState			// GAMA_VILLAGESTATE
{
	UINT nAccountDBID;
	int nMapIndex;
	int nRoomID;
};

struct GAMADelPartyMember		// GAMA_DELPARTYMEMBER
{
	USHORT nServerID;
	UINT nAccountDBID;
	char cKickKind;
};

#define GAMAParty VIMAParty	//GAMA_PARTY

#define GAMAFriendAddNotice VIMAFriendAddNotice

struct GAMAPrivateChat			// GAMA_PRIVATECHAT
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

struct GAMAChat				// GAMA_CHAT
{
	UINT nAccountDBID;
	char cType;
	short wChatLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct GAMAWorldSystemMsg		// GAMA_WORLDSYSTEMMSG
{
	UINT nAccountDBID;
	char cType;
	int nID;
	int nValue;
	WCHAR wszToCharacterName[NAMELENMAX];
};

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct GAMAPrivateChannelChatMsg		// GAMA_PRIVATECHANNELCHAT
{
	UINT nAccountDBID;
	char cType;	
	short wChatLen;
	INT64 nChannelID;
	WCHAR wszChatMsg[CHATLENMAX];
};
#endif

struct GAMANotice		//GAMA_NOTICE
{
	int nLen;
	WCHAR szMsg[CHATLENMAX];
};

struct GAMABanUser		//GAMA_BANUSER
{
	UINT nAccountDBID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct GAMAUpdateWorldUserState		//GAMA_UPDATEWORLDUSERSTATE
{
	WCHAR szName[NAMELENMAX];
	int nMapIdx;
};

struct GAMAPVP_ROOMSYNCOK
{
	UINT	uiPvPIndex;
};

struct GAMAPVP_LADDERROOMSYNC
{
	INT64	biRoomIndexArr[2];
	LadderSystem::RoomState::eCode State;
};

struct GAMAPVP_BREAKINTOOK
{
	UINT	uiPvPIndex;
	UINT	uiAccountDBID;
};

struct GAMAPVP_COMMAND
{
	UINT	uiCommandType;		// See PvPCommon::GAMA_Command
	UINT	uiPvPIndex;
	UINT	uiRoomIndex;
};

#define GAMAPVPSwapMemberIndex VIMAPVPSwapMemberIndex //GAMA_PVP_SWAPTMEMBER_INDEX
#define GAMAPVPChangeMemberGrade VIMAPVPChangeMemberGrade	//GAMA_PVP_CHANGEMEMBER_GRADE

struct GAMAPvPGuildWarScore		//GAMA_PVP_GUILDWARSCORE
{
	UINT nGuildDBID;
	int nScore;
};

struct GAMAPvPGuildWarResult		//GAMA_PVP_GUILDWARRESULT
{
	UINT nWinGuildDBID;
	UINT nLoseGuildDBID;
	bool bIsDraw;
};

struct GAMAPvPGuildWarException		//GAMA_PVP_GUILDWAREXCEPTION
{
	UINT nRoomID;
	UINT nPvPIndex;
};

struct GAMAPCBangResult			// GAMA_PCBANGRESULT, // Receive PCBang related result
{
	UINT nAccountDBID;
};

struct GAMANotifyGift			// GAMA_NOTIFYGIFT
{
	UINT nToAccountDBID;	// Recipient AccountDBID
	INT64 biToCharacterDBID;
	bool bNew;
	int nGiftCount;
};

struct GAMAInvitePartyMember : public TInvitePartyInfo		//GAMA_INVITEPARTYMEMBER
{
	int nChannelID;

	UINT nInviterAccountDBID;
	INT64 biInviterCharacterDBID;
	WCHAR wszInviterName[NAMELENMAX];
	WCHAR wszInvitedName[NAMELENMAX];
	BYTE cPermitLevel;
	char cPassClassIds[PERMITMAPPASSCLASSMAX];
};

struct GAMAInvitedReturnMsg		//GAMA_INVITEPARTYMEMBER_RETURNMSG
{
	UINT nAccountDBID;
	int nRetCode;
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

typedef VIMASyncSystemMail GAMASyncSystemMail;
typedef MAVISyncSystemMail MAGASyncSystemMail;

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

namespace MasterSystem
{
	typedef VIMASyncSimpleInfo	GAMASyncSimpleInfo;
	typedef VIMASyncGraduate	GAMASyncGraduate;
	typedef VIMASyncConnect		GAMASyncConnect;
}

struct GAMAFarmDataLoaded		//GAMA_FARM_DATALOADED
{
	int nGameServerIdx;			//threadidx;
	UINT nRoomID;
	UINT nFarmDBID;
	int nMapID;
	int nFarmMaxUser;
};

struct GAMAFarmUserCount		//GAMA_FARM_USERCOUNT
{
	UINT nFarmDBID;
	UINT nFarmRoomID;
	int nCurUserCount;
	bool bStarted;
};

struct GAMAFarmIntendedDestroy	//GAMA_FARM_INTENDEDDESTROY
{
	UINT nFarmDBID;
};

struct GAMAFarmSync				//GAMA_FARM_SYNC
{
	INT64						biCharacterDBID;
	Farm::ServerSyncType::eType Type;
};

struct MAGAFarmSync
{
	UINT						uiAccountDBID;	// uiAccountDBID must come first
	INT64						biCharacterDBID;
	Farm::ServerSyncType::eType Type;
};

struct GAMAFarmSyncAddWater
{
	INT64	biCharacterDBID;
	WCHAR	wszCharName[NAMELENMAX];
	int		iAddPoint;
};

struct MAGAFarmSyncAddWater
{
	UINT	uiAccountDBID;						// uiAccountDBID must come first
	WCHAR	wszCharName[NAMELENMAX];
	int		iAddPoint;
};


struct GAMALoadUserTempData		// GAMA_LOAD_USERTEMPDATA
{
	UINT uiAccountDBID;
};

struct GAMASaveUserTempData		// GAMA_SAVE_USERTEMPDATA
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

struct GAMADuplicateLogin
{
	UINT nAccountDBID;
	UINT nSessionID;
	bool bIsDetach;
};

#pragma pack(pop)
