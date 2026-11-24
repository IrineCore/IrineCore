// DNFieldServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FireWall.h"
#include "IniFile.h"
#include "DNIocpManager.h"
#include "DNDBConnectionManager.h"
#include "DNMasterConnectionManager.h"
#include "DNGameServerManager.h"
#include "DNGameDataManager.h"
#include "Log.h"
#include "Util.h"
#include "DnMainFrame.h"
#include "DNQuestManager.h"
#include "ExceptionReport.h"
#include "MAAiScript.h"
#include "DnScriptManager.h"
#include "DNRUDPGameServer.h"
#include "DNTalk.h"
#include "DNLogConnection.h"
#if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
#include "DNGameWorldUserState.hpp"
#else
#include "DNWorldUserState.h"
#endif // #if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
#include "DNGameServerScriptAPI.h"
#include "DNQuest.h"
#include "DNEvent.h"
#include "Version.h"
#include "MemPool.h"
#include "DNMissionScheduler.h"
#include "NoticeSystem.h"
#include "PerfCheck.h"
#include "DNAuthManager.h"
#include "DNGuildSystem.h"
#include "MAScanner.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "CloseSystem.h"
#include "MasterSystemCacheRepository.h"
#include "DNSecure.h"
#include "DNCashConnection.h"
#include "DNChatRoomManager.h"
#include "DNPeriodQuestSystem.h"
#include "DNGuildRecruitCacheRepository.h"
#ifdef PRE_ADD_LIMITED_CASHITEM
#include "DNLimitedCashItemRepository.h"
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DNPrivateChatChannel.h"
#include "DnPrivateChatManager.h"
#endif
#include "ModuleLoader.h"
#include "ModuleRegistry.h"

struct AssertReportMode {
	AssertReportMode() {
		_set_error_mode( _OUT_TO_MSGBOX );
	}
}assertReportMode;

#if !defined(_FINAL_BUILD)
CLog g_ScriptLog;
#endif

TGameConfig g_Config;


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

int AddToFirewall()
{
#if !defined(_WIN32)
	return 0;
#else
	HRESULT hr = S_OK;
	HRESULT comInit = E_FAIL;
	INetFwProfile* fwProfile = NULL;

	// Initialize COM.
	comInit = CoInitializeEx(
		0,
		COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
		);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
	// initialized with a different mode. Since we don't care what the mode is,
	// we'll just use the existing mode.
	if (comInit != RPC_E_CHANGED_MODE)
	{
		hr = comInit;
		if (FAILED(hr))
		{
			printf("CoInitializeEx failed: 0x%08lx\n", hr);
			goto error;
		}
	}

	// Retrieve the firewall profile currently in effect.
	hr = WindowsFirewallInitialize(&fwProfile);
	if (FAILED(hr))
	{
		printf("WindowsFirewallInitialize failed: 0x%08lx\n", hr);
		goto error;
	}

	WCHAR wszFileName[ _MAX_PATH ];
	HMODULE hModule;
	hModule = GetModuleHandle( NULL );
	GetModuleFileNameW( hModule, wszFileName, _MAX_PATH );
	// Add Windows Messenger to the authorized application collection.
	hr = WindowsFirewallAddApp(
		fwProfile,
		wszFileName,
		L"Dragon Nest"
		);
	if (FAILED(hr))
	{
		printf("WindowsFirewallAddApp failed: 0x%08lx\n", hr);
		goto error;
	}

error:

	// Release the firewall profile.
	WindowsFirewallCleanup(fwProfile);

	// Uninitialize COM.
	if (SUCCEEDED(comInit))
	{
		CoUninitialize();
	}

	return 0;
#endif
}

