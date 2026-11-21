#pragma once

#include "DNServerPacketCash.h"
#include "Util.h"

#pragma pack(push, 1)

struct TQHeader
{
	BYTE cThreadID;
	char cWorldSetID;
	UINT nAccountDBID;
};

struct TAHeader
{
	UINT nAccountDBID;
	int nRetCode;
};

//--------------------------------------------------------------
//	DB Packet
//--------------------------------------------------------------

namespace DBPacket
{
	struct TSkillCoolTime
	{
		int nSkillID;
		int nCoolTime;
	};

	struct TItemCoolTime
	{
		INT64 biItemSerial;
		int nCoolTime;
	};

	struct TDurabilityInfo
	{
		INT64 biSerial;
		USHORT wDur;
	};

	struct TItemCountInfo
	{
		INT64 biSerial;
		short wCount;
	};

	struct TExchangeItem
	{
		BYTE cSlotIndex;
		int nItemID;
		INT64 biSerial;
		INT64 biNewSerial;
		short wCount;
	};

	struct TSendMailItem: public TItemInfo
	{
		INT64 biNewSerial;
	};

	struct TAttachMail
	{
		int nMailDBID;
		char cItemAttachCount;	// Attached item count
		INT64 biAttachCoin;		// Attached coin amount
#ifdef PRE_ADD_JOINGUILD_SUPPORT
		BYTE cMailType;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
		TItem Item[MAILATTACHITEMMAX];
	};

	struct TRequestTakeAttachInfo
	{
		int nMailDBID;
		char cAttachSlotIndex;		// -1: Claim all 5 slots, 0 - 4: Claim individually		
		bool bAttachCoin;
		INT64 biSerial[MAILATTACHITEMMAX];
		BYTE cSlotIndex[MAILATTACHITEMMAX];
	};

	struct TTakeAttachInfo
	{
		int nMailDBID;
		INT64 biAttachCoin;		// Attached coin amount
#ifdef PRE_ADD_JOINGUILD_SUPPORT
		bool bAddGuildWare;
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
		TItemInfo TakeItem[MAILATTACHITEMMAX];
	};

	struct TPetalInfo
	{
		char cSlotIndex;
		int nItemSN;
		INT64 biSerial;
		int nItemID;
		int nPetal;
	};

	struct TPetalInfoEx: TPetalInfo
	{
		int nFKey;
	};

	struct TCashFailItem
	{
		INT64 biPurchaseOrderID;
		INT64 biSenderCharacterDBID;
		bool bGift;
		int nItemSN;
		int nItemID;
		char cItemOption;
		int nPaidCashAmount;
	};

	struct TModCashFailItemEx: TCashItemBase
	{
		INT64 biPurchaseOrderID;
		INT64 biSenderCharacterDBID;
		bool bGift;
	};

	struct TMaterialItemInfo
	{
		DBDNWorldDef::ItemLocation::eCode Code;
		TItemInfo ItemInfo;
	};

	struct TVehicleInfo: public TVehicleItem, TVehicleBase
	{
		int nPetIndex;
		__time64_t tLastHungerModifyDate;
	};

	struct TAddMaterializedPet
	{
		BYTE cSlotIndex;
		int nItemID;
		INT64 biItemSerial;
	};

	struct TItemIndexSerial
	{
		BYTE cSlotIndex;
		INT64 biItemSerial;
	};

	struct TItemIDOption
	{
		int nItemID;
		char cOption;
	};

	struct TItemSNIDOption
	{
		int nItemSN;
		int nItemID;
		char cOption;
	};

	struct TReceiveGift
	{
		TReceiveGiftData GiftData;
		int nItemSN;
		DBPacket::TItemIDOption ItemIDOptions[PACKAGEITEMMAX];
		bool bFail;
	};
};

struct TDBListCharData
{
#if defined(PRE_MOD_SELECT_CHAR)
	BYTE cWorldID;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	INT64 biCharacterDBID;
#if !defined(PRE_MOD_SELECT_CHAR)
	BYTE cCharIndex;
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	WCHAR wszCharacterName[NAMELENMAX];
	BYTE cLevel;
	BYTE cJob;
	int	nLastVillageMapID;
	int	nLastSubVillageMapID;
	int	nMapID;
	int	nDefaultBody;	// Equip set on initial creation
	int	nDefaultLeg;
	int	nDefaultHand;
	int	nDefaultFoot;
	DWORD dwHairColor;
	DWORD dwEyeColor;
	DWORD dwSkinColor;
	bool bVillageFirstVisit;
	int	nChecksum;
	int	nEquipArray[EQUIPMAX];
	int	nCashEquipArray[CASHEQUIPMAX];
	bool bDeleteFlag;
	__time64_t DeleteDate;
	__time64_t LastLoginDate;
	__time64_t CreateDate;
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCombackEffectItem;
#endif
};

struct TCreateCharacterItem
{
	int nItemID;
	BYTE cCount;
	USHORT wDur;
};

struct TRestraintForAccountAndCharacter
{
	char cLevelCode;
	__time64_t _tStartDate;
	__time64_t _tEndDate;
	WCHAR wszRestraintReason[RESTRAINTREASONMAX];
	int iReasonID;
};

//#########################################################################################################################################
//	MAINCMD_ETC
//#########################################################################################################################################

// Query-Queue
struct TDBQueryData : public DNTPacketHeader
{
public:
	void Set(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen)
	{
		DN_ASSERT(iMainCmd < 255,	"Invalid!");
		DN_ASSERT(iSubCmd < 255,	"Invalid!");
		DN_ASSERT(NULL != cBuf,		"Invalid!");
		DN_ASSERT(uLen > 0,			"Invalid!");

		iLen = uLen;
		cMainCmd = iMainCmd;
		cSubCmd = iSubCmd;
		::memcpy(buf, cBuf, uLen);
	}
	void Reset()
	{
		iLen = 0;
		cMainCmd = 0;
		cSubCmd = 0;
	}
public:
	char buf[1];	// Intentionally matched the name with DNTPacket::buf to align TDBQueryData structure with DNTPacket structure, assuming minimum buffer size of 1
};

struct TDBQueryQueue
{
public:
	TDBQueryQueue() {
		Reset();
	}
	bool IsEmpty() const
	{
		return(0 >= m_nSize);
	}
	bool IsFull() const
	{
		return(sizeof(m_Buffer) <= m_nSize);
	}
	bool CheckIntegrity() const
	{
		if (IsEmpty()) {
			return false;
		}
		if (GetSize() > sizeof(*this)) {
			return false;
		}
		return true;
	}
	void Reset()
	{
		m_nCheckSum = DBQUERYAUTOCHKSUM;	// 20091022 Temporarily added for integrity check during packet processing ??? (b4nfter)
		m_nSize = 0;
		/*
		// P.S.> Must thoroughly pre-check with IsEmpty()
		for (int iIndex = 0 ; _countof(m_Queue) > iIndex ; ++iIndex) {
		m_Queue[iIndex].Reset();
		}
		*/
	}
	bool Add(const int iMainCmd, const int iSubCmd, const char *cBuf, const unsigned short uLen)
	{
		DN_ASSERT(iMainCmd < 255,	"Invalid!");
		DN_ASSERT(iSubCmd < 255,	"Invalid!");
		DN_ASSERT(NULL != cBuf,		"Invalid!");

		if (MAINCMD_ETC == iMainCmd && DB::QUERY_UPDATEQUERYQUEUE == iSubCmd) {
			DN_ASSERT(0,	"Invalid!");	// This packet must never be requested as a DB task
		}

		if (IsFull() || static_cast<int>(sizeof(DNTPacketHeader) + uLen) > GetFreeBufferSize()) {
			DN_RETURN(false);
		}

		TDBQueryData* pQueryData = reinterpret_cast<TDBQueryData*>(&m_Buffer[m_nSize]);
		DN_ASSERT(NULL != pQueryData,	"Invalid!");

		pQueryData->Set(iMainCmd, iSubCmd, cBuf, uLen);

		m_nSize += (sizeof(DNTPacketHeader) + uLen);
		DN_ASSERT(GetTotalBufferSize() >= m_nSize,	"Invalid!");

		return true;
	}
	TDBQueryData* At(const int nCurPtr)
	{
		DN_ASSERT(GetTotalBufferSize() >= m_nSize,	"Invalid!");
		DN_ASSERT(m_nSize > static_cast<int>(nCurPtr + sizeof(TDBQueryData)),	"Invalid!");

		return(reinterpret_cast<TDBQueryData*>(&m_Buffer[nCurPtr]));
	}
	const TDBQueryData* At(const int nCurPtr) const
	{
		return(const_cast<TDBQueryQueue*>(this)->At(nCurPtr));
	}
	int GetTotalBufferSize() const
	{ 
		return(sizeof(m_Buffer));
	}
	int GetFreeBufferSize() const
	{ 
		return(sizeof(m_Buffer) - m_nSize);
	}
	int GetSize() const
	{
		return(static_cast<int>((sizeof(*this) - (sizeof(this->m_Buffer[0]) * (_countof(m_Buffer) - m_nSize)))&ULONG_MAX));
	}
public:
	unsigned int m_nCheckSum;	// Temporary ???
	int m_nSize;
	char m_Buffer[DBQUERYSTREAMDATAMAX];
};

//QUERY_EVENTLIST
struct TQEventList:public TQHeader
{

};

struct TEventInfo
{
	int EventID;
	int WroldID;
#if defined(PRE_ADD_WORLD_EVENT)
#else
	WCHAR wszMapIndex[EVENTMAPSTRMAX];
#endif //#if defined(PRE_ADD_WORLD_EVENT)
	time_t _tBeginTime;
	time_t _tEndTime;

	union 
	{
		struct
		{
			int EventType1;
			int EventType2;
			int EventType3;
		};
		int EventType[3];
	};

	union
	{
		struct
		{
			int Att1;
			int Att2;
			int Att3;
			int Att4;
			int Att5;
		};
		int Att[5];
	};
};

struct TAEventList: public TAHeader
{
	char cWorldSetID;
	BYTE cCount;
	TEventInfo EventList[EVENTLISTMAX];
};

#ifdef PRE_MOD_RESTRICT_IDENTITY_IP
struct TQSimpleConfig : public TQHeader
{
};

struct TASimpleConfig : public TAHeader
{
	BYTE cCount;
	SimpleConfig::ConfigData Configs[SimpleConfig::ConfigDataCountMax];
};
#endif		//#ifdef PRE_MOD_RESTRICT_IDENTITY_IP

struct TQGetDBSID : public TQHeader
{

};

struct TAGetDBSID : public TAHeader
{
	int nServerID;
};

// QUERY_LOGOUT
struct TQLogout:public TQHeader
{
	UINT nSessionID;
};

// QUERY_LOGINCHARACTER
struct TQLoginCharacter:public TQHeader
{
	INT64 biCharacterDBID;
	UINT uiSessionID;
	WCHAR wszIP[IPLENMAX];
};

// QUERY_LOGOUTCHARACTER
struct TQLogoutCharacter:public TQHeader
{
	INT64 biCharacterDBID;
	UINT uiSessionID;
	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];	// Handle normal/cash equipment as a bitmap
};

// GameOption ----------------------------------------------------------------------
// QUERY_GETGAMEOPTION
struct TQGetGameOption:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetGameOption: public TAHeader
{
	INT64 biCharacterDBID;
	TGameOptions Option;
};

//QUERY_SETGAMEOPTION
struct TQSetGameOption:public TQHeader
{
	INT64 biCharacterDBID;
	TGameOptions Option;
};

struct TQUpdateSecondAuthPassword:public TQHeader
{
	WCHAR wszOldPW[SecondAuth::Common::PWMaxLength+1];
	WCHAR wszNewPW[SecondAuth::Common::PWMaxLength+1];
};

struct TAUpdateSecondAuthPassword: public TAHeader
{
	char cFailCount;
};

struct TQUpdateSecondAuthLock:public TQHeader
{
	bool bIsLock;
	WCHAR wszPW[SecondAuth::Common::PWMaxLength+1];		// Password if UnLock
};

struct TAUpdateSecondAuthLock: public TAHeader
{
	bool bIsLock;
	char cFailCount;
};

struct TQInitSecondAuth:public TQHeader
{
};

struct TQValidateSecondAuth:public TQHeader
{
	int	nAuthCheckType;
	WCHAR wszPW[SecondAuth::Common::PWMaxLength+1];
};

struct TAValidateSecondAuth:public TAHeader
{
	int	nAuthCheckType;
	BYTE cFailCount;
};

// Restraint ----------------------------------------------------------------------
struct TQRestraint:public TQHeader		//QUERY_RESTRAINT
{
};

struct TARestraint: public TAHeader
{
	BYTE cCount;
	TRestraint restraint[RESTRAINTMAX];
};

struct TQSetRestraint :  public TQHeader	//QUERY_SETRESTRAINT
{
	BYTE cTargetCode;				//DBDNWorldDef::RestraintTargetCode::eCode
	BYTE cRestraintCode;			//DBDNWorldDef::RestraintTypeCode::eCode
	INT64 biCharacterDBID;			//if target code "character"
	WCHAR wszRestraintreason[RESTRAINTREASONMAX];	//Reason String
	WCHAR wszRestraintreasonForDolis[RESTRAINTREASONFORDOLISMAX];
	USHORT nDurationDays;				//duration
	int nDolisReasonCode;				//Default is 110; penalty by abuse log is 111; contact Solution Team (Wonsang or Siyeon) for code number management
};

// QUERY_GET_KEYSETTINGOPTION
struct TQGetKeySettingOption:public TQHeader
{
};

struct TAGetKeySettingOption: public TAHeader
{
	TKeySetting	sKeySetting;
};

// QUERY_MOD_KEYSETTINGOPTION
struct TQModKeySettingOption:public TQHeader
{
	bool bUseDefault;
	TKeySetting	sKeySetting;
};

struct TAModKeySettingOption: public TAHeader
{
};

// QUERY_GET_PADSETTINGOPTION
struct TQGetPadSettingOption:public TQHeader
{
};

struct TAGetPadSettingOption: public TAHeader
{
	TPadSetting	sPadSetting;
};

// QUERY_MOD_PADSETTINGOPTION
struct TQModPadSettingOption:public TQHeader
{
	bool bUseDefault;
	TPadSetting	sPadSetting;
};

struct TAModPadSettingOption: public TAHeader
{
};

struct TQGetProfile : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetProfile : public TAHeader
{
	TProfile sProfile;
};

struct TQSetProfile : public TQHeader
{
	INT64 biCharacterDBID;
	TProfile sProfile;
};

struct TASetProfile : public TAHeader
{
	TProfile sProfile;
};

struct TQAddAbuseLog : public TQHeader
{
	BYTE cAbuseCode;
	INT64 biCharacterDBID;
	USHORT unLen;
	WCHAR wszBuf[1024];
};


struct TQAddAbuseMonitor:public TQHeader
{
	INT64 biCharacterDBID;
	int iCount;
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	int iCount2; // For chat abuse prevention
#endif
};

struct TQDelAbuseMonitor:public TQHeader
{
	INT64 biCharacterDBID;
#if defined(PRE_ADD_ANTI_CHAT_SPAM)
	char cType;  // 1-Auto abusing, 2-Chat abusing, 3-Reset both
#endif
};

struct TQGetAbuseMonitor:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetAbuseMonitor:public TAHeader
{
	int iAbuseCount;
	int iCallCount;
	int iPlayRestraintValue;
	int iDBResetRestraintValue;
};

struct TQGetWholeAbuseMonitor:public TQHeader
{
	UINT nAccountDBID;
};

struct TAbuseMonitor
{
	INT64 biCharacterDBID;
	int iAbuseCount;
	int iCallCount;
	int iPlayRestraintValue;
	int iDBResetRestraintValue;
};

struct TAGetWholeAbuseMonitor:public TAHeader
{
	BYTE cCount;
	TAbuseMonitor Abuse[CHARCOUNTMAX];
};


// QUERY_DBRESULTERROR
struct TADBResultError: public TAHeader
{
	char cMainCmd;
	char cSubCmd;
};


// QUERY_CHANGECHARACTERNAME // Change character name
struct TQChangeCharacterName : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	WCHAR wszOriginName[NAMELENMAX];	   // Existing character name
	WCHAR wszCharacterName[NAMELENMAX];  // Character name to change to
	BYTE btChangeCode;		// 1 = Admin change, 2 = Character name change
	int	nReservedPeriodDay;	// Duration for which the previous character name is reserved
	INT64 biItemSerial;		// Used item ID
	int	nNextPeriodDay;		// Cooldown period from last name change date until next allowed name change. Unit : day
};

struct TAChangeCharacterName : public TAHeader
{
	char cWorldSetID;
	INT64 nCharacterDBID;		// Character DBID
	INT64 biItemSerial;
	WCHAR wszOriginName[NAMELENMAX];		// Existing character name
	WCHAR wszCharacterName[NAMELENMAX];  // Changed character name
	__time64_t tNextChangeDate;
};

// QUERY_GETLIST_ETCPOINT
struct TQGetListEtcPoint : public TQHeader
{
	INT64 nCharacterDBID;	
};

struct TAGetListEtcPoint : public TAHeader
{
	INT64 nCharacterDBID;
	INT64 biEtcPoint[DBDNWorldDef::EtcPointCode::Max];
};

struct TQAddEtcPoint :public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cType;
	INT64 biAddPoint;
	int nMapID;
	WCHAR wszIP[IPLENMAX];
};

struct TAAddEtcPoint:public TAHeader
{
	BYTE cType;
	INT64 biLeftPoint;
};

struct TQUseEtcPoint :public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cType;
	INT64 biUsePoint;
	int nMapID;
	WCHAR wszIP[IPLENMAX];
};

struct TAUseEtcPoint:public TAHeader
{
	BYTE cType;
	INT64 biLeftPoint;
};

struct TQSaveConnectDurationTime: public TQHeader
{
	INT64 biCharacterDBID;
	char cCount;
	TTimeEvent Event[TIMEEVENTMAX];
};

struct TQModTimeEventDate:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TQChangeJobCode : public TQHeader
{
	INT64 biCharacterDBID;
	int nFirstJobIDBefore;
	int nFirstJobIDAfter;
	int nSecondJobIDBefore;
	int nSecondJobIDAfter;
	int nChannelID;
	int nMapID;
	char cSkillPage;
};

struct TAChangeJobCode : public TAHeader
{
	USHORT wSkillPoint;	// Skill point increase amount after skill reset.
	USHORT wTotalSkillPoint;	// Total skill points owned after skill reset.
};

struct TQGetListVariableReset:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListVariableReset:public TAHeader
{
	BYTE cCount;
	CommonVariable::Data Data[CommonVariable::Type::Max];
};

struct TQModVariableReset:public TQHeader
{
	INT64 biCharacterDBID;
	CommonVariable::Type::eCode Type;
	INT64 biValue;
	__time64_t tResetTime;
};

#if defined(PRE_ADD_GAMEQUIT_REWARD)
struct TQModNewbieRewardFlag:public TQHeader
{
	bool bRewardFlag;
};
#endif	// #if defined(PRE_ADD_GAMEQUIT_REWARD)

#if defined(PRE_ADD_CP_RANK)
struct TQAddStageClearBest : public TQHeader
{
	int nMapID;
	INT64 biCharacterDBID;
	DBDNWorldDef::ClearGradeCode::eCode Code;
	int nClearPoint;
	int nLimitLevel;
};

struct TQGetStageClearBest : public TQHeader
{
	int nMapID;
};

struct TAGetStageClearBest : public TAHeader
{
	TStageClearBest sLegendClearBest;
	TStageClearBest sMonthlyClearBest;
};

struct TQGetStageClearPersonalBest : public TQHeader
{
	int nMapID;
	INT64 biCharacterDBID;
};

struct TAGetStageClearPersonalBest : public TAHeader
{
	DBDNWorldDef::ClearGradeCode::eCode Code;
	int nClearPoint;
};

struct TQInitStageCP : public TQHeader
{
	char cInitType;
};
#endif //#if defined(PRE_ADD_CP_RANK)

// QUERY_MODCHARACTERSLOTCOUNT
struct TQModCharacterSlotCount: public TQHeader
{
	INT64 biCharacterDBID;
};


//#########################################################################################################################################
//	MAINCMD_STATUS
//#########################################################################################################################################

// QUERY_SELECTCHARACTER
struct TQSelectCharacter:public TQHeader
{
	INT64 biCharacterDBID;
#if !defined( PRE_PARTY_DB )
	TPARTYID PartyID;
#endif
	UINT nLastServerType;
	WCHAR wszAccountName[IDLENMAX];
};

const int nItemListMaxCount = EQUIPMAX + INVENTORYMAX + WAREHOUSEMAX + QUESTINVENTORYMAX + CASHEQUIPMAX + GLYPHMAX + TALISMAN_MAX + PERIODINVENTORYMAX + PERIODWAREHOUSEMAX;

struct TASelectCharacter:public TAHeader
{
	INT64 biCharacterDBID;
	TRestraintData RestraintData;	// Sanction-related data
	TKeySetting KeySetting;		// Related to key settings
	TPadSetting	PadSetting;
#if !defined( PRE_PARTY_DB )
	TPARTYID PartyID;
#endif
	TGuildSelfView GuildSelfView;	// Guild visual info (personal)
	bool bIsSetSecondAuthPW;	// Whether 2nd auth password is set
	bool bIsSetSecondAuthLock;	// 2nd auth account Lock status

#if defined(PRE_ADD_VIP)
	int nVIPTotalPoint;			// Total VIP points
	__time64_t tVIPEndDate;		// VIP period end date
	bool bAutoPay;				// 0=Manual billing, 1=Auto billing
#endif	// #if defined(PRE_ADD_VIP)

	TVehicleCompact VehicleEquip;
 	TVehicleCompact PetEquip;
	TUserData UserData;
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	BYTE cGuildJoinLevel;				//Guild join level (first time only)
	bool bWasRewardedGuildMaxLevel;		//Flag for whether max-level reward was received in guild
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT
#if defined( PRE_ADD_CHARACTERCHECKSUM )
	__time64_t tLastLoginDate;
	__time64_t tLastLogoutDate;
	UINT uiCheckSum;
#endif // #if defined( PRE_ADD_CHARACTERCHECKSUM )
	int nItemListCount;
	DBPacket::TMaterialItemInfo ItemList[nItemListMaxCount];	// Must be at the very bottom
};

// QUERY_UPDATEUSERDATA, QUERY_CHANGESERVERUSERDATA, QUERY_LASTUPDATEUSERDATA
struct TQUpdateCharacter:public TQHeader
{
	UINT uiDBRandomSeed;

