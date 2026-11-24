#pragma once

#include "DnUnknownRenderObject.h"
#include "EtMatrixEx.h"
#include "Timer.h"
#include "EtOctree.h"
#include "DnActionBase.h"
#include "DnActorState.h"
#include "MATransAction.h"
#include "MAActorRenderBase.h"
#include "MADamageBase.h"
#include "MASkillUser.h"
#include "DnBlowDef.h"
#include "MultiRoom.h"
#include "MAMovementInterface.hpp"
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
#include "FrameSync.h"
#endif

class CDnStateBlow;
class CDnProjectile;
class CDNAggroSystem;

class CDnStateBlowSignalProcessor;

class CDnActor : public CDnUnknownRenderObject< CDnActor, MAX_SESSION_COUNT >, 
				 public CDnActionBase, 
				 virtual public CDnActorState,
				 public MASkillUser, 
				 public MATransAction,
				 virtual public MADamageBase,
				 public MAMovementInterface,
				 virtual public MAActorRenderBase
{
public:

	enum eStateBlowAfterProcessType
	{
		eRebirth	= 1,
		eDelZombie	= (1<<1),
	};

	CDnActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnActor();

	friend class CDnActorActionStateCache;

	enum HitCheckTypeEnum {
		BoundingBox,
		Collision,
	};

	STATIC_DECL( DWORD s_dwUniqueCount );
	static float s_fDieDelta;

protected:

	STATIC_DECL( CEtOctree<DnActorHandle> *s_pOctree );
	typedef std::map<DWORD, DnActorHandle> mapActorSearch;
	STATIC_DECL( mapActorSearch s_dwMapActorSearch );

	int m_nClassID;
	DWORD m_dwUniqueID;
	MatrixEx m_Cross;
	EtVector3 m_vStaticPosition;
	SHitParam m_HitParam;
	DnWeaponHandle m_hWeapon[2];
	bool m_bSelfDeleteWeapon[2];

	CDNAggroSystem*	m_pAggroSystem;
	CEtOctreeNode<DnActorHandle> *m_pCurrentNode;

	bool m_bToggleIngnoreHit;

	int m_nTeam;
	float m_fStiffDelta;
	float m_fDownDelta;
	float m_fDieDelta;
	float m_fMaxDieDelta;
	float m_fLastDownRatio;

#ifdef PRE_ADD_GRAVITY_PROPERTY
	float m_fGravityEnd;
#endif // PRE_ADD_GRAVITY_PROPERTY

	map<DWORD, LOCAL_TIME> m_mapLastDamageTime;
	int m_nLastDamageHitterActionIndex;

	LOCAL_TIME m_LastHitSignalTime;
	
	// Similarly, store the unique ID of the last hit signal used by projectiles per target. 
	// If multiple targets hit simultaneously, it gets reset to -1 in OnDamage.
	map<DWORD, int> m_mapLastHitUniqueID;
	int m_nLastHitSignalIndex;
	int m_nAirComboCount;

	bool m_bEnableNormalSuperAmmor;
	int m_nNormalSuperAmmorTime;
	float m_fBreakNormalSuperAmmorDurability;

	int m_nLastUpdateSkillSuperAmmorValue[4];
	int m_nSkillSuperAmmorValue[4];
	int m_nSkillSuperAmmorTime;
	float m_fSkillSuperAmmorDamageDecreaseProb;

	float m_fAddHeight;
	HitCheckTypeEnum m_HitCheckType;

	bool m_bModifyPlaySpeed;
	LOCAL_TIME m_PlaySpeedStartTime;
	DWORD m_dwPlaySpeedTime;
	float m_fPlaySpeed;
	bool m_bShow;
	bool m_bOctreeUpdate;
	DWORD	m_dwGenTick;

	EtVector3 m_vPrevPosition;

	struct PreStateStruct {
		int nState;
		int nOffset;

		PreStateStruct( int State, int Offset ) {
			nState = State;
			nOffset = Offset;
		};
	};

	std::vector<std::vector<PreStateStruct>> m_VecPreActionState;
	std::vector<std::vector<PreStateStruct>> m_VecPreActionCustomState;
	std::vector<std::vector<PreStateStruct>> m_VecPreCanMoveState;
	
	DNVector(DnActorHandle) m_hVecLastHitListByRemainTime;
	DNVector(DnActorHandle) m_hVecLastHitList;

	CDnStateBlow *m_pStateBlow;

	// Whether to ignore normal super armor
	bool m_bIngnoreNormalSuperArmor;
	
	// Whether skill usability check mid-action was handled by signal
	bool m_bUseSignalSkillCheck;
	bool m_abSignalSkillCheck[ 3 ];	// 0:Movable 1:Jumpable 2:GroundMovable

	// Hanki 2009.7.27 
	// Related to effect application duration used on self (#1911)
	// State effect queue applied independently on server/client without sending packets to the server.
	// If the skill action has a separate ApplyStateEffect signal, it enters this queue and 
	// Applied at the timing defined in the action tool.
	struct S_NO_PACKET_SELF_STATEBLOW
	{
		CDnSkill::SkillInfo ParentSkillInfo;
		CDnSkill::StateEffectStruct StateEffect;
		bool bUsed;

		S_NO_PACKET_SELF_STATEBLOW( const CDnSkill::SkillInfo& SkillInfo, CDnSkill::StateEffectStruct SE ) : 
		ParentSkillInfo( SkillInfo ), StateEffect(SE), bUsed( false )
		{

		};

		void Used( void ) { bUsed = true; };
	};
	deque<S_NO_PACKET_SELF_STATEBLOW> m_dqApplySelfStateBlowQ;
	//
	

	// Signal Rotate.
	LOCAL_TIME m_dwRotateStartTime;
	LOCAL_TIME m_dwRotateTime;	
	bool m_bRotate;
	bool m_bRotLeft;	  // Rotation direction.	
	float m_fSpeedRot;    // Rotation speed.
	float m_fStartSpeed;  // Initial speed.
	float m_fEndSpeed;    // End speed.
	EtVector3 m_vRotAxis; // Rotation axis coordinates.
		

	INT64 m_iLastDamage;
	int m_iLastProcessPressCount;

	//Save the final Hit actor needed for calling Die.
	DnActorHandle m_hCompleteKillActor;
	bool m_bCompleteKill_AfterProcessStateBlow;

#ifdef _SHADOW_TEST
	bool m_bIsShadowActor;
	bool m_bEnableShadowActor;
#endif
	std::set<std::string> m_setUseActionName;

	int m_iCantMoveReferenceCount;
	int m_iCantActionReferenceCount;
	int m_iCantUseSkillReferenceCount;
	int m_iCantAttackReferenceCount;

	int m_iCantXZMoveSE;

	// When running signal at enemy's position via Charger state effect.
	bool m_bOnSignalFromChargerSE;
	DnActorHandle m_hChargetDestActor;
	DnBlowHandle m_hChargerBlowCalledOnSignal;

	// Flag to skip combo count calculation when OnDamage is called by a projectile fired from Charger state effect, etc.
	bool m_bAllowCalcCombo;

	std::list<DnMonsterActorHandle>	m_listSummonMonster;

	// #32426 Related to adding summon control feature. 
	// If a monster summoned by STE_Summon has a group ID defined, it is not added to m_listSummonMonster, and instead categorized and managed here.
	// Cannot summon more than the max count for summons of the same group.
	std::map<int, std::list<DnMonsterActorHandle> > m_mapSummonMonsterByGroup;

	int m_iFrameStopRefCount;

	vector<DnBlowHandle> m_vlReservedRemoveBlows;
	std::vector<int> m_vTriggerBuff;

	// #33312 Forcefully set element. #36689 Since it can be used in multiple places, handle it as a stack.
	DNVector(int) m_vlForceSettedHitElement;

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	STATIC_DECL( CFrameSkip s_MonsterProcess );
	STATIC_DECL( float s_fMonsterProcessDelta );

	STATIC_DECL( CFrameSkip s_NpcProcess );
	STATIC_DECL( float s_fNpcProcessDelta );
#endif
public:

	DWORD	GetGenTick(){ return m_dwGenTick; }
	bool bIsCanSummonMonster( const SummonMonsterStruct* pSummonMonsterStruct );
	virtual void PushSummonMonster( DnMonsterActorHandle hMonster, const SummonMonsterStruct* pSummonMonsterStruct, bool bReCreateFollowStageMonster = false );

	// Note: bOnPlayerInit means called when adding passive skills during initial stage load character creation and initialization
	//		 IsDie() is checked inside AddStateBlow when character is initialized during stage loading
	//		 Use this flag to distinguish because passive skill state effect is skipped when HP is 0
	int AddStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex, const CDnSkill::SkillInfo* pParentSkill, int nDurationTime, 
					  const char *szParam, bool bOnPlayerInit = false, bool bCheckCanBegin = true , bool bEternity = false );

	void DelStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	DnBlowHandle GetStateBlowFromID( int nStateBlowID );
	int RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	void RemoveStateBlowFromID( int nStateBlowID );
	void RemoveAllBlow();
	void RemoveAllBlowExpectPassiveSkill();
	bool IsShouldRemainBlowWhenDie( DnBlowHandle hBlow );
	int GetNumAppliedStateBlow( void );
	DnBlowHandle GetAppliedStateBlow( int iIndex );
	void GetAllAppliedStateBlow(DNVector(DnBlowHandle)& /*IN OUT*/ out);
	void GatherAppliedStateBlowByBlowIndex( STATE_BLOW::emBLOW_INDEX BlowIndex, /*IN OUT*/ DNVector(DnBlowHandle)& out );
	bool IsAppliedThisStateBlow( STATE_BLOW::emBLOW_INDEX iBlowIndex );
	CDnStateBlow *GetStateBlow() { return m_pStateBlow; }

	void GetAllAppliedStateBlowBySkillID(int nSkillID, DNVector(DnBlowHandle)& /*IN OUT*/ out);

	virtual void OnBeginStateBlow( DnBlowHandle hBlow ) {}
	virtual void OnEndStateBlow( DnBlowHandle hBlow );// {}

	void SetOnSignalFromChargerSE( bool bOnSignalFromChargerSE ) { m_bOnSignalFromChargerSE = bOnSignalFromChargerSE; };
	bool IsOnSignalFromChargerSE( void ) { return m_bOnSignalFromChargerSE; };
	void SetChargerDestActor( DnActorHandle hChargerDestActor ) { m_hChargetDestActor = hChargerDestActor; };
	void SetChargerBlow( DnBlowHandle hBlow ) { m_hChargerBlowCalledOnSignal = hBlow; };

	// Called whenever there are targets passing Hit check. Called from the attacker's side
	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct ) {}
	// Called after a HitSignal check finishes. Called from the attacker's side
	virtual void OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct );
	// Called on the actor object that fired the projectile when the projectile hits. 
	virtual void OnHitProjectile( LOCAL_TIME LocalTime, DnActorHandle hHittedTarget, const CDnDamageBase::SHitParam& HitParam ) {};
	// Action check
	virtual bool IsLimitAction( const char* pszActionName ){ return false; }
	virtual bool IsLimitAction( DnSkillHandle hSkill ){ return false; }

	void Die( DnActorHandle hHitter );
	void RequestKillAfterProcessStateBlow(DnActorHandle hHitter);
	void ExecuteKillAfterProcessStateBlow();

	virtual void OnStateBlowProcessAfter(){}
	virtual void OnAddStateBlowProcessAfterType( eStateBlowAfterProcessType Type ){}
	virtual void OnDie( DnActorHandle hHitter );
	virtual void OnBreakSkillSuperAmmor( int nIndex, int nOriginalSupperAmmor, int nDescreaseSupperAmmor ) {} // Attacker when broken
	virtual void OnBreakSkillSuperAmmor( SHitParam &HitParam ) {}	// Broken target when broken
	virtual void OnAirCombo( int nComboCount ) {}

	virtual int	 OnGetJobClassID(){ return 0; }
	void OnRequestHPMPDelta( /*IN OUT*/ INT64& nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta = false );
