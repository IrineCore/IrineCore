#pragma once

#pragma pack(push, 4)
// Prop type enum
enum PropTypeEnum {
	PTE_Static = 0,
	PTE_Action = 1,
	PTE_Trap = 3,
	PTE_Light = 4,
	PTE_Operation = 5,
	PTE_Chest = 6,
	PTE_Broken = 2,
	PTE_ProjectileShooter = 7,
	PTE_SkillUser = 8,
	PTE_Npc = 9,
	PTE_BrokenDamage = 10,
	PTE_HitMoveDamageBroken = 11,
	PTE_Buff = 12,
	PTE_BuffBroken = 13,
	PTE_Camera = 14,
	PTE_ShooterBroken = 15,
	PTE_EnvironmentProp = 16,
	PTE_MultiDurabilityBrokenProp = 17,
	PTE_KeepOperation = 18,
	PTE_OperationDamage = 19,
	PTE_HitStateEffect = 20,
};

// Structure per prop type
struct StaticStruct : public TBoostMemoryPool< StaticStruct > {
	char cReserved1[1024];
};

struct ActionStruct : public TBoostMemoryPool< ActionStruct > {
	char cReserved1[1024];
};

struct TrapStruct : public TBoostMemoryPool< TrapStruct > {
	int nMonsterTableID;
	int ActivateType;
	BOOL MonsterWeight;
	char cReserved1[1012];
};

struct LightStruct : public TBoostMemoryPool< LightStruct > {
	int nType;
	EtVector4 *vDiffuse;
	EtVector4 *vSpecular;
	EtVector3 *vPosition;
	EtVector3 *vDirection;
	float fRange;
	float fFalloff;
	float fTheta;
	float fPhi;
	BOOL bCastShadow;
	char cReserved1[984];
};

struct OperationStruct : public TBoostMemoryPool< OperationStruct > {
	int nIconType;
	char cReserved1[1020];
};

struct ChestStruct : public TBoostMemoryPool< ChestStruct > {
	int nNeedKeyID;
	int nItemDropGroupTableID;
	BOOL bRemoveKeyItem;
	int nNumNeedItem;
	int nLevelLimitOne;
	char *strLevelBoundDescription;
	char *strLevelBoundRewardItemDropGroupID;
	BOOL bEnableOwnership;
	char cReserved1[992];
};

struct BrokenStruct : public TBoostMemoryPool< BrokenStruct > {
	int nDurability;
	int nItemDropGroupTableID;
	char cReserved1[1016];
};

struct ProjectileShooterStruct : public TBoostMemoryPool< ProjectileShooterStruct > {
	int nMonsterTableID;
	int nSkillTableID;
	int nSkillLevel;
	BOOL MonsterWeight;
	char cReserved1[1008];
};

struct SkillUserStruct : public TBoostMemoryPool< SkillUserStruct > {
	int SkillTableID;
	int SkillLevel;
	float fRadius;
	int UseType;
	char cReserved1[1008];
};

struct NpcStruct : public TBoostMemoryPool< NpcStruct > {
	int nNpcID;
	char cReserved1[1020];
};

struct BrokenDamageStruct : public TBoostMemoryPool< BrokenDamageStruct > {
	int nMonsterTableID;
	int nItemDropGroupTableID;
	int nDurability;
	char cReserved1[4];
	int nSkillTableID;
	int nSkillLevel;
	int SkillApplyType;
	BOOL MonsterWeight;
	char cReserved2[992];
};

struct HitMoveDamageBrokenStruct : public TBoostMemoryPool< HitMoveDamageBrokenStruct > {
	int nMonsterTableID;
	int nItemDropGroupTableID;
	int nDurability;
	int MovingAxis;
	float fDefaultAxisMovingSpeed;
	float fMaxAxisMoveDistance;
	BOOL MonsterWeight;
	char cReserved1[996];
};

struct BuffStruct : public TBoostMemoryPool< BuffStruct > {
	int nMonsterTableID;
	int nSkillTableID;
	int nSkillLevel;
	float fCheckRange;
	int nTeam;
	char *TargetActorID;
	char cReserved1[1000];
};

