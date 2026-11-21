#include "StdAfx.h"
#include "DnInventory.h"
#include "DnItemTask.h"
#include "DnInventoryDlg.h"
#include <bitset>
#include "ItemSendPacket.h"
#include "DnWeapon.h"
#include "DnParts.h"
#include "DnActor.h"
#include "DnTableDB.h"
#include "DnPlayerState.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnInventory::CDnInventory(void)
	: m_nUsableSlotCount(0)
	, m_pInvenDlg(NULL)
	, m_pItemTask(NULL)
	, m_bSortRequesting(false)
#if defined(PRE_PERIOD_INVENTORY)
	, m_bSortPeriod( false )
#endif	// #if defined(PRE_PERIOD_INVENTORY)
{
	memset( m_SortSlotInfo, 0, sizeof(m_SortSlotInfo) );
}

CDnInventory::~CDnInventory(void)
{
	ClearInventory();
}

void CDnInventory::ClearInventory()
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		SAFE_DELETE( iter->second );
	}

	m_mapInventory.clear();
}

CDnItem* CDnInventory::GetItem( int nSlotIndex )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( iter != m_mapInventory.end() )
	{
		return iter->second;
	}

	return NULL;
}

void CDnInventory::CreateItem( TItemInfo &itemInfo )
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

	CDnItem *pItem = m_pItemTask->CreateItem( itemInfo );

	if( pItem == NULL )
	{
		CDebugSet::ToLogFile( "CDnInventory::CreateItem, failed to create item(%d)!", itemInfo.Item.nItemID );
		return;
	}

	m_mapInventory[pItem->GetSlotIndex()] = pItem;
	m_pInvenDlg->SetItem( pItem );
}

int CDnInventory::FindItemList( int nItemTableID, char cOption, std::vector<CDnItem *> &pVecResultList )
{
	CDnItem *pItem(NULL);

	int nCount = 0;
	pVecResultList.clear();
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			if( cOption != -1 && pItem->GetOptionIndex() != cOption ) continue;
			pVecResultList.push_back( pItem );
			nCount += pItem->GetOverlapCount();
		}
	}
	return nCount;
}

CDnItem* CDnInventory::FindItem( int nItemTableID, char cOption )
{
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			if( cOption != -1 && pItem->GetOptionIndex() != cOption ) continue;
			return pItem;
		}
	}

	return NULL;
}

int CDnInventory::FindItemSlotIndex( int nItemTableID )
{
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			return iter->first;
		}
	}

	return -1;
}

CDnItem* CDnInventory::FindItemFromSerialID( INT64 nSerialID )
{
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		if( !pItem ) continue;
		if( pItem->GetSerialID() == nSerialID ) return pItem;
	}

	return NULL;
}

void CDnInventory::FindItemFromItemType( eItemTypeEnum Type, std::vector<CDnItem *> &pVecResult )
{
	CDnItem *pItem;
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		if( !pItem ) continue;
		if( pItem->GetItemType() == Type ) {
			pVecResult.push_back( pItem );
		}
	}
}

void CDnInventory::FindItemFromItemType( eItemTypeEnum Type, std::vector<CDnItem *> &pVecItem, std::vector<int> &pVecIndex )
{
	CDnItem *pItem;
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		if( !pItem ) continue;
		if( pItem->GetItemType() == Type ) {
			pVecItem.push_back( pItem );
			pVecIndex.push_back( iter->first );
		}
	}
}

int CDnInventory::GetItemCount( int nItemTableID, char cOption )
{
	int nItemCount(0);
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;

		if( pItem && (pItem->GetClassID() == nItemTableID) )
		{
			if( cOption != -1 && pItem->GetOptionIndex() != cOption ) continue;
			nItemCount += pItem->GetOverlapCount();
		}
	}

	return nItemCount;
}

void CDnInventory::BeginSameSkillCoolTime( const CDnItem *pItem, int nSkillID )
{
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		if( iter->second == pItem )
			continue;

		if( iter->second->GetSkillID() == nSkillID )
			iter->second->BeginCoolTime();
	}
}

void CDnInventory::RefreshInventory()
{
	if( !m_pInvenDlg )
	{
		CDebugSet::ToLogFile( "CDnInventory::RefreshInventory, m_pInvenDlg is NULL!" );
		return;
	}

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		m_pInvenDlg->ResetSlot( iter->second );
		m_pInvenDlg->SetItem( iter->second );

#if defined( PRE_ADD_STAGE_USECOUNT_ITEM )
		iter->second->SetUseLimit(false);
#endif
	}
}

