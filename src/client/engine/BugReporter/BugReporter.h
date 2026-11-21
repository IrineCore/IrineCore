#pragma once

#include "SystemInfo.h"
#include "RyeolHttpClient.h"
#include "AssertX.h"
#include "CriticalSection.h"
#include <vector>

using namespace Ryeol;

class ExceptionCallback
{
public:
	virtual void OnException(_EXCEPTION_POINTERS* pExceptionInfo) { }
};
/*
	�� ���� �������ϰ� ���׸����� ������ ����� ������ �������ִ� Ŭ���� 
*/
class BugReporter
{
public:
	enum DumpLevel
	{
		eMiniDumpNormal = 0,
		eMiniDumpWithDataSegs = 1,
		eMiniDumpWithFullMemory = 2,
	};

	struct ReportingServerInfo
	{
		ReportingServerInfo()
		{
			// kb ����
			nMaxReportSize = 20000; // �⺻�� 20�ް� ����
			szViewServerURL = "";
			szServerURL = "";
			szSIDURL = "";
			
			szNoticeMsg = L"";
			bUseAutoView = false;
			bSaveLocal = false;		// ������ �ϵ忡 ������ �α׸� ������ΰ�.
			bUseConfirm = false;
			hWnd = NULL;
			hAppIcon = 0;
			hInst = 0;

		}
		std::string szServerURL;
		std::string szViewServerURL;
		std::string szSIDURL;
		std::wstring szAppName;
		std::wstring szNoticeTitle;
		std::wstring szNoticeMsg;
		std::wstring szSendButtonStr;
		std::wstring szNotSendButtonStr;
		HICON hAppIcon;
		
		DWORD		nMaxReportSize;
		bool		bUseAutoView; // ������ ����Ʈ�� ��������
		bool		bSaveLocal;	// �������� �ϵ忡 �����Ұ�����
		bool		bUseConfirm; // ����� ����Ұ����� �ɼ�
		HWND		hWnd;
		HINSTANCE	hInst;
		bool		bEnableReporting;	// ������ ����
	};

	BugReporter();
	virtual ~BugReporter();

	// ���׸������� url
	void SetReportingServerInfo(ReportingServerInfo& info)
	{ 
		m_ReportHttpInfo = info;
	}

	ReportingServerInfo& GetReportingServerInfo() { return m_ReportHttpInfo; }

	// ������ �����ϱ����� ���ڿ� 
	void SetBuildVersion(std::string& szBuildVersion) { m_szBuildVersion = szBuildVersion; }
	
	/*
		Ŀ���� ����� ������ ����Ʈ�� �߰��ϱ� ���ؼ� ����Ѵ�.
		ex ) 
		std::string g_szUserNickname;
		BugReporter.AddDebugData(&g_szUserNickname); // �����͸� ����Ѵ�.

		g_szUserNickname �� �α��� �Ϸ� ���� ��Ŷ�� ������ �����г����� ����صд�.  UserNick = [����] UserGuild = [���ν�Ʈ���] 

		���� �߻��� �����õ� ���Ͽ� UserDebugData �׸� ���� �ش� ������ Ȯ���Ҽ� �ִ�.
	*/
	void AddDebugData(std::string* pStr) { m_UserDataArray.push_back(pStr); }

	// �����̶���� ��Ÿ �߰������� Ȯ���� �α׸� �־�д�. �ʹ� ���� �α׸� ���� ������ ������ �ʹ� Ŀ���Ƿ� �� �ʿ��ϰ� �߿��� ���븸 �α׷� �߰��Ѵ�.
	void AddLogA(char* fmt, ... );

	void AttachCurrentTimeLog();

	void AddLogW(WCHAR* fmt, ... );

	void Enable(DumpLevel level = eMiniDumpWithDataSegs );
	void Disable();

	static DWORD WINAPI _StackOverflowException( LPVOID pParam );
	static  LONG	__stdcall Exception_Minidump(_EXCEPTION_POINTERS* pExceptionInfo);
	static  void	__cdecl	  DefaultBadAllocFilter();

	long BugReport(_EXCEPTION_POINTERS* pExceptionInfo);
	long MakeBugReportText(_EXCEPTION_POINTERS* pExceptionInfo);
	long MakeDumpFile(_EXCEPTION_POINTERS* pExceptionInfo);
	long HttpReporting();
	long SIDReporting();
	void SetSID( char *szSID );
	void EnableCompressDump( bool bEnable ) { m_bEnableCompress = bEnable; }

	void ConfirmDialog();
	// �ͼ����� �� ��Ȳ�� �ƴ����� �׳� ������ �α׸� �޾ƺ��� ���ؼ� ������ �ϴ� ����̴�.
	long ForceReporting();

	void SetExceptionCallBack(ExceptionCallback* pCallback) { m_pExceptionCallback = pCallback; }
private:


private:
	int				m_nDumpLevel;
	bool			m_bEnableDump;
	bool			m_bEnableCompress;

	// �α׸� ���ܼ� ������ ����� �߰��ϸ鼭,, ���� �����忡�� �α׸� ���� �� �����Ƿ� ����ȭ ó���� ��.
	// ������ �������ϴ� �κе�(BugReport()) ���Ŀ� ���� �����忡�� �� �� �ְ� �ȴٸ� ����ȭ ó���� �ؾ��Ѵ�.
	CSyncLock		m_LogLock;
	bool m_bForceLogReporting;

	static  LPTOP_LEVEL_EXCEPTION_FILTER m_pExceptionFilter;
	static  _invalid_parameter_handler m_pInvalidParameterHandler;
	static  _purecall_handler m_pPureCallHandler;

	SystemInfo m_SystemInfo;

	std::string m_szProcessName;
	std::string m_szBuildVersion;
	std::string m_szUserLog;
	std::string m_szReport;
	std::string m_szStartTime;	
	std::string m_szReportingTime;	
	std::string m_szFileTime;	
	std::string m_szDumpFileName;
	std::string m_szCompressDumpFileName;
	std::string m_szReportFileName;
	std::vector<std::string*>	m_UserDataArray;

	ReportingServerInfo m_ReportHttpInfo;
	CHttpClientA m_HttpClient;
	CHttpClientA m_HttpClientForSID;

	ExceptionCallback*	m_pExceptionCallback;
	char			m_szSID[256];

};

extern BugReporter gs_BugReporter;