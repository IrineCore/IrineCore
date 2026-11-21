#pragma once

#include "DNGuildWar.h"

//Move Nanbong's code here and modify... preserve Nanbong's code as much as possible
class CDNVillageConnection;

class CDNGuildWarManager
{
private:
	DWORD m_TickCheckGuildWar;	
	
	char m_cStepIndex;			// Current step	
	bool m_bWarEvent;			// Whether event is in progress
	short m_wScheduleID;		// Tier	
	short m_wWinersWeightRate;	// Winning team weight	
	__time64_t m_tRewardExpireDate;	// Reward (farm, gesture) expiration period 
	CDNGuildWar* m_pWarEventStep[GUILDWAR_STEP_END];

	BYTE m_cSecretTeam;			// Whether secret is enabled
	int m_nSecretRandomSeed;	// Secret random seed	

	int m_nFinalTeamCount;		// Finals-advancing team count
	bool m_bFinalTeamSetting;	// Whether finals-advancing teams are set

	TGuildUID m_PreWinGuildUID;	// Previous round winning guild UID
	DWORD m_dwPreWinSkillCoolTime; // Previous round winning skill cooldown

	// Various data stored in DB..
	////////////////////////////////////////////////////////////////////////////////////////
	// Preliminaries
	LONG m_nBlueTeamPoint;		// Blue team points
	LONG m_nRedTeamPoint;		// Red team points
	// Group per round
	sTournamentGroup m_bTournamentGroup[GUILDWAR_TOURNAMENT_GROUP_MAX];
	// Full schedule
	TGuildWarEventInfo m_sGuildWarSchedule[GUILDWAR_STEP_END];
	// Finals schedule
	TGuildWarEventInfo m_sGuildWarFinalSchedule[GUILDWAR_FINALPART_MAX];
	// Finals bracket
	SGuildTournamentInfo m_GuildWarFinalInfo[GUILDWAR_FINALS_TEAM_MAX]; // Sort guilds by bracket order
	int	m_GuildWarOpeningPoints[GUILDWAR_FINALS_TEAM_MAX]; // Total preliminary points for each bracket entry
	////////////////////////////////////////////////////////////////////////////////////////
	// Status of top 24 in preliminaries
	SGuildWarRankingInfo m_sGuildWarPointTrialRanking[GUILDWAR_TRIAL_POINT_TEAM_MAX];
	short m_wPreWinScheduleID;		// Previous winning round..
	bool m_bFrinalProgress;			// Whether finals are in progress	
	bool m_bFinalStart;				// Whether finals have started..

	// Whether set
	eGuildWarSettingStep m_eSettingStep;	// Info setup phase at start.
	bool m_bSendGuildWarInfo;			// Whether to request info from village
	//DWORD m_dwSendVillageTick; // Tick when info was requested from Village

	bool m_bCheatFlag;
	bool m_bFinalWinGuild;				// Whether the final winner is set (gesture, farm reward check)

	bool m_bResetSchedule;				// Initialize schedules and reload all.
#if defined(PRE_FIX_75807)
	bool m_bFarmForceHarbest;			// Reset winner farm
#endif //#if defined(PRE_FIX_75807)

private:
	int  FindWorldEventIndex();

	bool CheckStep(char cType);

	bool RegisterScheduleGroup(short wScheduleID, const TGuildWarEventInfo vEventInfo[]);
	bool UpdateScheduleGroup(short wScheduleID, const TGuildWarEventInfo vEventInfo[]);

	void ResetStep();
	void StartStep();
	void NextStep();
	void FinalStep();

	void Clear();

public:
	CDNGuildWarManager();
	virtual ~CDNGuildWarManager();
	
	inline void SetSendGuildWarInfo(bool bSendGuildWarInfo) { m_bSendGuildWarInfo = bSendGuildWarInfo; };
	inline void SetGuildWarSettingStep(eGuildWarSettingStep eSettingStep) { m_eSettingStep = eSettingStep; };
	inline eGuildWarSettingStep GetGuildWarSettingStep() { return m_eSettingStep;};
	inline short GetScheduleID() { return m_wScheduleID;};
	inline int GetBlueTeamPoint() { return (int)m_nBlueTeamPoint; };
	inline int GetRedTeamPoint() { return (int)m_nRedTeamPoint; };
	inline TGuildWarEventInfo* GetGuildWarSchedule() { return m_sGuildWarSchedule;};
	inline TGuildWarEventInfo* GetGuildWarFinalSchedule() { return m_sGuildWarFinalSchedule;};
	inline short GetWinersWeightRate() { return m_wWinersWeightRate; };

