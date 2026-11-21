
#pragma once

class CDNPvPGameRoom;
class CPvPOccupactionSystem;
class CPvPOccupationTeamSkill;

class CPvPOccupationTeam : public TBoostMemoryPool<CPvPOccupationTeam>
{
public:
	CPvPOccupationTeam(CDNGameRoom * pGameRoom, CPvPGameMode * pGameMode, int nTeam, UINT nLimit, int nMaxPoint, int nBossID, UINT nGuildDBID, int nQualifyingScore);
	~CPvPOccupationTeam();

	void InitializeOccupationTeam(int nBossID, CEtWorldEventArea * pArea = NULL);

	void Process(LOCAL_TIME LocalTime, float fDelta);

	UINT GetGuildID() { return m_nGuildDBID; }
	UINT GetTeamID() { return m_nTeamID; }
	int GetBossID() {return m_nTeamBossID; }
	EtVector3 * GetSpawnPos() { return &m_OBB.Center; }
	SOBB * GetSOBB() { return &m_OBB; }
	void SetTeamResource(UINT nValue) { m_nTeamResource = nValue; }
	UINT GetTeamResource() { return m_nTeamResource; }
	UINT GetTeamResourceSum() { return m_nTeamResourceSum; }
	UINT GetOwnedCount() { return (UINT)m_lOwnedAreaList.size(); }
	int GetCurSkillPoint() { return m_nCurSkillPoint; }
	void GetTeamSkillInfo(PvPCommon::OccupationSkillSlotInfo &Info);
	int GetQualifyingScore() { return m_nQualifyingScore; }
	int GetPrevDuplicateID() { return m_nPrevDuplicateID; }
	int GetPrevUsingSkillID() { return m_nPrevUsingSkillID; }

	//Boss
	bool IsBoss(DnActorHandle hActor);
	bool IsBossDie();
	void SetBossHandle(DnActorHandle hBoss);

	//Point
	bool AcquirePoint(int nAreaID);
	bool SnatchedPoint(int nAreaID);
	void GainPoint(int nGain);

	//Skill
	void ClearTeamSkill();
	bool AddTeamSkill(int nSlotIndex, TBattleGroundSkillInfo * pSkill, TBattleGroundEffectValue * pEffect);
	bool UseSkill(CDNUserSession * pSession, int nSkillID);
	bool SwapSkillIndex(char cFrom, char cTo);
	bool RemovePrevSkillEffect();

private:
	CDNGameRoom * m_pGameRoom;
	CPvPGameMode * m_pGameMode;
	UINT m_nGuildDBID;			//If Guild War
	int m_nQualifyingScore;		//Preliminary score when in Guild War
	int m_nTeamID;
	int m_nMaxSkillPoint;
	int m_nCurSkillPoint;
	int m_nPrevDuplicateID;
	int m_nPrevUsingSkillID;

	int m_nTeamBossID;		//boss
	SOBB m_OBB;				//boss spawn
	std::vector <DnActorHandle> m_vhBossActor;

	UINT m_nTeamResource;
	UINT m_nTeamResourceLimit;
	UINT m_nTeamResourceSum;
	std::list <int> m_lOwnedAreaList;
	std::map <int, CPvPOccupationTeamSkill*> m_mOccupationTeamSkill;

	void _CrealSkill();
	CPvPOccupationTeamSkill * GetTeamSkill(int nSkillID);
	CPvPOccupationTeamSkill * GetTeamSkillBySlotIndex(char cIndex);
};
