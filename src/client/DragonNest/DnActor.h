#pragma once

#include "DnUnknownRenderObject.h"
#include "EtMatrixEx.h"
#include "Timer.h"
#include "EtOctree.h"
#include "DnActionBase.h"
#include "DnActorState.h"
#include "MAEnvironmentEffect.h"
#include "MAActorRenderBase.h"
#include "MASkillUser.h"
#include "MATransAction.h"
#include "MAFaceAniBase.h"
#include "DnWeapon.h"
#include "DnBlowDef.h"
#include "DnActionSignalImp.h"
#include "MADamageBase.h"
#include "MAHeadNameRender.h"
#include "MAHeadLookController.h"
#include "DnHeadEffectRender.h"
#include "MAMovementInterface.hpp"

class CDnStateBlow;
class CDnProjectile;
struct EffectOutputInfo;

class CDnStateBlowSignalProcessor;

#define _SOCIAL_NEXT_ACTION_KALLISTE

#define STATEBLOWEFFECT_ETCOFFSET 10000000

class CDnActor : public CDnUnknownRenderObject< CDnActor >, 
				 //virtual public CDnActionBase, 
				 virtual public CDnActorState, 
				 public MAEnvironmentEffect, 
				 public MASkillUser, 
				 public MATransAction,
				 public MAMovementInterface,
				 virtual public MAActorRenderBase, 
				 virtual public MADamageBase,
				 public CEtCustomRender, 
				 public CDnActionSignalImp,
				 public MAHeadNameRender,
				 public MAHeadLookController,
				 public CDnHeadEffectRender
{
public:
	CDnActor( int nClassID, bool bProcess = true );
	virtual ~CDnActor();

	enum HitCheckTypeEnum {
		BoundingBox,
		Collision,
	};

	enum DamageCheckTypeEnum {
		DifferentTeam, // If on a different team
		Anyone_AllDamage, // Anyone.. same damage
		Anyone_DifferentTeam, // Anyone.. deal damage only when on a different team.
	};

	enum SocialActionEnum {
		Laugh,
		Smile,
		Serious,
		Pain,
		Sad,
		Shy,
		Normal,
		Count,
	};

	enum FXDummyBoneEnum
	{
		FX_01,
		FX_02,
		FX_03,
		FX_04,
		FX_05,
		FX_06,
		FX_07,
		FX_08,
		FX_09,
		FX_10,
		FX_11,	//Added for prefix
		FX_12,
		FX_13,
		FX_14,
		Max_FX_Dummy_Bone,
	};

	enum APPEAR_STEP {
		AS_NONE = 0,
		AS_APPEAR = 1,
		AS_SUSPEND = 2,
		AS_DISAPPEAR = 3,
	};

	static DnActorHandle s_hLocalActor;
	static std::map<DWORD, DnActorHandle> s_dwMapActorSearch;
	static CSyncLock s_LockSearchMap;

	long m_lBreakIntoLoadingFlag;

	static int s_nFontIndex;
	static float s_fDieDelta;
	static DWORD s_dwUniqueCount;
	static const char* s_acEffectBoneName[ Max_FX_Dummy_Bone ];

protected:
	static CEtOctree<CDnActor*> *s_pOctree;
	CEtOctreeNode<CDnActor*> *m_pCurrentNode;


	// Identification Values
	DWORD m_dwUniqueID;
	int m_nClassID;
	int m_nTeam;


	// Position Values
	MatrixEx m_matexWorld;
	EtVector3 m_vStaticPos;
	EtVector3 m_vPrevPos;
	float m_fAddHeight;


	// Visual Values 
	bool m_bShow;
	bool m_bShowHeadName;

	APPEAR_STEP m_SoftAppearStep;
	LOCAL_TIME	m_SoftTime;

	std::map< int , EtSwordTrailHandle > m_TrailList;

	DnWeaponHandle m_hWeapon[ CDnWeapon::EquipSlot_Amount ];
	bool m_bSelfDeleteWeapon[ CDnWeapon::EquipSlot_Amount ];
	std::string m_szAdditionalWeaponAction;

#ifdef _SOCIAL_NEXT_ACTION_KALLISTE
	struct SocialActionCtrlStruct
	{
		int nCondition;
		std::string szNextActionName;

		SocialActionCtrlStruct()
		{
			nCondition = 0;
		}

		void Clear()
		{
			nCondition			= 0;
			szNextActionName.clear();
		}

		bool IsEmpty() const { return (nCondition == 0); }
	};

	SocialActionCtrlStruct m_SocialActCtrler;
#endif

#ifdef _SHADOW_TEST
	bool m_bIsShadowActor;
	bool m_bEnableShadowActor;
	DnActorHandle m_hShadowActor;
#endif

	// Skill / StatBlow Values
	CDnStateBlow *m_pStateBlow;
	DnActorHandle m_hChargetDestActor;
	
	map<int, set<string> > m_mapPassiveSkillActions;
	std::vector<DnEtcHandle> m_vecBindActionObjectHandle;





	int m_nSwapActorID;
#if !defined( PRE_ADD_ACTION_OBJECT_END_ACTION )
	map<int, vector<CDnActionSignalImp*> > m_mapBindingActionObjects;
#endif

	std::set<CDnSkill::SkillInfo>		m_setNowPlayingEffectSkill;
	static std::vector<CDnPlayerActor *>	s_pVecDeleteList;

	SHitParam m_HitParam;
	HitCheckTypeEnum m_HitCheckType;
	DamageCheckTypeEnum m_DamageCheckType;

	bool m_bModifyPlaySpeed;
	float m_fPlaySpeed;
	DWORD m_dwPlaySpeedTime;
	LOCAL_TIME m_PlaySpeedStartTime;

	LOCAL_TIME m_LastDamageTime;
	LOCAL_TIME m_LastHitSignalTime;
	int m_nLastHitSignalIndex;
	
	bool m_bUseSignalSkillCheck;
	bool m_bHideExposureInfo[2];
	bool m_bOnSignalFromChargerSE;
	bool m_bIngnoreNormalSuperArmor; 	// Whether to ignore normal super armor (used on client for groggy state check) 

	int m_iCantXZMoveSE;
	int m_iFrameStopRefCount;

	int m_iCantMoveReferenceCount;
	int m_iCantActionReferenceCount;
	int m_iCantUseSkillReferenceCount;
	int m_iCantAttackReferenceCount;

	int m_nSuperAmmorTime;
	int	m_nCurrSuperArmor;
	int m_nMaxSuperArmor;

	float m_fStiffDelta;
	float m_fDownDelta;
	float m_fDieDelta;
	float m_fMaxDieDelta;

#ifdef PRE_ADD_GRAVITY_PROPERTY
	float m_fGravityEnd;
#endif // PRE_ADD_GRAVITY_PROPERTY

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



	// Signal Rotate.
	LOCAL_TIME m_dwRotateStartTime;
	LOCAL_TIME m_dwRotateTime;	
	bool m_bRotate;
	bool m_bRotLeft;	  // Rotation direction.	
	float m_fSpeedRot;    // Rotation speed.
	float m_fStartSpeed;  // Initial speed.
	float m_fEndSpeed;    // End speed.
	EtVector3 m_vRotAxis; // Rotation axis coordinates.



protected:    //  PROTECTED FUNCTIONS  //

	void PreCalcStateList();
	void CalcCustomRenderDepth();

	void LinkWeapon( int nEquipIndex );

	void ProcessPress( DnActorHandle hTarget );
	void ProcessPlaySpeed( LOCAL_TIME LocalTime, float fDelta );
	void ProcessSoftAppear( LOCAL_TIME LocalTime );
	void ProcessRotate( LOCAL_TIME LocalTime, float fDelta ); // Rotate.
		
	// Signal Rotate.
	void SetRotate( DWORD dwTime, float fStart, float fEnd, EtVector3 & vPos, bool bLeft );

	// Handle index distinguishing Effect output
	bool _CheckEffectDuplicate( DnSkillHandle hNewSkill );
	const char *CheckChangeActionBlow( const char* szActionName );

	bool CheckPressCircle2Clrcle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );		// 2D circle - 2D circle
	bool CheckPressCapsule2Capsule2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );	// 2D capsule - 2D capsule
	bool CheckPressCapsule2Circle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist );	// 2D capsule - 2D circle

	// Load-related
	static int __stdcall OnReleaseActorUniqueID( void *pParam, int nSize );
	static bool __stdcall OnReleaseActorCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime );
	static bool __stdcall OnCheckLoadingPacket( void *pThis, UINT nUniqueID, int nMainCmd, int nSubCmd, void *pParam, int nSize );

	static bool Compare_PreStateOrder( CDnActor::PreStateStruct &a, CDnActor::PreStateStruct &b );

	virtual void OnDie( DnActorHandle hHitter );
	virtual void OnRestorePlaySpeed( float fPrevFPS ) {}
	virtual void OnDamageActor( DnActorHandle hHitter, SHitParam &HitParam );

	virtual void ProcessState( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessDown( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessDie( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessStiff( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessPress();


public:   //  PUBLIC FUNCTIONS  //


	// Identifications

	int GetClassID() { return m_nClassID; }
	int GetTeam() { return m_nTeam; }
	DWORD GetUniqueID() { return m_dwUniqueID; }
	virtual int OnGetJobClassID(){ return 0; }
	virtual DWORD GetSummonerUniqueID( void ) { return UINT_MAX; };
	
	void SetUniqueID( DWORD dwUniqueID );
	void SetTeam( int nValue ) { m_nTeam = nValue; }

	virtual bool IsPlayerActor() { return false; }
	virtual bool IsNpcActor(){ return false; }
	virtual bool IsMonsterActor(){ return false; }
	virtual bool IsVehicleActor() { return false; }
	virtual bool IsPartsMonsterActor(){ return false; }
	virtual bool IsPetActor() { return false; }
#ifdef PRE_ADD_CHECK_PART_MONSTER
	virtual bool IsEnablePartsHPDisplay() { return false; }
#endif

	// Primary

	LOCAL_TIME GetLocalTime() { return CDnActionBase::m_LocalTime; }

	virtual DnActorHandle GetActorHandle( void ) { return GetMySmartPtr(); };
	static DnActorHandle GetLocalActor() { return s_hLocalActor; }

	virtual void ResetActor();
	virtual bool Initialize();

	virtual void SetProcess( bool bFlag );
	virtual void SetDestroy();
	virtual void PreThreadRelease();
	
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessLook( LOCAL_TIME LocalTime, float fDelta ){}
	virtual void ProcessAI( LOCAL_TIME LocalTime, float fDelta ){}
	virtual bool SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bCheck = true, bool bCheckStateEffect = true );
	virtual void SetCustomAction( const char *szActionName, float fFrame );
	virtual void SyncClassTime( LOCAL_TIME LocalTime );

	void ProcessActor( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessVisual( LOCAL_TIME LocalTime, float fDelta );

	// Visual

	virtual void Show( bool bShow );
	virtual bool IsShow() { return m_bShow; }

	void ShowHeadName( bool bShow ) { m_bShowHeadName = bShow; }

	void SetSoftAppear( bool bSoft ) { m_SoftAppearStep = ( bSoft ? AS_APPEAR : AS_NONE ); }
	void ProcessAppear( LOCAL_TIME LocalTime  );

#ifdef _SHADOW_TEST
	void EnableShadowActor( bool bValue ) { m_bEnableShadowActor = bValue; }
	bool IsEnableShadowActor() { return m_bEnableShadowActor; }
	void SetShadowActor( bool bValue ) { m_bIsShadowActor = bValue; }
	bool IsShadowActor() const { return m_bIsShadowActor; }

	DnActorHandle GetShadowActorHandle() { return m_hShadowActor; }
#endif _SHADOW_TEST



	// Position 

	MatrixEx *GetMatEx() { return &m_matexWorld; }

	EtVector3 *GetPosition() { return &m_matexWorld.m_vPosition; }
	EtVector3 *GetStaticPosition();
	EtVector3 *GetPrevPosition();

	virtual void SetPosition( EtVector3 &vPos );
	void SetStaticPosition( EtVector3 &vPos );
	void SetPrevPosition( EtVector3 &vPos );

	float GetAddHeight() { return m_fAddHeight; }
	void SetAddHeight( float fValue );

	// For Octree..

	void InsertOctreeNode();
	void RemoveOctreeNode();

	virtual void GetBoundingSphere( SSphere &Sphere, bool bActorSize = false );
	virtual void GetBoundingBox( SAABox &Box );

	EtVector3 GetHeadPosition();
	EtVector3 GetBodyPosition();

	virtual void ProcessCollision( EtVector3 &vMove );
	virtual DWORD GetDamageObjectUniqueID() { return GetUniqueID(); }

#ifdef RENDER_PRESS_hayannal2009
protected:
	static bool s_bEnableRenderPress;
public:
	static void SetRenderPress(bool bEnable) { s_bEnableRenderPress = bEnable; }
	void RenderPress();
#endif



	// Status
	
	void SetPlaySpeed( DWORD dwFrame, float fSpeed );
	void ResetPlaySpeed();
	bool IsModifyPlaySpeed() { return m_bModifyPlaySpeed; }
	DWORD GetPlaySpeedFrame() { return m_dwPlaySpeedTime; }
	//void ModifyPlaySpeedDuration( DWORD dwNewTimeGap );

	void SetDieDelta(float fDelta) { m_fDieDelta = fDelta; }
	void SetMaxDieDelta(float fDelta) { m_fMaxDieDelta = fDelta; }

	virtual int GetState( const char *szActionName = NULL );
	virtual bool IsStay( const char *szActionName = NULL );
	virtual bool IsMove( const char *szActionName = NULL );
	virtual bool IsAttack( const char *szActionName = NULL );
	virtual bool IsHit( const char *szActionName = NULL );
	virtual bool IsAir( const char *szActionName = NULL );
	virtual bool IsDown( const char *szActionName = NULL );
	virtual bool IsStun( const char *szActionName = NULL );
	virtual bool IsStiff( const char *szActionName = NULL );
	virtual bool IsFly( const char *szActionName = NULL );
	virtual bool IsGround( const char *szActionName = NULL );
	virtual bool IsUnderGround( const char* szActionName = NULL );
	virtual bool IsLifeSkill( const char* szActionName = NULL );

	bool IsStandHit( const char *szActionName = NULL );
	bool IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal = NULL );


	int GetCurrentSuperArmor() { return m_nCurrSuperArmor; }
	int GetMaxSuperArmor() { return m_nMaxSuperArmor; }

#ifdef PRE_ADD_MONSTER_PARTS_UI_TRIGGER
	void SetCurrentSuperArmor( int nSuperArmor ){ m_nCurrSuperArmor = nSuperArmor; }
	void SetMaxSuperArmor( int nSuperArmor ) { m_nMaxSuperArmor = nSuperArmor; }
#endif 

	// Whether to ignore normal super armor; ignore if true.
	void SetIgnoreNormalSuperArmor(bool bIgnore) { m_bIngnoreNormalSuperArmor = bIgnore; }
	bool GetIgnoreNormalSuperArmor() { return m_bIngnoreNormalSuperArmor; }



	// ETC

	void OnInitializeNextStage( void );
	virtual void OnInitializePVPRoundRestart( void );

	virtual void SetSocialAction( SocialActionEnum SocialEnum ) {};
#ifdef _SOCIAL_NEXT_ACTION_KALLISTE
	void SetSocialCtrl(int nCondition, const char* szNextActionName);
#endif

	// Hit

	SHitParam *GetHitParam() { return &m_HitParam; }
	HitCheckTypeEnum GetHitCheckType() { return m_HitCheckType; }

	// Damage

	void SetDamageCheckType( DamageCheckTypeEnum Type ) { m_DamageCheckType = Type; }
	DamageCheckTypeEnum GetDamageCheckType() { return m_DamageCheckType; }
	LOCAL_TIME GetDamageRemainTime() { return m_LastDamageTime; }
	
	// from CDnDamageBase
	virtual void OnComboCount( int nValue, int nDelay ) {}
	virtual bool bIsAILook(){ return false; }


	// Movement

	virtual void OnClash( EtVector3 &vPosition, OnClashFloorCheckType bFloorClash );

	void SetDownRemainDelta( float fDownDelta ) { m_fDownDelta = fDownDelta; };
	void SetStiffDelta( float fStiffDelta ) { m_fStiffDelta = fStiffDelta; };
	
	float GetDownRemainDelta() { return m_fDownDelta; }
	float GetStiffDelta() { return m_fStiffDelta; };

	void GetProperlyDieAction( /*IN OUT*/ string& strDieAction );



	// Action
	
	void UpdateFPS( void );

	bool CheckAnimation(const char* szActionName);

	void AddedFrameStop( void );
	void RemovedFrameStop( void );
	int GetFrameStopRefCount( void ) { return m_iFrameStopRefCount; };

	void SetAdditionalWeaponAction( std::string &szStr ) { m_szAdditionalWeaponAction = szStr; }
	
	std::string &GetAdditionalWeaponAction() { return m_szAdditionalWeaponAction; }
	virtual void OnResetAttachWeaponAction( DnWeaponHandle hWeapon, int nEquipIndex ) {}

	// Note: Terminate skill on action change timing, since another action can now trigger mid-skill.
	// End the skill to accurately match the end timing of state effect application.
	// Client doesn't judge damage and only handles skill visuals, but added to stay in sync with the server
	virtual void OnChangeAction( const char *szPrevAction );

	//Moved handling from MonsterActor's OnChangeAction to CDnActor (previously used only by MonsterActor, now used by Kali too).
#if !defined( PRE_ADD_ACTION_OBJECT_END_ACTION )
	void AddBindActionObject( CDnActionSignalImp* pActionObject ) { m_mapBindingActionObjects[ m_nActionIndex ].push_back( pActionObject ); };
#endif
	void ReleaseBindActionObject(const char* szPrevAction);
	void AddBindeActionObjectHandle( DnEtcHandle hHandle );
	void SyncBindActionObjectFPS( float fFPS );

	// ActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual bool LoadAction( const char *szFullPathName );
	virtual void FreeAction();
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	static void CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase );
#endif
	virtual void OnLoopAction( float fFrame, float fPrevFrame );

	// Actor Command
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true, bool bFromStateBlow = false, bool bSkillChain = false );
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMove( DnActorHandle hActor, float fMinDistance, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdMove( EtVector2 &vDir, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bResetStop = false, bool bForce = false ) {}
	virtual void CmdLook( EtVector2 &vVec, bool bForce = false ) {}
	virtual void CmdLook( DnActorHandle hActor, bool bLock = true ) {}
	// Note: bOnPlayerInit means called when adding passive skills during initial stage load character creation and initialization
	//		 IsDie() is checked inside AddStateBlow when character is initialized during stage loading
	//		 Use this flag to distinguish because passive skill state effect is skipped when HP is 0
	virtual DnBlowHandle CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkillInfo, 
											STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit = false );
	virtual void CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bSendServer = false );
	virtual void CmdRemoveStateEffectImmediately( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	virtual void CmdSuicide( bool bDropItem, bool bDropExp ) {}
	virtual void CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, int nLoopCount = 0, float fFrame = 0.f, float fBlendFrame = 3.f );
	virtual void CmdWarp( EtVector3 &vPos, EtVector2 &vLook );
	virtual void CmdShowExposureInfo( bool bShow ) { m_bHideExposureInfo[1] = !bShow; }
	bool IsShowExposureInfo() { return ( !m_bHideExposureInfo[0] && !m_bHideExposureInfo[1] ); }

	// Actor Message
