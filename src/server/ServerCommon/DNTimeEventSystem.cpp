#include "stdafx.h"
#include "DNTimeEventSystem.h"
#include "DnTableDB.h"
#include "DNUserSession.h"
#include "DNMissionSystem.h"
#include "DNDBConnection.h"

#ifdef _GAMESERVER
#include "DNGameRoom.h"
#include "DnGameTask.h"
#endif

CDNTimeEventSystem::CDNTimeEventSystem( CDNUserSession *pSession )
{
	m_pUserSession = pSession;
	m_dwPrevTime = 0;
}

CDNTimeEventSystem::~CDNTimeEventSystem()
{
	SAFE_DELETE_PVEC( m_pVecEventList );
}

void CDNTimeEventSystem::RegisterTableData()
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TCONNECTINGTIME );
	DNTableFileFormat *pMissionSox = GetDNTable( CDnTableDB::TMISSION );

	if( !pSox ) 
	{
		g_Log.Log( LogType::_FILELOG, L"TimeEventTable.ext failed\r\n");
		return;
	}
	CTimeSet tCurTime;
	tCurTime.Reset();
	
	for( int i=0; i<pSox->GetItemCount(); i++ ) 
	{
		int nItemID = pSox->GetItemID(i);
		bool bActivate = ( pSox->GetFieldFromLablePtr( nItemID, "_Run" )->GetInteger() ) ? true : false;
		if( !bActivate ) continue;

		char *szPeriod = pSox->GetFieldFromLablePtr( nItemID, "_EventPeriod" )->GetString();
		std::vector<string> vEventPeriod;
		TokenizeA( szPeriod, vEventPeriod, ":" );
		if( vEventPeriod.size() != 2 ) continue;

		CTimeSet tBeginSet( vEventPeriod[0].c_str(), true );
		CTimeSet tEndSet( vEventPeriod[1].c_str(), true );

		if( tCurTime.GetTimeT64_LC() > tEndSet.GetTimeT64_LC() ) continue;

		TimeEventStruct *pStruct = new TimeEventStruct;
		pStruct->nEventID = nItemID;
		pStruct->Type = (CDNTimeEventSystem::MaintenanceType)pSox->GetFieldFromLablePtr( nItemID, "_Type" )->GetInteger();
		pStruct->tBeginTime = tBeginSet;
		pStruct->tEndTime = tEndSet;
		pStruct->RepeatType = (CDNTimeEventSystem::RepeatType)pSox->GetFieldFromLablePtr( nItemID, "_RepeatType" )->GetInteger();
		pStruct->nMaintenanceTime = (INT64)( pSox->GetFieldFromLablePtr( nItemID, "_TimeCheck" )->GetInteger() * 60 * 1000);
		pStruct->nRemainTime = pStruct->nMaintenanceTime;
		pStruct->nRewardMissionID = pMissionSox->GetArrayIndex( pSox->GetFieldFromLablePtr( nItemID, "_MissionID" )->GetInteger() );
		pStruct->bDailyFlag = false;

		m_pVecEventList.push_back( pStruct );
	}
}

CDNTimeEventSystem::TimeEventStruct *CDNTimeEventSystem::GetTimeEvent( int nEventID )
{
	for( DWORD i=0; i<m_pVecEventList.size(); i++ ) {
		if( m_pVecEventList[i]->nEventID == nEventID ) return m_pVecEventList[i];
	}
	return NULL;
}

void CDNTimeEventSystem::UpdateEventStructFromUserData()
{
	TTimeEventGroup *pEventGroup = m_pUserSession->GetTimeEventData();
	for( int i=0; i<TIMEEVENTMAX; i++ ) {
		TTimeEvent *pEvent = &pEventGroup->Event[i];
		if( pEvent->nItemID < 1 ) continue;
		TimeEventStruct *pStruct = GetTimeEvent( pEvent->nItemID );
		if( !pStruct ) continue;
		/*
		if( !pStruct ) {
			pEvent->nItemID = 0;
			pEvent->dwRemainTime = 0;
		}
		*/
		pStruct->nRemainTime = pEventGroup->Event[i].nRemainTime;
		pStruct->bDailyFlag = pEventGroup->Event[i].bCheckFlag;

		// If Repeat changes from false to true, it cannot run when -1, so check here once and update the time.
		if( pStruct->RepeatType == CDNTimeEventSystem::Repeat && pStruct->nRemainTime == -1 ) {
			pStruct->nRemainTime = pStruct->nMaintenanceTime;
		}
	}
}

