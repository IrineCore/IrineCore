#include "stdafx.h"
#include "DnActor.h"
#include "GameSendPacket.h"
#include "DnWorldProp.h"
#include "Task.h"
#include "TaskManager.h"
#include "DnPartyTask.h"
#include "DNUserSession.h"
#include "DNRUDPGameServer.h"
#include "DNGameRoom.h"
#include "DNPvPGameRoom.h"
#include "DNIocpManager.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"
#include "DnPlayerActor.h"
#include "DnActor.h"
#include "Log.h"
#include "DNProtocol.h"
#include "DNGameServerScriptAPI.h"
#include "DNUserTcpConnection.h"
#include "DNLogConnection.h"
#include "DnGameTask.h"
#include "DNGameDataManager.h"
#include "PvPRespawnMode.h"
#include "DNFriend.h"
#include "DNIsolate.h"
#include "DNWorldUserState.h"
#include "DNMissionSystem.h"
#include "DNAppellation.h"
#include "DNMissionScheduler.h"
#include "DNAuthManager.h"
#include "DNRestraint.h"
#include "DNGuildSystem.h"
#include "VarArg.h"
#include "DnSkillTask.h"
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "ReputationSystemRepository.h"
#include "ReputationSystemEventHandler.h"
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
#include "MasterRewardSystem.h"
#include "MasterSystemCacheRepository.h"
#if defined( PRE_ADD_SECONDARY_SKILL )
#include "SecondarySkillRepository.h"
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )
#include "DNChatRoom.h"
#include "DNChatRoomManager.h"
#include "LadderStats.h"
#include "DNTimeEventSystem.h"
#include "DnBlow.h"
#include "DNGuildRecruitCacheRepository.h"
#if defined (PRE_ADD_BESTFRIEND)
#include "DNBestFriend.h"
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
#endif
#if defined(PRE_FIX_69108)
#include "DNDLGameRoom.h"
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
#include "DnWorldOperationProp.h"
#include "DnItemTask.h"
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
using namespace eItem;

extern TGameConfig g_Config;

CDNUserSession::CDNUserSession(unsigned int iUID, CDNRUDPGameServer * pServer, CDNGameRoom * pRoom)
{
	m_pGameServer = pServer;
	m_pGameRoom = pRoom;
	m_pTcpConnection = NULL;

	m_iState = SESSION_STATE_NONE;
	m_iNetID = 0; //Management ID generated at RUDPSocketFrame layer; required for sending and receiving.
	m_nSessionID = iUID;
	m_cPacketSeq = 1;

	memset(m_szNextVillageIP, 0, sizeof(IPLENMAX));
	m_nNextVillagePort		= 0;


	m_bStageAbortLog = true;

	for( int i=0; i<3; i++ ) {
		memset( m_pPacketQueueBuffer[i], 0, PACKET_QUEUE_SIZE );
		m_nPacketQueueOffset[i] = 0;
		m_nPacketQueueCount[i] = 0;
	}
	
	m_bIntededDisconnected = false;
	m_bRudpDisconnected = false;
	m_nRudpDisconnectedTick = 0;

	m_iTeam = 0;

	m_cKickKind = -1;
	m_bAbsent = false;
	m_bPvPGameModeFinish = false;
	m_bGMTrace = false;
	m_bIsAIBot = false;
	m_iDecreaseFatigue = 0;
	m_bLoadingComplete = false;
	m_iTotalMasterDecreaseFatigue = 0;
	m_nPartyMemberIndex = -1;
	m_nChatRoomID = 0;
	m_bPvPFatigueOption = false;
	m_biPickUpCoin = 0;
	m_bOuted = false;
	m_dwCheckUdpPing	= timeGetTime();
	m_pairUdpPing		= std::make_pair(0,0);
	m_dwKeepOperationID = 0;

	m_bHide = false;
	m_bTutorial = false;
	m_uiPvPUserState = 0;
	m_mGuildRewardItem.clear();
	m_nPeriodeExpItemRate = 0;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	m_nGoldBoxCheatID = 0;
#endif
#if defined(PRE_ADD_ACTIVEMISSION)
	m_nActiveMissionCheatID = 0;
#endif
#if defined( PRE_ADD_DIRECTNBUFF )
	m_bDirectPartyBuffItem = false;
	m_DirectPartyBuffItemList.clear();
#endif
#ifdef PRE_MOD_INDUCE_TCPCONNECT
	m_dwSendReqTCPConnect = 0;
	m_bRecvReqTCPConnectMsg = false;
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

#if defined( PRE_ADD_GUILDREWARDITEM )
	m_GuildRewardbuffItemList.clear();
#endif
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	m_nAlteiaWorldPosition = 1;
	m_dwAlteiaWorldMoveNextMapTick = 0;
	m_cDailyPlayCount = 0;
	m_bCanDice = true;
#endif

#ifdef PRE_ADD_PACKETSIZE_CHECKER
	m_nSendCheckTick = 0;
	m_nSendCheckSize = 0;
	m_nSendCheckCount = 0;

	memset(m_SendDebugInfo, 0, sizeof(m_SendDebugInfo));
	m_nSendDebugInfoCount = 0;
	//memset(m_SendActorDebugInfo, 0, sizeof(m_SendActorDebugInfo));
	//m_nSendActorDebugInfoCount = 0;

	_ACTOR_SENDINFO Info;
	memset(&Info, 0, sizeof(_ACTOR_SENDINFO));
	for (int i = 0; i < eActor::SC_ACTOR_STATE_MAX; i++)
		m_vActorSendInfo.push_back(Info);
	m_bActorDebugSended = false;
#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER
}

CDNUserSession::~CDNUserSession()
{
	for( DNVector(_STORE_PACKET)::iterator ii = m_pStoredPacketList.begin(); ii != m_pStoredPacketList.end(); ii++)
	{
		if( (*ii).pMsg )
			CLfhHeap::GetInstance()->_DeAlloc((*ii).pMsg);
	}
}

const char * CDNUserSession::GetIp()
{
	if(m_pTcpConnection)
		return m_pTcpConnection->GetIp();
	return NULL;
}

const WCHAR * CDNUserSession::GetIpW()
{
	if(m_pTcpConnection)
		return m_pTcpConnection->GetwszIp();
	return NULL;
}

int CDNUserSession::GetServerID()
{
	// 2009.01.20 kimbap
	// Fixed improper return value
	if(m_pGameServer)
		return m_pGameServer->GetServerID();
	return -1;
}

int CDNUserSession::GetRoomID()
{
	if(m_pGameRoom) return m_pGameRoom->GetRoomID();
	return 0;
}

TPARTYID CDNUserSession::GetPartyID()
{
	return m_pGameRoom->GetPartyIndex();
}

void CDNUserSession::SetSessionState(int nState)
{
	if(m_iState == SESSION_STATE_DISCONNECTED) return;	//Do not change state if final user has already been called.
	m_iState = nState;

	if (m_iState == SESSION_STATE_LOADED)
		m_bLoadUserData = true;
}

int CDNUserSession::GetPromotionValue(int nType)
{
	std::vector <TUserPromotionData>::iterator ii;
	for(ii = m_vPromotionData.begin(); ii != m_vPromotionData.end(); ii++)
	{
		if((*ii).nType == nType)
			return(*ii).nValue;
	}
	return 0;
}

void CDNUserSession::SetKeepOperationID(DWORD dwUniqueID)
{
	m_dwKeepOperationID = dwUniqueID;
}

bool CDNUserSession::IsConnected()
{
	if( m_bIsAIBot )
		return (m_iState != SESSION_STATE_DISCONNECTED);

	return(m_iState >= SESSION_STATE_CONNECTED) &&(m_iState != SESSION_STATE_DISCONNECTED) &&
		(m_pTcpConnection != NULL && m_pTcpConnection->GetDelete() == false) ? true : false;
}

bool CDNUserSession::RUDPConnected(int iNetID)
{
	_ASSERT(iNetID != 0);
	if(m_iState >= SESSION_STATE_RUDP_CONNECTED || iNetID == 0)
	{
		g_Log.Log(LogType::_ERROR, this, L"Connect|ReConnected RUDP err NetID:%d\n", iNetID);
		return false;
	}

	m_iNetID = iNetID;
	SetSessionState(SESSION_STATE_RUDP_CONNECTED);
	return true;
}

bool CDNUserSession::TCPConnected(CDNTcpConnection * pCon)
{
	if(m_iState != SESSION_STATE_RUDP_CONNECTED)
	{
		g_Log.Log(LogType::_ERROR, this, L"Connect|RUDP Connected Not Yet ADBID[%d] CNAME[%s]", GetAccountDBID(), GetCharacterName());
		return false;
	}

	if(g_pMasterConnectionManager->SendEnterGame(m_nWorldSetID, GetAccountDBID(), GetGameRoom()->GetRoomID(), m_pGameServer->GetServerID()) == false)
	{
		g_Log.Log(LogType::_ERROR, this, L"Connect|MasterCon Not Found ADBID[%d] CNAME[%s]", GetAccountDBID(), GetCharacterName());
		return false;
	}

	m_pTcpConnection = pCon;
	SetSessionState(SESSION_STATE_CONNECTED);

	m_bCertified = false;
	m_dwCertifyingTick = timeGetTime();

	g_pAuthManager->QueryCheckAuth(GetCertifyingKey(), this);

	InitVoice();

#ifdef PRE_MOD_INDUCE_TCPCONNECT

#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

	return true;
}

void CDNUserSession::RudpDisConnected()
{
	//Only RUDP disconnected. Need to check TCP connection and room/session state to determine if it should actually disconnect
	if(IsConnected() && m_iState != SESSION_STATE_SERVER_CHANGE)		//Do not change state for now. Trigger RUDP reconnection.
	{
		SendReConnectReq();

		m_bRudpDisconnected = true;
		m_nRudpDisconnectedTick = GetGameRoom()->GetGameTick();	//Record the disconnection time.
	}
} 

void CDNUserSession::_SendReconnectLogin()
{
	if( m_pGameRoom->GetRoomState() != _GAME_STATE_SYNC2PLAY )
		g_pMasterConnectionManager->SendLoginState(m_nWorldSetID, m_nAccountDBID);
	else {
		SendReconnectLogin(ERROR_GENERIC_INVALIDREQUEST, 0, 0);
	}
}

void CDNUserSession::SetPromotionData(int nPromotionType, int nValue)
{
	bool bAdded = false;
	std::vector <TUserPromotionData>::iterator ii;
	for(ii = m_vPromotionData.begin(); ii != m_vPromotionData.end(); ii++)
	{
		if((*ii).nType == nPromotionType)
		{
			(*ii).nValue = (*ii).nValue < nValue ? nValue :(*ii).nValue;
			bAdded = true;
		}
	}

	if(bAdded == false)
	{
		TUserPromotionData Promo;
		memset(&Promo, 0, sizeof(TUserPromotionData));

		Promo.nType = nPromotionType;
		Promo.nValue = nValue;
		// Fatigue reduction type uses a 10000 scale, so multiply by 100.
		if( Promo.nType == PROMOTIONTYPE_DECREASEFATIGUE )
			Promo.nValue *= 100;

		m_vPromotionData.push_back(Promo);
	}
}

void CDNUserSession::IntendedDisconnected()
{
	m_bIntededDisconnected = true;
}

void CDNUserSession::DetachConnection(wchar_t *pwszIdent)
{
	if(m_iState == SESSION_STATE_DISCONNECTED) return;
	
	// For drop-in, if before pushback into m_VecMember, call RemoveMember() immediately.
	// Do not comment this out!
	if(GetGameRoom())
	{
		if( GetGameRoom()->bIsBreakIntoUser( this ) )
		{
			if( GetGameRoom()->GetPartyData(this) == NULL )
			{
				GetGameRoom()->RemoveMember(this, pwszIdent);
				if(GetTcpConnection())
					GetTcpConnection()->DetachConnection(pwszIdent);
				return;
			}
		}
	}

	if(GetTcpConnection())
		GetTcpConnection()->DetachConnection(pwszIdent);
	else if(GetGameRoom())
		GetGameRoom()->RemoveMember(this, pwszIdent);
	else _DANGER_POINT();

#ifdef _USE_VOICECHAT
	LeaveVoiceChannel();
	if(GetGameRoom())
		GetGameRoom()->SendRefreshVoiceInfo(GetAccountDBID());
	else _DANGER_POINT();
#endif
}

#ifdef PRE_MOD_INDUCE_TCPCONNECT
void CDNUserSession::SendReqTCPConnect(DWORD dwCurTick)
{
	if (GetState() != SESSION_STATE_RUDP_CONNECTED) return;
	if (m_bRecvReqTCPConnectMsg == true) return;
	if (m_dwSendReqTCPConnect == 0 || m_dwSendReqTCPConnect + 3000 < dwCurTick)
	{
		m_dwSendReqTCPConnect = dwCurTick;
		SendPacket(SC_SYSTEM, eSystem::SC_TCP_CONNECT_REQ, NULL, 0, _RELIABLE);
		g_Log.Log(LogType::_GAMECONNECTLOG, this, L"[%d] CS_CONNECT_REQUEST\n", g_Config.nManagedID);
	}
}

void CDNUserSession::RecvReqTCPConnect()
{
	g_Log.Log(LogType::_GAMECONNECTLOG, this, L"[%d] CS_TCP_CONNECT_REQ\n", g_Config.nManagedID);
}
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT

void CDNUserSession::DoUpdate( DWORD dwCurTick )
{
	if( m_bIsAIBot )
		return;
#ifdef PRE_MOD_INDUCE_TCPCONNECT
	if (GetGameRoom()->GetRoomState() == _GAME_STATE_READY2CONNECT || GetGameRoom()->GetRoomState() == _GAME_STATE_CONNECT2CHECKAUTH)
	{
		SendReqTCPConnect(dwCurTick);
		return;
	}
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
	if( bIsCheckPing() == true )
	{
		if( dwCurTick-m_dwCheckUdpPing >= CHECK_UDP_PING_TICK )
		{
			m_dwCheckUdpPing = dwCurTick;
			SendUdpPing( GetSessionID(), dwCurTick );
		}
	}

#if defined( PRE_ALTEIAWORLD_EXPLORE )
//	if( m_dwAlteiaWorldMoveNextMapTick > 0 && dwCurTick - m_dwAlteiaWorldMoveNextMapTick >= AlteiaWorld::Common::MoveNextMapTick)
//	{
//		MoveAlteiaNextMap();
//	}
#endif

	CDNUserBase::DoUpdate(dwCurTick);
}

void CDNUserSession::DecreaseFatigue( bool bFinalUser/*=false*/ )
{
	if( m_bIsAIBot )
		return;
	if( GetGameRoom() && GetGameRoom()->GetMasterRewardSystem() )	
	{
		if( GetGameRoom()->GetMasterRewardSystem()->bIsFatigueReward( this ) )
		{
			m_iTotalMasterDecreaseFatigue += GetDecreaseFatigue(); // Accumulate fatigue
			return;
		}
	}

	int iDecreaseFatigue = GetDecreaseFatigue();
	if( iDecreaseFatigue > 0 )
	{
		// Only for FinalUser, do not deduct fatigue for users disconnected during loading.
		if( bFinalUser && !bIsLoadingComplete() )
		{
			g_Log.Log(LogType::_LOADINGDISCONNECT, this, L"Disconnected during loading; FTG not deducted. Fatigue(Normal:%d Weekly:%d PCBang:%d Event:%d VIP:%d)\r\n", GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(),GetEventFatigue());
			return;
		}
		if( CDnWorld::GetInstance( GetGameRoom() ).GetMapType() == EWorldEnum::MapTypeDungeon )
		{
			int iPrevValue = iDecreaseFatigue;

			// Decrease level
			int iLevelDecreaseValue = (iDecreaseFatigue*g_pDataManager->GetFatigue10000RatioFromPlayerCommonLevelTable( GetLevel() )/10000);
			// Reduce max level
			int iMaxLevelDecreaseValue = (iDecreaseFatigue*GetPromotionValue(PROMOTIONTYPE_DECREASEFATIGUE)/10000);

			iDecreaseFatigue -= (iLevelDecreaseValue+iMaxLevelDecreaseValue);
			if( iDecreaseFatigue < 0 )
				iDecreaseFatigue = 0;

			if( iDecreaseFatigue > 0 )
				DecreaseFatigue( iDecreaseFatigue );
		}
		SetDecreaseFatigue(0);
	}
}

void CDNUserSession::MasterMessageProcess(int iMainCmd, int iSubCmd, const void * pData, int iLen)
{//When external message must be processed at session layer.
	switch(iMainCmd)
	{
		case MAGA_NEXTVILLAGEINFO:
		{
			MAGANextVillageInfo * pPacket = (MAGANextVillageInfo*)pData;

			SetWindowState(WINDOW_NONE);
			GetGameRoom()->SendNextVillageInfo(pPacket->szIP, pPacket->nPort, pPacket->nMapIdx, pPacket->nNextMapIdx, pPacket->nNextGateIdx, pPacket->nRet, pPacket->nItemSerial);

			break;
		}

		case MAGA_REBIRTHVILLAGEINFO:
		{
			MAGARebirthVillageInfo *pInfo = (MAGARebirthVillageInfo*)pData;

			if( pInfo->nRet == ERROR_NONE )
			{
				_strcpy( m_szNextVillageIP, _countof(m_szNextVillageIP), pInfo->szIP, (int)strlen(pInfo->szIP));
				m_nNextVillagePort = pInfo->nPort;
				
				m_bCharOutLog = false;

				SetSessionState(SESSION_STATE_READY_TO_VILLAGE);

				m_biCertifyingKey = g_pAuthManager->GetCertifyingKey();
				DN_ASSERT(0 != m_biCertifyingKey,	"Invalid!");	// Auth key must not be generated as 0! (0 means none)

				g_pAuthManager->QueryStoreAuth(SERVERTYPE_GAME, this);
			}
			else 
			{
				SendVillageInfo(m_szNextVillageIP, m_nNextVillagePort, pInfo->nRet, 0, 0);
				_SendReconnectLogin();
			}

			if(m_pGameRoom)
			{
				GetGameRoom()->OnDelPartyMember(GetSessionID(), m_cKickKind);
#if defined(PRE_ADD_REVENGE)
				if( m_pGameRoom->bIsPvPRoom() && !GetPvPGameModeFinish() )
					static_cast<CDNPvPGameRoom*>(m_pGameRoom)->OnLeaveRoomBeforeFinish(GetSessionID());
#endif
			}

			break;
		}
		case MAGA_MOVEPVPGAMETOPVPLOBBY:
		{
			MAGAMovePvPGameToPvPLobby* pPacket = (MAGAMovePvPGameToPvPLobby*)pData;

			if( pPacket->nRet == ERROR_NONE )
			{
				// Notify leaving game
				if( m_pGameRoom )
#if defined(PRE_ADD_REVENGE)
				{
					m_pGameRoom->OnDelPartyMember(GetSessionID(), -1);

					if( m_pGameRoom->bIsPvPRoom() && !GetPvPGameModeFinish() )
						static_cast<CDNPvPGameRoom*>(m_pGameRoom)->OnLeaveRoomBeforeFinish(GetSessionID());
				}
#else
					m_pGameRoom->OnDelPartyMember(GetSessionID(), -1);
#endif
				else
					_DANGER_POINT();

				_strcpy( m_szNextVillageIP, _countof(m_szNextVillageIP), pPacket->szIP, (int)strlen(pPacket->szIP) );
				m_nNextVillagePort = pPacket->unPort;

				m_bCharOutLog = false;

				SetSessionState(SESSION_STATE_READY_TO_VILLAGE);

				m_biCertifyingKey = g_pAuthManager->GetCertifyingKey();
				DN_ASSERT(0 != m_biCertifyingKey,	"Invalid!");	// Auth key must not be generated as 0! (0 means none)

				g_pAuthManager->QueryStoreAuth(SERVERTYPE_GAME, this);
			}
			else {
				SendVillageInfo( m_szNextVillageIP, m_nNextVillagePort, pPacket->nRet, 0, 0);
			}
			break;
		}

		case MAGA_LOGINSTATE:
		{
			MAGALoginState *pState = (MAGALoginState*)pData;

			if(ERROR_NONE != pState->nRet) {
				SendReconnectLogin(pState->nRet, 0, 0);
				break;
			}

			SetSessionState(SESSION_STATE_READY_TO_LOGIN);

			m_biCertifyingKey = g_pAuthManager->GetCertifyingKey();
			DN_ASSERT(0 != m_biCertifyingKey,	"Invalid!");	// Auth key must not be generated as 0! (0 means none)

			g_pAuthManager->QueryStoreAuth(SERVERTYPE_GAME, this);
			
			if(m_pGameRoom)
			{
				GetGameRoom()->OnDelPartyMember(GetSessionID(), m_cKickKind);
#if defined(PRE_ADD_REVENGE)
				if( GetGameRoom()->bIsPvPRoom() && !GetPvPGameModeFinish() )
					static_cast<CDNPvPGameRoom*>(GetGameRoom())->OnLeaveRoomBeforeFinish(GetSessionID());
#endif
			}

			break;
		}

		case MAGA_FRIENDADDNOTICE:
		{
			MAGAFriendAddNotice * pNotice = (MAGAFriendAddNotice*)pData;
			SendFriendAddNotice(pNotice->wszAddName);
			break;
		}

		case MAGA_PRIVATECHAT:
		{
			MAPrivateChat *pChat = (MAPrivateChat*)pData;

			if(pChat->wChatLen > CHATLENMAX-1)	break;
			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pChat->wszChatMsg, pChat->wChatLen);

			if( m_pIsolate && m_pIsolate->IsIsolateItem(pChat->wszFromCharacterName))		//Ignore messages from blocked characters
				break;

			if( pChat->nRet == ERROR_NONE )
			{
				// If GM, treat as if not present
				if((GetAccountLevel() <= AccountLevel_QA) &&(GetAccountLevel() >= AccountLevel_New))
				{
					if(!GetGMCommand()->IsWhisperName(pChat->wszFromCharacterName))
					{
						break;
					}
				}
			}

			SendChat(pChat->cType, pChat->wChatLen, pChat->wszFromCharacterName, wszChatMsg, NULL, pChat->nRet);
			break;
		}

		case MAGA_CHAT:
		{
			MAChat *pChat = (MAChat*)pData;

			if(pChat->wChatLen > CHATLENMAX-1 || pChat->wChatLen <= 0)	break;
			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pChat->wszChatMsg, pChat->wChatLen);

			SendChat(static_cast<eChatType>(pChat->cType), pChat->wChatLen, pChat->wszFromCharacterName, wszChatMsg);
			break;
		}

		case MAGA_DETACHUSER:
		{
			MADetachUser* pDetach = (MADetachUser*)pData;
			if( pDetach->bIsDuplicate )		
			{
				bool bIsDetach = true;
				if( GetGameRoom() && GetGameRoom()->IsRoomCrash() )				
					// If crash room, send request to reset Auth.
					bIsDetach = false;

				g_pMasterConnectionManager->SendDuplicateLogin(GetWorldSetID(), pDetach->nAccountDBID, bIsDetach, pDetach->nSessionID);
				if( GetGameRoom() )
					g_Log.Log(LogType::_NORMAL, GetWorldSetID(), GetAccountDBID(), GetCharacterDBID(), GetSessionID(), L"[MAGA_DETACHUSER] Duplicate User InGame!! RoomState:%d, SessionState:%d, RoomCrash:%d\r\n", GetGameRoom()->GetRoomState(), m_iState, GetGameRoom()->IsRoomCrash());
				else
					g_Log.Log(LogType::_NORMAL, GetWorldSetID(), GetAccountDBID(), GetCharacterDBID(), GetSessionID(), L"[MAGA_DETACHUSER] Duplicate User InGame!! SessionState:%d\r\n", m_iState);

#if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
				// When disconnecting due to duplicate login, first request the client to disconnect.
				// Server disconnects only when DetachFlag is On (if client is tampered or stops receiving packets, a disconnect request for duplicate login may arrive again)
				if(bIsDetach && GetTcpConnection() && !GetTcpConnection()->GetDetachFlag())
					SendCompleteDetachMsg( ERROR_DUPLICATEUSER_INGAME, L"ERROR_DUPLICATEUSER_INGAME", true);
				else
					DetachConnection(L"MAGA_DETACHUSER | DUPLICATEUSER");

				break;
#endif	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
			}
			DetachConnection(L"MAGA_DETACHUSER");
			break;
		}


		case MAGA_NOTIFYMAIL:
		{
			MANotifyMail *pMail = (MANotifyMail*)pData;

			if(pMail->biToCharacterDBID == m_biCharacterDBID)
				SendNotifyMail(pMail->wTotalMailCount, pMail->wNotReadMailCount, pMail->w7DaysLeftCount, pMail->bNewMail);	// Notify of incoming mail
			break;
		}

		case MAGA_NOTIFYMARKET:
		{
			MANotifyMarket *pMarket = (MANotifyMarket*)pData;

			if(pMarket->biSellerCharacterDBID == m_biCharacterDBID)
				SendNotifyMarket(pMarket->nItemID, pMarket->wCalculationCount);
			break;
		}

		case MAGA_NOTIFYGIFT:
		{
			MANotifyGift *pGift = (MANotifyGift*)pData;

			if(pGift->biToCharacterDBID == GetCharacterDBID())
				SendCashshopNotifyGift(pGift->bNew, pGift->nGiftCount);		// Notify that a gift arrived
			break;
		}

		case MAGA_RESTRAINT:
		{
			MARestraint * pPacket = (MARestraint*)pData;
			GetDBConnection()->QueryGetRestraint( this );
			break;
		}

		case MAGA_PCBANGRESULT:
		{
			MAPCBangResult *pResult = (MAPCBangResult*)pData;
			PCBangResult(pResult);
			break;
		}

		case MAGA_INVITEPARTYMEMBERRESULT:
		{
			MAGAInvitePartyMemberResult * pPacket = (MAGAInvitePartyMemberResult*)pData;

			GetGameRoom()->ResetInvite(pPacket->nRetCode);
			SendPartyInviteFail(pPacket->nRetCode);
			break;
		}
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
		case MAGA_GETPARTYID:
		{
			MAGAGetPartyID* pPacket = (MAGAGetPartyID*)(pData);
			if(g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendGetPartyIDResult(GetGameRoom()->GetWorldSetID(), pPacket->nSenderAccountDBID, GetPartyID());
			break;
		}
		case MAGA_REQPARTYASKJOIN:
		{
			MAGAReqPartyAskJoin* pPacket = (MAGAReqPartyAskJoin*)(pData);
			if( GetPartyID() <= 0 )
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND, pPacket->nReqAccountDBID );
				break;
			}
			// Check if world zone
			const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( GetMapIndex() );
			if( pMapInfo == NULL || pMapInfo->MapType != GlobalEnum::MAP_WORLDMAP )
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_FAIL, pPacket->nReqAccountDBID );
				break;
			}
			CDNUserSession* pLeader = GetGameRoom()->GetUserSessionByCharDBID((GetGameRoom()->GetPartyStructData().biLeaderCharacterDBID));
			if( pLeader == NULL )
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_FAIL, pPacket->nReqAccountDBID );
				break;
			}
			if( pLeader->GetIsolate()->IsIsolateItem(pPacket->wszReqChracterName) )
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_FAIL, pPacket->nReqAccountDBID );
				break;
			}
			if( pLeader->IsAcceptAbleOption(pPacket->biReqCharacterDBID, pPacket->nReqAccountDBID, _ACCEPTABLE_CHECKTYPE_PARTYINVITE) == false)
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_FAIL, pPacket->nReqAccountDBID );
				break;
			}
			if( !pLeader->IsNoneWindowState() )
			{
				if(g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendResPartyAskJoin( GetGameRoom()->GetWorldSetID(), ERROR_PARTY_ASKJOIN_DESTUSER_SITUATION_NOTALLOWED, pPacket->nReqAccountDBID );
				break;
			}

			pLeader->SetWindowState(WINDOW_ISACCEPT);
			pLeader->SendAskJoinToLeader(pPacket->wszReqChracterName, pPacket->cReqUserJob, pPacket->cReqUserLevel);
			break;
		}		
