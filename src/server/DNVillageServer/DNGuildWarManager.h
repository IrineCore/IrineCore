#pragma once

class CDNGuildWarManager
{
private:
	char m_cStepIndex;				// Current guild war step in progress
	int m_nBlueTeamPoint;			// Blue team score
	int m_nRedTeamPoint;			// Red team score
	short m_wScheduleID;			// Schedule ID
	short m_wWinersWeightRate;		// Winning team weight

	bool m_bGuildWarStats;			// Whether Guild War preliminary result loading is complete	

	// Main tournament schedule for the current round in progress.	
	char m_cGuildWarFinalPart;
	__time64_t m_tGuildWarFinalPartBeginTime;

	TGuildUID m_PreWinGuildUID;	// Previous round winning guild UID
	DWORD m_dwPreWinSkillCoolTime;	// Winner skill cooldown

	// Full schedule
	TGuildWarEventInfo m_sGuildWarSchedule[GUILDWAR_STEP_END-1];
	// Finals schedule
	TGuildWarEventInfo m_sGuildWarFinalSchedule[GUILDWAR_FINALPART_MAX-1];

	// Static ranking info
	// Finals-qualifying guild
	SGuildWarRankingInfo	m_sGuildWarRankingInfo[GUILDWAR_FINALS_TEAM_MAX];	
	int						m_nGuildWarTrialRankingCount;
	SGuildWarRankingInfo	m_sGuildWarTrialRankingInfo[GUILDWAR_TRIAL_POINT_TEAM_MAX];
	bool					m_bFinalProgress;

	// Guild rank
	SGuildWarMissionGuildRankingInfo m_sGuildWarMissionGuildRankingInfo[GUILDWAR_RANKINGTYPE_MAX];	// Overall guild ranking
	
	// Individual ranking
	INT64				m_nGuildWarMissionRankingCharacterDBID[GUILDWAR_RANKINGTYPE_MAX];	// CharacterDBID of player in individual ranking (for score calculation)
	INT64				m_nGuildWarDailyAwardCharacterDBID[GUILDWAR_DAILY_AWARD_MAX];		// CharacterDBID of the daily reward recipient (for score calculation)

	SGuildWarMissionRankingInfo m_sGuildWarMissionRankingInfo[GUILDWAR_RANKINGTYPE_MAX];		// Individual ranking	

	// Daily reward
	SGuildWarDailyAward			m_sGuildWarDailyAward[GUILDWAR_DAILY_AWARD_MAX];	

	// Finals bracket
	SGuildTournamentInfo		m_sGuildWarFinalInfo[GUILDWAR_FINALS_TEAM_MAX]; // Sort guilds by bracket order

	// Popularity vote 1st place info
	bool m_bGetGuildWarVote;					// Whether info was retrieved
	WCHAR m_wszVoteGuildName[GUILDNAME_MAX];	// Guild name
	int m_nVoteCount;						// Total vote count

	BYTE m_cSecretTeam;
	int m_nSecretRandomSeed;

	int m_vSecretMissionID[GUILDWARMISSION_MAXGROUP];

public:
	CDNGuildWarManager();
	virtual ~CDNGuildWarManager();

	void Reset();
	inline short GetScheduleID() { return m_wScheduleID; };
	inline char GetStepIndex() { return m_cStepIndex; };
	void SetStepIndex (char cStepIndex) {m_cStepIndex = cStepIndex;}
	bool GetRestriction(int nSubCmd, TGuildUID GuildUID);
	void SetGuildWarStep(char cStepIndex, short wScheduleID, short wWinersWeightRate );
	inline bool GetGuildWarStats() { return m_bGuildWarStats;};

	inline void SetTeamPoint(int nBlueTeamPoint, int nRedTeamPoint) { m_nBlueTeamPoint=nBlueTeamPoint; m_nRedTeamPoint=nRedTeamPoint; CalcTeamSecret();};
	inline int GetBlueTeamPoint() { return m_nBlueTeamPoint; };
	inline int GetRedTeamPoint() { return m_nRedTeamPoint; };
	inline void AddBlueTeamPoint(int nAddPoint) { m_nBlueTeamPoint += nAddPoint; CalcTeamSecret();};
	inline void AddRedTeamPoint(int nAddPoint) { m_nRedTeamPoint += nAddPoint; CalcTeamSecret();};

