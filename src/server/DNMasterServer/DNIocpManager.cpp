#include "stdafx.h"
#include "Connection.h"
#include "DNIocpManager.h"
#include "DNDivisionManager.h"
#include "DNLoginConnection.h"
#include "DNVillageConnection.h"
#include "DNGameConnection.h"
#include "DNUser.h"
#include "Log.h"
#include "DNLogConnection.h"
#ifdef PRE_ADD_DOORS
#include "DNDoorsConnection.h"
#endif		//#ifdef PRE_ADD_DOORS

extern TMasterConfig g_Config;
extern CSyncLock g_UpdateLock;

HANDLE CDNIocpManager::m_hSignalProcess = INVALID_HANDLE_VALUE;

CDNIocpManager* g_pIocpManager = NULL;

CDNIocpManager::CDNIocpManager(void): CIocpManager()
{
	m_hProcessThread = INVALID_HANDLE_VALUE;
	m_hIdleThread = INVALID_HANDLE_VALUE;
	
	Final();

	m_hSignalProcess = CreateEvent(NULL, TRUE, FALSE, NULL);
}

CDNIocpManager::~CDNIocpManager(void)
{
	Final();
}

UINT __stdcall CDNIocpManager::ProcessThread(void *pParam)
{
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

			if (pCon && pCon->FlushRecvData(timeGetTime()) == false)
			{
				g_pDivisionManager->DelConnection(pCon, (eConnectionKey)pSocketContext->m_dwKeyParam);
				g_Log.Log(LogType::_NORMAL, L"## IN_DISCONNECT connection destroyed %x\r\n", pCon);
				
				int nConType = pSocketContext->m_dwKeyParam;
				pIocp->ClearSocketContext(pSocketContext);
				pCon->SetSocketContext(NULL, NULL);

				switch( nConType )
				{
					case CONNECTIONKEY_LOGIN:
					case CONNECTIONKEY_LOG:
						break;
					default:
					{
						SAFE_DELETE(pCon);
						pCon = NULL;
						break;
					}
				}
			}
		}
		else
		{
			// If process queue is empty, switch to non-signaled and wait for signal
			ResetEvent(m_hSignalProcess);
			::WaitForSingleObject(m_hSignalProcess, 1000);
		}

		ULONG nCurTick = 0;
		static ULONG nPreTick = 0;

		nCurTick = timeGetTime();
		if (nPreTick == 0)
			nPreTick = nCurTick;

		if (nPreTick != 0 && nPreTick + DISTRIBUTE_IDLE_TICK < nCurTick)
		{
			if (g_pDivisionManager)
				g_pDivisionManager->InternalIdleProcess(nCurTick);
			nPreTick = nCurTick;
		}

		// DISTRIBUTE_IDLE_TICK value is too large, so it runs separately.
		static UINT uiPvPUpdateTick = 0;
		if( uiPvPUpdateTick+100 < nCurTick )
		{
			if( g_pDivisionManager )
				g_pDivisionManager->UpdatePvPRoom( nCurTick );
			uiPvPUpdateTick = nCurTick;
		}
	}

	return 0;
}

extern void SavePartyID();
extern void SaveVoiceChannelID();
UINT __stdcall CDNIocpManager::IdleThread(void *pParam)
{
	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	DWORD CurTick = 0;
	static DWORD PreTick = 0;

	while(pIocp->m_bThreadSwitch){
		CurTick = timeGetTime();
		if (PreTick == 0)
			PreTick = CurTick;
		
		if (PreTick != 0 && PreTick + 1000 < CurTick)
		{
			if (g_pDivisionManager)
				g_pDivisionManager->ExternalIdleProcess(CurTick);			
			PreTick = CurTick;

			static int s_nCount = 0;
			if (s_nCount == 0 || s_nCount%(60) == 0)
			{
				SavePartyID();
				SaveVoiceChannelID();
			}
			s_nCount++;
		}


#ifdef PRE_ADD_DOORS
		if (g_pDoorsConnection)
			g_pDoorsConnection->Reconnect(CurTick);
#endif		//#ifdef PRE_ADD_DOORS
		Sleep(500);
	}
	return 0;
}

