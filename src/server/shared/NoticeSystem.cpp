
#include "stdafx.h"
#include "NoticeSystem.h"
#include "DNGameDataManager.h"

CDNNoticeSystem * g_pNoticeSystem = NULL;

CDNNoticeSystem::CDNNoticeSystem()
{
}

CDNNoticeSystem::~CDNNoticeSystem()
{
}

bool CDNNoticeSystem::AddNotice(const TNoticeTypeInfo * pInfo, const WCHAR * pNoticeMsg)
{
	if (pInfo == NULL || pNoticeMsg == NULL) return false;
	if (pInfo->nSlideShowSec <= 0) return false;
	if (CheckNotice(pInfo) == false) return false;

	TNoticeInfo Info;
	memset(&Info, 0, sizeof(Info));
	
	memcpy(&Info.TypeInfo, pInfo, sizeof(TNoticeTypeInfo));
	_wcscpy(Info.wszMsg, _countof(Info.wszMsg), pNoticeMsg, (int)wcslen(pNoticeMsg));
	Info.nCreateTime = timeGetTime();
	Info.nDestroyTime = Info.nCreateTime + (pInfo->nSlideShowSec * 1000);

	ScopeLock <CSyncLock> Lock(m_Sync);
	m_pNoticeInfoList.push_back(Info);
	return true;
}

bool CDNNoticeSystem::CancelNotice()
{
	ScopeLock <CSyncLock> Lock(m_Sync);

	if (m_pNoticeInfoList.empty())
		return false;

	m_pNoticeInfoList.clear();
	return true;
}

bool CDNNoticeSystem::GetNotice(int nChannelID, int nMapIdx, TNoticeInfo & Info)
{
	ULONG nCurTick = timeGetTime();

	ScopeLock <CSyncLock> Lock(m_Sync);

	std::list <TNoticeInfo>::iterator ii;
	for (ii = m_pNoticeInfoList.begin(); ii != m_pNoticeInfoList.end(); )
	{
		if ((*ii).nDestroyTime > nCurTick)
		{
			(*ii).TypeInfo.nSlideShowSec = (int)((*ii).nDestroyTime - nCurTick)/1000;

			switch ((*ii).TypeInfo.nNoticeType)
			{
			case _NOTICETYPE_WORLD:
			case _NOTICETYPE_EACHSERVER:
				{
					Info = (*ii);
					return true;
				}
				break;
			case _NOTICETYPE_CHANNEL:
				if ((*ii).TypeInfo.nChannelID == nChannelID)
				{
					Info = (*ii);
					return true;
				}
				break;
			case _NOTICETYPE_ZONE:
				if ((*ii).TypeInfo.nMapIdx == nMapIdx)
				{
					Info = (*ii);
					return true;
				}
				break;
			
			default:
				return false;
			}
			ii++;
		}
		else
			ii = m_pNoticeInfoList.erase(ii);
	}
	return false;
}

bool CDNNoticeSystem::CheckNotice(const TNoticeTypeInfo * pInfo)
{
	ScopeLock <CSyncLock> Lock(m_Sync);

	std::list <TNoticeInfo>::iterator ii;
	for (ii = m_pNoticeInfoList.begin(); ii != m_pNoticeInfoList.end(); ii++)
	{
		if ((*ii).nDestroyTime > timeGetTime())
		{
			switch ((*ii).TypeInfo.nNoticeType)
			{
			case _NOTICETYPE_WORLD:
			case _NOTICETYPE_EACHSERVER: return false;
			case _NOTICETYPE_ZONE:
			case _NOTICETYPE_CHANNEL:
				{
					if (pInfo->nNoticeType == _NOTICETYPE_WORLD || pInfo->nNoticeType == _NOTICETYPE_EACHSERVER) return false;
					if (pInfo->nNoticeType == _NOTICETYPE_ZONE && (*ii).TypeInfo.nMapIdx == pInfo->nMapIdx) return false;
					if (pInfo->nNoticeType == _NOTICETYPE_CHANNEL && ((*ii).TypeInfo.nMapIdx == pInfo->nMapIdx ||
						(*ii).TypeInfo.nChannelID == pInfo->nChannelID)) return false;
				}
				break;
			}
		}
	}
	return true;
}