#include "stdafx.h"
#include "EtWorld.h"
#include "EtWorldSector.h"
#include "EtWorldGrid.h"
#include "EtWorldProp.h"
#include "EtWorldEventArea.h"
#include "EtWorldEventControl.h"
#include "EtWorldSound.h"
#include "EtWorldSoundEnvi.h"
#include "EtTrigger.h"
#include "EtWorldWater.h"
#include "EtWorldDecal.h"
#include "navigationmesh.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

template<> float CEtOctreeNode<CEtWorldProp *>::s_fMinRadius = 1000.f;
template<> float CEtQuadtreeNode<CEtWorldDecal *>::s_fMinRadius = 1000.f;
template<> float CEtQuadtreeNode<CEtWorldEventArea *>::s_fMinRadius = 1000.f;

CEtWorldSector::CEtWorldSector()
{
	m_pParentGrid = NULL;
	m_pHeight = NULL;
	m_nHeightSize = 0;
	m_Index = SectorIndex( -1, -1 );
	m_Offset = EtVector3( 0.f, 0.f, 0.f );
	m_fTileSize = 200.f;
	m_fHeightMultiply = 1.f;
	m_nBlockCount = 0;
	m_pPropOctree = NULL;
	m_pDecalQuadtree = NULL;
	m_pEventAreaQuadtree = NULL;
	m_pSoundInfo = NULL;
	m_bLoaded = false;
	m_pAttribute = NULL;
	m_nAttributeBlockSize = 50;
	m_pTrigger = NULL;
	m_pWater = NULL;
	m_pNaviMesh = NULL;
	m_nPropCreateUniqueCount = 0;
	m_nEventAreaCreateUniqueCount = 0;
	m_nTileWidthCount = m_nTileHeightCount = 0;
	m_bLoadFailed = false;
	m_fCenterHeight = 0.f;
	m_fRadius = 0.f;
	m_pCollisionHeight = NULL;
	m_nAttributeSize = 0;
}

CEtWorldSector::~CEtWorldSector()
{
	Free();
}

void CEtWorldSector::Free()
{
	SAFE_DELETEA( m_pCollisionHeight );
	SAFE_DELETEA( m_pHeight );
	SAFE_DELETE( m_pPropOctree );
	SAFE_DELETE( m_pDecalQuadtree );
	SAFE_DELETE( m_pEventAreaQuadtree );
	SAFE_DELETE_PVEC( m_pVecPropList );
	SAFE_DELETE_PVEC( m_pVecAreaControl );
	SAFE_DELETE_PVEC( m_pVecDecalList );
	SAFE_DELETE( m_pSoundInfo );
	SAFE_DELETEA( m_pAttribute );
	SAFE_DELETE( m_pTrigger );
	SAFE_DELETE( m_pWater );
	SAFE_DELETE(m_pNaviMesh);

	m_pParentGrid = NULL;
	m_pHeight = NULL;
	m_Index = SectorIndex( -1, -1 );
	m_Offset = EtVector3( 0.f, 0.f, 0.f );
	m_fTileSize = 200.f;
	m_fHeightMultiply = 1.f;
	m_nBlockCount = 0;
	m_bLoaded = false;
	m_nAttributeBlockSize = 50;
	m_nTileWidthCount = m_nTileHeightCount = 0;
	m_fCenterHeight = 0.f;
	m_fRadius = 0.f;
}

bool CEtWorldSector::Initialize( CEtWorldGrid *pParentGrid, SectorIndex Index )
{
	_ASSERT( pParentGrid != NULL );
	m_pParentGrid = pParentGrid;
	m_Index = Index;

	m_Offset.x = -( ( m_pParentGrid->GetGridX() / 2.f ) * (float)m_pParentGrid->GetGridWidth() * 100.f ) + (float)( m_Index.nX * m_pParentGrid->GetGridWidth() * 100.f );
	m_Offset.x += ( m_pParentGrid->GetGridWidth() * 100.f ) / 2.f;
	m_Offset.y = 0.f;
	m_Offset.z = -( ( m_pParentGrid->GetGridY() / 2.f ) * (float)m_pParentGrid->GetGridHeight() * 100.f ) + (float)( m_Index.nY * m_pParentGrid->GetGridHeight() * 100.f );
	m_Offset.z += ( m_pParentGrid->GetGridHeight() * 100.f ) / 2.f;

	float fSize = max( m_pParentGrid->GetGridWidth() * 100.f, m_pParentGrid->GetGridHeight() * 100.f );

	char szPath[_MAX_PATH] = { 0, };
	sprintf_s( szPath, "%s\\Grid\\%s\\%d_%d", m_pParentGrid->GetWorld()->GetWorldFolder(), m_pParentGrid->GetName(), m_Index.nX, m_Index.nY );
	if( !LoadSectorSize( szPath ) ) {
		m_fCenterHeight = 0.f;
		m_fRadius = max( fSize, GetHeightMultiply() * 65535.f );
	}

	m_pDecalQuadtree = new CEtQuadtree<CEtWorldDecal *>;
	m_pDecalQuadtree->Initialize( EtVector2( m_Offset.x, m_Offset.z ), fSize );

	m_pPropOctree = new CEtOctree<CEtWorldProp *>( false );
	m_pPropOctree->Initialize( EtVector3( m_Offset.x, m_fCenterHeight, m_Offset.z ), m_fRadius );

	m_pEventAreaQuadtree = new CEtQuadtree<CEtWorldEventArea *>;
	m_pEventAreaQuadtree->Initialize( EtVector2( m_Offset.x, m_Offset.z ), fSize );

	CalcTileCount();
	return true;
}

