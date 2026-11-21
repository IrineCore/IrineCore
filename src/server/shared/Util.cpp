#include "stdafx.h"
#include "Util.h"
#include "StringUtil.h"
#include "MtRandom.h"
#if !defined(_GAMESERVER) && !defined(_VILLAGESERVER)
const int MAX_SESSION_COUNT = 2;	//compile time warning
#endif

CIDGenerator g_IDGenerator;

CIDGenerator::CIDGenerator()
{
	m_nUserBaseID = 0x00000000;
	m_nNpcBaseID = 0x80000000;

	m_nUserConnectionID = 10000;
	m_nUserConnectionMask = 0;
	m_bUserConnectionSet = false;
	m_nDBConnectionID = m_nVillageConnectionID = 0;
	m_nUserID = m_nNpcID = m_nPartyID = m_nVoiceChannelID = 0;
	m_nGameServerID = m_nFieldID = 0;
	m_nGameConnectionID = m_nMasterConnectionID = 0;
}

CIDGenerator::~CIDGenerator()
{
	
}

void CIDGenerator::SetUserConnectionID(UINT nID)
{
	m_IDLock.Lock();
	m_nUserConnectionID = nID;
	m_IDLock.UnLock();
}

void CIDGenerator::SetUserConnectionMask(BYTE btMask)
{
	DN_ASSERT(0 != btMask,	"Check!");

	m_nUserConnectionMask = (btMask << 24);
	m_bUserConnectionSet = true;
}

UINT CIDGenerator::GetUserConnectionID(bool bDoGetRaw)
{
	m_IDLock.Lock();
	if (0x00FFFFFF <= m_nUserConnectionID) m_nUserConnectionID = DEFAULTUSERSESSIONID;
	// UINT ID = ++m_nUserConnectionID;
	UINT nID = m_nUserConnectionID;
	if (!bDoGetRaw)
		nID = (m_nUserConnectionID += 5);
	m_IDLock.UnLock();

	if (bDoGetRaw)
		return nID;

	return(nID | m_nUserConnectionMask);
}

UINT CIDGenerator::GetDBConnectionID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nDBConnectionID;
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetVillageConnectionID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nVillageConnectionID;
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetGameConnectionID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nGameConnectionID;
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetMasterConnectionID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nMasterConnectionID;
	m_IDLock.UnLock();
	return ID;
}

void CIDGenerator::SetPartyID(UINT nID)
{
	m_IDLock.Lock();
	m_nPartyID = nID;
	if (m_nPartyID == 2000000000) m_nPartyID = 0;
	m_IDLock.UnLock();
}

UINT CIDGenerator::GetPartyID()
{
	m_IDLock.Lock();
	UINT nID = ++m_nPartyID;
	m_IDLock.UnLock();
	return nID;
}

void CIDGenerator::SetVoiceChannelID(UINT nID)
{
	m_IDLock.Lock();
	m_nVoiceChannelID = nID;
	m_IDLock.UnLock();
}

UINT CIDGenerator::GetVoiceChannelID()
{
	m_IDLock.Lock();
	USHORT nID = ++m_nVoiceChannelID;
	if (m_nVoiceChannelID == 0) m_nVoiceChannelID = 1;
	m_IDLock.UnLock();
	return nID;
}

UINT CIDGenerator::GetUserID()
{
	m_IDLock.Lock();
	UINT ID = m_nUserBaseID + (++m_nUserID);
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetNpcID()
{
	m_IDLock.Lock();
	UINT ID = m_nNpcBaseID + (++m_nNpcID);
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetGameServerID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nGameServerID;
	m_IDLock.UnLock();
	return ID;
}

UINT CIDGenerator::GetFieldID()
{
	m_IDLock.Lock();
	UINT ID = ++m_nFieldID;
	m_IDLock.UnLock();
	return ID;
}

bool CIDGenerator::IsUser(UINT ID)
{
	if (ID < 0x80000000) return true;
	return false;
}

bool CIDGenerator::IsNpc(UINT ID)
{
	if (ID >= 0x80000000) return true;
	return false;
}

std::wstring MakeSecondPasswordString( char *szVersion, const int nSeed, const int nValue[] )
{
	std::wstring szResult;
	if( nSeed == 0 ) return szResult;
	if( nValue[0] == -1 || nValue[1] == -1 || nValue[2] == -1 || nValue[3] == -1 ) return szResult;

	CMtRandom Random;

	std::vector<std::string> tokens;
	TokenizeA( std::string(SecondPassword::GetVersion()), tokens, "." );

	int nKey = atoi(tokens[0].c_str()) + atoi(tokens[1].c_str());
	if( nKey % 2 ) nKey = -nKey;

	int nTempSeed = nSeed + nKey;
	Random.srand( nTempSeed );
	std::vector<int> nVecList, nVecList2;
	int nArray[10] = { 7, 8, 9, 4, 5, 6, 1, 2, 3, 0 };
	for( int i=0; i<10; i++ ) nVecList2.push_back(nArray[i]);

	while( !nVecList2.empty() ) {
		int nOffset = Random.rand( (int)nVecList2.size() );
		nVecList.push_back( nVecList2[nOffset] );
		nVecList2.erase( nVecList2.begin() + nOffset );
	}

	for( int i=0; i<4; i++ ) {
		WCHAR wszTemp[4] = { 0, };

		// Exception handling
		if( nValue[i] >= static_cast<int>(nVecList.size()) || nValue[i] < 0 )
			return szResult;

		_itow_s( nVecList[nValue[i]], wszTemp, 4, 10 );
		szResult += wszTemp;
	}
	return szResult;
}

#if defined( PRE_ADD_CHARACTERCHECKSUM )
UINT MakeCharacterCheckSum( INT64 biCharacterDBID, char cLevel, int nExp, INT64 biCoin, INT64 biWarehouseCoin )
{
	const BYTE cCharSize = sizeof(char) * 2 + 1;
	const BYTE cIntSize = sizeof(int) * 2 + 1;
	const BYTE cBigIntSize = sizeof(INT64) * 2 + 1;

	char szCharacterDBID[cBigIntSize];
	char szLevel[cCharSize];
	char szExp[cIntSize];
	char szCoin[cBigIntSize];
	char szWarehouseCoin[cBigIntSize];

	sprintf_s(szCharacterDBID, "%016I64x", biCharacterDBID );
	sprintf_s(szLevel, "%02x", cLevel);
	sprintf_s(szExp, "%08I32x", nExp);	
	sprintf_s(szCoin, "%016I64x", biCoin );
	sprintf_s(szWarehouseCoin, "%016I64x", biWarehouseCoin );

	std::string strPin;
	strPin += szCharacterDBID;
	strPin += szLevel;
	strPin += szExp;
	strPin += szCoin;
	strPin += szWarehouseCoin;

	std::string strMD5;
	MD5Hash(strPin.c_str(), static_cast<int>(strPin.size()), strMD5);
	strMD5.resize( sizeof(int) * 2 ); // Store the first 8 hex digits in an int variable.

	UINT uiCheckSum = 0;
	std::stringstream stream(strMD5.c_str());
	stream >> std::hex >> uiCheckSum;

	return uiCheckSum;
}
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )