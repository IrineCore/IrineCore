#include "stdafx.h"
#include "DNGuildVillage.h"
#include "DNMasterConnection.h"
#include "DNGameDataManager.h"
#include "DNSchedule.h"
#include "DNGuildWarManager.h"

CDNGuildVillage::CDNGuildVillage() 
{
	m_GuildWare = NULL;
	m_nResetWareHour = 4;
	GuildWarReset();	
};

CDNGuildVillage::~CDNGuildVillage() 
{

};

void CDNGuildVillage::Reset()
{
	if (m_GuildWare)
		m_GuildWare->ReportWareSlot();

	CDNGuildBase::Reset ();
	GuildWarReset();

	SAFE_DELETE(m_GuildWare);	
}

void CDNGuildVillage::OpenWare(const TAGetGuildWareInfo* pPacket)
{
	if (m_GuildWare == NULL)
		m_GuildWare = new CDNGuildWare();

	if (m_GuildWare)
		m_GuildWare->Open(this, pPacket);
	else
		_DANGER_POINT ();
}

// Check if guild member
bool CDNGuildVillage::IsValiadMember(CDNUserSession *pUserObj, INT64 nCharacterDBID)
{
	if (!pUserObj || !pUserObj->GetGuildUID().IsSet() || GetUID() != pUserObj->GetGuildUID() || (nCharacterDBID != pUserObj->GetCharacterDBID()))
		return false;

	return true;
}

// Broadcast to all logged-in guild members
void CDNGuildVillage::SendGuildMemberLoginList(SCGuildMemberLoginList* pPacket)
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID (iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendGuildMemberLoginList(pPacket);
	}
}

// Check guild resource.
bool CDNGuildVillage::CheckGuildResource(TGuildMember* pGuildMemeber)
{
	if (false == CDNGuildBase::CheckGuildResource(pGuildMemeber))
		return false;

	CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID (pGuildMemeber->nAccountDBID);

	// Check user state
	if (pUserObj && pUserObj->GetGuildUID().IsSet())
	{
		// Check guild info
		// Even if you get the user session pointer, synchronization is not guaranteed, so use it for verification only!
		if (GetUID() == pUserObj->GetGuildUID() && (pGuildMemeber->nCharacterDBID == pUserObj->GetCharacterDBID()))
			return false;
	}

	// Continuously query the member list
	return true;
}

void CDNGuildVillage::SendAddGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszCharacterName, TP_JOB nJob, char cLevel, __time64_t pJoinDate, TCommunityLocation* pLocation)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendInviteGuildMemberAck (nAccountDBID, nCharacterDBID, lpwszCharacterName, ERROR_NONE, 0, 0, nJob, cLevel, pLocation, &GetUID());
	}
}

void CDNGuildVillage::SendDelGuildMember(UINT nAccountDBID, INT64 nCharacterDBID, bool bIsExiled)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		// Check if winning guild
		if( pUserObj && g_pGuildWarManager && g_pGuildWarManager->GetPreWinGuildUID().IsSet() 
			&& GetUID() == g_pGuildWarManager->GetPreWinGuildUID() && nAccountDBID == pUserObj->GetAccountDBID() )
			pUserObj->SendGuildWarPreWinGuild(false);
		
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		if( nCharacterDBID == pUserObj->GetCharacterDBID() )
		{

			pUserObj->GetItem()->RemoveGuildReversionItem( true );
			pUserObj->GetItem()->RemoveGuildReversionVehicleItemData( true );
			pUserObj->SendGuildRewardItem(NULL);
		}
		if (bIsExiled)
			pUserObj->SendExileGuildMember(nAccountDBID, nCharacterDBID, ERROR_NONE, &GetUID());
		else
			pUserObj->SendLeaveGuildMember(nAccountDBID, nCharacterDBID, ERROR_NONE, &GetUID(), false);
	}
}

void CDNGuildVillage::SendChangeGuildInfo(UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iInt1, int iInt2, int iInt3, int iInt4, INT64 biInt64, LPCWSTR pText)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendChangeGuildInfoEx(nAccountDBID, nCharacterDBID, btGuildUpdate, ERROR_NONE, iInt1, iInt2, iInt3, iInt4, biInt64, pText, &GetUID());
	}
}