bool CEtWorldSector::Load( int nBlockIndex, bool bThreadLoad, int nLoadSectorEnum )
{
	// No threaded read for now.
	// When creating threads later, do not read data and create immediately
	// While class holds already-read data, create in between
	// Need to change it to do so.
	char szPath[_MAX_PATH] = { 0, };
	bool bResult = false;

	sprintf_s( szPath, "%s\\Grid\\%s\\%d_%d", m_pParentGrid->GetWorld()->GetWorldFolder(), m_pParentGrid->GetName(), m_Index.nX, m_Index.nY );
	m_szSectorPath = szPath;

	if( nLoadSectorEnum & LSE_Terrain ) bResult = LoadTerrain( szPath, nBlockIndex, bThreadLoad );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Prop ) bResult = LoadProp( szPath, nBlockIndex, bThreadLoad );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Control ) bResult = LoadControlArea( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Sound ) bResult = LoadSound( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Attribute ) bResult = LoadAttribute( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Navigation ) bResult = LoadNavigationMesh( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Trigger ) bResult = LoadTrigger( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Water ) bResult = LoadWater( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( nLoadSectorEnum & LSE_Decal ) bResult = LoadDecal( szPath );
	else bResult = true;
	if( !CEtWorld::s_bIgnoreInvalidFile && bResult == false ) {
		Free();
		return false;
	}
	ThreadDelay();
	if( !( nLoadSectorEnum & LSE_GenCollisionHeight ) ) 
		SAFE_DELETEA( m_pCollisionHeight );

	m_bLoaded = true;
	return true;
}

bool CEtWorldSector::LoadTerrain( const char *szSectorPath, int nBlockIndex, bool bThreadLoad )
{
	char szTemp[_MAX_PATH] = { 0, };

	int nSize;
	DWORD *pAlpha = NULL;
	std::vector<int> nVecBlockTypeList;
	std::vector< std::vector<std::string> > szVecLayerTexture;
	std::vector< std::vector<float> > fVecLayerTextureDistance;
	std::vector< std::vector<float> > fVecLayerTextureRotation;

	char *pGrass = NULL;
	float fGrassWidth[4] = { 100.f, }, fGrassHeightMin[4] = { 100.f, }, fGrassHeightMax[4] = { 100.f, };
	float fGrassShakeMin = 8.f, fGrassShakeMax = 15.f;
	std::string szGrassTexture;


	CResMngStream Stream;
	// Load Height Table
	sprintf_s( szTemp, "%s\\Height.ini", szSectorPath );
	Stream.Open( szTemp );
	if( !Stream.IsValid() ) return false;
	Stream.Read( &m_fHeightMultiply, sizeof(float) );
	Stream.Read( &nSize, sizeof(int) );
	m_pHeight = new short[nSize];
	m_nHeightSize = nSize;
	Stream.Read( m_pHeight, nSize * sizeof(short) );
	if( !Stream.IsEnd() && long(Stream.Size() - Stream.Tell()) >= long(nSize * sizeof(short)) ) {
		m_pCollisionHeight = new short[nSize];
		Stream.Read( m_pCollisionHeight, nSize * sizeof(short) );
	}
	Stream.Close();

	// Load Alpha Table
	sprintf_s( szTemp, "%s\\AlphaTable.ini", szSectorPath );
	Stream.Open( szTemp );
	if( Stream.IsValid() ) {
		Stream.Read( &nSize, sizeof(int) );
		pAlpha = new DWORD[nSize];
		Stream.Read( pAlpha, nSize * sizeof(DWORD) );
		Stream.Close();
	}

	// Load Grass Table
	sprintf_s( szTemp, "%s\\GrassTable.ini", szSectorPath );
	Stream.Open( szTemp );
	if( Stream.IsValid() ) {
		Stream.Read( fGrassWidth, sizeof(fGrassWidth) );
		Stream.Read( fGrassHeightMin, sizeof(fGrassHeightMin) );
		Stream.Read( fGrassHeightMax, sizeof(fGrassHeightMax) );
		Stream.Read( &fGrassShakeMin, sizeof(float) );
		Stream.Read( &fGrassShakeMax, sizeof(float) );
		ReadStdString( szGrassTexture, &Stream );
		Stream.Read( &nSize, sizeof(int) );
		pGrass = new char[nSize];
		Stream.Read( pGrass, nSize * sizeof(char) );
		Stream.Close();
	}

	// Load Texture Table
	sprintf_s( szTemp, "%s\\TexTable.ini", szSectorPath );
	Stream.Open( szTemp );
	if( Stream.IsValid() ) {
		Stream.Read( &m_nBlockCount, sizeof(int) );

		// Read Block Type
		int nType;
		for( int i=0; i<m_nBlockCount; i++ ) {
			Stream.Read( &nType, sizeof(int) );
			nVecBlockTypeList.push_back( nType );
		}
		// Read Block Texture
		std::string szTexture;
		std::vector<std::string> szVecList;
		for( int i=0; i<m_nBlockCount; i++ ) {
			szVecList.clear();
			Stream.Read( &nSize, sizeof(int) );
			for( int j=0; j<nSize; j++ ) {
				ReadStdString( szTexture, &Stream );
				szVecList.push_back( szTexture );
			}
			szVecLayerTexture.push_back( szVecList );
		}
		SAFE_DELETE_VEC( szVecList );

		// Read Block Texture Distance
		float fDistance;
		std::vector<float> fVecList;
		for( int i=0; i<m_nBlockCount; i++ ) {
			fVecList.clear();
			Stream.Read( &nSize, sizeof(int) );
			for( int j=0; j<nSize; j++ ) {
				Stream.Read( &fDistance, sizeof(int) );
				fVecList.push_back( fDistance );
			}
			fVecLayerTextureDistance.push_back( fVecList );
		}

		// Read Block Texture Rotation - add exception handling for backward compatibility (possible since it is at the very end).
		bool bReadRotation = false;
		int nValue = 0;
		int nRead = Stream.Read( &nSize, sizeof(int) );
		if( nRead != 0 )
		{
			bReadRotation = true;
			nValue = sizeof(int);
			Stream.Seek( -nValue, SEEK_CUR );
		}
		float fRotation;
		std::vector<float> fVecList2;
		for( int i=0; i<m_nBlockCount; i++ ) {
			fVecList2.clear();
			if( bReadRotation ) Stream.Read( &nSize, sizeof(int) );
			else nSize = 4;
			for( int j=0; j<nSize; j++ ) {
				if( bReadRotation ) Stream.Read( &fRotation, sizeof(int) );
				else fRotation = 0.0f;
				fVecList2.push_back( fRotation );
			}
			fVecLayerTextureRotation.push_back( fVecList2 );
		}
		Stream.Close();
		SAFE_DELETE_VEC( fVecList );
	}

	InitializeTerrain( pAlpha, nVecBlockTypeList, szVecLayerTexture, fVecLayerTextureDistance, fVecLayerTextureRotation, (char*)szGrassTexture.c_str(), pGrass, fGrassWidth, fGrassHeightMin, fGrassHeightMax, fGrassShakeMin, fGrassShakeMax );

	// Clear Used Vector
	SAFE_DELETEA( pGrass );
	SAFE_DELETEA( pAlpha );
	SAFE_DELETE_VEC( nVecBlockTypeList );
	for( DWORD i=0; i<szVecLayerTexture.size(); i++ ) {
		SAFE_DELETE_VEC( szVecLayerTexture[i] );
	}
	SAFE_DELETE_VEC( szVecLayerTexture );
	for( DWORD i=0; i<fVecLayerTextureDistance.size(); i++ ) {
		SAFE_DELETE_VEC( fVecLayerTextureDistance[i] );
	}
	SAFE_DELETE_VEC( fVecLayerTextureDistance );
	for( DWORD i=0; i<fVecLayerTextureRotation.size(); i++ ) {
		SAFE_DELETE_VEC( fVecLayerTextureRotation[i] );
	}
	SAFE_DELETE_VEC( fVecLayerTextureRotation );

	return true;
}

