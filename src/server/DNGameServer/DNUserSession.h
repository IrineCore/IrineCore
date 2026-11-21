
#pragma once

#include "DNUserSendManager.h"
#include "RUDPCommonDef.h"
#include "DNGameServerManager.h"
#include "DNRUDPGameServer.h"
#include "DNUserBase.h"
#include "MemPool.h"

class CDNRUDPGameServer;
class CDNGameRoom;
class CDNTcpConnection;
class CPvPGameMode;
class CDnPlayerActor;

#define PACKET_QUEUE_SIZE 220

struct _STORE_PACKET
{
	int iMainCmd;
	int iSubCmd;
	unsigned short iSize;
	char * pMsg;
};

#ifdef PRE_ADD_PACKETSIZE_CHECKER
struct _ACTOR_SENDINFO
{
	UINT nSendCount;
	UINT nSendSize;
};

struct _SEND_DEBUGINFO
{
	unsigned char _DebugMainCmd;
	unsigned char _DebugSubCmd;
	unsigned long _DebugTick;
	unsigned long _DebugSize;
};
//
//struct _ACTOR_DEBUGINFO
//{
//	DWORD _dwUniqueID;
//	unsigned char _ActorSubCmd;
//	unsigned char _PacketBroadFlag;		//0 S->C 1 C->S->C(BroadCast)
//};
#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER

class CDNUserSession : public CDNUserBase
{
protected:
	CDNRUDPGameServer * m_pGameServer;
	CDNGameRoom * m_pGameRoom;
	CDNTcpConnection * m_pTcpConnection;

	int m_iState;						//Session state value: loading, connected, disconnected, etc.
	int m_iNetID;						//Management ID issued at RUDPSocketFrame layer
	bool m_bIntededDisconnected;
	bool m_bRudpDisconnected;
	unsigned long m_nRudpDisconnectedTick;
	bool m_bPvPGameModeFinish;
	bool m_bTutorial;		// When arriving directly without going through village
	bool m_bHide;	// GM invisible state(?)
	char m_cKickKind;	//
	bool m_bAbsent;		//Whether in AFK state
	BYTE m_cPacketSeq;		//udp tcp seq var (UDP sequencing is matched only for ordered reliable; i.e., order between reliable UDP and TCP)
	INT64	m_biPickUpCoin;	// Game coin delta

	int	m_iTeam;

	char m_szNextVillageIP[IPLENMAX];
	USHORT m_nNextVillagePort;

	DnActorHandle m_hActor;		//Actor handle

	BYTE m_pPacketQueueBuffer[3][PACKET_QUEUE_SIZE];
	int m_nPacketQueueOffset[3];
	int m_nPacketQueueCount[3];
	std::vector<DWORD> m_dwVecPacketQueueUniqueList[3];
	
	bool m_bStageAbortLog;	// If true, write log during FinalUser (090327)
	bool m_bGMTrace;
	bool m_bIsAIBot;

	int	m_iDecreaseFatigue;	// Current Stage fatigue reduction value
	bool m_bLoadingComplete;
	int m_iTotalMasterDecreaseFatigue;	// Accumulate and hold the master's unspent fatigue.

	DNVector(_STORE_PACKET) m_pStoredPacketList;
	void StorePacket(int nMainCmd, int nSubCmd, const char * pData, int nLen);
	void _SendReconnectLogin();

	std::vector <TUserPromotionData> m_vPromotionData;
	void SetPromotionData(int nPromotionType, int nValue);

	int m_nPartyMemberIndex;
	bool m_bPvPFatigueOption;
	int m_uiPvPUserState;

	bool m_bOuted;

	DWORD					m_dwCheckUdpPing;
	std::pair<DWORD,UINT>	m_pairUdpPing;	// <Tick/Count>

	DWORD m_dwKeepOperationID;

	std::map<int, TGuildRewardItem> m_mGuildRewardItem;		// Guild effect reward applied item
	int m_nPeriodeExpItemRate;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	int m_nGoldBoxCheatID;						//Specify gold box item
#endif
#if defined(PRE_ADD_ACTIVEMISSION)
	int m_nActiveMissionCheatID;	//Cheat to set active mission ID
#endif
#if defined( PRE_ADD_DIRECTNBUFF )
	bool m_bDirectPartyBuffItem;
	std::list<int> m_DirectPartyBuffItemList;
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	int m_nRewardBoxNumber;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)

#ifdef PRE_MOD_INDUCE_TCPCONNECT
	DWORD m_dwSendReqTCPConnect;
	bool m_bRecvReqTCPConnectMsg;
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

