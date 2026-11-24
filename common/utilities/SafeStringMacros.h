/*//===============================================================

	<< SAFE STRING MACROS >>

	FILE : SAFESTRINGMACROS.H
DESC : Header file collecting macros related to safe string CRT functions used project-wide
	INIT BUILT DATE : 2007. 03. 21
	LAST BUILT DATE : 2007. 05. 17

Copyright (c) 2002~2009 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

/*
	P.S.>
- This header must be included in the project's top-level header
- Assumes basic headers such as WINDOWS.H are already included before this header (thus WINDOWS.H etc. are excluded here)
*/

// The headers below were not previously included and are unrelated to safe string CRT functions
#define	_CONVERSION_DONT_USE_THREAD_LOCALE	// Use CP_THREAD_ACP macro like legacy ATL 3.0 instead of getting per-thread code page introduced in ATL 7.0
#include <atlbase.h>	// For ATL 7.0 string conversion classes and macros
#include <atlconv.h>


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////

// Macro functions guaranteeing string integrity
#if defined(_DEBUG)
#define	CHECK_STRINGA(pMSG, pSZ)				DN_ASSERT((NULL != pMSG) && ('\0'  != pMSG[0]) && (0 < pSZ),		"Invalid string!");
#define	CHECK_STRINGW(pMSG, pSZ)				DN_ASSERT((NULL != pMSG) && (L'\0' != pMSG[0]) && (0 < pSZ),		"Invalid string!");
//#define	CHECK_STRINGA(pMSG, pSZ)			DN_ASSERT(!::IsBadStringPtrA(pMSG, pSZ) && ('\0'  != pMSG[0]) && (::strlen(pMSG) <= (size_t)pSZ),		"Invalid string!");
//#define	CHECK_STRINGW(pMSG, pSZ)			DN_ASSERT(!::IsBadStringPtrW(pMSG, pSZ) && (L'\0' != pMSG[0]) && (::wcslen(pMSG) <= (size_t)pSZ),		"Invalid string!");
#else	// #if defined(_DEBUG)
#define	CHECK_STRINGA(pMSG, pSZ)
#define	CHECK_STRINGW(pMSG, pSZ)
#endif	// #if defined(_DEBUG)
#define	MAKE_STRINGA(pSTR)						(pSTR[COUNT_OF(pSTR)-1] = '\0')
#define	MAKE_STRINGW(pSTR)						(pSTR[COUNT_OF(pSTR)-1] = L'\0')
#if defined(_UNICODE)
#define	CHECK_STRING							CHECK_STRINGW
#define	MAKE_STRING(pSTR)						MAKE_STRINGW(pSTR)
#else	// #if defined(_UNICODE)
#define	CHECK_STRING							CHECK_STRINGA
#define	MAKE_STRING(pSTR)						MAKE_STRINGA(pSTR)
#endif	// #if defined(_UNICODE)

// Macro functions related to safe string CRT functions
//#if defined(_DEBUG)
//#if !defined( _FINAL_BUILD )
	//#if (_MSC_VER >= 1400) // VS.NET 2005 or later
		#define	STRCPYA(pDEST, pDESTSZ, pSTR)			{ int error_t = ::strcpy_s(pDEST, pDESTSZ, pSTR );				DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRCPYW(pDEST, pDESTSZ, pSTR)			{ int error_t = ::wcscpy_s(pDEST, pDESTSZ, pSTR);				DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRNCPYA(pDEST, pDESTSZ, pSTR)			{ int error_t = ::strncpy_s(pDEST, pDESTSZ, pSTR, pDESTSZ);		DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRNCPYW(pDEST, pDESTSZ, pSTR)			{ int error_t = ::wcsncpy_s(pDEST, pDESTSZ, pSTR, pDESTSZ);		DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRCATA(pDEST, pDESTSZ, pSTR)			{ int error_t = ::strcat_s(pDEST, pDESTSZ, pSTR);				DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRCATW(pDEST, pDESTSZ, pSTR)			{ int error_t = ::wcscat_s(pDEST, pDESTSZ, pSTR);				DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRNCATA(pDEST, pDESTSZ, pSTR)			{ int error_t = ::strncat_s(pDEST, pDESTSZ, pSTR, pDESTSZ);		DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRNCATW(pDEST, pDESTSZ, pSTR)			{ int error_t = ::wcsncat_s(pDEST, pDESTSZ, pSTR, pDESTSZ);		DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRUPA(pSTR, pSTRSZ)					{ int error_t = ::_strupr_s(pSTR, pSTRSZ);						DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRUPW(pSTR, pSTRSZ)					{ int error_t = ::_wcsupr_s(pSTR, pSTRSZ);						DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRLWA(pSTR, pSTRSZ)					{ int error_t = ::_strlwr_s(pSTR, pSTRSZ);						DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	STRLWW(pSTR, pSTRSZ)					{ int error_t = ::_wcslwr_s(pSTR, pSTRSZ);						DN_ASSERT( 0 == error_t, "Invalid!"); }
		#define	SPRINTFA								::sprintf_s
		#define	SPRINTFW								::swprintf_s
		#define	SNPRINTFA								::_snprintf_s
		#define	SNPRINTFW								::_snwprintf_s
		#define	VSNPRINTFA(pDEST, pDESTSZ, pFMT, pARG)	::_vsnprintf_s(pDEST, pDESTSZ, pDESTSZ-1, pFMT, pARG)
		#define	VSNPRINTFW(pDEST, pDESTSZ, pFMT, pARG)	::_vsnwprintf_s(pDEST, pDESTSZ, pDESTSZ-1, pFMT, pARG)
