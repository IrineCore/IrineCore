#include "stdafx.h"
#include "DNCashItem.h"
#include "DNUserSession.h"
#include "DNGameDataManager.h"
#include "DNGesture.h"
#include "DNAppellation.h"

CDNEffectItem::CDNEffectItem( CDNUserSession* pSession, INT64 nItemSerial, TItemData* pItemData, __time64_t tExpireDate )
:m_pSession(pSession), m_nItemSerial(nItemSerial), m_pItemData(pItemData),m_tExpireDate(tExpireDate)
{
}

bool CDNEffectItem::Add()
{
	if( !OnAdd() )
		return false;

	// Cash item increase notification
	return true;
}

bool CDNEffectItem::Remove()
{
	if( !OnRemove() )
		return false;

	// Cash item removal notification
	return true;
}

// Inventory expansion item
bool CDNInventoryExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	int	iCurCount = m_pSession->GetItem()->GetInventoryCount();
	int	iAfterCount	= iCurCount + m_pItemData->nTypeParam[0];
	if (iAfterCount > INVENTORYMAX)
		iAfterCount = INVENTORYMAX;

	m_pSession->GetItem()->SetInventoryCount( iAfterCount );
	m_pSession->SendInventoryMaxCount( iAfterCount );
	return true;
}

bool CDNInventoryExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	int	iCurCount = m_pSession->GetItem()->GetInventoryCount();
	int	iAfterCount	= iCurCount - m_pItemData->nTypeParam[0];
	if( iAfterCount < DEFAULTINVENTORYMAX )
		return false;

	m_pSession->GetItem()->SetInventoryCount( iAfterCount );
	m_pSession->SendInventoryMaxCount( iAfterCount );
	return true;
}

#if defined(PRE_PERIOD_INVENTORY)
// Timed inventory expansion item
bool CDNPeriodInventoryExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetEnablePeriodInventory(true, m_tExpireDate);

	return true;
}

bool CDNPeriodInventoryExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetEnablePeriodInventory(false, m_tExpireDate);

	return true;
}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

// Crest expansion item
bool CDNGlyphExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	int	iCurCount	= m_pSession->GetItem()->GetGlyphEntendCount();
	int	iAfterCount	= iCurCount + m_pItemData->nTypeParam[0];
#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
	if( iAfterCount > GLYPH_CASH_TOTAL_MAX )	
#else
	if( iAfterCount > GLYPHMAX-GLYPH_CASH1 )
#endif //#if defined(PRE_ADD_DRAGON_FELLOWSHIP_GLYPH)
		return false;

	if(m_pItemData->nTypeParam[0] > 1)
	{
		//For an item that opens all crest slots at once
		for(BYTE i = iCurCount;i<iAfterCount;i++)
		{
			m_pSession->GetItem()->SetGlyphExpireDate( i, m_tExpireDate );
		}
	}
	else
	{
		//When opening one by one
		BYTE count =  m_pItemData->nTypeParam[1];
		if(count == 0)
			count = 1;
		m_pSession->GetItem()->SetGlyphExpireDate( count-1, m_tExpireDate );		
	}

	m_pSession->GetItem()->SetGlyphExtendCount( iAfterCount );
	return true;
}

bool CDNGlyphExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	int	iCurCount	= m_pSession->GetItem()->GetGlyphEntendCount();
	int	iAfterCount	= iCurCount - m_pItemData->nTypeParam[0];
	if( iAfterCount < 0 )
		return false;

	m_pSession->GetItem()->SetGlyphExtendCount( iAfterCount );
	m_pSession->SendGlyphExtendCount( iAfterCount );
	return true;
}

#if defined(PRE_ADD_TALISMAN_SYSTEM)
bool CDNTalismanExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetTalismanExpireDate( true, m_tExpireDate );
	return true;
}

bool CDNTalismanExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetTalismanExpireDate( false, 0 );
	return true;
}
#endif	// #if defined(PRE_ADD_TALISMAN_SYSTEM)