int CDNIocpManager::Init(int nSocketCountMax)
{
	CIocpManager::Init(nSocketCountMax);
	return CreateThread();
}

void CDNIocpManager::Final()
{
	CIocpManager::Final();

	while (!m_ProcessCalls.empty()){
		m_ProcessCalls.pop();
	}

	if( m_hProcessThread != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject( m_hProcessThread, INFINITE );
		CloseHandle( m_hProcessThread );
	}
	if( m_hIdleThread != INVALID_HANDLE_VALUE )
	{
		WaitForSingleObject( m_hIdleThread, INFINITE );
		CloseHandle( m_hIdleThread );
	}

	CloseHandle(m_hSignalProcess);
}

int CDNIocpManager::CreateThread()
{
	UINT ThreadID;

	m_hProcessThread = (HANDLE)_beginthreadex(NULL, 0, &ProcessThread, this, 0, &ThreadID);
	if (m_hProcessThread == INVALID_HANDLE_VALUE) return -1;

	m_hIdleThread = (HANDLE)_beginthreadex(NULL, 0, &IdleThread, this, 0, &ThreadID);
	if (m_hIdleThread == INVALID_HANDLE_VALUE) return -1;

	return 0;
}

void CDNIocpManager::AddProcessCall(CSocketContext *pSocketContext)
{
	m_ProcessLock.Lock();
	m_ProcessCalls.push(pSocketContext);
	SetEvent(m_hSignalProcess);
	m_ProcessLock.UnLock();
}

