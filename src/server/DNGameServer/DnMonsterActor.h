#pragma once

#include "DnActor.h"
#include "DnMonsterState.h"
#include "MAAiBase.h"
#include "MAAiReceiver.h"
#include "DnDropItem.h"
#include "PerfCheck.h"

class CDnMonsterActor : public CDnActor, public MAAiReceiver, public CDnMonsterState
{
public:
	CDnMonsterActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnMonsterActor();

	static int s_nPositionRevisionTime;
protected:
	SOBB m_GenerationArea;
	DNVector(CDnItem::DropItemStruct) m_VecDropItemList;
	int m_nMonsterClassID;
	TDUNGEONDIFFICULTY m_AIDifficult;
	float m_fScale;
	float m_fRevisionWeight;
	int m_nMonsterWeightTableID;
	
	bool m_bTimeMonster;		// Is this a monster that disappears after a specific time?
	bool m_bNoDamage;
	LOCAL_TIME m_nDestroyTime;		// How much time before disappearing?
	
	ElementEnum m_eElementType;	// Unlike players, monsters have their own ElementType.

#if defined( PRE_MOD_LOCK_TARGET_LOOK )
	int			m_nLockLookEventArea;
#endif	// #if defined( PRE_MOD_LOCK_TARGET_LOOK )
	bool		m_bLockLookTarget;
	char*		m_pszCanBumpActionName;
	
	bool		m_bCanBumpWall;		// Flag for whether to perform a specific action upon hitting a wall
	LOCAL_TIME	m_LastSendMoveMsg;

	float m_fRotateResistance;

	int m_nPartyComboCount;
	int m_nPartyComboDelay;
	DnActorHandle m_hPartyHitActor;

	bool	m_bIsTriggerMonster;	// Monster spawned by trigger
	int		m_iTriggerRandomSeed;	// RandomSeed value for monster spawned by trigger
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	int		m_nEventAreaUniqueID;	// EventAreaID when spawning from trigger
#endif // PRE_MOD_DARKLAIR_RECONNECT

	DnPropHandle m_hProp;
	UINT		m_uiForcePositionRevisionTick;
	UINT		m_uiPrevForcePositionRevisionTick;
	float		m_fForcePositionRevisionDelta;

	bool		m_bEnableDropItem;

	DnActorHandle m_hSummonerPlayerActor;
	bool m_bSuicideWhenSummonerDie;
	bool m_bFollowSummonerStage;
	DWORD m_dwSummonerActorID;
	int m_iSummonGroupID;
	float m_fLimitSummonerDistanceSQ;
	int m_nBirthAreaHandle;

	bool m_bSummoned;
	bool m_bReCreatedFollowStageMonster;

	std::map<int,int> m_mUseSkillCount;

#ifdef PRE_ADD_MONSTER_CATCH
	string m_strCatchBoneName;
	string m_strTargetActorCatchBoneName;
	string m_strCatchedActorAction;
	int m_iCatchedActorActionIndex;

	struct S_CATCH_ACTOR_INFO
	{
		DnActorHandle hCatchedActor;
		int iCatchCantMoveBlowID;
		int iCatchCantActionBlowID;

		S_CATCH_ACTOR_INFO( void ) : iCatchCantMoveBlowID( 0 ), iCatchCantActionBlowID( 0 ) {};
	};
	vector<S_CATCH_ACTOR_INFO> m_vlCatchedActors;

	set<DWORD> m_setCatchCheckedActorIDs;
#endif //#ifdef PRE_ADD_MONSTER_CATCH

	std::map<int,int> m_mOrderCount;

#if defined( PRE_FIX_MOVEBACK )
	bool	m_bNearMoveBack;				// Handling on action execution
	bool	m_bPrevMoveBack;				// Force execute Move_Back
#endif

	int		m_nAutoRecallRange;
	bool m_bChangeAxisOnFinishAction;

#ifdef PRE_ADD_TRANSFORM_MONSTER_ACTOR
	int m_nSwapActorID;
#endif

protected:
	void GenerationDropItem();

	// Movement Message
	virtual void OnDrop( float fCurVelocity );
	virtual void OnStop( EtVector3 &vPosition );
	virtual void OnBeginNaviMode();

	void DropItems();

	virtual void ProcessDie( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct );
	// Called after a HitSignal check finishes. Called from the attacker's side
	virtual void OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct );

	void CalcMonsterWeightIndex();

	void CalcItemDropTableID();
	std::string GetAIFileName();

	bool CheckPositionRevision();
	bool CheckSignalPositionRevision( float fDelta );
	void ResetPositionRevision();
	void ProcessPositionRevision( float fDelta );
	void ProcessPartyCombo( LOCAL_TIME LocalTime, float fDelta );
	void OnPartyCombo( DnActorHandle hHitter, int nComboDelay );
	void Process_AutoRecallRange();

