#pragma once

#pragma pack(push, 4)
// Signal type enumeration
enum SignalTypeEnum {
	STE_DnNullSignal = 0,
	STE_Input = 1,
	STE_CanMove = 2,
	STE_Hit = 4,
	STE_VelocityAccel = 5,
	STE_Jump = 6,
	STE_State = 9,
	STE_CanHit = 10,
	STE_Sound = 11,
	STE_Particle = 12,
	STE_SendAction_Weapon = 14,
	STE_Projectile = 15,
	STE_EnvironmentEffect = 16,
	STE_ShowWeapon = 17,
	STE_Destroy = 18,
	STE_AlphaBlending = 19,
	STE_CanDefense = 20,
	STE_AttachTrail = 21,
	STE_CustomAction = 23,
	STE_MotionSpeed = 25,
	STE_NormalSuperAmmor = 27,
	STE_InputHasPassiveSkill = 29,
	STE_CameraEffect_Shake = 31,
	STE_ResetVelocity = 32,
	STE_FX = 33,
	STE_PickupItem = 34,
	STE_CanRotate = 35,
	STE_EnableCollision = 36,
	STE_ActionObject = 37,
	STE_ObjectVisible = 38,
	STE_EmptySignal = 39,
	STE_AttachSwordTrail = 43,
	STE_ShowSwordTrail = 44,
	STE_CameraEffect_RadialBlur = 45,
	STE_SkillSuperAmmor = 46,
	STE_SummonMonster = 48,
	STE_SummonProp = 49,
	STE_CustomState = 50,
	STE_Decal = 51,
	STE_HeadLook = 52,
	STE_LockTargetLook = 55,
	STE_ShaderCustomParameter = 56,
	STE_ChangeWeaponLink = 57,
	STE_FreezeCamera = 58,
	STE_SocialAction = 59,
	STE_ProjectileTargetPosition = 60,
	STE_CanUseSkill = 61,
	STE_RebirthAnyPlayer = 62,
	STE_RotateResistance = 63,
	STE_HideExposureInfo = 64,
	STE_ChangeAction = 65,
	STE_ApplyStateEffect = 66,
	STE_PhysicsSkip = 67,
	STE_CanBumpWall = 69,
	STE_DropItem = 70,
	STE_SummonChestProp = 71,
	STE_SummonBrokenProp = 72,
	STE_SummonBrokenDamageProp = 73,
	STE_SummonHitMoveDamageBrokenProp = 74,
	STE_SummonBuffProp = 75,
	STE_SummonBuffBrokenProp = 76,
	STE_PositionRevision = 77,
	STE_Parring = 78,
	STE_Particle_LoopEnd = 79,
	STE_FX_LoopEnd = 80,
	STE_OutlineFilter = 81,
	STE_CancelChangeStandActionSE = 82,
	STE_SuicideMonster = 83,
	STE_Billboard = 84,
	STE_EyeLightTrail = 85,
	STE_PointLight = 87,
	STE_ImmediatelyAttach = 88,
	STE_TriggerEvent = 89,
	STE_Announce = 90,
	STE_Dialogue = 91,
	STE_OtherSelfEffect = 92,
	STE_Gravity = 93,
	STE_AttachLine = 94,
	STE_SyncChangeAction = 95,
	STE_ReserveProjectileTarget = 96,
	STE_ShootCannon = 97,
	STE_CannonTargeting = 98,
	STE_CooltimeParring = 99,
	STE_SkillChecker = 100,
	STE_ChangeActionSECheck = 101,
	STE_NextCustomAction = 102,
	STE_CancelChangeStandEndActionSE = 103,
	STE_CatchActor = 104,
	STE_ReleaseActor = 105,
	STE_OrderMySummonedMonster = 106,
	STE_ChangeEmissiveTexture = 107,
	STE_MoveY = 109,
	STE_ChangeWeapon = 111,
	STE_Rotate = 118,
	STE_CameraEffect_Swing = 119,
	STE_AttachEffectTrail = 120,
	STE_anno = 121,
	STE_CameraEffect_Movement = 122,
	STE_CameraEffect_KeyFrame = 123,
	STE_MarkProjectile = 124,
	STE_SyncPlayerCameraYaw = 125,
	STE_SummonMonsterInfo = 126,
	STE_ChangeAxis = 127,
	STE_ChangeSkin = 128,
};

// Structure per signal
struct DnNullSignalStruct : public TBoostMemoryPool< DnNullSignalStruct > {
	char cReserved1[256];

private:
	DnNullSignalStruct(const DnNullSignalStruct&);
	const DnNullSignalStruct& operator=(const DnNullSignalStruct&);

public:
	DnNullSignalStruct() {}
};

struct InputStruct : public TBoostMemoryPool< InputStruct > {
	int nButton;
	char *szChangeAction;
	char cReserved1[4];
	int nBlendFrame;
	int nChangeActionFrame;
	char *szMaintenanceBoneName;
	char *szActionBoneName;
	int nEventType;
	int nMinTime;
	int nMaxTime;
	BOOL bSkillChain;
	char cReserved2[4];
	int nNeedSkillIndex;
	BOOL isSkipEndAction;
	char cReserved3[200];

private:
	InputStruct(const InputStruct&);
	const InputStruct& operator=(const InputStruct&);

public:
	InputStruct() {}
};

struct CanMoveStruct : public TBoostMemoryPool< CanMoveStruct > {
	BOOL bCanMove;
	char cReserved1[4];
	char *szActionBoneName;
	char *szMaintenanceBoneName;
	char cReserved2[240];

private:
	CanMoveStruct(const CanMoveStruct&);
	const CanMoveStruct& operator=(const CanMoveStruct&);

public:
	CanMoveStruct() {}
};

struct HitStruct : public TBoostMemoryPool< HitStruct > {
	float fDamageProb;
	char *szTargetHitAction;
	float fDistanceMax;
	float fDistanceMin;
	float fHeightMax;
	float fHeightMin;
	float fCenterAngle;
	float fAngle;
	EtVector3 *vVelocity;
	EtVector3 *vResistance;
	BOOL bIncludeWeaponLength;
	char cReserved1[4];
	float fDurabilityDamageProb;
	int nStateCondition;
	float fStiffProb;
	EtVector3 *vOffset;
	int nTargetType;
	int nDamageType;
	int nComboDelay;
	char cReserved2[4];
	int nApplySuperAmmorIndex;
	int nApplySuperAmmorDamage;
	int nAttackType;
	char *szLinkBoneName;
	int nDistanceType;
	BOOL bUseSkillApplyWeaponElement;
	char cReserved3[4];
	BOOL bUseHitAreaOnProjectile;
	int nPartyComboDelay;
	EtVector3 *vSelfVelocity;
	EtVector3 *vSelfResistance;
	int StateEffectFilter;
	BOOL bIgnoreCanHit;
	BOOL bIgnoreParring;
	float fCriticalVelocityRevision;
	float fStunVelocityRevision;
	BOOL bUseMyRelicFilter;
	int nHitLimitCount;
	int nSuperAmmorTime;
	int nActorType;
	BOOL bIsCatchedActor;
	BOOL bReleaseCatchedActor;
	int nWeightValueLimit;
	BOOL isSelfCheck;
	char cReserved4[4];
	char *szSkipStateBlows;
	char *szRemoveStateIndexList;
	char cReserved5[4];
	char *szUnStateEffectFilter;
	int CheckHitDirection;
	char *szTriggerEventParameter;
	char cReserved6[52];

private:
	HitStruct(const HitStruct&);
	const HitStruct& operator=(const HitStruct&);

public:
	HitStruct() {}
};