#ifdef PRE_FIX_77172
	virtual void CheckDamageVelocity( DnActorHandle hActor, bool bKeepJumpMovement = false );
#else // PRE_FIX_77172
	virtual void CheckDamageVelocity( DnActorHandle hActor );
#endif // PRE_FIX_77172
	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor ) {}

	// CDnDamageBase
	virtual void OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam );

	// EtCustomRender
	virtual void RenderCustom( float fElapsedTime );

	// CDnActionSignalImp
	virtual MatrixEx *GetObjectCross() { return &m_matexWorld; }
	virtual CDnRenderBase *GetRenderBase();
	virtual EtAniObjectHandle GetAniObjectHandle() { return m_hObject;}

	// Weapon
	virtual void AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex = 0, bool bDelete = false );
	virtual void DetachWeapon( int nEquipIndex = 0 );
	virtual DnWeaponHandle GetWeapon( int nEquipIndex = 0, bool bActionMatchWeapon = true ) { return m_hWeapon[nEquipIndex]; }
	virtual void ShowWeapon( int nEquipIndex, bool bShow );
	virtual DnWeaponHandle GetActiveWeapon( int nEquipIndex ) { return m_hWeapon[nEquipIndex]; }
	virtual void SetWeaponFPS( float fValue );
	virtual void ReleaseWeaponSignalImp();

	// Projectile is currently used only in CDnLocalPlayerActor.
