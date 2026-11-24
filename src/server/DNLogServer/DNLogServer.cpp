// DNDBServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerConsole.h"
#include "IniFile.h"
#include "DNIocpManager.h"
#include "DNThreadManager.h"
#include "DNSQLConnectionManager.h"
#include "DNConfig.h"
#include "Log.h"
#include "ExceptionReport.h"
#include "Version.h"
#include "DNConnectionManager.h"

TLogConfig g_Config;

bool LoadConfig(int argc, TCHAR * argv[])
{
	memset(&g_Config, 0, sizeof(TLogConfig));
	WCHAR wszBuf[128] = { 0, };

	//Version
	strcpy_s(g_Config.szVersion, szLogVersion);
	strcpy_s(g_Config.szResVersion, "Unknown Version");

	g_Config.nThreadMax = THREADMAX;
	{
		if (!g_IniFile.Open(L"./config/DNLog.ini")){
			g_Log.Log(LogType::_FILELOG, L"DNLog.ini File not Found!!\r\n");
			return false;
		}

		g_IniFile.GetValue(L"Info", L"ThreadMax", &g_Config.nThreadMax);
		g_IniFile.GetValue(L"Connection", L"AcceptPort", &g_Config.nAcceptPort);

		WCHAR wszBuf[128] = { 0, };

		g_IniFile.GetValue(L"MasterLogDB", L"DBIP", wszBuf);
		WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.MasterLogDB.szIP, sizeof(g_Config.MasterLogDB.szIP), NULL, NULL);
		g_IniFile.GetValue(L"MasterLogDB", L"DBPort", &g_Config.MasterLogDB.nPort);
		g_IniFile.GetValue(L"MasterLogDB", L"DBID", g_Config.MasterLogDB.wszDBID);
		g_IniFile.GetValue(L"MasterLogDB", L"DBName", g_Config.MasterLogDB.wszDBName);

		g_IniFile.GetValue(L"ServerLogDB", L"DBIP", wszBuf);
		WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.ServerLogDB.szIP, sizeof(g_Config.ServerLogDB.szIP), NULL, NULL);
		g_IniFile.GetValue(L"ServerLogDB", L"DBPort", &g_Config.ServerLogDB.nPort);
		g_IniFile.GetValue(L"ServerLogDB", L"DBID", g_Config.ServerLogDB.wszDBID);
		g_IniFile.GetValue(L"ServerLogDB", L"DBName", g_Config.ServerLogDB.wszDBName);

		g_IniFile.GetValue(L"LogDB", L"LogDBCount", &g_Config.nLogDBCount);

		int nWorldID = 0;
		WCHAR wszData[128] = { 0, };
		for (int i = 0; i < g_Config.nLogDBCount; i++){
			swprintf(wszData, L"Log%dWorldID", i + 1);
			g_IniFile.GetValue(L"LogDB", wszData, &nWorldID);
			if (nWorldID <= 0) continue;

			g_Config.LogDB[i].nWorldSetID = nWorldID;
			swprintf(wszData, L"DBIP%d", i + 1);
			g_IniFile.GetValue(L"LogDB", wszData, wszBuf);
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.LogDB[i].szIP, sizeof(g_Config.LogDB[i].szIP), NULL, NULL);
			swprintf(wszData, L"DBPort%d", i + 1);
			g_IniFile.GetValue(L"LogDB", wszData, &g_Config.LogDB[i].nPort);
			swprintf(wszData, L"Log%dDBID", i + 1);
			g_IniFile.GetValue(L"LogDB", wszData, g_Config.LogDB[i].wszDBID);
			swprintf(wszData, L"Log%dDBName", i + 1);
			g_IniFile.GetValue(L"LogDB", wszData, g_Config.LogDB[i].wszDBName);
		}
	}
	
	if (g_Config.nThreadMax > THREADMAX) g_Config.nThreadMax = THREADMAX;

	return true;
}

bool CheckWorkingFolder(const WCHAR* szFolderName)
{
	WCHAR szCurrentDir[MAX_PATH] = L"";
	GetCurrentDirectory(MAX_PATH, szCurrentDir);
	std::wstring wszCurrentDir;
	wszCurrentDir = szCurrentDir;
	std::transform(wszCurrentDir.begin(), wszCurrentDir.end(), wszCurrentDir.begin(), towlower); 
	if ( wszCurrentDir.find(szFolderName) > wszCurrentDir.size() )
		return false;

	return true;
}

bool InitApp(int argc, TCHAR * argv[])
{
	//if (!CheckSerial()) return false;


	WCHAR wszLogName[128];
	memset(wszLogName, 0, sizeof(wszLogName));

	swprintf(wszLogName, L"LogServer");
	
	//g_Log.Init(L"LogServer", LOGTYPE_CRT_FILE_DAY);
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_DAY);	

	if (!LoadConfig(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"LoadConfig Failed\r\n");
		return false;
	}
	g_Log.SetServerID(g_Config.nManagedID);

	g_pConnectionManager = new CDNConnectionManager;
	if (!g_pConnectionManager) return false;

	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager) return false;

	if (g_pIocpManager->Init(100) < 0){
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Fail(%d)\r\n", 100);
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## Iocp Initialize Success(%d)\r\n", 100);
	}

	if (!g_ThreadManager.Init(g_Config.nThreadMax)){
		g_Log.Log(LogType::_FILELOG, L"ThreadManager Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## ThreadManager Initialize Success(%d)\r\n", g_Config.nThreadMax);
	}

	if (!g_SQLConnectionManager.CreateDB()){
		g_Log.Log(LogType::_FILELOG, L"CreateDB Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## CreateDB OK!!\r\n");
	}

	if (g_pIocpManager->AddAcceptConnection(CONNECTIONKEY_DEFAULT, g_Config.nAcceptPort, 1000) < 0){
		g_Log.Log(LogType::_FILELOG, L"AcceptPort Fail(%d)\r\n", g_Config.nAcceptPort);
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## AcceptPort (%d)\r\n", g_Config.nAcceptPort);
	}	
	g_pIocpManager->CreateUpdateThread();

	g_Log.Log(LogType::_FILEDBLOG, L"Application Initialize Success\r\n");
	return true;
}

void ClearApp()
{

	if(g_pIocpManager)
	{
		g_pIocpManager->CloseAcceptors();
		g_pIocpManager->ThreadStop();
	}
	SAFE_DELETE(g_pIocpManager);
}

int _tmain(int argc, TCHAR* argv[])
{
	setlocale(LC_ALL, "Korean");

	// Prepare exception handler
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE);	// Changed Singleton implementation to fix C4744 warning in Release build; do not inline CExceptionReport::GetInstancePtr() (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}

	if (!InitApp(argc, argv)){
		ClearApp();
		return 0;
	}

	wprintf(L"Type 'exit' to shut down.\r\n");

	char szCmd[256] = {0};	
	while (1)
	{

		if (strcmp(szCmd, "exit") == 0)	break;		

		if (!Irine::ServerConsole::ReadCommand(szCmd, sizeof(szCmd))) break;
	}

	ClearApp();

	return 0;
}