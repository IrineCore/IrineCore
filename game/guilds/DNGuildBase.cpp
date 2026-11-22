#include "stdafx.h"
#include "Util.h"
#include "DNWorldUserState.h"
#include "DNUserSession.h"

#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"
#include "DNGameDataManager.h"

#include "DNGuildBase.h"
#include "EtUIXML.h"
#if defined(_VILLAGESERVER)
#include "DNGuildWarManager.h"
#elif defined(_GAMESERVER)
#include "DNMasterConnectionManager.h"
#endif //#if defined(_VILLAGESERVER)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////						PUBLIC FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDNGuildBase::CDNGuildBase()
{
	m_bOpen				= false;
	m_pManager			= NULL;
	m_pDBCon			= NULL;
	GuildWarReset();	
	m_GuildMaster = NULL;
	m_bRecvAllMember = false;
}

CDNGuildBase::~CDNGuildBase()
{

}

DWORD CDNGuildBase::Open(CDNGuildSystem* pManager)
{
	// Handle error for duplicate open.
	if(true == IsOpen())
		return(HASERROR+0);

	m_bOpen			= true;
	m_pManager		= pManager;

#if !defined( PRE_ADD_NODELETEGUILD )
	// Initialize access reference object
	DWORD dwRetVal = m_Access.Open(FALSE);
	if(NOERROR != dwRetVal)
		DN_RETURN(dwRetVal);
#endif

	Reset();

	return NOERROR;
}

void CDNGuildBase::Close()
{
#if !defined( PRE_ADD_NODELETEGUILD )
	m_Access.Close();
#endif
	m_bOpen = false;
}

void CDNGuildBase::Release()
{
	if(!m_pManager) return;

	DWORD dwRetVal = m_pManager->Release(this);
	if(NOERROR != dwRetVal) 
	{
		// Error - terminate completely if an error occurs during return
		DN_ASSERT(0,	"Check!");
		//		Close();
	}
}

void CDNGuildBase::Reset()
{
	// Initialize guild info
	m_Info.Reset();
	m_TickCheckMember	= 0;
	m_bCheckEmpty		= false;

#if defined(_DEBUG)
	m_bSkipDisableOnDebugging = false;
#endif	// _DEBUG

	////////////////////////////////////////////////////////////
	// Reset guild member info
	ScopeLock <CSyncLock> AutoLock(GetLock());
	m_MapTotalMember.clear();
	m_GuildMaster = NULL;
}

void CDNGuildBase::Set(const TGuild* pInfo)
{
	// Possible because it is P.O.D format
	m_Info = (*pInfo);

	// Get DB object
	if(!m_pDBCon)
		m_pDBCon = g_pDBConnectionManager->GetDBConnection( m_cDBThreadID );

	CheckGuildInfo();
}

#if defined(_DEBUG)
#if !defined( PRE_ADD_NODELETEGUILD )
bool CDNGuildBase::IsLock() const
{
	return((m_Access.IsLock())?(true):(false));
}
#endif
#endif	// _DEBUG

#if !defined( PRE_ADD_NODELETEGUILD )
void CDNGuildBase::Attach()
{
	DN_ASSERT(IsOpen(),		"Not Opened!");

	ScopeLock <CSyncLock> AutoLock(GetLock());

	if(m_Access.CanAttachNoLock()) 
		m_Access.AttachMain();
}

void CDNGuildBase::Detach()
{
	DN_ASSERT(IsOpen(),		"Not Opened!");

	if(m_Access.Detach()) 
		DetachWork();
}

void CDNGuildBase::DetachWork()
{
	DN_ASSERT(IsOpen(),		"Not Opened!");

	Release();
}

void CDNGuildBase::DisableAttach()
{
	m_Access.DisableAttach();
}

void CDNGuildBase::DisableAttachNoLock()
{
	m_Access.DisableAttachNoLock();
}

bool CDNGuildBase::IsAttach() const
{
	DN_ASSERT(IsOpen(),	"Not Opened!");

	return(m_Access.IsAttach()?(true):(false));
}