#ifdef PRE_FIX_77172
	void RequestHPMPDelta( ElementEnum eElement, INT64 nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta = false, 
							bool bShowValue = true, CDnDamageBase::SHitParam* pHitParam = NULL, bool bKeepJumpMovement = false );
#else // PRE_FIX_77172
	void RequestHPMPDelta( ElementEnum eElement, INT64 nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta = false, bool bShowValue = true, CDnDamageBase::SHitParam* pHitParam = NULL );
#endif // PRE_FIX_77172
	void RequestSEProbSuccess( int iSkillID, STATE_BLOW::emBLOW_INDEX eBlowIndex );
	void RequestCooltimeReset( int iSkillID );
#ifdef PRE_ADD_GRAVITY_PROPERTY
	void SetGravityEnd( float fGravityEnd ) { m_fGravityEnd = fGravityEnd; }
#endif // PRE_ADD_GRAVITY_PROPERTY

protected:

	void RequestDamage( CDnDamageBase *pHitter, int nSeed, INT64 nDamage );

	virtual void ProcessState( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessDown( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessDie( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessStiff( LOCAL_TIME LocalTime, float fDelta );
	void ProcessPlaySpeed( LOCAL_TIME LocalTime, float fDelta );
	void ProcessRotate( LOCAL_TIME LocalTime, float fDelta ); // Rotate.

	virtual void ProcessPress();
	void ProcessPress( DnActorHandle hTarget );
	bool CheckPressCircle2Clrcle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );		// 2D circle - 2D circle
	bool CheckPressCapsule2Capsule2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );	// 2D capsule - 2D capsule
	bool CheckPressCapsule2Circle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );	// 2D capsule - 2D circle

	void LinkWeapon( int nEquipIndex );

	static bool Compare_PreStateOrder( CDnActor::PreStateStruct &a, CDnActor::PreStateStruct &b );
	virtual const char *CheckChangeActionBlow( const char *szActionName );

	void ApplyStateEffectSignalProcess( const S_NO_PACKET_SELF_STATEBLOW &StateBlowInfo, ApplyStateEffectStruct* pStruct );

	// Signal Rotate.
	void SetRotate( DWORD dwTime, float fStart, float fEnd, EtVector3 & vPos, bool bLeft );

