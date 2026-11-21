#include "stdafx.h"
#include "DNSchedule.h"

CDNSchedule::CDNSchedule( DWORD dwCycleTime )
{
	m_dwCycleTime = dwCycleTime;
	m_dwPrevTime = 0;
	m_bInitialize = false;
}

CDNSchedule::~CDNSchedule()
{
}

void CDNSchedule::Process( DWORD dwCurTick )
{
	if( !m_bInitialize ) return;
	if( dwCurTick - m_dwPrevTime < m_dwCycleTime ) {
		return;
	}
	m_dwPrevTime = dwCurTick;

	time_t Time;
	tm pTime;

	time(&Time);
	pTime = *localtime(&Time);

	ScheduleStruct *pSchedule;
	for( DWORD i=0; i<m_VecScheduleList.size(); i++ ) {
		pSchedule = &m_VecScheduleList[i];
		bool bEvent = false;
		switch( pSchedule->Cycle ) {
			case None:
				{
					DWORD dwTemp = 1000 * 60 * ( pSchedule->cMinute + ( pSchedule->cHour * 60 ) );
					if( dwCurTick >= pSchedule->dwLastUpdateTick + dwTemp ) {
						if( pSchedule->dwLastUpdateTick == 0 ) {
							pSchedule->dwLastUpdateTick = dwCurTick;
							break;
						}
						pSchedule->dwLastUpdateTick = dwCurTick;

						bEvent = true;
					}
				}
				break;
			case Day:
				if( pTime.tm_hour == pSchedule->cHour ) {
					if( !pSchedule->bUpdate ) {
						pSchedule->bUpdate = true;
						bEvent = true;
					}
				}
				else pSchedule->bUpdate = false;
				break;
#if defined( PRE_ADD_MONTHLY_MISSION )
			case Month:
				{
					// Do nothing if it is not the 1st (DEFAULT_MONTHLY_RESET_DAY) or scheduled reset time.
					if((pTime.tm_mday != DEFAULT_MONTHLY_RESET_DAY) || (pTime.tm_hour != pSchedule->cHour))
					{
						pSchedule->bUpdate = false;
						continue;
					}

					tm pDate = *localtime(&pSchedule->tDate);
					// If 1 month has passed since receiving the mission, or reset after logging in for the first time in a while today (1st day 00:00~08:59)
					if((pDate.tm_mon != pTime.tm_mon) || (pDate.tm_mon == pTime.tm_mon && pDate.tm_hour < pSchedule->cHour))
					{
						if( !pSchedule->bUpdate ) {
							pSchedule->bUpdate = true;
							bEvent = true;
						}
					}
					else
						pSchedule->bUpdate = false;
				}
				break;
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )
			default:
				{
					if( pTime.tm_wday == (int)pSchedule->Cycle - Sunday && pTime.tm_hour == pSchedule->cHour ) {
						if( !pSchedule->bUpdate ) {
							pSchedule->bUpdate = true;
							bEvent = true;
						}
					}
					else pSchedule->bUpdate = false;
				}
				break;
		}
		if( bEvent ) {
			pSchedule->tDate = Time;
			OnScheduleEvent( pSchedule->EventType, pSchedule, false );
		}
	}
}

void CDNSchedule::RegisterSchedule( ScheduleEventEnum ScheduleEvent, __time64_t tDate )
{
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TSCHEDULE );
	if( !pSox )
	{
		g_Log.Log( LogType::_FILELOG, L"ScheduleTable.ext failed\r\n");
		return;
	}

	int nItemID = ScheduleEvent + 1;
	if( !pSox->IsExistItem( nItemID ) ) return;

	ScheduleStruct Struct;
	Struct.EventType = ScheduleEvent;
	Struct.cHour = (char)pSox->GetFieldFromLablePtr( nItemID, "_Hour" )->GetInteger();
	Struct.cMinute = (char)pSox->GetFieldFromLablePtr( nItemID, "_Minute" )->GetInteger();
	Struct.Cycle = (CycleTypeEnum)pSox->GetFieldFromLablePtr( nItemID, "_Cycle" )->GetInteger();
	Struct.tDate = tDate;

	m_VecScheduleList.push_back( Struct );
}

