#pragma once

#include "DNProtocol.h"
#include "DNUserData.h"
#include <math.h>

#if !defined( PRE_TEST_PACKETMODULE )
#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
#include "EtMathWrapperD3D.h"
#endif
#endif // #if !defined( PRE_TEST_PACKETMODULE )

#include "DNNotifierCommon.h"
#include "DNCommonDef.h"

#if defined(_SERVER)
#include "DNServerDef.h"
#endif
#pragma pack(push, 1)

//--------------------------------------------------------------
//                       Packet structure (header + data)
//--------------------------------------------------------------

struct DNTPacketHeader
{
	unsigned short iLen;
	unsigned char cMainCmd;
	unsigned char cSubCmd;
};

struct DNTPacketCompHeader
{
	unsigned short nLen;
	unsigned char cSeq;
	unsigned char cMainCmd;
	unsigned char cSubCmd;
};

struct DNTPacket
{
	unsigned short iLen;
	unsigned char cMainCmd;
	unsigned char cSubCmd;
	char buf[1024*40];
};

struct DNTCompPacket
{
	DNTPacketCompHeader header;
	char buf[1024*40];
};

struct DNGAME_PACKET
{
	unsigned short datasize;
	unsigned char header;			//Extend header to share interface with tcp packet. Same as main_cmd
	unsigned char sub_header;	//Same as sub_cmd
	unsigned short checksum;
	unsigned char seq;					//tcp udp seq
	char data[1024*4];
};

/* Security Association per Client */
struct ClientSA
{
    DWORD   _attr1;
    DWORD   _attr2;
};

struct DNEncryptPacketSeq
{
	USHORT nLen;
	BYTE cSeq;
	
	union
	{
		struct
		{
			char Buf[1024*50];
		};
		DNTPacket Packet;
	};
};

#if defined(_GAMESERVER)

//External PacketMessage Process (udp = Internal, tcp = external) (just conceptually)

struct _STORE_BUFFER_HEADER
{
	unsigned short	iBufSize;
	unsigned short	iSize;
	unsigned int nAccountDBID;
	int				iMainCmd;
	int				iSubCmd;
	BYTE cMsgType;				//eStoreExternalType
	USHORT nMasterID;
#if defined( PRE_WORLDCOMBINE_PARTY )
	int iWorldSetID;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	//bool			bIsDataMsg;
};

#endif

struct DNDoorsPacketHeader
{
	unsigned short datasize;
	unsigned short protocol;
};

struct DNDoorsPacket
{
	DNDoorsPacketHeader header;
	char data[8192 - sizeof(DNDoorsPacketHeader)];
};

//--------------------------------------------------------------
//				Misc structures used in multiple places (?)
//--------------------------------------------------------------


struct TDatabaseVersion
{
	enum
	{
		MaxDatabaseNameLen = 128,
	};

	TDatabaseVersion()
	{
		iVersion = -1;
		memset( wszName, 0, sizeof(wszName) );
	}

	int		iVersion;
	WCHAR	wszName[MaxDatabaseNameLen];
};

struct SPartyMemberInfo
{
	UINT	nSessionID;
	int		nExp;
	BYTE	cLevel;
	BYTE	cPvPLevel;
	BYTE	cJobArray[JOBMAX];
	char	cClassID;
	int		nMapIndex;
	WCHAR	wszCharacterName[NAMELENMAX];
	int		nHairID;
	int		nFaceID;
	DWORD	dwHairColor;
	DWORD	dwEyeColor;
	DWORD	dwSkinColor;
	USHORT	wFatigue;
	USHORT	wStdRebirthCoin;
	USHORT	wCashRebirthCoin;
	int		nHelmetID;
	int		nEarringID;
	BYTE cMemberIndex;		//Starts from 0
	UINT uiPvPUserState;
#if defined( PRE_WORLDCOMBINE_PARTY )
	int		nWorldSetID;				//World ID
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComebackAppellation;	
#endif
};

struct TKeySetting
{
	enum
	{
			Max1 = 29,
#if defined(PRE_ADD_SKILLSLOT_EXTEND)
			Max2 = 49,
#else	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
			Max2 = 39,
#endif	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
	};

	BYTE cVersion;
	BYTE arrKey1[Max1];
	BYTE arrKey2[Max2];

	TKeySetting()
	{
		cVersion = 0;
		memset( arrKey1, 0, Max1 );
		memset( arrKey2, 0, Max2 );
	}
};

struct TPadSetting
{
	enum
	{
#if defined(PRE_ADD_SKILLSLOT_EXTEND)
		Max = 50,
#else	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
		Max = 40,
#endif	// #if defined(PRE_ADD_SKILLSLOT_EXTEND)
	};

	BYTE cVersion;
	BYTE arrKey1[Max];
	BYTE arrKey2[Max];
	BYTE arrKey3[Max];
	BYTE arrKey4[Max];

	TPadSetting()
	{
		cVersion = 0;
		memset( arrKey1, 255, Max );
		memset( arrKey2, 255, Max );
		memset( arrKey3, 255, Max );
		memset( arrKey4, 255, Max );
	}
};

struct TSwapMemberIndex
{
	UINT nSessionID;
	BYTE cIndex;
};

//GameOptions 20 bytes
struct TGameOptions
{
	//To use directly without bit-shifting.... stored as contiguous memory in DB, so be careful when changing. Getting it out of sync is a headache.
	union
	{
		struct
		{
			//Please do not change the order; add new items at the bottom
			bool bUserName : 1;		//Display other users' names
			bool bPartyUserName : 1;	//Display party member names
			bool bMonsterName : 1;		//Display monster name
			bool bStateBarInsideCount : 1;	//Status bar number display
			bool bMyHPBar : 1;			//Display my HP bar
			bool bPartyHPBar : 1;		//Display party HP bar
			bool bMonsterHPBar : 1;		//Display monster bar
			bool bMyCombatInfo : 1;		//Display combat info
			bool bPartyCombatInfo : 1;	//Display party member combat info
			bool bDamageDirMelee : 1;	 // Melee hit direction indicator 
			bool bDamageDirRange : 1;	 // Ranged hit direction indicator 
			bool bGuildName : 1;		// Display guild name
			bool bGameTip : 1;			// Display bottom sliding game tip
			bool bSimpleGuide : 1;		// Quick guide
			bool bPVPLevelMark : 1;		// Display Colosseum rank
			bool bSkillCoolTimeCounter : 1; // Display skill cooldown

			//When adding, increment DISPLAYOPTIONCOUNT by the number of added items.
		};
		char cDisplayOption[5];		//1bit*40  = 5Byte
	};

	union
	{
		struct
		{
			//Please do not change the order; add new items at the bottom
			bool bDisplayGuildName : 1;		//Whether others can see my guild name
			bool bDisplayGender : 1;		//Whether others can see my gender
			bool bDisplayLevel : 1;			//Whether others can see my level

			//When adding, increment COMMDISPLAYOPTIONCOUNT by the number of added items.
		};
		char cCommunityDisplayOption[2];		//1bit*16  = 2byte
	};

	union
	{
		struct
		{
			//Please do not change the order; add new items at the bottom
			//use eCommunityAcceptableOptionType
			BYTE cPartyInviteAcceptable : 2;
			BYTE cGuildInviteAcceptable : 2;
			BYTE cTradeRequestAcceptable : 2;
			BYTE cDuelRequestAcceptable : 2;
			BYTE cObserveStuffAcceptable : 2;
			BYTE cLadderInviteAcceptable : 2;
			BYTE cQuickPvPInviteAcceptable : 2;
			BYTE cPartyChatAcceptable : 2; // PRE_ADD_CHAT_RENEWAL

			//When adding, increment COMMOPTIONCOUNT by the number of added items.
		};
		char cCommunityOption[11];			//11byte
	};

	union
	{
		struct
		{
			BYTE cDisplayOptionCount : 4;
			BYTE cCommunityDisplayOptionCount : 4;
			BYTE cCommunityOptionCount : 4;
		};
		char cGameOptionCount[2];			//2byte
	};

	void SetDefault()
	{
		bUserName = true;
		bPartyUserName = true;
		bMonsterName = true;
		bStateBarInsideCount = true;
		bMyHPBar = false;
		bPartyHPBar = false;
		bMonsterHPBar = true;
		bMyCombatInfo = true;
		bPartyCombatInfo = false;
		bDamageDirMelee = true;
		bDamageDirRange = true;
		bGuildName = true;
		bGameTip = true;
		bSimpleGuide = true;
		bPVPLevelMark = true;
		bSkillCoolTimeCounter = false;

		bDisplayGuildName = true;
		bDisplayGender = false;
		bDisplayLevel = true;

		cPartyInviteAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cGuildInviteAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cTradeRequestAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cDuelRequestAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cObserveStuffAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cLadderInviteAcceptable	= _CA_OPTIONTYPE_ACCEPT;
		cQuickPvPInviteAcceptable = _CA_OPTIONTYPE_ACCEPT;
		cPartyChatAcceptable = _CA_OPTIONTYPE_ACCEPT;

		cDisplayOptionCount = DISPLAYOPTIONCOUNT;
		cCommunityDisplayOptionCount = COMMDISPLAYOPTIONCOUNT;
		cCommunityOptionCount = COMMOPTIONCOUNT;
	}
};

struct TProfile
{
	BYTE	cGender;
	WCHAR	wszGreeting[PROFILEGREETINGLEN+1];
	bool	bOpenPublic;
};

struct TItemInfo
{
	BYTE cSlotIndex;
	TItem Item;
};

struct TRepurchaseItemInfo
{
	int iRepurchaseID;
	__time64_t tSellDate;
	int iSellPrice;
	TItem Item;
};

struct TGuildWareSlot
{
	BYTE cSlotIndex;
	INT64 nSerial;		// Serial
};

struct TGuildWareHistory
{
	int			nIndex;				// Index
	WCHAR		wszCharacterName[NAMELENMAX];		// Character name
	BYTE		cInOut;								// In/Out
	short		wCount;								// Quantity 0:Coin / +1:ItemCode
	int			nContent;							// Usage varies depending on detail cCount (quantity)
	__time64_t	EventDate;							// Event date
};


struct TEffectItemInfo
{
	int			iItemID;
	bool		bEternityFlag;
	__time64_t	ExpireDate;
	int			iProperty[5];
	INT64		biItemSerial;
};

struct TQuestItemCnt			// Declared for temporary creation to check if items can be added to inventory list
{
public:
	int nItemID;
	short wCount;		// Count

public:
	void Set(int _nItemID, short _wCount)
	{
		nItemID = _nItemID;
		wCount = _wCount;
	}

	void Reset()
	{
		nItemID = 0;
		wCount = 0;
	}
};

struct TInvenItemCnt			// Declared for temporary creation to check if items can be added to inventory list
{
public:
	int nItemID;
	short wCount;		// Count
	bool bSoulbound;
	char cSealCount;

public:
	void Set(int _nItemID, short _wCount, bool _bSoulbound, char _cSealCount )
	{
		nItemID = _nItemID;
		wCount = _wCount;
		bSoulbound = _bSoulbound;
		cSealCount = _cSealCount;
	}

	void Reset()
	{
		nItemID = 0;
		wCount = 0;
		bSoulbound = false;
		cSealCount = 0;
	}
};

struct TCommunityLocation
{
public:
	BYTE cServerLocation;	// eLocationState
	int nChannelID;			// 0 or -1 if in game
	int nMapIdx;			// -1 if unknown
public:
	TCommunityLocation()
	{
		Reset();
	}

	void Reset()
	{
		cServerLocation = _LOCATION_NONE;
		nChannelID = 0;
		nMapIdx = 0;
	}
	bool IsSet() const 
	{
		return(_LOCATION_NONE != cServerLocation);
	}
};

struct TCommunityLocationEx:public TCommunityLocation
{
public:
	INT64 biCharacterDBID;

	void Reset( INT64 CharDBID )
	{
		biCharacterDBID = CharDBID;
		TCommunityLocation::Reset();
	}
};

struct TGuildUID	// Guild UID
{
public:
	TGuildUID()
	{
		Reset();
	}
	TGuildUID(const TGuildUID& pRhv)
	{
		ui64UID = pRhv.ui64UID;
	}
	TGuildUID(int nWorldID, UINT nDBID)
	{
		Set(nWorldID, nDBID);
	}
	void Set(int nSetWorldID, UINT nSetDBID)
	{
		nWorldID = nSetWorldID;
		nDBID = nSetDBID;
	}
	void Reset()
	{
		ui64UID = GUILDUID_DEF;
	}
	bool IsSet() const
	{
		return(GUILDUID_DEF != ui64UID);
	}
	operator UINT64() const
	{
		return ui64UID;
	}
	bool operator==(const TGuildUID& pRhv) const	{ return(ui64UID == pRhv.ui64UID); }
	bool operator!=(const TGuildUID& pRhv) const	{ return(ui64UID != pRhv.ui64UID); }
	bool operator==(UINT64 pUid) const			{ return(ui64UID == pUid); }
	bool operator!=(UINT64 pUid) const			{ return(ui64UID != pUid); }
	bool operator<(const TGuildUID& pRhv) const		{ return(ui64UID < pRhv.ui64UID); }		// for stdext::hash_map etc.

private:
	union {
		UINT64 ui64UID;		// Guild UID
		struct {
			int nWorldID;			// World ID
			UINT nDBID;	// Guild DBID
		};
	};
};

struct TGuildView	// Guild time info
{
public:
	TGuildView()
	{
		Reset();
	}
	void Set(const TGuildView& pGuildView)
	{
		Set(pGuildView.GuildUID, pGuildView.wszGuildName);

		wWarSchduleID = pGuildView.wWarSchduleID;
		cTeamColorCode = pGuildView.cTeamColorCode;	

		wGuildMark = pGuildView.wGuildMark;
		wGuildMarkBG = pGuildView.wGuildMarkBG;
		wGuildMarkBorder = pGuildView.wGuildMarkBorder;
	}
	void Set(const TGuildUID pGuildUID, LPCWSTR lpwszGuildName)
	{
		GuildUID = pGuildUID;
		if (lpwszGuildName) 
			::wcsncpy_s(wszGuildName, _countof(wszGuildName), lpwszGuildName, _countof(wszGuildName));
		else 
			wszGuildName[0] = L'\0';		
	}
	void Reset()
	{
		GuildUID.Reset();
		wszGuildName[0] = L'\0';
		wWarSchduleID = 0;
		cTeamColorCode = 0;
		wGuildMark = 0;
		wGuildMarkBG = 0;
		wGuildMarkBorder = 0;
	}
	bool IsSet() const
	{
		return(GuildUID.IsSet());
	}
	bool operator==(const TGuildView& pRhv) const
	{
		return((pRhv.GuildUID == GuildUID));
	}
	bool operator!=(const TGuildView& pRhv) const
	{
		return(!(this->operator==(pRhv)));
	}
public:
	TGuildUID GuildUID;						// Guild UID (World ID + Guild DBID)
	WCHAR wszGuildName[GUILDNAME_MAX];		// Guild name (assumes name does not change after guild creation)
	short wWarSchduleID;
	BYTE cTeamColorCode;					// 0: none / 1: Blue team / 2: Red team
	short wGuildMark;						// Guild mark emblem
	short wGuildMarkBG;						// Guild mark background
	short wGuildMarkBorder;					// Guild mark border
};

struct TGuildSelfView : public TGuildView	// Guild time info (personal) // P.S. Never use multiple inheritance
{
public:
	TGuildSelfView()
	{
		Reset();
	}
	TGuildSelfView(const TGuildView& pGuildView, BYTE btGuildRole)
	{
		Set(pGuildView, btGuildRole);
	}
	TGuildSelfView(const TGuildSelfView& pGuildSelfView)
	{
		Set(pGuildSelfView);
	}
	void Reset()
	{
		btGuildRole = GUILDROLE_TYPE_CNT;
		TGuildView::Reset();
	}
	void Set(const TGuildView& pGuildView, BYTE cGuildRole)
	{
		btGuildRole = cGuildRole;
		TGuildView::Set(pGuildView);
	}	
	void Set(const TGuildSelfView& pGuildSelfView)
	{
		btGuildRole = pGuildSelfView.btGuildRole;
		TGuildView::Set(pGuildSelfView);
	}	
	bool operator==(const TGuildSelfView& pRhv) const
	{
		// Only consider parent class (guild itself) members during equality comparison?
		return(TGuildView::operator==(pRhv) && pRhv.btGuildRole == btGuildRole);
	}
	bool operator!=(const TGuildSelfView& pRhv) const
	{
		return(!(this->operator==(pRhv)));
	}
public:
	BYTE btGuildRole;	// Guild rank
};

struct TGuildRewardItem
{
	int			nItemID;		// Item ID	
	int			nItemType;		// Type
	short		    nEffectValue;	// Effect value
#if defined( PRE_ADD_GUILDREWARDITEM )
	int		    nEffectValue2;	// Effect value 2
#endif
	bool		bEternity;		// Whether permanent
	__time64_t	m_tExpireDate;	// Duration
};

struct TGuild		// Guild info
{
public:
	TGuild()
	{
		Reset();
	}
	void Reset()
	{
		GuildView.Reset();
		ui64GuildMoney = 0;
		wGuildLevel = 1;
		wszGuildNotice[0] = L'\0';
		::memset(iGuildMedal, 0, sizeof(iGuildMedal));
		wGuildSize = 0;
		wGuildWareSize = 0;
		::memset(&OpenDate, 0, sizeof(OpenDate));
		::memset(wszGuildRoleName, 0, sizeof(wszGuildRoleName));
		::memset(wGuildRoleAuth, 0, sizeof(wGuildRoleAuth));
		::memset(nRoleMaxItem, 0, sizeof(nRoleMaxItem));
		::memset(nRoleMaxCoin, 0, sizeof(nRoleMaxCoin));
		nGuildWarFinalCount = 0;
		nGuildWarFinalWinCount = 0;
		iTotalGuildExp = 0;
		iDailyStagePoint = 0;
		iDailyMissionPoint = 0;
		iWarPoint = 0;
		::memset(GuildRewardItem, 0, sizeof(GuildRewardItem));
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		::memset(wszGuildHomePage, 0, sizeof(wszGuildHomePage));
#endif
	}
	bool IsSet() const
	{
		return(GuildView.IsSet());
	}
	bool CheckAuth(eGuildRoleType pGuildRoleType, eGuildAuthType pGuildAuthType) const
	{
		if (wGuildRoleAuth[pGuildRoleType] & (static_cast<int>(1) << pGuildAuthType)) {
			return true;
		}		
		return false;
	}

public:
	TGuildView GuildView;							// Guild time info
	INT64 ui64GuildMoney;							// Guild funds
	short wGuildLevel;								// Guild level
	WCHAR wszGuildNotice[GUILDNOTICE_MAX+1];		// Guild notice
	int iGuildMedal[GUILDMEDAL_TYPE_CNT];			// Guild medal
	short wGuildSize;								// Guild extra/total member capacity
	short wGuildWareSize;							// Max guild storage count
	__time64_t OpenDate;							// Guild creation date
	WCHAR wszGuildRoleName[GUILDROLE_TYPE_CNT][GUILDROLENAME_MAX+1];// Name per guild role
	int wGuildRoleAuth[GUILDROLE_TYPE_CNT];			// Permissions per guild role
	INT nRoleMaxItem[GUILDROLE_TYPE_CNT];			// Guild storage item count limit by guild role (index)
	INT nRoleMaxCoin[GUILDROLE_TYPE_CNT];			// Max withdrawal amount by guild role (index)
	short nGuildWarFinalCount;						// Guild war finals entry count
	short nGuildWarFinalWinCount;					// Guild war finals win count
	int iTotalGuildExp;								// Guild EXP
	int iDailyStagePoint;							// Stage points
	int iDailyMissionPoint;							// Mission points
	int iWarPoint;									// Guild war points
	TGuildRewardItem GuildRewardItem[GUILDREWARDEFFECT_TYPE_CNT];	// Applied guild reward effect
	WCHAR wszGuildHomePage[GUILDHISTORYTEXT_MAX+1];		// Guild homepage
	char cGuildType;							// Ref: BeginnerGuild
};

struct TGuildMember		// Guild member info
{
public:
	TGuildMember()
	{
		Reset();
	}
	void Set(UINT nMemberADBID, INT64 nMemberCDBID, LPCWSTR lpwszMemberCharacterName, char cMemberJob, char cMemberLevel, BYTE cMemberGuildRole, __time64_t tMemberJoinDate = 0, long nMemberUseGuildCommonPoint = 0, 
		long nMemberTotGuildCommonPoint = 0, LPCWSTR lpwszGuildMemberIntroduce = NULL, __time64_t tLastLogoutDate = 0, BYTE cMemberServerLocation = 0, int nMemberChannelID = 0, int nMemberMapIdx = 0)
	{
		nAccountDBID = nMemberADBID;
		nCharacterDBID = nMemberCDBID;
		::wcsncpy_s(wszCharacterName, _countof(wszCharacterName), lpwszMemberCharacterName, _countof(wszCharacterName));
		nJob = cMemberJob;
		cLevel = cMemberLevel;
		btGuildRole = cMemberGuildRole;
		JoinDate = tMemberJoinDate;
		iUseGuildCommonPoint = nMemberUseGuildCommonPoint;
		iTotGuildCommonPoint = nMemberTotGuildCommonPoint;
		if (lpwszGuildMemberIntroduce) {
			::wcsncpy_s(wszGuildMemberIntroduce, _countof(wszGuildMemberIntroduce), lpwszGuildMemberIntroduce, _countof(wszGuildMemberIntroduce));
		}
		LastLogoutDate = tLastLogoutDate;
		Location.cServerLocation = cMemberServerLocation;
		Location.nChannelID = nMemberChannelID;
		Location.nMapIdx = nMemberMapIdx;
	}
	void Reset()
	{
		nCharacterDBID = 0;
		btGuildRole = GUILDROLE_TYPE_CNT;
		iUseGuildCommonPoint = 0;
		iTotGuildCommonPoint = 0;
		wszGuildMemberIntroduce[0] = L'\0';
		JoinDate = 0;

		nAccountDBID = 0;
		wszCharacterName[0] = L'\0';
		nJob = 0;
		cLevel = 0;
		LastLogoutDate = 0;

		Location.cServerLocation = _LOCATION_NONE;
		Location.nChannelID = 0;
		Location.nMapIdx = 0;

		nDelMembNo = 0;

		TakeWareItemCount = 0;
		WithdrawCoin = 0;

		WareVeriosnDate = 0;
		bOpenWareStatus = false;

		LastWareUseDate = 0;
	}
	bool IsSet() const
	{
		return(0 != nCharacterDBID);
	}
public:
	// Guild member DB info
	INT64 nCharacterDBID;					// Character DBID
	BYTE btGuildRole;				// Guild role
	long iUseGuildCommonPoint;	// Guild normal points (spent)
	long iTotGuildCommonPoint;	// Guild normal points (contribution)
	WCHAR wszGuildMemberIntroduce[GUILDMEMBINTRODUCE_MAX+1];	// Guild member self-introduction
	__time64_t JoinDate;				// Guild join date

	// Character DB info
	UINT nAccountDBID;					// Account DBID
	WCHAR wszCharacterName[NAMELENMAX];	// Character name
	TP_JOB nJob;							// Job change class
	CHAR cLevel;							// Level
	__time64_t LastLogoutDate;			// Last logout date

	// Character real-time info
	TCommunityLocation Location;			// Current location (login status is also checked via this member's state)

	int TakeWareItemCount;				// Number of item withdrawals today
	INT64 WithdrawCoin;					// Amount withdrawn today

	__time64_t WareVeriosnDate;			// Storage access version
	bool bOpenWareStatus;					// Storage open state

	__time64_t LastWareUseDate;			// Last date storage was used

	// Server management
	BYTE nDelMembNo;			// Character deletion management ID
};

struct TGuildHistory	// Guild history info (instance)
{
	INT64 biIndex;		// Guild history index
	__time64_t RegDate;				// Guild history registration date
	BYTE btHistoryType;	// Guild history type (eGuildHistoryType)
	int nInt1;
	int nInt2;
	INT64 biInt64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
};

// Guild recruitment board registered guild info
struct TGuildRecruitInfo
{
	TGuildUID GuildUID;										// Guild UID (World ID + Guild DBID)
	WCHAR wszGuildName[GUILDNAME_MAX];					// Guild name
	short wGuildMark;										// Guild mark emblem
	short wGuildMarkBG;										// Guild mark background
	short wGuildMarkBorder;									// Guild mark border
	
	WCHAR wszGuildRecruitNotice[GUILDRECRUITINTRODUCE+1];	// Guild recruitment board introduction	
	short wGuildLevel;										// Guild level
	short wGuildMemberPresentSize;							// Current number of guild members	
	short wGuildMemberMaxSize;								// Max guild member count
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE		cPurposeCode;								// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
	WCHAR		wszGuildHomePage[GUILDHISTORYTEXT_MAX+1];		//Guild homepage
	WCHAR		wszGuildMasterName[NAMELENMAX+1];			// Guild master name
#endif
};
// List of users who applied to join guild via recruitment board
struct TGuildRecruitCharacter 
{
	INT64 nCharacterDBID;					// Character DBID	
	WCHAR wszCharacterName[NAMELENMAX+1];		// Character name
	TP_JOB nJob;							// Job change class
	BYTE cLevel;							// Level
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TNpcReputation
{
	int					iNpcID;
	int					iFavorPoint;
	int					iMalicePoint;

	TNpcReputation()
		:iNpcID(0),iFavorPoint(0),iMalicePoint(0)
	{
	}
	TNpcReputation( int NpcID, int FavorPoint, int MalicePoint )
		:iNpcID(NpcID),iFavorPoint(FavorPoint),iMalicePoint(MalicePoint)
	{
	}

	void Set( int NpcID, int FavorPoint, int MalicePoint )
	{
		iNpcID			= NpcID;
		iFavorPoint		= FavorPoint;
		iMalicePoint	= MalicePoint;
	}
};
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TUnionReputePointInfo
{
	INT64	iUnionPoint[NpcReputation::UnionType::Etc];

	void SetUnionReputePoint( BYTE cType, INT64 iPoint )
	{
		if (NpcReputation::UnionType::Commercial > cType || cType >= NpcReputation::UnionType::Etc)
			return;

		iUnionPoint[cType] = iPoint;
	}

	INT64	GetUnionReputePoint( BYTE cType ) const
	{
		if (NpcReputation::UnionType::Commercial > cType || cType >= NpcReputation::UnionType::Etc)
			return 0;

		return iUnionPoint[cType];
	}

	bool DelUnionReputePoint (BYTE cType, INT64 iPoint)
	{
		if (NpcReputation::UnionType::Commercial > cType || cType >= NpcReputation::UnionType::Etc)
			return false;

		if (iUnionPoint[cType] < iPoint)
			return false;

		iUnionPoint[cType] -= iPoint;

		return true;
	}
};

struct SCUnionPoint
{
	INT64 biUnionPoint[NpcReputation::UnionType::Etc];
};

struct SCUnionMembershipList		// SC_REPUTATION / SC_UNIONMEMBERSHIPLIST
{	
	int nItemID[NpcReputation::UnionType::Etc];
	int nLeftUseTime[NpcReputation::UnionType::Etc];
};

struct SCBuyUnionMembershipResult		// SC_REPUTATION / SC_REFRESHUNIONMEMBERSHIP
{
	BYTE cType;
	int nItemID;
	int nLeftUseTime; // Remaining usage time (in minutes)
};

struct TMasterInfo
{
	INT64	biCharacterDBID;
	WCHAR	wszCharName[NAMELENMAX];
	BYTE	cLevel;
	BYTE	cJobCode;
	BYTE	cGenderCode;
	int		iRespectPoint;
	WCHAR	wszSelfIntrodution[MasterSystem::Max::SelfIntrotuctionLen];
	int		iGraduateCount;
};

struct TPupilInfo
{
	INT64		biCharacterDBID;
	WCHAR		wszCharName[NAMELENMAX];
	BYTE		cLevel;
	BYTE		cJob;
	int			iFavorPoint;
	__time64_t	tLastConnectDate;

	TPupilInfo()
	{
		biCharacterDBID		= 0;
		cLevel				= 0;
		cJob				= 0;
		iFavorPoint			= 0;
		tLastConnectDate	= 0;
		memset( wszCharName, 0, sizeof(wszCharName) );
	}
};

struct TPupilInfoWithLocation:public TPupilInfo
{
	TCommunityLocation	Location;

	TPupilInfoWithLocation()
	{
		Location.Reset();
	}

	TPupilInfoWithLocation( TPupilInfo& data )
	{
		Location.Reset();

		biCharacterDBID		= data.biCharacterDBID;
		cLevel				= data.cLevel;
		cJob				= data.cJob;
		iFavorPoint			= data.iFavorPoint;
		tLastConnectDate	= data.tLastConnectDate;
		memcpy( wszCharName, data.wszCharName, sizeof(wszCharName) );
	}
};

struct TMasterAndClassmateInfo
{
	MasterSystem::MasterClassmateKind::eCode	Code;
	INT64										biCharacterDBID;
	WCHAR										wszCharName[NAMELENMAX];
	__time64_t									tLastConnectDate;

	TMasterAndClassmateInfo()
	{
		Code				= MasterSystem::MasterClassmateKind::Max;
		biCharacterDBID		= 0;
		tLastConnectDate	= 0;
		memset( wszCharName, 0, sizeof(wszCharName) );
	}
};

struct TMasterAndClassmateInfoWithLocation:public TMasterAndClassmateInfo
{
	TCommunityLocation	Location;

	TMasterAndClassmateInfoWithLocation()
	{
		Location.Reset();
	}

	TMasterAndClassmateInfoWithLocation( TMasterAndClassmateInfo& data )
	{
		Location.Reset();

		Code				= data.Code;
		biCharacterDBID		= data.biCharacterDBID;
		tLastConnectDate	= data.tLastConnectDate;
		memcpy( wszCharName, data.wszCharName, sizeof(wszCharName) );
	}
};

struct TClassmateInfo
{
	INT64	biCharacterDBID;
	WCHAR	wszCharName[NAMELENMAX];
	BYTE	cJob;
	BYTE	cLevel;
	BYTE	cMasterCount;
	WCHAR	wszMasterCharName[MasterSystem::Max::MasterCount][NAMELENMAX];

	TClassmateInfo()
	{
		biCharacterDBID = 0;
		cJob			= 0;
		cLevel			= 0;
		cMasterCount	= 0;
		memset( wszCharName, 0, sizeof(wszCharName) );
		memset( wszMasterCharName, 0, sizeof(wszMasterCharName) );
	}
};

struct TClassmateInfoWithLocation:public TClassmateInfo
{
	TCommunityLocation Location;

	TClassmateInfoWithLocation()
	{
		Location.Reset();
	}

	TClassmateInfoWithLocation( TClassmateInfo& data )
	{
		Location.Reset();

		biCharacterDBID = data.biCharacterDBID;
		cJob			= data.cJob;
		cLevel			= data.cLevel;
		cMasterCount	= data.cMasterCount;
		memcpy( wszCharName, data.wszCharName, sizeof(wszCharName) );
		for( UINT i=0 ; i<cMasterCount ; ++i )
			memcpy( wszMasterCharName[i], data.wszMasterCharName[i], sizeof(wszMasterCharName[i]) );
	}
};

struct TMasterCharacterInfo
{
	bool	bMasterListShowFlag;
	WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
	int		iGraduateCount;
	int		iRespectPoint;
};

struct TMyMasterInfo
{
	INT64	biCharacterDBID;
	WCHAR	wszCharName[NAMELENMAX];
	BYTE	cJob;
	BYTE	cLevel;
	int		iRespectPoint;
	int		iFavorPoint;

	TMyMasterInfo()
	{
		biCharacterDBID = 0;
		cJob			= 0;
		cLevel			= 0;
		iRespectPoint	= 0;
		iFavorPoint		= 0;
		memset( wszCharName, 0, sizeof(wszCharName) );
	}
};

struct TMyMasterInfoWithLocation:public TMyMasterInfo
{
	TCommunityLocation Location;

	TMyMasterInfoWithLocation()
	{
		Location.Reset();
	}

	TMyMasterInfoWithLocation( TMyMasterInfo& data )
	{
		Location.Reset();

		biCharacterDBID	= data.biCharacterDBID;
		cJob			= data.cJob;
		cLevel			= data.cLevel;
		iRespectPoint	= data.iRespectPoint;
		iFavorPoint		= data.iFavorPoint;
		memcpy( wszCharName, data.wszCharName, sizeof(wszCharName) );
	}
};

struct TMasterAndFavorPoint
{
	INT64	biCharacterDBID;
	int		iFavorPoint;
	WCHAR	wszCharName[NAMELENMAX];
};

struct TVehicleBase
{
	DWORD dwPartsColor1;	// Vehicle:Hair, Pet:Body
	DWORD dwPartsColor2;	// Vehicle: None, Pet: Nose
	int nExp;
	int nRange;				// Pet loot pickup range
	WCHAR	wszNickName[NAMELENMAX];
	bool bSkillSlot;	// Whether pet skill slot is expanded
	int nSkillID1;	// Skill ID 1
	int nSkillID2;	// Skill ID 2
	int nCurrentSatiety;
	int nType;
	DWORD dwLastHungerTick;
};

struct TVehicle: public TVehicleBase
{
	TItem Vehicle[Vehicle::Slot::Max];
};

struct TPartyVehicle
{
	UINT nSessionID;
	TVehicle tVehicle;
};

struct TVehicleItem
{
	int nItemID;
	INT64 nSerial;
	short wCount;
	bool bEternity;					// Permanent item
	__time64_t tExpireDate;

	void SetItem(const TItem &Item)
	{
		nItemID = Item.nItemID;
		nSerial = Item.nSerial;
		wCount = Item.wCount;
		bEternity = Item.bEternity;
		tExpireDate = Item.tExpireDate;
	}
};

struct TVehicleCompact: TVehicleBase
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TVehicleItem Vehicle[Vehicle::Slot::Max];
	__time64_t tLastHungerModifyDate;

	void SetCompact( const TVehicle &VehicleData )
	{
		dwPartsColor1 = VehicleData.dwPartsColor1;
		dwPartsColor2 = VehicleData.dwPartsColor2;
		nExp = VehicleData.nExp;
		nRange = VehicleData.nRange;
		memcpy( wszNickName, VehicleData.wszNickName, sizeof(wszNickName) );
		bSkillSlot = VehicleData.bSkillSlot;
		nSkillID1 = VehicleData.nSkillID1;
		nSkillID2 = VehicleData.nSkillID2;
		nType = VehicleData.nType;
		nCurrentSatiety = VehicleData.nCurrentSatiety;
		for (int i = 0; i < Vehicle::Slot::Max; i++){
			Vehicle[i].SetItem(VehicleData.Vehicle[i]);
		}
	}
};

#if defined( PRE_ADD_SECONDARY_SKILL )

struct TSecondarySkill
{
	int							iSkillID;
	int							iExp;
	//SecondarySkill::Type::eType	Type;
};

struct TSecondarySkillInfo
{
	int								iSkillID;
	int								iExp;
	SecondarySkill::Grade::eType	Grade;
	int								iLevel;
};

struct TSecondarySkillRecipe
{
	int			iSkillID;
	INT64		biItemSerial;
	int			iItemID;
	short		nExp;
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

struct TPvPLadderMatchTypeScore
{
	BYTE			cPvPLadderCode;						// LadderSystem::MatchType::eCode
	int				iPvPLadderGradePoint;
	int				iHiddenPvPLadderGradePoint;
	int				iWin,iLose,iDraw;
	short			nTodayWin,nTodayLose,nTodayDraw;
	short			nConsecutiveWin,nConsecutiveLose;
	__time64_t		tLasePvPLadderScoreDate;
	short			nWeeklyCount;

	void InitWeeklyGradePoint()
	{
		nWeeklyCount = 0;
	}

