#pragma once

#include "DNDataManagerBase.h"
#include "DNTableFile.h"

#if defined(_LOGINSERVER)
#define FLAG_NICK_EQUAL		0
#define FLAG_NICK_INCLUDE	1

#define PROHIBITWORDMAX		64

struct ProhibitWordInfo
{
	unsigned int nFlag;
	wchar_t szNick[PROHIBITWORDMAX];
};

#endif	// _LOGINSERVER

enum GlobalWeightTableIndex		// Moved here rather than serverdef because village and game use DnTableDB.h
{
	CharacterSlotMax = 34,				// 34: Character slot count limit
	CHARACTERNAME_MAXLENGTH = 1010,		//Index value since it fetches from GlobalWeightTable
	PVPCHANNEL_LIMITLEVEL = 1018,		//Index value since it fetches from GlobalWeightTable
	Login_Max_CreateCharCount = 1022,	// Max creatable characters per account (GlobalWeight 1022)
	Login_Base_CreateCharCount = 1023,	// Default character creation count per account (GlobalWeight 1023)	
	BeginnerGuild_UserMax = 1026,		// Beginner guild member limit 1026
	BeginnerGuild_GiftMailID = 1027,	// Beginner guild join gift 1027
	MasterSystem_MasterAndPupilAllowLevelGap = 1038		// 1038 Master-Apprentice system: level difference between master and apprentice
};

enum GlobalWeightIntTableIndex		// Moved here rather than serverdef because village and game use DnTableDB.h
{
	ComebackTerm_Day = 10006,
	ComebackEffectItemID_Login = 10009,	
	SpecialBoxKeepPeriod = 10010,
	NewbieReConnectReward_Hour = 10011, 
};

