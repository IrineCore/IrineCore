#pragma once

//Synchronize Rule
//Since it only uses the Process Thread Cursor via Process Thread calls, it can logically be used freely like single-threaded code.
//Exception Rule exists. Because connection-related parts and calls via IdleProcess() are invoked through a different thread cursor
//Concurrent duplicate access becomes possible. Therefore, if called by IdleProcess() (something periodic), use a sync object
//You just need to synchronize.
//- 2hogi

#include "Util.h"

class CDNDataManager;
class CConnection;
class CDNLoginConnection;
class CDNGameConnection;
class CDNVillageConnection;
class CDNUser;
class CDNPvP;
class CDNFarm;

class CDivisionManager
{
public:
	typedef struct RESETAUTHINFO
	{
	public:
		RESETAUTHINFO(UINT uiAccountDBID, UINT nSessionID, int nCurServerID, int nOldServerID, char cWorldSetID, INT64 nCharacterDBID) : m_uiAccountDBID(uiAccountDBID), m_nSessionID(nSessionID), m_uiSetTick(0), m_uiCurTick(0), m_nCurServerID(nCurServerID), m_nOldServerID(nOldServerID), m_cWorldSetID(cWorldSetID), m_nCharacterDBID(nCharacterDBID) { }
	public:
		UINT m_uiAccountDBID;
		UINT m_nSessionID;
		UINT m_uiSetTick;
		UINT m_uiCurTick;
		int m_nCurServerID;		// Use LO only (to avoid disconnecting the user of the initial LO request?)
		int m_nOldServerID;		// Purpose undecided?
		char m_cWorldSetID;		// To distinguish LO from GA / VI
		INT64 m_nCharacterDBID;	// To distinguish LO from GA / VI
	} *LPRESETAUTHINFO;

	typedef	std::map<UINT, RESETAUTHINFO>		TP_RESETAUTHLIST;		// Auth info reset caching list type (Account DBID / last access time (unit: ms))
	typedef	TP_RESETAUTHLIST::iterator			TP_RESETAUTHLIST_ITR;
	typedef	TP_RESETAUTHLIST::const_iterator	TP_RESETAUTHLIST_CTR;
	typedef	std::vector<RESETAUTHINFO>			TP_RESETAUTHAUTO;
	typedef	TP_RESETAUTHAUTO::iterator			TP_RESETAUTHAUTO_ITR;
	typedef	TP_RESETAUTHAUTO::const_iterator	TP_RESETAUTHAUTO_CTR;

public:
	enum eTick
	{
		eTickSortLoginServerList,	// Sort login server sessions
		eTickCnt,
	};

private:
	CSyncLock m_Sync;

	// Tick
	mutable DWORD m_dwTick[eTickCnt+1];		// Tick count (for common registration and use when managing the time range of a specific action per entity)

	//Connection List
	mutable CDNLoginConnection * m_pLoginConnection;				// Current login server connection
	CDNLoginConnection * m_pLoginConnectionList[LOGINCOUNTMAX+1];	// Login server connection list
	std::vector <CDNGameConnection*> m_GameServerConList;
	std::vector <CDNVillageConnection*> m_VillageServerConList;

	//UserData List
	std::map <UINT, CDNUser*> m_AccountDBIDList;	// first:AccountDBID
	std::map <UINT, CDNUser*> m_SessionIDList;		// first:SessionID
	std::map <INT64,CDNUser*> m_CharacterDBIDList;	// first:CharacterDBID
	std::map <std::wstring, CDNUser*> m_UserNameList;
	
	int m_nProcessCount;

	//PvP
	typedef std::map<UINT,CDNPvP*> _TPvPMap;
	_TPvPMap m_mPvP;
	std::list<UINT> m_listPvPIndex;
	BYTE m_cPvPLobbyVillageID;
	USHORT m_unPvPLobbyChannelID;
	int m_nPvPLobbyMapIndex;

#if defined(PRE_FIX_DELAY_MASTER_CHECKVILLAGECHANNEL)
	DWORD m_dwCheckVillageChannelStartTick;
#endif
	DWORD m_dwCheckVillageChannelTick;
	void CheckVillageChannel( DWORD dwCurTick );

