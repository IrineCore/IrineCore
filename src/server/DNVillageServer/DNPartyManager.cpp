
#include "stdafx.h"
#include "DNPartyManager.h"
#include "DNUserSession.h"
#include "DNUserSendManager.h"
#include "DNMasterConnection.h"
#include "Util.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#if defined( PRE_PARTY_DB )
#include "DNPartySortObject.hpp"
#endif // #if defined( PRE_PARTY_DB )

CDNPartyManager* g_pPartyManager;
extern TVillageConfig g_Config;

CDNPartyManager::CDNPartyManager(void): m_nProcessCount(0)
{
#if defined( PRE_PARTY_DB )
	m_pDBCon = NULL;
	m_dwGetListPartyTick = 0;
#else
	ScopeLock<CSyncLock> Lock(m_Sync);
#endif // #if defined( PRE_PARTY_DB )

	m_pMapPartyList.clear();
	m_nLastCheckTick = 0;
#if defined( PRE_PARTY_DB )
	m_bDeletePartyForServer = false;
#else
	m_pVecPartyList.clear();
	m_pMultiMapWaitParty.clear();
#endif // #if defined( PRE_PARTY_DB )
#if defined( PRE_WORLDCOMBINE_PARTY )
	m_nCreateWorldPartyTick = 0;
	m_WorldReqCombineParty.clear();
#endif
}

CDNPartyManager::~CDNPartyManager(void)
{
#if defined( PRE_PARTY_DB )
	if( m_pMapPartyList.empty() ) 
		return;
#else
	if (m_pVecPartyList.empty() && m_pMapPartyList.empty()) 
		return;
	ScopeLock<CSyncLock> Lock(m_Sync);
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
	for( TMapParty::iterator itor=m_pMapPartyList.begin() ; itor!=m_pMapPartyList.end() ; ++itor )
		delete (*itor).second;
	m_pMapPartyList.clear();
#else
	if (!m_pVecPartyList.empty())
	{
		std::vector <CDNParty*>::iterator ii;
		for (ii = m_pVecPartyList.begin(); ii != m_pVecPartyList.end(); ii++)
			SAFE_DELETE((*ii));
	}

	m_pVecPartyList.clear();
	m_pMapPartyList.clear();
	m_pMultiMapWaitParty.clear();
#endif // #if defined( PRE_PARTY_DB )
}

void CDNPartyManager::DoUpdate(DWORD CurTick)
{
#if defined( PRE_PARTY_DB )
	if( m_nLastCheckTick > 0 && CurTick < (DWORD)m_nLastCheckTick) 
		return;
	m_nLastCheckTick = CurTick + 50;

	for( TMapParty::iterator itor=m_pMapPartyList.begin() ; itor!=m_pMapPartyList.end() ;  )
	{
		CDNParty* pParty = (*itor).second;
		if( pParty )
		{
			if( !(pParty->DoUpdate( CurTick )) )
			{				
				itor++;
				DestroyParty(pParty->GetPartyID());
			}
			else
				itor++;
		}
	}	
#else
	if (m_pVecPartyList.empty()) return;
	if (m_nLastCheckTick > 0 && CurTick < (DWORD)m_nLastCheckTick) return;
	m_nLastCheckTick = CurTick + 50;

	int nCount;
	CDNParty *pParty = NULL;

	ScopeLock<CSyncLock> Lock(m_Sync);

	nCount = (int)m_pVecPartyList.size();
	if (nCount > PROCESSCOUNT) nCount = PROCESSCOUNT;

	for (int i = 0; i < nCount; i++){
		if (m_nProcessCount >= (int)m_pVecPartyList.size())
			m_nProcessCount = 0;

		pParty = m_pVecPartyList[m_nProcessCount];
		if (pParty) 
			pParty->DoUpdate(CurTick);

		m_nProcessCount++;
	}
#endif // #if defined( PRE_PARTY_DB )
}

#if defined( PRE_PARTY_DB )

CDNDBConnection* CDNPartyManager::GetDBConnection()
{
	if( m_pDBCon == NULL )
	{
		BYTE cThreadID;
		m_pDBCon = g_pDBConnectionManager->GetDBConnection( g_Config.nWorldSetID, cThreadID );
	}

	return m_pDBCon;
}

