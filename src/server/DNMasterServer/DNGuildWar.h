
#pragma once

// Application period -> Preliminaries -> Rewards (including finals) -> End
class CDNGuildWar
{

public:
	CDNGuildWar();
	virtual ~CDNGuildWar();

	virtual void Reset();
	virtual void Process();
	virtual void OnStartEvent();
	virtual void OnEndEvent();
	
	bool UpdateTime(short wScheduleID, time_t tStart, time_t tEnd, short wWinersWeightRate);
	bool IsValidPeriod();
	bool IsFinishPeriod();
	
	time_t GetEventTime(char cType);
	bool IsEventComplete(char cType);
	void SetEventComplete(char cType, bool bComplete);
	inline void SetDBJobSend(bool bSend) { m_bDBJobSend = bSend; };	
	inline void SetDBJobSeq(int nJobSeq) { m_nDBJobSeq = nJobSeq; };
	inline void SetDBJobSuccess(bool bSuccess) { m_bDBJobSuccess = bSuccess; };	
	inline bool GetDBJobSuccess() { return m_bDBJobSuccess; };

private:
	bool CheckEventType(char cType);

protected:
	time_t m_tEventMoment[GUILDWAR_EVENT_MAX];
	time_t m_tCurrent;

	bool m_bEventComplete[GUILDWAR_EVENT_MAX];

	char m_cEventStep;
	short m_wScheduleID;
	short m_wWinersWeightRate;

	int m_nDBJobSeq;												// DBJob Seq
	DWORD m_nDBJobSendTick;											// DBJob Tick
	bool m_bDBJobSend;
	bool m_bDBJobSuccess;
};

// Application period
class CDNGuildWarPreparation : public CDNGuildWar
{
public:
	CDNGuildWarPreparation();
	~CDNGuildWarPreparation();

	void Process();
};

// Preliminary period
class CDNGuildWarTrial : public CDNGuildWar
{
public:
	CDNGuildWarTrial();
	~CDNGuildWarTrial();

	void Process();	

	DWORD m_nTrialPointSendTick;									// Preliminary ranking update tick
};

// Finals, reward period
class CDNGuildWarReward : public CDNGuildWar
{
private:
	time_t m_tFinalsStartMoment[GUILDWAR_FINALPART_MAX];			// Start time for Round of 16, Quarterfinals, Semifinals, and Finals (0 is discarded)
	time_t m_tFinalsEndMoment[GUILDWAR_FINALPART_MAX];				// End time for Round of 16, Quarterfinals, Semifinals, and Finals
	bool m_bFinalsComplete[GUILDWAR_FINALPART_MAX];					// Completion status per round
	char m_cCurrentFinals;											// Current round	
	bool m_bFinalSchedule;											// Whether finals schedule is set		
public:
	CDNGuildWarReward();
	~CDNGuildWarReward();

	bool m_bTrialStatsRequest;										// Aggregate preliminary results

	virtual void OnStartEvent();
	virtual void Reset();

	void Process();
	void SetFinalScheduleInfo(MASetGuildWarFinalSchedule* pData);
	char GetCurFinalPart();
	time_t GetCurFinalPartBeginTime();	
};
