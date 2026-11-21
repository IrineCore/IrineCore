#pragma once
 
#include "Connection.h"
#include "Util.h"
#include "MemPool.h"
#include "DNDWCCharPacket.h"

class CDNUserConnection: public CConnection, public TBoostMemoryPool<CDNUserConnection>
{
private:
#if defined(PRE_MOD_SELECT_CHAR)
	typedef std::map<INT64, TDBListCharData> TMapCharacterList;
	TMapCharacterList m_MapCharacterList;

	typedef std::map<int,int> TMapWorldCharCount;
	TMapWorldCharCount m_MapWorldCharCount;	// WorldID, CharCount
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	TDBListCharData m_CharDataArray[CHARCOUNTMAX];		// Save the fetched character unique ID
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	TMapChannel m_ChannelList;

#if defined(PRE_MOD_SELECT_CHAR)
	int m_nCreateCharacterSlotMaxPerAccount;		// Max creatable characters per account (GlobalWeight 1022)
	int m_nDefaultCreateCharacterSlotPerAccount;	// Default character creation count per account (GlobalWeight 1023)
	char m_cAvailableCharacterSlotCount;			// Maximum number of characters that can be created
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	char m_cCharacterLimitCount;	// Max creatable count (default 4 + cash items..)
	char m_cCharCountMax;	// Maximum number of owned characters
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	Partner::Type::eType m_PartnerType;

	UINT m_nAccountDBID;		// Account unique number (ID generated in DB Account Table)
	WCHAR m_wszAccountName[IDLENMAX];	// Account

	char m_cWorldID;			// World ID it belongs to
	char m_cWaitWorldID;		// Waiting world number
	int m_nWorldSetID;
	int m_nWaitWorldSetID;

	bool m_bPCBang;
	char m_cPCBangGrade;

	LONG m_dwSendInsideDisconnect;

#if defined(PRE_ADD_LOGIN_USERCOUNT)
	DWORD m_dwKickTick;					// User kick Tick
#endif

	DWORD m_dwLastMessageTick;
	BYTE m_cLastMainCmd;
	BYTE m_cLastSubCmd;
#if defined( PRE_LOGINDUMP_LOG )
	CSReconnectLogin m_ReconnectData;
	int m_nReconnectLoginStep;
#endif

	WCHAR m_wszVirtualIp[IPLENMAX];
	char m_szVirtualIp[IPLENMAX];

	bool m_bIsSetSecondAuthPW;
	bool m_bIsSetSecondAuthLock;
	__time64_t	m_tSecondAuthResetDate;
#ifdef PRE_ADD_23829
	bool m_bPassSecondAuth;
#endif	//#ifdef PRE_ADD_23829
#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
	bool m_bCompletlyPassSecondAuth;		//When using Ekey/Ecard in China, disable secondary auth creation button if 'hide for 3 days' is checked
#endif

	//Parse


	int OnRecvLoginMessage(int nSubCmd, const char * pData, int nLen);
	int OnRecvSystemMessage(int nSubCmd, const char * pData, int nLen);
	int OnRecvGameOptionMessage( int nSubCmd, const char* pData, int nLen );
#ifdef PRE_ADD_DOORS
	int OnRecvDoorsMessage(int nSubCmd, const char * pData, int nLen);
#endif		//#ifdef PRE_ADD_DOORS

	int RequestCheckVersion(CSCheckVersion *pVersion, int nLen);

	int RequestCheckLogin(CSCheckLogin *pLogin, int nLen);
	int RequestCheckLoginUS(const CSCheckLoginUS *pLogin, int nLen);
	int _ProcessLoginCheck(const WCHAR *pUserId, const WCHAR *pPassword, const WCHAR *pVirtualIp);

