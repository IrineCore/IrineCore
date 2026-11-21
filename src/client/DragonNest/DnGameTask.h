#pragma once

#include "Task.h"
#include "InputReceiver.h"
#include "DnCamera.h"
#include "GameClientSession.h"
#include "DnLoadingTask.h"
#include "FrameSync.h"
#include "DnWorld.h"
#include "DnBridgeTask.h"
#include "DnHangCrashTask.h"
#include "DnStageClearImp.h"

class CDnMinimap;
class CEtWorldEventArea;
class CDnStageClearImp;
class CDnLocalPlayerActor;
class CCameraData;

class CDnGameTask : public CTask, public CInputReceiver, public CGameClientSession, public CDnLoadingStateMachine, public CEtUICallback, public CDnHangMonitorObject
{
public:
	CDnGameTask();
	virtual ~CDnGameTask();

	enum WarTimeEnum {
		None,
		Normal,
		Boss,
	};

	enum
	{
		MAX_CLEAR_RANK_COUNT = 7,
	};

#if defined( PRE_ADD_CP_RANK )
	struct SBestInfo
	{
		std::wstring wszName;
		char cRank;
		int nClearPoint;
		bool bNew;

		SBestInfo() : cRank( 0 ), nClearPoint( 0 ), bNew( false )
		{}

		void Reset()
		{
			cRank = 0;
			nClearPoint = 0;
			bNew = false;
		}
	};

	struct SDungeonClearBestInfo
	{
		SBestInfo sLegendClearBest;
		SBestInfo sMonthlyClearBest;
		SBestInfo sMyClearBest;
		
		void Reset()
		{
			sLegendClearBest.Reset();
			sMonthlyClearBest.Reset();
			sMyClearBest.Reset();
		}
	};
#endif	// #if defined( PRE_ADD_CP_RANK )

	static bool s_bCompleteFirstRender;

private:
	bool	m_bIsBreakIntoTask;	// Is this task created by drop-in??
	void	_MonsterAIProcess( LOCAL_TIME LocalTime, float fDelta );
	void	_Process( LOCAL_TIME LocalTime, float fDelta );

protected:
	GameTaskType::eType m_GameTaskType;
	CDnWorld *m_pWorld;
	DnCameraHandle m_hDebugCamera;
	DnCameraHandle m_hPlayerCamera;
	DnCameraHandle m_hDummyCamera;

	int m_nMapTableID;
#if defined(PRE_ADD_RENEW_RANDOM_MAP)
	int m_nRootMapTableID;
#endif
	int m_nStageConstructionLevel;
	int m_nStartPositionIndex;
	int m_nExtendDropRate;
	bool m_bFinalizeStage;
	bool m_bInitComplete;
	bool m_bShowDungeonOpenNotice;
	eBridgeState m_BridgeState;

	WarTimeEnum m_WarTimeType;
	int m_nAggroValue;
	std::vector<DnActorHandle> m_hVecAggroList;

	CDnMinimap *m_pMinimap;

	std::vector<DnActorHandle> m_hVecMonsterList;
	std::vector<DnActorHandle> m_hVecNpcList;

	int m_nDirectConnectMapIndex;
	int m_nDirectConnectGateIndex;
	CDnStageClearImp *m_pStageClearImp;

	bool m_bIsCombat;
	float m_fCombatDelta;

	bool m_bSyncStartToggleWeapon;
	bool m_bNeedSendReqSyncWait;
	bool m_bLookStageEntrance;
	bool m_bIsPVPShowHelmetMode;

	int m_nMaxCharCount;

#ifdef PRE_ADD_CRAZYDUC_UI
	bool m_bShowModDlg;
#endif 

#ifdef _FINAL_BUILD
#else
	float m_fTimerSec;
#endif

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	struct StageDamageLimitStruct 
	{
		float fDamageLimit;
		float fStateLimit;
	};
	StageDamageLimitStruct m_sStageDamageLimitStruct;
#endif

