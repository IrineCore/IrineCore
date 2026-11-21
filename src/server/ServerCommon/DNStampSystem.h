#pragma once

#if defined( PRE_ADD_STAMPSYSTEM )
class CDNStampSystem : public TBoostMemoryPool<CDNStampSystem>
{
public:
	CDNStampSystem( CDNUserSession * pUserSession );
	~CDNStampSystem();

	void LoadUserData();
	void InitCompleteSlot( const StampSystem::StampCompleteInfo * pCompleteInfo );
	void ResetDailyData( time_t tCurrentTime );
	void ResetWeeklyData( time_t tCurrentTime );
	
	void OnCompleteQuest( int nQuestID );
	void OnCompleteDailyMission( int nArrayIndex );

	void SendInitData();

	// for cheat command
	void ForceAddCompleteSlot( int nChallengeIndex, int nWeekDay );
	void ForceClearCompleteSlot();

private:
	INT64 _ConvertToStampTableTime( time_t tTime );
	bool _CheckQuestAllComplete( const std::set<int> & rSetAssignmentID ) const;
	bool _CheckMissionComplete( const std::set<int> & rSetAssignmentID, int nArrayIndex ) const;
	bool _CheckAndInsertSlot( int nChallengeIndex, int nWeekDay );
	void _CheckAndGiveReward( int nChallengeIndex, const StampSystem::TStampChallenge & rChallengeData ) const;
	void _ClearCompleteSlot();
	void _UpdateWeekDay( time_t tTime );

private:
	typedef					std::set<int>
							SlotSet;

	CDNUserSession *		m_pUserSession;
	std::map<int,SlotSet>	m_MapCompleteSlotSet;

	// Items that can be factored out as common..
	const StampSystem::TStampTableData * m_pStampTableData;
	int						m_nCheckDailyTime;
	int						m_nWeekDay;
};

#endif // #if defined( PRE_ADD_STAMPSYSTEM )
