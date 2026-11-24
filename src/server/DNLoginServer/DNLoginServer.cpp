// DNLoginServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerConsole.h"
// #include "FireWall.h"
#include "IniFile.h"
#include "DNIocpManager.h"
#include "DNSQLMembershipManager.h"
#include "DNSQLWorldManager.h"
#include "DNMasterConnectionManager.h"
#include "DNUserConnectionManager.h"
#include "DNExtManager.h"
#include "Log.h"
#include "Util.h"
#include "ExceptionReport.h"
#include "DNLogConnection.h"
#include "Version.h"
#include "DNAuthManager.h"
#include "DNSecure.h"
#include "EtResourceMng.h"


#if defined(PRE_FIX_DEBUGSET_INIT)
#include "DebugSet.h"
#endif
#include <oledb.h>

TLoginConfig g_Config;
TGameOptions g_GameOption;
#ifdef PRE_ADD_BEGINNERGUILD
#include "EtUIXML.h"
CEtUIXML *g_pUIXML;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD


//int AddToFirewall()
//{
//	HRESULT hr = S_OK;
//	HRESULT comInit = E_FAIL;
//	INetFwProfile* fwProfile = NULL;
//
//	// Initialize COM.
//	comInit = CoInitializeEx(
//		0,
//		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
//		);
//
//	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
//	// initialized with a different mode. Since we don't care what the mode is,
//	// we'll just use the existing mode.
//	if (comInit != RPC_E_CHANGED_MODE)
//	{
//		hr = comInit;
//		if (FAILED(hr))
//		{
//			printf("CoInitializeEx failed: 0x%08lx\n", hr);
//			goto error;
//		}
//	}
//
//	// Retrieve the firewall profile currently in effect.
//	hr = WindowsFirewallInitialize(&fwProfile);
//	if (FAILED(hr))
//	{
//		printf("WindowsFirewallInitialize failed: 0x%08lx\n", hr);
//		goto error;
//	}
//
//	WCHAR wszFileName[ _MAX_PATH ];
//	HMODULE hModule;
//	hModule = GetModuleHandle( NULL );
//	GetModuleFileNameW( hModule, wszFileName, _MAX_PATH );
//	// Add Windows Messenger to the authorized application collection.
//	hr = WindowsFirewallAddApp(
//		fwProfile,
//		wszFileName,
//		L"Dragon Nest"
//		);
//	if (FAILED(hr))
//	{
//		printf("WindowsFirewallAddApp failed: 0x%08lx\n", hr);
//		goto error;
//	}
//
//error:
//
//	// Release the firewall profile.
//	WindowsFirewallCleanup(fwProfile);
//
//	// Uninitialize COM.
//	if (SUCCEEDED(comInit))
//	{
//		CoUninitialize();
//	}
//
//	return 0;
//}

void LoadUserSessionID()
{
	DN_ASSERT(0 != g_Config.nManagedID,	"Check!");

	g_IDGenerator.SetUserConnectionMask(static_cast<BYTE>(g_Config.nManagedID));

	FILE *fp = fopen(CVarArgA<MAX_PATH>("./config/SessionID_%08d.dat", g_Config.nManagedID), "rb");

	if (!fp) return;

	UINT nSessionID = 0;
	fread(&nSessionID, 1, sizeof(UINT), fp);

	fclose(fp);

	nSessionID += DEFAULTUSERSESSIONID;	// Just in case saving fails and skips, add N as a hack!
	if (0x00FFFFFF <= nSessionID) nSessionID = DEFAULTUSERSESSIONID;

	g_IDGenerator.SetUserConnectionID(nSessionID);
}

void SaveUserSessionID()
{
	FILE *fp = fopen(CVarArgA<MAX_PATH>("./config/SessionID_%08d.dat", g_Config.nManagedID), "wb");

	if (!fp) return;

	UINT nSessionID = g_IDGenerator.GetUserConnectionID(true);

	fwrite(&nSessionID, 1, sizeof(UINT), fp);

	fclose(fp);
}

