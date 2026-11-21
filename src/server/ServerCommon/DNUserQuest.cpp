#include "stdafx.h"
#include "DNUserQuest.h"
#include "DNUserSendManager.h"
#include "DNQuestManager.h"
#include "DNQuest.h"
#include "DNGameDataManager.h"
#include "DNLogConnection.h"
#include "DNDBConnectionManager.h"
#include "DNUserSession.h"
#include "DNMissionSystem.h"
#include "DNDBConnection.h"
#include "DNQuestCondition.h"
#include "DNPeriodQuestSystem.h"
#if defined( PRE_ADD_STAMPSYSTEM )
#include "DNStampSystem.h"
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined(_VILLAGESERVER)
#include "DNScriptAPI.h"

extern TVillageConfig g_Config;

#elif defined(_GAMESERVER)
#include "DNGameRoom.h"
#include "DnActor.h"
#include "DNGameServerScriptAPI.h"
#endif

CDNUserQuest::CDNUserQuest(CDNUserSession *pSession)
{
	m_pSession = pSession;
	m_RewardQuestID = 0;
	m_RewardCheck = 0;
	m_pQuest = pSession->GetQuestData();

	memset(&m_PacketReward, 0x00, sizeof(m_PacketReward));
	memset(&m_QuestReward, 0, sizeof(m_QuestReward));
}

CDNUserQuest::~CDNUserQuest()
{
#if defined(PRE_ADD_REMOTE_QUEST)
	_ResetRemoteQuest();
#endif
}

int	CDNUserQuest::AddHuntingQuest(int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt)
{

	if( _FindPlayingQuest(nQuestID) > -1 ) 
	{
#ifndef _FINAL_BUILD
		// Already in progress.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Quest is already in progress : %d" , nQuestID );
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
		return -2;
	}

	int nEmptyIdx = _FindEmptyPlayQuestIndex();
	if( nEmptyIdx == -1 )
	{
#ifndef _FINAL_BUILD
		// Cannot accept any more quests.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Cannot accept more quests. Complete active quests first : %d" , nQuestID );

		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
		return -1;
	}

	int nQuestType = g_pQuestManager->GetQuestType(nQuestID);
	if( nQuestType <= QuestType_Min || nQuestType >= QuestType_Max )
	{
#ifndef _FINAL_BUILD
		// Cannot accept any more quests.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Invalid quest type : %d" , nQuestID );

		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
		return -1;
	}

	// Check quest conditions.	
	QuestCondition * pQuestCondition = g_pDataManager->GetQuestCondition( nQuestID );
	if( pQuestCondition )
	{
		if( pQuestCondition->Check( m_pSession ) == false )
		{
			g_Log.Log(LogType::_QUESTHACK, 0, m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), 0, L"[ADBID:%u, CDBID:%I64d] [AddQuest - ConditionCheck] [QID:%d]\r\n", 
				m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), nQuestID);

			if( m_pSession->GetAccountLevel() == AccountLevel_QA )
			{
				std::wstring wszMsg;
				wszMsg = FormatW( L"Quest requirements not met");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
			}
			else
			{
				m_pSession->DetachConnection(L"AddQuest - ConditionCheck");
				return -1;
			}
		}
	}

	if( nCountingSlot < 0 || nCountingSlot >= QUEST_EXTRACOUNT_MAX) return -3;
	// Check out-of-range for each variable 
	if( nCountingType < 1 || nCountingType >= CountingType_Max || nTargetCnt < 1 || nTargetCnt > 31000 || nCountingIndex < 0  ) return -4;

	// Add quest 
	m_pQuest->Quest[nEmptyIdx].nQuestID = nQuestID;

	m_pQuest->Quest[nEmptyIdx].cQuestState = QuestState_Playing;
	m_pQuest->Quest[nEmptyIdx].nQuestStep = nQuestStep;
	m_pQuest->Quest[nEmptyIdx].cQuestJournal = nJournalStep;

	TCount* pCount = (TCount*)m_pQuest->Quest[nEmptyIdx].Extra;
	pCount[nCountingSlot].cType  = nCountingType;
	pCount[nCountingSlot].nIndex = nCountingIndex;
	pCount[nCountingSlot].nCnt = 0;
	pCount[nCountingSlot].nTargetCnt = nTargetCnt;

	CDNUserItem *pItem = m_pSession->GetItem();

	// When item count needs to be tallied
	if( nCountingType == CountingType_Item && pItem )
	{
		// If item is already in inventory 
		int nIdx = pItem->GetInventoryItemCount( nCountingIndex );

		if( nIdx > 0 )
			OnAddItem(nCountingIndex, nIdx);

		// Or if the item is already in the quest inventory 
		nIdx = pItem->GetQuestInventoryItemCount( nCountingIndex );
		if( nIdx > 0 )
			OnAddItem(nCountingIndex,nIdx );
	}

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest added : %d" , nQuestID );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::ADD_QUEST, nQuestID);

	m_pSession->GetDBConnection()->QueryAddQuest( m_pSession, nEmptyIdx, m_pQuest->Quest[nEmptyIdx]);	// Save to DB
	m_pSession->GetEventSystem()->OnEvent( EventSystem::OnQuestGain, 1, EventSystem::QuestID, nQuestID );

	return 1;
}

int CDNUserQuest::AddQuest(int nQuestID, bool bSendErrorMsg/*=false*/)
{

	if(IsClearQuest(nQuestID)) {
		g_Log.Log(LogType::_QUESTHACK, 0, m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), 0, L"[ADBID:%u, CDBID:%I64d] [AddQuest - IsClearQuest] [QID:%d]\r\n", 
			m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), nQuestID
			);

		m_pSession->DetachConnection(L"AddQuest - IsClearQuest");
		return -1;
	}

	if( _FindPlayingQuest(nQuestID) > -1 ) 
	{
#ifndef _FINAL_BUILD
		// Already in progress.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Quest is already in progress : %d" , nQuestID );
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
#if defined(PRE_ADD_REMOTE_QUEST)
		if(bSendErrorMsg)
			m_pSession->SendQuestResult(ERROR_QUEST_ALREADY_PLAYING);
#endif
		return -2;
	}

	int nEmptyIdx = _FindEmptyPlayQuestIndex();
	if( nEmptyIdx == -1 )
	{
#ifndef _FINAL_BUILD
		// Cannot accept any more quests.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Cannot accept more quests. Complete active quests first : %d" , nQuestID );

		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
#if defined(PRE_ADD_REMOTE_QUEST)
		if(bSendErrorMsg)
			m_pSession->SendQuestResult(ERROR_QUEST_FULLOFCOUNT_PLAYING);
#endif

		return -1;
	}

	int nQuestType = g_pQuestManager->GetQuestType(nQuestID);
	if( nQuestType <= QuestType_Min || nQuestType >= QuestType_Max )
	{
#ifndef _FINAL_BUILD
		// Cannot accept any more quests.
		std::wstring wszMsg;
		wszMsg = FormatW( L"Invalid quest type : %d" , nQuestID );

		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
		return -1;
	}

	// Check quest conditions.	
	QuestCondition * pQuestCondition = g_pDataManager->GetQuestCondition( nQuestID );
	if( pQuestCondition )
	{
		if( pQuestCondition->Check( m_pSession ) == false )
		{
			g_Log.Log(LogType::_QUESTHACK, 0, m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), 0, L"[ADBID:%u, CDBID:%I64d] [AddQuest - ConditionCheck] [QID:%d]\r\n", 
				m_pSession->GetAccountDBID(), m_pSession->GetCharacterDBID(), nQuestID);

			if( m_pSession->GetAccountLevel() == AccountLevel_QA )
			{
				std::wstring wszMsg;
				wszMsg = FormatW( L"Quest requirements not met");
				m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
			}
			else
			{
				m_pSession->DetachConnection(L"AddQuest - ConditionCheck");
				return -1;
			}
			
		}
	}

	// Add quest 
	m_pQuest->Quest[nEmptyIdx].nQuestID = nQuestID;
	m_pQuest->Quest[nEmptyIdx].cQuestState = QuestState_Playing;

#if defined(PRE_ADD_REMOTE_QUEST)
	RemoveRemoteQuestFromAcceptWaitList(nQuestID);
#endif

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest added : %d" , nQuestID );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::ADD_QUEST, nQuestID);

	m_pSession->GetDBConnection()->QueryAddQuest(m_pSession, nEmptyIdx, m_pQuest->Quest[nEmptyIdx]);	// Save to DB
	m_pSession->GetEventSystem()->OnEvent( EventSystem::OnQuestGain, 1, EventSystem::QuestID, nQuestID );

	return 1;
}


