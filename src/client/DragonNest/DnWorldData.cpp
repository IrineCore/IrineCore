#include "StdAfx.h"
#include "DnWorldData.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "DnTableDB.h"
#include "PropHeader.h"
#include "DnDataManager.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldData::CDnWorldData( int nMapIndex )
	: m_nMapIndex(nMapIndex)
	, m_nGridWidth(0)
	, m_nGridHeight(0)
{
}

CDnWorldData::~CDnWorldData()
{
	SAFE_DELETE_PVEC( m_pVecNpcData );
	SAFE_DELETE_PVEC( m_pVecGateData );
	SAFE_DELETE_PVEC( m_pVecPlantData );
}

bool CDnWorldData::Initialize( const char *szWorldFolder, const char *szGridName )
{
	CEtWorld *pWorld = new CEtWorld;

	if( pWorld->Initialize( szWorldFolder, szGridName ) == false ) 
	{
		SAFE_DELETE( pWorld );
		return false;
	}

	pWorld->ForceInitialize( CEtWorldSector::LSE_Control | CEtWorldSector::LSE_Prop );

	CEtWorldGrid *pGrid = pWorld->GetGrid();
	for( DWORD j=0; j<pGrid->GetGridY(); j++ ) 
	{
		for( DWORD i=0; i<pGrid->GetGridX(); i++ ) 
		{
			CEtWorldSector *pSector = pGrid->GetSector( SectorIndex(i,j) );
			LoadControl( pSector );
			LoadProp( pSector );
		}
	}

	m_nGridWidth = pWorld->GetGridWidth();
	m_nGridHeight = pWorld->GetGridHeight();

	SAFE_DELETE( pWorld );
	return true;
}

