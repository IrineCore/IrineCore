#include "stdafx.h"
#include "DNQuest.h"
#include "DnScriptManager.h"
#include "DNUserSession.h"

#if defined(_VILLAGESERVER)
	#include "DNScriptAPI.h"
	#include "DNUserSessionManager.h"

#elif defined(_GAMESERVER)
	#include "DNGameServerScriptAPI.h"
	#include "DNRUDPGameServer.h"
	#include "DNGameServerManager.h"
#endif

CDNQuest::CDNQuest()
{

}

CDNQuest::~CDNQuest()
{
	//Destroy();
}

std::string CDNQuest::_MakeFuncName( const WCHAR* pwszName )
{
	std::wstring wszFuncName;
	wszFuncName = m_wszScriptFilename;
	wszFuncName += pwszName;

	char szFunctionName[512] = {0,};
	ZeroMemory(szFunctionName, sizeof(char)*512);
	WideCharToMultiByte( CP_ACP, 0, wszFuncName.c_str(), -1, szFunctionName, 512, NULL, NULL );
	
	return std::string( szFunctionName );
}

bool CDNQuest::Create(CREATE_INFO& info)
{
	std::transform(info.szScriptFileName.begin(), info.szScriptFileName.end(), info.szScriptFileName.begin(), towlower); 
	std::transform(info.szTalkFileName.begin(), info.szTalkFileName.end(), info.szTalkFileName.begin(), towlower); 

	if( CDNTalk::Create(info.szTalkFileName.c_str(), info.szScriptFileName.c_str() ) == false )
		return false;

	m_CreateInfo = info;
	//
	m_strFuncNameOnTalk					= _MakeFuncName( L"_OnTalk" );
	m_strFuncNameOnCompleteCounting		= _MakeFuncName( L"_OnCompleteCounting" );
	m_strFuncNameOnCounting				= _MakeFuncName( L"_OnCounting" );
	m_strFuncNameOnCompleteAllCounting	= _MakeFuncName( L"_OnCompleteAllCounting" );
	m_strFuncNameOnItemCollecting		= _MakeFuncName( L"_OnItemCollecting" );
#if defined(PRE_ADD_REMOTE_QUEST)
	m_strFuncNameOnRemoteStart			= _MakeFuncName( L"_OnRemoteStart" );
	m_strFuncNameOnRemoteComplete		= _MakeFuncName( L"_OnRemoteComplete" );
	m_strFuncNameCanRemoteCompleteStep	= _MakeFuncName( L"_CanRemoteCompleteStep" );
#endif // #if defined(PRE_ADD_REMOTE_QUEST)
	return true;

}

void CDNQuest::Destroy()
{

}

#if defined(_VILLAGESERVER)
bool CDNQuest::OnTalk(UINT nUserObjectID, UINT nNpcUniqueID, IN std::wstring& wszIndex, IN std::wstring& wszTarget)
{
#elif defined(_GAMESERVER)
bool CDNQuest::OnTalk(CDNGameRoom *pRoom, UINT nUserObjectID, UINT nNpcUniqueID, IN std::wstring& wszIndex, IN std::wstring& wszTarget)
{
	DN_ASSERT(NULL != pRoom,	"Invalid!");
#endif	// _VILLAGESERVER
	if( m_pScriptManager == NULL )
	{
		_DANGER_POINT();
		return false;
	}

#if defined(_VILLAGESERVER)
	CDNUserSession* pUser = static_cast<CDNUserSession*>(g_pUserSessionManager->FindUserSessionByObjectID(nUserObjectID));
	if(!pUser) return false;

#elif defined(_GAMESERVER)
	CDNUserSession * pUser = NULL;
	CDNRUDPGameServer * pServer = g_pGameServerManager->GetGameServerByUID(nUserObjectID);
	if(pServer == NULL)	
	{
		_DANGER_POINT();
		return false;
	}

	pUser = pServer->GetSession(nUserObjectID);
	if(!pUser) return false;
#endif	// _VILLAGESERVER


	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua )
	{
		_DANGER_POINT();
		return false;
	}

	char szIndex[512] = "";
	ZeroMemory(szIndex, sizeof(char)*512);
	WideCharToMultiByte( CP_ACP, 0, wszIndex.c_str(), -1, szIndex, 512, NULL, NULL );

	char szTarget[512] = "";
	ZeroMemory(szTarget, sizeof(char)*512);
	WideCharToMultiByte( CP_ACP, 0, wszTarget.c_str(), -1, szTarget, 512, NULL, NULL );

	const char* szFunctionName = m_strFuncNameOnTalk.c_str();

	//g_Log.Log(LogType::_QUESTHACK, pUser, L"CDNQuest::OnTalk func(%S) user(%d) npc(%d) index(%S) , target(%S) questIdx(%d)\n", szFunctionName, nUserObjectID, nNpcUniqueID, szIndex, szTarget,m_CreateInfo.nQuestIndex);

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, nNpcUniqueID, szIndex, szTarget, m_CreateInfo.nQuestIndex );
#elif defined(_GAMESERVER)
	//lua_tinker::call<void>(pLua, szFunctionName, pRoom, nUserObjectID, nNpcUniqueID, szIndex, szTarget, m_CreateInfo.nQuestIndex );
	m_pScriptManager->call<void>( pLua, szFunctionName, pRoom, nUserObjectID, nNpcUniqueID, szIndex, szTarget, m_CreateInfo.nQuestIndex );