void CDnInventory::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// Iterating through maps... room for optimization.
	// Calculate cooldown
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
		iter->second->ProcessCoolTime( LocalTime, fDelta );
	}
}

int CDnInventory::ScanItemFromID( int nItemTableID, std::vector<CDnItem *> *pVecResult )
{
	//ASSERT(pVecResult&&"CDnInventory::ScanItemFromID, pVecResult is NULL!");

	int nCount(0);
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		if( !pItem ) continue;

		if( pItem->GetClassID() == nItemTableID )
		{
			if( pVecResult )
				pVecResult->push_back( pItem );

			nCount += pItem->GetOverlapCount();
		}
	}

	return nCount;
}

bool CDnInventory::RequestUseItem( int nSlotIndex, char cType )
{
	// Cannot use item if cooldown remains.
	INVENTORY_MAP_ITER iter = m_mapInventory.find( nSlotIndex );
	if( m_mapInventory.end() != iter )
	{
#if defined( _FINAL_BUILD )
		if( iter->second->GetCoolTime() == 0.0f )
#else
		if( CGlobalValue::GetInstance().m_bIgnoreCoolTime == true || iter->second->GetCoolTime() == 0.0f )
#endif // #if defined( _FINAL_BUILD )
		{
			switch( cType )
			{
			case ITEMPOSITION_INVEN:		SendUseItem( cType, nSlotIndex, iter->second->GetSerialID() );	break;
			case ITEMPOSITION_CASHINVEN:	SendUseItem( cType, 0, iter->second->GetSerialID() );	break;
			}
			return true;
		}
	}

	return false;
}

void CDnInventory::SetUsableSlotCount( int nCount )
{ 
	if (m_pInvenDlg)
	{
		m_nUsableSlotCount = nCount;
		m_pInvenDlg->SetUseItemCnt( nCount );
	}
}

int CDnInventory::GetRepairInvenPrice()
{
	float fTotalPrice = 0.f;
	CDnItem *pItem(NULL);

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		if( !pItem ) continue;
		if( pItem->GetItemType() == ITEMTYPE_WEAPON )
		{
			CDnWeapon *pWeapon = dynamic_cast<CDnWeapon *>(pItem);
			if( pWeapon )
				fTotalPrice += pWeapon->GetRepairPrice();
		}
		else if( pItem->GetItemType() == ITEMTYPE_PARTS )
		{
			CDnParts *pParts = dynamic_cast<CDnParts *>(pItem);
			if( pParts )
				fTotalPrice += pParts->GetRepairPrice();
		}
	}
	if( fTotalPrice != 0.f && fTotalPrice < 1.f ) fTotalPrice = 1.f;
	return (int)fTotalPrice;
}
//blondy
/*
bool CDnInventory::IsDisableItem( int iGameMode )
{
	CDnItem *pItem(NULL);
	int iAllowedGameType = 0;

	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
	{
		pItem = iter->second;
		iAllowedGameType = pItem->GetAllowGameType( pItem->GetClassID() );
		pItem->SetDisable( iAllowedGameType|iGameMode?true:false);		
	}
}
*/
//blondy end

//CDnItem *CDnInventory::GetItemFromSerialID( INT64 nSerialID )
//{
//	CDnItem *pItem(NULL);
//
//	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
//	for( ; iter != m_mapInventory.end(); ++iter )
//	{
//		pItem = iter->second;
//		if( !pItem ) continue;
//
//		if( pItem->GetSerialID() == nSerialID )
//			return pItem;
//	}
//
//	return NULL;
//}

int CDnInventory::FindFirstEmptyIndex()
{
	if( m_mapInventory.empty() ) return 0;

	// Find the last slot index, and
	int nLastCashInvenSlotIndex = 0;
	std::map<int,CDnItem*>::reverse_iterator riter = m_mapInventory.rbegin();
	if( riter != m_mapInventory.rend() )
		nLastCashInvenSlotIndex = riter->first;

	// Maximum number of slots
	int nMaxNumSlot = nLastCashInvenSlotIndex+1;	

	// Increase temporary variable max size accordingly and initialize
	if( (int)m_vecTempCheck.size() < nMaxNumSlot )
		m_vecTempCheck.resize( nMaxNumSlot );
	for( int i = 0; i < (int)m_vecTempCheck.size(); ++i )
		m_vecTempCheck[i] = false;

	// Scan inventory and check slots containing items
	CDnItem *pItem(NULL);
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
		m_vecTempCheck[iter->first] = true;

	// Finally, find the very first index.
	for( int i = 0; i < nMaxNumSlot; ++i )
	{
		if( m_vecTempCheck[i] == false )
			return i;
	}

	// If not found, inventory is full with no empty slots in between, so return max index + 1.
	return nMaxNumSlot;
}