public:

	// MASkillUser
	virtual void OnSkillProjectile( CDnProjectile *pProjectile );

public:
	// Local
	int GetClassID() { return m_nClassID; }

	// Movement Message
	virtual void OnClash( EtVector3 &vPosition, OnClashFloorCheckType bFloorClash );
	__forceinline MatrixEx *GetMatEx() { return &m_Cross; }

	__forceinline void SetPosition( const EtVector3 &vPos ) { m_Cross.m_vPosition = vPos; }
	__forceinline EtVector3 *GetPosition() { return &m_Cross.m_vPosition; }

	__forceinline void SetStaticPosition( const EtVector3 &vPos ) { m_vStaticPosition = vPos; }
	__forceinline EtVector3 *GetStaticPosition() { return &m_vStaticPosition; }

	__forceinline void SetPrevPosition( EtVector3 &vPos ) { m_vPrevPosition = vPos; }
	__forceinline EtVector3 *GetPrevPosition() { return &m_vPrevPosition; }

	DWORD GetUniqueID() { return m_dwUniqueID; }
	virtual void SetUniqueID( DWORD dwUniqueID );

	CDNAggroSystem* GetAggroSystem() const { return m_pAggroSystem; }

	virtual void SetTeam( int nValue ) { m_nTeam = nValue; }
	int GetTeam() { return m_nTeam; }

	float GetAddHeight() { return m_fAddHeight; }
	void SetAddHeight( float fValue ) { m_fAddHeight = fValue; }

	void SetDownRemainDelta( float fDownDelta ) { m_fDownDelta = fDownDelta; };
	float GetDownRemainDelta() { return m_fDownDelta; }

	LOCAL_TIME GetDamageRemainTime( DWORD dwActorUniqueID );
	void SetDamageRemainTime( DWORD dwActorUniqueID, LOCAL_TIME LastDamageTime, int nCurrentActionIndex = -1 );
	void SetLastDamageHitUniqueID( DWORD dwActorUniqueID, int iLastHitUniqueID );
	int GetLastDamageHitUniqueID( DWORD dwActorUniqueID );

	void ResetDamageRemainTime();
	void SetStiffDelta( float fStiffDelta ) { m_fStiffDelta = fStiffDelta; };
	float GetStiffDelta() { return m_fStiffDelta; };

	bool IsModifyPlaySpeed() { return m_bModifyPlaySpeed; }
	void SetPlaySpeed( DWORD dwFrame, float fSpeed );
	void ResetPlaySpeed( void );
	void SetDieDelta(float fDelta) { m_fDieDelta = fDelta; }
	void SetMaxDieDelta(float fDelta) { m_fMaxDieDelta = fDelta; }
	DWORD GetPlaySpeedFrame() { return m_dwPlaySpeedTime; }

	void SetOctreeUpdate( bool bFlag ){ m_bOctreeUpdate = bFlag; }

	// Whether to ignore normal super armor; ignore if true.
	void SetIgnoreNormalSuperArmor(bool bIgnore) { m_bIngnoreNormalSuperArmor = bIgnore; }
	bool GetIgnoreNormalSuperArmor() { return m_bIngnoreNormalSuperArmor; }

	void InsertOctreeNode();
	void RemoveOctreeNode();

	SHitParam *GetHitParam() { return &m_HitParam; }
	HitCheckTypeEnum GetHitCheckType() { return m_HitCheckType; }

	virtual void ResetActor();

	virtual bool Initialize() { return true; }
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessAI( LOCAL_TIME LocalTime, float fDelta ){}
	virtual void ProcessLook( LOCAL_TIME LocalTime, float fDelta ){};
	virtual bool SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bCheck = true, bool bCheckStateEffect = true );
	virtual void SetCustomAction( const char *szActionName, float fFrame );

	virtual void SyncClassTime( LOCAL_TIME LocalTime );

	// For Octree..
	virtual void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false );
	virtual void GetBoundingBox( SAABox &Box );

	bool IsShow() { return m_bShow; }
	void Show( bool bShow );

	// ActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual bool LoadAction( const char *szFullPathName );
	virtual void FreeAction();
	void OnLoopAction( float fFrame, float fPrevFrame );

	// Actor Command
	virtual void CmdRefreshHPSP( INT64 nHP, int nSP );
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true, bool bFromStateBlow = false, bool bSkillChain = false );
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMove( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMoveNavi( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMoveNavi( EtVector3& vTargetPos, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMove( EtVector2 &vDir, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f ) {}
	virtual void CmdLook( EtVector2 &vVec, bool bForce = false ) {}
	virtual void CmdLook( DnActorHandle hActor, bool bLock = true ) {}

	// Note: bOnPlayerInit means called when adding passive skills during initial stage load character creation and initialization
	//		 IsDie() is checked inside AddStateBlow when character is initialized during stage loading
	//		 Use this flag to distinguish because passive skill state effect is skipped when HP is 0
	// [2011/03/18 semozz]
	//		Probably need to handle via packet whether OnBegin applies when creating state effects added by prefix skills
	virtual int CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit = false, bool bCheckCanBegin = true , bool bEternity = false );
	virtual void CmdModifyStateEffect( int iBlowID, STATE_BLOW& StateBlowInfo );

	virtual void CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo = true );
	virtual void SendRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex ) {};
	virtual void CmdRemoveStateEffectFromID( int nID );
	virtual void CmdRemoveStateEffectImmediately( STATE_BLOW::emBLOW_INDEX emBlowIndex );

	void SendRemoveStateEffectFromID( int nID );
	void SendAddSEFail( int iAddSEResult, STATE_BLOW::emBLOW_INDEX emBlowIndex );
	
	virtual void CmdSuicide( bool bDropItem, bool bDropExp ) {}
	virtual void CmdWarp();
	virtual void CmdWarp( const EtVector3 &vPos, const EtVector2 &vLook, CDNUserSession* pGameSession=NULL, bool bCheckPlayerFollowSummonedMonster=false );
	virtual void CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, float fFrame, float fBlendFrame = 3.f ) { }
	virtual void CmdChangeTeam( int nTeam );
	virtual void CmdForceVelocity( EtVector3 &vVelocity, EtVector3 &vResistance );
	virtual void CmdChatBalloon( LPCWSTR wszMessage );
	virtual void CmdShowExposureInfo( bool bShow );
	virtual void CmdForceAddSkill( int nSkillID );
	virtual void CmdForceRemoveSkill( int nSkillID );

	// Actor Message
	void SetAction( const char *szActionName, float fFrame, float fBlendFrame, bool bLoop = false );
	virtual bool CheckDamageAction( DnActorHandle hActor );
	virtual void CheckDamageVelocity( DnActorHandle hActor );
	virtual float PreCalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam, const float fDefenseRate=1.f, float fStateEffectAttackM = -1.0f );
	virtual float CalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam );

	// CDnDamageBase
	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct );

	// Weapon
	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false );
	virtual void DetachWeapon( int nEquipIndex = 0 );
	virtual DnWeaponHandle GetWeapon( int nEquipIndex = 0 ) { return m_hWeapon[nEquipIndex]; }
	void ShowWeapon( int nEquipIndex, bool bShow );

	virtual DnWeaponHandle GetActiveWeapon( int nEquipIndex ) { return m_hWeapon[nEquipIndex]; }

	// Projectile
	virtual void OnProjectile( CDnProjectile *pProjectile, ProjectileStruct* pStruct, MatrixEx& LocalCross, int nSignalIndex );

	// Additional Packet Message
	// Damage Packet
	virtual void OnDamageWriteAdditionalPacket( CPacketCompressStream *pStream ) {}
	virtual void OnDamageReadAdditionalPacket( CPacketCompressStream *pStream ) {}

	// AI
	virtual bool bIsAILook(){ return false; }

	// 
	virtual bool IsGMTrace() const { return false; }

	// Static functions
	static bool InitializeClass( CMultiRoom *pRoom );
	static void ProcessClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta );
	static void ProcessAIClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta );
	static void ReleaseClass( CMultiRoom *pRoom );
	static DnActorHandle FindActorFromUniqueID( CMultiRoom *pRoom, DWORD dwUniqueID );
	static DnActorHandle FindActorFromName( CMultiRoom *pRoom, TCHAR *szName );
	static void InsertUniqueSearchMap( CMultiRoom *pRoom, CDnActor *pActor );
	static void RemoveUniqueSearchMap( CMultiRoom *pRoom, CDnActor *pActor );
	static void InitializeNextStage( CMultiRoom* pRoom  );
	
	// Range Scan functions
	static int ScanActor( CMultiRoom *pRoom, EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside = false );
	static void GetActorList(CMultiRoom *pRoom, DNVector(DnActorHandle) &VecList);
	static void GetOpponentActorList(CMultiRoom *pRoom, int nTeam, DNVector(DnActorHandle) &VecList, EtVector3* pPos=NULL, float fMaxRange=0.f );

	static int ScanActorByActorSize( CMultiRoom *pRoom, EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside = false, bool bActorSize = false );

	// For performance, game server splits the function itself instead of using = NULL.
	__inline int GetState() { return CDnActorState::GetState(); }
	int GetState( const char *szActionName );
	int GetState( int nElementIndex );

	bool IsStay( const char *szActionName );
	bool IsMove( const char *szActionName );
	bool IsAttack( const char *szActionName );
	bool IsHit( const char *szActionName );
	bool IsAir( const char *szActionName );
	bool IsDown( const char *szActionName );
	bool IsStun( const char *szActionName );
	bool IsStiff( const char *szActionName );
	bool IsFly( const char *szActionName );
	bool IsGround( const char *szActionName );
	bool IsStandHit( const char *szActionName );

	__inline bool IsStay() { return ( CDnActorState::GetState() & ActorStateEnum::Stay ) ? true : false; }
	__inline bool IsMove() { return ( CDnActorState::GetState() & ActorStateEnum::Move ) ? true : false; }
	__inline bool IsAttack() { return ( CDnActorState::GetState() & ActorStateEnum::Attack ) ? true : false; }
	__inline bool IsHit() { return ( CDnActorState::GetState() & ActorStateEnum::Hit ) ? true : false; }
	__inline bool IsAir() { return ( CDnActorState::GetState() & ActorStateEnum::Air ) ? true : false; }
	__inline bool IsDown() { return ( CDnActorState::GetState() & ActorStateEnum::Down ) ? true : false; }
	__inline bool IsStun() { return ( CDnActorState::GetState() & ActorStateEnum::Stun ) ? true : false; }
	__inline bool IsStiff() { return ( CDnActorState::GetState() & ActorStateEnum::Stiff ) ? true : false; }
	__inline bool IsFly() { return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_Fly ) ? true : false; }
	__inline bool IsFly2() { return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_Fly2 ) ? true : false; }
	__inline bool IsGround() { return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_Ground ) ? true : false; }
	__inline bool IsUnderGround() { return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_UnderGround ) ? true : false; }
	__inline bool IsStandHit() {
		int nState = CDnActorState::GetState();;
		if( nState == ActorStateEnum::Hit || nState == ( ActorStateEnum::Hit | ActorStateEnum::Stiff ) ) return true;
		return false;
	}

	virtual bool IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal = NULL, int iHitUniqueID = -1 );
	virtual bool IsHittableSkill( int iSkillIndex , bool &bSendResist );

	virtual void ProcessCollision( EtVector3 &vMove );

	virtual bool	IsPlayerActor() { return false; }
	virtual bool	IsNpcActor(){ return false; }
	virtual bool	IsMonsterActor(){ return false; }
	virtual bool	IsPartsMonsterActor(){ return false; }
	virtual bool    IsVehicleActor() { return false; }
	virtual UINT	GetSessionID(){ return 0; }
	virtual int GetSwapSingleSkinActorID() { return -1; }
	virtual int GetMonsterMutationTableID(){ return -1; }

	virtual bool bIsDestroyParts( const UINT uiTableID ){ return false; }
	virtual bool bIsDestroySetParts( const UINT uiSetID ){ return false; }
	virtual DWORD GetDamageObjectUniqueID() { return GetUniqueID(); }

	virtual void OnInitializeNextStage( void );
	void OnInitializePVPRoundRestart( void );

	void AddedCantMoveSE( void ) { ++m_iCantMoveReferenceCount; };
	void RemovedCantMoveSE( void ) { if( m_iCantMoveReferenceCount > 0 ) --m_iCantMoveReferenceCount; };
	int GetCantMoveSEReferenceCount( void ) { return m_iCantMoveReferenceCount; };
	void AddedCantActionSE( void ) { ++m_iCantActionReferenceCount; };
	void RemovedCantActionSE( void ) { if( m_iCantActionReferenceCount > 0 ) --m_iCantActionReferenceCount; };
	int GetCantActionSEReferenceCount( void ) { return m_iCantActionReferenceCount; };
	void AddedCantUseSkillSE( void ) { ++m_iCantUseSkillReferenceCount; };
	void RemovedCantUseSkillSE( void ) { if( m_iCantUseSkillReferenceCount > 0 ) --m_iCantUseSkillReferenceCount; };
	int GetCantUseSkillSEReferenceCount( void ) { return m_iCantUseSkillReferenceCount; };
	void AddedCantAttackSE( void ) { ++m_iCantAttackReferenceCount; };
	void RemovedCantAttackSE( void ) { if( m_iCantAttackReferenceCount > 0 ) --m_iCantAttackReferenceCount; };
	int GetCantAttackSEReferenceCount( void ) { return m_iCantAttackReferenceCount; };
	void AddedCantXZMoveSE( void ) { ++m_iCantXZMoveSE; };
	void RemovedCantXZMoveSE( void ) { if( m_iCantXZMoveSE ) --m_iCantXZMoveSE; };
	int GetCantXZMoveSEReferenceCount( void ) { return m_iCantXZMoveSE; };

	void ResetStateEffect( int nSkillDurationType );
	void CmdSyncBlow( CDNUserSession* pGameSession );

	// from CDnDamageBase
	virtual DnActorHandle GetActorHandle( void ) { return GetMySmartPtr(); };

	bool IsUseSignalSkillCheck( void ) { return m_bUseSignalSkillCheck; };
	bool IsAllowedSkillCheck( int nCheckType )
	{
		if( 0 <= nCheckType && nCheckType < 3 )
			return m_abSignalSkillCheck[ nCheckType ];
		else
			return false;
	}
	void SetSignalSkillCheck( int nCheckType, bool bCheck )
	{ 
		if( 0 <= nCheckType && nCheckType < 3 )
			m_abSignalSkillCheck[ nCheckType ] = bCheck;
	}

	void ResetProcessSkill()
	{
		if( IsProcessSkill() )
		{
			if( m_hProcessSkill )
			{
				m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
				m_hProcessSkill.Identity();
			}
		}
	}
	
	bool bIsObserver();
	static float SquaredDistance( const EtVector3& vPos, const SAABox& BBox, bool bNear = true );

	// Catch the action change point and end the skill, since another action can now execute mid-skill.
	// End the skill to accurately match the end timing of state effect application.
	//virtual void OnChangeAction( const char *szPrevAction );

	ElementEnum CalcHitElementType( bool bUseSkillApplyWeaponElement, CDnState::ElementEnum eSkillElement = CDnState::ElementEnum_Amount, bool bUsingSkill = false );

	// Hanki 2009.7.27 
	// Related to effect application duration used on self (#1911)
	void AddStateEffectQueue( const CDnSkill::SkillInfo& ParentSkillInfo, const CDnSkill::StateEffectStruct& StateEffectInfo );
	void ClearSelfStateSignalBlowQueue( bool bItemSkill = false );
	// 