struct VelocityAccelStruct : public TBoostMemoryPool< VelocityAccelStruct > {
	EtVector3 *vVelocity;
	EtVector3 *vResistance;
	char cReserved1[248];

private:
	VelocityAccelStruct(const VelocityAccelStruct&);
	const VelocityAccelStruct& operator=(const VelocityAccelStruct&);

public:
	VelocityAccelStruct() {}
};

struct JumpStruct : public TBoostMemoryPool< JumpStruct > {
	float fJumpVelocity;
	float fJumpResistance;
	BOOL bResetPrevVelocity;
	BOOL bIgnoreJumpDir;
	char cReserved1[240];

private:
	JumpStruct(const JumpStruct&);
	const JumpStruct& operator=(const JumpStruct&);

public:
	JumpStruct() {}
};

struct StateStruct : public TBoostMemoryPool< StateStruct > {
	int nStateOne;
	int nStateTwo;
	int nStateThree;
	char cReserved1[244];

private:
	StateStruct(const StateStruct&);
	const StateStruct& operator=(const StateStruct&);

public:
	StateStruct() {}
};

struct CanHitStruct : public TBoostMemoryPool< CanHitStruct > {
	BOOL bHittable;
	char cReserved1[252];

private:
	CanHitStruct(const CanHitStruct&);
	const CanHitStruct& operator=(const CanHitStruct&);

public:
	CanHitStruct() {}
};

struct SoundStruct : public TBoostMemoryPool< SoundStruct > {
	char *szFileName;
	float fVolume;
	float fRange;
	float fRollOff;
	std::vector<int> * nVecSoundIndex;
	int nRandom;
	int nType;
	BOOL bUseRandomSound;
	BOOL bHighPrior;
	BOOL bLoop;
	float fFadeDelta;
	BOOL bLoadingComplete;
	char cReserved1[208];

private:
	SoundStruct(const SoundStruct&);
	const SoundStruct& operator=(const SoundStruct&);

public:
	SoundStruct() {}
};

struct ParticleStruct : public TBoostMemoryPool< ParticleStruct > {
	char *szFileName;
	BOOL bLoop;
	BOOL bIterator;
	EtVector3 *vPosition;
	int nParticleDataIndex;
	char cReserved1[4];
	BOOL bLinkObject;
	int nCullDistance;
	BOOL bClingTerrain;
	EtVector3 *vRotation;
	char *szBoneName;
	float fScale;
	BOOL bDefendenceParent;
	BOOL bOptimize;
	BOOL bDeleteByChangeAction;
	BOOL bDisableCull;
	BOOL bEnable;
	char cReserved2[188];

private:
	ParticleStruct(const ParticleStruct&);
	const ParticleStruct& operator=(const ParticleStruct&);

public:
	ParticleStruct() {}
};

struct SendAction_WeaponStruct : public TBoostMemoryPool< SendAction_WeaponStruct > {
	char *szActionName;
	int nWeaponIndex;
	int nLoopCount;
	char cReserved1[244];

private:
	SendAction_WeaponStruct(const SendAction_WeaponStruct&);
	const SendAction_WeaponStruct& operator=(const SendAction_WeaponStruct&);

public:
	SendAction_WeaponStruct() {}
};

struct ProjectileStruct : public TBoostMemoryPool< ProjectileStruct > {
	int nWeaponTableID;
	EtVector3 *vOffset;
	char cReserved1[20];
	int nProjectileIndex;
	int nOrbitType;
	int nTargetType;
	int nDestroyOrbitType;
	float fSpeed;
	float fResistance;
	char cReserved2[4];
	EtVector3 *vDirection;
	char cReserved3[4];
	BOOL bIncludeMainWeaponLength;
	BOOL bPierce;
	int nTargetPosRandomValue;
	char cReserved4[4];
	int nValidTime;
	int VelocityType;
	char cReserved5[4];
	int nMaxHitCount;
	BOOL bDirectionFollowView;
	char cReserved6[4];
	BOOL bHitFXUseWorldAxis;
	EtVector3 *vDestPosition;
	float fProjectileOrbitRotateZ;
	BOOL bUseServerSideInfo;
	BOOL bTraceHitTarget;
	BOOL bUseTargetPositionDir;
	BOOL bTraceHitActorHittable;
	BOOL bDoNotDestroyEffect;
	BOOL bNormalizeDirectionVector;
	int nIgnoreHitType;
	int nTargetStateIndex;
	BOOL bHitActionVectorInit;
	char cReserved7[4];
	BOOL bApplyStateBlowInfo;
	char *RandomWeaponParam;
	char cReserved8[92];

private:
	ProjectileStruct(const ProjectileStruct&);
	const ProjectileStruct& operator=(const ProjectileStruct&);

public:
	ProjectileStruct() {}
};

struct EnvironmentEffectStruct : public TBoostMemoryPool< EnvironmentEffectStruct > {
	int nCondition;
	char *szBoneName;
	EtVector3 *vOffset;
	char cReserved1[12];
	EtVector3 *vRotate;
	char cReserved2[228];

private:
	EnvironmentEffectStruct(const EnvironmentEffectStruct&);
	const EnvironmentEffectStruct& operator=(const EnvironmentEffectStruct&);

public:
	EnvironmentEffectStruct() {}
};

struct ShowWeaponStruct : public TBoostMemoryPool< ShowWeaponStruct > {
	int nEquipIndex;
	BOOL bEnable;
	BOOL EnableByShowOption;
	char cReserved1[244];

private:
	ShowWeaponStruct(const ShowWeaponStruct&);
	const ShowWeaponStruct& operator=(const ShowWeaponStruct&);

public:
	ShowWeaponStruct() {}
};

struct DestroyStruct : public TBoostMemoryPool< DestroyStruct > {
	char cReserved1[256];

private:
	DestroyStruct(const DestroyStruct&);
	const DestroyStruct& operator=(const DestroyStruct&);

public:
	DestroyStruct() {}
};

struct AlphaBlendingStruct : public TBoostMemoryPool< AlphaBlendingStruct > {
	float fStartAlpha;
	float fEndAlpha;
	char cReserved1[248];

private:
	AlphaBlendingStruct(const AlphaBlendingStruct&);
	const AlphaBlendingStruct& operator=(const AlphaBlendingStruct&);

public:
	AlphaBlendingStruct() {}
};

struct CanDefenseStruct : public TBoostMemoryPool< CanDefenseStruct > {
	int nProb;
	char cReserved1[252];

private:
	CanDefenseStruct(const CanDefenseStruct&);
	const CanDefenseStruct& operator=(const CanDefenseStruct&);

public:
	CanDefenseStruct() {}
};

struct AttachTrailStruct : public TBoostMemoryPool< AttachTrailStruct > {
	char *szTrailTextureName;
	EtVector3 *vOffset;
	float fLifeTime;
	float fTrailWidth;
	float fMinSegment;
	int nTextureIndex;
	char cReserved1[232];

private:
	AttachTrailStruct(const AttachTrailStruct&);
	const AttachTrailStruct& operator=(const AttachTrailStruct&);

public:
	AttachTrailStruct() {}
};

