#include "stdafx.h"
#include "DNGuildGame.h"
#include "DNMasterConnectionManager.h"
#include "DNGameDataManager.h"

CDNGuildGame::CDNGuildGame() 
{

}

CDNGuildGame::~CDNGuildGame() 
{

}

void CDNGuildGame::Reset()
{

}

void CDNGuildGame::SendGuildMemberLoginList(SCGuildMemberLoginList* pPacket)
{
	// Send packet to all guild members
	MAGuildMemberLoginList Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.GuildUID = pPacket->GuildUID;
	for (int aIndex = 0 ; MIN(GUILDSIZE_MAX, pPacket->nCount) > aIndex ; ++aIndex) {
		if (GUILDSIZE_MAX <= Packet.nCount) {
			break;
		}

		Packet.List[Packet.nCount] = pPacket->List[aIndex];
		++Packet.nCount;
	}	

	int nLen = static_cast<int>(sizeof(Packet) - sizeof(Packet.List) + sizeof(Packet.List[0]) * Packet.nCount);

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer) {
			pServer->StoreExternalBuffer(0, MAGA_GUILDMEMBLOGINLIST, 0, reinterpret_cast<char*>(&Packet), nLen, EXTERNALTYPE_MASTER, GetUID().nWorldID);
		}
	}
}

bool CDNGuildGame::CheckGuildResource(TGuildMember* pGuildMemeber)
{
	if (false == CDNGuildBase::CheckGuildResource(pGuildMemeber))
		return false;

	CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerByAID(pGuildMemeber->nAccountDBID);
	if (pServer) 
		return false;

	// Continuously query the member list
	return true;
}
void CDNGuildGame::SendAddGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszCharacterName, TP_JOB nJob, char cLevel, __time64_t pJoinDate, TCommunityLocation* pLocation)
{
	MAAddGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nManagedID = LONG_MAX;
	Packet.GuildUID = GetUID();
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nJob = nJob;
	Packet.cLevel = cLevel;
	Packet.JoinDate = pJoinDate;
	_wcscpy(Packet.wszCharacterName, _countof(Packet.wszCharacterName), lpwszCharacterName, (int)wcslen(lpwszCharacterName));
	if (pLocation)
		Packet.Location = (*pLocation);

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_ADDGUILDMEMBER, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendDelGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, bool bIsExiled)
{
	MADelGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nManagedID = LONG_MAX;
	Packet.GuildUID = GetUID();
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.bIsExiled = bIsExiled;

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_DELGUILDMEMBER, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendChangeGuildMemberInfo(UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText)
{
	MAChangeGuildMemberInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nManagedID = LONG_MAX;
	Packet.GuildUID = GetUID();
	Packet.nReqAccountDBID = nReqAccountDBID;
	Packet.nReqCharacterDBID = nReqCharacterDBID;
	Packet.nChgAccountDBID = nChgAccountDBID;
	Packet.nChgCharacterDBID = nChgCharacterDBID;
	Packet.btGuildMemberUpdate = btGuildMemberUpdate;
	Packet.Int1 = iInt1;
	Packet.Int2 = iInt2;
	Packet.Int64 = biInt64;
	if (pText) {
		_wcscpy(Packet.Text, _countof(Packet.Text), pText, (int)wcslen(pText));
	}
	
	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex)
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_CHANGEGUILDMEMBERINFO, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendGuildChat(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	MAGuildChat Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nManagedID = LONG_MAX;
	Packet.GuildUID = GetUID();
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;

	_wcscpy( Packet.wszChatMsg, _countof(Packet.wszChatMsg), lpwszChatMsg, nLen );
	Packet.nLen = nLen;
	
	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex)
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_GUILDCHAT, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
void CDNGuildGame::SendDoorsGuildChat(INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	MADoorsGuildChat Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.nManagedID = LONG_MAX;
	Packet.GuildUID = GetUID();
	Packet.nCharacterDBID = nCharacterDBID;

	_wcscpy( Packet.wszChatMsg, _countof(Packet.wszChatMsg), lpwszChatMsg, nLen );
	Packet.nLen = nLen;

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex)
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_GUILDCHAT_MOBILE, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

void CDNGuildGame::SendChangeGuildName(const wchar_t* wszGuildName)
{
	MAGuildChangeName Packet;
	memset(&Packet, 0, sizeof(Packet));

	memset(&Packet, 0, sizeof(MAGuildChangeName));
	Packet.GuildUID = GetUID();
	_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), wszGuildName, (int)wcslen(wszGuildName));

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex)
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_CHANGEGUILDNAME, 0, reinterpret_cast<char*>(&Packet), sizeof(MAGuildChangeName), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendChangeGuildMark(const MAGuildChangeMark *pPacket)
{
	MAGuildChangeMark Packet;
	memset(&Packet, 0, sizeof(MAGuildChangeMark));
	Packet.GuildUID = GetUID();
	Packet.wMark = pPacket->wMark;
	Packet.wMarkBG = pPacket->wMarkBG;
	Packet.wMarkBorder = pPacket->wMarkBorder;

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex)
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_CHANGEGUILDMARK, 0, reinterpret_cast<char*>(&Packet), sizeof(MAGuildChangeMark), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::UpdateMemberInfo(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	switch(pPacket->btGuildMemberUpdate) 
	{
	case GUILDMEMBUPDATE_TYPE_INTRODUCE:	// Change self-introduction (guild member self)
	case GUILDMEMBUPDATE_TYPE_ROLE:			// Change rank (Guild Master -> Guild Member ?)
	case GUILDMEMBUPDATE_TYPE_GUILDMASTER:	// Delegate guild master (guild master -> member ?)
		{
			// GA does not process this packet
		}
		DN_BREAK;
	default:
		{
			pSession->SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, 
				pPacket->btGuildMemberUpdate, ERROR_GENERIC_INVALIDREQUEST, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
		}
		DN_BREAK;
	}
}

