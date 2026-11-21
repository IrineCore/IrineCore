#include "stdafx.h"
#include "DNIocpManager.h"
#include "DNConnection.h"
#include "DNThreadManager.h"
#include "Log.h"
#include "DNLogConnection.h"
#include "SpinBuffer.h"

extern TDBConfig g_Config;

CDNIocpManager* g_pIocpManager = NULL;

CDNIocpManager::CDNIocpManager(void): CIocpManager()
{
	for( int i=0 ; i<g_Config.nThreadMax ; ++i )
	{
		m_vSpinBuffer.push_back( new CTcpRecvSpinBuffer(1,1024*1000*10) );
		m_vCallEvent.push_back( false );
	}
	m_uiLastUpdateTick = timeGetTime();
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	m_nLastHearbeatTick = timeGetTime();
#endif
#if defined( PRE_FIX_67546 )
	m_vVillageGameConnection.clear();
	memset(m_nConnectionCount, 0, sizeof(m_nConnectionCount));
#endif
}

CDNIocpManager::~CDNIocpManager(void)
{
	for( UINT i=0 ; i<m_vSpinBuffer.size() ; ++i )
	{
		if( m_vSpinBuffer[i] )
			delete m_vSpinBuffer[i];
	}
	m_vSpinBuffer.clear();
}

UINT __stdcall CDNIocpManager::ReconnectThread(void *pParam)
{
	CDNIocpManager *pIocp = (CDNIocpManager*)pParam;

	DWORD CurTick = 0;

	while(pIocp->m_bThreadSwitch){
		CurTick = timeGetTime();
		
		if( g_pLogConnection )
			g_pLogConnection->Reconnect( CurTick );

		pIocp->DoUpdate( CurTick );

		Sleep(1000);
	}
	return 0;
}

void CDNIocpManager::DoUpdate( UINT uiCurTick )
{
	if( uiCurTick-m_uiLastUpdateTick >= DNProcessCheck::UpdateInterval )
	{
		CSocketContext* pSocketContext = reinterpret_cast<CSocketContext*>(DNProcessCheck::Key);

		m_uiLastUpdateTick = timeGetTime();

		for( UINT i=0 ; i<m_vSpinBuffer.size() ; ++i )
		{
			if( m_vSpinBuffer[i] )
			{
				m_vSpinBuffer[i]->Lock();
				m_vSpinBuffer[i]->RawPush( &pSocketContext, sizeof(CSocketContext*) );
				m_vSpinBuffer[i]->RawPush( &uiCurTick, sizeof(uiCurTick) );
				m_vSpinBuffer[i]->UnLock();
			}
		}
	}
	
#if defined(PRE_ADD_HEARTBEAT_TO_SQL)
	if (m_nLastHearbeatTick + DNProcessCheck::SQLHeartbeatInterval < uiCurTick )
	{
		m_nLastHearbeatTick = timeGetTime();
		CSocketContext* pSocketContext = reinterpret_cast<CSocketContext*>(DNProcessCheck::HeartbeatKey);

		for( UINT i=0 ; i<m_vSpinBuffer.size() ; ++i )
		{
			if( m_vSpinBuffer[i] )
			{
				m_vSpinBuffer[i]->Lock();
				m_vSpinBuffer[i]->RawPush( &pSocketContext, sizeof(CSocketContext*) );
				m_vSpinBuffer[i]->UnLock();
			}
		}
	}
#endif
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

			g_Log.Log( LogType::_NORMAL, 0, 0, 0, 0, L"[OnAccept:Log] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, g_pLogConnection->GetIp());
			break;
		}
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
			// Temporarily disable this!!! 080918 saset
		}
		break;
	}
}

