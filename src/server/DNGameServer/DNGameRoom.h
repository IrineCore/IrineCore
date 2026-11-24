
#pragma once

//Actual game room! .. -_-;
//The created room is managed and used by the game server and created session object

#include "GameListener.h"
#include "DnParts.h"
#include "DNPeerManager.h"
#include "DnDamageBase.h"
#include "DnSkill.h"
#include "DNEvent.h"
#include "DnActor.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "DnPlayerChecker.h"
#include "DNChatRoomManager.h"


const int DEFAULT_ACTOR_BROADTICK = 300;
const int DEFAULT_PACKETCULLING_DISTANCE = 1500;

class CDNUserSession;
class CTaskManager;
class CDnGameTask;
class CDnItemTask;
class CDnSkillTask;
class CDnWorld;
class CDnPartyTask;
class CDnGuildTask;
class CDnItem;
class CDNRUDPGameServer;
class CRandom;
class CEtCollisionMng;
class CPvPGameMode;
class CDNDBConnection;
namespace MasterSystem
{
	class CRewardSystem;
}
#if defined(PRE_ADD_ACTIVEMISSION)
#include "DNEventSystem.h"
#endif

class CDnChatTask;
class CDNChatRoom;

extern TGameConfig g_Config;

class CDNGameRoom : public CMultiRoom
{
private:

	BYTE m_cReqGameIDType;
	TINSTANCEID m_InstanceID;

	CDNDBConnection* m_pDBConnection;
	BYTE m_cDBThreadID;

	void _CalcDBConnection( UINT uiRoomID );

	std::vector<std::pair<int,int>>	m_vChangeMapQueue;

	//test
	ULONG m_ActorSendTick;
	ULONG m_ActorTick;
	LONG m_nCullingDistance;

	bool m_bStageStargLog;
	bool m_bStageEndLog;
	bool m_bGotoVillageFlag;
#if defined( PRE_ADD_DIRECTNBUFF )	
	std::list<int> m_DirectPartyBuffItemList;	// Item ID
#endif

#if defined( PRE_TRIGGER_LOG )
	bool			m_bLog;
	LARGE_INTEGER	m_liFrequency;
	LARGE_INTEGER	m_liStartTime;
	LARGE_INTEGER	m_liCurTime;
#endif // #if defined( PRE_TRIGGER_LOG )

	WCHAR m_wszInvitedCharacterName[NAMELENMAX];
	ULONG m_nInivitedTime;

	bool  m_bCheckTick;

	void _DeleteSession();
	std::vector<CDNUserSession*> m_vDeleteSession;
	volatile bool m_bRoomCrash;

protected:

	///////////////////////////////////////////////////////////////////////////////////////
	//Not directly used on game server; for refreshing party info if Master restarts.
#if defined( PRE_PARTY_DB )
	Party::Data m_PartyStructData;
#else
	int m_nMemberMax;
	int m_nUserLvMin;
	int m_nTargetMapIdx;
	TDUNGEONDIFFICULTY m_PartyDifficulty;
	WCHAR m_wszPartyName[PARTYNAMELENMAX];
	int m_nUpkeepCount;
	int m_nUserLvMax;
	BYTE m_cIsJobDice;
	WCHAR m_wszPartyPass[PARTYPASSWORDMAX];
#endif
	
	int m_nChannelID;
	UINT m_nKickedMemberList[PARTYKICKMAX];

#if defined( PRE_WORLDCOMBINE_PVP )
	WorldPvPMissionRoom::Common::eReqType m_eWorldReqRoom;
	int m_nWorldPvPRoomDBIndex;
	bool m_bWorldPvPRoomStart;
	TWorldPvPRoomDBData m_tPvPRoomDBData;
	UINT m_nCreateGMAccountDBID;
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	bool m_bAlteiaWorld;
	bool m_bAlteiaWorldMap;	
	DWORD m_dwAlteiaWorldStartTime;
	DWORD m_dwAlteiaWorldPlayTime;
#endif
	///////////////////////////////////////////////////////////////////////////////////////

public:

	long m_lRoomProcessInterLocked;

#if defined( PRE_THREAD_ROOMDESTROY )
	enum eRoomDestoryStep
	{
		None = 0,
		PushQueue,
		Destroyed,
		Max,
	};
	long m_lRoomDestroyInterLocked;

#endif // #if defined( PRE_THREAD_ROOMDESTROY )

#ifdef PRE_ADD_FRAMEDELAY_LOG
	DWORD m_dwProcessElapsedTime;
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
	

	bool IsRoomCrash() { return m_bRoomCrash; }
	void SetRoomCrashFlag( bool bFlag ){ m_bRoomCrash = bFlag; }

#if defined( PRE_TRIGGER_LOG )
	// test
	double m_dTriggerTime;
	double m_dProcessTime;
#endif // #if defined( PRE_TRIGGER_LOG )
	bool m_bForceDestroyRoom;
	bool m_bFinalizeRoom;
	//test
	void SetActorTick(int nTick) { m_ActorTick = nTick; }
	void SetCullingDistance(int nDistance) { m_nCullingDistance = nDistance; }

	CDNGameRoom( CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket );
	virtual ~CDNGameRoom();
	
	void	FinalizeGameRoom();

	int m_iWorldID;
	int m_iPartMemberCnt;
	int m_iMapIdx;
#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	int m_iRootMapIndex;
#endif
	int m_iGateIdx;
	int m_iGateSelect;
	TDUNGEONDIFFICULTY m_StageDifficulty;
	int m_iRandomSeed;

#if defined(PRE_FIX_INITSTATEANDSYNC)
	volatile bool m_bInitStateAndSyncReserved;
	int m_nReservedMapIdx;
	int m_nReservedGateIdx;
	int m_nReservedRandomSeed;
	TDUNGEONDIFFICULTY m_ReservedStageDifficulty;
	bool m_bReservedDirectConnect;
	int m_bReservedGateSelect;
#endif

#if !defined( PRE_PARTY_DB )
	TPARTYITEMLOOTRULE m_ItemLootRule;
	TITEMRANK m_ItemLootRank;
#endif
	int m_nMeritBonusID;
	bool m_bDirectConnect;
	GameTaskType::eType m_GameTaskType;
	GameTaskType::eType GetGameTaskType() const { return m_GameTaskType; }

#ifdef _USE_VOICECHAT
	UINT m_nVoiceChannelID[PvPCommon::TeamIndex::Max];
	ULONG m_nTalkingTick;
#endif

