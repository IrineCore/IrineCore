#pragma once
/*
Roughly estimate GPU performance.
Used to auto-configure graphics options (High, Medium, Low) on first launch.
*/
#include "Singleton.h"
class CEtBenchMark : public CSingleton<CEtBenchMark>
{
public:
	enum PerfGrade {
		PF_Unknown = -1,
		PF_Low,		
		PF_Middle,
		PF_High,
	};

public:
	CEtBenchMark();
	~CEtBenchMark();

private:
	PerfGrade		m_PerfGrade;
	DWORD			m_dwScore;

	bool				m_bRun;

public:
	void	Run();
	void Process();
	void Reset();
	PerfGrade GetResult( DWORD *pScore = NULL );
};