#if defined( PRE_ADD_GUILDREWARDITEM )
	std::list<int> m_GuildRewardbuffItemList;	// Item ID
#endif // #if defined( PRE_ADD_GUILDREWARDITEM )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	int m_nAlteiaWorldPosition;	
	BYTE m_cDailyPlayCount;
	DWORD m_dwAlteiaWorldMoveNextMapTick;
	bool m_bCanDice;
#endif

#ifdef PRE_ADD_PACKETSIZE_CHECKER
	unsigned long m_nSendCheckTick;
	unsigned long m_nSendCheckSize;
	unsigned long m_nSendCheckCount;
	_SEND_DEBUGINFO m_SendDebugInfo[128];
	int m_nSendDebugInfoCount;
	//_ACTOR_DEBUGINFO m_SendActorDebugInfo[32];
	//int m_nSendActorDebugInfoCount;

	bool m_bActorDebugSended;
	std::vector<_ACTOR_SENDINFO> m_vActorSendInfo;
#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER

public:
	std::pair<INT64,INT64> m_BackupEquipInfo;	// OriginalSerial/ReplacementSerial
	void BackUpEquipInfo();

public :
	CDNUserSession(unsigned int iUID, CDNRUDPGameServer * pServer, CDNGameRoom * pRoom);
	virtual ~CDNUserSession();

	int GetNetID() { return m_iNetID; }
	virtual void SetUserJob(BYTE cJob);

	void SetSessionState(int nState);
	int GetState() { return m_iState; }
	const char* GetIp();
	const WCHAR * GetIpW();
	virtual int GetRoomID();
	int GetChannelAttribute() { return 0; }	// To match village (for shared source - unused)
	int GetServerID();

	virtual TPARTYID GetPartyID() override;
	bool GetAbsent() { return m_bAbsent; }
	void SetAbsent(bool bAbsent) { m_bAbsent = bAbsent; }
	void SetGMTrace( bool bGMTrace ){ m_bGMTrace = bGMTrace; }
	bool bIsGMTrace(){ return m_bGMTrace; }
	bool bIsAIBot() const { return m_bIsAIBot; }
	void SetAIBot(bool bBot) { m_bIsAIBot = bBot; }
	void DecreaseFatigue( bool bFinalUser=false );
	int	 GetDecreaseFatigue(){ return m_iDecreaseFatigue; }
	void SetDecreaseFatigue( int iValue ){ m_iDecreaseFatigue = iValue; }
	int GetTotalMasterDecreaseFatigue(){ return m_iTotalMasterDecreaseFatigue; }
	void ClearTotalMasterDecreaseFatigue(){ m_iTotalMasterDecreaseFatigue = 0; }
	void SetLoadingComplete( bool bFlag ){ m_bLoadingComplete = bFlag; }
	bool bIsLoadingComplete(){ return m_bLoadingComplete; }
	int GetPromotionValue(int nType);


	void AddPickUpCoin( INT64 biCoin ){ m_biPickUpCoin += biCoin; }
	void DelPickUpCoin( INT64 biCoin ){ m_biPickUpCoin -= biCoin; }
	void SelPickUpCoin( INT64 biCoin ){ m_biPickUpCoin = biCoin; }
	INT64 GetPickUpCoin(){ return m_biPickUpCoin; }

	void SetOutedMember() { m_bOuted = true; }
	bool GetOutedMember() { return m_bOuted; }

	void SetPartyMemberIndex(int nIndex) { m_nPartyMemberIndex = nIndex; }
	int GetPartyMemberIndex() { return m_nPartyMemberIndex; }

	DWORD GetKeepOperationID() { return m_dwKeepOperationID; }
	void SetKeepOperationID(DWORD dwUniqueID);

	int GetPeriodExpItemRate() { return m_nPeriodeExpItemRate; }
	void SetPeriodExpItemRate();

	bool IsTutorial() { return m_bTutorial; }

	//Connect & Disconnect with TCP
	bool IsConnected();
	bool RUDPConnected(int iNetID);
	bool TCPConnected(CDNTcpConnection * pCon);
	inline CDNTcpConnection * GetTcpConnection() { return m_pTcpConnection; }
	void SetTcpConnection( CDNTcpConnection* pCon ){ m_pTcpConnection = pCon; }
	
	void RudpDisConnected();
	void IntendedDisconnected();
	void DetachConnection(wchar_t *pwszIdent);		//Disconnect all!
