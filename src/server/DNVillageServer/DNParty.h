#pragma once

class CSyncLock;
class CDNUserSession;
class CDNUserSendManager;
class CDNParty
{
private:
	UINT m_nPartyMemberArray[PARTYMAX];		// Managed by m_nAccountDBID
	bool m_bPartyCheckMemberArray[PARTYMAX];		// Needed when pushed.
	bool m_boCompleteMember;				// When everyone is at the same gate
	UINT m_nLeaderSessionID;				//
	UINT m_nLeaderAccountDBID;				//

	ULONG m_nCreateTick, m_nPreTick, m_nCurTick;
	std::vector < std::pair <ULONG, WCHAR*> > m_VecInviteList;
	CSyncLock m_Sync;

#if defined( PRE_PARTY_DB )
	
	Party::Data m_Data;

	INT64 m_biSortPoint;

#else
	TPARTYID m_PartyID;
	int m_nChannelID;
	int m_nChannelMeritID;
	int m_nPartyMapIndex;
	WCHAR m_wszPartyName[PARTYNAMELENMAX];
	int m_nMemberMax;
	int m_nTargetMapIdx;
	TDUNGEONDIFFICULTY m_Difficulty;
	int m_nUserLvLimitMin;
	int m_nUserLvLimitMax;
	TPARTYITEMLOOTRULE m_ItemLootRule;
	TITEMRANK m_ItemLootRank;
	int m_nUpkeepCount;								//Party clear retention count
#endif // #if defined( PRE_PARTY_DB )

	UINT m_nRandomSeed;
	//Related to party move synchronization
	bool m_bIsMove;				//This is on party move (after receiving SetGameID)
	bool m_bIsStartGame;		//
	std::vector <UINT> m_VecKickList;				//Permanent kick list

public:

#if defined( PRE_PARTY_DB )
#else
	WCHAR m_wszPartyPassword[PARTYPASSWORDMAX];
	bool m_bRaidParty;
	BYTE m_cIsJobDice;
#endif // #if defined( PRE_PARTY_DB )

#ifdef _USE_VOICECHAT
	UINT m_nVoiceChannelID;
	ULONG m_nTalkingTick;
	TMemberInfo m_MemberVoiceInfo[PARTYMAX];		//Used only when returning from game to village
#endif

public:

	CDNParty(ULONG nCurTick);
	virtual ~CDNParty(void);

#if defined( PRE_PARTY_DB )
	void Create( TPARTYID PartyID, const DBCreateParty* pData );
#else
	void Create(CDNUserSession * pUser, TPARTYID PartyID, const WCHAR * pPartyName, const WCHAR * pPassWord, int nMemberMax, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemRank, int nLvLimitMin, int nLvLimitMax, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, BYTE cIsPartyJobDice);
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
	void PushParty( TAAddPartyAndMemberVillage* pPacket );
#else
	void PushParty( MAVIPushParty* pPacket );
#endif

#if defined( PRE_PARTY_DB )
	void SetPartyID( TPARTYID PartyID ){ m_Data.PartyID = PartyID; }	
	Party::Data GetPartyStructData() const { return m_Data; }
	void SetPartyStructData( Party::Data PartyData ) { m_Data = PartyData; }
	TPARTYID GetPartyID() const { return m_Data.PartyID; }
#else
	void SetPartyID( TPARTYID PartyID ){ m_PartyID = PartyID; }
	TPARTYID GetPartyID() const { return m_PartyID; }
#endif // #if defined( PRE_PARTY_DB )
	bool DoUpdate(DWORD CurTick);

	bool AddPartyMember(UINT nAccountDBID, UINT nSessionID, const WCHAR * pMemberName, int &nMemberIdx, bool boLeader = false);	// Add member
	bool DelPartyMember(CDNUserSession * pUserObj, char cKickKind);
#if defined( PRE_PARTY_DB )
	bool DelPartyMember( const TAOutParty* pA, char cKickKind );
#endif
	bool SetPartyMemberID(UINT nLeaderDBID, UINT nAccountDBID, int nMemberIdx);	//For cramming into a party

#if defined( PRE_PARTY_DB )
	bool ModifyPartyInfo(int nChannelID, const WCHAR * pName, int iPassword, BYTE cPartyMemberMax, TPARTYITEMLOOTRULE LootRule, TITEMRANK LootRank, BYTE cUserLvLimitMin, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, int iBitFlag, int &nWaitListRet);
#else
	bool ModifyPartyInfo(int nChannelID, const WCHAR * pName, const WCHAR * pPass, BYTE cPartyMemberMax, TPARTYITEMLOOTRULE LootRule, TITEMRANK LootRank, BYTE cUserLvLimitMin, BYTE cUserLvLimitMax, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, BYTE cJobDice, int &nWaitListRet);
#endif // #if defined( PRE_PARTY_DB )
	int GetPartyMemberIdx(UINT nAccountDBID);
	bool IsKickedMember(UINT nAccountDBID);