struct CustomActionStruct : public TBoostMemoryPool< CustomActionStruct > {
	char *szChangeAction;
	int nBlendFrame;
	int nChangeActionFrame;
	char *szMaintenanceBoneName;
	char *szActionBoneName;
	char cReserved1[236];

private:
	CustomActionStruct(const CustomActionStruct&);
	const CustomActionStruct& operator=(const CustomActionStruct&);

public:
	CustomActionStruct() {}
};

struct MotionSpeedStruct : public TBoostMemoryPool< MotionSpeedStruct > {
	float fFrame;
	char cReserved1[252];

private:
	MotionSpeedStruct(const MotionSpeedStruct&);
	const MotionSpeedStruct& operator=(const MotionSpeedStruct&);

public:
	MotionSpeedStruct() {}
};

struct NormalSuperAmmorStruct : public TBoostMemoryPool< NormalSuperAmmorStruct > {
	BOOL bEnable;
	int nTime;
	float fBreakDurability;
	char cReserved1[244];

private:
	NormalSuperAmmorStruct(const NormalSuperAmmorStruct&);
	const NormalSuperAmmorStruct& operator=(const NormalSuperAmmorStruct&);

public:
	NormalSuperAmmorStruct() {}
};

struct InputHasPassiveSkillStruct : public TBoostMemoryPool< InputHasPassiveSkillStruct > {
	int nButton;
	int nSkillIndex;
	char *szChangeAction;
	int nBlendFrame;
	int nChangeActionFrame;
	int nEventType;
	int nMinTime;
	int nMaxTime;
	int nSkillLevel;
	BOOL bOnlyCheck;
	BOOL bNotCheckPushKey;
	char *szEXSkillChangeAction;
	BOOL bCanUseSkill;
	char cReserved1[204];

private:
	InputHasPassiveSkillStruct(const InputHasPassiveSkillStruct&);
	const InputHasPassiveSkillStruct& operator=(const InputHasPassiveSkillStruct&);

public:
	InputHasPassiveSkillStruct() {}
};

struct CameraEffect_ShakeStruct : public TBoostMemoryPool< CameraEffect_ShakeStruct > {
	float fShakeValue;
	float fRange;
	float fRolloff;
	float fBeginRatio;
	float fEndRatio;
	int nCameraEffectRefIndex;
	char cReserved1[232];

private:
	CameraEffect_ShakeStruct(const CameraEffect_ShakeStruct&);
	const CameraEffect_ShakeStruct& operator=(const CameraEffect_ShakeStruct&);

public:
	CameraEffect_ShakeStruct() {}
};

struct ResetVelocityStruct : public TBoostMemoryPool< ResetVelocityStruct > {
	BOOL bXAxis;
	BOOL bYAxis;
	BOOL bZAxis;
	char cReserved1[244];

private:
	ResetVelocityStruct(const ResetVelocityStruct&);
	const ResetVelocityStruct& operator=(const ResetVelocityStruct&);

public:
	ResetVelocityStruct() {}
};

struct FXStruct : public TBoostMemoryPool< FXStruct > {
	char *szFileName;
	int nFXDataIndex;
	EtVector3 *vOffset;
	BOOL bLinkObject;
	BOOL bLoop;
	EtVector3 *vRotation;
	char *szBoneName;
	float fScale;
	char cReserved1[4];
	BOOL bClingTerrain;
	char cReserved2[4];
	BOOL bDisableCull;
	BOOL bUseTraceHitProjectileActorAdditionalHeight;
	char cReserved3[4];
	BOOL bEnable;
	char cReserved4[196];

private:
	FXStruct(const FXStruct&);
	const FXStruct& operator=(const FXStruct&);

public:
	FXStruct() {}
};

struct PickupItemStruct : public TBoostMemoryPool< PickupItemStruct > {
	int nRange;
	char cReserved1[252];

private:
	PickupItemStruct(const PickupItemStruct&);
	const PickupItemStruct& operator=(const PickupItemStruct&);

public:
	PickupItemStruct() {}
};

struct CanRotateStruct : public TBoostMemoryPool< CanRotateStruct > {
	BOOL bRotate;
	char cReserved1[252];

private:
	CanRotateStruct(const CanRotateStruct&);
	const CanRotateStruct& operator=(const CanRotateStruct&);

public:
	CanRotateStruct() {}
};

struct EnableCollisionStruct : public TBoostMemoryPool< EnableCollisionStruct > {
	BOOL bEnable;
	char cReserved1[252];

private:
	EnableCollisionStruct(const EnableCollisionStruct&);
	const EnableCollisionStruct& operator=(const EnableCollisionStruct&);

public:
	EnableCollisionStruct() {}
};

struct ActionObjectStruct : public TBoostMemoryPool< ActionObjectStruct > {
	char *szSkinName;
	char *szAniName;
	char *szActionName;
	int nActionObjectIndex;
	char *szAction;
	EtVector3 *vRotate;
	EtVector3 *vOffset;
	BOOL bLinkObject;
	char cReserved1[4];
	BOOL bDefendenceParent;
	EtVector3 *vScale;
	BOOL bClingTerrain;
	BOOL bBindParentAction;
	BOOL bShadowOff;
	BOOL bEnable;
	char cReserved2[196];

private:
	ActionObjectStruct(const ActionObjectStruct&);
	const ActionObjectStruct& operator=(const ActionObjectStruct&);

public:
	ActionObjectStruct() {}
};

struct ObjectVisibleStruct : public TBoostMemoryPool< ObjectVisibleStruct > {
	BOOL bShow;
	char cReserved1[252];

private:
	ObjectVisibleStruct(const ObjectVisibleStruct&);
	const ObjectVisibleStruct& operator=(const ObjectVisibleStruct&);

public:
	ObjectVisibleStruct() {}
};

struct EmptySignalStruct : public TBoostMemoryPool< EmptySignalStruct > {
	char cReserved1[256];

private:
	EmptySignalStruct(const EmptySignalStruct&);
	const EmptySignalStruct& operator=(const EmptySignalStruct&);

public:
	EmptySignalStruct() {}
};

struct AttachSwordTrailStruct : public TBoostMemoryPool< AttachSwordTrailStruct > {
	char *szTextureFileName;
	char *szNormalTextureFileName;
	float fLifeTime;
	char cReserved1[4];
	float fMinSegment;
	int nTextureIndex;
	int nNormalTextureIndex;
	int nBlendOP;
	int nSrcBlend;
	int nDestBlend;
	int nTechIndex;
	char cReserved2[212];

private:
	AttachSwordTrailStruct(const AttachSwordTrailStruct&);
	const AttachSwordTrailStruct& operator=(const AttachSwordTrailStruct&);

public:
	AttachSwordTrailStruct() {}
};

struct ShowSwordTrailStruct : public TBoostMemoryPool< ShowSwordTrailStruct > {
	char cReserved1[16];
	int nWeaponIndex;
	char cReserved2[4];
	char *szTextureFileName;
	char *szNormalTextureFileName;
	int nTextureIndex;
	int nNormalTextureIndex;
	float fLifeTime;
	float fMinSegment;
	int nBlendOP;
	int nSrcBlend;
	int nDestBlend;
	int nTechIndex;
	BOOL bUseTextureTechique;
	char cReserved3[188];

private:
	ShowSwordTrailStruct(const ShowSwordTrailStruct&);
	const ShowSwordTrailStruct& operator=(const ShowSwordTrailStruct&);

public:
	ShowSwordTrailStruct() {}
};