void CDNIocpManager::OnAccept(CSocketContext *pSocketContext, const char* pIp, const int nPort)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_LOGIN:
		{
			CDNLoginConnection * pCon = g_pDivisionManager->FindLoginConnection(pIp, nPort);
			if (!pCon) {
				DetachSocket(pSocketContext, L"DivisionManager Login Connect Failed");
				return;
			}

			pSocketContext->SetParam(pCon);
			pCon->SetSocketContext(this, pSocketContext);
			pCon->SetServerConnection(false);
			g_Log.Log(LogType::_FILEDBLOG, L"[OnAccept:Login] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;

	case CONNECTIONKEY_VILLAGE:
		{
			CDNVillageConnection * pCon = g_pDivisionManager->VillageServerConnected(pIp, nPort);
			if (!pCon)
			{
				DetachSocket(pSocketContext, L"DivisionManager Village Connect Failed");
				return;
			}

			pSocketContext->SetParam(pCon);
			pCon->SetSocketContext(this, pSocketContext);
			pCon->SetServerConnection(true);
			g_Log.Log(LogType::_FILEDBLOG, L"[OnAccept:Village] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;

	case CONNECTIONKEY_GAME:
		{
			CDNGameConnection * pCon = g_pDivisionManager->GameServerConnected(pIp, nPort);
			if (!pCon)
			{
				DetachSocket(pSocketContext, L"DivisionManager Game Connect Failed");
				return;
			}

			pSocketContext->SetParam(pCon);
			pCon->SetSocketContext(this, pSocketContext);
			pCon->SetServerConnection(true);

			g_Log.Log(LogType::_FILEDBLOG, L"[OnAccept:Game] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
		}
		break;
		case CONNECTIONKEY_LOG:
		{
			pSocketContext->SetParam(g_pLogConnection);
			g_pLogConnection->SetSocketContext(this, pSocketContext);
			g_pLogConnection->SetServerConnection(false);

			g_Log.Log( LogType::_FILEDBLOG, g_Config.nWorldSetID, 0, 0, 0, L"[OnAccept:Log] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, g_pLogConnection->GetIp());
			break;
		}

#ifdef PRE_ADD_DOORS
	case CONNECTIONKEY_DOORS:
		{
			pSocketContext->SetParam(g_pDoorsConnection);
			g_pDoorsConnection->SetSocketContext(this, pSocketContext);
			g_pDoorsConnection->SetServerConnection(false);

			g_Log.Log(LogType::_FILEDBLOG, L"[OnAccept:DoorsServer] Socket:%d, Ip:%S, Port:%d\r\n", pSocketContext->m_Socket, g_pDoorsConnection->GetIp(), g_pDoorsConnection->GetPort());
		}
		break;
#endif		//#ifdef PRE_ADD_DOORS
	}
}

// 2009.02.03 kimbap
// Calling BufferClear() in OnConnected() corrupts buffer and loses packets if packets are sent right after connecting.
// Changed to BufferClear() in CIocpManager::AttachSocket().

void CDNIocpManager::OnConnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_LOGIN:
		{
			CDNLoginConnection *pLoginCon = reinterpret_cast<CDNLoginConnection*>(pSocketContext->GetParam());
			if (!pLoginCon) {
				return;
			}

			pLoginCon->SetActive(true);
			pLoginCon->SetDelete(false);
		
			g_Log.Log(LogType::_FILEDBLOG, L"[OnConnected:Login] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;

	case CONNECTIONKEY_VILLAGE:
		{
			CDNVillageConnection *pVillageCon = (CDNVillageConnection*)pSocketContext->GetParam();
			if (!pVillageCon) return;

			pVillageCon->SetActive(true);
			pVillageCon->SetDelete(false);
			//pVillageCon->BufferClear();

			//Village Connection Resgistration Start Point
			if (g_Config.nWorldSetID > 0)
			{
				pVillageCon->SendRegistWorldID();
			}
			else
			{
				DetachSocket(pSocketContext, L"MasterServer Initialized Yet Try Again");
				return;
			}

			g_Log.Log(LogType::_FILEDBLOG, L"[OnConnected:Village] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;

	case CONNECTIONKEY_GAME:
		{
			CDNGameConnection *pGameCon = (CDNGameConnection*)pSocketContext->GetParam();
			if (!pGameCon) return;

			pGameCon->SetActive(true);
			pGameCon->SetDelete(false);
			//pGameCon->BufferClear();

			//Game Connection Resgistration Start Point
			if (g_Config.nWorldSetID > 0)
			{
				pGameCon->SendRegistWorldID();
			}
			else
			{
				DetachSocket(pSocketContext, L"MasterServer Initialized Yet Try Again");
				return;
			}

			g_Log.Log(LogType::_FILEDBLOG, L"[OnConnected:Game] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			g_pLogConnection->SetActive(true);
			g_pLogConnection->SetDelete(false);
		}
		break;


#ifdef PRE_ADD_DOORS
	case CONNECTIONKEY_DOORS:
		{
			g_pDoorsConnection->SetActive(true);
			g_pDoorsConnection->SetDelete(false);
			g_Log.Log(LogType::_FILEDBLOG, L"[OnConnected:DoorsServer] Socket:%d\r\n", pSocketContext->m_Socket);

			g_pDoorsConnection->SendRegist(g_Config.nWorldSetID, g_Config.DoorsIdenty[0].szIP, g_Config.DoorsIdenty[0].nPort, g_Config.DoorsIdenty[1].szIP, g_Config.DoorsIdenty[1].nPort);
		}
		break;
#endif		//#ifdef PRE_ADD_DOORS
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
	case CONNECTIONKEY_LOGIN:
		{
#if !defined( STRESS_TEST )
			g_Log.Log(LogType::_FILEDBLOG, L"LoginServer Fail(%S, %d)\r\n", pConnection->GetIp(), pConnection->GetPort());
#endif
		}
		break;
	case CONNECTIONKEY_LOG:
		{
			// g_Log.Log(_ERROR, L"LogServer Fail(%S, %d)\r\n", pConnection->GetIp(), pConnection->GetPort());
			// Temporarily disable this!!! 080918 saset
		}
		break;
	}
}

void CDNIocpManager::OnDisconnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_LOGIN:
		{
			CDNLoginConnection *pLoginCon = reinterpret_cast<CDNLoginConnection*>(pSocketContext->GetParam());
			if (!pLoginCon) return;
			
			DNTPacket Header = { 0, };
			Header.cMainCmd = IN_DISCONNECT;
			Header.iLen = sizeof(USHORT) + (sizeof(BYTE) * 2);

			pLoginCon->BufferClear();
			pLoginCon->SetActive(false);
			pLoginCon->SetDelete(true);
			pLoginCon->AddRecvData( Header );

			g_Log.Log(LogType::_FILEDBLOG, L"[OnDisconnected:Login] Socket:%d\r\n", pSocketContext->m_Socket);
			AddProcessCall(pSocketContext);
		}
		break;

	case CONNECTIONKEY_VILLAGE:
		{
			CDNVillageConnection *pVillageCon = (CDNVillageConnection*)pSocketContext->GetParam();
			if (!pVillageCon) return;

			DNTPacket Header = { 0, };
			Header.cMainCmd = IN_DISCONNECT;
			Header.iLen = sizeof(USHORT) + (sizeof(BYTE) * 2);

			pVillageCon->BufferClear();
			pVillageCon->SetActive(false);
			pVillageCon->SetDelete(true);
			pVillageCon->AddRecvData( Header );
			pVillageCon->SetConnectionCompleted( false );

			g_Log.Log(LogType::_FILEDBLOG, L"[OnDisconnected:Village] Socket:%d\r\n", pSocketContext->m_Socket);
			AddProcessCall(pSocketContext);
		}
		break;

	case CONNECTIONKEY_GAME:
		{
			CDNGameConnection *pGameCon = (CDNGameConnection*)pSocketContext->GetParam();
			if (!pGameCon) return;

			DNTPacket Header = { 0, };
			Header.cMainCmd = IN_DISCONNECT;
			Header.iLen = sizeof(USHORT) + (sizeof(BYTE) * 2);

			pGameCon->BufferClear();
			pGameCon->SetActive(false);
			pGameCon->SetDelete(true);
			pGameCon->AddRecvData( Header );

			g_Log.Log(LogType::_FILEDBLOG, L"[OnDisconnected:Game] Socket:%d\r\n", pSocketContext->m_Socket);
			AddProcessCall(pSocketContext);
		}
		break;

	case CONNECTIONKEY_LOG:
		{
			DNTPacket Header = { 0, };
			Header.cMainCmd = IN_DISCONNECT;
			Header.iLen = sizeof(USHORT) + (sizeof(BYTE) * 2);

			g_pLogConnection->BufferClear();
			g_pLogConnection->SetActive(false);
			g_pLogConnection->SetDelete(true);
			g_pLogConnection->AddRecvData( Header );
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
			g_pLogConnection->ResetLiveCheck();
#endif
			g_Log.Log(LogType::_FILELOG, L"[OnDisconnected:Log] Socket:%d\r\n", pSocketContext->m_Socket);
			AddProcessCall(pSocketContext);
		}
		break;


#ifdef PRE_ADD_DOORS
	case CONNECTIONKEY_DOORS:
		{
			g_pDoorsConnection->SetActive(false);
			g_pDoorsConnection->SetDelete(true);
			ClearSocketContext(pSocketContext);
			g_pDoorsConnection->SetSocketContext(NULL, NULL);
			g_Log.Log(LogType::_FILEDBLOG, L"[OnDisconnected:DoorServer] Socket:%d\r\n", pSocketContext->m_Socket);
		}
		break;
#endif		//#ifdef PRE_ADD_DOORS
	}
}

void CDNIocpManager::OnReceive(CSocketContext *pSocketContext, DWORD dwBytesTransferred)
{
	CConnection *pCon = (CConnection*)pSocketContext->GetParam();
	if (pCon) {
		int nResult = pCon->AddRecvData( pSocketContext );

		if (nResult > 0){
			AddProcessCall(pSocketContext);
		}
		else if (nResult == SIZEERR){
			g_Log.Log(LogType::_ERROR, L"[OnReceive SizeError] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, pSocketContext->m_RecvIO.Len);
			DetachSocket(pSocketContext, L"OnReceive SizeError");
		}
		else if (nResult == -1){
			g_Log.Log(LogType::_ERROR, L"[OnReceive] Socket:%lld Size:%d\r\n", pSocketContext->m_Socket, pSocketContext->m_RecvIO.Len);
			DetachSocket(pSocketContext, L"OnReceive");
		}
	}
}
