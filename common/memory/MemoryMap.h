/*//===============================================================

	<< MEMORY MAP >>

	FILE : MEMORYMAP.H, MEMORYMAP.CPP
DESC : Memory Map management class
	INIT BUILT DATE : 2005. 12. 11
	LAST BUILT DATE : 2005. 12. 11

	P.S.> 
- Memory map created solely to access and read an existing file
- When creating a new physical file or if file size is 0, ::CreateFileMapping in CMemoryMap::Open can fail, so check file size

Copyright (c) 2002~2009 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include <windows.h>


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CMemoryMap
//*---------------------------------------------------------------
// DESC : Memory Map management class
// PRGM : B4nFter
//*---------------------------------------------------------------
class CMemoryMap
{

public:
	CMemoryMap();
	~CMemoryMap();

	BOOL Open(
		LPCTSTR lpszFileName, 
		DWORD dwFileOffsetHigh = 0, 
		DWORD dwFileOffsetLow = 0, 
		SIZE_T dwNumberOfBytesToMap = 0,
		LPCTSTR lpszMemoryMapName = NULL, 
		DWORD dwDesiredAccess = GENERIC_READ, 
		DWORD dwCreationDisposition = OPEN_EXISTING
		);
	VOID Close();

	BOOL ChangeMapView(
		DWORD dwFileOffsetHigh = 0, 
		DWORD dwFileOffsetLow = 0, 
		SIZE_T dwNumberOfBytesToMap = 0
		);
	BOOL Flush();

	LPVOID GetBaseAddress();
	UINT64 GetSize();

private:
	HANDLE m_hFile;
	HANDLE m_hMemoryMap;

	LPVOID m_lpBaseAddr;
	DWORD m_dwSizeLow;
	DWORD m_dwSizeHigh;
};