bool CDNGuildBase::IsEnable() const
{
	DN_ASSERT(IsOpen(),	"Not Opened!");

	return(m_Access.IsEnable()?(true):(false));
}

DWORD CDNGuildBase::GetAccessCount() const
{
	DN_ASSERT(IsOpen(),	"Not Opened!");

	return(m_Access.GetAccessCount());
}
#endif

void CDNGuildBase::ResetAccess()
{
	m_Access.Reset();
}

const TGuild* CDNGuildBase::GetInfo() const
{ 
	return(&m_Info);
}

TGuild* CDNGuildBase::SetInfo()
{
	return(&m_Info);
}

void CDNGuildBase::DoUpdate(DWORD CurTick)
{
	// Check every 30 seconds.
	if(GUILDCHKRESCTERM > GetTickTerm(m_TickCheckMember, CurTick))
		return;

	// Save the current tick.
	m_TickCheckMember = CurTick;

	bool bIsEmpty = true;

	SCGuildMemberLoginList Packet;
	memset(&Packet, 0, sizeof(Packet));
	Packet.GuildUID = GetUID();

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Sync start
	{
		ScopeLock <CSyncLock> AutoLock(GetLock());

		std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.begin();

		for(; iter != m_MapTotalMember.end(); iter++)
		{
			TGuildMember* pGuildMemeber = &(iter->second);

			// [Warning] Double synchronization occurs in the function below

#if !defined( PRE_ADD_NODELETEGUILD )	// No need to call CheckGuildResource since guild resources are not released
			// Check whether resource exists
			if(bIsEmpty)
				bIsEmpty = CheckGuildResource(pGuildMemeber);
#endif

			// Check member status
			if(true == UpdateMemberState(pGuildMemeber)){
				if(GUILDSIZE_MAX <= Packet.nCount) break;

				Packet.List[Packet.nCount] = pGuildMemeber->nCharacterDBID;
				++Packet.nCount;
			}
		}
		time_t CurTime;	time(&CurTime);
		for( int i=GUILDREWARDEFFECT_TYPE_EXTRAEXP;i<GUILDREWARDEFFECT_TYPE_CNT;i++)
		{
			if( m_Info.GuildRewardItem[i].nItemID > 0 && !m_Info.GuildRewardItem[i].bEternity )
			{
				if( m_Info.GuildRewardItem[i].m_tExpireDate < CurTime )
				{
					// If there are expired reward items, retrieve the info again.
					m_pDBCon->QueryGetGuildRewardItem( m_cDBThreadID, GetUID().nWorldID, 0, GetUID().nDBID );
					break;
				}
			}
		}
	}
	// Sync end
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	if(Packet.nCount > 0)
		SendGuildMemberLoginList(&Packet);

#if !defined( PRE_ADD_NODELETEGUILD )
	if(bIsEmpty)
	{
		if(m_bCheckEmpty)
			DisableAttach();	// Release guild resource
		else
			m_bCheckEmpty = true;
	}
#endif
}

void CDNGuildBase::SetGuildInfoAuth(int pGuildRoleAuth[GUILDROLE_TYPE_CNT]) const
{
	for(int iIndex = 0 ; GUILDROLE_TYPE_CNT > iIndex ; ++iIndex) {
		pGuildRoleAuth[iIndex] = m_Info.wGuildRoleAuth[iIndex];
	}
}
void CDNGuildBase::SetGuildInfoAuth(eGuildRoleType pGuildRoleType, int pGuildAuth)
{
	m_Info.wGuildRoleAuth[pGuildRoleType] = pGuildAuth;
}
void CDNGuildBase::SetGuildInfoAuth(eGuildRoleType pGuildRoleType, eGuildAuthType pGuildAuthType)
{
	m_Info.wGuildRoleAuth[pGuildRoleType] |= (static_cast<int>(1) << pGuildAuthType);
}
void CDNGuildBase::ResetGuildInfoAuth(eGuildRoleType pGuildRoleType, eGuildAuthType pGuildAuthType)
{
	m_Info.wGuildRoleAuth[pGuildRoleType] &= ~(static_cast<int>(1) << pGuildAuthType);
}
bool CDNGuildBase::CheckGuildInfoAuth(eGuildRoleType pGuildRoleType, eGuildAuthType pGuildAuthType) const
{
#ifdef PRE_ADD_BEGINNERGUILD
	if (m_Info.cGuildType == BeginnerGuild::Type::Beginner)
		return false;
#endif		//PRE_ADD_BEGINNERGUILD
	return m_Info.CheckAuth(pGuildRoleType, pGuildAuthType);
}