	//Farm
	int m_nFarmGameindex;
	bool m_bDestroyFarm;
	std::vector <TFarmItemFromDB> m_vFarmCreateSync;
	std::map <UINT, CDNFarm*> m_mFarmList;
#ifdef PRE_MOD_OPERATINGFARM
	DWORD m_dwFarmDataCreateTick;
#endif		//#ifdef PRE_MOD_OPERATINGFARM

	int	_CheckVillageToPvPVillage( CDNVillageConnection* pVillageCon, CDNUser* pUser, char cReqType, const BYTE cNextVillageID, const int nNextVillageChannelID );
	void _CheckPvPVillageToPvPLobby( VIMAVillageToVillage* pPacket, CDNUser* pUser, const BYTE cNextVillageID, const int nNextChannelIndex );
	void _CheckVillageToPvPLobby( VIMAVillageToVillage* pPacket, CDNUser* pUser, const BYTE cNextVillageID, const int nNextChannelIndex );

	//Select Server
	struct TSelectJoin
	{
		int nSID;
		WCHAR wszCharacterName[NAMELENMAX];
	};
	std::vector <TSelectJoin> m_SelectJoinList;

#if defined(PRE_MOD_SELECT_CHAR)
	void _AddUserList(CDNUser *pUser);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)


#if defined( PRE_PVP_GAMBLEROOM )
	bool m_bGambleRoomFlag;
	DWORD m_dwGambleRoomSendTick;
#endif

public:
	CDivisionManager();
	~CDivisionManager();

#if !defined (__COVERITY__)
#endif


	//IdleProcess
	void InternalIdleProcess(ULONG nCurTick);
	void ExternalIdleProcess(ULONG nCurTick);
	void UpdatePvPRoom( const UINT uiCurTick );

	//SelectJoin
	bool AddSelectJoin(int nSID, const WCHAR * pwszCharacterName);
	void ClearSelectJoin();
	bool IsSelectJoin(const WCHAR * pwszCharacterName, int &nSID);

	// Tick
	DWORD GetTick(eTick pTick) const;
	void SetTick(eTick pTick) const;
	void SetTick(eTick pTick, DWORD dwVal) const;
	void ResetTick(eTick pTick) const;
	BOOL IsTick(eTick pTick) const;

	//Connections
	bool DelConnection(CConnection * pCon, eConnectionKey eConType);
	CDNLoginConnection * GetLoginConnection() { return m_pLoginConnection; }
	CDNLoginConnection * FindLoginConnection(const char* pIp, const int nPort);
	void SortLoginConnectionList() const;
	CDNGameConnection * GameServerConnected(const char * pIP, USHORT nPort);
	void GameServerRegist(CDNGameConnection * pCon);

#if defined(PRE_FIX_LIVE_CONNECT)
	void AddGameServerConnection(CDNGameConnection* pCon);
	void AddVillageServerConnection(CDNVillageConnection* pCon);
#endif

	CDNVillageConnection * VillageServerConnected(const char * pIP, USHORT nPort);
	void SendWorldUserStateList(CDNVillageConnection * pCon);
	CDNVillageConnection * GetFirstVillageServer();
	CDNVillageConnection* GetFirstEnableVillageServer();

	int GetGameConnectionCount();
	CDNGameConnection * GetFreeGameConnection(int * pExpServerID, int nReqRoomType);
	CDNGameConnection * GetGameConnectionByGameID(int nGameID);
	CDNGameConnection * GetGameConnectionByManagedID(int nSID );
	CDNVillageConnection * GetVillageConnectionByVillageID(int nVillageID);