#ifdef PRE_MOD_INDUCE_TCPCONNECT
	void SendReqTCPConnect(DWORD dwCurTick);
	void RecvReqTCPConnect();
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

	//Update
	virtual void DoUpdate( DWORD dwCurTick );

	//Message Parse
	void MasterMessageProcess(int iMainCmd, int iSubCmd, const void * pData, int iLen);

	virtual void OnDBRecvAuth(int nSubCmd, char *pData);
	virtual void OnDBRecvCharInfo(int nSubCmd, char *pData);
	virtual void OnDBRecvEtc(int nSubCmd, char *pData);
	virtual void OnDBRecvQuest(int nSubCmd, char *pData);
	virtual void OnDBRecvMission(int nSubCmd, char *pData);
	virtual void OnDBRecvSkill(int nSubCmd, char *pData);
	virtual void OnDBRecvFriend(int nSubCmd, char *pData);
	virtual void OnDBRecvIsolate(int nSubCmd, char *pData);
	virtual void OnDBRecvPvP(int nSubCmd, char *pData);
	virtual void OnDBRecvDarkLair(int nSubCmd, char *pData);
	virtual void OnDBRecvGuild(int nSubCmd, char *pData);
	virtual void OnDBRecvMail(int nSubCmd, char *pData);
	virtual void OnDBRecvMarket(int nSubCmd, char *pData);
	virtual void OnDBRecvItem(int nSubCmd, char *pData);
	virtual void OnDBRecvCash(int nSubCmd, char *pData);
	virtual void OnDBRecvMasterSystem(int nSubCmd, char *pData);
#if defined( PRE_PARTY_DB )
	virtual void OnDBRecvParty( int nSubCmd, char* pData );
#endif // #if defined( PRE_PARTY_DB )
#if defined (PRE_ADD_BESTFRIEND)
	virtual void OnDBRecvBestFriend(int nSubCmd, char* pData);
#endif // #if defined (PRE_ADD_BESTFRIEND)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	virtual void OnDBRecvAlteiaWorld( int nSubCmd, char* pData );
#endif

	virtual int OnRecvCharMessage(int iSubCmd, char * pData, int iLen);
	virtual int OnRecvTradeMessage(int iSubCmd, char * pData, int iLen);
	virtual int OnRecvQuestMessage(int iSubCmd, char * pData, int iLen);
	virtual int OnRecvSystemMessage(int iSubCmd, char * pData, int iLen);
	virtual int OnRecvFriendMessage(int nSubCmd, char * pData, int nLen);
	virtual int OnRecvIsolateMessage(int nSubCmd, char * pData, int nLen);
	virtual int OnRecvGameOptionMessage(int nSubCmd, char * pData, int nLen);
	virtual int OnRecvRadioMessage(int nSubCmd, char * pData, int nLen);
	virtual int OnRecvEtcMessage(int nSubCmd, char * pData, int nLen);
#ifdef _USE_VOICECHAT
	virtual int OnRecvVoiceChatMessage(int nSubCmd, char * pData, int nLen);
#endif
#if defined( PRE_ADD_SECONDARY_SKILL )
	virtual int OnRecvSecondarySkillMessage( int nSubCmd, char* pData, int nLen );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	virtual int OnRecvChatRoomMessage(int nSubCmd, char *pData, int nLen);
	virtual int OnRecvPvPMessage(int nSubCmd, char * pData, int nLen);

#if defined( PRE_PRIVATECHAT_CHANNEL )
	virtual int OnRecvPrivateChatChannelMessage( int nSubCmd, char* pData, int nLen );
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	virtual int OnRecvWorldAlteiaMessage( int nSubCmd, char* pData, int nLen );
#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

	//send
	void SendPacket(int iMainCmd, int iSubCmd, const void * pMsg, int iLen, int iPrior);		//udp direct send
	int AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen);

	//initialize & finalize User
	virtual void PreInitializeUser(WCHAR * wszAccountName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bTutorial, bool bAdult, char cPCBangGrade, TMemberVoiceInfo * pInfo = NULL);
	void InitialUser();
	void FinalUser();
	void InitVoice();

	//Room Seq Level
	void SendSeqLevel(BYTE cSeqLevel);

	//
	void SendBackToVillage( bool bIsPartyOut, char cKickKind = -1, const int iMapIndex=-1 );
	void SendPvPGameToPvPLobby();
	void SendConnectedResult();
	void SendUserInfo();	// Items sent in bulk upon initial connection

	//UserData
	virtual bool LoadUserData(TASelectCharacter *pSelect);
	virtual bool SaveUserData();

	void ChangeServerUserData();
	void LastUpdateUserData();
	void ChangeStageUserData(int nAddPetExp = 0);	// Pet EXP is only needed at stage clear time
	// void UpdateAttributePart(); // Save on zone transition
	// Move remaining real-time save-like things to g_pDBConnectionManager (to share between village and game). Implement functions there!

	// Fatigue
	virtual void SetFatigue(int nFatigue, int nWeeklyFatigue, int nPCBangFatigue, int nEventFatigue, int nVIPFatigue, bool bDBSave = true);
	virtual void DecreaseFatigue(int nGap);
	virtual void IncreaseFatigue(int nGap);
	virtual void SetDefaultMaxFatigue(bool bSend = true);
	virtual void SetDefaultMaxWeeklyFatigue(bool bSend = true);
	virtual void SetDefaultMaxRebirthCoin(bool bSend = true);
	virtual void SetHide(bool bHide);

	virtual void RefreshGuildSelfView();

	unsigned int GetSessionID() { return m_nSessionID; }

	//Actor Handle
	void SetActorHandle(DnActorHandle hActor) { m_hActor = hActor; }
	DnActorHandle GetActorHandle() { return m_hActor; }
	CDnPlayerActor* GetPlayerActor();

	//Get
	inline CDNGameRoom * GetGameRoom() { return m_pGameRoom; }