// Finish all quests in this function! (Some quests are handled via ForceCompleteQuest.)
// Had to write this procedurally for now; sorry to whoever reads this code next!
// 0. Process quest without reward
// 1. Check reward
// 2. Check quest
// 3. Mark quest finished
// 4. Delete quest
// 5. Reward processing
int CDNUserQuest::RewardAfterCompletingQuest(int nQuestID, bool bDelPlayList)
{

	// Do not trust the LUA script value for whether a quest is repeatable; trust only QuestTable info.
	CDNQuest* pQuest = g_pQuestManager->GetQuest(nQuestID);
	if(!pQuest)
	{
		m_pSession->SendQuestResult(ERROR_QUEST_NOTFOUND);
		return -2;
	}

	bool bRepeat = (pQuest->GetQuestInfo().cRepeatable == 1) ? true : false;

//--------------------------------------------------------------------------------------------------------------------------
// 0. Process quest without reward (mark -> complete)
	if(m_RewardQuestID == 0 && m_PacketReward.nIndex == 0 )
	{
		// Do not mark repeatable quests.
		if(bRepeat == false)
			MarkingCompleteQuest(nQuestID);
		
		return CompleteQuest(nQuestID, bDelPlayList, bRepeat);
	}


//--------------------------------------------------------------------------------------------------------------------------
// 1. Check reward

	// Check selection index (compare variable set by reward API against client packet value and handle exception)
#if defined(PRE_FIX_QUEST_GUILD_BOARD)
	if( m_PacketReward.nIndex != m_QuestReward.nIndex || m_QuestReward.nQuestID != nQuestID )
#else
	if( m_PacketReward.nIndex != m_QuestReward.nIndex )
#endif
	{
		m_pSession->SendQuestResult(ERROR_QUEST_REWARD_FAIL);
		return -1;
	}	

#if defined(_VILLAGESERVER)
	UINT nUserID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	UINT nUserID = m_pSession->GetSessionID();
#endif

	// Check max coin
	if(0 != m_QuestReward.nCoin && !m_pSession->CheckMaxCoin(m_QuestReward.nCoin))
	{
		m_pSession->SendQuestResult(ERROR_QUEST_REWARD_FAIL);
		return -1;
	}

	int nResult = CheckInvenForQuestReward(m_pSession, m_QuestReward, m_PacketReward.SelectArray);
	if (nResult < 1)
		return -1;

//--------------------------------------------------------------------------------------------------------------------------
// 2. Check quest
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot == -1 || nSlot >= MAX_PLAY_QUEST )
	{
		m_pSession->SendQuestResult(ERROR_QUEST_NOTFOUND);
		return -2;
	}

#if defined(PRE_FIX_QUEST_GUILD_BOARD)
// Verify quest completion step
	if( GetQuestStep(nQuestID) != m_QuestReward.nQuestStep )
	{
		m_pSession->SendQuestResult(ERROR_QUEST_REWARD_FAIL);
		return -1;
	}
#endif

//--------------------------------------------------------------------------------------------------------------------------
// 3. Mark quest completed

	std::wstring wszMsg;

	// Do not mark if it is a repeatable quest.
	if(bRepeat == false)
		MarkingCompleteQuest(nQuestID);

//--------------------------------------------------------------------------------------------------------------------------
// 4. Delete quest
	CompleteQuest(nQuestID, bDelPlayList, bRepeat);
//--------------------------------------------------------------------------------------------------------------------------
// 5. Reward processing
	OnSelectRewardEx(m_PacketReward.SelectArray, m_PacketReward.SelectCashArray);

	return 1;
}

void CDNUserQuest::ResetRewardFlag()
{
	m_RewardQuestID = 0;
	m_RewardCheck = 0;

	memset(&m_PacketReward, 0x00, sizeof(m_PacketReward));
#if defined(PRE_FIX_QUEST_GUILD_BOARD)
	memset(&m_QuestReward, 0, sizeof(m_QuestReward));
#endif
}

int CDNUserQuest::CompleteQuest(int nQuestID, bool bDelPlayList, bool bRepeat)
{

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot == -1 || nSlot >= MAX_PLAY_QUEST )
	{
		// Not an active quest
		return -2;
	}

#ifndef _FINAL_BUILD
	// After completing the quest
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest completed : %d" , nQuestID );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Remove from the currently active quest list
	if( bDelPlayList )
	{
		_RemoveQuest(nQuestID, true, bRepeat);

		// Send quest struct to client to update
		m_pSession->SendCompleteQuest(true, nSlot, nQuestID, false);
#if defined( PRE_ADD_STAMPSYSTEM )
		m_pSession->GetStampSystem()->OnCompleteQuest( nQuestID );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
		return 1;
	}

	m_pSession->SendCompleteQuest(false, nSlot, nQuestID, false);
#if defined( PRE_ADD_STAMPSYSTEM )
	m_pSession->GetStampSystem()->OnCompleteQuest( nQuestID );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
	return 1;
}

int	CDNUserQuest::MarkingCompleteQuest(int nQuestID)
{
	/////////////////////////////////////////////////////////////////
	//This is defensive code because it is called from the script side.
	CDNQuest* pQuest = g_pQuestManager->GetQuest(nQuestID);
	if(!pQuest)
	{
		ASSERT(0);
		return -2;
	}

	if(pQuest->GetQuestInfo().cRepeatable == 1)
		return 1;
	/////////////////////////////////////////////////////////////////

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot == -1 || nSlot >= MAX_PLAY_QUEST )
	{
		// Not an active quest
		return -2;
	}

#ifndef _FINAL_BUILD
	// After completing the quest
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest marked complete : %d" , nQuestID );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Add to the completed list here.
	SetCompleteQuestFlag(nQuestID, true);

	if (g_pPeriodQuestSystem->IsPeriodQuest(nQuestID))
		AddCompletedPeriodQuest(nQuestID);

	// Send quest struct to client to update
	m_pSession->SendMarkingCompleteQuest(nQuestID);

	m_pSession->GetEventSystem()->OnEvent( EventSystem::OnQuestClear, 1,	EventSystem::QuestID, nQuestID );
	return 1;
}

int CDNUserQuest::ForceCompleteQuest(int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat)
{
	// Does not matter whether the quest is currently in progress

	if(bDoMark) {
#ifndef _FINAL_BUILD
		// After completing the quest
		std::wstring wszMsg;
		wszMsg = FormatW( L"Quest marked complete : %d" , nQuestID );
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

		// Add to the completed list here.
		SetCompleteQuestFlag(nQuestID, true);

		if (g_pPeriodQuestSystem->IsPeriodQuest(nQuestID))
			AddCompletedPeriodQuest(nQuestID);

		// Send quest struct to client to update
		m_pSession->SendMarkingCompleteQuest(nQuestID);
	}

	int nSlot = _FindPlayingQuest(nQuestID);
	if(0 <= nSlot) 
	{
		int nResult = CompleteQuest(nQuestID, ((bDoDelete)?(true):(false)), ((bDoRepeat)?(true):(false)));

		if( nResult < 0 ) {
#ifndef _FINAL_BUILD
			wstring wszString = FormatW(L"Failed to complete quest.\r\n");
			m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszString.size()*sizeof(WCHAR), L"", (WCHAR*)wszString.c_str());
#endif	// _FINAL_BUILD
			return -3;
		}
	}
	else {
		m_pSession->GetDBConnection()->QueryForceCompleteQuest(m_pSession, nQuestID, nQuestCode, GetQuestCompleteBinary());
		
//		m_pSession->SendCompleteQuest(true, nSlot, nQuestID, false); // Since the quest is not owned anyway, do we not need to send quest completion info to the client???
	}

	m_pSession->GetEventSystem()->OnEvent( EventSystem::OnQuestClear, 1, EventSystem::QuestID, nQuestID );

	return 1;
}

int	CDNUserQuest::GetPlayingQuestCount()
{
	int nCnt = 0;
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
#if defined(PRE_ADD_REMOTE_QUEST)
		if( m_pQuest->Quest[i].nQuestID > 0 && m_pQuest->Quest[i].cQuestState >= QuestState_Playing ) 
#else
		if( m_pQuest->Quest[i].nQuestID > 0 && m_pQuest->Quest[i].cQuestState == QuestState_Playing ) 
#endif
		{
			nCnt++;
		}
	}
	return nCnt;
}