void CDNGuildVillage::SendChangeGuildMemberInfo(UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID (iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendChangeGuildMemberInfo (nReqAccountDBID, nReqCharacterDBID, nChgAccountDBID, nChgCharacterDBID, btGuildMemberUpdate, ERROR_NONE, iInt1, iInt2, biInt64, pText, &GetUID());

		// [Warning] Must be processed based on character ID.
		if (nChgCharacterDBID == pUserObj->GetCharacterDBID())
		{
			switch(btGuildMemberUpdate) 
			{
			case GUILDMEMBUPDATE_TYPE_ROLE: // Change rank
				{
					pUserObj->GetGuildSelfView().btGuildRole = static_cast<BYTE>(iInt1);

					// Update guild visual info
					pUserObj->RefreshGuildSelfView();
				}
				break;
			
			case GUILDMEMBUPDATE_TYPE_GUILDMASTER: // Delegate guild master
				{
					pUserObj->GetGuildSelfView().btGuildRole = static_cast<BYTE>(GUILDROLE_TYPE_MASTER);

					// Update guild visual info
					pUserObj->RefreshGuildSelfView();
				}
				break;
			default:
				break;
			}
		}
	}
}

void CDNGuildVillage::SendGuildChat(UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList(List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendGuildChat(GetUID(), nAccountDBID, nCharacterDBID, lpwszChatMsg, nLen);
	}
}

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
void CDNGuildVillage::SendDoorsGuildChat(INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList(List);

	if (List.empty())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();
	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendDoorsGuildChat(GetUID(), nCharacterDBID, lpwszChatMsg, nLen);
	}
}
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

void CDNGuildVillage::SendChangeGuildName(const wchar_t* wszGuildName)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendChangeGuildName(wszGuildName);
		_wcscpy( pUserObj->GetGuildSelfView().wszGuildName, _countof(pUserObj->GetGuildSelfView().wszGuildName), wszGuildName, (int)wcslen(wszGuildName) );
		pUserObj->RefreshGuildSelfView();
	}
}

void CDNGuildVillage::SendChangeGuildMark(const MAGuildChangeMark *pPacket)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendChangeGuildMark(pPacket);
		pUserObj->GetGuildSelfView().wGuildMark = pPacket->wMark;
		pUserObj->GetGuildSelfView().wGuildMarkBG = pPacket->wMarkBG;
		pUserObj->GetGuildSelfView().wGuildMarkBorder = pPacket->wMarkBorder;
		pUserObj->RefreshGuildSelfView();
	}
}

void CDNGuildVillage::SendExtendGuildWare(short wTotalSize)
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendExtendGuildWare(wTotalSize);
	}
}

void CDNGuildVillage::SendGuildLevelUp (int nLevel)
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		pUserObj->SendGuildLevelUp(nLevel);
	}
}

void CDNGuildVillage::SendUpdateGuildExp(BYTE cPointType, int nPointValue, int nGuildExp, INT64 biCharacterDBID, int nMissionID)
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember (pUserObj, iter->second))
			continue;

		if (cPointType == GUILDPOINTTYPE_STAGE)
		{
			if (pUserObj->GetCharacterDBID() == biCharacterDBID)
			{
				pUserObj->SendUpdateGuildExp(ERROR_NONE, cPointType, nPointValue, nGuildExp, biCharacterDBID, nMissionID);
				break;
			}
		}
		else
			pUserObj->SendUpdateGuildExp(ERROR_NONE, cPointType, nPointValue, nGuildExp, biCharacterDBID, nMissionID);
	}
}