// Warehouse inventory expansion item
bool CDNWareHouseInventoryExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}
	
	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	int	iCurCount	= m_pSession->GetItem()->GetWarehouseCount();
	int	iAfterCount	= iCurCount + m_pItemData->nTypeParam[0];
	if (iAfterCount > WAREHOUSEMAX)
		iAfterCount = WAREHOUSEMAX;

	m_pSession->GetItem()->SetWarehouseCount( iAfterCount );
	m_pSession->SendWarehouseMaxCount( iAfterCount );
	return true;
}

bool CDNWareHouseInventoryExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	int	iCurCount	= m_pSession->GetItem()->GetWarehouseCount();
	int	iAfterCount	= iCurCount - m_pItemData->nTypeParam[0];
	if( iAfterCount < DEFAULTWAREHOUSEMAX )
		return false;

	m_pSession->GetItem()->SetWarehouseCount( iAfterCount );
	m_pSession->SendWarehouseMaxCount( iAfterCount );
	return true;
}

#if defined(PRE_PERIOD_INVENTORY)
// Timed storage inventory expansion cash item
bool CDNPeriodWarehouseExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetEnablePeriodWarehouse(true, m_tExpireDate);

	return true;
}

bool CDNPeriodWarehouseExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->SetEnablePeriodWarehouse(false, 0);

	return true;
}
#endif	// #if defined(PRE_PERIOD_INVENTORY)

bool CDNGestureAddItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}
	
	int nGestureID = m_pItemData->nTypeParam[0];
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TGESTURE );
	int nGestureType = pSox->GetFieldFromLablePtr( nGestureID, "_GestureType" )->GetInteger();
	
	if(nGestureType == Gesture::Type::GuildRewardItem)
	{		
		m_pSession->GetGesture()->AddEffectItemGesture( nGestureID );
		m_pSession->SendEffectItemGestureList();
	}
	else
	{
		if( !m_pItemData->IsCash )
		{
			_ASSERT(0);
			return false;
		}		
		m_pSession->GetGesture()->AddCashGesture( nGestureID );
		m_pSession->SendCashGestureList();
	}

	return true;
}

bool CDNGestureAddItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	int nGestureID = m_pItemData->nTypeParam[0];
	if( m_pItemData->cReversion == ITEMREVERSION_GUILD )
	{
		m_pSession->GetGesture()->DelEffectItemGesture( nGestureID );
	}
	else
	{
		m_pSession->GetGesture()->DelCashGesture( nGestureID );
	}

	return true;
}

bool CDNGuildWareSlotExtendItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	if( !m_pItemData->IsCash )
	{
		_ASSERT(0);
		return false;
	}

	// Warehouse size
	int nWareSize = m_pSession->GetGuildWareAllowCount();
	if (nWareSize == -1)
		return false;
	
	int nAddSize = m_pItemData->nTypeParam[0];
	nWareSize += nAddSize;

	if (nWareSize > GUILD_WAREHOUSE_MAX)
		nWareSize = GUILD_WAREHOUSE_MAX;

	m_pSession->ExtendGuildWareAllowCount(nWareSize);
	
	return true;
}

bool CDNGuildWareSlotExtendItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	// Guild warehouse expansion item is permanent, so do not remove it.

	return true;
}

bool CDNUnionMembershipItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}
	
	m_pSession->GetItem()->SetUnionMembership( m_pItemData->nTypeParam[2], m_pItemData->nItemID, m_tExpireDate );

	return true;
}

bool CDNUnionMembershipItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}
	
	return true;
}

#if defined( PRE_ADD_VIP_FARM )

bool CDNFarmVipItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDNFarmVipItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	return true;
}

#endif // #if defined( PRE_ADD_VIP_FARM )

