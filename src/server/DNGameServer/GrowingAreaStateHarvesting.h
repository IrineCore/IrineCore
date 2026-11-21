
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateHarvesting:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateHarvesting>
{
public:

	CFarmAreaStateHarvesting( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
		m_bQuery					= false;
		m_biHarvestingCharacterDBID = 0;
	}

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::HARVESTING; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// CheckState
	virtual bool					CanCompleteHarvest();					// State where harvest can be completed?
	virtual bool					CanCancelHarvest(){ return true; }		// State where harvest can be canceled?

	// HarvestInfo
	void	SetHarvestInfo( CDNUserSession* pSession );
	INT64	GetHarvestingCharacterDBID() const { return m_biHarvestingCharacterDBID; }

	void	SetQueryFlag( bool bFlag ){ m_bQuery = bFlag; }
	bool	GetQueryFlag(){ return m_bQuery; }

protected:

	bool			m_bQuery;
	INT64			m_biHarvestingCharacterDBID;
	std::wstring	m_wstrHarvestingCharacterName;
};