struct CameraEffect_RadialBlurStruct : public TBoostMemoryPool< CameraEffect_RadialBlurStruct > {
	EtVector2 *vCenter;
	float fBlurSize;
	float fBeginRatio;
	float fEndRatio;
	int nCameraEffectRefIndex;
	char cReserved1[236];

private:
	CameraEffect_RadialBlurStruct(const CameraEffect_RadialBlurStruct&);
	const CameraEffect_RadialBlurStruct& operator=(const CameraEffect_RadialBlurStruct&);

public:
	CameraEffect_RadialBlurStruct() {}
};

struct SkillSuperAmmorStruct : public TBoostMemoryPool< SkillSuperAmmorStruct > {
	float fSuperAmmorOneProb;
	float fSuperAmmorTwoProb;
	float fSuperAmmorThreeProb;
	float fSuperAmmorFourProb;
	int nTime;
	float fDamageDecreaseProb;
	char cReserved1[232];

private:
	SkillSuperAmmorStruct(const SkillSuperAmmorStruct&);
	const SkillSuperAmmorStruct& operator=(const SkillSuperAmmorStruct&);

public:
	SkillSuperAmmorStruct() {}
};

struct SummonMonsterStruct : public TBoostMemoryPool< SummonMonsterStruct > {
	int MonsterID;
	EtVector3 *vPosition;
	int nLifeTime;
	int nTeam;
	int nMaxCount;
	BOOL bCopySummonerState;
	float fHPAdjustValue;
	int nSummonRate;
	float fAngleOffset;
	int nForceSkillLevel;
	int nGroupID;
	BOOL bSuicideWhenSummonerDie;
	BOOL bFollowSummonerStage;
	float fLimitSummonerDistance;
	BOOL bDontCopySkillStateEffect;
	char cReserved1[20];
	BOOL AddImmuneBlow;
	BOOL bPreLoad;
	int AutoRecallRange;
	BOOL PositionCheck;
	BOOL bEnable;
	char cReserved2[156];

private:
	SummonMonsterStruct(const SummonMonsterStruct&);
	const SummonMonsterStruct& operator=(const SummonMonsterStruct&);

public:
	SummonMonsterStruct() {}
};

struct SummonPropStruct : public TBoostMemoryPool< SummonPropStruct > {
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	char cReserved1[240];

private:
	SummonPropStruct(const SummonPropStruct&);
	const SummonPropStruct& operator=(const SummonPropStruct&);

public:
	SummonPropStruct() {}
};

struct CustomStateStruct : public TBoostMemoryPool< CustomStateStruct > {
	int nStateOne;
	int nStateTwo;
	int nStateThree;
	char cReserved1[244];

private:
	CustomStateStruct(const CustomStateStruct&);
	const CustomStateStruct& operator=(const CustomStateStruct&);

public:
	CustomStateStruct() {}
};

struct DecalStruct : public TBoostMemoryPool< DecalStruct > {
	char *szFileName;
	float fRadius;
	EtVector3 *vPosition;
	float fLifeTime;
	float fRotate;
	char cReserved1[4];
	int nTextureIndex;
	int nSrcBlend;
	int nDestBlend;
	float fAppearTime;
	BOOL bLinkObject;
	char cReserved2[212];

private:
	DecalStruct(const DecalStruct&);
	const DecalStruct& operator=(const DecalStruct&);

public:
	DecalStruct() {}
};

struct HeadLookStruct : public TBoostMemoryPool< HeadLookStruct > {
	int nRotateSpeed;
	float fHeadRatio;
	int nResponeLenth;
	float fYAxisMaxAngle;
	float fXAxisMaxAngle;
	BOOL bReverseYAxis;
	BOOL bReverseXAxis;
	char cReserved1[228];

private:
	HeadLookStruct(const HeadLookStruct&);
	const HeadLookStruct& operator=(const HeadLookStruct&);

public:
	HeadLookStruct() {}
};

struct LockTargetLookStruct : public TBoostMemoryPool< LockTargetLookStruct > {
	int LookEeventAreaID;
	char cReserved1[252];

private:
	LockTargetLookStruct(const LockTargetLookStruct&);
	const LockTargetLookStruct& operator=(const LockTargetLookStruct&);

public:
	LockTargetLookStruct() {}
};

struct ShaderCustomParameterStruct : public TBoostMemoryPool< ShaderCustomParameterStruct > {
	char *szParamStr;
	int nParamType;
	int nValue;
	float fValue;
	EtVector4 *vValue;
	char *szValue;
	char cReserved1[16];
	int nSubmeshIndex;
	BOOL bContinueToNextAction;
	char cReserved2[208];

private:
	ShaderCustomParameterStruct(const ShaderCustomParameterStruct&);
	const ShaderCustomParameterStruct& operator=(const ShaderCustomParameterStruct&);

public:
	ShaderCustomParameterStruct() {}
};

struct ChangeWeaponLinkStruct : public TBoostMemoryPool< ChangeWeaponLinkStruct > {
	char *szLinkBoneName;
	int nWeaponIndex;
	char cReserved1[248];

private:
	ChangeWeaponLinkStruct(const ChangeWeaponLinkStruct&);
	const ChangeWeaponLinkStruct& operator=(const ChangeWeaponLinkStruct&);

public:
	ChangeWeaponLinkStruct() {}
};

struct FreezeCameraStruct : public TBoostMemoryPool< FreezeCameraStruct > {
	float fResistanceRatio;
	BOOL bIgnoreRefreshPlayerView;
	char cReserved1[248];

private:
	FreezeCameraStruct(const FreezeCameraStruct&);
	const FreezeCameraStruct& operator=(const FreezeCameraStruct&);

public:
	FreezeCameraStruct() {}
};

struct SocialActionStruct : public TBoostMemoryPool< SocialActionStruct > {
	char *szActionName;
	int nRandom;
	int nCondition;
	char *szNextActionName;
	char cReserved1[240];

private:
	SocialActionStruct(const SocialActionStruct&);
	const SocialActionStruct& operator=(const SocialActionStruct&);

public:
	SocialActionStruct() {}
};

struct ProjectileTargetPositionStruct : public TBoostMemoryPool< ProjectileTargetPositionStruct > {
	char cReserved1[256];

private:
	ProjectileTargetPositionStruct(const ProjectileTargetPositionStruct&);
	const ProjectileTargetPositionStruct& operator=(const ProjectileTargetPositionStruct&);

public:
	ProjectileTargetPositionStruct() {}
};

struct CanUseSkillStruct : public TBoostMemoryPool< CanUseSkillStruct > {
	int CheckType;
	BOOL bUseSignalSkillCheck;
	char cReserved1[248];

private:
	CanUseSkillStruct(const CanUseSkillStruct&);
	const CanUseSkillStruct& operator=(const CanUseSkillStruct&);

public:
	CanUseSkillStruct() {}
};

struct RebirthAnyPlayerStruct : public TBoostMemoryPool< RebirthAnyPlayerStruct > {
	int nRadius;
	char cReserved1[252];

private:
	RebirthAnyPlayerStruct(const RebirthAnyPlayerStruct&);
	const RebirthAnyPlayerStruct& operator=(const RebirthAnyPlayerStruct&);

public:
	RebirthAnyPlayerStruct() {}
};