#endif

		case MAGA_USERTEMPDATA_RESULT:
		{
			MAGAUserTempDataResult * pPacket = (MAGAUserTempDataResult*)pData;
			SetDungeonClearCount(pPacket->nDungeonClearCount);
#if defined(PRE_ADD_REMOTE_QUEST)
			GetQuest()->RefreshRemoteQuest(pPacket->nAcceptWaitRemoteQuestCount, pPacket->AcceptWaitRemoteQuestList);
#endif		
#if defined(PRE_ADD_GAMEQUIT_REWARD)
			SetUserGameQuitRewardType(pPacket->eUserGameQuitRewardType);
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
			break;
		}
		case MAGA_PVP_MEMBERINDEX:
		{
			MAGAPvPMemberIndex * pPacket = (MAGAPvPMemberIndex*)pData;

			if(GetGameRoom()->bIsPvPRoom())
			{
				if(pPacket->nRetCode == ERROR_NONE)
				{
					CDNPvPGameRoom * pPvPRoom = static_cast<CDNPvPGameRoom*>(GetGameRoom());
					if(pPvPRoom)
					{
						CSPartySwapMemberIndex packet;
						memset(&packet, 0, sizeof(CSPartySwapMemberIndex));

						packet.cCount = pPacket->cCount;
						for (int h = 0; h < packet.cCount; h++)
						{
							packet.Index[h].cIndex = pPacket->Index[h].cIndex;
							packet.Index[h].nSessionID = pPacket->Index[h].nSessionID;
						}

						if(pPvPRoom->PartySwapMemberIndex(&packet) == false)
							_DANGER_POINT();			//Waaaaah
						pPvPRoom->SendMemberIndex(pPacket->nTeam, pPacket->cCount, pPacket->Index, pPacket->nRetCode);
					}
				}
				else
					SendPvPMemberIndex(pPacket->nTeam, 0, NULL, pPacket->nRetCode);
			}
			else
				_DANGER_POINT();
			break;
		}

		case MAGA_PVP_MEMBERGRADE:
		{
			MAGAPvPMemberGrade * pPacket = (MAGAPvPMemberGrade*)pData;

			if(GetGameRoom()->bIsPvPRoom())
			{
				CDNPvPGameRoom * pPvPRoom = static_cast<CDNPvPGameRoom*>(GetGameRoom());
				if(pPvPRoom)
					pPvPRoom->SendMemberGrade(pPacket->nTeam, pPacket->uiUserState, pPacket->nChangedSessionID, pPacket->nRetCode);
			}
			else _DANGER_POINT();
			break;
		}
	}
}

void CDNUserSession::OnDBRecvAuth(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvAuth(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvCharInfo(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvCharInfo(nSubCmd, pData);

	switch(nSubCmd)
	{
	case QUERY_SELECTCHARACTER:
		{
			TASelectCharacter *pSelect = (TASelectCharacter*)pData;
			if(pSelect->nRetCode != ERROR_NONE){
				DetachConnection(L"QUERY_SELECTCHARACTER");
#ifdef _DEBUG
				g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] QUERY_SELECTCHARACTER Ret:%d\r\n", pSelect->nAccountDBID, m_biCharacterDBID, m_nSessionID, pSelect->nRetCode);
#endif
				return;
			}

			if(!m_pGameRoom){
				DetachConnection(L"QUERY_SELECTCHARACTER m_pGameRoom = NULL");
#ifdef _DEBUG
				g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] QUERY_SELECTCHARACTER Session not Found\r\n", pSelect->nAccountDBID, m_biCharacterDBID, m_nSessionID);
#endif
				return;
			}

#if defined( PRE_ADD_CHARACTERCHECKSUM )
			if( m_bTutorial && pSelect->tLastLoginDate <= pSelect->tLastLogoutDate && pSelect->uiCheckSum )
			{
				UINT uiCheckSum = MakeCharacterCheckSum( pSelect->biCharacterDBID, pSelect->UserData.Status.cLevel, pSelect->UserData.Status.nExp, pSelect->UserData.Status.nCoin, pSelect->UserData.Status.nWarehouseCoin );
				if( uiCheckSum != pSelect->uiCheckSum )
				{
					if( m_pDBCon )
					{
						std::wstring wszRestraintReason = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8418);
						std::wstring wszRestraintReasonForDolis = GetEtUIXML().GetUIString(CEtUIXML::idCategory1, 8419);

						// Apply sanctions once stabilized.
						//m_pDBCon->QueryAddRestraint(this, DBDNWorldDef::RestraintTargetCode::Account, DBDNWorldDef::RestraintTypeCode::ConnectRestraint, wszRestraintReason.c_str(), wszRestraintReasonForDolis.c_str(), 9999);
					}
					//DetachConnection(L"QUERY_SELECTCHARACTER Invalid Character CheckSum");
					g_Log.Log(LogType::_ERROR, this, L"Invalid Character CheckSum(cid:%I64d, level:%d, exp:%d, coin:%I64d, wcoin:%I64d, mychecksum:%x, dbchecksum:%x)\r\n", pSelect->biCharacterDBID, pSelect->UserData.Status.cLevel, pSelect->UserData.Status.nExp, pSelect->UserData.Status.nCoin, pSelect->UserData.Status.nWarehouseCoin, uiCheckSum, pSelect->uiCheckSum );
					//return;
				}
			}
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )

			LoadUserData(pSelect);

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
			_ASSERT( m_pReputationSystem && m_pReputationSystem->GetEventHandler() );
			m_pReputationSystem->GetEventHandler()->OnConnect( pSelect->UserData.Status.tLastConnectDate, false );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

			GetDBConnection()->GetCountReceiveMail(this);	// Request mail and market count
			GetDBConnection()->QueryGetCountHarvestDepotItem( this );	// Request farm warehouse count


#ifdef _DEBUG
			g_Log.Log(LogType::_NORMAL, this, L"[ADBID:%u CDBID:%I64d SID:%u] QUERY_SELECTCHARACTER\r\n", pSelect->nAccountDBID, m_biCharacterDBID, m_nSessionID);
#endif
		}
		break;

	case QUERY_CHANGESERVERUSERDATA:
		{
			TAUpdateCharacter *pUpdate = (TAUpdateCharacter*)pData;
			if(pUpdate->nRetCode != ERROR_NONE){

#ifdef _DEBUG
				g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] QUERY_UPDATECHANGESERVER Ret:%d\r\n", pUpdate->nAccountDBID, m_biCharacterDBID, m_nSessionID, pUpdate->nRetCode);
#endif
				return;
			}

			// Save temporary data on Master before user changes channel.
			if(g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendSaveUserTempData(GetWorldSetID(), this);

			SendVillageInfo(m_szNextVillageIP, m_nNextVillagePort, ERROR_NONE, GetAccountDBID(), GetCertifyingKey());
			SetSessionState(SESSION_STATE_SERVER_CHANGE);
			if( GetGameRoom() )
				GetGameRoom()->SetGoToVillageFlag( true );
		}
		break;

	case QUERY_CHANGESTAGEUSERDATA:
		{
			TAChangeStageUserData *pUpdate = (TAChangeStageUserData*)pData;

			if(pUpdate->nRetCode != ERROR_NONE){

#ifdef _DEBUG
				g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] QUERY_CHANGESTAGEUSERDATA Ret:%d\r\n", pUpdate->nAccountDBID, m_biCharacterDBID, m_nSessionID, pUpdate->nRetCode);
#endif
				return;
			}
		}
		break;

	case QUERY_GETCHARACTERPARTIALYBYDBID:
	case QUERY_GETCHARACTERPARTIALYBYNAME:
		{
			TAGetCharacterPartialy * pPacket = (TAGetCharacterPartialy*)pData;
			switch(pPacket->cReqType)
			{
			case _REQCHARACTERPARTIALY_FRIEND:
				{
					if(pPacket->nRetCode != ERROR_NONE) return;
					if(pPacket->biCharacterDBID > 0)
					{
						TFriend * pFriend = m_pFriend->GetFriend(pPacket->biCharacterDBID);
						if(pFriend == NULL) return;
						sWorldUserState State;
						memset(&State, 0, sizeof(sWorldUserState));
						if(g_pWorldUserState->GetUserState(pFriend->wszCharacterName, pFriend->biFriendCharacterDBID, &State) == false)
							State.nLocationState = _LOCATION_NONE;
						if(pFriend != NULL)
							SendFriendDetailInfo(pFriend->biFriendCharacterDBID, pFriend->nGroupDBID, pPacket->cClass, pPacket->cLevel, \
							pPacket->cJob, &State, NULL, pFriend->wszFriendMemo);
						else
							_DANGER_POINT();
					}
				}
			}
		}
		break;

	case QUERY_OWNCHRACTERLEVEL:
		{
			//Let's check how many max-level characters there are
			int nMaxLevelCount = GetMaxLevelCharacterCount();

			//Since level list is obtained, set up the required info.
			const TPromotionData * pPromo = NULL;
			int nPromotionCount = g_pDataManager->GetPromotionCount();

			for (int i = 0; i < nPromotionCount; i++)
			{
				pPromo = g_pDataManager->GetPromotionByIdx(i);
				if (pPromo && pPromo->nConditionType == PROMOTION_CONDTIONTYPE_MAXLEVEL)
				{
					if (pPromo->nConditionValue <= nMaxLevelCount)
						SetPromotionData(pPromo->nPromotionType, pPromo->nRewardValue);
				}
			}
		}
		break;
	}
}

