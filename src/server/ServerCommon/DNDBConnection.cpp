#include "stdafx.h"
#include "DNDBConnection.h"
#include "DNIocpManager.h"
#include "DNGameDataManager.h"
#include "Log.h"
#include "DNEvent.h"
#include "DNDBConnectionManager.h"
#include "DNUserSession.h"
#include "DNGuildSystem.h"
#include "DNUserItem.h"
#if defined(_VILLAGESERVER)
#include "DNUserSessionManager.h"
#include "MasterSystemCacheRepository.h"
#include "DNMasterConnection.h"
extern TVillageConfig g_Config;
#include "DNFarm.h"
#include "DNGuildWarManager.h"
#include "DNGuildVillage.h"
#if defined( PRE_PARTY_DB )
#include "DNPartyManager.h"
#endif // #if defined( PRE_PARTY_DB )

#elif defined(_GAMESERVER) // #if defined(_VILLAGESERVER)
#include "DNGameServerManager.h"
#include "DNRUDPGameServer.h"
#include "DNGameRoom.h"
#include "DnGameTask.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "DNMasterConnectionManager.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "GrowingArea.h"
#include "DNFarmGameRoom.h"

extern TGameConfig g_Config;
#if defined(PRE_ADD_DOORS_PROJECT)
#include "DnPlayerActor.h"
#endif //#if defined(PRE_ADD_DOORS_PROJECT)
#endif	// #if defined(_GAMESERVER)

#include "DNLogConnection.h"
#include "VarArg.h"
#include "DNAuthManager.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "DNMailSender.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "DNPeriodQuestSystem.h"
#include "Version.h"
#include "DNMissionSystem.h"
#include "DNGuildRecruitCacheRepository.h"
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
#if defined( _VILLAGESERVER )
#include "DNPvPRoomManager.h"
#endif
#endif

// Did this because every packet declaration was declaring and initializing repeatedly. Not particularly efficient, so I will delete it if it hurts readability at all. hgoori
#define ALLOC_PACKET(T) \
	T packet; \
	memset(&packet, 0, sizeof(packet))

CDNDBConnection::CDNDBConnection(void): CConnection()
{
#if defined(_FINAL_BUILD)
	Init(1024 * 1024 * 10, 1024 * 1024 * 10);
#else
	Init(1024 * 100, 1024 * 100);
#endif


	m_nServerID = -1;
}

CDNDBConnection::~CDNDBConnection(void)
{
}

int CDNDBConnection::AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen, BYTE cSeq)
{
	if(!GetActive()) return 0;

	DN_ASSERT(NULL != g_pDBConnectionManager,	"Invalid!");
	DN_ASSERT(CHECK_LIMIT(iMainCmd, 255),		"Invalid!");
	DN_ASSERT(CHECK_LIMIT(iSubCmd, 255),		"Invalid!");
	DN_ASSERT(NULL != pData,					"Invalid!");
	DN_ASSERT(0 < iLen,							"Invalid!");

	DWORD nThreadID = ::GetCurrentThreadId();
	DN_ASSERT(0 != nThreadID,	"Check!");

	return(CConnection::AddSendData(iMainCmd, iSubCmd, pData, iLen, NULL, cSeq));
}


void CDNDBConnection::GetDBSID()
{
	if( GetActive() == false )
		return;

	if( m_nServerID != -1 )
		return;

	TQGetDBSID packet;
	memset(&packet, 0, sizeof(packet));
	AddSendData(MAINCMD_ETC, QUERY_GET_DBSID, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::Reconnect()
{
	if(!GetActive() && !GetConnecting() && g_pIocpManager)
	{
		SetConnecting(true);
		if(g_pIocpManager->AddConnectionEx(this, CONNECTIONKEY_DB, m_szIP, m_wPort) < 0)
		{
			SetConnecting(false);
#if defined( STRESS_TEST )
#else
			g_Log.Log(LogType::_ERROR, L"DBServer Fail(%S, %d)\r\n", m_szIP, m_wPort);
#endif // #if defined( STRESS_TEST )
		}
		else {
#if defined( STRESS_TEST )
#else
			g_Log.Log(LogType::_NORMAL, L"DBServer(%S, %d) Connecting..\r\n", m_szIP, m_wPort);			
#endif // #if defined( STRESS_TEST )
		}
	}
}

#if defined( _GAMESERVER )

CDNRUDPGameServer* CDNDBConnection::_GetGameServer( int iMainCmd, int iSubCmd, char* pData )
{
	if( !g_pGameServerManager )
		return NULL;

	switch( iMainCmd )
	{
	case MAINCMD_DARKLAIR:
		{
			switch(iSubCmd )
			{
				case QUERY_UPDATE_DARKLAIR_RESULT:
				{
					TAUpdateDarkLairResult* pPacket = reinterpret_cast<TAUpdateDarkLairResult*>(pData);
					return g_pGameServerManager->GetGameServerByRoomID( pPacket->iRoomID );
				}
			}
		}
		break;

	case MAINCMD_FARM:
		{
			switch(iSubCmd)
			{
				case QUERY_GETLIST_FIELD:
				{
					TAGetListField* pPacket = reinterpret_cast<TAGetListField*>(pData);
					return g_pGameServerManager->GetGameServerByRoomID( pPacket->iRoomID );
				}
				// The top-level variable of TA~ packets below MUST be RoomID!!!!!!
				case QUERY_ADD_FIELD:
				case QUERY_DEL_FIELD:
				case QUERY_HARVEST:
				case QUERY_ADD_FIELD_ATTACHMENT:
				case QUERY_GET_FIELDITEMCOUNT:
				case QUERY_GETLIST_FIELD_FORCHARACTER:
				case QUERY_ADD_FIELD_FORCHARACTER:
				case QUERY_DEL_FIELD_FORCHARACTER:
				case QUERY_ADD_FIELD_FORCHARACTER_ATTACHMENT:
				case QUERY_HARVEST_FORCHARACTER:
				{
					int iRoomID;
					memcpy( &iRoomID, pData+sizeof(TAHeader), sizeof(int) );
					return g_pGameServerManager->GetGameServerByRoomID( iRoomID );
				}
			}
		}
		break;

#if defined( PRE_PARTY_DB )
	case MAINCMD_PARTY:
		{
			switch(iSubCmd)
			{
				case QUERY_ADDPARTYANDMEMBERGAME:
				{
					TAAddPartyAndMemberGame* pPacket = reinterpret_cast<TAAddPartyAndMemberGame*>(pData);
					pPacket->nAccountDBID = 0;
					return g_pGameServerManager->GetGameServerByRoomID( pPacket->Data.PartyData.iRoomID );
				}
				case QUERY_MODPARTY:
				{
					TAModParty* pPacket = reinterpret_cast<TAModParty*>(pData);
					return g_pGameServerManager->GetGameServerByRoomID( pPacket->PartyData.iRoomID );
				}				
#if defined( PRE_WORLDCOMBINE_PARTY )
				case QUERY_ADDWORLDPARTY:
				{
					TAAddParty* pPacket = reinterpret_cast<TAAddParty*>(pData);
					return g_pGameServerManager->GetGameServerByRoomID( pPacket->Data.PartyData.iRoomID );
				}
#endif
			}			
		}
		break;
#endif
	}

	UINT uiAccountDBID;
	memcpy( &uiAccountDBID, pData, sizeof(UINT) );

	return g_pGameServerManager->GetGameServerByAID( uiAccountDBID );
}

#endif // #if defined( _GAMESERVER )

int CDNDBConnection::MessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen)
{
	UINT nAccountDBID;
	memcpy(&nAccountDBID, pData, sizeof(UINT));

#if defined(_VILLAGESERVER)

	if(PreMessageProcess(nMainCmd, nSubCmd, pData, nLen) == true)
		return ERROR_NONE;

	CDNUserSession *pUserObj = NULL;
	if( nMainCmd == MAINCMD_AUTH && nSubCmd == QUERY_CHECKAUTH)
	{
		TACheckAuth *pA = (TACheckAuth*)pData;
		pUserObj = g_pUserSessionManager->FindTempUserSession(pA->nSessionID);

		if( pA->nRetCode == ERROR_NONE )
		{
			if (pUserObj == NULL)
			{
				// These won't get ResetAuth since they disconnected before receiving CheckAuth.. force it
				g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, nAccountDBID, 0, 0, L"[ADBID:%u] Duplicate ResetAuth \r\n", nAccountDBID);
				g_pAuthManager->QueryResetAuth(pA->cWorldSetID, pA->nAccountDBID, pA->nSessionID);
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			else
			{
				pUserObj->SetAccountDBID(nAccountDBID);
				if (!g_pUserSessionManager->InsertSession(pUserObj))
				{
#ifdef PRE_FIX_VILLAGEZOMBIE
					volatile bool bInserted = false;
					DWORD dwCurTime = timeGetTime();
					CDNUserSession * pDuplicateSession;
					pDuplicateSession = g_pUserSessionManager->FindUserSessionByAccountDBID(nAccountDBID);
					if (pDuplicateSession == NULL)
						pDuplicateSession = g_pUserSessionManager->FindUserSession(pUserObj->GetSessionID());
					if (pDuplicateSession && pDuplicateSession->GetInsideDisconnectTick() + (60*1000*30) < dwCurTime)
					{
						pDuplicateSession->SetZombieChecked();
						g_pUserSessionManager->DelSession(pDuplicateSession, pDuplicateSession->GetSocketContext());
						if (g_pUserSessionManager->InsertSession(pUserObj))
							bInserted = true;
					}					

					if (bInserted == false)
					{
						pUserObj->DetachConnection(L"Connect|Duplicate SessionID");
						return ERROR_GENERIC_UNKNOWNERROR;
					}
#else		//#ifdef PRE_FIX_VILLAGEZOMBIE
					pUserObj->DetachConnection(L"Connect|Duplicate SessionID");
					return ERROR_GENERIC_UNKNOWNERROR;
#endif		//PRE_FIX_VILLAGEZOMBIE
				}
			}
		}
	}
	else
	{
		pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(nAccountDBID);
	}

	if (pUserObj == NULL)
	{
		if (nMainCmd != MAINCMD_STATUS && nSubCmd != QUERY_LASTUPDATEUSERDATA)
		g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, nAccountDBID, 0, 0, L"[ADBID:%u] CDNDBConnection::MessageProcess UserCon not found Main:%d, Sub:%d\r\n", nAccountDBID, nMainCmd, nSubCmd);
		return ERROR_GENERIC_INVALIDREQUEST;
	}
	pUserObj->DBMessageProcess(nMainCmd, nSubCmd, pData, nLen);
#elif defined(_GAMESERVER)
	if(PreMessageProcess(nMainCmd, nSubCmd, pData, nLen) == false)
	{
		CDNRUDPGameServer* pServer = _GetGameServer( nMainCmd, nSubCmd, pData );
		if(pServer)	//Parse DB message
		{
			pServer->StoreExternalBuffer(nAccountDBID, nMainCmd, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE);
			return ERROR_NONE;
		}		
		// Check here if it is related to CheckAuth.
		else if( nMainCmd == MAINCMD_AUTH && nSubCmd == QUERY_CHECKAUTH)
		{
			TACheckAuth *pA = (TACheckAuth*)pData;
			if( pA->nRetCode == ERROR_NONE )
			{
				// These won't get ResetAuth since they disconnected before receiving CheckAuth.. force it
				g_Log.Log(LogType::_ERROR, pA->cWorldSetID, pA->nAccountDBID, 0, 0, L"[ADBID:%u] Duplicate ResetAuth \r\n", pA->nAccountDBID);
				g_pAuthManager->QueryResetAuth(pA->cWorldSetID, pA->nAccountDBID, pA->nSessionID );
			}
		}		
	}

	return ERROR_GENERIC_UNKNOWNERROR;
#endif
	return ERROR_NONE;
}

bool CDNDBConnection::PreMessageProcess(int nMainCmd, int nSubCmd, char * pData, int nLen)
{
	switch(nMainCmd)
	{
	case MAINCMD_AUTH: return OnRecvAuth(nSubCmd, pData); break;
	case MAINCMD_ETC: return OnRecvEtc(nSubCmd, pData); break;
	case MAINCMD_QUEST: return OnRecvQuest(nSubCmd, pData); break;
	case MAINCMD_GUILD: return OnRecvGuild(nSubCmd, pData); break;
	case MAINCMD_MASTERSYSTEM: return OnRecvMasterSystem( nSubCmd, pData ); break;
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case MAINCMD_MAIL:	return OnRecvMail(nSubCmd,pData); break;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case MAINCMD_FARM: return OnDBRecvFarm(nSubCmd, pData, nLen); break;
	case MAINCMD_JOBSYSTEM: return OnDBRecvDBJobSystem(nSubCmd, pData); break;
	case MAINCMD_GUILDRECRUIT: return OnRecvGuildRecruit(nSubCmd, pData); break;
#if defined (PRE_ADD_DONATION)
		case MAINCMD_DONATION: return OnRecvDonation(nSubCmd, pData, nLen);
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
		case MAINCMD_PARTY: return OnRecvParty( nSubCmd, pData, nLen );
#endif // #if defined( PRE_PARTY_DB ) && defined( _VILLAGESERVER )
#ifdef PRE_MOD_PVPRANK
		case MAINCMD_PVP: return OnRecvPvP(nSubCmd, pData, nLen);
#endif		//#ifdef PRE_MOD_PVPRANK
#if defined( PRE_WORLDCOMBINE_PVP ) && !defined(PRE_MOD_PVPRANK)
		case MAINCMD_PVP: return OnRecvPvP(nSubCmd, pData, nLen);
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
		case MAINCMD_PRIVATECHATCHANNEL: return OnRecvPrivateChannel( nSubCmd, pData, nLen );
#endif
	default: return false;
	}

	return false;
}

bool CDNDBConnection::OnRecvAuth(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_RESETAUTHSERVER:
		{
			TAResetAuthServer *pA = (TAResetAuthServer*)pData;

			if(!g_pAuthManager->IsResetAuthServer()) {
				if(ERROR_NONE != pA->nResult) {
					_DANGER_POINT();
					break;
					//					DN_BREAK;
				}

				g_pAuthManager->SetResetAuthServer(true);

				// Start server
				g_pIocpManager->VerifyAccept(ACCEPTOPEN_VERIFY_TYPE_RESETAUTH);
			}
		}
		break;

	default:
		return false;
	}

	return true;
}

bool CDNDBConnection::OnRecvEtc(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
		case QUERY_EVENTLIST:
		{
			TAEventList * pPacket = (TAEventList*)pData;

			if(pPacket->nRetCode == ERROR_NONE)
			{
				if(pPacket->cCount > 0)
					g_pEvent->AddWorldEvent(pPacket->cWorldSetID, pPacket->cCount, pPacket->EventList);
			}
			else
				_DANGER_POINT();
		}
		break;

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
		case QUERY_GET_SIMPLECONFIG:
		{
#ifdef _VILLAGESERVER
			TASimpleConfig * pPacket = (TASimpleConfig*)pData;

			if (pPacket->nRetCode != ERROR_NONE)
			{
				_DANGER_POINT();
				break;
			}
#endif		//#ifdef _VILLAGESERVER
		}
		break;
#endif
		case QUERY_GET_DBSID:
		{
			TAGetDBSID * pPacket = (TAGetDBSID*)pData;
			m_nServerID = pPacket->nServerID;

			g_Log.Log(LogType::_DBCONNECTIONLOG, L"QUERY_GET_DBSID [SessionID:%d][ServerID:%d]\r\n", m_nSessionID, m_nServerID);
		}
		break;
#if defined( PRE_FIX_67546 )
		case QUERY_ADD_CHANNELCOUNT:
			{
#if defined( _VILLAGESERVER )
				TAAddThreadCount * pPacket = (TAAddThreadCount*)pData;
				g_pDBConnectionManager->AddVillageDBConnectionInfo( GetSessionID(), pPacket->nConnectionCount);				
#endif
			}
			break;
#endif
		default:
			return false;
	}

	return true;
}

bool CDNDBConnection::OnRecvQuest(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_GET_WORLDEVENTQUESTCOUNTER:
		{
			TAGetWorldEventQuestCounter * pPacket = (TAGetWorldEventQuestCounter*)pData;

			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_GET_WORLDEVENTQUESTCOUNTER FAIL\r\n");
				break;
			}

			// Need to check completion status
			g_pPeriodQuestSystem->UpdateWorldEvent(pPacket->cWorldSetID, pPacket->nScheduleID, pPacket->nCount, true);
		}
		break;

	case QUERY_MOD_WORLDEVENTQUESTCOUNTER:
		{
			TAModWorldEventQuestCounter * pPacket = (TAModWorldEventQuestCounter*)pData;

			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_MOD_WORLDEVENTQUESTCOUNTER FAIL\r\n");
				break;
			}

#if defined(_VILLAGESERVER)
			// Broadcast to all servers via master
			g_pMasterConnection->SendWorldEventCounter(pPacket->cWorldSetID, pPacket->nScheduleID, pPacket->nCountAfter);
#endif // #if defined(_VILLAGESERVER)
		}
		break;
	default:
		return false;
	}

	return true;	
}

bool CDNDBConnection::OnRecvGuild(int nSubCmd, char *pData)
{
	switch(nSubCmd)
	{
	case QUERY_GETGUILDINFO:			// Guild info result
		{
			TAGetGuildInfo *pPacket = reinterpret_cast<TAGetGuildInfo*>(pData);

			// Inspect packet
			if(ERROR_NONE != pPacket->nRetCode || GUILDDBID_DEF == pPacket->nGuildDBID)
				break;

			g_pGuildManager->UpdateGuildResource(pPacket);

			// P.S.> Guild time info sync is handled independently per session, so unnecessary here ?
#if defined(_VILLAGESERVER)
			// Get guild score if in Guild War preliminaries or reward period
			if( g_pGuildWarManager && (g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_TRIAL || g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_REWARD)	)			
				QueryGetGuildWarPoint(rand()%THREADMAX, g_Config.nWorldSetID, 0, 'G', pPacket->nGuildDBID);
			
			// Guild reward completion history
			if( g_pGuildWarManager && g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_REWARD )
				QueryGetGuildWarRewarForGuild(rand()%THREADMAX, g_Config.nWorldSetID, g_pGuildWarManager->GetScheduleID(), pPacket->nGuildDBID);
#endif
		}
		break;	
	case QUERY_GET_GUILDMEMBER:
		{
			TAGetGuildMember *pPacket = reinterpret_cast<TAGetGuildMember*>(pData);
			g_pGuildManager->UpdateGuildMember(pPacket);
		}
		break;
	case QUERY_GET_GUILDREWARDITEM:
		{			
			//When received as a whole
			TAGetGuildRewardItem *pGuildRewardItem = (TAGetGuildRewardItem*)pData;
			g_pGuildManager->UpdateGuildRewardItem(pGuildRewardItem);
		}
		break;	
	case QUERY_CHANGEGUILDINFO:			// Guild info change result
		{
			return false;
		}
		break;

	case QUERY_GET_GUILDWAREINFO:		// Guild storage slot result
		{
			TAGetGuildWareInfo *pPacket = reinterpret_cast<TAGetGuildWareInfo*>(pData);

			// Inspect packet
			if(ERROR_NONE != pPacket->nErrNo || GUILDDBID_DEF == pPacket->nGuildDBID)
				break;

			g_pGuildManager->OpenGuildWare(pPacket);
		}
		break;
	case QUERY_ADD_GUILDWARPOINT :			// Earn points when clearing preliminary mission
		{
			const TAAddGuildWarPointRecodrd * pPacket = reinterpret_cast<TAAddGuildWarPointRecodrd*>(pData);

			// Inspect packet
			if(ERROR_NONE != pPacket->nRetCode)
				break;
			const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);
#if defined(_VILLAGESERVER)
			CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
			if( pUserObj )
			{
				pUserObj->AddGuildWarPoint(pPacket->nOpeningPoint);
			}
			if(g_pMasterConnection && g_pMasterConnection->GetActive() )
				g_pMasterConnection->SendAddGuildWarPoint(pPacket->cTeamColorCode, GuildUID, pPacket->nOpeningPoint);			
#endif//#if defined(_VILLAGESERVER)
#if defined(_GAMESERVER)
			if( g_pMasterConnectionManager )
			{
				//const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);

				g_pMasterConnectionManager->SendAddGuildWarPoint(pPacket->cWorldSetID, pPacket->cTeamColorCode, GuildUID, pPacket->nOpeningPoint );
			}
#endif //#if defined(_GAMESERVER)
		}
		break;
	case QUERY_ADD_GUILDWARFINALRESULTS :				// Save Guild War finals result
		{

		}
		break;
	case QUERY_ADD_GUILDWAR_REWARD_GUILD :		// Save Guild War guild reward result
		{
			TAAddGuildWarRewardGuild* pPacket = reinterpret_cast<TAAddGuildWarRewardGuild*>(pData);
			// Honestly, only 4-guild prelim reward points matter here; the rest are unused since there is no way to handle errors.
			const TGuildUID GuildUID(pPacket->cWorldSetID, static_cast<UINT>(pPacket->nGuildDBID));
			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
			if(pGuild)
			{
				// Set unconditionally.. if error occurred, reward was already given.
				pGuild->SetGuildWarRewardResultIndex(pPacket->cRewardType, true);
#if !defined( PRE_ADD_NODELETEGUILD )
				CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
				if(FALSE == pGuild->IsEnable()) break;
#endif
				if( pPacket->nRetCode == ERROR_NONE)
				{
#if defined(_VILLAGESERVER)
					switch(pPacket->cRewardType)
					{
					case GUILDWAR_REWARD_GUILD_TRIAL_POINT :
						{
							//Add guild points for preliminaries
							CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
							pGuild->AddPoint(GUILDPOINTTYPE_WAR, g_pGuildWarManager->GetGuildWarPoint(GuildUID.nDBID));
						}
						break;
					}
#endif					
				}
			}
		}
		break;
	case QUERY_GET_GUILDWAR_REWARD_GUILD :		// Get Guild War guild reward result
		{
			TAGetGuildWarRewardGuild* pPacket = reinterpret_cast<TAGetGuildWarRewardGuild*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)
			{
#if defined(_VILLAGESERVER)
				if( pPacket->cQueryType == 'M')
				{
					if( pPacket->bRewardResults[GUILDWAR_REWARD_GUILD_SKILL] ) // If the skill has been granted...
					{
						// Notify to grant gesture
						g_pMasterConnection->SendSetGuildWarPreWinReward();
						// Update guild points for all guilds
						QueryGetGuildWarPointFinalRewards(rand()%THREADMAX, g_Config.nWorldSetID);
					}
				}
				else
#endif
				{
					const TGuildUID GuildUID(pPacket->cWorldSetID, static_cast<UINT>(pPacket->nGuildDBID));

					CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
					if(pGuild)
					{
#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif
						pGuild->SetGuildWarRewardResults(pPacket->bRewardResults);					
					}
				}
				
			}
			else
				g_Log.Log(LogType::_GUILDWAR, L"[GUILDWAR] QUERY_GET_GUILDWAR_REWARD_GUILD Ret:%d !!\r\n", pPacket->nRetCode);
		}
		break;
#if defined(_VILLAGESERVER)
	case QUERY_GET_GUILDWARSCHEDULE:		// Guild War schedule
		{
			TAGetGuildWarSchedule* pPacket = reinterpret_cast<TAGetGuildWarSchedule*>(pData);			

			if(g_pMasterConnection && g_pMasterConnection->GetActive() )		
				g_pMasterConnection->SendSetGuildWarSchedule(pPacket->nRetCode, pPacket->wScheduleID, pPacket->wWinersWeightRate, pPacket->EventInfo, false, pPacket->bFinalProgress, pPacket->tRewardExpireDate);			
		}
		break;
	case QUERY_GET_GUILDWARFINALSCHEDULE :	// Fetch finals schedule
		{
			TAGetGuildWarFinalSchedule* pPacket = reinterpret_cast<TAGetGuildWarFinalSchedule*>(pData);			
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					g_pMasterConnection->SendSetGuildWarFinalSchedule(pPacket->GuildWarFinalSchedule);
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARFINALSCHEDULE Ret:%d !!\r\n", pPacket->nRetCode);
		}
		break;	
	case QUERY_GET_GUILDWARPOINT :			// Query preliminary points (character, guild, team)
		{
			TAGetGuildWarPoint* pPacket = reinterpret_cast<TAGetGuildWarPoint*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				switch(pPacket->cQueryType)
				{
				case 'T' : // Team.. send this to the master server.
					{
						if( g_pMasterConnection && g_pMasterConnection->GetActive() )
							g_pMasterConnection->SendSetGuildWarPoint(pPacket->nRetCode, pPacket->nBlueTeamPoint, pPacket->nRedTeamPoint);
					}
					break;
				case 'C' : // Character.. personal settings
					{
						CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
						if( pUserObj )
						{
							pUserObj->SetGuildWarPoint(pPacket->nCharOpeningPoint);
						}
					}
					break;
				case 'G' : // Guild settings
					{
						const TGuildUID GuildUID(g_Config.nWorldSetID, static_cast<UINT>(pPacket->biDBID));
						
						CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
						if(pGuild)
						{
#if !defined( PRE_ADD_NODELETEGUILD )
							CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
							if(FALSE == pGuild->IsEnable()) break;
#endif
							CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
							pGuildVillage->SetGuildWarPoint(pPacket->nGuildOpeningPoint);							
						}
					}
					break;
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT Ret:%d Type:%d!!\r\n", pPacket->nRetCode, pPacket->cQueryType);
		}
		break;
	case QUERY_GET_GUILDWARPOINT_PARTTOTAL :			// Query 1st place by category (individual)
		{
			TAGetGuildWarPointPartTotal* pPacket = reinterpret_cast<TAGetGuildWarPointPartTotal*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if( pPacket->cQueryType == 'A') // Overall ranking
				{
					g_pGuildWarManager->SetGuildWarMissionRankingInfo(pPacket);
					// Next query
					QueryGetGuildWarPointGuildPartTotal(rand()%THREADMAX, g_Config.nWorldSetID, 0, 'A', 0);
				}
				else if( pPacket->cQueryType == 'C') // Individual
				{
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
					if( pUserObj )
					{
						pUserObj->SetGuildWarMissionRankingInfo(pPacket);
						// If reaching here, it is a guild war preliminary result query.
						pUserObj->SendGuildWarTrialResultOpen();
					}
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT_PARTTOTAL Ret:%d Type:%d!!\r\n", pPacket->nRetCode, pPacket->cQueryType);
		}
		break;
	case QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL :		// Guild ranking by category
		{
			TAGetGuildWarPointGuildPartTotal* pPacket = reinterpret_cast<TAGetGuildWarPointGuildPartTotal*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if( pPacket->cQueryType == 'A')			
				{
					g_pGuildWarManager->SetGuildWarMissionGuildRankingInfo(pPacket);				
					// Next query
					QueryGetGuildWarPointDaily(rand()%THREADMAX, g_Config.nWorldSetID);
				}
				else if( pPacket->cQueryType == 'G')
				{
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
					if( pUserObj )
					{
						CDNGuildBase* pGuild = g_pGuildManager->At(pUserObj->GetGuildUID());
						if(pGuild)
						{	
#if !defined( PRE_ADD_NODELETEGUILD )
							CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
							if(FALSE == pGuild->IsEnable()) break;
#endif
							CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
							pGuildVillage->SetGuildWarTotalRankingInfo(pPacket);				

							// Next query
							QueryGetGuildWarPointGuildTotal(pGuild->GetDBThreadID(), g_Config.nWorldSetID, pPacket->nAccountDBID, false, 'G', g_pGuildWarManager->GetScheduleID(), pUserObj->GetGuildUID().nDBID);
						}
					}
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL Ret:%d Type:%d!!\r\n", pPacket->nRetCode, pPacket->cQueryType);
		}
		break;
	case QUERY_GET_GUILDWARPOINT_DAILY :				// Daily 1st place in preliminaries		
		{
			TAGetGuildWarPointDaily* pPacket = reinterpret_cast<TAGetGuildWarPointDaily*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)		
			{
				g_pGuildWarManager->SetGuildDailyAward(pPacket);
				// Next query
				QueryGetGuildWarPointGuildTotal(rand()%THREADMAX, g_Config.nWorldSetID, 0, false, 'A', g_pGuildWarManager->GetScheduleID(), 0);
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT_DAILY Ret:%d !!\r\n", pPacket->nRetCode);
			
		}
		break;
	case QUERY_GET_GUILDWARPOINT_GUILD_TOTAL :			// Aggregated point rankings of each guild (main tournament qualifiers can also be queried here)
		{
			TAGetGuildWarPointGuildTotal* pPacket = reinterpret_cast<TAGetGuildWarPointGuildTotal*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if( pPacket->cQueryType == 'A' ) // Overall ranking		
				{
					if( pPacket->bMasterServer )
					{
						if( g_pMasterConnection && g_pMasterConnection->GetActive() )						
							g_pMasterConnection->SendSetGuildWarFinalTeam(pPacket);
					}
					else
						g_pGuildWarManager->SetGuildWarRankingInfo(pPacket);
				}
				else if( pPacket->cQueryType == 'G') // Guild rank
				{
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
					if( pUserObj )
					{
						if( pPacket->nTotalCount == 1 ) // May not be in the guild rankings.
						{
							CDNGuildBase* pGuild = g_pGuildManager->At(pUserObj->GetGuildUID());
							if(pGuild)
							{	
#if !defined( PRE_ADD_NODELETEGUILD )
								CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
								if(FALSE == pGuild->IsEnable()) break;
#endif
								CDNGuildVillage* pGuildVillage = static_cast<CDNGuildVillage *>(pGuild);
								pGuildVillage->SetGuildWarMyRankingInfo(pPacket);						
							}
						}					
						// If reached here, now collect personal info.
						QueryGetGuildWarPointPartTotal(rand()%THREADMAX, g_Config.nWorldSetID, pPacket->nAccountDBID, 'C', pUserObj->GetCharacterDBID());
					}
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT_GUILD_TOTAL Ret:%d Type:%d!!\r\n", pPacket->nRetCode, pPacket->cQueryType);
		}
		break;
	case QUERY_ADD_GUILDWAR_FINAL_MATCHLIST :			// Save finals bracket (anything to do here?)
		{
			TAAddGuildWarFinalMatchList* pPacket = reinterpret_cast<TAAddGuildWarFinalMatchList*>(pData);
			if( g_pMasterConnection && g_pMasterConnection->GetActive() )
			{				
				g_pMasterConnection->SendMatchListSaveResult(pPacket->nRetCode);
			}
		}
		break;
	case QUERY_ADD_GUILDWARPOPULARITYVOTE :				// Save popularity vote
		{
			TAGetGuildWarPopularityVote* pPacket = reinterpret_cast<TAGetGuildWarPopularityVote*>(pData);
			CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
			if( pUserObj )
			{
				pUserObj->SendGuildWarVote(pPacket->nRetCode, 1000);
				if( pPacket->nRetCode == ERROR_NONE)
				{
					pUserObj->SetGuildWarVote(true);
					// Grant festival points here (fixed 1000 points).
					int nFestivalPoint = 1000;
					// Additional guild festival points
					const TGuildUID GuildUID = pUserObj->GetGuildUID();
					CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
					if(pGuild)
					{
#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
#endif
#if !defined( PRE_ADD_NODELETEGUILD )
						if ( pGuild->IsEnable() )
						{
#endif
							TGuildRewardItem* GuildRewardItem = pGuild->GetGuildRewardItem();
							if( GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nItemID > 0 &&
								GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue > 0 )
							{
								nFestivalPoint += (int)(nFestivalPoint * GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue * 0.01);
							}
#if !defined( PRE_ADD_NODELETEGUILD )
						}			
#endif
					}

					pUserObj->AddEtcPoint(DBDNWorldDef::EtcPointCode::GuildWar_Festival, nFestivalPoint);
				}
				else
					g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_ADD_GUILDWARPOPULARITYVOTE Ret:%d !!\r\n", pPacket->nRetCode);
			}
		}
		break;
	case QUERY_GET_GUILDWARPOPULARITYVOTE :				// Popularity vote result
		{
			TAGetGuildWarPopularityVote* pPacket = reinterpret_cast<TAGetGuildWarPopularityVote*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)
			{
				if( pPacket->cQueryType == 'C')
				{					
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
					if( pUserObj )
					{
						pUserObj->SetGuildWarVoteLoading(true);
						if( pPacket->nGuildDBID == 0 )// This means never voted.							
							pUserObj->SetGuildWarVote(false);							
						else
							pUserObj->SetGuildWarVote(true);
						pUserObj->SendGuildWarTournamentInfoOpen(!pUserObj->GetGuildWarVote());
					}				
				}
				else if( pPacket->cQueryType == 'G')
				{
					CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
					if( pUserObj )
					{					
						g_pGuildWarManager->SetGuildWarVoteCount(pPacket->nPopularityVoteCount);
						g_pGuildWarManager->SetGuildWarVoteGuildName(pPacket->wszGuildName);
						g_pGuildWarManager->SetGuildWarVote(true);

						pUserObj->SendGuildWarVoteTop(pPacket->nPopularityVoteCount, pPacket->wszGuildName);
					}
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOPULARITYVOTE Ret:%d Type:%d!!\r\n", pPacket->nRetCode, pPacket->cQueryType);
		}
		break;	
	case QUERY_GET_GUILDWARFINALRESULTS :				// Get Guild War finals result
		{
			TAGetGuildWarFinalResults* pPacket = reinterpret_cast<TAGetGuildWarFinalResults*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendSetGuildWarTournamentInfo(pPacket);
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARFINALRESULTS Ret:%d!!\r\n", pPacket->nRetCode);
		}
		break;
	case QUERY_GET_GUILDWAR_PRE_WIN_GUILD :				// Get previous Guild War season winning team  
		{
			TAGetGuildWarPreWinGuild* pPacket = reinterpret_cast<TAGetGuildWarPreWinGuild*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					TGuildUID GuildUID(g_Config.nWorldSetID, pPacket->nGuildDBID);
					g_pMasterConnection->SendSetGuildWarPreWinGuild(GuildUID, pPacket->wScheduleID);
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWAR_PRE_WIN_GUILD Ret:%d!!\r\n", pPacket->nRetCode);
		}
		break;
	case QUERY_ADD_GUILDWAR_REWARD_CHARACTER :	// Save Guild War preliminary reward result
		{
			TAAddGuildWarRewardCharacter* pPacket = reinterpret_cast<TAAddGuildWarRewardCharacter*>(pData);
			CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
			if( pUserObj )
			{
				// Set unconditionally.. if error occurred, reward was already given.
				pUserObj->SetGuildWarRewardResultIndex(pPacket->cRewardType, true);
				if( pPacket->nRetCode == ERROR_NONE)
				{			
					TGuildWarRewardData* pRewardData = NULL;
					//Give out rewards here by type.					
					switch(pPacket->cRewardType)
					{
					case GUILDWAR_REWARD_CHAR_PRESENT :
						{							
							bool bWin = g_pGuildWarManager->IsGuildWarTrialWIn(pUserObj->GetGuildSelfView().cTeamColorCode);
							if( bWin )							
								pRewardData = g_pDataManager->GetGuildWarRewardData(DBDNWorldDef::GuildWarRewardType::GuildWarFestivalWin, pUserObj->GetClassID());								
							else
								pRewardData = g_pDataManager->GetGuildWarRewardData(DBDNWorldDef::GuildWarRewardType::GuildWarFestivalLose, pUserObj->GetClassID());
							// Send crown to gift box.
							if( pRewardData )
								CDNMailSender::Process(pUserObj, pRewardData->nPresentID, DBDNWorldDef::PayMethodCode::GuildWar_Reward, 0);
						}
						break;
					case GUILDWAR_REWARD_CHAR_MAIL :
						{							
							// Send festival card via mail
							bool bWin = g_pGuildWarManager->IsGuildWarTrialWIn(pUserObj->GetGuildSelfView().cTeamColorCode);
							if( bWin )							
								pRewardData = g_pDataManager->GetGuildWarRewardData(DBDNWorldDef::GuildWarRewardType::GuildWarFestivalWin, pUserObj->GetClassID());								
							else
								pRewardData = g_pDataManager->GetGuildWarRewardData(DBDNWorldDef::GuildWarRewardType::GuildWarFestivalLose, pUserObj->GetClassID());							
							if( pRewardData )
								CDNMailSender::Process(pUserObj, pRewardData->nMailID, DBDNWorldDef::PayMethodCode::GuildWar_Reward, 0);
						}
						break;
					case GUILDWAR_REWARD_CHAR_POINT :
						{
							// Add festival points
							int nFestivalPoint = g_pGuildWarManager->CalcGuildWarFestivalPoint(pUserObj->GetGuildSelfView().cTeamColorCode, pUserObj->GetCharacterDBID(), pUserObj->GetGuildWarPoint());
							// Additional guild festival points
							const TGuildUID GuildUID = pUserObj->GetGuildUID();
							CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
							if(pGuild)
							{
#if !defined( PRE_ADD_NODELETEGUILD )
								CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
								if ( pGuild->IsEnable() )
								{
#endif
									TGuildRewardItem* GuildRewardItem = pGuild->GetGuildRewardItem();
									if( GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nItemID > 0 &&
										GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue > 0 )
									{
										nFestivalPoint += (int)(nFestivalPoint * GuildRewardItem[GUILDREWARDEFFECT_TYPE_EXTRAFESTPOINT].nEffectValue * 0.01);
									}
#if !defined( PRE_ADD_NODELETEGUILD )
								}
#endif
							}
							pUserObj->AddEtcPoint(DBDNWorldDef::EtcPointCode::GuildWar_Festival, nFestivalPoint);
						}
						break;
					}										
				}
				// Send the next one.
				if( pUserObj->IsGuildWarReward() )
				{
					QueryAddGuildWarRewarForCharacter(rand()%THREADMAX, pUserObj);
				}
				else
				{
					//Complete
					pUserObj->SendGuildWarReward(ERROR_NONE);
				}
			}
		}
		break;
	case QUERY_GET_GUILDWAR_REWARD_CHARACTER :	// Get Guild War preliminary reward result
		{
			TAGetGuildWarRewardCharacter* pPacket = reinterpret_cast<TAGetGuildWarRewardCharacter*>(pData);
			if( pPacket->nRetCode == ERROR_NONE)
			{
				// Set reward result
				CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
				if( pUserObj )				
					pUserObj->SetGuildWarRewardResults(pPacket->bRewardResults);				
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWAR_REWARD_CHARACTER Ret:%d!!\r\n", pPacket->nRetCode);
		}
		break;
	case QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL : // Guild War preliminary score standings (top 24)
		{
			TAGetGuildWarPointRunningTotal* pPacket = reinterpret_cast<TAGetGuildWarPointRunningTotal*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendSetGuildWarPointRunning(pPacket);
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR,g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL Ret:%d!!\r\n", pPacket->nRetCode);
		}
		break;
	case QUERY_GET_GUILDWARPOINT_FINAL_REWARDS : // Refresh guild points received as Guild War finals reward
		{
			TAGetGuildWarPointFinalRewards* pPacket = reinterpret_cast<TAGetGuildWarPointFinalRewards*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendGuildWarRefreshGuildPoint(pPacket);
				}
			}
		}
		break;
	case QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD:
		{
			TAGetListGuildWarItemTradeRecord* pPacket = reinterpret_cast<TAGetListGuildWarItemTradeRecord*>(pData);
			if (pPacket->nRetCode == ERROR_NONE)
			{
				CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pPacket->nAccountDBID);
				if (pUserObj)
				{
					for (int i = 0; i < pPacket->count; ++i)
					{
						pUserObj->AddGuildWarBuyedItem(pPacket->itemCount[i].id, pPacket->itemCount[i].count);
					}
				}
			}
			else
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD Ret:%d\r\n", pPacket->nRetCode);
		}
		break;
#endif //#if defined(_VILLAGESERVER)
	case QUERY_MOD_GUILDEXP:
		{
			TAModGuildExp* pPacket = reinterpret_cast<TAModGuildExp*>(pData);

			if (pPacket->nRetCode != ERROR_NONE)
				break;

			const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);
			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
			if (!pGuild)
				break;
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (FALSE == pGuild->IsEnable()) break;
#endif

#if defined(_VILLAGESERVER)
			if (g_pMasterConnection)
				g_pMasterConnection->SendUpdateGuildExp(GuildUID, pPacket);
#elif defined (_GAMESERVER)
			if (g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendUpdateGuildExp(GuildUID, pPacket);
#endif
		}
		break;

	case QUERY_MOD_GUILDCHEAT:
		{
			TAModGuildCheat* pPacket = reinterpret_cast<TAModGuildCheat*>(pData);

			if (pPacket->nRetCode != ERROR_NONE)
				break;

			const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);
			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
			if (!pGuild)
				break;

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (FALSE == pGuild->IsEnable()) break;
#endif

			if (pPacket->cCheatType == 1)
			{
				pGuild->SetInfo()->wGuildLevel = pPacket->cLevel;
				pGuild->SetInfo()->iTotalGuildExp = pPacket->nPoint;
#if defined(_VILLAGESERVER)
				pGuild->SendGuildLevelUp (pPacket->cLevel);
#endif
			}
			else if (pPacket->cCheatType == 2)
			{
				pGuild->SetInfo()->iDailyStagePoint = 0;
				pGuild->SetInfo()->iDailyMissionPoint = 0;
				
			}
		}
		break;
	default:
		return false;
	}
	return true;
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

bool CDNDBConnection::OnRecvMail( int nSubCmd, char* pData )
{
	UINT uiAccountDBID;
	memcpy( &uiAccountDBID, pData, sizeof(UINT) );

	switch( nSubCmd )
	{
		case QUERY_SENDSYSTEMMAIL:
		{
			TASendSystemMail* pMail = reinterpret_cast<TASendSystemMail*>(pData);
			if( pMail->nRetCode != ERROR_NONE )
				return true;

#if defined( _VILLAGESERVER )
			if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				g_pMasterConnection->SendSyncSystemMail( pMail );
#elif defined( _GAMESERVER )
			if( g_pMasterConnectionManager )
				g_pMasterConnectionManager->SendSyncSystemMail( pMail );
#endif // #if defined( _VILLAGESERVER )
			return true;
		}
	}
	return false;
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

bool CDNDBConnection::OnDBRecvFarm(int nSubCmd, char * pData, int nLen)
{
	switch(nSubCmd)
	{
#ifdef _GAMESERVER
		case QUERY_GETLIST_FARM:
			{
				TAGetListFarm * pPacket = (TAGetListFarm*)pData;

				if(pPacket->nRetCode != ERROR_NONE)
				{
					_DANGER_POINT();
					return true;
				}

				if(pPacket->cCount <= 0)
					return true;

				for (int i = 0; i < g_pGameServerManager->GetGameServerSize(); i++)
				{
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerIndex(i);
					if(pServer)
						pServer->StoreExternalBuffer(0, MAINCMD_FARM, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE);
				}
				return true;
			}

		/*case QUERY_UPDATEFARM_ACTIVATION:
			{
				TAUpdateFarmActivation * pPacket = (TAUpdateFarmActivation*)pData;

				if(pPacket->nRetCode != ERROR_NONE)
				{
					_DANGER_POINT();
					return true;
				}

				if(pPacket->cCount <= 0)
					return true;

				for (int i = 0; i < g_pGameServerManager->GetGameServerSize(); i++)
				{
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerIndex(i);
					if(pServer)
						pServer->StoreExternalBuffer(0, MAINCMD_FARM, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE);
				}
				return true;
			}*/
#endif

#ifdef _VILLAGESERVER
		case QUERY_GETLIST_FARM:
		{
			TAGetListFarm * pPacket = (TAGetListFarm*)pData;
			for (int i = 0; i < pPacket->cCount; i++)
#if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )
				g_pFarm->AddFarmList( &pPacket->Farms[i] );
#else
				g_pFarm->AddFarmList(pPacket->Farms[i].iFarmDBID, pPacket->Farms[i].iFarmMapID, pPacket->Farms[i].iFarmMaxUser, pPacket->Farms[i].wszFarmName, pPacket->Farms[i].bStartActivate);
#endif // #if defined( PRE_ADD_VIP_FARM ) || defined( PRE_ADD_FARM_DOWNSCALE )
			g_pFarm->SetInitFlag(true);

			BYTE cCount = 0;
			TFarmItemFromDB	Farms[Farm::Max::FARMCOUNT];
			memset(Farms, 0, sizeof(Farms));
			g_pFarm->GetFarmList(Farms, cCount);
			g_pMasterConnection->SendFarmListGot(Farms, cCount);
			return true;
		}
#endif
		break;
	}
	
	return false;
}

bool CDNDBConnection::OnRecvMasterSystem( int nSubCmd, char* pData )
{
#if defined( _VILLAGESERVER )
	
	UINT uiAccountDBID;
	memcpy( &uiAccountDBID, pData, sizeof(UINT) );

	CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByAccountDBID( uiAccountDBID );

	switch( nSubCmd )
	{
		case QUERY_GET_PAGEMASTERCHARACTER:
		{
			TAGetPageMasterCharacter* pA = reinterpret_cast<TAGetPageMasterCharacter*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMasterList( pSession, pA );
			return true;
		}
		case QUERY_GET_MASTERCHARACTER_TYPE1:
		{
			TAGetMasterCharacterType1* pA = reinterpret_cast<TAGetMasterCharacterType1*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMasterCharacter( pSession, pA );
			return true;
		}
		case QUERY_GET_MASTERCHARACTER_TYPE2:
		{
			TAGetMasterCharacterType2* pA = reinterpret_cast<TAGetMasterCharacterType2*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMyMasterInto( pSession, pA );
			return true;
		}
		case QUERY_GET_PUPILLIST:
		{
			TAGetListPupil* pA = reinterpret_cast<TAGetListPupil*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetPupilList( pSession, pA );
			return true;
		}
		case QUERY_REGISTER_MASTER:
		{
			TAAddMasterCharacter* pA = reinterpret_cast<TAAddMasterCharacter*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetRegisterMaster( pSession, pA );
			return true;
		}
		case QUERY_REGISTERCANCEL_MASTER:
		{
			TADelMasterCharacter* pA = reinterpret_cast<TADelMasterCharacter*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetRegisterMaster( pSession, pA );
			return true;
		}
		case QUERY_JOIN_MASTERSYSTEM:
		{
			TAAddMasterAndPupil* pA = reinterpret_cast<TAAddMasterAndPupil*>(pData);
			if( pSession )
				pSession->SendMasterSystemJoin( pA->nRetCode, pA->bIsDirectMenu, (pA->Code == DBDNWorldDef::TransactorCode::Master) ? true : false, true );

			if( pA->nRetCode == ERROR_NONE )
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendMasterSystemSyncSimpleInfo( pA->Code == DBDNWorldDef::TransactorCode::Master ? pA->biPupilCharacterDBID : pA->biMasterCharacterDBID, MasterSystem::EventType::Join );
					g_pMasterConnection->SendMasterSystemSyncJoin( pA->Code == DBDNWorldDef::TransactorCode::Master ? pA->biPupilCharacterDBID : pA->biMasterCharacterDBID, pA->Code == DBDNWorldDef::TransactorCode::Master ? false : true );
				}
			}

			else if( pSession && pA->bIsDirectMenu == true )
			{
				CDNUserSession* pOppositeSession = g_pUserSessionManager->FindUserSessionByAccountDBID( pA->uiOppositeAccountDBID );
				if( pOppositeSession )
				{
					if( pA->Code == DBDNWorldDef::TransactorCode::Master )
					{
						pOppositeSession->SendMasterSystemJoinDirect( ERROR_MASTERSYSTEM_CANT_JOINSTATE, pSession->GetCharacterName() );
					}
					else
					{
						pOppositeSession->SendMasterSystemInvitePupil( ERROR_MASTERSYSTEM_CANT_JOINSTATE, pSession->GetCharacterName() );
					}
				}
			}

			return true;
		}
		case QUERY_LEAVE_MASTERSYSTEM:
		{
			TADelMasterAndPupil* pA = reinterpret_cast<TADelMasterAndPupil*>(pData);
			if( pSession )
				pSession->SendMasterSystemLeave( pA->nRetCode, pA->Code == DBDNWorldDef::TransactorCode::Master ? true : false, pA->iPenaltyRespectPoint );

			if( pA->nRetCode == ERROR_NONE )
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendMasterSystemSyncSimpleInfo( pA->Code == DBDNWorldDef::TransactorCode::Master ? pA->biPupilCharacterDBID : pA->biMasterCharacterDBID, MasterSystem::EventType::Leave );
					g_pMasterConnection->SendMasterSystemSyncLeave( pA->Code == DBDNWorldDef::TransactorCode::Master ? pA->biPupilCharacterDBID : pA->biMasterCharacterDBID, pA->Code == DBDNWorldDef::TransactorCode::Master ? false : true );
				}
			}
			return true;
		}
		case QUERY_GET_MASTERANDCLASSMATE:
		{
			TAGetListMyMasterAndClassmate* pA = reinterpret_cast<TAGetListMyMasterAndClassmate*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMasterClassmateList( pSession, pA );
			return true;
		}
		case QUERY_GET_CLASSMATEINFO:
		{
			TAGetMyClassmate* pA = reinterpret_cast<TAGetMyClassmate*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetClassmateInfo( pSession, pA );
			return true;
		}
		case QUERY_GET_MASTERANDFAVORPOINT:
		{
			ParseGetMasterAndFavorPoint( reinterpret_cast<TAGetMasterAndFavorPoint*>(pData) );
			return true;
		}
		case QUERY_GRADUATE:
		{
			TAGraduate* pA = reinterpret_cast<TAGraduate*>(pData);

			// Notify master of graduation and sync SimpleInfo
			for( int i=0 ;i<pA->cCount ; ++i )
			{
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
				{
					g_pMasterConnection->SendMasterSystemSyncSimpleInfo( pA->biMasterCharacterDBIDList[i], MasterSystem::EventType::Graduated );
					g_pMasterConnection->SendMasterSystemSyncGraduate( pA->biMasterCharacterDBIDList[i], pA->wszPupilCharName );
				}
			}

			// Pass graduation handling to UserBase since there are still notifications to send to each session.
			return false;
		}
	}
#elif defined( _GAMESERVER )

	switch( nSubCmd )
	{
		case QUERY_GRADUATE:
		{
			TAGraduate* pA = reinterpret_cast<TAGraduate*>(pData);

			// Notify master of graduation and sync SimpleInfo
			for( int i=0 ;i<pA->cCount ; ++i )
			{
				if( g_pMasterConnectionManager )
				{
					g_pMasterConnectionManager->SendMasterSystemSyncSimpleInfo( pA->cWorldSetID, pA->biMasterCharacterDBIDList[i], MasterSystem::EventType::Graduated );
					g_pMasterConnectionManager->SendMasterSystemSyncGraduate( pA->cWorldSetID, pA->biMasterCharacterDBIDList[i], pA->wszPupilCharName );
				}
			}

			// Pass graduation handling to UserBase since there are still notifications to send to each session.
			return false;
		}
		case QUERY_GET_MASTERANDFAVORPOINT:
		{
			ParseGetMasterAndFavorPoint( reinterpret_cast<TAGetMasterAndFavorPoint*>(pData) );
			return true;
		}
	}

#endif // #if defined( _VILLAGESERVER )
	return false;
}

void CDNDBConnection::ParseGetMasterAndFavorPoint( TAGetMasterAndFavorPoint* pA )
{
	if( pA->cCount == 0 )
		return;

	TMasterSystemDecreaseTableData* pTableData = g_pDataManager->GetMasterSystemDecreaseTableData( pA->cLevel );
	if( pTableData == NULL )
		return;

	// Send mail to master
	for( UINT i=0 ; i<pA->cCount ; ++i )
	{
		int iMailID = 0;
		if( pA->MasterAndFavorPoint[i].iFavorPoint >= pTableData->iPupilFavor )
			iMailID = pTableData->iMasterSuccessMailID;
		else
			iMailID = pTableData->iMasterFailureMailID;

		CDNMailSender::Process( pA->MasterAndFavorPoint[i].biCharacterDBID, pA->cWorldSetID, iMailID, pA->wszPupilCharName );
	}

	// Send mail to pupil
	for( UINT i=0 ; i<pA->cCount ; ++i )
	{
		int iMailID = 0;
		if( pA->MasterAndFavorPoint[i].iFavorPoint >= pTableData->iPupilFavor )
			iMailID = pTableData->iPupilSuccessMailID;
		else
			iMailID = pTableData->iPupilFailureMailID;

		CDNMailSender::Process( pA->biPupilCharacterDBID, pA->cWorldSetID, iMailID, pA->MasterAndFavorPoint[i].wszCharName );
	}
}

bool CDNDBConnection::OnDBRecvDBJobSystem(int nSubCmd, char* pData)
{
#if defined(_VILLAGESERVER)
	switch(nSubCmd)
	{
		case QUERY_ADD_JOB_RESERVE:
			{
				TAAddJobReserve * pPacket = (TAAddJobReserve*)pData;
				//Notify Master directly for now. Branch here if used elsewhere.
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
					g_pMasterConnection->SendAddJobReserve(pPacket->nRetCode, pPacket->cJobType, pPacket->nJobSeq);
			}
			break;
		case QUERY_GET_JOB_RESERVE :
			{
				TAGetJobReserve * pPacket = (TAGetJobReserve*)pData;
				//Notify Master directly for now. Branch here if used elsewhere.
				if( g_pMasterConnection && g_pMasterConnection->GetActive() )
					g_pMasterConnection->SendGetJobReserve(pPacket->nRetCode, pPacket->nJobSeq, pPacket->cJobStatus);
			}
			break;
	}
#endif //#if defined(_VILLAGESERVER)
	return true;
}

bool CDNDBConnection::OnRecvGuildRecruit(int nSubCmd, char* pData)
{
#if defined(_VILLAGESERVER)
	UINT uiAccountDBID;
	memcpy( &uiAccountDBID, pData, sizeof(UINT) );

	CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByAccountDBID( uiAccountDBID );

	switch(nSubCmd)
	{
	case QUERY_GET_PAGEGUILDRECRUIT:
		{
			TAGetGuildRecruit* pA = reinterpret_cast<TAGetGuildRecruit*>(pData);
			GuildRecruitSystem::CCacheRepository::GetInstance().SetGuildRecruitList( pSession, pA );			
			return true;
		}
		break;
	case QUERY_GET_PAGEGUILDRECRUITCHARACTER:
		{
			TAGetGuildRecruitCharacter* pA = reinterpret_cast<TAGetGuildRecruitCharacter*>(pData);
			GuildRecruitSystem::CCacheRepository::GetInstance().SetGuildRecruitCharacter( pSession, pA );			
			return true;
		}
		break;
	case QUERY_GET_MYGUILDRECRUIT:
		{
			TAGetMyGuildRecruit* pA = reinterpret_cast<TAGetMyGuildRecruit*>(pData);
			GuildRecruitSystem::CCacheRepository::GetInstance().SetMyGuildRecruit( pSession, pA );
			return true;
		}
		break;
	case QUERY_GET_GUILDRECRUIT_REQUESTCOUNT:
		{
			TAGetGuildRecruitRequestCount* pA = reinterpret_cast<TAGetGuildRecruitRequestCount*>(pData);
			if( pSession )
				pSession->SendGuildRecruitRequestCount( pA->nRetCode, pA->cCount, GuildRecruitSystem::Max::MaxRequestCount );
			return true;
		}
		break;
	case QUERY_REGISTERINFO_GUILDRECRUIT:
		{
			TARegisterInfoGuildRecruit* pA = reinterpret_cast<TARegisterInfoGuildRecruit*>(pData);	
			if( pSession )
			{
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
				pSession->SendRegisterGuildRecruitInfo( pA->cClassCode, pA->nMinLevel, pA->nMaxLevel, pA->wszGuildIntroduction, pA->cPurposeCode, pA->bCheckHomePage );
#else
				pSession->SendRegisterGuildRecruitInfo( pA->cClassCode, pA->nMinLevel, pA->nMaxLevel, pA->wszGuildIntroduction );
#endif
			}
			return true;
		}
		break;
	}	
#endif
	return false;
}

#if defined (PRE_ADD_DONATION)
bool CDNDBConnection::OnRecvDonation(int nSubCmd, char* pData, int nLen)
{
	switch(nSubCmd)
	{
	case QUERY_DONATION_TOP_RANKER:
		{
#if defined(_VILLAGESERVER)
			TADonationTopRanker* pA = reinterpret_cast<TADonationTopRanker*>(pData);
			g_pUserSessionManager->SendWorldSystemMsg(pA->wszCharacterName, WORLDCHATTYPE_DONATION, 0, 0);
#else
			for (int i = 0; i < g_pGameServerManager->GetGameServerSize(); i++)
			{
				CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerIndex(i);
				if(pServer)
					pServer->StoreExternalBuffer(0, MAINCMD_DONATION, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE);
			}
#endif // #if defined(_VILLAGESERVER)
		}
		break;

	default:
		return false;
	}
	return true;
}
#endif // #if defined (PRE_ADD_DONATION)

#if defined( PRE_PARTY_DB ) && defined( _VILLAGESERVER )

bool CDNDBConnection::OnRecvParty( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
		case QUERY_DELPARTY_FORSERVER:
		{
			g_pPartyManager->QueryDelPartyForServerResponse( reinterpret_cast<const TADelPartyForServer*>(pData) );
			return true;
		}
		case QUERY_GETLISTPARTY:
		{
			g_pPartyManager->QueryGetListPartyResponse( reinterpret_cast<const TAGetListParty*>(pData) );
			return true;
		}
		case QUERY_ADDPARTY:
		{
			const TAAddParty* pA = reinterpret_cast<const TAAddParty*>(pData);
			
			CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByAccountDBID( pA->nAccountDBID );
			if( pSession )
				return false;

			if( pA->nRetCode == ERROR_NONE )
				g_pPartyManager->QueryDelParty( pA->Data.PartyData.PartyID );
			break;
		}
		case QUERY_DELPARTY:
		{
			g_pPartyManager->QueryDelPartyResponse( reinterpret_cast<const TADelParty*>(pData) );
			break;
		}
		case QUERY_JOINPARTY:
		{
			const TAJoinParty* pA = reinterpret_cast<const TAJoinParty*>(pData);

			CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByAccountDBID( pA->nAccountDBID );
			if( pSession  )
				return false;
			
			if( pA->nRetCode == ERROR_NONE )
				QueryOutParty( g_Config.nWorldSetID, pA->nAccountDBID, pA->biCharacterDBID, pA->nSessionID, pA->PartyID, Party::QueryOutPartyType::Normal );
			break;
		}
		case QUERY_OUTPARTY:
		{
			const TAOutParty* pA = reinterpret_cast<const TAOutParty*>(pData);
	
			g_pPartyManager->QueryOutPartyResponse( pA );

			CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByAccountDBID( pA->nAccountDBID );
			if( pSession )
				return false;
			// Handle party leave due to user disconnect here
			CDNParty* pParty = g_pPartyManager->GetParty( pA->PartyID );
			if( pParty == NULL )
				return false;
			g_pPartyManager->DelPartyMember( pParty, pA );
			break;
		}
		case QUERY_ADDPARTYANDMEMBERVILLAGE:
		{
			TAAddPartyAndMemberVillage* pA = reinterpret_cast<TAAddPartyAndMemberVillage*>(pData);
			if(pA->nRetCode != ERROR_NONE)
				break;

			// Handle this packet here since the user may not exist in this situation			
			if ( g_pPartyManager->PushParty(pA) == NULL )
				_DANGER_POINT();
			break;
		}
		case QUERY_MODPARTY:
		{
			const TAModParty* pA = reinterpret_cast<const TAModParty*>(pData);
			if(pA->nRetCode != ERROR_NONE)
				break;

			g_pPartyManager->ModParty(pA);
			break;
		}		
#if defined( PRE_WORLDCOMBINE_PARTY )
		case QUERY_GETLISTWORLDPARTY:
			{
				g_pPartyManager->QueryGetListWorldPartyResponse( reinterpret_cast<const TAGetListParty*>(pData) );
				return true;
			}
			break;
#endif
		default:
			return false;
	}
	return true;
}
#elif defined( PRE_PARTY_DB ) && defined( _GAMESERVER ) 
bool CDNDBConnection::OnRecvParty( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
	case QUERY_DELPARTY_FORGAMESERVER:
		{
			const TADelPartyForServer* pA = reinterpret_cast<const TADelPartyForServer*>(pData);
			if(pA->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_DELPARTY_FORGAMESERVER FAIL nRet = %d WorldID = %d\r\n", pA->nRetCode, pA->iServerID);				
			}
		}	
		break;	
	default:
		return false;
	}
	return true;
}
#endif // #if defined( PRE_PARTY_DB ) && defined( _GAMESERVER )

#ifdef PRE_MOD_PVPRANK
bool CDNDBConnection::OnRecvPvP( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
	case QUERY_UPDATE_PVPRANKCRITERIA:
		{
			g_pDBConnectionManager->UpdatedPvPRankCriteria();
		}	
		break;	
#if defined( PRE_WORLDCOMBINE_PVP )
#if defined( _GAMESERVER )
	case QUERY_ADD_WORLDPVPROOM:
		{
			const TAAddWorldPvPRoom* pPacket = reinterpret_cast<const TAAddWorldPvPRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_ADD_WORLDPVPROOM FAIL nRet = %d WorldID = %d\r\n", pPacket->nRetCode, pPacket->cWorldSetID);				
			}
			else
			{
				if( g_pMasterConnectionManager )
				{
					g_pMasterConnectionManager->SendDelWorldPvPRoom( pPacket->cWorldSetID, pPacket->nRoomIndex );					
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByRoomID(pPacket->nRoomID);
					if (pServer)
						pServer->StoreExternalBuffer( 0, MAINCMD_PVP, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE, pPacket->cWorldSetID );
				}
			}
		}
		break;
#endif	// #if defined( _GAMESERVER )
#if defined( _VILLAGESERVER )
	case QUERY_GETLIST_WORLDPVPROOM:
		{
			const TAGetListWorldPvPRoom* pPacket = reinterpret_cast<const TAGetListWorldPvPRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_GETLIST_WORLDPVPROOM FAIL nRet = %d\r\n", pPacket->nRetCode);				
			}
			else
			{
				CDNPvPRoomManager::GetInstance().OnRecvWorldPvPRoomList(pPacket);
			}
		}
		break;
	case QUERY_UPDATE_WORLDPVPROOM:
		{
			const TAUpdateWorldPvPRoom* pPacket = reinterpret_cast<const TAUpdateWorldPvPRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_UPDATE_WORLDPVPROOM FAIL nRet = %d\r\n", pPacket->nRetCode);				
			}
			else
			{
				CDNPvPRoomManager::GetInstance().SetApplyWorldPvPRoom(pPacket->bFlag);
				g_Log.Log(LogType::_NORMAL, L"QUERY_UPDATE_WORLDPVPROOM Sucess nRet = %d\r\n", pPacket->bFlag);				
			}
		}
		break;
#endif	// #if defined( _VILLAGESERVER )
#endif	// #if defined( PRE_WORLDCOMBINE_PVP )
#if defined( PRE_PVP_GAMBLEROOM )
#if defined( _GAMESERVER )
	case QUERY_ADD_GAMBLEROOM:
		{
			const TAAddGambleRoom* pPacket = reinterpret_cast<const TAAddGambleRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_ADD_GAMBLEROOM FAIL nRet = %d WorldID = %d\r\n", pPacket->nRetCode, pPacket->cWorldSetID);				
			}
			else
			{
				if( g_pMasterConnectionManager )
				{
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByRoomID(pPacket->nRoomID);
					if (pServer)
						pServer->StoreExternalBuffer( 0, MAINCMD_PVP, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE, pPacket->cWorldSetID );
				}
			}
		}
		break;
#endif	// #if defined( _GAMESERVER )
#endif // #if defined( PRE_PVP_GAMBLEROOM )
	default:
		return false;
	}
	return true;
}
#endif		//#ifdef PRE_MOD_PVPRANK

#if defined( PRE_WORLDCOMBINE_PVP ) && !defined(PRE_MOD_PVPRANK)
bool CDNDBConnection::OnRecvPvP( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{	
#if defined( PRE_WORLDCOMBINE_PVP )
#if defined( _GAMESERVER )
	case QUERY_ADD_WORLDPVPROOM:
		{
			const TAAddWorldPvPRoom* pPacket = reinterpret_cast<const TAAddWorldPvPRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_ADD_WORLDPVPROOM FAIL nRet = %d WorldID = %d\r\n", pPacket->nRetCode, pPacket->cWorldSetID);				
			}
			else
			{
				if( g_pMasterConnectionManager )
				{
					g_pMasterConnectionManager->SendDelWorldPvPRoom( pPacket->cWorldSetID, pPacket->nRoomIndex );					
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByRoomID(pPacket->nRoomID);
					if (pServer)
						pServer->StoreExternalBuffer( 0, MAINCMD_PVP, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE, pPacket->cWorldSetID );
				}
			}
		}
		break;
#endif // #if defined( _GAMESERVER )
#if defined( _VILLAGESERVER )
	case QUERY_GETLIST_WORLDPVPROOM:
		{
			const TAGetListWorldPvPRoom* pPacket = reinterpret_cast<const TAGetListWorldPvPRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_GETLIST_WORLDPVPROOM FAIL nRet = %d\r\n", pPacket->nRetCode);				
			}
			else
			{
				CDNPvPRoomManager::GetInstance().OnRecvWorldPvPRoomList(pPacket);
			}
		}
		break;
#endif // #if defined( _VILLAGESERVER )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#if defined( PRE_PVP_GAMBLEROOM )
#if defined( _GAMESERVER )
	case QUERY_ADD_GAMBLEROOM:
		{
			const TAAddGambleRoom* pPacket = reinterpret_cast<const TAAddGambleRoom*>(pData);
			if(pPacket->nRetCode != ERROR_NONE)
			{
				g_Log.Log(LogType::_ERROR, L"QUERY_ADD_GAMBLEROOM FAIL nRet = %d WorldID = %d\r\n", pPacket->nRetCode, pPacket->cWorldSetID);				
			}
			else
			{
				if( g_pMasterConnectionManager )
				{
					CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByRoomID(pPacket->nRoomID);
					if (pServer)
						pServer->StoreExternalBuffer( 0, MAINCMD_PVP, nSubCmd, pData, nLen, EXTERNALTYPE_DATABASE, pPacket->cWorldSetID );
				}
			}
		}
		break;
#endif	// #if defined( _GAMESERVER )
#endif // #if defined( PRE_PVP_GAMBLEROOM )
	default:
		return false;
	}
	return true;
}
#endif // #if defined( PRE_WORLDCOMBINE_PVP ) && !defined(PRE_MOD_PVPRANK)

#if defined( PRE_PRIVATECHAT_CHANNEL )
bool CDNDBConnection::OnRecvPrivateChannel( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
	case QUERY_GET_PRIVATECHATCHANNEL:
		{
			const TAPrivateChatChannelInfo* pA = reinterpret_cast<const TAPrivateChatChannelInfo*>(pData);
			if(g_pPrivateChatChannelManager)
			{
				for( int i=0;i<pA->nCount;i++ )
				{
					g_pPrivateChatChannelManager->AddPrivateChatChannel(pA->cWorldSetID, pA->tPrivateChatChannel[i]);
				}
			}			
		}
		break;
	case QUERY_GET_PRIVATECHATCHANNELMEMBER:
		{
			const TAPrivateChatChannelMember* pA = reinterpret_cast<const TAPrivateChatChannelMember*>(pData);			
			
			if (ERROR_NONE == pA->nRetCode)
			{			
				if(g_pPrivateChatChannelManager)
				{
					CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pA->cWorldSetID, pA->nPrivateChatChannelID);
					if(pPrivateChatChannel)
					{
						TPrivateChatChannelMember Member;						
						for( int i=0;i<pA->nCount;i++ )
						{
							memset(&Member, 0, sizeof(Member));
							Member = pA->Member[i];
							if( pPrivateChatChannel->GetMasterCharacterDBID() == Member.biCharacterDBID )
								Member.bMaster = true;
								
							pPrivateChatChannel->AddPrivateChannelMember(Member);
						}
						pPrivateChatChannel->SetLoaded(true);
					}				
				}			
			}
			return false;
		}
		break;
	case QUERY_CREATE_PRIVATECHATCHANNEL:
		{
			const TAAddPrivateChatChannel* pA = reinterpret_cast<const TAAddPrivateChatChannel*>(pData);
			
			if( pA->nRetCode == ERROR_NONE )
			{
				g_pPrivateChatChannelManager->AddPrivateChatChannel(pA->cWorldSetID, pA->tPrivateChatChannel);

				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pA->cWorldSetID, pA->tPrivateChatChannel.nPrivateChatChannelID);
				if(pPrivateChatChannel)
				{
					TPrivateChatChannelMember Member;
					memset(&Member, 0, sizeof(Member));
					Member.tJoinDate = pA->tJoinDate;
					Member.nAccountDBID = pA->nAccountDBID;
					Member.biCharacterDBID = pA->tPrivateChatChannel.biMasterCharacterDBID;
					Member.bMaster = true;
					_wcscpy(Member.wszCharacterName, _countof(Member.wszCharacterName), pA->wszCharacterName, (int)wcslen(pA->wszCharacterName));					

					pPrivateChatChannel->AddPrivateChannelMember( Member );
					pPrivateChatChannel->SetLoaded(true);
#if defined( _VILLAGESERVER )					
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{
						g_pMasterConnection->SendAddPrivateChatChannel( pPrivateChatChannel->GetChannelInfo() );
						g_pMasterConnection->SendAddPrivateChatChannelMember( pA->tPrivateChatChannel.nPrivateChatChannelID, Member );
					}
#elif defined( _GAMESERVER )
					if( g_pMasterConnectionManager )
					{
						g_pMasterConnectionManager->SendAddPrivateChatChannel( pA->cWorldSetID, pPrivateChatChannel->GetChannelInfo() );
						g_pMasterConnectionManager->SendAddPrivateChatChannelMember( pA->cWorldSetID, pA->tPrivateChatChannel.nPrivateChatChannelID, Member );
					}
#endif
				}
			}
			return false;
		}
		break;
	case QUERY_ADD_PRIVATECHATMEMBER:
		{
			const TAAddPrivateChatMember* pA = reinterpret_cast<const TAAddPrivateChatMember*>(pData);
			
			if( pA->nRetCode == ERROR_NONE || pA->nRetCode == 103345 )
			{
				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pA->cWorldSetID, pA->nPrivateChatChannelID);

				if(pPrivateChatChannel)
				{
					TPrivateChatChannelMember Member;
					memset(&Member, 0, sizeof(Member));
					Member = pA->Member;

					if( !pPrivateChatChannel->CheckPrivateChannelMember(pPrivateChatChannel->GetMasterCharacterDBID()) )
					{
						Member.bMaster = true;
						pPrivateChatChannel->ModPrivateChannelInfo(  PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), Member.biCharacterDBID );
					}

					pPrivateChatChannel->AddPrivateChannelMember( Member );					
#if defined( _VILLAGESERVER )					
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{					
						g_pMasterConnection->SendAddPrivateChatChannelMember( pA->nPrivateChatChannelID, Member );
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{					
						g_pMasterConnectionManager->SendAddPrivateChatChannelMember( pA->cWorldSetID, pA->nPrivateChatChannelID, Member );
					}
#endif
				}
			}
			return false;
		}
		break;		
	case QUERY_KICK_PRIVATECHATMEMBER:
		{
			const TADelPrivateChatMember* pA = reinterpret_cast<const TADelPrivateChatMember*>(pData);
			
			if( pA->nRetCode == ERROR_NONE )
			{
				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pA->cWorldSetID, pA->nPrivateChatChannelID);

				if(pPrivateChatChannel)
				{					
					pPrivateChatChannel->DelPrivateChannelMember( PrivateChatChannel::Common::KickMember, pA->biCharacterDBID );					

#if defined( _VILLAGESERVER )				
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{						
						g_pMasterConnection->SendKickPrivateChatChannelMemberResult( pA->biCharacterDBID );
						g_pMasterConnection->SendDelPrivateChatChannelMember( PrivateChatChannel::Common::KickMember, pA->nPrivateChatChannelID, pA->biCharacterDBID, pA->wszName );
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{					
						g_pMasterConnectionManager->SendKickPrivateChatChannelMemberResult( pA->cWorldSetID, pA->biCharacterDBID );
						g_pMasterConnectionManager->SendDelPrivateChatChannelMember( pA->cWorldSetID, PrivateChatChannel::Common::KickMember, pA->nPrivateChatChannelID, pA->biCharacterDBID, pA->wszName );
					}
#endif
				}
			}	
			return false;
		}
		break;
	case QUERY_INVITE_PRIVATECHATMEMBER:
		{
			const TAInvitePrivateChatMember* pA = reinterpret_cast<const TAInvitePrivateChatMember*>(pData);

			if( pA->nRetCode == ERROR_NONE )
			{
				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pA->cWorldSetID, pA->nPrivateChatChannelID);

				if(pPrivateChatChannel)
				{
					TPrivateChatChannelMember Member;
					memset(&Member, 0, sizeof(Member));
					Member = pA->Member;

					if(pPrivateChatChannel->GetMasterCharacterDBID() == 0)
						Member.bMaster = true;

					pPrivateChatChannel->AddPrivateChannelMember( Member );					
#if defined( _VILLAGESERVER )					
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{					
						g_pMasterConnection->SendAddPrivateChatChannelMember( pA->nPrivateChatChannelID, Member );
						g_pMasterConnection->SendInvitePrivateChatChannelResult(ERROR_NONE, pA->nMasterAccountDBID);
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{					
						g_pMasterConnectionManager->SendAddPrivateChatChannelMember( pA->cWorldSetID, pA->nPrivateChatChannelID, Member );
						g_pMasterConnectionManager->SendInvitePrivateChatChannelResult(pA->cWorldSetID, ERROR_NONE, pA->nMasterAccountDBID);
					}
#endif
				}
			}
			return false;
		}
		break;
	case QUERY_DEL_PRIVATEMEMBERSERVERID:
		{
			const TADelPrivateChatMemberServerID* pA = reinterpret_cast<const TADelPrivateChatMemberServerID*>(pData);
			for( int i=0;i<pA->nCount;i++ )
			{
				CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo(pA->cWorldSetID, pA->Member[i].nPrivateChatChannelID);
				if( pPrivateChatChannel )
				{
#if defined( _VILLAGESERVER )
					if(g_pMasterConnection && g_pMasterConnection->GetActive() )
					{					
						g_pMasterConnection->SendDelPrivateChatChannelMember( PrivateChatChannel::Common::OutMember, pA->Member[i].nPrivateChatChannelID, pA->Member[i].biCharacterDBID, pA->Member[i].wszCharacterName );
					}
#elif defined( _GAMESERVER )
					if(g_pMasterConnectionManager )
					{					
						g_pMasterConnectionManager->SendDelPrivateChatChannelMember( pA->cWorldSetID, PrivateChatChannel::Common::OutMember, pA->Member[i].nPrivateChatChannelID, pA->Member[i].biCharacterDBID, pA->Member[i].wszCharacterName );
					}
#endif
				}
			}
		}
		break;
	default:
		return false;
	}	
	return true;
}
#endif



int CDNDBConnection::QueryMWTest( TQMWTest* pQ )
{
	return AddSendData( MAINCMD_ITEM, QUERY_MWTEST, (char*)pQ, sizeof(TQMWTest) );
}

//#########################################################################################################################################
// MAINCMD_AUTH
//#########################################################################################################################################

int CDNDBConnection::QueryStoreAuth(char cLastServerType, CDNUserSession *pUserSession)
{
	TQStoreAuth Auth;
	memset(&Auth, 0, sizeof(Auth));

	Auth.cThreadID = pUserSession->GetDBThreadID();
	Auth.nAccountDBID = pUserSession->GetAccountDBID();
	Auth.biCertifyingKey = pUserSession->GetCertifyingKey();
	Auth.nCurServerID = g_pAuthManager->GetServerID();
	Auth.cWorldSetID = pUserSession->GetWorldSetID();
	Auth.nSessionID = pUserSession->GetSessionID();
	Auth.biCharacterDBID = pUserSession->GetCharacterDBID();
	if (pUserSession->GetAccountName())
		_wcscpy(Auth.wszAccountName, _countof(Auth.wszAccountName), pUserSession->GetAccountName(), (int)wcslen(pUserSession->GetAccountName()));
	if (pUserSession->GetCharacterName())
		_wcscpy(Auth.wszCharacterName, _countof(Auth.wszCharacterName), pUserSession->GetCharacterName(), (int)wcslen(pUserSession->GetCharacterName()));

	Auth.bIsAdult = pUserSession->m_bAdult;
	Auth.nAge = pUserSession->GetAge();
	Auth.nDailyCreateCount = pUserSession->GetDailyCreateCount();
	Auth.nPrmInt1 = pUserSession->GetPrmInt1();
	Auth.cLastServerType = cLastServerType;

	return (AddSendData(MAINCMD_AUTH, QUERY_STOREAUTH, (char*)&Auth, sizeof(Auth)));
}

int CDNDBConnection::QueryCheckAuth(INT64 biCertifyingKey, CDNUserSession *pUserSesion)
{
	return QueryCheckAuth(biCertifyingKey, pUserSesion->GetDBThreadID(), pUserSesion->GetAccountDBID(), pUserSesion->GetSessionID());
}

int CDNDBConnection::QueryCheckAuth(INT64 biCertifyingKey, BYTE cThreadID, int nAccountDBID, int nSessionID)
{
	TQCheckAuth Auth;

	Auth.cThreadID = cThreadID;
	Auth.nAccountDBID = nAccountDBID;
	Auth.biCertifyingKey = biCertifyingKey;
	Auth.nCurServerID = g_pAuthManager->GetServerID();
	Auth.nSessionID = nSessionID;

	return (AddSendData(MAINCMD_AUTH, QUERY_CHECKAUTH, (char*)&Auth, sizeof(Auth)));
}

int CDNDBConnection::QueryResetAuth(BYTE cThreadID, int uAccountDBID, UINT nSessionID)
{
	TQResetAuth Auth;

	Auth.cThreadID = cThreadID;
	Auth.nAccountDBID = uAccountDBID;
	Auth.nSessionID = nSessionID;

	return (AddSendData(MAINCMD_AUTH, QUERY_RESETAUTH, (char*)&Auth, sizeof(Auth)));
}

int CDNDBConnection::QueryResetAuthServer( BYTE cThreadID, int nServerID)
{
	TQResetAuthServer Auth;

	Auth.cThreadID = cThreadID;
	Auth.nServerID = nServerID;

	return (AddSendData(MAINCMD_AUTH, QUERY_RESETAUTHSERVER, (char*)&Auth, sizeof(Auth)));
}

int CDNDBConnection::QueryResetErrorCheck( UINT uAccountDBID )
{
	TQHeader Reset;
	memset(&Reset, 0, sizeof(TQHeader));
	
	Reset.nAccountDBID = uAccountDBID;

	return (AddSendData(MAINCMD_AUTH, QUERY_RESETERRORCHECK, (char*)&Reset, sizeof(Reset)));
}

//#########################################################################################################################################
// MAINCMD_STATUS
//#########################################################################################################################################

// Character related
#if defined( PRE_PARTY_DB )
void CDNDBConnection::QuerySelectCharacter(BYTE cThreadID, CDNUserSession *pSession)
#else
void CDNDBConnection::QuerySelectCharacter(BYTE cThreadID, CDNUserSession *pSession, TPARTYID PartyID)
#endif
{
	TQSelectCharacter Select;
	memset(&Select, 0, sizeof(TQSelectCharacter));

	Select.cThreadID = cThreadID;
	Select.cWorldSetID = pSession->GetWorldSetID();
	Select.nAccountDBID = pSession->GetAccountDBID();
	Select.biCharacterDBID = pSession->GetCharacterDBID();
#if !defined( PRE_PARTY_DB )
	Select.PartyID = PartyID;
#endif
	Select.nLastServerType = _SERVER_TYPEID;
	if (pSession->GetAccountName())
		_wcscpy(Select.wszAccountName, _countof(Select.wszAccountName), pSession->GetAccountName(), (int)wcslen(pSession->GetAccountName()));

	AddSendData(MAINCMD_STATUS, QUERY_SELECTCHARACTER, (char*)&Select, sizeof(TQSelectCharacter));
}

void CDNDBConnection::QueryUpdateUserData(int nMainCmd, CDNUserSession *pSession, TUserData *pUserData, bool bExecuteScheduleedTask )
{
	TQUpdateCharacter Update;
	memset(&Update, 0, sizeof(TQUpdateCharacter));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nMapIndex = pUserData->Status.nMapIndex;
	Update.nLastVillageMapIndex = pUserData->Status.nLastVillageMapIndex;
	Update.nLastSubVillageMapIndex = pUserData->Status.nLastSubVillageMapIndex;
	Update.cLastVillageGateNo = pUserData->Status.cLastVillageGateNo;
	Update.nPosX = pUserData->Status.nPosX;
	Update.nPosY = pUserData->Status.nPosY;
	Update.nPosZ = pUserData->Status.nPosZ;
	Update.fRotate = pUserData->Status.fRotate;
	Update.nGlyphDelayTime = pUserData->Status.nGlyphDelayTime;
	Update.nGlyphRemainTime = pUserData->Status.nGlyphRemainTime;
	Update.bExecuteScheduleedTask = bExecuteScheduleedTask;
	memcpy(Update.cViewCashEquipBitmap, pUserData->Status.cViewCashEquipBitmap, sizeof(Update.cViewCashEquipBitmap));

	memcpy(Update.QuickSlot, pUserData->Status.QuickSlot, sizeof(Update.QuickSlot));
	for (int i = 0; i < SKILLMAX; i++){
		Update.SkillCoolTime[i].nSkillID = pUserData->Skill[0].SkillList[i].nSkillID;
		Update.SkillCoolTime[i].nCoolTime = pUserData->Skill[0].SkillList[i].nCoolTime;
	}	

	for (int i = 0; i < SKILLMAX; i++){
		Update.SkillCoolTime2[i].nSkillID = pUserData->Skill[1].SkillList[i].nSkillID;
		Update.SkillCoolTime2[i].nCoolTime = pUserData->Skill[1].SkillList[i].nCoolTime;
	}	

#if defined( PRE_ITEMBUFF_COOLTIME )
	CDNUserItem *pItem = pSession->GetItem();
#if defined(PRE_PERIOD_INVENTORY)
	for (int i = 0; i < INVENTORYTOTALMAX; i++)
	{
		if (i == pItem->GetInventoryCount()){
			i = INVENTORYMAX;
		}
#else	// #if defined(PRE_PERIOD_INVENTORY)
	for (int i = 0; i < pItem->GetInventoryCount(); i++)
	{
#endif	// #if defined(PRE_PERIOD_INVENTORY)
		if(!pItem->GetInventory(i)) 
			continue;
		
		if( pItem->GetInventory(i)->nCoolTime <= 0)
			continue;

		TItemData* pItemData = g_pDataManager->GetItemData( pItem->GetInventory(i)->nItemID );
		if ( pItemData == NULL )
			return;

		if( pItemData->nType != ITEMTYPE_GLOBAL_PARTY_BUFF)
			continue;

		Update.Inventory[i].biItemSerial = pItem->GetInventory(i)->nSerial;
		Update.Inventory[i].nCoolTime = pItem->GetInventory(i)->nCoolTime;
	}

#if defined(PRE_PERIOD_INVENTORY)
	for (int i = 0; i < WAREHOUSETOTALMAX; i++){
		if (i == pItem->GetWarehouseCount()){
			i = WAREHOUSEMAX;
		}
#else	// #if defined(PRE_PERIOD_INVENTORY)
	for (int i = 0; i < pItem->GetWarehouseCount(); i++){
#endif	// #if defined(PRE_PERIOD_INVENTORY)
		if(!pItem->GetWarehouse(i)) 
			continue;
		
		if( pItem->GetWarehouse(i)->nCoolTime <= 0)
			continue;

		TItemData* pItemData = g_pDataManager->GetItemData( pItem->GetWarehouse(i)->nItemID );//GetInventory
		if ( pItemData == NULL )
			return;

		if( pItemData->nType != ITEMTYPE_GLOBAL_PARTY_BUFF)
			continue;

		Update.Warehouse[i].biItemSerial = pItem->GetWarehouse(i)->nSerial;
		Update.Warehouse[i].nCoolTime = pItem->GetWarehouse(i)->nCoolTime;
	}

	pItem->GetCashInventoryCoolTime(Update.CashInventory);
#if defined( PRE_ADD_SERVER_WAREHOUSE )
	pItem->GetServerWareCoolTime(Update.ServerWare);
	pItem->GetServerWareCashCoolTime(Update.ServerWareCash);
#endif // #if defined( PRE_ADD_SERVER_WAREHOUSE )
#endif // #if defined( PRE_ITEMBUFF_COOLTIME )
	/*
	CDNUserItem *pItem = pSession->GetItem();
	for (int i = 0; i < EQUIPMAX; i++){
		if(!pItem->GetEquip(i)) continue;
		Update.Equip[i].biItemSerial = pItem->GetEquip(i)->nSerial;
		Update.Equip[i].nCoolTime = pItem->GetEquip(i)->nCoolTime;
	}
	for (int i = 0; i < INVENTORYMAX; i++){
		if(!pItem->GetInventory(i)) continue;
		Update.Inventory[i].biItemSerial = pItem->GetInventory(i)->nSerial;
		Update.Inventory[i].nCoolTime = pItem->GetInventory(i)->nCoolTime;
	}
	for (int i = 0; i < WAREHOUSEMAX; i++){
		if(!pItem->GetWarehouse(i)) continue;
		Update.Warehouse[i].biItemSerial = pItem->GetWarehouse(i)->nSerial;
		Update.Warehouse[i].nCoolTime = pItem->GetWarehouse(i)->nCoolTime;
	}
	for (int i = 0; i < CASHEQUIPMAX; i++){
		if(!pItem->GetCashEquip(i)) continue;
		Update.CashEquip[i].biItemSerial = pItem->GetCashEquip(i)->nSerial;
		Update.CashEquip[i].nCoolTime = pItem->GetCashEquip(i)->nCoolTime;
	}
	for (int i = 0; i < pItem->GetCashInventoryCount(); i++){
		if(!pItem->GetCashInventory(i)) 
			continue;
		if( i >= _countof(Update.CashInventory) )
			continue;
		Update.CashInventory[i].biItemSerial = pItem->GetCashInventory(i)->nSerial;
		Update.CashInventory[i].nCoolTime = pItem->GetCashInventory(i)->nCoolTime;
	}
	*/

#if defined(_VILLAGESERVER)
	g_pUserSessionManager->AddSaveSendData(nMainCmd, (char*)&Update, sizeof(TQUpdateCharacter));

#elif defined(_GAMESERVER)
	CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByAID(pSession->GetAccountDBID());
	if(pServer)
	{
		pServer->AddSaveSendData(nMainCmd, (char*)&Update, sizeof(TQUpdateCharacter));
	}	
	else
		_DANGER_POINT();
#endif
}

//Pet EXP: added INT64 biPetItemSerial, int nPetExp.
void CDNDBConnection::QueryChangeStageUserData(CDNUserSession *pSession, BYTE cPCBangRebirthCoin, std::vector<INT64> &VecSerialList, std::vector<USHORT> &VecDurList, 
											   int nDeathCount, BYTE cDifficult, INT64 biPickUpCoin, INT64 biPetItemSerial, int nPetExp)
{
	TQChangeStageUserData Update;
	memset(&Update, 0, sizeof(TQChangeStageUserData));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.PartyID = pSession->GetPartyID();
	Update.nExp = pSession->GetExp();
	Update.biCoin = pSession->GetCoin();
	Update.cRebirthCoin = pSession->GetRebirthCoin() + pSession->GetVIPRebirthCoin();
	Update.cPCBangRebirthCoin = cPCBangRebirthCoin;
	Update.nDeathCount = nDeathCount;
	Update.cDifficult = cDifficult;
	Update.biPickUpCoin = biPickUpCoin;
	Update.biPetItemSerial = biPetItemSerial;
	Update.nPetExp = nPetExp;

	int nCount = 0;
	if(!VecSerialList.empty()){
		for (int i = 0; i <(int)VecSerialList.size(); i++){
			Update.ItemDurArr[i].biSerial = VecSerialList[i];
			nCount++;
		}
	}
	if(!VecDurList.empty()){
		for (int i = 0; i <(int)VecDurList.size(); i++){
			Update.ItemDurArr[i].wDur = VecDurList[i];
		}
	}

	Update.nCount = nCount;
	int nLen = sizeof(TQChangeStageUserData) - sizeof(Update.ItemDurArr) +(sizeof(DBPacket::TDurabilityInfo) * nCount);

	AddSendData(MAINCMD_STATUS, QUERY_CHANGESTAGEUSERDATA, (char*)&Update, nLen);
}

void CDNDBConnection::QueryGetCharacterPartialyByName(CDNUserSession *pSession, const WCHAR *pCharName, BYTE cReqType)
{
	TQGetCharacterPartialyByName Packet;
	memset( &Packet, 0, sizeof(Packet) );

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	if(pCharName) _wcscpy(Packet.wszName, NAMELENMAX, pCharName, (int)wcslen(pCharName));
	Packet.cReqType = cReqType;
	AddSendData(MAINCMD_STATUS, QUERY_GETCHARACTERPARTIALYBYNAME, (char*)&Packet, sizeof(TQGetCharacterPartialyByName));
}

void CDNDBConnection::QueryGetCharacterPartialyByDBID(CDNUserSession *pSession, INT64 biCharacterDBID, BYTE cReqType)
{
	TQGetCharacterPartialyByDBID Packet;
	memset( &Packet, 0, sizeof(Packet) );

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = biCharacterDBID;
	Packet.cReqType = cReqType;
	AddSendData(MAINCMD_STATUS, QUERY_GETCHARACTERPARTIALYBYDBID, (char*)&Packet, sizeof(TQGetCharacterPartialyByDBID));
}

void CDNDBConnection::QueryCheckFirstVillage(CDNUserSession *pSession)
{
	TQCheckFirstVillage Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STATUS, QUERY_CHECKFIRSTVILLAGE, (char*)&Update, sizeof(TQCheckFirstVillage));
}

void CDNDBConnection::QueryLevel(CDNUserSession *pSession, char cChangeCode)
{
	TQLevel Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cLevel = pSession->GetLevel();
	Update.cChangeCode = cChangeCode;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	if( pSession->GetCharacterName() )
		_wcscpy( Update.wszCharName, _countof(Update.wszCharName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );

	AddSendData(MAINCMD_STATUS, QUERY_LEVEL, (char*)&Update, sizeof(TQLevel));
}
void CDNDBConnection::QueryExp(CDNUserSession *pSession, char cChangeCode, INT64 biFKey)
{
	TQExp Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nExp = pSession->GetExp();
	Update.cChangeCode = cChangeCode;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.biFKey = biFKey;

	AddSendData(MAINCMD_STATUS, QUERY_EXP, (char*)&Update, sizeof(TQExp));
}

void CDNDBConnection::QueryCoin(CDNUserSession *pSession, char cCoinChangeCode, INT64 biChangeKey, INT64 nChangeCoin, INT64 nTotalCoin)
{
	TQCoin Update;
	memset(&Update, 0, sizeof(TQCoin));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cCoinChangeCode = cCoinChangeCode;
	Update.biChangeKey = biChangeKey;
	Update.nChangeCoin = nChangeCoin;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.nTotalCoin = nTotalCoin;

	AddSendData(MAINCMD_STATUS, QUERY_COIN, (char*)&Update, sizeof(TQCoin));
}

void CDNDBConnection::QueryWarehouseCoin(CDNUserSession *pSession, char cCoinChangeCode, INT64 nChangeCoin, INT64 nTotalCoin, INT64 nTotalWarehouseCoin)
{
	TQWarehouseCoin Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cCoinChangeCode = cCoinChangeCode;
	Update.nChangeCoin = nChangeCoin;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.nTotalCoin = nTotalCoin;
	Update.nTotalWarehouseCoin = nTotalWarehouseCoin;

	AddSendData(MAINCMD_STATUS, QUERY_WAREHOUSECOIN, (char*)&Update, sizeof(TQWarehouseCoin));
}

void CDNDBConnection::QueryMapInfo(CDNUserSession *pSession)
{
	TQMapInfo Update;
	memset(&Update, 0, sizeof(TQMapInfo));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cLastVillageGateNo = pSession->GetLastVillageGateNo();
	Update.nLastVillageMapIndex = pSession->GetLastVillageMapIndex();
	Update.nLastSubVillageMapIndex = pSession->GetLastSubVillageMapIndex();
	Update.nMapIndex = pSession->GetMapIndex();

	AddSendData(MAINCMD_STATUS, QUERY_MAPINFO, (char*)&Update, sizeof(TQMapInfo));
}

void CDNDBConnection::QueryCashRebirthCoin(CDNUserSession *pSession)
{
	TQCashRebirthCoin Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	_wcscpy( Update.wszIP, _countof(Update.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData(MAINCMD_STATUS, QUERY_CASHREBIRTHCOIN, (char*)&Update, sizeof(TQCashRebirthCoin));
}

void CDNDBConnection::QueryAddCashRebirthCoin(CDNUserSession *pSession, int nRebirthCount, int nRebirthPrice, int nRebirthCode, int nRebirthKey)
{
	TQAddCashRebirthCoin Update;
	memset(&Update, 0, sizeof(TQAddCashRebirthCoin));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nRebirthCount = nRebirthCount;
	Update.nRebirthPrice = nRebirthPrice;
	Update.nRebirthCode = nRebirthCode;
	Update.nRebirthKey = nRebirthKey;	

	AddSendData(MAINCMD_STATUS, QUERY_ADDCASHREBIRTHCOIN, (char*)&Update, sizeof(TQAddCashRebirthCoin));
}

void CDNDBConnection::QueryRebirthCoin(CDNUserSession *pSession, int nRebirthCoin, int nPCBangRebirthCoin, int nVIPRebirthCoin)
{
	TQRebirthCoin Update;
	memset(&Update, 0, sizeof(TQRebirthCoin));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cRebirthCoin = nRebirthCoin + nVIPRebirthCoin;
	Update.cPCBangRebirthCoin = nPCBangRebirthCoin;

	AddSendData(MAINCMD_STATUS, QUERY_REBIRTHCOIN, (char*)&Update, sizeof(TQRebirthCoin));
}

void CDNDBConnection::QueryFatigue(CDNUserSession *pSession, char cFatigueTypeCode, int nChangeFatigue)
{
	TQFatigue Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cFatigueTypeCode = cFatigueTypeCode;
	Update.nChangeFatigue = nChangeFatigue;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();

	AddSendData(MAINCMD_STATUS, QUERY_FATIGUE, (char*)&Update, sizeof(TQFatigue));
}

void CDNDBConnection::QueryEventFatigue(CDNUserSession *pSession, int nChangeFatigue, bool bReset, char cInitTime)
{
	TQEventFatigue Update;
	memset(&Update, 0, sizeof(TQEventFatigue));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nChangeFatigue = nChangeFatigue;
	Update.bReset = bReset;
	Update.cInitTime = cInitTime;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();

	AddSendData(MAINCMD_STATUS, QUERY_EVENTFATIGUE, (char*)&Update, sizeof(TQEventFatigue));
}

void CDNDBConnection::QueryJob(CDNUserSession *pSession, int nJob)
{
	TQJob Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cJob = nJob;

	AddSendData(MAINCMD_STATUS, QUERY_JOB, (char*)&Update, sizeof(TQJob));
}

void CDNDBConnection::QueryGlyphDelayTime(CDNUserSession *pSession, int nTime)
{
	TQGlyphAttributeTime Update;
	memset(&Update, 0, sizeof(Update));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nTime = nTime;

	AddSendData(MAINCMD_STATUS, QUERY_GLYPHDELAYTIME, (char*)&Update, sizeof(TQGlyphAttributeTime));
}

void CDNDBConnection::QueryGlyphRemainTime(CDNUserSession *pSession, int nTime)
{
	TQGlyphAttributeTime Update;
	memset(&Update, 0, sizeof(Update));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nTime = nTime;

	AddSendData(MAINCMD_STATUS, QUERY_GLYPHREMAINTIME, (char*)&Update, sizeof(TQGlyphAttributeTime));
}

void CDNDBConnection::QueryNotifier(CDNUserSession *pSession, int nSlot, int nNotifierType, int nID)
{
	TQNotifier Update;

	memset(&Update, 0, sizeof(TQNotifier));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cNotifierSlot = nSlot;
	Update.cNotifierType = static_cast<char>(nNotifierType);
	Update.nID = nID;

	AddSendData(MAINCMD_STATUS, QUEST_NOTIFIER, (char*)&Update, sizeof(TQNotifier));
}

#if !defined(PRE_DELETE_DUNGEONCLEAR)
void CDNDBConnection::QueryDungeonClear(CDNUserSession *pSession, int nType, int nMapIndex)
{
	TQDungeonClear Update;
	memset(&Update, 0, sizeof(TQDungeonClear));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cType = nType;
	Update.nMapIndex = nMapIndex;

	AddSendData(MAINCMD_STATUS, QUERY_DUNGEONCLEAR, (char*)&Update, sizeof(TQDungeonClear));
}
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

void CDNDBConnection::QueryEternityItem(CDNUserSession *pSession, int nEternityCode, int nValue)
{
	TQEternityItem Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cEternityCode = nEternityCode;
	Update.nEternityValue = nValue;

	AddSendData(MAINCMD_STATUS, QUERY_ETERNITYITEM, (char*)&Update, sizeof(TQEternityItem));
}

void CDNDBConnection::QueryColor(CDNUserSession *pSession, int nItemType, DWORD dwColor)
{
	TQColor Update;
	memset(&Update, 0, sizeof(Update));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cItemType = nItemType;
	Update.dwColor = dwColor;

	AddSendData(MAINCMD_STATUS, QUERY_COLOR, (char*)&Update, sizeof(TQColor));
}

void CDNDBConnection::QueryNestClear(CDNUserSession *pSession, int nMapIndex, char cClearType/* = NestClear::Type::Normal*/)
{
	TQNestClearCount Update;
	memset(&Update, 0, sizeof(TQNestClearCount));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nMapIndex = nMapIndex;
	Update.cClearType = cClearType;

	AddSendData(MAINCMD_STATUS, QUERY_NESTCLEARCOUNT, (char*)&Update, sizeof(TQNestClearCount));
}

void CDNDBConnection::QueryInitNestClearCount(CDNUserSession *pSession)
{
	TQInitNestClearCount Update;
	memset(&Update, 0, sizeof(TQInitNestClearCount));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STATUS, QUERY_INITNESTCLEARCOUNT, (char*)&Update, sizeof(TQInitNestClearCount));
}

void CDNDBConnection::QueryChangeCharacterName(CDNUserSession *pSession, LPCWSTR pChangeCharacterName, INT64 biItemSerial)
{
	TQChangeCharacterName packet;
	memset(&packet, 0, sizeof(TQChangeCharacterName));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.nCharacterDBID = pSession->GetCharacterDBID();
	packet.biItemSerial = biItemSerial;
	packet.btChangeCode = CHANGECODE_CHARNAME; // Character name change item code (set forcibly)
	packet.nReservedPeriodDay = RESERVEDPERIOD_CHARNAME;
	packet.nNextPeriodDay = NEXTPERIOD_CHARNAME;

	if(pSession->GetCharacterName()) _wcscpy(packet.wszOriginName, _countof(packet.wszOriginName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()));
	if(pChangeCharacterName) _wcscpy(packet.wszCharacterName, _countof(packet.wszCharacterName), pChangeCharacterName, (int)wcslen(pChangeCharacterName));

	AddSendData(MAINCMD_STATUS, QUERY_CHANGECHARACTERNAME, (char*)&packet, sizeof(TQChangeCharacterName));
}

void CDNDBConnection::QueryGetListEtcPoint(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetListEtcPoint  packet;
	memset(&packet, 0, sizeof(TQGetListEtcPoint));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.nCharacterDBID = pSession->GetCharacterDBID();
	
	AddSendData(MAINCMD_STATUS, QUERY_GETLIST_ETCPOINT, (char*)&packet, sizeof(TQGetListEtcPoint));
}

// Currently point usage is only used for favorability, so handle it here
// Readjust the type later when it becomes shared for common use.
void CDNDBConnection::QueryAddEtcPoint( CDNUserSession *pSession, char cType, INT64 biPoint )
{
	TQAddEtcPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cType = cType;
	TxPacket.biAddPoint = biPoint;
	TxPacket.nMapID = pSession->GetMapIndex();
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData( MAINCMD_STATUS, QUERY_ADD_ETCPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryUseEtcPoint( CDNUserSession *pSession, char cType, INT64 biPoint )
{
	TQUseEtcPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cType = cType;
	TxPacket.biUsePoint = biPoint;
	TxPacket.nMapID = pSession->GetMapIndex();
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData( MAINCMD_STATUS, QUERY_USE_ETCPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
void CDNDBConnection::QueryChangeJobCode( CDNUserSession *pSession, int nFirstJobIDBefore, int nFirstJobIDAfter, int nSecondJobIDBefore, int nSecondJobIDAfter )
{
	TQChangeJobCode Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nFirstJobIDBefore = nFirstJobIDBefore;
	Update.nFirstJobIDAfter = nFirstJobIDAfter;
	Update.nSecondJobIDBefore = nSecondJobIDBefore;
	Update.nSecondJobIDAfter = nSecondJobIDAfter;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();

	AddSendData( MAINCMD_STATUS, QUERY_CHANGEJOBCODE, (char*)&Update, sizeof(TQChangeJobCode) );
}
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

void CDNDBConnection::QueryGetListVariableReset( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetListVariableReset TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_STATUS, QUERY_GETLIST_VARIABLERESET, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModVariableReset( CDNUserSession *pSession, CommonVariable::Type::eCode Type, INT64 biValue, __time64_t ResetTime )
{
	TQModVariableReset TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.Type = Type;
	TxPacket.biValue = biValue;
	TxPacket.tResetTime = ResetTime;

	AddSendData( MAINCMD_STATUS, QUERY_MOD_VARIABLERESET, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryChagneSkillPage(CDNUserSession *pSession, char cSkillPage)
{
	TQChangeSkillPage TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cSkillPage	= cSkillPage;
	
	AddSendData( MAINCMD_STATUS, QUERY_CHANGE_SKILLPAGE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
void CDNDBConnection::QueryGetTotalSkillLevel(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetTotalLevelSkill TxPacket;
	memset(&TxPacket, 0, sizeof(TQGetTotalLevelSkill));

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STATUS, QUERY_GET_TOTALSKILLLEVEL, (char*)&TxPacket, sizeof(TQGetTotalLevelSkill));
}
void CDNDBConnection::QueryAddTotalSkillLevel(CDNUserSession *pSession, int nSlotIndex, int nSkillID)
{
	TQAddTotalLevelSkill TxPacket;
	memset(&TxPacket, 0, sizeof(TQAddTotalLevelSkill));

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nSlotIndex = nSlotIndex;
	TxPacket.nSkillID = nSkillID;

	AddSendData(MAINCMD_STATUS, QUERY_ADD_TOTALSKILLLEVEL, (char*)&TxPacket, sizeof(TQAddTotalLevelSkill));
}
#endif

#ifdef PRE_ADD_PRESET_SKILLTREE
void CDNDBConnection::QueryGetSkillPresetList(CDNUserSession * pSession)
{
	TQGetSKillPreSetList packet;
	memset(&packet, 0, sizeof(TQGetSKillPreSetList));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STATUS, QUERY_GET_SKILLPRESET_LIST, (char*)&packet, sizeof(TQGetSKillPreSetList));
}

void CDNDBConnection::QueryAddSkillPreset(CDNUserSession * pSession, CSSaveSkillSet * pSkill)
{
	TQAddSkillPreSet packet;
	memset(&packet, 0, sizeof(TQAddSkillPreSet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cIndex = pSkill->cIndex;
	_wcscpy(packet.wszName, SKILLPRESETNAMEMAX, pSkill->wszSetName, (int)wcslen(pSkill->wszSetName));
	packet.cCount = pSkill->cCount;
	memcpy_s(packet.SKill, sizeof(packet.SKill), pSkill->SKills, sizeof(TSkillSetPartialData) * packet.cCount);

	AddSendData(MAINCMD_STATUS, QUERY_ADD_SKILLPRESET, (char*)&packet, sizeof(TQAddSkillPreSet) - sizeof(packet.SKill) + (sizeof(TSkillSetPartialData) * packet.cCount));
}

void CDNDBConnection::QueryDelSkillPreset(CDNUserSession * pSession, BYTE cIndex)
{
	TQDelSkillPreSet packet;
	memset(&packet, 0, sizeof(TQDelSkillPreSet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cIndex = cIndex;

	AddSendData(MAINCMD_STATUS, QUERY_DEL_SKILLPRESET, (char*)&packet, sizeof(TQDelSkillPreSet));
}
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE


//#########################################################################################################################################
// MAINCMD_ETC
//#########################################################################################################################################

// Query-Queue
void CDNDBConnection::QueryUpdateQueryQueue( const TDBQueryQueue *pDBQueryQueue)
{
	DN_ASSERT(NULL != pDBQueryQueue,		"Invalid!");
	DN_ASSERT(pDBQueryQueue->CheckIntegrity(),	"Invalid!");

	AddSendData(MAINCMD_ETC, QUERY_UPDATEQUERYQUEUE, reinterpret_cast<char*>(const_cast<TDBQueryQueue*>(pDBQueryQueue)), pDBQueryQueue->GetSize());
}

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
void CDNDBConnection::QuerySimpleConfig(BYTE cThreadID)
{
	TQSimpleConfig packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;

	AddSendData(MAINCMD_ETC, QUERY_GET_SIMPLECONFIG,(char*)&packet, sizeof(packet));
}
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

#ifdef PRE_ADD_SEETMAINTENANCEFLAG
void CDNDBConnection::QuerySetMaintenanceFlag(BYTE cThreadID, int nFlag)
{
	TQMaintenanceInfo packet;
	memset(&packet, 0, sizeof(TQMaintenanceInfo));

	packet.cThreadID = cThreadID;
	packet.nFlag = nFlag;

	AddSendData(MAINCMD_ETC, QUERY_MAINTENANCEFLAG, reinterpret_cast<char*>(&packet), sizeof(packet));
}
#endif		//#ifdef PRE_ADD_SEETMAINTENANCEFLAG


// Event
void CDNDBConnection::QueryEventList(BYTE cThreadID, int nWorldSetID)
{
	TQEventList packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;

	AddSendData(MAINCMD_ETC, QUERY_EVENTLIST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryLogout(CDNUserSession *pSession)
{
	TQLogout Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nSessionID = pSession->GetSessionID();
	AddSendData(MAINCMD_ETC, QUERY_LOGOUT, (char*)&Update, sizeof(TQLogout));
}

void CDNDBConnection::QueryModLastConnectDate(CDNUserSession *pSession)
{
	TQHeader Packet = {0,};

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();

	AddSendData(MAINCMD_ETC, QUERY_MODLASTCONNECTDATE, (char*)&Packet, sizeof(TQHeader));
}

void CDNDBConnection::QueryLoginCharacter( CDNUserSession *pSession )
{
	TQLoginCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.uiSessionID = pSession->GetSessionID();
	if(pSession->GetIpW()) _wcscpy( TxPacket.wszIP, IPLENMAX, pSession->GetIpW(), (int)wcslen(pSession->GetIpW()) );

	AddSendData(MAINCMD_ETC, QUERY_LOGINCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryLogoutCharacter( CDNUserSession *pSession )
{
	TQLogoutCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.uiSessionID = pSession->GetSessionID();
	if(pSession->GetViewCashEquipBitmap())
		memcpy(TxPacket.cViewCashEquipBitmap, pSession->GetViewCashEquipBitmap(), sizeof(TxPacket.cViewCashEquipBitmap));

	AddSendData(MAINCMD_ETC, QUERY_LOGOUTCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

// GameOption
void CDNDBConnection::QueryGetGameOption(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetGameOption packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_ETC, QUERY_GETGAMEOPTION, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QuerySetGameOption(CDNUserSession *pSession, TGameOptions * pOption)
{
	TQSetGameOption packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	memcpy_s(&packet.Option, sizeof(TGameOptions), pOption, sizeof(TGameOptions));

	AddSendData(MAINCMD_ETC, QUERY_SETGAMEOPTION, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryUpdateSecondAuthPassword( BYTE cThreadID, UINT uiAccountDBID, CSGameOptionUpdateSecondAuthPassword* pPacket )
{
	TQUpdateSecondAuthPassword TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.nAccountDBID = uiAccountDBID;
	memcpy( TxPacket.wszOldPW, MakeSecondPasswordString( szVersion, pPacket->nOldSeed, pPacket->nOldValue ).c_str(), sizeof(WCHAR)*SecondAuth::Common::PWMaxLength );
	memcpy( TxPacket.wszNewPW, MakeSecondPasswordString( szVersion, pPacket->nNewSeed, pPacket->nNewValue ).c_str(), sizeof(WCHAR)*SecondAuth::Common::PWMaxLength );

	AddSendData( MAINCMD_ETC, QUERY_UPDATE_SECONDAUTH_PASSWORD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryUpdateSecondAuthLock( BYTE cThreadID, UINT uiAccountDBID, CSGameOptionUpdateSecondAuthLock* pPacket )
{
	TQUpdateSecondAuthLock TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.bIsLock		= pPacket->bIsLock;
	if( !pPacket->bIsLock )	// Verify password if UnLock
		memcpy( TxPacket.wszPW, MakeSecondPasswordString( szVersion, pPacket->nSeed, pPacket->nValue ).c_str(), sizeof(WCHAR)*SecondAuth::Common::PWMaxLength );

	AddSendData( MAINCMD_ETC, QUERY_UPDATE_SECONDAUTH_LOCK, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryInitSecondAuth( BYTE cThreadID, UINT uiAccountDBID )
{
	TQInitSecondAuth TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.nAccountDBID	= uiAccountDBID;

	AddSendData( MAINCMD_ETC, QUERY_INIT_SECONDAUTH, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryValidateSecondAuth( BYTE cThreadID, UINT uiAccountDBID, const WCHAR* wszPW, int nAuthCheckType )
{
	_ASSERT( wszPW );

	TQValidateSecondAuth TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.nAuthCheckType	= nAuthCheckType;
	memcpy( TxPacket.wszPW, wszPW, sizeof(WCHAR)*SecondAuth::Common::PWMaxLength );

	AddSendData( MAINCMD_ETC, QUERY_VALIDATE_SECONDAUTH, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID )
{
	TQGetKeySettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;

	AddSendData( MAINCMD_ETC, QUERY_GET_KEYSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, TKeySetting* pKeySetting )
{
	TQModKeySettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;
	TxPacket.bUseDefault	= (pKeySetting == NULL) ? true : false;
	if( pKeySetting )
		memcpy( &TxPacket.sKeySetting, pKeySetting, sizeof(TKeySetting) );

	AddSendData( MAINCMD_ETC, QUERY_MOD_KEYSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryUpdateKeySetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, CSGameOptionUpdateKeySetting* pPacket )
{
	TQModKeySettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;
	TxPacket.bUseDefault	= pPacket->bIsDefault;
	if( !TxPacket.bUseDefault )
		memcpy( &TxPacket.sKeySetting, &pPacket->sKeySetting, sizeof(TKeySetting) );

	AddSendData( MAINCMD_ETC, QUERY_MOD_KEYSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID )
{
	TQGetPadSettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;

	AddSendData( MAINCMD_ETC, QUERY_GET_PADSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModPadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, TPadSetting* pPadSetting )
{
	TQModPadSettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;
	TxPacket.bUseDefault	= (pPadSetting == NULL) ? true : false;
	if( pPadSetting )
		memcpy( &TxPacket.sPadSetting, pPadSetting, sizeof(TPadSetting) );

	AddSendData( MAINCMD_ETC, QUERY_MOD_PADSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryUpdatePadSetting( BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, CSGameOptionUpdatePadSetting* pPacket )
{
	TQModPadSettingOption TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= nWorldSetID;
	TxPacket.nAccountDBID	= nAccountDBID;
	TxPacket.bUseDefault	= pPacket->bIsDefault;
	if( !TxPacket.bUseDefault )
		memcpy( &TxPacket.sPadSetting, &pPacket->sPadSetting, sizeof(TPadSetting) );

	AddSendData( MAINCMD_ETC, QUERY_MOD_PADSETTING_OPTION, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetProfile( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetProfile TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_ETC, QUERY_GET_PROFILE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QuerySetProfile( CDNUserSession *pSession, BYTE cGender, const WCHAR * pwzGreeting, bool bOpenPublic )
{
	TQSetProfile TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	TxPacket.sProfile.cGender = cGender;
	memcpy( TxPacket.sProfile.wszGreeting, pwzGreeting, sizeof(WCHAR) * PROFILEGREETINGLEN );
	TxPacket.sProfile.bOpenPublic = bOpenPublic;

	AddSendData( MAINCMD_ETC, QUERY_SET_PROFILE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetRestraint(CDNUserSession *pSession)
{
	TQRestraint	packet;
	memset(&packet, 0, sizeof(TQRestraint));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.cWorldSetID = pSession->GetWorldSetID();

	AddSendData(MAINCMD_ETC, QUERY_RESTRAINT, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddRestraint(CDNUserSession *pSession, int nTartgetCode, int nRestraintCode, const WCHAR * pwszRestraintReason, const WCHAR * pwszRestraintReasonForDolis, int nDurationDay, int nDolisReasonCode)
{
	TQSetRestraint packet;
	memset(&packet, 0, sizeof(TQSetRestraint));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.cTargetCode = (BYTE)nTartgetCode;
	packet.cRestraintCode = (BYTE)nRestraintCode;
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	if(pwszRestraintReason) _wcscpy(packet.wszRestraintreason, RESTRAINTREASONMAX, pwszRestraintReason, (int)wcslen(pwszRestraintReason));
	if(pwszRestraintReasonForDolis) _wcscpy(packet.wszRestraintreasonForDolis, RESTRAINTREASONFORDOLISMAX, pwszRestraintReasonForDolis, (int)wcslen(pwszRestraintReasonForDolis));
	packet.nDurationDays = (USHORT)nDurationDay;
	packet.nDolisReasonCode = nDolisReasonCode;

	AddSendData(MAINCMD_ETC, QUERY_SETRESTRAINT, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QuerySaveConnectDurationTime(CDNUserSession *pSession)
{
	TQSaveConnectDurationTime Packet;
	memset(&Packet, 0, sizeof(TQSaveConnectDurationTime));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	TTimeEventGroup *pTimeEvent = pSession->GetTimeEventData();
	for (int i = 0; i < TIMEEVENTMAX; i++){
		if(pTimeEvent->Event[i].nItemID <= 0) continue;
		Packet.Event[i] = pTimeEvent->Event[i];
		Packet.cCount++;
	}

	AddSendData(MAINCMD_ETC, QUERY_UPDATECONNECTINGTIME, (char*)&Packet, sizeof(TQSaveConnectDurationTime) - sizeof(Packet.Event) +(sizeof(TTimeEvent) * Packet.cCount));
}

void CDNDBConnection::QueryModTimeEventDate( CDNUserSession *pSession )
{
	TQModTimeEventDate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STATUS, QUERY_MOD_TIMEEVENT_DATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddAbuseLog( CDNUserSession *pSession, eAbuseCode eType, wchar_t* buf )
{
	TQAddAbuseLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cAbuseCode	= static_cast<BYTE>(eType);
	TxPacket.unLen	= static_cast<USHORT>(wcslen(buf));
	if( TxPacket.unLen >= _countof(TxPacket.wszBuf) )
		return;

	_wcscpy( TxPacket.wszBuf, _countof(TxPacket.wszBuf), buf, (int)wcslen(buf) );

	int iLen = sizeof(TxPacket)-sizeof(TxPacket.wszBuf)+(sizeof(WCHAR)*(TxPacket.unLen+1));
	AddSendData( MAINCMD_ETC, QUERY_ADD_ABUSELOG, reinterpret_cast<char*>(&TxPacket), iLen );

}

void CDNDBConnection::QueryAddAbuseMonitor( CDNUserSession *pSession, int iCount, int iCount2 )
{
	TQAddAbuseMonitor TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iCount	= iCount;
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	TxPacket.iCount2 = iCount2;
#endif
	AddSendData( MAINCMD_ETC, QUERY_ADD_ABUSEMONITOR, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDelAbuseMonitor( CDNUserSession *pSession, char cType )
{
	TQDelAbuseMonitor TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	TxPacket.cType = cType;
#endif
	AddSendData( MAINCMD_ETC, QUERY_DEL_ABUSEMONITOR, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetAbuseMonitor( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetAbuseMonitor TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_ETC, QUERY_GET_ABUSEMONITOR, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetWholeAbuseMonitor(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetWholeAbuseMonitor packet;
	memset(&packet, 0, sizeof(TQGetWholeAbuseMonitor));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();

	AddSendData(MAINCMD_ETC, QUERY_GET_WHOLE_ABUSEMONITOR, reinterpret_cast<char*>(&packet), sizeof(TQGetWholeAbuseMonitor));
}

void CDNDBConnection::QueryModCharacterSlotCount(CDNUserSession *pSession)
{
	TQModCharacterSlotCount Packet;
	
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_ETC, QUERY_MODCHARACTERSLOTCOUNT, reinterpret_cast<char*>(&Packet), sizeof(TQModCharacterSlotCount));
}


//#########################################################################################################################################
// MAINCMD_MISSION
//#########################################################################################################################################

void CDNDBConnection::QueryMissionGain(CDNUserSession *pSession)
{
	TQMissionGain Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	memcpy(Update.MissionGain, pSession->GetMissionData()->MissionGain, sizeof(Update.MissionGain));

	AddSendData(MAINCMD_MISSION, QUERY_MISSIONGAIN, (char*)&Update, sizeof(TQMissionGain));
}

void CDNDBConnection::QueryMissionAchieve(CDNUserSession *pSession, int nMissionID)
{
	TQMissionAchieve Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nMissionID = nMissionID;
	Update.cMissionType = g_pDataManager->GetMissionType(nMissionID);
	memcpy(Update.MissionAchieve, pSession->GetMissionData()->MissionAchieve, sizeof(Update.MissionAchieve));

	AddSendData(MAINCMD_MISSION, QUERY_MISSIONACHIEVE, (char*)&Update, sizeof(TQMissionAchieve));
}

void CDNDBConnection::QuerySetDailyMission(CDNUserSession *pSession)
{
	TQSetDailyMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < DAILYMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->DailyMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETDAILYMISSION, (char*)&Update, sizeof(TQSetDailyMission));
}

void CDNDBConnection::QuerySetWeeklyMission(CDNUserSession *pSession)
{
	TQSetWeeklyMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < WEEKLYMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->WeeklyMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETWEEKLYMISSION, (char*)&Update, sizeof(TQSetWeeklyMission));
}

#if defined(PRE_ADD_MONTHLY_MISSION)
void CDNDBConnection::QuerySetMonthlyMission(CDNUserSession *pSession)
{
	TQSetMonthlyMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < MONTHLYMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->MonthlyMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETMONTHLYMISSION, (char*)&Update, sizeof(TQSetMonthlyMission));
}
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

void CDNDBConnection::QuerySetWeekendEventMission(CDNUserSession *pSession)
{
	TQSetWeekendEventMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < WEEKENDEVENTMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->WeekendEventMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETWEEKENDEVENTMISSION, (char*)&Update, sizeof(TQSetWeekendEventMission));
}

void CDNDBConnection::QueryModDailyWeeklyMission(CDNUserSession *pSession, char cDailyMissionType, char cIndex, TDailyMission *pMission, bool bIsRepeatMission/*=false*/)
{
	TQModDailyWeeklyMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cDailyMissionType = cDailyMissionType;
	Update.cMissionIndex = cIndex;
	Update.Mission = *pMission;

	if(pMission->bAchieve)
	{
		if( bIsRepeatMission )
			Update.nMissionScore = 1;
		else
			Update.nMissionScore = g_pDataManager->GetDailyMissionRewarePoint(pMission->nMissionID);
	}

	AddSendData(MAINCMD_MISSION, QUERY_MODDAILYWEEKLYMISSION, (char*)&Update, sizeof(TQModDailyWeeklyMission));
}

void CDNDBConnection::QuerySetGuildWarMission(CDNUserSession *pSession)
{
	TQSetGuildWarMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < GUILDWARMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->GuildWarMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETGUILDWARMISSION, (char*)&Update, sizeof(TQSetGuildWarMission));
}

void CDNDBConnection::QueryModGuildWarMission(CDNUserSession *pSession, char cDailyMissionType, char cIndex, TDailyMission *pMission, bool bAchieve)
{
	TQModDailyWeeklyMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cDailyMissionType = cDailyMissionType;
	Update.cMissionIndex = cIndex;
	Update.Mission = *pMission;
	if(bAchieve) Update.nMissionScore = 1;

	AddSendData(MAINCMD_MISSION, QUERY_MODDAILYWEEKLYMISSION, (char*)&Update, sizeof(TQModDailyWeeklyMission));
}

void CDNDBConnection::QuerySetGuildCommonMission(CDNUserSession *pSession)
{
	TQSetGuildCommonMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < GUILDCOMMONMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->GuildCommonMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETGUILDCOMMONMISSION, (char*)&Update, sizeof(TQSetGuildCommonMission));
}

void CDNDBConnection::QuerySetWeekendRepeatMission(CDNUserSession *pSession)
{
	TQSetWeekendRepeatMission Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < WEEKENDREPEATMISSIONMAX; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->WeekendRepeatMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETWEEKENDREPEATMISSION, (char*)&Update, sizeof(TQSetWeekendRepeatMission));
}

void CDNDBConnection::QuerySetPCBangMission(CDNUserSession *pSession)
{
	TQSetPCBangMission Update;
	memset(&Update, 0, sizeof(TQSetPCBangMission));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	for (int i = 0; i < PCBangMissionMax; i++){
		Update.nMissionIDs[i] = pSession->GetMissionData()->PCBangMission[i].nMissionID;
	}

	AddSendData(MAINCMD_MISSION, QUERY_SETPCBANGMISSION, (char*)&Update, sizeof(TQSetPCBangMission));
}

void CDNDBConnection::QueryDelAllMissionsForCheat(CDNUserSession *pSession)
{
	TQClearMission ClearMission;
	memset(&ClearMission, 0, sizeof(TQClearMission));

	ClearMission.cThreadID = pSession->GetDBThreadID();
	ClearMission.cWorldSetID = pSession->GetWorldSetID();
	ClearMission.nAccountDBID = pSession->GetAccountDBID();
	ClearMission.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MISSION, QUERY_CLEARMISSION, (char*)&ClearMission, sizeof(TQClearMission));
}

//#########################################################################################################################################
// MAINCMD_APPELLATION
//#########################################################################################################################################

void CDNDBConnection::QueryAddAppellation(CDNUserSession *pSession, int nAppellationID)
{
	TQAddAppellation Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nAppellationID = nAppellationID;
	memcpy(Update.Appellation, pSession->GetAppellationData()->Appellation, sizeof(Update.Appellation));

	AddSendData(MAINCMD_APPELLATION, QUERY_ADDAPPELLATION, (char*)&Update, sizeof(TQAddAppellation));
}

void CDNDBConnection::QuerySelectAppellation(CDNUserSession *pSession, int nSelect, char cKind)
{
	TQSelectAppellation Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nSelectAppellation = nSelect;
	Update.cAppellationKind = cKind;

	AddSendData(MAINCMD_APPELLATION, QUERY_SELECTAPPELLATION, (char*)&Update, sizeof(TQSelectAppellation));
}

void CDNDBConnection::QueryDelAppellation(CDNUserSession *pSession, int ArrayIndex)
{
	TQDelAppellation DelAppellation;
	
	memset( &DelAppellation, 0, sizeof(DelAppellation) );	
	DelAppellation.cThreadID = pSession->GetDBThreadID();
	DelAppellation.cWorldSetID = pSession->GetWorldSetID();
	DelAppellation.nAccountDBID = pSession->GetAccountDBID();
	DelAppellation.biCharacterDBID = pSession->GetCharacterDBID();
	DelAppellation.nDelAppellation = ArrayIndex;

	AddSendData(MAINCMD_APPELLATION, QUERY_DELETEAPPELLATION, (char*)&DelAppellation, sizeof(TQDelAppellation));
}

//#########################################################################################################################################
// MAINCMD_SKILL
//#########################################################################################################################################

void CDNDBConnection::QueryAddSkill(CDNUserSession *pSession, int nSkillID, char cSkillLevel, int nCoolTime, char cSkillChangeCode, INT64 biBuyPrice, INT64 biCurrentCoin/*=0*/, INT64 biPickUpCoin/*=0*/)
{
	TQAddSkill Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nSkillID = nSkillID;
	Update.cSkillLevel = cSkillLevel;
	Update.nCoolTime = nCoolTime;
	Update.cSkillChangeCode = cSkillChangeCode;
	Update.biCoin = biBuyPrice;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.cSkillPage = pSession->GetSkillPage();
	Update.biCurrentCoin = biCurrentCoin;
	Update.biPickUpCoin = biPickUpCoin;

	AddSendData(MAINCMD_SKILL, QUERY_ADDSKILL, (char*)&Update, sizeof(TQAddSkill));
}

void CDNDBConnection::QueryModSkillLevel(CDNUserSession *pSession, int nSkillID, BYTE cSkillLevel, int nCoolTime, USHORT wUsedSkillPoint, char cSkillChangeCode)
{
	TQModSkillLevel Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nSkillID = nSkillID;
	Update.cSkillLevel = cSkillLevel;
	Update.nCoolTime = nCoolTime;
	Update.wUsedSkillPoint = wUsedSkillPoint;
	Update.cSkillChangeCode = cSkillChangeCode;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.cSkillPage = pSession->GetSkillPage();

	AddSendData(MAINCMD_SKILL, QUERY_MODSKILLLEVEL, (char*)&Update, sizeof(TQModSkillLevel));
}

void CDNDBConnection::QueryIncreaseSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nChangeCode, int nSkillPage)
{
	TQIncreaseSkillPoint Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.wSkillPoint = wSkillPoint;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.cSkillPointChangeCode = nChangeCode;
	Update.cSkillPage = (char)nSkillPage;

	AddSendData(MAINCMD_SKILL, QUERY_INCREASESKILLPOINT, (char*)&Update, sizeof(TQIncreaseSkillPoint));
}

void CDNDBConnection::QueryDecreaseSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nSkillPage)
{
	TQDecreaseSkillPoint Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nDecreaseSkillPoint = wSkillPoint;
	Update.iChannelID = pSession->GetChannelID();
	Update.iMapID = pSession->GetMapIndex();
	Update.cSkillPage = (char)nSkillPage;

	AddSendData(MAINCMD_SKILL, QUERY_DECREASESKILLPOINT, (char*)&Update, sizeof(Update));
}

void CDNDBConnection::QueryDelSkill(CDNUserSession *pSession, int nSkillID, char cSkillChangeCode, bool bSkillPointBack, int nSkillPage)
{
	TQDelSkill Update;
	memset(&Update, 0, sizeof(TQDelSkill));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nSkillID = nSkillID;
	Update.cSkillChangeCode = cSkillChangeCode;
	Update.bSkillPointBack = bSkillPointBack;
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.cSkillPage = (char)nSkillPage;

	AddSendData(MAINCMD_SKILL, QUERY_DELSKILL, (char*)&Update, sizeof(TQDelSkill));
}

void CDNDBConnection::QueryResetSkill(CDNUserSession *pSession, int nSkillPage)
{
	TQResetSkill Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nChannelID = pSession->GetChannelID();
	Update.nMapID = pSession->GetMapIndex();
	Update.cSkillPage = (char)nSkillPage;

	AddSendData(MAINCMD_SKILL, QUERY_RESETSKILL, (char*)&Update, sizeof(TQResetSkill));
}

void CDNDBConnection::QueryResetSkillBySkillIDs( CDNUserSession *pSession, std::vector<int>& vSkillIDs )
{
	_ASSERT( vSkillIDs.size() < SKILLMAX );

	TQResetSkillBySkillIDS TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iChannelID = pSession->GetChannelID();
	TxPacket.iMapID = pSession->GetMapIndex();
	TxPacket.cSkillPage	= pSession->GetSkillPage();
	
	for( UINT i=0 ; i<vSkillIDs.size() ; ++i )
		TxPacket.iSkillIDs[i] = vSkillIDs[i];

	AddSendData(MAINCMD_SKILL, QUERY_RESETSKILLBYSKILLIDS, (char*)&TxPacket, sizeof(TxPacket));
}

void CDNDBConnection::QueryUseExpandSkillPage( CDNUserSession *pSession, int *nSkillArray, short wSkillPoint)
{
	TQUseExpandSkillPage TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.wSkillPoint = wSkillPoint;

	memcpy(TxPacket.nSkillArray, nSkillArray, sizeof(int) * DEFAULTSKILLMAX);

	AddSendData(MAINCMD_SKILL, QUERY_USEEXPANDSKILLPAGE, (char*)&TxPacket, sizeof(TxPacket));
}

void CDNDBConnection::QuerySetSkillPoint(CDNUserSession *pSession, USHORT wSkillPoint, int nSkillPage/*=0*/ )
{
	TQSetSkillPoint Update;
	memset( &Update, 0, sizeof(Update) );

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.nSkillPoint = wSkillPoint;	
	Update.cSkillPage = (char)nSkillPage;

	AddSendData(MAINCMD_SKILL, QUERY_SETSKILLPOINT, (char*)&Update, sizeof(Update));
}

//#########################################################################################################################################
// MAINCMD_ITEM
//#########################################################################################################################################

void CDNDBConnection::QueryAddMaterializedItem(CDNUserSession *pSession, BYTE cItemSlotIndex, char cAddMaterializedItemCode, INT64 biFKey, const TItem &AddItem, INT64 biItemPrice,
											   INT64 biSenderCharacterDBID, BYTE cItemLocationCode, BYTE cPayMethodCode, bool bMerge, INT64 biMergeTargetItemSerial, TVehicle *pVehicleEquip/* = NULL*/)
{
	TQAddMaterializedItem Item;
	memset(&Item, 0, sizeof(TQAddMaterializedItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.cItemLocationIndex = cItemSlotIndex;
	Item.Code = (DBDNWorldDef::AddMaterializedItem::eCode)cAddMaterializedItemCode;
	Item.biFKey = biFKey;
	Item.AddItem = AddItem;
	Item.bStackableFlag = (g_pDataManager->GetItemOverlapCount(Item.AddItem.nItemID) == 1) ? false : true;
	Item.biItemPrice = biItemPrice;
	Item.biSenderCharacterDBID = biSenderCharacterDBID;
	Item.cItemLocationCode = cItemLocationCode;
	Item.cPayMethodCode = cPayMethodCode;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapIndex = pSession->GetMapIndex();
	_wcscpy( Item.wszIP, _countof(Item.wszIP), pSession->GetIpW(), IPLENMAX );
	Item.bMerge = bMerge;
	Item.biMergeTargetItemSerial = biMergeTargetItemSerial;

	if(pVehicleEquip){
		Item.dwPetPartsColor1 = pVehicleEquip->dwPartsColor1;
		Item.dwPetPartsColor2 = pVehicleEquip->dwPartsColor2;
		Item.nPetSkillID1 = pVehicleEquip->nSkillID1;
		Item.nPetSkillID2 = pVehicleEquip->nSkillID2;
		for (int i = Vehicle::Slot::Saddle; i < Vehicle::Slot::Max; i++){
			if(pVehicleEquip->Vehicle[i].nItemID <= 0) continue;
			Item.PetList[Item.cPetCount].cSlotIndex = i;
			Item.PetList[Item.cPetCount].nItemID = pVehicleEquip->Vehicle[i].nItemID;
			Item.PetList[Item.cPetCount].biItemSerial = pVehicleEquip->Vehicle[i].nSerial;
			Item.cPetCount++;
		}
	}

	AddSendData(MAINCMD_ITEM, QUERY_ADDITEM, (char*)&Item, sizeof(TQAddMaterializedItem) - sizeof(Item.PetList) +(sizeof(DBPacket::TAddMaterializedPet) * Item.cPetCount));
}

void CDNDBConnection::QueryUseItem(CDNUserSession *pSession, char cUseItemCode, INT64 biItemSerial, short wUsedItemCount, bool bAllFlag, int iTargetLogItemID/*=0*/, char cTargetLogItemLevel/*=0*/ )
{
	return QueryUseItemEx(pSession->GetDBThreadID(), pSession->GetWorldSetID(), pSession->GetAccountDBID(), cUseItemCode, biItemSerial, wUsedItemCount, pSession->GetChannelID(), pSession->GetMapIndex(), 
		pSession->GetIpW(), bAllFlag, iTargetLogItemID, cTargetLogItemLevel);
}

void CDNDBConnection::QueryUseItemEx(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, char cUseItemCode, INT64 biItemSerial, short wUsedItemCount, int nChannelID, int nMapID, const WCHAR* pwszIP, bool bAllFlag, int iTargetLogItemID/*=0*/, char cTargetLogItemLevel/*=0*/)
{
	TQUseItem Item;
	memset(&Item, 0, sizeof(TQUseItem));

	Item.cThreadID = cThreadID;
	Item.cWorldSetID = nWorldSetID;
	Item.nAccountDBID = nAccountDBID;
	Item.Code = (DBDNWorldDef::UseItem::eUseItemCode)cUseItemCode;
	Item.biItemSerial = biItemSerial;
	Item.nUsedItemCount = wUsedItemCount;
	Item.iChannelID = nChannelID;
	Item.iMapID = nMapID;
	_wcscpy( Item.wszIP, _countof(Item.wszIP), pwszIP, IPLENMAX );
	Item.bAllFlag = bAllFlag;
	Item.iTargetLogItemID = iTargetLogItemID;
	Item.cTargetLogItemLevel = cTargetLogItemLevel;

	AddSendData(MAINCMD_ITEM, QUERY_USEITEM, (char*)&Item, sizeof(TQUseItem));
}

void CDNDBConnection::QueryResellItem(CDNUserSession *pSession, INT64 biItemSerial, short wItemCount, INT64 biResellPrice, bool bAllFlag, bool bRebuyFlag, bool bBelongFlag)
{
	TQResellItem Item;
	memset(&Item, 0, sizeof(TQResellItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.biItemSerial = biItemSerial;
	Item.nItemCount = wItemCount;
	Item.biResellPrice = biResellPrice;
	Item.iNpcID = pSession->m_nClickedNpcID;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapID = pSession->GetMapIndex();
	_wcscpy( Item.wszIP, _countof(Item.wszIP), pSession->GetIpW(), IPLENMAX );
	Item.bAllFlag = bAllFlag;

	Item.bRebuyFlag = bRebuyFlag;
#ifdef PRE_MOD_SELL_SEALEDITEM
	Item.bBelongFlag = bBelongFlag;
#endif		//#ifdef PRE_MOD_SELL_SEALEDITEM

	AddSendData(MAINCMD_ITEM, QUERY_RESELLITEM, (char*)&Item, sizeof(TQResellItem));
}

void CDNDBConnection::QueryEnchantItem(CDNUserSession *pSession, INT64 biItemSerial, INT64 biCoin, bool bEnchantSuccessFlag, bool bItemDestroyFlag, BYTE cCurrentItemLevel, int nCurrentItemPotential, 
									   TItem *pNewItem, int nSupportItemID, std::vector<TItemInfo> &VecItemList, INT64 biExtractItemSN)
{
	TQEnchantItem Item;
	memset(&Item, 0, sizeof(TQEnchantItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biItemSerial = biItemSerial;
	Item.biCoin = biCoin;
	Item.bEnchantSuccessFlag = bEnchantSuccessFlag;
	Item.bItemDestroyFlag = bItemDestroyFlag;
	Item.cCurrentItemLevel = cCurrentItemLevel;
	Item.cCurrentItemPotential = nCurrentItemPotential;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapID = pSession->GetMapIndex();
	Item.iNpcID = pSession->m_nClickedNpcID;
	//Optional
	if(pNewItem) Item.NewItem = *pNewItem;
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.nSupportItemID = nSupportItemID;
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	Item.biExtractItemSerial = biExtractItemSN;
#endif

	if(!VecItemList.empty()){
		Item.cMaterialCount = (BYTE)VecItemList.size();
		for (int i = 0; i <(int)VecItemList.size(); i++){
			Item.MaterialArray[i].wCount = VecItemList[i].Item.wCount;
			Item.MaterialArray[i].biSerial = VecItemList[i].Item.nSerial;
		}
	}

	int nLen = sizeof(TQEnchantItem) - sizeof(Item.MaterialArray) +(sizeof(DBPacket::TItemCountInfo) * Item.cMaterialCount);

	AddSendData(MAINCMD_ITEM, QUERY_ENCHANTITEM, (char*)&Item, nLen);
}

void CDNDBConnection::QueryChangeItemLocation(CDNUserSession *pSession, INT64 biBeforeItemSerial, INT64 biBeforeNewItemSerial, int nBeforeItemID, char cAfterItemLocationCode, BYTE cAfterItemSlotIndex, short wBeforeItemCount, 
											  bool bMerge, INT64 biMergeTargetItemSerial, bool bMergeAll, bool bVehicleParts/* = false*/, INT64 biVehicleItemSerial/* = 0*/, char cVehicleSlotIndex/* = 0*/, int nGoldFee)
{
	TQChangeItemLocation Item;
	memset(&Item, 0, sizeof(TQChangeItemLocation));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.biItemSerial = biBeforeItemSerial;
	Item.biNewItemSerial = (biBeforeItemSerial == biBeforeNewItemSerial) ? 0 : biBeforeNewItemSerial;
	Item.iItemID = nBeforeItemID;
	Item.Code = (DBDNWorldDef::ItemLocation::eCode)cAfterItemLocationCode;
	Item.cItemLocationIndex = cAfterItemSlotIndex;
	Item.nItemCount = wBeforeItemCount;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapID = pSession->GetMapIndex();
	Item.bMerge = bMerge;
	Item.biMergeTargetItemSerial = biMergeTargetItemSerial;
	Item.bMergeAll = bMergeAll;
	Item.bVehicleParts = bVehicleParts;
	Item.biVehicleSerial = biVehicleItemSerial;
	Item.cVehicleSlotIndex = cVehicleSlotIndex;
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	Item.nGoldFee = nGoldFee;
#endif
	AddSendData(MAINCMD_ITEM, QUERY_CHANGEITEMLOCATION, (char*)&Item, sizeof(TQChangeItemLocation));
}

void CDNDBConnection::QuerySwitchItemLocation(CDNUserSession *pSession, char cSrcCode, BYTE cSrcSlotIndex, INT64 biDestSerial, char cDestCode, BYTE cDestSlotIndex, INT64 biSrcSerial)
{
	TQSwitchItemLocation Item;
	memset(&Item, 0, sizeof(TQSwitchItemLocation));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.SrcCode = (DBDNWorldDef::ItemLocation::eCode)cSrcCode;
	Item.cSrcSlotIndex = cSrcSlotIndex;
	Item.biDestItemSerial = biDestSerial;
	Item.DestCode = (DBDNWorldDef::ItemLocation::eCode)cDestCode;
	Item.cDestSlotIndex = cDestSlotIndex;
	Item.biSrcItemSerial = biSrcSerial;

	AddSendData(MAINCMD_ITEM, QUERY_SWITCHITEMLOCATION, (char*)&Item, sizeof(TQSwitchItemLocation));
}

void CDNDBConnection::QueryModItemDurability(CDNUserSession *pSession, INT64 biTax, std::vector<INT64> &VecSerialList, std::vector<USHORT> &VecDurList, INT64 biCurrentCoin/*=0*/, INT64 biPickUpCoin/*=0*/)
{
	TQModItemDurability Item;
	memset(&Item, 0, sizeof(TQModItemDurability));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.biTax = biTax;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapID = pSession->GetMapIndex();
	Item.biCurrentCoin = biCurrentCoin;
	Item.biPickUpCoin = biPickUpCoin;

	int nCount = 0;
	if(!VecSerialList.empty()){
		for (int i = 0; i <(int)VecSerialList.size(); i++){
			Item.ItemDurArr[i].biSerial = VecSerialList[i];
			nCount++;
		}
	}
	if(!VecDurList.empty()){
		for (int i = 0; i <(int)VecDurList.size(); i++){
			Item.ItemDurArr[i].wDur = VecDurList[i];
		}
	}

	Item.nCount = nCount;
	int nLen = sizeof(TQModItemDurability) - sizeof(Item.ItemDurArr) +(sizeof(DBPacket::TDurabilityInfo) * nCount);

	AddSendData(MAINCMD_ITEM, QUERY_MODITEMDURABILITY, (char*)&Item, nLen);
}

void CDNDBConnection::QueryModMaterializedItem(CDNUserSession *pSession, TItem &ModItem, int nModFieldBitmap/* = ModItemAll*/, int nEnchantJewelID/*=0*/)
{
	TQModMaterializedItem Item;
	memset(&Item, 0, sizeof(TQModMaterializedItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biItemSerial = ModItem.nSerial;
	Item.iChannelID = pSession->GetChannelID();
	Item.iMapID = pSession->GetMapIndex();
	Item.iModFieldBitmap = nModFieldBitmap;
	Item.nItemDurability = ModItem.wDur;
	Item.cItemLevel = ModItem.cLevel;
	Item.cItemPotential = ModItem.cPotential;
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	Item.cItemPotentialMoveCount = ModItem.cPotentialMoveCount;
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	Item.bSoudBoundFlag = ModItem.bSoulbound;
	Item.cSealCount = ModItem.cSealCount;
	Item.cItemOption = ModItem.cOption;
	_wcscpy( Item.wszIP, _countof(Item.wszIP), pSession->GetIpW(), IPLENMAX );
	Item.nEnchantJewelID = nEnchantJewelID;
	//Item.cDragonJewelType = ModItem.cDragonJewelType;

	AddSendData(MAINCMD_ITEM, QUERY_MODITEM, (char*)&Item, sizeof(TQModMaterializedItem));
}

void CDNDBConnection::QueryExchangeProperty(CDNUserSession *pSession, DBPacket::TExchangeItem *ExchangeItemArray, INT64 biExchangeCoin, int nTax, INT64 biTargetCharacterDBID,
											DBPacket::TExchangeItem *TargetExchangeItemArray, INT64 biTargetExchangeCoin, int nTargetTax, UINT uiTargetAccountDBID )
{
	TQExchangeProperty Item;
	memset(&Item, 0, sizeof(TQExchangeProperty));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	memcpy(Item.ExchangeItem, ExchangeItemArray, sizeof(Item.ExchangeItem));
	Item.biExchangeCoin = biExchangeCoin;
	Item.nExchangeTax = nTax;
	Item.biTargetCharacterDBID = biTargetCharacterDBID;
	memcpy(Item.TargetExchangeItem, TargetExchangeItemArray, sizeof(Item.TargetExchangeItem));
	Item.biTargetExchangeCoin = biTargetExchangeCoin;
	Item.nTargetExchangeTax = nTargetTax;
	Item.nChannelID = pSession->GetChannelID();
	Item.nMapID = pSession->GetMapIndex();
	Item.uiTargetAccountDBID = uiTargetAccountDBID;

	AddSendData(MAINCMD_ITEM, QUERY_EXCHANGEPROPERTY, (char*)&Item, sizeof(TQExchangeProperty));
}

void CDNDBConnection::QueryUpsEquip(CDNUserSession *pSession, int nItemLocation, int nSlotIndex, INT64 biSerial)
{
	TQUpsEquip Update;
	memset(&Update, 0, sizeof(TQUpsEquip));

	Update.cThreadID = pSession->GetDBThreadID();
	Update.cWorldSetID = pSession->GetWorldSetID();
	Update.nAccountDBID = pSession->GetAccountDBID();
	Update.biCharacterDBID = pSession->GetCharacterDBID();
	Update.cItemLocation = nItemLocation;
	Update.cEquipSlotIndex = nSlotIndex;
	Update.biSerial = biSerial;

	AddSendData(MAINCMD_ITEM, QUERY_UPSEQUIP, (char*)&Update, sizeof(TQUpsEquip));
}

void CDNDBConnection::QueryTakeCashEquipmentOff(CDNUserSession *pSession, int nTotalCount, int *nCashEquipIndexList)
{
	TQTakeCashEquipmentOff Item;
	memset(&Item, 0, sizeof(TQTakeCashEquipmentOff));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	if(nCashEquipIndexList){
		Item.cCashEquipTotalCount = nTotalCount;
		if(Item.cCashEquipTotalCount > CASHEQUIPMAX)
			Item.cCashEquipTotalCount = CASHEQUIPMAX;

		for (int i = 0; i < Item.cCashEquipTotalCount; i++){
			Item.cCashEquipIndexArray[i] = nCashEquipIndexList[i];
		}
	}

	int nLen = sizeof(TQTakeCashEquipmentOff) - sizeof(Item.cCashEquipIndexArray) +(sizeof(char) * Item.cCashEquipTotalCount);
	AddSendData(MAINCMD_ITEM, QUERY_TAKECASHEQUIPMENTOFF, (char*)&Item, nLen);
}

void CDNDBConnection::QueryPutOnepieceOn(CDNUserSession *pSession, INT64 biOnepieceItemSerial, char cOnepieceEquipIndex, int nTotalCount, int *nSubCashEquipIndexList)
{
	TQPutOnepieceOn Item;
	memset(&Item, 0, sizeof(TQPutOnepieceOn));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.biOnepieceItemSerial = biOnepieceItemSerial;
	Item.cOnepieceCashEquipIndex = cOnepieceEquipIndex;
	if(nSubCashEquipIndexList){
		Item.cCashEquipTotalCount = nTotalCount;
		if(Item.cCashEquipTotalCount > CASHEQUIPMAX)
			Item.cCashEquipTotalCount = CASHEQUIPMAX;

		for (int i = 0; i < Item.cCashEquipTotalCount; i++){
			Item.cSubCashEquipIndexArray[i] = nSubCashEquipIndexList[i];
		}
	}

	int nLen = sizeof(TQPutOnepieceOn) - sizeof(Item.cSubCashEquipIndexArray) +(sizeof(char) * Item.cCashEquipTotalCount);
	AddSendData(MAINCMD_ITEM, QUERY_PUTONEPIECEON, (char*)&Item, nLen);
}

void CDNDBConnection::QueryGetPageMaterializedItem(BYTE cThreadID, CDNUserSession *pSession, int nPageNumber, int nPageSize)
{
	TQGetPageMaterializedItem Item;
	memset(&Item, 0, sizeof(TQGetPageMaterializedItem));

	Item.cThreadID = cThreadID;
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.nPageNumber = nPageNumber;
	Item.nPageSize = nPageSize;

	AddSendData(MAINCMD_ITEM, QUERY_GETPAGEMATERIALIZEDITEM, (char*)&Item, sizeof(TQGetPageMaterializedItem));
}

void CDNDBConnection::QueryGetPageVehicle(CDNUserSession *pSession, int nPageNumber, int nPageSize)
{
	TQGetPageVehicle Item;
	memset(&Item, 0, sizeof(TQGetPageVehicle));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.nPageNumber = nPageNumber;
	Item.nPageSize = nPageSize;

	AddSendData(MAINCMD_ITEM, QUERY_GETPAGEVEHICLE, (char*)&Item, sizeof(TQGetPageVehicle));
}

void CDNDBConnection::QueryModPetSkinColor(CDNUserSession *pSession, INT64 biBodySerial, DWORD dwColor, DWORD dwColor2)
{
	TQModPetSkinColor Item;
	memset(&Item, 0, sizeof(TQModPetSkinColor));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biBodySerial = biBodySerial;
	Item.dwSkinColor = dwColor;
	Item.dwSkinColor2 = dwColor2;

	AddSendData(MAINCMD_ITEM, QUERY_MODPETSKINCOLOR, (char*)&Item, sizeof(TQModPetSkinColor));
}

void CDNDBConnection::QueryDelPetEquipment(CDNUserSession *pSession, INT64 biBodySerial, INT64 biPartsSerial)
{
	TQDelPetEquipment Item;
	memset(&Item, 0, sizeof(TQDelPetEquipment));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biBodySerial = biBodySerial;
	Item.biPartsSerial = biPartsSerial;
	_wcscpy(Item.wszIp, _countof(Item.wszIp), pSession->GetIpW(), IPLENMAX);

	AddSendData(MAINCMD_ITEM, QUERY_DELPETEQUIPMENT, (char*)&Item, sizeof(TQDelPetEquipment));
}

void CDNDBConnection::QueryRecoverMissingItem(CDNUserSession *pSession, char cItemLocationCode, BYTE cSlotIndex, TItem &RecoverItem)
{
	TQRecoverMissingItem Item;
	memset(&Item, 0, sizeof(TQRecoverMissingItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.cItemLocationCode = cItemLocationCode;
	Item.cSlotIndex = cSlotIndex;
	Item.RecoverItem = RecoverItem;

	AddSendData(MAINCMD_ITEM, QUERY_RECOVERMISSINGITEM, (char*)&Item, sizeof(TQRecoverMissingItem));
}

void CDNDBConnection::QueryAddEffectItems(CDNUserSession *pSession, int iItemID, char cLifeSpanRenewal, DBDNWorldDef::EffectItemGetCode::eCode Code, INT64 biFkey, 
										  int iPrice, bool bEnternityFlag, int iLifeSpan, INT64 biItemSerial, int iRemoveItemID)
{
	TQAddEffectItems Item;
	memset(&Item, 0, sizeof(TQAddEffectItems));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.iItemID = iItemID;
	Item.cItemLifeSpanRenewal = cLifeSpanRenewal;
	Item.Code = Code;
	Item.biFKey = biFkey;
	Item.iPrice;
	Item.bEternityFlag = bEnternityFlag;
	Item.iItemLifeSpan = iLifeSpan;
	Item.biItemSerial = biItemSerial;
	Item.iRemoveItemID = iRemoveItemID;

	AddSendData(MAINCMD_ITEM, QUERY_ADDEFFECTITEMS, (char*)&Item, sizeof(TQAddEffectItems));
}

void CDNDBConnection::QueryModAdditiveItem(CDNUserSession *pSession, INT64 biItemSerial, int iAdditiveItemID, INT64 biAdditiveSerial, INT64 biMixItemSerial)
{
	TQModAdditiveItem Item;
	memset(&Item, 0, sizeof(TQModAdditiveItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	Item.nAdditiveItemID = iAdditiveItemID;

	Item.biItemSerial = biItemSerial;
	Item.biAdditiveSerial = biAdditiveSerial;
	Item.biMixItemSerial = biMixItemSerial;
	Item.nMapID = pSession->GetMapIndex();
	_wcscpy(Item.wszIP, _countof(Item.wszIP), pSession->GetIpW(), (int)wcslen(pSession->GetIpW()));

	AddSendData(MAINCMD_ITEM, QUERY_MODADDITIVEITEM, (char*)&Item, sizeof(TQModAdditiveItem));
}

#if defined (PRE_ADD_COSRANDMIX)
void CDNDBConnection::QueryModRandomItem(CDNUserSession *pSession, INT64 biStuffSerialA, INT64 biStuffSerialB, INT64 biMixItemSerial, short wMixItemCount, TItem &RandomItem, INT64 biFee)
{
	TQModRandomItem Item;
	memset(&Item, 0, sizeof(TQModRandomItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();

	Item.biStuffSerialA = biStuffSerialA;
	Item.biStuffSerialB = biStuffSerialB;
	Item.biMixItemSerial = biMixItemSerial;
	Item.wMixItemCount = wMixItemCount;
	Item.biFee = biFee;
	Item.Code = DBDNWorldDef::AddMaterializedItem::CostumeRandomMixItem;

	Item.RandomItem = RandomItem;
	Item.cItemLocationCode = DBDNWorldDef::ItemLocation::CashInventory;

	Item.nChannelID = pSession->GetChannelID();
	Item.nMapIndex = pSession->GetMapIndex();
	_wcscpy( Item.wszIP, _countof(Item.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData(MAINCMD_ITEM, QUERY_MODRANDOMITEM, (char*)&Item, sizeof(TQModRandomItem));
}
#endif

void CDNDBConnection::QuerySaveItemLocationIndex(CDNUserSession *pSession)
{
	TQSaveItemLocationIndex Item;
	memset(&Item, 0, sizeof(TQSaveItemLocationIndex));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	Item.biCharacterDBID = pSession->GetCharacterDBID();
	pSession->GetItem()->GetIndexSerialList(Item.nTotalInvenWareCount, Item.SaveList);

	AddSendData(MAINCMD_ITEM, QUERY_SAVEITEMLOCATIONINDEX, (char*)&Item, sizeof(TQSaveItemLocationIndex) - sizeof(Item.SaveList) +(sizeof(DBPacket::TItemIndexSerial) * Item.nTotalInvenWareCount));
}

void CDNDBConnection::QueryGetListRepurchaseItem( CDNUserSession *pSession )
{
	TQGetListRepurchaseItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_ITEM, QUERY_GETLIST_REPURCHASEITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDelExpireitem( BYTE cThreadID, int nWorldSetID, UINT uiAccountDBID, INT64 biCharacterDBID )
{
	TQDelExpiritem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = nWorldSetID;
	TxPacket.nAccountDBID = uiAccountDBID;
	TxPacket.biCharacterDBID = biCharacterDBID;

	AddSendData( MAINCMD_ITEM, QUERY_DELEXPIREITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDelCashItem(CDNUserSession *pSession, const TItem *pDelItem)
{
	TQDelCashItem Item;
	memset(&Item, 0, sizeof(TQDelCashItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	if (pDelItem){
		Item.biItemSerial = pDelItem->nSerial;
		Item.wCount = pDelItem->wCount;

		//if (pDelItem->nLifespan > REMOVECASHWAITINGTIME)
		if (pDelItem->nLifespan > REMOVECASHWAITINGTIME || pDelItem->bEternity == true ) // #55004 [Cash Delete] Attempting to delete certain items shows 0 days remaining until deletion
			Item.nWatingTime = REMOVECASHWAITINGTIME;
		else
			Item.nWatingTime = pDelItem->nLifespan;
	}

	AddSendData(MAINCMD_ITEM, QUERY_DELCASHITEM, reinterpret_cast<char*>(&Item), sizeof(TQDelCashItem) );
}

void CDNDBConnection::QueryRecoverCashItem(CDNUserSession *pSession, const TItem *pRecoverItem)
{
	TQRecoverCashItem Item;
	memset(&Item, 0, sizeof(TQRecoverCashItem));

	Item.cThreadID = pSession->GetDBThreadID();
	Item.cWorldSetID = pSession->GetWorldSetID();
	Item.nAccountDBID = pSession->GetAccountDBID();
	if (pRecoverItem){
		Item.biItemSerial = pRecoverItem->nSerial;
		Item.wCount = pRecoverItem->wCount;
	}

	AddSendData(MAINCMD_ITEM, QUERY_RECOVERCASHITEM, reinterpret_cast<char*>(&Item), sizeof(TQRecoverCashItem) );
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNDBConnection::QueryCheckNamedItemCount(CDNUserSession *pSession, int nItemID, BYTE cIndex)
{
	TQCheckNamedItemCount NamedItemCount;
	memset(&NamedItemCount, 0, sizeof(TQCheckNamedItemCount));

	NamedItemCount.cThreadID = pSession->GetDBThreadID();
	NamedItemCount.cWorldSetID = pSession->GetWorldSetID();
	NamedItemCount.nAccountDBID = pSession->GetAccountDBID();
	NamedItemCount.nItemID = nItemID;	
	NamedItemCount.cIndex = cIndex;

	AddSendData(MAINCMD_ITEM, QUERY_CHECK_NAMEDITEMCOUNT, reinterpret_cast<char*>(&NamedItemCount), sizeof(TQCheckNamedItemCount) );
}
void CDNDBConnection::QueryCheatCheckNamedItemCount(CDNUserSession *pSession, int nItemID, BYTE cIndex)
{
	TQCheckNamedItemCount NamedItemCount;
	memset(&NamedItemCount, 0, sizeof(TQCheckNamedItemCount));

	NamedItemCount.cThreadID = pSession->GetDBThreadID();
	NamedItemCount.cWorldSetID = pSession->GetWorldSetID();
	NamedItemCount.nAccountDBID = pSession->GetAccountDBID();
	NamedItemCount.nItemID = nItemID;	
	NamedItemCount.cIndex = cIndex;

	AddSendData(MAINCMD_ITEM, QUERY_CHEATCHECK_NAMEDITEMCOUNT, reinterpret_cast<char*>(&NamedItemCount), sizeof(TQCheckNamedItemCount) );
}
#endif

#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
void CDNDBConnection::QueryMovePotential(CDNUserSession * pSession, INT64 biExtractSerial, char cExtractPotential, INT64 biInjectSerial, char cInjectPotential, char cMoveableLimitCount, int nMapID)
{
	if (pSession == NULL) return;

	TQMovePotential packet;
	memset(&packet, 0, sizeof(TQMovePotential));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.biExtractSerial = biExtractSerial;
	packet.cExtractPotential = cExtractPotential;
	packet.biInjectSerial = biInjectSerial;
	packet.cInjectPotential = cInjectPotential;
	packet.cMoveableLimitCount = cMoveableLimitCount;
	packet.nMapID = nMapID;
	
	AddSendData(MAINCMD_ITEM, QUERY_MOVEPOTENTIAL, reinterpret_cast<char*>(&packet), sizeof(TQMovePotential));
}
#endif		//#if defined (PRE_ADD_EXCHANGE_POTENTIAL)

#if defined( PRE_ADD_LIMITED_SHOP )
void CDNDBConnection::QueryGetLimitedShopItem(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetLimitedShopItem packet;
	memset(&packet, 0, sizeof(TQGetLimitedShopItem));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();	

	AddSendData(MAINCMD_ITEM, QUERY_GETLIMITEDSHOPITEM, reinterpret_cast<char*>(&packet), sizeof(TQGetLimitedShopItem));
}
void CDNDBConnection::QueryAddLimitedShopItem(CDNUserSession * pSession, LimitedShop::LimitedItemData ItemData, int nBuyCount)
{
	TQAddLimitedShopItem packet;
	memset(&packet, 0, sizeof(TQAddLimitedShopItem));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.ItemData = ItemData;	
	packet.nBuyCount = nBuyCount;

	AddSendData(MAINCMD_ITEM, QUERY_ADDLIMITEDSHOPITEM, reinterpret_cast<char*>(&packet), sizeof(TQAddLimitedShopItem));
}
void CDNDBConnection::QueryResetLimitedShopItem(CDNUserSession * pSession, int nResetCycle)
{
	TQResetLimitedShopItem packet;
	memset(&packet, 0, sizeof(TQResetLimitedShopItem));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nResetCycle = nResetCycle;	

	AddSendData(MAINCMD_ITEM, QUERY_RESETLIMITEDSHOPITEM, reinterpret_cast<char*>(&packet), sizeof(TQResetLimitedShopItem));
}
#endif		//#if defined( PRE_ADD_LIMITED_SHOP )


void CDNDBConnection::QueryDelEffectItem( CDNUserSession * pSession, INT64 nItemSerial )
{
	TQDelEffectItem packet;
	memset(&packet, 0, sizeof(TQDelEffectItem));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nItemSerial = nItemSerial;

	AddSendData(MAINCMD_ITEM, QUERY_DEL_EFFECTITEM, reinterpret_cast<char*>(&packet), sizeof(TQDelEffectItem));
}


//#########################################################################################################################################
// MAINCMD_QUEST
//#########################################################################################################################################

void CDNDBConnection::QueryAddQuest(CDNUserSession *pSession, char cIdx, TQuest &Quest)
{
	TQAddQuest packet;
	memset(&packet, 0, sizeof(TQAddQuest));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cIdx = cIdx;
	packet.Quest = Quest;

	AddSendData(MAINCMD_QUEST, QUERY_ADDQUEST, (char*)&packet, sizeof(TQAddQuest));
}

void CDNDBConnection::QueryDelQuest(CDNUserSession *pSession, char cIdx, int nQuestID, bool bComplete, bool bRepeat, char *CompleteQuestArray)
{
	TQDelQuest packet;
	memset(&packet, 0, sizeof(TQDelQuest));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cIdx = cIdx;
	packet.nQuestID = nQuestID;
	packet.bComplete = bComplete;
	packet.bRepeat = bRepeat;
	memcpy(packet.CompleteQuest, CompleteQuestArray, COMPLETEQUEST_BITSIZE);

	AddSendData(MAINCMD_QUEST, QUERY_DELQUEST, (char*)&packet, sizeof(TQDelQuest));
}

void CDNDBConnection::QueryModQuest(CDNUserSession *pSession, char cIdx, TQuest &Quest, char cQuestStatusCode)
{
	TQModQuest packet;
	memset( &packet, 0, sizeof(packet) );

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cIndex = cIdx;
	packet.Quest = Quest;
	packet.cQuestStatusCode = cQuestStatusCode;

	AddSendData(MAINCMD_QUEST, QUERY_MODQUEST, (char*)&packet, sizeof(TQModQuest));
}

void CDNDBConnection::QueryClearQuest(CDNUserSession *pSession)
{
	TQClearQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_QUEST, QUERY_CLEARQUEST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryForceCompleteQuest(CDNUserSession *pSession, int nQuestID, int nQuestCode, char *pCompleteQuestArray)
{
	TQForceCompleteQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.Set(pSession->GetDBThreadID(), pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), nQuestID, nQuestCode, pCompleteQuestArray);

	AddSendData(MAINCMD_QUEST, QUERY_FORCECOMPLETEQUEST, (char*)&packet, packet.GetSize());
}

void CDNDBConnection::QueryGetListCompleteEventQuest(CDNUserSession *pSession)
{
	TQGetListCompleteEventQuest packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_QUEST, QUERY_GET_LISTCOMPLETE_EVENTQUEST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryCompleteEventQuest(CDNUserSession *pSession, int nScheduleID, __time64_t tExpireDate)
{
	TQCompleteEventQuest  packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	packet.nScheduleID = nScheduleID;
	packet.tExpireDate = tExpireDate;

	AddSendData(MAINCMD_QUEST, QUERY_COMPLETE_EVENTQUEST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetWorldEventQuestCounter(BYTE cThreadID, int nWorldSetID, int nScheduleID)
{
	TQGetWorldEventQuestCounter packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;
	packet.nScheduleID = nScheduleID;

	AddSendData(MAINCMD_QUEST, QUERY_GET_WORLDEVENTQUESTCOUNTER, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryModWorldEventQuestCounter(CDNUserSession *pSession, int nScheduleID, int nCounterDelta)
{
	TQModWorldEventQuestCounter packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nScheduleID = nScheduleID;
	packet.nCounterDelta = nCounterDelta;
	
	AddSendData(MAINCMD_QUEST, QUERY_MOD_WORLDEVENTQUESTCOUNTER, (char*)&packet, sizeof(packet));
}

//#########################################################################################################################################
// MAINCMD_FRIEND
//#########################################################################################################################################

void CDNDBConnection::QueryFriendList(BYTE cThreadID, CDNUserSession *pSession)
{
	TQFriendList packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_FRIEND, QUERY_FRIENDLIST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGroup(CDNUserSession *pSession, WCHAR *pGroupName)
{
	TQAddGroup Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	if(pGroupName) _wcscpy(Friend.wszGroupName, FRIEND_GROUP_NAMELENMAX, pGroupName, (int)wcslen(pGroupName));
	Friend.nGroupCountLimit = FRIEND_GROUP_MAX;

	AddSendData(MAINCMD_FRIEND, QUERY_ADDGROUP, (char*)&Friend, sizeof(Friend));
}

void CDNDBConnection::QueryModGroupName(CDNUserSession *pSession, int nGroupDBID, WCHAR *pGroupName)
{
	TQModGroupName Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	Friend.nGroupDBID = nGroupDBID;
	if(pGroupName) _wcscpy(Friend.wszGroupName, FRIEND_GROUP_NAMELENMAX, pGroupName, (int)wcslen(pGroupName));

	AddSendData(MAINCMD_FRIEND, QUERY_MODGROUPNAME, (char*)&Friend, sizeof(Friend));
}

void CDNDBConnection::QueryDelGroup(CDNUserSession *pSession, int nGroupDBID)
{
	TQDelGroup Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	Friend.nGroupDBID = nGroupDBID;

	AddSendData(MAINCMD_FRIEND, QUERY_DELGROUP, (char*)&Friend, sizeof(Friend));
}

void CDNDBConnection::QueryModFriendAndGroupMapping(CDNUserSession *pSession, int nGroupDBID, char cCount, INT64 *biFriendCharacterDBIDs)
{
	TQModFriendAndGroupMapping Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	Friend.nGroupDBID = nGroupDBID;

	/*int nCount = 0;
	for (int i = 0; i < FRIEND_MAXCOUNT; i++){
		if(biFriendCharacterDBIDs[i] <= 0) continue;
		Friend.biFriendCharacterDBIDs[i] = biFriendCharacterDBIDs[i];
		nCount++;
	}
	Friend.cCount = nCount;*/
	//Enable parent handling when needed.
	Friend.biFriendCharacterDBIDs[0] = *biFriendCharacterDBIDs;
	Friend.cCount = 1;

	AddSendData(MAINCMD_FRIEND, QUERY_MODFRIENDANDGROUPMAPPING, (char*)&Friend, sizeof(Friend) - sizeof(Friend.biFriendCharacterDBIDs) +(sizeof(INT64) * Friend.cCount));
}

void CDNDBConnection::QueryAddFriend(CDNUserSession *pSession, WCHAR *pFriendName, int nGroupDBID)
{
	TQAddFriend Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	if(pFriendName) _wcscpy(Friend.wszFriendName, NAMELENMAX, pFriendName, (int)wcslen(pFriendName));
	Friend.nFriendCountLimit = FRIEND_MAXCOUNT;
	Friend.nGroupDBID = nGroupDBID;

	AddSendData(MAINCMD_FRIEND, QUERY_ADDFRIEND, (char*)&Friend, sizeof(Friend));
}

void CDNDBConnection::QueryModFriendMemo(CDNUserSession *pSession, INT64 biFriendCharacterDBID, WCHAR *pMemo)
{
	TQModFriendMemo Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();
	Friend.biFriendCharacterDBID = biFriendCharacterDBID;
	if(pMemo) _wcscpy(Friend.wszMemo, FRIEND_MEMO_LEN_MAX, pMemo, (int)wcslen(pMemo));

	AddSendData(MAINCMD_FRIEND, QUERY_MODFRIENDMEMO, (char*)&Friend, sizeof(Friend));
}

void CDNDBConnection::QueryDelFriend(CDNUserSession *pSession, char cCount, INT64 *biFriendCharacterDBIDs)
{
	TQDelFriend Friend;
	memset( &Friend, 0, sizeof(Friend) );

	Friend.cThreadID = pSession->GetDBThreadID();
	Friend.cWorldSetID = pSession->GetWorldSetID();
	Friend.nAccountDBID = pSession->GetAccountDBID();
	Friend.biCharacterDBID = pSession->GetCharacterDBID();

	/*int nCount = 0;
	for (int i = 0; i < cCount; i++){
		if(biFriendCharacterDBIDs[i] <= 0) continue;
		Friend.biFriendCharacterDBIDs[i] = biFriendCharacterDBIDs[i];
		nCount++;
	}
	Friend.cCount = nCount;*/
	//Uncomment above once multi-delete UI is added
	Friend.biFriendCharacterDBIDs[0] = *biFriendCharacterDBIDs;
	Friend.cCount = 1;

	AddSendData(MAINCMD_FRIEND, QUERY_DELFRIEND, (char*)&Friend, sizeof(Friend) - sizeof(Friend.biFriendCharacterDBIDs) +(sizeof(INT64) * Friend.cCount));
}


//#########################################################################################################################################
// MAINCMD_GUILD
//#########################################################################################################################################

void CDNDBConnection::QueryDismissGuild(CDNUserSession *pSession)
{
	TQDismissGuild Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nCharacterDBID = pSession->GetCharacterDBID();
	Packet.nGuildDBID = pSession->GetGuildUID().nDBID;

	AddSendData(MAINCMD_GUILD, QUERY_DISMISSGUILD, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetGuildHistoryList(CDNUserSession *pSession, INT64 biIndex, bool bDirection)
{
	TQGetGuildHistoryList Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nCharacterDBID = pSession->GetCharacterDBID();
	Packet.nGuildDBID = pSession->GetGuildUID().nDBID;
	Packet.biIndex = biIndex;
	Packet.bDirection = bDirection;

	AddSendData(MAINCMD_GUILD, QUERY_GETGUILDHISTORYLIST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryCreateGuild(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, LPCWSTR wszGuildName, int nMapIndex, char cLevel, int nEstablishExpense, char cWorldSetID, int wGuildRoleAuth[GUILDROLE_TYPE_CNT])
{
	TQCreateGuild Packet;
	memset(&Packet, 0, sizeof(TQCreateGuild));

	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nReqAccountDBID;
	Packet.cWorldSetID = cWorldSetID;

	Packet.nCharacterDBID = nReqCharacterDBID;
	_wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), wszGuildName, (int)wcslen(wszGuildName));
	memcpy(Packet.wGuildRoleAuth, wGuildRoleAuth, sizeof(Packet.wGuildRoleAuth));
	Packet.nMapID = nMapIndex;
	Packet.nEstablishExpense = nEstablishExpense;
	Packet.cLevel = cLevel;

	AddSendData(MAINCMD_GUILD, QUERY_CREATEGUILD, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryDelGuildMember(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nDelAccountDBID, INT64 nDelCharacterDBID, UINT nGuildDBID, char cLevel, char cWorldSetID, bool bIsExiled, bool bGraduateBeginnerGuild)
{
	TQDelGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nReqAccountDBID = nReqAccountDBID;
	Packet.nReqCharacterDBID = nReqCharacterDBID;
	Packet.nDelAccountDBID = nDelAccountDBID;
	Packet.nDelCharacterDBID = nDelCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.bIsExiled = bIsExiled;
	Packet.cLevel = cLevel;
#ifdef PRE_ADD_BEGINNERGUILD
	Packet.bIsGraduateBeginnerGuild = bGraduateBeginnerGuild;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

	AddSendData(MAINCMD_GUILD, QUERY_DELGUILDMEMBER, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddGuildMember(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cLevel, char cWorldSetID, short wGuildSize)
{
	TQAddGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cLevel = cLevel;
	Packet.wGuildSize = wGuildSize;

	AddSendData(MAINCMD_GUILD, QUERY_ADDGUILDMEMBER, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGuildInviteMember(BYTE cThreadID, UINT nAccountDBID, UINT nGuildDBID, char cWorldSetID, UINT nToSessionID, bool bNeedMembList)
{
	TQInviteGuildMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.cWorldSetID = cWorldSetID;

	Packet.GuildInfo.cThreadID = cThreadID;
	Packet.GuildInfo.cWorldSetID = cWorldSetID;
	Packet.GuildInfo.nAccountDBID = nAccountDBID;
	Packet.GuildInfo.bNeedMembList = bNeedMembList;
	Packet.GuildInfo.nGuildDBID = nGuildDBID;

	AddSendData(MAINCMD_GUILD, QUERY_INVITEGUILDMEMBER, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryMoveItemInGuildWare(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveItemInGuildWare* pMove)
{
	pMove->nAccountDBID = nAccountDBID;
	pMove->cWorldSetID = cWorldSetID;
	pMove->cThreadID = cThreadID;

	AddSendData(MAINCMD_GUILD, QUERY_MOVE_ITEM_IN_GUILDWARE, (char*)pMove, sizeof(TQMoveItemInGuildWare));
}

void CDNDBConnection::QueryMoveInvenToGuildWare(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveInvenToGuildWare* pMove)
{
	pMove->nAccountDBID = nAccountDBID;
	pMove->cWorldSetID = cWorldSetID;
	pMove->cThreadID = cThreadID;

	AddSendData(MAINCMD_GUILD, QUERY_MOVE_INVEN_TO_GUILDWARE, (char*)pMove, sizeof(TQMoveInvenToGuildWare));
}

void CDNDBConnection::QueryMoveGuildWareToInven(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQMoveGuildWareToInven* pMove)
{
	pMove->nAccountDBID = nAccountDBID;
	pMove->cWorldSetID = cWorldSetID;
	pMove->cThreadID = cThreadID;

	AddSendData(MAINCMD_GUILD, QUERY_MOVE_GUILDWARE_TO_INVEN, (char*)pMove, sizeof(TQMoveGuildWareToInven));
}

void CDNDBConnection::QueryGuildWarehouseCoin(BYTE cThreadID, UINT nAccountDBID, char cWorldSetID, TQGuildWareHouseCoin* pPacket)
{
	pPacket->nAccountDBID = nAccountDBID;
	pPacket->cWorldSetID = cWorldSetID;
	pPacket->cThreadID = cThreadID;

	AddSendData(MAINCMD_GUILD, QUERY_GUILDWARE_COIN, (char*)pPacket, sizeof(TQMoveGuildWareToInven));
}

void CDNDBConnection::QueryGetGuildWareInfo(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, short wMaxCount)
{
	DN_ASSERT(GUILDDBID_DEF != nGuildDBID,	"Invalid!");
	DN_ASSERT(0 != cWorldSetID,				"Invalid!");

	TQGetGuildWareInfo Packet;
	Packet.cThreadID	= cThreadID;
	Packet.nGuildDBID	= nGuildDBID;
	Packet.cWorldSetID	= cWorldSetID;
	Packet.wMaxCount    = wMaxCount;
	
	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWAREINFO, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QuerySetGuildWareInfo(BYTE cTheadID, UINT nGuildDBID, char cWorldSetID, BYTE cSlotCount, TGuildWareSlot* pWareSlot)
{	
	TQSetGuildWareInfo Packet;
	Packet.cThreadID = cTheadID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.cSlotListCount = cSlotCount;
	memcpy(Packet.WareSlotList, pWareSlot, sizeof(TGuildWareSlot)*cSlotCount);

	AddSendData(MAINCMD_GUILD, QUERY_SET_GUILDWAREINFO, (char*)&Packet, sizeof(Packet)-sizeof(Packet.WareSlotList) +(sizeof(TGuildWareSlot)*cSlotCount));
}

void CDNDBConnection::QueryGetGuildWareHistory(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, int nIndex)
{
	DN_ASSERT(GUILDDBID_DEF != nGuildDBID,	"Invalid!");
	DN_ASSERT(0 != cWorldSetID,				"Invalid!");

	TQGetGuildWareHistory Packet;
	Packet.nAccountDBID		= nAccountDBID;
	Packet.cThreadID		= cThreadID;
	Packet.nCharacterDBID	= nCharacterDBID;
	Packet.nGuildDBID		= nGuildDBID;
	Packet.cWorldSetID		= cWorldSetID;
	Packet.nIndex			= nIndex;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWAREHISTORY, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryChangeGuildInfo(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, BYTE btGuildUpdate, 
											int iInt1, int iInt2, int iInt3, int iInt4, INT64 biInt64, LPCWSTR pText, long nAddCommonPoint)
{	
	TQChangeGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.btGuildUpdate = btGuildUpdate;
	Packet.nAddCommonPoint = nAddCommonPoint;
	Packet.nInt1 = iInt1;
	Packet.nInt2 = iInt2;
	Packet.nInt3 = iInt3;
	Packet.nInt4 = iInt4;
	Packet.biInt64 = biInt64;
	if(pText) {
		_wcscpy(Packet.Text, _countof(Packet.Text), pText, (int)wcslen(pText));
	}

	AddSendData(MAINCMD_GUILD, QUERY_CHANGEGUILDINFO, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryChangeGuildMemberInfo(BYTE cThreadID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, UINT nGuildDBID, char cWorldSetID, BYTE btGuildMemberUpdate, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText, char cLevel)
{
	TQChangeGuildMemberInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.nReqAccountDBID = nReqAccountDBID;
	Packet.nReqCharacterDBID = nReqCharacterDBID;
	Packet.nChgAccountDBID = nChgAccountDBID;
	Packet.nChgCharacterDBID = nChgCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.btGuildMemberUpdate = btGuildMemberUpdate;
	Packet.nInt1 = iInt1;
	Packet.nInt2 = iInt2;
	Packet.biInt64 = biInt64;
	if(pText)
		_wcscpy(Packet.Text, _countof(Packet.Text), pText, (int)wcslen(pText));
	Packet.cLevel = cLevel;

	AddSendData(MAINCMD_GUILD, QUERY_CHANGEGUILDMEMBERINFO, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryChangeGuildWareSize(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, short wSize)
{	
	TQChangeGuildWareSize Packet;
	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.wStorageSize = wSize;
	
	AddSendData(MAINCMD_GUILD, QUERY_CHANGEGUILDWARESIZE, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryChangeGuildMark(BYTE cThreadID, UINT nAccountDBID, INT64 nCharacterDBID, UINT nGuildDBID, char cWorldSetID, short wGuildMark, short wGuildMarkBG, short wGuildMarkBorder)
{	
	TQChangeGuildMark Packet;
	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nCharacterDBID = nCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;

	Packet.wGuildMark = wGuildMark;
	Packet.wGuildMarkBG = wGuildMarkBG;
	Packet.wGuildMarkBorder = wGuildMarkBorder;

	AddSendData(MAINCMD_GUILD, QUERY_CHANGEGUILDMARK, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddGuildHistory(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, BYTE btHistoryType, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText)
{
	TQAddGuildHistory Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;

	Packet.History.biIndex = 0;
	Packet.History.RegDate = 0;
	Packet.History.btHistoryType = btHistoryType;
	Packet.History.nInt1 = iInt1;
	Packet.History.nInt2 = iInt2;
	Packet.History.biInt64 = biInt64;
	if(pText) {
		_wcscpy(Packet.History.Text, _countof(Packet.History.Text), pText, (int)wcslen(pText));
	}

	AddSendData(MAINCMD_GUILD, QUERY_ADDGUILDHISTORY, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetGuildInfo(CDNUserSession *pSession, UINT nGuildDBID, bool bNeedMembList)
{
	TQGetGuildInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nGuildDBID = nGuildDBID;
	Packet.bNeedMembList = bNeedMembList;

	AddSendData(MAINCMD_GUILD, QUERY_GETGUILDINFO, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryChangeGuildName(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID, LPCWSTR pGuildName, INT64 biItemSerial)
{
	TQChangeGuildName Packet;
	memset(&Packet, 0, sizeof(TQChangeGuildName));

	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.biItemSerial = biItemSerial;
	if(pGuildName) _wcscpy(Packet.wszGuildName, _countof(Packet.wszGuildName), pGuildName, (int)wcslen(pGuildName));

	AddSendData(MAINCMD_GUILD, QUERY_CHANGEGUILDNAME, (char*)&Packet, sizeof(TQChangeGuildName));
}
void CDNDBConnection::QueryGetGuildRewardItem(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID)
{
	TQGetGuildRewardItem Packet;
	memset(&Packet, 0, sizeof(TQGetGuildRewardItem));

	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;	
	Packet.nGuildDBID = nGuildDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDREWARDITEM, (char*)&Packet, sizeof(TQGetGuildRewardItem));
}
void CDNDBConnection::QueryAddGuildRewardItem(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID, int nItemID, bool bEnternityFlag, int nLifeSpan, BYTE cItemBuyType, int nNeedGold, int nMapID, int nRemoveItemID)
{
	TQAddGuildRewardItem Packet;
	memset(&Packet, 0, sizeof(TQAddGuildRewardItem));

	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = nAccountDBID;	
	Packet.cWorldSetID = cWorldSetID;	
	Packet.biCharacterDBID = biCharacterDBID;
	Packet.nGuildDBID = nGuildDBID;
	Packet.nItemID = nItemID;
	Packet.bEnternityFlag = bEnternityFlag;
	Packet.nLifeSpan = nLifeSpan;
	Packet.cItemBuyType = cItemBuyType;
	Packet.nNeedGold = nNeedGold;
	Packet.nMpaID = nMapID;
	Packet.nRemoveItemID = nRemoveItemID;
	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDREWARDITEM, (char*)&Packet, sizeof(TQAddGuildRewardItem));
}
void CDNDBConnection::QueryChangeGuildSize(BYTE cThreadID, UINT nAccountDBID, UINT nGuildDBID, char cWorldSetID, int nGuildSize)
{
	TQChangeGuildSize Packet;
	memset(&Packet, 0, sizeof(TQChangeGuildSize));

	Packet.cThreadID = cThreadID;	
	Packet.nGuildDBID = nGuildDBID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nAccountDBID = nAccountDBID;	
	Packet.nGuildDBID = nGuildDBID;
	Packet.nGuildSize = nGuildSize;

	AddSendData(MAINCMD_GUILD, QUERY_CHANGE_GUILDSIZE, (char*)&Packet, sizeof(TQChangeGuildSize));
}
void CDNDBConnection::QueryCheatDelGuildEffectItem( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID, int nItemID )
{
	TQDelGuildEffectItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID	= iWorldID;		
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.biCharacterDBID = biCharacterDBID;
	TxPacket.nGuildDBID		= nGuildDBID;	
	TxPacket.nItemID		= nItemID;

	AddSendData( MAINCMD_GUILD, QUERY_DEL_GUILDREWARDITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryItemExpireByCheat(BYTE cTheadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, INT64 biSerial)
{
	TQItemExpire packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cTheadID;
	packet.cWorldSetID = nWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.biCharacterDBID = biCharacterDBID;
	packet.biSerial = biSerial;

	AddSendData(MAINCMD_ITEM, QUERY_ITEMEXPIREBYCHEAT, (char*)&packet, sizeof(TQItemExpire));
}

void CDNDBConnection::QueryChangePetName(BYTE threadID, char worldSetID, UINT accountDBID, INT64 itemSerial, INT64 petSerial, LPCWSTR pName)
{
	if (!pName)
	{
		_ASSERT(0);
		return;
	}

	ALLOC_PACKET(TQChangePetName);

	packet.cThreadID = threadID;
	packet.cWorldSetID = worldSetID;
	packet.nAccountDBID = accountDBID;
	packet.itemSerial = itemSerial;
	packet.petSerial = petSerial;
	_wcscpy(packet.name, _countof(packet.name), pName, (int)wcslen(pName));

	AddSendData(MAINCMD_ITEM, QUERY_CHANGEPETNAME, (char*)&packet, sizeof(TQChangePetName));
}

void CDNDBConnection::QueryModItemExpireDate(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, INT64 biExpireDateItemSerial, INT64 biItemSerial, int nMin, int nMapID, const WCHAR* pwszIP)
{
	TQModItemExpireDate Item;
	memset(&Item, 0, sizeof(TQModItemExpireDate));

	Item.cThreadID = cThreadID;
	Item.cWorldSetID = nWorldSetID;
	Item.nAccountDBID = nAccountDBID;
	Item.biItemSerial = biItemSerial;
	Item.biExpireDateItemSerial = biExpireDateItemSerial;
	Item.biCharacterDBID = biCharacterDBID;
	Item.nMin = nMin;
	Item.iMapID = nMapID;
	_wcscpy(Item.wszIP, IPLENMAX, pwszIP, IPLENMAX);
	AddSendData(MAINCMD_ITEM, QUERY_MODITEMEXPIREDATE, (char*)&Item, sizeof(TQModItemExpireDate));
}

void CDNDBConnection::QueryModPetExp(BYTE cTheadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetSerialID, int nExp)
{
 	TQModPetExp packet;
 	packet.cThreadID = cTheadID;
 	packet.cWorldSetID = nWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.biPetSerial = biPetSerialID;
	packet.nExp = nExp;

	AddSendData(MAINCMD_ITEM, QUERY_MODPETEXP, (char*)&packet, sizeof(TQModPetExp));
}

void CDNDBConnection::QueryModPetSkill(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetItemSerial, char cSlotNum, int nSkillID )
{
	TQModPetSkill packet;
	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.biItemSerial = biPetItemSerial;
	packet.cSkillSlot = cSlotNum;
	packet.nSkillID = nSkillID;

	AddSendData(MAINCMD_ITEM, QUERY_MODPETSKILL, (char*)&packet, sizeof(TQModPetSkill));
}
void CDNDBConnection::QueryModPetSkillExpand(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biPetItemSerial)
{
	TQModPetSkillExpand packet;
	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.biItemSerial = biPetItemSerial;	

	AddSendData(MAINCMD_ITEM, QUERY_MODPETSKILLEXPAND, (char*)&packet, sizeof(TQModPetSkillExpand));
}

void CDNDBConnection::QueryModDegreeOfHunger(BYTE cThreadID, int nWorldSetID, UINT nAccoutDBID, INT64 biPetItemSerial, DWORD nDegreeOfHunger)
{
	TQModDegreeOfHunger packet;
	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;
	packet.nAccountDBID = nAccoutDBID;
	packet.biBodySerial = biPetItemSerial;
	packet.nDegreeOfHunger = nDegreeOfHunger;
	AddSendData(MAINCMD_ITEM, QUERY_MODDEGREEOFHUNGER, (char*)&packet, sizeof(TQModDegreeOfHunger));
}

void CDNDBConnection::QueryEnrollGuildWar(BYTE cThreadID, UINT nGuildDBID, char cWorldSetID, UINT nAccountDBID)
{
	TQEnrollGuildWar packet;
	memset(&packet, 0, sizeof(packet));

	packet.nAccountDBID = nAccountDBID;
#if defined(_VILLAGESERVER)
	packet.wScheduleID = g_pGuildWarManager->GetScheduleID();
#elif defined(_GAMESERVER)
	packet.wScheduleID = g_pMasterConnectionManager->GetGuildWarScheduleID(cWorldSetID);
#endif
	packet.nGuildDBID = nGuildDBID;
	packet.cWorldSetID = cWorldSetID;

	AddSendData(MAINCMD_GUILD, QUERY_ENROLL_GUILDWAR, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetGuildWarSchedule(BYTE cThreadID, char cWorldSetID)
{
	TQGetGuildWarSchedule packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARSCHEDULE, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarFinalSchedule(BYTE cThreadID, char cWorldSetID, short wScheduleID)
{
	TQGetGuildWarFinalSchedule packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.wScheduleID = wScheduleID;	

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARFINALSCHEDULE, (char*)&packet, sizeof(packet));

}
void CDNDBConnection::QueryAddGuildWarPoint(CDNUserSession *pSession, int nMissionID, int nOpeningPoint)
{
	TQAddGuildWarPointRecodrd packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nMissionID = nMissionID;
	packet.nOpeningPoint = nOpeningPoint;
	packet.nGuildDBID = pSession->GetGuildUID().nDBID;
	packet.cTeamColorCode = pSession->GetGuildSelfView().cTeamColorCode;

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWARPOINT, (char*)&packet, sizeof(packet));

}
void CDNDBConnection::QueryGetGuildWarPoint(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biDBID)
{
	TQGetGuildWarPoint packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.cQueryType = cQueryType;
	packet.biDBID = biDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPointPartTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biCharacterDBID)
{
	TQGetGuildWarPointPartTotal packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.cQueryType = cQueryType;
	packet.biCharacterDBID = biCharacterDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_PARTTOTAL, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPointGuildPartTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType,	UINT nGuildDBID)
{
	TQGetGuildWarPointGuildPartTotal packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.cQueryType = cQueryType;
	packet.nGuildDBID = nGuildDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPointDaily(BYTE cThreadID, char cWorldSetID )
{
	TQGetGuildWarPointDaily packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_DAILY, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPointGuildTotal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, bool bMasterServer, char cQueryType,	short wScheduleID, UINT nGuildDBID)
{
	TQGetGuildWarPointGuildTotal packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nAccountDBID = nAccountDBID;
	packet.bMasterServer = bMasterServer;
	packet.cQueryType = cQueryType;
	packet.wScheduleID = wScheduleID;
	packet.nGuildDBID = nGuildDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_GUILD_TOTAL, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPointRunningTotal(BYTE cThreadID, char cWorldSetID )
{
	TQGetGuildWarPointRunningTotal packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetGuildWarPointFinalRewards(BYTE cThreadID, char cWorldSetID )
{
	TQHeader packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOINT_FINAL_REWARDS, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarFinalMatchList(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT* pGuildDBID)
{
	TQAddGuildWarFinalMatchList packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.wScheduleID = wScheduleID;
	memcpy(packet.nGuildDBID, pGuildDBID, sizeof(packet.nGuildDBID));

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_FINAL_MATCHLIST, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryAddGuildWarPopularityVote(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, UINT nGuildDBID)
{
	TQAddGuildWarPopularityVote packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.nAccountDBID = nAccountDBID;
	packet.biCharacterDBID = biCharacterDBID;
	packet.nGuildDBID = nGuildDBID;	

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWARPOPULARITYVOTE, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarPopularityVote(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, char cQueryType, INT64 biCharacterDBID)
{
	TQGetGuildWarPopularityVote packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.nAccountDBID = nAccountDBID;
	packet.cQueryType = cQueryType;
	packet.biCharacterDBID = biCharacterDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARPOPULARITYVOTE, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryAddGuildWarFinalResults(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, short wScheduleID, char cMatchSequence, char cMatchTypeCode, char cMatchResultCode)
{
	TQAddGuildWarFinalResults packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.nGuildDBID = nGuildDBID;
	packet.wScheduleID = wScheduleID;
	packet.cMatchSequence = cMatchSequence;
	packet.cMatchTypeCode = cMatchTypeCode;
	packet.cMatchResultCode = cMatchResultCode;

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWARFINALRESULTS, (char*)&packet, sizeof(packet));
}
void CDNDBConnection::QueryGetGuildWarFinalResults(BYTE cThreadID, char cWorldSetID, short wScheduleID)
{
	TQGetGuildWarFinalResults packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.wScheduleID = wScheduleID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWARFINALRESULTS, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetGuildWarPreWinGuild(BYTE cThreadID, char cWorldSetID)
{
	TQGetGuildWarPreWinGuild packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWAR_PRE_WIN_GUILD, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarRewarForCharacter(BYTE cThreadID, CDNUserSession *pSession)
{
#if defined(_VILLAGESERVER)
	TQAddGuildWarRewardCharacter packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.wScheduleID = g_pGuildWarManager->GetScheduleID();
	packet.cRewardType = pSession->GetGuildWarRewardEnable();

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_REWARD_CHARACTER, (char*)&packet, sizeof(packet));
#endif	// #if defined(_VILLAGESERVER)
}

void CDNDBConnection::QueryGetGuildWarRewarForCharacter(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetGuildWarRewardCharacter packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
#if defined(_VILLAGESERVER)
	packet.wScheduleID = g_pGuildWarManager->GetScheduleID();
#endif	// #if defined(_VILLAGESERVER)

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWAR_REWARD_CHARACTER, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarRewarForGuild(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT nGuildDBID, char cRewardType)
{
	TQAddGuildWarRewardGuild packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.wScheduleID = wScheduleID;
	packet.nGuildDBID = nGuildDBID;
	packet.cRewardType = cRewardType;

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_REWARD_GUILD, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetGuildWarRewarForGuild(BYTE cThreadID, char cWorldSetID, short wScheduleID, UINT nGuildDBID, char cQueryType)
{
	TQGetGuildWarRewardGuild packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;	
	packet.nGuildDBID = nGuildDBID;
	packet.wScheduleID = wScheduleID;	
	packet.cQueryType = cQueryType;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDWAR_REWARD_GUILD, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarPointQueue(BYTE cThreadID, char cWorldSetID, UINT nGUildDBID, INT64 biAddPoint, const WCHAR* pwszIP)
{
	TQAddGuildWarPointQueue packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nGuildDBID = nGUildDBID;
	packet.biAddPoint = biAddPoint;
	memcpy(packet.wszIP, pwszIP, sizeof(packet.wszIP));
	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_POINT_QUEUE, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarSystemMailQueue(BYTE cThreadID, int nWorldSetID, WCHAR *pSenderName, UINT nGuildDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
													  TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag )
{
	TQSendSystemMail Mail;
	memset(&Mail, 0, sizeof(TQSendSystemMail));

	Mail.cThreadID = cThreadID;
	Mail.cWorldSetID = nWorldSetID;	
	if(pSenderName) _wcscpy(Mail.wszSenderName, MAILNAMELENMAX, pSenderName, (int)wcslen(pSenderName));
	Mail.biReceiverCharacterDBID = nGuildDBID;
	Mail.Code = MailCode;
	Mail.biFKey = biFKey;
	if(pSubject) _wcscpy(Mail.wszSubject, MAILTITLELENMAX, pSubject, (int)wcslen(pSubject) );
	if(pContent) _wcscpy(Mail.wszContent, MAILTEXTLENMAX, pContent, (int)wcslen(pContent) );
	Mail.biAttachCoin = biAttachCoin;

	Mail.iChannelID = nChannelID;
	Mail.iMapID = nMapID;
	Mail.bNewFlag = bNewFlag;

	int nCount = 0;
	if(AttachItemArray){
		for (int i = 0; i < MAILATTACHITEMMAX; i++){
			if(AttachItemArray[i].nItemID <= 0) continue;
			Mail.sAttachItem[nCount] = AttachItemArray[i];
			nCount++;
		}
	}
	Mail.cAttachItemCount = nCount;

	int nLen = sizeof(TQSendSystemMail) - sizeof(Mail.sAttachItem) +(sizeof(TItem) * nCount);

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_SYSTEM_MAIL_QUEUE, (char*)&Mail, nLen);
}

void CDNDBConnection::QueryAddGuildWarMakeGiftQueue(BYTE cThreadID, int nWorldSetID, char cRewardType, char* cClassID, INT64* biCharacterDBID)
{
	TQAddGuilWarMakeGiftQueue Cash;
	memset(&Cash, 0, sizeof(TQAddGuilWarMakeGiftQueue));

	Cash.cThreadID = cThreadID;
	Cash.cWorldSetID = nWorldSetID;	
	Cash.cRewardType = cRewardType;	
	memcpy(Cash.cClass, cClassID, sizeof(Cash.cClass));
	memcpy(Cash.biCharacterDBIDs, biCharacterDBID, sizeof(Cash.biCharacterDBIDs));	
	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE, (char*)&Cash, sizeof(TQAddGuilWarMakeGiftQueue));
}

void CDNDBConnection::QueryAddGuildWarMakeGiftNotGuildInfo(BYTE cThreadID, int nWorldSetID, char cRewardType, UINT nGuildDBID)
{
	TQAddGuildWarMakeGiftNotGuildInfo packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;	
	packet.cRewardType = cRewardType;
	packet.nGuildDBID = nGuildDBID;		
	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddDBJobSystemReserve(BYTE cThreadID, int nWorldSetID, char cJobType)
{
	TQAddJobReserve packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;	
	packet.cJobType = cJobType;	
	AddSendData(MAINCMD_JOBSYSTEM, QUERY_ADD_JOB_RESERVE, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetDBJobSystemReserve(BYTE cThreadID, int nWorldSetID, int nJobSeq)
{
	TQGetJobReserve packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = nWorldSetID;	
	packet.nJobSeq = nJobSeq;	
	AddSendData(MAINCMD_JOBSYSTEM, QUERY_GET_JOB_RESERVE, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryGetListGuildWarItemTradeRecord(BYTE threadID, CDNUserSession *pSession)
{
	ALLOC_PACKET(TQGetListGuildWarItemTradeRecord);

	packet.cThreadID = threadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.characterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_GUILD, QUERY_GETLIST_GUILDWAR_ITEM_TRADE_RECORD, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddGuildWarItemTradeRecord(CDNUserSession *pSession, int itemID, int count)
{
	ALLOC_PACKET(TQAddGuildWarItemTradeRecord);

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.characterDBID = pSession->GetCharacterDBID();
	packet.itemCount.id = itemID;
	packet.itemCount.count = count;

	AddSendData(MAINCMD_GUILD, QUERY_ADD_GUILDWAR_ITEM_TRADE_RECORD, (char*)&packet, sizeof(packet));
}
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
void CDNDBConnection::QueryModGuildExp(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cPointType, int nPointValue, int nMaxPoint, int nLevel, INT64 biCharacterDBID, int nMissionID, bool IsGuildLevelMax/* = false */)
#else	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
void CDNDBConnection::QueryModGuildExp(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cPointType, int nPointValue, int nMaxPoint, int nLevel, INT64 biCharacterDBID, int nMissionID)
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
{
	TQModGuildExp packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nGuildDBID = nGuildDBID;
	packet.cPointType = cPointType;
	packet.nPointValue = nPointValue;
	packet.nMaxPoint = nMaxPoint;
	packet.nLevel = nLevel;
	packet.biCharacterDBID = biCharacterDBID;
	packet.nMissionID = nMissionID;
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	packet.IsGuildLevelMax = IsGuildLevelMax;
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)

	AddSendData(MAINCMD_GUILD, QUERY_MOD_GUILDEXP, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryModGuildCheat(BYTE cThreadID, char cWorldSetID, UINT nGuildDBID, char cCheatType, int nPoint, char cLevel)
{
	TQModGuildCheat packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldSetID;
	packet.nGuildDBID = nGuildDBID;
	packet.cCheatType = cCheatType;
	packet.nPoint = nPoint;
	packet.cLevel = cLevel;
	

	AddSendData(MAINCMD_GUILD, QUERY_MOD_GUILDCHEAT, (char*)&packet, sizeof(packet));
}

//#########################################################################################################################################
// MAINCMD_MAIL
//#########################################################################################################################################

void CDNDBConnection::GetCountReceiveMail(CDNUserSession *pSession)
{
	TQGetCountReceiveMail Mail;
	memset(&Mail, 0, sizeof(TQGetCountReceiveMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.biReceiverCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MAIL, QUERY_GETCOUNTRECEIVEMAIL, (char*)&Mail, sizeof(TQGetCountReceiveMail));
}

void CDNDBConnection::QueryGetPageReceiveMail(CDNUserSession *pSession, int nPageNo)
{
	TQGetPageReceiveMail Mail;
	memset(&Mail, 0, sizeof(TQGetPageReceiveMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.iPageNumber = nPageNo;
	Mail.biReceiverCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MAIL, QUERY_GETPAGERECEIVEMAIL, (char*)&Mail, sizeof(TQGetPageReceiveMail));
}

void CDNDBConnection::QuerySendMail(CDNUserSession *pSession, const CSSendMail *pPacket, DBPacket::TSendMailItem *SendItemArray, int nTax)
{
	TQSendMail Mail;
	memset(&Mail, 0, sizeof(TQSendMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.biSenderCharacterDBID = pSession->GetCharacterDBID();
	if(pPacket->wszToCharacterName[0] != '\0') 
		_wcscpy(Mail.wszReceiverCharacterName, NAMELENMAX, pPacket->wszToCharacterName, (int)wcslen(pPacket->wszToCharacterName));
	Mail.Code = (MailType::Delivery)pPacket->cDeliveryType;
	if(pPacket->wszTitle[0] != '\0') 
		_wcscpy(Mail.wszSubject, MAILTITLELENMAX, pPacket->wszTitle, (int)wcslen(pPacket->wszTitle));
	if(pPacket->wszText[0] != '\0') 
		_wcscpy(Mail.wszContent, MAILTEXTLENMAX, pPacket->wszText, (int)wcslen(pPacket->wszText));
	Mail.biAttachCoin = pPacket->nAttachCoin;
	Mail.cAttachItemCount = pPacket->cAttachItemTotalCount;
	if(SendItemArray)
		memcpy(Mail.AttachItemArr, SendItemArray, sizeof(Mail.AttachItemArr));
	Mail.iChannelID = pSession->GetChannelID();
	Mail.iMapID = pSession->GetMapIndex();
	Mail.nTax = nTax;

	int nLen = sizeof(TQSendMail) - sizeof(Mail.AttachItemArr) +(sizeof(DBPacket::TSendMailItem) * Mail.cAttachItemCount);

	AddSendData(MAINCMD_MAIL, QUERY_SENDMAIL, (char*)&Mail, sizeof(TQSendMail));
}

void CDNDBConnection::QuerySendSystemMail(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, WCHAR *pSenderName, INT64 biReceiverCharDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
										  TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag/* = true*/)
{
	TQSendSystemMail Mail;
	memset(&Mail, 0, sizeof(TQSendSystemMail));

	Mail.cThreadID = cThreadID;
	Mail.cWorldSetID = nWorldSetID;
	Mail.nAccountDBID = nAccountDBID;
	if(pSenderName) _wcscpy(Mail.wszSenderName, MAILNAMELENMAX, pSenderName, (int)wcslen(pSenderName));
	Mail.biReceiverCharacterDBID = biReceiverCharDBID;
	Mail.Code = MailCode;
	Mail.biFKey = biFKey;
	if(pSubject) _wcscpy(Mail.wszSubject, MAILTITLELENMAX, pSubject, (int)wcslen(pSubject) );
	if(pContent) _wcscpy(Mail.wszContent, MAILTEXTLENMAX, pContent, (int)wcslen(pContent) );
	Mail.biAttachCoin = biAttachCoin;

	Mail.iChannelID = nChannelID;
	Mail.iMapID = nMapID;
	Mail.bNewFlag = bNewFlag;

	if(AttachItemArray){
		for (int i = 0; i < MAILATTACHITEMMAX; i++){
			if(AttachItemArray[i].nItemID <= 0) continue;
			Mail.sAttachItem[Mail.cAttachItemCount++] = AttachItemArray[i];
		}
	}

	int nLen = sizeof(TQSendSystemMail) - sizeof(Mail.sAttachItem) +(sizeof(TItem) * Mail.cAttachItemCount);

	AddSendData(MAINCMD_MAIL, QUERY_SENDSYSTEMMAIL, (char*)&Mail, nLen);
}

void CDNDBConnection::QueryReadMail(CDNUserSession *pSession, int nMailDBID)
{
	TQReadMail Mail;
	memset(&Mail, 0, sizeof(TQReadMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.iMailID = nMailDBID;

	AddSendData(MAINCMD_MAIL, QUERY_READMAIL, (char*)&Mail, sizeof(TQReadMail));
}

void CDNDBConnection::QueryDelMail(CDNUserSession *pSession, int *nMailDBIDArray)
{
	TQDelMail Mail;
	memset(&Mail, 0, sizeof(TQDelMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.biReceiverCharacterDBID = pSession->GetCharacterDBID();
	memcpy(Mail.iMailIDArray, nMailDBIDArray, sizeof(Mail.iMailIDArray));

	AddSendData(MAINCMD_MAIL, QUERY_DELMAIL, (char*)&Mail, sizeof(TQDelMail));
}

void CDNDBConnection::QueryGetListMailAttachment(CDNUserSession *pSession, int *nMailDBIDArray)
{
	TQGetListMailAttachment Mail;
	memset(&Mail, 0, sizeof(TQGetListMailAttachment));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.biCharacterDBID = pSession->GetCharacterDBID();
	memcpy(Mail.nMailDBIDArray, nMailDBIDArray, sizeof(Mail.nMailDBIDArray));

	AddSendData(MAINCMD_MAIL, QUERY_GETLISTMAILATTACHMENT, (char*)&Mail, sizeof(TQGetListMailAttachment));
}

void CDNDBConnection::QueryTakeMailAttachList(CDNUserSession *pSession, char cCount, DBPacket::TRequestTakeAttachInfo *InfoList)
{
	TQTakeAttachMailList Mail;
	memset(&Mail, 0, sizeof(TQTakeAttachMailList));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.nChannelID = pSession->GetChannelID();
	Mail.nMapID = pSession->GetMapIndex();
	_wcscpy( Mail.wszIP, _countof(Mail.wszIP), pSession->GetIpW(), IPLENMAX );
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	if (pSession->GetGuildUID().IsSet())
		Mail.nGuildDBID = pSession->GetGuildUID().nDBID;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	Mail.cPageCount = cCount;
	if(InfoList){
		for (int i = 0; i < cCount; i++){
			Mail.Info[i] = InfoList[i];
		}
	}

	int nLen = sizeof(TQTakeAttachMailList) - sizeof(Mail.Info) +(sizeof(DBPacket::TRequestTakeAttachInfo) * cCount);

	AddSendData(MAINCMD_MAIL, QUERY_TAKEATTACHMAILLIST, (char*)&Mail, nLen);
}

void CDNDBConnection::QueryTakeMailAttach(CDNUserSession *pSession, DBPacket::TRequestTakeAttachInfo &Info)
{
	TQTakeAttachMail Mail;
	memset(&Mail, 0, sizeof(TQTakeAttachMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.nChannelID = pSession->GetChannelID();
	Mail.nMapID = pSession->GetMapIndex();
	Mail.Info = Info;
	_wcscpy( Mail.wszIP, _countof(Mail.wszIP), pSession->GetIpW(), IPLENMAX );
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	if (pSession->GetGuildUID().IsSet())
		Mail.nGuildDBID = pSession->GetGuildUID().nDBID;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT

	AddSendData(MAINCMD_MAIL, QUERY_TAKEATTACHMAIL, (char*)&Mail, sizeof(TQTakeAttachMail));
}

#ifdef PRE_ADD_BEGINNERGUILD
void CDNDBConnection::QueryGetWillSendMails(CDNUserSession * pSession)
{
	TQGetWillMails packet;
	memset(&packet, 0, sizeof(TQGetWillMails));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MAIL, QUERY_GETWILLSENDMAIL, (char*)&packet, sizeof(TQGetWillMails));
}

void CDNDBConnection::QueryDelWillSendMails(CDNUserSession * pSession, int nWillSendMailID)
{
	TQDelWillMail packet;
	memset(&packet, 0, sizeof(TQDelWillMail));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nMailID = nWillSendMailID;

	AddSendData(MAINCMD_MAIL, QUERY_DELWILLSENDMAIL, (char*)&packet, sizeof(TQDelWillMail));
}
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#if defined(PRE_ADD_CADGE_CASH)
void CDNDBConnection::QuerySendWishMail(CDNUserSession *pSession, const CSCashShopCadge *pPacket)
{
	TQSendWishMail Mail;
	memset(&Mail, 0, sizeof(TQSendWishMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.biSenderCharacterDBID = pSession->GetCharacterDBID();
	if(pPacket->wszToCharacterName[0] != '\0') 
		_wcscpy(Mail.wszReceiverCharacterName, NAMELENMAX, pPacket->wszToCharacterName, (int)wcslen(pPacket->wszToCharacterName));
	if(pPacket->wszMessage[0] != '\0') 
		_wcscpy(Mail.wszContent, MAILTEXTLENMAX, pPacket->wszMessage, (int)wcslen(pPacket->wszMessage));
	Mail.nWishLimitCount = (int)CGlobalWeightIntTable::GetInstance().GetValue(CGlobalWeightIntTable::CadgeMaxCountOneDay);
	
	Mail.cWishListCount = pPacket->cCount;
	if (Mail.cWishListCount > PACKAGEITEMMAX)
		Mail.cWishListCount = PACKAGEITEMMAX;

	Mail.nPackageSN = pPacket->nPackageSN;
	for (int i = 0; i < Mail.cWishListCount; i++){
		Mail.WishList[i].nItemSN = pPacket->PackageGiftList[i].nItemSN;
		Mail.WishList[i].nItemID = pPacket->PackageGiftList[i].nItemID;
		Mail.WishList[i].cOption = pPacket->PackageGiftList[i].nOptionIndex;
	}
	
	AddSendData(MAINCMD_MAIL, QUERY_SENDWISHMAIL, (char*)&Mail, sizeof(TQSendWishMail));
}

void CDNDBConnection::QueryReadWishMail(CDNUserSession *pSession, int nMailDBID)
{
	TQReadWishMail Mail;
	memset(&Mail, 0, sizeof(TQReadWishMail));

	Mail.cThreadID = pSession->GetDBThreadID();
	Mail.cWorldSetID = pSession->GetWorldSetID();
	Mail.nAccountDBID = pSession->GetAccountDBID();
	Mail.nMailID = nMailDBID;

	AddSendData(MAINCMD_MAIL, QUERY_READWISHMAIL, (char*)&Mail, sizeof(TQReadWishMail));	
}
#endif	// #if defined(PRE_ADD_CADGE_CASH)

// MAINCMD_SPECIALBOX
#if defined(PRE_SPECIALBOX)
void CDNDBConnection::QueryAddEventReward(CDNUserSession *pSession, BYTE cReceiveTypeCode, BYTE cTargetTypeCode, BYTE cTargetClassCode, BYTE cTargetMinLevel, BYTE cTargetMaxLevel, 
										  int nEventNameUIString, int nSenderNameUIString, int nContentUIString, int nRewardCoin, char cEventRewardTypeCode, std::vector<TSpecialBoxItemInfo> &VecItemList)
{
	TQAddEventReward Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cReceiveTypeCode = cReceiveTypeCode;
	Packet.cTargetTypeCode = cTargetTypeCode;
	if (pSession->GetAccountName())
		_wcscpy(Packet.wszAccountName, IDLENMAX, pSession->GetAccountName(), (int)wcslen(pSession->GetAccountName()));
	if (pSession->GetCharacterName()) 
		_wcscpy(Packet.wszCharacterName, NAMELENMAX, pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()));
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.cTargetClassCode = cTargetClassCode;
	Packet.cTargetMinLevel = cTargetMinLevel;
	Packet.cTargetMaxLevel = cTargetMaxLevel;
	Packet.nEventNameUIString = nEventNameUIString;
	Packet.nSenderNameUIString = nSenderNameUIString;
	Packet.nContentUIString = nContentUIString;
	Packet.nRewardCoin = nRewardCoin;
	Packet.cEventRewardTypeCode = cEventRewardTypeCode;
	if (!VecItemList.empty()){
		for (int i = 0; i < (int)VecItemList.size(); i++){
			Packet.EventRewardItems[i] = VecItemList[i];
		}
		Packet.cCount = (BYTE)VecItemList.size();
	}

	int nLen = sizeof(TQAddEventReward) - sizeof(Packet.EventRewardItems) + (sizeof(TItem) * Packet.cCount);

	AddSendData(MAINCMD_MAIL, QUERY_ADDEVENTREWARD, reinterpret_cast<char*>(&Packet), nLen);
}

void CDNDBConnection::QueryGetCountEventReward(CDNUserSession *pSession)
{
	TQGetCountEventReward Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();	

	AddSendData(MAINCMD_MAIL, QUERY_GETCOUNTEVENTREWARD, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetListEventReward(CDNUserSession *pSession)
{
	TQGetListEventReward Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.cClassCode = pSession->GetClassID();
	Packet.nLevel = pSession->GetLevel();

	AddSendData(MAINCMD_MAIL, QUERY_GETLISTEVENTREWARD, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetListEventRewardItem(CDNUserSession *pSession, int nEventRewardID)
{
	TQGetListEventRewardItem Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nEventRewardID = nEventRewardID;

	AddSendData(MAINCMD_MAIL, QUERY_GETLISTEVENTREWARDITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryAddEventRewardReceiver(CDNUserSession *pSession, int nEventRewardID, int nItemID)
{
	TQAddEventRewardReceiver Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nEventRewardID = nEventRewardID;
	Packet.nItemID = nItemID;

	AddSendData(MAINCMD_MAIL, QUERY_ADDEVENTREWARDRECEIVER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif	// #if defined(PRE_SPECIALBOX)

#ifdef PRE_ADD_JOINGUILD_SUPPORT
void CDNDBConnection::QueryGuildSupportRewardInfo(CDNUserSession * pSession)
{
	TQGuildSupportRewardInfo packet;
	memset(&packet, 0, sizeof(TQGuildSupportRewardInfo));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MAIL, QUERY_GUILDSUPPORT_REWARDINFO, reinterpret_cast<char*>(&packet), sizeof(TQGuildSupportRewardInfo));
}

void CDNDBConnection::QuerySendGuildMail(BYTE cThreadID, int nWorldSetID, WCHAR *pSenderName, UINT nReceiverAccountDBID, INT64 biReceiverCharDBID, UINT nAchivementAccountDBID, INT64 biAchivementCharacterDBID, DBDNWorldDef::MailTypeCode::eCode MailCode, INT64 biFKey, WCHAR *pSubject, WCHAR *pContent, INT64 biAttachCoin,
										 TItem *AttachItemArray, int nChannelID, int nMapID, bool bNewFlag)
{
	TQSendGuildMail Mail;
	memset(&Mail, 0, sizeof(TQSendGuildMail));

	Mail.cThreadID = cThreadID;
	Mail.cWorldSetID = nWorldSetID;
	if(pSenderName) _wcscpy(Mail.wszSenderName, MAILNAMELENMAX, pSenderName, (int)wcslen(pSenderName));
	Mail.nAccountDBID = nAchivementAccountDBID;
	Mail.nReceiverAccountDBID = nReceiverAccountDBID;
	Mail.biReceiverCharacterDBID = biReceiverCharDBID;
	Mail.biAchivementCharacterDBID = biAchivementCharacterDBID;
	Mail.Code = MailCode;
	Mail.biFKey = biFKey;
	if(pSubject) _wcscpy(Mail.wszSubject, MAILTITLELENMAX, pSubject, (int)wcslen(pSubject) );
	if(pContent) _wcscpy(Mail.wszContent, MAILTEXTLENMAX, pContent, (int)wcslen(pContent) );
	Mail.biAttachCoin = biAttachCoin;

	Mail.iChannelID = nChannelID;
	Mail.iMapID = nMapID;
	Mail.bNewFlag = bNewFlag;

	if(AttachItemArray){
		for (int i = 0; i < MAILATTACHITEMMAX; i++){
			if(AttachItemArray[i].nItemID <= 0) continue;
			Mail.sAttachItem[Mail.cAttachItemCount++] = AttachItemArray[i];
		}
	}

	int nLen = sizeof(TQSendGuildMail) - sizeof(Mail.sAttachItem) +(sizeof(TItem) * Mail.cAttachItemCount);

	AddSendData(MAINCMD_MAIL, QUERY_SENDGUILDMAIL, (char*)&Mail, nLen);
}
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT


//#########################################################################################################################################
// MAINCMD_MARKET
//#########################################################################################################################################

void CDNDBConnection::QueryGetListMyTrade(CDNUserSession *pSession)
{
	TQGetListMyTrade Market;
	memset(&Market, 0, sizeof(TQGetListMyTrade));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MARKET, QUERY_GETLISTMYTRADE, (char*)&Market, sizeof(TQGetListMyTrade));
}

void CDNDBConnection::QueryAddTrade(CDNUserSession *pSession, const TItem &RegisterItem, INT64 biItemNewSerial, BYTE cInvenIndex, int nPrice, int nRegisterTax, int nSellTax, int nSellPeriod, bool bDisplayFlag1, bool bDisplayFlag2, 
									WCHAR *pItemName, char cItemGrade, char cLimitLevel, char cJob, int nInvenType, bool bPremiumTrade, int nMaxRegisterItemCount, bool bCash, char cMethodCode, int nExchangeCode)
{
	TQAddTrade Market;
	memset(&Market, 0, sizeof(TQAddTrade));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.wRegisterMaxCount = (int)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::MarketRegisterCount_Basic );
	Market.wItemRegisterMaxCount = nMaxRegisterItemCount;
	Market.nItemID = RegisterItem.nItemID;
	Market.nSerial = RegisterItem.nSerial;
	Market.wCount = RegisterItem.wCount;
	Market.cLevel = RegisterItem.cLevel;
	Market.biNewSerial = biItemNewSerial;
	Market.cInvenType = nInvenType;
	Market.cInvenIndex = cInvenIndex;
	Market.nPrice = nPrice;
	Market.nSellTax = nSellTax;
	Market.cSellPeriod = nSellPeriod;
	if(pItemName){
		_wcscpy(Market.wszItemName, EXTDATANAMELENMAX, pItemName, (int)wcslen(pItemName));
		Market.cItemGrade = cItemGrade;
		Market.cLimitLevel = cLimitLevel;
	}
	if(nInvenType == ITEMPOSITION_INVEN)
		Market.nMainType = g_pDataManager->GetItemMainType(Market.nItemID);
	else if(nInvenType == ITEMPOSITION_CASHINVEN)
		Market.nMainType = g_pDataManager->GetItemMainType(Market.nItemID) + 100;

	Market.cDetailType = g_pDataManager->GetItemDetailType(Market.nItemID);
	// Crest item
	if( Market.nMainType == 38 && Market.cDetailType == 3 )
	{
		Market.cDetailType = 1;
	}

	Market.cSkillUsingType = g_pDataManager->GetItemSkillUsingType(Market.nItemID);
	Market.cJob = cJob;
	Market.nUnitPrice = (int)(nPrice / RegisterItem.wCount);
	Market.nRegisterTax = nRegisterTax;
	Market.bDisplayFlag1 = bDisplayFlag1;
	Market.bDisplayFlag2 = bDisplayFlag2;
	Market.nChannelID = pSession->GetChannelID();
	Market.nMapID = pSession->GetMapIndex();
	Market.bPremiumTrade = bPremiumTrade;
	Market.cMethodCode = cMethodCode;
	Market.nExchangeCode = nExchangeCode;
	Market.bCash = bCash;

	AddSendData(MAINCMD_MARKET, QUERY_ADDTRADE, (char*)&Market, sizeof(TQAddTrade));
}

void CDNDBConnection::QueryGetPageTrade(CDNUserSession *pSession, const CSMarketList *pMarketList, char *cJobCodeArray, char *cItemExchangeArray, WCHAR *pSearchItemName)
{
	TQGetPageTrade Market;
	memset(&Market, 0, sizeof(TQGetPageTrade));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.wPageNum = pMarketList->wPageNum;
	Market.cMinLevel = pMarketList->cMinLevel;
	Market.cMaxLevel = pMarketList->cMaxLevel;
	Market.cMethodCode = pMarketList->cPayMethodCode;

	if(pMarketList->cItemGrade) memcpy(Market.cItemGrade, pMarketList->cItemGrade, sizeof(Market.cItemGrade));
	if(cJobCodeArray) memcpy(Market.cJobCode, cJobCodeArray, sizeof(Market.cJobCode));
	if(cItemExchangeArray) memcpy(Market.cItemExchange, cItemExchangeArray, sizeof(Market.cItemExchange));
	if(pSearchItemName) _wcscpy(Market.wszSearchItemName, SEARCHLENMAX, pSearchItemName, (int)wcslen(pSearchItemName));
	
	Market.cSortType = pMarketList->cSortType;

	AddSendData(MAINCMD_MARKET, QUERY_GETPAGETRADE, (char*)&Market, sizeof(TQGetPageTrade));
}

void CDNDBConnection::QueryCancelTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bCashItem)
{
	TQCancelTrade Market;
	memset(&Market, 0, sizeof(TQCancelTrade));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.nMarketDBID = nMarketDBID;
	Market.cInvenSlotIndex = nItemSlotIndex;
	Market.nChannelID = pSession->GetChannelID();
	Market.nMapID = pSession->GetMapIndex();
	Market.bCashItem = bCashItem;

	AddSendData(MAINCMD_MARKET, QUERY_CANCELTRADE, (char*)&Market, sizeof(TQCancelTrade));
}

#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
void CDNDBConnection::QueryBuyTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bMini)
#else	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
void CDNDBConnection::QueryBuyTrade(CDNUserSession *pSession, int nMarketDBID, int nItemSlotIndex, bool bCashItem, int nPetalPrice)
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
{
	TQBuyTradeItem Market;
	memset(&Market, 0, sizeof(TQBuyTradeItem));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.nMarketDBID = nMarketDBID;
	Market.cInvenSlotIndex = nItemSlotIndex;
	Market.nChannelID = pSession->GetChannelID();
	Market.nMapID = pSession->GetMapIndex();

#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	Market.bMini = bMini;
#else	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	Market.bCashItem = bCashItem;
#if defined(PRE_ADD_PETALTRADE)
	Market.nPetalPrice = nPetalPrice;
#endif
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)

	_wcscpy( Market.wszIP, _countof(Market.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData(MAINCMD_MARKET, QUERY_BUYTRADEITEM, (char*)&Market, sizeof(TQBuyTradeItem));
}

void CDNDBConnection::QueryGetListTradeforCalculation(CDNUserSession *pSession)
{
	TQGetListTradeForCalculation Market;
	memset(&Market, 0, sizeof(TQGetListTradeForCalculation));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_MARKET, QUERY_GETLISTTRADEFORCALCULATION, (char*)&Market, sizeof(TQGetListTradeForCalculation));
}

void CDNDBConnection::QueryTradeCalculation(CDNUserSession *pSession, int nMarketDBID)
{
	TQTradeCalculate Market;
	memset(&Market, 0, sizeof(TQTradeCalculate));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.nMarketDBID = nMarketDBID;
	Market.nChannelID = pSession->GetChannelID();
	Market.nMapID = pSession->GetMapIndex();

	AddSendData(MAINCMD_MARKET, QUERY_TRADECALCULATE, (char*)&Market, sizeof(TQTradeCalculate));
}

void CDNDBConnection::QueryTradeCalculationAll(CDNUserSession *pSession)
{
	TQTradeCalculateAll Market;
	memset(&Market, 0, sizeof(TQTradeCalculateAll));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.nChannelID = pSession->GetChannelID();
	Market.nMapID = pSession->GetMapIndex();

	AddSendData(MAINCMD_MARKET, QUERY_TRADECALCULATEALL, (char*)&Market, sizeof(TQTradeCalculateAll));
}

#if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
void CDNDBConnection::QueryGetTradeItemID(CDNUserSession *pSession, int nMarketDBID)
{
	TQGetTradeItemID Market;

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.nMarketDBID = nMarketDBID;

	AddSendData(MAINCMD_MARKET, QUERY_GETTRADEITEMID, (char*)&Market, sizeof(TQGetTradeItemID));
}
#endif	// #if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)

void CDNDBConnection::QueryGetPetalBalance(CDNUserSession *pSession)
{
	TQHeader Market;

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();

	AddSendData(MAINCMD_MARKET, QUERY_GETPETALBALANCE, (char*)&Market, sizeof(TQHeader));
}

void CDNDBConnection::QueryGetTradePrice(CDNUserSession *pSession, const CSMarketPrice *pMarket)
{
	TQGetTradePrice Market;
	memset(&Market, 0, sizeof(TQGetTradePrice));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();

	Market.nMarketDBID = pMarket->nMarketDBID;
	Market.nItemID = pMarket->nItemID;
	Market.cLevel = pMarket->cLevel;
	Market.cOption = pMarket->cOption;

	AddSendData(MAINCMD_MARKET, QUERY_GETTRADEPRICE, (char*)&Market, sizeof(TQGetTradePrice));
}

void CDNDBConnection::QueryGetListMiniTrade(CDNUserSession *pSession, int nItemID, char cPayMethodCode)
{
	TQGetListMiniTrade Market;
	memset(&Market, 0, sizeof(TQGetListMiniTrade));

	Market.cThreadID = pSession->GetDBThreadID();
	Market.cWorldSetID = pSession->GetWorldSetID();
	Market.nAccountDBID = pSession->GetAccountDBID();
	Market.biCharacterDBID = pSession->GetCharacterDBID();
	Market.nItemID = nItemID;
	Market.cPayMethodCode = cPayMethodCode;

	AddSendData(MAINCMD_MARKET, QUERY_GETLISTMINITRADE, (char*)&Market, sizeof(TQGetListMiniTrade));
}


//#########################################################################################################################################
// MAINCMD_ISOLATE
//#########################################################################################################################################

void CDNDBConnection::QueryGetIsolateList(BYTE cThreadID, CDNUserSession *pSession)
{
	TQGetIsolateList packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = cThreadID;
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_ISOLATE, QUERY_GETISOLATELIST, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddIsolate(CDNUserSession *pSession, const WCHAR * pIsolateName)
{
	TQAddIsolate packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	if(pIsolateName) _wcscpy(packet.wszIsolateName, NAMELENMAX, pIsolateName, (int)wcslen(pIsolateName));

	AddSendData(MAINCMD_ISOLATE, QUERY_ADDISOLATE, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryAddIsolateAfterDelFriend(CDNUserSession *pSession, const WCHAR * pIsolateName, INT64 biDelFriendCharacterDBID)
{
	TQAddIsolate packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	if(pIsolateName) _wcscpy(packet.wszIsolateName, NAMELENMAX, pIsolateName, (int)wcslen(pIsolateName));
	packet.biFriendCharacterDBID = biDelFriendCharacterDBID;

	AddSendData(MAINCMD_ISOLATE, QUERY_ADDISOLATE, (char*)&packet, sizeof(packet));
}

void CDNDBConnection::QueryDelIsolate(CDNUserSession *pSession, INT64 biIsolateCharacterDBID)
{
	TQDelIsolate packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.cCount = 1;
	packet.biIsolateCharacterDBIDs[0] = biIsolateCharacterDBID;

	AddSendData(MAINCMD_ISOLATE, QUERY_DELISOLATE, (char*)&packet, sizeof(packet) - sizeof(packet.biIsolateCharacterDBIDs) +(sizeof(INT64) * packet.cCount));
}


//#########################################################################################################################################
// PvP
//#########################################################################################################################################

bool CDNDBConnection::QueryUpdatePvPData( BYTE cThreadID, CDNUserSession* pGameSession, PvPCommon::QueryUpdatePvPDataType::eCode Type )
{
	if( !pGameSession || !pGameSession->GetPvPData() )
	{
		_DANGER_POINT();
		return false;
	}

	// Do not save user before PvP info is initialized.
	if( pGameSession->GetPvPData()->cLevel == 0 || pGameSession->GetPvPData()->cLevel > PvPCommon::Common::MaxRank )
	{
		_DANGER_POINT();
		return false;
	}

	TQUpdatePvPData TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.nAccountDBID = pGameSession->GetAccountDBID();
	TxPacket.cWorldSetID = pGameSession->GetWorldSetID();
	TxPacket.biCharacterDBID = pGameSession->GetCharacterDBID();
	TxPacket.Type = Type;
	memcpy( &TxPacket.PvP, pGameSession->GetPvPData(), sizeof(TPvPGroup) );

	AddSendData( MAINCMD_PVP, QUERY_UPDATEPVPDATA, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	return true;
}

#ifdef PRE_MOD_PVPRANK
void CDNDBConnection::QuerySetPvPRankCriteria(BYTE cThreadID, UINT nThreshold)
{
	TQUpdatePvPRankCriteria packet;
	memset(&packet, 0, sizeof(TQUpdatePvPRankCriteria));

#ifdef _VILLAGESERVER
	packet.cWorldSetID = g_Config.nWorldSetID;
#endif		//#ifdef _VILLAGESERVER
	packet.cThreadID = cThreadID;
	packet.nThresholdPvPExp = nThreshold;

	AddSendData( MAINCMD_PVP, QUERY_UPDATE_PVPRANKCRITERIA, reinterpret_cast<char*>(&packet), sizeof(packet) );
}

void CDNDBConnection::QueryCalcPvPRank(BYTE cThreadID)
{
	TQForceCalcPvPRank packet;
	memset(&packet, 0, sizeof(TQForceCalcPvPRank));

	packet.cThreadID = cThreadID;

	AddSendData( MAINCMD_PVP, QUERY_FORCE_CALCPVPRANK, reinterpret_cast<char*>(&packet), sizeof(packet) );
}
#endif		//#ifdef PRE_MOD_PVPRANK

#if defined( PRE_PVP_GAMBLEROOM )
void CDNDBConnection::QueryAddGambleRoom(BYTE cThreadID, char cWorldSetID, int nRoomID, BYTE cGambleType, int nGamblePrice)
{
	TQAddGambleRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nRoomID = nRoomID;
	Packet.cGambleType = cGambleType;
	Packet.nGamblePrice = nGamblePrice;

	AddSendData(MAINCMD_PVP, QUERY_ADD_GAMBLEROOM, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddGambleRoomMember(BYTE cThreadID, char cWorldSetID, INT64 nGambleDBID, INT64 biCharacterDBID)
{
	TQAddGambleRoomMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nGambleDBID = nGambleDBID;
	Packet.biCharacterDBID = biCharacterDBID;

	AddSendData(MAINCMD_PVP, QUERY_ADD_GAMBLEROOMMEMBER, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryEndGambleRoom(BYTE cThreadID, char cWorldSetID, INT64 nGambleDBID, TPvPGambleRoomWinData* Member)
{
	TQEndGambleRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nGambleDBID = nGambleDBID;
	memcpy( &Packet.Member, Member, sizeof(Packet.Member)*PvPGambleRoom::Max);

	AddSendData(MAINCMD_PVP, QUERY_END_GAMBLEROOM, (char*)&Packet, sizeof(Packet));
}
#endif

//#########################################################################################################################################
// DarkLair
//#########################################################################################################################################

bool CDNDBConnection::QueryUpdateDarkLairResult( BYTE cThreadID, TQUpdateDarkLairResult* pPacket, const int iPacketSize )
{
	pPacket->cThreadID = cThreadID;
	AddSendData( MAINCMD_DARKLAIR, QUERY_UPDATE_DARKLAIR_RESULT, reinterpret_cast<char*>(pPacket), iPacketSize );
	return true;
}

void CDNDBConnection::QueryGetDarkLairRankBoard( CDNUserSession* pSession, int iMapIndex, BYTE cPartyUserCount )
{
	TQGetDarkLairRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iMapIndex = iMapIndex;
	TxPacket.cPartyUserCount = cPartyUserCount;

	AddSendData( MAINCMD_DARKLAIR, QUERY_GET_DARKLAIR_RANK_BOARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

// CashShop
void CDNDBConnection::SendModGiveFail(CDNUserSession* pSession, int nCount, DBPacket::TModCashFailItemEx *CashItemArray)
{
	TQModGiveFailFlag Cash;
	memset(&Cash, 0, sizeof(TQModGiveFailFlag));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.nCount = nCount;
	if(CashItemArray) memcpy(Cash.CashItem, CashItemArray, sizeof(DBPacket::TModCashFailItemEx) * nCount);
	_wcscpy( Cash.wszIP, _countof(Cash.wszIP), pSession->GetIpW(), IPLENMAX );

	int nLen = sizeof(TQModGiveFailFlag) - sizeof(Cash.CashItem) + (sizeof(DBPacket::TModCashFailItemEx) * nCount);
	AddSendData(MAINCMD_CASH, QUERY_MODGIVEFAILFLAG, (char*)&Cash, nLen);
}

void CDNDBConnection::QueryCheckGiftReceiver(CDNUserSession* pSession, WCHAR *pCharName)
{
	TQCheckGiftReceiver Cash;
	memset( &Cash, 0, sizeof(Cash) );

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	if(pCharName) _wcscpy(Cash.wszCharacterName, NAMELENMAX, pCharName, (int)wcslen(pCharName));

	AddSendData(MAINCMD_CASH, QUERY_CHECKGIFTRECEIVER, (char*)&Cash, sizeof(TQCheckGiftReceiver));
}

void CDNDBConnection::QueryGetListGiftBox(CDNUserSession* pSession)
{
	TQGetListGiftBox Cash;
	memset(&Cash, 0, sizeof(TQGetListGiftBox));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_CASH, QUERY_GETLISTGIFTBOX, (char*)&Cash, sizeof(TQGetListGiftBox));
}

void CDNDBConnection::QueryModGiftReceive(CDNUserSession* pSession, bool bReceiveAll, TReceiveGiftData *GiftData)
{
	TQModGiftReceiveFlag Cash;
	memset(&Cash, 0, sizeof(TQModGiftReceiveFlag));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
#if defined(PRE_RECEIVEGIFTALL)
	Cash.bReceiveAll = bReceiveAll;
#endif	// #if defined(PRE_RECEIVEGIFTALL)

	if (GiftData){
		if (bReceiveAll){
			Cash.cCount = 0;
			for (int i = 0; i < GIFTPAGEMAX; i++){
				if (GiftData[i].nGiftDBID <= 0) continue;

				Cash.GiftData[Cash.cCount] = GiftData[i];
				Cash.cCount++;
			}
		}
		else{
			Cash.cCount = 1;
			Cash.GiftData[0] = GiftData[0];
		}
	}

	int nLen = sizeof(TQModGiftReceiveFlag) - sizeof(Cash.GiftData) + (sizeof(TReceiveGiftData) * Cash.cCount);

	AddSendData(MAINCMD_CASH, QUERY_MODGIFTRECEIVEFLAG, (char*)&Cash, nLen);
}

void CDNDBConnection::QueryReceiveGift(CDNUserSession* pSession, bool bReceiveAll, char cTotalCount, const TReceiveGiftData &GiftData, char cItemCount, TGiftItem *AddItemList, int nItemSN)
{
	TQReceiveGift Cash;
	memset(&Cash, 0, sizeof(TQReceiveGift));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
#if defined(PRE_RECEIVEGIFTALL)
	Cash.bReceiveAll = bReceiveAll;
	Cash.cTotalCount = cTotalCount;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
	Cash.GiftData = GiftData;
	Cash.nChannelID = pSession->GetChannelID();
	Cash.nMapID = pSession->GetMapIndex();
	Cash.nItemSN = nItemSN;
	Cash.cCount = cItemCount;
	memcpy(Cash.GiftItem, AddItemList, (sizeof(TGiftItem) * cItemCount));
	_wcscpy( Cash.wszIP, _countof(Cash.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData(MAINCMD_CASH, QUERY_RECEIVEGIFT, (char*)&Cash, sizeof(TQReceiveGift));
}

void CDNDBConnection::QueryNotifyGift(CDNUserSession* pSession, bool bNew)
{
	TQNotifyGift Cash;
	memset(&Cash, 0, sizeof(TQNotifyGift));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.bNew = bNew;

	AddSendData(MAINCMD_CASH, QUERY_NOTIFYGIFT, (char*)&Cash, sizeof(TQNotifyGift));
}

void CDNDBConnection::QueryMakeGiftByQuest(CDNUserSession* pSession, int nQuestID, WCHAR *pMemo, int *nRewardItemList, bool bNewFlag)
{
	TQMakeGiftByQuest Cash;
	memset(&Cash, 0, sizeof(TQMakeGiftByQuest));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.bPCBang = pSession->IsPCBang();
	Cash.nQuestID = nQuestID;
	if(pMemo) _wcscpy(Cash.wszMemo, GIFTMESSAGEMAX, pMemo, (int)wcslen(pMemo) );
	Cash.nLifeSpan = 7 * 24 * 60;	// The design doc says 7 days for now
	if( pSession->GetIp() )
		_strcpy(Cash.szIp, _countof(Cash.szIp), pSession->GetIp(), (int)strlen(pSession->GetIp()));
	Cash.bNewFlag = bNewFlag;

	int nCount = 0;
	for (int i = 0; i < MAILATTACHITEMMAX; i++){
		if(nRewardItemList[i] <= 0) continue;
		Cash.nRewardItemSN[nCount] = nRewardItemList[i];
		nCount++;
	}

	Cash.cRewardCount = nCount;
	int nLen = sizeof(TQMakeGiftByQuest) - sizeof(Cash.nRewardItemSN) +(sizeof(int) * Cash.cRewardCount);

	AddSendData(MAINCMD_CASH, QUERY_MAKEGIFTBYQUEST, (char*)&Cash, sizeof(TQMakeGiftByQuest));
}

void CDNDBConnection::QueryMakeGift(BYTE cThreadID, int nWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, bool bPCBang, WCHAR *pMemo, char cPayMethod, int iOrderKey, char *pIp, int *nRewardItemList, bool bNewFlag)
{
	TQMakeGift Cash;
	memset(&Cash, 0, sizeof(TQMakeGift));

	Cash.cThreadID = cThreadID;
	Cash.cWorldSetID = nWorldSetID;
	Cash.nAccountDBID = nAccountDBID;
	Cash.biCharacterDBID = biCharacterDBID;
	Cash.bPCBang = bPCBang;
	if(pMemo) _wcscpy(Cash.wszMemo, GIFTMESSAGEMAX, pMemo, (int)wcslen(pMemo) );
	Cash.cPayMethodCode = cPayMethod;
	Cash.iOrderKey = iOrderKey;
	Cash.nLifeSpan = 7 * 24 * 60;	// The design doc says 7 days for now
	if(pIp)
		_strcpy(Cash.szIp, _countof(Cash.szIp), pIp, (int)strlen(pIp));
	Cash.bNewFlag = bNewFlag;

	int nCount = 0;
	for (int i = 0; i < 5; i++){
		if(nRewardItemList[i] <= 0) continue;
		Cash.nRewardItemSN[nCount] = nRewardItemList[i];
		nCount++;
	}

	Cash.cRewardCount = nCount;
	int nLen = sizeof(TQMakeGift) - sizeof(Cash.nRewardItemSN) +(sizeof(int) * Cash.cRewardCount);

	AddSendData(MAINCMD_CASH, QUERY_MAKEGIFT, (char*)&Cash, sizeof(TQMakeGift));
}

void CDNDBConnection::QueryIncreaseVIPPoint(CDNUserSession* pSession, int nBasicPoint, INT64 biPurchaseOrderID, short wVIPPeriod, bool bAutoPay)
{
	TQIncreaseVIPPoint Cash;
	memset(&Cash, 0, sizeof(TQIncreaseVIPPoint));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.nBasicPoint = nBasicPoint;
	Cash.biPurchaseOrderID = biPurchaseOrderID;
	Cash.wVIPPeriod = wVIPPeriod;
	Cash.bAutoPay = bAutoPay;

	AddSendData(MAINCMD_CASH, QUERY_INCREASEVIPPOINT, (char*)&Cash, sizeof(TQIncreaseVIPPoint));
}

void CDNDBConnection::QueryGetVIPPoint(CDNUserSession* pSession)
{
	TQGetVIPPoint Cash;
	memset(&Cash, 0, sizeof(TQGetVIPPoint));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_CASH, QUERY_GETVIPPOINT, (char*)&Cash, sizeof(TQGetVIPPoint));
}

void CDNDBConnection::QueryModVIPAutoPayFlag(CDNUserSession* pSession, bool bAutoPay)
{
	TQModVIPAutoPayFlag Cash;
	memset(&Cash, 0, sizeof(TQModVIPAutoPayFlag));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.bAutoPay = bAutoPay;

	AddSendData(MAINCMD_CASH, QUERY_MODVIPAUTOPAYFLAG, (char*)&Cash, sizeof(TQModVIPAutoPayFlag));
}

void CDNDBConnection::QueryGiftByCheat(CDNUserSession* pSession, int nItemSN)
{
	TQGiftByCheat Cash;
	memset(&Cash, 0, sizeof(TQGiftByCheat));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	if(pSession->GetCharacterName()) _wcscpy(Cash.wszCharacterName, _countof(Cash.wszCharacterName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()));
	if(pSession->GetIp()) _strcpy(Cash.szIp, _countof(Cash.szIp), pSession->GetIp(), (int)strlen(pSession->GetIp()));
	Cash.nItemSN = nItemSN;

	AddSendData(MAINCMD_CASH, QUERY_GIFTBYCHEAT, (char*)&Cash, sizeof(TQGiftByCheat));
}

void CDNDBConnection::QueryPetal(CDNUserSession* pSession, int nPetal, int nUseItemID, char cInvenType, BYTE cInvenIndex, INT64 biInvenSerial)
{
	TQPetal Cash;
	memset(&Cash, 0, sizeof(TQPetal));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.nUseItemID = nUseItemID;
	Cash.nAddPetal = nPetal;
	Cash.cInvenType = cInvenType;
	Cash.cInvenIndex = cInvenIndex;
	Cash.biInvenSerial = biInvenSerial;
	Cash.iMapID	= pSession->GetMapIndex();
	_wcscpy( Cash.wszIP, _countof(Cash.wszIP), pSession->GetIpW(), IPLENMAX );

	AddSendData(MAINCMD_CASH, QUERY_PETAL, (char*)&Cash, sizeof(TQPetal));
}

#if defined( PRE_PVP_GAMBLEROOM )
void CDNDBConnection::QueryPetalLog(CDNUserSession* pSession, int nPetal, int nUseItemID, BYTE cLogCode, INT64 nGambleDBID)
{
	TQAddPetalLogCode Cash;
	memset(&Cash, 0, sizeof(TQAddPetalLogCode));

	Cash.cThreadID = pSession->GetDBThreadID();
	Cash.cWorldSetID = pSession->GetWorldSetID();
	Cash.nAccountDBID = pSession->GetAccountDBID();
	Cash.biCharacterDBID = pSession->GetCharacterDBID();
	Cash.nUseItemID = nUseItemID;
	Cash.nAddPetal = nPetal;
	Cash.cLogCode = cLogCode;
	Cash.nGambleDBID = nGambleDBID;
	
	AddSendData(MAINCMD_CASH, QUERY_LOGCODE_PETAL, (char*)&Cash, sizeof(TQAddPetalLogCode));
}

void CDNDBConnection::QueryUsePetal(BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, int nPetal, BYTE cLogCode, INT64 nGambleDBID)
{
	TQUsePetal Cash;
	memset(&Cash, 0, sizeof(TQUsePetal));

	Cash.cThreadID = cThreadID;
	Cash.cWorldSetID = cWorldSetID;
	Cash.nAccountDBID = nAccountDBID;
	Cash.biCharacterDBID = biCharacterDBID;
	Cash.nDelPetal = nPetal;
	Cash.cLogCode = cLogCode;
	Cash.nGambleDBID = nGambleDBID;

	AddSendData(MAINCMD_CASH, QUERY_USE_PETAL, (char*)&Cash, sizeof(TQUsePetal));
}
#endif

#if defined(PRE_ADD_GIFT_RETURN)
void CDNDBConnection::QueryModGiftRejectFlag(CDNUserSession* pSession, INT64 biPurchaseOrderID)
{
	TQGiftReturn GiftReturn;
	memset(&GiftReturn, 0, sizeof(TQGiftReturn));

	GiftReturn.cThreadID = pSession->GetDBThreadID();
	GiftReturn.cWorldSetID = pSession->GetWorldSetID();
	GiftReturn.nAccountDBID = pSession->GetAccountDBID();
	GiftReturn.biPurchaseOrderID = biPurchaseOrderID;

	AddSendData(MAINCMD_CASH, QUERY_GIFTRETURN, (char*)&GiftReturn, sizeof(TQGiftReturn));
}
#endif // #if defined(PRE_ADD_GIFT_RETURN)

#if defined(PRE_ADD_CASH_REFUND)
void CDNDBConnection::QueryGetListRefundableProducts(CDNUserSession* pSession)
{
	TQPaymentItemList PaymentItemList;
	memset(&PaymentItemList, 0, sizeof(TQPaymentItemList));

	PaymentItemList.cThreadID = pSession->GetDBThreadID();
	PaymentItemList.cWorldSetID = pSession->GetWorldSetID();
	PaymentItemList.nAccountDBID = pSession->GetAccountDBID();
	PaymentItemList.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_CASH, QUERY_PAYMENTINVEN_LIST, (char*)&PaymentItemList, sizeof(TQPaymentItemList));
}
#endif

void CDNDBConnection::QueryCheatGiftClear(CDNUserSession* pSession)
{	
	TQCheatGiftClear CheatGiftClear;
	memset(&CheatGiftClear, 0, sizeof(TQCheatGiftClear));

	CheatGiftClear.cThreadID = pSession->GetDBThreadID();
	CheatGiftClear.cWorldSetID = pSession->GetWorldSetID();
	CheatGiftClear.nAccountDBID = pSession->GetAccountDBID();
	CheatGiftClear.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_CASH, QUERY_CHEAT_GIFTCLEAR, (char*)&CheatGiftClear, sizeof(TQCheatGiftClear));
}

void CDNDBConnection::QueryMsgAdj(CDNUserSession* pSession, int nChannelID, int nVerifyMapIndex, char cGateNo)
{
	TQMsgAdjust packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.nChannelID = nChannelID;
	packet.nVerifyMapIndex = nVerifyMapIndex;
	packet.cGateNo = cGateNo;
	
	AddSendData(MAINCMD_MSGADJUST, QUERY_MSGADJUST, (char*)&packet, sizeof(packet));
}

#ifdef PRE_FIX_63822
void CDNDBConnection::QueryMsgAdjSeq(CDNUserSession * pSession, int nSubCmd)
{
	TQMsgAdjustSeq packet;
	memset(&packet, 0, sizeof(packet));

	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();	

	AddSendData(MAINCMD_MSGADJUST, nSubCmd, (char*)&packet, sizeof(packet));
}
#endif		//#ifdef PRE_FIX_63822

void CDNDBConnection::QueryAddPartyStartLog( CDNUserSession* pSession, INT64 biPartyLogID )
{
	TQAddPartyStartLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biPartyLogID = biPartyLogID;

	AddSendData( MAINCMD_LOG, QUERY_ADDPARTY_STARTLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddPartyEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biPartyLogID )
{
	TQAddPartyEndLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.biPartyLogID	= biPartyLogID;

	AddSendData( MAINCMD_LOG, QUERY_ADDPARTY_ENDLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddStageStartLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biPartyLogID, BYTE cPartySize, int iMapID, DBDNWorldDef::DifficultyCode::eCode Code )
{
	TQAddStageStartLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.biRoomLogID	= biRoomLogID;
	TxPacket.biPartyLogID	= biPartyLogID;
	TxPacket.cPartySize		= cPartySize;
	TxPacket.iMapID			= iMapID;
	TxPacket.Code			= Code;

	AddSendData( MAINCMD_LOG, QUERY_ADDSTAGE_STARTLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_36870 )
void CDNDBConnection::QueryAddStageClearLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, int iJobID, BYTE cLevel, bool bClearFlag, DBDNWorldDef::ClearGradeCode::eCode ClearCode, 
											int iMaxComboCount, BYTE cBossKillCount, int iRewardCharacterExp, int iBonusCharacterExp, int iClearPoint, int iBonusClearPoint, int iClearTimeSec, int iClearRound )
#else
void CDNDBConnection::QueryAddStageClearLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, int iJobID, BYTE cLevel, bool bClearFlag, DBDNWorldDef::ClearGradeCode::eCode ClearCode, 
											int iMaxComboCount, BYTE cBossKillCount, int iRewardCharacterExp, int iBonusCharacterExp, int iClearPoint, int iBonusClearPoint, int iClearTimeSec )
#endif // #if defined( PRE_ADD_36870 )
{
	TQAddStageClearLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID				= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.biRoomLogID			= biRoomLogID;
	TxPacket.biCharacterDBID		= biCharacterDBID;;
	TxPacket.iJobID					= iJobID;
	TxPacket.cCharacterLevel		= cLevel;
	TxPacket.bClearFlag				= bClearFlag;
	TxPacket.Code					= ClearCode;
	TxPacket.iMaxComboCount			= iMaxComboCount;
	TxPacket.cBossKillCount			= cBossKillCount;
	TxPacket.iRewardCharacterExp	= iRewardCharacterExp;
	TxPacket.iBonusCharacterExp		= iBonusCharacterExp;
	TxPacket.iClearPoint			= iClearPoint;
	TxPacket.iBonusClearPoint		= iBonusClearPoint;
	TxPacket.iClearTimeSec			= iClearTimeSec;
#if defined( PRE_ADD_36870 )
	TxPacket.iClearRound			= iClearRound;
#endif // #if defined( PRE_ADD_36870 )

	AddSendData( MAINCMD_LOG, QUERY_ADDSTAGE_CLEARLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddStageRewardLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, INT64 biCharacterDBID, DBDNWorldDef::RewardBoxCode::eCode Code, INT64 biRewardItemSerial, int iRewardItemID, int iRewardRet )
{
	TQAddStageRewardLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID				= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.biRoomLogID			= biRoomLogID;
	TxPacket.biCharacterDBID		= biCharacterDBID;
	TxPacket.Code					= Code;
	TxPacket.biRewardItemSerial		= biRewardItemSerial;
	TxPacket.iRewardItemID			= iRewardItemID;
	TxPacket.iRewardRet				= iRewardRet;

	AddSendData( MAINCMD_LOG, QUERY_ADDSTAGE_REWARDLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddStageEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomLogID, DBDNWorldDef::WhereToGoCode::eCode Code )
{
	TQAddStageEndLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.biRoomLogID	= biRoomLogID;
	TxPacket.Code			= Code;

	AddSendData( MAINCMD_LOG, QUERY_ADDSTAGE_ENDLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNDBConnection::QueryAddPvPStartLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID1, int iRoomID2, BYTE cMaxPlayerCount, DBDNWorldDef::PvPModeCode::eCode Code, int iPvPModeDetailValue,
										  bool bBreakIntoFlag, bool bItemUseFlag, bool bSecretFlag, int iMapID, INT64 biRoomMasterCharacterDBID, INT64 biCreateRoomCharacterDBID )
{
	TQAddPvPStartLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID			= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.biRoomID1				= biRoomID1;
	TxPacket.iRoomID2				= iRoomID2;
	TxPacket.cMaxPlayerCount		= cMaxPlayerCount;
	TxPacket.Code					= Code;
	TxPacket.iPvPModeDetailValue	= iPvPModeDetailValue;
	TxPacket.bBreakIntoFlag			= bBreakIntoFlag;
	TxPacket.bItemUseFlag			= bItemUseFlag;
	TxPacket.bSecretFlag			= bSecretFlag;
	TxPacket.iMapID					= iMapID;
	TxPacket.biRoomMasterCharacterDBID = biRoomMasterCharacterDBID;
	TxPacket.biCreateRoomCharacterDBID = biCreateRoomCharacterDBID;
	AddSendData( MAINCMD_LOG, QUERY_ADDPVP_STARTLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddPvPResultLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64	biRoomID1, int iRoomID2, INT64 biCharacterDBID, DBDNWorldDef::PvPTeamCode::eCode TeamCode,
										   bool bBreakIntoFlag, DBDNWorldDef::PvPResultCode::eCode ResultCode, int iPlayTimeSec, int iPvPKillPoint, int iPvPAssistPoint, int iPvPSupportPoint,
										   UINT* uiKillScore, UINT* uiKilledScore, int nOccupationCount, BYTE cOccupationWinType )
{
	TQAddPvPResultLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID			= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.biRoomID1				= biRoomID1;
	TxPacket.iRoomID2				= iRoomID2;
	TxPacket.biCharacterDBID		= biCharacterDBID;
	TxPacket.TeamCode				= TeamCode;
	TxPacket.bBreakIntoFlag			= bBreakIntoFlag;
	TxPacket.ResultCode				= ResultCode;
	TxPacket.iPlayTimeSec			= iPlayTimeSec;
	TxPacket.iPvPKillPoint			= iPvPKillPoint;
	TxPacket.iPvPAssistPoint		= iPvPAssistPoint;
	TxPacket.iPvPSupportPoint		= iPvPSupportPoint;
	TxPacket.nOccupationCount = nOccupationCount;
	TxPacket.cOccupationWinType = cOccupationWinType;

	for (int i=CLASS_WARRIER; i<CLASS_MAX; i++)
	{
		if (uiKillScore)
			TxPacket.iVsKOWin[i-1] = uiKillScore[i-1];

		if (uiKilledScore)
			TxPacket.iVsKOLose[i-1] = uiKilledScore[i-1];
	}

	AddSendData( MAINCMD_LOG, QUERY_ADDPVP_RESULTLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}


void CDNDBConnection::QueryAddPvPEndLog( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID1, int iRoomID2 )
{
	TQAddPvPEndLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID				= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.biRoomID1				= biRoomID1;
	TxPacket.iRoomID2				= iRoomID2;

	AddSendData( MAINCMD_LOG, QUERY_ADDPVP_ENDLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_58761 )
void CDNDBConnection::QueryAddNestGateStartLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, BYTE cPartySize, int nGateID)
{
	TQAddNestGateStartLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = uiAccountDBID;	
	TxPacket.biRoomID = biRoomID;
	TxPacket.cPartySize = cPartySize;
	TxPacket.nGateID = nGateID;	

	AddSendData( MAINCMD_LOG, QUERY_ADDNESTGATE_STARTLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddNestGateEndLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, BYTE cPartySize)
{
	TQAddNestGateEndLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = uiAccountDBID;	
	TxPacket.biRoomID = biRoomID;
	TxPacket.cPartySize = cPartySize;

	AddSendData( MAINCMD_LOG, QUERY_ADDNESTGATE_ENDLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddNestGateClearLog(BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biRoomID, bool bClearFlag, INT64 biChracterDBID, BYTE cCharacterJob, BYTE cCharacterLevel)
{
	TQAddNestGateClearLog TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = uiAccountDBID;	
	TxPacket.biRoomID = biRoomID;
	TxPacket.bClearFlag = bClearFlag;
	TxPacket.biChracterDBID = biChracterDBID;
	TxPacket.cCharacterJob = cCharacterJob;
	TxPacket.cCharacterLevel = cCharacterLevel;

	AddSendData( MAINCMD_LOG, QUERY_ADDNESTGATE_CLEARLOG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNDBConnection::QueryNestDeathLog(CDNUserSession *pSession, INT64 biRoomID, int nMonsterID, int nSkillID, BYTE cCharacterJob, BYTE cCharacterLevel)
{
	TQNestDeathLog Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.biRoomID = biRoomID;
	Packet.nMonterID = nMonsterID;
	Packet.nSkillID = nSkillID;
	Packet.cCharacterJob = cCharacterJob;
	Packet.cCharacterLevel = cCharacterLevel;

	AddSendData(MAINCMD_LOG, QUERY_ADDNESTDEATHLOG, (char*)&Packet, sizeof(Packet));
}
#endif
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

//#########################################################################################################################################
// MAINCMD_REPUTATION
//#########################################################################################################################################

void CDNDBConnection::QueryGetListNpcReputation( BYTE cThreadID, CDNUserSession *pSession, int iMapID/*=0*/ )
{
	TQGetListNpcFavor TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iMapID = iMapID;

	AddSendData( MAINCMD_REPUTATION, QUERT_GET_LISTNPCFAVOR, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModNpcReputation( CDNUserSession *pSession, std::vector<TNpcReputation>& Data )
{
	TQModNPCFavor TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_ASSERT( Data.size() > 0 && Data.size() < _countof(TxPacket.UpdateArr) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cCount	= static_cast<BYTE>(Data.size());

	if( TxPacket.cCount > _countof(TxPacket.UpdateArr) )
	{
		_DANGER_POINT();
		TxPacket.cCount = static_cast<BYTE>(_countof(TxPacket.UpdateArr));
	}

	for( UINT i=0 ; i<TxPacket.cCount ; ++i )
		TxPacket.UpdateArr[i] = Data[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.UpdateArr)+(sizeof(TxPacket.UpdateArr[0])*TxPacket.cCount);

	AddSendData( MAINCMD_REPUTATION, QUERY_MOD_NPCFAVOR, reinterpret_cast<char*>(&TxPacket), iSize );
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

//#########################################################################################################################################
// MAINCMD_MASTERSYSTEM
//#########################################################################################################################################

void CDNDBConnection::QueryGetMasterSystemSimpleInfo( BYTE cThreadID, CDNUserSession *pSession, bool bClientSend, MasterSystem::EventType::eCode EventCode )
{
	TQGetMasterPupilInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.bClientSend = bClientSend;
	TxPacket.EventCode = EventCode;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_SIMPLEINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetMasterSystemCountInfo( BYTE cThreadID, CDNUserSession *pSession, bool bClientSend, std::vector<INT64>& biPartyUserList )
{
	TQGetMasterSystemCountInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.bClientSend = bClientSend;
	TxPacket.cCount	= static_cast<BYTE>(biPartyUserList.size());
	if( TxPacket.cCount > _countof(TxPacket.biPartyCharacterDBID) )
	{
		_DANGER_POINT();
		return;
	}

	for( UINT i=0 ; i<biPartyUserList.size() ; ++i )
		TxPacket.biPartyCharacterDBID[i] = biPartyUserList[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.biPartyCharacterDBID)+TxPacket.cCount+sizeof(TxPacket.biPartyCharacterDBID[0]);
	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_COUNTINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPageMasterCharacter( CDNUserSession *pSession, UINT uiPage, BYTE cJob, BYTE cGender )
{
	TQGetPageMasterCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.uiPage	= uiPage;
	TxPacket.cJobCode = cJob;
	TxPacket.cGenderCode = cGender;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_PAGEMASTERCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetMasterCharacterType1( CDNUserSession *pSession, INT64 biMasterCharacterDBID )
{
	TQGetMasterCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.Code = DBDNWorldDef::GetMasterCharacterCode::MyInfo;
	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_MASTERCHARACTER_TYPE1, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetMasterCharacterType2( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID )
{
	TQGetMasterCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.Code = DBDNWorldDef::GetMasterCharacterCode::MyMasterInfo;
	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;
	TxPacket.biPupilCharacterDBID = biPupilCharacterDBID;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_MASTERCHARACTER_TYPE2, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPupilList( CDNUserSession *pSession, INT64 biMasterCharacterDBID )
{
	TQGetListPupil TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = biMasterCharacterDBID;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_PUPILLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterMasterCharacter( CDNUserSession *pSession, WCHAR* pwszText )
{
	TQAddMasterCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	_wcscpy( TxPacket.wszSelfIntroduction, _countof(TxPacket.wszSelfIntroduction), pwszText, (int)wcslen(pwszText) );
	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_REGISTER_MASTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterCancelMasterCharacter( CDNUserSession *pSession )
{
	TQDelMasterCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_REGISTERCANCEL_MASTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryJoinMasterSystem( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, bool bIsDirectMenu, DBDNWorldDef::TransactorCode::eCode Code, bool bIsSkip/*=false*/ )
{
	TQAddMasterAndPupil TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;
	TxPacket.biPupilCharacterDBID = biPupilCharacterDBID;
	TxPacket.bCheckRegisterFlag	= (bIsDirectMenu ? false : true);
	TxPacket.bIsDirectMenu = bIsDirectMenu;
	TxPacket.Code = Code;
#if defined( _FINAL_BUILD )
	TxPacket.bIsSkip = false;
#else
	TxPacket.bIsSkip = bIsSkip;
#endif // #if defined( _FINAL_BUILD )

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_JOIN_MASTERSYSTEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetMasterAndClassmate( CDNUserSession *pSession, INT64 biCharacterDBID )
{
	TQGetListMyMasterAndClassmate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = biCharacterDBID;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_MASTERANDCLASSMATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetClassmateInfo( CDNUserSession *pSession, INT64 biClassmateCharacterDBID )
{
	TQGetMyClassmate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.biClassmateCharacterDBID = biClassmateCharacterDBID;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GET_CLASSMATEINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryLeaveMasterSystem( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, DBDNWorldDef::TransactorCode::eCode Code, int iPenaltyDay, int iPenaltyRespectPoint, bool bIsSkip/*=false*/ )
{
	TQDelMasterAndPupil TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;
	TxPacket.biPupilCharacterDBID = biPupilCharacterDBID;
	TxPacket.Code = Code;
	TxPacket.iPenaltyDay = iPenaltyDay;
	TxPacket.iPenaltyRespectPoint = iPenaltyRespectPoint;
#if defined( _FINAL_BUILD )
	TxPacket.bIsSkip = false;
#else
	TxPacket.bIsSkip = bIsSkip;
#endif // #if defined( _FINAL_BUILD )

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_LEAVE_MASTERSYSTEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryMasterSystemGraduate( CDNUserSession *pSession )
{
	TQGraduate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_GRADUATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModMasterSystemRespectPoint( CDNUserSession *pSession, int iRespectPoint )
{
	TQModRespectPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iRespectPoint = iRespectPoint;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_MOD_RESPECTPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModMasterSystemFavorPoint( CDNUserSession *pSession, INT64 biMasterCharacterDBID, INT64 biPupilCharacterDBID, int iFavorPoint )
{
	TQModMasterFavorPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;
	TxPacket.biPupilCharacterDBID = biPupilCharacterDBID;
	TxPacket.iFavorPoint = iFavorPoint;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_MOD_FAVORPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModMasterSystemGraduateCount( CDNUserSession *pSession, int iGraduateCount )
{
	TQModMasterSystemGraduateCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iGraduateCount	= iGraduateCount;

	AddSendData( MAINCMD_MASTERSYSTEM, QUERY_MOD_GRADUATECOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_SECONDARY_SKILL )

void CDNDBConnection::QueryAddSecondarySkill( CDNUserSession *pSession, int iSkillID, SecondarySkill::Type::eType Type )
{
	TQAddSecondarySkill TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iSecondarySkillID = iSkillID;
	TxPacket.Type = Type;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_ADD_SECONDARYSKILL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDeleteSecondarySkill( CDNUserSession *pSession, int iSkillID )
{
	TQDelSecondarySkill TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iSecondarySkillID = iSkillID;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_DELETE_SECONDARYSKILL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetListSecondarySkill( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetListSecondarySkill TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_GETLIST_SECONDARYSKILL, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModSecondarySkillExp( CDNUserSession *pSession, int iSkillID, int iExp, int iExpAfter )
{
	TQModSecondarySkillExp TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iSecondarySkillID = iSkillID;
	TxPacket.iSecondarySkillExp = iExp;
	TxPacket.iSecondarySkillExpAfter = iExpAfter;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_MOD_SECONDARYSKILL_EXP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddSecondarySkillRecipe( CDNUserSession *pSession, bool bIsCash, INT64 biItemSerial, int iSkillID )
{
	TQSetManufactureSkillRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.LocationCode = bIsCash ? DBDNWorldDef::ItemLocation::CashInventory : DBDNWorldDef::ItemLocation::Inventory;
	TxPacket.biItemSerial = biItemSerial;
	TxPacket.iSecondarySkillID = iSkillID;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_ADD_SECONDARYSKILL_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDeleteSecondarySkillRecipe( CDNUserSession *pSession, INT64 biItemSerial )
{
	TQDelManufactureSkillRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.biItemSerial = biItemSerial;
	if( pSession->GetIp() )
		_strcpy( TxPacket.szIP, _countof(TxPacket.szIP), pSession->GetIp(), static_cast<int>(strlen(pSession->GetIp())) );

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_DELETE_SECONDARYSKILL_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryExtractSecondarySkillRecipe( CDNUserSession *pSession, INT64 biItemSerial, DBDNWorldDef::ItemLocation::eCode Code )
{
	TQExtractManufactureSkillRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.biItemSerial = biItemSerial;
	TxPacket.LocationCode = Code;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_EXTRACT_SECONDARYSKILL_RECIPE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModSecondarySkillRecipeExp( CDNUserSession *pSession, INT64 biItemSerial, short nAddExp, short nExpAfter )
{
	TQModManufactureSkillRecipe TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.biItemSerial = biItemSerial;
	TxPacket.nDurability = nAddExp;
	TxPacket.nDurabilityAfter = nExpAfter;

	AddSendData( MAINCMD_SECONDARYSKILL, QUERY_MOD_SECONDARYSKILL_RECIPE_EXP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )


void CDNDBConnection::QueryFarmList(BYTE cThreadID, int iWorldID)
{
	TQGetListFarm TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.cEnableFlag	= 1;		//Get only the enabled ones
	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID	= iWorldID;

	AddSendData(MAINCMD_FARM, QUERY_GETLIST_FARM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryGetListField( BYTE cThreadID, int iWorldID, int iRoomID, int iFarmDBID )
{
	TQGetListField TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.iRoomID		= iRoomID;
	TxPacket.iFarmDBID		= iFarmDBID;

	AddSendData( MAINCMD_FARM, QUERY_GETLIST_FIELD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetListFieldByCharacter(CDNUserSession *pSession, bool bRefreshGate)
{
	TQGetListFieldByCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.bRefreshGate = bRefreshGate;

	AddSendData( MAINCMD_FARM, QUERY_GETLIST_FIELD_BYCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetCountHarvestDepotItem( CDNUserSession *pSession )
{
	TQGetCountHarvestDepotItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_FARM, QUERY_GETCOUNT_HARVESTDEPOTITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( _GAMESERVER )

void CDNDBConnection::QueryAddField( BYTE cThreadID, CDNUserSession *pSession, CGrowingArea* pArea, int iMaxFieldCount, BYTE cInvenIndex, INT64 biItemSerial, const CSFarmPlant* pPacket )
{
	if( pArea == NULL || pArea->GetPlantSeed() == NULL )
	{
		_ASSERT( 0 );
		return;
	}

	CPlantSeed* pSeed = pArea->GetPlantSeed();

	TQAddField TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pArea->GetFarmGameRoom()->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.iRoomID = pArea->GetFarmGameRoom()->GetRoomID();
	TxPacket.iFarmDBID = pArea->GetFarmGameRoom()->GetFarmIndex();
	TxPacket.nFieldIndex = pArea->GetIndex();
	TxPacket.biCharacterDBID = pSeed->GetOwnerCharacterDBID();
	TxPacket.iItemID = pSeed->GetSeedItemID();
	TxPacket.iAttachItemID = pSeed->GetFirstAttachItemID();
	TxPacket.iMaxFieldCount = iMaxFieldCount;
	// Info for removing seed
	TxPacket.cInvenIndex = cInvenIndex;
	TxPacket.biSeedItemSerial = biItemSerial;
	TxPacket.cAttachCount = pPacket->cCount;
	TxPacket.iMapID	 = pSession->GetMapIndex();
	for( int i=0 ; i<pPacket->cCount ; ++i )
		TxPacket.AttachItems[i]	= pPacket->AttachItems[i];
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pSession->GetIpW(), (int)wcslen(pSession->GetIpW()) );

	AddSendData( MAINCMD_FARM, QUERY_ADD_FIELD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDelField( BYTE cThreadID, int iWorldID, int iFarmDBID, int iRoomID, int iAreaIndex )
{
	TQDelField TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID	= iWorldID;
	TxPacket.iFarmDBID		= iFarmDBID;
	TxPacket.iRoomID		= iRoomID;
	TxPacket.nFieldIndex	= iAreaIndex;

	AddSendData( MAINCMD_FARM, QUERY_DEL_FIELD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddFieldAttachment( BYTE cThreadID, CDNUserSession *pSession, int iFarmDBID, int iRoomID, int iAreaIndex, int iAttachItemID, BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, bool bVirtualAttach/*=false*/ )
{
	TQAddFieldAttachment TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= pSession->GetWorldSetID();
	TxPacket.nAccountDBID		= pSession->GetAccountDBID();
	TxPacket.biCharacterDBID	= pSession->GetCharacterDBID();
	TxPacket.iRoomID			= iRoomID;
	TxPacket.iFarmDBID			= iFarmDBID;
	TxPacket.nFieldIndex		= iAreaIndex;
	TxPacket.iAttachItemID		= iAttachItemID;
	// Info for removing AttachItem
#if defined( PRE_ADD_VIP_FARM )
	TxPacket.bVirtualAttach		= bVirtualAttach;
#endif // #if defined( PRE_ADD_VIP_FARM )
	TxPacket.cInvenType			= cInvenType;
	TxPacket.cInvenIndex		= cInvenIndex;
	TxPacket.biAttachItemSerial	= biItemSerial;
	TxPacket.iMapID				= pSession->GetMapIndex();
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pSession->GetIpW(), (int)wcslen(pSession->GetIpW()) );

	AddSendData( MAINCMD_FARM, QUERY_ADD_FIELD_ATTACHMENT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModFieldElapsedTime( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iFarmDBID, std::vector<int>& vAreaIndex, std::vector<int>& vElapsedTimeSec )
{
	_ASSERT( vAreaIndex.size() > 0 );
	_ASSERT( vAreaIndex.size() < Farm::Max::FIELDCOUNT );
	_ASSERT( vAreaIndex.size() == vElapsedTimeSec.size() );

	TQModFieldElapsedTime TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID	= uiAccountDBID;
	TxPacket.iFarmDBID		= iFarmDBID;
	TxPacket.cCount			= static_cast<BYTE>(vAreaIndex.size());
	for( UINT i=0 ; i<vAreaIndex.size() ; ++i )
	{
		TxPacket.Updates[i].nFieldIndex		= vAreaIndex[i];
		TxPacket.Updates[i].iElapsedTimeSec	= vElapsedTimeSec[i];
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Updates)+TxPacket.cCount*sizeof(TxPacket.Updates[0]);
	AddSendData( MAINCMD_FARM, QUERY_MOD_FIELD_ELAPSEDTIME, reinterpret_cast<char*>(&TxPacket), iSize );

}

void CDNDBConnection::QueryHarvest( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iFarmDBID, int iRoomID, int iAreaIndex, INT64 biOwnerCharacterDBID, INT64 biCharacterDBID, std::vector<TItem>& vItem, int iMapID, const char* pszIP )
{
	_ASSERT( vItem.size() > 0 && vItem.size() <= Farm::Max::HARVESTITEM_COUNT );
	if( vItem.size() == 0 || vItem.size() > Farm::Max::HARVESTITEM_COUNT )
		return;

	TQHarvest TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID			= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.iFarmDBID				= iFarmDBID;
	TxPacket.iRoomID				= iRoomID;
	TxPacket.nFieldIndex			= iAreaIndex;
	TxPacket.biCharacterDBID		= biOwnerCharacterDBID;
	TxPacket.biHarvestCharacterDBID	= biCharacterDBID;
	TxPacket.iMapID					= iMapID;
	_strcpy( TxPacket.szIP, _countof(TxPacket.szIP), pszIP, (int)strlen(pszIP) );

	for( UINT i=0 ; i<vItem.size() ; ++i )
	{
		TxPacket.HarvestItems[i].biItemSerial		= vItem[i].nSerial;
		TxPacket.HarvestItems[i].iItemID			= vItem[i].nItemID;
		TxPacket.HarvestItems[i].nItemCount			= vItem[i].wCount;
		TxPacket.HarvestItems[i].bSoulBoundFlag		= vItem[i].bSoulbound;
		TxPacket.HarvestItems[i].cSealCount			= vItem[i].cSealCount;
		TxPacket.HarvestItems[i].cItemOption		= vItem[i].cOption;
		TxPacket.HarvestItems[i].bEternityFlag		= vItem[i].bEternity;
		TxPacket.HarvestItems[i].iLifeSpan			= 0;
	}

	AddSendData( MAINCMD_FARM, QUERY_HARVEST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetListHarvestDepotItem( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, INT64 biLastUniqueID )
{
	TQGetListHarvestDepotItem TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.biLastUniqueID		= biLastUniqueID;

	AddSendData( MAINCMD_FARM, QUERY_GETLIST_HARVESTDEPOTITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetFieldCountByCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsSend )
{
	TQGetFieldCountByCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.bIsSend			= bIsSend;

	AddSendData( MAINCMD_FARM, QUERY_GETFIELDCOUNT_BYCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetFieldItemCount( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iItemID, int iRoomID, int iAreaIndex, UINT nSessionID )
{
	TQGetFieldItemCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.nSessionID = nSessionID;
	TxPacket.iItemID			= iItemID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.iAreaIndex			= iAreaIndex;

	AddSendData( MAINCMD_FARM, QUERY_GET_FIELDITEMCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetListFieldForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID )
{
	TQGetListFieldForCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.biCharacterDBID	= biCharacterDBID;

	AddSendData( MAINCMD_FARM, QUERY_GETLIST_FIELD_FORCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddFieldForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, CGrowingArea* pArea, int iMaxFieldCount, BYTE cInvenIndex, INT64 biSeedItemSerial, const CSFarmPlant* pPacket, int iMapID, const WCHAR* pwszIP )
{
	if( pArea == NULL || pArea->GetPlantSeed() == NULL )
	{
		_ASSERT( 0 );
		return;
	}

	CPlantSeed* pSeed = pArea->GetPlantSeed();

	TQAddField TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.nFieldIndex		= pArea->GetIndex();
	TxPacket.biCharacterDBID	= pSeed->GetOwnerCharacterDBID();
	TxPacket.iItemID			= pSeed->GetSeedItemID();
	TxPacket.iAttachItemID		= pSeed->GetFirstAttachItemID();
	TxPacket.iMaxFieldCount		= iMaxFieldCount;
	// Info for removing seed
	TxPacket.cInvenIndex		= cInvenIndex;
	TxPacket.biSeedItemSerial	= biSeedItemSerial;
	TxPacket.cAttachCount		= pPacket->cCount;
	TxPacket.iMapID				= iMapID;
	for( int i=0 ; i<pPacket->cCount ; ++i )
		TxPacket.AttachItems[i]	= pPacket->AttachItems[i];
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pwszIP, (int)wcslen(pwszIP) );

	AddSendData( MAINCMD_FARM, QUERY_ADD_FIELD_FORCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryDelFieldForCharacter( BYTE cThreadID, int iWorldID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex )
{
	TQDelFieldForCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.nFieldIndex		= iAreaIndex;

	AddSendData( MAINCMD_FARM, QUERY_DEL_FIELD_FORCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_VIP_FARM )
void CDNDBConnection::QueryAddFieldForCharacterAttachment( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex, int iAttachItemID, 
														   BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, int iMapID, const WCHAR* pwszIP, bool bVirtualAttach/*=false*/ )
#else
void CDNDBConnection::QueryAddFieldForCharacterAttachment( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, int iRoomID, int iAreaIndex, int iAttachItemID, 
														   BYTE cInvenType, BYTE cInvenIndex, INT64 biItemSerial, int iMapID, const WCHAR* pwszIP )
#endif // #if defined( PRE_ADD_VIP_FARM )
{
	TQAddFieldAttachment TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.nFieldIndex		= iAreaIndex;
	TxPacket.iAttachItemID		= iAttachItemID;
	// Info for removing AttachItem
#if defined( PRE_ADD_VIP_FARM )
	TxPacket.bVirtualAttach		= bVirtualAttach;
#endif // #if defined( PRE_ADD_VIP_FARM )
	TxPacket.cInvenType			= cInvenType;
	TxPacket.cInvenIndex		= cInvenIndex;
	TxPacket.biAttachItemSerial	= biItemSerial;
	TxPacket.iMapID				= iMapID;
	_wcscpy( TxPacket.wszIP, _countof(TxPacket.wszIP), pwszIP, (int)wcslen(pwszIP) );

	AddSendData( MAINCMD_FARM, QUERY_ADD_FIELD_FORCHARACTER_ATTACHMENT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModFieldForCharacterElapsedTime( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, std::vector<int>& vAreaIndex, std::vector<int>& vElapsedTimeSec )
{
	_ASSERT( vAreaIndex.size() > 0 );
	_ASSERT( vAreaIndex.size() < Farm::Max::PRIVATEFIELDCOUNT );
	_ASSERT( vAreaIndex.size() == vElapsedTimeSec.size() );

	TQModFieldForCharacterElapsedTime TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID			= cThreadID;
	TxPacket.cWorldSetID		= iWorldID;
	TxPacket.nAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.cCount				= static_cast<BYTE>(vAreaIndex.size());
	for( UINT i=0 ; i<vAreaIndex.size() ; ++i )
	{
		TxPacket.Updates[i].nFieldIndex		= vAreaIndex[i];
		TxPacket.Updates[i].iElapsedTimeSec	= vElapsedTimeSec[i];
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Updates)+TxPacket.cCount*sizeof(TxPacket.Updates[0]);
	AddSendData( MAINCMD_FARM, QUERY_MOD_FIELD_FORCHARACTER_ELAPSEDTIME, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNDBConnection::QueryHarvestForCharacter( BYTE cThreadID, int iWorldID, UINT uiAccountDBID, int iRoomID, int iAreaIndex, INT64 biOwnerCharacterDBID, INT64 biCharacterDBID, std::vector<TItem>& vItem, int iMapID, const char* pszIP )
{
	_ASSERT( vItem.size() > 0 && vItem.size() <= Farm::Max::HARVESTITEM_COUNT );
	if( vItem.size() == 0 || vItem.size() > Farm::Max::HARVESTITEM_COUNT )
		return;

	TQHarvest TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID				= cThreadID;
	TxPacket.cWorldSetID			= iWorldID;
	TxPacket.nAccountDBID			= uiAccountDBID;
	TxPacket.iRoomID				= iRoomID;
	TxPacket.nFieldIndex			= iAreaIndex;
	TxPacket.biCharacterDBID		= biOwnerCharacterDBID;
	TxPacket.biHarvestCharacterDBID	= biCharacterDBID;
	TxPacket.iMapID					= iMapID;
	_strcpy( TxPacket.szIP, _countof(TxPacket.szIP), pszIP, (int)strlen(pszIP) );

	for( UINT i=0 ; i<vItem.size() ; ++i )
	{
		TxPacket.HarvestItems[i].biItemSerial		= vItem[i].nSerial;
		TxPacket.HarvestItems[i].iItemID			= vItem[i].nItemID;
		TxPacket.HarvestItems[i].nItemCount			= vItem[i].wCount;
		TxPacket.HarvestItems[i].bSoulBoundFlag		= vItem[i].bSoulbound;
		TxPacket.HarvestItems[i].cSealCount			= vItem[i].cSealCount;
		TxPacket.HarvestItems[i].cItemOption		= vItem[i].cOption;
		TxPacket.HarvestItems[i].bEternityFlag		= vItem[i].bEternity;
		TxPacket.HarvestItems[i].iLifeSpan			= 0;
	}

	AddSendData( MAINCMD_FARM, QUERY_HARVEST_FORCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif // #if defined( _GAMESERVER )

void CDNDBConnection::QueryDeletePeriodQuest( CDNUserSession *pSession, int nPeriodQuestType, std::vector<int>& vRemoveQuestIDs )
{
	TQDeletePeriodQuest TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nPeriodQuestType = nPeriodQuestType;
	TxPacket.nQuestCount = (int)vRemoveQuestIDs.size();
	int idx = 0;

	for( std::vector<int>::iterator i=vRemoveQuestIDs.begin(); i != vRemoveQuestIDs.end(); i++ )
	{
		if (idx >= MAX_PERIODQUEST_LIST )
			break;

		TxPacket.nQuestIDs[idx++] = (*i);
	}

	int nLen = sizeof(TQDeletePeriodQuest) - sizeof(TxPacket.nQuestIDs) +(sizeof(int) * TxPacket.nQuestCount);
	AddSendData( MAINCMD_QUEST, QUERY_DELETE_CHARACTER_PERIODQUEST, reinterpret_cast<char*>(&TxPacket), nLen );
}

void CDNDBConnection::QueryGetPeriodQuestDate( CDNUserSession *pSession )
{
	TQGetPeriodQuestDate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_QUEST, QUERY_GET_CHARACTER_PERIODQUESTDATE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

bool CDNDBConnection::QueryAddPvPLadderResult( CDNUserSession *pSession, PvPCommon::QueryUpdatePvPDataType::eCode Type, LadderSystem::MatchType::eCode MatchType, int iGradePoint, int iHiddenGradePoint, LadderSystem::MatchResult::eCode Result, BYTE cVSJobCode, const std::vector<LadderKillResult>& vData )
{
	TQAddPvPLadderResult TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	if( vData.size() > _countof(TxPacket.KillDeathCounts) )
	{
		_ASSERT(0);
		return false;
	}

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.Type = Type;
	TxPacket.cPvPLadderCode = static_cast<BYTE>(MatchType);
	TxPacket.iPvPLadderGradePoint = iGradePoint;
	TxPacket.iHiddenPvPLadderGradePoint	= iHiddenGradePoint;
	TxPacket.cResult = static_cast<BYTE>(Result);
	TxPacket.cVersusCharacterJobCode = cVSJobCode;
	TxPacket.cKillDeathCount = static_cast<BYTE>(vData.size());
	for( int i=0 ; i<TxPacket.cKillDeathCount ; ++i )
		TxPacket.KillDeathCounts[i] = vData[i];

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.KillDeathCounts)+TxPacket.cKillDeathCount*sizeof(TxPacket.KillDeathCounts[0]);
	return(AddSendData( MAINCMD_PVP, QUERY_ADD_PVP_LADDERRESULT, reinterpret_cast<char*>(&TxPacket), iSize ) == 0);
}

void CDNDBConnection::QueryGetListPvPLadderScore( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetListPvPLadderScore TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_PVP, QUERY_GETLIST_PVP_LADDERSCORE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetListPvPLadderScoreByJob( CDNUserSession *pSession )
{
	TQGetListPvPLadderScoreByJob TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_PVP, QUERY_GETLIST_PVP_LADDERSCORE_BYJOB, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryInitPvPLadderGradePoint( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int iAddLadderPoint, int iLadderGradePoint )
{
	TQInitPvPLadderGradePoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cPvPLadderCode	= static_cast<BYTE>(MatchType);
	TxPacket.iPvPLadderPoint = iAddLadderPoint;
	TxPacket.iPvPLadderGradePoint = iLadderGradePoint;
	if( pSession->GetIp() )
		_strcpy( TxPacket.szIP, _countof(TxPacket.szIP), pSession->GetIp(), (int)strlen(pSession->GetIp()) );

	AddSendData( MAINCMD_PVP, QUERY_INIT_PVP_LADDERGRADEPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryUsePvPLadderPoint( CDNUserSession *pSession, int iUseLadderPoint )
{
	TQUsePvPLadderPoint TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.iPvPLadderPoint = iUseLadderPoint;
	TxPacket.iAfterLadderPoint = pSession->GetPvPLadderScoreInfoPtr()->iPvPLadderPoint;
	TxPacket.iMapID	= pSession->GetMapIndex();
	if( pSession->GetIp() )
		_strcpy( TxPacket.szIP, _countof(TxPacket.szIP), pSession->GetIp(), (int)strlen(pSession->GetIp()) );

	AddSendData( MAINCMD_PVP, QUERY_USE_PVP_LADDERPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddPvPLadderCUCount( BYTE cThreadID, int iWorldID, LadderSystem::MatchType::eCode MatchType, int iCUCount )
{
	TQAddPvPLadderCUCount TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID		= cThreadID;
	TxPacket.cWorldSetID	= iWorldID;
	TxPacket.cPvPLadderCode	= static_cast<BYTE>(MatchType);
	TxPacket.iCUCount		= iCUCount;

	AddSendData( MAINCMD_PVP, QUERY_ADD_PVP_LADDERCUCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryModPvPLadderScoresForCheat( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int iPvPLadderPoint, int iPvPLadderGradePoint, int iHiddenPvPLadderGradePoint )
{
	TQModPvPLadderScoresForCheat TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cPvPLadderCode = static_cast<BYTE>(MatchType);
	TxPacket.iPvPLadderPoint = iPvPLadderPoint;
	TxPacket.iPvPLadderGradePoint = iPvPLadderGradePoint;
	TxPacket.iHiddenPvPLadderGradePoint	= iHiddenPvPLadderGradePoint;

	AddSendData( MAINCMD_PVP, QUERY_MOD_PVP_LADDERSCORE_FORCHEAT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPLadderRankBoard( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType )
{
	TQGetListPvPLadderRanking TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.cPvPLadderCode	= static_cast<BYTE>(MatchType);

	AddSendData( MAINCMD_PVP, QUERY_GETLIST_PVPLADDERRANKING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined(PRE_ADD_PVP_RANKING)
void CDNDBConnection::QueryGetPvPRankBoard( CDNUserSession *pSession )
{
	TQGetPvPRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_RANKBOARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPRankList( CDNUserSession *pSession, int nPage, BYTE cClassCode, BYTE cSubClassCode, const WCHAR* wszGuildName )
{
	TQGetPvPRankList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.iPage = nPage;
	TxPacket.cClassCode = cClassCode;
	TxPacket.cSubClassCode = cSubClassCode;
	
	if(wszGuildName != NULL)
		_wcscpy( TxPacket.wszGuildName, _countof(TxPacket.wszGuildName), wszGuildName, static_cast<int>(wcslen(wszGuildName)) );

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_RANKLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPRankInfo( CDNUserSession *pSession, const WCHAR* wszCharName )
{
	TQGetPvPRankInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();

	if(wszCharName != NULL)
		_wcscpy(TxPacket.wszCharName, _countof(TxPacket.wszCharName), wszCharName, static_cast<int>(wcslen(wszCharName)) );

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_RANKINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPLadderRankBoard2( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType )
{
	TQGetPvPLadderRankBoard TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.cPvPLadderCode = static_cast<BYTE>(MatchType);
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_LADDER_RANKBOARD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPLadderRankList( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, int nPage, BYTE cClassCode, BYTE cSubClassCode, const WCHAR* wszGuildName )
{
	TQGetPvPLadderRankList TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.cPvPLadderCode = static_cast<BYTE>(MatchType);
	TxPacket.iPage = nPage;
	TxPacket.cClassCode = cClassCode;
	TxPacket.cSubClassCode = cSubClassCode;

	if(wszGuildName != NULL)
		_wcscpy( TxPacket.wszGuildName, _countof(TxPacket.wszGuildName), wszGuildName, static_cast<int>(wcslen(wszGuildName)) );

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_LADDER_RANKLIST, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetPvPLadderRankInfo( CDNUserSession *pSession, LadderSystem::MatchType::eCode MatchType, const WCHAR* wszCharName )
{
	TQGetPvPLadderRankInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.cPvPLadderCode = static_cast<BYTE>(MatchType);

	if(wszCharName != NULL)
		_wcscpy(TxPacket.wszCharName, _countof(TxPacket.wszCharName), wszCharName, static_cast<int>(wcslen(wszCharName)) );

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_LADDER_RANKINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAggregatePvPRank(CDNUserSession *pSession, bool bIsPvPLadder)
{
	TQForceAggregatePvPRank TxPacket;
	memset(&TxPacket, 0, sizeof(TQForceCalcPvPRank));

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.bIsPvPLadder = bIsPvPLadder;

	AddSendData( MAINCMD_PVP, QUERY_FORCE_AGGREGATE_PVP_RANKING, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif	//#if defined(PRE_ADD_PVP_RANKING)

void CDNDBConnection::QueryGetPVPGhoulScores( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetPVPGhoulScores TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket));

	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_PVP, QUERY_GET_PVP_GHOULSCORES, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAddPVPGhoulScores( CDNUserSession *pSession )
{
	TQAddPVPGhoulScores TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket));

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGhoulWin = pSession->GetAddGhoulScore(GhoulMode::PointType::GhoulWin);
	TxPacket.nHumanWin = pSession->GetAddGhoulScore(GhoulMode::PointType::HumanWin);
	TxPacket.nTimeOver = pSession->GetAddGhoulScore(GhoulMode::PointType::TimeOver);
	TxPacket.nGhoulKill = pSession->GetAddGhoulScore(GhoulMode::PointType::GhoulKill);
	TxPacket.nHumanKill = pSession->GetAddGhoulScore(GhoulMode::PointType::HumanKill);
	TxPacket.nHolyWaterUse = pSession->GetAddGhoulScore(GhoulMode::PointType::HolyWaterUse);

	AddSendData( MAINCMD_PVP, QUERY_ADD_PVP_GHOULSCORES, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined(PRE_PVP_GAMBLEROOM )
void CDNDBConnection::QueryAddTournamentResult(BYTE cThreadID, int nWorldSetID, STournamentMatchInfo* pTournamentMatchInfo, INT64 nGableRoomDBID)
#else // #if defined(PRE_PVP_GAMBLEROOM )
void CDNDBConnection::QueryAddTournamentResult(BYTE cThreadID, int nWorldSetID, STournamentMatchInfo* pTournamentMatchInfo)
#endif // #if defined(PRE_PVP_GAMBLEROOM )
{
	TQAddPVPTournamentResult TxPacket;
	memset(&TxPacket, 0, sizeof(TQAddPVPTournamentResult));
	TxPacket.cThreadID = cThreadID;
	TxPacket.cWorldSetID = nWorldSetID;
	for(int i=0; i<PvPCommon::Common::PvPTournamentUserMax; ++i)
	{
		if( pTournamentMatchInfo[i].sTournamentUserInfo.uiAccountDBID>0)
		{
			TxPacket.biCharacterDBID[i] = pTournamentMatchInfo[i].sTournamentUserInfo.biCharacterDBID;
			TxPacket.cTournamentStep[i] = pTournamentMatchInfo[i].cTournamentStep;
			TxPacket.bWin[i] = pTournamentMatchInfo[i].bWin;
			++TxPacket.nTotalCount;
		}
		else
			break;
	}

#if defined( PRE_PVP_GAMBLEROOM )
	TxPacket.nGambleDBID = nGableRoomDBID;
#endif
	AddSendData( MAINCMD_PVP, QUERY_ADD_PVP_TOURNAMENTRESULT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)

void CDNDBConnection::QueryGetPageGuildRecruit( CDNUserSession *pSession, UINT uiPage, BYTE cJob, BYTE cLevel, BYTE cPurposeCode, WCHAR* pwszGuildName, BYTE cSortType )
{
	TQGetGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.uiPage = uiPage;
	TxPacket.cJobCode = cJob;	
	TxPacket.cLevel	= cLevel;
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	TxPacket.cPurposeCode = cPurposeCode;
	if (pwszGuildName)
		_wcscpy( TxPacket.wszGuildName, _countof(TxPacket.wszGuildName), pwszGuildName, (int)wcslen(pwszGuildName) );
	TxPacket.cSortType = cSortType;
#endif

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_GET_PAGEGUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetGuildRecruitCharacter( CDNUserSession *pSession, UINT nGuildDBID)
{
	TQGetGuildRecruitCharacter TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.nGuildDBID	= nGuildDBID;	

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_GET_PAGEGUILDRECRUITCHARACTER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetMyGuildRecruit( CDNUserSession *pSession )
{
	TQGetMyGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_GET_MYGUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryGetGuildRecruitRequestCount( CDNUserSession *pSession )
{
	TQGetMyGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_GET_GUILDRECRUIT_REQUESTCOUNT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterOnGuildRecruit( CDNUserSession *pSession, WCHAR* pwszText, BYTE* cClassCode, int nMinLevel, int nMaxLevel, BYTE cPurposeCode, bool bCheckHomePage )
{
	TQRegisterOnGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.nGuildDBID = pSession->GetGuildUID().nDBID;
	TxPacket.nMinLevel = nMinLevel;
	TxPacket.nMaxLevel = nMaxLevel;
	if (pwszText)
		_wcscpy( TxPacket.wszGuildIntroduction, _countof(TxPacket.wszGuildIntroduction), pwszText, (int)wcslen(pwszText) );
	if (cClassCode)
		memcpy(TxPacket.cClassCode, cClassCode, sizeof(TxPacket.cClassCode));

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	TxPacket.cPurposeCode = cPurposeCode;
	TxPacket.bCheckHomePage = bCheckHomePage;
#endif

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REGISTERON_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterInfoGuildRecruit( CDNUserSession *pSession )
{
	TQRegisterInfoGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.nGuildDBID = pSession->GetGuildUID().nDBID;

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REGISTERINFO_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterModGuildRecruit( CDNUserSession *pSession, WCHAR* pwszText, BYTE* cClassCode, int nMinLevel, int nMaxLevel, BYTE cPurposeCode, bool bCheckHomePage )
{
	TQRegisterOnGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.nGuildDBID = pSession->GetGuildUID().nDBID;
	TxPacket.nMinLevel = nMinLevel;
	TxPacket.nMaxLevel = nMaxLevel;
	if (pwszText)
		_wcscpy( TxPacket.wszGuildIntroduction, _countof(TxPacket.wszGuildIntroduction), pwszText, (int)wcslen(pwszText) );
	if (cClassCode)
		memcpy(TxPacket.cClassCode, cClassCode, sizeof(TxPacket.cClassCode));

#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	TxPacket.cPurposeCode = cPurposeCode;
	TxPacket.bCheckHomePage = bCheckHomePage;
#endif

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REGISTERMOD_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRegisterOffGuildRecruit( CDNUserSession *pSession )
{
	TQRegisterOnGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.nGuildDBID = pSession->GetGuildUID().nDBID;

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REGISTEROFF_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRequestOnGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID )
{
	TQGuildRecruitRequestOn TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGuildDBID = nGuildDBID;	

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REQUESTON_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryRequestOffGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID )
{
	TQGuildRecruitRequestOff TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGuildDBID	= nGuildDBID;	

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_REQUESTOFF_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAcceptOnGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, INT64 biAcceptCharacterDBID, short wGuildSize, WCHAR* pwszCharacterName )
{
	TQGuildRecruitAcceptOn TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGuildDBID = nGuildDBID;
	TxPacket.biAcceptCharacterDBID = biAcceptCharacterDBID;
	TxPacket.wGuildSize	= wGuildSize;	
	_wcscpy( TxPacket.wszToCharacterName, _countof(TxPacket.wszToCharacterName), pwszCharacterName, (int)wcslen(pwszCharacterName) );

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_ACCEPTON_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNDBConnection::QueryAcceptOffGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, INT64 biDenyCharacterDBID, WCHAR* pwszCharacterName )
{
	TQGuildRecruitAcceptOff TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGuildDBID	= nGuildDBID;	
	TxPacket.biDenyCharacterDBID = biDenyCharacterDBID;	
	_wcscpy( TxPacket.wszToCharacterName, _countof(TxPacket.wszToCharacterName), pwszCharacterName, (int)wcslen(pwszCharacterName) );

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_ACCEPTOFF_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNDBConnection::QueryCheatDelGuildRecruit( CDNUserSession *pSession, UINT nGuildDBID, BYTE DelType )
{
	TQDelGuildRecruit TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nGuildDBID	= nGuildDBID;	
	TxPacket.nDelType = DelType;

	AddSendData( MAINCMD_GUILDRECRUIT, QUERY_DELETE_GUILDRECRUIT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined (PRE_ADD_DONATION)
void CDNDBConnection::QueryDonate(CDNUserSession *pSession, time_t tDonationTime, INT64 nCoin)
{
	TQDonate packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();
	packet.tDonationTime = tDonationTime;
	packet.nCoin = nCoin;

	AddSendData(MAINCMD_DONATION, QUERY_DONATE, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNDBConnection::QueryDonationRanking(CDNUserSession *pSession)
{
	TQDonationRanking packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.cThreadID = pSession->GetDBThreadID();
	packet.cWorldSetID = pSession->GetWorldSetID();
	packet.nAccountDBID = pSession->GetAccountDBID();
	packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_DONATION, QUERY_DONATION_RANKING, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNDBConnection::QueryDonationTopRanker(BYTE cThreadID, char cWorldID)
{
	TQDonationTopRanker packet;
	ZeroMemory(&packet, sizeof(packet));
	packet.cThreadID = cThreadID;
	packet.cWorldSetID = cWorldID;

	AddSendData(MAINCMD_DONATION, QUERY_DONATION_TOP_RANKER, reinterpret_cast<char*>(&packet), sizeof(packet));
}
#endif // #if defined (PRE_ADD_DONATION)

#if defined( PRE_PARTY_DB )

void CDNDBConnection::QueryDelPartyForServer( int iWorldID, int iServerID )
{
	TQDelPartyForServer TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.iServerID = iServerID;

	AddSendData( MAINCMD_PARTY, QUERY_DELPARTY_FORSERVER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#if defined( _GAMESERVER )
void CDNDBConnection::QueryDelPartyForGameServer( int iWorldID, int iServerID )
{
	TQDelPartyForServer TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.iServerID = iServerID;

	AddSendData( MAINCMD_PARTY, QUERY_DELPARTY_FORGAMESERVER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif

void CDNDBConnection::QueryGetListParty( int iWorldID )
{
	TQGetListParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.iExecptServerID = g_Config.nManagedID;

	AddSendData( MAINCMD_PARTY, QUERY_GETLISTPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#if defined(PRE_FIX_62281)
void CDNDBConnection::QueryAddParty( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSCreateParty* pPacket, const WCHAR * wszAfterInviteCharacterName/*=NULL*/, bool bCheat/*=false*/ )
#else
void CDNDBConnection::QueryAddParty( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSCreateParty* pPacket, UINT uiAfterInvenAccountDBID/*=0*/, bool bCheat/*=false*/ )
#endif
{
	TQAddParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.Data.PartyData.biLeaderCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.Data.PartyData.iServerID = iManagedID;
	TxPacket.Data.PartyData.LocationType = LocationType;
	TxPacket.Data.PartyData.iLocationID = iLocationID;
	TxPacket.Data.PartyData.Type = pPacket->PartyType;
	_wcscpy(TxPacket.Data.PartyData.wszPartyName, _countof(TxPacket.Data.PartyData.wszPartyName), pPacket->wszPartyName, static_cast<int>(wcslen(pPacket->wszPartyName)) );
	TxPacket.Data.PartyData.nPartyMaxCount = pPacket->cPartyMemberMax;
	TxPacket.Data.PartyData.iTargetMapIndex = pPacket->nTargetMapIdx;
	TxPacket.Data.PartyData.TargetMapDifficulty = pPacket->Difficulty;
	TxPacket.Data.PartyData.cMinLevel = pPacket->cUserLvLimitMin;
	TxPacket.Data.PartyData.LootRule = pPacket->ItemLootRule;
	TxPacket.Data.PartyData.LootItemRank = pPacket->ItemRank;
#if defined(PRE_FIX_62281)
	if(wszAfterInviteCharacterName != NULL)
		_wcscpy(TxPacket.Data.wszAfterInviteCharacterName, _countof(TxPacket.Data.wszAfterInviteCharacterName), wszAfterInviteCharacterName, static_cast<int>(wcslen(wszAfterInviteCharacterName)) );
	else
		TxPacket.Data.wszAfterInviteCharacterName[0] = _T('\0');
#else
	TxPacket.Data.uiAfterInviteAccountDBID = uiAfterInvenAccountDBID;
#endif
	TxPacket.Data.PartyData.iBitFlag = pPacket->iBitFlag;	
	if( pPacket->iBitFlag&Party::BitFlag::Password )
	{
		TxPacket.Data.PartyData.iPassword = pPacket->iPassword;
		TxPacket.Data.PartyData.iBitFlag |= Party::BitFlag::Password;
	}
	else
	{
		TxPacket.Data.PartyData.iBitFlag &= ~Party::BitFlag::Password;
	}
#if defined( _FINAL_BUILD )
	TxPacket.bCheat = false;
#else
	TxPacket.bCheat = bCheat;
#endif // #if defined( _FINAL_BUILD )
#if defined( PRE_ADD_NEWCOMEBACK )
	if( pSession->GetComebackAppellation() > 0 )
		TxPacket.Data.PartyData.bCheckComeBackParty = true;
#endif

	AddSendData( MAINCMD_PARTY, QUERY_ADDPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#if defined( _GAMESERVER )
void CDNDBConnection::QueryAddPartyAndMemberGame( CDNUserSession *pSession, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const Party::AddPartyAndMemberGame *PartyData, int iRoomID/*=0*/ )
{
	TQAddPartyAndMemberGame TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.	
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.Data.PartyData = PartyData->PartyData;
	TxPacket.Data.PartyData.iServerID = iManagedID;
	TxPacket.Data.PartyData.iCurMemberCount = 0;
	TxPacket.Data.PartyData.iRoomID = iRoomID;	
	TxPacket.Data.PartyData.LocationType = LocationType;
	TxPacket.Data.PartyData.iLocationID = iLocationID;
	
	memcpy( TxPacket.nKickedMemberList, PartyData->nKickedMemberList, sizeof(TxPacket.nKickedMemberList) );	
	for(int i=0;i<PARTYMAX;i++)
	{
		if( PartyData->biCharacterDBID[i] > 0 )
		{
			TxPacket.Member[i].biCharacterDBID = PartyData->biCharacterDBID[i];
			TxPacket.Member[i].nSessionID = PartyData->nSessionID[i];
#if defined( PRE_ADD_NEWCOMEBACK )
			TxPacket.Member[i].bCheckComebackAppellation = PartyData->bCheckComebackAppellation[i];
#endif
			TxPacket.Data.PartyData.iCurMemberCount++;
		}
		else
			break;
	}

	int nSize = ( sizeof(TQAddPartyAndMemberGame) - sizeof(TxPacket.Member) ) + ( sizeof(DBAddPartyMemberGame) * TxPacket.Data.PartyData.iCurMemberCount );
	AddSendData( MAINCMD_PARTY, QUERY_ADDPARTYANDMEMBERGAME, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNDBConnection::QueryModParty( CDNUserSession* pSession, const Party::Data& PartyData, int iBitFlag )
{
	TQModParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.PartyData = PartyData;
	TxPacket.PartyData.iBitFlag = iBitFlag;
	TxPacket.iRoomID = pSession->GetRoomID();

	AddSendData( MAINCMD_PARTY, QUERY_MODPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#endif

#if defined( _VILLAGESERVER )
void CDNDBConnection::QueryAddPartyAndMemberVillage( int iWorldID, UINT uiAccountDBID, int iManagedID, Party::LocationType::eCode LocationType, const Party::AddPartyAndMemberVillage *PartyData )
{
	TQAddPartyAndMemberVillage TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = uiAccountDBID;
	TxPacket.Data.PartyData = PartyData->PartyData;
	TxPacket.Data.PartyData.iServerID = iManagedID;
	TxPacket.Data.PartyData.LocationType = LocationType;
	memcpy( TxPacket.nKickedMemberList, PartyData->nKickedMemberList, sizeof(TxPacket.nKickedMemberList) );
	TxPacket.Data.PartyData.iCurMemberCount = 0;

	for(int i=0;i<PARTYMAX;i++)
	{
		if( PartyData->MemberData[i].nAccountDBID > 0 )
		{
			TxPacket.MemberInfo[i].nAccountDBID = PartyData-> MemberData[i].nAccountDBID;
			TxPacket.MemberInfo[i].biCharacterDBID = PartyData-> MemberData[i].biCharacterDBID;
			TxPacket.MemberInfo[i].cMemberIndex = PartyData-> MemberData[i].cMemberIndex;
			TxPacket.MemberInfo[i].VoiceInfo.cVoiceAvailable = PartyData->MemberData[i].cVoiceAvailable;
#if defined( PRE_ADD_NEWCOMEBACK )
			TxPacket.MemberInfo[i].bCheckComebackAppellation = PartyData-> MemberData[i].bCheckComebackAppellation;
#endif
			memcpy( TxPacket.MemberInfo[i].VoiceInfo.nMutedList, PartyData->MemberData[i].nMutedList, sizeof(TxPacket.MemberInfo[i].VoiceInfo.nMutedList) );						
			TxPacket.Data.PartyData.iCurMemberCount++;
		}
		else
			break;
	}

	int nSize = ( sizeof(TQAddPartyAndMemberVillage) - sizeof(TxPacket.MemberInfo) ) + ( sizeof(TMemberInfo) * TxPacket.Data.PartyData.iCurMemberCount );
	AddSendData( MAINCMD_PARTY, QUERY_ADDPARTYANDMEMBERVILLAGE, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNDBConnection::QueryModParty( CDNUserSession *pSession, CDNParty* pParty, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, const CSPartyInfoModify* pPacket )
{
	TQModParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.PartyData.PartyID = pParty->GetPartyID();
	TxPacket.PartyData.iServerID = iManagedID;
	TxPacket.PartyData.LocationType = LocationType;
	TxPacket.PartyData.iLocationID = iLocationID;
	TxPacket.PartyData.Type = pParty->GetPartyType();
	_wcscpy(TxPacket.PartyData.wszPartyName, _countof(TxPacket.PartyData.wszPartyName), pPacket->wszName, static_cast<int>(wcslen(pPacket->wszName)) );
	TxPacket.PartyData.nPartyMaxCount = pPacket->cPartyMemberMax;
	TxPacket.PartyData.iTargetMapIndex = pPacket->nTargetMapIdx;
	TxPacket.PartyData.TargetMapDifficulty = pPacket->Difficulty;
	TxPacket.PartyData.cMinLevel = pPacket->cUserLvLimitMin;
	TxPacket.PartyData.LootRule = pPacket->LootRule;
	TxPacket.PartyData.LootItemRank = pPacket->LootRank;
	TxPacket.PartyData.iUpkeepCount = pParty->GetUpkeepCount();
	TxPacket.PartyData.biLeaderCharacterDBID = pParty->GetLeaderCharacterID();
	TxPacket.PartyData.iCurMemberCount = pParty->GetCurMemberCount();
	TxPacket.PartyData.iBitFlag = pPacket->iBitFlag;	
	if( pPacket->iBitFlag&Party::BitFlag::Password )
	{
		TxPacket.PartyData.iPassword = pPacket->iPassword;
		TxPacket.PartyData.iBitFlag |= Party::BitFlag::Password;
	}
	else
	{
		TxPacket.PartyData.iBitFlag &= ~Party::BitFlag::Password;
	}

	AddSendData( MAINCMD_PARTY, QUERY_MODPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryModParty( CDNUserSession *pSession, CDNParty* pParty, int iManagedID, Party::LocationType::eCode LocationType, int iLocationID )
{
	TQModParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.PartyData.PartyID = pParty->GetPartyID();
	TxPacket.PartyData.iServerID = iManagedID;
	TxPacket.PartyData.LocationType = LocationType;
	TxPacket.PartyData.iLocationID = iLocationID;
	TxPacket.PartyData.Type = pParty->GetPartyType();
	_wcscpy(TxPacket.PartyData.wszPartyName, _countof(TxPacket.PartyData.wszPartyName), pParty->GetPartyName(), static_cast<int>(wcslen(pParty->GetPartyName())) );
	TxPacket.PartyData.nPartyMaxCount = pParty->GetMemberMax();
	TxPacket.PartyData.iTargetMapIndex = pParty->GetTargetMapIndex();
	TxPacket.PartyData.TargetMapDifficulty = pParty->GetDifficulty();
	TxPacket.PartyData.cMinLevel = pParty->GetMinLevel();
	TxPacket.PartyData.LootRule = pParty->GetItemLootRule();
	TxPacket.PartyData.LootItemRank = pParty->GetItemLootRank();
	TxPacket.PartyData.iUpkeepCount = pParty->GetUpkeepCount();
	TxPacket.PartyData.biLeaderCharacterDBID = pParty->GetLeaderCharacterID();
	TxPacket.PartyData.iCurMemberCount = pParty->GetCurMemberCount();
	TxPacket.PartyData.iBitFlag = pParty->GetBitFlag();	
	if( pParty->GetBitFlag()&Party::BitFlag::Password )
	{
		TxPacket.PartyData.iPassword = pParty->GetPassword();
		TxPacket.PartyData.iBitFlag |= Party::BitFlag::Password;
	}
	else
	{
		TxPacket.PartyData.iBitFlag &= ~Party::BitFlag::Password;
	}

	AddSendData( MAINCMD_PARTY, QUERY_MODPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif

void CDNDBConnection::QueryDelParty( int iWorldID, TPARTYID PartyID )
{
	TQDelParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.PartyID = PartyID;

	AddSendData( MAINCMD_PARTY, QUERY_DELPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryJoinParty( CDNUserSession *pSession, int iWorldID, TPARTYID PartyID, int iMaxUserCount )
{
	TQJoinParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.PartyID = PartyID;
	TxPacket.iMaxUserCount = iMaxUserCount;
#if defined( PRE_ADD_NEWCOMEBACK )
	if( pSession->GetComebackAppellation() > 0 )
		TxPacket.bCheckComeBackParty = true;
#endif

	AddSendData( MAINCMD_PARTY, QUERY_JOINPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryOutParty( int iWorldID, UINT uiAccountDBID, INT64 biCharacterDBID, UINT nSessionID, TPARTYID PartyID, Party::QueryOutPartyType::eCode Type )
{
	TQOutParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.nAccountDBID = uiAccountDBID;
	TxPacket.biCharacterDBID = biCharacterDBID;
	TxPacket.nSessionID = nSessionID;
	TxPacket.PartyID = PartyID;
	TxPacket.Type = Type;

	AddSendData( MAINCMD_PARTY, QUERY_OUTPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryModPartyLeader( CDNUserSession* pSession, TPARTYID PartyID )
{
#if defined( PRE_WORLDCOMBINE_PARTY ) && defined( _GAMESERVER )
	CDNGameRoom* pGameRoom = pSession->GetGameRoom();
	if( pGameRoom->bIsWorldCombineParty() )
		return;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY ) && defined( _GAMESERVER )

	TQModPartyLeader TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.PartyID = PartyID;
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();;

	AddSendData( MAINCMD_PARTY, QUERY_MODPARTYLEADER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryGetPartyJoinMembers( CDNUserSession* pSession, TPARTYID PartyID )
{
	TQGetListPartyMember TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.PartyID = PartyID;

	AddSendData( MAINCMD_PARTY, QUERY_GETPARTY_JOINMEMBERS, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

#if defined( PRE_ADD_NEWCOMEBACK )
void CDNDBConnection::QueryModComebackFlag( CDNUserSession* pSession, bool bRewardComeback )
{
	TQModComebackFlag TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();	
	TxPacket.bRewardComeback = bRewardComeback;
	
	AddSendData( MAINCMD_STATUS, QUERY_MOD_COMEBACKFLAG, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
void CDNDBConnection::QueryModMemberComebackInfo( CDNUserSession* pSession, TPARTYID PartyID )
{
	TQModPartyMemberComeback TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	if( pSession->GetComebackAppellation() > 0 )
		TxPacket.bComebackAppellation = true;
	TxPacket.PartyID = PartyID;

	AddSendData( MAINCMD_PARTY, QUERY_MOD_COMEBACKINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif	// #if defined( PRE_ADD_NEWCOMEBACK )
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNDBConnection::QueryGetListWorldParty( int iWorldID )
{
	TQGetListParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = iWorldID;
	TxPacket.iExecptServerID = g_Config.nManagedID;

	AddSendData( MAINCMD_PARTY, QUERY_GETLISTWORLDPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}

void CDNDBConnection::QueryAddWorldParty( int iManagedID, Party::LocationType::eCode LocationType, int iLocationID, Party::Data *PartyData )
{
	TQAddParty TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	// Unify party-related threads to thread 0 for serialization.
	TxPacket.cWorldSetID = PartyData->nWorldSetID;
	TxPacket.nAccountDBID = 0;
	TxPacket.Data.PartyData.biLeaderCharacterDBID = 0;
	TxPacket.Data.PartyData.iServerID = iManagedID;
	TxPacket.Data.PartyData.LocationType = LocationType;
	TxPacket.Data.PartyData.iLocationID = iLocationID;
	TxPacket.Data.PartyData.Type = PartyData->Type;
	TxPacket.Data.PartyData.iRoomID = PartyData->iRoomID;
	_wcscpy(TxPacket.Data.PartyData.wszPartyName, _countof(TxPacket.Data.PartyData.wszPartyName), PartyData->wszPartyName, static_cast<int>(wcslen(PartyData->wszPartyName)) );
	TxPacket.Data.PartyData.nPartyMaxCount = PartyData->nPartyMaxCount;
	TxPacket.Data.PartyData.iTargetMapIndex = PartyData->iTargetMapIndex;
	TxPacket.Data.PartyData.TargetMapDifficulty = PartyData->TargetMapDifficulty;
	TxPacket.Data.PartyData.cMinLevel = PartyData->cMinLevel;
	TxPacket.Data.PartyData.LootRule = PartyData->LootRule;
	TxPacket.Data.PartyData.LootItemRank = PartyData->LootItemRank;
#if defined(PRE_FIX_62281)
	TxPacket.Data.wszAfterInviteCharacterName[0] = _T('\0');
#else
	TxPacket.Data.uiAfterInviteAccountDBID = 0;
#endif
	TxPacket.Data.PartyData.iBitFlag = PartyData->iBitFlag;	
	TxPacket.Data.PartyData.nPrimaryIndex = PartyData->nPrimaryIndex;
	
	TxPacket.bCheat = false;
	AddSendData( MAINCMD_PARTY, QUERY_ADDWORLDPARTY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif

#if defined(PRE_ADD_DOORS_PROJECT)
#if defined(_GAMESERVER)
void CDNDBConnection::QuerySaveCharacterAbility(CDNUserSession* pSession)
{
	TQSaveCharacterAbility TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	
	CDnPlayerActor* pActor = pSession->GetPlayerActor();
	if( pActor )
	{
		CDnState PlayerState;
		pSession->GetPlayerActor()->MakeEquipAndPassiveState( PlayerState );

		TxPacket.nCharacterAblity[Doors::CharacterAbility::Strength] = PlayerState.GetStrength();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Agility] = PlayerState.GetAgility();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Intelligence] = PlayerState.GetIntelligence();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Stamina] = PlayerState.GetStamina();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::AttackPMin] = PlayerState.GetAttackPMin();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::AttackPMax] = PlayerState.GetAttackPMax();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::AttackMMin] = PlayerState.GetAttackMMin();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::AttackMMax] = PlayerState.GetAttackMMax();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::DefenseP] = PlayerState.GetDefenseP();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::DefenseM] = PlayerState.GetDefenseM();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Stiff] = PlayerState.GetStiff();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::StiffResistance] = PlayerState.GetStiffResistance();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Critical] = PlayerState.GetCritical();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::CriticalResistance] = PlayerState.GetCriticalResistance();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::Stun] = PlayerState.GetStun();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::StunResistance] = PlayerState.GetStunResistance();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::FinalDamage] = PlayerState.GetFinalDamage();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::FireAttack] = (int)(PlayerState.GetElementAttack(CDnState::Fire)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::IceAttack] = (int)(PlayerState.GetElementAttack(CDnState::Ice)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::LightAttack] = (int)(PlayerState.GetElementAttack(CDnState::Light)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::DarkAttack] = (int)(PlayerState.GetElementAttack(CDnState::Dark)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::FireDefense] = (int)(PlayerState.GetElementDefense(CDnState::Fire)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::IceDefense] = (int)(PlayerState.GetElementDefense(CDnState::Ice)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::LightDefense] = (int)(PlayerState.GetElementDefense(CDnState::Light)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::DarkDefense] = (int)(PlayerState.GetElementDefense(CDnState::Dark)* 100.0f);
		TxPacket.nCharacterAblity[Doors::CharacterAbility::MoveSpeed] = PlayerState.GetMoveSpeed();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::MaxHP] = (int)PlayerState.GetMaxHP();
		TxPacket.nCharacterAblity[Doors::CharacterAbility::MaxMP] = PlayerState.GetMaxSP();

		AddSendData( MAINCMD_STATUS, QUERY_SAVE_CHARACTER_ABILITY, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
	}
}
#endif //#if defined(_GAMESERVER)
#endif //#if defined(PRE_ADD_DOORS_PROJECT)

#if defined (PRE_ADD_BESTFRIEND)
void CDNDBConnection::QueryGetBestFriend(BYTE cThreadID, CDNUserSession *pSession, bool bSend)
{
	TQGetBestFriend Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.bSend = bSend;

	AddSendData(MAINCMD_BESTFRIEND, QUERY_GET_BESTFRIEND, (char*)&Packet, sizeof(Packet));
}


void CDNDBConnection::QueryRegistBestFriend(CDNUserSession *pSession, UINT nAccountDBID, INT64 biCharacterDBID, LPCWSTR lpwszCharacterName, INT64 biRegistSerial, int nItemID)
{
	TQRegistBestFriend Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();

	Packet.nFromAccountDBID = pSession->GetAccountDBID();
	Packet.biFromCharacterDBID = pSession->GetCharacterDBID();
	_wcscpy(Packet.wszFromName, _countof(Packet.wszFromName), pSession->GetCharacterName(), _countof(Packet.wszFromName));

	Packet.nToAccountDBID = nAccountDBID;
	Packet.biToCharacterDBID = biCharacterDBID;
	_wcscpy(Packet.wszToName, _countof(Packet.wszToName), lpwszCharacterName, _countof(Packet.wszToName));

	Packet.nItemID = nItemID;

	Packet.biRegistSerial = biRegistSerial;
	Packet.nMapID = pSession->GetMapIndex();
	_wcscpy(Packet.wszIP, _countof(Packet.wszIP), pSession->GetIpW(), (int)wcslen(pSession->GetIpW()));

	AddSendData(MAINCMD_BESTFRIEND, QUERY_REGIST_BESTFRIEND, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryCancelBestFriend(CDNUserSession *pSession, bool bCancel)
{
	TQCancelBestFriend Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();

	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.bCancel = bCancel;

	AddSendData(MAINCMD_BESTFRIEND, QUERY_CANCEL_BESTFRIEND, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryCloseBestFriend(CDNUserSession *pSession)
{
	TQCloseBestFriend Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();

	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_BESTFRIEND, QUERY_CLOSE_BESTFRIEND, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryEditBestFriendMemo(CDNUserSession *pSession, LPCWSTR lpwszMemo)
{
	TQEditBestFriendMemo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	_wcscpy(Packet.wszMemo, _countof(Packet.wszMemo), lpwszMemo, _countof(Packet.wszMemo));

	AddSendData(MAINCMD_BESTFRIEND, QUERY_EDIT_BESTFRIENDMEMO, (char*)&Packet, sizeof(Packet));
}

#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNDBConnection::QueryGetPrivateChatChannelInfo(BYTE cThreadID, int nWorldID)
{

	TQPrivateChatChannelInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = nWorldID;	
	
	
	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_GET_PRIVATECHATCHANNEL, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetPrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID)
{
	TQPrivateChatChannelMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nPrivateChatChannelID = nChannelID;

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_GET_PRIVATECHATCHANNELMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryAddPrivateChatChannel(CDNUserSession *pSession, WCHAR* wszChannelName, int nPassWord)
{
	TQAddPrivateChatChannel Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.tPrivateChatChannel.biMasterCharacterDBID = pSession->GetCharacterDBID();	
	Packet.tPrivateChatChannel.nPassWord = nPassWord;
	CTimeSet CurTime;
	Packet.tJoinDate = CurTime.GetTimeT64_LC();
	
	_wcscpy(Packet.wszCharacterName, _countof(Packet.wszCharacterName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );
	if(wszChannelName)
		_wcscpy(Packet.tPrivateChatChannel.wszName, _countof(Packet.tPrivateChatChannel.wszName), wszChannelName, (int)wcslen(wszChannelName) );

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_CREATE_PRIVATECHATCHANNEL, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryAddPrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID, PrivateChatChannel::Common::eModType eType)
{
	TQAddPrivateChatMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nServerID = g_Config.nManagedID;
	Packet.eType = eType;
	Packet.Member.nAccountDBID = pSession->GetAccountDBID();
	Packet.nPrivateChatChannelID = nChannelID;
	Packet.Member.biCharacterDBID = pSession->GetCharacterDBID();
	CTimeSet CurTime;
	Packet.Member.tJoinDate = CurTime.GetTimeT64_LC();

	_wcscpy(Packet.Member.wszCharacterName, _countof(Packet.Member.wszCharacterName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_ADD_PRIVATECHATMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryInvitePrivateChatChannelMember(CDNUserSession *pSession, INT64 nChannelID, UINT nInviteAccountID )
{
	TQInvitePrivateChatMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nServerID = g_Config.nManagedID;
	Packet.Member.nAccountDBID = pSession->GetAccountDBID();
	Packet.nPrivateChatChannelID = nChannelID;
	Packet.Member.biCharacterDBID = pSession->GetCharacterDBID();	
	Packet.nMasterAccountDBID = nInviteAccountID;
	CTimeSet CurTime;
	Packet.Member.tJoinDate = CurTime.GetTimeT64_LC();

	_wcscpy(Packet.Member.wszCharacterName, _countof(Packet.Member.wszCharacterName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_INVITE_PRIVATECHATMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryOutPrivateChatChannelMember(CDNUserSession *pSession, PrivateChatChannel::Common::eModType eType)
{
	TQDelPrivateChatMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.eType = eType;
	Packet.nPrivateChatChannelID = pSession->GetPrivateChannelID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();		
	_wcscpy(Packet.wszName, _countof(Packet.wszName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()) );	
	
	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_DEL_PRIVATECHATMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryKickPrivateChatChannelMember(CDNUserSession *pSession, WCHAR* wszKickName, INT64 biKickCharacterDBID)
{
	TQDelPrivateChatMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nPrivateChatChannelID = pSession->GetPrivateChannelID();
	Packet.biCharacterDBID = biKickCharacterDBID;
	
	if(wszKickName)
		_wcscpy(Packet.wszName, _countof(Packet.wszName), wszKickName, (int)wcslen(wszKickName) );	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_KICK_PRIVATECHATMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryModPrivateChatChannelInfo( CDNUserSession *pSession, WCHAR* wszChannelName, int nModType, int nPassWord, INT64 biCharacterDBID )
{
	TQModPrivateChatChannelInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.eType = (PrivateChatChannel::Common::eModType)nModType;
	Packet.nPrivateChatChannelID = pSession->GetPrivateChannelID();
	Packet.nPassWord = nPassWord;
	Packet.biCharacterDBID = biCharacterDBID;	

	if(wszChannelName)
		_wcscpy(Packet.wszChannelName, _countof(Packet.wszChannelName), wszChannelName, (int)wcslen(wszChannelName) );	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_MOD_PRIVATECHATCHANNELINFO, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryModPrivateMemberServerID( CDNUserSession *pSession )
{
	TQModPrivateChatMemberServerID Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nServerID = g_Config.nManagedID;
	Packet.biCharacterDBID = pSession->GetCharacterDBID();	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_MOD_PRIVATEMEMBERSERVERID, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryDelPrivateMemberServerID( BYTE cThreadID, int nWorldID, int nServerID)
{
	TQDelPrivateChatMemberServerID Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = nWorldID;	
	Packet.nServerID = g_Config.nManagedID;	

	AddSendData(MAINCMD_PRIVATECHATCHANNEL, QUERY_DEL_PRIVATEMEMBERSERVERID, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif

#if defined( PRE_FIX_67546 )
void CDNDBConnection::QueryAddThreadCount( BYTE cChoiceThreadID )
{
	TQAddThreadCount Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = 0;	
	Packet.cChoiceThreadID = cChoiceThreadID;

	AddSendData(MAINCMD_ETC, QUERY_ADD_CHANNELCOUNT, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
#if defined( _GAMESERVER )

void CDNDBConnection::QueryAddWorldPvPRoom( BYTE cThreadID, int nWorldID, UINT nGMAccountDBID, int nServerID, int nRoomID, TWorldPvPMissionRoom* pMissonRoom )
{
	TQAddWorldPvPRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;	
	Packet.cWorldSetID = nWorldID;
	Packet.nAccountDBID = nGMAccountDBID;
	Packet.nServerID = nServerID;
	Packet.nRoomID = nRoomID;
	memcpy(&Packet.TMissionRoom, pMissonRoom, sizeof(Packet.TMissionRoom));	

	if( pMissonRoom->unRoomOptionBit&PvPCommon::RoomOption::BreakInto )
		Packet.bBreakIntoFlag = true;

	if( pMissonRoom->unRoomOptionBit&PvPCommon::RoomOption::NoRegulation )
		Packet.bRegulationFlag = true;	

	if(pMissonRoom->unRoomOptionBit&PvPCommon::RoomOption::RandomTeam )
		Packet.bRandomFlag = true;	

	_wcscpy( Packet.wszRoomName, _countof(Packet.wszRoomName), pMissonRoom->wszRoomName, (int)wcslen(pMissonRoom->wszRoomName) );
	
	AddSendData(MAINCMD_PVP, QUERY_ADD_WORLDPVPROOM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryAddWorldPvPRoomMember( BYTE cThreadID, int nWorldID,  CDNUserSession* pSession, BYTE cMaxMemberCount, int nWorldPvPRoomDBIndex, bool bObserverFlag )
{
	TQAddWorldPvPRoomMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;	
	Packet.cWorldSetID = nWorldID;
	Packet.nWorldPvPRoomDBIndex = nWorldPvPRoomDBIndex;
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.cMaxPlayers = cMaxMemberCount;		
	Packet.bObserverFlag = bObserverFlag;	
	
	AddSendData(MAINCMD_PVP, QUERY_ADD_WORLDPVPROOMMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryDelWorldPvPRoomForServer( int nServerID )
{
	TQDelWorldPvPRoomForServer Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = 0;		
	Packet.nServerID = nServerID;

	AddSendData(MAINCMD_PVP, QUERY_DEL_WORLDPVPROOMFORSERVER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryDelWorldPvPRoom( int nWorldID, int nPvPRoomDBIndex )
{
	TQDelWorldPvPRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = 0;	
	Packet.cWorldSetID = nWorldID;
	Packet.nWorldPvPRoomDBIndex = nPvPRoomDBIndex;

	
	AddSendData(MAINCMD_PVP, QUERY_DEL_WORLDPVPROOM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryDelWorldPvPRoomMember( BYTE cThreadID, int nWorldID, int nPvPRoomDBIndex,  CDNUserSession* pSession )
{
	TQDelWorldPvPRoomMember Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;	
	Packet.cWorldSetID = nWorldID;	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nWorldPvPRoomDBIndex = nPvPRoomDBIndex;
	
	AddSendData(MAINCMD_PVP, QUERY_DEL_WORLDPVPROOMMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

#endif

#if defined( _VILLAGESERVER )
void CDNDBConnection::QueryGetListWorldPvPRoom()
{
	TQGetListWorldPvPRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = 0;	
	Packet.cWorldSetID = g_Config.nWorldSetID;		
	Packet.nCombineWorldID = g_Config.nCombinePartyWorld;

	AddSendData(MAINCMD_PVP, QUERY_GETLIST_WORLDPVPROOM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryUpdateWorldPvPRoom(BYTE cThreadID, int nWorldID)
{
	TQUpdateWorldPvPRoom Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = cThreadID;	
	Packet.cWorldSetID = nWorldID;	

	AddSendData(MAINCMD_PVP, QUERY_UPDATE_WORLDPVPROOM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

#endif

#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )

void CDNDBConnection::QueryResetAlteiaWorldEvent( BYTE cThreadID, int nWorldID )
{
	TQResetAlteiaWorldEvent Packet;
	memset(&Packet, 0, sizeof(TQResetAlteiaWorldEvent));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = nWorldID;	
	
	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_RESET_ALTEIAWORLDEVENT, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddAlteiaWorldEventTime( BYTE cThreadID, int nWorldID, time_t tAlteiaEventStartTime, time_t tAlteiaEventEndTime )
{
	TQAddAlteiaWorldEvent Packet;
	memset(&Packet, 0, sizeof(TQAddAlteiaWorldEvent));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = nWorldID;	
	Packet.tAlteiaEventStartTime = tAlteiaEventStartTime;
	Packet.tAlteiaEventEndTime = tAlteiaEventEndTime;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_ADD_ALTEIAWORLDEVENT, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddAlteiaWorldPlayResult( CDNUserSession *pSession, int nGoldKeyCount, DWORD dwPlayTime, int nGuildID )
{
	TQADDAlteiaWorldPlayResult Packet;
	memset(&Packet, 0, sizeof(TQADDAlteiaWorldPlayResult));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();	
	Packet.nGoldKeyCount = nGoldKeyCount;
	Packet.dwPlayTime = dwPlayTime;
	Packet.nGuildID = nGuildID;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_ADD_ALTEIAWORLDPLAYRESULT, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetAlteiaWorldInfo( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetAlteiaWorldInfo Packet;
	memset(&Packet, 0, sizeof(TQGetAlteiaWorldInfo));
	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();	

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_GET_ALTEIAWORLDINFO, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetAlteiaWorldPrivateGoldKeyRank( CDNUserSession *pSession )
{
	TQGetAlteiaWorldRankInfo Packet;
	memset(&Packet, 0, sizeof(TQGetAlteiaWorldRankInfo));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nMaxRank = AlteiaWorld::Common::MaxRankCount;
	Packet.eType = AlteiaWorld::Info::PrivateGoldKeyRankInfo;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_GET_ALTEIAWORLDPRIVATEGOLDKEYRANK, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetAlteiaWorldPrivatePlayTimeRank( CDNUserSession *pSession )
{
	TQGetAlteiaWorldRankInfo Packet;
	memset(&Packet, 0, sizeof(TQGetAlteiaWorldRankInfo));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.nMaxRank = AlteiaWorld::Common::MaxRankCount;
	Packet.eType = AlteiaWorld::Info::PrivatePlayTimeRankInfo;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_GET_ALTEIAWORLDPRIVATEPLAYTIMERANK, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetAlteiaWorldGuildGoldKeyRank( CDNUserSession *pSession )
{
	TQGetAlteiaWorldRankInfo Packet;
	memset(&Packet, 0, sizeof(TQGetAlteiaWorldRankInfo));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nMaxRank = AlteiaWorld::Common::MaxRankCount;
	Packet.eType = AlteiaWorld::Info::GuildGoldKeyRankInfo;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_GET_ALTEIAWORLDGUILDGOLDKEYRANK, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetAlteiaWorldSendTicketList( BYTE cThreadID, CDNUserSession *pSession )
{
	TQGetAlteiaWorldSendTicketList Packet;
	memset(&Packet, 0, sizeof(TQGetAlteiaWorldSendTicketList));
	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_GET_ALTEIAWORLDSENDTICKETLIST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddAlteiaWorldSendTicketList( CDNUserSession *pSession, INT64 biSendCharacterDBID, WCHAR* wszSendCharacterName, int nSendMaxCount )
{
	TQAddAlteiaWorldSendTicketList Packet;
	memset(&Packet, 0, sizeof(TQAddAlteiaWorldSendTicketList));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.biSendCharacterDBID = biSendCharacterDBID;
	Packet.nSendMaxCount = nSendMaxCount;

	if( wszSendCharacterName )
		_wcscpy( Packet.wszSendCharacterName, _countof(Packet.wszSendCharacterName), wszSendCharacterName, (int)wcslen(wszSendCharacterName) );

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_ADD_ALTEIAWORLDSENDTICKETLIST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryResetAlteiaWorldPlayAlteia( CDNUserSession *pSession, AlteiaWorld::ResetType::eResetType eType )
{
	TQResetAlteiaWorldPlayAlteia Packet;
	memset(&Packet, 0, sizeof(TQResetAlteiaWorldPlayAlteia));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.eType = eType;

	AddSendData(MAINCMD_ALTEIAWORLD, QUERY_RESET_ALTEIAWORLDPLAYALTEIA, (char*)&Packet, sizeof(Packet));
}

#endif

#if defined( PRE_ADD_STAMPSYSTEM )
void CDNDBConnection::QueryGetListCompleteChallenges( BYTE cThreadID, CDNUserSession * pSession )
{
	TQGetListCompleteChallenges Packet;
	memset(&Packet, 0, sizeof(TQGetListCompleteChallenges));
	Packet.cThreadID = cThreadID;
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STAMPSYSTEM, QUERY_GETLIST_COMPLETECHALLENGES, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryInitCompleteChallenge( CDNUserSession* pSession )
{
	TQInitCompleteChallenge Packet;
	memset(&Packet, 0, sizeof(TQInitCompleteChallenge));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();

	AddSendData(MAINCMD_STAMPSYSTEM, QUERY_INIT_COMPLETECHALLENGE, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryAddCompleteChallenge( CDNUserSession* pSession, BYTE cChallengeIndex, int nWeekDay )
{
	TQAddCompleteChallenge Packet;
	memset(&Packet, 0, sizeof(TQAddCompleteChallenge));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.cChallengeIndex = cChallengeIndex;
	Packet.nWeekDay = nWeekDay;

	AddSendData(MAINCMD_STAMPSYSTEM, QUERY_ADD_COMPLETECHALLENGE, (char*)&Packet, sizeof(Packet));
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined(PRE_ADD_GAMEQUIT_REWARD)
void CDNDBConnection::QueryModNewbieRewardFlag(CDNUserSession *pSession, bool bRewardFlag)
{
	TQModNewbieRewardFlag Packet;
	memset(&Packet, 0, sizeof(TQModNewbieRewardFlag));
	Packet.bRewardFlag = bRewardFlag;
	Packet.nAccountDBID = pSession->GetAccountDBID();

	AddSendData(MAINCMD_ETC, QUERY_MOD_NEWBIE_REWARDFLAG, (char*)&Packet, sizeof(Packet));
}
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#if defined(PRE_ADD_CP_RANK)
void CDNDBConnection::QueryAddStageClearBest( CDNUserSession* pSession, int nMapID, DBDNWorldDef::ClearGradeCode::eCode ClearCode, int nClearPoint, int nLimitLevel)
{
	TQAddStageClearBest Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nMapID = nMapID;
	Packet.Code = ClearCode;
	Packet.nClearPoint = nClearPoint;
	Packet.nLimitLevel = nLimitLevel;

	AddSendData(MAINCMD_ETC, QUERY_ADD_STAGE_CLEAR_BEST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetStageClearBest( CDNUserSession* pSession, int nMapID )
{
	TQGetStageClearBest Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();	
	Packet.nMapID = nMapID;

	AddSendData(MAINCMD_ETC, QUERY_GET_STAGE_CLEAR_BEST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetStageClearPersonalBest( CDNUserSession* pSession, int nMapID )
{
	TQGetStageClearPersonalBest Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nMapID = nMapID;

	AddSendData(MAINCMD_ETC, QUERY_GET_STAGE_PERSONAL_BEST, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryInitStageCP(CDNUserSession * pSession, char cInitType)
{
	TQInitStageCP Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.cInitType = cInitType;

	AddSendData(MAINCMD_ETC, QUERY_INIT_STAGE_BEST, (char*)&Packet, sizeof(Packet));
}
#endif //#if defined(PRE_ADD_CP_RANK)

#if defined( PRE_FIX_76282 )
void CDNDBConnection::SendSyncGoPvPLobby( CDNUserSession* pSession )
{
	TQHeader Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cWorldSetID = pSession->GetWorldSetID();

	AddSendData(MAINCMD_PVP, SYNC_GOPVPLOBBY, (char*)&Packet, sizeof(Packet));
}
#endif // #if defined( PRE_FIX_76282 )

#if defined(PRE_ADD_GUILD_CONTRIBUTION)
void CDNDBConnection::QueryGetGuildContributionPoint( BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID )
{
	TQGetGuildContributionPoint Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.biCharacterDBID = biCharacterDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDCONTRIBUTION_POINT, (char*)&Packet, sizeof(Packet));
}

void CDNDBConnection::QueryGetWeeklyGuildContributionPointRanking( BYTE cThreadID, char cWorldSetID, UINT nAccountDBID, UINT nGuildDBID)
{
	TQGetGuildContributionRanking Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = cWorldSetID;
	Packet.nAccountDBID = nAccountDBID;
	Packet.nGuildDBID = nGuildDBID;

	AddSendData(MAINCMD_GUILD, QUERY_GET_GUILDCONTRIBUTION_WEEKLYRANKING, (char*)&Packet, sizeof(Packet));
}
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)


#if defined(PRE_ADD_EQUIPLOCK)
void CDNDBConnection::QueryAddLockItem( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex )
{
	TQLockItemInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	const TItem* pItem;
	if(Code == DBDNWorldDef::ItemLocation::Equip)
		pItem = pSession->GetItem()->GetEquip(cItemSlotIndex);
	else if(Code == DBDNWorldDef::ItemLocation::CashEquip)
		pItem = pSession->GetItem()->GetCashEquip(cItemSlotIndex);
	else
		return;

	if(!pItem) return;
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.Code = Code;
	Packet.cItemSlotIndex = cItemSlotIndex;
	Packet.nItemID = pItem->nItemID;
	Packet.biItemSerial = pItem->nSerial;

	AddSendData(MAINCMD_ITEM, QUERY_ADDLOCK_ITEM, (char*)&Packet, sizeof(Packet));
}
void CDNDBConnection::QueryRequestItemLock( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex, int nUnLockWaitTime )
{
	TQUnLockRequsetItemInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	const TItem* pItem;
	if(Code == DBDNWorldDef::ItemLocation::Equip)
		pItem = pSession->GetItem()->GetEquip(cItemSlotIndex);
	else if(Code == DBDNWorldDef::ItemLocation::CashEquip)
		pItem = pSession->GetItem()->GetCashEquip(cItemSlotIndex);
	else
		return;

	if(!pItem) return;
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.Code = Code;
	Packet.cItemSlotIndex = cItemSlotIndex;
	Packet.nItemID = pItem->nItemID;
	Packet.biItemSerial = pItem->nSerial;
	Packet.nUnLockWaitTime = nUnLockWaitTime;

	AddSendData(MAINCMD_ITEM, QUERY_REQUEST_ITEMUNLOCK, (char*)&Packet, sizeof(Packet));
}
void CDNDBConnection::QueryUnLockItem( CDNUserSession* pSession, DBDNWorldDef::ItemLocation::eCode Code, BYTE cItemSlotIndex, int nItemID, INT64 biItemSerial )
{
	TQLockItemInfo Packet;
	memset(&Packet, 0, sizeof(Packet));

	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.Code = Code;
	Packet.cItemSlotIndex = cItemSlotIndex;
	Packet.nItemID = nItemID;
	Packet.biItemSerial = biItemSerial;

	AddSendData(MAINCMD_ITEM, QUERY_UNLOCK_ITEM, (char*)&Packet, sizeof(Packet));
}
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined(PRE_ADD_TALISMAN_SYSTEM)
void CDNDBConnection::QueryModTalismanSlotBitmap( CDNUserSession* pSession, int nTalismanOpenFlag )
{
	if (!pSession) return;

	TQModTalismanSlotBitmap TxPacket;
	memset(&TxPacket, 0, sizeof(TxPacket));

	TxPacket.cThreadID = pSession->GetDBThreadID();
	TxPacket.cWorldSetID = pSession->GetWorldSetID();
	TxPacket.nAccountDBID = pSession->GetAccountDBID();
	TxPacket.biCharacterDBID = pSession->GetCharacterDBID();
	TxPacket.nTalismanOpenFlag = nTalismanOpenFlag;

	AddSendData(MAINCMD_STATUS, QUERY_MOD_TALISMANSLOTBITMAP, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

#if defined(PRE_ADD_CHAOSCUBE)
void CDNDBConnection::QueryGetListChaosGaugeItems( BYTE cThreadID, CDNUserSession* pSession )
{
	if (!pSession) return;
	TQGetListChaosGaugeItems Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	AddSendData(MAINCMD_ITEM, QUERY_GETLIST_CHAOSGAUGEITEMS, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryUpsChaosGaugeItem( CDNUserSession* pSession, int nChaosGaugeType, int nItemGauge )
{
	if (!pSession) return;
	TQUpsChaosGaugeItem Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.nChaosGaugeType = nChaosGaugeType;
	Packet.nItemGauge = nItemGauge;
	AddSendData(MAINCMD_ITEM, QUERY_UPS_CHAOSGAUGEITEM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif	// #if defined(PRE_ADD_CHAOSCUBE)

#if defined(PRE_ADD_DWC)
void CDNDBConnection::QueryAddDWCTeam( CDNUserSession* pSession, const WCHAR* wszTeamName )
{
	if (!pSession || !wszTeamName) return;
	TQAddDWCTeam Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	_wcscpy(Packet.wszTeamName, _countof(Packet.wszTeamName), wszTeamName, (int)wcsnlen(wszTeamName, DWC::MAX_TEAMNAME_LEN - 1));
	AddSendData(MAINCMD_DWC, QUERY_ADD_DWCTEAM, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryAddDWCTeamMember( CDNUserSession* pSession, UINT nTeamID, INT64 biTargetCharDBID, const WCHAR* wszCharName, int nMaxMemberCount )
{
	if (!pSession) return;
	TQAddDWCTeamMember Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nTeamID = nTeamID;
	Packet.biCharacterDBID = biTargetCharDBID;
	Packet.nMaxMemberCount = nMaxMemberCount;
	if (wszCharName)
		_wcscpy(Packet.wszCharacterName, _countof(Packet.wszCharacterName), wszCharName, (int)wcsnlen(wszCharName, NAMELENMAX - 1));
	AddSendData(MAINCMD_DWC, QUERY_ADD_DWCTEAMMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryDelDWCTeamMember( CDNUserSession* pSession, UINT nTeamID )
{
	if (!pSession) return;
	TQDelDWCTeamMember Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nTeamID = nTeamID;
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	if (pSession->GetCharacterName())
		_wcscpy(Packet.wszCharacterName, _countof(Packet.wszCharacterName), pSession->GetCharacterName(), (int)wcsnlen(pSession->GetCharacterName(), NAMELENMAX - 1));
	AddSendData(MAINCMD_DWC, QUERY_DEL_DWCTEAMMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetDWCTeamByChar( BYTE cThreadID, CDNUserSession* pSession, bool bNeedMemberList )
{
	if (!pSession) return;
	TQGetDWCTeamByChar Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.biCharacterDBID = pSession->GetCharacterDBID();
	Packet.bNeedMemberList = bNeedMemberList;
	AddSendData(MAINCMD_DWC, QUERY_GET_DWCTEAM_BY_CHAR, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetListDWCTeamMember( CDNUserSession* pSession, UINT nTeamID )
{
	if (!pSession) return;
	TQGetListDWCTeamMember Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nTeamID = nTeamID;
	AddSendData(MAINCMD_DWC, QUERY_GETLIST_DWCTEAMMEMBER, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetPageDWCRanking( CDNUserSession* pSession, int nPageNum, int nPageSize )
{
	if (!pSession) return;
	TQGetPageDWCRanking Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.nPageNum = nPageNum;
	Packet.nPageSize = nPageSize;
	AddSendData(MAINCMD_DWC, QUERY_GETPAGE_DWCRANKING, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetDWCRanking( CDNUserSession* pSession, BYTE cFindType, const WCHAR* wszFindName )
{
	if (!pSession) return;
	TQGetDWCRanking Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = pSession->GetDBThreadID();
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	Packet.cFindType = cFindType;
	if (wszFindName)
		_wcscpy(Packet.wszFindName, _countof(Packet.wszFindName), wszFindName, (int)wcsnlen(wszFindName, DWC::MAX_FIND_NAME_LEN - 1));
	AddSendData(MAINCMD_DWC, QUERY_GET_DWCRANKING, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}

void CDNDBConnection::QueryGetDWCChannel( BYTE cThreadID, CDNUserSession* pSession )
{
	if (!pSession) return;
	TQGetDWCChannel Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cThreadID = cThreadID;
	Packet.cWorldSetID = pSession->GetWorldSetID();
	Packet.nAccountDBID = pSession->GetAccountDBID();
	AddSendData(MAINCMD_DWC, QUERY_GET_DWCCHANNEL, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif	// #if defined(PRE_ADD_DWC)