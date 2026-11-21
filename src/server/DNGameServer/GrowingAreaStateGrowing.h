
#pragma once

#include "GrowingAreaState.h"


class CFarmAreaStateAddWater;

class CFarmAreaStateGrowing:public IFarmAreaState,public TBoostMemoryPool<CFarmAreaStateGrowing>
{
public:

	CFarmAreaStateGrowing( CGrowingArea* pArea )
		:IFarmAreaState(pArea)
	{
		m_pAddWaterState = NULL;
	}

	virtual ~CFarmAreaStateGrowing();

	virtual Farm::AreaState::eState GetState(){ return Farm::AreaState::GROWING; }
	virtual void					BeginProcess();
	virtual void					Process( float fDelta );
	virtual void					EndProcess();

	// CheckState
	virtual bool					CanStartAddWater();									// Is it in a state where watering is possible?
	virtual bool					CanCancelAddWater( CDNUserSession* pSession );		// Is it in a state to cancel watering?
	virtual bool					CanCompleteAddWater( CDNUserSession* pSession );	// Is it in a state to complete watering?
	virtual bool					CanDBUpdate();										// Check if DB update is needed?
	virtual void					RejectReasonLog(WCHAR* wszCondition, CDNUserSession* pSession = NULL);

	//
	void StartAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket );
	void CancelAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket );
	void QueryAddWater();
	void CompleteAddWater( int iRet );

private:

	CFarmAreaStateAddWater*			m_pAddWaterState;

};
