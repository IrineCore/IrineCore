#include "StdAfx.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"
// Prop Include
#include "DnWorldProp.h"
#include "DnWorldActProp.h"
#include "DnWorldBrokenProp.h"
#include "DnWorldLightProp.h"
#include "DnWorldTrapProp.h"
#include "DnWorldChestProp.h"
#include "DnWorldOperationProp.h"
#include "DnShooterProp.h"
#include "DnWorldNpcProp.h"
#include "DnWorldBrokenDamageProp.h"
#include "DnWorldHitMoveDamageBrokenProp.h"
#include "DnBuffProp.h"
#include "DnWorldBrokenBuffProp.h"
#include "DnWorldCameraProp.h"
#include "DnWorldShooterBrokenProp.h"
#include "DnWorldEnvironmentProp.h"
#include "DnWorldMultiDurabilityProp.h"
#include "DnWorldKeepOperationProp.h"
#include "DnWorldOperationDamageProp.h"
#include "DnWorldHitStateEffectProp.h"

#include "DnWorldSound.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "DnTableDB.h"

#include "DnTrigger.h"
#include "DnWorldWater.h"
#include "DnWorldDecal.h"
#include "PerfCheck.h"

#include "EtCollisionMng.h"
#include "navigationmesh.h"
#include "navigationcell.h"
#include "navigationpath.h"
#include "navigationwaypoint.h"

#include "PropHeader.h"

#include "GameOption.h"
#include "EtOptionController.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

bool g_bNaviDraw = false;
bool g_bRenderBlur = true;
EtVector3 g_vStart;
EtVector3 g_vEnd;

CDnWorldSector::CDnWorldSector()
{
	m_pTileTypePtr = NULL;
	m_nDrawAttributeCount = 50;
} 

CDnWorldSector::~CDnWorldSector()
{
	Free();
}

void CDnWorldSector::Free()
{
	CEtWorldSector::Free();
	SAFE_RELEASE_SPTR( m_Handle );
	SAFE_DELETE_VEC( m_pVecProcessProp );
	SAFE_DELETE( m_pTileTypePtr );
	CEtCollisionMng::GetInstance().Clear();
}

void CDnWorldSector::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_Handle ) return;
	bool bShow = ((CDnWorld*)m_pParentGrid->GetWorld())->IsShow();
	m_Handle->Show( bShow );

	// Prop Process
	for( DWORD i=0; i<m_pVecProcessProp.size(); i++ ) {
		m_pVecProcessProp[i]->Process( LocalTime, fDelta );
		if( ((CDnWorldActProp*)m_pVecProcessProp[i])->IsDestroy() ) {
			CDnWorldProp *pProp = m_pVecProcessProp[i];
			DeleteProp( pProp );
			SAFE_DELETE( pProp );
			m_pVecProcessProp.erase( m_pVecProcessProp.begin() + i );
			i--;
		}
	}
	if( m_pWater && bShow ) m_pWater->Render( LocalTime );

#ifndef _FINAL_BUILD
	static float s_fDelta = 5.0f;
	
	if ( m_pNaviMesh && g_bNaviDraw )
	{
		// Draw navmesh ----------------------------------------------------------------------------
		EtVector3 vPos[3];
		for( int i = 0; i < m_pNaviMesh->TotalCells(); i++ )
		{
			NavigationCell *pCell = m_pNaviMesh->Cell( i );
			memcpy( &vPos[0].x, &pCell->Vertex( 0 ), sizeof(float) * 3 );
			memcpy( &vPos[1].x, &pCell->Vertex( 1 ), sizeof(float) * 3 );
			memcpy( &vPos[2].x, &pCell->Vertex( 2 ), sizeof(float) * 3 );

			vPos[0].y += s_fDelta; 
			vPos[1].y += s_fDelta;
			vPos[2].y += s_fDelta;

			DWORD dwColor;
			dwColor = 0xffffffff;
			if( pCell->GetType() == NavigationCell::CT_PROP )
			{
				dwColor = 0xffff0000;
			}
			else if( pCell->GetType() == NavigationCell::CT_EVENT )
			{
				dwColor = 0xff00ff00;
			}

			EternityEngine::DrawLine3D( vPos[0], vPos[1], dwColor );
			EternityEngine::DrawLine3D( vPos[1], vPos[2], dwColor );
			EternityEngine::DrawLine3D( vPos[2], vPos[0], dwColor );
		}
	}
#endif //_FINAL_BUILD

	// Trigger Process
	if( m_pTrigger ) {
		m_pTrigger->Process( LocalTime, fDelta );
	}
}

