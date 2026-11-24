#include "stdafx.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "DnWorldProp.h"
#include "DnActor.h"
#include "DnTableDB.h"
#include "DnTrigger.h"
#include "DnPartyTask.h"
#include "DnPlayerActor.h"
#include "DnWorldActProp.h"
#include "DNUserSession.h"
#include "shlwapi.h"
#include "navigationmesh.h"
#include "DnWorldGateStruct.h"
#include "DNGameDataManager.h"
#include "TimeSet.h"

EWorldEnum::WeatherEnum CDnWorld::s_WeatherDefine[EWorldEnum::WeatherEnum_Amount][5] = {
	{ EWorldEnum::FineDay, EWorldEnum::CloudyDay, EWorldEnum::FineGlow, EWorldEnum::CloudyGlow, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::CloudyGlow, EWorldEnum::FineNight, EWorldEnum::CloudyNight, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::CloudyNight, EWorldEnum::FineDawn, EWorldEnum::CloudyDawn, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::FineDay, EWorldEnum::CloudyDay, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::FineDay, EWorldEnum::CloudyDay, EWorldEnum::RainDay, EWorldEnum::HeavyrainDay, EWorldEnum::CloudyGlow },
	{ EWorldEnum::FineNight, EWorldEnum::CloudyNight, EWorldEnum::RainNight, EWorldEnum::HeavyrainNight, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::RainNight, EWorldEnum::HeavyrainNight, EWorldEnum::FineDawn, EWorldEnum::CloudyDawn, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::FineDay, EWorldEnum::CloudyDay, EWorldEnum::RainDay, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::FineDay, EWorldEnum::CloudyDay, EWorldEnum::RainDay, EWorldEnum::HeavyrainDay, EWorldEnum::CloudyGlow },
	{ EWorldEnum::CloudyNight, EWorldEnum::HeavyrainNight, EWorldEnum::CloudyDawn, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::CloudyDay, EWorldEnum::RainDay, EWorldEnum::HeavyrainDay, EWorldEnum::CloudyGlow, (EWorldEnum::WeatherEnum)-1 },
	{ EWorldEnum::RainNight, EWorldEnum::CloudyDawn, (EWorldEnum::WeatherEnum)-1 },
};

CDnWorldImp::CDnWorldImp()
{
}

CDnWorldImp::~CDnWorldImp()
{
}

CEtWorldGrid *CDnWorldImp::AllocGrid()
{
	return new CDnWorldGrid( this );
}

bool CDnWorldImp::Initialize( const char *szWorldFolder, const char *szGridName )
{
	// Add Resource Path 
	char szPath[_MAX_PATH];

	sprintf_s( szPath, "%s\\Resource\\Tile", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Prop", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Envi", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Trigger", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	return CEtWorld::Initialize( szWorldFolder, szGridName );
}

void CDnWorldImp::Destroy()
{
	CEtWorld::Destroy();
}

CDnWorld::CDnWorld( CMultiRoom *pRoom )
: CMultiSingleton<CDnWorld, MAX_SESSION_COUNT>( pRoom )
{
//	m_dwPropUniqueCount = -1;
	m_LandEnvironment = (EWorldEnum::LandEnvironment)-1;
	m_CurrentWeather = EWorldEnum::WeatherEnum::FineDay;
	m_MapType = EWorldEnum::MapTypeUnknown;
	m_MapSubType = EWorldEnum::MapSubTypeNone;
	m_pRoom = (CDNGameRoom*)pRoom;

	// World default settings [2011/01/12 semozz]
	m_DragonNestType = eDragonNestType::None;
#if defined(PRE_ADD_ACTIVEMISSION)
	m_nMaxPartyCount = 0;
#endif
}

CDnWorld::~CDnWorld()
{
}

bool CDnWorld::Initialize( const char *szWorldFolder, const char *szGridName )
{
	// Add Resource Path 
	char szPath[_MAX_PATH];


	sprintf_s( szPath, "%s\\Resource\\Tile", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Prop", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Envi", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );

	sprintf_s( szPath, "%s\\Resource\\Trigger", szWorldFolder );
	CEtResourceMng::GetInstance().AddResourcePath( szPath, true );


	return CEtWorld::Initialize( szWorldFolder, szGridName );
}

void CDnWorld::Destroy()
{
	CEtWorld::Destroy();
	CDnWorldProp::DeleteAllObject( GetRoom() );
	CDnWorldProp::Reset(GetRoom());
	SAFE_DELETE_PVEC( m_pVecGateList );

//	m_dwPropUniqueCount = 0;
}

void CDnWorld::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CEtWorld::Process( LocalTime, fDelta );
}

CEtWorldGrid *CDnWorld::AllocGrid()
{
	return new CDnWorldGrid( this );
}