void CDNGuildGame::UpdateGuildInfo(CDNUserSession* pSession, const TAChangeGuildInfo *pPacket)
{
	switch(pPacket->btGuildUpdate) 
	{
	case GUILDUPDATE_TYPE_ROLENAME:			// Change rank name
	case GUILDUPDATE_TYPE_ROLEAUTH:			// Change rank permissions
		{
			// GA does not process this packet
		}
		DN_BREAK;

	case GUILDUPDATE_TYPE_NOTICE:			// Change notice
		{
			{
				ScopeLock <CSyncLock> AutoLock (GetLock());
				_wcscpy(SetInfo()->wszGuildNotice, _countof(GetInfo()->wszGuildNotice), pPacket->Text, (int)wcslen(pPacket->Text));
			}
		}
		break;
	default:
		{
			pSession->SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, ERROR_GENERIC_INVALIDREQUEST, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text);
		}
		DN_BREAK;
	}

	// Notify guild members currently connected to this server
	SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, pPacket->nInt1, pPacket->nInt2, pPacket->nInt3, pPacket->nInt4, pPacket->biInt64, pPacket->Text);

	// Specify to also update general guild info on other servers via MA
	g_pMasterConnectionManager->SendChangeGuildInfo(pSession->GetWorldSetID(), GetUID(), pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, pPacket->nInt1, pPacket->nInt2, pPacket->nInt3, pPacket->nInt4, pPacket->biInt64, pPacket->Text);
}

void CDNGuildGame::UpdateWarInfoOnAllMember(short wScheduleID, BYTE cTeamColorCode)
{
	if (g_pMasterConnectionManager->GetGuildWarScheduleID(GetUID().nWorldID) != wScheduleID)
		return;

	m_Info.GuildView.wWarSchduleID = wScheduleID;
	m_Info.GuildView.cTeamColorCode = cTeamColorCode;

	MAEnrollGuildWar Packet;
	Packet.GuildUID		= GetUID();
	Packet.wScheduleID	= wScheduleID;
	Packet.cTeamColorCode = cTeamColorCode;
	
	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex (iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_ENROLLGUILDWAR, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendGuildLevelUp (int nLevel)
{
	MAGuildLevelUp Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.GuildUID		= GetUID();
	Packet.nLevel = nLevel;

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex (iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_GUILDLEVELUP, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}
}

void CDNGuildGame::SendUpdateGuildExp(BYTE cPointType, int nPointValue, int nGuildExp, INT64 biCharacterDBID, int nMissionID)
{
	MAGuildExp Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.GuildUID		= GetUID();
	Packet.cPointType = cPointType;
	Packet.nPointValue = nPointValue;
	Packet.nGuildExp = nGuildExp;
	Packet.biCharacterDBID = biCharacterDBID;
	Packet.nMissionID = nMissionID;

	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex (iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAGA_UPDATEGUILDEXP, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_MASTER, GetUID().nWorldID);
	}	
};

void CDNGuildGame::SendGuildRewardItem( TGuildRewardItem* GuildRewardItem )
{
	// QUERY_GET_GUILDREWARDITEM	
	DBGetGuildRewardItem Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.GuildUID		= GetUID();
	
	for (int iIndex = 0 ; g_pGameServerManager->GetGameServerSize() > iIndex ; ++iIndex) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex (iIndex);
		if (pServer)
			pServer->StoreExternalBuffer(0, MAINCMD_GUILD, QUERY_GET_GUILDREWARDITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet), EXTERNALTYPE_DATABASE, GetUID().nWorldID);
	}	
}