#if !defined(PRE_DELETE_DUNGEONCLEAR)
	// Dungeon level related functions
	bool IsNeedDungeonLevelSave( int nMapTableID );

	//int GetDungeonLevelIndex( int nMapTableID, bool *bExist );
	bool CheckDungeonEnterLevel( int nMapTableID );
	void CalcDungeonEnterLevel(int nMapTableID, char &cOpenHard, char &cOpenVeryHard);

	bool UpdateDungenEnterLevel( int nCurrentStageConstructionLevel, int nRank );
	void RefreshDungeonEnterLevel();
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

	//mapidx
	void SetNextVillageData(const char * pIP, USHORT nPort, int nMapIndex, int nNextMapIndex, char cNextGateNo);	// Setting map IDs before returning to village
	virtual void SetMapIndex(int nMapIndex);	// Save only map when transitioning between stages

	inline int	GetTeam(){ return m_iTeam; }
	void SetTeam( int iTeam ){ m_iTeam = iTeam; }

	inline bool IsRudpDisconnected() { return m_bRudpDisconnected; }
	inline ULONG GetRudpDisconnectedTick() { return m_nRudpDisconnectedTick; }

	void AddPacketQueue( const DWORD dwUniqueID, const BYTE cSubCmd, const BYTE *pBuffer, const int nSize, const int nPrior );
	void FlushPacketQueue( int nPrior = -1 );
	void FlushStoredPacket();
	void CheckFlushQueue( int nPrior, DWORD dwUniqueID, BYTE cSubCmd );

	CDNUserSession* FindUserSession(UINT nSessionID);	// Find session by session ID	
	// 
	void CopyDefaultParts( int* pDest );

	// Send
	void SendPvPModeStartTick( const UINT uiStartTick );
	void SendPvPModeFinish( const CPvPGameMode* pPvPMode );
	void SendPvPRoundStart();
#if defined(PRE_ADD_PVP_TOURNAMENT)
	void SendPvPRoundFinish( const UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason, const CPvPGameMode* pPvPMode, const UINT uiWInSessionID, const char cTournamentStep );
#else
	void SendPvPRoundFinish( const UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason, const CPvPGameMode* pPvPMode );
#endif
	void SendSuccessBreakInto( const UINT uiSessionID );
	void SendPvPAddPoint( UINT uiSessionID, const UINT uiScoreType, const UINT uiAddPoint );
	void SendPvPSelectCaptain( UINT uiSessionID, WCHAR* wszName );
	void SendPvPSelectZombie( std::map<DWORD,DnActorHandle>& mZombie );
	void SendPvPSelectZombie( DnActorHandle hActor, int nTableID, bool bZombie, bool bRemoveStateBlow, int iScale );
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	void SendStageClearBonusRewardResult(int nResult);
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	virtual bool BreakIntoProcess(){ return true; }
	// PvP
	void	SetPvPGameModeFinish( const bool bFlag ){ m_bPvPGameModeFinish = bFlag; }
	bool	GetPvPGameModeFinish(){ return m_bPvPGameModeFinish; }
	void	SetPvPFatigueOption( const bool bFatigue ) { m_bPvPFatigueOption = bFatigue; }
	bool	GetPvPFatigueOption() { return m_bPvPFatigueOption; }
	void SetPvPUserState(UINT uiState) { m_uiPvPUserState = uiState; }
	UINT GetPvPUserState() { return m_uiPvPUserState; }
#if defined(PRE_MOD_PVP_LADDER_XP)
	void UpdatePvPLevel();
