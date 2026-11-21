
#pragma once

#include "PvPAllKillMode.h"
#include "DnActor.h"

class IPvPAllKillModeState;

#if defined(PRE_ADD_PVP_TOURNAMENT)
class CPvPTournamentMode:public CPvPAllKillMode
{
public:	
	CPvPTournamentMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket );

	virtual ~CPvPTournamentMode();

	virtual bool	bIsAllKillMode(){ return false; }
	virtual bool	bIsTournamentMode(){ return true; }		
	virtual void	OnSuccessBreakInto( CDNUserSession* pGameSession );	
	virtual void	SetTournamentUserInfo(MAGAPVP_TOURNAMENT_INFO* pPacket);	
	virtual void	ChangeState( eState State );
	virtual void	_BeforeProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual void	_AfterProcess( LOCAL_TIME LocalTime, float fDelta );
	virtual void	OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason );
	virtual BYTE	GetTournamentRanking(UINT uiSessionID);	
	virtual void	OnLeaveUser( DnActorHandle hActor );

	bool CheckTournamentActiveActor();
	bool CheckLastRound();
	void GetTournamentActiveActorIndex(char cStep, char& cATeamIndex, char& cBTeamIndex);
	void SendTournamentMatchList(CDNUserSession* pBreakIntoUserSession=NULL);
	void SendTournamentDefaultWin(char cTournamentStep, UINT uiWinSessionID);
	void SendIdleTick(UINT uiIdleTick);
	void SendTournamentTop4();
	void SendLastResult();
#if defined( PRE_WORLDCOMBINE_PVP )	
	virtual void	SetWorldPvPRoomStart();
	virtual void	SetTournamentUserInfo(int nIndex, CDNUserSession* pGameSession, bool bIgnorePrevData = true);
	virtual void	SetTournamentUserInfo();
	void			SendTournamentUserInfo();
	bool			CheckTournamentTeam();
#endif

#if defined( PRE_PVP_GAMBLEROOM )
	void AddGambleMemberDB();
#endif

protected:	
	virtual bool	_CheckWinContition(){ return false; }
	virtual bool	_CheckFinishGameMode( UINT& uiWinTeam );
	virtual UINT	_GetPvPRoundStartStateEffectDurationTick(){ return PvPCommon::Common::GameModeStartDelta*1000; }
	virtual void	_ProcessFinishRoundMode( CDNUserSession* pSession, bool bIsWin, UINT uiWinTeam, PvPCommon::FinishReason::eCode Reason );

	char GetTournamentActorIndex(PvPCommon::TeamIndex::eCode Team);
	void SetTournamentActorIndex( PvPCommon::TeamIndex::eCode Team, BYTE cActorIndex );	

private:	
	BYTE m_cTournamentType;
	BYTE m_cTournamentArea;			// Group by 2, 4, 8, 16 according to each round.
	STournamentMatchInfo m_sTournamentMatchInfo[PvPCommon::Common::PvPTournamentUserMax];
	BYTE m_cTournamentActorIndex[PvPCommon::TeamIndex::Max];
};

#endif // #if defined( PRE_ADD_PVP_TOURNAMENT )