void CDNPartyManager::InternalDoUpdate( DWORD dwCurTick )
{
	if( m_bDeletePartyForServer == false )
	{
		QueryDelPartyForServer( g_Config.nManagedID );
		m_bDeletePartyForServer = true;
	}

	const DWORD dwInterval = 3000;	
	
	// DelPartyForServer
	if( m_DeletePartyForServerList.empty() == false )
	{
		for( TDELETEPARTYFORSERVERLIST::iterator itor=m_DeletePartyForServerList.begin() ; itor!=m_DeletePartyForServerList.end() ; ++itor )	
		{
			if( (*itor).second+dwInterval < dwCurTick )	
				QueryDelPartyForServer( (*itor).first, dwCurTick );
		}
		return;
	}
	
	// OutParty
	BYTE cThreadID;
	CDNDBConnection* pLoopDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );	
	for( TOUTPARTYLIST::iterator itor=m_OutPartyList.begin() ; itor!=m_OutPartyList.end() ; )
	{
		bool bRet = true;
		if( (*itor).second+dwInterval < dwCurTick )					
			bRet = QueryOutParty( GetTupleValue((*itor).first,0), GetTupleValue((*itor).first,1), GetTupleValue((*itor).first,2), GetTupleValue((*itor).first,3), GetTupleValue((*itor).first,4), dwCurTick, pLoopDBCon );

		if( bRet == true )
			++itor;
		else
			m_OutPartyList.erase( itor++ );
	}

	// DelParty
	pLoopDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
	for( TDELETEPARTYLIST::iterator itor=m_DeletePartyList.begin() ; itor!=m_DeletePartyList.end() ; )
	{
		bool bRet = true;
		if( (*itor).second+dwInterval < dwCurTick )					
			bRet = QueryDelParty( (*itor).first, dwCurTick, pLoopDBCon );

		if( bRet == true )
			++itor;
		else
			m_DeletePartyList.erase( itor++ );
	}

#if defined( PRE_WORLDCOMBINE_PARTY )
	if( m_nCreateWorldPartyTick + 30 * 1000 < dwCurTick )
	{
		const sChannelInfo* pChannelInfo = g_pMasterConnection->GetChannelInfoFromAttr( GlobalEnum::CHANNEL_ATT_PVPLOBBY );

		if( g_Config.nCombinePartyWorld > 0 && g_Config.nWorldSetID == g_Config.nCombinePartyWorld && pChannelInfo &&
			pChannelInfo->cVillageID == g_Config.nVillageID )
		{				
			QueryGetListWorldParty();
			m_nCreateWorldPartyTick = dwCurTick;
		}	
	}	
#endif

	// PartyUpdate
	DoUpdate( dwCurTick );

	// GetListParty
	if( m_dwGetListPartyTick+Party::Interval::GetListPartyTick < dwCurTick )
	{
		QueryGetListParty( dwCurTick );
	}
}

#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )

CDNParty* CDNPartyManager::CreateParty( CDNUserSession* pUser, TPARTYID PartyID, const DBCreateParty* pData )
{
	if( PartyID == 0 )
		return NULL;

	CDNParty* pParty = new CDNParty(0);
	if( pParty == NULL ) 
		return NULL;

	//Set Values
	pParty->Create( PartyID, pData );

	bool bAdd = true;
	UINT uiRandomSeed = pParty->GetRandomSeed();
	errno_t err = rand_s(&uiRandomSeed);
	if (err == 0)
	{
		if (m_pMapPartyList.find(PartyID) != m_pMapPartyList.end())
			bAdd = false;

		if (bAdd)
		{
#if defined( PRE_PARTY_DB )
#else
			m_pVecPartyList.push_back(pParty);
#endif // #if defined( PRE_PARTY_DB )
			m_pMapPartyList[pParty->GetPartyID()] = pParty;

#if defined( PRE_PARTY_DB )
#else
			for (TMultiMapParty::iterator ii = m_pMultiMapWaitParty.begin(); ii != m_pMultiMapWaitParty.end(); )
			{
				if ((*ii).second->GetPartyID() == pParty->GetPartyID() )
				{
					ii = m_pMultiMapWaitParty.erase(ii);
				}
				else
					ii++;
			}

			m_pMultiMapWaitParty.insert(std::make_pair(pParty->GetPartyMapIndex(), pParty));
#endif // #if defined( PRE_PARTY_DB )

			// Party start log (Yoontaek requested not to leave party start/end logs on party renewal)
			//if( pUser && pUser->GetDBConnection() )
			//{
			//	pUser->GetDBConnection()->QueryAddPartyStartLog( pUser, PartyID );
			//}
			return pParty;
		}
	}

	SAFE_DELETE(pParty);
	_DANGER_POINT();

	return NULL;
}

void CDNPartyManager::PushParty( const Party::Data& Data )
{
	DBCreateParty DBCreateParty;
	memset( &DBCreateParty, 0, sizeof(DBCreateParty) );

	DBCreateParty.PartyData = Data;

	CreateParty( NULL, Data.PartyID, &DBCreateParty );
}