	int RequestServerList(int nLen);
	int RequestSelectServer(CSSelectServer *pSelectServer, int nLen);
	int RequestSelectCharacter(CSSelectChar *pSelectChar, int nLen);
	int RequestCreateCharacter(CSCreateChar *pCreate, int nLen);
	int RequestCreateDWCCharacter(CSCreateDWCChar *pCreate, int nLen);
	int RequestDeleteCharacter(CSDeleteChar *pDelete, int nLen);
	int RequestSelectChannel(CSSelectChannel *pSelectChannel, int nLen);
	int RequestBackButton(int nLen);
	int RequestReconnectLogin(CSReconnectLogin *pReconnect, int nLen);
	int RequestChannelList(int nLen);
	int RequestReviveCharacter( CSReviveChar* pRevive, int nLen );

	int RequestUpdateSecondAuthPassword( const char* pData, int nLen );
	int RequestInitSecondAuth( const char* pData, int nLen );

#ifdef PRE_ADD_DOORS
	int RequestGetDoorsAuthKey(const char * pData, int nLen);
	int RequestCancelDoorsAuth(const char * pData, int nLen);
	int RequestGetAuthFlag(const char * pData, int nLen);
#endif		//#ifdef PRE_ADD_DOORS
	int RequestCharacterSortCode(CSCharacterSortCode *pSortCode, int nLen);

#ifdef PRE_ADD_23829
	int InitSecondAuth();
#endif	//#ifdef PRE_ADD_23829

#if defined(PRE_MOD_SELECT_CHAR)
	TDBListCharData* GetCharacterData(INT64 biCharacterDBID);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

public:
	BYTE m_cVillageID;					// Zone ID it belongs to (created when Village is split)
	UINT m_nChannelID;

#if defined(PRE_MOD_SELECT_CHAR)
	TDBListCharData m_SelectCharacterData;	// Selected data
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE m_cSelectCharIndex;			// Selected index on character selection (number generated in DB Char Table)
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE m_cGender;						// Gender
	TGameOptions m_GameOptions;			// Game options

	bool m_bAdult;						// Whether adult or not (China fatigue)
	char m_cAge;						// Age
	BYTE m_cDailyCreateCount;			// Daily character creation limit
	char m_cAccountLevel;				// Account level (GM, developer, normal user, etc.)
	int m_nPrmInt1;						// Parameter 1
	char m_cLastServerType;				// Recent server type (eServerType / 0 : Login, 2 : Village, 3 : Game)

	BYTE m_cRegion;						// Region code

#if defined(PRE_ADD_LOGIN_USERCOUNT)
	bool m_bSummitUserCount;			// Whether to apply user count
#endif

#if defined(PRE_ADD_SENDLOGOUT)
	bool m_bSendLogOut;
	CSyncLock m_SendLogOutLock;
#endif

	int m_nTutorialMapIndex;			// Tutorial map index
	char m_cTutorialGateNo;				// Tutorial gate

	char m_cUserState;					// User state (eUserState)
#if !defined(PRE_MOD_SELECT_CHAR)
	bool m_bMasterAddUser;				// Whether I am already added to master
#endif	// #if defined(PRE_MOD_SELECT_CHAR)

	bool m_bReconnected;				// Whether reconnected from existing VI/GA for character selection, etc. (true: yes / false: first login)
	bool m_bCertified;					// Whether auth is complete (must be set to false before sending QUERY_CHECKAUTH)
	INT64 m_biCertifyingKey;			// Auth key


#ifdef PRE_ADD_COMEBACK
	bool m_bComebackUser;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined( PRE_ADD_GAMEQUIT_REWARD )
	bool m_bReConnectNewbieReward;
	__time64_t m_tAccountRegistDate;
	GameQuitReward::RewardType::eType m_eUserGameQuitReward;	// Matches GameQuitReward::RewardType
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )
#ifdef PRE_ADD_DOORS
	bool m_bMobileAuthentication;
#endif		//#ifdef PRE_ADD_DOORS
	BYTE m_cCharacterSortCode;

