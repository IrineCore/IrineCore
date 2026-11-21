#include "stdafx.h"
#include "DNUserConnection.h"
#include "DNUserConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNSQLMembershipManager.h"
#include "DNSQLWorldManager.h"
#include "DNIocpManager.h"
#include "DNExtManager.h"
#include "Log.h"
#include "DNLogConnection.h"
#include "DNSecure.h"
#include "DNAuthManager.h"
#include "DNCountryUnicodeSet.h"
#include "TimeSet.h"
#include "Version.h"

extern TLoginConfig g_Config;

CDNUserConnection::CDNUserConnection(void)
{
	Clear();


	InitWrap(1024 * 100, 1024 * 100);
}

CDNUserConnection::~CDNUserConnection(void)
{

	SendDelUser();
}

int CDNUserConnection::InitWrap(int nRecvLen, int nSendLen, int nCompLen)
{
	int iRetVal = 0;

	__try {	// Start exception filter
		iRetVal = Init(nRecvLen, nSendLen, nCompLen);
	}
	__except(CExceptionCodeLog(GetExceptionCode(), L"CDNUserConnection::InitWrap()"), EXCEPTION_CONTINUE_SEARCH) {	// End exception filter // Do not handle exception here; look for upper exception handler
		// This code block is never executed
	}

	return iRetVal;
}

void CDNUserConnection::Clear()
{
	m_cGender = m_cDailyCreateCount = 0;

#if defined(PRE_MOD_SELECT_CHAR)
	m_nCreateCharacterSlotMaxPerAccount = g_pExtManager->GetGlobalWeightValue(Login_Max_CreateCharCount);
	m_nDefaultCreateCharacterSlotPerAccount = g_pExtManager->GetGlobalWeightValue(Login_Base_CreateCharCount);

	m_cAvailableCharacterSlotCount = m_nDefaultCreateCharacterSlotPerAccount;	// Use default settings initially

	memset(&m_SelectCharacterData, 0, sizeof(m_SelectCharacterData));

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	m_cSelectCharIndex = -1;
	m_cCharCountMax = CreateCharacterDefaultCountMax;

	m_cCharacterLimitCount = CreateCharacterDefaultCountMax + g_pExtManager->GetGlobalWeightValue(CharacterSlotMax);	// 34: Character slot count limit
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	
	m_cVillageID = 1;
	m_nChannelID = 0;
	m_cUserState = STATE_NONE;
	m_cAccountLevel = 0;
	m_nPrmInt1 = 0;
	m_cLastServerType = 0;

	m_cRegion = 0;

	m_nTutorialMapIndex = 0;
	m_cTutorialGateNo = 0;

#if defined(PRE_MOD_SELECT_CHAR)
	m_MapCharacterList.clear();
	m_MapWorldCharCount.clear();
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	memset(&m_CharDataArray, 0, sizeof(m_CharDataArray));
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	m_ChannelList.clear();
	

	m_nAccountDBID = 0;
	memset(&m_wszAccountName, 0, sizeof(m_wszAccountName));	// Account
	m_cWorldID = m_cWaitWorldID = 0;
	m_nWorldSetID = m_nWaitWorldSetID = 0;
	m_bAdult = false;
	m_cAge = 0;

	memset(&m_wszVirtualIp, 0, sizeof(m_wszVirtualIp));
	memset(&m_szVirtualIp, 0, sizeof(m_szVirtualIp));

	m_bPCBang = false;
	m_cPCBangGrade = 0;

	m_bIsSetSecondAuthPW	= false;
	m_bIsSetSecondAuthLock	= false;
	m_tSecondAuthResetDate	= -1;
#ifdef PRE_ADD_23829
	m_bPassSecondAuth = false;
#endif	//#ifdef PRE_ADD_23829
#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
	m_bCompletlyPassSecondAuth = false;
#endif


	m_PartnerType = Partner::Type::None;

	m_dwLastMessageTick = timeGetTime();
	m_cLastMainCmd = 0;
	m_cLastSubCmd = 0;
	m_dwSendInsideDisconnect = 0;
	m_cWaitWorldID = m_cWorldID = 0;

	// Auth
	m_bReconnected = false;
	m_bCertified = false;
	m_biCertifyingKey = 0;
#if !defined(PRE_MOD_SELECT_CHAR)
	m_bMasterAddUser = false;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

#if defined(PRE_ADD_LOGIN_USERCOUNT)
	m_bSummitUserCount = false;
	m_dwKickTick = timeGetTime();
#endif

#if defined(PRE_ADD_SENDLOGOUT)
	m_bSendLogOut = false;
#endif

#ifdef PRE_ADD_COMEBACK
	m_bComebackUser = false;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	m_bReConnectNewbieReward = false;
	m_eUserGameQuitReward = GameQuitReward::RewardType::None;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#ifdef PRE_ADD_DOORS
	m_bMobileAuthentication = false;
#endif		//#ifdef PRE_ADD_DOORS
	m_cCharacterSortCode = Login::CharacterSortCode::None;

#if defined( PRE_LOGINDUMP_LOG )
	memset(&m_ReconnectData, 0, sizeof(m_ReconnectData));
	m_nReconnectLoginStep = 0;
#endif
}

void CDNUserConnection::DoUpdate(DWORD CurTick)
{
#if defined(PRE_ADD_LOGIN_USERCOUNT)
	// Unauthenticated users get disconnected after 5 minutes.
	if( !IsCertified() && m_dwKickTick + CHECKNOTAUTHLOGINUSERKICKTICK <= CurTick)
	{
		DetachConnection(L"LOGIN Not Auth UserKick");
		return;
	}
	if( m_dwKickTick + CHECKLOGINUSERKICKTICK <= CurTick )
	{
		if (m_cUserState != STATE_WAITUSER)
			DetachConnection(L"LOGIN UserKick");
		return;
	}
#endif
}