CDNParty * CDNPartyManager::PushParty(MAVIPushParty * pPacket)
{
	int iPermitLevel = g_pDataManager->GetMapPermitLevel( g_pMasterConnection->GetChannelMapIndex( pPacket->PartyData.iLocationID ) );
	pPacket->PartyData.cMinLevel = (pPacket->PartyData.cMinLevel < iPermitLevel) ? iPermitLevel : pPacket->PartyData.cMinLevel;

	Party::AddPartyAndMemberVillage PartyData;
	memset(&PartyData, 0, sizeof(Party::AddPartyAndMemberVillage));
	
	PartyData.PartyData = pPacket->PartyData;
	memcpy( PartyData.nKickedMemberList, pPacket->nKickedMemberList, sizeof(PartyData.nKickedMemberList) );
	PartyData.PartyData.iRoomID = 0;

	for( int i=0;i<pPacket->PartyData.iCurMemberCount;i++ )
	{
		if( pPacket->MemberInfo[i].nAccountDBID > 0 )
		{
			PartyData.MemberData[i].nAccountDBID = pPacket->MemberInfo[i].nAccountDBID;
			PartyData.MemberData[i].biCharacterDBID = pPacket->MemberInfo[i].biCharacterDBID;
			PartyData.MemberData[i].cMemberIndex = pPacket->MemberInfo[i].cMemberIndex;
#if defined( PRE_ADD_NEWCOMEBACK )
			PartyData.MemberData[i].bCheckComebackAppellation = pPacket->MemberInfo[i].bCheckComebackAppellation;			
#endif
			PartyData.MemberData[i].cVoiceAvailable = pPacket->MemberInfo[i].VoiceInfo.cVoiceAvailable;
			memcpy( PartyData.MemberData[i].nMutedList, pPacket->MemberInfo[i].VoiceInfo.nMutedList, sizeof(PartyData.MemberData[i].nMutedList) );			
		}
		else
			break;
	}

	if( GetDBConnection() )
	{
		GetDBConnection()->QueryAddPartyAndMemberVillage( g_Config.nWorldSetID, pPacket->nLeaderAccountDBID, g_Config.nManagedID, Party::LocationType::Village, &PartyData );
	}	
	return NULL;
}

CDNParty * CDNPartyManager::PushParty(TAAddPartyAndMemberVillage * pPacket)
{
	if (pPacket == NULL) return NULL;

	CDNParty * pParty = new CDNParty(timeGetTime());
	if (pParty == NULL) return NULL;

	pParty->PushParty( pPacket );

	for(int i=0;i<pPacket->Data.PartyData.iCurMemberCount;i++)
		m_AddPartyMemberList.insert( std::make_pair(pPacket->MemberInfo[i].nAccountDBID, pPacket->Data.PartyData.PartyID ) );

	if (m_pMapPartyList.find(pParty->GetPartyID()) == m_pMapPartyList.end())
	{		
		m_pMapPartyList[pParty->GetPartyID()] = pParty;
	}
	else
	{
		_DANGER_POINT();
		SAFE_DELETE(pParty);
		return NULL;
	}

#ifdef _USE_VOICECHAT
	if (pParty->CreateVoiceChannel(pPacket->nVoiceChannelID))
		pParty->SetInitVoiceInfo(pPacket->MemberInfo, pPacket->Data.PartyData.iCurMemberCount);
#endif

	return pParty;
}

#else

CDNParty * CDNPartyManager::CreateParty(CDNUserSession * pUser, TPARTYID PartyID, const WCHAR * pPartyName, const WCHAR * pPassWord, int nMemberMax, TPARTYITEMLOOTRULE ItemLootRule, TITEMRANK ItemRank, int nLvLimitMin, int nLvLimitMax, int nTargetMapIdx, TDUNGEONDIFFICULTY Difficulty, BYTE cIsPartyJobDice)
{
	if (PartyID == 0) return NULL;

	CDNParty * pParty = new CDNParty(0);
	if (!pParty) return NULL;

	//Set Values
	pParty->Create(pUser, PartyID, pPartyName, pPassWord, nMemberMax, ItemLootRule, ItemRank, nLvLimitMin, nLvLimitMax, nTargetMapIdx, Difficulty, cIsPartyJobDice );

	bool bAdd = true;
	UINT uiRandomSeed = pParty->GetRandomSeed();
	errno_t err = rand_s(&uiRandomSeed);
	if (err == 0)
	{
		ScopeLock<CSyncLock> Lock(m_Sync);

		if (m_pMapPartyList.find(PartyID) != m_pMapPartyList.end())
			bAdd = false;

		if (bAdd)
		{
			m_pVecPartyList.push_back(pParty);
			m_pMapPartyList[pParty->GetPartyID()] = pParty;

			for (TMultiMapParty::iterator ii = m_pMultiMapWaitParty.begin(); ii != m_pMultiMapWaitParty.end(); )
			{
				if ((*ii).second->GetPartyID() == pParty->GetPartyID())
				{
					ii = m_pMultiMapWaitParty.erase(ii);
				}
				else
					ii++;
			}

			m_pMultiMapWaitParty.insert(std::make_pair(pParty->GetPartyMapIndex(), pParty));

			// Party start log
			if( pUser->GetDBConnection() )
			{
				pUser->GetDBConnection()->QueryAddPartyStartLog( pUser, PartyID );
			}
			return pParty;
		}
	}
	
	SAFE_DELETE(pParty);
	_DANGER_POINT();

	return NULL;
}

CDNParty * CDNPartyManager::PushParty(MAVIPushParty * pPacket)
{
	if (pPacket == NULL) return NULL;

	CDNParty * pParty = new CDNParty(timeGetTime());
	if (pParty == NULL) return NULL;

	pParty->PushParty( pPacket );

	ScopeLock<CSyncLock> Lock(m_Sync);

	if (m_pMapPartyList.find(pParty->GetPartyID()) == m_pMapPartyList.end())
	{
		m_pVecPartyList.push_back(pParty);
		m_pMapPartyList[pParty->GetPartyID()] = pParty;
	}
	else
	{
		_DANGER_POINT();
		SAFE_DELETE(pParty);
		return NULL;
	}

#ifdef _USE_VOICECHAT
	if (pParty->CreateVoiceChannel(pPacket->nVoiceChannelID))
		pParty->SetInitVoiceInfo(pPacket->MemberInfo, pPacket->cMemberCount);
#endif

	return pParty;
}

