#pragma once

#include "Task.h"
#include "MessageListener.h"
#include "DnSkill.h"

#if defined(PRE_FIX_NEXTSKILLINFO)
#include "DnTableDB.h"
#endif // PRE_FIX_NEXTSKILLINFO

#define MAX_GUILD_WAR_SKILL_POINT 8

class CDnGuildWarTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnGuildWarTask>
{
public:
	CDnGuildWarTask();
	virtual ~CDnGuildWarTask();

public:
	bool Initialize();
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

public:
	struct stPreliminaryStatus	// Guild war preliminary status
	{
		int		nBluePoint;			// Blue team score	
		int		nRedPoint;			// Red team score	
		int		nMyPoint;			// My score
		int		nMyGuildPoint;		// Current guild points

		stPreliminaryStatus() : nBluePoint( 0 ), nRedPoint( 0 ), nMyPoint( 0 ), nMyGuildPoint( 0 ) {}
	};

	struct stPreliminaryResult		// Guild war preliminary result
	{
		int		nBlueFinalPoint;	// Blue team final score	
		int		nRedFinalPoint;		// Red team final score
		bool	bMyWin;				// My guild war result
		bool	bJoinGuildWar;		// Whether participating in guild war
		bool	bReward;			// Whether reward was claimed
		int		nFestivalPoint;		// Points earned in guild war result
		int		nFestivalPointAdd;	// Bonus points earned in guild war result

		stPreliminaryResult() : nBlueFinalPoint( 0 ), nRedFinalPoint( 0 ), bMyWin( false ), bJoinGuildWar( false ), bReward( false ), nFestivalPoint( 0 ), nFestivalPointAdd( 0 ) {}
	};

	struct stJoinGuildRankInfo			// Guild war preliminary result - guild ranking info
	{
		int		m_nGuildRank;			// Rank
		int		m_nGuildUserCount;		// Player count
		int		m_nGuildUserMax;		// Max guild members
		int		m_nTotalPoints;			// Total score
		tstring m_strGuildName;			// Guild name
		tstring m_strGuildMasterName;	// Guild master

		stJoinGuildRankInfo() : m_nGuildRank( 0 ), m_nGuildUserCount( 0 ), m_nGuildUserMax( 0 ), m_nTotalPoints( 0 ), m_strGuildName( L"" ), m_strGuildMasterName( L"" ) {}
	};

	struct stEventRankInfo			// Guild war preliminary result - category ranking info
	{
		int		m_nRank;			// Rank
		int		m_nTotalPoints;		// Total score
		tstring m_strName1;			// Name1
		tstring m_strName2;			// Name2

		stEventRankInfo() : m_nRank( 0 ), m_nTotalPoints( 0 ), m_strName1( L"" ), m_strName2( L"" ) {}
	};

	struct stTournamanetGuild
	{
		TGuildUID		m_tGuildUID;
		tstring			m_strGuildName;
		TGuildSelfView	m_tGuildSelfView;
		bool			m_bWin;
		char			m_cMatchTypeCode;

		stTournamanetGuild() : m_strGuildName( L"" ), m_bWin( false ), m_cMatchTypeCode( 0 ) {}
	};

	struct stGuildTournamentInfo
	{
		stTournamanetGuild	m_TournamentGuild[GUILDWAR_FINALS_TEAM_MAX];
		char				m_cMatchTypeCode;				// 1=Finals, 2=Semifinals, 3=Quarterfinals, 4=Round of 16
		__time64_t			m_tStartTime;					// If this is 0, it is currently in progress; if it has a value, it starts at that time.
		bool				m_bPopularityVote;				// Enable/disable popularity vote

		stGuildTournamentInfo() : m_cMatchTypeCode( 0 ), m_tStartTime( 0 ), m_bPopularityVote( false ) {}
	};

	struct SGuildWarEventTime
	{
		SGuildWarTime term;
		std::wstring timeString[GUILDWAR_EVENT_MAX];

		SGuildWarEventTime() { memset(&term, 0, sizeof(term)); }
	};

	struct stGuildWarShopItemInfo		// Record guild war shop purchase history
	{
		int			m_nItemID;
		int			m_nCurrentBuyCount;

		stGuildWarShopItemInfo() : m_nItemID( -1 ), m_nCurrentBuyCount( 0 ) {}
	};

public:
	stPreliminaryStatus& GetPreliminaryStatus() { return m_stPreliminaryStatus; }
	stPreliminaryResult& GetPreliminaryResult() { return m_stPreliminaryResult; }
	std::vector<stJoinGuildRankInfo>& GetPreliminaryResultGuildRanking() { return m_vecPreliminaryResultGuildRanking; }
	std::vector<stEventRankInfo>& GetEventGuildRankInfo() { return m_vecEventGuildRankInfo; }
	std::vector<stEventRankInfo>& GetEventPersonalRankInfo() { return m_vecEventPersonalRankInfo; }
	std::vector<tstring>& GetDailyBestChar() { return m_vecDailyBestChar; }
	stGuildTournamentInfo& GetGuildTournamentInfo() { return m_stGuildTournamentInfo; }
	void SetGuildWarState( int nGuildWarState ) { m_nGuildWarState = nGuildWarState; }
	int GetGuildWarState() { return m_nGuildWarState; }
	int GetGuildWarSkillUsedPoint() { return m_nGuildWarSkillUsedPoint; }
	int GetGuildWarSkillLeavePoint() { return m_nGuildWarSkillLeavePoint; }
	void UseGuildWarSkillPoint();
	INT64 GetGuildWarFestivalPoint() { return m_biGuildWarFestivalPoint; }
	eGuildWarStepType	GetCurrentGuildWarEventTimeType() const { return m_CurGuildWarStep; }
	eGuildFinalPart		GetCurrentGuildWarFinalRoundStep() const { return m_CurGuildWarFinalRoundStep; }
	void GetGuildWarEventTime(std::wstring& startTime, std::wstring& endTime, eGuildWarStepType type) const;
	void GetGuildWarFinalRoundTime(std::wstring& startTime, std::wstring& endTime, eGuildFinalPart step) const;
	void SetForceUpdateAlarm(bool bSet)	{ m_bSetForceUpdateAlarm = bSet; }
	bool GetGuildWarFinalProgress() { return m_bGuildWarFinalProgress; }

