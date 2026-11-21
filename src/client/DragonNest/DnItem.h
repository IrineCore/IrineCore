#pragma once

#include "DnActorState.h"
#include "MIInventoryItem.h"
#include "DNItemTypeDef.h"
#include "DNTableFile.h"

const int NUM_DISJOINT_DROP = 2;

class CDnItem : public CDnState, public MIInventoryItem {
public:
	CDnItem();
	virtual ~CDnItem();

	enum ItemReversionEnum {
		NoReversion,
		Belong,
		Trade,
		GuildBelong,
		OnlyTradeWorldStorage,

		ItemReversionEnum_Amount,
	};


	// Use the one in DnCommonDef.h to share with the server
	// Intended to just use DnCommonDef.h.. but there is already too much existing code, so lazy redefine
	enum ItemSkillApplyType
	{
		None = Item::SkillUsingType::None,
		ApplyStateBlow = Item::SkillUsingType::ApplyStateBlow,
		ApplySkill = Item::SkillUsingType::ApplySkill,
		Eternity = Item::SkillUsingType::Eternity,					// Permanent effect item
		SkillLevelUp = Item::SkillUsingType::SkillLevelUp,			// Skill level-up item
		PrefixSkill = Item::SkillUsingType::PrefixSkill,			// Item for prefix skill
	};

	struct DropItemStruct {
		int nItemID;
		int nSeed;
		char cOption;
		int nCount;
		DWORD dwUniqueID;
	};

	static int s_nStateValueMatchUIStringIndex[];
	static int GetStateValueMatchUIStringIndex(int nIndex);
	static bool IsEffectSkillItem(int nItemType, bool bCheckVisible = false);
	static bool IsVisualChangeItem(int nItemType);

protected:
	int m_nSeed;
	int m_nClassID;

	tstring m_szName;
	eItemTypeEnum m_ItemType;
	eItemRank m_Rank;
	ItemReversionEnum m_Reversion;
	ItemSkillApplyType m_SkillApplyType;
	int m_nLevelLimit;
	bool m_bIsCashItem;

#ifdef PRE_ADD_CASHREMOVE	
	__time64_t m_tExpireTime; // Pending deletion time.
	bool m_bIsCashRemoveItem; // Whether cash item is deleted.
#endif

	bool m_bNeedAuthentication;
	bool m_bCanDestruction;
	int m_nAmount;
	int m_nNeedBuyLadderPoint;  // Required ladder points for purchase
	int m_nNeedBuyGuildWarPoint;
	int m_nNeedBuyUnionPoint;
	int m_nMaxOverlapCount;
	int m_nOverlapCount;
	int m_nSellAmount;

	int m_nRootSoundIndex;
	int m_nDragSoundIndex;
	int m_nUseSoundIndex;

	DnSkillHandle m_hSkill;
	int m_nSkillID;
	int m_nSkillLevel;
	int m_nTypeParam[2];
	int m_nExchangeType;

	INT64 m_nSerialID;

	std::vector<int> m_nVecPermitJobList;

	float m_fCoolTime;		// Variable to use for external display in UI.. (clamped value 0.0 ~ 1.0)
	float m_fDelayTime;
	float m_fElapsedDelayTime;

	bool m_bNewGain;

	bool m_bAuthentication;
	bool m_bIdentity;

	int m_nEnchantTableID;
	int m_nEnchantID;
	char m_cEnchantLevel;
	char m_cMaxEnchantLevel;
	char m_cPotentialIndex;
	char m_cOptionIndex;
	bool m_bSoulBound;
	char m_cSealCount;
	int m_nSealID;
	bool	m_bEternity;
	time_t	m_ExpireDate;
	int m_nLookItemID;
#ifdef PRE_ADD_DMIX_DESIGNNAME
	tstring m_szLookItemDescriptionCache;
#endif // PRE_ADD_DMIX_DESIGNNAME
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	char m_cPotentialMoveCount;
#endif
#ifdef PRE_ADD_REDUCE_TABLE_RESOURCE
	int	m_nOptionSkillID;
	int m_nOptionSkillLevel;
	int m_nOptionSkillUsingType;
#endif // PRE_ADD_REDUCE_TABLE_RESOURCE

	CDnState *m_pEnchant;
	CDnState *m_pPotential;

