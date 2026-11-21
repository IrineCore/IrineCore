
#include "stdafx.h"
#include "MiniDump.h"
#include "ExceptionReport.h"

LPTOP_LEVEL_EXCEPTION_FILTER pOldPointer = NULL;

#pragma comment (lib, "dbghelp.lib")

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

LONG __stdcall Exception_Minidump(_EXCEPTION_POINTERS* pExceptionInfo)
{
	TCHAR fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	TCHAR * ext = _tcsrchr(fileName, '.');
	srand(timeGetTime());
	_stprintf(ext ? ext : fileName+_tcslen(fileName), _T(".%d%d%d%d.dmp"), rand()%10, rand()%10, rand()%10, rand()%10);

	HANDLE hProcess = GetCurrentProcess();
	DWORD dwProcessID = GetCurrentProcessId();
	HANDLE hFile = CreateFile(fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	MINIDUMP_EXCEPTION_INFORMATION eInfo;
	eInfo.ThreadId = GetCurrentThreadId();
	eInfo.ExceptionPointers = pExceptionInfo;
	eInfo.ClientPointers = FALSE;

	MiniDumpWriteDump(hProcess, dwProcessID, hFile, MiniDumpWithFullMemory, pExceptionInfo ? &eInfo : NULL, NULL, NULL);

	return pOldPointer ? pOldPointer(pExceptionInfo) : EXCEPTION_EXECUTE_HANDLER;
}

// STL invalid index reference
static void myInvalidParameterHandler(const wchar_t* expression,
							   const wchar_t* function, 
							   const wchar_t* file, 
							   unsigned int line, 
							   uintptr_t pReserved)
{
	RaiseException( EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, 0);
}

// Pure virtual function call
static void myPurecallHandler()
{
	RaiseException( EXCEPTION_ACCESS_VIOLATION, EXCEPTION_NONCONTINUABLE , 0, 0);
}

void SetMiniDump()
{
	_set_invalid_parameter_handler( myInvalidParameterHandler );
	_set_purecall_handler( myPurecallHandler );
	SetUnhandledExceptionFilter(Exception_Minidump);

	printf("DbgHelp API Version Num[%d]\n", API_VERSION_NUMBER);
}

void StoreMiniDump(const TCHAR * szFName, _MINIDUMP_TYPE dumptype)
{
	if (CExceptionReport::GetInstancePtr()->IsDumping()) return;

	HANDLE hProcess = GetCurrentProcess();
	DWORD dwProcessID = GetCurrentProcessId();
	HANDLE hFile = CreateFile(szFName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);	
	MiniDumpWriteDump(hProcess, dwProcessID, hFile, dumptype, NULL, NULL, NULL);
	CloseHandle(hFile);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined (API_VERSION_NUMBER) && (API_VERSION_NUMBER >= 11)
const MINIDUMP_TYPE MINIDUMP = (MINIDUMP_TYPE)(MiniDumpNormal);
const MINIDUMP_TYPE MIDIDUMP = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData |
											   MiniDumpWithUnloadedModules);
const MINIDUMP_TYPE MAXIDUMP = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData |
											   MiniDumpWithUnloadedModules);
#else
const MINIDUMP_TYPE MINIDUMP = (MINIDUMP_TYPE)(MiniDumpNormal);
const MINIDUMP_TYPE MIDIDUMP = (MINIDUMP_TYPE)(MiniDumpWithPrivateReadWriteMemory | MiniDumpWithDataSegs | MiniDumpWithHandleData |
											   MiniDumpWithUnloadedModules);
const MINIDUMP_TYPE MAXIDUMP = (MINIDUMP_TYPE)(MiniDumpWithFullMemory | MiniDumpWithHandleData |
											   MiniDumpWithUnloadedModules );
#endif
//Handled Exception
LONG StoreMiniDump(_DUMPTYPE eType, EXCEPTION_POINTERS * pE)
{
	//if you want use EXCEPTION_POINTERS case use __try __exception can using this func GetExceptionInformation()
	// Open the file 
	TCHAR fileName[MAX_PATH];
	GetModuleFileName(NULL, fileName, MAX_PATH);
	TCHAR * ext = _tcsrchr(fileName, '.');
	srand(timeGetTime());
	_stprintf(ext ? ext : fileName+_tcslen(fileName), _T(".Exception.%d%d%d%d.dmp"), rand()%10, rand()%10, rand()%10, rand()%10);

	HANDLE hProcess = GetCurrentProcess();
	DWORD dwProcessID = GetCurrentProcessId();
	HANDLE hFile = CreateFile(fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		// Create the minidump
		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pE; 
		mdei.ClientPointers     = FALSE; 

		MINIDUMP_CALLBACK_INFORMATION mci; 

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallBack; 
		mci.CallbackParam       = 0; 

		MINIDUMP_TYPE mdt;
		switch (eType)
		{
		case _DUMPTYPE_MINI: mdt = MINIDUMP; break;
		case _DUMPTYPE_MIDI: mdt = MIDIDUMP; break;
		case _DUMPTYPE_MAXI: mdt = MAXIDUMP; break;
		default: mdt = MINIDUMP; break;
		}

		BOOL rv = MiniDumpWriteDump(hProcess, dwProcessID, hFile, mdt, (pE != NULL) ? &mdei : NULL, NULL, eType != _DUMPTYPE_MAXI ? &mci : NULL); 

		if(!rv) _tprintf(_T("MiniDumpWriteDump failed. Error: %d \n"), GetLastError()); 
		else _tprintf(_T("Minidump created.\n") ); 

		// Close the file
		CloseHandle(hFile);
	}
	else 
		_tprintf(_T("CreateFile failed. Error: %u \n"), GetLastError()); 

	return EXCEPTION_EXECUTE_HANDLER;
}

BOOL CALLBACK MiniDumpCallBack(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
{
	BOOL bRet = FALSE;

	// Check parameters
	if(pInput == 0)	return FALSE;
	if(pOutput == 0) return FALSE;

	// Process the callbacks
	switch(pInput->CallbackType)
	{
	case IncludeModuleCallback:
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

	case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

	case ModuleCallback: 
		{
			// Are data sections available for this module ?
			if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg) 
			{
				// Yes, they are, but do we need them?
				if(!IsDataSectionNeeded(pInput->Module.FullPath)) 
				{
					wprintf(L"Excluding module data sections: %s \n", pInput->Module.FullPath);
					pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
				}
			}

			bRet = TRUE; 
		}
		break; 

	case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

	case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

	case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 
#if defined (API_VERSION_NUMBER) && (API_VERSION_NUMBER >= 11)
	case CancelCallback: 
		break; 
#endif
	}

	return bRet; 

}

bool IsDataSectionNeeded(const WCHAR* pModuleName)
{
	// Check parameters
	if(pModuleName == 0) 
	{
		_ASSERT(_T("Parameter is null."));
		return false; 
	}

	// Extract the module name 
	WCHAR szFileName[_MAX_FNAME] = L"";
	_wsplitpath(pModuleName, NULL, NULL, szFileName, NULL);

	// Compare the name with the list of known names and decide
	// Note: For this to work, the executable name must be "mididump.exe"
	if(wcsicmp(szFileName, L"mididump") == 0) 
		return true;
	else if(wcsicmp(szFileName, L"ntdll") == 0)
		return true;

	// Complete
	return false;
}