	struct MonsterTableStruct {
		std::string szName;
		int nActorTableID;
	};

	struct GenerationMonsterStruct {
		int nActorTableID;
		int nMonsterTableID;
		//		SAABox Box;
		CEtWorldEventArea *pArea;
		int nMonsterSpawn;
	};

	struct AreaStruct {
		//		SAABox Box;
		int nCompoundIndex;
		int nMin;
		int nMax;
		CEtWorldEventArea *pArea;
		int nMonsterSpawn;
	};

	struct MonsterCompoundStruct {
		int nMonsterTableID;
		int nPossessionProb;
		int nValue;
	};
	struct MonsterSetStruct {
		int nSetID;
		int nPosID;
		CEtWorldEventArea *pArea;
		int nMonsterSpawn;
	};

	struct MonsterSetProbStruct {
		int nItemID;
		int nProb;
	};
	struct MonsterGroupStruct {
		int nGroupID;
		int nCount;
	};

#ifdef PRE_ADD_CRAZYDUC_UI
	struct TimeAttackSetStruct
	{
		int nRemain;
		int nOrigin;
	};
	TimeAttackSetStruct m_sTimeAttackStruct;
#endif 

	bool m_bEnableDungeonClearLeaderWarp;

	struct ActorBirthStruct :public TBoostMemoryPool<ActorBirthStruct>
	{
		int nAreaUniqueID;
		DNVector(DnActorHandle) hVecList;
	};
	std::vector<ActorBirthStruct *> m_pVecActorBirthList;

#if defined( PRE_ADD_CP_RANK )
	bool m_bDungeonClearBestInfo;
	SDungeonClearBestInfo m_DungeonClearBestInfo;
#endif	// #if defined( PRE_ADD_CP_RANK )


#ifdef PRE_ADD_BESTFRIEND
	SCPartyBestFriend m_TempBFpacket;
#endif

	bool m_bIsCreateLocalActor;	
	bool m_bTutorial;

	// STE_CameraEffect_KeyFrame ---
	std::string m_strPreCameraEffectCamFile; // Previously referenced .cam file name.
	CCameraData * m_pCDCameraEffect;    // Data of the .cam file above.
	//------------------------------                           

protected:
#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
	virtual void PreGenerationMonster( int nRandomSeed );
#endif
	bool GenerationMonster( int nRandomSeed );
	bool GenerationNpc( int nRandomSeed );
	bool GenerationSetMonster( int nRandomSeed );
	int CalcMonsterIDFromMonsterGroupID( int nItemID );
	int CalcSetMonsterReference( int nSetID );
	void CalcSetMonsterGroupList( int nSetTableID, std::map<int, std::vector<MonsterGroupStruct>> &nMapResult );

	bool InitializeWorld( const char *szGridName );
	bool InitializeEnvi( int nMapIndex, const char *szGridName, const char *szEnviName );

	void InitializeMonster( DnActorHandle hMonster, DWORD dwUniqueID, int nMonsterID, SOBB &GenerationArea, CEtWorldEventArea *pBirthArea = NULL );
	DnActorHandle GenerationMonsterFromMonsterID( int nSeed, int nMonsterID, DWORD dwUniqueID, EtVector3 &vPosition, EtVector3 &vVel, EtVector3 &vResistance, SOBB *pGenerationArea, CEtWorldEventArea *pBirthArea = NULL, LOCAL_TIME CreateTime = 0, int nTeam = -1, bool bRandomFrameSummon = true, EtQuat* pqRotation = NULL );

	void InitializeCP();	

	static bool Sort_SetMonster_SetID( MonsterSetStruct &a, MonsterSetStruct &b );
	static bool Sort_SetMonster_PosID( MonsterSetStruct &a, MonsterSetStruct &b );
protected:
	bool CheckSyncProcess();
	void ProcessCombat( LOCAL_TIME LocalTime, float fDelta );