struct BuffBrokenStruct : public TBoostMemoryPool< BuffBrokenStruct > {
	int nMonsterTableID;
	int nDurability;
	int nItemDropGroupTableID;
	int nSkillTableID;
	int nSkillLevel;
	float fCheckRange;
	int nTeam;
	char cReserved1[996];
};

struct CameraStruct : public TBoostMemoryPool< CameraStruct > {
	char cReserved1[1024];
};

struct ShooterBrokenStruct : public TBoostMemoryPool< ShooterBrokenStruct > {
	int nDurability;
	int nItemDropGroupTableID;
	int nMonsterTableID;
	int nSkillTableID;
	int nSkillLevel;
	BOOL MonsterWeight;
	char cReserved1[1000];
};

struct EnvironmentPropStruct : public TBoostMemoryPool< EnvironmentPropStruct > {
	float fRange;
	char *OnRangeInActionName;
	char *OnRangeOutActionName;
	char cReserved1[1012];
};

struct MultiDurabilityBrokenPropStruct : public TBoostMemoryPool< MultiDurabilityBrokenPropStruct > {
	int nMainDurability;
	int nItemDropGroupTableID;
	int nMonsterTableID;
	int nSubDurability_One;
	char *szActionName_SubDurabilityOne;
	int nSubDurability_Two;
	char *szActionName_SubDurabilityTwo;
	int nSubDurability_Three;
	char *szActionName_SubDurabilityThree;
	int nSubDurability_Four;
	char *szActionName_SubDurabilityFour;
	int nSkillTableID;
	int nSkillLevel;
	char cReserved1[972];
};

struct KeepOperationStruct : public TBoostMemoryPool< KeepOperationStruct > {
	int nIconType;
	char cReserved1[1020];
};

struct OperationDamageStruct : public TBoostMemoryPool< OperationDamageStruct > {
	int nIconType;
	int nMonsterTableID;
	BOOL MonsterWeight;
	char cReserved1[1012];
};

struct HitStateEffectStruct : public TBoostMemoryPool< HitStateEffectStruct > {
	int SkillTableID;
	int SkillLevel;
	int MonsterTableID;
	BOOL MonsterWeight;
	char cReserved1[1008];
};

static int GetPropSignalDataUsingCount(int signalIdx) {
	switch(signalIdx) {
	case 0: return 0;
	case 1: return 0;
	case 2: return 2;
	case 3: return 3;
	case 4: return 10;
	case 5: return 1;
	case 6: return 8;
	case 7: return 4;
	case 8: return 4;
	case 9: return 1;
	case 10: return 8;
	case 11: return 7;
	case 12: return 6;
	case 13: return 7;
	case 14: return 0;
	case 15: return 6;
	case 16: return 3;
	case 17: return 13;
	case 18: return 1;
	case 19: return 3;
	case 20: return 4;
	}
	return 256;
};

static int GetPropSignalDataUsingCount64(int signalIdx) {
	switch(signalIdx) {
	case 0: return 0;
	case 1: return 0;
	case 2: return 2;
	case 3: return 3;
	case 4: return 14;
	case 5: return 1;
	case 6: return 10;
	case 7: return 4;
	case 8: return 4;
	case 9: return 1;
	case 10: return 8;
	case 11: return 7;
	case 12: return 7;
	case 13: return 7;
	case 14: return 0;
	case 15: return 6;
	case 16: return 5;
	case 17: return 17;
	case 18: return 1;
	case 19: return 3;
	case 20: return 4;
	}
	return 256;
};

static bool IsPropPointerSlot32(int signalIdx, int slot32) {
	switch(signalIdx) {
	case 4: return (slot32 == 1 || slot32 == 2 || slot32 == 3 || slot32 == 4); // LightStruct
	case 6: return (slot32 == 5 || slot32 == 6); // ChestStruct
	case 12: return (slot32 == 5); // BuffStruct
	case 16: return (slot32 == 1 || slot32 == 2); // EnvironmentPropStruct
	case 17: return (slot32 == 4 || slot32 == 6 || slot32 == 8 || slot32 == 10); // MultiDurabilityBrokenPropStruct
	default: return false;
	}
};

#pragma pack(pop)