#endif	// _VILLAGESERVER

#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnTalk func(%s) user(%d) npc(%d) index(%s) , target(%s) questIdx(%d) \n", szFunctionName, nUserObjectID, nNpcUniqueID,
		szIndex, szTarget,m_CreateInfo.nQuestIndex );

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD

	// If NPC could not respond
	if( pUser->GetCalledNpcResponse() == false )
	{
		pUser->SendNextTalkError();

#ifndef _FINAL_BUILD
		std::string szLocalMsg;
		szLocalMsg = FormatA("GetCalledNpcResponse is FALSE func(%s) user(%d) npc(%d) index(%s) , target(%s) questIdx(%d) \n", szFunctionName, nUserObjectID, nNpcUniqueID,
			szIndex, szTarget,m_CreateInfo.nQuestIndex );

#if defined(_VILLAGESERVER)
		api_log_UserLog(nUserObjectID, szLocalMsg.c_str() );
#elif defined(_GAMESERVER)
		api_log_UserLog(pRoom, nUserObjectID, szLocalMsg.c_str() );
#endif	// _VILLAGESERVER

#endif
	}

	return true;
}

#if defined(_VILLAGESERVER)
void CDNQuest::OnCompleteCounting(UINT nUserObjectID, TCount& Count)
{
#elif defined(_GAMESERVER)
void CDNQuest::OnCompleteCounting(CDNGameRoom *pRoom, UINT nUserObjectID, TCount& Count)
{
	DN_ASSERT(NULL != pRoom,	"Invalid!");
#endif	// _VILLAGESERVER
	if( m_pScriptManager == NULL )
		return;
	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua )
		return ;

	const char* szFunctionName = m_strFuncNameOnCompleteCounting.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex   );
#elif defined(_GAMESERVER)
	//lua_tinker::call<void>(pLua, szFunctionName, pRoom, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex   );
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex );
#endif	// _VILLAGESERVER

#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnCompleteCounting func(%s) user(%d) countType(%d) countIndex(%d) , Count.Cnt(%d) Count.TargetCnt(%d) QuestID(%d) \n", 
		szFunctionName, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt , m_CreateInfo.nQuestIndex  );

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD
}

#if defined(_VILLAGESERVER)
void CDNQuest::OnCounting(UINT nUserObjectID, TCount& Count)
{
#elif defined(_GAMESERVER)
void CDNQuest::OnCounting(CDNGameRoom *pRoom, UINT nUserObjectID, TCount& Count)
{
	DN_ASSERT(NULL != pRoom,	"Invalid!");
#endif	// _VILLAGESERVER
	if( m_pScriptManager == NULL )
		return;
	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua )
		return ;

	const char* szFunctionName = m_strFuncNameOnCounting.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex );
#elif defined(_GAMESERVER)
	//lua_tinker::call<void>(pLua, szFunctionName, pRoom, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex );
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex );
#endif	// _VILLAGESERVER

#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnCounting func(%s) user(%d) countType(%d) countIndex(%d) , Count.Cnt(%d) Count.TargetCnt(%d) QuestID(%d) \n", 
		szFunctionName, nUserObjectID, (int)Count.cType, (int)Count.nIndex, (int)Count.nCnt, (int)Count.nTargetCnt, m_CreateInfo.nQuestIndex );

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD
}

#if defined(_VILLAGESERVER)
void CDNQuest::OnAllCompleteCounting(UINT nUserObjectID)
{
#elif defined(_GAMESERVER)
void CDNQuest::OnAllCompleteCounting(CDNGameRoom *pRoom, UINT nUserObjectID)
{
	DN_ASSERT(NULL != pRoom,	"Invalid!");
#endif	// _VILLAGESERVER
	if( m_pScriptManager == NULL )
		return;
	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua )
		return ;

	const char* szFunctionName = m_strFuncNameOnCompleteAllCounting.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID,  m_CreateInfo.nQuestIndex );