#ifdef PRE_MOD_PROJECTILE_HACK
	virtual void OnProjectile( CDnProjectile *pProjectile ) {}
#else
	virtual void OnProjectile( CDnProjectile *pProjectile, int nSignalIndex ) {}
#endif

	// TransAction Message
	virtual void OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket );

	// Additional Packet Message
	// Damage Packet
	virtual void OnDamageWriteAdditionalPacket( CPacketCompressStream *pStream ) {}
	virtual void OnDamageReadAdditionalPacket( CPacketCompressStream *pStream ) {}

	// Static functions
	static bool InitializeClass();
	static void ProcessClass( LOCAL_TIME LocalTime, float fDelta );
	static void ProcessAIClass( LOCAL_TIME LocalTime, float fDelta );
	static void ProcessFlushPacketQueue();
	static void ReleaseClass();
	static DnActorHandle FindActorFromUniqueID( DWORD dwUniqueID );
	static DnActorHandle FindActorFromName( TCHAR *szName );
	static DnActorHandle FindActorFromUniqueIDBySmartPtrList( DWORD dwUniqueID );
	static void InsertUniqueSearchMap( CDnActor *pActor );
	static void RemoveUniqueSearchMap( CDnActor *pActor );
	static void InitializeNextStage();
	static void DeleteAllActor();
	
	// for Debug
	static void AllMonsterKill();
	static void	SetAllMonsterDebugDrawing(bool bEnableCollision, bool bEnableBound);


	// Range Scan functions
	static int ScanActor( EtVector3 &vPos, float fRadius, DNVector(CDnActor*) &VecList );
	static int ScanActor( EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList );
	static int ScanActorByActorSize( EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside = false, bool bActorSize = false );



	// StateBlow 

	bool ProcessIgnoreGravitySignal();

	CDnStateBlow *GetStateBlow() { return m_pStateBlow; }
	DnBlowHandle GetStateBlowFromID( int nStateBlowID );

	// Note: bOnPlayerInit means called when adding passive skills during initial stage load character creation and initialization
	//		 IsDie() is checked inside AddStateBlow when character is initialized during stage loading
	//		 Use this flag to distinguish because passive skill state effect is skipped when HP is 0
	int AddStateBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szValue, bool bOnPlayerInit = false );

	virtual void OnBeginStateBlow( DnBlowHandle hBlow );
	virtual void OnEndStateBlow( DnBlowHandle hBlow );

	void ForceBeginStateBlow(DnBlowHandle hBlow);


	int RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex );
	void RemoveStateBlowFromID( int nStateBlowID );
	void RemoveAllBlow();
	void RemoveAllBlowExpectPassiveSkill();

	DnBlowHandle GetAppliedStateBlow( int iIndex );
	int GetNumAppliedStateBlow( void );
	void GetAllAppliedStateBlow( DNVector(DnBlowHandle)& /*IN OUT*/ out );
	void GatherAppliedStateBlowByBlowIndex( STATE_BLOW::emBLOW_INDEX BlowIndex, DNVector(DnBlowHandle)& /*IN OUT*/ out );
	void GetAllAppliedStateBlowBySkillID(int nSkillID, DNVector(DnBlowHandle)& /*IN OUT*/ out);
	bool IsAppliedThisStateBlow( STATE_BLOW::emBLOW_INDEX iBlowIndex );


	void SetOnSignalFromChargerSE( bool bOnSignalFromChargerSE ) { m_bOnSignalFromChargerSE = bOnSignalFromChargerSE; };
	bool IsOnSignalFromChargerSE( void ) { return m_bOnSignalFromChargerSE; };
	void SetChargerDestActor( DnActorHandle hChargerDestActor ) { m_hChargetDestActor = hChargerDestActor; };

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

	bool DetachSEEffectSkillInfo( const CDnSkill::SkillInfo& ParentSkillInfo );

	bool IsShouldRemainBlowWhenDie( DnBlowHandle hBlow );

	virtual bool AttachSEEffect( const CDnSkill::SkillInfo& ParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nIndex, const EffectOutputInfo* pEffectInfo, 
		/*OUT*/ DnEtcHandle& hEtcObject, /*OUT*/ DNVector(DnEtcHandle)& vlhDummyBoneEtcObjects, bool bAllowSameSkillDuplicate = false );
	virtual bool DetachSEEffect( const CDnSkill::SkillInfo& ParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nIndex, const EffectOutputInfo* pEffectInfo );

	// Skill
	void ResetProcessSkill();

	void CmdFinishAuraSkill(DWORD dwSkillID);

	bool bIsAppliedSkill( const int iSkillIndex );
	bool IsUseSignalSkillCheck( void ) { return m_bUseSignalSkillCheck; };

	void MakePassiveSkillActionInfo( void );
	void FillPassiveSkillActions( int iSkillID, set<string>& setUseActionNames );

	//#if defined(SkillCooldownRatio)
	void FindSkillBySkillType(CDnSkill::SkillTypeEnum eSkillType, DNVector(DnSkillHandle)& vlSkillList);
	//#endif // Skill cooldown ratio

	// To check the moment MP is consumed in a state effect... currently used only on the server.
	// Used by state effects that decrease or increase MP consumption. 
	// MP consumed through this function receives a separate packet from the server, so nothing to do on client!
	void UseMP( int iUseMP ) {};


