#include "stdafx.h"
#include "Connection.h"
#include "DNIocpManager.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNUserConnection.h"
#include "DNUserConnectionManager.h"
#include "DNExtManager.h"
#include "Log.h"
#include "DNLogConnection.h"
#include "DNAuthManager.h"

#include "DNSQLMembershipManager.h"
#include "DNSQLWorldManager.h"

#ifdef _USE_ACCEPTEX
#include "SocketContextMgr.h"
#endif

CDNIocpManager* g_pIocpManager;

#ifndef _USE_ACCEPTEX
HANDLE CDNIocpManager::m_hSignalProcess = INVALID_HANDLE_VALUE;
#endif

extern TLoginConfig g_Config;

CDNIocpManager::CDNIocpManager(void): CIocpManager()
{
	Final();

#ifdef _USE_ACCEPTEX
	m_hReconnectThread	= INVALID_HANDLE_VALUE;
	m_hUpdateThread		= INVALID_HANDLE_VALUE;
#else
	m_hSignalProcess = CreateEvent(NULL, TRUE, FALSE, NULL);
#endif
}

CDNIocpManager::~CDNIocpManager(void)
{
	Final();
}

#ifndef _USE_ACCEPTEX

UINT __stdcall CDNIocpManager::ProcessThread(void *pParam)
{
	g_Log.Log(LogType::_FILELOG, L"[Thread-Start] ProcessThread - TID : %d\r\n", ::GetCurrentThreadId());

	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	CSocketContext *pSocketContext;
	CConnection *pCon = NULL;
	USHORT nRecv = 0;

	while(pIocp->m_bThreadSwitch)
	{
		if (!pIocp->m_ProcessCalls.empty())
		{
			pIocp->m_ProcessLock.Lock();
			pSocketContext = pIocp->m_ProcessCalls.front();
			pIocp->m_ProcessCalls.pop();
			pIocp->m_ProcessLock.UnLock();

			pCon = (CConnection*)pSocketContext->GetParam();


			if (pCon && pCon->FlushRecvData() == false)
			{
				if (pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER){
					g_pUserConnectionManager->DelConnection(pCon);
				}
				else if (pSocketContext->m_dwKeyParam == CONNECTIONKEY_MASTER){
					g_pMasterConnectionManager->DelConnection(pCon);
					g_Log.Log(LogType::_NORMAL, pCon, L"## Master IN_DISCONNECT connection destroyed %x World:%d\r\n", pCon, pCon->GetSessionID());
				}

#ifdef _USE_ACCEPTEX
				CSocketContextMgr::GetInstance().OnDisconnect( pSocketContext );
#endif
				pIocp->ClearSocketContext(pSocketContext);
				pCon->SetSocketContext(NULL, NULL);
				SAFE_DELETE(pCon);
			}
		}
		// If process queue is empty, switch to non-signaled and wait for signal
		else
		{
			ResetEvent(m_hSignalProcess);
			::WaitForSingleObject(m_hSignalProcess, 1000);
		}
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] ProcessThread - TID : %d\r\n", ::GetCurrentThreadId());

	return 0;
}

#endif

UINT __stdcall CDNIocpManager::UpdateThread(void *pParam)
{
	g_Log.Log(LogType::_FILELOG, L"[Thread-Start] UpdateThread - TID : %d\r\n", ::GetCurrentThreadId());

	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	DWORD CurTick = 0;
	int nCount = 0;

	while(pIocp->m_bThreadSwitch){
		CurTick = timeGetTime();
		if (g_pUserConnectionManager) g_pUserConnectionManager->DoUpdate(CurTick);

		if (g_pSQLMembershipManager)
			g_pSQLMembershipManager->DoUpdate(CurTick);
		if (g_pSQLWorldManager)
			g_pSQLWorldManager->DoUpdate(CurTick);
		

		Sleep(1);
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] UpdateThread - TID : %d\r\n", ::GetCurrentThreadId());

	return 0;
}

extern void SaveUserSessionID();
UINT __stdcall CDNIocpManager::ReconnectThread(void *pParam)
{
	g_Log.Log(LogType::_FILELOG, L"[Thread-Start] ReconnectThread - TID : %d\r\n", ::GetCurrentThreadId());

	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	DWORD CurTick = 0;

	while(pIocp->m_bThreadSwitch){
		CurTick = timeGetTime();
		
		if (g_pLogConnection) g_pLogConnection->Reconnect(CurTick);
		if (g_pAuthManager) g_pAuthManager->Run(CurTick);


		static int nCount = 0;
		if (nCount == 0 || (nCount%(1000*3)) == 0){
			if (g_IDGenerator.IsUserConnectionSet()) {	// Added because during LO init this was called before loading session ID, unconditionally resetting session ID to 0 (20100215 b4nfter)
				SaveUserSessionID();	// Rent a spot (?) here and save... 080918 saset
			}
		}
		nCount++;

		Sleep(1);
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] ReconnectThread - TID : %d\r\n", ::GetCurrentThreadId());

	return 0;
}

