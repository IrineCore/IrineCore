#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"


//-----------------------------------------------
//				CONTROL_TCP
//-----------------------------------------------


inline void SendConnectVillage(UINT nSessionID, UINT nAccountDBID, INT64 biCertifyingKey)
{
	FUNC_LOG();
	CSConnectVillage Connect;
	memset(&Connect, 0, sizeof(CSConnectVillage));

	Connect.nSessionID = nSessionID;
	Connect.nAccountDBID = nAccountDBID;
	Connect.biCertifyingKey = biCertifyingKey;
	CClientSessionManager::GetInstance().GetLocalIP( Connect.wszVirtualIp );

	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_CONNECTVILLAGE, (char*)&Connect, sizeof(CSConnectVillage));
}

inline void SendVillageReady(bool bFirst, bool bChannelMove)
{
	FUNC_LOG();
	CSVillageReady Ready;
	memset(&Ready, 0, sizeof(CSVillageReady));

	Ready.boFirst = bFirst;
	Ready.boChannelMove = bChannelMove;

	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_VILLAGEREADY, (char*)&Ready, sizeof(CSVillageReady));
}

inline void SendReconnectLogin(UINT nSessionID, UINT nAccountDBID, INT64 biCertifyingKey)
{
	FUNC_LOG();
	CSReconnectLogin Reconnect;
	memset(&Reconnect, 0, sizeof(CSReconnectLogin));

	Reconnect.nSessionID = nSessionID;
	Reconnect.nAccountDBID = nAccountDBID;
	Reconnect.biCertifyingKey = biCertifyingKey;


	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_RECONNECTLOGIN, (char*)&Reconnect, sizeof(CSReconnectLogin));
}

inline void SendReconnectLogin()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_RECONNECTLOGIN, NULL, 0);
}

#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
inline void SendAbandonStage(bool bIsPartyOut, bool bIntendedDisconnect)
#else
inline void SendAbandonStage(bool bIsPartyOut)
#endif
{
	FUNC_LOG();

	CSAbandonStage packet;
	memset(&packet, 0, sizeof(CSAbandonStage));

	packet.bIsPartyOut = bIsPartyOut;
#ifdef PRE_FIX_GAMEQUIT_TO_VILLAGE
	packet.bIntenedDisconnect = bIntendedDisconnect;
#endif

	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_ABANDONSTAGE, (char*)&packet, sizeof(packet));
}

inline void SendMovePvPVillageToPvPLobby()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_MOVE_PVPVILLAGETOLOBBY, NULL, 0 );
}

inline void SendMovePvPLobbyToPvPVillage()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_MOVE_PVPLOBBYTOPVPVILLAGE, NULL, 0 );
}

inline void SendMovePvPGameToPvPLobby()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_MOVE_PVPGAMETOPVPLOBBY, NULL, 0 );
}

#ifdef PRE_ADD_VIP
inline void SendVIPAutomaticPay(bool bAutoPay)
{
	FUNC_LOG();

	CSVIPAutomaticPay packet;
	memset(&packet, 0, sizeof(CSVIPAutomaticPay));
	packet.bAutomaticPay = bAutoPay;

	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_VIPAUTOMATICPAY, (char*)&packet, sizeof(packet));
}
#endif // PRE_ADD_VIP

inline void SendTcpPing( DWORD dwTick )
{
	FUNC_LOG();

	CSTCPPing TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.dwTick = dwTick;

	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_TCP_PING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

inline void SendClientPing( DWORD dwTick )
{
	CSClientPing TxPacket;
	TxPacket.dwTick = dwTick;
	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_CLIENT_PING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

