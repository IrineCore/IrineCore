#include "stdafx.h"
#include "EtWorldEventArea.h"
#include "EtWorldEventControl.h"
#include "EtWorldSector.h"
#include "EventControlHeader.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

int CEtWorldEventArea::s_nDummySize = 508;
CEtWorldEventArea::CEtWorldEventArea( CEtWorldEventControl *pControl )
: m_vMin( 0.f, 0.f, 0.f )
, m_vMax( 0.f, 0.f, 0.f )
{
	m_pControl = pControl;
	m_bActive = true;
	m_pData = NULL;
	m_fRotate = 0.f;
	m_nCreateUniqueID = 0;
	memset(&m_ObbBox,0,sizeof(m_ObbBox));
	m_pCurrentNode = NULL;
	m_Circle.fRadius = 0.f;
	m_Circle.Center = EtVector2( 0.f, 0.f );
}

CEtWorldEventArea::~CEtWorldEventArea()
{
	if( m_pData ) {
		for( DWORD i=0; i<m_nVec2Index.size(); i++ ) {
			EtVector2 *pVec;
			memcpy( &pVec, (int*)(m_pData) + m_nVec2Index[i], sizeof(int*) );
			SAFE_DELETE( pVec );
		}

		for( DWORD i=0; i<m_nVec3Index.size(); i++ ) {
			EtVector3 *pVec;
			memcpy( &pVec, (int*)(m_pData) + m_nVec3Index[i], sizeof(int*) );
			SAFE_DELETE( pVec );
		}
		for( DWORD i=0; i<m_nVec4Index.size(); i++ ) {
			EtVector4 *pVec;
			memcpy( &pVec, (int*)(m_pData) + m_nVec4Index[i], sizeof(int*) );
			SAFE_DELETE( pVec );
		}
		for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) {
			char *pPtr;
			memcpy( &pPtr, (int*)(m_pData) + m_nVecStrIndex[i], sizeof(int*) );
			SAFE_DELETEA( pPtr );
		}
	}

	SAFE_DELETEA( m_pData );
}

bool CEtWorldEventArea::CheckArea( EtVector3 &vPosition, bool bIgnoreY )
{
	if( bIgnoreY ) {
		EtVector3 vVertex = vPosition - m_ObbBox.Center;

		float fDot = fabs( EtVec3Dot( &m_ObbBox.Axis[0], &vVertex ) );
		if( fDot > m_ObbBox.Extent[0] ) return false;

		fDot = fabs( EtVec3Dot( &m_ObbBox.Axis[2], &vVertex ) );
		if( fDot > m_ObbBox.Extent[2] ) return false;
		return true;
	}

	return m_ObbBox.IsInside( vPosition );
}

// note by kalliste : Blocked for now because signal structures are sometimes copied and used in-game. (#40311)
//#define _TEST_DEFAULT_EVENT_ALLOC