#ifdef _SHADOW_TEST
	void EnableShaodowActor( bool bValue ) { m_bEnableShadowActor = bValue; }
	bool IsShadowActor() { return m_bIsShadowActor; }
	bool IsShadowActor() const { return m_bIsShadowActor; }
#endif

	void ResetSkillSuperAmmor( void ) { ZeroMemory( m_nSkillSuperAmmorValue, sizeof(m_nSkillSuperAmmorValue) ); };
	void SetSkillSuperAmmor( int iValue ) { m_nSkillSuperAmmorValue[ 0 ] = iValue; };

	INT64 GetLastDamage( void ) { return m_iLastDamage; };

	// To check the moment MP is consumed in a state effect... currently used only on the server.
	// Used by state effects that decrease or increase MP consumption. 
	// MP consumed through this function receives a separate packet from the server, so nothing to do on client!
	void UseMP( int iMPDelta );

	// If summoned actor, set the master.
	// Overridden in monster actor to send packet to client. 
	virtual void SlaveOf( DWORD dwSummonerActorUniqueID, bool bSummoned = false, bool bSuicideWhenSummonerDie = false, bool bFollowSummonerStage = false, bool bReCreateFollowStageMonster = false ) {};

	void SetAllowCombo( bool bAllowCombo ) { m_bAllowCalcCombo = bAllowCombo; };
	bool IsAllowCombo( void ) { return m_bAllowCalcCombo; };

	void ToggleIgnoreHit(){ m_bToggleIngnoreHit ^= 1; }
	bool bIsIgnoreHit(){ return m_bToggleIngnoreHit; }
	
	void AddedFrameStop( void );
	void RemovedFrameStop( void );
	int GetFrameStopRefCount( void ) { return m_iFrameStopRefCount; };
	void UpdateFPS( void );

	void AddReserveRemoveBlow( DnBlowHandle hBlow ) { m_vlReservedRemoveBlows.push_back( hBlow ); };
	void ProcessReservedRemoveBlows( void );

	// Case where damage is dealt by a state effect. Currently only used for damage reflection. Damage value is received as positive. #23104
	virtual void RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage, CDnDamageBase::SHitParam* pHitParam = NULL );

	// Add/remove buffs added via trigger
	void ClearTriggerBuff();
	void AddTriggerBuff( int nID )	{ m_vTriggerBuff.push_back(nID); }

	void CmdFreezingPrisonDurablity(int nStateBlowID, float fDuabilityRate, bool bShowGauge);

	virtual void RemoveResetStateBlow();

	//////////////////////////////////////////////////////////////////////////
	// Function to knock back the attacker [2011/01/14 semozz]
	void ForceKnockBack(DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam);
	//////////////////////////////////////////////////////////////////////////

	//Remove state effects applied by items not allowed on this map.
	void RemoveNonAvailableStateBlow();

	// Remove active state effects.
	void RemovedActivatedStateBlow( bool bIgnoreItemSkill = false );


