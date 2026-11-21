#pragma once

#include "DnGameTask.h"


class CDnDarklairRequestChallengeDlg;
class CDnDLGameTask : public CDnGameTask
{
public:
	CDnDLGameTask();
	virtual ~CDnDLGameTask();

protected:
	enum { MSGBOX_REQUEST_CHALLENGE = 0 };
	CDnDarklairRequestChallengeDlg* m_pDnDarklairRequestChallengeDlg;

	int m_nCurrentRound;
	int m_nTotalRound;
#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	bool m_bChallengeDarkLair;
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )
#ifdef PRE_MOD_DARKLAIR_RECONNECT
	bool m_bBossRound;
#endif // PRE_MOD_DARKLAIR_RECONNECT

protected:
	virtual bool OnDungeonReadyGate( CDnWorld::DungeonGateStruct *pStruct );

#ifdef PRE_MOD_LOAD_DARKLAIR_MONSTER
	virtual void PreGenerationMonster( int nRandomSeed );
#endif

	void ResetRound( bool bForceReset = false );

public:
	void UpdateRound( bool bBoss );
	int GetTotalRound() { return m_nTotalRound; }
	int GetRound() { return m_nCurrentRound; }

	virtual bool InitializeStage( int nCurrentMapIndex, int nGateIndex, int nStageConstructionLevel, int nRandomSeed, bool bContinueStage, bool bDirectConnect );
	virtual bool InitializeNextStage( const char *szGridName, const char *szEnviName, int nMapTableID, int nStageConstructionLevel, int nStartPositionIndex );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	virtual void OnGhost( DnActorHandle hActor );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	virtual void OnRecvRoomDLDungeonClearMsg( SCDLDungeonClear *pPacket );
	virtual void OnRecvRoomDLDungeonClearRankInfo( SCDLRankInfo *pPacket );
	virtual void OnRecvRoomDungeonClearSelectRewardItem( char *pPacket ); // Go to reward selection screen
	virtual void OnRecvRoomSelectRewardItem( SCSelectRewardItem *pPacket ); // When someone selects an item
	virtual void OnRecvRoomDungeonClearRewardItem( SCDungeonClearRewardItem *pPacket ); // Open box color
#if defined ( PRE_ADD_NAMEDITEM_SYSTEM )
	virtual void OnRecvRoomDungeonClearRewardBoxType(SCDungeonClearBoxType* pData);
#endif
	virtual void OnRecvRoomDungeonClearRewardItemResult( SCDungeonClearRewardItemResult *pPacket ); // Open reward item
	virtual void OnRecvRoomWarpDungeonClear( SCWarpDungeonClear *pPacket );
	virtual void OnRecvRoomIdentifyRewardItem( SCIdentifyRewardItem *pPacket );
	virtual void OnRecvRoomDungeonClearWarpFailed(SCDungeonClearWarpOutFail* pPacket);
	virtual void OnRecvDLChallengeRequest();
	virtual void OnRecvDLChallengeResponse( SCDLChallengeResponse *pPacket );
	void ShowRequestChallengeDlg( LPCWSTR pwszMessage );
	void EnableRequestChallengeDlg(bool bEnable);

#if defined( PRE_ADD_CHALLENGE_DARKLAIR )
	bool IsChallengeDarkLair() { return m_bChallengeDarkLair; }
#endif	// #if defined( PRE_ADD_CHALLENGE_DARKLAIR )

#ifdef PRE_MOD_DARKLAIR_RECONNECT
	void OnRecvDarkLairRoundInfo( SCDLRoundInfo* pData );
#endif 

};
