#include "StdAfx.h"
#include "DnStorageInventory.h"
#include "DnItem.h"
#include "DnInventoryDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnStorageInventory::CDnStorageInventory(void)
{
}

CDnStorageInventory::~CDnStorageInventory(void)
{
}

bool CDnStorageInventory::InsertItem( CDnItem *pItem, bool bMoving )
{
	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnStorageInventory::InsertItem, m_pInvenDlg is NULL!" );
		return false;
	}

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

bool CDnStorageInventory::RemoveItem( int nSlotIndex )
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