//#if defined(SkillCooldownRatio)
	void FindSkillBySkillType(CDnSkill::SkillTypeEnum eSkillType, DNVector(DnSkillHandle)& vlSkillList);
//#endif // Skill cooldown ratio

	// Currently only Elemental Lord's Icy Fraction exceptionally filters 2 state effects
	// Want only one of the two to be hit, so prevent the first checked one from being caught in the next filtering. #28747
	// Inherited from CDnPlayerActor.
	virtual void OnHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex ) {};
	virtual bool CheckHitSignalStateEffectFilterException( DWORD dwTargetActorUniqueID, int iBlowIndex ) { return true; };

	virtual bool IsMyRelicMonster( DnActorHandle hActor ) { return false; };

#ifdef PRE_ADD_MONSTER_CATCH
	bool CatchCalcSuperArmor( DnActorHandle hCatcherActor, int iSuperArmorDamage );
#endif // #ifdef PRE_ADD_MONSTER_CATCH


	void CmdUpdateStateBlow(int nBlowID);
	void CmdFinishAuraSkill(DWORD nSkillID);

	bool IsImmuned(STATE_BLOW::emBLOW_INDEX blowIndex);

	//Function to force OnBegin call for prefix state effect
	void ForceBeginStateBlow(DnBlowHandle hBlow);