static bool IsSubWeapon( CDnWeapon::EquipTypeEnum emType )
{
	switch( emType )
	{
	case CDnWeapon::Sword:
	case CDnWeapon::Axe:
	case CDnWeapon::Hammer:
	case CDnWeapon::SmallBow:
	case CDnWeapon::BigBow:
	case CDnWeapon::CrossBow:
	case CDnWeapon::Staff:
	case CDnWeapon::Mace:
	case CDnWeapon::Flail:
	case CDnWeapon::Wand:		
	case CDnWeapon::Cannon:
	case CDnWeapon::BubbleGun:	
	case CDnWeapon::Chakram:
	case CDnWeapon::Fan:
	case CDnWeapon::Scimiter:
	case CDnWeapon::Dagger:
	case CDnWeapon::Spear:
	case CDnWeapon::KnuckleGear:
		return false;
	case CDnWeapon::Shield:
	case CDnWeapon::Arrow:
	case CDnWeapon::Book:
	case CDnWeapon::Orb:
	case CDnWeapon::Puppet:
	case CDnWeapon::Gauntlet:	
	case CDnWeapon::Glove:		
	case CDnWeapon::Charm:
	case CDnWeapon::Crook:
	case CDnWeapon::Bracelet:
	case CDnWeapon::Claw:
		return true;
	default:
		CDebugSet::ToLogFile( "CDnInventory::IsSubWeapon, default case(%d)", emType );
		ASSERT(0&&"CDnInventory::IsSubWeapon");
		break;
	}
	return false;
}