bool CEtWorldEventArea::Load( CStream *pStream )
{
	ReadStdString( m_szName, pStream );
	pStream->Read( &m_vMin, sizeof(EtVector3) );
	pStream->Read( &m_vMax, sizeof(EtVector3) );
	pStream->Read( &m_fRotate, sizeof(float) );
	pStream->Read( &m_nCreateUniqueID, sizeof(int) );

	pStream->Seek( s_nDummySize, SEEK_CUR );

#ifdef _TEST_DEFAULT_EVENT_ALLOC
	m_pData = new int[256];
	memset( m_pData, 0, 1024 );
	pStream->Read( m_pData, 1024 );
#elif defined(WIN64)
	const int eventID = (GetControl() != NULL) ? GetControl()->GetUniqueID() : -1;
	const int usingCount32 = (eventID >= 0) ? GetEventSignalDataUsingCount(eventID) : 256;
	const int usingCount = (eventID >= 0) ? GetEventSignalDataUsingCount64(eventID) : 256;
	int rawData32[256];
	memset( rawData32, 0, sizeof(rawData32) );
	int datasize32 = (usingCount32 > 0 && usingCount32 <= 256) ? (usingCount32 * (int)sizeof(int)) : 0;
	if (datasize32 > 0)
	{
		pStream->Read( rawData32, datasize32 );
		if (datasize32 < 1024)
			pStream->Seek((1024 - datasize32), SEEK_CUR);
	}
	else
	{
		pStream->Seek(1024, SEEK_CUR);
	}
#else
	int usingCount = 256;
	if( GetControl() != NULL ) {
		usingCount = GetEventSignalDataUsingCount(GetControl()->GetUniqueID());
	}
	int datasize = usingCount * sizeof(int);

	if (usingCount > 0)
	{
		m_pData = new int[usingCount];
		memset( m_pData, 0, datasize );
		pStream->Read( m_pData, datasize );
		pStream->Seek((1024 - datasize), SEEK_CUR);
	}
	else
	{
		pStream->Seek(1024, SEEK_CUR);
	}
#endif

	std::vector<EtVector2> vVec2List;
	std::vector<EtVector3> vVec3List;
	std::vector<EtVector4> vVec4List;
	std::vector<std::string> szVecList;

	// Table Load
	int nTableCount;
	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		vVec2List.resize( nTableCount );
		m_nVec2Index.resize( nTableCount );
		pStream->Read( &m_nVec2Index[0], sizeof(int) * nTableCount );
		pStream->Read( &vVec2List[0], sizeof(EtVector2) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		vVec3List.resize( nTableCount );
		m_nVec3Index.resize( nTableCount );
		pStream->Read( &m_nVec3Index[0], sizeof(int) * nTableCount );
		pStream->Read( &vVec3List[0], sizeof(EtVector3) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		vVec4List.resize( nTableCount );
		m_nVec4Index.resize( nTableCount );
		pStream->Read( &m_nVec4Index[0], sizeof(int) * nTableCount );
		pStream->Read( &vVec4List[0], sizeof(EtVector4) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		m_nVecStrIndex.resize( nTableCount );
		pStream->Read( &m_nVecStrIndex[0], sizeof(int) * nTableCount );
	}
	for( int i=0; i<nTableCount; i++ ) {
		std::string szStr;
		ReadStdString( szStr, pStream );
		szVecList.push_back( szStr );
	}
#ifdef WIN64
	// For 64-bit, expand every schema pointer slot (even if empty/omitted in older maps)
	// as well as serialized pointer table slots from 4 bytes to 8 bytes.
	m_nVec2Index64.clear();
	m_nVec3Index64.clear();
	m_nVec4Index64.clear();
	m_nVecStrIndex64.clear();

#ifdef _TEST_DEFAULT_EVENT_ALLOC
	const int eventID = (GetControl() != NULL) ? GetControl()->GetUniqueID() : -1;
	int slotMap[256];
	int nTargetOffset = 0;
	for( int i=0; i<256; i++ ) {
		slotMap[i] = nTargetOffset;
		if( IsEventPointerSlot32(eventID, i) || IsPointerTable(i) != -1 )
			nTargetOffset += 2;
		else
			nTargetOffset += 1;
	}
	int *pTempBuf = new int[nTargetOffset];
	memset( pTempBuf, 0, nTargetOffset * sizeof(int) );
	for( int i=0; i<256; i++ ) {
		if( !IsEventPointerSlot32(eventID, i) && IsPointerTable(i) == -1 )
			pTempBuf[slotMap[i]] = ((int*)m_pData)[i];
	}
	for( DWORD i=0; i<m_nVec2Index.size(); i++ ) if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] < 256 ) m_nVec2Index[i] = slotMap[m_nVec2Index[i]];
	for( DWORD i=0; i<m_nVec3Index.size(); i++ ) if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] < 256 ) m_nVec3Index[i] = slotMap[m_nVec3Index[i]];
	for( DWORD i=0; i<m_nVec4Index.size(); i++ ) if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] < 256 ) m_nVec4Index[i] = slotMap[m_nVec4Index[i]];
	for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] < 256 ) m_nVecStrIndex[i] = slotMap[m_nVecStrIndex[i]];
	SAFE_DELETEA( m_pData );
	m_pData = pTempBuf;
