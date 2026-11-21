#pragma once

#include "DnUnknownRenderObject.h"
#include "EtMatrixEx.h"
#include "DnActionBase.h"
#include "DnActor.h"
#include "Timer.h"
#include "DnWeapon.h"
#include "DnCalcCatmullrom.h"
#include "DnSkill.h"

class CDnProjectile;
class CDnWeapon;
class IDnOrbitProcessor;

class CDnProjectile : public CDnWeapon, public TBoostMemoryPool<CDnProjectile>
{
public:
	CDnProjectile( CMultiRoom *pRoom, DnActorHandle hActor, bool bProcess = true, bool bIncreaseUniqueID = true );
	virtual ~CDnProjectile();

	// Trajectory type
	enum OrbitTypeEnum {
		Linear,        // Constant velocity SetSpeed
		Acceleration,  // Acceleration SetSpeed, SetResistance
		Homing,		   // Homing SetSpeed
		TerrainLinear, // Constant-speed SetSpeed along ground
		Projectile,    // Parabolic SetSpeed
		TerrainHoming, // Constant-speed homing SetSpeed along ground
		OrbitTypeCount,
	};

	// Type on expiration
	enum DestroyOrbitTypeEnum {
		Instantly,	       // Disappear immediately
		FallGravity,       // Fall under gravity
		RangeFallGravity,  // Basically same as FallGravity, but destroys immediately when range is reached (client executes Destroy action).
	};

	// Type where the projectile can no longer deal damage
	enum ValidTypeEnum {
		WeaponLength = 0x01, // When flown beyond weapon length
		Time = 0x02,		 // After a certain time
		Stick = 0x04,		 // When stuck in the ground or a Prop
	};

	// Target type
	enum TargetTypeEnum
	{
		CrossHair = 0,  // Crosshair
		Direction,	    // Actor's View direction
		TargetPosition, // Target position
		Target,         // Target
		DestPosition,   // Destination hardcoded in the action tool
		Shooter,		// Target type is set to Target, referencing the actor that fired the projectile. Currently only used for monsters, so implemented only up to that.
		TargetTypeCount,
	};

	// Acceleration/constant-velocity type. Only valid when Orbit type is Projectile or Homing.
	enum VelocityTypeEnum
	{
		Constant,	// Default. Projectile flies at constant velocity.
		Accell,		// Projectile receives acceleration.
		VelocityTypeCount,
	};


protected:
	IDnOrbitProcessor* m_pOrbitProcessor;

	static int s_iHitUniqueID;

	int m_iHitUniqueID;

	DnActorHandle m_hShooter;
	MatrixEx m_Cross;
	MatrixEx m_OffsetCross;
	EtVector3 m_vPrevPos;
	EtVector3 m_vStartPos;
	EtVector3 m_vHitPos;
	LOCAL_TIME m_InvalidLocalTime;
	LOCAL_TIME m_CreateLocalTime;
	
	OrbitTypeEnum m_OrbitType;
	DestroyOrbitTypeEnum m_DestroyOrbitType;
	TargetTypeEnum m_TargetType; 
	ValidTypeEnum m_ValidType;

	CDnDamageBase::SHitParam m_HitParam;

	bool m_bValidDamage;
	bool m_bFallGravity;

	bool m_bStick;
	float m_fGravity;

	float m_fSpeed;
	float m_fResistance;
	int m_nValidTime;
	int m_nDestroyOrbitTimeGap;
	bool m_bFirstProcess;

	DnSkillHandle m_hParentSkill;

	std::vector<CDnSkill::StateEffectStruct> m_VecStateEffectList;
	std::vector<CDnSkill::StateEffectStruct> m_ComboLimitStateEffectList;

	HitStruct m_HitStruct;
	DnActorHandle m_hTargetActor;
	EtVector3 m_vTargetPosition;

	CDnCalcCatmullRom m_CatmullRom;
	CDnHoming m_Homing;

	bool m_bOnCollisionCalled;

	bool m_bPierce;

	LOCAL_TIME			m_LastHitSignalEndTime;

	// For explosive projectiles, do not deal explosion damage to a target already hit once..
	vector<DnActorHandle> m_VecHittedActor;