static bool CompareInvenItem( CDnItem *s1, CDnItem *s2 )
{
	if( !CDnActor::s_hLocalActor ) return false;

	// When both are junk items - checked separately above because this is the only case that should not be sorted by item type order.
	if( s1->GetItemType() == ITEMTYPE_NORMAL && s1->GetSkillID() == 0 &&
		s2->GetItemType() == ITEMTYPE_NORMAL && s2->GetSkillID() == 0 )
	{
		// If items are of the same type and stackable, move the one with a higher stack count to the front.
		if( (s1->GetClassID() == s2->GetClassID()) && (s1->GetMaxOverlapCount() > 1) )
		{
			if( s1->GetOverlapCount() < s2->GetOverlapCount() ) return false;
			else if( s1->GetOverlapCount() > s2->GetOverlapCount() ) return true;
		}

		// Sort in order of ID.
		if( s1->GetClassID() < s2->GetClassID() ) return true;
		else if( s1->GetClassID() > s2->GetClassID() ) return false;
		return false;
	}
	// Misc items are sorted after any other type.
	else if( s1->GetItemType() == ITEMTYPE_NORMAL && s1->GetSkillID() == 0 ) return false;
	else if( s2->GetItemType() == ITEMTYPE_NORMAL && s2->GetSkillID() == 0 ) return true;

	// Sort by item type order by default
	if( s1->GetItemType() < s2->GetItemType() ) return true;
	else if( s1->GetItemType() > s2->GetItemType() ) return false;

	// Weapons and armor are also sorted by sub-type. Since types match, checking s1 alone is sufficient.
	if( s1->GetItemType() == ITEMTYPE_WEAPON )
	{
 		CDnWeapon *pWeapon1 = dynamic_cast<CDnWeapon *>(s1);
 		CDnWeapon *pWeapon2 = dynamic_cast<CDnWeapon *>(s2);

		// For weapons, distinguish only main weapon vs sub-weapon instead of equip type, then pass down.
		// When s1 is a sub-weapon and s2 is not a sub-weapon,
		if( IsSubWeapon(pWeapon1->GetEquipType()) && !IsSubWeapon(pWeapon2->GetEquipType()) )
			return false;
		if( IsSubWeapon(pWeapon2->GetEquipType()) && !IsSubWeapon(pWeapon1->GetEquipType()) )
			return true;
	}
	else if( s1->GetItemType() == ITEMTYPE_PARTS )
	{
		CDnParts *pParts1 = dynamic_cast<CDnParts *>(s1);
		CDnParts *pParts2 = dynamic_cast<CDnParts *>(s2);
		if( pParts1->GetPartsType() < pParts2->GetPartsType() ) return true;
		else if( pParts1->GetPartsType() > pParts2->GetPartsType() ) return false;
	}

	// For weapons, distinguish main vs sub-weapon; for armor, if even sub-type matches,
	// And skill books require class differentiation handling, so they are included here.
	if( s1->GetItemType() == ITEMTYPE_WEAPON || s1->GetItemType() == ITEMTYPE_PARTS || s1->GetItemType() == ITEMTYPE_SKILLBOOK )
	{
		// First, move items without class restrictions to the front,
		if( !s1->IsNeedJob() && s2->IsNeedJob() ) return true;
		else if( s1->IsNeedJob() && !s2->IsNeedJob() ) return false;

		// If both have a required job, pick the first job ID
		if( s1->IsNeedJob() && s2->IsNeedJob() )
		{
			// Convert Job to class ID (1~4).
			int nS1NeedClassID = CDnPlayerState::GetJobToBaseClassID(s1->GetNeedJob(0));
			int nS2NeedClassID = CDnPlayerState::GetJobToBaseClassID(s2->GetNeedJob(0));

			// When the required top-level parent classes of the two items differ
			if( nS1NeedClassID != nS2NeedClassID )
			{
				// Own class line first, the rest after.
				if( nS1NeedClassID == CDnActor::s_hLocalActor->GetClassID() ) return true;
				else if( nS2NeedClassID == CDnActor::s_hLocalActor->GetClassID() ) return false;
				else if( nS1NeedClassID < nS2NeedClassID ) return true;
				else if( nS1NeedClassID > nS2NeedClassID ) return false;
			}
		}

		// Remaining checks for level limit, grade, etc. are handled only for Weapon and Parts.
		if( s1->GetItemType() != ITEMTYPE_SKILLBOOK )
		{
			// If job is also the same, order by lowest required level
			if( s1->GetLevelLimit() < s2->GetLevelLimit() ) return true;
			else if( s1->GetLevelLimit() > s2->GetLevelLimit() ) return false;

			// Higher grade
			if( s1->GetItemRank() < s2->GetItemRank() ) return false;
			else if( s1->GetItemRank() > s2->GetItemRank() ) return true;

			// Higher enhancement.
			if( s1->GetEnchantLevel() < s2->GetEnchantLevel() ) return false;
			else if( s1->GetEnchantLevel() > s2->GetEnchantLevel() ) return true;

			// One with higher durability. Since types match, checking s1 alone is enough.
			if( s1->GetItemType() == ITEMTYPE_WEAPON )
			{
				CDnWeapon *pWeapon1 = dynamic_cast<CDnWeapon *>(s1);
				CDnWeapon *pWeapon2 = dynamic_cast<CDnWeapon *>(s2);
				float fRate1 = (float)pWeapon1->GetDurability() / pWeapon1->GetMaxDurability();
				float fRate2 = (float)pWeapon2->GetDurability() / pWeapon2->GetMaxDurability();
				if( fRate1 < fRate2 ) return false;
				else if( fRate1 > fRate2 ) return true;
			}
			else if( s1->GetItemType() == ITEMTYPE_PARTS )
			{
				CDnParts *pParts1 = dynamic_cast<CDnParts *>(s1);
				CDnParts *pParts2 = dynamic_cast<CDnParts *>(s2);
				float fRate1 = (float)pParts1->GetDurability() / pParts1->GetMaxDurability();
				float fRate2 = (float)pParts2->GetDurability() / pParts2->GetMaxDurability();
				if( fRate1 < fRate2 ) return false;
				else if( fRate1 > fRate2 ) return true;
			}
		}
	}

	//Farm related
	if(s1->GetItemType() == ITEMTYPE_SEED && s2->GetItemType() == ITEMTYPE_SEED)
	{
		DNTableFileFormat* pSoxFile = GetDNTable( CDnTableDB::TFARMCULTIVATE );
			
		int nGrade_s1 = pSoxFile->GetFieldFromLablePtr( s1->GetClassID(), "_SecondarySkillClass" )->GetInteger();
		int nLevel_s1 = pSoxFile->GetFieldFromLablePtr( s1->GetClassID(), "_SecondarySkillLevel" )->GetInteger();

		int nGrade_s2 = pSoxFile->GetFieldFromLablePtr( s2->GetClassID(), "_SecondarySkillClass" )->GetInteger();
		int nLevel_s2 = pSoxFile->GetFieldFromLablePtr( s2->GetClassID(), "_SecondarySkillLevel" )->GetInteger();

		if( nGrade_s1 > nGrade_s2 )
			return false;
		else if( nGrade_s1 == nGrade_s2 )
		{
			if( nLevel_s1 > nLevel_s2 )
				return false;
			else
				return true;
		}
		else
			return true;
	}

	// If items are of the same type and stackable, move the one with a higher stack count to the front.
	if( (s1->GetClassID() == s2->GetClassID()) && (s1->GetMaxOverlapCount() > 1) )
	{
		if( s1->GetOverlapCount() < s2->GetOverlapCount() ) return false;
		else if( s1->GetOverlapCount() > s2->GetOverlapCount() ) return true;
	}

	// If all equipment conditions are equal, sort consumables, jewels, plates, and misc items by ID order.
	if( s1->GetClassID() < s2->GetClassID() ) return true;
	else if( s1->GetClassID() > s2->GetClassID() ) return false;

	return false;
}

