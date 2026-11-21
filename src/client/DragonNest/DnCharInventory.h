#pragma once
#include "DnInventory.h"

class CDnItem;

class CDnCharInventory : public CDnInventory
{
public:
	enum emINVEN_TYPE
	{
		typeItemInven,
		typeQuestInven,
	};

public:
	CDnCharInventory(void);
	virtual ~CDnCharInventory(void);

protected:
	emINVEN_TYPE m_emInvenType;

public:
	void SetInvenType( emINVEN_TYPE nType ) { m_emInvenType = nType; }

public:
	// bMoving - whether moving within inventory.
	// Moving items within inventory, equipment, or storage on the server actually uses a different message.
	virtual bool InsertItem( CDnItem *pItem, bool bMoving = false );
	virtual bool RemoveItem( int nSlotIndex );

	virtual void SetUsableSlotCount( int nCount );

	void DecreaseDurability( int nValue );
	void DecreaseDurability( float fValue );
};