void CDNUserSession::OnDBRecvEtc(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvEtc(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvQuest(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvQuest(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvMission(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvMission(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvSkill(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvSkill(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvFriend(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvFriend(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvIsolate(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvIsolate(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvPvP(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvPvP(nSubCmd, pData);

	switch(nSubCmd)
	{
		case QUERY_UPDATEPVPDATA:
		{
			TAUpdatePvPData* pPacket = reinterpret_cast<TAUpdatePvPData*>(pData);

			if( pPacket->nRetCode != ERROR_NONE )
				_DANGER_POINT();

			switch( pPacket->Type )
			{
				case PvPCommon::QueryUpdatePvPDataType::FinishGameMode:
				{
					if( GetGameRoom() && GetGameRoom()->GetPvPGameMode() )
						GetGameRoom()->GetPvPGameMode()->SendFinishGameMode( this );
					else
						_DANGER_POINT();
					break;
				}
				case PvPCommon::QueryUpdatePvPDataType::GoPvPLobby:
				{
					SendPvPGameToPvPLobby();
					break;
				}
				case PvPCommon::QueryUpdatePvPDataType::GoSelectChar:
				{
					_SendReconnectLogin();
					break;
				}
#if defined(PRE_MOD_PVP_LADDER_XP)
				case PvPCommon::QueryUpdatePvPDataType::FinishLadder:
				{
					break;
				}
#endif
			}
			break;
		}
		case QUERY_ADD_PVP_LADDERRESULT:
		{
			TAAddPvPLadderResult* pPacket = reinterpret_cast<TAAddPvPLadderResult*>(pData);

			if( pPacket->nRetCode != ERROR_NONE )
				_DANGER_POINT();

			switch( pPacket->Type )
			{
				case PvPCommon::QueryUpdatePvPDataType::FinishGameMode:
				{
					if( GetGameRoom() && GetGameRoom()->GetPvPGameMode() )
						GetGameRoom()->GetPvPGameMode()->SendFinishGameMode( this );
					else
						_DANGER_POINT();
					break;
				}
				case PvPCommon::QueryUpdatePvPDataType::GoPvPLobby:
				{
					SendPvPGameToPvPLobby();
					break;
				}
				case PvPCommon::QueryUpdatePvPDataType::GoSelectChar:
				{
					_SendReconnectLogin();
					break;
				}
			}
			break;
		}
		case QUERY_GETLIST_PVP_LADDERSCORE:
		{
			TAGetListPvPLadderScore* pA = reinterpret_cast<TAGetListPvPLadderScore*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				if( GetGameRoom()->bIsLadderRoom() )
				{
					LadderSystem::CStatsRepository* pRepository = static_cast<CDNPvPGameRoom*>(GetGameRoom())->GetLadderStatsRepositoryPtr();
					if( pRepository )
					{
						pRepository->OnRecvLadderScore( GetCharacterDBID(), GetTeam(), pA );
					}
					else
						_ASSERT(0);
				}
			}
			break;
		}
#if defined( PRE_FIX_76282 )
		case SYNC_GOPVPLOBBY:
		{
			TAHeader * pA = reinterpret_cast<TAHeader*>(pData);
			if( pA->nRetCode == ERROR_NONE )
			{
				SendPvPGameToPvPLobby();
			}
			break;
		}
#endif // #if defined( PRE_FIX_76282 )
	}
}

void CDNUserSession::OnDBRecvDarkLair(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvDarkLair(nSubCmd, pData);
}


void CDNUserSession::OnDBRecvGuild(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvGuild(nSubCmd, pData);

	switch(nSubCmd)
	{
	case QUERY_CREATEGUILD:				// Guild creation result
		{
			// GA does not process this packet
		}
		DN_BREAK;

	case QUERY_DISMISSGUILD:			// Guild disband result
		{
			// GA does not process this packet
		}
		DN_BREAK;

	case QUERY_ADDGUILDMEMBER:			// Add guild member result
		{
			// GA does not process this packet
		}
		DN_BREAK;

	case QUERY_DELGUILDMEMBER:			// Guild member removal (leave/kick) result
		{
			// GA does not process this packet
		}
		DN_BREAK;

	case QUERY_CHANGEGUILDINFO:			// Guild info change result
		{
			const TAChangeGuildInfo *pPacket = reinterpret_cast<TAChangeGuildInfo*>(pData);
			if(ERROR_NONE != pPacket->nRetCode) {
				// Error occurred
				SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, pPacket->nRetCode, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
				break;
			}

			const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);

			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

			if(!pGuild) 
			{
				SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, ERROR_GUILD_NOTEXIST_GUILDINFO, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
				break;
			}

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if(FALSE == pGuild->IsEnable())
			{
				SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, ERROR_GUILD_NOTEXIST_GUILDINFO, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
				break;
			}
#endif

			pGuild->UpdateGuildInfo(this, pPacket);
		}
		break;

	case QUERY_CHANGEGUILDMEMBERINFO:		// Guild member info update result
		{				  
			const TAChangeGuildMemberInfo *pPacket = reinterpret_cast<TAChangeGuildMemberInfo*>(pData);
			if(ERROR_NONE != pPacket->nRetCode) {
				// Error occurred
				SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, pPacket->nRetCode, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
				break;
			}
			DN_ASSERT(GetAccountDBID() == pPacket->nReqAccountDBID,	"Invalid!");

			const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);

			CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);

			if(!pGuild) 
			{
				SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, ERROR_GENERIC_UNKNOWNERROR, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
				break;
			}

#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if(FALSE == pGuild->IsEnable()) break;
#endif

			pGuild->UpdateMemberInfo(this, pPacket);
		}
		break;

	case QUERY_GETGUILDHISTORYLIST:		// Guild history list result
		{
			// GA does not process this packet
		}
		DN_BREAK;
	case  QUERY_CHANGEGUILDNAME:		// Change guild name
		{
			const TAChangeGuildName *pPacket = reinterpret_cast<TAChangeGuildName*>(pData);
			if(ERROR_NONE != pPacket->nRetCode)
			{
				SendGuildRenameResult(pPacket->nRetCode);
				break;
			}
			// Delete item, send guild name change 
			int nItemID = 0;
			const TItem *pItem = m_pItem->GetCashInventory(pPacket->biItemSerial);
			if(pItem)
				nItemID = pItem->nItemID;

			if(m_pItem->DeleteItemByUse(ITEMPOSITION_CASHINVEN, -1, pPacket->biItemSerial))
			{
				GetEventSystem()->OnEvent( EventSystem::OnItemUse, 1, EventSystem::ItemID, nItemID );
				const TGuildUID GuildUID(pPacket->cWorldSetID, pPacket->nGuildDBID);
				g_pMasterConnectionManager->SendGuildChangeName(GetWorldSetID(), GuildUID, pPacket->wszGuildName);
				SendGuildRenameResult(ERROR_NONE);
			}
		}
		break;
	}
}

void CDNUserSession::OnDBRecvMail(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvMail(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvMarket(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvMarket(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvItem(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvItem(nSubCmd, pData);

	switch( nSubCmd )
	{
		case QUERY_GETPAGEMATERIALIZEDITEM:
		{
			int nRemain = m_pItem->GetCashInventoryTotalCount() - m_pItem->GetCashInventoryCount();
			if( nRemain <= 0)
			{
				TItem *pCashItem = m_pItem->GetCashItemByType(ITEMTYPE_EXPAND);
				if(pCashItem)
					m_nExpandNestClearCount = g_pDataManager->GetItemTypeParam1(pCashItem->nItemID);

#if defined(PRE_ADD_TSCLEARCOUNTEX)
				pCashItem = m_pItem->GetCashItemByType(ITEMTYPE_EXPAND_TS);
				if(pCashItem)
					m_nExpandTreasureStageClearCount = g_pDataManager->GetItemTypeParam1(pCashItem->nItemID);
#endif	// #if defined(PRE_ADD_TSCLEARCOUNTEX)

				// Vehicle Inventory
				GetDBConnection()->QueryGetPageVehicle(this, 1, VEHICLEINVENTORYPAGEMAX);
			}
			break;
		}

		case QUERY_GETPAGEVEHICLE:
		{
			if(m_pItem->IsCompleteLimitlessItem())
				SetSessionState(SESSION_STATE_LOADED);
			break;
		}
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
		case QUERY_CHECK_NAMEDITEMCOUNT:
		{
			TACheckNamedItemCount* pA = reinterpret_cast<TACheckNamedItemCount*>(pData);
			if( m_pGameRoom && m_pGameRoom->GetTaskMng() )
			{
				CDnGameTask* pTask = (CDnGameTask *)m_pGameRoom->GetTaskMng()->GetTask( "GameTask" );
				if( pTask )
				{
					if( pTask->GetDungeonClearState() >= CDnGameTask::DungeonClearStateEnum::DCS_SelectRewardItemStay && pTask->GetDungeonClearState() <= CDnGameTask::DungeonClearStateEnum::DCS_RewardItemStay )
					{
						pTask->CheckSelectNamedItemResult( this, pA );
					}
				}				
			}			
		}
		break;
#endif
	}
}

void CDNUserSession::OnDBRecvCash(int nSubCmd, char *pData)
{
	CDNUserBase::OnDBRecvCash(nSubCmd, pData);
}

void CDNUserSession::OnDBRecvMasterSystem(int nSubCmd, char *pData)
{
	switch( nSubCmd )	
	{
		case QUERY_GET_COUNTINFO:
		{
			TAGetMasterSystemCountInfo* pA = reinterpret_cast<TAGetMasterSystemCountInfo*>(pData);

			if( GetGameRoom() )
			{
				if( GetGameRoom()->GetMasterRewardSystem() )
					GetGameRoom()->GetMasterRewardSystem()->Update( this, pA );
			}
			return;
		}
		case QUERY_MOD_RESPECTPOINT:
		{
			TAModRespectPoint* pA = reinterpret_cast<TAModRespectPoint*>(pData);
			SendMasterSystemRespectPoint( pA->iRespectPoint );
			return;
		}
		case QUERY_MOD_FAVORPOINT:
		{
			TAModMasterFavorPoint* pA = reinterpret_cast<TAModMasterFavorPoint*>(pData);
			SendMasterSystemFavorPoint( pA->biMasterCharacterDBID, pA->biPupilCharacterDBID, pA->iFavorPoint );
			// Add favorability.
			GetGameRoom()->GetMasterRewardSystem()->UpdateFavor(this, pA->biPupilCharacterDBID, pA->iFavorPoint );

			// Notify FavorPoint to pupil
			if( GetGameRoom() )
			{
				for( UINT i=0 ;i<GetGameRoom()->GetUserCount() ; ++i )
				{
					CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
					if( pSession && pSession->GetCharacterDBID() == pA->biPupilCharacterDBID )
					{
						pSession->SendMasterSystemFavorPoint( pA->biMasterCharacterDBID, pA->biPupilCharacterDBID, pA->iFavorPoint );
						pSession->GetDBConnection()->QueryGetMasterSystemSimpleInfo( pSession->GetDBThreadID(), pSession, true, MasterSystem::EventType::DungeonClear );
						pSession->QueryGetMasterSystemCountInfo( true );
					}
				}
			}
			return;
		}
		case QUERY_GET_MASTERCHARACTER_TYPE1:
		{
			TAGetMasterCharacterType1* pA = reinterpret_cast<TAGetMasterCharacterType1*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMasterCharacter( this, pA );
			return;
		}
		case QUERY_GET_PUPILLIST:
		{
			TAGetListPupil* pA = reinterpret_cast<TAGetListPupil*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetPupilList( this, pA );
			return;
		}
		case QUERY_GET_MASTERANDCLASSMATE:
		{
			TAGetListMyMasterAndClassmate* pA = reinterpret_cast<TAGetListMyMasterAndClassmate*>(pData);
			MasterSystem::CCacheRepository::GetInstance().SetMasterClassmateList( this, pA );
			return;
		}
	}

	// Pass packets not handled at the UserSession level to UserBase.
	CDNUserBase::OnDBRecvMasterSystem(nSubCmd, pData);
}

#if defined( PRE_PARTY_DB )
void CDNUserSession::OnDBRecvParty( int nSubCmd, char* pData )
{
	// Pass packets not handled at the UserSession level to UserBase.
	CDNUserBase::OnDBRecvParty(nSubCmd, pData);
}
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_ALTEIAWORLD_EXPLORE )
void CDNUserSession::OnDBRecvAlteiaWorld(int nSubCmd, char* pData)
{
	CDNUserBase::OnDBRecvAlteiaWorld(nSubCmd, pData);
	switch( nSubCmd )
	{	
	case QUERY_ADD_ALTEIAWORLDSENDTICKETLIST:
		{	
			const TAAddAlteiaWorldSendTicketList* pPacket = reinterpret_cast<TAAddAlteiaWorldSendTicketList*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{				
				SendAlteiaWorldSendTicket( pPacket->wszSendCharacterName );

				if (g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendAddAlteiaWorldSendTicketResult( GetWorldSetID(), pPacket->nRetCode, pPacket->biSendCharacterDBID );
				
			}
			else
			{
				if (g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendAddAlteiaWorldSendTicketResult( GetWorldSetID(), pPacket->nRetCode, pPacket->biSendCharacterDBID );
				
			}
		}
		break;
	case QUERY_GET_ALTEIAWORLDINFO:
		{
			const TAGetAlteiaWorldInfo* pPacket = reinterpret_cast<const TAGetAlteiaWorldInfo*>(pData);
			if( pPacket->nRetCode == ERROR_NONE )
			{
				m_cDailyPlayCount = (BYTE)pPacket->nDailyPlayCount;
			}
		}
		break;
	default:
		break;
	}	
}
#endif // #if defined(PRE_ALTEIAWORLD_EXPLORE)

#if defined (PRE_ADD_BESTFRIEND)
void CDNUserSession::OnDBRecvBestFriend(int nSubCmd, char* pData)
{
	CDNUserBase::OnDBRecvBestFriend(nSubCmd, pData);
}
#endif // #if defined (PRE_ADD_BESTFRIEND)

int CDNUserSession::OnRecvCharMessage(int iSubCmd, char * pData, int iLen)
{
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	switch (iSubCmd)
	{
		case eChar::CS_STAGECLEAR_BONUSREWARD_SELECT:
		{
			int iResult = GetStageClearBonusReward(pData, iLen);
			SendStageClearBonusRewardResult(iResult);
			return iResult;
		}
		break;
	}
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	return CDNUserBase::OnRecvCharMessage(iSubCmd, pData, iLen);
}

int CDNUserSession::OnRecvTradeMessage(int iSubCmd, char * pData, int iLen)
{
	return CDNUserBase::OnRecvTradeMessage(iSubCmd, pData, iLen);
}

int CDNUserSession::OnRecvQuestMessage(int iSubCmd, char * pData, int iLen)
{
	return CDNUserBase::OnRecvQuestMessage(iSubCmd, pData, iLen);
}

int CDNUserSession::OnRecvSystemMessage(int iSubCmd, char * pData, int iLen)
{
	switch(iSubCmd)
	{
	case eSystem::CS_RECONNECTLOGIN:	// Select character
		{
			if(m_iState != SESSION_STATE_GAME_PLAY)	 
				return ERROR_NONE;

			if( GetGameRoom() )
				GetGameRoom()->DelLastPartyDungeonInfo( this );

			BackUpEquipInfo();
	
			if( m_pGameRoom->bIsPvPRoom() )
			{
				if( !GetPvPGameModeFinish() && m_pGameRoom->GetPvPGameMode() )
				{
					if( m_pGameRoom->GetPvPGameMode()->bIsPlayingUser( GetActorHandle() ) )
					{
						if( m_pGameRoom->bIsZombieMode() )
						{
							_SendReconnectLogin();
							return ERROR_NONE;
						}
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
						if( m_pGameRoom->bIsComboExerciseMode() )
						{
							_SendReconnectLogin();
							return ERROR_NONE;
						}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
						IScoreSystem* pScoreSystem = m_pGameRoom->GetPvPGameMode()->GetScoreSystem();
						if( pScoreSystem )
						{
							if( pScoreSystem->QueryUpdatePvPData( PvPCommon::Team::Max, this, PvPCommon::QueryUpdatePvPDataType::GoSelectChar ) == false )
							{
								_DANGER_POINT();
								_SendReconnectLogin();
							}
#if defined( PRE_ADD_RACING_MODE)
							if( m_pGameRoom->GetPvPGameMode()->bIsRacingMode() )
							{
								_SendReconnectLogin();
							}
#endif
							return ERROR_NONE;
						}
					}
				}				
			}
			else
			{
				// Must apply stage abandon penalty: reduce durability
				DnActorHandle hActor = GetActorHandle();
				if( hActor ) 
				{
					CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
					pPlayer->OnStageGiveUp();
				}
			}

			ChangeStageUserData();

			_SendReconnectLogin();
			return ERROR_NONE;
		}
		break;

	case eSystem::CS_READY_2_RECIEVE:
		{
			CSReady2Recieve *pPacket = (CSReady2Recieve*)pData;

			if(sizeof(CSReady2Recieve) != iLen)
				return ERROR_INVALIDPACKET;

			wcsncpy(m_wszVirtualIp, pPacket->wszVirtualIp, IPLENMAX);
			WideCharToMultiByte(CP_ACP, 0, m_wszVirtualIp, -1, m_szVirtualIp, IPLENMAX, NULL, NULL);

			SetSessionState(SESSION_STATE_READY_TO_SYNC);
			SendUserInfo();
			return ERROR_NONE;
		}

	case eSystem::CS_INTENDED_DISCONNECT:
		{
			//Normal exit or in-game; block by state for now, excluding unconnected users and handling only mid-game disconnects
			//if(m_GameState >= _GAME_STATE_SYNC2SYNC && m_GameState <= _GAME_STATE_PLAY)
			//	pSession->IntendedDisconnected();
			return ERROR_NONE;
		}

	case eSystem::CS_ABANDONSTAGE:	// Abandon stage
	{
		CDnGameTask *pTask = ( m_pGameRoom ) ? m_pGameRoom->GetGameTask() : NULL;
		if (!pTask) return ERROR_GENERIC_INVALIDREQUEST;
		if( pTask->IsWaitPlayCutScene() ) return ERROR_GENERIC_INVALIDREQUEST;

		CSAbandonStage * pPacket = (CSAbandonStage*)pData;

		if(iLen != sizeof(CSAbandonStage))
			return ERROR_INVALIDPACKET;

		int nResult = ERROR_NONE;
		nResult = CmdAbandonStage( true, pPacket->bIsPartyOut, pPacket->bIntenedDisconnect );
		if( nResult == ERROR_NONE ) {
			ChangeStageUserData();
		}
		return nResult;
	}

		// Request move from PvP game to PvP lobby
	case eSystem::CS_MOVE_PVPGAMETOPVPLOBBY:
		{
			if(iLen != 0)
				return ERROR_INVALIDPACKET;

			if( m_iState != SESSION_STATE_GAME_PLAY )
				return ERROR_NONE;

			BackUpEquipInfo();

			// If exited normally...
			if( !GetPvPGameModeFinish() )
			{
				if( m_pGameRoom && m_pGameRoom->GetPvPGameMode() )
				{
					if( m_pGameRoom->GetPvPGameMode()->bIsPlayingUser( GetActorHandle() ) )
					{
#if defined( PRE_FIX_76282 )
						bool bSyncGoPvP = false;
#endif // // #if defined( PRE_FIX_76282 )
						if( m_pGameRoom->GetPvPGameMode()->bIsZombieMode() )
						{
#if defined( PRE_FIX_76282 )
							bSyncGoPvP = true;
#else // #if defined( PRE_FIX_76282 )
							SendPvPGameToPvPLobby();
							return ERROR_NONE;
#endif // // #if defined( PRE_FIX_76282 )
						}
#if defined( PRE_ADD_RACING_MODE)
						if( m_pGameRoom->GetPvPGameMode()->bIsRacingMode() )
						{
#if defined( PRE_FIX_76282 )
							bSyncGoPvP = true;
#else // #if defined( PRE_FIX_76282 )
							m_pItem->RemoveInstantEquipVehicleData(true);
							m_pItem->RemoveInstantVehicleItemData(true);
							SendPvPGameToPvPLobby();
							return ERROR_NONE;
#endif // #if defined( PRE_FIX_76282 )
						}
#endif // #if defined( PRE_ADD_RACING_MODE)
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
						if( m_pGameRoom->GetPvPGameMode()->bIsComboExerciseMode() )
						{
#if defined( PRE_FIX_76282 )
							bSyncGoPvP = true;
#else // #if defined( PRE_FIX_76282 )
							SendPvPGameToPvPLobby();
							return ERROR_NONE;
#endif // #if defined( PRE_FIX_76282 )
						}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

						// Applying #76282: if query is sent here, must prevent duplicate query in CPvPScoreSystem::QueryUpdatePvPData.
						IScoreSystem* pScoreSystem = m_pGameRoom->GetPvPGameMode()->GetScoreSystem();
						if( pScoreSystem )
						{
							if( pScoreSystem->QueryUpdatePvPData( PvPCommon::Team::Max, this, PvPCommon::QueryUpdatePvPDataType::GoPvPLobby ) == false )
							{
								_DANGER_POINT();
								SendPvPGameToPvPLobby();
							}
#if defined( PRE_FIX_76282 )
							if( bSyncGoPvP && m_pDBCon )
							{
								m_pDBCon->SendSyncGoPvPLobby( this );
							}
#endif // #if defined( PRE_FIX_76282 )
							return ERROR_NONE;
						}
					}
				}
			}

			SendPvPGameToPvPLobby();
			return ERROR_NONE;
		}

	case eSystem::CC_PEER_CONNECT_REQUEST:
			//{
			//	CCPeerConnectRequest * pPacket = (CCPeerConnectRequest*)pData;
			//	if(pPacket->cState == 1)		//PeerConnected Msg
			//	{
			//		if(m_PeerManager.IsConnectedPeer(pPacket->nSessionID[0], pPacket->nSessionID[1]) == false)
			//			m_PeerManager.ConnectPeer(pPacket->nSessionID[0], pPacket->nSessionID[1]);
			//		else
			//			_DANGER_POINT();
			//	}	
			//	else if(pPacket->cState == 2)		//PeerDisConnected Msg
			//	{
			//		if(m_PeerManager.IsConnectedPeer(pSession->GetSessionID(), pPacket->nSessionID[1]) == true)
			//			m_PeerManager.DisConnectPeer(pSession->GetSessionID(), pPacket->nSessionID[1]);
			//		else
			//			_DANGER_POINT();
			//	}
			//}
			return ERROR_NONE;

#ifdef PRE_MOD_INDUCE_TCPCONNECT
	case eSystem::CS_CONNECTGAME:
			{
				m_bRecvReqTCPConnectMsg = true;
				g_Log.Log( LogType::_GAMECONNECTLOG, L"MessageProcess CS_CONNECTGAME!!\n" );
				return ERROR_NONE;
			}	
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
		}

	return CDNUserBase::OnRecvSystemMessage(iSubCmd, pData, iLen);
}

int CDNUserSession::OnRecvFriendMessage(int nSubCmd, char * pData, int nLen)
{
	return CDNUserBase::OnRecvFriendMessage(nSubCmd, pData, nLen);
}

int CDNUserSession::OnRecvIsolateMessage(int nSubCmd, char * pData, int nLen)
{
	return CDNUserBase::OnRecvIsolateMessage(nSubCmd, pData, nLen);
}

int CDNUserSession::OnRecvGameOptionMessage(int nSubCmd, char * pData, int nLen)
{
	return CDNUserBase::OnRecvGameOptionMessage(nSubCmd, pData, nLen);
}

int CDNUserSession::OnRecvEtcMessage(int nSubCmd, char * pData, int nLen)
{
	switch( nSubCmd )
	{
		case eEtc::CS_DARKLAIR_RANK_BOARD:
		{
			// Not used on game server.
			// Might be used in the game server later, so handle Process in UserBase
			// Currently blocked at UserSession level on game server.
			_ASSERT(0);
			return ERROR_INVALIDPACKET;	// Disconnect if this request arrives at game server
		}
	}

	return CDNUserBase::OnRecvEtcMessage(nSubCmd, pData, nLen);
}

int CDNUserSession::OnRecvRadioMessage(int nSubCmd, char * pData, int nLen)
{
	switch(nSubCmd)
	{
	case eRadio::CS_USERADIO:
		{
			CSUseRadio * pPacket = (CSUseRadio*)pData;

			if(sizeof(CSUseRadio) != nLen)
				return ERROR_INVALIDPACKET;

			if( !GetActorHandle() )
				return ERROR_NONE;

			CDNGameRoom * pRoom = GetGameRoom();
			CDnPlayerActor * pActor = (CDnPlayerActor*)GetActorHandle().GetPointer();

			if(pRoom && pActor)
			{
				int nTeam = pActor->GetTeam();

				//#43512 Fix radio message not being delivered when a party member is in Ice Prison.
				//When Ice Prison is set, team inversion applies..
				if (pActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
					nTeam = pActor->GetOriginalTeam();

				for(DWORD i = 0; i < pRoom->GetUserCount(); i++) 
				{
					CDNUserSession * pSession = pRoom->GetUserData(i);
					if(pSession && pSession->GetActorHandle())
					{
						CDnPlayerActor * pDestActor = (CDnPlayerActor*)pSession->GetActorHandle().GetPointer();

						//#43512 Fix radio message not being delivered when a party member is in Ice Prison.
						//When Ice Prison is set, team inversion applies..
						int nDestActorTeam = pDestActor->GetTeam();
						if (pDestActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							nDestActorTeam = pDestActor->GetOriginalTeam();

						if(nTeam == nDestActorTeam)
							pSession->SendUseRadio(GetSessionID(), pPacket->nID);
					}
				}
				return ERROR_NONE;
			}
			else
				_DANGER_POINT();

			return ERROR_NONE;
		}
		break;
	}
	return CDNUserBase::OnRecvRadioMessage(nSubCmd, pData, nLen);
}

#ifdef _USE_VOICECHAT
int CDNUserSession::OnRecvVoiceChatMessage(int nSubCmd, char * pData, int nLen)
{
	switch(nSubCmd)
	{
	case eVoiceChat::CS_VOICEAVAILABLE:
		{
			CSVoiceChatAvailable * pPacket = (CSVoiceChatAvailable*)pData;

			if(sizeof(CSVoiceChatAvailable) != nLen)
				return ERROR_INVALIDPACKET;

			m_bVoiceAvailable = pPacket->cAvailable > 0 ? true : false;
			if(m_bVoiceAvailable)
			{
				if(m_nVoiceChannelID > 0) return ERROR_NONE;
				if(GetGameRoom() && GetGameRoom()->m_nVoiceChannelID[0] > 0)
					JoinVoiceChannel(GetGameRoom()->m_nVoiceChannelID[0], GetIp(), NULL, 0);
			}
			else
				LeaveVoiceChannel();

			//Refresh it
			if(GetGameRoom())
				GetGameRoom()->SendRefreshVoiceInfo();
			return ERROR_NONE;
		}
		break;

	case eVoiceChat::CS_VOICEMUTE:
		{
			CSVoiceMute * pPacket = (CSVoiceMute*)pData;

			if(sizeof(CSVoiceMute) != nLen)
				return ERROR_INVALIDPACKET;
			
			if(g_pVoiceChat && GetGameRoom()->m_nVoiceChannelID[0] > 0)
			{
				CDNUserSession * pSession = GetGameRoom()->GetUserSession(pPacket->nSessionID);
				if(pSession)
				{
					g_pVoiceChat->MuteOneToOne(GetAccountDBID(), pSession->GetAccountDBID(), pPacket->cMute == 0 ? false : true);
					//Looks like mute-related info is needed.
				}
				else
					_DANGER_POINT();
			}
			else
				_DANGER_POINT();
			return ERROR_NONE;
		}
		break;

	case eVoiceChat::CS_VOICECOMPLAINTREQ:
		{
			CSVoiceComplaintReq * pPacket = (CSVoiceComplaintReq*)pData;

			if(sizeof(CSVoiceComplaintReq) != nLen)
				return ERROR_INVALIDPACKET;

			if(g_pVoiceChat && GetGameRoom()->m_nVoiceChannelID[0] > 0)
			{
				CDNUserSession * pSession = GetGameRoom()->GetUserSession(pPacket->nSessionID);
				if(pSession)
				{
					g_pVoiceChat->ComplaintRequest(GetAccountDBID(), pSession->GetAccountDBID(), pSession->GetAccountDBID(), pPacket->szCategory, pPacket->szSubject, pPacket->szMsg);
				}
				else
					_DANGER_POINT();
			}
			else
				_DANGER_POINT();
			return ERROR_NONE;
		}
		break;
	}
	return ERROR_UNKNOWN_HEADER;
}
#endif

#if defined( PRE_ADD_SECONDARY_SKILL )

int CDNUserSession::OnRecvSecondarySkillMessage( int nSubCmd, char* pData, int nLen )
{
	switch( nSubCmd )
	{
		case eSecondarySkill::CS_DELETE:
		{
			// Check world zone
			if( CDnWorld::GetInstance( GetGameRoom() ).GetMapType() != EWorldEnum::MapTypeWorldMap )
			{
				SecondarySkill::CSDelete* pPacket = reinterpret_cast<SecondarySkill::CSDelete*>(pData);
				SendDelSecondarySkill( ERROR_SECONDARYSKILL_DELETE_FAILED_DONTALLOWMAP, pPacket->iSkillID );
				return ERROR_GENERIC_INVALIDREQUEST;
			}

			break;
		}
	}

	return CDNUserBase::OnRecvSecondarySkillMessage( nSubCmd, pData, nLen );
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

int CDNUserSession::OnRecvChatRoomMessage(int nSubCmd, char *pData, int nLen)
{
	// Chat room can only be used on the farm.
	if(!m_pGameRoom->bIsFarmRoom())
		return ERROR_INVALIDPACKET;
	
	switch( nSubCmd )
	{
	case eChatRoom::CS_CREATECHATROOM:
		{
			CSCreateChatRoom * pCreateChatRoom = (CSCreateChatRoom *)pData;
			if(sizeof(CSCreateChatRoom) != nLen)	return ERROR_INVALIDPACKET;

			TChatRoomView ChatRoomView;

			// 0. Is already in another chat room?
			if( m_nChatRoomID > 0 )
			{
				SendChatRoomCreateChatRoom( ERROR_CHATROOM_ANOTHERROOM, ChatRoomView );
				return ERROR_NONE;
			}

			int nChatRoomID = 0;
			// 1. Create room
			nChatRoomID = m_pGameRoom->m_ChatRoomManager.CreateChatRoom( GetSessionID(), pCreateChatRoom->wszName, pCreateChatRoom->nRoomType,
				pCreateChatRoom->nRoomAllow, pCreateChatRoom->wszPassword, pCreateChatRoom->PRLine1, pCreateChatRoom->PRLine2, pCreateChatRoom->PRLine3, m_pGameRoom );

			if( nChatRoomID == 0 )
			{
				// Room creation failed - ERROR_CHATROOM_FAILEDMAKEROOM
				SendChatRoomCreateChatRoom( ERROR_CHATROOM_FAILEDMAKEROOM, ChatRoomView );
				return ERROR_NONE;
			}

			// Find room
			CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( nChatRoomID );
			if( pChatRoom == NULL )
			{
				// Room creation failed - ERROR_CHATROOM_FAILEDMAKEROOM
				SendChatRoomCreateChatRoom( ERROR_CHATROOM_FAILEDMAKEROOM, ChatRoomView );
				return ERROR_NONE;
			}

		
			// Room display info
			ChatRoomView.Set( pChatRoom->GetChatRoomID(), pChatRoom->GetChatRoomName(), pChatRoom->GetChatRoomType(), pChatRoom->UsePassword(),
				pChatRoom->GetChatRoomPRLine1(), pChatRoom->GetChatRoomPRLine2(), pChatRoom->GetChatRoomPRLine3() );

			// 2. Leader enters
			int nRet = ERROR_NONE;
			nRet = pChatRoom->EnterRoom( m_nSessionID, pCreateChatRoom->wszPassword, true );

			if( nRet != ERROR_NONE )
			{
				// Destroy room because leader failed to enter.
				if( pChatRoom->GetUserCount() <= 0 )
				{
					m_pGameRoom->m_ChatRoomManager.DestroyChatRoom( pChatRoom->GetChatRoomID() );
				}
			}

			// Send result - sent to the character who created the room.
			SendChatRoomCreateChatRoom( nRet, ChatRoomView );

			// Send to nearby characters
			m_pGameRoom->BroadcastChatRoomView(this, ChatRoomView);

			return ERROR_NONE;
		}
		break;
	case eChatRoom::CS_ENTERCHATROOM:
		{
			CSChatRoomEnterRoom * pEnterRoom = (CSChatRoomEnterRoom *)pData;
			if(sizeof(CSChatRoomEnterRoom) != nLen)	return ERROR_INVALIDPACKET;

			int nRet = ERROR_NONE;
			int UserSessionIDs[CHATROOMMAX];
			int nUserCount = 0;
			int LeaderSID = 0;
			memset( UserSessionIDs, 0, sizeof(UserSessionIDs) );

			// Find room
			CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( pEnterRoom->nChatRoomID );
			if( pChatRoom == NULL )
			{
				SendChatRoomEnterRoom( ERROR_CHATROOM_INVALIDCHATROOM, LeaderSID, UserSessionIDs, nUserCount );
				return ERROR_NONE;
			}

			// Normal user entry
			nRet = pChatRoom->EnterRoom( m_nSessionID, pEnterRoom->wszPassword, false );
			if( nRet == ERROR_NONE )
			{
				// Room master's session ID
				if( pChatRoom->GetLeaderID() > 0 )
				{
					CDNUserSession * pLeaderSession = m_pGameRoom->GetUserSession(pChatRoom->GetLeaderID());
					if( pLeaderSession )	LeaderSID = pLeaderSession->GetSessionID();
				}

				// Session IDs of participants
				for( int i=0; i<CHATROOMMAX; i++ )
				{
					if( pChatRoom->GetMemberAIDFromIndex(i) <= 0 )	 continue;
					CDNUserSession * pUserSession =  m_pGameRoom->GetUserSession( pChatRoom->GetMemberAIDFromIndex(i) );
					if( pUserSession == NULL )						 continue;

					UserSessionIDs[ nUserCount ] = pUserSession->GetSessionID();
					nUserCount++;
				}
			}

			// Send result
			SendChatRoomEnterRoom( nRet, LeaderSID, UserSessionIDs, nUserCount );

			if( nRet == ERROR_NONE )
			{
				// Send to nearby characters
				m_pGameRoom->BroadcastChatRoomEnterUser(this, pChatRoom);
				// Send profile
				if( pChatRoom->GetChatRoomType() == CHATROOMTYPE_PR && GetProfile()->bOpenPublic )
					pChatRoom->SendProfileToAll( GetSessionID(), *GetProfile() );
			}

			return ERROR_NONE;
		}
		break;
	case eChatRoom::CS_CHANGEROOMOPTION:
		{
			CSChatRoomChangeRoomOption * pChangeOption = (CSChatRoomChangeRoomOption*)pData;
			if(sizeof(CSChatRoomChangeRoomOption) != nLen)	return ERROR_INVALIDPACKET;

			// Find room
			CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( m_nChatRoomID );
			if( pChatRoom == NULL )
			{
				SendChatRoomChangeRoomOption( ERROR_CHATROOM_INVALIDCHATROOM, pChangeOption->ChatRoomView );
				return ERROR_NONE;
			}

			// Cannot change room options if not the room master.
			if( pChatRoom->IsLeader( m_nSessionID ) == false )
			{
				SendChatRoomChangeRoomOption( ERROR_CHATROOM_ONLYLEADER, pChangeOption->ChatRoomView );
				return ERROR_NONE;
			}

			pChatRoom->ChangeRoomOption( pChangeOption->ChatRoomView.m_wszChatRoomName, pChangeOption->ChatRoomView.m_nChatRoomType, 
				pChangeOption->nRoomAllow, pChangeOption->wszPassword,
				pChangeOption->ChatRoomView.m_wszChatRoomPRLine1,
				pChangeOption->ChatRoomView.m_wszChatRoomPRLine2,
				pChangeOption->ChatRoomView.m_wszChatRoomPRLine3 );

			TChatRoomView RoomView;
			RoomView.Set( pChatRoom->GetChatRoomID(), pChatRoom->GetChatRoomName(), pChatRoom->GetChatRoomType(), pChatRoom->UsePassword(),
				pChatRoom->GetChatRoomPRLine1(), pChatRoom->GetChatRoomPRLine2(), pChatRoom->GetChatRoomPRLine3() );

			SendChatRoomChangeRoomOption( ERROR_NONE, RoomView );

			// Send to nearby characters
			m_pGameRoom->BroadcastChatRoomView(this, RoomView);

			return ERROR_NONE;
		}
		break;
	case eChatRoom::CS_KICKUSER:
		{
			CSChatRoomKickUser * pKickUser = (CSChatRoomKickUser *)pData;
			if(sizeof(CSChatRoomKickUser) != nLen)	return ERROR_INVALIDPACKET;

			int nRet = ERROR_NONE;

			// Find room
			CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( m_nChatRoomID );
			if( pChatRoom == NULL )
			{
				SendChatRoomKickUser( ERROR_CHATROOM_INVALIDCHATROOM, pKickUser->nKickUserSessionID	);
				return ERROR_NONE;
			}

			// Cannot use kick command if not the room master.
			if( pChatRoom->IsLeader( m_nSessionID ) == false )
			{
				SendChatRoomKickUser( ERROR_CHATROOM_ONLYLEADER, pKickUser->nKickUserSessionID	);
				return ERROR_NONE;
			}

			// Cannot kick self.
			if( GetSessionID() == pKickUser->nKickUserSessionID )
			{
				return ERROR_NONE;
			}

			CDNUserSession *pUserSession = m_pGameRoom->GetUserSession(pKickUser->nKickUserSessionID);
			if( pUserSession == NULL )
			{
				return ERROR_NONE;
			}

			// Delete user
			nRet = pChatRoom->LeaveUser( pUserSession->GetSessionID(), pKickUser->nKickReason );
			if( nRet == ERROR_NONE )
			{
				// Send message to the kicked character
				pUserSession->SendChatRoomLeaveUser( pUserSession->GetSessionID(), pKickUser->nKickReason );

				// Send to nearby characters
				m_pGameRoom->BroadcastChatRoomLeaveUser(pUserSession, pKickUser->nKickReason);
			}

			return ERROR_NONE;
		}
		break;
	case eChatRoom::CS_LEAVEROOM:
		{
			if(0 != nLen)	return ERROR_INVALIDPACKET;

			int nRet = ERROR_NONE;

			CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( m_nChatRoomID );
			if( pChatRoom == NULL )
			{
				return ERROR_NONE;
			}

			if( pChatRoom->IsLeader(GetSessionID()) )
			{
				// Room is deleted if the leader leaves the room.
				while( pChatRoom->GetUserCount() > 0 )
				{
					UINT UserAID = pChatRoom->GetMemberAIDFromIndex( 0 );
					if( UserAID <= 0 )	break;	// Break out of loop to prevent infinite loop.

					CDNUserSession * pUserSession =  m_pGameRoom->GetUserSession( UserAID );
					if( pUserSession == NULL )	break;	// Break out of loop to prevent infinite loop.

					nRet = pChatRoom->LeaveUser( pUserSession->GetSessionID(), CHATROOMLEAVE_DESTROYROOM );
					if( nRet == ERROR_NONE )
					{
						// Send a message to the character leaving the room and nearby characters.
						if( pChatRoom->IsLeader( GetSessionID() ) )
						{
							TChatRoomView RoomView;
							RoomView.Set( 0, L"", CHATROOMTYPE_NONE, false, L"", L"", L"" );
							pUserSession->SendChatRoomLeaveUser( pUserSession->GetSessionID(), CHATROOMLEAVE_DESTROYROOM );
							m_pGameRoom->BroadcastChatRoomView( pUserSession, RoomView );
						}
						else
						{
							pUserSession->SendChatRoomLeaveUser( pUserSession->GetSessionID(), CHATROOMLEAVE_DESTROYROOM );
							m_pGameRoom->BroadcastChatRoomLeaveUser( pUserSession, CHATROOMLEAVE_DESTROYROOM );
						}
					}
				}
				// Room destroyed!
				m_pGameRoom->m_ChatRoomManager.DestroyChatRoom( pChatRoom->GetChatRoomID() );
			}
			else
			{
				// User left the room
				nRet = pChatRoom->LeaveUser( GetSessionID(), CHATROOMLEAVE_LEAVE );
				if( nRet == ERROR_NONE )
				{
					// Send message to the character who left the room
					SendChatRoomLeaveUser( GetSessionID(), CHATROOMLEAVE_LEAVE );

					// Send to nearby characters
					m_pGameRoom->BroadcastChatRoomLeaveUser( this, CHATROOMLEAVE_LEAVE );
				}
			}

			return ERROR_NONE;
		}
		break;
	}

	return ERROR_UNKNOWN_HEADER;
}

int CDNUserSession::OnRecvPvPMessage(int nSubCmd, char * pData, int nLen)
{
	switch(nSubCmd)
	{
	case ePvP::CS_SWAPMEMBERINDEX:
		{
			CSPvPTeamSwapMemberIndex * pPacket = (CSPvPTeamSwapMemberIndex*)pData;
			if(g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendPvPSwapMemberIndex(GetWorldSetID(), GetAccountDBID(), pPacket->cCount, pPacket->Index);
			else
				SendPvPMemberIndex(0, 0, NULL, ERROR_PVP_SWAPTEAM_MEMBERINDEX_FAIL);
			return ERROR_NONE;
		}

	case ePvP::CS_GUILDWAR_CHANGEMEMBER_GRADE:
		{
			CSPvPGuildWarChangeMemberGrade * pPacket = (CSPvPGuildWarChangeMemberGrade*)pData;

			if(sizeof(CSPvPGuildWarChangeMemberGrade) != nLen)
				return ERROR_INVALIDPACKET;

			if(g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendPvPChangeMemberGrade(GetWorldSetID(), GetAccountDBID(), pPacket->nType, pPacket->nSessionID, pPacket->bAsign);
			else
				SendPvPGuildWarMemberGrade(0, 0, ERROR_PVP_CHANGEMEMBERGRADE_FAIL);
			return ERROR_NONE;
		}

	case ePvP::CS_CONCENTRATE_ORDER:
		{
			CSOrderConcentrate * pPacket = (CSOrderConcentrate*)pData;

			if(sizeof(CSOrderConcentrate) != nLen)
				return ERROR_INVALIDPACKET;

			if(!(GetPvPUserState()&PvPCommon::UserState::GuildWarCaptain) && !(GetPvPUserState()&PvPCommon::UserState::GuildWarSedcondCaptain))
				return ERROR_NONE;

			for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
			{
				CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
				if(pSession)
					pSession->SendGuildWarConcentrateOrder(GetSessionID(), pPacket->vPosition);
			}
			return ERROR_NONE;
		}
#if defined( PRE_WORLDCOMBINE_PVP )
	case ePvP::CS_PVP_WORLDPVPROOM_STARTMSG:
		{
			WorldPvPMissionRoom::CSWorldPvPRoomStartMsg * pPacket = (WorldPvPMissionRoom::CSWorldPvPRoomStartMsg*)pData;

			if(sizeof(WorldPvPMissionRoom::CSWorldPvPRoomStartMsg) != nLen)
				return ERROR_INVALIDPACKET;
			
			if( GetGameRoom()->bIsWorldPvPRoom() && !GetGameRoom()->bIsWorldPvPRoomStart() )
			{
				CPvPGameMode * pMode = GetGameRoom()->GetPvPGameMode();
				if( pMode )
				{
					if( pMode->CheckWorldPvPRoomMinMemberCount() )
					{
						if( GetGameRoom()->GetWorldPvPRoomReqType() == WorldPvPMissionRoom::Common::GMRoom )
						{
							// Check team setup
						}
						GetGameRoom()->SetWorldPvPRoomStart(true);
					}
					else
					{
						SendWorldPvPRoomStartResult(ERROR_GENERIC_INVALIDREQUEST);
						GetGameRoom()->GetPvPGameMode()->SetSendStartMsg(false);
					}
				}
			}
			return ERROR_NONE;
		}
		break;
#endif
	}	
	return ERROR_UNKNOWN_HEADER;
}

#if defined( PRE_PRIVATECHAT_CHANNEL )
int CDNUserSession::OnRecvPrivateChatChannelMessage(int nSubCmd, char * pData, int nLen)
{	
	return CDNUserBase::OnRecvPrivateChatChannelMessage( nSubCmd, pData, nLen );
}
#endif

#if defined( PRE_ALTEIAWORLD_EXPLORE )
int CDNUserSession::OnRecvWorldAlteiaMessage(int nSubCmd, char *pData, int nLen)
{		
	switch( nSubCmd )
	{
	case eAlteiaWorld::CS_ALTEIAWORLD_DICE:
		{			
			int nRet = AlteiaWorldDice();
			if( nRet != ERROR_NONE )
			{
				SendAlteiaWorldDiceResult( nRet, 0 );
			}
		}
		break;
	case eAlteiaWorld::CS_ALTEIAWORLD_NEXTMAP:
		{			
			MoveAlteiaNextMap();
		}
		break;
	}			
	return CDNUserBase::OnRecvWorldAlteiaMessage( nSubCmd, pData, nLen );
}
#endif


void CDNUserSession::SendPacket(int iMainCmd, int iSubCmd, const void * pMsg, int iLen, int iPrior)
{
	if( m_bIsAIBot )
		return;
#ifdef PRE_ADD_PACKETSIZE_CHECKER
	if (m_nSendCheckTick == 0)
		m_nSendCheckTick = timeGetTime();
	unsigned long CurTick = timeGetTime();

	m_nSendCheckSize += iLen;
	m_nSendCheckCount++;

	int cnt = m_nSendDebugInfoCount++&127;
	m_SendDebugInfo[cnt]._DebugMainCmd = iMainCmd;
	m_SendDebugInfo[cnt]._DebugSubCmd = iSubCmd;
	m_SendDebugInfo[cnt]._DebugTick = CurTick;
	m_SendDebugInfo[cnt]._DebugSize = iLen;

	if (m_bActorDebugSended == false && m_nSendCheckTick + 10000 < CurTick && GetGameRoom())
	{
		if (m_nSendCheckSize > (1024 * 500) || m_nSendCheckCount > 10000)
		{
			g_Log.Log(LogType::_ERROR, L"[%d] Report|Frame SendSize SID[%d] Class[%d] Job[%d] MapIndex[%d] AvrCur[%d] Sum[%d] Cnt[%d]\n", g_Config.nManagedID, GetSessionID(), GetClassID(), GetUserJob(), GetGameRoom()->GetGameTask()->GetMapTableID(), m_nSendCheckSize / m_nSendCheckCount, m_nSendCheckSize, m_nSendCheckCount);

			USES_CONVERSION;
			std::wstring wstrLogText;
			bool bSended = false;
			int nAddedCount = 0;
			for (int nLogCout = 0; nLogCout < (int)m_vActorSendInfo.size(); nLogCout++)
			{
				if (bSended == false)
				{
					wstrLogText.clear();
					wstrLogText = L"[";
					wstrLogText += I2W(g_Config.nManagedID);
					wstrLogText += L"] Report|Frame SendActorInfo ";
					bSended = true;
				}

				if (m_vActorSendInfo[nLogCout].nSendCount <= 0)
					continue;

				wstrLogText += L"[SC:";
				wstrLogText += I2W(nLogCout);
				wstrLogText += L" SCC:";
				wstrLogText += I2W(m_vActorSendInfo[nLogCout].nSendCount);
				wstrLogText += L"]";
				nAddedCount++;

				if (nAddedCount >= 4)
				{
					wstrLogText += L"\n";
					g_Log.Log(LogType::_ERROR, L"%s\n", wstrLogText.c_str());

					bSended = false;
					nAddedCount = 0;
				}
			}
			m_bActorDebugSended = true;

			for (int nLogCount = 0; nLogCount < 128; nLogCount++)
			{
				g_Log.Log(LogType::_ERROR, L"[%d] Report|Frame SendDebugInfo MCmd[%d] SCmd[%d] Tick[%d] Size[%d]\n", g_Config.nManagedID, m_SendDebugInfo[nLogCount]._DebugMainCmd, m_SendDebugInfo[nLogCount]._DebugSubCmd, \
					m_SendDebugInfo[nLogCount]._DebugTick, m_SendDebugInfo[nLogCount]._DebugSize);//, m_SendActorDebugInfo[nLogCount]._dwUniqueID, m_SendActorDebugInfo[nLogCount]._ActorSubCmd, m_SendActorDebugInfo[nLogCount]._PacketBroadFlag);
			}
		}
		
		m_nSendCheckTick = CurTick;
		m_nSendCheckSize = 0;
		m_nSendCheckCount = 0;
	}
#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER

	//Except for a few things, they are sent over TCP~
	bool bStoreForSync = (m_pGameRoom->GetRoomState() == _GAME_STATE_SYNC2SYNC);
	if( !bStoreForSync &&
		m_pGameRoom->GetRoomState() == _GAME_STATE_SYNC2PLAY &&
		!bIsBreakIntoSession() &&
		!bIsLoadingComplete() &&
		GetState() != SESSION_STATE_READY_TO_PLAY &&
		iMainCmd != SC_SYSTEM )
	{
		// Keep packets queued until this client finishes stage loading and sends CS_SYNC_WAIT.
		// Otherwise an earlier-ready member (e.g. AI bot or faster party mate) broadcasting
		// SC_SYNC_WAIT flushes packets prematurely while this client is still on the previous stage.
		bStoreForSync = true;
	}

	if( bStoreForSync )
		StorePacket(iMainCmd, iSubCmd, (char*)pMsg, iLen);
	else
	{
		// Here are those few cases.. a bit cumbersome, but the dispatch layer is unified. The rest can be fired freely.
		// Note: Suspecting potion-drinking desync, in-game item packets are moved to UDP like actor messages..
		// UDP/TCP ordering is aligned, so it's safe to send freely while connected.
		bool bSendUdp = false;
		switch( iMainCmd ) {
			case SC_ACTOR:
			case SC_ACTORBUNDLE:
			case SC_PROP:
				bSendUdp = true;
				break;

			case SC_SYSTEM:
				switch( iSubCmd ) {
					case eSystem::SC_TCP_CONNECT_REQ:
						bSendUdp = true;
						break;
				}
				break;
			case SC_ROOM:
			{
				switch( iSubCmd )
				{
					case eRoom::SC_SYNC_MEMBERINFO:
					case eRoom::SC_SYNC_MEMBERTEAM:
					case eRoom::SC_SYNC_MEMBERDEFAULTPARTS:
					case eRoom::SC_SYNC_MEMBEREQUIP:
					case eRoom::SC_SYNC_MEMBERSKILL:
					case eRoom::SC_SYNC_MEMBER_BREAKINTO:
					case eRoom::SC_SYNC_MEMBERHPSP:
					case eRoom::SC_SYNC_MEMBERBATTLEMODE:
					case eRoom::SC_SYNC_DROPITEMLIST:
					case eRoom::SC_SYNC_DATUMTICK:
					case eRoom::SC_DLDUNGEONCLEAR_RANKINFO:
					case eRoom::SC_START_DRAGONNEST:
					case eRoom::SC_DUNGEONCLEAR_MSG:
					case eRoom::SC_DLDUNGEONCLEAR_MSG:
					{
						// Send PvP Sync packet over TCP.
						bSendUdp = false;
						break;
					}
					default:
					{
						bSendUdp = true;
						break;
					}
				}
				break;
			}
			case SC_QUEST:
				switch( iSubCmd ) {
					case eQuest::SC_PLAYCUTSCENE:
					case eQuest::SC_COMPLETE_CUTSCENE:
						bSendUdp = true;
						break;
				}
				break;

		}

		bool bSendOK		= true;
		BYTE cPrevPacketSeq = m_cPacketSeq;

		if( bSendUdp && iLen >= 512 )
		{
#ifdef _FINAL_BUILD
#else		//#ifdef _FINAL_BUILD
			g_Log.Log(LogType::_ERROR, this,  L"SendError Check Packet Size MCMD[%d] SCMD[%d] PL[%d]\n", iMainCmd, iSubCmd, iLen);
#endif		//#ifdef _FINAL_BUILD
			bSendUdp = false;
		}

		if (bSendUdp && iMainCmd != SC_ACTORBUNDLE && iPrior == _RELIABLE)
			FlushPacketQueue();

		//Only upload/send for reliable UDP and TCP
		BYTE cSeq = 0;
		if((bSendUdp && iPrior == _RELIABLE) || bSendUdp == false)
			cSeq = m_cPacketSeq += 2;


		if( bSendUdp ) 
		{
#ifdef _PACKET_COMP
			//if(m_pTcpConnection && m_pTcpConnection->GetIsUseComp()) m_pTcpConnection->SetPacketComp(false);
#endif

			if(m_pGameServer->Send(m_iNetID, iMainCmd, iSubCmd, pMsg, iLen, iPrior, cSeq) < 0)
			{
				bSendOK = false;
#ifdef _DEBUG
				if (iMainCmd == SC_SYSTEM && iSubCmd == eSystem::SC_TCP_CONNECT_REQ)
					g_Log.Log(LogType::_ERROR, this,  L"UDP SendError NETID[%d] MCMD[%d] SCMD[%d] PL[%d] PRIOR[%d]\n", m_iNetID, iMainCmd, iSubCmd, iLen, iPrior);
#endif
				//g_Log.Log(LogType::_ERROR, this,  L"UDP SendError NETID[%d] MCMD[%d] SCMD[%d] PL[%d] PRIOR[%d]\n", m_iNetID, iMainCmd, iSubCmd, iLen, iPrior);
			}
		}
		else if(m_pTcpConnection != NULL)// && m_pTcpConnection->GetDelete() == false && m_pTcpConnection->GetActive())
		{
			if(m_pTcpConnection->AddSendData(iMainCmd, iSubCmd, (char*)pMsg, iLen, NULL, cSeq) < 0)
			{
				bSendOK = false;
				//g_Log.Log(LogType::_ERROR, this, L"TCP SendError ADBID[%d] CNAME[%s] MCMD[%d] SCMD[%d] PL[%d]\n", GetAccountDBID(), GetCharacterName(), iMainCmd, iSubCmd, iLen);
			}
		}
#if !defined( STRESS_TEST )
		else
		{
			bSendOK = false;
			//g_Log.Log(LogType::_ERROR, this, L"SendError ADBID[%d] CNAME[%s] MCMD[%d] SCMD[%d] PL[%d]\n", GetAccountDBID(), GetCharacterName(), iMainCmd, iSubCmd, iLen);
		}
#endif

		if((bSendUdp && iPrior == _RELIABLE) || bSendUdp == false)
		{
			if( !bSendOK )
			{
				m_cPacketSeq = cPrevPacketSeq;
			}
		}

	}
}

int CDNUserSession::AddSendData(int iMainCmd, int iSubCmd, char *pData, int iLen)
{
	SendPacket(iMainCmd, iSubCmd, pData, iLen, _RELIABLE);
	return 0;
}

void CDNUserSession::PreInitializeUser(WCHAR * wszAccountName, UINT nAccountDBID, UINT nSessionID, INT64 biCharacterDBID, int iTeam, int nWorldID, int nVillageID, bool bTutorial, bool bAdult, char cPCBangGrade, TMemberVoiceInfo * pInfo)
{
	SetAccountName(wszAccountName);
	m_nAccountDBID = nAccountDBID;
	m_nSessionID = nSessionID;
	m_biCharacterDBID = biCharacterDBID;
	m_iTeam	= iTeam;
	m_nWorldSetID = nWorldID;
	m_cVillageID = nVillageID;
	m_bAdult = bAdult;
	SetPCBangGrade(cPCBangGrade);

	if(m_bPCBang)
	{
		SendPCBang(m_cPCBangGrade, NULL);
	}
	m_bTutorial = bTutorial;

#ifdef _USE_VOICECHAT
	if(pInfo)
	{
		m_bVoiceAvailable = pInfo->cVoiceAvailable == 0 ? false : true;
		memcpy(m_nVoiceMutedList, pInfo->nMutedList, sizeof(UINT[PARTYCOUNTMAX]));
		m_nVoiceChannelID = 0;
		m_cIsTalking = 0;
		m_nVoiceJoinType = _VOICEJOINTYPE_NONE;
	}
#endif
	SetSessionState(SESSION_STATE_READY);
}

void CDNUserSession::QueryGetMasterSystemCountInfo( bool bClientSend, CDNDBConnection* pDBCon/*=NULL*/, BYTE cThreadID/*=0*/ )
{
	if( m_bIsAIBot || GetGameRoom() == NULL )
		return;

	// Some Tasks do not need this information.
	switch( GetGameRoom()->GetGameTaskType() )
	{
		case GameTaskType::Farm:
		case GameTaskType::PvP:
		{
			return;
		}
	}

	std::vector<INT64> vPartyListExceptMe;
	for( UINT i=0 ; i<GetGameRoom()->GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetGameRoom()->GetUserData(i);
		if( pSession && pSession != this && pSession->bIsGMTrace() == false )
			vPartyListExceptMe.push_back( pSession->GetCharacterDBID() );
	}

	if( pDBCon )
		pDBCon->QueryGetMasterSystemCountInfo( cThreadID, this, bClientSend, vPartyListExceptMe );
	else
		GetDBConnection()->QueryGetMasterSystemCountInfo( GetDBThreadID(), this, bClientSend, vPartyListExceptMe );
}

void CDNUserSession::InitialUser()
{	
	if( m_bIsAIBot ) return;
	//Query Character Data
	BYTE cThreadID = 0;
	CDNDBConnection *pTempDBCon = g_pDBConnectionManager->GetDBConnection(cThreadID);
	pTempDBCon->QueryGetMasterSystemSimpleInfo( cThreadID, this, false, MasterSystem::EventType::Init );
	QueryGetMasterSystemCountInfo( false, pTempDBCon, cThreadID );

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	pTempDBCon->QueryGetListNpcReputation( cThreadID, this );
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

#if defined( PRE_ADD_SECONDARY_SKILL )
	pTempDBCon->QueryGetListSecondarySkill( cThreadID, this );
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

	if( GetGameRoom()->bIsFarmRoom() )
	{
		pTempDBCon->QueryGetFieldCountByCharacter( cThreadID, GetWorldSetID(), m_nAccountDBID, m_biCharacterDBID, false );
		pTempDBCon->QueryGetListFieldForCharacter( cThreadID, GetWorldSetID(), GetAccountDBID(), GetCharacterDBID(), GetRoomID() );
	}
	pTempDBCon->QueryGetListPvPLadderScore( cThreadID, this );
	pTempDBCon->QueryGetPVPGhoulScores( cThreadID, this );

	pTempDBCon->QueryGetListEtcPoint( cThreadID, this );

	pTempDBCon->QueryGetAbuseMonitor( cThreadID, this );

#if defined(PRE_ADD_ABUSE_ACCOUNT_RESTRAINT)

	pTempDBCon->QueryGetWholeAbuseMonitor( cThreadID, this );

#endif		//#if defined(PRE_ADD_ABUSE_ACCOUNT_RESTRAINT)
	pTempDBCon->QueryGetListVariableReset( cThreadID, this );
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	pTempDBCon->QueryGetTotalSkillLevel(cThreadID, this);
#endif

#if defined( PRE_PARTY_DB )
	pTempDBCon->QuerySelectCharacter( cThreadID, this );
#else
	pTempDBCon->QuerySelectCharacter( cThreadID, this, 0 );
#endif

#if defined (PRE_ADD_BESTFRIEND)
	pTempDBCon->QueryGetBestFriend(cThreadID, this, false);
#endif

	//Query Friend Data
	pTempDBCon->QueryFriendList(cThreadID, this );

	//Query Isolate Data
	pTempDBCon->QueryGetIsolateList(cThreadID, this);

	//Query GameOption
	pTempDBCon->QueryGetGameOption(cThreadID, this);

	//Cash Inventory
	pTempDBCon->QueryGetPageMaterializedItem(cThreadID, this, 1, CASHINVENTORYPAGEMAX);

	pTempDBCon->QueryGetProfile(cThreadID, this);
#if defined( PRE_ADD_LIMITED_SHOP )
	pTempDBCon->QueryGetLimitedShopItem(cThreadID, this);
#endif

#if defined( PRE_ADD_STAMPSYSTEM )
	pTempDBCon->QueryGetListCompleteChallenges(cThreadID, this);
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
#if defined( PRE_ALTEIAWORLD_EXPLORE )
	pTempDBCon->QueryGetAlteiaWorldInfo(cThreadID, this);
#endif

#ifdef _DEBUG
	g_Log.Log(LogType::_NORMAL, this, L"[ADBID:%u CDBID:%I64d SID:%u] [InitialUser]\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
#endif
	
	SetSessionState(SESSION_STATE_LOAD);
}

void CDNUserSession::BackUpEquipInfo()
{
	if( m_BackupEquipInfo.first > 0 )
	{
		int iInvenIndex = GetItem()->FindInventorySlotBySerial( m_BackupEquipInfo.first );
		int iDestIndex	= GetItem()->GetInventory(iInvenIndex) ? GetItem()->GetEquipSlotIndex( GetItem()->GetInventory(iInvenIndex)->nItemID ) : -1;
		if( iInvenIndex >= 0 && iDestIndex >= 0 )
		{
			CSMoveItem TxPacket;
			memset( &TxPacket, 0, sizeof(TxPacket) );

			TxPacket.cMoveType			= MoveType_InvenToEquip;
			TxPacket.cSrcIndex			= static_cast<BYTE>(iInvenIndex);
			TxPacket.biSrcItemSerial	= m_BackupEquipInfo.first;
			TxPacket.cDestIndex			= static_cast<BYTE>(iDestIndex);
			TxPacket.wCount				= 1;

			GetItem()->SetMoveItemCheckGameMode( false );
			GetGameRoom()->OnDispatchMessage( this, CS_ITEM, eItem::CS_MOVEITEM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket), 0 );
			GetItem()->SetMoveItemCheckGameMode( true );
		}

		m_BackupEquipInfo = std::make_pair(0,0);
	}
}

void CDNUserSession::CheckAndSendStageClearLog(bool bClear)
{
	if( m_bIsAIBot )
		return;
	if( bClear )
	{
		if( !m_pGameRoom )
			return;

		// Stage clear log (when succeeded without calling RequestDungeonClear)
		if( m_pGameRoom->GetStageStartLogFlag() == true )
		{
			CDnPlayerActor *pPlayer = GetPlayerActor();
			if( !pPlayer ) 
				return;

			BYTE cThreadID;
			CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
			if( pDBCon )
			{

#if defined( PRE_ADD_36870 )
				pDBCon->QueryAddStageClearLog( cThreadID, m_pGameRoom->GetWorldSetID(), GetAccountDBID(), m_pGameRoom->GetRoomLogIndex(), GetCharacterDBID(), GetUserJob(), GetLevel(),
					true, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(0), pPlayer->GetMaxComboCount(), pPlayer->GetKillBossCount(), 0, 0, 0, 0, 
					m_pGameRoom->GetDungeonPlayTime()/1000, m_pGameRoom->GetDungeonClearRound());
#else
				pDBCon->QueryAddStageClearLog( cThreadID, m_pGameRoom->GetWorldSetID(), GetAccountDBID(), m_pGameRoom->GetRoomLogIndex(), GetCharacterDBID(), GetUserJob(), GetLevel(),
					true, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(0), pPlayer->GetMaxComboCount(), pPlayer->GetKillBossCount(), 0, 0, 0, 0, 0 );
#endif // #if defined( PRE_ADD_36870 )
			}
		}
	}
	else
	{
		// Stage clear log (on failure) && when StartLog exists
		if( GetLastStageClearRank() == -1 && m_pGameRoom->GetStageStartLogFlag() == true )
		{
			BYTE cThreadID;
			CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
			if( pDBCon )
			{
#if defined( PRE_ADD_36870 )
				pDBCon->QueryAddStageClearLog( cThreadID, m_pGameRoom->GetWorldSetID(), GetAccountDBID(), m_pGameRoom->GetRoomLogIndex(), GetCharacterDBID(), GetUserJob(), GetLevel(),
					false, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(0), 0, 0, 0, 0, 0, 0, 0, m_pGameRoom->GetDungeonClearRound());
#else
				pDBCon->QueryAddStageClearLog( cThreadID, m_pGameRoom->GetWorldSetID(), GetAccountDBID(), m_pGameRoom->GetRoomLogIndex(), GetCharacterDBID(), GetUserJob(), GetLevel(),
					false, static_cast<DBDNWorldDef::ClearGradeCode::eCode>(0), 0, 0, 0, 0, 0, 0, 0 );
#endif // #if defined( PRE_ADD_36870 )
			}
		}

	}
}

void CDNUserSession::FinalUser()
{
	if(m_iState == SESSION_STATE_DISCONNECTED) return;
	if(m_bIsAIBot) {
		m_iState = SESSION_STATE_DISCONNECTED;
		return;
	}

	FinalizeEvent();

#if defined( STRESS_TEST )
#else
	g_Log.Log( LogType::_GAMECONNECTLOG, this, L"[%d] FinalUser() SessionState=%d RoomState=%d ip=%s\n", g_Config.nManagedID, m_iState, GetGameRoom() ? GetGameRoom()->GetRoomState() : -1, GetIpW() ? GetIpW() : L"None" );
#endif // #if defined( STRESS_TEST )

	BackUpEquipInfo();

	// Send InstantItem.
	if( m_iState != SESSION_STATE_SERVER_CHANGE )
		GetItem()->RemoveInstantItemData( false );

	if(m_iState != SESSION_STATE_SERVER_CHANGE && m_iState != SESSION_STATE_RECONNECTLOGIN)
	{
		if(GetGameRoom())
		{
			GetGameRoom()->OnDelPartyMember(GetSessionID(), m_cKickKind);
#if defined(PRE_ADD_REVENGE)
			if( m_pGameRoom->bIsPvPRoom() && !GetPvPGameModeFinish() )
				static_cast<CDNPvPGameRoom*>(m_pGameRoom)->OnLeaveRoomBeforeFinish(GetSessionID());
#endif
		}
		else
			_DANGER_POINT();
	}
#if defined( PRE_PARTY_DB )
	else if( GetGameRoom() && GetGameRoom()->GetGameType() != REQINFO_TYPE_SINGLE )
	{		
		if( GetGameRoom()->GetTaskMng() )
		{
			CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetGameRoom()->GetTaskMng()->GetTask("PartyTask"));
			if( pPartyTask )
				pPartyTask->DelPartyMemberDB( this );
		}		
	}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
	if( GetGameRoom() && GetGameRoom()->bIsWorldPvPRoom() )
	{
		if( GetGameRoom()->GetWorldPvPRoomReqType() == WorldPvPMissionRoom::Common::GMRoom && GetAccountDBID() == GetGameRoom()->GetWorldPvPRoomCreateGMAccountDBID() )
		{
			CDNGameRoom::PartyStruct * pStruct = NULL;
			CDNUserSession * pSession = NULL;

			for (DWORD i = 0; i < GetGameRoom()->GetUserCount(); i++)
			{
				pStruct = GetGameRoom()->GetPartyData(i);
				pSession = pStruct->pSession;
				if ( pStruct && pSession && pSession != this )
				{
					pSession->SendPvPGameToPvPLobby();
				}
			}
		}

		GetGameRoom()->DelDBWorldPvPRoomMember( this );
		if( GetGameRoom()->GetPvPGameMode() )
		{
			if( GetGameRoom()->GetPvPGameMode()->bIsSendStartMsg() && !GetGameRoom()->GetPvPGameMode()->bIsWaitStartCount() )
			{
				if( GetSessionID() == GetGameRoom()->GetPvPGameMode()->GetSendStartMsgSessionID() )
				{
					GetGameRoom()->GetPvPGameMode()->SetSendStartMsg(false);
				}
			}
		}
	}
#endif
#if defined( PRE_ADD_DIRECTNBUFF )
	if( !m_DirectPartyBuffItemList.empty() )
	{
		for(std::list<int>::iterator itor=m_DirectPartyBuffItemList.begin();itor!=m_DirectPartyBuffItemList.end();itor++)
		{
			GetGameRoom()->RemoveDirectPartyBuff(*itor);
		}		
	}
#endif

	SendPartyBonusValue(0, 0);

	CheckAndSendStageClearLog();

	if(m_bStageAbortLog && m_pGameRoom->GetTaskMng() ) 
	{
		CDnGameTask* pTask = (CDnGameTask *)m_pGameRoom->GetTaskMng()->GetTask( "GameTask" );
		if(pTask && pTask->IsEnteredDungeon())
			ChangeStageUserData();
	}

	// Cancel trade if currently trading
	if(m_nExchangeTargetSessionID > 0){
		CDNUserSession *pSession = FindUserSession(m_nExchangeTargetSessionID);
		if(pSession){
			pSession->SendExchangeCancel();
			pSession->ClearExchangeInfo();
		}
		SendExchangeCancel();
		ClearExchangeInfo();
	}
	if(m_nExchangeSenderSID > 0){
		CDNUserSession *pSession = FindUserSession(m_nExchangeSenderSID);
		if(pSession){
			pSession->SendExchangeReject(m_nSessionID);
			pSession->ClearExchangeInfo();
		}
	}
	if(m_nExchangeReceiverSID > 0){
		CDNUserSession *pSession = FindUserSession(m_nExchangeReceiverSID);
		if(pSession){
			pSession->SendExchangeRequest(m_nSessionID, ERROR_EXCHANGE_SENDERCANCEL);	// Hide accept/decline button window
			pSession->ClearExchangeInfo();
		}
	}

	// Fatigue reduction check to prevent force-quit abuse
	DecreaseFatigue(true);

	if( m_hActor ) {
		if( m_iState != SESSION_STATE_SERVER_CHANGE && m_iState != SESSION_STATE_RECONNECTLOGIN ) {
			// Decrease durability
			if( m_hActor->IsPlayerActor() )
			{
				CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_hActor.GetPointer());
				if(pPlayer)
					pPlayer->OnStageGiveUp();
				else
					_DANGER_POINT();
			}
			else
			{
				_DANGER_POINT();
			}
		}
		if( m_iState == SESSION_STATE_GAME_PLAY || m_iState == SESSION_STATE_RECONNECTLOGIN ) {
			SaveUserData();
			LastUpdateUserData();
		}
	}
	else{
		if (m_iState > SESSION_STATE_LOADED && m_pTimeEventSystem)
			m_pTimeEventSystem->SaveUserData( true );
	}

	if(m_bNeedUpdateOption)
		GetDBConnection()->QuerySetGameOption(this, &m_GameOption);

	// Rotha : Unequip vehicle
	if( m_hActor )
	{
		if( m_hActor->IsPlayerActor() )
		{
			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_hActor.GetPointer());
			if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor())
				pPlayer->UnRideVehicle();
		}
	}

	// If chatting in a chat room, leave the room
	if( m_nChatRoomID > 0 )
	{
		CDNChatRoom * pChatRoom = m_pGameRoom->m_ChatRoomManager.GetChatRoom( m_nChatRoomID );
		if( pChatRoom != NULL )
		{
			if( pChatRoom->IsLeader( GetSessionID() ) )
			{
				// Room is deleted if the leader disconnects.
				while( pChatRoom->GetUserCount() > 0 )
				{
					UINT UserAID = pChatRoom->GetMemberAIDFromIndex( 0 );
					if( UserAID <= 0 )	break;	// Break out of loop to prevent infinite loop.

					CDNUserSession * pUserSession = NULL;
					if( UserAID == GetSessionID() )
						pUserSession = this;
					else 
						pUserSession = m_pGameRoom->GetUserSession( UserAID );
					if( pUserSession == NULL )	break;	// Break out of loop to prevent infinite loop.

					int nRet = pChatRoom->LeaveUser( pUserSession->GetSessionID(), CHATROOMLEAVE_DESTROYROOM );
					if( nRet == ERROR_NONE )
					{
						// Send a message to the character leaving the room and nearby characters.
						if( UserAID != GetSessionID() )	// Since self has already disconnected from the game, do not deliver the message.	
							pUserSession->SendChatRoomLeaveUser( pUserSession->GetSessionID(), CHATROOMLEAVE_DESTROYROOM );
						
						m_pGameRoom->BroadcastChatRoomLeaveUser( pUserSession, CHATROOMLEAVE_DESTROYROOM );
					}
				}
			}
			else
			{
				// Since self has already disconnected from the game, do not deliver the message.
				// Send message to nearby players (including room members)
				m_pGameRoom->BroadcastChatRoomLeaveUser( this, CHATROOMLEAVE_LEAVE );

				pChatRoom->LeaveUser( GetSessionID(), CHATROOMLEAVE_LEAVE );
			}

			// Delete the room if there are no players.
			if( pChatRoom->GetUserCount() <= 0 )
			{
				m_pGameRoom->m_ChatRoomManager.DestroyChatRoom( pChatRoom->GetChatRoomID() );
			}
		}
	}

	GetGameRoom()->AddBackupPartyInfo( this );

	// Originally should be deleted here
	SAFE_RELEASE_SPTR( m_hActor );

	if((m_iState != SESSION_STATE_SERVER_CHANGE) &&(m_iState != SESSION_STATE_RECONNECTLOGIN)) 
	{
		GetDBConnection()->QueryLogout(this);

		if(m_bCertified) {		// Caution !!! - Auth info reset must only be performed for users authenticated through QUERY_CHECKAUTH !!!
			// Process immediately instead of adding to list
			g_pAuthManager->QueryResetAuth(m_nWorldSetID, m_nAccountDBID, GetSessionID());
		}

		if( m_bCertified ) // Only tell Master to delete those who passed auth..
		// Must call SendDelUser() here.
		{
			g_pMasterConnectionManager->SendDelUser(GetWorldSetID(), GetAccountDBID(), m_bIntededDisconnected, m_nSessionID);
		}		
	}

	if(m_iState == SESSION_STATE_RECONNECTLOGIN)
		g_pMasterConnectionManager->SendCheckReconnectLogin(GetWorldSetID(), GetAccountDBID());

	if( m_iState != SESSION_STATE_SERVER_CHANGE )
	{
		if( GetDBConnection() ){
			GetDBConnection()->QueryLogoutCharacter( this );
		}
		if( g_pMasterConnectionManager )
			g_pMasterConnectionManager->SendMasterSystemSyncConnect( GetWorldSetID(), false, GetCharacterName(), GetMasterSystemData() );

		const TGuildUID GuildUID = GetGuildUID();
		if(GuildUID.IsSet())
		{
			if(g_pMasterConnectionManager)
			{
				g_pMasterConnectionManager->SendChangeGuildMemberInfo(GetWorldSetID(), GuildUID, GetAccountDBID(), GetCharacterDBID(), GetAccountDBID(), GetCharacterDBID(), GUILDMEMBUPDATE_TYPE_LOGINOUT, _LOCATION_NONE, 0, 0, NULL, true);
			}			
		}
#if defined( PRE_PRIVATECHAT_CHANNEL )
		if(GetPrivateChannelID())
		{
			CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( GetWorldSetID(), GetPrivateChannelID() );
			if(pPrivateChatChannel)
			{
				m_pDBCon->QueryOutPrivateChatChannelMember( m_pSession, PrivateChatChannel::Common::ExitMember );
				pPrivateChatChannel->DelPrivateChannelMember( PrivateChatChannel::Common::OutMember, GetCharacterDBID() );
				if( GetCharacterDBID() == pPrivateChatChannel->GetMasterCharacterDBID() )
				{					
					m_pDBCon->QueryModPrivateChatChannelInfo( m_pSession, pPrivateChatChannel->GetChannelName(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetNextMasterCharacterDBID());
					pPrivateChatChannel->ModPrivateChannelInfo(  PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetNextMasterCharacterDBID() );

					if(g_pMasterConnectionManager)
					{
						g_pMasterConnectionManager->SendModPrivateChatChannelInfo(m_pSession->GetWorldSetID(), pPrivateChatChannel->GetChannelID(), PrivateChatChannel::Common::ChangeMaster, pPrivateChatChannel->GetChannelPassword(), pPrivateChatChannel->GetMasterCharacterDBID() );
					}
				}		

				if(g_pMasterConnectionManager)
				{					
					g_pMasterConnectionManager->SendDelPrivateChatChannelMember( m_pSession->GetWorldSetID(), PrivateChatChannel::Common::OutMember, pPrivateChatChannel->GetChannelID(), GetCharacterDBID(), GetCharacterName() );
				}
				SetPrivateChannelID(0);
			}
		}
#endif
	}

	m_bCertified = false;

	if(m_pTcpConnection)
	{//Can occur if the session is deleted before connection is fully established
#if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
		DWORD dwTime = timeGetTime();
		bool bDelete = false;
		while( timeGetTime()-dwTime <= 1000 )
		{
			CSocketContext*		pSocketContext	= m_pTcpConnection->GetSocketContext();
			CDNTcpConnection*	pTcpCon			= m_pTcpConnection;

			if( InterlockedCompareExchange( &pSocketContext->m_lActiveCount, CSocketContext::DeleteFlag, 0 ) == 0 )
			{
				bDelete = true;
				pTcpCon->SetSocketContext(NULL, NULL);
				m_pTcpConnection = NULL;
				SAFE_DELETE(pTcpCon);
				g_pIocpManager->ClearSocketContext(pSocketContext);
				break;
			}
			else
			{
				g_Log.Log(LogType::_ERROR, L"[%d] GameServer Occur DanglingPointer!!!\r\n", g_Config.nManagedID );
			}
		}

		if( bDelete == false )
		{
			g_Log.Log(LogType::_ERROR, L"[%d] GameServer DanglingPointer Delete Fail!!!\r\n", g_Config.nManagedID );
		}
#else
		g_pIocpManager->ClearSocketContext(GetTcpConnection()->GetSocketContext());
		GetTcpConnection()->SetSocketContext(NULL, NULL);
		delete GetTcpConnection();
		m_pTcpConnection = NULL;
#endif // #if defined( PRE_FIX_SOCKETCONTEXT_DANGLINGPTR )
	}
	
	SetSessionState(SESSION_STATE_DISCONNECTED);

	m_pGameRoom->m_iPartMemberCnt--;
	if(GetGameRoom()->GetPartyIndex() > 0)
	{
		GetGameRoom()->SortMemberIndex(GetPartyMemberIndex());
		SetPartyMemberIndex(-1);
		GetGameRoom()->SendRefreshParty(0, NULL);
	}

	g_Log.Log(LogType::_NORMAL, this, L"[ADBID:%u CDBID:%I64d SID:%u] CDNUserConnection::FinalUser\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
}

void CDNUserSession::InitVoice()
{
#ifdef _USE_VOICECHAT
	if(GetGameRoom()->m_nVoiceChannelID[0] > 0)
		JoinVoiceChannel(GetGameRoom()->m_nVoiceChannelID[0], GetIp(), NULL, 0, false, true);
#endif
}

void CDNUserSession::SendSeqLevel(BYTE cSeqLevel)
{
	SendRoomSeqLevel(this, cSeqLevel);
}

void CDNUserSession::SendBackToVillage( bool bIsPartyOut, char cKickKind/*=-1*/, const int iMapIndex/*=-1*/ )
{
	m_cKickKind = cKickKind;
	if(m_cKickKind == 1)
	{
		for (int i = 0; i < PARTYKICKMAX; i++)
			if(GetGameRoom()->GetPartyKickedAccountID(i) == 0)
				GetGameRoom()->SetPartyKickedAccountID(i,GetAccountDBID());
	}

	int iLastVillageMapIndex = ( iMapIndex != -1 ) ? iMapIndex : m_UserData.Status.nLastVillageMapIndex;
	if(GetLastSubVillageMapIndex() > 0)
		iLastVillageMapIndex = GetLastSubVillageMapIndex();

	if(bIsGMTrace())
	{
		//If a GM followed in, just exit unconditionally
		g_pMasterConnectionManager->SendRebirthVillageInfo(m_nWorldSetID, m_nAccountDBID,  iLastVillageMapIndex );
		return;
	}
#if defined(PRE_WORLDCOMBINE_PARTY )
	else if(GetGameRoom()->GetGameType() == REQINFO_TYPE_PARTY && GetGameRoom()->GetUserCountWithoutGM() == 1 && bIsPartyOut == false && !GetGameRoom()->bIsWorldCombineParty())
#else
	else if(GetGameRoom()->GetGameType() == REQINFO_TYPE_PARTY && GetGameRoom()->GetUserCountWithoutGM() == 1 && bIsPartyOut == false)
#endif
	{
		//If solo and not giving up, keep the party.
		g_pMasterConnectionManager->SendRequestNextVillageInfo(m_nWorldSetID, iLastVillageMapIndex, -1, true, GetGameRoom());
#if defined(PRE_FIX_69108)
		//Process ranking update
		CDNGameRoom* pGameRoom = GetGameRoom();
		if( pGameRoom )
		{
			CDnGameTask *pTask = (CDnGameTask*)pGameRoom->GetTaskMng()->GetTask("GameTask");
			if( pTask && pTask->GetGameTaskType() == GameTaskType::DarkLair && CDnWorld::GetInstance(GetGameRoom()).GetMapType() == EWorldEnum::MapTypeDungeon)
			{
				GetGameRoom()->AddDungeonPlayTime( timeGetTime() - pTask->GetStageCreateTime() );
				for( DWORD i=0; i<pTask->GetUserCount(); i++ ) {
					CDNGameRoom::PartyStruct *pStruct = pTask->GetPartyData(i);
					if( pStruct == NULL ) continue;
					((CDNDLGameRoom*)GetGameRoom())->SetUpdateRankData( pStruct->pSession );
				}
				((CDNDLGameRoom*)GetGameRoom())->RequestRankQueryData();
			}
		}
#endif
		return;
	}

#if defined( PRE_ADD_58761 )
	if(GetGameRoom()->GetDungeonGateID() > 0)
	{
		if (GetGameRoom()->GetDBConnection())
			GetGameRoom()->GetDBConnection()->QueryAddNestGateClearLog( GetGameRoom()->GetDBThreadID(), GetGameRoom()->GetWorldSetID(), 0, GetGameRoom()->GetRoomLogIndex(), false, GetCharacterDBID(), GetUserJob(), GetLevel());
	}
#endif		//#if defined( PRE_ADD_58761 )

	if( GetGameRoom() )
		GetGameRoom()->DelLastPartyDungeonInfo( this );

	if (g_pMasterConnectionManager->SendRebirthVillageInfo(m_nWorldSetID, m_nAccountDBID,  iLastVillageMapIndex ))
		SetOutedMember();
}

void CDNUserSession::SendPvPGameToPvPLobby()
{
	if( g_pMasterConnectionManager )
	{
		bool bIsLadderRoom = false;
		if( GetGameRoom()  )
			bIsLadderRoom = GetGameRoom()->bIsLadderRoom();
		g_pMasterConnectionManager->SendPvPGameToPvPLobby( m_nWorldSetID, m_nAccountDBID, m_pGameRoom->GetPvPIndex(), GetLastVillageMapIndex(), bIsLadderRoom );
	}
	else
		_DANGER_POINT();
}

void CDNUserSession::SendConnectedResult()
{
	SendConnectedGameServer(this, m_nSessionID);
}

void CDNUserSession::SendUserInfo()
{
	CheckInitializeSchedule();

#if defined (PRE_ADD_BESTFRIEND)
	m_pBestFriend->CheckAndSendData();
#endif
	//Must send title on every move
	SendAppellationList(m_UserData.Appellation.Appellation);
	GetAppellation()->SendPeriodAppellationTime();
#if defined(PRE_ADD_REMOTE_QUEST)
	ModifyCompleteQuest();	//Needed for marking unselected daily quests
#endif
	SendGlyphExpireData(m_pItem->GetGlyphExpireDate());
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	SendTalismanExpireData(m_pItem->IsActiveCashTalisman(), m_pItem->GetCashTalismanExpireDate());
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

//#if defined( PRE_ADD_GUILDREWARDITEM )
//#else // #if defined( PRE_ADD_GUILDREWARDITEM )
	const TGuildUID GuildUID = GetGuildUID();

	if(GuildUID.IsSet()) 
	{		
		CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
		if (pGuild) 
		{	
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (pGuild->IsEnable()) 
			{
#endif
				TGuildRewardItem *RewardItemInfo;
				RewardItemInfo = pGuild->GetGuildRewardItem();
				SetGuildRewardItem(RewardItemInfo);
				SendGuildRewardItem(RewardItemInfo);
#if !defined( PRE_ADD_NODELETEGUILD )
			}			
#endif
		}		
	}
//#endif // #if defined( PRE_ADD_GUILDREWARDITEM )

//#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
//	for(int i=0;i<TotalLevelSkill::Common::MAXSLOTCOUNT;i++)
//	{
//		if(g_pDataManager->bIsTotalLevelSkillCashSlot(i))
//			m_pSession->SendTotalLevelSkillCashSlot(m_pSession->GetSessionID(), i, m_pSession->bIsTotalLevelSkillCashSlot(i), m_pSession->GetTotalLevelSkillCashSlot(i));
//	}
//#endif

	if(m_bTutorial){	// To tutorial
#ifdef _PACKET_COMP
		//if(m_pTcpConnection) m_pTcpConnection->SetPacketComp(true);
#endif

#ifdef _USE_VOICECHAT
		SendVoiceChatInfo(g_Config.szPublicDolbyIp, g_Config.nControlPort, g_Config.nAudioPort, m_nAccountDBID);
#endif
		SendEnter(this);
		SendEquipList( m_pItem );

		SendInvenList( m_pItem );
		SendWarehouseList(m_UserData.Status.nWarehouseCoin, m_pItem);
#if defined(PRE_ADD_SERVER_WAREHOUSE)
		SendServerWareHouseList(m_pItem);
		SendServerWareHouseCashList(m_pItem);
#endif
		SendSkill(m_UserData.Skill[DualSkill::Type::Primary].SkillList, DualSkill::Type::Primary);
		SendSkill(m_UserData.Skill[DualSkill::Type::Secondary].SkillList, DualSkill::Type::Secondary);
		SendSkillPageCount( GetItem()->GetSkillPageCount() );
		SendQuickSlotList(m_UserData.Status.QuickSlot);
		SendMissionList(m_UserData.Mission.nMissionScore, m_UserData.Mission.MissionGain, m_UserData.Mission.MissionAchieve, m_UserData.Mission.wLastMissionAchieve);

		SendPvPData( m_UserData.PvP );
		SendGlyphTimeInfo(m_UserData.Status.nGlyphDelayTime, m_UserData.Status.nGlyphRemainTime);
		SendDailyMissionList( 0, m_UserData.Mission.DailyMission );
		SendDailyMissionList( 1, m_UserData.Mission.WeeklyMission );
		SendDailyMissionList( 2, m_UserData.Mission.GuildWarMission );
		SendDailyMissionList( 4, m_UserData.Mission.GuildCommonMission );
		SendDailyMissionList( 5, m_UserData.Mission.WeekendEventMission );
		SendDailyMissionList( 6, m_UserData.Mission.WeekendRepeatMission );
#if defined(PRE_ADD_MONTHLY_MISSION)
		SendDailyMissionList( 7, m_UserData.Mission.MonthlyMission );
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
		SendKeySetting( &m_KeySetting );
		SendPadSetting( &m_PadSetting );
		SendNestClear(m_UserData.Status.NestClear);
#if defined( PRE_ADD_SECONDARY_SKILL )
		if( m_pSecondarySkillRepository )
			m_pSecondarySkillRepository->SendList();
#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

		if(m_pRestraint) m_pRestraint->SendRestraintList();

		if( GetDBConnection() )
			GetDBConnection()->QueryLoginCharacter( this );
		SendGuildWarFestivalPoint();
		SendPvPGhoulScores(&m_PvPTotalGhoulScores);

	}
#if defined(PRE_ADD_EQUIPLOCK)
	SendLockItems(m_pItem);
#endif	// #if defined(PRE_ADD_EQUIPLOCK)
}

bool CDNUserSession::LoadUserData(TASelectCharacter *pSelect)
{
	bool bResult = CDNUserBase::LoadUserData(pSelect);
	if( !bResult ) return false;

	if(g_pMasterConnectionManager)
		g_pMasterConnectionManager->SendLoadUserTempData(GetWorldSetID(), GetAccountDBID());

	return true;
}

bool CDNUserSession::SaveUserData()
{
	CDNUserBase::SaveUserData();
	if( GetPlayerActor() )
		GetPlayerActor()->SaveUserData(m_UserData);
	return true;
}

void CDNUserSession::ChangeServerUserData()
{
	if( m_bIsAIBot ) return;
	if(m_UserData.Status.cClass <= 0) return;	// Sometimes saved as 0, so handle it like this for now
	if(IsCertified() == false)
	{
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] CDNUserConnection::ChangeServerUserData IsCertified() == false!!\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
		return;
	}

	m_pTimeEventSystem->SaveUserData( false );
	SaveUserData();

	CDNDBConnection *pDBCon = GetDBConnection();
	if(!pDBCon){
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] CDNUserConnection::ChangeServerUserData DBConnection NULL!!\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
		return;
	}

	pDBCon->QueryUpdateUserData(QUERY_CHANGESERVERUSERDATA, this, &m_UserData, true );
	pDBCon->QuerySaveItemLocationIndex(this);
#if defined(PRE_ADD_DOORS_PROJECT)
	pDBCon->QuerySaveCharacterAbility(this);
#endif
}

void CDNUserSession::LastUpdateUserData()
{
	if( m_bIsAIBot ) return;
	if(m_UserData.Status.cClass <= 0) return;	// Sometimes saved as 0, so handle it like this for now
	if(!m_bLoadUserData) return;
	
	m_pTimeEventSystem->SaveUserData( true );

	CDNDBConnection *pDBCon = GetDBConnection();
	if(!pDBCon){
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u CDBID:%I64d SID:%u] CDNUserConnection::LastUpdateCharacter DBConnection NULL!!\r\n", m_nAccountDBID, m_biCharacterDBID, m_nSessionID);
		return;
	}

	pDBCon->QueryUpdateUserData(QUERY_LASTUPDATEUSERDATA, this, &m_UserData, false );
	pDBCon->QuerySaveItemLocationIndex(this);
#if defined(PRE_ADD_DOORS_PROJECT)
	pDBCon->QuerySaveCharacterAbility(this);
#endif
}

void CDNUserSession::ChangeStageUserData(int nAddPetExp/* = 0*/)
{
	if( m_bIsAIBot ) return;
	if(IsCertified() == false)
		return;

	if(m_UserData.Status.cClass <= 0) return;	// Sometimes saved as 0, so handle it like this for now

	SaveUserData();	// Save first

	std::vector<INT64> VecSerialList;
	std::vector<USHORT> VecDurList;
	VecSerialList.clear();
	VecDurList.clear();

	m_pItem->GetEquipItemDurability(VecSerialList, VecDurList);
	m_pItem->GetInventoryItemDurability(VecSerialList, VecDurList);
	
	int nDeathCount = 0;
	BYTE cDifficult = 0;
	CDnPlayerActor *pPlayer = GetPlayerActor();
	if( pPlayer )			
	{
		nDeathCount = pPlayer->GetStageDeathCount();
		pPlayer->ClearStageDeathCount(); // Save and reset
	}
	CDNGameRoom *pGameRoom = GetGameRoom();
	if( pGameRoom )
		cDifficult = pGameRoom->m_StageDifficulty+1;

	// Reason for using PCBangRebirthCoin instead of GetPCBangRebirthCoin() -> to send a fixed value regardless of whether it's a PCBang (calling the function returns 0 when not a PCBang)
	m_pDBCon->QueryChangeStageUserData(this, m_UserData.Status.cPCBangRebirthCoin, VecSerialList, VecDurList, nDeathCount, cDifficult, GetPickUpCoin(), m_pItem->GetPetBodySerial(), nAddPetExp);


	// Reset
	m_biPickUpCoin = 0;
}

void CDNUserSession::SetNextVillageData(const char * pIP, USHORT nPort, int nMapIndex, int nNextMapIndex, char cNextGateNo)
{
	GetGameRoom()->DelLastPartyDungeonInfo( this );

	if(pIP != NULL && nPort > 0)
	{
		_strcpy( m_szNextVillageIP, _countof(m_szNextVillageIP), pIP, (int)strlen(pIP) );
		m_nNextVillagePort = nPort;

		m_biCertifyingKey = g_pAuthManager->GetCertifyingKey();
		DN_ASSERT(0 != m_biCertifyingKey,	"Invalid!");	// Auth key must not be generated as 0! (0 means none)

		SetSessionState(SESSION_STATE_READY_TO_VILLAGE);

		g_pAuthManager->QueryStoreAuth(SERVERTYPE_GAME, this);
	}

	if(cNextGateNo > 0){		// Save only when properly standing in front of a gate (if Random, gateidx is -1, so do not save)
		if(g_pDataManager->GetMapType( nNextMapIndex ) == GlobalEnum::eMapTypeEnum::MAP_VILLAGE){
			SetMapIndex(nMapIndex);

			if(nNextMapIndex > 0){
				bool bUpdate = true;
				switch( g_pDataManager->GetMapSubType( nNextMapIndex ) )
				{
					case GlobalEnum::eMapSubTypeEnum::MAPSUB_PVPVILLAGE:
					case GlobalEnum::eMapSubTypeEnum::MAPSUB_PVPLOBBY:
					{
						bUpdate = false;
						break;
					}
				}
				if( bUpdate )
					SetLastMapIndex(nNextMapIndex);
			}
		}
		else
			SetMapIndex(nNextMapIndex);

		m_UserData.Status.cLastVillageGateNo = cNextGateNo;
	}

	ChangeStageUserData();

	m_bCharOutLog = false;
}

void CDNUserSession::SetMapIndex(int nMapIndex)
{
	if( m_bIsAIBot ) {
		CDNUserBase::SetMapIndex(nMapIndex);
		return;
	}
	if(m_bTutorial) return;	// Tutorial must not be saved

	CDNUserBase::SetMapIndex(nMapIndex);

	GetDBConnection()->QueryMapInfo(this);	// Save to DB 100128
}

void CDNUserSession::SetFatigue(int nFatigue, int nWeeklyFatigue, int nPCBangFatigue, int nEventFatigue, int nVIPFatigue, bool bDBSave/* = true*/)
{
	CDNUserBase::SetFatigue(nFatigue, nWeeklyFatigue, nPCBangFatigue, nEventFatigue, nVIPFatigue, bDBSave);

	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		pStruct->pSession->SendFatigue(GetSessionID(), nFatigue, nWeeklyFatigue, nPCBangFatigue, nEventFatigue, nVIPFatigue);
	}
}

void CDNUserSession::DecreaseFatigue(int nGap)
{
#if defined(PRE_ADD_WORLD_EVENT) // This will be removed.
#else
	// Deduct PvP fatigue consumption regardless of events.
	if( !GetGameRoom()->bIsPvPRoom() ) 
	{
		TEvent * pEvent = GetGameRoom()->GetApplyEvent(_EVENT_1_FATIGUE);
		if(pEvent) return;	// Exit without deducting if it is an event
	}
#endif //#if defined(PRE_ADD_WORLD_EVENT)

	// No fatigue reduction when GM drops in!
	if( bIsGMTrace() )
		return;
	// No fatigue reduction for observers!
	if( m_hActor && m_hActor->bIsObserver() )
		return;

	CDNUserBase::DecreaseFatigue(nGap);

	int nMax = g_pDataManager->GetFatigue(GetUserJob(), m_UserData.Status.cLevel);
	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		pStruct->pSession->SendFatigue(GetSessionID(), GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
	}
}

void CDNUserSession::IncreaseFatigue(int nGap)
{
	if(nGap <= 0) return;

	CDNUserBase::IncreaseFatigue(nGap);

	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		pStruct->pSession->SendFatigue(GetSessionID(), GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
	}
}

void CDNUserSession::SetDefaultMaxFatigue(bool bSend)
{
	CDNUserBase::SetDefaultMaxFatigue(bSend);

	if( bSend ) {
		for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
			CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
			if(pStruct == NULL) continue;
			if(pStruct->pSession != this)
				pStruct->pSession->SendFatigue(GetSessionID(), GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
		}
	}
}

void CDNUserSession::SetDefaultMaxWeeklyFatigue(bool bSend)
{
	CDNUserBase::SetDefaultMaxWeeklyFatigue(bSend);

	if( bSend ) {
		for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
			CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
			if(pStruct == NULL) continue;
			if(pStruct->pSession != this)
				pStruct->pSession->SendFatigue(GetSessionID(), GetFatigue(), GetWeeklyFatigue(), GetPCBangFatigue(), GetEventFatigue(), GetVIPFatigue());
		}
	}
}

void CDNUserSession::SetDefaultMaxRebirthCoin(bool bSend)
{
	CDNUserBase::SetDefaultMaxRebirthCoin(bSend);

	if(bSend)
	{
		if( GetPlayerActor() && GetPlayerActor()->GetPartyData() )
			SendRebirthCoin(ERROR_NONE, GetPlayerActor()->GetPartyData()->nUsableRebirthCoin, _REBIRTH_SELF, GetSessionID());
	}
}

CDnPlayerActor* CDNUserSession::GetPlayerActor()
{
	if( !m_hActor )
		return NULL;

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_hActor.GetPointer());
	return pPlayer;
}

void CDNUserSession::AddPacketQueue( const DWORD dwUniqueID, const BYTE cSubCmd, const BYTE *pBuffer, const int nSize, const int nPrior )
{
	if( m_bIsAIBot )
		return;
	if(( sizeof(DWORD) + sizeof(BYTE) + sizeof(WORD) + nSize ) >= PACKET_QUEUE_SIZE ) {
		_ASSERT(0);
		return;
	}
	if( m_nPacketQueueOffset[nPrior] +( sizeof(DWORD) + sizeof(BYTE) + sizeof(WORD) + nSize ) >= PACKET_QUEUE_SIZE ) {
		FlushPacketQueue( nPrior );
	}
	CheckFlushQueue( nPrior, dwUniqueID, cSubCmd );

	memcpy( m_pPacketQueueBuffer[nPrior] + m_nPacketQueueOffset[nPrior], &dwUniqueID, sizeof(DWORD) ); m_nPacketQueueOffset[nPrior] += sizeof(DWORD);
	memcpy( m_pPacketQueueBuffer[nPrior] + m_nPacketQueueOffset[nPrior], &cSubCmd, sizeof(BYTE) ); m_nPacketQueueOffset[nPrior] += sizeof(BYTE);
	memcpy( m_pPacketQueueBuffer[nPrior] + m_nPacketQueueOffset[nPrior], &nSize, sizeof(WORD) ); m_nPacketQueueOffset[nPrior] += sizeof(WORD);
	memcpy( m_pPacketQueueBuffer[nPrior] + m_nPacketQueueOffset[nPrior], pBuffer, nSize ); m_nPacketQueueOffset[nPrior] += nSize;

#ifdef PRE_ADD_PACKETSIZE_CHECKER
	if (cSubCmd < (BYTE)m_vActorSendInfo.size())
	{
		m_vActorSendInfo[cSubCmd].nSendCount++;
		m_vActorSendInfo[cSubCmd].nSendSize += nSize;
	}
	/*int cnt = m_nSendActorDebugInfoCount++&31;
	m_SendActorDebugInfo[cnt]._dwUniqueID = dwUniqueID;
	m_SendActorDebugInfo[cnt]._ActorSubCmd = cSubCmd;
	m_SendActorDebugInfo[cnt]._PacketBroadFlag = bBroad ? 1 : 0;*/
#endif		//#ifdef PRE_ADD_PACKETSIZE_CHECKER

	m_nPacketQueueCount[nPrior]++;
	m_dwVecPacketQueueUniqueList[nPrior].push_back( dwUniqueID );
}

void CDNUserSession::CheckFlushQueue( int nPrior, DWORD dwUniqueID, BYTE cSubCmd )
{
	bool bCheckFlush = true;
	switch( cSubCmd ) {
		case eActor::SC_CP: 
			bCheckFlush = false;
			break;
		default: break;
	}
	for( int i=0; i<3; i++ ) {
		if( i == nPrior ) continue;
		if( std::find( m_dwVecPacketQueueUniqueList[i].begin(), m_dwVecPacketQueueUniqueList[i].end(), dwUniqueID ) != m_dwVecPacketQueueUniqueList[i].end() ) {
			FlushPacketQueue( i );
		}
	}
}

void CDNUserSession::FlushPacketQueue( int nPrior )
{
	if( m_bIsAIBot )
		return;

	if( nPrior == -1 ) {
		for( int i=0; i<3; i++ ) FlushPacketQueue(i);
		return;
	}

	if( m_nPacketQueueCount[nPrior] == 0 ) return;
	SendGameActorBundleMsg( this, m_nPacketQueueCount[nPrior], m_pPacketQueueBuffer[nPrior], m_nPacketQueueOffset[nPrior], nPrior );

	m_nPacketQueueOffset[nPrior] = 0;
	m_nPacketQueueCount[nPrior] = 0;
	m_dwVecPacketQueueUniqueList[nPrior].clear();
}

void CDNUserSession::FlushStoredPacket()
{
	DNVector(_STORE_PACKET) StoredPackets;
	{
		ScopeLock<CSyncLock> Lock( m_StoreSync );
		StoredPackets.swap( m_pStoredPacketList );
	}

	DNVector(_STORE_PACKET)::iterator ii;
	for(ii = StoredPackets.begin(); ii != StoredPackets.end(); ii++)
	{
		SendPacket((*ii).iMainCmd, (*ii).iSubCmd, (*ii).pMsg, (*ii).iSize, _RELIABLE);
		if( (*ii).pMsg )
			CLfhHeap::GetInstance()->_DeAlloc((*ii).pMsg);
	}
}

void CDNUserSession::StorePacket(int iMainCmd, int iSubCmd, const char * pData, int nLen)
{
	ScopeLock<CSyncLock> Lock( m_StoreSync );

	_STORE_PACKET Store;
	Store.iMainCmd = iMainCmd;
	Store.iSubCmd = iSubCmd;
	Store.iSize = nLen;
	if( nLen > 0 && pData )
	{
		Store.pMsg = static_cast<char*>(CLfhHeap::GetInstance()->_Alloc(nLen));
		memcpy(Store.pMsg, pData, nLen);
	}
	else
	{
		Store.pMsg = NULL;
	}

	m_pStoredPacketList.push_back(Store);
}

CDNUserSession* CDNUserSession::FindUserSession(UINT nSessionID)
{
	return m_pGameRoom->GetUserSession(nSessionID);
}

#if !defined(PRE_DELETE_DUNGEONCLEAR)
bool CDNUserSession::IsNeedDungeonLevelSave( int nMapTableID )
{
	DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat *pEnterDungeonSox = GetDNTable( CDnTableDB::TDUNGEONENTER );

	if( !pMapSox->IsExistItem( nMapTableID ) ) return false;
	int nDungeonEnterTableID = pMapSox->GetFieldFromLablePtr( nMapTableID, "_EnterConditionTableID" )->GetInteger();
	if( nDungeonEnterTableID < 1 || !pEnterDungeonSox->IsExistItem( nDungeonEnterTableID ) ) return false;

	int nLevel = pEnterDungeonSox->GetFieldFromLablePtr( nDungeonEnterTableID, "_LvlMin" )->GetInteger();
	if( nLevel < 1 || GetLevel() - nLevel >= 30 ) return false;

	return true;
}

bool CDNUserSession::CheckDungeonEnterLevel( int nMapTableID )
{
	if( !IsNeedDungeonLevelSave( nMapTableID ) ) return false;
	return CDNUserBase::CheckDungeonEnterLevel(nMapTableID);
}

void CDNUserSession::CalcDungeonEnterLevel(int nMapTableID, char &cOpenHard, char &cOpenVeryHard)
{
	CDNUserBase::CalcDungeonEnterLevel( nMapTableID, cOpenHard, cOpenVeryHard );
	if( GetMaxLevelCharacterCount() > 0 ) {
		cOpenHard = cOpenVeryHard = true;
	}
}

bool CDNUserSession::UpdateDungenEnterLevel( int nCurrentStageConstructionLevel, int nRank )
{
	CDnGameTask *pTask = (CDnGameTask *)m_pGameRoom->GetTaskMng()->GetTask( "GameTask" );
	if( !pTask ) return false;

	int nEnterMapTableID = pTask->GetEnterMapTableID();
	if( nEnterMapTableID < 1 ) return false;
	int nDungeonEnterTableID = pTask->GetDungeonEnterTableID();
	if( nDungeonEnterTableID < 1 ) return false;
	if( !IsNeedDungeonLevelSave( nEnterMapTableID ) ) return false;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	static char *szRankStr[] = { "_Hard", "_VeryHard" };
	char szLabel[32];

	sprintf_s( szLabel, "%sOpenRank", szRankStr[nCurrentStageConstructionLevel-1] );
	int nNeedRank = pSox->GetFieldFromLablePtr( nDungeonEnterTableID, szLabel )->GetInteger();
	if( nRank > nNeedRank ) return false;

	int nSlotIndex = GetDungeonLevelIndex( nEnterMapTableID, NULL );
	if( nSlotIndex == -1 ) return false;

	if( nCurrentStageConstructionLevel - 1 != GetDungeonClearType(nSlotIndex) ) return false;

	SetDungeonClearType(nSlotIndex, nCurrentStageConstructionLevel);	// Includes DB save

	SendGameOpenDungeonLevel( this, nEnterMapTableID, nCurrentStageConstructionLevel + 1 );

	return true;
}

void CDNUserSession::RefreshDungeonEnterLevel()
{
	for( int i=0; i<DUNGEONCLEARMAX; i++ ) {
		int nMapIndex = GetDungeonClearMapIndex(i);
		if( nMapIndex < 1 ) continue;

		if( !IsNeedDungeonLevelSave( nMapIndex ) ) {
			SetDungeonClear(i, 0, 0);	// Save to DB?
		}
	}
}
#endif	// #if !defined(PRE_DELETE_DUNGEONCLEAR)

void CDNUserSession::CopyDefaultParts( int* pDest )
{
	*pDest		= GetDefaultBody();
	*(pDest+1)	= GetDefaultLeg();
	*(pDest+2)	= GetDefaultHand();
	*(pDest+3)	= GetDefaultFoot();
}

void CDNUserSession::SendPvPModeStartTick( const UINT uiStartTick )
{
	if( m_iState != SESSION_STATE_GAME_PLAY )
	{
		_DANGER_POINT();
		return;
	}

	if( m_hActor )
		m_hActor->ResetActor();

	SCPVP_MODE_STARTTICK TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiStartTick	= uiStartTick;
	TxPacket.uiCurTick		= timeGetTime();

	AddSendData( SC_PVP, ePvP::SC_MODE_STARTTICK, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSession::SendPvPModeFinish( const CPvPGameMode* pPvPMode )
{
	SCPVP_FINISH_PVPMODE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiWinTeam	= pPvPMode->GetWinTeam();
	TxPacket.Reason		= pPvPMode->GetFinishReason();
#if defined(PRE_ADD_PVP_TOURNAMENT)
	TxPacket.uiWinSessionID = pPvPMode->GetLastWinSessionID();
#endif
	pPvPMode->GetGameModeScore( TxPacket.uiATeamScore, TxPacket.uiBTeamScore );

	AddSendData( SC_PVP, ePvP::SC_FINISH_PVPMODE, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSession::SendPvPRoundStart()
{
	AddSendData( SC_PVP, ePvP::SC_START_PVPROUND, NULL, 0 );
}

#if defined(PRE_ADD_PVP_TOURNAMENT)
void CDNUserSession::SendPvPRoundFinish( const UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason, const CPvPGameMode* pPvPMode, const UINT uiWInSessionID, const char cTournamentStep )
#else
void CDNUserSession::SendPvPRoundFinish( const UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason, const CPvPGameMode* pPvPMode )
#endif
{
	SCPVP_FINISH_PVPROUND TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiWinTeam	= uiWinTeam;
	TxPacket.Reason		= Reason;
#if defined(PRE_ADD_PVP_TOURNAMENT)
	TxPacket.uiWinSessionID = uiWInSessionID;
	TxPacket.cTournamentStep = cTournamentStep;
#endif  //#if defined(PRE_ADD_PVP_TOURNAMENT)
	pPvPMode->GetGameModeScore( TxPacket.uiATeamScore, TxPacket.uiBTeamScore );

	AddSendData( SC_PVP, ePvP::SC_FINISH_PVPROUND, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSession::SendSuccessBreakInto( const UINT uiSessionID )
{
	SCPVP_SUCCESSBREAKINTO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID = uiSessionID;

	AddSendData( SC_PVP, ePvP::SC_BREAKINTO_SUCCESS, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}
void CDNUserSession::SendPvPAddPoint( UINT uiSessionID, const UINT uiScoreType, const UINT uiAddPoint )
{
	SCPVP_ADDPOINT TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID	= uiSessionID;
	TxPacket.uiScoreType	= uiScoreType;
	TxPacket.uiAddPoint		= uiAddPoint;

	AddSendData( SC_PVP, ePvP::SC_ADDPOINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserSession::SendPvPSelectCaptain( UINT uiSessionID, WCHAR* wszName )
{
	SCPVP_SELECTCAPTAIN TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiSessionID = uiSessionID;

	AddSendData( SC_PVP, ePvP::SC_SELECTCAPTAIN, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

#if !defined( _FINAL_BUILD )
	WCHAR wszChat[MAX_PATH];
	wsprintf( wszChat, L"%s is the Captain.", wszName );
	SendChat( CHATTYPE_TEAMCHAT, static_cast<int>(wcslen(wszChat)), L"", wszChat );
#endif // #if defined( _FINAL_BUILD )
}

void CDNUserSession::SendPvPSelectZombie( std::map<DWORD,DnActorHandle>& mZombie )
{
	SCPVP_SELECTZOMBIE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	if( _countof(TxPacket.Zombies) < mZombie.size() )
		return;

	for( std::map<DWORD,DnActorHandle>::iterator itor=mZombie.begin() ; itor!=mZombie.end() ; ++itor )
	{
		TxPacket.Zombies[TxPacket.cCount].uiSessionID				= (*itor).second->GetSessionID();
		TxPacket.Zombies[TxPacket.cCount].nMonsterMutationTableID	= (*itor).second->GetMonsterMutationTableID();
		TxPacket.Zombies[TxPacket.cCount].bZombie					= true;
		TxPacket.Zombies[TxPacket.cCount].iScale					= static_cast<int>((*itor).second->GetScale()->x*100);
		++TxPacket.cCount;

#if !defined( _FINAL_BUILD )
		WCHAR wszChat[MAX_PATH];
		wsprintf( wszChat, L"%s is a Zombie.", (*itor).second->GetName() );
		SendChat( CHATTYPE_TEAMCHAT, static_cast<int>(wcslen(wszChat)), L"", wszChat );
#endif // #if defined( _FINAL_BUILD )
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Zombies)+(TxPacket.cCount*sizeof(TZombieInfo));
	AddSendData( SC_PVP, ePvP::SC_SELECTZOMBIE, reinterpret_cast<char*>(&TxPacket), iSize );
}

void CDNUserSession::SendPvPSelectZombie( DnActorHandle hActor, int nTableID, bool bZombie, bool bRemoveStateBlow, int iScale )
{
	SCPVP_SELECTZOMBIE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.Zombies[TxPacket.cCount].uiSessionID				= hActor->GetSessionID();
	TxPacket.Zombies[TxPacket.cCount].nMonsterMutationTableID	= nTableID;
	TxPacket.Zombies[TxPacket.cCount].bZombie					= bZombie;
	TxPacket.Zombies[TxPacket.cCount].bRemoveStateBlow			= bRemoveStateBlow;
	TxPacket.Zombies[TxPacket.cCount].iScale					= iScale;
	++TxPacket.cCount;

#if !defined( _FINAL_BUILD )
	if( bZombie )
	{
		WCHAR wszChat[MAX_PATH];
		wsprintf( wszChat, L"%s is a Zombie.", hActor->GetName() );
		SendChat( CHATTYPE_TEAMCHAT, static_cast<int>(wcslen(wszChat)), L"", wszChat );
	}
#endif // #if defined( _FINAL_BUILD )

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.Zombies)+(TxPacket.cCount*sizeof(TZombieInfo));
	AddSendData( SC_PVP, ePvP::SC_SELECTZOMBIE, reinterpret_cast<char*>(&TxPacket), iSize );
}

#if defined(PRE_MOD_PVP_LADDER_XP)
void CDNUserSession::UpdatePvPLevel()
{
	TPvPGroup*	pPvPData = GetPvPData();
	if( !pPvPData ) return;

	// PvP_Level
	for( UINT i=pPvPData->cLevel ; i<PvPCommon::Common::MaxRank ; ++i )
	{
		const TPvPRankTable* pPvPRankTable = g_pDataManager->GetPvPRankTable( i );
		if( !pPvPRankTable )
		{
			_DANGER_POINT();
			break;
		}

#ifdef PRE_MOD_PVPRANK
		if (pPvPRankTable->cType != PvPCommon::RankTable::ExpValue)
			continue;

		if( pPvPData->uiXP >= pPvPRankTable->uiXP )
#if defined(PRE_ADD_PVPLEVEL_MISSION)
			SetPvPLevel(static_cast<BYTE>(i+1));
#else
			pPvPData->cLevel = static_cast<BYTE>(i+1);
#endif
		else
			break;
#else		//#ifdef PRE_MOD_PVPRANK
		if( pPvPData->uiXP >= pPvPRankTable->uiXP )
#if defined(PRE_ADD_PVPLEVEL_MISSION)
			SetPvPLevel(static_cast<BYTE>(i+1));
#else
			pPvPData->cLevel = static_cast<BYTE>(i+1);
#endif
		else
			break;
#endif		//#ifdef PRE_MOD_PVPRANK
	}
}
#endif

void CDNUserSession::SetHide(bool bHide)
{
	m_bHide = bHide;

	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		pStruct->pSession->SendHide(m_nSessionID, bHide);
	}
}

// Guild
void CDNUserSession::RefreshGuildSelfView()
{
	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++) {
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(NULL == pStruct) {
			continue;
		}
		pStruct->pSession->SendGuildSelfView(GetSessionID(), m_GuildSelfView);
	}
}

int CDNUserSession::CmdAbandonStage( const bool bDecreaseDurability, bool bPartyOut, bool bIntenedDisconnect, const int iMapIndex/*=-1*/ )
{
	if(m_iState != SESSION_STATE_GAME_PLAY || GetGameRoom()->GetRoomState() != _GAME_STATE_PLAY)	 
		return ERROR_GENERIC_INVALIDREQUEST;

	if( m_pGameRoom && m_pGameRoom->bIsPvPRoom() )
	{
		_DANGER_POINT();
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	CDNGameRoom::PartyStruct * pStruct = GetGameRoom()->GetPartyData(this);
	if(pStruct)
	{
		if(pStruct->nEnteredGateIndex != -1)
		{
			_DANGER_POINT();
			return ERROR_GENERIC_INVALIDREQUEST;
		}
	}

	DnActorHandle hActor = GetActorHandle();
	if( hActor )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
		if(pPlayer)
		{
			if( bDecreaseDurability )
			{
				//Do it in dungeon
				if(m_pGameRoom->GetWorld()->GetMapType() == EWorldEnum::MapTypeEnum::MapTypeDungeon)
				{
					// Must apply stage abandon penalty: reduce durability
					pPlayer->OnStageGiveUp();
					pPlayer->CmdRemoveStateEffect(STATE_BLOW::BLOW_099);
				}
			}
		}
	}

	if( bIntenedDisconnect == false )
		SendBackToVillage( bPartyOut, -1, iMapIndex );
	m_bStageAbortLog = false;

	return ERROR_NONE;
}

void CDNUserSession::SetUserJob(BYTE cJob)
{
	
	CDNUserBase::SetUserJob( cJob );

	DnActorHandle hActor = GetActorHandle();
	if( hActor )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
		pPlayer->CmdChangeJob( cJob );
	}

	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		if( pStruct->pSession == this ) continue;

		pStruct->pSession->SendChangeJob( m_nSessionID, cJob );
	}
}

void CDNUserSession::BroadcastingEffect(char cType, char cState)
{
	for(DWORD i = 0; i < m_pGameRoom->GetUserCount(); i++){
		CDNGameRoom::PartyStruct *pStruct = m_pGameRoom->GetPartyData(i);
		if(pStruct == NULL) continue;
		if( pStruct->pSession->GetState() != SESSION_STATE_GAME_PLAY ) 
			continue;
		pStruct->pSession->SendBroadcastingEffect(GetSessionID(), cType, cState);
	}
}

void CDNUserSession::VerifyValidMap(int nNextMapIndex)
{
#if defined(_FINAL_BUILD)
	// Check normal accounts only
	if((GetAccountLevel() <= AccountLevel_QA) &&(GetAccountLevel() >= AccountLevel_New))
		return;
#endif // #if defined(_FINAL_BUILD)

	if(!g_pDataManager->CheckChangeMap(GetMapIndex(), nNextMapIndex))
	{
		WCHAR wszBuf[100];
		wsprintf( wszBuf, L"GAME Prev[%d]->Next[%d]", GetMapIndex(), nNextMapIndex);
		GetDBConnection()->QueryAddAbuseLog(this, ABUSE_MOVE_SERVERS, wszBuf);
	}
}


void CDNUserSession::RecvUdpPing( DWORD dwTick )
{
	DWORD dwCur = timeGetTime();
	if( dwCur < dwTick )
	{
		_DANGER_POINT();
		return;
	}

	DWORD dwGap = (dwCur-dwTick);
	m_pairUdpPing.first += dwGap;
	++m_pairUdpPing.second;
}

void CDNUserSession::WritePingLog()
{
	int iTcpPing = -1;
	int iUdpPing = -1;
	if( m_pairTcpPing.second > 0 )
		iTcpPing = m_pairTcpPing.first/m_pairTcpPing.second;
	if( m_pairUdpPing.second > 0 )
		iUdpPing = m_pairUdpPing.first/m_pairUdpPing.second;
		
	if( iTcpPing == -1 && iUdpPing == -1 )
		return;

	g_Log.Log( LogType::_PING, this, L"[%d] IP=%S TcpPing=%dms(%d/%d) UdpPing=%dms(%d/%d)\r\n", g_Config.nManagedID, GetIp(), iTcpPing, m_pairTcpPing.first, m_pairTcpPing.second, iUdpPing, m_pairUdpPing.first, m_pairUdpPing.second );
}

void CDNUserSession::ApplySourceEffect(CDnPlayerActor* pPlayerActor, const TSourceData& source, bool bOnInitialize/* = false*/)
{
	if (!pPlayerActor)
		return;

	if (source.nItemID <= 0 || source.nRemainTime <= 0)
		return;

	const TItemData* pItemData = g_pDataManager->GetItemData(source.nItemID);
	if (!pItemData)
		return;

	const TSkillData* pSkill = g_pDataManager->GetSkillData(pItemData->nSkillID);
	if (!pSkill || pSkill->vLevelDataList.empty())
		return;

	CDnSkill::SkillInfo skillInfo;
	std::vector<CDnSkill::StateEffectStruct> stateEffects;
	CDnSkill::CreateSkillInfo(pSkill->nSkillID, pSkill->vLevelDataList[0].cSkillLevel, skillInfo, stateEffects);

	skillInfo.hSkillUser = pPlayerActor->GetActorHandle();

	// If a state effect from a source item already exists, remove the existing one and add the new one.
	int iSE = pPlayerActor->GetNumAppliedStateBlow();
	for( int i = 0; i < iSE; ++i )
	{
		DnBlowHandle hBlow = pPlayerActor->GetAppliedStateBlow( i );
		if( hBlow->IsFromSourceItem() )
			pPlayerActor->AddStateBlowIDToRemove( hBlow->GetBlowID() );
	}

	// Delete right now on the server as well what was requested to be deleted.
	pPlayerActor->RemoveResetStateBlow();

	for (CDnSkill::StateEffectStruct stateEffect : stateEffects)
	{
		int iBlowID = pPlayerActor->CmdAddStateEffect(&skillInfo, static_cast<STATE_BLOW::emBLOW_INDEX>(stateEffect.nID), (source.nRemainTime*1000), stateEffect.szValue.c_str(), false, true);
		pPlayerActor->SendRemoveStateEffectGraphic(static_cast<STATE_BLOW::emBLOW_INDEX>(stateEffect.nID));
		if( -1 != iBlowID )
		{
			DnBlowHandle hBlow = pPlayerActor->GetStateBlowFromID( iBlowID );
			hBlow->FromSourceItem();

			// #45646 Fill HP/MP to full only on initial entry to game server after using an Origin item.
			if( bOnInitialize )
				hBlow->SetHPMPFullWhenBegin();
		}

		// Nothing applies to mounts except movement speed change state effects. Add later if requested by design. 
		//if (pPlayerActor->GetMyVehicleActor())
		//{
		//	int iBlowID = pPlayerActor->GetMyVehicleActor()->CmdAddStateEffect(&skillInfo, static_cast<STATE_BLOW::emBLOW_INDEX>(stateEffect.nID), source.nRemainTime, stateEffect.szValue.c_str(), false, true);
		//}
	}
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNUserSession::ApplyEffectSkill( CDnPlayerActor* pPlayerActor, std::vector<TEffectSkillData>& vEffectSkill, bool bOnInitialize , bool bIsPvpGameRoom )
{	
	if( !pPlayerActor || ( pPlayerActor && !pPlayerActor->GetActorHandle() ) )
		return;

	if( vEffectSkill.empty() )
		return;

	std::vector<TEffectSkillData>::iterator itor = vEffectSkill.begin();

	for(;itor != vEffectSkill.end(); itor++)
	{
		ApplyEffectSkill( pPlayerActor, &(*itor), bOnInitialize , bIsPvpGameRoom );
	}	
}

void CDNUserSession::ApplyEffectSkill( CDnPlayerActor* pPlayerActor, TEffectSkillData* EffectSkill, bool bOnInitialize , bool bIsPvpGameRoom )
{
	if( !pPlayerActor || ( pPlayerActor && !pPlayerActor->GetActorHandle() ) )
		return;
	if( !EffectSkill )
		return;
	if (EffectSkill->nSkillID <= 0 || ( EffectSkill->tExpireDate <= 0 && EffectSkill->bEternity == false ))
		return;

	const TSkillData* pSkill = g_pDataManager->GetSkillData(EffectSkill->nSkillID);
	if (!pSkill || pSkill->vLevelDataList.empty())
		return;

	int nItemType = g_pDataManager->GetItemMainType(EffectSkill->nItemID);
	if( ( (nItemType == ITEMTYPE_GLOBAL_PARTY_BUFF) || (nItemType == ITEMTYPE_BESTFRIEND) ) && bIsPvpGameRoom )
		return;

	CDnSkill::SkillInfo skillInfo;
	std::vector<CDnSkill::StateEffectStruct> stateEffects;
#if defined( PRE_FIX_BUFFITEM )
	CDnSkill::CreateSkillInfo(EffectSkill->nSkillID, EffectSkill->nSkillLevel, skillInfo, stateEffects);
#else
	CDnSkill::CreateSkillInfo(pSkill->nSkillID, pSkill->vLevelDataList[0].cSkillLevel, skillInfo, stateEffects);
#endif

	skillInfo.hSkillUser = pPlayerActor->GetActorHandle();

	for (CDnSkill::StateEffectStruct stateEffect : stateEffects)
	{
		int nRemainTime = EffectSkill->nRemainTime * 1000;
		if( EffectSkill->bEternity == true )
			nRemainTime = -1;

		int iBlowID = pPlayerActor->CmdAddStateEffect(&skillInfo, static_cast<STATE_BLOW::emBLOW_INDEX>(stateEffect.nID), nRemainTime , stateEffect.szValue.c_str(), true, true);
		pPlayerActor->SendRemoveStateEffectGraphic(static_cast<STATE_BLOW::emBLOW_INDEX>(stateEffect.nID));
		if( -1 != iBlowID )
		{			
			DnBlowHandle hBlow = pPlayerActor->GetStateBlowFromID( iBlowID );
			if( hBlow )
			{
				// #45646 Fill HP/MP to full only on initial entry to game server after using an Origin item.
				hBlow->FromSourceItem();

				if( bOnInitialize )
					hBlow->SetHPMPFullWhenBegin();

				EffectSkill->bApplySkill = true;
			}
		}
	}
}

void CDNUserSession::RemoveEffectSkill( CDnPlayerActor* pPlayerActor, TEffectSkillData* EffectSkill )
{
	if( !pPlayerActor || ( pPlayerActor && !pPlayerActor->GetActorHandle() ) )
		return;
	if( !EffectSkill )
		return;
	if( EffectSkill->nSkillID <= 0 )
		return;

	const TSkillData* pSkill = g_pDataManager->GetSkillData(EffectSkill->nSkillID);
	if (!pSkill || pSkill->vLevelDataList.empty())
		return;

	int iSE = pPlayerActor->GetNumAppliedStateBlow();
	for( int i = 0; i < iSE; ++i )
	{
		DnBlowHandle hBlow = pPlayerActor->GetAppliedStateBlow( i );
		if( hBlow )
		{
			if( hBlow->GetParentSkillInfo() && hBlow->GetParentSkillInfo()->iSkillID == pSkill->nSkillID )
				pPlayerActor->AddStateBlowIDToRemove( hBlow->GetBlowID() );
		}
	}
	// Delete right now on the server as well what was requested to be deleted.
	EffectSkill->bApplySkill = false;
	pPlayerActor->RemoveResetStateBlow();
}

#if defined( PRE_FIX_BUFFITEM )
void CDNUserSession::RemoveApplySkill( CDnPlayerActor* pPlayerActor, int nSkillID )
{
	if( !pPlayerActor || ( pPlayerActor && !pPlayerActor->GetActorHandle() ) )
		return;
	if( nSkillID <= 0 )
		return;

	const TSkillData* pSkill = g_pDataManager->GetSkillData(nSkillID);
	if (!pSkill || pSkill->vLevelDataList.empty())
		return;

	int iSE = pPlayerActor->GetNumAppliedStateBlow();
	for( int i = 0; i < iSE; ++i )
	{
		DnBlowHandle hBlow = pPlayerActor->GetAppliedStateBlow( i );
		if( hBlow )
		{
			if( hBlow->GetParentSkillInfo() && hBlow->GetParentSkillInfo()->iSkillID == pSkill->nSkillID )
				pPlayerActor->AddStateBlowIDToRemove( hBlow->GetBlowID() );
		}
	}
	// Delete right now on the server as well what was requested to be deleted.
	pPlayerActor->RemoveResetStateBlow();
}
#endif
#endif

int CDNUserSession::TryWarpVillage(int nMapIndex, INT64 nItemSerial)
{
	if (m_pGameRoom->GetRoomState() != _GAME_STATE_PLAY)
		return ERROR_GENERIC_INVALIDREQUEST;

	if (!IsWindowState(WINDOW_BLIND))
		return ERROR_ITEM_FAIL;

	CDnGameTask *pTask = (CDnGameTask*)m_pGameRoom->GetTaskMng()->GetTask("GameTask");
	SetNextVillageData(NULL, 0, pTask->GetMapTableID(), nMapIndex, 1);
	g_pMasterConnectionManager->SendRequestNextVillageInfo(GetWorldSetID(), GetAccountDBID(), nMapIndex, 1, true, nItemSerial);

	return ERROR_NONE;
}

void CDNUserSession::SetGuildRewardItem( TGuildRewardItem* GuildRewardItem )
{
#if defined( PRE_ADD_GUILDREWARDITEM )
	if(GetPlayerActor())
	{
		for(int i = GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE1; i <= GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE4; i++)
		{			
			std::map<int, TGuildRewardItem>::iterator itor = m_mGuildRewardItem.find(i);
			if( itor != m_mGuildRewardItem.end() )
			{
				// For skills, nEffectValue is level and nEffectValue2 is skill index			
				DnSkillHandle hSkill = GetPlayerActor()->FindSkill( itor->second.nEffectValue2 );
				if( hSkill )
					GetPlayerActor()->RemoveSkill(hSkill->GetClassID());
			}
		}
	}
	m_mGuildRewardItem.clear();
#endif
	for(int i=GUILDREWARDEFFECT_TYPE_EXTRAEXP; i<GUILDREWARDEFFECT_TYPE_CNT; i++)
	{
		if( GuildRewardItem[i].nItemID > 0 )
		{
			m_mGuildRewardItem.insert(pair<int,TGuildRewardItem>(i, GuildRewardItem[i]));
		}
	}
}

int CDNUserSession::GetGuildRewardItemValue( int nType )
{
	std::map<int, TGuildRewardItem>::iterator itor = m_mGuildRewardItem.find(nType);
	if( itor != m_mGuildRewardItem.end() )
	{
		if( itor->second.nItemID > 0 && itor->second.nEffectValue > 0 )
			return itor->second.nEffectValue;
	}
	return 0;
}

#if defined( PRE_ADD_GUILDREWARDITEM )
void CDNUserSession::SetGuildRewardItem()
{
	const TGuildUID GuildUID = GetGuildUID();

	if(GuildUID.IsSet()) 
	{		
		CDNGuildBase* pGuild = g_pGuildManager->At(GuildUID);
		if (pGuild) 
		{	
#if !defined( PRE_ADD_NODELETEGUILD )
			CDetachAutoEx<CDNGuildBase>	AutoDetach(pGuild);
			if (pGuild->IsEnable()) 
			{
#endif
				TGuildRewardItem *RewardItemInfo;
				RewardItemInfo = pGuild->GetGuildRewardItem();
				SetGuildRewardItem(RewardItemInfo);
				SendGuildRewardItem(RewardItemInfo);
#if !defined( PRE_ADD_NODELETEGUILD )
			}			
#endif
		}		
	}
}

void CDNUserSession::ApplyGuildRewardSkill( bool bUseItem/*=false*/ )
{
	if(!GetPlayerActor())
		return;

	for(int i = GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE1; i <= GUILDREWARDEFFECT_TYPE_ADDSKILLTYPE4; i++)
	{
		std::map<int, TGuildRewardItem>::iterator itor = m_mGuildRewardItem.find(i);
		if( itor != m_mGuildRewardItem.end() )
		{			
			AddGuildRewardItemBuff( itor->second.nEffectValue2);
		}
	}
	ApplyGuildRewardItemBuff(bUseItem);
}

void CDNUserSession::AddGuildRewardItemBuff( int nItemID )
{	
	m_GuildRewardbuffItemList.push_back(nItemID);
}

void CDNUserSession::ApplyGuildRewardItemBuff( bool bUseItem/*=false*/ )
{		
	for( std::list<int>::iterator itor = m_GuildRewardbuffItemList.begin();itor != m_GuildRewardbuffItemList.end(); itor++ )
	{
		CSUseItem pPacket;
		memset(&pPacket, 0, sizeof(pPacket));
		TItemData *pItemData = g_pDataManager->GetItemData(*itor);
		if( g_pDataManager->IsUseItemAllowMapTypeCheck( pItemData->nItemID, GetGameRoom()->m_iMapIdx ) )
		{
			TEffectSkillData* EffectSkill = GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pItemData->nSkillID );
			if( !EffectSkill )
			{						
				GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pItemData, GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, false, bUseItem);
			}
		}

	}
}

#endif
void CDNUserSession::SetPeriodExpItemRate()
{
	// Check EXP gain item..	
	std::vector<const TItem*> vItem;
	m_pItem->GetCashInventoryItemListByType( ITEMTYPE_PERIODEXPITEM, vItem );
	m_nPeriodeExpItemRate = 0;

	if( vItem.empty() )		
		return;
	
	for( UINT i=0 ; i<vItem.size() ; ++i )
	{
		if( m_nPeriodeExpItemRate < g_pDataManager->GetItemTypeParam1(vItem[i]->nItemID) )
		{
			if( g_pDataManager->GetItemTypeParam2(vItem[i]->nItemID) > 0 &&	GetLevel() >= g_pDataManager->GetItemTypeParam2(vItem[i]->nItemID) )
				continue;

			m_nPeriodeExpItemRate = g_pDataManager->GetItemTypeParam1(vItem[i]->nItemID);
		}
	}	
}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
void CDNUserSession::ApplyPartyEffectSkillItemData( CSUseItem *pPacket, TItemData *pItemData, UINT nUseSessionID, EffectSkillNameSpace::ShowEffectType::eType eType, bool bSendDB, bool bUseItem )
{
	if( GetGameRoom() )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(GetGameRoom()->GetTaskMng()->GetTask("PartyTask"));
		if( pPartyTask && pItemData)
		{
			if(pItemData->nTypeParam[0] == EffectSkillNameSpace::BuffType::Partybuff )
			{
				pPartyTask->ApplyPartyEffectSkillItemData(pPacket, pItemData, nUseSessionID, eType, bSendDB, bUseItem);				
			}
			else
			{
				GetItem()->ApplyPartyEffectSkillItemData( pPacket, pItemData, GetSessionID(), eType, true, true );
			}
			
		}
	}
}
#endif


#if defined( PRE_ADD_BESTFRIEND )
void CDNUserSession::BestFriendChangeLevel(BYTE cLevel, bool bSend /*= false*/)
{
	if(bSend)
	{
		if( GetBestFriend()->IsRegistered() )
		{
			sWorldUserState State;
			if (g_pWorldUserState->GetUserState(GetBestFriend()->GetInfo().wszName, GetBestFriend()->GetInfo().biCharacterDBID, &State))
			{
				if (g_pMasterConnectionManager)
					g_pMasterConnectionManager->SendLevelUpBestFriend(GetWorldSetID(), cLevel, m_pBestFriend->GetInfo().wszName);
			}
		}
	}
	else
		GetBestFriend()->ChangeLevel(cLevel);
}
#endif

#if defined( PRE_ADD_DIRECTNBUFF )
void CDNUserSession::SendDirectPartyBuffMsg()
{
	for(std::list<int>::iterator itor = m_DirectPartyBuffItemList.begin();itor!=m_DirectPartyBuffItemList.end();itor++)
	{
		for (DWORD i = 0; i < m_pSession->GetGameRoom()->GetUserCount(); ++i)
		{
			if( !g_pDataManager->IsUseItemAllowMapTypeCheck( *itor, GetGameRoom()->m_iMapIdx ) )
				continue;
			const CDNGameRoom::PartyStruct* pPartyStruct = m_pSession->GetGameRoom()->GetPartyData(i);
			if(pPartyStruct && pPartyStruct->pSession)
			{				
				pPartyStruct->pSession->SendWorldSystemMsg(WORLDCHATTYPE_DIRECTPARTYBUFF, *itor, GetCharacterName(), 0 );
			}			
		}
	}
}
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
void CDNUserSession::SetComebackAppellation( int nAppelationID )
{
	if( m_nComebackAppellation != nAppelationID )
	{
		int nPrevComebackAppellation = m_nComebackAppellation;
		m_nComebackAppellation = nAppelationID;

		GetDBConnection()->QueryModMemberComebackInfo( this, GetPartyID() );

		if( nPrevComebackAppellation > 0 )
		{			
			// Delete existing buff
			if( GetGameRoom() && GetGameRoom()->bIsComebackParty() )
			{
				GetGameRoom()->DelPartyMemberAppellation(nPrevComebackAppellation);
			}
		}		

		if( m_nComebackAppellation > 0 )
		{
			// Apply buff and update party state
			TAppellationData *pAData = g_pDataManager->GetAppellationData( nAppelationID );
			if( pAData )
			{
				if( pAData->nSkillItemID > 0 )
				{
					CSUseItem pPacket;
					memset(&pPacket, 0, sizeof(pPacket));
					pPacket.biInvenSerial = GetItem()->MakeItemSerial();
					TItemData* pBuffItemData = g_pDataManager->GetItemData( pAData->nSkillItemID );
					if( pBuffItemData && GetGameRoom() )
					{	
						GetGameRoom()->SetComebackParty(true);
						bool bUseItem = true;
						EWorldEnum::MapTypeEnum MapType = CDnWorld::GetInstance(GetGameRoom()).GetMapType();
						if( MapType == EWorldEnum::MapTypeWorldMap )
							bUseItem = false;
						ApplyPartyEffectSkillItemData(&pPacket, pBuffItemData, m_pSession->GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, false, bUseItem);
					}					
				}
			}
		}		
	}
}
#endif

#if defined( PRE_FIX_BUFFITEM )
void CDNUserSession::CheckEffectSkillItemData()
{
	std::vector<TEffectSkillData> vEffectSkill;
	vEffectSkill.clear();
	GetItem()->GetEffectSkillItem( vEffectSkill );
	if(vEffectSkill.empty())
		return;

	for( int i=0;i<vEffectSkill.size();i++ )
	{
		if( !g_pDataManager->IsUseItemAllowMapTypeCheck( vEffectSkill[i].nItemID, GetGameRoom()->m_iMapIdx ) )
		{			
			CDNUserSession::RemoveEffectSkill(GetPlayerActor(), &vEffectSkill[i]);
			GetItem()->BroadcastDelEffectSkillItemData( vEffectSkill[i].nItemID );
		}
		else
		{
			CSUseItem pPacket;
			memset(&pPacket, 0, sizeof(pPacket));
			pPacket.biInvenSerial = GetItem()->MakeItemSerial();
			TItemData* pBuffItemData = g_pDataManager->GetItemData(vEffectSkill[i].nItemID);
			if( pBuffItemData )
			{				
				TEffectSkillData* EffectSkill = GetItem()->GetEffectSkillItem( EffectSkillNameSpace::SearchType::SkillID, pBuffItemData->nSkillID );
				if( EffectSkill && EffectSkill->bApplySkill == false )
				{
					GetItem()->BroadcastEffectSkillItemData( false, pBuffItemData->nSkillID, EffectSkillNameSpace::ShowEffectType::NONEEFFECT );
					CDNUserSession::ApplyEffectSkill(GetPlayerActor(), EffectSkill, true, GetGameRoom()->bIsPvPRoom() );
				}				
			}			
		}
	}
}
#endif

#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
int CDNUserSession::GetStageClearBonusReward(char *pData, int nLen)
{
	if(nLen != sizeof(CSStageClearBonusRewardSelect))
		return ERROR_INVALIDPACKET;

	CSStageClearBonusRewardSelect* pPacket = (CSStageClearBonusRewardSelect*)pData;
	if(pPacket->nItemID <= 0 || pPacket->nPropID <= 0)	
		return ERROR_GENERIC_INVALIDREQUEST;

	//Find reward item
	int nMapID = m_pGameRoom->GetGameTask()->GetMapTableID();
	int nDropItemGroup = g_pDataManager->GetBonusDropGroupID(nMapID, pPacket->nItemID);
	if(nDropItemGroup == 0)
	{
		//If reward group cannot be found by map and required item, it is packet manipulation or invalid resource
		g_Log.Log(LogType::_ERROR, this, L"[StageClearBonusReward] BonusReward Find Fail(MapID[%d] NeedItemID[%d])\n", nMapID, pPacket->nItemID);
		return ERROR_GENERIC_INVALIDREQUEST;
	}
	//Check if item exists in inven
	bool IsCashItem = g_pDataManager->IsCashItem(pPacket->nItemID);
	 
	if(IsCashItem)
		if(m_pItem->GetCashItemCountByItemID(pPacket->nItemID) <= 0)	
			return ERROR_GENERIC_INVALIDREQUEST;
	else
		if(m_pItem->GetInventoryItemCount(pPacket->nItemID) <= 0)
			return ERROR_GENERIC_INVALIDREQUEST;
	
	//Check prop
	std::vector<CEtWorldProp *> VecProp;
	m_pGameRoom->GetWorld()->FindPropFromCreateUniqueID( pPacket->nPropID, &VecProp );
	if( VecProp.empty() ) 
		return ERROR_GENERIC_INVALIDREQUEST;
	
	//Check prop type. If not Chest type, it is not the target...
	if(static_cast<CDnWorldProp *>( VecProp[0] )->GetPropType() != PTE_Operation)
		return ERROR_GENERIC_INVALIDREQUEST;

	CDnWorldOperationProp *pProp = static_cast<CDnWorldOperationProp *>( VecProp[0] );
	if( !pProp )
		return ERROR_GENERIC_INVALIDREQUEST;

	if(!pProp->IsShow())
		return ERROR_GENERIC_INVALIDREQUEST;

	CDnItemTask *pItemTask = (CDnItemTask *)GetGameRoom()->GetTaskMng()->GetTask( "ItemTask" );
	if(!pItemTask) 
		return ERROR_GENERIC_INVALIDREQUEST;

	//Check if already received reward or clicked the prop
	if(pProp->IsClickPropAndCheckUser(m_hActor))
	{
		//If a user who already received the reward or never clicked the prop reaches here, it could be a hack or packet manipulation...
		g_Log.Log(LogType::_ERROR, this, L"[StageClearBonusReward] Retry BonusReward(MapID:[%d] PropID[%d] NeedItemID[%d]\n", nMapID, pPacket->nPropID,  pPacket->nItemID);
		return ERROR_GENERIC_INVALIDREQUEST;
	}
	//Check reward
	DNVector(CDnItem::DropItemStruct) VecDropItemList;
	CDnDropItem::CalcDropItemList(GetGameRoom(),  m_pGameRoom->GetGameTask()->GetStageDifficulty(), nDropItemGroup, VecDropItemList);
	if(VecDropItemList.empty())
	{
		//Drop group ID exists, but if there is no actual item to drop, it is a resource issue.. log it
		g_Log.Log(LogType::_ERROR, this, L"[StageClearBonusReward] DropItemGroup Is Empty(MapID:[%d] NeedItemID[%d] DropGroupID[%d])\n", nMapID, pPacket->nItemID, nDropItemGroup);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	//Delete key item
	if(IsCashItem)
	{
		if(!m_pItem->DeleteCashInventoryByItemID(pPacket->nItemID, 1, DBDNWorldDef::UseItem::Use))
		{
			g_Log.Log(LogType::_ERROR, this, L"[StageClearBonusReward] Delete Item Failed(Cash)(MapID:[%d] NeedItemID[%d] DropGroupID[%d]) \n", nMapID, pPacket->nItemID, nDropItemGroup);
			return ERROR_GENERIC_INVALIDREQUEST;
		}
	}
	else
	{
		if(!m_pItem->DeleteInventoryByItemID(pPacket->nItemID, 1, DBDNWorldDef::UseItem::Use))
		{
			g_Log.Log(LogType::_ERROR, this, L"[StageClearBonusReward] Delete Item Failed(Normal)(MapID:[%d] NeedItemID[%d] DropGroupID[%d]) \n", nMapID, pPacket->nItemID, nDropItemGroup);
			return ERROR_GENERIC_INVALIDREQUEST;
		}
	}

	for( DWORD j=0; j<VecDropItemList.size(); j++ ) {
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		pItemTask->RequestDropItem( VecDropItemList[j].dwUniqueID, *(GetActorHandle()->GetPosition()), VecDropItemList[j].nItemID, VecDropItemList[j].nSeed, VecDropItemList[j].nCount, (short)( _rand(GetGameRoom())%360 ), m_nSessionID, VecDropItemList[j].nEnchantID);
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
		pItemTask->RequestDropItem( VecDropItemList[j].dwUniqueID, *(GetActorHandle()->GetPosition()), VecDropItemList[j].nItemID, VecDropItemList[j].nSeed, VecDropItemList[j].nCount, (short)( _rand(GetGameRoom())%360 ), m_nSessionID);
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	}
	GetEventSystem()->OnEvent( EventSystem::OnItemUse, 1, EventSystem::ItemID, pPacket->nItemID);
	//Prop Disabled
	pProp->CmdEnableOperator(m_hActor, false);
	return ERROR_NONE;
}
void CDNUserSession::SendStageClearBonusRewardResult(int nResult)
{
	SCStageClearBonusRewardSelect TxPacket;
	memset(&TxPacket, 0, sizeof(SCStageClearBonusRewardSelect));
	TxPacket.nResult = nResult;

	AddSendData(SC_CHAR, eChar::SC_STAGECLEAR_BONUSREWARD_SELECT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket));
}
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)

#if defined( PRE_ALTEIAWORLD_EXPLORE )

int CDNUserSession::AlteiaWorldDice()
{
	if( GetGameRoom() )
	{
		if( GetGameRoom()->bIsAlteiaWorld() )
		{
			if( !m_bCanDice )
			{
				SendAlteiaWorldDiceResult( ERROR_GENERIC_INVALIDREQUEST );
				return ERROR_GENERIC_INVALIDREQUEST;
			}
			int nDiceNumber = (_rand(GetGameRoom()) % MAXALTEIADICENUMBER)+1;
			m_nAlteiaWorldPosition += nDiceNumber;
			if( m_nAlteiaWorldPosition >= g_pDataManager->GetAlteiaWorldMapMaxCount() )
			{
				nDiceNumber = g_pDataManager->GetAlteiaWorldMapMaxCount() - (m_nAlteiaWorldPosition - nDiceNumber);
				m_nAlteiaWorldPosition = g_pDataManager->GetAlteiaWorldMapMaxCount();
			}
			
			TAlteiaWorldMapInfo* pAlteiaWorldMapInfo = g_pDataManager->GetAlteiaWorldMapInfo(m_nAlteiaWorldPosition);

			if(pAlteiaWorldMapInfo)
			{				
				m_dwAlteiaWorldMoveNextMapTick = timeGetTime();
				SendAlteiaWorldDiceResult( ERROR_NONE, nDiceNumber);
				m_bCanDice = false;
				return ERROR_NONE;
			}
			else
			{
				g_Log.Log(LogType::_NORMAL, m_pSession, L"AlateiaWorldMapInfo Index:d Error!!\r\n",m_nAlteiaWorldPosition);
				SendAlteiaWorldDiceResult( ERROR_GENERIC_INVALIDREQUEST );
				return ERROR_GENERIC_INVALIDREQUEST;
			}

		}
	}
	return ERROR_GENERIC_INVALIDREQUEST;
}

void CDNUserSession::MoveAlteiaNextMap()
{
	if( m_bCanDice )
	{
		SendAlteiaWorldDiceResult( ERROR_GENERIC_INVALIDREQUEST );
		return;
	}

	TAlteiaWorldMapInfo* pAlteiaWorldMapInfo = g_pDataManager->GetAlteiaWorldMapInfo(m_nAlteiaWorldPosition);

	if(pAlteiaWorldMapInfo)
	{
		GetGameRoom()->SetAlteiaWorldMap(true);
		GetGameRoom()->ReserveInitStateAndSync( pAlteiaWorldMapInfo->nMapID, 1, CRandom::Seed(GetGameRoom()), pAlteiaWorldMapInfo->Difficulty, true );
		m_dwAlteiaWorldMoveNextMapTick = 0;
		m_bCanDice = true;
	}
}

void CDNUserSession::AddAlteiaWorldResult( bool bClearFlag )
{
	if( bClearFlag )
	{
		int nGoldKeyCount = 0;
		int nGoldKeyItemID = 0;
		m_cDailyPlayCount++;
		std::vector<TItem *> pVecResultList;
		int nGoldkeyItemID = (int)CGlobalWeightIntTable::GetInstance().GetValue(CGlobalWeightIntTable::AlteiaWorldGoldKeyItemID);
		m_pItem->GetInventoryItemListFromItemID(nGoldkeyItemID , pVecResultList );
		for( std::vector<TItem *>::iterator itor = pVecResultList.begin(); itor != pVecResultList.end();itor++)
		{
			nGoldKeyCount += (*itor)->wCount;
		}
		DWORD dwPlayTick = GetGameRoom()->GetAlteiaPlayTime();
		if( dwPlayTick > 0 )
			dwPlayTick = dwPlayTick/1000;
		GetDBConnection()->QueryAddAlteiaWorldPlayResult( this, nGoldKeyCount, dwPlayTick, GetGuildUID().nDBID );
		if( m_cDailyPlayCount >= (BYTE)CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AlteiaWorldDailyPlayCount ) )
		{
			// Grant buff
			int nBuffItemID = (int)CGlobalWeightIntTable::GetInstance().GetValue(CGlobalWeightIntTable::AlteiaWorldBuffItemID);
			TItemData *pItemData = g_pDataManager->GetItemData(nBuffItemID);
			if( pItemData )
			{
				CSUseItem pPacket;
				memset(&pPacket, 0, sizeof(pPacket));
				GetItem()->ApplyPartyEffectSkillItemData(&pPacket, pItemData, GetSessionID(), EffectSkillNameSpace::ShowEffectType::NONEEFFECT, true, false);
			}
		}
	}	
}

#endif
