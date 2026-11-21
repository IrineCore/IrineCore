
#include "stdafx.h"
#include "PvPRespawnLogic.h"
#include "DnActor.h"
#include "DNGameRoom.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "DnGameTask.h"
#include "ItemRespawnLogic.h"

CPvPRespawnLogic::CPvPRespawnLogic( CDNGameRoom* pGameRoom ):m_pGameRoom(pGameRoom),m_bInitialize(false),m_bCheat(false),m_fCheat(Common::eCheatSec)
{
	m_pItemRespawnLogic = new CPvPItemRespawnLogic( pGameRoom );
}

CPvPRespawnLogic::~CPvPRespawnLogic()
{
	SAFE_DELETE( m_pItemRespawnLogic );
}

void CPvPRespawnLogic::Process( LOCAL_TIME LocalTime, float fDelta )
{
	_ASSERT( m_pGameRoom );

	if( !m_pGameRoom->bIsPvPStart() )
		return;

#ifndef _FINAL_BUILD
	if( m_bCheat )
	{
		m_fCheat -= fDelta;
		if( m_fCheat <= 0.f )
		{
			_SendCheat();
			m_fCheat = Common::eCheatSec;
		}
	}
#endif

	_ProcessItemRespawn( fDelta );
}

void CPvPRespawnLogic::AddRespawnArea( CEtWorldEventArea* pArea )
{
	if( m_bInitialize )
		return;

	PvPRespawnAreaStruct* pStruct = reinterpret_cast<PvPRespawnAreaStruct*>(pArea->GetData());

	DNVector(SRespawnPoint)* pVector = NULL;
	switch( pStruct->nTeam )
	{
		case PvPCommon::Team::A:	pVector = &m_vTeamARespawnPoint;	break;
		case PvPCommon::Team::B:	pVector = &m_vTeamBRespawnPoint;	break;
		default:					return;
	}

	pVector->push_back( SRespawnPoint(pArea) );
}

void CPvPRespawnLogic::AddItemRespawnArea( CEtWorldEventArea* pArea )
{
	m_pItemRespawnLogic->AddItemRespawnArea( pArea );
}

void CPvPRespawnLogic::ResetPoint()
{
	for( UINT i=0 ; i<m_vTeamARespawnPoint.size() ; ++i )
		m_vTeamARespawnPoint[i].iPoint = 0;
	for( UINT i=0 ; i<m_vTeamBRespawnPoint.size() ; ++i )
		m_vTeamBRespawnPoint[i].iPoint = 0;
}

void CPvPRespawnLogic::ToggleCheat()
{
#ifdef _FINAL_BUILD
	return;
#endif

	m_bCheat	= (m_bCheat == true) ? false : true;
	m_fCheat	= 0.f;
}

void CPvPRespawnLogic::FinishRound()
{
	_ASSERT( m_pGameRoom );

	if( !CDnDropItem::s_pVecProcessList[m_pGameRoom->GetRoomID()].empty() )
	{
		std::vector<CDnDropItem*> vDropItem;
		vDropItem.reserve( CDnDropItem::s_pVecProcessList[m_pGameRoom->GetRoomID()].size() );

		vDropItem = CDnDropItem::s_pVecProcessList[m_pGameRoom->GetRoomID()];
		
		SAFE_DELETE_PVEC( vDropItem );
	}

	m_pItemRespawnLogic->FinishRound();

	_ASSERT( CDnDropItem::s_pVecProcessList[m_pGameRoom->GetRoomID()].empty() );
}

// Sort
bool SortTotalPoint( const CPvPRespawnLogic::SRespawnPoint& lhs, const CPvPRespawnLogic::SRespawnPoint& rhs )
{
	return lhs.GetTotalPoint() > rhs.GetTotalPoint();
}

CEtWorldEventArea* CPvPRespawnLogic::OnRespawn( DnActorHandle hActor )
{
	if( !hActor )
		return NULL;

	DNVector(SRespawnPoint)* pVector = NULL;
	UINT uiTeam = hActor->GetTeam();
	switch( uiTeam )
	{
		case PvPCommon::Team::A:	pVector = &m_vTeamARespawnPoint;	break;
		case PvPCommon::Team::B:	pVector = &m_vTeamBRespawnPoint;	break;
		default:					return NULL;
	}

	std::sort( pVector->begin(), pVector->end(), SortTotalPoint );
	for( UINT i=0 ; i<pVector->size() ; ++i )
	{
		if( i==0 )
		{
			_AddRespawnPoint( &pVector->at(i), RespawnPoint::eRespawnPoint );
		}
		else
		{
			if( pVector->at(i).iPoint == 0 )
				continue;

			int iAddPoint = pVector->at(i).iPoint > 0 ? -RespawnPoint::eNotUseRespawnPoint : RespawnPoint::eNotUseRespawnPoint;
			_AddRespawnPoint( &pVector->at(i), iAddPoint );
		}
	}

	return pVector->at(0).pArea;
}

