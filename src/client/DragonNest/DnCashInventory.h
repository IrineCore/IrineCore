#pragma once
#include "DnInventory.h"

class CDnItem;

class CDnCashInventory : public CDnInventory
{
public:
	CDnCashInventory(void);
	virtual ~CDnCashInventory(void);

protected:

public:
	void CreateCashItem( TItem &item, int nSlotIndex );

	// bMoving - whether moving within inventory.
	// Moving items within inventory, equipment, or storage on the server actually uses a different message.
	virtual bool InsertItem( CDnItem *pItem, bool bMoving = false );
	virtual bool RemoveItem( int nSlotIndex );

	virtual void SetUsableSlotCount( int nCount );

	void DecreaseDurability( int nValue );
	void DecreaseDurability( float fValue );
};