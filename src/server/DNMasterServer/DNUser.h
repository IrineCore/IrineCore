#pragma once

class CDNVillageConnection;
class CDNGameConnection;

class CDNUser
{
private:
	UINT m_nSessionID;
	UINT m_nAccountDBID;
	INT64 m_biCharacterDBID;
	WCHAR m_wszAccountName[IDLENMAX];
	char m_szAccountName[IDLENMAX];		// Added another char type because of fatigue 081211
	WCHAR m_wszCharacterName[NAMELENMAX];
	char m_szCharacterName[NAMELENMAX];	// Added another char type because of fatigue 081211

	int m_nLoginServerID;		//
	BYTE m_cPreVillageID;		// Find previous VillageID before moving when in STATE_CHECKVILLAGE

	BYTE m_cVillageID;			// Used when finding village
	USHORT m_nChannelID;		// Channel is set together with village
	
	USHORT m_wGameID;			// When finding Game Con
	short m_nGameServerIdx;	// ThreadIdx is set with Game	

	eUserState m_eUserState;	// State value (also changes when setting gameid or villageid)
	ULONG m_nCheckStateTick;

	char m_szIp[IPLENMAX];		// Connection IP needed for fatigue 081211
	char m_szVirtualIp[IPLENMAX];		// Connection IP needed for fatigue 081211
	bool m_bAdult;				// Whether adult or not (for fatigue check)

	bool m_bPCBang;				// Somehow pcbang sounds better than pcroom! lol
	char m_cPCBangGrade;
	int m_nAccountLevel;

	// PvP
	UINT m_uiPvPIndex;
	USHORT m_usPvPTeam;
	UINT m_uiPvPUserState;

	BYTE m_cPvPVillageID;			// PvPVillage info for returning to PvP village
	USHORT m_nPvPVillageChannelID;
	bool m_bPvPFatigue;				// Fatigue consumption On,Off
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	LadderSystem::MatchType::eCode	m_MatchType;	// Save ladder match type..
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	PvPCommon::RoomType::eRoomType m_ePvPChannelType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER


#if defined(PRE_ADD_PVP_TOURNAMENT)
	BYTE m_cJob;
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	GameQuitReward::RewardType::eType m_eUserGameQuitRewardType;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
public:

#if defined(PRE_ADD_SENDLOGOUT)
	bool m_bSendLogOut;
	CSyncLock m_SendLogOutLock;
#endif


	ULONG m_nCreateTick;			///Test
	int m_nRoomID;

	TPARTYID m_VillageCheckPartyID;			//Party does not exist on master. Temporarily kept when moving from game to village (needed for CheckUser).

	int m_nRandomSeed;
	int m_nMapIndex;
	BYTE m_cGateNo;
	BYTE m_cGateSelect;

	int	m_nDungeonClearCount;

#ifdef PRE_ADD_COMEBACK
	bool m_bComebackUser;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	bool m_bReConnectNewbieReward;
	void SetUserGameQuitRewardType(GameQuitReward::RewardType::eType eType){ m_eUserGameQuitRewardType = eType;}
	GameQuitReward::RewardType::eType GetUserGameQuitRewardType(){ return m_eUserGameQuitRewardType; }
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

#if defined(PRE_ADD_REMOTE_QUEST)
	std::vector<int> m_AcceptWaitRemoteQuestList;
#endif

public:
	CDNUser(void);
	~CDNUser(void);

#if defined(PRE_MOD_SELECT_CHAR)
	bool InitUser(LOMAAddUser *pPacket, int nServerID);
	bool InitUser(GAMAAddUserList *pPacket);
	bool InitUser(VIMAAddUserList *pPacket);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	void PreInit(int nServerID, UINT nAccountDBID, int nAccountLevel, const char * pszIP);
	void Init(UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, WCHAR *pwszCharacterName, WCHAR *pwszAccountName, USHORT nChannelID, bool bAdult, char *pszIp, char *pszVirtualIp);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	bool SetCheckVillageInfo(BYTE cVillageID, USHORT nChannelID);
	bool SetCheckGameInfo(USHORT nGameID, short nServerIdx);
	bool SetVillageInfo(BYTE cVillageID, USHORT nChannelID, bool bForce);
	bool SetGameInfo(USHORT nGameID, USHORT nServerIdx, bool bForce);
	void SetCheckReconnectLogin();

	CDNVillageConnection* GetCurrentVillageConnection();
	CDNGameConnection* GetCurrentGameConnection();


	void SetSessionID(UINT nSessionID) { m_nSessionID = nSessionID; }
	UINT GetSessionID() { return m_nSessionID; }
	void SetAccountDBID(UINT nAccountDBID) { m_nAccountDBID = nAccountDBID; }
	UINT GetAccountDBID() { return m_nAccountDBID; }
	void SetCharacterDBID(INT64 biCharacterDBID) { m_biCharacterDBID = biCharacterDBID; }
	INT64 GetCharacterDBID() { return m_biCharacterDBID; }