#endif // #if defined( PRE_PARTY_DB )

bool CDNPartyManager::DestroyParty(TPARTYID PartyID, bool bLog/*=true*/)
{
#if defined( PRE_PARTY_DB )
#else
	ScopeLock<CSyncLock> Lock(m_Sync);
#endif // #if defined( PRE_PARTY_DB )

	CDNParty * pParty = NULL;
	std::map <TPARTYID, CDNParty*>::iterator ii = m_pMapPartyList.find(PartyID);
	if (ii != m_pMapPartyList.end())
	{
		//Basic conditions must be checked at the upper level beforehand
		pParty = (*ii).second;
		pParty->BanAllPartyMember(pParty->GetLeaderAccountDBID());	//If anyone remains, notify them that all were kicked

#if defined( PRE_PARTY_DB )
#else
		std::vector <CDNParty*>::iterator ih;
		for (ih = m_pVecPartyList.begin(); ih != m_pVecPartyList.end(); ih++)
		{
			if ((*ih)->GetPartyID() == PartyID)
			{
				m_pVecPartyList.erase(ih);
				break;
			}
		}
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
#else
		TMultiMapParty::iterator ij;
		for (ij = m_pMultiMapWaitParty.begin(); ij != m_pMultiMapWaitParty.end(); )
		{
			if ((*ij).second->GetPartyID() == PartyID)
			{
				ij = m_pMultiMapWaitParty.erase(ij);
			}
			else
				ij++;
		}
#endif // #if defined( PRE_PARTY_DB )

#ifdef _USE_VOICECHAT
		pParty->DestroyVoiceChannel();
#endif

		m_pMapPartyList.erase(ii);
		SAFE_DELETE(pParty);

#if !defined(PRE_PARTY_DB)
		if( bLog )
		{
			// Party end log
			BYTE cThreadID;
			CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
			if( pDBCon )
			{
				pDBCon->QueryAddPartyEndLog( cThreadID, g_Config.nWorldSetID, 0, PartyID );
			}
		}
#endif // #if !defined(PRE_PARTY_DB)

#if defined( PRE_PARTY_DB )
		QueryDelParty( PartyID );
#endif // #if defined( PRE_PARTY_DB )
		return true;
	}
	_DANGER_POINT();
	return false;
}

CDNParty * CDNPartyManager::GetParty(TPARTYID PartyID)
{
	if( PartyID <= 0 || m_pMapPartyList.empty()) 
		return NULL;

#if defined( PRE_PARTY_DB )
#else
	ScopeLock<CSyncLock> Lock(m_Sync);
#endif // #if defined( PRE_PARTY_DB )
	std::map <TPARTYID, CDNParty*>::iterator ii = m_pMapPartyList.find(PartyID);
	if (ii != m_pMapPartyList.end())
		return (*ii).second;
	return NULL;
}

#if defined( PRE_PARTY_DB )
#else
void CDNPartyManager::PushWaitPartyList(CDNParty *pParty)
{
	if (!pParty) return;
	if (pParty->HasEmptyPlace() == false)	return;
	if (pParty->GetMemberCount() <= 0) return;

	ScopeLock<CSyncLock> Lock(m_Sync);

	bool bChecker = false;
	for (TMultiMapParty::iterator ii = m_pMultiMapWaitParty.begin(); ii != m_pMultiMapWaitParty.end(); ii++)
	{
		if ((*ii).second->GetPartyID() == pParty->GetPartyID() || (*ii).second == pParty)
		{
			bChecker = true;
			break;
		}
	}

	if (bChecker == false)
		m_pMultiMapWaitParty.insert(std::make_pair(pParty->GetPartyMapIndex(), pParty));
}