bool LoadConfig(int argc, TCHAR * argv[])
{
	memset(&g_Config, 0, sizeof(TLoginConfig));



	WCHAR wszBuf[128] = { 0, }; //, wszStr[64] = { 0, };
	char szData[128] = { 0, }, *pStr = NULL;

	wstring wszFileName = L"./config/DNLogin.ini";

	if (!g_IniFile.Open(wszFileName.c_str())){
		g_Log.Log(LogType::_FILELOG, L"%s File not Found!!\r\n", wszFileName.c_str());
		// return false;
	}
	
	//Version
	_strcpy(g_Config.szVersion, SERVERVERSIONMAX, szLoginVersion, (int)strlen(szLoginVersion));
	_strcpy(g_Config.szResVersion, SERVERVERSIONMAX, "Unknown Version", (int)strlen("Unknown Version"));

	std::string strCmd;
	strCmd = g_Config.szVersion;
	ToLowerA(strCmd);

	std::vector<std::string> tokens;
	TokenizeA(strCmd, tokens, ".");

	g_Config.nMajorVersion = atoi(tokens[0].c_str());
	g_Config.nMinorVersion = atoi(tokens[1].c_str());

//#if defined (_FINAL_BUILD)
	g_Config.nSQLMax = 50;
//#else
	//g_Config.nSQLMax = 8;
//#endif // #if defined (_FINAL_BUILD)

//////////////////////////////////////////////////////////////////
	//[debug] Override SQL and worker thread counts from INI config
	g_IniFile.GetValue(L"Info", L"IocpMax", &g_Config.nIocpMax);
	g_IniFile.GetValue(L"Info", L"SQLMax", &g_Config.nSQLMax);
	g_IniFile.GetValue(L"Info", L"WorkerThreadMax", &g_Config.nWorkerThreadMax);
//////////////////////////////////////////////////////////////////

	{
		g_IniFile.GetValue(L"Info", L"Version", &g_Config.nVersion);
		g_IniFile.GetValue(L"Info", L"IocpMax", &g_Config.nIocpMax);
		g_IniFile.GetValue(L"Info", L"SQLMax", &g_Config.nSQLMax);
		g_IniFile.GetValue(L"Info", L"WorkerThreadMax", &g_Config.nWorkerThreadMax);

		g_Log.Log(LogType::_FILELOG, L"## Ver:%d, IocpMax:%d SQLMax:%d\r\n", g_Config.nVersion, g_Config.nIocpMax, g_Config.nSQLMax);

		g_IniFile.GetValue(L"Connection", L"MasterAcceptPort", &g_Config.nMasterAcceptPort);
		g_IniFile.GetValue(L"Connection", L"ClientAcceptPort", &g_Config.nClientAcceptPort);
		g_Log.Log(LogType::_FILELOG, L"## ClientAcceptPort:%d\r\n", g_Config.nClientAcceptPort);

		WCHAR wszStr[128] = {0,};
		std::vector<std::string> Tokens;

		g_IniFile.GetValue(L"Connection", L"Log", wszStr);
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

	return true;
}

bool InitApp(int argc, TCHAR * argv[])
{
	//if (!CheckSerial()) return false;

	WCHAR wszLogName[128];
	memset(wszLogName, 0, sizeof(wszLogName));

	swprintf(wszLogName, L"LoginServer");

#if defined(_FINAL_BUILD)
	g_Log.Init(wszLogName, LOGTYPE_FILE_DAY);
#else
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_DAY);
#endif	

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

#if defined(PRE_FIX_DEBUGSET_INIT)
	CDebugSet::Initialize( CEtResourceMng::GetInstance().GetCurrentDirectory().c_str(), true );
	bool bParsingComplete = true;
	std::string strErrorString = "";
#endif
	// UI XML String File Load
#ifdef PRE_ADD_BEGINNERGUILD
	g_pUIXML = new CEtUIXML;
	CStream *pStream = CEtResourceMng::GetInstance().GetStream( "uistring.xml" );
#if defined(PRE_FIX_DEBUGSET_INIT)
	if( g_pUIXML->Initialize( pStream, CEtUIXML::idCategory1) == false )
	{
		bParsingComplete = false;
		strErrorString.append( "[uistring.xml]" );
	}
#else	//#if defined(PRE_FIX_DEBUGSET_INIT)
	g_pUIXML->Initialize( pStream, CEtUIXML::idCategory1);
#endif	//#if defined(PRE_FIX_DEBUGSET_INIT)
	SAFE_DELETE( pStream );


#if defined(PRE_FIX_DEBUGSET_INIT)
	if( !bParsingComplete )
	{
		g_Log.Log(LogType::_FILELOG, L"UI XML String File Load Failed : %S\r\n", strErrorString.c_str());
		return false;
	}
#endif	//#if defined(PRE_FIX_DEBUGSET_INIT)
#endif		//#ifdef PRE_ADD_BEGINNERGUILD


	CDNSecure::CreateInstance();

	g_pExtManager = new CDNExtManager;
	if (!g_pExtManager) return false;

	if (!g_pExtManager->AllLoad())
	{
		g_Log.Log(LogType::_FILELOG, L"CDNExtManager::AllLoad Fail\r\n");
		return false;
	}

	g_pMasterConnectionManager = new CDNMasterConnectionManager;
	if (!g_pMasterConnectionManager) return false;

	g_pUserConnectionManager = new CDNUserConnectionManager;
	if (!g_pUserConnectionManager) return false;


	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager) return false;

	if (g_Config.nIocpMax <= 0) g_Config.nIocpMax = 100;

//#ifdef _FINAL_BUILD
	//if (g_Config.nIocpMax < 2000)
	//{
	//	g_Config.nIocpMax = 2000;
	//	g_Log.Log(LogType::_FILELOG, L"Iocp Initialize NeedMore socket check serverStruct.xml\r\n");
	//}