	// Let's change it to the OnXX style.
	virtual bool OnPostInitializeStage() { return true; }
	virtual bool OnDungeonReadyGate( CDnWorld::DungeonGateStruct *pStruct );

	virtual void ShowDebugInfo(){};
	virtual DnActorHandle InitializePlayerActor( CDnPartyTask::PartyStruct *pStruct ,int nPartyIndex , bool bLocalPlayer, bool bIsBreakInto=false );

	void ShowFPS_Information(const float& fDelta);
	void ShowDPS_Information( const float fDelta );
	void ShowCurrentAction_Information();
	void ShowMonsterAction_Information();
	void ShowCP_Information();
	void ShowVersion_Information();

	void ProcessDebugMsg( LOCAL_TIME LocalTime, float fDelta );
	void ProcessSound( LOCAL_TIME LocalTime, float fDelta );
	bool	_PostInitializeBreakIntoStage( int nRandomSeed );

//blondy end
protected:
//	void CalcDungeonClearInfo();

	// Loading-related
	static bool __stdcall OnLoadRecvGenerationPropCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime );
	static int __stdcall OnLoadRecvGenerationPropUniqueID( void *pParam, int nSize );

	static bool __stdcall OnLoadRecvGenerationMonsterCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime );
	static int __stdcall OnLoadRecvGenerationMonsterUniqueID( void *pParam, int nSize );

public:
	CDnStageClearImp *GetStageClearImp() { return m_pStageClearImp; }
	// Temporary for Test.
	void SetStageClearImp( CDnStageClearImp *pImp ) { m_pStageClearImp = pImp; }
	/*
	std::vector<SDungeonClearInfo>& GetDungeonClearInfoList() { return m_vecDungeonClearInfo; }
	SDungeonClearInfo* GetDungeonClearInfo( int nSessionID );
	SDungeonClearValue& GetDungeonClearValue() { return m_dungeonClearValue; }
	*/

	void SetBridgeState( eBridgeState State ) { m_BridgeState = State; }

	int GetEventExtendDropRate() { return m_nExtendDropRate; }

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	virtual bool InitializeAllPlayerActor();
	
	virtual int GetFarmZoneNumber()	{ return 0; }
	GameTaskType::eType GetGameTaskType() { return m_GameTaskType; }
	virtual bool InitializeStage( int nCurrentMapIndex, int nGateIndex, int nStageConstructionLevel, int nRandomSeed, bool bContinueStage, bool bDirectConnect );
	virtual bool InitializeNextStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex );

	// Note that nStartPositionIndex starts from 1.
	bool InitializeStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex );
	
	bool PostInitializeStage( int nRandomSeed );
	void FinalizeStage();

	void InitializeInterface();

	void RefreshConnectUser();
	void SetStartPosition( CDnPartyTask::PartyStruct* pStruct, int nPartyIndex, const UINT uiSessionID );
	EtVector3 GetGenerationRandomPosition( SOBB *pOBB );
#ifdef PRE_MOD_PRELOAD_MONSTER
	bool IsNoPreloadMonsterMap();
#endif

	// ClientSession
	virtual void OnDisconnectUdp( bool bValidDisconnect );
	virtual void OnDisconnectTcp( bool bValidDisconnect );

	virtual void SetSyncComplete( bool bFlag );
	virtual void AddConnectUser( DnActorHandle hActor, TCHAR *szUserID, bool bLocalPalyer );

	virtual void OnRecvRoomGenerationMonsterMsg( SCGenerationMonster *pPacket );
//	virtual void OnRecvRoomDestoryMonsterMsg( SCDestroyMonster* pPacket );
	virtual void OnRecvRoomGateInfoMsg( SCGateInfoMessage *pPacket );
	virtual void OnRecvRoomDungeonClearMsg( SCDungeonClear *pData );
	virtual void OnRecvRoomChangeGateState( SCChangeGateState *pPacket );
	//virtual void OnRecvRoomGenerationPropMsg( SCGenerationProp *pPacket );
	virtual void OnRecvRoomGenerationPropMsg( char* pPacket, int nSize );
	virtual void OnRecvRoomChangeGameSpeed( SCChangeGameSpeed *pPacket );
