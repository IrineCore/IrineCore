#pragma once

#include "Log.h"

class CIDGenerator
{
private:
	UINT m_nUserBaseID;
	UINT m_nNpcBaseID;

	UINT m_nUserConnectionID;
	UINT m_nUserConnectionMask;
	bool m_bUserConnectionSet;
	UINT m_nDBConnectionID;
	UINT m_nVillageConnectionID;
	UINT m_nGameConnectionID;
	UINT m_nMasterConnectionID;
	
	UINT m_nPartyID;
	USHORT m_nVoiceChannelID;

	UINT m_nUserID;
	UINT m_nNpcID;

	UINT m_nGameServerID;
	UINT m_nFieldID;

	CSyncLock m_IDLock;

public:
	CIDGenerator();
	~CIDGenerator();

	void SetUserConnectionID(UINT nID);
	void SetUserConnectionMask(BYTE btMask);
	UINT GetUserConnectionID(bool bDoGetRaw = false);
	bool IsUserConnectionSet() const { return m_bUserConnectionSet; }
	UINT GetDBConnectionID();
	UINT GetVillageConnectionID();
	UINT GetGameConnectionID();
	UINT GetMasterConnectionID();

	void SetPartyID(UINT nID);
	UINT GetPartyID();

	void SetVoiceChannelID(UINT nID);
	UINT GetVoiceChannelID();

	UINT GetUserID();
	UINT GetNpcID();

	UINT GetGameServerID();
	UINT GetFieldID();

	bool IsUser(UINT ID);
	bool IsNpc(UINT ID);
};

extern CIDGenerator g_IDGenerator;

// Generate serial number
//---------------------------------------------------------------------
//| 2byte(ServerID) | 2byte(Adder) | 4byte(Create Time) |
//---------------------------------------------------------------------

inline INT64 MakeSerial( short nServerID )
{
	static long Adder = rand();
	
	time_t CreateTime;
	time(&CreateTime);

	long Local = static_cast<short>(InterlockedIncrement( &Adder ));

	INT64 Serial = (static_cast<INT64>(nServerID&0xFFFF)<<48) | (static_cast<INT64>(Local&0xFFFF)<<32) | (CreateTime&0xFFFFFFFF);
	
	return Serial;
}

inline INT64 MakeCashSerial( short nServerID, UINT nPeriodSecond)
{
	static long Adder = rand();
	
	time_t CreateTime;
	time(&CreateTime);
	CreateTime += nPeriodSecond;	// Add remaining duration (seconds) of cash item

	long Local = static_cast<short>(InterlockedIncrement( &Adder ));

	INT64 Serial = (static_cast<INT64>(nServerID&0xFFFF)<<48) | (static_cast<INT64>(Local&0xFFFF)<<32) | (CreateTime&0xFFFFFFFF);

	return Serial;
}


// Function that forces a NULL-terminated string.
// Added nDestLen due to a previous overrun issue.
// Check because string may be truncated if nLen >= nDestLen. // Kimbap
inline void _wcscpy( WCHAR* pDest, const int nDestLen, const WCHAR* pSrc, const int nLen )
{	
	if(pDest == pSrc) return;
	memset( pDest, 0, nDestLen*sizeof(WCHAR) );
	if( nLen <= 0 )
		return;
	wmemcpy_s( pDest, nDestLen, pSrc, nDestLen < nLen ? nDestLen : nLen );
	if( nLen >= nDestLen )
	{
		pDest[nDestLen-1] = '\0';
	}
	else
		pDest[nLen] = '\0';
}

inline void _strcpy( char* pDest, const int nDestLen, const char* pSrc, const int nLen )
{	
	if(pDest == pSrc) return;
	memset( pDest, 0, nDestLen );
	if( nLen <= 0 )
		return;
	memcpy_s( pDest, nDestLen, pSrc, nDestLen < nLen ? nDestLen : nLen );
	if( nLen >= nDestLen )
	{
		if( nLen > nDestLen )
			g_Log.Log( LogType::_ERROR, "_strcpy err %s\n", pSrc );
		pDest[nDestLen-1] = '\0';
	}
	else
		pDest[nLen] = '\0';
}

#include <map>

#if !defined( _SERVICEMANAGER )
template <class _Tx> struct less_str : std::binary_function <_Tx, _Tx, bool>
{
public:
	bool operator() (const _Tx &a, const _Tx &b) const { return __wcsicmp_l(a, b) < 0 ? true : false; }
};

template <class _Tx>
class map_wstr : public std::map <const WCHAR*, _Tx, less_str <const WCHAR*> >
{
};
#endif	// #if !defined( _SERVICEMANAGER )

#if !defined(_GAMESERVER)
inline unsigned long _inet_addr(const char * cp)
{
	int s_b[4] = { 0, 0, 0, 0};
	sscanf(cp, "%d.%d.%d.%d", &s_b[0], &s_b[1], &s_b[2], &s_b[3]);
	unsigned char b[4] = {
		static_cast<unsigned char>(s_b[0]),
		static_cast<unsigned char>(s_b[1]),
		static_cast<unsigned char>(s_b[2]),
		static_cast<unsigned char>(s_b[3])
	};
	uint32_t addr = 0;
	memcpy(&addr, b, sizeof(addr));
	return static_cast<unsigned long>(addr);
}