#ifdef PRE_MOD_OPERATINGFARM
	bool IsFarmGameConnectionAvailable(DWORD dwCurTick);
	CDNGameConnection * GetFarmGameConnection(int * pExpServerID);
#endif		//#ifdef PRE_MOD_OPERATINGFARM
#if defined( PRE_FIX_WORLDCOMBINEPARTY )
	CDNGameConnection * GetWorldCombineGameConnection();
#endif

	void SendLoginServer(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen);
	void SendAllLoginServer(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen);
	void SendAllVillageServer(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen);
	void SendAllGameServer(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen);

	// Login Server
	CDNLoginConnection* GetFirstEnableLoginServer();

	void SendAllLoginServerDetachUser(UINT nAccountDBID);
	void SendAllLoginServerVillageInfo(UINT nUserCnt, UINT nMaxUserCount, std::vector<TVillageInfo> * vList);
	void SendAllLoginServerVillageInfoDelete(int nVillageID);

	//Server Operator
	bool GetVillageServerInfo(std::vector <TVillageInfo> * vList);
	bool GetVillageServerInfo(int nVillageID, TVillageInfo * pVillageInfo, const char * pszIP, short nPort);
	const TChannelInfo * GetChannelInfo( const int nVillageID, const int nChannelID );
	const TChannelInfo * GetChannelInfo(int nChannelID);

	bool ChannelControl(int nChannelID, bool bVisibility, int nChannelPopulation);
	bool PopulationControl(int nServerType, int nManagedID, bool bZeroPopulation);
	void SetGameConAffinityType(int nManagedID, BYTE cAffinityType);
	void UpdateUserRestraint(UINT nAccountDBID);

	bool GetGameInfoByID(int nGameID, int nGameServerID, char * pIP, USHORT * pPort, USHORT * pTcpPort);
	bool GetVillageInfo(int nMapIdx, int nOriginChannelId, int &nChannelId, BYTE &cVillageID, char * pIP, USHORT &pPort);

	void CheckPvPLobbyChannel( CDNVillageConnection* pVillageCon );

	//WaitUser Process
	void SendWaitProcess(int nServerID, MALOWaitUserProcess * pProcess);

	void SendChangeCharacterName(MAChangeCharacterName* pPacket);

	//User State
	void AddUserState(const WCHAR * pName, INT64 biChracterDBID);
	void DelUserState(const WCHAR * pName, INT64 biChracterDBID);
	void UpdateUserState(const WCHAR * pName, INT64 biChracterDBID, int nLocationState, int nCommunityState, int nChannelID = -1, int nMapIdx = -1, CDNGameConnection * pCon = NULL);

	//UserData
	UINT GetCurUserCount();