	int GetExchangeLadderPoint( bool bCheckMinGamePlay=true ) const
	{
		if( bCheckMinGamePlay == true )
		{
			if( nWeeklyCount < LadderSystem::Exchange::MinGamePlay )
				return 0;
		}

		int iLP = 0;
		if( iPvPLadderGradePoint <= 1500 )
		{
			iLP = static_cast<int>((0.34f*iPvPLadderGradePoint))+450;
		}
		else
		{
			iLP = static_cast<int>(1511.26f/static_cast<float>((1+40.f*pow(2.f,(-0.00412f*iPvPLadderGradePoint)))));
		}

		switch( cPvPLadderCode )
		{
			case LadderSystem::MatchType::_1vs1:
			{
				iLP = static_cast<int>(iLP*0.72f);
				break;
			}
			case LadderSystem::MatchType::_2vs2:
			{
#ifdef PRE_ADD_2vs2_LADDER
				iLP = static_cast<int>(iLP*0.89f);
#else
				iLP = static_cast<int>(iLP*0.84f);
#endif
				
				break;
			}
			case LadderSystem::MatchType::_3vs3:
			{
#ifdef PRE_ADD_2vs2_LADDER
				iLP = static_cast<int>(iLP*0.95f);
#else
				iLP = static_cast<int>(iLP*0.90f);
#endif
				
				break;
			}
		}
		return iLP;
	}
};

struct TPvPLadderScoreInfo
{
	int							iPvPLadderPoint;
	BYTE						cMaxKillJobCode;
	BYTE						cMaxDeathJobCode;
	BYTE						cLadderTypeCount;
	TPvPLadderMatchTypeScore	LadderScore[LadderSystem::MatchType::MaxCount];

	void SetLadderScoreForCheat( LadderSystem::MatchType::eCode Type, int iLadderPoint, int iGradePoint, int iHiddenGradePoint )
	{
		iPvPLadderPoint = iLadderPoint;

		bool bEmpty = true;
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			if( LadderScore[i].cPvPLadderCode == Type )
			{
				LadderScore[i].iPvPLadderGradePoint			= iGradePoint;
				LadderScore[i].iHiddenPvPLadderGradePoint	= iHiddenGradePoint;
				LadderScore[i].nWeeklyCount = LadderSystem::Exchange::MinGamePlay;
				bEmpty = false;
				break;
			}
		}

		if( bEmpty == true )
		{
			if( cLadderTypeCount < LadderSystem::MatchType::MaxCount )
				return;
			TPvPLadderMatchTypeScore Data;
			memset( &Data, 0, sizeof(Data) );
			Data.iPvPLadderGradePoint		= iGradePoint;
			Data.iHiddenPvPLadderGradePoint	= iHiddenGradePoint;
			LadderScore[cLadderTypeCount++] = Data;
		}
	}

	int GetGradePoint( LadderSystem::MatchType::eCode Type ) const
	{
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			if( LadderScore[i].cPvPLadderCode == Type )
				return LadderScore[i].iPvPLadderGradePoint;
		}

		return LadderSystem::Stats::InitGradePoint;
	}

	int GetTopGradePoint() const
	{
		int iTop = 0;
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			if( LadderScore[i].iPvPLadderGradePoint > iTop )
				iTop = LadderScore[i].iPvPLadderGradePoint;
		}

		return iTop;
	}

	int GetHiddenGradePoint( LadderSystem::MatchType::eCode Type ) const
	{
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			if( LadderScore[i].cPvPLadderCode == Type )
				return LadderScore[i].iHiddenPvPLadderGradePoint;
		}

		return LadderSystem::Stats::InitGradePoint;
	}

	int GetExchageLadderPoint( LadderSystem::MatchType::eCode Type ) const
	{
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			if( LadderScore[i].cPvPLadderCode == Type )
				return LadderScore[i].GetExchangeLadderPoint();
		}

		return 0;
	}

	int GetTopExchangeLadderPoint() const
	{
		int iTop = 0;
		for( int i=0 ; i<cLadderTypeCount ; ++i )
		{
			int iVal = LadderScore[i].GetExchangeLadderPoint();
			if( iVal > iTop )
				iTop = iVal;
		}

		return iTop;
	}

	void UseLadderPoint( int iPoint )
	{
		if( iPvPLadderPoint < iPoint )
		{
			_ASSERT(0);
			return;
		}

		iPvPLadderPoint -= iPoint;
	}
};

struct TPvPLadderJobScore
{
	BYTE	cJobCode;
	int		iWin;
	int		iLose;
	int		iDraw;
};

struct TPvPLadderScoreInfoByJob
{
	BYTE				cJobCount;
	TPvPLadderJobScore	LadderScoreByJob[JOB_KIND_MAX];
};

struct TPvPLadderRanking
{
	INT64	biCharacterDBID;
	int		iRank;
	int		iPvPLadderGradePoint;
	WCHAR	wszCharName[NAMELENMAX];
	BYTE	cJobCode;
	int		iWin,iLose,iDraw;
};
struct TPvPGhoulScores
{
	int		nPoint[GhoulMode::PointType::Max];	
};

struct TPeriodResetItem
{
	BYTE cType;
	int iValue;
	__time64_t tDate;
};


//--------------------------------------------------------------
//                         Packet-related
//--------------------------------------------------------------
//-------------------------------------------------
//	Client <-> Login
//-------------------------------------------------
/* 
	CS_CHECKLOGIN -> SC_CHECKLOGIN
	CS_SERVERLIST -> SC_SERVERLIST
	CS_SELECTSERVER -> SC_CHARLIST
	CS_SELECTCHAR -> SC_CHANNELLIST
CS_SELECTCHANNEL -> To village!
*/


struct CSCheckVersion	 		// CS_LOGIN / CS_CHECKVERSION
{
	BYTE cNation;
	BYTE cVersion;
	bool bCheck;
	USHORT nMajorVersion;
	USHORT nMinorVersion;

};

struct SCCheckVersion			// SC_LOGIN / SC_CHECKVERISON
{
	int nRet;
	char szServerVersion[SERVERVERSIONMAX];
	TDatabaseVersion MembershipDBVersion;
	TDatabaseVersion WorldDBVersion;
};

struct CSCheckLogin				// CS_LOGIN / CS_CHECKLOGIN
{
	WCHAR wszUserId[IDLENMAX];
	WCHAR wszPassword[PASSWORDLENMAX];
	WCHAR wszVirtualIp[IPLENMAX];
};

struct CSCheckLoginUS			// CS_LOGIN / CS_CHECKLOGIN_US (0x22)
{
	BYTE cAuthFlag;
	WCHAR wszPassport[1024];
	WCHAR wszVirtualIp[IPLENMAX];
};

struct SCCheckLogin				// SC_LOGIN / SC_CHECKLOGIN
{
	UINT nSessionID;
	int nRet;
#if !defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharCountMax;
	BYTE cGender;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	__time64_t _tServerTime;
	TGameOptions GameOption;
#ifdef PRE_ADD_COMEBACK
	bool bComebackUser;
#endif		//#ifdef PRE_ADD_COMEBACK
#if defined( PRE_ADD_GAMEQUIT_REWARD )
	bool bReConnectNewbieReward;	// e.g. New user reconnect reward recipient (NewbieReward: true/RegTime: creation time)
	__time64_t _tAccountRegistTime; //     If new user reward time remains (NewbieReward: false / RegTime: creation time); if ineligible (false / 0)
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )
#ifdef PRE_ADD_DOORS
	bool bMobileAuthentication;
#endif		//PRE_ADD_DOORS
};

struct TServerListData
{
	BYTE cWorldID;
	WCHAR wszServerName[WORLDNAMELENMAX];
	UINT nWorldMaxUser;
	UINT nWorldCurUser;
	bool bOnline;
	bool bOnTop;
	BYTE	cMyCharCount;
};

struct SCServerList				// SC_LOGIN / SC_SERVERLIST
{
	BYTE cServerCount;
	TServerListData ServerListData[WORLDCOUNTMAX];
};

struct CSSelectServer			// CS_LOGIN / CS_SELECTSERVER
{
	char cWorldID;
};

struct TCharListData
{
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	BYTE cWorldSetID;
#endif
#if defined(PRE_MOD_SELECT_CHAR)
	BYTE cWorldID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	WCHAR wszCharacterName[NAMELENMAX];
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cLevel;
	BYTE cJob;
	int nMapID;
	int nEquipArray[EQUIPMAX];
	int nDefaultBody;	// Equip set on initial creation
	int nDefaultLeg;
	int nDefaultHand;
	int nDefaultFoot;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	INT64 biUniqueID;	// For use in Nexon Messenger (?)
	int nCashEquipArray[CASHEQUIPMAX];
	bool bDeleteFlag;
	__time64_t DeleteDate;
#if defined(PRE_CHARLIST_SORTING)
	__time64_t LastLoginDate;
	__time64_t CreateDate;
#endif	// #if defined(PRE_CHARLIST_SORTING)
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCombackEffectItem;
#endif
};

struct SCCharList				// SC_LOGIN / SC_CHARLIST
{
	int nRet;
#if defined(PRE_CHARLIST_SORTING)
	BYTE cCharacterSortCode;	// Login::CharacterSortCode
#endif	// #if defined(PRE_CHARLIST_SORTING)
	BYTE cAccountLevel;
	BYTE cCharCountMax;
	BYTE cCharCount;	// How many
	TCharListData CharListData[CHARCOUNTMAX];
};

struct CSSelectChar				// CS_LOGIN / CS_SELECTCHAR
{
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	int nSeed;
	int nValue[SecondAuth::Common::PWMaxLength];	// 2nd auth password value
#ifdef PRE_ADD_23829
	bool bSetIgnoreAuthNotify;
#endif
};

struct CSCreateChar				// CS_LOGIN / CS_CREATECHAR
{
#if defined(PRE_MOD_SELECT_CHAR)
	BYTE cWorldID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	WCHAR wszCharacterName[NAMELENMAX];
	int nEquipArray[EQUIPMAX];
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	BYTE cClass;
};

struct SCCreateChar				// SC_LOGIN / SC_CREATECHAR
{
	WCHAR wszCharacterName[NAMELENMAX];
	int nRet;
	char cDailyCreateMaxCount;	// Max count
};

struct CSDeleteChar				// CS_LOGIN / CS_DELETECHAR
{
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	int nSeed;
	int nValue[SecondAuth::Common::PWMaxLength];
};

struct SCDeleteChar				// SC_LOGIN / SC_DELETECHAR
{
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	int nRet;
	BYTE cFailCount;
};

struct sChannelInfo
{
	UINT nChannelID;
	USHORT nChannelIdx;			//for UI
	UINT nMapIdx;
	USHORT nCurrentUserCount;
	USHORT nMaxUserCount;
	int nChannelAttribute;
	int nMeritBonusID;
	BYTE cMinLevel;
	BYTE cMaxLevel;
	BYTE cVillageID;
	bool bVisibility;

	char szIP[IPLENMAX];
	USHORT nPort;
	int nLimitLevel;
	bool bShow;
	USHORT nDependentMapID;
	int nServerID;
	CHAR cThreadID;
#if defined(PRE_ADD_CHANNELNAME)
	WCHAR wszLanguageName[CHANNELLANGUAGEMAX];
#endif // #if defined(PRE_ADD_CHANNELNAME)
};

struct TMeritInfo
{
	int nID;
	int nMinLevel;
	int nMaxLevel;
	int nMeritType;
	int nExtendValue;
};

struct SCChannelList			// SC_LOGIN / SC_CHANNELLIST
{
	int  nRet;
	BYTE cFailCount;
	BYTE cCount;
	sChannelInfo Info[VILLAGECHANNELMAX];	
};

struct CSSelectChannel			// SC_LOGIN / SC_SELECTCHANNEL
{
	int nChannelID;
};

struct SCTutorialMapInfo		// SC_LOGIN / SC_TUTORIALMAPINFO
{
	int nMapIndex;
	char cGateNo;
};

struct SCWaitUser				// SC_LOGIN / SC_WAITUSER
{
	BYTE cWorldID;
	UINT nWaitUserCount;
	ULONG nEstimateTime;
};

struct SCCHeckBlock				// SC_LOGIN / SC_CHECKBLOCK
{
	__time64_t ServerDate;
	__time64_t StartDate;
	__time64_t EndDate;
	WCHAR wszBlockReason[RESTRAINTREASONMAX];
	BYTE cType;
	int iReasonID;
};

struct CSReviveChar
{
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
};

struct SCReviveChar
{
#if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	int nRet;
};

struct CSCharacterSortCode		// CS_LOGIN / CS_CHARACTERSORTCODE
{
	BYTE cCharacterSortCode;
};


//-------------------------------------------------
//	SYSTEM (CS_SYSTEM, SC_SYSTEM)
//-------------------------------------------------

struct SCVillageInfo			// SC_SYSTEM / SC_VILLAGEINFO
{
	UINT	nSessionID;
	char	szVillageIp[IPLENMAX];
	USHORT	wVillagePort;
	short	nRet;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
};

struct CSConnectVillage			// CS_SYSTEM / CS_CONNECTVILLAGE
{
	UINT nSessionID;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
	WCHAR wszVirtualIp[IPLENMAX];	// virtual ip
};

struct SCDetachMessage			//SC_SYSTEM / SC_DETACHCONNECTION_MSG
{
	int nRetCode;
#if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
	bool bDisConnectByClient;
#endif	// #if defined(PRE_MOD_DUPLICATION_LOGIN_MES)
};

struct SCZeroPopulation			//SC_SYSTEM / SC_ZEROPOPULATION
{
	bool bZeroPopulation;
};

struct SCConnectVillage			// SC_SYSTEM / SC_CONNECTVILLAGE
{
	short	nRet;
	char	szServerVersion[SERVERVERSIONMAX];
	int		nChannelAttr;
	BYTE	cWithoutLoading;
};

struct CSVillageReady			// CS_SYSTEM / CS_VILLAGEREADY
{
	bool boFirst;
	bool boChannelMove;
};

struct SCGameInfo				// SC_SYSTEM / SC_GAMEINFO
{
	GameTaskType::eType GameTaskType;
	BYTE	cReqGameIDType;
	ULONG	nGameServerIP;
	USHORT	nGameServerPort;
	USHORT	nGameServerTcpPort;
	int nRet;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
};

struct CSConnectRequest			// CS_SYSTEM / CS_CONNECT_REQUEST
{
	UINT nSessionID;
	ULONG nAddrIP;
	USHORT nPort;
};

struct SCConnectedResult		// SC_SYSTEM / SC_CONNECTED_RESULT
{
	UINT nSessionID;				//Just lol okay~
	char szServerVersion[SERVERVERSIONMAX];
};

struct CSReady2Recieve			// CS_SYSTEM / CS_READY_2_RECIEVE
{
	UINT nSessionID;						//Just in case
	WCHAR wszVirtualIp[IPLENMAX];	// nexon pcbang
};

struct CSIntendedDisconnect		// CS_SYSTEM / CS_INTENDED_DISCONNECT
{
};

struct CSConnectGame				// CS_SYSTEM / CS_CONNECTGAME
{
	UINT nSessionID;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
};

struct CCPeerConnectRequest		// CS_SYSTEM / CC_PEER_CONNECT_REQUEST
{
	UINT nSessionID[2];
	char cState;			//0 = Connect Request, 1 = Connected, 2 = Disconnected
};

struct SCPeerConnectRequest		// SC_SYSTEM / SC_PEER_CONNECT_REQUEST
{
	//Hey guys, connect with each other~
	ULONG nDestAddrIP;
	USHORT nDestAddrPort;
	UINT nSessionID[2];
};

struct CSReconnectLogin			// CS_SYSTEM / CS_RECONNECTLOGIN
{
	UINT nSessionID;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
};

struct CSAbandonStage			// CS_SYSTEM / CS_ABANDONSTAGE
{
	bool bIsPartyOut;			//true if leaving the party, false if just giving up
	bool bIntenedDisconnect;
};

struct SCReconnectLogin			// SC_SYSTEM / SC_RECONNECTLOGIN
{
	int nRet;
	UINT nAccountDBID;				// Account DBID
	INT64 biCertifyingKey;			// Auth key
};

struct SCCountDownMsg			// SC_SYSTEM / SC_COUNTDOWNMSG
{
	BYTE cIsStart;				// 1:start 0:stop
	int nCountTime;
	UINT nCode;					//	uistringnum to use
	BYTE msgboxType;			//	0:prefix 1:number_only 2:progress
};

#if defined(PRE_ADD_VIP)
struct CSVIPAutomaticPay		// CS_SYSTEM / CS_VIPAUTOMATICPAY, // VIP auto-payment 
{
	bool bAutomaticPay;
};

struct SCVIPAutomaticPay		// SC_SYSTEM / SC_VIPAUTOMATICPAY, // VIP auto-payment 
{
	bool bAutomaticPay;
};

#endif	// #if defined(PRE_ADD_VIP)


struct CSTCPPing
{
	DWORD dwTick;
};

typedef CSTCPPing SCTCPPing;

struct CSClientPing
{
	DWORD dwTick;
};

typedef CSClientPing SCClientPing;


struct SCServiceClose			//SC_SYSTEM / SC_SERVICECLOSE // Service close notifier~
{
	__time64_t _tNow;				//Server sent timestamp
	__time64_t _tOderedTime;		//Timestamp when command was issued to Service Manager (just in case it's needed)
	__time64_t _tCloseTime;			//Scheduled service close time
};

//-------------------------------------------------
//	CHAR (CS_CHAR, SC_CHAR)
//-------------------------------------------------

struct CSLoockUser			// CS_CHAR / CS_LOOKUSER
{
	WCHAR wszCharacterName[NAMELENMAX];
};

struct SCLoockUser			// SC_CHAR / SC_LOOKUSER
{
};

struct SCMapInfo				// SC_CHAR / SC_MAPINFO
{
	int nMapIndex;
	BYTE cMapArrayIndex;
	BYTE cEnviIndex;
	BYTE cEnviArrayIndex;
	int nChannelID;
	short wChannelIdx;
	TMeritInfo MeritInfo;
};

struct SCPvPInfo
{
	TPvPGroup	PvP;
};

struct SCPvPMemberIndex		//SC_PVP_MEMBERINDEX
{
	USHORT nTeam;
	int nRetCode;
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct SCPvPMemberGrade		//SC_PVP_MEMBERGRADE
{
	int nRetCode;
	UINT uiUserState;
	UINT nSessionID;
};

struct SCPvPTryAcquirePoint		//SC_PVP_TRYACQUIREPOINT
{
	int nAreaID;
	int nRetCode;
};

struct SCPvPTryAcquireSkill		//SC_PVP_TRYACQUIRESKILL
{
	int nSkillID;
	int nLevel;
	int nRetCode;
};

struct SCPvPInitSkill			//SC_PVP_INITSKILL
{
	int nRetCode;
};

struct SCPvPUseSkill			//SC_PVP_USESKILL
{
	UINT nSessionID;		//User who used it, feels like it might be needed
	int nSkillID;
	int nRetCode;
};

struct SCPvPSwapSkillIndex		//SC_PVP_SWAPSKILLINDEX
{
	char cFrom;
	char cTo;
	int nRetCode;
};

//#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || defined(_WINDOWS)
#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct SCEnter					// SC_CHAR / SC_ENTER
{
	UINT nSessionID;
	UINT nVehicleObjectID;
	WCHAR wszCharacterName[NAMELENMAX];
	char cAccountLevel;	// Account authority (eAccountLevel)
	BYTE cPartyState;
	BYTE cClassID;
	BYTE cJobArray[JOBMAX];
	BYTE cLevel;
	int	nExp;
	INT64 nCoin;	// Use int64 since this is total held coins
	BYTE cRebirthCoin;
	BYTE cPCBangRebirthCoin;
	USHORT wRebirthCashCoin;
	EtVector3 Position;
	float fRotate;
	USHORT wSkillPoint[DualSkill::Type::MAX];
	int	nDefaultBody;	// Equip set on initial creation
	int	nDefaultLeg;
	int	nDefaultHand;
	int	nDefaultFoot;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	short nAppellation;
	short nCoverAppellation;
	USHORT wFatigue;
	USHORT wWeekFatigue;
	USHORT wPCBangFatigue;
	USHORT wEventFatigue;
	USHORT wMaxEventFatigue;
#if defined(PRE_ADD_VIP)
	bool bVIP;
	BYTE cVIPRebirthCoin;
	USHORT wVIPFatigue;	// Remaining fatigue
	int nVIPPoint;		// Point
	__time64_t tVIPExpirationDate;	// Expiration date
	bool bVIPAutomaticPay;	// Whether auto-payment is enabled
#endif	// #if defined(PRE_ADD_VIP)
	TPvPGroup sPvPInfo;
	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];
	TGuildSelfView GuildSelfView;	// Guild visual info (personal)
	char cCurrentGuildWarEventStep;	// Current guild war step
	short wCurrentGuldWarScheduleID;	// Current Guild War round in progress
	char cSkillPage;					// Currently active skill page
	char cAge;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	int nNamedItemID;
#endif
};

struct SCEnterUser				// SC_CHAR / SC_ENTERUSER
{
	UINT nSessionID;
	UINT nVehicleObjectID;
	WCHAR wszCharacterName[NAMELENMAX];
	char cAccountLevel;	// Account authority
	bool bHide;
	BYTE cPartyState;
	BYTE cClassID;
	BYTE cLevel;
	BYTE cPvPLevel;
	BYTE cJob;
	EtVector3 Position;
	float fRotate;
	bool bBattleMode;
	int	nDefaultBody;	// Equip set on initial creation
	int	nDefaultLeg;
	int	nDefaultHand;
	int	nDefaultFoot;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	short nAppellation;
	short nCoverAppellation;
	TGuildSelfView GuildSelfView;	// Guild visual info (personal)
	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];
#if defined(PRE_ADD_VIP)
	bool bVIP;
#endif	// #if defined(PRE_ADD_VIP)
	bool bIsPartyLeader;
	TVehicle VehicleEquip;
	TVehicle PetEquip;

	char cCount;
	char cCashCount;
	char cGlyphCount;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	char cTalismanCount;
#endif

	int	nSourceItemID;
#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	int nNamedItemID;
#endif
#if defined ( PRE_ADD_BESTFRIEND)
	INT64 biBestFriendItemSerial;
	WCHAR wszBestFriendName[NAMELENMAX];
#endif
#if defined(PRE_ADD_TRANSFORM_POTION)
	int	nTransformID;
#endif //#if defined(PRE_ADD_TRANSFORM_POTION)

#if defined(PRE_ADD_TALISMAN_SYSTEM)
	TItemInfo EquipArray[EQUIPMAX+CASHEQUIPMAX+GLYPHMAX+TALISMAN_MAX];		// This must always be at the very end.
#else
	TItemInfo EquipArray[EQUIPMAX+CASHEQUIPMAX+GLYPHMAX];		// This must always be at the very end.
#endif
};

struct SCEnterNpc				// SC_CHAR / SC_ENTERNPC
{
	UINT nNpcObjectID;
	UINT nNpcID;
	EtVector3 Position;
	float fRotate;
};
#endif

struct SCLeaveUser				// SC_CHAR / SC_LEAVEUSER
{
	UINT nSessionID;
	char cLeaveType;		// eLeaveType
	char cAccountLevel;	// Account authority
};

struct SCLeaveNpc				// SC_CHAR / SC_LEAVENPC
{
	UINT nObjectID;
};

struct SCEnterPropNpc
{
	UINT nNpcObjectID;
	int nPropUniqueID;
};

struct SCLeavePropNpc
{
	int nPropUniqueID;
};

struct TQuickSlotEx: TQuickSlot
{
	BYTE cIndex;
};

struct SCQuickSlotList		// SC_CHAR / SC_QUICKSLOTLIST
{
	BYTE cSlotCount;
	TQuickSlotEx QuickSlot[QUICKSLOTMAX];
};

struct CSAddQuickSlot		// SC_CHAR / SC_ADDQUICKSLOT
{
	BYTE cSlotIndex;		// Quick slot bar index
	BYTE cSlotType;			// See eQuickSlotType
	INT64 nID;				// Target ID (item ID, skill ID, or mount item serial)
};

struct SCAddQuickSlot		// SC_CHAR / SC_ADDQUICKSLOT
{
	BYTE cSlotIndex;		// Quick slot bar index
	BYTE cSlotType;			// See eQuickType
	int nID;				// Target ID (item ID or skill ID)
	int nRet;
};

struct CSDelQuickSlot		// SC_CHAR / CS_DELQUICKSLOT
{
	BYTE cSlotIndex;
};

struct SCDelQuickSlot		// SC_CHAR / SC_DELQUICKSLOT
{
	BYTE cSlotIndex;
	int nRet;
};


struct CSCloseUIWindow
{
	eWindowState WindowState; // eWindowState
};


struct SCFatigue			// SC_CHAR / SC_FATIGUE
{
	UINT iResult;
	UINT nSessionID;
	short wFatigue;		// Current value
	short wWeekFatigue;		// Weekly current value
	short wPCBangFatigue;	// PC cafe current value
	short wEventFatigue;
#if defined(PRE_ADD_VIP)
	short wVIPFatigue;		// Current VIP value
#endif	// #if defined(PRE_ADD_VIP)
};

struct SCHide				// SC_CHAR / SC_HIDE
{
	UINT nSessionID;
	bool bHide;
};

struct TShutdownData
{
	char cType;	// eShutdownType
	short nYear;
	char nMonth;
	char nDay;
	char nHour;
};

struct SCPCBang				// SC_CHAR / SC_PCBANG
{
	char cPCBangGrade;
};

struct SCNestClear			// SC_CHAR / SC_NESTCLEAR
{
	char cCount;
	TNestClearData NestClear[NESTCLEARMAX];
};

struct SCUpdateNestClear	// SC_CHAR / SC_UPDATENESTCLEAR
{
	TNestClearData Update;
};

struct SCMaxLevelCharacterCount
{
	int iCount;
};

struct SCCommonVariableList
{
	BYTE cCount;
	CommonVariable::Data List[CommonVariable::Type::Max];
};

struct SCModCommonVariable
{
	CommonVariable::Data Data;
};

#if defined( PRE_ADD_NOTIFY_ITEM_COMPOUND )
struct CSCompoundNotify
{
	int nItemID;
	int nItemOption;
};
#endif
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
struct CSStageClearBonusRewardSelect			// CS_CHAR / CS_STAGECLEAR_BONUSREWARD_SELECT
{
	UINT nPropID;
	int nItemID;
};

struct SCStageClearBonusRewardSelect			// SC_CHAR / SC_STAGECLEAR_BONUSREWARD_SELECT
{
	int nResult;
};
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
//-------------------------------------------------
//	ACTOR (CS_ACTOR, SC_ACTOR)
//-------------------------------------------------

struct CSActorMessage			// CS_ACTOR / CS_ACTOR_MSG
{
	UINT nSessionID;
	char cBuf[128];
};

struct SCActorMessage			// SC_ACTOR / SC_ACTOR_MSG
{
	UINT nSessionID;
	char cBuf[128];
};

//-------------------------------------------------
//	ACTOR BUNDLE (CS_ACTORBUNDLE, SC_ACTORBUNDLE)
//-------------------------------------------------	
struct SCActorBundleMessage
{
	UINT nSessionID;
	char cBuf[256];
};

struct CSActorBundleMessage
{
	UINT nSessionID;
	char cBuf[256];
};

struct SCActorShadowMessage
{
	UINT nSessionID;
	char cBuf[256];
};


//-------------------------------------------------
//	PROP (CS_PROP, SC_PROP)
//-------------------------------------------------

struct CSPropMessage			// CS_PROP / CS_PROP_MSG
{
	UINT nUniqueID;
	char cBuf[128];
};

struct SCPropMessage			// SC_PROP / SC_PROP_MSG
{
	UINT nUniqueID;
	char cBuf[128];
};


//-------------------------------------------------
//	PARTY (CS_PARTY, SC_PARTY)
//-------------------------------------------------

#ifdef PRE_MOD_SYNCPACKET
const int MAX_SEND_SYNCPLAYER = 4;
#else		//#ifdef PRE_MOD_SYNCPACKET
const int MAX_SEND_SYNCPLAYER = 8;
#endif		//#ifdef PRE_MOD_SYNCPACKET

struct CSCreateParty			// CS_PARTY / CS_CREATEPARTY
{
#if defined( PRE_PARTY_DB )
	ePartyType PartyType;
	int iBitFlag;
	int iPassword;
#else
	BYTE cRaidParty;			//For now, refers to parties of 5 to 8 members (1:true, otherwise:false)
	BYTE cIsJobDice;
	BYTE cUserLvLimitMax;
#endif // #if defined( PRE_PARTY_DB )
	BYTE cPartyMemberMax;

	TPARTYITEMLOOTRULE ItemLootRule;			//Refer to ePartyItemLootRule
	TITEMRANK ItemRank;				//Item rank (refer to loot rule design)
	BYTE cUserLvLimitMin;		//It says there is a min/max value.

	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;

#if defined( PRE_PARTY_DB )
	WCHAR wszPartyName[PARTYNAMELENMAX];
#else
	BYTE cUseVoice;				//zero is nouse otherwise use
	BYTE cNameLen;
	BYTE cPassWordLen;
	WCHAR wszBuf[PARTYNAMELENMAX + PARTYPASSWORDMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct SCCreateParty			// SC_PARTY / SC_CREATEPARTY
{
	int nRetCode;
	TPARTYID PartyID;
	ePartyType PartyType;
	BYTE cIsAutoCreate;			//Auto-generation flag (0: normal generation, 1: auto generation)
};

struct CSDestroyParty			// CS_PARTY / CS_DESTROYPARTY
{
	TPARTYID PartyIndex;
};

struct CSInvitePartyMember			//CS_PARTY / CS_PARTYINVITE
{
	BYTE cInviteMemberSize;
	WCHAR wszBuf[NAMELENMAX];
};

struct CSInviteDenied
{
	TPARTYID PartyIdx;
	BYTE cIsOpenBlind;		//When talking or a window is open
	BYTE cInviterNameLen;
	WCHAR wszBuf[NAMELENMAX];
};

struct CSPartyMemberInfo			//CS_PARTY / CS_PARTYMEMBER_INFO
{
	TPARTYID PartyID;
};

struct CSPartyInviteOverServerResult			//CS_PARTY / CS_PARTYINVITE_OVERSERVER_RESULT
{
	int nGameServerID;				//from
	WCHAR wszInviterName[NAMELENMAX];
	int nRetCode;					//0 invite agreed otherwise denied
};

struct CSPartyMemberKick			//CS_PARTY / CS_PARTYMEMBER_KICK
{
	UINT nSessionID;
	char cKickKind;						//0: Normal, 1: Permanent
};

struct CSPartyLeaderSwap			//CS_PARTY / CS_PARTYLEADER_SWAP
{
	UINT nSessionID;
};

struct CSJoinGetReversionItem			//CS_PARTY / CS_JOINGETREVERSEITEM
{
	bool bJoin;
};

struct CSMemberAbsent			//CS_PARTY / CS_ABSENT
{
	BYTE cAbsentFlag;			// 0 absent, 1 present
};

struct CSVerifyDungeon			//CS_PARTY / CS_VERIFYTARGETDUNGEON
{
	int nMapIdx;
	BYTE cDifficulty;
};

struct CSPartyAskJoin			//CS_PARTY / CS_PARTYASKJOIN
{
	WCHAR wszPartyLeaderName[NAMELENMAX];
};

struct CSPartyAskJoinDecision			//CS_PARTY / CS_PARTYASKJOINDECISION
{
	BYTE cRet;								//0 denied, 1 agree
	WCHAR wszPartyAsker[NAMELENMAX];		//Target recipient
};

struct CSPartySwapMemberIndex		//CS_PARTY / CS_SWAPMEMBERINDEX
{
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct SCPartySwapMemberIndex		//SC_SWAPMEMBERINDEX
{
	int nRetCode;
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct CSPartyWorldZoneList
{
	bool bFlag;
};

struct SCPartyWorldZoneList
{
	int iRet;
};

struct CSVillageSelectChannel			//CS_PARTY / CS_SELECTCHANNEL
{
	int nChannelID;	
};

struct CSPartyInfoModify					//CS_PARTY / CS_PARTYMODIFY
{
	BYTE cPartyMemberMax;
	TPARTYITEMLOOTRULE LootRule;
	TITEMRANK LootRank;
	BYTE cUserLvLimitMin;
	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;
	WCHAR wszName[PARTYNAMELENMAX];
#if defined( PRE_PARTY_DB )
	int iBitFlag;
	int iPassword;
#else
	BYTE cUserLvLimitMax;
	BYTE cIsJobDice;
	WCHAR wszPass[PARTYPASSWORDMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct CSSelectDunGeon				//CS_PARTY / CS_SELECTDUNGEON
{
	BYTE cDiffyculty;
	UINT nMapIndex;
	//short nCanEnterCode; // Can enter if ERROR_NONE; otherwise context-dependent value (see eError)
};

struct CSJoinParty				// CS_PARTY / CS_JOINPARTY
{
	TPARTYID PartyID;
#if defined( PRE_PARTY_DB )
	int iPassword;
#else
	BYTE cPartyPasswordSize;
	WCHAR wszBuf[PARTYPASSWORDMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct CSPartyListInfo				// CS_PARTY / CS_PARTYLISTINFO
{
#if defined( PRE_PARTY_DB )
	USHORT unReqPage;
	WCHAR wszSearchWord[Party::Max::SearchWordLen];
	Party::OrderType::eCode OrderType;
	Party::StageOrderType::eCode StageOrderType;
	bool bOnlySameVillage;
#if defined( PRE_FIX_61545 )
	WCHAR cSeperator;
#endif // #if defined( PRE_FIX_61545 )

	bool operator==(const CSPartyListInfo& rhs) const
	{
		if( this->unReqPage == rhs.unReqPage && this->OrderType == rhs.OrderType && this->StageOrderType == rhs.StageOrderType && this->bOnlySameVillage == rhs.bOnlySameVillage && wcscmp(this->wszSearchWord,rhs.wszSearchWord) == 0 )
			return true;
		return false;
	}

	void Clear()
	{
		this->OrderType = Party::OrderType::Max;
	}
#else
	BYTE cOffSetCnt;		//Count to receive at once
	BYTE cGetPage;		//Which page to receive using the count above as offset
	TDUNGEONDIFFICULTY Difficulty;
	BYTE cIsRefresh;
	int nSortMapIdx[PARTYSORTCONDITIONMAX];
#endif // #if defined( PRE_PARTY_DB )
};

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
struct CSPartyInfo						// CS_PARTY / CS_PARTYINFO
{
	TPARTYID PartyID;
	WCHAR wszCharName[NAMELENMAX];
};
#endif

struct SCJoinParty				// SC_PARTY / SC_JOINPARTY
{
	int nRetCode;
	TPARTYID PartyID;
	WCHAR wszPartyName[PARTYNAMELENMAX];
};

struct SCRefreshParty			// SC_PARTY / SC_REFRESHPARTY
{
	UINT nLeaderSessionID;
	BYTE cCurrentUserCount;
	BYTE cPartyMemberMax;
	BYTE cUserLvLimitMin;
	TPARTYITEMLOOTRULE ItemLootRule;
	TITEMRANK ItemLootRank;
	BYTE cVoiceChatAvailable;
	int nTargetMapIdx;
	TDUNGEONDIFFICULTY Difficulty;
#if defined( PRE_PARTY_DB )
	int iBitFlag;
	ePartyType PartyType;
	int iPassword;
#else
	BYTE cUserLvLimitMax;
	BYTE cIsJobDice;
	BYTE cPartyType;							//ePartyType
	WCHAR wszPass[PARTYPASSWORDMAX];
#endif // #if defined( PRE_PARTY_DB )
	BYTE cPartyRefreshSubJect;					//See ePartyRefreshSubject
#if defined( PRE_WORLDCOMBINE_PARTY )
	int nWorldCombinePartyTableIndex;
#endif
	WCHAR wszName[PARTYNAMELENMAX];
	SPartyMemberInfo Member[PARTYMAX];
};

struct SCPartyOut				// SC_PARTY / SC_PARTYOUT
{
	UINT nSessionID;		//Who left
	int nRetCode;	// Distinguish success and fail
	char cKickKind;	//-1: left (not kicked), 0: normal kick, 1: permanent kick
};

//struct SCDelPartyMember			// SC_PARTY / SC_DELPARTYMEMBER
//{
//	UINT nMemberUID;
//	BYTE cIsUnintenedDelMember;
//};

struct SPartyListInfo
{
	TPARTYID PartyID;
	BYTE cUserLvLimitMin;
#if defined( PRE_PARTY_DB )
	int iCurMapIndex;
	bool bIsBonus;
#if defined( _FINAL_BUILD )
#else
	INT64 biSortPoint;
#endif // #if defined( _FINAL_BUILD )
#else
	BYTE cUserLvLimitMax;
	USHORT nChannelMaxUser;
	USHORT nChannelCurUser;
#endif // #if defined( PRE_PARTY_DB )
	BYTE cCurrentCount;
	BYTE cMaxCount;
	BYTE cIsPrivateRoom;
	int nTargetMapIdx;
	TDUNGEONDIFFICULTY TargetDifficulty;
#if defined( PRE_PARTY_DB )
#else
	USHORT wMasterFatigue;
#endif // #if defined( PRE_PARTY_DB )
	BYTE cUseVoice;
	ePartyType PartyType;				//ePartyType
	BYTE cPartyNameLen;
	WCHAR wszBuf[PARTYNAMELENMAX];
#if defined( PRE_WORLDCOMBINE_PARTY )
	int nWorldCombinePartyTableIndex;		// World party index
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComeBackParty;
#endif
};

struct SCPartyListInfo				// SC_PARTY / SC_PARTYLISTINFO
{
	int nRetCode;
#if defined( PRE_PARTY_DB )
	USHORT unReqPage;
	int iTotalCount;
	int iListCount;
	SPartyListInfo PartyListInfo[PARTYLISTOFFSET*2];	// Send 2 pages at a time to reduce paging cost.
#else
	USHORT nWholeCount;
	BYTE cCount;
	SPartyListInfo PartyListInfo[PARTYLISTOFFSET*2];
#endif // #if defined( PRE_PARTY_DB )
};

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
struct SCPartyInfo				// SC_PARTY / SC_PARTYINFO
{
	int nRetCode;
	SPartyListInfo PartyInfo;	
};
#endif


//#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || defined(_WINDOWS)
#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct CSRefreshGateInfo		// CS_PARTY / CS_REFRESHGATEINFO
{
	EtVector3 Position;
	bool boEnter;
};
#endif

struct SCRefreshGateInfo		// SC_PARTY / SC_REFRESHGATEINFO
{
	int nRet;
	BYTE cCount;
	char GateNo[PARTYMAX];
};

struct SCGateInfo				// SC_PARTY / SC_GATEINFO
{
	UINT nLeaderUID;
	char cGateNo;
	char cMapCount;
	char cPermitFlag[5];
	char cCanEnter[5][5];
};

struct TPartyMemberDefaultParts
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	int iDefaultPartsIndex[4];
};

struct TPartyMemberWeaponOrder
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];
};

struct TPartyMemberEquip
{
	char cCount;
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TItemInfo EquipArray[EQUIPMAX];
};

struct TPartyMemberCashEquip
{
	char cCount;
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TItemInfo EquipArray[CASHEQUIPMAX];
};

struct TPartyMemberSkill
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	char cCount;
	TSkill SkillArray[SKILLMAX];
};

struct TPartyMemberGlyphEquip
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	char cCount;
	TItemInfo EquipArray[GLYPHMAX];
};


struct CSStartStage
{

	bool bReturnVillage;
	TDUNGEONDIFFICULTY Difficulty;
	char cSelectDungeonIndex;
	UINT nFarmDBID;
	char cSelectMapIndex;
};


#define SCSelectDunGeon CSSelectDunGeon 		// SC_PARTY //SC_SELECTDUNGEON

struct SCPartyInfoModify			// SC_PARTY / SC_PARTYMODIFY
{
	int nRet;
};

struct SCPartyQuickJoinFail		// SC_PARTY / SC_PARTYQUICKJOINFAIL
{
	int nRet;
};

struct SCVerifyJoinParty			// SC_PARTY / SC_VERIFYJOINPARTY
{
	int nPartyIdx;
	int nChannelID;
	int nRet;
};

struct SCDefaultPartsData	// SC_PARTY //SC_DEFAULTPARTSDATA
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberDefaultParts DefaultParts[MAX_SEND_SYNCPLAYER];
};

struct SCWeaponOrderData
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberWeaponOrder WeaponOrder[MAX_SEND_SYNCPLAYER];
};

struct SCEquipData				// SC_PARTY //SC_EQUIPDATA
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberEquip MemberEquip[MAX_SEND_SYNCPLAYER];
};

