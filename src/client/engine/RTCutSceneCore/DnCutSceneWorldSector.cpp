#include "StdAfx.h"
#include "PropHeader.h"
#include "DnCutSceneWorldSector.h"
#include "EternityEngine.h"
#include "DnCutSceneWorldGrid.h"
#include "DNTableFile.h"
#include "DnCutSceneWorldProp.h"
#include "DnCutSceneActProp.h"
#include "DnCutSceneLightProp.h"
#include "DnCutSceneTable.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


using namespace EternityEngine;

CDnCutSceneWorldSector::CDnCutSceneWorldSector(void) : m_pResPathFinder( NULL )
{
}

CDnCutSceneWorldSector::~CDnCutSceneWorldSector(void)
{
	Free();
}

void CDnCutSceneWorldSector::Free( void )
{
	CEtWorldSector::Free();
	SAFE_RELEASE_SPTR( m_Handle );
	SAFE_DELETE_VEC( m_vlpProcessProp );
}


void CDnCutSceneWorldSector::InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, 
												std::vector< std::vector<float> > &fVecLayerTextureDistance, std::vector< std::vector<float> > &fVecLayerTextureRotation, 
												char *szGrassTexture, char *pGrassTable, float *fGrassWidth, float *fGrassHeightMin, float *fGrassHeightMax, float fShakeMin, float fShakeMax )
{
	STerrainInfo Terrain;
	Terrain.nSizeX = GetTileWidthCount() - 1;
	Terrain.nSizeY = GetTileHeightCount() - 1;
	Terrain.pHeight = m_pHeight;
	Terrain.pLayerDensity = pAlphaTable;
	Terrain.fTileSize = m_fTileSize;
	Terrain.fHeightMultiply = m_fHeightMultiply;
	Terrain.fTextureDistance = 5000.0f;
	Terrain.Type = TT_NORMAL;
	memcpy( Terrain.fGrassWidth, fGrassWidth, sizeof(Terrain.fGrassWidth) );
	memcpy( Terrain.fMinGrassHeight, fGrassHeightMin, sizeof(Terrain.fMinGrassHeight) );
	memcpy( Terrain.fMaxGrassHeight, fGrassHeightMax, sizeof(Terrain.fMaxGrassHeight) );
	Terrain.fMinShake = fShakeMin;
	Terrain.fMaxShake = fShakeMax;
	Terrain.pGrassBuffer = pGrassTable;

	EtVector3 vPos = m_Offset;
	vPos.x -= (m_pParentGrid->GetGridWidth() * 100.0f ) / 2.0f;
	vPos.z -= (m_pParentGrid->GetGridHeight() * 100.0f ) / 2.0f;
	Terrain.TerrainOffset = vPos;

	m_Handle = CreateTerrain( &Terrain );
	assert( m_Handle && "Failed to create terrain!" );

	for( int i=0; i<m_nBlockCount; i++ ) {
		m_Handle->ChangeBlockType( (TerrainType)nVecBlockType[i], i );
		for( DWORD j=0; j<szVecLayerTexture[i].size(); j++ ) 
		{
			m_Handle->SetTexture( j, szVecLayerTexture[i][j].c_str(), i );
			m_Handle->SetTextureDistance( j, fVecLayerTextureDistance[i][j], i );
			m_Handle->SetTextureRotation( j, fVecLayerTextureRotation[i][j], i );
		}
	}
	m_Handle->InitializeBlock( -1 );
	m_Handle->SetGrassTexture( szGrassTexture );
	m_Handle->Show( true );
}

int CDnCutSceneWorldSector::GetPropClassID( const char *szFileName )
{
	
	DNTableFileFormat*  pPropTable = CDnCutSceneTable::GetInstance().GetTable( CDnCutSceneTable::PROP_TABLE );
	if( NULL == pPropTable )
		return false;
	int iTableID = pPropTable->GetItemIDFromField( "_Name", szFileName );
	
	// Do not create entries that are not in the table.
	if( iTableID == -1 ) 
		return -1;

	int iPropType = pPropTable->GetFieldFromLablePtr( iTableID, "_ClassID" )->GetInteger();
	////if( iPropType >= CDnCutSceneActProp::LIGHT )
	if( iPropType != PTE_Action &&
		iPropType != PTE_Trap &&
		iPropType != PTE_Operation &&
		iPropType != PTE_Chest &&
		iPropType != PTE_Broken &&
		iPropType != PTE_Npc &&
		iPropType != PTE_BrokenDamage &&
		iPropType != PTE_HitMoveDamageBroken &&
		iPropType != PTE_Buff &&
		iPropType != PTE_BuffBroken &&
		iPropType != PTE_ShooterBroken &&
		iPropType != PTE_MultiDurabilityBrokenProp &&
		iPropType != PTE_KeepOperation &&
		iPropType != PTE_HitStateEffect )
		iPropType = PTE_Static;

	return iPropType;
}