bool CDnInventory::PrepareSort( bool bPeriod )
{
	// If currently requesting sort,
	if( m_bSortRequesting )
	{
		ASSERT(0&&"Do not sort twice before receiving sort result.");
		return false;	// An 'already sorted' message will appear, but it is a rare edge case so leave it as is.
	}

	int nSortStartIndex = 0;

#if defined(PRE_PERIOD_INVENTORY)
	m_bSortPeriod = bPeriod;
	nSortStartIndex = m_bSortPeriod ? INVENTORYMAX : 0;
	m_mapNotSortItem.clear();
#endif	// #if defined(PRE_PERIOD_INVENTORY)

	m_vecSortItem.clear();

	// Iterate through maps
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();

	for( iter; iter != iterEnd; ++iter )
	{
#if defined(PRE_PERIOD_INVENTORY)
		if( ( false == m_bSortPeriod && INVENTORYMAX <= iter->first ) 
		 || ( true == m_bSortPeriod && INVENTORYMAX > iter->first ) )
		{
			m_mapNotSortItem.insert( std::pair<int, CDnItem*>( iter->first, iter->second ) );
			continue;
		}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

		m_vecSortItem.push_back(iter->second);
	}

	// If there are no items at all,
	if( (int)m_vecSortItem.size() == 0 )
		return false;

	std::sort( m_vecSortItem.begin(), m_vecSortItem.end(), CompareInvenItem );

	bool bChanged = false;
	for( BYTE i = 0; i < (BYTE)m_vecSortItem.size(); ++i )
	{
		m_SortSlotInfo[i].cNew = i;

		m_SortSlotInfo[i].cCurrent = m_vecSortItem[i]->GetSlotIndex();

		if( m_SortSlotInfo[i].cNew != m_SortSlotInfo[i].cCurrent - nSortStartIndex )
			bChanged = true;
	}

	// If no slot number changed after sorting,
	if( bChanged == false )
		return false;

	m_bSortRequesting = true;
	return true;
}