	CDnSkill::SkillInfo m_ParentSkillInfo;

	bool m_bHasHitAction;		// For a projectile that has a hit action (explosion) 
	bool m_bHasHitSignalInHitAction;	// Whether the Hit action has a Hit signal.
	bool m_bHasHitSignalInShootAction;	// Whether Shoot action has a Hit signal.
	bool m_bHitSignalProcessed;
	bool m_bBombHitSignalStarted;	// 2009.7.29 hanki - Since the hit signal can follow after an explosion, turn on the flag at the start of the hit signal.
	bool m_bHitActionStarted;
	int m_iShootActionIndex;
	int m_iHitActionIndex;
	int m_iDestroyActionIndex;
	
	bool m_bHasDestroyAction;

	VelocityTypeEnum m_VelocityType;

	LOCAL_TIME m_nLastHitSignalIndex;
	
	float m_fElapsedTime;
	int m_nMaxHitCount;		// Max hit count valid only for piercing type (bPierce == true).
	map<CDnActor*, int> m_mapMaxHitCount;

	// Hold the pointer to the creation info.
	int m_iShooterShootActionIndex;	// Action index that fired this projectile
	int m_iSignalArrayIndex;	// Array index of this projectile signal
	boost::shared_ptr<ProjectileStruct> m_pProjectileSignal;
	boost::shared_ptr<CDnState> m_pShooterState;
#ifdef PRE_ADD_PROJECTILE_SE_INFO
	boost::shared_ptr<CDnStateBlow> m_pShooterStateBlow;
#endif

	// Actor that fired this projectile under the actual state effect, used only for chain attack state effect
	DnActorHandle m_hPrevAttacker;
	// hit prob value defined in the skill table used by the chain attack state effect.
	float m_fHitApplyPercent;
	// 

	// Whether the currently exploding projectile uses an area hit signal. (ex) summon blackhole
	bool m_bProcessingBombHitSignal;

	// If the projectile was fired from a prop, record it and exclude it from collision checks.
	DnPropHandle m_hShooterProp;

	bool m_bFromCharger;

	float m_fProjectileOrbitRotateZDegree;

	// Packet stream object is not created until a packet is requested.
	char* m_pPacketBuffer;
	CPacketCompressStream* m_pPacketStream;

	// Feature to follow the hit target. (#22666)
	bool m_bTraceHitTarget;
	bool m_bTraceHitActorHittable;
	DnActorHandle m_hTraceActor;
	//////////////////////////////////////////////////////////////////////////

	// Direction forced from outside. Currently used by cannon. (Only used in monster projectile packets..)
	EtVector3 m_vForceDir;
	bool m_bUseForceDir;

	char m_cShooterType;
	DWORD m_dwShooterUniqueID;
	INT64 m_nShooterSerialID;
	int m_nShooterActionIndex;
	int m_nShooterSignalIndex;

	ElementEnum m_eForceHitElement;

	// #41382 For monster-summon projectiles, apply forced skill level override if specified for monsters summoned by state effect 167.
	// When the projectile hits the ground and spawns, state effect 167 may already have been removed. 
	// (When a skill action ends due to moving or being hit and the self-applied state effect is removed)
	int m_iSummonMonsterForceSkillLevel;

protected:
	void MakeHitParam( CDnDamageBase::SHitParam &HitParam );

	void ProcessOrbit( LOCAL_TIME LocalTime, float fDelta );
	void ProcessValid( LOCAL_TIME LocalTime, float fDelta );
	bool ProcessDamage( LOCAL_TIME LocalTime, float fDelta, 
						LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, 
						int nSignalIndex, BOOL bUseHitSignalArea );

	DnPropHandle _CheckPropCollision( EtVector3& vCenter, float fLength, float& fPropContactDistance, float& fPropContactTime );
	void ProcessDestroyOrbit( LOCAL_TIME LocalTime, float fDelta );

	void CheckAndApplyDamage( DnActorHandle hActor, bool bValidDamageGoOn = true, int* nHitCount = NULL );