short CDNGuildBase::GetGuildMemberSize() const
{
	return(std::min<short>(m_Info.wGuildSize, GUILDSIZE_MAX));
}

// Updates the member list.
void CDNGuildBase::UpdateMember(TAGetGuildMember *pPacket)
{
	if(pPacket->nCount > 0)
	{
		ScopeLock <CSyncLock> AutoLock(GetLock());

		if( pPacket->bFirstPage )
		{
			m_MapTotalMember.clear();
			m_GuildMaster = NULL;
		}
		SetRecvAllMember( pPacket->bEndPage );		

		for(int i=0; i < MIN(pPacket->nCount, SENDGUILDMEMBER_MAX); i++)
		{
			AddMemberAsync(&(pPacket->MemberList[i]));
		}
	}
	else
		_DANGER_POINT();
}

void CDNGuildBase::UpdateGuildExp(const MAUpdateGuildExp* pPacket)
{

	switch (pPacket->cPointType)
	{
	case GUILDPOINTTYPE_STAGE:
		{
			SetInfo()->iDailyStagePoint = pPacket->nDailyPointValue;
			SendUpdateGuildExp(pPacket->cPointType, pPacket->nPointValue, pPacket->nTotalGuildExp, pPacket->biCharacterDBID);
			break;
		}
	case GUILDPOINTTYPE_MISSION:
		{
			SetInfo()->iDailyMissionPoint = pPacket->nDailyPointValue;
			SendUpdateGuildExp(pPacket->cPointType, pPacket->nPointValue, pPacket->nTotalGuildExp, pPacket->biCharacterDBID, pPacket->nMissionID);
			break;
		}
	case GUILDPOINTTYPE_WAR:
		{
			SendUpdateGuildExp(pPacket->cPointType, pPacket->nPointValue, pPacket->nTotalGuildExp);
			break;
		}
	}
	
	SetInfo()->iTotalGuildExp = pPacket->nTotalGuildExp;

	// Process level up
#if defined (_FINAL_BUILD)
	if (pPacket->nLevel > GetLevel())
#else
	if (pPacket->nLevel != GetLevel())
#endif
	{
		SetInfo()->wGuildLevel = pPacket->nLevel;		
		SendGuildLevelUp (pPacket->nLevel);
	}

	CheckGuildInfo();
}

// Add guild member.
bool CDNGuildBase::AddMember(const TGuildMember* pGuildMember, TCommunityLocation* pLocation)
{
	ScopeLock <CSyncLock> AutoLock(GetLock());

	return AddMemberAsync(pGuildMember, pLocation);
}

// Add guild member (async).
bool CDNGuildBase::AddMemberAsync(const TGuildMember* pGuildMember, TCommunityLocation* pLocation)
{
	std::pair<std::map<INT64, TGuildMember>::iterator, bool> pRetVal;

	pRetVal = m_MapTotalMember.insert(std::map<INT64, TGuildMember>::value_type(pGuildMember->nCharacterDBID, (*pGuildMember)));

	TGuildMember* pMember = static_cast<TGuildMember*>(&pRetVal.first->second);

	if(pGuildMember)
	{
		sWorldUserState aWorldUserState;

		bool bRetVal = g_pWorldUserState->GetUserState(pMember->nCharacterDBID, &aWorldUserState);

		if(bRetVal)
		{
			pMember->Location.cServerLocation	= aWorldUserState.nLocationState;
			pMember->Location.nChannelID			= aWorldUserState.nChannelID;
			pMember->Location.nMapIdx			= aWorldUserState.nMapIdx;

			if(pLocation)
			{
				pLocation->cServerLocation					= aWorldUserState.nLocationState;
				pLocation->nChannelID						= aWorldUserState.nChannelID;
				pLocation->nMapIdx							= aWorldUserState.nMapIdx;
			}
		}
		if( pMember->btGuildRole == GUILDROLE_TYPE_MASTER )
		{
			// Master settings
			SetGuildMaster( pMember );
		}
	}

	return true;
}