	UINT m_nPartyMemberIndex[PARTYMAX];

	int	m_nEventRoomIndex;

	CDNChatRoomManager m_ChatRoomManager;

	CMtRandom m_mtRandom;

	std::map<const DWORD,DnDropItemHandle> m_mDropItem;

	struct DungeonClearInfo {
		BYTE cRewardItemType[4];
		TItem RewardItem[4];
//		int nRewardItemID[4];
		char cSelectRewardItem;
		char cSelectRemainCount;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		bool bNamedGiveResult[4];
#endif

		DungeonClearInfo() {
//			memset( nRewardItemID, 0, sizeof(nRewardItemID) );
			memset( RewardItem, 0, sizeof(RewardItem) );
			memset( cRewardItemType, 0, sizeof(cRewardItemType) );
			cSelectRewardItem = 0;
			cSelectRemainCount = 0;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
			memset( bNamedGiveResult, 0, sizeof(bNamedGiveResult));
#endif
		}
		void ResetSelectArray() { cSelectRewardItem = 0; }
		void SetFlag( int nIndex, bool bCheck ) {
			if( bCheck )
				cSelectRewardItem |= (char)( 0x01 << nIndex );
			else cSelectRewardItem |= ~(char)( 0x01 << nIndex );
		}
		bool IsFlag( int nIndex ) {
			return ( cSelectRewardItem & (char)( 0x01 << nIndex ) ) ? true : false;
		};
	};

	struct PartyStruct {
		CDNUserSession *pSession;
		bool bLeader;
		int nEnteredGateIndex;
		int nUsableRebirthCoin;
		std::list<int> ReverseItemList;		
		int nUsableRebirthItemCoin;
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
		std::map<int,int> UseLimitItem;
#endif

		CDnItem * pEquip[EQUIPMAX];
		CDnItem * pInventory[INVENTORYMAX];

		CDnItem * pCashEquip[CASHEQUIPMAX];
		std::map<INT64, CDnItem*> pMapCashInventory;

		CDnItem * pGlyph[GLYPHMAX];
#if defined(PRE_ADD_TALISMAN_SYSTEM)
		CDnItem * pTalisman[TALISMAN_MAX];
#endif
		CDnItem *pVehicleEquip[Vehicle::Slot::Max];
		std::map<INT64, CDnItem*> pMapVehicleInventory;
		CDnItem *pPetEquip[Pet::Slot::Max];

//		PlayInfoStruct PlayInfo;
#if !defined( PRE_ADD_DUNGEONCLEARINFO )
		DungeonClearInfo ClearInfo;
#endif

		PartyStruct() {
			pSession = NULL;
			bLeader = false;
			nEnteredGateIndex = -1;
			nUsableRebirthCoin = -1;
			ReverseItemList.clear();			
			nUsableRebirthItemCoin = 0;
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
			UseLimitItem.clear();
#endif
			memset( pEquip, 0, sizeof(pEquip) );
			memset( pCashEquip, 0, sizeof(pCashEquip) );
			memset( pInventory, 0, sizeof(pInventory) );
			memset( pGlyph, 0, sizeof(pGlyph) );
#if defined(PRE_ADD_TALISMAN_SYSTEM)
			memset( pTalisman, 0, sizeof(pTalisman) );
#endif
			pMapCashInventory.clear();
			memset( pVehicleEquip, 0, sizeof(pVehicleEquip) );
			memset( pPetEquip, 0, sizeof(pPetEquip) );
			pMapVehicleInventory.clear();
		}

		void ReleaseEquipInventory()
		{
			for( int j=EQUIP_FACE; j<EQUIPMAX; j++ ) {
				if( j == EQUIP_WEAPON1 || j == EQUIP_WEAPON2 ) continue;
				SAFE_DELETE( pEquip[j] );
			}
			for( int j=CASHEQUIP_HELMET; j<CASHEQUIPMAX; j++ ) {
				if( j == CASHEQUIP_WEAPON1 || j == CASHEQUIP_WEAPON2 ) continue;
				SAFE_DELETE( pCashEquip[j] );
			}
			for( int j=GLYPH_ENCHANT1; j<GLYPHMAX; j++ ) {
				SAFE_DELETE( pGlyph[j] );
			}
#if defined(PRE_ADD_TALISMAN_SYSTEM)
			for( int j=0; j<TALISMAN_MAX; j++ ) {
				SAFE_DELETE( pTalisman[j] );
			}
#endif
			for( int j=0; j<INVENTORYMAX; j++ )
				SAFE_DELETE( pInventory[j] );

			std::map<INT64, CDnItem*>::iterator iter;
			for (iter = pMapCashInventory.begin(); iter != pMapCashInventory.end(); ++iter )
				SAFE_DELETE(iter->second);
			pMapCashInventory.clear();

			for( int j=Vehicle::Slot::Body; j<Vehicle::Slot::Max; j++ )
			{
				SAFE_DELETE( pVehicleEquip[j] );
				SAFE_DELETE( pPetEquip[j] );
			}

			for (iter = pMapVehicleInventory.begin(); iter != pMapVehicleInventory.end(); ++iter )
				SAFE_DELETE(iter->second);
			pMapVehicleInventory.clear();
		}
	};