bool LoadConfig(int argc, TCHAR * argv[])
{
	
	memset(&g_Config, 0, sizeof(TGameConfig));

	WCHAR wszBuf[128] = { 0, }, wszStr[64] = { 0, };
	char szData[128] = { 0, };
	memset(wszBuf, 0, sizeof(wszBuf));
	memset(szData, 0, sizeof(szData));

	//Version
	_strcpy(g_Config.szVersion, _countof(g_Config.szVersion), szGameVersion, (int)strlen(szGameVersion));
	_strcpy(g_Config.szResVersion, _countof(g_Config.szResVersion), "Unknown Version", (int)strlen("Unknown Version"));

	wstring wszFileName = L"./config/DNGame.ini";

	if (!g_IniFile.Open(wszFileName.c_str())){
		g_Log.Log(LogType::_FILELOG, L"DNGame.ini File not Found!!\r\n");
		return false;
	}

	g_Config.cAffinityType = _GAMESERVER_AFFINITYTYPE_HYBRYD;

	{
		int nTemp = 1;
		g_IniFile.GetValue(L"Info", L"PreLoad", &nTemp);
		g_Config.bPreLoad = ( nTemp == 0 ) ? false : true;

		g_IniFile.GetValue(L"Info", L"OpenCount", &g_Config.nGameServerOpenCount);
		g_IniFile.GetValue(L"Info", L"StartPort", &g_Config.nGameAcceptPortBegin);
		g_IniFile.GetValue(L"Info", L"TcpPort", &g_Config.nClientAcceptPort);
		_ASSERT(g_Config.nClientAcceptPort != 0 && "Check Game.ini needs TcpPort");

		g_IniFile.GetValue(L"Info", L"IocpMax", &g_Config.nIocpMax);
		g_Log.Log(LogType::_FILELOG, L"IocpMax:%d\r\n", g_Config.nIocpMax);

		std::vector<std::string> Tokens;

		g_IniFile.GetValue(L"Connection", L"Cash", wszBuf);
		if (wszBuf[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);

			Tokens.clear();
			TokenizeA(szData, Tokens, ":");
			if (!Tokens.empty()){
				_strcpy(g_Config.CashInfo.szIP, _countof(g_Config.CashInfo.szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
				g_Config.CashInfo.nPort = atoi(Tokens[1].c_str());
			}
		}

		g_Log.Log(LogType::_FILELOG, L"Cash(%S, %d)\r\n", g_Config.CashInfo.szIP, g_Config.CashInfo.nPort);

		g_IniFile.GetValue(L"Connection", L"MasterCount", &g_Config.nMasterCount);

		for (int i = 0; i < g_Config.nMasterCount; i++){
			swprintf(wszStr, L"Master%d", i + 1);
			g_IniFile.GetValue(L"Connection", wszStr, wszBuf);
			if (wszBuf[0] != '\0'){
				WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);

				Tokens.clear();
				TokenizeA(szData, Tokens, ":");
				if (!Tokens.empty()){
					_strcpy(g_Config.MasterInfo[i].szIP, _countof(g_Config.MasterInfo[i].szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
					g_Config.MasterInfo[i].nPort = atoi(Tokens[1].c_str());
				}
			}
		}

		g_IniFile.GetValue(L"Connection", L"DBCount", &g_Config.nDBCount);

		for (int i = 0; i < g_Config.nDBCount; i++){
			swprintf(wszStr, L"DB%d", i + 1);
			g_IniFile.GetValue(L"Connection", wszStr, wszBuf);
			if (wszBuf[0] != '\0'){
				WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);

				Tokens.clear();
				TokenizeA(szData, Tokens, ":");
				if (!Tokens.empty()){
					_strcpy(g_Config.DBInfos[i].szIP, _countof(g_Config.DBInfos[i].szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
					g_Config.DBInfos[i].nPort = atoi(Tokens[1].c_str());
				}
			}
		}
		
		WCHAR wszLogStr[128] = {0,};	
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

		WCHAR wszProbeStr[128] = {0,};	
		g_IniFile.GetValue(L"Probe", L"ProbePort", &g_Config.nProbePort);
		g_IniFile.GetValue(L"Probe", L"ProbeIP", wszProbeStr);
		if (wszProbeStr[0] != '\0')
			WideCharToMultiByte(CP_ACP, 0, wszProbeStr, (int)wcslen(wszProbeStr), g_Config.szProbeIP, sizeof(g_Config.szProbeIP), NULL, NULL);

		// Register ResourcePath.
		WCHAR wszPath[_MAX_PATH] = { 0, };
		char szPath[_MAX_PATH] = { 0, };
		g_IniFile.GetValue( L"Resource", L"Path", wszPath );
		if (wszPath[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszPath, -1, szPath, sizeof(szPath), NULL, NULL);
		g_Config.szResourcePath = szPath;
		if( g_Config.szResourcePath.empty() ) g_Config.szResourcePath = ".";

		g_IniFile.GetValue(L"DolbyInfo", L"PrivateIp", wszBuf);
		if (wszBuf[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);
		_strcpy(g_Config.szPrivateDolbyIp, _countof(g_Config.szPrivateDolbyIp), szData, (int)strlen(szData));

		g_IniFile.GetValue(L"DolbyInfo", L"PublicIp", wszBuf);
		if (wszBuf[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);
		_strcpy(g_Config.szPublicDolbyIp, _countof(g_Config.szPublicDolbyIp), szData, (int)strlen(szData));

		g_IniFile.GetValue(L"DolbyInfo", L"ControlPort", &g_Config.nControlPort);
		g_IniFile.GetValue(L"DolbyInfo", L"AudioPort", &g_Config.nAudioPort);
		g_Log.Log(LogType::_FILELOG, L"DolbyInfo (PrivateIp:%S, PublicIP:%S, CPort:%d, APort:%d)\r\n", g_Config.szPrivateDolbyIp, g_Config.szPublicDolbyIp, g_Config.nControlPort, g_Config.nAudioPort);

#if defined( PRE_FIX_WORLDCOMBINEPARTY )
		g_IniFile.GetValue(L"Info", L"WorldCombineServer", &nTemp);
		g_Config.bWorldCombineGameServer = ( nTemp == 0 ) ? false : true;
#endif
	}

	return true;
}

bool LoadNpcQuest()
{

	g_pNpcQuestScriptManager->CreateLuaState(g_pGameServerManager->GetThreadCount());
	// Load all NPC dialog and quest-related scripts per thread.
	for ( int i = 0 ; i < g_pGameServerManager->GetThreadCount(); i++ )
	{
		lua_State* pLuaState = g_pNpcQuestScriptManager->OpenStateByIndex(i);
		DefAllAPIFunc(pLuaState);

		//-----------------------------------------------------------------------------------------------------------
		// Load common script file first
		std::vector<CFileNameString> CommonFileList;
		g_Log.Log(LogType::_FILELOG, L"QuestNPC_Common...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "QuestNPC_Common" ).c_str());
		CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "QuestNPC_Common", "*.lua", CommonFileList );

		for ( int j = 0 ; j < (int)CommonFileList.size() ; j++ )
		{
			bool bResult = g_pNpcQuestScriptManager->LoadScript(CEtResourceMng::GetInstance().GetFullName(CommonFileList[j].c_str()).c_str(), false, i, false);
			if ( !bResult )
			{
				g_Log.Log(LogType::_FILELOG, L"CommonLuaFile Load FAILED...\n");
				g_Log.Log(LogType::_FILELOG, L"%S\n", CEtResourceMng::GetInstance().GetFullName(CommonFileList[j].c_str()).c_str());
			}
		}
		//-----------------------------------------------------------------------------------------------------------


		g_Log.Log(LogType::_FILELOG, L"LoadScript Data...\n");
		g_Log.Log(LogType::_FILELOG, L"Talk_Quest...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "Talk_Quest" ).c_str());


		std::vector<CFileNameString> FileList;
		//		FindFileListInDirectory(CEtResourceMng::GetInstance().GetFullPath( "Talk_Quest" ).c_str(), "*.lua", FileList);
		CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Quest", "*.lua", FileList );

		for ( int j = 0 ; j < (int)FileList.size() ; j++ )
		{
			bool aRetVal = g_pNpcQuestScriptManager->LoadScript(CEtResourceMng::GetInstance().GetFullName(FileList[j].c_str()).c_str(), true, i, true);
			if (!aRetVal) {
				g_Log.Log(LogType::_FILELOG, L"LoadNpcQuest() Failed !!! - g_pNpcQuestScriptManager->LoadScript(...)\n");
				return false;
				//				DN_RETURN(false);
			}
		}

		int nColor = _GREEN;
		if ( FileList.empty() )
		{
			nColor = _RED;
		}

		g_Log.Log(LogType::_FILELOG, L"Talk_Quest...Size : %d\n", (int)FileList.size());
		g_Log.Log(LogType::_FILELOG, L"LoadScript Data...\n");
		g_Log.Log(LogType::_FILELOG, L"Talk_Npc...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "Talk_Npc" ).c_str());

		FileList.clear();
		//		FindFileListInDirectory(CEtResourceMng::GetInstance().GetFullPath( "Talk_Npc" ).c_str(), "*.lua", FileList);
		CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Npc", "*.lua", FileList );

		for ( int j = 0 ; j < (int)FileList.size() ; j++ )
		{
			g_pNpcQuestScriptManager->LoadScript(CEtResourceMng::GetInstance().GetFullName(FileList[j].c_str()).c_str(), true, i, false);
		}

		nColor = _GREEN;
		if ( FileList.empty() )
		{
			nColor = _RED;
		}
		g_Log.Log(LogType::_FILELOG, L"Talk_Npc...Size : %d\n", (int)FileList.size());
	}

	return true;
}

bool InitApp(int argc, TCHAR * argv[])
{
	//if (!CheckSerial()) return false;
	WCHAR wszLogName[128], wszScriptLogName[128] = { 0, };
	memset(wszLogName, 0, sizeof(wszLogName));
#if defined( PRE_TRIGGER_LOG )
	WCHAR wszTriggerLogName[128];
#endif // #if defined( PRE_TRIGGER_LOG )

	swprintf(wszLogName, L"GameServer");
	swprintf(wszScriptLogName, L"GameServerScript");
#if defined( PRE_TRIGGER_LOG )
	swprintf(wszTriggerLogName, L"TriggerLog");
#endif // #if defined( PRE_TRIGGER_LOG )

#if defined(_FINAL_BUILD)
	g_Log.Init(wszLogName, LOGTYPE_FILE_HOUR);
#else
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_HOUR);
#endif
	g_Log.SetServerID(g_Config.nManagedID);

#if defined( PRE_TRIGGER_LOG )
	g_TriggerLog.Init( wszTriggerLogName, LOGTYPE_CRT_FILE_TRIGGER_TEST );
#endif // #if defined( PRE_TRIGGER_LOG )

#if defined( PRE_TRIGGER_UNITTEST_LOG )
	WCHAR wszTriggerUnitTestLogName[MAX_PATH];
	if (argc > 14)
		swprintf( wszTriggerUnitTestLogName, L"TriggerUnitTestLog_%s", argv[4] );
	else
		swprintf( wszTriggerUnitTestLogName, L"TriggerUnitTestLog" );
	
	g_TriggerUnitTestLog.Init( wszTriggerUnitTestLogName, LOGTYPE_FILE_HOUR );
#endif // #if defined( PRE_TRIGGER_UNITTEST_LOG )

#if defined( PRE_QUESTSCRIPT_LOG )
	WCHAR wszQuestLogName[MAX_PATH];
	if (argc > 14)
		swprintf( wszQuestLogName, L"QuestLog_%s", argv[4] );
	else
		swprintf( wszQuestLogName, L"QuestLog" );

	g_QuestLog.Init( wszQuestLogName, LOGTYPE_FILE_HOUR );
#endif // #if defined( PRE_QUESTSCRIPT_LOG )

#if !defined(_FINAL_BUILD)
	g_ScriptLog.Init(wszScriptLogName, LOGTYPE_FILE_HOUR);
#endif


	//
	CDNSecure::CreateInstance();

	CDnActorActionStateCache::CreateInstance();

	g_pQuestManager = new CDNQuestManager();
	if ( !g_pQuestManager ) return false;
	g_pQuestManager->LoadAllQuest("QuestTable.ext", g_pNpcQuestScriptManager);	

	g_pDataManager = new CDNGameDataManager;
	if (!g_pDataManager) return false;
	if( !g_pDataManager->AllLoad() )
		return false;

	if( !g_AiScriptLoader.AllLoadScript() )
		return false;

	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager) return false;	


	g_pGuildManager = new CDNGuildSystem;
	if (!g_pGuildManager) {
		return false;
	}
	DWORD aRetVal = g_pGuildManager->Open();
	if (NOERROR != aRetVal) {
		return false;
	}

#if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
	g_pWorldUserState = new CDNGameWorldUserState;
#else
	g_pWorldUserState = new CDNWorldUserState;
#endif // #if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
	if (!g_pWorldUserState)	return false;

	g_pEvent = new CDNEvent;
	if (!g_pEvent) return false;

	// PeriodQuest System
	g_pPeriodQuestSystem = new CDNPeriodQuestSystem;
	if (!g_pPeriodQuestSystem) return false;
	if( !g_pPeriodQuestSystem->Initialize() )
		return false;

	g_pNoticeSystem = new CDNNoticeSystem;
	if (!g_pNoticeSystem) return false;

	g_pCloseSystem = new CCloseSystem;
	if (!g_pCloseSystem) return false;

	g_pMasterConnectionManager = new CDNMasterConnectionManager;
	if (!g_pMasterConnectionManager) return false;

	g_pDBConnectionManager = new CDNDBConnectionManager;
	if (!g_pDBConnectionManager) return false;

	g_pCashConnection = new CDNCashConnection;
	if (!g_pCashConnection) return false;

#ifdef PRE_ADD_LIMITED_CASHITEM
	g_pLimitedCashItemRepository = new CDNLimitedCashItemRepository;
	if (!g_pLimitedCashItemRepository) return false;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

#if defined( PRE_PRIVATECHAT_CHANNEL )
	g_pPrivateChatChannelManager = new CDNPrivateChatManager;
	if(!g_pPrivateChatChannelManager) return false;
#endif

	g_pAuthManager = new CDNAuthManager;
	if (!g_pAuthManager) return false;
	if (!g_pAuthManager->Init()) return false;

	g_pNpcQuestScriptManager = new DnScriptManager();
	if ( !g_pNpcQuestScriptManager ) return false;

	g_pGameServerManager = new CDNGameServerManager;
	if (!g_pGameServerManager) return false;

	CDNMissionScheduler::CreateInstance();
	MasterSystem::CCacheRepository::CreateInstance();
	MAScanner::CreateInstance();

	g_pGameServerManager->PreOpenGameServer();

	CDNTalk::SetScriptManager(g_pNpcQuestScriptManager);
	if (!LoadNpcQuest()) {
		g_Log.Log(LogType::_FILELOG, L"LoadNpcQuest() Failed !!!\r\n");
		return false;
	}

	//init rudp gameserver
	if (g_pGameServerManager->StartGameServer(g_Config.nGameAcceptPortBegin, g_Config.szProbeIP, g_Config.nProbePort) == false)
	{
		g_Log.Log(LogType::_FILELOG, L"RUDP GameServer Start Failed\n");
		return false;
	}

	g_Log.Log(LogType::_FILELOG, L"RUDP GameServer Start ThreadCount : %d\n", g_pGameServerManager->GetThreadCount() );

	if (g_Config.nIocpMax <= 0) g_Config.nIocpMax = 100;

#ifdef _FINAL_BUILD
	if (g_Config.nIocpMax < 2000)
	{
		g_Config.nIocpMax = 2000;
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize NeedMore socket check serverStruct.xml\r\n");
	}
#endif

	if (g_pIocpManager->Init(g_Config.nIocpMax, (2)) < 0){
		g_Log.Log(LogType::_FILELOG,L"Iocp Initialize Fail\r\n");
		return false;
	}	

	// Master Connection
	for (int i = 0; i < g_Config.nMasterCount; i++){
		if (!g_pMasterConnectionManager->AddConnection(g_Config.MasterInfo[i].szIP, g_Config.MasterInfo[i].nPort))
			g_Log.Log(LogType::_FILELOG, L"Master (Ip:%S, Port:%d) AddConnection Fail!!!!\r\n", g_Config.MasterInfo[i].szIP, g_Config.MasterInfo[i].nPort);
		else 
			g_Log.Log(LogType::_FILELOG, L"Master (Ip:%S, Port:%d) AddConnection Success\r\n", g_Config.MasterInfo[i].szIP, g_Config.MasterInfo[i].nPort);
	}
	
	// DB Connection
	for (int i = 0; i < g_Config.nDBCount; i++){
		if (!g_pDBConnectionManager->AddConnection(g_Config.DBInfos[i].szIP, g_Config.DBInfos[i].nPort)){
			g_Log.Log(LogType::_FILELOG, L"DBInfos (Ip:%S, Port:%d) AddConnection Fail!!!!\r\n", g_Config.DBInfos[i].szIP, g_Config.DBInfos[i].nPort);
		}
		g_Log.Log(LogType::_FILELOG, L"DBInfos (Ip:%S, Port:%d) AddConnection Success\r\n", g_Config.DBInfos[i].szIP, g_Config.DBInfos[i].nPort);
	}

	g_pLogConnection = new CDNLogConnection;
	if( !g_pLogConnection ) return false;
	g_pLogConnection->SetIp(g_Config.LogInfo.szIP);
	g_pLogConnection->SetPort(g_Config.LogInfo.nPort);

	// Just in case.. all threads are created first.
	Sleep(1000);

#ifdef _USE_VOICECHAT
	if (g_Config.nControlPort > 0)
	{
		g_pVoiceChat = new CDNVoiceChat();
		if (!g_pVoiceChat) return false;
		if (g_pVoiceChat->Initialize(0, "DragonNest", g_Config.szPrivateDolbyIp, g_Config.nControlPort) == false)
			return false;
	}
	else
	{
		g_Log.Log(LogType::_FILELOG, L"_USE_VOICECHAT check VoiceInfo\n");
	}
#endif

	g_Log.Log(LogType::_FILEDBLOG, L"Application Initialize Success\r\n");

	return true;
}

void ClearApp()
{

	// 2008.01.20 kimbap
	// Terminate ReconnectThread first due to sync issue between Connection and ReconnectThread
	if( g_pIocpManager )
	{
		g_pIocpManager->CloseAcceptors();
		g_pIocpManager->ThreadStop();
		g_pIocpManager->FinalReconnectThread();
	}

	SAFE_DELETE(g_pLogConnection);
	SAFE_DELETE(g_pQuestManager);
	SAFE_DELETE(g_pNpcQuestScriptManager);
	SAFE_DELETE(g_pCashConnection);
	if (g_pIocpManager)
		g_pIocpManager->Final();
	SAFE_DELETE(g_pDBConnectionManager);
	SAFE_DELETE(g_pMasterConnectionManager);

	SAFE_DELETE( g_pNoticeSystem );
	SAFE_DELETE( g_pCloseSystem );

	SAFE_DELETE(g_pGameServerManager);
	SAFE_DELETE(g_pWorldUserState);
	SAFE_DELETE(g_pEvent);
	if (g_pGuildManager) {
		g_pGuildManager->Close();
		delete g_pGuildManager;
		g_pGuildManager = NULL;
	}
	SAFE_DELETE(g_pIocpManager);
	SAFE_DELETE(g_pDataManager);
	SAFE_DELETE(g_pAuthManager);

#ifdef _USE_VOICECHAT
	SAFE_DELETE(g_pVoiceChat);
#endif
	SAFE_DELETE(g_pPeriodQuestSystem);

	if( MAScanner::IsActive() )
		MAScanner::DestroyInstance();

	if( CDNMissionScheduler::GetInstancePtr() )
		CDNMissionScheduler::DestroyInstance();

	if( MasterSystem::CCacheRepository::GetInstancePtr() )
		MasterSystem::CCacheRepository::DestroyInstance();

	if( CDNSecure::IsActive() )
		CDNSecure::DestroyInstance();

	if( CDnActorActionStateCache::IsActive() )
		CDnActorActionStateCache::DestroyInstance();
}

#ifndef _FINAL_BUILD


#endif

void ConsoleOutputDebug( const char *szString, ... )
{
	va_list list;
	char szTemp[65535];

	va_start( list, szString );
	vsprintf_s( szTemp, szString, list );
	printf( szTemp );
	va_end( list );
}
void (__stdcall *s_pPrevOutputDebugFunc)(const char*, ...) = NULL;

#if !defined( _FINAL_BUILD )

#define TESTTHREADCOUNT 30

UINT __stdcall LogTestThread(void *pParam)
{
	int iCount =1;
	do
	{
		g_Log.Log( (LogType::eLogType)100, L"LogTest %d %s\r\n", 23, L"test" );
	}while( iCount++ < 10000 );
	return 0;
}

#endif // #if !defined( _FINAL_BUILD )

int _tmain(int argc, TCHAR* argv[])
{
	BOOST_STATIC_ASSERT( eActor::CS_ACTOR_STATE_MAX == eActor::SC_ACTOR_STATE_MAX );
	BOOST_STATIC_ASSERT( eProp::CS_PROP_STATE_MAX == eProp::SC_PROP_STATE_MAX );
	/*
#if defined(_WORK)
	if (!g_PsUpdater)
		g_PsUpdater = new CPsUpdater;

	g_PsUpdater->AsyncUpdate ();
#endif	// #if defined(_WORK)
	*/
#ifndef _FINAL_BUILD
#endif


	setlocale(LC_ALL, "Korean");

	// Prepare exception handler
//#if !defined(_FINAL_BUILD)
	//Always create full memory dump on unhandledexception
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE, MiniDumpWithFullMemory);	// Changed Singleton implementation to fix C4744 warning in Release build; do not inline CExceptionReport::GetInstancePtr() (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
//#else	// _FINAL_BUILD
//	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE); // Changed Singleton implementation because C4744 warning occurs in Release build; do not inline CExceptionReport::GetInstancePtr() (Ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
//#endif	// _FINAL_BUILD
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}
	
	::srand(timeGetTime());
	if (!LoadConfig(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"LoadConfig Failed\r\n");
		return 0;
	}

	AddToFirewall();
#ifdef NDEBUG
//	SetOutputDebugFuncPtr( NULL );
#endif

	g_Config.bAllLoaded = false;
	if (!CLfhHeap::GetInstance()->InitPool())
	{
		g_Log.Log(LogType::_FILELOG, L"** InitPool Failed!!! Check!!!!!\r\n");
		return 0;
	}

	CDnMainFrame *pMainFrame = new CDnMainFrame;
	pMainFrame->PreInitialize();

	pMainFrame->InitializeDevice();

	if (!InitApp(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"** InitApp Failed!!! Check!!!!!\r\n");
		ClearApp();
		delete pMainFrame;
		return 0;
	}
	pMainFrame->Initialize();
	Irine::Modules::LoadAllModules( L"./module" );
	Irine::Modules::ModuleRegistry::Instance().NotifyServerInitialize();
	g_pIocpManager->VerifyAccept(ACCEPTOPEN_VERIFY_TYPE_APPINITCOMPLETE);
	g_Config.bAllLoaded = true;

	wprintf(L"Type 'exit' to shut down.\r\n");


	char szCmd[256] = {0,};	
	while (1)
	{
		if (strcmp(szCmd, "exit") == 0)	break;		

		if (strcmp(szCmd, "assert") == 0)
		{
			EnableAssert(true);
			printf("Asserts Enabled.\n");
		}

		if (strcmp(szCmd, "disable_assert") == 0)
		{
			EnableAssert(false);
			printf("Asserts Disabled.\n");
		}

		if (strcmp(szCmd, "reload") == 0)
		{
			g_pDataManager->LoadTalkData();
			g_pQuestManager->LoadAllQuest("QuestTable.ext", g_pNpcQuestScriptManager);
			g_pNpcQuestScriptManager->CloseAllState();
			LoadNpcQuest();
			printf("Reload all script..\n");
		}
		if ( strcmp(szCmd, "quest_list") == 0 )
		{
			std::vector<CDNQuest*> questList;
			g_pQuestManager->DumpAllQuest(questList);
			wprintf(L"Quest list size = %d\n", questList.size());

			for ( size_t i = 0; i < questList.size() ; i++ )
			{
				CDNQuest* pQuest = questList[i];

				wprintf(L"Cnt %d: QuestIndex[%d]\nTitle[%s]\nXML[%s]\nLUA[%s]\n\n", i,
					pQuest->GetQuestInfo().nQuestIndex,
					pQuest->GetQuestInfo().szQuestName.c_str(),
					pQuest->GetQuestInfo().szTalkFileName.c_str(),
					pQuest->GetQuestInfo().szScriptFileName.c_str());
			}
		}

		if ( strcmp(szCmd, "talk_list") == 0 )
		{
			std::vector<std::wstring> talklist;
			g_pDataManager->GetTalkFileList(talklist);

			wprintf(L"talk list size = %d\n", talklist.size());

			for ( size_t i = 0 ; i < talklist.size() ; i++ )
			{
				wprintf(L"%s\n", talklist[i].c_str());
			}

		}
		if (strcmp(szCmd, "enableprofile") == 0)
		{
			s_pPrevOutputDebugFunc = s_pProfileOutputDebugFunc;
//			SetProfileOutputDebugFuncPtr( ConsoleOutputDebug );
			g_bEnableProfile = true;
		}
		if (strcmp(szCmd, "disableprofile") == 0)
		{
			if( s_pPrevOutputDebugFunc ) 
				SetProfileOutputDebugFuncPtr( s_pPrevOutputDebugFunc );
			g_bEnableProfile = false;
		}

#if !defined( _FINAL_BUILD )
		if( strcmp( szCmd, "logtest" ) == 0 )
		{
			CPerformanceLog Perf("LogTest");
			HANDLE hThread[TESTTHREADCOUNT];
			for( int i=0 ; i<TESTTHREADCOUNT ; ++i )
			{
				UINT ThreadID;
				hThread[i] = (HANDLE)_beginthreadex(NULL, 0, &LogTestThread, NULL, 0, &ThreadID);
			}

			WaitForMultipleObjects( TESTTHREADCOUNT, hThread, true, INFINITE );
		}
#endif // #if defined( _FINAL_BUILD )

		printf("CMD>");
		(fgets(szCmd, sizeof(szCmd), stdin) ? (szCmd[strcspn(szCmd, "\r\n")] = '\0') : (szCmd[0] = '\0'));
	}

	Irine::Modules::ModuleRegistry::Instance().NotifyServerShutdown();
	SAFE_DELETE( pMainFrame );

	ClearApp();


	return 0;
}
