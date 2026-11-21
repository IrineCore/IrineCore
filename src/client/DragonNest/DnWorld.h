#pragma once

#include "EtWorld.h"
#include "Singleton.h"
#include "DnWorldEnvironment.h"
#include "DnCamera.h"
#include "DnWorldProp.h"
#include "EventControlHeader.h"
#include "PropHeader.h"

class CTask;
class CDnWorldProp;
class CDnWorldEnvironment;

#define DUNGEONGATE_OFFSET 20001
#if defined( PRE_EXPANDGATE )
#define DNWORLD_GATE_COUNT 15
#else
#define DNWORLD_GATE_COUNT 10
#endif
class CDnWorld : public CEtWorld, public CSingleton< CDnWorld > {
public:
	CDnWorld();
	virtual ~CDnWorld();

	enum LandEnvironment {
		NoneLand,
		Forest,
		Vestiges,
		Dungeon,
		Snowy,
		EnemyCamp,
		Mine,
		RuinedForest,
		Island,
		Hell,
		Event_Coupon,
		Event_Experience,
		Event_Random,
		Event_Material,
		Event_Treasure,
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

	enum MapDetailTypeEnum {
		MapDetailDungeon,
		MapDetailDarkLair,
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
		MapSubTypeNestNormal,		// #47865 Map subtype for nest practice mode.
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
	};

	struct GateStruct {
		char cGateIndex;
		char cStartGateIndex;
		int nMapIndex;
		DnPropHandle hGateProp;
		CDnWorld::MapTypeEnum MapType;
		CDnWorld::LandEnvironment Environment;
		std::vector<std::string> szVecToolMapName;
		tstring szMapName;
		tstring szDesc;
//		std::vector<std::string> szVecWeatherName[CDnWorld::WeatherEnum_Amount];
		PermitGateEnum PermitFlag;
		CEtWorldEventArea *pGateArea;
		CEtWorldEventArea *pSafeArea;
		bool bIncludeBuild;

		std::vector<char> cVecLessLevelActorIndex;
		std::vector<char> cVecNotEnoughItemActorIndex;
		std::vector<char> cVecExceedTryActorIndex;
		std::vector<char> cVecNotRideVehicleActorIndex;

		GateStruct() {
			cGateIndex = -1;
			cStartGateIndex = -1;
			nMapIndex = -1;
			MapType = MapTypeEnum::MapTypeUnknown;
			Environment = LandEnvironment::NoneLand;
			PermitFlag = PermitGateEnum::PermitEnter;
			pGateArea = pSafeArea = NULL;
			bIncludeBuild = true;
		}
		virtual ~GateStruct() {
		}
		bool CanEnterGate();
	};

	struct DungeonGateStruct : public GateStruct {
		enum DungeonSetType {
			Defaualt,
			Event,
			Nest,
			DarkLair,
		};
		bool bNeedVehicle;
		int nMinLevel;
		int nMaxLevel;
		int nMinPartyCount;
		int nMaxPartyCount;
		int nMaxUsableCoin;
		char cCanDifficult[5];
		int nRecommendLevel[5];
		int nRecommendPartyCount[5];
		std::string szGateEnterImage;
		std::vector<GateStruct *> pVecMapList;
		int nNeedItemID;
		int nNeedItemCount;
		int nAbyssMinLevel;
		int nAbyssMaxLevel;
		int nAbyssNeedQuestID;
		DungeonSetType SetType;

		DungeonGateStruct() {
			bNeedVehicle = false;
			nMinLevel = nMaxLevel = -1;
			nAbyssMinLevel = nAbyssMaxLevel = -1;
			nMinPartyCount = nMaxPartyCount = -1;
			nMaxUsableCoin = -1;
			memset( cCanDifficult, 1, sizeof(cCanDifficult) );
			memset( nRecommendLevel, 0, sizeof(nRecommendLevel) );
			memset( nRecommendPartyCount, 0, sizeof(nRecommendPartyCount) );
			nNeedItemID = 0;
			nAbyssNeedQuestID = -1;
			SetType = DungeonSetType::Defaualt;
			nNeedItemCount = 0;
		}
		virtual ~DungeonGateStruct() {
			SAFE_DELETE_PVEC( pVecMapList );
		}