short CDNUserQuest::HasQuest(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	return m_pQuest->Quest[nSlot].nQuestStep;
}


int	CDNUserQuest::_RemoveQuest(int nQuestID, bool bComplete, bool bRepeat)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}
	
#if defined(_VILLAGESERVER)
	UINT nUserID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	UINT nUserID = m_pSession->GetSessionID();
#endif

	//---------------------------------------------------------------------------------------------
	{
		// Delete quest items here.
		std::set<int> questItemSet;
		g_pDataManager->GetQuestItemSet(nQuestID, questItemSet);

		// Delete all symbols related to the cancelled quest
		std::set<int>::iterator it = questItemSet.begin();
		for( it ; it != questItemSet.end() ; it++ )
		{
			int nQuestItemIndex = *it;
#if defined(_VILLAGESERVER)
			int nCount = api_quest_HasQuestItem(nUserID, nQuestItemIndex, 1);
#elif defined(_GAMESERVER)
			int nCount = api_quest_HasQuestItem(m_pSession->GetGameRoom(), nUserID, nQuestItemIndex, 1);
#endif	// _VILLAGESERVER
			if( nCount > 0 )
			{
#if defined(_VILLAGESERVER)
				int nReturn = api_quest_DelQuestItem(nUserID, nQuestItemIndex, nCount, nQuestID);
#elif defined(_GAMESERVER)
				int nReturn = api_quest_DelQuestItem(m_pSession->GetGameRoom(), nUserID, nQuestItemIndex, nCount, nQuestID);
#endif	// _VILLAGESERVER
				//if(nReturn < 0 ) return nReturn;
			}
			// Error if less than 0.
			//else return nCount;
		}
	}

	// Erase quest
	m_pQuest->Quest[nSlot].nQuestID = 0;
	m_pQuest->Quest[nSlot].cQuestState = QuestState_None;

	ZeroMemory( &(m_pQuest->Quest[nSlot]), sizeof(TQuest));

	m_pSession->GetDBConnection()->QueryDelQuest( m_pSession, nSlot, nQuestID, bComplete, bRepeat, m_pQuest->CompleteQuest);

	if(bComplete)
	{
		int nScheduleID = -1;
		__time64_t tExpireDate = 0;
		if(g_pPeriodQuestSystem->FindEventScheduleID(nQuestID, nScheduleID, tExpireDate) && nScheduleID > 0 && tExpireDate > 0)
			m_pSession->GetDBConnection()->QueryCompleteEventQuest(m_pSession, nScheduleID, tExpireDate);
	}

#ifndef _FINAL_BUILD
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest removed : %d" , nQuestID );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	return nSlot;
}

int CDNUserQuest::_FindPlayingQuest(int nQuestID)
{
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
#if defined(PRE_ADD_REMOTE_QUEST)
		if( m_pQuest->Quest[i].nQuestID == nQuestID && m_pQuest->Quest[i].cQuestState >= QuestState_Playing ) 
#else
		if( m_pQuest->Quest[i].nQuestID == nQuestID && m_pQuest->Quest[i].cQuestState == QuestState_Playing ) 
#endif
			return i;
	}

	return -1;
}

int CDNUserQuest::_FindEmptyPlayQuestIndex()
{
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( m_pQuest->Quest[i].nQuestID < 1 ) 
			return i;
	}
	return -1;
}

int	CDNUserQuest::SetQuestStepAndJournalStep(int nQuestID, short nQuestStep, int nJournalStep, bool bFromScript)
{

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  ) // Quest not found.
		return -1;

	// Update quest step and..
	m_pQuest->Quest[nSlot].nQuestStep = nQuestStep;
	m_pQuest->Quest[nSlot].cQuestJournal = nJournalStep;

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest journal step changed : %d step: %d, journal: %d" , nQuestID, (int)nQuestStep, (int)nJournalStep );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::REFRESH_STEPANDJOURNAL, nQuestID);

#if defined( PRE_ADD_REMOTE_QUEST )
	CDNQuest * pQuest = g_pQuestManager->GetQuest( nQuestID );
	if( bFromScript && pQuest && QuestType_RemoteQuest == pQuest->GetQuestInfo().cQuestType && pQuest->GetQuestInfo().bCheckRemoteComplete )
	{
#if defined(_VILLAGESERVER)
		if( pQuest->CanRemoteCompleteStep( m_pSession->GetObjectID(), nQuestStep ) )
#elif defined(_GAMESERVER)
		if( pQuest->CanRemoteCompleteStep( m_pSession->GetGameRoom(), m_pSession->GetSessionID(), nQuestStep ) )
#endif	// _VILLAGESERVER
		{
			m_pQuest->Quest[nSlot].cQuestState = QuestState_Recompense;
			m_pSession->SendCompleteRemoteQuest( nQuestID );
		}
	}
#endif // #if defined( PRE_ADD_REMOTE_QUEST )

	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 100128

	return 1;
}

int CDNUserQuest::SetQuestStep(int nQuestID, short nQuestStep)
{

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -1;
	}

	// Update quest step and..
	m_pQuest->Quest[nSlot].nQuestStep = nQuestStep;

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest step changed : %d step: %d" , nQuestID, (int)nQuestStep );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::REFRESH_STEP, nQuestID);

#if defined( PRE_ADD_REMOTE_QUEST )
	CDNQuest * pQuest = g_pQuestManager->GetQuest( nQuestID );
	if( pQuest && QuestType_RemoteQuest == pQuest->GetQuestInfo().cQuestType && pQuest->GetQuestInfo().bCheckRemoteComplete )
	{
#if defined(_VILLAGESERVER)
		if( pQuest->CanRemoteCompleteStep( m_pSession->GetObjectID(), nQuestStep ) )
#elif defined(_GAMESERVER)
		if( pQuest->CanRemoteCompleteStep( m_pSession->GetGameRoom(), m_pSession->GetSessionID(), nQuestStep ) )
#endif	// _VILLAGESERVER
		{
			m_pQuest->Quest[nSlot].cQuestState = QuestState_Recompense;
			m_pSession->SendCompleteRemoteQuest( nQuestID );
		}
	}
#endif // #if defined( PRE_ADD_REMOTE_QUEST )

	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 100128

	return 1;
}

short CDNUserQuest::GetQuestStep(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	// Update quest step and..
	return m_pQuest->Quest[nSlot].nQuestStep;
}

int	CDNUserQuest::SetJournalStep(int nQuestID, int nJournalStep)
{

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -1;
	}

	// Update quest step and..
	m_pQuest->Quest[nSlot].cQuestJournal = nJournalStep;

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest journal step changed : %d step: %d" , nQuestID, (int)nJournalStep );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::REFRESH_JOURNAL, nQuestID);

	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 100128
	
	return 1;
}

int CDNUserQuest::GetJournalStep(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	// Update quest step and..
	return m_pQuest->Quest[nSlot].cQuestJournal;
}

int CDNUserQuest::SetQuestMemo(int nQuestID, char nMemoIndex, TP_QUESTMEMO nVal)
{

	if(!CHECK_LIMIT(nMemoIndex, QUESTMEMOMAX)) {
		// Quest memo index out of range
		DN_RETURN(-3);
	}

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		DN_RETURN(-2);
	}

	// Update quest memo
	m_pQuest->Quest[nSlot].nMemo[nMemoIndex] = nVal;

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest memo changed : %d Memo: %d / %d" , nQuestID, (int)nMemoIndex, nVal );
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	m_pSession->GetDBConnection()->QueryModQuest( m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 20100128

	return 1;
}

TP_QUESTMEMO CDNUserQuest::GetQuestMemo(int nQuestID, char nMemoIndex)
{
	if(!CHECK_LIMIT(nMemoIndex, QUESTMEMOMAX)) {
		// Quest memo index out of range
		DN_RETURN(-3);
	}

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		DN_RETURN(-2);
	}

	return(m_pQuest->Quest[nSlot].nMemo[nMemoIndex]);
}

int	CDNUserQuest::IncQuestMemo(int nQuestID, char nMemoIndex)
{

	if(!CHECK_LIMIT(nMemoIndex, QUESTMEMOMAX)) {
		// Quest memo index out of range
		DN_RETURN(LONG_MIN);
	}

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		DN_RETURN(LONG_MIN);
	}

	// Increase quest memo
	TP_QUESTMEMO nRetVal = ++m_pQuest->Quest[nSlot].nMemo[nMemoIndex];

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest memo incremented : %d Memo: %d / %d" , nQuestID, (int)nMemoIndex, nRetVal);
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 20100128

	return nRetVal;
}