private:

public:

	void SendProjectile( CDnProjectile *pProjectile, ProjectileStruct* pStruct, MatrixEx& LocalCross, int iSignalIndex );
	
	// Related to issue #15557. Case where a monster fires a projectile from a projectile.
	// Basically the same as what the client's LocalPlayerActor sends.
	// On client side, process packets identically to projectiles fired by party members.
	// Therefore CDnLocalPlayerActor::OnProjectile() must also be modified together. 
	void SendProjectileFromProjectile( CDnProjectile* pProjectile, int nSignalIndex );
	
	void SetGenerationArea( SOBB &Box );
	SOBB *GetGenerationArea();
	void SetBirthAreaHandle( int nIndex ) { m_nBirthAreaHandle = nIndex; }
	int GetBirthAreaHandle() { return m_nBirthAreaHandle; }

	virtual void SetTeam( int nValue );
	void SetAIDifficult( TDUNGEONDIFFICULTY Level ) { m_AIDifficult = Level; }

	void SetPropHandle( DnPropHandle hProp ){ m_hProp = hProp; }
	void SetMonsterClassID( int nValue ) { m_nMonsterClassID = nValue; }
	int GetMonsterClassID() { return m_nMonsterClassID; }

	int GetMonsterWeightTableID() { return m_nMonsterWeightTableID; }
	float	GetThreatRange();
	float	GetCognizanceThreatRange();
	float	GetCognizanceThreatRangeSq();

	void EnableNoDamage( bool bEnable ) { m_bNoDamage = bEnable; }
	bool IsEnableNoDamage() { return m_bNoDamage; }

	void SetScale( float fValue );
	float GetScale() { return m_fScale; }
	virtual float GetWeight();

	virtual float	GetRotateAngleSpeed();
	virtual bool	IsMonsterActor(){ return true; }
	virtual bool    IsSummonedMonster(){return m_bSummoned;}
	virtual bool	IsCannonMonsterActor(){ return false; }

	DnPropHandle	GetPropHandle(){ return m_hProp; }

	void SetTimeMonster(bool bEnable, LOCAL_TIME nTime) { m_bTimeMonster = bEnable; m_nDestroyTime = nTime; }
	LOCAL_TIME GetRemainDestroyTime( void ) { return m_nDestroyTime; };
	
	bool	_bIsCheckVaildPosition( EtVector3& vTargetPos );
	void	_SendCmdMoveNavi( EtVector3& vTargetPos, float fMinDistance, const char* szActionName, int nLoopCount );
	int		GetUseSkillCount( int iSkillID );

	// Actor Command
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdMove( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdMoveNavi( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdMoveNavi( EtVector3& vTargetPos, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f );
	virtual void CmdLook( EtVector2 &vVec, bool bForce = false );
	virtual void CmdLook( DnActorHandle hActor, bool bLock = true );
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true, bool bFromStateBlow = false, bool bSkillChain = false );
	virtual int  CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit = false, bool bCheckCanBegin = true , bool bEternity = false );
	virtual void CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo = true );
	virtual void SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	virtual void CmdSuicide( bool bDropItem, bool bDropExp );
	virtual void CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, float fFrame, float fBlendFrame = 3.f );
	virtual void CmdWarp( const EtVector3 &vPos, const EtVector2 &vLook, CDNUserSession* pGameSession=NULL, bool bCheckPlayerFollowSummonedMonster=false );

	virtual void SyncClassTime( LOCAL_TIME LocalTime );
	virtual bool Initialize();
	virtual void ResetActor();
	virtual bool IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal = NULL, int iHitUniqueID = -1 );

	// Actor Message
	virtual void ProcessAI(LOCAL_TIME LocalTime, float fDelta);
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessLook( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct );
	virtual void OnDie( DnActorHandle hHitter );
	virtual void OnBeginStateBlow( DnBlowHandle hBlow );
	
	virtual void ReTransmitSlaveMsg(CDNUserSession* pBreakIntoSession); // Used when sending to drop-in user for synchronization.
	virtual void SlaveOf( DWORD dwSummonerActorUniqueID, bool bSummoned = false, bool bSuicideWhenSummonerDie = false, bool bFollowSummonerStage = false, bool bReCreateFollowStageMonster = false );
	virtual void SlaveRelease( void );

	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false );

	virtual void RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage );

	// MASkillUser
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	virtual bool AddSkill( int nSkillTableID, int nLevel = 1, int iSkillLevelApplyType = CDnSkill::PVE );
	virtual void OnAddSkill( DnSkillHandle hSkill, bool isInitialize = false );