bool CDNPartyManager::EraseWaitPartyList(CDNParty *pParty)
{
	if (!pParty) return false;
	if (m_pMultiMapWaitParty.empty()) return false;

	ScopeLock<CSyncLock> Lock(m_Sync);

	bool bFlag = false;
	for (TMultiMapParty::iterator ii = m_pMultiMapWaitParty.begin(); ii != m_pMultiMapWaitParty.end(); )
	{
		if ((*ii).second->GetPartyID() == pParty->GetPartyID() || (*ii).second == pParty)
		{
			ii = m_pMultiMapWaitParty.erase(ii);
			bFlag = true;
		}
		else
			ii++;
	}
	return bFlag;
}
#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_PARTY_DB )
void CDNPartyManager::GetPartyListInfo(int nMapIdx, int nChannelID, int nChannelMeritID, int nOffSetCount, const CSPartyListInfo* pPacket, std::list<CDNParty*>& PartyList )
{

	UINT uiTick = timeGetTime();

	std::vector <std::pair<int, TPARTYID>> vDelList;
	{
		for( TMapParty::iterator itor=m_pMapPartyList.begin() ; itor!=m_pMapPartyList.end() ; ++itor )
		{
			CDNParty* pParty = (*itor).second;
			
			// Do not show full parties in the list.
			if( pParty->HasEmptyPlace() == false )
				continue;
			// Cross-server party - check full list
			if( pParty->GetServerID() != g_Config.nManagedID )
			{
				if( pParty->GetCurMemberCount() >= pParty->GetMemberMax() )
					continue;
			}
			else
			{
				// Servers I manage may appear empty briefly before connecting, so skip if 0.
				if( pParty->GetMemberCount() == 0 )
					continue;
			}
			// Do not show parties that are in the game server and do not allow world zone(?).
			if( pParty->bIsInGame() )
			{
				if( !(pParty->GetBitFlag()&Party::BitFlag::WorldmapAllow) )
					continue;
			}
			// Same village option
			if( pPacket->bOnlySameVillage == true )
			{
#if defined( PRE_WORLDCOMBINE_PARTY )
				if( !(pParty->bIsInVillage() && pParty->GetPartyMapIndex() == nMapIdx) && !pParty->bIsWorldCombieParty() )
					continue;
#else
				if( !(pParty->bIsInVillage() && pParty->GetPartyMapIndex() == nMapIdx) )
					continue;
#endif
			}
			
			Party::CalcSortPoint SortObject(pPacket,nMapIdx);
			SortObject.CalcPoint( pParty );

			PartyList.push_back( pParty );
		}
	}

	PartyList.sort( Party::SortObject() );

	UINT uiGap = timeGetTime()-uiTick;
	if( uiGap >= 500 )
	{
		g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] CDNPartyManager::GetPartyListInfo Delay=%u PartyCount=%u\n", g_Config.nManagedID, uiGap, (UINT)m_pMapPartyList.size());
	}
		
}
#else
int CDNPartyManager::GetPartyListInfo(int nMapIdx, int nChannelID, int nChannelMeritID, int nOffSetCount, int nPageNum, int * nSortMapIdx, TDUNGEONDIFFICULTY Difficulty, int &nRet, std::list<SPartyListInfo> * pPartyList)
{
	if (nSortMapIdx == NULL) return 0;
	if (m_pMultiMapWaitParty.empty())	return 0;
	int nWholeCnt, nCount, nFirst, nLast;
	nCount = 0;

	std::vector <std::pair<int, TPARTYID>> vDelList;
	{
		ScopeLock<CSyncLock> Lock(m_Sync);	

		nWholeCnt = (int)m_pMultiMapWaitParty.count(nMapIdx);
		if (nWholeCnt <= 0) return 0;

		int nLastPageNum = nWholeCnt/((nOffSetCount <= 0) ? PARTYLISTOFFSET : nOffSetCount);
		nPageNum = nLastPageNum < nPageNum ? nLastPageNum : nPageNum;

		nFirst = nPageNum * nOffSetCount;
		nLast = nFirst + nOffSetCount - 1;

		if (nSortMapIdx[0] > 0 || Difficulty > 0)
			nRet = ERROR_PARTY_CONDITION_MATCHED_LIST_NOT_FOUND;

		SPartyListInfo sPartyInfo;
		std::multimap <int, std::pair<CDNParty*, const sChannelInfo *>> SortTemp;
		for (TMultiMapParty::iterator ii = m_pMultiMapWaitParty.lower_bound(nMapIdx); ii != m_pMultiMapWaitParty.upper_bound(nMapIdx); ii++)
		{
			int nKeyCode = -1;
			if (nSortMapIdx[0] > 0 || Difficulty > 0)
			{
				bool bSortMapFlag = false;
				bool bIsPrivate = false;
				for (int j = 0; j < PARTYSORTCONDITIONMAX; j++)
				{
					if (nSortMapIdx[j] > 0 && (*ii).second->GetTargetMapIndex() == nSortMapIdx[j])
					{
						bSortMapFlag = true;
						break;
					}
				}

				bIsPrivate = (*ii).second->bIsPasswordParty();

				if ((bSortMapFlag) && (bIsPrivate == false) && (Difficulty > 0 && (*ii).second->GetDifficulty() == Difficulty))
				{
					nRet = ERROR_NONE;
					nKeyCode = 0;
				}
				else if ((bSortMapFlag) && (bIsPrivate == false))
				{
					nRet = ERROR_NONE;
					nKeyCode = 1;
				}
				else if ((Difficulty > 0 && (*ii).second->GetDifficulty() == Difficulty) && (bIsPrivate == false))
				{
					nRet = ERROR_NONE;
					nKeyCode = 2;
				}
				else
				{
					nKeyCode = 3;
				}
			}
			else
			{
				bool bIsPrivate = (*ii).second->bIsPasswordParty();
				if (bIsPrivate == false)
				{
					nRet = ERROR_NONE;
					nKeyCode = 0;
				}
				else
				{
					nRet = ERROR_NONE;
					nKeyCode = 1;
				}
			}

			const sChannelInfo * pInfo = g_pMasterConnection->GetChannelInfo((*ii).second->GetChannelID());
			if (pInfo == NULL || pInfo->bVisibility == false || pInfo->nMapIdx != nMapIdx) continue;

			SortTemp.insert(std::make_pair(nKeyCode, std::make_pair((*ii).second, pInfo)));
		}

		pPartyList->clear();
		std::multimap <int, std::pair<CDNParty*, const sChannelInfo *>>::iterator ih;
		for (ih = SortTemp.begin(); ih != SortTemp.end(); ih++)
		{
			if (nCount >= nFirst && nLast >= nCount)
			{
				memset(&sPartyInfo, 0, sizeof(sPartyInfo));
				int nPartyRet = ((*ih).second).first->GetPartyInfo(&sPartyInfo, ((*ih).second).second);
				if (nPartyRet != ERROR_NONE)
				{
					vDelList.push_back(std::make_pair(nPartyRet, ((*ih).second).first->GetPartyID()));
					continue;
				}

				pPartyList->push_back(sPartyInfo);

				if (pPartyList->size() >= PARTYLISTOFFSET)
				{
					AdjustParty(vDelList);
					return nWholeCnt;
				}
			}
			nCount++;
		}
	}

	AdjustParty(vDelList);
	return nWholeCnt;
}
#endif // #if defined( PRE_PARTY_DB )


