

///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileSet.h"
#include "CommonMacros.h"
#include "BaseSet.h"
#include "DebugUtil.h"
#include "SafeStringMacros.h"

///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

#pragma warning (disable:4238)
CFileSet::FILEBOMTYPEDAT CFileSet::m_stBomTypeDat[EV_BT_NONE] = {
	CFileSet::FILEBOMTYPEDAT(EV_BT_UTF8,		&CVarArgA<UCHAR_MAX>("%c%c%c",		0xEF,0xBB,0xBF),		3),
	CFileSet::FILEBOMTYPEDAT(EV_BT_UTF16_LE,	&CVarArgA<UCHAR_MAX>("%c%c",		0xFF,0xFE),				2),
	CFileSet::FILEBOMTYPEDAT(EV_BT_UTF16_BE,	&CVarArgA<UCHAR_MAX>("%c%c",		0xFE,0xFF),				2),
// 	CFileSet::FILEBOMTYPEDAT(EV_BT_UTF32_LE,	&CVarArgA<UCHAR_MAX>("%c%c%c%c",	0xFF,0xFE,0x00,0x00),	4),
// 	CFileSet::FILEBOMTYPEDAT(EV_BT_UTF32_BE,	&CVarArgA<UCHAR_MAX>("%c%c%c%c",	0x00,0x00,0xFE,0xFF),	4),
};
#pragma warning (default:4238)


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::CFileSet
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CFileSet::CFileSet()
{
	Reset();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::~CFileSet
//*---------------------------------------------------------------
// DESC : Destructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CFileSet::~CFileSet()
{
	Close();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Reset
//*---------------------------------------------------------------
// DESC : Initialize file-related resources
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
// P.S.>
//		- Note: Does not close the handle
//*---------------------------------------------------------------
VOID CFileSet::Reset()
{
	m_hFile = INVALID_HANDLE_VALUE;
	::memset(m_szFileFullPath, 0, sizeof(m_szFileFullPath));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Open
//*---------------------------------------------------------------
// DESC : Open file and allocate resources
// PARM : 1 . lpszFileName - Absolute/relative path including file name
//			2 . bIsReadOnly - whether read-only
//			3 . bIsAppend - Whether to append data to file (valid only when bIsReadOnly == FALSE)
//			4. bIsReadShare - Whether to allow read sharing
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::Open(LPCTSTR lpszFileName, BOOL bIsReadOnly, BOOL bIsAppend, BOOL bIsReadShare)
{
	DN_ASSERT(FALSE == IsOpen(),		"Invalid!");
	DN_ASSERT(NULL != lpszFileName,		"Invalid!");

	DWORD dwDesiredAccess;
	DWORD dwShareMode;
	DWORD dwCreationDisposition;
	DWORD dwFlagsAndAttributes;

	if (bIsReadOnly) {
		dwDesiredAccess = GENERIC_READ;
		dwShareMode = FILE_SHARE_READ;
		dwCreationDisposition = OPEN_EXISTING;
		dwFlagsAndAttributes = FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_READONLY;
		if (!bIsReadShare) { 
			dwShareMode |= FILE_SHARE_WRITE;
		}
	}
	else {
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = FILE_SHARE_READ;
		dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
		if (bIsAppend) {
			dwCreationDisposition = OPEN_ALWAYS;
		}
		else {
			dwCreationDisposition = CREATE_NEW;
		}
	}

	DWORD dwRetVal = Open(lpszFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes, NULL, NULL);
	if (NOERROR != dwRetVal) {
		return dwRetVal;
	}

	if (!bIsReadOnly) {
		if (bIsAppend) {
			dwRetVal = Seek(FILE_END, 0, 0);
		}
		else {
			if (FALSE == ::SetEndOfFile(m_hFile)) {
				dwRetVal = ::GetLastError();
			}
		}
	}
	
	return dwRetVal;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Open
//*---------------------------------------------------------------
// DESC : Open file and allocate resources
// PARM : 1 . lpszFileName - Absolute/relative path including file name
//			2 . dwDesiredAccess - access mode
//			3 . dwShareMode - Share mode
//			4. dwCreationDisposition - Creation disposition
//			5. dwFlagsAndAttributes - Flags and attributes (multiple allowed)
//			6. lpSecurityAttributes - Security attributes
//			7. hTemplateFile - Template file handle
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::Open(
  LPCTSTR lpszFileName,	
  DWORD dwDesiredAccess, 
  DWORD dwShareMode, 
  DWORD dwCreationDisposition, 
  DWORD dwFlagsAndAttributes, 
  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
  HANDLE hTemplateFile
  )
{
	DN_ASSERT(FALSE == IsOpen(),		"Invalid!");
	DN_ASSERT(NULL != lpszFileName,		"Invalid!");
	if (NULL == lpszFileName) {
		return HASERROR;
	}

	m_hFile = ::CreateFile(lpszFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	if (INVALID_HANDLE_VALUE == m_hFile) {
		return(::GetLastError());
	}

	TCHAR szFileFullPath[MAX_PATH] = { _T('\0') };
	STRNCPY(szFileFullPath, COUNT_OF(szFileFullPath), lpszFileName);
	LPTSTR lpszFilePart;
	DWORD dwRetVal = ::GetFullPathName(szFileFullPath, _countof(m_szFileFullPath), m_szFileFullPath, &lpszFilePart);
	if (0 == dwRetVal) {
		return(::GetLastError());
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Close
//*---------------------------------------------------------------
// DESC : Close file handle and clean up object resources
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CFileSet::Close()
{
	if (INVALID_HANDLE_VALUE != m_hFile) {
		::CloseHandle(m_hFile);
		Reset();
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Flush
//*---------------------------------------------------------------
// DESC : Flush and empty the write buffer of the currently accessed file immediately
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::Flush()
{
	DN_ASSERT(TRUE == IsOpen(),		"Invalid!");
	if (FALSE == IsOpen()) {
		return FALSE;
	}

	return(::FlushFileBuffers(m_hFile));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::IsOpen
//*---------------------------------------------------------------
// DESC : Return whether the file object is currently open
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::IsOpen() const
{
	return(INVALID_HANDLE_VALUE != m_hFile);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetFullPath
//*---------------------------------------------------------------
// DESC : Get the full path of the currently accessed file
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetFullPath(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	return (_tcsncpy_s(lpszBuffer, nBufferSize, m_szFileFullPath, _TRUNCATE)?(TRUE):(FALSE));
#else	// #if	(_MSC_VER >= 1400)
	return (_tcsncpy(lpszBuffer, m_szFileFullPath, nBufferSize)?(TRUE):(FALSE));
#endif	// #if	(_MSC_VER >= 1400)
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetDrive
//*---------------------------------------------------------------
// DESC : Get the drive of the currently accessed file
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetDrive(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	return (!_tsplitpath_s(m_szFileFullPath, lpszBuffer, nBufferSize, NULL, 0, NULL, 0, NULL, 0)?(TRUE):(FALSE));
#else	// #if	(_MSC_VER >= 1400)
	return (!_tsplitpath(m_szFileFullPath, lpszBuffer, NULL, NULL, NULL)?(TRUE):(FALSE));
#endif	// #if	(_MSC_VER >= 1400)
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetPath
//*---------------------------------------------------------------
// DESC : Get the path of the currently accessed file
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetPath(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	return (!_tsplitpath_s(m_szFileFullPath, NULL, 0, lpszBuffer, nBufferSize, NULL, 0, NULL, 0)?(TRUE):(FALSE));
#else	// #if	(_MSC_VER >= 1400)
	return (!_tsplitpath(m_szFileFullPath, NULL, lpszBuffer, NULL, NULL)?(TRUE):(FALSE));
#endif	// #if	(_MSC_VER >= 1400)
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetName
//*---------------------------------------------------------------
// DESC : Get the name of the currently accessed file (excluding extension)
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetName(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	return (!_tsplitpath_s(m_szFileFullPath, NULL, 0, NULL, 0, lpszBuffer, nBufferSize, NULL, 0)?(TRUE):(FALSE));
#else	// #if	(_MSC_VER >= 1400)
	return (!_tsplitpath(m_szFileFullPath, NULL, NULL, lpszBuffer, NULL)?(TRUE):(FALSE));
#endif	// #if	(_MSC_VER >= 1400)
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetExt
//*---------------------------------------------------------------
// DESC : Get the extension of the currently accessed file
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetExt(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	return (!_tsplitpath_s(m_szFileFullPath, NULL, 0, NULL, 0, NULL, 0, lpszBuffer, nBufferSize)?(TRUE):(FALSE));
#else	// #if	(_MSC_VER >= 1400)
	return (!_tsplitpath(m_szFileFullPath, NULL, NULL, NULL, lpszBuffer)?(TRUE):(FALSE));
#endif	// #if	(_MSC_VER >= 1400)
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetTitle
//*---------------------------------------------------------------
// DESC : Get the name + extension of the currently accessed file
// PARM : 1 . lpszBuffer - String output buffer
//			2 . nBufferSize - string output buffer size 
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetTitle(/*out*/ LPTSTR lpszBuffer, DWORD nBufferSize) const
{
	DN_ASSERT(TRUE == IsOpen(),						"Invalid!");
	DN_ASSERT(NULL != lpszBuffer,					"Invalid!");
	DN_ASSERT(0 != nBufferSize,						"Invalid!");
	DN_ASSERT(_T('\0') != m_szFileFullPath[0],		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpszBuffer ||
		0 == nBufferSize ||
		0 == _tcscmp( m_szFileFullPath, _T("") )
		)
	{
		return FALSE;
	}

	TCHAR szFileName[MAX_PATH] = { _T('\0'), };
	TCHAR szExtName[MAX_PATH] = { _T('\0'), };
#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	INT iRetVal = _tsplitpath_s(m_szFileFullPath, NULL, 0, NULL, 0, szFileName, _countof(szFileName), szExtName, _countof(szExtName));
#else	// #if	(_MSC_VER >= 1400)
	INT iRetVal = _tsplitpath(m_szFileFullPath, NULL, NULL, szFileName, szExtName);
#endif	// #if	(_MSC_VER >= 1400)
	if (iRetVal) {
		return FALSE;
	}

#if	(_MSC_VER >= 1400)	// VS.NET 2005 or later
	iRetVal = _stprintf_s(lpszBuffer, nBufferSize, _T("%s%s"), szFileName, szExtName);
#else	// #if	(_MSC_VER >= 1400)
	iRetVal = _sntprintf(lpszBuffer, nBufferSize, _T("%s%s"), szFileName, szExtName);
#endif	// #if	(_MSC_VER >= 1400)
	if (iRetVal < 0) {
		return FALSE;
	}

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Seek
//*---------------------------------------------------------------
// DESC : Position the file pointer at a specific location in the currently accessed file
// PARM : 1 . dwMoveMethod - File pointer move method
/*
FILE_BEGIN : seek to a specific file position from the beginning of the file
FILE_CURRENT : seek to a specific file position from the current file position
FILE_END : seek to a specific file position from the end of the file
*/
//			2 . lOffsetLow - relative position to move file pointer (low LONG)
//			3 . lOffsetHigh - Relative offset to move file pointer (high LONG)
//			4. lpu64Offset - Moved file pointer position
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::Seek(DWORD dwMoveMethod, LONG lOffsetLow, LONG lOffsetHigh, ULONGLONG* lpu64Offset)
{
	DN_ASSERT(TRUE == IsOpen(),		"Invalid!");
	if (FALSE == IsOpen()) {
		return HASERROR;
	}

	LPLONG lpOffsetHigh = NULL;
	if (lOffsetHigh > 0) {
		lpOffsetHigh = &lOffsetHigh;
	}

	DWORD dwOffsetLow = ::SetFilePointer(m_hFile, lOffsetLow, lpOffsetHigh, dwMoveMethod);
	if (INVALID_SET_FILE_POINTER == dwOffsetLow) {
		DWORD dwErrNo = ::GetLastError();
		if (NO_ERROR != dwErrNo) {
			return dwErrNo;
		}
	}
	if (lpu64Offset) {
		if (lpOffsetHigh) {
			*lpu64Offset = MAKELONGLONG(dwOffsetLow, *lpOffsetHigh);
		}
		else {
			*lpu64Offset = MAKELONGLONG(dwOffsetLow, 0);
		}
	}
	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::SetLength
//*---------------------------------------------------------------
// DESC : Force-adjust the size of the currently accessed file to a specific size
// PARM : 1 . dwFileLength - File size to adjust
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::SetLength(ULONGLONG dwFileLength)
{
	DN_ASSERT(IsOpen(),				"Invalid!");

	DWORD dwRetVal = Seek(FILE_BEGIN, LOLONG(dwFileLength), HILONG(dwFileLength));
	if (NOERROR != dwRetVal) {
		return dwRetVal;
	}

	if (!::SetEndOfFile(m_hFile)) {
		dwRetVal = ::GetLastError();
		if (::GetLastError() != NO_ERROR) {
			return dwRetVal;
		}
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetLength
//*---------------------------------------------------------------
// DESC : Get the size of the currently accessed file
// PARM : 1 . dwFileLength - Retrieved file size
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::GetLength(/*out*/ ULONGLONG& dwFileLength) const
{
	DN_ASSERT(IsOpen(),				"Invalid!");

	ULARGE_INTEGER unFileSize;
	unFileSize.LowPart = ::GetFileSize(m_hFile, &unFileSize.HighPart);
	if (unFileSize.LowPart == INVALID_FILE_SIZE) {
		DWORD dwRetVal = ::GetLastError();
		if (NO_ERROR != dwRetVal) {
			return dwRetVal;
		}
	}

	dwFileLength = unFileSize.QuadPart;

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Read
//*---------------------------------------------------------------
// DESC : Read data starting from the current file pointer of the currently accessed file
// PARM : 1 . lpBuffer - Buffer pointer to read data into
//			2 . nBufferSize - buffer size
// RETV : NOERROR - success / ERROR_HANDLE_EOF - end of file read / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::Read(/*out*/ LPVOID lpBuffer, /*int,out*/ DWORD& nBufferSize)
{
	DN_ASSERT(TRUE == IsOpen(),		"Invalid!");
	DN_ASSERT(NULL != lpBuffer,		"Invalid!");
	DN_ASSERT(0 != nBufferSize,		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpBuffer ||
		0 == nBufferSize
		)
	{
		return HASERROR;
	}
	
	DWORD dwRetVal = ::ReadFile(m_hFile, lpBuffer, nBufferSize, &nBufferSize, NULL);
	if (0 == dwRetVal) {
		return(::GetLastError());
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Write
//*---------------------------------------------------------------
// DESC : Write data starting from the current file pointer of the currently accessed file
// PARM : 1 . lpBuffer - Buffer pointer with data to write
//			2 . nBufferSize - buffer size
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::Write(LPCVOID lpBuffer, /*int,out*/ DWORD& nBufferSize)
{
	DN_ASSERT(TRUE == IsOpen(),		"Invalid!");
	DN_ASSERT(NULL != lpBuffer,		"Invalid!");
	DN_ASSERT(0 != nBufferSize,		"Invalid!");
	if (FALSE == IsOpen() ||
		NULL == lpBuffer ||
		0 == nBufferSize
		)
	{
		return HASERROR;
	}

	DWORD dwRetVal = ::WriteFile(m_hFile, lpBuffer, nBufferSize, &nBufferSize, NULL);
	if (0 == dwRetVal) {
		return(::GetLastError());
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::LockRange
//*---------------------------------------------------------------
// DESC : Set an exclusive lock at a specific position of the currently accessed file to prevent access by other processes
// PARM : 1 . uOffset - Relative offset from start of file
//			2 . uNumberOfBytes - number of bytes to lock
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::LockRange(ULONGLONG uOffset, ULONGLONG uNumberOfBytes)
{
	DN_ASSERT(IsOpen(),				"Invalid!");
	DN_ASSERT(0 != uNumberOfBytes,	"Invalid!");

	ULARGE_INTEGER unULargeInterger;
	ULARGE_INTEGER unCount;

	unULargeInterger.QuadPart = uOffset;
	unCount.QuadPart = uNumberOfBytes;
	if (!::LockFile(m_hFile, unULargeInterger.LowPart, unULargeInterger.HighPart, unCount.LowPart, unCount.HighPart)) {
		return (::GetLastError());
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::SkipBOM
//*---------------------------------------------------------------
// DESC : Skip BOM (Byte Order Mark) indicating document file encoding if present
// PARM : N/A
// RETV : BOM type (not included if EV_BT_NONE)
// PRGM : B4nFter
// P.S.>
//		- Inevitably moves the file pointer to the front, then advances by the BOM size, leaving the file pointer right after the BOM
//		- If there is no BOM, move file pointer to the beginning
//*---------------------------------------------------------------
CFileSet::EF_BOMTYPE CFileSet::SkipBOM()
{
	DN_ASSERT(IsOpen(),				"Invalid!");

	if (!IsOpen()) {
		return EV_BT_NONE;
	}

	BYTE btData[UCHAR_MAX];
	DWORD dwSize, dwRetVal;
	for (DWORD dwIndex = 0 ; dwIndex < EV_BT_NONE ; ++dwIndex) {
		dwRetVal = Seek(FILE_BEGIN, 0, 0);
		if (NOERROR != dwRetVal) {
			DN_CONTINUE;
		}
		dwSize = m_stBomTypeDat[dwIndex].m_nSize;
		dwRetVal = Read(btData, dwSize);
		if (NOERROR != dwRetVal) {
			DN_CONTINUE;
		}
		if (dwSize != m_stBomTypeDat[dwIndex].m_nSize) {
			continue;
		}
		if (!::memcmp(btData, m_stBomTypeDat[dwIndex].m_btData, m_stBomTypeDat[dwIndex].m_nSize)) {
			return m_stBomTypeDat[dwIndex].m_eType;
		}
	}

	return EV_BT_NONE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::WriteBOM
//*---------------------------------------------------------------
// DESC : Write a specific BOM (Byte Order Mark) at the beginning of the file
// PARM : 1 . eBomType - BOM type to write
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::WriteBOM(CFileSet::EF_BOMTYPE eBomType)
{
	DN_ASSERT(IsOpen(),								"Invalid!");
	DN_ASSERT(CHECK_LIMIT(eBomType, EV_BT_NONE),	"Invalid!");

	if (!IsOpen() ||
		!CHECK_LIMIT(eBomType, EV_BT_NONE)
		)
	{
		return HASERROR;
	}

	DWORD dwRetVal = Seek(FILE_BEGIN, 0, 0);
	if (NOERROR != dwRetVal) {
		DN_RETURN(dwRetVal);
	}

	DWORD dwSize = m_stBomTypeDat[eBomType].m_nSize;
	dwRetVal = Write(m_stBomTypeDat[eBomType].m_btData, dwSize);
	if (NOERROR != dwRetVal) {
		DN_RETURN(dwRetVal);
	}
	if (dwSize != m_stBomTypeDat[eBomType].m_nSize) {
		DN_RETURN(HASERROR);
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::UnlockRange
//*---------------------------------------------------------------
// DESC : Release exclusive lock at specified position of previously locked file
// PARM : 1 . uOffset - Relative offset from start of file
//			2 . uNumberOfBytes - number of bytes to unlock
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::UnlockRange(ULONGLONG uOffset, ULONGLONG uNumberOfBytes)
{
	DN_ASSERT(IsOpen(),				"Invalid!");
	DN_ASSERT(0 != uNumberOfBytes,	"Invalid!");

	ULARGE_INTEGER unULargeInterger;
	ULARGE_INTEGER unCount;

	unULargeInterger.QuadPart = uOffset;
	unCount.QuadPart = uNumberOfBytes;
	if (!::UnlockFile(m_hFile, unULargeInterger.LowPart, unULargeInterger.HighPart, unCount.LowPart, unCount.HighPart)) {
		return (::GetLastError());
	}

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetPosition
//*---------------------------------------------------------------
// DESC : Get the file pointer position of the currently accessed file
// PARM : 1 . u64FilePositon - Variable to receive file pointer position
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::GetPosition(/*out*/ ULONGLONG& u64FilePositon) const
{
	DN_ASSERT(IsOpen(),				"Invalid!");

	LARGE_INTEGER unLargeInteger;
	unLargeInteger.QuadPart = 0;
	unLargeInteger.LowPart = ::SetFilePointer(m_hFile, unLargeInteger.LowPart, &unLargeInteger.HighPart , FILE_CURRENT);
	if (unLargeInteger.LowPart == (DWORD)-1) {
		DWORD dwErrNo = ::GetLastError();
		if (NO_ERROR != dwErrNo) {
			return dwErrNo;
		}
	}

	u64FilePositon = unLargeInteger.QuadPart;

	return NOERROR;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetTime
//*---------------------------------------------------------------
// DESC : Get the file time of the currently accessed file
// PARM : 1 . lpstCreateFileTime - Creation time
//			2 . lpstAccessFileTime - last access time
//			3 . lpstWriteFileTime - Last write time
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::GetTime(/*out*/ LPFILETIME lpstCreateFileTime, /*out*/ LPFILETIME lpstAccessFileTime, /*out*/ LPFILETIME lpstWriteFileTime) const
{
	DN_ASSERT(NULL != lpstCreateFileTime,		"Invalid!");
	DN_ASSERT(NULL != lpstAccessFileTime,		"Invalid!");
	DN_ASSERT(NULL != lpstWriteFileTime,		"Invalid!");

	DWORD dwRetVal = 0;
	if (INVALID_HANDLE_VALUE != m_hFile)
	{
		if (!::GetFileTime(m_hFile, lpstCreateFileTime, lpstAccessFileTime, lpstWriteFileTime))
		{
			dwRetVal = ::GetLastError();
			DN_ASSERT(0, "Error!");
		}
//		DN_VERIFY(::CloseHandle(m_hFile), TRUE,	"error!");
	}
	else
	{
		DN_RETURN(HASERROR);
	}

	return(dwRetVal);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetTime
//*---------------------------------------------------------------
// DESC : Get file time of a file at a specific path (static method)
// PARM : 1 . lpszFileName - Absolute/relative path including file name
//			2 . lpstCreateFileTime - creation time
//			3 . lpstAccessFileTime - Last access time
//			4. lpstWriteFileTime - Last write time
// RETV : NOERROR - success / other - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::GetTime(LPCTSTR lpszFileName, /*out*/ LPFILETIME lpstCreateFileTime, /*out*/ LPFILETIME lpstAccessFileTime, /*out*/ LPFILETIME lpstWriteFileTime)
{
	DN_ASSERT(NULL != lpstCreateFileTime,		"Invalid!");
	DN_ASSERT(NULL != lpstCreateFileTime,		"Invalid!");
	DN_ASSERT(NULL != lpstAccessFileTime,		"Invalid!");
	DN_ASSERT(NULL != lpstWriteFileTime,		"Invalid!");

	DWORD dwRetVal = 0;
	HANDLE hFile = ::CreateFile(lpszFileName, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE != hFile)
	{
		if (!::GetFileTime(hFile, lpstCreateFileTime, lpstAccessFileTime, lpstWriteFileTime))
		{
			dwRetVal = ::GetLastError();
			DN_ASSERT(0, "Error!");
		}
		DN_VERIFY(::CloseHandle(hFile), TRUE,	"Error!");
	}
	else
	{
		DN_RETURN(HASERROR);
	}

	return(dwRetVal);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetVersion
//*---------------------------------------------------------------
// DESC : Get version info of file at specified path (static method)
// PARM : 1 . lpszFileName - Absolute/relative path including file name
//			2 . dwFileVersion1 - version info 1
//			3 . dwFileVersion2 - Version info 2
//			4. dwFileVersion3 - Version info 3
//			5. dwFileVersion4 - Version info 4
// RETV : NOERROR - success / other - failure (returns ERROR_RESOURCE_TYPE_NOT_FOUND (1813L) if file binary has no version resource)
// PRGM : B4nFter
// P.S.>
//		- Version resource must already exist in the file binary
//*---------------------------------------------------------------
DWORD CFileSet::GetVersion(LPCTSTR lpszFileName, DWORD& dwFileVersion1, DWORD& dwFileVersion2, DWORD& dwFileVersion3, DWORD& dwFileVersion4)
{
	DWORD dwHandle= 0;
	DWORD dwFileVersionInfoSize = ::GetFileVersionInfoSize(lpszFileName, &dwHandle);
	if (0 == dwFileVersionInfoSize)
	{
		return (::GetLastError());	// If Version Resource is missing, ERROR_RESOURCE_TYPE_NOT_FOUND (1813L)
	}

	std::auto_ptr<BYTE>	aVerBuf(NEW BYTE[dwFileVersionInfoSize]);
	if (NULL == aVerBuf.get())
	{
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	DWORD	dwRetVal = 0;
	if (TRUE == ::GetFileVersionInfo(lpszFileName, dwHandle, dwFileVersionInfoSize, aVerBuf.get()))
	{
		UINT uLength    = 0;
		VS_FIXEDFILEINFO* lpVsFixedFileInfo = NULL;
		if (TRUE == ::VerQueryValue(aVerBuf.get(), _T("\\"), (LPVOID*)&lpVsFixedFileInfo, &uLength))
		{
			DN_ASSERT(sizeof(VS_FIXEDFILEINFO) == uLength,	"Error!");
			dwFileVersion1	= HIWORD(lpVsFixedFileInfo->dwFileVersionMS);
			dwFileVersion2	= LOWORD(lpVsFixedFileInfo->dwFileVersionMS);
			dwFileVersion3	= HIWORD(lpVsFixedFileInfo->dwFileVersionLS);
			dwFileVersion4	= LOWORD(lpVsFixedFileInfo->dwFileVersionLS);
		}
		else
		{
			dwRetVal = ERROR_NOT_FOUND;	// 1168L
		}
	}
	else
	{
		dwRetVal = ::GetLastError();
	}

	return dwRetVal;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetAttributes
//*---------------------------------------------------------------
// DESC : Get attribute info of file at specified path (static method)
// PARM : 1 . lpszFileName - Absolute/relative path including file name
// RETV : file attributes : success / INVALID_FILE_ATTRIBUTES : failure
// PRGM : B4nFter
//*---------------------------------------------------------------
DWORD CFileSet::GetAttributes(LPCTSTR lpszFileName)
{
	DN_ASSERT(NULL != lpszFileName,		"Invalid!");
	return (::GetFileAttributes(lpszFileName));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::SetAttributes
//*---------------------------------------------------------------
// DESC : Set attributes of file at specified path (static method)
// PARM : 1 . lpszFileName - Absolute/relative path including file name
//			2 . dwFileAttributes - file attributes
/*
				FILE_ATTRIBUTE_READONLY             0x00000001
				FILE_ATTRIBUTE_HIDDEN               0x00000002
				FILE_ATTRIBUTE_SYSTEM               0x00000004
				FILE_ATTRIBUTE_DIRECTORY            0x00000010
				FILE_ATTRIBUTE_ARCHIVE              0x00000020
				FILE_ATTRIBUTE_DEVICE               0x00000040
				FILE_ATTRIBUTE_NORMAL               0x00000080
				FILE_ATTRIBUTE_TEMPORARY            0x00000100
				FILE_ATTRIBUTE_SPARSE_FILE          0x00000200
				FILE_ATTRIBUTE_REPARSE_POINT        0x00000400
				FILE_ATTRIBUTE_COMPRESSED           0x00000800
				FILE_ATTRIBUTE_OFFLINE              0x00001000
				FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000
				FILE_ATTRIBUTE_ENCRYPTED            0x00004000
*/
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::SetAttributes(LPCTSTR lpszFileName, DWORD dwFileAttributes)
{
	DN_ASSERT(NULL != lpszFileName,			"Invalid!");
	return (::SetFileAttributes(lpszFileName, dwFileAttributes));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Remove
//*---------------------------------------------------------------
// DESC : Remove file at specified path (static method)
// PARM : 1 . lpszSrcFileName - Source file
//			2 . lpszTgtFileName - destination file
//			3 . pIsFailIfExist - Whether to fail if file already exists (TRUE: fail / FALSE: succeed (overwrite))
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::Copy(LPCTSTR lpszSrcFileName, LPCTSTR lpszTgtFileName, BOOL pIsFailIfExist)
{
	DN_ASSERT(NULL != lpszSrcFileName,	"Invalid!");
	DN_ASSERT(NULL != lpszTgtFileName,	"Invalid!");

	return (::CopyFile(lpszSrcFileName, lpszTgtFileName, pIsFailIfExist));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Remove
//*---------------------------------------------------------------
// DESC : Remove file at specified path (static method)
// PARM : 1 . lpszFileName - Absolute/relative path including file name
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::Remove(LPCTSTR lpszFileName)
{
	DN_ASSERT(NULL != lpszFileName,			"Invalid!");

	return (::DeleteFile(lpszFileName));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::Rename
//*---------------------------------------------------------------
// DESC : Change file name and path at a specific path (static method)
// PARM : 1 . lpszOldFileName - Absolute/relative path including previous file name
//			2 . lpszNewFileName - absolute/relative path including new file name
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::Rename(LPCTSTR lpszOldFileName, LPCTSTR lpszNewFileName)
{
	DN_ASSERT(NULL != lpszOldFileName,		"Invalid!");
	DN_ASSERT(NULL != lpszNewFileName,		"Invalid!");
	
	return (::MoveFile(lpszOldFileName, lpszNewFileName));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetTempName
//*---------------------------------------------------------------
// DESC : Generate temporary filename based on specified path, prefix, and number (static method)
// PARM : 1 . lpPathName - Base path
//			2 . lpPrefixString - base file prefix
//			3 . uUnique - Base number
//			4. lpTempFileName - Buffer to receive temporary file name
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CFileSet::GetTempName(LPCTSTR lpPathName, LPCTSTR lpPrefixString, UINT uUnique, /*out*/ LPTSTR lpTempFileName)
{
	DN_ASSERT(NULL != lpPathName,				"Invalid!");
	DN_ASSERT(NULL != lpPrefixString,			"Invalid!");
	DN_ASSERT(NULL != lpTempFileName,			"Invalid!");
	if (NULL == lpPathName) { return FALSE; }
	if (NULL == lpPrefixString) { return FALSE; }
	if (NULL == lpTempFileName) { return FALSE; }

	UINT uRetVal = ::GetTempFileName(lpPathName, lpPrefixString, uUnique, lpTempFileName);
	if (0 == uRetVal) {
		return FALSE;
	}

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CFileSet::GetBomTypeDat
//*---------------------------------------------------------------
// DESC : Get info for specified BOM type
// PARM : 1 . pBomType - EF_BOMTYPE enum value for specific BOM type
// RETV : info pointer for specific BOM type
// PRGM : B4nFter
//*---------------------------------------------------------------
CFileSet::LPFILEBOMTYPEDAT CFileSet::GetBomTypeDat(EF_BOMTYPE pBomType)
{
	if (!CHECK_LIMIT(pBomType, EV_BT_NONE)) {
		DN_RETURN(NULL);
	}

	return &m_stBomTypeDat[pBomType];
}
