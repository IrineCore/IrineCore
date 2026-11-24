// DNVillageServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerConsole.h"
#include "FireWall.h"
#include "IniFile.h"
#include "EtResourceMng.h"
#include "DNDBConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNGameDataManager.h"
#include "DNFieldDataManager.h"
#include "DNFieldManager.h"
#include "DNIocpManager.h"
#include "DNPartyManager.h"
#include "DNUserSessionManager.h"
#include "DNQuestManager.h"
#include "Log.h"
#include "Util.h"
#include "ExceptionReport.h"
#include "DnScriptManager.h"
#include "DNTalk.h"
#include "DNScriptAPI.h"
#include "DNLogConnection.h"
#if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
#include "DNVillageWorldUserState.hpp"
#else
#include "DNWorldUserState.h"
#endif // #if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
#include "DNGuildSystem.h"
#include "DNEvent.h"
#include "EtUIXML.h"
#include "DebugSet.h"
#include "DNQuest.h"
#include "DNPvPRoomManager.h"
#include "Version.h"
#include "DNMissionScheduler.h"
#include "NoticeSystem.h"
#include "DNAuthManager.h"
#include "DNSecure.h"
#include "DNDungeonManager.h"
#include "DNDBConnection.h"
#include "DNCashConnection.h"
#include "CloseSystem.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "DNChatRoomManager.h"
#include "MasterSystemCacheRepository.h"

#include "DNFarm.h"
#include "DNGuildWarManager.h"
#include "DNPeriodQuestSystem.h"
#include "DNLadderSystemManager.h"

#include "TradeCacheRepository.h"
#include "DNGuildRecruitCacheRepository.h"

#if defined (PRE_ADD_DONATION)
#include "DNDonationScheduler.h"
#endif // #if defined (PRE_ADD_DONATION)

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

CDnTableDB g_TableDB;
CEtResourceMng g_ResMng;
CEtUIXML *g_pUIXML;
#if !defined(_FINAL_BUILD)
CLog g_ScriptLog;
#endif
#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
CEtExceptionalUIXML*	g_pExceptionalUIXML;
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING

TVillageConfig g_Config;


/*
int AddToFirewall()
{
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
}
*/

bool LoadConfig(int argc, TCHAR * argv[])
{
	memset(&g_Config, 0, sizeof(TVillageConfig));
	
	//Version
	_strcpy(g_Config.szVersion, _countof(g_Config.szVersion), szVillageVersion, (int)strlen(szVillageVersion));
	_strcpy(g_Config.szResVersion, _countof(g_Config.szResVersion), "Unknown Version", (int)strlen("Unknown Version"));

	WCHAR wszBuf[128] = { 0, }, wszStr[64] = { 0, };
	char szData[128] = { 0, };
	memset(wszBuf, 0, sizeof(wszBuf));
	memset(szData, 0, sizeof(szData));

	wstring wszFileName = L"./config/DNVillage.ini";

	if (!g_IniFile.Open(wszFileName.c_str())){
		g_Log.Log(LogType::_FILELOG, L"DNVillage.ini File not Found!!\r\n");
		return false;
	}


	{
		g_IniFile.GetValue(L"Info", L"VillageID", &g_Config.nVillageID);
		g_IniFile.GetValue(L"Info", L"IocpMax", &g_Config.nIocpMax);
#if defined( PRE_WORLDCOMBINE_PARTY )
		g_IniFile.GetValue(L"Info", L"CombinePartyWorld", &g_Config.nCombinePartyWorld);
#endif
		g_Log.Log(LogType::_FILELOG, L"VillageID:%d IocpMax:%d\r\n", g_Config.nVillageID, g_Config.nIocpMax);

		g_IniFile.GetValue(L"Connection", L"ClientAcceptPort", &g_Config.nClientAcceptPort);
		g_Log.Log(LogType::_FILELOG, L"## ClientAcceptPort:%d\r\n", g_Config.nClientAcceptPort);		

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

		g_IniFile.GetValue(L"Connection", L"Master", wszBuf);
		if (wszBuf[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, szData, sizeof(szData), NULL, NULL);

			Tokens.clear();
			TokenizeA(szData, Tokens, ":");
			if (!Tokens.empty()){
				_strcpy(g_Config.MasterInfo.szIP, _countof(g_Config.MasterInfo.szIP), Tokens[0].c_str(), (int)strlen(Tokens[0].c_str()));
				g_Config.MasterInfo.nPort = atoi(Tokens[1].c_str());
			}
		}

		g_Log.Log(LogType::_FILELOG, L"Master(%S, %d)\r\n", g_Config.MasterInfo.szIP, g_Config.MasterInfo.nPort);

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

		g_IniFile.GetValue(L"DB_DNMembership", L"DBIP", wszBuf);
		if (wszBuf[0] != '\0')
			WideCharToMultiByte(CP_ACP, NULL, wszBuf, -1, g_Config.MembershipDB.szIP, sizeof(g_Config.MembershipDB.szIP), NULL, NULL);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBPort", &g_Config.MembershipDB.nPort);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBID", g_Config.MembershipDB.wszDBID);
		g_IniFile.GetValue(L"DB_DNMembership", L"DBName", g_Config.MembershipDB.wszDBName);

		// Register ResourcePath.
		WCHAR wszPath[_MAX_PATH] = { 0, };
		char szPath[_MAX_PATH] = { 0, };
		g_IniFile.GetValue( L"Resource", L"Path", wszPath );
		if (wszPath[0] != '\0'){
			WideCharToMultiByte(CP_ACP, NULL, wszPath, -1, szPath, sizeof(szPath), NULL, NULL);
			g_Config.szResourcePath = szPath;
			if( g_Config.szResourcePath.empty() ) g_Config.szResourcePath = ".";
		}
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

	}

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

