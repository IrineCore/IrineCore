#pragma once

//#include "ClientTcpSession.h"
//#include "ClientUdpSession.h"
#include "MessageListener.h"
#include "DNPacket.h"
#include "Task.h"
#include "Singleton.h"
#include "DnPartyTask.h"
#ifdef PRE_ADD_PVP_TOURNAMENT
#include "DnPVPTournamentDataMgr.h"
#endif

//class CDnBridgeTask : public CTask, public CClientTcpSession, public CClientUdpSession, public CEtUICallback, public CSingleton<CDnBridgeTask>
class CDnBridgeTask : public CTask, public CTaskListener, public CEtUICallback, public CSingleton<CDnBridgeTask>
{
public:
	CDnBridgeTask();
	virtual ~CDnBridgeTask();
#ifdef PRE_ADD_PVP_TOURNAMENT
	struct sGameStatus{
		std::wstring wszPVPRoomName;
		byte m_cMaxPlayer;
		int m_nObjectiveCount;
		int m_nGameModeTableID;
		int m_nTimeSec;
		bool m_bPVPShowHp;
		bool m_bDisablePVPRegulation;

		sGameStatus():
		m_nObjectiveCount(0),
			m_nGameModeTableID(0),
			m_nTimeSec(0),
			m_cMaxPlayer(0)
			,m_bPVPShowHp(0)
			,m_bDisablePVPRegulation( false )
		{
			wszPVPRoomName = L"";
		};
	};
#endif

	void ConnectResult(bool bRet, eBridgeState eState, char cReqGameIDType, int nGameTaskType);

protected:
	enum UICallbackEnum {
		GameInfoFailed = 100000,
		VillageInfoFailed,
		ConnectVillageFailed,
		ReconnectLoginFailed,
		DisconnectTCP,
	};

	eBridgeState m_BridgeState;
	UINT		m_nSessionID;
	int			m_nVillageMapIndex;
	int			m_nEnteredGateIndex;
	int			m_nStageRandomSeed;
	int			m_nStageConstructionLevel;
	int			m_nCurrentCharLevel;
	int			m_nGameTaskType;
	BYTE		m_cGateSelectIndex;
//	BYTE		m_cReqGameIDType;

	UINT m_nAccountDBID;
	INT64 m_biCertifyingKey;

#if defined( PRE_ADD_FARM_DOWNSCALE )
	int m_iAttr;
#elif defined( PRE_ADD_VIP_FARM )
	Farm::Attr::eType m_Attr;
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )

	char m_szLoginServerVersion[128];
	char m_szVillageServerVersion[16];
	char m_szGameServerVersion[16];
//blondymarry start
	int m_ChannelAtt;//Save channel attributes and branch task accordingly
#ifdef PRE_ADD_PVP_TOURNAMENT
#else
	struct sGameStatus{
		std::wstring wszPVPRoomName;
		byte m_cMaxPlayer;
		int m_nObjectiveCount;
		int m_nGameModeTableID;
		int m_nTimeSec;
		bool m_bPVPShowHp;
		bool m_bDisablePVPRegulation;

		sGameStatus():
		m_nObjectiveCount(0),
		m_nGameModeTableID(0),
		m_nTimeSec(0),
		m_cMaxPlayer(0)
		,m_bPVPShowHp(0)
		,m_bDisablePVPRegulation( false )
		{
			wszPVPRoomName = L"";
		};
	};
#endif
	sGameStatus m_sBackUpRoomStatus;//Save because village and game servers differ and cannot send the info  

	std::vector< const char *> m_TaskNameList;

	std::wstring m_wszCurrentServerName;	// Leaving it here prevents it from being erased.
	bool m_bZeroPopulation;	// Comes as true if the game server is adjusting population.
	bool m_bInvalidDisconnect;
	int m_nClientPingMs;
	float m_fClientPingTimer;

	bool m_bIsGuildWarSystem;

	int m_nPreviousMapIndexForPvP;