void CDNSchedule::CheckInitializeSchedule()
{
	time_t Time;
	tm pTime;

	time(&Time);
	pTime = *localtime(&Time);

	ScheduleStruct *pSchedule;
	for( DWORD i=0; i<m_VecScheduleList.size(); i++ ) {
		pSchedule = &m_VecScheduleList[i];

		bool bEvent = false;
		tm pDate = *localtime(&pSchedule->tDate);
		int nDayValueDB = GetDateValue( 1900 + pDate.tm_year, pDate.tm_mon + 1, pDate.tm_mday );
		int nDayValueCur = GetDateValue( 1900 + pTime.tm_year, pTime.tm_mon + 1, pTime.tm_mday );

		switch( pSchedule->Cycle ) {
			case None: break;
			case Day:
				/*
				{
					int nDBValue = ( ( pDate.tm_year * 1000 ) + ( pDate.tm_mon * 100 ) ) + ( ( ( pDate.tm_mday * 24 ) + ( pDate.tm_hour - pSchedule->cHour ) ) / 24 );

					time_t Time;
					tm *pTime;

					time(&Time);
					pTime = localtime(&Time);
					int nCurValue = ( ( pTime.tm_year * 1000 ) + ( pTime.tm_mon * 100 ) ) + ( ( ( pTime.tm_mday * 24 ) + ( pTime.tm_hour - pSchedule->cHour ) ) / 24 );

					if( nDBValue != nCurValue ) bEvent = true;
				}
				*/
//				/*
				if( nDayValueCur - nDayValueDB == 0 && pDate.tm_hour >= pSchedule->cHour ) break;
				if( nDayValueCur - nDayValueDB == 0 ) {
					if( pTime.tm_hour < pSchedule->cHour ) break;
				}
				else if( nDayValueCur - nDayValueDB == 1 ) {
					if( pTime.tm_hour < pSchedule->cHour ) {
						if( pDate.tm_hour >= pSchedule->cHour ) break;
					}
				}
				bEvent = true;
//				*/

				break;
#if defined( PRE_ADD_MONTHLY_MISSION )
			case Month:
				{
					int nMonthResetCur = 0;
					//Get the DateValue of the last reset day based on current time
					if( pTime.tm_hour < pSchedule->cHour && pTime.tm_mday <= DEFAULT_MONTHLY_RESET_DAY)
					{
						int nYear = 1900 + pTime.tm_year;
						int nMonth = pTime.tm_mon;
						if(nMonth == 0)
						{
							nMonth = 11;
							nYear--;
						}
						else
							nMonth--;

						nMonthResetCur = GetDateValue( nYear, nMonth + 1, DEFAULT_MONTHLY_RESET_DAY ); 
					}
					else
						nMonthResetCur = GetDateValue( 1900 + pTime.tm_year, pTime.tm_mon + 1, DEFAULT_MONTHLY_RESET_DAY );

					if( (nMonthResetCur > nDayValueDB) || 
						((nMonthResetCur == nDayValueDB) && ( pSchedule->cHour > pDate.tm_hour))) // If reset date is the same, compare reset time
						bEvent = true;
				}
				break;
#endif	// #if defined( PRE_ADD_MONTHLY_MISSION )
			default:
				{
					int nWeek = pSchedule->Cycle - CDNSchedule::Sunday;
					int nValue;

					/*
					int nDBValue = ( GetDateValue( 1900 + pDate.tm_year, pDate.tm_mon + 1, 0 ) - 4 - nWeek ) / 7;
					nValue = GetDateValue( 1900 + pDate.tm_year, pDate.tm_mon + 1, pDate.tm_mday ) - 4 - nWeek;
					nOffset = nValue % 7;
					nWeekCount = ( 7 + ( pDate.tm_mday - nOffset ) + ( nOffset / 7 ) ) / 7;
					if( nOffset == 0 ) {
						if( pDate.tm_hour < pSchedule->cHour ) nWeekCount--;
					}
					nDBValue += nWeekCount;
					*/
					nValue = GetDateValue( 1900 + pDate.tm_year, pDate.tm_mon + 1, pDate.tm_mday ) - 4 - nWeek;
					int nDBValue = nValue / 7;
					if( nValue % 7 == 0 ) {
						if( pDate.tm_hour < pSchedule->cHour ) nDBValue--;
					}
					
					time_t LocalTime;
					tm pLocalTime;

					time(&LocalTime);
					pLocalTime = *localtime(&LocalTime);

					/*
					int nCurValue = ( GetDateValue( 1900 + pLocalTime.tm_year, pLocalTime.tm_mon + 1, 0 ) - 4 - nWeek ) / 7;
					nValue = GetDateValue( 1900 + pLocalTime.tm_year, pLocalTime.tm_mon + 1, pLocalTime.tm_mday ) - 4 - nWeek;
					nOffset = nValue % 7;
					nWeekCount = ( 7 + ( pLocalTime.tm_mday - nOffset ) + ( nOffset / 7 ) ) / 7;
					if( nOffset == 0 ) {
						if( pLocalTime.tm_hour < pSchedule->cHour ) nWeekCount--;
					}
					nCurValue += nWeekCount;
					*/
					nValue = GetDateValue( 1900 + pLocalTime.tm_year, pLocalTime.tm_mon + 1, pLocalTime.tm_mday ) - 4 - nWeek;
					int nCurValue = nValue / 7;
					if( nValue % 7 == 0 ) {
						if( pLocalTime.tm_hour < pSchedule->cHour ) nCurValue--;
					}

					if( nDBValue != nCurValue ) bEvent = true;
				}
				break;
		}
		if( bEvent ) {
			pSchedule->tDate = Time;
			OnScheduleEvent( pSchedule->EventType, pSchedule, true );
		}
		pSchedule->bUpdate = true;
	}
	m_bInitialize = true;
}

int CDNSchedule::GetDateValue( int nYear, int nMonth, int nDay )
{
	int nResult = 0;
	for( int i=2009; i<nYear; i++ ) {
		bool bLeapMonth = false;
//		int nValue = ( i - 1583 ) + 1;
		if( i % 4 == 0 ) bLeapMonth = true;
		if( i % 100 == 0 ) bLeapMonth = false;
		if( i % 400 == 0 ) bLeapMonth = true;

		if( bLeapMonth ) nResult += 366;
		else nResult += 365;
	}

	bool bLeapMonth = false;
//	int nValue = ( nYear - 1583 ) + 1;
	if( nYear % 4 == 0 ) bLeapMonth = true;
	if( nYear % 100 == 0 ) bLeapMonth = false;
	if( nYear % 400 == 0 ) bLeapMonth = true;
	int nDayCount[12] = { 31, ( bLeapMonth == true ) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	for( int i=0; i<nMonth - 1; i++ ) {
		nResult += nDayCount[i];
	}
	nResult += nDay;

	return nResult;
}