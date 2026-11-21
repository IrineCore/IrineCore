#pragma once

#ifdef PRE_ADD_DONATION

// Donation system Task.

#include "Task.h"
#include "MessageListener.h"

// Donation ranker Data.
struct SContributionRanker
{
	int nRank;
	int nJob;
	std::wstring strGuildName;
	std::wstring strID;
	INT64 nGold;

	SContributionRanker(){}
	SContributionRanker( int rank, int job, wchar_t *guidName, wchar_t *id, INT64 gold )
		: nRank(rank), nJob(job), strGuildName(guidName), strID(id), nGold(gold)
	{}	
};


// Donation reward item.
struct SContributionRewardItems
{
	UINT nType : 2; // Monthly/daily distinction (currently 2 types).
	UINT nRank : 4; // Rank.

	// 5 reward items.
	int nItemID[5];
	WORD nItemCnt[5];	

	SContributionRewardItems()
		: nType(0), nRank(0)
	{
		memset( nItemID, 0, sizeof(int) * 5 );
		memset( nItemCnt, 0, sizeof( WORD ) * 5 );
	}	
	//SContributionRewardItems( SContributionRewardItems & value ){
	//	nType = value.nType; nRank = value.nRank;
	//	memcpy( nItemID, value.nItemID, sizeof(int) );
	//	memcpy( nItemCnt, value.nItemCnt, sizeof(WORD) );
	//}
};

class CDnContributionTask : public CTask, public CTaskListener, public CSingleton<CDnContributionTask>
{

private:

	// Donation ranker Data List.
	std::vector< SContributionRanker * > m_vListRanker;

	// LocalPlayer.
	int m_nLPRank;
	INT64 m_nLPGold;
	INT64 m_nContributionGold; // Requested donation amount - added to LPGold on donation success.


	// Donation reward - ordered by rank.
	std::vector< SContributionRewardItems > m_vRewardMouth; // Monthly.
	std::vector< SContributionRewardItems > m_vRewardDay;   // Daily.


public:

	CDnContributionTask();
	~CDnContributionTask(){
		Finalize();
	}

	bool Initialize();
	void Finalize();


	// Override - CTask ------------//
	//virtual void Process( LOCAL_TIME LocalTime, float fDelta ) {}
	

	// Override - CTaskListener ----//
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );


private:

	void OnRecvRankListMessage( char * pData ); // Ranking list refresh result packet.
	
	// Return LocalPlayer index in ranking list.
	int GetRListToLPIndex( std::wstring & strID );

	// Sort ranking list.
	void RankerListSort();

	// Remove ranking list.
	void ReleaseRankerList();

public:

	// Send ranking list refresh packet.
	void OnSendRefreshRankerList();

	// Send donation packet.
	void OnSendMakeDonations( INT64 nCoin );

	// Called when DonationRewardDlg is first opened.
	// : No need to preload reward items, so load on first Open.
	void OnFirstOpenRewardDlg();

	// Donation result.
	void OnRecvContributionResult( char * pData );

	// Open donation window.
	void OnRecvShowContribution();

	// Sort ranking list - on donation, if LocalPlayer is in top 10, update my list's gold and sort.
	void SortRankerList( INT64 nContributionGold );


	// Ranker List.
	const std::vector< SContributionRanker * > & GetListRanker(){
		return m_vListRanker;
	}
	
	// LocalPlayer.
	int GetLPRank(){
		return m_nLPRank;
	}
	INT64 GetLPGold(){
		return m_nLPGold;
	}	

	// Donation reward item list.
	const std::vector< SContributionRewardItems > & GetRewardItemsMonth(){ 
		return m_vRewardMouth; // Monthly. 
	}
	const std::vector< SContributionRewardItems > & GetRewardItemsDay(){
		return m_vRewardDay;   // Daily.
	}

};

#endif