	bool SetTraceActor( DnActorHandle hActor, bool bValidDamageGoOn );

public:
	OrbitTypeEnum GetOrbitType() { return m_OrbitType; }
	DestroyOrbitTypeEnum GetDestroyOrbitType() { return m_DestroyOrbitType; } 
	TargetTypeEnum GetTargetType() { return m_TargetType; } 

	MatrixEx *GetMatEx() { return &m_Cross; }
	
	void SetSpeed( float fValue ) { m_fSpeed = fValue; }
	float GetSpeed() { return m_fSpeed; }
	void SetResistance( float fValue ) { m_fResistance = fValue; }
	float GetResistance() { return m_fResistance; }
	void SetValidTime( int nValidTime ) 
	{ 
		m_nValidTime = nValidTime; 
		if( 0 < nValidTime )
			m_ValidType = (ValidTypeEnum)( m_ValidType | Time ); 
	};
	int GetValidTime( void ) { return m_nValidTime; };

	EtVector3 *GetTargetPosition() { return &m_vTargetPosition; }
	DnActorHandle GetTargetActor() { return m_hTargetActor; }

	void SetTargetActor( DnActorHandle hActor, bool bUpdateTargetPartsIndex = true );// { m_hTargetActor = hActor; };
	void SetTargetPosition( EtVector3 &vPos ) { m_vTargetPosition = vPos; }

	void SetValidType(ValidTypeEnum ValidType) { m_ValidType = ValidType; }
	void SetDestroyOrbitTime( int nValue ) { m_nDestroyOrbitTimeGap = nValue; }

	bool Initialize( MatrixEx &Offset, OrbitTypeEnum OrbitType, DestroyOrbitTypeEnum DestroyType, TargetTypeEnum TargetType );
	// When created directly via new instead of the static member function CreateProjectile... and actually fired and flying,
	// Like Initialize, this function must be called at the end after creating and configuring the projectile.
	bool PostInitialize( void );		

	void OnDamageSuccess( DnActorHandle hDamagedActor, CDnDamageBase::SHitParam &HitParam );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	// CDnActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

	void SetParentSkill( DnSkillHandle hParentSkill ) 
	{ 
		m_hParentSkill = hParentSkill;
		if (hParentSkill)
			m_ParentSkillInfo = *(hParentSkill->GetInfo());
		m_ParentSkillInfo.hSkillUser = m_hShooter; 
	};
	void SetParentSkillInfo( CDnSkill::SkillInfo& SkillInfo ) 
	{ 
		m_ParentSkillInfo = SkillInfo; 
		m_ParentSkillInfo.hSkillUser = m_hShooter; 
		m_iShooterShootActionIndex = m_ParentSkillInfo.iProjectileShootActionIndex;
		m_iSignalArrayIndex = m_ParentSkillInfo.iProjectileSignalArrayIndex;
		m_hPrevAttacker = m_ParentSkillInfo.hPrevAttacker;
	}

	void AddStateEffect( CDnSkill::StateEffectStruct &Struct );

	void OnCollisionWithActor( void );
	void OnCollisionWithGround( void );
	void OnCollisionWithProp( void );
	void OnChangedNextActionToHit( void );			// For projectiles that auto-explode because next action is set to hit in the shoot action in the action tool, process pending tasks when the action changes.

	void SetPierce( bool bPierce ) { m_bPierce = bPierce; };
	bool GetPierce( void ) { return m_bPierce; };

	void FromSkill( bool bFromSkill ) { m_HitParam.bFromProjectileSkill = bFromSkill; };
	bool IsFromSkill() { return m_HitParam.bFromProjectileSkill; }

	void SetVelocityType( VelocityTypeEnum VelocityType ) { m_VelocityType = VelocityType; };
	VelocityTypeEnum GetVelocityType( void ) { return m_VelocityType; };

	void SetMaxHitCount( int iMaxHitCount ) { m_nMaxHitCount = iMaxHitCount; };
	int GetMaxHitCount( void ) { return m_nMaxHitCount; };

	virtual void OnChangeAction( const char *szPrevAction );
	virtual void OnFinishAction( const char* szPrevAction, LOCAL_TIME time );

	void SetProjectileSignal( boost::shared_ptr<ProjectileStruct> pProjectileStruct ) { m_pProjectileSignal = pProjectileStruct; };
	boost::shared_ptr<ProjectileStruct> GetProjectileSignal( void ) { return m_pProjectileSignal; };
	
