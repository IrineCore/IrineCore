

///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryMap.h"
#include "CommonMacros.h"
#include "BaseSet.h"
#include "DebugUtil.h"
#include "SafeStringMacros.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::CMemoryMap
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CMemoryMap::CMemoryMap() :
	m_hFile(INVALID_HANDLE_VALUE),m_hMemoryMap(INVALID_HANDLE_VALUE),
	m_lpBaseAddr(0), m_dwSizeLow(0), m_dwSizeHigh(0)
{
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::~CMemoryMap
//*---------------------------------------------------------------
// DESC : Destructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CMemoryMap::~CMemoryMap()
{
	Close();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::Open
//*---------------------------------------------------------------
// DESC : Access and open file to use as Memory Map
// PARM : 1 . lpszFileName - File name to use as Memory Map
//			2 . dwFileOffsetHigh - file offset to access (high DWORD)
//			3 . dwFileOffsetLow - File offset to access (low DWORD)
//			4. dwNumberOfBytesToMap - Number of bytes to map
//			5. lpszMemoryMapName - Name of Memory Map
//			6. dwDesiredAccess - Desired access mode
//			7. dwCreationDisposition - How to create file (e.g., CREATE_ALWAYS, OPEN_EXISTING, etc.)
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CMemoryMap::Open(
					  LPCTSTR lpszFileName, 
					  DWORD dwFileOffsetHigh,  
					  DWORD dwFileOffsetLow, 
					  SIZE_T dwNumberOfBytesToMap,
					  LPCTSTR /*lpszMemoryMapName*/, 
					  DWORD dwDesiredAccess, 
					  DWORD dwCreationDisposition
					  )
{
	if (m_hFile != INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	m_hFile = ::CreateFile(lpszFileName, dwDesiredAccess, FILE_SHARE_READ, 0, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
	if (m_hFile == INVALID_HANDLE_VALUE) {
		DN_TRACE(_T("m_hFile == INVALID_HANDLE_VALUE : %d - CMemoryMap::Open()"), GetLastError());
		return FALSE;
	}

	m_hMemoryMap = ::CreateFileMapping(m_hFile, 0, PAGE_WRITECOPY, 0, 0, 0);
	if (!m_hMemoryMap) {
		DN_TRACE(_T("!m_hMemoryMap : %d - CMemoryMap::Open()"), GetLastError());
		return FALSE;
	}

	m_lpBaseAddr = ::MapViewOfFile(m_hMemoryMap, FILE_MAP_COPY, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
	if (!m_lpBaseAddr) {
		DN_TRACE(_T("!m_lpBaseAddr : %d - CMemoryMap::Open()"), GetLastError());
		return FALSE;
	}

	m_dwSizeLow = ::GetFileSize(m_hFile, &m_dwSizeHigh);

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::Close
//*---------------------------------------------------------------
// DESC : Close file used as Memory Map
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CMemoryMap::Close()
{
	if (m_lpBaseAddr != 0) {
		::UnmapViewOfFile(m_lpBaseAddr);
		m_lpBaseAddr = 0;
	}

	if (m_hMemoryMap != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_hMemoryMap);
		m_hMemoryMap = INVALID_HANDLE_VALUE;
	}

	if (m_hFile != INVALID_HANDLE_VALUE)	{
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	m_dwSizeLow = 0;
	m_dwSizeHigh = 0;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::ChangeMapView
//*---------------------------------------------------------------
// DESC : Change only the view position of Memory Map
// PARM : 1 . dwFileOffsetHigh - File position to access (high DWORD)
//			2 . dwFileOffsetLow - file offset to access (low DWORD)
//			3 . dwNumberOfBytesToMap - Number of bytes to map
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CMemoryMap::ChangeMapView(
	DWORD dwFileOffsetHigh , 
	DWORD dwFileOffsetLow, 
	SIZE_T dwNumberOfBytesToMap
	)
{
	if (m_hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	if (!m_hMemoryMap) {
		return FALSE;
	}
	if (m_lpBaseAddr != 0) {
		::UnmapViewOfFile(m_lpBaseAddr);
		m_lpBaseAddr = 0;
	}

	m_lpBaseAddr = ::MapViewOfFile(m_hMemoryMap, FILE_MAP_COPY, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
	if (!m_lpBaseAddr) {
		DN_TRACE(_T("!m_lpBaseAddr : %d - CMemoryMap::ChangeMapView()"), GetLastError());
		return FALSE;
	}

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::Flush
//*---------------------------------------------------------------
// DESC : Flush Memory Map buffer (I/O operation)
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CMemoryMap::Flush()
{
	if (::FlushViewOfFile(m_lpBaseAddr, 0) == FALSE) {
		DN_TRACE(_T("::FlushViewOfFile(m_lpBaseAddr, 0) == FALSE : %d - CMemoryMap::Open()"), GetLastError());
		return FALSE;
	}

	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::GetBaseAddress
//*---------------------------------------------------------------
// DESC : Get base address of Memory Map file
// PARM : N/A
// RETV : base address of Memory Map file
// PRGM : B4nFter
//*---------------------------------------------------------------
LPVOID CMemoryMap::GetBaseAddress()
{
	return m_lpBaseAddr;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMemoryMap::GetSize
//*---------------------------------------------------------------
// DESC : Get size of Memory Map file
// PARM : N/A
// RETV : size of Memory Map file (INT64)
// PRGM : B4nFter
//*---------------------------------------------------------------
UINT64 CMemoryMap::GetSize()
{
	return MAKELONGLONG(m_dwSizeLow, m_dwSizeHigh);
}