	struct PartyBackUpStruct
	{
		int nUsableRebirthCoin;
		std::list<int> ReverseItemList;		
		int nUsableRebirthItemCoin;
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
		std::map<int,int> UseLimitItem;
#endif
		int	nHPPercent;
		int nSPPercent;

		PartyBackUpStruct()
		{
			nUsableRebirthCoin	= 0;
			ReverseItemList.clear();			
			nUsableRebirthItemCoin = 0;
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
			UseLimitItem.clear();
#endif
			nHPPercent = 0;
			nSPPercent = 0;
		}

		PartyBackUpStruct& operator=( PartyStruct& Struct );
	};

#if defined( PRE_FIX_49129 )
	struct PartyFirstStruct
	{		
		//Add more here later if needed
		int nIndex;
		INT64 biCharacterDBID;		// Character DB ID		
#if defined( PRE_ADD_DUNGEONCLEARINFO )
		DungeonClearInfo ClearInfo;
#endif
		PartyFirstStruct()
		{			
			nIndex = 0;
			biCharacterDBID = 0;
		}
	};
#endif

	void DestroyGameRoom(bool bForce = false);
	int GetRoomState() { return m_GameState; }	//Get the state.
	unsigned long GetGameTick() { return timeGetTime() - m_iPivotTick; }

	//Process
	void FinalizeProcess();
	void Process();
	void _ProcessImpl();
	void TcpProcess();
#if defined( STRESS_TEST )
	void OrgProcess();
#endif
	virtual void FarmUpdate() {}

	void CopyDBConnectionInfo( CDNDBConnection*& pDBCon, BYTE& cThreadID )
	{
		pDBCon		= m_pDBConnection;
		cThreadID	= m_cDBThreadID;
	}

	BYTE m_cSeqLevel;	//Sequencing value for position sync
	void SendSeqLevel();
	
	//Field move, dungeon move, etc.
#if defined(PRE_FIX_INITSTATEANDSYNC)
	void ReserveInitStateAndSync(int iMapIdx, int iGateIdx, int iRandomSeed, TDUNGEONDIFFICULTY StageDifficulty, bool bDirectConnect, int iGateSelect = 0);
	void ResetReservedStateAndSyncData();
	void InitStateAndSync();
#else
	void InitStateAndSync(int iMapIdx, int iGateIdx, int iRandomSeed, TDUNGEONDIFFICULTY StageDifficulty, bool bDirectConnect, int iGateSelect = 0);					
#endif	//#if defined(PRE_FIX_INITSTATEANDSYNC)
	bool LoadData(bool bContinue);	//Only to load using external thread cursor.	

	CDNUserSession* CreateBreakIntoGameSession( WCHAR* wszAccountName, const WCHAR * pwszChracterName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bAdult,
		char cPCBangGrade, int BreakIntoType );

	CDNUserSession* CreateGameSession(WCHAR * wszAccountName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bIsMaster, bool bTutorial, bool bAdult, 
		char cPCBangGrade, TMemberVoiceInfo * pInfo = NULL);		//Create session per room in the room at initial setup.
	bool VerifyMember(CDNUserSession * pSession);
	void RemoveMember(CDNUserSession * pSession, wchar_t * pwszIdent);
	void CheckRemovedMember();
	void CheckRudpDisconnectedMember();
	void UserUpdate( DWORD dwCurTick );

	//Lisetner
	void AddGameListener(CGameListener * pListener);
	void RemoveGameListener(CGameListener * pListener);

	//dispatch msg
	int OnDispatchMessage(CDNUserSession * pSession, int iMainCmd, int iSubCmd, char *pData, int iLen, BYTE cSeqLevel);
	
	//Room UserData
	bool IsMemberEmpty() { return m_VecMember.empty(); }
	__forceinline DWORD GetUserCount() { return (DWORD)m_VecMember.size(); }
	DWORD GetUserCountWithoutGM();
	DWORD GetRealUserCountWithoutGM();
	DWORD GetGMCount();
#ifdef IRINE_MOD_PLAYER_BOTS
	bool HasAIBots();
	int ResolveTargetMapIndex( int nMapIdx, int nGateIdx, int nGateSelect, bool bDirectConnect );
	int GetTargetMapAIBotCount( int nTargetMapIdx, bool bContinueStage, int nCurrentBots, TDUNGEONDIFFICULTY StageDifficulty );
	bool EnsureAIBotsForMap( int nTargetMapIdx, bool bContinueStage, TDUNGEONDIFFICULTY StageDifficulty );
	void DespawnAndRemoveAIBots( bool bNotifyClients );
#else
	bool HasAIBots() { return false; }
	int ResolveTargetMapIndex( int nMapIdx, int /*nGateIdx*/, int /*nGateSelect*/, bool /*bDirectConnect*/ ) { return nMapIdx; }
	int GetTargetMapAIBotCount( int /*nTargetMapIdx*/, bool /*bContinueStage*/, int /*nCurrentBots*/, TDUNGEONDIFFICULTY /*StageDifficulty*/ ) { return 0; }
	bool EnsureAIBotsForMap( int /*nTargetMapIdx*/, bool /*bContinueStage*/, TDUNGEONDIFFICULTY /*StageDifficulty*/ ) { return false; }
	void DespawnAndRemoveAIBots( bool /*bNotifyClients*/ ) {}
#endif
	DWORD GetUserCount( int iTeam );
	bool GetOutMemberExist();
	enum eGetUserCountType
	{
		ePICKUPITEM
	};
	DWORD GetPartyUserCount(eGetUserCountType type);
	DWORD GetStartMemberCount() { return m_iPartMemberCnt; }
	DWORD GetLiveUserCount();
	DWORD GetUserCountWithoutPartyOutUser();
	CDNUserSession * GetUserData(DWORD dwIndex);
	CDNUserSession * GetUserSession(UINT nSessionID);
	DWORD GetPartyUserClassCount(int nClassID);