int	CDNUserQuest::DecQuestMemo(int nQuestID, char nMemoIndex)
{

	if(!CHECK_LIMIT(nMemoIndex, QUESTMEMOMAX)) {
		// Quest memo index out of range
		DN_RETURN(LONG_MAX);
	}

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		DN_RETURN(LONG_MAX);
	}

	// Decrease quest memo
	TP_QUESTMEMO nRetVal = --m_pQuest->Quest[nSlot].nMemo[nMemoIndex];

#ifndef _FINAL_BUILD
	// Should notify client about this too so it updates.
	std::wstring wszMsg;
	wszMsg = FormatW( L"Quest memo decremented : %d Memo: %d / %d" , nQuestID, (int)nMemoIndex, nRetVal);
	m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD

	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 20100128

	return nRetVal;
}

int	CDNUserQuest::SetCountingInfo(int nQuestID, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt)
{

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	if( nCountingSlot < 0 || nCountingSlot >= QUEST_EXTRACOUNT_MAX)
	{
		return -3;
	}

	// Check out-of-range for each variable 
	if( nCountingType < 1 || nCountingType >= CountingType_Max || nTargetCnt < 1 || nTargetCnt > 31000 || nCountingIndex < 0  )
	{
		return -4;
	}

	TCount* pCount = (TCount*)m_pQuest->Quest[nSlot].Extra;
	pCount[nCountingSlot].cType  = nCountingType;
	pCount[nCountingSlot].nIndex = nCountingIndex;
	pCount[nCountingSlot].nCnt = 0;
	pCount[nCountingSlot].nTargetCnt = nTargetCnt;

	CDNUserItem* pItem = NULL;
	m_pSession->GetDBConnection()->QueryModQuest( m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 20100128

	pItem = m_pSession->GetItem();

	// When item count needs to be tallied
	if( nCountingType == CountingType_Item && pItem )
	{
		// If item is already in inventory 
		int nIdx = pItem->GetInventoryItemCount( nCountingIndex );
		
		if( nIdx > 0 )
		{
			OnAddItem(nCountingIndex, nIdx);
		}

		// Or if the item is already in the quest inventory 
		nIdx = pItem->GetQuestInventoryItemCount( nCountingIndex );
		if( nIdx > 0 )
		{
			OnAddItem(nCountingIndex,nIdx );
		}

	}

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::REFRESH_COUNTING, nQuestID);

	return 1;
}

int CDNUserQuest::ClearCountingInfo(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	ZeroMemory( &(m_pQuest->Quest[nSlot].Extra), ExtraSize);
	m_pSession->GetDBConnection()->QueryModQuest( m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 20100128

	// Send quest struct to client to update
	SendQuestRefresh(RefreshType::REFRESH_COUNTING, nQuestID);

	return 1;
}

int CDNUserQuest::IsAllCompleteCounting(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  )
	{
		// Quest not found.
		return -2;
	}

	if( m_pQuest->Quest[nSlot].cQuestState == QuestState_Playing )
	{
		bool CompleteAllCounting = false;

		TCount* pCount = (TCount*)m_pQuest->Quest[nSlot].Extra;

		for( int j = 0 ; j < QUEST_EXTRACOUNT_MAX ; j++ )
		{
			if( pCount[j].cType != CountingType_None && pCount[j].cType != CountingType_Max )
			{
				// If any goal has not been fully met 
				if( pCount[j].nCnt < pCount[j].nTargetCnt )
				{
					return -3;
				}
				else if( pCount[j].nCnt == pCount[j].nTargetCnt )
				{
					CompleteAllCounting = true;
				}
			}
		}

		// When all counting conditions of the current quest are met 
		if( CompleteAllCounting )
		{
			return 1;
		}
	}

	return -3;

}

#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
void CDNUserQuest::OnCounting(int nCountingType, int nCountItemID, int nItemCount)
#else
void CDNUserQuest::OnCounting(int nCountingType, int nCountItemID)
#endif // #if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
{

	// P.S.> At the bottom of this method, there is a section that batch-writes TQuest::Extra updates to DB when needed, so be careful not to return mid-routine

	bool bCounting[MAX_PLAY_QUEST] = {0,};
	bool bComplete[MAX_PLAY_QUEST] = {0,};
	bool bModifyQuestExtra[MAX_PLAY_QUEST] = { false, };

	// Increment monster kill count when killing a monster.
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( m_pQuest->Quest[i].cQuestState == QuestState_Playing )
		{
			TCount* pCount = (TCount*)m_pQuest->Quest[i].Extra;

			for( int j = 0 ; j < QUEST_EXTRACOUNT_MAX ; j++ )
			{
				// Type to count is monster, and
				bool bNeedContinue = false;
				bool bNeedCount = true;

				// When counting type only counts monsters killed by me and I killed a monster.
				if( pCount[j].cType == CountingType_MyMonsterKill && nCountingType == CountingType_MyMonsterKill )
				{
					bNeedContinue = false;
				}
				// When set to global counting and a monster is killed by someone (including me)
				else if( pCount[j].cType == CountingType_AllMonsterKill && nCountingType == CountingType_AllMonsterKill )
				{
					bNeedContinue = false;
				}
				// Item 
				else if( pCount[j].cType == CountingType_Item && nCountingType == CountingType_Item )
				{
					bNeedContinue = false;
				}
				/*
				// Symbol item 
				else if( pCount[j].cType == CountingType_SymbolItem && nCountingType == CountingType_SymbolItem )
				{
					bNeedContinue = false;
				}
				*/
				else if( pCount[j].cType == CountingType_StageClear && nCountingType == CountingType_StageClear )
				{
					bNeedContinue = false;
					bNeedCount = false;
				}
				// Otherwise, do not count.
				else
				{
					bNeedContinue = true;
				}

				if( bNeedContinue )
					continue;

				//At that index, increment.
				if( pCount[j].nIndex == nCountItemID )
				{
#if defined( PRE_MOD_QUEST_COUNTING )
					if( pCount[j].nCnt < pCount[j].nTargetCnt ) // If there are multiple count targets and more than the specified count is recorded, progress is displayed incorrectly.
#else // #if defined( PRE_MOD_QUEST_COUNTING )
					// Remove condition considering potential issues with next step depending on OnCounting.
#endif // #if defined( PRE_MOD_QUEST_COUNTING )
					{
						bCounting[i] = true;
						if(bNeedCount)
						{
#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
							if( nCountingType == CountingType_Item && nItemCount > 0) // Just in case, only check the item count.
								pCount[j].nCnt += nItemCount;
							else
#endif //#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
								++pCount[j].nCnt;

							bModifyQuestExtra[i] = true; 
						}
						// Call the script.
						CDNQuest* pQuest = g_pQuestManager->GetQuest(m_pQuest->Quest[i].nQuestID);
						if( pQuest )
						{
#if defined(_VILLAGESERVER)
							pQuest->OnCounting(GetUserObjID(), pCount[j]);
#elif defined(_GAMESERVER)
							pQuest->OnCounting(m_pSession->GetGameRoom(), GetUserObjID(), pCount[j]);
#endif	// _VILLAGESERVER
							// Send quest struct to client to update
							SendQuestRefresh(RefreshType::REFRESH_COUNTING, m_pQuest->Quest[i].nQuestID);
						}
					}

#if defined ( PRE_MOD_QUEST_COUNTING )
					if( pCount[j].nCnt >= pCount[j].nTargetCnt && true == bCounting[i] ) // Do not call if OnCounting was not called.
#else // #if defined( PRE_MOD_QUEST_COUNTING )
					if( pCount[j].nCnt >= pCount[j].nTargetCnt )
#endif // #if defined( PRE_MOD_QUEST_COUNTING )
					{
						bCounting[i] = true;
						bComplete[i] = true;
						// Call the script.
						CDNQuest* pQuest = g_pQuestManager->GetQuest(m_pQuest->Quest[i].nQuestID);
						if( pQuest )
						{
#if defined(_VILLAGESERVER)
							pQuest->OnCompleteCounting(GetUserObjID(), pCount[j] );
#elif defined(_GAMESERVER)
							pQuest->OnCompleteCounting(m_pSession->GetGameRoom(), GetUserObjID(), pCount[j] );
#endif	// _VILLAGESERVER
							// Send quest struct to client to update
							SendQuestRefresh(RefreshType::REFRESH_COMPLETECOUNTING, m_pQuest->Quest[i].nQuestID);
						}
					}
				}
			}
		}
	}

	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( bComplete[i] && bCounting[i] &&
			 IsAllCompleteCounting(m_pQuest->Quest[i].nQuestID) > 0 )
		{
			// Call the script.
			CDNQuest* pQuest = g_pQuestManager->GetQuest(m_pQuest->Quest[i].nQuestID);
			if( pQuest )
			{
#if defined(_VILLAGESERVER)
				pQuest->OnAllCompleteCounting(GetUserObjID());
#elif defined(_GAMESERVER)
				pQuest->OnAllCompleteCounting(m_pSession->GetGameRoom(), GetUserObjID());
#endif	// _VILLAGESERVER
				// Send quest struct to client to update
				SendQuestRefresh(RefreshType::REFRESH_ALLCOMPLETECOUNTING, m_pQuest->Quest[i].nQuestID);
			}
		}
	}

	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ ) {
		if(bModifyQuestExtra[i]) {
			m_pSession->GetDBConnection()->QueryModQuest(m_pSession, i, m_pQuest->Quest[i], DBDNWorldDef::QuestStatusCode::Progress);	// Save to DB 100128
		}
	}
}