void CEtWorldSector::LoadIgnorePropFolder( const char *szSectorPath )
{
	m_vecIgnorePropFolder.clear();

	char szTemp[_MAX_PATH] = { 0, };

	sprintf_s( szTemp, "%s\\IgnorePropFolderInfo.ini", szSectorPath );
	CResMngStream Stream( szTemp );
	if( !Stream.IsValid() ) return;

	int nCount;
	std::string szStr;
	Stream.Read( &nCount, sizeof(int) );
	for( int i=0; i<nCount; i++ ) {
		ReadStdString( szStr, &Stream );
		m_vecIgnorePropFolder.push_back( szStr );
	}
}

bool CEtWorldSector::CheckIgnoreProp( const char *szPropName )
{
	if( m_vecIgnorePropFolder.empty() ) return false;

	// Designed to run after map-prop resource folder registration via AddResourcePath is complete.
	std::string szFullName = CEtResourceMng::GetInstance().GetFullNameInCacheList( szPropName ).c_str();
	if( szFullName.empty() ) return false;

	std::vector<std::string> vecToken;
	TokenizeA( szFullName, vecToken, "\\" );
	if( vecToken.size() > 1 ) {
		std::string szFolderName = vecToken[vecToken.size()-2];
		for( int i = 0; i < (int)m_vecIgnorePropFolder.size(); ++i ) {
			if( szFolderName == m_vecIgnorePropFolder[i] )
				return true;
		}
	}
	return false;
}

bool CEtWorldSector::LoadProp( const char *szSectorPath, int nBlockIndex, bool bThreadLoad )
{
	char szTemp[_MAX_PATH] = { 0, };

	sprintf_s( szTemp, "%s\\PropInfo.ini", szSectorPath );
	CResMngStream Stream( szTemp );
	if( !Stream.IsValid() ) return false;

	LoadIgnorePropFolder( szSectorPath );

	int nSize;
	Stream.Read( &m_nPropCreateUniqueCount, sizeof(int) );
	Stream.Read( &nSize, sizeof(int) );

	int nStructSize;
	CEtWorldProp::PropStruct Struct;
	CEtWorldProp *pProp;

	for( int i=0; i<nSize; i++ ) {
		// Prepend PropSize and Class
		Stream.Read( &nStructSize, sizeof(int) );
		Stream.Read( &Struct, sizeof(CEtWorldProp::PropStruct) );

		if( CheckIgnoreProp( Struct.szPropName ) ) {
			int nCustomSize = nStructSize - sizeof(CEtWorldProp::PropStruct);
			Stream.Seek( nCustomSize, SEEK_CUR );
			continue;
		}

		int nPropClassID = GetPropClassID( Struct.szPropName );
		pProp = AllocProp( nPropClassID );
		if( pProp == NULL ) {
			int nCustomSize = nStructSize - sizeof(CEtWorldProp::PropStruct);
			Stream.Seek( nCustomSize, SEEK_CUR );
			OutputDebug( "Warning : Unreadable prop found. [ %s ] / Prop Alloc failed\n", Struct.szPropName );
			continue;
		}
		else pProp->SetClassID( nPropClassID );

		pProp->LoadCustomInfo( &Stream );

		if( pProp->Initialize( this, Struct.szPropName, Struct.vPosition, Struct.vRotation, Struct.vScale ) == false ) {
			SAFE_DELETE( pProp );
			continue;
		}
		pProp->SetCreateUniqueID( Struct.nUniqueID );
		pProp->EnableCastShadow( Struct.bCastShadow );
		pProp->EnableReceiveShadow( Struct.bReceiveShadow );
		pProp->EnableIgnoreBuildColMesh( Struct.bIgnoreBuildColMesh );
		pProp->SetNavType( Struct.nNavType );

		InsertProp( pProp );
		ThreadDelay();
	}

	return true;
}


