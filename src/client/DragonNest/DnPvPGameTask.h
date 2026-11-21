#pragma once
#include "DnGameTask.h"
#include "DnDamageBase.h"
#include "DnActor.h"
#include "DnMutatorGame.h"

const float fInGameKickTime = 180.0f;

class CDnMutatorGame;
#ifdef PRE_ADD_PVP_TOURNAMENT
class CDnPVPTournamentDataMgr;
#endif
class CDnPvPGameTask:public CDnGameTask
{
protected:

	CDnMutatorGame * m_pMutatorClass;

	LOCAL_TIME m_lKickTime;
	DWORD	m_dwKey;
	byte m_cGameMode;
	
	bool m_IsStartRegulation;
	bool	m_isInGameJoin;	//Whether entered via drop-in
	int m_nLadderType;

	float m_fPlayerDieRespawnDelta;
	float m_fKickElapsedTime;
	int nObjectiveUIString;
	int m_iKOItemDropTableID;
	int m_nMedalExp;
	int	m_nWaringNum;
	int m_nBattleGroundID;
	int m_nNextObservingPartyPlayerIndex;

#ifdef PRE_MOD_PVPOBSERVER	
	int m_nCrrObservingPlayerIndex;
	UINT m_uiSessionBattlePlayer[2];
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_ADD_PVP_COMBOEXERCISE
	bool m_bPvPComboModeMaster; // Whether host in combo practice mode.
#endif // PRE_ADD_PVP_COMBOEXERCISE

protected:

	virtual DnActorHandle InitializePlayerActor( CDnPartyTask::PartyStruct *pStruct ,int nPartyIndex , bool bLocalPlayer, bool bIsBreakInto=false );
	virtual void ShowDebugInfo();
	bool OnPostInitializeStage();

public:
	
	CDnPvPGameTask();
	virtual ~CDnPvPGameTask();

	virtual bool IsUseRegulation();
	virtual bool OnInitializeBreakIntoActor( CDnPartyTask::PartyStruct* pStruct, const int iVectorIndex );
	virtual bool Initialize();

	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	virtual void OnInitializeStartPosition( CDnPartyTask::PartyStruct *pStruct, const int iPartyIndex, EtVector3& vStartPos, float& fRotate, const UINT uiSessionID );

	float GetPlayerDieRespawnDelta() { return m_fPlayerDieRespawnDelta; }
	int GetMedalExp() { return m_nMedalExp; }

	CDnMutatorGame * GetMutator() { return m_pMutatorClass; }
	byte GetGameMode(){return m_cGameMode;}
	bool IsLadderMode() { return m_nLadderType > 0 ? true : false; }
	int GetLadderType() { return m_nLadderType; }

#ifdef PRE_MOD_PVPOBSERVER
	void RotateObservingCamera( bool bNext, bool bFreeView = false );	
	bool GetObservingTargetActor( CDnPartyTask *pPartyTask, int tergetIndex, CDnPlayerActor ** pObservingTarget/*OUT*/ ); // // Detect spectate target player.
	bool IsMatchedBattlePlayer( UINT uiSessionID );
#endif // PRE_MOD_PVPOBSERVER

#ifdef PRE_ADD_PVP_TOURNAMENT
	CDnPVPTournamentDataMgr* GetTournamentDataMgr() const;
#endif

	bool IsShowPartyGauge();
	void RotatePartyPlayerCamera(bool bFreeView = false);
	void ResetPlayerCamera();
	int GetBattleGroundID() { return m_nBattleGroundID; }
	void ProcessAbsenceKick( LOCAL_TIME LocalTime, float fDelta );

	virtual void SetSyncComplete( bool bFlag );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessExitUser( DnActorHandle hActor );
	virtual bool InitializeAllPlayerActor();

