#include "stdafx.h"
#include "DNCommonDef.h"
#include "DNVillageConnection.h"
#include "DNUser.h"
#include "DNGameConnection.h"
#include "DNLoginConnection.h"
#include "DNDivisionManager.h"
#include "DNExtManager.h"
#include "DNPvP.h"
#include "Log.h"
#include <boost/format.hpp>
#include <boost/static_assert.hpp>
#include "EtUIXML.h"

#include "DNFarm.h"
#include "DNGuildWarManager.h"

extern TMasterConfig g_Config;

CDNVillageConnection::CDNVillageConnection(void): CConnection(), m_cVillageID(0)
{
#if defined(_FINAL_BUILD)
	Init(1024 * 1024, 1024 * 1024);
#else	// #if defined(_FINAL_BUILD)
	Init(1024 * 100, 1024 * 100);
#endif	// #if defined(_FINAL_BUILD)

	m_VillageChannelInfo.cVillageID = 0;
	memset(m_VillageChannelInfo.szIP, 0, sizeof(char[IPLENMAX]));
	m_VillageChannelInfo.nPort = 0;
	m_bZeroPopulation = false;
	m_nManagedID = 0;
	m_bConnectComplete = false;
}

CDNVillageConnection::~CDNVillageConnection(void)
{
}

int CDNVillageConnection::MessageProcess(int iMainCmd, int iSubCmd, char * pData, int iLen)
{
	switch (iMainCmd)
	{
	case VIMA_REGIST:
		{
			VIMARegist * pRegist = (VIMARegist*)pData;
			if (m_cVillageID > 0)
			{
				_DANGER_POINT();
				return ERROR_GENERIC_UNKNOWNERROR;
			}

			m_nManagedID = pRegist->nManagedID;
			if (g_pDivisionManager->GetVillageServerInfo(pRegist->cVillageID, &m_VillageChannelInfo, pRegist->szIP, pRegist->nPort) == true)
			{
				SendReqUserList(ERROR_NONE);

				m_cVillageID = pRegist->cVillageID;
				m_VillageChannelInfo.nManagedID = m_nManagedID;

				g_Log.Log(LogType::_FILEDBLOG, g_Config.nWorldSetID, 0, 0, 0, L"[VIMA_REGIST] VillageID:%d \r\n", m_cVillageID);
				return ERROR_NONE;
			}
			
			//This needs immediate verification, so print with printf and trigger assert.
			//printf("VillageID Already Connected Check ServerStruct.xml villageID\n");
			//SendReqUserList(ERROR_GENERIC_INVALIDREQUEST);
			g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, 0, 0, 0, L"Village Info Not Found [VillageID:%d] [IP:%S] [PORT:%d]\n", pRegist->cVillageID, pRegist->szIP, pRegist->nPort);

			DetachConnection(L"VillageID Already Connected");
			return ERROR_GENERIC_UNKNOWNERROR;
		}
		break;

	case VIMA_ENDOFREGIST:
		{
			m_bConnectComplete = true;
#if defined(PRE_FIX_LIVE_CONNECT)
			g_pDivisionManager->AddVillageServerConnection(this);
#endif //#if defined(PRE_FIX_LIVE_CONNECT)
			SendVillageRegistComplete();

			g_pDivisionManager->SendVillageInfo();				//Send channel sync
			g_pDivisionManager->SendWorldUserStateList(this);	//Send user state sync

			g_pDivisionManager->SendFarmInfo();
			g_pDivisionManager->CheckPvPLobbyChannel( this );
			g_pDivisionManager->SendGuildWarInfoToVillage( this );
			g_Log.Log(LogType::_FILEDBLOG, g_Config.nWorldSetID, 0, 0, 0, L"[VIMA_ENDOFREGIST] VillageID:%d \r\n", m_cVillageID);
			return ERROR_NONE;
		}
		break;

	case VIMA_CHECKUSER:
		{
			VIMACheckUser *pCheckUser = (VIMACheckUser*)pData;
			if (g_pDivisionManager->VillageCheckUser(this, pCheckUser->nSessionID) == false)
				return ERROR_GENERIC_UNKNOWNERROR;			
			return ERROR_NONE;
		}
		break;

	case VIMA_ADDUSERLIST:
		{
			VIMAAddUserList *pAdd = (VIMAAddUserList*)pData;
			// Add User in master and

#if defined(PRE_MOD_SELECT_CHAR)
			g_pDivisionManager->AddUser(pAdd, m_cVillageID, true);
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pAdd->nAccountDBID );			
#else	// #if defined(PRE_MOD_SELECT_CHAR)

			BYTE szMID[MACHINEIDMAX] = {0,};
			DWORD dwGRC = 0;
			g_pDivisionManager->AddUser(m_cVillageID, pAdd->wChannelID, 0, 0, pAdd->nAccountDBID, pAdd->nSessionID, pAdd->biCharacterDBID, pAdd->wszCharacterName, pAdd->wszAccountName, 
				pAdd->bAdult, pAdd->szIP, pAdd->szVirtualIp, 0, szMID, dwGRC, pAdd->cPCBangGrade, 0, true);

			// Set PvP village info
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pAdd->nAccountDBID );
			if( pUser )
				pUser->SetPvPVillageInfo( pAdd->cPvPVillageID, pAdd->unPvPVillageChannelID );

#endif	// #if defined(PRE_MOD_SELECT_CHAR)

			return ERROR_NONE;
		}

	case VIMA_DELUSER:
		{
			VIMADelUser *pDelUser = (VIMADelUser*)pData;
			g_pDivisionManager->DelUser(pDelUser->nAccountDBID, false, pDelUser->nSessionID);
			return ERROR_NONE;
		}
		break;

	// Go~ Game~
	case VIMA_REQGAMEID:
		{
			VIMAReqGameID* pPacket = reinterpret_cast<VIMAReqGameID*>(pData);
			if (g_pDivisionManager->RequestGameRoom(pPacket) == false)
			{
#if defined( PRE_WORLDCOMBINE_PARTY )
				SendSetGameID( pPacket->GameTaskType, pPacket->cReqGameIDType, pPacket->InstanceID, 0, 0, 0, 0, 0, 0, pPacket->unVillageChannelID, GetVillageID(), -1, pPacket->PartyData.PartyData.Type );
				if( Party::bIsWorldCombineParty(pPacket->PartyData.PartyData.Type) == false )
#else
				SendSetGameID( pPacket->GameTaskType, pPacket->cReqGameIDType, pPacket->InstanceID, 0, 0, 0, 0, 0, 0, pPacket->unVillageChannelID, GetVillageID(), -1 );
#endif
				_DANGER_POINT();
			}
			return ERROR_NONE;
		}
		break;
	case VIMA_LADDER_REQGAMEID:
		{
			VIMALadderReqGameID* pPacket = reinterpret_cast<VIMALadderReqGameID*>(pData);
			if( g_pDivisionManager->RequestGameRoom(pPacket) == false )
			{
				GAMASetRoomID TempPacket;
				memset( &TempPacket, 0, sizeof(TempPacket) );
				for( int i=0 ; i<pPacket->LadderData.cMemberCount ; ++i )
					TempPacket.nRoomMember[i] = pPacket->LadderData.MemberInfoArr[i].uiAccountDBID;

				SendLadderSetGameID( 0, 0, 0, 0, 0, 0, pPacket->unVillageChannelID, GetVillageID(), -1, &TempPacket );
			}
			return ERROR_NONE;
		}
	case VIMA_LADDER_OBSERVER:
		{
			VIMALadderObserver* pPacket = reinterpret_cast<VIMALadderObserver*>(pData);

			CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biCharacterDBID );
			if( pUser == NULL )
				break;
			CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pPacket->iGameServerID );
			if( pGameCon == NULL )
				break;
			pGameCon->SendLadderObserver( pPacket->iGameRoomID, pUser );
			return ERROR_NONE;
		}
	case VIMA_LADDER_ENTERCHANNEL:
		{
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
			VIMALadderEnterChannel* pPacket = reinterpret_cast<VIMALadderEnterChannel*>(pData);
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( pUser )
			{
				if( !pUser->bIsPvPLobby() ) // If entered when not in PvP lobby, it was clicked from village..				
					pUser->SetLadderMatchType(pPacket->MatchType);
			}			
#endif
			// Sent for packet synchronization, so send it back as-is.
			AddSendData( MAVI_LADDERSYSTEM_ENTERCHANNEL, 0, pData, iLen );
			return ERROR_NONE;
		}
	case VIMA_LADDER_INVITEUSER:
		{
			VIMALadderInviteUser * pPacket = (VIMALadderInviteUser*)pData;
			CDNUser * pDestUser = g_pDivisionManager->GetUserByName(pPacket->wszCharName);
			if (pDestUser)
			{
				if (pDestUser->bIsPvPLobby() == false)
				{
					//The user is not in the waiting room.
					pPacket->nRetCode = ERROR_LADDERSYSTEM_CANT_RECVINVITESTATE;
					AddSendData( MAVI_LADDERSYSTEM_INVITEUSER, 0, pData, iLen );
					return ERROR_NONE;
				}
			}

			// Sent for packet synchronization, so send it back as-is.
			AddSendData( MAVI_LADDERSYSTEM_INVITEUSER, 0, pData, iLen );
			return ERROR_NONE;
		}
	case VIMA_LADDER_INVITECONFIRM:
		{
			// Sent for packet synchronization, so send it back as-is.
			AddSendData( MAVI_LADDERSYSTEM_INVITECONFIRM, 0, pData, iLen );
			return ERROR_NONE;
		}
	//Enter Village~
	case VIMA_ENTERVILLAGE:
		{
			VIMAEnterVillage * pPacket = (VIMAEnterVillage*)pData;		//Separate from the check, actual enter processing is complete.
			if (g_pDivisionManager->EnterVillage(pPacket->nAccountDBID, m_cVillageID, pPacket->nChannelID) == false)
			{
				g_pDivisionManager->SendDetachUser(pPacket->nAccountDBID);
				g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, pPacket->nAccountDBID, 0, 0, L"EnterVillage CheckFail ADBID[%d]\n", pPacket->nAccountDBID);
			}
			return ERROR_NONE;
		}
		break;

		//Reconnect Login
	case VIMA_RECONNECTLOGIN:
		{
			VIMAReconnectLogin * pPacket = (VIMAReconnectLogin*)pData;
			g_pDivisionManager->ReconnectLogin(pPacket->nAccountDBID);
			CDNUser * pUser = g_pDivisionManager->GetUserByAccountDBID(pPacket->nAccountDBID);
			if(pUser)
			{
				g_pDivisionManager->UpdateUserState(pUser->GetCharacterName(), pUser->GetCharacterDBID(), _LOCATION_NONE, _COMMUNITY_NONE);
			}			
			return ERROR_NONE;
		}

	case VIMA_PROCESSDELAYED:
		{
			VIMAProcessDelayed * pPacket = (VIMAProcessDelayed*)pData;
			g_Log.Log(LogType::_ERROR, g_Config.nWorldSetID, 0, 0, 0, L"Village Process Delayed msec[%d]\n", pPacket->nDelayedTick);

			return ERROR_NONE;
		}
		break;

	case VIMA_VILLAGEUSERREPORT:
		{
			VIMAUserReport * pPacket = (VIMAUserReport*)pData;

			for (int i = 0; i < pPacket->nCount; i++)
			{
				std::vector <TChannelInfo>::iterator ii;
				for (ii = m_VillageChannelInfo.vOwnedChannelList.begin(); ii != m_VillageChannelInfo.vOwnedChannelList.end(); ii++)
					if ((*ii).nChannelID == pPacket->Info[i].nChannelID)
						(*ii).nCurrentUser = pPacket->Info[i].nChannelUserCount;
			}
			return ERROR_NONE;
		}
		break;

	// Go~ Village To Village~
	case VIMA_VILLAGETOVILLAGE:
		{
			VIMAVillageToVillage * pPacket = (VIMAVillageToVillage*)pData;
			g_pDivisionManager->MoveVillageToVillage(this, pPacket);
			return ERROR_NONE;
		}
		break;

	case VIMA_LOGINSTATE:
		{
			VIMALoginState *pState = (VIMALoginState*)pData;

			if (!g_pDivisionManager->GetLoginConnection() || g_pDivisionManager->GetLoginConnection()->GetDelete()) {
				SendLoginState(pState->nSessionID, ERROR_LOGIN_FAIL);
			}
			else {
				CDNUser * pUser = g_pDivisionManager->GetUserBySessionID(pState->nSessionID);
				if (pUser) {
//					DecreaseUserCount(pUser->GetChannelID());
					SendLoginState(pState->nSessionID, ERROR_NONE);
				}
				else {
					SendLoginState(pState->nSessionID, ERROR_LOGIN_FAIL);
				}
			}
		}
		break;

#if defined( PRE_PARTY_DB )
	case VIMA_REQPARTYINVITE:
		{
			VIMAReqPartyInvite* pPacket = reinterpret_cast<VIMAReqPartyInvite*>(pData);

			CDNUser* pInviteUser = g_pDivisionManager->GetUserByName( pPacket->wszInviteCharName );
			if( pInviteUser == NULL || pInviteUser->GetUserState() != STATE_VILLAGE )
			{
				SendResPartyInvite( ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND, pPacket->uiReqAccountDBID );
				return ERROR_NONE;
			}

			CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pInviteUser->GetVillageID() );
			if( pVillageCon == NULL )
			{
				SendResPartyInvite( ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND, pPacket->uiReqAccountDBID );
				return ERROR_NONE;
			}

			pVillageCon->SendReqPartyInvite( pPacket );

			return ERROR_NONE;
		}
	case VIMA_RESPARTYINVITE:
		{
			VIMAResPartyInvite* pPacket = reinterpret_cast<VIMAResPartyInvite*>(pData);

			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiReqAccountDBID );
			if( pUser == NULL || pUser->GetUserState() != STATE_VILLAGE )
				return ERROR_NONE;

			CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
			if( pVillageCon == NULL )
				return ERROR_NONE;

			pVillageCon->SendResPartyInvite( pPacket->iRet, pPacket->uiReqAccountDBID );
			return ERROR_NONE;
		}
	case VIMA_PARTYINVITEDENIED:
		{
			VIMAPartyInviteDenied* pPacket = reinterpret_cast<VIMAPartyInviteDenied*>(pData);

			CDNUser* pReqUser = g_pDivisionManager->GetUserByName( pPacket->wszReqCharName );
			if( pReqUser == NULL || pReqUser->GetUserState() != STATE_VILLAGE )
				return ERROR_NONE;

			CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pReqUser->GetVillageID() );
			if( pVillageCon == NULL )
				return ERROR_NONE;

			pVillageCon->SendPartyInviteDenied( pPacket );
			return ERROR_NONE;
		}
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	case VIMA_REQPARTYASKJOIN:
		{
			VIMAReqPartyAskJoin* pPacket = reinterpret_cast<VIMAReqPartyAskJoin*>(pData);

			CDNUser* pTargetUser = g_pDivisionManager->GetUserByName( pPacket->wszTargetChracterName );
			if( pTargetUser == NULL || (pTargetUser->GetUserState() != STATE_VILLAGE && pTargetUser->GetUserState() != STATE_GAME ) )
			{
				SendResPartyAskJoin( ERROR_PARTY_ASKJOIN_FAIL, pPacket->uiReqAccountDBID );
				return ERROR_NONE;
			}

			if( pTargetUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pTargetUser->GetVillageID() );
				if( pVillageCon )
					pVillageCon->SendReqPartyAskJoin( pPacket );
				else
					SendResPartyAskJoin( ERROR_PARTY_ASKJOIN_FAIL, pPacket->uiReqAccountDBID );
			}
			else if( pTargetUser->GetUserState() == STATE_GAME )
			{
				CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pTargetUser->GetGameID() );
				if( pGameCon )
					pGameCon->SendReqPartyAskJoin( pTargetUser->GetAccountDBID(), pPacket );
				else
					SendResPartyAskJoin( ERROR_PARTY_ASKJOIN_FAIL, pPacket->uiReqAccountDBID );				
			}
			
			return ERROR_NONE;
		}
	case VIMA_RESPARTYASKJOIN:
		{
			VIMAResPartyAskJoin* pPacket = reinterpret_cast<VIMAResPartyAskJoin*>(pData);

			CDNUser* pUser = NULL;
			if(pPacket->uiReqAccountDBID > 0 )
				pUser = g_pDivisionManager->GetUserByAccountDBID(pPacket->uiReqAccountDBID);
			else if( pPacket->wszReqChracterName[0] != '\0' )
				pUser = g_pDivisionManager->GetUserByName(pPacket->wszReqChracterName);

			if( pUser && pUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
				if( pVillageCon )
					pVillageCon->SendResPartyAskJoin( pPacket->iRet, pUser->GetAccountDBID());
			}
			return ERROR_NONE;
		}
	case VIMA_ASKJOINAGREEINFO:
		{
			VIMAAskJoinAgreeInfo* pPacket = reinterpret_cast<VIMAAskJoinAgreeInfo*>(pData);

			CDNUser* pUser = g_pDivisionManager->GetUserByName(pPacket->wszAskerCharName);
			if( pUser && pUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
				if( pVillageCon )
					pVillageCon->SendAskJoinAgreeInfo( pUser->GetAccountDBID(), pPacket->PartyID, pPacket->iPassword );
			}
			return ERROR_NONE;
		}
#endif
#else
	case VIMA_REQPARTYID:
		{
			VIMAReqPartyID * pPacket = (VIMAReqPartyID*)pData;

			CDNUser * pUser = g_pDivisionManager->GetUserByAccountDBID(pPacket->nAccountDBID);
			if (pUser == NULL)
			{
				_DANGER_POINT();
				SendResult(pPacket->nAccountDBID, MAVI_REQPARTYIDFAIL, ERROR_PARTY_CREATE_FAIL);
				return ERROR_NONE;
			}

			WCHAR wszName[PARTYNAMELENMAX];			
			memset(wszName, 0, sizeof(wszName));
			_wcscpy(wszName, PARTYNAMELENMAX, pPacket->wszBuf, pPacket->cNameLen);
			WCHAR szPass[PARTYPASSWORDMAX];
			memset(szPass, 0, sizeof(szPass));
			if (pPacket->cPassWordLen > 0)
				_wcscpy(szPass, PARTYPASSWORDMAX, pPacket->wszBuf + pPacket->cNameLen, pPacket->cPassWordLen);
			
			UINT nGetPartyID = g_IDGenerator.GetPartyID();

			SendAddParty(pPacket->nAccountDBID, pUser->GetSessionID(), nGetPartyID, wszName, szPass, pPacket->nChannelID, pPacket->cMemberMax, pPacket->ItemLootRule, pPacket->ItemLootRank, pPacket->cUserLvLimitMin, pPacket->cUserLvLimitMax, pPacket->nTargetMapIdx, pPacket->Difficulty, pPacket->cPartyJobDice, pPacket->nCreateAfterInvite, pPacket->cUseVoice);
			return ERROR_NONE;
		}
		break;
#endif // #if defined( PRE_PARTY_DB )

	case VIMA_RETINVITEPARTYMEMBER:
		{
			VIMAInvitePartyMemberResult * pPacket = (VIMAInvitePartyMemberResult*)pData;

			if (g_pDivisionManager->SendInvitePartyMemberResult(pPacket->nGameServerID, pPacket->wszInviterName, pPacket->wszInvitedName, pPacket->nRetCode) == false)
			{
				//Notify that accept failed.
				CDNUser * pinvitedUser = g_pDivisionManager->GetUserByName(pPacket->wszInvitedName);
				if (pinvitedUser)
					SendResult(pinvitedUser->GetAccountDBID(), MAVI_INVITEPARTYMEMBER_RETMSG, ERROR_PARTY_INVITEAGREEFAIL);
				else
					_DANGER_POINT();
			}
		}
		break;

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	case VIMA_GETPARTYID:
		{
			VIMAGetPartyID * pPacket = (VIMAGetPartyID*)pData;
			CDNUser* pUser = g_pDivisionManager->GetUserByName(pPacket->wszCharName);
			
			if(!pUser) return ERROR_NONE;
			if( pUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
				if( pVillageCon )
					pVillageCon->SendGetPartyID( pUser->GetAccountDBID(), pPacket->nSenderAccountDBID );
			}
			else if( pUser->GetUserState() == STATE_GAME )
			{
				CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pUser->GetGameID() );
				if( pGameCon )
					pGameCon->SendGetPartyID( pUser->GetAccountDBID(), pPacket->nSenderAccountDBID );
			}
			else
				SendGetPartyIDResult( pPacket->nSenderAccountDBID, 0 );
			return ERROR_NONE;
		}
		break;

	case VIMA_GETPARTYID_RESULT:
		{
			VIMAGetPartyIDResult * pPacket = (VIMAGetPartyIDResult*)pData;
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID(pPacket->nSenderAccountDBID);

			if( pUser && pUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
				if( pVillageCon )
				{
					pVillageCon->SendGetPartyIDResult( pPacket->nSenderAccountDBID, pPacket->PartyID );
				}
			}

			return ERROR_NONE;
		}
		break;