inline void _inet_addr(unsigned long i, char * p)
{
	sprintf(p, "%d.%d.%d.%d", ((unsigned char*)&i)[0], ((unsigned char*)&i)[1], ((unsigned char*)&i)[2], ((unsigned char*)&i)[3]);
}

#endif	// #if !defined(_GAMESERVER)

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMisc::GetNextDay
//*---------------------------------------------------------------
// DESC : Calculate the date after elapsed time from a specific date
// PARM : 1 . stTime - [IN/OUT] SYSTEMTIME struct variable to input time data to convert and receive result
//			2. iAfterDay - day offset (- for past / + for future)
//			3. iAfterHour - hours to shift (- value is earlier / + value is later)
//			4. iAfterMin - minutes to shift (- value is earlier / + value is later)
//			5. iAfterSec - Seconds to offset (- value for past / + value for future)
// RETV : N/A
// PRGM : B4nFter
//*---------------------------------------------------------------
inline VOID GetNextDay(SYSTEMTIME &stTime, INT iAfterDay, INT iAfterHour, INT iAfterMin, INT iAfterSec)
{
	INT64 i64;
	FILETIME stTempTime;

	stTime.wSecond = stTime.wMilliseconds = stTime.wDayOfWeek = 0;

	::SystemTimeToFileTime(&stTime,&stTempTime);
	i64=(((INT64)stTempTime.dwHighDateTime) << 32) + stTempTime.dwLowDateTime;
	i64 = i64 + (INT64)864000000000*(INT64)iAfterDay + (INT64)36000000000*(INT64)iAfterHour + (INT64)600000000*(INT64)iAfterMin + (INT64)10000000*(INT64)iAfterSec;
	stTempTime.dwHighDateTime = (DWORD)(i64 >> 32);
	stTempTime.dwLowDateTime = (DWORD)(i64 & 0xffffffff);
	::FileTimeToSystemTime(&stTempTime, &stTime);
}

//*---------------------------------------------------------------
// TYPE : FUNCTION
// NAME : CMisc::GetTickTerm
//*---------------------------------------------------------------
// DESC : Get the interval between two Ticks
// PARM : 1 . dwOldTick - Previous Tick
//			2. dwCurTick - current Tick
// RETV : Difference between two tick counts (positive)
// PRGM : B4nFter
// P.S.>
//		- Simply calculates the difference between previous and current tick counts, so it is always positive
//		- Note that this result only represents the difference between dwOldTick and dwCurTick, not a magnitude comparison
//		- For the above case, record only the tick count at that time in dwOldTick (::GetTickCount())
//*---------------------------------------------------------------
inline DWORD GetTickTerm(DWORD dwOldTick, DWORD dwCurTick)
{
	return((dwCurTick >= dwOldTick)?(dwCurTick - dwOldTick):(ULONG_MAX - dwOldTick + dwCurTick));
}

inline bool CheckLastSpace(const TCHAR * pwords)
{
	if (pwords == NULL) return true;

	int nLen = (int)_tcslen(pwords);
	if (nLen <= 0 || nLen == 1) return false;
	if (pwords[nLen - 1] == _T(' ') || pwords[0] == _T(' '))
		return true;
	return false;
}

std::wstring MakeSecondPasswordString( char *szVersion, const int nSeed, const int nValue[] );

#define MAKELONG64(a, b)      ((INT64)(((DWORD)((INT64)(a) & 0xffffffff)) | ((INT64)((DWORD)((INT64)(b) & 0xffffffff))) << 32))
#define LODWORD(l)           ((DWORD)((INT64)(l) & 0xffffffff))
#define HIDWORD(l)           ((DWORD)((INT64)(l) >> 32))

const int MD5_DIGEST_LEN = 16;

#include "md5.h"
inline void MakeMD5String(std::string& result, unsigned char* pDigest)
{
	result.clear();
	result.resize(MD5_DIGEST_LEN * 2 + 1);

	int di = 0;
	for (; di < MD5_DIGEST_LEN; ++di)
		sprintf(&result[di * 2], "%02x", pDigest[di]);
}

inline void MD5Hash(const char * pin, int nlen, std::string &out)
{
	md5_state_s md5struct;
	memset(&md5struct, 0, sizeof(md5_state_s));

	md5_init(&md5struct);
	md5_append(&md5struct, (const md5_byte_t*)pin, nlen);

	md5_byte_t digest[MD5_DIGEST_LEN];
	memset(digest, 0, sizeof(digest));
	md5_finish(&md5struct, digest);
 
	MakeMD5String(out, digest);
}

#include "SHA256.h"
inline void SHA256Hash(const char* pin, int nlen, unsigned char* pDigest)
{
	CSha256 c = {0,};
	//Byte digest[SHA256_DIGEST_SIZE] = {0,};

	Sha256_Init(&c);
	Sha256_Update(&c, (Byte*)pin, nlen);
	Sha256_Final(&c, pDigest);
}

#if defined( PRE_ADD_CHARACTERCHECKSUM )
UINT MakeCharacterCheckSum( INT64 biCharacterDBID, char cLevel, int nExp, INT64 biCoin, INT64 biWarehouseCoin );
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )