#pragma once;

#include "LiteHTMLCommon.h"
#include "LiteHTMLAttributes.h"

#pragma warning(push, 4)
#pragma warning (disable : 4290)	// C++ Exception Specification ignored

class CLiteHTMLTag
{
// Construction/Destruction
public:
	CLiteHTMLTag()
		: m_pcollAttr(NULL)
	{ }

	CLiteHTMLTag(CLiteHTMLTag &rSource, bool bCopy = false)
		:m_pcollAttr(NULL)
	{
		m_strTagName = rSource.m_strTagName;
		if (!bCopy)
		{
			m_pcollAttr = rSource.m_pcollAttr;
			rSource.m_pcollAttr = NULL;
		}
		else if (rSource.m_pcollAttr != NULL)
		{
			m_pcollAttr = new CLiteHTMLAttributes(*(rSource.m_pcollAttr), true);
		}
	}

	virtual ~CLiteHTMLTag()
		{ SAFE_DELETE_POINTER(m_pcollAttr); }

// Attributes
public:
	CStringW getTagName(void) const
		{ return (m_strTagName); }

	const CLiteHTMLAttributes* getAttributes(void) const
		{ return (m_pcollAttr);  }

// Parsing Helpers
public:
	// parses tag information from the given string
	UINT parseFromStr(LPCWSTR lpszString, 
					  bool &bIsOpeningTag, 
					  bool &bIsClosingTag, 
					  bool bParseAttrib = true);

// Data Members
private:
	CLiteHTMLAttributes	*m_pcollAttr;
	CStringW				m_strTagName;
};