/*
int CDnWorld::ScanProp( EtVector3 &vPos, float fRadius, PropTypeEnum PropType, std::vector<DnPropHandle> &hVecList )
{
	std::vector<CEtWorldProp *> pVecList;
	CEtWorld::ScanProp( vPos, fRadius, &pVecList );
	for( DWORD i=0; i<pVecList.size(); i++ ) {
		CDnWorldProp *pProp = (CDnWorldProp *)pVecList[i];
		if( PropType == -1 || pProp->GetPropType() == PropType ) {
			hVecList.push_back( pProp->GetMySmartPtr() );
		}
	}
	return (int)hVecList.size();
}
*/

EWorldEnum::WeatherEnum CDnWorld::GetNextWeather( int nMapTableID )
{
	std::vector<EWorldEnum::WeatherEnum> VecWeather;
	for( int i=0; i<5; i++ ) {
		if( s_WeatherDefine[m_CurrentWeather][i] == (EWorldEnum::WeatherEnum)-1 ) break;
		VecWeather.push_back( s_WeatherDefine[m_CurrentWeather][i] );
	}
//	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pWeatherSox = GetDNTable( CDnTableDB::TWEATHER );
	int *nWeatherRandomCount = new int[VecWeather.size()];
	int nOffsetMax = 0;
	DNTableCell Field;
	char szEnviField[64];
	for( DWORD i=0; i<VecWeather.size(); i++ ) {
		sprintf_s( szEnviField, "_Envi%d_Prob", VecWeather[i] + 1 );
		pWeatherSox->GetFieldFromLable( nMapTableID, szEnviField, Field );

		nOffsetMax += (int)(Field.GetFloat() * 100.f );
		nWeatherRandomCount[i] = nOffsetMax;
	}
	if( nOffsetMax == 0 )
		nOffsetMax = 1;
	int nRandom = _rand(GetRoom())%nOffsetMax;
	DWORD i;
	for( i=0; i<VecWeather.size(); i++ ) {
		if( nRandom < nWeatherRandomCount[i] ) break;
	}

	SAFE_DELETEA( nWeatherRandomCount );
	return s_WeatherDefine[ m_CurrentWeather ][ i ];
}

void CDnWorld::InitializeGateInfo( int nCurrentMapIndex, int nArrayIndex )
{
	SAFE_DELETE_PVEC( m_pVecGateList );
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONMAP );
	char szLabel[64];
	std::string szTemp;
	std::vector<CEtWorldEventArea *> VecArea;
	int nTemp;

	for( int i=0; i<DNWORLD_GATE_COUNT; i++ ) { 
		GateStruct *pStruct = NULL;

		// For world zone and dungeon move gates, use only the first gate number.
		sprintf_s( szLabel, "_Gate%d_MapIndex_txt", i + 1 );
		char* pszGateMapIndexs = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetString();
		std::vector<string> vGateMapIndexs;
		TokenizeA(pszGateMapIndexs, vGateMapIndexs, ";");
		int nMapIndex = atoi(vGateMapIndexs[0].c_str());
		if( nMapIndex < 1 ) continue;

		if( nMapIndex < DUNGEONGATE_OFFSET ) {
			pStruct = CalcMapInfo( nMapIndex );
			if( pStruct == NULL ) continue;

			sprintf_s( szLabel, "_Gate%d_StartGate_txt", i + 1 );
			char* pszGateStartIndexs = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetString();
			std::vector<string> vGateStartIndexs;
			TokenizeA(pszGateStartIndexs, vGateStartIndexs, ";");
			pStruct->cStartGateIndex = (CHAR)atoi(vGateStartIndexs[0].c_str());
		}
		else {
			if( pDungeonSox->IsExistItem( nMapIndex ) == false ) continue;

			pStruct = new DungeonGateStruct;
			pStruct->nMapIndex = nMapIndex;
			pStruct->MapType = EWorldEnum::MapTypeDungeon;

			for( int j=0; j<5; j++ ) {
				sprintf_s( szLabel, "_MapIndex%d", j + 1 );
				nTemp = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();
				if( nTemp < 1 ) continue;

				GateStruct *pStructSub = NULL;
				pStructSub = CalcMapInfo( nTemp );
				if( pStructSub == NULL ) continue;

				CalcDungeonConstructionLevel(nTemp, ((DungeonGateStruct*)pStructSub));

				sprintf_s( szLabel, "_Map%d_StartGate", j + 1 );
				pStructSub->cStartGateIndex = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();
				((DungeonGateStruct*)pStruct)->pVecMapList.push_back( pStructSub );
			}
		}
		pStruct->cGateIndex = i + 1;
		pStruct->nMapIndex = nMapIndex;

		sprintf_s( szLabel, "_Gate%d_PropIndex%d_txt", i + 1, nArrayIndex + 1 );
		char* pszGatePropIDs = pSox->GetFieldFromLablePtr( nCurrentMapIndex, szLabel )->GetString();
		std::vector<string> vGatePropIDs;
		TokenizeA(pszGatePropIDs, vGatePropIDs, ";");
		if (vGatePropIDs.size() > 0)
			pStruct->dwGatePropID = atoi(vGatePropIDs[0].c_str());
		else
			pStruct->dwGatePropID = 0;

		VecArea.clear();
		sprintf_s( szLabel, "Gate %d", i + 1 );
		CDnWorld::GetInstance(GetRoom()).FindEventAreaFromName( ETE_EventArea, szLabel, &VecArea );
		if( VecArea.empty() ) pStruct->pGateArea = NULL;
		else pStruct->pGateArea = VecArea[0];

		m_pVecGateList.push_back( pStruct );
	}
}