bool LoadNpcQuest()
{

	g_pNpcQuestScriptManager->CreateLuaState(1);
	lua_State* pLuaState = g_pNpcQuestScriptManager->OpenStateByIndex(0);
	DefAllAPIFunc(pLuaState);

	std::vector<CFileNameString> FileList;
	//-----------------------------------------------------------------------------------------------------------
	// Load common script file first
	std::vector<CFileNameString> CommonFileList;
	g_Log.Log(LogType::_FILELOG, L"QuestNPC_Common...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "QuestNPC_Common" ).c_str());
	//	FindFileListInDirectory(CEtResourceMng::GetInstance().GetFullPath( "QuestNPC_Common" ).c_str(), "*.lua", CommonFileList);
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "QuestNPC_Common", "*.lua", CommonFileList );

	for ( int i = 0 ; i < (int)CommonFileList.size() ; i++ )
	{
		bool bResult = g_pNpcQuestScriptManager->LoadScript( CEtResourceMng::GetInstance().GetFullName(CommonFileList[i].c_str()).c_str(), false, 0, false);
		if ( !bResult )
		{
			g_Log.Log(LogType::_FILELOG, L"CommonLuaFile Load FAILED...\n");
			g_Log.Log(LogType::_FILELOG, L"%S\n", CEtResourceMng::GetInstance().GetFullName(CommonFileList[i].c_str()).c_str());
		}
	}
	//-----------------------------------------------------------------------------------------------------------

	g_Log.Log(LogType::_FILELOG, L"LoadScript Data...\n");
	g_Log.Log(LogType::_FILELOG, L"Talk_Quest...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "Talk_Quest" ).c_str());

	//	FindFileListInDirectory(CEtResourceMng::GetInstance().GetFullPath( "Talk_Quest" ).c_str(), "*.lua", FileList);
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Quest", "*.lua", FileList );

	for ( int i = 0 ; i < (int)FileList.size() ; i++ )
	{
		bool aRetVal = g_pNpcQuestScriptManager->LoadScript( CEtResourceMng::GetInstance().GetFullName(FileList[i].c_str()).c_str(), true, 0, true);
		if (!aRetVal) {
			g_Log.Log(LogType::_FILELOG, L"LoadNpcQuest() Failed !!! - g_pNpcQuestScriptManager->LoadScript(...)\n");
			return false;
			//			DN_RETURN(false);
		}
	}

	int nColor = _GREEN;
	if ( FileList.size() < 1 )
	{
		nColor = _RED;
	}

	g_Log.Log(LogType::_FILELOG, L"Talk_Quest...Size : %d\n", (int)FileList.size());
	g_Log.Log(LogType::_FILELOG, L"LoadScript Data...\n");
	g_Log.Log(LogType::_FILELOG, L"Talk_Npc...Folder : %S\n", CEtResourceMng::GetInstance().GetFullPath( "Talk_Npc" ).c_str());


	FileList.clear();
	//	FindFileListInDirectory(CEtResourceMng::GetInstance().GetFullPath( "Talk_Npc" ).c_str(), "*.lua", FileList);
	CEtResourceMng::GetInstance().FindFileListAll_IgnoreExistFile( "Talk_Npc", "*.lua", FileList );

	for ( int i = 0 ; i < (int)FileList.size() ; i++ )
	{
		g_pNpcQuestScriptManager->LoadScript( CEtResourceMng::GetInstance().GetFullName(FileList[i].c_str()).c_str(), true, 0, false);
	}

	nColor = _GREEN;
	if ( FileList.size() < 1 )
	{
		nColor = _RED;
	}
	g_Log.Log(LogType::_FILELOG, L"Talk_Npc...Size : %d\n", (int)FileList.size());

	return true;
}

bool InitApp(int argc, TCHAR * argv[])
{
	//if (!CheckSerial()) return false;

	WCHAR wszLogName[128], wszScriptLogName[128] = { 0, };
	memset(wszLogName, 0, sizeof(wszLogName));

	swprintf(wszLogName, L"VillageServer");
	swprintf(wszScriptLogName, L"VillageServerScript");

#if defined(_FINAL_BUILD)
	//g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_HOUR);
	g_Log.Init(wszLogName, LOGTYPE_FILE_HOUR);
#else
	g_Log.Init(wszLogName, LOGTYPE_CRT_FILE_HOUR);
#endif

#if !defined(_FINAL_BUILD)
	g_ScriptLog.Init(wszScriptLogName, LOGTYPE_FILE_HOUR);
#endif	

	if (!LoadConfig(argc, argv)){
		g_Log.Log(LogType::_FILELOG, L"LoadConfig Failed\r\n");
		return false;
	}
	g_Log.SetServerID(g_Config.nManagedID);


	std::string szResource = g_Config.szResourcePath + "\\Resource";
	std::string szMapData = g_Config.szResourcePath + "\\MapData";


	CEtResourceMng::GetInstance().AddResourcePath( szResource.c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\Ext").c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( szMapData.c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\Script").c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\Script\\QuestNPC_Common").c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\Script\\Talk_Npc").c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\Script\\Talk_Quest").c_str() );
	CEtResourceMng::GetInstance().AddResourcePath( std::string(szResource + "\\UIString").c_str() );

	CDnTableDB::GetInstance().Initialize();

	CDebugSet::Initialize( CEtResourceMng::GetInstance().GetCurrentDirectory().c_str(), true );

	CDNSecure::CreateInstance();

	// UI XML String File Load
	g_pUIXML = new CEtUIXML;
	CStream *pStream = CEtResourceMng::GetInstance().GetStream( "uistring.xml" );
	g_pUIXML->Initialize( pStream, CEtUIXML::idCategory1 );
	SAFE_DELETE( pStream );



#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
	g_pExceptionalUIXML = new CEtExceptionalUIXML;
	if( g_pExceptionalUIXML )
		g_pExceptionalUIXML->LoadXML( "uistring_exception.xml" );
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING


	g_pNpcQuestScriptManager = new DnScriptManager();
	if ( !g_pNpcQuestScriptManager ) return false;

	g_pQuestManager = new CDNQuestManager();
	if ( !g_pQuestManager ) return false;
	
	g_pIocpManager = new CDNIocpManager;
	if (!g_pIocpManager) return false;

	CDNTalk::SetScriptManager(g_pNpcQuestScriptManager);
	g_pQuestManager->LoadAllQuest("QuestTable.ext", g_pNpcQuestScriptManager);

	g_pFieldDataManager = new CDNFieldDataManager;
	if (!g_pFieldDataManager) return false;

	g_pFieldManager = new CDNFieldManager;
	if (!g_pFieldManager) return false;

	g_pDataManager = new CDNGameDataManager;
	if (!g_pDataManager) return false;
	if( !g_pDataManager->AllLoad() )
		return false;

	g_pAuthManager = new CDNAuthManager;
	if (!g_pAuthManager) {
		return false;
	}
	if (!g_pAuthManager->Init()) {
		return false;
	}

	g_pGuildManager = new CDNGuildSystem;

	if (!g_pGuildManager) {
		return false;
	}
	DWORD aRetVal = g_pGuildManager->Open();
	if (NOERROR != aRetVal) {
		return false;
	}

#if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
	g_pWorldUserState = new CDNVillageWorldUserState;
#else
	g_pWorldUserState = new CDNWorldUserState;
#endif // #if defined( PRE_WORLDUSERSTATE_OPTIMIZE )
	if (!g_pWorldUserState) return false;

	g_pEvent = new CDNEvent;
	if (!g_pEvent) return false;

	g_pPeriodQuestSystem = new CDNPeriodQuestSystem;
	if (!g_pPeriodQuestSystem) return false;
	if( !g_pPeriodQuestSystem->Initialize() ) return false;

	g_pFarm = new CDNFarm;
	if (!g_pFarm) return false;

	g_pGuildWarManager = new CDNGuildWarManager;
	if (!g_pGuildWarManager) return false;

	g_pNoticeSystem = new CDNNoticeSystem;
	if (!g_pNoticeSystem) return false;

	g_pCloseSystem = new CCloseSystem;
	if (!g_pCloseSystem) return false;

	g_pUserSessionManager = new CDNUserSessionManager;
	if (!g_pUserSessionManager) return false;

	g_pDBConnectionManager = new CDNDBConnectionManager;
	if (!g_pDBConnectionManager) return false;

	// Log Connection
	g_pLogConnection = new CDNLogConnection;
	if( !g_pLogConnection ) return false;
	g_pLogConnection->SetIp(g_Config.LogInfo.szIP);
	g_pLogConnection->SetPort(g_Config.LogInfo.nPort);

	g_pMasterConnection = new CDNMasterConnection;
	if (!g_pMasterConnection) return false;
	g_pMasterConnection->SetIp(g_Config.MasterInfo.szIP);
	g_pMasterConnection->SetPort(g_Config.MasterInfo.nPort);

	g_pCashConnection = new CDNCashConnection;
	if (!g_pCashConnection) return false;
	g_pCashConnection->SetIp(g_Config.CashInfo.szIP);
	g_pCashConnection->SetPort(g_Config.CashInfo.nPort);	

#ifdef PRE_ADD_LIMITED_CASHITEM
	g_pLimitedCashItemRepository = new CDNLimitedCashItemRepository;
	if (!g_pLimitedCashItemRepository) return false;
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

	g_pPartyManager = new CDNPartyManager;
	if (!g_pPartyManager) return false;

	g_pChatRoomManager = new CDNChatRoomManager;
	if (!g_pChatRoomManager) return false;

	
	CDNMissionScheduler::CreateInstance();
	CDNPvPRoomManager::CreateInstance();
	MasterSystem::CCacheRepository::CreateInstance();
	LadderSystem::CManager::CreateInstance();
	TradeSystem::CMarketPriceCache::CreateInstance();
	GuildRecruitSystem::CCacheRepository::CreateInstance();

#if defined (PRE_ADD_DONATION)
	CDNDonationScheduler::CreateInstance();
#endif // #if defined (PRE_ADD_DONATION)	
#if defined( PRE_PRIVATECHAT_CHANNEL )
	g_pPrivateChatChannelManager = new CDNPrivateChatManager;
	if(!g_pPrivateChatChannelManager) return false;
#endif

	// db connection
	for( int i=0 ; i<g_Config.nDBCount ; ++i )
	{
		if (!g_pDBConnectionManager->AddConnection(g_Config.DBInfos[i].szIP, g_Config.DBInfos[i].nPort))
			g_Log.Log(LogType::_FILELOG, L"DBInfos (Ip:%S, Port:%d) AddConnection Fail!!!!\r\n", g_Config.DBInfos[i].szIP, g_Config.DBInfos[i].nPort);
	}

	if (g_Config.nIocpMax <= 0) g_Config.nIocpMax = 100;

#ifdef _FINAL_BUILD
	if (g_Config.nIocpMax < 2000)
	{
		g_Config.nIocpMax = 2000;
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize NeedMore socket check serverStruct.xml\r\n");
	}
#endif

	if (!LoadNpcQuest()) {
		g_Log.Log(LogType::_FILELOG, L"LoadNpcQuest() Failed !!!\r\n");
		return false;
	}

	if (g_pIocpManager->Init(g_Config.nIocpMax) < 0){
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Fail\r\n");
		return false;
	}
	else {
		g_Log.Log(LogType::_FILELOG, L"Iocp Initialize Sucess(%d)\r\n", g_Config.nIocpMax);
	}

	Sleep(200);

	_srand( timeGetTime() );
	srand(timeGetTime());


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

	//Once all initialization is complete
	g_pIocpManager->VerifyAccept(ACCEPTOPEN_VERIFY_TYPE_APPINITCOMPLETE);


	g_Log.Log(LogType::_FILEDBLOG, L"Application Initialize Success\r\n");
	return true;
}