	inline void SetGuildWarVoteCount(int nVoteCount) { m_nVoteCount=nVoteCount;};
	inline int GetGuildWarVoteCount() { return m_nVoteCount; };
	inline void SetGuildWarVote(bool bVote) { m_bGetGuildWarVote = bVote;};
	inline bool GetGuildWarVote() { return m_bGetGuildWarVote;};
	inline WCHAR* GetGuildWarVoteGuildName() { return m_wszVoteGuildName; };
	inline void SetGuildWarVoteGuildName(WCHAR* wszGuildName) { memcpy(m_wszVoteGuildName, wszGuildName, sizeof(m_wszVoteGuildName)); };

	bool IsGuildWarFinal(TGuildUID nGuildUID);	
	
	bool IsGuildWarTrialWIn(char cTeamType);

	int CalcGuildWarFestivalPoint(char cTeamType, INT64 nCharacterDBID, int nGuildWarPoint);
	int GetGuildAwardCount(INT64 nCharacterDBID);

	// Guild schedule
	void SetGuildWarSechdule(MASetGuildWarEventTime* SetGuildWarEventTime);
	inline TGuildWarEventInfo* GetGuildWarSchedule() { return m_sGuildWarSchedule;};
	inline TGuildWarEventInfo* GetGuildWarFinalSchedule() { return m_sGuildWarFinalSchedule;};

	// Previous round winning guild
	inline TGuildUID GetPreWinGuildUID() { return m_PreWinGuildUID; };
	inline void SetPreWinGuildUID(TGuildUID GuildUID) { m_PreWinGuildUID = GuildUID;};
	inline void ResetPreWinGuildUID() { m_PreWinGuildUID.Reset(); };

	// Winner skill cooldown
	inline DWORD GetPreWinSKillCoolTime() { return m_dwPreWinSkillCoolTime; };
	inline void SetPreWinSKillCoolTime(DWORD dwCoolTime ) { m_dwPreWinSkillCoolTime = dwCoolTime;};	
	
	// Finals-qualifying guild
	void SetGuildWarRankingInfo(TAGetGuildWarPointGuildTotal* pGetGuildWarPointGuildTotal );
	inline SGuildWarRankingInfo* GetGuildWarRankingInfo() { return m_sGuildWarRankingInfo; };
	int GetGuildWarPoint(UINT nGuildDBID);

	void SetGuildWarTrialRankingInfo(MASetGuildWarPointRunningTotal* pSetGuildWarPointRunningTotal );
	inline SGuildWarRankingInfo* GetGuildWarTrialRankingInfo() { return m_sGuildWarTrialRankingInfo; };	
	inline int GetGuildWarTrialRankingCount() { return m_nGuildWarTrialRankingCount;};
	inline bool GetFinalProgress() { return m_bFinalProgress; };

	// Set guild ranking by category
	void SetGuildWarMissionGuildRankingInfo(TAGetGuildWarPointGuildPartTotal* pGetGuildWarPointGuildPartTotal);
	inline SGuildWarMissionGuildRankingInfo* GetGuildWarMissionGuildRankingInfo() { return m_sGuildWarMissionGuildRankingInfo; };

	// Set individual ranking by category
	void SetGuildWarMissionRankingInfo(TAGetGuildWarPointPartTotal* pGetGuildWarPointPartTotal);
	inline SGuildWarMissionRankingInfo* GetGuildWarMissionRankingInfo() { return m_sGuildWarMissionRankingInfo; };

	// Awards by date
	void SetGuildDailyAward(TAGetGuildWarPointDaily* pGetGuildWarPointDaily);
	inline SGuildWarDailyAward* GetGuildDailyAward() { return m_sGuildWarDailyAward; };	

	// Tournament bracket	
	void SetGuildTournamentInfo(SGuildTournamentInfo* pGuildTournamentInfo);
	inline SGuildTournamentInfo* GetGuildTournamentInfo() { return m_sGuildWarFinalInfo;};
	// Finals result
	void SetGuildTournamentResult(MAVISetGuildWarFinalResult* pGuildTournamentResult);

	// Finals schedule	
	void SetGuildWarFinalPart(char cGuildFinalPart, __time64_t tFinalPartBeginTime);
	inline char GetGuildWarFinalPart() { return m_cGuildWarFinalPart;};
	inline __time64_t GetGuildWarFinalPartBeginTime() { return m_tGuildWarFinalPartBeginTime; };	

	// Secret info
	bool IsSecretMission(BYTE cTeamCode, int nMissionID);
	void SetGuildSecretMission(MASetGuildWarSecretMission* pSecret);
	void CalcTeamSecret();

	// Guild mark
	void OnRecvMaGuildChangeMark( MAGuildChangeMark* pPacket );

	// Change guild name
	void OnRecvMaGuildChangeName( MAGuildChangeName* pPacket );	
};

extern CDNGuildWarManager * g_pGuildWarManager;
