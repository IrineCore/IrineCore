/*//===============================================================

	<< VARIABLE-ARGUMENT >>

	FILE : VARARG.H
DESC : Class that formats a string from variadic arguments
	INIT BUILT DATE : 2007. 09. 06
	LAST BUILT DATE : 2007. 09. 07

Copyright (c) 2002~2009 B4nFter (b4nfter@gmail.com) and Co-Writers. All Rights Reserved.

*///===============================================================


#pragma once


///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include <WINDOWS.H>
#include <CSTDARG>		// std::var_list


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////

#if defined(_UNICODE)
#define CVarArg		CVarArgW
#else	// _UNICODE
#define CVarArg		CVarArgA
#endif	// _UNICODE


///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CVarArgA
//*---------------------------------------------------------------
// DESC : Class that formats a string from variadic arguments (multibyte)
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
class CVarArgA
{
//	DISABLE_COPY_ASSIGN(CVarArgA) // Disallow object copy and assignment

public:
	explicit CVarArgA(const CHAR* lpFormat, ...);

	operator const CHAR*() const;
	const CHAR* GetMsg() const;

private:
	CHAR m_cBuffer[TP_MSGSIZE];
};


//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CVarArgW
//*---------------------------------------------------------------
// DESC : Class that formats a string from variadic arguments (Unicode)
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
class CVarArgW
{
//	DISABLE_COPY_ASSIGN(CVarArgW) // Disallow object copy and assignment

public:
	explicit CVarArgW(const WCHAR* lpFormat, ...);

	operator const WCHAR*() const;
	const WCHAR* GetMsg() const;

private:
	WCHAR m_cBuffer[TP_MSGSIZE];
};


///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgA::CVarArgA
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : 1 . lpFormat - Output format
//			2 . ... - variadic arguments
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
CVarArgA<TP_MSGSIZE>::CVarArgA(const CHAR* lpFormat, ...)
{
//	CHECK_STRINGA(lpFormat, INFINITE);
	
	std::va_list aArgs;
	va_start(aArgs, lpFormat);
//	VSNPRINTFA(m_cBuffer, COUNT_OF(m_cBuffer), lpFormat, aArgs);
	::_vsnprintf_s(m_cBuffer, _countof(m_cBuffer), lpFormat, aArgs);
	va_end(aArgs);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgA::operator CHAR*
//*---------------------------------------------------------------
// DESC : Cast object to CHAR* type
// PARM : N/A
// RETV : text buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
inline CVarArgA<TP_MSGSIZE>::operator const CHAR*() const
{
	return (GetMsg());
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgA::GetMsg
//*---------------------------------------------------------------
// DESC : Return object text buffer
// PARM : N/A
// RETV : text buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
inline const CHAR* CVarArgA<TP_MSGSIZE>::GetMsg() const
{
	return (m_cBuffer);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgW::CVarArgW
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : 1 . lpFormat - Output format
//			2 . ... - variadic arguments
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
CVarArgW<TP_MSGSIZE>::CVarArgW(const WCHAR* lpFormat, ...)
{
//	CHECK_STRINGW(lpFormat, INFINITE);
	
	std::va_list aArgs;
	va_start(aArgs, lpFormat);
//	VSNPRINTFW(m_cBuffer, COUNT_OF(m_cBuffer), lpFormat, aArgs);
	::_vsnwprintf_s(m_cBuffer, _countof(m_cBuffer), lpFormat, aArgs);
	va_end(aArgs);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgW::operator WCHAR*
//*---------------------------------------------------------------
// DESC : Cast object to WCHAR* type
// PARM : N/A
// RETV : text buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
inline CVarArgW<TP_MSGSIZE>::operator const WCHAR*() const
{
	return (GetMsg());
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CVarArgW::GetMsg
//*---------------------------------------------------------------
// DESC : Return object text buffer
// PARM : N/A
// RETV : text buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
template<DWORD TP_MSGSIZE>
inline const WCHAR* CVarArgW<TP_MSGSIZE>::GetMsg() const
{
	return (m_cBuffer);
}
