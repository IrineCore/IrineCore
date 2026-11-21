
#pragma once

class CDNEffectItem
{
public:

	CDNEffectItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate );
	virtual ~CDNEffectItem(){}
	bool Add();
	bool Remove();
	const TItemData* GetItemData(){ return m_pItemData; }
	__time64_t GetExpireDate(){ return m_tExpireDate; }

private:

	virtual bool OnAdd() = 0;
	virtual bool OnRemove() = 0;

protected:

	CDNUserSession*	m_pSession;
	INT64 m_nItemSerial;
	TItemData* m_pItemData;
	__time64_t m_tExpireDate;
};

// Inventory expansion cash item
class CDNInventoryExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNInventoryExtendItem>
{
public:
	CDNInventoryExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNInventoryExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

#if defined(PRE_PERIOD_INVENTORY)
// Timed inventory expansion cash item
class CDNPeriodInventoryExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNPeriodInventoryExtendItem>
{
public:
	CDNPeriodInventoryExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData, tExpireDate){}
	virtual ~CDNPeriodInventoryExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};
#endif	// #if defined(PRE_PERIOD_INVENTORY)

// Crest expansion cash item
class CDNGlyphExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNGlyphExtendItem>
{
public:
	CDNGlyphExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNGlyphExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

#if defined(PRE_ADD_TALISMAN_SYSTEM)
class CDNTalismanExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNTalismanExtendItem>
{
public:
	CDNTalismanExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNTalismanExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

// Storage inventory expansion cash item
class CDNWareHouseInventoryExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNWareHouseInventoryExtendItem>
{
public:
	CDNWareHouseInventoryExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNWareHouseInventoryExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

#if defined(PRE_PERIOD_INVENTORY)
// Timed storage inventory expansion cash item
class CDNPeriodWarehouseExtendItem:public CDNEffectItem,public TBoostMemoryPool<CDNPeriodWarehouseExtendItem>
{
public:
	CDNPeriodWarehouseExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNPeriodWarehouseExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};
#endif	// #if defined(PRE_PERIOD_INVENTORY)

// Cash item for adding gesture
class CDNGestureAddItem:public CDNEffectItem,public TBoostMemoryPool<CDNGestureAddItem>
{
public:
	CDNGestureAddItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNGestureAddItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

// Guild warehouse expansion cash item
class CDNGuildWareSlotExtendItem : public CDNEffectItem, public TBoostMemoryPool<CDNGuildWareSlotExtendItem>
{
public:
	CDNGuildWareSlotExtendItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNGuildWareSlotExtendItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

// Union affinity item
class CDNUnionMembershipItem : public CDNEffectItem, public TBoostMemoryPool<CDNUnionMembershipItem>
{
public:
	CDNUnionMembershipItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNUnionMembershipItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

#if defined( PRE_ADD_VIP_FARM )

// Farm VIP
class CDNFarmVipItem : public CDNEffectItem, public TBoostMemoryPool<CDNFarmVipItem>
{
public:
	CDNFarmVipItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNFarmVipItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

#endif // #if defined( PRE_ADD_VIP_FARM )

class CDNPeriodAppellationItem : public CDNEffectItem, public TBoostMemoryPool<CDNUnionMembershipItem>
{
public:
	CDNPeriodAppellationItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNPeriodAppellationItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

class CDNExpandSkillPageItem : public CDNEffectItem, public TBoostMemoryPool<CDNUnionMembershipItem>
{
public:
	CDNExpandSkillPageItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate ):CDNEffectItem(pSession, nItemSerial, pItemData,tExpireDate){}
	virtual ~CDNExpandSkillPageItem(){}
private:
	virtual bool OnAdd();
	virtual bool OnRemove();
};

class CDNSourceItem : public CDNEffectItem, public TBoostMemoryPool<CDNSourceItem>
{
public:
	CDNSourceItem(CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t expireDate) : CDNEffectItem(pSession,  nItemSerial, pItemData, expireDate) {}
	virtual ~CDNSourceItem() {}

private:
	virtual bool OnAdd() override;
	virtual bool OnRemove() override;
};

#if defined (PRE_ADD_NAMEDITEM_SYSTEM )
class CDNEffectSkillItem : public CDNEffectItem, public TBoostMemoryPool<CDNEffectSkillItem>
{
public:
	CDNEffectSkillItem(CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t expireDate) : CDNEffectItem(pSession,  nItemSerial, pItemData, expireDate) {}
	virtual ~CDNEffectSkillItem() {}

private:
	virtual bool OnAdd() override;
	virtual bool OnRemove() override;
};
#endif // #if defined (PRE_ADD_NAMEDITEM_SYSTEM)

#if defined (PRE_ADD_BESTFRIEND )
class CDNEffectBestFriendBufftem : public CDNEffectItem, public TBoostMemoryPool<CDNEffectBestFriendBufftem>
{
public:
	CDNEffectBestFriendBufftem(CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t expireDate) : CDNEffectItem(pSession,  nItemSerial, pItemData, expireDate) {}
	virtual ~CDNEffectBestFriendBufftem() {}

private:
	virtual bool OnAdd() override;
	virtual bool OnRemove() override;
};
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
class CDNEffectTotalLevelSkilltem : public CDNEffectItem, public TBoostMemoryPool<CDNEffectTotalLevelSkilltem>
{
public:
	CDNEffectTotalLevelSkilltem(CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t expireDate) : CDNEffectItem(pSession,  nItemSerial, pItemData, expireDate) {}
	virtual ~CDNEffectTotalLevelSkilltem() {}

private:
	virtual bool OnAdd() override;
	virtual bool OnRemove() override;
};
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
class CDNEffectComebackItem : public CDNEffectItem, public TBoostMemoryPool<CDNEffectComebackItem>
{
public:
	CDNEffectComebackItem(CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t expireDate) : CDNEffectItem(pSession,  nItemSerial, pItemData, expireDate) {}
	virtual ~CDNEffectComebackItem() {}

private:
	virtual bool OnAdd() override;
	virtual bool OnRemove() override;
};
#endif
