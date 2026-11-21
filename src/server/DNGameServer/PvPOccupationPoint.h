
#pragma once

class CDNPvPGameRoom;
class CPvPOccupactionSystem;

class CPvPOccupationPoint : public TBoostMemoryPool<CPvPOccupationPoint>
{
public:
	CPvPOccupationPoint(CDNGameRoom * pGameRoom, CPvPOccupactionSystem * pSystem, SOBB * pOBB, TPositionAreaInfo * pAreaInfo);
	~CPvPOccupationPoint();

	void Process(LOCAL_TIME LocalTime, float fDelta);
	void OnDamage(DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage);

	int GetID() { return m_OccupationInfo.nAreaID; }
	bool CheckInside(EtVector3 &Position);
	void GetPointState(PvPCommon::OccupationStateInfo &Info);

	bool TryAcquirePoint(DnActorHandle hActor, LOCAL_TIME LocalTime);
	bool CancelAcquirePoint(DnActorHandle hActor);

	bool ChangeClimaxMode();

private:
	CDNGameRoom * m_pGameRoom;
	CPvPOccupactionSystem * m_pOccupationSystem;
	SOBB m_OBB;

	int m_nOccupantState;		//Occupation state PvPCommon::OccupationState	
	UINT m_nOccupantUniqueID;	//Occupation winner
	short m_nOccupantTeamID;			//Owner (team)
	
	UINT m_nTryUniqueID;		//Attempting player/entity
	short m_nTryTeamID;
	std::wstring m_wstrTryName;	//Name of attempting player/entity

	UINT m_nWaitUniqueID;
	short m_nWaitTeamID;
	std::wstring m_wstrMoveWaitName;	//Name of attempting player/entity

	LOCAL_TIME m_nTryOccupationTime;		//Capture attempt start time
	LOCAL_TIME m_nMoveWaitOccupationTime;	//Capture move start time
	LOCAL_TIME m_nLastOccupationCheckTime;	//Capture time

	TPositionAreaInfo m_OccupationInfo;
	volatile bool m_bClimaxMode;

	//Check
	void _CheckPoint(LOCAL_TIME LocalTime);
	void _ClearTry();
	void _ClearMoveWait();
	void _OnOccupyArea();

	//
	bool _AcquirePoint(int nUniqueID);
	bool _IsAcquireResourceTerm(LOCAL_TIME LocalTime);
	bool _IsFinishedTryTerm(LOCAL_TIME LocalTime);
	bool _IsFinishedMoveWaitTerm(LOCAL_TIME LocalTime);

	//send
	void _SendOccupationPointState();
};