CEtWorldProp *CDnWorldSector::AllocProp( int nClass )
{
	CDnWorldProp *pProp = NULL;
	switch( (PropTypeEnum)nClass ) {
		case PTE_Static: pProp = new CDnWorldProp; break;
		case PTE_Action: pProp = new CDnWorldActProp; break;
		case PTE_Broken: pProp = new CDnWorldBrokenProp; break;
		case PTE_Trap: pProp = new CDnWorldTrapProp; break;
		case PTE_Light: pProp = new CDnWorldLightProp; break;
		case PTE_Operation: pProp = new CDnWorldOperationProp; break;
		case PTE_Chest: pProp = new CDnWorldChestProp; break;
		case PTE_ProjectileShooter: pProp = new CDnShooterProp; break;
		case PTE_Npc: pProp = new CDnWorldNpcProp; break;
		case PTE_BrokenDamage: pProp = new CDnWorldBrokenDamageProp; break;
		case PTE_HitMoveDamageBroken: pProp = new CDnWorldHitMoveDamageBrokenProp; break;
		case PTE_Buff: pProp = new CDnBuffProp; break;
		case PTE_BuffBroken: pProp = new CDnWorldBrokenBuffProp; break;
		case PTE_Camera: pProp = new CDnWorldCameraProp; break;
		case PTE_ShooterBroken: pProp = new CDnWorldShooterBrokenProp; break;
		case PTE_EnvironmentProp: pProp = new CDnWorldEnvironmentProp; break;
		case PTE_MultiDurabilityBrokenProp: pProp = new CDnWorldMultiDurabilityProp; break;
		case PTE_KeepOperation: pProp = new CDnWorldKeepOperationProp; break;
		case PTE_OperationDamage: pProp = new CDnWorldOperationDamageProp; break;
		case PTE_HitStateEffect: pProp = new CDnWorldHitStateEffectProp; break; // #60784
		default: pProp = new CDnWorldProp; break;
	}
	if( pProp ) {
		pProp->InitializeSession( ((CDnWorld*)GetParentGrid()->GetWorld())->GetCurrentTask() );
	}
	return pProp;
}

int CDnWorldSector::GetPropTableID( const char *szFileName )
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );
	if( !pSox ) return -1;
	return pSox->GetItemIDFromField( "_Name", szFileName );
}

int CDnWorldSector::GetPropClassID( const char *szFileName )
{
	if( strcmp( szFileName, "Light.skn" ) == NULL ) return PTE_Light;
	if( strcmp( szFileName, "Camera.skn" ) == NULL ) return PTE_Camera;
	int nIndex = GetPropTableID( szFileName );
	if( nIndex == -1 ) return 0;

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TPROP );
	return pSox->GetFieldFromLablePtr( nIndex, "_ClassID" )->GetInteger();
}

CEtWorldDecal *CDnWorldSector::AllocDecal()
{
	return new CDnWorldDecal(this);
}

void CDnWorldSector::InitializeTerrain( DWORD *pAlphaTable, std::vector<int> &nVecBlockType, std::vector< std::vector<std::string> > &szVecLayerTexture, std::vector< std::vector<float> > &fVecLayerTextureDistance, std::vector< std::vector<float> > &fVecLayerTextureRotation, char *szGrassTexture, char *pGrassTable, float *fGrassWidth, float *fGrassHeightMin, float *fGrassHeightMax, float fShakeMin, float fShakeMax )
{
	// Create Terrain
	STerrainInfo Info;
	Info.nSizeX = GetTileWidthCount() - 1;
	Info.nSizeY = GetTileHeightCount() - 1;
	Info.pHeight = m_pHeight;
	Info.pLayerDensity = pAlphaTable;
	Info.fTileSize = m_fTileSize;
	Info.fHeightMultiply = m_fHeightMultiply;
	Info.fTextureDistance = 5000.f;
	Info.Type = TT_NORMAL;
	memcpy( Info.fGrassWidth, fGrassWidth, sizeof(Info.fGrassWidth) );
	memcpy( Info.fMinGrassHeight, fGrassHeightMin, sizeof(Info.fMinGrassHeight) );
	memcpy( Info.fMaxGrassHeight, fGrassHeightMax, sizeof(Info.fMaxGrassHeight) );
	Info.fMinShake = fShakeMin;
	Info.fMaxShake = fShakeMax;
	Info.pGrassBuffer = pGrassTable;

	EtVector3 vPos = m_Offset;
	vPos.x -= ( m_pParentGrid->GetGridWidth() * 100.f ) / 2.f;
	vPos.z -= ( m_pParentGrid->GetGridHeight() * 100.f ) / 2.f;
	Info.TerrainOffset = vPos;

	//////////////////////////////////////////////////////////////////////////
	// For low-spec, move the most frequently used layer textures to 1 and 2.
	// Renders only the 2 most frequently used sheets.
	// If the card cannot even run the 2-layer splatting shader, fall back to the legacy low-spec terrain mode.
	SGraphicOption Option;
	GetEtOptionController()->GetGraphicOption( Option );
	if( false == Option.bUseSplatting &&
		false == Option.bIsOnlyLowShaderAvailable )
	{
		// Default is auto extract.
		int aiSelectedLayerTexture[ 2 ] = { -1, -1 };

		// Check if there is a texture specified by cheat key.
#ifndef _FINAL_BUILD
		if( CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 0 ] != 0 &&
			CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 1 ] != 0 )
		{
			aiSelectedLayerTexture[ 0 ] = CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 0 ] - 1;
			aiSelectedLayerTexture[ 1 ] = CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 1 ] - 1;

			CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 0 ] = 0;
			CGlobalValue::GetInstance().m_aiSelectedMapLayerTexture[ 1 ] = 0;
		}
		else
		{
#endif // #ifndef _FINAL_BUILD

		// Login screen has current map index -1.
		int nMapTableID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
		if( -1 != nMapTableID )
		{
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
			aiSelectedLayerTexture[ 0 ] = pSox->GetFieldFromLablePtr( nMapTableID, "_texture1" )->GetInteger() - 1;
			aiSelectedLayerTexture[ 1 ] = pSox->GetFieldFromLablePtr( nMapTableID, "_texture2" )->GetInteger() - 1;

		}
#ifndef _FINAL_BUILD
		}