void CDnWorld::RefreshDungeonDifficult( int nCurrentMapIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );

	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		if( m_pVecGateList[i]->MapType == EWorldEnum::MapTypeDungeon ) {
			DungeonGateStruct *pGateStruct = (DungeonGateStruct *)m_pVecGateList[i];
			for( DWORD k=0; k<pGateStruct->pVecMapList.size(); k++ ) {
				int nTemp = pSox->GetFieldFromLablePtr( pGateStruct->pVecMapList[k]->nMapIndex, "_EnterConditionTableID" )->GetInteger();
				if( nTemp > 0 ) {
					DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pGateStruct->pVecMapList[k];
					DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
					if( pDungeonSox->IsExistItem( nTemp ) ) 
					{
						CalcDungeonConstructionLevel( pGateStruct->pVecMapList[k]->nMapIndex, pDungeonStruct );
					}
				}
			}
		}
	}
}

bool CDnWorld::GetPermitMapCondition(int nCurrentMapIdx, std::vector <int> &vPassClassList, int &nPermitMapLevel)
{
	if (GetMapType() != GlobalEnum::MAP_WORLDMAP)
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );

	std::vector<int> nVecItemList;
	pSox->GetItemIDListFromField( "_MapIndex", nCurrentMapIdx, nVecItemList );

	if (nVecItemList.size() > 1)
		_DANGER_POINT();		//Must not have two or more map indices

	std::vector<int>::iterator ii = nVecItemList.begin();
	if (ii != nVecItemList.end())
	{
		int ntemp = pSox->GetFieldFromLablePtr((*ii), "_GateIndex")->GetInteger();
		nPermitMapLevel = pSox->GetFieldFromLablePtr((*ii), "_PermitClassLevel")->GetInteger();
		char * pPassPtr = pSox->GetFieldFromLablePtr((*ii), "_PassClass")->GetString();
		if (pPassPtr)
		{
			std::vector <std::string> vToken;
			TokenizeA(pPassPtr, vToken, ";");
			for ( UINT i = 0; i < vToken.size(); i++)
				vPassClassList.push_back(atoi(vToken[i].c_str()));
		}
	}
	return true;
}