//	virtual void OnRecvRoomOKDungeonClear( SCOKDungeonClear *pPacket );
	virtual void OnRecvRoomWarpDungeonClear( SCWarpDungeonClear *pPacket );
	virtual void OnRecvRoomDungeonClearWarpFailed(SCDungeonClearWarpOutFail* pPacket);
	virtual void OnRecvRoomDungeonClearEnableLeaderWarp(SCDungeonClearLeaderWarp* pPacket);
	virtual void OnRecvRoomGateEjectInfo( SCGateEjectInfo *pPacket );
	virtual void OnRecvRoomDungeonFailedMsg( SCDungeonFailed *pPacket );
	virtual void OnRecvRoomOpenDungeonLevel( SCOpenDungeonOpenLevel *pPacket );
	virtual void OnRecvRoomSelectRewardItem( SCSelectRewardItem *pPacket );
	virtual void OnRecvRoomDungeonClearSelectRewardItem( char *pPacket );
	virtual void OnRecvRoomDungeonClearRewardItem( SCDungeonClearRewardItem *pPacket );
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	virtual void OnRecvRoomDungeonClearRewardBoxType(SCDungeonClearBoxType* pData);
#endif
	virtual void OnRecvRoomDungeonClearRewardItemResult( SCDungeonClearRewardItemResult *pPacket );
	virtual void OnRecvRoomIdentifyRewardItem( SCIdentifyRewardItem *pPacket );

	virtual void OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket );
	virtual void OnRecvPartyReadyGate( SCGateInfo *pPacket );
	virtual void OnRecvPartyStageStart( SCStartStage *pPacket );
	virtual void OnRecvPartyStageStartDirect( SCStartStageDirect *pPacket );
	virtual void OnRecvPartyStageCancel( SCCancelStage *pPacket );
	virtual void OnRecvPartyVillageStart( char *pData );
	virtual void OnRecvPartyRefresh( SCRefreshParty *pPacket );
	virtual void OnRecvDungeonTimeAttack( SCSyncDungeonTimeAttack* pPacket );
	virtual void OnRecvDungeonTimeAttackReset();
	virtual void OnRecvCharEntered( SCEnter *pPacket );
	virtual void OnRecvCharHide( SCHide *pPacket );
	virtual void OnRecvMaxCharCount( SCMaxLevelCharacterCount* pPacket );
	int GetMaxCharCount() { return m_nMaxCharCount; }
	virtual void OnRecvSpecialRebirthItem( SCSpecialRebirthItem * pData );
#if defined(PRE_ADD_REBIRTH_EVENT)
	virtual void OnRecvRebirthMaxCoin( SCRebirthMaxCoin* pPacket );
#endif

#ifdef PRE_ADD_BESTFRIEND
	virtual void OnRecvBestFriendData( char * pData );
#endif
#if defined(PRE_ADD_CP_RANK)
	virtual void OnRecvCPRank( SCAbyssStageClearBest *pData );
