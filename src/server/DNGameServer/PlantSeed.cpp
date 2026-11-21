
#include "stdafx.h"
#include "PlantSeed.h"
#include "DNGameDataManager.h"


CPlantSeed::CPlantSeed( int iItemID, DWORD dwElapsedTick, INT64 biCharacterDBID, WCHAR* pwszCharName )
:m_iSeedItemID(iItemID),m_dwElapsedTick(dwElapsedTick),m_biOwnerCharacterDBID(biCharacterDBID),m_wstrOwnerCharacterName(pwszCharName)
,m_dwRequiredTick(0),m_dwUpdateTick(dwElapsedTick),m_iDefaultWater(0),m_iConsumeWaterPerSec(0)
{
	m_iAttachWater			= 0;
	m_iAttachWaterBottle	= 0;
	m_bIsDBCreate			= false;
	m_bForceComplete		= false;

	_LoadData();
}

void CPlantSeed::Process( float fDelta )
{
	if( m_dwElapsedTick >= m_dwRequiredTick )
	{
		m_dwElapsedTick = m_dwRequiredTick;
		return;
	}

	m_dwElapsedTick += static_cast<DWORD>(fDelta*1000);
}

void CPlantSeed::_LoadData()
{
	m_vResultItems.reserve( Farm::Max::HARVESTITEM_COUNT );

	TFarmCultivateTableData* pTableData = g_pDataManager->GetFarmCultivateTableData( m_iSeedItemID );
	if( pTableData == NULL )
	{
		g_Log.Log( LogType::_FARM, L"_LoadData Failed! SeedID:%d", m_iSeedItemID );
		_ASSERT(0);
		return;
	}

	m_dwRequiredTick		= static_cast<DWORD>(pTableData->iCultivateMaxTimeSec*1000);
	m_iDefaultWater			= pTableData->iMaxWater;
	m_iConsumeWaterPerSec	= pTableData->iConsumeWater;
}

bool CPlantSeed::bIsValid()
{
	if( m_dwRequiredTick <= 0 || m_iDefaultWater <= 0 || m_iConsumeWaterPerSec <= 0 )
		return false;

	return true;
}

bool CPlantSeed::bIsComplete()
{
	if( m_dwRequiredTick <= 0 )
	{
		_ASSERT(0);
		return false;
	}

	return (m_dwElapsedTick >= m_dwRequiredTick);
}

bool CPlantSeed::bIsEnoughWater()
{
#if defined( PRE_REMOVE_FARM_WATER )
	return true;
#endif // #if defined( PRE_REMOVE_FARM_WATER )
	int iConsumeWater = GetConsumeWater();

	if( m_iDefaultWater+m_iAttachWaterBottle+m_iAttachWater > iConsumeWater )
		return true;

	return false;
}

bool CPlantSeed::bIsFullWater()
{
	int iConsumeWater = GetConsumeWater();
	if( m_iAttachWaterBottle+m_iAttachWater-iConsumeWater > 0 )
		return true;

	return false;
}

bool CPlantSeed::AttachItem( int iItemID )
{
	std::map<int,int>::iterator itor = m_mAttachItems.find( iItemID );
	if( itor != m_mAttachItems.end() )
	{
		++(*itor).second;
	}
	else
	{
		m_mAttachItems.insert( std::make_pair(iItemID,1) );
	}

#if !defined( _FINAL_BUILD )
	if( m_mAttachItems.size() >= Farm::Max::ATTACHITEM_KIND )
		_ASSERT(0);
#endif // #if !defined( _FINAL_BUILD )

	CalcAttachItem();
	return true;
}

bool CPlantSeed::ForceAttachItem( int iItemID, int iCount )
{
	std::map<int,int>::iterator itor = m_mAttachItems.find( iItemID );
	if( itor != m_mAttachItems.end() )
	{
		_ASSERT(0);
		return false;
	}

	m_mAttachItems.insert( std::make_pair(iItemID,iCount) );

#if !defined( _FINAL_BUILD )
	if( m_mAttachItems.size() >= Farm::Max::ATTACHITEM_KIND )
		_ASSERT(0);
#endif // #if !defined( _FINAL_BUILD )

	return true;
}

void CPlantSeed::CalcAttachItem()
{
	TFarmCultivateTableData* pTableData = g_pDataManager->GetFarmCultivateTableData( m_iSeedItemID );
	if( pTableData == NULL )
	{
		_ASSERT(0);
		return;
	}

	m_dwRequiredTick		= static_cast<DWORD>(pTableData->iCultivateMaxTimeSec*1000);
	m_iAttachWater			= 0;
	m_iAttachWaterBottle	= 0;

	for( std::map<int,int>::iterator itor=m_mAttachItems.begin() ; itor!=m_mAttachItems.end() ; ++itor )
	{
		const TItemData* pItemData = g_pDataManager->GetItemData( (*itor).first );
		if( pItemData == NULL )
		{
			_ASSERT(0);
			continue;
		}
		
		switch( pItemData->nType ) 
		{
			case ITEMTYPE_WATER:
			{
				m_iAttachWater += (pItemData->nTypeParam[0]*(*itor).second);
				break;
			}
			case ITEMTYPE_WATERBOTTLE:
			{
				m_iAttachWaterBottle += (pItemData->nTypeParam[0]*(*itor).second);
				break;
			}
			case ITEMTYPE_GROWING_BOOST:
			{
				for( int i=0 ; i<(*itor).second ; ++i )
				{
					INT64 biDecreaseTime = (INT64)pTableData->iCultivateMaxTimeSec * 1000 * pItemData->nTypeParam[0]/100;
					m_dwRequiredTick -= (DWORD)biDecreaseTime;
				}
				break;
			}
		}
	}
}