#endif

		//// For testing..
		//aiSelectedLayerTexture[ 0 ] = 0;
		//aiSelectedLayerTexture[ 1 ] = 1;

		int nWidth = (int)GetTileWidthCount();
		int nHeight = (int)GetTileHeightCount();

		int nBlockSizeX, nBlockSizeY;
		nBlockSizeX = nBlockSizeY = DEFAULT_TERRAIN_SIZE;
		int nBlockCountX, nBlockCountY;
		nBlockCountX = Info.nSizeX / nBlockSizeX + ((Info.nSizeX % nBlockSizeX) > 0 ? 1 : 0);
		nBlockCountY = Info.nSizeY / nBlockSizeY + ((Info.nSizeY % nBlockSizeY) > 0 ? 1 : 0);
		int nTerrainCount = nBlockCountX * nBlockCountY;

		if( aiSelectedLayerTexture[ 0 ] == -1 &&
			aiSelectedLayerTexture[ 1 ] == -1 )
		{
			D3DXCOLOR* pAlphaAccumulateByTerrain = new D3DXCOLOR[ nTerrainCount ];
			SecureZeroMemory( pAlphaAccumulateByTerrain, sizeof(EtVector4)*nTerrainCount );

			// Load the 2 most used textures for each Terrain.
			// Although seams might be noticeable between Terrain boundaries,,
			// Try to cover most cases for now.
			for( int iY = 0; iY < nHeight; iY++ ) 
			{
				for( int iX = 0; iX < nWidth; iX++ ) 
				{
					int nBlockX = iX / nBlockSizeX;
					int nBlockY = iY / nBlockSizeY;
					if( nBlockX >= nBlockCountX ) nBlockX -= 1;
					if( nBlockY >= nBlockCountY ) nBlockY -= 1;
					int iCurrentTerrainIndex = nBlockY * nBlockCountX + nBlockX;

					int iAlphaTablePos = iY * nWidth + iX;
					D3DXCOLOR Color = D3DXCOLOR( pAlphaTable[iAlphaTablePos] );
					if( iCurrentTerrainIndex < nTerrainCount )
					{
						pAlphaAccumulateByTerrain[ iCurrentTerrainIndex ] += Color;
					}
				}
			}

			// Re-sort each Terrain's layer texture data based on the resulting output.
			for( int i = 0; i < nTerrainCount; ++i )
			{
				D3DXCOLOR& Color = pAlphaAccumulateByTerrain[ i ];
				float afColorTable[4] = { Color.a, Color.r, Color.g, Color.b };

				int aiFrequentlyUsedLayer[ 2 ] = { 0 };
				float afGreatestAlphaValue[ 2 ] = { 0.0f };
				for( int k = 0; k < 4; k++ )
				{
					if( afColorTable[ k ] > afGreatestAlphaValue[ 0 ] ) 
					{
						afGreatestAlphaValue[ 0 ] = afColorTable[ k ];
						aiFrequentlyUsedLayer[ 0 ] = k;
					}
					else
					if( afColorTable[ k ] > afGreatestAlphaValue[ 1 ] )
					{
						afGreatestAlphaValue[ 1 ] = afColorTable[ k ];
						aiFrequentlyUsedLayer[ 1 ] = k;
					}
				}

				// Move the most frequently used layer textures for each Terrain to slots 1 and 2
				// Only textures 1 and 2 are used during actual terrain rendering.
				vector<string>& vlTextureLayersInTerrain = szVecLayerTexture.at( i );
				std::swap( vlTextureLayersInTerrain.at( 0 ), vlTextureLayersInTerrain.at( aiFrequentlyUsedLayer[ 0 ] ) );
				std::swap( vlTextureLayersInTerrain.at( 1 ), vlTextureLayersInTerrain.at( aiFrequentlyUsedLayer[ 1 ] ) );

				vector<float>& vlDistanceInTerrain = fVecLayerTextureDistance.at( i );
				std::swap( vlDistanceInTerrain.at( 0 ), vlDistanceInTerrain.at( aiFrequentlyUsedLayer[ 0 ] ) );
				std::swap( vlDistanceInTerrain.at( 1 ), vlDistanceInTerrain.at( aiFrequentlyUsedLayer[ 1 ] ) );

				vector<float>& vlRotationInTerrain = fVecLayerTextureRotation.at( i );
				std::swap( vlRotationInTerrain.at( 0 ), vlRotationInTerrain.at( aiFrequentlyUsedLayer[ 0 ] ) );
				std::swap( vlRotationInTerrain.at( 1 ), vlRotationInTerrain.at( aiFrequentlyUsedLayer[ 1 ] ) );

				// Also update the alpha table values.
				int iXOffset = i%nBlockCountX;
				int iYOffset = i/nBlockCountX;
				int iOffset = iYOffset*nWidth*DEFAULT_TERRAIN_SIZE + (iXOffset*DEFAULT_TERRAIN_SIZE);
				for( int k = 0; k < DEFAULT_TERRAIN_SIZE*DEFAULT_TERRAIN_SIZE; ++k )
				{
					int iAlphaTablePos = iOffset + (k/DEFAULT_TERRAIN_SIZE)*nWidth + k%DEFAULT_TERRAIN_SIZE;
					if( nHeight*nWidth <= iAlphaTablePos )
						continue;

					D3DXCOLOR LegacyColor = D3DXCOLOR( pAlphaTable[iAlphaTablePos] );
					float fColorTable[4] = { LegacyColor.a, LegacyColor.r, LegacyColor.g, LegacyColor.b };
					float fSwapedColorTable[4] = { LegacyColor.a, LegacyColor.r, LegacyColor.g, LegacyColor.b };
					
					std::swap( fSwapedColorTable[ 0 ], fColorTable[ aiFrequentlyUsedLayer[ 0 ] ] );
					std::swap( fSwapedColorTable[ 1 ], fColorTable[ aiFrequentlyUsedLayer[ 1 ] ] );

					// D3DXCOLOR constructor order is r, g, b, a. 
					// Keep in mind the alpha table storage order is a, r, g, b.
					D3DXCOLOR SwapColor( fSwapedColorTable[ 1 ], fSwapedColorTable[ 2 ],
										 fSwapedColorTable[ 3 ], fSwapedColorTable[ 0 ] );
					pAlphaTable[ iAlphaTablePos ] = SwapColor;
				}
			}

			delete [] pAlphaAccumulateByTerrain;
		}
		else
		{
			// If layer texture number is specified, reorder by that number.
			// Re-sort each Terrain's layer texture data based on the resulting output.
			for( int i = 0; i < nTerrainCount; ++i )
			{
				// Move the specified texture and related data to slots 1 and 2.
				vector<string>& vlTextureLayersInTerrain = szVecLayerTexture.at( i );
				std::swap( vlTextureLayersInTerrain.at( 0 ), vlTextureLayersInTerrain.at( aiSelectedLayerTexture[ 0 ] ) );
				std::swap( vlTextureLayersInTerrain.at( 1 ), vlTextureLayersInTerrain.at( aiSelectedLayerTexture[ 1 ] ) );

				vector<float>& vlDistanceInTerrain = fVecLayerTextureDistance.at( i );
				std::swap( vlDistanceInTerrain.at( 0 ), vlDistanceInTerrain.at( aiSelectedLayerTexture[ 0 ] ) );
				std::swap( vlDistanceInTerrain.at( 1 ), vlDistanceInTerrain.at( aiSelectedLayerTexture[ 1 ] ) );

				vector<float>& vlRotationInTerrain = fVecLayerTextureRotation.at( i );
				std::swap( vlRotationInTerrain.at( 0 ), vlRotationInTerrain.at( aiSelectedLayerTexture[ 0 ] ) );
				std::swap( vlRotationInTerrain.at( 1 ), vlRotationInTerrain.at( aiSelectedLayerTexture[ 1 ] ) );

				// Also update the alpha table values.
				int iXOffset = i%nBlockCountX;
				int iYOffset = i/nBlockCountX;
				int iOffset = iYOffset*nWidth*DEFAULT_TERRAIN_SIZE + (iXOffset*DEFAULT_TERRAIN_SIZE);
				for( int k = 0; k < DEFAULT_TERRAIN_SIZE*DEFAULT_TERRAIN_SIZE; ++k )
				{
					int iAlphaTablePos = iOffset + (k/DEFAULT_TERRAIN_SIZE)*nWidth + k%DEFAULT_TERRAIN_SIZE;
					if( nHeight*nWidth <= iAlphaTablePos )
						continue;

					D3DXCOLOR LegacyColor = D3DXCOLOR( pAlphaTable[iAlphaTablePos] );
					float fColorTable[4] = { LegacyColor.a, LegacyColor.r, LegacyColor.g, LegacyColor.b };
					float fSwapedColorTable[4] = { LegacyColor.a, LegacyColor.r, LegacyColor.g, LegacyColor.b };
					
					std::swap( fSwapedColorTable[ 0 ], fColorTable[ aiSelectedLayerTexture[ 0 ] ] );
					std::swap( fSwapedColorTable[ 1 ], fColorTable[ aiSelectedLayerTexture[ 1 ] ] );

					// D3DXCOLOR constructor order is r, g, b, a. 
					// Keep in mind the alpha table storage order is a, r, g, b.
					D3DXCOLOR SwapColor( fSwapedColorTable[ 1 ], fSwapedColorTable[ 2 ],
										 fSwapedColorTable[ 3 ], fSwapedColorTable[ 0 ] );
					pAlphaTable[ iAlphaTablePos ] = SwapColor;
				}
			}
		}
		
	}
	//////////////////////////////////////////////////////////////////////////


	m_Handle = EternityEngine::CreateTerrain( &Info );
	if( !m_Handle ) return;

	int nType;
	std::vector<int> nVecTemp;
	std::vector< std::vector<int> > nVecTextureType;
	char szFileName[_MAX_PATH];
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TTILE );

	for( int i=0; i<m_nBlockCount; i++ ) {
		m_Handle->ChangeBlockType( (TerrainType)nVecBlockType[i], i );
		nVecTemp.clear();
		for( DWORD j=0; j<szVecLayerTexture[i].size(); j++ ) {
			m_Handle->SetTexture( j, szVecLayerTexture[i][j].c_str(), i );
			m_Handle->SetTextureDistance( j, fVecLayerTextureDistance[i][j], i );
			m_Handle->SetTextureRotation( j, fVecLayerTextureRotation[i][j], i );

			_GetFullFileName( szFileName, _countof(szFileName), szVecLayerTexture[i][j].c_str() );
			int nItemID = pSox->GetItemIDFromFieldCaseFree( "_Name", szFileName );
			if( nItemID == -1 ) nType = TileTypeEnum::None;
			else nType = pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
			nVecTemp.push_back( nType );
			ThreadDelay();
		}
		nVecTextureType.push_back( nVecTemp );
	}
	m_Handle->InitializeBlock( -1 );
	m_Handle->SetGrassTexture( szGrassTexture );
	m_Handle->Show( false );


	// Create tile attribute table using pAlphaTable.
	int nWidth = (int)GetTileWidthCount();
	int nHeight = (int)GetTileHeightCount();
	m_pTileTypePtr = new char[ nWidth * nHeight ];
	memset( m_pTileTypePtr, 0, sizeof(char) * nWidth * nHeight );


	int nBlockSizeX, nBlockSizeY;
	int nBlockCountX, nBlockCountY;
	m_Handle->GetBlockCount( nBlockCountX, nBlockCountY );
	m_Handle->GetBlockSize( nBlockSizeX, nBlockSizeY );

	for( int i=0; i<nHeight; i++ ) {
		for( int j=0; j<nWidth; j++ ) {

			int nMaxAlphaLayer = 0;
			float fCurAlphaValue = 0.f;
			int nOffset = i * nWidth + j;

			D3DXCOLOR Color = D3DXCOLOR( pAlphaTable[nOffset] );
			float fColorTable[4] = { Color.a, Color.r, Color.g, Color.b };

			for( int k=0; k<4; k++ ) {
				if( fColorTable[k] > fCurAlphaValue ) {
					fCurAlphaValue = fColorTable[k];
					nMaxAlphaLayer = k;
				}
			}

			int nBlockX = j / nBlockSizeX;
			int nBlockY = i / nBlockSizeY;
			if( nBlockX >= nBlockCountX ) nBlockX -= 1;
			if( nBlockY >= nBlockCountY ) nBlockY -= 1;

			char cTileType = (char)nVecTextureType[ nBlockY * nBlockCountX + nBlockX ][nMaxAlphaLayer];
			m_pTileTypePtr[ i * nWidth + j ] = cTileType;
			m_SetUseTileTypeList.insert( (GlobalEnum::TileTypeEnum)cTileType );
		}
	}
}