protected:
	CDnStateBlowSignalProcessor* m_pStateBlowSignalProcessor;

	bool CanUsePrefixSkill();
public:
	void AddStateBlowSignal(DnBlowHandle hBlow);
	void RemoveStateBlowSignal(DnBlowHandle hBlow);

	virtual bool CheckCollisionHitCondition(const EtVector3& vObjPos, const MatrixEx& objCross, EtVector3 &vTargetPos, float angleAllow);

public:
	struct HitLimitCountInfo
	{
		int nHitLimitCount;
		int nHitCount;

		HitLimitCountInfo(int nLimitCount, int hitCount)
		{
			nHitCount = hitCount;
			nHitLimitCount = nLimitCount;
		}
	};
	typedef std::map<int, HitLimitCountInfo> HIT_LIMIT_COUNT_INFO_LIST;
	HIT_LIMIT_COUNT_INFO_LIST m_HitLimitCountInfoList;

protected:
	struct ActorHandleComp
	{
		bool operator()(const DnActorHandle& h1, const DnActorHandle& h2) const
		{
			return (h1.GetHandle() < h2.GetHandle());
		}
	};

	struct StateBlowInfo
	{
		STATE_BLOW::emBLOW_INDEX blowIndex;
		std::string strValue;
	};

	typedef std::list<StateBlowInfo> STATE_BLOW_INFO_LIST;
	struct StealMagicBuffInfo
	{
		CDnSkill::SkillInfo skillInfo;	//Skill info
		int nDurationTime;				//State effect duration

		STATE_BLOW_INFO_LIST stateBlowList;
	};
	
	typedef std::map<DnActorHandle, StealMagicBuffInfo, ActorHandleComp> STEAL_MAGIC_BUFF_INFO_LIST;	//Candidate state effect list to add via Steal Magic
	typedef std::map<int, std::list<int>> STEAL_MAGIC_BUFF_LIST;			//State effect list applied by Steal Magic

	STEAL_MAGIC_BUFF_INFO_LIST m_StealMagicBuffAddList;	//Candidate list to add from Steal Magic
	STEAL_MAGIC_BUFF_LIST m_StealMagicBuffList;	//Actually applied list

	std::map<int, int> m_RemovedStealMagicStateBlowIDList;	//List storing state effects removed in OnEndStateBlow that were caused by Steal Magic
