
#pragma once


#include "GrowingAreaState.h"
#include "PlantSeed.h"

class CPlantSeed;
class CDNFarmGameRoom;
class CDnFarmGameTask;

class CGrowingArea:public TBoostMemoryPool<CGrowingArea>
{
public:

	CGrowingArea( CDNFarmGameRoom* pGameRoom, const int iIndex, SOBB* pOBB, bool bPrivate=false, INT64 biCharacterDBID=0 );
	virtual ~CGrowingArea();

	void Process( float fDelta );
	// Plant
	void ProcessStartPlant( CDNUserSession* pSession, CSFarmPlant* pPacket );
	void ProcessCancelPlant( CDNUserSession* pSession, CSFarmPlant* pPacket );
	void QueryCompletePlant( CDNUserSession* pSession, CSFarmPlant* pPacket );
	// Harvest
	void ProcessStartHarvest( CDNUserSession* pSession, CSFarmHarvest* pPacket );
	void ProcessCancelHarvest( CDNUserSession* pSession, CSFarmHarvest* pPacket );
#if defined( PRE_ADD_FARM_DOWNSCALE )
	void QueryCompleteHarvest( CDNUserSession* pSession, CSFarmHarvest* pPacket, bool bAutoHarvest=false );
#else
	void QueryCompleteHarvest( CDNUserSession* pSession, CSFarmHarvest* pPacket );
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	// AddWater
	void ProcessStartAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket );
	void ProcessCancelAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket );
	void QueryCompleteAddWater( CDNUserSession* pSession, const CSFarmAddWater* pPacket );

	void ChangeState( Farm::AreaState::eState State, CDNUserSession* pSession=NULL );
	void InitializeSeed( const TFarmField* pPacket, bool bResetUpdateTick=false );

	void DestroySeed();
	void MakeAreaInfo( TFarmAreaInfo& Info );
	void SendAreaInfo();

	// GET
	int						GetIndex() const { return m_iIndex; }
	IFarmAreaState*			GetStatePtr() const { return m_pState; }
	Farm::AreaState::eState	GetState() const { return m_pState->GetState(); }
	INT64 GetOwnerCharacterDBID() const 
	{ 
		if( bIsPrivateArea() )
			return m_biPrivateCharacterDBID;
		return m_pSeed ? m_pSeed->GetOwnerCharacterDBID() : 0;
	}
	DWORD					GetElapsedStateTick() const { return m_dwStateElapsedTick; }
	CDNFarmGameRoom*		GetFarmGameRoom() const { return m_pFarmGameRoom; }
	CDnFarmGameTask*		GetFarmGameTask() const { return m_pFarmGameTask; }
	CPlantSeed*				GetPlantSeed() const { return m_pSeed; }
	CDNUserSession*			GetUserSession( INT64 biCharacterDBID );
	UINT					GetRandSeed();
	bool					bIsPrivateArea() const { return m_bPrivateArea; }

	// AreaState
	bool				CanStartPlantSeed( CDNUserSession* pSession );
	bool				CanCompletePlantSeed( INT64 biCharacterDBID );
	bool				CanCancelPlantSeed( CDNUserSession* pSession );
	bool				CanStartHarvest( CDNUserSession* pSession );
	bool				CanCompleteHarvest( INT64 biCharacterDBID );
	bool				CanCancelHarvest( CDNUserSession* pSession );
	bool				CanStartAddWater( CDNUserSession* pSession, const TItem* pItem );
	bool				CanCancelAddWater( CDNUserSession* pSession );
	bool				CanCompleteAddWater( CDNUserSession* pSession );
	bool				CanDBUpdate(){ return m_pState->CanDBUpdate(); }

	// Check
	bool				CheckPlantItem( CDNUserSession* pSession, CSFarmPlant* pPacket, const TItem*& pItem, const TItem*& pAttachItem );
	bool				CheckHarvestItem( CDNUserSession* pSession, CSFarmHarvest* pPacket );
	bool				CheckAddWaterItem( CDNUserSession* pSession, const CSFarmAddWater* pPacket, const TItem*& pItem );

private:

	void	_DeleteProcess();

	const int						m_iIndex;
	UINT							m_uiRandSeed;
	CDNFarmGameRoom*				m_pFarmGameRoom;
	CDnFarmGameTask*				m_pFarmGameTask;
	IFarmAreaState*					m_pState;
	std::vector<IFarmAreaState*>	m_vDeleteState;
	CPlantSeed*						m_pSeed;
	
	DWORD							m_dwStateElapsedTick;
	SOBB							m_OBB;			// Area position info

	long							m_lChangeStateLock;
	bool							m_bPrivateArea;
	INT64							m_biPrivateCharacterDBID;
};