// Remove guild member from list
void CDNGuildBase::DelMember(INT64 nCharacterDBID)
{
	ScopeLock <CSyncLock> AutoLock(GetLock());

	std::map<INT64, TGuildMember>::const_iterator con_iter = m_MapTotalMember.find(nCharacterDBID);

#if defined(_DEBUG)
	if(con_iter == m_MapTotalMember.end())
		DN_ASSERT(0,	"Invalid!");
#endif	// _DEBUG

	TGuildMember *GuildMaster = GetGuildMaster();
	if( GuildMaster && GuildMaster->nCharacterDBID == nCharacterDBID)
		SetGuildMaster( NULL );
	m_MapTotalMember.erase(nCharacterDBID);
}

void CDNGuildBase::UpdateMemberLevel(INT64 nCharacterDBID, char cLevel)
{
	TGuildMember* pGuildMember = NULL;

	ScopeLock <CSyncLock> AutoLock(GetLock());

	pGuildMember = GetMemberInfoAsync(nCharacterDBID);

	if(pGuildMember)
		pGuildMember->cLevel = cLevel;
}

void CDNGuildBase::UpdateMemberName(INT64 nCharacterDBID, const WCHAR* wszName)
{
	TGuildMember* pGuildMember = NULL;

	ScopeLock <CSyncLock> AutoLock(GetLock());

	pGuildMember = GetMemberInfoAsync(nCharacterDBID);

	if(pGuildMember)
	{
		_wcscpy(pGuildMember->wszCharacterName, _countof(pGuildMember->wszCharacterName), wszName, (int)wcslen(wszName));
	}
}

INT64 CDNGuildBase::UpdateMemberCoin(INT64 nCharacterDBID, INT64 nMoveCoin, __time64_t tUseDate)
{
	TGuildMember* pGuildMember = NULL;

	ScopeLock <CSyncLock> AutoLock(GetLock());

	pGuildMember = GetMemberInfoAsync(nCharacterDBID);

	if(pGuildMember)
	{
		pGuildMember->WithdrawCoin += nMoveCoin;
		pGuildMember->LastWareUseDate = tUseDate;
		return pGuildMember->WithdrawCoin;
	}
	return 0;
}

void CDNGuildBase::ResetMemberWareVersion(INT64 nCharacterDBID)
{
	TGuildMember* pGuildMember = NULL;

	ScopeLock <CSyncLock> AutoLock(GetLock());

	pGuildMember = GetMemberInfoAsync(nCharacterDBID);

	if(pGuildMember)
		pGuildMember->WareVeriosnDate = 0;
}

bool CDNGuildBase::IsMemberAddable() const
{
	if(GetGuildMemberSize() > GetMemberCount())
		return true;

	return false;	
}

bool CDNGuildBase::IsMemberExist(INT64 nCharacterDBID, TGuildMember* pGuildMember) const
{
	ScopeLock <CSyncLock> AutoLock(GetLock());

	std::map<INT64, TGuildMember>::const_iterator con_iter = m_MapTotalMember.find(nCharacterDBID);
	if(con_iter == m_MapTotalMember.end())
	{
		if(pGuildMember)
			pGuildMember = NULL;

		return false;
	}

	if(pGuildMember)
		(*pGuildMember) = con_iter->second;

	return true;
}

int CDNGuildBase::GetMemberCount() const
{
	return(static_cast<int>(m_MapTotalMember.size()));
}

