
#pragma once

#include "PvPGameModeChecker.h"
#include "DNPvPGameRoom.h"

class CPvPGameModeLadderNoGameChecker:public IPvPGameModeChecker,public TBoostMemoryPool<CPvPGameModeLadderNoGameChecker>
{
public:

	CPvPGameModeLadderNoGameChecker( CPvPGameMode* pGameMode ):IPvPGameModeChecker(pGameMode)
	{
		m_bFirst = true;
	}

	virtual ~CPvPGameModeLadderNoGameChecker(){}

	virtual void Check()
	{
		if( !m_pPvPGameMode )
			return;

		if( m_pPvPGameMode->bIsFinishFlag() )
			return;

		if( m_bFirst == false )
		{
			if( m_pPvPGameMode->GetRemainStartTick() == 0 )
				return;
		}
		m_bFirst = false;

		CDNPvPGameRoom* pPvPGameRoom = static_cast<CDNPvPGameRoom*>(m_pPvPGameMode->GetGameRoom());

		int iTeamNeedCount = 0;
		switch( pPvPGameRoom->GetPvPLadderMatchType() )
		{
			case LadderSystem::MatchType::_1vs1:
			{
				iTeamNeedCount = 1;
				break;
			}
			case LadderSystem::MatchType::_2vs2:
			{
				iTeamNeedCount = 2;
				break;
			}
			case LadderSystem::MatchType::_3vs3:
			{
				iTeamNeedCount = 3;
				break;
			}
			case LadderSystem::MatchType::_4vs4:
			{
				iTeamNeedCount = 4;
				break;
			}
		}

		if( iTeamNeedCount == 0 )
		{
			_ASSERT(0);
			return;
		}

		int iATeamCount = pPvPGameRoom->GetUserCount( PvPCommon::Team::A );
		int iBTeamCount = pPvPGameRoom->GetUserCount( PvPCommon::Team::B );

		if( iATeamCount != iTeamNeedCount || iBTeamCount != iTeamNeedCount )
			m_pPvPGameMode->OnCheckFinishRound( PvPCommon::FinishReason::LadderNoGame );
	}

protected:

	bool m_bFirst;
};
