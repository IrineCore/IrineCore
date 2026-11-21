#include "stdafx.h"
#include "EtWorld.h"
#include "EtWorldGrid.h"
#include "EtWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CEtWorldGrid::CEtWorldGrid( CEtWorld *pWorld )
{
	m_dwGridX = m_dwGridY = 1;
	m_dwGridWidth = m_dwGridHeight = 2000;
	m_pParent = NULL;
	m_pWorld = pWorld;
	m_pProcessSector = NULL;
	m_dwTileSize = 0;
	m_fSectorCalcValueX = m_fSectorCalcValueY = 0.f;
}

CEtWorldGrid::~CEtWorldGrid()
{
	SAFE_DELETE_PVEC( m_pVecChild );
	FreeSector();
}

bool CEtWorldGrid::Initialize( const char *szName, DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, DWORD dwTileSize )
{
	if( szName ) SetName( szName );
	m_dwGridX = dwX;
	m_dwGridY = dwY;
	m_dwGridWidth = dwWidth;
	m_dwGridHeight = dwHeight;
	m_dwTileSize = dwTileSize;
	m_fSectorCalcValueX = ( ( GetGridWidth() * 100 ) * GetGridX() ) / 2.f;
	m_fSectorCalcValueY = ( ( GetGridHeight() * 100 ) * GetGridY() ) / 2.f;

	// Returns -1 if root.
	if( dwX == 0 && dwY == 0 && dwWidth == 0 && dwHeight == 0 ) return true;

	// Alloc Sector
	CEtWorldSector *pSector;
	std::vector< CEtWorldSector * > pVecList;
	for( DWORD j=0; j<m_dwGridX; j++ ) {
		pVecList.clear();
		for( DWORD i=0; i<m_dwGridY; i++ ) {
			pSector = AllocSector();
			pSector->Initialize( this, SectorIndex( j, i ) );
			pSector->SetTileSize( (float)dwTileSize );
			pVecList.push_back( pSector );
		}
		m_ppSector.push_back( pVecList );
	}
	return true;
}

bool CEtWorldGrid::LoadGrid( const char *szName )
{
	CResMngStream Stream( szName );
	if( !Stream.IsValid() ) 
		return false;

	InfoFile_Header Header;
	_ASSERT( sizeof(InfoFile_Header) == Stream.Size() );
	if( Stream.Read( &Header, sizeof(InfoFile_Header) ) < 0  ) 
		return false;

	return Initialize( NULL, Header.dwGridX, Header.dwGridY, Header.dwGridWidth, Header.dwGridHeight, Header.dwTileSize );

}

bool CEtWorldGrid::LoadSector( SectorIndex Index, bool bThreadLoad, int nLoadSectorEnum )
{
	CEtWorldSector *pSector = GetSector( Index );
	if( !pSector ) return false;
	if( pSector->IsLoaded() ) return false;

	if( pSector->Load( -1, bThreadLoad, nLoadSectorEnum ) == false ) {
		pSector->SetLoadFailed( true );
		return false;
	}
	m_VecActiveSector.push_back( Index );
	return true;
}

void CEtWorldGrid::FreeSector( SectorIndex Index )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		if( m_VecActiveSector[i] == Index ) {
			m_ppSector[Index.nX][Index.nY]->Free();
			m_VecActiveSector.erase( m_VecActiveSector.begin() + i );
			break;
		}
	}
}

CEtWorldSector *CEtWorldGrid::AllocSector()
{
	return new CEtWorldSector;
}

void CEtWorldGrid::FreeSector()
{
	for( DWORD i=0; i<m_ppSector.size(); i++ ) {
		SAFE_DELETE_PVEC( m_ppSector[i] );
	}
	SAFE_DELETE_VEC( m_ppSector );
}


CEtWorldSector *CEtWorldGrid::GetSector( SectorIndex Index ) 
{ 
	return m_ppSector[Index.nX][Index.nY]; 
}

DWORD CEtWorldGrid::GetActiveSectorCount()
{
	return (DWORD)m_VecActiveSector.size();
}

CEtWorldSector *CEtWorldGrid::GetActiveSector( DWORD dwIndex )
{
	if( dwIndex >= m_VecActiveSector.size() ) return NULL;
	return m_ppSector[ m_VecActiveSector[dwIndex].nX ][ m_VecActiveSector[dwIndex].nY ];
}

CEtWorldSector *CEtWorldGrid::GetSector( float fX, float fZ, float *fResultX, float *fResultZ )
{
	int nWidth = GetGridWidth() * 100;
	int nHeight = GetGridHeight() * 100;
	fX += m_fSectorCalcValueX;
	fZ += m_fSectorCalcValueY;

	int nX = (int)( fX / nWidth );
	int nY = (int)( fZ / nHeight );
	if( fResultX ) *fResultX = fX - ( nX * nWidth );
	if( fResultZ ) *fResultZ = fZ - ( nY * nHeight );

	if( nX < 0 || nX >= (int)m_dwGridX ) return NULL;
	if( nY < 0 || nY >= (int)m_dwGridY ) return NULL;

	return m_ppSector[nX][nY];
}