#elif defined(_GAMESERVER)
	//lua_tinker::call<void>(pLua, szFunctionName, pRoom, nUserObjectID,  m_CreateInfo.nQuestIndex );
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID,  m_CreateInfo.nQuestIndex );
#endif	// _VILLAGESERVER

#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnCompleteAllCounting func(%s) user(%d) nQuestIndex(%d) \n", 
		szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex );

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD
}

#if defined(_VILLAGESERVER)
void CDNQuest::OnItemCollecting(UINT nUserObjectID, int nItemID, short wCount)
#elif defined(_GAMESERVER)
void CDNQuest::OnItemCollecting(CDNGameRoom *pRoom, UINT nUserObjectID, int nItemID, short wCount)
#endif
{
	if(m_pScriptManager == NULL) return;

	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua ) return;

	const char* szFunctionName = m_strFuncNameOnItemCollecting.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, nItemID, m_CreateInfo.nQuestIndex);
#elif defined(_GAMESERVER)
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID,  nItemID, m_CreateInfo.nQuestIndex);
#endif	// _VILLAGESERVER


#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnItemCollecting func(%s) user(%d) nQuestIndex(%d)\n", szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex);

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD

}

#if defined(PRE_ADD_REMOTE_QUEST)

#if defined(_VILLAGESERVER)
void CDNQuest::OnRemoteComplete(UINT nUserObjectID)
#elif defined(_GAMESERVER)
void CDNQuest::OnRemoteComplete(CDNGameRoom *pRoom, UINT nUserObjectID)
#endif
{
	if(m_pScriptManager == NULL) return;

	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua ) return;

	const char* szFunctionName = m_strFuncNameOnRemoteComplete.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex);
#elif defined(_GAMESERVER)
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID, m_CreateInfo.nQuestIndex);
#endif	// _VILLAGESERVER


#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnRemoteComplete func(%s) user(%d) nQuestIndex(%d)\n", szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex);

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD

}

#if defined(_VILLAGESERVER)
void CDNQuest::OnRemoteStart(UINT nUserObjectID)
#elif defined(_GAMESERVER)
void CDNQuest::OnRemoteStart(CDNGameRoom *pRoom, UINT nUserObjectID)
#endif
{
	if(m_pScriptManager == NULL) return;

	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua ) return;

	const char* szFunctionName = m_strFuncNameOnRemoteStart.c_str();

#if defined(_VILLAGESERVER)
	lua_tinker::call<void>(pLua, szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex);
#elif defined(_GAMESERVER)
	m_pScriptManager->call<void>(pLua, szFunctionName, pRoom, nUserObjectID, m_CreateInfo.nQuestIndex);
#endif	// _VILLAGESERVER


#ifndef _FINAL_BUILD
	std::string szMsg;
	szMsg = FormatA("OnRemoteStart func(%s) user(%d) nQuestIndex(%d)\n", szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex);

#if defined(_VILLAGESERVER)
	api_log_UserLog(nUserObjectID, szMsg.c_str() );
#elif defined(_GAMESERVER)
	api_log_UserLog(pRoom, nUserObjectID, szMsg.c_str() );
#endif	// _VILLAGESERVER
#endif // _FINAL_BUILD

}

#if defined(_VILLAGESERVER)
bool CDNQuest::CanRemoteCompleteStep(UINT nUserObjectID, UINT nQuestStep)
#elif defined(_GAMESERVER)
bool CDNQuest::CanRemoteCompleteStep(CDNGameRoom *pRoom, UINT nUserObjectID, UINT nQuestStep)
#endif
{
	if(m_pScriptManager == NULL) return false;

	lua_State* pLua = m_pScriptManager->OpenState();
	if( !pLua ) return false;

	const char* szFunctionName = m_strFuncNameCanRemoteCompleteStep.c_str();

	bool bResult = false;
#if defined(_VILLAGESERVER)
	bResult = lua_tinker::call<bool>(pLua, szFunctionName, nUserObjectID, m_CreateInfo.nQuestIndex, nQuestStep);
#elif defined(_GAMESERVER)
	bResult = m_pScriptManager->call<bool>(pLua, szFunctionName, pRoom, nUserObjectID, m_CreateInfo.nQuestIndex, nQuestStep);
#endif	// _VILLAGESERVER

	return bResult;
}


#endif // #if defined(PRE_ADD_REMOTE_QUEST)