bool CDnInventory::Sort( bool bServerSucceed )
{
	// If not currently requesting sort,
	if( m_bSortRequesting == false )
	{
		ASSERT(0&&"Why is sorting performed when no sort request was made?");
		return false;
	}

	if( !bServerSucceed )
	{
		m_bSortRequesting = false;
		return false;
	}

	// Verify whether inventory changed after request
	for( BYTE i = 0; i < (BYTE)m_vecSortItem.size(); ++i )
	{
		int nSlotIndex = m_vecSortItem[i]->GetSlotIndex();
		if( m_mapInventory.count( nSlotIndex ) != 0 &&  m_mapInventory[ nSlotIndex ] != m_vecSortItem[ i ] )		// Fixed crash caused by NULL being inserted by default when Key is missing.
		{
			m_bSortRequesting = false;
			return false;
		}
	}

	// Initialize inventory. Also initialize slots.
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	for( ; iter != m_mapInventory.end(); ++iter )
		m_pInvenDlg->ResetSlot( iter->second );
	m_mapInventory.clear();

	int nSortStartIndex = 0;
#if defined(PRE_PERIOD_INVENTORY)
	nSortStartIndex = m_bSortPeriod ? INVENTORYMAX : 0;
#endif	// #if defined(PRE_PERIOD_INVENTORY)

	// Convert the map according to exchange info
	for( BYTE i = 0; i < (BYTE)m_vecSortItem.size(); ++i )
	{
		m_vecSortItem[i]->SetSlotIndex( nSortStartIndex + i );
		m_mapInventory[nSortStartIndex + i] = m_vecSortItem[i];

		m_pInvenDlg->SetItem(m_vecSortItem[i]);
	}

#if defined(PRE_PERIOD_INVENTORY)
	iter = m_mapNotSortItem.begin();
	for( ; iter != m_mapNotSortItem.end(); ++iter )
	{
		iter->second->SetSlotIndex( iter->first );
		m_mapInventory[iter->first] = iter->second;
		m_pInvenDlg->SetItem( iter->second );
	}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

	m_bSortRequesting = false;
	return true;
}

CDnInventory::INVENTORY_MAP& CDnInventory::GetInventoryItemList()
{
	return m_mapInventory;
}

static bool CompareCashInvenItem( CDnItem *s1, CDnItem *s2 )
{
	if( !CDnActor::s_hLocalActor ) return false;

	// Place Weapon and Parts at the very back by default
	if( s1->GetItemType() == ITEMTYPE_WEAPON && s2->GetItemType() == ITEMTYPE_PARTS ) return true;
	else if( s1->GetItemType() == ITEMTYPE_PARTS && s2->GetItemType() == ITEMTYPE_WEAPON ) return false;
	else if( s1->GetItemType() != ITEMTYPE_WEAPON && s2->GetItemType() == ITEMTYPE_WEAPON ) return true;
	else if( s1->GetItemType() == ITEMTYPE_WEAPON && s2->GetItemType() != ITEMTYPE_WEAPON ) return false;
	else if( s1->GetItemType() != ITEMTYPE_PARTS && s2->GetItemType() == ITEMTYPE_PARTS ) return true;
	else if( s1->GetItemType() == ITEMTYPE_PARTS && s2->GetItemType() != ITEMTYPE_PARTS ) return false;

	// Weapons and armor are also sorted by sub-type. Since types match, checking s1 alone is sufficient.
	if( s1->GetItemType() == ITEMTYPE_WEAPON )
	{
		CDnWeapon *pWeapon1 = dynamic_cast<CDnWeapon *>(s1);
		CDnWeapon *pWeapon2 = dynamic_cast<CDnWeapon *>(s2);

		// For weapons, distinguish only main weapon vs sub-weapon instead of equip type, then pass down.
		// When s1 is a sub-weapon and s2 is not a sub-weapon,
		if( IsSubWeapon(pWeapon1->GetEquipType()) && !IsSubWeapon(pWeapon2->GetEquipType()) )
			return false;
		if( IsSubWeapon(pWeapon2->GetEquipType()) && !IsSubWeapon(pWeapon1->GetEquipType()) )
			return true;
	}
	else if( s1->GetItemType() == ITEMTYPE_PARTS )
	{
		CDnParts *pParts1 = dynamic_cast<CDnParts *>(s1);
		CDnParts *pParts2 = dynamic_cast<CDnParts *>(s2);
		if( pParts1->GetPartsType() < pParts2->GetPartsType() ) return true;
		else if( pParts1->GetPartsType() > pParts2->GetPartsType() ) return false;
	}

	// For weapons, distinguish main vs sub-weapon; for armor, if even sub-type matches,
	// And skill books require class differentiation handling, so they are included here.
	if( s1->GetItemType() == ITEMTYPE_WEAPON || s1->GetItemType() == ITEMTYPE_PARTS || s1->GetItemType() == ITEMTYPE_SKILLBOOK )
	{
		// First, move items without class restrictions to the front,
		if( !s1->IsNeedJob() && s2->IsNeedJob() ) return true;
		else if( s1->IsNeedJob() && !s2->IsNeedJob() ) return false;

		// If both have a required job, pick the first job ID
		if( s1->IsNeedJob() && s2->IsNeedJob() )
		{
			// Convert Job to class ID (1~4).
			int nS1NeedClassID = CDnPlayerState::GetJobToBaseClassID(s1->GetNeedJob(0));
			int nS2NeedClassID = CDnPlayerState::GetJobToBaseClassID(s2->GetNeedJob(0));

			// When the required top-level parent classes of the two items differ
			if( nS1NeedClassID != nS2NeedClassID )
			{
				// Own class line first, the rest after.
				if( nS1NeedClassID == CDnActor::s_hLocalActor->GetClassID() ) return true;
				else if( nS2NeedClassID == CDnActor::s_hLocalActor->GetClassID() ) return false;
				else if( nS1NeedClassID < nS2NeedClassID ) return true;
				else if( nS1NeedClassID > nS2NeedClassID ) return false;
			}
		}

		// Remaining checks for level limit, grade, etc. are handled only for Weapon and Parts.
		if( s1->GetItemType() != ITEMTYPE_SKILLBOOK )
		{
			// If job is also the same, order by lowest required level
			if( s1->GetLevelLimit() < s2->GetLevelLimit() ) return true;
			else if( s1->GetLevelLimit() > s2->GetLevelLimit() ) return false;

			// Higher grade
			if( s1->GetItemRank() < s2->GetItemRank() ) return false;
			else if( s1->GetItemRank() > s2->GetItemRank() ) return true;
		}
	}

	// If all equipment conditions are equal, sort consumables, jewels, plates, and misc items by ID order.
	if( s1->GetClassID() < s2->GetClassID() ) return true;
	else if( s1->GetClassID() > s2->GetClassID() ) return false;

	// If items are of the same type and have remaining duration, move the one with less remaining time to the front.
	if( s1->GetClassID() == s2->GetClassID() )
	{
		if( s1->IsEternityItem() == true && s2->IsEternityItem() == false ) return false;
		else if( s1->IsEternityItem() == false && s2->IsEternityItem() == true ) return true;
		else if( s1->IsEternityItem() == false && s2->IsEternityItem() == false )
		{
			if( s1->GetExpireDate() < s2->GetExpireDate() ) return true;
			else if( s1->GetExpireDate() > s2->GetExpireDate() ) return false;
		}
	}

	// If items are of the same type, have the same remaining duration, and are stackable, move the one with a higher stack count to the front.
	if( (s1->GetClassID() == s2->GetClassID()) && (s1->GetMaxOverlapCount() > 1) )
	{
		if( s1->GetOverlapCount() < s2->GetOverlapCount() ) return false;
		else if( s1->GetOverlapCount() > s2->GetOverlapCount() ) return true;
	}

	return false;
}

