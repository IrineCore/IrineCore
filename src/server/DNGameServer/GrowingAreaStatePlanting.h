
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStatePlanting:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStatePlanting>
{
public:

	CFarmAreaStatePlanting( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
		m_bQuery		= false;
		m_iLastError	= 0;
		m_bCheckOverlapCount = false;
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::PLANTING; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// CheckState
	virtual bool					CanCancelPlantSeed(){ return true; }	// State where seed planting can be canceled?
	virtual bool					CanCompletePlantSeed();					// State where seed planting can be completed?

	void	SetQueryFlag( bool bFlag ){ m_bQuery = bFlag; }
	bool	GetQueryFlag(){ return m_bQuery; }
	void	SetLastError( int iErr ){ m_iLastError = iErr; }
	void	SetCheckOverlapCountFlag( bool bFlag ){ m_bCheckOverlapCount = bFlag; }

private:

	bool	m_bQuery;
	int		m_iLastError;
	bool	m_bCheckOverlapCount;
};