#ifdef PRE_ADD_COMEBACK
	bool m_bComebackUser;
	bool m_bComeBackMessage; // #58893 Show comeback message only once.
#endif

#ifdef PRE_ADD_DOORS
	bool m_bIsDoorsMobileAuthUser;
#endif

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	PvPCommon::RoomType::eRoomType m_eSelectedChannelFromVillageAccessMode;
	bool m_bEnterLadderFromVillageAccessMode;
	PvPCommon::RoomType::eRoomType m_eGameStartedGradeChannel;
	int m_iGameStartedLadderChannel;
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
#endif // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS

	
#ifdef PRE_ADD_STAMPSYSTEM
	bool m_bShowStampDlg; // 
#endif // PRE_ADD_STAMPSYSTEM

#ifdef PRE_ADD_GAMEQUIT_REWARD
	bool m_bTakeNewbieReward;
	bool m_bNewbieReward;
	bool m_bNewbieRewardMessage;
	bool m_bLoginSequence; // Came through login.
	__time64_t m_timeAccountReg;
	__time64_t m_timeRewardCrrTime;
#endif // PRE_ADD_GAMEQUIT_REWARD

//blondymarry end
protected:
	void OnRecvLoginMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvSystemMessage( int nSubCmd, char *pData, int nSize );
	void OnRecvRoomMessage( int iSubCmd, char* pData, int iSize );

public:
	void InitializeDefaultTask();

public:
	UINT GetSessionID() { return m_nSessionID; }
	UINT GetAccountDBID() { return m_nAccountDBID; }
	virtual void OnConnectTcp();
	virtual void OnDisconnectTcp( bool bValidDisconnect );
	virtual void OnDisconnectUdp( bool bValidDisconnect );

	virtual void OnDispatchMessage( int nMainCmd, int nSubCmd, char *pData, int nSize );

	void OnRecvLoginCheckLogin( SCCheckLogin *pPacket );
	void OnRecvLoginCheckVersion( SCCheckVersion *pPacket );
	void OnRecvTutorialMapInfo( SCTutorialMapInfo *pPacket );
	void OnRecvSystemVillageInfo( SCVillageInfo *pPacket );
	void OnRecvSystemConnectVillage( SCConnectVillage *pPacket );
	void OnRecvSystemGameInfo( SCGameInfo *pPacket );
	void OnRecvSystemConnectedResult( SCConnectedResult* pPacket );
	void OnRecvSystemReconnectReq();
	void OnRecvSystemGameTcpConnectReq();	
#ifdef PRE_ADD_VIP
	void OnRecvVIPAutoPay(SCVIPAutomaticPay* pData);
#endif
	void OnRecvTcpPing( SCTCPPing* pData );
	void OnRecvClientPing( SCClientPing* pData );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	int GetClientPingMs() const { return m_nClientPingMs; }

//blondymarry start
	void OnRecvSystemMoveToLobby();	
	//void OnRecvSystemCharCommOption( SCCharCommOption *pPacket );

	int GetChannelAttribute() { return m_ChannelAtt; }
//	int IsPVPVillage() { return ( m_ChannelAtt & eChannelAttribute::CHANNEL_ATT_PVP ); }
//	int IsPVPLobby() { return ( m_ChannelAtt & eChannelAttribute::CHANNEL_ATT_PVPLOBBY ); }

#ifdef PRE_ADD_PVP_TOURNAMENT
	const sGameStatus& GetPVPRoomStatus(){return m_sBackUpRoomStatus;}
	bool IsLocalActorSessionID(UINT sessionID) const;
#else
	sGameStatus GetPVPRoomStatus(){return m_sBackUpRoomStatus;}
#endif
//blondymarry end

	void OnRecvDetachMessage(SCDetachMessage * pPacket);
	void OnRecvZeroPopulation(SCZeroPopulation * pPacket);
	void OnRecvSystemReconnectLogin( SCReconnectLogin *pPacket );
	// CEtUICallback
	virtual void OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg = 0 );