	INT64 biCharacterDBID;
	int nMapIndex;							// Current map ID
	int nLastVillageMapIndex;				// Last village map number
	int nLastSubVillageMapIndex;			// Last sub-village map number
	char cLastVillageGateNo;				// Last village gate number
	int nPosX;								// Current X coordinate
	int nPosY;								// Current Y coordinate
	int nPosZ;								// Current Z coordinate
	float fRotate;							// Rotate
	int nGlyphDelayTime;
	int nGlyphRemainTime;
	bool bExecuteScheduleedTask;
	char cViewCashEquipBitmap[VIEWCASHEQUIPMAX_BITSIZE];	// Handle normal/cash equipment as a bitmap
	TQuickSlot QuickSlot[QUICKSLOTMAX];		// Quick slot window (2 rows of 10)
	DBPacket::TSkillCoolTime SkillCoolTime[SKILLMAX];	// Skill
	DBPacket::TSkillCoolTime SkillCoolTime2[SKILLMAX];	// Skill 2
	DBPacket::TItemCoolTime Equip[EQUIPMAX];			// Equipment window
#if defined(PRE_PERIOD_INVENTORY)
	DBPacket::TItemCoolTime Inventory[INVENTORYTOTALMAX];	// Inventory window
	DBPacket::TItemCoolTime Warehouse[WAREHOUSETOTALMAX];	// Storage window
#else	// #if defined(PRE_PERIOD_INVENTORY)
	DBPacket::TItemCoolTime Inventory[INVENTORYMAX];	// Inventory window
	DBPacket::TItemCoolTime Warehouse[WAREHOUSEMAX];	// Storage window
#endif	// #if defined(PRE_PERIOD_INVENTORY)
	DBPacket::TItemCoolTime CashEquip[CASHEQUIPMAX];	// Equipment window (cash)
	DBPacket::TItemCoolTime CashInventory[CASHINVENTORYDBMAX];	// Inventory window (Cash)
#if defined( PRE_ITEMBUFF_COOLTIME )
	DBPacket::TItemCoolTime ServerWare[WAREHOUSEMAX];	// Server storage
	DBPacket::TItemCoolTime ServerWareCash[WAREHOUSEMAX];	// Server storage (Cash)
#endif
};

struct TAUpdateCharacter:public TAHeader
{
};

struct TQMWTest:public TQHeader
{
	int iCount;
};

// QUERY_CHANGESTAGEUSERDATA
struct TQChangeStageUserData:public TQHeader
{
	INT64 biCharacterDBID;
	int nChannelID;
	int nMapID;
	TPARTYID PartyID;
	int nExp;
	INT64 biCoin;
	BYTE cRebirthCoin;			// Used normal resurrection coins
	BYTE cPCBangRebirthCoin;	// Used PC bang resurrection coins
	short nCount;
	short nDeathCount;			// Death count
	BYTE cDifficult;			// Difficulty
	INT64 biPetItemSerial;
	int nPetExp;
	INT64 biPickUpCoin;
	DBPacket::TDurabilityInfo ItemDurArr[INVENTORYMAX + EQUIPMAX];
};

struct TAChangeStageUserData:public TAHeader
{
	BYTE cRebirthCoin;			// Used normal resurrection coins
	BYTE cPCBangRebirthCoin;	// Used PC bang resurrection coins
};

// QUERY_GETCHARACTERPARTIALYBYNAME,
struct TQGetCharacterPartialyByName:public TQHeader
{
	WCHAR wszName[NAMELENMAX];
	BYTE cReqType;			//eReqCharacterPartialy
};

// QUERY_GETCHARACTERPARTIALYBYDBID
struct TQGetCharacterPartialyByDBID:public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cReqType;			//eReqCharacterPartialy
};

struct TAGetCharacterPartialy:public TAHeader
{
	INT64 biCharacterDBID;
	BYTE cClass;
	BYTE cLevel;
	BYTE cJob;
	BYTE cReqType;			//eReqCharacterPartialy
};

// QUERY_CHECKFIRSTVILLAGE
struct TQCheckFirstVillage:public TQHeader
{
	INT64 biCharacterDBID;
};

// QUERY_LEVEL, // Level
struct TQLevel:public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cLevel;
	char cChangeCode;	// CharacterLevelChangeCode 
	int nChannelID;
	int nMapID;
	WCHAR wszCharName[NAMELENMAX];
};

// QUERY_EXP,						// exp
struct TQExp:public TQHeader
{
	INT64 biCharacterDBID;
	int nExp;
	char cChangeCode;	// eCharacterExpChangeCode 
	int nChannelID;
	int nMapID;
	INT64 biFKey;			// Reference key per CharacterExpChangeCode. 1:QuestID, 2:PartyID or NULL, 3:NULL, 4:AuditLogID 5:ItemSerial
};

// QUERY_COIN
struct TQCoin:public TQHeader
{
	INT64 biCharacterDBID;
	char cCoinChangeCode;	// CoinChangeCode
	INT64 biChangeKey;			// Quest ID
	INT64 nChangeCoin;		// Coin delta only (+/-)
	int nChannelID;
	int nMapID;
	INT64 nTotalCoin;		// To compare with return value after saving to DB
};

// QUERY_WAREHOUSECOIN
struct TQWarehouseCoin:public TQHeader
{
	INT64 biCharacterDBID;
	char cCoinChangeCode;	// CoinChangeCode
	INT64 nChangeCoin;		// Coin delta only (+/-)
	int nChannelID;
	int nMapID;
	INT64 nTotalCoin;			// To compare with return value after saving to DB
	INT64 nTotalWarehouseCoin;	// To compare with return value after saving to DB
};

// QUERY_MAPINFO
struct TQMapInfo:public TQHeader
{
	INT64 biCharacterDBID;
	char cLastVillageGateNo;				// Last village gate number
	int nLastVillageMapIndex;				// Last village map number
	int nLastSubVillageMapIndex;			// Last sub-village map number
	int nMapIndex;							// Current map ID
};

// QUERY_ADDCASHREBIRTHCOIN
struct TQAddCashRebirthCoin: public TQHeader
{
	INT64 biCharacterDBID;
	int nRebirthCount;
	int nRebirthPrice;
	int nRebirthCode;
	int nRebirthKey;
};

// QUERY_CASHREBIRTHCOIN
struct TQCashRebirthCoin:public TQHeader
{
	INT64 biCharacterDBID;
	int nChannelID;
	int nMapID;
	WCHAR wszIP[IPLENMAX];
};

// QUERY_FATIGUE
struct TQFatigue:public TQHeader
{
	INT64 biCharacterDBID;
	char cFatigueTypeCode;		// FatigueTypeCode
	int nChangeFatigue;			// Delta value
	int nChannelID;
	int nMapID;
};

// QUERY_EVENTFATIGUE
struct TQEventFatigue:public TQHeader
{
	INT64 biCharacterDBID;
	int nChangeFatigue;		// Delta value
	bool bReset;			// Whether to reset event fatigue. 0=no reset, 1=reset
	char cInitTime;			// Hour when event fatigue resets. 0 ~ 23
	int nChannelID;
	int nMapID;
};

// QUERY_JOB
struct TQJob:public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cJob;
};

// QUERY_GLYPHDELAYTIME
// QUERY_GLYPHREMAINTIME
struct TQGlyphAttributeTime: public TQHeader
{
	INT64 biCharacterDBID;
	int nTime;
};

// QUERY_NOTIFIER
struct TQNotifier:public TQHeader
{
	INT64 biCharacterDBID;
	char cNotifierSlot;	// Notifier_
	char cNotifierType;	// DNNotifier::Type value
	int nID;
};

// QUERY_DUNGEONCLEAR
struct TQDungeonClear:public TQHeader
{
	INT64 biCharacterDBID;
	char cType;
	int nMapIndex;
};

// QUERY_ETERNITYITEM
struct TQEternityItem:public TQHeader
{
	INT64 biCharacterDBID;
	char cEternityCode;		// eEternityItem
	int nEternityValue;
};

// QUERY_COLOR
struct TQColor: public TQHeader
{
	INT64 biCharacterDBID;
	char cItemType;
	DWORD dwColor;
};

#ifdef PRE_FIX_63822
struct TAColor : public TAHeader
{
};
#endif		//#ifdef PRE_FIX_63822

// QUERY_REBIRTHCOIN
struct TQRebirthCoin: public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cRebirthCoin;
	BYTE cPCBangRebirthCoin;
};

struct TAEffectItem:public TAHeader
{
	int	iOffset;
	TEffectItemInfo ItemInfo;
};

// QUERY_NESTCLEARCOUNT, // UPSERT NEST clear count 
struct TQNestClearCount: public TQHeader
{
	INT64 biCharacterDBID;
	int nMapIndex;
	char cClearType;		// eNestClearType
};

// QUERY_INITNESTCLEARCOUNT, // Reset NEST weekly clear count 
struct TQInitNestClearCount: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAOwnChracterLevel : public TAHeader		//MAINCMD_STATUS / QUERY_OWNCHRACTERLEVEL
{
	BYTE cCount;
	TChracterLevel Level[CHARCOUNTMAX];
};

struct TQChangeSkillPage : public TQHeader
{
	INT64 biCharacterDBID;
	char cSkillPage;
};

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
struct TQGetTotalLevelSkill : public TQHeader
{	
	INT64 biCharacterDBID;
};
struct TAGetTotalLevelSkill : public TAHeader
{	
	int nTotalSkillLevel;
	TotalLevelSkill::TTotalLevelSkillData TotalSkill[TotalLevelSkill::Common::MAXSLOTCOUNT];
};
struct TQAddTotalLevelSkill : public TQHeader
{	
	INT64 biCharacterDBID;
	int nSlotIndex;
	int nSkillID;
};
#endif

#ifdef PRE_ADD_PRESET_SKILLTREE
struct TQGetSKillPreSetList : public TQHeader
{	
	INT64 biCharacterDBID;
};

struct TAGetSKillPreSetList : public TAHeader
{
	TSkillSetIndexData SKillIndex[SKILLPRESETMAX];
	TSkillSetPartialData SKills[SKILLMAX * SKILLPRESETMAX];
};

struct TQAddSkillPreSet : public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cIndex;
	WCHAR wszName[SKILLPRESETNAMEMAX];
	BYTE cCount;
	TSkillSetPartialData SKill[SKILLMAX];
};

struct TAAddSkillPreSet : public TAHeader
{
};

struct TQDelSkillPreSet : public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cIndex;
};

struct TADelSkillPreSet : public TAHeader
{
};
#endif		//#ifdef PRE_ADD_PRESET_SKILLTREE

//#########################################################################################################################################
//	MAINCMD_MISSION
//#########################################################################################################################################

// QUERY_MISSIONGAIN
struct TQMissionGain:public TQHeader
{
	INT64 biCharacterDBID;
	char MissionGain[MISSIONMAX_BITSIZE];		// Acquisition status	
};

// QUERY_MISSIONACHIEVE
struct TQMissionAchieve:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionID;
	char cMissionType;		// eMissionType
	char MissionAchieve[MISSIONMAX_BITSIZE];	// Goal achieved (on/off)
};

// QUERY_SETDAILYMISSION, // Save daily mission info (initial bulk insert)
struct TQSetDailyMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[DAILYMISSIONMAX];
};

// QUERY_SETWEEKLYMISSION, // Save weekly mission info (initial bulk insert)
struct TQSetWeeklyMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[WEEKLYMISSIONMAX];
};

#if defined(PRE_ADD_MONTHLY_MISSION)
// QUERY_SETMONTHLYMISSION, // Save monthly event mission info (initial bulk insert)
struct TQSetMonthlyMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[MONTHLYMISSIONMAX];
};
#endif	// #if defined(PRE_ADD_MONTHLY_MISSION)

// QUERY_SETWEEKENDEVENTMISSION, // Save weekend event mission info (initial bulk insert)
struct TQSetWeekendEventMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[WEEKENDEVENTMISSIONMAX];
};

// QUERY_SETGUILDWARMISSION, // Save guild war festival mission info
struct TQSetGuildWarMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[GUILDWARMISSIONMAX];
};

// QUERY_SETGUILDCOMMONMISSION, // Save guild common mission info
struct TQSetGuildCommonMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[GUILDCOMMONMISSIONMAX];
};

struct TQSetWeekendRepeatMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[WEEKENDREPEATMISSIONMAX];
};

// QUERY_SETPCBANGMISSION, // Save daily mission info (initial bulk insert)
struct TQSetPCBangMission:public TQHeader
{
	INT64 biCharacterDBID;
	int nMissionIDs[PCBangMissionMax];		// 0: silver, 1: gold
};

// QUERY_MODDAILYWEEKLYMISSION, // Save daily/weekly mission info
struct TQModDailyWeeklyMission:public TQHeader
{
	INT64 biCharacterDBID;
	char cDailyMissionType;	// eDailyMissionType
	char cMissionIndex;
	TDailyMission Mission;
	int nMissionScore;
};

struct TQClearMission : public TQHeader
{
	INT64 biCharacterDBID;
};

//#########################################################################################################################################
//	MAINCMD_APPELLATION
//#########################################################################################################################################

// QUERY_ADDAPPELLATION
struct TQAddAppellation:public TQHeader
{
	INT64 biCharacterDBID;
	int nAppellationID;
	char Appellation[APPELLATIONMAX_BITSIZE];	// Title
};

// QUERY_SELECTAPPELLATION
struct TQSelectAppellation:public TQHeader
{
	INT64 biCharacterDBID;
	int nSelectAppellation;
	char cAppellationKind;		// DBDNWorldDef::AppellationKind
};
// QUERY_DELAPPELLATION
struct TQDelAppellation:public TQHeader
{
	INT64 biCharacterDBID;	
	int	  nDelAppellation;	// Title to delete (individual delete)
};


//#########################################################################################################################################
//	MAINCMD_SKILL
//#########################################################################################################################################

// QUERY_ADDSKILL, // Add skill
struct TQAddSkill:public TQHeader
{
	INT64 biCharacterDBID;
	int nSkillID;			// Skill ID
	char cSkillLevel;
	int nCoolTime;			// Cooldown
	char cSkillChangeCode;	// SkillChangeCode
	INT64 biCoin;			// Purchase cost if the skill was bought. Enter 0 if not purchased.
	int nChannelID;
	int nMapID;
	char cSkillPage;		// Skill page (skill code)
	INT64 biCurrentCoin;
	INT64 biPickUpCoin;
};

// QUERY_MODSKILLLEVEL, // Change skill level
struct TQModSkillLevel:public TQHeader
{
	INT64 biCharacterDBID;
	int nSkillID;			// Skill ID
	BYTE cSkillLevel;		// Skill level
	int nCoolTime;			// Cooldown
	USHORT wUsedSkillPoint;	// Skill points spent to level up the skill
	char cSkillChangeCode;	// SkillChangeCode (2=Skill level up & SP used, 13=Skill level adjusted (by admin))
	int nChannelID;
	int nMapID;
	char cSkillPage;
};

// QUERY_INCREASESKILLPOINT, // Increase skill points
struct TQIncreaseSkillPoint:public TQHeader
{
	INT64 biCharacterDBID;
	USHORT wSkillPoint;
	int nChannelID;
	int nMapID;
	char cSkillPointChangeCode;
	char cSkillPage;
};

// QUERY_DELSKILL
struct TQDelSkill:public TQHeader
{
	INT64 biCharacterDBID;
	int nSkillID;		// Skill ID
	char cSkillChangeCode;	// SkillChangeCode (9=Skill deleted (drop), 10=Skill deleted (quest), 11=Skill deleted (event expired), 12=Skill deleted (by admin))
	bool bSkillPointBack;	// Whether to refund skill points after deleting a skill. 0=no refund, 1=refund skill points
	int nChannelID;
	int nMapID;
	char cSkillPage;		// Skill page (skill code)
};

// QUERY_RESETSKILL
struct TQResetSkill:public TQHeader
{
	INT64 biCharacterDBID;
	int nChannelID;
	int nMapID;
	char cSkillPage;		// Skill page (skill code)
};

struct TAResetSkill: public TAHeader
{
	USHORT wSkillPoint;	// Total skill points owned after skill reset
	USHORT wTotalSkillPoint;	// Total skill points owned after skill reset
	char cSkillPage;		// Skill page (skill code)
};

// QUERY_RESETSKILLBYSKILLIDS
struct TQResetSkillBySkillIDS:public TQHeader
{
	INT64	biCharacterDBID;
	int		iSkillIDs[SKILLMAX];
	int		iChannelID;
	int		iMapID;
	char	cSkillPage;
};

struct TAResetSkillBySkillIDS:public TAHeader
{
	short	nSkillPoint;
	short	nTotalSkillPoint;
	char	cSkillPage;
};

struct TQDecreaseSkillPoint:public TQHeader
{
	INT64 biCharacterDBID;
	short nDecreaseSkillPoint;
	int iChannelID;
	int iMapID;
	char cSkillPage;
};

struct TQUseExpandSkillPage:public TQHeader
{
	INT64 biCharacterDBID;
	int		nSkillArray[DEFAULTSKILLMAX];	
	short	wSkillPoint;
};
struct TAUseExpandSkillPage:public TAHeader
{

};
struct TQSetSkillPoint:public TQHeader
{
	INT64 biCharacterDBID;
	short nSkillPoint;	
	char cSkillPage;
};


//#########################################################################################################################################
//	MAINCMD_ITEM
//#########################################################################################################################################

struct TQAddMaterializedItem:public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cItemLocationIndex;	// Index value where the item will be located
	DBDNWorldDef::AddMaterializedItem::eCode Code;
	INT64 biFKey;
	TItem AddItem;
	bool bStackableFlag;
	INT64 biItemPrice;	// Item price value. Enter 0 for unpurchased items. For gifted or purchased package items, enter 1/n of the package price for each component item
	INT64 biSenderCharacterDBID;	// Sender character ID if gift
	BYTE cItemLocationCode;	// 2=Inventory, 4=Quest Inventory, 5=Cash Inventory, 6=Gift Box, 7=Cash Equipment
	BYTE cPayMethodCode;	// NULL=not a purchase, 1=coin, 2=cash, 3=reward points, 4=DNA points
	int	iChannelID;
	int	iMapIndex;
	WCHAR wszIP[IPLENMAX];
	bool bMerge;			// 0=Item not merged, 1=Item merged
	INT64 biMergeTargetItemSerial;	// Target item serial when the created item is merged at the destination
	DWORD dwPetPartsColor1;
	DWORD dwPetPartsColor2;
	int	nPetSkillID1;
	int	nPetSkillID2;
	char cPetCount;		// Must send pet and mount list
	DBPacket::TAddMaterializedPet PetList[Pet::Slot::Max];
};

struct TQUseItem:public TQHeader
{
	DBDNWorldDef::UseItem::eUseItemCode	Code;
	INT64 biItemSerial;
	short nUsedItemCount;
	int	iChannelID;
	int	iMapID;
	WCHAR wszIP[IPLENMAX];
	bool bAllFlag;
	int	iTargetLogItemID;
	char cTargetLogItemLevel;
};

struct TQResellItem:public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biItemSerial;
	short nItemCount;
	INT64 biResellPrice;
	int	iNpcID;
	int	iChannelID;
	int	iMapID;
	WCHAR wszIP[IPLENMAX];
	bool bAllFlag;
	bool bRebuyFlag;
#ifdef PRE_MOD_SELL_SEALEDITEM
	bool bBelongFlag;
#endif		//#ifdef PRE_MOD_SELL_SEALEDITEM
};

struct TQEnchantItem:public TQHeader
{
	INT64 biItemSerial;
	INT64 biCoin;						// Enhancement cost
	bool bEnchantSuccessFlag;		// 0=Failure, 1=Success
	bool bItemDestroyFlag;			// Whether the original item was destroyed on enhancement failure. 0=not destroyed, 1=destroyed
	BYTE cCurrentItemLevel;			// Level after enhancement
	char cCurrentItemPotential;		// Item potential after enhancement
	int	iChannelID;
	int	iMapID;
	int	iNpcID;

	//Optional
	TItem NewItem;			// Item granted instead upon enhancement failure 

	BYTE cMaterialCount;
	INT64 biCharacterDBID;			// Enhancement owner
	int	nSupportItemID;				// ItemID of used enhancement support item (e.g., Item Protection Jelly)
#if defined(PRE_ADD_EXCHANGE_ENCHANT)
	INT64 biExtractItemSerial;			// Original item extracted during enhancement transfer (DB automatically resets its enhancement level to 0)
#endif
	DBPacket::TItemCountInfo MaterialArray[INVENTORYMAX];
};

struct TQChangeItemLocation:public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biItemSerial;		// Unique serial of item to move
	INT64 biNewItemSerial;	// (optional) Newly created item unique ID when moving a partial quantity of an item.
	int	iItemID;			// Item ID to move
	DBDNWorldDef::ItemLocation::eCode Code;	// ItemLocationCode after move: 1=Equipment, 2=Inventory, 3=Warehouse, 4=Quest Inventory, 5=Cash Inventory, 6=Gift Box, 7=Cash Equipment
	BYTE cItemLocationIndex;	// Item slot index after move
	short nItemCount;			// Count of items to move
	int	iChannelID;
	int	iMapID;
	bool bVehicleParts;		// Whether moved item is "mount or pet equip item". 0=false, 1=true -> set true only for minor ones(?)
	INT64 biVehicleSerial;	// When equipping on a mount or pet: unique item ID of the mount or pet
	char cVehicleSlotIndex;	// When equipping on a mount or pet: equip slot
	bool bMerge;				// 0=Item not merged, 1=Item merged
	INT64 biMergeTargetItemSerial;	// Target item serial when the created item is merged at the destination
	bool bMergeAll;			// When item is merged. 0=merged quantity is not full original quantity, 1=merged quantity is full original quantity
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	int nGoldFee;			// Server storage gold fee
#endif
};

struct TQSwitchItemLocation: public TQHeader
{
	INT64 biCharacterDBID;
	DBDNWorldDef::ItemLocation::eCode SrcCode;	// ItemLocationCode after move: 1=Equipment, 2=Inventory, 3=Warehouse, 4=Quest Inventory, 5=Cash Inventory, 6=Gift Box, 7=Cash Equipment
	BYTE cSrcSlotIndex;
	INT64 biDestItemSerial;
	DBDNWorldDef::ItemLocation::eCode DestCode;	// ItemLocationCode after move: 1=Equipment, 2=Inventory, 3=Warehouse, 4=Quest Inventory, 5=Cash Inventory, 6=Gift Box, 7=Cash Equipment
	BYTE cDestSlotIndex;
	INT64 biSrcItemSerial;
};

struct TQModItemDurability:public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biTax;				// Repair cost when repairing an item.
	int	iChannelID;
	int	iMapID;
	short nCount;
	INT64 biCurrentCoin;
	INT64 biPickUpCoin;
	DBPacket::TDurabilityInfo ItemDurArr[INVENTORYMAX + EQUIPMAX];
};

struct TQModMaterializedItem:public TQHeader
{
	INT64 biItemSerial;
	int	iChannelID;
	int	iMapID;
	int	iModFieldBitmap;
	USHORT nItemDurability;
	BYTE cItemLevel;
	char cItemPotential;
	bool bSoudBoundFlag;
	BYTE cSealCount;
	BYTE cItemOption;
	WCHAR wszIP[IPLENMAX];
    int nEnchantJewelID;	// Enhancement code item ID
#if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
	char cItemPotentialMoveCount;	//Potential transfer count
#endif	// #if defined(PRE_MOD_POTENTIAL_JEWEL_RENEWAL)
};

struct TQGetListMaterializedItem:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListMaterializedItem:public TAHeader
{
	short nItemCount;
	DBPacket::TMaterialItemInfo	MaterialItemInfoArr[256];	// Currently unknown max count, so temporarily set to 256.
};

struct TQExchangeProperty: public TQHeader
{
	INT64 biCharacterDBID;	// Character ID
	DBPacket::TExchangeItem ExchangeItem[EXCHANGEMAX];
	INT64 biExchangeCoin;	// Coin to exchange
	int nExchangeTax;	// Exchange fee
	INT64 biTargetCharacterDBID;
	DBPacket::TExchangeItem TargetExchangeItem[EXCHANGEMAX];
	INT64 biTargetExchangeCoin;
	int nTargetExchangeTax;	// Exchange fee
	int nChannelID;
	int nMapID;
	UINT uiTargetAccountDBID;
};