	bool m_bCanDisjoint;
	int m_nDisjointType;
	int m_nDisjointCost;
	int m_nDisjointDropTableID[NUM_DISJOINT_DROP];
#if defined(PRE_FIX_69709)
	int m_nEnchantDisjointDrop;
#endif

	int m_nAllowedGameType;

	int m_nNeedBuyItem;			// Required item for purchase
	int m_nNeedBuyItemCount;	// Required item count for purchase
	int m_nNeedPvpRank;			// Required PVPRank to equip

	bool m_bShowAlarmDialog;

	static std::map<INT64, CDnItem *> s_nMapSerialSearch;
	static CSyncLock s_SerialSearchLock;

	bool m_bEnableCostumeMix;
	int m_nCashTradeCount;

	bool m_bInfoItem;

	int m_nApplicableValue;
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	int m_nCategoryType;
#endif
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	bool m_bWstorage;
#endif
#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
	bool m_bUseLimited;
#endif


protected:
	static int GetRandomStateNumber(int nMin, int nMax);
	static float GetRandomStateRatio(float fMin, float fMax);

	bool IsPossibleProb(DNTableFileFormat* pSox, const char *szLabel);
	static void DummyRandom(int nCount = 1);

	void InitializeEnchant();
	void InitializeOption();
	void InitializePotential();

	static void InsertSerialSearchMap(CDnItem *pItem);
	static void RemoveSerialSearchMap(CDnItem *pItem);

	static int GetOptionTableID(int nItemTableID, char cOptionIndex);

public:
	virtual bool Initialize(int nTableID, int nSeed, char cOptionIndex = 0, char cEnchantLevel = 0, char cPotentialIndex = 0, char cSealCount = 0, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE);

	// Init function with only essential info to view basic details. Currently used in gacha.
	bool InitializeForInformation(int nTableID, int nSeed, char cOptionIndex);

#ifdef PRE_ADD_DMIX_DESIGNNAME
	TCHAR* GetName() const;
	const TCHAR *GetDescription();
#else
	TCHAR* GetName() const { return (TCHAR*)m_szName.c_str(); }
#endif
	virtual int GetClassID() const { return m_nClassID; }
	int GetSeed() const { return m_nSeed; }

	int GetMaxOverlapCount() { return m_nMaxOverlapCount; }
	int GetItemAmount() const { return m_nAmount; }
	int GetItemSellAmount() const { return m_nSellAmount; }
	int GetLevelLimit() const { return m_nLevelLimit; }

	void SetOverlapCount(int nValue) { m_nOverlapCount = nValue; }
	int GetOverlapCount() const { return m_nOverlapCount; }

	INT64 GetSerialID() const { return m_nSerialID; }
	void SetSerialID(INT64 nSerial);

	eItemTypeEnum GetItemType() const { return m_ItemType; }
	eItemRank GetItemRank() const { return m_Rank; }
	ItemReversionEnum GetReversion() const { return m_Reversion; }
	bool IsNeedAuthentication() { return m_bNeedAuthentication; }
	bool IsCanDestruction() { return m_bCanDestruction; }
	bool IsCashItem() const { return m_bIsCashItem; }

#ifdef PRE_ADD_CASHREMOVE
	void SetCashRemoveItem(bool bRemove) {
		m_bIsCashRemoveItem = bRemove;
	} // Whether cash item is deleted.
	bool IsCashRemoveItem() const { return m_bIsCashRemoveItem; } // Whether cash item is deleted.

	void SetExpireTime(__time64_t _time) { m_tExpireTime = _time; }
	__time64_t GetExpireTime() { return m_tExpireTime; }
#endif

	bool IsEnableCostumeMixItem() const { return m_bEnableCostumeMix; }
	int GetLookItemID() const { return m_nLookItemID; }
	void SetLookItemID(int nLookItemID, bool bChangeIconIdx);
#ifdef PRE_ADD_DMIX_DESIGNNAME
	void MakeLookItemDescription(DNTableFileFormat* pTableFormat);
#endif // PRE_ADD_DMIX_DESIGNNAME

	int GetRootSoundIndex() { return m_nRootSoundIndex; }
	int GetDragSoundIndex() { return m_nDragSoundIndex; }
	int GetUseSoundIndex() { return m_nUseSoundIndex; }

