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
#elif defined(WIN64)
	int usingCount = 256;
	m_pData = AllocPropData(usingCount);
	if (usingCount * 4 > 1024)
	{
		_ASSERT(0);
		SAFE_DELETEA( m_pData );
		return false;
	}

	// PropInfo.ini custom blocks are always serialized in 32-bit (256 x 4-byte) slot layout.
	int rawData32[256];
	memset( rawData32, 0, sizeof(rawData32) );
	int usingCount32 = 0;
	if (m_pData != NULL)
	{
		const int schema32 = GetPropSignalDataUsingCount(m_nClassID);
		usingCount32 = (usingCount >= 256) ? 256 : ((schema32 > 0 && schema32 <= 256) ? schema32 : 256);
		int dataSize32 = usingCount32 * 4;
		pStream->Read( rawData32, dataSize32 );
		if (dataSize32 < 1024)
			pStream->Seek((1024 - dataSize32), SEEK_CUR);
	}
	else
	{
		pStream->Seek(1024, SEEK_CUR);
	}
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
	// For 64-bit, expand every schema pointer slot (even if empty/omitted in older map data,
	// e.g. ChestStruct::strLevelBound*) and serialized pointer table slot from 4 bytes to 8 bytes.
	m_nVec2Index64.clear();
	m_nVec3Index64.clear();
	m_nVec4Index64.clear();
	m_nVecStrIndex64.clear();

#ifdef _TEST_DEFAULT_PROP_ALLOC
	int slotMap[256];
	int nTargetOffset = 0;
	for( int i=0; i<256; i++ ) {
		slotMap[i] = nTargetOffset;
		if( IsPropPointerSlot32(m_nClassID, i) || IsPointerTable(i) != -1 )
			nTargetOffset += 2;
		else
			nTargetOffset += 1;
	}
	int *pTempBuf = new int[nTargetOffset];
	memset( pTempBuf, 0, nTargetOffset * sizeof(int) );
	for( int i=0; i<256; i++ ) {
		if( !IsPropPointerSlot32(m_nClassID, i) && IsPointerTable(i) == -1 )
			pTempBuf[slotMap[i]] = ((int*)m_pData)[i];
	}
	for( DWORD i=0; i<m_nVec2Index.size(); i++ ) if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] < 256 ) m_nVec2Index[i] = slotMap[m_nVec2Index[i]];
	for( DWORD i=0; i<m_nVec3Index.size(); i++ ) if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] < 256 ) m_nVec3Index[i] = slotMap[m_nVec3Index[i]];
	for( DWORD i=0; i<m_nVec4Index.size(); i++ ) if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] < 256 ) m_nVec4Index[i] = slotMap[m_nVec4Index[i]];
	for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] < 256 ) m_nVecStrIndex[i] = slotMap[m_nVecStrIndex[i]];
	SAFE_DELETEA( m_pData );
	m_pData = pTempBuf;
#else
	int nCorrectionSize = (int)( m_nVec2Index.size() + m_nVec3Index.size() + m_nVec4Index.size() + m_nVecStrIndex.size() );
	if (m_pData == NULL && nCorrectionSize != 0)
	{
		_ASSERT(0);
		return false;
	}

	if (m_pData != NULL)
	{
		int slotMap[256];
		int nTargetOffset = 0;
		for( int i=0; i<usingCount32; i++ ) {
			slotMap[i] = nTargetOffset;
			if( IsPropPointerSlot32(m_nClassID, i) || IsPointerTable(i) != -1 )
				nTargetOffset += 2;
			else
				nTargetOffset += 1;
		}

		int nAllocCount = (usingCount > nTargetOffset) ? usingCount : nTargetOffset;
		int *pTempBuf = new int[nAllocCount];
		memset( pTempBuf, 0, nAllocCount * sizeof(int) );
		for( int i=0; i<usingCount32; i++ ) {
			if( !IsPropPointerSlot32(m_nClassID, i) && IsPointerTable(i) == -1 )
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
	SAFE_DELETE_VEC( vVec3List );
	SAFE_DELETE_VEC( vVec4List );
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