#include "stdafx.h"
#include "DNMissionSystem.h"
#include "DNMissionCommonAPI.h"
#include "DNUserData.h"
#include "DnTableDB.h"
#include "DNGameDataManager.h"
#include "EtUIXML.h"
#include "DNUserSendManager.h"
#include "DNDBConnectionManager.h"
#include "DNLogConnection.h"
#include "DNAppellation.h"
#include "DNMissionScheduler.h"
#include "DNUserSession.h"
#include "DNDBConnection.h"
#include "DNMailSender.h"
#if defined( _VILLAGESERVER )
#include "DNMasterConnection.h"
#else
#include "DNMasterConnectionManager.h"
#endif
#include "DNGuildSystem.h"
#if defined(_VILLAGESERVER)
#include "DNGuildWarManager.h"
#endif // #if defined(_VILLAGESERVER)

#if defined( PRE_ADD_STAMPSYSTEM )
#include "DNStampSystem.h"
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

/*
CDNMissionSystem::MissionInfoStruct CDNMissionSystem::s_MissionInfoStruct[] = {
	{ CDNMissionSystem::AttainPlayerLevel, 2, api_AttainPlayerLevel },
};
*/

CDNMissionSystem::CDNMissionSystem( CDNUserSession *pUser ) 
#if defined(PRE_ADD_ACTIVEMISSION)
: m_ActiveMissionState(ActiveMission::NOT_ASSIGNED)
,m_pActiveMission(NULL)
#endif
{
	m_pUserSession = pUser;
}

CDNMissionSystem::~CDNMissionSystem()
{
}

bool CDNMissionSystem::LoadUserData()
{
	RefreshOnOffMissionList();
	RefreshDailyMissionList();
	RefreshWeeklyMissionList();
#if defined(PRE_ADD_MONTHLY_MISSION)
	RefreshMonthlyMissionList();
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	RefreshWeekendEventMissionList();
	RefreshGuildWarMissionList();
	RefreshPCBangMissionList();
	RefreshGuildCommonMissionList();
	RefreshWeekendRepeatMissionList();

	return true;
}

void CDNMissionSystem::RefreshOnOffMissionList()
{
	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecMissionGainList[i] );
		SAFE_DELETE_VEC( m_nVecMissionAchieveList[i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	// Set On/Off mission.
	bool bModifyGain = false;
	for( int i=0; i<MISSIONMAX; i++ ) {
#if defined(PRE_MOD_SET_ACHIEVED_MISSION_GAINFLAG)
		TMissionData *pData = g_pDataManager->GetMissionData(i);
		if( !pData ) continue;
		
		if( GetBitFlag( pMission->MissionAchieve, i ) ) 
		{	//Process Gain for achievements marked as achieved in DB but not yet gained
			if(!GetBitFlag(pMission->MissionGain, i))
			{
				SetBitFlag(pMission->MissionGain, i, true);
				bModifyGain = true;
			}
			continue;	//Discard completed ones.
		}
#else
		// Discard if already achieved.
		if( GetBitFlag( pMission->MissionAchieve, i ) ) continue;

		TMissionData *pData = g_pDataManager->GetMissionData(i);
		if( !pData ) continue;
#endif

		// Discard if not Active in the table.
		if( !pData->bActivate )
		{
			// When changing an already-acquired client mission to Active-False due to various reasons
			// Must not be visible to client either, so force-modify Gain (without touching DB-recorded value).
			if( GetBitFlag( pMission->MissionGain, i ) == true )
				SetBitFlag( pMission->MissionGain, i, false );
			continue;
		}


		// If there is no acquisition condition, turn on the acquisition flag in DB. Turn it on at creation.
		if( pData->GainCondition.cEvent == 0 ) {
			if( GetBitFlag( pMission->MissionGain, i ) == false ) {
				bModifyGain = true;
				SetBitFlag( pMission->MissionGain, i, true );
			}
		}

		// Completion conditions cannot be absent. If absent, it is not configured, so skip.
		if( pData->AchieveCondition.cEvent == 0 ) continue;
		if( GetBitFlag( pMission->MissionGain, i ) == false )
			m_nVecMissionGainList[pData->GainCondition.cEvent].push_back(i);
		else m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].push_back(i);
	}

	m_pUserSession->GetDBConnection()->QueryMissionGain( m_pUserSession );
}

void CDNMissionSystem::RefreshDailyMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[Daily][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<DAILYMISSIONMAX; i++ ) {
		if( pMission->DailyMission[i].nMissionID < 1 ) continue;
		if( pMission->DailyMission[i].bAchieve ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->DailyMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[Daily][pData->AchieveCondition.cEvent].push_back(i);
	}
}

void CDNMissionSystem::RefreshWeeklyMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[Weekly][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<WEEKLYMISSIONMAX; i++ ) {
		if( pMission->WeeklyMission[i].nMissionID < 1 ) continue;
		if( pMission->WeeklyMission[i].bAchieve ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->WeeklyMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[Weekly][pData->AchieveCondition.cEvent].push_back(i);
	}
}
#if defined( PRE_ADD_MONTHLY_MISSION )
void CDNMissionSystem::RefreshMonthlyMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[Monthly][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<MONTHLYMISSIONMAX; i++ ) {
		if( pMission->MonthlyMission[i].nMissionID < 1 ) continue;
		if( pMission->MonthlyMission[i].bAchieve ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->MonthlyMission[i].nMissionID );
		if( !pData ) continue;

		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[Monthly][pData->AchieveCondition.cEvent].push_back(i);
	}
}
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )

void CDNMissionSystem::RefreshWeekendEventMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[WeekendEvent][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<WEEKENDEVENTMISSIONMAX; i++ ) {
		if( pMission->WeekendEventMission[i].nMissionID < 1 ) continue;
		if( pMission->WeekendEventMission[i].bAchieve ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->WeekendEventMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[WeekendEvent][pData->AchieveCondition.cEvent].push_back(i);
	}
}

