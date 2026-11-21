
#include "stdafx.h"
#include "DNPeriodQuestSystem.h"
#include "DNQuestManager.h"
#include "DNSchedule.h"
#include "DNEvent.h"
#include "Util.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#include "DNGameDataManager.h"

#if defined(_VILLAGESERVER)
#include "DNUserSessionManager.h"
#include "DNMasterConnection.h"
#elif defined(_GAMESERVER)
#include "DNRUDPGameServer.h"
#include "DNGameServerManager.h"
#include "DNMasterConnectionManager.h"
#endif

CDNPeriodQuestSystem * g_pPeriodQuestSystem = NULL;

//////////////////////////////////////////////////////////////////////////
//							CDNPeriodQuest
//////////////////////////////////////////////////////////////////////////

CDNPeriodQuest::CDNPeriodQuest()
{
	Init();
}

CDNPeriodQuest::~CDNPeriodQuest()
{
	m_vecQuestIDs.clear();
}

void CDNPeriodQuest::Init()
{
	memset(m_EventTime, 0, sizeof(m_EventTime));
	memset(m_EventComplete, 0, sizeof(m_EventComplete));

	m_bActivate = true;
	m_bOnEvent = false;
	m_vecQuestIDs.clear();

	m_nScheduleID = 0;
	m_nSelectCount = 0;
}

void CDNPeriodQuest::Set(int nItemID, int nType, time_t tStartDate, time_t tEndDate, int nScheduleID, int nSelectCount)
{
	m_nItemID = nItemID;
	m_nType = nType;

	m_EventTime[PERIODQUEST_TIME_START] = tStartDate;
	m_EventTime[PERIODQUEST_TIME_END]	= tEndDate;

	m_nScheduleID = nScheduleID;
	m_nSelectCount = nSelectCount;
}

void CDNPeriodQuest::AddQuestID(int nQuestID)
{
	m_vecQuestIDs.push_back(nQuestID);
}

bool CDNPeriodQuest::CanAcceptDate() const
{
	__time64_t tCurrentDate = CTimeSet().GetTimeT64_LC();

	// Check date
	if( m_EventTime[PERIODQUEST_TIME_START] <= tCurrentDate && 
		 tCurrentDate <= m_EventTime[PERIODQUEST_TIME_END] || 
		 m_EventTime[PERIODQUEST_TIME_START] == m_EventTime[PERIODQUEST_TIME_END])
		return true;

	return false;
}

int CDNPeriodQuest::SelectQuestID(int nKey) const
{
	if(nKey <= 0)
		nKey = 1;

	size_t nCount = m_vecQuestIDs.size();
	if( nCount <= 0 )	
		return 0;

	UINT nIndex = nKey %(UINT)nCount;

	return m_vecQuestIDs[nIndex];
}

