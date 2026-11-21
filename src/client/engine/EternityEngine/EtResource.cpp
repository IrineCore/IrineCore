#include "StdAfx.h"
#include "EtResource.h"
#include "EtEngine.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

DECL_SMART_PTR_STATIC( CEtResource, 6000 )

std::vector< CEtResource * > CEtResource::s_vecWaitDelete;
std::map< INT64, EtResourceHandle > CEtResource::s_mapResourceSearch;

CEtResource::CEtResource()
: CSmartPtrBase< CEtResource >()
{
	m_nRefCount = 0;
	m_ResourceType = RT_NONE;
	m_bDeleteImmediate = true;
}

CEtResource::~CEtResource()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	s_vecWaitDelete.erase( std::remove( s_vecWaitDelete.begin(), s_vecWaitDelete.end(), this ), s_vecWaitDelete.end() ); 
	RemoveResourceSearchMap();
}

void CEtResource::Delete()
{
	delete this;
}

int CEtResource::AddRef()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	if( m_nRefCount == 0 )
	{
		s_vecWaitDelete.erase( std::remove( s_vecWaitDelete.begin(), s_vecWaitDelete.end(), this ), s_vecWaitDelete.end() ); 
	}
	m_nRefCount++;
	ASSERT( m_nRefCount >= 1 );

	if( m_nRefCount % 2000 == 0 )	// If RefCount keeps growing, it may not have been deleted, so check it.
	{	
		if( strstr( m_szFileName.c_str(), ".fx" ) == NULL )
		{
			OutputDebug("[%s] File Refcount is %d\n", m_szFileName.c_str(), m_nRefCount );
		}
	}

	return m_nRefCount;
} 

int CEtResource::Release()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	bool bDelete = false;
	m_nRefCount--;
	if( m_nRefCount < 0 )
	{
		m_nRefCount = 0;
	}
	if( m_nRefCount <= 0 )
	{
		if( m_bDeleteImmediate )
		{
			bDelete = true;
		}
		else 
		{
			if( std::find( s_vecWaitDelete.begin(), s_vecWaitDelete.end(), this ) == s_vecWaitDelete.end() )
			{
				s_vecWaitDelete.push_back( this );
			}
		}
	}

	// Back up m_nRefCount since direct deletion below will destroy it.
	int nRefCount = m_nRefCount;
	if( bDelete )
	{
		delete this;
	}
	return nRefCount;
}

void CEtResource::AddResourceSearchMap()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	INT64 ulHashCode = GetHashCode64( GetFileName() );
	std::map< INT64, EtResourceHandle >::iterator it = s_mapResourceSearch.find( ulHashCode );
	while( it != s_mapResourceSearch.end() )
	{
		ulHashCode++;
		it = s_mapResourceSearch.find( ulHashCode );
	}
	s_mapResourceSearch.insert(  std::make_pair( ulHashCode, GetMySmartPtr() ) );
}

void CEtResource::RemoveResourceSearchMap()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	if( strlen( GetFileName() ) == 0 )
	{
		return;
	}
	bool bErase = false;
	INT64 ulHashCode = GetHashCode64( GetFileName() );
	std::map< INT64, EtResourceHandle >::iterator it = s_mapResourceSearch.find( ulHashCode );
	while( it != s_mapResourceSearch.end() )
	{
		if( it->second == GetMySmartPtr() )
		{
			s_mapResourceSearch.erase( it );
			bErase = true;
			break;
		}
		ulHashCode++;
		it = s_mapResourceSearch.find( ulHashCode );
	}
	ASSERT( bErase && "Resource not found!!!" );
}

void CEtResource::FlushWaitDelete()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	while( s_vecWaitDelete.size() )
	{
		CEtResource *pDelResource = s_vecWaitDelete[ 0 ];
		s_vecWaitDelete.erase( s_vecWaitDelete.begin() );
		if( pDelResource )
		{
			delete pDelResource;
		}
	}
}

void CEtResource::FlushWaitDelete( int nResourceType )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	for( DWORD i=0; i<s_vecWaitDelete.size(); i++ ) {
		CEtResource *pDelResource = s_vecWaitDelete[i];
		if( !pDelResource ) continue;
		if( pDelResource->GetResourceType() != nResourceType ) continue;
		s_vecWaitDelete.erase( s_vecWaitDelete.begin() + i );
		delete pDelResource;
		i--;
	}
}

