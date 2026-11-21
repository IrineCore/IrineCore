#include "StdAfx.h"
#include "DnCharInventory.h"
#include "DnItem.h"
#include "DnInventoryDlg.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "DnMasterTask.h"
#include "DnMainMenuDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCharInventory::CDnCharInventory(void)
	: m_emInvenType(typeItemInven)
{
}

CDnCharInventory::~CDnCharInventory(void)
{
}

bool CDnCharInventory::InsertItem( CDnItem *pItem, bool bMoving )
{
	if (pItem == NULL)
	{
		_ASSERT(0 && "CDnCharInventory::InsertItem - ITEM is NULL\n");
		return false;
	}

	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnCharInventory::InsertItem, m_pInvenDlg is NULL!" );
		return false;
	}

	int nExistCnt = GetItemCount(pItem->GetClassID());

	// Check whether only the count of an already existing item changed
	if( GetItem( pItem->GetSlotIndex() ) ) 
	{
		INVENTORY_MAP_ITER iter = m_mapInventory.find( pItem->GetSlotIndex() );
		if( iter != m_mapInventory.end() )
		{
			m_pInvenDlg->ResetSlot( iter->second );

			SAFE_DELETE( iter->second );
			m_mapInventory.erase( iter );
		}
	}

	// DnInvenSlotDlg already plays the sound; it would play twice.
//	CEtSoundEngine::GetInstance().PlaySound( "2D", pItem->GetDragSoundIndex() );

	// Brief note about inventory item sounds. by hayannal2009
	//
	// First, basically when there is a server response (SC_MOVEITEM, SC_REFRESHINVEN, SC_REFRESHCASHINVEN, SC_PICKUP, SC_USEITEM)
	// In principle, play an appropriate type of sound.
	// And anything else handled on the UI side is when picked up locally with the mouse, etc., which can only be done per dialog.
	//
	// Lastly, to add a supplementary note,
	// Do not play sound when receiving the packet above, and
	// If done in CharInventory Insert/Remove -> looting sound plays in SC_PICKUP and again when entering inventory, which is bad. Plus moving an item to an adjacent slot plays the sound twice.
	// For this reason it is not handled in CharInventory, so please do not add sound playback here.

	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );

	int nNowCount = GetItemCount(pItem->GetClassID());

	switch( pItem->GetItemType() ) {
		case ITEMTYPE_NORMAL:
		case ITEMTYPE_COOKING:
		case ITEMTYPE_RETURN_HOME:
			GetInterface().GetMainBarDialog()->OnRefreshQuickSlot();
			break;
	}

	// When increased beyond the originally existing items 
	int nDeltaCnt = nNowCount - nExistCnt;
	if ( nDeltaCnt > 0 && bMoving == false )
	{
		// Show alert message that more items were obtained.
		if( pItem->GetItemType() == ITEMTYPE_QUEST )
		{
			GetInterface().ShowSymbolAlarmDialog( pItem->GetName(), pItem, nDeltaCnt, textcolor::WHITE, 3.0f );
		}
		else
		{
			GetInterface().ShowItemAlarmDialog( pItem->GetName(), pItem, nDeltaCnt, textcolor::WHITE, 3.0f );
		}
	}

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg )
	{
		pMainMenuDlg->RefreshCompoundNotifyPercentage();
	}
#endif

	return true;
}

bool CDnCharInventory::RemoveItem( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( iter != m_mapInventory.end() )
	{
		// DnInvenSlotDlg already plays the sound; it would play twice.
//		CEtSoundEngine::GetInstance().PlaySound( "2D", iter->second->GetDragSoundIndex() );

		eItemTypeEnum emItemType = iter->second->GetItemType();
		m_pInvenDlg->ResetSlot( iter->second );

		SAFE_DELETE( iter->second );
		m_mapInventory.erase( iter );

		switch( emItemType ) {
			case ITEMTYPE_NORMAL:
			case ITEMTYPE_COOKING:
			case ITEMTYPE_RETURN_HOME:
				GetInterface().GetMainBarDialog()->OnRefreshQuickSlot();
				break;
		}

#ifdef PRE_ADD_NOTIFY_ITEM_COMPOUND
		CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
		if( pMainMenuDlg )
		{
			pMainMenuDlg->RefreshCompoundNotifyPercentage();
		}
#endif

		return true;
	}

	return false;
}

void CDnCharInventory::SetUsableSlotCount( int nCount )
{ 
	if (m_pInvenDlg == NULL) return;
	m_nUsableSlotCount = nCount;

	if( m_emInvenType == typeItemInven )
	{
		m_pInvenDlg->SetUseItemCnt( nCount );
	}
	else
	{
		m_pInvenDlg->SetUseQuestItemCnt( nCount );
	}
}


void CDnCharInventory::DecreaseDurability( int nValue )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		CDnItem *pItem = iter->second;
		if( !pItem ) continue;
		switch( pItem->GetItemType() ) {
			case ITEMTYPE_WEAPON:
				{
					CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
					if( !pWeapon ) break;
					if( pWeapon->IsInfinityDurability() ) break;
					if( pWeapon->GetDurability() == 0 ) break;

					int nTemp = pWeapon->GetDurability() - nValue;
					if( nTemp < 0 ) nTemp = 0;
					pWeapon->SetDurability( nTemp );
				}
				break;
			case ITEMTYPE_PARTS:
				{
					CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
					if( !pParts ) break;
					if( pParts->IsInfinityDurability() ) break;
					if( pParts->GetDurability() == 0 ) break;

					int nTemp = pParts->GetDurability() - nValue;
					if( nTemp < 0 ) nTemp = 0;
					pParts->SetDurability( nTemp );
				}
				break;
		}
	}
}

void CDnCharInventory::DecreaseDurability( float fValue )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		CDnItem *pItem = iter->second;
		if( !pItem ) continue;
		switch( pItem->GetItemType() ) {
			case ITEMTYPE_WEAPON:
				{
					CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
					if( !pWeapon ) break;
					if( pWeapon->IsInfinityDurability() ) break;
					if( pWeapon->GetDurability() == 0 ) break;

					int nTemp = (int)( pWeapon->GetDurability() - ( pWeapon->GetMaxDurability() * fValue ) );
					if( nTemp < 0 ) nTemp = 0;
					pWeapon->SetDurability( nTemp );
				}
				break;
			case ITEMTYPE_PARTS:
				{
					CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
					if( !pParts ) break;
					if( pParts->IsInfinityDurability() ) break;
					if( pParts->GetDurability() == 0 ) break;

					int nTemp = (int)( pParts->GetDurability() - ( pParts->GetMaxDurability() * fValue ) );
					if( nTemp < 0 ) nTemp = 0;
					pParts->SetDurability( nTemp );
				}
				break;
		}
	}
}
