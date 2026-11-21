#include "stdafx.h"
#include "DNIocpManager.h"
#include "DNThreadManager.h"
#include "DNConnection.h"
#include "Log.h"
#include "DNProtocol.h"

CDNThreadManager g_ThreadManager;

CDNThreadManager::CDNThreadManager(void): CThreadManager()
{
}

CDNThreadManager::~CDNThreadManager(void)
{
}

#if defined(PRE_DEL_ONNULL)
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pBuffer, TThreadData* tThreadData)
#else
void CDNThreadManager::OnCalled(CSocketContext *pSocketContext, char *pBuffer, int nThreadID)
#endif
{
#if defined(PRE_DEL_ONNULL)
	int nThreadID = tThreadData->nThreadID;
#endif
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
			if( Scope.bIsDelete() )
				break;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

			CDNConnection *pCon = (CDNConnection*)pSocketContext->GetParam();
			if (!pCon) return;

			pCon->FlushDBData( nThreadID );
		}
		break;

	}
	g_pIocpManager->DetachSocket(pSocketContext, L"OnCalled");
}