#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	void FlushSoundSignal();
	void PreloadSkillSoundSignal();
#endif
#ifdef PRE_ADD_GRAVITY_PROPERTY
	void SetGravityEnd( float fGravityEnd ) { m_fGravityEnd = fGravityEnd; }
#endif // PRE_ADD_GRAVITY_PROPERTY


protected:
	CDnStateBlowSignalProcessor* m_pStateBlowSignalProcessor;
public:
	void AddStateBlowSignal(DnBlowHandle hBlow);
	void RemoveStateBlowSignal(DnBlowHandle hBlow);


protected:
	int m_nOriginalTeam;
public:
	void SetOriginalTeam(int nTeam) { m_nOriginalTeam = nTeam; }
	int GetOriginalTeam() { return m_nOriginalTeam; }

public:
	void ResetStateEffects();	//State effect visuals disappear after cutscene, so recreate state effect visuals when cutscene ends.
	void ActionChange(int nActionIndex);

	void CreateChainAttackProjectile(CPacketCompressStream& Stream);

	void RequestSummonOff(DWORD dwSummonMonsterUniqueID);

	virtual bool IsWaitingThread(){return false;}

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

#if defined(PRE_ADD_50907)
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
#endif // PRE_ADD_50907

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

#if defined(PRE_FIX_57706)
protected:
	int m_nStickAniDlgRefCount;
public:
	void AddStickAniDlgRefCount() { ++m_nStickAniDlgRefCount; }
	void RemoveStickAniDlgRefCount() { --m_nStickAniDlgRefCount; }
	int GetStickAniDlgRefCount() { return m_nStickAniDlgRefCount; }
#endif // PRE_FIX_57706

#if defined(PRE_FIX_61382)
	//For puppet monsters, display the status effect on the owner actor.
	static DnActorHandle GetOwnerActorHandle(DnActorHandle hActor);
#endif // PRE_FIX_61382

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
protected:
	bool m_isMailBoxOpenByShortCutKey;

public:
	void SetMailBoxOpenByShortCutKey(bool isOpen) { m_isMailBoxOpenByShortCutKey = isOpen; }
#endif // PRE_ADD_MAILBOX_OPEN

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

#endif // PRE_ADD_65808

#if defined(PRE_FIX_68898)
public:
	virtual void SetSkipEndAction(bool isSkipEndAction);
#endif // PRE_FIX_68898


	void TransformToActorID(int nActorIndex);
	void TransformToNormal();
};