// Update guild mission
void CDNMissionSystem::RefreshGuildWarMissionList()
{
	// Reset if no initial data exists
	bool bCreateMission = true;
	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<GUILDWARMISSIONMAX; i++ )
	{
		if( pMission->GuildWarMission[i].nMissionID > 0 ) 
		{
			bCreateMission = false;
			break;
		}
	}

	if(bCreateMission) 
	{
		if(InitGuildWarMissionList(false))
			return;
	}

	// Perform reset in advance if not in preliminaries.
	if(!bIsGuildWarMission()) 
	{
		if(InitGuildWarMissionList(true))
			return;
	}
		
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[GuildWar][i] );
	}

	for( int i=0; i<GUILDWARMISSIONMAX; i++ )
	{
		if( pMission->GuildWarMission[i].nMissionID < 1 ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->GuildWarMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[GuildWar][pData->AchieveCondition.cEvent].push_back(i);
	}
}

void CDNMissionSystem::RefreshPCBangMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[PCBang][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for (int i = 0; i < PCBangMissionMax; i++){
		if( pMission->PCBangMission[i].nMissionID < 1 ) continue;
		if( pMission->PCBangMission[i].bAchieve ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->PCBangMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount ) continue;

		m_nVecDailyMissionList[PCBang][pData->AchieveCondition.cEvent].push_back(i);
	}
}

void CDNMissionSystem::RefreshGuildCommonMissionList()
{
	//Guild missions are not deleted upon Achieve and can be cleared repeatedly
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[GuildCommon][i] );
	}

	bool bAddedNewGuildMission = false;
	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<GUILDCOMMONMISSIONMAX; i++ ) {
		TDailyMissionData *pData = g_pDataManager->GetGuildCommonMissionData( i );		
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		if( pMission->GuildCommonMission[i].nMissionID < 1 )
		{
			pMission->GuildCommonMission[i].nMissionID = pData->nItemID;
			bAddedNewGuildMission = true;
		}
		m_nVecDailyMissionList[GuildCommon][pData->AchieveCondition.cEvent].push_back(i);
	}

	if(bAddedNewGuildMission)
		m_pUserSession->GetDBConnection()->QuerySetGuildCommonMission(m_pUserSession);
}

void CDNMissionSystem::RefreshWeekendRepeatMissionList()
{
	ScopeLock<CSyncLock> Lock(m_Lock);

	for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) {
		SAFE_DELETE_VEC( m_nVecDailyMissionList[WeekendRepeat][i] );
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=0; i<WEEKENDREPEATMISSIONMAX; i++ )
	{
		if( pMission->WeekendRepeatMission[i].nMissionID < 1 ) continue;

		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pMission->WeekendRepeatMission[i].nMissionID );
		if( !pData ) continue;
		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

		m_nVecDailyMissionList[WeekendRepeat][pData->AchieveCondition.cEvent].push_back(i);
	}
}

void CDNMissionSystem::OnGainMission( int nArrayIndex )
{
	m_pUserSession->SendMissionGain( nArrayIndex );
}

void CDNMissionSystem::OnAchieveMission( int nArrayIndex )
{
	m_pUserSession->SendMissionAchieve( nArrayIndex );
	m_pUserSession->GetDBConnection()->QueryMissionAchieve(m_pUserSession, nArrayIndex);
}

void CDNMissionSystem::OnCounterDailyMission( DailyMissionTypeEnum Type, int nArrayIndex, int nCounter )
{
	m_pUserSession->SendDailyMissionCount( Type, nArrayIndex, nCounter );
}

void CDNMissionSystem::OnAchieveDailyMission( DailyMissionTypeEnum Type, int nArrayIndex, bool bSecret /*=false*/ )
{
	m_pUserSession->SendDailyMissionAchieve( Type, nArrayIndex, bSecret );

#if defined( PRE_ADD_STAMPSYSTEM )
	m_pUserSession->GetStampSystem()->OnCompleteDailyMission( nArrayIndex );
#endif // #if defined( PRE_ADD_STAMPSYSTEM )
}

void CDNMissionSystem::OnEventPopUp( int nArrayIndex )
{
	// Event popup flag (included in AchieveList) is saved to DB via OnAchieveMission
	m_pUserSession->SendMissionEventPopUp( nArrayIndex );
}

void CDNMissionSystem::OnHelpAlarm( int nArrayIndex )
{
	m_pUserSession->SendMissionHelpAlarm( nArrayIndex );
}

void CDNMissionSystem::RequestMissionReward( int nArrayIndex )
{
	TMissionData *pData = g_pDataManager->GetMissionData(nArrayIndex);
	if( !pData ) return;

	if(pData->nMailID > 0){
		char cPayMethod = 0;
		switch(pData->nType)
		{
		case MissionType_TimeEvent:
			cPayMethod = DBDNWorldDef::PayMethodCode::Event;
			break;

		default:
			cPayMethod = DBDNWorldDef::PayMethodCode::Mission;
			break;
		}

		CDNMailSender::Process(m_pUserSession, pData->nMailID, cPayMethod, nArrayIndex);
	}
	else
	{
#ifdef PRE_ADD_JOINGUILD_SUPPORT
		//Ugh... damn hardcoding...
		if (nArrayIndex == 43)
		{
			int nMailID = pData->nMailID;
			char cPayMethod = 0;

			switch(pData->nType)
			{
			case MissionType_TimeEvent:
				cPayMethod = DBDNWorldDef::PayMethodCode::Event;
				break;

			default:
				cPayMethod = DBDNWorldDef::PayMethodCode::Mission;
				break;
			}

			if (m_pUserSession)
			{
				int nRewardMailID = g_pDataManager->GetRewardFisrtGuildJoinMailID(m_pUserSession->GetLevel());
				if (nRewardMailID > 0)
					CDNMailSender::Process(m_pUserSession, nRewardMailID, cPayMethod, nArrayIndex);
				else
					_DANGER_POINT();
			}
		}
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
	}

	if( pData->nRewardAppellation != -1 ) {
		m_pUserSession->GetAppellation()->RequestAddAppellation( pData->nRewardAppellation );
	}
	if( pData->bIsAchieveNotice ) {
#if defined(_VILLAGESERVER)
		g_pMasterConnection->SendWorldSystemMsg( m_pUserSession->GetAccountDBID(), WORLDCHATTYPE_MISSION, nArrayIndex, 0 );
#else
		g_pMasterConnectionManager->SendWorldSystemMsg( m_pUserSession->GetWorldSetID(), m_pUserSession->GetAccountDBID(), WORLDCHATTYPE_MISSION, nArrayIndex, 0 );
#endif
	}
}