public:
	void ReserveStealMagicBuff(DnActorHandle hActor, CDnSkill::SkillInfo *pSkillInfo, int nDurationTime, STATE_BLOW::emBLOW_INDEX blowIndex, const char* szValue);	//Reserve state effect info to add via Steal Magic.
	void UpdateStealMagicBuff();					//Select one of reserved <skill, state effect list> and apply state effect to actual actor
	
protected:
	void AddStealMagicStateBlow(int nSkillID, int nBlowID);	
	void RemoveStealMagicStateBlow(int nDeletedBlowID);

	LOCAL_TIME CheckRemainFrameTime();


protected:
	int m_nOriginalTeam;
public:
	void SetOriginalTeam(int nTeam) { m_nOriginalTeam = nTeam; }
	int GetOriginalTeam() { return m_nOriginalTeam; }

	void PushForceHitElement( ElementEnum eElement ) { m_vlForceSettedHitElement.push_back(eElement); };
	void PopForceHitElement( void ) { m_vlForceSettedHitElement.pop_back(); };

	const std::list<DnMonsterActorHandle>& GetSummonedMonsterList( void );
	const std::map<int, std::list<DnMonsterActorHandle> >& GetGroupingSummonedMonsterList( void );

	DnMonsterActorHandle FindOldSummonMonster(const SummonMonsterStruct* pSummonMonsterStruct);

	void RequestActionChange(int nActionIndex);

	void SendChainAttackProjectile(DnActorHandle hRootAttacker, DWORD dwPrevAttackerActorUniqueID, int iActionIndex, int iProjectileSignalArrayIndex, DnActorHandle hActorToAttack, int iSkillID);

#ifdef PRE_FIX_GAMESERVER_PERFOMANCE
	virtual bool IsAllowCallSkillProcess( float fDelta ) { return false; };
#endif // #ifdef PRE_FIX_GAMESERVER_PERFOMANCE

	void RemoveStateEffectByHitSignal(HitStruct* pHitStruct);

	virtual void UpdateMissionByMonsterKillCount(int nCount) {};

	bool ProcessIgnoreHitSignal();
	bool ProcessIgnoreGravitySignal();

//Add skill state effect via CDnApplyStateBlowOnTimeBlow
protected:
	std::vector<DnSkillHandle> m_ApplySkillStateEffectList;
public:
	void AddSkillStateEffect(DnSkillHandle hSkill);
	void ApplySkillStateEffect();

	void SendProbInvincibleSuccess( void );

protected:
	std::string m_FindAutoTargetName;
public:
	void SetFindAutoTargetName(const char* szBipName) { m_FindAutoTargetName = szBipName ? szBipName : ""; }
	std::string& GetFindAutoTargetName() { return m_FindAutoTargetName; }

	//Function to return "Bip01" position when the monster's actual character position does not move
	//Guided Shot issue in Green Dragon. For now, other actors return their existing actor position.
	EtVector3 FindAutoTargetPos();


//////////////////////////////////////////////////////////////////////////
// Function to get SelfDelete info of the equipped weapon on weapon change.
public:
	bool GetWeaponSelfDelete(int nIndex) { return m_bSelfDeleteWeapon[nIndex]; }
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//Attach/Detach Weapon Lock functions used for weapon change state effect
protected:
	bool m_bChangeWeaponLocked;
	bool m_bSkipChangeWeaponAction;	//Skip weapon swap action..
	bool m_bSkipOnAttatchDetachWeapon;	//Set to skip calling OnAttachWeapon/OnDetachWeapon after weapon swap
public:
	void SetChangeWeaponLock(bool bLock) { m_bChangeWeaponLocked = bLock; }
	bool IsChangeWeaponLock() { return m_bChangeWeaponLocked; }
	void SetSkipChangeWeaponAction(bool bSkip) { m_bSkipChangeWeaponAction = bSkip; }
	bool IsSkipChangeWeaponAction() { return m_bSkipChangeWeaponAction; }
	void SetSkipOnAttatchDetachWeapon(bool bSkip) { m_bSkipOnAttatchDetachWeapon = bSkip; }
	bool IsSkipOnAttatchDetachWeapon() { return m_bSkipOnAttatchDetachWeapon; }
//////////////////////////////////////////////////////////////////////////

#if defined(PRE_ADD_50917)
protected:
	int m_ChangeWeaponRefCount;

	DnWeaponHandle m_hOrigWeaponWhenChangeWeapon;		//Original weapon info at weapon unequip time
	bool m_bOrigWeaponWhenChangeWeaponSelfDelete;		//Original SelfDelete info at weapon unequip time

public:
	void AddChangeWeaponRefCount() { ++m_ChangeWeaponRefCount; }
	void RemoveChangeWeaponRefCount() { --m_ChangeWeaponRefCount; }
	int GetChangeWeaponRefCount() { return m_ChangeWeaponRefCount; }

	void SetOrigWeaponWhenChangeWeapon(DnWeaponHandle hWeapon, bool bSelfDelete) { m_hOrigWeaponWhenDisarmament = hWeapon, m_bOrigWeaponWhenDisarmamentSelfDelete = bSelfDelete; }
	DnWeaponHandle GetOrigWeaponWhenChangeWeapon() { return m_hOrigWeaponWhenDisarmament; }
	bool GetOrigWeaponWhenChangeWeaponSelfDelete() { return m_bOrigWeaponWhenDisarmamentSelfDelete; }
#endif // PRE_ADD_50917

#if defined(PRE_ADD_50907)
protected:
	int m_DisarmamentRefCount;

	DnWeaponHandle m_hOrigWeaponWhenDisarmament;		//Original weapon info at weapon unequip time
	bool m_bOrigWeaponWhenDisarmamentSelfDelete;		//Original SelfDelete info at weapon unequip time

