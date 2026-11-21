#pragma once

#pragma pack(push, 1)

//--------------------------------------------------------------
//	Login Packet
//--------------------------------------------------------------

namespace Login
{
	struct TQueryLoginOutput
	{
		UINT nAccountDBID;
		bool IsNewAccount;
		TIMESTAMP_STRUCT LogOutDate;
#if defined(PRE_ADD_GAMEQUIT_REWARD)
		TIMESTAMP_STRUCT RegistDate;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
	};
};

struct LOMARegist
{
	int nServerID;			// Meaning of g_Config.nManagedID
};

#if defined(PRE_MOD_SELECT_CHAR)

struct LOMAAddUser
{
	UINT nAccountDBID;
	UINT nSessionID;
	INT64 biCharacterDBID;
	WCHAR wszAccountName[IDLENMAX];
	WCHAR wszCharacterName[NAMELENMAX];
	char cPCBangGrade;	// PC bang grade (0: not PC bang, others: ePCBangGrade)
	char szIP[IPLENMAX];	// Added for fatigue
	char szVirtualIp[IPLENMAX];
	USHORT wChannelID;
	bool bAdult;			// Whether adult or not (fatigue)

	bool bForce;
	BYTE cVillageID;
	BYTE cAccountLevel;
	bool bTutorial;
#ifdef PRE_ADD_COMEBACK
	bool bComebackUser;
#endif		//PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	bool bReConnectNewbieReward;
	GameQuitReward::RewardType::eType eUserGameQuitReward;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
};

#else		// #if defined(PRE_MOD_SELECT_CHAR)

struct LOMAAddUser				// LOMA_ADDUSER
{
	UINT nAccountDBID;
	UINT nSessionID;
	WCHAR wszAccountName[IDLENMAX];
	bool bForce;
	BYTE cAccountLevel;
	char cWorldID;
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	char szIP[IPLENMAX];
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
#ifdef PRE_ADD_COMEBACK
	bool bComebackUser;
#endif		//PRE_ADD_COMEBACK
#if defined(PRE_ADD_GAMEQUIT_REWARD)
	bool bReConnectNewbieReward;
	GameQuitReward::RewardType::eType eUserGameQuitReward;
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)
};

struct LOMAUserInfo				// LOMA_USERINFO
{
	UINT	nSessionID;
	UINT	nAccountDBID;
	INT64	biCharacterDBID;
	WCHAR	wszAccountName[IDLENMAX];
	WCHAR	wszCharacterName[NAMELENMAX];
	BYTE	cVillageID;
	USHORT	nChannelID;
	bool	bAdult;			// Adult or not (fatigue)
	char	szIP[IPLENMAX];	// Added for fatigue
	char	szVirtualIp[IPLENMAX];
	bool	bTutorial;
	char	cPCBangGrade;	// PC bang grade (0: not PC bang, others: ePCBangGrade)
};

#endif	// #if defined(PRE_MOD_SELECT_CHAR)

struct LOMAAddWaitUser			// LOMA_ADDWAITUSER
{
	int nServerID;
	UINT nAccountDBID;
};

struct LOMADelWaitUser			// LOMA_DELWAITUSER
{
	int nServerID;
	UINT nAccountDBID;
};

struct LOMADetachUser			// LOMA_DETACHUSER
{
	UINT nAccountDBID;
	BYTE cIsReconnectLogin;
	bool bIsDuplicate;
	UINT nSessionID;
#if !defined(PRE_MOD_SELECT_CHAR)
	BYTE cAccountLevel;
#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
	char szIP[IPLENMAX];
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
};

struct LOMADetachUserOther
{
	UINT nAccountDBID;
	int nLoginServerID;	
};

struct LOMADetachUserbyIP
{
	char szIP[IPLENMAX];
};

struct LOMAReqTutorialGameID	// LOMA_REQTUTORIALGAMEID
{
	UINT nAccountDBID;
	UINT nSessionID;
	int nTutorialMapIndex;
	char cTutorialGateNo;
};

struct LOMAConCountInfo			// LOMA_CONCOUNTINFO
{
public:
	LOMAConCountInfo()
	{
		Reset();
	}
	void Set(int pServerID, int pCurConCount, int pMaxConCount)
	{
		// 		DN_ASSERT(0 != nServerID,		"Invalid!");

		nServerID = pServerID;
		nCurConCount = pCurConCount;
		nMaxConCount = pMaxConCount;
	}
	void Reset()
	{
		nServerID = 0;
		nCurConCount = 0;
		nMaxConCount = 0;
	}
public:
	int nServerID;
	int nCurConCount;
	int nMaxConCount;
};

#pragma pack(pop)