void CDNMissionSystem::RequestDailyMissionReward( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if(pData->nType != DAILYMISSION_PCBANG)
		pMission->nMissionScore += pData->nRewardPoint;

	if(pData->nMailID > 0){
		CDNMailSender::Process(m_pUserSession, pData->nMailID, DBDNWorldDef::PayMethodCode::Mission, nItemID);
	}

	if( pData->nRewardAppellation != -1 ) {
		m_pUserSession->GetAppellation()->RequestAddAppellation( pData->nRewardAppellation );
	}
}

void CDNMissionSystem::UpdateLastAchieveList( int nArrayIndex )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( int i=LASTMISSIONACHIEVEMAX - 1; i>0; i-- ) {
		pMission->wLastMissionAchieve[i] = pMission->wLastMissionAchieve[i-1];
	}
	pMission->wLastMissionAchieve[0] = nArrayIndex;

}

void CDNMissionSystem::RequestDailyMissionList( DailyMissionTypeEnum Type, int nRandomSeed, bool bSend )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	switch( Type ) {
		case Daily:
			{
				CDNMissionScheduler::GetInstance().RefreshDailyMissionList( nRandomSeed, m_pUserSession->GetMissionData(), m_pUserSession->GetLevel(), m_pUserSession->GetClassID() );

				RefreshDailyMissionList();
				if( bSend ) m_pUserSession->SendDailyMissionList( Daily, pMission->DailyMission );
				m_pUserSession->GetDBConnection()->QuerySetDailyMission( m_pUserSession );

				// Include PC bang mission here too.
				m_pUserSession->RefreshPCBangMissionList();	// Set missionid here.

				RefreshPCBangMissionList();
				m_pUserSession->GetDBConnection()->QuerySetPCBangMission(m_pUserSession);
			}
			break;

		case Weekly:
			{
				CDNMissionScheduler::GetInstance().RefreshWeeklyMissionList( nRandomSeed, m_pUserSession->GetMissionData(), m_pUserSession->GetLevel(), m_pUserSession->GetClassID() );

				RefreshWeeklyMissionList();
				if( bSend ) m_pUserSession->SendDailyMissionList( Weekly, pMission->WeeklyMission );
				m_pUserSession->GetDBConnection()->QuerySetWeeklyMission( m_pUserSession );
			}
			break;

#if defined(PRE_ADD_MONTHLY_MISSION)
		case Monthly:
			{
				CDNMissionScheduler::GetInstance().RefreshMonthlyMissionList( nRandomSeed, m_pUserSession->GetMissionData(), m_pUserSession->GetLevel(), m_pUserSession->GetClassID() );

				RefreshMonthlyMissionList();
				if( bSend ) m_pUserSession->SendDailyMissionList( Monthly, pMission->MonthlyMission );
				m_pUserSession->GetDBConnection()->QuerySetMonthlyMission( m_pUserSession );
			}
			break;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

		case WeekendEvent:
			{
				CDNMissionScheduler::GetInstance().RefreshWeekendEventMissionList( m_pUserSession->GetMissionData() );

				RefreshWeekendEventMissionList();
				if( bSend ) m_pUserSession->SendDailyMissionList( WeekendEvent, pMission->WeekendEventMission );
				m_pUserSession->GetDBConnection()->QuerySetWeekendEventMission( m_pUserSession );
			}
			break;

		case WeekendRepeat:
			{
				CDNMissionScheduler::GetInstance().RefreshWeekendRepeatMissionList( m_pUserSession->GetMissionData() );

				RefreshWeekendRepeatMissionList();
				if( bSend ) m_pUserSession->SendDailyMissionList( WeekendRepeat, pMission->WeekendRepeatMission );
				m_pUserSession->GetDBConnection()->QuerySetWeekendRepeatMission( m_pUserSession );
			}
			break;
	}
}

void CDNMissionSystem::RequestDailyMissionAlarm( DailyMissionTypeEnum Type )
{
	m_pUserSession->SendDailyMissionAlarm( Type );
}

void CDNMissionSystem::RequestForceGainMission( int nItemID )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSION );
	int nTargetArrayIndex = pSox->GetArrayIndex( nItemID );
	TMissionData *pData = g_pDataManager->GetMissionData( nTargetArrayIndex );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	for( DWORD i=0; i<m_nVecMissionGainList[pData->GainCondition.cEvent].size(); i++ ) {
		int nArrayIndex = m_nVecMissionGainList[pData->GainCondition.cEvent][i];
		if( nArrayIndex == nTargetArrayIndex ) {
			SetBitFlag( pMission->MissionGain, nArrayIndex, true );
			OnGainMission( nArrayIndex );

			// If acquisition succeeds, add to the achieved list.
			m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].push_back(nArrayIndex);
			///////////////////////////////////////////////
			m_nVecMissionGainList[pData->GainCondition.cEvent].erase( m_nVecMissionGainList[pData->GainCondition.cEvent].begin() + i );
			i--;
		}
	}
}

void CDNMissionSystem::RequestForceAchieveMission( int nItemID )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSION );
	int nTargetArrayIndex = pSox->GetArrayIndex( nItemID );
	TMissionData *pData = g_pDataManager->GetMissionData( nTargetArrayIndex );
	if( !pData ) return;

	std::vector<int> nVecAchieveList;
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	for( DWORD i=0; i<m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].size(); i++ ) {
		int nArrayIndex = m_nVecMissionAchieveList[pData->AchieveCondition.cEvent][i];
		if( nArrayIndex == nTargetArrayIndex ) {
			SetBitFlag( pMission->MissionAchieve, nArrayIndex, true );
			RequestMissionReward( nArrayIndex );
			OnAchieveMission( nArrayIndex );
			if( pData->nType == MissionType_Normal || pData->nType == MissionType_MetaShow ) UpdateLastAchieveList( nArrayIndex );

			nVecAchieveList.push_back( nArrayIndex + 1 );
			m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].erase( m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].begin() + i );
			i--;
		}
	}

	for( DWORD i=0; i<nVecAchieveList.size(); i++ ) {
		m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnMissionClear, 1, EventSystem::MissionID, nVecAchieveList[i] );
	}
}