void ClearApp()
{

	if( g_pIocpManager)
	{
		g_pIocpManager->CloseAcceptors();
		g_pIocpManager->ThreadStop();
	}
	SAFE_DELETE(g_pLogConnection);
	SAFE_DELETE(g_pPartyManager);
	SAFE_DELETE(g_pCashConnection);
	if (g_pIocpManager)
		g_pIocpManager->Final();
	SAFE_DELETE(g_pDBConnectionManager);
	SAFE_DELETE(g_pMasterConnection);
	SAFE_DELETE(g_pUserSessionManager);
	SAFE_DELETE(g_pIocpManager);
	SAFE_DELETE(g_pWorldUserState);
	if (g_pGuildManager) {
		g_pGuildManager->Close();
		delete g_pGuildManager;
		g_pGuildManager = NULL;
	}
	SAFE_DELETE(g_pGuildWarManager);
	SAFE_DELETE(g_pDataManager);
	SAFE_DELETE(g_pFieldManager);
	SAFE_DELETE(g_pFieldDataManager);
	SAFE_DELETE(g_pQuestManager);
	SAFE_DELETE(g_pNpcQuestScriptManager);
	SAFE_DELETE(g_pUIXML);
	SAFE_DELETE(g_pAuthManager);
#ifdef PRE_ADD_EXCEPTIONAL_MAKESTRING
	SAFE_DELETE(g_pExceptionalUIXML);
#endif // PRE_ADD_EXCEPTIONAL_MAKESTRING
	if( LadderSystem::CManager::IsActive() )
		LadderSystem::CManager::DestroyInstance();
	if( CDNPvPRoomManager::IsActive() )
		CDNPvPRoomManager::DestroyInstance();
	if( MasterSystem::CCacheRepository::IsActive() )
		MasterSystem::CCacheRepository::DestroyInstance();
	if( CDNMissionScheduler::IsActive() )
		CDNMissionScheduler::DestroyInstance();
	if( GuildRecruitSystem::CCacheRepository::IsActive() )
		GuildRecruitSystem::CCacheRepository::DestroyInstance();	

#ifdef _USE_VOICECHAT
	SAFE_DELETE(g_pVoiceChat);
#endif
	SAFE_DELETE(g_pChatRoomManager);
	SAFE_DELETE(g_pPeriodQuestSystem);


	if( CDNSecure::IsActive() )
		CDNSecure::DestroyInstance();
}