void CEtWorldSector::SetTileSize( float fValue )
{
	m_fTileSize = fValue;

	CalcTileCount();
}

void CEtWorldSector::CalcTileCount()
{
	m_nTileWidthCount = 1 + ( m_pParentGrid->GetGridWidth() * 100 ) / (int)m_fTileSize;
	m_nTileHeightCount = 1 + ( m_pParentGrid->GetGridHeight() * 100 ) / (int)m_fTileSize;
}

float CEtWorldSector::GetHeightToWorld( float fX, float fZ, EtVector3 *pNormal )
{
	fX -= ( m_Offset.x - ( m_pParentGrid->GetGridWidth() * 100.f / 2.f ) );
	fZ -= ( m_Offset.z - ( m_pParentGrid->GetGridHeight() * 100.f / 2.f ) );
	return GetHeight( fX, fZ, pNormal );
}

float CEtWorldSector::GetHeightFromArray( float fX, float fZ, EtVector3 *pNormal, int nTileScale, short *pHeight )
{
	if( pHeight == NULL ) {
		if( pNormal ) *pNormal = EtVector3( 0.f, 0.f, 0.f );
		return 0.f;
	}

	if( fX < 0.f ) fX = 0.f;
	else if( fX >= m_pParentGrid->GetGridWidth() * 100.f ) fX = ( m_pParentGrid->GetGridWidth() * 100.f ) - 0.1f;

	if( fZ < 0.f ) fZ = 0.f;
	else if( fZ >= m_pParentGrid->GetGridHeight() * 100.f ) fZ = ( m_pParentGrid->GetGridHeight() * 100.f ) - 0.1f;

	int nBlockX = (int)( fX / (m_fTileSize*nTileScale) );
	int nBlockZ = (int)( fZ / (m_fTileSize*nTileScale) );

	float fCenterX = fX / (m_fTileSize*nTileScale) - nBlockX;
	float fCenterZ = fZ / (m_fTileSize*nTileScale) - nBlockZ;

	nBlockX *= nTileScale;
	nBlockZ *= nTileScale;

	DWORD dwSectorWidth = GetTileWidthCount();

	int nTemp = ( nBlockZ * dwSectorWidth ) + nBlockX;

	// Height exception handling.
	if( nTemp < 0 || nTemp >= m_nHeightSize || nTemp + nTileScale >= m_nHeightSize )
	{
		if( pNormal ) *pNormal = EtVector3( 0.f, 0.f, 0.f );
		return 0.f;
	}

	float fEdgeHeight[4];

	fEdgeHeight[0] = pHeight[nTemp];
	fEdgeHeight[1] = pHeight[nTemp + nTileScale];
	nTemp = ( ( nBlockZ + nTileScale ) * dwSectorWidth ) + nBlockX;

	// Height exception handling.
	if( nTemp < 0 || nTemp >= m_nHeightSize || nTemp + nTileScale >= m_nHeightSize )
	{
		if( pNormal ) *pNormal = EtVector3( 0.f, 0.f, 0.f );
		return 0.f;
	}

	fEdgeHeight[2] = pHeight[nTemp];
	fEdgeHeight[3] = pHeight[nTemp + nTileScale];

	float fResultHeight;
	if( fCenterX > fCenterZ ) 
	{
		float fTemp = fEdgeHeight[0] * ( 1.f - fCenterX );
		float fCalcHeight1 = fTemp + fEdgeHeight[3] * fCenterX;
		float fCalcHeight2 = fTemp + fEdgeHeight[1] * fCenterX;
		fTemp = fCenterZ / fCenterX;
		fResultHeight = fCalcHeight1 * fTemp + fCalcHeight2 * ( 1 - fTemp );
	}
	else if( fCenterX < fCenterZ ) 
	{
		float fTemp = fEdgeHeight[0] * ( 1 - fCenterZ );
		float fCalcHeight1 = fTemp + fEdgeHeight[2] * fCenterZ;
		float fCalcHeight2 = fTemp + fEdgeHeight[3] * fCenterZ;
		fTemp = fCenterX / fCenterZ;

		fResultHeight = fCalcHeight1 * ( 1 - fTemp ) + fCalcHeight2 * fTemp;
	}
	else
	{
		fResultHeight = fEdgeHeight[0] * ( 1 - fCenterZ ) + fEdgeHeight[3] * fCenterZ;
	}

	if( pNormal ) *pNormal = GetHeightNormal( nBlockX, nBlockZ );

	return fResultHeight * m_fHeightMultiply;
}

float CEtWorldSector::GetHeight( float fX, float fZ, EtVector3 *pNormal, int nTileScale )
{
	return GetHeightFromArray( fX, fZ, pNormal, nTileScale, m_pHeight );
}

float CEtWorldSector::GetCollisionHeight( float fX, float fZ, EtVector3 *pNormal, int nTileScale )
{
	return GetHeightFromArray( fX, fZ, pNormal, nTileScale, m_pCollisionHeight );
}