// QUERY_GETPAGEMATERIALIZEDITEM
struct TQGetPageMaterializedItem: public TQHeader
{
	INT64 biCharacterDBID;	// Character ID
	int nPageNumber;		// Page number.
	int nPageSize;			// Page size.
};

struct TAGetPageMaterializedItem: public TAHeader
{
	short wPageNumber;		// Page number.
	int nTotalListCount;	// Has a value only when page number is 1 (total count)
	char cCount;
	TItem CashItem[CASHINVENTORYMAX];
};

// QUERY_UPSEQUIP
struct TQUpsEquip: public TQHeader
{
	INT64 biCharacterDBID;	// Character ID
	char cItemLocation;
	char cEquipSlotIndex;
	INT64 biSerial;
};

// QUERY_PUTONEPIECEON, // Equip one-piece
struct TQPutOnepieceOn: public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biOnepieceItemSerial;
	char cOnepieceCashEquipIndex;
	char cCashEquipTotalCount;
	BYTE cSubCashEquipIndexArray[CASHEQUIPMAX];
};

// QUERY_TAKECASHEQUIPMENTOFF, // Unequip all at once (cashequip -> cashinven)
struct TQTakeCashEquipmentOff: public TQHeader
{
	INT64 biCharacterDBID;
	char cCashEquipTotalCount;
	BYTE cCashEquipIndexArray[CASHEQUIPMAX];
};

// QUERY_GETPAGEVEHICLE
struct TQGetPageVehicle: public TQHeader
{
	INT64 biCharacterDBID;	// Character ID
	int nPageNumber;		// Page number.
	int nPageSize;			// Page size.
};

struct TAGetPageVehicle: public TAHeader
{
	short wPageNumber;		// Page number.
	int nTotalListCount;	// Has a value only when page number is 1 (total count)
	char cCount;
	TVehicleCompact VehicleItem[VEHICLEINVENTORYPAGEMAX];
};

// QUERY_DELPETEQUIPMENT
struct TQDelPetEquipment: public TQHeader
{
	INT64 biBodySerial;
	INT64 biPartsSerial;
	WCHAR wszIp[IPLENMAX];
};

struct TADelPetEquipment: public TAHeader
{
};

// QUERY_MODPETSKINCOLOR
struct TQModPetSkinColor: public TQHeader
{
	INT64 biBodySerial;
	DWORD dwSkinColor;
	DWORD dwSkinColor2;
};

struct TAModPetSkinColor: public TAHeader
{
};

// QUERY_MISSINGITEMLIST,
struct TAMissingItemList: public TAHeader
{
	char cCount;
	TItem MissingList[MISSINGITEMMAX];
};

// QUERY_RECOVERMISSINGITEM,
struct TQRecoverMissingItem: public TQHeader
{
	INT64 biCharacterDBID;
	//INT64 biItemSerial;
	char cItemLocationCode;
	BYTE cSlotIndex;
	TItem RecoverItem;
};

struct TARecoverMissingItem: public TAHeader
{
	char cItemLocationCode;
	BYTE cSlotIndex;
	TItem RecoverItem;
};

struct TQGetListRepurchaseItem:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListRepurchaseItem:public TAHeader
{
	BYTE cCount;
	TRepurchaseItemInfo ItemList[Shop::Repurchase::MaxList];
};

struct TQDelExpiritem:public TQHeader
{
	INT64 biCharacterDBID;	
};
struct TADelExpiritem:public TAHeader
{	
	BYTE cCount;
	INT64 biItemSerial[MAXDELEXPIREITEM];		
};

struct TQDelCashItem: public TQHeader	// QUERY_DELCASHITEM,
{
	INT64 biItemSerial;
	short wCount;
	int nWatingTime;	// Removal wait period. Unit: minutes
};

struct TADelCashItem: public TAHeader
{
	INT64 biItemSerial;
	__time64_t tExpireDate;
};

struct TQRecoverCashItem: public TQHeader	// QUERY_RECOVERCASHITEM,
{
	INT64 biItemSerial;
	short wCount;
};

struct TARecoverCashItem: public TAHeader
{
	INT64 biItemSerial;
	bool bEternity;
	__time64_t tExpireDate;
};

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
struct TQCheckNamedItemCount: public TQHeader	// QUERY_CHECK_NAMEDITEMCOUNT
{
	int nItemID;	
	BYTE cIndex;
};
struct TACheckNamedItemCount: public TAHeader
{
	int nItemID;
	int nCount;
	BYTE cIndex;
};
#endif

#if defined (PRE_ADD_EXCHANGE_POTENTIAL)
struct TQMovePotential : public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biExtractSerial;
	char cExtractPotential;
	INT64 biInjectSerial;
	char cInjectPotential;
	char cMoveableLimitCount;
	int nMapID;
};

struct TAMovePotential : public TAHeader
{
};
#endif		//#if defined (PRE_ADD_EXCHANGE_POTENTIAL)

// QUERY_MODDEGREEOFHUNGER
struct TQModDegreeOfHunger : public TQHeader
{
	INT64 biBodySerial;
	DWORD nDegreeOfHunger;
};

struct TQItemExpire : public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biSerial;
};

struct TAItemExpire : public TAHeader
{	
	INT64 biSerial;
};

struct TQChangePetName : public TQHeader
{
	INT64 itemSerial;
	INT64 petSerial;
	WCHAR name[NAMELENMAX];
};

struct TAChangePetName : public TAHeader
{
	INT64 itemSerial;
	INT64 petSerial;
	WCHAR name[NAMELENMAX];
};

struct TQClearPetVehicleInven : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAClearPetVehicleInven : public TAHeader
{

};

struct TQModItemExpireDate : public TQHeader
{
	int	iMapID;
	WCHAR wszIP[IPLENMAX];
	INT64 biItemSerial;
	INT64 biExpireDateItemSerial;
	INT64 biCharacterDBID;
	int	nMin;
};

struct TAModItemExpireDate : public TAHeader
{
	INT64 biCharacterDBID;
	INT64 biItemSerial;
	int nMin;
};

struct TQModPetExp : public TQHeader
{
	INT64 biPetSerial;
	int nExp;
};

struct TAModPetExp : public TAHeader
{

};

struct TQModPetSkill : public TQHeader
{
	INT64 biItemSerial;
	char cSkillSlot;
	int nSkillID;
};

struct TQModPetSkillExpand : public TQHeader
{
	INT64 biItemSerial;
};

struct TQModAdditiveItem:public TQHeader
{
	INT64	biCharacterDBID;
	int		nAdditiveItemID;

	INT64	biItemSerial;		// Stat item
	INT64	biAdditiveSerial;	// Design item
	INT64	biMixItemSerial;	// Synthesizer

	// Info for item removal
	int		nMapID;
	WCHAR	wszIP[IPLENMAX];	
};

struct TAModAdditiveItem:public TAHeader
{
	INT64	biItemSerial;
	int		nAdditiveItemID;
	int		nInvenIndex;
	INT64	biAdditiveSerial;
};

#if defined (PRE_ADD_COSRANDMIX)
struct TQModRandomItem:public TQHeader
{
	INT64 biCharacterDBID;

	INT64 biStuffSerialA;
	INT64 biStuffSerialB;
	INT64 biMixItemSerial;	// Synthesizer
	short wMixItemCount;

	INT64 biFee;
	DBDNWorldDef::AddMaterializedItem::eCode Code;
	TItem RandomItem;
	BYTE cItemLocationCode;

	int		nChannelID;
	int		nMapIndex;
	WCHAR	wszIP[IPLENMAX];	
};

struct TAModRandomItem:public TAHeader
{
	INT64 biStuffSerialA;
	INT64 biStuffSerialB;
	INT64 biMixItemSerial;	// Synthesizer
	INT64 biFee;
	TItem RandomItem;
};
#endif

// QUERY_SAVEITEMLOCATIONINDEX,	// P_SaveItemLocationIndex
struct TQSaveItemLocationIndex: public TQHeader
{
	INT64 biCharacterDBID;
	int nTotalInvenWareCount;	// Insert in order regardless of inven/warehouse. 
	DBPacket::TItemIndexSerial SaveList[INVENTORYMAX + WAREHOUSEMAX];
};

#if defined( PRE_ADD_LIMITED_SHOP )
struct TQGetLimitedShopItem: public TQHeader
{
	INT64 biCharacterDBID;	
};
struct TAGetLimitedShopItem:public TAHeader
{
	int nCount;
	LimitedShop::LimitedItemData ItemData[INVENTORYMAX + WAREHOUSEMAX];
};
struct TQAddLimitedShopItem: public TQHeader
{
	INT64 biCharacterDBID;
	int nBuyCount;
	LimitedShop::LimitedItemData ItemData;
};
struct TQResetLimitedShopItem: public TQHeader
{
	INT64 biCharacterDBID;
	int nResetCycle;
};
#endif

struct TQDelEffectItem: public TQHeader
{
	INT64 biCharacterDBID;
	INT64 nItemSerial;	
};



#if defined(PRE_ADD_EQUIPLOCK)
struct TQLockItemInfo: public TQHeader	//Lock, unlock
{
	INT64 biCharacterDBID;
	DBDNWorldDef::ItemLocation::eCode Code;
	BYTE cItemSlotIndex;
	int nItemID;
	INT64 biItemSerial;
};

struct TALockItemInfo: public TAHeader //Used when requesting a lock
{
	DBDNWorldDef::ItemLocation::eCode Code;
	BYTE cItemSlotIndex;
	__time64_t tLockDate;
};

struct TQUnLockRequsetItemInfo: public TQHeader	//Used when requesting an unlock
{
	INT64 biCharacterDBID;
	DBDNWorldDef::ItemLocation::eCode Code;
	BYTE cItemSlotIndex;
	int nItemID;
	int nUnLockWaitTime;
	INT64 biItemSerial;
};

struct TAUnLockRequestItemInfo: public TAHeader //Used when requesting an unlock
{
	DBDNWorldDef::ItemLocation::eCode Code;
	BYTE cItemSlotIndex;
	__time64_t tUnLockDate;
	__time64_t tUnLockRequestDate;
};

struct TQGetListLockedItems: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListLockedItems: public TAHeader
{
	int nCount;
	TDBLockItemInfo LockItem[EQUIPMAX+ CASHEQUIPMAX];
};
#endif	// #if defined(PRE_ADD_EQUIPLOCK)


//#########################################################################################################################################
//	MAINCMD_QUEST
//#########################################################################################################################################

// QUERY_ADDQUEST
struct TQAddQuest:public TQHeader
{
	INT64 biCharacterDBID;
	char cIdx;
	TQuest Quest;
};

// QUERY_DELQUEST
struct TQDelQuest:public TQHeader
{
	INT64 biCharacterDBID;
	char cIdx;
	int nQuestID;
	bool bComplete;		// 0=Simple quest deletion, 1=Deletion due to quest completion
	bool bRepeat;		// 0=One-time quest, 1=Repeatable quest
	char CompleteQuest[COMPLETEQUEST_BITSIZE];
};

// QUERY_MODQUEST, // Modify quest
struct TQModQuest:public TQHeader
{
	INT64 biCharacterDBID;
	char cQuestStatusCode;
	char cIndex;
	TQuest Quest;
};

// QUERY_CLEARQUEST
struct TQClearQuest:public TQHeader
{
	INT64 biCharacterDBID;
};


// QUERY_FORCECOMPLETEQUEST
struct TQForceCompleteQuest : public TQHeader
{
public:
	TQForceCompleteQuest()
	{
		Reset();
	}
	void Set(BYTE pThreadID, char pWorldID, UINT pAccountDBID, INT64 pCharacterDBID, int pQuestID, int pQuestCode, char pCompleteQuest[COMPLETEQUEST_BITSIZE])
	{
		cThreadID = pThreadID;
		cWorldSetID = pWorldID;
		nAccountDBID = pAccountDBID;
		biCharacterDBID = pCharacterDBID;
		nQuestID = pQuestID;
		nQuestCode = pQuestCode;
		::memcpy_s(CompleteQuest, sizeof(CompleteQuest), pCompleteQuest, sizeof(CompleteQuest));
	}
	void Reset()
	{
		cThreadID = 0;
		cWorldSetID = 0;
		nAccountDBID = 0;
		biCharacterDBID = 0;
		nQuestID = 0;
		nQuestCode = 0;
	}
	int GetSize() const
	{ 
		return(static_cast<int>(sizeof(*this)));
	}
public:
	INT64 biCharacterDBID;						// Character ID
	int nQuestID;								// Quest ID
	int nQuestCode;								// Quest code (0: Normal / 1: Timed)
	char CompleteQuest[COMPLETEQUEST_BITSIZE];	// Completed quest binary list
};


// QUERY_DELETE_CHARACTER_PERIODQUEST
struct TQDeletePeriodQuest : public TQHeader
{
	char cWorldID;
	INT64 biCharacterDBID;
	int nPeriodQuestType;
	int nQuestCount;
	int nQuestIDs[MAX_PERIODQUEST_LIST];
};

struct TADeletePeriodQuest : public TAHeader
{
	char cWorldID;
	INT64 biCharacterDBID;
	__time64_t tAssignDate;

	int nPeriodQuestType;
	int nQuestCount;
	int nQuestIDs[MAX_PERIODQUEST_LIST];
};

// QUERY_GET_CHARACTER_PERIODQUESTDATE
struct TQGetPeriodQuestDate : public TQHeader
{
	char cWorldID;
	INT64 biCharacterDBID;
};

struct TAGetPeriodQuestDate : public TAHeader
{
	char cWorldID;
	INT64 biCharacterDBID;

	int nCount;
	TPeriodQuestDate PeriodQuestDate[PERIODQUEST_RESET_MAX];
};

// QUERY_GET_LISTCOMPLETE_EVENTQUEST
struct TQGetListCompleteEventQuest : public TQHeader
{
	char cWorldID;
	INT64 biCharacterDBID;
};

struct TAGetListCompleteEventQuest : public TAHeader
{
	char cWorldID;
	INT64 biCharacterDBID;

	int nCount;
	TCompleteEventQuest EventQuestList[EVENTQUESTMAX];
};

// QUERY_COMPLETE_EVENTQUEST
struct TQCompleteEventQuest : public TQHeader
{
	char cWorldID;
	INT64 biCharacterDBID;

	int nScheduleID;
	__time64_t tExpireDate;
};

struct TACompleteEventQuest : public TAHeader
{
	char cWorldID;
	INT64 biCharacterDBID;

	int nScheduleID;
	short wCompleteCount;
};

// QUERY_GET_WORLDEVENTQUESTCOUNTER
struct TQGetWorldEventQuestCounter : public TQHeader
{
	int nScheduleID;
};

struct TAGetWorldEventQuestCounter : public TAHeader
{
	char cWorldSetID;
	int nScheduleID;
	int nCount;
};

// QUERY_MOD_WORLDEVENTQUESTCOUNTER
struct TQModWorldEventQuestCounter : public TQHeader
{
	INT64 biCharacterDBID;
	int nScheduleID;
	int nCounterDelta;
};

struct TAModWorldEventQuestCounter : public TAHeader
{
	char cWorldSetID;
	int nScheduleID;
	int nCounterDelta;
	int nCountAfter;
};

//#########################################################################################################################################
//	MAINCMD_MAIL
//#########################################################################################################################################

// QUERY_GETCOUNTRECEIVEMAIL
struct TQGetCountReceiveMail:public TQHeader
{
	INT64 biReceiverCharacterDBID;
};

struct TAGetCountReceiveMail:public TAHeader
{
	int	iTotalMailCount;
	int	iNotReadMailCount;
	int	i7DaysLeftMailCount;
};

// QUERY_GETPAGERECEIVEMAIL
struct TQGetPageReceiveMail:public TQHeader
{
	int	iPageNumber;
	INT64 biReceiverCharacterDBID;
};

struct TAGetPageReceiveMail:public TAHeader
{
	int	iBasicMailCount;

	int	iTotalMailCount;
	int	iNotReadMailCount;
	int	i7DaysLeftMailCount;

	BYTE cMailCount;
	TMailBoxInfo sMail[MAILPAGEMAX];
};

// QUERY_SENDMAIL
struct TQSendMail:public TQHeader
{
	INT64 biSenderCharacterDBID;
	wchar_t	wszReceiverCharacterName[MAILNAMELENMAX];
	MailType::Delivery Code;
	wchar_t wszSubject[MAILTITLELENMAX];
	wchar_t wszContent[MAILTEXTLENMAX];
	INT64 biAttachCoin;
	int	iChannelID;
	int	iMapID;
	int	nTax;								// Fee
	BYTE cAttachItemCount;
	DBPacket::TSendMailItem AttachItemArr[MAILATTACHITEMMAX];
};

struct TASendMail:public TAHeader
{
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverTotalMailCount;
	int	nReceiverNotReadMailCount;
	int	nReceiver7DaysLeftMailCount;
	int nBasicMailCount;
	MailType::Delivery Code;

	INT64 biAttachCoin;
	int nTax;								// Fee
	BYTE cAttachItemCount;
	DBPacket::TSendMailItem AttachItemArr[MAILATTACHITEMMAX];
};

// QUERY_SENDSYSTEMMAIL
struct TQSendSystemMail:public TQHeader
{
	wchar_t	wszSenderName[MAILNAMELENMAX];
	INT64 biReceiverCharacterDBID;
	DBDNWorldDef::MailTypeCode::eCode Code;
	INT64 biFKey;
	wchar_t	wszSubject[MAILTITLELENMAX];
	wchar_t wszContent[MAILTEXTLENMAX];
	INT64 biAttachCoin;
	int	iChannelID;
	int	iMapID;
	bool bNewFlag;
	char cAttachItemCount;
	TItem sAttachItem[MAILATTACHITEMMAX];
};

struct TASendSystemMail:public TAHeader
{
	INT64 biReceiverCharacterDBID;
	int nMailDBID;
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	BYTE cWorldSetID;
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
	int	iTotalMailCount;
	int	iNotReadMailCount;
	int	i7DaysLeftMailCount;
};

// QUERY_READMAIL
struct TQReadMail:public TQHeader
{
	int iMailID;
};

struct TAReadMail:public TAHeader
{
	int	nMailDBID;							// mail db table id
	int	iNotReadMailCount;
	wchar_t	wszContent[MAILTEXTLENMAX];
	__time64_t SendDate;
	INT64 biAttachCoin;
	char cDeliveryType;
	bool bNewFlag;		// Whether it is a renewed system mail
	BYTE cMailType;
	BYTE cAttachItemCount;
	TItem sAttachItem[MAILATTACHITEMMAX];
};

// QUERY_GETLISTMAILATTACHMENT
struct TQGetListMailAttachment: public TQHeader
{
	INT64 biCharacterDBID;
	int nMailDBIDArray[MAILPAGEMAX];
};

struct TAGetListMailAttachment: public TAHeader
{
	char cCount;
	DBPacket::TAttachMail MailAttachArray[MAILPAGEMAX];
};

// QUERY_TAKEATTACHMAILLIST
struct TQTakeAttachMailList: public TQHeader
{
	int nChannelID;
	int	nMapID;
	WCHAR wszIP[IPLENMAX];
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	UINT nGuildDBID;
#endif		//PRE_ADD_JOINGUILD_SUPPORT
	char cPageCount;
	DBPacket::TRequestTakeAttachInfo Info[MAILPAGEMAX];
};

struct TATakeAttachMailList: public TAHeader
{
	char cPageCount;
	DBPacket::TTakeAttachInfo TakeMailList[MAILPAGEMAX];
};

// QUERY_TAKEATTACHMAIL
struct TQTakeAttachMail: public TQHeader
{
	int nChannelID;
	int	nMapID;
	WCHAR wszIP[IPLENMAX];
#ifdef PRE_ADD_JOINGUILD_SUPPORT
	UINT nGuildDBID;
#endif		//PRE_ADD_JOINGUILD_SUPPORT
	DBPacket::TRequestTakeAttachInfo Info;
};

struct TATakeAttachMail: public TAHeader
{
	char cAttachSlotIndex;		// -1: Claim all 5 slots, 0 - 4: Claim individually
	DBPacket::TTakeAttachInfo TakeMail;
};

// QUERY_DELMAIL
struct TQDelMail:public TQHeader
{
	INT64 biReceiverCharacterDBID;
	int	iMailIDArray[MAILPAGEMAX];
};

struct TADelMail:public TAHeader
{
	int	iTotalMailCount;
};

// QUERY_GETWILLSENDMAIL
struct TQGetWillMails : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetWillMails : public TAHeader
{
	BYTE cCount;
	WillSendMail::TWillSendInfo Mails[WillSendMail::Common::WIllMailPacketMax];
};

// QUERY_DELWILLSENDMAIL
struct TQDelWillMail : public TQHeader
{
	INT64 biCharacterDBID;
	int nMailID;
};

// QUERY_SENDWISHMAIL, // Send wish mail
struct TQSendWishMail: public TQHeader
{
	INT64 biSenderCharacterDBID;
	WCHAR wszReceiverCharacterName[MAILNAMELENMAX];
	WCHAR wszSubject[MAILTITLELENMAX];
	WCHAR wszContent[MAILTEXTLENMAX];
	int nWishLimitCount;	// Daily gift request limit
	int nPackageSN;
	char cWishListCount;
	DBPacket::TItemSNIDOption WishList[PACKAGEITEMMAX];
};

struct TASendWishMail: public TAHeader
{
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverTotalMailCount;
	int	nReceiverNotReadMailCount;
	int	nReceiver7DaysLeftMailCount;
};

// QUERY_READWISHMAIL, // Read wish mail
struct TQReadWishMail: public TQHeader
{
	int nMailID;
};

struct TAReadWishMail: public TAHeader
{
	int	nMailDBID;							// mail db table id
	wchar_t	wszContent[MAILTEXTLENMAX];
	__time64_t SendDate;
	INT64 biPurchaseOrderID;
	int	nNotReadMailCount;
	bool bNewFlag;		// Whether it is a renewed system mail
	int nPackageSN;
	char cWishListCount;
	DBPacket::TItemSNIDOption WishList[PACKAGEITEMMAX];
};

#if defined(PRE_SPECIALBOX)
// QUERY_ADDEVENTREWARD
struct TQAddEventReward: public TQHeader
{
	BYTE cReceiveTypeCode;		// SpecialBox::ReceiveTypeCode
	BYTE cTargetTypeCode;		// SpecialBox::TargetTypeCode
	WCHAR wszAccountName[IDLENMAX];
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
	BYTE cTargetClassCode;
	BYTE cTargetMinLevel;
	BYTE cTargetMaxLevel;
	int nEventNameUIString;
	int nSenderNameUIString;
	int nContentUIString;
	int nRewardCoin;
	char cEventRewardTypeCode;	// eEventRewardType
	BYTE cCount;
	TSpecialBoxItemInfo EventRewardItems[SpecialBox::Common::RewardSelectMax];
};

struct TAAddEventReward: public TAHeader
{
	int nEventRewardID;
	int nEventTotalCount;
};

// QUERY_GETCOUNTEVENTREWARD
struct TQGetCountEventReward: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetCountEventReward: public TAHeader
{
	int nEventTotalCount;
};

// QUERY_GETLISTEVENTREWARD
struct TQGetListEventReward: public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cClassCode;
	int nLevel;
};

struct TAGetListEventReward: public TAHeader
{
	BYTE cCount;
	TSpecialBoxInfo SpecialBoxInfo[SpecialBox::Common::ListMax];
};