void CDNMissionSystem::RequestForceGainDailyMission( int nCount, ... )
{
	va_list list;
	va_start( list, nCount );
	std::vector<int> nVecList;
	for( int i=0; i<nCount; i++ ) {
		nVecList.push_back( va_arg( list, int ) );
		if( nVecList.size() == DAILYMISSIONMAX ) break;
	}
	va_end( list );

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	int nOffset = 0;
	memset( pMission->DailyMission, 0, sizeof(pMission->DailyMission) );

	for( DWORD i=0; i<nVecList.size(); i++ ) {
		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nVecList[i] );
		if( !pData ) continue;

		pMission->DailyMission[nOffset].nMissionID = nVecList[i];
		nOffset++;
	}
	RefreshDailyMissionList();

	m_pUserSession->SendDailyMissionList( Daily, pMission->DailyMission );
}

void CDNMissionSystem::RequestForceAchieveDailyMission( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	TDailyMission *pDailyMission;

	for( DWORD i=0; i<m_nVecDailyMissionList[Daily][pData->AchieveCondition.cEvent].size(); i++ ) {
		int nArrayIndex = m_nVecDailyMissionList[Daily][pData->AchieveCondition.cEvent][i];
		pDailyMission = &pMission->DailyMission[ nArrayIndex ];
		if( pDailyMission->nMissionID == nItemID ) {
			pDailyMission->bAchieve = true;
			pDailyMission->nCounter = pData->nCounterParam;
			RequestDailyMissionReward( pDailyMission->nMissionID );
			OnAchieveDailyMission( Daily, nArrayIndex );

			m_pUserSession->GetDBConnection()->QueryModDailyWeeklyMission( m_pUserSession, DAILYMISSION_DAY, nArrayIndex, pDailyMission);	// Save to DB

			m_nVecDailyMissionList[Daily][pData->AchieveCondition.cEvent].erase( m_nVecDailyMissionList[Daily][pData->AchieveCondition.cEvent].begin() + i );
			break;
		}
	}
}

void CDNMissionSystem::RequestForceGainWeeklyMission( int nCount, ... )
{
	va_list list;
	va_start( list, nCount );
	std::vector<int> nVecList;
	for( int i=0; i<nCount; i++ ) {
		nVecList.push_back( va_arg( list, int ) );
		if( nVecList.size() == WEEKLYMISSIONMAX ) break;
	}
	va_end( list );

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	int nOffset = 0;
	memset( pMission->WeeklyMission, 0, sizeof(pMission->WeeklyMission) );

	for( DWORD i=0; i<nVecList.size(); i++ ) {
		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nVecList[i] );
		if( !pData ) continue;

		pMission->WeeklyMission[nOffset].nMissionID = nVecList[i];
		nOffset++;
	}
	RefreshWeeklyMissionList();

	m_pUserSession->SendDailyMissionList( Weekly, pMission->WeeklyMission );
}

void CDNMissionSystem::RequestForceAchieveWeeklyMission( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	TDailyMission *pWeeklyMission;

	for( DWORD i=0; i<m_nVecDailyMissionList[Weekly][pData->AchieveCondition.cEvent].size(); i++ ) {
		int nArrayIndex = m_nVecDailyMissionList[Weekly][pData->AchieveCondition.cEvent][i];
		pWeeklyMission = &pMission->WeeklyMission[ nArrayIndex ];
		if( pWeeklyMission->nMissionID == nItemID ) {
			pWeeklyMission->bAchieve = true;
			pWeeklyMission->nCounter = pData->nCounterParam;
			RequestDailyMissionReward( pWeeklyMission->nMissionID );
			OnAchieveDailyMission( Weekly, nArrayIndex );

			m_pUserSession->GetDBConnection()->QueryModDailyWeeklyMission(m_pUserSession, DAILYMISSION_WEEK, nArrayIndex, pWeeklyMission);	// Save to DB

			m_nVecDailyMissionList[Weekly][pData->AchieveCondition.cEvent].erase( m_nVecDailyMissionList[Weekly][pData->AchieveCondition.cEvent].begin() + i );
			break;
		}
	}
}

#if defined(PRE_ADD_MONTHLY_MISSION)
void CDNMissionSystem::RequestForceGainMonthlyMission( int nCount, ... )
{
	va_list list;
	va_start( list, nCount );
	std::vector<int> nVecList;
	for( int i=0; i<nCount; i++ ) {
		nVecList.push_back( va_arg( list, int ) );
		if( nVecList.size() == MONTHLYMISSIONMAX ) break;
	}
	va_end( list );

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	int nOffset = 0;
	memset( pMission->MonthlyMission, 0, sizeof(pMission->MonthlyMission) );

	for( DWORD i=0; i<nVecList.size(); i++ ) {
		TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nVecList[i] );
		if( !pData ) continue;

		pMission->MonthlyMission[nOffset].nMissionID = nVecList[i];
		nOffset++;
	}
	RefreshMonthlyMissionList();

	m_pUserSession->SendDailyMissionList( Monthly, pMission->MonthlyMission );
}

void CDNMissionSystem::RequestForceAchieveMonthlyMission( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	TDailyMission *pMonthlyMission;

	for( DWORD i=0; i<m_nVecDailyMissionList[Monthly][pData->AchieveCondition.cEvent].size(); i++ ) {
		int nArrayIndex = m_nVecDailyMissionList[Monthly][pData->AchieveCondition.cEvent][i];
		pMonthlyMission = &pMission->MonthlyMission[ nArrayIndex ];
		if( pMonthlyMission->nMissionID == nItemID ) {
			pMonthlyMission->bAchieve = true;
			pMonthlyMission->nCounter = pData->nCounterParam;
			RequestDailyMissionReward( pMonthlyMission->nMissionID );
			OnAchieveDailyMission( Monthly, nArrayIndex );

			m_pUserSession->GetDBConnection()->QueryModDailyWeeklyMission(m_pUserSession, DAILYMISSION_MONTH, nArrayIndex, pMonthlyMission);	// Save to DB

			m_nVecDailyMissionList[Monthly][pData->AchieveCondition.cEvent].erase( m_nVecDailyMissionList[Monthly][pData->AchieveCondition.cEvent].begin() + i );
			break;
		}
	}
}
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

