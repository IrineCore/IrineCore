
#pragma once

#include "EtWorldEventArea.h"

#ifdef _VILLAGESERVER
#include "DNParty.h"
#include "DNUserSession.h"
#endif

#define DUNGEONGATE_OFFSET 20001
#if defined(PRE_EXPANDGATE)
#define DNWORLD_GATE_COUNT 15
#else
#define DNWORLD_GATE_COUNT 10
#endif
namespace EWorldEnum {
	enum LandEnvironment {
		NoneLand,
		Forest,
		Vestiges,
		Dungeon,
		Snowy,
		EnemyCamp,
		LandEnvironment_Amount,
	};

	enum WeatherEnum {
		FineDay,
		FineGlow,
		FineNight,
		FineDawn,
		CloudyDay,
		CloudyGlow,
		CloudyNight,
		CloudyDawn,
		RainDay,
		RainNight,
		HeavyrainDay,
		HeavyrainNight,

		WeatherEnum_Amount,
	};

	enum MapTypeEnum {
		MapTypeUnknown,
		MapTypeVillage,
		MapTypeWorldMap,
		MapTypeDungeon,
	};

	enum MapSubTypeEnum {
		MapSubTypeNone,
		MapSubTypeNest,
		MapSubTypePvPVillage,
		MapSubTypePvPLobby,
		MapSubTypeEvent,
		MapSubTypeFarm,
		MapSubTypeChaosField,
		MapSubTypeDarkLair,
		MapSubTypeNestNormal,
		MapSubTypeDarkLairWorldZone,
		MapSubTypeTreasureStage,
		MapSubTypeFellowship,
	};

	enum PermitGateEnum {
		PermitEnter = 0x00,
		PermitLessPlayerLevel = 0x01,
		PermitLessPartyCount = 0x02,
		PermitNotEnoughItem = 0x04,
		PermitExceedTryCount = 0x08,
		PermitClose = 0x10,
		PermitNotRideVehicle = 0x20,
		PermitMission = 0x40,
	};
}

struct GateStruct {
	char cGateIndex;
	char cStartGateIndex;
	int nMapIndex;
	DWORD dwGatePropID;
	EWorldEnum::MapTypeEnum MapType;
	EWorldEnum::MapSubTypeEnum MapSubType;
	EWorldEnum::LandEnvironment Environment;
	std::vector<std::string> szVecToolMapName;
	EWorldEnum::PermitGateEnum PermitFlag;
	CEtWorldEventArea *pGateArea;
	int nFatigue[5];
	int nMaxTryCount;
	bool bExpandable;

	std::vector<char> cVecLessLevelActorIndex;
	std::vector<char> cVecNotEnoughItemActorIndex;
	std::vector<char> cVecExceedTryActorIndex;
	std::vector<char> cVecNotRideVehicleActorIndex;

	GateStruct() {
		cGateIndex = -1;
		cStartGateIndex = -1;
		nMapIndex = -1;
		dwGatePropID = 0;
		MapType = EWorldEnum::MapTypeUnknown;
		MapSubType = EWorldEnum::MapSubTypeNone;
		Environment = EWorldEnum::NoneLand;
		PermitFlag = EWorldEnum::PermitEnter;
		pGateArea = NULL;
		nMaxTryCount = 0;
		memset( nFatigue, 0, sizeof(nFatigue) );
		bExpandable = false;
	}
	virtual ~GateStruct() {
	}
};

struct IBoostPoolGateStruct:public GateStruct, public TBoostMemoryPool<IBoostPoolGateStruct>
{
	IBoostPoolGateStruct(){}
	~IBoostPoolGateStruct(){}
};

struct DungeonGateStruct : public GateStruct , public TBoostMemoryPool< DungeonGateStruct >
{
	int nMinLevel;
	int nMaxLevel;
	int nMinPartyCount;
	int nMaxPartyCount;
	int nMaxUsableCoin;
	char cCanDifficult[5];
	int nRecommandedLv[5];
	std::vector<GateStruct *> pVecMapList;
	UINT nNeedItemID;
	int nNeedItemCount;
	int nAbyssMinLevel;
	int nAbyssMaxLevel;
	int nAbyssNeedQuestID;
	bool bNeedVehicle;
	int nNeedClearMissionID;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
	int nShareCountMapIndex;	//Clear count shared map index -- 2012-07-02 by stupidfox
	EWorldEnum::MapSubTypeEnum ShareMapSubType;	//Clear count shared map type
#endif

	DungeonGateStruct() {
		bNeedVehicle = false;
		nMinLevel = nMaxLevel = -1;
		nAbyssMinLevel = nAbyssMaxLevel = -1;
		nMinPartyCount = nMaxPartyCount = -1;
		nMaxUsableCoin = -1;
		nNeedItemID = 0;
		nNeedItemCount = -1;
		nAbyssNeedQuestID = -1;
		nNeedClearMissionID = 0;
#if defined(PRE_SHARE_MAP_CLEARCOUNT)
		nShareCountMapIndex = 0;
		ShareMapSubType = EWorldEnum::MapSubTypeNone;
#endif
		memset( cCanDifficult, 1, sizeof(cCanDifficult) );
		memset( nRecommandedLv, 1, sizeof(nRecommandedLv) );
	}
	virtual ~DungeonGateStruct() {
		SAFE_DELETE_PVEC( pVecMapList );
	};
#ifdef _GAMESERVER
	EWorldEnum::PermitGateEnum CanEnterDungeon( CMultiRoom *pRoom, std::vector<char> *cVecLessLevelActorIndex = NULL, std::vector<char> *cVecNotEnoughItemActorIndex = NULL, std::vector<char> *cVecExceedTryActorIndex = NULL , std::vector<char> *cVecNotRideVehicleIndex = NULL );
#elif defined _VILLAGESERVER
	EWorldEnum::PermitGateEnum CanEnterDungeon( CDNParty * pParty );
	EWorldEnum::PermitGateEnum CanEnterDungeon( CDNUserSession * pSession );
#endif
};

GateStruct * CalcMapInfo( int nMapIndex );