public:
	void AddDisarmamentRefCount() { ++m_DisarmamentRefCount; }
	void RemoveDisarmamentRefCount() { --m_DisarmamentRefCount; }
	int GetDisarmamentRefCount() { return m_DisarmamentRefCount; }

	void SetOrigWeaponWhenDisarmament(DnWeaponHandle hWeapon, bool bSelfDelete) { m_hOrigWeaponWhenDisarmament = hWeapon, m_bOrigWeaponWhenDisarmamentSelfDelete = bSelfDelete; }
	DnWeaponHandle GetOrigWeaponWhenDisarmament() { return m_hOrigWeaponWhenDisarmament; }
	bool GetOrigWeaponWhenDisarmamentSelfDelete() { return m_bOrigWeaponWhenDisarmamentSelfDelete; }
#endif // PRE_ADD_50907

#if defined(PRE_FIX_50482)
protected:
	int m_nChangeTeamRefCount;
public:
	void AddChangeTeamRefCount() { ++m_nChangeTeamRefCount; }
	void RemoveChangeTeamRefCount() { --m_nChangeTeamRefCount; }
	int GetChangeTeamRefCount() { return m_nChangeTeamRefCount; }
#endif // PRE_FIX_50482

#if defined(PRE_FIX_51048)
	void RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta);
#endif // PRE_FIX_51048

	virtual void SetState(ActorStateEnum State);
	virtual void OnAttackChange();

	void ScanActorByStateIndex(DNVector(DnActorHandle) &Veclist, STATE_BLOW::emBLOW_INDEX blowIndex);

#if defined(PRE_FIX_55378)
	//In Charger state effect SignalProcess, for Player actor, projectile creation does not matter since server and client each create their own, however
	//In case of a monster actor, the client does not create the projectile locally and instead creates it upon receiving a packet from the server,
	//ActionIndex received at this point does not match, so projectile is not created on client.
	//In Charger state effect SignalProcess, set charger action before calling DnActor::OnSignal, and reset it when finished.
	//In projectile packet sending, if ChargerAction is set, replace m_nActionIndex with ChargerAction's action index when sending the packet.
protected:
	std::string m_ChargerAction;

public:
	void SetChargerAction(const char* szActionName) { m_ChargerAction = szActionName ? szActionName : ""; }
	std::string& GetChargerAction() { return m_ChargerAction; }
#endif // PRE_FIX_55378

	//#53454 During Puppet Hit signal processing, both the summoner actor and Puppet 2 actor process the Hit signal
	//Damage is applied twice; when processing the same signal, check puppet/puppet-owner actors in the Hit actor list, 
	//Exclude puppet owner actor from Hit signal processing.
public:
	static int ExceptionHitList(DNVector(DnActorHandle) &hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct);
	static int ExceptionHitList2(DNVector(DnActorHandle) &hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct, 
								 DNVector(DnActorHandle)& hAddStateEffectActorList, 
								 int checkType, float fScanLength, EtVector3& vPrePos);
	static void CheckHitAreaActorList(DNVector(DnActorHandle)& hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct, int checkType, float fScanLength, EtVector3& vPrePos);

#if defined(PRE_FIX_59347)
protected:
	bool m_bApplyPartsDamage;

public:
	void SetApplyPartsDamage(bool bEnable) { m_bApplyPartsDamage = bEnable; }
	bool GetApplyPartsDamage() { return m_bApplyPartsDamage; }

	virtual void ApplyPartsDamage(float& fDamage, DnActorHandle hHitterActor) {}
#endif // PRE_FIX_59347

#if defined(PRE_FIX_61382)
	//Return owner actor if it is a puppet monster.
	static DnActorHandle GetOwnerActorHandle(DnActorHandle hActor);
#endif // PRE_FIX_61382

#if defined(PRE_ADD_65808)
protected:
	//Multiple skill heraldries possible on a monster..
	std::map<int, std::list<int>> m_SummonMonsterGlyphInfo;
	//Monster, skill heraldry, state effect..
	struct _StateEffectInfo
	{
		int nStateEffectID;
		DWORD dwUniqueID;
	};
	std::map<int, std::map<int, std::list<_StateEffectInfo>>> m_SummonMonsterGlyphStateEffectIDs;

public:
	void AddSummonMonsterGlyphInfo(int monsterID, int glyphID);
	void AddSummonMonsterGlyphStateEffectID(int monsterID, int glyphID, DWORD dwMonsterUniqueID, int stateEffectID);

	//Called when a skill crest is removed...
	void RemoveSummonMonsterGlyphInfo(int monsterID, int glyphID);
	void RemoveSummonMonsterStateEffectByGlyph(DnMonsterActorHandle hMonsterActor, int monsterID, int glyphID);

	//When monster dies, remove all registered effect IDs..
	void RemoveSummonMonsterGlyphStateEffects(int monsterID);

	bool GetSummonMonsterGlyphInfo(int monsterID, std::list<int>& glyphIDs);

	//Find summoned monsters with the same monsterID.
	void FindSummonMonster(int monsterID, std::list<DnMonsterActorHandle>& monsterActorList);
#endif // PRE_ADD_65808


#if defined(PRE_FIX_68898)
public:
	void SetSkipEndAction(bool isSkipEndAction);
#endif // PRE_FIX_68898
};

class CDnActorActionStateCache:public CSingleton<CDnActorActionStateCache>
{
public:

	void LoadAction( CDnActor* pActor, const char* szFullPathName, std::vector<CEtActionBase::ActionElementStruct*>* pVecActionElementList,	\
					 std::vector<std::vector<CDnActor::PreStateStruct>>& ActionState,														\
					 std::vector<std::vector<CDnActor::PreStateStruct>>& ActionCustomState,													\
					 std::vector<std::vector<CDnActor::PreStateStruct>>& CanMoveState );

private:

	typedef struct tagData
	{
		std::vector<std::vector<CDnActor::PreStateStruct>> VecPreActionState;
		std::vector<std::vector<CDnActor::PreStateStruct>> VecPreActionCustomState;
		std::vector<std::vector<CDnActor::PreStateStruct>> VecPreCanMoveState;
	}SData;

	CSyncLock m_Lock;
	std::map<std::string,SData>	m_mRepository;
};
