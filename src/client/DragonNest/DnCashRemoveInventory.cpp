#include "StdAfx.h"

#ifdef PRE_ADD_CASHREMOVE

#include "DnCashInventory.h"
#include "DnItem.h"
#include "DnInventoryDlg.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "DnItemTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnCashRemoveInventory::CDnCashRemoveInventory(void)
{
}

CDnCashRemoveInventory::~CDnCashRemoveInventory(void)
{
	m_mapBeginExpireTimes.clear();
}

void CDnCashRemoveInventory::CreateCashItem( TItem &item, int nSlotIndex, __time64_t beginTime, __time64_t expireTime, bool bExpireComplete )
{
	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnInventory::CreateItem, m_pInvenDlg is NULL!" );
		return;
	}

	if( !m_pItemTask )
	{
		CDebugSet::ToLogFile( "CDnInventory::CreateItem, m_pItemTask is NULL!" );
		return;
	}

	CDnItem *pItem = m_pItemTask->CreateItem( item );

	if( pItem == NULL )
	{
		CDebugSet::ToLogFile( "CDnInventory::CreateItem, failed to create item(%d)!", item.nItemID );
		return;
	}

	std::pair< std::map< INT64, __time64_t >::iterator, bool > mapRet;
	mapRet = m_mapBeginExpireTimes.insert( std::pair< INT64, __time64_t >( item.nSerial, beginTime ) ); // Time when item entered the cash pending tab - for calculating wait time.
	if( mapRet.second == false )
	{
		CDebugSet::ToLogFile( "CDnInventory::CreateCashItem(), m_mapBeginExpireTimes.insert() failed - duplicate key" );
		OutputDebugStringW( L"CDnCashRemoveInventory::CreateCashItem() - Duplicate Item.nSerial\n" );
	}
	
	pItem->SetExpireTime( expireTime );
	pItem->SetCashRemoveItem( bExpireComplete );
	pItem->SetSlotIndex( nSlotIndex );
	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );
}

bool CDnCashRemoveInventory::InsertItem( CDnItem *pItem, bool bMoving )
{
	if (pItem == NULL)
	{
		_ASSERT(0 && "CDnCashRemoveInventory::InsertItem - ITEM is NULL\n");
		return false;
	}

	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnCashRemoveInventory::InsertItem, m_pInvenDlg is NULL!" );
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

	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );

	int nNowCount = GetItemCount(pItem->GetClassID());

	switch( pItem->GetItemType() ) {
		case eItemTypeEnum::ITEMTYPE_NORMAL:
		case eItemTypeEnum::ITEMTYPE_COOKING:
			GetInterface().GetMainBarDialog()->OnRefreshQuickSlot();
			break;
	}

	// When increased beyond the originally existing items 
	int nDeltaCnt = nNowCount - nExistCnt;
	if ( nDeltaCnt > 0 && bMoving == false )
	{
		// Show alert message that more items were obtained.
		if( pItem->GetItemType() == eItemTypeEnum::ITEMTYPE_QUEST )
		{
			GetInterface().ShowSymbolAlarmDialog( pItem->GetName(), pItem, nDeltaCnt, textcolor::WHITE, 3.0f );
		}
		else
		{
			GetInterface().ShowItemAlarmDialog( pItem->GetName(), pItem, nDeltaCnt, textcolor::WHITE, 3.0f );
		}
	}

	return true;
}

bool CDnCashRemoveInventory::RemoveItem( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( iter != m_mapInventory.end() )
	{
		// Clear time when restoring cash item.
		CDnItem * pItem = iter->second;
		std::map< INT64, __time64_t >::iterator timeIter = m_mapBeginExpireTimes.find( pItem->GetSerialID() );
		if( timeIter != m_mapBeginExpireTimes.end() )
			m_mapBeginExpireTimes.erase( timeIter );


		// DnInvenSlotDlg already plays the sound; it would play twice.
		//		CEtSoundEngine::GetInstance().PlaySound( "2D", iter->second->GetDragSoundIndex() );

		eItemTypeEnum emItemType = iter->second->GetItemType();
		m_pInvenDlg->ResetSlot( iter->second );

		SAFE_DELETE( iter->second );
		m_mapInventory.erase( iter );

		switch( emItemType ) {
			case eItemTypeEnum::ITEMTYPE_NORMAL:
			case eItemTypeEnum::ITEMTYPE_COOKING:
				GetInterface().GetMainBarDialog()->OnRefreshQuickSlot();
				break;
		}

		return true;
	}

	return false;
}

void CDnCashRemoveInventory::SetUsableSlotCount( int nCount )
{ 
	if (m_pInvenDlg == NULL) return;
	m_nUsableSlotCount = nCount;
}


void CDnCashRemoveInventory::DecreaseDurability( int nValue )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		CDnItem *pItem = iter->second;
		if( !pItem ) continue;
		switch( pItem->GetItemType() ) {
			case eItemTypeEnum::ITEMTYPE_WEAPON:
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
			case eItemTypeEnum::ITEMTYPE_PARTS:
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

void CDnCashRemoveInventory::DecreaseDurability( float fValue )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		CDnItem *pItem = iter->second;
		if( !pItem ) continue;
		switch( pItem->GetItemType() ) {
			case eItemTypeEnum::ITEMTYPE_WEAPON:
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
			case eItemTypeEnum::ITEMTYPE_PARTS:
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


__time64_t CDnCashRemoveInventory::GetTimeBySN( INT64 nSerial )
{
	std::map< INT64, __time64_t >::iterator it = m_mapBeginExpireTimes.find( nSerial );
	if( it != m_mapBeginExpireTimes.end() )
	{
		return (*it).second;
	}	

	return -1;
}


#endif