bool CDNPeriodAppellationItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	int nArrayID = g_pDataManager->GetPeriodAppellationArrayID( m_pItemData->nTypeParam[0] );
	if(nArrayID > -1)
		m_pSession->GetAppellation()->AddPeriodAppellation(nArrayID, m_tExpireDate);
	return true;
}

bool CDNPeriodAppellationItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}
	
	return true;
}

bool CDNExpandSkillPageItem::OnAdd()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}	
	
	m_pSession->GetItem()->SetSkillPageCount( DualSkill::Type::MAX );
	return true;
}

bool CDNExpandSkillPageItem::OnRemove()
{
	if( !m_pSession || !m_pSession->GetItem() )
	{
		_ASSERT( 0 );
		return false;
	}

	return true;
}

bool CDNSourceItem::OnAdd()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )	
	m_pSession->GetItem()->AddEffectSkillItem( m_nItemSerial, m_pItemData->nItemID, m_pItemData->nSkillID, m_pItemData->cSkillLevel, GetExpireDate() );
#else
	m_pSession->GetItem()->SetSource(m_nItemSerial, m_pItemData->nItemID, GetExpireDate());
#endif

	return true;
}

bool CDNSourceItem::OnRemove()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	m_pSession->GetItem()->DelEffectSkillItem( m_pItemData->nSkillID );
#else
	m_pSession->GetItem()->RemoveSource();
#endif

	return true;
}

#if defined (PRE_ADD_NAMEDITEM_SYSTEM)
bool CDNEffectSkillItem::OnAdd()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->AddEffectSkillItem( m_nItemSerial, m_pItemData->nItemID, m_pItemData->nSkillID, m_pItemData->cSkillLevel, GetExpireDate() );

	return true;
}

bool CDNEffectSkillItem::OnRemove()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->DelEffectSkillItem( m_pItemData->nSkillID );
	return true;
}
#endif // #if defined (PRE_ADD_NAMEDITEM_SYSTEM)

#if defined (PRE_ADD_BESTFRIEND)
bool CDNEffectBestFriendBufftem::OnAdd()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->AddEffectSkillItem( m_nItemSerial, m_pItemData->nItemID, m_pItemData->nSkillID, m_pItemData->cSkillLevel, GetExpireDate(), true );

	return true;
}

bool CDNEffectBestFriendBufftem::OnRemove()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}

	m_pSession->GetItem()->DelEffectSkillItem( m_pItemData->nSkillID );
	return true;
}
#endif // #if defined (PRE_ADD_BESTFRIEND)

#if defined( PRE_ADD_TOTAL_LEVEL_SKILL )
bool CDNEffectTotalLevelSkilltem::OnAdd()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}
	int nIndex = 0;
	if( m_pItemData->nTypeParam[1] > 0 && m_pItemData->nTypeParam[1] <= TotalLevelSkill::Common::MAXSLOTCOUNT)
		nIndex = m_pItemData->nTypeParam[1] - 1;
	else
		nIndex = TotalLevelSkill::Common::MAXSLOTCOUNT - 1;	
	TItemData *pItemData = g_pDataManager->GetItemData(m_pItemData->nItemID);
	if( !pItemData )
	{
		_ASSERT(0);
		return false;
	}
	if( pItemData->nTypeParam[2] == 0 )
		m_tExpireDate = 0;

	m_pSession->SetTotalLevelSkillCashSlot(nIndex, GetExpireDate());
	m_pSession->SendTotalLevelSkillCashSlot(m_pSession->GetSessionID(), nIndex, m_pSession->bIsTotalLevelSkillCashSlot(nIndex), GetExpireDate());

	return true;
}

bool CDNEffectTotalLevelSkilltem::OnRemove()
{
	if (!m_pSession || !m_pSession->GetItem())
	{
		_ASSERT(0);
		return false;
	}	
	return true;
}
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
bool CDNEffectComebackItem::OnAdd()
{
	return true;
}

bool CDNEffectComebackItem::OnRemove()
{
	return true;
}
#endif
