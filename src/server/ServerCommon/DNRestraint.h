	
#pragma once

class CDNUserBase;

class CDNRestraint:public TBoostMemoryPool<CDNRestraint>
{
public:
	CDNRestraint(CDNUserBase* pSession);
	~CDNRestraint();

	void Initialize();

	bool LoadRestraint(TRestraintData * pData);
	bool DelRestraint(UINT nRestraintKey);	
	
	bool CheckRestraint(int nType, bool bSend = true);
	void SendRestraintList();

protected:
	TRestraintData m_RestraintData;

	TRestraint * m_pRestraintChat;
	TRestraint * m_pRestraintTrade;

private:
	CDNUserBase* m_pSession;
};