bool CPlantSeed::PushResultItem( int iItemID )
{
	if( m_vResultItems.size() >= Farm::Max::HARVESTDEPOT_COUNT )
	{
		g_Log.Log( LogType::_FARM, L"PushResultItem Failed! SeedID:%d ResultItemSize:%d", m_iSeedItemID, m_vResultItems.size() );
		_ASSERT(0);
		return false;
	}

	m_vResultItems.push_back( iItemID );
	return true;
}

bool CPlantSeed::CanDBUpdate()
{
	// Completed state
	if( m_dwElapsedTick == m_dwRequiredTick )
	{
		if( m_dwUpdateTick != m_dwElapsedTick )
		{
			m_dwUpdateTick = m_dwElapsedTick;
			return true;
		}
	}

	if( m_dwUpdateTick+(Farm::Common::SEED_UPDATE_SEC*1000) <= m_dwElapsedTick )
	{
		m_dwUpdateTick = m_dwElapsedTick;
		return true;
	}

	return false;
}

bool CPlantSeed::CanAttach( const TItem* pItem, bool bFirst )
{
	const TItemData* pItemData = g_pDataManager->GetItemData( pItem->nItemID );
	if( pItemData == NULL )
		return false;

	// Filter attachable Items
	if( bFirst )
	{
		switch( pItemData->nType )
		{
			case ITEMTYPE_GROWING_BOOST:	// Growth booster
			{
				break;
			}
			default:
			{
				g_Log.Log( LogType::_FARM, L"CanAttach Failed! SeedID:%d ItemID:%d", m_iSeedItemID, pItem->nItemID );
				_ASSERT(0);
				return false;
			}
		}

	}
	else
	{
		switch( pItemData->nType )
		{
			case ITEMTYPE_WATER:			// Water
			case ITEMTYPE_WATERBOTTLE:		// Watering can
			{
				break;
			}
			default:
			{
				g_Log.Log( LogType::_FARM, L"CanAttach Failed! SeedID:%d ItemID:%d", m_iSeedItemID, pItem->nItemID );
				_ASSERT(0);
				return false;
			}
		}
	}

	// Check duplicate watering can
	if( pItemData->nType == ITEMTYPE_WATERBOTTLE )
	{
		// Water container still remains
		if( m_iAttachWaterBottle > GetConsumeWater() )
			return false;
	}
	else if( pItemData->nType == ITEMTYPE_WATER )
	{
		// Check if stock/quantity is full
		if( bIsFullWater() == true )
			return false;
	}

	if( m_mAttachItems.find( pItem->nItemID ) != m_mAttachItems.end() )
		return true;

	if( m_mAttachItems.size() < Farm::Max::ATTACHITEM_KIND )
		return true;

	return false;
}

void CPlantSeed::MakeAttachItemInfo( TFarmAreaInfo& Info )
{
	Info.cAttachCount = 0;
	for( std::map<int,int>::iterator itor=m_mAttachItems.begin() ; itor!=m_mAttachItems.end() ; ++itor )
	{
		if( Info.cAttachCount >= _countof(Info.AttachItems) )
		{
			_ASSERT(0);
			break;
		}

		Info.AttachItems[Info.cAttachCount].iItemID = (*itor).first;
		Info.AttachItems[Info.cAttachCount].iCount = (*itor).second;
		++Info.cAttachCount;
	}
}

void CPlantSeed::SkipSeed( float fMinusSec )
{
#if !defined( _FINAL_BUILD )
	if( fMinusSec <= 0.f )
		return;
	if( m_dwElapsedTick>static_cast<DWORD>(fMinusSec*1000) )
		m_dwElapsedTick = m_dwRequiredTick-static_cast<DWORD>(fMinusSec*1000);
	else
		m_dwElapsedTick = 0;
	m_dwUpdateTick	= 0;
	m_iAttachWater	= static_cast<int>(m_dwRequiredTick/1000*m_iConsumeWaterPerSec);
#endif // #if !defined( _FINAL_BUILD )
}

bool CPlantSeed::UpdateOwnerCharacterName (MAChangeCharacterName* pPacket)
{
	std::wstring wstrOriginName = pPacket->wszOriginName;

	if (wstrOriginName == m_wstrOwnerCharacterName)
	{
		m_wstrOwnerCharacterName.clear();
		m_wstrOwnerCharacterName = pPacket->wszCharacterName;
		return true;
	}

	return false;
}