#ifndef _FINAL_BUILD


#endif

#if defined( _FINAL_BUILD )
#else
void TestParty()
{
	#define TEST_COUNT 1000

	_srand( timeGetTime() );

	WCHAR CharList[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789DRAGONNEST";
	struct Data
	{
		WCHAR StringList[10][MAX_PATH];
		int SortPoint;
	};
	std::vector<Data> TestList;
	TestList.reserve(TEST_COUNT);

	class OrderBySortPointDesc
	{
	public:		
		bool operator()( const Data& lhs, const Data& rhs )
		{
			if( lhs.SortPoint <= rhs.SortPoint )
				return false;
			return true;
		}
	};

	Data Temp;
	int aaa = _countof(Temp.StringList);
	int bbb = _countof(CharList);

	for( int i=0 ; i<TEST_COUNT ; ++i )
	{
		Temp.SortPoint = rand()%1200;
		std::wstring TempString;
		for( int j=0 ; j<_countof(Temp.StringList) ; ++j )
		{
			TempString.clear();
			for( int k=0 ; k<8 ; ++k )
			{
				WCHAR szTemp[MAX_PATH];
				wsprintf( szTemp, L"%c", CharList[rand()%_countof(CharList)] );
				TempString += szTemp;
			}
			wsprintf( Temp.StringList[j], L"%s", TempString.c_str() );
		}
		TestList.push_back( Temp );
	}

	{
		CPerformanceLog Log( "StringCompareTest-String" );

		for( UINT i=0 ; i<TestList.size() ; ++i )
		{
			for( int j=0 ; j<_countof(TestList[i].StringList) ; ++j )
			{
				if( wcsstr( TestList[i].StringList[j], L"AB" ) )
				{
					TestList[i].SortPoint += ((j+1)*11);
				}
			}
		}
	}
	{
		CPerformanceLog Log( "StringSortTest" );
		std::sort( TestList.begin(), TestList.end(), OrderBySortPointDesc() );
	}
}
#endif // #if defined( _FINAL_BUILD )

int _tmain(int argc, TCHAR* argv[])
{
#if !defined( PRE_ADD_FARM_DOWNSCALE )
	BOOST_STATIC_ASSERT( false );
#endif // #if !defined( PRE_ADD_FARM_DOWNSCALE )

#ifdef __COVERITY__
#else
#ifndef _FINAL_BUILD
#endif
#endif
	/*
#if defined(_WORK)
	if (!g_PsUpdater)
		g_PsUpdater = new(std::nothrow) CPsUpdater;

	g_PsUpdater->AsyncUpdate ();
#endif	// #if defined(_WORK)
	*/
	setlocale(LC_ALL, "Korean");
	
#if defined( _FINAL_BUILD )
#else
	//TestParty();
#endif // #if defined( _FINAL_BUILD )

//	AddToFirewall();
//#if defined(_WORK)
	// Prepare exception handler
	DWORD dwRetVal = CExceptionReport::GetInstancePtr()->Open(_T(".\\"), TRUE, TRUE, MiniDumpWithFullMemory);	// Excluded via project settings due to C4744 warning in Release build (ref: http://msdn.microsoft.com/ko-kr/library/a7za416f.aspx)
	if (NOERROR != dwRetVal) {
		DWORD dwErrNo = ::GetLastError();
		DN_RETURN(dwErrNo);
	}

	if (!InitApp(argc, argv)){
		g_Log.Log(LogType::_FILEDBLOG, L"** InitApp Failed!!! Please Restart Server!!!!!\r\n");
		ClearApp();
		return 0;
	}

	Irine::Modules::LoadAllModules( L"./module" );
	Irine::Modules::ModuleRegistry::Instance().NotifyServerInitialize();

	wprintf(L"Type 'exit' to shut down.\r\n");

	//SetConsoleTitleA(FormatA("VillageServer Rev.%s", revDNVillageServer).c_str());

	char szCmd[256] = {0,};	
	while (1)
	{
		if (strcmp(szCmd, "exit") == 0)	break;		

		if (strcmp(szCmd, "reload") == 0)
		{
			//g_pQuestManager->LoadAllQuest("QuestTable.ext");

			
			g_pQuestManager->LoadAllQuest("QuestTable.ext", g_pNpcQuestScriptManager);
			g_pDataManager->LoadTalkData();
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

		if (!strcmp(szCmd, "report"))
		{
			UINT nA, nP, nR;
			g_pIocpManager->GetSendReport(&nA, &nP, &nR);
			printf("A[%d]P[%d]R[%d]\n", nA, nP, nR);
		}

		if (!strcmp(szCmd, "recvreport"))
		{
			UINT nA, nP, nR;
			g_pIocpManager->GetRecvReport(&nA, &nP, &nR);
			printf("recv A[%d]P[%d]R[%d]\n", nA, nP, nR);
		}

#if !defined( _FINAL_BUILD )

		if( !strcmp( szCmd, "sptest") )
		{
			int iCount[THREADMAX];
			for( int i=0 ; i<THREADMAX ; ++i )
				iCount[i] = 0;

			for( int k=0 ; k<1000 ; ++k )
			{
				TQMWTest TxPacket;
				TxPacket.cWorldSetID	= 14;
				TxPacket.cThreadID	= 0;

				CDNDBConnection* pCon = g_pDBConnectionManager->GetDBConnection( TxPacket.cThreadID );
				TxPacket.iCount = iCount[TxPacket.cThreadID]++;

				pCon->QueryMWTest( &TxPacket );
			}
		}

#endif // #if !defined( _FINAL_BUILD )


	
		if (!Irine::ServerConsole::ReadCommand(szCmd, sizeof(szCmd))) break;
	}	

	Irine::Modules::ModuleRegistry::Instance().NotifyServerShutdown();
	ClearApp();


	return 0;
}
