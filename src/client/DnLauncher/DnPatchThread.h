#pragma once

#include "EtFileSystem.h"
#include "HTTPClient.h"

// DnLauncher Download Patch Thread

#ifdef _USE_RTPATCH
#define  MAX_MESSAGES 10
#endif // _USE_RTPATCH

// CThread Class
class CThread
{
public:
	CThread();
	virtual ~CThread();

	BOOL Start();
	BOOL Terminate( DWORD nExitCode );
	void SetThreadName( DWORD dwThreadID, const char* szThreadName );
	BOOL WaitForTerminate( DWORD nTimeout = INFINITE );
	HANDLE const GetThreadHandle() { return m_hHandleThread; }

protected:
	virtual	void Run() = 0;
	HANDLE	m_hHandleThread;
	DWORD	m_nThreadID;
	BOOL	m_bThreadLoop;

	typedef struct tagTHREADNAME_INFO 
	{ 
		DWORD dwType;		// must be 0x1000 
		LPCSTR szName;		// pointer to name (in user addr space) 
		DWORD dwThreadID;	// thread ID (-1=caller thread) 
		DWORD dwFlags;		// reserved for future use, must be zero 
	} THREADNAME_INFO; 

private:
	static DWORD WINAPI _Runner( LPVOID pParam );
};

// CDnPatchDownloadThread Class
enum PatchReturn
{
	PR_FAIL = 0,
	PR_OK,
	PR_LAUNCHER_PATCH,
	PR_FULLPATCH,
	PR_TERMINATE,
};

enum FirstPatchReturn
{
	FPR_FAIL = 0 ,
	FPR_OK ,
	FPR_LAUNCHER_PATCH,
	FPR_NEED_FULLPATCH,

	FPR_MAX
};

#define WM_STATUS_MSG							WM_APP+101	// Patch status message

enum DownloadPatchState
{
	PATCH_NONE = 0,
	PATCH_CHECK_VERSION,			// Version check
	PATCH_CHECK_VERSION_FAILED,		// Version check failed
	PATCH_DOWNLOAD_DATA,			// Download patch data
	PATCH_DOWNLOAD_DATA_FAILED,		// Failed to download patch data
	PATCH_MD5_CHECK,				// Validate patch data
	PATCH_MD5_CHECK_FAILED,			// Patch data validation failed
	PATCH_APPLY_PAK_FILE,			// Install patch data
	PATCH_APPLY_PAK_FILE_FAILED,	// Failed to install patch data
	PATCH_APPLY_PAK_FILE_END,		// Patch data installation complete
	PATCH_COMPLETE,					// Patch complete
	PATCH_LAUNCHER,					// Launcher patch
	PATCH_FAILED,					// Patch failed
	PATCH_TERMINATE					// Cancel patch
};

struct stDownloadInfoUpdateData
{
	CString m_strFileName;
	__int64	m_nFileSize;
	__int64	m_nDownloadProgressMin;
	__int64	m_nDownloadProgressMax;
	int		m_nCurrentFileCount;
	int		m_nTotalFileCount;
	float	m_fDownloadRate;

	stDownloadInfoUpdateData() : m_nFileSize( 0 ), m_nDownloadProgressMin( 0 ), m_nDownloadProgressMax( 0 ), 
		m_nCurrentFileCount( 0 ), m_nTotalFileCount( 0 ), m_fDownloadRate( 0.0f ) {}
};

enum FileCheck{ FILE_FALSE = 0, FIlE_OK, FIlE_EXIST };

#ifdef _USE_SPLIT_COMPRESS_FILE
struct RARHeaderData
{
	char         ArcName[260];
	char         FileName[260];
	unsigned int Flags;
	unsigned int PackSize;
	unsigned int UnpSize;
	unsigned int HostOS;
	unsigned int FileCRC;
	unsigned int FileTime;
	unsigned int UnpVer;
	unsigned int Method;
	unsigned int FileAttr;
	char         *CmtBuf;
	unsigned int CmtBufSize;
	unsigned int CmtSize;
	unsigned int CmtState;
};

struct RAROpenArchiveDataEx
{
	char         *ArcName;
	wchar_t      *ArcNameW;
	unsigned int OpenMode;
	unsigned int OpenResult;
	char         *CmtBuf;
	unsigned int CmtBufSize;
	unsigned int CmtSize;
	unsigned int CmtState;
	unsigned int Flags;
	unsigned int Reserved[32];
};
#endif // _USE_SPLIT_COMPRESS_FILE