	//PartyData
	bool			AddPartyStruct( CDNUserSession* pSession, bool bLeader);
	PartyStruct*	GetPartyData(DWORD dwIndex) { return &m_VecMember[dwIndex]; }
	PartyStruct*	GetPartyData(CDNUserSession * pSession);
	PartyStruct*	GetPartyData( WCHAR *pwszCharacterName );
	CDNUserSession*	GetUserSessionByCharDBID( INT64 biCharDBID );
	PartyStruct*	GetPartyDatabySessionID(UINT nSessionID, int &Seq);
	bool SwapLeader(UINT nDelSessionID, UINT &nNewLeaderSessionID);
	bool IsPartyLeader(UINT nSessionID);
	int GetLeftMemberIndex();
	bool SetPartyMemberIndex(int nIdx, UINT nSessionID, int nTeam = -1);
	bool PartySwapMemberIndex(CSPartySwapMemberIndex * pPacket);
	bool IsRaidParty();
	void SortMemberIndex(int nOutIndex);

#if defined( PRE_FIX_49129 )
	DWORD GetFirstUserCount() { return (DWORD)m_MapFirstPartyMember.size(); }
	void ResetFirstPartyMember() { m_MapFirstPartyMember.clear(); }
	bool				AddFirstPartyStruct( CDNUserSession* pSession );
	PartyFirstStruct*	GetFirstPartyData(DWORD dwIndex);		// Index
	PartyFirstStruct*	GetFirstPartyData(INT64 biCharacterID);	// Character code
#endif

	virtual BYTE GetPvPMaxUser() const { return 0; }

#if defined (PRE_WORLDCOMBINE_PARTY)
	bool IsCloseTargetMap(int nMapIndex);
#endif

#if defined( PRE_PARTY_DB )
	int GetPartyMemberMax() { return m_PartyStructData.nPartyMaxCount; }
	BYTE GetPartyLimitLvMin() { return m_PartyStructData.cMinLevel; }
	int GetPartyUpkeepCount(){ return m_PartyStructData.iUpkeepCount; }
	void IncreasePartyUpkeepCount(){ ++m_PartyStructData.iUpkeepCount; }
	void SetInstanceID(TINSTANCEID InstanceID) { m_InstanceID = InstanceID; }
#else
	int GetPartyMemberMax() { return m_nMemberMax; }
	int GetPartyLimitLvMin() { return m_nUserLvMin; }
	int GetPartyLimitLvMax() { return m_nUserLvMax; }
	int GetPartyUpkeepCount(){ return m_nUpkeepCount; }
	void IncreasePartyUpkeepCount(){ ++m_nUpkeepCount; }
#endif // #if defined( PRE_PARTY_DB )	
	
#if defined( PRE_FATIGUE_DROPITEM_PENALTY )
	int GetFatigueDropRate();
#endif // #if defined( PRE_FATIGUE_DROPITEM_PENALTY )

	int GetPartyAvrLevel();	
	void GetLeaderSessionID(UINT &nSessionID);	
	void SetLeaderSession(UINT nSessionID);

	void SendRefreshParty(UINT nSessionID, TProfile * pProfile);

	bool SetInviteCharacterName(const WCHAR * pwszInvitedCharacterName);
	bool IsInviting();
	bool IsInvitingUser(const WCHAR * pwszName);
	void ResetInvite(int nRetCode, bool bNotice = true);
	int AdjustBreakintoUser(const WCHAR * pwszName, UINT nSessionID, int nRetCode, bool bNotice = true);

	//world
	CDnWorld * GetWorld() { return m_pWorld; }
	CTaskManager *GetTaskMng() { return m_pTaskMng; }

	//ProcessID
	DWORD GetProcessID() { return 0; }

	//Event
#if defined(PRE_ADD_WORLD_EVENT)
	TEvent * GetApplyEventType(int nType);	
	int GetEventType(int nType, char cClassID);
#else
	TEvent * GetApplyEvent(int nEventType1, int nEventType2 = _EVENT_2_NONE, int nEventType3 = _EVENT_3_NONE);	
#endif //#if defined(PRE_ADD_WORLD_EVENT)
	bool GetExtendDropRateIgnoreTime(int &nRate);	

	float GetEventExpWhenMonsterDie(float fExp, bool bFriendBonus, char cClassID, BYTE cJobID);
	void GetEventExpWhenStageClear(CDNUserSession * pSession, int nCalcVal, BYTE &cCount, sEventStageClearBonus * pBonus);		//3rd event type: bonus EXP on stage clear

	void GetAppliedEventValue(int &nDropRate, int &nUpkeepRate, int &nFriendBonusRate, bool * pUpkeepMax = NULL);
	void CheckFriendBonus(UINT nDelSessionID = 0);
#ifdef PRE_ADD_BEGINNERGUILD
	void CheckBeginnerGuildBonus();
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
	void UpdateAppliedEventValue();

	MasterSystem::CRewardSystem* GetMasterRewardSystem(){ return m_pMasterRewardSystem; }
	// Functions for values accumulated during dungeon progress
	void ResetCompleteExperience();
	void AddCompleteExperience( int nValue );
	int GetCompleteExperience();

	void ResetDungeonPlayTime();
	void AddDungeonPlayTime( DWORD dwValue );
	DWORD GetDungeonPlayTime();
#if defined( PRE_ADD_58761 )
	int GetDungeonGateID() { return m_nDungeonGateID; }
	void StartDungeonGateTime(int nGateNumber);
	void EndDungeonGateTime(int nGateNumber, bool bClearFlag = false);
	void NestDeathLog(CDNUserSession *pSession, int nMonsterID, int nSkillID, BYTE cCharacterJob, BYTE cCharacterLevel);
#endif

