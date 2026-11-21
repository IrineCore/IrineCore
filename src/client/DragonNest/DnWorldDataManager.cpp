#include "StdAfx.h"
#include "DnWorldDataManager.h"
#include "DnWorldData.h"
#include "DnTableDB.h"
#include "DnWorld.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldDataManager::CDnWorldDataManager()
{
}

CDnWorldDataManager::~CDnWorldDataManager()
{

	// Existing map data is std::map<int, std::vector<CDnWorldData*>> holding a vector of world data pointers per map ID
	// Using std::map<std::string, int>::iterator it = MapLoaded.find( szStr ); to skip inserting duplicates.
	// Because duplicate map data caused improper behavior, while improving this part 
	// Cases occur where containers hold the same CDnWorldDate* simultaneously (to prevent duplicate creation), causing an issue on release when deleting an already deleted pointer inside the container.
	// In principle, fixing it properly requires using Shared_ptr or allowing duplicates, which would apply to the whole container and cause performance issues, so better not to do it
	// Set to manage via CDnWorldDate* pointer object manager for now.
	// A bit cumbersome since objects are managed separately.. Later it would be better for m_MapList to reference using indexing.
#ifdef PRE_ADD_RENEW_RANDOM_MAP
	if( m_MapWorldDataPtr.empty() == false )
	{
		std::map< std::string, CDnWorldData* >::iterator it;
		for( it = m_MapWorldDataPtr.begin(); it != m_MapWorldDataPtr.end(); it++ )
			SAFE_DELETE( it->second );

		m_MapWorldDataPtr.clear();
	}

	std::map<int, std::vector<CDnWorldData*>>::iterator it;
	for( it = m_MapList.begin(); it != m_MapList.end(); it++ ) 
		it->second.clear();
#else
	std::map<int, std::vector<CDnWorldData*>>::iterator it;
	for( it = m_MapList.begin(); it != m_MapList.end(); it++ ) {
		SAFE_DELETE_PVEC( it->second );
	}
#endif

	SAFE_DELETE_VEC( m_MapList );
}

bool CDnWorldDataManager::Initialize( const char *szWorldFolder )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );

	int nCount = pSox->GetItemCount();
	char szLabel[64];
	std::map<std::string, int> MapLoaded;

	std::string szStr;
	int nWorldLevel(0);

	for( int i=0; i<nCount; i++ ) {
		int nItemID = pSox->GetItemID(i);

		std::vector<CDnWorldData *> pVecList;
		bool bValid = false;

		for( int j=0; j<10; j++ ) {
			pVecList.push_back( NULL );

			sprintf_s( szLabel, "_ToolName%d", j + 1 );
			szStr = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();
			if( szStr.empty() ) continue;

			std::map<std::string, int>::iterator it = MapLoaded.find( szStr );
			if( it != MapLoaded.end() ) 
			{
				it->second++;

#ifdef PRE_ADD_RENEW_RANDOM_MAP
				std::map< std::string, CDnWorldData* >::iterator world_it = m_MapWorldDataPtr.find( szStr );
				if( world_it != m_MapWorldDataPtr.end() )
				{
					pVecList[j] = world_it->second;
				}
#endif
				continue;
			}

			MapLoaded.insert( make_pair( szStr, 1 ) );
			CDnWorldData *pData = new CDnWorldData( nItemID );
			if( pData->Initialize( szWorldFolder, szStr.c_str() ) == false ) {
				SAFE_DELETE( pData );
				continue;
			}

			// Note : Enter info from map table
			//
			pData->SetMapName( std::wstring(GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( nItemID, "_MapNameID" )->GetInteger() )) );
			pData->SetMiniMapName( szStr );
//			pData->SetWorldLevel( pSox->GetFieldFromLablePtr( nItemID, "_WorldLevel" )->GetInteger() );

			bValid = true;
			pVecList[j] = pData;

#ifdef PRE_ADD_RENEW_RANDOM_MAP
			m_MapWorldDataPtr.insert( make_pair( szStr, pData ) );
#endif
		}

		CDnWorld::MapTypeEnum mapType = (CDnWorld::MapTypeEnum)pSox->GetFieldFromLablePtr(nItemID, "_MapType")->GetInteger();
		if (mapType == CDnWorld::MapTypeDungeon)
		{
			int mapNameId = pSox->GetFieldFromLablePtr( nItemID, "_MapNameID" )->GetInteger();
			char* pMapAreaString = pSox->GetFieldFromLablePtr(nItemID, "_MapArea")->GetString();
			int dungeonEnterTableID = pSox->GetFieldFromLablePtr(nItemID, "_EnterConditionTableID")->GetInteger();
			bool bIncludeBuild = pSox->GetFieldFromLablePtr(nItemID, "_IncludeBuild")->GetInteger() ? true : false;
			int mapAreaIdx = atoi(pMapAreaString);
			if (mapNameId != 0 && mapAreaIdx != 0 && dungeonEnterTableID != 0 && bIncludeBuild)
			{
				SMapAreaInfoUnit unit;
				unit.mapId = nItemID;
				unit.mapNameId = mapNameId;

				std::map<int, std::vector<SMapAreaInfoUnit> >::iterator iter = m_MapAreaList.find(mapAreaIdx);

				bool bNewInsert = true;
				if (iter != m_MapAreaList.end())
				{
					std::vector<SMapAreaInfoUnit>& mapNameList = (*iter).second;
					std::vector<SMapAreaInfoUnit>::iterator mapNameListIter = std::find(mapNameList.begin(), mapNameList.end(), unit);
					if (mapNameListIter == mapNameList.end())
						mapNameList.push_back(unit);
					bNewInsert = false;
				}

				if (bNewInsert)
				{
					std::vector<SMapAreaInfoUnit> mapNameList;
					mapNameList.push_back(unit);

					m_MapAreaList.insert(std::make_pair(mapAreaIdx, mapNameList));
				}
			}

			if (mapNameId != 0)
			{
				std::map<int, std::vector<int> >::iterator mgnIter = m_MapGroupByName.find(mapNameId);
				if (mgnIter != m_MapGroupByName.end())
				{
					std::vector<int>& mapGroup = (*mgnIter).second;
					mapGroup.push_back(nItemID);
				}
				else
				{
					std::vector<int> mapGroup;
					mapGroup.push_back(nItemID);
					m_MapGroupByName.insert(std::make_pair(mapNameId, mapGroup));
				}
			}
		}

		if( !bValid ) continue;
		m_MapList.insert( make_pair( nItemID, pVecList ) );
	}