	int SetGuildTournamentInfo(SGuildTournamentInfo* pGuildTournamentInfo);
	inline SGuildTournamentInfo* GetGuildTournamentInfo() { return m_GuildWarFinalInfo;};

	void SetGuildWarPointTrialRanking(const MASetGuildWarPointRunningTotal* pData);
	inline SGuildWarRankingInfo* GetGuildWarPointTrialRanking() { return m_sGuildWarPointTrialRanking;};

	void SetGuildTournamentInfoWin(char cIndex, bool bWin);
	void SetGuildWarTournamentResult(GAMAPvPGuildWarResult* pGuildWarResult);
	void SendSetGuildWarFinalResult(char cWinGuildIndex=-1, char cLoseGuildIndex=-1 );
	void SetGuildWarOpenningPoint(VIMASetGuildWarFinalTeam* pData );
	void SetGuildWarTournamentPoint(GAMAPvPGuildWarScore* pGuildWarPoint);
	void SendSetGuildWarTournamentWin(char cWinGuildIndex, char cFinalPart);

	bool LoadScheduleInfo(const VIMASetGuildWarSchedule* pData);	
	bool LoadFinalScheduleInfo(MASetGuildWarFinalSchedule* pData);
	void DoUpdate(DWORD CurTick);	
	
	CDNGuildWar* GetWarEvent(char cStep);
	inline char GetStepIndex() { return m_cStepIndex;};
	bool GetRestriction(int nSubCmd);

	void SendGuildWarInfoReq(); //Request various info from village.	
	void SetGuildWarPoint(int nBlueTeam, int nRedTeam);
	void AddGuildWarPoint(char cTeamType, int nAddPoint);
	void SetGuildWarPreWinGuild(TGuildUID GuildUID);

	inline void SetPreWinGuild(TGuildUID GuildUID) { m_PreWinGuildUID = GuildUID;};
	inline TGuildUID GetPreWinGuild() { return m_PreWinGuildUID; };	// Previous round winning guild UID
	// Winner skill cooldown
	inline DWORD GetPreWinSKillCoolTime() { return m_dwPreWinSkillCoolTime; };
	inline void SetPreWinSKillCoolTime(DWORD dwCoolTime ) { m_dwPreWinSkillCoolTime = dwCoolTime;};		
	UINT GetGuildDBIDWithFinal(const UINT uiPvPIndex, const WCHAR * pGuildName);

	bool IsTrialStats();
	void SetTrialStats();
	// Generate bracket..
	void CalcGuildWarTournament(VIMASetGuildWarFinalTeam* pData);
	// Guild war room setup per round
	void SetGuildWarTournamentGroup();
	// Create Guild War room
	void GuildWarCreateRoom();
	// Close Guild War room
	void GuildWarEndRoom();

	char GetCurFinalPart();
	time_t GetCurFinalPartBeginTime();
	inline int GetFinalTeamCount() { return m_nFinalTeamCount; };
	inline bool IsFinalTeamSetting() { return m_bFinalTeamSetting; };

	void CalcTeamSecret();
	inline BYTE GetSecretTeam() {return m_cSecretTeam;}
	inline int GetSecretRandomSeed() {return m_nSecretRandomSeed;}	

	bool GetCheatFlag () {return m_bCheatFlag;}
	void SetCheatFlag (bool bFlag) {m_bCheatFlag = bFlag;}

	// Call when the room is destroyed or the game server crashes.
	void SetDropTournament(UINT  unPvPIndex);
	void CalcGuildWarTournamentWin(BYTE cGroupIndex);

	void SetDBJobSend(bool bSend);
	void SetDBJobSeq(int nJobSeq);
	void SetDBJobSuccess(bool bSuccess);
	bool GetDBJobSuccess();
	void CalcGuildWarTournamentResult();

	inline void SetFinalWinGuild(bool bFinalWin) { m_bFinalWinGuild = bFinalWin;};
	inline void SetResetSchedule(bool bReset) { m_bResetSchedule = bReset;};
	inline void SetFinalStart(bool bStart) { m_bFinalStart = bStart;};
	inline bool GetFinalStart() { return m_bFinalStart;};
	inline bool GetFinalWinGuildReward() { return m_bFinalWinGuild;};
	inline void SetPreWinScheduleID(short wScheduleID) { m_wPreWinScheduleID = wScheduleID;};
	inline short GetPreWinScheduleID() { return m_wPreWinScheduleID; };
	inline void SetFinalProgress(bool bFinalProgress) { m_bFrinalProgress = bFinalProgress;};
	inline bool GetFinalProgress() { return m_bFrinalProgress;};
};

extern CDNGuildWarManager * g_pGuildWarManager;