int CDNIocpManager::Init(int nSocketCountMax, int nWorkerThreadCount )
{
	if( CIocpManager::Init(nSocketCountMax, nWorkerThreadCount) < 0)
		return -1;
	return CreateThread();
}

void CDNIocpManager::Final()
{
	CIocpManager::Final();

#ifndef _USE_ACCEPTEX
	while (!m_ProcessCalls.empty())
	{
		m_ProcessCalls.pop();
	}

	CloseHandle(m_hSignalProcess);
#endif
}

int CDNIocpManager::CreateThread()
{
	UINT ThreadID;

#ifndef _USE_ACCEPTEX
	HANDLE hThread;
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ProcessThread, this, 0, &ThreadID);
	if (hThread == INVALID_HANDLE_VALUE) return -1;
	CloseHandle(hThread);
#endif

#ifdef _USE_ACCEPTEX
	m_hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, &UpdateThread, this, 0, &ThreadID);
	if (m_hUpdateThread == INVALID_HANDLE_VALUE) 
		return -1;

	m_hReconnectThread = (HANDLE)_beginthreadex(NULL, 0, &ReconnectThread, this, 0, &ThreadID);
	if (m_hReconnectThread == INVALID_HANDLE_VALUE) 
		return -1;
#else
	hThread = (HANDLE)_beginthreadex(NULL, 0, &UpdateThread, this, 0, &ThreadID);
	if (hThread == INVALID_HANDLE_VALUE) return -1;
	CloseHandle(hThread);

	hThread = (HANDLE)_beginthreadex(NULL, 0, &ReconnectThread, this, 0, &ThreadID);
	if (hThread == INVALID_HANDLE_VALUE) return -1;
	CloseHandle(hThread);
#endif

	return 0;
}

#ifdef _USE_ACCEPTEX

void CDNIocpManager::PostDisconnect(CSocketContext *pSocketContext)
{
	PostQueuedCompletionStatus( m_hIOCP, 4, (ULONG_PTR)pSocketContext, &pSocketContext->m_RecvIO );
}

void CDNIocpManager::ThreadStop()
{
	CIocpManager::ThreadStop();

	if( m_hUpdateThread != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject( m_hUpdateThread, INFINITE );
		CloseHandle( m_hUpdateThread );
	}

	if( m_hReconnectThread != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject( m_hReconnectThread, INFINITE );
		CloseHandle( m_hReconnectThread );
	}
}

#else

void CDNIocpManager::AddProcessCall(CSocketContext *pSocketContext)
{
	m_ProcessLock.Lock();
	m_ProcessCalls.push(pSocketContext);
	SetEvent(m_hSignalProcess);
	m_ProcessLock.UnLock();
}

#endif

void CDNIocpManager::OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_USER:
		{
			CDNUserConnection *pUserCon = (CDNUserConnection*)g_pUserConnectionManager->AddConnection(pIp, nPort);
			if (!pUserCon) return;

			pSocketContext->SetParam(pUserCon);
			pUserCon->SetSocketContext(this, pSocketContext);
		}
		break;

	case CONNECTIONKEY_MASTER:
		{
			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)g_pMasterConnectionManager->AddConnection(pIp, nPort);
			if (!pMasterCon) return;

			pSocketContext->SetParam(pMasterCon);
			pMasterCon->SetSocketContext(this, pSocketContext);
			pMasterCon->SetServerConnection(false);

			g_Log.Log(LogType::_FILELOG, L"[OnAccept:Master] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pMasterCon->GetIp());
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			pSocketContext->SetParam(g_pLogConnection);
			g_pLogConnection->SetSocketContext(this, pSocketContext);
			g_pLogConnection->SetServerConnection(false);

			g_Log.Log(LogType::_FILELOG, L"[OnAccept:Log] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, g_pLogConnection->GetIp());
		}
		break;


	}
}

// 2009.02.03 kimbap
// Calling BufferClear() in OnConnected() corrupts buffer and loses packets if packets are sent right after connecting.
// Changed to BufferClear() in CIocpManager::AttachSocket().

