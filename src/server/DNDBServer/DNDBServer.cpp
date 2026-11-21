// DNDBServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "IniFile.h"
#include "DNIocpManager.h"
#include "DNThreadManager.h"
#include "DNSQLConnectionManager.h"
#include "DNConfig.h"
#include "Log.h"
#include "ExceptionReport.h"
#include "Version.h"
#include "Util.h"
#include "DNLogConnection.h"
#include "DNExtManager.h"
#include "EtResourceMng.h"
#include "DNServerPacketDB.h"
#include "DNMessageTask.h"
TDBConfig g_Config;

bool LoadConfig(int argc, TCHAR * argv[])
{
	memset(&g_Config, 0, sizeof(TDBConfig));
	WCHAR wszBuf[128] = { 0, };
	
	//Version
	_strcpy(g_Config.szVersion, _countof(g_Config.szVersion), szDBVersion, (int)strlen(szDBVersion));
	_strcpy(g_Config.szResVersion, _countof(g_Config.szResVersion), "Unknown Version", (int)strlen("Unknown Version"));

	g_Config.nThreadMax = THREADMAX;
	{
		if (!g_IniFile.Open(L"./config/DNDB.ini")){
			g_Log.Log(LogType::_FILELOG, L"DNDB.ini File not Found!!\r\n");
			return false;
		}
		g_IniFile.GetValue(L"Info", L"ThreadMax", &g_Config.nThreadMax);
		g_IniFile.GetValue(L"Connection", L"AcceptPort", &g_Config.nAcceptPort);

		WCHAR wszBuf[128] = { 0, };
		
		g_IniFile.GetValue(L"DB_DNMembership", L"DBIP", wszBuf);
		if (wszBuf[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.MembershipDB.szIP, sizeof(g_Config.MembershipDB.szIP), NULL, NULL);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBPort", &g_Config.MembershipDB.nPort);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBID", g_Config.MembershipDB.wszDBID);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBName", g_Config.MembershipDB.wszDBName);

		g_IniFile.GetValue(L"DB_DNWorld", L"WorldDBCount", &g_Config.nWorldDBCount);

#if defined( PRE_WORLDCOMBINE_PARTY )
		g_IniFile.GetValue(L"DB_DNWorld", L"CombineWorldDB", &g_Config.nCombineWorldDBID);
		if( g_Config.nCombineWorldDBID <= 0 )
			g_Log.Log(LogType::_FILELOG, L"Please verify CombineWorldDBID is configured; world-combine features require it.\r\n");
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

		WCHAR wszLogStr[128] = {0,};	
		char szData[MAX_PATH];

		std::vector<std::string> Tokens;

		g_IniFile.GetValue(L"Connection", L"Log", wszLogStr);
		if (wszLogStr[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszLogStr, -1, szData, sizeof(szData), NULL, NULL);

			Tokens.clear();
			TokenizeA(szData, Tokens, ":");
			if (!Tokens.empty()){
				_strcpy(g_Config.LogInfo.szIP, _countof(g_Config.LogInfo.szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
				g_Config.LogInfo.nPort = atoi(Tokens[1].c_str());
			}
		}

		g_Log.Log(LogType::_FILELOG, L"LogInfo (Ip:%S, Port:%d)\r\n", g_Config.LogInfo.szIP, g_Config.LogInfo.nPort);		

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

	//Varies by country, and fixed values are commonly read from config.
	if (g_Config.nThreadMax > THREADMAX) g_Config.nThreadMax = THREADMAX;

	// Hardcoded in code regardless of middleware thread argument.
	g_Config.nThreadMax = THREADMAX;


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

	swprintf(wszLogName, L"DBServer");

	//g_Log.Init(L"DBServer", LOGTYPE_CRT_FILE_DAY);
#if defined (_FINAL_BUILD)
	g_Log.Init(wszLogName, LOGTYPE_FILEDB_DAY);
#else
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_DAY);
#endif
	
	WCHAR wszSPLogName[128];
	memset(wszSPLogName, 0, sizeof(wszSPLogName));
	swprintf(wszSPLogName, L"%s_SP", wszLogName);
	g_SPLog.Init(wszSPLogName, LOGTYPE_FILE);	

	if (!LoadConfig(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"LoadConfig Failed\r\n");
		return false;
	}

	g_Log.SetServerID(g_Config.nManagedID);

	// Create ResourceMng
	CEtResourceMng::CreateInstance();

	// Set Path
	std::string szResource = g_Config.szResourcePath + "\\Resource";

	CEtResourceMng::GetInstance().AddResourcePath( szResource, true );

	g_pSPErrorCheckManager = new CDNSPErrorCheckManager;
	if( !g_pSPErrorCheckManager )
		return false;

	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager) return false;

	if (g_pIocpManager->Init(100) < 0){
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Fail(%d)\r\n", 100);
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## Iocp Initialize Success(%d)\r\n", 100);
	}

	g_pExtManager = new CDNExtManager;
	if (!g_pExtManager) return false;
	if (!g_pExtManager->AllLoad())
	{
		g_Log.Log(LogType::_FILELOG, L"CDNExtManager::AllLoad Fail\r\n");
		return false;
	}

	if (!g_ThreadManager.Init(g_Config.nThreadMax)){
		g_Log.Log(LogType::_FILELOG, L"ThreadManager Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"## ThreadManager Initialize Success(%d)\r\n", g_Config.nThreadMax);
	}

	g_pLogConnection = new CDNLogConnection;
	if( !g_pLogConnection ) return false;
	g_pLogConnection->SetIp(g_Config.LogInfo.szIP);
	g_pLogConnection->SetPort(g_Config.LogInfo.nPort);

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

	g_pIocpManager->CreateThread();

	g_Log.Log(LogType::_FILELOG, L"Application Initialize Success\r\n");

	return true;
}

void ClearApp()
{
	g_ThreadManager.Final();
	if( g_pIocpManager )
	{
		g_pIocpManager->CloseAcceptors();
		g_pIocpManager->ThreadStop();
	}	
	SAFE_DELETE(g_pIocpManager);
	SAFE_DELETE(g_pSPErrorCheckManager);
}

int _tmain(int argc, TCHAR* argv[])
{
#if defined( PRE_PARTY_DB )
	// PartyList is split and sent; check at compile time whether the max size of one packet exceeds the max sendable size on Network.
	BOOST_STATIC_ASSERT( (40*1024) >= sizeof(TAGetListParty) );
#endif // #if defined( PRE_PARTY_DB )

	// Since the binary size for storing KeySetting in DB is fixed at 59 bytes 
	// Trigger a compile error if it exceeds the specified size.
#if defined(PRE_ADD_SKILLSLOT_EXTEND)
	BOOST_STATIC_ASSERT( 79 >= sizeof(TKeySetting) );
	BOOST_STATIC_ASSERT( 201 >= sizeof(TPadSetting) );
#else	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
	BOOST_STATIC_ASSERT( 69 >= sizeof(TKeySetting) );
	BOOST_STATIC_ASSERT( 161 >= sizeof(TPadSetting) );
#endif	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)

	setlocale(LC_ALL, "Korean");

	// Prepare exception handler
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE);	// Changed Singleton implementation to fix C4744 warning in Release build; do not inline CExceptionReport::GetInstancePtr() (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}

	if (!InitApp(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"** InitApp Failed!!! Check!\r\n");		
		ClearApp();
		return 0;
	}

	wprintf(L"Type 'exit' to shut down.\r\n");

	char szCmd[256] = {0};	
	while (1)
	{

		if (strcmp(szCmd, "exit") == 0)	
			break;		

		if (strcmp(szCmd, "onsp") == 0)
			g_SPLog.SetActive(true);

		if (strcmp(szCmd, "offsp") == 0)
			g_SPLog.SetActive(false);

		printf("CMD>");
		(fgets(szCmd, sizeof(szCmd), stdin) ? (szCmd[strcspn(szCmd, "\r\n")] = '\0') : (szCmd[0] = '\0'));
	}

	ClearApp();

	return 0;
}