struct RotateResistanceStruct : public TBoostMemoryPool< RotateResistanceStruct > {
	float fResistanceRatio;
	char cReserved1[252];

private:
	RotateResistanceStruct(const RotateResistanceStruct&);
	const RotateResistanceStruct& operator=(const RotateResistanceStruct&);

public:
	RotateResistanceStruct() {}
};

struct HideExposureInfoStruct : public TBoostMemoryPool< HideExposureInfoStruct > {
	char cReserved1[256];

private:
	HideExposureInfoStruct(const HideExposureInfoStruct&);
	const HideExposureInfoStruct& operator=(const HideExposureInfoStruct&);

public:
	HideExposureInfoStruct() {}
};

struct ChangeActionStruct : public TBoostMemoryPool< ChangeActionStruct > {
	int nRandom;
	char *szAction;
	char cReserved1[248];

private:
	ChangeActionStruct(const ChangeActionStruct&);
	const ChangeActionStruct& operator=(const ChangeActionStruct&);

public:
	ChangeActionStruct() {}
};

struct ApplyStateEffectStruct : public TBoostMemoryPool< ApplyStateEffectStruct > {
	int StateEffectIndex;
	char cReserved1[252];

private:
	ApplyStateEffectStruct(const ApplyStateEffectStruct&);
	const ApplyStateEffectStruct& operator=(const ApplyStateEffectStruct&);

public:
	ApplyStateEffectStruct() {}
};

struct PhysicsSkipStruct : public TBoostMemoryPool< PhysicsSkipStruct > {
	char cReserved1[256];

private:
	PhysicsSkipStruct(const PhysicsSkipStruct&);
	const PhysicsSkipStruct& operator=(const PhysicsSkipStruct&);

public:
	PhysicsSkipStruct() {}
};

struct CanBumpWallStruct : public TBoostMemoryPool< CanBumpWallStruct > {
	char *szActionName;
	char cReserved1[252];

private:
	CanBumpWallStruct(const CanBumpWallStruct&);
	const CanBumpWallStruct& operator=(const CanBumpWallStruct&);

public:
	CanBumpWallStruct() {}
};

struct DropItemStruct : public TBoostMemoryPool< DropItemStruct > {
	int nDropItemTableID;
	char *szActionName;
	EtVector3 *vOffset;
	BOOL bFixPosition;
	char cReserved1[240];

private:
	DropItemStruct(const DropItemStruct&);
	const DropItemStruct& operator=(const DropItemStruct&);

public:
	DropItemStruct() {}
};

struct SummonChestPropStruct : public TBoostMemoryPool< SummonChestPropStruct > {
	int nNeedKeyID;
	int nItemDropGroupTableID;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nLifeTime;
	char cReserved1[228];

private:
	SummonChestPropStruct(const SummonChestPropStruct&);
	const SummonChestPropStruct& operator=(const SummonChestPropStruct&);

public:
	SummonChestPropStruct() {}
};

struct SummonBrokenPropStruct : public TBoostMemoryPool< SummonBrokenPropStruct > {
	int nDurability;
	int nItemDropGroupTableID;
	float fCheckRange;
	int nSkillLevel;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nLifeTime;
	char cReserved1[220];

private:
	SummonBrokenPropStruct(const SummonBrokenPropStruct&);
	const SummonBrokenPropStruct& operator=(const SummonBrokenPropStruct&);

public:
	SummonBrokenPropStruct() {}
};

struct SummonBrokenDamagePropStruct : public TBoostMemoryPool< SummonBrokenDamagePropStruct > {
	int nItemDropGroupTableID;
	int nDurability;
	int nSkillTableID;
	int nSkillLevel;
	int SkillApplyType;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nLifeTime;
	char cReserved1[216];

private:
	SummonBrokenDamagePropStruct(const SummonBrokenDamagePropStruct&);
	const SummonBrokenDamagePropStruct& operator=(const SummonBrokenDamagePropStruct&);

public:
	SummonBrokenDamagePropStruct() {}
};

struct SummonHitMoveDamageBrokenPropStruct : public TBoostMemoryPool< SummonHitMoveDamageBrokenPropStruct > {
	int nItemDropGroupTableID;
	int nDurability;
	int MovingAxis;
	float fDefaultAxisMovingSpeed;
	float fMaxAxisMoveDistance;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nLifeTime;
	char cReserved1[216];

private:
	SummonHitMoveDamageBrokenPropStruct(const SummonHitMoveDamageBrokenPropStruct&);
	const SummonHitMoveDamageBrokenPropStruct& operator=(const SummonHitMoveDamageBrokenPropStruct&);

public:
	SummonHitMoveDamageBrokenPropStruct() {}
};

struct SummonBuffPropStruct : public TBoostMemoryPool< SummonBuffPropStruct > {
	int nSkillTableID;
	int nSkillLevel;
	float fCheckRange;
	int nTeam;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nLifeTime;
	char cReserved1[220];

private:
	SummonBuffPropStruct(const SummonBuffPropStruct&);
	const SummonBuffPropStruct& operator=(const SummonBuffPropStruct&);

public:
	SummonBuffPropStruct() {}
};

struct SummonBuffBrokenPropStruct : public TBoostMemoryPool< SummonBuffBrokenPropStruct > {
	int nDurability;
	int nItemDropGroupTableID;
	int nSkillTableID;
	float fCheckRange;
	int nTeam;
	int nSummonPropTableID;
	EtVector3 *vPosition;
	EtVector3 *vRotate;
	EtVector3 *vScale;
	int nSkillLevel;
	int nLifeTime;
	char cReserved1[212];

private:
	SummonBuffBrokenPropStruct(const SummonBuffBrokenPropStruct&);
	const SummonBuffBrokenPropStruct& operator=(const SummonBuffBrokenPropStruct&);

public:
	SummonBuffBrokenPropStruct() {}
};

struct PositionRevisionStruct : public TBoostMemoryPool< PositionRevisionStruct > {
	int nRevisionTick;
	char cReserved1[252];

private:
	PositionRevisionStruct(const PositionRevisionStruct&);
	const PositionRevisionStruct& operator=(const PositionRevisionStruct&);

public:
	PositionRevisionStruct() {}
};

struct ParringStruct : public TBoostMemoryPool< ParringStruct > {
	int nSkillID;
	float fProb;
	char *szParringActionName;
	char cReserved1[244];

private:
	ParringStruct(const ParringStruct&);
	const ParringStruct& operator=(const ParringStruct&);

public:
	ParringStruct() {}
};

struct Particle_LoopEndStruct : public TBoostMemoryPool< Particle_LoopEndStruct > {
	char *szFileName;
	char *szApplyActionName;
	char cReserved1[248];

private:
	Particle_LoopEndStruct(const Particle_LoopEndStruct&);
	const Particle_LoopEndStruct& operator=(const Particle_LoopEndStruct&);

public:
	Particle_LoopEndStruct() {}
};

struct FX_LoopEndStruct : public TBoostMemoryPool< FX_LoopEndStruct > {
	char *szFileName;
	BOOL bImmediate;
	char cReserved1[248];

private:
	FX_LoopEndStruct(const FX_LoopEndStruct&);
	const FX_LoopEndStruct& operator=(const FX_LoopEndStruct&);

public:
	FX_LoopEndStruct() {}
};

struct OutlineFilterStruct : public TBoostMemoryPool< OutlineFilterStruct > {
	char cReserved1[4];
	EtVector4 *vColor;
	float fWidth;
	int nType;
	char cReserved2[240];

private:
	OutlineFilterStruct(const OutlineFilterStruct&);
	const OutlineFilterStruct& operator=(const OutlineFilterStruct&);

public:
	OutlineFilterStruct() {}
};

