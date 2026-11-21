#pragma once

#include "Singleton.h"
#include "CriticalSection.h"
#include "Stream.h"
#include "StringUtil.h"
#include <map>

class CEtFileSystem;
class CEtResourceMng : public CSingleton<CEtResourceMng> {
public:
	CEtResourceMng( bool bUsePackingFile = false, bool bUseDynamic = true );
	virtual ~CEtResourceMng();

protected:
	struct OffsetStruct {
		std::vector<CFileNameString> szVecFileList;
	};
	std::vector<CFileNameString> m_szVecResourcePath;
	std::vector<OffsetStruct> m_VecPathCacheFileOffset;

	std::map<CFileNameString, CFileNameString> m_szMapAccessSuccessFileName;
	std::map<CFileNameString, CFileNameString> m_szMapAccessFailedFileName;

	std::map<CFileNameString, CFileNameString> m_szMapCacheFileList;
	std::map<CFileNameString, int> m_szMapCacheFolderList;

	struct CacheMemoryBuffer {
		char *pPtr;
		int nSize;
	};
	std::map<CFileNameString, CacheMemoryBuffer *> m_szMapCacheMemoryBufferList;

	UINT m_nCodePage;
	std::map<int, CFileNameString> m_mapCodePageString;
	CSyncLock m_SyncLock;

	CEtFileSystem *m_pFileSystem;
	bool m_bUsePackingFile;
	bool m_bUseDynamic;
	std::string m_strLanguage;

	// Store only the root folder separately as WCHAR.
	std::wstring m_wszRootDirectory;
	bool m_bUseMapAccessSuccessFileName;

protected:
	bool IsExistFile( const char *szFileName );
	void CacheFileList( int nIndex, bool bAdd );

	bool _AddResourcePath( const char *szPath, bool bIncludeSubDir = false );
	void _RemoveResourcePath( const char *szPath, bool bIncludeSubDir = false );

	void ClearCacheMemoryStream();

public:
	const char *FindCodePageString( int nCodePage );
	bool IsCodePageFolder( const char *szCodePageStr );
	bool IsUsePackingFile() { return m_bUsePackingFile; }
	bool IsUseDynamic() { return m_bUseDynamic; }

	bool AddResourcePath( const char *szPath, bool bIncludeSubDir = false );
	void RemoveResourcePath( const char *szPath, bool bIncludeSubDir = false );
	bool AddResourcePathByCodePage( const char *szPath, int nCodePage = -1, bool bIncludeSubDir = false );

	bool AddResourcePath( CFileNameString &szPath, bool bIncludeSubDir = false );
	void RemoveResourcePath( CFileNameString &szPath, bool bIncludeSubDir = false );
	bool AddResourcePathByCodePage( CFileNameString &szPath, int nCodePage = -1, bool bIncludeSubDir = false );
	bool AddResourcePath( std::string &szPath, bool bIncludeSubDir = false );
	void RemoveResourcePath( std::string &szPath, bool bIncludeSubDir = false );
	bool AddResourcePathByCodePage( std::string &szPath, int nCodePage = -1, bool bIncludeSubDir = false );

	void SetPackingFolder( const char *szPath );
	void SetPackingFolder( const WCHAR *szPath );	// Receive as WCHAR when getting the initial root folder.
	void FindFileListInFolder( const char *szFolderName, const char *szExt, std::vector<CFileNameString> &szVecResult);
	// Get file list from the global manager matching the folder name and filtered by szExt.
	// However, if file names are duplicated, subsequent duplicate files are not retrieved.
	// ex.) 
	// Resource/ext -> ItemTable.ext ItemTable_Cash.ext ItemTable_Equip.ext
	// Resource_chn/ext -> ItemTable_Cash.ext
	// Result -> Resource_chn/ext/ItemTable_Cash.ext, Resource/ext/ItemTable.ext, Resource/ext/ItemTable_Equip.ext
	void FindFileListAll_IgnoreExistFile( const char *szFolderName, const char *szExt, std::vector<CFileNameString> &szVecResult);

	CFileNameString GetFullName( CFileNameString &szFileName, bool *bFind = NULL );
	CFileNameString GetFullNameRandom( CFileNameString &szFileName, int nIndex, bool *bFind = NULL );
	CFileNameString GetFullPath( CFileNameString &szPath, bool bSearchSubDir = false );
	CStream *GetStreamRandom( CFileNameString &szFileName, int nIndex, bool *bFind = NULL );

	CFileNameString GetFullName( const char *szFileName, bool *bFind = NULL );
	CFileNameString GetFullName( std::string &szFileName, bool *bFind = NULL );
	CFileNameString GetFullNameRandom( std::string &szFileName, int nIndex, bool *bFind = NULL );
	CFileNameString GetFullNameInCacheList( const char *szFileName );
	CFileNameString GetFullPath( const char *szPath, bool bSearchSubDir = false );
	void FindFullPathList( const char *szPath, std::vector<CFileNameString> &szVecResult );
	CFileNameString GetCurrentDirectory() { return m_szVecResourcePath[0]; }
	std::wstring GetRootDirectoryW() { return m_wszRootDirectory; }

	CStream *GetStream( const char *szFileName, bool *bFind = NULL );
	CStream *GetStream( CFileNameString &szFileName, bool *bFind = NULL );

	bool AddCacheMemoryStream( const char *szFileName );
	bool RemoveCacheMemoryStream( const char *szFileName );
	CStream *GetCacheMemoryStream( const char *szFileName );

	void Initialize();
	void AddCodePage( int nCodePage, const char *szCodePageStr );
	void SetCodePage( int nCodePage ) { m_nCodePage = nCodePage; }
	UINT GetCodePage() { return m_nCodePage; }

	void UseMapAccessSuccessFileName( bool bUse ) { m_bUseMapAccessSuccessFileName = bUse; }

	void SetLanguage( std::string strLanguage ) { m_strLanguage = strLanguage; }
	CFileNameString ChangeFileNameByLanguage( CFileNameString strFullName );

	void ReserchAddResourcePath( const char* szPath );
};

class CEtResourceAccess
{
public:
	CEtResourceAccess();
	CEtResourceAccess( const char *pFileName, bool bSelfDelete = true, bool bIncludeSubPath = false );
	virtual ~CEtResourceAccess();

public:
	void Initialize( const char *pFileName, bool bSelfDelete = true, bool bIncludeSubPath = false );
	void Finalize();
	bool IsValid() { return m_bValid; }
	const char *GetFullName() { return m_szFullName.c_str(); }
	CStream *GetStream();
	const char *GetAddPath() { return m_szAddPath; }
	bool IsAddPath() { return m_bAddPath; }

protected:
	bool m_bAddPath;
	bool m_bIncludeSubPath;
	bool m_bValid;
	char m_szAddPath[_MAX_PATH];
	CFileNameString m_szFullName;
	CStream *m_pStream;
	bool m_bSelfDelete;
};

class CResMngStream : public CEtResourceAccess, public CStream
{
public:
	CResMngStream();
	CResMngStream( const char *pFileName, bool bSelfDelete = true, bool bIncludeSubPath = false );
	virtual ~CResMngStream();

protected:

public:
	bool Open( const char *pFileName, bool bSelfDelete = true, bool bIncludeSubPath = false );

	virtual bool IsEnd() const;
	virtual long Tell();
	virtual int Size() const;
	virtual bool IsValid() const;
	virtual void Close();
	virtual void Activate();

	virtual int Seek( long lOffset, int nOrigin );
	virtual int Read( void *pBuffer, int nSize );
	virtual int Write( const void *pBuffer, int nSize );

	virtual const char *GetName();
};