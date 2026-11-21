#pragma once

#include "Task.h"

class CDnGameTipTask : public CTask, public CSingleton<CDnGameTipTask>
{
	enum
	{
		EXCEPTION_DELAY_TIME = 5,	// If something seems wrong, re-process after 5 seconds.
		DELAY_TIME = 40,			// Default delay is once every 40 seconds.

		MIN_NO_BATTLE = 1301000,	// Check non-combat message range with <=.
		MAX_NO_BATTLE = 1303000,
	};

	enum
	{
		GRADE_ALL = 0,
		GRADE_NEW,
		GRADE_BEGINNER,
		GRADE_INTERMEDIATE,
	};

	enum
	{
		LEVEL_ALL = 0,
		LEVEL_NEW = 10,
		LEVEL_BEGINNER = 23
	};

	enum
	{
		OutputType_ALL = 0,
		OutputType_Custom = 1,
	};

	enum{
		MAP_COUNT = 5,
	};

	struct STipData
	{
		int  nStringID;
		std::vector<int> vMapIDs;

		STipData() : nStringID( -1 )
		{}
	};

public:
	CDnGameTipTask();
	virtual ~CDnGameTipTask();

	bool Initialize();
	void Finalize();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void ResetGameTip( float fCurDelayTime = 2.0f );	// Better than showing immediately upon entry.

	int GetTodayTipStringID( int nMapID, int nLevel );
	int GetCustomStringID( int nMapID );

protected:
	std::vector<int> m_vecNoBattleStringID;
	std::vector<int> m_vecBattleStringID;

	std::vector<STipData> m_vecAll;				// All levels
	std::vector<STipData> m_vecNew;				// Tutorial
	std::vector<STipData> m_vecBeginner;		// Beginner
	std::vector<STipData> m_vecIntermediate;	// Intermediate
	std::vector<STipData> m_vecCustom;			// Custom map

	float m_fDelayTime;
};

#define GetGameTipTask()	CDnGameTipTask::GetInstance()