// DNCashServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DNConfig.h"
#include "IniFile.h"
#include "Log.h"
#include "ExceptionReport.h"
#include "Util.h"
#include "DNIocpManager.h"
#include "DNSQLManager.h"
#include "DNConnection.h"
#include "DNLogConnection.h"
#include "DNExtManager.h"
#include "DNThreadManager.h"
#include "DNManager.h"
#include "Version.h"
#include "Stream.h"
#include "EtUIXML.h"
#include "EtResourceMng.h"
#include "DNUserDeleteManager.h"
#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#include "DNBillingConnection.h"
#if defined(PRE_FIX_DEBUGSET_INIT)
#include "DebugSet.h"
#endif

TCashConfig g_Config;

bool LoadConfig(int argc, TCHAR * argv[])
{
	memset(&g_Config, 0, sizeof(TCashConfig));
	WCHAR wszBuf[128] = { 0, }, wszStr[64] = { 0, };
	char szData[128] = { 0, };

	//Version
	_strcpy(g_Config.szVersion, _countof(g_Config.szVersion), szCashVersion, (int)strlen(szCashVersion));
	_strcpy(g_Config.szResVersion, _countof(g_Config.szResVersion), "Unknown Version", (int)strlen("Unknown Version"));

	if (!g_IniFile.Open(L"./config/DNCash.ini")){
		g_Log.Log(LogType::_FILEDBLOG, L"DNCash.ini File not Found!!\r\n");
		return false;
	}

	g_Config.nThreadMax = 8;
	{
		g_IniFile.GetValue(L"Info", L"ThreadMax", &g_Config.nThreadMax);
		g_IniFile.GetValue(L"Connection", L"AcceptPort", &g_Config.nAcceptPort);
		g_Log.Log(LogType::_FILELOG, L"AcceptPort:%d\r\n", g_Config.nAcceptPort);

		g_IniFile.GetValue(L"DB_DNMembership", L"DBIP", wszBuf);
		if (wszBuf[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.MembershipDB.szIP, sizeof(g_Config.MembershipDB.szIP), NULL, NULL);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBPort", &g_Config.MembershipDB.nPort);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBID", g_Config.MembershipDB.wszDBID);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBName", g_Config.MembershipDB.wszDBName);

		g_IniFile.GetValue(L"DB_DNWorld", L"WorldDBCount", &g_Config.nWorldDBCount);

		WCHAR wszData[128] = { 0, };
		for (int i = 0; i < g_Config.nWorldDBCount; i++){
			swprintf(wszData, L"WorldID%d", i + 1);
			g_IniFile.GetValue(L"DB_DNWorld", wszData, &g_Config.WorldDB[i].nWorldSetID);
			if (g_Config.WorldDB[i].nWorldSetID <= 0) continue;

			swprintf(wszData, L"DBIP%d", i + 1);
			g_IniFile.GetValue(L"DB_DNWorld", wszData, wszBuf);
			if (wszBuf[0] != '\0')
				WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.WorldDB[i].szIP, sizeof(g_Config.WorldDB[i].szIP), NULL, NULL);
			swprintf(wszData, L"DBPort%d", i + 1);
			g_IniFile.GetValue(L"DB_DNWorld", wszData, &g_Config.WorldDB[i].nPort);
			swprintf(wszData, L"DBID%d", i + 1);
			g_IniFile.GetValue(L"DB_DNWorld", wszData, g_Config.WorldDB[i].wszDBID);
			swprintf(wszData, L"DBName%d", i + 1);
			g_IniFile.GetValue(L"DB_DNWorld", wszData, g_Config.WorldDB[i].wszDBName);
		}

		std::vector<std::string> Tokens;

		g_IniFile.GetValue(L"Connection", L"Log", wszStr, 64);
		if (wszStr[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszStr, -1, szData, sizeof(szData), NULL, NULL);

			Tokens.clear();
			TokenizeA(szData, Tokens, ":");
			if (!Tokens.empty()){
				_strcpy(g_Config.LogInfo.szIP, _countof(g_Config.LogInfo.szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
				g_Config.LogInfo.nPort = atoi(Tokens[1].c_str());
			}
		}

		g_Log.Log(LogType::_FILELOG, L"LogInfo (Ip:%S, Port:%d) Success\r\n", g_Config.LogInfo.szIP, g_Config.LogInfo.nPort);

		// Register ResourcePath.
		WCHAR wszPath[_MAX_PATH] = { 0, };
		char szPath[_MAX_PATH] = { 0, };
		g_IniFile.GetValue( L"Resource", L"Path", wszPath );

		if (wszPath[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszPath, -1, szPath, sizeof(szPath), NULL, NULL);
			g_Config.szResourcePath = szPath;
			if( g_Config.szResourcePath.empty() ) g_Config.szResourcePath = ".";
		}
	}


	return true;
}

bool InitApp(int argc, TCHAR * argv[])
{
	WCHAR wszLogName[128];
	memset(wszLogName, 0, sizeof(wszLogName));

	swprintf(wszLogName, L"CashServer");

#if defined(_FINAL_BUILD)
	g_Log.Init(wszLogName, LOGTYPE_FILE_HOUR);
#else
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_DAY);
#endif

	if (!LoadConfig(argc, argv)){
		g_Log.Log(LogType::_FILEDBLOG, L"LoadConfig Failed\r\n");
		return false;
	}
	g_Log.SetServerID(g_Config.nManagedID);

	// Create ResourceMng
	CEtResourceMng::CreateInstance();

	// Set Path
	std::string szResource = g_Config.szResourcePath + "\\Resource";

	CEtResourceMng::GetInstance().AddResourcePath( szResource, true );

#if defined(PRE_FIX_DEBUGSET_INIT)
	CDebugSet::Initialize( CEtResourceMng::GetInstance().GetCurrentDirectory().c_str(), true );
	bool bParsingComplete = true;
	std::string strErrorString = "";
#endif
	// UI XML String File Load
	CEtUIXML::CreateInstance();
	CStream *pStream = CEtResourceMng::GetInstance().GetStream( "uistring.xml" );
#if defined(PRE_FIX_DEBUGSET_INIT)
	if( CEtUIXML::GetInstance().Initialize( pStream, CEtUIXML::idCategory1 ) == false )
	{
		bParsingComplete = false;
		strErrorString.append( "[uistring.xml]" );
	}
#else
	CEtUIXML::GetInstance().Initialize( pStream, CEtUIXML::idCategory1 );
#endif		
	SAFE_DELETE( pStream );


#if defined(PRE_FIX_DEBUGSET_INIT)
	if( !bParsingComplete )
	{
		g_Log.Log(LogType::_FILELOG, L"UI XML String File Load Failed : %S\r\n", strErrorString.c_str());
		return false;
	}
#endif	//#if defined(PRE_FIX_DEBUGSET_INIT)

	g_pExtManager = new CDNExtManager;
	if (!g_pExtManager) return false;	
	if (!g_pExtManager->AllLoad())
	{
		g_Log.Log(LogType::_FILEDBLOG, L"CDNExtManager::AllLoad Fail\r\n");
		return false;
	}

	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager){
		g_Log.Log(LogType::_FILEDBLOG, L"new CDNIocpManager Fail\r\n");
		return false;
	}

	if (g_pIocpManager->Init(100) < 0){
		g_Log.Log(LogType::_FILEDBLOG, L"Iocp Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Sucess(%d)\r\n", 100);
	}

	g_pThreadManager = new CDNThreadManager;
	if (!g_pThreadManager) return false;

	if (!g_pThreadManager->Init(g_Config.nThreadMax)){
		g_Log.Log(LogType::_FILEDBLOG, L"ThreadManager Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## ThreadManager Initialize Success(%d)\r\n", g_Config.nThreadMax);
	}

	g_pLogConnection = new CDNLogConnection;
	if( !g_pLogConnection ) return false;
	g_pLogConnection->SetIp(g_Config.LogInfo.szIP);
	g_pLogConnection->SetPort(g_Config.LogInfo.nPort);

	g_pUserDeleteManager = new CDNUserDeleteManager;
	if (!g_pUserDeleteManager) return false;

	g_pIocpManager->CreateThread();

	g_pSQLManager = new CDNSQLManager;
	if (!g_pSQLManager) return false;

	if (!g_pSQLManager->CreateDB()){
		g_Log.Log(LogType::_FILEDBLOG, L"CreateDB Fail\r\n");
		return false;
	}