void CPvPRespawnLogic::OnDie( DnActorHandle hActor )
{
	if( !hActor )
		return;
	
	// Ally team
	SRespawnPoint* pMyRespawnPoint = _GetNearestRespawnArea( hActor->GetPosition(), hActor->GetTeam() );
	if( !pMyRespawnPoint )
		return;
	_AddRespawnPoint( pMyRespawnPoint, RespawnPoint::eDiePoint );

	// Opposing team
	UINT uiOpponentTeam	= hActor->GetTeam() == PvPCommon::Team::A ? PvPCommon::Team::B : PvPCommon::Team::A;
	SRespawnPoint* pOpponentRespawnPoint = _GetNearestRespawnArea( hActor->GetPosition(), uiOpponentTeam );
	if( !pOpponentRespawnPoint )
		return;
	_AddRespawnPoint( pOpponentRespawnPoint, RespawnPoint::eOpponentDiePoint );
}

void CPvPRespawnLogic::_AddRespawnPoint( SRespawnPoint* pRespawnPoint, int iAddPoint )
{
	pRespawnPoint->iPoint += iAddPoint;
	pRespawnPoint->CheckRange();
}

CPvPRespawnLogic::SRespawnPoint* CPvPRespawnLogic::_GetNearestRespawnArea( EtVector3* pVec, const UINT uiTeam )
{
	DNVector(SRespawnPoint)* pVector = NULL;
	switch( uiTeam )
	{
		case PvPCommon::Team::A:	pVector = &m_vTeamARespawnPoint;	break;
		case PvPCommon::Team::B:	pVector = &m_vTeamBRespawnPoint;	break;
		default:					return NULL;
	}

	int		iFound			= -1;
	float	fMinLengthSq	= FLT_MAX;

	for( UINT i=0 ; i<pVector->size() ; ++i )
	{
		float fLengthSq = EtVec3LengthSq( &(*pVec-pVector->at(i).pArea->GetOBB()->Center) );
		if( fLengthSq < fMinLengthSq )
		{
			fMinLengthSq	= fLengthSq;
			iFound			= i;
		}
	}

	if( iFound >= 0 )
		return &pVector->at(iFound);

	return NULL;
}

void CPvPRespawnLogic::_SendCheat()
{
	_ASSERT( m_pGameRoom );

	std::vector<SRespawnPoint> vRespawnPoint;
	vRespawnPoint.reserve( m_vTeamARespawnPoint.size() + m_vTeamBRespawnPoint.size() );
	vRespawnPoint.insert( vRespawnPoint.end(), m_vTeamARespawnPoint.begin(), m_vTeamARespawnPoint.end() );
	vRespawnPoint.insert( vRespawnPoint.end(), m_vTeamBRespawnPoint.begin(), m_vTeamBRespawnPoint.end() );

	for( UINT j=0 ; j<vRespawnPoint.size() ; ++j )
	{
		SCPVP_RESPAWN_POINT TxPacket;
		memset( &TxPacket, 0, sizeof(TxPacket) );

		_strcpy( TxPacket.szName, _countof(TxPacket.szName), vRespawnPoint[j].pArea->GetName(), static_cast<int>(strlen(vRespawnPoint[j].pArea->GetName())) );
		TxPacket.iPoint				= vRespawnPoint[j].iPoint;
		TxPacket.iBasePreferPoint	= reinterpret_cast<PvPRespawnAreaStruct*>(vRespawnPoint[j].pArea->GetData())->nBasePreferScore;
		TxPacket.unTeam				= reinterpret_cast<PvPRespawnAreaStruct*>(vRespawnPoint[j].pArea->GetData())->nTeam;
		TxPacket.Position			= vRespawnPoint[j].pArea->GetOBB()->Center;

		for( UINT i=0; i<m_pGameRoom->GetUserCount() ; ++i )
		{
			CDNUserSession* pGameSession = m_pGameRoom->GetUserData(i);
			if( !pGameSession )
				continue;

			pGameSession->AddSendData( SC_PVP, ePvP::SC_RESPAWN_POINT, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
		}
	}
}

void CPvPRespawnLogic::_ProcessItemRespawn( const float fDelta )
{
	_ASSERT( m_pGameRoom->GetGameTask() );

	if( !static_cast<CDNPvPGameRoom*>(m_pGameRoom)->bIsDropItemRoom() )
		return;

	m_pItemRespawnLogic->Process( fDelta );
}