void CDnWorldSector::BakeLightmap( int nBlurSize )
{
	if( m_Handle )
	{
		if( CheckBakedLightMap() ) return;
		m_Handle->BakeLightMap( -1, DEFAULT_LIGHTMAP_SIZE, DEFAULT_LIGHTMAP_SIZE, (float)nBlurSize );
	}
}

bool CDnWorldSector::CheckBakedLightMap()
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

CEtWorldSound *CDnWorldSector::AllocSound()
{
	return new CDnWorldSound( this );
}

void CDnWorldSector::InsertProcessProp( CDnWorldProp *pProp )
{
	m_pVecProcessProp.push_back( pProp );
}

void CDnWorldSector::RemoveProcessProp( CDnWorldProp *pProp )
{
	for( DWORD i=0; i<m_pVecProcessProp.size(); i++ ) {
		if( m_pVecProcessProp[i] == pProp ) {
			m_pVecProcessProp.erase( m_pVecProcessProp.begin() + i );
			break;
		}
	}
}

TileTypeEnum CDnWorldSector::GetTileType( float fX, float fZ )
{
	if( fX < 0.f ) fX = 0.f;
	else if( fX >= m_pParentGrid->GetGridWidth() * 100.f ) fX = ( m_pParentGrid->GetGridWidth() * 100.f ) - 0.1f;

	if( fZ < 0.f ) fZ = 0.f;
	else if( fZ >= m_pParentGrid->GetGridHeight() * 100.f ) fZ = ( m_pParentGrid->GetGridHeight() * 100.f ) - 0.1f;

	int nCellX = (int)( fX / m_fTileSize );
	int nCellZ = (int)( fZ / m_fTileSize );

	DWORD dwSectorWidth = GetTileWidthCount();

	return (TileTypeEnum)m_pTileTypePtr[ nCellZ * dwSectorWidth + nCellX ];
}