//#endif

	if (g_pIocpManager->Init(g_Config.nIocpMax, g_Config.nWorkerThreadMax) < 0){
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Sucess(%d)\r\n", g_Config.nIocpMax);
	}

	//Ensure changelog and ServiceManager connection only work when launched via the launcher (via command-line arguments)
	g_pLogConnection = new CDNLogConnection;
	if( !g_pLogConnection ) return false;
	g_pLogConnection->SetIp(g_Config.LogInfo.szIP);
	g_pLogConnection->SetPort(g_Config.LogInfo.nPort);

	g_pSQLMembershipManager = new CDNSQLMembershipManager(g_Config.nWorkerThreadMax);
	if (!g_pSQLMembershipManager) return false;

	if (!g_pSQLMembershipManager->CreateDB()){
		g_Log.Log(LogType::_FILELOG, L"g_pSQLMembershipManager CreateDB Fail\r\n");
		return false;
	}

	g_pSQLWorldManager = new CDNSQLWorldManager(g_Config.nWorkerThreadMax);
	if (!g_pSQLWorldManager) return false;

	if (!g_pSQLWorldManager->CreateDB()){
		g_Log.Log(LogType::_FILELOG, L"g_pSQLWorldManager CreateDB Fail\r\n");
	}


	g_pAuthManager = new CDNAuthManager;	// P.S.> When not _FINAL_BUILD, auth manager must be created and initialized before MasterServer Acceptor to set g_Config.nManagedID
	if (!g_pAuthManager) {
		g_Log.Log(LogType::_FILELOG, L"g_pAuthManager Create Fail\r\n");
		return false;
	}
	if (!g_pAuthManager->Init()) {
		g_Log.Log(LogType::_FILELOG, L"g_pAuthManager Initialize Fail\r\n");
		return false;
	}

	// Master Accept 
	if (g_pIocpManager->AddAcceptConnection(CONNECTIONKEY_MASTER, g_Config.nMasterAcceptPort, 1000) < 0){
		g_Log.Log(LogType::_FILELOG, L"MasterAcceptPort Fail(%d)\r\n", g_Config.nMasterAcceptPort);
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"MasterAcceptPort (%d)\r\n", g_Config.nMasterAcceptPort);
	}

	
	LoadUserSessionID();

	// Client Accept
	// Accept upon receiving QUERY_RESETAUTHSERVER response.	
	

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
	SAFE_DELETE(g_pExtManager);
	SAFE_DELETE(g_pMasterConnectionManager);
	SAFE_DELETE(g_pSQLWorldManager);
	SAFE_DELETE(g_pSQLMembershipManager);
	SAFE_DELETE(g_pUserConnectionManager);
	SAFE_DELETE(g_pIocpManager);
	SAFE_DELETE(g_pLogConnection);
	SAFE_DELETE(g_pAuthManager);


	if( CDNSecure::IsActive() )
		CDNSecure::DestroyInstance();

}

#ifndef _FINAL_BUILD

#endif

int _tmain(int argc, TCHAR* argv[])
{
	BOOST_STATIC_ASSERT( sizeof(TIMESTAMP_STRUCT) == sizeof(DBTIMESTAMP) );

#ifndef _FINAL_BUILD
#endif

	g_GameOption.SetDefault();

	setlocale(LC_ALL, "Korean");


	// Prepare exception handler
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE, MiniDumpWithFullMemory);	// Changed Singleton implementation to fix C4744 warning in Release build; do not inline CExceptionReport::GetInstancePtr() (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}	

	//AddToFirewall();

	if (!InitApp(argc, argv)){
		g_Log.Log(LogType::_FILEDBLOG, L"** InitApp Failed!!! Check!!!!!\r\n");
		ClearApp();
		return 0;
	}

	g_Log.Log(LogType::_FILELOG, L"[Thread-Start] MainThread - PID ; %d, TID : %d\r\n", ::GetCurrentProcessId(), ::GetCurrentThreadId());

	wprintf(L"Type 'exit' to shut down.\r\n");

	char szCmd[256] = {0,};	
	while (1)
	{

		if (strcmp(szCmd, "exit") == 0)	break;		

		if (!strcmp(szCmd, "report"))
		{
			UINT nA, nP, nR;
			g_pIocpManager->GetSendReport(&nA, &nP, &nR);
			printf("send A[%d]P[%d]R[%d]\n", nA, nP, nR);
		}

		if (!strcmp(szCmd, "recvreport"))
		{
			UINT nA, nP, nR;
			g_pIocpManager->GetRecvReport(&nA, &nP, &nR);
			printf("recv A[%d]P[%d]R[%d]\n", nA, nP, nR);
		}

		if (!Irine::ServerConsole::ReadCommand(szCmd, sizeof(szCmd))) break;
	}

	SaveUserSessionID();	

	g_Log.Log(LogType::_FILELOG, L"[Thread-End] MainThread - PID ; %d, TID : %d\r\n", ::GetCurrentProcessId(), ::GetCurrentThreadId());

	ClearApp();


	return 0;
}
