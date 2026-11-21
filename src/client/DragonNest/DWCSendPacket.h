#pragma once

#if defined( PRE_ADD_DWC )

#include "DNPacket.h"
#include "ClientSessionManager.h"

inline void SendDWCCreateTeam( const WCHAR *wszTeamName )
{
	CSDWCCreateTeam packet;
	memset( &packet, 0, sizeof( packet ) );
	if( wszTeamName )
		_wcscpy( packet.wszTeamName, _countof( packet.wszTeamName ), wszTeamName, (int)wcslen( wszTeamName ) );

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_CREATETEAM, (char*)&packet, sizeof( packet ) );
}

inline void SendDWCLeaveTeam()
{
	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_LEAVETEAM, NULL, 0 );
}

inline void SendDWCInviteMember( const WCHAR *wszCharacterName )
{
	CSDWCInviteMember packet;
	memset( &packet, 0, sizeof( packet ) );
	if( wszCharacterName )
		_wcscpy( packet.wszCharacterName, _countof( packet.wszCharacterName ), wszCharacterName, (int)wcslen( wszCharacterName ) );

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_INVITE_MEMBER, (char*)&packet, sizeof( packet ) );
}

inline void SendDWCInviteMemberAck( UINT nTeamID, UINT nFromSessionID, bool bAccept )
{
	CSDWCInviteMemberAck packet;
	memset( &packet, 0, sizeof( packet ) );
	packet.nTeamID = nTeamID;
	packet.nFromSessionID = nFromSessionID;
	packet.bAccept = bAccept;

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_INVITE_MEMBER_ACK, (char*)&packet, sizeof( packet ) );
}

inline void SendDWCGetTeamInfo( bool bNeedMemberList )
{
	CSDWCGetTeamInfo packet;
	memset( &packet, 0, sizeof( packet ) );
	packet.bNeedMemberList = bNeedMemberList;

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_GET_TEAMINFO, (char*)&packet, sizeof( packet ) );
}

inline void SendDWCGetRankPage( int nPageNum )
{
	CSDWCGetRankPage packet;
	memset( &packet, 0, sizeof( packet ) );
	packet.nPageNum = nPageNum;

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_GET_RANKPAGE, (char*)&packet, sizeof( packet ) );
}

inline void SendDWCFindRank( BYTE cFindType, const WCHAR *wszFindName )
{
	CSDWCFindRank packet;
	memset( &packet, 0, sizeof( packet ) );
	packet.cFindType = cFindType;
	if( wszFindName )
		_wcscpy( packet.wszFindName, _countof( packet.wszFindName ), wszFindName, (int)wcslen( wszFindName ) );

	CClientSessionManager::GetInstance().SendPacket( CS_DWC, eDWC::CS_DWC_FIND_RANK, (char*)&packet, sizeof( packet ) );
}

#endif // PRE_ADD_DWC
