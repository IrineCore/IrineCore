#pragma once

#pragma pack(push, 4)
// Event control type enum
enum EventControlTypeEnum {
	ETE_UnitArea = 0,
	ETE_EventArea = 1,
	ETE_VolumeFog = 2,
	ETE_PvPRespawnArea = 3,
	ETE_PvPRespawnItemArea = 4,
	ETE_MonsterSetArea = 5,
	ETE_FarmGrowingArea = 6,
	ETE_FarmFishingArea = 7,
	ETE_GuildWarFlag = 8,
	ETE_PvPOccupationItemArea = 9,
	ETE_EnvironmentEffectAera = 10,
};

// Structure per event control
struct UnitAreaStruct : public TBoostMemoryPool< UnitAreaStruct > {
	EtVector2 *vCount;
	char cReserved1[4];
	int nGenCount;
	int MonsterSpawn;
	EtVector3 *Position;
	char cReserved2[1004];
};

struct EventAreaStruct : public TBoostMemoryPool< EventAreaStruct > {
	int MonsterSpawn;
	EtVector3 *Position;
	int GateType;
	char cReserved1[1012];
};

struct VolumeFogStruct : public TBoostMemoryPool< VolumeFogStruct > {
	char *szTextureName;
	float fDensity;
	EtVector4 *vFogColor;
	BOOL bApplyFineDay;
	BOOL bApplyFineGlow;
	BOOL bApplyFineNight;
	BOOL bApplyFineDawn;
	BOOL bApplyCloudyDay;
	BOOL bApplyCloudyGlow;
	BOOL bApplyCloudyNight;
	BOOL bApplyCloudyDawn;
	BOOL bApplyRainDay;
	BOOL bApplyRainNight;
	BOOL bApplyHeavyrainDay;
	BOOL bApplyHeavyrainNight;
	EtVector3 *Position;
	char cReserved1[960];
};

struct PvPRespawnAreaStruct : public TBoostMemoryPool< PvPRespawnAreaStruct > {
	int nTeam;
	BOOL IsStartArea;
	int nBasePreferScore;
	EtVector3 *Position;
	char cReserved1[1008];
};

struct PvPRespawnItemAreaStruct : public TBoostMemoryPool< PvPRespawnItemAreaStruct > {
	int nRespawnIntervalSec;
	int nItemDropTableID;
	BOOL bIsStartSpawn;
	EtVector3 *Position;
	char cReserved1[1008];
};

struct MonsterSetAreaStruct : public TBoostMemoryPool< MonsterSetAreaStruct > {
	int nSetID;
	int nPosID;
	int nGenCount;
	int MonsterSpawn;
	EtVector3 *Position;
	char cReserved1[1004];
};

struct FarmGrowingAreaStruct : public TBoostMemoryPool< FarmGrowingAreaStruct > {
	int nFarmType;
	EtVector3 *Position;
	char cReserved1[1016];
};

struct FarmFishingAreaStruct : public TBoostMemoryPool< FarmFishingAreaStruct > {
	EtVector3 *Position;
	char cReserved1[1020];
};

struct GuildWarFlagStruct : public TBoostMemoryPool< GuildWarFlagStruct > {
	EtVector3 *Position;
	char cReserved1[1020];
};

struct PvPOccupationItemAreaStruct : public TBoostMemoryPool< PvPOccupationItemAreaStruct > {
	int nRespawnIntervalSec;
	int nItemDropTableID;
	BOOL bIsStartSpawn;
	EtVector3 *Position;
	char cReserved1[1008];
};

struct EnvironmentEffectAeraStruct : public TBoostMemoryPool< EnvironmentEffectAeraStruct > {
	EtVector3 *Position;
	int EffectType;
	char cReserved1[1016];
};

static int GetEventSignalDataUsingCount(int signalIdx) {
	switch(signalIdx) {
	case 0: return 5;
	case 1: return 3;
	case 2: return 16;
	case 3: return 4;
	case 4: return 4;
	case 5: return 5;
	case 6: return 2;
	case 7: return 1;
	case 8: return 1;
	case 9: return 4;
	case 10: return 2;
	}
	return 256;
};
static int GetEventSignalDataUsingCount64(int signalIdx) {
	switch(signalIdx) {
	case 0: return 7;
	case 1: return 4;
	case 2: return 19;
	case 3: return 5;
	case 4: return 5;
	case 5: return 6;
	case 6: return 3;
	case 7: return 2;
	case 8: return 2;
	case 9: return 5;
	case 10: return 3;
	}
	return 256;
};

#pragma pack(pop)