#endif // #ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	virtual bool ExecuteSkill( DnSkillHandle hSkill, LOCAL_TIME LocalTime, float fDelta );
	virtual CDnSkill::UsingResult UseSkill( int nSkillTableID, bool bCheckValid = true, bool bAutoUseFromServer = false, int nLuaSkillIndex=-1 );

	// TransAction Message
	virtual void OnChangeAction( const char *szPrevAction );
	virtual void OnFinishAction(const char* szPrevAction, LOCAL_TIME time);
	virtual bool OnPreAiProcess(const char* szPrevAction, LOCAL_TIME time) { return false; }
	void _CheckActionWithProcessPassiveActionSkill( const char* szPrevAction );

	// AI
	virtual bool bIsAILook(){ return (m_pAi) ? m_pAi->bIsAILook() : false; }
	virtual bool bIsTurnOnAILook(){ return true; }
	virtual bool bIsProjectileTargetSignal(){ return m_pAi ? m_pAi->bIsProjectileTargetSignal() : false; }
	virtual bool OnAINonTarget();
	virtual void OnBumpWall();

	// CDnActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	// True if it is a relic monster type summoned by a player (CDnClericRelicMonster)
	virtual bool IsClericRelicMonster( void ) { return false; };

	// Temporary log-related code 090226
	//void QueryLogStage(int nRoomID, int nMapIndex);

	ElementEnum GetElementType( void ) { return m_eElementType; };
	void SetMonsterElementType( ElementEnum eType ) { m_eElementType = eType; }

	// AI
	void	SetNaviDestination( SOBB* pOBB, UINT uiMoveFrontRate );
	int		GetWaitOrderCount( int iSkillID );
	void	AddWaitOrderCount( int iSkillID );
	void	DelWaitOrderCount( int iSkillID );

	// Trigger
	void	SetTriggerMonster( const int iRandomSeed ){ m_bIsTriggerMonster = true; m_iTriggerRandomSeed = iRandomSeed; }
	bool	bIsTriggerMonster(){ return m_bIsTriggerMonster; }
	int		GetTriggerRandomSeed(){ return m_iTriggerRandomSeed; }
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	void	SetEventAreaUniqueID( int nEventAreaUniqueID ) { m_nEventAreaUniqueID = nEventAreaUniqueID; }
	int		GetEventAreaUniqueID() { return m_nEventAreaUniqueID; }
#endif // PRE_MOD_DARKLAIR_RECONNECT

	void EnableDropItem( bool bEnable ) { m_bEnableDropItem = bEnable; }

	// Used only to back up and restore stats separately when copying stats to a summon monster.
	void SetRevisionWeight( float fRevisionWeight ) { m_fRevisionWeight = fRevisionWeight; };
	DnActorHandle GetSummonerPlayerActor( void ) { return m_hSummonerPlayerActor; };

	// Reset monster aggro (#21673)
	void ResetAggro( void );
	void ResetAggro( DnActorHandle hActor );

#ifdef PRE_ADD_MONSTER_CATCH
	void ProcessCatchActor( LOCAL_TIME LocalTime, float fDelta );
	void CatchActor( CatchActorStruct* pCatchActor, int nSignalIndex );
	void _CatchThisActor( DnActorHandle hResultActor, int nSignalIndex );
	void ReleaseAllActor( void );

	void ReleaseThisActor( S_CATCH_ACTOR_INFO& CatchActorInfo );
	void ReleaseThisActor( DnActorHandle hActor );
#endif // #ifdef PRE_ADD_MONSTER_CATCH

#ifdef PRE_ADD_TRANSFORM_MONSTER_ACTOR
	virtual void SwapActor( int nActorIndex ) {}
#endif

	bool IsFollowSummonerStage( void ) { return m_bFollowSummonerStage; };
	void SetLimitSummonerDistance( float fLimitSummonerDistance ) { m_fLimitSummonerDistanceSQ = fLimitSummonerDistance*fLimitSummonerDistance; };
	void SetSummonGroupID( int iSummonGroupID ) { m_iSummonGroupID = iSummonGroupID; };
	int GetSummonGroupID( void ) { return m_iSummonGroupID; };

	void SetAutoRecallRange( int nAutoRecallRange ) { m_nAutoRecallRange = nAutoRecallRange; }

protected:
	LOCAL_TIME m_RequestSummonTime;	//Summon monster creation time
public:
	void SetRequestSummonTime(LOCAL_TIME localTime) { m_RequestSummonTime = localTime; }
	LOCAL_TIME GetRequestSummonTime() { return m_RequestSummonTime; }