	void PlayInstantUseSound();

	static bool IsPermitPlayer(const int nItemID, const std::vector<int> &nVecJobList);
	bool IsNeedJob() const;
	bool IsPermitPlayer(const std::vector<int> &nVecJobList) const;
	int GetNeedJobCount() const { return (int)m_nVecPermitJobList.size(); }
	int GetNeedJob(int nIndex) const;

	bool ActivateSkillEffect(DnActorHandle hActor, bool bPickUp = false);
	bool DeactivateSkillEffect();

	static CDnItem *CreateItem(int nTableID, int nSeed, bool bSoulBound = false, int nLookItemID = ITEMCLSID_NONE);
	static eItemTypeEnum GetItemType(int nItemTableID);
	static int GetItemTypeParam(int nItemTableID, int nIndex = 0);
	static tstring GetItemName(int nItemTableID);
	static tstring GetItemFullName(int nItemTableID, char cOptionIndex = 0);
	static bool IsCashItem(int nTableID);
	int  GetAllowedGameType() { return m_nAllowedGameType; }

	void SetElapsedDelayTime(float fDelayTime) { m_fElapsedDelayTime = fDelayTime; };
#ifdef PRE_ADD_SKILLCOOLTIME
	float GetElapsedDelayTime() const { return m_fElapsedDelayTime; }
#endif

	// from MIInventoryItem
	virtual float GetCoolTime() { return m_fCoolTime; };
	virtual ItemConditionEnum GetItemCondition();

	void ProcessCoolTime(LOCAL_TIME LocalTime, float fDelta);
	int GetSkillID(void) { return m_nSkillID; };
	int GetSkillLevel(void) { return m_nSkillLevel; };
	int GetSkillLevelID(void);
	void BeginCoolTime(void);
	virtual int GetClickedEquipPage();

	bool IsNewGain() { return m_bNewGain; }

	// bExistItem
	// false if moving into a new slot,
	// true if going into an existing slot (potion count increased)
	void SetNewGain(bool bNew, bool bExistItem = false);

	virtual bool IsQuickSlotItem();

	//	DWORD GetColor() const { return m_dwColor; }
	//	void SetColor( DWORD dwColor ) { m_dwColor = dwColor; }
	bool IsAuthentication() { return m_bAuthentication; }
	void SetAuthentication(bool bValue) { m_bAuthentication = bValue; }
	bool IsIdentity() { return m_bIdentity; }
	void SetIdentity(bool bValue) { m_bIdentity = bValue; }

	int GetEnchantTableID() { return m_nEnchantTableID; }
	int GetEnchantID() { return m_nEnchantID; }
	bool IsActiveEnchant() { return (m_cEnchantLevel == 0) ? false : true; }
	bool IsActivePotential() { return (m_cPotentialIndex == 0) ? false : true; }
	char GetEnchantLevel() const { return m_cEnchantLevel; }
	void SetEnchantLevel(char nLevel) { m_cEnchantLevel = nLevel; }
	char GetMaxEnchantLevel() { return m_cMaxEnchantLevel; }
	char GetPotentialIndex() const { return m_cPotentialIndex; }
	char GetOptionIndex() const { return m_cOptionIndex; }
	int GetTypeParam(int nIndex = 0) const { return m_nTypeParam[nIndex]; };
	void SetTypeParam(int nIndex, int nValue) { m_nTypeParam[nIndex] = nValue; }
	int GetExchangeType() { return m_nExchangeType; }
	bool IsSoulbBound() const;
	char GetSealCount() const { return m_cSealCount; }
	void SetSoulBound(bool bSet) { m_bSoulBound = bSet; }
	int GetSealID() { return m_nSealID; }
	void SetSealCount(char cCount) { m_cSealCount = cCount; } // Shouldn't use this.. because of tooltips..
#ifdef PRE_ADD_EXCHANGE_POTENTIAL
	int GetPotentialMoveCount() const { return m_cPotentialMoveCount; }
	bool IsPotentialExtractableItem();
#endif
#if defined(PRE_ADD_PCBANG_RENTAL_ITEM)
	bool IsPcCafeRentItem();
#endif
#ifdef PRE_ADD_ITEMCAT_TOOLTIP_INFO
	int GetCategoryType() const { return m_nCategoryType; }
#endif
#if defined(PRE_ADD_SERVER_WAREHOUSE)
	int IsWstorage() const { return m_bWstorage; }
#endif

