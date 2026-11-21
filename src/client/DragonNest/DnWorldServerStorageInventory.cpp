#include "StdAfx.h"

#ifdef PRE_ADD_SERVER_WAREHOUSE

#include "DnWorldServerStorageInventory.h"
#include "DnItem.h"
#include "DnItemTask.h"
#include "DnWorldServerStorageInventoryDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnWorldServerStorageInventory::CDnWorldServerStorageInventory(void)
{
}

CDnWorldServerStorageInventory::~CDnWorldServerStorageInventory(void)
{
}

void CDnWorldServerStorageInventory::CreateItem(TItem &item, int nSlotIndex)
{
	if (!m_pInvenDlg)
	{
		CDebugSet::ToLogFile( "CDnWorldServerStorageInventory::CreateItem, m_pInvenDlg is NULL!" );
		return;
	}

	if (!m_pItemTask)
	{
		CDebugSet::ToLogFile("CDnWorldServerStorageInventory::CreateItem, m_pItemTask is NULL!");
		return;
	}

	CDnItem *pItem = m_pItemTask->CreateItem(item);

	if (pItem == NULL)
	{
		CDebugSet::ToLogFile("CDnWorldServerStorageInventory::CreateItem, failed to create item (%d)!", item.nItemID);
		return;
	}

	pItem->SetSlotIndex(nSlotIndex);
	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem(pItem);
}

bool CDnWorldServerStorageInventory::InsertItem( CDnItem *pItem, bool bMoving )
{
	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnStorageInventory::InsertItem, m_pInvenDlg is NULL!" );
		return false;
	}

	int nExistCnt = GetItemCount(pItem->GetClassID());

	// Check whether only the count of an already existing item changed
	if (FindItemFromSerialID(pItem->GetSerialID())) 
	{
		INVENTORY_MAP_ITER iter = m_mapInventory.find( pItem->GetSlotIndex() );
		if( iter != m_mapInventory.end() )
		{
			m_pInvenDlg->ResetSlot( iter->second );

			SAFE_DELETE( iter->second );
			m_mapInventory.erase( iter );
		}
	}

	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );

	return true;
}

bool CDnWorldServerStorageInventory::RemoveItem( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if (iter != m_mapInventory.end())
	{
		m_pInvenDlg->ResetSlot( iter->second );

		SAFE_DELETE( iter->second );
		m_mapInventory.erase( iter );

		return true;
	}

	return false;
}

int CDnWorldServerStorageInventory::GetMaxStorageCount(bool bCash) const
{
	int nMaxInvenCount = 0;
	if (bCash)
		nMaxInvenCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_CASHCOUNT);
	else
		nMaxInvenCount = (int)CGlobalWeightTable::GetInstance().GetValue(CGlobalWeightTable::SERVER_WAREHOUSE_INVENCOUNT);

	return nMaxInvenCount;
}

#endif