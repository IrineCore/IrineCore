#include "stdafx.h"
#include "DNLogConnection.h"
#include "DNIocpManager.h"
#include "Log.h"
#include "Util.h"
#include "DNServerPacket.h"
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
#include "DNServerProtocol.h"
#endif

#if defined( _GAMESERVER )
#include "DNGameRoom.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "PvPGameMode.h"
#include "DnPlayerActor.h"
#endif

CDNLogConnection* g_pLogConnection;

CDNLogConnection::CDNLogConnection(void) : m_dwReconnectTick(0)
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
, m_dwLastRecvTick(0)
, m_dwSendSYNTick(0)
, m_dwSendSYNACKTick(0)
#endif
{
#if defined(_FINAL_BUILD)
	Init( 1024*1024, 1024*1024 );
#else	// #if defined(_FINAL_BUILD)
	Init( 1024*100, 1024*100 );
#endif	// #if defined(_FINAL_BUILD)
}

CDNLogConnection::~CDNLogConnection(void)
{
}

#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
int CDNLogConnection::MessageProcess(int iMainCmd, int iSubCmd, char *pData, int iLen)
{
	if( iMainCmd == LOG_HEARTBEAT )
	{
		switch(iSubCmd)
		{
		case SYN:
			{				
				m_dwSendSYNACKTick = timeGetTime();
				AddSendData(LOG_HEARTBEAT, SYN_ACK, NULL, 0);
			}
			break;
		case SYN_ACK:
			{
				m_dwSendSYNTick = 0;
				AddSendData(LOG_HEARTBEAT, ACK, NULL, 0);
			}
			break;
		case ACK:
			{
				m_dwSendSYNACKTick = 0;
			}
			break;
		}
	}
	
	m_dwLastRecvTick = timeGetTime();
	return ERROR_NONE;
}

void CDNLogConnection::ResetLiveCheck()
{
	m_dwSendSYNTick = 0;
	m_dwSendSYNACKTick = 0;
	m_dwLastRecvTick = 0;		
};

void CDNLogConnection::LiveCheck(DWORD CurTick)
{
	if( !GetActive() || GetConnecting() )
		return;

	if( m_dwSendSYNTick > 0 && m_dwSendSYNTick + 1000*60*2 < CurTick ||
		m_dwSendSYNACKTick > 0 && m_dwSendSYNACKTick + 1000*60*2 < CurTick )	//Wait for exactly 2 minutes.
	{
		g_Log.Log(LogType::_FILELOG, L"[LogServer Disconnected]Heart failure!!\r\n");
		if(m_pSocketContext)
			DetachConnection(L"[LogServer]Heart failure!\r\n");
		ResetLiveCheck();
		m_dwReconnectTick = CurTick;
		return;
	}

	if( m_dwSendSYNTick == 0 && m_dwLastRecvTick + CHECK_TCP_PING_TICK < CurTick )
	{
		m_dwSendSYNTick = CurTick;
		AddSendData(LOG_HEARTBEAT, SYN, NULL, 0);
	}
}

#endif

void CDNLogConnection::Reconnect(DWORD CurTick)
{
#if defined(PRE_ADD_LOGSERVER_HEARTBEAT)
	LiveCheck(CurTick);
#endif

	if (m_dwReconnectTick + 5000 < CurTick){
		m_dwReconnectTick = CurTick;

		if (!GetActive() && !GetConnecting() )
		{
			SetConnecting(true);
			if (g_pIocpManager->AddConnectionEx(this, CONNECTIONKEY_LOG, m_szIP, m_wPort) < 0)
			{
				SetConnecting(false);
				g_Log.Log(LogType::_FILELOG, L"LogServer Fail(%S, %d)\r\n", m_szIP, m_wPort);
			}
			else {
				g_Log.Log(LogType::_FILELOG, L"LogServer(%S, %d) Connecting..\r\n", m_szIP, m_wPort);
			}
		}
	}
}