void CDNMissionSystem::RequestForceAchieveGuildWarMission( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if(!pMission) return;

	int nArrayIndex = 0;
	char Event = pData->AchieveCondition.cEvent;
	TDailyMission *pDailyMission;

	for( DWORD i=0; i<m_nVecDailyMissionList[GuildWar][Event].size(); i++ ) 
	{
		nArrayIndex = m_nVecDailyMissionList[GuildWar][Event][i];
		pDailyMission = &pMission->GuildWarMission[nArrayIndex];

		if( pDailyMission->nMissionID == nItemID )
		{
			bool bAchieve = true;
			bool bSecret = false;
			pDailyMission->bAchieve = true;		
			pDailyMission->nCounter = 0;

			// Accumulate preliminary points
			int nRewardPoint = g_pDataManager->GetDailyMissionRewarePoint(pDailyMission->nMissionID);
#if defined(_VILLAGESERVER)
			if(g_pGuildWarManager->IsSecretMission(m_pUserSession->GetGuildSelfView().cTeamColorCode, pDailyMission->nMissionID))
#elif defined(_GAMESERVER)
			if(g_pMasterConnectionManager->IsSecretMission(m_pUserSession->GetWorldSetID(), m_pUserSession->GetGuildSelfView().cTeamColorCode, pDailyMission->nMissionID))
#endif  
			{
				nRewardPoint *= 2; // Double points when secret is active
				bSecret = true;
			}
			if (Event == EventSystem::OnDungeonClear && api_Mission_MapSubType(m_pUserSession) == GlobalEnum::MAPSUB_NONE)
			{
				int nLevelLimit =  static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::GuildWarMission_LevelLimit));

				if (m_pUserSession->GetLevel() <= nLevelLimit)
				{
					nRewardPoint  = (int)(nRewardPoint * static_cast<float>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::GuildWarMission_Rate)));
					if (nRewardPoint <= 0)
						nRewardPoint = 1;
				}
			}

			m_pUserSession->GetDBConnection()->QueryAddGuildWarPoint(m_pUserSession, pDailyMission->nMissionID, nRewardPoint);

			pMission->nMissionScore++;	// Increment guild mission by 1 point.
			OnAchieveDailyMission( GuildWar, nArrayIndex, bSecret);
		
			m_pUserSession->GetDBConnection()->QueryModGuildWarMission(m_pUserSession, DAILYMISSION_GUILDWAR, nArrayIndex, pDailyMission, bAchieve); // Save to DB
		}
	}
}

void CDNMissionSystem::RequestForceAchieveGuildCommonMission( int nItemID )
{
	TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( nItemID );
	if( !pData ) return;

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if(!pMission) return;

	int nArrayIndex = 0;
	char Event = pData->AchieveCondition.cEvent;
	TDailyMission *pDailyMission;

	for( DWORD i=0; i<m_nVecDailyMissionList[GuildCommon][Event].size(); i++ ) 
	{
		nArrayIndex = m_nVecDailyMissionList[GuildCommon][Event][i];
		pDailyMission = &pMission->GuildCommonMission[nArrayIndex];

		if( pDailyMission->nMissionID == nItemID ) 
		{
			bool bAchieve = true;
			pDailyMission->bAchieve = true;
			pDailyMission->nCounter = 0;

			// Accumulate guild points
			int nRewardPoint = g_pDataManager->GetDailyMissionRewarePoint(pDailyMission->nMissionID);

			m_pUserSession->AddGuildPoint(GUILDPOINTTYPE_MISSION, nRewardPoint, pDailyMission->nMissionID);

			RequestDailyMissionReward( pDailyMission->nMissionID );
			OnAchieveDailyMission(GuildCommon, nArrayIndex);
		
			m_pUserSession->GetDBConnection()->QueryModGuildWarMission(m_pUserSession, DAILYMISSION_GUILDCOMMON, nArrayIndex, pDailyMission, bAchieve); // Save to DB

		}
	}
}

bool CDNMissionSystem::bIsDailyMission( const int nItemID )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	for( UINT i=0 ; i<_countof(pMission->DailyMission) ; ++i )
	{
		if( pMission->DailyMission[i].nMissionID == nItemID )
			return true;
	}
	return false;
}

bool CDNMissionSystem::bIsWeeklyMission( const int nItemID )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	for( UINT i=0 ; i<_countof(pMission->WeeklyMission) ; ++i )
	{
		if( pMission->WeeklyMission[i].nMissionID == nItemID )
			return true;
	}
	return false;
}

#if defined(PRE_MOD_MISSION_HELPER)
bool CDNMissionSystem::bCheckDailyMission( const int nItemID, DNNotifier::Type::eType eType )
{
	if( eType == DNNotifier::Type::NormalMission )
		return bIsCommonMission(nItemID);

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if(!pMission) return false;
	TDailyMission *pDailyMission = NULL;
	int nCount = 0;

	switch(eType)
	{
	case DNNotifier::Type::DailyMission:
		pDailyMission = pMission->DailyMission;
		nCount = _countof(pMission->DailyMission);
		break;
	case DNNotifier::Type::WeeklyMission:
		pDailyMission = pMission->WeeklyMission;
		nCount = _countof(pMission->WeeklyMission);
		break;
#if defined(PRE_ADD_MONTHLY_MISSION)
	case DNNotifier::Type::MonthlyMission:
		pDailyMission = pMission->MonthlyMission;
		nCount = _countof(pMission->MonthlyMission);
		break;
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
	case DNNotifier::Type::GuildWarMission:
		pDailyMission = pMission->GuildWarMission;
		nCount = _countof(pMission->GuildWarMission);
		break;
	case DNNotifier::Type::PCBangMission:
		pDailyMission = pMission->PCBangMission;
		nCount = _countof(pMission->PCBangMission);
		break;			
	case DNNotifier::Type::GuildCommonMission:
		pDailyMission = pMission->GuildCommonMission;
		nCount = _countof(pMission->GuildCommonMission);
		break;			
	case DNNotifier::Type::WeekendEventMission:
		pDailyMission = pMission->WeekendEventMission;
		nCount = _countof(pMission->WeekendEventMission);
		break;
	case DNNotifier::Type::WeekendRepeatMission:
		pDailyMission = pMission->WeekendRepeatMission;
		nCount = _countof(pMission->WeekendRepeatMission);
		break;
	}
	if(!pDailyMission) return false;

	for( int i=0 ; i<nCount ; ++i )
	{
		if( pDailyMission[i].nMissionID == nItemID )
			return true;
	}
	return false;
}