CEtTrigger *CDnWorldSector::AllocTrigger()
{
	return new CDnTrigger( this );
}

CEtWorldWater *CDnWorldSector::AllocWater()
{
	return new CDnWorldWater( this );
}

bool CDnWorldSector::LoadProp( const char *szSectorPath, int nBlockIndex, bool bThreadLoad )
{
	bool bResult = CEtWorldSector::LoadProp( szSectorPath, nBlockIndex, bThreadLoad );
	if( bResult ) {
		char szTemp[_MAX_PATH] = { 0, };

		sprintf_s( szTemp, "%s\\ColBuild.ini", szSectorPath );

		CResMngStream Stream( szTemp );
		if( Stream.IsValid() ) CEtCollisionMng::GetInstance().Load( &Stream );

		int nCurGraphicQuality = 0;
		if( CGameOption::GetInstancePtr() ) {
			nCurGraphicQuality = CGameOption::GetInstance().GetCurGraphicQuality();
		}

		if( nCurGraphicQuality > 0 ) {
			ScopeLock<CSyncLock> Lock( CDnWorldProp::s_SmartPtrLock );
			int nCount = CDnWorldProp::GetItemCount();
			for( int i=0; i<nCount; i++ ) {
				CDnWorldProp *pProp = CDnWorldProp::GetItem(i);
				if( !pProp ) {
					assert(0);
					continue;
				}
				if( pProp->GetSpecLevel() > nCurGraphicQuality )
				{
					pProp->Show( false );
				}
			}
		}
		CEtObject::CalcLightInfluenceList( true );

		int nCount = CDnWorldProp::GetItemCount();
		for( int i=0; i<nCount; i++ )
		{
			CDnWorldProp *pProp = CDnWorldProp::GetItem( i );
			if( !pProp ) continue;
			
			int nNavType = pProp->GetNavType();
			if( nNavType > 0 && !IsUsingTileType( (GlobalEnum::TileTypeEnum)nNavType ) )
			{
				m_SetUseTileTypeList.insert( (GlobalEnum::TileTypeEnum)nNavType );
			}
		}
	}
	return bResult;
}