#ifdef PRE_ADD_JOINGUILD_SUPPORT
void CDNGuildVillage::SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession, bool bNoUpdate)
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
void CDNGuildVillage::SendRefreshGuildItem(char cType, INT64 biSrcSerial, INT64 biDestSerial, const TItemInfo *pSrcInfo, const TItemInfo *pDestInfo, CDNUserSession* pSession)
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
{
	// Return if no member
	if (m_MapTotalMember.empty())
		return;

	// Return if guild does not exist
	if (!m_GuildWare)
		return;

	ScopeLock <CSyncLock> AutoLock (GetLock());

	std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.begin ();

	for (; iter != m_MapTotalMember.end(); iter++)
	{
		TGuildMember* pGuildMember = &(iter->second);
	
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pGuildMember->nAccountDBID);
		if (false == IsValiadMember (pUserObj, pGuildMember->nCharacterDBID))
			continue;

		if (pUserObj == pSession)
		{
			// Update version for the requester itself.
#ifdef PRE_ADD_JOINGUILD_SUPPORT
			if (bNoUpdate == true)
				continue;

			pGuildMember->WareVeriosnDate = m_GuildWare->GetVersionDate();
#else		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
			pGuildMember->WareVeriosnDate = m_GuildWare->GetVersionDate();
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
			continue;
		}

		// Do not update version for users who have not opened the guild window.
		if (pGuildMember->bOpenWareStatus == false)
			continue;

		pGuildMember->WareVeriosnDate = m_GuildWare->GetVersionDate();

		pUserObj->SendRefreshGuildItem(cType, biSrcSerial, biDestSerial, pSrcInfo, pDestInfo);
	}
}

void CDNGuildVillage::SendRefreshGuildCoin(INT64 biTotalCoin, CDNUserSession* pSession)
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		if (pUserObj == pSession)
			continue;

		pUserObj->SendRefreshGuildCoin(biTotalCoin);
	}
}

void CDNGuildVillage::UpdateGuildInfo(CDNUserSession* pSession, const TAChangeGuildInfo *pPacket)
{
	switch(pPacket->btGuildUpdate) 
	{
	case GUILDUPDATE_TYPE_NOTICE:			// Change notice
		{
			{
				ScopeLock <CSyncLock> AutoLock (GetLock());

				_wcscpy(SetInfo()->wszGuildNotice, _countof(GetInfo()->wszGuildNotice), pPacket->Text, (int)wcslen(pPacket->Text));
			}
		}
		break;

	case GUILDUPDATE_TYPE_ROLEAUTH:			// Change rank permissions
		{
			SetInfo()->wGuildRoleAuth[pPacket->nInt1] = static_cast<int>(pPacket->nInt2);
			SetInfo()->nRoleMaxItem[pPacket->nInt1] = static_cast<int>(pPacket->nInt3);
			SetInfo()->nRoleMaxCoin[pPacket->nInt1] = static_cast<int>(pPacket->nInt4);
		}
		break;
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	case GUILDUPDATE_TYPE_HOMEPAGE:
		{
			{
				ScopeLock <CSyncLock> AutoLock (GetLock());
				_wcscpy(SetInfo()->wszGuildHomePage, _countof(GetInfo()->wszGuildHomePage), pPacket->Text, (int)wcslen(pPacket->Text));
			}			
		}
		break;
#endif
		

	
	default:
		{
			pSession->SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, ERROR_GENERIC_INVALIDREQUEST, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
			return;
		}
		break;
	}

	// Notify guild members currently connected to this server
	SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, pPacket->nInt1, pPacket->nInt2, pPacket->nInt3, pPacket->nInt4, pPacket->biInt64, pPacket->Text);

	// Specify to also update general guild info on other servers via MA
	if (g_pMasterConnection)
		g_pMasterConnection->SendChangeGuildInfo(GetUID(), pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, 
													pPacket->nInt1, pPacket->nInt2, pPacket->nInt3, pPacket->nInt4, pPacket->biInt64, pPacket->Text);
}

CDNGuildWare* CDNGuildVillage::GetGuildWare()
{
	return m_GuildWare;
}

void CDNGuildVillage::RecvCsCloseGuildWare(CDNUserSession * pSession)
{
	// Check if this is the first person to open the warehouse.
	TGuildMember* pGuildMember = GetMemberInfo(pSession->GetCharacterDBID());
	if (pGuildMember)
	{
		pGuildMember->bOpenWareStatus = false;
	}
}

