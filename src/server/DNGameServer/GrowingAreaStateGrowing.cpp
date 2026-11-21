
#include "stdafx.h"
#include "GrowingAreaStateGrowing.h"
#include "GrowingAreaStateAddWater.h"
#include "PlantSeed.h"
#include "GrowingArea.h"
#include "DNFarmGameRoom.h"
#include "DNDBConnection.h"
#include "DNUserSession.h"


CFarmAreaStateGrowing::~CFarmAreaStateGrowing()
{
	SAFE_DELETE( m_pAddWaterState );
}

void CFarmAreaStateGrowing::BeginProcess()
{
	_ASSERT( m_pArea->GetPlantSeed() );
	
	m_pArea->GetPlantSeed()->SetDBCreateFlag( true );
	m_pArea->SendAreaInfo();
	SendBeginAreaState();
}

void CFarmAreaStateGrowing::Process( float fDelta )
{
	CPlantSeed* pSeed = m_pArea->GetPlantSeed();
	pSeed->Process( fDelta );
	bool bIsCheckWater = true;
	if( m_pAddWaterState )
	{
		m_pAddWaterState->Process( fDelta );
		if( m_pAddWaterState->bIsFinish() )
		{
			m_pAddWaterState->EndProcess();
			SAFE_DELETE( m_pAddWaterState );
		}
		if( m_pAddWaterState )
		{
			bIsCheckWater = m_pAddWaterState->GetQueryFlag() ? false : true;
		}
	}

	// Skip water check on forced harvest
	if( pSeed->bIsForceComplete() == true )
		bIsCheckWater = false;

	if( bIsCheckWater && pSeed->bIsEnoughWater() == false )
		m_pArea->ChangeState( Farm::AreaState::NONE );
	else if( pSeed->bIsComplete() )
		m_pArea->ChangeState( Farm::AreaState::COMPLETED );
}

void CFarmAreaStateGrowing::EndProcess()
{
	if( m_pArea->GetState() == Farm::AreaState::NONE )
	{
		if( m_pAddWaterState )
		{
			m_pAddWaterState->SetCancelFlag( true );
			m_pAddWaterState->EndProcess();
		}
	}

	if( m_pAddWaterState )
	{
		m_pAddWaterState->EndProcess();
		SAFE_DELETE( m_pAddWaterState );
	}
}

bool CFarmAreaStateGrowing::CanDBUpdate()
{
	_ASSERT( m_pArea && m_pArea->GetPlantSeed() );
	if( m_pArea == NULL || m_pArea->GetPlantSeed() == NULL )
		return false;

	return m_pArea->GetPlantSeed()->CanDBUpdate();
}

bool CFarmAreaStateGrowing::CanStartAddWater()
{ 
	if( m_pAddWaterState )
		return false;

	return true; 
}

void CFarmAreaStateGrowing::RejectReasonLog(WCHAR* wszReason, CDNUserSession* pSession)
{
	if (m_pAddWaterState)
		g_Log.Log(LogType::_FARM, pSession, L"CFarmAreaStateGrowing [Reason:%s][CHRID:%d]\n", wszReason, m_pAddWaterState->GetCharacterDBID());
	else
		g_Log.Log(LogType::_FARM, pSession, L"CFarmAreaStateGrowing [Reason:%s]\n", wszReason);
}

bool CFarmAreaStateGrowing::CanCancelAddWater( CDNUserSession* pSession )
{
	if( m_pAddWaterState == NULL )
		return false;

	if( m_pAddWaterState->GetCharacterDBID() != pSession->GetCharacterDBID() )
		return false;

	return true;
}

bool CFarmAreaStateGrowing::CanCompleteAddWater( CDNUserSession* pSession )
{
	if( m_pAddWaterState == NULL )
		return false;

	if( m_pAddWaterState->GetCharacterDBID() != pSession->GetCharacterDBID() )
		return false;

	if( m_pAddWaterState->GetElapsedTick() < static_cast<DWORD>(Farm::Max::ADDWATERING_TICK*Farm::ElapsedSyncPer) )
		return false;

	return true;
}

void CFarmAreaStateGrowing::StartAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket )
{
	m_pAddWaterState = new CFarmAreaStateAddWater( m_pArea );
	m_pAddWaterState->SetUserInfo( pSession );
	m_pAddWaterState->BeginProcess();
}

void CFarmAreaStateGrowing::CancelAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket )
{
	m_pAddWaterState->SetCancelFlag( true );
	m_pAddWaterState->EndProcess();
	SAFE_DELETE( m_pAddWaterState );
}

void CFarmAreaStateGrowing::QueryAddWater()
{
	m_pAddWaterState->SetQueryFlag( true );
}

void CFarmAreaStateGrowing::CompleteAddWater( int iRet )
{
	if( iRet != ERROR_NONE )
		m_pAddWaterState->SetCancelFlag( true );

	m_pAddWaterState->EndProcess();
	SAFE_DELETE( m_pAddWaterState );
}