#ifdef PRE_PARTY_DB
	DNTableFileFormat* pGateTable = GetDNTable(CDnTableDB::TMAPGATE);

	nCount = pGateTable->GetItemCount();
	int i = 0;
	for (; i<nCount; i++)
	{
		int nItemID = pGateTable->GetItemID(i);
		DNTableFileFormat::Cell* pCell = pGateTable->GetFieldFromLablePtr(nItemID, "_MapIndex");
		if (pCell != NULL)
		{
			int mapIdx = pCell->GetInteger();
			if (mapIdx > 0)
			{
				DNTableFileFormat::Cell* pMapTypeCell = pSox->GetFieldFromLablePtr(mapIdx, "_MapType");
				if (pMapTypeCell == NULL)
					continue;

				CDnWorld::MapTypeEnum mapType = (CDnWorld::MapTypeEnum)pMapTypeCell->GetInteger();
				if (mapType == CDnWorld::MapTypeVillage)
					m_VillageMapIndexList.push_back(mapIdx);
			}
		}
	}
#endif

	return true;
}


CDnWorldData *CDnWorldDataManager::GetWorldData( int nMapIndex, int nArrayIndex )
{
	std::map<int, std::vector<CDnWorldData*>>::iterator it = m_MapList.find( nMapIndex );
	if( it == m_MapList.end() ) return NULL;
	if( nArrayIndex < 0 || nArrayIndex >= (int)it->second.size() ) return NULL;
	return it->second[nArrayIndex];
}

const std::vector<CDnWorldDataManager::SMapAreaInfoUnit>* CDnWorldDataManager::GetMapAreaList(int areaToFind) const
{
	std::map<int, std::vector<SMapAreaInfoUnit> >::const_iterator iter = m_MapAreaList.find(areaToFind);
	if (iter != m_MapAreaList.end())
	{
		const std::vector<SMapAreaInfoUnit>& mapNameIdList = (*iter).second;
		return &mapNameIdList;
	}

	return NULL;
}

const std::vector<int>* CDnWorldDataManager::GetMapGroupByName(int mapNamId) const
{
	std::map<int, std::vector<int> >::const_iterator iter = m_MapGroupByName.find(mapNamId);
	if (iter != m_MapGroupByName.end())
	{
		const std::vector<int>& mapIdList = (*iter).second;
		return &mapIdList;
	}

	return NULL;
}

#ifdef PRE_PARTY_DB
const std::vector<int>& CDnWorldDataManager::GetVillageMapIndexList() const
{
	return m_VillageMapIndexList;
}
#endif