struct CancelChangeStandActionSEStruct : public TBoostMemoryPool< CancelChangeStandActionSEStruct > {
	char cReserved1[256];

private:
	CancelChangeStandActionSEStruct(const CancelChangeStandActionSEStruct&);
	const CancelChangeStandActionSEStruct& operator=(const CancelChangeStandActionSEStruct&);

public:
	CancelChangeStandActionSEStruct() {}
};

struct SuicideMonsterStruct : public TBoostMemoryPool< SuicideMonsterStruct > {
	BOOL bGetExp;
	BOOL bDropItem;
	char cReserved1[248];

private:
	SuicideMonsterStruct(const SuicideMonsterStruct&);
	const SuicideMonsterStruct& operator=(const SuicideMonsterStruct&);

public:
	SuicideMonsterStruct() {}
};

struct BillboardStruct : public TBoostMemoryPool< BillboardStruct > {
	BOOL bYBillboard;
	char cReserved1[252];

private:
	BillboardStruct(const BillboardStruct&);
	const BillboardStruct& operator=(const BillboardStruct&);

public:
	BillboardStruct() {}
};

struct EyeLightTrailStruct : public TBoostMemoryPool< EyeLightTrailStruct > {
	char *szSkinName;
	int nLightTrailCount;
	char *szLinkBoneName;
	float fScaleMin;
	float fScaleMax;
	int nHashCode;
	char cReserved1[232];

private:
	EyeLightTrailStruct(const EyeLightTrailStruct&);
	const EyeLightTrailStruct& operator=(const EyeLightTrailStruct&);

public:
	EyeLightTrailStruct() {}
};

struct PointLightStruct : public TBoostMemoryPool< PointLightStruct > {
	EtVector4 *vDiffuse;
	EtVector4 *vSpecular;
	EtVector3 *vOffset;
	float fRange;
	float fBeginRatio;
	float fEndRatio;
	char cReserved1[232];

private:
	PointLightStruct(const PointLightStruct&);
	const PointLightStruct& operator=(const PointLightStruct&);

public:
	PointLightStruct() {}
};

struct ImmediatelyAttachStruct : public TBoostMemoryPool< ImmediatelyAttachStruct > {
	char *szTrailTextureName;
	int nTextureIndex;
	int ContinueTime;
	char cReserved1[244];

private:
	ImmediatelyAttachStruct(const ImmediatelyAttachStruct&);
	const ImmediatelyAttachStruct& operator=(const ImmediatelyAttachStruct&);

public:
	ImmediatelyAttachStruct() {}
};

struct TriggerEventStruct : public TBoostMemoryPool< TriggerEventStruct > {
	int nEventID;
	char cReserved1[252];

private:
	TriggerEventStruct(const TriggerEventStruct&);
	const TriggerEventStruct& operator=(const TriggerEventStruct&);

public:
	TriggerEventStruct() {}
};

struct AnnounceStruct : public TBoostMemoryPool< AnnounceStruct > {
	int nRange;
	int nTargetMonsterID;
	char cReserved1[248];

private:
	AnnounceStruct(const AnnounceStruct&);
	const AnnounceStruct& operator=(const AnnounceStruct&);

public:
	AnnounceStruct() {}
};

struct DialogueStruct : public TBoostMemoryPool< DialogueStruct > {
	char *UIFileName;
	int Index;
	int LifeTime;
	int Rate;
	char cReserved1[12];
	EtVector4 *MonIDFirstGroup;
	EtVector4 *MonIDSecondGroup;
	EtVector4 *ExeptMonID;
	char cReserved2[216];

private:
	DialogueStruct(const DialogueStruct&);
	const DialogueStruct& operator=(const DialogueStruct&);

public:
	DialogueStruct() {}
};

struct OtherSelfEffectStruct : public TBoostMemoryPool< OtherSelfEffectStruct > {
	float fLifeTime;
	EtVector4 *vBeginColor;
	EtVector4 *vEndColor;
	float fBeginAlpha;
	float fEndAlpha;
	char *szSkinName;
	int nBlendOP;
	int nSrcBlend;
	int nDestBlend;
	char cReserved1[4];
	BOOL bAllCalcPosition;
	char cReserved2[212];

private:
	OtherSelfEffectStruct(const OtherSelfEffectStruct&);
	const OtherSelfEffectStruct& operator=(const OtherSelfEffectStruct&);

public:
	OtherSelfEffectStruct() {}
};

struct GravityStruct : public TBoostMemoryPool< GravityStruct > {
	float fDistanceMax;
	float fMinVelocity;
	float fMaxVelocity;
	int nApplyTargetState;
	int nTargetType;
	EtVector3 *vOffset;
	float fDistanceMin;
	float fAngle;
	float fCenterAngle;
	float fHeightMax;
	float fHeightMin;
	float fMinVelocity_Vertical;
	float fMaxVelocity_Vertical;
	char cReserved1[204];

private:
	GravityStruct(const GravityStruct&);
	const GravityStruct& operator=(const GravityStruct&);

public:
	GravityStruct() {}
};

struct AttachLineStruct : public TBoostMemoryPool< AttachLineStruct > {
	char *szTextureFileName;
	float fLifeTime;
	int nTextureIndex;
	float fLineWidth;
	float fUVMoveSpeed;
	char cReserved1[236];

private:
	AttachLineStruct(const AttachLineStruct&);
	const AttachLineStruct& operator=(const AttachLineStruct&);

public:
	AttachLineStruct() {}
};

struct SyncChangeActionStruct : public TBoostMemoryPool< SyncChangeActionStruct > {
	char *szActionName;
	int nRandom;
	int nConditionOne;
	int nOperatorOne;
	char *szValueOne;
	char cReserved1[236];

private:
	SyncChangeActionStruct(const SyncChangeActionStruct&);
	const SyncChangeActionStruct& operator=(const SyncChangeActionStruct&);

public:
	SyncChangeActionStruct() {}
};

struct ReserveProjectileTargetStruct : public TBoostMemoryPool< ReserveProjectileTargetStruct > {
	char cReserved1[256];

private:
	ReserveProjectileTargetStruct(const ReserveProjectileTargetStruct&);
	const ReserveProjectileTargetStruct& operator=(const ReserveProjectileTargetStruct&);

public:
	ReserveProjectileTargetStruct() {}
};

struct ShootCannonStruct : public TBoostMemoryPool< ShootCannonStruct > {
	int CannonMonsterSkillID;
	char cReserved1[252];

private:
	ShootCannonStruct(const ShootCannonStruct&);
	const ShootCannonStruct& operator=(const ShootCannonStruct&);

public:
	ShootCannonStruct() {}
};

struct CannonTargetingStruct : public TBoostMemoryPool< CannonTargetingStruct > {
	char cReserved1[256];

private:
	CannonTargetingStruct(const CannonTargetingStruct&);
	const CannonTargetingStruct& operator=(const CannonTargetingStruct&);

public:
	CannonTargetingStruct() {}
};

struct CooltimeParringStruct : public TBoostMemoryPool< CooltimeParringStruct > {
	int nSkillID;
	float fProb;
	char *szParringActionName;
	char cReserved1[244];

private:
	CooltimeParringStruct(const CooltimeParringStruct&);
	const CooltimeParringStruct& operator=(const CooltimeParringStruct&);

public:
	CooltimeParringStruct() {}
};

