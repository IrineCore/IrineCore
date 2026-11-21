#include "stdafx.h"
#include "DNCountryUnicodeSet.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDNCountryUnicodeSet g_CountryUnicodeSet;

CDNCountryUnicodeSet::CDNCountryUnicodeSet()
{
	m_mapAllowString.insert(HASH_MAP_ALLOW_STRING::value_type(CHECK_TYPE_ALL,     MIN_MAX(UNICODE_TYPE_ALL_START, UNICODE_TYPE_ALL_END)));
	m_mapAllowString.insert(HASH_MAP_ALLOW_STRING::value_type(CHECK_TYPE_NUMBER,  MIN_MAX(UNICODE_TYPE_NUM_START, UNICODE_TYPE_NUM_END)));
	m_mapAllowString.insert(HASH_MAP_ALLOW_STRING::value_type(CHECK_TYPE_ENG,     MIN_MAX(UNICODE_TYPE_ENG_BIG_START, UNICODE_TYPE_ENG_BIG_END)));
	m_mapAllowString.insert(HASH_MAP_ALLOW_STRING::value_type(CHECK_TYPE_ENG,     MIN_MAX(UNICODE_TYPE_ENG_SMALL_START, UNICODE_TYPE_ENG_SMALL_END)));
	m_mapAllowString.insert(HASH_MAP_ALLOW_STRING::value_type(CHECK_TYPE_SPECIAL, MIN_MAX(UNICODE_TYPE_SPECIAL_START, UNICODE_TYPE_SPECIAL_END)));
}

CDNCountryUnicodeSet::~CDNCountryUnicodeSet()
{
	m_mapAllowString.clear();
}

bool CDNCountryUnicodeSet::Check(const wchar_t* pChr, DWORD dwCheckType)
{
	if (pChr)
	{
		int nLength = (int)wcslen(pChr);
		for (int i = 0; i < nLength; i++)
		{
			wchar_t szChar = pChr[i];
			if ((dwCheckType & CHECK_TYPE_ALL)     && IsExist(szChar, CHECK_TYPE_ALL))     continue;
			if ((dwCheckType & CHECK_TYPE_NUMBER)  && IsExist(szChar, CHECK_TYPE_NUMBER))  continue;
			if ((dwCheckType & CHECK_TYPE_ENG)     && IsExist(szChar, CHECK_TYPE_ENG))     continue;
			if ((dwCheckType & CHECK_TYPE_SPECIAL) && IsExist(szChar, CHECK_TYPE_SPECIAL)) continue;
			return false;
		}
	}
	return true;
}

bool CDNCountryUnicodeSet::IsExist(wchar_t pChr, STRING_CHECK_TYPE eType)
{
	HASH_MAP_ALLOW_STRING_I iter = m_mapAllowString.lower_bound(eType);
	for (; iter != m_mapAllowString.upper_bound(eType); iter++)
	{
		if (IS_IN_RANGE(iter->second.nMin, iter->second.nMax, pChr))
			return true;
	}
	return false;
}
