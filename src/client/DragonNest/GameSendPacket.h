#pragma once

#include "GameClientSession.h"
#include "ClientSessionManager.h"
#include "MATransAction.h"
#include "NetSocketRUDP.h"
#include "DNPacket.h"


inline void SendGameTcpConnect(UINT nSessionID, UINT nAccountDBID, INT64 biCertifyingKey)
{
	//Send the info from here for now
	CSConnectGame packet;
	memset(&packet, 0, sizeof(packet));
	packet.nSessionID = nSessionID;
	packet.nAccountDBID = nAccountDBID;
	packet.biCertifyingKey = biCertifyingKey;
	
	CClientSessionManager::GetInstance().SendTcpPacket(CS_SYSTEM, eSystem::CS_CONNECTGAME, (char*)&packet, sizeof(packet));
	WriteLog( 0, ", Info, SendGameTcpConnect\n" );
}

#ifdef PRE_MOD_INDUCE_TCPCONNECT
inline void SendGameTcpConnectReq()
{
	CClientSessionManager::GetInstance().SendPacket( CS_SYSTEM, eSystem::CS_TCP_CONNECT_REQ, NULL, 0, _RELIABLE);
	WriteLog( 0, ", Info, SendGameTcpConnectReq\n" );
}
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

inline void SendGameActorMsg(UINT nSessionID, USHORT nProtocol, BYTE *pBuf, int nSize, int iPrior, BYTE cSeqLevel)
{
	CSActorMessage packet;
	memset(&packet, 0, sizeof(CSActorMessage));

	packet.nSessionID = nSessionID;
	memcpy(packet.cBuf, pBuf, nSize);

	CClientSessionManager::GetInstance().SendPacket( CS_ACTOR, nProtocol, (char*)&packet, \
		sizeof(packet) - sizeof(packet.cBuf) + nSize, iPrior, cSeqLevel);
}

inline void SendGameActorBundleMsg(UINT nSessionID, USHORT nCount, BYTE *pBuf, int nSize, int iPrior, BYTE cSeqLevel)
{
	CSActorBundleMessage packet;

	packet.nSessionID = nSessionID;
	memcpy(packet.cBuf, pBuf, nSize);

	CClientSessionManager::GetInstance().SendPacket( CS_ACTORBUNDLE, nCount, (char*)&packet, sizeof(packet) - sizeof(packet.cBuf) + nSize, \
		iPrior, cSeqLevel );
}

inline void SendReady2Receive(UINT nSessionID)
{
	CSReady2Recieve packet;
	packet.nSessionID = nSessionID;
	CClientSessionManager::GetInstance().GetLocalIP( packet.wszVirtualIp );

	CClientSessionManager::GetInstance().SendPacket(CS_SYSTEM, eSystem::CS_READY_2_RECIEVE, (char*)&packet, sizeof(packet), _RELIABLE);
}

inline void SendGameSyncWait(UINT nUID, int iPrior)
{
	CSSyncWait packet;
	packet.nSessionID = nUID;

	CClientSessionManager::GetInstance().SendPacket(CS_ROOM, eRoom::CS_SYNC_WAIT, (char*)&packet, sizeof(packet), iPrior);
}


//Is this needed?
/*inline void SendGameSyncStart(BYTE cMode, const char *pIp, USHORT nPort)
{
	TUDP_HEADER Header;
	memset(&Header, 0, sizeof(TUDP_HEADER));

	Header.wLen = sizeof(TUDP_HEADER);
	Header.nUniqueID = CClientSessionManager::GetInstance().GetConnectUniqueID();
	Header.Cmd = PT_GAME_SYNC_START;
	//##Header.Mode = cMode;

	SOCKADDR_IN Addr;
	memset(&Addr, 0, sizeof(SOCKADDR_IN));

	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = inet_addr(pIp);
	Addr.sin_port = htons(nPort);

	//##CClientSessionManager::GetInstance().GetUdpSocket()->Send(cMode, (char*)&Header, Header.wLen, &Addr);
	//CClientSessionManager::GetInstance().GetUdpSocket()->Send(Addr, (char*)&Header, Header.wLen, cMode);
	_SendMsg((char*)&Header, Header.wLen, cMode, pIp, nPort);
}*/

inline void SendGamePropMsg(UINT nUID, USHORT nProtocol, BYTE *pBuf, int nSize, int iPrior)
{
	CSPropMessage packet;
	packet.nUniqueID = nUID;
	memcpy(packet.cBuf, pBuf, nSize);

	CClientSessionManager::GetInstance().SendPacket(CS_PROP, nProtocol, (char*)&packet, 
		sizeof(packet) - sizeof(packet.cBuf) + nSize, iPrior);
}

/*
inline void SendOKDungeonClear(UINT nUID)
{
	CSOKDungeonClear packet;
	packet.nSessionID = nUID;

	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_OK_DUNGEONCLEAR, (char*)&packet, sizeof(packet), _RELIABLE );
}
*/

inline void SendRequestDungeonFailed()
{
	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_REQUEST_DUNGEONFAILED, NULL, 0, _RELIABLE );
}

//inline void SendWarpDungeonClear( bool bWarpVillage )
//{
//	CSWarpDungeonClear packet;
//	packet.bWarpVillage = bWarpVillage;
//
//	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_WARP_DUNGEONCLEAR, (char*)&packet, sizeof(packet), _RELIABLE );
//}

inline void SendWarpDungeon( bool bWarpVillage )
{
	CSWarpDungeon packet;
	packet.bWarpVillage = bWarpVillage;

	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_WARP_DUNGEON, (char*)&packet, sizeof(packet), _RELIABLE );
}

inline void SendCompleteCutScene( UINT nUID )
{
	CClientSessionManager::GetInstance().SendPacket( CS_QUEST, eQuest::CS_COMPLETE_CUTSCENE, NULL, 0, _RELIABLE);
}

inline void SendSkipCutScene( UINT nUID )
{
	CClientSessionManager::GetInstance().SendPacket( CS_QUEST, eQuest::CS_SKIP_CUTSCENE, NULL, 0, _RELIABLE);
}

inline void SendSelectRewardItem( char cItemIndex )
{
	CSSelectRewardItem packet;
	packet.cItemIndex = cItemIndex;

	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_SELECT_REWARDITEM, (char*)&packet, sizeof(packet), _RELIABLE );
}

inline void SendIdentifyRewardItem( char cItemIndex )
{
	CSIdentifyRewardItem packet;
	packet.cItemIndex = cItemIndex;

	CClientSessionManager::GetInstance().SendPacket( CS_ROOM, eRoom::CS_IDENTIFY_REWARDITEM, (char*)&packet, sizeof(packet), _RELIABLE );
}