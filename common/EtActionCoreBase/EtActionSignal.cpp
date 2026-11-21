#include "stdafx.h"
#include "EtActionSignal.h"
#include "EtActionCoreMng.h"
#include "SignalHeader.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CEtActionSignal::CEtActionSignal()
{
	m_nSignalIndex = -1;
	m_nSignalListArrayIndex	= -1;
	m_nStartFrame = -1;
	m_nEndFrame = -1;
	m_cOrder = 0;
	m_pData = NULL;
}

CEtActionSignal::~CEtActionSignal()
{
	if( m_pData ) {

#ifdef WIN64
		const int usingCount = GetSignalDataUsingCount64(m_nSignalIndex);
		const bool bUnbounded = (usingCount >= 64);
#else
		const int usingCount = GetSignalDataUsingCount(m_nSignalIndex);
		const bool bUnbounded = false;
#endif

		for( DWORD i=0; i<m_nVec2Index.size(); i++ ) {
			if( m_nVec2Index[i] >= 0 && (bUnbounded || m_nVec2Index[i] < usingCount) )
			{
				EtVector2 *pVec;
				memcpy( &pVec, (int*)(m_pData) + m_nVec2Index[i], sizeof(int*) );
				SAFE_DELETE( pVec );
			}
		}

		for( DWORD i=0; i<m_nVec3Index.size(); i++ ) {
			if( m_nVec3Index[i] >= 0 && (bUnbounded || m_nVec3Index[i] < usingCount) )
			{
				EtVector3 *pVec;
				memcpy( &pVec, (int*)(m_pData) + m_nVec3Index[i], sizeof(int*) );
				SAFE_DELETE( pVec );
			}
		}
		for( DWORD i=0; i<m_nVec4Index.size(); i++ ) {
			if( m_nVec4Index[i] >= 0 && (bUnbounded || m_nVec4Index[i] < usingCount) )
			{
				EtVector4 *pVec;
				memcpy( &pVec, (int*)(m_pData) + m_nVec4Index[i], sizeof(int*) );
				SAFE_DELETE( pVec );
			}
		}
		for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) {
			if( m_nVecStrIndex[i] >= 0 && (bUnbounded || m_nVecStrIndex[i] < usingCount) )
			{
				char *pPtr;
				memcpy( &pPtr, (int*)(m_pData) + m_nVecStrIndex[i], sizeof(int*) );
				SAFE_DELETEA( pPtr );
			}
		}
	}

	SAFE_DELETEA( m_pData );
}

// note by kalliste : Blocked for now because signal structures are sometimes copied and used in-game. (#40311)
//#define _TEST_DEFAULT_SIGNAL_ALLOC