bool CEtWorldSector::GetWaterHeight( float fX, float fZ, float *pfResult )
{
	if( !m_pWater ) return false;
	return m_pWater->GetHeight( fX, fZ, pfResult );
}

EtVector3 CEtWorldSector::GetHeightNormal( int nBlockX, int nBlockZ )
{
	int nHeightIndex;
	float fDX, fDZ;
	D3DXVECTOR3 Return;

	DWORD dwTileWidth = GetTileWidthCount();
	DWORD dwTileHeight = GetTileHeightCount();
	nHeightIndex = dwTileWidth * nBlockZ + nBlockX;
	if( nBlockX == 0 ) 
		fDX = ( m_pHeight[nHeightIndex] - m_pHeight[nHeightIndex+1] ) / m_fTileSize;
	else if( nBlockX == dwTileWidth - 1 ) 
		fDX = ( m_pHeight[nHeightIndex-1] - m_pHeight[nHeightIndex] ) / m_fTileSize;
	else fDX = ( m_pHeight[nHeightIndex-1] - m_pHeight[nHeightIndex+1] ) / ( m_fTileSize * 2 );

	if( nBlockZ == 0 ) 
		fDZ = ( m_pHeight[nHeightIndex] - m_pHeight[nHeightIndex+dwTileWidth] ) / m_fTileSize;
	else if( nBlockZ == dwTileHeight - 1 )
		fDZ = ( m_pHeight[nHeightIndex-dwTileWidth-2] - m_pHeight[nHeightIndex] ) / m_fTileSize;
	else
		fDZ = ( m_pHeight[nHeightIndex-dwTileWidth-2] - m_pHeight[nHeightIndex+dwTileWidth] ) / ( m_fTileSize * 2 );

	Return.x = fDX;
	Return.y = 1.0f;
	Return.z = fDZ;
	EtVec3Normalize( &Return, &Return );

	return Return;
}

char CEtWorldSector::GetAttribute( float fX, float fZ )
{
	if( m_pAttribute == NULL ) return 0;

	int nBlockX, nBlockZ;

	if( fX < 0.f ) fX = 0.f;
	else if( fX >= m_pParentGrid->GetGridWidth() * 100.f ) fX = ( m_pParentGrid->GetGridWidth() * 100.f ) - 0.1f;

	if( fZ < 0.f ) fZ = 0.f;
	else if( fZ >= m_pParentGrid->GetGridHeight() * 100.f ) fZ = ( m_pParentGrid->GetGridHeight() * 100.f ) - 0.1f;

	nBlockX = (int)( fX / (float)m_nAttributeBlockSize );
	nBlockZ = (int)( fZ / (float)m_nAttributeBlockSize );

	int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;

	// Session dump exception handling
	int nIndex = nBlockZ * nWidthCount + nBlockX;
	if( nIndex >= m_nAttributeSize || nIndex < 0 )
		return 0;

	return m_pAttribute[ nIndex ];
}

bool CEtWorldSector::Pick( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	if( !m_Handle ) return false;
	return m_Handle->Pick( vOrig, vDir, vPickPos );
}

bool CEtWorldSector::PickWater( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	/*
	if( !m_Handle ) return false;
	return m_Handle->Pick( vOrig, vDir, vPickPos );
	*/
	if( !m_pWater ) return false;
	return m_pWater->Pick( vOrig, vDir, vPickPos );
}

CEtWorldProp *CEtWorldSector::AddProp( const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale, void *pCustomParam )
{
	int nClassID = GetPropClassID( szPropName );
	CEtWorldProp *pProp = AllocProp( nClassID );
	if( pProp == NULL ) return NULL;

	EtVector3 vOffset = *GetOffset();
	vOffset.x -= ( GetTileWidthCount() * GetTileSize() ) / 2.f;
	vOffset.z -= ( GetTileHeightCount() * GetTileSize() ) / 2.f;
	vOffset.y = 0.f;

	pProp->SetCustomParam( pCustomParam );
	EtVector3 vLocalPos = vPos - vOffset;
	if( pProp->Initialize( this, szPropName, vLocalPos, vRotate, vScale ) == false ) {
		SAFE_DELETE( pProp );
		return NULL;
	}
	pProp->SetClassID( nClassID );
	pProp->SetCreateUniqueID( AddPropCreateUniqueCount() );

	InsertProp( pProp );
	return pProp;
}

void CEtWorldSector::InsertProp( CEtWorldProp *pProp )
{
	SSphere Sphere;
	m_pVecPropList.push_back( pProp );

	pProp->GetBoundingSphere( Sphere );
	pProp->SetCurOctreeNode( m_pPropOctree->Insert( pProp, Sphere ) );
}

bool CEtWorldSector::DeleteProp( CEtWorldProp *pProp )
{
	if( !m_pPropOctree->Remove( pProp, pProp->GetCurOctreeNode() ) ) {
		m_pPropOctree->Remove( pProp, NULL );
	}
	pProp->SetCurOctreeNode( NULL );

	std::vector<CEtWorldProp *>::iterator it = std::find( m_pVecPropList.begin(), m_pVecPropList.end(), pProp );
	if( it == m_pVecPropList.end() ) 
		return false;

	m_pVecPropList.erase( it );
	return true;
}

DWORD CEtWorldSector::GetPropCount()
{
	return (DWORD)m_pVecPropList.size();
}

CEtWorldProp *CEtWorldSector::GetPropFromIndex( DWORD dwIndex )
{
	if( dwIndex >= (DWORD)m_pVecPropList.size() ) return NULL;
	return m_pVecPropList[dwIndex];
}


CEtWorldProp *CEtWorldSector::AllocProp( int nClass )
{
	return new CEtWorldProp;
}

/*
void CEtWorldSector::InsertControl( const char *szControlName )
{
	if( GetControlFromName( szControlName ) ) return;
	CEtWorldEventControl *pControl = AllocControl();
	pControl->SetName( szControlName );
	m_pVecAreaControl.push_back( pControl );
}
*/

void CEtWorldSector::InsertControl( CEtWorldEventControl *pControl )
{
	if( GetControlFromUniqueID( pControl->GetUniqueID() ) ) return;
	m_pVecAreaControl.push_back( pControl );
}

void CEtWorldSector::RemoveControl( int nUniqueID )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		if( m_pVecAreaControl[i]->GetUniqueID() == nUniqueID ) {
			SAFE_DELETE( m_pVecAreaControl[i] );
			m_pVecAreaControl.erase( m_pVecAreaControl.begin() + i );
			break;
		}
	}
}

DWORD CEtWorldSector::GetControlCount()
{
	return (DWORD)m_pVecAreaControl.size();
}

CEtWorldEventControl *CEtWorldSector::GetControlFromUniqueID( int nUniqueID )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		if( m_pVecAreaControl[i]->GetUniqueID() == nUniqueID ) return m_pVecAreaControl[i];
	}
	return NULL;
}

CEtWorldEventControl *CEtWorldSector::GetControlFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_pVecAreaControl.size() ) return NULL;
	return m_pVecAreaControl[dwIndex];
}

/*
CEtWorldEventControl *CEtWorldSector::GetControlFromName( const char *szControlName )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		if( strcmp( szControlName, m_pVecAreaControl[i]->GetName() ) == NULL ) return m_pVecAreaControl[i];
	}
	return NULL;
}
*/

CEtWorldEventControl *CEtWorldSector::AllocControl()
{
	return new CEtWorldEventControl( this );
}

bool CEtWorldSector::LoadControlArea( const char *szSectorPath )
{
	EtVector3 vOffset = m_Offset;
	vOffset.x -= ( GetTileWidthCount() * GetTileSize() ) / 2.f;
	vOffset.z -= ( GetTileHeightCount() * GetTileSize() ) / 2.f;

	char szFullName[_MAX_PATH];
	int nCount;
	sprintf_s( szFullName, "%s\\EventAreaInfo.ini", szSectorPath );
	CResMngStream Stream( szFullName );
	if( !Stream.IsValid() ) return true;
	Stream.Seek( sizeof(int), SEEK_CUR );
	Stream.Read( &nCount, sizeof(int) );

	for( int i=0; i<nCount; i++ ) {
		CEtWorldEventControl *pControl = AllocControl();
		if( pControl->Load( &Stream ) == false ) {
			SAFE_DELETE( pControl );
			continue;
		}

		InsertControl( pControl );

		// Move by the actual offset value.
		CEtWorldEventArea *pArea;
		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			pArea = pControl->GetAreaFromIndex(j);
			*pArea->GetMax() += vOffset;
			*pArea->GetMin() += vOffset;
			pArea->CalcOBB();

			SCircle Circle;
			pArea->GetBoundingCircle( Circle );
			pArea->SetCurQuadtreeNode( m_pEventAreaQuadtree->Insert( pArea, Circle ) );
		}
		ThreadDelay();
	}
	return true;
}

CEtWorldSound *CEtWorldSector::AllocSound()
{
	return new CEtWorldSound( this );
}

bool CEtWorldSector::LoadSound( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];
	sprintf_s( szFullName, "%s\\SoundInfo.ini", szSectorPath );

	m_pSoundInfo = AllocSound();
	if( m_pSoundInfo ) {
		if( m_pSoundInfo->Load( szFullName ) == false ) {
			SAFE_DELETE( m_pSoundInfo );
			return false;
		}
//		m_pSoundInfo->Play();
	}
	return true;
}

bool CEtWorldSector::LoadAttribute( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];
	sprintf_s( szFullName, "%s\\HeightAttribute.ini", szSectorPath );

	CResMngStream Stream( szFullName );
	if( !Stream.IsValid() ) {
		int nWidthCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
		int nHeightCount = (int)( GetTileWidthCount() * GetTileSize() ) / m_nAttributeBlockSize;
		int nSize = nWidthCount * nHeightCount;
		m_pAttribute = new char[nSize];
		m_nAttributeSize = nSize;
		memset( m_pAttribute, 0, nSize );
	}
	else {
		int nSize;
		Stream.Read( &m_nAttributeBlockSize, sizeof(int) );
		Stream.Read( &nSize, sizeof(int) );

		m_pAttribute = new char[nSize];
		m_nAttributeSize = nSize;
		Stream.Read( m_pAttribute, nSize );
	}
	return true;
}

bool CEtWorldSector::LoadNavigationMesh( const char *szSectorPath )
{
	char szFullName[_MAX_PATH] = {0,};
	sprintf_s( szFullName, "%s\\Navigation.ini", szSectorPath );

	CResMngStream Stream( szFullName );
	if( Stream.IsValid() ) 
	{
		m_pNaviMesh = new NavigationMesh();
		bool bResult = m_pNaviMesh->LoadFromStream(&Stream);

		return bResult;
		
	}

	return true;
}

CEtTrigger *CEtWorldSector::AllocTrigger()
{
	return new CEtTrigger( this );
}

bool CEtWorldSector::LoadTrigger( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];

	m_pTrigger = AllocTrigger();

	sprintf_s( szFullName, "%s\\TriggerDefine.ini", szSectorPath );
	m_pTrigger->LoadDefine( szFullName );

	sprintf_s( szFullName, "%s\\Trigger.ini", szSectorPath );
	if( m_pTrigger->Load( szFullName ) == false ) {
		SAFE_DELETE( m_pTrigger );
		return false;
	}
	return true;
}

