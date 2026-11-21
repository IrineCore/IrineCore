#pragma once
#include "DNEventSystem.h"
/*---------------------------------------------------------------------------------------
									CDNUserQuest

- Class responsible for user character quests
- Same as Item, since this class is shared by VillageServer and GameServer
Heavily plastered with if defined (?)
---------------------------------------------------------------------------------------*/


class CDNUserSession;
class CDNUserSendManager;
class CDNQuest;

class CDNUserQuest:public TBoostMemoryPool<CDNUserQuest>
{

private:
	CDNUserSession*	m_pSession;

	TQuestGroup* m_pQuest;

	TQuestReward m_QuestReward;
	int	m_RewardQuestID;
	int	m_RewardCheck;		// Check if quest reward payout order matches the user-specified number; target is a specific quest memo index (QUESTMEMOREWARDCHECK)

	CSSelectQuestReward m_PacketReward;
	std::set<int> m_CompletedPeriodQuests;

#if defined(PRE_ADD_REMOTE_QUEST)
	std::vector<int> m_nVecRemoteQuestGainList[EventSystem::EventTypeEnum_Amount];
	std::vector<int> m_nVecRemoteQuestAcceptWaitList;	//Try passing quest ID for now
#endif

public:
	CDNUserQuest(CDNUserSession *pSession);
	virtual ~CDNUserQuest();

	UINT GetUserObjID();

	bool IsClearQuest(int nQuestID);
	void SetCompleteQuestFlag(int nQuestID, bool bFlag);

	int	AddHuntingQuest(int nQuestID, int nQuestStep, int nJournalStep, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt);

	int	AddQuest(int nQuestID, bool bSendErrorMsg = false);
	void OnCancelQuest(int nQuestID);
	int	CompleteQuest(int nQuestID, bool bDelPlayList, bool bRepeat);
	int	MarkingCompleteQuest(int nQuestID);
	int	ForceCompleteQuest(int nQuestID, int nQuestCode, int bDoMark, int bDoDelete, int bDoRepeat);

	int	RewardAfterCompletingQuest(int nQuestID, bool bDelPlayList);
	void ResetRewardFlag();

	char* GetQuestCompleteBinary() { return(m_pQuest->CompleteQuest); }

	int	GetPlayingQuestCount();

	short HasQuest(int nQuestID);

	int	SetQuestStepAndJournalStep(int nQuestID, short nQuestStep, int nJournalStep, bool bFromScript);

	int	SetQuestStep(int nQuestID, short nQuestStep);
	short GetQuestStep(int nQuestID);
	int	SetJournalStep(int nQuestID, int nJournalStep);
	int	GetJournalStep(int nQuestID);

	int	SetQuestMemo(int nQuestID, char nMemoIndex, TP_QUESTMEMO nVal);
	TP_QUESTMEMO GetQuestMemo(int nQuestID, char nMemoIndex);
	int	IncQuestMemo(int nQuestID, char nMemoIndex);
	int	DecQuestMemo(int nQuestID, char nMemoIndex);

	int	SetCountingInfo(int nQuestID, int nCountingSlot, int nCountingType, int nCountingIndex, int nTargetCnt);
	int	ClearCountingInfo(int nQuestID);

	int	IsAllCompleteCounting(int nQuestID);

#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
	void OnCounting(int nCountingType, int nCountItemID, int nItemCount=0);
#else
	void OnCounting(int nCountingType, int nCountItemID);
#endif//#if defined(PRE_FIX_QUEST_ITEM_ONCOUNT)
	void OnKillMonster(int nMonsterID);
	void OnDieMonster(int nMonsterID);
	void OnAddItem(int nItemID, short wCount);
	void OnAddSymbolItem(int nItemID);
	void OnAddQuestItem(int nItemID);
	void OnStageClear(int nMapIndex);
	void OnAddItemEx(int nItemID, short wCount);

	// Update quest info whose state changed to the client.
	int	SendQuestRefresh(int nRefreshType, int nQuestID);

	// Refresh quest notifier
	void OnSetActiveQuest(int nQuestID);

	// Set the quest reward.
	void SetReward(TQuestReward& Reward, int nQuestID, int nRewardCheck);

	// Temporarily store the reward packet.
	void SavePacketReward(CSSelectQuestReward* pPacket);

	// Extract reward data from the table and store temporarily.
	bool PreparePacketReward(int nQuestID);

	bool IsRewardItem();
	bool IsRewardCashItem();

	void OnSelectRewardEx(bool bSelectedArray[], bool bSelectedCashArray[]);
	void OnPeriodEvent(int nQuestID, bool bFlag);

	// For CheatCommand
	void SendAllQuestRefresh();
	void ClearAllQuestInfo();
	void DumpQuest(std::vector<TQuest>& output);
	int	FindPlayingQuest(int nQuestID);

	void AddCompletedPeriodQuest(int questID);
	void RemoveCompletedPeriodQuest(int questID);
	bool IsCompletedPeriodQuest(int questID) const;	

#if defined(PRE_ADD_REMOTE_QUEST)

	void RefreshRemoteQuest(int nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList);
	void GetAcceptWaitRemoteQuest(int *nAcceptWaitRemoteQuestCount, int *AcceptWaitRemoteQuestList);
	
	bool OnEvent( EventSystem::EventTypeEnum Event );

	void CheckRemoteQuestGain(EventSystem::EventTypeEnum Event);
	bool CheckRemotePreQuestCondition( TRemoteQuestData *pData );
	void CheckAcceptWaitRemoteQuestList();

	bool RemoveRemoteQuestFromAcceptWaitList(int nQuestID);
	bool AddRemoteQuestToAcceptWaitList(int nQuestID);
	void AcceptRemoteQuest(int nQuestID);
	void CancelRemoteQuest(int nQuestID);
	void CompleteRemoteQuest(int nQuestID, CSSelectQuestReward* pPacket);

	int FindRecompenseQuestIndex(int nQuestID);
	bool SetQuestToRecompenseState(int nQuestID);

#endif	//#if defined(PRE_ADD_REMOTE_QUEST)


private:
	int	_RemoveQuest(int nQuestID, bool bComplete, bool bRepeat);

	// Check if quest is already in progress. Return its index if found, or -1.
	int	_FindPlayingQuest(int nQuestID);
	// Find an empty play quest slot. Returns -1 if none
	int	_FindEmptyPlayQuestIndex();

	// Find the highest quest step among active sub-quests. return QuestID; 
	TQuest _FindHighestQStepSubQuest();
#if defined(PRE_ADD_REMOTE_QUEST)
	void _ResetRemoteQuest();
	void _RefreshRemoteQuestList();
	// Check if it is a completed remote quest awaiting reward. Return its index if found, else -1.	
	int _FindRecompenseQuestIndex(int nQuestID);
	// Check if it is a remote quest awaiting acceptance. Return its index if found, else -1.	
	int _FindAcceptWaitQuest(int nQuestID);
	// Verify whether it is an acquirable remote quest
	bool _CheckRemoteQuestGainValidation(int nQuestID, CDNQuest* pQuest);
#endif

	int _AddItemByQuestReward( const TQuestReward::_ITEMSET* pItemSet );
};