	void SetCharacterName(const WCHAR *pwszCharacterName);
	WCHAR* GetCharacterName() { return m_wszCharacterName; }
	const char *GetChracterNameA() { return m_szCharacterName; }

	void SetAccountName(const WCHAR *pwszAccountName);
	WCHAR* GetAccountName() { return m_wszAccountName; }
	const char* GetAccountNameA() { return m_szAccountName; } 

	void SetAdult(bool bAdult) { m_bAdult = bAdult; }
	bool IsAdult() { return m_bAdult; }
	void SetIp(char *pszIp);
	char* GetIp() { return m_szIp; }
	void SetVirtualIp(char *pszIp);
	char* GetVirtualIp() { return m_szVirtualIp; }
	void SetMapIndex(int nMapIndex) { m_nMapIndex = nMapIndex; }
	int GetMapIndex(){ return m_nMapIndex; }
	void SetLoginServerID(int nLoginID) { m_nLoginServerID = nLoginID; }
	int GetLoginServerID() { return m_nLoginServerID; }
	BYTE GetVillageID() { return m_cVillageID; }
	BYTE GetPreVillageID() { return m_cPreVillageID; }	
	USHORT GetGameID() { return m_wGameID; }
	void SetChannelID(USHORT wChannelID) { m_nChannelID = wChannelID; }
	USHORT GetChannelID() { return m_nChannelID; }
	USHORT GetGameServerIdx() { return m_nGameServerIdx; }
	void SetUserState(eUserState eState) { m_eUserState = eState; }
	eUserState GetUserState() { return m_eUserState; }
	ULONG GetCheckStateTick() { return m_nCheckStateTick; }

	eLocationState GetLocationState();

	void SetPCGrade(char cGrade);
	void SetPCBang(bool bFlag);
	char GetPCBangGrade();
	inline bool IsPCBang() { return m_bPCBang; }
	inline int GetAccountLevel() { return m_nAccountLevel; }

	// PvP
	bool bIsPvPVillage();
	bool bIsPvPLobby();
	bool bIsPvPGroupCaptain(){ return GetPvPUserState()&PvPCommon::UserState::GroupCaptain ? true : false; }
	BYTE GetPvPVillageID(){ return m_cPvPVillageID; }
	USHORT GetPvPVillageChannelID(){ return m_nPvPVillageChannelID; }
	void SetPvPVillageInfo(){ m_cPvPVillageID=m_cVillageID; m_nPvPVillageChannelID=m_nChannelID; }
	void SetPvPVillageInfo( const BYTE cVillageID, const USHORT unChannelID ){ m_cPvPVillageID=cVillageID; m_nPvPVillageChannelID=unChannelID; }
	UINT GetPvPIndex(){ return m_uiPvPIndex; }
	USHORT GetPvPTeam(){ return m_usPvPTeam; }
	UINT GetPvPUserState(){ return m_uiPvPUserState; }
	void SetPvPIndex( const UINT uiIndex ){ m_uiPvPIndex = uiIndex; }
	void SetPvPTeam( const USHORT usTeam ){ m_usPvPTeam	= usTeam; }
	void SetPvPUserState( const UINT uiState ){ m_uiPvPUserState = uiState; }

	bool bIsObserver(){ return m_usPvPTeam == PvPCommon::Team::Observer; }

	int	GetDungeonClearCount() {return m_nDungeonClearCount;}
	void SetDungeonClearCount(const int nCount) {m_nDungeonClearCount = nCount;}
	void SetPvPFatigue(bool bFatigue) { m_bPvPFatigue = bFatigue;};
	bool GetPvPFatiue() { return m_bPvPFatigue;};
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	void SetLadderMatchType(LadderSystem::MatchType::eCode MatchType) { m_MatchType = MatchType; };
	LadderSystem::MatchType::eCode GetLadderMatchType() { return m_MatchType;};
#endif
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	void SetPvPChannelType(BYTE cType) { m_ePvPChannelType = static_cast<PvPCommon::RoomType::eRoomType>(cType); }
	PvPCommon::RoomType::eRoomType GetPvPChannelType() { return m_ePvPChannelType; }
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

#if defined(PRE_ADD_REMOTE_QUEST)
	void SetAcceptWaitRemoteQuest(int nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList);
	void GetAcceptWaitRemoteQuest(int *nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList);
#endif
#if defined(PRE_ADD_PVP_TOURNAMENT)
	void SetUserJob(BYTE cJob) { m_cJob = cJob; }
	BYTE GetUserJob() { return m_cJob; }
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	void SetGameQuitUserReward(GameQuitReward::RewardType::eType eRewardType){ m_eUserGameQuitRewardType = eRewardType; }
	GameQuitReward::RewardType::eType GetGameQuitUserReward() { return m_eUserGameQuitRewardType; }
#endif
};
