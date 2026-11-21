
#pragma once

class CEtWorldEventArea;
class CPvPOccupationPoint;
class CPvPOccupationTeam;
class CDNPvPGameRoom;
class CDNUserSession;
class CPvPGameMode;

class CPvPOccupactionSystem : public TBoostMemoryPool<CPvPOccupactionSystem>
{
public:
	CPvPOccupactionSystem(CDNGameRoom * pGameRoom, CPvPGameMode * pMode);
	~CPvPOccupactionSystem();

	//Init
	bool InitializeOccupationSystem(TBattleGourndModeInfo * pModeInfo);

	void Process(LOCAL_TIME LocalTime, float fDelta);

	void OnDie( DnActorHandle hActor, DnActorHandle hHitter );
	void OnDamage(DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage);
	void OnSuccessBreakInto(CDNUserSession * pSession);
	bool OnCheckFinishRound(PvPCommon::FinishReason::eCode Reason, USHORT &nTeam, UINT &nWinGuild, UINT &nLoseGuild, bool bForce = false);
	bool OnTryAcquirePoint(DnActorHandle hActor, int nAreaID, LOCAL_TIME LocalTime);
	bool OnAcquirePoint(const WCHAR * pName, int nAcquireTeam, int nSnatchedTeam, int nAreaID, LOCAL_TIME LocalTime);
	bool OnCancelAcquirePoint(DnActorHandle hActor);
	bool OnTryAcquireSkill(CDNUserSession * pSession, int nSKillID, int nLevel, int nSlotIndex);
	bool OnInitSkill(CDNUserSession * pSession);
	bool OnUseSkill(CDNUserSession * pSession, int nSkillID);
	bool SwapSkillIndex(CDNUserSession * pSession, char cFrom, char cTo);

	//Point
	bool AddOccupationArea(CEtWorldEventArea * pArea, TPositionAreaInfo * pInfo);

	//Team
	bool IsTeamBoss(DnActorHandle hActor);
	CPvPOccupationTeam * AddOccupationTeam(int nTeam, UINT nGuildDBID, int nQualifyingScore);

	//TeamResource
	void GainResource(int nTeam, int nGain);

	//Send
	void SendOccupationPointState(BYTE cCount, PvPCommon::OccupationStateInfo * pInfo, CDNUserSession * pSession = NULL);
	void SendOccupationTeamState(int nTeam, int nResource, int nResourceSum, CDNUserSession * pSession = NULL);
	void SendOccupationSkillState(int nTeam, PvPCommon::OccupationSkillSlotInfo * pInfo, CDNUserSession * pSession = NULL);

	//report
	void SendOccupationrealTimeScore(int nTeam);
	UINT GetRespawnSec();

	//Exception
	bool SetForceWinGuild(UINT nWinGuildDBID);
	void OccupationReportLog(PvPCommon::FinishReason::eCode Reason, int nWinGuildDBID, int nLoseGuildID);

private:
	bool m_bInit;
	int m_nOccupationSystemState;
	LOCAL_TIME m_nStartOccupationSystem;

	CDNGameRoom * m_pGameRoom;
	CPvPGameMode * m_pGameMode;
	TBattleGourndModeInfo m_OccupationModeInfo;	
	bool m_bClimaxMode;

	int _GetBonusResource(int nGain, CPvPOccupationTeam * pTeam);

	std::list <CPvPOccupationPoint*> m_lOccupationPointList;
	std::list <CPvPOccupationTeam*> m_lOccupationTeamList;

	bool IsExistPoint(int nAreaID);
	CPvPOccupationPoint * _GetOccupationArea(EtVector3 * pPosition);	

	bool IsExistTeam(int nTeam);
	CPvPOccupationTeam * _GetOccupationTeam(UINT nTeamID);

	//Prepare & Check
	bool _PrepareOccupation();

	//Send
	void _SyncOccupationModeState();
	void _SyncOccupationState(CDNUserSession * pSession = NULL);
	void _SendClimaxMode();
	int m_nForceWinTem;			//Force set
};