	void SetShootActionIndex( int iActionIndex ) { m_iShooterShootActionIndex = iActionIndex; };
	void SetSignalArrayIndex( int iSignalArrayIndex ) { m_iSignalArrayIndex = iSignalArrayIndex; };

	// Save only when fired by a skill.
	void SetShooterStateSnapshot( boost::shared_ptr<CDnState> pShooterState ) { m_pShooterState = pShooterState; };
	boost::shared_ptr<CDnState> GetShooterStateSnapshot( void ) { return m_pShooterState; };
#ifdef PRE_ADD_PROJECTILE_SE_INFO
	void SetShooterStateBlow( boost::shared_ptr<CDnStateBlow> pShooterStateBlow ) { m_pShooterStateBlow = pShooterStateBlow; };
	boost::shared_ptr<CDnStateBlow> GetShooterStateBlow( void ) { return m_pShooterStateBlow; };
#endif

	void SetHitApplyPercent( float fHitApplyPercent ) { m_fHitApplyPercent = fHitApplyPercent; };
	
	DnActorHandle GetShooterActor( void ) { return m_hShooter; };
	DnSkillHandle GetParentSkill( void ) { return m_hParentSkill; };

	void SetShooterProp( DnPropHandle hProp ) { m_hShooterProp = hProp; };

	void FromCharger( void ) { m_bFromCharger = true; };
	bool IsFromCharger( void ) { return m_bFromCharger; };

	void SetProjectileOrbitRotateZ( float fProjectileOrbitRotateZ ) { m_fProjectileOrbitRotateZDegree = fProjectileOrbitRotateZ; };
	float GetProjectileOrbitRotateZ( void ) { return m_fProjectileOrbitRotateZDegree; };

	void SetTraceHitTarget( bool bTraceHitTarget, bool bTraceHitActorHittable ) { m_bTraceHitTarget = bTraceHitTarget; m_bTraceHitActorHittable = bTraceHitActorHittable; };
	bool IsTraceHitTarget( void ) { return m_bTraceHitTarget; };
	bool IsTraceHitActorHittable( void ) { return m_bTraceHitActorHittable; };

	void SetForceDir( const EtVector3& vForceDir ) { m_vForceDir = vForceDir; m_bUseForceDir = true; };
	bool IsUsedForceDir( void ) { return m_bUseForceDir; };
	const EtVector3& GetForceDir( void ) { return m_vForceDir; };

	void SetShooterType( DnActorHandle hActor, int nActionIndex, int nSignalIndex );
	void SetShooterType( DnWeaponHandle hWeapon, int nActionIndex, int nSignalIndex );
	// Create projectile, build send packet from projectile object, etc.
	// Must also be updated whenever contents are added to the projectile signal.
	static CDnProjectile *CreateProjectile( CMultiRoom *pRoom, DnActorHandle hShooter, MatrixEx &LocalCross, ProjectileStruct *pStruct, EtVector3* pForceTargetPos=NULL, 
											DnActorHandle hForceTarget=CDnActor::Identity(), EtVector3* pForceDir = NULL );
	static CDnProjectile* CreatePlayerProjectileFromClientPacket( DnActorHandle hShooter, const BYTE* pPacket );
	CPacketCompressStream* GetPacketStream( void );

	const std::vector<CDnSkill::StateEffectStruct>& GetStateEffectList() { return m_VecStateEffectList; }
	void ApplyParentProjectile(CDnProjectile* pParentProjectile);

protected:
	int m_nTargetPartsIndex;		//Selected parts index if it is a Parts monster
	int m_nTargetPartsBoneIndex;	//Index holding actual Bone info in selected Parts.. (MonsterParts has a vector, and _Info has _PartsInfo as a vector)
public:
	void SetTargetPartsIndex(int nPartsIndex, int nBoneIndex) { m_nTargetPartsIndex = nPartsIndex; m_nTargetPartsBoneIndex = nBoneIndex; }
	int GetTargetPartsIndex() { return m_nTargetPartsIndex; }
	int GetTargetPartsBoneIndex() { return m_nTargetPartsBoneIndex; }

