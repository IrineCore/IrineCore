
#pragma once

#include "PvPRoundMode.h"

class CPvPZombieMode:public CPvPRoundMode,public TBoostMemoryPool<CPvPZombieMode>
{
public:

	CPvPZombieMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket );
	virtual ~CPvPZombieMode();

	virtual void	Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void	OnInitializeActor( CDNUserSession* pSession );
	virtual void	OnFinishProcessDie( DnActorHandle hActor );
	virtual void	OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason );
	virtual void	OnFinishRound();
	virtual void	OnDie( DnActorHandle hActor, DnActorHandle hHitter );
	virtual void	OnLeaveUser( DnActorHandle hActor );
	virtual void	OnSuccessBreakInto( CDNUserSession* pGameSession );
	virtual void	OnRebirth( DnActorHandle hActor, bool bForce=false );
	virtual bool	bIsZombieMode(){ return true; }

public:

	bool	bIsZombie( DnActorHandle hActor );
	int GetZombieCount() { return (int)m_mZombieActor.size(); }	
	bool DelZombie(DnActorHandle hActor, bool bRemoveStateBlow=false );

protected:

	virtual void	_OnStartRound();
	virtual void	_ProcessFinishRoundMode( CDNUserSession* pSession, bool bIsWin, UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason );

	void			_ChangeZombie( DnActorHandle hActor, const TMonsterMutationData &Data);
	void			_AddZombieBlow( DnActorHandle hActor, const TMonsterMutationData &Data );
	void			_AddHumanBlow( DnActorHandle hActor );
	void			_ProcessSelectZombie();
	void			_UpdateSelectZombieCount( UINT uiSessionID );
	UINT			_GetSelectZombieCount( UINT uiSessionID );
	void			_GetZombieCandidate( std::vector<DnActorHandle>& vResult );
	
	void InitZombieGroupData();
	bool GetZombieActorData(TMonsterMutationData &Data);

	std::map<DWORD,DnActorHandle>	m_mZombieActor;				// UniqueID/Actor
	std::map<UINT,UINT>				m_mSelectZombieCount;		// SessionID/SlectZombieCount

	bool AddZombie(DnActorHandle hActor, const TMonsterMutationData &Data );

	TMonsterMutationGroup m_MutationGroup;
	int m_nGhoulCount;

private:

	void InitializeZombie();
	void AllReturnHuman();

	bool m_bInitializeZombie;
};
