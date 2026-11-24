/*//===============================================================

	<< BASE-SET >>

	FILE : BASESET.H
DESC : Class for checking basic project settings
	INIT BUILT DATE : 2007. 03. 13
	LAST BUILT DATE : 2007. 03. 13

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


///////////////////////////////////////////////////////////////////
//	DEFINE
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//	DECLARATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : CLASS
// NAME : CBaseSet
//*---------------------------------------------------------------
// DESC : Class for checking basic project settings
// PRGM : B4nFter
//*---------------------------------------------------------------

class CBaseSet
{
public:
	static BOOL IsDebug();
	static BOOL IsUnicode();
	static BOOL IsMultiThread();
	static BOOL IsLittleEndian();
	static BOOL IsWindowStationVisible();
};


///////////////////////////////////////////////////////////////////
//	IMPLEMENTATION
///////////////////////////////////////////////////////////////////

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBaseSet::
//*---------------------------------------------------------------
// DESC : Return whether the current project configuration is _DEBUG
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
inline BOOL CBaseSet::IsDebug()
{
	return(MODE_CMP(TRUE,FALSE));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBaseSet::IsUnicode
//*---------------------------------------------------------------
// DESC : Return whether the current project language option is _UNICODE
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
inline BOOL CBaseSet::IsUnicode()
{
	return(MODE_CHARSET(FALSE, TRUE));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBaseSet::IsMultiThread
//*---------------------------------------------------------------
// DESC : Return whether the current project code generation option is _MT
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
inline BOOL CBaseSet::IsMultiThread()
{
	return(MODE_MT(FALSE,TRUE));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBaseSet::IsLittleEndian
//*---------------------------------------------------------------
// DESC : Return whether the current platform is little-endian based
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
inline BOOL CBaseSet::IsLittleEndian()
{
	const unsigned long	ulHost = 0x12345678;
	return(::htonl(ulHost) != ulHost);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CBaseSet::IsWindowStationVisible
//*---------------------------------------------------------------
// DESC : Return whether the current process window station is USER-INTERACTIVE
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
inline BOOL CBaseSet::IsWindowStationVisible()
{
#if defined(_WIN32)
	HWINSTA hWindowStation = ::GetProcessWindowStation();
	if (hWindowStation != NULL) {
		USEROBJECTFLAGS	stUOF;
		DWORD dwNeed = 0;
		if (::GetUserObjectInformation(hWindowStation, UOI_FLAGS, &stUOF, sizeof(USEROBJECTFLAGS), &dwNeed) == TRUE) {
			return ((WSF_VISIBLE & stUOF.dwFlags) == WSF_VISIBLE);
		}
	}
#endif
	return FALSE;
}