float CEtWorldGrid::GetHeight( float fX, float fZ, EtVector3 *pNormal, int nTileScale )
{
	float fResultX, fResultZ;
	CEtWorldSector *pSector = GetSector( fX, fZ, &fResultX, &fResultZ );
	if( pSector == NULL ) return 0.f;
	if( !pSector->IsLoaded() ) return 0.f;

	return pSector->GetHeight( fResultX, fResultZ, pNormal, nTileScale );
}

float CEtWorldGrid::GetCollisionHeight( float fX, float fZ, EtVector3 *pNormal, int nTileScale )
{
	float fResultX, fResultZ;
	CEtWorldSector *pSector = GetSector( fX, fZ, &fResultX, &fResultZ );
	if( pSector == NULL ) return 0.f;
	if( !pSector->IsLoaded() ) return 0.f;

	return pSector->GetCollisionHeight( fResultX, fResultZ, pNormal, nTileScale );
}

bool CEtWorldGrid::GetWaterHeight( float fX, float fZ, float *pfHeight )
{
	float fResultX, fResultZ;
	CEtWorldSector *pSector = GetSector( fX, fZ, &fResultX, &fResultZ );
	if( pSector == NULL ) return false;
	if( !pSector->IsLoaded() ) return false;

	return pSector->GetWaterHeight( fResultX, fResultZ, pfHeight );
}

char CEtWorldGrid::GetAttribute( float fX, float fZ )
{
	float fResultX, fResultZ;
	CEtWorldSector *pSector = GetSector( fX, fZ, &fResultX, &fResultZ );
	if( pSector == NULL ) return 0;
	if( !pSector->IsLoaded() ) return 0;

	return pSector->GetAttribute( fResultX, fResultZ );
}

int CEtWorldGrid::GetAttributeBlockSize( float fX, float fZ )
{
	CEtWorldSector *pSector = GetSector( fX, fZ );
	if( pSector == NULL ) return 0;
	if( !pSector->IsLoaded() ) return 0;

	return pSector->GetAttributeBlockSize();
}

NavigationMesh *CEtWorldGrid::GetNavMesh( EtVector3 &vPos )
{
	CEtWorldSector *pSector = GetSector( vPos.x, vPos.z );
	if( pSector == NULL ) return 0;
	if( !pSector->IsLoaded() ) return 0;

	return pSector->GetNavMesh();
}

bool CEtWorldGrid::Pick( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	CEtWorldSector *pSector = GetSector( vOrig.x, vOrig.z );
	if( pSector == NULL ) return false;
	if( !pSector->IsLoaded() ) return false;

	return pSector->Pick( vOrig, vDir, vPickPos );
}

bool CEtWorldGrid::PickWater( EtVector3 &vOrig, EtVector3 &vDir, EtVector3 &vPickPos )
{
	CEtWorldSector *pSector = GetSector( vOrig.x, vOrig.z );
	if( pSector == NULL ) return false;
	if( !pSector->IsLoaded() ) return false;

	return pSector->PickWater( vOrig, vDir, vPickPos );
}

void CEtWorldGrid::SetName( const char *szName )
{
	if( szName == NULL ) m_szName.clear();
	else m_szName = szName;
}

const char *CEtWorldGrid::GetName()
{
	return m_szName.c_str();
}

void CEtWorldGrid::SetParent( CEtWorldGrid *pGrid )
{
	m_pParent = pGrid;
}

DWORD CEtWorldGrid::GetChildCount()
{
	return (DWORD)m_pVecChild.size();
}

CEtWorldGrid *CEtWorldGrid::GetChildFromIndex( DWORD dwIndex )
{
	if( dwIndex >= m_pVecChild.size() ) return NULL;
	return m_pVecChild[dwIndex];
}


void CEtWorldGrid::InsertChild( CEtWorldGrid *pGrid )
{
	m_pVecChild.push_back( pGrid );
}

void CEtWorldGrid::RemoveChild( CEtWorldGrid *pGrid )
{
	for( DWORD i=0; i<m_pVecChild.size(); i++ ) {
		if( m_pVecChild[i] == pGrid ) {
			m_pVecChild.erase( m_pVecChild.begin() + i );
			break;
		}
	}
}

void CEtWorldGrid::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// Here CEtWorld's m_vPosition, m_vDirection, m_fDirectionLengt
	// Loads what needs to be loaded and deletes what needs to be deleted.
	for( DWORD i=0; i<m_ppSector.size(); i++ ) {
		for( DWORD j=0; j<m_ppSector[i].size(); j++ ) {
			EtVector3 vOffset = *m_ppSector[i][j]->GetOffset() - *m_pWorld->GetUpdatePosition();
			float fLength = EtVec3Length( &vOffset );
			if( fLength < m_pWorld->GetUpdateDirectionLength() || fLength < max( m_pWorld->GetGridWidth() * 100.f, m_pWorld->GetGridHeight() * 100.f ) / 2.f ) {
				if( !m_ppSector[i][j]->IsLoaded() )
					LoadSector( SectorIndex( i, j ), false );
			}
			else {
				if( m_ppSector[i][j]->IsLoaded() )
					FreeSector( SectorIndex( i, j ) );
			}
		}
	}
	if( LocalTime == -1 ) return;

	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		m_pProcessSector = pSector;
		pSector->Process( LocalTime, fDelta );
	}
	m_pProcessSector = NULL;
}

