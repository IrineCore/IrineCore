#pragma once
#include "DNPacket.h"
#include "DnItem.h"

class CDnItemTask;
class CDnInventoryDlg;

class CDnInventory
{
public:
	typedef std::map<int,CDnItem*>		INVENTORY_MAP;
	typedef INVENTORY_MAP::iterator		INVENTORY_MAP_ITER;
	typedef INVENTORY_MAP::const_iterator	INVENTORY_MAP_ITER_CONST;

	// Following cash inventory improvements, the server no longer processes based on index.
	// However, on the client side, the inventory shape is the same as a normal inventory, and
	// Because we need to preserve the existing code to some extent,
	// Will use std::map with slot index as the key directly.

	CDnInventory(void);
	virtual ~CDnInventory(void);

protected:
	INVENTORY_MAP m_mapInventory;
	int m_nUsableSlotCount;			// Note : Number of available storage slots
	
	CDnInventoryDlg *m_pInvenDlg;
	CDnItemTask *m_pItemTask;

	// Temporary data used in FindFirstEmptyIndex function
	std::vector<bool> m_vecTempCheck;

	// Sort data
	std::vector<CDnItem*> m_vecSortItem;
	TSortSlot m_SortSlotInfo[INVENTORYMAX];		// INVENTORYMAX is the largest at 150, so it has plenty of room.
	bool m_bSortRequesting;

#if defined(PRE_PERIOD_INVENTORY)
	INVENTORY_MAP m_mapNotSortItem;
	bool m_bSortPeriod;
#endif	// #if defined(PRE_PERIOD_INVENTORY)

public:
	void SetInventoryDialog( CDnInventoryDlg *pDialog ) { m_pInvenDlg = pDialog; }
	void SetItemTask( CDnItemTask *pItemTask ) { m_pItemTask = pItemTask; }

public:
	virtual bool InsertItem( CDnItem *pItem, bool bMoving = false ) = 0;
	virtual bool RemoveItem( int nSlotIndex ) = 0;

public:
	void CreateItem( TItemInfo &itemInfo );
	CDnItem *GetItem( int nSlotIndex );
	CDnItem* FindItem( int nItemTableID, char cOption = -1 );
	int FindItemSlotIndex( int nItemTableID );
	CDnItem* FindItemFromSerialID( INT64 nSerialID );
	void FindItemFromItemType( eItemTypeEnum Type, std::vector<CDnItem *> &pVecResult );
	void FindItemFromItemType( eItemTypeEnum Type, std::vector<CDnItem *> &pVecItem, std::vector<int> &pVecIndex );
	void BeginSameSkillCoolTime( const CDnItem *pItem, int nSkillID );
	int GetItemCount( int nItemTableID, char cOption = -1 );
	//blondy iterate inventory and set items that should currently be disabled
	void SetDisableItemInven( int iGameMode );
	//blondyend
	int FindItemList( int nItemTableID, char cOption, std::vector<CDnItem *> &pVecResultList );

	INVENTORY_MAP& GetInventoryItemList();

	void RefreshInventory();
	void ClearInventory();

	int ScanItemFromID( int nItemTableID, std::vector<CDnItem *> *pVecResult );
	bool RequestUseItem( int nSlotIndex, char cType = 0 );

	// Function created to find an empty slot when the cash inventory is refreshed.
	// Since it just finds an empty spot in the map without considering locked inventory slots,
	// Recommended not to use in normal inventory.
	int FindFirstEmptyIndex();

	virtual void SetUsableSlotCount( int nCount );
	int GetUsableSlotCount() const { return m_nUsableSlotCount; }
	// Calculate repair cost of items in inventory. Currently used only in CharInven.
	int GetRepairInvenPrice();

	// Get item by serial ID or similar. Currently unused.
	//CDnItem *GetItemFromSerialID( INT64 nSerialID );

	// Sort
	bool PrepareSort( bool bPeriod = false );
	bool Sort( bool bServerSucceed );
	TSortSlot *GetSortArray() { return m_SortSlotInfo; }
	int GetSortCount() { return (int)m_vecSortItem.size(); }

	// Sort for cash items. Keep separate from normal items.
	bool SortCashInventory();

public:

#ifdef PRE_ADD_CASHREMOVE
	// Changed to virtual function for Dlg that does not tick item CoolTime.
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
#else
	void Process( LOCAL_TIME LocalTime, float fDelta );
#endif

};
