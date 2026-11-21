
#pragma once

#include "EventControlHeader.h"
#include "EtWorldEventArea.h"
#include "DnDropItem.h"

class CEtWorldEventArea;
class CDNGameRoom;
class CPvPItemRespawnLogic;

class CPvPRespawnLogic:public TBoostMemoryPool<CPvPRespawnLogic>
{
public:

	CPvPRespawnLogic( CDNGameRoom* pGameRoom );
	~CPvPRespawnLogic();

	// Process
	void				Process( LOCAL_TIME LocalTime, float fDelta );
	//
	void				FinishInitialize(){ m_bInitialize = true; }
	void				AddRespawnArea( CEtWorldEventArea* pArea );
	void				AddItemRespawnArea( CEtWorldEventArea* pArea );
	void				ResetPoint();
	void				ToggleCheat();
	void				FinishRound();
	//
	void				OnDie( DnActorHandle hActor );
	CEtWorldEventArea*	OnRespawn( DnActorHandle hActor );

	typedef struct tagRespawnPoint
	{
		CEtWorldEventArea*	pArea;
		int					iPoint;
		//
		tagRespawnPoint( CEtWorldEventArea* _pArea ):pArea(_pArea),iPoint(0){}

		// Final preference score
		int GetTotalPoint() const
		{
			PvPRespawnAreaStruct* pStruct = reinterpret_cast<PvPRespawnAreaStruct*>(pArea->GetData());
			return pStruct->nBasePreferScore + iPoint;
		}

		// Check final preference score Range value
		void CheckRange()
		{
			PvPRespawnAreaStruct* pStruct = reinterpret_cast<PvPRespawnAreaStruct*>(pArea->GetData());
			if( pStruct->nBasePreferScore + iPoint > RespawnPoint::eMaxRange )
				iPoint = RespawnPoint::eMaxRange-pStruct->nBasePreferScore;
			else if( pStruct->nBasePreferScore + iPoint < -RespawnPoint::eMaxRange )
				iPoint = -RespawnPoint::eMaxRange-pStruct->nBasePreferScore;
		}
	}SRespawnPoint;

private:

	struct RespawnPoint
	{
		enum
		{
			eDiePoint			= -10,	// Respawn point when our team dies
			eOpponentDiePoint	= 20,	// Respawn point when opposing team dies
			eRespawnPoint		= -10,	// Points granted on each respawn
			eNotUseRespawnPoint	= 5,	// Unused respawn position point
			eMaxRange			= 100,	// Final preference score -,+ MAX
		};
	};

	struct Common
	{
		enum
		{
			eCheatSec			= 1,
		};
	};

	void						_AddRespawnPoint( SRespawnPoint* pRespawnPoint, int iAddPoint );
	SRespawnPoint*				_GetNearestRespawnArea( EtVector3* pVec, const UINT uiTeam );
	void						_SendCheat();

	CDNGameRoom*				m_pGameRoom;
	bool						m_bInitialize;
	bool						m_bCheat;
	float						m_fCheat;
	DNVector(SRespawnPoint)		m_vTeamARespawnPoint;
	DNVector(SRespawnPoint)		m_vTeamBRespawnPoint;

	CPvPItemRespawnLogic*		m_pItemRespawnLogic;

	void						_ProcessItemRespawn( const float fDelta );
};