struct SCCashEquipData
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberCashEquip MemberEquip[MAX_SEND_SYNCPLAYER];
};

struct SCGlyphEquipData
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberGlyphEquip MemberEquip[MAX_SEND_SYNCPLAYER];
};


struct SCVehicleEquipData
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TVehicleCompact MemberVehicle[MAX_SEND_SYNCPLAYER];
};

struct SCSkillData				// SC_PARTY //SC_SKILLDATA
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyMemberSkill MemberSkill[MAX_SEND_SYNCPLAYER];
};

struct SCMoveChannelFail		// SC_PARTY / SC_MOVECHANNELFAIL
{
	int nRetCode;
};

struct SCPartyInfoErr			// SC_PARTY / SC_PARTYINFO_ERROR
{
	int nRetCode;
};

struct SCStartStage				// SC_PARTY / SC_STARTSTAGE
{
#if defined(PRE_FIX_RANDOM_MAPINDEX)
	int nMapIndex;
#endif	// #if defined(PRE_FIX_RANDOM_MAPINDEX)
	int nRandomSeed;
	TDUNGEONDIFFICULTY Difficulty;
	short nExtendDropRate;			//Additional drop rate for event
};
	
struct SCStartStageDirect
{
	int nMapIndex;
	char cGateNo;
	int nRandomSeed;
	TDUNGEONDIFFICULTY Difficulty;
	short nExtendDropRate;			//Additional drop rate for event
};

struct SCCancelStage
{
	int nRetCode;
};

struct CSSelectStage
{
	char cSelectMapIndex;
};		

struct SCSelectStage
{
	char cSelectMapIndex;
};

struct SCPartyChangeLeader		// SC_PARTY  / SC_CHANGEPARTYLEADER
{
	UINT nNewLeaderSessionID;
};