#endif

	case VIMA_INVAITEGUILDMEMB:			// Guild invite
		{
			MAInviteGuildMember * pPacket = (MAInviteGuildMember*)pData;
			if (g_pDivisionManager->SendInviteGuildMember(reinterpret_cast<MAInviteGuildMember*>(pPacket)) == false)
			{
				SendInviteGuildMemberResult(pPacket->nAccountDBID, ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED, false, pPacket->wszToCharacterName);
			}
		}
		break;

	case VIMA_RETINVITEGUILDMEMBER:
		{
			g_pDivisionManager->SendInviteGuildMemberResult(reinterpret_cast<VIMAInviteGuildMemberResult*>(pData));
		}
		break;

	case VIMA_GUILDWAREINFO:
		{
			g_pDivisionManager->SendGuildWareInfo(reinterpret_cast<MAGuildWareInfo*>(pData));
		}
		break;

	case VIMA_RETGUILDWAREINFO:
		{
			g_pDivisionManager->SendGuildWareInfoResult(reinterpret_cast<MAGuildWareInfoResult*>(pData));
		}
		break;

	case VIMA_GUILDMEMBER_LEVELUP:
		{
			g_pDivisionManager->SendGuildMemberLevelUp(reinterpret_cast<MAGuildMemberLevelUp*>(pData));
		}
		break;

	case VIMA_REFRESH_GUILDITEM:
		{
			g_pDivisionManager->SendRefreshGuildItem(reinterpret_cast<MARefreshGuildItem*>(pData));
		}
		break;

	case VIMA_REFRESH_GUILDCOIN:
		{
			g_pDivisionManager->SendRefreshGuildCoin(reinterpret_cast<MARefreshGuildCoin*>(pData));
		}
		break;

	case VIMA_EXTEND_GUILDWARE:
		{
			g_pDivisionManager->SendExtendGuildWareSize(reinterpret_cast<MAExtendGuildWare*>(pData));
		}
		break;

	case VIMA_DISMISSGUILD:				// Guild disband notification
		{
			g_pDivisionManager->SendDismissGuild(reinterpret_cast<MADismissGuild*>(pData));
		}
		break;

	case VIMA_ADDGUILDMEMB:				// Notify add guild member
		{
			g_pDivisionManager->SendAddGuildMember(reinterpret_cast<MAAddGuildMember*>(pData));
		}
		break;

	case VIMA_DELGUILDMEMB:				// Guild member removal (leave/kick) notification
		{
			g_pDivisionManager->SendDelGuildMember(reinterpret_cast<MADelGuildMember*>(pData));
		}
		break;

	case VIMA_CHANGEGUILDINFO:			// Guild info change notification
		{
			g_pDivisionManager->SendChangeGuildInfo(reinterpret_cast<MAChangeGuildInfo*>(pData));
		}
		break;

	case VIMA_CHANGEGUILDMEMBINFO:		// Guild member info update notification
		{
			g_pDivisionManager->SendChangeGuildMemberInfo(reinterpret_cast<MAChangeGuildMemberInfo*>(pData));
		}
		break;

	case VIMA_GUILDCHAT:				// Guild chat
		{
			g_pDivisionManager->SendGuildChat(reinterpret_cast<MAGuildChat*>(pData));
		}
		break;
	case VIMA_GUILDCHANGENAME :
		{
			g_pDivisionManager->SendChangeGuildName(reinterpret_cast<MAGuildChangeName*>(pData));
		}
		break;
	case VIMA_GUILDCHANGEMARK :
		{
			g_pDivisionManager->SendChangeGuildMark(reinterpret_cast<MAGuildChangeMark*>(pData));
		}
		break;
	case VIMA_UPDATEGUILDEXP:
		{
			g_pDivisionManager->SendUpdateGuildExp(reinterpret_cast<MAUpdateGuildExp*>(pData));
		}
		break;
	case VIMA_ENROLLGUILDWAR:
		{
			g_pDivisionManager->SendEnrollGuildWar(reinterpret_cast<MAEnrollGuildWar*>(pData));
		}
		break;
	case VIMA_SETGUILDWAR_SCHEDULE :
		{
			VIMASetGuildWarSchedule * pPacket = (VIMASetGuildWarSchedule*)pData;
			if( pPacket->nResultCode != ERROR_NONE )
			{
				g_Log.Log(LogType::_FILEDBLOG, g_Config.nWorldSetID, 0, 0, 0, L"GetGuildWarSchedule Fail Errcode[%d]\n", pPacket->nResultCode);				
				break;
			}
			g_pGuildWarManager->LoadScheduleInfo(pPacket);			
		}
		break;
	case VIMA_SETGUILDWAR_FINALSCHEDULE :
		{
			MASetGuildWarFinalSchedule* pPacket = (MASetGuildWarFinalSchedule*)pData;			
			g_pGuildWarManager->LoadFinalScheduleInfo(pPacket);
		}
		break;
	case VIMA_SETGUILDWAR_POINT :
		{
			MASetGuildWarPoint* pPacket = (MASetGuildWarPoint*)pData;
			if( pPacket->nResultCode != ERROR_NONE )
			{
				g_Log.Log(LogType::_FILEDBLOG, g_Config.nWorldSetID, 0, 0, 0, L"GetGuildWarPoint Fail Errcode[%d]\n", pPacket->nResultCode);				
				break;
			}
			g_pGuildWarManager->SetGuildWarPoint(pPacket->nBlueTeamPoint, pPacket->nRedTeamPoint);
			g_pDivisionManager->SendSetGuildWarPoint(pPacket); // Notify all games and villages.			
		}
		break;
	case VIMA_ADDGUILDWAR_POINT :
		{
			MAAddGuildWarPoint* pPacket = (MAAddGuildWarPoint*)pData;			
			g_pGuildWarManager->AddGuildWarPoint(pPacket->cTeamType, pPacket->nAddPoint);
			g_pDivisionManager->SendAddGuildWarPoint(pPacket);
		}
		break;
	case VIMA_SETGUILDWAR_FINAL :
		{			
			VIMASetGuildWarFinalTeam* pPacket = (VIMASetGuildWarFinalTeam*)pData;

			// Check if tournament bracket is already generated
			if( g_pGuildWarManager->GetGuildTournamentInfo()->GuildUID.IsSet() )
			{
				//This is for setting total points.
				g_pGuildWarManager->SetGuildWarOpenningPoint(pPacket);
			}
			else // If bracket is not set up, generate the bracket.
			{
				// Check here if statistics haven't been collected yet.
				if( pPacket->nCount != 0 )
				{
					// Build tournament bracket here.
					g_pGuildWarManager->CalcGuildWarTournament(pPacket);
					// Notify to save tournament bracket
					SendSetGuildWarFinalTeam(g_pGuildWarManager->GetGuildTournamentInfo());					
				}
				else				
				{
					if( !g_pGuildWarManager->GetDBJobSuccess() )
					{
						// Run the statistics job here.
						SendAddDBJobSystemReserve(DBJOB_GUILDWAR_STATS);
						g_pGuildWarManager->SetDBJobSend(true);
						// This will basically happen once each..
						g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Not TrialStats(Get Final Team) and AddJobSystem\n");				
					}				
					else
					{
						// No guild participated in the preliminaries... nothing shows up even when pulling stats... what to do here...
						g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Not TrialStats(Get Final Team) Trial Guild Empty\n");				
					}
				}
			}			
		}
		break;
	case VIMA_SETGUILDWAR_TOURNAMENTINFO :
		{
			// Query finals result
			MAGuildWarTournamentInfo* pPacket = (MAGuildWarTournamentInfo*)pData;
			if( g_pGuildWarManager->SetGuildTournamentInfo(pPacket->sGuildWarFinalInfo) != 0 )
			{
				// Broadcast tournament bracket
				g_pDivisionManager->SendSetGuildWarTournamentInfo(g_pGuildWarManager->GetGuildTournamentInfo());
				// Notify to collect statistics..
				g_pDivisionManager->SendGetGuildWarTrialStats();
				g_pGuildWarManager->SetTrialStats();
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] SendGetGuildWarTrialStats and SetTrialStats !!\r\n");
			}
			// Tournament is not set up, or get advancing teams to obtain preliminary scores
			SendGetGuildWarFinalTeam();
		}
		break;
	case VIMA_MATCHLIST_SAVE_RESULT :
		{
			// Once bracket is saved, fetch tournament info once more.. for guild marks.
			VIMAMatchListSaveResult* pPacket = (VIMAMatchListSaveResult*)pData;
			if( pPacket->nResultCode == ERROR_NONE )			
				SendGetGuildWarTournamentInfo(g_pGuildWarManager->GetScheduleID());
			else
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] MatchList Save Error!\n");				
		}
		break;
	case VIMA_SETGUILDWAR_PRE_WIN_GUILD:
		{
			MAGuildWarPreWinGuild* pPacket = (MAGuildWarPreWinGuild*)pData;			
			g_pGuildWarManager->SetGuildWarPreWinGuild(pPacket->GuildUID);

			// Check whether gesture and farm were granted and send.
			g_pGuildWarManager->SetPreWinScheduleID(pPacket->wScheduleID);
			SendGetGuildWarPreWindGuildReward(pPacket->wScheduleID, pPacket->GuildUID.nDBID);	
#if defined(PRE_FIX_75807)
			if( g_pGuildWarManager->GetScheduleID() == GUILDWARSCHEDULE_DEF )
			{
				// Schedule is not set yet, so request schedule again
				g_pGuildWarManager->SetGuildWarSettingStep(REQ_NONE);
				g_pGuildWarManager->SendGuildWarInfoReq();
			}
#endif //#if defined(PRE_FIX_75807)
		}
		break;
	case VIMA_SETGUILDWAR_PRE_WIN_REWARD :
		{
			MAGuildWarPreWinGuild pPacket;
			memset(&pPacket, 0, sizeof(MAGuildWarPreWinGuild));
			pPacket.GuildUID = g_pGuildWarManager->GetPreWinGuild();
			pPacket.bPreWin = true;
			g_pGuildWarManager->SetFinalWinGuild(true);
			g_pDivisionManager->SendSetGuildWarPreWinGuild(&pPacket); // Notify all games and villages.
			g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Set PreWinGuildReward! GuildID:%d\n", pPacket.GuildUID.nDBID);
		}
		break;
	case VIMA_SETGUILDWAR_POINT_RUNNING_TOTAL :
		{
			MASetGuildWarPointRunningTotal* pPacket = (MASetGuildWarPointRunningTotal*)pData;
			g_pGuildWarManager->SetGuildWarPointTrialRanking(pPacket);
			g_pDivisionManager->SendSetGuildWarTrialRanking(pPacket); // Notify all villages.
		}
		break;
	case VIMA_GUILDWAR_REFRESH_GUILD_POINT :
		{
			MAGuildWarRefreshGuildPoint* pPacket = (MAGuildWarRefreshGuildPoint*)pData;
			g_pDivisionManager->SendGuildWarRefreshGuildPoint(pPacket);
		}
		break;
	case VIMA_GUILDWAR_SCHEDULE_RELOAD :
		{
			g_pGuildWarManager->SetResetSchedule(true);
		}
		break;
	case VIMA_SETGUILDWAR_PREWIN_SKILLCOOLTIME :
		{
			MAGuildWarPreWinSkillCoolTime* pPacket = (MAGuildWarPreWinSkillCoolTime*)pData;
			g_pGuildWarManager->SetPreWinSKillCoolTime(pPacket->dwSkillCoolTime);
			g_pDivisionManager->SendSetGuildWarPreWinSkillCoolTime(pPacket);
		}
		break;
	case VIMA_ADD_DBJOBSYSTEM_RESERVE :
		{
			VIMAAddDBJobSystemReserve* pPacket = (VIMAAddDBJobSystemReserve*)pData;
			if( pPacket->nResultCode == ERROR_NONE )
			{
				g_pGuildWarManager->SetDBJobSeq(pPacket->nJobSeq);
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] AddDBJobSystemReserve %d\n", pPacket->nJobSeq);
				g_pDivisionManager->SendResetGuildWarBuyedItemCount();
			}
			else
			{
				// Should I Send again? Not sure..
				if( g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_PREPARATION )
				{
					SendAddDBJobSystemReserve(DBJOB_GUILDWAR_INIT);
					g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Retry AddDBJobSystemReserve Init!!\n");
				}
				else
				{
					SendAddDBJobSystemReserve(DBJOB_GUILDWAR_STATS);
					g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Retry AddDBJobSystemReserve Stats!!\n");
				}
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] AddDBJobSystemReserve Fail!!\n");
			}
		}
		break;
	case VIMA_GET_DBJOBSYSTEM_RESERVE :  
		{
			VIMAGetDBJobSystemReserve* pPacket = (VIMAGetDBJobSystemReserve*)pData;
			if( pPacket->nResultCode == ERROR_NONE )
			{
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] GetDBJobSystemReserve %d\n", pPacket->cJobStatus);
				//0=Reserved, 1=Completed, 2=Processing, 3=Failed
				if( pPacket->cJobStatus == JOB_STATUS_COMPLETED) //Complete
				{
					g_pGuildWarManager->SetDBJobSuccess(true);
				}
				else if(pPacket->cJobStatus == JOB_STATUS_FAIL ) // Failed
				{
					//Job Add again?
					if( g_pGuildWarManager->GetStepIndex() == GUILDWAR_STEP_PREPARATION )
					{
						SendAddDBJobSystemReserve(DBJOB_GUILDWAR_INIT);
						g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Retry AddDBJobSystemReserve Init!!\n");
					}
					else
					{
						SendAddDBJobSystemReserve(DBJOB_GUILDWAR_STATS);
						g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] Retry AddDBJobSystemReserve Stats!!\n");
					}
				}
			}
			else
			{
				// Should I Send again? Not sure..
				g_Log.Log(LogType::_GUILDWAR, g_Config.nWorldSetID, 0, 0, 0, L"[GUILDWAR] AddDBJobSystemReserve Fail!!\n");
			}
		}
		break;
	case VIMA_GUILDRECRUIT_MEBMER:
		{
			g_pDivisionManager->SendGuildRecruitMember(reinterpret_cast<MAGuildRecruitMember*>(pData));
			break;
		}
	case VIMA_GUILD_ADD_REWARDITEM:
		{
			g_pDivisionManager->SendAddGuildRewardItem(reinterpret_cast<MAGuildRewardItem*>(pData));			
		}
		break;
	case VIMA_GUILD_EXTEND_GUILDSIZE:
		{
			g_pDivisionManager->SendExtendGuildSize(reinterpret_cast<MAExtendGuildSize*>(pData));			
		}
		break;
	case VIMA_CHANGECHARACTERNAME:
		{
			g_pDivisionManager->SendChangeCharacterName(reinterpret_cast<MAChangeCharacterName*>(pData));
		}
		break;

	case VIMA_FRIENDADD:
		{
			VIMAFriendAddNotice * pPacket = (VIMAFriendAddNotice*)pData;
			g_pDivisionManager->SendFriendAddNotice(pPacket->nAddedAccountDBID, pPacket->wszAddName);
		}
		break;

	case VIMA_PRIVATECHAT:
		{
			VIMAPrivateChat *pChat = (VIMAPrivateChat*)pData;

			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pChat->wszChatMsg, pChat->wChatLen);

#ifdef PRE_ADD_DOORS
			int nRet = g_pDivisionManager->PrivateChat(pChat->nAccountDBID, pChat->wszToCharacterName, pChat->cType, wszChatMsg, pChat->wChatLen, pChat->biDestCharacterDBID);
#else		//#ifdef PRE_ADD_DOORS
			int nRet = g_pDivisionManager->PrivateChat(pChat->nAccountDBID, pChat->wszToCharacterName, pChat->cType, wszChatMsg, pChat->wChatLen);
#endif		//#ifdef PRE_ADD_DOORS
			if (nRet != ERROR_NONE)
				SendPrivateChat(pChat->nAccountDBID, pChat->wszToCharacterName, pChat->cType, wszChatMsg, pChat->wChatLen, nRet);

			return ERROR_NONE;
		}
		break;

	case VIMA_CHAT:
		{
			VIMAChat *pChat = (VIMAChat*)pData;

			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pChat->wszChatMsg, pChat->wChatLen);

			switch (pChat->cType)
			{
				// Added because VIMA_CHAT packet didn't support CHATTYPE_GM! by Kimbap
			case CHATTYPE_GM:
				{
					CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pChat->nAccountDBID );
					if( pUser )
					{
						if( pUser->GetUserState() == STATE_VILLAGE )
						{
							CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
							if( pVillageCon )
								pVillageCon->SendChat( pChat->cType, pChat->nAccountDBID, L"", wszChatMsg, pChat->wChatLen );
						}
					}
					return ERROR_NONE;
				}

			case CHATTYPE_CHANNEL:
				{
					if (pChat->nMapIdx == -1)
					{
						_DANGER_POINT();
						return ERROR_NONE;
					}

					if (g_pDivisionManager->ZoneChat(pChat->nAccountDBID, wszChatMsg, pChat->wChatLen, pChat->nMapIdx) != ERROR_NONE)
						_DANGER_POINT();
					return ERROR_NONE;
				}
			case CHATTYPE_WORLD:
#if defined(PRE_ADD_WORLD_MSG_RED)
			case CHATTYPE_WORLD_POPMSG:
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)
				{
#if defined(PRE_ADD_WORLD_MSG_RED)
					if (g_pDivisionManager->WorldChat(pChat->nAccountDBID, wszChatMsg, pChat->wChatLen, pChat->cType) != ERROR_NONE)
#else	// #if defined(PRE_ADD_WORLD_MSG_RED)
					if (g_pDivisionManager->WorldChat(pChat->nAccountDBID, wszChatMsg, pChat->wChatLen) != ERROR_NONE)
#endif	// #if defined(PRE_ADD_WORLD_MSG_RED)
						_DANGER_POINT();
					return ERROR_NONE;
				}
				break;
			}
		}
		break;

	case VIMA_WORLDSYSTEMMSG:
		{
			VIMAWorldSystemMsg *pMsg = (VIMAWorldSystemMsg*)pData;
			g_pDivisionManager->WorldSystemMsg(pMsg->nAccountDBID, pMsg->cType, pMsg->nID, pMsg->nValue, pMsg->wszToCharacterName);
		}
		break;

#if defined( PRE_PRIVATECHAT_CHANNEL )
	case VIMA_PRIVATECHANNELCHAT:
		{
			VIMAPrivateChannelChatMsg *pChat = (VIMAPrivateChannelChatMsg*)pData;

			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pChat->wszChatMsg, pChat->wChatLen);

			if (g_pDivisionManager->PrivateChannelChat(pChat->nAccountDBID, wszChatMsg, pChat->wChatLen, pChat->nChannelID) != ERROR_NONE)
				return ERROR_NONE;
		}
		break;
