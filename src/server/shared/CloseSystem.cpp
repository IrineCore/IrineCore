
#include "stdafx.h"
#include "CloseSystem.h"

CCloseSystem * g_pCloseSystem = NULL;

CCloseSystem::CCloseSystem()
{
	m_tOderedTime = 0;
	m_tCloseTime = 0;
}

CCloseSystem::~CCloseSystem()
{
}

bool CCloseSystem::AddClose(__time64_t _tOderedTime, __time64_t _tCloseTime)
{
	ScopeLock <CSyncLock> Lock(m_Sync);

	m_tOderedTime = _tOderedTime;
	m_tCloseTime = _tCloseTime;
	return true;
}

bool CCloseSystem::CancelClose()
{
	ScopeLock <CSyncLock> Lock(m_Sync);

	if (m_tOderedTime <= 0)
		return false;

	m_tOderedTime = 0;
	m_tCloseTime = 0;
	return true;
}

bool CCloseSystem::IsClosing(__time64_t &_tOderedTime, __time64_t &_tCloseTime)
{
	ScopeLock <CSyncLock> Lock(m_Sync);
	if (m_tCloseTime <= 0)
		return false;

	_tOderedTime = m_tOderedTime;
	_tCloseTime = m_tCloseTime;
	return true;
}