void CDNIocpManager::OnConnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_USER:
		{
			CDNUserConnection *pUserCon = (CDNUserConnection*)pSocketContext->GetParam();
			if (!pUserCon) return;

			pUserCon->SetActive(true);
			pUserCon->SetDelete(false);
		}
		break;

	case CONNECTIONKEY_MASTER:
		{
			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)pSocketContext->GetParam();
			if (!pMasterCon) return;

			pMasterCon->SetActive(true);
			pMasterCon->SetDelete(false);
			//pMasterCon->BufferClear();

			if (pMasterCon->SendRegist(g_pAuthManager->GetServerID()) == false)
			{
				DetachSocket(pSocketContext, L"SendRegist Failed");
				g_Log.Log(LogType::_FILELOG, L"[OnConnected:Master] Socket:%d SendRegist Failed!!!!\r\n", pSocketContext->m_Socket);
			}

			g_Log.Log(LogType::_FILELOG, L"[OnConnected:Master] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			g_pLogConnection->SetActive(true);
			g_pLogConnection->SetDelete(false);

			g_Log.Log(LogType::_FILELOG, L"[OnConnected:Log] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;


	}
}

void CDNIocpManager::OnConnectFail(CSocketContext *pSocketContext)
{
	// Only include ones that attempt to connect from the server.. ones calling AddConnectionEx?
	CConnection* pConnection = static_cast<CConnection*>(pSocketContext->GetParam());
	if( !pConnection )
		return;

	pConnection->SetConnecting(false);

	switch(pSocketContext->m_dwKeyParam)
	{	
	case CONNECTIONKEY_LOG :
		{
			g_Log.Log(LogType::_FILELOG, L"LogServer Fail(%S, %d)\r\n", pConnection->GetIp(), pConnection->GetPort());
		}
		break;
	}
}

void CDNIocpManager::OnDisconnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_USER:
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
			if( Scope.bIsDelete() )
				break;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

			CDNUserConnection *pUserCon = (CDNUserConnection*)pSocketContext->GetParam();
			if (!pUserCon) return;

			if ((pUserCon->GetUserState() != STATE_CONNECTVILLAGE) && (pUserCon->GetUserState() != STATE_CONNECTGAME)) // Not a user disconnecting due to moving to village or game server
			{		
				if (pUserCon->IsCertified()) {	// Caution !!! - Auth info reset must only be performed for users authenticated through QUERY_BEGINAUTH and QUERY_CHECKAUTH !!!

					if (0 != pUserCon->GetAccountDBID()) {	// P.S.> This state can be reached via CDNUserConnection::RequestBackButton(...)
						{
							// Process immediately instead of adding to list
							g_pAuthManager->QueryResetAuth(pUserCon->GetWorldSetID(), pUserCon->GetAccountDBID(), pUserCon->GetSessionID());
						}
					}

					pUserCon->ResetCertified();
				}
			}
#if defined(PRE_ADD_LOGIN_USERCOUNT)
			if( pUserCon->m_bSummitUserCount ) 
			{
				g_pUserConnectionManager->DelUserCount();
				pUserCon->m_bSummitUserCount = false;
			}
#endif

			// 2010-10-08 haling Moved down due to QueryResetAuth..
			pUserCon->SendInsideDisconnectPacket(L"OnDisconnected");		// Create object cleanup packet
		}
		break;

	case CONNECTIONKEY_MASTER:
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
			if( Scope.bIsDelete() )
				break;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

			CDNMasterConnection *pMasterCon = (CDNMasterConnection*)pSocketContext->GetParam();
			if (!pMasterCon) return;

			g_Log.Log( pMasterCon->GetWorldSetID()>0?LogType::_NORMAL:LogType::_FILELOG, L"[OnDisconnect] %x WorldID:%d\r\n", this, pMasterCon->GetWorldSetID() );
			g_pExtManager->SetOnOffServerInfo(pMasterCon->GetWorldSetID(), false);	// Set on and off
			//If master dies, disconnect users currently in this queue for now.

#ifdef _USE_ACCEPTEX
			ScopeLock<CSyncLock> Lock( g_pUserConnectionManager->m_ConSync );
#endif
			g_pUserConnectionManager->DetachUserByWorldSetID(pMasterCon->GetWorldSetID());

			DNTPacket Header = { 0, };
			Header.cMainCmd = IN_DISCONNECT;
			Header.iLen = sizeof(USHORT) + (sizeof(BYTE) * 2);

			pMasterCon->BufferClear();
			pMasterCon->SetActive(false);
			pMasterCon->SetDelete(true);
			pMasterCon->AddRecvData( Header );

			g_Log.Log(LogType::_FILELOG, L"[OnDisconnected:Master] Socket:%d\r\n", pSocketContext->m_Socket);
#ifdef _USE_ACCEPTEX
			PostDisconnect( pSocketContext );
#else
			AddProcessCall(pSocketContext);
#endif
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			g_pLogConnection->SetActive(false);
			g_pLogConnection->SetDelete(true);
			ClearSocketContext(pSocketContext);
			g_pLogConnection->SetSocketContext(NULL,NULL);
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
			g_pLogConnection->ResetLiveCheck();