#else
	if (usingCount32 > 0 && usingCount32 <= 256)
	{
		int slotMap[256];
		int nTargetOffset = 0;
		for( int i=0; i<usingCount32; i++ ) {
			slotMap[i] = nTargetOffset;
			if( IsEventPointerSlot32(eventID, i) || IsPointerTable(i) != -1 )
				nTargetOffset += 2;
			else
				nTargetOffset += 1;
		}

		int nAllocCount = (usingCount > nTargetOffset) ? usingCount : nTargetOffset;
		int *pTempBuf = new int[nAllocCount];
		memset( pTempBuf, 0, nAllocCount * sizeof(int) );
		for( int i=0; i<usingCount32; i++ ) {
			if( !IsEventPointerSlot32(eventID, i) && IsPointerTable(i) == -1 )
				pTempBuf[slotMap[i]] = rawData32[i];
		}

		{
			std::vector<int> keptIdx; std::vector<EtVector2> keptVal;
			for( DWORD i=0; i<m_nVec2Index.size(); i++ ) {
				if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] < usingCount32 ) {
					keptIdx.push_back( slotMap[m_nVec2Index[i]] );
					keptVal.push_back( vVec2List[i] );
				}
			}
			m_nVec2Index.swap( keptIdx ); vVec2List.swap( keptVal );
		}
		{
			std::vector<int> keptIdx; std::vector<EtVector3> keptVal;
			for( DWORD i=0; i<m_nVec3Index.size(); i++ ) {
				if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] < usingCount32 ) {
					keptIdx.push_back( slotMap[m_nVec3Index[i]] );
					keptVal.push_back( vVec3List[i] );
				}
			}
			m_nVec3Index.swap( keptIdx ); vVec3List.swap( keptVal );
		}
		{
			std::vector<int> keptIdx; std::vector<EtVector4> keptVal;
			for( DWORD i=0; i<m_nVec4Index.size(); i++ ) {
				if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] < usingCount32 ) {
					keptIdx.push_back( slotMap[m_nVec4Index[i]] );
					keptVal.push_back( vVec4List[i] );
				}
			}
			m_nVec4Index.swap( keptIdx ); vVec4List.swap( keptVal );
		}
		{
			std::vector<int> keptIdx; std::vector<std::string> keptVal;
			for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) {
				if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] < usingCount32 ) {
					keptIdx.push_back( slotMap[m_nVecStrIndex[i]] );
					keptVal.push_back( szVecList[i] );
				}
			}
			m_nVecStrIndex.swap( keptIdx ); szVecList.swap( keptVal );
		}

		m_pData = pTempBuf;
	}
	else
	{
		m_nVec2Index.clear(); vVec2List.clear();
		m_nVec3Index.clear(); vVec3List.clear();
		m_nVec4Index.clear(); vVec4List.clear();
		m_nVecStrIndex.clear(); szVecList.clear();
	}
#endif // _TEST_DEFAULT_EVENT_ALLOC
#endif
	////////////////////
	if( m_pData != NULL ) {
		nTableCount = (int)m_nVec2Index.size();
		for( int i=0; i<nTableCount; i++ ) {
			EtVector2 *pVec = new EtVector2;
			*pVec = vVec2List[i];
			memcpy( (int*)m_pData + ( m_nVec2Index[i] ), &pVec, sizeof(int*) );
		}
		nTableCount = (int)m_nVec3Index.size();
		for( int i=0; i<nTableCount; i++ ) {
			EtVector3 *pVec = new EtVector3;
			*pVec = vVec3List[i];
			memcpy( (int*)m_pData + ( m_nVec3Index[i] ), &pVec, sizeof(int*) );
		}
		nTableCount = (int)m_nVec4Index.size();
		for( int i=0; i<nTableCount; i++ ) {
			EtVector4 *pVec = new EtVector4;
			*pVec = vVec4List[i];
			memcpy( (int*)m_pData + ( m_nVec4Index[i] ), &pVec, sizeof(int*) );
		}
		nTableCount = (int)m_nVecStrIndex.size();
		for( int i=0; i<nTableCount; i++ ) {
			char *pStr = new char[szVecList[i].size()+1];
			memset( pStr, 0, szVecList[i].size()+1 );
			strcpy_s( pStr, szVecList[i].size()+1, szVecList[i].c_str() );

			memcpy( (int*)m_pData + ( m_nVecStrIndex[i] ), &pStr, sizeof(int*) );
		}
	}

	SAFE_DELETE_VEC( vVec2List );
	SAFE_DELETE_VEC( vVec3List );
	SAFE_DELETE_VEC( vVec4List );
	SAFE_DELETE_VEC( szVecList );

	return true;
}