	virtual void OnRecvFinishPvPMode( void* pData );
	virtual void OnRecvFinishPvPRound( void* pData );
	virtual void OnRecvStartPVPRound( void* pData );
	virtual void OnRecvModeStartTick( SCPVP_MODE_STARTTICK * pData );
	virtual void OnRecvPartyReadyGate( SCGateInfo *pPacket );
	virtual void OnRecvBreakIntoSuccess( SCPVP_SUCCESSBREAKINTO *pData );
	virtual void OnRecvRespawnPoint ( SCPVP_RESPAWN_POINT * pData );
	virtual void OnRecvModeScrore( SCPVP_MODE_SCORE * pData );
	virtual void OnRecvCaptainKillCount( SCPVP_CAPTAINKILL_COUNT * pData );
	virtual void OnRecvPvPAddPoint( char* pData );
	virtual void OnRecvPvPSelectCaptain( SCPVP_SELECTCAPTAIN *pData);
	virtual void OnRecvPvPSelectZombie( SCPVP_SELECTZOMBIE *pData);
	virtual void OnRecvPVPAddGhoulScore(SCPVP_HOLYWATER_KILLCOUNT *pData);
	virtual void OnRecvPvPFatigueReward( SCPVP_FATIGUE_REWARD *pData );
	virtual void OnRecvPvPTryAcquirePoint( SCPvPTryAcquirePoint * pData );
	virtual void OnRecvPvPOccupationPointState( SCPvPOccupationState * pData);
	virtual void OnRecvPvPOccupationTeamState( SCPvPOccupationTeamState * pData);
	virtual void OnRecvOccupationScore( TPvPOccupationScore * pData );
	virtual void OnRecvConcentrateOrder( SCConcentrateOrder *pData );
	virtual void OnRecvOccupationClimaxMode();

	virtual void OnRecvAllKillModeShowSelectPlayer( SCPVP_ALLKILL_SHOW_SELECTPLAYER *pData );
	virtual void OnRecvAllKillModeActivePlayer( SCPVP_ALLKILL_ACTIVEPLAYER *pData );
	virtual void OnRecvAllKillModeSelectGroupCaptain( SCPVP_ALLKILL_GROUPCAPTAIN *pData );
	virtual void OnRecvAllKillContinousWinCount( SCPVP_ALLKILL_CONTINUOUSWIN *pData );
	virtual void OnRecvAllKillFinishDetailReason( SCPVP_ALLKILL_FINISHDETAILREASON *pData );
	virtual void OnRecvAllKillBattlePlayer( SCPVP_ALLKILL_BATTLEPLAYER *pData );

#ifdef PRE_ADD_PVP_TOURNAMENT
	void OnRecvPVPTournamentBattlePlayer(SCPVP_ALLKILL_BATTLEPLAYER *pData);
	virtual void OnRecvPVPTournamentGameMatchList(SCPvPTournamentMatchList* pData);
	virtual void OnRecvPVPTournamentDefaultWin(SCPvPTournamentDefaultWin* pData);
	virtual void OnRecvPVPTournamentIdleTick(SCPvPTournamentIdleTick* pData);
	virtual void OnRecvPVPTournamentTop4(SCPvPTournamentTop4* pData);
#endif

#ifdef PRE_WORLDCOMBINE_PVP
	virtual void OnRecvWorldPVPRoomStartMsg( WorldPvPMissionRoom::SCWorldPvPRoomStartMsg* pPacket );
	virtual void OnRecvWorldPVPRoomStartResult( WorldPvPMissionRoom::SCWorldPvPRoomStartReturn* pPacket );
	virtual void OnRecvWorldPVPRoomJoinResult( WorldPvPMissionRoom::SCWorldPvPRoomJoinResult* pPacket );
	virtual void OnRecvWorldPVPRoomAllKillTeamInfo( WorldPvPMissionRoom::SCWorldPvPRoomAllKillTeamInfo* pPacket );
	virtual void OnRecvWorldPVPRoomTournamentUserInfo( WorldPvPMissionRoom::SCWorldPvPRoomTournamentUserInfo* pPacket );
#endif // PRE_WORLDCOMBINE_PVP

#if defined( PRE_ADD_RACING_MODE )
	virtual void OnRecvRacingRapTime( SCPVP_RACING_RAPTIME * pData );
	virtual void OnRecvRacingFirstEnd( SCPVP_RACING_FIRST_END * pData );
#endif	// #if defined( PRE_ADD_RACING_MODE )


#ifdef PRE_ADD_PVP_COMBOEXERCISE
	virtual void OnRecvPVPComboModeMaster( SCPvPComboExerciseRoomMasterInfo * pData );
	virtual void OnRecvPVPComboModeChangeMaster( SCPvPComboExerciseChangeRoomMaster * pData );
	virtual void OnRecvPVPComboModeMonsterGen( SCPvPComboExerciseRecallMonsterResult * pData );

	// Request monster summon.
	void SummonDummyMonster();

	bool IsPvPComboModeMaster(){ 
		return m_bPvPComboModeMaster; // Whether host in combo practice mode.
	}
#endif // PRE_ADD_PVP_COMBOEXERCISE

	// ScoreSystem
	virtual void OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage );
	virtual void OnDie( DnActorHandle hActor, DnActorHandle hHitter );
	virtual void OnGhost( DnActorHandle hActor ) {}
	virtual void OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo );

};
