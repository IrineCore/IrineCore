#pragma once

#include "ConnectionManager.h"
#include "DNBaseObjectManager.h"

/*---------------------------------------------------------------------------------------
									CDNUserSessionManager

- Inherits from CDNBaseObjectManager
- Manager that manages UserSession
- Creates and destroys UserSession

	m_KeyObjects(TMapConnections): first - ObjectUID
	m_NameKeyObjects(TMapNameObjects): first - CharacterName
---------------------------------------------------------------------------------------*/

class CDNUserSession;

typedef map<UINT, CDNUserSession*> TMapSession;
typedef std::map<std::wstring,CDNUserSession*> TMapNameSession;
typedef vector<CSocketContext*> TVecSession;
typedef vector<CDNUserSession*> TVecTempSession;

class CDNUserSessionManager
{
private:
	DWORD m_dwChangeSaveTick;
	DWORD m_dwLastSaveTick;

	CBuffer* m_pChangeSaveBuffer;
	CBuffer* m_pLastSaveBuffer;
	bool m_bServiceClose;

	int m_nProcessCount;

	CSyncLock m_NameLock;
	TMapNameSession m_pMapName;		// key: charactername

	TMapSession m_pMapAccountDBID;	// key: AccountDBID
	TMapSession m_pMapSessionID;	// key: SessionID

	CSyncLock m_VecLock;
	TVecSession m_pVecSession;

	std::map <int, int> m_MapChannelUserCount;	//User count per channel (counting only)
	CSyncLock m_AccountLock;

	CSyncLock m_VecTempLock;
	TVecTempSession m_pVecTempSession;

public:
	//typedef	std::vector<CDNUserSession*, boost::pool_allocator<CDNUserSession*> >	TP_LISTAUTO;
	//typedef	TP_LISTAUTO::iterator				TP_LISTAUTO_ITR;
	//typedef	TP_LISTAUTO::const_iterator			TP_LISTAUTO_CTR;

private:
	CDNUserSession * FindUserSessionByAccountDBIDAsync(UINT nAccountDBID);

protected:
	void Clear();

public:
	CDNUserSessionManager(void);
	virtual ~CDNUserSessionManager(void);

	void InternalDoUpdate(DWORD CurTick);
	void ExternalDoUpdate(DWORD CurTick);

	CDNUserSession *AddSession(const char *pIp, const USHORT nPort);
	bool DelSession(CDNUserSession *pUserSession, CSocketContext* pDelSocketContext );
	bool InsertSession(CDNUserSession *pUserSession);
	bool DelTempSession(CDNUserSession *pUserSession, CSocketContext* pDelSocketContext );
	bool InsertTempSession(CDNUserSession * pUserSession);
	bool InsertSessionByName(CDNUserSession *pUserSession);
	bool SwapUserKeyName(const WCHAR *pwszOriginName, const WCHAR* pwszCharacterName);

	int GetUserCount();
	UINT GetUserCount(std::map <int, int> & pMap);
	void IncreaseChannelUserCount(int nChannelID);
	void DecreaseChannelUserCount(int nChannelID);

	CDNUserSession *FindUserSession(UINT nSessionID);
	CDNUserSession *FindUserSessionByName(const WCHAR *pwszName);
	CDNUserSession *FindUserSessionByObjectID(UINT nObjID);
	CDNUserSession* FindUserSessionByAccountDBID(UINT nAccountDBID);
	CDNUserSession* FindTempUserSession(UINT nSessionID);

	void SendAddUserList();	// Send all connected users to Master (when Master crashed and restarted)
	void SendNotice(const WCHAR * pMsg, const int nLen, int nShowSec);		//Notice! Send to all connected users
	void SendChat(eChatType eType, int cLen, const WCHAR *pwszCharacterName, const WCHAR *pwszChatMsg, char cRet = 1);
	void SendZoneChat(int nMapIdx, const WCHAR * pFromName, const WCHAR * pMsg, int nLen);
	void SendWorldSystemMsg(const WCHAR *pFromName, char cType, int nID, int nValue, const WCHAR *pToName=NULL);
	void SendNoticeCancel();
	void SendCloseService(__time64_t _tNow, __time64_t _tOderedTime, __time64_t _tCloseTime);

	void SendSaleAbortList(BYTE cCount, int *nAbortList);
	void SendAssginPeriodQuest (UINT nQuestID, bool bFlag);
	void SendPeriodQuestNotice(int nScheduleID, int nNoticeCount);

	void SendGuildWarEvent (short wScheduleID, char cEventStep, char cEventType);
	void SendGuildWarEventTime();
	void SendGuildWarTournamentWin(SCGuildWarTournamentWin* pPacket);
	void ResetGuildWarBuyedItems();

#ifdef PRE_ADD_CHAGNESERVER_LOGGING
	bool AddSaveSendData(int nMainCmd, char *pData, int nSize);
#else		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING
	void AddSaveSendData(int nMainCmd, char *pData, int nSize);
#endif		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING
	void DetachAllConnection();

	//void GetList(TP_LISTAUTO& pList);
#ifdef _USE_VOICECHAT
	void TalkingUpdate(UINT * pMemberArr);
#endif

#if defined( PRE_DRAGONBUFF )
	void ApplyWorldBuff( WCHAR *wszCharacterName, int nItemID, int nMapIdx );
#endif

};

extern CDNUserSessionManager* g_pUserSessionManager;