// 		#define	VSNPRINTFA(pDEST, pDESTSZ, pFMT, pARG)	DN_ASSERT(0 < ::_vsnprintf_s(pDEST, pDESTSZ, pDESTSZ-1, pFMT, pARG),	"Invalid!")
// 		#define	VSNPRINTFW(pDEST, pDESTSZ, pFMT, pARG)	DN_ASSERT(0 < ::_vsnwprintf_s(pDEST, pDESTSZ, pDESTSZ-1, pFMT, pARG),	"Invalid!")
		#define	SSCANFA									::sscanf
		#define	SSCANFW									::swscanf
// 	#define	STRLENA(pSTR, pSTRSZ)						CDebugUtil::StrLenA(pSTR, pSTRSZ)
// 	#define	STRLENW(pSTR, pSTRSZ)						CDebugUtil::StrLenW(pSTR, pSTRSZ)
	#define	FOPENA(pPATH, pMODE)						CDebugUtil::FOpenA(pPATH, pMODE)
	#define	FOPENW(pPATH, pMODE)						CDebugUtil::FOpenW(pPATH, pMODE)
	#define SPLITPATHA(pPATH, pDRV, pDRVSZ, pDIR, pDIRSZ, pFNM, pFNMSZ, pEXT, pEXTSZ)	::_splitpath_s(pPATH, pDRV, pDRVSZ, pDIR, pDIRSZ, pFNM, pFNMSZ, pEXT, pEXTSZ)
	#define SPLITPATHW(pPATH, pDRV, pDRVSZ, pDIR, pDIRSZ, pFNM, pFNMSZ, pEXT, pEXTSZ)	::_wsplitpath_s(pPATH, pDRV, pDRVSZ, pDIR, pDIRSZ, pFNM, pFNMSZ, pEXT, pEXTSZ)

#if defined(_UNICODE)
	#define	STRCPY		STRCPYW
	#define	STRNCPY		STRNCPYW
	#define	STRCAT		STRCATW
	#define	STRNCAT		STRNCATW
	#define	STRUP		STRUPW
	#define	STRLW		STRLWW
	#define	SPRINTF		SPRINTFW
	#define	SNPRINTF	SNPRINTFW
	#define	VSNPRINTF	VSNPRINTFW
	#define	SSCANF		SSCANFW
//	#define	STRLEN		STRLENW
	#define	FOPEN		FOPENW
	#define SPLITPATH	SPLITPATHW
#else	// _UNICODE
	#define	STRCPY		STRCPYA
	#define	STRNCPY		STRNCPYA
	#define	STRCAT		STRCATA
	#define	STRNCAT		STRNCATA
	#define	STRUP		STRUPA
	#define	STRLW		STRLWA
	#define	SPRINTF		SPRINTFA
	#define	SNPRINTF	SNPRINTFA
	#define	VSNPRINTF	VSNPRINTFA
	#define	SSCANF		SSCANFA
//	#define	STRLEN		STRLENA
	#define	FOPEN		FOPENA
	#define SPLITPATH	SPLITPATHA
#endif	// _UNICODE
