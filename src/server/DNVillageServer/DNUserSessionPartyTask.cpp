
#include "stdafx.h"
#include "DNUserSession.h"
#include "DNMasterConnection.h"
#include "DNDBConnection.h"
#include "DNPartyManager.h"
#include "DNUserSessionManager.h"
#include "DNFieldDataManager.h"
#include "DNField.h"
#include "DNGameDataManager.h"
#include "DNFarm.h"
#include "DNIsolate.h"
#include "EtUIXML.h"
#include "DNGuildWarManager.h"
#if defined( PRE_PARTY_DB )
#include "DNPartySortObject.hpp"
#endif // #if defined( PRE_PARTY_DB )

extern TVillageConfig g_Config;

int CDNUserSession::OnRecvPartyMessage(int nSubCmd, char *pData, int nLen)
{
	switch(nSubCmd)
	{
		case CS_CREATEPARTY:
		{
			int iRet = _RecvPartyCreate( reinterpret_cast<const CSCreateParty*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				{
					return iRet;
				}
			case ERROR_PARTY_CREATE_FAIL:
			case ERROR_PARTY_CREATE_FAIL_NEED_PARTYNAME:
			case ERROR_PARTY_CREATE_FAIL_PASSWORD_ERR:
			case ERROR_PARTY_CREATE_FAIL_RESTRICT_LEVEL_ERROR:
				{
					SendCreateParty( iRet, 0 );
					return ERROR_GENERIC_INVALIDREQUEST;
				}
			}
			break;
		}
	case CS_JOINPARTY:
		{
			int iRet = _RecvPartyJoin( reinterpret_cast<const CSJoinParty*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				{
					return iRet;
				}
			case ERROR_PARTY_JOINFAIL:
			case ERROR_PARTY_JOINFAIL_INREADYSTATE:
			case ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_SHORTAGE:
			case ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_OVER:
			case ERROR_PARTY_JOINFAIL_PASSWORDINCORRECT:
			case ERROR_PARTY_JOINFAIL_YOU_ARE_KICKEDUSER:
				{
					SendJoinParty( iRet, 0, NULL );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYOUT:
		{
			int iRet = _RecvPartyOut( nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				{
					return iRet;
				}
			case ERROR_PARTY_LEAVEFAIL:
				{
					SendPartyOut( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYLISTINFO:
		{
			int iRet = _RecvPartyListInfo( reinterpret_cast<const CSPartyListInfo*>(pData), nLen );
			switch( iRet )
			{
				case ERROR_NONE:
				case ERROR_INVALIDPACKET:
				{
					return iRet;
				}
				case ERROR_PARTY_LIST_ERROR:
#if defined( PRE_PARTY_DB )
				case ERROR_PARTY_REFRESHLIST_TIMERESTRAINT:
#endif // #if defined( PRE_PARTY_DB )
				{
					SendPartyInfoErr( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_REFRESHGATEINFO:
		{
			int iRet = _RecvPartyRefreshGateInfo( reinterpret_cast<const CSRefreshGateInfo*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
			case ERROR_GENERIC_UNKNOWNERROR:
				return iRet;
			}
			break;
		}
	case CS_STARTSTAGE:
		{
			int iRet = _RecvPartyStartStage( reinterpret_cast<const CSStartStage*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_STARTSTAGE_FAIL:
			case ERROR_CASHSHOP_OPEN:
			case ERROR_GENERIC_MASTERCON_NOT_FOUND:
			case ERROR_FARM_INVALID_VIP:
			case ERROR_FARM_ENTERANCE_CONDITION:
				{
					SendCancelStage( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}

	case CS_CANCELSTAGE:
		{
			int iRet = _RecvPartyCancelStage( nLen );
			switch( iRet )			
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			}
			break;
		}

	case CS_SELECTSTAGE:
		{
			int iRet = _RecvPartySelectStage( reinterpret_cast<const CSSelectStage*>(pData), nLen );
			switch( iRet )			
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			}
			break;
		}

	case CS_PARTYINVITE:
		{
			int iRet = _RecvPartyInvite( reinterpret_cast<const CSInvitePartyMember*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_INVITEFAIL:
			case ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND:
			case ERROR_PARTY_INVITEFAIL_DEST_USER_NOWREADY:
			case ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED:
			case ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME:
			case ERROR_PARTY_INVITEFAIL_ALREADY_HAS_PARTY:
			case ERROR_PARTY_INVITEFAIL_CHARACTERLEVEL_SHORTAGE:
			case ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_OVER:
			case ERROR_PARTY_INVITEFAIL_ALREADY_INVITED:
				{
					SendPartyInviteFail( iRet );
					return ERROR_NONE;
				}
			case ERROR_ISOLATE_REQUESTFAIL:
				{
					SendIsolateResult( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYINVITE_DENIED:
		{
			int iRet = _RecvPartyInviteDenied( reinterpret_cast<const CSInviteDenied*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			}
			break;
		}
	case CS_PARTYINVITE_OVERSERVER_RESULT:
		{
			int iRet = _RecvPartyInviteOverServerResult( reinterpret_cast<const CSPartyInviteOverServerResult*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			}
			break;
		}
	case CS_PARTYMEMBER_INFO:
		{
			int iRet = _RecvPartyMemberInfo( reinterpret_cast<const CSPartyMemberInfo*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_NOTFOUND_MEMBERINFO:
				{
					SendPartyInfoErr( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYMEMBER_KICK:
		{
			int iRet = _RecvPartyMemberKick( reinterpret_cast<const CSPartyMemberKick*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_KICKFAIL:
			case ERROR_PARTY_KICKFAIL_DEST_IS_YOU:
				{
					SendKickPartymember( reinterpret_cast<const CSPartyMemberKick*>(pData)->nSessionID, iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYLEADER_SWAP:
		{
			int iRet = _RecvPartyLeaderSwap( reinterpret_cast<const CSPartyLeaderSwap*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_LEADER_APPOINTMENTFAIL:
				{
					SendSwapPartyLeader(0, iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_CHANNELLIST:
		{
			int iRet = _RecvPartyChannelList( nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
			case ERROR_GENERIC_UNKNOWNERROR:
			case ERROR_GENERIC_INVALIDREQUEST:
				return iRet;
			}
			break;
		}
	case CS_SELECTCHANNEL:
		{
			return _RecvPartySelectChannel( reinterpret_cast<const CSVillageSelectChannel*>(pData), nLen );
		}
	case CS_PARTYMODIFY:
		{
			int iRet = _RecvPartyInfoModify( reinterpret_cast<const CSPartyInfoModify*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
			case ERROR_GENERIC_INVALIDREQUEST:
				return iRet;
			case ERROR_PARTY_MODIFYINFO_FAIL:
			case ERROR_PARTY_MODIFYINFO_FAIL_NEED_NAME:
			case ERROR_PARTY_MODIFYINFO_FAIL_PASSWORD_ERR:
			case ERROR_PARTY_MODIFYINFO_FAIL_RESTRICT_LEVEL_ERROR:
				{
					SendPartyModify( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}	
	case CS_READYREQUEST:
		{
			int iRet = _RecvPartyReadyQuest( nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
			case ERROR_GENERIC_INVALIDREQUEST:
				return iRet;
			}
			break;
		}
	case CS_PARTYASKJOIN:
		{
			int iRet = _RecvPartyAskJoin( reinterpret_cast<const CSPartyAskJoin*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_ASKJOIN_FAIL:
			case ERROR_PARTY_ASKJOIN_DEST_DIFFMAP:
			case ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND:
			case ERROR_PARTY_ASKJOIN_DESTUSER_SITUATION_NOTALLOWED:
				{
					SendAskJoinResult( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_PARTYASKJOINDECISION:
		{
			int iRet = _RecvPartyAskJoinDecision( reinterpret_cast<const CSPartyAskJoinDecision*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL:
				{
					SendAskJoinResult( iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_SWAPMEMBERINDEX:
		{
			int iRet = _RecvPartySwapMemberIndex( reinterpret_cast<const CSPartySwapMemberIndex*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_SWAPINDEX_FAIL:
				{
					SendPartySwapMemberIndex( NULL, iRet );
					return ERROR_NONE;
				}
			}
			break;
		}
	case CS_CONFIRM_LASTDUNGEONINFO:
		{
			int iRet = _RecvPartyConfirmLastDungeonInfo( reinterpret_cast<const CSPartyConfirmLastDungeonInfo*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			}
			break;
		}
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)	
	case CS_PARTYINFO:
		{
			int iRet = _RecvPartyInfo( reinterpret_cast<const CSPartyInfo*>(pData), nLen );
			switch( iRet )
			{
			case ERROR_NONE:
			case ERROR_INVALIDPACKET:
				return iRet;
			case ERROR_PARTY_JOINFAIL:
			case ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND:
				SendPartyInfo(iRet, NULL);
				return ERROR_NONE;
			}
		}
		break;
#endif
	}	

	_DANGER_POINT();
	return ERROR_UNKNOWN_HEADER;			//Must return at caller level!
}

//============================================================================================================================================================

int CDNUserSession::_RecvPartyCreate( const CSCreateParty* pPacket, int iLen )
{
#if defined( PRE_PARTY_DB )
	if (sizeof(CSCreateParty) != iLen)
		return ERROR_INVALIDPACKET;
#else
	if (sizeof(CSCreateParty) - sizeof(pPacket->wszBuf) +((pPacket->cNameLen + pPacket->cPassWordLen) * sizeof(WCHAR)) != iLen)
		return ERROR_INVALIDPACKET;
#endif // #if defined( PRE_PARTY_DB )

	WCHAR wszName[PARTYNAMELENMAX];
	memset(wszName, 0, sizeof(wszName));
#if defined( PRE_PARTY_DB )
#else
	WCHAR wszPass[PARTYPASSWORDMAX];
	memset(wszPass, 0, sizeof(wszPass));
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
	if( wcslen(pPacket->wszPartyName) <= 0 )
#else
	if (pPacket->cNameLen <= 0)
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_PARTY_CREATE_FAIL_NEED_PARTYNAME;

#if defined( PRE_PARTY_DB )
	if( pPacket->iBitFlag&Party::BitFlag::Password && CDNParty::ValidatePassword(pPacket->iPassword) == false )
#else
	if (pPacket->cPassWordLen > PARTYPASSWORDMAX-1)
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_PARTY_CREATE_FAIL_PASSWORD_ERR;

#if defined( PRE_PARTY_DB )
	if( wcslen(pPacket->wszPartyName) >= PARTYNAMELENMAX)
#else
	if (pPacket->cNameLen >= PARTYNAMELENMAX)
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_PARTY_CREATE_FAIL;

#if defined( PRE_PARTY_DB )
	if( pPacket->PartyType < _NORMAL_PARTY || pPacket->PartyType >= _PARTY_TYPE_MAX )
		return ERROR_PARTY_CREATE_FAIL;

	if( pPacket->PartyType == _RAID_PARTY_8 )
#else
	if (pPacket->cRaidParty == 1)
#endif // #if defined( PRE_PARTY_DB )
	{
		if (pPacket->cPartyMemberMax > RAIDPARTYCOUNTMAX || pPacket->cPartyMemberMax < RAIDPARTYCOUNTMIN)
			return ERROR_PARTY_CREATE_FAIL;
	}
	else
	{
		if (pPacket->cPartyMemberMax > NORMPARTYCOUNTMAX || pPacket->cPartyMemberMax < NORMPARTYCOUNTMIN)
			return ERROR_PARTY_CREATE_FAIL;
	}

	const sChannelInfo * pChInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
	if (pChInfo)
	{
		if (pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVP || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN)
			return ERROR_PARTY_CREATE_FAIL;

#if defined( PRE_PARTY_DB )
		if (pPacket->PartyType != _NORMAL_PARTY && pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_DARKLAIR)
#else
		if (pPacket->cRaidParty >= 1 && pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_DARKLAIR)
#endif // #if defined( PRE_PARTY_DB )
			return ERROR_PARTY_CREATE_FAIL;
	}
	else
	{
		return ERROR_PARTY_CREATE_FAIL;
	}			

#if defined( PRE_PARTY_DB )
	int iPermitLevel = g_pDataManager->GetMapPermitLevel( GetMapIndex() );
	if( pPacket->cUserLvLimitMin > GetLevel() || pPacket->cUserLvLimitMin <= 0 || pPacket->cUserLvLimitMin < iPermitLevel )
#else
	if (pPacket->cUserLvLimitMin > pPacket->cUserLvLimitMax || pPacket->cUserLvLimitMin > GetLevel() || pPacket->cUserLvLimitMin <= 0 || pPacket->cUserLvLimitMax > 100)
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_PARTY_CREATE_FAIL_RESTRICT_LEVEL_ERROR;

	if ( pPacket->ItemLootRule >= ITEMLOOTRULE_MAX)
		return ERROR_PARTY_CREATE_FAIL;

	if ( pPacket->ItemRank > ITEMRANK_NONE)
		return ERROR_PARTY_CREATE_FAIL;

#if defined( PRE_PARTY_DB )
	_wcscpy(wszName, PARTYNAMELENMAX, pPacket->wszPartyName, static_cast<int>(wcslen(pPacket->wszPartyName)));
	int nPass = pPacket->iPassword;
#else
	_wcscpy(wszName, PARTYNAMELENMAX, pPacket->wszBuf, pPacket->cNameLen);
	if (pPacket->cPassWordLen > 0)
		_wcscpy(wszPass, PARTYPASSWORDMAX, pPacket->wszBuf + pPacket->cNameLen, pPacket->cPassWordLen);

	int nPass = _wtoi(wszPass);
#endif // #if defined( PRE_PARTY_DB )

	if( nPass < 0 || nPass > MAXPARTYPASSWORD )
		return ERROR_PARTY_CREATE_FAIL_PASSWORD_ERR;

#if defined( PRE_PARTY_DB )
	if ( m_pDBCon && m_pDBCon->GetActive() )
		m_pDBCon->QueryAddParty( this, g_Config.nManagedID, Party::LocationType::Village, GetChannelID(), pPacket );
#else
	//After creation timing, party ID management cannot be done standalone, so the master creates and notifies it
	if (g_pMasterConnection && g_pMasterConnection->GetActive())
		g_pMasterConnection->SendReqPartyID(GetAccountDBID(), GetChannelID(), pPacket->cPartyMemberMax, pPacket->ItemLootRule, pPacket->ItemRank, pPacket->cUserLvLimitMin, pPacket->cUserLvLimitMax, pPacket->nTargetMapIdx, pPacket->Difficulty, pPacket->cIsJobDice, wszName, wszPass, 0, pPacket->cUseVoice > 0 ? true : false);
#endif // #if defined( PRE_PARTY_DB )
	else
		return ERROR_PARTY_CREATE_FAIL;

	return ERROR_NONE;
}

int CDNUserSession::_RecvPartyJoin( const CSJoinParty* pPacket, int iLen )
{
	if( m_eUserState == STATE_READY )
		return ERROR_PARTY_JOINFAIL_INREADYSTATE;

	if( GetPartyID() > 0 )
		return ERROR_PARTY_JOINFAIL;

	if( m_bIsMove || m_bIsStartGame )
		return ERROR_PARTY_JOINFAIL;

#if defined( PRE_PARTY_DB )
	if( sizeof(CSJoinParty) != iLen )
#else
	if( sizeof(CSJoinParty) - sizeof(pPacket->wszBuf) +(pPacket->cPartyPasswordSize * sizeof(WCHAR)) != iLen )
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_INVALIDPACKET;

#if defined( PRE_PARTY_DB )
#else
	if( pPacket->cPartyPasswordSize >= PARTYPASSWORDMAX )
		return ERROR_PARTY_JOINFAIL;

	WCHAR wszPass[PARTYPASSWORDMAX] = { 0, };
	if( pPacket->cPartyPasswordSize > 0 )
		_wcscpy( wszPass, PARTYPASSWORDMAX, pPacket->wszBuf, pPacket->cPartyPasswordSize );
#endif // #if defined( PRE_PARTY_DB )

	// Clear state
	IsWindowStateNoneSet(WINDOW_ISACCEPT);

	CDNParty* pParty = g_pPartyManager->GetParty( pPacket->PartyID );
	if( pParty == NULL )
	{
#if defined( PRE_PARTY_DB )
		m_PrevPartyListInfo.Clear();
#endif // #if defined( PRE_PARTY_DB )
		return ERROR_PARTY_JOINFAIL;
	}

	pParty->InviteDenied(GetCharacterName());

	if( pParty->CheckSameGateNo() != -1 || pParty->GetCompleteMember() || pParty->bIsMove() || pParty->bIsStartGame() )
		return ERROR_PARTY_JOINFAIL_INREADYSTATE;

	if (pParty->HasEmptyPlace() == false)
		return ERROR_PARTY_JOINFAIL;

#if defined( PRE_PARTY_DB )
#else
	CDNUserSession * pUser = g_pUserSessionManager->FindUserSessionByAccountDBID(pParty->GetLeaderAccountDBID());
	if (pUser == NULL)
		return ERROR_PARTY_JOINFAIL;
#endif // #if defined( PRE_PARTY_DB )

	if (pParty->GetMinLevel() > GetLevel())
		return ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_SHORTAGE;

#if defined( PRE_PARTY_DB )
#else
	if (pParty->GetMaxLevel() < GetLevel())
		return ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_OVER;
#endif // #if defined( PRE_PARTY_DB )

	if (pParty->bIsPasswordParty() == true )
	{
#if defined( PRE_PARTY_DB )
		if( pParty->CheckPassword(pPacket->iPassword) == false )
#else
		if( pParty->CheckPassword(wszPass) == false )
#endif // #if defined( PRE_PARTY_DB )
			return ERROR_PARTY_JOINFAIL_PASSWORDINCORRECT;
	}

	if (pParty->IsKickedMember(GetAccountDBID()))
		return ERROR_PARTY_JOINFAIL_YOU_ARE_KICKEDUSER;

#if defined( PRE_WORLDCOMBINE_PARTY )
	if(Party::bIsWorldCombineParty(pParty->GetPartyType()))
	{
		WorldCombineParty::WrldCombinePartyData* WorldCombinePartyData = g_pDataManager->GetCombinePartyData(pParty->GetWorldPartyPrimaryIndex());
		if( WorldCombinePartyData )
		{
			// Check entry ticket
			if(WorldCombinePartyData->nItemID > 0)
			{
				if( GetItem()->GetInventoryItemCount( WorldCombinePartyData->nItemID ) <= 0 )
					return ERROR_PARTY_JOINFAIL;
			}
			// Check entry count
			const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pParty->GetTargetMapIndex() );
			if( pMapInfo && pMapInfo->nMaxClearCount > 0 )
			{
				int nExpandNestClearCount = m_nExpandNestClearCount;
				int nPCBangExpandNestClearCount = g_pDataManager->GetPCBangNestClearCount(GetPCBangGrade(), pParty->GetTargetMapIndex());
				if (!pMapInfo->bExpandable)
				{
					nExpandNestClearCount = 0;
					nPCBangExpandNestClearCount = 0;
				}
				if( GetNestClearTotalCount( pParty->GetTargetMapIndex() ) >= (pMapInfo->nMaxClearCount + nExpandNestClearCount + nPCBangExpandNestClearCount) ) {
					return ERROR_PARTY_JOINFAIL;
				}
			}
		}		
		else
		{
			return ERROR_PARTY_JOINFAIL;
		}
	}
#endif
#if defined( PRE_PARTY_DB )

	if( m_pDBCon == NULL || m_pDBCon->GetActive() == false )
		return ERROR_PARTY_JOINFAIL;

	if( pParty->bIsInVillage() )
	{
		if( pParty->GetPartyStructData().iServerID == g_Config.nManagedID && pParty->GetChannelID() == GetChannelID() )
		{
			m_pDBCon->QueryJoinParty( this, this->GetWorldSetID(), pParty->GetPartyID(), pParty->GetMemberMax() );		
		}
		else
		{
			ModCommonVariableData( CommonVariable::Type::MOVESERVER_PARTYID, pParty->GetPartyID() );
			CmdSelectChannel( pParty->GetChannelID(), true );
		}
	}
	else if( pParty->bIsInGame() )
	{
		const TMapInfo* pMapInfo = g_pDataManager->GetMapInfo( pParty->GetPartyMapIndex() );
		if( pMapInfo == NULL )
			return ERROR_PARTY_JOINFAIL;

		if( pMapInfo->MapType != GlobalEnum::MAP_WORLDMAP )
			return ERROR_PARTY_JOINFAIL;

		INT64 biValue = MAKELONG64( pParty->GetServerID(),pParty->GetPartyStructData().iRoomID );
		g_pMasterConnection->SendConfirmLastDungeonInfo( this, biValue, BreakInto::Type::PartyJoin );
	}
	else
	{
		return ERROR_PARTY_JOINFAIL;
	}

#else
	if (pParty->GetChannelID() != GetChannelID())
	{
		const sChannelInfo * pChInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
		const sChannelInfo * pPartyChInfo = g_pMasterConnection->GetChannelInfo(pParty->GetChannelID());
		if (pChInfo && pPartyChInfo)
		{
			if (pChInfo->nMapIdx != pPartyChInfo->nMapIdx)
				return ERROR_PARTY_JOINFAIL;
		}
		else
			return ERROR_PARTY_JOINFAIL;

		if (!g_pMasterConnection || g_pMasterConnection->GetActive() == false)
			return ERROR_PARTY_JOINFAIL;
	}

	int nMemberIdx = -1;
	if( pParty->AddPartyMember(GetAccountDBID(), GetSessionID(), GetCharacterName(), nMemberIdx ) == false )
		return ERROR_PARTY_JOINFAIL;
	if( pParty->HasEmptyPlace() == false )
		g_pPartyManager->EraseWaitPartyList(pParty);

	m_PartyID = pParty->GetPartyID();
	m_nPartyMemberIndex = nMemberIdx;

	SendJoinParty(ERROR_NONE, pParty->GetPartyID(), pParty->GetPartyName() );
	pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
	pParty->SendPartyMemberMoveEachOther(GetSessionID(), m_BaseData.TargetPos.nX, m_BaseData.TargetPos.nY, m_BaseData.TargetPos.nZ);
	if( GetProfile()->bOpenPublic )
		pParty->SendUserProfile( GetSessionID(), *GetProfile() );

	if( pParty->GetChannelID() != GetChannelID() )
		g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_SINGLE_SAMEDEST, GetAccountDBID(), GetMapIndex(), -1, pParty->GetChannelID());
#endif // #if defined( PRE_PARTY_DB )

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyOut( int iLen )
{
	if( iLen != 0 )
		return ERROR_INVALIDPACKET;

	if (m_eUserState == STATE_READY)
		return ERROR_PARTY_LEAVEFAIL;

	if (GetPartyID() <= 0)
		return ERROR_PARTY_LEAVEFAIL;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty == NULL )
	{
		SendPartyOut(ERROR_NONE, -1, GetSessionID());
		ClearPartyInfo();
		return ERROR_NONE;
	}

#if defined( PRE_PARTY_DB )
	g_pPartyManager->QueryOutParty( GetPartyID(), GetAccountDBID(), GetCharacterDBID(), GetSessionID(), Party::QueryOutPartyType::Normal );
	return ERROR_NONE;
#else
	return g_pPartyManager->DelPartyMember( pParty, this );
#endif // #if defined( PRE_PARTY_DB )
}

int	CDNUserSession::_RecvPartyListInfo( const CSPartyListInfo* pPacket, int iLen )
{
	if (sizeof(CSPartyListInfo) != iLen)
		return ERROR_INVALIDPACKET;

	if (GetPartyID() > 0)
		return ERROR_PARTY_LIST_ERROR;

	int nRetCode = ERROR_NONE;
#if defined( PRE_PARTY_DB )

	DWORD dwCurTime = timeGetTime();
	if( m_PrevPartyListInfo == *pPacket )
	{
		if( dwCurTime-m_dwPrevPartyListInfoTick < Party::Constants::PARTY_LIST_REFRESH_SEC_CLIENT*1000 )
			return ERROR_PARTY_REFRESHLIST_TIMERESTRAINT;
	}
	
	m_PrevPartyListInfo = *pPacket;
	m_dwPrevPartyListInfoTick = dwCurTime;

	std::list<CDNParty*> lList;
	g_pPartyManager->GetPartyListInfo(GetMapIndex(), GetChannelID(), g_pMasterConnection->GetMeritBonusID(GetChannelID()), PARTYLISTOFFSET, pPacket, lList );
#else
	std::list <SPartyListInfo> lList;
	int nWholeCount = g_pPartyManager->GetPartyListInfo(GetMapIndex(), GetChannelID(), g_pMasterConnection->GetMeritBonusID(GetChannelID()), PARTYLISTOFFSET, pPacket->cGetPage, const_cast<CSPartyListInfo*>(pPacket)->nSortMapIdx, pPacket->Difficulty, nRetCode, &lList);
	if( pPacket->cIsRefresh > 0 )
		nRetCode = ERROR_NONE;
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )

	if( lList.empty() == false )
	{
		if( pPacket->unReqPage*PARTYLISTOFFSET >= (int)(lList.size()) )
			nRetCode = ERROR_PARTY_PARTYLIST_OVERFLOW;
	}

	SendPartyList( nRetCode, pPacket, lList );
#else
	SendPartyList( &lList, nWholeCount, nRetCode );
#endif // #if defined( PRE_PARTY_DB )
	return ERROR_NONE;
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
int	CDNUserSession::_RecvPartyInfo( const CSPartyInfo* pPacket, int iLen )
{
	if (sizeof(CSPartyInfo) != iLen)
		return ERROR_INVALIDPACKET;
	
	CDNParty* pParty = NULL;
	if( pPacket->PartyID > 0 )
	{
		pParty = g_pPartyManager->GetParty(pPacket->PartyID);
	}
	else
	{
		if( CheckLastSpace(pPacket->wszCharName) )
			return ERROR_PARTY_JOINFAIL;

		if( !__wcsicmp_l(pPacket->wszCharName, GetCharacterName()) )
			return ERROR_PARTY_JOINFAIL;

		CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(pPacket->wszCharName);
		if( !pUser )
		{	//Request party ID from master server
			if (!g_pMasterConnection || g_pMasterConnection->GetActive() == false)
				return ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND;

			g_pMasterConnection->SendGetPartyID( GetAccountDBID(), pPacket->wszCharName );
			return ERROR_NONE;
		}

		pParty = g_pPartyManager->GetParty(pUser->GetPartyID());
	}

	SendPartyInfo(ERROR_NONE, pParty);
	return ERROR_NONE;
}
#endif

int CDNUserSession::_RecvPartyRefreshGateInfo( const CSRefreshGateInfo* pPacket, int iLen )
{
	if( sizeof(CSRefreshGateInfo) != iLen )
		return ERROR_INVALIDPACKET;

	if( m_pField == NULL )
		return ERROR_NONE;

	TPosition Pos = { 0, };
	Pos.nX = (int)(pPacket->Position.x * 1000.f);
	Pos.nY = (int)(pPacket->Position.y * 1000.f);
	Pos.nZ = (int)(pPacket->Position.z * 1000.f);

	if (m_pField == NULL)
	{
		char Gate[PARTYMAX] = { 0, };
		Gate[0] = g_pFieldDataManager->CheckGateArea(GetMapIndex(), Pos);
		SendRefreshGateInfo(Gate, 1, ERROR_PARTY_CANT_ENTERGATE);
		return ERROR_NONE;
	}

	int GateType = -1;
	int GateNo = -1;
	if( pPacket->boEnter )
	{
		if (m_cGateNo > 0)
			return ERROR_NONE;

		if (m_pField)
		{
			GateNo = m_pField->GetFieldData()->CheckGateArea(Pos);
			GateType = m_pField->GetFieldData()->GetGateType(Pos);
		}
		else
			GateNo = g_pFieldDataManager->CheckGateArea(GetMapIndex(), Pos);

		if (GateNo < 0) 
			GateNo = -1;
	}
	else 
	{
		GateNo = -1;
	}

	m_cGateNo = GateNo;		// Set GateNo and
	m_nGateType = GateType;
	m_cGateSelect = 0;

	const sChannelInfo * pChannelInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
	if (pChannelInfo == NULL)
		return ERROR_GENERIC_UNKNOWNERROR;

	int TargetMapIndex = GetTargetMapIndex();

	if (GetPartyID() > 0)
	{
		CDNParty *pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty == NULL )
			return ERROR_GENERIC_UNKNOWNERROR;

		pParty->SendAllRefreshGateInfo();

		if ((GateNo = pParty->CheckSameGateNo()) > 0)
		{
			if (g_pDataManager->GetMapType(TargetMapIndex) != GlobalEnum::MAP_VILLAGE )
			{
				if (pChannelInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN || GateNo == 11 && TargetMapIndex == 15106)
					pParty->SendAllFarmInfo();
				else
					pParty->SendAllGateInfo(GateNo);
			}
			else
			{
				if (g_pMasterConnection && g_pMasterConnection->GetActive())
					g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_PARTY, pParty->GetPartyID(), TargetMapIndex, GateNo, -1, pParty);
			}
		}
	}
	else 
	{
		char Gate[PARTYMAX] = { 0, };
		Gate[0] = GateNo;
		SendRefreshGateInfo(Gate, 1, ERROR_NONE);
		if ( GateNo > 0 )
		{
			m_nSingleRandomSeed = timeGetTime();//_rand();
			m_eUserState = STATE_READY;
			if (g_pDataManager->GetMapType(TargetMapIndex) != GlobalEnum::MAP_VILLAGE)
			{
				if (pChannelInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN || GateNo == 11 && TargetMapIndex == 15106)
					ReqFarmInfo();
				else
					SendGateInfo(GetSessionID(), GateNo, 0, NULL, NULL);
			}
			else
			{
				if (g_pMasterConnection && g_pMasterConnection->GetActive())
					g_pMasterConnection->SendVillageToVillage(REQINFO_TYPE_SINGLE, GetAccountDBID(), GetMapIndex(), m_cGateNo);
			}
		}
	}

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartySelectStage( const CSSelectStage* pPacket, int iLen )
{
	if (sizeof(CSSelectStage) != iLen)
		return ERROR_INVALIDPACKET;

	if( GetPartyID() > 0 )
	{
		CDNParty *pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty)
			pParty->SendSelectStage(pPacket->cSelectMapIndex);
	}
	else
		SendSelectStage(pPacket->cSelectMapIndex);

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyStartStage( const CSStartStage* pPacket, int iLen )
{
	if (sizeof(CSStartStage) != iLen)
		return ERROR_INVALIDPACKET;

	if( GetPartyID() > 0 )
	{
		CDNParty *pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty == NULL )
		{
			if (!IsNoneWindowState())
				return ERROR_CASHSHOP_OPEN;
		}
		else
		{
			if( !pParty->IsPartyMemberWindowStateNone() )
				pParty->SendAllCancelStage( ERROR_CASHSHOP_OPEN );
		}
	}
	else 
	{
		if (!IsNoneWindowState())
		{
			m_cGateNo = -1;
			m_cGateSelect = -1;

			return ERROR_CASHSHOP_OPEN;
		}
	}

	if (m_bIsStartGame || m_bIsMove)
	{
		g_Log.Log(LogType::_GAMECONNECTLOG, this, L"CS_STARTSTAGE Repetitive Move\n");
		return ERROR_PARTY_STARTSTAGE_FAIL;
	}

	int TargetMapIndex = -1;
	int TargetGateNo = -1;

	m_cGateSelect = pPacket->cSelectMapIndex;
	SendSelectStage(m_cGateSelect);

	TargetMapIndex = GetTargetMapIndex();
	TargetGateNo = g_pDataManager->GetGateNoByGateNo(GetMapIndex(), m_cGateNo, m_cGateSelect);

	if( TargetMapIndex <= 0 || m_pField == NULL )
	{
		if (GetPartyID() > 0)
		{
			CDNParty *pParty = g_pPartyManager->GetParty(GetPartyID());
			if( pParty == NULL )
				return ERROR_PARTY_STARTSTAGE_FAIL;

			pParty->SendAllCancelStage( ERROR_PARTY_STARTSTAGE_FAIL );
		}
		else 
		{
			m_cGateNo = -1;
			m_cGateSelect = -1;

			return ERROR_PARTY_STARTSTAGE_FAIL;
		}

		return ERROR_NONE;
	}

	// If a client moves arbitrarily after the party is Ready, the moved client requests RefreshGateInfo
	// That client's GateIndex becomes -1.
	// In that case, since none of the party members will start, prevent starting here if any party member is no longer Ready.
	// Or we could resend GateIndex and RandomSeed on StartStage.. but then the unlocked one might do trading or something
	// Not sure what will happen if forcibly moved later, so it seems best to Cancel it for now.
	// Self (leader or individual) is already canceled above when gate is -1 and TargetMapIndex becomes -1, so ignore~
	if( g_pMasterConnection->GetActive() == false )
		return ERROR_GENERIC_MASTERCON_NOT_FOUND;

	const sChannelInfo * pChannelInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
	if (pChannelInfo == NULL)
		return ERROR_PARTY_STARTSTAGE_FAIL;

	int nFarmMapID = -1;
	bool bEnterFarm = false;
	bEnterFarm = g_pFarm->GetFarmMapID(pPacket->nFarmDBID, nFarmMapID);

	GameTaskType::eType TaskType = m_pField->GetGameTaskType();
	if (m_nGateType > 0)
		TaskType = (GameTaskType::eType)m_nGateType;

	CDNParty* pParty = NULL;

#if defined(_FINAL_BUILD)
	if ((GetAccountLevel() <= AccountLevel_QA) && (GetAccountLevel() >= AccountLevel_New))
	{
		// Skip GM.
	}
	else
#endif // #if defined(_FINAL_BUILD)
	{
		if (!g_pDataManager->CheckChangeMap(GetMapIndex(), TargetMapIndex))
		{
			WCHAR wszBuf[100];
			wsprintf( wszBuf, L"VILLAGE Prev[%d]->Next[%d]", GetMapIndex(), TargetMapIndex);
			GetDBConnection()->QueryAddAbuseLog(this, ABUSE_MOVE_SERVERS, wszBuf);
		}
	}

	if (GetPartyID() > 0 ) 
	{
		if (bEnterFarm)
			return ERROR_PARTY_STARTSTAGE_FAIL;

		if( m_boPartyLeader == false )
			return ERROR_PARTY_STARTSTAGE_FAIL;

		pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty == NULL )
			return ERROR_PARTY_STARTSTAGE_FAIL;

		//If in party state, check duplicate movement across all users
		if( pParty->CheckPartyUserStartFlag() == false )
		{
			g_Log.Log(LogType::_GAMECONNECTLOG, this, L"CS_STARTSTAGE Repetitive Move\n");
			return ERROR_PARTY_STARTSTAGE_FAIL;
		}

		if( pParty->DiffMemberGateNo(m_cGateNo) )
		{
			if (g_pMasterConnection && g_pMasterConnection->GetActive())
			{
				pParty->SetStartGame( true );
				pParty->SendSelectStage(m_cGateSelect);

				g_pMasterConnection->SendReqGameID( TaskType, REQINFO_TYPE_PARTY, pParty->GetPartyID(), GetChannelID(), pParty->GetRandomSeed(), GetMapIndex(), m_cGateNo, pPacket->Difficulty, false, pParty, 0, m_cGateSelect );		// Request GameID
			}
			else
			{
				pParty->SendAllCancelStage(ERROR_PARTY_STARTSTAGE_FAIL);
				return ERROR_NONE;
			}
		}
		else
		{
			pParty->SendAllCancelStage(ERROR_PARTY_STARTSTAGE_FAIL);
			return ERROR_NONE;
		}
	}
	else
	{
		if (g_pMasterConnection && g_pMasterConnection->GetActive())
		{
			if (bEnterFarm)
			{
#if defined( PRE_ADD_FARM_DOWNSCALE )
				int iAttr = Farm::Attr::None;
				if( g_pFarm->GetFarmAttr( pPacket->nFarmDBID, iAttr ) == false )
					return ERROR_NONE;

#if defined( PRE_ADD_VIP_FARM )
				if( iAttr&Farm::Attr::Vip )
				{
					if( bIsFarmVip() == false )
						return ERROR_FARM_INVALID_VIP;
				}
#endif // #if defined( PRE_ADD_VIP_FARM )
				if( iAttr&Farm::Attr::GuildChampion )
				{
					// Check guild farm entry
					if( GetGuildUID().IsSet() == false || g_pGuildWarManager->GetPreWinGuildUID().IsSet() == false || g_pGuildWarManager->GetPreWinGuildUID() != GetGuildUID() )
						return ERROR_FARM_ENTERANCE_CONDITION;
				}
#elif defined( PRE_ADD_VIP_FARM )
				Farm::Attr::eType Attr = Farm::Attr::None;
				if( g_pFarm->GetFarmAttr( pPacket->nFarmDBID, Attr ) == false )
				{
					_DANGER_POINT();
					return ERROR_NONE;
				}

				switch( Attr )
				{
				case Farm::Attr::Vip:
					{
						if( bIsFarmVip() == false )
							return ERROR_FARM_INVALID_VIP;
						break;
					}
				}
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
				SetLastSubVillageMapIndex( GetMapIndex() );
				g_pMasterConnection->SendReqGameID( GameTaskType::Farm, REQINFO_TYPE_FARM, m_nAccountDBID, GetChannelID(), timeGetTime(), nFarmMapID, 0, CONVERT_TO_DUNGEONDIFFICULTY(0), true, NULL, pPacket->nFarmDBID );
			}
			else
			{
				g_pMasterConnection->SendReqGameID( TaskType, REQINFO_TYPE_SINGLE, m_nAccountDBID, GetChannelID(), m_nSingleRandomSeed, GetMapIndex(), m_cGateNo, pPacket->Difficulty, false, NULL, 0, m_cGateSelect );	// Request GameID
			}
		}
		else
			return ERROR_PARTY_STARTSTAGE_FAIL;
	}

	//If execution reaches here, it was sent to Master. Set a flag for restriction handling until the return message arrives.
	if (pParty)
	{
		//If pParty is valid, party-level movement occurred. Update the flags of party users.
		pParty->SetPartyUserStartFlag(true);
	}
	else
	{
		//Otherwise, move individually
		m_bIsStartGame = true;
	}

	g_Log.Log(LogType::_NORMAL, this, L"[ADBID:%u, CDBID:%I64d, SID:%u, ObjID:%u] [CS_STARTSTAGE] TargetMap:%d, MapIndex:%d GateNo:%d\r\n", m_nAccountDBID, m_biCharacterDBID, GetSessionID(), m_BaseData.nObjectID, TargetMapIndex, GetMapIndex(), m_cGateNo);
	return ERROR_NONE;
}

int CDNUserSession::_RecvPartyCancelStage( int iLen )
{
	if( iLen != 0)
		return ERROR_INVALIDPACKET;

	if( GetPartyID() > 0 )
	{
		CDNParty *pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty == NULL )
			return ERROR_NONE;

		if( pParty->bIsStartGame() == false )
			pParty->SendAllCancelStage( ERROR_NONE );
	}
	else 
	{
		m_eUserState = STATE_NONE;
		SendCancelStage( ERROR_NONE );
	}

	return ERROR_NONE;
}

#if defined(PRE_FIX_62281)
int CDNUserSession::_RecvPartyInvite( const CSInvitePartyMember* pPacket, int iLen )
{
	if( sizeof(CSInvitePartyMember)-sizeof(pPacket->wszBuf)+(pPacket->cInviteMemberSize*sizeof(WCHAR)) != iLen )
		return ERROR_INVALIDPACKET;

	if( pPacket->cInviteMemberSize <= 0 || pPacket->cInviteMemberSize >= NAMELENMAX )
		return ERROR_PARTY_INVITEFAIL;


	WCHAR wszName[NAMELENMAX];
	memset(wszName, 0, sizeof(wszName));
	_wcscpy(wszName, NAMELENMAX, pPacket->wszBuf, pPacket->cInviteMemberSize);

	if( CheckLastSpace(wszName) )
		return ERROR_PARTY_INVITEFAIL;

	if( !__wcsicmp_l(wszName, GetCharacterName()) )
		return ERROR_PARTY_INVITEFAIL;

	if (m_eUserState != STATE_NONE)		//Cannot invite if target is moving, ready, etc.
		return ERROR_PARTY_INVITEFAIL;	

	const sChannelInfo* pChInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
	if( pChInfo == NULL )
		return ERROR_PARTY_INVITEFAIL;

	if (pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVP || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN )
		return ERROR_PARTY_INVITEFAIL;

	if( m_pIsolate->IsIsolateItem(wszName) )
		return ERROR_ISOLATE_REQUESTFAIL;
	
	//Check party
	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if(pParty)
	{
		if( pParty->GetLeaderAccountDBID() != m_nAccountDBID )
			return ERROR_PARTY_INVITEFAIL;

		if( pParty->IsKickedMember(GetAccountDBID()) )
			return ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED;

		if( pParty->bIsMove() || pParty->bIsStartGame() )
			return ERROR_PARTY_INVITEFAIL;

		if( pParty->HasEmptyPlace() == false )
			return ERROR_PARTY_INVITEFAIL;

		//Confirm target
		CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(wszName);
		if( pUser == NULL )
		{
#if defined( PRE_PARTY_DB )
			//Send Invite request to master server
			if( pParty->InvitePartyMember(wszName) == false )	//Add to m_VecInviteList
				return ERROR_PARTY_INVITEFAIL_ALREADY_INVITED;
			g_pMasterConnection->SendReqPartyInvite( this, wszName, pParty );
			return ERROR_NONE;
#else
			return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;
#endif // #if defined( PRE_PARTY_DB )
		}

		//Check if target is in a state to receive an invitation
#if defined(PRE_PARTY_DB)
		int nRet = pUser->CheckPartyInvitableState(GetCharacterName(), pParty->GetMinLevel());
#else
		int nRet = pUser->CheckPartyInvitableState(GetCharacterName(), pParty->GetMinLevel(), pParty->GetMaxLevel());
#endif
		if(nRet != ERROR_NONE)
			return nRet;

		if( pParty->InvitePartyMember(wszName) == false )
			return ERROR_PARTY_INVITEFAIL_ALREADY_INVITED;

		pUser->SetWindowState(WINDOW_ISACCEPT);
#if defined( PRE_PARTY_DB )
		pUser->SendPartyInviteMsg(pParty->GetPartyID(), pParty->GetPartyName(), pParty->GetPassword(), GetCharacterName(), (BYTE)pParty->GetMemberMax(), (BYTE)pParty->GetMemberCount(), (USHORT)pParty->GetAvrUserLv(), -1, GetMapIndex() ); 
#else
		pUser->SendPartyInviteMsg(pParty->GetPartyID(), pParty->GetPartyName(), pParty->m_wszPartyPassword, GetCharacterName(), (BYTE)pParty->GetMemberMax(), (BYTE)pParty->GetMemberCount(), (USHORT)pParty->GetAvrUserLv()); 
#endif // #if defined( PRE_PARTY_DB )

		return ERROR_NONE;
	}
	else	//Create new party
	{
		WCHAR wszInvitePartyName[PARTYNAMELENMAX];
		memset(wszInvitePartyName, 0, sizeof(wszInvitePartyName));

		if (wcslen(GetCharacterName()) + wcslen(CEtUIXML::GetInstance().GetUIString(CEtUIXML::idCategory1, 3535)) >= PARTYNAMELENMAX)
			_wcscpy(wszInvitePartyName, _countof(wszInvitePartyName), GetCharacterName(), (int)wcslen(GetCharacterName()));
		else
			swprintf(wszInvitePartyName, L"%s%s", GetCharacterName(), CEtUIXML::GetInstance().GetUIString(CEtUIXML::idCategory1, 3535));

#if defined( PRE_PARTY_DB )
		if( m_pDBCon && m_pDBCon->GetActive() )
		{
			CSCreateParty CreatePartyPacket;
			memset( &CreatePartyPacket, 0, sizeof(CreatePartyPacket) );

			CreatePartyPacket.PartyType = _NORMAL_PARTY;
			CreatePartyPacket.cPartyMemberMax = NORMPARTYCOUNTMAX;
			CreatePartyPacket.ItemLootRule = ITEMLOOTRULE_INORDER;
			CreatePartyPacket.ItemRank = ITEMRANK_B;
			int iPermitLevel = g_pDataManager->GetMapPermitLevel(GetMapIndex());
			CreatePartyPacket.cUserLvLimitMin = (iPermitLevel == 0) ? 1 : iPermitLevel ;
			CreatePartyPacket.nTargetMapIdx = 0;
			CreatePartyPacket.Difficulty = Dungeon::Difficulty::Easy;
			if( m_bVoiceAvailable )
				CreatePartyPacket.iBitFlag |= Party::BitFlag::VoiceChat;
			_wcscpy( CreatePartyPacket.wszPartyName, _countof(CreatePartyPacket.wszPartyName), wszInvitePartyName, static_cast<int>(wcslen(wszInvitePartyName)) );

			m_pDBCon->QueryAddParty( this, g_Config.nManagedID, Party::LocationType::Village, GetChannelID(), &CreatePartyPacket, wszName );
		}
#else
		if (g_pMasterConnection && g_pMasterConnection->GetActive())
		{
			CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(wszName);
			if( pUser == NULL )
				return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;

			g_pMasterConnection->SendReqPartyID(GetAccountDBID(), GetChannelID(), NORMPARTYCOUNTMAX, ITEMLOOTRULE_INORDER, ITEMRANK_B, 1, 100, 0, 0, 1, wszInvitePartyName, L"", pUser->GetAccountDBID(), m_bVoiceAvailable); // ITEMRANK modified by kalliste 2010.07.09
		}
#endif // #if defined( PRE_PARTY_DB )
		else
			return ERROR_PARTY_INVITEFAIL;		
	}

	return ERROR_NONE;
}

#else
int CDNUserSession::_RecvPartyInvite( const CSInvitePartyMember* pPacket, int iLen )
{
	if( sizeof(CSInvitePartyMember)-sizeof(pPacket->wszBuf)+(pPacket->cInviteMemberSize*sizeof(WCHAR)) != iLen )
		return ERROR_INVALIDPACKET;

	if (m_eUserState != STATE_NONE)		//Cannot invite if target is moving, ready, etc.
		return ERROR_PARTY_INVITEFAIL;

	if( pPacket->cInviteMemberSize <= 0 || pPacket->cInviteMemberSize >= NAMELENMAX )
		return ERROR_PARTY_INVITEFAIL;

	WCHAR wszName[NAMELENMAX];
	memset(wszName, 0, sizeof(wszName));
	_wcscpy(wszName, NAMELENMAX, pPacket->wszBuf, pPacket->cInviteMemberSize);

	if( CheckLastSpace(wszName) )
		return ERROR_PARTY_INVITEFAIL;

	if( !__wcsicmp_l(wszName, GetCharacterName()) )
		return ERROR_PARTY_INVITEFAIL;

	const sChannelInfo* pChInfo = g_pMasterConnection->GetChannelInfo(GetChannelID());
	if( pChInfo == NULL )
		return ERROR_PARTY_INVITEFAIL;
	if (pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVP || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY || pChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_FARMTOWN )
		return ERROR_PARTY_INVITEFAIL;

	if( m_pIsolate->IsIsolateItem(wszName) )
		return ERROR_ISOLATE_REQUESTFAIL;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty != NULL )
	{
		if( pParty->GetLeaderAccountDBID() != m_nAccountDBID )
			return ERROR_PARTY_INVITEFAIL;

		if( pParty->bIsMove() || pParty->bIsStartGame() )
			return ERROR_PARTY_INVITEFAIL;

		if( pParty->HasEmptyPlace() == false )
			return ERROR_PARTY_INVITEFAIL;

		CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(wszName);
		if( pUser == NULL )
		{
#if defined( PRE_PARTY_DB )
			if( pParty->InvitePartyMember(wszName) == false )
				return ERROR_PARTY_INVITEFAIL_ALREADY_INVITED;
			g_pMasterConnection->SendReqPartyInvite( this, wszName, pParty );
			return ERROR_NONE;
#else
			return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;
#endif // #if defined( PRE_PARTY_DB )
		}

		if( pUser->m_eUserState != STATE_NONE )
			return ERROR_PARTY_INVITEFAIL_DEST_USER_NOWREADY;

		if( pParty->IsKickedMember(pUser->GetAccountDBID()) )
			return ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED;

		if( pUser->IsAcceptAbleOption(GetCharacterDBID(), GetAccountDBID(), _ACCEPTABLE_CHECKTYPE_PARTYINVITE) == false )
			return ERROR_PARTY_INVITEFAIL;

		if( pUser->GetIsolate()->IsIsolateItem(GetCharacterName()) )
			return ERROR_PARTY_INVITEFAIL;

		if( (pUser->GetAccountLevel() >= AccountLevel_New) && (pUser->GetAccountLevel() <= AccountLevel_Master))
			return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;

		const sChannelInfo* pUserChInfo = g_pMasterConnection->GetChannelInfo(pUser->GetChannelID());
		if( pUserChInfo == NULL )
			return ERROR_PARTY_INVITEFAIL;

#if defined( PRE_PARTY_DB )
#else
		if( (pUser->m_cVillageID != m_cVillageID) ||(pUserChInfo->nMapIdx != pChInfo->nMapIdx) )
			return ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME;
#endif // #if defined( PRE_PARTY_DB )
		if( pUserChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVP || pUserChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY)
			return ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME;

		if( !pUser->IsNoneWindowState())
			return ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED;

		if( pUser->GetPartyID() > 0)
			return ERROR_PARTY_INVITEFAIL_ALREADY_HAS_PARTY;

		if( pUser->GetLevel() < pParty->GetMinLevel())
			return ERROR_PARTY_INVITEFAIL_CHARACTERLEVEL_SHORTAGE;

#if defined( PRE_PARTY_DB )
#else
		if( pUser->GetLevel() > pParty->GetMaxLevel())
			return ERROR_PARTY_JOINFAIL_CHARACTERLEVEL_OVER;
#endif // #if defined( PRE_PARTY_DB )

		if( pParty->InvitePartyMember(wszName) == false )
			return ERROR_PARTY_INVITEFAIL_ALREADY_INVITED;

		pUser->SetWindowState(WINDOW_ISACCEPT);
#if defined( PRE_PARTY_DB )
		pUser->SendPartyInviteMsg(pParty->GetPartyID(), pParty->GetPartyName(), pParty->GetPassword(), GetCharacterName(), (BYTE)pParty->GetMemberMax(), (BYTE)pParty->GetMemberCount(), (USHORT)pParty->GetAvrUserLv(), -1, GetMapIndex() ); 
#else
		pUser->SendPartyInviteMsg(pParty->GetPartyID(), pParty->GetPartyName(), pParty->m_wszPartyPassword, GetCharacterName(), (BYTE)pParty->GetMemberMax(), (BYTE)pParty->GetMemberCount(), (USHORT)pParty->GetAvrUserLv()); 
#endif // #if defined( PRE_PARTY_DB )

		return ERROR_NONE;
	}

	CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByName(wszName);
	if( pUser == NULL )
		return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;

	if( pUser->m_eUserState != STATE_NONE )
		return ERROR_PARTY_INVITEFAIL_DEST_USER_NOWREADY;

	if( pUser->IsAcceptAbleOption(GetCharacterDBID(), GetAccountDBID(), _ACCEPTABLE_CHECKTYPE_PARTYINVITE) == false )
		return ERROR_PARTY_INVITEFAIL;

	if( pUser->GetIsolate()->IsIsolateItem(GetCharacterName()) )
		return ERROR_PARTY_INVITEFAIL;

	if( (pUser->GetAccountLevel() >= AccountLevel_New) && (pUser->GetAccountLevel() <= AccountLevel_Master))
		return ERROR_PARTY_INVITEFAIL_DESTUSER_NOTFOUND;

	const sChannelInfo* pUserChInfo = g_pMasterConnection->GetChannelInfo(pUser->GetChannelID());
	if( pUserChInfo == NULL )
		return ERROR_PARTY_INVITEFAIL;

#if defined( PRE_PARTY_DB )
#else
	if( (pUser->m_cVillageID != m_cVillageID) ||(pUserChInfo->nMapIdx != pChInfo->nMapIdx))
		return ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME;
#endif
	if( pUserChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVP || pUserChInfo->nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY )
		return ERROR_PARTY_INVITEFAIL_DESTLOCATION_NOT_SAME;

	if( !pUser->IsNoneWindowState())
		return ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED;

	if( pUser->GetPartyID() > 0 )
		return ERROR_PARTY_INVITEFAIL_ALREADY_HAS_PARTY;

	WCHAR wszInvitePartyName[PARTYNAMELENMAX];
	memset(wszInvitePartyName, 0, sizeof(wszInvitePartyName));

	if (wcslen(GetCharacterName()) + wcslen(CEtUIXML::GetInstance().GetUIString(CEtUIXML::idCategory1, 3535)) >= PARTYNAMELENMAX)
		_wcscpy(wszInvitePartyName, _countof(wszInvitePartyName), GetCharacterName(), (int)wcslen(GetCharacterName()));
	else
		swprintf(wszInvitePartyName, L"%s%s", GetCharacterName(), CEtUIXML::GetInstance().GetUIString(CEtUIXML::idCategory1, 3535));

#if defined( PRE_PARTY_DB )
	if( m_pDBCon && m_pDBCon->GetActive() )
	{
		CSCreateParty CreatePartyPacket;
		memset( &CreatePartyPacket, 0, sizeof(CreatePartyPacket) );

		CreatePartyPacket.PartyType = _NORMAL_PARTY;
		CreatePartyPacket.cPartyMemberMax = NORMPARTYCOUNTMAX;
		CreatePartyPacket.ItemLootRule = ITEMLOOTRULE_INORDER;
		CreatePartyPacket.ItemRank = ITEMRANK_B;
		int iPermitLevel = g_pDataManager->GetMapPermitLevel(GetMapIndex());
		CreatePartyPacket.cUserLvLimitMin = (iPermitLevel == 0) ? 1 : iPermitLevel ;
		CreatePartyPacket.nTargetMapIdx = 0;
		CreatePartyPacket.Difficulty = Dungeon::Difficulty::Easy;
		if( m_bVoiceAvailable )
			CreatePartyPacket.iBitFlag |= Party::BitFlag::VoiceChat;
		_wcscpy( CreatePartyPacket.wszPartyName, _countof(CreatePartyPacket.wszPartyName), wszInvitePartyName, static_cast<int>(wcslen(wszInvitePartyName)) );

		m_pDBCon->QueryAddParty( this, g_Config.nManagedID, Party::LocationType::Village, GetChannelID(), &CreatePartyPacket, pUser->GetAccountDBID() );
	}
#else
	if (g_pMasterConnection && g_pMasterConnection->GetActive())
	{
		g_pMasterConnection->SendReqPartyID(GetAccountDBID(), GetChannelID(), NORMPARTYCOUNTMAX, ITEMLOOTRULE_INORDER, ITEMRANK_B, 1, 100, 0, 0, 1, wszInvitePartyName, L"", pUser->GetAccountDBID(), m_bVoiceAvailable); // ITEMRANK modified by kalliste 2010.07.09
	}
#endif // #if defined( PRE_PARTY_DB )
	else
		return ERROR_PARTY_INVITEFAIL;

	return ERROR_NONE;
}
#endif

int CDNUserSession::_RecvPartyInviteDenied( const CSInviteDenied* pPacket, int iLen )
{
	if( sizeof(CSInviteDenied) - sizeof(pPacket->wszBuf) +(pPacket->cInviterNameLen * sizeof(WCHAR)) != iLen || pPacket->cInviterNameLen >= NAMELENMAX )
		return ERROR_INVALIDPACKET;

	// Clear state
	IsWindowStateNoneSet(WINDOW_ISACCEPT);

#if defined( PRE_PARTY_DB )
	CDNParty* pParty = g_pPartyManager->GetParty(pPacket->PartyIdx);
	if( pParty )
	{
		if( pParty->InviteDenied(GetCharacterName()) == true )
		{
			CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByAccountDBID(pParty->GetLeaderAccountDBID());
			if (pUser)
				pUser->SendPartyInviteDenied(GetCharacterName(), pPacket->cIsOpenBlind > 0 ? ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED : ERROR_NONE);
			return ERROR_NONE;
		}
	}

	WCHAR wszReqCharName[NAMELENMAX];
	memset( wszReqCharName, 0, sizeof(wszReqCharName) );
	memcpy( wszReqCharName, pPacket->wszBuf, sizeof(WCHAR)*pPacket->cInviterNameLen );

	g_pMasterConnection->SendPartyInviteDenied( wszReqCharName, pPacket->PartyIdx, GetCharacterName(), pPacket->cIsOpenBlind );

#else
	CDNParty* pParty = g_pPartyManager->GetParty(pPacket->PartyIdx);
	if( pParty == NULL )	
		return ERROR_NONE;

	if( pParty->InviteDenied(GetCharacterName()) == true )
	{
		CDNUserSession* pUser = g_pUserSessionManager->FindUserSessionByAccountDBID(pParty->GetLeaderAccountDBID());
		if (pUser)
			pUser->SendPartyInviteDenied(GetCharacterName(), pPacket->cIsOpenBlind > 0 ? ERROR_PARTY_INVITEFAIL_DESTUSER_SITUATION_NOTALLOWED : ERROR_NONE);
	}
#endif // #if defined( PRE_PARTY_DB )

	return ERROR_NONE;
}

int CDNUserSession::_RecvPartyInviteOverServerResult( const CSPartyInviteOverServerResult* pPacket, int iLen )
{
	if( sizeof(CSPartyInviteOverServerResult) != iLen )
		return ERROR_INVALIDPACKET;

	IsWindowStateNoneSet(WINDOW_ISACCEPT);
	g_pMasterConnection->SendInvitePartyResult(pPacket->nGameServerID, pPacket->wszInviterName, GetCharacterName(), pPacket->nRetCode);
	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyMemberInfo( const CSPartyMemberInfo* pPacket, int iLen )
{
	if( sizeof(CSPartyMemberInfo) != iLen)
		return ERROR_INVALIDPACKET;

#if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PARTY )
	CDNParty* pParty = g_pPartyManager->GetParty(pPacket->PartyID);
	if( pParty == NULL )
	{
		g_Log.Log(LogType::_PARTYERROR, this, L"[_RecvPartyMemberInfo]Not Exist pParty:%I64d\n", pPacket->PartyID);
		return ERROR_PARTY_NOTFOUND_MEMBERINFO;
	}
	if( Party::bIsWorldCombineParty( pParty->GetPartyType() ) )
	{		
		// Get cross-world party room directly from game server
		g_pMasterConnection->SendGetWorldPartyMember( GetAccountDBID(), GetCharacterDBID(), pParty->GetPartyStructData() );
		return ERROR_NONE;
	}
#endif
	GetDBConnection()->QueryGetPartyJoinMembers( this, pPacket->PartyID );
#else
	CDNParty* pParty = g_pPartyManager->GetParty(pPacket->PartyID);
	if( pParty == NULL )
		return ERROR_PARTY_NOTFOUND_MEMBERINFO;

	pParty->SendPartyMemberInfo( pParty->bIsRaidParty(), this);
#endif // #if defined( PRE_PARTY_DB )
	return ERROR_NONE;
}

int CDNUserSession::_RecvPartyMemberKick( const CSPartyMemberKick* pPacket, int iLen )
{
	if( sizeof(CSPartyMemberKick) != iLen)
		return ERROR_INVALIDPACKET;

	if( m_eUserState != STATE_NONE)		//Cannot kick while moving or at a gate, etc.
		return ERROR_PARTY_KICKFAIL;

	if( pPacket->nSessionID == GetSessionID() )
		return ERROR_PARTY_KICKFAIL_DEST_IS_YOU;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty == NULL )
		return ERROR_PARTY_KICKFAIL;

	if( pParty->bIsMove() || pParty->bIsStartGame())
		return ERROR_PARTY_KICKFAIL;

	CDNUserSession* pUser = g_pUserSessionManager->FindUserSession(pPacket->nSessionID);
	if (pUser == NULL)
		return ERROR_PARTY_KICKFAIL;

	if (pParty->GetLeaderAccountDBID() == GetAccountDBID() )
	{
#if defined( PRE_PARTY_DB )
		g_pPartyManager->QueryOutParty( GetPartyID(), pUser->GetAccountDBID(), pUser->GetCharacterDBID(), pUser->GetSessionID(), pPacket->cKickKind==0 ? Party::QueryOutPartyType::Kick : Party::QueryOutPartyType::EternityKick );
#else
		if( pParty->DelPartyMember(pUser, pPacket->cKickKind) )
		{
			pParty->SendKickPartyMember(pUser->GetSessionID());
			pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
			g_pPartyManager->PushWaitPartyList(pParty);
		}
#endif // #if defined( PRE_PARTY_DB )
	}

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyLeaderSwap( const CSPartyLeaderSwap* pPacket, int iLen )
{
	if( sizeof(CSPartyLeaderSwap) != iLen)
		return ERROR_INVALIDPACKET;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty == NULL )
		return ERROR_PARTY_LEADER_APPOINTMENTFAIL;

	if( pParty->GetLeaderAccountDBID() != GetAccountDBID() )
		return ERROR_PARTY_LEADER_APPOINTMENTFAIL;

	CDNUserSession* pUser = g_pUserSessionManager->FindUserSession(pPacket->nSessionID);
	if (pUser == NULL)
		return ERROR_PARTY_LEADER_APPOINTMENTFAIL;

	if( pParty->ChangePartyLeader(pUser->GetAccountDBID()) == false )
		return ERROR_PARTY_LEADER_APPOINTMENTFAIL;
	pParty->SendAllRefreshParty(PARTYREFRESH_CHANGE_LEADER);

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyChannelList( int iLen )
{
	if( iLen != 0)
		return ERROR_INVALIDPACKET;

	if( m_pField == NULL )	
		return ERROR_GENERIC_UNKNOWNERROR;

	if( GetPartyID() > 0 )
	{
		CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
		if( pParty == NULL )
			return ERROR_GENERIC_UNKNOWNERROR;

		if (pParty->GetLeaderAccountDBID() != GetAccountDBID())
			return ERROR_GENERIC_INVALIDREQUEST;
	}

	SCChannelList packet;
	memset(&packet, 0, sizeof(packet));

	int nMapIndex	= m_pField->GetMapIndex();
	int nChannelID	= m_pField->GetChnnelID();

	if( m_pField->bIsUseLastMapIndex() )
	{
		nMapIndex	= GetLastVillageMapIndex();
		nChannelID	= -1;
	}
	else if( m_pField->bIsPvPLobby() )
	{
		SendChannelInfo(&packet);
		return ERROR_NONE;
	}

	std::vector < std::pair < int, sChannelInfo > >::iterator ii;
	for(ii = g_pMasterConnection->m_vChannelList.begin(); ii != g_pMasterConnection->m_vChannelList.end(); ii++)
	{
		if ((*ii).second.bVisibility == false) 
			continue;

		if ((*ii).second.bShow == false )
			continue;

		// Check level limit
		if ((*ii).second.nLimitLevel > GetLevel() )
			continue;

			// PvP lobby is not always shown.
			if ((*ii).second.nChannelAttribute&GlobalEnum::CHANNEL_ATT_PVPLOBBY )
				continue;

			// PvP channel and Dark Lair channel are always shown.
			if ((*ii).second.nChannelAttribute&(GlobalEnum::CHANNEL_ATT_PVP|GlobalEnum::CHANNEL_ATT_DARKLAIR|GlobalEnum::CHANNEL_ATT_FARMTOWN) )
			{
				// Do not show the PvP channel the user is currently in.
				if ((*ii).second.nMapIdx == m_pField->GetMapIndex() && m_pField->GetChnnelID() == (*ii).second.nChannelID )
					continue;

				//Farm dependency check
				if ((*ii).second.nChannelAttribute&(GlobalEnum::CHANNEL_ATT_FARMTOWN) && (*ii).second.nDependentMapID != nMapIndex)
					continue;

				packet.Info[packet.cCount] = (*ii).second;
				++packet.cCount;
				continue;
			}

		if ((*ii).second.nMapIdx == nMapIndex && (*ii).second.nChannelID != nChannelID )
		{
			packet.Info[packet.cCount] = (*ii).second;
			++packet.cCount;
		}
	}

	SendChannelInfo(&packet);
	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartySelectChannel( const CSVillageSelectChannel* pPacket, int iLen )
{
	if( sizeof(CSVillageSelectChannel) != iLen)
		return ERROR_INVALIDPACKET;

	return CmdSelectChannel( pPacket->nChannelID );
}

int CDNUserSession::_RecvPartyInfoModify( const CSPartyInfoModify* pPacket, int iLen )
{
	if( sizeof(CSPartyInfoModify) != iLen )
		return ERROR_INVALIDPACKET;

	if( GetPartyID() <= 0 )
		return ERROR_GENERIC_INVALIDREQUEST;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty == NULL || pParty->GetLeaderAccountDBID() != GetAccountDBID() )
		return ERROR_GENERIC_INVALIDREQUEST;

	if( wcslen(pPacket->wszName) <= 0 )
		return ERROR_PARTY_MODIFYINFO_FAIL_NEED_NAME;

#if defined( PRE_PARTY_DB )
#else
	if( wcslen(pPacket->wszPass) > PARTYPASSWORDMAX-1 )
		return ERROR_PARTY_MODIFYINFO_FAIL_PASSWORD_ERR;
#endif // #if defined( PRE_PARTY_DB )

	if( wcslen(pPacket->wszName) >= PARTYNAMELENMAX || pPacket->cPartyMemberMax > PARTYMAX || pPacket->cPartyMemberMax < pParty->GetMemberCount())
		return ERROR_PARTY_MODIFYINFO_FAIL;

	if( pPacket->LootRule < ITEMLOOTRULE_NONE || pPacket->LootRule >= ITEMLOOTRULE_MAX)
		return ERROR_PARTY_MODIFYINFO_FAIL;

	if( pPacket->LootRank < ITEMRANK_D || pPacket->LootRank > ITEMRANK_NONE)
		return ERROR_PARTY_MODIFYINFO_FAIL;

#if defined( PRE_PARTY_DB )
	int iPermitLevel = g_pDataManager->GetMapPermitLevel( GetMapIndex() );
	if( pPacket->cUserLvLimitMin > GetLevel() || pPacket->cUserLvLimitMin <= 0 || pPacket->cUserLvLimitMin < iPermitLevel )
		return ERROR_PARTY_MODIFYINFO_FAIL_RESTRICT_LEVEL_ERROR;
#else
	if (pPacket->cUserLvLimitMin > pPacket->cUserLvLimitMax || pPacket->cUserLvLimitMin > GetLevel() || pPacket->cUserLvLimitMin <= 0 || pPacket->cUserLvLimitMax > 100)
		return ERROR_PARTY_MODIFYINFO_FAIL_RESTRICT_LEVEL_ERROR;
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
	int nPass = pPacket->iPassword;
#else
	int nPass = _wtoi(pPacket->wszPass	);
#endif // #if defined( PRE_PARTY_DB )
	if (nPass > MAXPARTYPASSWORD)
		return ERROR_PARTY_MODIFYINFO_FAIL_PASSWORD_ERR;	

#if defined( PRE_PARTY_DB )
	if ( m_pDBCon && m_pDBCon->GetActive() )
		m_pDBCon->QueryModParty( this, pParty, g_Config.nManagedID, Party::LocationType::Village, GetChannelID(), pPacket );
#else  // #if defined( PRE_PARTY_DB )
	int nWaitRet;
#if defined( PRE_PARTY_DB )
	if( pParty->ModifyPartyInfo(GetChannelID(), pPacket->wszName, pPacket->iPassword, pPacket->cPartyMemberMax, pPacket->LootRule, pPacket->LootRank, pPacket->cUserLvLimitMin, pPacket->nTargetMapIdx, pPacket->Difficulty, pPacket->iBitFlag, nWaitRet))
#else
	if( pParty->ModifyPartyInfo(GetChannelID(), pPacket->wszName, pPacket->wszPass, pPacket->cPartyMemberMax, pPacket->LootRule, pPacket->LootRank, pPacket->cUserLvLimitMin, pPacket->cUserLvLimitMax, pPacket->nTargetMapIdx, pPacket->Difficulty, pPacket->cIsJobDice, nWaitRet))
#endif // #if defined( PRE_PARTY_DB )
	{
#if defined( PRE_PARTY_DB )
#else
		//Add to and remove from list
		if( nWaitRet == _PARTY_MOD_RET_ADD)
			g_pPartyManager->PushWaitPartyList(pParty);
		else if (nWaitRet == _PARTY_MOD_RET_DEL)
			g_pPartyManager->EraseWaitPartyList(pParty);
#endif // #if defined( PRE_PARTY_DB )
listinfo:
		pParty->SendAllRefreshParty(PARTYREFRESH_MODIFY_INFO);
	}
#endif  // #if defined( PRE_PARTY_DB )

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyReadyQuest( int iLen )
{
	if( iLen != 0)
		return ERROR_INVALIDPACKET;

	if( GetPartyID() <= 0 )
		return ERROR_GENERIC_INVALIDREQUEST;

	g_pPartyManager->SendPartyReadyRequest(GetPartyID(), GetAccountDBID());
	return ERROR_NONE;
}

int CDNUserSession::_RecvPartyAskJoin( const CSPartyAskJoin* pPacket, int iLen )
{
	if( iLen != sizeof(CSPartyAskJoin) )
		return ERROR_INVALIDPACKET;

	if( GetPartyID() > 0 )
		return ERROR_PARTY_ASKJOIN_FAIL;

	CDNUserSession* pSession = g_pUserSessionManager->FindUserSessionByName(pPacket->wszPartyLeaderName);	
#if defined( PRE_PARTY_DB ) && defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	//Send AskJoin request to master server
	if( pSession == NULL )
	{
		g_pMasterConnection->SendReqPartyAskJoin( this, pPacket->wszPartyLeaderName );
		return ERROR_NONE;
	}
#else
	if( pSession == NULL )
		return ERROR_PARTY_ASKJOIN_FAIL;

	if( pSession->GetMapIndex() != GetMapIndex() )
		return ERROR_PARTY_ASKJOIN_DEST_DIFFMAP;
#endif // #if defined( PRE_PARTY_DB )

	if( pSession->GetPartyID() <= 0 )
		return ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND;

	CDNParty* pParty = g_pPartyManager->GetParty(pSession->GetPartyID());
	if( pParty == NULL )
		return ERROR_PARTY_ASKJOIN_PARTY_NOTFOUND;

	CDNUserSession* pLeader = g_pUserSessionManager->FindUserSessionByAccountDBID(pParty->GetLeaderAccountDBID());
	if( pLeader == NULL )
		return ERROR_PARTY_ASKJOIN_FAIL;

	if( pLeader->GetIsolate()->IsIsolateItem(GetCharacterName()) )
		return ERROR_PARTY_ASKJOIN_FAIL;

	if( pLeader->IsAcceptAbleOption(GetCharacterDBID(), GetAccountDBID(), _ACCEPTABLE_CHECKTYPE_PARTYINVITE) == false)
		return ERROR_PARTY_ASKJOIN_FAIL;

	if( !pLeader->IsNoneWindowState() )
		return ERROR_PARTY_ASKJOIN_DESTUSER_SITUATION_NOTALLOWED;

	pLeader->SetWindowState(WINDOW_ISACCEPT);
	pLeader->SendAskJoinToLeader(GetCharacterName(), GetUserJob(), GetLevel());
	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyAskJoinDecision( const CSPartyAskJoinDecision* pPacket, int iLen )
{
	if( sizeof(CSPartyAskJoinDecision) != iLen)
		return ERROR_INVALIDPACKET;

	// Release the state first.
	IsWindowStateNoneSet(WINDOW_ISACCEPT);

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if (pParty == NULL)
		return ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL;
	
	CDNUserSession* pDestSession = g_pUserSessionManager->FindUserSessionByName(pPacket->wszPartyAsker);
#if defined( PRE_PARTY_DB ) && defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	//Send AskJoin request to master server
	if( pDestSession == NULL )
	{
		if(pPacket->cRet == 0)
			g_pMasterConnection->SendResPartyAskJoin( ERROR_PARTY_ASKJOIN_RESULT_PARTYLEADER_DENIED, 0, pPacket->wszPartyAsker );
		else
			g_pMasterConnection->SendAskJoinAgreeInfo( pPacket->wszPartyAsker, pParty->GetPartyID(), pParty->GetPassword() );
		return ERROR_NONE;
	}
#else
	if (pDestSession == NULL)
		return ERROR_PARTY_ASKJOIN_RESULT_AGREE_FAIL;
#endif

	if( pPacket->cRet == 0 )
	{
		pDestSession->SendAskJoinResult(ERROR_PARTY_ASKJOIN_RESULT_PARTYLEADER_DENIED);
		return ERROR_NONE;
	}
	
#if defined( PRE_PARTY_DB )
	pDestSession->SendAskJoinAgreeInfo(pParty->GetPartyID(), pParty->GetPassword() );
#else
	pDestSession->SendAskJoinAgreeInfo(pParty->GetPartyID(), pParty->m_wszPartyPassword);
#endif // #if defined( PRE_PARTY_DB )

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartySwapMemberIndex( const CSPartySwapMemberIndex* pPacket, int iLen )
{
	if( sizeof(CSPartySwapMemberIndex) != iLen )
		return ERROR_INVALIDPACKET;

	if( GetPartyID() <= 0 )
		return ERROR_PARTY_SWAPINDEX_FAIL;

	CDNParty* pParty = g_pPartyManager->GetParty(GetPartyID());
	if( pParty == NULL )
		return ERROR_PARTY_SWAPINDEX_FAIL;

	if( pParty->GetLeaderAccountDBID() != GetAccountDBID() )
		return ERROR_PARTY_SWAPINDEX_FAIL;

	if( pParty->SwapMemberIndex(pPacket) == false )
		return ERROR_PARTY_SWAPINDEX_FAIL;

	return ERROR_NONE;
}

int	CDNUserSession::_RecvPartyConfirmLastDungeonInfo( const CSPartyConfirmLastDungeonInfo* pPacket, int iLen )
{
	if( sizeof(CSPartyConfirmLastDungeonInfo) != iLen )
		return ERROR_INVALIDPACKET;

	if( bIsCheckLastDungeonInfo() == false )
		return ERROR_NONE;


	if ( pPacket->bIsOK == true )
	{
		INT64 biValue = 0;
		if ( GetCommonVariableDataValue( CommonVariable::Type::LastPartyDungeonInfo, biValue ) == false || biValue == 0 )
			return ERROR_NONE;

		g_pMasterConnection->SendConfirmLastDungeonInfo( this, biValue );
	}
	else
	{
		INT64 biValue = 0;
		if ( GetCommonVariableDataValue( CommonVariable::Type::LastPartyDungeonInfo, biValue ) == true || biValue > 0 )
#if defined( PRE_PARTY_DB )
			g_pMasterConnection->SendConfirmLastDungeonInfo( this, biValue, BreakInto::Type::PartyRestore, false );
#else
			g_pMasterConnection->SendConfirmLastDungeonInfo( this, biValue, false );
#endif // #if defined( PRE_PARTY_DB )

		ModCommonVariableData( CommonVariable::Type::LastPartyDungeonInfo, 0 );
	}

	return ERROR_NONE;
}