		bool IsEnableLevel( int nLevel )
		{
			if( nMaxLevel == -1 && nMinLevel == -1 )		return true;
			if( nMinLevel == -1 && nLevel <= nMaxLevel )	return true;
			if( nMaxLevel == -1 && nLevel >= nMinLevel )	return true;
			if( nLevel<=nMaxLevel && nLevel>=nMinLevel )	return true;

			return false;
		}

		bool IsEnablePartyCount( int nCount )
		{
			if( nCount == 1 && nMaxPartyCount != 1 )
				return true;
			if( nMaxPartyCount == -1 && nMinPartyCount == -1 )		return true;
			if( nMinPartyCount == -1 && nCount <= nMaxPartyCount )	return true;
			if( nMaxPartyCount == -1 && nCount >= nMinPartyCount )	return true;
			if( nCount<=nMaxPartyCount && nCount>=nMinPartyCount )	return true;

			return false;
		}
	};

	enum BattleBGMEnum {
		NoneBattle = -1,
		NormalBattle = 0,
		BossBattle,
		NestBattle,
		BattleBGMEnum_Amount = 3,
	};

protected:
	CDnWorldEnvironment m_Environment;
	LandEnvironment m_LandEnvironment;
	MapTypeEnum m_MapType;
	MapSubTypeEnum m_MapSubType;

	static WeatherEnum s_WeatherDefine[WeatherEnum_Amount][5];
	WeatherEnum m_CurrentWeather;

	float m_fPropCollisionDistance;
	struct VisiblePropStruct {
		CDnWorldProp *pProp;
		float fDelta;
	};

	std::vector<VisiblePropStruct> m_VecVisibleOnCheckList;
	std::vector<VisiblePropStruct> m_VecVisibleOffCheckList;

	CTask *m_pCurrentTask;
//	DWORD m_dwPropUniqueCount;

	std::map<int, std::wstring> m_mGateName;
	std::vector<GateStruct*> m_pVecGateList;

	struct BattleBGMStruct {
		BattleBGMStruct() {
			nSoundIndex = -1;
			hChannel.Identity();
			fCurrentVolume = 0.f;
			fTargetVolume = 0.f;;
			fDelta = 0.f;
			fCurrentDelta = 0.f;
		};
		int nSoundIndex;
		EtSoundChannelHandle hChannel;
		float fCurrentVolume;
		float fTargetVolume;
		float fDelta;
		float fCurrentDelta;
	};
	BattleBGMStruct m_BattleBGM[BattleBGMEnum_Amount];
	BattleBGMStruct m_DefaultBGM;
	BattleBGMEnum m_CurrentBattleBGM;
	bool m_bApplyWaterEnvironment;

	bool m_bProcessBattleBGM;			// Do not process combat music in PVP. Summoning a relic turns off existing BGM because the relic is a monster.
	bool m_bShow;
	int m_iAllowMapType;


//#ifdef PRE_ADD_FILTEREVENT
	bool m_bMonochrome;
	EtVector3 m_vFilterEventColor;
	float m_fFilterEventVolume;
//#endif PRE_ADD_FILTEREVENT

protected:
	virtual CEtWorldGrid *AllocGrid();
	void ProcessVisibleProp( LOCAL_TIME LocalTime, float fDelta );
	void ProcessBattleBGM( LOCAL_TIME LocalTime, float fDelta );

	GateStruct *CalcMapInfo( int nMapIndex );

public:
	virtual bool Initialize( const char *szWorldFolder, const char *szGridName, CTask *pTask, bool bPreLoad = false );
	virtual void Destroy();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void RefreshQualityLevel( int nLevel );

	bool InitializeEnviroment( const char *szFileName, DnCameraHandle hCamera, int nCameraFar = -1, float fWaterFarRatio = 0.5f );
	CDnWorldEnvironment *GetEnvironment() { return &m_Environment; }

	void InitializeBattleBGM( int nCurrentMapIndex, int nStageConstructionLevel );

	void PlayBGM();
	void StopBGM();

//	TileTypeEnum GetTileType( float fX, float fZ, float *fpHeight = NULL );
	TileTypeEnum GetTileType( EtVector3 &vPos, float *fpHeight = NULL );

	void SetLandEnvironment( LandEnvironment Type ) { m_LandEnvironment = Type; }
	LandEnvironment GetLandEnvironment() { return m_LandEnvironment; }

