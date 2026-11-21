#include "stdafx.h"
#include "MPTransAction.h"

#include "GameSendPacket.h"

#include "DnWorldProp.h"
#include "TaskManager.h"

#include "DNGameRoom.h"

MPTransAction::MPTransAction()
{
	m_bIsValidProp = true;
	m_pProp = NULL;
}

MPTransAction::~MPTransAction()
{
}

bool MPTransAction::IsValidProp()
{
	if( m_pProp ) return true;
	else {
		if( !m_bIsValidProp ) return false;
		m_pProp = dynamic_cast<CDnWorldProp*>(this);
		if( !m_pProp ) m_bIsValidProp = false;
	}
	return true;
}

void MPTransAction::Send( DWORD dwProtocol, DnActorHandle hActor, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType )
{
	if( !m_pRoom ) return;
	if( !hActor ) return;
	if( !IsValidProp() ) return;
	DWORD dwUniqueID = m_pProp->GetUniqueID();

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) {
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		if( pStruct->GetSessionID() == hActor->GetUniqueID() ) {
			SendGamePropMsg( pStruct, dwUniqueID, (USHORT)dwProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell() );
			break;
		}
	}
}

void MPTransAction::Send( DWORD dwProtocol, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType, bool bImmediate )
{
	if( !m_pRoom ) return;
	if( !IsValidProp() ) return;
	DWORD dwUniqueID = m_pProp->GetUniqueID();

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) {
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		SendGamePropMsg( pStruct, dwUniqueID, (USHORT)dwProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell() );
	}
}

void MPTransAction::Send( DWORD dwProtocol, CMemoryStream *pStream, CDNUserSession* pSession, GlobalEnum::ClientSessionTypeEnum SessionType, bool bImmediate )
{
	DN_ASSERT( pSession != NULL, "MPTransAction::Send() pSession != NULL" );

	if( !m_pRoom ) return;
	if( !IsValidProp() ) return;
	DWORD dwUniqueID = m_pProp->GetUniqueID();

	SendGamePropMsg( pSession, dwUniqueID, (USHORT)dwProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell() );
}