// When I killed a monster.
void CDNUserQuest::OnKillMonster(int nMonsterID)
{
	OnCounting(CountingType_MyMonsterKill, nMonsterID);
}

// When the monster dies.
void CDNUserQuest::OnDieMonster(int nMonsterID)
{
	OnCounting(CountingType_AllMonsterKill, nMonsterID);
}

// When item is obtained.
void CDNUserQuest::OnAddItem(int nItemID, short wCount)
{

#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
	OnCounting(CountingType_Item, nItemID, wCount);
#else
	for( short i = 0 ; i < wCount ; i++ )
	{
		OnCounting(CountingType_Item, nItemID);
	}
#endif // #if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
}

// When item is obtained.
void CDNUserQuest::OnAddSymbolItem(int nItemID)
{

	OnCounting(CountingType_SymbolItem, nItemID);
}

void CDNUserQuest::OnAddQuestItem(int nItemID)
{

	OnCounting(CountingType_Item, nItemID);
}

void CDNUserQuest::OnStageClear(int nRank)
{

	OnCounting(CountingType_StageClear, nRank);
}

UINT CDNUserQuest::GetUserObjID()
{
#if defined(_VILLAGESERVER)
	return m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	return m_pSession->GetSessionID();
#endif
}

bool CDNUserQuest::IsClearQuest(int nQuestID)
{
	if( nQuestID < 0 || nQuestID >= MAX_QUEST_INDEX )
		return false;

	int nIndex = nQuestID / 8;
	int nMaskType = nQuestID % 8;
	char Result = 0;

	switch(nMaskType)
	{
	case 1: Result = m_pQuest->CompleteQuest[nIndex] & 0x80; break;
	case 2: Result = m_pQuest->CompleteQuest[nIndex] & 0x40; break;
	case 3: Result = m_pQuest->CompleteQuest[nIndex] & 0x20; break;
	case 4: Result = m_pQuest->CompleteQuest[nIndex] & 0x10; break;
	case 5: Result = m_pQuest->CompleteQuest[nIndex] & 0x08; break;
	case 6: Result = m_pQuest->CompleteQuest[nIndex] & 0x04; break;
	case 7: Result = m_pQuest->CompleteQuest[nIndex] & 0x02; break;
	case 0: Result = m_pQuest->CompleteQuest[nIndex] & 0x01; break;		
	}

	if(Result == 0x00) return false;

	return true;
}

void CDNUserQuest::SetCompleteQuestFlag(int nQuestID, bool bFlag)
{
	int nIndex = nQuestID / 8;
	int nMaskType = nQuestID % 8;

	if(nIndex >= COMPLETEQUEST_BITSIZE) return; 	

	if( bFlag )
	{
		switch(nMaskType)
		{
		case 1: m_pQuest->CompleteQuest[nIndex] |= 0x80; break;
		case 2: m_pQuest->CompleteQuest[nIndex] |= 0x40; break;
		case 3: m_pQuest->CompleteQuest[nIndex] |= 0x20; break;
		case 4: m_pQuest->CompleteQuest[nIndex] |= 0x10; break;
		case 5: m_pQuest->CompleteQuest[nIndex] |= 0x08; break;
		case 6: m_pQuest->CompleteQuest[nIndex] |= 0x04; break;
		case 7: m_pQuest->CompleteQuest[nIndex] |= 0x02; break;
		case 0: m_pQuest->CompleteQuest[nIndex] |= 0x01; break;		
		}
	}
	else
	{
		switch(nMaskType)
		{
		case 1: m_pQuest->CompleteQuest[nIndex] &= ~0x80; break;
		case 2: m_pQuest->CompleteQuest[nIndex] &= ~0x40; break;
		case 3: m_pQuest->CompleteQuest[nIndex] &= ~0x20; break;
		case 4: m_pQuest->CompleteQuest[nIndex] &= ~0x10; break;
		case 5: m_pQuest->CompleteQuest[nIndex] &= ~0x08; break;
		case 6: m_pQuest->CompleteQuest[nIndex] &= ~0x04; break;
		case 7: m_pQuest->CompleteQuest[nIndex] &= ~0x02; break;
		case 0: m_pQuest->CompleteQuest[nIndex] &= ~0x01; break;		
		}
	}
}

// Update quest info whose state changed to the client.
int CDNUserQuest::SendQuestRefresh(int nRefreshType, int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0  || nSlot > MAX_PLAY_QUEST )
	{
		// Quest not found.
		return -2;
	}

	m_pSession->SendQuestRefresh((char)nSlot, nRefreshType, nQuestID, m_pQuest->Quest[nSlot] );
	return 1;
}

void CDNUserQuest::SendAllQuestRefresh()
{
	m_pSession->SendQuestInfo(m_pSession->GetQuestData());
	m_pSession->SendQuestCompleteInfo(*m_pQuest);
}

void CDNUserQuest::ClearAllQuestInfo()
{
	DN_ASSERT(NULL != m_pQuest,	"Invalid!");

	::memset(m_pQuest->Quest, 0, sizeof(m_pQuest->Quest));
	::memset(m_pQuest->CompleteQuest, 0, sizeof(m_pQuest->CompleteQuest));

	m_pSession->GetDBConnection()->QueryClearQuest(m_pSession);
}

void CDNUserQuest::DumpQuest(std::vector<TQuest>& output)
{
	for (int i = 0; i < MAX_PLAY_QUEST; i++)
	{
		if(m_pQuest->Quest[i].nQuestID > 0 ) 
		{
			output.push_back(m_pQuest->Quest[i]);
		}
	}
}

int CDNUserQuest::FindPlayingQuest(int nQuestID)
{
	DN_ASSERT(0 != nQuestID,	"Invalid!");

	return _FindPlayingQuest(nQuestID);
}

#if defined(PRE_ADD_REMOTE_QUEST)
int CDNUserQuest::FindRecompenseQuestIndex(int nQuestID)
{
	DN_ASSERT(0 != nQuestID,	"Invalid!");

	return _FindRecompenseQuestIndex(nQuestID);
}

bool CDNUserQuest::SetQuestToRecompenseState(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0 ) return false;

	if( m_pQuest->Quest[nSlot].nQuestID != nQuestID ) return false;
	
	m_pQuest->Quest[nSlot].cQuestState = QuestState_Recompense;
	//SendQuestRefresh(RefreshType::REFRESH_STEPANDJOURNAL, nQuestID);
	m_pSession->GetDBConnection()->QueryModQuest(m_pSession, nSlot, m_pQuest->Quest[nSlot], DBDNWorldDef::QuestStatusCode::Progress);
	return true;
}

#endif

void CDNUserQuest::OnSetActiveQuest(int nQuestID)
{
	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0 )
	{
		m_pSession->SendQuestResult(ERROR_QUEST_SETACTIVEFAIL);
		return;
	}
}

// Find the highest quest step among active sub-quests. return QuestID; -1 if none
TQuest	CDNUserQuest::_FindHighestQStepSubQuest()
{
	TQuest FoundQuest;
	ZeroMemory(&FoundQuest, sizeof(TQuest));

	int nQuest = -1;
	int nMaxQuestStep = -1;

	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		// Is a sub-quest, and 
		if( m_pQuest->Quest[i].nQuestID > 0 
			&& g_pQuestManager->GetQuestType(m_pQuest->Quest[i].nQuestID) == QuestType_SubQuest 
			 && m_pQuest->Quest[i].nQuestStep > 0 
			 == false )
		{
			if( nMaxQuestStep <= m_pQuest->Quest[i].nQuestStep )
			{
				nQuest = m_pQuest->Quest[i].nQuestID;
				nMaxQuestStep = m_pQuest->Quest[i].nQuestStep;

				FoundQuest = m_pQuest->Quest[i];
			}
		}
	}

	return FoundQuest;
}