	// GetMethod
	CDnGameTask*			GetGameTask() const { return m_pGameTask; }
	CDnItemTask*			GetItemTask() const { return m_pItemTask; }
	virtual CPvPGameMode*	GetPvPGameMode() const { return NULL; }
	CDNRUDPGameServer * GetGameServer() const { return m_pGameServer; }
	int GetServerID() const;
#ifdef _USE_VOICECHAT
	void GetUserTalking(TTalkingInfo * pInfo, int &nCount);
	void TalkingUpdate(ULONG nCurTick);
	void SendRefreshVoiceInfo(UINT nOutAccountDBID = 0);
#endif

#if defined( PRE_PARTY_DB )
	TPARTYID		GetPartyIndex() const { return (m_cReqGameIDType == REQINFO_TYPE_PARTY) ? m_PartyStructData.PartyID : 0; }
#else
	TPARTYID		GetPartyIndex() const { return (m_cReqGameIDType == REQINFO_TYPE_PARTY) ? m_InstanceID : 0; }
#endif // #if defined( PRE_PARTY_DB )
	UINT			GetPvPIndex() const { return (m_cReqGameIDType == REQINFO_TYPE_PVP) ? static_cast<UINT>(m_InstanceID) : 0; }

	UINT GetFarmIndex() const { return (m_cReqGameIDType == REQINFO_TYPE_FARM) ? static_cast<UINT>(m_InstanceID) : 0; }
	virtual void QueryGetListField() {}
	virtual int GetFarmMaxUser() { return -1; }
	INT64			GetRoomLogIndex() { return m_i64RoomLogIndex; }
	INT64			GetPartyLogIndex() { return GetPartyIndex(); }
	BYTE GetGameType() { return m_cReqGameIDType; }
	TINSTANCEID GetInstanceID() { return m_InstanceID; }
	int	GetEventRoomIndex() const { return m_nEventRoomIndex; }

#if defined( PRE_PARTY_DB )
	TPARTYITEMLOOTRULE GetPartyItemLootRule() { return m_PartyStructData.LootRule; }
	TITEMRANK GetPartyItemLootRank() { return m_PartyStructData.LootItemRank; }
#else
	TPARTYITEMLOOTRULE GetPartyItemLootRule() { return m_ItemLootRule; }
	TITEMRANK GetPartyItemLootRank() { return m_ItemLootRank; }
#endif

	UINT GetCurrentItemLooterIdx();
	void CheckCurrentItemLooterIdx( PartyStruct *pEraseUser );
	//Send GameData
	void SendNextVillageInfo(const char * pIP, USHORT nPort, int nMapIndex, int nNextMapIndex, char cNextGateNo, short nRet, INT64 nItemSerial = 0);
	void SendPvPUserState(UINT nAccountDBID, UINT uiUserState);

	// Request item drop table drop
	DnDropItemHandle	RequestItemDropTable( const UINT uiTableID, EtVector3* pPos );
	void				AddDropItem( const DWORD dwUniqueID, DnDropItemHandle hDropItem );
	void				EraseDropItem( const DWORD dwUniqueID );
	DnDropItemHandle	FindDropItem( const DWORD dwUniqueID );

	int GetWorldSetID() { return m_iWorldID; }

	void RequestChangeMapFromTrigger( int iMapIndex, int iGateNo );
	void SendRefreshRebirthCoin(UINT nSessionID, BYTE cRebirthCoin, BYTE cPCBangRebirthCoin, short nCashRebirthCoin, BYTE cVIPRebirthCoin);
	//GuildWar
	virtual void QueryAddGuildWarFinalResults(UINT nGuildDBID, char CMatchResult) {}
#if defined( PRE_PARTY_DB )
	bool bIsPartyJobDice(){ return m_PartyStructData.iBitFlag&Party::BitFlag::JobDice ? true : false; }
	int GetPartyBitFlag() const { return m_PartyStructData.iBitFlag; }
	int GetPartyPassword() const { return m_PartyStructData.iPassword; }
	ePartyType GetPartyType() const { return m_PartyStructData.Type; }
	const WCHAR* GetPartyName() const { return m_PartyStructData.wszPartyName; }
	int GetPartyTargetMapID() const { return m_PartyStructData.iTargetMapIndex; }
	TDUNGEONDIFFICULTY GetPartyDifficulty() const { return m_PartyStructData.TargetMapDifficulty; }
	const Party::Data&	GetPartyStructData() { return m_PartyStructData; }
	void SetPartyRoomID( int RoomID ){ m_PartyStructData.iRoomID = RoomID; }
	void SetPartyID( TPARTYID PartyID );
	void AddPartyDB( CDNUserSession* pBreakIntoSession );	
	void AddPartyMemberDB( CDNUserSession* pSession );	
	void DelPartyDB();
	void DelPartyMemberDB( CDNUserSession *pSession );
#if defined( PRE_WORLDCOMBINE_PARTY )
	int GetWorldPartyPrimaryIndex() const { return m_PartyStructData.nPrimaryIndex; }
	bool bIsWorldCombineParty();
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_ADD_NEWCOMEBACK )
	void SetComebackParty( bool bFlag ) { m_PartyStructData.bCheckComeBackParty = bFlag; }
	bool bIsComebackParty() { return m_PartyStructData.bCheckComeBackParty; }	
#endif
#else
	bool bIsPartyJobDice(){ return m_cIsJobDice ? true : false; }
	BYTE GetPartyJobDice(){ return m_cIsJobDice; }
	const WCHAR* GetPartyPassword() const { return m_wszPartyPass; }
	const WCHAR* GetPartyName() const { return m_wszPartyName; }
	int GetPartyTargetMapID() const { return m_nTargetMapIdx; }
	TDUNGEONDIFFICULTY GetPartyDifficulty() const { return m_PartyDifficulty; }
