#pragma once

#include <map>

#define UNICODE_TYPE_ALL_START          0x0080
#define UNICODE_TYPE_ALL_END            0xFFFF
#define UNICODE_TYPE_NUM_START          0x0030
#define UNICODE_TYPE_NUM_END            0x0039
#define UNICODE_TYPE_ENG_BIG_START      0x0041
#define UNICODE_TYPE_ENG_BIG_END        0x005A
#define UNICODE_TYPE_ENG_SMALL_START    0x0061
#define UNICODE_TYPE_ENG_SMALL_END      0x007A
#define UNICODE_TYPE_SPECIAL_START      0x2100
#define UNICODE_TYPE_SPECIAL_END        0x2AFF

#define IS_IN_RANGE(nMin,nMax,nValue)   ((nMin) <= (nValue) && (nMax) >= (nValue))

enum STRING_CHECK_TYPE
{
	CHECK_TYPE_ALL      = 1,
	CHECK_TYPE_NUMBER   = (1<<1),
	CHECK_TYPE_ENG      = (1<<2),
	CHECK_TYPE_SPECIAL  = (1<<4),
};

#define ALLOW_STRING_DEFAULT_ENG        (CHECK_TYPE_ENG | CHECK_TYPE_NUMBER)
#define ALLOW_STRING_CHARACTERNAME_ENG  (CHECK_TYPE_ENG)
#define ALLOW_STRING_DEFAULT            (ALLOW_STRING_DEFAULT_ENG | CHECK_TYPE_ALL)

struct MIN_MAX
{
	wchar_t nMin, nMax;
	MIN_MAX(wchar_t min, wchar_t max) : nMin(min), nMax(max) {}
	MIN_MAX() : nMin(0), nMax(0) {}
};

typedef std::multimap<STRING_CHECK_TYPE, MIN_MAX>                   HASH_MAP_ALLOW_STRING;
typedef std::multimap<STRING_CHECK_TYPE, MIN_MAX>::const_iterator   HASH_MAP_ALLOW_STRING_I;

class CDNCountryUnicodeSet
{
private:
	HASH_MAP_ALLOW_STRING m_mapAllowString;

public:
	CDNCountryUnicodeSet();
	~CDNCountryUnicodeSet();

	bool Check(const wchar_t* pChr, DWORD dwCheckType = ALLOW_STRING_DEFAULT);

private:
	bool IsExist(wchar_t pChr, STRING_CHECK_TYPE eType);
};

extern CDNCountryUnicodeSet g_CountryUnicodeSet;
