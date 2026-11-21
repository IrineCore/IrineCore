
#pragma once

#if defined( PRE_ADD_RACING_MODE )

#include "PvPGameMode.h"

class CPvPRacingMode:public CPvPGameMode
{
public:

	CPvPRacingMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket );
	virtual ~CPvPRacingMode();

	virtual bool bIsRacingMode() { return true; }

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );	
	virtual void OnCheckFinishRound( PvPCommon::FinishReason::eCode Reason );	

	virtual void SetRacingStart();
	virtual void SetRacingEnd(UINT nSessionID);
	virtual void OnLeaveUser( DnActorHandle hActor );

	virtual BYTE GetRacingRanking(UINT uiSessionID);

private:	
	bool  m_bStartRacing;
	bool  m_bEndRacing;	
	bool  m_bFirstEnd;									// Whether 1st place has entered
	std::map<UINT, DWORD> m_mStartTick;				// Start time per user
	std::map<UINT, DWORD> m_mEndTick;					// Goal-in time per user	
	UINT m_nRacingRanking[PvPCommon::Common::MaxPlayer];  // SessionID by rank
	int m_nRacingIndex;	

	void SendRacingRapTime();	
	void SendRacingFirstEnd(UINT uiSessionID);
	bool CheckRacingEnd(DnActorHandle hActor);	
};

class IBoostPoolPvPRacingMode : public CPvPRacingMode, public TBoostMemoryPool<IBoostPoolPvPRacingMode>
{
public :
	IBoostPoolPvPRacingMode( CDNGameRoom* pGameRoom, const TPvPGameModeTable* pPvPGameModeTable, const MAGAPVP_GAMEMODE* pPacket )
		:CPvPRacingMode(pGameRoom,pPvPGameModeTable,pPacket){}
	virtual ~IBoostPoolPvPRacingMode(){}
};

#endif //#if defined( PRE_ADD_RACING_MODE )