public:
	CDNUserConnection(void);
	virtual ~CDNUserConnection(void);

	void Clear();
	void DoUpdate(DWORD CurTick);

	int MessageProcess(int nMainCmd, int nSubCmd, char *pData, int nLen);
	void SendInsideDisconnectPacket(wchar_t *pIdent);	// Handle IN_DISCONNECT

#if defined(PRE_MOD_SELECT_CHAR)
	void SetAvailableCharacterSlotCount();
	char GetAvailableCharacterSlotCount();

	int GetEmptyCharacterSlotCount();

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	char GetCharacterLimitCount() const { return m_cCharacterLimitCount; }
	char GetCharacterCountMax() { return m_cCharCountMax; }
	void SetCharacterCountMax(BYTE cMaxCount);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	void ClearCharacterData();
#if defined(PRE_MOD_SELECT_CHAR)
	void SetSelectCharacterData(INT64 biCharacterDBID);
	void DelCharacterData(INT64 biCharacterDBID);

#else	// #if defined(PRE_MOD_SELECT_CHAR)
	void SetCharData(char cCharIndex, TDBListCharData &CharData);
	void DelCharData(char cCharIndex);
	TDBListCharData* SelectCharData();
	TDBListCharData* GetCharacterData(char cCharIndex);
	int GetEmptyCharIndex();
	INT64 GetCharacterDBID( BYTE cIndex );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	INT64 GetSelectCharacterDBID();	// Selected character dbid
	WCHAR* GetSelectCharacterName();
	int GetSelectCharacterLastMapIndex();	// LastMapIndex of selected character
	int GetSelectCharacterMapIndex();
	BYTE GetSelectCharacterJob();

	char GetUserState() const { return m_cUserState; }

	TChannelInfoEx* GetChannelInfo(int nChannelID);
	TChannelInfoEx* GetOtherChannelInfo(int nChannelID);

	void SetAccountDBID(UINT nAccountDBID) { m_nAccountDBID = nAccountDBID; }
	UINT GetAccountDBID() { return m_nAccountDBID; }
#if !defined(PRE_MOD_SELECT_CHAR)
	TDBListCharData* GetCharDataArray(){ return  m_CharDataArray; }
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	void SetAdult(bool bFlag) { m_bAdult = bFlag; }
	bool GetAdult() { return m_bAdult; }
	void SetAge(char cAge) { m_cAge = cAge; }
	char GetAge() const { return m_cAge; }
	void SetDailyCreateCount(char cDailyCreateCount) { m_cDailyCreateCount = cDailyCreateCount; }
	void DelDailyCreateCount();
	BYTE GetDailyCreateCount() const { return m_cDailyCreateCount; }
	int GetPrmInt1() const { return m_nPrmInt1; }
	void SetPrmInt1(int nPrmInt1) { m_nPrmInt1 = nPrmInt1; }
	eServerType GetLastServerType() const { return(static_cast<eServerType>(m_cLastServerType)); }
	void SetWorldID(char cWorldID);
	char GetWorldID() { return m_cWorldID; }
	void SetAccountName(const WCHAR *pName) { wcsncpy(m_wszAccountName, pName, IDLENMAX); }
	WCHAR* GetAccountName() { return m_wszAccountName; }
	void SetAccountLevel(char cAccountLevel) { m_cAccountLevel = cAccountLevel; }
	char GetAccountLevel() { return m_cAccountLevel; }
	WCHAR* GetVirtualIpW() { return m_wszVirtualIp; }
	char* GetVirtualIp() { return m_szVirtualIp; }
	bool IsReconnected() { return m_bReconnected; }
	bool IsCertified() { return m_bCertified; }
	void ResetCertified() { m_bCertified = false; }
	INT64 GetCertifyingKey() { return m_biCertifyingKey; }
	void SetPCBang(bool bFlag) { m_bPCBang = bFlag; }
	bool IsPCBang() { return m_bPCBang; }
	void SetPCBangGrade(char cGrade) { m_cPCBangGrade = cGrade; }
	char GetPCBangGrade() { return m_cPCBangGrade; }
	void SetWaitWorldID(char cWaitWorld) { m_cWaitWorldID = cWaitWorld; }
	char GetWaitWorldID() { return m_cWaitWorldID; }
	void SetWorldSetID(int nWorldSetID) { m_nWorldSetID = nWorldSetID; }
	int GetWorldSetID() { return m_nWorldSetID; }
	void SetWaitWorldSetID(int nWorldSetID) { m_nWaitWorldSetID = nWorldSetID; }
	int GetWaitWorldSetID() { return m_nWaitWorldSetID; }
	Partner::Type::eType GetPartnerType() const { return m_PartnerType; }

