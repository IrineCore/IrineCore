#include "StdAfx.h"
#include "DnWorldGrid.h"
#include "DnWorldSector.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnWorldGrid::CDnWorldGrid( CEtWorld *pWorld )
: CEtWorldGrid( pWorld )
{
	m_fWorldPosConvX = 0.f;
	m_fWorldPosConvZ = 0.f;
}

CDnWorldGrid::~CDnWorldGrid()
{
}

CEtWorldSector *CDnWorldGrid::AllocSector()
{
	return new CDnWorldSector;
}

TileTypeEnum CDnWorldGrid::GetTileType( float fX, float fZ )
{
	float fResultX, fResultZ;
	CEtWorldSector *pSector = GetSector( fX, fZ, &fResultX, &fResultZ );
	if( pSector == NULL ) return TileTypeEnum::None;
	if( !pSector->IsLoaded() ) return TileTypeEnum::None;

	return ((CDnWorldSector*)pSector)->GetTileType( fResultX, fResultZ );
}

void CDnWorldGrid::RefreshQualityLevel( int nLevel )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CDnWorldSector *pSector = (CDnWorldSector *)CEtWorldGrid::GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		if( !pSector->IsLoaded() ) continue;

		pSector->RefreshQualityLevel( nLevel );
	}
}

bool CDnWorldGrid::Initialize( const char *szName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize )
{
	bool bResult = CEtWorldGrid::Initialize( szName, dwX, dwY, dwWidth, dwHeight, dwTileSize );
	if( !bResult ) return false;

	m_fWorldPosConvX = ( GetGridWidth() * 100.f ) / 2.f;
	m_fWorldPosConvZ = ( GetGridHeight() * 100.f ) / 2.f;

	return true;
}

CEtWorldSector *CDnWorldGrid::GetSector( float fX, float fZ, float *fResultX, float *fResultZ )
{
	if( fResultX ) *fResultX = fX + m_fWorldPosConvX;
	if( fResultZ ) *fResultZ = fZ + m_fWorldPosConvZ;

	return m_ppSector[0][0];
}