struct SkillCheckerStruct : public TBoostMemoryPool< SkillCheckerStruct > {
	char *szChangeAction;
	int nSkillID;
	char cReserved1[248];

private:
	SkillCheckerStruct(const SkillCheckerStruct&);
	const SkillCheckerStruct& operator=(const SkillCheckerStruct&);

public:
	SkillCheckerStruct() {}
};

struct ChangeActionSECheckStruct : public TBoostMemoryPool< ChangeActionSECheckStruct > {
	int nStateEffectID;
	char *szChangeAction;
	char cReserved1[248];

private:
	ChangeActionSECheckStruct(const ChangeActionSECheckStruct&);
	const ChangeActionSECheckStruct& operator=(const ChangeActionSECheckStruct&);

public:
	ChangeActionSECheckStruct() {}
};

struct NextCustomActionStruct : public TBoostMemoryPool< NextCustomActionStruct > {
	char cReserved1[256];

private:
	NextCustomActionStruct(const NextCustomActionStruct&);
	const NextCustomActionStruct& operator=(const NextCustomActionStruct&);

public:
	NextCustomActionStruct() {}
};

struct CancelChangeStandEndActionSEStruct : public TBoostMemoryPool< CancelChangeStandEndActionSEStruct > {
	char *szEndAction;
	char cReserved1[252];

private:
	CancelChangeStandEndActionSEStruct(const CancelChangeStandEndActionSEStruct&);
	const CancelChangeStandEndActionSEStruct& operator=(const CancelChangeStandEndActionSEStruct&);

public:
	CancelChangeStandEndActionSEStruct() {}
};

struct CatchActorStruct : public TBoostMemoryPool< CatchActorStruct > {
	float fCatchDistance;
	char *szCatchBoneName;
	char cReserved1[4];
	char *szCatchedActorAction;
	char cReserved2[8];
	float fHeightMax;
	float fHeightMin;
	char cReserved3[8];
	int nApplySuperArmorDamage;
	BOOL bIgnoreCanHit;
	BOOL bIgnoreParring;
	char *szTargetActorCatchBoneName;
	BOOL bMultiCatch;
	char cReserved4[196];

private:
	CatchActorStruct(const CatchActorStruct&);
	const CatchActorStruct& operator=(const CatchActorStruct&);

public:
	CatchActorStruct() {}
};

struct ReleaseActorStruct : public TBoostMemoryPool< ReleaseActorStruct > {
	char cReserved1[256];

private:
	ReleaseActorStruct(const ReleaseActorStruct&);
	const ReleaseActorStruct& operator=(const ReleaseActorStruct&);

public:
	ReleaseActorStruct() {}
};

struct OrderMySummonedMonsterStruct : public TBoostMemoryPool< OrderMySummonedMonsterStruct > {
	int nMonsterID;
	float fOrderRange;
	char cReserved1[248];

private:
	OrderMySummonedMonsterStruct(const OrderMySummonedMonsterStruct&);
	const OrderMySummonedMonsterStruct& operator=(const OrderMySummonedMonsterStruct&);

public:
	OrderMySummonedMonsterStruct() {}
};

struct ChangeEmissiveTextureStruct : public TBoostMemoryPool< ChangeEmissiveTextureStruct > {
	char *szEmissiveTexture;
	BOOL bImmediately;
	char cReserved1[248];

private:
	ChangeEmissiveTextureStruct(const ChangeEmissiveTextureStruct&);
	const ChangeEmissiveTextureStruct& operator=(const ChangeEmissiveTextureStruct&);

public:
	ChangeEmissiveTextureStruct() {}
};

struct MoveYStruct : public TBoostMemoryPool< MoveYStruct > {
	float fVelocityY;
	BOOL bMaintainYPos;
	char cReserved1[248];

private:
	MoveYStruct(const MoveYStruct&);
	const MoveYStruct& operator=(const MoveYStruct&);

public:
	MoveYStruct() {}
};

struct ChangeWeaponStruct : public TBoostMemoryPool< ChangeWeaponStruct > {
	int nWeaponIndex;
	int nWeaponID;
	char cReserved1[248];

private:
	ChangeWeaponStruct(const ChangeWeaponStruct&);
	const ChangeWeaponStruct& operator=(const ChangeWeaponStruct&);

public:
	ChangeWeaponStruct() {}
};

struct RotateStruct : public TBoostMemoryPool< RotateStruct > {
	float fStartSpeed;
	float fEndSpeed;
	BOOL bLeft;
	EtVector3 *vAxis;
	char cReserved1[240];

private:
	RotateStruct(const RotateStruct&);
	const RotateStruct& operator=(const RotateStruct&);

public:
	RotateStruct() {}
};

struct CameraEffect_SwingStruct : public TBoostMemoryPool< CameraEffect_SwingStruct > {
	float fSpeed;
	float fAngle;
	int nCameraEffectRefIndex;
	BOOL bSmooth;
	char cReserved1[240];

private:
	CameraEffect_SwingStruct(const CameraEffect_SwingStruct&);
	const CameraEffect_SwingStruct& operator=(const CameraEffect_SwingStruct&);

public:
	CameraEffect_SwingStruct() {}
};

struct AttachEffectTrailStruct : public TBoostMemoryPool< AttachEffectTrailStruct > {
	char *szTextureName;
	int nTextureIndex;
	int nHashCode;
	char *szLinkBoneName;
	float fLifeTime;
	float fTrailWidth;
	float fMinSegment;
	int nBlendOP;
	char cReserved1[4];
	int nSrcBlend;
	int nDestBlend;
	EtVector3 *vOffset;
	BOOL bLoop;
	char cReserved2[204];

private:
	AttachEffectTrailStruct(const AttachEffectTrailStruct&);
	const AttachEffectTrailStruct& operator=(const AttachEffectTrailStruct&);

public:
	AttachEffectTrailStruct() {}
};

struct annoStruct : public TBoostMemoryPool< annoStruct > {
	char cReserved1[256];

private:
	annoStruct(const annoStruct&);
	const annoStruct& operator=(const annoStruct&);

public:
	annoStruct() {}
};

struct CameraEffect_MovementStruct : public TBoostMemoryPool< CameraEffect_MovementStruct > {
	EtVector2 *vDir;
	float fSpeedBegin;
	float fSpeedEnd;
	int nCameraEffectRefIndex;
	char cReserved1[240];

private:
	CameraEffect_MovementStruct(const CameraEffect_MovementStruct&);
	const CameraEffect_MovementStruct& operator=(const CameraEffect_MovementStruct&);

public:
	CameraEffect_MovementStruct() {}
};

struct CameraEffect_KeyFrameStruct : public TBoostMemoryPool< CameraEffect_KeyFrameStruct > {
	char *strCamFile;
	int nCameraEffectRefIndex;
	char cReserved1[248];

private:
	CameraEffect_KeyFrameStruct(const CameraEffect_KeyFrameStruct&);
	const CameraEffect_KeyFrameStruct& operator=(const CameraEffect_KeyFrameStruct&);

public:
	CameraEffect_KeyFrameStruct() {}
};