bool CDNPeriodQuest::CheckSelectedQuestID( int nKey, int nSelectedID, UINT uiOffset ) const
{
	size_t nCount = m_vecQuestIDs.size();
	if( nCount <= 0 )	
		return false;

	if(nKey <= 0)
		nKey = 1;

	UINT uiIndex = nKey %(UINT)nCount;

	if( m_vecQuestIDs[uiIndex] == nSelectedID )
		return true;

	if( uiIndex < uiOffset && uiOffset < uiIndex + m_nSelectCount )
		return true;

	if( uiIndex + m_nSelectCount > nCount && uiOffset < uiIndex + m_nSelectCount - nCount )
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
//						CDNPeriodQuestSystem
//////////////////////////////////////////////////////////////////////////

CDNPeriodQuestSystem::CDNPeriodQuestSystem()
{
	memset(m_nKeys, 0x00, sizeof(m_nKeys));
	RefreshQuestKey(PERIODQUEST_RESET_MAX);

	m_TickCheckQuest = 0;
	m_nWorldQuestIndex = 0;
	m_nPeriodQuestCount = 0;

	m_nResetHour = 4;
	DNTableFileFormat *pSox = GetDNTable(CDnTableDB::TSCHEDULE );
	if(pSox)
		m_nResetHour = pSox->GetFieldFromLablePtr(CDNSchedule::AlarmResetDailyMission + 1, "_Hour")->GetInteger();


	for(MWorldQuestItor itor = m_mWorldEventQuest.begin(); itor != m_mWorldEventQuest.end(); itor++)
		(*itor).second.clear();
	m_mWorldEventQuest.clear();
	m_bInitFlag = false;
}

CDNPeriodQuestSystem::~CDNPeriodQuestSystem()
{
}

bool CDNPeriodQuestSystem::Initialize()
{
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TQUESTPERIOD);
	if(!pSox) 
	{
		g_Log.Log( LogType::_FILELOG, L"QuestDailyTable.ext failed\r\n");
		return false;
	}

	int nItemID;
	int nType;
	int nScheuldeID;
	char * pszQuestIDs;
	char * pszCompleteDate;
	int nSelectCount;

	for( int i=0; i<pSox->GetItemCount(); i++ )
	{
		CDNPeriodQuest PeriodQuest;

		nItemID = pSox->GetItemID(i);
		nType = pSox->GetFieldFromLablePtr( nItemID, "_DailyQuestType" )->GetInteger();
		nScheuldeID = pSox->GetFieldFromLablePtr( nItemID, "_ScheduleID" )->GetInteger();
		pszQuestIDs = pSox->GetFieldFromLablePtr( nItemID, "_RandomQuestID" )->GetString();
		pszCompleteDate = pSox->GetFieldFromLablePtr( nItemID, "_CompleteDate" )->GetString();
		nSelectCount = pSox->GetFieldFromLablePtr( nItemID, "_SelectCount" )->GetInteger();

		if(!IsValidType(nType))
		{
			g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext InvalidType(_DailyQuestType)[%d][ScheduleID:%d]\r\n", nType, nScheuldeID);
			return false;
		}

		std::vector<string> vQuestIDs;
		std::vector<string> vCompleteDate;
		TokenizeA(pszQuestIDs, vQuestIDs, ":");
		TokenizeA(pszCompleteDate, vCompleteDate, ":");
		                       
		if(vQuestIDs.size() <= 0)
		{
			g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext InvalidID(_RandomQuestID) [ScheduleID:%d \r\n", nScheuldeID);
			return false;
		}

		if(!RegisterBaseQuest(nItemID, nType, vQuestIDs, vCompleteDate, nScheuldeID, nSelectCount))
			return false;
	}

	return true;
}

void CDNPeriodQuestSystem::BuildCompletedPeriodQuests(CDNUserQuest* pQuest)
{
	for each (int questID in m_PeriodQuestIDs)
	{
		if (pQuest->IsClearQuest(questID))
			pQuest->AddCompletedPeriodQuest(questID);
	}
}

void CDNPeriodQuestSystem::GetResetQuestList(CDNUserQuest* pQuest, int nType, std::vector<int>& vOutQuestList)
{
	if(!IsValidType(nType))
		return;

	for each(int questID in m_vQuestIDs[nType])
	{
		if(pQuest->HasQuest(questID) != -2)
		{
			vOutQuestList.push_back(questID);
			continue;
		}
	
		if(pQuest->IsCompletedPeriodQuest(questID))
		{
			vOutQuestList.push_back(questID);
			pQuest->RemoveCompletedPeriodQuest (questID);
			continue;
		}
	}
}

// Mark all quests except the selected one.
void CDNPeriodQuestSystem::MarkingSelectQuest(CDNUserQuest* pQuest)
{	
	RefreshQuestKey(PERIODQUEST_RESET_MAX);

	for each(const CDNPeriodQuest& periodQuest in m_vBaseQuest)
	{
		int nType = periodQuest.GetType();
		int nSelectedID = periodQuest.SelectQuestID(m_nKeys[nType]);
		if(nSelectedID <= 0)
			continue;

		bool bCanAcceptDate = periodQuest.CanAcceptDate();
		bool bActivate = periodQuest.GetActivate();
		const std::vector<int>& periodQuestIDs = periodQuest.GetQuestIDs();

		for( UINT i = 0 ; i < periodQuestIDs.size() ; ++ i )
		{
			int questID = periodQuestIDs[i];

			if( bCanAcceptDate && bActivate && periodQuest.CheckSelectedQuestID( m_nKeys[nType], questID, i ) )
				continue;

			if(nType == PERIODQUEST_RESET_EVENT || nType == PERIODQUEST_RESET_WORLD)
				pQuest->OnPeriodEvent(questID, true);
			else
				pQuest->SetCompleteQuestFlag(questID, true);
		}
	}
}

// Mark event quest
void CDNPeriodQuestSystem::RefreshEventQuest(CDNUserQuest* pQuest, int nCount, const TCompleteEventQuest* pEventQuest)
{	
	RefreshQuestKey(PERIODQUEST_RESET_MAX);

	for each(const CDNPeriodQuest& periodQuest in m_vBaseQuest)
	{
		int nType = periodQuest.GetType();
		int nSelectedID = periodQuest.SelectQuestID(m_nKeys[nType]);
		if(nSelectedID <= 0)
			continue;

		if(nType == PERIODQUEST_RESET_EVENT || nType == PERIODQUEST_RESET_WORLD)
		{
			// Pass
		}
		else 
			continue;

		// Mark if quest is completed
		bool bComplete = false;
		for(int i=0; i<nCount; i++)
		{
			if(pEventQuest[i].nScheduleID == periodQuest.GetScheduleID())
			{
				bComplete = true;
				break;
			}
		}

		// Unmark quests that are currently in progress and not completed
		bool bMarking = false;
		if(!bComplete)
		{
			if(!periodQuest.CanAcceptDate())
				continue;
		}
		else
			bMarking = true;

		// Marking process
		const std::vector<int>& periodQuestIDs = periodQuest.GetQuestIDs();
		for each(int questID in periodQuestIDs)
		{
			pQuest->OnPeriodEvent(questID, bMarking);
		}
	}
}

// Mark daily/weekly quests.
void CDNPeriodQuestSystem::RefreshPeriodQuest(int nType, CDNUserQuest* pQuest)
{
	// Check type
	if(!IsPeriodType(nType))
		return;

	RefreshQuestKey(nType);

	for each(const CDNPeriodQuest& periodQuest in m_vBaseQuest)
	{
		if(!periodQuest.IsType(nType))
			continue;

		int nSelectedID = periodQuest.SelectQuestID(m_nKeys[nType]);
		if(nSelectedID <= 0)
			continue;

		bool bActivate = periodQuest.GetActivate();
		bool bCanAcceptDate = periodQuest.CanAcceptDate();
		const std::vector<int>& periodQuestIDs = periodQuest.GetQuestIDs();

		for( UINT i = 0 ; i < periodQuestIDs.size() ; ++ i )
		{
			int questID = periodQuestIDs[i];

			//Mark daily/weekly quests.
			if( bActivate && bCanAcceptDate && periodQuest.CheckSelectedQuestID( m_nKeys[nType], questID, i ) )
				pQuest->SetCompleteQuestFlag(questID, false); // Unmark the selected quest.
			else
				pQuest->SetCompleteQuestFlag(questID, true);  // Mark unselected quests.
		}
	}
}

bool CDNPeriodQuestSystem::FindEventScheduleID(int nQuestID, int& nScheduleID, __time64_t& tExpireDate)
{
	bool bActivate;
	bool bCanAcceptDate;;
	std::vector<int> vPeriodQuestIDs;

	std::vector<CDNPeriodQuest>::iterator iter = m_vBaseQuest.begin();
	for( ; iter != m_vBaseQuest.end(); iter++ )
	{
		CDNPeriodQuest * pPeriodQuest = &(*iter);

		if( !pPeriodQuest)
			continue;

		int nType = pPeriodQuest->GetType();
		if(nType == PERIODQUEST_RESET_EVENT || nType == PERIODQUEST_RESET_WORLD)
		{

		}
		else
			continue;

		bActivate = pPeriodQuest->GetActivate();
		bCanAcceptDate = pPeriodQuest->CanAcceptDate();

		if(!bActivate || !bCanAcceptDate)
			continue;

		vPeriodQuestIDs.clear();
		vPeriodQuestIDs = pPeriodQuest->GetQuestIDs();

		std::vector<int>::iterator marking_iter = vPeriodQuestIDs.begin();
		for( ; marking_iter != vPeriodQuestIDs.end(); marking_iter++ )
		{
			if(nQuestID == (*marking_iter))
			{
				nScheduleID = pPeriodQuest->GetScheduleID();
				tExpireDate = pPeriodQuest->GetEventTime(PERIODQUEST_TIME_END);
				return true;
			}
		}
	}

	return false;
}

void CDNPeriodQuestSystem::DoUpdate(DWORD CurTick)
{
	if(PERIODQUESTTERM > GetTickTerm(m_TickCheckQuest, CurTick))
		return;

	m_TickCheckQuest = CurTick;

	time(&m_Time);

	std::vector<CDNPeriodQuest>::iterator iter = m_vBaseQuest.begin();
	while(iter != m_vBaseQuest.end())
	{
		CDNPeriodQuest * pQuest = &(*iter);

		if(pQuest->GetActivate())
		{
			if(pQuest->GetType() == PERIODQUEST_RESET_EVENT || pQuest->GetType() == PERIODQUEST_RESET_WORLD)
			{
				if(pQuest->GetEventTime(PERIODQUEST_TIME_START) < m_Time && !pQuest->GetEventComplete(PERIODQUEST_TIME_START))
					OnStartEvent(pQuest);
				else if(pQuest->GetEventTime(PERIODQUEST_TIME_END) < m_Time && !pQuest->GetEventComplete(PERIODQUEST_TIME_END))
					OnEndEvent(pQuest);
			}
		}

		++iter;
	}
}

bool CDNPeriodQuestSystem::CheckValidTime(time_t timeStart, time_t timeEnd)
{
	// Check if it hits reset timing before setting.
	CTimeSet tStartDate;
	CTimeSet tEndDate;

	tStartDate.Set(timeStart, true);
	tEndDate.Set(timeEnd, true);

	if(!tStartDate.CheckIntegrity())
		return false;

	if(!tEndDate.CheckIntegrity())
		return false;

	// Disallow scheduling during reset timing because Key value changes.
	if(tStartDate.GetHour() < m_nResetHour && m_nResetHour < tEndDate.GetHour())
		return false;

	return true;
}

bool CDNPeriodQuestSystem::SetActivateQuest(int nScheduleID, bool bFlag, time_t timeStart, time_t timeEnd)
{
	if(!CheckValidTime(timeStart, timeEnd))
		return false;

	time(&m_Time);

	std::vector<CDNPeriodQuest>::iterator iter = m_vBaseQuest.begin();
	for( ; iter != m_vBaseQuest.end(); iter++ )
	{
		CDNPeriodQuest * pQuest = &(*iter);

		if(pQuest->GetScheduleID() == nScheduleID)
		{
			if(pQuest->GetType() == PERIODQUEST_RESET_EVENT || pQuest->GetType() == PERIODQUEST_RESET_WORLD)
			{
				// Pass
			}
			else
				return false;

			if(pQuest->GetOnEvent() && false == bFlag)
				OnEndEvent(pQuest);

			pQuest->SetEventTime(PERIODQUEST_TIME_START, timeStart);
			pQuest->SetEventTime(PERIODQUEST_TIME_END, timeEnd);

			pQuest->SetEventComplete(PERIODQUEST_TIME_START, false);
			pQuest->SetEventComplete(PERIODQUEST_TIME_END, false);

			pQuest->SetOnEvent(false);
			pQuest->SetActivate(bFlag);

			return true;
		}	
	}

	return false;
}

void CDNPeriodQuestSystem::LoadWorldQuestInfo(char cWorldSetID)
{
	if(m_bInitFlag)
		return;

	m_bInitFlag = true;

	int nScheduleID = 0;

	BYTE cThreadID;
	CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
	if(!pDBCon)
	{
		_DANGER_POINT();
		return;
	}

	std::vector<CDNPeriodQuest>::iterator iter = m_vBaseQuest.begin();
	while(iter != m_vBaseQuest.end())
	{
		CDNPeriodQuest * pQuest = &(*iter);

		if(pQuest->GetType() == PERIODQUEST_RESET_WORLD)
		{
			nScheduleID = pQuest->GetScheduleID();

			// Call DB
			 pDBCon->QueryGetWorldEventQuestCounter(cThreadID, cWorldSetID, nScheduleID);
		}
		
		++iter;
	}
}

void CDNPeriodQuestSystem::UpdateWorldEvent(char cWorldSetID, int nScheduleID, int nCount, bool bCheckNotice)
{
	TGlobalEventData* pEvent = g_pDataManager->GetGlobalEventData(nScheduleID);
	if(!pEvent) 
		return;

	// If a value is stored in DB, activate bCheckNotice up to the count threshold.
	if(bCheckNotice)
	{
		for(int i=0; i<MAX_NOTICE_WORLDEVENT; i++)
		{
			if(nCount >= pEvent->nNoticeCount[i])
				pEvent->bCheckNotice[i] = true;
		}
	}

	// Handle notice
	for(int i=0; i<MAX_NOTICE_WORLDEVENT; i++)
	{
		if(nCount >= pEvent->nNoticeCount[i] && 
			false == pEvent->bCheckNotice[i] &&
			pEvent->nNoticeCount[i] > 0)
		{
			pEvent->bCheckNotice[i] = true;

			// Send message to all users
#if defined(_VILLAGESERVER)
			g_pUserSessionManager->SendPeriodQuestNotice(pEvent->nItemID, pEvent->nNoticeCount[i]);
#elif defined(_GAMESERVER)
			SendNoticePeriodQuestForGameServer(cWorldSetID, pEvent->nItemID, pEvent->nNoticeCount[i]);
#endif
		}
	}

	TWorldQuestInfo Data;
	Data.cWorldSetID = cWorldSetID;
	Data.nScheduleID = nScheduleID;
	Data.nCount = nCount;

	MWorldQuestItor itor = m_mWorldEventQuest.find(cWorldSetID);
	if(itor != m_mWorldEventQuest.end())
	{
		bool bIsUpdate = false;
		std::vector<TWorldQuestInfo>::iterator iter = (*itor).second.begin();
		while(iter != (*itor).second.end())
		{
			TWorldQuestInfo * pQuest = &(*iter);

			if(pQuest->nScheduleID == nScheduleID)
			{
				pQuest->nCount = nCount;
				bIsUpdate = true;
				break;
			}
			++iter;
		}

		if(false == bIsUpdate)
			(*itor).second.push_back(Data);

	}
	else
	{
		std::vector <TWorldQuestInfo> vList;
		vList.push_back(Data);

		m_mWorldEventQuest.insert(make_pair(cWorldSetID, vList));
	}

}

int CDNPeriodQuestSystem::GetWorldEventCount(char cWorldSetID, int nScheduleID)
{
	MWorldQuestItor itor = m_mWorldEventQuest.find(cWorldSetID);
	if(itor != m_mWorldEventQuest.end())
	{
		std::vector<TWorldQuestInfo>::iterator iter = (*itor).second.begin();
		while(iter != (*itor).second.end())
		{
			TWorldQuestInfo * pQuest = &(*iter);

			if(pQuest->nScheduleID == nScheduleID)
				return pQuest->nCount;
			
			++iter;
		}
	}

	return 0;
}

// 0: Completed, 1: Start notice, 2: Collection period, 3: Reward notice, 4: Reward period
int CDNPeriodQuestSystem::GetWorldEventStep(char cWorldSetID, int nScheduleID)
{
	time_t tStartDate = 0, tEndDate = 0;

	std::vector<CDNPeriodQuest>::iterator iter = m_vBaseQuest.begin();
	for( ; iter != m_vBaseQuest.end(); iter++ )
	{
		CDNPeriodQuest * pQuest = &(*iter);
		if(pQuest->GetScheduleID() == nScheduleID)
		{
			tStartDate = pQuest->GetEventTime(PERIODQUEST_TIME_START);
			tEndDate = pQuest->GetEventTime(PERIODQUEST_TIME_END);
			break;
		}
	}

	if(tStartDate == 0 || tEndDate == 0)
		return 0;		

	TGlobalEventData* pEvent = g_pDataManager->GetGlobalEventData(nScheduleID);
	if(!pEvent)
		return 0;

	time(&m_Time);

	// Start notice
	if(tStartDate < m_Time && m_Time < pEvent->tCollectStartDate)
		return 1;

	// Collection period
	if(pEvent->tCollectStartDate < m_Time && m_Time < pEvent->tCollectEndDate)
		return 2;

	// Reward notice
	if(pEvent->tCollectEndDate < m_Time && m_Time < pEvent->tRewardDate)
		return 3;

	// Reward period
	if(pEvent->tRewardDate <m_Time && m_Time < tEndDate)
		return 4;

	return 0;
}

bool CDNPeriodQuestSystem::SetWorldEventTime(int nScheduleID, time_t tCollectStartDate, time_t tCollectEndDate, time_t tRewardDate)
{
	TGlobalEventData* pEvent = g_pDataManager->GetGlobalEventData(nScheduleID);
	if(!pEvent)
		return false;

	pEvent->tCollectStartDate = tCollectStartDate;
	pEvent->tCollectEndDate = tCollectEndDate;
	pEvent->tRewardDate = tRewardDate;

	return true;
}

bool CDNPeriodQuestSystem::IsPeriodQuest(int questID) const
{
	return (m_PeriodQuestIDs.find(questID) != m_PeriodQuestIDs.end()) ? true : false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register default quest.
bool CDNPeriodQuestSystem::RegisterBaseQuest(int nItemID, int nType, std::vector<string>& vQuestIDs, std::vector<string>& vCompleteDate, int nScheduleID, int nSelectCount)
{
	time_t timeStart = 0, timeEnd = 0;

	if(vCompleteDate.size() == PERIODQUEST_TIME_MAX)
	{
		CTimeSet tStartSet(vCompleteDate[PERIODQUEST_TIME_START].c_str(), true);
		CTimeSet tEndSet(vCompleteDate[PERIODQUEST_TIME_END].c_str(), true);

		timeStart = tStartSet.GetTimeT64_LC();
		timeEnd = tEndSet.GetTimeT64_LC();

		if(!CheckValidTime(timeStart, timeEnd))
		{
			g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext CheckValidTime Fail [ItemID:%d]\r\n", nItemID);
			return false;
		}

		// Check world event time
		if(nType == PERIODQUEST_RESET_WORLD)
		{
			TGlobalEventData* pEvent = g_pDataManager->GetGlobalEventData(nScheduleID);
			if(!pEvent) 
			{
				g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext can't find valid ScheduleID in GlobalEvent.ext [ScheduleID:%d][ItemID:%d] \r\n", nScheduleID, nItemID);
				return false;
			}

			if(timeStart < pEvent->tCollectStartDate || timeStart < pEvent->tCollectEndDate || timeStart < pEvent->tRewardDate)
			{

			}
			else
			{
				g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext WorldEvent StartTime Sequence Invalid [ScheduleID:%d][ItemID:%d] \r\n", nScheduleID, nItemID);
				return false;
			}

			if(pEvent->tCollectStartDate < timeEnd || pEvent->tCollectEndDate < timeEnd || pEvent->tRewardDate < timeEnd)
			{

			}
			else
			{
				g_Log.Log(LogType::_FILEDBLOG, L"questdailytable.ext WorldEvent EndTime Sequence Invalid [ScheduleID:%d][ItemID:%d] \r\n", nScheduleID, nItemID);
				return false;
			}
		}

	}
		
	CDNPeriodQuest PeriodQuest;
	PeriodQuest.Set(nItemID, nType, timeStart, timeEnd, nScheduleID, nSelectCount);

	int nQuestID = 0;
	for( std::vector<string>::iterator iter = vQuestIDs.begin(); iter != vQuestIDs.end() ; iter++ )
	{
		nQuestID = atoi((*iter).c_str() );

		PeriodQuest.AddQuestID(nQuestID);
			
		m_vQuestIDs[nType].push_back(nQuestID);
		m_PeriodQuestIDs.insert(nQuestID);
	}

	m_vBaseQuest.push_back(PeriodQuest);

	return true;
}

// Refresh quest key value
void CDNPeriodQuestSystem::RefreshQuestKey(int nPeriodQuestType)
{	
	time(&m_Time);
	m_pTime = localtime(&m_Time);

	int nKey = 0;

	// Daily
	if(nPeriodQuestType == PERIODQUEST_RESET_DAILY)
	{
		nKey = GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, 0 );
		int nValue = (( m_pTime->tm_mday * 24 ) +( m_pTime->tm_hour - m_nResetHour ) ) / 24;
		nKey += nValue;
		m_nKeys[PERIODQUEST_RESET_DAILY] = nKey;
	}
	else if(nPeriodQuestType == PERIODQUEST_RESET_WEEKLY) // Weekly
	{
		int nWeek = CDNSchedule::Saturday - CDNSchedule::Sunday;
		nKey = ( GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, 0 ) - 4 - nWeek ) / 7;
		int nValue = GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, m_pTime->tm_mday ) - 4 - nWeek;
		int nOffset = nValue % 7;
		int nWeekCount = ( 7 +( m_pTime->tm_mday - nOffset ) +( nOffset / 7 ) ) / 7;
		if( nOffset == 0 && m_pTime->tm_hour < m_nResetHour) nWeekCount--;
		nKey += nWeekCount;

		m_nKeys[PERIODQUEST_RESET_WEEKLY] = nKey;
	}
	else // All
	{
		nKey = GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, 0 );
		int nValue = (( m_pTime->tm_mday * 24 ) +( m_pTime->tm_hour - m_nResetHour ) ) / 24;
		nKey += nValue;
		m_nKeys[PERIODQUEST_RESET_DAILY] = nKey;

		int nWeek = CDNSchedule::Saturday - CDNSchedule::Sunday;
		nKey = ( GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, 0 ) - 4 - nWeek ) / 7;
		nValue = GetDateValue( 1900 + m_pTime->tm_year, m_pTime->tm_mon + 1, m_pTime->tm_mday ) - 4 - nWeek;
		int nOffset = nValue % 7;
		int nWeekCount = ( 7 +( m_pTime->tm_mday - nOffset ) +( nOffset / 7 ) ) / 7;
		if( nOffset == 0 && m_pTime->tm_hour < m_nResetHour) nWeekCount--;
		nKey += nWeekCount;
		m_nKeys[PERIODQUEST_RESET_WEEKLY] = nKey;
	}
}