void CDnWorld::CalcDungeonConstructionLevel( int nMapIndex, DungeonGateStruct *pDungeonStruct )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	int nTemp = pSox->GetFieldFromLablePtr( nMapIndex, "_EnterConditionTableID" )->GetInteger();

	pDungeonStruct->cCanDifficult[4] = TRUE;
	for( DWORD j=0; j<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); j++ ) {
		CDNGameRoom::PartyStruct *pStruct = CDnPartyTask::GetInstance(GetRoom()).GetPartyData(j);
		//#62499 Sea Dragon / Green Dragon Hardcore entry disallowed.. remove the weird check below (keep level check)
		/*
		// Check this only when leader. -> Turns out all must do it..
		if( pStruct->bLeader )
			pStruct->pSession->CalcDungeonEnterLevel( nMapIndex, pDungeonStruct->cCanDifficult[2], pDungeonStruct->cCanDifficult[3] );
		else pStruct->pSession->CheckDungeonEnterLevel( nMapIndex );
		*/
#if !defined(PRE_DELETE_DUNGEONCLEAR)
		pStruct->pSession->CheckDungeonEnterLevel( nMapIndex );
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

		// Check Abyss mode.
		if( pDungeonStruct->cCanDifficult[4] == TRUE ) {
			if( pDungeonStruct->nAbyssMinLevel != -1 && pStruct->pSession->GetLevel() < pDungeonStruct->nAbyssMinLevel )
				pDungeonStruct->cCanDifficult[4] |= 0x10;
			if( pDungeonStruct->nAbyssMaxLevel != -1 && pStruct->pSession->GetLevel() > pDungeonStruct->nAbyssMaxLevel )
				pDungeonStruct->cCanDifficult[4] |= 0x10;
			if( pStruct->bLeader ) {
				if( pDungeonStruct->nAbyssNeedQuestID > 0 && !pStruct->pSession->GetQuest()->IsClearQuest( pDungeonStruct->nAbyssNeedQuestID ) )
					pDungeonStruct->cCanDifficult[4] |= 0x20;
			}
		}
	}

	int nValue = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanEasyLevel" )->GetInteger();
	if( nValue > 0 ) pDungeonStruct->cCanDifficult[0] = ( pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanEasyLevel" )->GetInteger() == 1 ) ? false : true;
	nValue = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanNormalLevel" )->GetInteger();
	if( nValue > 0 ) pDungeonStruct->cCanDifficult[1] = ( pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanNormalLevel" )->GetInteger() == 1 ) ? false : true;
	nValue = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanHardLevel" )->GetInteger();
	if( nValue > 0 ) pDungeonStruct->cCanDifficult[2] = ( pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanHardLevel" )->GetInteger() == 1 ) ? false : true;
	nValue = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanVeryHardLevel" )->GetInteger();
	if( nValue > 0 ) pDungeonStruct->cCanDifficult[3] = ( pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanVeryHardLevel" )->GetInteger() == 1 ) ? false : true;
	nValue = pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanNightmareLevel" )->GetInteger();
	if( nValue > 0 ) pDungeonStruct->cCanDifficult[4] = ( pDungeonSox->GetFieldFromLablePtr( nTemp, "_CanNightmareLevel" )->GetInteger() == 1 ) ? false : true;
}

GateStruct *CDnWorld::GetGateStruct( char cGateIndex )
{
	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		if( m_pVecGateList[i]->cGateIndex == cGateIndex ) return m_pVecGateList[i];
	}
	return NULL;
}

#if !defined(PRE_ADD_CHALLENGE_DARKLAIR)
bool CDnWorld::IsCloseGateByTime(int nItemID)
{
	if (nItemID <= 0)
		return false;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );
	
	char* pszDateTime = pSox->GetFieldFromLablePtr( nItemID, "_DateTime" )->GetString();

	std::vector<string> vGateDateTime;
	TokenizeA(pszDateTime , vGateDateTime, ":");

	int nType = 0, nAttr1 = 0, nAttr2 = 0, nAttr3 = 0;
	if (vGateDateTime.size() > 0)
	{
		CTimeSet LocalSet;
		nType = atoi(vGateDateTime[0].c_str());
		if (nType == 0 && vGateDateTime.size() == 3)	// Time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());

			if (nAttr1 <= LocalSet.GetHour() && LocalSet.GetHour() < nAttr2)
				return false;
			else
				return true;
		}
		else if (nType == 1 && vGateDateTime.size() == 2) // Day of week
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());		
			if (LocalSet.GetDayOfWeek() != nAttr1)
				return true;
		}
		else if (nType == 2 && vGateDateTime.size() == 4) // Day of week & time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());
			nAttr3 = atoi(vGateDateTime[3].c_str());

			if (LocalSet.GetDayOfWeek() != nAttr1)
				return true;

			if (nAttr2 <= LocalSet.GetHour() && LocalSet.GetHour() < nAttr3)
				return false;
			else
				return true;
		}
		else if (nType == 3 && vGateDateTime.size() == 2) // Day-of-week fatigue time
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			if( LocalSet.GetDayOfWeek() != nAttr1 && LocalSet.GetDayOfWeek() != (nAttr1+1)%7 ) 
				return true;

			DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSCHEDULE );
			if( !pSox )
				return true;

			int nItemID = CDNSchedule::ResetFatigue +1;
			if( !pSox->IsExistItem( nItemID ) ) return true;
			int nHour = pSox->GetFieldFromLablePtr( nItemID, "_Hour" )->GetInteger();
			int nMinute = pSox->GetFieldFromLablePtr( nItemID, "_Minute" )->GetInteger();

			if( LocalSet.GetDayOfWeek() == nAttr1 )	//Check if after reset time
			{
				if( LocalSet.GetHour() >= nHour && LocalSet.GetMinute() >= nMinute )
					continue;
				else
					return true;
			}
			else if( LocalSet.GetDayOfWeek() == (nAttr1+1)%7 )	//Check if before reset time
			{
				if( LocalSet.GetHour() < nHour || (LocalSet.GetMinute() < nMinute && LocalSet.GetHour() <= nHour) )
					continue;
				else
					return true;
			}
		}
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
		else if (nType == 10 && vGateDateTime.size() == 4) // Close only on that day & time every month
		{
			nAttr1 = atoi(vGateDateTime[1].c_str());
			nAttr2 = atoi(vGateDateTime[2].c_str());
			nAttr3 = atoi(vGateDateTime[3].c_str());

			if (LocalSet.GetDay() != nAttr1)
				return false;

			if (LocalSet.GetHour() >= nAttr2 && LocalSet.GetHour() < nAttr3)
				return true;
			else
				return false;
		}
#endif
	}
	
	return false;
}
#endif

bool CDnWorld::RefreshGate( int nCurrentMapIndex )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAPGATE );

	std::vector<int> nVecItemList;
	pSox->GetItemIDListFromField( "_MapIndex", nCurrentMapIndex, nVecItemList );

	GateStruct *pStruct;

	std::vector<char> cVecPrevPermit;
	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		cVecPrevPermit.push_back( m_pVecGateList[i]->PermitFlag );
	}

	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		pStruct = m_pVecGateList[i];
		pStruct->PermitFlag = EWorldEnum::PermitEnter;
		pStruct->cVecLessLevelActorIndex.clear();
		pStruct->cVecNotEnoughItemActorIndex.clear();
		pStruct->cVecExceedTryActorIndex.clear();
		pStruct->cVecNotRideVehicleActorIndex.clear();

