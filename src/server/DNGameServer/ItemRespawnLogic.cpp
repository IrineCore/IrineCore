
#include "stdafx.h"
#include "ItemRespawnLogic.h"
#include "EventControlHeader.h"
#include "EtWorldEventArea.h"
#include "DNGameRoom.h"

void CItemRespawnLogic::Reset()
{
	m_vItemRespawn.clear();
}

void CItemRespawnLogic::AddItemRespawnArea( CEtWorldEventArea* pArea )
{
	PvPRespawnItemAreaStruct* pData = reinterpret_cast<PvPRespawnItemAreaStruct*>(pArea->GetData());

	m_vItemRespawn.push_back( SItemRespawn(pData->nRespawnIntervalSec,pData->nItemDropTableID,pArea->GetOBB(),(pData->bIsStartSpawn ? true : false) ) );
}

void CItemRespawnLogic::Process( const float fDelta )
{
	for( UINT i=0 ; i<m_vItemRespawn.size() ; ++i )
	{
		// Check if previous DropItem remains
		if( m_vItemRespawn[i].hLastDropItem )
		{
			m_vItemRespawn[i].Reset();
			continue;
		}

		// Item spawned at start
		if( m_vItemRespawn[i].bIsStartSpawn )
		{
			m_vItemRespawn[i].bIsStartSpawn = false;
			m_vItemRespawn[i].fRespawnDelta = 0.f;
		}

		m_vItemRespawn[i].fRespawnDelta -= fDelta;
		if( m_vItemRespawn[i].fRespawnDelta <= 0.f )
		{
			EtVector3 vPos = m_vItemRespawn[i].sOBB.Center;
			m_vItemRespawn[i].hLastDropItem = m_pGameRoom->RequestItemDropTable( m_vItemRespawn[i].uiItemDropTableID, &vPos );
		}
	}
}

//*******************************************************************
// [PvP] ItemRespawnLogic
//*******************************************************************

void CPvPItemRespawnLogic::FinishRound()
{
	for( UINT i=0 ; i<m_vItemRespawn.size() ; ++i )
	{
		m_vItemRespawn[i].FinishRound();
	}
}
