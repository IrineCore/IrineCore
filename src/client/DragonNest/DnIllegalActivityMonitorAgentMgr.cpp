#include "StdAfx.h"
#include <time.h>
#include "DnIllegalActivityMonitorAgentMgr.h"
#include "DnItemTask.h"
#include "DnBridgeTask.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

#ifdef PRE_MONITOR_SUPER_NOTE

CDnHTTPReporter::CDnHTTPReporter()
{
	try
	{
		m_HttpClient.SetInternet("IllegaActivityMonitor Agent");

	#if defined(_DEBUG) || defined(_RDEBUG)
		m_ReportHttpInfo.szServerURL = "http://192.168.0.22:8088/BugTrace.aspx";
	#else
		m_ReportHttpInfo.szViewServerURL = "";//"http://211.39.129.30/Bugreport/ReportList.aspx";
		m_ReportHttpInfo.szServerURL = "http://211.39.159.9/BugTrace.aspx";
	#endif // _DEBUG
	}
	catch (httpclientexceptionA&)
	{
		m_ReportHttpInfo.szViewServerURL = "";
		m_ReportHttpInfo.szServerURL = "";
		return;
	}
}

long CDnHTTPReporter::HttpReporting()
{
//#ifdef _FINAL_BUILD
	if (m_szReportFileName.empty() || m_ReportHttpInfo.szServerURL.empty())
		return -1;

	CHttpResponse *	pResponse = NULL;
	const DWORD		dwStep = 1024;

	std::string szName1("textfile10");

	try
	{
		m_HttpClient.AddParam(szName1.c_str(), m_szReportFileName.c_str(), CHttpClient::ParamFile);
		m_HttpClient.SetUseUtf8(TRUE);
		m_HttpClient.BeginUpload(m_ReportHttpInfo.szServerURL.c_str());

		DWORD nCnt = 0;
		while(pResponse == NULL)
		{
			pResponse = (CHttpResponse*)m_HttpClient.Proceed(dwStep);
			nCnt++;

			if (nCnt > m_ReportHttpInfo.nMaxReportSize)
				break;
		}
	}
	catch (httpclientexceptionA&)
	{
		SAFE_DELETE(pResponse);
		m_HttpClient.Cancel();

		return -1;
	}

	SAFE_DELETE(pResponse);
//#endif // _FINAL_BUILD

	return 0;
}

//------------------------------------------------------------------------

CDnIllegalActivityMonitorAgentMgr::CDnIllegalActivityMonitorAgentMgr()
{
}

CDnIllegalActivityMonitorAgentMgr::~CDnIllegalActivityMonitorAgentMgr()
{
	SAFE_DELETE_PVEC( m_Activities );
}

void CDnIllegalActivityMonitorAgentMgr::AddCheckMonitor(eIAMA_CheckType type, float checkTerm)
{
	if (checkTerm <= 0.f)
		return;

	if (IsMonitorInList(type))
		return;

	CDnIllegalActivityMonitorAgent* pAgent = NULL;
	if (type == eIAMA_MONEY)	pAgent = new CDnIAMMoneyAgent(&m_HTTPReporter);

	pAgent->OnAdd(type, checkTerm);
	m_Activities.push_back(pAgent);
}

bool CDnIllegalActivityMonitorAgentMgr::IsMonitorInList(eIAMA_CheckType type) const
{
	std::vector<CDnIllegalActivityMonitorAgent*>::const_iterator iter = m_Activities.begin();
	for (; iter != m_Activities.end(); ++iter)
	{
		const CDnIllegalActivityMonitorAgent* pAgent = *iter;
		if (pAgent)
		{
			if (pAgent->GetType() == type)
				return true;
		}
	}

	return false;
}

void CDnIllegalActivityMonitorAgentMgr::DelCheckMonitor(eIAMA_CheckType type)
{
	std::vector<CDnIllegalActivityMonitorAgent*>::iterator iter = m_Activities.begin();
	for (; iter != m_Activities.end(); ++iter)
	{
		CDnIllegalActivityMonitorAgent* pAgent = *iter;
		if (pAgent)
		{
			if (pAgent->GetType() == type)
			{
				pAgent->OnTerminate();
				delete pAgent;
				m_Activities.erase(iter);
				return;
			}
		}
	}
}

void CDnIllegalActivityMonitorAgentMgr::Process(float fElapsedTime)
{
	std::vector<CDnIllegalActivityMonitorAgent*>::iterator iter = m_Activities.begin();
	for (; iter != m_Activities.end(); ++iter)
	{
		CDnIllegalActivityMonitorAgent* pAgent = *iter;
		if (pAgent)
			pAgent->Process(fElapsedTime);
	}
}

//-------------------------------------------------------------------------

bool CDnIAMMoneyAgent::OnAdd(eIAMA_CheckType type, float checkTerm)
{
	if (CDnItemTask::IsActive() == false)
		return false;

	m_InitialCoin = CDnItemTask::GetInstance().GetCoin();

	return CDnIllegalActivityMonitorAgent::OnAdd(type, checkTerm);
}

#define ABNORMAL_INCREASE_COIN_VALUE	1

bool CDnIAMMoneyAgent::Process(float fElapsedTime)
{
	m_fCurrentTerm += fElapsedTime;
	if (m_fCurrentTerm >= m_fCheckTerm)
	{
		if (CDnItemTask::IsActive() == false)
			return false;

		if (!CDnActor::s_hLocalActor)
			return false;

		CDnLocalPlayerActor *pActor = ((CDnLocalPlayerActor *)CDnActor::s_hLocalActor.GetPointer());
		if (pActor == NULL)
			return false;

		int nIllegalAmount	= CPlayerLevelTable::GetInstance().GetValue(pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::IllegalAmount);
		int nBanAmount		= CPlayerLevelTable::GetInstance().GetValue(pActor->GetJobClassID(), pActor->GetLevel(), CPlayerLevelTable::BanAmount);
		INT64 currentCoin = CDnItemTask::GetInstance().GetCoin();

		bool bBan = false;
		bool bIllegal = false;
		if (currentCoin - m_InitialCoin < nBanAmount)
		{
			bIllegal = (currentCoin - m_InitialCoin >= nIllegalAmount);
		}
		else
		{
			bBan = true;
		}

		if (bIllegal || bBan)
		{
			if( CDnBridgeTask::IsActive() ) {
				struct tm* now=NULL;
				time_t systemTime;
				time(&systemTime);
				now = localtime(&systemTime);

				CHAR szTail[MAX_PATH] = {0,};
				sprintf(szTail, "%u_%04d-%02d-%02d_%02d-%02d-%02d", CDnBridgeTask::GetInstance().GetSessionID(),
					1900+now->tm_year, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
				static const unsigned short UNICODE_TAG = 0xFEFF;

				std::wstring wszReport;
				wszReport = FormatW(L"[DATE] %04d/%02d/%02d %02d:%02d:%02d\n[CHRNAME] %s\n[RESTRICT_TYPE] %s\n[INIT_COIN] %I64d\n[CURRENT_COIN] %I64d\n[LEVEL] %d\n[BAN_AMOUNT] %d\n[ILLEGAL_AMOUNT] %d", 1900+now->tm_year, now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, 
					pActor->GetName(), bBan ? L"BAN_AMOUNT" : ((bIllegal) ? L"ILLEGAL_AMOUNT" : L"NONE"),
					m_InitialCoin, currentCoin, pActor->GetLevel(), nBanAmount, nIllegalAmount);

				TCHAR wszFileName[MAX_PATH] = {0, };
				MultiByteToWideChar(CP_ACP, 0, szTail, -1, wszFileName, MAX_PATH);
				FILE* pFile = _wfopen(wszFileName, L"wb");
				if( pFile == NULL ) return false;
				fwrite(&UNICODE_TAG, sizeof(unsigned short), 1, pFile);
				fwprintf(pFile, wszReport.c_str());
				//fwrite(wszReport.c_str(), 2, wszReport.size(), pFile );
				fclose(pFile);

				m_pHTTPReporter->SetReportFileName(std::string(szTail));
				m_pHTTPReporter->HttpReporting();

				DeleteFileA(szTail);

				m_InitialCoin = currentCoin;
			}
		}
		m_fCurrentTerm = 0.f;
	}

	return CDnIllegalActivityMonitorAgent::Process(fElapsedTime);
}

#endif // PRE_MONITOR_SUPER_NOTE