class CDnFistPatchDownloadThread : public CPatchCallback, public CThread
{
public:
	CDnFistPatchDownloadThread();
	virtual ~CDnFistPatchDownloadThread();
	
	virtual void Run();
	void		SetWindowHandle( HWND hWnd ){ m_hWnd = hWnd; }
	void		TerminateThread()			{ m_bTerminateThread = true; }
	BOOL		IsThreadEnd()				{ return m_bTerminateThread; }

	FirstPatchReturn BegineModulePatch();									// Start FirstPatch (module & launcher files).
	int			ParsingModuleList();										// Parse information required for module patch.
	int			ChangeModuleFiles(int nVer);								// Apply module file.
	BOOL		DownLoadModulePatch( int nVersion );						// Download module file.
	BOOL		PatchLauncherFiles( int nVersion );							// Download & apply launcher.
	BOOL		ProcessFullVersionPatch();									// Full version patch process
	void		ProcessRebootLauncher();									// Restart launcher.
	BOOL		SaveModuleVersionToPak( int nVersion );						// Save Version.cfg to the Pak file.
	BOOL		SaveModuleVersionFile( CString& szFilePath, int nVersion ); // Save Version.cfg.
	BOOL		SaveModuleVersionFileWithOutPakSave( CString& szFilePath, int nVersion );
	void		CloseThread();

private:
	HWND		m_hWnd;								// Window handle
	BOOL		m_bTerminateThread;					// Thread termination flag
	CString		m_strFirstPatchURLPath;				// Save PatchURL/FirstPatch/ path.
	CString		m_strPatchListFile;					// Client patch path
	std::vector<std::string>	m_vecCopyList;		// Copy file list
	std::vector<std::string>	m_vecDeleteList;	// Delete file list
};



class CDnPatchDownloadThread : public CPatchCallback, public CThread
{
public:
	CDnPatchDownloadThread();
	virtual ~CDnPatchDownloadThread();
	void SetWindowHandle( HWND hWnd ) { m_hWnd = hWnd; }

	virtual void Run();
	void TerminateThread() { m_bTerminateThread = true; }

private:
	PatchReturn BeginPatch();				// Patch processing function

	// Download
	BOOL DownLoadPatch( int nVersion );						// Download process

#ifdef _USE_SPLIT_COMPRESS_FILE
	HRESULT URLDownload( LPCTSTR strPatchFileUrl, LPCTSTR strFilePath, LPCTSTR strFileName, 
						BOOL bSplitDownload = FALSE, DWORD dwAlreadyDownloadSize = 0, DWORD dwRemainDownloadFileSize = 0 );		// Actual resume download process
	FileCheck CheckPatchFile( HINTERNET hInternetSession, LPCTSTR strFileName,
							CFile& file, DWORD& dwDownloadFileSize, BOOL bSplitDownload = FALSE );								// Check file before download
#else // _USE_SPLIT_COMPRESS_FILE
	HRESULT URLDownload( LPCTSTR strPatchFileUrl, LPCTSTR strFilePath, LPCTSTR strFileName );	// Actual resume download process
	FileCheck CheckPatchFile( HINTERNET hInternetSession, LPCTSTR strFileName,
							CFile& file, DWORD& dwDownloadFileSize );							// Check file before download
#endif // _USE_SPLIT_COMPRESS_FILE

	DWORD GetDownloadFileSize( HINTERNET hInternetSession, CString strHttpPath );				// Function to get size of file to download
	BOOL GetMD5Checksum();																		// Validate downloaded file (MD5)

	// Apply Packing File
	PatchReturn PreApplyPakPatch();									// Preparation before applying Pak file (set patch list, retrieve packing file)
	PatchReturn ApplyPakPatch();									// Pak file application process
	PatchReturn ApplyDeleteList( CEtPackingFile& PatchSource );		// Apply delete list Pak file
	PatchReturn ApplyPatchList( CEtPackingFile& PatchSource );		// Apply patch list Pak file
	PatchReturn ApplyCopyList( CEtPackingFile& PatchSource );		// Apply copy list Pak file
	PatchReturn GetPackingFile();									// Find resource pack file and get info
	BOOL LoadPatchList();											// Read patch list and set patch info
	PatchReturn PatchFileExist( int nPatchFileCount, CEtPackingFile& PatchSource, SPackingFileInfo* pFileInfo, BOOL& bPatch );
	PatchReturn PatchFileSuitableSpace( CEtPackingFile& PatchSource, SPackingFileInfo* pFileInfo, BOOL& bPatch );
	void PatchFileReserve( int nPatchFileCount, SPackingFileInfo* pFileInfo );

