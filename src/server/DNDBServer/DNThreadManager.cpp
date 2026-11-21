#include "stdafx.h"
#include "DNIocpManager.h"
#include "DNThreadManager.h"
#include "DNConnection.h"
#include "Log.h"
#include "DNProtocol.h"
#include "SpinBuffer.h"
#include "DNSQLConnectionManager.h"
#include "DNSQLMembership.h"
#include "DNSQLWorld.h"

CDNThreadManager g_ThreadManager;
extern TDBConfig g_Config;

CDNThreadManager::CDNThreadManager(void): CThreadManager()
{
}

CDNThreadManager::~CDNThreadManager(void)
{
}

#if defined(PRE_DEL_ONNULL)
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pTempBuffer, TThreadData* pThreadData)
#else
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pTempBuffer, int nThreadID)
#endif
{
#if defined(PRE_DEL_ONNULL)
	int nThreadID = pThreadData->nThreadID;
#endif
	CSpinBuffer* pSpinBuffer = g_pIocpManager->GetSpinBuffer( nThreadID );

	ScopeSpinBufferSwitch Scope( pSpinBuffer );

	char*	pBuffer	= Scope.pGetBuffer();
	UINT32	uiSize	= Scope.uiGetSize();

	std::vector<int> MainCmd;
	std::vector<int> SubCmd;
	std::vector<DWORD> Elapsed;	
	std::vector<UINT> AccountDBID;	

	while( uiSize )
	{
		memcpy( &pSocketContext, pBuffer, sizeof(CSocketContext*) );
		pBuffer += sizeof(CSocketContext*);
		uiSize  -= sizeof(CSocketContext*);

		if( pSocketContext == reinterpret_cast<CSocketContext*>(reinterpret_cast<void*>(DNProcessCheck::Key)) )
		{
			UINT uiTime;
			memcpy( &uiTime, pBuffer, sizeof(UINT) );
			UINT uiCurTime = timeGetTime();

			if( uiCurTime >= uiTime+DNProcessCheck::DisconnectInterval )
			{
				// If execution reaches here, DB M/W cannot handle the load just like during FCBT. Shut down the server immediately and call GG.
				// Check for timeGetTime 49-day wraparound
				if(uiTime > (uiTime+DNProcessCheck::DisconnectInterval))
					g_Log.Log( LogType::_FILELOG, L"[DB M/W][TID:%d][TIndex:%d] DB M/W Machine Not Reboot Last 49Days!!!!\n", ::GetCurrentThreadId(), nThreadID );
				else
				{
					g_Log.Log( LogType::_FILELOG, L"[DB M/W Delay] ThreadID:%d ThreadIndex:%d !!!!\n", ::GetCurrentThreadId(), nThreadID );

					for( UINT i=0 ; i<MainCmd.size() ; ++i )
						g_Log.Log( LogType::_FILELOG, L"[DB M/W Delay Protocol] MainCmd:%d SubCmd:%d Elapsed:%d AccountDBID:%d \n", MainCmd[i], i < SubCmd.size() ? SubCmd[i] : -1, i < Elapsed.size() ? Elapsed[i] : -1, i < AccountDBID.size() ? AccountDBID[i] : -1 );
				}
			}
			pBuffer += sizeof(UINT);
			uiSize  -= sizeof(UINT);
			continue;
		}
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
		else if (pSocketContext == reinterpret_cast<CSocketContext*>(reinterpret_cast<void*>(DNProcessCheck::HeartbeatKey)) )
		{
			CDNSQLMembership * pMembershipDB = g_SQLConnectionManager.FindMembershipDB(nThreadID);
			if (pMembershipDB)
			{
				if (pMembershipDB->QueryHeartbeat() != ERROR_NONE)
					_DANGER_POINT();
			}

			CDNSQLWorld * pWorldDB;
			for (int i = 0; i < g_Config.nWorldDBCount; i++)
			{
				pWorldDB = g_SQLConnectionManager.FindWorldDB(nThreadID, g_Config.WorldDB[i].nWorldSetID);
				if (pWorldDB)
				{
					if (pWorldDB->QueryHeartbeat() != ERROR_NONE)
						_DANGER_POINT();
				}
			}
			continue;
		}
#endif

		DNTPacket* pHeader = reinterpret_cast<DNTPacket*>(pBuffer);

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
		if( Scope.bIsDelete() )
		{
			pBuffer += pHeader->iLen;
			uiSize  -= pHeader->iLen;
			continue;
		}
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

		MainCmd.push_back( pHeader->cMainCmd );
		SubCmd.push_back( pHeader->cSubCmd );		

		TQHeader* pTempHeader = (TQHeader*)pHeader->buf;		
		AccountDBID.push_back( pTempHeader->nAccountDBID);

		DWORD dwCur = timeGetTime();

		CConnection* pCon = static_cast<CConnection*>(pSocketContext->GetParam());
		if( pCon ) {
			pCon->DBMessageProcess( pBuffer, nThreadID );
		}

		Elapsed.push_back( timeGetTime()-dwCur );

		pBuffer += pHeader->iLen;
		uiSize  -= pHeader->iLen;

		if( pCon )
		{
			long lRet = InterlockedExchangeAdd( &(static_cast<CDNConnection*>(pCon)->m_lDBQueueRemainSize), -pHeader->iLen );
			if( pCon->GetDelete() && lRet <= pHeader->iLen )
			{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#else
				delete pCon;
				//delete pSocketContext;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			}
		}
		else
		{
			_DANGER_POINT();
		}
	}
}
