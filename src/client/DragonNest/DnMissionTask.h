#pragma once

#include "Task.h"
#include "MessageListener.h"

class CDnMissionTask : public CTask, public CTaskListener, public CSingleton<CDnMissionTask> {
public:
	CDnMissionTask();
	virtual ~CDnMissionTask();

#ifdef PRE_ADD_ACTIVEMISSION
	#define ACTIVEMISSIONID -1111 // Identifier for removing an active mission after forcibly adding it to the achieved mission list.
#endif // PRE_ADD_ACTIVEMISSION

	enum DailyMissionTypeEnum {
		Daily,
		Weekly,
		GuildWar,
		PCBang,
		GuildCommon,
		WeekendEvent,
		WeekendRepeat,
#ifdef PRE_ADD_MONTHLY_MISSION
		MonthlyEvent,
#endif 
		LongTerm,
		DailyMissionTypeEnum_Amount,
	};
	enum MainCategoryEnum {
		Normal,
		Dungeon,
		Battle,
#ifdef PRE_ADD_MISSION_NEST_TAB
		Nest,
#endif 
		Special,
		MainCategoryEnum_Amount
	};

	enum RewardTypeEnum {
		Item,
		Appellation,
	};

	enum
	{
		RewardItem_Amount = 3,
	};

	struct MissionInfoStruct {
		int nArrayIndex;
		int nNotifierID;
		MainCategoryEnum MainCategory;
		std::string szSubCategory;
		tstring szParentSubCategory;
		tstring szTitle;
		tstring szSubTitle;
		int nIcon;			// Small icon displayed in mission list
		int nAchieveIcon;	// Icon displayed as an alarm upon achievement
		int nRewardItemID[RewardItem_Amount];
		int nRewardCashItemSN[RewardItem_Amount];		// SN in the cashcommodity table
		int nRewardItemCount[RewardItem_Amount];		// Reward item count
		int nRewardCashItemCount[RewardItem_Amount];	// Cash reward item count
		int nRewardCoin;
		int nRewardAppellationID;
		int nRewardPoint;
		int nRewardCounter;
		int nCounter;
		int nAchieveSystemMsgID;
		int nType;

		bool bAchieve;

		MissionInfoStruct();
		void Clone( MissionInfoStruct * p );
	};

protected:
	int m_nMissionScore[MainCategoryEnum_Amount];
	int m_nQuestScore;
	std::vector<MissionInfoStruct *> m_pVecMissionList;
	std::map<int, MissionInfoStruct *> m_nMapSearch;
	short m_nLastUpdateAchieveList[LASTMISSIONACHIEVEMAX];

#ifdef PRE_ADD_STAMPSYSTEM
	std::map<int, MissionInfoStruct *> m_mapMission; // <int(mission id),MissionInfoStruct *>
#endif // PRE_ADD_STAMPSYSTEM

	std::vector<MissionInfoStruct *> m_pVecDailyMissionList[DailyMissionTypeEnum_Amount];
	std::map<int, MissionInfoStruct *> m_nMapDailySearch[DailyMissionTypeEnum_Amount];

	struct AlarmQueueStruct {
		float fDelta;
		MissionInfoStruct *pInfo;

		AlarmQueueStruct( float delta, MissionInfoStruct *pstruct ) { fDelta = delta; pInfo = pstruct; }
	};

	std::vector<AlarmQueueStruct> m_VecGainAlarmQueueList;
	std::vector<AlarmQueueStruct> m_VecAchieveAlarmQueueList;
	DnEtcHandle m_hAchieveAlarmEffect;

	DNNotifier::Data	m_NotifierData[DNNotifier::RegisterCount::TotalMission];

#ifdef PRE_MOD_MISSION_HELPER
	std::vector<MissionInfoStruct *> m_pVecDungeonExpectMissionList;
#endif

#if defined( PRE_ADD_CHAT_MISSION )
	std::vector< std::pair<int, std::wstring> > m_vChatMission;
#endif	// #if defined( PRE_ADD_CHAT_MISSION )

protected:
	virtual void OnRecvMissionMessage( int nSubCmd, char *pData, int nSize );

	MissionInfoStruct *AddMission( int nArrayIndex );
	MissionInfoStruct *AddDailyMission( DailyMissionTypeEnum Type, int nArrayIndex, int nItemID );