	void SetCurrentWeather( WeatherEnum Weather ) { m_CurrentWeather = Weather; }
	WeatherEnum GetCurrentWeather() { return m_CurrentWeather; }
	WeatherEnum GetNextWeather( int nMapTableID );

	static std::string GetEnviName( int nMapTableID, int nStageConstructionLevel, WeatherEnum CurrentWeather );

	void SetMapType( MapTypeEnum Type ) { m_MapType = Type; }
	MapTypeEnum GetMapType() { return m_MapType; }
	void SetMapSubType( MapSubTypeEnum Type ) { m_MapSubType = Type; }
	MapSubTypeEnum GetMapSubType() { return m_MapSubType; }
	void SetAllowMapType( int Type ){ m_iAllowMapType = Type; }
	int GetAllowMapType() const { return m_iAllowMapType; }

	float GetPropCollisionDistance() { return m_fPropCollisionDistance; }
	void SetPropCollisionDistance( float fValue ) { m_fPropCollisionDistance = fValue; }

	int ScanProp( EtVector3 &vPos, float fRadius, PropTypeEnum PropType, DNVector(DnPropHandle) &hVecList );

	void CheckAndRemoveVisibleProp( CDnWorldProp *pProp );

	CTask *GetCurrentTask() { return m_pCurrentTask; }

//	DWORD AddPropUniqueID() { return m_dwPropUniqueCount++; }

	// Gate-related
	void InitializeGateInfo( int nCurrentMapIndex, int nArrayIndex );
	GateStruct *GetGateStruct( char cGateIndex );
	void GetGateStructList( const char cGateIndex, std::vector<GateStruct *> & vecGateList );
	std::wstring GetGateTitleName( const char cGateIndex );
//	void CheckAndRemovePermitGateInfo( int nPartyIndex );

	PermitGateEnum GetPermitGate( int nGateIndex );
	bool CanEnterGate( char cGateIndex );

	DWORD GetGateCount() { return (DWORD)m_pVecGateList.size(); } 
	GateStruct *GetGateStructFromIndex( DWORD dwIndex );

	void SetPermitGate( int nGateIndex, PermitGateEnum Flag );

	void CallActionTrigger( SectorIndex Index, int nObjectIndex, int nRandomSeed );

	void ChangeBattleBGM( BattleBGMEnum Type, float fPrevVolume, float fPrevFadeDelta, float fChangeVolume, float fChangeFadeDelta );

	// Pick using screen coordinates. 
	bool PickFromScreenCoord(float x, float y, OUT EtVector3& vPickPos);

	// 2009.7.30 hanki
	// Return y position accounting for prop height
	float GetHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell = NULL, EtVector3 *pNormal = NULL, int nTileScale = 1 );
	//

#if defined(PRE_FIX_55855)
	//#55855 Higher value between terrain/prop navmesh (decal handling issue).
	float GetMaxHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell = NULL, EtVector3 *pNormal = NULL, int nTileScale = 1);
#endif // PRE_FIX_55855

	void SetProcessBattleBGM( bool bProcessBattleBGM );

	void Show( bool bShow );
	bool IsShow() { return m_bShow; }

	bool IsUsingTileType( GlobalEnum::TileTypeEnum Type );


//#ifdef PRE_ADD_FILTEREVENT
	virtual void SetSceneAbsoluteColor( bool bMonochrome, EtVector3 & vColor, float fVolume );
//#endif PRE_ADD_FILTEREVENT

	//////////////////////////////////////////////////////////////////////////
	// Code for checking Sea Dragon Nest [2011/01/12 semozz]
protected:
	eDragonNestType m_DragonNestType;
public:
	void SetDragonNestType(eDragonNestType _type) { m_DragonNestType = _type; }
	eDragonNestType GetDragonNestType() { return m_DragonNestType; }
	//////////////////////////////////////////////////////////////////////////

	static void GetGateMapIndex( const int nCurrentMapIndex, const int nIndex, std::vector<int> & vecMapIndex );
	static void GetGateStartGate( const int nCurrentMapIndex, const int nIndex, std::vector<int> & vecStartGate );
	static void GetGatePropIndex( const int nCurrentMapIndex, const int nIndex, const int nArrayIndex, std::vector<int> & vecPropIndex );
};

extern CDnWorld g_DnWorld;
