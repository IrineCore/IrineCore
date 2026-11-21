#include "stdafx.h"
#include "DNMissionScheduler.h"
#include "DnTableDB.h"
#include "PrandGen.h"
#include "DNGameDataManager.h"
#include "DNMissionSystem.h"
#include "TimeSet.h"

CDNMissionScheduler::CDNMissionScheduler()
{
	Initialize();
}

CDNMissionScheduler::~CDNMissionScheduler()
{
}

bool CDNMissionScheduler::Initialize()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSIONSCHEDULE );

	char szLabel[32];
	char *szGroupStr;
	//char *szValue;
	std::string strValue;

	for( int i=0; i<10; i++ ) {
		for( int j=0; j<PLAYER_MAX_LEVEL; j++ ) {
			int nItemID = ( i * 100 ) +( j + 1 );
			for( int k=0; k<DAILYMISSIONMAX; k++ ) {
				sprintf_s( szLabel, "_DailyGroup%d", k + 1 );
				szGroupStr = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();

				if( strlen( szGroupStr ) > 0 ) {
					for( int n=0;; n++ ) {
						strValue = _GetSubStrByCountSafe( n, szGroupStr, ';' );
						if( !strValue.size() ) break;
						int nResultID = atoi(strValue.c_str());
						if( nResultID < 1 ) continue;
						m_nVecDailyMissionList[i][j][k].push_back( nResultID );
					}
				}

				sprintf_s( szLabel, "_WeeklyGroup%d", k + 1 );
				szGroupStr = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();

				if( strlen( szGroupStr ) > 0 ) {
					for( int n=0;; n++ ) {
						strValue = _GetSubStrByCountSafe( n, szGroupStr, ';' );
						if( !strValue.size() ) break;
						int nResultID = atoi(strValue.c_str());
						if( nResultID < 1 ) continue;
						m_nVecWeeklyMissionList[i][j][k].push_back( nResultID );
					}
				}
#if defined(PRE_ADD_MONTHLY_MISSION)
				sprintf_s( szLabel, "_MonthlyGroup%d", k + 1 );
				szGroupStr = pSox->GetFieldFromLablePtr( nItemID, szLabel )->GetString();

				if( strlen( szGroupStr ) > 0 ) {
					for( int n=0;; n++ ) {
						strValue = _GetSubStrByCountSafe( n, szGroupStr, ';' );
						if( !strValue.size() ) break;
						int nResultID = atoi(strValue.c_str());
						if( nResultID < 1 ) continue;
						m_nVecMonthlyMissionList[i][j][k].push_back( nResultID );
					}
				}
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)
			}
		}
	}
	return true;
}

void CDNMissionScheduler::RefreshDailyMissionList( int nRandomSeed, TMissionGroup *pMission, int nLevel, int nClassID )
{
	CMtRandom Random;
	Random.srand( nRandomSeed );

	int nResult;
	int nCount;
	int nOffset = 0;
	memset( pMission->DailyMission, 0, sizeof(pMission->DailyMission) );
	for( int i=0; i<DAILYMISSIONMAX; i++ ) {
		nCount = (int)m_nVecDailyMissionList[nClassID-1][nLevel-1][i].size();
		if( nCount < 1 ) continue;
		nResult = m_nVecDailyMissionList[nClassID-1][nLevel-1][i][Random.rand()%nCount];
		if( nResult < 1 ) continue;

		pMission->DailyMission[nOffset].nMissionID = nResult;
		nOffset++;
		if( nOffset >= DAILYMISSIONMAX ) break;
	}
}

void CDNMissionScheduler::RefreshWeeklyMissionList( int nRandomSeed, TMissionGroup *pMission, int nLevel, int nClassID )
{
	CMtRandom Random;
	Random.srand( nRandomSeed );

	int nResult;
	int nCount;
	int nOffset = 0;
	memset( pMission->WeeklyMission, 0, sizeof(pMission->WeeklyMission) );

	for( int i=0; i<WEEKLYMISSIONMAX; i++ ) {
		nCount = (int)m_nVecWeeklyMissionList[nClassID-1][nLevel-1][i].size();
		if( nCount < 1 ) continue;
		nResult = m_nVecWeeklyMissionList[nClassID-1][nLevel-1][i][Random.rand()%nCount];
		if( nResult < 1 ) continue;

		bool bExist = false;
		for( int j=0; j<nOffset-1; j++ ) {
			if( pMission->WeeklyMission[j].nMissionID == nResult ) {
				bExist = true;
				break;
			}
		}
		if( bExist ) continue; 
		pMission->WeeklyMission[nOffset].nMissionID = nResult;
		nOffset++;
		if( nOffset >= WEEKLYMISSIONMAX ) break;
	}
}
#if defined(PRE_ADD_MONTHLY_MISSION)
void CDNMissionScheduler::RefreshMonthlyMissionList( int nRandomSeed, TMissionGroup *pMission, int nLevel, int nClassID )
{
	CMtRandom Random;
	Random.srand( nRandomSeed );

	int nResult;
	int nCount;
	int nOffset = 0;
	memset( pMission->MonthlyMission, 0, sizeof(pMission->MonthlyMission) );
	for( int i=0; i<MONTHLYMISSIONMAX; i++ ) {
		nCount = (int)m_nVecMonthlyMissionList[nClassID-1][nLevel-1][i].size();
		if( nCount < 1 ) continue;
		nResult = m_nVecMonthlyMissionList[nClassID-1][nLevel-1][i][Random.rand()%nCount];
		if( nResult < 1 ) continue;

		bool bExist = false;
		for( int j=0; j<nOffset-1; j++ ) {
			if( pMission->MonthlyMission[j].nMissionID == nResult ) {
				bExist = true;
				break;
			}
		}
		if( bExist ) continue; 
		pMission->MonthlyMission[nOffset].nMissionID = nResult;
		nOffset++;
		if( nOffset >= MONTHLYMISSIONMAX ) break;
	}
}
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