#endif

	int CmdAbandonStage( const bool bDecreaseDurability, bool bPartyOut, bool bIntenedDisconnect, const int iMapIndex=-1 );

	// Cheat
	void BroadcastingEffect(char cType, char cState);

	void QueryGetMasterSystemCountInfo( bool bClientSend, CDNDBConnection* pDBCon=NULL, BYTE cThreaID=0 );
	void	VerifyValidMap(int nNextMapIndex);

	virtual bool bIsBreakIntoSession(){ return false; }

	virtual bool bIsCheckPing(){ return GetState() == SESSION_STATE_GAME_PLAY; }
	virtual void WritePingLog();
	void RecvUdpPing( DWORD dwTick );

	static void ApplySourceEffect(CDnPlayerActor* pPlayerActor, const TSourceData& source, bool bOnInitialize = false);

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	static void ApplyEffectSkill( CDnPlayerActor* pPlayerActor, std::vector<TEffectSkillData>& vEffectSkill, bool bOnInitialize = false , bool bIsPvpGameRoom = false );	// Multiple at once (internally ends up calling the function below)
	static void ApplyEffectSkill( CDnPlayerActor* pPlayerActor, TEffectSkillData* EffectSkill, bool bOnInitialize = false , bool bIsPvpGameRoom = false );					// One by one
	static void RemoveEffectSkill( CDnPlayerActor* pPlayerActor, TEffectSkillData* EffectSkill );
#if defined( PRE_FIX_BUFFITEM )
	static void RemoveApplySkill( CDnPlayerActor* pPlayerActor, int nSkillID );
#endif
#endif

	int TryWarpVillage(int nMapIndex, INT64 nItemSerial);
	void	SetGuildRewardItem( TGuildRewardItem* GuildRewardItem );
	int		GetGuildRewardItemValue( int nType );
#if defined( PRE_ADD_GUILDREWARDITEM )
	void SetGuildRewardItem();
	void ApplyGuildRewardSkill( bool bUseItem = false );
	void AddGuildRewardItemBuff( int nItemID );
	void ApplyGuildRewardItemBuff( bool bUseItem = false );	
#endif
	void CheckAndSendStageClearLog(bool bClear = false);

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )	
	void ApplyPartyEffectSkillItemData( CSUseItem *pPacket, TItemData *pItemData, UINT nUseSessionID, EffectSkillNameSpace::ShowEffectType::eType eType, bool bSendDB, bool bUseItem );
	void SetGoldBoxItemID( int nItemID ) { m_nGoldBoxCheatID = nItemID; }
	int GetGoldBoxItemID() { return m_nGoldBoxCheatID; }
#endif

#if defined(PRE_ADD_ACTIVEMISSION)
	void SetActiveMissionCheatID(int nActiveMissionID) { m_nActiveMissionCheatID = nActiveMissionID; }
	int GetActiveMissionCheatID() { return m_nActiveMissionCheatID; }
#endif

#if defined( PRE_ADD_BESTFRIEND )	
	void BestFriendChangeLevel(BYTE cLevel, bool bSend = false);
#endif

#if defined( PRE_ADD_DIRECTNBUFF )
	bool bIsDirectPartyBuff() {return m_bDirectPartyBuffItem;}
	void SetDirectPartyBuffItem(bool bFlag, int nItemID) {m_bDirectPartyBuffItem = bFlag; m_DirectPartyBuffItemList.push_back(nItemID);}
	void SendDirectPartyBuffMsg();
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
	void SetComebackAppellation( int nAppelationID );
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	int GetStageClearBonusReward(char* pData, int nLen);
#endif
#if defined( PRE_FIX_BUFFITEM )
	void CheckEffectSkillItemData();
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	void SetBoxNumber(int nNum){ m_nRewardBoxNumber = nNum; }
	int GetBoxNumber(void){ return m_nRewardBoxNumber;}
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)

#if defined( PRE_ALTEIAWORLD_EXPLORE )
	int AlteiaWorldDice();
	void MoveAlteiaNextMap();
	void AddAlteiaWorldResult( bool bClearFlag );
	void SetAlteiaDailyPlayCount( BYTE cDailyPlayCount ) { m_cDailyPlayCount = cDailyPlayCount; }
#endif
	UINT m_nChatRoomID;
	CSyncLock m_StoreSync;
};

class IBoostPoolDNGameSession:public CDNUserSession, public TBoostMemoryPool<IBoostPoolDNGameSession>
{
public :
	IBoostPoolDNGameSession(unsigned int iUID, CDNRUDPGameServer * pServer, CDNGameRoom * pRoom):CDNUserSession(iUID,pServer,pRoom){}
	virtual ~IBoostPoolDNGameSession(){}
};
