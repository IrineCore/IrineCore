#include "stdafx.h"
#include "MATransAction.h"
#include "DnActor.h"

#include "GameSendPacket.h"

#include "TaskManager.h"
#include "DnGameTask.h"
#include "DNGameRoom.h"

MATransAction::MATransAction()
{
	m_pActor = NULL;
	m_bIsValidActor = true;
}

MATransAction::~MATransAction()
{
}

bool MATransAction::IsValidActor()
{
	if( m_pActor ) return true;
	else {
		if( !m_bIsValidActor ) return false;
		m_pActor = dynamic_cast<CDnActor*>(this);
		if( !m_pActor ) m_bIsValidActor = false;
	}
	return true;
}

int	MATransAction::_GetPrior( DWORD dwActorProtocol )
{
	int nPrior = _FAST;
	switch( dwActorProtocol ) 
	{
		case eActor::SC_ONDAMAGE:
		case eActor::SC_PROJECTILE:
		case eActor::SC_MONSTER_PROJECTILE_FROM_PROJECTILE:
		case eActor::SC_CMDADDSTATEEFFECT:
		case eActor::SC_CMDREMOVESTATEEFFECT:
		case eActor::SC_CMDREMOVESTATEEFFECTFROMID:
		case eActor::SC_CMDWARP:
		case eActor::SC_PARTSDAMAGE:
		case eActor::SC_SETHPMP_DELTA:
		case eActor::SC_REFRESHHPSP:
		case eActor::SC_USESKILL:
		case eActor::SC_CMDLOOKTARGET:
		case eActor::SC_PLAYERDIE:
		case eActor::SC_CP:
		case eActor::SC_LEVELUP:
		case eActor::SC_RECOVERYSP:
		case eActor::SC_CMDTOGGLEBATTLE:
		case eActor::SC_CMDSUICIDE:
		case eActor::SC_CMDSHOW:
		case eActor::SC_CMDCHANGETEAM:
		case eActor::SC_FINISH_PROCESSSKILL:
		case eActor::SC_SLAVE_OF:
		case eActor::SC_SKILLUSING_FAILED:
		case eActor::SC_STATEEFFECT_PROB_SUCCESS:
		case eActor::SC_COOLTIME_RESET:
		case eActor::SC_CANNONPOSSESS_RES:
		case eActor::SC_CANNONRELEASE:
		case eActor::SC_SLAVE_RELEASE:
		case eActor::SC_COOLTIMEPARRY_SUCCESS:
		case eActor::SC_FREEZINGPRISON_DURABILITY:
		case eActor::SC_FISHINGROD_CAST:
		case eActor::SC_FISHINGROD_LIFT:
		case eActor::SC_FISHINGROD_HIDE:
		case eActor::SC_CMDSHOOTMODE:
		case eActor::SC_MONSTERPARTS_STATE:
		case eActor::SC_GHOST_TYPE:
		case eActor::SC_UDP_PING:
		{
			nPrior = _RELIABLE;
			break;
		}
	}

	return nPrior;
}

void MATransAction::Send( DWORD dwActorProtocol, DnActorHandle hActor, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType )
{
	if( !m_pRoom ) return;
	if( !hActor ) return;
	if( !IsValidActor() ) return;
	DWORD dwUniqueID = m_pActor->GetUniqueID();

	int nPrior = _GetPrior( dwActorProtocol );

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) 
	{
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		if( pStruct->GetSessionID() == hActor->GetUniqueID() ) 
		{
			pStruct->AddPacketQueue( dwUniqueID, (BYTE)dwActorProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell(), nPrior );
			break;
		}
	}
}

void MATransAction::Send( DWORD dwActorProtocol, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType, bool bImmediate )
{
	if( !m_pRoom ) return;
	if( !IsValidActor() ) return;
	DWORD dwUniqueID = m_pActor->GetUniqueID();

	int nPrior = _GetPrior( dwActorProtocol );

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) 
	{
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		pStruct->AddPacketQueue( dwUniqueID, (BYTE)dwActorProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell(), nPrior );
	}
}

void MATransAction::Send( DWORD dwActorProtocol, UINT nExceptSessionID, CMemoryStream *pStream, GlobalEnum::ClientSessionTypeEnum SessionType )
{
	if( !m_pRoom ) return;
	if( !IsValidActor() ) return;
	DWORD dwUniqueID = m_pActor->GetUniqueID();

	int nPrior = _GetPrior( dwActorProtocol );

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) 
	{
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		if (pStruct->GetSessionID() != nExceptSessionID)
			pStruct->AddPacketQueue( dwUniqueID, (BYTE)dwActorProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell(), nPrior );
	}
}

void MATransAction::Send( DWORD dwActorProtocol, DWORD dwUniqueID, CMemoryStream *pStream )
{
	if( !m_pRoom ) return;
	if( !IsValidActor() ) return;

	int nPrior = _GetPrior( dwActorProtocol );

	for( DWORD i=0; i<m_pRoom->GetUserCount(); i++ ) 
	{
		CDNUserSession *pStruct = m_pRoom->GetUserData(i);
		pStruct->AddPacketQueue( dwUniqueID, (BYTE)dwActorProtocol, (BYTE*)pStream->GetBuffer(), pStream->Tell(), nPrior );
	}
}

void MATransAction::Send( CDNUserSession* pGameSession, DWORD dwActorProtocol, DWORD dwUniqueID, CMemoryStream *pStream )
{
	if( !m_pRoom ) return;
	if( !IsValidActor() ) return;

	int nPrior = _GetPrior( dwActorProtocol );

	pGameSession->AddPacketQueue( dwUniqueID, (BYTE)dwActorProtocol, pStream ? (BYTE*)pStream->GetBuffer() : NULL, pStream ? pStream->Tell() : 0, nPrior );
}
