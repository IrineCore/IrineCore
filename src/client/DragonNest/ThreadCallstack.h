/*//===============================================================

	<< TREAD CALLSTACK >>

	PRGM : B4nFter

	FILE : TREADCALLSTACK.H, TREADCALLSTACK.CPP
DESC : Thread callstack log class
	INIT BUILT DATE : 2009. 09. 25
	LAST BUILT DATE : 2009. 09. 25

	P.S.> 
- Note: This class was created based on BugSlayer's CrashHandler from Debugging Applications for Windows .NET.
- This class itself is Thread-Safe, though whether the user's log output method is Thread-Safe is separate.
- When logging with an instance of this class, if the program database (.pdb) file is in the same folder, symbol info is loaded during object initialization (...::Open()) to record source filename and line number together.
- When compiling in Release (X86/X64) mode, code optimization prevents thread callstack info from being preserved properly; if accurate data is needed, adjust optimization options. (Project Properties -> C/C++ -> Optimization -> Optimization -> Disabled '/Od')

Copyright (c) 2002~2009 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include <WINDOWS.H>
#include <TCHAR.H>
#include <DBGHELP.H>
#include <PSAPI.H>
#define	_CONVERSION_DONT_USE_THREAD_LOCALE	// Use CP_THREAD_ACP macro like legacy ATL 3.0 instead of getting per-thread code page introduced in ATL 7.0
#include <ATLCONV.H>

#pragma comment (lib, "DBGHELP.LIB")
#pragma comment(lib, "PSAPI.LIB")


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////

#define DF_THREADCALLSTACK_TEXTBUFFERSIZE		(2048)
#define DF_THREADCALLSTACK_MAXSYMBOLSIZE		(512)


///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CThreadCallstack
//*---------------------------------------------------------------
// DESC : Thread callstack log class
// PRGM : B4nFter
//*---------------------------------------------------------------
class CThreadCallstack
{

public:
	typedef VOID (*LPFN_LOGPROC)(LPCSTR /*Text buffer*/, UINT /*Text buffer size (in TCHARs)*/, LPVOID /*User parameter*/);		// Function pointer declaration for sending call stack output to an external log output function
	typedef VOID (WINAPI *LPFN_RTLCAPTURECONTEXT)(PCONTEXT /*Context*/);

private:
// 	DISABLE_COPY_ASSIGN(CThreadCallstack); // Disallow copy and assignment between objects

//private:
public:
	CThreadCallstack();
	~CThreadCallstack();

 	DWORD Open(LPFN_LOGPROC pLogProc = NULL);
	VOID Close();
	BOOL IsOpen() const { return(m_InitSymbolEngine); }

 	DWORD Write(LPFN_LOGPROC pLogProc = NULL, LPVOID pParam = NULL, LPCSTR pPrefix = NULL, SIZE_T pPrefixSize = 0);

private:
	VOID Reset();

	const LPCSTR GetStackTraceString(PCONTEXT pContext, PIMAGEHLP_LINE64 pLine, LPSTACKFRAME64 pStackFrame, LPCSTR pPrefix, SIZE_T pPrefixSize, LPSTR pText, SIZE_T pTextSize, PBYTE pSymbol, SIZE_T pSymbolSize);

private:
	static BOOL __stdcall ReadProcessMemoryWork(HANDLE hModule, DWORD64 dw64BaseAddr, LPVOID lpBuffer, DWORD dwBufferSize, LPDWORD lpReadBytesSize);

private:
 	DWORD m_MachineType;
 	BOOL m_InitSymbolEngine;
	LPFN_LOGPROC m_LogProc;
	LPFN_RTLCAPTURECONTEXT m_RtlCaptureContext;

};