class CDNExtManager
#if defined(_LOGINSERVER) || defined(_MASTERSERVER)
: public CDNDataManagerBase
#endif	// #if defined(_LOGINSERVER) || defined(_MASTERSERVER)
{
private:
	typedef map<int, TMapInfo*> TMapInfoMap;
	TMapInfoMap m_pMapInfo;

	typedef map<int, TPCBangData*> TMapPCBangData;
	TMapPCBangData m_pPCBangData;

	typedef map<BYTE, TDefaultCreateData*> TMapDefaultCreate;	// first: ClassID
	TMapDefaultCreate m_pMapDefaultCreate;

public:
	struct TDWCCreateData
	{
		BYTE cClassID;
		BYTE cLevel;
		int nExperience;
		int nCreateMapIndex;
		BYTE cJobCode1;
		BYTE cJobCode2;
		int nGold;
		int nSkillPoint;
		int nCreateMapStartPositionX[DEFAULTPOSITIONMAX];
		int nCreateMapStartPositionY[DEFAULTPOSITIONMAX];
		int nCreateMapStartRadius[DEFAULTPOSITIONMAX];
		int nDefaultItemID[DEFAULTITEMMAX];
		int nDefaultItemCount[DEFAULTITEMMAX];
		int nDefaultSkillID[DEFAULTSKILLMAX];
		TQuickSlot DefaultQuickSlot[DEFAULTQUICKSLOTMAX];
	};
private:
	std::vector<TDWCCreateData> m_VecDWCCreate;

	typedef map<int, TSkillData*> TMapSkillData;		// Select skills unlocked by default and insert them into the owned skill list at level 0.
	TMapSkillData m_pMapSkillData;

	map<int, vector<TSkillData*> > m_mapDefaultUnlockSkillsByClass;

	typedef map<BYTE, TCoinCountData*> TMapCoinCount;	// first: Level
	TMapCoinCount m_pMapCoinCount;

	typedef map<int, TLevelData*> TMapLevelData;
	TMapLevelData m_pLevelData;

#if defined(_LOGINSERVER)
	typedef std::map <unsigned int, std::wstring> TProhibitWord;
	TProhibitWord m_ProhibitWordList;
#endif	// #if defined(_LOGINSERVER)

	typedef map<int, TWeaponData*> TMapWeaponData;
	TMapWeaponData m_pWeaponData;

	typedef map<int, TPartData*> TMapPartData;
	TMapPartData m_pPartData;

	typedef map<UINT, TPvPGameStartConditionTable*>	TPvPGameStartConditionData;
	TPvPGameStartConditionData		m_PvPGameStartConditionTable;

	typedef map<UINT, TSchedule>	TScheduleData;
	TScheduleData					m_ScheduleTable;

	typedef map<UINT, TPvPGameModeTable> TPvPGameModeTableData;
	TPvPGameModeTableData			m_PvPGameModeTable;

	std::map<LadderSystem::MatchType::eCode,std::vector<int>>	m_MatchTypePvPGameModeID;
	typedef map<UINT, TPvPMapTable*>	TPvPMapTableData;
	TPvPMapTableData				m_PvPMapTable;

	typedef map<char, TGuildWarMapInfo> TGuildWarMapInfoData;
	TGuildWarMapInfoData			m_GuildWarMapInfo;

	typedef map<int, TMailTableData>		TMapMailTableData;
	TMapMailTableData		m_MailTableData;

	std::vector<TGuildWarRewardData *> m_pGuildWarRewardData;

	typedef map<int, TJobTableData>		TMapJobTableData;
	TMapJobTableData		m_JobTableData;

	typedef map<int, TCashCommodityData*> TMapCashCommodity;	// first: ItemSN
	TMapCashCommodity m_pMapCashCommodity;

	typedef map<int, TCashPackageData*> TMapCashPackage;	// first: ItemSN
	TMapCashPackage m_pMapCashPackage;

	typedef map<int, TCashLimitData*> TMapCashLimit;	// first: ItemSN
	TMapCashLimit m_pMapCashLimit;

	typedef map<int, TVehicleData> TMapVehicleData;	// first: ItemID
	TMapVehicleData m_MapVehicle;

	typedef map<int, TVehiclePartsData> TMapVehicleParts;	// first: ItemID
	TMapVehicleParts m_MapVehicleParts;

	typedef map<int, TItemData*> TMapItemData;
	TMapItemData m_pItemData;

	typedef map<int, TGlobalWeightData*> TMapGlobalWeight;	// first: ID
	TMapGlobalWeight m_pMapGlobalWeight;

	typedef map<int, TGlobalWeightIntData*> TMapGlobalWeightInt;	// first: ID
	TMapGlobalWeightInt m_pMapGlobalWeightInt;

	CSyncLock m_Sync;		//Protect m_pMapCashCommodity and m_pMapCashPackage
	bool LoadCashCommodity(bool bReload = false);
	bool LoadCashPackage(bool bReload = false);

	bool LoadCashLimit();
	bool LoadItemData();

	bool LoadPlayerLevel();
	bool LoadGlobalWeight();
	bool LoadGlobalWeightInt();

	// VehicleTable
	bool LoadVehicleData();
	// VehiclePartsTable
	bool LoadVehiclePartsData();
	bool LoadPvPGameStartConditionTable();
	bool LoadScheduleTable();
	bool LoadPvPGameModeTable();
	bool LoadGuildWarMapInfoTable();
	bool LoadPvPMapTable();
	bool LoadMailTableData();
	bool LoadGuildWarRewardData();	
	bool LoadJobTableData();

	unsigned int MemoryToRSHashKey(const wchar_t* pszData, long nSize);

	void FindExtFileList( const char *szFileName, std::vector<std::string> &szVecList );
	DNTableFileFormat* LoadExtTable( const char *szFileName );

public:
	struct PlayerLevelTableIndex{
		enum ePlayerLevelTableIndex {
			Strength,
			Agility,
			Intelligence,
			Stamina,
			Experience,
			SuperAmmor,
			SkillPoint,
			Fatigue,
			DeadDurabilityRatio,
			KillScore,
			AssistScore,
			AssistDecision,
			AggroPer,
			WeekFatigue,
			SPDecreaseRatio,
			Amount,
		};
	};

protected:
	struct LevelValue {
		union {
			int nValue[PlayerLevelTableIndex::Amount];
			float fValue[PlayerLevelTableIndex::Amount];
		};
	};
	std::map<int, LevelValue *> m_nMapTable;

public:
	CDNExtManager(void);
	~CDNExtManager(void);

	bool AllLoad();

	int GetValue( int nClassID, int nLevel, PlayerLevelTableIndex::ePlayerLevelTableIndex Index );
	float GetValueFloat( int nClassID, int nLevel, PlayerLevelTableIndex::ePlayerLevelTableIndex Index );

	TPvPGameStartConditionTable*	GetPvPGameStartConditionTable( const UINT uiMaxUser );
	TSchedule*						GetSchedule( int nSchedule );
	const TPvPGameModeTable*		GetPvPGameModeTableByMatchType( LadderSystem::MatchType::eCode MatchType );
	UINT							GetGuildWarMapInfoID(char cMonth);
	const TPvPMapTable*				GetPvPMapTable( const int nItemID );
	const TPvPGameModeTable*		GetPvPGameModeTable( const int nItemID );

	bool LoadMapInfo();

	//MapInfo
	int GetMapIndexByGateNo(int nMapIndex, int nGateNo, int nSelect=0);
	int GetGateNoByGateNo( int nMapIndex, int nGateNo, int nSelect=0);
	GlobalEnum::eMapTypeEnum GetMapType( int nMapIndex );
	const char * GetMapName(int nMapIdx);

#if defined(_LOGINSERVER) || defined(_MASTERSERVER)
	void SetOnOffServerInfo(char cWorldSetID, bool bOnline);

	int GetWorldID();
	WCHAR* GetWorldName(int nWorldID);
	bool GetLocalIp(OUT std::vector<std::string>& Out);
#endif	// #if defined(_LOGINSERVER) || defined(_MASTERSERVER)

#if defined(_LOGINSERVER)
	void GetChannelListByMapIndex(char cWorldID, int nMapIndex, int nLastMapIndex, sChannelInfo *ChannelArray, BYTE &cCount, TMapChannel *ChannelList);	
	void GetServerList(TServerListData *ServerList, BYTE &cCount, bool DenyWorld=false);

	// DefaultCreateData
	bool LoadDefaultCreateData();
	bool LoadDWCCreateData();
	const TDWCCreateData* GetDWCCreateData(BYTE cClassID, BYTE cJobCode1 = 0, BYTE cJobCode2 = 0) const;
	int GetDWCCreateDefaultItem(const TDWCCreateData* pDWC, TCreateCharacterItem *CreateItemArray);
	bool GetDWCCreateDefaultPosition(const TDWCCreateData* pDWC, int &nPosX, int &nPosY) const;
	bool GetDWCCreateDefaultSkill(const TDWCCreateData* pDWC, int *CreateSkillArray) const;
	bool GetDWCCreateDefaultQuickSlot(const TDWCCreateData* pDWC, TQuickSlot *QuickSlotArray) const;
	bool LoadSkillData();
	TDefaultCreateData* GetDefaultCreateData(BYTE cClassID);
	int GetCreateDefaultItem(BYTE cClassID, TCreateCharacterItem *CreateItemArray);
	bool IsDefaultParts(BYTE cClassID, char cEquipType, int nPartID);
	bool IsDefaultHairColor(BYTE cClassID, DWORD dwColor);
	bool IsDefaultSkinColor(BYTE cClassID, DWORD dwColor);
	bool IsDefaultEyeColor(BYTE cClassID, DWORD dwColor);

	bool GetCreateDefaultPosition(BYTE cClassID, int &nPosX, int &nPosY);
	bool GetCreateDefaultMapInfo(BYTE cClassID, int &nMapIndex, int &nTutorialMapIndex, char &cTutorialGateNo);
	bool GetCreateDefaultSkill(BYTE cClassID, int *CreateSkillArray);
	bool GetCreateDefaultQuickSlot(BYTE cClassID, TQuickSlot *QuickSlotArray);
	bool GetCreateDefaultGesture(BYTE cClassID, int * CreateGestureArr);
	bool GetCreateDefaultUnlockSkill(BYTE cClassID, int *CreateUnlockSkillArray);

	// DefaultCreateData

	// CoinCountByLevel
	bool LoadCoinCount();
	int GetRebirthCoin(BYTE cLevel, char cWorldID);
	int GetCashRebirthCoin(BYTE cLevel, char cWorldID);

	bool LoadLevelData();
	int GetFatigue(char cClass, char cLevel);

	// Forbidden word
	bool LoadProhibitWord();
	bool CheckProhibitWord(const wchar_t *pwszWord);
	bool CheckSpecialCharacter(const wchar_t *pwszWord);

	bool LoadWeaponData();
	TWeaponData* GetWeaponData(int nWeaponIndex);

	bool LoadPartData();
	TPartData* GetPartData(int nPartIndex);

	int GetItemDurability( int nItemID );
#endif

	// PCBang
	bool LoadPCBangData();
	int GetPCBangDataCount();
	TPCBangData *GetPCBangData(int nPCBangType);

	int GetPCBangParam1(int nPCBangType, int nPCBangGrade);	// When there is only 1 data row: (PCBang::Type::)Exp, Appellation, Fatigue, RebirthCoin, Mission
	int GetPCBangParam2(int nPCBangType, int nPCBangGrade);	// When there is only 1 data row: (PCBang::Type::)Exp, Appellation, Fatigue, RebirthCoin, Mission
	int GetPCBangNestClearCount(int nPCBangGrade, int nMapID);
	bool GetPCBangClearBoxFlag(int nPCBangGrade);

	// Cash side
	bool ReLoadCashCommodity();
	bool ReLoadCashPackage();

	bool GetCashCommodityData(int nSN, TCashCommodityData &Data);
	int GetCashCommodityPrice(int nSN);
	bool IsReserveCommodity(int nSN);	// Whether the product is eligible for reward points
	bool IsPresentCommodity(int nSN);	// Whether item is giftable
	bool IsOnSaleCommodity(int nSN);	// Whether the item is available for sale
	bool IsLimitCommodity(int nSN);		// Whether it is a limited sale
	int GetCashCommodityPeriod(int nSN);
	int GetCashCommodityCount(int nSN);
	int GetCashCommodityReserve(int nSN);	// Reward points granted on purchase
	bool GetCashCommodityName(int nSN, std::string &outStr);
	bool GetCashCommodityNameW(int nSN, std::wstring &outStr);
	int GetCashCommodityItem0(int nSN);
	int GetCashCommodityVIPPoint(int nSN);	// Get vippoint
	bool GetCashCommodityPay(int nSN);
	int GetCashCommodityPaySale(int nSN);
	int GetCashCommodityVIPAutomaticPaySalePrice(int nSN);	// Discount price for VIP auto-billing
	std::vector<int> m_VecProhibitSaleList;
	void SetProhibitSaleList();
#if defined(PRE_ADD_CASH_REFUND)
	bool GetCashCommodityNoRefund(int nID, int nSN); // Cash non-refundable
#endif
#if defined(PRE_ADD_SALE_COUPON)
	bool GetCashCommodityUseCoupon(int nSN);
#endif

	bool GetCashPackageData(int nSN, TCashPackageData &Data);
	TCashLimitData *GetCashLimitData(int nSN);
	int GetCashBuyAbleCount(int nSN);
#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)
	int GetCashLimitedItemCount(int nSN);
#endif		//#if defined(PRE_ADD_LIMITED_CASHITEM) && defined(_CASHSERVER)

	TItemData* GetItemData(int nItemID);
	int GetItemMainType(int nItemID);		// Whether item type is eItemTypeEnum
	int GetItemOverlapCount(int nItemID);
	bool IsCashItem(int nItemID);

	// GlobalWeight
	TGlobalWeightData *GetGlobalWeightData(int nID);
	float GetGlobalWeightValueFloat(int nID);
	int GetGlobalWeightValue(int nID);
	TGlobalWeightIntData *GetGlobalWeightIntData(int nID);
	float GetGlobalWeightIntValueFloat(int nID);
	int GetGlobalWeightIntValue(int nID);

	// VehicleTable
	TVehicleData *GetVehicleData(int nItemID);
	int GetVehicleClassID(int nItemID);

	// VehiclePartsTable
	TVehiclePartsData *GetVehiclePartsData(int nItemID);
	int GetVehiclePartsType(int nItemID);
	TMailTableData*		GetMailTableData( int iItemID );
	TGuildWarRewardData* GetGuildWarRewardData(char cType, char cClass);
	TJobTableData* GetJobTableData( int nJobTableID );

	time_t GetTimeForTextDate(std::string& strDate);  // Get time_t from "00/00/00" text
	bool IsOnSaleDate(int nSN);

	void GetXMLAccountDBID(UINT nAccountDBID, std::wstring &wstrOut);
	void GetXMLAccountName(WCHAR *pAccountName, std::wstring &wstrOut);
	void GetXMLCharacterDBID(INT64 biCharacterDBID, std::wstring &wstrOut);
	void GetXMLCharacterName(WCHAR *pCharacterName, std::wstring &wstrOut);
	void GetXMLItems(int nItemTotalCount, TSpecialBoxItemInfo *Items, std::wstring &wstrOut);


};

extern CDNExtManager *g_pExtManager;