#endif // #if defined( PRE_PARTY_DB )
	
	int GetPartyChannelID() const { return m_nChannelID; }
	
	void SetPartyKickedMemberList( UINT* pArr )
	{
		memcpy( m_nKickedMemberList, pArr, sizeof(m_nKickedMemberList) );
	}
	UINT GetPartyKickedAccountID( int idx )
	{ 
		if( idx < 0 || idx >= _countof(m_nKickedMemberList) )
			return 0;
		return m_nKickedMemberList[idx]; 
	}
	void SetPartyKickedAccountID( int idx, UINT uiAccountID )
	{ 
		if( idx < 0 || idx >= _countof(m_nKickedMemberList) )
			return;
		m_nKickedMemberList[idx] = uiAccountID; 
	}
	void CopyPartyKickedMemberList( UINT* pArr )
	{
		memcpy( pArr, m_nKickedMemberList, sizeof(m_nKickedMemberList) );
	}

protected :
	//PeerManagement
#ifdef _USE_PEERCONNECT
	void ConnectPeerRequest();
#endif

	//Inituser
	virtual void OnInitGameRoomUser();
	virtual void OnGameStatePlay(ULONG iCurTick);

#ifdef _USE_VOICECHAT
	void OnInitVoice();
#endif

	//Send GameData
	void SendConnectedResult();
	void SendEquipData( CDNUserSession* pBreakIntoSession );
	void SendSkillData( CDNUserSession* pBreakIntoSession );
	void SendEtcData( CDNUserSession *pBreakIntoSession );
	void SendSecondAuthInfo( CDNUserSession* pBreakIntoSession );
	void SendMaxLevelCharacterCount( CDNUserSession* pBreakIntoSession );
	void SendCompleteGameReady( CDNUserSession* pBreakIntoSession );
	void SendGuildData( CDNUserSession* pBreakIntoSession );
	bool IsItemLootUserValid(DWORD userIdx);
	void SendMasterSystemSimpleInfo( CDNUserSession* pBreakIntoSession );
	void SendMasterSystemCountInfo( CDNUserSession* pBreakIntoSession );
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	void SendReputationList( CDNUserSession* pBreakIntoSession );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#if defined (PRE_ADD_BESTFRIEND)
	void SendBestFriendData( CDNUserSession* pBreakIntoSession );
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void SendPartyEffectSkillItemData( CDNUserSession* pBreakIntoSession );	//	Send party member effect skill item info to drop-in user	
#endif
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	void SendTotalLevelSkillInfo( CDNUserSession* pBreakIntoSession );
#endif

	//
	//bool _bSendSync();
	//bool _bSendSyncEquipData();
	//bool _bSendSyncSkillData();


	unsigned long m_iPivotTick;			//Time when the game room was created
	unsigned long m_iNextGameState;		//Time of game room state transition

	int m_GameState;							//roomstate

	CDNRUDPGameServer * m_pGameServer;
	CRandom *m_pRandom;
	CDnWorld * m_pWorld;
	CEtCollisionMng *m_pCollisionMng;
	CTaskManager * m_pTaskMng;
	CDnGameTask * m_pGameTask;
	CDnPartyTask * m_pPartyTask;
	CDnItemTask * m_pItemTask;
	CDnSkillTask * m_pSkillTask;
	CDnGuildTask * m_pGuildTask;
	CDnChatTask * m_pChatTask;
		
	//UserConteiner
	DNVector(PartyStruct)			m_VecMember;
	std::map<INT64,CDNUserSession*>	m_mCharDBIDMember;
	std::list<CDNUserSession*>		m_UserList;
	std::list<CDNUserSession*>		m_BreakIntoUserList;
	std::vector<unsigned int>		m_DeleteList;
#if defined( PRE_FIX_49129 )	
	std::map<INT64, PartyFirstStruct>	m_MapFirstPartyMember;	// Index, first party info
#endif

	//EventConteiner
	DNVector(TEvent) m_EventList;

	//PeerManager
	CDNPeerManager m_PeerManager;

	//ListenerList
	std::vector <CGameListener*> m_GameListener;
	MasterSystem::CRewardSystem* m_pMasterRewardSystem;
	// Values that must be accumulated during dungeon progress.
	int m_nCompleteExperience;
	DWORD m_dwDungeonPlayerTime;

	int m_nCurItemLooterInOrder;
	int m_iHackPenalty;
	INT64 m_i64RoomLogIndex;

	std::list<CDNUserSession*>::iterator m_ItorSession;
	std::map<INT64,PartyBackUpStruct>	m_mPartyBackUp;
#if defined( PRE_ADD_36870 )
	int m_iDungeonClearRound;
#endif // #if defined( PRE_ADD_36870 )

	int m_nDungeonGateID;	// Gate number
#if defined(PRE_ADD_ACTIVEMISSION)
	bool m_bIsFirstInitializeDungeon;
#endif

#if defined(PRE_ADD_CP_RANK)
	TStageClearBest m_sLegendClearBest;
	TStageClearBest m_sMonthlyClearBest;
#endif //#if defined(PRE_ADD_CP_RANK)

#if defined( PRE_PVP_GAMBLEROOM )
	INT64 m_nGambleRoomDBID;
#endif

protected:

	void					_AddPacketQueue( CDNUserSession* pSession, const DWORD dwUniqueID, const BYTE cSubCmd, const BYTE* pBuffer, const int iSize, const int iPrior );
	void					InitRoomState( CDNUserSession* pBreakIntoSession );											//Match each room to specific conditions~
	void					_BreakIntoProcess();

	//Event
	void InitEvent();