void CDNGuildBase::SetMemberList(TGuildMember *GuildMember)
{
	ScopeLock <CSyncLock> AutoLock(GetLock());

	std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.begin();

	int nCount = 0;
	for(; iter != m_MapTotalMember.end(); iter++)
	{
		if(GUILDSIZE_MAX <= nCount) break;

		GuildMember[nCount] = iter->second;
		nCount++;
	}
}

bool CDNGuildBase::AddPoint(char cPointType, int nPointValue, INT64 biCharacterDBID, int nMissionID)
{
#ifdef PRE_ADD_BEGINNERGUILD
	if (m_Info.cGuildType == BeginnerGuild::Type::Beginner)
		return true;
#endif		//PRE_ADD_BEGINNERGUILD
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	int nLimitPoint = 0;
	int nAddPoint = 0;
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)

	// Check guild max EXP
	int nMaxExp = g_pDataManager->GetMaxGuildExp();
	if (GetInfo()->iTotalGuildExp >= nMaxExp)
	{
		CheckGuildInfo();
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
		if( !IsAddGuildExp(cPointType, nPointValue, nAddPoint, nLimitPoint) ) return false;
		if( nAddPoint <= 0 || nLimitPoint <= 0) return true;

		m_pDBCon->QueryModGuildExp(m_cDBThreadID, GetUID().nWorldID, GetUID().nDBID, cPointType, nAddPoint, nLimitPoint, GetLevel(), biCharacterDBID, nMissionID, true);
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
		return true;
	}
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	if( !IsAddGuildExp(cPointType, nPointValue, nAddPoint, nLimitPoint) ) return false;
	if( nAddPoint <= 0 || nLimitPoint <= 0) return true;

	// Adjust guild max EXP
	if (GetInfo()->iTotalGuildExp+nAddPoint > nMaxExp)
		nAddPoint = nMaxExp - GetInfo()->iTotalGuildExp;

	// Check points
	if (nAddPoint <= 0 || nLimitPoint <= 0)
		return true;
#else	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	TGuildLevel* GuildLevel = g_pDataManager->GetGuildLevelData(GetLevel());
	if (!GuildLevel)
		return true;

	int nTempPoint = 0;
	int nAddPoint = 0;
	int nLimitPoint = 0;

	if (cPointType == GUILDPOINTTYPE_STAGE)
	{
		if (GetInfo()->iDailyStagePoint >= GuildLevel->nDailyGuildPointLimit)
			return false; // Daily stage acquisition limit

		nLimitPoint = GuildLevel->nDailyGuildPointLimit;
		nTempPoint = GetInfo()->iDailyStagePoint + nPointValue - nLimitPoint;

		if (nTempPoint > 0)
			nAddPoint = nLimitPoint - GetInfo()->iDailyStagePoint;
		else
			nAddPoint = nPointValue;
			
	}
	else if (cPointType == GUILDPOINTTYPE_MISSION)
	{
		if (GetInfo()->iDailyMissionPoint >= GuildLevel->nDailyMissionPointLimit)
			return false;	// Daily mission acquisition limit

		nLimitPoint = GuildLevel->nDailyMissionPointLimit;
		nTempPoint = GetInfo()->iDailyMissionPoint + nPointValue - nLimitPoint;

		if (nTempPoint > 0)
			nAddPoint = nLimitPoint - GetInfo()->iDailyMissionPoint;
		else
			nAddPoint = nPointValue;
	}
	else if (cPointType == GUILDPOINTTYPE_WAR)
	{
		nAddPoint = nPointValue;
		nLimitPoint = nPointValue;
	}
	else
		return true;

	// Adjust guild max EXP
	if (GetInfo()->iTotalGuildExp+nAddPoint > nMaxExp)
		nAddPoint = nMaxExp - GetInfo()->iTotalGuildExp;

	// Check points
	if (nAddPoint <= 0 || nLimitPoint <= 0)
		return true;
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
	// Process level
	int nLevel = GetLevel();
	std::vector<int> vLevels;
	vLevels.clear();

	while (true)
	{
		TGuildLevel* GuildNextLevel = g_pDataManager->GetGuildLevelData(nLevel+1);
		if (GuildNextLevel)
		{
			if (GetInfo()->iTotalGuildExp+nAddPoint >= GuildNextLevel->nReqGuildPoint)
				vLevels.push_back(++nLevel);
			else
				break;
		}
		else
			break;
	}

	if (vLevels.empty())
		m_pDBCon->QueryModGuildExp(m_cDBThreadID, GetUID().nWorldID, GetUID().nDBID, cPointType, nAddPoint, nLimitPoint, nLevel, biCharacterDBID, nMissionID);
	else
	{
		int nSize = (int)vLevels.size();
		for (int i=0; i<nSize; i++)
			m_pDBCon->QueryModGuildExp(m_cDBThreadID, GetUID().nWorldID, GetUID().nDBID, cPointType, (i == nSize-1 ? nAddPoint : 0), nLimitPoint, vLevels[i], biCharacterDBID, nMissionID);
	}


	return true;
}