// Set the quest reward.
void CDNUserQuest::SetReward(TQuestReward& Reward, int nQuestID, int nRewardCheck)
{
	m_QuestReward = Reward;
	m_RewardQuestID = nQuestID;
	m_RewardCheck = nRewardCheck;
	
#if defined(PRE_ADD_REMOTE_QUEST)
	if( g_pQuestManager->GetQuestType(nQuestID) != QuestType_RemoteQuest )	//For remote quests, m_PacketReward is already set.
#endif
	// Initialize temporary data for storing the reward packet.
	// uIndex = 0 : defined as no reward.
	memset(&m_PacketReward, 0x00, sizeof(m_PacketReward));
}

// Temporarily store the reward packet.
void CDNUserQuest::SavePacketReward(CSSelectQuestReward* pPacket)
{
	memcpy(&m_PacketReward, pPacket, sizeof(CSSelectQuestReward));
}

bool CDNUserQuest::PreparePacketReward(int nQuestID)
{
	TQuestReward table;
	memset(&table, 0, sizeof(TQuestReward));

	bool bResult = g_pDataManager->GetQuestCommonRewardByQuestID(nQuestID, table);	//Modified to process only class-common reward info

	if(!bResult)
	{
#ifndef _FINAL_BUILD

#if defined(_VILLAGESERVER)
		UINT nUserID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
		UINT nUserID = m_pSession->GetSessionID();
#endif

		std::wstring wszMsg;
		wszMsg = FormatW( L"Quest reward table lookup failed UserObjID: %d QuestID:%d " , nUserID, nQuestID);
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif
		return false;
	}

	// Set up data so the reward can be claimed.
	memset(&m_PacketReward, 0x00, sizeof(m_PacketReward));

	m_PacketReward.nIndex = table.nIndex;

	m_QuestReward = table;
	m_RewardQuestID = nQuestID;
	m_RewardCheck = m_PacketReward.nIndex;

	return true;
}

int CDNUserQuest::_AddItemByQuestReward( const TQuestReward::_ITEMSET* pItemSet )
{

	CDNUserItem* pUserItem = m_pSession->GetItem();
	if( !pUserItem )
		return -2;

#if defined(_VILLAGESERVER)
	UINT nUserObjectID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	UINT nUserObjectID = m_pSession->GetSessionID();
#endif

#if defined( _GAMESERVER )
	int nResult = api_user_CheckInvenForAddItem( m_pSession->GetGameRoom(), nUserObjectID, pItemSet->nItemID, pItemSet->nItemCount );
#else
	int nResult = api_user_CheckInvenForAddItem(nUserObjectID, pItemSet->nItemID, pItemSet->nItemCount );
#endif // #if defined( _GAMESERVER )
	if( nResult < 0 )
		return nResult;

	bool bResult = pUserItem->AddInventoryByQuest( pItemSet, m_RewardQuestID, 0 );

#ifndef _FINAL_BUILD
	if(bResult == false)
	{
		std::wstring wszMsg;
		wszMsg = FormatW( L"Add item failed %s: id %d count : %d" , bResult ? L"" : L"FAILED", pItemSet->nItemID, pItemSet->nItemCount );
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
	}
#endif

	return 1;
}

void CDNUserQuest::OnSelectRewardEx(bool bSelectedArray[], bool bSelectedCashArray[])
{

#if defined(_VILLAGESERVER)
	UINT nUserID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	UINT nUserID = m_pSession->GetSessionID();
#endif

	TMailTableData *pMailData = g_pDataManager->GetMailTableData(m_QuestReward.nMailID);	

	if (m_QuestReward.nFatigue > 0)
	{
		int nFatigue = m_QuestReward.nFatigue;
		int nMaxFatigue = g_pDataManager->GetFatigue(m_pSession->GetUserJob(), m_pSession->GetLevel());
		if (nMaxFatigue - m_pSession->GetFatigue() < nFatigue)
			nFatigue = nMaxFatigue - m_pSession->GetFatigue();

		if (nFatigue > 0)
			m_pSession->IncreaseFatigue(nFatigue);
	}

	switch( m_QuestReward.cType)
	{
		//--------------------------------------------------------------------------------------------------------------------------
		// In case of full reward
	case 1:
		{

#if defined(_VILLAGESERVER)

			if(m_QuestReward.nCoin != 0) 
				api_user_AddCoin(nUserID, m_QuestReward.nCoin, m_RewardQuestID);
			if(m_QuestReward.nExp != 0)
				api_user_AddExp(nUserID, m_RewardQuestID, m_QuestReward.nExp);

			for( int i = 0 ; i < QUESTREWARD_INVENTORYITEMMAX ; i++ )
			{
				if( m_QuestReward.ItemArray[i].nItemID > 0 && m_QuestReward.ItemArray[i].nItemCount > 0 ) 
				{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					// If it is a favorability-increasing reward, only display the item and do not actually put it in inventory.
					if( NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID == m_QuestReward.ItemArray[i].nItemID ||
						NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID == m_QuestReward.ItemArray[i].nItemID )
						continue;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)

					_AddItemByQuestReward( &m_QuestReward.ItemArray[i] );
				}
			}

#elif defined(_GAMESERVER)
			if(m_QuestReward.nCoin != 0) 
				api_user_AddCoin(m_pSession->GetGameRoom(), nUserID, m_QuestReward.nCoin, m_RewardQuestID);
			if(m_QuestReward.nExp != 0)
				api_user_AddExp(m_pSession->GetGameRoom(), nUserID, m_RewardQuestID, m_QuestReward.nExp);

			for( int i = 0 ; i < QUESTREWARD_INVENTORYITEMMAX ; i++ )
			{
				if( m_QuestReward.ItemArray[i].nItemID > 0 && m_QuestReward.ItemArray[i].nItemCount > 0 ) 
				{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					// If it is a favorability-increasing reward, only display the item and do not actually put it in inventory.
					if( NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID == m_QuestReward.ItemArray[i].nItemID || 
						NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID == m_QuestReward.ItemArray[i].nItemID )
						continue;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					_AddItemByQuestReward( &m_QuestReward.ItemArray[i] );
				}
			}

#endif	// _VILLAGESERVER

			for (int i = 0; i < MAX_QUEST_LEVEL_CAP_REWARD; ++i)
			{
				if( m_QuestReward.LevelCapItemArray[i].nItemID > 0 && m_QuestReward.LevelCapItemArray[i].nItemCount > 0 ) 
				{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					// If it is a favorability-increasing reward, only display the item and do not actually put it in inventory.
					if( NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID == m_QuestReward.LevelCapItemArray[i].nItemID || 
						NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID == m_QuestReward.LevelCapItemArray[i].nItemID )
						continue;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					_AddItemByQuestReward( &m_QuestReward.LevelCapItemArray[i] );
				}
			}

			if(IsRewardCashItem())
			{
				if(pMailData)
					m_pSession->MakeGiftByQuest(m_RewardQuestID, pMailData->ItemSNArr);
				else
					return;
			}
		}
		break;

		//--------------------------------------------------------------------------------------------------------------------------
		// If selectable reward
	case 2:
		{
			int nRewardItem[MAILATTACHITEMMAX] = { 0, };
			int nClientItemCnt = 0, nClientCashItemCnt = 0;

			// Check how many items the client selected and sent.
			for( int i = 0 ; i < MAILATTACHITEMMAX ; i++ )
			{
				if(bSelectedArray[i])
					nClientItemCnt++;

				if(bSelectedCashArray[i]){
					if(pMailData)
						nRewardItem[i] = pMailData->ItemSNArr[i];
					else
						return;

					nClientCashItemCnt++;
				}
			}

			// Fail if count of items originally receivable differs from count selected by client
			if( m_QuestReward.cSelectMax != (nClientItemCnt + nClientCashItemCnt) )
			{
				m_pSession->SendQuestResult(ERROR_QUEST_REWARD_FAIL);
				return;
			}

#if defined(_VILLAGESERVER)
			if(m_QuestReward.nCoin != 0) 
				api_user_AddCoin(nUserID, m_QuestReward.nCoin, m_RewardQuestID);
			if(m_QuestReward.nExp != 0)
				api_user_AddExp(nUserID, m_RewardQuestID, m_QuestReward.nExp);
#elif defined(_GAMESERVER)
			if(m_QuestReward.nCoin != 0) 
				api_user_AddCoin(m_pSession->GetGameRoom(), nUserID, m_QuestReward.nCoin, m_RewardQuestID);
			if(m_QuestReward.nExp != 0)
				api_user_AddExp(m_pSession->GetGameRoom(), nUserID, m_RewardQuestID, m_QuestReward.nExp);
#endif	// _VILLAGESERVER

			// Otherwise, grant the gift now.
			if(nClientItemCnt > 0){
				for( int i = 0 ; i < QUESTREWARD_INVENTORYITEMMAX ; i++ )
				{
					// If selected item
					if( bSelectedArray[i] )
					{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
						// If it is a favorability-increasing reward, only display the item and do not actually put it in inventory.
						if( NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID == m_QuestReward.ItemArray[i].nItemID ||
							NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID == m_QuestReward.ItemArray[i].nItemID )
							continue;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)

						_AddItemByQuestReward( &m_QuestReward.ItemArray[i] );
					}
				}
			}

			for (int i = 0; i < MAX_QUEST_LEVEL_CAP_REWARD; ++i)
			{
				if( m_QuestReward.LevelCapItemArray[i].nItemID > 0 && m_QuestReward.LevelCapItemArray[i].nItemCount > 0 ) 
				{
#if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					// If it is a favorability-increasing reward, only display the item and do not actually put it in inventory.
					if( NPC_FAVOR_QUEST_REWARD_FAVOR_UP_ITEM_ID == m_QuestReward.LevelCapItemArray[i].nItemID || 
						NPC_FAVOR_QUEST_REWARD_BROMIDE_ITEM_ID == m_QuestReward.LevelCapItemArray[i].nItemID )
						continue;
#endif // #if defined(PRE_ADD_NPC_REPUTATION_SYSTEM)
					_AddItemByQuestReward( &m_QuestReward.LevelCapItemArray[i] );
				}
			}

			if((nClientCashItemCnt > 0) && IsRewardCashItem())
			{
				// Otherwise, grant the gift now.
				m_pSession->MakeGiftByQuest(m_RewardQuestID, nRewardItem);
			}
		}
		break;

	}
}