	void UpdateTargetPartsIndex();

	void SetForceHitElement( ElementEnum eElement ) { m_eForceHitElement = eElement; };

private:
	// Check the state effect settings.
	void GetChainAttackInfo(const CDnSkill::StateEffectStruct& stateEffectStruct, float& fRange, int& nMaxCount);
	//Search for next opponent
	DnActorHandle FindNextChainActor(int iRootAttackerTeam, DnActorHandle hActor, DnActorHandle hPrevActor, float fRange);
	void CreateChainAttackProjectile(DnActorHandle hRootAttacker, DnActorHandle hActor, DnActorHandle hActorToAttack, ProjectileStruct* pProjectileSignalInfo, CDnSkill::SkillInfo& parentSkillInfo);

public:
	void SetSummonMonsterForceSkillLevel( int iLevel ) { m_iSummonMonsterForceSkillLevel = iLevel; };

#if defined(PRE_FIX_52329)
protected:
	int m_nIgnoreHitType;	//

public:
	void SetIgnoreHitType(int nType) { m_nIgnoreHitType = nType; }
	int GetIgnoreHitType() { return m_nIgnoreHitType; }
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
protected:
	bool m_bHitActionVectorInit;

public:
	//Reset the Y vector while preserving the projectile's direction.
	void ChangeProjectileRotation();

	void SetHitActionVectorInit(bool bHitActionVectorInit) { m_bHitActionVectorInit = bHitActionVectorInit; }
	bool IsHitActionVectorInit() { return m_bHitActionVectorInit; }
#endif // PRE_ADD_55295

//////////////////////////////////////////////////////////////////////////
//#56880
//Set the start time of the projectile skill related to the hit count limit.
protected:
	LOCAL_TIME m_SkillStartTime;

public:
	void SetSkillStartTime(LOCAL_TIME startTime) { m_SkillStartTime = startTime; }
	LOCAL_TIME GetSkillStartTime() { return m_SkillStartTime; }
//////////////////////////////////////////////////////////////////////////

#if defined(PRE_FIX_59238)
//Handling to prevent Summon Puppet and owner actor from being hit simultaneously
//Store actors hit by the projectile in a list.
protected:
	std::map<DWORD, DnActorHandle> m_HittedActorList;

public:
	void AddHittedActor(DnActorHandle hHittedActor);
	bool IsHittable(DnActorHandle hActor);
#endif // PRE_FIX_59238

#if defined(PRE_FIX_59336)
	void ApplyComboLimitStateEffect( DnActorHandle hActor );
#endif // PRE_FIX_59336

	void ApplySkillStateEffect(DnActorHandle hActor);

#if defined(PRE_FIX_65287)
	//While state effect 50 is applied to the skill caster themselves, a projectile is fired, and
	//If the projectile's lifetime exceeds the skill's duration, the caster's state effect disappears,
	//and it can no longer affect the projectile's damage calculation.
	//When creating a projectile and saving the firing actor's stat info, if state effect #50 exists
	//Sum up the state effect settings, set them on the projectile, and use this info during projectile hit processing.
protected:
	float m_fShooterFinalDamageRate;

public:
	void SetShooterFinalDamageRate(float fRate) { m_fShooterFinalDamageRate = fRate; }
	float GetShooterFinalDamageRate() { return m_fShooterFinalDamageRate; }

#endif // PRE_FIX_65287
};


// Information needed to determine trajectory.
struct S_PROJECTILE_PROPERTY
{
	CDnProjectile::OrbitTypeEnum eOrbitType;
	CDnProjectile::TargetTypeEnum eTargetType;
	CDnProjectile::VelocityTypeEnum eVelocityType;
	float fSpeed;
	float fResistance;
	float fProjectileOrbitRotateZ;

	S_PROJECTILE_PROPERTY( void ) : eOrbitType( CDnProjectile::OrbitTypeCount ),
									eTargetType( CDnProjectile::TargetTypeCount ), 
									eVelocityType( CDnProjectile::VelocityTypeCount ),
									fSpeed( 0.0f ),
									fResistance( 0.0f ),
									fProjectileOrbitRotateZ( 0.0f ) {}
};