void CDNGuildBase::GuildWarReset()
{
	m_wGuildWarScheduleID = 0;
	memset(m_bRewardResults, 0, sizeof(m_bRewardResults));
}
#if defined(_VILLAGESERVER)
bool CDNGuildBase::IsEnrollGuildWar()
{
	if(!g_pGuildWarManager)
		return false;

	if(m_Info.GuildView.wWarSchduleID != g_pGuildWarManager->GetScheduleID())
		return false;

	return true;
}
#elif defined(_GAMESERVER)
bool CDNGuildBase::IsEnrollGuildWar()
{
	if(!g_pMasterConnectionManager)
		return false;
	if( m_Info.GuildView.wWarSchduleID != g_pMasterConnectionManager->GetGuildWarScheduleID(m_Info.GuildView.GuildUID.nWorldID) )
		return false;	
	return true;
}
#endif

void CDNGuildBase::SetGuildWarRewardResults(bool* pRewardResults )
{
	for( char i=0; i<GUILDWAR_REWARD_GUILD_MAX; ++i)
	{
		m_bRewardResults[i] = pRewardResults[i];
	}
}

void CDNGuildBase::SetGuildWarRewardResultIndex(char cRewardType, bool bResult)
{
	if( cRewardType >= GUILDWAR_REWARD_GUILD_MAX || cRewardType < GUILDWAR_REWARD_GUILD_SKILL )
		return;
	m_bRewardResults[cRewardType] = bResult;
}

bool CDNGuildBase::GetGuildWarRewardEnable(char cRewardType)
{
	if( cRewardType >= GUILDWAR_REWARD_GUILD_MAX || cRewardType < GUILDWAR_REWARD_GUILD_SKILL )
		return true;
	return m_bRewardResults[cRewardType];
}