#if defined (PRE_WORLDCOMBINE_PARTY)
		// Block everything except the target dungeon.
		CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(GetRoom());
		if (pGameRoom->bIsWorldCombineParty())
			pStruct->PermitFlag = EWorldEnum::PermitClose;
#endif

		if( pStruct->nMapIndex >= DUNGEONGATE_OFFSET ) {
			DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pStruct;

			bool bValidEnter = false;
			for( DWORD j=0; j<pDungeonStruct->pVecMapList.size(); j++ ) {
				((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag = ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->CanEnterDungeon( GetRoom(), &pStruct->cVecLessLevelActorIndex, &pStruct->cVecNotEnoughItemActorIndex, &pStruct->cVecExceedTryActorIndex , &pStruct->cVecNotRideVehicleActorIndex );

#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
				if(g_pDataManager->IsCloseGateByTime(((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->nMapIndex))
				{
#if defined(PRE_FIX_72521)	//Restrict to cases where entry is allowed and only the time is blocked					
					if( ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag == EWorldEnum::PermitEnter )
#endif
						bValidEnter = true; // Dungeon gate is always opened, but individual dungeons are disabled.

					((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag = EWorldEnum::PermitClose;
				}
#else	//#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
				std::vector<int> nVecDeungeonItemList;
				pSox->GetItemIDListFromField( "_MapIndex", ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->nMapIndex, nVecDeungeonItemList );
				for( DWORD k=0; k<nVecDeungeonItemList.size(); k++ ) 
				{
					if (IsCloseGateByTime(nVecDeungeonItemList[k]))
					{
#if defined(PRE_FIX_72521)
						//Restrict to cases where entry is allowed and only the time is blocked
						if( ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag == EWorldEnum::PermitEnter )
#endif
							bValidEnter = true; // Dungeon gate is always opened, but individual dungeons are disabled.

						((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag = EWorldEnum::PermitClose;
					}
				}
#endif	//#if defined(PRE_ADD_CHALLENGE_DARKLAIR)

				if( ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag == EWorldEnum::PermitEnter ) {
					bValidEnter = true;
				}
			}
			if( bValidEnter == false ) {
				if( pDungeonStruct->pVecMapList.empty() ) pStruct->PermitFlag = EWorldEnum::PermitClose;
				for( DWORD j=0; j<pDungeonStruct->pVecMapList.size(); j++ ) {
					pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)(pStruct->PermitFlag | ((DungeonGateStruct *)pDungeonStruct->pVecMapList[j])->PermitFlag );
				}
			}
#if defined (PRE_WORLDCOMBINE_PARTY)
			else
				pStruct->PermitFlag = EWorldEnum::PermitEnter;
#endif
		}
		else {	//If portal leads to world map/village or dungeon-to-dungeon (not all of the below may be needed...)
			int nItemID = -1;
			for( DWORD j=0; j<nVecItemList.size(); j++ ) {
				int nGateIndex = pSox->GetFieldFromLablePtr( nVecItemList[j], "_GateIndex" )->GetInteger();
				if( pStruct->cGateIndex == nGateIndex ) {
					nItemID = nVecItemList[j];
					break;
				}
			}

#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			if(g_pDataManager->IsCloseGateByTime(pStruct->nMapIndex))
				pStruct->PermitFlag = EWorldEnum::PermitClose;
#else	//#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
			if (IsCloseGateByTime(nItemID))
				pStruct->PermitFlag = EWorldEnum::PermitClose;
#endif	//#if defined(PRE_ADD_CHALLENGE_DARKLAIR)

			if( nItemID != -1 ) {
				bool bValidPlayerLevel = false;
				bool bValidNestTryCount = true;

				int nPermitLevel = pSox->GetFieldFromLablePtr( nItemID, "_PermitPlayerLevel" )->GetInteger();

				for( DWORD j=0; j<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); j++ ) {
					CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(j);

					//Skip check if GM dropped in
					if (pSession->bIsGMTrace() || pSession->bIsAIBot()) continue;

					// Player Level Check
					// Attribute used to always be updated.. strangely it doesn't now.
					int nLevel = pSession->GetLevel();
					if( pSession->GetActorHandle() ) nLevel = pSession->GetActorHandle()->GetLevel();

					if( nLevel >= nPermitLevel ) {
						if( j == 0 || bValidPlayerLevel == true ) bValidPlayerLevel = true;
					}
					else {
						bValidPlayerLevel = false;
						pStruct->cVecLessLevelActorIndex.push_back((char)j);
					}
					
					// Must check entry limit count for nests.
					switch( pStruct->MapSubType ) {
						case EWorldEnum::MapSubTypeNest:
							{
								int nExpandNestClearCount = pSession->m_nExpandNestClearCount;
								int nPCBangExpandNestClearCount = g_pDataManager->GetPCBangNestClearCount(pSession->GetPCBangGrade(), pStruct->nMapIndex);

								if (!pStruct->bExpandable){
									nExpandNestClearCount = 0;
									nPCBangExpandNestClearCount = 0;
								}
								if( pSession->GetNestClearTotalCount( pStruct->nMapIndex ) >= (pStruct->nMaxTryCount + nExpandNestClearCount + nPCBangExpandNestClearCount) ) {
									bValidNestTryCount = false;
									pStruct->cVecExceedTryActorIndex.push_back((char)j);
								}
							}
							break;

#if defined(PRE_ADD_TSCLEARCOUNTEX)
						case EWorldEnum::MapSubTypeTreasureStage:
							{
								int nExpandTreasureStageClearCount = pSession->m_nExpandTreasureStageClearCount;

								if (!pStruct->bExpandable){
									nExpandTreasureStageClearCount = 0;
								}
								if( pSession->GetNestClearTotalCount( pStruct->nMapIndex ) >= (pStruct->nMaxTryCount + nExpandTreasureStageClearCount) ) {
									bValidNestTryCount = false;
									pStruct->cVecExceedTryActorIndex.push_back((char)j);
								}
							}
							break;
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)


						case EWorldEnum::MapSubTypeEvent:
						case EWorldEnum::MapSubTypeChaosField:
						case EWorldEnum::MapSubTypeDarkLair:
#if !defined(PRE_ADD_TSCLEARCOUNTEX)
#if defined( PRE_ADD_SUBTYPE_TREASURESTAGE )
						case EWorldEnum::MapSubTypeTreasureStage:
#endif
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)
#if defined(PRE_ADD_DRAGON_FELLOWSHIP)
						case EWorldEnum::MapSubTypeFellowship:
#endif	// #if defined(PRE_ADD_DRAGON_FELLOWSHIP)
							if( pSession->GetNestClearTotalCount( pStruct->nMapIndex ) >= pStruct->nMaxTryCount ) {
								bValidNestTryCount = false;
								pStruct->cVecExceedTryActorIndex.push_back((char)j);
							}
							break;
					}
				}
				if( bValidPlayerLevel && bValidNestTryCount ) continue;

				if( bValidPlayerLevel == false ) pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)(pStruct->PermitFlag | EWorldEnum::PermitLessPlayerLevel ); 
				if( bValidNestTryCount == false ) pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)(pStruct->PermitFlag | EWorldEnum::PermitExceedTryCount ); 
			}
			// Check required item.
			if( pStruct->MapType == EWorldEnum::MapTypeDungeon ) {
				DungeonGateStruct *pDungeonStruct = (DungeonGateStruct *)pStruct;
				pDungeonStruct->PermitFlag = pDungeonStruct->CanEnterDungeon( GetRoom() );
#if defined(PRE_ADD_68838)
				int nNeedItemType = g_pDataManager->GetItemMainType(pDungeonStruct->nNeedItemID);
#endif	// #if defined(PRE_ADD_68838)
				if( pDungeonStruct->nNeedItemID > 0 && pDungeonStruct->nNeedItemCount > 0 ) {
					for( DWORD j=0; j<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); j++ ) {
						CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(j);

						//Skip check if GM dropped in
						if (pSession->bIsGMTrace() || pSession->bIsAIBot()) continue;
#if defined(PRE_ADD_68838)
						if(nNeedItemType == ITEMTYPE_STAGE_COMPLETE_USEITEM)
						{
							//Check if inven + cash inven has enough count to use
							int nUserTicketCount = 0;
							if(g_pDataManager->IsCashItem(nNeedItemType))
								nUserTicketCount = pSession->GetItem()->GetCashItemCountByItemID(pDungeonStruct->nNeedItemID);
							else
								nUserTicketCount = pSession->GetItem()->GetInventoryItemCount(pDungeonStruct->nNeedItemID);
							
							if(nUserTicketCount < pDungeonStruct->nNeedItemCount) 
							{
								pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)( pStruct->PermitFlag | EWorldEnum::PermitNotEnoughItem ); 
								pStruct->cVecNotEnoughItemActorIndex.push_back( (char)j );
							}
						}
						else if( pSession->GetItem()->GetInventoryItemCount( pDungeonStruct->nNeedItemID ) < pDungeonStruct->nNeedItemCount )
						{
							pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)( pStruct->PermitFlag | EWorldEnum::PermitNotEnoughItem ); 
							pStruct->cVecNotEnoughItemActorIndex.push_back( (char)j );
						}
#else
						if( pSession->GetItem()->GetInventoryItemCount( pDungeonStruct->nNeedItemID ) < pDungeonStruct->nNeedItemCount ) {
							pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)( pStruct->PermitFlag | EWorldEnum::PermitNotEnoughItem ); 
							pStruct->cVecNotEnoughItemActorIndex.push_back( (char)j );
						}
#endif  // #if defined(PRE_ADD_68838)
					}
				}
				
				if(pDungeonStruct->bNeedVehicle)
				{
					for( DWORD j=0; j<CDnPartyTask::GetInstance(GetRoom()).GetUserCount(); j++ ) {
						CDNUserSession *pSession = CDnPartyTask::GetInstance(GetRoom()).GetUserData(j);

						bool bExistVehicle = false;
						TVehicle *pEquipVehicle = pSession->GetItem()->GetVehicleEquip();
						if( ( pEquipVehicle && pEquipVehicle->Vehicle[Vehicle::Slot::Body].nItemID > 0 ) || pSession->GetItem()->GetVehicleInventoryCount() > 0 )
							bExistVehicle = true;

						if( !bExistVehicle )
						{
							pStruct->PermitFlag = (EWorldEnum::PermitGateEnum)( pStruct->PermitFlag | EWorldEnum::PermitNotRideVehicle );
							pStruct->cVecNotRideVehicleActorIndex.push_back( (char)j );
						}
					}
				}
			}
		}
	}

	for( DWORD i=0; i<m_pVecGateList.size(); i++ ) {
		if( m_pVecGateList[i]->PermitFlag != cVecPrevPermit[i] ) return true;
	}

	return false;
}