#if defined( PRE_FIX_MOVEBACK )
	void SetPrevMoveBack( bool bMoveBack ) { m_bPrevMoveBack = bMoveBack; }
	void SetNearMoveBack() { m_bNearMoveBack = m_bPrevMoveBack; }
	bool IsNearMoveBack() { return m_bNearMoveBack; }
	bool IsPrevMoveBack() { return m_bPrevMoveBack; }
#endif

#if defined(PRE_FIX_51048)
protected:
	struct PassiveStateEffectInfo
	{
		const CDnSkill::SkillInfo* pParentSkill;
		STATE_BLOW::emBLOW_INDEX emBlowIndex;
		int nDurationTime;
		std::string szParam;

		PassiveStateEffectInfo()
		{
			pParentSkill = NULL;
			emBlowIndex = STATE_BLOW::BLOW_NONE;
			nDurationTime = 0;
		}
	};

	std::list<PassiveStateEffectInfo> m_PassiveStateEffectInfoList;
	//If this flag is on when creating a monster, store it in a list and apply state effects by calling ApplyPassiveStateEffect after calling SendGameGenerationMonsterMsg.
	//If flag is not set, behave as before..
	bool m_EnablePassiveStateEffect;
public:
	//Set whether to store passive state effects in a list and process them later.
	void SetEnablePassiveStateEffectList(bool bSet) { m_EnablePassiveStateEffect = bSet; }
	bool GetEnablePassiveStateEffectList() { return m_EnablePassiveStateEffect; }

	void AddPassiveStateEffectInfo(PassiveStateEffectInfo& stateEffectInfo);	//In ApplyPassiveSkill
	void InitPassiveStateEffectInfo();											//Initialize list
	void ApplyPassiveStateEffect();												//Apply passive skill state effects.

	virtual bool ApplyPassiveSkill( DnSkillHandle hSkill, bool isInitialize = false );
#endif // PRE_FIX_51048

#if defined(PRE_FIX_61382)
	// Case where damage is dealt by a state effect. Currently only used for damage reflection. Damage value is received as positive. #23104
	virtual void RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage, CDnDamageBase::SHitParam* pHitParam = NULL );
#endif // PRE_FIX_61382

protected:
	bool m_isPuppetSummonMonster;
	bool m_bIsHelperAlly;
public:
	void SetPuppetSummonMonster(bool bFlag) { m_isPuppetSummonMonster = bFlag; }
	bool IsPuppetSummonMonster() { return m_isPuppetSummonMonster; }
	void SetHelperAlly(bool bFlag) { m_bIsHelperAlly = bFlag; }
	bool IsHelperAlly() const { return m_bIsHelperAlly; }
#ifdef IRINE_MOD_HERO_ALLY
	void ScaleHelperCombatStats( int nTargetLevel, int nRefAttack );
#else
	void ScaleHelperCombatStats( int /*nTargetLevel*/, int /*nRefAttack*/ ) {}
#endif

#if defined(PRE_FIX_64312)
public:
	void SendApplySummonMonsterExSkill(int nBaseSkillID, int nLevel, int nSelectedType, DWORD dwMasterUniqueID, int nMasterExSkillID);
#endif // PRE_FIX_64312
};

template <class RenderType, class DamageType>
class TDnMonsterActor : public CDnMonsterActor, virtual public RenderType, virtual public DamageType
{
public:
	TDnMonsterActor( CMultiRoom *pRoom, int nClassID ) : CDnMonsterActor( pRoom, nClassID )
	{
	}
	virtual ~TDnMonsterActor() {}

protected:

public:
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) {
		CDnMonsterActor::Process( LocalTime, fDelta );

		RenderType::PreProcess( LocalTime, fDelta );

		// #39347 AniDistance value reappears after PreProcess is called.
		if( 0 < GetCantXZMoveSEReferenceCount() )
			m_vAniDistance.x = m_vAniDistance.z = 0.0f;

		m_pMovement->Process( LocalTime, fDelta );
		RenderType::Process( m_Cross, LocalTime, fDelta );
	}

	virtual void Look( const EtVector2 &vVec, bool bForceRotate=true )
	{
		MAMovementInterface::Look( vVec, bForceRotate );

		if( bForceRotate )
		{
			if( strstr( m_hActor->GetCurrentAction(), "Turn" ) )
			{
				CmdAction( "Stand" );
			}
		}
	}
};

class IBoostPoolDnMonsterActor : public CDnMonsterActor, public TBoostMemoryPool< IBoostPoolDnMonsterActor >
{
public:
	IBoostPoolDnMonsterActor( CMultiRoom *pRoom, int nClassID ):CDnMonsterActor( pRoom, nClassID ){}
	virtual ~IBoostPoolDnMonsterActor(){}
};