// QUERY_GETLISTEVENTREWARDITEM
struct TQGetListEventRewardItem: public TQHeader
{
	int nEventRewardID;
	INT64 biCharacterDBID;
};

struct TAGetListEventRewardItem: public TAHeader
{
	int nEventRewardID;
	char cCharacterCount;
	BYTE cCount;
	TSpecialBoxItemInfo SpecialBoxItem[SpecialBox::Common::RewardSelectMax];
};

// QUERY_ADDEVENTREWARDRECEIVER
struct TQAddEventRewardReceiver: public TQHeader
{
	int nEventRewardID;
	int nItemID;
	INT64 biCharacterDBID;
};

struct TAAddEventRewardReceiver: public TAHeader
{
	int nEventRewardID;
	int nItemID;
};
#endif	// #if defined(PRE_SPECIALBOX)

#ifdef PRE_ADD_JOINGUILD_SUPPORT
// QUERY_GUILDSUPPORT_REWARDINFO
struct TQGuildSupportRewardInfo:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGuildSupportRewardInfo:public TAHeader
{
	INT64 biCharacterDBID;
	bool bWasGuildSupportRewardFlag;
	BYTE cJoinGuildLevel;
};

// QUERY_SENDGUILDMAIL
struct TQSendGuildMail:public TQHeader
{
	wchar_t	wszSenderName[MAILNAMELENMAX];
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	INT64 biAchivementCharacterDBID;
	DBDNWorldDef::MailTypeCode::eCode Code;
	INT64 biFKey;
	wchar_t	wszSubject[MAILTITLELENMAX];
	wchar_t wszContent[MAILTEXTLENMAX];
	INT64 biAttachCoin;
	int	iChannelID;
	int	iMapID;
	bool bNewFlag;
	char cAttachItemCount;
	TItem sAttachItem[MAILATTACHITEMMAX];
};

struct TASendGuildMail:public TAHeader
{
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nMailDBID;
	BYTE cWorldSetID;
	int	iTotalMailCount;
	int	iNotReadMailCount;
	int	i7DaysLeftMailCount;
};
#endif		//#ifdef PRE_ADD_JOINGUILD_SUPPORT


//#########################################################################################################################################
//	MAINCMD_MARKET
//#########################################################################################################################################

// QUERY_MARKETTAX
struct TQMarketTax:public TQHeader
{
	INT64 biCharacterDBID;
	int nMarketRegisterTax;					// Market cumulative registration fee
	int nMarketSellTax;						// Market cumulative sales fee
};

// QUERY_GETPAGETRADE
struct TQGetPageTrade: public TQHeader
{
	short wPageNum;						// Page number
	char cMinLevel;						// Min level
	char cMaxLevel;						// Max level
	char cItemGrade[ITEMRANK_MAX];		// Grade classification
	char cJob;							// Job classification
	short nMainType;					// Default category
	char cDetailType;					// Subcategory
	WCHAR wszSearchWord1[SEARCHLENMAX];	// Search keyword
	WCHAR wszSearchWord2[SEARCHLENMAX];	// Search keyword
	WCHAR wszSearchWord3[SEARCHLENMAX];	// Search keyword
	char cSortType;						// eSortType
	char cItemExchange[ITEMCATAGORY_MAX]; // Item category
	WCHAR wszSearchItemName[SEARCHLENMAX];	// Item name
	char cJobCode[ITEMJOB_MAX];			// Job code
	bool bCash;							// 0=Normal item, 1=Cash item
	char cMethodCode;					// 1=Game money, 3=Petal
};

struct TAGetPageTrade: TAHeader
{
	int nMarketTotalCount;
	char cMarketCount;
	TMarketInfo MarketInfo[MARKETMAX];
};

// QUERY_GETLISTMYTRADE
struct TQGetListMyTrade:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListMyTrade: public TAHeader
{
	short wSellingCount;	// On sale
	short wExpiredCount;
	short wWeeklyRegisterCount;		// Weekly registered trade count based on Saturday 04:00 AM / remaining count registerable now
	short wRegisterItemCount;		// Number of placed items
	char cMarketCount;
	TMyMarketInfo MarketInfo[MYMARKETMAX];
};

// QUERY_ADDTRADE
struct TQAddTrade: public TQHeader
{
	INT64 biCharacterDBID;			// Character ID
	short wRegisterMaxCount;		// Registrable attempts
	short wItemRegisterMaxCount;	// Registrable count
	char cInvenType;				// eItemPositionType
	BYTE cInvenIndex;				// Inven index
	char cSkillUsingType;			// Sell price
	int nPrice;						// Market registration fee
	int nSellTax;					// Trade completion fee (fee when claiming proceeds of sold item)
	char cSellPeriod;				// Sale period
	WCHAR wszItemName[EXTDATANAMELENMAX];	// Item name
	short nMainType;				// Main type
	char cDetailType;				// Detail type
	char cItemGrade;				// 0=Normal, 1=Magic, 2=Rare, 3=Epic, 4=Unique, 5=Heroic
	char cLimitLevel;				// Minimum character level to use the item
	char cJob;						// Job code
	int nItemID;					// Registered item ID
	INT64 nSerial;					// Unique serial number of item to register
	short wCount;					// Quantity of item to register
	char cLevel;					// Level of item to register
	INT64 biNewSerial;				// Enter a new ItemSerial when registering only a partial quantity of an item, otherwise enter NULL
	int nUnitPrice;					// Sale unit price
	int nRegisterTax;				// Market registration fee
	bool bDisplayFlag1;				// Please enter 0. Currently unused field
	bool bDisplayFlag2;				// Please enter 0. Currently unused field
	int nChannelID;					// Channel ID
	int nMapID;						// Map ID
	bool bPremiumTrade;				// 0=Not premium, 1=Premium
	bool bCash;						// 0=Normal item, 1=Cash item
	char cMethodCode;				// 1=Game money, 3=Petal
	int nExchangeCode;				// Category ID the item belongs to
};

struct TAAddTrade: TAHeader
{
	int nMarketDBID;			// Market trade ID
	char cInvenType;			// eItemPositionType
	INT64 biSerial;				// Serial
	BYTE cInvenIndex;			// Inven index
	short wCount;				// Item count
	int nPrice;					// Market registration fee
	bool bPremiumTrade;			// 0=Not premium, 1=Premium
#if defined(PRE_ADD_PETALTRADE)
	char cMethodCode;			// 1=Game money, 3=Petal
#endif
};

// QUERY_CANCELTRADE
struct TQCancelTrade:public TQHeader
{
	INT64 biCharacterDBID;		// Character ID
	int nMarketDBID;			// Trade ID
	BYTE cInvenSlotIndex;		// Index number where the item will be placed
	int nChannelID;				// Channel ID
	int nMapID;					// Map ID
	bool bCashItem;				
};

struct TACancelTrade: public TAHeader
{
	int nMarketDBID;
	BYTE cInvenSlotIndex;
	TItem Item;
	int nPrice;
	bool bCashItem;
};

// QUERY_BUYTRADEITEM
struct TQBuyTradeItem: public TQHeader
{
	INT64 biCharacterDBID;
	int nMarketDBID;
	BYTE cInvenSlotIndex;
	int nChannelID;
	int nMapID;
	bool bCashItem;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	bool bMini;
#else	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
#if defined(PRE_ADD_PETALTRADE)
	int nPetalPrice;
#endif
#endif	// #if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	WCHAR wszIP[IPLENMAX];
};

struct TABuyTradeItem: public TAHeader
{
	int nMarketDBID;
	BYTE cInvenSlotIndex;
	TItem Item;
	UINT nSellerADBID;	// Seller ADBID
	INT64 biSellerCharacterDBID;	// Seller CDBID
	int nPrice;
	char cPayMethodCode;
#if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
	bool bMini;
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
};

// QUERY_GETLISTTRADEFORCALCULATION
struct TQGetListTradeForCalculation: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListTradeForCalculation: public TAHeader
{
	char cMarketCount;
	TMarketCalculationInfo MarketInfo[MYMARKETMAX];
};

// QUERY_TRADECALCULATE
struct TQTradeCalculate: public TQHeader
{
	INT64 biCharacterDBID;
	int nMarketDBID;
	int nChannelID;
	int nMapID;
};

struct TATradeCalculate: public TAHeader
{
	int nMarketDBID;
	int nCalculatePrice;
#if defined(PRE_ADD_PETALTRADE)
	char cPayMethodCode;
#endif
};

// QUERY_TRADECALCULATEALL
struct TQTradeCalculateAll: public TQHeader
{
	INT64 biCharacterDBID;
	int nChannelID;
	int nMapID;
};

struct TATradeCalculateAll: public TAHeader
{
	int nCalculatePrice;
#if defined(PRE_ADD_PETALTRADE)
	int nCalculatePetal;
#endif
};

#if !defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)
// QUERY_GETTRADEITEMID, // Get ItemID by MarketDBID
struct TQGetTradeItemID: public TQHeader
{
	int nMarketDBID;
};
#endif	// #if defined(PRE_ADD_DIRECT_BUY_UPGRADEITEM)

struct TAGetTradeItemID: public TAHeader
{
	int nMarketDBID;
	int nMarketItemID;
#if defined(PRE_ADD_PETALTRADE)
	char cMethodCode;
	int nPrice;
#endif
};

// QUERY_GETPETALBALANCE, // Query petal balance
struct TAGetPetalBalance : public TAHeader
{
	int nPetalBalance;
};

// QUERY_GETTRADEPRICE
struct TQGetTradePrice : public TQHeader		
{
	int nMarketDBID;
	int nItemID;
	BYTE cLevel;
	BYTE cOption;
};

struct TAGetTradePrice : public TAHeader
{
	int nMarketDBID;
	int nItemID;
	BYTE cLevel;
	BYTE cOption;

	TMarketPrice ItemPrices[2];		// Index 0: Game money, 1: Petal
};

//QUERY_GETLISTMINITRADE
struct TQGetListMiniTrade: public TQHeader
{
	INT64 biCharacterDBID;
	int nItemID;
	char cPayMethodCode;					// 0-Mixed, 1-Coin, 3-Petal
};

typedef TAGetPageTrade TAGetListMiniTrade;


//#########################################################################################################################################
//	MAINCMD_FRIEND
//#########################################################################################################################################

//QUERY_FRIENDLIST,
struct TQFriendList:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TDBFriendGroupData
{
	UINT nGroupDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
};

struct TDBFriendData
{
	UINT nGroupDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
	INT64 biCharacterDBID;
	UINT nFriendAccountDBID;
	INT64 biFriendCharacterDBID;
	WCHAR wszFriendCharacterName[NAMELENMAX];
	WCHAR wszMemo[FRIEND_MEMO_LEN_MAX];
	bool bMobileAuthentication;
};

struct TAFriendList: public TAHeader
{
	INT64 biCharacterDBID;
	BYTE cCount;
	TDBFriendData FriendData[FRIEND_MAXCOUNT + FRIEND_GROUP_MAX];
};

// QUERY_ADDGROUP, // Add group
struct TQAddGroup:public TQHeader
{
	INT64 biCharacterDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
	int nGroupCountLimit;		// Max friend group count
};

struct TAAddGroup: public TAHeader
{
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];	
	UINT nGroupDBID;
};

// QUERY_MODGROUPNAME, // Modify group name
struct TQModGroupName:public TQHeader
{
	INT64 biCharacterDBID;
	int nGroupDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];
};

struct TAModGroupName: public TAHeader
{
	UINT nGroupDBID;
	WCHAR wszGroupName[FRIEND_GROUP_NAMELENMAX];	
};

// QUERY_DELGROUP, // Delete group
struct TQDelGroup:public TQHeader
{
	INT64 biCharacterDBID;
	UINT nGroupDBID;
};

struct TADelGroup: public TAHeader
{
	UINT nGroupDBID;
};

// QUERY_MODFRIENDANDGROUPMAPPING, // Modify friend and friend group mapping
struct TQModFriendAndGroupMapping:public TQHeader
{
	INT64 biCharacterDBID;
	int nGroupDBID;
	char cCount;
	INT64 biFriendCharacterDBIDs[FRIEND_MAXCOUNT];
};

struct TAModFriendAndGroupMapping: public TAHeader
{
	int nGroupDBID;
	char cCount;
	INT64 biFriendCharacterDBIDs[FRIEND_MAXCOUNT];
};

//QUERY_ADDFRIEND,
struct TQAddFriend:public TQHeader
{
	INT64 biCharacterDBID;

	WCHAR wszFriendName[NAMELENMAX];
	UINT nGroupDBID;
	int nFriendCountLimit;	// Max number of friends that can be registered
};

struct TAAddFriend: public TAHeader
{
	INT64 biCharacterDBID;
	UINT nFriendAccountDBID;
	INT64 biFriendCharacterDBID;
	UINT nGroupDBID;
	WCHAR wszFriendName[NAMELENMAX];
	BYTE cNeedDelIsolate;
};

// QUERY_MODFRIENDMEMO, // Modify friend memo
struct TQModFriendMemo:public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biFriendCharacterDBID;
	WCHAR wszMemo[FRIEND_MEMO_LEN_MAX];
};

struct TAModFriendMemo: public TAHeader
{
	INT64 biCharacterDBID;
};

//QUERY_DELFRIEND,
struct TQDelFriend:public TQHeader
{
	INT64 biCharacterDBID;
	//Only single-item interface exists for now
	WCHAR wszDelCharacterName[NAMELENMAX];
	char cCount;
	INT64 biFriendCharacterDBIDs[FRIEND_MAXCOUNT];
};

struct TADelFriend: public TAHeader
{
	char cCount;
	INT64 biFriendCharacterDBIDs[FRIEND_MAXCOUNT];
};


//#########################################################################################################################################
//	MAINCMD_ISOLATE
//#########################################################################################################################################

struct TIsolateItem:public TBoostMemoryPool<TIsolateItem>
{
	INT64 biIsolateCharacterDBID;
	WCHAR wszIsolateName[NAMELENMAX];
};

//QUERY_GETISOLATELIST
struct TQGetIsolateList:public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetIsolateList: public TAHeader
{
	BYTE cCount;
	TIsolateItem Isolate[ISOLATELISTMAX];
};

//QUERY_ADDISOLATE
struct TQAddIsolate:public TQHeader
{
	INT64 biCharacterDBID;
	WCHAR wszIsolateName[NAMELENMAX];
	INT64 biFriendCharacterDBID;
};

struct TAAddIsolate: public TAHeader
{
	BYTE cNeedDelFriend;
	TIsolateItem IsolateItem;
};

//QUERY_DELISOLATE
struct TQDelIsolate:public TQHeader
{
	INT64 biCharacterDBID;
	char cCount;
	INT64 biIsolateCharacterDBIDs[ISOLATELISTMAX];
};

struct TADelIsolate: public TAHeader
{
	char cCount;
	INT64 biIsolateCharacterDBIDs[ISOLATELISTMAX];
};


//#########################################################################################################################################
//	MAINCMD_GUILD
//#########################################################################################################################################

// QUERY_CREATEGUILD // Request/result to create guild
struct TQCreateGuild: public TQHeader
{
	INT64 nCharacterDBID;		// Guild master character DBID
	WCHAR wszGuildName[GUILDNAME_MAX];				// Guild name
	int wGuildRoleAuth[GUILDROLE_TYPE_CNT];	// Guild role

	int nMapID;				// Map number
	int nEstablishExpense;	// Guild creation fee
	char cLevel;				// Creator level
	INT nRoleMaxItem[GUILDROLE_TYPE_CNT];				// Guild storage item count limit by guild role (index)
	INT nRoleMaxCoin[GUILDROLE_TYPE_CNT];				// Max withdrawal amount by guild role (index)
};

struct TACreateGuild: public TAHeader
{
	INT64 nCharacterDBID;		// Guild master character DBID
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	char cWorldSetID;			// World ID
	UINT nGuildDBID;	// Guild DBID (ignored on failure)
	__time64_t JoinDate;	// Guild join date
	int nEstablishExpense;	// Guild creation fee
};

// QUERY_GETGUILDINFO // Request guild info
struct TQGetGuildInfo: public TQHeader
{
	UINT nGuildDBID;	// Guild DBID
	bool bNeedMembList;		// Whether guild member list is also needed
};

struct TAGetGuildInfo: public TAHeader
{
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;	// World ID
	TGuild Info;		// Guild info (ignored on failure)
};

struct TAGetGuildMember: public TAHeader
{
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;	// World ID	
	bool bFirstPage;	// Check if first time
	bool bEndPage;		// Check if it is the last one
	int nCount;			// Guild member info count (ignored on failure)	
	TGuildMember MemberList[SENDGUILDMEMBER_MAX];	// Guild member info
};

// QUERY_DISMISSGUILD // Request to disband guild
struct TQDismissGuild: public TQHeader
{
	INT64 nCharacterDBID;		// Guild master character DBID
	UINT nGuildDBID;	// Guild DBID
};

struct TADismissGuild: public TAHeader
{
	INT64 nCharacterDBID;		// Guild master character DBID
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;			// World ID
};

// QUERY_ADDGUILDMEMBER // Request to add guild member
struct TQAddGuildMember: public TQHeader
{
	UINT nAccountDBID;		// Account DBID to add
	INT64 nCharacterDBID;		// Character DBID to add
	UINT nGuildDBID;	// Guild DBID
	char cLevel;				// Character level
	short wGuildSize;			// Max guild member count
};

struct TAAddGuildMember: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID to add
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;			// World ID
	__time64_t JoinDate;	// Guild join date
};

// QUERY_DELGUILDMEMBER // Request to remove guild member (leave/kick)
struct TQDelGuildMember: public TQHeader
{
	UINT nReqAccountDBID;	// Requesting account DBID (differs from removed target in case of kick)
	INT64 nReqCharacterDBID;	// Requesting character DBID (differs from removed target in case of kick)
	UINT nDelAccountDBID;	// Account DBID to remove 
	INT64 nDelCharacterDBID;	// Character DBID to remove
	UINT nGuildDBID;	// Guild DBID
	bool bIsExiled;			// Whether expelling (true : expel / false : leave)
	char cLevel;				// Character level
#ifdef PRE_ADD_BEGINNERGUILD
	bool bIsGraduateBeginnerGuild;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
};

struct TADelGuildMember: public TAHeader
{
	UINT nReqAccountDBID;	// Requesting account DBID (differs from removed target in case of kick)
	INT64 nReqCharacterDBID;	// Requesting character DBID (differs from removed target in case of kick)
	UINT nDelAccountDBID;	// Account DBID to remove 
	INT64 nDelCharacterDBID;	// Character DBID to remove
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;			// World ID
	bool bIsExiled;			// Whether expelling (true : expel / false : leave)
#ifdef PRE_ADD_BEGINNERGUILD
	bool bIsGraduateBeginnerGuild;
#endif		//#ifdef PRE_ADD_BEGINNERGUILD
};

// QUERY_CHANGEGUILDINFO // Request to change guild info
struct TQChangeGuildInfo:public TQHeader
{
	INT64 nCharacterDBID;				// Character DBID
	UINT nGuildDBID;			// Guild DBID
	BYTE btGuildUpdate;		// Guild info change type (eGuildUpdateType)
	long nAddCommonPoint;	// Guild normal point delta (0 : no change / otherwise : must be negative to deduct !!!)
	int nInt1;
	int nInt2;
	int nInt3;
	int nInt4;
	INT64 biInt64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
};

struct TAChangeGuildInfo: public TAHeader
{
	INT64 nCharacterDBID;				// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;					// World ID
	BYTE btGuildUpdate;		// Guild info change type (eGuildUpdateType)
	long nCurCommonPoint;	// Guild normal point current amount (filled only when change is requested)
	long nTotCommonPoint;	// Guild normal point accumulated amount (filled only when change is requested)
	int nInt1;
	int nInt2;
	int nInt3;
	int nInt4;
	INT64 biInt64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
};

// QUERY_CHANGEGUILDMEMBERINFO // Request to change guild member info
struct TQChangeGuildMemberInfo:public TQHeader
{
	UINT nReqAccountDBID;	// Requesting account DBID (differs from target when guild master modifies guild member info)
	INT64 nReqCharacterDBID;	// Requesting character DBID (differs from target when guild master modifies guild member info)
	UINT nChgAccountDBID;	// Account DBID to be changed 
	INT64 nChgCharacterDBID;	// Character DBID to be changed
	UINT nGuildDBID;	// Guild DBID
	BYTE btGuildMemberUpdate;		// Guild member info update type (eGuildMemberUpdateType)
	int nInt1;
	int nInt2;
	INT64 biInt64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
	char cLevel;
};

struct TAChangeGuildMemberInfo: public TAHeader
{
	UINT nReqAccountDBID;	// Requesting account DBID (differs from target when guild master modifies guild member info)
	INT64 nReqCharacterDBID;	// Requesting character DBID (differs from target when guild master modifies guild member info)
	UINT nChgAccountDBID;	// Account DBID to be changed 
	INT64 nChgCharacterDBID;	// Character DBID to be changed
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;			// World ID
	BYTE btGuildMemberUpdate;		// Guild member info update type (eGuildMemberUpdateType)
	int nInt1;
	int nInt2;
	INT64 biInt64;
	WCHAR Text[GUILDHISTORYTEXT_MAX+1];
};

// QUERY_ADDGUILDHISTORY: // Request to add guild history
struct TQAddGuildHistory: public TQHeader
{
	UINT nGuildDBID;	// Guild DBID
	TGuildHistory History;	// Guild history info
};

// QUERY_GETGUILDHISTORYLIST // Request guild history list
struct TQGetGuildHistoryList: public TQHeader
{
	INT64 nCharacterDBID;			// Character DBID
	UINT nGuildDBID;		// Guild DBID
	INT64 biIndex;	// Guild history list index (if 0, check from most recent?)
	bool bDirection;				// Direction to get list (0: before index / 1: after index)
};

struct TAGetGuildHistoryList: public TAHeader
{
	INT64 nCharacterDBID;			// Character DBID
	UINT nGuildDBID;		// Guild DBID
	char cWorldSetID;				// World ID
	INT64 biIndex;	// Guild history list index (if 0, check from most recent?)
	bool bDirection;				// Direction to get list (0: before index / 1: after index)
	int nCurCount;				// Count from first guild history index to input index
	int nTotCount;				// Total guild history count
	int nCount;					// Guild history list count (ignored on failure)
	TGuildHistory HistoryList[GUILDHISTORYLIST_MAX];	// Guild history list (ignored on failure)
};

//QUERY_INVITEGUILDMEMBER: // Request to invite guild member
struct TQInviteGuildMember : public TQHeader
{
	TQGetGuildInfo GuildInfo;
	UINT nToSessionID;		// Session ID of the user who accepted the invite
};

struct TAInviteGuildMember: public TAHeader
{
	TAGetGuildInfo GuildInfo;
	UINT nToSessionID;				// Session ID of the user who accepted the invite
};

// QUERY_MOVE_ITEM_IN_GUILDWARE // Move item from inventory -> guild storage
struct TQMoveItemInGuildWare : public TQHeader
{
	INT64 nCharacterDBID;			// Character DBID
	UINT nGuildDBID;				// Guild DBID

	int	nItemID;				// Item ID to move
	INT64 biItemSerial;			// Unique serial of the item to move

	INT64 biNewItemSerial;		// Unique ID of the item produced by Split
	INT64 biMergeTargetItemSerial;// Serial number of target item to merge

	short wItemCount;				// Total quantity before change for the item matching @inbItemSerial
	short wSplitItemCount;		// Item quantity for @inbNewItemSerial
	short wMergeItemCount;		// Previous quantity of the item being merged