#if defined(PRE_MOD_SELECT_CHAR)
	int AddUser(LOMAAddUser *pPacket, int nServerID);
	bool AddUser(GAMAAddUserList *pPacket, int nGameID, bool bIsForceAdd = false);
	bool AddUser(VIMAAddUserList *pPacket, int nVillageID, bool bIsForceAdd = false);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int AddUser(int nServerID, UINT nAccountDBID, const BYTE * pMachineID, DWORD dwGRC, const char * pszIP, char cSelectedLanguage, BYTE cAccountLevel = 0, bool bForce=false );
	bool SetUserInfo(LOMAUserInfo *pUserInfo, int nGameID, int nGameThreadIdx, BYTE * pMachineID, DWORD &dwGRC);
	bool AddUser(int nVillageID, int nChannelID, int nGameID, int nGameThreadIdx, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, WCHAR *pwszCharacterName, WCHAR *pwszAccountName, 
		bool bAdult, char *pszIp, char *pszVirtualIp, TPARTYID PartyID, const BYTE * pMachineID, DWORD dwGRC, BYTE cPcBangGrade, char cSelectedLanguage, bool bIsForceAdd = false);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	bool DelUser(UINT nAccountDBID, bool bIsReconnectLogin = false, UINT nSessionID=0);


	bool IsExistUser(UINT nAccountDBID);
	CDNUser * GetUserByAccountDBID(UINT nAccountDBID);
	CDNUser * GetUserBySessionID(UINT nSessionID);
	CDNUser * GetUserByCharacterDBID( INT64 biCharacterDBID );
	CDNUser * GetUserByName(const WCHAR * pName);
	CDNUser * GetUserByAccountName(const WCHAR *pAccountName);

	void DetachUserloginID(int nServerID);
	void DetachUserByVillageID(BYTE cVillageID);
	void DetachUserByGameID(USHORT wGameID);
	void EraseUserListExceptAccountDBList(CDNUser *pUser);	// Function to remove users in m_SessionIDList, m_UserNameList, and m_PartyList except m_AccountDBIDList (consolidated to avoid repeating)
	void ReplaceCharacterName(CDNUser* pUser, WCHAR* wszCharacterName);
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	bool DetachUserByIP(const char * pszIP);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

	//User Check
	bool VillageCheckUser(CDNVillageConnection * pCon, UINT nSessionID);
	void CheckZombie(ULONG nCurTick);

	//Move
	void MovePvPLobbyToVillage( CDNVillageConnection* pCon, VIMAPVP_MOVELOBBYTOVILLAGE* pPacket );
	void MoveVillageToVillage(CDNVillageConnection * pCon, VIMAVillageToVillage * pPacket);
	void MoveGameToVillage(CDNGameConnection * pCon, GAMARequestNextVillageInfo * pPacket);

	//Trace User Location
	bool VillageToVillage(CDNVillageConnection * pCon, UINT nAccountDBID, int nVillageID, int nChanneID, TPARTYID PartyID);
	bool GameToVillage(CDNGameConnection * pCon, UINT nAccouDBID, int nVillageID, int nChannelID);
	bool VillageToGame(CDNVillageConnection * pCon, UINT nAccountDBID, int nGameID, int nRoomID, int nServerIdx);
	
	bool EnterVillage(UINT nAccountDBID, int nVillageID, int nChannelID);
	bool EnterGame(UINT nAccountDBID, int nGameID, int nRoomID, int nServerIdx);
	bool ReconnectLogin(UINT nAccountDBID);
	
	CDNPvP * GetPvPRoomByIdx( UINT uiIndex );
	
	//Guild
	bool SendInviteGuildMember(MAInviteGuildMember* pPacket);
	void SendInviteGuildMemberResult(VIMAInviteGuildMemberResult* pPacket);
	void SendGuildWareInfo(MAGuildWareInfo* pPacket);
	void SendGuildWareInfoResult(MAGuildWareInfoResult* pPacket);
	void SendGuildMemberLevelUp(MAGuildMemberLevelUp* pPacket);
	void SendRefreshGuildItem(MARefreshGuildItem* pPacket);
	void SendRefreshGuildCoin(MARefreshGuildCoin* pPacket);
	void SendExtendGuildWareSize(MAExtendGuildWare* pPacket);
	void SendDismissGuild(MADismissGuild* pPacket);
	void SendAddGuildMember(MAAddGuildMember* pPacket);
	void SendDelGuildMember(MADelGuildMember* pPacket);
	void SendChangeGuildInfo(MAChangeGuildInfo* pPacket);
	void SendChangeGuildMemberInfo(MAChangeGuildMemberInfo* pPacket);
	void SendGuildChat(MAGuildChat* pPacket);
	void SendChangeGuildName(MAGuildChangeName* pPacket);
	void SendChangeGuildMark(MAGuildChangeMark* pPacket);
	void SendUpdateGuildExp(MAUpdateGuildExp* pPacket);
	void SendEnrollGuildWar(MAEnrollGuildWar* pPacket);
	void SendChangeGuildWarStep(MAChangeGuildWarStep* pPacket);
	void SendSetGuildWarPoint(MASetGuildWarPoint* pPacket);