#endif	// #if defined(PRE_ADD_CP_RANK)

	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	// LoadingStateMachine
	bool RenderScreen( LOCAL_TIME LocalTime, float fDelta );

	void CheckBattleMode( DnActorHandle hActor, bool bAddRemoveValue );

	void InsertNpcList( DnActorHandle hActor );
	void RemoveNpcList( DnActorHandle hActor );

	int GetGameMapIndex() { return m_nMapTableID; }
	bool IsCombat() { return m_bIsCombat; }

	void GetEnableGateIndex( CDnWorld::DungeonGateStruct *pStruct, std::vector<int> &vecGateIndex );
	void ResetPlayerCamera();

	virtual void OnInitializeStartPosition( CDnPartyTask::PartyStruct *pStruct, const int iPartyIndex, EtVector3& vStartPos, float& fRotate, const UINT uiSessionID );
	virtual bool OnInitializeBreakIntoActor( CDnPartyTask::PartyStruct* pStruct, const int iVectorIndex );
	virtual bool Initialize();
	virtual void ProcessExitUser( DnActorHandle hActor ){};

	// Current stage difficulty
	int GetStageConstructionLevel() { return m_nStageConstructionLevel; }

	virtual void OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage ){}
	virtual void OnDie( DnActorHandle hActor, DnActorHandle hHitter ) {}
	virtual void OnGhost( DnActorHandle hActor );
	virtual void OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo ){}
	void SetRebirthCaptionOnGhost(CDnLocalPlayerActor* pLocalActor, bool bColorFilterOnDie);

	// Drop-in related
	void	SetBreakIntoTask(){ m_bIsBreakIntoTask = true; }
	bool	bIsBreakIntoTask(){ return m_bIsBreakIntoTask; }

	void SetLookStageEntrance() { m_bLookStageEntrance = true; }

	void	SetEnableDungeonClearLeaderWarp(bool bSet) { m_bEnableDungeonClearLeaderWarp = bSet; }
	bool	IsEnableDungeonClearLeaderWarp() const { return m_bEnableDungeonClearLeaderWarp; }

	DnCameraHandle GetPlayerCamera() { return m_hPlayerCamera; }

	void SetPVPShowHelmetMode(bool bTrue){m_bIsPVPShowHelmetMode = bTrue;};
	bool IsPVPShowHelmetMode(){return m_bIsPVPShowHelmetMode;};
	void InsertBirthAreaList( DnActorHandle hActor, int nAreaUniqueID );
	void RemoveBirthAreaList( int nAreaUniqueID );
	void RemoveBirthAreaList( DnActorHandle hActor, int nAreaUniqueID );
	DWORD GetBirthAreaLiveCount( int nAreaUniqueID );
	DWORD GetBirthAreaTotalCount( int nAreaUniqueID );
	bool GetBirthAreaLiveActors( int nAreaUniqueID, DNVector(DnActorHandle)& out );

#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	void CalcStageDamageLimit(  int nMapIndex  );
	StageDamageLimitStruct *GetStageDamageLimit(){ return &m_sStageDamageLimitStruct; }
#endif

	void TestAA();
#ifdef PRE_FIX_PARTYRESTORE_DGNCLEAR
	void OnAbnormalBreakIntoDungeonClearProcess(bool bShow);
#endif

#ifdef PRE_ADD_BESTFRIEND
	void SetBestFriendData();
#endif

#if defined( PRE_ADD_CP_RANK )
	bool IsDungeonClearBestInfo() { return m_bDungeonClearBestInfo; }
	SDungeonClearBestInfo & GetDungeonClearBestInfo() { return m_DungeonClearBestInfo; }
#endif	// #if defined( PRE_ADD_CP_RANK )

	bool IsCreateLocalActor(){
		return m_bIsCreateLocalActor;
	}
	bool IsTutorial() const { return m_bTutorial; }


	// STE_CameraEffect_KeyFrame ---
	bool CompareCamFileName( std::string & str ); // Compare .cam filename.
	void SetCamFileName( std::string & str ){
		m_strPreCameraEffectCamFile.assign( str );
	}	
	void SetCamData( CCameraData * pCam ){
		m_pCDCameraEffect = pCam;
	}
	CCameraData * GetCamData(){
		return m_pCDCameraEffect;    
	}
	//------------------------------     
#ifdef PRE_ADD_CRAZYDUC_UI
	void ShowMODDialog( bool bShow );
	bool IsShowMODDialog() { return m_bShowModDlg; }
	void SetTimeAttack( int nRemainSec, int nOriginSec );
	const TimeAttackSetStruct& GetTimeAttackInfo() { return m_sTimeAttackStruct;}
#endif 
};
