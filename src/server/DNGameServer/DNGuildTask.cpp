#include "stdafx.h"
#include "DNGuildTask.h"
#include "GameSendPacket.h"
#include "DnPlayerActor.h"
#include "DNUserSession.h"
#include "DNLogConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNLogConnection.h"
#include "DNUserSession.h"
#include "DNGameDataManager.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DNDBConnectionManager.h"
#include "DNGuildSystem.h"
#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"

CDnGuildTask::CDnGuildTask(CDNGameRoom * pRoom) : CTask(pRoom), CMultiSingleton<CDnGuildTask, MAX_SESSION_COUNT>( pRoom ) /*, m_pEmblemFactory( new CDnEmblemFactory )*/
{
	
}

CDnGuildTask::~CDnGuildTask()
{
	
}

bool CDnGuildTask::Initialize()
{
	
	return true;
}

void CDnGuildTask::Process(LOCAL_TIME LocalTime, float fDelta)
{
	
}

int CDnGuildTask::OnDispatchMessage(CDNUserSession * pSession, int nMainCmd, int nSubCmd, char *pData, int nLen)
{
	switch( nMainCmd ) {
		case CS_GUILD: return OnRecvGuildMessage( pSession, nSubCmd, pData, nLen );
	}

	return ERROR_NONE;
}

int CDnGuildTask::OnRecvGuildMessage(CDNUserSession * pSession, int nSubCmd, char *pData, int nLen)
{
	switch( nSubCmd ) 
	{
	case eGuild::CS_GETGUILDINFO: 
		{
			if (sizeof(CSGetGuildInfo) != nLen)
				return ERROR_INVALIDPACKET;

			g_pGuildManager->OnRecvCsGetGuildInfo(pSession, reinterpret_cast<CSGetGuildInfo*>(pData));
		}
		return ERROR_NONE;
	case eGuild::CS_PLAYER_REQUEST_GUILDINFO:
		{
			const CSPlayerGuildInfo *pPacket = reinterpret_cast<CSPlayerGuildInfo*>(pData);

			if (sizeof(CSPlayerGuildInfo) != nLen)
				return ERROR_INVALIDPACKET;			

			CDNUserSession *pUserObj = pSession->GetGameRoom()->GetUserSession(pPacket->nSessionID);

			if( !pUserObj )
			{
				pSession->SendPlayerGuildInfo( ERROR_GENERIC_INVALIDREQUEST, 0, NULL, NULL, 0, 0, 0, NULL );
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			const TGuildUID GuildUID = pUserObj->GetGuildUID();
			if (!GuildUID.IsSet()) 
			{
				pSession->SendPlayerGuildInfo( ERROR_NONE, pUserObj->GetSessionID(), NULL, NULL, 0, 0, 0, NULL );
				return ERROR_NONE;
			}

			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

			if (!pGuild) 
			{
				pSession->SendPlayerGuildInfo( ERROR_NONE, pUserObj->GetSessionID(), NULL, NULL, 0, 0, 0, NULL );
				return ERROR_NONE;
			}

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (FALSE == pGuild->IsEnable())
			{
				pSession->SendPlayerGuildInfo( ERROR_NONE, pUserObj->GetSessionID(), NULL, NULL, 0, 0, 0, NULL );
				return ERROR_NONE;
			}
#endif

			TGuildMember *GuildMaster = pGuild->GetGuildMaster();
			if( GuildMaster )
				pSession->SendPlayerGuildInfo( ERROR_NONE, pUserObj->GetSessionID(), pGuild->GetInfo()->GuildView.wszGuildName, GuildMaster->wszCharacterName, pGuild->GetLevel(), pGuild->GetMemberCount(), pGuild->GetGuildMemberSize(), pGuild->GetGuildRewardItem() );
			else
				pSession->SendPlayerGuildInfo( ERROR_NONE, pUserObj->GetSessionID(), pGuild->GetInfo()->GuildView.wszGuildName, NULL, pGuild->GetLevel(), pGuild->GetMemberCount(), pGuild->GetGuildMemberSize(), pGuild->GetGuildRewardItem() );

			return ERROR_NONE;
		}
		break;
	default:
		return ERROR_UNKNOWN_HEADER;	// If it did not return at caller level, a truly unhandled packet was received
	}
}
