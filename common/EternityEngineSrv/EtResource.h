#pragma once

#include "MultiCommon.h"
#include "MultiSmartPtr.h"
#include "CriticalSection.h"
class CEtResource;
typedef CSmartPtr< CEtResource > EtResourceHandle;

class CEtResource : public CSmartPtrBase< CEtResource >
{
public:
	CEtResource( CMultiRoom *pRoom );
	virtual ~CEtResource();

protected:
	int m_nRefCount;
	int m_ResourceType;
	std::string m_szFullName;
	std::string m_szFileName;

	CMultiRoom *m_pBaseRoom;
	static bool s_bUseDynamic;
	static CSyncLock s_SyncLock;
	static std::map< std::string, EtResourceHandle > s_ResourceMap;

public:
	int AddRef() { m_nRefCount++; return m_nRefCount; }
	int Release();
	const char *GetFullName() { return m_szFullName.c_str(); }
	const char *GetFileName() { return m_szFileName.c_str(); }
	void SetFileName( const char *pFileName ) { m_szFileName = pFileName; }

	void SetResourceType( int Type ) { m_ResourceType = Type; }
	int GetResourceType() { return m_ResourceType; }

	int Load( const char *pFileName );
	virtual int LoadResource( CStream *pStream ) = 0;

	CMultiRoom *GetRoom() { return m_pBaseRoom; }
	static CSmartPtr< CEtResource > GetResource( CMultiRoom *pRoom, const char *pFileName );
	static CSmartPtr< CEtResource > GetResource( CMultiRoom *pRoom, int nIndex );

	static bool Initialize( bool bUseDynamic = true );
};