void CDNMissionScheduler::RefreshWeekendEventMissionList( TMissionGroup *pMission )
{
	memset( pMission->WeekendEventMission, 0, sizeof(pMission->WeekendEventMission) );

	CTimeSet tLocalSet;
#if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	int nOffset = 0;
	for( int i = 0; i < g_pDataManager->GetWeekendMissionCount(DAILYMISSION_WEEKENDEVENT); i++)
#else	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	for( int i=0; i<WEEKENDEVENTMISSIONMAX; i++ ) 
#endif	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	{
		TDailyMissionData *pData = g_pDataManager->GetWeekendEventMissionData( i );
		if( !pData ) continue;

		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

#if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
		if ( tLocalSet.GetTimeT64_LC() < pData->tStartDate || 
			 ((pData->tEndDate != -1) && ( tLocalSet.GetTimeT64_LC() > pData->tEndDate)) )
			continue;

		pMission->WeekendEventMission[nOffset].nMissionID = pData->nItemID;
		nOffset++;
		if(nOffset >= WEEKENDEVENTMISSIONMAX) break;
#else	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
		if (tLocalSet.GetTimeT64_LC() > pData->tStartDate)
			pMission->WeekendEventMission[i].nMissionID = pData->nItemID;
#endif	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	}
}

void CDNMissionScheduler::RefreshWeekendRepeatMissionList( TMissionGroup *pMission )
{
	memset( pMission->WeekendRepeatMission, 0, sizeof(pMission->WeekendRepeatMission) );

	CTimeSet tLocalSet;
#if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	int nOffset = 0;
	for( int i = 0; i < g_pDataManager->GetWeekendMissionCount(DAILYMISSION_WEEKENDREPEAT); i++)
#else	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	for( int i=0; i<WEEKENDREPEATMISSIONMAX; i++ ) 
#endif	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	{
		TDailyMissionData *pData = g_pDataManager->GetWeekendRepeatMissionData( i );
		if( !pData ) continue;

		if( pData->AchieveCondition.cEvent >= EventSystem::EventTypeEnum_Amount )
			continue;

#if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
		if ( tLocalSet.GetTimeT64_LC() < pData->tStartDate || 
			 ((pData->tEndDate != -1) && ( tLocalSet.GetTimeT64_LC() > pData->tEndDate)) )
			continue;

		pMission->WeekendRepeatMission[nOffset].nMissionID = pData->nItemID;
		nOffset++;
		if(nOffset >= WEEKENDREPEATMISSIONMAX) break;
#else	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
		if (tLocalSet.GetTimeT64_LC() > pData->tStartDate)
			pMission->WeekendRepeatMission[i].nMissionID = pData->nItemID;
#endif	// #if defined(PRE_ADD_WEEKLYEVENT_MISSION_ENDTIME)
	}
}

#if defined( PRE_ADD_STAMPSYSTEM )
bool CDNMissionScheduler::IsDailyMissionGroup( int nLevel, int nClassID, int nMissionGroupID, int nMissionID ) const
{
	if( nLevel > PLAYER_MAX_LEVEL || nLevel < 1 ||
		nClassID > 10 || nClassID < 1 || 
		nMissionGroupID > DAILYMISSIONMAX || nMissionGroupID < 1 )
	{
		return false;
	}

	const std::vector<int> & rVecMissionID = m_nVecDailyMissionList[nClassID - 1][nLevel - 1][nMissionGroupID - 1];

	std::vector<int>::const_iterator iter = std::find( rVecMissionID.begin(), rVecMissionID.end(), nMissionID );
	if(iter != rVecMissionID.end())
	{
		return true;
	}
	return false;
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )