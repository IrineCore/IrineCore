#pragma once


#ifdef PRE_ADD_CASHREMOVE


#include "DnInventory.h"

class CDnItem;

class CDnCashRemoveInventory : public CDnInventory
{
private:

	std::map< INT64, __time64_t > m_mapBeginExpireTimes; // Time when item entered the cash pending tab - for calculating wait time.

public:
	CDnCashRemoveInventory(void);
	virtual ~CDnCashRemoveInventory(void);

protected:

public:

	// Override - CDnInventory //
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ){} // Overridden because item CoolTime is not ticked.


	void CreateCashItem( TItem &item, int nSlotIndex, __time64_t beginTime, __time64_t expireTime, bool bExpireComplete );

	// bMoving - whether moving within inventory.
	// Moving items within inventory, equipment, or storage on the server actually uses a different message.
	virtual bool InsertItem( CDnItem *pItem, bool bMoving = false );
	virtual bool RemoveItem( int nSlotIndex );

	virtual void SetUsableSlotCount( int nCount );

	void DecreaseDurability( int nValue );
	void DecreaseDurability( float fValue );

	__time64_t GetTimeBySN( INT64 nSerial );
};


#endif