bool CDnInventory::SortCashInventory()
{
	std::vector<CDnItem*> vecSortItem;
	vecSortItem.clear();

	// If there are no items at all,
	if( (int)m_mapInventory.size() == 0 )
		return false;

	// Iterate through maps
	INVENTORY_MAP_ITER iter = m_mapInventory.begin();
	INVENTORY_MAP_ITER iterEnd = m_mapInventory.end();
	vecSortItem.reserve(m_mapInventory.size());
	for( iter; iter != iterEnd; ++iter )
		vecSortItem.push_back(iter->second);

	std::sort( vecSortItem.begin(), vecSortItem.end(), CompareCashInvenItem );

	bool bChanged = false;
	for( int i = 0; i < (int)vecSortItem.size(); ++i )
	{
		if( vecSortItem[i]->GetSlotIndex() != i )
		{
			bChanged = true;
			break;
		}
	}

	// If no slot number changed after sorting,
	if( bChanged == false )
		return false;

	// After removing all from the back (must remove this way to refresh cleanly without leftover pages at the end)
	std::map<int,CDnItem*>::reverse_iterator riter = m_mapInventory.rbegin();
	while( riter != m_mapInventory.rend() )
	{
		INVENTORY_MAP_ITER iter2 = m_mapInventory.find( riter->first );
		if( iter2 != m_mapInventory.end() )
		{
			m_pInvenDlg->ResetSlot( iter2->second );
			m_mapInventory.erase( iter2 );
		}
	}

	// Rebuild with sorted result
	for( int i = 0; i < (int)vecSortItem.size(); ++i )
	{
		vecSortItem[i]->SetSlotIndex(i);
		m_mapInventory[i] = vecSortItem[i];
		m_pInvenDlg->SetItem(vecSortItem[i]);
	}

	return true;
}