void CDNGuildBase::SetGuildRewardItem(TGuildRewardItem* GuildRewardItem)
{
	memcpy( m_Info.GuildRewardItem, GuildRewardItem, sizeof(m_Info.GuildRewardItem) );
	
	if( m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXGUILDWARE].nItemID > 0 )
	{
		if( m_Info.wGuildWareSize < m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXGUILDWARE].nEffectValue)
			m_Info.wGuildWareSize = m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXGUILDWARE].nEffectValue;
	}
	if( m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXMEMBER].nItemID > 0 )
	{
		if( m_Info.wGuildSize < m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXMEMBER].nEffectValue)
			m_Info.wGuildSize = m_Info.GuildRewardItem[GUILDREWARDEFFECT_TYPE_MAXMEMBER].nEffectValue;
	}
}
void CDNGuildBase::AddGuildRewardItem( int nItemID, __time64_t	tExpireDate, TGuildRewardItem &GuildRewardItem, CDNUserSession* pSession )
{
	TGuildRewardItemData* pGuildRewardItemData = g_pDataManager->GetGuildRewardItemData(nItemID);
	if(!pGuildRewardItemData)
		return;

	if( pGuildRewardItemData->nItemType >= GUILDREWARDEFFECT_TYPE_CNT )
		return;
	int nIdex = pGuildRewardItemData->nItemType;

	m_Info.GuildRewardItem[nIdex].nItemID = pGuildRewardItemData->nItemID;
	m_Info.GuildRewardItem[nIdex].nItemType = pGuildRewardItemData->nItemType;
	m_Info.GuildRewardItem[nIdex].nEffectValue = pGuildRewardItemData->nTypeParam1;
#if defined( PRE_ADD_GUILDREWARDITEM )
	m_Info.GuildRewardItem[nIdex].nEffectValue2 = pGuildRewardItemData->nTypeParam2;
#endif
	m_Info.GuildRewardItem[nIdex].bEternity = pGuildRewardItemData->bEternity;	
	m_Info.GuildRewardItem[nIdex].m_tExpireDate = tExpireDate;

	if( pGuildRewardItemData->nItemType == GUILDREWARDEFFECT_TYPE_MAXGUILDWARE )
	{
		// Expand guild storage		
		TGuildRewardItemData* PrevGuildRewardItemData = g_pDataManager->GetGuildRewardItemData(pGuildRewardItemData->nCheckID);
		short nGuildWareSize = GetWareSize();
		int nCashGuildWareSize = nGuildWareSize;
		if(PrevGuildRewardItemData)
		{
			nCashGuildWareSize = nGuildWareSize - PrevGuildRewardItemData->nTypeParam1;
		}		
		nGuildWareSize = nCashGuildWareSize + pGuildRewardItemData->nTypeParam1;
		if( pSession )
		{
			UpdateWareSize( pSession, (short)nGuildWareSize );
		}
	}
	else if( pGuildRewardItemData->nItemType == GUILDREWARDEFFECT_TYPE_MAXMEMBER )
	{
		// Expand max member count
		if( pSession )
		{
			if(m_pDBCon)
			{
				m_pDBCon->QueryChangeGuildSize( m_cDBThreadID, pSession->GetAccountDBID(), GetUID().nDBID, GetUID().nWorldID, pGuildRewardItemData->nTypeParam1 );
			}
		}
	}

	memcpy( &GuildRewardItem, &m_Info.GuildRewardItem[nIdex], sizeof(TGuildRewardItem) );
}
TGuildRewardItem* CDNGuildBase::GetGuildRewardItem()
{
	return m_Info.GuildRewardItem;
}