bool CDNUserQuest::IsRewardItem()
{
	for (int i = 0; i < QUESTREWARD_INVENTORYITEMMAX; i++){
		if(m_QuestReward.ItemArray[i].nItemID > 0) return true;
	}

	return false;
}

bool CDNUserQuest::IsRewardCashItem()
{
	return(m_QuestReward.nMailID > 0) ? true : false;
}

void CDNUserQuest::OnCancelQuest(int nQuestID)
{
#if defined(_VILLAGESERVER)
	UINT nUserID =  m_pSession->GetObjectID();
#elif defined(_GAMESERVER)
	UINT nUserID = m_pSession->GetSessionID();
#endif

	int nSlot = _FindPlayingQuest(nQuestID);
	if( nSlot < 0 )
	{
		m_pSession->SendRemoveQuest(0, nQuestID, ERROR_QUEST_CANCELFAIL);
		return;
	}

	// Erase quest and
	int nSlotID = _RemoveQuest(nQuestID, false, false);

	if(nSlotID == -2)
		m_pSession->SendRemoveQuest(nSlotID, nQuestID, ERROR_QUEST_NOTFOUND);
	else		// Send notification that it was deleted.
		m_pSession->SendRemoveQuest(nSlotID, nQuestID, ERROR_NONE);

#if defined(PRE_ADD_REMOTE_QUEST)
	if( nSlotID >= 0 && g_pQuestManager->GetQuestType(nQuestID) == QuestType_RemoteQuest )
	{	//If remote quest is accepted and abandoned, return to pending accept list
		m_pSession->GetQuest()->AddRemoteQuestToAcceptWaitList(nQuestID);
	}
#endif

}

void CDNUserQuest::OnPeriodEvent(int nQuestID, bool bFlag)
{
	if(true == bFlag)
	{
		int nSlot = _FindPlayingQuest(nQuestID);
		if( nSlot > 0 )
			_RemoveQuest(nQuestID, false, false);
	}
	
	SetCompleteQuestFlag(nQuestID, bFlag);
}

void CDNUserQuest::OnAddItemEx(int nItemID, short wCount)
{
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( m_pQuest->Quest[i].cQuestState == QuestState_Playing )
		{
			TCount* pCount = (TCount*)m_pQuest->Quest[i].Extra;
			for (int j=0; j<QUEST_EXTRACOUNT_MAX ; j++)
			{
				if(pCount[j].cType == CountingType_Item && 
					pCount[j].nIndex == nItemID &&
					wCount >= pCount[j].nTargetCnt)
				{
					CDNQuest* pQuest = g_pQuestManager->GetQuest(m_pQuest->Quest[i].nQuestID);
					if(pQuest)
					{
#if defined(_VILLAGESERVER)
						pQuest->OnItemCollecting(GetUserObjID(), nItemID, wCount);
#elif defined(_GAMESERVER)
						pQuest->OnItemCollecting(m_pSession->GetGameRoom(), GetUserObjID(), nItemID, wCount);
#endif	// _VILLAGESERVER
					}
				}
			}
		}
	}
}

void CDNUserQuest::AddCompletedPeriodQuest(int questID)
{
	m_CompletedPeriodQuests.insert(questID);
}

void CDNUserQuest::RemoveCompletedPeriodQuest(int questID)
{
	std::set<int>::iterator it = m_CompletedPeriodQuests.find(questID);
	if (it == m_CompletedPeriodQuests.end())
		return;

	m_CompletedPeriodQuests.erase(it);
#if defined(PRE_ADD_REMOTE_QUEST)
	SetCompleteQuestFlag(questID, false);	//Clear completion flags for daily quests being reset
#endif
}

bool CDNUserQuest::IsCompletedPeriodQuest(int questID) const
{
	return (m_CompletedPeriodQuests.find(questID) != m_CompletedPeriodQuests.end()) ? true : false;
}

#if defined(PRE_ADD_REMOTE_QUEST)

bool CDNUserQuest::OnEvent( EventSystem::EventTypeEnum Event )
{
	CheckRemoteQuestGain(Event);

	return true;
}

void CDNUserQuest::CheckRemoteQuestGain(EventSystem::EventTypeEnum Event)
{
	bool bQuestGain = false;
	int nArrayIndex;
	for( DWORD i=0; i<m_nVecRemoteQuestGainList[Event].size(); i++ ) {
		nArrayIndex = m_nVecRemoteQuestGainList[Event][i];

 		TRemoteQuestData *pData = g_pDataManager->GetRemoteQuestData(nArrayIndex);
		if(!pData) continue;

		bool bGain = false;
		bGain = m_pSession->GetEventSystem()->CheckEventCondition( &pData->GainCondition );
		if( bGain )
		{
			bGain = CheckRemotePreQuestCondition( pData );
		}

		if( bGain ) 
		{
			CDNQuest* pQuest = g_pQuestManager->GetQuest(pData->nQuestID);

			if( pQuest && _CheckRemoteQuestGainValidation(pData->nQuestID, pQuest) )
			{
				// If remote quest cannot be obtained
				if( !AddRemoteQuestToAcceptWaitList(pData->nQuestID) )
					continue;
			}

			if(!pQuest || pQuest->GetQuestInfo().cRepeatable != 1)
			{
				m_nVecRemoteQuestGainList[Event].erase( m_nVecRemoteQuestGainList[Event].begin() + i );
				i--;
			}
		}
	}
}

bool CDNUserQuest::CheckRemotePreQuestCondition( TRemoteQuestData *pData )
{
	if(!pData) return false;

	if( 0 == pData->nVecPreQuestCondition.size() )
	{
		return true;
	}

	if( pData->bPreQuestCheckType == false ) 
	{
		for( int i = 0 ; i < pData->nVecPreQuestCondition.size() ; ++ i ) 
		{
			if( IsClearQuest( pData->nVecPreQuestCondition[i] ) == false ) 
				return false;
		}
		return true;
	}
	else 
	{
		for( int i = 0 ; i < pData->nVecPreQuestCondition.size() ; ++ i ) 
		{
			if( IsClearQuest( pData->nVecPreQuestCondition[i] ) == true ) 
				return true;
		}
		return false;
	}
	return false;	
}

