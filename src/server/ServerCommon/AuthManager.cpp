#include "stdafx.h"
#include "AuthManager.h"
#include "DNPacket.h"
#include "Util.h"
#include "DNConfig.h"
#include "Log.h"

#if defined(_LOGINSERVER)
extern TLoginConfig g_Config;
#elif defined(_VILLAGESERVER)
extern TVillageConfig g_Config;
#elif defined(_GAMESERVER)
extern TGameConfig g_Config;
#endif	// _GAMESERVER

CAuthManager::CAuthManager() : m_bResetAuthServer(false), m_ServerID(0), m_ResetAuthServerTick(0)
{
}

bool CAuthManager::Init()
{
	m_ServerID = CalculateServerID();
	DN_ASSERT(0 != m_ServerID,	"Check!");

	g_Config.nManagedID = m_ServerID;
	g_Log.SetServerID(g_Config.nManagedID);

	return true;
}

void CAuthManager::Run(DWORD CurTick)
{
	if (!IsResetAuthServer() &&
		(RESETAUTHSERVERTERM < GetTickTerm(m_ResetAuthServerTick, CurTick)))
	{
		m_ResetAuthServerTick = CurTick;

		QueryResetAuthServer();
	}
}

INT64 CAuthManager::GetCertifyingKey()
{
	INT64 biCertifyingKey = 0;

	for (int iIndex = 0 ; 4 > iIndex ; ++iIndex) {
		WORD wRand = rand();
		wRand = (wRand)?(wRand):(1);
		biCertifyingKey |= static_cast<INT64>(wRand) << (16 * iIndex);
	}

	return(biCertifyingKey);
}

const int CAuthManager::CalculateServerID()
{
	int iServerID = 0;

	in_addr aInAddr;
	::memset(&aInAddr, 0, sizeof(aInAddr));
	GetLocalIp(&aInAddr);
	WORD aLowID = ::htons(static_cast<WORD>(aInAddr.s_imp));
	DN_ASSERT(0 != aLowID,	"Check!");

#if defined(_LOGINSERVER)
	iServerID = MAKELONG(aLowID, MAKEWORD((g_Config.nClientAcceptPort), SERVERTYPE_LOGIN+1));
#elif defined(_VILLAGESERVER)
	iServerID = MAKELONG(aLowID, MAKEWORD((g_Config.nVillageID), SERVERTYPE_VILLAGE+1));
#elif defined(_GAMESERVER)
	iServerID = MAKELONG(aLowID, MAKEWORD(0, SERVERTYPE_GAME+1));
#else
	iServerID = 0;
#endif
	DN_ASSERT(0 != iServerID,	"Check!");

	return iServerID;
}

char* CAuthManager::GetLocalIp(OUT in_addr* pInAddr)
{
	WSADATA WsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &WsaData);
	if (0 > iResult) {
		DN_RETURN(NULL);
	}

	char Name[256] = { 0, };

	gethostname(Name, sizeof(Name));
	PHOSTENT host = gethostbyname(Name);
	if (host) {
		if (pInAddr) {
			memcpy(pInAddr, host->h_addr_list[0], sizeof(in_addr));
		}
		return inet_ntoa(*(struct in_addr*)*host->h_addr_list);
	}

	WSACleanup();

	return NULL;
}