#ifdef PRE_ADD_LIMITED_CASHITEM
	g_pLimitedCashItemRepository = new CDNLimitedCashItemRepository;
	if (!g_pLimitedCashItemRepository) return false;

	g_pLimitedCashItemRepository->QueryGetLimitedItem();
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	g_pManager = new CDNManager;
	if (!g_pManager) return false;

	// Billing Server
	g_pBillingConnection = new CDNBillingConnection;
	if (!g_pBillingConnection) return false;

	if (g_pIocpManager->AddAcceptConnection(CONNECTIONKEY_DEFAULT, g_Config.nAcceptPort, 1000) < 0){
		g_Log.Log(LogType::_FILEDBLOG, L"AcceptPort Fail(%d)\r\n", g_Config.nAcceptPort);
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## AcceptPort (%d)\r\n", g_Config.nAcceptPort);
	}

	::srand( timeGetTime() );

	g_Log.Log(LogType::_FILEDBLOG, L"Application Initialize Success\r\n");

	return true;
}

void ClearApp()
{
	if( g_pIocpManager )
	{
		g_pIocpManager->CloseAcceptors();
		g_pIocpManager->ThreadStop();
	}
	SAFE_DELETE(g_pBillingConnection);

	SAFE_DELETE(g_pManager);
	SAFE_DELETE(g_pUserDeleteManager);
	SAFE_DELETE(g_pLogConnection);
	SAFE_DELETE(g_pThreadManager);
	SAFE_DELETE(g_pIocpManager);
	SAFE_DELETE(g_pSQLManager);
	SAFE_DELETE(g_pExtManager);
#ifdef PRE_ADD_LIMITED_CASHITEM
	SAFE_DELETE(g_pLimitedCashItemRepository);
#endif	// #ifdef PRE_ADD_LIMITED_CASHITEM
}

#ifndef _FINAL_BUILD

#endif

int _tmain(int argc, _TCHAR* argv[])
{
#ifndef _FINAL_BUILD
#endif

	setlocale(LC_ALL, "Korean");

	// Prepare exception handler
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE, MiniDumpWithFullMemory);	// Changed Singleton implementation to fix C4744 warning in Release build; do not inline CExceptionReport::GetInstancePtr() (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}

	if (!InitApp(argc, argv)){
		g_Log.Log(LogType::_FILEDBLOG, L"** InitApp Failed!!! Check!!!!!\r\n");
		ClearApp();
		return 0;
	}

	wprintf(L"Type 'exit' to shut down.\r\n");

	char szCmd[256] = {0};	
	while (1)
	{
		if (strcmp(szCmd, "exit") == 0)	break;

		if (strcmp(szCmd, "prohibitsale") == 0){
			g_pExtManager->SetProhibitSaleList();
		}

		printf("CMD>");
		(fgets(szCmd, sizeof(szCmd), stdin) ? (szCmd[strcspn(szCmd, "\r\n")] = '\0') : (szCmd[0] = '\0'));
	}	

	ClearApp();
	return 0;
}
