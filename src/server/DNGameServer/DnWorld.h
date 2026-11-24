#pragma once

#include "EtWorld.h"
#include "DnWorldProp.h"
#include "MultiSingleton.h"
#include "EventControlHeader.h"
#include "DnWorldUtil.h"
#include "DnWorldGateStruct.h"

class CTask;
class CDnWorldProp;

#include "DnWorldGateStruct.h"

class CDnWorldImp : public CEtWorld
{
public:
	CDnWorldImp();
	virtual ~CDnWorldImp();

protected:
	virtual CEtWorldGrid *AllocGrid();

public:
	virtual bool Initialize( const char *szWorldFolder, const char *szGridName );
	virtual void Destroy();
};

class CDnWorld : public CDnWorldImp, public CMultiSingleton<CDnWorld, MAX_SESSION_COUNT>, public TBoostMemoryPool<CDnWorld>
{
public:
	CDnWorld( CMultiRoom *pRoom );
	virtual ~CDnWorld();

protected:
	CDNGameRoom * m_pRoom;

	EWorldEnum::LandEnvironment m_LandEnvironment;
	EWorldEnum::WeatherEnum m_CurrentWeather;
	EWorldEnum::MapTypeEnum m_MapType;
	EWorldEnum::MapSubTypeEnum m_MapSubType;
#if defined(PRE_ADD_ACTIVEMISSION)
	int m_nMaxPartyCount;
#endif
	std::vector<GateStruct*> m_pVecGateList;
	
	static EWorldEnum::WeatherEnum s_WeatherDefine[EWorldEnum::WeatherEnum_Amount][5];

protected:
	virtual CEtWorldGrid *AllocGrid();

	//Changed position to share with Village server -2hogi 2010/1/14
	void CalcDungeonConstructionLevel( int nMapIndex, DungeonGateStruct *pDungeonStruct );

public:
	virtual bool Initialize( const char *szWorldFolder, const char *szGridName );
	virtual void Destroy();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	//int ScanProp( EtVector3 &vPos, float fRadius, PropTypeEnum PropType, std::vector<DnPropHandle> &hVecList );
	
	template<class _Pr> 
	int ScanProp( EtVector3 &vPos, float fRadius, DNVector(DnPropHandle) &hVecList, _Pr _Pred )
	{
		DNVector(CEtWorldProp*) pVecList;
		CEtWorld::ScanProp( vPos, fRadius, &pVecList );

		hVecList.reserve( pVecList.size() );
		for( DNVector(CEtWorldProp*)::iterator itor = pVecList.begin() ; itor!=pVecList.end() ;  )
		{
			itor = std::find_if( itor, pVecList.end(), _Pred );
			if( itor != pVecList.end() )
			{
				CDnWorldProp *pProp = static_cast<CDnWorldProp*>(*itor);
				hVecList.push_back( pProp->GetMySmartPtr() );
				++itor;
			}
		}

		return (int)hVecList.size();
	}
	int ScanProp( EtVector3 &vPos, float fRadius, DNVector(CEtWorldProp*) &pVecList )
	{
		CEtWorld::ScanProp( vPos, fRadius, &pVecList );
		return (int)pVecList.size();
	}

	void SetLandEnvironment( EWorldEnum::LandEnvironment Type ) { m_LandEnvironment = Type; }
	EWorldEnum::LandEnvironment GetLandEnvironment() { return m_LandEnvironment; }

	void SetCurrentWeather( EWorldEnum::WeatherEnum Weather ) { m_CurrentWeather = Weather; }
	EWorldEnum::WeatherEnum GetCurrentWeather() { return m_CurrentWeather; }
	EWorldEnum::WeatherEnum GetNextWeather( int nMapTableID );

	CDNGameRoom *GetCurrentTask() { return m_pRoom; }

//	DWORD AddPropUniqueID() { return m_dwPropUniqueCount++; }

	void SetMapType( EWorldEnum::MapTypeEnum MapType ) { m_MapType = MapType; }
	EWorldEnum::MapTypeEnum GetMapType() { return m_MapType; }
	void SetMapSubType( EWorldEnum::MapSubTypeEnum MapSubType ) { m_MapSubType = MapSubType; }
	EWorldEnum::MapSubTypeEnum GetMapSubType() { return m_MapSubType; }
#if defined(PRE_ADD_ACTIVEMISSION)
	void SetMaxPartyCount(int nMaxPartyCount) { m_nMaxPartyCount = nMaxPartyCount; }
	int GetMaxPartyCount() { return m_nMaxPartyCount; }
#endif

	void InitializeGateInfo( int nCurrentMapIndex, int nArrayIndex );
	bool RefreshGate( int nCurrentMapIndex );
	void RefreshDungeonDifficult( int nCurrentMapIndex );
	bool GetPermitMapCondition(int nCurrentMapIdx, std::vector <int> &vPassClassList, int &nPermitMapLevel);
#if !defined(PRE_ADD_CHALLENGE_DARKLAIR)
	bool IsCloseGateByTime(int nItemID);
#endif

	GateStruct *GetGateStruct( char cGateIndex );
	EWorldEnum::PermitGateEnum GetPermitGate( int nGateIndex );
	void SetPermitGate( char cGateIndex, EWorldEnum::PermitGateEnum PermitFlag );
	DWORD GetGateCount() { return (DWORD)m_pVecGateList.size(); }
	GateStruct *GetGateStructFromIndex( DWORD dwIndex ) { return m_pVecGateList[dwIndex]; }

	// 2009.7.30 hanki
	// Return y position accounting for prop height
	float GetHeightWithProp( const EtVector3 &vPos, NavigationCell *pCurCell = NULL, EtVector3 *pNormal = NULL );
	//

#if defined(PRE_FIX_55855)
	//#55855 Higher value between terrain/prop navmesh (decal handling issue).
	float GetMaxHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell = NULL, EtVector3 *pNormal = NULL, int nTileScale = 1);
#endif // PRE_FIX_55855

	void RevisionPosByNaviMesh( MatrixEx& Cross, const EtVector3& vPrevPos );
	bool IsOnNavigationMesh( const EtVector3& vPos );

	//////////////////////////////////////////////////////////////////////////
	// Code for checking Sea Dragon Nest [2011/01/12 semozz]
protected:
	eDragonNestType m_DragonNestType;
public:
	void SetDragonNestType(eDragonNestType _type) { m_DragonNestType = _type; }
	eDragonNestType GetDragonNestType() { return m_DragonNestType; }
	//////////////////////////////////////////////////////////////////////////
};