bool CEtActionSignal::LoadSignal( CStream *pStream )
{
	int nTemp;
	pStream->Read( &nTemp, sizeof(int) );
	m_nSignalIndex = nTemp;
	pStream->Read( &nTemp, sizeof(int) );
	m_nStartFrame = nTemp;
	pStream->Read( &nTemp, sizeof(int) );
	m_nEndFrame = nTemp;
	pStream->Read( &nTemp, sizeof(int) );
	m_cOrder = nTemp;
//	pStream->Seek( sizeof(int), SEEK_CUR );

#ifdef _TEST_DEFAULT_SIGNAL_ALLOC
	m_pData = new int[64];
	pStream->Read( m_pData, 256 );
#elif defined(WIN64)
	// Binary .act signal payloads are always serialized in 32-bit (64 x 4-byte) slot layout.
	const int usingCount32 = GetSignalDataUsingCount(m_nSignalIndex);
	const int usingCount = GetSignalDataUsingCount64(m_nSignalIndex);
	int rawData32[64];
	memset( rawData32, 0, sizeof(rawData32) );
	DWORD dataSize32 = (usingCount32 > 0 && usingCount32 <= 64) ? (DWORD)(usingCount32 * sizeof(int)) : 0;
	if (dataSize32 > 0)
	{
		pStream->Read( rawData32, dataSize32 );
		if (dataSize32 < 256)
			pStream->Seek((256 - dataSize32), SEEK_CUR);
	}
	else
	{
		pStream->Seek(256, SEEK_CUR);
	}
#else
	const int usingCount = GetSignalDataUsingCount(m_nSignalIndex);
	DWORD dataSize = usingCount * sizeof(int);
	if (dataSize > 0 && dataSize <= 256)
	{
		m_pData = new int[usingCount];
		pStream->Read( m_pData, dataSize );
		pStream->Seek((256 - dataSize), SEEK_CUR);
	}
	else
	{
		pStream->Seek(256, SEEK_CUR);
	}
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
		for( int i=0; i<nTableCount; i++ ) {
			pStream->Read( &nTemp, sizeof(int) );
			m_nVec2Index[i] = nTemp;
		}
		pStream->Read( &vVec2List[0], sizeof(EtVector2) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		vVec3List.resize( nTableCount );
		m_nVec3Index.resize( nTableCount );
		for( int i=0; i<nTableCount; i++ ) {
			pStream->Read( &nTemp, sizeof(int) );
			m_nVec3Index[i] = nTemp;
		}
		pStream->Read( &vVec3List[0], sizeof(EtVector3) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		vVec4List.resize( nTableCount );
		m_nVec4Index.resize( nTableCount );
		for( int i=0; i<nTableCount; i++ ) {
			pStream->Read( &nTemp, sizeof(int) );
			m_nVec4Index[i] = nTemp;
		}
		pStream->Read( &vVec4List[0], sizeof(EtVector4) * nTableCount );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		m_nVecStrIndex.resize( nTableCount );
		for( int i=0; i<nTableCount; i++ ) {
			pStream->Read( &nTemp, sizeof(int) );
			m_nVecStrIndex[i] = nTemp;
		}
	}
	for( int i=0; i<nTableCount; i++ ) {
		std::string szStr;
		ReadStdString( szStr, pStream );
		szVecList.push_back( szStr );
	}
#ifdef WIN64
	// For 64-bit, expand every schema pointer slot (including runtime pointers like
	// SoundStruct::nVecSoundIndex and pointer fields omitted in older .act files)
	// as well as serialized pointer table slots from 4 bytes to 8 bytes.
	m_nVec2Index64.clear();
	m_nVec3Index64.clear();
	m_nVec4Index64.clear();
	m_nVecStrIndex64.clear();

#ifdef _TEST_DEFAULT_SIGNAL_ALLOC
	int slotMap[64];
	int nTargetOffset = 0;
	for( int i=0; i<64; i++ ) {
		slotMap[i] = nTargetOffset;
		if( IsSignalPointerSlot32(m_nSignalIndex, i) || IsPointerTable(i) != -1 )
			nTargetOffset += 2;
		else
			nTargetOffset += 1;
	}
	int *pTempBuf = new int[nTargetOffset];
	memset( pTempBuf, 0, nTargetOffset * sizeof(int) );
	for( int i=0; i<64; i++ ) {
		if( !IsSignalPointerSlot32(m_nSignalIndex, i) && IsPointerTable(i) == -1 ) {
			pTempBuf[slotMap[i]] = ((int*)m_pData)[i];
		}
	}
	for( DWORD i=0; i<m_nVec2Index.size(); i++ ) if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] < 64 ) m_nVec2Index[i] = (short)slotMap[m_nVec2Index[i]];
	for( DWORD i=0; i<m_nVec3Index.size(); i++ ) if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] < 64 ) m_nVec3Index[i] = (short)slotMap[m_nVec3Index[i]];
	for( DWORD i=0; i<m_nVec4Index.size(); i++ ) if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] < 64 ) m_nVec4Index[i] = (short)slotMap[m_nVec4Index[i]];
	for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] < 64 ) m_nVecStrIndex[i] = (short)slotMap[m_nVecStrIndex[i]];
	SAFE_DELETEA( m_pData );
	m_pData = pTempBuf;
#else
	int nAllocCount = usingCount;
	if (usingCount32 > 0 && usingCount32 <= 64)
	{
		int slotMap[64];
		int nTargetOffset = 0;
		for( int i=0; i<usingCount32; i++ ) {
			slotMap[i] = nTargetOffset;
			if( IsSignalPointerSlot32(m_nSignalIndex, i) || IsPointerTable(i) != -1 )
				nTargetOffset += 2;
			else
				nTargetOffset += 1;
		}

		if( nTargetOffset > nAllocCount )
			nAllocCount = nTargetOffset;

		int *pTempBuf = new int[nAllocCount];
		memset( pTempBuf, 0, nAllocCount * sizeof(int) );
		for( int i=0; i<usingCount32; i++ ) {
			if( !IsSignalPointerSlot32(m_nSignalIndex, i) && IsPointerTable(i) == -1 ) {
				pTempBuf[slotMap[i]] = rawData32[i];
			}
		}

		// Remap in-range pointer table slots to 64-bit offsets and discard out-of-range trailing slots.
		{
			std::vector<short> keptIdx; std::vector<EtVector2> keptVal;
			for( DWORD i=0; i<m_nVec2Index.size(); i++ ) {
				if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] < usingCount32 ) {
					keptIdx.push_back( (short)slotMap[m_nVec2Index[i]] );
					keptVal.push_back( vVec2List[i] );
				}
			}
			m_nVec2Index.swap( keptIdx ); vVec2List.swap( keptVal );
		}
		{
			std::vector<short> keptIdx; std::vector<EtVector3> keptVal;
			for( DWORD i=0; i<m_nVec3Index.size(); i++ ) {
				if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] < usingCount32 ) {
					keptIdx.push_back( (short)slotMap[m_nVec3Index[i]] );
					keptVal.push_back( vVec3List[i] );
				}
			}
			m_nVec3Index.swap( keptIdx ); vVec3List.swap( keptVal );
		}
		{
			std::vector<short> keptIdx; std::vector<EtVector4> keptVal;
			for( DWORD i=0; i<m_nVec4Index.size(); i++ ) {
				if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] < usingCount32 ) {
					keptIdx.push_back( (short)slotMap[m_nVec4Index[i]] );
					keptVal.push_back( vVec4List[i] );
				}
			}
			m_nVec4Index.swap( keptIdx ); vVec4List.swap( keptVal );
		}
		{
			std::vector<short> keptIdx; std::vector<std::string> keptVal;
			for( DWORD i=0; i<m_nVecStrIndex.size(); i++ ) {
				if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] < usingCount32 ) {
					keptIdx.push_back( (short)slotMap[m_nVecStrIndex[i]] );
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
#endif // _TEST_DEFAULT_SIGNAL_ALLOC
#endif

	////////////////////
#ifdef WIN64
	const int nWriteLimit = nAllocCount;
#else
	const int nWriteLimit = usingCount;
#endif
	if( m_pData != NULL ) {
		nTableCount = (int)m_nVec2Index.size();
		for( int i=0; i<nTableCount; i++ ) {
			if( m_nVec2Index[i] >= 0 && m_nVec2Index[i] + (int)(sizeof(int*) / sizeof(int)) <= nWriteLimit )
			{
				EtVector2 *pVec = new EtVector2;
				*pVec = vVec2List[i];
				memcpy( (int*)m_pData + ( m_nVec2Index[i] ), &pVec, sizeof(int*) );
			}
		}
		nTableCount = (int)m_nVec3Index.size();
		for( int i=0; i<nTableCount; i++ ) {		
			if( m_nVec3Index[i] >= 0 && m_nVec3Index[i] + (int)(sizeof(int*) / sizeof(int)) <= nWriteLimit )
			{
				EtVector3 *pVec = new EtVector3;
				*pVec = vVec3List[i];
				memcpy( (int*)m_pData + ( m_nVec3Index[i] ), &pVec, sizeof(int*) );
			}
		}
		nTableCount = (int)m_nVec4Index.size();
		for( int i=0; i<nTableCount; i++ ) {
			if( m_nVec4Index[i] >= 0 && m_nVec4Index[i] + (int)(sizeof(int*) / sizeof(int)) <= nWriteLimit )
			{
				EtVector4 *pVec = new EtVector4;
				*pVec = vVec4List[i];
				memcpy( (int*)m_pData + ( m_nVec4Index[i] ), &pVec, sizeof(int*) );
			}
		}
		
		nTableCount = (int)m_nVecStrIndex.size();
		for( int i=0; i<nTableCount; i++ ) {
			if( m_nVecStrIndex[i] >= 0 && m_nVecStrIndex[i] + (int)(sizeof(int*) / sizeof(int)) <= nWriteLimit )
			{
				char *pStr = new char[szVecList[i].size()+1];
				memset( pStr, 0, szVecList[i].size()+1 );
				strcpy_s( pStr, szVecList[i].size()+1, szVecList[i].c_str() );
				memcpy( (int*)m_pData + ( m_nVecStrIndex[i] ), &pStr, sizeof(int*) );
			}
		}
	}

	SAFE_DELETE_VEC( vVec2List );
	SAFE_DELETE_VEC( vVec3List );
	SAFE_DELETE_VEC( vVec4List );
	SAFE_DELETE_VEC( szVecList );

	return true;
}

bool CEtActionSignal::SkipSignal( CStream *pStream )
{
	pStream->Seek( 256 + sizeof(int) * 4, SEEK_CUR );

	int nTableCount;
	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		pStream->Seek( (sizeof(EtVector2)+sizeof(int)) * nTableCount, SEEK_CUR );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		pStream->Seek( (sizeof(EtVector3)+sizeof(int)) * nTableCount, SEEK_CUR );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		pStream->Seek( (sizeof(EtVector4)+sizeof(int)) * nTableCount, SEEK_CUR );
	}

	pStream->Read( &nTableCount, sizeof(int) );
	if( nTableCount > 0 ) {
		pStream->Seek( sizeof(int) * nTableCount, SEEK_CUR );
	}

	for( int i=0; i<nTableCount; i++ ) {
		char szBuf[4096] = {0,};
		int nSize;		
		pStream->Read( &nSize, sizeof(int) );
		if( nSize ) {
			if( nSize >= 4096 ) nSize = 4095;
			pStream->Seek( nSize, SEEK_CUR );
		}
	}
	return true;
}