#if defined(PRE_FIX_75807)
	void SendSetGuildWarPreWinGuildGameServer(MAGuildWarPreWinGuild* pPacket);
#endif //#if defined(PRE_FIX_75807)
	void SendSetGuildWarPreWinGuild(MAGuildWarPreWinGuild* pPacket);

	void SendAddGuildWarPoint(MAAddGuildWarPoint* pPacket);
	void SendGuildWarInfoToVillage(CDNVillageConnection* pCon);
	void SendGuildWarInfoToGame(CDNGameConnection* pCon);
	void SendGetGuildWarTrialStats();
	void SendSetGuildWarTournamentInfo(SGuildTournamentInfo* pGuildTournamentInfo);
	void SendSetGuildwarFinalProcess(char cGuildFinalPart, __time64_t tBeginTime);
	void SendSetGuildWarSecretMission(MASetGuildWarSecretMission * pPacket);
	void SendSetGuildWarFinalResult(MAVISetGuildWarFinalResult* pPacket);
	void SendSetGuildWarPreWinSkillCoolTime(MAGuildWarPreWinSkillCoolTime* pPacket);
	void SendSetGuildWarSchedule(MASetGuildWarEventTime* pPacket);
	void SendSetGuildWarTournamentWin(MASetGuildWarTournamentWin* pPacket);
	void SendResetGuildWarBuyedItemCount();
	void SendGuildWarAllStop();

	void SendGuildRecruitMember(MAGuildRecruitMember* pPacket);
	void SendAddGuildRewardItem(MAGuildRewardItem* pPacket);
	void SendExtendGuildSize(MAExtendGuildSize* pPacket);

	void SendSetGuildWarTrialRanking(MASetGuildWarPointRunningTotal* pPacket);
	void SendGuildWarRefreshGuildPoint(MAGuildWarRefreshGuildPoint* pPacket);
	void SendUpdateGuildWare(int nGuildID);

	//Friend
	void SendFriendAddNotice(UINT nAddedAccountDBID, const WCHAR * pAddName);

	//GameRoom
	bool RequestGameRoom(BYTE cReqGameIDType, UINT uiIndex, int nRandomSeed, int nMapIndex, char cGateNo, char cStageConstructionLevel, bool bDirectConnect, bool bTutorial, int nServerID /* Enter LO ID only for tutorial */);
	bool RequestGameRoom(VIMAReqGameID * pPacket);
	bool RequestGameRoom( VIMALadderReqGameID* pPacket );

#if defined( PRE_WORLDCOMBINE_PVP )
	bool RequestGameRoom( VIMACreateWorldPvPRoom* pPacket, BYTE cVillageID );
#endif

#if defined( PRE_ADD_FARM_DOWNSCALE )
	bool RequestFarmGameRoom(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart, int &nServerID, int iAttr );
#elif defined( PRE_ADD_VIP_FARM )
	bool RequestFarmGameRoom(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart, int &nServerID, Farm::Attr::eType Attr );
#else
	bool RequestFarmGameRoom(UINT nFarmDBID, int nMapID, int nMaxUser, bool bStart, int &nServerID);
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	bool SetGameRoom(CDNGameConnection * pGameCon, GAMASetRoomID * pMsg);
	bool SetTutorialGameRoom(int nLocalGameID, UINT nAccountDBID, int nGameID, int nServerIdx, int nRoomID, int nLoginServerID);
	void ExceptionPvPRoom(GAMASetRoomID* pMsg, USHORT& unVillageChannelID);

	//Chat
#ifdef PRE_ADD_DOORS
	int PrivateChat(UINT nSenderAccountDBID, const WCHAR * pRecieverName, char cType, const WCHAR * pChatMsg, int nLen, INT64 biDoorDestCharacterDBID);