EWorldEnum::PermitGateEnum CDnWorld::GetPermitGate( int nGateIndex )
{
	GateStruct *pStruct = GetGateStruct( (char)nGateIndex );
	if( pStruct == NULL ) return EWorldEnum::PermitEnter;
	return pStruct->PermitFlag;
}

void CDnWorld::SetPermitGate( char cGateIndex, EWorldEnum::PermitGateEnum PermitFlag )
{
	GateStruct *pStruct = GetGateStruct( cGateIndex );
	if( pStruct == NULL ) return;

	// Later, if the door itself deals damage while opening, uncomment this and update the client side as well.
	// Doesn't seem likely to happen right now, so comment it out.
	/*
	EWorldEnum::PermitGateEnum PrevFlag = pStruct->PermitFlag;
	std::string szAction;

	if( ( PrevFlag == EWorldEnum::PermitEnter ) && ( PermitFlag != EWorldEnum::PermitEnter ) ) szAction = "Open_Close";
	else if( ( PrevFlag != EWorldEnum::PermitEnter ) && ( PermitFlag == EWorldEnum::PermitEnter ) ) szAction = "Close_Open";
	else if( ( PrevFlag != EWorldEnum::PermitEnter ) && ( PermitFlag != EWorldEnum::PermitEnter ) ) szAction = "Close";
	else if( ( PrevFlag == EWorldEnum::PermitEnter ) && ( PermitFlag == EWorldEnum::PermitEnter ) ) szAction = "Open";
	*/

	pStruct->PermitFlag = PermitFlag;

	/*
	std::vector<CEtWorldProp *> VecProp;
	CDnWorld::GetInstance(GetRoom()).FindPropFromCreateUniqueID( pStruct->dwGatePropID, &VecProp );

	for( DWORD i=0; i<VecProp.size(); i++ ) {
		CDnWorldActProp *pProp = dynamic_cast<CDnWorldActProp *>(VecProp[i]);
		if( !pProp ) continue;

		pProp->SetActionQueue( szAction.c_str() );
	}
	*/
}