#endif
		// VoiceChannel
	case VIMA_REQVOICECHANNELID:
		{
			VIMAReqVoiceChannelID * pPacket = (VIMAReqVoiceChannelID*)pData;

			//Depending on voice channel type, this may be extended so Master holds the information
			//e.g., guild channel; in this case it is bound to the world rather than a party or single server
			//Or when tracing users by voice channel, etc.; currently created instances simply move across servers
			UINT nVoiceChannelD[PvPCommon::TeamIndex::Max];
			memset(nVoiceChannelD, 0, sizeof(nVoiceChannelD));

			switch (pPacket->cVoiceChannelType)
			{
			case _VOICECHANNEL_REQTYPE_PARTY:
				{
					nVoiceChannelD[0] = g_IDGenerator.GetVoiceChannelID();
				}
				break;

			case _VOICECHANNEL_REQTYPE_PVP:
				{
					for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
						nVoiceChannelD[i] = g_IDGenerator.GetVoiceChannelID();

					CDNPvP * pPvP = g_pDivisionManager->GetPvPRoomByIdx(static_cast<UINT>(pPacket->InstanceID));
					if (pPvP)
						pPvP->SetVoiceChannelID(nVoiceChannelD);
					else
						_DANGER_POINT();
				}
				break;

			default:
				{
					_DANGER_POINT();
					return ERROR_NONE;
				}
			}					
			SendVoiceChannelID(pPacket->InstanceID, pPacket->cVoiceChannelType, pPacket->nPvPLobbyChannelID, nVoiceChannelD);
			return ERROR_NONE;
		}
		break;
		
	// PvP
	case VIMA_PVP_MOVELOBBYTOVILLAGE:
	{
		VIMAPVP_MOVELOBBYTOVILLAGE* pPacket = reinterpret_cast<VIMAPVP_MOVELOBBYTOVILLAGE*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->MovePvPLobbyToVillage( this, pPacket );
		
		return ERROR_NONE;
	}
	case VIMA_PVP_CREATEROOM:
	{
		VIMAPVP_CREATEROOM* pPacket = reinterpret_cast<VIMAPVP_CREATEROOM*>(pData);
		if( g_pDivisionManager )
		{
#if defined( PRE_ADD_PVP_VILLAGE_ACCESS)
			short nRetCode = ERROR_NONE;
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID(pPacket->uiCreateAccountDBID);
			if( pUser )
			{
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				if (pPacket->sCSPVP_CREATEROOM.cRoomType != static_cast<BYTE>(pUser->GetPvPChannelType()))
				{
					_DANGER_POINT();
					pPacket->sCSPVP_CREATEROOM.cRoomType = static_cast<BYTE>(pUser->GetPvPChannelType());
				}				
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
				if( !pUser->bIsPvPLobby() )
				{
					pPacket->unVillageChannelID = g_pDivisionManager->GetPvPLobbyChannelID();
					pPacket->uiVillageMapIndex = g_pDivisionManager->GetPvPLobbyMapIndex();
				}

				if( GetVillageID() != g_pDivisionManager->GetPvPLobbyVillageID() ) // It's troublesome if village is elsewhere..
				{
					CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
					if( pVillageCon == NULL || pVillageCon->GetActive() == false )
						nRetCode = ERROR_PVP_CREATEROOM_FAILED;
					else				
						nRetCode = g_pDivisionManager->CreatePvPRoom( pVillageCon, pPacket );				
				}
				else
					nRetCode = g_pDivisionManager->CreatePvPRoom( this, pPacket );
			}
			else
				nRetCode = g_pDivisionManager->CreatePvPRoom( this, pPacket );
#else
			short nRetCode = g_pDivisionManager->CreatePvPRoom( this, pPacket );
#endif //#if defined( PRE_ADD_PVP_VILLAGE_ACCESS)

			if( nRetCode != ERROR_NONE && pPacket->nEventID <= 0 )
				SendPvPCreateRoom( nRetCode, NULL, pPacket );
		}
		else
		{
			SendPvPCreateRoom( ERROR_PVP_CREATEROOM_FAILED, NULL, pPacket );
		}
		break;
	}
	case VIMA_PVP_MODIFYROOM:
	{
		VIMAPVP_MODIFYROOM* pPacket = reinterpret_cast<VIMAPVP_MODIFYROOM*>(pData);
		if( g_pDivisionManager )
		{
			short nRetCode = g_pDivisionManager->ModifyPvPRoom( this, pPacket );
			if( nRetCode != ERROR_NONE )
				SendPvPModifyRoom( nRetCode, NULL, pPacket );
		}
		else
		{
			SendPvPModifyRoom( ERROR_PVP_MODIFYROOM_FAILED, NULL, pPacket );
		}
		break;
	}
	case VIMA_PVP_LEAVEROOM:
	{
		VIMAPVP_LEAVEROOM* pPacket = reinterpret_cast<VIMAPVP_LEAVEROOM*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->LeavePvPRoom( pPacket->uiLeaveAccountDBID, true );
		break;
	}
	case VIMA_PVP_CHANGECAPTAIN:
	{
		VIMAPVP_CHANGECAPTAIN* pPacket = reinterpret_cast<VIMAPVP_CHANGECAPTAIN*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->ChangePvPRoomCaptain( this, pPacket );
		break;
	}
	case VIMA_PVP_ROOMLIST:
	{
		VIMAPVP_ROOMLIST* pPacket = reinterpret_cast<VIMAPVP_ROOMLIST*>(pData);
		if( g_pDivisionManager )
		{
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( pUser )
			{
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
				if( g_pDivisionManager->GetPvPLobbyVillageID() == GetVillageID() ) // If I am Colosseum Village, just send the list immediately.			
				{
					if( !pUser->bIsPvPLobby() )
						pPacket->unVillageChannelID = g_pDivisionManager->GetPvPLobbyChannelID();
					SendPvPRoomList( pPacket );
				}
				else
				{
					// Otherwise request relay					
					CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( g_pDivisionManager->GetPvPLobbyVillageID() );
					if( pVillageConnection == NULL || pVillageConnection->GetActive() == false )
						break;
					pPacket->unVillageChannelID = g_pDivisionManager->GetPvPLobbyChannelID();
					pVillageConnection->SendPvPRoomListRelay(GetVillageID(), pPacket);
				}
#else
				// In PvP lobby and not joined a room
				if( pUser->bIsPvPLobby() && pUser->GetPvPIndex() == 0 )
					SendPvPRoomList( pPacket );
#endif // #if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
			}
		}
		break;
	}
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
	case VIMA_PVP_ROOMLIST_RELAY :
		{
			PVP_ROOMLIST_RELAY* pPacket = reinterpret_cast<PVP_ROOMLIST_RELAY*>(pData);
			CDNVillageConnection* pVillageConnection = g_pDivisionManager->GetVillageConnectionByVillageID( pPacket->cVillageID );
			if( pVillageConnection == NULL || pVillageConnection->GetActive() == false )
				break;
			pVillageConnection->SendPvPRoomListRelayAck(pPacket);
		}
		break;
#endif

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	case VIMA_PVP_CHANGECHANNEL:
		{
			if (g_pDivisionManager)
				g_pDivisionManager->SetPvPChangeChannel(this, reinterpret_cast<const PVP_CHANGECHANNEL*>(pData));
		}
		break;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

	case VIMA_PVP_WAITUSERLIST:
	{
		VIMAPVP_WAITUSERLIST* pPacket = reinterpret_cast<VIMAPVP_WAITUSERLIST*>(pData);
		if( g_pDivisionManager )
		{
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( pUser )
			{
				// In PvP lobby and not joined a room
				if( pUser->bIsPvPLobby() && pUser->GetPvPIndex() == 0 )
					SendPvPWaitUserList( pPacket );
					
			}
		}
		break;
	}
	case VIMA_PVP_JOINROOM:
	{
		VIMAPVP_JOINROOM* pPacket = reinterpret_cast<VIMAPVP_JOINROOM*>(pData);
		if( g_pDivisionManager )
		{
			short nRetCode = g_pDivisionManager->JoinPvPRoom( this, pPacket );
			if( nRetCode != ERROR_NONE )
				SendPvPJoinRoom( nRetCode, pPacket->uiAccountDBID );
		}
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_READY:
	{
		VIMAPVP_READY* pPacket = reinterpret_cast<VIMAPVP_READY*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->ReadyPvP( this, pPacket );
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_START:
	{
		VIMAPVP_START* pPacket = reinterpret_cast<VIMAPVP_START*>(pData);
		if( g_pDivisionManager )
		{
#if defined(_FINAL_BUILD)
			short nRetCode = g_pDivisionManager->StartPvP( this, pPacket, true );
#else
			short nRetCode = g_pDivisionManager->StartPvP( this, pPacket );
#endif
			if( nRetCode != ERROR_NONE )
			{
				CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
				if( !pUser )
				{
					SendPvPStart( ERROR_PVP_STARTPVP_FAILED, 0, 0, pPacket->uiAccountDBID );
					break;
				}
				CDNPvP*		pPvPRoom			= g_pDivisionManager->GetPvPRoomByIdx( pUser->GetPvPIndex() );
				BYTE		cVillageChannelID	= pPvPRoom ? pPvPRoom->GetVillageChannelID() : 0;
				UINT		uiPvPIndex			= pPvPRoom ? pPvPRoom->GetIndex() : 0;
				SendPvPStart( nRetCode, cVillageChannelID, uiPvPIndex, pPacket->uiAccountDBID );
			}
		}
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_RANDOMJOINROOM:
	{
		VIMAPVP_RANDOMJOINROOM* pPacket = reinterpret_cast<VIMAPVP_RANDOMJOINROOM*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->RandomJoinPvPRoom( this, pPacket );
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_CHANGETEAM:
	{
		VIMAPVP_CHANGETEAM* pPacket = reinterpret_cast<VIMAPVP_CHANGETEAM*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->ChangePvPTeam( this, pPacket );
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_BAN:
	{
		if( g_pDivisionManager )
			g_pDivisionManager->BanPvPRoom( this, reinterpret_cast<VIMAPVP_BAN*>(pData) );
		else
			_DANGER_POINT();
		break;
	}
	case VIMA_PVP_ENTERLOBBY:
	{
		VIMAPVP_ENTERLOBBY* pPacket = reinterpret_cast<VIMAPVP_ENTERLOBBY*>(pData);
		if( g_pDivisionManager )
			g_pDivisionManager->EnterLobby( this, pPacket );
		else
			_DANGER_POINT();
		break;
	}
	case  VIMA_PVP_FATIGUE_OPTION :
		{
			VIMAPVP_FATIGUE_OPTION* pPacket = reinterpret_cast<VIMAPVP_FATIGUE_OPTION*>(pData);
			if( g_pDivisionManager )
				g_pDivisionManager->SetPvPFatigueOption(pPacket);
			else
				_DANGER_POINT();
			break;
		}
	case VIMA_PVP_SWAPTMEMBER_INDEX:
		{
			VIMAPVPSwapMemberIndex * pPacket = (VIMAPVPSwapMemberIndex*)pData;

			bool bRet = false;
			USHORT nTeam = PvPCommon::Team::A;
			if (g_pDivisionManager)
				bRet = g_pDivisionManager->SetPvPMemberIndex(pPacket->nAccountDBID, pPacket->cCount, pPacket->Index, nTeam);

			SendPvPMemberIndex(nTeam, pPacket, bRet ? ERROR_NONE : ERROR_PVP_SWAPTEAM_MEMBERINDEX_FAIL);
			break;
		}

	case VIMA_PVP_CHANGEMEMBER_GRADE:
		{
			VIMAPVPChangeMemberGrade * pPacket = (VIMAPVPChangeMemberGrade*)pData;

			bool bRet = false;
			USHORT nUserState = PvPCommon::UserState::None;
			USHORT nTeam = PvPCommon::Team::A;
			if (g_pDivisionManager)
				bRet = g_pDivisionManager->SetGuildWarMemberGrade(pPacket->nAccountDBID, pPacket->bAsign, pPacket->nType, pPacket->nTargetSessionID, nUserState, nTeam);

			SendPvPMemberGrade(pPacket->nAccountDBID, nTeam, nUserState, pPacket->nTargetSessionID, bRet ? ERROR_NONE : ERROR_PVP_CHANGEMEMBERGRADE_FAIL);
			break;
		}
#if defined(PRE_ADD_PVP_TOURNAMENT)
	case VIMA_PVP_SWAP_TOURNAMENT_INDEX :
		{
			VIMAPVPSwapTournamentIndex* pPacket = reinterpret_cast<VIMAPVPSwapTournamentIndex*>(pData);			
			if( g_pDivisionManager )
			{
				if( g_pDivisionManager->SwapTournamentIndex(pPacket->nAccountDBID, pPacket->cSourceIndex, pPacket->cDestIndex) == true)
					SendPvPSwapTournamentIndex(ERROR_NONE, pPacket->nAccountDBID, pPacket->cSourceIndex, pPacket->cDestIndex);
				else
					SendPvPSwapTournamentIndex(ERROR_PVP_SWAP_TOURNAMENT_INDEX_FAIL, pPacket->nAccountDBID, 0, 0);
			}
			else
				_DANGER_POINT();
			break;
		}
#endif
	case VIMA_MASTERSYSTEM_CHECKMASTERAPPLICATION:
	{
		MasterSystem::VIMACheckMasterApplication* pPacket = reinterpret_cast<MasterSystem::VIMACheckMasterApplication*>(pData);

		CDNUser* pMasterUser	= g_pDivisionManager->GetUserByCharacterDBID( pPacket->biMasterCharacterDBID );
		CDNUser* pPupilUser		= g_pDivisionManager->GetUserByCharacterDBID( pPacket->biPupilCharacterDBID );

		if( pPupilUser == NULL )
		{
			_DANGER_POINT();
			break;
		}

		int iRet = ERROR_MASTERSYSTEM_MASTERUSER_OFFLINE;
		if( pMasterUser )
		{
			switch( pMasterUser->GetUserState() )
			{
				case STATE_CHECKVILLAGE:
				case STATE_CHECKGAME:
				case STATE_VILLAGE:
				case STATE_GAME:
				{
					iRet = ERROR_NONE;
					break;
				}
			}
		}
		SendMasterSystemCheckMasterApplication( iRet, pPupilUser->GetAccountDBID(), pPacket->biMasterCharacterDBID );
		break;
	}
	case VIMA_MASTERSYSTEM_SYNC_SIMPLEINFO:
	{
		g_pDivisionManager->SyncMasterSystemSimpleInfo( reinterpret_cast<MasterSystem::VIMASyncSimpleInfo*>(pData) );
		break;
	}
	case VIMA_MASTERSYSTEM_SYNC_JOIN:
	{
		MasterSystem::VIMASyncJoin* pPacket = reinterpret_cast<MasterSystem::VIMASyncJoin*>(pData);

		CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biCharacterDBID );
		if( pUser )
		{
			switch( pUser->GetUserState() )
			{
				case STATE_VILLAGE:
				{
					CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
					if( pVillageCon )
						pVillageCon->SendMasterSystemSyncJoin( pUser->GetAccountDBID(), pPacket->biCharacterDBID, pPacket->bIsAddPupil );
					break;
				}
				case STATE_GAME:
				{
					CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pUser->GetGameID() );
					if( pGameCon )
						pGameCon->SendMasterSystemSyncJoin( pUser->GetAccountDBID(), pPacket->biCharacterDBID, pPacket->bIsAddPupil );
					break;
				}
			}
		}
		break;
	}
	case VIMA_MASTERSYSTEM_SYNC_LEAVE:
	{
		MasterSystem::VIMASyncLeave* pPacket = reinterpret_cast<MasterSystem::VIMASyncLeave*>(pData);

		CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biCharacterDBID );
		if( pUser )
		{
			switch( pUser->GetUserState() )
			{
				case STATE_VILLAGE:
				{
					CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
					if( pVillageCon )
						pVillageCon->SendMasterSystemSyncLeave( pUser->GetAccountDBID(), pPacket->biCharacterDBID, pPacket->bIsDelPupil );
					break;
				}
				case STATE_GAME:
				{
					CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pUser->GetGameID() );
					if( pGameCon )
						pGameCon->SendMasterSystemSyncJoin( pUser->GetAccountDBID(), pPacket->biCharacterDBID, pPacket->bIsDelPupil );
					break;
				}
			}
		}
		break;
	}
	case VIMA_MASTERSYSTEM_SYNC_GRADUATE:
	{
		g_pDivisionManager->SyncMasterSystemGraduate( reinterpret_cast<MasterSystem::VIMASyncGraduate*>(pData) );
		break;
	}
	case VIMA_MASTERSYSTEM_SYNC_CONNECT:
	{
		g_pDivisionManager->SyncMasterSystemConnect( reinterpret_cast<MasterSystem::VIMASyncConnect*>(pData) );
		break;
	}
	case VIMA_MASTERSYSTEM_CHECK_LEAVE:
	{
		MasterSystem::VIMACheckLeave* pPacket = reinterpret_cast<MasterSystem::VIMACheckLeave*>(pData);

		int iRet = ERROR_NONE;
		if( pPacket->bIsMaster )
		{
			CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biDestCharacterDBID );
			if( pUser )
			{
				if( pUser->GetUserState() != STATE_VILLAGE )
					iRet = ERROR_MASTERSYSTEM_LEAVE_DESTUSER_CANTSTATE;
			}
			else
			{
				iRet = ERROR_MASTERSYSTEM_LEAVE_DESTUSER_OFFLINE;
			}
		}

		SendMasterSystemCheckLeave( iRet, pPacket->uiAccountDBID, pPacket->biDestCharacterDBID, pPacket->bIsMaster );
		break;
	}
	case VIMA_MASTERSYSTEM_JOIN_CONFIRM :
	{
		MasterSystem::VIMAJoinConfirm* pPacket = reinterpret_cast<MasterSystem::VIMAJoinConfirm*>(pData);
		int iRet = ERROR_MASTERSYSTEM_CANT_JOINSTATE;
		CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biMasterCharacterDBID );
		if( pUser )
		{
			if( pUser->GetUserState() == STATE_VILLAGE )		
			{
				CDNVillageConnection *pVillageCon = pUser->GetCurrentVillageConnection();
				if( pVillageCon )					
				{
					pVillageCon->SendMasterSystemJoinConfirm(pUser->GetAccountDBID(), pPacket->cLevel, pPacket->cJob, pPacket->wszPupilCharName);
					break;
				}
			}
		}
		SendMasterSystemJoinConfirmResult(iRet, false, pPacket->biMasterCharacterDBID, pPacket->wszPupilCharName);
		break;
	}
	case VIMA_MASTERSYSTEM_JOIN_CONFIRM_RESULT :
	{
		MasterSystem::VIMAJoinConfirmResult* pPacket = reinterpret_cast<MasterSystem::VIMAJoinConfirmResult*>(pData);		

		CDNUser* pUser = g_pDivisionManager->GetUserByName( pPacket->wszPupilCharName );
		if( pUser )
		{	
			if( pUser->GetUserState() == STATE_VILLAGE )
			{
				CDNVillageConnection *pVillageCon = pUser->GetCurrentVillageConnection();
				if( pVillageCon )					
				{
					pVillageCon->SendMasterSystemJoinConfirmResult(pPacket->iRet, pPacket->bIsAccept, pPacket->biMasterCharacterDBID, pPacket->wszPupilCharName);		
				}
			}			
		}		
		break;
	}
	case VIMA_MASTERSYSTEM_RECALL_MASTER:
	{
		MasterSystem::VIMARecallMaster* pPacket = reinterpret_cast<MasterSystem::VIMARecallMaster*>(pData);

		int iRet = ERROR_MASTERSYSTEM_MASTERUSER_OFFLINE;  
		CDNUser* pMasterUser = g_pDivisionManager->GetUserByName( pPacket->wszMasterCharName );
		if( pMasterUser )
		{
			if( pMasterUser->GetUserState() == STATE_VILLAGE )
			{
				bool bCheck = false;
				for( UINT i=0 ; i<pPacket->cCharacterDBIDCount ; ++i )
				{
					if( pMasterUser->GetCharacterDBID() == pPacket->CharacterDBIDList[i] )
					{
						bCheck = true;
						break;
					}
				}

				if( bCheck == true )
					iRet = ERROR_NONE;
				else 
					iRet = ERROR_MASTERSYSTEM_RECALL_FAILED;
			}
			else
				iRet = ERROR_MASTERSYSTEM_RECALL_CANTSTATUS;
		}

		SendMasterSystemRecallMaster( iRet, pPacket->uiPupilAccountDBID, pPacket->wszMasterCharName, false );
		if( iRet == ERROR_NONE )
		{
			CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pMasterUser->GetVillageID() );
			if( pVillageCon )
				pVillageCon->SendMasterSystemRecallMaster( ERROR_NONE, pMasterUser->GetAccountDBID(), pPacket->wszPupilCharName, true );
		}
		break;
	}
	case VIMA_MASTERSYSTEM_BREAKINTO:
	{
		MasterSystem::VIMABreakInto* pPacket = reinterpret_cast<MasterSystem::VIMABreakInto*>(pData);

		CDNUser* pMasterUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiMasterAccountDBID );
		if( pMasterUser == NULL )
			break;
		CDNUser* pPupilUser = g_pDivisionManager->GetUserByName( pPacket->wszPupilCharName );
		if( pPupilUser == NULL )
			break;

		// Check master-apprentice relationship
		bool bCheck = false;
		for( UINT i=0 ; i<pPacket->cCharacterDBIDCount ; ++i )
		{
			if( pPupilUser->GetCharacterDBID() == pPacket->CharacterDBIDList[i] )
			{
				bCheck = true;
				break;
			}
		}

		if( bCheck == false )
		{
			_DANGER_POINT();
			break;
		}

		if( pPacket->iRet != ERROR_NONE )
		{
			CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pPupilUser->GetVillageID() );
			if( pVillageCon )
				pVillageCon->SendMasterSystemRecallMaster( pPacket->iRet, pPupilUser->GetAccountDBID(), pMasterUser->GetCharacterName(), false );
		}
		else
		{
			if( pPupilUser->GetUserState() == STATE_VILLAGE )
			{
				const TChannelInfo* pChannelInfo = g_pDivisionManager->GetChannelInfo( pPupilUser->GetVillageID(), pPupilUser->GetChannelID() );
				if( pChannelInfo == NULL )
				{
					_DANGER_POINT();
					break;
				}

				// Cannot go to PvP lobby
				if( pChannelInfo->nAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY )
					break;

				SendVillageTrace( pPupilUser, pMasterUser->GetAccountDBID(), false, 0 );
				break;
			}
		}

		break;
	}
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case VIMA_SYNC_SYSTEMMAIL:
	{
		VIMASyncSystemMail* pPacket = reinterpret_cast<VIMASyncSystemMail*>(pData);
		g_pDivisionManager->SyncSystemMail( pPacket );
		break;
	}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	case VIMA_NOTICE:
		{
			VIMANotice * pPacket = (VIMANotice*)pData;

			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pPacket->szMsg, pPacket->nLen);

			g_pDivisionManager->Notice(wszChatMsg, pPacket->nLen, 0);
			return ERROR_NONE;
		}
		break;

	case VIMA_ZONENOTICE:
		{
			VIMAZoneNotice * pPacket = (VIMAZoneNotice*)pData;

			WCHAR wszChatMsg[CHATLENMAX] = { 0, };
			_wcscpy(wszChatMsg, CHATLENMAX, pPacket->szMsg, pPacket->nLen);

			g_pDivisionManager->NoticeZone(pPacket->nMapIndex, wszChatMsg, pPacket->nLen, 0);
			return ERROR_NONE;
		}
		break;

	case VIMA_NOTIFYMAIL:
		{
			VIMANotifyMail *pMail = (VIMANotifyMail*)pData;
			g_pDivisionManager->SendNotifyMail(pMail->nToAccountDBID, pMail->biToCharacterDBID, pMail->wTotalMailCount, pMail->wNotReadMailCount, pMail->w7DaysLeftMailCount, pMail->bNewMail);
		}
		break;

	case VIMA_NOTIFYMARKET:
		{
			VIMANotifyMarket *pMarket = (VIMANotifyMarket*)pData;
			g_pDivisionManager->SendNotifyMarket(pMarket->nSellerAccountDBID, pMarket->biSellerCharacterDBID, pMarket->nItemID, pMarket->wCalculationCount);
		}
		break;

	case VIMA_NOTIFYGIFT:
		{
			VIMANotifyGift *pGift = (VIMANotifyGift*)pData;
			g_pDivisionManager->SendNotifyGift(pGift->nToAccountDBID, pGift->biToCharacterDBID, pGift->bNew, pGift->nGiftCount);
		}
		break;

	case VIMA_TRACEBREAKINTO:
		{
			VIMATraceBreakInto* pPacket = reinterpret_cast<VIMATraceBreakInto*>(pData);

			CDNUser* pTargetUser = NULL;
			switch (pPacket->nType)
			{
			case TRACE_CHARACTERNAME:
				{
					pTargetUser = g_pDivisionManager->GetUserByName( pPacket->wszParam );
					break;
				}

			case TRACE_ACCOUNTDBID:
				{
					pTargetUser = g_pDivisionManager->GetUserByAccountDBID( _wtoi(pPacket->wszParam) );
					break;
				}

			case TRACE_CHRACTERDBID:
				{
					pTargetUser = g_pDivisionManager->GetUserByCharacterDBID( _wtoi64(pPacket->wszParam) );
					break;
				}
			}

			if( !pTargetUser )
			{
				//std::wstring wString = boost::io::str( boost::wformat( L"Cannot find %s.") % pPacket->wszCharacterName );
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4418) ) % pPacket->wszParam );
				SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				return ERROR_NONE;
			}

			if( pTargetUser->GetUserState() == STATE_VILLAGE )
			{
				const TChannelInfo* pChannelInfo = g_pDivisionManager->GetChannelInfo( pTargetUser->GetVillageID(), pTargetUser->GetChannelID() );
				do 
				{
					if( !pChannelInfo )
						break;

					if( pChannelInfo->nAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY )
					{
						//std::wstring wString = boost::io::str( boost::wformat( L"Cannot track %s because they are in the Colosseum lobby.") % pPacket->wszCharacterName );
						std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4419) ) % pPacket->wszParam );
						SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
						return ERROR_NONE;
					}

					SendVillageTrace( pTargetUser, pPacket->uiAccountDBID, true, 0 );
					return ERROR_NONE;
				}while( false );

				//std::wstring wString = boost::io::str( boost::wformat( L"Failed to track %s.") % pPacket->wszCharacterName );
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4420) ) % pPacket->wszParam );
				SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				return ERROR_NONE;
			}

			if( pTargetUser->GetUserState() != STATE_GAME )
			{
				//std::wstring wString = boost::io::str( boost::wformat( L"Cannot track-join %s because they are not in a dungeon.") % pPacket->wszCharacterName );
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4421) ) % pPacket->wszParam );
				SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				return ERROR_NONE;
			}

			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( !pUser )
				return ERROR_GENERIC_INVALIDREQUEST;

			// GM drop-in into a PvP room enters as an observer.
			if( pTargetUser->GetPvPIndex() > 0 )
			{
				CDNPvP* pPvP = g_pDivisionManager->GetPvPRoomByIdx( pTargetUser->GetPvPIndex() );
				if( !pPvP )
					return ERROR_GENERIC_INVALIDREQUEST;

				SendPvPRoomInfo( pUser, pPvP );
				pUser->SetPvPTeam( PvPCommon::Team::Observer );
			}

			CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pTargetUser->GetGameID() );
			if( pGameCon )
			{
				pGameCon->SendBreakintoRoom( pTargetUser->m_nRoomID, pUser, BreakInto::Type::GMTrace );
			}

			return ERROR_NONE;
		}

		case VIMA_REQRECALL:
		{
			VIMAReqRecall* pPacket = reinterpret_cast<VIMAReqRecall*>(pData);


			CDNUser* pRecallUser = g_pDivisionManager->GetUserByName( pPacket->wszCharacterName );
			if( !pRecallUser )
			{
				//std::wstring wString = boost::io::str( boost::wformat( L"Cannot find %s.") % pPacket->wszCharacterName );
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4418) ) % pPacket->wszCharacterName );
				SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				return ERROR_NONE;
			}

			eUserState eTargetUserState = pRecallUser->GetUserState();
			switch( pRecallUser->GetUserState() )
			{
				case STATE_VILLAGE:
				{
					if( pRecallUser->GetPvPIndex() > 0 )
					{
						//std::wstring wString = boost::io::str( boost::wformat( L"Cannot recall %s because they are in a PvP room.") % pPacket->wszCharacterName );
						std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4423) ) % pPacket->wszCharacterName );
						SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
						return ERROR_NONE;
					}

					CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pRecallUser->GetVillageID() );
					if( pVillageCon )
					{
						pVillageCon->SendResRecall( pRecallUser, pPacket );
						return ERROR_NONE;
					}						
					break;
				}
				case STATE_GAME:
				{
					if( pRecallUser->GetPvPIndex() > 0 )
					{
						//std::wstring wString = boost::io::str( boost::wformat( L"Cannot recall %s because they are in a PvP game room.") % pPacket->wszCharacterName );
						std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4423) ) % pPacket->wszCharacterName );
						SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
						return ERROR_NONE;
					}

					const TChannelInfo* pChannelInfo = g_pDivisionManager->GetChannelInfo( pPacket->uiRecallChannelID );
					if( !pChannelInfo )
						break;

					CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID( pRecallUser->GetGameID() );
					if( pGameCon )
					{
						pGameCon->SendResRecall( pRecallUser, pPacket, pChannelInfo->nMapIdx );
						return ERROR_NONE;
					}
					break;
				}
				default:
				{
					//std::wstring wString = boost::io::str( boost::wformat( L"%s is in a State(%d) that cannot be recalled.") % pPacket->wszCharacterName % static_cast<int>(pRecallUser->GetUserState()) );
					std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4417) ) % pPacket->wszCharacterName % static_cast<int>(pRecallUser->GetUserState()) );
					SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
					return ERROR_NONE;
				}
			}

			//std::wstring wString = boost::io::str( boost::wformat( L"Failed to recall %s. If this message persists, please contact a developer.") % pPacket->wszCharacterName );
			std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4425) ) % pPacket->wszCharacterName );
			SendChat( CHATTYPE_GM, pPacket->uiAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
			return ERROR_NONE;
		}

		case VIMA_BANUSER:
		{
			VIMABanUser * pPacket = (VIMABanUser*)pData;

			CDNUser * pUser = g_pDivisionManager->GetUserByName(pPacket->wszCharacterName);
			if(!pUser)
			{
				//std::wstring wString = boost::io::str( boost::wformat( L"Cannot find %s.") % pPacket->wszCharacterName );
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4418) ) % pPacket->wszCharacterName );
				SendChat( CHATTYPE_GM, pPacket->nAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				return ERROR_NONE;
			}

			g_pDivisionManager->SendDetachUser(pUser->GetAccountDBID());
			return ERROR_NONE;
		}
		case VIMA_DESTROYPVP:
		{
			VIMADestroyPvP * pPacket = (VIMADestroyPvP*)pData;

			bool bRet = false;
			if (g_pDivisionManager)
				bRet = g_pDivisionManager->ForceStopPvP(pPacket->nPvPIndex, pPacket->wszGuildName);
			
			if (bRet == false)
			{
				//Notification handling
				std::wstring wString = boost::io::str( boost::wformat( L"[%d] Failed to destroy PvPRoom.") % pPacket->nPvPIndex );
				SendChat( CHATTYPE_GM, pPacket->nAccountDBID, L"", wString.c_str(), static_cast<short>(wString.size()) );
			}
			return ERROR_NONE;
		}
		case VIMA_UPDATECHANNELSHOWINFO:
		{
			VIMAUpdateChannelShowInfo* pPacket = reinterpret_cast<VIMAUpdateChannelShowInfo*>(pData);

			TChannelInfo* pChannelInfo = const_cast<TChannelInfo*>(g_pDivisionManager->GetChannelInfo( pPacket->unChannelID ));
			if( pChannelInfo )
			{
				pChannelInfo->bShow = pPacket->bShow;
				pChannelInfo->nServerID = pPacket->nServerID;
				pChannelInfo->cThreadID = pPacket->cThreadID;
			}
			
			g_pDivisionManager->SendVillageChannelShowInfo( pPacket->unChannelID, pPacket->bShow );
			return ERROR_NONE;
		}

	case VIMA_PCBANGRESULT:
		{
			VIMAPCBangResult *pResult = (VIMAPCBangResult*)pData;

			CDNUser *pUser = g_pDivisionManager->GetUserByAccountDBID(pResult->nAccountDBID);
			if (!pUser)
			{
				SendPCBangResult(pResult->nAccountDBID, NULL);
				return ERROR_NONE;
			}

			SendPCBangResult(pResult->nAccountDBID, pUser);
		}
		break;

	case VIMA_FARMLIST:
		{
			VIMAFarmList * pPacket = (VIMAFarmList*)pData;

			//Need to create farm data here.
#ifdef PRE_MOD_OPERATINGFARM
			for (int i = 0; i < pPacket->cCount; i++)			
				g_pDivisionManager->CreateFarm(pPacket->Farms[i], 0);
#else		//#ifdef PRE_MOD_OPERATINGFARM
			for (int i = 0; i < pPacket->cCount; i++)
				g_pDivisionManager->CreateFarm(pPacket->Farms[i]);
#endif		//#ifdef PRE_MOD_OPERATINGFARM			
		}
		break;

	case VIMA_SAVE_USERTEMPDATA:
		{
			VIMASaveUserTempData * pPacket = (VIMASaveUserTempData*)pData;
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( pUser )
			{
				pUser->SetDungeonClearCount(pPacket->nDungeonClearCount);
#if defined(PRE_ADD_REMOTE_QUEST)
				pUser->SetAcceptWaitRemoteQuest(pPacket->nAcceptWaitRemoteQuestCount, pPacket->AcceptWaitRemoteQuestList);
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
				pUser->SetUserGameQuitRewardType(pPacket->eUserGameQuitRewardType);
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
			}
		}
		break;

	case VIMA_LOAD_USERTEMPDATA:
		{
			VIMALoadUserTempData * pPacket = (VIMALoadUserTempData*)pData;
			CDNUser* pUser = g_pDivisionManager->GetUserByAccountDBID( pPacket->uiAccountDBID );
			if( pUser )
			{
#if defined(PRE_ADD_REMOTE_QUEST)
				SendUserTempDataResult(pPacket->uiAccountDBID, pUser);
#else
#if defined(PRE_ADD_GAMEQUIT_REWARD)
				SendUserTempDataResult(pPacket->uiAccountDBID, pUser->GetDungeonClearCount(), pUser->GetUserGameQuitRewardType());
#else	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
				SendUserTempDataResult(pPacket->uiAccountDBID, pUser->GetDungeonClearCount());
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#endif	//#if defined(PRE_ADD_REMOTE_QUEST)
			}
		}
		break;		

	case VIMA_DUPLICATE_LOGIN :
		{
			VIMADuplicateLogin*  pPacket = (VIMADuplicateLogin*)pData;			
			// On failure, notify Login to call ResetAuth
			if( !pPacket->bIsDetach )
			{
				CDNLoginConnection* pLoginConnection = g_pDivisionManager->GetFirstEnableLoginServer();
				if( pLoginConnection )				
				{
					pLoginConnection->SendDuplicateLogin(pPacket->nAccountDBID, pPacket->nSessionID);
					g_Log.Log(LogType::_FILEDBLOG, g_Config.nWorldSetID, pPacket->nAccountDBID, 0, pPacket->nSessionID, L"[ADBID:%u] VIMA_DUPLICATE_LOGIN \r\n", pPacket->nAccountDBID);
				}
			}
		}
		break;
	case VIMA_UPPDATE_WORLDEVENTCOUNTER:
		{
			MAUpdateWorldEventCounter * pPacket = (MAUpdateWorldEventCounter*)pData;
			g_pDivisionManager->SendUpdateWorldEventCounter (pPacket);
		}
		break;
		case VIMA_CHECK_LASTDUNGEONINFO:
		{
			VIMACheckLastDungeonInfo* pPacket = reinterpret_cast<VIMACheckLastDungeonInfo*>(pData);
			CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByManagedID( pPacket->iManagedID );
			if( pGameCon )
			{
				pGameCon->SendCheckLastDungeonInfo( g_Config.nWorldSetID, pPacket );
			}
			else
			{
				SendCheckLastDungeonInfo( pPacket->uiAccountDBID, pPacket->biCharacterDBID, false, NULL );
			}
			break;
		}
		case VIMA_CONFIRM_LASTDUNGEONINFO:
		{
			VIMAConfirmLastDungeonInfo* pPacket = reinterpret_cast<VIMAConfirmLastDungeonInfo*>(pData);
			CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByManagedID( pPacket->iManagedID );

			CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biCharacterDBID );
			if( pUser == NULL )
				break;

