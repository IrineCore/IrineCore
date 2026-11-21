#pragma once

#include "Task.h"
#include "VillageClientSession.h"
#include "DNProtocol.h"
#include "DnCamera.h"
#include "DnLoadingTask.h"
#include "EtUIDialog.h"
#include "InputReceiver.h"
#include "DnHangCrashTask.h"

class CDnWorld;
class CDnGameTask;
class CDnMinimap;
class CClientSocket;

class CDnVillageTask : public CTask, public CVillageClientSession, public CDnLoadingStateMachine, public CEtUICallback, public CInputReceiver, public CDnHangMonitorObject
{
public:
	CDnVillageTask();
	virtual ~CDnVillageTask();

	static bool s_bCompleteFirstRender;
protected:
	CDnWorld *m_pWorld;
	DnCameraHandle m_hDummyCamera;
	DnCameraHandle m_hPlayerCamera;
	DnCameraHandle m_hDebugCamera;
	bool m_bFinalizeStage;
	bool m_bEnterSuccess;
	bool m_bEnterLocal;
	bool m_bFirstConnect;
	bool m_bChannelMove;

	CDnMinimap *m_pMinimap;
	bool m_bRequestCancelStageSelf;

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	DnActorHandle m_hPreloadPlayer[10];
	bool m_bPreloadComplete;
#endif

	bool m_bIsCreateLocalActor;
#ifdef _FINAL_BUILD
#else
	float m_fTimerSec;
#endif

protected:
	// Loading-related functions
	static bool __stdcall OnLoadRecvCharUserEnteredCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime );
	static int __stdcall OnLoadRecvCharUserEnteredUniqueID( void *pParam, int nSize );
	static bool __stdcall OnLoadRecvCharUserEnteredCheckSkip( int nMainCmd, int nSubCmd );

	void ProcessDebugMsg( LOCAL_TIME LocalTime, float fDelta );
	void ProcessSound( LOCAL_TIME LocalTime, float fDelta );
	
public:
	// Local
	bool Initialize( bool bFirstConnect = true, bool bChannelMove = false );
	bool InitializeStage( const char *szGridName, const char *szEnviName, int nMapTableID );
	void FinalizeStage();
	bool InitializeEnvi( int nMapIndex, const char *szGridName, const char *szEnviName );

	void EnterPVPLobby();
	bool IsChannelMove() { return m_bChannelMove; }
	bool IsEnterSuccess() { return m_bEnterSuccess; }
	bool IsEnterLocal() { return m_bEnterLocal; }

	// CDnLoadingStateMachine
	bool RenderScreen( LOCAL_TIME LocalTime, float fDelta );

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime );
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

	// CVillageClientSession
	virtual void OnConnectTcp();
	virtual void OnDisconnectTcp( bool bValidDisconnect );

public:
	virtual void OnRecvCharMapInfo( SCMapInfo *pPacket );
	virtual void OnRecvCharEntered( SCEnter *pPacket );
	virtual void OnRecvCharUserEntered( SCEnterUser *pPacket, int nSize );
	virtual void OnRecvCharUserLeave( SCLeaveUser *pPacket );
	virtual void OnRecvCharHide( SCHide *pPacket );

#ifdef PRE_PARTY_DB
	virtual bool OnRecvPartyListInfo( SCPartyListInfo *pPacket );
#else
	virtual void OnRecvPartyListInfo( SCPartyListInfo *pPacket );
#endif
	virtual void OnRecvPartyMemberInfo( SCPartyMemberInfo *pPacket );
	
	virtual void OnRecvPartyCreateParty( SCCreateParty *pPacket );
	virtual void OnRecvPartyRefresh( SCRefreshParty *pPacket );
	virtual void OnRecvPartyJoinParty( SCJoinParty *pPacket );
	virtual void OnRecvPartyOutParty( SCPartyOut *pPacket );

	virtual void OnRecvPartyRefreshGateInfo( SCRefreshGateInfo *pPacket );
	virtual void OnRecvPartyReadyGate( SCGateInfo *pPacket );
//	virtual void OnRecvPartyStageStart( SCStartStage *pPacket );
	virtual void OnRecvPartyStageCancel( SCCancelStage *pPacket );

	virtual void OnRecvPartyMemberMove(SCPartyMemberMove *pData);

	virtual void OnRecvFarmInfo( SCFarmInfo * pPacket );
	virtual void OnRecvFarmPlantedInfo( SCFarmPlantedInfo * pPacket );
	virtual void OnRecvOpenFarmList();

#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	void InitializePreloadPlayer();
	void ReleasePreloadPlayer();
#endif

	bool IsCreateLocalActor(){
		return m_bIsCreateLocalActor;
	}
};
