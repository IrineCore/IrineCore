#include "StdAfx.h"
#include "SyncTimer.h"
#include <mmsystem.h>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CSyncTimer::CSyncTimer()
{
	m_bStarted = false;
	m_tCurTime = 0;
	m_dwBasisTime = 0;
}

CSyncTimer::~CSyncTimer()
{
}

void CSyncTimer::SetServerTime( __time64_t _tStartTime )
{
	m_tCurTime = _tStartTime;
	m_dwBasisTime = timeGetTime();
	m_bStarted = true;
}

void CSyncTimer::Process()
{
	if( !m_bStarted ) return;

	DWORD dwDelta = timeGetTime() - m_dwBasisTime;
	if( dwDelta >= 1000 ) {

		// In case of a frame spike,
		DWORD dwSec = dwDelta / 1000;

		// Minimum unit is seconds
		m_tCurTime += dwSec;

		// Update reference time
		m_dwBasisTime += (dwSec * 1000);
	}
}

__time64_t CSyncTimer::GetCurTime()
{
	if( !m_bStarted ) {
		_ASSERT(0&&"Attempting to use SyncTimer without initializing it.");
		return 0;
	}
	return m_tCurTime;
}