	void UpdateLastAchieveList( int nArrayIndex );

public:
	bool Initialize();
	void Finalize();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnDisconnectTcp( bool bValidDisconnect );
	virtual void OnDisconnectUdp();

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	
	virtual void OnRecvMissionList( SCMissionList *pPacket );
	virtual void OnRecvMissionGain( SCMissionGain *pPacket );
	virtual void OnRecvMissionAchieve( SCMissionAchieve *pPacket );

	virtual void OnRecvDailyMissionList( SCDailyMissionList *pPacket );
	virtual void OnRecvDailyMissionCount( SCDailyMissionCount *pPacket );
	virtual void OnRecvDailyMissionAchieve( SCDailyMissionAchieve *pPacket );
	virtual void OnRecvDailyMissionAlarm( SCDailyMissionAlarm *pPacket );

	virtual void OnRecvEventPopUp( SCMissionEventPopUp* pPacket );
	virtual void OnRecvHelpAlarm( SCMissionHelpAlarm *pPacket );

	void OnGainMission( MissionInfoStruct *pStruct );
	void OnAchieveMission( MissionInfoStruct *pStruct );

	void OnCounterDailyMission( MissionInfoStruct *pStruct );

	void RefreshNotifier();
	void OnAchieveDailyMission( MissionInfoStruct *pStruct );

#ifdef PRE_ADD_ACTIVEMISSION
	void OnGainActiveMission( char * pData );
	void OnAchieveActiveMission( char * pData );
#endif // PRE_ADD_ACTIVEMISSION
	// Mission
	DWORD GetMissionCount();
	MissionInfoStruct *GetMissionInfo( int nIndex );

#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL
	MissionInfoStruct *GetAllMission( int nIndex );
	DWORD GetAchieveMissionCount();
#endif
	MissionInfoStruct *GetMissionFromArrayIndex( int nArrayIndex );
	int GetLastUpdateAchieveIndex( int nIndex );
	
	// Daily Quest
	DWORD GetDailyMissionCount( DailyMissionTypeEnum Type );
	MissionInfoStruct *GetDailyMissionInfo( DailyMissionTypeEnum Type, int nIndex );
	int GetQuestScore() { return m_nQuestScore; }
#ifdef PRE_MOD_MISSION_HELPER
	bool RegisterNotifier( DNNotifier::Type::eType NotifierType, int nIndex, bool bInit = false );
	bool RemoveNotifier( DNNotifier::Type::eType NotifierType, int nIndex );
	bool IsRegisterNotifier( DNNotifier::Type::eType NotifierType, int nIndex );
	CDnMissionTask::MissionInfoStruct * GetMissionInfoForNotifier( int nIndex );

	DNNotifier::Type::eType ToNotifierType( DailyMissionTypeEnum DailyMissionType );
	DailyMissionTypeEnum ToDailyMissionType( DNNotifier::Type::eType Type );
	DNNotifier::Type::eType GetNotifierType( MissionInfoStruct *pStruct );

	bool IsMissionNotifierFull();

	void AddDungeonExpectMission( MissionInfoStruct *pStruct );
	void ClearDungeonExpectMission();
	void AutoRegisterMissionNotifier();

#ifdef PRE_ADD_STAMPSYSTEM

	bool GetMissionInfoByID( MissionInfoStruct *& rMission, int itemID );
	// When all missions are registered -
	// : Add immediately if there is an empty slot; if full, shift registered missions up by one and add the new mission in the last slot.
	bool PushBack( DNNotifier::Type::eType NotifierType, int nIndex, bool bInit = false );

	//MissionInfoStruct * GetMissionByID( int itemID );
	
#endif // PRE_ADD_STAMPSYSTEM

#else
	void RegisterNotifier( DailyMissionTypeEnum Type, int nIndex, bool bInit=false );
	void RemoveNotifier( int nIndex );
	bool IsRegisterNotifier( DailyMissionTypeEnum Type, int nIndex );
	CDnMissionTask::MissionInfoStruct * GetMissionInfoForNotifier( DailyMissionTypeEnum Type );
#endif

#if defined( PRE_ADD_CHAT_MISSION )
	void InitChatMission();
	void RequestChatMission( std::wstring & wszChat );
#endif	// #if defined( PRE_ADD_CHAT_MISSION )
};

#define GetMissionTask()		CDnMissionTask::GetInstance()