	bool bIsPasswordParty() const;
	bool bIsMove() const { return m_bIsMove; }
	bool bIsStartGame() const { return m_bIsStartGame; }
#if defined( PRE_PARTY_DB )
	int GetPartyMapIndex() const;
	const WCHAR* GetPartyName() const { return m_Data.wszPartyName; }
	int GetTargetMapIndex() const { return m_Data.iTargetMapIndex; }
	int GetMinLevel() const { return m_Data.cMinLevel; }
	int GetMemberMax() const { return m_Data.nPartyMaxCount; }
	void SetMemberMax( short nCount ){ m_Data.nPartyMaxCount = nCount; }
	TDUNGEONDIFFICULTY GetDifficulty() const { return m_Data.TargetMapDifficulty; }
	TPARTYITEMLOOTRULE GetItemLootRule() const { return m_Data.LootRule; }
	TITEMRANK GetItemLootRank() const { return m_Data.LootItemRank; }
	int GetUpkeepCount() const { return m_Data.iUpkeepCount; }
	void SetUpkeepCount( int iCount ){ m_Data.iUpkeepCount = iCount; }
	bool bIsInVillage() const { return m_Data.LocationType == Party::LocationType::Village; }
	bool bIsInGame() const { return m_Data.LocationType == Party::LocationType::Worldmap; }
	INT64 GetLeaderCharacterID() { return m_Data.biLeaderCharacterDBID; }
	int GetCurMemberCount() { return m_Data.iCurMemberCount; }
	void SetTargetMap( int nTargetMap ) { m_Data.iTargetMapIndex = nTargetMap; }
	void SetPartyType( ePartyType ePartyType ) { m_Data.Type = ePartyType; }
	void SetLocationType( Party::LocationType::eCode eType ) { m_Data.LocationType = eType; }
#if defined( PRE_ADD_NEWCOMEBACK )
	bool CheckPartyMemberAppellation( int nAppellationID );	
	bool CheckPartyMemberAppellation();	
	void ApplyJoinMemberAppellation( CDNUserSession * pSession );
	void DelOutMemberAppellation( CDNUserSession * pSession );
	void DelPartyMemberAppellation( int nAppellationID );
	void SetComebackParty( bool bFlag ) { m_Data.bCheckComeBackParty = bFlag; }
	bool bIsComebackParty() { return m_Data.bCheckComeBackParty; }
#endif
#else
	int GetPartyMapIndex() const{ return m_nPartyMapIndex; }
	const WCHAR* GetPartyName() const { return m_wszPartyName; }
	int GetTargetMapIndex() const { return m_nTargetMapIdx; }
	int GetMinLevel() const { return m_nUserLvLimitMin; }
	int GetMaxLevel() const { return m_nUserLvLimitMax; }
	int GetMemberMax() const { return m_nMemberMax; }
	void SetMemberMax( short nCount ){ m_nMemberMax = nCount; }
	TDUNGEONDIFFICULTY GetDifficulty() const { return m_Difficulty; }
	TPARTYITEMLOOTRULE GetItemLootRule() const { return m_ItemLootRule; }
	TITEMRANK GetItemLootRank() const { return m_ItemLootRank; }
	int GetUpkeepCount() const { return m_nUpkeepCount; }
	void SetUpkeepCount( int iCount ){ m_nUpkeepCount = iCount; }
#endif // #if defined( PRE_PARTY_DB )
	void SetRandomSeed( UINT uiSeed );
	UINT GetRandomSeed() const { return m_nRandomSeed; }
#if defined( PRE_PARTY_DB )
	int GetChannelID() const;
	int GetChannelMeritID() const;
#else
	int GetChannelID() const { return m_nChannelID; }
	int GetChannelMeritID() const { return m_nChannelMeritID; }
#endif // #if defined( PRE_PARTY_DB )
	void SetChannelID( int iChannelID );
	void SetMove( bool bFlag );
	void SetStartGame( bool bFlag );
	const std::vector<UINT>& GetKickList(){ return m_VecKickList; }

#if defined( PRE_PARTY_DB )
	bool CheckPassword( int iPassword );
	void SetSortPoint( INT64 biPoint ){ m_biSortPoint = biPoint; }
	INT64 GetSortPoint() const { return m_biSortPoint; }
	void GetCharNameList( std::vector<std::wstring>& vCharName );
	int GetServerID() const { return m_Data.iServerID; }
#else
	bool CheckPassword( const WCHAR* pwszPassword );
#endif // #if defined( PRE_PARTY_DB )
	bool HasEmptyPlace();
	void BanAllPartyMember(UINT nLeaderAccountDBID);					// Kick everyone~
	int GetMemberInfo(SPartyMemberInfo *pInfo);								// Get party members info
	void GetMemberCharacterDBIDInfo( std::vector<INT64>& Data, INT64 biExceptCharDBID=0 );
	int GetMemberPartsInfo(int *EquipArray);								// Get equip info of party members
	void SetMemberTargetMapIndexByStartStage();								// Set all party member values cleanly when the start button is pressed
	int CheckSameGateNo();													// Check if all party members have the same gate number
	int GetMemberCount() const;													// Get the number of party members
	void GetMembersByAccountDBID(UINT * nMembers);
	void GetMemberCheckFlag(bool * bMembers);
	bool DiffMemberGateNo(char cGateNo);
	bool InvitePartyMember(const WCHAR * pMemberName);
	bool InviteDenied(const WCHAR * pMemberName);
#if defined( PRE_PARTY_DB )
	int GetPartyInfo(SPartyListInfo * pInfo);
#else
	int GetPartyInfo(SPartyListInfo * pInfo, const sChannelInfo * pChInfo);
#endif // #if defined( PRE_PARTY_DB )
	int GetAvrUserLv();
	bool CheckCompleteParty();
	bool AdjustmentPartyInfo();
	bool ChangePartyLeader(UINT nNewLeaderAID = 0);
	bool IsPartyMemberWindowStateNone();	// true if in window_none state, otherwise false
	bool IsMember(UINT nAccountDBID);
	bool SwapMemberIndex( const CSPartySwapMemberIndex * pPacket);
	void SortMemberIndex(int nOutIndex);
	bool CheckPartyUserStartFlag();
	void SetPartyUserStartFlag(bool bFlag);
	void SetPartyUserMoveFlag(bool bFlag);
#if defined( PRE_PARTY_DB )
	ePartyType GetPartyType() const { return m_Data.Type; }
#if defined( PRE_WORLDCOMBINE_PARTY )
	int GetWorldPartyPrimaryIndex() const { return m_Data.nPrimaryIndex; }
	bool bIsWorldCombieParty() const { return Party::bIsWorldCombineParty(m_Data.Type); }
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	int GetBitFlag() const { return m_Data.iBitFlag; }
	int GetPassword() const { return GetBitFlag()&Party::BitFlag::Password ? m_Data.iPassword : Party::Constants::INVALID_PASSWORD; }
	static bool ValidatePassword( int iPassword );
#endif // #if defined( PRE_PARTY_DB )
	bool bIsRaidParty();

#ifdef _USE_VOICECHAT
	bool CreateVoiceChannel(UINT nVoiceChannelID);
	void DestroyVoiceChannel();
	void TalkingUpdate(ULONG nCurTick);
	void SetInitVoiceInfo(TMemberInfo * pInfo, int nCount);
	TMemberVoiceInfo * GetInitVoiceInfo(UINT nAccountDBID);
	