bool CDNMissionSystem::bIsCommonMission( const int nItemID )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if( GetBitFlag( pMission->MissionGain, nItemID ) ) 
		return true;

	return false;
}
#endif

bool CDNMissionSystem::bIsGuildWarMission()
{
	short wScheduleID = 0;
	char cStep = 0;
#if defined(_VILLAGESERVER)
	wScheduleID = g_pGuildWarManager->GetScheduleID();
	cStep = g_pGuildWarManager->GetStepIndex();
#elif(_GAMESERVER)
	wScheduleID = g_pMasterConnectionManager->GetGuildWarScheduleID(m_pUserSession->GetWorldSetID());
	cStep = g_pMasterConnectionManager->GetGuildWarStepIndex(m_pUserSession->GetWorldSetID());
#endif // #elif(_GAMESERVER)

	if(wScheduleID <= 0)
		return false;

	if(wScheduleID != m_pUserSession->GetGuildSelfView().wWarSchduleID || cStep != GUILDWAR_STEP_TRIAL)
		return false;

	const TGuildUID GuildUID = m_pUserSession->GetGuildUID();
	if(!GuildUID.IsSet())
		return false;

	return true;
}


// Reset guild missions
bool CDNMissionSystem::InitGuildWarMissionList(bool bCheckAchieve)
{
	bool bReset = false;
	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	if(bCheckAchieve)
	{
		for( int i=0; i<GUILDWARMISSIONMAX; i++ )
		{
			if( pMission->GuildWarMission[i].bAchieve == true || pMission->GuildWarMission[i].nCounter > 0) 
			{
				bReset = true;	// Proceed with initialization
				break;
			}
		}
	}
	else
	{
		bReset = true;	// Create a new mission.
	}

	if(!bReset)
		return false;	// If not initialized, populate the mission list.


	memset( pMission->GuildWarMission, 0, sizeof(pMission->GuildWarMission) );

	{
		ScopeLock<CSyncLock> Lock(m_Lock);
		for( int i=0; i<EventSystem::EventTypeEnum_Amount; i++ ) 
		{
			SAFE_DELETE_VEC( m_nVecDailyMissionList[GuildWar][i] );
		}

		int nMissionID = 0;
		for( int i=0; i<GUILDWARMISSIONMAX; i++ )
		{
			TDailyMissionData *pData = g_pDataManager->GetGuildWarMissionData( i );
			if( !pData ) continue;
			if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
				continue;

			pMission->GuildWarMission[i].nMissionID = pData->nItemID;
			m_nVecDailyMissionList[GuildWar][pData->AchieveCondition.cEvent].push_back(i);
		}
	}
	m_pUserSession->GetDBConnection()->QuerySetGuildWarMission(m_pUserSession);
	return true;
}

bool CDNMissionSystem::OnEvent( EventSystem::EventTypeEnum Event )
{
	std::vector<int> nVecAchieveList;

	CheckMissionGain(Event);			// Check acquisition
	CheckMissionAchieve(Event, nVecAchieveList);	// Check achievement
	CheckDailyMissionAchieve(Event);	// Check DailyMission

#if defined(PRE_ADD_ACTIVEMISSION) && defined(_GAMESERVER)
	int nActiveMissionAchieveID = CheckActiveMissionAchieve(Event);	// Check ActiveMission

	// When recursively calling OnEvent inside here, preserve the exact order!
	if( nActiveMissionAchieveID > 0 )
		m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnActiveMissionClear, 1, EventSystem::ActiveMissionID, nActiveMissionAchieveID );	
#endif
	// Fire OnMissionClear event when a normal Mission is cleared.
	if( !nVecAchieveList.empty() )
	{		
		for( DWORD i=0; i<nVecAchieveList.size(); i++ )
			m_pUserSession->GetEventSystem()->OnEvent( EventSystem::OnMissionClear, 1, EventSystem::MissionID, nVecAchieveList[i] );
	}

	return true;
}

bool CDNMissionSystem::CheckMetaMissionAchieveEvent( TMissionData *pData )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	// Meta achievement processing also supports AND and OR operations.
	if( pData->AchieveCondition.bCheckType == false ) {
		for( int i=0; i<5; i++ ) {
			if( pData->AchieveCondition.cType[i] == EventSystem::None ) continue;	// Pass if condition is set to none
			if( GetBitFlag( pMission->MissionAchieve, pData->AchieveCondition.nParam[i] ) == false ) return false;
		}
		return true;
	}
	else {
		for( int i=0; i<5; i++ ) {
			if( pData->AchieveCondition.cType[i] == EventSystem::None ) continue;
			if( GetBitFlag( pMission->MissionAchieve, pData->AchieveCondition.nParam[i] ) == true ) return true;
		}
		return false;
	}
	return false;
}

void CDNMissionSystem::CheckMissionGain( EventSystem::EventTypeEnum Event )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	bool bMissionGain = false;
	int nArrayIndex;
	for( DWORD i=0; i<m_nVecMissionGainList[Event].size(); i++ ) {
		nArrayIndex = m_nVecMissionGainList[Event][i];
		TMissionData *pData = g_pDataManager->GetMissionData(nArrayIndex);
		if(!pData) continue;

		if( m_pUserSession->GetEventSystem()->CheckEventCondition( &pData->GainCondition ) == true ) {
			SetBitFlag( pMission->MissionGain, nArrayIndex, true );
			OnGainMission( nArrayIndex );

			// If acquisition succeeds, add to the achieved list.
			m_nVecMissionAchieveList[pData->AchieveCondition.cEvent].push_back(nArrayIndex);
			m_nVecMissionGainList[Event].erase( m_nVecMissionGainList[Event].begin() + i );
			i--;
			bMissionGain = true;
		}
	}

	if( bMissionGain == true )
		m_pUserSession->GetDBConnection()->QueryMissionGain(m_pUserSession);
}