void CDNTimeEventSystem::UpdateUserDataFromEventStruct( bool bLogout, bool bSaveDB )
{
	// Reset everything in UserData for now.
	TTimeEventGroup *pEventGroup = m_pUserSession->GetTimeEventData();
	for( int i=0; i<TIMEEVENTMAX; i++ ) {
		pEventGroup->Event[i].nItemID = 0;
		pEventGroup->Event[i].nRemainTime = 0;
	}

	CTimeSet tCurTime;
	tCurTime.Reset();
	int nSlotIndex = 0;
	for( DWORD i=0; i<m_pVecEventList.size(); i++ ) {
		if( nSlotIndex >= TIMEEVENTMAX ) break;
		TimeEventStruct *pStruct = m_pVecEventList[i];
		TTimeEvent *pEvent = &pEventGroup->Event[nSlotIndex];

		// Do not run if start time has not been reached yet.
		// Must save even if not started yet, because it might start once the time arrives.
//		if( tCurTime.GetTimeT64_LC() < pStruct->tBeginTime.GetTimeT64_LC() ) continue;

		// If the type is persist, remove it from the save list on logout.
		if( bLogout && pStruct->Type == CDNTimeEventSystem::Endure ) {
			switch( pStruct->RepeatType ) {
				case CDNTimeEventSystem::Daily:
					if( pStruct->nRemainTime > 0 ) continue;
					break;
				default:
					continue;
			}
		}
		
		pEvent->nItemID = pStruct->nEventID;
		pEvent->nRemainTime = pStruct->nRemainTime;
		pEvent->bCheckFlag = pStruct->bDailyFlag;
		nSlotIndex++;
	}

	if( bSaveDB ) {
		if (!m_pVecEventList.empty())
			m_pUserSession->GetDBConnection()->QuerySaveConnectDurationTime(m_pUserSession);
	}
}

bool CDNTimeEventSystem::LoadUserData()
{
	RegisterTableData();
	UpdateEventStructFromUserData();
	// Re-sort UserData to match the order used here.
	UpdateUserDataFromEventStruct();
	return true;
}

bool CDNTimeEventSystem::SaveUserData( bool bLogout )
{
	UpdateUserDataFromEventStruct( bLogout );
	return true;
}

void CDNTimeEventSystem::Process( DWORD dwCurTick )
{
	if( m_dwPrevTime == 0 ) m_dwPrevTime = dwCurTick;

	if( dwCurTick - m_dwPrevTime < 1000 ) {
		return;
	}

	DWORD dwDelta = dwCurTick - m_dwPrevTime;
	m_dwPrevTime = dwCurTick;

	CTimeSet tCurTime;
	tCurTime.Reset();

	for( DWORD i=0; i<m_pVecEventList.size(); i++ ) {
		TimeEventStruct *pStruct = m_pVecEventList[i];
		// Check event period
		if( tCurTime.GetTimeT64_LC() > pStruct->tEndTime.GetTimeT64_LC() ) {
			OnExpireTimeEvent( pStruct );
			SAFE_DELETE( m_pVecEventList[i] );
			m_pVecEventList.erase( m_pVecEventList.begin() + i );
			i--;
			continue;
		}
		if( tCurTime.GetTimeT64_LC() < pStruct->tBeginTime.GetTimeT64_LC() ) continue;
		if( pStruct->RepeatType == CDNTimeEventSystem::Daily && pStruct->bDailyFlag ) continue;

		// Actual event check
		if( pStruct->nRemainTime == -1 ) continue;
		pStruct->nRemainTime -= dwDelta;

		if( pStruct->nRemainTime <= 0 ) {
			switch( pStruct->RepeatType ) {
				case CDNTimeEventSystem::None:
					pStruct->nRemainTime = -1;
					break;
				case CDNTimeEventSystem::Repeat:
					pStruct->nRemainTime = pStruct->nMaintenanceTime;
					break;
				case CDNTimeEventSystem::Daily:
					pStruct->nRemainTime = -1;
					pStruct->bDailyFlag = true;
					break;
			}

			OnAchieveTimeEvent( pStruct );
			continue;
		}
	}
}

void CDNTimeEventSystem::OnAchieveTimeEvent( TimeEventStruct *pStruct )
{
	// When referencing reward only
	m_pUserSession->GetMissionSystem()->RequestMissionReward( pStruct->nRewardMissionID );
	m_pUserSession->SendTimeEventAchieve( pStruct->nEventID, pStruct->nRemainTime );
}

void CDNTimeEventSystem::OnExpireTimeEvent( TimeEventStruct *pStruct )
{
	m_pUserSession->SendTimeEventExpire( pStruct->nEventID );
}


void CDNTimeEventSystem::RequestSyncTimeEvent()
{
	UpdateUserDataFromEventStruct( false, false );
	m_pUserSession->SendTimeEventList( m_pUserSession->GetTimeEventData() );
}

void CDNTimeEventSystem::ResetDailyTimeEvent()
{
	for( DWORD i=0; i<m_pVecEventList.size(); i++ ) {
		TimeEventStruct *pStruct = m_pVecEventList[i];
		if( pStruct->RepeatType == CDNTimeEventSystem::Daily ) {
			pStruct->bDailyFlag = false;
			pStruct->nRemainTime = pStruct->nMaintenanceTime;
		}
	}

	RequestSyncTimeEvent();

	// DB Update
	m_pUserSession->GetDBConnection()->QueryModTimeEventDate( m_pUserSession );
}