void CEtActionSignal::InsertVec2Table( EtVector2 *pPtr, EtVector2 &vVec )
{
	if( !m_pData ) return;
#ifdef WIN64
	const int nMaxSlot = GetSignalDataUsingCount64(m_nSignalIndex) - (int)(sizeof(int*) / sizeof(int));
#else
	const int nMaxSlot = GetSignalDataUsingCount(m_nSignalIndex) - 1;
#endif
	for( int i=0; i<=nMaxSlot; i++ ) {
		if( (int*)(m_pData) + i == (int*)(pPtr) ) {
			void *pExisting = NULL;
			memcpy( &pExisting, (int*)(m_pData) + i, sizeof(void*) );
			if( pExisting != NULL ) break;
			EtVector2 *pVec = new EtVector2;
			*pVec = vVec;
			m_nVec2Index.push_back((short)i);
			memcpy( (int*)m_pData + i, &pVec, sizeof(int*) );
			break;
		}
	}
}

void CEtActionSignal::InsertVec3Table( EtVector3 *pPtr, EtVector3 &vVec )
{
	if( !m_pData ) return;
#ifdef WIN64
	const int nMaxSlot = GetSignalDataUsingCount64(m_nSignalIndex) - (int)(sizeof(int*) / sizeof(int));
#else
	const int nMaxSlot = GetSignalDataUsingCount(m_nSignalIndex) - 1;
#endif
	for( int i=0; i<=nMaxSlot; i++ ) {
		if( (int*)(m_pData) + i == (int*)(pPtr) ) {
			void *pExisting = NULL;
			memcpy( &pExisting, (int*)(m_pData) + i, sizeof(void*) );
			if( pExisting != NULL ) break;
			EtVector3 *pVec = new EtVector3;
			*pVec = vVec;
			m_nVec3Index.push_back((short)i);
			memcpy( (int*)m_pData + i, &pVec, sizeof(int*) );
			break;
		}
	}
}

void CEtActionSignal::InsertVec4Table( EtVector4 *pPtr, EtVector4 &vVec )
{
	if( !m_pData ) return;
#ifdef WIN64
	const int nMaxSlot = GetSignalDataUsingCount64(m_nSignalIndex) - (int)(sizeof(int*) / sizeof(int));
#else
	const int nMaxSlot = GetSignalDataUsingCount(m_nSignalIndex) - 1;
#endif
	for( int i=0; i<=nMaxSlot; i++ ) {
		if( (int*)(m_pData) + i == (int*)(pPtr) ) {
			void *pExisting = NULL;
			memcpy( &pExisting, (int*)(m_pData) + i, sizeof(void*) );
			if( pExisting != NULL ) break;
			EtVector4 *pVec = new EtVector4;
			*pVec = vVec;
			m_nVec4Index.push_back((short)i);
			memcpy( (int*)m_pData + i, &pVec, sizeof(int*) );
			break;
		}
	}
}

void CEtActionSignal::InsertStrTable( char *pPtr, std::string &szStr )
{
	if( !m_pData ) return;
#ifdef WIN64
	const int nMaxSlot = GetSignalDataUsingCount64(m_nSignalIndex) - (int)(sizeof(int*) / sizeof(int));
#else
	const int nMaxSlot = GetSignalDataUsingCount(m_nSignalIndex) - 1;
#endif
	for( int i=0; i<=nMaxSlot; i++ ) {
		if( (int*)(m_pData) + i == (int*)(pPtr) ) {
			void *pExisting = NULL;
			memcpy( &pExisting, (int*)(m_pData) + i, sizeof(void*) );
			if( pExisting != NULL ) break;
			char *pStr = new char[szStr.size() + 1];
			memset( pStr, 0, szStr.size()+1 );
			strcpy_s( pStr, szStr.size()+1, szStr.c_str() );

			m_nVecStrIndex.push_back((short)i);
			memcpy( (int*)m_pData + i, &pStr, sizeof(int*) );
			break;
		}
	}
}

#ifdef WIN64
char CEtActionSignal::IsPointerTable( int nIndex )
{
	if( std::find( m_nVec2Index.begin(), m_nVec2Index.end(), nIndex ) != m_nVec2Index.end() ) return 0;
	if( std::find( m_nVec3Index.begin(), m_nVec3Index.end(), nIndex ) != m_nVec3Index.end() ) return 1;
	if( std::find( m_nVec4Index.begin(), m_nVec4Index.end(), nIndex ) != m_nVec4Index.end() ) return 2;
	if( std::find( m_nVecStrIndex.begin(), m_nVecStrIndex.end(), nIndex ) != m_nVecStrIndex.end() ) return 3;

	return -1;
}

void CEtActionSignal::IncreasePointerTableIndex( int nValue )
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