public:
	
	virtual void			OnSendPartyMemberInfo( CDNUserSession* pBreakIntoSession );
	virtual void			OnSendTeamData( CDNUserSession* pBreakIntoSession ){}
	virtual void			OnSync2Sync( CDNUserSession* pBreakIntoSession ){}

	virtual void OnDBMessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen){}
	virtual void OnDBRecvDarkLair(int nSubCmd, char *pData){}

	void SetSync2SyncStateTemplateMethod( const int iCurTick, CDNUserSession* pBreakIntoSession=NULL);
	void SetSync2PvPModeStateTemplateMethod(const int iCurTick);
	void SyncMonster( CDNUserSession* pBreakIntoGameSession );
	void SyncNpc( CDNUserSession* pBreakIntoGameSession );
	void SyncProp( CDNUserSession* pBreakIntoGameSession );
	void SyncPlayer( CDNUserSession* pBreakIntoGameSession );
	void SyncGate( CDNUserSession* pBreakIntoGameSession );
	
	void CheckDiePlayer( CDNUserSession* pBreakIntoGameSession );

	virtual void			OnSetLoad2SyncState( const int iCurTick );
	virtual void			OnSetPlayState();

	void GetBreakIntoUserTeamCount( int& iATeam, int& iBTeam );

	//
	bool					bIsExistBreakIntoUser(){ return !m_BreakIntoUserList.empty(); }
	int						GetBreakIntoUserCount(){ return static_cast<int>(m_BreakIntoUserList.size()); }
	const std::list<CDNUserSession*>&	GetBreakIntoUserList(){ return m_BreakIntoUserList; }
	virtual bool			bIsBreakIntoUser( CDNUserSession* pGameSession );
	virtual void			OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage ){}
	virtual void			OnRebirth( DnActorHandle hActor ){}
	virtual void			OnDie( DnActorHandle hActor, DnActorHandle hHitter );
	virtual void			OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo ){}
	virtual void			OnLeaveUser( const UINT uiSessionID ){}

	virtual void			OnFinishProcessDie( DnActorHandle hActor ){}
	virtual void			OnDelPartyMember( UINT iDelMemberSessionID, char cKickKind);
	virtual bool			bIsPvPRoom(){ return false; }
	virtual bool			bIsIgnorePrefixSkill();
	virtual bool			bIsLadderRoom(){ return false; }
	virtual bool			bIsFarmRoom(){ return false; }
	virtual void SetFarmActivation(bool bActivation) {}
	virtual bool			bIsZombieMode(){ return false; }
	virtual bool            bIsAllKillMode(){ return false; }
#if defined( PRE_ADD_PVP_TOURNAMENT)
	virtual bool			bIsTournamentMode() { return false;}
#endif
	virtual bool bIsGuildWarMode() { return false; }
	virtual bool bIsGuildWarSystem() { return false; }
	virtual bool bIsOccupationMode() { return false; }
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	virtual bool bIsComboExerciseMode() { return false; }
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

	virtual void OnSendPvPLobby() {}

	virtual void			UpdateCharacterName(MAChangeCharacterName* pPacket) {}
	virtual bool			bIsDLRoom(){ return false; }
	virtual bool			bIsLevelRegulation(){ return false; }

	void					SendBreakIntoUser( CDNUserSession* pBreakIntoSession );
	void					SendHPSP( CDNUserSession* pBreakIntoSession );
	void					SendBattleMode( CDNUserSession* pBreakIntoSession );
	void					SendAddStateEffect( CDNUserSession* pBreakIntoGameSession );
	void					SendPosition( CDNUserSession* pBreakIntoGameSession );
	void					SendDropItemList( CDNUserSession* pBreakIntoGameSession );

	virtual bool			InitializePvPGameMode( const MAGAPVP_GAMEMODE* pPacket ){ return false; }

	virtual void			OnRequestSyncStartMsg( CDNUserSession* pGameSession );
	virtual void			GetPvPSN( INT64& biMain, int& iSub ){ biMain = 0; iSub = 0; }
	virtual bool			bIsPvPStart(){ return false; }
	virtual void			OnSuccessBreakInto( CDNUserSession* pGameSession );
	virtual void			OnSuccessBreakInto( std::list<CDNUserSession*>::iterator& itor );

	void					AddRequestGetReversionItem(const TItem& itemInfo, DnDropItemHandle hDropItem);
	bool					IsEnableAddRequestGetReversionItem(DnDropItemHandle hDropItem) const;

	void					SendChatRoomInfo (CDNUserSession* pBreakIntoGameSession);
	void					BroadcastChatRoomView(CDNUserSession* pSender, TChatRoomView& ChatRoomView);
	void					BroadcastChatRoomEnterUser(CDNUserSession* pSender, CDNChatRoom* pChatRoom);
	void					BroadcastChatRoomLeaveUser(CDNUserSession* pSender, int nErrorCode);

	void	SetStageStartLogFlag( bool bFlag ){ m_bStageStargLog = bFlag; }
	bool	GetStageStartLogFlag(){ return m_bStageStargLog; }
	void	SetStageEndLogFlag( bool bFlag ){ m_bStageEndLog = bFlag; }
	bool	GetStageEndLogFlag(){ return m_bStageEndLog; }
	void	SetGoToVillageFlag( bool bFlag ){ m_bGotoVillageFlag = bFlag; }
	bool	GetGoToVillageFlag(){ return m_bGotoVillageFlag; }

	CDNDBConnection*	GetDBConnection(){ return m_pDBConnection; }
	BYTE				GetDBThreadID(){ return m_cDBThreadID; }

	void	AddHackPenalty( int iPenalty )
	{
		if( m_iHackPenalty + iPenalty >= HackPenanty::Common::MaxRate )
			m_iHackPenalty = HackPenanty::Common::MaxRate;
	}
	int		GetHackPenalty(){ return m_iHackPenalty; }
	float	GetHackRate(){ return m_iHackPenalty/static_cast<float>(HackPenanty::Common::MaxRate); }

	bool bIsFinalizeRoom(){ return m_bFinalizeRoom; }
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void SendCompleteBreakIntoUser( CDNUserSession* pBreakIntoSession );
#endif
#if defined( PRE_WORLDCOMBINE_PARTY )
	bool CheckDestroyWorldCombineParty();	
	void ApplyWorldCombinePartyBuff( CDNUserSession* pBreakIntoSession );	
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

	void AddLastPartyDungeonInfo( CDNUserSession* pSession );
	void DelLastPartyDungeonInfo( CDNUserSession* pSession );
	void AddBackupPartyInfo( CDNUserSession* pSession );
	void DelBackupPartyInfo();
	void DelBackupPartyInfo( INT64 biCharacterDBID );
	bool bIsBackupPartyInfo( INT64 biCharacterDBID );
	bool GetBackupPartyInfo( INT64 biCharacterDBID, PartyBackUpStruct& BackupInfo );

