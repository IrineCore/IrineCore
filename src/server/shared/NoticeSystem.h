
#pragma once
//When there is something to notify users who are in or enter a certain area for a certain duration

struct TNoticeInfo
{
	TNoticeTypeInfo TypeInfo;	//apply region
	WCHAR wszMsg[CHATLENMAX];	//message
	ULONG nCreateTime;
	ULONG nDestroyTime;
};

class CDNNoticeSystem
{
public:
	CDNNoticeSystem();
	~CDNNoticeSystem();

	bool AddNotice(const TNoticeTypeInfo * pInfo, const WCHAR * pNoticeMsg);
	bool CancelNotice();

	bool GetNotice(int nChannelID, int nMapIdx, TNoticeInfo & Info);

protected:
	CSyncLock m_Sync;
	std::list <TNoticeInfo> m_pNoticeInfoList;

	bool CheckNotice(const TNoticeTypeInfo * pInfo);
};

extern CDNNoticeSystem * g_pNoticeSystem;