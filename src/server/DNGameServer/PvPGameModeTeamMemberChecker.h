
#pragma once

#include "PvPGameModeChecker.h"

class CPvPGameModeTeamMemberChecker:public IPvPGameModeChecker,public TBoostMemoryPool<CPvPGameModeTeamMemberChecker>
{
public:

	CPvPGameModeTeamMemberChecker( CPvPGameMode* pGameMode ):IPvPGameModeChecker(pGameMode){}
	virtual ~CPvPGameModeTeamMemberChecker(){}

	virtual void Check()
	{
		if( !m_pPvPGameMode )
			return;

		if( !(m_pPvPGameMode->GetGameModeCheck()&PvPCommon::Check::CheckTeamMemberZero) )
			return;

		if( !m_pPvPGameMode->bIsFinishFlag()  )
		{
			// Check if an entire team has left
			if( m_pPvPGameMode->OnCheckZeroUserWinTeam() != PvPCommon::Team::Max )
				m_pPvPGameMode->OnCheckFinishRound( PvPCommon::FinishReason::OpponentTeamAllGone );
		}
	}
};