struct MarkProjectileStruct : public TBoostMemoryPool< MarkProjectileStruct > {
	char *szFileName;
	float fLifeTime;
	float fRadius;
	float fRotate;
	EtVector3 *vPosition;
	int nDestBlend;
	int nSrcBlend;
	int nTextureIndex;
	int nMarkType;
	int nFXDataIndex;
	int nParticleDataIndex;
	BOOL bLoop;
	float fScale;
	float fAppearTime;
	char cReserved1[200];

private:
	MarkProjectileStruct(const MarkProjectileStruct&);
	const MarkProjectileStruct& operator=(const MarkProjectileStruct&);

public:
	MarkProjectileStruct() {}
};

struct SyncPlayerCameraYawStruct : public TBoostMemoryPool< SyncPlayerCameraYawStruct > {
	char cReserved1[256];

private:
	SyncPlayerCameraYawStruct(const SyncPlayerCameraYawStruct&);
	const SyncPlayerCameraYawStruct& operator=(const SyncPlayerCameraYawStruct&);

public:
	SyncPlayerCameraYawStruct() {}
};

struct SummonMonsterInfoStruct : public TBoostMemoryPool< SummonMonsterInfoStruct > {
	int MonsterID;
	char cReserved1[252];

private:
	SummonMonsterInfoStruct(const SummonMonsterInfoStruct&);
	const SummonMonsterInfoStruct& operator=(const SummonMonsterInfoStruct&);

public:
	SummonMonsterInfoStruct() {}
};

struct ChangeAxisStruct : public TBoostMemoryPool< ChangeAxisStruct > {
	char cReserved1[256];

private:
	ChangeAxisStruct(const ChangeAxisStruct&);
	const ChangeAxisStruct& operator=(const ChangeAxisStruct&);

public:
	ChangeAxisStruct() {}
};

struct ChangeSkinStruct : public TBoostMemoryPool< ChangeSkinStruct > {
	char *szSkinFileName;
	char *szAniFileName;
	int nObjectSkinIndex;
	char cReserved1[244];

private:
	ChangeSkinStruct(const ChangeSkinStruct&);
	const ChangeSkinStruct& operator=(const ChangeSkinStruct&);

public:
	ChangeSkinStruct() {}
};

static int GetSignalDataUsingCount(int signalIdx) {
	switch(signalIdx) {
	case 0: return 0;
	case 1: return 14;
	case 2: return 4;
	case 4: return 51;
	case 5: return 2;
	case 6: return 4;
	case 9: return 3;
	case 10: return 1;
	case 11: return 12;
	case 12: return 17;
	case 14: return 3;
	case 15: return 41;
	case 16: return 7;
	case 17: return 3;
	case 18: return 0;
	case 19: return 2;
	case 20: return 1;
	case 21: return 6;
	case 23: return 5;
	case 25: return 1;
	case 27: return 3;
	case 29: return 13;
	case 31: return 6;
	case 32: return 3;
	case 33: return 15;
	case 34: return 1;
	case 35: return 1;
	case 36: return 1;
	case 37: return 15;
	case 38: return 1;
	case 39: return 0;
	case 43: return 11;
	case 44: return 17;
	case 45: return 5;
	case 46: return 6;
	case 48: return 25;
	case 49: return 4;
	case 50: return 3;
	case 51: return 11;
	case 52: return 7;
	case 55: return 1;
	case 56: return 12;
	case 57: return 2;
	case 58: return 2;
	case 59: return 4;
	case 60: return 0;
	case 61: return 2;
	case 62: return 1;
	case 63: return 1;
	case 64: return 0;
	case 65: return 2;
	case 66: return 1;
	case 67: return 0;
	case 69: return 1;
	case 70: return 4;
	case 71: return 7;
	case 72: return 9;
	case 73: return 10;
	case 74: return 10;
	case 75: return 9;
	case 76: return 11;
	case 77: return 1;
	case 78: return 3;
	case 79: return 2;
	case 80: return 2;
	case 81: return 4;
	case 82: return 0;
	case 83: return 2;
	case 84: return 1;
	case 85: return 6;
	case 87: return 6;
	case 88: return 3;
	case 89: return 1;
	case 90: return 2;
	case 91: return 10;
	case 92: return 11;
	case 93: return 13;
	case 94: return 5;
	case 95: return 5;
	case 96: return 0;
	case 97: return 1;
	case 98: return 0;
	case 99: return 3;
	case 100: return 2;
	case 101: return 2;
	case 102: return 0;
	case 103: return 1;
	case 104: return 15;
	case 105: return 0;
	case 106: return 2;
	case 107: return 2;
	case 109: return 2;
	case 111: return 2;
	case 118: return 4;
	case 119: return 4;
	case 120: return 13;
	case 121: return 0;
	case 122: return 4;
	case 123: return 2;
	case 124: return 14;
	case 125: return 0;
	case 126: return 1;
	case 127: return 0;
	case 128: return 3;
	}
	return 64;
};

static int GetSignalDataUsingCount64(int signalIdx) {
	switch(signalIdx) {
	case 0: return 0;
	case 1: return 17;
	case 2: return 6;
	case 4: return 62;
	case 5: return 4;
	case 6: return 4;
	case 9: return 3;
	case 10: return 1;
	case 11: return 14;
	case 12: return 21;
	case 14: return 4;
	case 15: return 45;
	case 16: return 10;
	case 17: return 3;
	case 18: return 0;
	case 19: return 2;
	case 20: return 1;
	case 21: return 8;
	case 23: return 8;
	case 25: return 1;
	case 27: return 3;
	case 29: return 15;
	case 31: return 6;
	case 32: return 3;
	case 33: return 19;
	case 34: return 1;
	case 35: return 1;
	case 36: return 1;
	case 37: return 22;
	case 38: return 1;
	case 39: return 0;
	case 43: return 13;
	case 44: return 19;
	case 45: return 6;
	case 46: return 6;
	case 48: return 26;
	case 49: return 7;
	case 50: return 3;
	case 51: return 13;
	case 52: return 7;
	case 55: return 1;
	case 56: return 15;
	case 57: return 3;
	case 58: return 2;
	case 59: return 6;
	case 60: return 0;
	case 61: return 2;
	case 62: return 1;
	case 63: return 1;
	case 64: return 0;
	case 65: return 3;
	case 66: return 1;
	case 67: return 0;
	case 69: return 2;
	case 70: return 6;
	case 71: return 10;
	case 72: return 12;
	case 73: return 13;
	case 74: return 13;
	case 75: return 12;
	case 76: return 14;
	case 77: return 1;
	case 78: return 4;
	case 79: return 4;
	case 80: return 3;
	case 81: return 5;
	case 82: return 0;
	case 83: return 2;
	case 84: return 1;
	case 85: return 8;
	case 87: return 9;
	case 88: return 4;
	case 89: return 1;
	case 90: return 2;
	case 91: return 14;
	case 92: return 14;
	case 93: return 14;
	case 94: return 6;
	case 95: return 7;
	case 96: return 0;
	case 97: return 1;
	case 98: return 0;
	case 99: return 4;
	case 100: return 3;
	case 101: return 3;
	case 102: return 0;
	case 103: return 2;
	case 104: return 18;
	case 105: return 0;
	case 106: return 2;
	case 107: return 3;
	case 109: return 2;
	case 111: return 2;
	case 118: return 5;
	case 119: return 4;
	case 120: return 16;
	case 121: return 0;
	case 122: return 5;
	case 123: return 3;
	case 124: return 16;
	case 125: return 0;
	case 126: return 1;
	case 127: return 0;
	case 128: return 5;
	}
	return 64;
};

#pragma pack(pop)