void CDNIocpManager::OnConnected(CSocketContext *pSocketContext)
{
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
			CIocpManager::OnConnected(pSocketContext);

			CConnection *pCon = (CConnection*)pSocketContext->GetParam();
			pCon->SetActive(true);
			pCon->SetDelete(false);

			g_Log.Log(LogType::_NORMAL, L"[OnConnected] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());
#if defined( PRE_FIX_67546 )
			AddVillageGameConnection( (CDNConnection*)pCon );
			TAAddThreadCount pA;
			memset(&pA, 0, sizeof(pA));
			int nCount[THREADMAX];
			GetConnectionCount( nCount );
			memcpy( pA.nConnectionCount, nCount, sizeof(pA.nConnectionCount) );
			((CDNConnection*)(pCon))->AddSendData( MAINCMD_ETC, QUERY_ADD_CHANNELCOUNT, reinterpret_cast<char*>(&pA), sizeof(pA) );
#endif
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

#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
#include <iostream>
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

void CDNIocpManager::OnDisconnected(CSocketContext *pSocketContext)
{	
	switch(pSocketContext->m_dwKeyParam)
	{
	case CONNECTIONKEY_DEFAULT:
		{
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
			DWORD dwTime = timeGetTime();
			bool bDelete = false;
			bool bLog = true;
			while( timeGetTime()-dwTime <= 1000 )
			{
				CConnection *pCon = (CConnection*)pSocketContext->GetParam();				

				if( InterlockedCompareExchange( &pSocketContext->m_lActiveCount, CSocketContext::DeleteFlag, 0 ) == 0 )
				{
					wstring strReason;
					pSocketContext->GetDetachReason(strReason);				
					g_Log.Log(LogType::_ERROR, L"[%d] Disconnect Connection %s!!!\r\n", g_Config.nManagedID, strReason.c_str());

#if defined( PRE_FIX_67546 )
					DelVillageGameConnection( (CDNConnection*)pCon );
#endif
					bDelete = true;
					pCon->SetSocketContext(NULL, NULL);
					SAFE_DELETE(pCon);
					ClearSocketContext(pSocketContext);
					break;
				}
				else
				{
					if( bLog == true )
					{
						wstring strReason;
						pSocketContext->GetDetachReason(strReason);						
						g_Log.Log(LogType::_ERROR, L"[%d] DBServer Occur DanglingPointer %s!!!\r\n", g_Config.nManagedID, strReason.c_str());
						bLog = false;
					}
				}
			}

			if( bDelete == false )
			{
				g_Log.Log(LogType::_ERROR, L"[%d] DBServer DanglingPointer Delete Fail!!!\r\n", g_Config.nManagedID );
			}
#else

			CConnection *pCon = (CConnection*)pSocketContext->GetParam();

			g_Log.Log(LogType::_NORMAL, L"[OnDisconnected] Socket:%d, Ip:%S\r\n", pSocketContext->m_Socket, pCon->GetIp());

			pCon->SetActive(false);
			pCon->SetDelete(true);
			ClearSocketContext(pSocketContext);
			pCon->SetSocketContext(NULL, NULL);

#endif	// #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
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
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	CScopeInterlocked Scope( &pSocketContext->m_lActiveCount );
	if( Scope.bIsDelete() )
		return;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )

	CConnection* pCon = (CConnection*)pSocketContext->GetParam();

	if( pSocketContext->m_dwKeyParam == CONNECTIONKEY_LOG )
	{
		if (pCon)
		{
			int iRet = pCon->AddRecvData( pSocketContext );
			if( iRet > 0 )
			{
				pCon->FlushRecvData(0);
			}
		}

		return;
	}

	int			iPushCount		= 0;
	const char* pRecvBuffer		= pSocketContext->m_RecvIO.buffer;
	UINT		uiRecvBufferLen	= pSocketContext->m_RecvIO.Len;

	DN_ASSERT( m_vCallEvent.size() == m_vSpinBuffer.size(), "m_vCallEvent.size() == m_vSpinBuffer.size()" );
	
	std::vector<bool> vCallEvent;
	vCallEvent.reserve( m_vSpinBuffer.size() );
	vCallEvent.assign( m_vSpinBuffer.size(), 0 );

	while( uiRecvBufferLen )
	{
		static CTcpRecvSpinBuffer sTcpSpinBuffer(1,0);
		UINT uiPacketLen = 0;
		uiPacketLen = sTcpSpinBuffer.GetCompletePacketLength( pRecvBuffer, uiRecvBufferLen );
		if( uiPacketLen <= 0 )
			break;

		const DNTPacket* pPacket = reinterpret_cast<const DNTPacket*>(pRecvBuffer);

		DN_ASSERT( pPacket->iLen > 0, "pPacket->iLen" );

		BYTE cThreadID;
		memcpy( &cThreadID, pPacket->buf, sizeof(BYTE) );

		DN_ASSERT( cThreadID < m_vSpinBuffer.size(), "cThread < m_vSpinBuffer.size()" );

		if( cThreadID < m_vSpinBuffer.size() )
		{
			m_vSpinBuffer[cThreadID]->Lock();
			int iRet = m_vSpinBuffer[cThreadID]->RawPush( &pSocketContext, sizeof(CSocketContext*) );
			DN_ASSERT( iRet == 0, "m_vSpinBuffer[cThreadID]->RawPush() 1" );
			iRet = m_vSpinBuffer[cThreadID]->RawPush( pRecvBuffer, uiPacketLen );
			DN_ASSERT( iRet == 0, "m_vSpinBuffer[cThreadID]->RawPush() 2" );
			m_vSpinBuffer[cThreadID]->UnLock();
		}
		else
		{
			CConnection* pCon = static_cast<CConnection*>(pSocketContext->GetParam());
			g_Log.Log( LogType::_ERROR, L"DB ThreadID Error!!! MainCmd:%d SubCmd:%d ThreadID:%d/%d ConKeyParam=%d PacketLen=%d RemainBufLen=%d IP=%s\r\n", (int)pPacket->cMainCmd, (int)pPacket->cSubCmd, (int)cThreadID, m_vSpinBuffer.size(), pSocketContext->m_dwKeyParam, uiPacketLen, uiRecvBufferLen, pCon?pCon->GetwszIp():L"NULL" );
			
			pRecvBuffer		+= uiPacketLen;
			uiRecvBufferLen	-= uiPacketLen;
			continue;
		}

		pRecvBuffer		+= uiPacketLen;
		uiRecvBufferLen	-= uiPacketLen;
		++iPushCount;

		InterlockedExchangeAdd( &(static_cast<CDNConnection*>(pCon)->m_lDBQueueRemainSize), uiPacketLen );

		vCallEvent[cThreadID] = true;
	}

	if( iPushCount > 0 )
	{
		pSocketContext->m_RecvIO.Len = uiRecvBufferLen;
		if( uiRecvBufferLen )
			memmove( &pSocketContext->m_RecvIO.buffer, pRecvBuffer, uiRecvBufferLen );
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

CSpinBuffer* CDNIocpManager::GetSpinBuffer( UINT uiIndex )
{
	DN_ASSERT( m_vSpinBuffer.size() > uiIndex, "m_vSpinBuffer.size() > uiIndex" );
	return m_vSpinBuffer[uiIndex];
}

#if defined( PRE_FIX_67546 )

void CDNIocpManager::AddVillageGameConnection( CDNConnection* pCon )
{
	m_vVillageGameConnection.push_back(pCon);
}

void CDNIocpManager::DelVillageGameConnection( CDNConnection* pCon )
{
	for(std::vector<CDNConnection*>::iterator itor = m_vVillageGameConnection.begin();itor != m_vVillageGameConnection.end();itor++ )
	{
		if( *itor == pCon )
		{
			m_vVillageGameConnection.erase(itor);
			break;
		}
	}	
}

CDNConnection* CDNIocpManager::GetVillageGameConnection( int nIndex )
{
	if( nIndex >= m_vVillageGameConnection.size() )
		return NULL;
	
	return m_vVillageGameConnection[nIndex];
}

void CDNIocpManager::AddConnectionCount( int nIndex )
{
	if( nIndex >= THREADMAX )
		return;
	m_nConnectionCount[nIndex]++;
}

void CDNIocpManager::GetConnectionCount( int *ConnectionCount )
{
	for( int i=0;i<THREADMAX;i++ )
	{
		ConnectionCount[i] = m_nConnectionCount[i];
	}
}

#endif