	void SendStatusMsg( DownloadPatchState nID );				// Pass download status value to dialog

	BOOL ProcessFullVersionPatch();								// Full version patch process
	BOOL SaveVersionFile( CString& szFilePath, int nVersion );	// Update Version.cfg file after patch completes
	BOOL SaveNewVersionToPak( int nVersion );					// Put Version.cfg file inside Pak file

	void RecordFileExceptionLog( CFileException& e );

#ifdef _USE_RTPATCH
	BOOL ExportFile( CEtPackingFile& PakFile, const char* pExportFileName, const char* pCreateFilePath );	// Extract files inside pak file
	PatchReturn ApplyRTPatch( CEtPackingFile& PatchSource, CEtPackingFile& RTPatchSource );					// Convert existing pak file into RTPatch-applied pak file
	static LPVOID CALLBACK EXPORT RTPatchCALLBACK( UINT Id, LPVOID Param );
#endif // _USE_RTPATCH
#ifdef _FIRST_PATCH
	BOOL CheckExistFirstPatch( int nVersion );
#endif // _FIRST_PATCH

#ifdef _USE_SPLIT_COMPRESS_FILE
	HRESULT DownLoadSplitFile( CString strUrl, int nVersion );
	DWORD GetSplitFileSizeList( CString strUrl, int nVersion, int nSplitCount, int nSplitCountCipher, std::vector<DWORD>& vecSplitFileSizeList );
	int LoadSplitCount( CString& szFilePath );
	HRESULT UnRARArchive( CString strPath, CString strRARFile );
#endif // _USE_SPLIT_COMPRESS_FILE

private:
	HINTERNET	m_hHttpConnection;		// Http Connection Handle

	DWORD		m_dwTempFileLength;		// Amount of downloaded files
	DWORD		m_dwDownloadFileLength;	// Amount of files to download
	DWORD		m_dwTotalFileLength;	// Total file size
	DWORD		m_dwTotalBytes;			// Total amount of files received

	CString		m_strPatchFileUrl;		// Http patch file URL
	CString		m_strPatchFileLocal;	// Local file path (when download completes: .tmp -> .pak)
	CString		m_strPatchFile;			// Patch file (pak file)
	CString		m_strPatchListFile;		// Patch list file

	float		m_fRate;				// Download Rate
	float		m_fPercent;				// Download Percent

	DownloadPatchState m_emDownPatchState;	// Current download progress
	
	HWND		m_hWnd;					// Window handle
	BOOL		m_bTerminateThread;		// Thread termination flag

	std::vector<CEtPackingFile*>	m_vecPackingFile;		// Packed file list
	std::vector<std::string>		m_vecDeleteList;		// Delete file list
	std::vector<std::string>		m_vecPatchList;			// Patch file list
	std::vector<std::string>		m_vecCopyList;			// Copy file list
	struct stFileIndexSet
	{
		DWORD m_dwPakFileNum;
		DWORD m_dwPatchFileIndex;
		DWORD m_dwReserveFileIndex;
	};

	std::vector<stFileIndexSet>		m_vecReserveIndex;		// Reserved patch file index


	int			m_nCurPatchCount;		// Current patch count (used for progress bar info when packing files)
	int			m_nTotalPatchCount;		// Total patch count (used for progress bar info during file packing)

#ifdef _USE_RTPATCH
	std::string	m_strRTPatchTempFolder;		// RTPatch temporary folder
#endif // _USE_RTPATCH

#ifdef _USE_SPLIT_COMPRESS_FILE
	HINSTANCE	hUnrarDLL;					// UnrarDLL handle

	HANDLE (WINAPI *OpenArchiveEx)( RAROpenArchiveDataEx* pArchiveData );
	int    (WINAPI *CloseArchive)( HANDLE hArcData );
	int    (WINAPI *ReadRARHeader)( HANDLE hArcData, RARHeaderData *pHeaderData );
	int    (WINAPI *ProcessRARFile)( HANDLE hArcData, int iOperation, char* strDestFolder, char* strDestName );
#endif // _USE_SPLIT_COMPRESS_FILE
};