bool CDnWorldSector::LoadControlArea( const char *szSectorPath )
{
	bool bRtnValue = CEtWorldSector::LoadControlArea( szSectorPath );

	CEtWorldEventControl* pControl = GetControlFromUniqueID( ETE_EnvironmentEffectAera );

	if( pControl )
	{
		if( pControl->GetAreaCount() > 0 )
		{
			for( DWORD j=0; j<pControl->GetAreaCount(); j++ )
			{
				CEtWorldEventArea *pArea = pControl->GetAreaFromIndex( j );
				if( pArea == NULL )
					continue;

				EnvironmentEffectAeraStruct* pStruct = (EnvironmentEffectAeraStruct*)pArea->GetData();
				if( pStruct )
				{
					if( pStruct->EffectType > 0 && !IsUsingTileType( (GlobalEnum::TileTypeEnum)pStruct->EffectType ) )
					{
						m_SetUseTileTypeList.insert( (GlobalEnum::TileTypeEnum)pStruct->EffectType );
					}
				}
			}
		}
	}

	return bRtnValue;
}

void CDnWorldSector::RefreshQualityLevel( int nLevel )
{
	ScopeLock<CSyncLock> Lock( CDnWorldProp::s_SmartPtrLock );
	int nCount = CDnWorldProp::GetItemCount();
	for( int i=0; i<nCount; i++ ) {
		CDnWorldProp *pProp = CDnWorldProp::GetItem(i);

		if( nLevel > 0 && pProp->GetSpecLevel() > nLevel )
			pProp->Show( false );
		else pProp->Show( true );
	}
}