void CDNGuildBase::CheckGuildInfo()
{
	if (m_pDBCon && GetInfo()->iTotalGuildExp >= g_pDataManager->GetMaxGuildExp())
	{
		if (GetLevel() != g_pDataManager->GetMaxMaxGuildLevel())
			m_pDBCon->QueryModGuildExp(m_cDBThreadID, GetUID().nWorldID, GetUID().nDBID, GUILDPOINTTYPE_MAX, 0, 0, g_pDataManager->GetMaxMaxGuildLevel(), 0, 0);
	}
}
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
bool CDNGuildBase::IsAddGuildExp(char cPointType, int nPointValue, OUT int& nAddPoint, OUT int& nLimitPoint)
{
	TGuildLevel* GuildLevel = g_pDataManager->GetGuildLevelData(GetLevel());
	if (!GuildLevel)
		return true;

	int nTempPoint = 0;

	if (cPointType == GUILDPOINTTYPE_STAGE)
	{
		if (GetInfo()->iDailyStagePoint >= GuildLevel->nDailyGuildPointLimit)
			return false; // Daily stage acquisition limit

		nLimitPoint = GuildLevel->nDailyGuildPointLimit;
		nTempPoint = GetInfo()->iDailyStagePoint + nPointValue - nLimitPoint;

		if (nTempPoint > 0)
			nAddPoint = nLimitPoint - GetInfo()->iDailyStagePoint;
		else
			nAddPoint = nPointValue;

	}
	else if (cPointType == GUILDPOINTTYPE_MISSION)
	{
		if (GetInfo()->iDailyMissionPoint >= GuildLevel->nDailyMissionPointLimit)
			return false;	// Daily mission acquisition limit

		nLimitPoint = GuildLevel->nDailyMissionPointLimit;
		nTempPoint = GetInfo()->iDailyMissionPoint + nPointValue - nLimitPoint;

		if (nTempPoint > 0)
			nAddPoint = nLimitPoint - GetInfo()->iDailyMissionPoint;
		else
			nAddPoint = nPointValue;
	}
	else if (cPointType == GUILDPOINTTYPE_WAR)
	{
		nAddPoint = nPointValue;
		nLimitPoint = nPointValue;
	}

	return true;
}
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////						PRIVATE FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDNGuildBase::UpdateMemberState(TGuildMember* pGuildMember)
{
	sWorldUserState WorldUserState;
	bool bRetVal = g_pWorldUserState->GetUserState(pGuildMember->nCharacterDBID, &WorldUserState);

	// When user info is missing in world, reset location value and pass.
	if(!bRetVal)
	{
		pGuildMember->Location.Reset();
		return false;
	} 

	bool bResult = false;

	if(_LOCATION_NONE == WorldUserState.nLocationState)
	{
		//Exclude offline check
		return false;
	}

	// Update guild member's current location.
	pGuildMember->Location.cServerLocation	= WorldUserState.nLocationState;
	pGuildMember->Location.nChannelID			= WorldUserState.nChannelID;
	pGuildMember->Location.nMapIdx			= WorldUserState.nMapIdx;

	return bResult;
}

bool CDNGuildBase::CheckGuildResource(TGuildMember* pGuildMemeber)
{
#if defined(_DEBUG)
	// Prevent releasing guild resources during debugging!
	if(::IsDebuggerPresent() && m_bSkipDisableOnDebugging)
		return false;
#endif	// _DEBUG

	return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns the member list
void CDNGuildBase::GetMemberList(std::vector <std::pair<UINT, INT64>, boost::pool_allocator<std::pair<UINT, INT64>>>& pList) const
{
	DN_ASSERT(IsOpen(),		"Not Opened!");
	DN_ASSERT(IsAttach(),	"Not Attached!");
	DN_ASSERT(!IsLock(),	"Already Locked!");


	// Return if no member
	if(m_MapTotalMember.empty())
		return;

	ScopeLock <CSyncLock> AutoLock(GetLock());

	std::map<INT64, TGuildMember>::const_iterator iter = m_MapTotalMember.begin();

	for(; iter != m_MapTotalMember.end(); iter++)
	{
		const TGuildMember* pGuildMember = &(iter->second);

		DN_ASSERT(NULL != pGuildMember,					"Invalid!");
		DN_ASSERT(0 != pGuildMember->nAccountDBID,	"Invalid!");

		pList.push_back(std::make_pair(pGuildMember->nAccountDBID, pGuildMember->nCharacterDBID));

	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Returns member information.
TGuildMember* CDNGuildBase::GetMemberInfo(INT64 nCharacterDBID)
{
	DN_ASSERT(IsOpen(),				"Not Opened!");
	DN_ASSERT(IsAttach(),			"Not Attached!");
	DN_ASSERT(0 != nCharacterDBID,	"Invalid!");

	ScopeLock <CSyncLock> AutoLock(GetLock());

	return GetMemberInfoAsync(nCharacterDBID);
}

TGuildMember* CDNGuildBase::GetMemberInfoAsync(INT64 nCharacterDBID)
{
	DN_ASSERT(IsOpen(),				"Not Opened!");
	DN_ASSERT(IsAttach(),			"Not Attached!");
	DN_ASSERT(0 != nCharacterDBID,	"Invalid!");

	std::map<INT64, TGuildMember>::iterator iter = m_MapTotalMember.find(nCharacterDBID);
	if(iter != m_MapTotalMember.end())
		return(&iter->second);

	return NULL;
}