	void SendRefreshVoiceInfo(UINT nOutAccountDBID = 0);
#endif

	void SendAllRefreshParty(int nRefreshSubject);
	void SendAllRefreshGateInfo();
	void SendAllGateInfo(int nGateNo);
	void SendAllFarmInfo();
	void SendAllCancelStage( short cRetCode );
	void SendAllReadyToGame(ULONG nIP, USHORT nPort, USHORT nTcpPort);
	void SendAllRoomID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, USHORT wGameID, int nRoomID);
	void SendSelectStage(char cSelectIndex);
#if defined( PRE_PARTY_DB )
#else
	void SendPartyMemberInfo(bool bIsRaidParty, CDNUserSendManager * pSender);
#endif // #if defined( PRE_PARTY_DB )
	void SendKickPartyMember(UINT nSessionID);
	void SendPartyMemberMove(UINT nSessionID, EtVector3 vPos);
	void SendPartyMemberMoveEachOther(UINT nSessionID, int nX, int nY, int nZ);
	void SendPartyMemberPart(CDNUserSession *pSession);
	void SendPartyChat(char cType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, int nRet = ERROR_NONE);
	void SendPartyUseRadio(UINT nSessionID, USHORT nRadioID);
	void SendPartyReadyRequest();
	void SendUserProfile(UINT nSessionID, TProfile & profile);
	void SendAllSwapMemberIndex(CSPartySwapMemberIndex * pPacket);
	
	inline void SetCompleteMember(bool boFlag) { m_boCompleteMember = boFlag; }		// Everyone pressed start, ready to jump
	inline bool GetCompleteMember() { return m_boCompleteMember; }
	inline UINT GetLeaderSessionID() { return m_nLeaderSessionID; }
	inline void SetLeaderSessionID(UINT nLeaderUID) { m_nLeaderSessionID = nLeaderUID; }
	inline UINT GetLeaderAccountDBID() { return m_nLeaderAccountDBID; }
	inline void SetLeaderAccountDBID(UINT nNewLeaderAccountDBID) { m_nLeaderAccountDBID = nNewLeaderAccountDBID; }

	// Arbitrarily set gate number for all party members;
	void SetAllMemberGate(int nGateNo);

	void UpdateMasterSystemCountInfo();
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void SendPartyEffectSkillItemData();
	void ApplyPartyEffectSkillItemData( CSUseItem *pPacket, TItemData *pItemData, UINT nUseSessionID, EffectSkillNameSpace::ShowEffectType::eType eType, bool bSendDB, bool bUseItem );
	void RemovePartyEffectSkillItemData( int nSkillID, int nItemID );
#endif

	//Bonus
	void CheckFriendBonus();
#ifdef PRE_ADD_BEGINNERGUILD
	void CheckBeginnerGuildBonus();
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

protected:
	UINT PickNewLeader();
};