public:
	int GetVillageMapIndex() { return m_nVillageMapIndex; }
	int GetEnteredGateIndex() { return m_nEnteredGateIndex; }
	BYTE GetGateSelectIndex() { return m_cGateSelectIndex; }
	void SetGateSelectIndex( const BYTE cGateSelectIndex ) { m_cGateSelectIndex = cGateSelectIndex; }
	int GetStageRandomSeed() { return m_nStageRandomSeed; }
	void SetStageRandomSeed(int nRandomSeed) { m_nStageRandomSeed = nRandomSeed; }
	int GetStageConstructionLevel() { return m_nStageConstructionLevel; }

	void SetBridgeState( eBridgeState State ) { m_BridgeState = State; }
	eBridgeState GetBridgeState() { return m_BridgeState; }

	int GetCurrentCharLevel() { return m_nCurrentCharLevel; }
	void SetCurrentCharLevel( int nValue ) { m_nCurrentCharLevel = nValue; }

	const char *GetCurrentServerVersion();

	const WCHAR *GetCurrentServerName() { return m_wszCurrentServerName.c_str(); }
	void SetCurrentServerName( LPCWSTR wszServerName ) { m_wszCurrentServerName = wszServerName; }

	void SetPVPGameStatus( std::wstring RoomName , int nObjectiveCount , int nGameMode , int nTimeSec, byte cMaxPlayer , bool bPVPShowHp, bool bDisablePVPRegulation );
	void ResetPVPGameStatus();

	void SetZeroPopulation( bool bZeroPopulation ) { m_bZeroPopulation = bZeroPopulation; }
	bool IsZeroPopulation() { return m_bZeroPopulation; }

	void ResetCertifyingKey();

	char * AddTaskName( char *pTaskName ) { m_TaskNameList.push_back( pTaskName ); return pTaskName; }

	bool IsInvalidDisconnect() { return m_bInvalidDisconnect; }
	int GetGameTaskType() { return m_nGameTaskType; }
	bool IsGuildWarSystem() { return m_bIsGuildWarSystem; }
	void SetGuildWarSystem( bool bIsGuildWarSystem ) { m_bIsGuildWarSystem = bIsGuildWarSystem; }
	bool IsPvPGameMode( int eMode );

#if defined( PRE_ADD_FARM_DOWNSCALE )
	void InitAttr() { m_iAttr = Farm::Attr::None; }
	int GetAttr(){ return m_iAttr; }
	void AddAttr( Farm::Attr::eType Attr )	{ m_iAttr |= Attr; }
#elif defined( PRE_ADD_VIP_FARM )
	Farm::Attr::eType GetAttr(){ return m_Attr; }
	void SetAttr( Farm::Attr::eType Attr )	{ m_Attr = Attr; }
#endif // #if defined( PRE_ADD_FARM_DOWNSCALE )
	int GetPreviousMapIndexForPvP() const { return m_nPreviousMapIndexForPvP; }
	void SetPreviousMapIndexForPvP(int nMapIdx) { m_nPreviousMapIndexForPvP = nMapIdx; }

#ifdef PRE_ADD_COMEBACK
	void SetComeback( bool bComeback );
	bool GetComeback();
	void EndComebackMessage();
	bool GetComebackMessage();
#endif

#ifdef PRE_ADD_STAMPSYSTEM
	bool IsShowStampDlg(){
		return m_bShowStampDlg; // 
	}
	void SetStampDlg( bool b ){
		m_bShowStampDlg = b; // 
	}
#endif // PRE_ADD_STAMPSYSTEM