void CDNPartyManager::AdjustParty(std::vector <std::pair<int, TPARTYID>> &vList)
{
	if (!vList.empty())
	{
		CDNParty * pEraseParty = NULL;
		for (int h = 0; h < (int)vList.size(); h++)
		{
#if defined( PRE_PARTY_DB )
			if (vList[h].first == -2)
#else
			if (vList[h].first == -1)
			{
				pEraseParty = GetParty(vList[h].second);
				if (pEraseParty)
					EraseWaitPartyList(pEraseParty);
			}
			else if (vList[h].first == -2)
#endif // #if defined( PRE_PARTY_DB )
			{
				DestroyParty(vList[h].second);
			}
		}
	}
}

void CDNPartyManager::SendPartyMemberPart(TPARTYID PartyID, CDNUserSession *pSession)
{
	CDNParty *pParty = GetParty(PartyID);
	if (!pParty) return;

	pParty->SendPartyMemberPart(pSession);
}

void CDNPartyManager::SendPartyUseRadio(TPARTYID PartyID, UINT nSessionID, USHORT nRadioID)
{
	CDNParty * pParty = GetParty(PartyID);
	if (!pParty) return;

	pParty->SendPartyUseRadio(nSessionID, nRadioID);
}

void CDNPartyManager::SendPartyReadyRequest(TPARTYID PartyID, UINT nAccountDBID)
{
	CDNParty * pParty = GetParty(PartyID);
	if (!pParty) return;

	if (pParty->GetLeaderAccountDBID() != nAccountDBID)
	{
		_DANGER_POINT();
		return;
	}

	pParty->SendPartyReadyRequest();
}

#if defined( PRE_PARTY_DB )
int CDNPartyManager::DelPartyMember( CDNParty* pParty, CDNUserSession* pSession, const TAOutParty* pPacket )
#else
int CDNPartyManager::DelPartyMember( CDNParty* pParty, CDNUserSession* pSession )
#endif // #if defined( PRE_PARTY_DB )
{
	int nMemberIdx = pSession->m_nPartyMemberIndex;
	char cKickKind = -1;
#if defined( PRE_PARTY_DB )
	switch( pPacket->Type )
	{
		case Party::QueryOutPartyType::Kick:
		{
			cKickKind = 0;
			break;
		}
		case Party::QueryOutPartyType::EternityKick:
		{
			cKickKind = 1;
			break;
		}
	}
#endif // #if defined( PRE_PARTY_DB )
	if( pParty->DelPartyMember( pSession, cKickKind ) == false )
		return ERROR_PARTY_LEAVEFAIL;

#if defined( PRE_PARTY_DB)
	if(pPacket->bIsDelParty)
	{
		if( pParty->GetMemberCount() != 0 )
		{			
			g_Log.Log(LogType::_PARTYERROR, L"[Party] DelPartyError1 - PartyDBID:%I64d MemberCount:%d MemberOutType:%d\r\n", pPacket->PartyID, pParty->GetMemberCount(), pPacket->Type);			
			DestroyParty( pParty->GetPartyID(), true );
		}
		else
		{
			DestroyParty( pParty->GetPartyID(), ((pSession->m_eUserState != STATE_MOVETOVILLAGE) && (pSession->m_eUserState != STATE_MOVETOGAME)) );
		}
	}
	else
	{
		if( pParty->GetMemberCount() == 0 )
		{			
			DestroyParty( pParty->GetPartyID(), true );
		}
		else if( pParty->bIsMove() == false )
		{
			if( cKickKind != -1 )
				pParty->SendKickPartyMember( pSession->GetSessionID() );
			pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
		}		
	}
#else
	if( pParty->GetMemberCount() == 0 )
	{
		DestroyParty( pParty->GetPartyID(), ((pSession->m_eUserState != STATE_MOVETOVILLAGE) && (pSession->m_eUserState != STATE_MOVETOGAME)) );
	}
	else
	{
		if( pParty->bIsMove() == false )
		{
			pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
			PushWaitPartyList( pParty );
		}
	}
#endif

	return ERROR_NONE;
}

#if defined( PRE_PARTY_DB )

