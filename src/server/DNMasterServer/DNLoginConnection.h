#pragma once

#include "Connection.h"

class CDNLoginConnection: public CConnection
{
public:
	// Functor for sorting login server sessions (descending by highest available concurrent user capacity)
	class CSortLoginServerSession
	{
	public:
		CSortLoginServerSession()
		{
			
		}
		bool operator()(const CDNLoginConnection* pLv, const CDNLoginConnection* pRv) const {
			if (pLv && pRv) {
				return(pLv->GetActive() && !pRv->GetActive());
			}
			return(pLv && !pRv);

			// Scheduled for deletion !!! 20100106
/*
			if (pLv && pRv) {
				if (pLv->GetActive() && pRv->GetActive()) {
					return(pLv->GetAvlConCount() > pRv->GetAvlConCount());
				}
				return(pLv->GetActive() && !pRv->GetActive());
			}
			return(pLv && !pRv);
*/
		}
	};

	// Functor for copying login server sessions (selects valid sessions only)
	class CRemoveCopyIfLoginServerSession
	{
	public:
		CRemoveCopyIfLoginServerSession()
		{
			
		}
		bool operator()(const CDNLoginConnection* pNode) const {
			if (pNode && pNode->GetActive()) {
				return true;
			}
			return false;
		}
	};

private:
	DWORD m_dwReconnectTick;
	int m_nServerID;
	int m_nCurConCount;
	int m_nMaxConCount;

public:
	CDNLoginConnection(void);
	virtual ~CDNLoginConnection(void);

	void Reconnect(DWORD CurTick);
	int MessageProcess(int iMainCmd, int iSubCmd, char * pData, int iLen);

	int OnRecvAdduser(LOMAAddUser *pPacket, bool bRecusive);

	void SendRegistWorldID();
	void SendVillageInfoDelete(int nVillageID);

#if defined(PRE_MOD_SELECT_CHAR)
	void SendAddUser(UINT nAccountDBID, UINT nSessionID, int nRet);
#else	// #if defined(PRE_MOD_SELECT_CHAR)
	void SendAddUser(UINT nAccountDBID, UINT nSessionID, char cWorldID, int nRet);
	void SendUserInfo(UINT nAccountDBID, UINT nSessionID, int nRet, const char * pszIP = NULL);
#endif	// #if defined(PRE_MOD_SELECT_CHAR)
	void SendDetachUser(UINT nAccountDBID);

	void SendSetTutorialGameID(UINT nSessionID, ULONG nIP, USHORT nPort, USHORT nTcpPort, int iServerIdx, USHORT wGameID, int nRoomID, int nRet);

	void SendWaitUserResult(UINT nAccountDBID, USHORT nTicketNum, short nRetCode);
	void SendWaitProcess(MALOWaitUserProcess * pProcess);

	int GetServerID() const { return m_nServerID; }
	int GetCurConCount() const { return m_nCurConCount; }
	int GetMaxConCount() const { return m_nMaxConCount; }
	int GetAvlConCount() const { return(m_nMaxConCount - m_nCurConCount); }
	void SendDuplicateLogin(UINT nAccountDBID, UINT nSessionID);
};