CEtWorldWater *CEtWorldSector::AllocWater()
{
	return new CEtWorldWater( this );
}

bool CEtWorldSector::LoadWater( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];
	sprintf_s( szFullName, "%s\\Water.ini", szSectorPath );

	m_pWater = AllocWater();
	m_pWater->Initialize();
	if( m_pWater->Load( szFullName ) == false ) {
		SAFE_DELETE( m_pWater );
		return false;
	}
	return true;
}

bool CEtWorldSector::LoadSectorSize( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];
	sprintf_s( szFullName, "%s\\SectorSize.ini", szSectorPath );

	CResMngStream Stream( szFullName );
	if( !Stream.IsValid() ) return false;

	Stream.Read( &m_fCenterHeight, sizeof(float) );
	Stream.Read( &m_fRadius, sizeof(float) );

	return true;
}

void CEtWorldSector::ScanProp( EtVector3 &vPos, float fRadius, DNVector(CEtWorldProp*) *pVecResult )
{
	if( !m_pPropOctree ) return;

	/*
	EtVector3 vOffset = m_Offset;
	vOffset.x -= ( GetTileWidthCount() * GetTileSize() ) / 2.f;
	vOffset.z -= ( GetTileHeightCount() * GetTileSize() ) / 2.f;
	*/

	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;
	m_pPropOctree->Pick( Sphere, *pVecResult );
}

void CEtWorldSector::ScanDecal( EtVector2 &vPos, float fRadius, std::vector<CEtWorldDecal *> *pVecResult )
{
	if( !m_pDecalQuadtree ) return;

	SCircle Circle;
	Circle.Center = vPos;
	Circle.fRadius = fRadius;
	m_pDecalQuadtree->Pick( Circle, *pVecResult );
}

void CEtWorldSector::ScanEventArea( const EtVector2 &vPos, float fRadius, std::vector<CEtWorldEventArea *> *pVecResult )
{
	if( !m_pEventAreaQuadtree ) return;

	SCircle Circle;
	Circle.Center = vPos;
	Circle.fRadius = fRadius;
	m_pEventAreaQuadtree->Pick( Circle, *pVecResult );
}

void CEtWorldSector::ScanEventArea( SAABox &Box, std::vector<CEtWorldEventArea *> *pVecResult )
{
	if( !m_pEventAreaQuadtree ) return;

	CEtWorldEventArea *pArea;
	EtMatrix matIdentity;
	SOBB BoxOBB;
	EtMatrixIdentity( &matIdentity );
	BoxOBB.Init( Box, matIdentity );

	SCircle Circle;
	Circle.Center = EtVector2( BoxOBB.Center.x, BoxOBB.Center.z );
	Circle.fRadius = max( BoxOBB.Center[0], max( BoxOBB.Center[1], BoxOBB.Center[2] ) );
	m_pEventAreaQuadtree->Pick( Circle, *pVecResult );

	for( DWORD i=0; i<pVecResult->size(); i++ ) {
		pArea = (*pVecResult)[i];
		/*
		if( !( ( pArea->GetMin()->x >= vMin.x || pArea->GetMax()->x >= vMin.x ) &&
			( pArea->GetMin()->x <= vMax.x || pArea->GetMax()->x <= vMax.x ) &&
			( pArea->GetMin()->y >= vMin.y || pArea->GetMax()->y >= vMin.y ) &&
			( pArea->GetMin()->y <= vMax.y || pArea->GetMax()->y <= vMax.y ) ) ) {
				pVecResult->erase( pVecResult->begin() + i );
				i--;
		}
		*/
		if( !TestOBBToOBB( BoxOBB, *pArea->GetOBB() ) ) {
			pVecResult->erase( pVecResult->begin() + i );
			i--;
		}
	}
}

void CEtWorldSector::ScanEventArea( SOBB &Box, std::vector<CEtWorldEventArea *> *pVecResult )
{
	if( !m_pEventAreaQuadtree ) return;

	SCircle Circle;
	Circle.Center = EtVector2( Box.Center.x, Box.Center.z );
	Circle.fRadius = max( Box.Extent[0], max( Box.Extent[1], Box.Extent[2] ) );
	m_pEventAreaQuadtree->Pick( Circle, *pVecResult );

	CEtWorldEventArea *pArea;
	for( DWORD i=0; i<pVecResult->size(); i++ ) {
		pArea = (*pVecResult)[i];
		if( !TestOBBToOBB( Box, *pArea->GetOBB() ) ) {
			pVecResult->erase( pVecResult->begin() + i );
			i--;
		}
	}
}

int CEtWorldSector::AddPropCreateUniqueCount()
{
	m_nPropCreateUniqueCount++;
	return m_nPropCreateUniqueCount;
}

CEtWorldProp *CEtWorldSector::GetPropFromCreateUniqueID( DWORD dwUniqueID )
{
	for( DWORD i=0; i<m_pVecPropList.size(); i++ ) {
		if( m_pVecPropList[i]->GetCreateUniqueID() == dwUniqueID ) return m_pVecPropList[i];
	}
	return NULL;
}

int CEtWorldSector::AddEventAreaCreateUniqueCount()
{
	m_nEventAreaCreateUniqueCount++;
	return m_nEventAreaCreateUniqueCount;
}