TPARTYID CDNPartyManager::GetPartyID( UINT nAccountDBID )
{
	TPARTYID PartyID = 0;
	TADDPARTYMEMBERLIST::iterator itor = m_AddPartyMemberList.find( nAccountDBID );
	if( itor != m_AddPartyMemberList.end() )
	{
		PartyID = itor->second;
		m_AddPartyMemberList.erase( itor );
	}
	return PartyID;
}

int CDNPartyManager::DelPartyMember( CDNParty* pParty, const TAOutParty* pA )
{	
	if( pParty->DelPartyMember( pA, -1 ) == false )
		return ERROR_PARTY_LEAVEFAIL;

	if(pA->bIsDelParty)
	{
		if( pParty->GetMemberCount() != 0 )
		{			
			g_Log.Log(LogType::_PARTYERROR, L"[Party] DelPartyError3 - PartyDBID:%I64d MemberCount:%d MemberOutType:%d\r\n", pA->PartyID, pParty->GetMemberCount(), pA->Type);			
			DestroyParty( pParty->GetPartyID(), true );
		}
		else
		{
			DestroyParty( pParty->GetPartyID(), true );
		}
	}
	else
	{
		if( pParty->GetMemberCount() == 0 )
		{			
			DestroyParty( pParty->GetPartyID(), true );
		}
		else if( pParty->bIsMove() == false )
		{
			pParty->SendAllRefreshParty(PARTYREFRESH_NONE);
		}
	}

	return ERROR_NONE;
}

void CDNPartyManager::QueryDelPartyForServer( int iServerID, DWORD dwTick/*=0*/ )
{
	if( dwTick == 0 )	
		dwTick = timeGetTime();

	m_DeletePartyForServerList[iServerID] = dwTick;

	CDNDBConnection* pDBCon = GetDBConnection();
	
	if( pDBCon && pDBCon->GetActive() )
		pDBCon->QueryDelPartyForServer( g_Config.nWorldSetID, iServerID );
}

void CDNPartyManager::QueryDelPartyForServerResponse( const TADelPartyForServer* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	m_DeletePartyForServerList.erase( pPacket->iServerID );
}

void CDNPartyManager::QueryGetListParty( DWORD dwTick/*=0*/ )
{
	if( dwTick == 0 )
		dwTick = timeGetTime();

	CDNDBConnection* pDBCon = GetDBConnection();
	if( pDBCon && pDBCon->GetActive() )
		pDBCon->QueryGetListParty( g_Config.nWorldSetID );

	m_dwGetListPartyTick = dwTick;
}

void CDNPartyManager::QueryGetListPartyResponse( const TAGetListParty* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	if( pPacket->bIsStart == true )
	{
		// Remove parties not on my server.
		for( TMapParty::iterator itor=m_pMapPartyList.begin() ; itor!=m_pMapPartyList.end() ;  )
		{
			CDNParty* pParty = (*itor).second;

			if( pParty->GetServerID() != g_Config.nManagedID )
			{
				m_pMapPartyList.erase( itor++ );
				SAFE_DELETE( pParty );
			}
			else
				++itor;
		}
	}

	for( int i=0 ; i<pPacket->nCount ; ++i )
	{
		PushParty( pPacket->DataArr[i] );
	}
}

bool CDNPartyManager::QueryDelParty( TPARTYID PartyID, DWORD dwTick/*=0*/, CDNDBConnection* pLoopDBCon/*=NULL*/ )
{
	if( dwTick == 0 )
		dwTick = timeGetTime();
	
	/*
	CDNParty* pParty = GetParty( PartyID );
	if( pParty == NULL )
		return false;
	*/

	m_DeletePartyList[PartyID] = dwTick;

	if( pLoopDBCon && pLoopDBCon->GetActive() )
	{		
		pLoopDBCon->QueryDelParty( g_Config.nWorldSetID, PartyID );
		return true;
	}

	CDNDBConnection* pDBCon = GetDBConnection();

	if( pDBCon && pDBCon->GetActive() )
		pDBCon->QueryDelParty( g_Config.nWorldSetID, PartyID );

	return true;
}

void CDNPartyManager::QueryDelPartyResponse( const TADelParty* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	m_DeletePartyList.erase( pPacket->PartyID );
}

bool CDNPartyManager::QueryOutParty( TPARTYID PartyID, UINT uiAccountDBID, INT64 biCharacterDBID, UINT nSessionID, Party::QueryOutPartyType::eCode Type, DWORD dwTick/*=0*/, CDNDBConnection* pLoopDBCon/*=NULL*/ )
{
	if( dwTick == 0 )
		dwTick = timeGetTime();

	CDNParty* pParty = GetParty( PartyID );
	if( pParty == NULL )
		return false;

	m_OutPartyList[std::make_tuple(PartyID,uiAccountDBID,biCharacterDBID,nSessionID,Type)] = dwTick;

	if( pLoopDBCon && pLoopDBCon->GetActive() )
	{
		pLoopDBCon->QueryOutParty( g_Config.nWorldSetID, uiAccountDBID, biCharacterDBID, nSessionID, PartyID, Type );
		return true;
	}

	CDNDBConnection* pDBCon = GetDBConnection();

	if( pDBCon && pDBCon->GetActive() )
		pDBCon->QueryOutParty( g_Config.nWorldSetID, uiAccountDBID, biCharacterDBID, nSessionID, PartyID, Type );
	
	return true;
}