// 2009.7.30 hanki
// Return y position accounting for prop height
float CDnWorld::GetHeightWithProp( const EtVector3 &vPos, NavigationCell *pCurCell, EtVector3 *pNormal/* = NULL */)
{
	float fPropHeight = 0.f;
	EtVector3 pos = vPos;

	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( pos );
	bool bFindCollision = false;
	if( pNavMesh ) 
	{
		if( !pCurCell ) pCurCell = pNavMesh->FindCell( pos );
		if( pCurCell )
		{
			if( pCurCell->GetType() == NavigationCell::CT_PROP ) {
				fPropHeight = pCurCell->GetPlane()->SolveForY( pos.x, pos.z );
				bFindCollision = true;
			}
		}
	}

	float fTerrainHeight = CEtWorld::GetHeight( pos );
	float fResult;
	if( bFindCollision ) {
		float fValue1 = abs(vPos.y - fPropHeight);
		float fValue2 = abs(vPos.y - fTerrainHeight);
		fResult = ( fValue1 >= fValue2 ) ? fTerrainHeight : fPropHeight;
	}
	else fResult = fTerrainHeight;

	return fResult;
}

#if defined(PRE_FIX_55855)
float CDnWorld::GetMaxHeightWithProp( EtVector3 &vPos, NavigationCell *pCurCell, EtVector3 *pNormal/* = NULL */, int nTileScale/* = 1 */)
{
	float fPropHeight = 0.f;

	NavigationMesh *pNavMesh = INSTANCE(CDnWorld).GetNavMesh( vPos );
	bool bFindCollision = false;
	if( pNavMesh ) 
	{
		if( !pCurCell ) pCurCell = pNavMesh->FindCell( vPos );
		if( pCurCell )
		{
			if( pCurCell->GetType() == NavigationCell::CT_PROP ) {
				fPropHeight = pCurCell->GetPlane()->SolveForY( vPos.x, vPos.z );
				bFindCollision = true;
			}
		}
	}

	float fTerrainHeight = CEtWorld::GetHeight( vPos, NULL, nTileScale );

	float fResult = vPos.y;

	//If navmesh height is found, compare with heightmap height and use the higher value.
	if( bFindCollision )
		fResult = ( fTerrainHeight >= fPropHeight ) ? fTerrainHeight : fPropHeight;
	else fResult = fTerrainHeight;

	return fResult;
}
#endif // PRE_FIX_55855