void CDNGuildVillage::RecvApiOpenGuildWare(CDNUserSession * pSession)
{
	if (!m_GuildWare)
	{
		// If guild warehouse info is not populated, re-request from DB.
		ReqGetGuildWareInfo();
		g_Log.Log(LogType::_ERROR, pSession, L"ReqGetGuildWareInfo[CDBID:%I64d][GuildDBID:%d][WorldSetID:%d]\r\n", pSession->GetCharacterDBID(), GetUID().nDBID, pSession->GetWorldSetID());
		return;
	}

	TGuildMember* pGuildMember = GetMemberInfo(pSession->GetCharacterDBID());
	if (!pGuildMember)
		return;

	// Check warehouse usage count reset.
	CheckResetWareLimit (pGuildMember);

	// Check if this is the first person to open the warehouse.
	if (pGuildMember->WareVeriosnDate != m_GuildWare->GetVersionDate())
		SendGuildWareList (pSession, 0);
	else 
		SendGuildWareList (pSession, 1);


	pGuildMember->WareVeriosnDate = m_GuildWare->GetVersionDate();
	pGuildMember->bOpenWareStatus = true;
}

void CDNGuildVillage::CheckResetWareLimit(TGuildMember* pGuildMember)
{
	if (!pGuildMember)
		return;

	if (pGuildMember->LastWareUseDate < 0)
		return;

	tm tLastTime = *localtime(&pGuildMember->LastWareUseDate);

	time_t tCurTime;
	time(&tCurTime);

	tm tCurWareTime = *localtime(&tCurTime);


	tm tResetTime;
	tResetTime = tCurWareTime;
	tResetTime.tm_hour = m_nResetWareHour;
	tResetTime.tm_min = 0;
	tResetTime.tm_sec = 0;

	if (tCurWareTime.tm_hour < m_nResetWareHour)
		tResetTime.tm_yday--;

	if (tLastTime.tm_yday < tResetTime.tm_yday)
	{
		pGuildMember->TakeWareItemCount = 0;
		pGuildMember->WithdrawCoin = 0;	
	}
	else if (tLastTime.tm_yday == tResetTime.tm_yday && tLastTime.tm_hour < tResetTime.tm_hour)
	{
		pGuildMember->TakeWareItemCount = 0;
		pGuildMember->WithdrawCoin = 0;	
	}
}

void CDNGuildVillage::RecvCsGetGuildWareHistory(CDNUserSession * pSession, CSGetGuildWareHistory *pPacket)
{
	if (m_GuildWare)
		m_GuildWare->OnRecvGetWareHistory(pSession, pPacket);
}

void CDNGuildVillage::RecvMaRefreshGuildItem(MARefreshGuildItem* pPacket)
{
	if (!m_GuildWare)
		return; // Return because info may arrive from another server before guild warehouse is created

	switch (pPacket->cMoveType)
	{
		case MoveType_GuildWare:
		{
			// Move to empty slot
			if (pPacket->biDestSerial == 0)
				m_GuildWare->UpdateWareInfo(pPacket->DestInfo);
			else // Merge case
			{
				m_GuildWare->RemoveWareItem(pPacket->biDestSerial);
				m_GuildWare->AddWareItemInfo(pPacket->DestInfo);
			}
		}
		break;

		case MoveType_InvenToGuildWare:
		{
			// Move to empty slot
			if (pPacket->biDestSerial == 0)
				m_GuildWare->UpdateWareInfo(pPacket->DestInfo);
			else // Merge case
			{
				m_GuildWare->RemoveWareItem(pPacket->biDestSerial);
				m_GuildWare->AddWareItemInfo(pPacket->DestInfo);
			}			
		}	
		break;

		case MoveType_GuildWareToInven:
		{
			m_GuildWare->UpdateWareInfo (pPacket->SrcInfo);
		}
		break;

#ifdef PRE_ADD_JOINGUILD_SUPPORT
		case MoveType_MailToGuildWare:
		{
			m_GuildWare->AddWareItemInfo(pPacket->DestInfo);
		}
		break;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	}

	// Set the version.
	m_GuildWare->SetVersionDate(pPacket->VersionDate);

	// Notify guild members of info.
	SendRefreshGuildItem (pPacket->cMoveType, pPacket->biSrcSerial, pPacket->biDestSerial, &pPacket->SrcInfo, &pPacket->DestInfo);
}