#ifdef PRE_ADD_GAMEQUIT_REWARD	
	void SetGameQuitReward( bool bNewbieReward, __time64_t time, __time64_t crrTime ){
		m_bNewbieReward = bNewbieReward;  m_timeAccountReg = time;  m_timeRewardCrrTime = crrTime; m_bLoginSequence=true;
	}
	bool GetNewbieReward(){
		return m_bNewbieReward;
	}
	void TakeNewbieReward(){
		m_bNewbieReward = false;
	}
	void SetNewbieRewardMessage(){
		m_bNewbieRewardMessage = true;
	}
	bool GetNewbieRewardMessage(){
		return m_bNewbieRewardMessage;
	}
	bool GetNewbieLoginSequence(){
		bool b = m_bLoginSequence;
		if( m_bLoginSequence )
			m_bLoginSequence = false;
		return b;
	}
	void EndNewbieRewardMessage();
	__time64_t GetAccountRegTime(){
		return m_timeAccountReg;
	} 
	__time64_t GetTimeRewardCrrTime(){
		return m_timeRewardCrrTime;
	}
#endif // PRE_ADD_GAMEQUIT_REWARD

#ifdef PRE_ADD_COLOSSEUM_BEGINNER
#ifdef PRE_ADD_PVP_VILLAGE_ACCESS
	void SetSelectedChannelEnterPVPLobbyFromVillageAccessMode( PvPCommon::RoomType::eRoomType eSelectedChannel ) { m_eSelectedChannelFromVillageAccessMode = eSelectedChannel; };
	PvPCommon::RoomType::eRoomType GetSelectedChannelEnterPVPLobbyFromVillageAccessMode( void ) { return m_eSelectedChannelFromVillageAccessMode; };
	void ResetSelectedChannelEnterPVPLobbyFromVillageAccessMode() { m_eSelectedChannelFromVillageAccessMode = PvPCommon::RoomType::max; };
	void SetEnterLadderFromVillageAccessMode() { m_bEnterLadderFromVillageAccessMode = true; };
	bool GetEnterLadderFromVillageAccessMode() { return m_bEnterLadderFromVillageAccessMode; };
	void ResetEnterLadderFromVillageAccessMode() { m_bEnterLadderFromVillageAccessMode = false; };
	void SetSelectedPVPGradeChannelOrLadder( PvPCommon::RoomType::eRoomType eGradeChannel, int iLadderChannel ) {  m_eGameStartedGradeChannel = eGradeChannel; m_iGameStartedLadderChannel = iLadderChannel; };
	void GetSelectedPVPGradeChannelOrLadder( PvPCommon::RoomType::eRoomType& eGradeChannel, int& iLadderChannel ) { eGradeChannel = m_eGameStartedGradeChannel; iLadderChannel = m_iGameStartedLadderChannel; };
	void ResetSelectedPVPGradeChannelOrLadder() { m_eGameStartedGradeChannel = PvPCommon::RoomType::max; m_iGameStartedLadderChannel = LadderSystem::MatchType::eCode::None; };
#endif // #ifdef PRE_ADD_COLOSSEUM_BEGINNER
#endif // #ifdef PRE_ADD_PVP_VILLAGE_ACCESS

#ifdef PRE_ADD_DOORS
	bool IsDoorsMobileAuthUser() const { return m_bIsDoorsMobileAuthUser; }
	void SetDoorsMobileAuthUser(bool bAlreadyAuth) { m_bIsDoorsMobileAuthUser = bAlreadyAuth; }
#endif

#ifdef PRE_ADD_PVP_TOURNAMENT
	CDnPVPTournamentDataMgr& GetPVPTournamentDataMgr();
#endif

private:
	std::vector<CDnPartyTask::PartyStruct> m_vPartyStruct;
#ifdef PRE_ADD_PVP_TOURNAMENT
	CDnPVPTournamentDataMgr m_PVPTournamentDataMgr;
#endif

	//void _OnRecvRoomMemberInfo( char* pData );
	//void _OnRecvRoomMemberTeam( char* pData );
	void _OnRecvRoomMemberDefaultParts( char* pData );
	void _OnRecvRoomMemberEquip( char* pData );
	void _OnRecvRoomMemberSkill( char* pData );
	void _OnRecvRoomStartDragonNest( char* pData );
	
	//void _OnRecvRoomMemberEternityItemData( char* pData );
};