void CDnWorldData::LoadControl( CEtWorldSector *pSector )
{
	DNTableFileFormat* pSoxActor = GetDNTable( CDnTableDB::TACTOR );
	if( !pSoxActor ) return;

	DNTableFileFormat* pSoxNpc = GetDNTable( CDnTableDB::TNPC );
	if( !pSoxNpc ) return;

	DNTableFileFormat* pSoxMap = GetDNTable( CDnTableDB::TMAP );
	if( !pSoxMap ) return;

	DNTableFileFormat* pDungeonSox = GetDNTable( CDnTableDB::TDUNGEONMAP );
	if( !pDungeonSox ) return;

	DNTableFileFormat* pDungeonEnterSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( !pDungeonEnterSox ) return;

	DNTableFileFormat* pSoxGate = GetDNTable( CDnTableDB::TMAPGATE );
	if( !pSoxGate ) return;

	CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( ETE_UnitArea );
	char szName[256];
	int nValue;

	if( pControl ) {
		for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
			CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);
			sscanf_s( pArea->GetName(), "%s %d", szName, 256, &nValue );
			_strlwr_s( szName );

			if( strcmp( szName, "npc" ) == NULL ) {
				NpcData *pData = new NpcData;

				pData->nNpcID = nValue;
				pData->vPos.x = pArea->GetOBB()->Center.x;
				pData->vPos.y = pArea->GetOBB()->Center.z;
				pData->fRotate = pArea->GetRotate();
				int nActorID = pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_ActorIndex" )->GetInteger();
				pData->nUnitSize = max( pSoxActor->GetFieldFromLablePtr( nActorID, "_SizeParam1" )->GetInteger(), pSoxActor->GetFieldFromLablePtr( nActorID, "_SizeParam2" )->GetInteger() );

				// Note : Read info from NPC table.
				//				
				if( pSoxNpc->IsExistItem( pData->nNpcID )  ) {
					pData->strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_NameID" )->GetInteger() );				
					pData->nJobType = pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_NpcJobIndex" )->GetInteger();
				}
				else {
					pData->nJobType = -1;
					OutputDebug(" %d Npc ID is not in the NPC Table~\n", pData->nNpcID );
				}

				m_pVecNpcData.push_back( pData ); 
			}
		}
	}

	pControl = pSector->GetControlFromUniqueID( ETE_EventArea );
	if( pControl ) {
		for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) {
			CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);

			if( strstr( pArea->GetName(), "Gate " ) ) {		// Needs a space here.. haha
				GateData *pData = new GateData;

				pData->vPos.x = pArea->GetOBB()->Center.x;
				pData->vPos.y = pArea->GetOBB()->Center.z;				
				sscanf_s( pArea->GetName(), "Gate %d", &pData->nGateNo, sizeof(int) );

				// Note : Read info from MAP table.
				//
				if( pData->nGateNo > 0 && pData->nGateNo < DNWORLD_GATE_COUNT+1 )
				{
					std::vector<int> vecMapIndex;
					char szLabel[128] = {0};
					CDnWorld::GetGateMapIndex( m_nMapIndex, pData->nGateNo, vecMapIndex );
					
					for( DWORD itr = 0; itr < vecMapIndex.size(); ++itr )
					{
						int nMapIndex = vecMapIndex[itr];
						if( nMapIndex < DUNGEONGATE_OFFSET )
						{
							if( !pSoxMap->IsExistItem( nMapIndex ) ) {
								SAFE_DELETE( pData );
								continue;
							}
							pData->nMapIndex = nMapIndex;
							pData->strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSoxMap->GetFieldFromLablePtr( nMapIndex, "_MapNameID" )->GetInteger() );
							pData->nMapType = pSoxMap->GetFieldFromLablePtr( nMapIndex, "_MapType" )->GetInteger();
							//pData->nLevel = pSoxMap->GetFieldFromLablePtr( nMapIndex, "_WorldLevel" )->GetInteger();

							std::vector<int> nVecItemList;
							pSoxGate->GetItemIDListFromField( "_MapIndex", m_nMapIndex, nVecItemList );
							int nGateItemID = -1;
							for( DWORD j=0; j<nVecItemList.size(); j++ ) {
								int nGateIndex = pSoxGate->GetFieldFromLablePtr( nVecItemList[j], "_GateIndex" )->GetInteger();
								if( pData->nGateNo == nGateIndex ) {
									nGateItemID = nVecItemList[j];
									break;
								}
							}
							if( nGateItemID == -1 ) pData->nLevel = 0;
							else pData->nLevel = pSoxGate->GetFieldFromLablePtr( nGateItemID, "_PermitPlayerLevel" )->GetInteger();
						}
						else
						{
							if( !pDungeonSox->IsExistItem( nMapIndex ) ) {
								SAFE_DELETE( pData );
								continue;
							}
							pData->nMapIndex = nMapIndex;
							pData->strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pDungeonSox->GetFieldFromLablePtr( nMapIndex, "_Title" )->GetInteger() );
							pData->nMapType = CDnWorld::MapTypeDungeon;

							for( int j=0; j<5; j++ )
							{
								sprintf_s( szLabel, "_MapIndex%d", j + 1 );
								int nIndex = pDungeonSox->GetFieldFromLablePtr( nMapIndex, szLabel )->GetInteger();
								if( nIndex < 1 ) continue;

								GateData gateData;
								gateData.nMapIndex = nIndex;
								gateData.strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSoxMap->GetFieldFromLablePtr( nIndex, "_MapNameID" )->GetInteger() );									
								gateData.nLevel = -1;
								int nDungeonEnterTableID = pSoxMap->GetFieldFromLablePtr( nIndex, "_EnterConditionTableID" )->GetInteger();
								if( nDungeonEnterTableID > 0 ) {
									if( pDungeonEnterSox->IsExistItem( nDungeonEnterTableID ) ) {									
										gateData.nLevel = pDungeonEnterSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();
									}
								}
								pData->vecGateData.push_back( gateData );
							}
						}
					}
				}

				if( NULL != pData )
					m_pVecGateData.push_back( pData );
			}
		}
	}

	pControl = pSector->GetControlFromUniqueID( ETE_FarmGrowingArea );
	if( pControl )
	{
		for( DWORD i=0; i<pControl->GetAreaCount(); i++ ) 
		{
			CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(i);

			PlantData *pData = new PlantData;

			pData->vPos.x = pArea->GetOBB()->Center.x;
			pData->vPos.y = pArea->GetOBB()->Center.z;

			sscanf_s( pArea->GetName(), "FarmGrowingArea - %d", &pData->nAreaIndex, sizeof(int) );

			m_pVecPlantData.push_back( pData );
		}
	}
}