void CDNGuildVillage::RecvMaRefreshGuildCoin(MARefreshGuildCoin* pPacket)
{
	SetInfo()->ui64GuildMoney = pPacket->biTotalCoin;

	SendRefreshGuildCoin (pPacket->biTotalCoin);
}
void CDNGuildVillage::UpdateMemberInfo(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	DN_ASSERT(IsOpen(),								"Not Opened!");
	DN_ASSERT(IsAttach(),							"Not Attached!");
	DN_ASSERT(0 != pPacket->nReqCharacterDBID,	"Invalid!");

	switch (pPacket->btGuildMemberUpdate)
	{

		// Change self-introduction (guild member self)
	case GUILDMEMBUPDATE_TYPE_INTRODUCE:	
		ModifyMembIntro(pSession, pPacket);
		break;

		// Change rank
	case GUILDMEMBUPDATE_TYPE_ROLE:			
		ModifyMembRole(pSession, pPacket);
		break;

		// Delegate guild master
	case GUILDMEMBUPDATE_TYPE_GUILDMASTER:  
		ChangeGuildMaster(pSession, pPacket);
		break;

		// Normal point increase/decrease (contribution)
	case GUILDMEMBUPDATE_TYPE_CMMPOINT:		
		ApplyGuildPoint(pSession, pPacket);
		break;

		// Handle the rest as errors!
	default:	
		pSession->SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, 
			ERROR_GENERIC_INVALIDREQUEST, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text, NULL);
		break;
	}

	// Notify guild members currently connected to this server
	SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text);

	// Notify guild members connected to other servers via MA
	if (g_pMasterConnection)
		g_pMasterConnection->SendChangeGuildMemberInfo(GetUID(), pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, pPacket->nInt1, pPacket->nInt2, pPacket->biInt64, pPacket->Text);
}

void CDNGuildVillage::GuildWarReset()
{
	CDNGuildBase::GuildWarReset();
	m_bGuildWarStats = false;
	m_nGuildWarPoint = 0;
	memset(&m_sMyGuildWarRankingInfo, 0, sizeof(m_sMyGuildWarRankingInfo));
	memset(&m_sMyGuildWarMissionGuildRankingInfo, 0, sizeof(m_sMyGuildWarMissionGuildRankingInfo));
}

void CDNGuildVillage::UpdateWarInfoOnAllMember(short wScheduleID, BYTE cTeamColorCode)
{
	if (g_pGuildWarManager && g_pGuildWarManager->GetScheduleID() != wScheduleID)
		return;

	m_Info.GuildView.wWarSchduleID = wScheduleID;
	m_Info.GuildView.cTeamColorCode = cTeamColorCode;

	ScopeLock <CSyncLock> AutoLock (GetLock());

	std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.begin ();

	for (; iter != m_MapTotalMember.end(); iter++)
	{
		TGuildMember* pGuildMember = &(iter->second);

		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(pGuildMember->nAccountDBID);
		if (false == IsValiadMember (pUserObj, pGuildMember->nCharacterDBID))
			continue;

		pUserObj->GetGuildSelfView().wWarSchduleID = wScheduleID;
		pUserObj->GetGuildSelfView().cTeamColorCode = cTeamColorCode;
		pUserObj->RefreshGuildSelfView();

		// Notify each user of the guild war registration.
		pUserObj->SendEnrollGuildWar(ERROR_NONE, wScheduleID, cTeamColorCode);
	}
}