void CDNMissionSystem::CheckMissionAchieve( EventSystem::EventTypeEnum Event, std::vector<int> &nVecAchieveList )
{
	int nArrayIndex;
 	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	for( DWORD i=0; i<m_nVecMissionAchieveList[Event].size(); i++ ) {
		nArrayIndex = m_nVecMissionAchieveList[Event][i];
		TMissionData *pData = g_pDataManager->GetMissionData(nArrayIndex);
		if(!pData) continue;
		
		bool bAchieve = false;		
		if( pData->nType == MissionType_MetaShow || pData->nType == MissionType_MetaHide )
			bAchieve = CheckMetaMissionAchieveEvent( pData );	// If it is a meta mission, check it using another method.
		else
			bAchieve = m_pUserSession->GetEventSystem()->CheckEventCondition( &pData->AchieveCondition );

		if( bAchieve ) {
			SetBitFlag( pMission->MissionAchieve, nArrayIndex, true );
			RequestMissionReward( nArrayIndex );
			OnAchieveMission( nArrayIndex );
			
			switch( pData->nType ) {
			case MissionType_Normal:
			case MissionType_MetaShow:
				UpdateLastAchieveList( nArrayIndex );
				break;
			case MissionType_EventPopUp:
				OnEventPopUp( nArrayIndex );
				break;
			case MissionType_HelpAlarm:
				OnHelpAlarm( nArrayIndex );
				break;
			case MissionType_MetaHide:
			case MissionType_Quest:
				break;
			}

			nVecAchieveList.push_back( nArrayIndex + 1 );
			m_nVecMissionAchieveList[Event].erase( m_nVecMissionAchieveList[Event].begin() + i );
			i--;
		}	
	}	
}

#if defined(PRE_ADD_ACTIVEMISSION)

void CDNMissionSystem::ResetActiveMission()
{
	m_pActiveMission = NULL;
	m_ActiveMissionState = ActiveMission::NOT_ASSIGNED;
}

void CDNMissionSystem::SetActiveMission( TActiveMissionData *pData )
{
	if(!pData) return;

	m_pActiveMission = pData;
	m_ActiveMissionState = ActiveMission::GAIN_MISSION;

	m_pUserSession->SendActiveMissionGain(pData->nIndex);
}

//Returns the mission ID when the active mission is completed.
int CDNMissionSystem::CheckActiveMissionAchieve( EventSystem::EventTypeEnum Event )
{
	if(m_ActiveMissionState != ActiveMission::GAIN_MISSION) return 0;
	if(!m_pActiveMission || m_pActiveMission->AchieveCondition.cEvent != Event ) return 0;

	if(m_pUserSession->GetEventSystem()->CheckEventCondition( &m_pActiveMission->AchieveCondition ))
	{
		m_ActiveMissionState = ActiveMission::COMPLETE_MISSION;
		m_pUserSession->SendActiveMissionAchieve( m_pActiveMission->nIndex );

		if(m_pActiveMission->nMailID > 0){
			CDNMailSender::Process(m_pUserSession, m_pActiveMission->nMailID, DBDNWorldDef::PayMethodCode::ActiveMission, m_pActiveMission->nIndex);
		}

		return m_pActiveMission->nIndex;
	}
	return 0;
}

#endif	//#if defined(PRE_ADD_ACTIVEMISSION)

TDailyMission * CDNMissionSystem::GetDailyMission(CDNMissionSystem::DailyMissionTypeEnum iMissionType,int nArrayIndex)
{
	if( nArrayIndex < 0 ) return NULL;
	TMissionGroup *pMission = m_pUserSession->GetMissionData();

	switch(iMissionType)
	{
	case CDNMissionSystem::Daily:
		if( nArrayIndex >= DAILYMISSIONMAX ) return NULL;
		return &pMission->DailyMission[nArrayIndex];
	case CDNMissionSystem::Weekly:
		if( nArrayIndex >= WEEKLYMISSIONMAX ) return NULL;
		return &pMission->WeeklyMission[nArrayIndex];
#if defined( PRE_ADD_MONTHLY_MISSION )
	case CDNMissionSystem::Monthly:
		if( nArrayIndex >= MONTHLYMISSIONMAX ) return NULL;
		return &pMission->MonthlyMission[nArrayIndex];
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )
	case CDNMissionSystem::GuildWar:
		if( nArrayIndex >= GUILDWARMISSIONMAX ) return NULL;
		return &pMission->GuildWarMission[nArrayIndex];
	case CDNMissionSystem::PCBang:
		if( nArrayIndex >= PCBangMissionMax ) return NULL;
		return &pMission->PCBangMission[nArrayIndex];
	case CDNMissionSystem::GuildCommon:
		if( nArrayIndex >= GUILDCOMMONMISSIONMAX ) return NULL;
		return &pMission->GuildCommonMission[nArrayIndex];
	case CDNMissionSystem::WeekendEvent:
		if( nArrayIndex >= WEEKENDEVENTMISSIONMAX ) return NULL;
		return &pMission->WeekendEventMission[nArrayIndex];
	case CDNMissionSystem::WeekendRepeat:
		if( nArrayIndex >= WEEKENDREPEATMISSIONMAX ) return NULL;
		return &pMission->WeekendRepeatMission[nArrayIndex];
	default:
		return NULL;
	}
}