	void SendGuildWarVote( int nGuildIndex );
	int GetGuildWarShopItemCurrentBuyCount( int nItemID );
	void ResetGuildWarSkillPoint();

private:
	void OnRecvGuildWarTrialResultOpenMsg();
	void OnRecvGuildWarTournamentOpenMsg();
	void OnRecvGuildWarStatusOpenMsg( SCGuildWarStatus* pPacket );
	void OnRecvGuildWarTrialResultInfoMsg( SCGuildWarTrialResult* pPacket );
	void OnRecvGuildWarTournamentInfoMsg( SCGuildTournamentInfo* pPacket );
	void OnRecvGuildWarModeState( SCPvPOccupationModeState* pPacket );
	void OnRecvGuildWarFestivalPoint( SCGuildWarFestivalPoint* pPacket );
	void OnRecvGuildWarGetReward( SCGuildWarCompensation* pPacket );
	void OnRecvGuildWarVote( SCGuildWarVote* pPacket );
	void OnRecvGuildWarVoteTop( SCGuildWarVoteTop* pPacket );
	void OnRecvGuildWarEventTime( SCGuildWarEventTime* pPacket );
	void OnRecvGuildWarShopItemInfoList( SCBuyedItemCount* pPacket );
	void OnRecvGuildWarShopItemInfo( SCBuyedItemCount* pPacket );

protected:
	stPreliminaryStatus					m_stPreliminaryStatus;					// Guild war preliminary status data
	stPreliminaryResult					m_stPreliminaryResult;					// Guild war result data
	std::vector<stJoinGuildRankInfo>	m_vecPreliminaryResultGuildRanking;		// 0: My guild, 1~: Actual guild ranking
	std::vector<stEventRankInfo>		m_vecEventGuildRankInfo;				// Guild ranking by category
	std::vector<stEventRankInfo>		m_vecEventPersonalRankInfo;				// Individual ranking by category
	std::vector<tstring>				m_vecDailyBestChar;						// Daily high score award
	stGuildTournamentInfo				m_stGuildTournamentInfo;				// Guild war tournament data
	std::vector<stGuildWarShopItemInfo> m_vecGuildWarShopItemInfo;				// Record guild war shop purchase history
	int									m_nGuildWarState;						// Guild war status
	int									m_nGuildWarSkillUsedPoint;				// Used guild war skill points
	int									m_nGuildWarSkillLeavePoint;				// Remaining guild war skill points

	INT64								m_biGuildWarFestivalPoint;				// Guild festival points
	std::map<eGuildWarStepType, SGuildWarEventTime>	m_GuildWarEventTime;
	std::map<eGuildFinalPart, SGuildWarEventTime>	m_GuildWarFinalRoundTime;
	eGuildWarStepType								m_CurGuildWarStep;
	eGuildFinalPart									m_CurGuildWarFinalRoundStep;
	bool											m_bSetForceUpdateAlarm;
	bool											m_bGuildWarFinalProgress;

#if defined(PRE_FIX_NEXTSKILLINFO)
public:
	SKILL_LEVEL_INFO* GetSkillLevelTableIDList(int nSkillID, int nApplyType);

protected:
	static SKILL_LEVEL_INFO ms_SkillLevelTableInfo;
	SKILL_LEVEL_TABLEID_LIST m_SkillLevelTableIDList[2];

	void InitSkillLevelTableIDList();
	void AddSkillLevelTableID(int nSkillID, int nSkillLevel, int nSkillLevelTableID, int nApplyType);

#endif // PRE_FIX_NEXTSKILLINFO
};

#define GetGuildWarTask()		CDnGuildWarTask::GetInstance()



// CDnGuildWarSkill

class CDnGuildWarSkill : public CDnSkill
{
public:
	CDnGuildWarSkill( DnActorHandle hActor ) : CDnSkill( hActor ) {}
	~CDnGuildWarSkill() {}

public:
	static DnSkillHandle CreateSkill( DnActorHandle hActor, int iSkillTableID, int iLevel );
	bool Initialize( int iSkillTableID, int iLevel );
	bool LoadSkillLevelData( int iSkillTableID, int iLevel );
	virtual MIInventoryItem::ItemConditionEnum GetItemCondition();
	CDnSkill::UsingResult CanExecuteGuildWarSkill();
};