	bool IsEternityItem() const;
	void SetEternityItem(bool bEternity);
	const time_t* GetExpireDate() const;
	void SetExpireDate(const time_t& date);

	CDnState *GetEnchantState() { return m_pEnchant; }
	CDnState *GetPotentialState() { return m_pPotential; }

	int GetDisjointType() { return m_nDisjointType; }
	bool CanDisjointType(int nDisjointType);
	bool CanDisjoint(void);
	int GetDisjointCost(void) { return m_nDisjointCost; };

	int GetNeedBuyItem() { return m_nNeedBuyItem; }
	int GetNeedBuyItemCount() { return m_nNeedBuyItemCount; }
	int GetNeedPvpRank() const { return m_nNeedPvpRank; }
	int GetNeedBuyLadderPoint() { return m_nNeedBuyLadderPoint; }
	int GetNeedBuyGuildWarPoint() { return m_nNeedBuyGuildWarPoint; }
	int GetNeedBuyUnionPoint() { return m_nNeedBuyUnionPoint; }

	int GetApplicableValue() { return m_nApplicableValue; }

	//// Pre-generate items to drop on disassemble
	//void GenerationDropItem( void ); 
	//bool Disjoint( void );

	//
	static CDnItem *FindItemFromSerialID(INT64 nSerialID);
	static void CalcStateValue(CDnState *pState, int nStateType, char *szMin, char *szMax, bool bRandomValue = true, bool bResetValue = true, float fRatio = 1.0f);
#ifdef PRE_FIX_MEMOPT_ENCHANT
	static void CalcStateValue(CDnState *pState, int nStateType, float fMin, float fMax, int nMin, int nMax, bool bRandomValue = true, bool bResetValue = true);
#endif

	int GetTotalSealCount(int nSealItemTypeParam);
	bool GetCharmItemKeyID(std::vector<int> &vecKeyID);

	float GetDelayTime(void) { return m_fDelayTime; }

	bool IsShowAlarmDialog() { return m_bShowAlarmDialog; }

	virtual CDnItem &operator = (TItem &e);

	// When registering an inventory item to another window, use this TItemInfo with CreateItem to create a copy.
	virtual void GetTItemInfo(TItemInfo &Info);

	// Use this function to show items that will be received rather than owned items, such as in mission, reward, or item crafting windows.
	static bool MakeItemInfo(int nItemID, int nCount, TItemInfo &ItemInfo);
	static bool MakeItemInfo(int nItemID, int nCount, TItem &ItemInfo);
	static void MakeItemDurationString(int iDurationTimeByMS, /*IN OUT*/ wstring &strDurationTime);

	int GetSkillApplyType(void) const { return m_SkillApplyType; };
	bool IsInfoItem() const { return m_bInfoItem; }
	void SetInfoItem(bool bInfoItem);

	//#ifdef PRE_ADD_GACHA_JAPAN
	//	const CDnState* GetOptionState( void ) { return &m_OptionState; };
	//#endif // PRE_ADD_GACHA_JAPAN

	bool HasPrefixSkill();
	bool HasPrefixSkill(int &nSkillID, int &nSkillLevel);
	bool HasLevelUpInfo(int &nSkillID, int &nSkillLevel, int &nSkillUsingType);

	const TCHAR* GetSkillDescription(void);
	int GetSkillSEDurationTime(void);
	void ChangeToSkillIconIndex(void);

	int GetSkillEffectIconIndex(bool bBuff = true);
#ifdef PRE_ADD_SERVER_WAREHOUSE
	bool IsTradable(bool bIsWStorage) const;
#endif

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
	bool IsUseLimited() const { return m_bUseLimited; }
	void SetUseLimit(bool bFlag) { m_bUseLimited = bFlag; }
#endif
public:
	void ChangeIconIndex(int nIconIndex) { m_nIconImageIndex = nIconIndex; m_Reversion = NoReversion; }
	void ChangeRank(eItemRank itemRank) { m_Rank = itemRank; m_Reversion = NoReversion; }
	void ChangeItemType(eItemTypeEnum itemType) { m_ItemType = itemType; m_Reversion = NoReversion; }
};