CEtWorldProp *CEtWorldGrid::AddProp( const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale, void *pCustomParam )
{
	CEtWorldSector *pSector = GetSector( vPos.x, vPos.z );
	if( pSector == NULL ) return NULL;
	if( !pSector->IsLoaded() ) return NULL;

	return pSector->AddProp( szPropName, vPos, vRotate, vScale, pCustomParam );
}

void CEtWorldGrid::ScanProp( EtVector3 &vPos, float fRadius, DNVector(CEtWorldProp*) *pVecResult )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		pSector->ScanProp( vPos, fRadius, pVecResult );
	}
}

void CEtWorldGrid::ScanDecal( EtVector2 &vPos, float fRadius, std::vector<CEtWorldDecal *> *pVecResult )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		pSector->ScanDecal( vPos, fRadius, pVecResult );
	}
}

void CEtWorldGrid::ScanEventArea( EtVector2 &vPos, float fRadius, std::vector<CEtWorldEventArea *> *pVecResult )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		pSector->ScanEventArea( vPos, fRadius, pVecResult );
	}
}

void CEtWorldGrid::ScanEventArea( SAABox &Box, std::vector<CEtWorldEventArea *> *pVecResult )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		pSector->ScanEventArea( Box, pVecResult );
	}
}

void CEtWorldGrid::ScanEventArea( SOBB &Box, std::vector<CEtWorldEventArea *> *pVecResult )
{
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		pSector->ScanEventArea( Box, pVecResult );
	}
}


int CEtWorldGrid::FindPropFromCreateUniqueID( DWORD dwUniqueID, std::vector<CEtWorldProp *> *pVecResult )
{
	int nCount = 0;
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		CEtWorldProp *pProp = pSector->GetPropFromCreateUniqueID( dwUniqueID );
		if( pProp ) {
			if( pVecResult ) pVecResult->push_back( pProp );
			nCount++;
		}
	}
	return nCount;
}

int CEtWorldGrid::FindEventControlFromUniqueID( int nUniqueID, std::vector<CEtWorldEventControl *> *pVecList )
{
	int nCount = 0;
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( nUniqueID );
		if( pControl ) {
			if( pVecList ) pVecList->push_back( pControl );
			nCount++;
		}
	}
	return nCount;
}

int CEtWorldGrid::FindEventAreaFromName( int nUniqueID, const char *szAreaName, std::vector<CEtWorldEventArea *> *pVecList )
{
	int nCount = 0;
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( nUniqueID );
		if( !pControl ) continue;

		std::vector<CEtWorldEventArea *> pVecAreaList;
		nCount += pControl->FindAreaFromName( szAreaName, pVecList );
	}
	return nCount;
}

int CEtWorldGrid::FindEventAreaFromSubStr( int nUniqueID, const char *szStr, std::vector<CEtWorldEventArea *> *pVecList )
{
	int nCount = 0;
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		CEtWorldEventControl *pControl = pSector->GetControlFromUniqueID( nUniqueID );
		if( !pControl ) continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) {
			CEtWorldEventArea *pArea = pControl->GetAreaFromIndex(j);
			if( strstr( pArea->GetName(), szStr ) ) {
				if( pVecList ) pVecList->push_back( pArea );
				nCount++;
			}
		}
	}
	return nCount;
}

int CEtWorldGrid::FindEventAreaFromCreateUniqueID( int nCreateUniqueID, std::vector<CEtWorldEventArea *> *pVecList )
{
	int nCount = 0;
	for( DWORD i=0; i<m_VecActiveSector.size(); i++ ) {
		CEtWorldSector *pSector = GetSector( m_VecActiveSector[i] );
		if( !pSector ) continue;
		for( DWORD j=0; j<pSector->GetControlCount(); j++ ) {
			CEtWorldEventControl *pControl = pSector->GetControlFromIndex(j);
			if( !pControl ) continue;

			CEtWorldEventArea *pArea = pControl->GetAreaFromCreateUniqueID( nCreateUniqueID );
			if( pArea ) {
				if( pVecList ) pVecList->push_back( pArea );
				nCount++;
			}
		}
	}
	return nCount;
}

CEtWorldSector *CEtWorldGrid::GetProcessSector()
{
	return m_pProcessSector;
}

void CEtWorldGrid::CalcWorldBasePos( float fX, float fZ, float &fResultX, float &fResultZ )
{
	GetSector( fX, fZ, &fResultX, &fResultZ );
}