// Type validity check
bool CDNPeriodQuestSystem::IsValidType(int nType) const
{
	switch(nType)
	{
	case PERIODQUEST_RESET_EVENT:
	case PERIODQUEST_RESET_DAILY:
	case PERIODQUEST_RESET_WORLD:
	case PERIODQUEST_RESET_WEEKLY:
		return true;
	}
	
	return false;
}

bool CDNPeriodQuestSystem::IsPeriodType(int nType) const
{
	switch(nType)
	{
	case PERIODQUEST_RESET_DAILY:
	case PERIODQUEST_RESET_WEEKLY:
		return true;
	}

	return false;
}

// Handle quest start point
void CDNPeriodQuestSystem::OnStartEvent(CDNPeriodQuest* pQuest)
{
	pQuest->SetEventComplete(PERIODQUEST_TIME_START, true);
	pQuest->SetOnEvent(true);

#if defined(_VILLAGESERVER)
	for( UINT i = 0 ; i < pQuest->GetQuestIDs().size() ; ++ i )
	{
		int nSelectedID = pQuest->GetQuestIDs()[i];
		if( pQuest->CheckSelectedQuestID( m_nKeys[PERIODQUEST_RESET_DAILY], nSelectedID, i ) )
		{
			g_pUserSessionManager->SendAssginPeriodQuest( nSelectedID, false );
		}
	}
#elif defined(_GAMESERVER)
	for( UINT i = 0 ; i < pQuest->GetQuestIDs().size() ; ++ i )
	{
		int nSelectedID = pQuest->GetQuestIDs()[i];
		if( pQuest->CheckSelectedQuestID( m_nKeys[PERIODQUEST_RESET_DAILY], nSelectedID, i ) )
		{
			SendAssginPeriodQuestForGameServer(nSelectedID, false);
		}
	}
#endif
}

