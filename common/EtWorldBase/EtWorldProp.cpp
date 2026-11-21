#include "stdafx.h"
#include "EtWorldProp.h"
#include "PropHeader.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CEtWorldProp::CEtWorldProp() :
 m_vPosition( 0.f, 0.f, 0.f ),
 m_vRotation( 0.f, 0.f, 0.f ),
 m_vScale( 0.f, 0.f, 0.f ),
 m_vPrePosition( 0.f, 0.f, 0.f ),
 m_vPreRotation(0.f, 0.f, 0.f )
{
	m_nClassID = 0;
	m_pParentSector = NULL;
	m_pCurOctreeNode = NULL;
	m_bCastShadow = false;
	m_bReceiveShadow = false;
	m_bCastLightmap = false;
	m_bLightmapInfluence = false;
	m_bIgnoreBuildNavMesh = false;
	m_bIgnoreBuildColMesh = false;
	m_nCreateUniqueID = 0;
	m_pData = NULL;
	m_bPostCreateCustomParam = false;
	m_bCastWater = false;
	m_bControlLock = false;
	m_nNavType = 0;
}

CEtWorldProp::~CEtWorldProp()
{
	ReleaseCustomInfo();
	SAFE_RELEASE_SPTR( m_Handle );
}

bool CEtWorldProp::Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale )
{
	m_pParentSector = pParentSector;

	m_szPropName = szPropName;
	m_vPosition = vPos;
	m_vRotation = vRotate;
	m_vScale = vScale;

	return true;
}

void CEtWorldProp::GetBoundingSphere( SSphere &Sphere, bool bActorSize/* = false*/ )
{
	if( !m_Handle ) return;
	m_Handle->GetBoundingSphere( Sphere );
}

void CEtWorldProp::GetBoundingBox( SAABox &Box )
{
	if( !m_Handle ) return;
	m_Handle->GetBoundingBox( Box );
}

void CEtWorldProp::GetBoundingBox( SOBB &Box )
{
	if( !m_Handle ) return;
	m_Handle->GetBoundingBox( Box );
}

void CEtWorldProp::SetCurOctreeNode( CEtOctreeNode<CEtWorldProp *> *pNode )
{
	m_pCurOctreeNode = pNode;
}

void* CEtWorldProp::AllocPropData(int& usingCount)
{
	usingCount = 256;
	return new int[usingCount];
}

// note by kalliste : Blocked for now because signal structures are sometimes copied and used in-game. (#40311)
//#define _TEST_DEFAULT_PROP_ALLOC

bool CEtWorldProp::LoadCustomInfo( CStream *pStream )
{
	ReleaseCustomInfo();
	m_bPostCreateCustomParam = false;

#ifdef _TEST_DEFAULT_PROP_ALLOC
	m_pData = new int[256];
	memset( m_pData, 0, 1024 );
	pStream->Read( m_pData, 1024 );
#else
	int usingCount = 256;
	m_pData = AllocPropData(usingCount);
	int dataSize = usingCount * 4;
	if (dataSize > 1024)
	{
		_ASSERT(0);
		SAFE_DELETEA( m_pData );
		return false;
	}

	if (m_pData != NULL)
	{
		memset( m_pData, 0, dataSize );
		pStream->Read( m_pData, dataSize );
	}

	if (dataSize < 1024)
		pStream->Seek((1024 - dataSize), SEEK_CUR);
#endif

	std::vector<EtVector2> vVec2List;
	std::vector<EtVector3> vVec3List;
	std::vector<EtVector4> vVec4List;
	std::vector<std::string> szVecList;

	m_nVec2Index.clear();
	m_nVec3Index.clear();
	m_nVec4Index.clear();
	m_nVecStrIndex.clear();

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
	// For 64-bit, buffer must be realigned due to pointer size.
	m_nVec2Index64.assign( m_nVec2Index.size(), 0 );
	m_nVec3Index64.assign( m_nVec3Index.size(), 0 );
	m_nVec4Index64.assign( m_nVec4Index.size(), 0 );
	m_nVecStrIndex64.assign( m_nVecStrIndex.size(), 0 );

#ifdef _TEST_DEFAULT_PROP_ALLOC
	int nAddSize = 256 + (int)( m_nVec2Index.size() + m_nVec3Index.size() + m_nVec4Index.size() + m_nVecStrIndex.size() );
	int *pTempBuf = new int[nAddSize];
	memset( pTempBuf, 0, nAddSize * 4 );
	int nTargetOffset = 0;
	for( int i=0; i<256; i++ ) {
		char cFlag = IsPointerTable(i);
		if( cFlag == -1 ) {
			pTempBuf[nTargetOffset] = ((int*)m_pData)[i];
			nTargetOffset++;
		}
		else {
			IncreasePointerTableIndex(i);
			nTargetOffset += 2;
		}
	}
	for( DWORD i=0; i<m_nVec2Index.size(); i++ ) m_nVec2Index[i] += m_nVec2Index64[i];
	for( DWORD i=0; i<m_nVec3Index.size(); i++ ) m_nVec3Index[i] += m_nVec3Index64[i];
	for( DWORD i=0; i<m_nVec4Index.size(); i++ ) m_nVec4Index[i] += m_nVec4Index64[i];
	for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) m_nVecStrIndex[i] += m_nVecStrIndex64[i];
	SAFE_DELETEA( m_pData );
	m_pData = pTempBuf;