#else		//#ifdef PRE_ADD_DOORS
	int PrivateChat(UINT nSenderAccountDBID, const WCHAR * pRecieverName, char cType, const WCHAR * pChatMsg, int nLen);
#endif		//#ifdef PRE_ADD_DOORS	
	int ZoneChat(UINT nSenderAccountDBID, const WCHAR * pChatMsg, int nLen, int nMapIdx);
#if defined(PRE_ADD_WORLD_MSG_RED)
	int WorldChat(UINT nSenderAccountDBID, const WCHAR * pChatMsg, int nLen, char cChatType = CHATTYPE_WORLD);
#else	// #if defined(PRE_ADD_WORLD_MSG_RED)
	int WorldChat(UINT nSenderAccountDBID, const WCHAR * pChatMsg, int nLen);
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)
	int WorldSystemMsg(UINT nSenderAccountDBID, char cType, int nID, int nValue, WCHAR* pwszToCharacterName);
#ifdef PRE_ADD_DOORS
	int DoorsChat(const WCHAR * pSenderName, INT64 biCharacterID, const WCHAR * pMessage, int nLen);
#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
	int DoorsGuildChat(INT64 biCharacterID, int nGuildID, const WCHAR * pMessage, int nLen);
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
#endif		//#ifdef PRE_ADD_DOORS
#if defined( PRE_PRIVATECHAT_CHANNEL )
	int PrivateChannelChat(UINT nSenderAccountDBID, const WCHAR * pChatMsg, int nLen, INT64 nChannelID);
#endif

	//PvP
	CDNPvP*	GetPvPRoom( const UINT uiAccountDBID );

	void PushPvPIndex( const UINT uiIndex ){ m_listPvPIndex.push_back(uiIndex); }
	short CreatePvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_CREATEROOM* pPacket, UINT* nOutPvPIndex=NULL );
	bool DestroyPvPGameRoom(const UINT uiPvPIndex);
	short ModifyPvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_MODIFYROOM* pPacket );
	void LeavePvPRoom( const UINT unAccountDBID, const bool bReq );
	short JoinPvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_JOINROOM* pPacket );
	short RandomJoinPvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_RANDOMJOINROOM* pPacket );
	void ChangePvPTeam( CDNVillageConnection* pVillageCon, const VIMAPVP_CHANGETEAM* pPacket );
	void BanPvPRoom( CDNVillageConnection* pVillageCon, const VIMAPVP_BAN* pPacket );
	void EnterLobby( CDNVillageConnection* pVillageCon, const VIMAPVP_ENTERLOBBY* pPacket );
	void ChangePvPRoomCaptain( CDNVillageConnection* pVillageCon, const VIMAPVP_CHANGECAPTAIN* pPacket );
	void ReadyPvP( CDNVillageConnection* pVillageCon, VIMAPVP_READY* pPacket );
	short StartPvP( CDNVillageConnection* pVillageCon, VIMAPVP_START* pPacket, bool bFromUser = false );
	bool ForceStopPvP(const UINT uiPvPIndex, const WCHAR * pGuildName = NULL);
	void SetPvPRoomState( const UINT uiPvPIndex, const UINT uiRoomState );
	void SetPvPRoomSyncOK( const UINT uiPvPIndex );
	void SetPvPBreakIntoOK( const GAMAPVP_BREAKINTOOK* pPacket );
	void SetPvPFinishGameMode( const UINT uiPvPIndex, const UINT uiRoomIndex );
	void SetDestroyPvPGameRoom( const UINT uiPvPIndex, const UINT uiRoomIndex );
	void SetPvPNoMoreBreakInto( const UINT uiPvPIndex, const UINT uiRoomIndex );
	void SetPvPFatigueOption(const VIMAPVP_FATIGUE_OPTION* pPacket );
	bool SetGuildWarMemberGrade(UINT nAccountDBID, bool bAsign, USHORT nType, UINT nTargetSessionID, USHORT &nOutUserState, USHORT &nTeam);
	bool SetPvPMemberIndex(UINT nAccountDBID, BYTE cCount, const TSwapMemberIndex * pIndex, USHORT &nTeam);
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	void SetPvPChangeChannel(CDNVillageConnection * pVillageCon, const PVP_CHANGECHANNEL * pChange);
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#if defined(PRE_ADD_PVP_TOURNAMENT)
	bool SwapTournamentIndex(UINT nAccountDBID, char cSourceIndex, char cDestIndex);
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)

	BYTE GetPvPLobbyVillageID(){ return m_cPvPLobbyVillageID; }
	USHORT GetPvPLobbyChannelID(){ return m_unPvPLobbyChannelID; }
	int GetPvPLobbyMapIndex() { return m_nPvPLobbyMapIndex; }