	int	nMapIndex;				// Map ID

	DBPacket::TExchangeItem SrcItem;
	DBPacket::TExchangeItem DestItem;
};

struct TAMoveItemInGuildWare: public TAHeader
{
	char cWorldSetID;					// World ID
	INT64 nCharacterDBID;				// Character DBID
	UINT nGuildDBID;					// Guild DBID

	DBPacket::TExchangeItem SrcItem;
	DBPacket::TExchangeItem DestItem;
};

// QUERY_MOVE_INVEN_TO_GUILDWARE // Move item from inventory -> guild storage
struct TQMoveInvenToGuildWare : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID

	int nItemID;			// Item ID to move
	INT64 biItemSerial;		// Unique serial of the item to move
	
	INT64 biNewItemSerial;	// New serial of the split item when moving a partial quantity. Pass NULL if not split.
	INT64 biMergeTargetItemSerial; // Target item serial when merging into an existing item. Pass NULL if not merging.

	short wItemCount;			// Quantity of the item to move
	short wMovingItemCount;	// Previous quantity of item being moved
	short wMergeItemCount;	// Previous quantity of the item being merged
	int nMapIndex;			// Map ID

	DBPacket::TExchangeItem InvenItem;
	DBPacket::TExchangeItem GuildWareItem;
};

struct TAMoveInvenToGuildWare: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;			// World ID

	DBPacket::TExchangeItem InvenItem;
	DBPacket::TExchangeItem GuildWareItem;
};

// QUERY_MOVE_GUILDWARE_TO_INVEN // Move item from guild storage -> inventory
struct TQMoveGuildWareToInven : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID

	int nItemID;			// Item ID to move
	INT64 biItemSerial;		// Unique serial of the item to move

	INT64 biNewItemSerial;	// New serial of the split item when moving a partial quantity. Pass NULL if not split.
	INT64 biMergeTargetItemSerial; // Target item serial when merging into an existing item. Pass NULL if not merging.

	short wItemCount;			// Quantity of the item to move
	short wMovingItemCount;	// Previous quantity of item being moved
	short wMergeItemCount;	// Previous quantity of the item being merged

	int	nMaxDailyTakeItemCount;
	int	nMapIndex;			// Map ID

	DBPacket::TExchangeItem GuildWareItem;
	DBPacket::TExchangeItem InvenItem;
};

struct TAMoveGuildWareToInven: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;			// World ID

	int	nDailyTakeItemCount;
	__time64_t tUseDate;

	DBPacket::TExchangeItem GuildWareItem;
	DBPacket::TExchangeItem InvenItem;
};

struct TQGuildWareHouseCoin : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID

	char cMoveType;
	bool bInOutFlag;			// 0: Deposit, 1: Withdraw
	INT64 nMoveCoin;			// Deposit amount
	int	nMapIndex;			// Map ID

	char szIP[IPLENMAX];		// IP address
};

struct TAGuildWareHouseCoin: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;		// World ID

	char cMoveType;
	bool bInOutFlag;			// 0: Deposit, 1: Withdraw
	INT64 nMoveCoin;			// Move amount
	INT64 biTotalCoin;		// Total amount

	__time64_t tUseDate;			
};

// QUERY_GET_GUILDWAREINFO // Request guild storage items
struct TQGetGuildWareInfo : public TQHeader
{	
	UINT nGuildDBID;	// Guild DBID
	short wMaxCount;	// Max warehouse count
};

struct TAGetGuildWareInfo
{
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;			// World ID
	int nErrNo;				// Error

	BYTE cItemListCount;					// Item list count
	TItemInfo ItemList[GUILD_WAREHOUSE_MAX];	// Wear item list
};

//QUERY_SET_GUILDWAREINFO // Save guild storage items
struct TQSetGuildWareInfo : public TQHeader
{
	UINT nGuildDBID;							// Guild DBID
	BYTE cSlotListCount;						// Slot count
	TGuildWareSlot WareSlotList[GUILD_WAREHOUSE_MAX];	// Slot list
};

struct TASetGuildWareInfo
{
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;			// World ID
	int	nRetCode;			
};

// QUERY_GET_GUILDWAREHISTORY // Request guild storage history
struct TQGetGuildWareHistory : public TQHeader
{
	INT64 nCharacterDBID;	// Character DBID
	UINT nGuildDBID;		// Guild DBID
	int	nIndex;			// Request index
};

struct TAGetGuildWareHistory
{	
	UINT nAccountDBID;		// Account DBID
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;			// World ID

	int	nErrNo;				

	int	nCurrCount;			// Current list count
	int	nTotalCount;		// Total list count
	int	nCount;
	TGuildWareHistory HistoryList[GUILD_WAREHOUSE_HISTORYLIST_MAX];	// History list
};

// QUERY_CHANGEGUILDNAME // Change guild name
struct TQChangeGuildName : public TQHeader
{
	UINT nGuildDBID;	
	INT64 biItemSerial;
	WCHAR wszGuildName[GUILDNAME_MAX];  // Guild name to change to	
};
struct TAChangeGuildName : public TAHeader
{
	char cWorldSetID;
	UINT nGuildDBID;
	INT64 biItemSerial;
	WCHAR wszGuildName[GUILDNAME_MAX];  // Changed guild name
};

// QUERY_MOD_GUILDEXP // Modify guild EXP
struct TQModGuildExp: public TQHeader
{
	UINT nGuildDBID;
	char cPointType;
	int nPointValue;
	int nMaxPoint;
	int nLevel;
	INT64 biCharacterDBID;
	int nMissionID;
#if defined(PRE_ADD_GUILD_CONTRIBUTION)
	bool IsGuildLevelMax;
#endif	// #if defined(PRE_ADD_GUILD_CONTRIBUTION)
};

struct TAModGuildExp: public TAHeader
{
	char cWorldSetID;
	UINT nGuildDBID;
	char cPointType;
	int nPointValue;
	int nTotalGuildExp;
	int nDailyPointValue;
	int nLevel;
	INT64 biCharacterDBID;
	int nMissionID;
};

// QUERY_MOD_GUILDCHEAT // Guild cheat
struct TQModGuildCheat: public TQHeader
{
	UINT nGuildDBID;
	char cCheatType;
	int nPoint;
	char cLevel;
};

struct TAModGuildCheat: public TAHeader
{
	char cWorldSetID;
	UINT nGuildDBID;
	char cCheatType;
	int nPoint;
	char cLevel;
};

struct TQGetGuildRewardItem: public TQHeader
{
	UINT nGuildDBID;	
};
struct TAGetGuildRewardItem: public TAHeader
{
	char cWorldSetID;
	UINT nGuildDBID;
	int nCount;
	TGuildRewardItem RewardItemInfo[GUILDREWARDEFFECT_TYPE_CNT]; 
};
struct TQAddGuildRewardItem: public TQHeader
{
	INT64 biCharacterDBID;
	UINT nGuildDBID;
	int nItemID;
	bool bEnternityFlag;
	int nLifeSpan;
	BYTE cItemBuyType;
	int nNeedGold;
	int nMpaID;
	int nRemoveItemID;
};
struct TAAddGuildRewardItem: public TAHeader
{
	INT64 biCharacterDBID;
	char cWorldSetID;
	UINT nGuildDBID;
	int nItemID;	
	BYTE cItemBuyType;
	__time64_t	m_tExpireDate;

};
struct TQChangeGuildSize: public TQHeader
{	
	UINT	nGuildDBID;
	short	nGuildSize;
};
struct TAChangeGuildSize: public TAHeader
{	
	char	cWorldSetID;
	UINT	nGuildDBID;
	short	nGuildSize;
};
struct TQDelGuildEffectItem:public TQHeader
{
	INT64		biCharacterDBID;	// Character code	
	UINT		nGuildDBID;			// Guild DBID
	int			nItemID;
};

// QUERY_ENROLL_GUILDWAR
struct TQEnrollGuildWar : public TQHeader
{
	short			wScheduleID;
	UINT	nGuildDBID;
};

struct TAEnrollGuildWar : public TAHeader
{
	char			cWorldSetID;
	UINT	nGuildDBID;
	short			wScheduleID;
	BYTE			cTeamColorCode;
};

// QUERY_GET_GUILDWARSCHEDULE
struct TQGetGuildWarSchedule : public TQHeader
{

};

struct TAGetGuildWarSchedule : public TAHeader
{	
	short wScheduleID;				// Guild War season
	short wWinersWeightRate;		// Winning team weight
	bool bFinalProgress;			// Whether finals are in progress
	TGuildWarEventInfo EventInfo[GUILDWAR_STEP_END];	// Period info	
	__time64_t tRewardExpireDate;	// Reward (farm, gesture) expiration period 
};

// QUERY_GET_GUILDWARFINALSCHEDULE // Get finals schedule
struct TQGetGuildWarFinalSchedule : public TQHeader
{
	short wScheduleID;				// Guild War season
};

struct TAGetGuildWarFinalSchedule : public TAHeader
{
	TGuildWarEventInfo GuildWarFinalSchedule[GUILDWAR_FINALPART_MAX];	
};

// QUERY_ADD_GUILDWARPOINT // Accumulate points on clearing prelim mission
struct TQAddGuildWarPointRecodrd : public TQHeader
{
	INT64 biCharacterDBID;			// Character DBID
	int nMissionID;					// Guild War mission ID
	int nOpeningPoint;				// Acquired mission points
	UINT nGuildDBID;		// Affiliated guild ID
	char cTeamColorCode;			// 1-Blue team, 2-Red team
};

struct TAAddGuildWarPointRecodrd : public TAHeader
{
	char cWorldSetID;
	char cTeamColorCode;			// 1-Blue team, 2-Red team
	UINT nGuildDBID;	 	// Affiliated guild ID
	int nOpeningPoint;				// Acquired mission points	
};
// QUERY_GET_GUILDWARPOINT // Query prelim points
struct TQGetGuildWarPoint : public TQHeader
{
	char cQueryType;				// 'C' = Query character qualifier points, 'G' = Query guild qualifier points, 'T' = Query team qualifier points
	INT64 biDBID;					// C-Character ID, G-Guild ID, T-0
};

struct TAGetGuildWarPoint  : public TAHeader
{
	char cQueryType;
	INT64 biDBID;
	int nCharOpeningPoint;
	int nGuildOpeningPoint;
	int nBlueTeamPoint;
	int nRedTeamPoint;
};

// QUERY_GET_GUILDWARPOINT_PARTTOTAL // Query 1st place by category (individual)
struct TQGetGuildWarPointPartTotal : public TQHeader
{
	char cQueryType;					// 'A' = All, 'C' = Individual
	INT64 biCharacterDBID;
};

struct SGuildWarPointPartTotal
{
	short wRanking;							// Rank
	INT64 biCharacterDBID;					// Character ID
	WCHAR wszCharName[NAMELENMAX];			// Character name
	UINT nGuildDBID;				// Guild ID
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	int nPoint;								// Score	
};

struct TAGetGuildWarPointPartTotal : public TAHeader
{
	char cQueryType;
	SGuildWarPointPartTotal GuildWarPointPartTotal[GUILDWAR_RANKINGTYPE_MAX]; // Aggregated rankings by category
};
// QUERY_GET_GUILDWARPOINT_GUILD_PARTTOTAL // Guild ranking by category
struct TQGetGuildWarPointGuildPartTotal : public TQHeader
{
	char cQueryType;						// 'A' = All, 'G' = Guild
	UINT nGuildDBID;				// Guild ID
};

struct TAGetGuildWarPointGuildPartTotal : public TAHeader
{
	char cQueryType;						// 'A' = All, 'G' = Guild
	SGuildWarPointPartTotal GuildWarPointPartTotal[GUILDWAR_RANKINGTYPE_MAX]; // Aggregated rankings by category
};
// QUERY_GET_GUILDWARPOINT_DAILY // 1st place for each day in prelims		
struct TQGetGuildWarPointDaily : public TQHeader
{

};

struct SGuildWarPointDaily
{
	int nTotalPoint;
	INT64 biCharacterDBID;					// Character ID
	WCHAR wszCharName[NAMELENMAX];			// Character name
	UINT nGuildDBID;				// Guild ID
};

struct TAGetGuildWarPointDaily : public TAHeader
{	
	SGuildWarPointDaily sGuildWarPointDaily[GUILDWAR_DAILY_AWARD_MAX];
};
// QUERY_GET_GUILDWARPOINT_GUILD_TOTAL // Aggregated point rankings per guild (can also query finals qualifiers here)
struct TQGetGuildWarPointGuildTotal : public TQHeader
{
	bool bMasterServer;				// Whether request is from master server..
	char cQueryType;				// 'A' = All, 'G' = Guild
	short wScheduleID;				// Guild War season	
	UINT nGuildDBID;		// Guild ID
};

struct SGuildWarPointGuildTotal
{
	short wRanking;							// Guild ranking
	UINT nGuildDBID;				// Guild ID
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	INT64 biMasterDBID;						// Guild master character ID
	WCHAR wszMasterName[NAMELENMAX];		// Guild master name
	short wGuildLevel;				// Guild level
	short wGuildMemberCount;				// Current guild member count
	int nOpeningPointSum;					// Sum preliminary scores
	int nMaxMemberCount;					// Max guild member capacity
};

struct TAGetGuildWarPointGuildTotal : public TAHeader
{
	bool bMasterServer;				// Whether request is from master server..
	char cQueryType;				// 'A' = All, 'G' = Guild
	int nTotalCount;
	SGuildWarPointGuildTotal sGuildWarPointGuildTotal[GUILDWAR_FINALS_TEAM_MAX];
};

//QUERY_GET_GUILDWARPOINT_RUNNING_TOTAL // Guild war prelim score standings (top 24)
struct TQGetGuildWarPointRunningTotal : public TQHeader
{
	//Fetching by GUILD isn't used for now, so skip..
};

struct TAGetGuildWarPointRunningTotal : public TAHeader
{	
	int nTotalCount;
	SGuildWarPointGuildTotal sGuildWarPointGuildTotal[GUILDWAR_TRIAL_POINT_TEAM_MAX];
};

//QUERY_GET_GUILDWARPOINT_FINAL_REWARDS
struct TAGetGuildWarPointFinalRewards : public TAHeader
{
	UINT nGuildDBID[GUILDWAR_FINALS_TEAM_MAX]; // GuildDBID to update
	int nGuildPoint[GUILDWAR_FINALS_TEAM_MAX]; // GuildPoint to update
};

// QUERY_ADD_GUILDWAR_FINAL_MATCHLIST // Save finals match bracket
struct TQAddGuildWarFinalMatchList : public TQHeader
{
	short wScheduleID;										// Guild War season
	UINT nGuildDBID[GUILDWAR_FINALS_TEAM_MAX];		// Guild ID by bracket order (may have empty slots in between)
};

struct TAAddGuildWarFinalMatchList : public TAHeader
{

};

// QUERY_ADD_GUILDWARPOPULARITYVOTE // Save popularity vote
struct TQAddGuildWarPopularityVote : public TQHeader
{
	INT64 biCharacterDBID;			// Character DBID
	UINT nGuildDBID;		// Voted guild ID	
};

struct TAAddGuildWarPopularityVote : public TAHeader
{
	
};
// QUERY_GET_GUILDWARPOPULARITYVOTE // Popularity vote results (also used to check if character voted)
struct TQGetGuildWarPopularityVote : public TQHeader
{
	char cQueryType;				// 'C' - Check character vote status, 'G' - Query guild
	INT64 biCharacterDBID;			// Character DBID	
};

struct TAGetGuildWarPopularityVote : public TAHeader
{
	char cQueryType;				// 'C' - Check character vote status, 'G' - Query guild
	UINT nGuildDBID;		// Voted or 1st-place guild ID	
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	int nPopularityVoteCount;			// Total vote count
};
// QUERY_ADD_GUILDWARFINALRESULTS // Save guild war finals results
struct TQAddGuildWarFinalResults : public TQHeader
{
	UINT nGuildDBID;		// Voted guild ID	
	short wScheduleID;				// Guild War season
	char cMatchSequence;			// Number on the bracket
	char cMatchTypeCode;			// 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	char cMatchResultCode;			// 1=Win, 2=Loss, 3=Forfeit, 4=Bye
};

struct TAAddGuildWarFinalResults : public TAHeader
{
	
};
// QUERY_GET_GUILDWARFINALRESULTS // Get guild war finals results
struct TQGetGuildWarFinalResults : public TQHeader
{	
	short wScheduleID;				// Guild War season
};

struct SGuildFinalResultInfo
{
	UINT nGuildDBID;				// Guild ID
	WCHAR wszGuildName[GUILDNAME_MAX];	// Guild name
	char cMatchSequence;			// Number on the bracket
	char cMatchTypeCode;			// 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
	char cMatchResultCode;			// 1=Win, 2=Loss, 3=Forfeit, 4=Bye
	short wGuildMark;				// Guild mark emblem
	short wGuildMarkBG;				// Guild mark background
	short wGuildMarkBorder;			// Guild mark border
};

struct TAGetGuildWarFinalResults : public TAHeader
{
	SGuildFinalResultInfo GuildFinalResultInfo[GUILDWAR_FINALS_TEAM_MAX]; // Finals result info...
};

// QUERY_GET_GUILDWAR_PRE_WIN_GUILD // Get previous guild war winning guild
struct TQGetGuildWarPreWinGuild : public TQHeader
{	

};

struct TAGetGuildWarPreWinGuild : public TAHeader
{
	UINT nGuildDBID;		// Guild ID
	short wScheduleID;				// Guild War season
};

//QUERY_ADD_GUILDWAR_REWARD_CHARACTER
struct TQAddGuildWarRewardCharacter : public TQHeader
{
	short wScheduleID;				// Guild War season
	INT64 biCharacterDBID;			// Character DBID	
	char cRewardType;				// Reward code (1=Preliminaries reward gift, 2=Preliminaries reward mail, 3=Preliminaries reward points)
};
struct TAAddGuildWarRewardCharacter : public TAHeader
{
	char cRewardType;				// Reward code (1=Preliminaries reward gift, 2=Preliminaries reward mail, 3=Preliminaries reward points)	
};

//QUERY_GET_GUILDWAR_REWARD_CHARACTER
struct TQGetGuildWarRewardCharacter : public TQHeader
{
	short wScheduleID;				// Guild War season
	INT64 biCharacterDBID;			// Character DBID	
};

struct TAGetGuildWarRewardCharacter : public TAHeader
{
	bool bRewardResults[GUILDWAR_REWARD_CHAR_MAX];  //Saved personal reward result
};
//QUERY_ADD_GUILDWAR_REWARD_GUILD // Save guild war guild reward results
struct TQAddGuildWarRewardGuild : public TQHeader
{
	short wScheduleID;				// Guild War season
	UINT nGuildDBID;				// Guild DBID	
	char cRewardType;				// Reward code (1=Finals reward skill, 2=Finals reward gift, 3=Finals reward mail, 4=Preliminaries reward points, 5=Finals reward points)
};
struct TAAddGuildWarRewardGuild : public TAHeader
{
	char cWorldSetID;
	UINT nGuildDBID;				// Guild DBID	
	char cRewardType;				// Reward code (1=Finals reward skill, 2=Finals reward gift, 3=Finals reward mail, 4=Preliminaries reward points, 5=Finals reward points)
};
//QUERY_GET_GUILDWAR_REWARD_GUILD // Get guild war guild reward results
struct TQGetGuildWarRewardGuild : public TQHeader
{
	char cQueryType;				// 'M' - Check master gesture grant status, 'V' - Village request
	short wScheduleID;				// Guild War season
	UINT nGuildDBID;				// Guild DBID
};

struct TAGetGuildWarRewardGuild : public TAHeader
{
	char cQueryType;				// 'M' - Check master gesture grant status, 'V' - Village request
	char cWorldSetID;
	UINT nGuildDBID;				// Guild DBID	
	bool bRewardResults[GUILDWAR_REWARD_GUILD_MAX];  //Saved guild reward result
};

//QUERY_ADD_GUILDWAR_POINT_QUEUE (grant festival points)
struct TQAddGuildWarPointQueue : public TQHeader
{
	UINT nGuildDBID;				// Guild ID	
	INT64 biAddPoint;	
	WCHAR wszIP[IPLENMAX];			// Just server IP?
};
//QUERY_ADD_GUILDWAR_SYSTEM_MAIL_QUEUE
//This uses TQSendSystemMail struct directly..
//QUERY_ADD_GUILDWAR_MAKE_GIFT_QUEUE
struct TQAddGuilWarMakeGiftQueue : public TQHeader
{		
	char cRewardType;
	INT64 biCharacterDBIDs[GUILDSIZE_MAX];	// Character DBIDs of targets to send to
	char cClass[GUILDSIZE_MAX];				// Class of targets to send to
};
//QUERY_ADD_GUILDWAR_MAKE_GIFT_NOTGUILDINFO
struct TQAddGuildWarMakeGiftNotGuildInfo : public TQHeader
{
	char cRewardType;
	UINT nGuildDBID;
};
// QUERY_ADD_JOB_RESERVE
struct TQAddJobReserve : public TQHeader
{
	char cJobType;
};
struct TAAddJobReserve : public TAHeader
{
	char cJobType;
	int nJobSeq;
};
// QUERY_GET_JOB_RESERVE
struct TQGetJobReserve : public TQHeader
{
	int nJobSeq;
};
struct TAGetJobReserve : public TAHeader
{
	int nJobSeq;
	char cJobStatus;
};

struct TQGetListGuildWarItemTradeRecord : public TQHeader
{
	INT64 characterDBID;
};

struct TAGetListGuildWarItemTradeRecord : public TAHeader
{
	int count;
	ItemCount itemCount[INVENTORYMAX + WAREHOUSEMAX];
};

struct TQAddGuildWarItemTradeRecord : public TQHeader
{
	INT64 characterDBID;
	ItemCount itemCount;
};

//QUERY_CHANGEGUILDWARESIZE
struct TQChangeGuildWareSize : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	short wStorageSize;		// Warehouse size
};

struct TAChangeGuildWareSize: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;		// World ID
	short wStorageSize;					// Warehouse size
};

//QUERY_CHANGEGUILDMARK
struct TQChangeGuildMark : public TQHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	short wGuildMark;			// Guild mark emblem
	short wGuildMarkBG;		// Guild mark background
	short wGuildMarkBorder;	// Guild mark border
};

struct TAChangeGuildMark: public TAHeader
{
	INT64 nCharacterDBID;		// Character DBID
	UINT nGuildDBID;			// Guild DBID
	char cWorldSetID;		// World ID

	short wGuildMark;		// Guild mark emblem
	short wGuildMarkBG;		// Guild mark background
	short wGuildMarkBorder;	// Guild mark border
};

#if defined(PRE_ADD_GUILD_CONTRIBUTION)
struct TQGetGuildContributionPoint: public TQHeader
{
	INT64 biCharacterDBID;
};
struct TAGetGuildContributionPoint: public TAHeader
{
	int nWeeklyGuildContributionPoint;	// Weekly guild contribution
	int nTotalGuildContributionPoint;	// Cumulative guild contribution points earned so far
};
struct TQGetGuildContributionRanking: public TQHeader
{
	UINT nGuildDBID;
};
struct TAGetGuildContributionRanking: public TAHeader
{
	int nCount;			// Guild member info count
	GuildContribution::TGuildContributionRankingData ContributionRankingList[GUILDSIZE_MAX];
};
#endif


//#########################################################################################################################################
//	MAINCMD_PVP
//#########################################################################################################################################