CEtWorldEventArea *CEtWorldSector::GetEventAreaFromCreateUniqueID( int nValue )
{
	for( DWORD i=0; i<m_pVecAreaControl.size(); i++ ) {
		CEtWorldEventControl *pControl = m_pVecAreaControl[i];
		CEtWorldEventArea *pArea = pControl->GetAreaFromCreateUniqueID( nValue );
		if( pArea ) return pArea;
	}
	return NULL;
}

CEtWorldDecal *CEtWorldSector::AllocDecal()
{
	return new CEtWorldDecal( this );
}

bool CEtWorldSector::LoadDecal( const char *szSectorPath )
{
	char szFullName[_MAX_PATH];
	sprintf_s( szFullName, "%s\\DecalInfo.ini", szSectorPath );

	CResMngStream Stream( szFullName );
	if( !Stream.IsValid() ) return true;

	int nCount;
	Stream.Read( &nCount, sizeof(int) );

	CEtWorldDecal::DecalStruct Struct;
	for( int i=0; i<nCount; i++ ) {
		Stream.Read( &Struct, sizeof(CEtWorldDecal::DecalStruct) );
		CEtWorldDecal *pDecal = AllocDecal();
		if( pDecal == NULL ) continue;
		if( pDecal->Initialize( Struct.vPos, Struct.fRadius, Struct.fRotate, Struct.vColor, Struct.fAlpha, Struct.szTextureName ) == false ) {
			SAFE_DELETE( pDecal );
		}
		InsertDecal( pDecal );
	}

	return true;
}

void CEtWorldSector::InsertDecal( CEtWorldDecal *pDecal )
{
	std::vector<CEtWorldDecal *>::iterator it = std::find( m_pVecDecalList.begin(), m_pVecDecalList.end(), pDecal );
	if( it != m_pVecDecalList.end() ) return;

	m_pVecDecalList.push_back( pDecal );

	SCircle Circle;
	pDecal->GetBoundingCircle( Circle );
	pDecal->SetCurQuadtreeNode( m_pDecalQuadtree->Insert( pDecal, Circle ) );
}

void CEtWorldSector::DeleteDecal( CEtWorldDecal *pDecal )
{
	std::vector<CEtWorldDecal *>::iterator it = std::find( m_pVecDecalList.begin(), m_pVecDecalList.end(), pDecal );
	if( it == m_pVecDecalList.end() ) return;

	if( !m_pDecalQuadtree->Remove( pDecal, pDecal->GetCurQuadtreeNode() ) ) {
		m_pDecalQuadtree->Remove( pDecal, NULL );
	}
	pDecal->SetCurQuadtreeNode( NULL );
	m_pVecDecalList.erase( it );
}

DWORD CEtWorldSector::GetDecalCount()
{
	return (DWORD)m_pVecDecalList.size();
}

CEtWorldDecal *CEtWorldSector::GetDecalFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_pVecDecalList.size() ) return NULL;
	return m_pVecDecalList[dwIndex];
}


bool CEtWorldSector::GenerationCollisionHeight( short *pCollisionHeight )
{
	NavigationMesh *pNavMesh = GetNavMesh();

	EtVector3 vOffset;
	vOffset.x = ( m_pParentGrid->GetGridWidth() * 100.f ) / 2.f;
	vOffset.z = ( m_pParentGrid->GetGridHeight() * 100.f ) / 2.f;

	for( DWORD i=0; i<GetTileWidthCount(); i++ ) {
		for( DWORD j=0; j<GetTileHeightCount(); j++ ) {
			pCollisionHeight[j*GetTileWidthCount()+i] = m_pHeight[j*GetTileWidthCount()+i];

			float fX = (float)( i * m_fTileSize );
			float fZ = (float)( j * m_fTileSize );

			DNVector(CEtWorldProp*) pVecProp;
			float fHeight = GetHeight( fX, fZ );

			fX -= vOffset.x;
			fZ -= vOffset.z;

			EtVector3 vScanPos( fX, fHeight, fZ );
			ScanProp( vScanPos, 100.f, &pVecProp );

			float fPropHeight = 0.f;
			if( pNavMesh ) {
				bool bFindCollision = false;
				NavigationCell *pCell = pNavMesh->FindCell( EtVector3( fX, fHeight, fZ ) );
				if( pCell ) {
					if( pCell->GetType() == NavigationCell::CT_PROP ) {
						fPropHeight = pCell->GetPlane()->SolveForY( fX, fZ );
						bFindCollision = true;
					}
				}
				if( bFindCollision ) {
					fHeight = max( fHeight, fPropHeight );
				}
			}

			SSegment Segment;
			SCollisionResponse Response;
			Segment.vOrigin = EtVector3( fX, fHeight + 100000.f, fZ );
			Segment.vDirection = EtVector3( 0.f, -100000.f, 0.f );
			fPropHeight = -100000.f;
			for( DWORD k=0; k<pVecProp.size(); k++ ) {
				EtObjectHandle hHandle = pVecProp[k]->GetObjectHandle();
				if( !hHandle ) continue;
				if( !hHandle->FindSegmentCollision( Segment, Response ) ) continue;

				float fTemp = fHeight + ( 100000.f * ( 1.f - Response.fContactTime ) );
				if( fTemp > fPropHeight ) fPropHeight = fTemp;
			}
			if( fPropHeight != FLT_MIN && fPropHeight > fHeight ) fHeight = fPropHeight;

			int nTemp = (int)( fHeight / m_fHeightMultiply );
			if( nTemp > SHRT_MAX ) nTemp = SHRT_MAX;
			else if( nTemp < SHRT_MIN ) nTemp = SHRT_MIN;

			pCollisionHeight[j*GetTileWidthCount()+i] = (short)nTemp;
		}
	}
	return true;
}
