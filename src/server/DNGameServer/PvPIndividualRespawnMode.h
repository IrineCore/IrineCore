
#pragma once

#include "PvPRespawnMode.h"

class CPvPIndividualRespawnMode:public CPvPRespawnMode,public TBoostMemoryPool<CPvPIndividualRespawnMode>
{
public:

	CPvPIndividualRespawnMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket );
	virtual ~CPvPIndividualRespawnMode();

	virtual bool bIsIndividualMode(){ return true; }

	virtual void OnInitializeActor( CDNUserSession* pSession );
	virtual UINT OnCheckZeroUserWinTeam( const bool bCheckBreakIntoUser=true );
	virtual UINT OnCheckFinishWinTeam();
	virtual void OnLeaveUser( DnActorHandle hActor );
	virtual void OnDie( DnActorHandle hActor, DnActorHandle hHitter );

	void	PushUsedRespawnArea( const std::string& name );
	bool	bIsUsedRespawnArea( const std::string& name );

	long m_lDieCount;	// Related to issue #22708: used as a variable to count deaths in Free-for-All to prevent duplicate respawns.

protected:

	std::list<int>			m_UsableTeamIndex;
	std::list<std::string>	m_UsedRespawnAreaList;
};