struct TQUpdatePvPData:public TQHeader
{
	INT64		biCharacterDBID;
	PvPCommon::QueryUpdatePvPDataType::eCode Type;
	TPvPGroup	PvP;
};

struct TAUpdatePvPData: public TAHeader
{
	PvPCommon::QueryUpdatePvPDataType::eCode Type;
};

struct TQUpdatePvPRankCriteria : public TQHeader
{
	UINT nThresholdPvPExp;
};

struct TQForceCalcPvPRank : public TQHeader
{
};

struct LadderKillResult
{
	BYTE	cJobCode;
	short	nKillCount;
	short	nDeathCount;
};

struct TQAddPvPLadderResult:public TQHeader
{
	PvPCommon::QueryUpdatePvPDataType::eCode Type;
	INT64				biCharacterDBID;	
	BYTE				cPvPLadderCode;	// LadderSystem::MatchType::eCode
	int					iPvPLadderGradePoint;
	int					iHiddenPvPLadderGradePoint;
	BYTE				cResult;		// LadderSystem::MatchResult::eCode
	BYTE				cVersusCharacterJobCode;
	BYTE				cKillDeathCount;
	LadderKillResult	KillDeathCounts[JOB_KIND_MAX];
};

struct TAAddPvPLadderResult:public TAHeader
{
	PvPCommon::QueryUpdatePvPDataType::eCode Type;
};

struct TQGetListPvPLadderScore:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetListPvPLadderScore:public TAHeader
{
	TPvPLadderScoreInfo	Data;
};

struct TQGetListPvPLadderScoreByJob:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetListPvPLadderScoreByJob:public TAHeader
{
	TPvPLadderScoreInfoByJob Data;
};

struct TQInitPvPLadderGradePoint:public TQHeader
{
	INT64	biCharacterDBID;
	BYTE	cPvPLadderCode;
	int		iPvPLadderPoint;		// Input increment value (negative allowed)
	int		iPvPLadderGradePoint;	// PvP ladder rating points to reset
	char	szIP[IPLENMAX];
};

struct TAInitPvPLadderGradePoint:public TAHeader
{

};

struct TQUsePvPLadderPoint:public TQHeader
{
	INT64	biCharacterDBID;
	int		iPvPLadderPoint;		// Used ladder points
	int		iMapID;
	char	szIP[IPLENMAX];
	int		iAfterLadderPoint;
};

struct TAUsePvPLadderPoint:public TAHeader
{
	int		iPvPLadderPointAfter;	// Remaining PvP ladder rating after use
};

struct TQAddPvPLadderCUCount:public TQHeader
{
	BYTE	cPvPLadderCode;
	int		iCUCount;
};

struct TQModPvPLadderScoresForCheat:public TQHeader
{
	INT64	biCharacterDBID;
	BYTE	cPvPLadderCode;
	int		iPvPLadderPoint;
	int		iPvPLadderGradePoint;
	int		iHiddenPvPLadderGradePoint;
};

struct TAModPvPLadderScoresForCheat:public TAHeader
{
	BYTE	cPvPLadderCode;
	int		iPvPLadderPoint;
	int		iPvPLadderGradePoint;
	int		iHiddenPvPLadderGradePoint;
};

struct TQGetListPvPLadderRanking:public TQHeader
{
	INT64	biCharacterDBID;
	BYTE	cPvPLadderCode;
};

struct TAGetListPvPLadderRanking:public TAHeader
{
	TPvPLadderRanking MyRanking;
	TPvPLadderRanking Top[LadderSystem::Common::RankingTop];
};

#if defined(PRE_ADD_PVP_RANKING)
//Normal Ranking
struct TQGetPvPRankBoard:public TQHeader
{
	INT64	biCharacterDBID;	
};

struct TAGetPvPRankBoard:public TAHeader
{
	TPvPRankingDetail MyRanking;
};

struct TQGetPvPRankList:public TQHeader
{
	int	iPage;
	BYTE cClassCode;
	BYTE cSubClassCode;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct TAGetPvPRankList:public TAHeader
{
	INT64 biRankingTotalCount;
	BYTE cRankingCount;
	TPvPRanking RankingInfo[RankingSystem::RANKINGMAX];
};

struct TQGetPvPRankInfo:public TQHeader
{
	WCHAR	wszCharName[NAMELENMAX];	
};

struct TAGetPvPRankInfo:public TAHeader
{
	TPvPRankingDetail RankingInfo;
};

//Ladder Ranking
struct TQGetPvPLadderRankBoard:public TQHeader
{
	INT64	biCharacterDBID;
	BYTE	cPvPLadderCode;
};

struct TAGetPvPLadderRankBoard:public TAHeader
{
	BYTE cPvPLadderCode;
	TPvPLadderRankingDetail MyRanking;
};

struct TQGetPvPLadderRankList:public TQHeader
{
	int	iPage;
	BYTE cClassCode;
	BYTE cSubClassCode;
	BYTE cPvPLadderCode;
	WCHAR wszGuildName[GUILDNAME_MAX];
};

struct TAGetPvPLadderRankList:public TAHeader
{
	INT64 biRankingTotalCount;
	BYTE cRankingCount;
	BYTE cPvPLadderCode;
	TPvPLadderRanking2 RankingInfo[RankingSystem::RANKINGMAX];
};

struct TQGetPvPLadderRankInfo:public TQHeader
{
	WCHAR	wszCharName[NAMELENMAX];
	BYTE	cPvPLadderCode;
};

struct TAGetPvPLadderRankInfo:public TAHeader
{
	BYTE cPvPLadderCode;
	TPvPLadderRankingDetail RankingInfo;
};

struct TQForceAggregatePvPRank:public TQHeader
{
	bool bIsPvPLadder;
};

#endif

struct TQAddPVPGhoulScores : public TQHeader
{
	INT64	biCharacterDBID;
	int		nGhoulWin;
	int		nHumanWin;
	int		nTimeOver;
	int		nGhoulKill;
	int		nHumanKill;
	int		nHolyWaterUse;
};

struct TQGetPVPGhoulScores : public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetPVPGhoulScores : public TAHeader
{
	int		nPlayCount;
	int		nGhoulWin;
	int		nHumanWin;
	int		nTimeOver;
	int		nGhoulKill;
	int		nHumanKill;
	int		nHolyWaterUse;
};
#if defined(PRE_ADD_PVP_TOURNAMENT)
struct TQAddPVPTournamentResult : public TQHeader
{
	int nTotalCount;
#if defined( PRE_PVP_GAMBLEROOM )
	INT64 nGambleDBID;
#endif
	INT64 biCharacterDBID[PvPCommon::Common::PvPTournamentUserMax];
	char cTournamentStep[PvPCommon::Common::PvPTournamentUserMax];
	bool bWin[PvPCommon::Common::PvPTournamentUserMax];
};
#endif

#if defined( PRE_PVP_GAMBLEROOM )
struct TQAddGambleRoom: public TQHeader
{
	int nRoomID;
	BYTE cGambleType;
	int nGamblePrice;
};

struct TAAddGambleRoom: public TAHeader
{
	int nRetCode;
	char cWorldSetID;
	int nRoomID;
	INT64 nGambleDBID;
};

struct TQAddGambleRoomMember: public TQHeader
{
	INT64 nGambleDBID;
	INT64 biCharacterDBID;
};

struct TQEndGambleRoom: public TQHeader
{
	INT64 nGambleDBID;
	TPvPGambleRoomWinData Member[PvPGambleRoom::Max];	
};
#endif

//#########################################################################################################################################
//	MAINCMD_DARKLAIR
//#########################################################################################################################################

struct SDarkLairUserData
{
	USHORT					unLevel;									// Level
	int						iJobIndex;									// Job Index
	WCHAR					wszCharacterName[NAMELENMAX];				// Character name (NULL allowed)

	SDarkLairUserData()
	{
		unLevel				= 0;
		iJobIndex			= 0;
		memset( wszCharacterName, 0, sizeof(wszCharacterName) );
	}

	void Set( USHORT _unLevel, int _iJob, WCHAR* _pwszName )
	{
		unLevel				= _unLevel;
		iJobIndex			= _iJob;
		if( _pwszName )
			_wcscpy( wszCharacterName, _countof(wszCharacterName), _pwszName, (int)wcslen(_pwszName) );
	}
};

struct SDarkLairUpdateUserData : public SDarkLairUserData
{
	bool					bUpdate;									// Flag whether to play to the end and update personal record
	INT64					i64CharacterDBID;							// CharDBID

	SDarkLairUpdateUserData()
	{
		bUpdate				= false;
		i64CharacterDBID	= 0;
	}
};

struct SDarkLairHistory
{
	USHORT unPlayRound;								// Number of played Rounds
	UINT uiPlaySec;									// Played time (seconds)
	WCHAR wszPartyName[PARTYNAMELENMAX];			// Party name (including NULL)
	BYTE cPartyUserCount;
	SDarkLairUserData sUserData[DarkLair::Common::MaxPlayer];

	SDarkLairHistory()
	{
		unPlayRound		= 0;
		uiPlaySec		= 0;
		cPartyUserCount = 0;
		memset( wszPartyName, 0, sizeof(wszPartyName) );
	}

	void Set( USHORT _unPlayRound, UINT _uiPlaySec, WCHAR* _pwszPartyName )
	{
		unPlayRound = _unPlayRound;
		uiPlaySec	= _uiPlaySec;
		if( _pwszPartyName )
			_wcscpy( wszPartyName, _countof(wszPartyName), _pwszPartyName, (int)wcslen(_pwszPartyName) );
	}

	void AddUserData( USHORT _unLevel, int _iJob, WCHAR* _pwszName )
	{
		if( cPartyUserCount >= DarkLair::Common::MaxPlayer )
			return;

		sUserData[cPartyUserCount++].Set( _unLevel, _iJob, _pwszName );
	}
};

struct SDarkLairBestHistory : public SDarkLairHistory
{
	INT64	i64CharacterDBID;
	int		iRank;

	SDarkLairBestHistory()
	{
		i64CharacterDBID	= 0;
		iRank				= 0;
	}
};

struct TQUpdateDarkLairResult:public TQHeader
{
	int	iRoomID;									// Game server RoomID
	int	iProtectionKey;								// Protection key
	int	iMapIndex;									// Played MapIndex
	USHORT unPlayRound;								// Number of played Rounds
	UINT uiPlaySec;									// Play time (seconds)
	WCHAR wszPartyName[PARTYNAMELENMAX];			// Party name (including NULL)
	BYTE cPartyUserCount;
	SDarkLairUpdateUserData	sUserData[DarkLair::Common::MaxPlayer];
};

struct TAUpdateDarkLairResult:public TBoostMemoryPool<TAUpdateDarkLairResult>
{
	short					nRetCode;
	int						iRoomID;									// Game server RoomID
	int						iProtectionKey;								// Protection key
	int						iRank;										// Currently played Dark Lair ranking
	SDarkLairHistory		sHistoryTop[DarkLair::Rank::SelectTop];		// Top ranking
	BYTE					cPartyUserCount;
	SDarkLairBestHistory	sBestUserData[DarkLair::Common::MaxPlayer];	// Personal best
};

struct TQGetDarkLairRankBoard:public TQHeader
{
	INT64	biCharacterDBID;
	int		iMapIndex;
	BYTE	cPartyUserCount;
};

struct TAGetDarkLairRankBoard:public TAHeader
{
	SDarkLairBestHistory sBestHistory;
	BYTE cPartyUserCount;
#if defined(PRE_ADD_CHALLENGE_DARKLAIR)
	int iMapIndex;
#endif
	SDarkLairHistory sHistoryTop[DarkLair::Rank::SelectRankBoardTop];
};

//#########################################################################################################################################
//	MAINCMD_AUTH
//#########################################################################################################################################

struct TABeginAuth: public TAHeader
{
	UINT nSessionID;						// Session ID
	char cCertifyingStep;					// Auth step
	char cWorldSetID;							// World ID (*)
};

struct TQStoreAuth:public TQHeader						// Used by GA, VI
{
	INT64 biCertifyingKey;					// Auth key
	int nCurServerID;						// Current server ID
	char cWorldSetID;							// World ID (*)
	UINT nSessionID;						// Session ID
	INT64 biCharacterDBID;					// Character DBID
	WCHAR wszAccountName[IDLENMAX];			// Account name
	WCHAR wszCharacterName[NAMELENMAX];		// Character name
	BYTE bIsAdult;							// Whether adult
	BYTE nAge;								// Age
	BYTE nDailyCreateCount;					// Daily character creation limit
	int nPrmInt1;							// Parameter 1 (multi-purpose by region)
	char cLastServerType;					// Recent server type (eServerType / 0 : Login, 2 : Village, 3 : Game)
};

struct TAStoreAuth: public TAHeader			// Used by GA, VI
{
	UINT nSessionID;						// Session ID
};

struct TQCheckAuth:public TQHeader						// Used by GA, VI
{
	INT64 biCertifyingKey;					// Auth key
	int nCurServerID;						// Current server ID
	int nSessionID;
};

struct TACheckAuth: public TAHeader						// Used by GA, VI
{
	char cWorldSetID;							// World ID (*)
	UINT nSessionID;						// Session ID
	INT64 biCertifyingKey;					// Auth key
	INT64 biCharacterDBID;					// Character DBID
	WCHAR wszAccountName[IDLENMAX];			// Account name
	WCHAR wszCharacterName[NAMELENMAX];		// Character name
	char cAccountLevel;						// Account level for GM, developer, etc.
	BYTE bIsAdult;							// Whether adult
	char nAge;								// Age
	BYTE nDailyCreateCount;					// Daily character creation limit
	int nPrmInt1;							// Parameter 1 (multi-purpose by region)
	char cLastServerType;					// Recent server type (eServerType / 0 : Login, 2 : Village, 3 : Game)
};

struct TQResetAuth:public TQHeader						// Used by GA, VI
{
	UINT nSessionID;						// Session ID
};

struct TAResetAuth: public TAHeader						// Used by GA, VI (unused ???)
{
	UINT nSessionID;						// Session ID
};

struct TQResetAuthServer:public TQHeader
{
	int nServerID;							// Server unique ID
};

struct TAResetAuthServer
{
	int nServerID;							// Server unique ID
	int nResult;							// Query result
};

struct TQGetAuthWorldUserCnt:public TQHeader		//QUERY_AUTHUSERCOUNT
{
	int nServerID;
};

struct TAuthWorldUserCnt
{
	int nWorldSetID;
	UINT nWorldUserCnt;
};

struct TAGetAuthWorldUserCnt
{
	int nServerID;
	short nResult;
	BYTE cCount;
	TAuthWorldUserCnt User[WORLDCOUNTMAX];
};

struct TQSetWorldIDAuth:public TQHeader		//QUERY_SETWOLRDID
{
};

//#########################################################################################################################################
//	MAINCMD_CASH
//#########################################################################################################################################

// QUERY_GETLISTGIVEFAILITEM, // Get list of failed cash items (received together on select)
struct TAGetListGiveFailItem: public TAHeader
{
	int nCount;
	DBPacket::TCashFailItem CashFailItem[CASHINVENTORYDBMAX];
};

// QUERY_MODGIVEFAILFLAG
struct TQModGiveFailFlag: public TQHeader
{
	INT64 biCharacterDBID;
	int nMapID;
	int nChannelID;
	WCHAR wszIP[IPLENMAX];
	int nCount;
	DBPacket::TModCashFailItemEx CashItem[CASHINVENTORYDBMAX];
};

struct TAModGiveFailFlag: public TAHeader
{
	int nCount;
	TCashItemBase CashItem[CASHINVENTORYDBMAX];
};

// QUERY_GETREMAINPETAL
struct TQGetRemainPetal: public TQHeader
{
};

struct TAGetRemainPetal: public TAHeader
{
	int nPetal;
};

// QUERY_PURCHASEITEMBYFAKEPETAL
struct TQPurchaseItemByFakePetal:public TQHeader
{
	int nChannelID;
	int nMapID;
	char cCartType;
	char cCount;
	DBPacket::TPetalInfo PetalInfo[CARTLISTMAX];
};

struct TAPurchaseItemByFakePetal: public TAHeader
{
	char cCartType;
	char cCount;
	DBPacket::TPetalInfoEx PetalInfo[CARTLISTMAX];
};

// QUERY_CHECKGIFTRECEIVER
struct TQCheckGiftReceiver:public TQHeader
{
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TACheckGiftReceiver: public TAHeader
{
	BYTE cLevel;
	BYTE cJob;
};

// QUERY_GETLISTGIFTBOX
struct TQGetListGiftBox: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListGiftBox: public TAHeader
{
	char cCount;
	TGiftInfo GiftBox[GIFTBOXLISTMAX];
};

// QUERY_MODGIFTRECEIVEFLAG
struct TQModGiftReceiveFlag: public TQHeader
{
	INT64 biCharacterDBID;
#if defined(PRE_RECEIVEGIFTALL)
	bool bReceiveAll;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
	char cCount;
	TReceiveGiftData GiftData[GIFTPAGEMAX];
};

struct TAModGiftReceiveFlag: public TAHeader
{
#if defined(PRE_RECEIVEGIFTALL)
	bool bReceiveAll;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
	char cCount;
	DBPacket::TReceiveGift ReceiveGift[GIFTPAGEMAX];
};

// QUERY_RECEIVEGIFT, // Receive gift
struct TGiftItem
{
	int nItemSN;
	TItem AddItem;
	DWORD dwPartsColor1;		// Parts color
	DWORD dwPartsColor2;
	TVehicleItem VehiclePart1;
	TVehicleItem VehiclePart2;
};

struct TQReceiveGift: public TQHeader
{
#if defined(PRE_RECEIVEGIFTALL)
	bool bReceiveAll;
	char cTotalCount;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
	TReceiveGiftData GiftData;

