#pragma once

#include "DnBlow.h"

// Currently shared between game server and client via #define, though this may change later.
class CDnActorStatIntervalManipulator
{
private:
	DnBlowHandle			m_hParentBlow;
	DnActorHandle			m_hActor;

	LOCAL_TIME				m_LocalStartTime;
	LOCAL_TIME				m_LocalElapsedTime;

	DWORD					m_dwInterval;
	LOCAL_TIME				m_LastIntervalTime;

private:
	// Please make sure to construct using the constructor below that takes arguments.
	CDnActorStatIntervalManipulator( void ) {};

	//void _ProcessInterval( int& iProcessManipulateCount, int iDestProcessCount );

public:
	CDnActorStatIntervalManipulator( DnActorHandle hActor, DnBlowHandle hParentBlow );
	~CDnActorStatIntervalManipulator(void);

	void OnBegin( LOCAL_TIME LocalTime, DWORD dwInterval );
	void Process( LOCAL_TIME LocalTime, float fDelta );
	void OnEnd( LOCAL_TIME LocalTime, float fDelta );
};