inline UINT CLiteHTMLTag::parseFromStr(LPCWSTR lpszString, 
									   bool &bIsOpeningTag, 
									   bool &bIsClosingTag, 
									   bool bParseAttrib /* = true */)
{
	ASSERT(lpszString);
	//ASSERT(AfxIsValidString(lpszString));

	bool				bClosingTag = false;
	bool				bOpeningTag = false;
	CLiteHTMLAttributes	*pcollAttr = NULL;
	CStringW				strTagName;
	UINT				nRetVal = 0U, 
						nTemp = 0U;
	LPCWSTR				lpszBegin = lpszString;
	LPCWSTR				lpszEnd = NULL;

	// skip leading white-space characters
	while (::iswspace(*lpszBegin))
		//lpszBegin = ::_tcsinc(lpszBegin);
		lpszBegin++;

	// HTML tags always begin with a less-than symbol
	if (*lpszBegin != L'<')
		return (0U);
	
	// skip tag's opening delimeter '<'
	//lpszBegin = ::_tcsinc(lpszBegin);
	lpszBegin++;

	// optimization for empty opening tags
	if (*lpszBegin == L'>')
	{
		ASSERT(strTagName.IsEmpty());
		ASSERT(pcollAttr == NULL);
		ASSERT(!bClosingTag);
		nRetVal = UINT(lpszBegin - lpszString);
		goto LUpdateAndExit;
	}

	// tag names always begin with an alphabet
	if (!::iswalnum(*lpszBegin))
	{
		bClosingTag = (*lpszBegin == L'/');
		if (bClosingTag)
			//lpszBegin = ::_tcsinc(lpszBegin);
			lpszBegin++;
		else
			return (0U);
	}

	bOpeningTag = !bClosingTag;
	lpszEnd = lpszBegin;
	do
	{
		// tag name may contain letters (a-z, A-Z), digits (0-9), 
		// underscores '_', hyphen '-', colons ':', and periods '.'
		if ( (!::iswalnum(*lpszEnd)) && 
			 (*lpszEnd != L'-') && (*lpszEnd != L':') && 
			 (*lpszEnd != L'_') && (*lpszEnd != L'.') )
		{
			ASSERT(lpszEnd != lpszBegin);
			
			// only white-space characters, a null-character, a 
			// greater-than symbol, or a forward-slash can break 
			// a tag name
			if (*lpszEnd == NULL || ::iswspace(*lpszEnd) || 
				*lpszEnd == L'>' || 
				(*lpszEnd == L'/' && (!bClosingTag)) )
			{
				break;
			}

			return (0U);	// any other character will fail parsing process
		}

		//lpszEnd = ::_tcsinc(lpszEnd);
		lpszEnd++;
	}
	while(true);

	// store tag name for later use
	strTagName = CStringW(lpszBegin, UINT(lpszEnd - lpszBegin));

	// is this a closing tag?
	if (bClosingTag)
	{
		// in a closing tag, there can be only one symbol after 
		// tag name, i.e., the tag end delimeter '>'. Anything 
		// else will result in parsing failure
		if (*lpszEnd != L'>')
			return (0U);

		// skip tag's ending delimeter
		//lpszEnd = ::_tcsinc(lpszEnd);
		lpszEnd++;

		ASSERT(strTagName.GetLength());
		ASSERT(pcollAttr == NULL);
		nRetVal = UINT(lpszEnd - lpszString);
		goto LUpdateAndExit;
	}

	// tag contains attribute/value pairs?
	if (*lpszEnd != L'>')
	{
		lpszBegin = lpszEnd;
		lpszEnd = NULL;

		// skip white-space characters after tag name
		while (::iswspace(*lpszBegin))
			//lpszBegin = ::_tcsinc(lpszBegin);
			lpszBegin++;

		nTemp = 0U;
		if (bParseAttrib)	// parse attribute/value pairs?
		{
			ASSERT(pcollAttr == NULL);
			// instantiate collection ...
			if ((pcollAttr = new CLiteHTMLAttributes) == NULL)
			{
				//TRACE0("(Error) CLiteHTMLTag::parseFromStr: Out of memory.\n";
				return (0U);
			}

			// ... and delegate parsing process
			nTemp = pcollAttr->parseFromStr(lpszBegin);
		}

		if (nTemp == 0)	// attribute/value pair parsing is disabled? 
						//	- OR - 
						// attribute/value pairs could not be parsed?
		{
			SAFE_DELETE_POINTER(pcollAttr);
			if ((lpszEnd = ::wcsstr(lpszBegin, L"/>")) == NULL)
			{
				if ((lpszEnd = ::wcschr(lpszBegin, L'>')) == NULL)
					return (0U);
			}
		}
		else
		{
			lpszEnd = lpszBegin + nTemp;

			// skip white-space after attribute/value pairs
			while (::iswspace(*lpszEnd))
				//lpszEnd = ::_tcsinc(lpszEnd);
				lpszEnd++;
			
			// tag's ending delimeter could not be found?
			if (*lpszEnd == NULL)
			{
				SAFE_DELETE_POINTER(pcollAttr);
				return (0U);
			}
		}

		// a tag like this one: <BR/>
		if (*lpszEnd == L'/')
		{
			ASSERT(bOpeningTag);
			bClosingTag = true;
			//lpszEnd = ::_tcsinc(lpszEnd);
			lpszEnd++;
		}
	}

	// HTML tags always end with a greater-than '>' symbol
	if (*lpszEnd != L'>')
	{
		SAFE_DELETE_POINTER(pcollAttr);
		return (0U);
	}
	else
		//lpszEnd = ::_tcsinc(lpszEnd);
		lpszEnd++;

	nRetVal = UINT(lpszEnd - lpszString);
	goto LUpdateAndExit;	// just to show the flow-of-control

LUpdateAndExit:
	bIsClosingTag = bClosingTag;
	bIsOpeningTag = bOpeningTag;
	m_strTagName = strTagName;
	m_strTagName.TrimLeft();	
	m_strTagName.TrimRight();	// just-in-case!
	SAFE_DELETE_POINTER(m_pcollAttr);
	m_pcollAttr = pcollAttr;
	pcollAttr = NULL;

	return (nRetVal);
}

#pragma warning (default : 4290)	// C++ Exception Specification ignored
#pragma warning(pop)