// Handle quest end point
void CDNPeriodQuestSystem::OnEndEvent(CDNPeriodQuest* pQuest)
{
	pQuest->SetEventComplete(PERIODQUEST_TIME_END, true);
	pQuest->SetOnEvent(false);

#if defined(_VILLAGESERVER)
	for( UINT i = 0 ; i < pQuest->GetQuestIDs().size() ; ++ i )
	{
		int nSelectedID = pQuest->GetQuestIDs()[i];
		if( pQuest->CheckSelectedQuestID( m_nKeys[PERIODQUEST_RESET_DAILY], nSelectedID, i ) )
		{
			g_pUserSessionManager->SendAssginPeriodQuest(nSelectedID, true);
		}
	}
#elif defined(_GAMESERVER)
	for( UINT i = 0 ; i < pQuest->GetQuestIDs().size() ; ++ i )
	{
		int nSelectedID = pQuest->GetQuestIDs()[i];
		if( pQuest->CheckSelectedQuestID( m_nKeys[PERIODQUEST_RESET_DAILY], nSelectedID, i ) )
		{
			SendAssginPeriodQuestForGameServer(nSelectedID, true);
		}
	}
#endif

	pQuest->SetActivate(false);
}

#if defined(_GAMESERVER)
void CDNPeriodQuestSystem::SendAssginPeriodQuestForGameServer(int nQuestID, bool bFlag)
{
	MAGAAssginPeriodQuest packet;
	packet.nQuestID = nQuestID;
	packet.bFlag = bFlag;

	for(int i=0 ; i<g_pGameServerManager->GetGameServerSize(); i++) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(i);
		if(pServer) 
			pServer->StoreExternalBuffer(0, MAGA_ASSIGN_PERIODQUEST, 0, reinterpret_cast<char*>(&packet), sizeof(packet), EXTERNALTYPE_MASTER);
	}
}

void CDNPeriodQuestSystem::SendNoticePeriodQuestForGameServer(char cWorldSetID, int nItemID, int nNoticeCount)
{
	MAGANoticePeriodQuest packet;
	packet.cWorldSetID = cWorldSetID;
	packet.nItemID = nItemID;
	packet.nNoticeCount = nNoticeCount;
	
	for(int i=0 ; i<g_pGameServerManager->GetGameServerSize(); i++) 
	{
		CDNRUDPGameServer *pServer = g_pGameServerManager->GetGameServerIndex(i);
		if(pServer) 
			pServer->StoreExternalBuffer(0, MAGA_NOTICE_PERIODQUEST, 0, reinterpret_cast<char*>(&packet), sizeof(packet), EXTERNALTYPE_MASTER);
	}
}

#endif