void CDNGuildVillage::SendGuildWarPreWinGuild(bool bPreWin)
{
	ScopeLock <CSyncLock> AutoLock (GetLock());

	std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.begin ();

	for (; iter != m_MapTotalMember.end(); iter++)
	{
		TGuildMember* pGuildMember = &(iter->second);

		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID (pGuildMember->nAccountDBID);
		if (false == IsValiadMember (pUserObj, pGuildMember->nCharacterDBID))
			continue;		

		// Notify each user of the guild war victory.
		pUserObj->SendGuildWarPreWinGuild(bPreWin);
	}
}

void CDNGuildVillage::SetGuildWarMyRankingInfo(TAGetGuildWarPointGuildTotal* pGetGuildWarPointGuildTotal )
{	
	m_sMyGuildWarRankingInfo.wRanking = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[0].wRanking;
	m_sMyGuildWarRankingInfo.nTotalPoint = pGetGuildWarPointGuildTotal->sGuildWarPointGuildTotal[0].nOpeningPointSum;
}

void CDNGuildVillage::SetGuildWarTotalRankingInfo(TAGetGuildWarPointGuildPartTotal* pGetGuildWarPointGuildPartTotal )
{
	for( int i=0; i<GUILDWAR_RANKINGTYPE_MAX; ++i)
	{
		m_sMyGuildWarMissionGuildRankingInfo[i].wRanking = pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].wRanking;
		m_sMyGuildWarMissionGuildRankingInfo[i].nTotalPoint = pGetGuildWarPointGuildPartTotal->GuildWarPointPartTotal[i].nPoint;
	}
	m_bGuildWarStats = true;
}
// Guild preliminary match guild point formula
int	CDNGuildVillage::CalcGuildWarTrialPoint()
{
	//Guild earned mission points + (guild earned mission points * total special awards won by guild * 0.1)
	return (int)(m_nGuildWarPoint+(m_nGuildWarPoint*g_pGuildWarManager->GetGuildAwardCount(GetUID().nDBID)*0.1f));
}

void CDNGuildVillage::ReqGetGuildWareInfo(bool bForceReq)
{
	if (bForceReq == false)
	{
		if (m_GuildWare)
			return;
	}

	// Request guild warehouse
	if (m_pDBCon)
		m_pDBCon->QueryGetGuildWareInfo(m_cDBThreadID, GetUID().nDBID, GetUID().nWorldID, GetWareSize());
	
}

void CDNGuildVillage::UpdateWareSize(CDNUserBase* pUserObject, short wSize)
{
	SetInfo()->wGuildWareSize = wSize;

	if (m_GuildWare)
		m_GuildWare->ChangeWareSize(pUserObject, wSize);
	else
		_ASSERT(0);
}