#if defined( PRE_PVP_GAMBLEROOM )
	void DeletePvPGambleRoom();
#endif
	//Farm
	//process
	void CheckProcessFarm();
#ifdef PRE_MOD_OPERATINGFARM
	void CheckFarmSync(DWORD dwCurTick);
#else		//#ifdef PRE_MOD_OPERATINGFARM
	void CheckFarmSync();
#endif		//#ifdef PRE_MOD_OPERATINGFARM

	//
#ifdef PRE_MOD_OPERATINGFARM
	bool IsFarmConnectionWatingTime(DWORD dwCurTick);
	void SetCreateFarmDataTick(DWORD dwTick) { m_dwFarmDataCreateTick = dwTick; }
	CDNFarm * CreateFarm(TFarmItemFromDB &FarmItem, DWORD dwCurTick);
#else		//#ifdef PRE_MOD_OPERATINGFARM
	CDNFarm * CreateFarm(TFarmItemFromDB &FarmItem);
#endif		//#ifdef PRE_MOD_OPERATINGFARM
	CDNFarm * GetFarm(UINT nFarmDBID);
	bool DestroyFarm(UINT nFarmDBID);

	bool SetFarmDataLoaded(UINT nFarmDBID, int nGameServerID, int nGameServerIdx, UINT nRoomID, int nMapID, int nFarmMaxUser);
	bool SetFarmUpdateUserCount(UINT nFarmDBID, UINT nRoomID, int nCurUserCount, bool bStarted, int nManagedID);

	// LadderSystem
	void LeaveLadderSystem( UINT uiAccountDBID );

	//notice
	void Notice(const WCHAR * pMsg, const int nLen, int nShowSec);
	void NoticeZone(int nMapIndex, const WCHAR * pMsg, const int nLen, int nShowSec);
	void NoticeChannel(int nChannelID, const WCHAR * pMsg, const int nLen, int nShowSec);
	void NoticeServer(int nManagedID, const WCHAR * pMsg, const int nLen, int nShowSec);		//Server notices can only be sent through ServiceManager.
	void NoticeCancel();

	int SendInvitePartyMember(int nGameServerID, const GAMAInvitePartyMember * pInvitePartyMember);
	bool SendInvitePartyMemberResult(int nGameServerID, const WCHAR * pwszInviterName, const WCHAR * pwszInvitedName, int nRetCode);
	bool SendInvitedPartyMemberReturn(UINT nAccountDBID, int nRetCode);

	//Net Func
	bool SendDetachUser(UINT nAccountDBID, bool bIsReconnectLogin = false, bool bIsDuplicate = false, UINT nSessionID = 0);

	bool SendDetachUserAllVillage(UINT nAccountDBID, INT nSessionID, BYTE cCurVillageID, bool bIsDuplicate );
	bool SendDetachUserAllGame(UINT nAccountDBID, INT nSessionID, INT nGameID, bool bIsDuplicate);

	void SendNotifyMail(UINT nToAccountDBID, INT64 biToCharacterDBID, short wTotalMailCount, short wNotReadMailCount, short w7DaysLeftCount, bool bNewMail);
	void SendNotifyMarket(UINT nSellerAccountDBID, INT64 biSellerCharacterDBID, int nItemID, short wCalculationCount);
	void SendNotifyGift(UINT nToAccountDBID, INT64 biToCharacterDBID, bool bNew, int nGiftCount);
	// void SendLoginDelUser(UINT nAccountDBID); // Send deluser to login server
	void SendVillageInfo();
	void SendVillageChannelShowInfo( USHORT unChannelID, bool bShow );
	void SendFarmInfo();
	void GetFarmInfo(TServiceReportMaster * pData);
	void SendUpdateWorldEventCounter( MAUpdateWorldEventCounter* pPacket );
	void SendPCBangResult(CDNUser *pUser);

	// Common
	void OnDisconnectServer( const int iConnectionKey, const int iServerID, int nManagedID );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SyncSystemMail( VIMASyncSystemMail* pMail );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SyncMasterSystemSimpleInfo( MasterSystem::VIMASyncSimpleInfo* pPacket );
	void SyncMasterSystemGraduate( MasterSystem::VIMASyncGraduate* pPacket );
	void SyncMasterSystemConnect( MasterSystem::VIMASyncConnect* pPacket );
	void SyncFarm( GAMAFarmSync* pPacket );
	void SyncFarmAddWater( GAMAFarmSyncAddWater* pPacket );

	void SendLoginUserDetach(UINT nAccountDBID, int nLoginServerID);