int CDNUserConnection::OnRecvLoginMessage(int nSubCmd, const char * pData, int nLen)
{
	switch (nSubCmd)
	{
	case CS_CHECKVERSION: return RequestCheckVersion((CSCheckVersion*)pData, nLen);

	case CS_CHECKLOGIN:	return RequestCheckLogin((CSCheckLogin*)pData, nLen);
	case CS_CHECKLOGIN_US:	return RequestCheckLoginUS((const CSCheckLoginUS*)pData, nLen);

	case CS_SERVERLIST:	return RequestServerList(nLen);
	case CS_SELECTSERVER: return RequestSelectServer((CSSelectServer*)pData, nLen);
	case CS_SELECTCHAR: return RequestSelectCharacter((CSSelectChar*)pData, nLen);
	case CS_CREATECHAR: return RequestCreateCharacter((CSCreateChar*)pData, nLen);
	case CS_CREATEDWCCHAR: return RequestCreateDWCCharacter((CSCreateDWCChar*)pData, nLen);
	case CS_DELETECHAR: return RequestDeleteCharacter((CSDeleteChar*)pData, nLen);
	case CS_SELECTCHANNEL: return RequestSelectChannel((CSSelectChannel*)pData, nLen);
	case CS_BACKBUTTON: return RequestBackButton(nLen);
	case CS_CHANNELLIST: return RequestChannelList(nLen);
	case CS_BACKBUTTON_LOGIN: return RequestBackButton(nLen);
	case CS_REVIVECHAR: return RequestReviveCharacter((CSReviveChar*)pData, nLen);
	case CS_CHARACTERSORTCODE: return RequestCharacterSortCode((CSCharacterSortCode*)pData, nLen);
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserConnection::OnRecvSystemMessage(int nSubCmd, const char * pData, int nLen)
{
	switch(nSubCmd)
	{
	case CS_RECONNECTLOGIN:	return RequestReconnectLogin((CSReconnectLogin*)pData, nLen);	// When reconnecting from village/game back to login to show character select screen
	}
	return ERROR_UNKNOWN_HEADER;
}

int CDNUserConnection::OnRecvGameOptionMessage( int nSubCmd, const char* pData, int nLen )
{
	switch( nSubCmd )
	{
		case eGameOption::CS_UPDATE_SECONDAUTH_PASSWORD:	return RequestUpdateSecondAuthPassword( pData, nLen );
		case eGameOption::CS_INIT_SECONDAUTH:				return RequestInitSecondAuth( pData, nLen );
	}

	return ERROR_UNKNOWN_HEADER;
}

#ifdef PRE_ADD_DOORS
int CDNUserConnection::OnRecvDoorsMessage(int nSubCmd, const char * pData, int nLen)
{
	switch( nSubCmd )
	{
	case eDoors::CS_GET_AUTHKEY: return RequestGetDoorsAuthKey(pData, nLen);
	case eDoors::CS_CANCEL_AUTH: return RequestCancelDoorsAuth(pData, nLen);
	case eDoors::CS_GET_MOBILEAUTHFLAG: return RequestGetAuthFlag(pData, nLen);
	}

	return ERROR_UNKNOWN_HEADER;
}
#endif		//#ifdef PRE_ADD_DOORS

int CDNUserConnection::RequestCheckVersion(CSCheckVersion *pVersion, int nLen)
{
	if (sizeof(CSCheckVersion) != nLen)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_NONE){
		DetachConnection(L"RequestCheckVersion State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCheckVersion State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

	bool bVersionCheck = false;
	if ((pVersion->nMajorVersion == g_Config.nMajorVersion) && (pVersion->nMinorVersion == g_Config.nMinorVersion))
		bVersionCheck = true;

#ifdef _FINAL_BUILD
	if (bVersionCheck)		//Check only in final builds.
#endif //_FINAL_BUILD
	{
		SendCheckVersion(ERROR_NONE);	// Normal


		m_cUserState = STATE_CHECKVERSION;


		return ERROR_NONE;
	}
#ifdef _FINAL_BUILD
	else
	{
		SendCheckVersion(ERROR_LOGIN_VERSION_UNMATCH);	// Abnormal(?)
		return ERROR_GENERIC_INVALIDREQUEST;
	}
#endif //_FINAL_BUILD
	return ERROR_NONE;
}

int CDNUserConnection::_ProcessLoginCheck(const WCHAR *pUserId, const WCHAR *pPassword, const WCHAR *pVirtualIp)
{
	if (m_cUserState != STATE_CHECKVERSION){
		DetachConnection(L"RequestCheckLogin State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCheckLogin State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

	memset(m_wszAccountName, 0, sizeof(m_wszAccountName));
	wcsncpy(m_wszAccountName, pUserId, IDLENMAX - 1);
	WCHAR wszPassword[PASSWORDLENMAX] = { 0, };
	if (pPassword)
		wcsncpy(wszPassword, pPassword, PASSWORDLENMAX - 1);
	g_Log.Log(LogType::_NORMAL, this, L"ID:%s\r\n", m_wszAccountName);

	memset(m_wszVirtualIp, 0, sizeof(m_wszVirtualIp));
	if (pVirtualIp)
		wcsncpy(m_wszVirtualIp, pVirtualIp, IPLENMAX - 1);
	WideCharToMultiByte(CP_ACP, 0, m_wszVirtualIp, -1, m_szVirtualIp, IPLENMAX, NULL, NULL);

	int nRet = g_pSQLMembershipManager->QueryLogin(this, wszPassword, TRUE);
	if (nRet == ERROR_DB)
		nRet = ERROR_LOGIN_FAIL;

	SendCheckLogin(nRet);

	if (nRet == ERROR_NONE)
		m_cUserState = STATE_CHECKLOGIN;
	else
		SetAccountDBID(0);

	return ERROR_NONE;
}

int CDNUserConnection::RequestCheckLogin(CSCheckLogin *pLogin, int nLen)
{
	if (sizeof(CSCheckLogin) != nLen)
		return ERROR_INVALIDPACKET;

	WCHAR wszUserId[IDLENMAX] = { 0, };
	WCHAR wszPassword[PASSWORDLENMAX] = { 0, };
	WCHAR wszVirtualIp[IPLENMAX] = { 0, };
	wcsncpy(wszUserId, pLogin->wszUserId, IDLENMAX - 1);
	wcsncpy(wszPassword, pLogin->wszPassword, PASSWORDLENMAX - 1);
	wcsncpy(wszVirtualIp, pLogin->wszVirtualIp, IPLENMAX - 1);

	if (wszUserId[0] == 0 || wszPassword[0] == 0){
		DetachConnection(L"Login Fail");
		return ERROR_NONE;
	}

	return _ProcessLoginCheck(wszUserId, wszPassword, wszVirtualIp);
}

int CDNUserConnection::RequestCheckLoginUS(const CSCheckLoginUS *pLogin, int nLen)
{
	if (sizeof(CSCheckLoginUS) != nLen)
		return ERROR_INVALIDPACKET;

	WCHAR wszPassport[1024] = { 0, };
	WCHAR wszVirtualIp[IPLENMAX] = { 0, };
	wcsncpy(wszPassport, pLogin->wszPassport, _countof(wszPassport) - 1);
	wcsncpy(wszVirtualIp, pLogin->wszVirtualIp, IPLENMAX - 1);

	if (wszPassport[0] == 0){
		DetachConnection(L"Login Fail");
		return ERROR_NONE;
	}

	WCHAR wszAccount[IDLENMAX] = { 0, };
	WCHAR wszPassword[PASSWORDLENMAX] = { 0, };
	const WCHAR *pSep = wcspbrk(wszPassport, L":|\t");
	if (pSep){
		size_t nAccLen = static_cast<size_t>(pSep - wszPassport);
		if (nAccLen >= IDLENMAX) nAccLen = IDLENMAX - 1;
		wcsncpy(wszAccount, wszPassport, nAccLen);
		wcsncpy(wszPassword, pSep + 1, PASSWORDLENMAX - 1);
	}else{
		wcsncpy(wszAccount, wszPassport, IDLENMAX - 1);
	}

	if (wszAccount[0] == 0){
		DetachConnection(L"Login Fail");
		return ERROR_NONE;
	}

	return _ProcessLoginCheck(wszAccount, wszPassword, wszVirtualIp);
}

int CDNUserConnection::RequestServerList(int nLen)
{
	if (nLen != 0)
		return ERROR_INVALIDPACKET;

	if ((m_cUserState != STATE_CHECKLOGIN) && (m_cUserState != STATE_CHARLIST) && (m_cUserState != STATE_WAITUSER)){
		DetachConnection(L"RequestServerList State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestServerList State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

	SendDelUser();

#if defined(PRE_MOD_SELECT_CHAR)
	g_pSQLMembershipManager->QueryGetCharacterCount( this, m_MapWorldCharCount );	// Since this SP provides supplementary info, do not handle Ret separately.
	SendServerList( m_MapWorldCharCount );
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	std::map<int,int> mWorldCharCount;	// WorldID,CharCount
	g_pSQLMembershipManager->QueryGetCharacterCount( this, mWorldCharCount );	// Since this SP provides supplementary info, do not handle Ret separately.
	SendServerList( mWorldCharCount );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	m_cUserState = STATE_SERVERLIST;

	return ERROR_NONE;
}

int CDNUserConnection::RequestSelectServer(CSSelectServer *pSelectServer, int nLen)
{
	if (sizeof(CSSelectServer) != nLen)
		return ERROR_INVALIDPACKET;

	if ((m_cUserState != STATE_SERVERLIST) && (m_cUserState != STATE_CHECKLOGIN) && (m_cUserState != STATE_CHANNELLIST)){
		DetachConnection(L"RequestSelectServer State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestSelectServer State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}
	// Retrieve 2nd auth info on SelectServer.
	int iRet = g_pSQLMembershipManager->QueryGetSecondAuthStatus( m_nAccountDBID, m_bIsSetSecondAuthPW, m_bIsSetSecondAuthLock, m_tSecondAuthResetDate );
	if( iRet != ERROR_NONE )
	{
		DetachConnection(L"QueryGetSecondAuthStatus Failed");
		return ERROR_NONE;
	}

	// If 2nd password is set, but 2nd security account Lock is not set
	// Force 2nd security account Lock state on SelectServer.
	if( m_bIsSetSecondAuthPW && !m_bIsSetSecondAuthLock )
	{
		iRet = g_pSQLMembershipManager->QueryModSecondAuthLockFlag( m_nAccountDBID, true );
		if( iRet == ERROR_NONE )
		{
			m_bIsSetSecondAuthLock = true;
		}
		else
		{
			DetachConnection(L"QueryModSecondAuthLockFlag Failed");
			return ERROR_NONE;
		}
	}

#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
	m_bPassSecondAuth = (CheckSndaAuthFlag(eChSndaAuthFlag_UseEKey) || CheckSndaAuthFlag(eChSndaAuthFlag_UseECard));
	if (m_bIsSetSecondAuthPW)
		m_bCompletlyPassSecondAuth = true;
#endif

#ifdef PRE_ADD_23829
	if (m_bPassSecondAuth == false && m_bIsSetSecondAuthPW == false)
	{
		//Check if e-Key, e-Card, and 2nd auth are all unused.
		iRet = g_pSQLMembershipManager->QueryCheckLastSecondAuthNotifyDate(m_nAccountDBID, SECONDAUTHIGNOREPERIOD);
		if (iRet == ERROR_NONE)		//If check period still remains, skip the prompt page!!!!
			m_bPassSecondAuth = true;
	}	
#endif

	// Send 2nd auth info.
	SendSecondAuthInfo();

	if (CHECK_RANGE(pSelectServer->cWorldID, 1, WORLDCOUNTMAX)) 
	{
		//Check before setting.
	}
	else if (pSelectServer->cWorldID != -1)
	{
		DetachConnection(L"ServerIndex mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] pSelectServer->cWorldID (%d)\r\n", m_nAccountDBID, m_nSessionID, pSelectServer->cWorldID);
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)

	SetWorldID(pSelectServer->cWorldID);

	SendCharacterList(false);

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (m_bMasterAddUser == false)
	{
#ifdef _USE_ACCEPTEX
		ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif

		int nWorldSetID = pSelectServer->cWorldID;
		nWorldSetID = g_pExtManager->GetWorldSetID(nWorldSetID);

		CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(nWorldSetID, false));
		if (pMasterCon)
			pMasterCon->SendAddUser(this, pSelectServer->cWorldID);
		else
		{
			DetachConnection(L"MasterCon NOtFound");
			g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] pSelectServer->cWorldID (%d)\r\n", m_nAccountDBID, m_nSessionID, pSelectServer->cWorldID);
			return ERROR_NONE;
		}
	}
	else
	{
		SendCharacterList(false);
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return ERROR_NONE;
}

bool CDNUserConnection::CheckAbuserCharLength()
{
#if defined(PRE_MOD_SELECT_CHAR)
	if (m_MapCharacterList.empty()) return true;

	TMapCharacterList::iterator iter;
	for (iter = m_MapCharacterList.begin(); iter != m_MapCharacterList.end(); iter++)
	{
		if (iter->second.biCharacterDBID <= 0 ) 
			continue;		

		int nMaxLength = g_pExtManager->GetGlobalWeightValue(CHARACTERNAME_MAXLENGTH);

		if( (nMaxLength > 0) && ((int)(wcslen(iter->second.wszCharacterName)) >= nMaxLength) )
			return false;
	}

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	for (int i = 0; i < m_cCharacterLimitCount; i++)
	{
		if (m_CharDataArray[i].biCharacterDBID <= 0 ) 
			continue;		

		int nMaxLength = g_pExtManager->GetGlobalWeightValue(CHARACTERNAME_MAXLENGTH);

		if( (nMaxLength > 0) && ((int)(wcslen(m_CharDataArray[i].wszCharacterName)) >= nMaxLength) )
			return false;
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return true;
}

void CDNUserConnection::QueryAddRestraint(wchar_t* pwszReason)
{
	//Permanent block
	TIMESTAMP_STRUCT StartDate, EndDate;
	// Start time
	CTimeSet TimeSet;
	memcpy( &StartDate, &TimeSet.GetDbTimeStamp(), sizeof(TIMESTAMP_STRUCT ));
	// End time.. block for 10 years
	TimeSet.Set(TimeSet.GetYear()+10, TimeSet.GetMonth(), TimeSet.GetDay(), TimeSet.GetHour(), TimeSet.GetMinute(), TimeSet.GetSecond() );
	memcpy( &EndDate, &TimeSet.GetDbTimeStamp(), sizeof(TIMESTAMP_STRUCT ));

	g_pSQLMembershipManager->QueryAddRestraint( DBDNWorldDef::RestraintTargetCode::Account, m_nAccountDBID, 0, 110, DBDNWorldDef::RestraintTypeCode::ConnectRestraint, pwszReason, NULL, StartDate, EndDate );
	DetachConnection( pwszReason );
	g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] CheckAbuserCharLength Restraint \r\n", m_nAccountDBID, m_nSessionID);
}

int CDNUserConnection::QueryCharacterListAndSecondAuth( bool bSecondAuthProcess )
{
	ClearCharacterData();
#if defined(PRE_MOD_SELECT_CHAR)
	SetAvailableCharacterSlotCount();
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	m_cCharCountMax = CreateCharacterDefaultCountMax;
	g_pSQLMembershipManager->QueryGetCharacterSlotCount(m_nAccountDBID, m_cWorldID, 0, m_cCharCountMax);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)


#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 16;
#endif

#if defined(PRE_MOD_SELECT_CHAR)
	g_pSQLMembershipManager->QueryGetCharacterCount( this, m_MapWorldCharCount );	// Since this SP provides supplementary info, do not handle Ret separately.

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 17;
#endif

	int nRet = ERROR_NONE;
	if (!m_MapWorldCharCount.empty()){
		nRet = ERROR_DB;
		for (TMapWorldCharCount::iterator iter = m_MapWorldCharCount.begin(); iter != m_MapWorldCharCount.end(); iter++){
			int nWorldSetID = g_pExtManager->GetWorldSetID(iter->first);

			nRet = g_pSQLWorldManager->QueryGetListCharacter(nWorldSetID, iter->first, m_nAccountDBID, m_cAvailableCharacterSlotCount, m_MapCharacterList);
			if (nRet != ERROR_NONE)
				return nRet;
		}
	}
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 18;
#endif

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int nWorldID = m_cWorldID;

	int nRet = g_pSQLWorldManager->QueryGetListCharacter(m_nWorldSetID, nWorldID, m_nAccountDBID, m_cCharCountMax, m_CharDataArray);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	if( nRet == ERROR_NONE ) 
	{
		if( bSecondAuthProcess )
		{
#if defined( PRE_LOGINDUMP_LOG )	
			m_nReconnectLoginStep = 19;
#endif
			// Get 2nd auth info
			if( g_pSQLMembershipManager->QueryGetSecondAuthStatus( m_nAccountDBID, m_bIsSetSecondAuthPW, m_bIsSetSecondAuthLock, m_tSecondAuthResetDate ) == ERROR_NONE )
			{
#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
				m_bPassSecondAuth = (CheckSndaAuthFlag(eChSndaAuthFlag_UseEKey) || CheckSndaAuthFlag(eChSndaAuthFlag_UseECard));
				if (m_bIsSetSecondAuthPW)
					m_bCompletlyPassSecondAuth = true;
#endif

#if defined( PRE_LOGINDUMP_LOG )	
				m_nReconnectLoginStep = 20;
#endif

#ifdef PRE_ADD_23829
				if (m_bPassSecondAuth == false && m_bIsSetSecondAuthPW == false)
				{
					//Check if e-Key, e-Card, and 2nd auth are all unused.
					int iRet = g_pSQLMembershipManager->QueryCheckLastSecondAuthNotifyDate(m_nAccountDBID, SECONDAUTHIGNOREPERIOD);
					if (iRet == ERROR_NONE)		//If check period still remains, skip the prompt page!!!!
						m_bPassSecondAuth = true;
				}	
#endif
#if defined( PRE_LOGINDUMP_LOG )	
				m_nReconnectLoginStep = 21;
#endif

				SendSecondAuthInfo();
				m_cUserState = STATE_CHARLIST;
			}
			else
			{
				_DANGER_POINT();
				nRet = ERROR_LOGIN_CHARACTERLIST_ERROR;
			}
		}
		else
		{
			m_cUserState = STATE_CHARLIST;
		}
	}

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 22;
#endif
	return nRet;
}

int CDNUserConnection::QueryValidataSecondAuthPassphrase( WCHAR* pwszPW, BYTE& cFailCount, bool bSetIgnoreSecondAuth )
{
	// If the account has a 2nd password set, verify the 2nd password
	if( m_bIsSetSecondAuthPW )
	{
		WCHAR wszSecondAuthPW[SecondAuth::Common::PWMaxLength+1];
		memset( wszSecondAuthPW, 0, sizeof(wszSecondAuthPW) );
		memcpy( wszSecondAuthPW, pwszPW, sizeof(WCHAR)*SecondAuth::Common::PWMaxLength );

		int nRet = g_pSQLMembershipManager->QueryValidataSecondAuthPassphrase( m_nAccountDBID, wszSecondAuthPW, cFailCount );
		if( nRet != ERROR_NONE )
		{
			if( cFailCount >= SecondAuth::Common::LimitCount )
			{
				TIMESTAMP_STRUCT StartDate,EndDate;
				// Start time
				CTimeSet TimeSet;
				memcpy( &StartDate, &TimeSet.GetDbTimeStamp(), sizeof(TIMESTAMP_STRUCT ));
				// End time
				TimeSet.AddSecond( 60*SecondAuth::Common::RestraintTimeMin );
				memcpy( &EndDate, &TimeSet.GetDbTimeStamp(), sizeof(TIMESTAMP_STRUCT ));

				g_pSQLMembershipManager->QueryAddRestraint( DBDNWorldDef::RestraintTargetCode::Account, m_nAccountDBID, 0, SecondAuth::Common::RestraintReasonID, DBDNWorldDef::RestraintTypeCode::ConnectRestraint, NULL, NULL, StartDate, EndDate );
				SendCompleteDetachMsg( ERROR_SECONDAUTH_CHECK_MAXFAILED, L"SecondAuth Restraint" );
			}
		}

		return nRet;
	}
#ifdef PRE_ADD_23829
	else if (bSetIgnoreSecondAuth)
	{
		int nRet = g_pSQLMembershipManager->QueryModLastSecondAuthNotifyDate(m_nAccountDBID);
		if (nRet == ERROR_NONE)
		{
			m_bPassSecondAuth = true;
			SendSecondAuthInfo();
		}
		return nRet;
	}
#endif

	return ERROR_NONE;
}

int CDNUserConnection::RequestSelectCharacter(CSSelectChar *pSelect, int nLen)
{
	if (sizeof(CSSelectChar) != nLen)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_CHARLIST){
		DetachConnection(L"RequestSelectCharacter State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestSelectCharacter State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	if (!IsValidCharacter(pSelect->biCharacterDBID)){
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (!IsValidCharacterIndex(pSelect->cCharIndex)){
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
		DetachConnection(L"Select Char Fail(Index not found)");
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	int cSelectWorldSetID = GetWorldSetIDByCharacterDBID(pSelect->biCharacterDBID);
	CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(cSelectWorldSetID, false));
	if (!pMasterCon){
		SendChannelList(ERROR_GENERIC_MASTERCON_NOT_FOUND, 0);
		return ERROR_NONE;
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	BYTE cFailCount = 0;
#ifdef PRE_ADD_23829
	int iRet = QueryValidataSecondAuthPassphrase( (WCHAR*)MakeSecondPasswordString( szVersion, pSelect->nSeed, pSelect->nValue ).c_str(), cFailCount, pSelect->bSetIgnoreAuthNotify );
#else
	int iRet = QueryValidataSecondAuthPassphrase( (WCHAR*)MakeSecondPasswordString( szVersion, pSelect->nSeed, pSelect->nValue ).c_str(), cFailCount );
#endif
	if( iRet != ERROR_NONE )
	{
		SendChannelList( iRet, cFailCount );
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	SetSelectCharacterData(pSelect->biCharacterDBID);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	m_cSelectCharIndex = pSelect->cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	// Tutorial setup
	if (GetSelectCharacterMapIndex() == 0){
#if defined(PRE_MOD_SELECT_CHAR)
		g_pExtManager->GetCreateDefaultMapInfo(m_SelectCharacterData.cJob, m_SelectCharacterData.nMapID, m_nTutorialMapIndex, m_cTutorialGateNo);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
		g_pExtManager->GetCreateDefaultMapInfo(SelectCharData()->cJob, SelectCharData()->nMapID, m_nTutorialMapIndex, m_cTutorialGateNo);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	}

	SendChannelList();

	m_cUserState = STATE_CHANNELLIST;

	return ERROR_NONE;
}

int CDNUserConnection::RequestCreateCharacter(CSCreateChar * pCreate, int nLen)
{
	if (sizeof(CSCreateChar) != nLen)
		return ERROR_INVALIDPACKET;

	if ((m_cUserState != STATE_CHARLIST)/* && (m_cUserState != STATE_CREATEDCHAR)*/){
		DetachConnection(L"RequestCreateCharacter State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCreateCharacter State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}
	if (m_nAccountDBID == 0){
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_CREATE_CHARACTER_FAIL);
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] CreateChar Fail (%s)\r\n", m_nAccountDBID, m_nSessionID, pCreate->wszCharacterName);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if (m_cDailyCreateCount == 0){	// Cannot create any more
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY);
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] CreateChar Fail (%s)\r\n", m_nAccountDBID, m_nSessionID, pCreate->wszCharacterName);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	//Exceeded max name length
	int nMaxLength = g_pExtManager->GetGlobalWeightValue(CHARACTERNAME_MAXLENGTH);
	if((nMaxLength > 0) && ((int)(wcslen(pCreate->wszCharacterName)) > nMaxLength))
	{
		QueryAddRestraint(L"CheckAbuserCharLength");
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	// #52058 Below minimum name length
	if( (int)(wcslen(pCreate->wszCharacterName)) < CHARNAMEMIN )
	{
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	DWORD dwCheckType = ALLOW_STRING_DEFAULT;
	if (!g_CountryUnicodeSet.Check(pCreate->wszCharacterName, dwCheckType)) {
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if (CheckLastSpace(pCreate->wszCharacterName))
	{
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if (g_pExtManager->CheckProhibitWord(pCreate->wszCharacterName)){	// If in the forbidden words list
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	if (GetEmptyCharacterSlotCount() <= 0){
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_ANYMORE);	// Need to define retcode
		return ERROR_GENERIC_UNKNOWNERROR;
	}

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	char cCharIndex = GetEmptyCharIndex();
	if (cCharIndex < 0){
		SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_ANYMORE);	// Need to define retcode
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	for (int i = 0; i < EQUIPMAX; i++){
		if (pCreate->nEquipArray[i] > 0){
			if (!g_pExtManager->IsDefaultParts(pCreate->cClass, i, pCreate->nEquipArray[i])){
				SendCreateChar(pCreate->wszCharacterName, ERROR_LOGIN_PARTS_NOT_FOUND);	// Need to define retcode
				return ERROR_GENERIC_UNKNOWNERROR;
			}
		}
	}
	if( g_pExtManager->IsDefaultHairColor( pCreate->cClass, pCreate->dwHairColor ) == false ) {
		SendCreateChar( pCreate->wszCharacterName, ERROR_LOGIN_HAIRCOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
	if( g_pExtManager->IsDefaultSkinColor( pCreate->cClass, pCreate->dwSkinColor ) == false ) {
		SendCreateChar( pCreate->wszCharacterName, ERROR_LOGIN_SKINCOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
	if( g_pExtManager->IsDefaultEyeColor( pCreate->cClass, pCreate->dwEyeColor ) == false ) {
		SendCreateChar( pCreate->wszCharacterName, ERROR_LOGIN_EYECOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	TCreateCharacterItem CreateItemArray[DEFAULTITEMMAX] = { 0, };
	int nCount = g_pExtManager->GetCreateDefaultItem(pCreate->cClass, CreateItemArray);
	
	TPosition Position = { 0, };
	g_pExtManager->GetCreateDefaultPosition(pCreate->cClass, Position.nX, Position.nZ);

	int nSkillArray[DEFAULTSKILLMAX] = { 0, };
	g_pExtManager->GetCreateDefaultSkill(pCreate->cClass, nSkillArray);

	int nUnlockSkillArray[DEFAULTUNLOCKSKILLMAX] = { 0, };
	g_pExtManager->GetCreateDefaultUnlockSkill(pCreate->cClass, nUnlockSkillArray);

	TQuickSlot QuickSlotArray[DEFAULTQUICKSLOTMAX] = { 0, };
	g_pExtManager->GetCreateDefaultQuickSlot(pCreate->cClass, QuickSlotArray);

	int nGestureArr[DEFAULTGESTUREMAX] = { 0, };
	g_pExtManager->GetCreateDefaultGesture(pCreate->cClass, nGestureArr);

#if defined(PRE_MOD_SELECT_CHAR)
	if( pCreate->cWorldID <= 0 )
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if( m_cWorldID <= 0 )
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	{
		DetachConnection(L"RequestCreateCharacter WorldID Mismatch");
#if defined(PRE_MOD_SELECT_CHAR)
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCreateCharacter WorldID Mismatch(worldid:%d)\r\n", m_nAccountDBID, m_nSessionID, static_cast<int>(pCreate->cWorldID) );
#else	// #if defined(PRE_MOD_SELECT_CHAR)
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCreateCharacter WorldID Mismatch(worldid:%d)\r\n", m_nAccountDBID, m_nSessionID, static_cast<int>(m_cWorldID) );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
		return ERROR_NONE;
	}

	INT64 biCharacterDBID = 0;
	TIMESTAMP_STRUCT tCreateDate = {0,};
	int nRet = 0;
#if defined(PRE_MOD_SELECT_CHAR)
	nRet = g_pSQLMembershipManager->QueryAddCharacter(m_nAccountDBID, pCreate->wszCharacterName, pCreate->cWorldID, m_nDefaultCreateCharacterSlotPerAccount, biCharacterDBID, tCreateDate);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	nRet = g_pSQLMembershipManager->QueryAddCharacter(m_nAccountDBID, pCreate->wszCharacterName, m_cWorldID, 0, biCharacterDBID, tCreateDate);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	if (nRet == ERROR_NONE){
		int nPCBangRebirthCoin = 0;
		nPCBangRebirthCoin = g_pExtManager->GetPCBangParam1(PCBang::Type::RebirthCoin, m_cPCBangGrade);

		UINT nBeginnerGuildDBID = 0;
#if defined(PRE_MOD_SELECT_CHAR)

		int nWorldSetID = g_pExtManager->GetWorldSetID(pCreate->cWorldID);
#ifdef PRE_ADD_BEGINNERGUILD
		nRet = g_pSQLWorldManager->QueryAddCharacter(nWorldSetID, biCharacterDBID, m_nAccountDBID, m_wszAccountName, m_cAccountLevel, pCreate->cWorldID, pCreate->wszCharacterName, pCreate->cClass, 0, pCreate->dwHairColor, pCreate->dwEyeColor, 
			pCreate->dwSkinColor, 0, Position, 0, g_pExtManager->GetRebirthCoin(1, nWorldSetID), nPCBangRebirthCoin, pCreate->nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray, CreateItemArray, nCount, true, nBeginnerGuildDBID, GetwszIp() );
#else		//#ifdef PRE_ADD_BEGINNERGUILD
		nRet = g_pSQLWorldManager->QueryAddCharacter(nWorldSetID, biCharacterDBID, m_nAccountDBID, m_wszAccountName, m_cAccountLevel, pCreate->cWorldID, pCreate->wszCharacterName, pCreate->cClass, 0, pCreate->dwHairColor, pCreate->dwEyeColor, 
			pCreate->dwSkinColor, 0, Position, 0, g_pExtManager->GetRebirthCoin(1, nWorldSetID), nPCBangRebirthCoin, pCreate->nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray, CreateItemArray, nCount, false, nBeginnerGuildDBID, GetwszIp() );
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#else	// #if defined(PRE_MOD_SELECT_CHAR)

#ifdef PRE_ADD_BEGINNERGUILD
		nRet = g_pSQLWorldManager->QueryAddCharacter(m_nWorldSetID, biCharacterDBID, m_nAccountDBID, m_wszAccountName, m_cAccountLevel, m_cWorldID, pCreate->wszCharacterName, pCreate->cClass, cCharIndex, pCreate->dwHairColor, pCreate->dwEyeColor, 
			pCreate->dwSkinColor, 0, Position, 0, g_pExtManager->GetRebirthCoin(1, m_nWorldSetID), nPCBangRebirthCoin, pCreate->nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray, CreateItemArray, nCount, true, nBeginnerGuildDBID, GetwszIp() );
#else		//#ifdef PRE_ADD_BEGINNERGUILD
		nRet = g_pSQLWorldManager->QueryAddCharacter(m_nWorldSetID, biCharacterDBID, m_nAccountDBID, m_wszAccountName, m_cAccountLevel, m_cWorldID, pCreate->wszCharacterName, pCreate->cClass, cCharIndex, pCreate->dwHairColor, pCreate->dwEyeColor, 
			pCreate->dwSkinColor, 0, Position, 0, g_pExtManager->GetRebirthCoin(1, m_nWorldSetID), nPCBangRebirthCoin, pCreate->nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray, CreateItemArray, nCount, false, nBeginnerGuildDBID, GetwszIp() );
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

#endif	// #if defined(PRE_MOD_SELECT_CHAR)

		if (nRet == ERROR_NONE){
			DelDailyCreateCount();
			m_cUserState = STATE_SERVERLIST;

#ifdef PRE_ADD_BEGINNERGUILD
			MAAddGuildMember AddGuild;
			memset(&AddGuild, 0, sizeof(MAAddGuildMember));

			AddGuild.nManagedID = g_Config.nManagedID;
			AddGuild.GuildUID.Set(m_nWorldSetID, nBeginnerGuildDBID);
			AddGuild.cLevel = 1;
			time(&AddGuild.JoinDate);
			AddGuild.Location.Reset();
			AddGuild.Location.cServerLocation = static_cast<BYTE>(_LOCATION_LOGIN);
			AddGuild.nAccountDBID = m_nAccountDBID;
			AddGuild.nCharacterDBID = biCharacterDBID;
			AddGuild.nJob = pCreate->cClass;
			_wcscpy(AddGuild.wszCharacterName, NAMELENMAX, pCreate->wszCharacterName, NAMELENMAX);
#ifdef _USE_ACCEPTEX
			ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
			CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(m_nWorldSetID, false));
			if (pMasterCon)
				pMasterCon->SendGuildMemberAdd(&AddGuild);

#if defined(PRE_MOD_SELECT_CHAR)
			int nWillSendRet = g_pSQLWorldManager->QueryAddWillSendMail(nWorldSetID, biCharacterDBID, g_pExtManager->GetGlobalWeightValue(BeginnerGuild_GiftMailID), L"");
#else	// #if defined(PRE_MOD_SELECT_CHAR)
			int nWillSendRet = g_pSQLWorldManager->QueryAddWillSendMail(m_nWorldSetID, biCharacterDBID, g_pExtManager->GetGlobalWeightValue(BeginnerGuild_GiftMailID), L"");
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
			if (nWillSendRet != ERROR_NONE)
				g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] QueryAddWillSendMail Error(ret:%d)\r\n", m_nAccountDBID, m_nSessionID, nWillSendRet);
			
#endif		//#ifdef PRE_ADD_BEGINNERGUILD

		}
		else{
#if defined(PRE_MOD_SELECT_CHAR)
			g_pSQLWorldManager->QueryRollbackAddCharacter(nWorldSetID, biCharacterDBID);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
			g_pSQLWorldManager->QueryRollbackAddCharacter(m_nWorldSetID, biCharacterDBID);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
			g_pSQLMembershipManager->QueryRollbackAddCharacter(biCharacterDBID);
			g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] QueryAddCharacter Error(ret:%d)\r\n", m_nAccountDBID, m_nSessionID, nRet );
		}
	}

	SendCreateChar(pCreate->wszCharacterName, nRet);

	return ERROR_NONE;
}

int CDNUserConnection::RequestCreateDWCCharacter(CSCreateDWCChar * pCreate, int nLen)
{
	if (sizeof(CSCreateDWCChar) != nLen)
		return ERROR_INVALIDPACKET;

	pCreate->wszCharacterName[NAMELENMAX - 1] = L'\0';

	if (m_cUserState != STATE_CHARLIST){
		DetachConnection(L"RequestCreateDWCCharacter State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCreateDWCCharacter State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}
	if (m_nAccountDBID == 0){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_CREATE_CHARACTER_FAIL);
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] CreateDWCChar Fail (%s)\r\n", m_nAccountDBID, m_nSessionID, pCreate->wszCharacterName);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if (m_cDailyCreateCount == 0){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY);
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] CreateDWCChar DailyLimit (%s)\r\n", m_nAccountDBID, m_nSessionID, pCreate->wszCharacterName);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	int nMaxLength = g_pExtManager->GetGlobalWeightValue(CHARACTERNAME_MAXLENGTH);
	if ((nMaxLength > 0) && ((int)(wcslen(pCreate->wszCharacterName)) > nMaxLength)){
		QueryAddRestraint(L"CheckAbuserCharLength");
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if ((int)(wcslen(pCreate->wszCharacterName)) < CHARNAMEMIN)
		return ERROR_GENERIC_UNKNOWNERROR;

	DWORD dwCheckType = ALLOW_STRING_DEFAULT;
	if (!g_CountryUnicodeSet.Check(pCreate->wszCharacterName, dwCheckType)){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if (CheckLastSpace(pCreate->wszCharacterName)){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_INVALIDREQUEST;
	}

	if (g_pExtManager->CheckProhibitWord(pCreate->wszCharacterName)){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_PROHIBITWORD);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex = 0;
	if (GetEmptyCharacterSlotCount() <= 0){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_ANYMORE);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#else
	char cCharIndex = GetEmptyCharIndex();
	if (cCharIndex < 0){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_CANNOT_CREATE_CHARACTER_ANYMORE);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#endif

	for (int i = 0; i < EQUIPMAX; i++){
		if (pCreate->nEquipArray[i] > 0){
			if (!g_pExtManager->IsDefaultParts(pCreate->cClass, i, pCreate->nEquipArray[i])){
				SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_PARTS_NOT_FOUND);
				return ERROR_GENERIC_UNKNOWNERROR;
			}
		}
	}
	if (g_pExtManager->IsDefaultHairColor(pCreate->cClass, pCreate->dwHairColor) == false){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_HAIRCOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
	if (g_pExtManager->IsDefaultSkinColor(pCreate->cClass, pCreate->dwSkinColor) == false){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_SKINCOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}
	if (g_pExtManager->IsDefaultEyeColor(pCreate->cClass, pCreate->dwEyeColor) == false){
		SendCreateDWCChar(pCreate->wszCharacterName, ERROR_LOGIN_EYECOLOR_NOT_FOUND);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	TCreateCharacterItem CreateItemArray[DEFAULTITEMMAX] = { 0, };
	int nCount = g_pExtManager->GetCreateDefaultItem(pCreate->cClass, CreateItemArray);

	TPosition Position = { 0, };
	g_pExtManager->GetCreateDefaultPosition(pCreate->cClass, Position.nX, Position.nZ);

	int nSkillArray[DEFAULTSKILLMAX] = { 0, };
	g_pExtManager->GetCreateDefaultSkill(pCreate->cClass, nSkillArray);

	int nUnlockSkillArray[DEFAULTUNLOCKSKILLMAX] = { 0, };
	g_pExtManager->GetCreateDefaultUnlockSkill(pCreate->cClass, nUnlockSkillArray);

	TQuickSlot QuickSlotArray[DEFAULTQUICKSLOTMAX] = { 0, };
	g_pExtManager->GetCreateDefaultQuickSlot(pCreate->cClass, QuickSlotArray);

#if defined(PRE_MOD_SELECT_CHAR)
	BYTE cTargetWorldID = pCreate->cWorldID;
#else
	BYTE cTargetWorldID = static_cast<BYTE>(m_cWorldID);
#endif
	if (cTargetWorldID <= 0){
		DetachConnection(L"RequestCreateDWCCharacter WorldID Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestCreateDWCCharacter WorldID Mismatch(worldid:%d)\r\n", m_nAccountDBID, m_nSessionID, static_cast<int>(cTargetWorldID));
		return ERROR_NONE;
	}

	BYTE cLevel = (pCreate->cLevel > 0) ? pCreate->cLevel : 60;
	BYTE cJobCode1 = (pCreate->cJobCode1 > 0) ? pCreate->cJobCode1 : pCreate->cClass;
	BYTE cJobCode2 = pCreate->cJobCode2;
	int nExp = (pCreate->nExp >= 0) ? pCreate->nExp : 0;
	int nMapID = (pCreate->nMapID > 0) ? pCreate->nMapID : 0;
	int nCoin = (pCreate->nCoin >= 0) ? pCreate->nCoin : 0;
	int nSkillPoint = (pCreate->nSkillPoint >= 0) ? pCreate->nSkillPoint : 0;

	INT64 biCharacterDBID = 0;
	TIMESTAMP_STRUCT tCreateDate = { 0, };
	int nRet = 0;
#if defined(PRE_MOD_SELECT_CHAR)
	nRet = g_pSQLMembershipManager->QueryAddCharacter(m_nAccountDBID, pCreate->wszCharacterName, cTargetWorldID, m_nDefaultCreateCharacterSlotPerAccount, biCharacterDBID, tCreateDate);
	int nWorldSetID = g_pExtManager->GetWorldSetID(cTargetWorldID);
#else
	nRet = g_pSQLMembershipManager->QueryAddCharacter(m_nAccountDBID, pCreate->wszCharacterName, cTargetWorldID, 0, biCharacterDBID, tCreateDate);
	int nWorldSetID = m_nWorldSetID;
#endif

	if (nRet == ERROR_NONE){
		BYTE cPCBangRebirthCoin = static_cast<BYTE>(g_pExtManager->GetPCBangParam1(PCBang::Type::RebirthCoin, m_cPCBangGrade));
		BYTE cRebirthCoin = static_cast<BYTE>(g_pExtManager->GetRebirthCoin(1, nWorldSetID));

		nRet = g_pSQLWorldManager->QueryAddDWCCharacter(
			nWorldSetID, biCharacterDBID, m_nAccountDBID, m_wszAccountName, cTargetWorldID, pCreate->wszCharacterName,
			pCreate->cClass, cJobCode1, cJobCode2, cLevel, nExp, static_cast<BYTE>(cCharIndex),
			pCreate->dwHairColor, pCreate->dwEyeColor, pCreate->dwSkinColor,
			nMapID, Position, 0.0f, cRebirthCoin, cPCBangRebirthCoin,
			0, nCoin, nSkillPoint,
			pCreate->nEquipArray, nSkillArray, nUnlockSkillArray, QuickSlotArray,
			CreateItemArray, static_cast<BYTE>(nCount), GetwszIp());

		if (nRet == ERROR_NONE){
			DelDailyCreateCount();
			m_cUserState = STATE_SERVERLIST;
		}
		else{
			g_pSQLWorldManager->QueryRollbackAddCharacter(nWorldSetID, biCharacterDBID);
			g_pSQLMembershipManager->QueryRollbackAddCharacter(biCharacterDBID);
			g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] QueryAddDWCCharacter Error(ret:%d)\r\n", m_nAccountDBID, m_nSessionID, nRet);
		}
	}

	SendCreateDWCChar(pCreate->wszCharacterName, nRet);
	return ERROR_NONE;
}

int CDNUserConnection::RequestDeleteCharacter(CSDeleteChar * pDelete, int nLen)
{
	if (sizeof(CSDeleteChar) != nLen)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_CHARLIST){
		DetachConnection(L"RequestDeleteCharacter State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestDeleteCharacter State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	if (!IsValidCharacter(pDelete->biCharacterDBID))
	{
		SendDeleteChar(pDelete->biCharacterDBID, ERROR_LOGIN_CHARACTER_INDEX_UNMATCH);	// Need to define retcode
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (!IsValidCharacterIndex(pDelete->cCharIndex))
	{
		SendDeleteChar(pDelete->cCharIndex, ERROR_LOGIN_CHARACTER_INDEX_UNMATCH);	// Need to define retcode
		return ERROR_GENERIC_UNKNOWNERROR;
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	BYTE cFailCount = 0;
	std::wstring wszSecondAuthPW = MakeSecondPasswordString( szVersion, pDelete->nSeed, pDelete->nValue );
	int iRet = QueryValidataSecondAuthPassphrase( (WCHAR*)wszSecondAuthPW.c_str(), cFailCount );

	if( iRet != ERROR_NONE )
	{
#if defined(PRE_MOD_SELECT_CHAR)
		SendDeleteChar(pDelete->biCharacterDBID, iRet, cFailCount );
#else	// #if defined(PRE_MOD_SELECT_CHAR)
		SendDeleteChar(pDelete->cCharIndex, iRet, cFailCount );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
		return ERROR_NONE;
	}

	bool bFirstVillage = true;
	TIMESTAMP_STRUCT DeleteDate;
	memset( &DeleteDate, 0, sizeof(DeleteDate) );
#if defined(PRE_MOD_SELECT_CHAR)
	int nWorldSetID = GetWorldSetIDByCharacterDBID(pDelete->biCharacterDBID);
	int nRet = g_pSQLWorldManager->QueryDelCharacter(nWorldSetID, pDelete->biCharacterDBID, bFirstVillage, DeleteDate );
	if (nRet == ERROR_NONE){
		g_pSQLMembershipManager->QueryDelCharacter(pDelete->biCharacterDBID, m_wszVirtualIp, m_wszIP, bFirstVillage, DeleteDate );
		DelCharacterData(pDelete->biCharacterDBID);
		m_cUserState = STATE_SERVERLIST;

	}

	SendDeleteChar(pDelete->biCharacterDBID, nRet);

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	int nRet = g_pSQLWorldManager->QueryDelCharacter(m_nWorldSetID, GetCharacterData(pDelete->cCharIndex)->biCharacterDBID, bFirstVillage, DeleteDate );
	if (nRet == ERROR_NONE){
		g_pSQLMembershipManager->QueryDelCharacter(GetCharacterData(pDelete->cCharIndex)->biCharacterDBID, m_wszVirtualIp, m_wszIP, bFirstVillage, DeleteDate );
		DelCharData(pDelete->cCharIndex);
		m_cUserState = STATE_SERVERLIST;
	}

	SendDeleteChar(pDelete->cCharIndex, nRet);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return ERROR_NONE;
}

int CDNUserConnection::RequestReviveCharacter( CSReviveChar* pRevive, int nLen )
{
	if (sizeof(CSReviveChar) != nLen)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_CHARLIST)
	{
		DetachConnection(L"RequestReviveCharacter State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestReviveCharacter State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

#if defined(PRE_MOD_SELECT_CHAR)
	if (IsValidCharacter(pRevive->biCharacterDBID) == false)
	{
		SendReviveChar( pRevive->biCharacterDBID, ERROR_LOGIN_CHARACTER_INDEX_UNMATCH );
		return ERROR_NONE;
	}

	int nWorldSetID = GetWorldSetIDByCharacterDBID(pRevive->biCharacterDBID);

	// World
	int nRet = g_pSQLWorldManager->QueryReviveCharacter( nWorldSetID, pRevive->biCharacterDBID );
	if( nRet != ERROR_NONE )
	{
		SendReviveChar( pRevive->biCharacterDBID, nRet );
		return ERROR_NONE;
	}

	// Membership
	nRet = g_pSQLMembershipManager->QueryReviveCharacter( pRevive->biCharacterDBID, m_wszVirtualIp, m_wszIP );
	if( nRet == ERROR_NONE ){
		m_cUserState = STATE_SERVERLIST;

	}
	SendReviveChar( pRevive->biCharacterDBID, nRet );

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (IsValidCharacterIndex(pRevive->cCharIndex) == false)
	{
		SendDeleteChar(pRevive->cCharIndex, ERROR_LOGIN_CHARACTER_INDEX_UNMATCH);	// Need to define retcode
		return ERROR_NONE;
	}

	// World
	int nRet = g_pSQLWorldManager->QueryReviveCharacter( m_nWorldSetID, GetCharacterData(pRevive->cCharIndex)->biCharacterDBID );
	if( nRet != ERROR_NONE )
	{
		SendReviveChar( pRevive->cCharIndex, nRet );
		return ERROR_NONE;
	}

	// Membership
	nRet = g_pSQLMembershipManager->QueryReviveCharacter( GetCharacterData(pRevive->cCharIndex)->biCharacterDBID, m_wszVirtualIp, m_wszIP );
	if( nRet == ERROR_NONE ){
		m_cUserState = STATE_SERVERLIST;
	}
	SendReviveChar( pRevive->cCharIndex, nRet );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return ERROR_NONE;
}

int CDNUserConnection::RequestSelectChannel(CSSelectChannel *pSelectChannel, int nLen)
{
	if (sizeof(CSSelectChannel) != nLen)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_CHANNELLIST){
		DetachConnection(L"RequestSelectChannel State Mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] RequestSelectChannel State Mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}


#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
	CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(m_nWorldSetID, false));
	if (!pMasterCon)
	{
		if (GetSelectCharacterLastMapIndex() == 0){
			SendGameInfo(0, 0, 0, ERROR_GENERIC_MASTERCON_NOT_FOUND, 0, 0);
		}
		else {
			SendVillageInfo(ERROR_GENERIC_MASTERCON_NOT_FOUND, 0, 0);
		}
		g_Log.Log(LogType::_ERROR, this, L"[RequestSelectChannel] MasterCon NULL!!\r\n");
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	m_nChannelID = pSelectChannel->nChannelID;
	TChannelInfoEx *pChannel = GetChannelInfo(m_nChannelID);
	if (!pChannel){
		DetachConnection(L"ChannelData NULL");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] pChannel mismatch(%d)\r\n", m_nAccountDBID, m_nSessionID, m_nChannelID);
		return ERROR_GENERIC_UNKNOWNERROR;
	}

	if ( GetAccountLevel() == 0 && pChannel->Channel.nCurrentUser >= pChannel->Channel.nChannelMaxUser)
	{
		//If channel capacity is exceeded (possible since sync is not real-time), send to another channel.
		pChannel = GetOtherChannelInfo(m_nChannelID);
		if (pChannel)
		{
			//Just refresh this one for now.
			m_nChannelID = pChannel->Channel.nChannelID;
		}
		else
		{
			//If no channel was obtained, everything is truly full; reject connections until waiting queue is implemented!
			DetachConnection(L"ChannelData NULL");
			g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] pChannel Congestion(%d)\r\n", m_nAccountDBID, m_nSessionID, m_nChannelID);
			return ERROR_GENERIC_UNKNOWNERROR;
		}
	}
	
	m_cVillageID = pChannel->cVillageID;

#if defined(PRE_MOD_SELECT_CHAR)
	pMasterCon->SendAddUser(this);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	pMasterCon->SendUserInfo(this, m_nChannelID, m_bAdult, GetVirtualIp(), GetIp(), (GetSelectCharacterLastMapIndex() == 0) ? true : false);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	return ERROR_NONE;
}

int CDNUserConnection::RequestBackButton(int nLen)
{
	if (nLen != 0)
		return ERROR_INVALIDPACKET;

	switch(m_cUserState)
	{
	case STATE_SERVERLIST:
		{
			if (IsCertified()) {	// Caution !!! - Auth info reset must only be performed for users authenticated through QUERY_BEGINAUTH and QUERY_CHECKAUTH !!!

				{
					int iResult = g_pAuthManager->QueryResetAuth(m_nWorldSetID, m_nAccountDBID, m_nSessionID);

					// Post-processing for sessions whose auth info was reset
					if (iResult != ERROR_NONE)
					{
						Clear();
						m_cUserState = STATE_CHECKVERSION;
						g_Log.Log(LogType::_ERROR, this, L"[QueryResetAuth Fail] LoginServer %d:%d,%u,%u \r\n", iResult, m_nWorldSetID, m_nAccountDBID, m_nSessionID);

						DetachConnection(L"QueryResetAuth Fail");
						break;
					}
				}

				g_pSQLMembershipManager->QueryLogout(m_nAccountDBID, m_nSessionID);
				m_bCertified = false;
			}

			SendBackButtonLogin();
			Clear();
			m_cUserState = STATE_CHECKVERSION;
			m_bReconnected = false;

			g_pUserConnectionManager->DelConnection(this);	// Delete first
			UINT nNewSessionID = g_IDGenerator.GetUserConnectionID();
			SetSessionID(nNewSessionID);	// Newly issue

			if (g_pUserConnectionManager->PushConnection(this) == ERROR_GENERIC_DUPLICATESESSIONID){	// Push again 
				SendReconnectLogin(ERROR_GENERIC_UNKNOWNERROR, 0, 0);
				DetachConnection(L"Reconnect SessonID Duplicate");
				g_Log.Log(LogType::_ERROR, this, L"[SID:%u] RequestBackButton SessionID Duplicate\r\n", nNewSessionID);	
			}
		}
		break;

	case STATE_CHARLIST:
		{
			SendDelUser();
			
			m_cUserState = STATE_CHECKLOGIN;
			m_cWorldID = 0;
			m_nWorldSetID = 0;
		}
		break;

	case STATE_CHANNELLIST:
		{
			m_cUserState = STATE_CHARLIST;
			SendCharList(ERROR_NONE);
		}
		break;

	case STATE_WAITUSER:
		{
#if defined(PRE_MOD_SELECT_CHAR)
			SendDelUser();

			m_cUserState = STATE_CHARLIST;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
		}
		break;

	default:
		m_cUserState -= 1;
	}

	return ERROR_NONE;
}

int CDNUserConnection::RequestReconnectLogin(CSReconnectLogin *pReconnect, int nLen)
{
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 0;
#endif
	if (sizeof(CSReconnectLogin) != nLen)
		return ERROR_INVALIDPACKET;

#if defined( PRE_LOGINDUMP_LOG )
	m_ReconnectData = *pReconnect;
#endif
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 1;
#endif
	g_pUserConnectionManager->DelConnection(this);	// Remove from list for now (since it is a newly assigned SessionID)
	SetSessionID(pReconnect->nSessionID);	// Set to existing sessionid and
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 2;
#endif
	if (g_pUserConnectionManager->PushConnection(this) == ERROR_GENERIC_DUPLICATESESSIONID){	// Push again 
		SendReconnectLogin(ERROR_GENERIC_UNKNOWNERROR, 0, 0);
		DetachConnection(L"Reconnect SessonID Duplicate");
		g_Log.Log(LogType::_ERROR, this, L"[SID:%u] ReconnectLogin SessionID Duplicate\r\n", pReconnect->nSessionID);
		return ERROR_NONE;
	}

	m_bReconnected = true;

	m_nAccountDBID = pReconnect->nAccountDBID;
	m_biCertifyingKey = pReconnect->biCertifyingKey;

	BYTE cAdult = 0;
	UINT nSessionID = 0;
	INT64 biCharacterDBID = 0;
	WCHAR wszCharName[NAMELENMAX] = {0,};
	char cWorldSetID = 0;
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 3;
#endif
	int nRet = g_pSQLMembershipManager->QueryCheckAuth(pReconnect->nAccountDBID, pReconnect->biCertifyingKey, g_pAuthManager->GetServerID(), cWorldSetID, nSessionID, biCharacterDBID, static_cast<LPWSTR*>(static_cast<LPVOID>(&m_wszAccountName)), static_cast<LPWSTR*>(static_cast<LPVOID>(&wszCharName)), m_cAccountLevel, cAdult, m_cAge, m_cDailyCreateCount, m_nPrmInt1, m_cLastServerType);
	m_nWorldSetID = cWorldSetID;
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 4;
#endif

	g_Log.Log((0 > nRet)?(LogType::_ERROR):(LogType::_NORMAL), this, L"[Auth] CheckAuth (REQ) - Result:%d, nAccountDBID:%d, CertifyingKey:%I64d, CerServerID:%d\r\n", nRet, pReconnect->nAccountDBID, pReconnect->biCertifyingKey, g_pAuthManager->GetServerID());

	if (nRet != ERROR_NONE){
		DetachConnection(L"ReconnectLogin CheckAuth Failed");
		return ERROR_NONE;
	}

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 5;
#endif
	int nWorldID = 0;
	g_pSQLWorldManager->QueryGetCharacterPartialy8(m_nWorldSetID, biCharacterDBID, NULL, nWorldID);
	m_cWorldID = nWorldID;
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 6;
#endif

#if !defined(PRE_MOD_SELECT_CHAR)
	m_cCharCountMax = CreateCharacterDefaultCountMax;
	g_pSQLMembershipManager->QueryGetCharacterSlotCount(m_nAccountDBID, m_cWorldID, 0, m_cCharCountMax);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	// Set WorldID to 0 for users entering from another login.
	g_pSQLMembershipManager->QuerySetWorldID(0, pReconnect->nAccountDBID);
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 7;
#endif
#if defined(PRE_CHARLIST_SORTING)
	g_pSQLMembershipManager->QueryGetCharacterSortCode(m_nAccountDBID, m_cCharacterSortCode);
#endif	// #if defined(PRE_CHARLIST_SORTING)
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 8;
#endif

	m_bCertified = true;
	m_bAdult = cAdult ? true : false;


#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 9;
#endif	

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 10;
#endif


	m_cUserState = STATE_RECONNECTLOGIN;	// Change to login reconnect state

#if defined(PRE_MOD_SELECT_CHAR)
	g_pMasterConnectionManager->SendDetachUserConnectionByWorldSetID(m_nWorldSetID, m_nAccountDBID, true, false, 0);	// 20090908 // Send disconnect since user may still remain in village or game.
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	m_bMasterAddUser = true;		//When entering via reconnect, already added on master

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	g_pMasterConnectionManager->SendDetachUserConnectionByWorldSetID(m_nWorldSetID, m_nAccountDBID, m_cAccountLevel, true, NULL, 0, false, 0, GetIp());	// 20090908 // Send disconnect since user may still remain in village or game.
#else		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	g_pMasterConnectionManager->SendDetachUserConnectionByWorldSetID(m_nWorldSetID, m_nAccountDBID, m_cAccountLevel, true, NULL, 0);	// 20090908 // Send disconnect since user may still remain in village or game.
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	SendReconnectLogin(ERROR_NONE, m_nAccountDBID, m_biCertifyingKey);

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 11;
#endif

	SendCharacterList(true);
#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 24;
#endif


	return ERROR_NONE;
}

int CDNUserConnection::RequestChannelList(int nLen)
{
	if (nLen != 0)
		return ERROR_INVALIDPACKET;

	if (m_cUserState != STATE_CHANNELLIST)
	{
		DetachConnection(L"STATE_CHANNELLIST mismatch");
		g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] STATE_CHANNELLIST mismatch(state:%d)\r\n", m_nAccountDBID, m_nSessionID, m_cUserState);
		return ERROR_NONE;
	}

	SendChannelList();
	return ERROR_NONE;
}

int CDNUserConnection::RequestUpdateSecondAuthPassword( const char* pData, int nLen )
{
	const CSGameOptionUpdateSecondAuthPassword* pPacket = reinterpret_cast<const CSGameOptionUpdateSecondAuthPassword*>(pData);

	BYTE cFailCount = 0;
	std::wstring wszOldPW = ( m_bIsSetSecondAuthPW ) ? MakeSecondPasswordString( szVersion, pPacket->nOldSeed, pPacket->nOldValue ) : L"";
	std::wstring wszNewPW = MakeSecondPasswordString( szVersion, pPacket->nNewSeed, pPacket->nNewValue );
	int iVaildateRet = QueryValidataSecondAuthPassphrase( (WCHAR*)wszOldPW.c_str(), cFailCount);
	if( iVaildateRet != ERROR_NONE )
	{
		// Notify 2nd auth password change result
		SCGameOptionUpdateSecondAuthPassword TxPacket;
		memset( &TxPacket, 0, sizeof(TxPacket) );

		TxPacket.iRet = iVaildateRet;
		TxPacket.cFailCount	= cFailCount;

		AddSendData( SC_GAMEOPTION, eGameOption::SC_UPDATE_SECONDAUTH_PASSWORD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

		return ERROR_NONE;
	}

	int iRet = g_pSQLMembershipManager->QueryModSecondAuthPassphrase( m_nAccountDBID, wszOldPW.c_str(), wszNewPW.c_str(), cFailCount );

	// Notify 2nd auth password change result
	SCGameOptionUpdateSecondAuthPassword TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	TxPacket.cFailCount	= cFailCount;

	AddSendData( SC_GAMEOPTION, eGameOption::SC_UPDATE_SECONDAUTH_PASSWORD, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	
	// Update 2nd auth info
	if( iRet == ERROR_NONE )
	{
		if( !m_bIsSetSecondAuthPW )
		{
			m_bIsSetSecondAuthPW = true;

			if( g_pSQLMembershipManager->QueryModSecondAuthLockFlag( m_nAccountDBID, true ) == ERROR_NONE )
				m_bIsSetSecondAuthLock = true;

#ifdef PRE_ADD_23829
			m_bPassSecondAuth = false;
#endif	//#ifdef PRE_ADD_23829

			SendSecondAuthInfo();
		}
	}

	return iRet;
}

int CDNUserConnection::RequestInitSecondAuth( const char* pData, int nLen )
{
	// Why reset 2nd password when it is not even set???
	if( !m_bIsSetSecondAuthPW )
	{
		SendResInitSecondAuth( ERROR_INVALIDPACKET );
		return ERROR_NONE;
	}

	const CSGameOptionInitSecondAuth* pPacket = reinterpret_cast<const CSGameOptionInitSecondAuth*>(pData);

	// Verify password
	BYTE cFailCount = 0;
	std::wstring wszSecondAuthPW = MakeSecondPasswordString( szVersion, pPacket->nSeed, pPacket->nValue );
	int iRet = QueryValidataSecondAuthPassphrase( (WCHAR*)wszSecondAuthPW.c_str(), cFailCount );
	if( iRet != ERROR_NONE )
	{
		SendResInitSecondAuth( iRet, cFailCount );
		return ERROR_NONE;
	}

	// Reset 2nd auth
#ifdef PRE_ADD_23829
	iRet = InitSecondAuth();
	if (iRet == ERROR_NONE)
		SendSecondAuthInfo();
	else
		SendResInitSecondAuth( iRet );
#else
	iRet = g_pSQLMembershipManager->QueryInitSecondAuth(GetAccountDBID() );
	if( iRet == ERROR_NONE )
	{
		// Send 2nd auth info
		m_bIsSetSecondAuthLock	= false;
		m_bIsSetSecondAuthPW	= false;
		SendSecondAuthInfo();
	}
	SendResInitSecondAuth( iRet );
#endif	
	return ERROR_NONE;
}

#ifdef PRE_ADD_DOORS
int CDNUserConnection::RequestGetDoorsAuthKey(const char * pData, int nLen)
{
	if (m_bMobileAuthentication)
		return ERROR_GENERIC_INVALIDREQUEST;

	char szAuthKey[Doors::Common::AuthenticationKey_LenMax];
	memset(szAuthKey, 0, sizeof(szAuthKey));

	bool bFlag = false;
	int nRetCode = g_pSQLMembershipManager->QueryGetAuthenticationFlag(GetAccountDBID(), bFlag);
	if (nRetCode == ERROR_NONE)
		m_bMobileAuthentication = bFlag;
	else if (nRetCode == 101115 || nRetCode != ERROR_NONE)
		m_bMobileAuthentication = false;
	
	nRetCode = g_pSQLMembershipManager->QueryGetDoorsAuthentication(GetAccountDBID(), szAuthKey);
	if (nRetCode != ERROR_NONE)
		_DANGER_POINT();
	
	SendMobileAuthenticationKey(szAuthKey, nRetCode, m_bMobileAuthentication);
	return ERROR_NONE;
}

int CDNUserConnection::RequestCancelDoorsAuth(const char * pData, int nLen)
{
	if (m_bMobileAuthentication == false)
		return ERROR_GENERIC_INVALIDREQUEST;

	int nWorldID = g_pSQLMembershipManager->QueryCancelDoorsAuthentication(GetAccountDBID());	
	//int nWorldRetCode = g_pSQLWorldManager->QueryCancelDoorsAuthentication(nWorldID, GetAccountDBID());
	//if (nWorldRetCode == ERROR_NONE)// && nWorldRetCode == ERROR_NONE)
	if (nWorldID == ERROR_NONE)// && nWorldRetCode == ERROR_NONE)
		m_bMobileAuthentication = false;
	///SendMobileCancelAuthentication(nWorldRetCode);
	SendMobileCancelAuthentication(nWorldID);
	return ERROR_NONE;
}

int CDNUserConnection::RequestGetAuthFlag(const char * pData, int nLen)
{
	bool bAuth = false;
	int nRet = g_pSQLMembershipManager->QueryGetAuthenticationFlag(GetAccountDBID(), bAuth);
	if (nRet == 0)
		m_bMobileAuthentication = bAuth;
	SendMobileAuthenticationFlag(m_bMobileAuthentication);
	return ERROR_NONE;
}
#endif		//#ifdef PRE_ADD_DOORS

int CDNUserConnection::RequestCharacterSortCode(CSCharacterSortCode *pSortCode, int nLen)
{
	if (sizeof(CSCharacterSortCode) != nLen)
		return ERROR_INVALIDPACKET;

	if (pSortCode->cCharacterSortCode <= Login::CharacterSortCode::None) return ERROR_GENERIC_INVALIDREQUEST;
	if (pSortCode->cCharacterSortCode >= Login::CharacterSortCode::Max) return ERROR_GENERIC_INVALIDREQUEST;

	m_cCharacterSortCode = pSortCode->cCharacterSortCode;
	g_pSQLMembershipManager->QueryModCharacterSortCode(m_nAccountDBID, m_cCharacterSortCode);

	return ERROR_NONE;
}


#ifdef PRE_ADD_23829
int CDNUserConnection::InitSecondAuth()
{
	// Reset 2nd auth
	int nRetCode = g_pSQLMembershipManager->QueryInitSecondAuth(GetAccountDBID() );
	if(nRetCode == ERROR_NONE)
	{
		// Send 2nd auth info
		m_bIsSetSecondAuthLock	= false;
		m_bIsSetSecondAuthPW	= false;
	}
	return nRetCode;
}
#endif //#ifdef PRE_ADD_23829

#if defined(PRE_MOD_SELECT_CHAR)
TDBListCharData* CDNUserConnection::GetCharacterData(INT64 biCharacterDBID)
{
	if (biCharacterDBID <= 0) return NULL;
	if (m_MapCharacterList.empty()) return NULL;

	TMapCharacterList::iterator iter = m_MapCharacterList.find(biCharacterDBID);
	if (iter == m_MapCharacterList.end()) return NULL;

	return &(iter->second);	
}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

int CDNUserConnection::MessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen)
{
	__try {
		DWORD dwCurTick = timeGetTime();
		if ((m_cLastMainCmd == nMainCmd) && (m_cLastSubCmd == nSubCmd)){
			if (dwCurTick - m_dwLastMessageTick <= 5){
				g_Log.Log(LogType::_ERROR, this, L"CDNUserConnection::MessageProcess repeat command(%d, %d)\n", nMainCmd, nSubCmd);
				return ERROR_GENERIC_UNKNOWNERROR;
			}
		}
		m_dwLastMessageTick = dwCurTick;
		m_cLastMainCmd = nMainCmd;
		m_cLastSubCmd = nSubCmd; 

		switch(nMainCmd)
		{
			case CS_LOGIN:		return OnRecvLoginMessage( nSubCmd, pData, nLen );
			case CS_SYSTEM:		return OnRecvSystemMessage( nSubCmd, pData, nLen );
			case CS_GAMEOPTION:	return OnRecvGameOptionMessage( nSubCmd, pData, nLen );
#ifdef PRE_ADD_DOORS
			case CS_DOORS: return OnRecvDoorsMessage(nSubCmd, pData, nLen);
#endif		//#ifdef PRE_ADD_DOORS
		}

		g_Log.Log(LogType::_ERROR, this, L"CDNUserConnection::MessageProcess (%d, %d)\n", nMainCmd, nSubCmd);
	}
#if defined( PRE_LOGINDUMP_LOG )
	__except(CExceptionCodeLogLoginServer(GetExceptionCode(), L"CDNUserConnection::MessageProcess()", nMainCmd, nSubCmd, m_nReconnectLoginStep, &m_ReconnectData), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
		// This code block is never executed
	}
#else
	__except(CExceptionCodeLog(GetExceptionCode(), L"CDNUserConnection::MessageProcess()", nMainCmd, nSubCmd), EXCEPTION_CONTINUE_SEARCH) {	// Do not handle exception here; look for an upper-level exception handler.
		// This code block is never executed
	}
#endif

	return ERROR_UNKNOWN_HEADER;
}

#if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SetAvailableCharacterSlotCount()
{
	m_cAvailableCharacterSlotCount = m_nDefaultCreateCharacterSlotPerAccount;
	char cAddSlotCount = 0;
	g_pSQLMembershipManager->QueryGetCharacterSlotCount(m_nAccountDBID, 0, 0, cAddSlotCount);
	m_cAvailableCharacterSlotCount += cAddSlotCount;

	if (m_cAvailableCharacterSlotCount < 0){
		g_Log.Log(LogType::_FILEDBLOG, this, L"[SetAvailableCharacterSlotCount] %d", m_cAvailableCharacterSlotCount);
		m_cAvailableCharacterSlotCount = 0;
	}
	if (m_cAvailableCharacterSlotCount > m_nCreateCharacterSlotMaxPerAccount){
		g_Log.Log(LogType::_FILEDBLOG, this, L"[SetAvailableCharacterSlotCount] %d", m_cAvailableCharacterSlotCount);
		m_cAvailableCharacterSlotCount = m_nCreateCharacterSlotMaxPerAccount;
	}
}

char CDNUserConnection::GetAvailableCharacterSlotCount()
{
	return m_cAvailableCharacterSlotCount;
}

int CDNUserConnection::GetEmptyCharacterSlotCount()
{
	if (m_MapCharacterList.empty())
		return m_cAvailableCharacterSlotCount;

	return m_cAvailableCharacterSlotCount - (int)m_MapCharacterList.size();
}

#else	// #if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SetCharacterCountMax(BYTE cMaxCount)
{
	m_cCharCountMax = cMaxCount;

	if (m_cCharCountMax < 0) m_cCharCountMax = 0;
	if (m_cCharCountMax > m_cCharacterLimitCount) m_cCharCountMax = m_cCharacterLimitCount;
}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

void CDNUserConnection::ClearCharacterData()
{
#if defined(PRE_MOD_SELECT_CHAR)
	m_MapCharacterList.clear();
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	memset(&m_CharDataArray, 0, sizeof(m_CharDataArray));
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

#if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SetSelectCharacterData(INT64 biCharacterDBID)
{
	TDBListCharData *pCharData = GetCharacterData(biCharacterDBID);
	if (pCharData){
		m_SelectCharacterData = *pCharData;

		SetWorldID(pCharData->cWorldID);
	}
}

void CDNUserConnection::DelCharacterData(INT64 biCharacterDBID)
{
	if (biCharacterDBID <= 0) return;
	if (m_MapCharacterList.empty()) return;

	TMapCharacterList::iterator iter = m_MapCharacterList.find(biCharacterDBID);
	if (iter == m_MapCharacterList.end()) return;

	m_MapCharacterList.erase(iter);
}

#else	// #if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SetCharData(char cCharIndex, TDBListCharData &CharData)
{
	if (IsValidCharacterIndex(cCharIndex) == false) return;
	m_CharDataArray[cCharIndex] = CharData;
}

void CDNUserConnection::DelCharData(char cCharIndex)
{
	if (IsValidCharacterIndex(cCharIndex) == false) return;
	memset(&m_CharDataArray[cCharIndex], 0, sizeof(TDBListCharData));
}

TDBListCharData* CDNUserConnection::SelectCharData()
{
	if (IsValidCharacterIndex(m_cSelectCharIndex) == false) return NULL;
	return &m_CharDataArray[m_cSelectCharIndex];
}

TDBListCharData* CDNUserConnection::GetCharacterData(char cCharIndex)
{
	if (IsValidCharacterIndex(cCharIndex) == false) return NULL;
	return &m_CharDataArray[cCharIndex];
}

int CDNUserConnection::GetEmptyCharIndex()
{
	for (int i = 0; i < m_cCharacterLimitCount; i++){
		if (m_CharDataArray[i].biCharacterDBID <= 0) return i;
	}

	return -1;
}

INT64 CDNUserConnection::GetCharacterDBID( BYTE cIndex )
{
	if (IsValidCharacterIndex(cIndex) == false) return 0;
	return m_CharDataArray[cIndex].biCharacterDBID;
}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

INT64 CDNUserConnection::GetSelectCharacterDBID()
{
#if defined(PRE_MOD_SELECT_CHAR)
	return m_SelectCharacterData.biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (IsValidCharacterIndex(m_cSelectCharIndex) == false) return 0;
	return m_CharDataArray[m_cSelectCharIndex].biCharacterDBID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

WCHAR* CDNUserConnection::GetSelectCharacterName()
{
#if defined(PRE_MOD_SELECT_CHAR)
	return m_SelectCharacterData.wszCharacterName;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (IsValidCharacterIndex(m_cSelectCharIndex) == false) return NULL;
	return m_CharDataArray[m_cSelectCharIndex].wszCharacterName;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

int CDNUserConnection::GetSelectCharacterLastMapIndex()
{
#if defined(PRE_MOD_SELECT_CHAR)
	return m_SelectCharacterData.nLastVillageMapID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	if (IsValidCharacterIndex(m_cSelectCharIndex) == false) return 0;
	return m_CharDataArray[m_cSelectCharIndex].nLastVillageMapID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

int CDNUserConnection::GetSelectCharacterMapIndex()
{
#if defined(PRE_MOD_SELECT_CHAR)
	return m_SelectCharacterData.nMapID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	return m_CharDataArray[m_cSelectCharIndex].nMapID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

BYTE CDNUserConnection::GetSelectCharacterJob()
{
#if defined(PRE_MOD_SELECT_CHAR)
	return m_SelectCharacterData.cJob;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	return m_CharDataArray[m_cSelectCharIndex].cJob;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}


TChannelInfoEx* CDNUserConnection::GetChannelInfo(int nChannelID)
{
	if (m_ChannelList.empty()) return NULL;

	TMapChannel::iterator iter = m_ChannelList.find(nChannelID);
	if (iter != m_ChannelList.end()){
		return &iter->second;
	}

	return NULL;
}

TChannelInfoEx* CDNUserConnection::GetOtherChannelInfo(int nChannelID)
{
	if (m_ChannelList.empty()) return NULL;

	TMapChannel::iterator ii;
	for (ii = m_ChannelList.begin(); ii != m_ChannelList.end(); ii++)
	{
		if ((*ii).second.Channel.nChannelID != nChannelID && (*ii).second.Channel.nChannelMaxUser > (*ii).second.Channel.nCurrentUser)
			return &(*ii).second;
	}

	return NULL;
}

void CDNUserConnection::SetWorldID(char cWorldID)
{
	m_cWorldID = cWorldID;
	m_cWaitWorldID = 0;
	m_nWaitWorldSetID = 0;
#if defined(PRE_MOD_SELECT_CHAR)
	m_nWorldSetID = g_pExtManager->GetWorldSetID(m_cWorldID);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
}

void CDNUserConnection::DelDailyCreateCount()
{
	if (m_cDailyCreateCount == 0) return;

	m_cDailyCreateCount -= 1;
	if (m_cDailyCreateCount < 0) m_cDailyCreateCount = 0;
}

void CDNUserConnection::SendInsideDisconnectPacket(wchar_t *pIdent)
{
#ifdef _USE_ACCEPTEX
	if( ::InterlockedIncrement( &m_dwSendInsideDisconnect ) != 1 )
		return;
#endif

	if(IsCertified()) // Only LogOut users who completed auth.
	{
		if ((m_cUserState != STATE_CONNECTVILLAGE) && (m_cUserState != STATE_CONNECTGAME))
		{		// Not a user disconnecting due to moving to village or game server
			g_pSQLMembershipManager->QueryLogout(m_nAccountDBID, m_nSessionID);
		}
	}

	DNEncryptPacketSeq EnPacket = { 0, };
	EnPacket.Packet.iLen = static_cast<unsigned short>(sizeof(DNTPacketHeader));
	EnPacket.Packet.cMainCmd = IN_DISCONNECT;
	EnPacket.Packet.cSubCmd = CONNECTIONKEY_USER;

	EnPacket.nLen = EnPacket.Packet.iLen + sizeof(BYTE) + sizeof(USHORT);
	CDNSecure::GetInstance().Tea_encrypt( reinterpret_cast<char*>(&EnPacket.Packet), EnPacket.Packet.iLen );

	SetDelete(true);
	AddRecvData( EnPacket );

#ifdef _USE_ACCEPTEX
	g_pIocpManager->PostDisconnect( m_pSocketContext );
#else
	g_pIocpManager->AddProcessCall(m_pSocketContext);
#endif

	m_bReconnected = false;
}


#if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SetWaitWorldIDBySelectCharacter()
{
	m_cWaitWorldID = m_SelectCharacterData.cWorldID;
	m_nWaitWorldSetID = g_pExtManager->GetWorldSetID(m_cWaitWorldID);
}

BYTE CDNUserConnection::GetWorldIDByCharacterDBID(INT64 biCharacterDBID)
{
	TDBListCharData *pCharData = GetCharacterData(biCharacterDBID);
	if (pCharData)
		return pCharData->cWorldID;

	return 0;
}

int CDNUserConnection::GetWorldSetIDByCharacterDBID(INT64 biCharacterDBID)
{
	return g_pExtManager->GetWorldSetID(GetWorldIDByCharacterDBID(biCharacterDBID));
}

bool CDNUserConnection::IsValidCharacter(INT64 biCharacterDBID)
{
	TDBListCharData *pCharData = GetCharacterData(biCharacterDBID);
	if (pCharData) return true;

	return false;
}

#else	// #if defined(PRE_MOD_SELECT_CHAR)
bool CDNUserConnection::IsValidCharacterIndex(char cCharIndex)
{
	if ((cCharIndex < 0) || (cCharIndex >= m_cCharacterLimitCount)) return false;
	return true;
}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

void CDNUserConnection::CheckVillageGameInfo()
{
	if (GetSelectCharacterLastMapIndex() == 0){	// If selected character has not completed the tutorial
		if (m_nTutorialMapIndex > 0) {
#ifdef _USE_ACCEPTEX
			ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
			CDNMasterConnection* pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(m_nWorldSetID, false));
			if(!pMasterCon) {
				SendVillageInfo(ERROR_GENERIC_MASTERCON_NOT_FOUND, GetAccountDBID(), GetCertifyingKey());
				return;
			}
			 pMasterCon->SendReqTutorialGameID(GetAccountDBID(), GetSessionID(), m_nTutorialMapIndex, m_cTutorialGateNo);
		}
		else {
			SendVillageInfo(ERROR_NONE, GetAccountDBID(), GetCertifyingKey());
			m_cUserState = STATE_CONNECTVILLAGE;
		}
	}
	else{
		// If selected channel has CHANNEL_ATT_USELASTMAPINDEX attribute, replace MapIndex with LastMapIndex and Update.
		TChannelInfoEx* pChannelInfo = GetChannelInfo( m_nChannelID );
		if( pChannelInfo && pChannelInfo->Channel.nAttribute&GlobalEnum::CHANNEL_ATT_USELASTMAPINDEX )
		{
#ifdef _USE_ACCEPTEX
			ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
			// Double-check whether MapIndex is Village 
			if( g_pExtManager->GetMapType(GetSelectCharacterMapIndex()) == GlobalEnum::MAP_VILLAGE )
			{
				CDNMasterConnection* pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(m_nWorldSetID, false));

				bool bUpdate = false;
				if( pMasterCon && !pMasterCon->bIsUseLastMapIndex(GetSelectCharacterMapIndex()))
					bUpdate = true;

				if( bUpdate )
					g_pSQLWorldManager->QueryModLastVillageMapID(m_nWorldSetID, GetAccountDBID(), GetSelectCharacterDBID(), GetSelectCharacterMapIndex());
			}
			else
			{
				_DANGER_POINT();
			}
		}

		SendVillageInfo(ERROR_NONE, GetAccountDBID(), GetCertifyingKey());
		m_cUserState = STATE_CONNECTVILLAGE;
	}
}

void CDNUserConnection::SendDelUser()
{
	if (m_nWaitWorldSetID > 0 || m_nWorldSetID > 0)
	{
#ifdef _USE_ACCEPTEX
		ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
		int nWorldSetID = m_cUserState == STATE_WAITUSER ? m_nWaitWorldSetID : m_nWorldSetID;
		CDNMasterConnection *pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(nWorldSetID, false));
		if (pMasterCon)
		{
			if (m_cUserState == STATE_WAITUSER)
			{
				pMasterCon->SendDelWaitUser(g_pAuthManager->GetServerID(), GetAccountDBID());
				pMasterCon->DelWaitUser(GetAccountDBID());
				m_cWaitWorldID = 0;
			}
			else
			{
				if (m_cUserState != STATE_CONNECTVILLAGE && m_cUserState != STATE_CONNECTGAME)
				{
#if defined(PRE_MOD_SELECT_CHAR)
					pMasterCon->SendDetachUser(GetAccountDBID());
#else	// #if defined(PRE_MOD_SELECT_CHAR)
					pMasterCon->SendDetachUser(GetAccountDBID(), m_cAccountLevel);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
				}
#if !defined(PRE_MOD_SELECT_CHAR)
				m_bMasterAddUser = false;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
			}
		}
		else
			_DANGER_POINT();
	}
}

// SC_System
void CDNUserConnection::SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason)
{
	SCDetachMessage packet;
	memset(&packet, 0, sizeof(SCDetachMessage));

	packet.nRetCode = nRetCode;

	AddSendData(SC_SYSTEM, eSystem::SC_DETACHCONNECTION_MSG, (char*)&packet, sizeof(packet));
	DelayDetachConnection(const_cast<wchar_t*>(pwszDetachReason));
}

void CDNUserConnection::SendVillageInfo(int nRet, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCVillageInfo VillageInfo;
	memset(&VillageInfo, 0, sizeof(SCVillageInfo));

	VillageInfo.nSessionID = m_nSessionID;
	VillageInfo.nAccountDBID = nAccountDBID;
	VillageInfo.biCertifyingKey = biCertifyingKey;

	TChannelInfoEx *pChannel = GetChannelInfo(m_nChannelID);
	if (pChannel){
		_strcpy(VillageInfo.szVillageIp, IPLENMAX, pChannel->szIP, (int)strlen(pChannel->szIP));
		VillageInfo.wVillagePort = pChannel->nPort;
	}

	VillageInfo.nRet = nRet;

	AddSendData(SC_SYSTEM, SC_VILLAGEINFO, (char*)&VillageInfo, sizeof(SCVillageInfo));	
}

void CDNUserConnection::SendGameInfo(ULONG nIP, USHORT nPort, USHORT nTcpPort, int nRet, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCGameInfo GameInfo;
	memset(&GameInfo, 0, sizeof(SCGameInfo));

	GameInfo.GameTaskType = GameTaskType::Normal;
	GameInfo.nGameServerIP = nIP;
	GameInfo.nGameServerPort = nPort;
	GameInfo.nGameServerTcpPort = nTcpPort;
	GameInfo.nRet = nRet;
	GameInfo.nAccountDBID = nAccountDBID;
	GameInfo.biCertifyingKey = biCertifyingKey;

	AddSendData(SC_SYSTEM, SC_GAMEINFO, (char*)&GameInfo, sizeof(GameInfo));
}

void CDNUserConnection::SendReconnectLogin(int nRet, UINT nAccountDBID, INT64 biCertifyingKey)
{
	SCReconnectLogin Reconnect = { 0, };
	Reconnect.nRet = nRet;
	Reconnect.nAccountDBID = nAccountDBID;
	Reconnect.biCertifyingKey = biCertifyingKey;

	AddSendData(SC_SYSTEM, SC_RECONNECTLOGIN, (char*)&Reconnect, sizeof(SCReconnectLogin));
#if defined(PRE_ADD_LOGIN_USERCOUNT)
	// Squeeze it in here.
	if( nRet == ERROR_NONE && !m_bSummitUserCount)
	{		
		g_pUserConnectionManager->AddUserCount();
		m_bSummitUserCount = true;
	}
#endif
}

// SC_Login
void CDNUserConnection::SendCheckVersion(int nRet)
{
	SCCheckVersion Check;
	memset(&Check, 0, sizeof(SCCheckVersion));
	_strcpy(Check.szServerVersion, SERVERVERSIONMAX, g_Config.szVersion, (int)strlen(g_Config.szVersion));
	Check.nRet = nRet;
	memcpy( &Check.MembershipDBVersion, g_pSQLMembershipManager->GetVersion(), sizeof(Check.MembershipDBVersion) );
	memcpy( &Check.WorldDBVersion, g_pSQLWorldManager->GetVersion(GetWorldSetID()), sizeof(Check.WorldDBVersion) );

	AddSendData(SC_LOGIN, SC_CHECKVERSION, (char*)&Check, sizeof(SCCheckVersion));
}

void CDNUserConnection::SendCheckLogin(int nRet, const char* pInfoURL/*=NULL*/)
{
	SCCheckLogin Login;
	memset(&Login, 0, sizeof(SCCheckLogin));

	Login.nRet = nRet;
	Login.nSessionID = m_nSessionID;
#if !defined(PRE_MOD_SELECT_CHAR)
	Login.cGender = m_cGender;
	Login.cCharCountMax = m_cCharCountMax;
#endif	// #if !defined(PRE_MOD_SELECT_CHAR)

	__time64_t servertime;
	time(&servertime);
	Login._tServerTime = servertime;

	memcpy_s(&Login.GameOption, sizeof(TGameOptions), &m_GameOptions, sizeof(TGameOptions));	
#ifdef PRE_ADD_COMEBACK
	Login.bComebackUser = m_bComebackUser;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	Login.bReConnectNewbieReward = m_bReConnectNewbieReward;
	Login._tAccountRegistTime = m_tAccountRegistDate;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
#ifdef PRE_ADD_DOORS
	Login.bMobileAuthentication = m_bMobileAuthentication;
#endif		//PRE_ADD_DOORS


	int nLen = sizeof(SCCheckLogin);


	AddSendData(SC_LOGIN, SC_CHECKLOGIN, (char*)&Login, nLen);

#if defined(PRE_ADD_LOGIN_USERCOUNT)
	// Squeeze it in here.
	if( nRet == ERROR_NONE && !m_bSummitUserCount)
	{
		g_pUserConnectionManager->AddUserCount();
		m_bSummitUserCount = true;
	}
#endif
}

void CDNUserConnection::SendServerList( std::map<int,int>& mWorldUserCount )
{
	SCServerList ServerList;
	memset(&ServerList, 0, sizeof(ServerList));

#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif

	g_pExtManager->GetServerList(ServerList.ServerListData, ServerList.cServerCount);
	for (int i = 0; i < ServerList.cServerCount; i++)
	{
		int nWorldSetID = g_pExtManager->GetWorldSetID(ServerList.ServerListData[i].cWorldID);
		CDNMasterConnection * pCon = (CDNMasterConnection*)g_pMasterConnectionManager->GetConnection(nWorldSetID, false);
		if (pCon)
		{
			ServerList.ServerListData[i].nWorldMaxUser = pCon->GetWorldMaxUserCount();
			ServerList.ServerListData[i].nWorldCurUser = pCon->GetWorldCurUserCount();
			BYTE cMyCharCount = 0;
			std::map<int,int>::iterator itor = mWorldUserCount.find( ServerList.ServerListData[i].cWorldID );
			if( itor != mWorldUserCount.end() )
			{
				cMyCharCount = static_cast<BYTE>(itor->second);
			}

			ServerList.ServerListData[i].cMyCharCount = cMyCharCount;

		}
		//else
		//	_DANGER_POINT();
	}

	AddSendData(SC_LOGIN, SC_SERVERLIST, (char*)&ServerList, sizeof(SCServerList) - \
		sizeof(ServerList.ServerListData) + (sizeof(TServerListData) * ServerList.cServerCount));
}

void CDNUserConnection::SendCharList(int nRet)
{
#ifdef _USE_ACCEPTEX
	ScopeLock<CSyncLock> Lock( g_pMasterConnectionManager->m_ConSync );
#endif
	SCCharList CharList;
	memset(&CharList, 0, sizeof(SCCharList));

	CharList.nRet = nRet;
	CharList.cAccountLevel	= GetAccountLevel();


#if !defined(PRE_MOD_SELECT_CHAR)
	CDNMasterConnection* pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(m_nWorldSetID, false));
	if( !pMasterCon )
	{
		//To return an error message when deleting a character etc. while master server is unavailable
		AddSendData(SC_LOGIN, SC_CHARLIST, (char*)&CharList, sizeof(CharList) - sizeof(CharList.CharListData) + (sizeof(TCharListData) * CharList.cCharCount));
		return;
	}
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	if (nRet == ERROR_NONE){
#if defined(PRE_CHARLIST_SORTING)
		CharList.cCharacterSortCode = m_cCharacterSortCode;
#endif	// #if defined(PRE_CHARLIST_SORTING)

		int nCount = 0;
#if defined(PRE_MOD_SELECT_CHAR)
		for (TMapCharacterList::iterator iter = m_MapCharacterList.begin(); iter != m_MapCharacterList.end(); iter++){
			if (iter->second.biCharacterDBID <= 0) continue;

			CharList.CharListData[nCount].cWorldID = iter->second.cWorldID;
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
			CharList.CharListData[nCount].cWorldSetID = g_pExtManager->GetWorldSetID(iter->second.cWorldID);
#endif
			CharList.CharListData[nCount].biCharacterDBID = iter->second.biCharacterDBID;
			_wcscpy(CharList.CharListData[nCount].wszCharacterName, NAMELENMAX, iter->second.wszCharacterName, NAMELENMAX);
			CharList.CharListData[nCount].cJob = iter->second.cJob;
			CharList.CharListData[nCount].cLevel = iter->second.cLevel;
			CharList.CharListData[nCount].biUniqueID = iter->second.biCharacterDBID;

			int nMapID = iter->second.nMapID;
			if (g_pExtManager->GetMapType(nMapID) != GlobalEnum::MAP_VILLAGE || iter->second.nLastVillageMapID == 0)
				nMapID = iter->second.nLastVillageMapID;

			int nWorldSetID = g_pExtManager->GetWorldSetID(iter->second.cWorldID);
			CDNMasterConnection* pMasterCon = static_cast<CDNMasterConnection*>(g_pMasterConnectionManager->GetConnection(nWorldSetID, false));
			if (pMasterCon && (pMasterCon->bIsUseLastMapIndex(nMapID)))
				nMapID = iter->second.nLastVillageMapID;

			CharList.CharListData[nCount].nMapID = nMapID;
			CharList.CharListData[nCount].nDefaultBody = iter->second.nDefaultBody;
			CharList.CharListData[nCount].nDefaultFoot = iter->second.nDefaultFoot;
			CharList.CharListData[nCount].nDefaultHand = iter->second.nDefaultHand;
			CharList.CharListData[nCount].nDefaultLeg = iter->second.nDefaultLeg;
			CharList.CharListData[nCount].dwHairColor = iter->second.dwHairColor;
			CharList.CharListData[nCount].dwEyeColor = iter->second.dwEyeColor;
			CharList.CharListData[nCount].dwSkinColor = iter->second.dwSkinColor;
			CharList.CharListData[nCount].bDeleteFlag = iter->second.bDeleteFlag;
			CharList.CharListData[nCount].DeleteDate = iter->second.DeleteDate;
#if defined(PRE_CHARLIST_SORTING)
			CharList.CharListData[nCount].LastLoginDate = iter->second.LastLoginDate;
			CharList.CharListData[nCount].CreateDate = iter->second.CreateDate;
#endif	// #if defined(PRE_CHARLIST_SORTING)

			memcpy(&CharList.CharListData[nCount].nEquipArray, iter->second.nEquipArray, sizeof(int) * EQUIPMAX);
			memcpy(&CharList.CharListData[nCount].nCashEquipArray, iter->second.nCashEquipArray, sizeof(int) * CASHEQUIPMAX);
#if defined( PRE_ADD_NEWCOMEBACK )
			CharList.CharListData[nCount].bCombackEffectItem = iter->second.bCombackEffectItem;
#endif
			nCount++;
		}
		CharList.cCharCount = nCount;
		CharList.cCharCountMax = m_cAvailableCharacterSlotCount;

#else	// #if defined(PRE_MOD_SELECT_CHAR)
		for (int i = 0; i < m_cCharacterLimitCount; i++)
		{
			if (m_CharDataArray[i].biCharacterDBID <= 0) 
				continue;
			wcsncpy(CharList.CharListData[nCount].wszCharacterName, m_CharDataArray[i].wszCharacterName, NAMELENMAX);
			CharList.CharListData[nCount].cJob = m_CharDataArray[i].cJob;
			CharList.CharListData[nCount].cLevel = m_CharDataArray[i].cLevel;
			CharList.CharListData[nCount].biUniqueID = m_CharDataArray[i].biCharacterDBID;

			int nMapID = m_CharDataArray[i].nMapID;
			if (g_pExtManager->GetMapType(nMapID) != GlobalEnum::MAP_VILLAGE || pMasterCon->bIsUseLastMapIndex(nMapID) || m_CharDataArray[i].nLastVillageMapID == 0 ) 
				nMapID = m_CharDataArray[i].nLastVillageMapID;

			CharList.CharListData[nCount].nMapID = nMapID;
			CharList.CharListData[nCount].cCharIndex = m_CharDataArray[i].cCharIndex;
			CharList.CharListData[nCount].nDefaultBody = m_CharDataArray[i].nDefaultBody;
			CharList.CharListData[nCount].nDefaultFoot = m_CharDataArray[i].nDefaultFoot;
			CharList.CharListData[nCount].nDefaultHand = m_CharDataArray[i].nDefaultHand;
			CharList.CharListData[nCount].nDefaultLeg = m_CharDataArray[i].nDefaultLeg;
			CharList.CharListData[nCount].dwHairColor = m_CharDataArray[i].dwHairColor;
			CharList.CharListData[nCount].dwEyeColor = m_CharDataArray[i].dwEyeColor;
			CharList.CharListData[nCount].dwSkinColor = m_CharDataArray[i].dwSkinColor;
			CharList.CharListData[nCount].bDeleteFlag = m_CharDataArray[i].bDeleteFlag;
			CharList.CharListData[nCount].DeleteDate = m_CharDataArray[i].DeleteDate;
#if defined(PRE_CHARLIST_SORTING)
			CharList.CharListData[nCount].LastLoginDate = m_CharDataArray[i].LastLoginDate;
#endif	// #if defined(PRE_CHARLIST_SORTING)

			memcpy(&CharList.CharListData[nCount].nEquipArray, m_CharDataArray[i].nEquipArray, sizeof(int) * EQUIPMAX);
			memcpy(&CharList.CharListData[nCount].nCashEquipArray, m_CharDataArray[i].nCashEquipArray, sizeof(int) * CASHEQUIPMAX);
#if defined( PRE_ADD_NEWCOMEBACK )
			CharList.CharListData[nCount].bCombackEffectItem	= m_CharDataArray[i].bCombackEffectItem;
#endif
			nCount++;
		}
		CharList.cCharCount = nCount;
		CharList.cCharCountMax = m_cCharCountMax;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	}
	
	AddSendData(SC_LOGIN, SC_CHARLIST, (char*)&CharList, sizeof(CharList) - sizeof(CharList.CharListData) + (sizeof(TCharListData) * CharList.cCharCount));
}

void CDNUserConnection::SendCharacterList(bool bProcessSecondAuth)
{
	// Get character list and secondary auth info.

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 15;
#endif
	int nRet = QueryCharacterListAndSecondAuth( bProcessSecondAuth );

	SendCharList(nRet);

#if defined( PRE_LOGINDUMP_LOG )	
	m_nReconnectLoginStep = 23;
#endif

}

void CDNUserConnection::SendChannelList( int nRet/*=ERROR_NONE*/, BYTE cFailCount/*=0*/ )
{
	SCChannelList ChannelList;
	memset(&ChannelList, 0, sizeof(SCChannelList));

	ChannelList.nRet = nRet;
	if( nRet != ERROR_NONE )
	{
		ChannelList.cFailCount = cFailCount;
		AddSendData( SC_LOGIN, SC_CHANNELLIST, (char*)&ChannelList, sizeof(SCChannelList) - sizeof(ChannelList.Info)- sizeof(ChannelList.cCount) );
		return;
	}

#if defined(PRE_MOD_SELECT_CHAR)
		g_pMasterConnectionManager->GetChannelListByMapIdx(m_nWorldSetID, &m_SelectCharacterData, ChannelList.Info, ChannelList.cCount, &m_ChannelList);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
		g_pMasterConnectionManager->GetChannelListByMapIdx(m_nWorldSetID, &m_CharDataArray[m_cSelectCharIndex], ChannelList.Info, ChannelList.cCount, &m_ChannelList );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	AddSendData(SC_LOGIN, SC_CHANNELLIST, (char*)&ChannelList, sizeof(SCChannelList) - sizeof(ChannelList.Info) + (sizeof(sChannelInfo) * ChannelList.cCount));
}

void CDNUserConnection::SendCreateChar(WCHAR *pName, int nRet)
{
	SCCreateChar Create;
	memset(&Create, 0, sizeof(SCCreateChar));

	wcsncpy(Create.wszCharacterName, pName, NAMELENMAX);
	Create.nRet = nRet;
	if (nRet == ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY)
		Create.cDailyCreateMaxCount = 7;	// Hardcode it for now

	AddSendData(SC_LOGIN, SC_CREATECHAR, (char*)&Create, sizeof(SCCreateChar));
}

void CDNUserConnection::SendCreateDWCChar(WCHAR *pName, int nRet)
{
	SCCreateChar Create;
	memset(&Create, 0, sizeof(SCCreateChar));

	wcsncpy(Create.wszCharacterName, pName, NAMELENMAX);
	Create.nRet = nRet;
	if (nRet == ERROR_LOGIN_CANNOT_CREATE_CHARACTER_TODAY)
		Create.cDailyCreateMaxCount = 7;

	AddSendData(SC_LOGIN, SC_CREATEDWCCHAR, (char*)&Create, sizeof(SCCreateChar));
}

#if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SendDeleteChar(INT64 biCharacterDBID, int nRet, BYTE cFailCount/*=0*/)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SendDeleteChar( BYTE cCharIndex, int nRet, BYTE cFailCount/*=0*/ )
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	SCDeleteChar Delete;
	memset(&Delete, 0, sizeof(SCDeleteChar));

#if defined(PRE_MOD_SELECT_CHAR)
	Delete.biCharacterDBID = biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	Delete.cCharIndex	= cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	Delete.nRet	= nRet;
	Delete.cFailCount = cFailCount;

	AddSendData(SC_LOGIN, SC_DELETECHAR, (char*)&Delete, sizeof(SCDeleteChar));	
}

#if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SendReviveChar(INT64 biCharacterDBID, int nRet)
#else	// #if defined(PRE_MOD_SELECT_CHAR)
void CDNUserConnection::SendReviveChar( BYTE cCharIndex, int nRet )
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
{
	SCReviveChar TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

#if defined(PRE_MOD_SELECT_CHAR)
	TxPacket.biCharacterDBID = biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	TxPacket.cCharIndex = cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	TxPacket.nRet = nRet;

	AddSendData(SC_LOGIN, SC_REVIVECHAR, (char*)&TxPacket, sizeof(TxPacket));	
}

void CDNUserConnection::SendTutorialMapInfo(int nMapIndex, char cGateNo)
{
	SCTutorialMapInfo MapInfo = { 0, };

	MapInfo.nMapIndex = nMapIndex;
	MapInfo.cGateNo = cGateNo;

	AddSendData(SC_LOGIN, SC_TUTORIALMAPINFO, (char*)&MapInfo, sizeof(SCTutorialMapInfo));
}

void CDNUserConnection::SendWaitUser(BYTE cWorldID, UINT nWaitNumber, ULONG nEstimateTime)
{
	SCWaitUser packet;				// SC_LOGIN / SC_WAITUSER
	memset(&packet, 0, sizeof(packet));

	packet.cWorldID = cWorldID;
	packet.nWaitUserCount = nWaitNumber;
	packet.nEstimateTime = nEstimateTime;

	AddSendData(SC_LOGIN, SC_WAITUSER, (char*)&packet, sizeof(packet));
}

void CDNUserConnection::SendBackButtonLogin()
{
	AddSendData(SC_LOGIN, SC_BACKBUTTON_LOGIN, NULL, 0);
#if defined(PRE_ADD_LOGIN_USERCOUNT)
	// Squeeze it in here.
	if( m_bSummitUserCount )
	{		
		g_pUserConnectionManager->DelUserCount();
		m_bSummitUserCount = false;
	}
#endif
}

void CDNUserConnection::SendCheckBlock(__time64_t serverTime, TRestraintForAccountAndCharacter* pRestraint )
{
	SCCHeckBlock packet;
	memset(&packet, 0, sizeof(SCCHeckBlock));

	packet.ServerDate = serverTime;
	packet.StartDate = pRestraint->_tStartDate;
	packet.EndDate = pRestraint->_tEndDate;
	_wcscpy(packet.wszBlockReason, RESTRAINTREASONMAX, pRestraint->wszRestraintReason, (int)wcslen(pRestraint->wszRestraintReason));
	packet.cType = pRestraint->cLevelCode;
	packet.iReasonID = pRestraint->iReasonID;

	AddSendData(SC_LOGIN, SC_CHECKBLOCK, (char*)&packet, sizeof(SCCHeckBlock));
}



void CDNUserConnection::SendSecondAuthInfo()
{
	SCGameOptionNotifySecondAuthInfo TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.bIsSetSecondAuthLock	= m_bIsSetSecondAuthLock;
	TxPacket.bIsSetSecondAuthPW		= m_bIsSetSecondAuthPW;
	TxPacket.tSecondAuthResetDate	= m_tSecondAuthResetDate;
#ifdef PRE_ADD_23829
	TxPacket.bSecondAuthPassFlag = m_bPassSecondAuth;
#endif	//#ifdef PRE_ADD_23829
#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
	TxPacket.bCompletlySecondAuthPassFlag = m_bCompletlyPassSecondAuth;
#endif

	AddSendData( SC_GAMEOPTION, eGameOption::SC_NOTIFY_SECONDAUTHINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}

void CDNUserConnection::SendResInitSecondAuth( int iRet, BYTE cFailCount/*=0*/ )
{
	SCGameOptionInitSecondAuth TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	TxPacket.iRet = iRet;
	TxPacket.cFailCount = cFailCount;

	AddSendData( SC_GAMEOPTION, eGameOption::SC_INIT_SECONDAUTH, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
}


#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
void CDNUserConnection::QueryAddRestraintIP(const WCHAR * pwszReason, const char * pszIP)
{
	if (pwszReason == NULL || pszIP == NULL)
	{
		_DANGER_POINT();
		return;
	}

	g_pSQLMembershipManager->QueryAddBlockedIP(GetWorldSetID(), GetAccountDBID(), pszIP);
	DetachConnection( const_cast<wchar_t*>(pwszReason) );
	g_Log.Log(LogType::_ERROR, this, L"[ADBID:%u, SID:%u] QueryAddRestraintIP Restraint \r\n", m_nAccountDBID, m_nSessionID);
}
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

#ifdef PRE_ADD_DOORS
//Doors
void CDNUserConnection::SendMobileAuthenticationKey(const char * szKeys, int nRetCode, bool bFlag)
{
	if (szKeys == NULL || strlen(szKeys) < Doors::Common::AuthenticationKey_LenMax - 1) return;

	SCDoorsGetAuthKey packet;
	memset(&packet, 0, sizeof(SCDoorsGetAuthKey));

	packet.nRetCode = nRetCode;
	packet.bMobileAuthentication = bFlag;
	MultiByteToWideChar(CP_ACP, 0, szKeys, -1, packet.wszAuthKey, Doors::Common::AuthenticationKey_LenMax);

	AddSendData(SC_DOORS, eDoors::SC_GET_AUTHKEY, (char*)&packet, sizeof(SCDoorsGetAuthKey));
}

void CDNUserConnection::SendMobileCancelAuthentication(int nRetCode)
{
	SCDoorsCancelAuth packet;
	memset(&packet, 0, sizeof(SCDoorsCancelAuth));

	packet.nRetCode = nRetCode;

	AddSendData(SC_DOORS, eDoors::SC_CANCEL_AUTH, (char*)&packet, sizeof(SCDoorsCancelAuth));
}

void CDNUserConnection::SendMobileAuthenticationFlag(bool bFlag)
{
	SCDoorsGetAuthFlag packet;
	memset(&packet, 0, sizeof(SCDoorsGetAuthFlag));

	packet.bFlag = bFlag;

	AddSendData(SC_DOORS, eDoors::SC_GET_MOBILEAUTHFLAG, (char*)&packet, sizeof(SCDoorsGetAuthFlag));
}
#endif		//#ifdef PRE_ADD_DOORS