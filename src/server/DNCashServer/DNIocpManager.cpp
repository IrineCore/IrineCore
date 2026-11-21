#include "stdafx.h"
#include "DNIocpManager.h"
#include "DNConnection.h"
#include "DNThreadManager.h"
#include "DNLogConnection.h"
#include "Log.h"
#include "DNUserDeleteManager.h"


CDNIocpManager* g_pIocpManager = NULL;

CDNIocpManager::CDNIocpManager(void)
{
}

CDNIocpManager::~CDNIocpManager(void)
{
}

UINT __stdcall CDNIocpManager::ReconnectThread(void *pParam)
{
	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	DWORD CurTick = 0;

	while(pIocp->m_bThreadSwitch){
		CurTick = timeGetTime();

		if (g_pLogConnection) g_pLogConnection->Reconnect(CurTick);
#if defined(PRE_DEL_ONNULL)
		if( g_pUserDeleteManager )
			g_pUserDeleteManager->DoUpdate();
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
		if( g_pThreadManager )
			g_pThreadManager->AddThreadCall((CSocketContext*)DNProcessCheck::HeartbeatKey);
#endif //#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
#else // #if defined(PRE_DEL_ONNULL)
		if (g_pUserDeleteManager && g_pThreadManager) 
		{
			g_pThreadManager->AddThreadCall(NULL);	// Return NULL from CThreadManager::WaitProcess() so it can be used like a timer
		}
#endif //#if defined(PRE_DEL_ONNULL)

		Sleep(1000);
	}
	return 0;
}

void CDNIocpManager::OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort)
{
	g_Log.Log(LogType::_NORMAL, L"[OnAccept:%d] Socket:%d, Ip:%S, Port:%d\r\n", pSocketContext->m_dwKeyParam, pSocketContext->m_Socket, pIp, nPort);

	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
			CDNConnection *pCon = new CDNConnection;
			if (!pCon) return;

			pCon->SetIp(pIp);
			pCon->SetPort(nPort);
			pCon->SetServerConnection(true);
			pCon->SetSocketContext(this, pSocketContext);
			pSocketContext->SetParam(pCon);

			g_Log.Log(LogType::_NORMAL, L"[OnAccept] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			pSocketContext->SetParam(g_pLogConnection);
			g_pLogConnection->SetSocketContext(this, pSocketContext);
			g_pLogConnection->SetServerConnection(false);

			g_Log.Log( LogType::_NORMAL, L"[OnAccept:Log] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, g_pLogConnection->GetIp());
			break;
		}

	}
}

void CDNIocpManager::OnConnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
			CConnection *pCon = (CConnection*)pSocketContext->GetParam();
			pCon->SetActive(true);
			pCon->SetDelete(false);
			g_Log.Log(LogType::_NORMAL, L"[OnConnected] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			g_pLogConnection->SetActive(true);
			g_pLogConnection->SetDelete(false);
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
	case CONNECTIONKEY_LOG:
		{
			// g_Log.Log(_ERROR, L"LogServer Fail(%S, %d)\r\n", pConnection->GetIp(), pConnection->GetPort());
		}
		break;
	}
}

void CDNIocpManager::OnDisconnected(CSocketContext *pSocketContext)
{
	if( !pSocketContext->GetParam())
		return;

	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
			CConnection *pCon = (CConnection*)pSocketContext->GetParam();
			pCon->SetActive(false);
			pCon->SetDelete(true);
			ClearSocketContext(pSocketContext);
			pCon->SetSocketContext(NULL, NULL);
			g_Log.Log(LogType::_NORMAL, L"[OnDisconnected] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			g_pLogConnection->SetActive(false);
			g_pLogConnection->SetDelete(true);
			ClearSocketContext(pSocketContext);
			g_pLogConnection->SetSocketContext(NULL, NULL);
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
			g_pLogConnection->ResetLiveCheck();
#endif
		}
		break;

	}
}

void CDNIocpManager::OnReceive(CSocketContext *pSocketContext, DWORD dwBytesTransferred)
{
	CConnection *pCon = (CConnection*)pSocketContext->GetParam();
	if (pCon)
	{
		int nResult = pCon->AddRecvData( pSocketContext );

		if (nResult > 0){
			g_pThreadManager->AddThreadCall(pSocketContext);
		}
		else if (nResult == SIZEERR){
			g_Log.Log(LogType::_NORMAL, L"[OnReceive SizeError] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, pSocketContext->m_RecvIO.Len);
			DetachSocket(pSocketContext, L"OnReceive SizeError");
		}
		else if (nResult == -1){
			g_Log.Log(LogType::_NORMAL, L"[OnReceive] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, pSocketContext->m_RecvIO.Len);
			DetachSocket(pSocketContext, L"OnReceive");
		}
	}
}

int CDNIocpManager::CreateThread()
{
	HANDLE hThread;
	UINT ThreadID;

	hThread = (HANDLE)_beginthreadex(NULL, 0, &ReconnectThread, this, 0, &ThreadID);
	if (hThread == INVALID_HANDLE_VALUE) return -1;
	CloseHandle(hThread);

	return 0;
}