void CEtWorldEventArea::CalcOBB()
{
	m_ObbBox.Center = m_vMin + ( m_vMax - m_vMin ) / 2.f;
	m_ObbBox.Extent[0] = ( m_vMax.x - m_vMin.x ) / 2.f;
	m_ObbBox.Extent[1] = ( m_vMax.y - m_vMin.y ) / 2.f;
	m_ObbBox.Extent[2] = ( m_vMax.z - m_vMin.z ) / 2.f;
	if( m_ObbBox.Extent[1] == 0.f ) m_ObbBox.Extent[1] = 100000.f;

	CEtWorldSector *pSector;
	if( GetControl() ) {
		pSector = GetControl()->GetSector();
		if( pSector ) {
			float fHeight = pSector->GetHeightToWorld( m_ObbBox.Center.x, m_ObbBox.Center.z );
			m_ObbBox.Center.y += fHeight;
		}
	}

	EtMatrix matRotate;
	EtMatrixRotationY( &matRotate, EtToRadian( m_fRotate ) );
	EtVec3TransformNormal( &m_ObbBox.Axis[0], &EtVector3( 1.f, 0.f, 0.f ), &matRotate );
	EtVec3TransformNormal( &m_ObbBox.Axis[1], &EtVector3( 0.f, 1.f, 0.f ), &matRotate );
	EtVec3TransformNormal( &m_ObbBox.Axis[2], &EtVector3( 0.f, 0.f, 1.f ), &matRotate );

	m_ObbBox.CalcVertices();

	m_Circle.Center = EtVector2( m_ObbBox.Center.x, m_ObbBox.Center.z );
	m_Circle.fRadius = max( m_ObbBox.Extent[0], m_ObbBox.Extent[2] );
}

void CEtWorldEventArea::GetBoundingCircle( SCircle &Circle )
{
	Circle = m_Circle;
}


#ifdef WIN64
char CEtWorldEventArea::IsPointerTable( int nIndex )
{
	if( std::find( m_nVec2Index.begin(), m_nVec2Index.end(), nIndex ) != m_nVec2Index.end() ) return 0;
	if( std::find( m_nVec3Index.begin(), m_nVec3Index.end(), nIndex ) != m_nVec3Index.end() ) return 1;
	if( std::find( m_nVec4Index.begin(), m_nVec4Index.end(), nIndex ) != m_nVec4Index.end() ) return 2;
	if( std::find( m_nVecStrIndex.begin(), m_nVecStrIndex.end(), nIndex ) != m_nVecStrIndex.end() ) return 3;

	return -1;
}

void CEtWorldEventArea::IncreasePointerTableIndex( int nValue )
{
	for( DWORD i=0; i<m_nVec2Index.size(); i++ ) {
		if( m_nVec2Index[i] > nValue ) m_nVec2Index64[i]++;
	}
	for( DWORD i=0; i<m_nVec3Index.size(); i++ ) {
		if( m_nVec3Index[i] > nValue ) m_nVec3Index64[i]++;
	}
	for( DWORD i=0; i<m_nVec4Index.size(); i++ ) {
		if( m_nVec4Index[i] > nValue ) m_nVec4Index64[i]++;
	}
	for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) {
		if( m_nVecStrIndex[i] > nValue ) m_nVecStrIndex64[i]++;
	}
}

#endif