#if defined( PRE_ADD_36870 )
	void SetDungeonClearRound( int iRound ){ m_iDungeonClearRound = iRound; }
	int GetDungeonClearRound() const { return m_iDungeonClearRound; }
#endif // #if defined( PRE_ADD_36870 )

#if defined( PRE_ADD_DIRECTNBUFF )
	bool bIsDirectPartyBuff() { return m_DirectPartyBuffItemList.empty() ? false:true; }
	void SetDirectPartyBuff( int nItemID ) {m_DirectPartyBuffItemList.push_back(nItemID);}	
	void ApplyDirectPartyBuff( bool bUseItem = false );
	void RemoveDirectPartyBuff( int nItemID, bool bAll = false );
	void SendDirectPartyBuffMsg(CDNUserSession * pSession=NULL);
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
	void DelPartyMemberAppellation( int nAppellationID );
	bool CheckPartyMemberAppellation( int nAppellationID = 0 );
	void ApplyJoinMemberAppellation( CDNUserSession * pSession );
#endif

#if defined( PRE_FIX_BUFFITEM )
	void CheckEffectSkillItemData();
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
	void AddDBWorldPvPRoom( TWorldPvPMissionRoom *pMissionRoom, UINT nGMAccountDBID );
	bool bIsWorldPvPRoom(){ return (m_eWorldReqRoom > WorldPvPMissionRoom::Common::NoneType) ? true:false; }
	WorldPvPMissionRoom::Common::eReqType GetWorldPvPRoomReqType(){ return m_eWorldReqRoom; }	
	bool bIsWorldPvPRoomStart() { return m_bWorldPvPRoomStart;}
	void SetWorldPvPRoomStart(bool bFlag);
	void SetWorldPvPRoomDBIndex( int nWorldPvPRoomDBIndex ) { m_nWorldPvPRoomDBIndex = nWorldPvPRoomDBIndex; }
	int GetWorldPvPRoomDBIndex() { return m_nWorldPvPRoomDBIndex; }	
	void AddDBWorldPvPRoomMember( CDNUserSession* pSession );
	void DelDBWorldPvPRoomMember( CDNUserSession* pSession );
	DWORD GetBreakIntoUserTeamCount( int nTeam );
	bool CheckWorldPvPRoomBreakInto( MAGABreakIntoRoom* pPacket );
	void SetWorldPvPRoomData(TWorldPvPRoomDBData Data, UINT nGMAccountDBID ) { m_tPvPRoomDBData = Data; m_nCreateGMAccountDBID = nGMAccountDBID; }
	UINT GetWorldPvPRoomCreateGMAccountDBID() { return m_nCreateGMAccountDBID; }
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
	void InitActiveMission();
	void SetIsFirstInitializeDungeon(bool bFlag) { m_bIsFirstInitializeDungeon = bFlag; }
	bool GetIsFirstInitializeDungeon() { return m_bIsFirstInitializeDungeon; }
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	void SetAlteiaWorld(bool bFlag);
	bool bIsAlteiaWorld() { return m_bAlteiaWorld; }
	void SetAlteiaWorldMap( bool bFlag ) { m_bAlteiaWorldMap = bFlag; }
	bool bIsAlteiaWorldmap() { return m_bAlteiaWorldMap; }
	DWORD GetAlteiaPlayTime();
	void ResetAlteiaWorldmap();
#endif

#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	void OnSelectRandomMap( int nRootMapIndex, int nRandomMapIndex );
#endif
#if defined(PRE_ADD_CP_RANK)
	void SetAbyssStageClearBest(const TAGetStageClearBest* pA);	
	const TStageClearBest* GetLegendClearBest() { return &m_sLegendClearBest;}
	const TStageClearBest* GetMonthlyClearBest() { return &m_sMonthlyClearBest;}
#endif //#if defined(PRE_ADD_CP_RANK)

#if defined( PRE_PVP_GAMBLEROOM )
	void CreateGambleRoom( BYTE cGambleType, int nGamblePrice );
	void SetGambleRoomDBID( INT64 nGambleRoomDBID ) { m_nGambleRoomDBID = nGambleRoomDBID; }
	INT64 GetGambleRoomDBID() { return m_nGambleRoomDBID; }
#endif

	inline int rand() { return m_mtRandom.rand() ;};
};


class IBoostPoolDNGameRoom : public CDNGameRoom, public TBoostMemoryPool<IBoostPoolDNGameRoom>
{
public :
	IBoostPoolDNGameRoom(CDNRUDPGameServer * pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket )
		:CDNGameRoom(pServer,iRoomID,pPacket){}
	virtual ~IBoostPoolDNGameRoom(){}
};

#if defined( PRE_TRIGGER_TEST )

class CEtTriggerObject;

class CDnTestTriggerGameRoom : public CDNGameRoom
{
public:
	CDnTestTriggerGameRoom( CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket ):CDNGameRoom(pServer,iRoomID,pPacket){}

	void FinishTrigger( CEtTriggerObject* pObject );
	void SetRoomState( eGameRoomState State );
	virtual void OnGameStatePlay(ULONG iCurTick);

	std::vector<std::pair<CEtTriggerObject*,double>> m_vFinish;
};

#endif // #if defined( PRE_TRIGGER_TEST )