void CDNMissionSystem::CheckDailyMissionAchieve( EventSystem::EventTypeEnum Event )
{
	TMissionGroup *pMission = m_pUserSession->GetMissionData();	
	TDailyMission *pDailyMission;
	int nArrayIndex;

	for( int iMissionType = 0 ; iMissionType < DailyMissionTypeEnum_Amount; iMissionType++ )
	{
		if( iMissionType == GuildWar )	//Check conditions for GuildWar
		{	
			if( !bIsGuildWarMission() ) continue;
		}
		if( iMissionType == GuildCommon && !m_pUserSession->GetGuildUID().IsSet() )
			continue;

		for( DWORD i=0; i<m_nVecDailyMissionList[iMissionType][Event].size(); i++ ) {
			nArrayIndex = m_nVecDailyMissionList[iMissionType][Event][i];
			pDailyMission = GetDailyMission((CDNMissionSystem::DailyMissionTypeEnum)iMissionType, nArrayIndex);
			if(!pDailyMission)
			{
				_DANGER_POINT();
				continue;
			}

			TDailyMissionData *pData = g_pDataManager->GetDailyMissionData( pDailyMission->nMissionID );
			if( !pData ) continue;

			//Daily mission completion condition met
			if( m_pUserSession->GetEventSystem()->CheckEventCondition(&pData->AchieveCondition) )
				pDailyMission->nCounter++;
			else
				continue;			
			
			bool bAchieve = true;
			switch( pData->cCounterOperator ) {
				case 0: if( !( pDailyMission->nCounter == pData->nCounterParam ) ) bAchieve = false; break;
				case 1: if( !( pDailyMission->nCounter != pData->nCounterParam ) ) bAchieve = false; break;
				case 2: if( !( pDailyMission->nCounter > pData->nCounterParam ) ) bAchieve = false; break;
				case 3: if( !( pDailyMission->nCounter < pData->nCounterParam ) ) bAchieve = false; break;
				case 4: if( !( pDailyMission->nCounter >= pData->nCounterParam ) ) bAchieve = false; break;
				case 5: if( !( pDailyMission->nCounter <= pData->nCounterParam ) ) bAchieve = false; break;
			}
			
			bool bIsRepeatMission = false;
			if( bAchieve ) //Mission achieved
			{
				bool bSecret = false;
				pDailyMission->bAchieve = true;
				RequestDailyMissionReward( pDailyMission->nMissionID );

				//Repeatable clearable mission
				if( iMissionType == WeekendRepeat || iMissionType == GuildWar || iMissionType == GuildCommon )
				{
					pDailyMission->nCounter = 0;
					bIsRepeatMission = true;
				}
				else	//If not repeatable clear, delete the completed one.
				{
					m_nVecDailyMissionList[iMissionType][Event].erase( m_nVecDailyMissionList[iMissionType][Event].begin() + i );
					i--;
				}
				
				//Additional work per mission type
				if( iMissionType == GuildWar )
				{
					bSecret = AddGuildWarPoint(pDailyMission, Event, pMission, nArrayIndex);
				}
				else if( iMissionType == PCBang )
				{	
					ApplyPCBangMissionEffect(Event);
				}
				else if( iMissionType == GuildCommon )
				{
					AddGuildPoint(pDailyMission);
				}

				OnAchieveDailyMission( (CDNMissionSystem::DailyMissionTypeEnum)iMissionType, nArrayIndex, bSecret );

				//Daily, Weekly mission clear event
				if( iMissionType == Daily || iMissionType == Weekly )
				{
					if(bIsGuildWarMission())
					{	//Currently OnDailyMissionClear and OnWeeklyMissionClear missions are used for earning guild points only during GuildWar (2012-08-14 by stupidfox)
						EventSystem::EventTypeEnum MissionEvent = (iMissionType == Daily ? EventSystem::OnDailyMissionClear : EventSystem::OnWeeklyMissionClear);
						OnEvent( MissionEvent );
					}
				}
			}
			else
			{	//If completion conditions are met but achieve count is insufficient, only increment count
				OnCounterDailyMission( (CDNMissionSystem::DailyMissionTypeEnum)iMissionType, nArrayIndex, pDailyMission->nCounter );
			}

			// eDailyMissionType differs from DailyMissionTypeEnum by 1 => iMissionType+1
			m_pUserSession->GetDBConnection()->QueryModDailyWeeklyMission(m_pUserSession, iMissionType+1, nArrayIndex, pDailyMission, bIsRepeatMission);	// Save to DB
		}
	}
}

bool CDNMissionSystem::AddGuildWarPoint( TDailyMission * pDailyMission, EventSystem::EventTypeEnum Event, TMissionGroup * pMission, int nArrayIndex )
{
	// Accumulate preliminary points
	int nRewardPoint = g_pDataManager->GetDailyMissionRewarePoint(pDailyMission->nMissionID);

	bool bSecret = false;
#if defined(_VILLAGESERVER)
	if(g_pGuildWarManager->IsSecretMission(m_pUserSession->GetGuildSelfView().cTeamColorCode, pDailyMission->nMissionID))
#elif defined(_GAMESERVER)
	if(g_pMasterConnectionManager->IsSecretMission(m_pUserSession->GetWorldSetID(), m_pUserSession->GetGuildSelfView().cTeamColorCode, pDailyMission->nMissionID))
#endif  
	{
		nRewardPoint *= 2; // Double points when secret is active
		bSecret = true;
	}

	int nMapSubType = api_Mission_MapSubType(m_pUserSession);
	if (Event == EventSystem::OnDungeonClear && ( GlobalEnum::MAPSUB_NONE == nMapSubType || GlobalEnum::MAPSUB_TREASURESTAGE == nMapSubType ) )
	{
		int nLevelLimit =  static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::GuildWarMission_LevelLimit));

		if (m_pUserSession->GetLevel() <= nLevelLimit)
		{
			nRewardPoint  = (int)(nRewardPoint * static_cast<float>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::GuildWarMission_Rate)));
			if (nRewardPoint <= 0)
				nRewardPoint = 1;
		}
	}

	m_pUserSession->GetDBConnection()->QueryAddGuildWarPoint(m_pUserSession, pDailyMission->nMissionID, nRewardPoint);
	pMission->nMissionScore++;	// Increment guild mission by 1 point.
	
	return bSecret;
}

void CDNMissionSystem::ApplyPCBangMissionEffect( EventSystem::EventTypeEnum Event )
{
	switch(Event)
	{
	case EventSystem::OnPCBang_Fatigue:
		m_pUserSession->SetDefaultPCBangFatigue();
		break;

	case EventSystem::OnPCBang_RebirthCoin:
		m_pUserSession->SetDefaultPCBangRebirthCoin();
		break;
	}
}

void CDNMissionSystem::AddGuildPoint(TDailyMission * pDailyMission)
{
	// Accumulate guild points
	int nRewardPoint = g_pDataManager->GetDailyMissionRewarePoint(pDailyMission->nMissionID);
	// Check limit amount
	m_pUserSession->AddGuildPoint(GUILDPOINTTYPE_MISSION, nRewardPoint, pDailyMission->nMissionID);
}

bool CDNMissionSystem::bIsAchieveMission( const int nItemID ) const
{
	if( 0 == nItemID ) // Remove ambiguity since 1 -> 0 and 0 -> 0
	{
		return false;
	}

	DNTableFileFormat *pSox	= GetDNTable( CDnTableDB::TMISSION );
	int nTargetArrayIndex	= pSox->GetArrayIndex( nItemID );
	TMissionData * pData	= g_pDataManager->GetMissionData( nTargetArrayIndex );
	if( NULL == pData || false == pData->bActivate )
	{
		return false;
	}

	TMissionGroup *pMission = m_pUserSession->GetMissionData();
	return GetBitFlag( pMission->MissionAchieve, nTargetArrayIndex );
}