/*
bool CDnWorldSector::LoadAttribute( const char *szSectorPath )
{
	bool bResult = CEtWorldSector::LoadAttribute( szSectorPath );
	if( bResult ) {
		GenerationAttribute();
	}

	return bResult;
}

void CDnWorldSector::GenerationAttribute()
{
	EtVector3 vOffset;
	float fWidth = m_pParentGrid->GetGridWidth() * 100.f;
	float fHeight = m_pParentGrid->GetGridHeight() * 100.f;

	vOffset.x = m_Offset.x - ( fWidth / 2.f );
	vOffset.z = m_Offset.z - ( fHeight / 2.f );
	vOffset.y = 0.f;

	int nBlockCountX, nBlockCountY;

	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
	int nHeightCount = (int)( GetTileHeightCount() * GetTileSize() ) / m_nAttributeBlockSize;

	nBlockCountX = nWidthCount / m_nDrawAttributeCount;
	nBlockCountY = nHeightCount / m_nDrawAttributeCount;
	if( nWidthCount % m_nDrawAttributeCount > 0 ) nBlockCountX += 1;
	if( nHeightCount % m_nDrawAttributeCount > 0 ) nBlockCountY += 1;
	SPrimitiveDraw3D *pBuffer;
	bool *pTriangleType;

	for( int m=0; m<nBlockCountY; m++ ) {
		for( int n=0; n<nBlockCountX; n++ ) {
			int nPrimitiveX = m_nDrawAttributeCount;
			int nPrimitiveY = m_nDrawAttributeCount;
			if( n == nBlockCountX - 1 && nWidthCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveX = nWidthCount % m_nDrawAttributeCount;
			}
			if( m == nBlockCountY - 1 && nHeightCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveY = nHeightCount % m_nDrawAttributeCount;
			}
			DWORD dwPrimitiveCount = nPrimitiveX * nPrimitiveY * 2;
			m_dwVecAttributePrimitiveCount.push_back( dwPrimitiveCount );
			pBuffer = new SPrimitiveDraw3D[ dwPrimitiveCount * 3 ];
			pTriangleType = new bool[ dwPrimitiveCount / 2 ];

			EtVector3 vPos;
			int nPosX = ( n * m_nDrawAttributeCount );
			int nPosY = ( m * m_nDrawAttributeCount );
			for( int j=0; j<nPrimitiveY; j++ ) {
				for( int i=0; i<nPrimitiveX; i++ ) {

					vPos.x = ( (i+nPosX) * m_nAttributeBlockSize ) + vOffset.x;
					vPos.z = ( (j+nPosY) * m_nAttributeBlockSize ) + vOffset.z;
					vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;

					pBuffer[(j*6) * nPrimitiveX + (i*6)].Position = vPos;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+3)].Position = vPos;

					vPos.x = ( ((i+nPosX)+1) * m_nAttributeBlockSize ) + vOffset.x;
					vPos.z = ( (j+nPosY) * m_nAttributeBlockSize ) + vOffset.z;
					vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+1)].Position = vPos;

					vPos.x = ( ((i+nPosX)+1) * m_nAttributeBlockSize ) + vOffset.x;
					vPos.z = ( ((j+nPosY)+1) * m_nAttributeBlockSize ) + vOffset.z;
					vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+2)].Position = vPos;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+4)].Position = vPos;

					vPos.x = ( (i+nPosX) * m_nAttributeBlockSize ) + vOffset.x;
					vPos.z = ( ((j+nPosY)+1) * m_nAttributeBlockSize ) + vOffset.z;
					vPos.y = GetHeight( vPos.x-vOffset.x, vPos.z-vOffset.z ) + 20.f;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+5)].Position = vPos;


					DWORD dwColor = 0;
					char cAttr = m_pAttribute[ (j+nPosY) * nWidthCount + (i+nPosX) ];
					if( cAttr & 0x01 ) dwColor = 0x55EE2222;
					if( cAttr & 0x02 ) dwColor = 0x552222EE;
					if( cAttr & 0x04 ) dwColor = 0x5522EE22;
					if( cAttr & 0x08 ) dwColor = 0x55EEEE11;

					pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = dwColor;

					pTriangleType[ j * nPrimitiveX + i ] = false;
				}
			}
			m_pVecAttributePrimitive.push_back( pBuffer );
			m_bVecAttributePrimitiveType.push_back( pTriangleType );

		}
	}

	for( int m=0; m<nBlockCountY; m++ ) {
		for( int n=0; n<nBlockCountX; n++ ) {
			int nPrimitiveX = m_nDrawAttributeCount;
			int nPrimitiveY = m_nDrawAttributeCount;
			if( n == nBlockCountX - 1 && nWidthCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveX = nWidthCount % m_nDrawAttributeCount;
			}
			if( m == nBlockCountY - 1 && nHeightCount % m_nDrawAttributeCount > 0 ) {
				nPrimitiveY = nHeightCount % m_nDrawAttributeCount;
			}
			int nPosX = ( n * m_nDrawAttributeCount );
			int nPosY = ( m * m_nDrawAttributeCount );
			SPrimitiveDraw3D *pBuffer = m_pVecAttributePrimitive[ m * nBlockCountX + n ];
			for( int j=0; j<nPrimitiveY; j++ ) {
				for( int i=0; i<nPrimitiveX; i++ ) {
					DWORD dwColor = 0;
					char cAttr = m_pAttribute[ (j+nPosY) * nWidthCount + (i+nPosX) ];
					if( cAttr & 0x01 ) dwColor = 0x55EE2222;
					if( cAttr & 0x02 ) dwColor = 0x552222EE;
					if( cAttr & 0x04 ) dwColor = 0x5522EE22;
					if( cAttr & 0x08 ) dwColor = 0x55EEEE11;

					pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = dwColor;
					pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = dwColor;

					if( (cAttr & 0x10) || (cAttr & 0x40) ) {
						bool *pTriangleType = m_bVecAttributePrimitiveType[ m * nBlockCountX + n ];
						if( pTriangleType[ j * nPrimitiveX + i ] == false ) {
							pTriangleType[ j * nPrimitiveX + i ] = true;

							EtVector3 vTemp = pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position = vTemp;
						}
						if( cAttr & 0x10 ) {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = 0;
						}
						else {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = 0;
						}

					}
					else if( (cAttr & 0x20) || (cAttr & 0x80) ) {
						bool *pTriangleType = m_bVecAttributePrimitiveType[ m * nBlockCountX + n ];
						if( pTriangleType[ j * nPrimitiveX + i ] == true ) {
							pTriangleType[ j * nPrimitiveX + i ] = false;

							EtVector3 vTemp = pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].Position = pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].Position;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].Position = vTemp;
						}

						if( cAttr & 0x20 ) {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+3) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+4) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+5) ].dwColor = 0;
						}
						else {
							pBuffer[(j*6) * nPrimitiveX + ((i*6)) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+1) ].dwColor = 0;
							pBuffer[(j*6) * nPrimitiveX + ((i*6)+2) ].dwColor = 0;
						}
					}
				}
			}
		}
	}
}

void CDnWorldSector::RenderAttribute()
{
	for( DWORD i=0; i<m_pVecAttributePrimitive.size(); i++ ) {
		EternityEngine::DrawTriangle3D( m_pVecAttributePrimitive[i], m_dwVecAttributePrimitiveCount[i] );
	}
}
*/

void CDnWorldSector::Show( bool bShow )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		((CDnWorldProp*)m_pVecPropList[i])->Show( bShow );
	}
}

bool CDnWorldSector::IsUsingTileType( GlobalEnum::TileTypeEnum Type )
{
	if( Type == GlobalEnum::TileTypeEnum::Water ) {
		if( !m_pWater ) return false;
		if( !m_pWater->IsEnableWater() && !m_pWater->GetRiverCount() ) return false;
		return true;
	}
	return ( m_SetUseTileTypeList.find( Type ) != m_SetUseTileTypeList.end() ) ? true : false;
}