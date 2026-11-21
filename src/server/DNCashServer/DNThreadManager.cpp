#include "stdafx.h"
#include "DNThreadManager.h"
#include "DNIocpManager.h"
#include "DNConnection.h"
#include "DNUserDeleteManager.h"

#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
#include "DNSQLManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"
#include "Log.h"
extern TCashConfig g_Config;
#endif		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL)

#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM


CDNThreadManager *g_pThreadManager = NULL;

CDNThreadManager::CDNThreadManager(void): CThreadManager()
{
}

CDNThreadManager::~CDNThreadManager(void)
{
}

#if defined(PRE_DEL_ONNULL)
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pBuffer, TThreadData* pThreadData)
#else
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pBuffer, int nThreadID)
#endif //#if defined(PRE_DEL_ONNULL)
{
#if defined(PRE_DEL_ONNULL)
	int nThreadID = pThreadData->nThreadID;
	if( pSocketContext == reinterpret_cast<CSocketContext*>(reinterpret_cast<void*>(DNProcessCheck::HeartbeatKey)))		
	{
		DoUpdate(pThreadData->nThreadID, pThreadData->nLastHearbeatTick);
		return;
	}
#endif // #if defined(PRE_DEL_ONNULL)
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
			CDNConnection *pCon = (CDNConnection*)pSocketContext->GetParam();
			if (!pCon) return;

			pCon->FlushDBData( nThreadID );
		}
		break;
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	case CONNECTIONKEY_LOG:
		{
			CConnection *pCon = (CConnection*)pSocketContext->GetParam();
			if (!pCon) return;

			pCon->FlushRecvData(0);
		}
		break;
#endif
	}
	g_pIocpManager->DetachSocket(pSocketContext, L"OnCalled");
}

#if defined(PRE_DEL_ONNULL)
void CDNThreadManager::DoUpdate(int nThreadID, ULONG &nHeartBeatTick)
#else // #if defined(PRE_DEL_ONNULL)
#if defined(PRE_ADD_HEARTBEAT_TO_SQL) && defined(_CASHSERVER)
void CDNThreadManager::OnNulled(int nThreadID, ULONG &nHeartBeatTick)
#else		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL) && defined(_CASHSERVER)
void CDNThreadManager::OnNulled(int nThreadID)
#endif		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL) && defined(_CASHSERVER)
#endif //#if defined(PRE_DEL_ONNULL)
{
#if !defined(PRE_DEL_ONNULL) // Moved this to ReconnectThread.
	if (g_pUserDeleteManager) {
		g_pUserDeleteManager->DoUpdate();
	}
#endif 

#ifdef PRE_ADD_LIMITED_CASHITEM
	if (g_pLimitedCashItemRepository)
	{
		g_pLimitedCashItemRepository->DoUpdate(nThreadID);
	}
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	if (g_pSQLManager)
	{
		ULONG nCurTick = timeGetTime();
		if (nCurTick - nHeartBeatTick >= DNProcessCheck::SQLHeartbeatInterval)
		{
			nHeartBeatTick = timeGetTime();

			CDNSQLMembership * pMembership = g_pSQLManager->FindMembershipDB(nThreadID);
			if (pMembership)
			{
				if (pMembership->QueryHeartbeat() != ERROR_NONE)
					_DANGER_POINT();
			}

			for (int i = 0; i < g_Config.nWorldDBCount; i++)
			{
				CDNSQLWorld * pWorld = g_pSQLManager->FindWorldDB(nThreadID, g_Config.WorldDB[i].nWorldSetID);
				if (pWorld)
				{
					if (pWorld->QueryHeartbeat() != ERROR_NONE)
						_DANGER_POINT();
				}
			}
		}
	}
#endif		//#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
}