	int nChannelID;
	int nMapID;
	WCHAR wszIP[IPLENMAX];
	int nItemSN;
	char cCount;
	TGiftItem GiftItem[PACKAGEITEMMAX];
};

struct TAReceiveGift: public TAHeader
{
	INT64 biPurchaseOrderID;
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverTotalMailCount;
	int	nReceiverNotReadMailCount;
	int	nReceiver7DaysLeftMailCount;
#if defined(PRE_ADD_VIP)
	char cPayMethodCode;
#endif	// #if defined(PRE_ADD_VIP)
#if defined(PRE_RECEIVEGIFTALL)
	bool bReceiveAll;
	char cTotalCount;
#endif	// #if defined(PRE_RECEIVEGIFTALL)
	int nItemSN;
	char cCount;
	TGiftItem GiftItem[PACKAGEITEMMAX];
};

// QUERY_GIFTRETURN
#if defined(PRE_ADD_GIFT_RETURN)
struct TQGiftReturn: public TQHeader
{
	INT64 biPurchaseOrderID;
};
struct TAGiftReturn : public TAHeader
{
	INT64 biPurchaseOrderID;
	UINT nReceiverAccountDBID;
	INT64 biReceiverCharacterDBID;
	int nReceiverTotalMailCount;
};
#endif //#if defined(PRE_ADD_GIFT_RETURN)

#if defined(PRE_ADD_CASH_REFUND)
struct TQPaymentItemList : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAPaymentItemList : public TAHeader
{
	int nPaymentCount;
	TPaymentItemInfoEx PaymentItemList[CASHINVENTORYMAX]; // Needs to be made infinite.. 150 for now
};

struct TAPaymentPackageItemList : public TAHeader
{
	int nPaymentPackegeCount;
	TPaymentPackageItemInfoEx PaymentPackageItemList[CASHINVENTORYMAX];
};
#endif

// QUERY_NOTIFYGIFT, // Gift notification
struct TQNotifyGift: public TQHeader
{
	INT64 biCharacterDBID;
	bool bNew;
};

struct TANotifyGift: public TAHeader
{
	bool bNew;
	int nGiftCount;
};

struct TQAddEffectItems:public TQHeader
{
	INT64	biCharacterDBID;
	char	cItemLifeSpanRenewal;
	int		iItemID;
	DBDNWorldDef::EffectItemGetCode::eCode Code;
	INT64	biFKey;
	int		iPrice;
	bool	bEternityFlag;
	int		iItemLifeSpan;
	int		iProperty[5];
	INT64	biItemSerial;
	int		iRemoveItemID;
};

// QUERY_MAKEGIFTBYQUEST, // Quest completion reward
struct TQMakeGiftByQuest: public TQHeader
{
	INT64 biCharacterDBID;
	bool bPCBang;
	int nQuestID;
	WCHAR wszMemo[GIFTMESSAGEMAX];
	int nLifeSpan;
	char szIp[IPLENMAX];
	bool bNewFlag;
	char cRewardCount;
	int nRewardItemSN[MAILATTACHITEMMAX];
};

struct TAMakeGiftByQuest: public TAHeader
{
	int nGiftCount;
};

// QUERY_MAKEGIFTBYMISSION, // Mission completion reward
struct TQMakeGiftByMission: public TQHeader
{
	INT64 biCharacterDBID;
	bool bPCBang;
	int nMissionID;
	WCHAR wszMemo[GIFTMESSAGEMAX];
	int nLifeSpan;
	char szIp[IPLENMAX];
	bool bNewFlag;
	char cRewardCount;
	int nRewardItemSN[MISSIONREWARDITEMMAX];
};

struct TAMakeGiftByMission: public TAHeader
{
	int nGiftCount;
};

// QUERY_MAKEGIFT
struct TQMakeGift: public TQHeader
{
	INT64 biCharacterDBID;
	bool bPCBang;
	WCHAR wszMemo[GIFTMESSAGEMAX];
	int nLifeSpan;
	char cPayMethodCode;	// Determine FKey value according to PayMethodCode
	int iOrderKey;
	char szIp[IPLENMAX];
	bool bNewFlag;
	char cRewardCount;
	int nRewardItemSN[5];
};

struct TAMakeGift: public TAHeader
{
	int nGiftCount;
};

// QUERY_INCREASEVIPPOINT, // Add base VIP points
struct TQIncreaseVIPPoint: public TQHeader
{
	INT64 biCharacterDBID;
	int nBasicPoint;			// Base points from purchasing VIP subscription product
	INT64 biPurchaseOrderID;	// Item purchase order ID
	short wVIPPeriod;			// VIP duration. Unit: Day. 30 days, 90 days, 180 days, 360 days
	bool bAutoPay;				// 0=Manual billing, 1=Auto billing
};

struct TAIncreaseVIPPoint: public TAHeader
{
	int nVIPTotalPoint;			// Total VIP points
	__time64_t tVIPEndDate;		// VIP period end date
};

// QUERY_GETVIPPOINT, // Query VIP duration and points
struct TQGetVIPPoint: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetVIPPoint: public TAHeader
{
	int nVIPTotalPoint;			// Total VIP points
	__time64_t tVIPEndDate;		// VIP period end date
	bool bAutoPay;				// 0=Manual billing, 1=Auto billing
};

// QUERY_MODVIPAUTOPAYFLAG, // Change VIP auto-payment status
struct TQModVIPAutoPayFlag: public TQHeader
{
	INT64 biCharacterDBID;
	bool bAutoPay;			// 0=Manual billing, 1=Auto billing
};

struct TAModVIPAutoPayFlag: public TAHeader
{
	bool bAutoPay;			// 0=Manual billing, 1=Auto billing
};

// QUERY_GIFTBYCHEAT
struct TQGiftByCheat: public TQHeader
{
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
	int nItemSN;
	char szIp[IPLENMAX];
};

// QUERY_PETAL
struct TQPetal: public TQHeader
{
	INT64 biCharacterDBID;
	int nUseItemID;
	int nAddPetal;
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
	int	iMapID;
	WCHAR wszIP[IPLENMAX];
};

struct TAPetal: public TAHeader
{
	int nUseItemID;
	int nTotalPetal;
	char cInvenType;			// eItemPositionType
	BYTE cInvenIndex;			// Only pass index when using normal inventory (cash inventory is not referenced)
	INT64 biInvenSerial;
};

// QUERY_CHEAT_GIFTCLEAR
struct TQCheatGiftClear : public TQHeader
{
	INT64 biCharacterDBID;
};


#if defined( PRE_PVP_GAMBLEROOM )
struct TQAddPetalLogCode: public TQHeader
{
	INT64 biCharacterDBID;
	int nUseItemID;
	int nAddPetal;
	BYTE cLogCode;
	INT64 nGambleDBID;
};

struct TQUsePetal: public TQHeader
{
	INT64 biCharacterDBID;	
	int nDelPetal;
	BYTE cLogCode;
	INT64 nGambleDBID;
};
#endif

//#########################################################################################################################################
//	MAINCMD_MSGADJUST
//#########################################################################################################################################

struct TQMsgAdjust :public TQHeader
{
	INT64 biCharacterDBID;
	int nChannelID;
	int nVerifyMapIndex;
	char cGateNo;
};

struct TAMsgAdjust
{
	UINT nAccountDBID;
	INT64 biCharacterDBID;
	int nChannelID;
	int nVerifyMapIndex;
	char cGateNo;
};

#ifdef PRE_FIX_63822
struct TQMsgAdjustSeq :public TQHeader
{
};

struct TAMsgAdjustSeq :public TAHeader
{
};
#endif		//#ifdef PRE_FIX_63822

//#########################################################################################################################################
//	MAINCMD_LOG
//#########################################################################################################################################

struct TQAddPartyStartLog:public TQHeader
{
	INT64 biPartyLogID;
};

struct TQAddPartyEndLog:public TQHeader
{
	INT64 biPartyLogID;
};

struct TQAddStageStartLog:public TQHeader
{
	INT64	biRoomLogID;
	INT64	biPartyLogID;
	BYTE	cPartySize;
	int		iMapID;
	DBDNWorldDef::DifficultyCode::eCode Code;
};

struct TQAddStageClearLog:public TQHeader
{
	INT64	biRoomLogID;
	INT64	biCharacterDBID;
	int		iJobID;
	BYTE	cCharacterLevel;
	bool	bClearFlag;
	DBDNWorldDef::ClearGradeCode::eCode Code;
	int		iMaxComboCount;
	BYTE	cBossKillCount;
	int		iRewardCharacterExp;
	int		iBonusCharacterExp;
	int		iClearPoint;
	int		iBonusClearPoint;
	int		iClearTimeSec;
#if defined( PRE_ADD_36870 )
	int		iClearRound;
#endif // #if defined( PRE_ADD_36870 )
};

struct TQAddStageRewardLog:public TQHeader
{
	INT64	biRoomLogID;
	INT64	biCharacterDBID;
	DBDNWorldDef::RewardBoxCode::eCode Code;
	INT64	biRewardItemSerial;
	int		iRewardItemID;
	int		iRewardRet;
};

struct TQAddStageEndLog:public TQHeader
{
	INT64	biRoomLogID;
	DBDNWorldDef::WhereToGoCode::eCode Code;
};

struct TQAddPvPStartLog:public TQHeader
{
	INT64	biRoomID1;
	int		iRoomID2;
	BYTE	cMaxPlayerCount;
	DBDNWorldDef::PvPModeCode::eCode Code;
	int		iPvPModeDetailValue;
	bool	bBreakIntoFlag;
	bool	bItemUseFlag;
	bool	bSecretFlag;
	int		iMapID;
	INT64	biRoomMasterCharacterDBID;
	INT64	biCreateRoomCharacterDBID;
};

struct TQAddPvPResultLog:public TQHeader
{
	INT64	biRoomID1;
	int		iRoomID2;
	INT64	biCharacterDBID;
	DBDNWorldDef::PvPTeamCode::eCode TeamCode;
	bool	bBreakIntoFlag;
	DBDNWorldDef::PvPResultCode::eCode ResultCode;
	int		iPlayTimeSec;
	int		iPvPKillPoint;
	int		iPvPAssistPoint;
	int		iPvPSupportPoint;
	int		iVsKOWin[PvPCommon::Common::MaxClass];
	int		iVsKOLose[PvPCommon::Common::MaxClass];
	int nOccupationCount;
	BYTE cOccupationWinType;
};

struct TQAddPvPEndLog:public TQHeader
{
	INT64	biRoomID1;
	int		iRoomID2;
};

struct TQAddNestGateStartLog:public TQHeader
{
	INT64	biRoomID;
	BYTE	cPartySize;
	int		nGateID;
};

struct TQAddNestGateEndLog:public TQHeader
{
	INT64	biRoomID;
	BYTE	cPartySize;
};

struct TQAddNestGateClearLog:public TQHeader
{
	INT64	biRoomID;
	bool	bClearFlag;
	INT64 biChracterDBID;
	BYTE cCharacterJob;
	BYTE cCharacterLevel;
};

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TQModNPCLocation:public TQHeader
{
	int		iNPCID;
	int		iMapID;
};

struct TQModNPCFavor:public TQHeader
{
	INT64				biCharacterDBID;
	BYTE				cCount;
	TNpcReputation		UpdateArr[NpcReputation::Common::MaxEffectNpcCount];
};

struct TQGetListNpcFavor:public TQHeader
{
	INT64				biCharacterDBID;
	int					iMapID;
};

struct TAGetListNpcFavor:public TAHeader
{
	bool				bLastPage;
	BYTE				cCount;
	TNpcReputation		ReputationArr[30];
};

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

struct TQGetMasterPupilInfo:public TQHeader
{
	INT64							biCharacterDBID;
	bool							bClientSend;
	MasterSystem::EventType::eCode	EventCode;
};

struct TAGetMasterPupilInfo:public TAHeader
{
	bool							bClientSend;
	MasterSystem::EventType::eCode	EventCode;
	TMasterSystemSimpleInfo			SimpleInfo;
};

struct TQAddMasterCharacter:public TQHeader
{
	INT64	biCharacterDBID;
	WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
};

struct TAAddMasterCharacter:public TAHeader
{
	INT64	biCharacterDBID;
	WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
};

struct TQDelMasterCharacter:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TADelMasterCharacter:public TAHeader
{

};

struct TQGetPageMasterCharacter:public TQHeader
{
	UINT	uiPage;
	BYTE	cJobCode;		// 0 : All
	BYTE	cGenderCode;	// 0 : All, 1 : Male, 2 : Female
};

struct TAGetPageMasterCharacter:public TAHeader
{
	UINT		uiPage;
	BYTE		cJobCode;
	BYTE		cGenderCode;
	BYTE		cCount;
	TMasterInfo	MasterInfoList[MasterSystem::Max::MasterPageRowCount];
};

struct TQGetListPupil:public TQHeader
{
	INT64		biCharacterDBID;
};

struct TAGetListPupil:public TAHeader
{
	INT64		biCharacterDBID;
	BYTE		cCount;
	TPupilInfo	PupilInfoList[MasterSystem::Max::PupilCount];
};

struct TQAddMasterAndPupil:public TQHeader
{
	INT64								biMasterCharacterDBID;
	INT64								biPupilCharacterDBID;
	bool								bCheckRegisterFlag;
	bool								bIsDirectMenu;
	DBDNWorldDef::TransactorCode::eCode Code;
	bool								bIsSkip;
};

struct TAAddMasterAndPupil:public TAHeader
{
	INT64								biMasterCharacterDBID;
	INT64								biPupilCharacterDBID;
	bool								bIsDirectMenu;
	DBDNWorldDef::TransactorCode::eCode Code;
	UINT								uiOppositeAccountDBID;
};

struct TQDelMasterAndPupil:public TQHeader
{
	INT64								biMasterCharacterDBID;
	INT64								biPupilCharacterDBID;
	DBDNWorldDef::TransactorCode::eCode	Code;
	int									iPenaltyDay;
	int									iPenaltyRespectPoint;
	bool								bIsSkip;
};

struct TADelMasterAndPupil:public TAHeader
{
	INT64								biMasterCharacterDBID;
	INT64								biPupilCharacterDBID;
	DBDNWorldDef::TransactorCode::eCode	Code;
	int									iPenaltyRespectPoint;
};

struct TQGetMasterCharacter:public TQHeader
{
	DBDNWorldDef::GetMasterCharacterCode::eCode Code;
	INT64										biMasterCharacterDBID;
	INT64										biPupilCharacterDBID;
};

struct TAGetMasterCharacterType1:public TAHeader
{
	INT64					biMasterCharacterDBID;
	TMasterCharacterInfo	MasterCharacterInfo;
};

struct TAGetMasterCharacterType2:public TAHeader
{
	INT64			biPupilCharacterDBID;
	INT64			biMasterCharacterDBID;
	TMyMasterInfo	MasterInfo;
};

struct TAGetMasterCharacterType3:public TAHeader
{
	INT64	biCharacterDBID;
	WCHAR	wszCharName[NAMELENMAX];
	WCHAR	wszSelfIntroduction[MasterSystem::Max::SelfIntrotuctionLen];
	BYTE	cGenderCode;	// 0 : All, 1 : Male, 2 : Female
	BYTE	cLevel;
	BYTE	cJob;
	int		iGraduateCount;
	int		iRespectPoint;
};

struct TQGetListMyMasterAndClassmate:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetListMyMasterAndClassmate:public TAHeader
{
	INT64					biCharacterDBID;
	BYTE					cCount;
	TMasterAndClassmateInfo	MasterAndClassmateInfoList[MasterSystem::Max::MasterAndClassmateCount];
};

struct TQGetMyClassmate:public TQHeader
{
	INT64	biCharacterDBID;
	INT64	biClassmateCharacterDBID;
};

struct TAGetMyClassmate:public TAHeader
{
	TClassmateInfo ClassmateInfo;
};

struct TQGraduate:public TQHeader
{
	INT64	biCharacterDBID;
	WCHAR	wszPupilCharName[NAMELENMAX];
};

struct TAGraduate:public TAHeader
{
	char	cWorldSetID;
	WCHAR	wszPupilCharName[NAMELENMAX];
	BYTE	cCount;
	INT64	biMasterCharacterDBIDList[MasterSystem::Max::MasterCount];
};

struct TQModRespectPoint:public TQHeader
{
	INT64	biCharacterDBID;
	int		iRespectPoint;
};

struct TAModRespectPoint:public TAHeader
{
	int		iRespectPoint;
};

struct TQModMasterFavorPoint:public TQHeader
{
	INT64	biMasterCharacterDBID;
	INT64	biPupilCharacterDBID;
	int		iFavorPoint;
};

struct TAModMasterFavorPoint:public TAHeader
{
	INT64	biMasterCharacterDBID;
	INT64	biPupilCharacterDBID;
	int		iFavorPoint;
};

struct TQGetMasterAndFavorPoint:public TQHeader
{
	WCHAR	wszPupilCharName[NAMELENMAX];
	INT64	biPupilCharacterDBID;
	BYTE	cLevel;
};

struct TAGetMasterAndFavorPoint:public TAHeader
{
	WCHAR					wszPupilCharName[NAMELENMAX];
	INT64					biPupilCharacterDBID;
	BYTE					cLevel;
	BYTE					cWorldSetID;
	BYTE					cCount;
	TMasterAndFavorPoint	MasterAndFavorPoint[MasterSystem::Max::MasterCount];
};

struct TQGetMasterSystemCountInfo:public TQHeader
{
	bool	bClientSend;
	INT64	biCharacterDBID;
	BYTE	cCount;
	INT64	biPartyCharacterDBID[PARTYCOUNTMAX-1];
};

struct TAGetMasterSystemCountInfo:public TAHeader
{
	INT64	biCharacterDBID;
	bool	bClientSend;
	int		iMasterCount;
	int		iPupilCount;
	int		iClassmateCount;
};

struct TQModMasterSystemGraduateCount:public TQHeader
{
	INT64	biCharacterDBID;
	int		iGraduateCount;
};

#if defined( PRE_ADD_SECONDARY_SKILL )

struct TQAddSecondarySkill:public TQHeader
{
	INT64									biCharacterDBID;
	int										iSecondarySkillID;
	SecondarySkill::Type::eType				Type;
};

struct TQDelSecondarySkill:public TQHeader
{
	INT64	biCharacterDBID;
	int		iSecondarySkillID;
};

struct TQGetListSecondarySkill:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetListSecondarySkill:public TAHeader
{
	BYTE			cCount;
	TSecondarySkill	SkillList[SecondarySkill::Max::LearnSecondarySkill];
};

struct TQModSecondarySkillExp:public TQHeader
{
	INT64	biCharacterDBID;
	int		iSecondarySkillID;
	int		iSecondarySkillExp;			// Delta value
	int		iSecondarySkillExpAfter;	// EXP after change (comparison value)
};

struct TQSetManufactureSkillRecipe:public TQHeader
{
	INT64								biCharacterDBID;
	DBDNWorldDef::ItemLocation::eCode	LocationCode;
	INT64								biItemSerial;
	int									iSecondarySkillID;
};

struct TQExtractManufactureSkillRecipe:public TQHeader
{
	INT64								biCharacterDBID;
	INT64								biItemSerial;
	DBDNWorldDef::ItemLocation::eCode	LocationCode;
};

struct TQDelManufactureSkillRecipe:public TQHeader
{
	INT64	biCharacterDBID;
	INT64	biItemSerial;
	char	szIP[IPLENMAX];
};

struct TQGetListManufactureSkillRecipe:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetListManufactureSkillRecipe:public TAHeader
{
	BYTE					cCount;
	TSecondarySkillRecipe	RecipeList[SecondarySkill::Max::LearnManufactureRecipe];
};

struct TQModManufactureSkillRecipe:public TQHeader
{
	INT64	biCharacterDBID;
	INT64	biItemSerial;
	USHORT	nDurability;			// Delta value
	USHORT	nDurabilityAfter;		// Durability after change (comparison value)
};

#endif // #if defined( PRE_ADD_SECONDARY_SKILL )

struct TQGetListFarm:public TQHeader
{
	char		cEnableFlag;	// 0:Disable 1:Enable -1:ALL
};

struct TAGetListFarm: public TAHeader
{
	BYTE			cCount;
	TFarmItemFromDB	Farms[Farm::Max::FARMCOUNT];
};

struct TQGetListField:public TQHeader
{
	int			iFarmDBID;
	int			iRoomID;
};

struct TAGetListField:public TAHeader
{
	int			iFarmDBID;
	int			iRoomID;
	BYTE		cCount;
	TFarmField	Fields[Farm::Max::FIELDCOUNT];
};

struct TQGetListFieldForCharacter:public TQHeader
{
	int			iRoomID;
	INT64		biCharacterDBID;
};

struct TAGetListFieldForCharacter:public TAHeader
{
	int						iRoomID;			// RoomID must ALWAYS be at the very top!
	INT64					biCharacterDBID;
	BYTE					cCount;
	TFarmFieldForCharacter	Fields[Farm::Max::PRIVATEFIELDCOUNT];
};

struct TQGetListFieldByCharacter:public TQHeader
{
	INT64		biCharacterDBID;
	bool		bRefreshGate;
};

struct TAGetListFieldByCharacter:public TAHeader
{
	bool				bRefreshGate;
	BYTE				cCount;
	TFarmFieldPartial	Fields[Farm::Max::FIELDCOUNT_PERCHARACTER];
};

struct TQAddField:public TQHeader
{
	int						iRoomID;
	int						iFarmDBID;
	short					nFieldIndex;
	INT64					biCharacterDBID;
	int						iItemID;
	int						iElapsedTimeSec;
	int						iAttachItemID;
	int						iMaxFieldCount;
	// Info for removing seed
	BYTE					cInvenIndex;
	INT64					biSeedItemSerial;
	int						iMapID;
	WCHAR					wszIP[IPLENMAX];
	BYTE					cAttachCount;
	TFarmAttachItemSerial	AttachItems[Farm::Max::PLANTING_ATTACHITEM];
};

struct TAAddField:public TAHeader
{
	int						iRoomID;			// RoomID must ALWAYS be at the very top!
	short					nFieldIndex;
	INT64					biCharacterDBID;
	// Info for removing seed
	BYTE					cInvenIndex;
	INT64					biSeedItemSerial;
	BYTE					cAttachCount;
	TFarmAttachItemSerial	AttachItems[Farm::Max::PLANTING_ATTACHITEM];
};

struct TQDelField:public TQHeader
{
	int		iFarmDBID;
	int		iRoomID;
	short	nFieldIndex;
};

struct TADelField:public TAHeader
{
	int		iRoomID;		// RoomID must ALWAYS be at the very top!
	short	nFieldIndex;
};

struct TQDelFieldForCharacter:public TQHeader
{
	int		iRoomID;
	short	nFieldIndex;
	INT64	biCharacterDBID;
};

struct TADelFieldForCharacter:public TAHeader
{
	int		iRoomID;		// RoomID must ALWAYS be at the very top!
	short	nFieldIndex;
	INT64	biCharacterDBID;
};

struct TQAddFieldAttachment:public TQHeader
{
	int		iRoomID;
	int		iFarmDBID;
	INT64	biCharacterDBID;
	short	nFieldIndex;
	int		iAttachItemID;
	// Info for removing AttachItem
#if defined( PRE_ADD_VIP_FARM )
	bool	bVirtualAttach;	// Like giving virtual boostitem on VIP farm, when true only attach occurs without deleting the item
#endif // #if defined( PRE_ADD_VIP_FARM )
	BYTE	cInvenType;
	BYTE	cInvenIndex;
	INT64	biAttachItemSerial;
	int		iMapID;
	WCHAR	wszIP[IPLENMAX];
};

struct TAAddFieldAttachment:public TAHeader
{
	int		iRoomID;			// RoomID must ALWAYS be at the very top!
	INT64	biCharacterDBID;
	short	nFieldIndex;
	int		iAttachItemID;
	// Info for removing AttachItem
#if defined( PRE_ADD_VIP_FARM )
	bool	bVirtualAttach;	// Like giving virtual boostitem on VIP farm, when true only attach occurs without deleting the item
#endif // #if defined( PRE_ADD_VIP_FARM )
	BYTE	cInvenType;
	BYTE	cInvenIndex;
	INT64	biAttachItemSerial;
};

struct TUpdateFieldElapsedTime
{
	short	nFieldIndex;
	int		iElapsedTimeSec;
};

struct TQModFieldElapsedTime:public TQHeader
{
	int						iFarmDBID;
	BYTE					cCount;
	TUpdateFieldElapsedTime	Updates[Farm::Max::FIELDCOUNT];
};

struct TQModFieldForCharacterElapsedTime:public TQHeader
{
	INT64					biCharacterDBID;
	BYTE					cCount;
	TUpdateFieldElapsedTime	Updates[Farm::Max::PRIVATEFIELDCOUNT];
};

struct TAModFieldForCharacterElapsedTime:public TAHeader
{
	
};

struct TAModFieldElapsedTime:public TAHeader
{

};

struct THarvestItem
{
	INT64	biItemSerial;
	int		iItemID;
	short	nItemCount;
	bool	bSoulBoundFlag;
	BYTE	cSealCount;
	BYTE	cItemOption;
	bool	bEternityFlag;
	int		iLifeSpan;
};

struct TQHarvest:public TQHeader
{
	int				iFarmDBID;
	int				iRoomID;
	short			nFieldIndex;
	INT64			biCharacterDBID;
	INT64			biHarvestCharacterDBID;
	int				iMapID;
	CHAR			szIP[IPLENMAX];
	THarvestItem	HarvestItems[Farm::Max::HARVESTITEM_COUNT];
};

struct TAHarvest:public TAHeader
{
	int				iRoomID;			// RoomID must ALWAYS be at the very top!
	int				nFieldIndex;
	INT64			biCharacterDBID;
};

struct TQGetListHarvestDepotItem:public TQHeader
{
	INT64	biCharacterDBID;
	INT64	biLastUniqueID;
};

struct TAGetListHarvestDepotItem:public TAHeader
{
	INT64				biLastUniqueID;
	BYTE				cCount;
	TFarmWareHouseItem	Items[Farm::Max::HARVESTDEPOT_COUNT];
};

struct TQGetCountHarvestDepotItem:public TQHeader
{
	INT64	biCharacterDBID;
};

struct TAGetCountHarvestDepotItem:public TAHeader
{
	int		iCount;
};

struct TQGetFieldCountByCharacter:public TQHeader
{
	INT64	biCharacterDBID;
	bool	bIsSend;
};

struct TAGetFieldCountByCharacter:public TAHeader
{
	int		iFieldCount;
	bool	bIsSend;
};

struct TQUpdateFarmActivation : public TQHeader			//QUERY_UPDATEFARM_ACTIVATION
{
	int nFarmDBID;
};

struct TFarmActivation
{
	int nFarmDBID;
	bool bFarmActivation;
};

struct TAUpdateFarmActivation : public TAHeader			//QUERY_UPDATEFARM_ACTIVATION
{
	BYTE cCount;
	TFarmActivation Farms[Farm::Max::FARMCOUNT];
};

struct TQGetFieldItemCount:public TQHeader
{
	INT64	biCharacterDBID;
	int		iItemID;
	int		iRoomID;
	int		iAreaIndex;
	UINT nSessionID;
};

struct TAGetFieldItemCount:public TAHeader
{
	int		iRoomID;
	int		iAreaIndex;
	int		iItemCount;
	UINT nSessionID;
};

struct TQGetGuildRecruit:public TQHeader
{
	UINT	uiPage;
	BYTE	cJobCode;
	BYTE	cLevel;	
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE	cPurposeCode;
	WCHAR wszGuildName[GUILDNAME_MAX];
	BYTE	cSortType;
#endif
};
struct TAGetGuildRecruit:public TAHeader
{
	UINT		uiPage;
	BYTE		cJobCode;
	BYTE		cLevel;
	BYTE		cCount;
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE	cPurposeCode;
	WCHAR wszGuildName[GUILDNAME_MAX];
	BYTE	cSortType;
#endif
	TGuildRecruitInfo	GuildRecruitList[GuildRecruitSystem::Max::GuildRecruitRowCount];
};
struct TQGetGuildRecruitCharacter:public TQHeader
{		
	UINT nGuildDBID;	// Guild DBID
};
struct TAGetGuildRecruitCharacter:public TAHeader
{
	UINT nGuildDBID;	// Guild DBID
	char cWorldSetID;	// World ID
	BYTE cCount;
	TGuildRecruitCharacter	GuildRecruitCharacterList[GuildRecruitSystem::Max::GuildRecruitCharacterRowCount];
};
struct TQGetMyGuildRecruit:public TQHeader
{
	INT64 biCharacterDBID;	// Character code
};
struct TAGetMyGuildRecruit:public TAHeader
{
	BYTE				cCount;
	TGuildRecruitInfo	GuildRecruitList[GuildRecruitSystem::Max::MaxRequestCount];
};
struct TQGetGuildRecruitRequestCount:public TQHeader
{
	INT64 biCharacterDBID;	// Character code
};
struct TAGetGuildRecruitRequestCount:public TAHeader
{
	BYTE				cCount;	
};
struct TQRegisterInfoGuildRecruit:public TQHeader
{	
	UINT		nGuildDBID;					// Guild DBID		
};
struct TARegisterInfoGuildRecruit:public TAHeader
{
	UINT		nGuildDBID;					// Guild DBID	
	BYTE		cClassCode[CLASSKINDMAX];	// Recruiting class
	int			nMinLevel;					// Min level
	int			nMaxLevel;					// Max level
	WCHAR		wszGuildIntroduction[GUILDRECRUITINTRODUCE+1];	//Guild introduction
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE		cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
	bool		bCheckHomePage;				//Guild homepage	
#endif
};
struct TQRegisterOnGuildRecruit:public TQHeader
{	
	UINT		nGuildDBID;					// Guild DBID	
	BYTE		cClassCode[CLASSKINDMAX];	// Recruiting class
	int			nMinLevel;					// Min level
	int			nMaxLevel;					// Max level
	WCHAR		wszGuildIntroduction[GUILDRECRUITINTRODUCE+1];	//Guild introduction
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE		cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
	bool		bCheckHomePage;				//Guild homepage
#endif
};
struct TARegisterOnGuildRecruit:public TAHeader
{
};
struct TQRegisterModGuildRecruit:public TQHeader
{	
	UINT		nGuildDBID;					// Guild DBID	
	BYTE		cClassCode[CLASSKINDMAX];	// Recruiting class
	int			nMinLevel;					// Min level
	int			nMaxLevel;					// Max level
	WCHAR		wszGuildIntroduction[GUILDRECRUITINTRODUCE+1];	//Guild introduction
#if defined( PRE_ADD_GUILD_EASYSYSTEM )
	BYTE		cPurposeCode;				// Guild purpose (1:Social 2:Combat 3:Nest 4:Colosseum)
	bool		bCheckHomePage;				//Guild homepage
#endif
};
struct TARegisterModGuildRecruit:public TAHeader
{
};
struct TQRegisterOffGuildRecruit:public TQHeader
{	
	UINT		nGuildDBID;					// Guild DBID		
};
struct TARegisterOffGuildRecruit:public TAHeader
{
};
struct TQGuildRecruitRequestOn:public TQHeader
{
	INT64		biCharacterDBID;	// Character code
	UINT		nGuildDBID;			// Guild DBID
};
struct TAGuildRecruitRequestOn:public TAHeader
{
};
struct TQGuildRecruitRequestOff:public TQHeader
{
	INT64		biCharacterDBID;	// Character code
	UINT		nGuildDBID;			// Guild DBID
};
struct TAGuildRecruitRequestOff:public TAHeader
{
};
struct TQGuildRecruitAcceptOn:public TQHeader
{
	INT64		biCharacterDBID;	// Character code
	UINT		nGuildDBID;			// Guild DBID
	INT64		biAcceptCharacterDBID;	// Character code that approved the join
	WCHAR		wszToCharacterName[NAMELENMAX];
	short		wGuildSize;			// Max guild member count	
};
struct TAGuildRecruitAcceptOn:public TAHeader
{
	UINT		nAddAccountDBID;			//DBID to add
	INT64		biAddCharacterDBID;			// Character DBID to add	
	WCHAR		wszToCharacterName[NAMELENMAX];
	BYTE		cJob;						// Job	
	BYTE		cLevel;						// Level	
	__time64_t	JoinDate;					// Guild join date
	bool		bDelGuildRecruit;			// Keep guild recruitment board	
};
struct TQGuildRecruitAcceptOff:public TQHeader
{
	INT64		biCharacterDBID;	// Character code	
	UINT		nGuildDBID;			// Guild DBID
	INT64		biDenyCharacterDBID;	// Character code to reject
	WCHAR		wszToCharacterName[NAMELENMAX];
};
struct TAGuildRecruitAcceptOff:public TAHeader
{
	WCHAR		wszToCharacterName[NAMELENMAX];
	INT64		biDenyCharacterDBID;// Character code to reject
};
struct TQDelGuildRecruit:public TQHeader
{
	INT64		biCharacterDBID;	// Character code	
	UINT		nGuildDBID;			// Guild DBID
	BYTE		nDelType;			// Delete type (1: Guild, 2: Personal)
};

#if defined (PRE_ADD_DONATION)
struct TQDonate : public TQHeader
{
	INT64 biCharacterDBID;
	time_t tDonationTime;
	INT64 nCoin;
};

struct TADonate : public TAHeader
{
};

struct TQDonationRanking : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TADonationRaking : public TAHeader
{
	Donation::Ranking Ranking[Donation::MAX_RANKING];
	INT64 nMyCoin;
	int nMyRanking;
};

struct TQDonationTopRanker : public TQHeader
{

};

struct TADonationTopRanker : public TAHeader
{
	char cWorldID;
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
	INT64 nCoin;
};
#endif // #if defined (PRE_ADD_DONATION)

struct TQMaintenanceInfo : public TQHeader
{
	int nFlag;
};

#if defined( PRE_PARTY_DB )

struct DBCreateParty
{
	Party::Data PartyData;
#if defined(PRE_FIX_62281)
	WCHAR wszAfterInviteCharacterName[NAMELENMAX];
#else
	UINT uiAfterInviteAccountDBID;	
#endif
};

struct TQAddParty:public TQHeader
{
	DBCreateParty Data;
	bool bCheat;
};

struct TAAddParty:public TAHeader
{
	DBCreateParty Data;
	bool bCheat;
};

struct DBAddPartyMemberGame
{	
	INT64 biCharacterDBID;
	UINT nSessionID;
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComebackAppellation;
#endif
};
struct TQAddPartyAndMemberGame:public TQHeader
{
	DBCreateParty Data;
	UINT nKickedMemberList[PARTYKICKMAX];
	DBAddPartyMemberGame Member[PARTYMAX];	// Only party members are variable-length; kick member above is not variable
};
struct TAAddPartyAndMemberGame:public TAHeader
{
	DBCreateParty Data;
	UINT nKickedMemberList[PARTYKICKMAX];
};
struct TQAddPartyAndMemberVillage:public TQHeader
{
	DBCreateParty Data;		
	int nKickCount;
	UINT nVoiceChannelID;
	UINT nKickedMemberList[PARTYKICKMAX];	
	TMemberInfo MemberInfo[PARTYMAX];	
};
struct TAAddPartyAndMemberVillage:public TAHeader
{
	DBCreateParty Data;	
	UINT nVoiceChannelID;
	UINT nKickedMemberList[PARTYKICKMAX];	
	TMemberInfo MemberInfo[PARTYMAX];
};

struct TQModParty:public TQHeader
{
	Party::Data PartyData;	
	int iRoomID; // Used only on game server
};

struct TAModParty:public TAHeader
{
	Party::Data PartyData;	
	int iRoomID; // Used only on game server
};

struct TQDelParty:public TQHeader
{
	TPARTYID PartyID;
};

struct TADelParty:public TAHeader
{
	TPARTYID PartyID;
};

struct TQDelPartyForServer:public TQHeader
{
	int iServerID;
};

struct TADelPartyForServer:public TAHeader
{
	int iServerID;
};

struct TQJoinParty:public TQHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
	UINT nSessionID;
	int iMaxUserCount;
#if defined( PRE_ADD_NEWCOMEBACK )
	bool bCheckComeBackParty;
#endif
};

struct TAJoinParty:public TAHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
	UINT nSessionID;
	BYTE cThreadID;
};