void CEtResource::FlushWaitDeleteByCount(int nCount)
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	if (nCount > (int)s_vecWaitDelete.size())
		nCount = (int)s_vecWaitDelete.size();

	DWORD i = 0;
	for (; i < nCount; ++i)
	{
		CEtResource *pDelResource = s_vecWaitDelete[ 0 ];
		s_vecWaitDelete.erase( s_vecWaitDelete.begin() );
		if( pDelResource )
		{
			delete pDelResource;
		}
	}
}

int CEtResource::Load( const char *pFileName ) 
{
	CResMngStream *pStream = new CResMngStream( pFileName );
	return Load( pStream );
}

bool CEtResource::CheckOutOfMemory( float fRemainRatio, DWORD dwTermTick )
{
	ASSERT( fRemainRatio >= 0.0f && fRemainRatio <= 1.0f );

	float fMemoryUsePercent = GetEtDevice()->GetMemoryUsePercent();
	if( fMemoryUsePercent > ( 1.0f - fRemainRatio ) )
	{
		return true;
	}

	MEMORYSTATUS MemStatus;
	GlobalMemoryStatus( &MemStatus );
	if( MemStatus.dwAvailPhys / ( float )MemStatus.dwTotalPhys < fRemainRatio )
	{
		return true;
	}
	if( MemStatus.dwAvailVirtual / ( float )MemStatus.dwTotalVirtual < fRemainRatio )
	{
		return true;
	}

	return false;
}

void CEtResource::CheckResourceMemory()
{
	if( !CheckOutOfMemory( 0.1f ) )
	{
		return;		
	}

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	while( s_vecWaitDelete.size() )
	{
		CEtResource *pDelResource = s_vecWaitDelete[ 0 ];
		s_vecWaitDelete.erase( s_vecWaitDelete.begin() );
		if( pDelResource )
		{
			delete pDelResource;
		}
		if( !CheckOutOfMemory( 0.5f ) )				// Clear generously.. keep memory usage under 50%..
		{
			break;
		}
	}
}

int CEtResource::Load( CStream *pStream )
{
	if( pStream == NULL ) return ETERR_INVALIDRESOURCESTREAM;

	CheckResourceMemory();

	int nRet = LoadResource( pStream );
	if( ( nRet == ET_OK ) && ( pStream->GetName() ) )
	{
		m_szFullName = pStream->GetName();
		m_szFileName = FindFileName( pStream->GetName() );		
	}
	SAFE_DELETE( pStream );
	if( nRet != ET_OK )
	{
		return nRet;
	}

	ASSERT( !m_szFileName.empty() );
	std::transform( m_szFileName.begin(), m_szFileName.end(), m_szFileName.begin(), tolower );

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	bool bAddSearchMap = ( GetRefCount() == 0 ) ? true : false;
	AddRef();
	if( bAddSearchMap ) {
		AddResourceSearchMap();
	}

	return ET_OK; 
}

CSmartPtr< CEtResource > CEtResource::GetResource( const char *pFileName, bool bAddRef )
{
	CFileNameString szTempName = FindFileName( pFileName );
	std::transform( szTempName.begin(), szTempName.end(), szTempName.begin(), tolower ); 

	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );

	char szName[ _MAX_PATH ];
	_GetFullFileName( szName, _countof(szName), pFileName );
	_strlwr( szName );
	TrimString( szName );

	INT64 ulHashCode = GetHashCode64( szTempName.c_str() );
	std::map< INT64, EtResourceHandle >::iterator it = s_mapResourceSearch.find( ulHashCode );
	while( it != s_mapResourceSearch.end() )
	{
		EtResourceHandle hFindResource = it->second;
		if( hFindResource )
		{
			if( _stricmp( szTempName.c_str(), hFindResource->GetFileName() ) == 0 )
			{
				if( bAddRef ) hFindResource->AddRef();
				return hFindResource;
			}
		}
		else s_mapResourceSearch.erase( it );

		ulHashCode++;
		it = s_mapResourceSearch.find( ulHashCode );
	}

	return CEtResource::Identity();
}