struct SCPartyInvite				// SC_PARTY  / SC_PARTYINVITE
{
	TPARTYID PartyID;
	USHORT nPartyNameLen;
	BYTE cPartyInviterNameLen;
	BYTE cMaxUserCount;
	BYTE cCurUserCount;
	USHORT nAvrLevel;
	int nGameServerID;					//
	int nGameMapIdx;					//If this is greater than 0, invited from the game server
#if defined( PRE_PARTY_DB )
	int iPassword;
	WCHAR wszBuf[PARTYNAMELENMAX+NAMELENMAX];
#else
	BYTE cPartyPassLen;
	WCHAR wszBuf[PARTYNAMELENMAX + PARTYPASSWORDMAX + NAMELENMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct SCPartyInviteNotice		//SC_PARTY / SC_PARTYINVITENOTICE
{
	int nRetCode;
	WCHAR wszInvitedCharacterName[NAMELENMAX];		//Inviter's character name
};

struct SCPartyInviteFail			//SC_PARTY / SC_PARTYINVITEFAIL
{
	int nRetCode;
};

struct SCPartyAskJoinToLeader				//SC_PARTY / SC_ASKJOINPARTYTOLEADER
{
	char cJob;
	BYTE cLevel;
	WCHAR wszAskChracterName[NAMELENMAX];
};

struct SCPartyAskJoinResult			//SC_PARTY / SC_ASKJOINPARTYRESULT
{
	int nRet;
};

struct SCPartyAskJoinDecisionInfo		//SC_PARTY / SC_ASKJOINPARTYDECISIONINFO
{
	TPARTYID PartyID;
#if defined( PRE_PARTY_DB )
	int iPassword;
#else
	WCHAR wszPartyPassword[PARTYPASSWORDMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct SCPartyUpdateFieldUI				//SC_PARTY / SC_PARTYUPDATEFIELDUI
{
	UINT nSessionID;
	BYTE cLeader;						//0 none, 1 leader
};

//struct SCPartySwapMemberIndex			//SC_PARTY / SC_SWAPMEMBERINDEX
//{
//	BYTE cOriginIndex;
//	BYTE cSwapIndex;
//	int nRetCode;
//};

struct TPartyMemberInfo
{
	BYTE cUserLevel;
	USHORT wJob;
	BYTE cClassID;
	WCHAR wszName[NAMELENMAX];
};

struct SCPartyMemberInfo		//SC_PARTY / SC_PARTYMEMBER_INFO
{
#if defined( PRE_PARTY_DB )
	TPARTYID PartyID;
	ePartyType PartyType;
	short nCount;
	Party::MemberData MemberData[PARTYCOUNTMAX];
#else
	TPARTYID PartyID;
	BYTE cLeaderIdx;
	BYTE cIsRaidParty;
	BYTE cCount;
	TPartyMemberInfo Info[PARTYMAX];
#endif // #if defined( PRE_PARTY_DB )
};

struct SCPartyInviteDenied		//SC_PARTY / SC_PARTYINVITEDENIED
{
	int nRetCode;
	WCHAR wszCharName[NAMELENMAX];
};

struct SCPartyKickedMember	//SC_PARTY / SC_PARTYMEMBERKICKED
{
	UINT nSessionID;
	int nRetCode;
};

struct SCSwapPartyLeader		//SC_PARTY / SC_PARTYLEADERSWAP
{
	UINT nSessionID;
	int nRetCode;
};

#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct SCPartyMemberMove		// SC_PARTY / SC_PARTYMEMBER_MOVE
{
	UINT nSessionID;
	EtVector3 Position;
};
#endif

struct SCPartyMemberPart		// SC_PARTY / SC_PARTYMEMBER_PART
{
	UINT nSessionID;
	int nExp;
	BYTE cLevel;
	BYTE cJobArray[JOBMAX];
	int	nHairID;
	int	nFaceID;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	USHORT wFatigue;
	int nHelmetID;
	int	nEarringID;
};

struct TPartyEtcData
{
#ifdef PRE_MOD_SYNCPACKET
	UINT nSessionID;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	int		nSelectAppellation;
	short	nCoverAppellation;
	BYTE	cGMTrace;
#if defined(PRE_ADD_VIP)
	bool	bVIP;
#endif	// #if defined(PRE_ADD_VIP)
	char	cAccountLevel; // Account authority (eAccountLevel)
};

struct SCEtcData
{
	BYTE	cCount;
#ifdef PRE_MOD_SYNCPACKET
#else		//#ifdef PRE_MOD_SYNCPACKET
	USHORT	unStartIndex;
#endif		//#ifdef PRE_MOD_SYNCPACKET
	TPartyEtcData EtcData[MAX_SEND_SYNCPLAYER];
};

// SC_PARTY / SC_GUILDDATA
struct TPartyMemberGuild
{
public:
	struct TPartyMemberGuildView : public TGuildSelfView
	{
	public:
		TPartyMemberGuildView()
		{
			Reset();
		}
		void Set(UINT nMemberSessionID, const TGuildSelfView& pGuildSelfView)
		{
			nSessionID = nMemberSessionID;
			TGuildSelfView::Set(pGuildSelfView);
		}
		void Reset()
		{
			nSessionID = 0;
			TGuildSelfView::Reset();
		}
		bool IsSet() {
			return((0 != nSessionID) && TGuildSelfView::IsSet());
		}
	public:
		UINT nSessionID;
	};
public:
	TPartyMemberGuild()
	{
		Reset();
	}
	void Set()
	{
		nCount = 0;
	}
	void Reset()
	{
		nCount = 0;
		unStartIndex = 0;
	}	
	bool Add(UINT nSessionID, const TGuildSelfView& pGuildSelfView)
	{
		if (PARTYMAX <= nCount) {
			return false;
		}

		List[nCount].Set(nSessionID, pGuildSelfView);
		++nCount;

		return true;
	}

	const TPartyMemberGuildView* At(int iIndex) const
	{
		if (0 > iIndex || _countof(List) <= iIndex)
			return NULL;
		
		return(&List[iIndex]);
	}
public:
	int nCount;	// Party member guild visualization info list count
	USHORT unStartIndex;
	TPartyMemberGuildView List[MAX_SEND_SYNCPLAYER];	// Party member guild visualization info list
};

struct SCRollDiceForGetReverseItem
{
	UINT nSessionID;
	bool bRoll;
	int	nDiceValue;
};

struct SCResultGetReverseItem
{
	UINT nSessionID;
	TItem itemInfo;
	DWORD dropItemUniqueId;
};

struct SCPartyJoinGetReversionItemInfo
{
	TItem itemInfo;
	DWORD dropItemUniqueId;
};

struct SCMemberAbsent		//SC_PARTY / SC_ABSENT
{
	UINT nSessionID;
	BYTE cAbsentFlag;		//0 absent, 1 present
};

struct SCVerifyDungeon		//SC_PARTY / SC_VERIFYTARGETDUNGEON
{	
	BYTE cVerifyFlag[5];		//Difficulty increases sequentially from 0 // 0 not allowed, 1 allowed
};

struct SCPartyBonusValue	//SC_PARTY / SC_PARTYBONUSVALUE
{
	BYTE cUpkeepBonus;		//% unit
	BYTE cFriendBonus;		//% unit
	BYTE cUpKeepBonusLimit;	//Maintenance bonus max (Jiho, if this is >= 0, display as full~) 
	//Even when full, sends 0 normally and evaluates only at clear time to send >=0. Display effect when receiving non-zero.
#ifdef PRE_ADD_BEGINNERGUILD
	BYTE cBeginnerGuildBonus;	//When the party consists of beginner guild members
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
};

struct SCPartyCheckLastDungeonInfo
{
	WCHAR wszPartyName[PARTYNAMELENMAX];
};

struct CSPartyConfirmLastDungeonInfo
{
	bool bIsOK;
};

struct SCPartyConfirmLastDungeonInfo
{
	int iRet;
};

#if defined (PRE_ADD_BESTFRIEND)
struct TPartyBestFriend
{
	int nSessionID;
	INT64 biBFItemSerial;
	WCHAR wszBFName[NAMELENMAX];
};

struct SCPartyBestFriend
{
	int nCount;
	TPartyBestFriend List[PARTYMAX];
};
#endif

//-------------------------------------------------
//	ITEM (CS_ITEM, SC_ITEM)
//-------------------------------------------------

struct SCEquipList
{
	char cCount;
	char cCashCount;
	char cGlyphCount;
#if defined(PRE_ADD_TALISMAN_SYSTEM)
	char cTalismanCount;
	int nTalismanOpenFlag;
	TItemInfo EquipArray[EQUIPMAX+CASHEQUIPMAX+GLYPHMAX+TALISMAN_MAX];
#else
	TItemInfo EquipArray[EQUIPMAX+CASHEQUIPMAX+GLYPHMAX];
#endif
};

struct SCInvenList					// SC_ITEM / SC_INVENLIST
{
	BYTE cInvenMaxCount;	// How many slots are unlocked (normal inventory only, excluding timed slots)
	BYTE cInvenCount;
#if defined(PRE_PERIOD_INVENTORY)
	TItemInfo Inven[INVENTORYTOTALMAX];
#else	// #if defined(PRE_PERIOD_INVENTORY)
	TItemInfo Inven[INVENTORYMAX];
#endif	// #if defined(PRE_PERIOD_INVENTORY)
};

struct SCCashInvenList
{
	int nTotalCashInventoryCount;
	BYTE cInvenCount;
	TItem Inven[CASHINVENTORYMAX];
};

struct SCBuyedItemCount
{
	short count;
	ItemCount items[INVENTORYMAX + WAREHOUSEMAX];
};

// Used in api_trigger_InvenToEquip trigger, so if this struct changes, that must be changed together!!!
struct CSMoveItem				// CS_ITEM / CS_MOVEITEM,
{
	char cMoveType;	// See eItemMoveType (MoveType_EquipToInven, MoveType_InvenToEquip)
	BYTE cSrcIndex;
	INT64 biSrcItemSerial;
	BYTE cDestIndex;
	short wCount;
};

struct SCMoveItem				// SC_ITEM / SC_MOVEITEM,
{
	char cMoveType;	// See eItemMoveType (MoveType_EquipToInven, MoveType_InvenToEquip...)
	int nRetCode;
	TItemInfo SrcItem;
	TItemInfo DestItem;
};

// Used in api_trigger_UnRideVehicle trigger, so if this struct changes, that must be changed together!!!
struct CSMoveCashItem			// CS_ITEM / CS_MOVECASHITEM,
{
	char cMoveType;	// See eItemMoveType (MoveType_CashEquipToCashInven, MoveType_CashInvenToCashEquip)
	BYTE cCashEquipIndex;
	INT64 biEquipItemSerial;
	INT64 biCashInvenSerial;
	short wCount;
};

struct SCMoveCashItem			// SC_ITEM / SC_MOVECASHITEM,
{
	char cMoveType;	// See eItemMoveType (MoveType_CashEquipToCashInven, MoveType_CashInvenToCashEquip)
	int nRetCode;
	TItemInfo CashEquipItem;
	TItem CashInvenItem;
};

#if defined(PRE_ADD_SERVER_WAREHOUSE)
struct SCServerWareList			// SC_ITEM / SC_SERVERWARE_LIST
{
	BYTE cWareCount;
	TItem WareItems[WAREHOUSEMAX];
};

struct SCCashServerWareList			// SC_ITEM / SC_CASHSERVERWARE_LIST
{
	BYTE cWareCount;
	TItem WareCashItems[WAREHOUSEMAX];
};

struct CSMoveServerWare			// CS_ITEM / CS_MOVE_SERVERWARE
{
	char cMoveType;	// See eItemMoveType (MoveType_InvenToServerWare, MoveType_ServerWareToInven)
	BYTE cInvenIndex;
	INT64 biInvenItemSerial;
	INT64 biWareItemSerial;	
	short wCount;
};

struct SCMoveServerWare			// SC_ITEM / SC_MOVE_SERVERWARE
{
	char cMoveType;	// See eItemMoveType (MoveType_CashEquipToCashInven, MoveType_CashInvenToCashEquip)
	int nRetCode;
	TItemInfo InvenItem;
	TItem ServerWareItem;
};

struct CSMoveCashServerWare		// CS_ITEM / CS_MOVE_CASHSERVERWARE
{
	char cMoveType;	// See eItemMoveType (MoveType_CashToServerWare, MoveType_ServerWareToCash)
	INT64 biItemSerial;		
};

struct SCMoveCashServerWare			// SC_ITEM / SC_MOVE_CASHSERVERWARE
{
	char cMoveType;	// See eItemMoveType (MoveType_CashToServerWare, MoveType_ServerWareToCash)
	int nRetCode;
	INT64 biSrcItemSerial;
	TItem SrcItem;
	TItem DestItem;
};
#endif //#if defined(PRE_ADD_SERVER_WAREHOUSE)

struct CSMoveGuildItem			// CS_ITEM / CS_MOVEGUILDITEM,
{
	char	cMoveType;				// See eItemMoveType (MoveType_InvenToGuildWare, MoveType_GuildWareToInven)
	BYTE	cDestIndex;				// Destination index (Guild storage <-> Inventory)
	INT64	biDestItemSerial;		// Serial of item to receive the event
	BYTE	cSrcIndex;				// Source index (guild storage <-> inventory)
	INT64	biSrcItemSerial;		// Item where event occurred
	short	wCount;					// Target count
};

struct SCMoveGuildItem			// SC_ITEM / SC_MOVEGUILDITEM,
{
	char cMoveType;	// See eItemMoveType (MoveType_InvenToGuildWare, MoveType_GuildWareToInven)

	int nRetCode;
	int nTakeItemCount;

	INT64	biSrcSerial;
	INT64   biDestSerial;

	TItemInfo DestItem;
	TItemInfo SrcItem;
};

struct SCRefreshGuildItem		// SC_ITEM / SC_REFRESH_GUILDITEM
{
	char cMoveType;	// See eItemMoveType (MoveType_InvenToGuildWare, MoveType_GuildWareToInven)

	INT64	biSrcSerial;
	INT64   biDestSerial;

	TItemInfo SrcItem;
	TItemInfo DestItem;	
};

struct SCRefreshGuildCoin	 // SC_ITEM / SC_REFRESH_GUILDCOIN
{
	INT64 biTotalCoin;		// Total amount
};

struct CSMoveGuildCoin			// CS_ITEM / CS_MOVEGUILDCOIN
{
	char	cMoveType;			// See eItemMoveType (MoveType_InvenToGuildWare, MoveType_GuildWareToInven)
	INT64	nMoveCoin;
};

struct SCMoveGuildCoin			// SC_ITEM / SC_MOVEGUILDCOIN
{
	int nRet;
	char cMoveType;				// See eItemMoveType (MoveType_InvenToGuildWare, MoveType_GuildWareToInven)
	INT64 nMoveCoin;
	INT64 nInventoryCoin;
	INT64 nWarehouseCoin;
	INT64 nWithdrawCoin;
};

struct CSPickUp					// CS_ITEM / CS_PICKUP,
{
	int nItemID;
	short wCount;
};

struct SCPickUp					// SC_ITEM / SC_PICKUP,
{
	int nRetCode;
	short wCount;
	TItemInfo PickUpItem;
};

struct CSRemoveItem				// CS_ITEM / CS_REMOVEITEM,
{
	char cType;				// eItemPositionType 1: Equip 2: Inven 3: Ware
	BYTE cSlotIndex;
	short wCount;
	INT64 biItemSerial;
};

struct SCRemoveItem				// SC_ITEM / SC_REMOVEITEM,
{
	int nRetCode;
	char cType;				// eItemPositionType 1: Equip 2: Inven 3: Ware
	TItemInfo ItemInfo;
};

struct SCChangeEquip			// SC_ITEM / SC_CHANGEEQUIP
{
	UINT nSessionID;
	TItemInfo ItemInfo;
};

struct SCChangeCashEquip			// SC_ITEM / SC_CHANGECASHEQUIP
{
	UINT nSessionID;
	TItemInfo ItemInfo;
};

struct SCRefreshInven			// SC_ITEM / SC_REFRESHINVEN
{
	bool bNewSign;	// Whether to show the 'new' badge or not
	TItemInfo ItemInfo;
};

struct SCRefreshCashInven		// SC_ITEM / SC_REFRESHINVEN
{
	bool bNewSign;	// Whether to show the 'new' badge or not
	int nCount;
	TItem ItemList[CASHINVENTORYMAX];
};

struct SCRefreshEquip			// SC_ITEM / SC_REFRESHEQUIP
{
	TItemInfo ItemInfo;
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	bool bRefreshState;
#endif
};

struct SCRefreshWarehouse			// SC_ITEM / SC_REFRESHWAREHOUSE
{
	TItemInfo ItemInfo;
};
struct SCCreateDropItem			// SC_ITEM / SC_CREATE_DROPITEM
{
	UINT nSessionID;
	float fpos[3];
	UINT nItemID;
	UINT nRandomSeed;
	char cOption;
	USHORT nCount;
	USHORT nRotate;
	UINT nOwnerSessionID;
};

struct SCCreateDropItemList
{
	int nCount;
	SCCreateDropItem sDropItem[20];	//	Maximum number of DropItems that can be sent at once
};

struct SCDropItem				// SC_ITEM / SC_DROPITEM_MSG
{
	UINT nSessionID;
	USHORT nItemProtocol;
	char cBuf[16];
};

struct SCWarehouseList			// SC_ITEM / SC_WAREHOUSELIST
{
	INT64 nWarehouseCoin;
	BYTE cWareMaxCount;	// How many storage slots can be opened (normal storage only, excluding timed slots)
	BYTE cWareCount;
#if defined(PRE_PERIOD_INVENTORY)
	TItemInfo Warehouse[WAREHOUSETOTALMAX];
#else	// #if defined(PRE_PERIOD_INVENTORY)
	TItemInfo Warehouse[WAREHOUSEMAX];
#endif	// #if defined(PRE_PERIOD_INVENTORY)
};

struct CSUseItem				// CS_ITEM / CS_USEITEM
{
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
};

struct SCUseItem				// SC_ITEM / SC_USEITEM
{
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
};

struct CSCompleteRandomItem
{
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
};

// Will probably need to add per-NPC list display later..
// Temporarily putting CompoundTableID for now
struct CSEmblemCompoundOpenReq	// CS_ITEM / CS_EMBLEM_COMPOUND_OPEN_REQ
{
	BYTE cPlateItemSlotIndex;
	INT64 biPlateItemSerialID;

	int nCompoundTableID;
};

struct SCEmblemCompundOpenRes	// CS_ITEM / SC_EMBLEM_COMPOUND_OPEN_RES
{
	int nResult;
};

struct CSItemCompoundOpenReq	// CS_ITEM / CS_ITEM_COMPOUND_OPEN_REQ
{
	int nCompoundTableID;
	INT64 biNeedItemSerialID;
};

struct SCItemCompoundOpenRes	// CS_ITEM / CS_ITEM_COMPOUND_OPEN_RES
{
	int nResult;
};

struct CSCompoundEmblemReq
{
	BYTE cPlateItemSlotIndex;
	INT64 biPlateItemSerialID;

	int nCompoundTableID;
};

struct CSCompoundItemReq		// CS_ITEM / CS_COMPOUND_EMBLEM_REQ
{
	int nCompoundTableID;
	INT64 biNeedItemSerialID;
};

struct SCCompoundEmblemRes		// CS_ITEM / CS_COMPOUND_EMBLEM_RES
{
	short sResult;
	int nResultItemID;
};

struct SCCompoundItemRes		// SC_ITEM / SC_ITEM_COMPOUND_RES
{
	short sResult;
	int nResultItemID;
	char cLevel;
	char cOption;
};

struct CSEmblemCompoundCancelReq	// CS_ITEM / CS_EMBLEM_COMPOUND_CANCEL_REQ
{
	int nCompoundTableID;
};

struct SCEmblemCompoundCancelRes	// CS_ITEM / CS_EMBLEM_COMPOUND_CANCEL_RES
{
	int nResult;
};

struct CSItemCompoundCancelReq		// CS_ITEM / CS_ITEM_COMPOUND_CANCEL_REQ
{
	int nCompoundTableID;
};

struct SCItemCompoundCancelRes		// CS_ITEM / CS_ITEM_COMPOUND_CANCEL_RES
{
	int nResult;
};

struct CSItemDisjointReq		// CS_ITEM / CS_DISJOINT_REQ 
{
	UINT nNpcUniqueID;			// Must be processed on game server and NPC position is needed, so also send UniqueID.
	BYTE cSlotIndex;
	INT64 biItemSerial;
	short nCount;
};

struct SCItemDisjointRes		// SC_ITEM / SC_DISJOINT_RES
{
	int nItemID;
	bool bSuccess;
};

struct SCInventoryMaxCount		// SC_ITEM / SC_INVENTORYMAXCOUNT
{
	BYTE cInvenMaxCount;	// How many slots are unlocked
};

struct SCGlyphExtendCount
{
	BYTE cCount;
};
struct SCGlyphExpireData
{
	BYTE cCount;
	TCashGlyphData TGlyphData[CASHGLYPHSLOTMAX];
};

struct SCWarehouseMaxCount		// SC_ITEM / SC_WAREHOUSEMAXCOUNT
{
	BYTE cWareMaxCount;	// How many storage slots can be opened (max value)
};

struct SCQuestInvenList			// SC_ITEM / SC_QUESTINVENLIST
{
	char cQuestInvenCount;
	TItemInfo QuestInventory[QUESTINVENTORYMAX];
};

struct SCRefreshQuestInven		// SC_ITEM / SC_REFRESHQUESTINVEN
{
	TItemInfo ItemInfo;
};

struct SCRebirthCoin			// SC_ITEM / SC_REBIRTH_COIN
{
	int nUsableRebirthCoin;		// Remaining coins available for dungeon
	BYTE cRebirthCoin;			// Remaining resurrection coin count
	USHORT wRebirthCashCoin;	// Remaining count of cash revival coins
	int nRet;						// Whether coin usage succeeded
	int nRebirthType;
	UINT nSessionID;
	BYTE cPCBangRebirthCoin;			// Remaining resurrection coin count
#if defined(PRE_ADD_VIP)
	BYTE cVIPRebirthCoin;
#endif	// #if defined(PRE_ADD_VIP)
};

struct SCRefreshRebirthCoin		// SC_PARTY / SC_REFRESHREBIRTHCOIN
{
	UINT nSessionID;
	BYTE cRebirthCoin;
	BYTE cPCBangRebirthCoin;
	short nCashRebirthCoin;
#if defined(PRE_ADD_VIP)
	BYTE cVIPRebirthCoin;
#endif	// #if defined(PRE_ADD_VIP)
};

#if defined(PRE_ADD_REBIRTH_EVENT)
struct SCRebirthMaxCoin		// SC_CHAR / SC_REBIRTH_MAXCOIN
{
	int nRebirthMaxCoin;
};
#endif

struct SCRepairResult			// SC_ITEM / SC_REPAIR_EQUIP, SC_REPAIR_ALL
{
	int nRet;
};

struct CSMoveCoin				// CS_ITEM / CS_MOVEITEM
{
	char cMoveType;	// See eItemMoveType (MoveType_EquipToInven, MoveType_InvenToEquip)
	INT64 nMoveCoin;
};

struct SCMoveCoin				// SC_ITEM / SC_MOVECOIN
{
	char cMoveType;	// See eItemMoveType (MoveType_EquipToInven, MoveType_InvenToEquip)
	INT64 nMoveCoin;
	INT64 nInventoryCoin;
	INT64 nWarehouseCoin;
	int nRet;
};

struct SCRequestRandomItem
{
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	int nRet;
};

struct SCCompleteRandomItem
{
	char cInvenType;			// eItemPositionType
	char cLevel;
	int nResultItemID;
	int nItemCount;
	int nRet;
};

struct SCChangeEternityItem		// SC_ITEM / SC_ETERNITYITEM
{
	UINT nSessionID;
	char cType;
	USHORT wEternityItem;
};

struct SCGlyphTimeInfo			// SC_ITEM / SC_GLYPHTIMEINFO
{
	int nDelayTime;			// glyph-related cooltime
	int nRemainTime;
};

struct TSortSlot
{
	BYTE cCurrent;
	BYTE cNew;
};

struct CSSortInventory			// CS_ITEM / CS_SORTINVENTORY, CS_SORTPERIODINVENTORY 
{
	BYTE cTotalCount;
	TSortSlot SlotInfo[INVENTORYMAX];
};

struct SCSortInventory			// SC_ITEM / SC_SORTINVENTORY, SC_SORTPERIODINVENTORY
{
	int nRet;
};

struct SCDecreaseDurabilityInventory //SC_ITEM / SC_DECREASE_DURABILITY_INVENTORY
{
	char cType;
	union {
		int nAbsolute;
		float fRatio;
	};
};

struct CSSortWarehouse			// CS_ITEM / CS_SORTWAREHOUSE, CS_SORTPERIODWAREHOUSE 
{
	BYTE cTotalCount;
	TSortSlot SlotInfo[WAREHOUSEMAX];
};

struct SCSortWarehouse			// SC_ITEM / SC_SORTWAREHOUSE, SC_SORTPERIODWAREHOUSE
{
	int nRet;
};

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
struct SCStageUseLimitItem	// SC_ITEM / SC_STAGE_USELIMITITEM
{
	int nItemID;
	int nUseCount;
};
#endif

struct SCSpecialItemList	// SC_ITEM / SC_SPECIAL_ITEM_LIST
{
	int nItemIDs[10];
};

struct SCAutoUnpackResult	// SC_ITEM / SC_AUTO_UNPACK_RESULT
{
	int nRet;
};

struct SCDeleteExpireItemList	// SC_ITEM / SC_DELETE_EXPIRE_ITEM_LIST
{
	WORD wCount;
	INT64 biSerial[CASHINVENTORYMAX];
};

#if defined(PRE_ADD_TALISMAN_SYSTEM)
struct CSTalismanOpenSlot		// CS_ITEM / CS_TALISMAN_OPENSLOT
{
	int nSlotIndex;
};

struct SCTalismanOpenSlot		// SC_ITEM / SC_TALISMAN_OPENSLOT
{
	int nRet;
	int nTalismanOpenFlag;
};

struct SCChangeTalisman			// SC_ITEM / SC_CHANGETALISMAN
{
	UINT nSessionID;
	char cSlotIndex;
	TItem Item;
};

struct SCTalismanExpireData		// SC_ITEM / SC_TALISMANEXPIREDATA
{
	bool bOpenFlag;
	__time64_t tExpireDate;
};
#endif

// Enhance
/*
CS_ENCHANT Place item to enchant and press the enchant button
SC_ENCHANT Receive response on whether possible and play movie
CS_ENCHANTCOMPLETE Send enchant complete when animation finishes
SC_ENCHANTCOMPLETE Notify enhancement result (end)
CS_ENCHANTCANCEL Send if canceling midway
SC_ENCHANTCANCEL	
*/

struct CSEnchantItem
{
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	BYTE cItemIndex;
	bool bEnchantEquipedItem;
#else
	BYTE cInvenIndex;
#endif
	INT64 biItemSerial;
#if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM) // Changed because more enhancement support items may be added later.
	char cEnchantGuardType;				// Enhancement protection type: 0-NONE, 1-Item Protection Jelly (no destroy), 2-Greater Item Protection Jelly (no destroy, no downgrade) eEnchantGuardType
#else
	bool bUseEnchantGuard;
#endif	// #if defined(PRE_ADD_ENCHANTSHIELD_CASHITEM)
};

struct SCEnchantItem
{
#if defined(PRE_ADD_EQUIPED_ITEM_ENCHANT)
	BYTE cItemIndex;
	bool bEnchantEquipedItem;
#else
	BYTE cInvenIndex;
#endif
	int nRet;
};

struct SCEnchantComplete
{
	int nItemID;
	char cLevel;
	char cOption;
	int nRet;
};

struct SCEnchantCancel
{
	int nRet;
};

struct SCBroadcastingEffect			// SC_ITEM / SC_BROADCASTINGEFFECT
{
	UINT nSessionID;
	char cType;		// eBroadcastingEffect
	char cState;	// eBroadcastingEffect
};

// SC_CHANGEGLYPH, // Broadcast glyph info to nearby players
struct SCChangeGlyph
{
	UINT nSessionID;
	TItemInfo ItemInfo;
};

struct SCVehicleEquipList			// SC_ITEM, SC_VEHICLEEQUIPLIST
{
	TVehicleCompact VehicleEquip;
};

struct SCVehicleInvenList			// SC_ITEM, SC_VEHICLEINVENLIST
{
	int nTotalInventoryCount;
	BYTE cInvenCount;
	TVehicleCompact VehicleInven[VEHICLEINVENTORYPAGEMAX];
};

struct SCChangeVehicleParts		// SC_ITEM, SC_CHANGEVEHICLEPARTS, SC_CHANGEPETPARTS
{
	UINT nSessionID;
	char cSlotIndex;
	TVehicleItem Equip;
};

struct SCChangeVehicleColor		// SC_ITEM, SC_CHANGEVEHICLECOLOR
{
	UINT nSessionID;
	DWORD dwColor;
	char cSlotIndex;
	INT64 biSerial;
};

struct SCRefreshVehicleInven		// SC_ITEM / SC_REFRESHVEHICLEINVEN
{
	bool bNewSign;	// Whether to show the 'new' badge or not
	int nCount;
	TVehicleCompact ItemList[VEHICLEINVENTORYPAGEMAX];
};

struct SCAddPetExp	// SC_ITEM / SC_ADDPETEXP
{
	UINT nSessionID;
	INT64 biPetSerial;
	int nExp;
};

struct SCChangePetBody			// SC_ITEM, SC_CHANGEPETBODY
{
	UINT nSessionID;
	TVehicleCompact PetInfo;
};

struct SCPetCurrentSatiety
{
	INT64 biPetSerial;
	int nCurrentSatiety;
};
struct SCPetFoodEat
{
	int nResult;
	int nIncSatiety;
};

#if defined(PRE_ADD_EXPUP_ITEM)
struct SCDailyLimitItemError
{
	int nWeightTableIndex;
};
#endif

struct CSRemoveCash		// CS_ITEM, CS_REMOVECASH
{
	INT64 biItemSerial;
	bool bRecovery;
};

struct SCRemoveCash		// SC_ITEM, SC_REMOVECASH
{
	INT64 biItemSerial;
	bool bEternity;		// Permanent item
	__time64_t tExpireDate;
	bool bExpireComplete;
	int nRet;
};

struct SCModItemExpireDate
{
	int nRet;
};

#if defined( PRE_ADD_LIMITED_SHOP )
struct SCLimitedShopItemData	// SC_ITEM, SC_LIMITEDSHOPITEMDATA
{
	short count;
	bool bAllClear;
	LimitedShop::LimitedItemData items[INVENTORYMAX + WAREHOUSEMAX];
};
#endif


#if defined(PRE_ADD_EQUIPLOCK)
struct CSItemLockReq	// CS_ITEM / CS_ITEM_LOCK_REQ, CS_ITEM_UNLOCK_REQ
{
	bool IsCashEquip;
	BYTE nItemSlotIndex; // Index number of the Item to Lock (UnLock)	
	INT64 biItemSerial;	// Target item Serial
};

struct SCItemLockRes	// SC_ITEM / SC_ITEM_LOCK_RES, SC_ITEM_UNLOCK_RES
{
	int nRet; //Result value
	bool IsCashEquip;
	EquipItemLock::TLockItemInfo LockItems;
};

struct SCEquipItemLockList	// SC_ITEM / SC_EQUIPITEM_LOCKLIST
{
	int nEquipCount;	//Normal equipment count
	int nCashEquipCount; //Cash equipment count
	EquipItemLock::TLockItemInfo LockItems[EQUIPMAX+CASHEQUIPMAX];
};
#endif	// #if defined(PRE_ADD_EQUIPLOCK)

#if defined(PRE_PERIOD_INVENTORY)
struct SCPeriodInventory		// SC_ITEM / SC_PERIODINVENTORY
{
	bool bEnable;
	__time64_t tExpireDate;
};

struct SCPeriodWarehouse		// SC_ITEM / SC_PERIODWAREHOUSE
{
	bool bEnable;
	__time64_t tExpireDate;
};
#endif	// #if defined(PRE_PERIOD_INVENTORY)


//--------------------------------------------------------------------------------------------------
//	Item Goods (CS_ITEMGOODS, SC_ITEMGOODS)
//--------------------------------------------------------------------------------------------------

struct CSUnSealItem
{
	BYTE cInvenIndex;
	INT64 biInvenSerial;
};

struct SCUnSealItem
{
	int nRet;
	BYTE cInvenIndex;
};

struct CSSealItem
{
	BYTE cInvenIndex;
	INT64 biInvenSerial;
	INT64 biItemSerial;
};

struct SCSealItem
{
	int nRet;
	BYTE cInvenIndex;
};

struct SCRequestCashWorldMsg
{
	INT64 biItemSerial;
	char cType;
};

// Respond whether skill reset item requested by client can be used.
struct SCCanUseSkillResetCashItem
{
	INT64 biItemSerial;
	int nRet;
	int nRewardSP;
};

// Client finally sends skill reset item use packet
struct CSUseSkillResetCashItemReq
{
#if defined(PRE_MOD_SKILLRESETITEM)
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
#endif
	INT64 biItemSerial;
	char cSkillPage;
};

// Notify client of skill reset cash item usage result
struct SCUseSkillResetCashItemRes
{
	int nRet;
	int nResultSP;		// Final resulting skill points.
	char cSkillPage;
};

#ifdef PRE_ADD_CHANGEJOB_CASHITEM
struct CSUseChangeJobCashItemReq
{
	INT64 biItemSerial;
	int iJobToChange;
};

struct SCUseChangeJobCashItemRes
{
	int nRet;
	int nFirstJobID;
	int nSecondJobID;
	int nResultSkillPoint;
};
#endif // #ifdef PRE_ADD_CHANGEJOB_CASHITEM

// SC_CHANGECOLOR
struct SCChangeColor
{
	UINT nSessionID;
	char cItemType;
	DWORD dwColor;
};

#ifdef PRE_ADD_GACHA_JAPAN
struct SCGachaShopOpen_JP	// SC_ITEMGOODS / SC_JP_GACHAPON_OPEN_SHOP_RES
{
	int nGachaShopIndex;
};

struct CSGachaRequest_JP	// SC_ITEMGOODS / CS_JP_GACHAPON_REQ
{
	int nJobClassID;
	int nSelectedPart;
};

struct SCGachaRes_JP		// SC_ITEMGOODS / SC_JP_GACHAPON_RES
{
	int nRet;
	int nResultItemID;
};
#endif

//	Costume synthesis
struct SCCosMixOpen
{
	int nRet;
};

struct SCCosMixClose
{
	int nRet;
	bool bComplete;
};

struct CSCosMix
{
	INT64	nStuffSerialID[MAXCOSMIXSTUFF];
	UINT	nResultItemID;
	char	cOption;
};

struct SCCosMix
{
	int	 nRet;
	UINT nResultItemID;
	char cOption;
};

struct SCCosDesignMixOpen
{
	int nRet;
};

struct SCCosDesignMixClose
{
	int nRet;
	bool bComplete;
};

struct CSCosDesignMix
{
	INT64	nStuffSerialID[eCDMST_MAX];
};

struct SCCosDesignMix
{
	int nRet;
	INT64 biItemSertial;
	int nLookItemID;

};

#ifdef PRE_ADD_COSRANDMIX
struct SCCosRandomMixOpen
{
	int nRet;
#ifdef PRE_ADD_COSRANDMIX_ACCESSORY
	int nOpenType;
#endif
};

struct SCCosRandomMixClose
{
	int nRet;
	bool bComplete;
};

struct CSCosRandomMix
{
	INT64 nStuffSerialID[MAX_COSTUME_RANDOMMIX_STUFF];
	UINT nResultItemID;
};

struct SCCosRandomMix
{
	int nRet;
	UINT nResultItemID;
};
#endif // PRE_ADD_COSRANDMIX

struct CSPotentialItem
{
	BYTE cInvenIndex;
	INT64 biInvenSerial;
	INT64 biItemSerial;
};

struct SCPotentialItem
{
	int nRet;
	BYTE cInvenIndex;
};
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
struct SCPotentialItemRollback
{
	int nRet;
};
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
struct CSCharmItemRequest		// CS_ITEMGOODS / CS_CHARMITEMREQUEST
{
	char cInvenType;			// eItemPositionType
	short sInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	BYTE cKeyInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	int nKeyItemID;
	INT64 biKeyItemSerial;
};

#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
struct TRangomGiveItemData
{
	int nItemID;	// Item
	int nCount;		// Count for user to acquire
	int nPeriod;	// Duration
	int nGold;
};
#endif		//#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE

struct SCCharmItemRequest		// CS_ITEMGOODS / SC_CHARMITEMREQUEST
{
	char cInvenType;			// eItemPositionType
	short sInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	int nRet;
#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
	BYTE cRandomGiveCount;
	TRangomGiveItemData RandomGiveItems[RANDOMGIVE_CHARMITEM_MAX];
#endif		//#ifdef PRE_ADD_GETITEM_WITH_RANDOMVALUE
};

struct CSCharmItemComplete		// CS_ITEMGOODS / CS_CHARMITEMCOMPLETE
{
	char cInvenType;			// eItemPositionType
	short sInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	BYTE cKeyInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	int nKeyItemID;
	INT64 biKeyItemSerial;
};

struct SCCharmItemComplete		// SC_ITEMGOODS / SC_CHARMITEMCOMPLETE
{
	char cInvenType;			// eItemPositionType
	int nResultItemID;
	int nItemCount;
	int nPeriod;
	int nRet;
};

#if defined (PRE_ADD_CHAOSCUBE)
struct TChaosItem
{
	short wCount;
	short sInvenIndex;
	INT64 biInvenSerial;
};

struct CSChaosCubeRequest		// CS_ITEMGOODS / CS_CHAOSCUBEREQUEST
{
	char cCubeInvenType;
	short sCubeInvenIndex;
	INT64 biCubeInvenSerial;
#if defined( PRE_ADD_CHOICECUBE )
	int nChoiceItemID;	// Selected result item (used only in Choice Box)
#endif
	char cInvenType;
	int nCount;
	TChaosItem ChaosItem[MAX_CHAOSCUBE_STUFF];

#if defined( PRE_ADD_CHOICECUBE )
	CSChaosCubeRequest() : nChoiceItemID(0) {}
#endif
};

struct SCChaosCubeRequest		// CS_ITEMGOODS / SC_CHAOSCUBEREQUEST
{
	int nRet;
	char cInvenType;
	int nCount;
	TChaosItem ChaosItem[MAX_CHAOSCUBE_STUFF];
};

struct CSChaosCubeComplete		// CS_ITEMGOODS / CS_CHAOSCUBECOMPLETE
{
	char cCubeInvenType;
	short sCubeInvenIndex;
	INT64 biCubeInvenSerial;
#if defined( PRE_ADD_CHOICECUBE )
	int nChoiceItemID;	// Selected result item (used only in Choice Box)
#endif
	char cInvenType;
	int nCount;
	TChaosItem ChaosItem[MAX_CHAOSCUBE_STUFF];

#if defined( PRE_ADD_CHOICECUBE )
	CSChaosCubeComplete() : nChoiceItemID(0) {}
#endif
};

struct SCChaosCubeComplete		// SC_ITEMGOODS / SC_CHAOSCUBECOMPLETE
{
	char cInvenType;
	int nResultItemID;
	int nItemCount;
	int nPeriod;
	int nRet;
};

struct CSChaosGaugeCubeAddStuff	// CS_ITEMGOODS / CS_CHAOSGAUGECUBE_ADDSTUFF (26 bytes)
{
	char cCubeInvenType;
	short sCubeInvenIndex;
	INT64 biCubeInvenSerial;
	char cStuffInvenType;
	short sStuffInvenIndex;
	INT64 biStuffInvenSerial;
	int nStuffCount;
};

struct CSChaosGaugeCubeComplete	// CS_ITEMGOODS / CS_CHAOSGAUGECUBE_COMPLETE (11 bytes)
{
	char cCubeInvenType;
	short sCubeInvenIndex;
	INT64 biCubeInvenSerial;
};

struct SCChaosGaugeCubeAddStuff	// SC_ITEMGOODS / SC_CHAOSGAUGECUBE_ADDSTUFF
{
	int nRet;
	int nChaosGaugeType;
	int nCurrentGauge;
};

struct SCChaosGaugeCubeComplete	// SC_ITEMGOODS / SC_CHAOSGAUGECUBE_COMPLETE
{
	int nRet;
	int nChaosGaugeType;
	int nResultItemID;
	int nItemCount;
	int nPeriod;
};

struct TChaosGaugeInfo
{
	int nChaosGaugeType;
	int nItemGauge;
};

struct SCChaosGaugeCubeList		// SC_ITEMGOODS / SC_CHAOSGAUGECUBE_LIST
{
	int nCount;
	TChaosGaugeInfo GaugeList[20];
};
#endif // #if defined (PRE_ADD_CHAOSCUBE)

struct SCPetalTokenResult		// SC_ITEMGOODS / SC_PETALTOKENRESULT, // Announce petal voucher purchase in chat window..
{
	int nUseItemID;
	int nTotalPetal;
};

struct SCAppellationGainResult
{
	int nArrayIndex;
	int nRet;
};

struct CSGuildRename			// CS_ITEMGOODS	/ CS_GUILDRENAME
{
	char cInvenType;
	BYTE cInvenIndex;
	INT64 biItemSerial;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct SCGuildRename			// SC_ITEMGOODS / SC_GUILDRENAME
{
	int nRet;
};

struct CSCharacterRename		// CS_ITEMGOODS / CS_CHARACTERRENAME
{
	char cInvenType;
	BYTE cInvenIndex;
	INT64 biItemSerial;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct SCCharacterRename		// SC_ITEMGOODS / SC_CHARACTERRENAME
{
	int nRet;					//For ERROR_DB_CHANGENAME_NEXTDAY, display date restriction using tNextChangeDate
	UINT nUserSessionID;
	__time64_t tNextChangeDate;
	WCHAR wszCharacterName[NAMELENMAX];
};

#if defined (PRE_ADD_BESTFRIEND)
struct CSBestFriendItemRequest
{
	INT64 biGiftSerial;
	int nMemuItemID;
};

struct SCBestFriendItemRequest
{
	INT64 biGiftSerial;
	int nRet;
};

struct CSBestFriendItemComplete
{
	INT64 biGiftSerial;
	int nMemuItemID;
};

struct SCBestFriendItemComplete
{
	int nResultItemID;
	int nItemCount;
	int nPeriod;
	int nRet;
};
#endif

struct CSChangePetName
{
	INT64 itemSerial;
	INT64 petSerial;
	WCHAR name[NAMELENMAX];
};

struct SCChangePetName
{
	int nRet;
	INT64 biItemSerial;
	UINT nUserSessionID;
	WCHAR wszPetName[NAMELENMAX];
};

struct CSModItemExpireDate
{
	INT64 biExpireDateItemSerial;	// Serial of the item used for duration extension
	INT64 biItemSerial;				// Serial of the item to extend duration
};

struct CSPetSkillItem
{	
	INT64 biItemSerial;				// Pet item serial number
};

struct SCPetSkillItem
{
	int nRet;
	char cSlotNum;					// Slot number
	int nSkillID;					// Skill ID
};

struct CSPetSkillDelete
{
	char cSlotNum;					// Slot number to delete
};

struct SCPetSkillDelete
{
	int nRet;
	char cSlotNum;
};

struct CSPetSkillExpand
{
	INT64 biItemSerial;				// Skill expansion item Serial
};

struct SCPetSkillExpand
{
	int nRet;
};

struct CSEnchantJewelItem
{
	BYTE cInvenIndex;
	INT64 biInvenSerial;
	INT64 biItemSerial;
};

struct SCEnchantJewelItem
{
	int nRet;
	BYTE cInvenIndex;
};

struct SCExpandSkillPageItem
{
	int nRet;	
};


struct CSGuildMark		// CS_ITEMGOODS / CS_GUILDMARK // Change guild mark
{
	int nItemID;
	INT64 biItemSerial;
	int nInvenIndex;		// Inventory index for normal item
	short wGuildMark;
	short wGuildMarkBG;
	short wGuildMarkBorder;

};

struct SCGuildMark		// SC_ITEMGOODS / SC_GUILDMARK
{
	int nRet;
	short wGuildMark;
	short wGuildMarkBG;
	short wGuildMarkBorder;
};

struct SCSourceItem
{
	int nSessionID;
	int nItemID;
	int nRemainTime;
	bool bUsedInGameServer;
};

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
struct SCEffectSkillItem
{
	int nSessionID;
	int nCount;
	bool bUsedInGameServer;
	TEffectSkillData EffectSkill[EFFECTSKILLMAX];
};
struct SCDelEffectSkillItem
{
	int nSessionID;
	int nItemID;
};
struct SCShowEffect
{
	int nItemID;
	int nSessionID;
	bool bUseSession;	
};
struct SCNamedItemID
{
	int nSessionID;
	int nItemID;	
};
#endif

#if defined(PRE_ADD_TRANSFORM_POTION)
struct SCChangeTransform
{
	int nSessionID;
	int nTransformID;
	int nExpireTime;	// minute;
};
#endif

//---------------------------------------------------------------1-----------------------------------
//	NPC (CS_NPC, SC_NPC)
//--------------------------------------------------------------------------------------------------

struct CSNpcTalk				// CS_NPC / CS_NPCTALK
{
	UINT  nNpcUniqueID;
	DWORD dwIndexHashCode;
	DWORD dwTargetHashCode;
};

struct CSNpcTalkEnd
{
	UINT nNpcUniqueID;
};

struct TalkParam 
{
	enum
	{
		STRING = 1,
		INT,
	};
	char  cType;
	WCHAR wszKey[NAMELENMAX];
	int   nValue;
};

const int TALK_PARAM_MAX = 10;
struct SCNpcTalk				// SC_NPC / SC_NPCTALK
{
	UINT  nNpcUniqueID;
	DWORD dwIndexHashCode;
	DWORD dwTargetHashCode;
	bool bIsEnd;

	short nCount;
	TalkParam TalkParamArray[TALK_PARAM_MAX];
};

struct SCShowWareHouse
{
	int iItemID;
};

struct SCOpenUpgradeItem
{
	int iItemID;
};

struct SCOpenCompoundItem				// SC_NPC / eNpc::SC_OPENCOMPOUND_ITEM
{
	UINT  nCompoundShopID;
};

struct SCOpenCompound2Item				// SC_NPC / eNpc::SC_OPENCOMPOUND2_ITEM
{
	UINT  nCompoundGroupID;
	int	  iItemID;
};

struct SCOpenGuildMgrBox				// SC_NPC / eNpc::SC_OPENGUILDMGRBOX
{
	int  nGuildMgrNo;	// ?
};

struct SCOpenBrowser
{
	char url[MAX_BROWSER_URL];
	int nX;
	int nY;
	int nWidth;
	int nHeight;
};

#if defined(PRE_ADD_OPEN_QUEST_TEXTURE_DIALOG)
struct SCOpenTextureDialog
{
	int		nFileIndex;
	float	fX;
	float	fY;
	int		nPosition;
	int		nTime;
	int		nDialogIndex;
	bool	bAutoCloseDialog;
};

struct SCCloseTextureDialog
{
	int nDialogID;
};
#endif

//--------------------------------------------------------------------------------------------------
//	QUEST (CS_QUEST, SC_QUEST)
//--------------------------------------------------------------------------------------------------
struct CSCancelQuest				// CS_QUEST / CS_CANCEL_QUEST
{
	int nQuestIndex;
};

struct SCQuestInfo					// SC_QUEST / SC_QUESTINFO
{
	DNNotifier::Data	NotifierData[DNNotifier::RegisterCount::Total];
	char cCount;
	TQuest Quest[MAX_PLAY_QUEST];
};

struct SCQuestCompleteInfo			// SC_QUEST / SC_QUESTCOMPLETEINFO
{
	char CompleteQuest[COMPLETEQUEST_BITSIZE];
};

struct RefreshType 
{
	enum
	{
		ADD_QUEST = 1,
		CANCEL_QUEST,
		REFRESH_STEP,
		REFRESH_JOURNAL,
		REFRESH_COUNTING,
		REFRESH_COMPLETECOUNTING,
		REFRESH_ALLCOMPLETECOUNTING,
		REFRESH_STEPANDJOURNAL,
	};
};

struct SCRefreshQuest				// SC_QUEST / SC_REFRESH_QUEST
{
	short		  nRefreshType;
	char		  cQuestSlot;
	UINT		  nQuestID;
	TQuest		  Quest;
};

struct SCCompleteQuest				// SC_QUEST / SC_COMPLETE_QUEST
{
	bool		  bDeletePlayList;
	char		  cQuestSlot;
	UINT		  nQuestID;
	bool		  bNeedMarking;
};

struct SCMarkingCompleteQuest			// SC_QUEST / SC_MARKING_COMPLETE_QUEST
{
	UINT		  nQuestID;
};


struct SCRemoveQuest				// SC_QUEST / SC_REMOVE_QUEST
{
	char		  cQuestSlot;
	UINT		  nQuestID;
	int nRetCode;
};

struct SCPlayCutScene			// SC_ETC / SC_PLAYCUTSCENE
{
	UINT nCutSceneTableID;
	int  nQuestIndex;
	int  nQuestStep;
	UINT nNpcObjectID;
	bool bQuestAutoFadeIn;
	DWORD dwQuestPlayerUniqueID;
};

struct SCQuestReward		// SC_QUEST / SC_QUEST_REWARD 
{
	int nRewardTableIndex;
	bool bActivate;
};

struct SCQuestResult		// SC_QUEST / SC_QUESTRESULT
{
	int nRetCode;
};

struct SCCompleteCutScene
{
	bool bFadeIn;
};

struct SCSkipCutScene {
	UINT nSessionID;
};

struct CSSelectQuestReward
{
	UINT		nIndex;				// Reward table index
	bool		SelectArray[QUESTREWARD_INVENTORYITEMMAX];
	bool		SelectCashArray[MAILATTACHITEMMAX];
};

struct CSCompleteCutScene
{
	UINT nSessionID;
};

struct SCAssignPeriodQuest			// SC_QUEST / SC_ASSIGN_PERIODQUEST
{
	UINT		  nQuestID;
	bool		  bFlag;
};

struct SCPeriodQuestNotice			// SC_QUEST / SC_NOTICE_PERIODQUEST
{
	int			nItemID;
	int			nNoticeCount;
};

struct SCScorePeriodQuest			// SC_QUEST / SC_SCORE_PERIODQUEST
{
	int			nCurrentCount;
	int			nMaxCount;
};

struct CSShortCutQuest
{
	UINT nQuestID;
};

#if defined(PRE_ADD_REMOTE_QUEST)
struct SCGainRemoteQuest			// CS_QUEST / CS_ACCEPT_REMOTE_QEUST
{
	UINT nQuestID;
};

struct CSAcceptRemoteQuest			// CS_QUEST / CS_ACCEPT_REMOTE_QEUST
{
	UINT nQuestID;
};

struct SCAcceptRemoteQuest			// SC_QUEST / SC_ACCEPT_REMOTE_QEUST
{
	int nRet;
	UINT nQuestID;
};

struct CSCompleteRemoteQuest : public CSSelectQuestReward	// CS_QUEST / CS_COMPLETE_REMOTE_QUEST
{
	UINT nQuestID;
};

struct CSCancelRemoteQuest			// CS_QUEST / CS_CANCEL_REMOTE_QUEST
{
	UINT nQuestID;
};

struct SCRemoveRemoteQuest			// SC_QUEST / SC_REMOVE_REMOTE_QEUST
{
	UINT nQuestID;
};

struct SCCompleteRemoteQuest			// SC_QUEST / SC_COMPLETE_REMOTE_QEUST
{
	UINT nQuestID;
};

#endif


//--------------------------------------------------------------------------------------------------
//	CHAT (CS_CHAT, SC_CHAT)
//--------------------------------------------------------------------------------------------------

struct CSChat					// CS_ETC / CS_CHAT
{
	eChatType eType;
	INT64 biItemSerial;
	short nLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct CSChatPrivate			// CS_ETC / CS_CHAT_PRIVATE
{
	WCHAR wszToCharacterName[NAMELENMAX];
	short nLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct SCChat					// SC_ETC / SC_CHATMSG
{
	eChatType eType;
	WCHAR wszFromCharacterName[NAMELENMAX];
	int nRet;					// 1: success, -1: target not found, -2: sent to self
	short nLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct SCWorldSystemMsg			// SC_WORLDSYSTEMMSG
{
	WCHAR wszFromCharacterName[NAMELENMAX];
	char cType;	// eChatType (WORLDCHATTYPE_...)
	int nID;
	int nValue;
	WCHAR wszToCharacterName[NAMELENMAX];
};

struct SCNotice			//SC_NOTICE
{
	int nShowSec;
	short nLen;
	WCHAR wszNoticeMsg[CHATLENMAX];
};

struct SCGuildChat
{
	TGuildUID GuildUID;
	UINT nAccountDBID;			// Account DBID
	INT64 nCharacterDBID;			// Character DBID
	short nLen;					// Chat length
	WCHAR wszChatMsg[CHATLENMAX];	// Chat message
};

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
struct SCDoorsGuildChat
{
	TGuildUID GuildUID;
	INT64 biCharacterDBID;			// Character DBID
	short nLen;					// Chat length
	WCHAR wszChatMsg[CHATLENMAX];	// Chat message
};
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT

struct CSDice
{
	eChatType eType;
	short range;
};

struct CSChatChatRoom			// CS_CHAT_CHATROOM
{
	short nLen;
	WCHAR wszChatMsg[CHATLENMAX];
};

struct SCChatServerMsg			// SC_SERVERMSG
{
	char cType;	
	char cStringType;
	int nMsgIndex;
	char cCaption;				// This is client's CDnInterface::emCAPTION_TYPE.
	BYTE cFadeTime;				// Fade duration (seconds)
};

//--------------------------------------------------------------------------------------------------
//	ROOM (CS_ROOM, SC_ROOM)
//--------------------------------------------------------------------------------------------------

struct CSSyncWait				// CS_ROOM / CS_SYNC_WAIT
{
	UINT nSessionID;
};

struct CSOKDungeonClear
{
	UINT nSessionID;
};

//struct CSWarpDungeonClear
//{
//	bool bWarpVillage;
//};

struct CSWarpDungeon
{
	bool bWarpVillage;
};

struct CSSelectRewardItem
{
	char cItemIndex;
};

struct CSIdentifyRewardItem
{
	char cItemIndex;
};

struct CSDLChallengeResponse
{
	bool bAccept;
};

struct SCDLChallengeResponse
{
	bool bAccept;
};

struct SCSyncWait				// SC_ROOM / SC_SYNC_WAIT
{
	UINT nSessionID;
};

struct SCSyncStart				// SC_ROOM / SC_SYNC_START
{
	UINT nSessionID;
};

struct SCRefreshMember		// SC_ROOM / SC_REFRESH_MEMBER
{
	BYTE cCount;
	UINT nMemberSessionID[PARTYMAX];
};

struct SCSelectRewardItem
{
	UINT nSessionID;
	char cItemIndex;
};

struct SCIdentifyRewardItem
{
	char cItemIndex;
	char cBoxType;
};

struct SCRoomSyncSeqLevel		//SC_ROOM / SC_ROOMSYNC_SEQLEVEL
{
	BYTE cSeqLevel;
};

//#if defined(_GAMESERVER) || defined(_WINDOWS)
#if defined(_GAMESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct SCGenerationMonster		//SC_GENERATION_MONSTER_MSG
{
	UINT nSessionID;
	int nSeed;
	UINT nMonsterID;
	int nEventAreaUniqueID;
	EtVector3 vPos;
	EtVector3 vVel;
	EtVector3 vResistance;
	int nTeam;
	bool bRandomFrameSummon;
	bool bUseRotationQuat;		// When SummonMonster signal is used, send the summoned monster's angle value in a packet.
	EtQuat qRotation;
};

// Replaced with variable-length packet. /////////////////////////////////////////////////
struct SCGenerationProp			//SC_GENERATION_PROP_MSG
{
	int nSummonPropTableID;
	int nPropUniqueID;
	int nSeed;
	EtVector3 vPos;
	EtVector3 vRotate;
	EtVector3 vScale;
};
//////////////////////////////////////////////////////////////////////////
#endif

struct SCGateInfoMessage		// SC_ROOM / SC_GATEINFO_MSG
{
	BYTE cCount;
	BYTE cGateIndex[WORLDMAP_GATECOUNT];
	BYTE cPermitFlag[WORLDMAP_GATECOUNT];
};

struct SCGateEjectInfo			// SC_ROOM / SC_GATEEJECTINFO_MSG
{
	BYTE cGateIndex;
	BYTE cLessLevelCount;
	BYTE cNotEnoughItemCount;
	BYTE cExceedCount;
	BYTE cNotRideVehicleCount;
	BYTE cActorIndex[PARTYMAX*2];
};

struct sEventStageClearBonus
{
	BYTE nType;							//See eEventType3
	UINT nClearEventBonusExperience;
};

struct TDungeonClearInfo
{
	UINT nSessionID;
	BYTE cRank;
	UINT nDefaultCP;
	UINT nBonusCP;
	USHORT nMaxComboCount;
	USHORT nKillBossCount;
	UINT nRewardGold;
	char cClearRewardType;
	UINT nCompleteExperience;
	UINT nRewardExperience;
	UINT nMeritBonusExperience;
	char cMaxLevelCharCount;
	UINT nPromotionExperience;
	char cShowBoxCount;
	char cSelectBoxCount;
	BYTE cBonusCount;
	sEventStageClearBonus EventClearBonus[EVENTCLEARBONUSMAX];
};

struct SCDungeonClear			// SC_ROOM / SC_DUNGEONCLEAR_MSG
{
	BYTE cCount;
	UINT nDungeonClearTableID;
	UINT nClearTime;
	char cCurLevel;
	int nCurExp;
	bool bIgnoreRewardItem;
	TDungeonClearInfo Info[PARTYMAX];
};

#if defined(PRE_ADD_CP_RANK)
struct TStageClearBest
{
	INT64 biCharacterDBID;
	WCHAR wszCharName[NAMELENMAX];
	char cRank;
	int nClearPoint;
};

struct SCAbyssStageClearBest		// SC_ROOM / SC_ABYSS_STAGE_CLEAR_BEST
{
	TStageClearBest sLegendClearBest;
	TStageClearBest sMonthlyClearBest;
	char cMyBestRank;
	int nMyBestCP;
};
#endif

struct TRewardItemResultStruct {
	UINT nSessionID;
	BYTE cIndex;
};

struct SCDungeonClearRewardItemResult
{
	BYTE cCount;
	TRewardItemResultStruct Info[PARTYMAX];
};

struct TRewardItemStruct {
	UINT nSessionID;
	UINT nRewardItemID[4];
#if !defined( PRE_ADD_NAMEDITEM_SYSTEM )
	BYTE cRewardItemType[4];
#endif
	int nRewardItemRandomSeed[4];
	char cRewardItemOption[4];
	short wRewardItemCount[4];
	char cRewardItemSoulBound[4];
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
	char cRewardItemLevel[4]; //Enhancement info
	char cRewardItemPotential[4]; //Potential info
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
};

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
struct TRewardBoxTypeStruct
{
	UINT nSessionID;
	BYTE cRewardItemType[4];
};
struct SCDungeonClearBoxType
{
	BYTE cCount;
	TRewardBoxTypeStruct Info[PARTYMAX];
};
#endif
struct SCDungeonClearRewardItem
{
	BYTE cCount;
	TRewardItemStruct Info[PARTYMAX];
};

struct SCWarpDungeonClear
{
#if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
	int nLastVillageMapIndex;
#endif	// #if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
};

struct SCDungeonClearLeaderWarp
{
	bool bEnable;
};

struct SCDungeonFailed
{
	bool bCanWarpWorldMap;
	bool bTimeOut;
#if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
	int nLastVillageMapIndex;
#endif	// #if defined(PRE_ADD_RETURN_VILLAGE_SHOWNAME)
};

struct SCDungeonClearWarpOutFail
{
	int nRet;
};

struct SCOpenDungeonOpenLevel
{
	UINT nMapIndex;
	char cDifficulty;
};

struct SCSyncDungeonTimeAttack
{
	int iRemainSec;
	int iOrgSec;
};

#if defined(PRE_MOD_DARKLAIR_RECONNECT)
struct SCDLRoundInfo	// SC_ROOM / SC_DLROUND_INFO
{
	int nCurrentRound;
	int nTotalRound;
	bool bBossRound;
};
#endif

struct TDLDungeonClearInfo
{
	UINT nSessionID;
	UINT nExperience;
	UINT nMeritBonusExperience;
	char cShowBoxCount;
	char cSelectBoxCount;
	UINT nRewardGold;
	char cClearRewardType;
	char cMaxLevelCharCount;
	UINT nPromotionExperience;
	BYTE cBonusCount;
	sEventStageClearBonus EventClearBonus[EVENTCLEARBONUSMAX];
};

struct SCDLDungeonClear			// SC_ROOM / SC_DUNGEONCLEAR_MSG
{
	BYTE cCount;
	UINT nClearTime;
	int nClearRound;
	bool bClear;
	TDLDungeonClearInfo Info[PARTYMAX];
};

struct TDLRankHistoryMemberInfo {
	USHORT nLevel;
	int	nJobIndex;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TDLRankHistoryPartyInfo {
	int nRank;
	USHORT nPlayRound;
	UINT nPlaySec;
	WCHAR wszPartyName[PARTYNAMELENMAX];
	BYTE cPartyUserCount;
	TDLRankHistoryMemberInfo Info[DarkLair::Common::MaxPlayer];
};

struct SCDLRankInfo
{
	TDLRankHistoryPartyInfo CurScore;
	TDLRankHistoryPartyInfo TopScore;
	TDLRankHistoryPartyInfo HistoryScore[DarkLair::Rank::SelectTop];
};

struct SCChangeGateState		// SC_ROOM / SC_CHANGE_GATESTATE_MSG
{
	BYTE cGateInde;
	UINT nPermitFlag;
};

struct SCChangeGameSpeed		// SC_ROOM /SC_CHANGE_GAMESPEED
{
	float fSpeed;
	UINT dwDelay;
};

struct SCDungeonHistory
{
	BYTE cBuf[1024];
};

struct SCROOM_SYNC_MEMBERINFO
{
	bool				bIsBreakInto;
	bool				bIsStart;
	short				nCount;
	SPartyMemberInfo	Member[MAX_SEND_SYNCPLAYER];
};

#ifdef PRE_MOD_SYNCPACKET
struct TSyncTeam
{
	UINT nSessionID;
	USHORT usTeam;
};

struct SCROOM_SYNC_MEMBERTEAM
{
	BYTE				cCount;
	TSyncTeam tTeam[MAX_SEND_SYNCPLAYER];
};
#else		//#ifdef PRE_MOD_SYNCPACKET
struct SCROOM_SYNC_MEMBERTEAM
{
	BYTE				cCount;
	USHORT				unStartIndex;
	USHORT				usTeam[MAX_SEND_SYNCPLAYER];
};
#endif		//#ifdef PRE_MOD_SYNCPACKET

struct SCROOM_SYNC_MEMBERDEFAULTPARTS
{
	short				nCount;
	int					iDefaultPartsIndex[PvPCommon::Common::MaxPlayer*4];
};

struct SCROOM_SYNC_MEMBEREQUIP
{
	short				nCount;
	TPartyMemberEquip	MemberEquip[PvPCommon::Common::MaxPlayer];
};

struct SCROOM_SYNC_MEMBERSKILL
{
	short				nCount;
	TPartyMemberSkill	MemberSkill[PvPCommon::Common::MaxPlayer];
};

struct SCROOM_SYNC_MEMBER_BREAKINTO
{
	SPartyMemberInfo	Member;
	USHORT				usTeam;
	BYTE				cGMTrace;
	int					iDefaultPartsIndex[4];
	char				cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];
	TGuildSelfView		GuildSelfView;	// Guild visual info (personal)
	short				nEquipCount;
	short				nCashEquipCount;
	short				nSkillCount;
	short				nGlyphCount;
	int					iSelectAppellation;
	int					nCoverAppellation;
#if defined(PRE_ADD_VIP)
	bool				bVIP;
#endif	// #if defined(PRE_ADD_VIP)
	char				cAccountLevel; // Account authority (eAccountLevel)
	char				cEternityItemCount;
	TVehicleCompact		VehicleInfo;
	TVehicleCompact		PetInfo;
	TItemInfo			EquipArray[EQUIPMAX];
	TItemInfo			CashEquipArray[CASHEQUIPMAX];
	TItemInfo			GlyphArray[GLYPHMAX];
	TSkill				SkillArray[SKILLMAX];
};

#ifdef PRE_MOD_SYNCPACKET
struct TSyncHPSP
{
	UINT nSessionID;
	INT64 iHP;
	int iSP;
};

struct SCROOM_SYNC_MEMBERHPSP
{
	BYTE	cCount;
	TSyncHPSP tHPSP[MAX_SEND_SYNCPLAYER];
};
#else		//#ifdef PRE_MOD_SYNCPACKET
struct SCROOM_SYNC_MEMBERHPSP
{
	BYTE	cCount;
	USHORT	unStartIndex;
	int		iHPSPArr[MAX_SEND_SYNCPLAYER*2];
};
#endif		//#ifdef PRE_MOD_SYNCPACKET

#ifdef PRE_MOD_SYNCPACKET
struct TBattleMode
{
	UINT nSessionID;
	BYTE cBattleMode;
};

struct SCROOM_SYNC_MEMBERBATTLEMODE
{
	BYTE	cCount;
	TBattleMode tBattleMode[MAX_SEND_SYNCPLAYER];
};
#else		//#ifdef PRE_MOD_SYNCPACKET
struct SCROOM_SYNC_MEMBERBATTLEMODE
{
	BYTE	cCount;
	USHORT	unStartIndex;
	BYTE	cBattleModeArr[MAX_SEND_SYNCPLAYER];
};
#endif		//#ifdef PRE_MOD_SYNCPACKET

struct TDropItemSync:public SCCreateDropItem
{
	UINT	uiLifeTime;	// m_fLifeTime*1000
	float	fDistance;
};

struct SCROOM_SYNC_DROPITEMLIST
{
	BYTE			cCount;
	TDropItemSync	sDropItemList[SENDDROPITEMLISTMAX];
};

//--------------------------------------------------------------------------------------------------
//	SKILL (CS_SKILL, SC_SKILL)
//--------------------------------------------------------------------------------------------------

struct SCSkillList				// SC_SKILL / SC_SKILLLIST
{
	BYTE cSkillCount;
	char cSkillPage;
	TSkill Skill[SKILLMAX];
};

struct CSAddSkill				// CS_SKILL / CS_ADDSKILL
{
	int nSkillID;
};

struct SCAddSkill				// SC_SKILL / SC_ADDSKILL
{
	int nSkillID;
	int nRet;
};

struct CSDelSkill				// CS_SKILL / CS_ADDSKILL
{
	int nSkillID;
};

struct SCDelSkill				// SC_SKILL / SC_DELSKILL
{
	int nSkillID;
	int nRet;
};

struct SCSkillLevelUp			// SC_SKILL / SC_SKILLLEVELUP 
{
	int nSkillID;
	BYTE cLevel;
	int nRetCode;
};

struct CSSkillLevelUpReq		// CS_SKILL / CS_SKILLLEVELUP_REQ
{
	int nSkillID;
	BYTE cUseSkillPoint;
};

struct SCOtherPlayerSkillLevelUp	// CS_SKILL / CS_OTHERPLAYERSKILLLEVELUP
{
	int nPartyIndex;
	int nSkillID;
	BYTE cLevel;
};

struct SCPushSkillPoint
{
	unsigned short usSkillPoint;
};

struct SCUnlockSkill	// CS_SKILL / SC_UNLOCKSKILL
{
	int nSkillID;
	int nRetCode;
};

struct CSAcquireSkillReq		// CS_SKILL / CS_ACQUIRESKILL_REQ
{
	int nSkillID;
};

struct SCAcquireSkill		// CS_SKILL / SC_ACQUIRESKILL
{
	int nSkillID;
	int nRetCode;
};


struct CSUnlockSkillByMoneyReq	// CS_SKILL / SC_UNLOCKSKILL_BY_MONEY
{
	int nSkillID;
};

#if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)
struct TReservationSkillReq
{
	char cType;			// eSkillReservation
	int nSkillID;		
	BYTE nLevel;		// Final level only in the case of LevelUp.
};

struct TReservationSkillAck
{
	TReservationSkillReq tReservationSkill;	
	int nResult;
};

struct CSReservationSkillListReq // CS_SKILL / CS_RESERVATION_SKILL_LIST_REQ
{
	int nCount;
	TReservationSkillReq tReservationSkill[ReservationSKillList::ReservationMax];
};

struct SCReservationSkillListAck // SC_SKILL / SC_RESERVATION_SKILL_LIST_ACK
{
	int nCount;
	TReservationSkillAck tReservationSkillAck[ReservationSKillList::ReservationMax];
};
#endif // #if defined(PRE_ADD_SKILL_LEVELUP_RESERVATION)

#ifdef PRE_ADD_PRESET_SKILLTREE
struct TSkillSetPartialData
{
	int nSkillID;
	BYTE cLevel;
};

struct CSSaveSkillSet			// CS_SKILL / CS_SAVE_SKILLSET
{
	BYTE cIndex;
	WCHAR wszSetName[SKILLPRESETNAMEMAX];
	BYTE cCount;
	TSkillSetPartialData SKills[SKILLMAX];
};

struct CSDeleteSkillSet			// CS_SKILL / CS_DELETE_SKILLSET
{
	BYTE cIndex;
};

struct TSkillSetIndexData
{
	BYTE cIndex;
	BYTE cCount;				//Number of skills in the preset
	WCHAR wszSetName[SKILLPRESETNAMEMAX];
};

struct SCSKillSetList			// CS_SKILL / SC_LIST_SKILLSET
{
	int nRetCode;
	TSkillSetIndexData SKillIndex[SKILLPRESETMAX];
	TSkillSetPartialData SKills[SKILLMAX * SKILLPRESETMAX];
};

struct SCSKillSaveResult		// CS_SKILL / SC_SAVE_SKILLSET
{
	int nRetCode;
};

struct SCSKillDeleteResult		// CS_SKILL / SC_DELETE_SKILLSET
{
	int nRetCode;
};
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE

struct SCSkillReset // CS_SKILL / SC_RESET_SKILL
{
	char cSkillPage;
	int nResultSkillPoint;	
};

struct SCChangeSkillPage		//SC_SKILL	/ SC_CHANGE_SKILLPAGE_RES	
{
	char cSkillPage;	
};

struct SCSkillPageCount		//SC_SKILLPAGE_COUNT
{
	char cSkillPageCount;	
};

struct CSChangeSkillPage		// CS_SKILL	/ CS_CHANGE_SKILLPAGE
{
	char cSkillPageIndex;
};
//--------------------------------------------------------------------------------------------------
//	TRADE (CS_TRADE, SC_TRADE)
//--------------------------------------------------------------------------------------------------

// Shop
// npc shop
struct CSShopRemoteOpen			// CS_TRADE / CS_SHOP_REMOTEOPEN
{
	Shop::Type::eCode Type;
};

struct SCShopOpen				// SC_TRADE / SC_SHOP_OPEN
{
	int nShopID;
	Shop::Type::eCode Type;
};

struct CSShopBuy				// CS_TRADE / CS_SHOP_BUY
{
	char cTapIndex;
	BYTE cShopIndex;			// Corresponding index value received from shoplist
	short wCount;
};

struct SCShopBuyResult			// SC_TRADE / SC_SHOP_BUY
{
	int nRet;
};

struct CSShopSell				// CS_TRADE / CS_SHOP_SELL
{
	BYTE cInvenIndex;			// inven index
	short wCount;
	INT64 biItemSerial;
};

struct SCShopSellResult			// SC_TRADE / SC_SHOP_SELL
{
	int nRet;
};

struct SCShopRepurchaseList
{
	int nRet;
	BYTE cCount;
	TRepurchaseItemInfo List[Shop::Repurchase::MaxList];
};

struct CSShopRepurchase
{
	int iRepurchaseID;
};

struct SCShopRepurchase
{
	int iRet;
	int iRepurchaseID;
};

// SkillShop
struct TSkillShopInfo
{
	BYTE cSlotIndex;
	int nSkillID;
	int nPrice;
};

struct SCSkillShopOpen			// SC_TRADE / SC_SKILLSHOP_OPEN
{
	int nRet;	// Currently meaningless.
};

struct SCSkillShopBuyResult		// SC_TRADE / SC_SKILLSHOP_BUY
{
	int nSkillID;
	short wSkillPoint;
	int nRet;
};

// Mail
struct CSMailBox				// CS_TRADE / CS_MAILBOX
{
	char cPageNum;			// Page number: 1-based
};

struct TMailBoxInfo
{
	int nMailDBID;							// mail db table id
	short wIndex;
	WCHAR wszFromCharacterName[MAILNAMELENMAX];	// Sender
	WCHAR wszTitle[MAILTITLELENMAX];		// Mail subject
	BYTE cRemainDay;						// Remaining days
	char cReadType;							// Unsealed state
	char cAttachType;						// Item attachment state
	char cDeliveryType;
	char cMailType;							// Mail type (0: normal mail, 1: mission mail, 2: market mail)
	bool bNewFlag;							// Whether title/body are encoded
};

struct SCMailBox				// SC_TRADE / SC_MAILBOX
{
	int nRetCode;
	char cDailyMailCount;		// Daily send count
	short wTotalCount;
	short wUnreadCount;
	char cMailBoxCount;					// How many to send
	TMailBoxInfo MailBox[MAILBOXMAX];	// Mail content
};

struct TAttachMailItem
{
	int nInvenIndex;	// Inventory index of the attached item
	int nItemID;
	int nCount;			// Number of attached items
	INT64 biSerial;
};

struct CSSendMail				// CS_TRADE / CS_SENDMAIL
{
	WCHAR wszToCharacterName[NAMELENMAX];	// Recipient
	WCHAR wszTitle[MAILTITLELENMAX];		// Title
	WCHAR wszText[MAILTEXTLENMAX];			// Content
	char cDeliveryType;						// eMailType (MAILTYPE_NORMAL, MAILTYPE_PREMIUM)
	int nAttachCoin;						// Attached coins (unlikely to exceed 2.1 billion)
	char cAttachItemTotalCount;
	TAttachMailItem AttachMailItem[MAILATTACHITEMMAX];
};

struct CSMailDBIDArray			// CS_TRADE / CS_MAIL_DELETE, CS_MAIL_ATTACHALL
{
	int nMailDBID[MAILPAGEMAX];
};

struct SCSendMail				// SC_TRADE / SC_MAIL_SEND
{
	char cDailyMailCount;		// Daily send count
	int nRet;			// Will be sent with ones starting with ERROR_MAIL_
};

struct CSAttachMail
{
	int nMailDBID;
	char cAttachSlotIndex;		// -1: Claim all 5 slots, 0 - 4: Claim individually
};

struct SCMailResult				// SC_TRADE / SC_MAIL_ATTACH
{
	int nRet;			// Will be sent with ones starting with ERROR_MAIL_
};

struct SCMailDeleteResult		// SC_TRADE / SC_MAIL_DELETE
{
	int nTotalMailCount;
	int nRet;			// Will be sent with ones starting with ERROR_MAIL_
};

struct SCAttachAllMailResult	// SC_TRADE / SC_MAIL_ATTACHALL
{
	int nMailDBID[MAILPAGEMAX];	
	int nRet;			// Will be sent with ones starting with ERROR_MAIL_
};

struct CSReadMail				// CS_TRADE / CS_MAIL_READ, CS_MAIL_READCADGE
{
	int nMailDBID;
};

struct SCReadMail				// SC_TRADE / SC_MAIL_READ
{
	int nRet;			// Will be sent with ones starting with ERROR_MAIL_
	bool bNewFlag;							// Whether title and content are encoded
	short wNotReadMailCount;				// Unread mail
	int nMailDBID;
	WCHAR wszText[MAILTEXTLENMAX];			// Content
	__time64_t tSendDate;
	int nAttachCoin;
	char cDeliveryType;
	char cAttachCount;
	TItemInfo AttachItem[MAILATTACHITEMMAX];	// Attached item
};

struct TWishItemInfo
{
	int nItemSN;
	int nItemID;
	char nOptionIndex;
};

struct SCReadCadgeMail			// SC_TRADE / SC_MAIL_READCADGE
{
	int nRet;								// Will be sent with ones starting with ERROR_MAIL_
	bool bNewFlag;							// Whether title and content are encoded
	short wNotReadMailCount;				// Unread mail
	int nMailDBID;
	WCHAR wszText[MAILTEXTLENMAX];			// Content
	__time64_t tSendDate;
	bool bCadgeComplete;					// Gift request completion status
	int nPackageSN;
	char cWishListCount;
	TWishItemInfo WishList[PACKAGEITEMMAX];
};

struct SCNotifyMail
{
	bool bNewMail;						// Whether it is new mail or just unread mail
	short wTotalMailCount;				// Total mail count
	short wNotReadMailCount;				// Unread mail
	bool bExpiration;					// Expiration warning
};

// Trading house
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
struct TSearchTrade
{
	int nSearchItemID;
	int nSearchNameID;
};
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE

struct CSMarketList				// CS_TRADE / CS_MARKETLIST
{
	short wPageNum;						// Page number: 1-based
	char cMinLevel;
	char cMaxLevel;
	char cItemGrade[ITEMRANK_MAX];		// Grade classification
	char cJob;							// Class category (All: -1)
	short cMainType;					// Default category (All: -1)
	char cDetailType;					// Subcategory (all: -1)
	WCHAR wszSearchWord1[SEARCHLENMAX];	// Search keyword
	WCHAR wszSearchWord2[SEARCHLENMAX];	// Search keyword
	WCHAR wszSearchWord3[SEARCHLENMAX];	// Search keyword
	char cSortType;						// eSortType
	WCHAR wszSearchItemName[SEARCHLENMAX];	// Item search keyword
	int nExchangeItemID;				// ItemID from ExchangeTable
	bool bLowJobGroup;					// Include sub-classes
	char cPayMethodCode;					// 0-Mixed, 1-Coin, 3-Petal
#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
	BYTE cCount;
	TSearchTrade SearchList[MultiLanguage::Common::SerachCountMax];
#endif		//#ifdef PRE_ADD_ITEMNAME_AUTOCOMPLETE
};

struct TMarketInfo
{
	int nMarketDBID;				// market table db ID

	WCHAR wszSellerName[NAMELENMAX];	// Seller name

	int nItemID;
	short wCount;		// Count
	USHORT wDur;			// Durability
	int nRandomSeed;
	char cLevel;		// Level
	char cPotential;	// Potential
	char cOption;
	char cSealCount;	// Seal count

	int nPrice;		// Price
	int nUnitPrice;	// Unit price
	bool bDisplayFlag1;		// Please enter 0. Currently unused field
	bool bDisplayFlag2;		// Please enter 0. Currently unused field
	bool bPremiumTrade;	// Premium trade item via paid item
	char cMethodCode;	// 1=Game money, 3=Petal
	char cPotentialMoveCount;		//Potential transfer count
};

struct SCMarketList				// SC_TRADE / SC_MARKETLIST
{
	int nRetCode;
	int nMarketTotalCount;
	char cMarketCount;
	TMarketInfo MarketInfo[MARKETMAX];
};

struct TMyMarketInfo
{
	int nMarketDBID;					// market table db ID

	int nItemID;
	short wCount;		// Count
	USHORT wDur;			// Durability
	int nRandomSeed;
	DWORD dwColor;		// Color
	char cLevel;		// Level
	char cPotential;	// Potential
	char cOption;
	char cSealCount;	// Seal count

	int nItemPrice;						// Price
	char cSellType;						// Sale status 0: unsold, 1: sold
	int nRemainTime;
	bool bPremiumTrade;	// Premium trade item via paid item
	char cPayMethodCode; // PayMethodCode (DB data) 1-Gold, 3-Petal
	char cItemPotentialMoveCount;		//Potential transfer count
};

struct SCMyMarketList			// SC_TRADE / SC_MYMARKETLIST
{
	int nRetCode;
	short wSellingCount;	// On sale
	short wClosingCount;	// Period closed
	short wWeeklyRegisterCount;		// Number of trades registered in a week starting from Saturday 04:00 AM
	short wRegisterItemCount;		// Number of registered items
	bool bPremiumTrade;
	char cMarketCount;
	TMyMarketInfo MarketInfo[MYMARKETMAX];
};

struct CSMarketRegister			// CS_TRADE / CS_MARKET_REGISTER
{
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	short wCount;
#if defined(PRE_ADD_PETALTRADE)
	char cPayMethodCode;		// PayMethodCode (DB data) 1-Gold, 3-Petal
#endif
	int nPrice;
	char cPeriodIndex;
	bool bPremiumTrade;
};

struct CSMarketBuy				// CS_TRADE / CS_MARKET_BUY
{
	int nMarketDBID;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	bool bMini;
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
};

struct SCMarketBuyResult		// SC_TRADE / SC_MARKET_BUY
{
	int nMarketDBID;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	bool bMini;
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	int nRet;
};

struct CSMarketDBID				// CS_TRADE / CS_MARKET_INTERRUPT, CS_MARKET_CALCULATION
{
	int nMarketDBID;
};

struct SCMarketResult			// SC_TRADE / SC_MARKET_REGISTER, SC_MARKET_CALCULATION
{
	int nMarketDBID;
	int nRet;
};

struct SCMarketInterrupt			// SC_TRADE / SC_MARKET_INTERRUPT
{
	int nMarketDBID;
	short wRegisterItemCount;		// Number of registered items
	int nRet;
};

struct TMarketCalculationInfo
{
	int nMarketDBID;					// market table db ID

	int nItemID;
	short wCount;		// Count
	USHORT wDur;			// Durability
	int nRandomSeed;
	char cLevel;		// Level
	char cPotential;	// Potential
	char cOption;
	char cSealCount;	// Seal count

	int nPrice;			// Price
	int nUnitPrice;		// Unit price
	__time64_t tBuyDate;	// Sale date
	char cPayMethodCode;	// PayMethodCode (DB data) 1-Gold, 3-Petal
	char cPotentialMoveCount;
};

struct SCMarketCalculationList			// SC_TRADE / SC_MARKET_CALCULATIONLIST
{
	int nRetCode;
	char cMarketCount;
	TMarketCalculationInfo MarketInfo[MYMARKETMAX];
};

struct SCMarketCalculationAll			// SC_TRADE / SC_MARKET_CALCULATIONALL
{
	int nRet;
};

struct SCNotifyMarket					// SC_TRADE / SC_MARKET_NOTIFY
{
	int nItemID;				// Set ItemID if sold
	short wCalculationCount;	// Settlement count
};

struct SCMarketPetalBalance				// SC_TRADE / SC_MARKET_PETALBALANCE
{
	int nRetCode;
	int nPetalBalance;				// Petal
};

struct TMarketPrice
{
	bool bFlag;					// Whether registered

	int nAvgPrice;
	int nMinPrice;
	int nMaxPrice;

	BYTE cPayMethodCode;		// 1 = Game money, 3 = Petal
};

struct CSMarketPrice			// CS_MARKET_PRICE
{
	int nMarketDBID;
	int nItemID;
	BYTE cLevel;
	BYTE cOption;
};

struct SCMarketPrice			// SC_MARKET_PRICE
{
	int nRetCode;

	int nMarketDBID;
	TMarketPrice ItemPrices[2];		// Index 0: Game money, 1: Petal
};

struct CSMarketMiniList						// CS_TRADE / CS_MARKETMINILIST
{
	int nItemID;
	char cPayMethodCode;					// 0-Mixed, 1-Coin, 3-Petal
};

typedef SCMarketList SCMarketMiniList;		// SC_TRADE / SC_MARKETMINILIST


// Personal trade
struct CSExchangeRequest		// CS_TRADE / CS_EXCHANGE_REQUEST // Trade request
{
	UINT nReceiverSessionID;	// User to trade with
	bool bCancel;				// Whether to cancel again
};

struct SCExchangeRequest		// SC_TRADE / SC_EXCHANGE_REQUEST // Trade request
{
	UINT nSenderSessionID;		// User who requested trade
	int nRet;
};

struct SCExchangeReject			// SC_TRADE / SC_EXCHANGE_REJECT // Cancel trade
{
	UINT nSessionID;
	int nRetCode;
};

struct CSExchangeAccept			// CS_TRADE / CS_EXCHANGE_ACCEPT // Accept trade
{
	UINT nSenderSessionID;		// User who requested trade
	bool bAccept;				// Acceptance status
};

struct SCExchangeStart			// SC_TRADE / SC_EXCHANGE_START // Start trade
{
	UINT nTargetSessionID;		// Opponent
	int nRet;
};

struct CSExchangeAddItem		// CS_TRADE / CS_EXCHANGE_ADDITEM, // Register item
{
	BYTE cExchangeIndex;		// Trade window index
	BYTE cInvenIndex;
	short wCount;
	INT64 biItemSerial;
};

struct SCExchangeAddItem		// SC_TRADE / SC_EXCHANGE_ADDITEM // Register item
{
	UINT nSessionID;
	BYTE cExchangeIndex;		// Trade window index
	TItemInfo ItemInfo;
	int nRet;
};

struct CSExchangeDeleteItem		// CS_TRADE / CS_EXCHANGE_DELETEITEM, // Unregister item
{
	BYTE cExchangeIndex;		// Trade window index
};

struct SCExchangeDeleteItem		// SC_TRADE / SC_EXCHANGE_DELETEITEM
{
	UINT nSessionID;			// User corresponding to the trade window
	BYTE cExchangeIndex;		// Trade window index
	int nRet;
};

struct CSExchangeAddCoin		// CS_TRADE / CS_EXCHANGE_ADDCOIN, // Register coin
{
	INT64 nCoin;	// Unlikely to exceed 2.1 billion
};

struct SCExchangeAddCoin		// SC_TRADE / SC_EXCHANGE_ADDCOIN
{
	UINT nSessionID;			// User corresponding to the trade window
	INT64 nCoin;	// Unlikely to exceed 2.1 billion
	int nRet;
};

struct CSExchangeConfirm		// CS_TRADE / CS_EXCHANGE_CONFIRM
{
	char cType;		// eExchangeButton
};

struct SCExchangeConfirm		// SC_TRADE / SC_EXCHANGE_CONFIRM
{
	char cType;		// eExchangeButton
	UINT nSessionID;
};

struct SCExchangeComplete		// SC_TRADE / SC_EXCHANGE_COMPLETE
{
	int nRet;
};

#if defined(PRE_SPECIALBOX)
// specialbox
struct TSpecialBoxInfo			// CS_TRADE / CS_SPECIALBOX_LIST
{
	int nEventRewardID;
	BYTE cReceiveTypeCode;		// SpecialBox::ReceiveTypeCode
	BYTE cTargetTypeCode;		// SpecialBox::TargetTypeCode
	BYTE cWorldID;
	BYTE cTargetClassCode;
	BYTE cTargetMinLevel;
	BYTE cTargetMaxLevel;
	__time64_t tReserveSendDate;
	__time64_t tExpirationDate;
	//WCHAR wszEventName[MAILTITLELENMAX];
	WCHAR wszSenderName[NAMELENMAX];
	WCHAR wszContent[MAILTEXTLENMAX];
	INT64 biRewardCoin;
	bool bSystemSendFlag;
	char cEventRewardTypeCode;
};

struct SCSpecialBoxList			// SC_TRADE / SC_SPECIALBOX_LIST
{
	int nRet;
	char cCount;
	TSpecialBoxInfo BoxInfo[SpecialBox::Common::ListMax];
};

struct CSSpecialBoxItemList		// CS_TRADE / CS_SPECIALBOX_ITEMLIST
{
	int nEventRewardID;
};

struct TSpecialBoxItemInfo
{
	bool bCashItem;
	TItem RewardItem;
};

struct SCSpecialBoxItemList		// SC_TRADE / SC_SPECIALBOX_ITEMLIST
{
	int nRet;
	INT64 biRewardCoin;
	BYTE cCount;
	TSpecialBoxItemInfo BoxItem[SpecialBox::Common::RewardSelectMax];
};

struct CSReceiveSpecialBoxItem	// CS_TRADE / CS_SPECIALBOX_RECEIVEITEM
{
	int nEventRewardID;
	int nItemID;	// Set only for selectable rewards
};

struct SCReceiveSpecialBoxItem	// SC_TRADE / SC_SPECIALBOX_RECEIVEITEM
{
	int nRet;
};

struct SCNotifySpecialBox		// SC_TRADE / SC_SPECIALBOX_NOTIFY
{
	int nNotifyCount;
	bool bNew;
};
#endif	// #if defined(PRE_SPECIALBOX)

//Community
//--------------------------------------------------------------------------------------------------
//	FRIEND (CS_FRIEND, SC_FRIEND)	
//--------------------------------------------------------------------------------------------------

//Client -> Server
struct CSFriendAddGroup			//CS_FRIEND / CS_FRIEND_GROUP_CREATE
{
	BYTE cNameLen;
	WCHAR wszBuf[FRIEND_GROUP_NAMELENMAX];
};

struct CSFriendDelGourp			//CS_FRIEND / CS_FRIEND_GROUP_DELETE
{
	UINT nGroupDBID;
};

struct CSFriendGroupUpdate	//CS_FRIEND / CS_FRIEND_GROUP_STATEUPDATE
{
	UINT nGroupDBID;
	BYTE cNameLen;
	WCHAR wszBuf[FRIEND_GROUP_NAMELENMAX];
};

struct CSFriendAdd					//CS_FRIEND / CS_FRIEND_ADD
{
	UINT nGroupDBID;
	BYTE cNameLen;
	WCHAR wszBuf[NAMELENMAX];
};

struct CSFriendDelete				//CS_FRIEND / CS_FRIEND_DELETE
{
	INT64 biFriendCharacterDBID;
};

struct CSFriendUpdate				//CS_FRIEND / CS_FRIEND_STATEUPDATE
{
	INT64 biFriendCharacterDBID;
	UINT nGroupDBID;
	BYTE cMemoLen;
	WCHAR wszBuf[FRIEND_MEMO_LEN_MAX];
};

struct CSFriendDetailInfo					//CS_FRIEND / CS_FRIEND_INFO
{
	INT64 biFriendCharacterDBID;
};

//Server -> Client
struct SCFriendGroupList			//SC_FRIEND / SC_FRIEND_GROUP_LIST
{
	BYTE cGroupCount;
	UINT nGroupDBID[FRIEND_GROUP_MAX];
	BYTE cGroupNameLen[FRIEND_GROUP_MAX];
	WCHAR wszBuf[FRIEND_GROUP_NAMELENMAX * FRIEND_GROUP_MAX];
};

struct TFriendInfo
{
	UINT nGroupDBID;
	INT64 biFriendCharacterDBID;
	TCommunityLocation Location;
	WCHAR wszFriendName[NAMELENMAX];
};

struct SCFriendList				//SC_FRIEND / SC_FRIEND_LIST
{
	int nRetCode;
	BYTE cCount;
	TFriendInfo Info[FRIEND_MAXCOUNT];
};

struct TFriendLocation
{
	UINT nGroupDBID;
	INT64 biFriendCharacterDBID;
	TCommunityLocation Location;
};

struct SCFriendLocationList		//SC_FRIEND / SC_FRIEND_LOCATIONLIST
{
	BYTE cCount;
	TFriendLocation FriendLocation[FRIEND_MAXCOUNT];
};

struct SCFriendGroupAdded		//SC_FRIEND / SC_FRIEND_GROUP_ADDED
{
	int nRetCode;
	UINT nGroupDBID;
	BYTE cNameLen;
	WCHAR wszBuf[FRIEND_GROUP_NAMELENMAX];
};

struct SCFriendAdded				//SC_FRIEND / SC_FRIEND_ADDED
{
	int nRetCode;
	TFriendInfo Info;
};

struct SCFriendGroupDeleted	//SC_FRIEND / SC_FRIEND_GROUP_DELETED
{
	UINT nGroupDBID;
	int nRetCode;
};


struct SCFriendGroupUpdated			//SC_FRIEND / SC_FRIEND_GROUP_UPDATED
{
	int nRetCode;
	UINT nGroupDBID;
	BYTE cNameLen;
	WCHAR wszBuf[FRIEND_GROUP_NAMELENMAX];
};

struct SCFriendDeleted				//SC_FRIEND / SC_FRIEND_ADDED_DELETED
{
	int nRetCode;
	BYTE cCount;
	INT64 biFriendCharacterDBID[FRIEND_MAXCOUNT];
};

struct SCFriendUpdated				//SC_FRIEND / SC_FRIEND_UPDATED
{
	int nRetCode;
	UINT nGroupDBID;					//DB ID of moved group
	BYTE cCount;
	INT64 biFriendCharacterDBID[FRIEND_MAXCOUNT];
};

struct SCFriendDetailInfo			//SC_FRIEND / SC_FRIEND_INFO
{
	INT64 biFriendCharacterDBID;
	UINT nGroupDBID;
	USHORT nClass;
	USHORT nJob;
	BYTE cCharacterLevel;
	TCommunityLocation Location;
};

struct SCFriendResult						//SC_FRIEND / SC_FRIEND_RESULT
{
	int nRet;
};

struct SCFriendAddNotice		//SC_FRIEND / SC_FRIEND_ADDNOTICE
{
	WCHAR wszName[NAMELENMAX];
};

//--------------------------------------------------------------------------------------------------
//	SC_ISOLATE (CS_BLACKLIST_GET, SC_GETGUILD)
//--------------------------------------------------------------------------------------------------
// Isolation
struct CSIsolateAdd			//CS_ISOLATE / CS_ISOLATE_ADD
{
	WCHAR wszIsolateName[NAMELENMAX];
};

struct CSIsoLateDelete		//CS_ISOLATE / CS_ISOLATE_DELETE
{
	WCHAR wszIsolateName[NAMELENMAX];
};

struct SCIsolateList		//SC_ISOLATE / SC_ISOLATE_LIST
{
	BYTE cCount;
	BYTE cNameLen[ISOLATELISTMAX];
	WCHAR wszIsolateName[NAMELENMAX * ISOLATELISTMAX];
};

struct SCIsolateAdd			//SC_ISOLATE / SC_ISOLATE_ADD
{
	WCHAR wszIsolateName[NAMELENMAX];
};

struct SCIsolateDelete		//SC_ISOLATE / SC_ISOLATE_DELETE
{
	BYTE cCount;
	BYTE cNameLen[ISOLATELISTMAX];
	WCHAR wszIsolateName[NAMELENMAX * ISOLATELISTMAX];
};

struct SCIsolateResult		//SC_ISOLATE / SC_ISOLATE_RESULT
{
	int nRetCode;
};


//--------------------------------------------------------------------------------------------------
//	GUILD (CS_GUILD, SC_GUILD)
//--------------------------------------------------------------------------------------------------

// [CL -> GA/VI]

// CS_GUILD / CS_CREATEGUILD // Request to create guild
struct CSCreateGuild
{
	WCHAR wszGuildName[GUILDNAME_MAX];		// Guild name
};

// CS_GUILD / CS_INVITEGUILDMEMBREQ // Guild member invite request
struct CSInviteGuildMemberReq
{
	WCHAR wszToCharacterName[NAMELENMAX];		// Character name to invite
};

// CS_GUILD / CS_INVITEGUILDMEMBACK // Guild member invite response
struct CSInviteGuildMemberAck
{
	TGuildUID GuildUID;		// Guild UID
	UINT nFromSessionID;		// Inviter session ID
	bool bAccept;				// Whether accepted

	UINT nFromAccountID;		// Inviter UINT
};

// CS_GUILD / CS_EXILEGUILDMEMB // Request to expel guild member
struct CSExileGuildMember
{
	UINT nAccountDBID;		// Account DBID
	INT64 nCharacterDBID;		// Character DBID
};

// CS_GUILD / CS_CHANGEGUILDINFO // Request to change guild info
struct CSChangeGuildInfo
{
	BYTE btGuildUpdate;		// Guild info change type (eGuildUpdateType)
	int Int1;
	int Int2;
	int Int3;
	int Int4;
	INT64 Int64;
	WCHAR Text[GUILDPARAMTEXT_MAX+1];
};

// CS_GUILD / CS_CHANGEGUILDMEMBINFO // Request to change guild member info
struct CSChangeGuildMemberInfo
{
	BYTE btGuildMemberUpdate;		// Guild member info update type (eGuildMemberUpdateType)
	UINT nChgAccountDBID;					// Account DBID to modify (enter 0 if modifying own info)
	INT64 nChgCharacterDBID;					// Character DBID to modify (enter 0 if modifying own info)
	int Int1;
	int Int2;
	INT64 Int64;
	WCHAR Text[GUILDPARAMTEXT_MAX+1];
};

// CS_GUILD / CS_GETGUILDHISTORYLIST // Request guild history list
struct CSGetGuildHistoryList
{
	INT64 biIndex;	// Guild history list index (if 0, check from most recent?)
	bool bDirection;				// Direction to get list (0: before index / 1: after index)
};

// CS_GUILD / CS_GETGUILDINFO // Request guild info
struct CSGetGuildInfo
{
	bool bNeedMembList;	// Whether guild member list is also needed
};

// SC_GUILD / SC_CREATEGUILD // Guild creation result
struct SCCreateGuild
{
	UINT nSessionID;				// Session ID
	TGuildUID GuildUID;			// Guild UID (ignored on failure)
	TGuild Info;					// Guild info
	UINT nAccountDBID;			// Account DBID of the founder
	INT64 nCharacterDBID;			// Character DBID of the founder
	TP_JOB nJob;					// Inviter character class
	CHAR cLevel;					// Inviter character level
	TCommunityLocation Location;	// Current location (login status is also checked via this member's state)
	int iErrNo;					// Result (NOERROR : success / others : fail)
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
};

// SC_GUILD / SC_DISMISSGUILD // Guild disband result
struct SCDismissGuild
{
	UINT nSessionID;		// Session ID
	int iErrNo;			// Result (NOERROR : success / others : fail)
	TGuildUID GuildUID;	// Guild UID (ignored on failure)
};

// SC_GUILD / SC_INVITEGUILDMEMBREQ // Guild member invite request
struct SCInviteGuildMemberReq
{
	TGuildUID GuildUID;		// Guild UID
	UINT nFromAccountDBID;	// Requesting user account DBID
	UINT nFromSessionID;		// Requesting session ID
	UINT nToAccountDBID;		// Inviter user account DBID
	UINT nToSessionID;		// Inviter session ID
	int iErrNo;				// Result (NOERROR : success / others : fail)
	WCHAR wszFromCharacterName[NAMELENMAX];	// Requesting character name (P.S. When sent as an error to the requesting user, this becomes the invited opponent's name)
	WCHAR wszGuildName[GUILDNAME_MAX];		// Guild name
};

// SC_GUILD / SC_INVITEGUILDMEMBACK // Guild member invite result
struct SCInviteGuildMemberAck
{
	UINT nToAccountDBID;			// Inviter account DBID
	INT64 nToCharacterDBID;		// Inviter character DBID
	UINT nFromAccountDBID;		// Requesting account DBID
	INT64 nFromCharacterDBID;		// Requesting character DBID
	TGuildUID GuildUID;			// Guild UID (ignored on failure)
	TP_JOB nJob;					// Inviter character class
	CHAR cLevel;					// Inviter character level
	TCommunityLocation Location;	// Current location (login status is also checked via this member's state)
	int iErrNo;					// Result (NOERROR : success / others : fail)
	WCHAR wszToCharacterName[NAMELENMAX];	// Inviter character name
};

// SC_GUILD / SC_LEAVEGUILDMEMB // Guild member leave result
struct SCLeaveGuildMember
{
	UINT nAccountDBID;		// Withdrawn account DBID
	INT64 nCharacterDBID;		// Deleted character DBID
	TGuildUID GuildUID;		// Guild UID (ignored on failure)
	int iErrNo;				// Result (NOERROR : success / others : fail)
#ifdef PRE_ADD_BEGINNERGUILD
	bool bGraduateBeginnerGuild;		//Graduate from beginner guild
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
};

// SC_GUILD / SC_EXILEGUILDMEMB // Guild member kick result
struct SCExileGuildMember
{
	UINT nAccountDBID;		// Account DBID of the kicker
	INT64 nCharacterDBID;		// Character DBID of the kicker
	TGuildUID GuildUID;		// Guild UID (ignored on failure)
	int iErrNo;				// Result (NOERROR : success / others : fail)
};

// SC_GUILD / SC_CHANGEGUILDINFO // Guild info change result
struct SCChangeGuildInfo
{
	UINT nAccountDBID;				// Requesting account DBID (usually not critical, used for reference only)
	INT64 nCharacterDBID;				// Requesting character DBID (usually not critical, used for reference only)
	BYTE btGuildUpdate;		// Guild info change type (eGuildUpdateType)
	int Int1;
	int Int2;
	int Int3;
	int Int4;
	INT64 Int64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
	TGuildUID GuildUID;		// Guild UID (ignored on failure)
	int iErrNo;				// Result (NOERROR : success / others : fail)
};

// SC_GUILD / SC_CHANGEGUILDMEMBINFO // Guild member info change result
struct SCChangeGuildMemberInfo
{
	UINT nReqAccountDBID;	// Requesting account DBID
	INT64 nReqCharacterDBID;	// Requesting character DBID
	UINT nChgAccountDBID;	// Target account DBID to modify (differs from requester when guild master edits a guild member's info)
	INT64 nChgCharacterDBID;	// Target character DBID to modify (differs from requester when guild master edits a guild member's info)
	BYTE btGuildMemberUpdate;		// Guild member info update type (eGuildMemberUpdateType)
	int Int1;
	int Int2;
	INT64 Int64;
	WCHAR Text[GUILDPARAMTEXT_MAX+1];
	TGuildUID GuildUID;		// Guild UID (ignored on failure)
	int iErrNo;				// Result (NOERROR : success / others : fail)
};

// SC_GUILD / SC_GETGUILDHISTORYLIST // Guild history list response
struct SCGetGuildHistoryList
{
	TGuildUID GuildUID;			// Guild UID
	INT64 biIndex;	// Guild history list index (if 0, check from most recent?)
	bool bDirection;				// Direction to get list (0: before index / 1: after index)
	int nCurCount;				// Count from first guild history index to input index
	int nTotCount;				// Total guild history count
	int iErrNo;					// Result (NOERROR : success / others : fail)
	int nCount;					// Guild history list count (ignored on failure)
	TGuildHistory HistoryList[GUILDHISTORYLIST_MAX];	// Guild history list (ignored on failure)
};

// SC_GUILD / SC_GETGUILDINFO // Guild info response
struct SCGetGuildInfo
{
	int iErrNo;	// Result (NOERROR : success / others : fail)
 	TGuild Info;	// Guild info (ignored on failure)	
	bool bNotRecruitMember;		// Check guild recruitment board join approval status (false: can join, true: cannot join)
};
struct SCGetGuildMember
{	
	int iErrNo;	// Result (NOERROR : success / others : fail)
	int nPage;	// Guild member page info
	bool bEndMember;	// Check if it is the last one
	int nCount;	// Guild member info count (ignored on failure)
	TGuildMember MemberList[SENDGUILDMEMBER_MAX];	// Guild member info

};

// SC_GUILD / SC_CHANGEGUILDSELFVIEW
struct SCChangeGuildSelfView
{
	UINT nSessionID;				// Session ID
	TGuildSelfView GuildSelfView;	// Guild visual info (personal)
};

// SC_GUILD / SC_GUILDMEMBLOGINLIST
struct SCGuildMemberLoginList
{
	TGuildUID GuildUID;			// Guild UID
	int nCount;					// Number of logged-in guild members
	INT64 List[GUILDSIZE_MAX];	// Logged-in guild member list
};


struct SCOpenGuildWare				// SC_GUILD	/ SC_OPEN_GUILDWARE
{
	int					nError;							// Error value handling
	int					nResult;						// 0: include in list, 1: exclude from list
	short				wWareSize;						// Storage size
	
	USHORT				nTakeWareItemCount;				// Number of storage item withdrawals (per day)
	INT64				nWithdrawCoin;					// Amount of gold withdrawn today
	INT64				nGuildCoin;						// Guild coin

	__time64_t			VersionDate;					// Warehouse version

	BYTE				cWareCount;						// Wear item count
	TItemInfo			WareList[GUILD_WAREHOUSE_MAX];	// Wear list
};

struct CSGetGuildWareHistory		// CS_GUILD / CS_GET_GUILDWARE_HISTORY // Guild warehouse usage history
{
	int					nIndex;							// History request index
};

struct SCGetGuildWareHistory		// SC_GUILD / SC_GET_GUILDWARE_HISTORY
{
	int					nCurrCount;
	int					nTotalCount;
	int					nErrNo;
	int					nCount;
	TGuildWareHistory	HistoryList[GUILD_WAREHOUSE_HISTORYLIST_MAX];// History list
};

// SC_GUILD / SC_GUILDMEMBLOGINLIST
struct SCChangeGuildName
{	
	WCHAR wszGuildName[GUILDNAME_MAX];		// Guild name (assumes name does not change after guild creation)
};

struct SCChangeGuildMark			// SC_GUILD / SC_CHANGEGUILDMARK
{
	short wMark;
	short wMarkBG;
	short wMarkBorder;
};

struct SCGuildLevelUp				// SC_GUILD / SC_GUILDLEVELUP
{
	int nLevel;
};

struct SCUpdateGuildExp				// SC_GUILD / SC_UPDATEGUILDEXP
{
	int nError;				// Result (ERROR_NONE : success / ERROR_GUILD_DAILYLIMIT : fail)
	BYTE cPointType;				
	int nPointValue;
	int nGuildExp;
	INT64 biCharacterDBID;
	int nMissionID;
};

// SC_GUILD_GET_REWARDITEM
struct SCGetGuildRewardItem
{
	TGuildRewardItem GuildRewardItem[GUILDREWARDEFFECT_TYPE_CNT];
};
struct CSBuyGuildRewardItem
{
	int nItemID;
};
struct SCBuyGuildRewardItem
{
	int	iRet;
	int nItemID;
};
struct SCAddGUildRewardItem
{
	TGuildRewardItem GuildRewardItem;
};
struct SCExtendGuildSize
{
	short	nGuildSize;
};
struct CSPlayerGuildInfo
{
	int nSessionID;
};
struct SCPlayerGuildInfo
{
	UINT nSessionID;							// Opponent session ID
	WCHAR wszMasterName[NAMELENMAX];			// Guild master character name
	WCHAR wszGuildName[GUILDNAME_MAX];		// Guild name
	int nGuildLevel;							// Guild level
	int nMemberSize;
	int nGuildSize;
	TGuildRewardItem GuildRewardItem[GUILDREWARDEFFECT_TYPE_CNT];
};

//SC_GUILD / SC_EXTEND_GUILDWARESIZE
struct SCExtendGuildWare
{
	short wTotalSize;
};

// SC_GUILD / SC_ENROLL_GUILDWAR
struct SCEnrollGuildWar
{
	int iErrorNo;
	short wScheduleID;
	BYTE cTeamColorCode;
};

// SC_GUILD / SC_CHANGE_WAREVENT
struct SCGuildWarEvent
{
	short wScheduleID;		// Tier
	char cEventStep;		// Event step: GUILDWAR_STEP_NONE ~ GUILDWAR_STEP_END
	char cEventType;		// GUILDWAR_EVENT_START (start) / GUILDWAR_EVENT_END (end)
};

// SC_GUILD / SC_GUILDWAR_FESTIVAL_POINT
struct SCGuildWarFestivalPoint
{
	INT64 biGuildWarFestivalPoint;
};

// SC_GUILD / SC_GUILDWAR_TRIAL_RESULT_OPEN
struct SGuildWarRankingInfo	
{
	short wRanking;							// Guild rank
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	WCHAR wszGuildMasterName[NAMELENMAX];	// Guild master name
	short wCurGuildUserCount;				// Current guild member count
	short wMaxGuildUserCount;				// Max guild members
	int nTotalPoint;						// Total score acquired
	UINT nGuildDBID;						// Guild DBID
};

struct SGuildWarMissionGuildRankingInfo	//Guild ranking by category
{
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	WCHAR wszGuildMasterName[NAMELENMAX];	// Guild master name
	int nTotalPoint;						// Total score acquired
};

struct SMyGuildWarMissionGuildRankingInfo // Our guild ranking by category
{
	short wRanking;							// Ranking by category
	int nTotalPoint;						// Total score acquired
};

struct SGuildWarMissionRankingInfo
{
	WCHAR wszCharName[NAMELENMAX];			// 1st place name
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	int nTotalPoint;						// Total score acquired
};

struct SMyGuildWarMissionRankingInfo
{
	short wRanking;							// Rank
	int nTotalPoint;						// Total score acquired
};

struct SGuildWarDailyAward
{
	WCHAR wszCharName[NAMELENMAX];			// Award recipient character name
	TGuildUID nGuildUID;					// UID of the winning guild;
};

struct SCGuildWarTrialResult
{
	// Guild festival result
	int nBluePoint;			// Final blue team score
	int nRedPoint;			// Final red team score
	bool bGuildWar;			// Guild war participation flag (if false, personal and our guild info below are not included in packet.)	
	bool bReward;			// Whether reward was claimed
	// Finals-qualifying guild
	SGuildWarRankingInfo	sGuildWarRankingInfo[GUILDWAR_FINALS_TEAM_MAX];	

	// Guild ranking by category (all)
	SGuildWarMissionGuildRankingInfo sGuildWarMissionGuildRankingInfo[GUILDWAR_RANKINGTYPE_MAX];	// Overall guild ranking
	// Individual ranking by category (all)
	SGuildWarMissionRankingInfo sGuildWarMissionRankingInfo[GUILDWAR_RANKINGTYPE_MAX];			// Individual ranking
	// Special award
	// For things like subjugation awards, just display the individual ranking number as-is.
	SGuildWarDailyAward sGuildWarDailyAward[GUILDWAR_DAILY_AWARD_MAX];							// Daily reward	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// My info and my guild info (omitted if not in a guild or not participating in Guild War).
	int nFestivalPoint;		// Base score 
	int nFestivalPointAdd;	// Bonus points from awards
	// Our guild ranking info (overall rank)
	SMyGuildWarMissionGuildRankingInfo sMyGuildWarRankingInfo;
	// Guild ranking by category (our guild)
	SMyGuildWarMissionGuildRankingInfo sMyGuildWarMissionGuildRankingInfo[GUILDWAR_RANKINGTYPE_MAX]; // Our guild rank	
	// My ranking by category
	SMyGuildWarMissionRankingInfo sMyGuildWarMissionRaningInfo[GUILDWAR_RANKINGTYPE_MAX];			 // My rank	
};

// SC_GUILD / SC_GUILDWAR_STATUS_OPEN
struct SCGuildWarStatus
{	
	int nBluePoint;			// Blue team score	
	int nRedPoint;			// Red team score	
	int nMyPoint;			// My score
	int nMyGuildPoint;		// Current guild points
	int nRankingCount;		// Ranking count.. this many items are sent.
	SGuildWarRankingInfo	sGuildWarPointTrialRanking[GUILDWAR_TRIAL_POINT_TEAM_MAX];
};
// CS_GUILD / CS_GUILDWAR_VOTE
struct CSGuildWarVote
{
	TGuildUID GuildUID;		// Guild UID to vote for (World ID + Guild DBID)
};
// SC_GUILD / SC_GUILDWAR_VOTE
struct SCGuildWarVote
{
	int nRetCode;			// Vote result
	int nAddFestivalPoint;	// Acquired festival points
};
// SC_GUILD / SC_GUILDWAR_TOURNAMENT_INFO_OPEN
struct SGuildTournamentInfo
{
	TGuildUID GuildUID;						// Guild UID (World ID + Guild DBID)
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	char cMatchTypeCode;					// Final round: 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	bool bWin;								// Whether won or lost	
	short wGuildMark;						// Guild mark emblem
	short wGuildMarkBG;						// Guild mark background
	short wGuildMarkBorder;					// Guild mark border
};
struct SCGuildTournamentInfo
{
	SGuildTournamentInfo TournamentInfo[GUILDWAR_FINALS_TEAM_MAX];
	char cMatchTypeCode;					// Current round in progress: 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	__time64_t tStartTime;					// If this is 0, it is currently in progress; if it has a value, it starts at that time.
	bool bPopularityVote;					// Enable/disable popularity vote
};
// SC_GUILD / SC_GUILDWAR_VOTE_TOP
struct SCGuildWarVoteTop
{
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	int nVoteCount;							// Total votes received
};
// SC_GUILD / SC_GUILDWAR_COMPENSATION
struct SCGuildWarCompensation
{
	int nRetCode;				// Reward claim result
};
// SC_GUILD / SC_GUILDWAR_PRE_WIN
struct SCGuildWarPreWin
{
	bool bPreWin;				// true if our guild is the winning guild, false to clear winning guild status.
};
// CS_GUILD / CS_GUILDWAR_WIN_SKILL
struct CSGuildWarWinSkill
{
	int nSkillID;				// Cast skill ID
};
// SC_GUILD / SC_GUILDWAR_WIN_SKILL
struct SCGuildWarWinSkill
{
	int nRetCode;
	DWORD dwCoolTime;			// Remaining cooldown
};
// SC_GUILD / SC_GUILDWAR_USER_WINSKILL
#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || defined(_CLIENT)
struct SCGuildWarUserWinSkill
{
	TGuildUID GuildUID;			// GuildUID of the skill caster
	EtVector3 vPos;				// Coordinates of the skill caster.
	int nSkillID;				// Skill ID
};
#endif //#if defined(_GAMESERVER) || defined(_VILLAGESERVER) || defined(_CLIENT)

// SC_GUILD / SC_GUILDWAR_EVENT_TIME
struct SGuildWarTime
{
	__time64_t tBeginTime;
	__time64_t tEndTime;
};
struct SCGuildWarEventTime
{	
	bool bFinalProgress;
	// Full guild war schedule
	SGuildWarTime tGuildWarTime[GUILDWAR_STEP_END-1]; // Subtract 1 from each in eGuildWarStepType..
	// Finals round number
	SGuildWarTime tFinalPartTime[GUILDWAR_FINALPART_MAX-1]; //Subtract 1 from each in eGuildFinalPart..
};

// SC_GUILD / SC_GUILDWAR_TOURNAMENT_WIN
struct SCGuildWarTournamentWin
{
	// Win round count
	char cMatchTypeCode;					// Current round in progress: 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	// Winning guild name
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
};

//--------------------------------------------------------------------------------------------------
//	PVP (CS_PVP, SC_PVP)
//--------------------------------------------------------------------------------------------------

//blondy 
//Container to use on client
struct sRoomInfo
{
	BYTE	cGameMode;		// PvPCommon::GameMode
	BYTE	cVSMode;		// PvPCommon::VSMode
	WCHAR	RoomName[PvPCommon::TxtMax::RoomName];
	BYTE	cRoomState;		// Room state
	BYTE    cPlayerNum;		//Current member count
	BYTE    cMaxPlayerNum;	//Max players
	UINT    nMapIndex;      //Map index
	UINT    nPVPIndex;      //Room index
	bool	bInGameJoin;	//Join-in-progress allowed
    BYTE    cMinLevel; 
	BYTE    cMaxLevel;    
	UINT    uiObjective;      //Objective
	bool	bDropItem;		//Allow drop items
	int		nEventID;
	BYTE	cShowHP;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	BYTE	cRoomType;		//PvPCommon::RoomType
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER

	UINT nGuildDBID[PvPCommon::TeamIndex::Max];
#ifdef PRE_MOD_PVPOBSERVER
	bool bExtendObserver;
#endif // PRE_MOD_PVPOBSERVER
#ifdef PRE_WORLDCOMBINE_PVP
	int nWorldPvPRoomType;
#endif // PRE_WORLDCOMBINE_PVP
#ifdef PRE_PVP_GAMBLEROOM
	BYTE cGambleType;
	int nGamblePrice;
#endif // PRE_PVP_GAMBLEROOM
};
//blondyend

struct CSPVP_CREATEROOM
{
	UINT	uiMapIndex;
	UINT	uiGameModeTableID;
	UINT	uiSelectWinCondition;
	UINT	uiSelectPlayTimeSec;
	BYTE	cMaxUser;
	BYTE	cMinUser;
	BYTE	cRoomNameLen;
	BYTE	cRoomPWLen;
	BYTE	cMinLevel;
	BYTE	cMaxLevel;
	USHORT	unRoomOptionBit;	// PvPCommon::RoomOption Bit
	UINT    uiEventItemID;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	BYTE cRoomType;		//PvPCommon::RoomType
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_MOD_PVPOBSERVER
	bool bExtendObserver;
#endif		//#ifdef PRE_MOD_PVPOBSERVER
#if defined( PRE_WORLDCOMBINE_PVP )
	int nWorldPvPRoomType;	
#endif
	WCHAR	wszBuf[PvPCommon::TxtMax::RoomName + PvPCommon::TxtMax::RoomPW];
};

struct SCPVP_CREATEROOM
{
	int					nRetCode;
	UINT				uiPvPIndex;
	CSPVP_CREATEROOM	sCSPVP_CREATEROOM;
};

struct CSPVP_MODIFYROOM
{
	CSPVP_CREATEROOM	sCSPVP_CREATEROOM;
};

struct SCPVP_MODIFYROOM
{
	int					nRetCode;
	CSPVP_MODIFYROOM	sCSPVP_MODIFYROOM;
};

struct SCPVP_LEAVEROOM
{
	short								nRetCode;
	UINT								uiLeaveUserSessionID;
	PvPCommon::LeaveType::eLeaveType	eType;
};

struct CSPVP_CHANGECAPTAIN
{
	PvPCommon::CaptainType::eCode Type;
	UINT	uiNewCaptainSessionID;
};

struct CSPVP_ROOMLIST
{
	UINT	uiPage;
	BYTE	cRoomSortType;		// See PvPCopmmon::RoomSortType
	BYTE	cIsAscend;			// Ascending:1 Descending:0
	UINT	uiSortData;			// Additional info needed depending on cRoomSortType
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	BYTE cChannelType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
};

struct SCPVP_ROOMLIST
{
	short					nRetCode;
	BYTE					cRoomCount;
	UINT					uiMaxPage;
	PvPCommon::RoomInfoList	RoomInfoList;
};

struct CSPVP_WAITUSERLIST
{
	PvPCommon::WaitUserList::SortType::eSortType	SortType;
	bool											bIsAscend;	// Ascending:true Descending:false
	USHORT											unPage;	// ZeroBase
};

struct SCPVP_WAITUSERLIST
{
	short								nRetCode;
	BYTE								cUserCount;
	USHORT								unMaxPage;
	PvPCommon::WaitUserList::Repository	Repository;
};

struct CSPVP_JOINROOM
{
	UINT	uiPvPIndex;
	bool	bIsObserver;		// Spectate
	UINT nGuildDBID;			//If participating in guild war
	BYTE	cRoomPWLen;
	WCHAR	wszRoomPW[PvPCommon::TxtMax::RoomPW];
};

struct SCPVP_JOINROOM
{
	short					nRetCode;
	PvPCommon::RoomInfo		RoomInfo;				// Room info
	BYTE					cUserCount;				// User count
	PvPCommon::UserInfoList	UserInfoList;
};

struct SCPVP_JOINUSER
{
	PvPCommon::UserInfo		UserInfo;
};

struct CSPVP_READY
{
	BYTE	cReady;
};

struct CSPVP_START
{
	USHORT	unCheck;		// See PvPCommon::StartCheat
};

struct CSPVP_CHANGETEAM
{
	USHORT	usTeam;			// PvPCommon::Team
};

struct SCPVP_CHANGETEAM
{
	short	nRetCode;
	UINT	uiSessionID;
	USHORT	usTeam;			// PvPCommon::Team
	char cTeamSlotIndex;
};

struct SCPVP_USERSTATE
{
	UINT	uiSessionID;
	UINT	uiUserState;		// PvPCommon::UserState
};

struct SCPVP_START
{
	short	nRetCode;
};

struct SCPVP_STARTMSG
{
	BYTE	cSec;
};

struct SCPVP_ROOMSTATE
{
	UINT	uiRoomState;	
};

struct SCPVP_MODE_STARTTICK
{
	UINT	uiStartTick;
	UINT	uiCurTick;
};

struct SCPVP_FINISH_PVPMODE
{
	UINT	uiWinTeam;		// See PvPCommon::Team
	PvPCommon::FinishReason::eCode	Reason;		// See PvPCommon::FinishReason
	UINT	uiATeamScore;
	UINT	uiBTeamScore;
#if defined(PRE_ADD_PVP_TOURNAMENT)
	UINT	uiWinSessionID;
#endif
};

struct SCPVP_FINISH_PVPROUND
{
	UINT	uiWinTeam;		// See PvPCommon::Team
	PvPCommon::FinishReason::eCode	Reason;		// See PvPCommon::FinishReason
	UINT	uiATeamScore;	// RoundMode receives GameModeScore from the server.
	UINT	uiBTeamScore;
#if defined(PRE_ADD_PVP_TOURNAMENT)
	UINT	uiWinSessionID;
	char	cTournamentStep; // PvPCommon::Tournament::eStep
#endif
};

struct SCPVP_SUCCESSBREAKINTO
{
	UINT	uiSessionID;
};

struct SCPVP_ADDPOINT
{
	UINT	uiSessionID;
	UINT	uiScoreType;
	UINT	uiAddPoint;
};

struct SCPVP_SELECTCAPTAIN
{
	UINT	uiSessionID;
};

struct TZombieInfo
{
	UINT	uiSessionID;
	int		nMonsterMutationTableID;
	bool	bZombie;
	bool	bRemoveStateBlow;
	int		iScale;
};

struct SCPVP_SELECTZOMBIE
{
	BYTE	cCount;
	TZombieInfo Zombies[PARTYMAX];
};

struct SCPVP_LEVEL
{
	UINT	uiSessionID;
	BYTE	cLevel;
};

struct SCPVP_ROOMINFO
{
	UINT	uiIndex;
	UINT	uiWinCondition;
	UINT	uiGameModeTableID;
	UINT	uiPlayTimeSec;
	BYTE	cMaxUser;
	bool bIsGuildWar;		//If this is true, only exit is enabled and everything else is blocked
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE cGambleType;
	int nGamblePrice;
#endif
};

struct TPvPUserScore
{
	UINT	uiSessionID;
	UINT	uiKillScore;
	UINT	uiAssistScore;
	UINT	uiSupportScore;
	UINT	uiInstantSupportScore;
	UINT	uiKillClassScore[PvPCommon::Common::MaxClass];
	UINT	uiKilledClassScore[PvPCommon::Common::MaxClass];
};

struct TPvPUserDieCount
{
	UINT	uiSessionID;
	UINT	uiDieCount;
};

struct TPvPUserCount
{
	UINT	uiSessionID;
	UINT	uiKillCount;
	UINT	uiDieCount;
	UINT	uiContinuousKillCount;
};

struct SCPVP_MODE_SCORE
{
	USHORT				unATeamScore;
	USHORT				unBTeamScore;
	BYTE				cScoreCount;
	BYTE				cCount;
	TPvPUserScore		sPvPUserScore[PvPCommon::Common::MaxPlayer];
	TPvPUserCount		sPvPUserCount[PvPCommon::Common::MaxPlayer];
};

struct TPvPCaptainKillCount
{
	UINT		uiSessionID;
	UINT		uiCaptainKillCount;
};

struct SCPVP_CAPTAINKILL_COUNT
{
	BYTE					cCount;
	TPvPCaptainKillCount	sCaptainKillCount[PvPCommon::Common::MaxPlayer];
};

struct TPvPZombieKillCount
{
	UINT		uiSessionID;
	UINT		uiZombieKillCount;
};

struct SCPVP_ZOMBIEKILL_COUNT
{
	BYTE				cCount;
	TPvPZombieKillCount	sZombieKillCount[PvPCommon::Common::MaxPlayer];
};

struct TPvPOccupationScoreInfo
{
	UINT nSessionID;
	USHORT nTryAcquireCount;
	USHORT nAcquireCount;
	BYTE cBossKillCount;
	USHORT nStealAquireCount;
};

struct TPvPOccupationTeamScoreInfo
{
	USHORT nTeamID;
	int nTeamKillScore;
	int nTeamAcquireScore;
	int nTeamStealAcquireScore;

	TPvPOccupationTeamScoreInfo()
		: nTeamID( )
		, nTeamKillScore( 0 )
		, nTeamAcquireScore( 0 )
		, nTeamStealAcquireScore( 0 )
	{}
};

struct TPvPOccupationScore
{
	TPvPOccupationTeamScoreInfo TeamScoreInfo[PvPCommon::TeamIndex::Max];
	BYTE cCount;
	TPvPOccupationScoreInfo Info[PvPCommon::Common::MaxPlayer];
};

#if defined(_GAMESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct SCPVP_RESPAWN_POINT
{
	char				szName[32];
	int					iBasePreferPoint;
	int					iPoint;
	USHORT				unTeam;
	EtVector3			Position;
};
#endif

struct TPvPUserXPScore
{
	UINT	uiSessionID;
	union
	{
		UINT uiAddXPScore;
		int  iAddLadderGradePoint;
	};
	union
	{
		UINT uiResultXPScore;
		UINT uiResultLadderGradePoint;
	};
	UINT	uiGainMedalCount;
#if defined(PRE_MOD_PVP_LADDER_XP)
	UINT	uiAddLadderXPScore;	//EXP granted in ladder game
#endif
};

struct SCPVP_XPSCORE
{
	char			cCount;
	TPvPUserXPScore	sXPScoreArr[PvPCommon::Common::MaxPlayer];
};

#if defined(PRE_ADD_RACING_MODE)
struct TPvPUserRapTime
{
	UINT	uiSessionID;
	DWORD	dwLapTime; //Tick
};

//SC_PVP_RACING_RAPTIME
struct SCPVP_RACING_RAPTIME
{
	char			cCount;
	TPvPUserRapTime	sRapTimeArr[PvPCommon::Common::MaxPlayer];
};

//SC_PVP_RACING_FIRST_END
struct SCPVP_RACING_FIRST_END
{
	UINT	uiSessionID;		// SessionID of the 1st place player
};
#endif // #if defined(PRE_ADD_RACING_MODE)

struct CSPVP_BAN
{
	UINT							uiSessionID;
	PvPCommon::BanType::eBanType	eType;
};

struct CSPVP_CHANGECHANNEL
{
	BYTE cType;		//PvPCommon::RoomType;
};

namespace LadderSystem
{
	struct CS_ENTER_CHANNEL
	{
		MatchType::eCode	MatchType;
	};

	struct SC_ENTER_CHANNEL
	{
		int iRet;
		MatchType::eCode	MatchType;
	};
	
	struct SC_LEAVE_CHANNEL
	{
		int iRet;
	};

	struct SC_NOTIFY_LEAVEUSER
	{
		INT64			biCharDBID;
		Reason::eCode	Reason;
	};

	struct LadderUserInfo
	{
		INT64	biCharDBID;
		WCHAR	wszCharName[NAMELENMAX];
		int		iGradePoint;
		BYTE	cJob;
	};

	struct SC_NOTIFY_JOINUSER
	{
		LadderUserInfo sUserInfo;
	};

	struct SC_NOTIFY_LEADER
	{
		WCHAR	wszLeaderName[NAMELENMAX];
	};

	struct SC_NOTIFY_ROOMSTATE
	{
		RoomState::eCode		State;
		RoomStateReason::eCode	Reason;
	};

	struct CS_LADDER_MATCHING
	{
		bool	bIsCancel;
	};

	struct SC_LADDER_MATCHING
	{
		int		iRet;
		bool	bIsCancel;
	};

	struct SC_NOTIFY_GAMEMODE_TABLEID
	{
		int		iTableID;
	};

	struct CS_PLAYING_ROOMLIST
	{
		UINT	uiReqPage;
	};

	struct SC_PLAYING_ROOMLIST
	{
		int		iRet;
		USHORT	unMaxPage;
		USHORT	unNameCount;
		BYTE	cJob[Common::RoomListPerPage*MatchType::MaxMatchType*2];
		WCHAR	wszCharName[Common::RoomListPerPage*MatchType::MaxMatchType*2][NAMELENMAX];
	};

	struct CS_OBSERVER
	{
		WCHAR wszCharName[NAMELENMAX];
	};

	struct SC_OBSERVER
	{
		int		iRet;
	};

	struct SC_SCOREINFO
	{
		TPvPLadderScoreInfo Data;
	};

	struct SC_SCOREINFO_BYJOB
	{
		int	iRet;
		TPvPLadderScoreInfoByJob Data;
	};

	struct SC_MATCHING_AVGSEC
	{
		int iSec;	// Unknown if -1
	};

	struct SC_LADDERPOINT_REFRESH
	{
		int iPoint;
	};

	struct CS_INVITE
	{
		WCHAR wszCharName[NAMELENMAX];
	};

	struct SC_INVITE
	{
		int iRet;
		WCHAR wszCharName[NAMELENMAX];
	};

	struct CS_INVITE_CONFIRM
	{
		bool bAccept;
		WCHAR wszCharName[NAMELENMAX];
	};

	struct SC_INVITE_CONFIRM_REQ
	{
		WCHAR wszCharName[NAMELENMAX];
		LadderSystem::MatchType::eCode MatchType;
		int iCurUserCount;
		int iAvgGradePoint;
	};

	struct SC_INVITE_CONFIRM
	{
		int iRet;
		WCHAR wszCharName[NAMELENMAX];
	};

	struct SC_REFRESH_USERINFO
	{
		BYTE			cCount;
		LadderUserInfo	sUserInfoArr[MatchType::MaxMatchType];
	};

	struct CS_KICKOUT
	{
		INT64 biCharacterDBID;
	};

	struct SC_KICKOUT
	{
		int iRet;
	};
};

struct SCPVP_FATIGUE_REWARD	// SC_FATIGUE_REWARD
{
	int nGainExp;
	int nGainMedal;
};

struct CSPVP_FATIGUE_OPTION // CS_FATIGUE_OPTION
{
	bool bOption; // On,Off
};

struct SCPVP_GHOUL_SCORES // SC_PVP_GHOULSCORES
{
	TPvPGhoulScores GhoulScores;
};
struct SCPVP_HOLYWATER_KILLCOUNT //SC_PVP_HOLYWATER_KILLCOUNT
{
	UINT nSessionID;      // Dead target
	UINT nKillerSessionID;// Killer target
};

struct SCPVP_CHANGE_CHANNEL
{
	BYTE cType;
	int nRetCode;
};

#if defined(PRE_ADD_QUICK_PVP)
// CS_PVP / CS_QUICKPVP_INVITE // Duel request
struct CSQuickPvPInvite
{
	UINT nReceiverSessionID;				// User to invite
};

// CS_PVP / CS_QUICKPVP_RESULT // Duel request response
struct CSQuickPvPResult
{		
	bool bAccept;							// Whether accepted
	UINT nSenderSessionID;					// Inviter user
};

// SC_PVP / SC_QUICKPVP_INVITE // Duel request	
struct SCQuickPvPInvite
{
	UINT nSenderSessionID;				// Inviter user
};

// SC_PVP / SC_QUICKPVP_RESULT // Duel request response
struct SCQuickPvPResult
{
	int	nResult;						// Application result
};
#endif //#if defined(PRE_ADD_QUICK_PVP)
#if defined(PRE_ADD_PVP_VILLAGE_ACCESS)
// SC_PVP / SC_PVP_LIST_OPEN_UI
struct SCPvPListOpenUI
{
	int nResult;						// UI open result
	int nLimitLevel;					// Level limit
};
#endif

struct SCPVP_ALLKILL_SHOW_SELECTPLAYER
{
	bool bSelectPlayer;
};

struct CSPVP_ALLKILL_SELECTPLAYER
{
	UINT uiSelectPlayerSessionID;
};

struct SCPVP_ALLKILL_SELECTPLAYER
{
	UINT uiSelectPlayerSessionID;
};

struct SCPVP_ALLKILL_GROUPCAPTAIN
{
	UINT uiGroupCaptainSessionID;
};

struct SCPVP_ALLKILL_ACTIVEPLAYER
{
	UINT uiActivePlayerSessionID;
};

struct SCPVP_ALLKILL_BATTLEPLAYER
{
	UINT uiSessionIDArr[2];
};

struct SCPVP_ALLKILL_CONTINUOUSWIN
{
	UINT uiSessionID;
	UINT uiCount;
};

struct SCPVP_ALLKILL_FINISHDETAILREASON
{
	PvPCommon::FinishDetailReason::eCode Reason;
};

struct CSPvPTeamSwapMemberIndex		//CS_SWAPMEMBERINDEX
{
	BYTE cCount;
	TSwapMemberIndex Index[PARTYMAX];
};

struct CSPvPGuildWarChangeMemberGrade		//CS_GUILDWAR_CHANGEMEMBER_GRADE
{
	bool bAsign;		//true to appoint, false to dismiss
	USHORT nType;		//PvPCommon::UserState
	UINT nSessionID;	//Target SessionID
};

#if defined(_GAMESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))
struct CSOrderConcentrate					//CS_CONCENTRATE_ORDER
{
	EtVector3 vPosition;
};

struct SCConcentrateOrder				 //SC_CONCENTRATE_ORDER
{
	UINT nSessionID;
	EtVector3 vPosition;
};
#endif		//#if defined(_GAMESERVER) || (defined(_WINDOWS) && !defined(_LAUNCHER))

struct CSPvPTryAcquirePoint		//CS_PVP / CS_TRYCAPTURE
{
	int nAreaID;
};

struct CSPvPTryAcquireSKill		//CS_PVP / CS_TRYACQUIRE_SKILL
{
	int nSkillID;
	int nLevel;
	char cSlotIndex;
};

struct CSPvPUseSkill			//CS_PVP / CS_USESKILL
{
	int nSkillID;
};

struct CSPvPSwapSkillIndex		//CS_PVP / CS_SWAPSKILLINDEX
{
	char cFromIndex;
	char cToIndex;
};

struct SCPvPOccupationModeState		//SC_PVP / SC_PVP_OCCUPATION_MODESTATE
{
	int nState;		//See PvPCommon::OccupationSystemState
};

struct SCPvPOccupationState		//SC_PVP / SC_PVP_OCCUPATIONSTATE
{
	BYTE cCount;
	PvPCommon::OccupationStateInfo Info[PARTYMAX];
};

struct SCPvPOccupationTeamState		//SC_PVP / SC_PVP_OCCUPATION_TEAMSTATE
{
	int nTeam;
	int nResource;			//Current resources
	int nResourceSum;		//Total acquired amount
};

struct SCPvPOccupationSkillState
{
	PvPCommon::OccupationSkillSlotInfo Info;
};

#if defined(PRE_ADD_REVENGE)
// CS_PVP / SC_PVP_SET_REVENGE_TARGET
struct SCPvPSetRevengeTarget
{
	UINT uiSessionID;
	UINT uiRevengeTargetSessionID;
	Revenge::TargetReason::eCode eReason;
};

// CS_PVP / CS_PVP_SUCCESS_REVENGE
struct SCPvPSuccessRevenge
{
	UINT uiSessionID;
	UINT uiRevengeTargetSessionID;
};
#endif

#if defined(PRE_ADD_PVP_TOURNAMENT)
// CS_PVP / CS_PVP_SWAP_TOURNAMENT_INDEX
struct CSPvPSwapTournamentIndex
{
	char cSourceIndex;
	char cDestIndex;
};

// SC_PVP / SC_PVP_SWAP_TOURNAMENT_INDEX
struct SCPvPSwapTournamentIndex
{
	int nRetCode;
	char cSourceIndex;
	char cDestIndex;
};

// SC_PVP / SC_PVP_TOURNAMENT_MATCHLIST
struct TPvPTournamentUserInfo
{	
	char	cTournamentStep;		// Tournament step (round of N?) // PvPCommon::Tournament::eStep
	bool	bWin;					// Whether won or lost.
	BYTE	cJob;					// Job
	UINT	uiSessionID;
	WCHAR	wszCharName[NAMELENMAX];
};
struct SCPvPTournamentMatchList
{
	int nCount;
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE cGambleType;
	int nGamblePrice;
#endif
	TPvPTournamentUserInfo sTournamentUserInfo[PvPCommon::Common::PvPTournamentUserMax];	
};

// SC_PVP / SC_PVP_TOURNAMENT_DEFAULTWIN
struct SCPvPTournamentDefaultWin
{
	char	cTournamentStep;		// Tournament step (round of N?) // PvPCommon::Tournament::eStep
	UINT	uiWinSessionID;			// SessionID of the winner by bye
};

// SC_PVP / SC_PVP_TOURNAMENT_IDLE_TICK
struct SCPvPTournamentIdleTick
{
	UINT	uiIdleTick;
	UINT	uiCurTick;
};

// SC_PVP / SC_PVP_TOURNAMENT_TOP4
struct SCPvPTournamentTop4
{
	UINT	uiSessionID[4];			// Tournament ranks 1 to 3 (two 3rd place); a rank without uiSeesionID means the player left.
#if defined( PRE_PVP_GAMBLEROOM )
	BYTE    cGambleType;
	int     nGamblePrice[4];
#endif
};
#endif

// SC

//--------------------------------------------------------------------------------------------------
//	Trigger (CS_TRIGGER, SC_TRIGGER)
//--------------------------------------------------------------------------------------------------
struct SCTriggerCallAction
{
	short wSectorIndex;
	short wObjectIndex;
	UINT nRandomSeed;
};

struct SCChangeMyBGM
{
	int nBGM;
	int nFadeDelta;
	UINT nSessionID;
};

struct SCRadioImage
{
	int nFileID;
	UINT nSessionID;
	DWORD nTime;
};

struct SCFileTableBGMOff
{
	UINT nSessionID;
};

struct SCForceEnableRide
{
	UINT nSessionID;
	bool bForceEnableRide;
};

//MISSION
//--------------------------------------------------------------------------------------------------
//	MISSION (CS_MISSION, SC_MISSION)
//--------------------------------------------------------------------------------------------------
struct SCMissionList
{
	int nMissionScore;
	char cGain[MISSIONMAX_BITSIZE];
	char cAchieve[MISSIONMAX_BITSIZE];
	short wLastUpdate[LASTMISSIONACHIEVEMAX];
};

struct SCMissionGain
{
	int nArrayIndex;
};

struct SCMissionAchieve
{
	int nArrayIndex;
};

struct SCMissionEventPopUp
{
	int nItemID;
};

struct SCMissionHelpAlarm
{
	int nItemID;
};

struct SCDailyMissionList
{
	char cType;
	int nCount;
	TDailyMission Mission[DAILYMISSIONMAX > WEEKLYMISSIONMAX ? DAILYMISSIONMAX : WEEKLYMISSIONMAX];
};

struct SCWeeklyMissionList
{
	int nCount;
	TDailyMission Mission[WEEKLYMISSIONMAX];
};


struct SCDailyMissionCount
{
	char cType;
	int nArrayIndex;
	int nCount;
};

struct SCDailyMissionAchieve
{
	char cType;
	int nArrayIndex;
	bool bSecret;
};

struct SCDailyMissionAlarm
{
	char cType;
};

struct SCConnectingTime
{
	int nItemID;	// ItemID in TimeEventTable
	int nDuration;	// Elapsed time (seconds) since event start time
};

#if defined(PRE_ADD_ACTIVEMISSION)
struct SCActiveMissionGain	//SC_MISSION / SC_GAIN_ACTIVE_MISSION
{
	int nIndex;
};

struct SCActiveMissionAchieve	//SC_MISSION / SC_ACHIEVE_ACTIVE_MISSION
{
	int nIndex;
};
#endif

#if defined(PRE_ADD_CHAT_MISSION)
struct CSChatMission	//CS_MISSION / CS_CHAT_MISSION
{
	int nUIStringMid;
};
#endif

//APPELLATION
//--------------------------------------------------------------------------------------------------
//	Appellation (CS_APPELLATION, SC_APPELLATION)
//--------------------------------------------------------------------------------------------------
struct SCAppellationList
{
	char cGain[APPELLATIONMAX_BITSIZE];
};

struct SCAppellationGain
{
	int nArrayIndex;
};

struct SCSelectAppellation
{
	int nArrayIndex;
	int nCoverArrayIndex;
};

struct SCChangeAppellation
{
	UINT nSessionID;
	int nArrayIndex;
	int nCoverArrayIndex;
};

struct CSSelectAppellation
{
	int nArrayIndex;
	int nCoverArrayIndex;
};

struct SCPeriodAppellationTIme
{	
	BYTE cPeriodCount;
	TAppellationTime AppellationTime[PERIODAPPELLATIONMAX];
};

struct CSCollectionBook		// CS_COLLECTIONBOOK
{
	int nCollectionID;
};

//--------------------------------------------------------------------------------------------------
//	GameOption (CS_GAMEOPTION, SC_GAMEOPTION)
//--------------------------------------------------------------------------------------------------
struct CSGameOptionUpdate		//CS_GAMEOPTION_UPDATEOPTION
{
	TGameOptions Option;
};

struct CSGameOptionReqComm		// CS_GAMEOPTION / CS_GAMEOPTION_REQCOMMOPTION
{
	UINT nSessionID;
};

struct SCGameOptionCommOption		// SC_GAMEOPTION / SC_GAMEOPTION_USERCOMMOPTION
{
	UINT nSessionID;
	char cCommunityOption[13];
	TPARTYID PartyID;
};

struct SCGameOptionRefreshNotifier
{
	DNNotifier::Data data[DNNotifier::RegisterCount::Total];
};

struct CSGameOptionQuestNotifier			// CS_GAMEOPTION / CS_GAMEOPTION_QUEST_NOTIFIER
{
	DNNotifier::Data	data[DNNotifier::RegisterCount::TotalQuest];
};

struct CSGameOptionMissionNotifier			// CS_GAMEOPTION / CS_GAMEOPTION_MISSION_NOTIFIER
{
	DNNotifier::Data	data[DNNotifier::RegisterCount::TotalMission];
};

struct SCGameOptionGetProfile
{
	TProfile	sProfile;
};

struct CSGameOptionSetProfile
{
	TProfile	sProfile;
};

struct SCGameOptionDisplayProfile	// Used when showing profile info to other users (joining chat room, joining party, etc.)
{
	int			nSessionID;		// Profile owner
	TProfile	sProfile;		// Profile content
};

struct SCGameOptionSelectKeySetting
{
	bool		bIsDefault;
	TKeySetting sKeySetting;
};

struct CSGameOptionUpdateKeySetting
{
	bool		bIsDefault;
	TKeySetting	sKeySetting;
};

struct SCGameOptionSelectPadSetting
{
	bool		bIsDefault;
	TPadSetting sPadSetting;
};

struct CSGameOptionUpdatePadSetting
{
	bool		bIsDefault;
	TPadSetting	sPadSetting;
};

struct SCGameOptionNotifySecondAuthInfo
{
	bool bIsSetSecondAuthPW;	// Whether 2nd auth password is set
	bool bIsSetSecondAuthLock;	// 2nd auth account Lock status
	__time64_t	tSecondAuthResetDate;
#ifdef PRE_ADD_23829
	bool bSecondAuthPassFlag;
#endif
#ifdef PRE_ADD_IGNORESECONDAUTH_EKEYECARD
	bool bCompletlySecondAuthPassFlag;
#endif
};

struct CSGameOptionUpdateSecondAuthPassword
{
	int nOldSeed;
	int nOldValue[SecondAuth::Common::PWMaxLength];
	int nNewSeed;
	int nNewValue[SecondAuth::Common::PWMaxLength];
};

struct SCGameOptionUpdateSecondAuthPassword
{
	int		iRet;
	BYTE	cFailCount;
};

struct CSGameOptionUpdateSecondAuthLock
{
	bool	bIsLock;
	int nSeed;
	int nValue[SecondAuth::Common::PWMaxLength];
};

struct SCGameOptionUpdateSecondAuthLock
{
	int		iRet;
	BYTE	cFailCount;
};

struct CSGameOptionValidateSecondAuth
{
	int nSeed;
	int nValue[SecondAuth::Common::PWMaxLength];
	int		nAuthCheckType;
};

struct SCGameOptionValidateSecondAuth
{
	int		nRet;
	int		nAuthCheckType;
	BYTE	cFailCount;
};

struct CSGameOptionInitSecondAuth
{
	int nSeed;
	int nValue[SecondAuth::Common::PWMaxLength];
};

struct SCGameOptionInitSecondAuth
{
	int		iRet;
	BYTE	cFailCount;
};

//--------------------------------------------------------------------------------------------------
//	Radio (CS_RADIO, SC_RADIO)
//--------------------------------------------------------------------------------------------------
struct CSUseRadio					//CS_RADIO / CS_USERADIO
{
	USHORT nID;
};

struct SCUseRadio					//SC_RADIO / SC_USERADIO
{
	UINT nSessionID;
	USHORT nID;
};

//--------------------------------------------------------------------------------------------------
//	Gesture (CS_GESTURE, SC_GESTURE)
//--------------------------------------------------------------------------------------------------
struct SCGestureList
{
	BYTE cGestureCount;
	USHORT nGestureID[GESTUREMAX];
};

struct SCGestureAdd
{
	USHORT nGestureID;
};

struct CSUseGesture
{
	USHORT nGestureID;
};

struct SCUseGesture
{
	USHORT nGestureID;
};

//--------------------------------------------------------------------------------------------------
//	VoiceChat (CS_VOICECHAT, SC_VOICECHAT)
//--------------------------------------------------------------------------------------------------
struct TTalkingInfo
{
	UINT nSessionID;
	BYTE cTalking;		//none zero is talking. 0-255 volume
};

struct SCTalkingInfo		//CS_VOICECHAT / SC_TALKINGINFO
{
	BYTE cCount;
	TTalkingInfo Talking[PARTYMAX];
};

struct CSVoiceChatAvailable		//CS_VOICECHAT / CS_VOICEAVAILABLE
{
	BYTE cAvailable;
};

struct CSVoiceMute		//CS_VOICECHAT / CS_VOICEMUTE
{
	UINT nSessionID;
	BYTE cMute;			//0 mute, 1 unmute
};

struct CSVoiceComplaintReq	//CS_VOICECHAT / CS_VOICECOMPLAINTREQ
{
	UINT nSessionID;
	char szCategory[32];
	char szSubject[32];
	char szMsg[32];
};

struct SCVoiceChatInfo				// SC_VOICECHAT / SC_VOICECHATINFO
{
	char szVoiceChatIp[IPLENMAX];
	USHORT wControlPort;
	USHORT wAudioPort;
	UINT nAccountDBID;
	BOOL	bInit;
	USHORT	wManagedID;

};

struct TVoiceMemberInfo
{
	UINT nSessionID;
	BYTE cVoiceAvailable;
};

struct SCVoiceMemberInfo		// SC_VOICECHAT / SC_VOICEMEMBERINFO
{
	UINT nVoiceMutedList[PARTYCOUNTMAX];
	BYTE cCount;
	TVoiceMemberInfo Info[PARTYMAX];
};

//--------------------------------------------------------------------------------------------------
//	Restraint (SC_RESTRAINT)
//--------------------------------------------------------------------------------------------------
struct SCRestraintAdd			// SC_RESTRAINT / SC_RESTRAINTADD
{
	TRestraint Restraint;
};

struct SCRestraintDel			// SC_RESTRAINT / SC_RESTRAINTDEL
{
	UINT nRestraintKey;
};

struct SCRestraintRet			// SC_RESTRAINT / SC_RESTRAINTRETCODE
{
	short nRestraintRet;
};

struct SCRestraintList		// SC_RESTRAINT / SC_RESTRAINTLIST
{
	BYTE cCount;
	TRestraint restraint[RESTRAINTMAX];
};

//--------------------------------------------------------------------------------------------------
//	CashShop (CS_CASHSHOP, SC_CASHSHOP)
//--------------------------------------------------------------------------------------------------

struct SCCashShopOpen			// SC_CASHSHOP / SC_SHOPOPEN, // Open shop
{
	UINT nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
	WCHAR wszAccountName[IDLENMAX];
};

struct SCCashShopClose			// SC_CASHSHOP / SC_SHOPCLOSE, // Close shop
{
	int nRet;
};

struct SCCashShopBalanceInquiry			// SC_CASHSHOP / SC_BALANCEINQUIRY
{
	int nRet;
	int nPetal;
	int nCash;
};

struct TCashShopInfo
{
	char cSlotIndex;	// Cart index
	int nItemSN;
	int nItemID;	// Item ID value between ItemID01-05 in Commodity
	char nOptionIndex;
};

typedef TWishItemInfo TCashShopPackageInfo;

struct TPaymentItem
{
	int nItemSN;
	int nItemID;
	char cItemOption;
};

struct TPaymentItemInfo
{	
	INT64 biDBID;				// (biPurchaseOrderDetailID)
	__time64_t tPaymentDate;	// Payment (purchase) date
	TPaymentItem ItemInfo;	// Item info
};

struct TPaymentPackageItemInfo
{	
	INT64 biDBID;				// (biPurchaseOrderDetailID)
	int	 nPackageSN;			// Package serial
	__time64_t tPaymentDate;	// Payment (purchase) date
	TPaymentItem ItemInfoList[PACKAGEITEMMAX];		// Needs to be handled dynamically.. variable inside variable is..
};

struct TPaymentItemInfoEx		// Struct used by server only
{
	UINT uiOrderNo;				// Nexon 
	int nPrice;					// Product price (for refund)
	TPaymentItemInfo PaymentItemInfo;
};

struct TPaymentPackageItemInfoEx  // Struct used by server only
{
	UINT uiOrderNo;
	int nPrice;					// Product price (for refund)
	TPaymentPackageItemInfo PaymentPackageItemInfo;
};

struct CSCashShopBuy			// CS_CASHSHOP / CS_BUY, // Buy
{
	char cType;		// Purchase cart type (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX, 50 + eInstantCashShopBuyType: quick purchase)	
#if defined(PRE_ADD_CASH_REFUND)
	bool bMoveCashInven;	// Flag to move directly to cash inventory
#endif
	char cPaymentRules;	// Cash::PaymentRules
#if defined(PRE_ADD_SALE_COUPON)
	int nSaleCouponSN;				// Sale coupon CashCommodity SN;
	INT64 biSaleCouponSerial;		// If purchasing with a Sale coupon (must only allow purchasing a single item)
#endif // #if defined(PRE_ADD_SALE_COUPON)
	char cCount;
	TCashShopInfo BuyList[PREVIEWCARTLISTMAX];
};

struct SCCashShopBuy			// SC_CASHSHOP / SC_BUY, // Buy
{
	UINT nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
	char cType;	// (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX)
#if defined(PRE_ADD_CASH_REFUND)
	TPaymentItemInfo ItemList[PREVIEWCARTLISTMAX];	//Just filling it in and proceeding. If biDBID is 0, you can probably ignore it.
#endif
	char cCount;
	TCashShopInfo BuyList[PREVIEWCARTLISTMAX];
};

struct CSCashShopPackageBuy		// CS_CASHSHOP / CS_PACKAGEBUY
{
	int	 nPackageSN;
#if defined(PRE_ADD_CASH_REFUND)
	bool bMoveCashInven;	// Flag to move directly to cash inventory
#endif
	char cPaymentRules;	// Cash::PaymentRules
#if defined(PRE_ADD_SALE_COUPON)
	int nSaleCouponSN;
	INT64 biSaleCouponSerial;
#endif
	char cCount;
	TCashShopPackageInfo BuyList[PACKAGEITEMMAX];
};

struct SCCashShopPackageBuy		// SC_CASHSHOP / SC_PACKAGEBUY
{
	int	nPackageSN;
	int nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
#if defined(PRE_ADD_CASH_REFUND)
	TPaymentPackageItemInfo PackageItem;	// Package count is always one.
#endif
};

struct CSCashShopCheckReceiver	// CS_CASHSHOP / CS_CHECKRECEIVER, // Check recipient (needed when gifting)
{
	WCHAR wszToCharacterName[NAMELENMAX];	// Recipient
};

struct SCCashShopCheckReceiver	// SC_CASHSHOP / SC_CHECKRECEIVER, // Check receiver (needed when gifting)
{
	BYTE cLevel;
	BYTE cJob;
	int nRet;
};

struct CSCashShopGift			// CS_CASHSHOP / CS_GIFT, // Gift, request gift
{
	WCHAR wszToCharacterName[NAMELENMAX];	// Recipient
	WCHAR wszMessage[GIFTMESSAGEMAX];	// Memo
#if defined(PRE_ADD_CADGE_CASH)
	int nMailDBID;
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	char cType;		// (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX)
	char cCount;
	TCashShopInfo GiftList[PREVIEWCARTLISTMAX];
};

struct SCCashShopGift			// SC_CASHSHOP / SC_GIFT, // Send gift
{
	int nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
	char cType;		// (3,7: PREVIEWCARTLISTMAX, others: CARTLISTMAX)
	char cCount;
	TCashShopInfo GiftList[PREVIEWCARTLISTMAX];
};

struct CSCashShopPackageGift	// CS_CASHSHOP / CS_PACKAGEGIFT
{
	WCHAR wszToCharacterName[NAMELENMAX];	// Recipient
	WCHAR wszMessage[GIFTMESSAGEMAX];	// Memo
#if defined(PRE_ADD_CADGE_CASH)
	int nMailDBID;		// Set value only when clicking beg in mail (0 when clicked in cash shop)
#endif	// #if defined(PRE_ADD_CADGE_CASH)
	int	 nPackageSN;
	char cCount;
	TCashShopPackageInfo PackageGiftList[PACKAGEITEMMAX];
};

struct SCCashShopPackageGift	// SC_CASHSHOP / SC_PACKAGEGIFT
{
	int nPackageSN;
	int nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
};

typedef CSCashShopPackageGift CSCashShopCadge;	// CS_CASHSHOP / CS_CADGE

struct SCCashShopCadge			// SC_CASHSHOP / SC_CADGE, // Beg / request gift
{
	int nRet;
};

// CS_CASHSHOP / CS_GIFTLIST, // Request gift box (received gifts)

struct TGiftInfo
{
	char cPayMethodCode;				// Gift type (db:PayMethodCode)
	INT64 nGiftDBID;					// (db:PurchaseOrderID)
	WCHAR wszSenderName[MAILNAMELENMAX];	// Sender
	int nItemSN;
	int nItemID;						// Ignore this value if package (?)
	char cItemOption;
	__time64_t tOrderDate;
	WCHAR wszMessage[GIFTMESSAGEMAX];	// Memo
	__time64_t tGiftExpireDate;			// Gift box retention period
#if defined(PRE_ADD_GIFT_RETURN)
	bool bGiftReturn;					// Whether gift is returned
#endif
	bool bNewFlag;
	int nPaidCashAmount;
};

struct SCCashShopGiftList		// SC_CASHSHOP / SC_GIFTLIST, // Gift box (received gifts)
{
	int nRet;
	char cCount;
	TGiftInfo GiftInfo[GIFTBOXLISTMAX];
};

struct TReceiveGiftData
{
	INT64 nGiftDBID;
	char cPayMethodCode;						// Gift type (db:PayMethodCode)
	WCHAR wszEmoticonTitle[MAILTITLELENMAX];
	WCHAR wszReplyMessage[GIFTMESSAGEMAX];	// Memo
};

struct CSCashShopReceiveGift	// CS_CASHSHOP / CS_RECEIVEGIFT
{
	TReceiveGiftData GiftData;
};

struct CSCashShopReceiveGiftAll	// CS_CASHSHOP / CS_RECEIVEGIFTALL
{
	char cCount;
	TReceiveGiftData GiftData[GIFTPAGEMAX];
};

struct SCCashShopReceiveGift	// SC_CASHSHOP / SC_RECEIVEGIFT
{
	int nRet;
	INT64 nGiftDBID;
};

struct SCCashShopReceiveGiftAll	// SC_CASHSHOP / SC_RECEIVEGIFTALL
{
	int nRet;
	char cCount;
	INT64 nGiftDBID[GIFTPAGEMAX];
};

#if defined(PRE_ADD_GIFT_RETURN)
struct CSCashShopGiftReturn		// CS_CASHSHOP / CS_GIFTRETURN
{
	INT64 nGiftDBID;	
};

struct SCCashShopGiftReturn	// SC_CASHSHOP / SC_GIFTRETURN
{
	INT64 nGiftDBID;
	int nRet;
};
#endif

// SC_CASHSHOP / SC_COUPON
struct CSCashShopCoupon			// CS_CASHSHOP / CS_COUPON
{
	WCHAR wszCoupon[COUPONMAX];
};

struct SCCashShopCoupon			// CS_CASHSHOP / SC_COUPON
{
	int nRet;
};

struct SCNotifyGift				// SC_CASHSHOP / SC_NOTIFYGIFT
{
	bool bNew;	// Whether it is a new gift (blinking)
	int nGiftCount;
};

struct CSVIPBuy				// CS_CASHSHOP / CS_VIPBUY
{
	int nItemSN;
};

struct SCVIPBuy				// CS_CASHSHOP / SC_VIPBUY, // Send vip info
{
	UINT nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
	int nVIPPoint;		// Point
	__time64_t tVIPExpirationDate;	// Expiration date
	bool bAutoPay;		// Whether auto-payment is enabled
};

struct CSVIPGift			// CS_CASHSHOP / CS_VIPGIFT
{
	WCHAR wszToCharacterName[NAMELENMAX];	// Recipient
	WCHAR wszMessage[GIFTMESSAGEMAX];	// Memo
	int nItemSN;
};

struct SCVIPGift			// CS_CASHSHOP / SC_VIPGIFT, // Send vip info
{
	UINT nCashAmount;	// Cash amount
	int nReserveAmount;	// Reward points
	int nRet;
};

struct SCVIPInfo			// SC_CASHSHOP / SC_VIPINFO
{
	bool bVIP;
	int nVIPPoint;		// Point
	__time64_t tVIPExpirationDate;	// Expiration date
	bool bAutoPay;		// Whether auto-payment is enabled
};

struct SCSaleAbortList		// SC_CASHSHOP / SC_SALEABORTLIST
{
	BYTE cCount;
	int nAbortList[SALEABORTLISTMAX];
};

#if defined(PRE_ADD_CASH_REFUND)
struct SCPaymentList		// SC_CASHSHOP / SC_PAYMENT_LIST
{
	int nTotalPaymentCount;		// Total billing inventory count (normal)
	BYTE cInvenCount;			// Number of inventory items currently being sent
	TPaymentItemInfo ItemList[CASHINVENTORYMAX];
};

struct SCPaymentPackageList	// SC_CASHSHOP / SC_PAYMENT_PACKAGELIST
{
	int nTotalPaymentCount;		// Total billing inventory count (package)
	BYTE cInvenCount;			// Number of inventory items currently being sent
	TPaymentPackageItemInfo ItemList[CASHINVENTORYMAX];		// Some bytes may be discarded here..
};

struct CSMoveCashInven		// CS_CASHSHOP / CS_MOVE_CASHINVEN
{
	BYTE cItemType;			// 1: Normal, 2: Package
	INT64 biDBID;			// (biPurchaseOrderDetailID)
};

struct SCMoveCashInven		// SC_CASHSHOP / SC_MOVE_CASHINVEN
{
	INT64 biDBID;			// (biPurchaseOrderDetailID)	
	int nRet;				// Result
	int nReserveAmount;		// Petal mileage points
};

struct CSCashRefund			// CS_CASHSHOP / CS_CASH_REFUND
{
	BYTE cItemType;			// 1: Normal, 2: Package
	INT64 biDBID;			// (biPurchaseOrderDetailID)
};

struct SCCashRefund			// SC_CASHSHOP / SC_CASH_REFUND
{
	INT64 biDBID;			// (biPurchaseOrderDetailID)
	int nRet;				// Result
	int nCashAmount;		// Current cash on success
};
#endif //#if defined(PRE_ADD_CASH_REFUND)

struct SCCashShopChargeTime
{
	__time64_t tTime;
};

struct CSSecretCashShopOpen		// CS_CASHSHOP / CS_SECRETCASHSHOP_OPEN
{
	int nHiddenShopID;
};

struct SCSecretCashShopNotify	// SC_CASHSHOP / SC_SECRETCASHSHOP_NOTIFY
{
	int nHiddenShopID;
};

struct SCSecretCashShopOpen		// SC_CASHSHOP / SC_SECRETCASHSHOP_OPEN
{
	int nRet;
	int nHiddenShopID;
};

#ifdef PRE_ADD_LIMITED_CASHITEM
struct CSQuantityLimitedItem
{
};

struct SCQuantityLimitedItem
{
	BYTE cIsLast;
	BYTE cCount;
	LimitedCashItem::TLimitedQuantityCashItem Limited[LimitedCashItem::Common::DefaultPacketCount];
};

struct SCChangedQuantityLimitedItem
{
	BYTE cCount;
	LimitedCashItem::TChangedLimitedQuantity Changed[LimitedCashItem::Common::DefaultPacketCount];
};
#endif		//#ifdef PRE_ADD_LIMITED_CASHITEM

struct CSDarkLairRankBoard
{
	int		iMapIndex;
	BYTE	cPartyUserCount;
};

struct SCDarkLairRankBoard
{
	int						iRet;
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	int						iMapIndex;
#endif
	TDLRankHistoryPartyInfo	sBestHistory;
	TDLRankHistoryPartyInfo	sHistoryTop[DarkLair::Rank::SelectRankBoardTop];
};

struct CSPvPLadderRankBoard
{
	LadderSystem::MatchType::eCode MatchType;
};

struct SCPvPLadderRankBoard
{
	int iRet;
	TPvPLadderRanking MyRanking;
	TPvPLadderRanking Top[LadderSystem::Common::RankingTop];
};

struct CSWindowState
{
	short sWinState;
};

#if defined(PRE_ADD_PVP_RANKING)

struct TPvPRanking
{
	int		iLevel;	
	BYTE	cJobCode;	
	BYTE	cPvPLevel;
	UINT	uiExp;
	int		iPvPRank;
	float	fPvPRaito;
	INT64	biRank;
	int		iChangedRank;	
	int		iKill;
	int		iDeath;

	WCHAR	wszGuildName[GUILDNAME_MAX];
	WCHAR	wszCharName[NAMELENMAX];
};

struct TPvPRankingDetail : public TPvPRanking
{
	INT64	biClassRank;
	int		iChangedClassRank;
	INT64	biSubClassRank;	
	int		iChangedSubClassRank;
};

struct TPvPLadderRanking2
{
	int		iLevel;
	BYTE	cJobCode;
	BYTE	cPvPLevel;
	UINT	uiExp;
	int		iPvPRank;
	float	fPvPRaito;
	INT64	biRank;
	int		iChangedRank;	
	int		iWin;
	int		iLose;
	int		iDraw;	
	int		iPvPLadderGradePoint;

	WCHAR	wszGuildName[GUILDNAME_MAX];
	WCHAR	wszCharName[NAMELENMAX];
};

struct TPvPLadderRankingDetail : public TPvPLadderRanking2
{
	INT64	biClassRank;
	int		iChangedClassRank;
	INT64	biSubClassRank;	
	int		iChangedSubClassRank;
};

// struct CSPvPRankBoard	//CS_PVP_RANK_BOARD
// {
// };

struct SCPvPRankBoard	//SC_PVP_RANK_BOARD
{
	int iRet;	
	TPvPRankingDetail MyRanking;
};

struct CSPvPRankInfo	//CS_PVP_RANK_INFO
{
	WCHAR	wszCharName[NAMELENMAX];
};

struct SCPvPRankInfo	//SC_PVP_RANK_INFO
{
	int iRet;	
	TPvPRankingDetail RankingInfo;
};

struct CSPvPRankList	//CS_PVP_RANK_LIST
{
	int	iPage;
	BYTE cClassCode;
	BYTE cSubClassCode;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct SCPvPRankList	//SC_PVP_RANK_LIST
{
	int iRet;	
	BYTE cRankingCount;
	INT64 nRankingTotalCount;
	TPvPRanking RankingInfo[RankingSystem::RANKINGMAX];
};

//Ladder Ranking
struct CSPvPLadderRankBoard2	//CS_PVP_LADDER_RANK_BOARD
{
	LadderSystem::MatchType::eCode MatchType;
};

struct SCPvPLadderRankBoard2	//SC_PVP_LADDER_RANK_BOARD
{
	int iRet;
	LadderSystem::MatchType::eCode MatchType;
	TPvPLadderRankingDetail MyRanking;
};

struct CSPvPLadderRankInfo	//CS_PVP_LADDER_RANK_INFO
{
	LadderSystem::MatchType::eCode MatchType;
	WCHAR	wszCharName[NAMELENMAX];
};

struct SCPvPLadderRankInfo	//SC_PVP_LADDER_RANK_INFO
{
	int iRet;
	LadderSystem::MatchType::eCode MatchType;
	TPvPLadderRankingDetail RankingInfo;
};

struct CSPvPLadderRankList	//CS_PVP_LADDER_RANK_LIST
{
	int	iPage;
	BYTE cClassCode;
	BYTE cSubClassCode;
	LadderSystem::MatchType::eCode MatchType;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct SCPvPLadderRankList	//SC_PVP_LADDER_RANK_LIST
{
	int iRet;	
	BYTE cRankingCount;
	INT64 nRankingTotalCount;
	LadderSystem::MatchType::eCode MatchType;
	TPvPLadderRanking2 RankingInfo[RankingSystem::RANKINGMAX];
};

#endif

#if defined( PRE_ADD_GAMEQUIT_REWARD )
struct SCGameQuitRewardCheckRes	//SC_ETC / SC_GAMEQUIT_REWARDCHECK_RES / Send reward type for user (client) to receive
{
	GameQuitReward::RewardType::eType eRewardType;	// Reward type on logout
};

struct CSGameQuitRewardReq		//CS_ETC / CS_GAMEQUIT_REWARD_REQ / Reward request packet
{
	GameQuitReward::RewardType::eType eRewardType;	// Reward kind (type) sent from SCGameQuitRewardCheckRes
};

struct SCGameQuitRewardRes		//SC_ETC / SC_GAMEQUIT_REWARD_REQ / Result of claiming reward
{
	int nRet;	// ERROR_NONE = Reward received successfully / On error: ERROR_ITEM_NOTFOUD (when failing to find item in dnt table, or user is ineligible for reward)
};
#endif	// #if defined( PRE_ADD_GAMEQUIT_REWARD )

//--------------------------------------------------------------------------------------------------
//	Farm
//--------------------------------------------------------------------------------------------------

struct TFarmItemPartial
{
	int		iFarmDBID;
	int		iFarmMapID;
#if defined( PRE_ADD_FARM_DOWNSCALE )
	int		iAttr;
#elif defined( PRE_ADD_VIP_FARM )
	Farm::Attr::eType	Attr;
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	WCHAR	wszFarmName[Farm::Max::FARMNAMELEN];		//Added for now just in case
};

struct TFarmItemFromDB:public TFarmItemPartial			// Farm info itself retrieved from DB
{
	int		iFarmMaxUser;
	bool	bEnableFlag;
	bool bStartActivate;		//Whether to start on open
};

struct TFarmItem:public TFarmItemFromDB					// Farm own info
{
	int		iFarmCurUser;
	bool	bActivate;				//Whether used
};

struct TFarmAttachItem
{
	int		iItemID;
	short	iCount;
};

struct TFarmField
{
	short			nFieldIndex;
	INT64			biCharacterDBID;
	WCHAR			wszCharName[NAMELENMAX];
	BYTE			cLevel;
	BYTE			cJob;
	int				iItemID;
	int				iElapsedTimeSec;
	TFarmAttachItem	AttachItems[Farm::Max::ATTACHITEM_KIND];
};

struct TFarmFieldForCharacter
{
	short			nFieldIndex;
	int				iItemID;
	int				iElapsedTimeSec;
	__time64_t		tUpdateDate;
	TFarmAttachItem	AttachItems[Farm::Max::ATTACHITEM_KIND];
};

struct TFarmFieldPartial
{
	int				iFarmDBID;
	short			nFieldIndex;
	int				iItemID;
	int				iElapsedTimeSec;
	TFarmAttachItem AttachItems[Farm::Max::ATTACHITEM_KIND];
};

//S-C

#if defined( PRE_ADD_VIP_FARM )
struct SCFarmStart
{
	int iRet;
};

struct SCEffectItemInfo
{
	int iItemID;
	__time64_t tExpireDate;
};

#endif // #if defined( PRE_ADD_VIP_FARM )

struct SCChangeOwnerName
{
	WCHAR		wszOriginName[NAMELENMAX];
	WCHAR		wszCharacterName[NAMELENMAX];	
};

struct SCFarmInfo				//SC_FARMINFO
{
	bool		bRefreshGate;
	BYTE		cCount;
	TFarmItem	Farms[Farm::Max::FARMCOUNT];
};

struct SCFarmPlantedInfo		//SC_FARMPLANTEDINFO
{
	BYTE				cCount;
	TFarmFieldPartial	FarmFields[Farm::Max::FIELDCOUNT_PERCHARACTER];
};

struct CSFarmEnter
{
	int iFarmDBID;
	int iMapID;
};

struct SCFarmBeginAreaState
{
	int						iAreaIndex;
	Farm::AreaState::eState State;
};

struct TFarmAttachItemSerial
{
	INT64	biSerial;
	BYTE	cCount;
#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
	bool bRemoveItem;			// Check if the item should be deleted (if granted by a skill, there is no item to delete)
#endif
};

struct CSFarmPlant
{
	int						iAreaIndex;
	Farm::ActionType::eType	ActionType;
	BYTE					cSeedInvenIndex;
	BYTE					cCount;
	TFarmAttachItemSerial	AttachItems[Farm::Max::PLANTING_ATTACHITEM];
};

struct SCFarmPlant
{
	int						iRet;
	Farm::ActionType::eType	ActionType;
	int						iAreaIndex;
	int						iItemID;
	int						iAttachItemID;
};

struct CSFarmHarvest
{
	int						iAreaIndex;
	Farm::ActionType::eType	ActionType;
	BYTE					cNeedItemInvenIndex;	// Inventory index containing the item required for harvesting
};

struct SCFarmHarvest
{
	int						iRet;
	int						iAreaIndex;
	Farm::ActionType::eType	ActionType;
	int						iResultItemIDs[Farm::Max::HARVESTDEPOT_COUNT];
};

struct CSFarmAddWater
{
	int						iAreaIndex;
	Farm::ActionType::eType	ActionType;
	BYTE					cInvenType;
	BYTE					cWaterItemInvenIndex;	// Inventory index containing water
	INT64					biWaterItemItemSerial;
};

struct SCFarmAddWater
{
	int						iRet;
	int						iAreaIndex;
	Farm::ActionType::eType	ActionType;
};

struct TFarmAreaInfo
{
	BYTE					iAreaIndex;
	int						iRandSeed;
	Farm::AreaState::eState	State;
	int						iElapsedTick;
	int						iItemID;
	WCHAR					wszCharName[NAMELENMAX];
	BYTE					cAttachCount;
	TFarmAttachItem			AttachItems[Farm::Max::ATTACHITEM_KIND];
};

struct SCAreaInfo
{
	TFarmAreaInfo	AreaInfo;
};

struct SCAreaInfoList
{
	USHORT			unCount;
	TFarmAreaInfo	AreaInfos[Farm::Max::FIELDCOUNT];
};

struct SCFarmWareHouseList
{
	int					iRet;
	BYTE				cCount;
	TFarmWareHouseItem	Items[Farm::Max::HARVESTDEPOT_COUNT];
};

struct CSFarmTakeWareHouseItem
{
	INT64	biUniqueID;
};

struct SCFarmTakeWareHouseItem
{
	int		iRet;
	INT64	biUniqueID;
};

struct SCFarmFieldCountInfo
{
	int		iCount;
};

struct SCFarmWareHouseItemCount
{
	int iCount;
};

struct SCFarmAddWaterAnotherUser
{
	WCHAR wszCharName[NAMELENMAX];
};

#ifdef PRE_ADD_CASHFISHINGITEM
struct TFishingToolInfo
{
	int nFishingRodInvenIdx;		//Inventory index of the fishing rod to use (to handle multiple rods and spot-specific rod restrictions)
	int nFishingBaitInvenIdx;		//Inventory index of the bait to use
	INT64 biCashRodSerial;
	INT64 biCashBaitSerial;
};
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM

struct CSFishingReady		//CS_READYFISHING
{
	int nFishingAreaIdx;
#ifdef PRE_ADD_CASHFISHINGITEM
	TFishingToolInfo ToolInfo;
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	int nFishingRodInvenIdx;		//Inventory index of the fishing rod to use (to handle multiple rods and spot-specific rod restrictions)
	int nFishingBaitInvenIdx;		//Inventory index of the bait to use
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
};

struct SCFishingReady		//SC_READYFISHING
{
	int nRetCode;
};

struct CSCastBait			//CS_CASTBAIT
{
	Fishing::Cast::eCast eCastType;					//See Fishing::Cast
#ifdef PRE_ADD_CASHFISHINGITEM
	TFishingToolInfo ToolInfo;
#else		//#ifdef PRE_ADD_CASHFISHINGITEM
	int nFishingRodInvenIdx;		//Inventory index of the fishing rod to use (to handle multiple rods and spot-specific rod restrictions)
	int nFishingBaitInvenIdx;		//Inventory index of the bait to use
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
};

struct CSPullingRod			//CS_PULLINGROD
{
	Fishing::Control::eControl ePullingControl;
};

struct SCCastBait			//SC_CASTBAITRESULT
{
	int nRetCode;
};

struct SCStopFishing		//SC_STOPFISHINGRESULT
{
	int nRetCode;
};

struct SCFishingPattern		//SC_FISHINGPATTERN
{
	int nPatternID;
#ifdef PRE_ADD_CASHFISHINGITEM
	int nFishingAutoMaxTime;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
};

struct SCFishingReward		//SC_FISHINGREWARD
{
	UINT nSessionID;
	int nRewardItemID;
	int nRetCode;			//-1: acquire failed, 0: success (nRewardItemID is acquired ID), > 0: error
};

struct SCFishingSync		//SC_FISHINGSYNC
{
	int nReduceTick;
	int nFishingGauge;
};

struct SCFishingPlayer		//SC_FISHINGPLAYER
{
	UINT nSessionID;
#ifdef PRE_ADD_CASHFISHINGITEM
	int nRodItemID;
#endif		//#ifdef PRE_ADD_CASHFISHINGITEM
};

struct SCStartDragonNest
{
	eDragonNestType Type;
	int nRebirthCount;
};

//--------------------------------------------------------------------------------------------------
//	ChatRoom
//--------------------------------------------------------------------------------------------------

struct TChatRoomView	// Chat room timestamp info
{
public:
	TChatRoomView()
	{
		Reset();
	}

	void Set(const TChatRoomView& pChatRoomView)
	{
		Set(pChatRoomView.m_nChatRoomID,
			pChatRoomView.m_wszChatRoomName,
			pChatRoomView.m_nChatRoomType,
			pChatRoomView.m_bHasPassword,
			pChatRoomView.m_wszChatRoomPRLine1,
			pChatRoomView.m_wszChatRoomPRLine2,
			pChatRoomView.m_wszChatRoomPRLine3
			);
	}

	void Set( UINT nChatRoomID, const LPCWSTR lpwszChatRoomName, BYTE nChatRoomType, bool bHasPassword,
			  const LPCWSTR lpwszChatRoomPRLine1, LPCWSTR lpwszChatRoomPRLine2, LPCWSTR lpwszChatRoomPRLine3 )
	{
		m_nChatRoomID = nChatRoomID;
		::wcsncpy_s(m_wszChatRoomName, _countof(m_wszChatRoomName), lpwszChatRoomName, _countof(m_wszChatRoomName));
		m_nChatRoomType = nChatRoomType;
		m_bHasPassword = bHasPassword;
		::wcsncpy_s(m_wszChatRoomPRLine1, _countof(m_wszChatRoomPRLine1), lpwszChatRoomPRLine1, _countof(m_wszChatRoomPRLine1));
		::wcsncpy_s(m_wszChatRoomPRLine2, _countof(m_wszChatRoomPRLine2), lpwszChatRoomPRLine2, _countof(m_wszChatRoomPRLine2));
		::wcsncpy_s(m_wszChatRoomPRLine3, _countof(m_wszChatRoomPRLine3), lpwszChatRoomPRLine3, _countof(m_wszChatRoomPRLine3));
	}

	void Reset()
	{
		m_nChatRoomID = 0;
		m_wszChatRoomName[0] = L'\0';
		m_nChatRoomType = 0;
		m_bHasPassword = false;
		m_wszChatRoomPRLine1[0] = L'\0';
		m_wszChatRoomPRLine2[0] = L'\0';
		m_wszChatRoomPRLine3[0] = L'\0';
	}

public:
	UINT m_nChatRoomID;
	WCHAR m_wszChatRoomName[CHATROOMNAME_MAX + 1];
	WCHAR m_wszChatRoomPRLine1[CHATROOMNAME_MAX + 1];
	WCHAR m_wszChatRoomPRLine2[CHATROOMNAME_MAX + 1];
	WCHAR m_wszChatRoomPRLine3[CHATROOMNAME_MAX + 1];
	BYTE m_nChatRoomType;
	bool m_bHasPassword;
};

struct CSCreateChatRoom
{
	WCHAR		wszName[CHATROOMNAME_MAX + 1];
	WCHAR		wszPassword[CHATROOMPASSWORDMAX + 1];

	BYTE		nRoomType;
	USHORT		nRoomAllow;

	WCHAR		PRLine1[CHATROOMNAME_MAX + 1];
	WCHAR		PRLine2[CHATROOMNAME_MAX + 1];
	WCHAR		PRLine3[CHATROOMNAME_MAX + 1];
};

struct CSChatRoomEnterRoom
{
	UINT		nChatRoomID;
	WCHAR		wszPassword[CHATROOMPASSWORDMAX + 1];	// Password
};

struct CSChatRoomChangeRoomOption
{
	TChatRoomView	ChatRoomView;

	WCHAR			wszPassword[CHATROOMPASSWORDMAX + 1];
	USHORT			nRoomAllow;
};

struct CSChatRoomKickUser
{
	int			nKickUserSessionID;		// SessionID of the character to kick
	int			nKickReason;			// CHATROOMLEAVE_KICKED or CHATROOMLEAVE_FOREVERKICKED
};

//----------------------------------------------------------------------------------
struct SCCreateChatRoom
{
	int				nRet;

	TChatRoomView	ChatRoomView;
};

struct SCChatRoomEnterRoom
{
	int				nRet;

	int				LeaderSID;
	int				UserSessionIDs[CHATROOMMAX];	// Participant list
};

struct SCChatRoomChangeRoomOption
{
	int				nRet;
	TChatRoomView	ChatRoomView;
};

struct SCChatRoomEnterUser
{
	UINT		nChatRoomID;
	BYTE		cChatRoomType;
	int			nUserSessionID;
};

struct SCChatRoomKickUser
{
	int			nRet;
	int			nKickedSessionID;
};

struct SCChatRoomLeaveUser
{
	int			nUserSessionID;
	BYTE		nLeaveReason;
};

struct SCChatRoomView
{
	int				nSessionID;		// Character that has a chat room
	TChatRoomView	ChatRoomView;	// Chat room brief info - if ChatRoomID is 0, the room is deleted.
};

struct SCROOM_SYNC_CHATROOMINFO
{
	UINT			nLeaderID;		// Character that has a chat room
	TChatRoomView	ChatRoomView;	// Chat room brief info - if ChatRoomID is 0, the room is deleted.
	int				nCount;
	UINT			nMembers[CHATROOMMAX];
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct SCReputationList
{
	BYTE				cCount;
	TNpcReputation		ReputationArr[30];
};

struct SCModReputation
{
	BYTE				cCount;
	TNpcReputation		UpdateArr[NpcReputation::Common::MaxEffectNpcCount];
};

struct SCOpenGiveNpcPresent
{
	UINT nNpcID;
};

struct SCRequestSendSelectedPresent
{
	UINT nNpcID;
};

struct SCShowNpcEffect
{
	UINT nNpcID;
	int nEffectIndex;
};

struct CSGiveNpcPresent
{
	UINT nNpcID;
	int nPresentID;		// itemid in the npc gift table
	int nPresentCount;  // Gift count
};

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct SCOpenDarkLairRankBoard
{
	UINT	uiNpcID;
	int		iMapIndex;
	BYTE	cPlayerCount;
};

struct SCOpenPvPLadderRankBoard
{
	UINT uiNpcID;
	LadderSystem::MatchType::eCode MatchType;
};

namespace MasterSystem
{
	struct SCSimpleInfo
	{
		TMasterSystemSimpleInfo SimpleInfo;
		bool bRefresh;	// Whether to refresh list
	};

	struct SCCountInfo
	{
		int		iMasterCount;
		int		iPupilCount;
		int		iClassmateCount;
	};

	struct CSMasterList
	{
		UINT	uiPage;		// Zerobase
		BYTE	cJob;		// 0: all
		BYTE	cGender;	// 0: all
	};

	struct SCMasterList
	{
		int			iRet;
		BYTE		cCount;
		TMasterInfo	MasterInfoList[MasterSystem::Max::MasterPageRowCount];
	};

	struct SCMasterCharacterInfo
	{
		int						iRet;
		TMasterCharacterInfo	MasterCharacterInfo;
	};

	struct SCPupilList
	{
		int						iRet;
		INT64					biMasterCharacterDBID;
		BYTE					cCount;
		TPupilInfoWithLocation	PupilInfoList[MasterSystem::Max::PupilCount];
	};

	struct CSIntroduction
	{
		bool	bRegister;	// Registration status
		WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
	};

	struct SCIntroduction
	{
		int		iRet;
		bool	bRegister;
		WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
	};

	struct CSMasterApplication
	{
		INT64	biCharacterDBID;
	};

	struct SCMasterApplication
	{
		int	iRet;
	};

	struct CSJoin
	{
		INT64	biMasterCharacterDBID;
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct SCJoin
	{
		int		iRet;
		bool	bIsDirectMenu;
		bool	bIsAddPupil;
		bool	bIsTransactor;	// Flag distinguishing whether actor is the performer
	};

	struct SCMasterClassmate
	{
		int									iRet;
		BYTE								cCount;
		TMasterAndClassmateInfoWithLocation	DataList[MasterSystem::Max::MasterAndClassmateCount];
	};

	struct CSClassmateInfo
	{
		INT64	biClassmateCharacterDBID;
	};

	struct SCClassmateInfo
	{
		int							iRet;
		TClassmateInfoWithLocation	ClassmateInfo;
	};

	struct CSMyMasterInfo
	{
		INT64	biMasterCharacterDBID;
	};

	struct SCMyMasterInfo
	{
		int							iRet;
		TMyMasterInfoWithLocation	MasterInfo;
	};

	struct CSLeave
	{
		INT64	biDestCharacterDBID;
		bool	bIsMaster;		// Expel apprentice: true, remove master: false
	};

	struct SCLeave
	{
		int		iRet;
		bool	bIsDelPupil;			// true: delete apprentice, false: delete master
		int		iPenaltyRespectPoint;	// If bIsDelPupil==true, respect points to decrease (positive value)
	};

	struct SCGraduate
	{
		WCHAR	wszCharName[NAMELENMAX];
	};

	struct SCConnect
	{
		bool	bIsConnect;
		WCHAR	wszCharName[NAMELENMAX];
	};

	struct SCRespectPoint
	{
		int iRespectPoint;
	};

	struct SCFavorPoint
	{
		INT64	biMasterCharacterDBID;
		INT64	biPupilCharacterDBID;
		int		iFavorPoint;
	};

	struct CSInvitePupil
	{
		WCHAR	wszCharName[NAMELENMAX];
	};

	struct SCInvitePupil
	{
		int		iRet;
		WCHAR	wszCharName[NAMELENMAX];
	};

	struct SCInvitePupilConfirm
	{
		int		iRet;
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct CSInvitePupilConfirm
	{
		bool	bIsAccept;
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct CSJoinDirect
	{
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct SCJoinDirect
	{
		int		iRet;
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct SCJoinDirectConfirm
	{
		int		iRet;
		WCHAR	wszPupilCharName[NAMELENMAX];
	};

	struct CSJoinDirectConfirm
	{
		bool	bIsAccept;
		WCHAR	wszPupilCharName[NAMELENMAX];
	};

	struct CSRecallMaster
	{
		WCHAR	wszMasterCharName[NAMELENMAX];
	};

	struct SCRecallMaster
	{
		int		iRet;
		bool	bIsConfirm;					// true: summon confirmation packet
		WCHAR	wszCharName[NAMELENMAX];	// bIsConfirm==true : pupil name, bIsConfirm==false : master name
	};

	struct CSBreakInto
	{
		int		iRet;
		WCHAR	wszPupilCharName[NAMELENMAX];
	};
	struct SCJoinComfirm
	{
		BYTE cLevel;				// Apprentice level
		BYTE cJob;					// Apprentice class
		WCHAR wszPupilCharName[NAMELENMAX];		// Apprentice name
	};
	struct CSJoinComfirm
	{
		bool	bIsAccept;
		WCHAR	wszPupilCharName[NAMELENMAX];	// Apprentice name
	};
}

#if defined( PRE_ADD_SECONDARY_SKILL )

namespace SecondarySkill
{
	struct SCList
	{
		BYTE				cCount;
		TSecondarySkillInfo	SkillList[SecondarySkill::Max::LearnSecondarySkill];
	};

	struct CSDelete
	{
		int iSkillID;
	};

	struct SCCreate
	{
		int	iRet;
		int iSkillID;
	};

	struct SCDelete
	{
		int iRet;
		int iSkillID;
	};

	struct SCUpdateExp
	{
		int	iRet;
		int iSkillID;
		SecondarySkill::Grade::eType Grade;
		int iExp;
		int iLevel;
	};

	struct SCRecipeList
	{
		BYTE					cCount;
		TSecondarySkillRecipe	RecipeList[SecondarySkill::Max::LearnManufactureRecipe];
	};

	struct CSAddRecipe
	{
		int		iSkillID;
		char	cInvenType;				// eItemPositionType
		BYTE	cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
		INT64	biInvenSerial;		
	};

	struct SCAddRecipe
	{
		int		iRet;
		int		iSkillID;
		int		iItemID;
		INT64	biItemSerial;
		short	nExp;
	};

	struct CSDeleteRecipe
	{
		int		iSkillID;
		int		iItemID;
	};

	struct SCDeleteRecipe
	{
		int		iRet;
		int		iSkillID;
		int		iItemID;
	};

	struct CSExtractRecipe
	{
		int		iSkillID;
		int		iItemID;
	};

	struct SCExtractRecipe
	{
		int		iRet;
		int		iSkillID;
		int		iItemID;
	};

	struct SCUpdateRecipeExp
	{
		int		iRet;
		int		iSkillID;
		int		iItemID;
		int		iExp;		// Final value (must be assigned)
	};

	struct CSManufacture
	{
		bool	bIsStart;
		int		iSkillID;
		int		iItemID;
	};

	struct SCManufacture
	{
		int		iRet;
		bool	bIsStart;
		int		iSkillID;
		int		iItemID;
	};

	struct SCCancelManufacture
	{
		int		iRet;
		int		iSkillID;
	};
}

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

#if defined (PRE_ADD_BESTFRIEND)

/* Check best friend status
bStatus : 0 / bCancel 0
-> Anyone can break the best-friend bond
bStatus : 1 / bCancel 1
-> Can cancel because best-friend breakup is in progress
bStatus : 1 / when tEndDate exceeds current time by 4 days
-> Delete best friend
*/
struct TBestFriendInfo
{
	// Common
	__time64_t tStartDate;
	__time64_t tEndDate;
	int nDay; // Day
	bool bStatus; // 0: Best friend, 1: Break off
	INT64 biWaitingTimeForDelete; // Deletion wait time

	// Me
	INT64 biItemSerial;
	WCHAR wszMyMemo[BESTFRIENDMEMOMAX];
	bool bCancel; // 0: Character whose best-friend status was broken off, 1: Character who broke off best-friend status

	// Friend
	UINT nSessionID;
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];
	WCHAR wszFriendMemo[BESTFRIENDMEMOMAX];
	char cLevel;
	char cJob;
	TCommunityLocation Location;
};

namespace BestFriend
{
	struct CSSearch
	{
		WCHAR wszName[NAMELENMAX];
	};

	struct CSRegist
	{
		INT64 biInvenSerial;
		WCHAR wszBestFriendName[NAMELENMAX];
	};

	struct CSAccept
	{
		bool bAccept;
		UINT nFromAccountDBID;
		INT64 biFromCharacterDBID;
		WCHAR wszFromName[NAMELENMAX];
	};

	struct CSCancel
	{
		bool bCancel;
	};

	struct CSEditMemo
	{
		WCHAR wszMemo[BESTFRIENDMEMOMAX];
	};

	struct SCGetInfo
	{
		int iRet;

		TBestFriendInfo Info;
	};

	struct SCSearch
	{
		int iRet;
		char cLevel;
		char cJob;
		WCHAR wszName[NAMELENMAX];
	};

	struct SCRegistReq
	{
		int iRet;

		// Requester
		UINT nFromAccountDBID;	
		INT64 biFromCharacterDBID;
		WCHAR wszFromName[NAMELENMAX];

		// Responder
		UINT nToAccountDBID;	
		INT64 biToCharacterDBID;
		WCHAR wszToName[NAMELENMAX];
	};

	struct SCRegistAck
	{
		int iRet;
		UINT nFromAccountDBID;	// Responder
		UINT nToAccountDBID;	// Requester
		WCHAR wszFromName[NAMELENMAX];
		WCHAR wszToName[NAMELENMAX];
	};

	struct SCComplete
	{
		int iRet;
		WCHAR wszName[NAMELENMAX];
	};

	struct SCEditMemo
	{
		int iRet;
		bool bFromMe;
		WCHAR wszMemo[BESTFRIENDMEMOMAX];
	};

	struct SCCancel
	{
		int iRet;
		bool bCancel;
		WCHAR wszName[NAMELENMAX];
	};

	struct SCClose
	{
		int iRet;
		WCHAR wszName[NAMELENMAX];
	};
}
#endif

//--------------------------------------------------------------------------------------------------
//	Etc
//--------------------------------------------------------------------------------------------------

struct TEvent
{
	int		nEventID;
	time_t _tBeginTime;
	time_t _tEndTime;

	union 
	{
		struct
		{
			int nEventType1;
			int nEventType2;
			int nEventType3;
		};
		int nEventType[3];
	};

	union
	{
		struct
		{
			int nAtt1;
			int nAtt2;
			int nAtt3;
			int nAtt4;
			int nAtt5;
		};
		int nAtt[5];
	};
};

struct SCIncreaseLife
{
	char cType;
	int nIncreaseLife;
};

struct SCSpecialRebirthItem
{
	int nItemID;
	int nRebirthItemCount;
};
// TIMEEVENT
//--------------------------------------------------------------------------------------------------
//	TIMEEVENT (CS_TIMEEVENT, SC_TIMEEVENT)
//--------------------------------------------------------------------------------------------------
struct SCTimeEventList
{
	int nCount;
	TTimeEvent Event[TIMEEVENTMAX];
};

struct SCTimeEventAchieve
{
	int nItemID;
	INT64 nRemainTime;
};

struct SCTimeEventExpire
{
	int nItemID;
};

struct CSAttendanceEvent
{
	bool bIsOK;
};

struct SCAttendanceEvent
{
	bool bCheckAttendanceFirst;
};

//PlayerCustomEventUI
//--------------------------------------------------------------------------------------------------
//	PlayerCustomEventUI (CS_CUSTOMEVENTUI, SC_CUSTOMEVENTUI)
//--------------------------------------------------------------------------------------------------
struct CSGetPlayerCustomEventUI
{
	UINT nSessionID;
};

struct TCustomEventInfo
{
	int nMissionID;
	BYTE cFlag;
};

struct SCGetPlayerCustomEventUI
{
	UINT nSessionID;
	BYTE cCount;
	TCustomEventInfo UIs[PlayerCustomEventUI::PlayerCustomEventUIMax];
};

//--------------------------------------------------------------------------------------------------
//	Guild recruitment board (CS_GUILDRECRUIT, SC_GUILDRECRUIT)
//--------------------------------------------------------------------------------------------------
namespace GuildRecruitSystem
{
	struct CSGuildRecruitList
	{
		UINT	uiPage;	
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		BYTE	cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
		WCHAR	wszGuildName[GUILDNAME_MAX];		//Guild homepage
		BYTE	cSortType;  
#endif
	};
	struct SCGuildRecruitList
	{
		int			iRet;
		UINT		uiPage;
		BYTE		cCount;
		TGuildRecruitInfo	GuildRecruitList[GuildRecruitSystem::Max::GuildRecruitRowCount];
	};
	struct SCGuildRecruitCharacterList
	{
		int			iRet;
		BYTE		cCount;
		TGuildRecruitCharacter	GuildRecruitCharacterList[GuildRecruitSystem::Max::GuildRecruitCharacterRowCount];
	};	
	struct SCMyGuildRecruitList
	{
		int			iRet;
		BYTE		cCount;
		TGuildRecruitInfo	MyGuildRecruitList[GuildRecruitSystem::Max::MaxRequestCount];
	};
	struct SCGuildRecruitRequestCount
	{
		int		iRet;
		BYTE	cRequestCount;
		BYTE	cMaxRequestCount;
	};
	struct SCGuildRecruitRegisterInfo
	{		
		BYTE		cClassGrade[CLASSKINDMAX];	// Recruiting class
		int			nMinLevel;					// Min level
		int			nMaxLevel;					// Max level
		WCHAR		wszGuildIntroduction[GUILDRECRUITINTRODUCE+1];	//Guild introduction
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		BYTE		cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
		bool		bCheckHomePage;		//Guild homepage
#endif
	};
	struct CSGuildRecruitRegister
	{
		BYTE		cRegisterType;				// Register Modify Delete
		BYTE		cClassGrade[CLASSKINDMAX];	// Recruiting class
		int			nMinLevel;					// Min level
		int			nMaxLevel;					// Max level
		WCHAR		wszGuildIntroduction[GUILDRECRUITINTRODUCE+1];	//Guild introduction
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
		BYTE		cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
		bool		bCheckHomePage;		//Guild homepage
#endif
	};
	struct SCGuildRecruitRegister
	{
		int			iRet;
		BYTE		cRegisterType;				// Register Modify Delete
	};
	struct CSGuildRecruitRequest
	{		
		BYTE		cRequestType;				// Apply/cancel join request
		TGuildUID	GuildUID;					// Guild UID (World ID + Guild DBID)
	};
	struct SCGuildRecruitRequest
	{		
		int			iRet;
		BYTE		cRequestType;				// Apply/cancel join request
	};
	struct CSGuildRecruitAccept
	{		
		BYTE		cAcceptType;				// Approve/reject join request
		TGuildUID	GuildUID;					// Guild UID (World ID + Guild DBID)
		INT64		biAcceptCharacterDBID;		// DBID of character who approved join
		WCHAR		wszToCharacterName[NAMELENMAX];		// Name of character who approved join
	};
	struct SCGuildRecruitAccept
	{
		int			iRet;
		BYTE		cAcceptType;
		INT64		biAcceptCharacterDBID;		// DBID of character who approved join		
		bool		bDelGuildRecruit;			// Whether to delete guild recruitment entry
	};
	struct SCGuildRecruitMemberResult
	{
		int			iRet;
		BYTE		cAcceptType;
		WCHAR		wszGuildName[GUILDNAME_MAX];	// Guild name
	};
};

struct CSWarpVillageList
{
	INT64 nItemSerial;
};

struct SCWarpVillageList
{
	BYTE btCount;
	WarpVillage::WarpVillageInfo VillageList[128];
};

struct CSWarpVillage
{
	int nMapIndex;
	INT64 nItemSerial;
};

struct SCWarpVillage
{
	int nResult;
};

#if defined (PRE_ADD_DONATION)
struct CSDonate
{
	INT64 nCoin;
};

struct SCDonate
{
	int nResult;
};

struct CSDonationRanking
{

};

struct SCDonationRanking
{
	Donation::Ranking Ranking[Donation::MAX_RANKING];
	INT64 nMyCoin;
	int nMyRanking;
};
#endif // #if defined (PRE_ADD_DONATION)

#ifdef PRE_ADD_AUTOUNPACK

struct CSAutoUnpack
{
	bool bShow;
	INT64 serial;
};

struct SCAutoUnpack
{	
	int nRetCode;
	INT64 serial;
};

#endif

#ifdef PRE_ADD_EXCHANGE_POTENTIAL
struct CSExchangePotential		//CS_ITEMGOODS : CS_EXCHANGE_POTENTIAL
{
	int nExtractItemIndex;
	INT64 biExtractItemSerial;

	int nInjectItemIndex;
	INT64 biInjectItemSerial;
};

struct SCExchangePotential		//SC_ITEMGOODS : SC_EXCHANGE_POTENTIAL
{
	int nRetCode;
};
#endif		//#ifdef PRE_ADD_EXCHANGE_POTENTIAL

#if defined(PRE_ADD_EXCHANGE_ENCHANT)
struct CSExchangeEnchant		//CS_ITEMGOODS : CS_EXCHANGE_ENCHANT
{
	int nExtractItemIndex;
	INT64 biExtractItemSerial;

	int nInjectItemIndex;
	INT64 biInjectItemSerial;
};

struct SCExchangeEnchant			//SC_ITEMGOODS : SC_EXCHANGE_ENCHANT
{
	int nRetCode;
};
#endif

#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
struct SCPcCafeRentItem				// SC_PCCAFE_RENT_ITEM
{
	int nRetCode;				// Result
	int nNeedCount;				// Required inventory slots
};
#endif // #if defined(PRE_ADD_PCBANG_RENTAL_ITEM)

#ifdef PRE_ADD_DOORS
struct SCDoorsGetAuthKey
{
	int nRetCode;
	bool bMobileAuthentication;
	WCHAR wszAuthKey[Doors::Common::AuthenticationKey_LenMax];
};

struct SCDoorsCancelAuth
{
	int nRetCode;
};

struct SCDoorsGetAuthFlag
{
	bool bFlag;
};
#endif		//#ifdef PRE_ADD_DOORS

#if defined( PRE_PRIVATECHAT_CHANNEL )
namespace PrivateChatChannel
{
	struct SCPrivateChannleInfo
	{
		int nCount;
		WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];
		PrivateChatChannel::TMemberInfo Members[PrivateChatChannel::Common::MaxMemberCount];
	};

	struct SCPrivateChatChannleResult	// Handle all result values for CS packets here
	{
		int nRet;		
	};

	struct CSPrivateChatChannleAdd	// CS_PRIVATECHAT_CHANNEL_ADD
	{
		WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];		
#if defined( PRE_ADD_PRIVATECHAT_CHANNEL )
		int nPassWord;
#endif
	};	

	struct CSPrivateChatChannleJoin	// CS_PRIVATECHAT_CHANNEL_JOIN
	{
		WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];
		int nPassWord;
	};

	struct CSPrivateChatChannleInvite	// CS_PRIVATECHAT_CHANNEL_INVITE
	{
		WCHAR wszCharacterName[NAMELENMAX];
	};
	
	struct SCPrivateChatChannleInviteResult	// SC_PRIVATECHAT_CHANNEL_INVITERESULT
	{
		int nRet;
	};

	struct SCPrivateChatChannleJoinResult	// SC_PRIVATECHAT_CHANNEL_JOINRESULT
	{
		int nRet;
		WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];		
	};

	struct CSPrivateChatChannleOut	// CS_PRIVATECHAT_CHANNEL_OUT 
	{	
	};	

	struct SCPrivateChatChannleOutResult	// SC_PRIVATECHAT_CHANNEL_OUTRESULT 
	{	
		int nRet;
	};

	struct CSPrivateChatChannleKick	// CS_PRIVATECHAT_CHANNEL_KICK
	{			
		WCHAR wszCharacterName[NAMELENMAX];
	};
	
	struct CSPrivateChatChannleKickResult	// SC_PRIVATECHAT_CHANNEL_KICKRESULT
	{			
		int nRet;
		bool bMe;
	};

	struct SCPrivateChatChannleAdd // SC_PRIVATECHAT_CHANNEL_ADD
	{
		int nRet;
		WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];
		PrivateChatChannel::TMemberInfo Member;
	};	

	struct SCPrivateChatChannleDel // SC_PRIVATECHAT_CHANNEL_DEL
	{
		int nRet;			
		INT64 biCharacterDBID;
		WCHAR wszCharacterName[NAMELENMAX];
	};	

	struct SCPrivateChatChannleKick	// SC_PRIVATECHAT_CHANNEL_KICK
	{
		int nRet;		
		INT64 biCharacterDBID;
	};

	struct CSPrivateChatChannleMod	// CS_PRIVATECHAT_CHANNEL_MOD
	{
		int nModType;
		int nPassWord;
		INT64 biCharacterDBID;
	};

	struct SCPrivateChatChannleMod	// SC_PRIVATECHAT_CHANNEL_MOD
	{
		int nRet;
		int nModType;
		INT64 biCharacterDBID;
		WCHAR wszCharacterName[NAMELENMAX];
	};
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )

namespace WorldPvPMissionRoom
{
	struct SCWorldPvPRoomStartMsg	// SC_PVP_WORLDPVPROOM_STARTMSG
	{
		bool bShow;
		UINT uiPvPIndex;
	};

	struct CSWorldPvPRoomStartMsg	// CS_PVP_WORLDPVPROOM_STARTMSG
	{		
		UINT uiPvPIndex;
	};

	struct SCWorldPvPRoomStartReturn	// SC_PVP_WORLDPVPROOM_STARTMSG
	{		
		int nRet;	
	};

	struct SCWorldPvPRoomJoinResult	// SC_PVP_WORLDPVPROOM_STARTMSG
	{		
		int nRet;	
	};

	struct SCWorldPvPRoomAllKillTeamInfo	// SC_PVP_WORLDPVPROOM_STARTMSG
	{		
		UINT nSessionID[PARTYMAX];
		int nTeam[PARTYMAX];
	};

	struct SCWorldPvPRoomTournamentUserInfo
	{		
		BYTE					cUserCount;				// User count
		PvPCommon::UserInfoList	UserInfoList;
	};
}

#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
struct SCPvPComboExerciseRoomMasterInfo // SC_PVP_COMBOEXERCISE_ROOMMASTERINFO
{
	UINT uiRoomMasterSessionID;
};

struct SCPvPComboExerciseChangeRoomMaster // SC_PVP_COMBOEXERCISE_CHANGEROOMMASTER
{
	UINT uiRoomMasterSessionID;
};

struct SCPvPComboExerciseRecallMonsterResult // SC_PVP_COMBOEXERCISE_RECALLMONSTER
{
	int nRet;
};
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

#if defined( PRE_ALTEIAWORLD_EXPLORE )

namespace AlteiaWorld
{
	struct CSAlteiaWorldInfo // CS_ALTEIAWORLD_INFO
	{
		AlteiaWorld::Info::eInfoType eType;
	};

	struct SCAlteiaWorldJoinInfo // SC_ALTEIAWORLD_JOIN_INFO
	{	
		BYTE cWeeklyClearCount;
		BYTE cDailyClearCount;
		BYTE cTicketCount;		
	};

	struct SCAlteiaWorldPrivateGoldKeyRankingInfo // SC_ALTEIAWORLD_PRIVATEGOLDKEYRANKING_INFO
	{
		int nMyBestGoldKeyCount;
		int nCount;
		AlteiaWorld::GoldKeyRankMemberInfo MemberInfo[AlteiaWorld::Common::MaxRankCount];
	};

	struct SCAlteiaWorldPrivatePlayTimeRankingInfo // SC_ALTEIAWORLD_PRIVATEPLAYTIMERANKING_INFO
	{
		UINT nPlaySec;		
		int nCount;
		AlteiaWorld::PlayTimeRankMemberInfo MemberInfo[AlteiaWorld::Common::MaxRankCount];
	};

	struct SCAlteiaWorldGuildGoldKeyRankingInfo // SC_ALTEIAWORLD_GUILDGOLDKEYRANKING_INFO
	{
		int nMyBestGoldKeyCount;	
		int nCount;
		AlteiaWorld::GuildGoldKeyRankInfo GuildRankInfo[AlteiaWorld::Common::MaxRankCount];
	};

	struct SCAlteiaWorldSendTicketInfo // SC_ALTEIAWORLD_GUILDGOLDKEYRANKING_INFO
	{
		BYTE cSendTicketCount;
		INT64 biCharacterDBID[AlteiaWorld::Common::MaxSendCount];
	};

	struct CSAlteiaWorldJoin // CS_ALTEIAWORLD_JOIN
	{	
	};

	struct SCAlteiaWorldJoinResult
	{
		int nRet;
	};	

	struct CSAlteiaWorldSendTicket // CS_ALTEIAWORLD_SENDTICKET
	{	
		WCHAR wszCharacterName[NAMELENMAX];
	};

	struct SCAlteiaWorldSendTicket // SC_ALTEIAWORLD_SENDTICKET
	{	
		WCHAR wszCharacterName[NAMELENMAX];
		int nTicketCount;
	};

	struct SCAlteiaWorldSendTicketResult // SC_ALTEIAWORLD_SENDTICKET_RESULT
	{	
		int nRet;
		int nSendTicketCount;
	};

	struct CSAlteiaWorldDice // CS_ALTEIAWORLD_DICE
	{
	};

	struct SCAlteiaWorldDiceResult // SC_ALTEIAWORLD_DICE_RESULT
	{
		int nRet;
		int nNumber;
	};
}

#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )

#if defined( PRE_ADD_STAMPSYSTEM )
namespace StampSystem
{
	struct SCStampSystemInit
	{
		int nWeekDay;
		INT64 biTableStartTime;
		bool bCompleteFlagData[Common::MaxStampSlotCount];
	};

	struct SCStampSystemAddComplete
	{
		int nChallengeIndex;
		int nWeekDay;
	};

	struct SCStampSystemChangeWeekDay
	{
		int nWeekDay;
	};

	struct SCStampSystemChangeTable
	{
		INT64 biTableStartTime;
	};
}
#endif // #if defined( PRE_ADD_STAMPSYSTEM )

#if defined( PRE_ADD_PVP_EXPUP_ITEM )
struct SCUsePvPExpup
{
	int nRet;
	BYTE cLevel;
	UINT uiXP;
	int nIncrement; 
};
#endif // #if defined( PRE_ADD_PVP_EXPUP_ITEM )

#if defined( PRE_DRAGONBUFF )
struct SCWorldBuffMsg
{
	WCHAR wszCharacterName[NAMELENMAX];
	int nItemID;
};
#endif

#if defined(PRE_ADD_GUILD_CONTRIBUTION)
namespace GuildContribution
{
	// SC_GUILD / SC_GUILD_CONTRIBUTION_POINT
	struct SCGuildContributionPoint
	{
		int nWeeklyContributionPoint;	//Weekly guild contribution
		int nTotalContributionPoint;	//Total guild contribution
	};

	// SC_GUILD / SC_GUILD_CONTRIBUTION_RANK
	struct SCGuildContributionRank
	{
		int nCount;	// Number of contribution rankers
		GuildContribution::TGuildContributionRankingData Data[GUILDSIZE_MAX]; // Contribution ranker info
	};
}
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)

//--------------------------------------------------------------------------------------------------
//	DWC (CS_DWC, SC_DWC)
//--------------------------------------------------------------------------------------------------
#if defined( PRE_ADD_DWC )
namespace DWC
{
	const int MAX_TEAMNAME_LEN = 21;
	const int MAX_TEAM_MEMBER = 4;
	const int MAX_RANK_PER_PAGE = 10;
	const int MAX_FIND_NAME_LEN = 38;
}

struct TDWCTeamInfo
{
	UINT nTeamID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
	int nDWCPoint;
	int nTotalWin;
	int nTotalLose;
	int nTotalDraw;
	int nTodayPlayCount;
	int nWeeklyPlayCount;
	int nRank;
};

struct TDWCTeamMemberInfo
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
	BYTE cJobCode;
	BYTE cLevel;
	bool bTeamLeader;
	int nMapIndex;
	int nChannelID;
	BYTE cConnectState;
};

struct TDWCRankInfo
{
	int nRank;
	UINT nTeamID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
	WCHAR wszLeaderName[NAMELENMAX];
	BYTE cLeaderJobCode;
	int nDWCPoint;
	int nTotalWin;
	int nTotalLose;
	int nTotalDraw;
};

struct TDWCChannelInfo
{
	BYTE cStatus;
	int nSeasonID;
	int nMaxWeeklyCount;
	int nMatchStartHour;
	int nMatchEndHour;
	bool bOpen;
};

struct CSDWCCreateTeam				// CS_DWC / CS_DWC_CREATETEAM
{
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
};

struct CSDWCInviteMember			// CS_DWC / CS_DWC_INVITE_MEMBER
{
	WCHAR wszCharacterName[NAMELENMAX];
};

struct CSDWCInviteMemberAck			// CS_DWC / CS_DWC_INVITE_MEMBER_ACK
{
	UINT nTeamID;
	UINT nFromSessionID;
	bool bAccept;
};

struct CSDWCGetTeamInfo				// CS_DWC / CS_DWC_GET_TEAMINFO
{
	bool bNeedMemberList;
};

struct CSDWCGetRankPage				// CS_DWC / CS_DWC_GET_RANKPAGE
{
	int nPageNum;
};

struct CSDWCFindRank				// CS_DWC / CS_DWC_FIND_RANK
{
	BYTE cFindType;
	WCHAR wszFindName[DWC::MAX_FIND_NAME_LEN];
};

struct SCDWCCreateTeam				// SC_DWC / SC_DWC_CREATETEAM
{
	int nRetCode;
	UINT nTeamID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
};

struct SCDWCLeaveTeam				// SC_DWC / SC_DWC_LEAVETEAM
{
	int nRetCode;
	UINT nTeamID;
	INT64 biLeaveCharDBID;
	WCHAR wszLeaveCharName[NAMELENMAX];
};

struct SCDWCDismissTeam				// SC_DWC / SC_DWC_DISMISS_TEAM
{
	int nRetCode;
	UINT nTeamID;
};

struct SCDWCInviteMemberReq			// SC_DWC / SC_DWC_INVITE_MEMBER_REQ
{
	UINT nTeamID;
	UINT nFromSessionID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
	WCHAR wszFromCharName[NAMELENMAX];
};

struct SCDWCInviteMemberAck			// SC_DWC / SC_DWC_INVITE_MEMBER_ACK
{
	int nRetCode;
	WCHAR wszTargetCharName[NAMELENMAX];
};

struct SCDWCGetTeamInfo				// SC_DWC / SC_DWC_GET_TEAMINFO
{
	int nRetCode;
	TDWCTeamInfo TeamInfo;
};

struct SCDWCGetTeamMember			// SC_DWC / SC_DWC_GET_TEAMMEMBER
{
	int nRetCode;
	UINT nTeamID;
	int nCount;
	TDWCTeamMemberInfo MemberList[DWC::MAX_TEAM_MEMBER];
};

struct SCDWCGetRankPage				// SC_DWC / SC_DWC_GET_RANKPAGE
{
	int nRetCode;
	int nTotalCount;
	int nPageNum;
	int nCount;
	TDWCRankInfo RankList[DWC::MAX_RANK_PER_PAGE];
};

struct SCDWCFindRank				// SC_DWC / SC_DWC_FIND_RANK
{
	int nRetCode;
	TDWCRankInfo RankInfo;
};

struct SCDWCUpdateMemberState		// SC_DWC / SC_DWC_UPDATE_MEMBERSTATE
{
	UINT nTeamID;
	INT64 biCharacterDBID;
	int nMapIndex;
	int nChannelID;
	BYTE cConnectState;
};

struct SCDWCChannelInfo				// SC_DWC / SC_DWC_CHANNELINFO
{
	TDWCChannelInfo ChannelInfo;
};

struct SCDWCTeamNameInfo			// SC_DWC / SC_DWC_TEAMNAME_INFO
{
	UINT nSessionID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
};

struct SCDWCScoreInfo				// SC_DWC / SC_DWC_SCORE_INFO
{
	int nDWCPoint;
	int nTotalWin;
	int nTotalLose;
	int nTotalDraw;
};

struct SCDWCGradeInfo				// SC_DWC / SC_DWC_GRADE_INFO
{
	int nRank;
	BYTE cGrade;
};
#endif // #if defined( PRE_ADD_DWC )


#pragma pack(pop)