struct TQOutParty:public TQHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
	UINT nSessionID;
	Party::QueryOutPartyType::eCode Type;
};

struct TAOutParty:public TAHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
	UINT nSessionID;
	Party::QueryOutPartyType::eCode Type;
	bool bIsDelParty;
};

struct TQGetListParty:public TQHeader
{
	int iExecptServerID;
};

struct TAGetListParty:public TAHeader
{
	bool bIsStart;
	short nCount;
	Party::Data DataArr[100];
};

struct TQGetListPartyMember:public TQHeader
{
	TPARTYID PartyID;
};

struct TAGetListPartyMember:public TAHeader
{
	TPARTYID PartyID;
	ePartyType PartyType;
	short nCount;
	Party::MemberData MemberData[PARTYCOUNTMAX];
};

struct TQModPartyLeader:public TQHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
};

struct TAModPartyLeader:public TAHeader
{

};

#if defined( PRE_ADD_NEWCOMEBACK )
struct TQModComebackFlag:public TQHeader
{
	bool bRewardComeback;
};

struct TQModPartyMemberComeback:public TQHeader
{
	TPARTYID PartyID;
	INT64 biCharacterDBID;
	bool bComebackAppellation;
};
#endif	// #if defined( PRE_ADD_NEWCOMEBACK )
#endif // #if defined( PRE_PARTY_DB )
#if defined(PRE_ADD_DOORS_PROJECT)
struct TQSaveCharacterAbility : public TQHeader
{
	INT64 biCharacterDBID;
	int nCharacterAblity[Doors::CharacterAbility::MaxType];	
};
#endif // #if defined(PRE_ADD_DOORS_PROJECT)

#if defined (PRE_ADD_BESTFRIEND)

// QUERY_GET_BESTFRIEND
struct TQGetBestFriend: public TQHeader
{
	bool bSend;
	INT64 biCharacterDBID;
};

struct TAGetBestFriend: public TAHeader
{
	bool bSend;
	TBestFriendInfo Info;
};

// QUERY_REGIST_BESTFRIEND
struct TQRegistBestFriend: public TQHeader
{
	UINT nFromAccountDBID;
	INT64 biFromCharacterDBID;
	WCHAR wszFromName[NAMELENMAX];

	UINT nToAccountDBID;
	INT64 biToCharacterDBID;
	WCHAR wszToName[NAMELENMAX];

	int nItemID;

	// Info for item removal
	INT64 biRegistSerial;
	int	nMapID;
	WCHAR wszIP[IPLENMAX];	
};

struct TARegistBestFriend: public TAHeader
{
	UINT nFromAccountDBID;
	INT64 biFromCharacterDBID;
	WCHAR wszFromName[NAMELENMAX];

	UINT nToAccountDBID;
	WCHAR wszToName[NAMELENMAX];

	int nItemID;
	INT64 biRegistSerial;
};

// QUERY_CANCEL_BESTFRIEND
struct TQCancelBestFriend: public TQHeader
{
	INT64 biCharacterDBID;
	bool bCancel;
};

struct TACancelBestFriend: public TAHeader
{
	INT64 biCharacterDBID;
	bool bCancel;
};

// QUERY_CLOSE_BESTFRIEND
struct TQCloseBestFriend: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TACloseBestFriend: public TAHeader
{
	INT64 biCharacterDBID;
};

// QUERY_EDIT_BESTFRIENDMEMO
struct TQEditBestFriendMemo: public TQHeader
{
	INT64 biCharacterDBID;
	WCHAR wszMemo[BESTFRIENDMEMOMAX];
};

struct TAEditBestFriendMemo: public TAHeader
{
	INT64 biToCharacterDBID;
	WCHAR wszMemo[BESTFRIENDMEMOMAX];
};

#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_ADD_58761 )
struct TQNestDeathLog: public TQHeader
{
	INT64 biCharacterDBID;
	INT64 biRoomID;
	int nMonterID;
	int nSkillID;
	BYTE cCharacterJob;
	BYTE cCharacterLevel;
};
#endif


//#########################################################################################################################################
//	MAINCMD_PRIVATECHATCHANNEL
//#########################################################################################################################################

#if defined( PRE_PRIVATECHAT_CHANNEL )
struct TQPrivateChatChannelInfo: public TQHeader
{	
};
struct TAPrivateChatChannelInfo: public TAHeader
{	
	char cWorldSetID;
	int nCount;
	TPrivateChatChannelInfo tPrivateChatChannel[PrivateChatChannel::Common::GetDBMaxChannel];		
};

struct TQPrivateChatChannelMember: public TQHeader
{
	INT64 nPrivateChatChannelID;
};

struct TAPrivateChatChannelMember: public TAHeader
{	
	char cWorldSetID;
	INT64 nPrivateChatChannelID;
	int nCount;
	TPrivateChatChannelMember Member[PrivateChatChannel::Common::MaxMemberCount];
};

struct TQAddPrivateChatChannel: public TQHeader
{	
	time_t tJoinDate;
	WCHAR wszCharacterName[NAMELENMAX];
	TPrivateChatChannelInfo tPrivateChatChannel;		
};

struct TAAddPrivateChatChannel: public TAHeader
{
	char cWorldSetID;
	time_t tJoinDate;
	WCHAR wszCharacterName[NAMELENMAX];
	TPrivateChatChannelInfo tPrivateChatChannel;
};

struct TQAddPrivateChatMember: public TQHeader
{
	int nServerID;
	PrivateChatChannel::Common::eModType eType;
	INT64 nPrivateChatChannelID;	
	TPrivateChatChannelMember Member;
};

struct TAAddPrivateChatMember: public TAHeader
{
	char cWorldSetID;
	INT64 nPrivateChatChannelID;
	TPrivateChatChannelMember Member;
};

struct TQInvitePrivateChatMember: public TQHeader
{
	int nServerID;
	UINT nMasterAccountDBID;
	INT64 nPrivateChatChannelID;	
	TPrivateChatChannelMember Member;
};

struct TAInvitePrivateChatMember: public TAHeader
{
	char cWorldSetID;
	UINT nMasterAccountDBID;
	INT64 nPrivateChatChannelID;
	TPrivateChatChannelMember Member;
};

struct TQDelPrivateChatMember: public TQHeader
{
	PrivateChatChannel::Common::eModType eType;
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];
	INT64 nPrivateChatChannelID;
};

struct TADelPrivateChatMember: public TAHeader
{		
	char cWorldSetID;
	INT64 biCharacterDBID;
	WCHAR wszName[NAMELENMAX];
	INT64 nPrivateChatChannelID;
};

struct TQModPrivateChatChannelInfo: public TQHeader
{
	PrivateChatChannel::Common::eModType eType;
	INT64 nPrivateChatChannelID;
	WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];
	int nPassWord;
	INT64 biCharacterDBID;	
};

struct TAModPrivateChatChannelInfo: public TAHeader
{
	char cWorldSetID;
	PrivateChatChannel::Common::eModType eType;
	INT64 nPrivateChatChannelID;
	WCHAR wszChannelName[PrivateChatChannel::Common::MaxNameLen];
	int nPassWord;
	INT64 biCharacterDBID;	
};

struct TQModPrivateChatMemberServerID: public TQHeader
{
	int nServerID;	
	INT64 biCharacterDBID;
};

struct TQDelPrivateChatMemberServerID: public TQHeader
{
	int nServerID;		
};

struct TADelPrivateChatMemberServerID: public TAHeader
{
	char cWorldSetID;
	int nCount;
	TPrivateMemberDelServer Member[PrivateChatChannel::Common::MaxMemberCount];	
};
#endif

#if defined( PRE_FIX_67546 )

struct TQAddThreadCount: public TQHeader
{
	BYTE cChoiceThreadID;
};

struct TAAddThreadCount: public TAHeader
{
	int nConnectionCount[THREADMAX];
};

#endif

#if defined( PRE_WORLDCOMBINE_PVP )

struct TQAddWorldPvPRoom: public TQHeader
{
	TWorldPvPMissionRoom TMissionRoom;
	int nServerID;	
	int nRoomID;
	WCHAR wszRoomName[PARTYNAMELENMAX];		
	bool bBreakIntoFlag;
	bool bRegulationFlag;
	bool bObserverFlag;
	bool bRandomFlag;	
	int nPassWord;	
};

struct TAAddWorldPvPRoom: public TAHeader
{
	char cWorldSetID;
	int nRoomIndex;
	int nRoomID;
	int nPvPRoomDBIndex;
	TWorldPvPRoomDBData Data;
};

struct TQAddWorldPvPRoomMember: public TQHeader
{
	int nWorldPvPRoomDBIndex;
	INT64 biCharacterDBID;
	bool bObserverFlag;
	BYTE cMaxPlayers;
};

struct TQDelWorldPvPRoom: public TQHeader
{	
	int nWorldPvPRoomDBIndex;	
};

struct TQDelWorldPvPRoomMember: public TQHeader
{
	int nWorldPvPRoomDBIndex;
	INT64 biCharacterDBID;	
};

struct TQDelWorldPvPRoomForServer: public TQHeader
{	
	int nServerID;	
};

struct TQGetListWorldPvPRoom: public TQHeader
{	
	int nCombineWorldID;
};

struct TAGetListWorldPvPRoom: public TAHeader
{
	int nCount;
	TWorldPvPRoomDBData WorldPvPRoomData[WorldPvPMissionRoom::Common::MaxDBDataCount];
};

struct TQUpdateWorldPvPRoom: public TQHeader
{	
};

struct TAUpdateWorldPvPRoom: public TAHeader
{
	bool bFlag;
};

#endif

//#########################################################################################################################################


//#########################################################################################################################################
//	MAINCMD_ALTEIAWORLD
//#########################################################################################################################################

#if defined( PRE_ALTEIAWORLD_EXPLORE )

struct TQResetAlteiaWorldEvent: public TQHeader
{	
};

struct TQAddAlteiaWorldEvent: public TQHeader
{
	time_t tAlteiaEventStartTime;
	time_t tAlteiaEventEndTime;
};

struct TQADDAlteiaWorldPlayResult: public TQHeader
{	
	INT64 biCharacterDBID;
	int nGoldKeyCount;
	DWORD dwPlayTime;
	int nGuildID;
};

struct TQGetAlteiaWorldInfo: public TQHeader
{	
	INT64 biCharacterDBID;	
};

struct TAGetAlteiaWorldInfo: public TAHeader
{
	int nBestGoldKeyCount;
	UINT uiBestPlayTime;	
	int nDailyPlayCount;
	int nWeeklyPlayCount;
};

struct TQGetAlteiaWorldRankInfo: public TQHeader
{	
	int nMaxRank;
	AlteiaWorld::Info::eInfoType eType;
};

struct TAGetAlteiaWorldPrivateGoldKeyRank: public TAHeader
{
	int nCount;
	AlteiaWorld::GoldKeyRankMemberInfo RankMemberInfo[AlteiaWorld::Common::MaxRankCount];
};

struct TAGetAlteiaWorldPrivatePlayTimeRank: public TAHeader
{
	int nCount;
	AlteiaWorld::PlayTimeRankMemberInfo RankMemberInfo[AlteiaWorld::Common::MaxRankCount];
};

struct TAGetAlteiaWorldGuildGoldKeyRank: public TAHeader
{
	int nCount;
	AlteiaWorld::GuildGoldKeyRankInfo RankMemberInfo[AlteiaWorld::Common::MaxRankCount];
};

struct TQGetAlteiaWorldSendTicketList: public TQHeader
{	
	INT64 biCharacterDBID;
};

struct TAGetAlteiaWorldSendTicketList: public TAHeader
{
	int nSendTicketCount;
	int nRecvTicketCount;
	INT64 biCharacterDBID[AlteiaWorld::Common::MaxSendCount];
};

struct TQAddAlteiaWorldSendTicketList: public TQHeader
{	
	INT64 biCharacterDBID;
	INT64 biSendCharacterDBID;
	WCHAR wszSendCharacterName[NAMELENMAX];
	int nSendMaxCount;
};

struct TAAddAlteiaWorldSendTicketList: public TAHeader
{	
	INT64 biSendCharacterDBID;
	WCHAR wszSendCharacterName[NAMELENMAX];
};

struct TQResetAlteiaWorldPlayAlteia: public TQHeader
{	
	INT64 biCharacterDBID;
	AlteiaWorld::ResetType::eResetType eType;
};

#endif // #if defined( PRE_ALTEIAWORLD_EXPLORE )


//#########################################################################################################################################
//	MAINCMD_STAMPSYSTEM
//#########################################################################################################################################

#if defined( PRE_ADD_STAMPSYSTEM )
struct TQGetListCompleteChallenges: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListCompleteChallenges: public TAHeader
{
	StampSystem::StampCompleteInfo CompleteInfo[StampSystem::Common::MaxStampSlotCount];
};

struct TQInitCompleteChallenge: public TQHeader
{
	INT64 biCharacterDBID;
};

struct TQAddCompleteChallenge: public TQHeader
{
	INT64 biCharacterDBID;
	BYTE cChallengeIndex;
	int nWeekDay;
};

struct TAAddCompleteChallenge: public TAHeader
{
	BYTE cChallengeIndex;
};

#endif // #if defined( PRE_ADD_STAMPSYSTEM )


//#########################################################################################################################################
//	MAINCMD_DWC & ChaosGauge
//#########################################################################################################################################

#if defined( PRE_ADD_CHAOSCUBE )
struct TQGetListChaosGaugeItems : public TQHeader
{
	INT64 biCharacterDBID;
};

struct TAGetListChaosGaugeItems : public TAHeader
{
	int nCount;
	TChaosGaugeInfo GaugeList[20];
};

struct TQUpsChaosGaugeItem : public TQHeader
{
	INT64 biCharacterDBID;
	int nChaosGaugeType;
	int nItemGauge;
};
#endif // #if defined( PRE_ADD_CHAOSCUBE )

#if defined( PRE_ADD_DWC )
struct TQAddDWCTeam : public TQHeader
{
	INT64 biCharacterDBID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
};

struct TAAddDWCTeam : public TAHeader
{
	UINT nTeamID;
	WCHAR wszTeamName[DWC::MAX_TEAMNAME_LEN];
};

struct TQAddDWCTeamMember : public TQHeader
{
	UINT nTeamID;
	INT64 biCharacterDBID;
	int nMaxMemberCount;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TAAddDWCTeamMember : public TAHeader
{
	UINT nTeamID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TQDelDWCTeamMember : public TQHeader
{
	UINT nTeamID;
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TADelDWCTeamMember : public TAHeader
{
	UINT nTeamID;
	INT64 biCharacterDBID;
	WCHAR wszCharacterName[NAMELENMAX];
};

struct TQGetDWCTeamByChar : public TQHeader
{
	INT64 biCharacterDBID;
	bool bNeedMemberList;
};

struct TAGetDWCTeam : public TAHeader
{
	TDWCTeamInfo TeamInfo;
	bool bNeedMemberList;
};

struct TQGetListDWCTeamMember : public TQHeader
{
	UINT nTeamID;
};

struct TAGetListDWCTeamMember : public TAHeader
{
	UINT nTeamID;
	int nCount;
	TDWCTeamMemberInfo MemberList[DWC::MAX_TEAM_MEMBER];
};

struct TQGetPageDWCRanking : public TQHeader
{
	int nPageNum;
	int nPageSize;
};

struct TAGetPageDWCRanking : public TAHeader
{
	int nTotalCount;
	int nPageNum;
	int nCount;
	TDWCRankInfo RankList[DWC::MAX_RANK_PER_PAGE];
};

struct TQGetDWCRanking : public TQHeader
{
	BYTE cFindType;
	WCHAR wszFindName[DWC::MAX_FIND_NAME_LEN];
};

struct TAGetDWCRanking : public TAHeader
{
	TDWCRankInfo RankInfo;
};

struct TQGetDWCChannel : public TQHeader
{
};

struct TAGetDWCChannel : public TAHeader
{
	TDWCChannelInfo ChannelInfo;
};
#endif // #if defined( PRE_ADD_DWC )

#if defined( PRE_ADD_TALISMAN_SYSTEM )
struct TQModTalismanSlotBitmap : public TQHeader
{
	INT64 biCharacterDBID;
	int nTalismanOpenFlag;
};
#endif	// #if defined( PRE_ADD_TALISMAN_SYSTEM )


#pragma pack(pop)