#if defined(PRE_MOD_SELECT_CHAR)
	void SetWaitWorldIDBySelectCharacter();
	BYTE GetWorldIDByCharacterDBID(INT64 biCharacterDBID);
	int GetWorldSetIDByCharacterDBID(INT64 biCharacterDBID);

	bool IsValidCharacter(INT64 biCharacterDBID);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	bool IsValidCharacterIndex(char cCharIndex);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	void CheckVillageGameInfo();

	//Master
	void SendDelUser();

	// -> User
	// System
	void SendCompleteDetachMsg(int nRetCode, const WCHAR * pwszDetachReason);
	void SendVillageInfo(int nRet, UINT nAccountDBID, INT64 biCertifyingKey);
	void SendGameInfo(ULONG nIP, USHORT nPort, USHORT nTcpPort, int nRet, UINT nAccountDBID, INT64 biCertifyingKey);
	void SendReconnectLogin(int nRet, UINT nAccountDBID, INT64 biCertifyingKey);

	// Login
	void SendCheckVersion(int nRet);
	void SendCheckLogin(int nRet, const char* pInfoURL=NULL);
	void SendServerList( std::map<int,int>& mWorldUserCount );
	void SendCharList(int nRet);	// Send without DB query
	void SendCharacterList(bool bProcessSecondAuth);	// Send after DB query
	void SendChannelList( int nRet=ERROR_NONE, BYTE cFailCount=0 );
	void SendSelectChar(int nRet);
	void SendCreateChar(WCHAR *pName, int nRet);
	void SendCreateDWCChar(WCHAR *pName, int nRet);
#if defined(PRE_MOD_SELECT_CHAR)
	void SendDeleteChar(INT64 biCharacterDBID, int nRet, BYTE cFailCount=0);
	void SendReviveChar(INT64 biCharacterDBID, int nRet);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	void SendDeleteChar(BYTE cCharIndex, int nRet, BYTE cFailCount=0 );
	void SendReviveChar( BYTE cCharIndex, int nRet );
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	void SendTutorialMapInfo(int nMapIndex, char cGateNo);
	void SendWaitUser(BYTE cWorldID, UINT nWaitNumber, ULONG nEstimateTime);
	void SendBackButtonLogin();
	void SendCheckBlock(__time64_t serverTime, TRestraintForAccountAndCharacter* pRestraint );

	// GameOption
	void SendSecondAuthInfo();
	void SendResInitSecondAuth( int iRet, BYTE cFailCount=0 );

	int QueryCharacterListAndSecondAuth( bool bProcessSecondAuth );
	int QueryValidataSecondAuthPassphrase( WCHAR* pwszPW, BYTE& cFailCount, bool bSetIgnoreSecondAuth = false );


	bool CheckAbuserCharLength();
	void QueryAddRestraint(wchar_t* pwszReason);

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	void QueryAddRestraintIP(const WCHAR * pwszReason, const char * pszIP);
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP


#ifdef PRE_ADD_DOORS
	//Doors
	void SendMobileAuthenticationKey(const char * szKeys, int nRetCode, bool bFlag);
	void SendMobileCancelAuthentication(int nRetCode);
	void SendMobileAuthenticationFlag(bool bFlag);
#endif		//#ifdef PRE_ADD_DOORS

private:
	int InitWrap(int nRecvLen, int nSendLen, int nCompLen = 0);
};
