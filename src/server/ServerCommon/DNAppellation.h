#pragma once

class CDNUserSession;

class CDNAppellation:public TBoostMemoryPool<CDNAppellation>
{
public:
	CDNAppellation( CDNUserSession *pUser );
	~CDNAppellation();

protected:
	CDNUserSession *m_pUserSession;
	std::map<int, __time64_t> m_mPeriodAppellation;	
	bool m_bResetPcbangAppellation;

protected:
	void BrodcastChangeAppellation();

public:
	bool LoadUserData();
	void CheckCollectionBookReward();

	bool IsExistAppellation( int nArrayIndex );	// Since it searches by ArrayIndex, subtract 1 when looking up by id in ext. Added this note because it's confusing /saset
	void AddAppellation( int nArrayIndex );
	void SelectAppellation( int nArrayIndex, int nCoverArrayIndex );
	int GetSelectAppellation();
	int GetCoverAppellation();
	bool OnRecvSelectAppellation( int nArrayIndex, int nCoverArrayIndex );
	bool OnRecvCollectionBook(int nTitleID);
#if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
	void RequestAddAppellation( int nArrayIndex, int nMailID = 0 );
#else
	void RequestAddAppellation( int nArrayIndex );
#endif	//	#if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
	void SetPCBang();

	void AddPeriodAppellation(int nArrayID, __time64_t tExpireDate);	
	void SetPeriodAppellation(char *Appellation, std::map<int, int> &mapDelAppellation);
	void SendPeriodAppellationTime( int nArrayID = 0, __time64_t tExpireDate = 0, bool bAllSend = true );	
	bool CheckResetPcbangAppellation() { return m_bResetPcbangAppellation; };
	void SetResetPcbangAppellation(bool bFlag) { m_bResetPcbangAppellation = bFlag; };

	bool CheckAppellation();
	void SendDelAppellation(int ArrayIndex);
	void SetPCbangAppellation(bool bSend = false);
	void ResetPCbangAppellation(bool bCheck);
	void SendDelPCBangAppellation(std::vector<TPCBangParam> &VecPCBangParam);
#if defined( PRE_ADD_BESTFRIEND )
	void DelAppellation( AppellationType::Type::eCode eType, bool CheckAppellation = false );
#endif
#if defined( PRE_ADD_NEWCOMEBACK )
	void CheckComebackAppellation( int nArrayIndex, int nCoverArrayIndex );
#endif
#if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
	void CheckCollectionBook(int nTitleID);
#endif	// #if defined(PRE_MOD_APPELLATIONBOOK_RENEWAL)
};