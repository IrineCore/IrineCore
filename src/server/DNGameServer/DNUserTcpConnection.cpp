
#include "stdafx.h"
#include "DNUserTcpConnection.h"
#include "DNUserSession.h"
#include "DNGameRoom.h"
#include "DNRUDPGameServer.h"
#include "DNGameServerManager.h"
#include "DNSecure.h"
#include "SpinBuffer.h"

extern TGameConfig g_Config;

CDNTcpConnection::CDNTcpConnection(CDNGameServerManager * pManager)
{
	m_bAttached = false;
	m_bAttachVerify = false;
	m_pSession = NULL;
	m_pManager = pManager;
	Init(1024 * 100, 1024 * 100);
}

CDNTcpConnection::~CDNTcpConnection()
{
	m_bAttached = false;
	m_bAttachVerify = false;
	m_bAttached = false;
	m_pSession = NULL;
	m_pManager = NULL;
}

void CDNTcpConnection::TcpConnected(char * pData, int iLen)
{
	CSConnectGame * pPacket = (CSConnectGame*)pData;
	CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByUID(pPacket->nSessionID);
	if (pServer)
	{
		++pServer->m_uiConnectSuccessCount;

		CDNUserSession * pSession = pServer->GetSession(pPacket->nSessionID);
		if (pSession != NULL)
		{
//			pSession->m_nAccountDBID = pPacket->nAccountDBID; // Needs additional implementation??? (Should we not do it this way???)
			pSession->m_biCertifyingKey = pPacket->biCertifyingKey;

#ifdef _SKIP_BLOCK
			if (pSession->GetTcpConnection() != NULL)
			{
				_DANGER_POINT();
				if (this != pSession->GetTcpConnection())
					_ASSERT(0);
			}
			else
			{
				if (pSession->TCPConnected(this))
				{
					if (SetSession(pSession))
					{
						m_bAttachVerify = true;
						return;
					}
					else
						g_Log.Log(LogType::_ERROR, L"Connect|Session Attached Already\n");
				}
				else
					g_Log.Log(LogType::_ERROR, L"Connect|RUDP Connect Notyet\n");
			}
#else
			if (pSession->GetTcpConnection() != NULL)	
			{
				//Must not appear for now!
				_DANGER_POINT();
				if (this != pSession->GetTcpConnection())
					m_pManager->PushOrphanPtr(pSession->GetTcpConnection());	//Handled just in case it occurs.
				else
					_ASSERT(0);				//If they are equal... wait... then it gets complicated...
			}

			if (pSession->TCPConnected(this) == true)
			{
				//Timing processing
				//m_pSession = pSession;
				g_pGameServerManager->PushToEjectTcpConnection(this, pSession);
			}
			else
				g_Log.Log(LogType::_ERROR, L"Connect|RUDP Connect Fail\n");
#endif
		}
		else
		{
			g_Log.Log( LogType::_GAMECONNECTLOG, 0, pPacket->nAccountDBID, 0, pPacket->nSessionID, L"pSession Not NULL!!\n" );
		}
	}
	else
	{
		g_Log.Log( LogType::_GAMECONNECTLOG, 0, pPacket->nAccountDBID, 0, pPacket->nSessionID, L"pServer NULL!!\n" );
	}
}

void CDNTcpConnection::TcpDisconnected(wchar_t *pIdent)
{
	if (m_pSession)
	{
		g_Log.Log(LogType::_NORMAL, m_pSession, L"TcpDisconnected(%s) SessionState=%d RoomState=%d\r\n", 
			pIdent, m_pSession->GetState(), m_pSession->GetGameRoom() ? m_pSession->GetGameRoom()->GetRoomState() : -1);
	}

	DNEncryptPacketSeq EnPacket = { 0, };
	EnPacket.Packet.iLen = static_cast<unsigned short>(sizeof(DNTPacketHeader));
	EnPacket.Packet.cMainCmd = IN_DISCONNECT;
	EnPacket.Packet.cSubCmd = CONNECTIONKEY_USER;

	EnPacket.nLen = EnPacket.Packet.iLen + sizeof(BYTE) + sizeof(USHORT);
	CDNSecure::GetInstance().Tea_encrypt( reinterpret_cast<char*>(&EnPacket.Packet), EnPacket.Packet.iLen );

	AddRecvData( EnPacket );
}

bool CDNTcpConnection::SetSession(CDNUserSession * pSession)
{
	if (m_pSession)	return false;
	m_pSession = pSession;
	return true;
}

int CDNTcpConnection::MessageProcess(int iMainCmd, int iSubCmd, char * pData, int iSize)
{
	if (m_pSession)
	{
		if (iMainCmd == IN_SC_ITEM)
		{
			_DANGER_POINT();
			return ERROR_NONE;
		}
#ifdef PRE_MOD_INDUCE_TCPCONNECT
#else		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
		if (iMainCmd == CS_SYSTEM && iSubCmd == eSystem::CS_CONNECTGAME)
		{
			g_Log.Log( LogType::_GAMECONNECTLOG, L"MessageProcess m_pSession()!!\n" );
		}
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

		if (m_pSession->GetGameRoom())
			m_pSession->GetGameRoom()->OnDispatchMessage(m_pSession, iMainCmd, iSubCmd, pData, iSize, 0);
	}
	else if (iMainCmd == CS_SYSTEM && iSubCmd == eSystem::CS_CONNECTGAME)
	{
		CSConnectGame * pPacket = (CSConnectGame*)pData;
		g_Log.Log( LogType::_GAMECONNECTLOG, 0, pPacket->nAccountDBID, 0, pPacket->nSessionID, L"[%d] CS_CONNECTGAME()!!\n", g_Config.nManagedID );
		TcpConnected(pData, iSize);
	}
	else
		_DANGER_POINT();
	return ERROR_NONE;
}

void CDNTcpConnection::SetAttachSession()
{
	if (m_bAttached)
	{
		_DANGER_POINT();
		return;
	}
	
	m_bAttached = true;
}