CEtWorldProp* CDnCutSceneWorldSector::AllocProp( int nClass )
{
	CDnCutSceneWorldProp* pProp = NULL;

	switch( nClass )
	{

		case PTE_Static:
			pProp = new CDnCutSceneWorldProp;
			break;

		case PTE_Action:
		case PTE_Trap:
		case PTE_Operation:
		case PTE_Chest:
		case PTE_Broken:
		case PTE_Npc:
		case PTE_BrokenDamage:
		case PTE_HitMoveDamageBroken:
		case PTE_Buff:
		case PTE_BuffBroken:
		case PTE_ShooterBroken:
		case PTE_MultiDurabilityBrokenProp:
		case PTE_KeepOperation:
		case PTE_HitStateEffect:
			{
				pProp = new CDnCutSceneActProp;
				static_cast<CDnCutSceneActProp*>(pProp)->SetResPathFinder( m_pResPathFinder );
			}
			break;

		//case CDnCutSceneWorldProp::LIGHT:
		//	pProp = new CDnCutSceneLightProp;
		//	break;
	}

	return pProp;
}


CEtWorldSound* CDnCutSceneWorldSector::AllocSound()
{
	// Sound later.

	return NULL;
}

void CDnCutSceneWorldSector::InsertProcessProp( CDnCutSceneWorldProp* pProp )
{
	m_vlpProcessProp.push_back( pProp );
}

void CDnCutSceneWorldSector::DeleteProcessProp( CDnCutSceneWorldProp* pProp )
{
	vector<CDnCutSceneWorldProp*>::iterator iter = find( m_vlpProcessProp.begin(), m_vlpProcessProp.end(), pProp );
	if( m_vlpProcessProp.end() != iter )
	{
		m_vlpProcessProp.erase( iter );
	}
}


void CDnCutSceneWorldSector::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_Handle->Show( true );

	// Prop Process
	int iNumProp = (int)m_vlpProcessProp.size();
	for( int iProp = 0; iProp < iNumProp; ++iProp )
	{
		CDnCutSceneWorldProp* pProp = m_vlpProcessProp.at( iProp );

		if( iProp == 16 )
		{
			int i = 0;
			i++;
		}

		pProp->Process( LocalTime, fDelta );

		//if( ((CDnCutSceneActProp*)pProp)->IsDestroy() ) 
		//{
		//	DeleteProp( pProp );
		//	SAFE_DELETE( pProp );
		//	m_vlpProcessProp.erase( m_vlpProcessProp.begin() + iProp );
		//	iProp--;
		//}
	}
}


void CDnCutSceneWorldSector::BakeLightMap( int nLightMapSize, int nBlurSize )
{
	if( m_Handle )
	{
		if( CheckBakedLightMap() ) return;
		m_Handle->BakeLightMap( -1, nLightMapSize, nLightMapSize, ( float )nBlurSize );
	}
}

bool CDnCutSceneWorldSector::CheckBakedLightMap()
{
	bool bReturn = false;
	int i, nStart, nEnd;
	m_Handle->CalcSelectCount( -1, nStart, nEnd );
	char szTemp[_MAX_PATH] = { 0, };
	for( i = nStart; i < nEnd; i++ )
	{
		sprintf_s( szTemp, "%s\\_LM\\_LM%d.dds", m_szSectorPath.c_str(), i );
		EtTextureHandle hTexture = LoadResource( szTemp, RT_TEXTURE );
		if( i == nStart && !hTexture )
		{
			// If it fails initially, load nothing.
			break;
		}
		if( hTexture )
		{
			// Textures created inside _BakeLightMap routine are configured like this.
			hTexture->SetDeleteImmediate( true );

			// If it succeeds even once, it will keep succeeding..
			if( m_Handle->_BakeLightMapFromFile( i, hTexture ) )
				bReturn = true;
			else
				SAFE_RELEASE_SPTR( hTexture );
		}
	}
	return bReturn;
}