#if defined(PRE_ADD_QUICK_PVP)
	int MakeQuickPvPRoom(UINT nMasterAccountDBID, UINT nSlaveAccountDBID);
#endif
#if defined (PRE_ADD_BESTFRIEND)
	bool SendSearchBestFriend(MASearchBestFriend* pPacket);
	bool SendRegistBestFriend(MARegistBestFriend* pPacket);
	void SendRegistBestFriendResult(MARegistBestFriendResult* pPacket);
	void SendCompleteBestFriend(MACompleteBestFriend* pPacket);
	void SendEditBestFriendMemo(MAEditBestFriendMemo* pPacket);
	void SendCancelBestFriend(MACancelBestFriend* pPacket);
	void SendCloseBestFriend(MACloseBestFriend* pPacket);
	void SendLevelBestFriend(MALevelUpBestFriend* pPacket);
#endif

#if defined( PRE_WORLDCOMBINE_PARTY )	
	int GetWorldPartyMember( MAGetWorldPartyMember* pPacket );
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
	int AddPrivateChatChannel( MAAddPrivateChannel* pPacket );
	int AddPrivateChatChannelMember( MAAddPrivateChannelMember* pPacket );
	int InvitePrivateChatChannelMember( MAInvitePrivateChannelMember* pPacket );
	int InviteResultPrivateChatChannelMember( MAInvitePrivateChannelMemberResult* pPacket );	
	int DelPrivateChatChannelMember( MADelPrivateChannelMember* pPacket );
	int KickPrivateChatChannelMemberResult( MAKickPrivateChannelMemberResult* pPacket );
	int ModPrivateChatChannel( MAModPrivateChannel* pPacket );	
	int ModPrivateChatChannelMemberName( MAModPrivateChannelMemberName* pPacket );	
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	int AlteiaSendTicket( MAAlteiaWorldSendTicket* pPacket );	
	int AlteiaSendTicketResult( MAAlteiaWorldSendTicketResult* pPacket );	
#endif
#if defined( PRE_DRAGONBUFF )
	int SendApplyWorldBuff( MAApplyWorldBuff* pPacket );
#endif


#if defined( PRE_PVP_GAMBLEROOM )
	void SetGambleRoomFlag( bool bFlag ) { m_bGambleRoomFlag = bFlag;}
#endif

};

extern CDivisionManager * g_pDivisionManager;