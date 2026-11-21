#include "stdafx.h"
#include "EtResource.h"

DECL_SMART_PTR_STATIC( CEtResource, 200 )

CSyncLock CEtResource::s_SyncLock;
std::map< std::string, EtResourceHandle > CEtResource::s_ResourceMap;
bool CEtResource::s_bUseDynamic = true;

CEtResource::CEtResource( CMultiRoom *pRoom )
: CSmartPtrBase< CEtResource >( CEtResource::s_bUseDynamic )
{
	m_pBaseRoom = pRoom;
	m_nRefCount = 0;
	m_ResourceType = RT_NONE;
}

CEtResource::~CEtResource()
{
	ScopeLock<CSyncLock> Lock(CEtResource::s_SyncLock, s_bUseDynamic);
	s_ResourceMap.erase( m_szFileName );
}

int CEtResource::Release()
{
	int nRefCount = m_nRefCount;
	if( s_bUseDynamic ) {
		m_nRefCount--;
		nRefCount--;
		if( m_nRefCount <= 0 )
		{
			ScopeLock<CSyncLock> Lock(CEtResource::s_SyncLock, s_bUseDynamic);
			CSmartPtrBase< CEtResource >::Release();
		}
	}
	return nRefCount;
}

bool CEtResource::Initialize( bool bUseDynamic )
{
	s_bUseDynamic = bUseDynamic;
	return true;
}

int CEtResource::Load( const char *pFileName ) 
{ 
	int nRet;
	CFileStream Stream( pFileName );
	if( !Stream.IsValid() ) return ETERR_INVALIDRESOURCESTREAM;

	nRet = LoadResource( &Stream );

	if( nRet != ET_OK )
	{
		return nRet;
	}

	m_szFullName = pFileName;
	m_szFileName = FindFileName( pFileName );
	std::transform(m_szFileName.begin(), m_szFileName.end(), m_szFileName.begin(), tolower); 

	ScopeLock<CSyncLock> Lock(CEtResource::s_SyncLock, s_bUseDynamic);
	s_ResourceMap[ m_szFileName ] = GetMySmartPtr();

	return ET_OK; 
}

EtResourceHandle CEtResource::GetResource( CMultiRoom *pRoom, const char *pFileName )
{
	ScopeLock<CSyncLock> Lock(CEtResource::s_SyncLock, s_bUseDynamic);
	
	EtResourceHandle hResource;
	std::string szTempName = FindFileName( pFileName );
	std::transform(szTempName.begin(), szTempName.end(), szTempName.begin(), tolower); 

	std::map< std::string, EtResourceHandle >::iterator it = s_ResourceMap.find( szTempName );
	if( it != s_ResourceMap.end() ) {
		hResource = it->second;
		if( hResource ) {
			return hResource;
		}
	}

	return Identity();
}

CSmartPtr< CEtResource > CEtResource::GetResource( CMultiRoom *pRoom, int nIndex ) 
{ 
//	ScopeLock<CSyncLock> Lock(CEtResource::s_SyncLock, s_bUseDynamic);
	return GetSmartPtr( nIndex ); 
}