void CDNGuildVillage::SendGuildRewardItem( TGuildRewardItem* GuildRewardItem )
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendGuildRewardItem(GuildRewardItem);
	}
}
void CDNGuildVillage::SendAddGuildRewardItem( TGuildRewardItem GuildRewardItem )
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendAddGuildRewardItem(GuildRewardItem);
	}
}
void CDNGuildVillage::SendExtendGuildSize(short nGuildSize)
{
	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>> List;
	GetMemberList (List);

	if (List.empty ())
		return;

	std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>::iterator iter = List.begin ();

	for (; iter != List.end (); iter++)
	{
		CDNUserSession *pUserObj = g_pUserSessionManager->FindUserSessionByAccountDBID(iter->first);
		if (false == IsValiadMember(pUserObj, iter->second))
			continue;

		pUserObj->SendExtendGuildSize(nGuildSize);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////						PRIVATE FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CDNGuildVillage::SendGuildWareList(CDNUserSession * pSession, int nResult)
{
	SCOpenGuildWare Packet;
	memset (&Packet, 0x00, sizeof(Packet));

	Packet.nError = ERROR_NONE;
	Packet.nResult = nResult;
	Packet.wWareSize = GetWareSize();

	int nSize = sizeof(Packet.nError) + sizeof (Packet.nResult) + sizeof(Packet.wWareSize);
	
	if (!m_GuildWare)
		Packet.nError = ERROR_GUILDWARE_NOT_READY;
	else
	{

		TGuildMember* pGuildMember = GetMemberInfo(pSession->GetCharacterDBID());
		if (pGuildMember)
		{
			Packet.nTakeWareItemCount = pGuildMember->TakeWareItemCount;
			Packet.nWithdrawCoin = pGuildMember->WithdrawCoin;

			nSize += sizeof(Packet.nTakeWareItemCount) + sizeof(Packet.nWithdrawCoin);
		
			if (GetWareSize() <= 0)
				Packet.nError = ERROR_GUILDWARE_NOT_READY;
			else
			{

				Packet.nGuildCoin = m_GuildWare->GetWarehouseCoin();
				nSize += sizeof(Packet.nGuildCoin);

				if (nResult == 0)
				{				
					Packet.VersionDate = m_GuildWare->GetVersionDate();
					nSize += sizeof(Packet.VersionDate);

					Packet.cWareCount = m_GuildWare->GetWareList (Packet.WareList);
					nSize += (sizeof(TItemInfo)*Packet.cWareCount) + sizeof(Packet.cWareCount);
					
				}
			}
			
		}
		else
			Packet.nError = ERROR_GUILD_YOUR_NOT_BELONGANYGUILD;
			
	}

	pSession->AddSendData (SC_GUILD, eGuild::SC_OPEN_GUILDWARE, (char*)&Packet, nSize);
}

// Modify guild member introduction
void CDNGuildVillage::ModifyMembIntro(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	{
		ScopeLock <CSyncLock> AutoLock (GetLock());

		TGuildMember* pGuildMember = GetMemberInfoAsync(pPacket->nReqCharacterDBID);
		if (pGuildMember)
			_wcscpy(pGuildMember->wszGuildMemberIntroduce, _countof(pGuildMember->wszGuildMemberIntroduce), pPacket->Text, (int)wcslen(pPacket->Text));
	}

	USES_CONVERSION;
}

// Change guild member rank.
void CDNGuildVillage::ModifyMembRole(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	ScopeLock <CSyncLock> AutoLock (GetLock());

	TGuildMember* pGuildMember = GetMemberInfoAsync(pPacket->nChgCharacterDBID);

	if (!pGuildMember)
		return;

	// Change rank
	pGuildMember->btGuildRole = static_cast<BYTE>(pPacket->nInt1);
}

// Delegate guild master.
void CDNGuildVillage::ChangeGuildMaster(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	WCHAR aCharacterName[NAMELENMAX] = { L'\0', };
	TGuildMember* pGuildMember = NULL;
	{
		ScopeLock <CSyncLock> AutoLock (GetLock());

		pGuildMember = GetMemberInfoAsync (pPacket->nChgCharacterDBID);

		// Change role to master
		if (pGuildMember)
		{
			pGuildMember->btGuildRole = static_cast<BYTE>(GUILDROLE_TYPE_MASTER);
			_wcscpy(aCharacterName, _countof(aCharacterName), pGuildMember->wszCharacterName, (int)wcslen(pGuildMember->wszCharacterName));
			SetGuildMaster( pGuildMember );
		}

		pGuildMember = GetMemberInfoAsync(pPacket->nReqCharacterDBID);

		// Master->Sub-master (change)
		if (pGuildMember)
			pGuildMember->btGuildRole = static_cast<BYTE>(GUILDROLE_TYPE_SUBMASTER);	

	}

	// Update personal session info
	pSession->GetGuildSelfView().btGuildRole = static_cast<BYTE>(GUILDROLE_TYPE_SUBMASTER);

	// Update guild visual info
	pSession->RefreshGuildSelfView();

	USES_CONVERSION;
}

// Apply guild pointer
void CDNGuildVillage::ApplyGuildPoint(CDNUserSession* pSession, const TAChangeGuildMemberInfo *pPacket)
{
	ScopeLock <CSyncLock> AutoLock (GetLock());

	TGuildMember* pGuildMember = GetMemberInfoAsync(pPacket->nReqCharacterDBID);

	if (pGuildMember)
	{
		pGuildMember->iUseGuildCommonPoint = static_cast<long>(pPacket->nInt1);
		pGuildMember->iTotGuildCommonPoint = static_cast<long>(pPacket->nInt2);
	}
}
