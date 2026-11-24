

///////////////////////////////////////////////////////////////////
//	INCLUDE
///////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringSet.h"
#include <algorithm>
#include <set>
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
// NAME : CStringSet::CStringSet
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::CStringSet()
{
	m_lpszStringBuffer = NULL;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::CStringSet
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::CStringSet(LPCTSTR pString)
{
	m_lpszStringBuffer = NULL;

	operator=(pString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::CStringSet
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::CStringSet(const CStringSet& pString)
{
	m_lpszStringBuffer = NULL;

	operator=(pString.Get());
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::CStringSet
//*---------------------------------------------------------------
// DESC : Constructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::CStringSet(const CStringSet* pString)
{
	m_lpszStringBuffer = NULL;

	operator=(pString->Get());
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::~CStringSet
//*---------------------------------------------------------------
// DESC : Destructor
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::~CStringSet()
{
	Empty();
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsEmpty
//*---------------------------------------------------------------
// DESC : Check whether string data exists
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsEmpty() const
{
	return (BOOL)(!m_lpszStringBuffer || _T('\0') == m_lpszStringBuffer[0]);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsNull
//*---------------------------------------------------------------
// DESC : Check whether string buffer pointer is NULL
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsNull() const
{
	return(!m_lpszStringBuffer);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Empty
//*---------------------------------------------------------------
// DESC : Remove and initialize string buffer
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::Empty()
{
	if (m_lpszStringBuffer) {
		::free((LPVOID)((LPBYTE)(m_lpszStringBuffer) - sizeof(STRINGHEADER)));
		m_lpszStringBuffer = NULL;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::ReAlloc
//*---------------------------------------------------------------
// DESC : Remove and initialize string data buffer
// PARM : 1 . iNewSize - New buffer size to allocate
//			2 . bForceTo - whether to force buffer size to the previously input value
// RETV : returns newly allocated string buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
LPTSTR CStringSet::ReAlloc(INT iNewSize, BOOL bForceTo)
{
	if (iNewSize <= 0) {
		return NULL;
	}

	LPBYTE lpbtTempBuffer;
	LPSTRINGHEADER lpstStringHeader;
	if (m_lpszStringBuffer) {
		lpbtTempBuffer = (LPBYTE)(m_lpszStringBuffer) - sizeof(STRINGHEADER);
		if (bForceTo) {
			lpbtTempBuffer = (LPBYTE)::realloc(lpbtTempBuffer, iNewSize*sizeof(TCHAR)+sizeof(STRINGHEADER));
			lpstStringHeader = (LPSTRINGHEADER)(lpbtTempBuffer);
			lpstStringHeader->m_iSize = iNewSize;
			if (lpstStringHeader->m_iLinePtr > iNewSize-1) {
				lpstStringHeader->m_iLinePtr = iNewSize-1;
			}
		}
		else {
			lpstStringHeader = (LPSTRINGHEADER)(lpbtTempBuffer);
			if (lpstStringHeader->m_iSize < iNewSize) {
				lpbtTempBuffer = (LPBYTE)::realloc(lpbtTempBuffer, iNewSize*sizeof(TCHAR)+sizeof(STRINGHEADER));
				lpstStringHeader = (LPSTRINGHEADER)(lpbtTempBuffer);
				lpstStringHeader->m_iSize = iNewSize;
			}
		}
	}
	else {
		lpbtTempBuffer = (LPBYTE)::calloc(iNewSize*sizeof(TCHAR)+sizeof(STRINGHEADER), sizeof(BYTE));
		if (lpbtTempBuffer) {
			lpstStringHeader = (LPSTRINGHEADER)(lpbtTempBuffer);
			lpstStringHeader->m_iLinePtr = 0;
			lpstStringHeader->m_iSize = iNewSize;
		}
	}

	if (lpbtTempBuffer) {
		m_lpszStringBuffer = (LPTSTR)(lpbtTempBuffer+sizeof(STRINGHEADER));
		lpstStringHeader = (LPSTRINGHEADER)(lpbtTempBuffer);
		m_lpszStringBuffer[lpstStringHeader->m_iSize-1] = _T('\0');
		return m_lpszStringBuffer;
	}
	return NULL;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Get
//*---------------------------------------------------------------
// DESC : Return string buffer pointer
// PARM : N/A
// RETV : returns string buffer pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
LPCTSTR CStringSet::Get() const
{
	return m_lpszStringBuffer;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetA
//*---------------------------------------------------------------
// DESC : Return string buffer pointer (ASCII format)
// PARM : N/A
// RETV : returns string buffer pointer (ASCII format, HEAP memory)
// PRGM : B4nFter
// P.S.>
//		- Returned string data must be freed with ::free() if non-NULL
//*---------------------------------------------------------------
LPCSTR CStringSet::GetA() const
{
	return ::strdup(CT2A(m_lpszStringBuffer));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetW
//*---------------------------------------------------------------
// DESC : Return string buffer pointer (UNICODE format)
// PARM : N/A
// RETV : returns string buffer pointer (UNICODE format, HEAP memory)
// PRGM : B4nFter
// P.S.>
//		- Returned string data must be freed with ::free() if non-NULL
//*---------------------------------------------------------------
LPCWSTR CStringSet::GetW() const
{
	return ::wcsdup(CT2W(m_lpszStringBuffer));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Get
//*---------------------------------------------------------------
// DESC : Return string buffer pointer (UNICODE format)
// PARM : 1 . nStart - Start point of string buffer pointer
// RETV : returns string buffer pointer (UNICODE format)
// PRGM : B4nFter
//*---------------------------------------------------------------
LPCTSTR CStringSet::Get(INT nStart) const
{
	if (!m_lpszStringBuffer) {
		return NULL;
	}

	INT nSize = GetSize();
	if (nSize <= 0 || nStart < 0 || nSize-1 < nStart) {
		return NULL;
	}

	return m_lpszStringBuffer + nStart;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetA
//*---------------------------------------------------------------
// DESC : Return string buffer pointer (ASCII format)
// PARM : 1 . nStart - Start point of string buffer pointer
// RETV : returns string buffer pointer (ASCII format, HEAP memory)
// PRGM : B4nFter
// P.S.>
//		- Returned string data must be freed with ::free() if non-NULL
//*---------------------------------------------------------------
LPCSTR CStringSet::GetA(INT nStart) const
{
	if (!m_lpszStringBuffer) {
		return NULL;
	}

	INT nSize = GetSize();
	if (nSize <= 0 || nStart < 0 || nSize-1 < nStart) {
		return NULL;
	}

	return ::strdup(CT2A(m_lpszStringBuffer + nStart));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetW
//*---------------------------------------------------------------
// DESC : Return string buffer pointer (UNICODE format)
// PARM : 1 . nStart - Start point of string buffer pointer
// RETV : returns string buffer pointer (UNICODE format, HEAP memory)
// PRGM : B4nFter
// P.S.>
//		- Returned string data must be freed with ::free() if non-NULL
//*---------------------------------------------------------------
LPCWSTR CStringSet::GetW(INT nStart) const
{
	if (!m_lpszStringBuffer) {
		return NULL;
	}

	INT nSize = GetSize();
	if (nSize <= 0 || nStart < 0 || nSize-1 < nStart) {
		return NULL;
	}

	return ::wcsdup(CT2W(m_lpszStringBuffer + nStart));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetLine
//*---------------------------------------------------------------
// DESC : Return one line from entire string
// PARM : N/A
// PARM : 1 . nStart - Start point of string buffer pointer
// RETV : CStringSet object returning one line from the full string
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::GetLine()
{ 
	CStringSet aString;
	if (!IsEmpty()) {
		INT iStartPtr = GetLinePtr();
		aString = Tokenize(DF_DEFAULT_NEWLINE_TOKEN_STRING, iStartPtr);
		SetLinePtr(iStartPtr);
	}
	return aString;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetLength
//*---------------------------------------------------------------
// DESC : Return length of string (TCHAR-based)
// PARM : N/A
// RETV : length of string
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::GetLength() const
{
	if (m_lpszStringBuffer) {
		return (INT)_tcslen(m_lpszStringBuffer);
	}
	else {
		return 0;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetHeader
//*---------------------------------------------------------------
// DESC : Return string buffer header
// PARM : N/A
// RETV : string buffer header
// PRGM : B4nFter
//*---------------------------------------------------------------
LPSTRINGHEADER CStringSet::GetHeader() const
{
	if (m_lpszStringBuffer) {
		return (LPSTRINGHEADER)((LPBYTE)(m_lpszStringBuffer) - sizeof(STRINGHEADER));
	}
	else {
		return NULL;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetSize
//*---------------------------------------------------------------
// DESC : Return length of string buffer (TCHAR-based)
// PARM : N/A
// RETV : length of string buffer
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::GetSize() const
{
	if (m_lpszStringBuffer) {
		return GetHeader()->m_iSize;
	}
	else {
		return 0;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetLinePtr
//*---------------------------------------------------------------
// DESC : Return pointer delimiting a line of the string
// PARM : N/A
// RETV : string line-delimiter pointer
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::GetLinePtr() const
{
	if (m_lpszStringBuffer) {
		return GetHeader()->m_iLinePtr;
	}
	else {
		return 0;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::SetLinePtr
//*---------------------------------------------------------------
// DESC : Set pointer delimiting a line of the string
// PARM : 1 . iLinePtr - Pointer value separating a line of string
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::SetLinePtr(INT iLinePtr)
{
	if (m_lpszStringBuffer) {
		GetHeader()->m_iLinePtr = iLinePtr;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Reset
//*---------------------------------------------------------------
// DESC : Reset string data and header
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::Reset()
{
	if (m_lpszStringBuffer) {
		m_lpszStringBuffer[0] = _T('\0');
		GetHeader()->m_iLinePtr = 0;
	}	
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::ResetLinePtr
//*---------------------------------------------------------------
// DESC : Initialize pointer delimiting a line of the string
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::ResetLinePtr()
{
	if (m_lpszStringBuffer) {
		GetHeader()->m_iLinePtr = 0;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::GetAt
//*---------------------------------------------------------------
// DESC : Return character at specified position in string data (TCHAR-based)
// PARM : 1 . nIndex - Specific position in string data
// RETV : character at specific position in string data
// PRGM : B4nFter
//*---------------------------------------------------------------
#pragma warning (disable:4310)
TCHAR CStringSet::GetAt(INT nIndex) const
{
	INT nLength = GetLength();
	if (nIndex >= 0 && nIndex < nLength) {
		return m_lpszStringBuffer[nIndex];
	}

	return (TCHAR)(0xFFFF);
}
#pragma warning (default:4310)


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::SetAt
//*---------------------------------------------------------------
// DESC : Set character at specified position in string data (TCHAR-based)
// PARM : 1 . nIndex - Specific position in string data
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::SetAt(INT nIndex, TCHAR cChar)
{
	INT nLength = GetLength();
	if (nIndex >= 0 && nIndex < nLength) {
		m_lpszStringBuffer[nIndex] = cChar;
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator[]
//*---------------------------------------------------------------
// DESC : Return character at specified position in string data, same as GetAt() (TCHAR-based)
// PARM : 1 . nIndex - Specific position in string data
// RETV : character at specific position in string data
// PRGM : B4nFter
//*---------------------------------------------------------------
TCHAR CStringSet::operator[](INT nIndex) const
{
	INT nLength = GetLength();
	if (nIndex >= 0 && nIndex < nLength-1) {
		return m_lpszStringBuffer[nIndex];
	}
	return _T('\0');
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator LPCTSTR
//*---------------------------------------------------------------
// DESC : LPCTSTR conversion operator of CStringSet class
// PARM : N/A
// RETV : LPCTSTR string data
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet::operator LPCTSTR() const
{
	if (m_lpszStringBuffer) {
		return const_cast<LPCTSTR>(m_lpszStringBuffer);
	}
	return NULL;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator=
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - String data to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator=(LPCTSTR pString)
{
	if (pString) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize = (INT)_tcslen(pString)+1;
		if (iSize > 0 && GetSize() < iSize) {
			aStringBuffer = ReAlloc(iSize);
		}
		if (aStringBuffer) {
			_tcsncpy(aStringBuffer, pString, GetSize()-1);
		}
	}
	else {
		Reset();
	}
	ResetLinePtr();
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator=
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - CStringSet object to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator=(CStringSet& pString)
{
	if (!pString.IsEmpty()) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize = pString.GetSize();
		if (iSize > 0 && GetSize() < iSize) {
			aStringBuffer = ReAlloc(iSize);
		}
		if (aStringBuffer) {
			_tcsncpy(aStringBuffer, pString.Get(), GetSize()-1);
		}
	}
	else {
		Reset();
	}
	ResetLinePtr();
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator=
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - Pointer to CStringSet object to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator=(CStringSet* pString)
{
	if (pString && !pString->IsEmpty()) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize = pString->GetSize();
		if (iSize > 0 && GetSize() < iSize) {
			aStringBuffer = ReAlloc(iSize);
		}
		if (aStringBuffer) {
			_tcsncpy(aStringBuffer, pString->Get(), GetSize()-1);
		}
	}
	else {
		Reset();
	}
	ResetLinePtr();
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator+=
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - String data to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator+=(LPCTSTR pString)
{
	if (pString) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize1 = (INT)_tcslen(pString)+1;
		INT iSize2 = GetSize();
		INT nLength = GetLength();
		if (iSize1 > iSize2 - nLength - 1) {
			aStringBuffer = ReAlloc(iSize1 + iSize2);
		}
		if (aStringBuffer) {
			_tcsncat(aStringBuffer, pString, GetSize()-1);
		}
	}
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator+=
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - CStringSet object (string data) to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator+=(CStringSet& pString)
{
	if (!pString.IsEmpty()) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize1 = pString.GetSize();
		INT iSize2 = GetSize();
		INT nLength = GetLength();
		if (iSize1 > iSize2 - nLength - 1) {
			aStringBuffer = ReAlloc(iSize1 + iSize2);
		}
		if (aStringBuffer) {
			_tcsncat(aStringBuffer, pString.Get(), GetSize()-1);
		}
	}
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator+=
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - Pointer to CStringSet object (string data) to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::operator+=(CStringSet* pString)
{
	if (pString && !pString->IsEmpty()) {
		LPTSTR aStringBuffer = m_lpszStringBuffer;
		INT iSize1 = pString->GetSize();
		INT iSize2 = GetSize();
		INT nLength = GetLength();
		if (iSize1 > iSize2 - nLength - 1) {
			aStringBuffer = ReAlloc(iSize1 + iSize2);
		}
		if (aStringBuffer) {
			_tcsncat(aStringBuffer, pString->Get(), GetSize()-1);
		}
	}
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator==
//*---------------------------------------------------------------
// DESC : == comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator==(LPCTSTR pString) const
{
	return (Compare(pString) == 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator==
//*---------------------------------------------------------------
// DESC : == comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator==(const CStringSet& pString) const
{
	return (Compare(pString.Get()) == 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator==
//*---------------------------------------------------------------
// DESC : == comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator==(const CStringSet* pString) const
{
	return (Compare(pString->Get()) == 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator!=
//*---------------------------------------------------------------
// DESC : != comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator!=(LPCTSTR pString) const
{
	return (Compare(pString) != 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator!=
//*---------------------------------------------------------------
// DESC : == comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator!=(const CStringSet& pString) const
{
	return (Compare(pString.Get()) != 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator!=
//*---------------------------------------------------------------
// DESC : != comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator!=(const CStringSet* pString) const
{
	return (Compare(pString->Get()) != 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<
//*---------------------------------------------------------------
// DESC : < comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<(LPCTSTR pString) const
{
	return (Compare(pString) < 0);
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<
//*---------------------------------------------------------------
// DESC : < comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<(const CStringSet& pString) const
{
	return (Compare(pString.Get()) < 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<
//*---------------------------------------------------------------
// DESC : < comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<(const CStringSet* pString) const
{
	return (Compare(pString->Get()) < 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>
//*---------------------------------------------------------------
// DESC : > comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>(LPCTSTR pString) const
{
	return (Compare(pString) > 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>
//*---------------------------------------------------------------
// DESC : > comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>(const CStringSet& pString) const
{
	return (Compare(pString.Get()) > 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>
//*---------------------------------------------------------------
// DESC : > comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>(const CStringSet* pString) const
{
	return (Compare(pString->Get()) > 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<=
//*---------------------------------------------------------------
// DESC : <= comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<=(LPCTSTR pString) const
{
	return (Compare(pString) <= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<=
//*---------------------------------------------------------------
// DESC : <= comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<=(const CStringSet& pString) const
{
	return (Compare(pString.Get()) <= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator<=
//*---------------------------------------------------------------
// DESC : <= comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator<=(const CStringSet* pString) const
{
	return (Compare(pString->Get()) <= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>=
//*---------------------------------------------------------------
// DESC : >= comparison between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>=(LPCTSTR pString) const
{
	return (Compare(pString) >= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>=
//*---------------------------------------------------------------
// DESC : >= comparison between object string data and specified string
// PARM : 1 . pString - CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>=(const CStringSet& pString) const
{
	return (Compare(pString.Get()) >= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::operator>=
//*---------------------------------------------------------------
// DESC : >= comparison between object string data and specified string
// PARM : 1 . pString - Pointer to CStringSet object (string data) to compare
// RETV : true - success / false - failure (uses bool instead of BOOL for use as STL container argument)
// PRGM : B4nFter
//*---------------------------------------------------------------
bool CStringSet::operator>=(const CStringSet* pString) const
{
	return (Compare(pString->Get()) >= 0);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Set
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - String data to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Set(LPCTSTR pString)
{
	return(operator=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Set
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - CStringSet object to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Set(CStringSet& pString)
{
	return(operator=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Set
//*---------------------------------------------------------------
// DESC : Assignment operator implementation
// PARM : 1 . pString - Pointer to CStringSet object to replace into CStringSet object
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Set(CStringSet* pString)
{
	return(operator=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Append
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - String data to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Append(LPCTSTR pString)
{
	return(operator+=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Append
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - CStringSet object (string data) to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Append(CStringSet& pString)
{
	return(operator+=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Append
//*---------------------------------------------------------------
// DESC : Increment operator implementation
// PARM : 1 . pString - Pointer to CStringSet object (string data) to append to CStringSet object's string data
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Append(CStringSet* pString)
{
	return(operator+=(pString));
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Tokenize
//*---------------------------------------------------------------
// DESC : Split into parts using delimiter characters as boundaries
// PARM : 1 . lpszTokens - Delimiters (character set, null-terminated with \0)
//			2 . iStartPtr - specific position in string data
// RETV : 
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::Tokenize(LPCTSTR lpszTokens, INT& iStartPtr)
{
	CStringSet aString;
	INT nLength = GetLength();
	if (IsEmpty() || iStartPtr < 0 || iStartPtr >= nLength) {
		return aString;
	}

	std::set<TCHAR> setTokens;
	INT iIndex;

	if (!lpszTokens || lpszTokens[0] == _T('\0')) {
		setTokens.insert(DF_DEFAULT_NEWLINE_TOKEN_CHAR);
	}
	else {
		for (iIndex = 0 ; lpszTokens[iIndex] != _T('\0') ; ++iIndex) {
			setTokens.insert(lpszTokens[iIndex]);
		}
	}
	
	for (iIndex = iStartPtr ; iIndex < nLength ; ++iIndex) {
		if (setTokens.find(m_lpszStringBuffer[iIndex]) != setTokens.end()) {
			break;
		}
	}

	if (iStartPtr < iIndex) {
		LPTSTR aStringBuffer = aString.ReAlloc(iStartPtr+iIndex+1);
		if (aStringBuffer) {
			::memcpy(aStringBuffer, m_lpszStringBuffer+iStartPtr, (iIndex - iStartPtr)*sizeof(TCHAR));
			aStringBuffer[iIndex - iStartPtr] = _T('\0');
		}
	}
	iStartPtr = ++iIndex;
	return(aString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Format
//*---------------------------------------------------------------
// DESC : Replace with a value in a specific string format
// PARM : 1 . lpszFormat - Specific string format
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::Format(LPCTSTR lpszFormat, ...)
{
	if (!lpszFormat) {
		return;
	}

	TCHAR szFormat[DF_MAX_FORMAT_TEMPBUFFER_SIZE] = {0,};

	va_list lpArgPtr;
	INT iChrWritten;

	va_start(lpArgPtr, lpszFormat);
	iChrWritten = VSNPRINTF(szFormat, COUNT_OF(szFormat), lpszFormat, lpArgPtr);
	DN_ASSERT(0 < iChrWritten,	"Invalid!");
	va_end(lpArgPtr);
	
	INT iLength = (INT)_tcslen(szFormat);
	if (iLength > 0) {
		LPTSTR aStringBuffer = ReAlloc(iLength+1);
		if (aStringBuffer) {
			_tcsncpy(aStringBuffer, szFormat, GetSize());
		}
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::TrimLeft
//*---------------------------------------------------------------
// DESC : Trim leading whitespace from string data
// PARM : N/A
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::TrimLeft()
{
	INT nLength = GetLength();
	if (nLength > 0) {
		INT iStartPtr = 0;
		for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
			if (m_lpszStringBuffer[iIndex] == _T(' ')) {
				iStartPtr = iIndex + 1;
			}
			else {
				break;
			}
		}
		if (iStartPtr > 1) {
			::memmove(m_lpszStringBuffer, m_lpszStringBuffer+iStartPtr, (nLength-iStartPtr)*sizeof(TCHAR));
			m_lpszStringBuffer[nLength-iStartPtr] = _T('\0');
		}
	}
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::TrimRight
//*---------------------------------------------------------------
// DESC : Trim trailing whitespace from string data
// PARM : N/A
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::TrimRight()
{
	INT nLength = GetLength();
	if (nLength > 0) {
		for (INT iIndex = nLength - 1 ; iIndex >= 0 ; --iIndex) {
			if (m_lpszStringBuffer[iIndex] == _T(' ')) {
				m_lpszStringBuffer[iIndex] = _T('\0');
			}
			else {
				break;
			}
		}
	}
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Trim
//*---------------------------------------------------------------
// DESC : Trim whitespace from both sides of string data
// PARM : N/A
// RETV : reference to the CStringSet object itself
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet& CStringSet::Trim()
{
	TrimLeft();
	TrimRight();
	return(*this);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Mid
//*---------------------------------------------------------------
// DESC : Extract substring from a specified middle range of string data
// PARM : 1 . nFirst - Start position to get string data
//			2 . nCount - number of characters to retrieve
// RETV : retrieved string
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::Mid(INT nFirst, INT nCount) const
{
	CStringSet aString;
	INT nLength = GetLength();
	if (nFirst >= 0 && 
		nCount > 0 &&
		!IsEmpty() &&
		nLength > nFirst
		) 
	{
		if (nFirst + nCount > nLength) {
			nCount = nLength - nFirst;
		}
		LPTSTR aStringBuffer1 = aString.ReAlloc(nCount+1);
		LPTSTR aStringBuffer2 = m_lpszStringBuffer;
		::memcpy(aStringBuffer1, aStringBuffer2+nFirst, nCount*sizeof(TCHAR));
		aStringBuffer1[nCount] = _T('\0');
	}
	return(aString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Mid
//*---------------------------------------------------------------
// DESC : Extract substring from a specified position to the end of string data
// PARM : 1 . nFirst - Start position to get string data
// RETV : retrieved string
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::Mid(INT nFirst) const
{
	CStringSet aString;
	INT nLength = GetLength();
	INT nCount = 0;
	if (nFirst >= 0 && 
		!IsEmpty() &&
		nLength > nFirst
		) 
	{
		nCount = nLength - nFirst;
		LPTSTR aStringBuffer1 = aString.ReAlloc(nCount+1);
		LPTSTR aStringBuffer2 = m_lpszStringBuffer;
		::memcpy(aStringBuffer1, aStringBuffer2+nFirst, nCount*sizeof(TCHAR));
		aStringBuffer1[nCount] = _T('\0');
	}
	return(aString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Left
//*---------------------------------------------------------------
// DESC : Extract specified number of characters from the left side of string data
// PARM : 1 . nCount - Number of characters to get
// RETV : retrieved string
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::Left(INT nCount) const
{
	CStringSet aString;
	INT nLength = GetLength();
	if (!IsEmpty() &&
		nCount > 0
		) 
	{
		if (nCount > nLength) {
			nCount = nLength;
		}
		LPTSTR aStringBuffer1 = aString.ReAlloc(nCount+1);
		LPTSTR aStringBuffer2 = m_lpszStringBuffer;
		::memcpy(aStringBuffer1, aStringBuffer2, nCount*sizeof(TCHAR));
		aStringBuffer1[nCount] = _T('\0');
	}
	return(aString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Right
//*---------------------------------------------------------------
// DESC : Extract specified number of characters from the right side of string data
// PARM : 1 . nCount - Number of characters to get
// RETV : retrieved string
// PRGM : B4nFter
//*---------------------------------------------------------------
CStringSet CStringSet::Right(INT nCount) const
{
	CStringSet aString;
	INT nLength = GetLength();
	if (!IsEmpty() &&
		nCount > 0
		) 
	{
		if (nCount > nLength) {
			nCount = nLength;
		}
		LPTSTR aStringBuffer1 = aString.ReAlloc(nCount+1);
		LPTSTR aStringBuffer2 = m_lpszStringBuffer;
		::memcpy(aStringBuffer1, aStringBuffer2+nLength-nCount, nCount*sizeof(TCHAR));
		aStringBuffer1[nCount] = _T('\0');
	}
	return(aString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsAlpha
//*---------------------------------------------------------------
// DESC : Check if string data is alphabetic
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsAlpha() const
{
	if (!m_lpszStringBuffer) {
		return FALSE;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return FALSE;
	}

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		if (!_istalpha(m_lpszStringBuffer[iIndex])) {
			return FALSE;
		}
	}
	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsNumeric
//*---------------------------------------------------------------
// DESC : Check if string data is numeric
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsNumeric() const
{
	if (!m_lpszStringBuffer) {
		return FALSE;
	}

	BOOL bHasNumeric = FALSE;
	INT nLength = GetLength();
	if (nLength <= 0) {
		return FALSE;
	}

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		if (iIndex == 0 && m_lpszStringBuffer[iIndex] == _T('-')) {
			continue;
		}
		if (!_istdigit(m_lpszStringBuffer[iIndex])) {
			return FALSE;
		}
		bHasNumeric = TRUE;
	}
	return bHasNumeric;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsNumeric
//*---------------------------------------------------------------
// DESC : Check if string data is numeric (including floating-point)
// PARM : 1 . bIsRealNumber - Returns whether the string data is a real number
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsNumeric(BOOL &bIsRealNumber) const
{
	if (!m_lpszStringBuffer) {
		return FALSE;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return FALSE;
	}

	BOOL bHasNumeric = FALSE;
	INT nDotCount = 0;	//  Number of decimal points ('.')
	bIsRealNumber = FALSE;

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		if (iIndex == 0 && m_lpszStringBuffer[iIndex] == _T('-')) {
			continue;
		}
		if (iIndex > 0 && m_lpszStringBuffer[iIndex] == _T('.')) {
			++nDotCount;
			continue;
		}
		if (!_istdigit(m_lpszStringBuffer[iIndex])) {
			return FALSE;
		}
		bHasNumeric = TRUE;
	}
	if (nDotCount > 1) {
		return FALSE;
	}
	if (nDotCount == 1) {
		bIsRealNumber = TRUE;
	}
	return bHasNumeric;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsAlNum
//*---------------------------------------------------------------
// DESC : Check if string data is alphanumeric
// PARM : N/A
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsAlNum() const
{
	if (!m_lpszStringBuffer) {
		return FALSE;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return FALSE;
	}

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		if (!_istalnum(m_lpszStringBuffer[iIndex])) {
			return FALSE;
		}
	}
	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::IsAlNum
//*---------------------------------------------------------------
// DESC : Check if string data is alphanumeric (including floating-point)
// PARM : 1 . bIsRealNumber - Returns whether the string data is a real number
// RETV : TRUE - success / FALSE - failure
// PRGM : B4nFter
//*---------------------------------------------------------------
BOOL CStringSet::IsAlNum(BOOL &bIsRealNumber) const
{
	if (!m_lpszStringBuffer) {
		return FALSE;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return FALSE;
	}

	INT nDotCount = 0;	//  Number of decimal points ('.')
	bIsRealNumber = FALSE;

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		if (_istalnum(m_lpszStringBuffer[iIndex])) {
			continue;
		}
		if (m_lpszStringBuffer[iIndex] == _T('.')) {
			nDotCount++;
			continue;
		}
		return FALSE;
	}
	if (nDotCount > 1) {
		return FALSE;
	}
	if (nDotCount == 1) {
		bIsRealNumber = TRUE;
	}
	return TRUE;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Compare
//*---------------------------------------------------------------
// DESC : Compare equality and ordering between object string data and specified string
// PARM : 1 . pString - String to compare
// RETV : 0: equal / negative: less / positive: greater / _NLSCMPERROR: error
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::Compare(LPCTSTR pString) const
{
	return _tcscmp(m_lpszStringBuffer, pString);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::CompareNoCase
//*---------------------------------------------------------------
// DESC : Compare equality and ordering between object string data and specified string (case-insensitive)
// PARM : 1 . pString - String to compare
// RETV : 0: equal / negative: less / positive: greater / _NLSCMPERROR: error
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::CompareNoCase(LPCTSTR pString) const
{
	return _tcsnicmp(m_lpszStringBuffer, pString, GetLength());
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::MakeUpper
//*---------------------------------------------------------------
// DESC : Convert string data to uppercase
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::MakeUpper()
{
	if (!m_lpszStringBuffer) {
		return;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return;
	}

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		m_lpszStringBuffer[iIndex] = (TCHAR)_totupper(m_lpszStringBuffer[iIndex]);
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::MakeLower
//*---------------------------------------------------------------
// DESC : Convert string data to lowercase
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::MakeLower()
{
	if (!m_lpszStringBuffer) {
		return;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return;
	}

	for (INT iIndex = 0 ; iIndex < nLength ; ++iIndex) {
		m_lpszStringBuffer[iIndex] = (TCHAR)_totlower(m_lpszStringBuffer[iIndex]);
	}
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::MakeReverse
//*---------------------------------------------------------------
// DESC : Reverse string data left-to-right
// PARM : N/A
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
VOID CStringSet::MakeReverse()
{
	if (!m_lpszStringBuffer) {
		return;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return;
	}

	std::reverse(&m_lpszStringBuffer[0], &m_lpszStringBuffer[nLength]);
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Find
//*---------------------------------------------------------------
// DESC : Find and return position of specified character within string data
// PARM : 1 . cChar - Character to find
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::Find(TCHAR cChar) const
{
	if (!m_lpszStringBuffer) {
		return -1;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return -1;
	}

	TCHAR *lpcChar = _tcschr(m_lpszStringBuffer, cChar);
	if (lpcChar) {
		return (INT)(lpcChar - m_lpszStringBuffer);
	}
	return -1;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::ReverseFind
//*---------------------------------------------------------------
// DESC : Find and return position of specified character within string data from the end
// PARM :	1 . cChar - 
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::ReverseFind(TCHAR cChar) const
{
	if (!m_lpszStringBuffer) {
		return -1;
	}

	INT nLength = GetLength();
	if (nLength <= 0) {
		return -1;
	}

	TCHAR *lpcChar = _tcsrchr(m_lpszStringBuffer, cChar);
	if (lpcChar) {
		return (INT)(lpcChar - m_lpszStringBuffer);
	}
	return -1;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Find
//*---------------------------------------------------------------
// DESC : Find and return position of specified character within string data starting from specified position
// PARM : 1 . cChar - Character to find
//			2 . nStart - starting position to search character
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::Find(TCHAR cChar, INT nStart) const
{
	if (!m_lpszStringBuffer) {
		return -1;
	}

	INT nLength = GetLength();
	if (nLength <= 0 || nStart < 0 || nLength-1 < nStart) {
		return -1;
	}

	TCHAR *lpcChar = _tcschr(m_lpszStringBuffer+nStart, cChar);
	if (lpcChar) {
		return (INT)(lpcChar - m_lpszStringBuffer);
	}
	return -1;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::FindOneOf
//*---------------------------------------------------------------
// DESC : Find and return position of any character from specified character set within string data
// PARM : 1 . lpszCharSet - Character set to find
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::FindOneOf(LPCTSTR lpszCharSet) const
{
	if (!m_lpszStringBuffer || !lpszCharSet) {
		return -1;
	}

	INT nLength1 = GetLength();
	if (nLength1 <= 0) {
		return -1;
	}
	INT nLength2 = (INT)_tcslen(lpszCharSet);
	if (nLength2 <= 0) {
		return -1;
	}

	std::set<TCHAR> setTokens;
	for (INT iIndex = 0 ; iIndex < nLength2 ; ++iIndex) {
		setTokens.insert(lpszCharSet[iIndex]);
	}

	for (INT iIndex = 0 ; iIndex < nLength1 ; ++iIndex) {
		if (setTokens.find(m_lpszStringBuffer[iIndex]) != setTokens.end()) {
			return iIndex;
		}
	}
	return -1;
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::FindOneOf
//*---------------------------------------------------------------
// DESC : Find and return position of any character from specified character set within string data (starting from specified position)
// PARM : 1 . lpszCharSet - Character set to find
//			2 . nStart - starting position to search
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::FindOneOf(LPCTSTR lpszCharSet, INT nStart) const
{
	if (!m_lpszStringBuffer || !lpszCharSet) {
		return -1;
	}

	INT nLength1 = GetLength();
	if (nLength1 <= 0 || nStart < 0 || nLength1-1 < nStart) {
		return -1;
	}
	INT nLength2 = (INT)_tcslen(lpszCharSet);
	if (nLength2 <= 0) {
		return -1;
	}

	std::set<TCHAR> setTokens;
	for (INT iIndex = 0 ; iIndex < nLength2 ; ++iIndex) {
		setTokens.insert(lpszCharSet[iIndex]);
	}

	for (INT iIndex = nStart ; iIndex < nLength1 ; ++iIndex) {
		if (setTokens.find(m_lpszStringBuffer[iIndex]) != setTokens.end()) {
			return iIndex;
		}
	}
	return -1;
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Find
//*---------------------------------------------------------------
// DESC : Find and return position of specified string within string data
// PARM : 1 . lpszSub - Substring to find
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::Find(LPCTSTR lpszSub) const
{
	if (!m_lpszStringBuffer || !lpszSub) {
		return -1;
	}

	INT nLength1 = GetLength();
	if (nLength1 <= 0) {
		return -1;
	}
	INT nLength2 = (INT)_tcslen(lpszSub);
	if (nLength2 <= 0) {
		return -1;
	}
	
	LPTSTR lpszStr = _tcsstr(m_lpszStringBuffer, lpszSub);
	if (lpszStr) {
		return (INT)(lpszStr - m_lpszStringBuffer);
	}
	return -1;	
}


//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CStringSet::Find
//*---------------------------------------------------------------
// DESC : Find and return position of specified string within string data starting from specified position
// PARM : 1 . lpszSub - Substring to find
//			2 . nStart - starting position to search string
// RETV : position of found character (returns -1 if not found)
// PRGM : B4nFter
//*---------------------------------------------------------------
INT CStringSet::Find(LPCTSTR lpszSub, INT nStart) const
{
	if (!m_lpszStringBuffer || !lpszSub) {
		return -1;
	}

	INT nLength1 = GetLength();
	if (nLength1 <= 0 || nStart < 0 || nLength1-1 < nStart) {
		return -1;
	}
	INT nLength2 = (INT)_tcslen(lpszSub);
	if (nLength2 <= 0) {
		return -1;
	}

	LPTSTR lpszStr = _tcsstr(m_lpszStringBuffer+nStart, lpszSub);
	if (lpszStr) {
		return (INT)(lpszStr - m_lpszStringBuffer);
	}
	return -1;	
}
