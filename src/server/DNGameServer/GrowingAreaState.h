
#pragma once


class CGrowingArea;

class IFarmAreaState
{
public:

	IFarmAreaState( CGrowingArea* pArea )
		:m_pArea(pArea)
	{
	}
	virtual ~IFarmAreaState(){}

	virtual Farm::AreaState::eState GetState() = 0;
	virtual void					BeginProcess() = 0;
	virtual void					Process( float fDelta ) = 0;
	virtual void					EndProcess() = 0;

	// CheckState
	virtual bool					CanStartPlantSeed(){ return false; }								// State where seed can be planted?
	virtual bool					CanCancelPlantSeed(){ return false; }								// State where seed planting can be canceled?
	virtual bool					CanCompletePlantSeed(){ return false; }								// State where seed planting can be completed?
	virtual bool					CanStartHarvest(){ return false; }									// State where harvest can be performed?
	virtual bool					CanCancelHarvest(){ return false; }									// State where harvest can be canceled?
	virtual bool					CanCompleteHarvest(){ return false; }								// State where harvest can be completed?
	virtual bool					CanStartAddWater(){ return false; }									// Is it in a state where watering is possible?
	virtual bool					CanCancelAddWater( CDNUserSession* pSession ){ return false; }		// Is it in a state to cancel watering?
	virtual bool					CanCompleteAddWater( CDNUserSession* pSession ){ return false; }	// Is it in a state to complete watering?
	virtual bool					CanDBUpdate(){ return false; }										// Check if DB update is needed?
	virtual void					RejectReasonLog(WCHAR* wszReason, CDNUserSession* pSession=NULL) {}

	void SendBeginAreaState();


protected:

	CGrowingArea* m_pArea;
};