#if defined( PRE_PARTY_DB )
			switch( pPacket->BreakIntoType )
			{
				case BreakInto::Type::PartyRestore:
				{
					if( pPacket->bIsOK == false )
					{
						if( pGameCon )
							pGameCon->SendDeleteBackupDungeonInfo( pPacket->uiAccountDBID, pPacket->biCharacterDBID, pPacket->iRoomID );
						break;
					}
					if( pGameCon )
					{
						pGameCon->SendBreakintoRoom(pPacket->iRoomID, pUser, pPacket->BreakIntoType );
					}
					else
					{
						SendConfirmLastDungeonInfo( ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID, pPacket->BreakIntoType );
					}
					break;
				}
				case BreakInto::Type::PartyJoin:
				{
					if( pGameCon )
					{
						pGameCon->SendBreakintoRoom(pPacket->iRoomID, pUser, pPacket->BreakIntoType );
					}
					else
					{
						SendConfirmLastDungeonInfo( ERROR_PARTY_JOINFAIL, pPacket->uiAccountDBID, pPacket->BreakIntoType );
					}
					break;
				}
				default:
				{
					_DANGER_POINT();
					return ERROR_GENERIC_UNKNOWNERROR;
				}
			}
#else
			if( pPacket->bIsOK == false )
			{
				if( pGameCon )
					pGameCon->SendDeleteBackupDungeonInfo( pPacket->uiAccountDBID, pPacket->biCharacterDBID, pPacket->iRoomID );
				break;
			}

			if( pGameCon )
			{
				pGameCon->SendBreakintoRoom(pPacket->iRoomID, pUser, BreakInto::Type::PartyRestore );
			}
			else
			{
				SendConfirmLastDungeonInfo( ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID );
			}
#endif // #if defined( PRE_PARTY_DB )
			break;
		}

#if defined (PRE_ADD_BESTFRIEND)
		case VIMA_SEARCH_BESTFRIEND:
			{
				MASearchBestFriend * pPacket = (MASearchBestFriend*)pData;
				if (g_pDivisionManager->SendSearchBestFriend(reinterpret_cast<MASearchBestFriend*>(pPacket)) == false)
				{
					MASearchBestFriend Packet;
					memset(&Packet, 0x00, sizeof(Packet));
					Packet.nAccountDBID = pPacket->nAccountDBID;
					_wcscpy(Packet.wszName, _countof(Packet.wszName), pPacket->wszName, _countof(Packet.wszName));
					Packet.nRet = ERROR_BESTFRIEND_NOTSEARCH;
					SendSearchBestFriend(&Packet);
				}
			}
			break;
		case VIMA_REGIST_BESTFRIEND:
			{
				MARegistBestFriend * pPacket = (MARegistBestFriend*)pData;
				if (g_pDivisionManager->SendRegistBestFriend(reinterpret_cast<MARegistBestFriend*>(pPacket)) == false)
				{
					MARegistBestFriendResult Packet;
					memset(&Packet, 0x00, sizeof(Packet));
					Packet.nAccountDBID = pPacket->nAccountDBID;
					Packet.nRet = ERROR_BESTFRIEND_REGISTFAIL_DESTUSER_SITUATION_NOTALLOWED;
					SendRegistBestFriendResult(&Packet);
				}
			}
			break;
		case VIMA_RETREGIST_BESTFRIEND:
			{
				g_pDivisionManager->SendRegistBestFriendResult(reinterpret_cast<MARegistBestFriendResult*>(pData));
			}
			break;
		case VIMA_COMPLETE_BESTFRIEND:
			{
				g_pDivisionManager->SendCompleteBestFriend(reinterpret_cast<MACompleteBestFriend*>(pData));
			}
			break;
		case VIMA_EDIT_BESTFRIENDMEMO:
			{
				g_pDivisionManager->SendEditBestFriendMemo(reinterpret_cast<MAEditBestFriendMemo*>(pData));
			}
			break;
		case VIMA_CANCEL_BESTFRIEND:
			{
				g_pDivisionManager->SendCancelBestFriend(reinterpret_cast<MACancelBestFriend*>(pData));
			}
			break;
		case VIMA_CLOSE_BESTFRIEND:
			{
				g_pDivisionManager->SendCloseBestFriend(reinterpret_cast<MACloseBestFriend*>(pData));
			}
			break;
		case VIMA_LEVELUP_BESTFRIEND:
			{
				g_pDivisionManager->SendLevelBestFriend(reinterpret_cast<MALevelUpBestFriend*>(pData));
			}
			break;
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined(PRE_ADD_QUICK_PVP)
		case VIMA_MAKEQUICKPVP_ROOM :
			{
				VIMAMakeQuickPvPRoom* pPacket = (VIMAMakeQuickPvPRoom*)pData;

				int nRet = g_pDivisionManager->MakeQuickPvPRoom(pPacket->uiMasterAccountDBID, pPacket->uiSlaveAccountDBID);
				SendMakeQuickPvPRoom(nRet, pPacket->uiMasterAccountDBID, pPacket->uiSlaveAccountDBID);				
			}
			break;
#endif
#if defined( PRE_WORLDCOMBINE_PARTY )
		case VIMA_GET_WORLDPARTYMEMBER:
			{
				int nRet = g_pDivisionManager->GetWorldPartyMember( reinterpret_cast<MAGetWorldPartyMember*>(pData) );
				if( nRet != ERROR_NONE )
				{
					MAGetWorldPartyMember* pPacket = (MAGetWorldPartyMember*)pData;
					GAMASendWorldPartyMember Packet;
					memset(&Packet, 0, sizeof(Packet));
					Packet.nAccountDBID = pPacket->nAccountDBID;
					Packet.biCharacterDBID = pPacket->biCharacterDBID;
					Packet.nRetCode = nRet;
					SendWorldPartyMember( &Packet );
				}
			}
			break;