#else
	int *pTempBuf = NULL;
	int nCorrectionSize = (int)( m_nVec2Index.size() + m_nVec3Index.size() + m_nVec4Index.size() + m_nVecStrIndex.size() );
	if (m_pData == NULL && nCorrectionSize != 0)
	{
		_ASSERT(0);
		return false;
	}

	if (m_pData != NULL)
	{
		int nAddSize = usingCount + nCorrectionSize;
		int *pTempBuf = new int[nAddSize];
		memset( pTempBuf, 0, nAddSize * 4 );
		int nTargetOffset = 0;
		for( int i=0; i<usingCount; i++ ) {
			char cFlag = IsPointerTable(i);
			if( cFlag == -1 ) {
				pTempBuf[nTargetOffset] = ((int*)m_pData)[i];
				nTargetOffset++;
			}
			else {
				IncreasePointerTableIndex(i);
				nTargetOffset += 2;
			}
		}

		for( DWORD i=0; i<m_nVec2Index.size(); i++ ) m_nVec2Index[i] += m_nVec2Index64[i];
		for( DWORD i=0; i<m_nVec3Index.size(); i++ ) m_nVec3Index[i] += m_nVec3Index64[i];
		for( DWORD i=0; i<m_nVec4Index.size(); i++ ) m_nVec4Index[i] += m_nVec4Index64[i];
		for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) m_nVecStrIndex[i] += m_nVecStrIndex64[i];

		SAFE_DELETEA( m_pData );
		m_pData = pTempBuf;
	}
#endif //_TEST_DEFAULT_PROP_ALLOC
#endif
	////////////////////

	if( NULL == m_pData )
	{
		_ASSERT( m_nVec2Index.empty() && "Prop type has no Custom info, yet Custom info exists. This happens when a prop type with CustomInfo was changed to one without CustomInfo while reading CustomInfo." );
		_ASSERT( m_nVec3Index.empty() && "Prop type has no Custom info, yet Custom info exists. This happens when a prop type with CustomInfo was changed to one without CustomInfo while reading CustomInfo." );
		_ASSERT( m_nVec4Index.empty() && "Prop type has no Custom info, yet Custom info exists. This happens when a prop type with CustomInfo was changed to one without CustomInfo while reading CustomInfo." );
		_ASSERT( m_nVecStrIndex.empty() && "Prop type has no Custom info, yet Custom info exists. This happens when a prop type with CustomInfo was changed to one without CustomInfo while reading CustomInfo." );
		
		return true;
	}

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

	SAFE_DELETE_VEC( vVec2List );
	SAFE_DELETE_VEC( vVec2List );
	SAFE_DELETE_VEC( vVec3List );
	SAFE_DELETE_VEC( szVecList );

	return true;
}

void CEtWorldProp::ReleaseCustomInfo()
{
	if( m_pData )
	{
		if( !m_bPostCreateCustomParam ) {
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

			SAFE_DELETEA( m_pData );
		}
	}
}

void CEtWorldProp::SetCustomParam( void *pCustomParam )
{
	ReleaseCustomInfo();
	m_bPostCreateCustomParam = true;

	m_pData = pCustomParam;
}

#ifdef WIN64
char CEtWorldProp::IsPointerTable( int nIndex )
{
	if( std::find( m_nVec2Index.begin(), m_nVec2Index.end(), nIndex ) != m_nVec2Index.end() ) return 0;
	if( std::find( m_nVec3Index.begin(), m_nVec3Index.end(), nIndex ) != m_nVec3Index.end() ) return 1;
	if( std::find( m_nVec4Index.begin(), m_nVec4Index.end(), nIndex ) != m_nVec4Index.end() ) return 2;
	if( std::find( m_nVecStrIndex.begin(), m_nVecStrIndex.end(), nIndex ) != m_nVecStrIndex.end() ) return 3;

	return -1;
}

void CEtWorldProp::IncreasePointerTableIndex( int nValue )
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