
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateAddWater:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateAddWater>
{
public:

	CFarmAreaStateAddWater( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
		m_fElapsedDelta		= 0.f;
		m_biCharacterDBID	= 0;
		m_bIsFinish			= false;
		m_bIsCancel			= false;
		m_bIsQuery			= false;
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::ADDWATER; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// Set
	void	SetCancelFlag( bool bFlag ){ m_bIsCancel = bFlag; }
	void	SetQueryFlag( bool bFlag ){ m_bIsQuery = bFlag; }

	// Get
	bool	GetQueryFlag(){ return m_bIsQuery; }
	bool	bIsFinish(){ return m_bIsFinish; }
	INT64	GetCharacterDBID() const { return m_biCharacterDBID; }
	DWORD	GetElapsedTick() const { return static_cast<DWORD>(m_fElapsedDelta*1000); }

	// CheckState

	// HarvestInfo
	void	SetUserInfo( CDNUserSession* pSession );

private:

	float	m_fElapsedDelta;
	INT64	m_biCharacterDBID;	// Info of Character currently watering
	bool	m_bIsFinish;
	bool	m_bIsCancel;
	bool	m_bIsQuery;
};