void CDNPartyManager::QueryOutPartyResponse( const TAOutParty* pPacket )
{
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	m_OutPartyList.erase( std::make_tuple(pPacket->PartyID,pPacket->nAccountDBID,pPacket->biCharacterDBID,pPacket->nSessionID,pPacket->Type) );
}

void CDNPartyManager::ModParty( const TAModParty* pPacket )
{
	CDNParty * pParty = GetParty(pPacket->PartyData.PartyID);
	if (!pParty) 
		return;
	if( pPacket->nRetCode != ERROR_NONE )
		return;

	pParty->SetPartyStructData( pPacket->PartyData );
	pParty->SendAllRefreshParty(PARTYREFRESH_MODIFY_INFO);
}

#endif // #if defined( PRE_PARTY_DB )

#if defined( PRE_WORLDCOMBINE_PARTY )

bool CDNPartyManager::QueryGetListWorldParty()
{
	CDNDBConnection* pDBCon = GetDBConnection();

	if( pDBCon && pDBCon->GetActive() )
		pDBCon->QueryGetListWorldParty( g_Config.nWorldSetID );

	return true;
}
void CDNPartyManager::QueryGetListWorldPartyResponse( const TAGetListParty* pPacket )
{
	// Single target map, so use it as reference		
	TWORLDCOBINEPARTY ExistWorldPartyList;
	ExistWorldPartyList.clear();
	TWORLDCOBINEPARTY::iterator itor;
	WorldCombineParty::WrldCombinePartyData WorldCombineData;
	memset( &WorldCombineData, 0, sizeof(WorldCombineParty::WrldCombinePartyData));

	for(itor = m_WorldReqCombineParty.begin(); itor != m_WorldReqCombineParty.end();itor++)
	{
		ExistWorldPartyList.insert( make_pair( itor->first, WorldCombineData) );		
	}

	for( int i=0 ; i<pPacket->nCount ; ++i )
	{
		itor = m_WorldReqCombineParty.find(pPacket->DataArr[i].iTargetMapIndex);
		if( itor == m_WorldReqCombineParty.end())
		{			
			ExistWorldPartyList.insert( make_pair(pPacket->DataArr[i].iTargetMapIndex, WorldCombineData) );			
		}	
	}
		

	TWORLDCOBINEPARTY CreateCombineParty;
	CreateCombineParty.clear();
	g_pDataManager->CheckCombinePartyData(ExistWorldPartyList, CreateCombineParty);	

	if( !CreateCombineParty.empty() )
	{
		// Create cross-world party room
		for( TWORLDCOBINEPARTY::iterator itor = CreateCombineParty.begin();itor != CreateCombineParty.end();itor++ )
		{
			WorldCombineParty::WrldCombinePartyData Data = itor->second;
			ReqAddWorldParty( &Data );
		}
	}
}

void CDNPartyManager::ReqAddWorldParty( WorldCombineParty::WrldCombinePartyData *Data )
{
	CDNParty* pParty = new CDNParty(0);

	Party::Data PartyData;
	PartyData.Clear();
	PartyData.iBitFlag |= Party::BitFlag::WorldmapAllow;
	PartyData.iBitFlag |= Party::BitFlag::JobDice;
	PartyData.LocationType = Party::LocationType::Worldmap;
	PartyData.iTargetMapIndex = Data->nTargetMap;
	PartyData.Type = Data->PartyType;
	PartyData.nPartyMaxCount = Data->cPartyMemberMax;
	PartyData.cMinLevel = Data->cUserLvLimitMin;
	PartyData.iLocationID = Data->nWorldMap;
	PartyData.LootItemRank = Data->ItemRank;
	PartyData.LootRule = Data->ItemLootRule;
	PartyData.TargetMapDifficulty = Data->Difficulty;
	_wcscpy( PartyData.wszPartyName, _countof(PartyData.wszPartyName), Data->wszPartyName, (int)wcslen(Data->wszPartyName) );
	PartyData.iBitFlag = Data->iBitFlag;
	PartyData.nWorldSetID = g_Config.nWorldSetID;
	PartyData.nPrimaryIndex = Data->nTargetMap;	
	pParty->SetPartyStructData(PartyData);
	pParty->SetPartyID( Data->nTargetMap );	

	if(g_pMasterConnection && g_pMasterConnection->GetActive())
	{	
		g_pMasterConnection->SendReqGameID( GameTaskType::Normal, REQINFO_TYPE_PARTY, pParty->GetPartyID(), g_Config.nVillageID, pParty->GetRandomSeed(), pParty->GetPartyMapIndex(), 1, pParty->GetDifficulty(), true, pParty );		// Request GameID
	}		

	m_WorldReqCombineParty.insert( make_pair(Data->nTargetMap, *Data) );
}
void CDNPartyManager::DelCombinePartyData( int nPrimaryIndex )
{	
	TWORLDCOBINEPARTY::iterator itor = m_WorldReqCombineParty.find(nPrimaryIndex);
	if( itor != m_WorldReqCombineParty.end() )
		m_WorldReqCombineParty.erase(itor);
}
void CDNPartyManager::ClearReqWorldParty( )
{		
	m_WorldReqCombineParty.clear();
}
#endif