#endif
			g_Log.Log(LogType::_FILELOG, L"[OnDisconnected:Log] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;

	}
}

void CDNIocpManager::OnReceive(CSocketContext *pSocketContext, DWORD dwBytesTransferred)
{
#ifdef _USE_ACCEPTEX

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
	if( Scope.bIsDelete() )
		return;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

	CConnection *pCon = (CConnection*)pSocketContext->GetParam();
	if (pCon) {
		int nResult = pCon->AddRecvData( pSocketContext );

		if (nResult > 0)
		{


			
			if( pCon->FlushRecvData(timeGetTime()) == false )
			{
#ifdef PRE_FIX_SOCKETCONTEXT_DANGLINGPTR
				while (true)
				{
					if (InterlockedCompareExchange(&pSocketContext->m_lActiveCount, CSocketContext::DeleteFlag, 1) == 1)
					{
#endif		//#ifdef PRE_FIX_SOCKETCONTEXT_DANGLINGPTR

						if( pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER)
						{
							g_pUserConnectionManager->DelConnection(pCon);

							ScopeLock<CSyncLock> Lock( g_pUserConnectionManager->m_ConSync );

							CConnection* pDeleteCon = static_cast<CConnection*>(pSocketContext->GetParam());
							if( pDeleteCon ) {
							}

							CSocketContextMgr::GetInstance().OnDisconnect( pSocketContext );
							SAFE_DELETE( pDeleteCon );					
							return;
						}
						else if (pSocketContext->m_dwKeyParam == CONNECTIONKEY_MASTER)
						{
							g_pMasterConnectionManager->DelConnection(pCon);

							ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );

							CConnection* pDeleteCon = static_cast<CConnection*>(pSocketContext->GetParam());
							if( pDeleteCon )
								g_Log.Log(LogType::_NORMAL, L"## Master IN_DISCONNECT connection destroyed %x World:%d\r\n", pDeleteCon, pDeleteCon->GetSessionID());

							CSocketContextMgr::GetInstance().OnDisconnect( pSocketContext );
							SAFE_DELETE( pDeleteCon );
							return;
						}

						ScopeLock<CSyncLock> Lock( m_DeleteLock );

						CConnection* pDeleteCon = static_cast<CConnection*>(pSocketContext->GetParam());

						CSocketContextMgr::GetInstance().OnDisconnect( pSocketContext );
						SAFE_DELETE( pDeleteCon );

#ifdef PRE_FIX_SOCKETCONTEXT_DANGLINGPTR
						break;
					}
				}
#endif		//#ifdef PRE_FIX_SOCKETCONTEXT_DANGLINGPTR
			}
			else
			{
				if (pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER)
					m_nAddRecvBufSize += dwBytesTransferred;
			}
		}
		else if (nResult == SIZEERR){
			g_Log.Log(LogType::_ERROR, L"[OnReceive SizeError] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, dwBytesTransferred);
			DetachSocket(pSocketContext, L"OnReceive SizeError");
		}
		else if (nResult == -1){
			g_Log.Log(LogType::_ERROR, L"[OnReceive] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, dwBytesTransferred);
			DetachSocket(pSocketContext, L"OnReceive");
		}
	}
#else

	CConnection *pCon = (CConnection*)pSocketContext->GetParam();
	if (pCon) {
		int nResult = pCon->AddRecvData( pSocketContext );

		if (nResult > 0){
			AddProcessCall(pSocketContext);

			if (pSocketContext->m_dwKeyParam == CONNECTIONKEY_USER)
				m_nAddRecvBufSize += dwBytesTransferred;
		}
		else if (nResult == SIZEERR){
			g_Log.Log(LogType::_ERROR, L"[OnReceive SizeError] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, dwBytesTransferred);
			DetachSocket(pSocketContext, L"OnReceive SizeError");
		}
		else if (nResult == -1){
			g_Log.Log(LogType::_ERROR, L"[OnReceive] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, dwBytesTransferred);
			DetachSocket(pSocketContext, L"OnReceive");
		}
	}

#endif // #ifdef _USE_ACCEPTEX
}

void CDNIocpManager::DetachSocket (CSocketContext *pSocketContext, wchar_t *pIdent)
{
	if (pSocketContext->DelRef() == 0){
		if (pSocketContext->m_dwKeyParam != CONNECTIONKEY_DEFAULT){
			//g_Log.Log(L"Socket:%d [!] DetachSocket (%x) %S \r\n", pSocketContext->m_Socket, pSocketContext->GetParam(), pIdent);
		}

		DelSocket(pSocketContext);
		OnDisconnected(pSocketContext);
	}
}