void CDnWorld::RevisionPosByNaviMesh( MatrixEx& Cross, const EtVector3& vPrevPos )
{
	// Prevent going outside the navigation mesh.
	NavigationMesh *pNavMesh = GetNavMesh( Cross.m_vPosition );
	if( !pNavMesh )
	{
		if( GetAttribute( Cross.m_vPosition ) != 0 )
		{
			Cross.m_vPosition = vPrevPos;
		}
		Cross.m_vPosition.y = GetHeight( Cross.m_vPosition );
	}
	else 
	{
		NavigationCell* pCurCell = NULL;
		if( pCurCell == NULL ) 
		{
			pCurCell = pNavMesh->FindClosestCell( Cross.m_vPosition );
			if( ( pCurCell == NULL ) || ( !pCurCell->IsPointInCellCollumn( Cross.m_vPosition ) ) ) 
			{
				pCurCell = NULL;

				//EtVector3 vPrevPos = Cross.m_vPosition;
				if( GetAttribute( Cross.m_vPosition ) != 0 )
				{
					Cross.m_vPosition = vPrevPos;
				}
				Cross.m_vPosition.y = GetHeight( Cross.m_vPosition );
			}
		}
		if( pCurCell )
		{
			//EtVector3 vPrevPos = Cross.m_vPosition;
			//Cross.MoveFrontBack( fValue );
			int nSide = -1;
			NavigationCell *pLastCell = NULL;
			pCurCell->FindLastCollision( const_cast<EtVector3&>(vPrevPos), Cross.m_vPosition, &pLastCell, nSide );
			if( nSide != -1 )
			{
				if( pLastCell->Link( nSide ) == NULL )
				{
					EtVector2 vMoveDir2D( Cross.m_vPosition.x - vPrevPos.x, Cross.m_vPosition.z - vPrevPos.z );
					float fMoveLength = EtVec2Length( &vMoveDir2D );
					vMoveDir2D /= fMoveLength;
					EtVector2 vWallDir2D = pLastCell->Side( nSide )->EndPointB() - pLastCell->Side( nSide )->EndPointA();
					EtVec2Normalize( &vWallDir2D, &vWallDir2D );
					fMoveLength *= EtVec2Dot( &vWallDir2D, &vMoveDir2D );
					Cross.m_vPosition.x = vPrevPos.x + fMoveLength * vWallDir2D.x;
					Cross.m_vPosition.z = vPrevPos.z + fMoveLength * vWallDir2D.y;

					int nNewSide = -1;
					pCurCell->FindLastCollision( const_cast<EtVector3&>(vPrevPos), Cross.m_vPosition, &pLastCell, nNewSide );
					if( nNewSide != -1 )
					{
						if( pLastCell->Link( nNewSide ) )
						{
							NavigationCell *pNewCell;
							pNewCell = pLastCell->Link( nNewSide );
							if( pNewCell->IsPointInCellCollumn( Cross.m_vPosition ) )
							{
								pCurCell = pNewCell;
							}
							else
							{
								Cross.m_vPosition.x = vPrevPos.x;
								Cross.m_vPosition.z = vPrevPos.z;
							}
						}
						else if( !pCurCell->IsPointInCellCollumn( Cross.m_vPosition ) )
						{
							Cross.m_vPosition.x = vPrevPos.x;
							Cross.m_vPosition.z = vPrevPos.z;
						}
					}
				}
				else
				{
					pCurCell = pLastCell->Link( nSide );
				}
			}

			if( pCurCell->GetType() == NavigationCell::CT_PROP )
				Cross.m_vPosition.y = pCurCell->GetPlane()->SolveForY( Cross.m_vPosition.x, Cross.m_vPosition.z );
			else
				Cross.m_vPosition.y = GetHeight( Cross.m_vPosition );
		}
	}
}

bool CDnWorld::IsOnNavigationMesh( const EtVector3& vPos )
{
	EtVector3 pos = vPos;
	NavigationMesh *pNavMesh = GetNavMesh( pos );
	if( !pNavMesh )
	{
		return false;
	}
	else 
	{
		NavigationCell* pCurCell = NULL;
		if( pCurCell == NULL ) 
		{
			pCurCell = pNavMesh->FindClosestCell( pos );
			if( ( pCurCell == NULL ) || ( !pCurCell->IsPointInCellCollumn( pos ) ) ) 
			{
				return false;
			}
		}
	}

	return true;
}