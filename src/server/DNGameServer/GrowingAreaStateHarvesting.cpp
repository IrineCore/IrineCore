
#include "stdafx.h"
#include "GrowingAreaStateHarvesting.h"
#include "DNFarmGameRoom.h"
#include "GrowingArea.h"
#include "DNUserSession.h"


void CFarmAreaStateHarvesting::BeginProcess()
{
	_ASSERT( m_biHarvestingCharacterDBID > 0 );
	if( m_biHarvestingCharacterDBID > 0 )
	{
		CDNUserSession* pSession = m_pArea->GetUserSession( m_biHarvestingCharacterDBID );
		if( pSession )
			pSession->BroadcastingEffect( EffectType_Harvest, EffectState_Start );
	}

	SendBeginAreaState();
}

void CFarmAreaStateHarvesting::Process( float fDelta )
{
	if( GetQueryFlag() == true )
		return;

	if( m_pArea->GetElapsedStateTick() >= Farm::Max::HARVESTING_TICK+Farm::Max::EXTRA_SERVER_TICK )
		m_pArea->ChangeState( Farm::AreaState::COMPLETED );
}

void CFarmAreaStateHarvesting::EndProcess()
{
	_ASSERT( m_biHarvestingCharacterDBID > 0 );
	if( m_biHarvestingCharacterDBID > 0 )
	{
		int						iRet			= ERROR_NONE;
		Farm::ActionType::eType	ActionType		= Farm::ActionType::CANCEL;
		std::vector<int>		vResultItem;

		if( m_pArea->GetState() == Farm::AreaState::HARVESTED )
		{
			ActionType		= Farm::ActionType::COMPLETE;
			vResultItem		= m_pArea->GetPlantSeed()->GetResultItems();
		}
		else if( m_pArea->GetState() == Farm::AreaState::COMPLETED )
		{
			if( GetQueryFlag() )
			{
				ActionType	= Farm::ActionType::COMPLETE;
				iRet		= ERROR_FARM_CANT_COMPLETE_HARVESTSTATE;
			}
		}

		CDNUserSession* pSession = m_pArea->GetUserSession( GetHarvestingCharacterDBID() );
		if( pSession )
		{
			pSession->SendFarmHarvest( iRet, ActionType, m_pArea->GetIndex(), vResultItem );
			pSession->BroadcastingEffect( EffectType_Harvest, EffectState_Cancel );
		}
	}
}

bool CFarmAreaStateHarvesting::CanCompleteHarvest()
{
	// Check time
	if( m_pArea->GetElapsedStateTick() < static_cast<DWORD>(Farm::Max::HARVESTING_TICK*Farm::ElapsedSyncPer) )
		return false;

	return true;
}

// HarvestInfo
void CFarmAreaStateHarvesting::SetHarvestInfo( CDNUserSession* pSession )
{
	m_biHarvestingCharacterDBID		= pSession->GetCharacterDBID();
	m_wstrHarvestingCharacterName	= pSession->GetCharacterName();
}
