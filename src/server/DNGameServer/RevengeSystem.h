#pragma once

#if defined(PRE_ADD_REVENGE)
class CPvPGameMode;
class CDNUserSession;

class CRevengeSystem : public TBoostMemoryPool<CRevengeSystem>	//Is a memory pool needed??
{
public:
	CRevengeSystem(CPvPGameMode* pGameMode);
	virtual ~CRevengeSystem();

	bool CheckRevenge( CDNUserSession *pGameSession, CDNUserSession *pHitterSession );
	void LeaveUser(UINT nSessionID);	

private:
	UINT GetRevengeTargetSessionID(UINT nSessionID)	{ return m_MapRevengeTarget[nSessionID]; }
	void SetRevengeTargetSessionID(UINT nSessionID, UINT nTargetSessionID) { m_MapRevengeTarget[nSessionID] = nTargetSessionID; }

	int IncreaseRevengeConditionCount(UINT nSessionID, UINT nTargetSessionID);
	void ResetRevengeConditionCount(UINT nSessionID);
	void ResetRevengeConditionTarget(UINT nSessionID);
	void ResetAllRevengeConditionCount();

protected:
	CPvPGameMode * m_pGameMode;

	std::map<UINT,std::map<UINT,int>> m_MapRevengeConditions;
	std::map<UINT, UINT> m_MapRevengeTarget;
};

#endif