void CDnWorldData::LoadProp( CEtWorldSector *pSector )
{
	DNTableFileFormat* pSoxNpc = GetDNTable( CDnTableDB::TNPC );
	if( !pSoxNpc ) return;
	DNTableFileFormat* pSoxProp = GetDNTable( CDnTableDB::TPROP );
	if( !pSoxProp ) return;
	DNTableFileFormat* pSoxActor = GetDNTable( CDnTableDB::TACTOR );
	if( !pSoxActor ) return;

	EtVector2 vOffset = *pSector->GetOffset();
	vOffset.x -= ( pSector->GetTileWidthCount() * pSector->GetTileSize() ) / 2.f;
	vOffset.y -= ( pSector->GetTileHeightCount() * pSector->GetTileSize() ) / 2.f;
	for( DWORD i=0; i<pSector->GetPropCount(); i++ ) {
		CEtWorldProp *pProp = pSector->GetPropFromIndex(i);
		if( !pProp ) continue;
		int nItemID = pSoxProp->GetItemIDFromField( "_Name", pProp->GetPropName() );
		if( nItemID == -1 ) continue;
		switch( pSoxProp->GetFieldFromLablePtr( nItemID, "_ClassID" )->GetInteger() ) {
			case PTE_Npc:
				{
					int nNpcID = ((NpcStruct*)pProp->GetData())->nNpcID;
					if( nNpcID < 1 ) break;
					NpcData *pData = new NpcData;

					pData->nNpcID = nNpcID;
					pData->vPos = vOffset + EtVec3toVec2( *pProp->GetPosition() );
					pData->fRotate = 180.f - ( pProp->GetRotation()->y );
					int nActorID = pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_ActorIndex" )->GetInteger();
					pData->nUnitSize = max( pSoxActor->GetFieldFromLablePtr( nActorID, "_SizeParam1" )->GetInteger(), pSoxActor->GetFieldFromLablePtr( nActorID, "_SizeParam2" )->GetInteger() );

					// Note : Read info from NPC table.
					//				
					if( pSoxNpc->IsExistItem( pData->nNpcID )  ) {
						pData->strName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_NameID" )->GetInteger() );				
						pData->nJobType = pSoxNpc->GetFieldFromLablePtr( pData->nNpcID, "_NpcJobIndex" )->GetInteger();
					}
					else {
						pData->nJobType = -1;
						OutputDebug(" %d Npc ID is not in the NPC Table~\n", pData->nNpcID );
					}

					m_pVecNpcData.push_back( pData ); 
				}
				break;
		}
	}
}

DWORD CDnWorldData::GetNpcCount()
{
	return (DWORD)m_pVecNpcData.size();
}

CDnWorldData::NpcData *CDnWorldData::GetNpcData( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecNpcData.size() ) return NULL;
	return m_pVecNpcData[dwIndex];
}

DWORD CDnWorldData::GetGateCount()
{
	return (DWORD)m_pVecGateData.size();
}

CDnWorldData::GateData *CDnWorldData::GetGateData( DWORD dwIndex )
{
	if( dwIndex < 0 || dwIndex >= m_pVecGateData.size() ) return NULL;
	return m_pVecGateData[dwIndex];
}

CDnWorldData::GateData *CDnWorldData::GetGateDataFromGateNo( int nGateNo )
{
	for( DWORD i=0; i<m_pVecGateData.size(); i++ ) {
		if( m_pVecGateData[i]->nGateNo == nGateNo ) return m_pVecGateData[i];
	}
	return NULL;
}

DWORD CDnWorldData::GetPlantCount()
{
	return (DWORD)m_pVecPlantData.size();
}

CDnWorldData::PlantData * CDnWorldData::GetPlantData( DWORD dwIndex )
{
	for( int itr = 0; itr < (int)m_pVecPlantData.size(); ++itr )
	{
		if( dwIndex == m_pVecPlantData[itr]->nAreaIndex )
			return m_pVecPlantData[itr];
	}

	return NULL;
}

void CDnWorldData::LoadNpcTalk()
{
	for( int itr = 0; itr < (int)m_pVecNpcData.size(); ++itr )
	{
		g_DataManager.LoadNpcTalkData( m_pVecNpcData[itr]->nNpcID );
		g_DataManager.LoadNpcQuestData( m_pVecNpcData[itr]->nNpcID );
	}
}