#endif	//	#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_PRIVATECHAT_CHANNEL )
		case VIMA_PRIVATECHATCHANNEL_ADD:
			{
				int nRet = g_pDivisionManager->AddPrivateChatChannel( reinterpret_cast<MAAddPrivateChannel*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MEMBERADD:
			{
				int nRet = g_pDivisionManager->AddPrivateChatChannelMember( reinterpret_cast<MAAddPrivateChannelMember*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MEMBERINVITE:
			{
				int nRet = g_pDivisionManager->InvitePrivateChatChannelMember( reinterpret_cast<MAInvitePrivateChannelMember*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MEMBERINVITERESULT:
			{
				int nRet = g_pDivisionManager->InviteResultPrivateChatChannelMember( reinterpret_cast<MAInvitePrivateChannelMemberResult*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MEMBERDEL:
			{
				int nRet = g_pDivisionManager->DelPrivateChatChannelMember( reinterpret_cast<MADelPrivateChannelMember*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MOD:
			{
				int nRet = g_pDivisionManager->ModPrivateChatChannel( reinterpret_cast<MAModPrivateChannel*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MEMBERKICKRESULT:
			{
				int nRet = g_pDivisionManager->KickPrivateChatChannelMemberResult( reinterpret_cast<MAKickPrivateChannelMemberResult*>(pData) );				
			}
			break;
		case VIMA_PRIVATECHATCHANNEL_MODMEMBERNAME:
			{
				int nRet = g_pDivisionManager->ModPrivateChatChannelMemberName( reinterpret_cast<MAModPrivateChannelMemberName*>(pData) );				
			}
			break;
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
		case VIMA_WORLDPVP_REQGAMEID:
			{
				VIMACreateWorldPvPRoom* pPacket = reinterpret_cast<VIMACreateWorldPvPRoom*>(pData);
				if( g_pDivisionManager->RequestGameRoom(pPacket, m_cVillageID) == false )
				{
					GAMASetRoomID TempPacket;
					memset( &TempPacket, 0, sizeof(TempPacket) );					
					SendSetGameID( GameTaskType::PvP, REQINFO_TYPE_PVP, pPacket->nRoomIndex, 0, 0, 0, 0, 0, 0, 0, GetVillageID(), -1, _NORMAL_PARTY, pPacket->Data.eWorldReqType );
				}
				return ERROR_NONE;
			}
			break;
		case VIMA_WORLDPVP_BREAKINTO:
			{
				VIMAWorldPvPRoomBreakInto* pPacket = reinterpret_cast<VIMAWorldPvPRoomBreakInto*>(pData);
				CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByManagedID( pPacket->nManagedID );

				CDNUser* pUser = g_pDivisionManager->GetUserByCharacterDBID( pPacket->biCharacterDBID );
				if( pUser == NULL )
					break;				

				switch( pPacket->BreakIntoType )
				{				
				case BreakInto::Type::WorldPvPRoom:
					{
						if( pGameCon )
						{
							UINT uiUserState = pUser->GetPvPUserState()|PvPCommon::UserState::Syncing;
							pUser->SetPvPUserState( uiUserState );
							if( !pPacket->bPvPLobby )
								pUser->SetPvPVillageInfo();
							pUser->SetPvPIndex( 0 );
							pUser->SetPvPTeam(pPacket->uiTeam);

							CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( m_cVillageID );
							if( pVillageCon )
								pVillageCon->SendPvPChangeUserState( g_pDivisionManager->GetPvPLobbyChannelID(), pPacket->nRoomIndex, pUser->GetAccountDBID(), uiUserState );

							pGameCon->SendBreakintoRoom(pPacket->nRoomID, pUser, pPacket->BreakIntoType, pPacket->uiTeam );
						}
						else
						{
							MAWorldPvPRoomJoinResult Packet;
							memset(&Packet, 0, sizeof(Packet));
							Packet.nRet = ERROR_PVP_JOINROOM_NOTFOUNDROOM;
							Packet.uiAccountDBID = pPacket->uiAccountDBID;
							SendWorldPvPRoomJoinResult( &Packet );
						}
						break;
					}
				default:
					{
						_DANGER_POINT();
						return ERROR_GENERIC_UNKNOWNERROR;
					}
				}				
			}
			break;
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
		case VIMA_MUTE_USERFIND:
			{
				MAMuteUserFind *pPacket = (MAMuteUserFind*)pData;
				CDNUser *pUser = g_pDivisionManager->GetUserByName(pPacket->wszMuteUserName);

				if(pUser)
				{			
					MAMuteUserChat RestraintData;
					memset(&RestraintData, 0, sizeof(RestraintData));
					RestraintData.uiGmAccountID = pPacket->uiGmAccountID;
					RestraintData.nMuteMinute = pPacket->nMuteMinute;
					RestraintData.uiMuteUserAccountID = pUser->GetAccountDBID();
					switch(pUser->GetUserState())
					{
					case STATE_VILLAGE:
						{
							CDNVillageConnection* pVillageCon = g_pDivisionManager->GetVillageConnectionByVillageID( pUser->GetVillageID() );
							if(pVillageCon)
							{
								pVillageCon->SendMuteUserChatting(&RestraintData);
								return ERROR_NONE;
							}
						}
						break;
					case STATE_GAME:
						{
							CDNGameConnection* pGameCon = g_pDivisionManager->GetGameConnectionByGameID(pUser->GetGameID());
							if(pGameCon)
							{
								pGameCon->SendMuteUserChatting(&RestraintData);
								return ERROR_NONE;
							}
						}
						break;
					}
				}			
				
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4418) ) % pPacket->wszMuteUserName );
				SendChat( CHATTYPE_GM, pPacket->uiGmAccountID, L"", wString.c_str(), static_cast<short>(wString.size()) );
				
				return ERROR_NONE;
			}
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
		case VIMA_ALTEIAWORLD_SENDTICKET:
			{
				MAAlteiaWorldSendTicket *pPacket = (MAAlteiaWorldSendTicket*)pData;
				int nRet = g_pDivisionManager->AlteiaSendTicket( pPacket );				
				if( nRet != ERROR_NONE )
				{
					MAAlteiaWorldSendTicketResult Packet;
					memset(&Packet, 0, sizeof(Packet));
					Packet.nRetCode = nRet;
					Packet.biSendCharacterDBID = pPacket->biSendCharacterDBID;					
					g_pDivisionManager->AlteiaSendTicketResult( &Packet );
				}
			}
			break;
		case VIMA_ALTEIAWORLD_SENDTICKET_RESULT:
			{
				g_pDivisionManager->AlteiaSendTicketResult( reinterpret_cast<MAAlteiaWorldSendTicketResult*>(pData) );				
			}
			break;
#endif
#if defined( PRE_DRAGONBUFF )
		case VIMA_APPLY_WORLDBUFF:
			{
				g_pDivisionManager->SendApplyWorldBuff( reinterpret_cast<MAApplyWorldBuff*>(pData) );
			}
			break;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
		case VIMA_GAMBLE_CREATEROOMFLAG:
			{
				MACreateGambleRoom * pPacket = (MACreateGambleRoom*)pData;	
				g_pDivisionManager->SetGambleRoomFlag(pPacket->bFlag);
			}
			break;		
		case VIMA_GAMBLE_DELETEROOM:
			{
				MADelGambleRoom * pPacket = (MADelGambleRoom*)pData;					
				g_pDivisionManager->DeletePvPGambleRoom();
			}
			break;	
#endif
	}

	return ERROR_UNKNOWN_HEADER;
}

//channel
int CDNVillageConnection::GetMapIdxbyChannel(int nChannelID)
{
	std::vector <TChannelInfo>::iterator ii;
	for (ii = m_VillageChannelInfo.vOwnedChannelList.begin(); ii != m_VillageChannelInfo.vOwnedChannelList.end(); ii++)
		if ((*ii).nChannelID == nChannelID)
			return (*ii).nMapIdx;
	return -1;
}

bool CDNVillageConnection::HasManagedMapIdx(int nMapIdx)
{
	std::vector <TChannelInfo>::iterator ii;
	for (ii = m_VillageChannelInfo.vOwnedChannelList.begin(); ii != m_VillageChannelInfo.vOwnedChannelList.end(); ii++)
		if ((*ii).nMapIdx == nMapIdx)
			return true;
	return false;
}


//Village -> Master Connection Flow
void CDNVillageConnection::SendRegistWorldID()
{
	MARegist Regist = { 0, };
	Regist.cWorldSetID = g_Config.nWorldSetID;
	AddSendData(MAVI_REGISTWORLDINFO, 0, (char*)&Regist, sizeof(MARegist));
}

void CDNVillageConnection::SendReqUserList(short nRetCode)
{
	MAReqUserList packet;
	memset(&packet, 0, sizeof(packet));

	packet.nRetCode = nRetCode;

	AddSendData(MAVI_REQUSERLIST, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendVillageRegistComplete()
{
	AddSendData(MAVI_REGISTCOMPLETE, 0, NULL, 0);
}

//Village ServerInfo Syncro
void CDNVillageConnection::SendVillageInfo(std::vector <TVillageInfo> * vList)
{
	std::vector <TVillageInfo>::iterator ii;
	for (ii = vList->begin(); ii != vList->end(); ii++)
	{
		MAVIVillageInfo packet;
		memset(&packet, 0, sizeof(packet));

		packet.nVillageID = (*ii).cVillageID;
		for (int j = 0; j < (int)(*ii).vOwnedChannelList.size(); j++)
		{
			packet.Info[packet.cCount].nChannelID = (*ii).vOwnedChannelList[j].nChannelID;
			packet.Info[packet.cCount].nChannelIdx = (*ii).vOwnedChannelList[j].nChannelIdx;
			packet.Info[packet.cCount].nMapIdx = (*ii).vOwnedChannelList[j].nMapIdx;
			packet.Info[packet.cCount].nCurrentUserCount = (*ii).vOwnedChannelList[j].nCurrentUser;
			packet.Info[packet.cCount].nMaxUserCount = (*ii).vOwnedChannelList[j].nChannelMaxUser;
			packet.Info[packet.cCount].nChannelAttribute = (*ii).vOwnedChannelList[j].nAttribute;
			packet.Info[packet.cCount].nMeritBonusID = (*ii).vOwnedChannelList[j].nMeritBonusID;
			if (packet.Info[packet.cCount].nMeritBonusID > 0)
			{
				const TMeritInfo * pInfo = g_pExtManager->GetMeritInfo(packet.Info[packet.cCount].nMeritBonusID);
				if (pInfo)
				{
					packet.Info[packet.cCount].cMinLevel = pInfo->nMinLevel;
					packet.Info[packet.cCount].cMaxLevel = pInfo->nMaxLevel;
				}
				else
					_DANGER_POINT();
			}
			packet.Info[packet.cCount].cVillageID = (*ii).cVillageID;
			packet.Info[packet.cCount].bVisibility = (*ii).vOwnedChannelList[j].bVisibility;
			_strcpy(packet.Info[packet.cCount].szIP, _countof(packet.Info[packet.cCount].szIP), (*ii).szIP, (int)strlen((*ii).szIP));
			packet.Info[packet.cCount].nPort = (*ii).nPort;
			packet.Info[packet.cCount].nLimitLevel = (*ii).vOwnedChannelList[j].nLimitLevel;
			packet.Info[packet.cCount].bShow = (*ii).vOwnedChannelList[j].bShow;
			packet.Info[packet.cCount].nDependentMapID = (*ii).vOwnedChannelList[j].nDependentMapID;
#if defined(PRE_ADD_CHANNELNAME)
			_wcscpy(packet.Info[packet.cCount].wszLanguageName, _countof(packet.Info[packet.cCount].wszLanguageName), (*ii).vOwnedChannelList[j].wszLanguageName, (int)wcslen((*ii).vOwnedChannelList[j].wszLanguageName));
#endif //#if defined(PRE_ADD_CHANNELNAME)
			packet.cCount++;
		}
		AddSendData(MAVI_VILLAGEINFO, 0, (char*)&packet, sizeof(MALOVillageInfo) - sizeof(packet.Info) + (sizeof(sChannelInfo) * packet.cCount));
	}
}

void CDNVillageConnection::SendVillageInfoDelete(int nVillageID)
{
	MAVIVillageInfoDelete packet;
	memset(&packet, 0, sizeof(packet));
	packet.nVillageID = nVillageID;
	AddSendData(MAVI_VILLAGEINFODEL, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendLoginState(UINT nSessionID, short nRet)
{
	MAVILoginState State = { 0, };
	State.nSessionID = nSessionID;
	State.nRet = nRet;
	AddSendData(MAVI_LOGINSTATE, 0, (char*)&State, sizeof(MAVILoginState));
}

//User
void CDNVillageConnection::SendCheckUser(CDNUser *pUser, UINT nSessionID, int nRet)
{
	MAVICheckUser packet;
	memset(&packet, 0, sizeof(packet));

	if (pUser)
	{
		packet.nSessionID = pUser->GetSessionID();
		packet.nAccountDBID = pUser->GetAccountDBID();
		_wcscpy(packet.wszAccountName, IDLENMAX, pUser->GetAccountName(), (int)wcslen(pUser->GetAccountName()));
		packet.biCharacterDBID = pUser->GetCharacterDBID();
#if !defined( PRE_PARTY_DB )
		packet.PartyID = pUser->m_VillageCheckPartyID;
#endif
		packet.nChannelID = pUser->GetChannelID();
		pUser->m_VillageCheckPartyID = 0;

		packet.cPvPVillageID			= pUser->GetPvPVillageID();
		packet.unPvPVillageChannelID	= pUser->GetPvPVillageChannelID();
		packet.bAdult = pUser->IsAdult();
		packet.cPCBangGrade = pUser->GetPCBangGrade();
#ifdef PRE_ADD_COMEBACK
		packet.bComebackUser = pUser->m_bComebackUser;
		pUser->m_bComebackUser = false;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
		packet.bReConnectNewbieReward = pUser->m_bReConnectNewbieReward;
		pUser->m_bReConnectNewbieReward = false;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	}
	else
	{
		//So village server only handles the error
		packet.nSessionID = nSessionID;
	}

	packet.nRet = nRet;
	int nLen = sizeof(MAVICheckUser);

	AddSendData(MAVI_CHECKUSER, 0, (char*)&packet, nLen);
}

void CDNVillageConnection::SendTargetVillageInfo(UINT nAccountDBID, BYTE cVillageID, int nTargetChannelID, int nTargetMapIdx, BYTE cTargetGateNo, const char * pIP, USHORT nPort, short nRet, bool bPartyInto, INT64 nItemSerial)
{
	MAVITargetVillageInfo packet;

	packet.nAccountDBID = nAccountDBID;
	packet.cVillageID = cVillageID;
	packet.nTargetChannelID = nTargetChannelID;
	packet.nTargetMapIdx = nTargetMapIdx;
	packet.cTargetGateNo = cTargetGateNo;
	_strcpy(packet.szIP, _countof(packet.szIP), pIP, (int)strlen(pIP));
	packet.nPort = nPort;
	packet.nRet = nRet;
	packet.cPartyInto = bPartyInto == true ? 1 : 0;
	packet.nItemSerial = nItemSerial;

	AddSendData(MAVI_TARGETVILLAGEINFO, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendAddUserState(const WCHAR * pName, INT64 biChracterDBID, int nLocationState, int nCommunityState, int nChannelID, int nMapIdx)
{
	MAUserState packet;
	memset(&packet, 0, sizeof(packet));

	packet.Type = WorldUserState::Add;
	packet.biCharacterDBID = biChracterDBID;
	packet.cLocationState = static_cast<BYTE>(nLocationState == -1 ? _LOCATION_LOGIN : nLocationState);		//Initially created in Login
	packet.cCommunityState = _COMMUNITY_NONE;	//Usage TBD
	packet.cNameLen = (BYTE)wcslen(pName);
	_wcscpy(packet.wszBuf, NAMELENMAX, pName, (int)wcslen(pName));
	packet.nChannelID = nChannelID == -1 ? 0 : nChannelID;
	packet.nMapIdx = nMapIdx == -1 ? 0 : nMapIdx;

	AddSendData(MAVI_USERSTATE, 0, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + (packet.cNameLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendDelUserState(const WCHAR * pName, INT64 biChracterDBID)
{
	MAUserState packet;
	memset(&packet, 0, sizeof(packet));

	packet.Type = WorldUserState::Delete;
	packet.biCharacterDBID = biChracterDBID;
	packet.cNameLen = (BYTE)wcslen(pName);
	_wcscpy(packet.wszBuf, NAMELENMAX, pName, (int)wcslen(pName));

	AddSendData(MAVI_USERSTATE, 0, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + (packet.cNameLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendUpdateUserState(const WCHAR * pName, INT64 biCharacterDBID, int nLocationState, int nCommunityState, int nChannelID, int nMapIdx)
{
	MAUserState packet;
	memset(&packet, 0, sizeof(packet));

	packet.Type = WorldUserState::Modify;
	packet.biCharacterDBID = biCharacterDBID;
	packet.cNameLen = (BYTE)wcslen(pName);
	_wcscpy(packet.wszBuf, NAMELENMAX, pName, (int)wcslen(pName));
	packet.cLocationState = nLocationState;
	packet.cCommunityState = nCommunityState;
	packet.nChannelID = nChannelID;
	packet.nMapIdx = nMapIdx;

	AddSendData(MAVI_USERSTATE, 0, (char*)&packet, sizeof(packet) - sizeof(packet.wszBuf) + (packet.cNameLen * sizeof(WCHAR)));
}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
void CDNVillageConnection::SendSetGameID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, ULONG nIP, USHORT nPort, USHORT nTcpPort, int iServerIdx, USHORT wGameID, int iRoomID, USHORT unVillageChannelID, BYTE cVillageID, int nRet, ePartyType Type, WorldPvPMissionRoom::Common::eReqType eWorldReqType )
#else	// #if defined( PRE_WORLDCOMBINE_PVP )
void CDNVillageConnection::SendSetGameID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, ULONG nIP, USHORT nPort, USHORT nTcpPort, int iServerIdx, USHORT wGameID, int iRoomID, USHORT unVillageChannelID, BYTE cVillageID, int nRet, ePartyType Type )
#endif //#if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
void CDNVillageConnection::SendSetGameID( GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, ULONG nIP, USHORT nPort, USHORT nTcpPort, int iServerIdx, USHORT wGameID, int iRoomID, USHORT unVillageChannelID, BYTE cVillageID, int nRet )
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
{
	MAVISetGameID TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.GameTaskType		= GameTaskType;
	TxPacket.cReqGameIDType		= cReqGameIDType;
	TxPacket.InstanceID			= InstanceID;
	TxPacket.nIP				= nIP;
	TxPacket.nPort				= nPort;
	TxPacket.nTcpPort			= nTcpPort;
	TxPacket.cServerIdx			= iServerIdx;
	TxPacket.wGameID			= wGameID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.cVillageID			= cVillageID;
	TxPacket.nRet				= nRet;
#if defined( PRE_WORLDCOMBINE_PARTY )
	TxPacket.Type				= Type;
#endif
#if defined( PRE_WORLDCOMBINE_PVP )
	TxPacket.eWorldReqType = eWorldReqType;
#endif

	AddSendData( MAVI_SETGAMEID, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendLadderSetGameID( ULONG nIP, USHORT nPort, USHORT nTcpPort, int iServerIdx, USHORT wGameID, int iRoomID, USHORT unVillageChannelID, BYTE cVillageID, int nRet, GAMASetRoomID* pPacket/*=NULL*/, int iGameModeTableID/*=-1*/ )
{
	MAVILadderSetGameID TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nIP				= nIP;
	TxPacket.nPort				= nPort;
	TxPacket.nTcpPort			= nTcpPort;
	TxPacket.cServerIdx			= iServerIdx;
	TxPacket.wGameID			= wGameID;
	TxPacket.iRoomID			= iRoomID;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.cVillageID			= cVillageID;
	TxPacket.nRet				= nRet;
	TxPacket.nGameModeTableID	= iGameModeTableID;

	if( TxPacket.nRet == ERROR_NONE || TxPacket.nRet == -1 )
	{
		if( pPacket == NULL )
		{
			_ASSERT(0);
			_DANGER_POINT();
			return;
		}

		for( int i=0 ; i<_countof(pPacket->nRoomMember) ; ++i )
		{
			if( pPacket->nRoomMember[i] <= 0 )
				continue;
			TxPacket.uiAccountDBIDArr[TxPacket.cUserCount++] = pPacket->nRoomMember[i];
		}
	}

	int iSize = sizeof(TxPacket)-sizeof(TxPacket.uiAccountDBIDArr)+sizeof(TxPacket.uiAccountDBIDArr[0])*TxPacket.cUserCount;
	AddSendData( MAVI_LADDERSYSTEM_SETGAMEID, 0, reinterpret_cast<char*>(&TxPacket), iSize );
}

#if defined( PRE_PARTY_DB )

void CDNVillageConnection::SendResPartyInvite( int iRet, UINT uiReqAccountDBID )
{
	MAVIResPartyInvite TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	TxPacket.uiReqAccountDBID	= uiReqAccountDBID;

	AddSendData( MAVI_RESPARTYINVITE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendReqPartyInvite( const VIMAReqPartyInvite* pPacket )
{
	MAVIReqPartyInvite TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket = *pPacket;

	AddSendData( MAVI_REQPARTYINVITE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPartyInviteDenied( const VIMAPartyInviteDenied* pPacket )
{
	MAVIPartyInviteDenided TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket = *pPacket;

	AddSendData( MAVI_PARTYINVITEDENIED, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
void CDNVillageConnection::SendReqPartyAskJoin( const VIMAReqPartyAskJoin* pPacket )
{
	MAVIReqPartyAskJoin TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket = *pPacket;

	AddSendData( MAVI_REQPARTYASKJOIN, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendResPartyAskJoin( int iRet, UINT uiAccountDBID )
{
	MAVIResPartyAskJoin TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	TxPacket.uiReqAccountDBID = uiAccountDBID;

	AddSendData( MAVI_RESPARTYASKJOIN, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendAskJoinAgreeInfo( UINT uiAccountDBID , TPARTYID PartyID, int iPassword )
{
	MAVIAskJoinAgreeInfo TxPacket;		
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAskerAccountDBID = uiAccountDBID;
	TxPacket.PartyID = PartyID;
	TxPacket.iPassword = iPassword;

	AddSendData( MAVI_ASKJOINAGREEINFO, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#endif

#else

void CDNVillageConnection::SendAddParty(UINT nLeaderAccountDBID, UINT nLeaderSessionID, TPARTYID PartyID, const WCHAR *pwszPartyName, const WCHAR * pwszPassword, int nChannelID, BYTE cPartyMemberMax, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemRank, BYTE cUserLvLimitMin, BYTE cUserLvLimitMax, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, BYTE cPartyJobDice, UINT nAfterInviteAccountDBID, BYTE cUseVoice)
{
	MAVIReqPartyID AddParty;
	memset(&AddParty, 0, sizeof(MAVIReqPartyID));

	AddParty.nAccountDBID = nLeaderAccountDBID;
	AddParty.nSessionID = nLeaderSessionID;
	AddParty.PartyID = PartyID;
	AddParty.cMemberMax = cPartyMemberMax;
	AddParty.nChannelID = nChannelID;
	AddParty.ItemLootRule = ItemLootRule;
	AddParty.ItemRank = ItemRank;
	AddParty.cUserLvLimitMin = cUserLvLimitMin;
	AddParty.cUserLvLimitMax = cUserLvLimitMax;
	AddParty.nTargetMapIdx = nTargetMapIdx;
	AddParty.Difficulty = Difficulty;
	AddParty.nAfterInviteAccountDBID = nAfterInviteAccountDBID;
	
	_wcscpy(AddParty.wszPartyName, PARTYNAMELENMAX, pwszPartyName, (int)wcslen(pwszPartyName));

	AddParty.cPartyJobDice = cPartyJobDice;
	AddParty.cUseVoice = cUseVoice;
	if (pwszPassword != NULL && pwszPassword[0] != '\0')
		_wcscpy(AddParty.wszPartyPass, PARTYPASSWORDMAX, pwszPassword, (int)wcslen(pwszPassword));

	AddSendData(MAVI_REQPARTYID, 0, (char*)&AddParty, sizeof(AddParty));
}
#endif // #if defined( PRE_PARTY_DB )

void CDNVillageConnection::SendPushParty(UINT nLeaderAccountDBID, TPARTYID PartyID, int nChannelID, int nRandomSeed, TPartyData * pPartyData)
{
	if (pPartyData == NULL) return;
	MAVIPushParty packet = { 0, };

	packet.nLeaderAccountDBID = nLeaderAccountDBID;		
#if defined( PRE_PARTY_DB )
	packet.PartyData = pPartyData->PartyData;
	packet.PartyData.iLocationID = nChannelID;
#else
	packet.PartyID = PartyID;
	packet.nChannelID = nChannelID;
	packet.cMemberMax = pPartyData->cMemberMax;
	packet.ItemLootRule = pPartyData->ItemLootRule;
	packet.ItemLootRank = pPartyData->ItemLootRank;	
	packet.nUserLvLimitMin = pPartyData->cUserLvLimitMin;
	packet.nUserLvLimitMax = pPartyData->cUserLvLimitMax;	
	packet.nTargetMapIdx = pPartyData->nTargetMapIdx;
	packet.Difficulty = pPartyData->Difficulty;
	packet.cUpkeepCount = pPartyData->cUpkeepCount;
	_wcscpy(packet.wszPartyName, PARTYNAMELENMAX, pPartyData->wszPartyName, (int)wcslen(pPartyData->wszPartyName));
#endif // #if defined( PRE_PARTY_DB )

	packet.nRandomSeed = nRandomSeed;
	memcpy(packet.nKickedMemberList, pPartyData->nKickedMemberDBID, sizeof(UINT[PARTYKICKMAX]));
	
	packet.nVoiceChannelID = pPartyData->nVoiceChannelID;

#if defined( PRE_PARTY_DB )	
#else
	packet.cIsPartyJobDice = pPartyData->cJobDice;
	_wcscpy(packet.wszPartyPass, PARTYPASSWORDMAX, pPartyData->wszPartyPass, (int)wcslen(pPartyData->wszPartyPass));
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )	
	memcpy(packet.MemberInfo, pPartyData->MemberInfo, sizeof(TMemberInfo) * pPartyData->PartyData.iCurMemberCount);
#else
	packet.cMemberCount = pPartyData->cMemberCount;
	memcpy(packet.MemberInfo, pPartyData->MemberInfo, sizeof(TMemberInfo) * pPartyData->cMemberCount);
#endif

#if defined( PRE_PARTY_DB )
	AddSendData(MAVI_PUSHPARTY, 0, (char*)&packet, sizeof(packet) - sizeof(packet.MemberInfo) + (sizeof(TMemberInfo) * packet.PartyData.iCurMemberCount));
#else
	AddSendData(MAVI_PUSHPARTY, 0, (char*)&packet, sizeof(packet) - sizeof(packet.MemberInfo) + (sizeof(TMemberInfo) * packet.cMemberCount));
#endif
}

void CDNVillageConnection::SendInvitepartyMember(int nGameServerID, int nVillageMapIdx, UINT nInvitedAccountDBID, const GAMAInvitePartyMember * pInvitePartyMember, const char * pPassClassIds, char cPermitLevel)
{
	MAVIInvitePartyMember packet;
	memset(&packet, 0, sizeof(MAVIInvitePartyMember));

	packet.nGameServerID = nGameServerID;
	packet.nVillageMapIdx = nVillageMapIdx;

	packet.PartyID = pInvitePartyMember->PartyID;
	packet.nMapIdx = pInvitePartyMember->nMapIdx;
	packet.cLimitLevelMin = pInvitePartyMember->cLimitLevelMin;
#if defined( PRE_PARTY_DB )
#else
	packet.cLimitLevelMax = pInvitePartyMember->cLimitLevelMax;
#endif // #if defined( PRE_PARTY_DB )
	packet.cPartyMemberMax = pInvitePartyMember->cPartyMemberMax;
	packet.cPartyMemberCount = pInvitePartyMember->cPartyMemberCount;
	packet.cPartyAvrLevel = pInvitePartyMember->cPartyAvrLevel;
	STRCPYW(packet.wszPartyName, PARTYNAMELENMAX, pInvitePartyMember->wszPartyName);

	packet.nInviterAccountDBID = pInvitePartyMember->nInviterAccountDBID;
	packet.biInviterCharacterDBID = pInvitePartyMember->biInviterCharacterDBID;
	STRCPYW(packet.wszInviterName, NAMELENMAX, pInvitePartyMember->wszInviterName);
	packet.nInvitedAccountDBID = nInvitedAccountDBID;
	STRCPYW(packet.wszInvitedName, NAMELENMAX, pInvitePartyMember->wszInvitedName);

	packet.cPermitLevel = cPermitLevel;
	memcpy(packet.cPassClassIds, pPassClassIds, sizeof(char[PERMITMAPPASSCLASSMAX]));

	AddSendData(MAVI_INVITEPARTYMEMBER, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendResult(UINT nAccountDBID, BYTE cMainCmd, short nRet)
{
	MAVIResult Result = { 0, };

	Result.nAccountDBID = nAccountDBID;
	Result.nMainCmd = cMainCmd;
	Result.nRet = nRet;

	AddSendData(cMainCmd, 0, (char*)&Result, sizeof(Result));
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
void CDNVillageConnection::SendGetPartyID( UINT nAccountDBID, UINT nSenderAccountDBID )
{
	MAVIGetPartyID packet;
	memset(&packet, 0, sizeof(MAVIGetPartyID));
	
	packet.nAccountDBID = nAccountDBID;
	packet.nSenderAccountDBID = nSenderAccountDBID;

	AddSendData(MAVI_GETPARTYID, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendGetPartyIDResult( UINT nSenderAccountDBID, TPARTYID PartyID )
{
	MAVIGetPartyIDResult packet;
	memset(&packet, 0, sizeof(MAVIGetPartyIDResult));

	packet.nSenderAccountDBID = nSenderAccountDBID;
	packet.PartyID = PartyID;

	AddSendData(MAVI_GETPARTYID_RESULT, 0, (char*)&packet, sizeof(packet));
}
#endif

void CDNVillageConnection::SendInviteGuildMember(MAInviteGuildMember* pPacket)
{
	AddSendData(MAVI_INVITEGUILDMEMB, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendInviteGuildMemberResult(UINT nAccountDBID, int nRet, bool bAck, LPCWSTR pwszToCharacterName)
{
	MAVIGuildMemberInvitedResult Result = { 0, };

	Result.nAccountDBID = nAccountDBID;
	Result.nRet			= nRet;
	Result.bAck			= bAck;
	STRCPYW(Result.wszInvitedName, NAMELENMAX, pwszToCharacterName);

	AddSendData(MAVI_INVITEGUILDMEMBER_RETMSG, 0, (char*)&Result, sizeof(Result));
}

void CDNVillageConnection::SendGuildWareInfo(MAGuildWareInfo* pPacket)
{
	AddSendData(MAVI_GUILDWAREINFO, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendGuildWareInfoResult(MAGuildWareInfoResult* pPacket)
{
	AddSendData(MAVI_RETGUILDWAREINFO, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendGuildMemberLevelUp(MAGuildMemberLevelUp* pPacket)
{
	AddSendData(MAVI_GUILDMEMBER_LEVELUP, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendRefreshGuildItem(MARefreshGuildItem* pPacket)
{
	AddSendData(MAVI_REFRESH_GUILDITEM, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendRefreshGuildCoin(MARefreshGuildCoin* pPacket)
{
	AddSendData(MAVI_REFRESH_GUILDCOIN, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendExtendGuildWareSize(MAExtendGuildWare* pPacket)
{
	AddSendData(MAVI_EXTEND_GUILDWARE, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendDismissGuild(MADismissGuild* pPacket)
{
	AddSendData(MAVI_DISMISSGUILD, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendAddGuildMember(MAAddGuildMember* pPacket)
{
	AddSendData(MAVI_ADDGUILDMEMB, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendDelGuildMember(MADelGuildMember* pPacket)
{
	AddSendData(MAVI_DELGUILDMEMB, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendChangeGuildInfo(MAChangeGuildInfo* pPacket)
{
	AddSendData(MAVI_CHANGEGUILDINFO, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendChangeGuildMemberInfo(MAChangeGuildMemberInfo* pPacket)
{
	AddSendData(MAVI_CHANGEGUILDMEMBINFO, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendGuildChat(MAGuildChat* pPacket)
{
	AddSendData(MAVI_GUILDCHAT, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
void CDNVillageConnection::SendDoorsGuildChat(MADoorsGuildChat* pPacket)
{
	AddSendData(MAVI_GUILDCHAT_MOBILE, 0, reinterpret_cast<char*>(pPacket), sizeof(MADoorsGuildChat) - sizeof(pPacket->wszChatMsg) + (sizeof(WCHAR) * pPacket->nLen));
}
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

void CDNVillageConnection::SendChangeGuildName(MAGuildChangeName* pPacket)
{
	AddSendData(MAVI_GUILDCHANGE_NAME, 0, reinterpret_cast<char*>(pPacket), sizeof(MAGuildChangeName));
}

void CDNVillageConnection::SendChangeGuildMark(MAGuildChangeMark* pPacket)
{
	AddSendData(MAVI_GUILDCHANGEMARK, 0, reinterpret_cast<char*>(pPacket), sizeof(MAGuildChangeMark));
}

void CDNVillageConnection::SendUpdateGuildExp(MAUpdateGuildExp* pPacket)
{
	AddSendData(MAVI_UPDATEGUILDEXP, 0, reinterpret_cast<char*>(pPacket), sizeof(MAUpdateGuildExp));
}

void CDNVillageConnection::SendEnrollGuildWar(MAEnrollGuildWar* pPacket)
{
	AddSendData(MAVI_ENROLL_GUILDWAR, 0, reinterpret_cast<char*>(pPacket), sizeof(MAEnrollGuildWar));
}
void CDNVillageConnection::SendGetGuildWarSchedule()
{
	AddSendData(MAVI_GETGUILDWAR_SCHEDULE, 0, NULL, 0);
}
void CDNVillageConnection::SendChangeGuildWarStep(MAChangeGuildWarStep* pPacket)
{
	AddSendData(MAVI_CHANGE_GUILDWAR_STEP, 0, reinterpret_cast<char*>(pPacket), sizeof(MAChangeGuildWarStep));
}
void CDNVillageConnection::SendGetGuildWarFinalSchedule(short wScheduleID)
{
	MAGetGuildWarFinalSchedule  GetGuildWarFinalSchedule;
	GetGuildWarFinalSchedule.wScheduleID = wScheduleID;

	AddSendData(MAVI_GETGUILDWAR_FINALSCHEDULE, 0, reinterpret_cast<char*>(&GetGuildWarFinalSchedule), sizeof(MAGetGuildWarFinalSchedule));
}
void CDNVillageConnection::SendGetGuildWarPoint()
{
	AddSendData(MAVI_GETGUILDWAR_POINT, 0, NULL, 0);
}
void CDNVillageConnection::SendAddGuildWarPoint(MAAddGuildWarPoint* pPacket)
{
	AddSendData(MAVI_ADDGUILDWAR_POINT, 0, reinterpret_cast<char*>(pPacket), sizeof(MAAddGuildWarPoint));
}

void CDNVillageConnection::SendSetGuildWarPoint(MASetGuildWarPoint* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_POINT, 0, reinterpret_cast<char*>(pPacket), sizeof(MASetGuildWarPoint));
}

void CDNVillageConnection::SendGetGuildWarFinalTeam()
{
	AddSendData(MAVI_GETGUILDWAR_FINAL_TEAM, 0, NULL, 0);
}

void CDNVillageConnection::SendGetGuildWarTournamentInfo(short wScheduleID)
{
	MAGetGuildWarTournamentInfo GetGuildWarTournamentInfo;
	GetGuildWarTournamentInfo.wScheduleID = wScheduleID;
	AddSendData(MAVI_GETGUILDWAR_TOURNAMENTINFO, 0, reinterpret_cast<char*>(&GetGuildWarTournamentInfo), sizeof(MAGetGuildWarTournamentInfo));
}

void CDNVillageConnection::SendGetGuildWarPreWinGuild()
{
	AddSendData(MAVI_GETGUILDWAR_PREWIN_GUILD, 0, NULL, 0);
}

void CDNVillageConnection::SendGetGuildWarTrialStats()
{
	AddSendData(MAVI_GETGUILDWAR_TRIAL_STATS, 0, NULL, 0);
}

void CDNVillageConnection::SendSetGuildwarFinalProcess(char cGuildFinalPart, __time64_t tBeginTime)
{
	MASetGuildWarFinalProcess SetGuildWarFinalProcess;
	memset(&SetGuildWarFinalProcess, 0, sizeof(MASetGuildWarFinalProcess));

	SetGuildWarFinalProcess.cGuildFinalPart = cGuildFinalPart;
	SetGuildWarFinalProcess.tBeginTime = tBeginTime;

	AddSendData(MAVI_SETGUILDWAR_FINALPROCESS, 0, reinterpret_cast<char*>(&SetGuildWarFinalProcess), sizeof(MASetGuildWarFinalProcess));
}

void CDNVillageConnection::SendSetGuildWarFinalTeam(SGuildTournamentInfo* pGuildTournamentInfo)
{
	MAVISetGuildWarFinalTeam SetGuildWarFinalTeam;
	memset(&SetGuildWarFinalTeam, 0, sizeof(MAVISetGuildWarFinalTeam));

	for( int i=0; i<GUILDWAR_FINALS_TEAM_MAX; ++i)	
	{
		SetGuildWarFinalTeam.GuildDBID[i] = pGuildTournamentInfo[i].GuildUID.nDBID;
	}
	
	AddSendData(MAVI_SETGUILDWAR_FINAL_TEAM, 0, reinterpret_cast<char*>(&SetGuildWarFinalTeam), sizeof(MAVISetGuildWarFinalTeam));
}

void CDNVillageConnection::SendSetGuildWarTournamentInfo(SGuildTournamentInfo* pGuildTournamentInfo)
{
	MAGuildWarTournamentInfo SetGuildWarTournamentInfo;
	memset(&SetGuildWarTournamentInfo, 0, sizeof(MAGuildWarTournamentInfo));

	memcpy(&SetGuildWarTournamentInfo, pGuildTournamentInfo, sizeof(SetGuildWarTournamentInfo));
	AddSendData(MAVI_SETGUILDWAR_TOURNAMENTINFO, 0, reinterpret_cast<char*>(&SetGuildWarTournamentInfo), sizeof(MAGuildWarTournamentInfo));
}
void CDNVillageConnection::SendSetGuildWarSecretMission(MASetGuildWarSecretMission * pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_SECRETMISSION, 0, reinterpret_cast<char*>(pPacket), sizeof(MASetGuildWarSecretMission));
}
void CDNVillageConnection::SendSetGuildWarFinalResult(MAVISetGuildWarFinalResult* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_FINAL_RESULT, 0, reinterpret_cast<char*>(pPacket), sizeof(MAVISetGuildWarFinalResult));
}

void CDNVillageConnection::SendSetGuildWarFinalResultDB(short wScheduleID, char cMatchSequence, UINT GuildDBID, char cMatchTypeCode,bool bWin)
{
	MAVISetGuildWarFinalDB SetGuildWarFinalDB;
	memset(&SetGuildWarFinalDB, 0, sizeof(SetGuildWarFinalDB));
	SetGuildWarFinalDB.wScheduleID = wScheduleID;
	SetGuildWarFinalDB.cMatchSequence = cMatchSequence;
	SetGuildWarFinalDB.GuildDBID = GuildDBID;
	SetGuildWarFinalDB.cMatchTypeCode = cMatchTypeCode;
	SetGuildWarFinalDB.bWin = bWin;

	AddSendData(MAVI_SETGUILDWAR_FINAL_RESULT_DB, 0, reinterpret_cast<char*>(&SetGuildWarFinalDB), sizeof(MAVISetGuildWarFinalDB));
}

void CDNVillageConnection::SendSetGuildWarPreWinGuild(MAGuildWarPreWinGuild* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_PREWIN_GUILD, 0, reinterpret_cast<char*>(pPacket), sizeof(MAGuildWarPreWinGuild));
}

void CDNVillageConnection::SendSetGuildWarPreWinSkillCoolTime(MAGuildWarPreWinSkillCoolTime* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_PREWIN_SKILLCOOLTIME, 0, reinterpret_cast<char*>(pPacket), sizeof(MAGuildWarPreWinSkillCoolTime));
}

void CDNVillageConnection::SendSetGuildWarSchedule(MASetGuildWarEventTime* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_EVENT_TIME, 0, reinterpret_cast<char*>(pPacket), sizeof(MASetGuildWarEventTime));
}
void CDNVillageConnection::SendSetGuildWarTournamentWin(MASetGuildWarTournamentWin* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_TOURNAMENTWIN, 0, reinterpret_cast<char*>(pPacket), sizeof(MASetGuildWarTournamentWin));
}
void CDNVillageConnection::SendAddDBJobSystemReserve(char cJobType)
{
	MAVIAddDBJobSystemReserve Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.cJobType = cJobType;

	AddSendData(MAVI_ADD_DBJOBSYSTEM_RESERVE, 0, reinterpret_cast<char*>(&Packet), sizeof(MAVIAddDBJobSystemReserve));
}
void CDNVillageConnection::SendGetDBJobSystemReserve(int nJobSeq)
{
	MAVIGetDBJobSystemReserve Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.nJobSeq = nJobSeq;

	AddSendData(MAVI_GET_DBJOBSYSTEM_RESERVE, 0, reinterpret_cast<char*>(&Packet), sizeof(MAVIGetDBJobSystemReserve));
}

void CDNVillageConnection::SendGetGuildWarPointRunning()
{
	AddSendData(MAVI_GETGUILDWAR_POINT_RUNNING, 0, NULL, 0);
}
void CDNVillageConnection::SendSetGuildWarPointRunning(MASetGuildWarPointRunningTotal* pPacket)
{
	AddSendData(MAVI_SETGUILDWAR_POINT_RUNNING, 0, reinterpret_cast<char*>(pPacket), sizeof(MASetGuildWarPointRunningTotal));
}
void CDNVillageConnection::SendGetGuildWarPreWindGuildReward(short wSchedulID, UINT nGuildDBID)
{
	MAVIGetGuildWarPrewinReward Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.wScheduleID = wSchedulID;
	Packet.nGuildDBID = nGuildDBID;

	AddSendData(MAVI_GETGUILDWAR_PREWIN_REWARD, 0, reinterpret_cast<char*>(&Packet), sizeof(MAVIGetGuildWarPrewinReward));
}

void CDNVillageConnection::SendGuildWarRefreshGuildPoint(MAGuildWarRefreshGuildPoint* pPacket)
{
	AddSendData(MAVI_GUILDWAR_REFRESH_GUILD_POINT, 0, reinterpret_cast<char*>(pPacket), sizeof(MAGuildWarRefreshGuildPoint));
}

void CDNVillageConnection::SendResetGuildWarBuyedItemCount()
{
	AddSendData(MAVI_GUILDWAR_RESET_BUYED_ITEM_COUNT, 0, NULL, 0);
}

void CDNVillageConnection::SendGuildRecruitMember(MAGuildRecruitMember* pPacket)
{
	AddSendData(MAVI_GUILDRECRUIT_MEMBER, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}
void CDNVillageConnection::SendAddGuildRewardItem(MAGuildRewardItem* pPacket)
{
	AddSendData(MAVI_GUILD_ADD_REWARDITEM, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}
void CDNVillageConnection::SendExtendGuildSize(MAExtendGuildSize* pPacket)
{
	AddSendData(MAVI_GUILD_EXTEND_GUILDSIZE, 0, reinterpret_cast<char*>(pPacket), sizeof(*pPacket));
}

void CDNVillageConnection::SendUpdateGuildWare(int nGuildID)
{
	MAUpdateGuildWare packet;
	memset(&packet, 0, sizeof(MAUpdateGuildWare));

	packet.GUildID.Set(g_Config.nWorldSetID, nGuildID);

	AddSendData(MAVI_UPDATEGUILDWARE, 0, reinterpret_cast<char*>(&packet), sizeof(MAUpdateGuildWare));
}

void CDNVillageConnection::SendChangeCharacterName(MAChangeCharacterName* pPacket)
{
	AddSendData(MAVI_CHANGE_CHARACTERNAME, 0, reinterpret_cast<char*>(pPacket), sizeof(MAChangeCharacterName));
}

void CDNVillageConnection::SendFriendAddNotice(UINT nAddedAccountDBID, const WCHAR * pName)
{
	MAVIFriendAddNotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.nAddedAccountDBID = nAddedAccountDBID;
	_wcscpy(packet.wszAddName, NAMELENMAX, pName, (int)wcslen(pName));

	AddSendData(MAVI_FRIENDADDNOTICE, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendPrivateChat(UINT nToAccountDBID, const WCHAR *pwszFromCharacterName, char cType, const WCHAR *pwszChatMsg, short wChatLen, int nRet)
{
	MAPrivateChat Private;
	memset(&Private, 0, sizeof(MAPrivateChat));

	Private.nAccountDBID = nToAccountDBID;
	Private.cType = cType;
	_wcscpy(Private.wszFromCharacterName, NAMELENMAX, pwszFromCharacterName, (int)wcslen(pwszFromCharacterName));
	Private.nRet = nRet;

	if (nRet < 0){	// It is an error
		AddSendData(MAVI_PRIVATECHAT, 0, (char*)&Private, sizeof(MAPrivateChat) - sizeof(Private.wszChatMsg));
	}
	else {
		Private.wChatLen = wChatLen;
		_wcscpy(Private.wszChatMsg, CHATLENMAX, pwszChatMsg, (int)wcslen(pwszChatMsg));
		AddSendData(MAVI_PRIVATECHAT, 0, (char*)&Private, sizeof(MAPrivateChat) - sizeof(Private.wszChatMsg) + (wChatLen * 2));
	}
}

void CDNVillageConnection::SendChat(char cType, UINT nToAccountDBID, const WCHAR *pwszFromCharacterName, const WCHAR *pwszChatMsg, short wChatLen)
{
	MAChat Chat;
	memset(&Chat, 0, sizeof(MAChat));

	Chat.nAccountDBID = nToAccountDBID;
	Chat.cType = cType;
	Chat.wChatLen = wChatLen;
	_wcscpy(Chat.wszFromCharacterName, NAMELENMAX, pwszFromCharacterName, (int)wcslen(pwszFromCharacterName));
	_wcscpy(Chat.wszChatMsg, CHATLENMAX, pwszChatMsg, (int)wcslen(pwszChatMsg));

	AddSendData(MAVI_CHAT, 0, (char*)&Chat, sizeof(Chat) - sizeof(Chat.wszChatMsg) + (wChatLen * 2));
}

void CDNVillageConnection::SendZoneChat(int nMapIdx, const WCHAR *pwszFromCharacterName, const WCHAR *pwszChatMsg, short wChatLen)
{
	MAZoneChat packet;
	memset(&packet, 0, sizeof(packet));

	packet.nMapIdx = nMapIdx;
	_wcscpy(packet.wszFromCharacterName, NAMELENMAX, pwszFromCharacterName, (int)wcslen(pwszFromCharacterName));
	packet.wChatLen = wChatLen;
	_wcscpy(packet.wszChatMsg, CHATLENMAX, pwszChatMsg, (int)wcslen(pwszChatMsg));

	AddSendData(MAVI_ZONECHAT, 0, (char*)&packet, sizeof(packet) - sizeof(packet.wszChatMsg) + (wChatLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendWorldSystemMsg(const WCHAR *pwszFromCharacterName, char cType, int nID, int nValue, WCHAR* pwszToCharacterName)
{
	MAWorldSystemMsg Msg;
	memset(&Msg, 0, sizeof(MAWorldSystemMsg));

	_wcscpy(Msg.wszFromCharacterName, NAMELENMAX, pwszFromCharacterName, (int)wcslen(pwszFromCharacterName));
	Msg.cType = cType;
	Msg.nID = nID;
	Msg.nValue = nValue;
	if (pwszToCharacterName) _wcscpy(Msg.wszToCharacterName, _countof(Msg.wszToCharacterName), pwszToCharacterName, (int)wcslen(pwszToCharacterName));

	AddSendData(MAVI_WORLDSYSTEMMSG, 0, (char*)&Msg, sizeof(MAWorldSystemMsg));
}

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNVillageConnection::SendPrivateChannelChat(const WCHAR *pwszFromCharacterName, const WCHAR *pwszChatMsg, short wChatLen, INT64 nChannelID)
{
	MAPrivateChannelChat Chat;
	memset(&Chat, 0, sizeof(MAPrivateChannelChat));
	
	Chat.cWorldSetID = g_Config.nWorldSetID;
	Chat.wChatLen = wChatLen;
	Chat.nChannelID = nChannelID;
	if(pwszFromCharacterName)
		_wcscpy(Chat.wszFromCharacterName, NAMELENMAX, pwszFromCharacterName, (int)wcslen(pwszFromCharacterName));
	if(pwszChatMsg)
		_wcscpy(Chat.wszChatMsg, CHATLENMAX, pwszChatMsg, (int)wcslen(pwszChatMsg));

	AddSendData(MAVI_PRIVATECHNNELCHAT, 0, (char*)&Chat, sizeof(Chat) - sizeof(Chat.wszChatMsg) + (wChatLen * 2));
}
#endif

void CDNVillageConnection::SendPvPCreateRoom( const short nRetCode, const CDNPvP* pPvPRoom, const VIMAPVP_CREATEROOM* pPacket )
{
	int nSize = 0;

	MAVIPVP_CREATEROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode				= nRetCode;
	TxPacket.uiCreateAccountDBID	= pPacket->uiCreateAccountDBID;

	if( nRetCode == ERROR_NONE )
	{
		if( pPvPRoom == NULL )
		{
			_DANGER_POINT();
			return;
		}

		TxPacket.uiPvPIndex			= pPvPRoom->GetIndex();
		TxPacket.nEventID			= pPvPRoom->GetEventRoomIndex();
		if( TxPacket.nEventID  > 0)
			memcpy( &TxPacket.EventData, const_cast<CDNPvP*>(pPvPRoom)->GetEventDataPtr(), sizeof(TEvent) );
		TxPacket.unVillageChannelID	= pPacket->unVillageChannelID;
		TxPacket.cGameMode			= pPacket->cGameMode;
#if defined( PRE_PVP_GAMBLEROOM )
		TxPacket.cGambleType = pPacket->cGambleType;
		TxPacket.nPrice = pPacket->nPrice;
#endif
		if (PvPCommon::GameMode::PvP_GuildWar == TxPacket.cGameMode && pPvPRoom->GetIsGuildWarSystem())
		{
			for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
				TxPacket.nGuildWarDBID[i] = pPvPRoom->GetGuildDBIDbyIdx(i);
		}
		memcpy( &TxPacket.sCSPVP_CREATEROOM, &pPacket->sCSPVP_CREATEROOM, sizeof(CSPVP_CREATEROOM)-sizeof(pPacket->sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->sCSPVP_CREATEROOM.cRoomNameLen+pPacket->sCSPVP_CREATEROOM.cRoomPWLen) );

#ifdef PRE_MOD_PVPOBSERVER
		TxPacket.sCSPVP_CREATEROOM.bExtendObserver = pPvPRoom->bIsExtendObserver();
#endif		//#ifdef PRE_MOD_PVPOBSERVER

		nSize = sizeof(MAVIPVP_CREATEROOM)-sizeof(pPacket->sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->sCSPVP_CREATEROOM.cRoomNameLen+pPacket->sCSPVP_CREATEROOM.cRoomPWLen);
	}
	else
	{
		nSize = sizeof(short)+sizeof(UINT);
	}

	AddSendData( MAVI_PVP_CREATEROOM, 0, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNVillageConnection::SendPvPModifyRoom( const short nRetCode, const CDNPvP* pPvPRoom, const VIMAPVP_MODIFYROOM* pPacket )
{
	int nSize = 0;

	MAVIPVP_MODIFYROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode		= nRetCode;
	TxPacket.uiAccountDBID	= pPacket->uiAccountDBID;

	if( nRetCode == ERROR_NONE )
	{
		if( pPvPRoom == NULL )
		{
			_DANGER_POINT();
			return;
		}

		TxPacket.uiPvPIndex			= pPvPRoom->GetIndex();
		TxPacket.unVillageChannelID	= pPvPRoom->GetVillageChannelID();
		TxPacket.cGameMode			= pPacket->cGameMode;		
		memcpy( &TxPacket.sCSPVP_MODIFYROOM, &pPacket->sCSPVP_MODIFYROOM, sizeof(CSPVP_MODIFYROOM)-sizeof(pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomNameLen+pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomPWLen) );

		nSize = sizeof(MAVIPVP_CREATEROOM)-sizeof(pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.wszBuf)+sizeof(WCHAR)*(pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomNameLen+pPacket->sCSPVP_MODIFYROOM.sCSPVP_CREATEROOM.cRoomPWLen);
	}
	else
	{
		nSize = sizeof(short)+sizeof(UINT);
	}

	AddSendData( MAVI_PVP_MODIFYROOM, 0, reinterpret_cast<char*>(&TxPacket), nSize );
}

void CDNVillageConnection::SendPvPDestroyRoom( const USHORT unVillageChannelID, const UINT uiPvPIndex )
{
	MAVIPVP_DESTROYROOM	TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;

	AddSendData( MAVI_PVP_DESTROYROOM, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPLeaveRoom( const short nRetCode, const USHORT unVillageChannelID, const UINT uiPvPIndex, const UINT uiLeaveAccountDBID, PvPCommon::LeaveType::eLeaveType Type )
{
	MAVIPVP_LEAVEROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode			= nRetCode;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.uiLeaveAccountDBID	= uiLeaveAccountDBID;
	TxPacket.eType				= Type;

	AddSendData( MAVI_PVP_LEAVEROOM, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPRoomList( const VIMAPVP_ROOMLIST* pPacket )
{
	MAVIPVP_ROOMLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= pPacket->uiAccountDBID;
	TxPacket.unVillageChannelID	= pPacket->unVillageChannelID;
	memcpy( &TxPacket.sCSPVP_ROOMLIST, &pPacket->sCSPVP_ROOMLIST, sizeof(CSPVP_ROOMLIST) );

	AddSendData( MAVI_PVP_ROOMLIST, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPWaitUserList( const VIMAPVP_WAITUSERLIST* pPacket )
{
	MAVIPVP_WAITUSERLIST TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= pPacket->uiAccountDBID;
	TxPacket.unVillageChannelID	= pPacket->unVillageChannelID;
	memcpy( &TxPacket.sCSPVP_WAITUSERLIST, &pPacket->sCSPVP_WAITUSERLIST, sizeof(CSPVP_WAITUSERLIST) );

	AddSendData( MAVI_PVP_WAITUSERLIST, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );	
}

void CDNVillageConnection::SendPvPJoinRoom( const short nRetCode, const UINT uiAccountDBID, const USHORT unVillageChannelID/*=0*/, const UINT uiPvPIndex/*=0*/, const USHORT unPvPTeam/*=PvPCommon::Team::Max*/, const UINT uiUserState/*=PvPCommon::UserState::None*/, char cIndex /*= -1*/ )
{
	MAVIPVP_JOINROOM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode			= nRetCode;
	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.unPvPTeam			= unPvPTeam;
	TxPacket.uiUserState		= uiUserState;
	TxPacket.cIndex = cIndex;

	AddSendData( MAVI_PVP_JOINROOM, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}


void CDNVillageConnection::SendPvPRoomState( const USHORT unVillageChannelID, const UINT uiPvPIndex, const UINT uiRoomState )
{
	MAVIPVP_ROOMSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.uiRoomState		= uiRoomState;

	AddSendData( MAVI_PVP_ROOMSTATE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPChangeTeam( const short nRetCode, const USHORT unVillageChannelID, const UINT uiPvPIndex, const VIMAPVP_CHANGETEAM* pPacket, char cTeamSlotIndex/* = -1*/ )
{
	MAVIPVP_CHANGETEAM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode			= nRetCode;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	memcpy( &TxPacket.sVIMAPVP_CHANGETEAM, pPacket, sizeof(VIMAPVP_CHANGETEAM) );
	TxPacket.cTeamSlotIndex = cTeamSlotIndex;

	AddSendData( MAVI_PVP_CHANGETEAM, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPChangeUserState( const USHORT unVillageChannelID, const UINT uiPvPIndex, const UINT uiAccountDBID, const UINT uiUserState )
{
	MAVIPVP_CHANGEUSERSTATE TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.uiUserState		= uiUserState;

	AddSendData( MAVI_PVP_CHANGEUSERSTATE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPStart( const short nRetCode, const USHORT unVillageChannelID, const UINT uiPvPIndex, const UINT uiAccountDBID )
{
	MAVIPVP_START TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRetCode			= nRetCode;
	TxPacket.unVillageChannelID	= unVillageChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.uiAccountDBID		= uiAccountDBID;

	AddSendData( MAVI_PVP_START, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPStartMsg( const USHORT unVilalgeChannelID, const UINT uiPvPIndex, const BYTE cSec )
{
	MAVIPVP_STARTMSG TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.unVillageChannelID	= unVilalgeChannelID;
	TxPacket.uiPvPIndex			= uiPvPIndex;
	TxPacket.cSec				= cSec;

	AddSendData( MAVI_PVP_STARTMSG, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPRoomInfo( CDNUser* pUser, CDNPvP* pPvP )
{
	MAVIPVP_ROOMINFO TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= pUser->GetAccountDBID();
	TxPacket.uiIndex			= pPvP->GetIndex();
	TxPacket.uiWinCondition		= pPvP->GetGameModeWinCondition();
	TxPacket.uiGameModeTableID	= pPvP->GetGameModeTableID();
	TxPacket.uiPlayTimeSec		= pPvP->GetGameModePlayTimeSec();
	TxPacket.cMaxUser			= static_cast<BYTE>(pPvP->GetMaxUser());
	TxPacket.bIsGuildWar = pPvP->GetIsGuildWarSystem();
#if defined( PRE_PVP_GAMBLEROOM )
	TxPacket.cGambleType = pPvP->GetGambleRoomType();
	TxPacket.nGamblePrice = pPvP->GetGambleRoomPrice();
#endif
	
	AddSendData( MAVI_PVP_ROOMINFO, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendPvPMemberGrade(UINT nAccountDBID, USHORT nTeam, UINT uiUserState, UINT nChangedSessionID, int nRetCode)
{
	MAVIPvPMemberGrade packet;
	memset(&packet, 0, sizeof(MAVIPvPMemberGrade));

	packet.nAccountDBID = nAccountDBID;
	packet.nTeam = nTeam;
	packet.uiUserState = uiUserState;
	packet.nChangedSessionID = nChangedSessionID;
	packet.nRetCode = nRetCode;

	AddSendData(MAVI_PVP_MEMBERGRADE, 0, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNVillageConnection::SendPvPMemberIndex(USHORT nTeam, const VIMAPVPSwapMemberIndex * pIndex, int nRetCode)
{
	MAVIPvPMemberIndex packet;
	memset(&packet, 0, sizeof(MAVIPvPMemberIndex));

	packet.nAccountDBID = pIndex->nAccountDBID;
	packet.nRetCode = nRetCode;
	packet.nTeam = nTeam;
	packet.cCount = pIndex->cCount;
	memcpy(packet.Index, pIndex->Index, sizeof(packet.Index));

	AddSendData(MAVI_PVP_MEMBERINDEX, 0, reinterpret_cast<char*>(&packet), sizeof(packet) - sizeof(packet.Index) + (sizeof(TSwapMemberIndex) * packet.cCount));
}

#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
void CDNVillageConnection::SendPvPRoomListRelay(const BYTE cVillageID,  const VIMAPVP_ROOMLIST* pPacket)
{
	MAVIPVP_ROOMLIST_RELAY packet;
	memset(&packet, 0, sizeof(MAVIPVP_ROOMLIST_RELAY));

	packet.cVillageID = cVillageID;
	packet.uiAccountDBID = pPacket->uiAccountDBID;
	memcpy(&packet.sRoomList, pPacket, sizeof(packet.sRoomList));

	AddSendData(MAVI_PVP_ROOMLIST_RELAY, 0, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNVillageConnection::SendPvPRoomListRelayAck(PVP_ROOMLIST_RELAY* pPacket)
{
	AddSendData(MAVI_PVP_ROOMLIST_RELAY_ACK, 0, reinterpret_cast<char*>(pPacket), sizeof(PVP_ROOMLIST_RELAY)-sizeof(pPacket->sSCPVP_ROOMLIST)+pPacket->nSize);
}
#endif

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
void CDNVillageConnection::SendPvPChangeChannelResult(UINT nAccountDBID, BYTE cType, int nRetCode)
{
	MAVIPVP_CHANGECHANNEL packet;
	memset(&packet, 0, sizeof(MAVIPVP_CHANGECHANNEL));

	packet.nAccountDBID = nAccountDBID;
	packet.cType = cType;
	packet.nRetCode = nRetCode;

	AddSendData(MAVI_PVP_CHANGECHANNEL, 0, reinterpret_cast<char*>(&packet), sizeof(packet));
}
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER

#if defined(PRE_ADD_PVP_TOURNAMENT)
void CDNVillageConnection::SendPvPSwapTournamentIndex( const short nRetCode, const UINT nAccounDBID, const char cSourceIndex, const char cDestIndex)
{
	MAVIPVPSwapTournamentIndex packet;
	memset(&packet, 0, sizeof(MAVIPVPSwapTournamentIndex));

	packet.nRetCode = nRetCode;
	packet.nAccountDBID = nAccounDBID;	
	packet.cSourceIndex = cSourceIndex;
	packet.cDestIndex = cDestIndex;

	AddSendData(MAVI_PVP_SWAP_TOURNAMENT_INDEX, 0, reinterpret_cast<char*>(&packet), sizeof(packet));
}

void CDNVillageConnection::SendPvPShuffleTournamentIndex(CDNPvP* pPvPRoom)
{
	MAVIPVPShuffleTournamentIndex packet;
	memset(&packet, 0, sizeof(MAVIPVPShuffleTournamentIndex));
	
	packet.uiPvPIndex			= pPvPRoom->GetIndex();
	packet.unVillageChannelID	= pPvPRoom->GetVillageChannelID();

	packet.nCount = pPvPRoom->GetMaxUser();
	memcpy(packet.uiAccountDBID, pPvPRoom->GetPvPTournamentIndexAll(), sizeof(UINT)*packet.nCount);

	int nLen = sizeof(MAVIPVPShuffleTournamentIndex) - sizeof(packet.uiAccountDBID) + (sizeof(UINT)*packet.nCount);

	AddSendData(MAVI_PVP_SHUFFLE_TOURNAMENT_INDEX, 0, reinterpret_cast<char*>(&packet), nLen);
}
#endif

void CDNVillageConnection::SendLadderSystemDelUser( CDNUser* pUser )
{
	LadderSystem::MAVIDelUser TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	_wcscpy( TxPacket.wszCharName, NAMELENMAX, pUser->GetCharacterName(), (int)wcslen(pUser->GetCharacterName()) );
	TxPacket.biCharDBID = pUser->GetCharacterDBID();

	AddSendData( MAVI_LADDERSYSTEM_DELUSER, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendLadderSystemRoomSync( GAMAPVP_LADDERROOMSYNC* pPacket )
{
	LadderSystem::MAVIRoomSync TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.biRoomIndexArr[0]	= pPacket->biRoomIndexArr[0];
	TxPacket.biRoomIndexArr[1]	= pPacket->biRoomIndexArr[1];
	TxPacket.State				= pPacket->State;

	AddSendData( MAVI_LADDERSYSTEM_ROOMSYNC, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

// MasterSystem
void CDNVillageConnection::SendMasterSystemCheckMasterApplication( short nRet, UINT uiPupilAccountDBID, INT64 biMasterCharacterDBID )
{
	MasterSystem::MAVICheckMasterApplication TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.nRet					= nRet;
	TxPacket.uiPupilAccountDBID		= uiPupilAccountDBID;
	TxPacket.biMasterCharacterDBID	= biMasterCharacterDBID;

	AddSendData( MAVI_MASTERSYSTEM_CHECK_MASTERAPPLICATION, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemSyncSimpleInfo( UINT uiAccountDBID, INT64 biCharacterDBID, MasterSystem::EventType::eCode EventCode )
{
	MasterSystem::MAVISyncSimpleInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.EventCode			= EventCode;

	AddSendData( MAVI_MASTERSYSTEM_SYNC_SIMPLEINFO, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemSyncJoin( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsAddPupil )
{
	MasterSystem::MAVISyncJoin TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.bIsAddPupil		= bIsAddPupil;

	AddSendData( MAVI_MASTERSYSTEM_SYNC_JOIN, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemSyncLeave( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsDelPupil )
{
	MasterSystem::MAVISyncLeave TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.bIsDelPupil		= bIsDelPupil;

	AddSendData( MAVI_MASTERSYSTEM_SYNC_LEAVE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemSyncGraduate( UINT uiAccountDBID, MasterSystem::VIMASyncGraduate* pPacket )
{
	MasterSystem::MAVISyncGraduate TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID = uiAccountDBID;
	_wcscpy( TxPacket.wszCharName, NAMELENMAX, pPacket->wszCharName, (int)wcslen(pPacket->wszCharName) );

	AddSendData( MAVI_MASTERSYSTEM_SYNC_GRADUATE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemSyncConnect( UINT uiAccountDBID, bool bIsConnect, WCHAR* pwszCharName )
{
	MasterSystem::MAVISyncConnect TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID	= uiAccountDBID;
	TxPacket.bIsConnect		= bIsConnect;
	_wcscpy( TxPacket.wszCharName, NAMELENMAX, pwszCharName, (int)wcslen(pwszCharName) );

	AddSendData( MAVI_MASTERSYSTEM_SYNC_CONNECT, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemCheckLeave( int iRet, UINT uiAccountDBID, INT64 biDestCharacterDBID, bool bIsMaster )
{
	MasterSystem::MAVICheckLeave TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet					= iRet;
	TxPacket.uiAccountDBID			= uiAccountDBID;
	TxPacket.biDestCharacterDBID	= biDestCharacterDBID;
	TxPacket.bIsMaster				= bIsMaster;

	AddSendData( MAVI_MASTERSYSTEM_CHECK_LEAVE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemRecallMaster( int iRet, UINT uiAccountDBID, WCHAR* pwszCharName, bool bIsConfirm )
{
	MasterSystem::MAVIRecallMaster TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet			= iRet;
	TxPacket.uiAccountDBID	= uiAccountDBID;
	TxPacket.bIsConfirm		= bIsConfirm;
	_wcscpy( TxPacket.wszCharName, NAMELENMAX, pwszCharName, (int)wcslen(pwszCharName) );

	AddSendData( MAVI_MASTERSYSTEM_RECALL_MASTER, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemJoinConfirm( const UINT uiMasterAccountDBID, const BYTE cLevel, const BYTE cJob, WCHAR* pwszPupilCharName)
{	
	MasterSystem::MAVIJoinConfirm TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );
	
	TxPacket.uiMasterAccountDBID	= uiMasterAccountDBID;
	TxPacket.cLevel = cLevel;
	TxPacket.cJob = cJob;
	_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), pwszPupilCharName, (int)wcslen(pwszPupilCharName) );

	AddSendData( MAVI_MASTERSYSTEM_JOIN_CONFIRM, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendMasterSystemJoinConfirmResult( const int iRet, const bool bIsAccept, const INT64 biMasterCharacterDBID, WCHAR* pwszPupilCharName)
{
	MasterSystem::MAVIJoinResult TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.biMasterCharacterDBID = biMasterCharacterDBID;
	TxPacket.iRet = iRet;
	TxPacket.bIsAccept = bIsAccept;	
	_wcscpy( TxPacket.wszPupilCharName, _countof(TxPacket.wszPupilCharName), pwszPupilCharName, (int)wcslen(pwszPupilCharName) );

	AddSendData( MAVI_MASTERSYSTEM_JOIN_CONFIRM_RESULT, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNVillageConnection::SendSyncSystemMail( UINT uiAccountDBID, VIMASyncSystemMail* pMail )
{
	MAVISyncSystemMail TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID			= uiAccountDBID;
	TxPacket.nMailDBID				= pMail->nMailDBID;
	TxPacket.iTotalMailCount		= pMail->iTotalMailCount;
	TxPacket.iNotReadMailCount		= pMail->iNotReadMailCount;
	TxPacket.i7DaysLeftMailCount	= pMail->i7DaysLeftMailCount;

	AddSendData( MAVI_SYNC_SYSTEMMAIL, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

// Cheat-related
void CDNVillageConnection::SendResRecall( CDNUser* pRecallUser, const VIMAReqRecall* pPacket )
{
	MAVIResRecall TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID			= pPacket->uiAccountDBID;
	TxPacket.uiRecallAccountDBID	= pRecallUser->GetAccountDBID();
	TxPacket.uiRecallChannelID		= pPacket->uiRecallChannelID;

	AddSendData( MAVI_RESRECALL, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendVillageTrace( CDNUser* pTargetUser, UINT uiAccountDBID, bool bIsGMCall, char cSelectedLang )
{
	MAVIVillageTrace TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID			= uiAccountDBID;
	TxPacket.uiTargetAccountDBID	= pTargetUser->GetAccountDBID();
	TxPacket.iTargetVillageID		= pTargetUser->GetVillageID();
	TxPacket.iTargetChannelID		= pTargetUser->GetChannelID();
	TxPacket.bIsGMCall				= bIsGMCall;
 
	AddSendData( MAVI_VILLAGETRACE, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendUserRestraint(UINT nAccountDBID)
{
	MARestraint packet;
	memset(&packet, 0, sizeof(MARestraint));

	packet.nAccountDBID = nAccountDBID;

	AddSendData(MAVI_RESTRAINT, 0, reinterpret_cast<char*>(&packet), sizeof(packet));	
}

//VoiceChat
void CDNVillageConnection::SendVoiceChannelID(TINSTANCEID InstanceID, int nVoiceChannelType, int nPvPLobbyChannelID, UINT *nVoiceChannelID)
{
	MAVIVoiceChannelID packet;
	memset(&packet, 0, sizeof(MAVIVoiceChannelID));

	packet.InstanceID = InstanceID;
	packet.cType = (BYTE)nVoiceChannelType;
	packet.nPvPLobbyChannelID = nPvPLobbyChannelID;
	for (int i = 0; i < PvPCommon::TeamIndex::Max; i++)
	{
		if (nVoiceChannelID[i] <= 0) continue;
		packet.nVoiceChannelD[i] = nVoiceChannelID[i];
	}

	AddSendData(MAVI_VOICECHANNELID, 0, (char*)&packet, sizeof(packet));
}

void CDNVillageConnection::SendFarmSync( UINT uiAccountDBID, INT64 biCharacterDBID, Farm::ServerSyncType::eType Type )
{
	MAVIFarmSync TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.Type				= Type;

	AddSendData( MAVI_FARM_SYNC, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNVillageConnection::SendNotice(const WCHAR * pMsg, const int nLen, int nSec)
{
	MANotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.Info.nNoticeType = _NOTICETYPE_WORLD;
	packet.Info.nSlideShowSec = nSec;
	_wcscpy(packet.wszNoticeMsg, CHATLENMAX, pMsg, (int)wcslen(pMsg));
	packet.nLen = nLen;

	AddSendData(MAVI_NOTICE, 0, (char*)&packet, sizeof(MANotice) - sizeof(packet.wszNoticeMsg) + (packet.nLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendNoticeZone(int nMapIndex, const WCHAR * pMsg, const int nLen, int nSec)
{
	MANotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.Info.nNoticeType = _NOTICETYPE_ZONE;
	packet.Info.nMapIdx = nMapIndex;
	packet.Info.nSlideShowSec = nSec;

	_wcscpy(packet.wszNoticeMsg, CHATLENMAX, pMsg, (int)wcslen(pMsg));
	packet.nLen = nLen;

	AddSendData(MAVI_NOTICE, 0, (char*)&packet, sizeof(MANotice) - sizeof(packet.wszNoticeMsg) + (packet.nLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendNoticeChannel(int nChannelID, const WCHAR * pMsg, int nLen, int nSec)
{
	MANotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.Info.nNoticeType = _NOTICETYPE_CHANNEL;
	packet.Info.nChannelID = nChannelID;
	packet.Info.nSlideShowSec = nSec;

	_wcscpy(packet.wszNoticeMsg, CHATLENMAX, pMsg, (int)wcslen(pMsg));
	packet.nLen = nLen;

	AddSendData(MAVI_NOTICE, 0, (char*)&packet, sizeof(MANotice) - sizeof(packet.wszNoticeMsg) + (packet.nLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendNoticeServer(int nMID, const WCHAR * pMsg, int nLen, int nSec)
{
	MANotice packet;
	memset(&packet, 0, sizeof(packet));

	packet.Info.nNoticeType = _NOTICETYPE_EACHSERVER;
	packet.Info.nManagedID = nMID;
	packet.Info.nSlideShowSec = nSec;

	_wcscpy(packet.wszNoticeMsg, _countof(packet.wszNoticeMsg), pMsg, (int)wcslen(pMsg));
	packet.nLen = nLen;

	AddSendData(MAVI_NOTICE, 0, (char*)&packet, sizeof(MANotice) - sizeof(packet.wszNoticeMsg) + (packet.nLen * sizeof(WCHAR)));
}

void CDNVillageConnection::SendNoticeCancel()
{
	AddSendData(MAVI_CANCELNOTICE, 0, NULL, 0);
}

void CDNVillageConnection::SendDetachUser(UINT nAccountDBID, bool bIsDuplicate, UINT nSessionID)
{
	MADetachUser Detach;
	memset(&Detach, 0, sizeof(MADetachUser));

	Detach.nAccountDBID = nAccountDBID;
	Detach.bIsDuplicate = bIsDuplicate;
	Detach.nSessionID = nSessionID;

	AddSendData(MAVI_DETACHUSER, 0, (char*)&Detach, sizeof(MADetachUser));
}


void CDNVillageConnection::SendNotifyMail(UINT nToAccountDBID, INT64 biToCharacterDBID, short wTotalMailCount, short wNotReadMailCount, short w7DaysLeftCount, bool bNewMail)
{
	MANotifyMail Mail = { 0, };
	Mail.nToAccountDBID = nToAccountDBID;
	Mail.biToCharacterDBID = biToCharacterDBID;
	Mail.wTotalMailCount = wTotalMailCount;
	Mail.wNotReadMailCount = wNotReadMailCount;
	Mail.w7DaysLeftCount = w7DaysLeftCount;
	Mail.bNewMail = bNewMail;
	AddSendData(MAVI_NOTIFYMAIL, 0, (char*)&Mail, sizeof(MANotifyMail));
}

void CDNVillageConnection::SendNotifyMarket(UINT nSellerAccountDBID, INT64 biSellerCharacterDBID, int nItemID, short wCalculationCount)
{
	MANotifyMarket Market = { 0, };
	Market.nSellerAccountDBID = nSellerAccountDBID;
	Market.biSellerCharacterDBID = biSellerCharacterDBID;
	Market.nItemID = nItemID;
	Market.wCalculationCount = wCalculationCount;
	AddSendData(MAVI_NOTIFYMARKET, 0, (char*)&Market, sizeof(MANotifyMarket));
}

void CDNVillageConnection::SendNotifyGift(UINT nToAccountDBID, INT64 biToCharacterDBID, bool bNew, int nGiftCount)
{
	MANotifyGift Gift = { 0, };
	Gift.nToAccountDBID = nToAccountDBID;
	Gift.biToCharacterDBID = biToCharacterDBID;
	Gift.bNew = bNew;
	Gift.nGiftCount = nGiftCount;
	AddSendData(MAVI_NOTIFYGIFT, 0, (char*)&Gift, sizeof(MANotifyGift));
}

void CDNVillageConnection::SendPCBangResult(UINT nAccountDBID, CDNUser *pUser)
{
	MAPCBangResult Result;
	memset(&Result, 0, sizeof(MAPCBangResult));

	Result.nAccountDBID = nAccountDBID;
	if (pUser){

		Result.cPCBangGrade = pUser->GetPCBangGrade();

	}

	AddSendData(MAVI_PCBANGRESULT, 0, (char*)&Result, sizeof(MAPCBangResult));
}

#if defined(PRE_ADD_REMOTE_QUEST)
void CDNVillageConnection::SendUserTempDataResult(UINT uiAccountDBID, CDNUser *pUser)
{
	if(!pUser) return;
	MAVIUserTempDataResult packet;
	memset(&packet, 0, sizeof(packet));

	packet.uiAccountDBID = uiAccountDBID;
	packet.nDungeonClearCount = pUser->GetDungeonClearCount();
#if defined(PRE_ADD_REMOTE_QUEST)
	pUser->GetAcceptWaitRemoteQuest(&packet.nAcceptWaitRemoteQuestCount, packet.AcceptWaitRemoteQuestList);
#endif
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	packet.eUserGameQuitRewardType = pUser->GetUserGameQuitRewardType();
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

	AddSendData(MAVI_USERTEMPDATA_RESULT, 0, (char*)&packet, sizeof(MAVIUserTempDataResult) - sizeof(packet.AcceptWaitRemoteQuestList) + sizeof(int) * packet.nAcceptWaitRemoteQuestCount );
}
#else	//#if defined(PRE_ADD_REMOTE_QUEST)
#if defined(PRE_ADD_GAMEQUIT_REWARD)
void CDNVillageConnection::SendUserTempDataResult(UINT uiAccountDBID, int nDungeonClearCount, GameQuitReward::RewardType::eType eUserGameQuitRewardType)
#else	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
void CDNVillageConnection::SendUserTempDataResult(UINT uiAccountDBID, int nDungeonClearCount)
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
{
	MAVIUserTempDataResult packet;
	memset(&packet, 0, sizeof(packet));

	packet.uiAccountDBID = uiAccountDBID;
	packet.nDungeonClearCount = nDungeonClearCount;
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	packet.eUserGameQuitRewardType = cUserGameQuitRewardType;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	AddSendData(MAVI_USERTEMPDATA_RESULT, 0, (char*)&packet, sizeof(MAGAUserTempDataResult));
}
#endif	//#if defined(PRE_ADD_REMOTE_QUEST)

void CDNVillageConnection::SendCheckLastDungeonInfo( UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsCheck, const WCHAR* pwszPartyName )
{
	MAVICheckLastDungeonInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.uiAccountDBID		= uiAccountDBID;
	TxPacket.biCharacterDBID	= biCharacterDBID;
	TxPacket.bIsCheck			= bIsCheck;
	if( pwszPartyName )
		_wcscpy( TxPacket.wszPartyName, _countof(TxPacket.wszPartyName), pwszPartyName, static_cast<int>(wcslen(pwszPartyName)) );

	AddSendData( MAVI_CHECK_LASTDUNGEONINFO, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined( PRE_PARTY_DB )
void CDNVillageConnection::SendConfirmLastDungeonInfo( int iRet, UINT uiAccountDBID, BreakInto::Type::eCode BreakIntoType )
#else
void CDNVillageConnection::SendConfirmLastDungeonInfo( int iRet, UINT uiAccountDBID )
#endif // #if defined( PRE_PARTY_DB )
{
	MAVIConfirmLastDungeonInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet				= iRet;
	TxPacket.uiAccountDBID		= uiAccountDBID;
#if defined( PRE_PARTY_DB )
	TxPacket.BreakIntoType		= BreakIntoType;
#endif // #if defined( PRE_PARTY_DB )

	AddSendData( MAVI_CONFIRM_LASTDUNGEONINFO, 0, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

#if defined(PRE_ADD_QUICK_PVP)
void CDNVillageConnection::SendMakeQuickPvPRoom(int nRet, UINT nMasterAccountDBID, UINT nSlaveAccountDBID)
{
	MAVIMakeQuickPvPRoom Packet;
	memset(&Packet, 0, sizeof(MAVIMakeQuickPvPRoom));

	Packet.nRet = nRet;
	Packet.uiMasterAccountDBID = nMasterAccountDBID;
	Packet.uiSlaveAccountDBID = nSlaveAccountDBID;

	AddSendData(MAVI_MAKEQUICKPVP_ROOM, 0, reinterpret_cast<char*>(&Packet), sizeof(Packet));
}
#endif

#if defined (PRE_ADD_BESTFRIEND)
void CDNVillageConnection::SendSearchBestFriend(MASearchBestFriend* pPacket)
{
	AddSendData(MAVI_SEARCH_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendRegistBestFriend(MARegistBestFriend* pPacket)
{
	AddSendData(MAVI_REGIST_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendRegistBestFriendResult(MARegistBestFriendResult* pPacket)
{
	AddSendData(MAVI_REGIST_BESTFRIEND_RETMSG, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendCompleteBestFriend(MACompleteBestFriend* pPacket)
{
	AddSendData(MAVI_COMPLETE_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendEditBestFriendMemo(MAEditBestFriendMemo* pPacket)
{
	AddSendData(MAVI_EDIT_BESTFRIENDMEMO, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendCancelBestFriend(MACancelBestFriend* pPacket)
{
	AddSendData(MAVI_CANCEL_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendCloseBestFriend(MACloseBestFriend* pPacket)
{
	AddSendData(MAVI_CLOSE_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
void CDNVillageConnection::SendLevelBestFriend(MALevelUpBestFriend* pPacket)
{
	AddSendData(MAVI_LEVELUP_BESTFRIEND, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif

#if defined( PRE_WORLDCOMBINE_PARTY )

void CDNVillageConnection::SendDelWorldParty(MADelWorldParty *pPacket)
{
	AddSendData(MAVI_DELWORLDPARTY, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendWorldPartyMember(GAMASendWorldPartyMember *pPacket)
{
	AddSendData(MAVI_WORLDPARTYMEMBER, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNVillageConnection::SendPrivateChatChannelAdd(MAAddPrivateChannel* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_ADD, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMemberAdd(MAAddPrivateChannelMember* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MEMBERADD, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMemberInvite(MAInvitePrivateChannelMember* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MEMBERINVITE, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMemberInviteResult(MAInvitePrivateChannelMemberResult* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MEMBERINVITERESULT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMemberDel(MADelPrivateChannelMember* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MEMBERDEL, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMemberKickResult(MAKickPrivateChannelMemberResult* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MEMBERKICKRESULT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelMod(MAModPrivateChannel* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MOD, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendPrivateChatChannelModMemberName(MAModPrivateChannelMemberName* pPacket)
{
	AddSendData(MAVI_PRIVATECHATCHANNEL_MODMEMBERNAME, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )
void CDNVillageConnection::SendDelWorldPvPRoom( MADelWorldPvPRoom *pPacket )
{
	AddSendData(MAVI_DEL_WORLDPVPROOM, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendWorldPvPRoomJoinResult( MAWorldPvPRoomJoinResult *pPacket )
{
	AddSendData(MAVI_WORLDPVPROOM_JOINRESULT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}

void CDNVillageConnection::SendWorldPvPRoomGMCreateResult( MAWorldPvPRoomGMCreateResult *pPacket )
{
	AddSendData(MAVI_WORLDPVPROOM_GMCRETERESULT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
void CDNVillageConnection::SendMuteUserChatting(MAMuteUserChat *pPacket)
{
	AddSendData(MAVI_MUTE_USERCHAT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)

#if defined( PRE_ALTEIAWORLD_EXPLORE )
void CDNVillageConnection::SendAlteiaWorldSendTicket( MAAlteiaWorldSendTicket *pPacket )
{
	AddSendData(MAVI_ALTEIAWORLD_SENDTICKET, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
void CDNVillageConnection::SendAlteiaWorldSendTicketResult( MAAlteiaWorldSendTicketResult *pPacket )
{
	AddSendData(MAVI_ALTEIAWORLD_SENDTICKET_RESULT, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif

#if defined( PRE_DRAGONBUFF )
void CDNVillageConnection::SendApplyWorldBuff( MAApplyWorldBuff *pPacket )
{
	AddSendData(MAVI_APPLYWORLDBUFF, 0, reinterpret_cast<char*>(pPacket), sizeof((*pPacket)));
}
#endif


#if defined( PRE_PVP_GAMBLEROOM )
void CDNVillageConnection::SendGambleRoomCreate( int nTotalGambleRoomCount )
{
	MACreateGambleRoom Packet = { 0, };	
	Packet.nTotalGambleRoomCount = nTotalGambleRoomCount;
	AddSendData(MAVI_GAMBLEROOM_CREATE, 0, (char*)&Packet, sizeof(Packet));
}
#endif