void CDNUserQuest::CheckAcceptWaitRemoteQuestList()
{
	for(int i = 0;  i < m_nVecRemoteQuestAcceptWaitList.size(); i++)
	{
		CDNQuest* pQuest = g_pQuestManager->GetQuest(m_nVecRemoteQuestAcceptWaitList[i]);
		if( !pQuest || (IsClearQuest(m_nVecRemoteQuestAcceptWaitList[i]) && pQuest->GetQuestInfo().cRepeatable != 1)
			|| FindPlayingQuest(m_nVecRemoteQuestAcceptWaitList[i]) >= 0 )
		{
			m_pSession->SendRemoveRemoteQuest(m_nVecRemoteQuestAcceptWaitList[i]);
			m_nVecRemoteQuestAcceptWaitList.erase(m_nVecRemoteQuestAcceptWaitList.begin() + i);
			i--;
		}
	}
}

bool CDNUserQuest::_CheckRemoteQuestGainValidation(int nQuestID, CDNQuest* pQuest)
{
	if(!pQuest) return false;

	//Discard if cleared when it is not a repeatable quest.
	if( IsClearQuest(nQuestID) && pQuest->GetQuestInfo().cRepeatable != 1 )
		return false;	

	//Cannot accept if quest is already in progress
	if( FindPlayingQuest(nQuestID) >= 0 )
		return false;

	//Remote quest already acquired
	if(_FindAcceptWaitQuest(nQuestID) >= 0)
		return false;

	return true;
}

bool CDNUserQuest::AddRemoteQuestToAcceptWaitList(int nQuestID)
{
	if( m_nVecRemoteQuestAcceptWaitList.size() >= MAX_PLAY_QUEST )
	{
#ifndef _FINAL_BUILD
		std::wstring wszMsg;
		wszMsg = FormatW( L"Cannot acquire any more remote quests : %d" , nQuestID );
		m_pSession->SendChat(CHATTYPE_NORMAL, (int)wszMsg.size()*sizeof(WCHAR), L"", (WCHAR*)wszMsg.c_str());
#endif // _FINAL_BUILD
		return false;
	}

	m_nVecRemoteQuestAcceptWaitList.push_back(nQuestID);
	m_pSession->SendGainRemoteQuest(nQuestID);
	return true;
}

bool CDNUserQuest::RemoveRemoteQuestFromAcceptWaitList(int nQuestID)
{
	int nIndex = _FindAcceptWaitQuest(nQuestID);
	if(nIndex < 0) return false;

	m_nVecRemoteQuestAcceptWaitList.erase(m_nVecRemoteQuestAcceptWaitList.begin() + nIndex);
	m_pSession->SendRemoveRemoteQuest(nQuestID);
	return true;
}

void CDNUserQuest::AcceptRemoteQuest(int nQuestID)
{
	int nIndex = _FindAcceptWaitQuest(nQuestID);
	if(nIndex < 0)
	{
		_DANGER_POINT();	//Users trying to accept a quest without acquiring it first (suspected abusers)
		return;
	}

	CDNQuest* pQuest = g_pQuestManager->GetQuest(nQuestID);
	if(!pQuest)
		return;
	
	if( AddQuest(nQuestID, true) == 1 )
	{
		SetQuestStepAndJournalStep(nQuestID, 1, 1, false);
#if defined(_VILLAGESERVER)
		pQuest->OnRemoteStart(m_pSession->GetObjectID());
#elif defined(_GAMESERVER)
		pQuest->OnRemoteStart(m_pSession->GetGameRoom(), m_pSession->GetSessionID());
#endif
	}
}

void CDNUserQuest::CancelRemoteQuest(int nQuestID)
{
	if(!RemoveRemoteQuestFromAcceptWaitList(nQuestID))
	{
		_DANGER_POINT();	//Users trying to abandon a quest without acquiring it first
		return;
	}

	//When abandoning an acquired remote quest, put it back into GainList (should it be made acquirable again?)
	TRemoteQuestData *pData = g_pDataManager->GetRemoteQuestDataByQuestID(nQuestID);
	if(pData)
	{
		CDNQuest* pQuest = g_pQuestManager->GetQuest(pData->nQuestID);
		if(!pQuest)
			return;

		if(pQuest->GetQuestInfo().cRepeatable != 1)	//If repeatable quest, do not delete; it is already in the list
			m_nVecRemoteQuestGainList[pData->GainCondition.cEvent].push_back(pData->nIndex);
	}
}

void CDNUserQuest::CompleteRemoteQuest(int nQuestID, CSSelectQuestReward* pPacket)
{
	if(FindRecompenseQuestIndex(nQuestID) < 0)
	{
		_DANGER_POINT();	//Not a completed quest -> handle as abusing
		ResetRewardFlag();
		return;
	}

	CDNQuest* pQuest = g_pQuestManager->GetQuest(nQuestID);
	if( pQuest )
	{
		SavePacketReward(pPacket);
#if defined(_VILLAGESERVER)
		pQuest->OnRemoteComplete(m_pSession->GetObjectID());
#elif defined(_GAMESERVER)
		pQuest->OnRemoteComplete(m_pSession->GetGameRoom(), m_pSession->GetSessionID());
#endif
	}
}

void CDNUserQuest::RefreshRemoteQuest(int nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList)
{
	_ResetRemoteQuest();
	//Set acquired remote quest list info
	if( nAcceptWaitRemoteQuestCount > MAX_PLAY_QUEST) nAcceptWaitRemoteQuestCount = MAX_PLAY_QUEST;
	for(int  i = 0 ; i < nAcceptWaitRemoteQuestCount ; i++)
			m_nVecRemoteQuestAcceptWaitList.push_back(AcceptWaitRemoteQuestList[i]);

	_RefreshRemoteQuestList();
}

int CDNUserQuest::_FindRecompenseQuestIndex(int nQuestID)
{
	for( int i = 0 ; i < MAX_PLAY_QUEST ; i++ )
	{
		if( m_pQuest->Quest[i].nQuestID == nQuestID && m_pQuest->Quest[i].cQuestState == QuestState_Recompense ) 
			return i;
	}

	return -1;
}

int CDNUserQuest::_FindAcceptWaitQuest(int nQuestID)
{
	for( int i = 0 ; i < m_nVecRemoteQuestAcceptWaitList.size() ; i++ )
	{
		if(m_nVecRemoteQuestAcceptWaitList[i] == nQuestID)
			return i;
	}

	return -1;
}

void CDNUserQuest::_ResetRemoteQuest()
{
	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ )
		SAFE_DELETE_VEC( m_nVecRemoteQuestGainList[i] );

	SAFE_DELETE_VEC(m_nVecRemoteQuestAcceptWaitList);
}

/*
For daily quests, ModifyCompleteQuest() marks unselected daily quests as Complete.
Because _RefreshRemoteQuestList() below is called earlier than ModifyCompleteQuest(), everything goes into the list first, though
When Gain event is actually called, unselected daily quests are caught by Complete marking, not acquired, and deleted.
*/
void CDNUserQuest::_RefreshRemoteQuestList()
{
	TQuestGroup *pQuest = m_pSession->GetQuestData();
	if(!pQuest)  return;

	int nSize = g_pDataManager->GetRemoteQuestDataSize();
	for(int i = 0 ; i < nSize; i++)
	{
		TRemoteQuestData *pData = g_pDataManager->GetRemoteQuestData(i);
		if(!pData || !pData->bActivate) continue;

		CDNQuest* pQuest = g_pQuestManager->GetQuest(pData->nQuestID);
		if( pQuest && _CheckRemoteQuestGainValidation(pData->nQuestID, pQuest) )
			m_nVecRemoteQuestGainList[pData->GainCondition.cEvent].push_back(i);
	}	
}

void CDNUserQuest::GetAcceptWaitRemoteQuest(int *nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList)
{	
	for(int i = 0 ; i < m_nVecRemoteQuestAcceptWaitList.size() ; i++)
	{
		if( i >= MAX_PLAY_QUEST ) break;
		AcceptWaitRemoteQuestList[i] = m_nVecRemoteQuestAcceptWaitList[i];
	}
	*nAcceptWaitRemoteQuestCount = (int)m_nVecRemoteQuestAcceptWaitList.size();
	if(*nAcceptWaitRemoteQuestCount > MAX_PLAY_QUEST)
		*nAcceptWaitRemoteQuestCount = MAX_PLAY_QUEST;
}
#endif	//#if defined(PRE_ADD_REMOTE_QUEST)
