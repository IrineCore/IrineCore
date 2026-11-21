#include "stdafx.h"
#include "DNMasterConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNExtManager.h"
#include "Util.h"
#include "Log.h"

// 20100110 MA LO auth info initialization supplement changes !!!
extern TLoginConfig g_Config;

CDNMasterConnectionManager* g_pMasterConnectionManager = NULL;

CDNMasterConnectionManager::CDNMasterConnectionManager(void): CConnectionManager()
{
}

CDNMasterConnectionManager::~CDNMasterConnectionManager(void)
{
}

CConnection* CDNMasterConnectionManager::AddConnection(const char *pIp, const USHORT nPort)
{
	CDNMasterConnection *pMasterCon = new CDNMasterConnection;
	if (!pMasterCon) return NULL;

	pMasterCon->SetIp(pIp);
	pMasterCon->SetPort(nPort);

	return pMasterCon;
}

void CDNMasterConnectionManager::GetChannelListByMapIdx(int nWorldSetID, TDBListCharData* pSelectCharData, sChannelInfo *ChannelArray, BYTE &cCount, TMapChannel *ChannelList)
{
#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( m_ConSync );
#endif
	CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(GetConnection(nWorldSetID, false));
	if( !pMasterCon ) 
		return;

	pMasterCon->GetChannelListByMapIdx(pSelectCharData, ChannelArray, cCount, ChannelList);	
}


UINT CDNMasterConnectionManager::GetWaitUserAmount()
{
	UINT nCount = 0;

#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( m_ConSync );
#endif

	for( UINT i=0 ; i<m_Connections.size() ; ++i )
	{
		CDNMasterConnection *pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon)
		{
			nCount += pMasterCon->GetWaitUSerCount();
		}
	}
	return nCount;
}

#if defined(PRE_MOD_SELECT_CHAR)
bool CDNMasterConnectionManager::SendDetachUserConnectionByWorldSetID(char cWorldSetID, UINT nAccountDBID, bool bIsReconnectLogin, bool bIsDuplicate/*=false*/, UINT nSessionID/*=0*/)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
bool CDNMasterConnectionManager::SendDetachUserConnectionByWorldSetID(char cWorldSetID, UINT nAccountDBID, BYTE cAccountLevel, bool bIsReconnectLogin, const BYTE * pMachineID/* = NULL*/, DWORD dwGRC/* = 0*/, 
																	  bool bIsDuplicate/*=false*/, UINT nSessionID/*=0*/, const char * pszIP/* = NULL*/)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	if (!CHECK_RANGE(cWorldSetID, 0, WORLDCOUNTMAX))
		return false;

	if (m_Connections.empty())
		return false;

	if (cWorldSetID == 0) // If login, simply true
		return true;

	// Otherwise, when properly set
#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( m_ConSync );
#endif
	CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(GetConnection(cWorldSetID, false));
	if( !pMasterCon ) 
		return false;

#if defined(PRE_MOD_SELECT_CHAR)
	pMasterCon->SendDetachUser(nAccountDBID, bIsReconnectLogin, bIsDuplicate, nSessionID);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	pMasterCon->SendDetachUser(nAccountDBID, cAccountLevel, bIsReconnectLogin, pMachineID, dwGRC, bIsDuplicate, nSessionID, pszIP);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return true;
}

void CDNMasterConnectionManager::SendConCountInfoAll(DWORD CurTick, int nServerID, int nCurConCount, int nMaxConCount)
{
	DN_ASSERT(0 != nServerID,		"Invalid!");
	DN_ASSERT(0 <= nCurConCount,	"Invalid!");
	DN_ASSERT(0 < nMaxConCount,		"Invalid!");

	if (m_Connections.empty()) return;

	{
		ScopeLock<CSyncLock> Lock(m_ConSync);

		for( UINT i=0 ; i<m_Connections.size() ; ++i )
		{
			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)m_Connections[i];
			if (pMasterCon && pMasterCon->GetActive() && 
				(LOGINCONCOUNTINFOTERM < GetTickTerm(pMasterCon->GetTick(CDNMasterConnection::eTickSendConCountInfo), CurTick)))
			{
				pMasterCon->SendConCountInfo(nServerID, nCurConCount, nMaxConCount);
				pMasterCon->SetTick(CDNMasterConnection::eTickSendConCountInfo, CurTick);
			}
		}
	}
}

void CDNMasterConnectionManager::SendDetachUserOtherLogin(UINT nAccountDBID, int nLoginServerID)
{
	if (m_Connections.empty()) return;

	{
		ScopeLock<CSyncLock> Lock(m_ConSync);

		for( UINT i=0 ; i<m_Connections.size() ; ++i )
		{
			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)m_Connections[i];
			if (pMasterCon && pMasterCon->GetActive() )
			{
				// Send to just one target
				pMasterCon->SendDetachUserOther(nAccountDBID, nLoginServerID);
				return;
			}
		}
	}
}

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
void CDNMasterConnectionManager::SendDetachUserbyIP(const char * pszIP)
{
	if (m_Connections.empty() || pszIP == NULL) return;

	{
		ScopeLock<CSyncLock> Lock(m_ConSync);

		for( UINT i=0 ; i<m_Connections.size() ; ++i )
		{
			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)m_Connections[i];
			if (pMasterCon && pMasterCon->GetActive() )
				pMasterCon->SendDetachUserbyIP(pszIP);
		}
	}
}
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
