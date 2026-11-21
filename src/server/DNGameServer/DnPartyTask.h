#pragma once

#include "Task.h"
#include "MultiSingleton.h"
#include "GameListener.h"
#include "DNGameRoom.h"
#include "DnWorld.h"

struct _KICKMEMBER
{
	ULONG nKickTick;
	BYTE cWorldID;
	UINT nAccountDBID;
	UINT nSessionID;
	INT64 biCharacterDBID;
	CSPartyMemberKick Kick;
};

class CDNUserSession;
class CDnPartyTask : public CTask, public CMultiSingleton<CDnPartyTask, MAX_SESSION_COUNT>, public CGameListener
{
public:
	CDnPartyTask(CDNGameRoom * pRoom);
	virtual ~CDnPartyTask();

protected:
	//int m_nPartyIndex;
	//tstring m_szPartyName;

	int m_nRandomSeed;
	int m_nEnteredGateIndex;
	bool m_bSyncComplete;
	int m_nPartyChainCount;
	bool m_bWaitGate;

	DNVector(_KICKMEMBER) m_KickList;
	struct ReversionItemGetUserInfo
	{
		UINT	sessionID;
		bool	bJoinDecided;
		int		diceValue;

		ReversionItemGetUserInfo()
		{
			bJoinDecided = false;
			sessionID = -1;
			diceValue = -1;
		}

		bool IsEmpty() const { return bJoinDecided == false; }
	};

	struct ReversionItemGetUnit
	{
		DnDropItemHandle hDropItem;
		TItem	itemInfo;
		float	time;
		std::vector<ReversionItemGetUserInfo>	userInfo;

		ReversionItemGetUnit()
		{
			time = 0.f;
		}
	};
	std::deque<ReversionItemGetUnit> m_RequestGetReversionItemList;
	bool	m_bHoldSharingReversionItem;
	CMtRandom	m_Random;

protected:	
	int GetCheckGateIndex(CDNUserSession * pSession);
	bool CheckAndRequestGateIndex(int nGateNo, int nMemberIdx);

	int OnRecvRoomMessage(CDNUserSession * pSession, int nSubCmd, char * pData, int nLen );
	int OnRecvPartyMessage( CDNUserSession *pSession, int nSubCmd, char *pData, int nLen );

//	virtual void CalcDungeonEnterGateCondition( CDnWorld::DungeonGateStruct *pStruct, short &nDungeonEnterPermit );
//	virtual void CalcDungeonEnterStartCondition();

	void ProcessGetReversionItemList(LOCAL_TIME LocalTime, float fDelta);

public:
	bool Initialize();
	void ResetGateIndex();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	void RequestPartyMember();
	void RequestSyncStart();
	bool CheckPartyFatigue();
	
	bool IsSyncComplete() { return m_bSyncComplete; }
	void SetSyncComplete( bool bValue ) { m_bSyncComplete = bValue; }	

	virtual void OutPartyMember( UINT nOutPartyMemberUID, UINT nNewLeaderUID, char cKickKind );
	void UpdateGateInfo();

	int OnDispatchMessage(CDNUserSession *pSession, int nMainCmd, int nSubCmd, char *pData, int nLen );

	virtual int OnRecvPartyRefreshGateInfo( CDNUserSession *pSession, CSRefreshGateInfo *pPacket, int nLen );
	virtual int OnRecvPartyStartStage(CDNUserSession *pSession, CSStartStage *pPacket, int nLen);
	virtual int OnRecvPartyCancelStage( CDNUserSession *pSession, char *pPacket, int nLen );
	virtual int OnRecvPartyMemberKick(CDNUserSession * pSession, char * pPacket, int nLen);
	virtual int OnRecvPartySelectDungeon(CDNUserSession * pSession, char * pPacket, int nLen);
	virtual int OnRecvPartyLeaderSwap(CDNUserSession * pSession, char * pPacket, int nLen);
	
	virtual int OnRecvRoomSyncWait( CDNUserSession *pSession, CSSyncWait *pPacket, int nLen );
	virtual int OnRecvPartyJoinGetReversionItem(CDNUserSession *pSession, char *pPacket);

	void AddRequestGetReversionItem(const TItem& itemInfo, DnDropItemHandle hDropItem);
	void AddRequestGetReversionItemUserInfo(ReversionItemGetUnit& unit, bool bSendUserJoin);
	void SendRequestJointGetReversionItem(const TItem& orgItemInfo, const DWORD& dropUniqueID);
	void PickOutInvalidReversionItemUnit();
	bool IsEnableAddRequestGetReversionItem(DnDropItemHandle hDropItem);
	bool IsPartySharingReversionItem() const;

	void SendRefreshGate(char * pGateNo, int nRet);
	bool SyncAIBotGateReady( CDNUserSession* pBotSession, int nGateIndex );
	int GetHumanEnteredGateIndex();

	void HoldSharingReversionItem();
	void ReleaseSharingReversionItem();
	void ClearReversionItemList();

	virtual int OnRecvPartyMemberAbsent(CDNUserSession * pSession, CSMemberAbsent * pPacket, int nLen);
	virtual int OnRecvPartyInviteMember(CDNUserSession * pSession, CSInvitePartyMember * pPacket, int nLen);
	virtual int OnRecvPartySwapMemberIndex(CDNUserSession * pSession, CSPartySwapMemberIndex * pPacket, int nLen);
#if defined( PRE_PARTY_DB )
	virtual int OnRecvPartyModify(CDNUserSession * pSession, CSPartyInfoModify * pPacket, int nLen);
#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)
	int OnRecvPartyAskJoinDecision(CDNUserSession * pSession, CSPartyAskJoinDecision * pPacket, int nLen);
#endif
#endif // #if defined( PRE_PARTY_DB )

	//inline int GetPartyIndex() { return m_nPartyIndex; }
	inline int GetEnteredGate() { return m_nEnteredGateIndex; }
	inline bool GetWaitGate() { return m_bWaitGate; }
	void SetWaitGate(bool bWait) { m_bWaitGate = bWait; }

#if defined( PRE_PARTY_DB )	
	void AddPartyDB( CDNUserSession* pBreakIntoSession );	
	void AddPartyMemberDB( CDNUserSession* pSession );	
	void DelPartyDB();
	void DelPartyMemberDB( CDNUserSession *pSession );	
#endif

#if defined( PRE_ADD_NAMEDITEM_SYSTEM )
	void ApplyPartyEffectSkillItemData( CSUseItem *pPacket, TItemData *pItemData, UINT nUseSessionID, EffectSkillNameSpace::ShowEffectType::eType eType, bool bSendDB, bool bUseItem );
#endif

#if defined( PRE_ADD_NEWCOMEBACK )
	bool CheckPartyMemberAppellation( int nAppellationID );		
	void DelPartyMemberAppellation( int nAppellationID );
	void ApplyJoinMemberAppellation( CDNUserSession * pSession );
#endif
	float GetPlayerDropUpBlowValue();

private:
	void _RequestSyncStartMsg();

	bool	IsSameDiceNumber(const std::vector<ReversionItemGetUserInfo>& userList, int diceNumber) const;
	int		Dice(const std::vector<ReversionItemGetUserInfo>& userList);
};

class IBoostPoolDnPartyTask:public CDnPartyTask, public TBoostMemoryPool< IBoostPoolDnPartyTask >
{
public:
	IBoostPoolDnPartyTask(CDNGameRoom* pRoom):CDnPartyTask(pRoom){}
	virtual ~IBoostPoolDnPartyTask(){}
};
