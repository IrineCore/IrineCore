
#pragma once

#include "PvPRoundModeScoreSystem.h"

class CPvPAllKillModeScoreSystem:public CPvPRoundModeScoreSystem,public TBoostMemoryPool<CPvPAllKillModeScoreSystem>
{
public:

	CPvPAllKillModeScoreSystem()
	{
#if defined( _GAMESERVER )
		m_uiContinuousCount = 0;
#endif // #if defined( _GAMESERVER )
	}
	virtual ~CPvPAllKillModeScoreSystem(){}

#if defined( _GAMESERVER )
	virtual void OnFinishRound( CDNGameRoom* pGameRoom, UINT uiWinTeam );
	DnActorHandle GetPreWinActor() const { return m_hPreWinActor; }

protected:

	void _SendContinuousInfo( CDNGameRoom* pGameRoom );
	void _SendFinishDetailReason( CDNGameRoom* pGameRoom, PvPCommon::FinishDetailReason::eCode Reason );
	void _ClearPreWinActor();

#endif // #if defined( _GAMESERVER )

protected:

#if defined( _GAMESERVER )
	DnActorHandle m_hPreWinActor;
	UINT m_uiContinuousCount;
#endif // #if defined( _GAMESERVER )
};
