#include "stdafx.h"
#include "DnGoldMonsterActor.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnDropItem.h"
#include "DNGameRoom.h"

CDnGoldMonsterActor::CDnGoldMonsterActor( CMultiRoom *pRoom, int nClassID )
	: CDnNormalMonsterActor( pRoom, nClassID )
{
}

CDnGoldMonsterActor::~CDnGoldMonsterActor()
{
}

void CDnGoldMonsterActor::OnDie( DnActorHandle hHitter )
{
	DNTableFileFormat* pGoldTable = GetDNTable( CDnTableDB::TMONSTERGOLDDROP );
	if( pGoldTable )
	{
		int nGoldDropID = GetMonsterClassID();
		if( pGoldTable->IsExistItem( nGoldDropID ) )
		{
			DNTableCell* pGoldCell = pGoldTable->GetFieldFromLablePtr( nGoldDropID, "_Gold" );
			if( !pGoldCell )
				pGoldCell = pGoldTable->GetFieldFromLablePtr( nGoldDropID, "_DropGold" );
			int nBonusGold = pGoldCell ? pGoldCell->GetInteger() : 0;

			DNTableCell* pCountCell = pGoldTable->GetFieldFromLablePtr( nGoldDropID, "_DropCount" );
			int nDropCount = ( pCountCell && pCountCell->GetInteger() > 0 ) ? pCountCell->GetInteger() : 1;

			if( nBonusGold > 0 )
			{
				int nPerDrop = max( 1, nBonusGold / nDropCount );
				for( int i = 0; i < nDropCount; ++i )
				{
					CDnItem::DropItemStruct Struct;
					Struct.nItemID = 0;
					Struct.nSeed = abs( _roomrand( GetRoom() ) );
					Struct.cOption = 0;
					Struct.nCount = nPerDrop;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					Struct.nEnchantID = 0;
#endif
					Struct.dwUniqueID = CDnDropItem::AllocUniqueID( GetRoom() );
					m_VecDropItemList.push_back( Struct );
				}
			}
		}
	}

	CDnNormalMonsterActor::OnDie( hHitter );
}
