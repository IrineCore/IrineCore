#pragma once

#include "DNPacket.h"
#include "ClientSessionManager.h"
#include "ClientSocket.h"

//-----------------------------------------------
//				CONTROL_TCP
//-----------------------------------------------

inline void SendEnter()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_ENTER, NULL, 0);
}

inline void SendCompleteLoading()
{
	FUNC_LOG();
	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_COMPLETELOADING, NULL, 0);
}

inline void SendAddQuickSlot(BYTE cSlotIndex, BYTE cSlotType, INT64 biID)
{
	CSAddQuickSlot AddSlot;
	memset(&AddSlot, 0, sizeof(CSAddQuickSlot));

	AddSlot.cSlotIndex = cSlotIndex;
	AddSlot.cSlotType = cSlotType;
	AddSlot.nID = biID;

	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_ADDQUICKSLOT, (char*)&AddSlot, sizeof(CSAddQuickSlot));
}

inline void SendDelQuickSlot(BYTE cSlotIndex)
{
	CSDelQuickSlot DelSlot;
	memset(&DelSlot, 0, sizeof(CSDelQuickSlot));

	DelSlot.cSlotIndex = cSlotIndex;

	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_DELQUICKSLOT, (char*)&DelSlot, sizeof(CSDelQuickSlot));
}

inline void SendLookUser(const WCHAR * pName)
{
	CSLoockUser packet;
	memset(&packet, 0, sizeof(packet));
	_wcscpy( packet.wszCharacterName, _countof(packet.wszCharacterName), pName, (int)wcslen(pName) );
	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_LOOKUSER, (char*)&packet, sizeof(packet));
}

inline void SendActorMsg(UINT dwSessionID, UINT nActorProtocol, BYTE *pBuf, int nSize) // SendSessionID
{
	CSActorMessage ActorMsg;
	memset( &ActorMsg, 0, sizeof(CSActorMessage) );	
	
	ActorMsg.nSessionID = dwSessionID; // SendSessionID
	memcpy( ActorMsg.cBuf, pBuf, nSize );

	CClientSessionManager::GetInstance().SendPacket(CS_ACTOR, nActorProtocol, (char*)&ActorMsg, sizeof(ActorMsg) - sizeof(ActorMsg.cBuf) + nSize);
}

inline void SendPropMsg( UINT nUID, UINT nProtocol, BYTE *pBuf, int nBufSize )
{
	CSPropMessage PropMsg;
	memset( &PropMsg, 0, sizeof(CSPropMessage) );

	PropMsg.nUniqueID = nUID;
	memcpy( PropMsg.cBuf, pBuf, nBufSize );
	CClientSessionManager::GetInstance().SendPacket(CS_PROP, nProtocol, (char*)&PropMsg, sizeof(CSPropMessage));
}

inline void SendChatMsg( eChatType eType, LPCWSTR wszChatMsg, INT64 biItemSerial = 0 )
{
	CSChat ChatMsg;
	memset(&ChatMsg, 0, sizeof(CSChat));

	ChatMsg.eType = eType;
	ChatMsg.biItemSerial = biItemSerial;
	ChatMsg.nLen = (short)wcslen( wszChatMsg );
	_wcscpy( ChatMsg.wszChatMsg, _countof(ChatMsg.wszChatMsg), wszChatMsg, (int)wcslen(wszChatMsg) );

	CClientSessionManager::GetInstance().SendPacket(CS_CHAT, eChat::CS_CHATMSG, (char*)&ChatMsg, sizeof(CSChat) - sizeof(ChatMsg.wszChatMsg) + (ChatMsg.nLen * 2));
}

inline void SendChatPrivateMsg( LPCWSTR wszToCharacterName, LPCWSTR wszChatMsg )
{
	CSChatPrivate ChatMsg;
	memset(&ChatMsg, 0, sizeof(CSChatPrivate));

	ChatMsg.nLen = (short)wcslen( wszChatMsg );
	_wcscpy( ChatMsg.wszToCharacterName, _countof(ChatMsg.wszToCharacterName), wszToCharacterName, (int)wcslen(wszToCharacterName) );
	_wcscpy( ChatMsg.wszChatMsg, _countof(ChatMsg.wszChatMsg), wszChatMsg, (int)wcslen(wszChatMsg) );

	CClientSessionManager::GetInstance().SendPacket(CS_CHAT, eChat::CS_CHAT_PRIVATE, (char*)&ChatMsg, sizeof(CSChatPrivate) - sizeof(ChatMsg.wszChatMsg) + (ChatMsg.nLen * 2));
}

#if defined(PRE_ADD_ANTI_CHAT_SPAM)
inline void SendChatSpammer(bool bSpammer)
{	
	CClientSessionManager::GetInstance().SendPacket(CS_CHAT, eChat::CS_CHAT_SPAMMER, (char*)&bSpammer, sizeof(bool));
}
#endif // #if defined(PRE_ADD_ANTI_CHAT_SPAM)

inline void SendRaidNoticeMsg(LPCWSTR wszChatMsg)
{
	CSChat ChatMsg;
	memset(&ChatMsg, 0, sizeof(CSChat));

	ChatMsg.eType = CHATTYPE_RAIDNOTICE;
	ChatMsg.biItemSerial = 0;
	ChatMsg.nLen = (short)wcslen( wszChatMsg );
	_wcscpy( ChatMsg.wszChatMsg, _countof(ChatMsg.wszChatMsg), wszChatMsg, (int)wcslen(wszChatMsg) );

	CClientSessionManager::GetInstance().SendPacket(CS_CHAT, eChat::CS_CHATMSG, (char*)&ChatMsg, sizeof(CSChat) - sizeof(ChatMsg.wszChatMsg) + (ChatMsg.nLen * 2));
}

inline void SendCloseWindow( eWindowState State )
{
	CSCloseUIWindow CloseWindow;
	CloseWindow.WindowState = State;
	CClientSessionManager::GetInstance().SendPacket(CS_CHAR, eChar::CS_CLOSE_UIWINDOW, (char*)&CloseWindow, sizeof(CSCloseUIWindow));
}


#ifdef PRE_ADD_DONATION // Donation system.

// Send donation packet.
inline void SendMakeDonationsMsg( INT64 nCoin )
{
	CSDonate msgDonate;
	memset( &msgDonate, 0, sizeof(CSDonate) );
	msgDonate.nCoin = nCoin;
	CClientSessionManager::GetInstance().SendPacket( eCSMainCmd::CS_DONATION, eDonation::CS_DONATE, (char*)&msgDonate, sizeof(CSDonate) );
}

// Send ranking list refresh packet.
inline void SendRefreshRankerList()
{
	CSDonationRanking msgRanking;
	CClientSessionManager::GetInstance().SendPacket( eCSMainCmd::CS_DONATION, eDonation::CS_DONATION_RANKING, (char*)&msgRanking, sizeof(CSDonationRanking) );
}
 
#endif
