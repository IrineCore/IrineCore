#pragma once

#include "Task.h"
#include "MessageListener.h"


#define FISHING_RANDOM_STAND_TIME	80.0f

enum eFishingState { STATE_NONE = 0, STATE_FISHING, STATE_FISHING_END, STATE_SHOW_RESULT };

class CDnLifeSkillFishingTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnLifeSkillFishingTask>
{
public:
	CDnLifeSkillFishingTask();
	virtual ~CDnLifeSkillFishingTask();

public:
	bool Initialize();
	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

public:
	void OnRecvFishingPlayer( SCFishingPlayer* pPacket );		// Receive info of character currently fishing
	void SendFishingReady();									// Request fishing preparation
	void OnRecvFishingReady( SCFishingReady* pPacket );			// Receive fishing ready complete

	void SendFishingStart();									// Request fishing start
	void OnRecvCastBaitResult( SCCastBait* pPacket );			// Receive fishing start error
	void OnRecvFishingStart( SCFishingPattern* pPacket );		// Receive fishing pattern and start
	void OnRecvFishingSync( SCFishingSync* pPacket );			// Sync during fishing

	void OnRecvStopFishingResult( SCStopFishing* pPacket );		// Receive error on fishing cancellation
	void OnRecvFishingEnd();									// Fishing ended
	void OnRecvFishingResult( SCFishingReward* pPacket );		// Receive fishing result
	
	void DoFishingEnd();						// Start fishing motion

	void SendFishingCancel( bool bRevisePos );	// Inform server of fishing cancellation

	eFishingState GetFishingState() { return m_eFishingState; }

	void SendPullingRodElapsedTime( bool bPullingRod );
	bool IsRecieveFishingEnd() { return m_bRecieveFishingEnd; }

	bool IsNowFishing();
	bool IsRequestFishingReady() { return m_bRequestFishingReady; }
#ifdef PRE_ADD_CASHFISHINGITEM
	int GetFishingRodItemID() { return m_nFishingRodItemID; }
#endif // PRE_ADD_CASHFISHINGITEM

private:
	void ResetFishing( bool bRevisePos );
	bool CheckFishingTool();
	int CheckFishingArea();							// Check fishing area; return fishing area ID if valid, else return -1
	void SendAction( eActor::eCSActor eAction );

private:
	eFishingState m_eFishingState;			// Fishing state

	bool		m_bRequestFishingReady;
	bool		m_bRunningFishingTask;		// Run fishing task
	bool		m_bActionNormalStand2;		// Check if performing the 2nd normal stand action in auto-fishing
	bool		m_bRecieveFishingEnd;		// Received fishing end from server?
#ifdef PRE_ADD_CASHFISHINGITEM
	bool		m_bCashFishingRod;			// If fishing rod is a cash item
	bool		m_bCashFishingBait;			// If bait is a cash item
	int			m_nFishingRodItemID;		// Fishing rod item ID
#endif // PRE_ADD_CASHFISHINGITEM
	int			m_nFishingRodIndex;			// Fishing rod inventory index
	int			m_nFishingBaitIndex;		// Fishing bait inventory index
//	float m_fFishingRandomStandTime; // Time to randomly take 2nd stand motion
	float		m_fRotate;					// Direction of Fishing Area
	EtVector3	m_vecPlayerStartPos;		// Character position before fishing starts (to compensate forward movement on manual<->auto switch)
	EtVector2	m_vecAreaDir;				// Direction toward the water's edge (Event Area direction)
	SCFishingReward	m_stFishingReward;		// Fishing result info

#ifdef PRE_ADD_FISHING_RESULT_SOUND
	int m_nFishingSuccessSound;
	int m_nFishingFailSound;
	EtSoundChannelHandle m_hFishingSound;
#endif
};

#define GetLifeSkillFishingTask()		CDnLifeSkillFishingTask::GetInstance()
