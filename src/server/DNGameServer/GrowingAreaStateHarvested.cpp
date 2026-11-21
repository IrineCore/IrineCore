
#include "stdafx.h"
#include "GrowingAreaStateHarvested.h"
#include "DNUserSession.h"
#include "GrowingArea.h"
#include "DNGameDataManager.h"
#include "DNMasterConnectionManager.h"
#include "DNFarmGameRoom.h"


void CFarmAreaStateHarvested::BeginProcess()
{
	if( m_pArea->GetPlantSeed() )
	{
		// Removed from DB because it was harvested.
		m_pArea->GetPlantSeed()->SetDBCreateFlag( false );

		// Sync harvest warehouse item count
		g_pMasterConnectionManager->SendFarmSync( m_pArea->GetFarmGameRoom()->GetWorldSetID(), m_pArea->GetPlantSeed()->GetOwnerCharacterDBID(), Farm::ServerSyncType::WAREHOUSE_ITEMCOUNT );
	}

	SendBeginAreaState();
}

void CFarmAreaStateHarvested::Process( float fDelta )
{
	m_pArea->ChangeState( Farm::AreaState::NONE );
}

void CFarmAreaStateHarvested::EndProcess()
{
	
}

// HarvestedInfo
void CFarmAreaStateHarvested::SetHarvestedInfo( CDNUserSession* pSession )
{
	if( pSession )
		m_biHarvestedCharacterDBID = pSession->GetCharacterDBID();
}
