#pragma once
//Monthly mission reset date (default 1st of each month)
const int DEFAULT_MONTHLY_RESET_DAY = 1;

class CDNSchedule
{
public:
	CDNSchedule( DWORD dwCycleTime = 1000 );
	virtual ~CDNSchedule();

	enum ScheduleEventEnum {
		SaveDB,
		ResetFatigue,
		ResetRebirthCoin,
		ResetDailyMission,
		ResetWeeklyMission,
		AlarmResetDailyMission,  
		AlarmResetWeeklyMission, 
		ResetWeeklyFatigue,
		ResetPCBangFatigue,
		CheckDailyQuest,
		ResetEventFatigue,
		ResetGuildWareLimit,
		ResetDailyTimeEvent,
		AlarmDailyStamp,
		ResetWeeklyStamp,
		ResetMonthlyMission,

		SchduleEventMax = 255,
		// Do not guard with #define here, just add directly; breaks when reading ext.
	};

	enum CycleTypeEnum {
		None,
		Day,
		Sunday,
		Monday,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
		Month,
	};

protected:

	struct ScheduleStruct {
		ScheduleEventEnum EventType;
		char cHour;
		char cMinute;
		CycleTypeEnum Cycle;
		bool bUpdate;
		DWORD dwLastUpdateTick;
		__time64_t tDate;

		ScheduleStruct() : EventType((ScheduleEventEnum)-1), cHour(0), cMinute(0), Cycle(None), bUpdate(false), dwLastUpdateTick(0), tDate(0) {}
	};
	DNVector(ScheduleStruct) m_VecScheduleList;
	DWORD m_dwPrevTime;
	DWORD m_dwCycleTime;

	int GetDateValue( int nYear, int nMonth, int nDay );

	bool m_bInitialize;
public:
	void CheckInitializeSchedule(); // Call and check on first login.
	void RegisterSchedule( ScheduleEventEnum ScheduleEvent, __time64_t tDate );
	bool ModifySchedule( ScheduleEventEnum ScheduleEvent, __time64_t tDate );

	virtual void Process( DWORD dwCurTick );
	virtual void OnScheduleEvent( ScheduleEventEnum ScheduleEvent, ScheduleStruct *pStruct, bool bInitialize ) {}

};