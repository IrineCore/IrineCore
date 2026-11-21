#include "StdAfx.h"
#include "DnGuildInventory.h"
#include "DnItem.h"
#include "DnInventoryDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnGuildInventory::CDnGuildInventory(void)
{
}

CDnGuildInventory::~CDnGuildInventory(void)
{
}

bool CDnGuildInventory::InsertItem( CDnItem *pItem, bool bMoving )
{
	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnGuildInventory::InsertItem, m_pInvenDlg is NULL!" );
		return false;
	}

	//Add defensive code for dump
	if (pItem == NULL)
		return false;
	
	if( GetItem( pItem->GetSlotIndex() ) ) 
	{
		RemoveItem( pItem->GetSlotIndex() );
	}

	// Sound playback is only triggered on server response or client-side UI actions (e.g. picking up).
	//CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );

	return true;
}

bool CDnGuildInventory::RemoveItem( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( iter != m_mapInventory.end() )
	{
		m_pInvenDlg->ResetSlot( iter->second );

		//CEtSoundEngine::GetInstance().PlaySound( "2D", iter->second->GetDragSoundIndex() );

		SAFE_DELETE( iter->second );
		m_mapInventory.erase( iter );

		return true;
	}

	return false;
}

void CDnGuildInventory::CheckInvalidSlot( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( iter != m_mapInventory.end() )
	{
		CDnItem *pInvalidItem = iter->second;
		if( pInvalidItem )
		{
			int nEmptySlotIndex = m_pInvenDlg->GetEmptySlot();
			if( nEmptySlotIndex != -1 )
			{
				m_pInvenDlg->ResetSlot( pInvalidItem );
				m_mapInventory.erase( iter );
				pInvalidItem->SetSlotIndex( nEmptySlotIndex );
				m_mapInventory[nEmptySlotIndex] = pInvalidItem;
				m_pInvenDlg->SetItem( pInvalidItem );
			}
		}
	}
}