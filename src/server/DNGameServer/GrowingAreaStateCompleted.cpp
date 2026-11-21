
#include "stdafx.h"
#include "GrowingAreaStateCompleted.h"
#include "GrowingAreaStateHarvesting.h"
#include "GrowingArea.h"
#include "DNDBConnection.h"
#include "DNFarmGameRoom.h"
#include "DNUserSession.h"


void CFarmAreaStateCompleted::BeginProcess()
{
	// Update
	if( m_pArea->GetFarmGameRoom() )
	{
		CDNFarmGameRoom* pFarmGameRoom	= m_pArea->GetFarmGameRoom();
		CDNDBConnection* pDBCon			= m_pArea->GetFarmGameRoom()->GetDBConnection();
		std::vector<int> vUpdateAreaIndex;
		std::vector<int> vUpdateElapsedTimeSec;
		
		vUpdateAreaIndex.push_back( m_pArea->GetIndex() );
		vUpdateElapsedTimeSec.push_back( m_pArea->GetPlantSeed()->GetRequiredTimeSec() );

		if( m_pArea->bIsPrivateArea() == true )
			pDBCon->QueryModFieldForCharacterElapsedTime( pFarmGameRoom->GetDBThreadID(), pFarmGameRoom->GetWorldSetID(), 0, m_pArea->GetOwnerCharacterDBID(), vUpdateAreaIndex, vUpdateElapsedTimeSec );
		else
			pDBCon->QueryModFieldElapsedTime( pFarmGameRoom->GetDBThreadID(), pFarmGameRoom->GetWorldSetID(), 0, pFarmGameRoom->GetFarmIndex(), vUpdateAreaIndex, vUpdateElapsedTimeSec );
	}

	m_pArea->GetPlantSeed()->ResetResultItem();
	SendBeginAreaState();

#if defined( PRE_ADD_FARM_DOWNSCALE )
	// Auto-harvest if in forced harvest state, or if not a private area and the farm is closed.
	if( m_pArea->GetPlantSeed()->bIsForceComplete() || (m_pArea->GetFarmGameRoom() && m_pArea->GetFarmGameRoom()->GetAttr()&Farm::Attr::DownScale && m_pArea->bIsPrivateArea() == false) )
	{
		m_pArea->QueryCompleteHarvest( NULL, NULL, true );
	}
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
}

void CFarmAreaStateCompleted::Process( float fDelta )
{

}

void CFarmAreaStateCompleted::EndProcess()
{
	if( m_pArea->GetState() == Farm::AreaState::HARVESTING )
	{
		CFarmAreaStateHarvesting* pState = static_cast<CFarmAreaStateHarvesting*>(m_pArea->GetStatePtr());
		
		CDNUserSession* pSession = m_pArea->GetUserSession( pState->GetHarvestingCharacterDBID() );
		if( pSession )
			pSession->SendFarmHarvest( ERROR_NONE, Farm::ActionType::START, m_pArea->GetIndex() );
	}
}
