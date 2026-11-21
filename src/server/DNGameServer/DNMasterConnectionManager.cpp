#include "stdafx.h"
#include "DNMasterConnectionManager.h"
#include "DNMasterConnection.h"
#include "DNUserSession.h"
#include "Util.h"
#include "Log.h"
#if defined( PRE_WORLDCOMBINE_PARTY )
#include "DNAuthManager.h"
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )

CDNMasterConnectionManager* g_pMasterConnectionManager = NULL;

CDNMasterConnectionManager::CDNMasterConnectionManager(void): CConnectionManager(), m_dwReconnectTick(0)
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
//m_bUseLock must not be changed after creation!
, m_bUseLock(g_Config.nMasterCount > 1? true : false)
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
{
	m_WorldIDConnections.clear();
}

CDNMasterConnectionManager::~CDNMasterConnectionManager(void)
{
	m_WorldIDConnections.clear();
}

void CDNMasterConnectionManager::Reconnect(DWORD CurTick)
{
#if defined( PRE_WORLDCOMBINE_PARTY )
	if( g_pAuthManager->IsResetAuthServer() == false )
		return;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	if (m_dwReconnectTick + 5000 < CurTick)
	{
		{
			ScopeLock<CSyncLock> Lock( m_ConSync );

			if (m_Connections.empty()) 
				return;

			for (int i = 0; i < (int)m_Connections.size(); i++)
			{
				CDNMasterConnection* pCon = (CDNMasterConnection*)m_Connections[i];
				if (pCon) 
					pCon->Reconnect();
			}
		}

		m_dwReconnectTick = CurTick;
	}
}

CConnection* CDNMasterConnectionManager::AddConnection(const char *pIp, const USHORT nPort)
{
	g_Log.Log( LogType::_NORMAL, L"CDNMasterConnectionManager::AddConnection (%S:%d)\r\n", pIp, nPort);

	CDNMasterConnection *pCon = new CDNMasterConnection;
	if (!pCon) return NULL;

	pCon->SetSessionID(g_IDGenerator.GetMasterConnectionID());
	pCon->SetIp(pIp);
	pCon->SetPort(nPort);

	PushConnection(pCon);

	return (CConnection*)pCon;
}

bool CDNMasterConnectionManager::DelConnection(CConnection *pCon)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	//Nowhere calls this, and in this case the connection container does not change while the game server is running, except at startup...
	return false;
#else
	if (!CConnectionManager::DelConnection(pCon)) return false;

	ScopeLock<CSyncLock> Lock( m_MasterLock );

	if (m_WorldIDConnections.empty()) 
		return false;

	CDNMasterConnection *pMasterCon = (CDNMasterConnection*)pCon;
	TMapConnections::iterator iter = m_WorldIDConnections.find(pMasterCon->m_cWorldSetID);
	if (iter != m_WorldIDConnections.end()){
		m_WorldIDConnections.erase(iter);
	}

	return true;
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
}

void CDNMasterConnectionManager::GetConnectedWorldID(std::vector <int> * pvList)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	TMapConnections::iterator ii;
	for (ii = m_WorldIDConnections.begin(); ii != m_WorldIDConnections.end(); ii++)
		pvList->push_back((*ii).first);
}

CDNMasterConnection* CDNMasterConnectionManager::AddWorldIDConnection(UINT nConID, char cWorldSetID)
{
	CDNMasterConnection *pMasterCon = (CDNMasterConnection*)GetConnection(nConID);
	if (!pMasterCon) return NULL;

#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	if (GetMasterConnectionByWorldID(cWorldSetID)){	// If it already exists...
		g_Log.Log( LogType::_ERROR, cWorldSetID, 0, 0, 0, L"[SID:%u] Duplicate MasterConnection (Count:%d)\r\n", cWorldSetID, GetCount());
		return NULL;
	}

	pMasterCon->m_cWorldSetID = cWorldSetID;
	m_WorldIDConnections.insert(make_pair(pMasterCon->m_cWorldSetID, pMasterCon));

	g_Log.Log( LogType::_NORMAL, pMasterCon->m_cWorldSetID, 0, 0, pMasterCon->GetSessionID(), L"[SID:%u] CDNMasterConnectionManager::AddWorldIDConnection (MasterID:%u, Count:%d)\r\n", pMasterCon->m_cWorldSetID, pMasterCon->GetSessionID(), GetCount());

	return pMasterCon;
}

int CDNMasterConnectionManager::GetWorldSetIDs(BYTE * pWorldID)
{
	int nCount = 0;
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = NULL;
	TMapConnections::iterator ii;
	for (ii = m_WorldIDConnections.begin(); ii != m_WorldIDConnections.end(); ii++)
		pWorldID[nCount++] = (*ii).first;
	return nCount;
}

bool CDNMasterConnectionManager::IsAllWorldMasterConnected()
{
	if (IsAllMasterConnected())
	{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
		ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
		ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
		if (m_WorldIDConnections.size() == m_Connections.size())
			return true;
	}
	return false;
}

bool CDNMasterConnectionManager::IsAllMasterConnected()
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = NULL;
	for (int i = 0; i < (int)m_Connections.size(); i++)
	{
		pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon->GetActive() == false)
			return false;
	}
	return true;
}

bool CDNMasterConnectionManager::IsMasterConnected(char cWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = NULL;
	for (int i = 0; i < (int)m_Connections.size(); i++)
	{
		pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon->m_cWorldSetID == cWorldID && pMasterCon->GetActive())
			return true;
	}
	return false;
}

void CDNMasterConnectionManager::SendAddUserList(char cWorldSetID, CDNUserSession *pSession)
{
	if (m_WorldIDConnections.empty()) return ;
	if (pSession->GetGameRoom() == NULL) return;

#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = NULL;
	TMapConnections::iterator iter = m_WorldIDConnections.find(cWorldSetID);
	if (iter != m_WorldIDConnections.end())
		pMasterCon = (CDNMasterConnection*)iter->second;
	if (!pMasterCon) return;
	pMasterCon->SendAddUserList(pSession);
}

void CDNMasterConnectionManager::SendEndofVillageInfo(char cWorldSetID, UINT nServerID)
{
	if (m_WorldIDConnections.empty()) return ;

#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = NULL;
	TMapConnections::iterator iter = m_WorldIDConnections.find(cWorldSetID);
	if (iter != m_WorldIDConnections.end())
		pMasterCon = (CDNMasterConnection*)iter->second;
	if (!pMasterCon) return;
	pMasterCon->SendEndofVillageInfo(nServerID);
}

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNMasterConnectionManager::SendConnectComplete( BYTE cWorldSetID )
#else
void CDNMasterConnectionManager::SendConnectComplete()
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
{
	if (m_WorldIDConnections.empty()) return ;

#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

#if defined( PRE_WORLDCOMBINE_PARTY )
	if (cWorldSetID == 0)
	{
		CDNMasterConnection *pMasterCon = NULL;
		for (int i = 0; i < (int)m_Connections.size(); i++)
		{
			pMasterCon = (CDNMasterConnection*)m_Connections[i];
			if (pMasterCon->GetActive())
				pMasterCon->SendConnectComplete();
		}
	}
	else
	{
		CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
		if (!pMasterCon) return;
		if (pMasterCon->GetActive())
			pMasterCon->SendConnectComplete();
	}
#else
	CDNMasterConnection *pMasterCon = NULL;
	for (int i = 0; i < (int)m_Connections.size(); i++)
	{
		pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon->GetActive())
			pMasterCon->SendConnectComplete();
	}
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
}

void CDNMasterConnectionManager::SendDelUser(char cWorldSetID, UINT nAccountDBID, bool bIsIntended, UINT nSessionID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendDelUser(nAccountDBID, bIsIntended, nSessionID);
}

void CDNMasterConnectionManager::SendCheckReconnectLogin(char cWorldSetID, UINT nAccountDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendReconnectLogin(nAccountDBID);
}

bool CDNMasterConnectionManager::SendEnterGame(char cWorldSetID, UINT nAccountDBID, UINT nRoomID, UINT nServerID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;
	pMasterCon->SendEnterGame(nAccountDBID, nRoomID, nServerID);
	return true;
}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
bool CDNMasterConnectionManager::SendSetRoomID( char cWorldSetID, GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int nRoomID, BYTE cZoneID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto, ePartyType Type, WorldPvPMissionRoom::Common::eReqType eWorldReqType )
#else // #if defined( PRE_WORLDCOMBINE_PVP )
bool CDNMasterConnectionManager::SendSetRoomID( char cWorldSetID, GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int nRoomID, BYTE cZoneID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto, ePartyType Type )
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
bool CDNMasterConnectionManager::SendSetRoomID( char cWorldSetID, GameTaskType::eType GameTaskType, BYTE cReqGameIDType, TINSTANCEID InstanceID, int iServerIdx, int nRoomID, BYTE cZoneID, UINT * pPartyMember, TPARTYID PartyIDForBreakInto )
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;

	if (pMasterCon->GetActive())
#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
		return (pMasterCon->SendSetRoomID( GameTaskType, cReqGameIDType, InstanceID, iServerIdx, nRoomID, cZoneID, pPartyMember, PartyIDForBreakInto, Type, eWorldReqType)==0);
#else // #if defined( PRE_WORLDCOMBINE_PVP )
		return (pMasterCon->SendSetRoomID( GameTaskType, cReqGameIDType, InstanceID, iServerIdx, nRoomID, cZoneID, pPartyMember, PartyIDForBreakInto, Type)==0);
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else // #if defined( PRE_WORLDCOMBINE_PARTY )
		return (pMasterCon->SendSetRoomID( GameTaskType, cReqGameIDType, InstanceID, iServerIdx, nRoomID, cZoneID, pPartyMember, PartyIDForBreakInto)==0);
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	return false;
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_ConSync );

	if (m_Connections.empty())
	{
		_DANGER_POINT();
		return false;
	}

	CDNMasterConnection *pMasterCon = NULL;
	for (int i = 0; i < (int)m_Connections.size(); i++)
	{
		pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon->m_cWorldSetID == cWorldSetID && pMasterCon->GetActive())
		{
			int nRet = pMasterCon->SendSetRoomID( GameTaskType, cReqGameIDType, InstanceID, iServerIdx, nRoomID, cZoneID, pPartyMember, PartyIDForBreakInto);
			return (nRet==0);
		}
	}
	return false;
#endif
}

void CDNMasterConnectionManager::SendSetTutorialRoomID(char cWorldSetID, UINT nAccountDBID, int iServerIdx, int nRoomID, int nLoginServerID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	if (pMasterCon->GetActive())
		pMasterCon->SendSetTutorialRoomID(nAccountDBID, iServerIdx, nRoomID, nLoginServerID);
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_ConSync );

	if (m_Connections.empty())	//Called because master connection is needed.... if missing.... huge trouble....... just kick them out
	{
		_DANGER_POINT();
		return;
	}

	CDNMasterConnection *pMasterCon = NULL;
	for (int i = 0; i < (int)m_Connections.size(); i++){
		pMasterCon = (CDNMasterConnection*)m_Connections[i];
		if (pMasterCon->m_cWorldSetID == cWorldSetID && pMasterCon->GetActive())
		{
			pMasterCon->SendSetTutorialRoomID(nAccountDBID, iServerIdx, nRoomID, nLoginServerID);
			return;
		}
	}
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
}

void CDNMasterConnectionManager::SendPvPCommand( const char cWorldSetID, const UINT uiCommand, const UINT uiPvPIndex, const UINT uiRoomIndex )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection* pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if( !pMasterCon ) 
	{
		_DANGER_POINT();
		return;
	}

	pMasterCon->SendPvPCommand( uiCommand, uiPvPIndex, uiRoomIndex );
}

bool CDNMasterConnectionManager::SendRequestNextVillageInfo(char cWorldSetID, int nMapIndex, int nEnteredGateIndex, bool bReturnVillage, CDNGameRoom * pRoom)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;

	pMasterCon->SendRequestNextVillageInfo(nMapIndex, nEnteredGateIndex, bReturnVillage, pRoom);
	return true;
}

bool CDNMasterConnectionManager::SendRequestNextVillageInfo( char cWorldSetID, UINT uiAccountDBID, int nMapIndex, int nGateIndex, bool bRetrunVillage, INT64 nItemSerial )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;
	pMasterCon->SendRequestNextVillageInfo(uiAccountDBID, nMapIndex, nGateIndex, bRetrunVillage, nItemSerial);

	return true;
}

bool CDNMasterConnectionManager::SendRebirthVillageInfo(char cWorldSetID, UINT nAccountDBID, int nLastVillageMapIdx)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;

	return pMasterCon->SendRebirthVillageInfo(nAccountDBID, nLastVillageMapIdx);
}

void CDNMasterConnectionManager::SendPvPGameToPvPLobby( const char cWorldSetID, const UINT uiAccountDBID, const UINT uiPvPIndex, const int iLastVillageMapIndex, bool bIsLadderRoom )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection* pMasterCon = GetMasterConnectionByWorldID( cWorldSetID );
	if( pMasterCon )
	{
		pMasterCon->SendPvPGameToPvPLobby( uiAccountDBID, uiPvPIndex, iLastVillageMapIndex, bIsLadderRoom );
	}
	else
		_DANGER_POINT();
}

//void CDNMasterConnectionManager::SendLocalSymbolData(char cWorldSetID, UINT nAccountDBID, TSymbolItem *SymbolDataArray)
//{
//	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
//	if (!pMasterCon) return;
//
//	pMasterCon->SendLocalSymbolData(nAccountDBID, SymbolDataArray);
//}

void CDNMasterConnectionManager::SendLoginState(char cWorldSetID, UINT nAccountDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendLoginState(nAccountDBID);
}

void CDNMasterConnectionManager::SendVillageState(char cWorldSetID, UINT nAccountDBID, int nMapIndex, int nRoomID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendVillageState(nAccountDBID, nMapIndex, nRoomID);
}

void CDNMasterConnectionManager::SendNoticeFromClinet(const WCHAR * pMsg, const int nLen)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	TMapConnections::iterator ii;
	CDNMasterConnection * pMasterCon = NULL;
	for (ii = m_WorldIDConnections.begin(); ii != m_WorldIDConnections.end(); ii++)
	{
		pMasterCon = (CDNMasterConnection*)(*ii).second;
		pMasterCon->SendNoticeFromClient(pMsg, nLen);
	}
}

void CDNMasterConnectionManager::SendWorldSystemMsg(char cWorldSetID, UINT nFromAccountDBID, char cType, int nID, int nValue, WCHAR* pwszToCharacterName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendWorldSystemMsg(nFromAccountDBID, cType, nID, nValue, pwszToCharacterName);
}

void CDNMasterConnectionManager::SendGameFrame(int nServerIdx, int nWholeRoomCnt, int nRoomCnt, int nUserCnt, int nMinFrame, int nMaxFrame, int nAvrFrame, bool bHasMargin)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	if (m_WorldIDConnections.empty()) return;

	TMapConnections::iterator ii;
	CDNMasterConnection *pMasterCon = NULL;
	for (ii = m_WorldIDConnections.begin(); ii != m_WorldIDConnections.end(); ii++){
		pMasterCon = (CDNMasterConnection*)(*ii).second;
		pMasterCon->SendGameFrame(nServerIdx, nWholeRoomCnt, nRoomCnt, nUserCnt, nMinFrame, nMaxFrame, nAvrFrame, bHasMargin);
	}
}

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNMasterConnectionManager::SendWorldUserState(char cWorldSetID, WCHAR* pwszCharacterName , int nMapIdx)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	
	if( pwszCharacterName )
		pMasterCon->SendUpdateWorldUserState(pwszCharacterName, nMapIdx);
}
#endif
void CDNMasterConnectionManager::SendWorldUserState(char cWorldSetID, DNVector(std::wstring) & vName, int nMapIdx)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	DNVector(std::wstring)::iterator ii;
	for (ii = vName.begin(); ii != vName.end(); ii++)
		pMasterCon->SendUpdateWorldUserState((*ii).c_str(), nMapIdx);
}

void CDNMasterConnectionManager::SendBanUser(UINT nAccountDBID, char cWorldSetID, const WCHAR * pCharacterName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendBanUser(nAccountDBID, pCharacterName);
}

bool CDNMasterConnectionManager::SendChat(char cWorldSetID, char cType, UINT nFromAccountDBID, const WCHAR *pwszChatMsg, short wChatLen)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;
	pMasterCon->SendChat(cType, nFromAccountDBID, pwszChatMsg, wChatLen);
	return true;
}

#ifdef PRE_ADD_DOORS
bool CDNMasterConnectionManager::SendPrivateChat(char cWorldSetID, UINT nFromAccountDBID, WCHAR *pwszToCharacterName, char cType, WCHAR *pwszChatMsg, short wChatLen, INT64 biDestCharacterDBID)
#else		//#ifdef PRE_ADD_DOORS
bool CDNMasterConnectionManager::SendPrivateChat(char cWorldSetID, UINT nFromAccountDBID, WCHAR *pwszToCharacterName, char cType, WCHAR *pwszChatMsg, short wChatLen)
#endif		//#ifdef PRE_ADD_DOORS
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;
#ifdef PRE_ADD_DOORS
	pMasterCon->SendPrivateChat(nFromAccountDBID, pwszToCharacterName, cType, pwszChatMsg, wChatLen, biDestCharacterDBID);
#else		//#ifdef PRE_ADD_DOORS
	pMasterCon->SendPrivateChat(nFromAccountDBID, pwszToCharacterName, cType, pwszChatMsg, wChatLen);
#endif		//#ifdef PRE_ADD_DOORS
	return true;
}

#if defined( PRE_PRIVATECHAT_CHANNEL )					
bool CDNMasterConnectionManager::SendPrivateChannelChat(char cWorldSetID, char cType, UINT nFromAccountDBID, const WCHAR *pwszChatMsg, short wChatLen, INT64 nChannelID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;
	pMasterCon->SendPrivateChannelChat(cType, nFromAccountDBID, pwszChatMsg, wChatLen, nChannelID);
	return true;
}
#endif

void CDNMasterConnectionManager::SendFriendAddNotify(char cWorldSetID, UINT nAccountDBID, const WCHAR * pCharacterName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendFriendAddNotify(nAccountDBID, pCharacterName);
}

void CDNMasterConnectionManager::SendPvPRoomSyncOK(char cWorldSetID, UINT nPvPIndex)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPRoomSyncOK(nPvPIndex);
}

void CDNMasterConnectionManager::SendPvPLadderRoomSync( char cWorldSetID, INT64 biRoomIndex, INT64 biRoomIndex2, LadderSystem::RoomState::eCode State )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPLadderRoomSync( biRoomIndex, biRoomIndex2, State );
}

void CDNMasterConnectionManager::SendPvPBreakIntoOK(char cWorldSetID, UINT nPvPIndex, UINT nAccountDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPBreakIntoOK(nPvPIndex, nAccountDBID);
}

void CDNMasterConnectionManager::SendPvPChangeMemberGrade(char cWorldSetID, UINT nAccountDBID, USHORT nType, UINT nSessionID, bool bAsign)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPChangeMemberGrade(nAccountDBID, nType, nSessionID, bAsign);
}

void CDNMasterConnectionManager::SendPvPSwapMemberIndex(char cWorldSetID, UINT nAccountDBID, BYTE cCount, const TSwapMemberIndex * pIndex)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPSwapMemberIndex(nAccountDBID, cCount, pIndex);
}

void CDNMasterConnectionManager::SendPvPRealTimeScore(char cWorldSetID, UINT nGuildDBID, int nScore)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPRealTimeScore(nGuildDBID, nScore);
}

void CDNMasterConnectionManager::SendPvPGuildWarResult(char cWorldSetID, UINT nWinGuildDBID, UINT nLoseGuildDBID, bool bIsDraw)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPGuildWarResult(nWinGuildDBID, nLoseGuildDBID, bIsDraw);
}

void CDNMasterConnectionManager::SendPvPDetectCrash(char cWorldSetID, UINT nRoomID, UINT nPvPIndex)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendPvPDetectCrash(nRoomID, nPvPIndex);
}

//Guild
void CDNMasterConnectionManager::SendChangeGuildInfo(char cWorldSetID, const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, BYTE btGuildUpdate, int iInt1, int iInt2, int iInt3, int iInt4, INT64 biInt64, LPCWSTR pText)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SendChangeGuildInfo(pGuildUID, nAccountDBID, nCharacterDBID, btGuildUpdate, iInt1, iInt2, iInt3, iInt4, biInt64, pText);
}

void CDNMasterConnectionManager::SendChangeGuildMemberInfo(char cWorldSetID, const TGuildUID pGuildUID, UINT nReqAccountDBID, INT64 nReqCharacterDBID, UINT nChgAccountDBID, INT64 nChgCharacterDBID, BYTE btGuildMemberUpdate, int iInt1, int iInt2, INT64 biInt64, LPCWSTR pText, bool bReturn)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	if(bReturn)
		pMasterCon->SendChangeGuildMemberInfo(pGuildUID, nReqAccountDBID, nReqCharacterDBID, nChgAccountDBID, nChgCharacterDBID, btGuildMemberUpdate, iInt1, iInt2, biInt64, pText, bReturn);
	else
		pMasterCon->SendChangeGuildMemberInfo(pGuildUID, nReqAccountDBID, nReqCharacterDBID, nChgAccountDBID, nChgCharacterDBID, btGuildMemberUpdate, iInt1, iInt2, biInt64, pText);
}

void CDNMasterConnectionManager::SendGuildChat(char cWorldSetID, const TGuildUID pGuildUID, UINT nAccountDBID, INT64 nCharacterDBID, LPCWSTR lpwszChatMsg, short nLen)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SendGuildChat(pGuildUID, nAccountDBID, nCharacterDBID, lpwszChatMsg, nLen);
}

CDNMasterConnection* CDNMasterConnectionManager::GetMasterConnectionByWorldID(char cWorldSetID)
{
	if (m_WorldIDConnections.empty()) 
		return NULL;

	CDNMasterConnection *pMasterCon = NULL;

	TMapConnections::iterator iter = m_WorldIDConnections.find(cWorldSetID);
	if (iter != m_WorldIDConnections.end()){
		if (((CDNMasterConnection*)iter->second)->GetActive())
			pMasterCon = (CDNMasterConnection*)iter->second;
	}

	return pMasterCon;
}

void CDNMasterConnectionManager::SendGuildChangeName(char cWorldSetID, const TGuildUID pGuildUID, LPCWSTR lpwszGuildName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SendGuildChangeName(pGuildUID, lpwszGuildName);
}

void CDNMasterConnectionManager::SendUpdateGuildExp(const TGuildUID pGuildUID, const TAModGuildExp* pPacket)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(pPacket->cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendUpdateGuildExp(pGuildUID, pPacket);
}

void CDNMasterConnectionManager::SetPreWinGuildUID( char cWorldSetID, TGuildUID GuildUID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SetPreWinGuildUID( GuildUID );
}

void CDNMasterConnectionManager::SetGuildWarFinalStartTimeForCheat( char cWorldSetID, __time64_t _tTime )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SetGuildWarFinalStartTimeForCheat( _tTime );
}

void CDNMasterConnectionManager::SendAddGuildWarPoint(char cWorldSetID, char cTeamType, TGuildUID GuildUID, int nAddPoint)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return;
	}

	pMasterCon->SendAddGuildWarPoint(cTeamType, GuildUID, nAddPoint);
}

TGuildUID CDNMasterConnectionManager::GetPreWinGuildUID( char cWorldSetID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return TGuildUID();
	}

	return pMasterCon->GetPreWinGuildUID();
}

__time64_t CDNMasterConnectionManager::GetGuildWarFinalStatTime( char cWorldSetID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) {
		return -1;
	}

	return pMasterCon->GetGuildWarFinalStatTime();
}

bool CDNMasterConnectionManager::SendInvitePartyMember(CDNGameRoom * pGameRoom, char cWorldSetID, int nMapIdx, int nChannelID, UINT nAccountDBID, const WCHAR * pwszInviterName, const WCHAR * pwszInvitedName, int * pPassClassIds, int nPermitLevel)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return false;

	return pMasterCon->SendInvitePartyMember(pGameRoom, nMapIdx, nChannelID, nAccountDBID, pwszInviterName, pwszInvitedName, pPassClassIds, nPermitLevel);
}

void CDNMasterConnectionManager::SendInvitedMemberReturnMsg(char cWorldSetID, UINT nAccountDBID, int nRetCode)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendInvitedMemberReturnMsg(nAccountDBID, nRetCode);
}

#if defined(PRE_MOD_REQ_JOIN_PARTY_ANSWER_MSG_APP)

void CDNMasterConnectionManager::SendGetPartyIDResult(char cWorldSetID, UINT nSenderAccountDBID , TPARTYID PartyID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendGetPartyIDResult(nSenderAccountDBID, PartyID);
}

void CDNMasterConnectionManager::SendResPartyAskJoin(char cWorldSetID, int iRet, UINT uiReqAccountDBID, const WCHAR* pwszReqChracterName )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendResPartyAskJoin(iRet, uiReqAccountDBID, pwszReqChracterName);
}

void CDNMasterConnectionManager::SendAskJoinAgreeInfo(char cWorldSetID, const WCHAR* pwszAskerCharName, TPARTYID PartyID, int iPassword )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	pMasterCon->SendAskJoinAgreeInfo(pwszAskerCharName, PartyID, iPassword);
}
#endif


#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
void CDNMasterConnectionManager::SendSyncSystemMail( TASendSystemMail* pA )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(pA->cWorldSetID);
	if(!pMasterCon) return;

	return pMasterCon->SendSyncSystemMail( pA );
}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void CDNMasterConnectionManager::SendMasterSystemSyncSimpleInfo( BYTE cWorldSetID, INT64 biCharacterDBID, MasterSystem::EventType::eCode Type )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID( cWorldSetID );
	if(!pMasterCon) return;

	return pMasterCon->SendMasterSystemSyncSimpleInfo( biCharacterDBID, Type );
}

void CDNMasterConnectionManager::SendMasterSystemSyncGraduate( BYTE cWorldSetID, INT64 biCharacterDBID, WCHAR* pwszCharName )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID( cWorldSetID);
	if(!pMasterCon) return;

	return pMasterCon->SendMasterSystemSyncGraduate( biCharacterDBID, pwszCharName );
}

void CDNMasterConnectionManager::SendMasterSystemSyncConnect( BYTE cWorldSetID, bool bIsConnect, WCHAR* pwszCharName, const TMasterSystemData* pMasterSystemData )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID( cWorldSetID);
	if(!pMasterCon) return;

	return pMasterCon->SendMasterSystemSyncConnect( bIsConnect, pwszCharName, pMasterSystemData );
}

bool CDNMasterConnectionManager::SendFarmDataLoaded(int nWorldID, int nGameServerIdx, UINT nRoomID, UINT nFarmDBID, int nMapID, int nFarmMaxUser)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if(!pMasterCon) return false;

	return pMasterCon->SendFarmDataLoaded(nGameServerIdx, nRoomID, nFarmDBID, nMapID, nFarmMaxUser);
}

bool CDNMasterConnectionManager::SendFarmUserCount(int nWorldID, UINT nFarmDBID, UINT nFarmRoomID, int nCurUserCount, bool bStarted)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if(!pMasterCon) return false;

	return pMasterCon->SendFarmUserCount(nFarmDBID, nFarmRoomID, nCurUserCount, bStarted);
}

bool CDNMasterConnectionManager::SendFarmIntendedDestroy(int nWorldID, UINT nFarmDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if(!pMasterCon) return false;

	return pMasterCon->SendFarmIntendedDestroy(nFarmDBID);
}

void CDNMasterConnectionManager::SendFarmSync( int nWorldID, INT64 biCharacterDBID, Farm::ServerSyncType::eType Type )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if(!pMasterCon) return;

	pMasterCon->SendFarmSync( biCharacterDBID, Type );
}

void CDNMasterConnectionManager::SendFarmSyncAddWater( int nWorldID, INT64 biCharacterDBID, WCHAR* pwszCharName, int iAddWaterPoint )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if(!pMasterCon) return;

	pMasterCon->SendFarmSyncAddWater( biCharacterDBID, pwszCharName, iAddWaterPoint );
}

void CDNMasterConnectionManager::SendGuildMemberLevelUp(int nWorldID, TGuildUID GuildUID, INT64 nCharacterDBID, char cLevel)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return;
	pMasterCon->SendGuildMemberLevelUp(GuildUID, nCharacterDBID, cLevel);
}

void CDNMasterConnectionManager::SendLoadUserTempData(int nWorldID, UINT uiAccountDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return;
	pMasterCon->SendLoadUserTempData(uiAccountDBID);
}

void CDNMasterConnectionManager::SendSaveUserTempData(int nWorldID, CDNUserSession* pSession)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return;
	pMasterCon->SendSaveUserTempData(pSession);
}

char CDNMasterConnectionManager::GetGuildWarStepIndex(int nWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return 0;
	return pMasterCon->GetGuildWarStepIndex();
}
short CDNMasterConnectionManager::GetGuildWarScheduleID(int nWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return 0;
	return pMasterCon->GetGuildWarScheduleID();
}
int CDNMasterConnectionManager::GetBlueTeamPoint(int nWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return 0;
	return pMasterCon->GetBlueTeamPoint();
}
int CDNMasterConnectionManager::GetRedTeamPoint(int nWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return 0;
	return pMasterCon->GetRedTeamPoint();
}

bool CDNMasterConnectionManager::IsSecretMission(int nWorldID, BYTE cTeamCode, int nMissionID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return false;
	return pMasterCon->IsSecretMission(cTeamCode, nMissionID);
}

void CDNMasterConnectionManager::GetGuildWarSeqInfo(char cWorldSetID, short& nScheduleID, char &cStepIndex, char &cFinalPart)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;

	cStepIndex = pMasterCon->GetGuildWarStepIndex();
	nScheduleID = pMasterCon->GetGuildWarScheduleID();
	cFinalPart = pMasterCon->GetGuildWarFinalPart();
}

char CDNMasterConnectionManager::GetGuildWarFinalPart(int nWorldID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(nWorldID);
	if (!pMasterCon) return 0;
	return pMasterCon->GetGuildWarFinalPart();
}

void CDNMasterConnectionManager::SendDuplicateLogin(char cWorldSetID, UINT nAccountDBID, bool bIsDetach, UINT nSessionID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendDuplicateLogin(nAccountDBID, bIsDetach, nSessionID);
}

void CDNMasterConnectionManager::SendCheckLastDungeonInfo( char cWorldSetID, UINT uiAccountDBID, INT64 biCharacterDBID, bool bIsCheck, const WCHAR* pwszPartyName/*=NULL*/ )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendCheckLastDungeonInfo( uiAccountDBID, biCharacterDBID, bIsCheck, pwszPartyName );
}

#if defined( PRE_PARTY_DB )
void CDNMasterConnectionManager::SendConfirmLastDungeonInfo( char cWorldSetID, int iRet, UINT uiAccountDBID, BreakInto::Type::eCode BreakIntoType )
#else
void CDNMasterConnectionManager::SendConfirmLastDungeonInfo( char cWorldSetID, int iRet, UINT uiAccountDBID )
#endif // #if defined( PRE_PARTY_DB )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
#if defined( PRE_PARTY_DB )
	pMasterCon->SendConfirmLastDungeonInfo( iRet, uiAccountDBID, BreakIntoType );
#else
	pMasterCon->SendConfirmLastDungeonInfo( iRet, uiAccountDBID );
#endif // #if defined( PRE_PARTY_DB )
}

#if defined( PRE_WORLDCOMBINE_PARTY )

void CDNMasterConnectionManager::SendDelWorldParty( char cWorldSetID, int nPrimaryIndex )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendDelWorldParty( nPrimaryIndex );
}
void CDNMasterConnectionManager::SendWorldPartyMember( char cWorldSetID, UINT nAccountDBID, INT64 biCharacterDBID, TPARTYID PartyID, short nCount, Party::MemberData *MemberData, int nRet )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendWorldPartyMember( nAccountDBID, biCharacterDBID, PartyID, nCount, MemberData, nRet );
}

#endif

#if defined( PRE_ADD_BESTFRIEND )
void CDNMasterConnectionManager::SendLevelUpBestFriend(char cWorldSetID, BYTE cLevel, LPCWSTR lpwszCharacterName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendLevelUpBestFriend( cLevel, lpwszCharacterName );
}
#endif

#if defined( PRE_PRIVATECHAT_CHANNEL )
void CDNMasterConnectionManager::SendAddPrivateChatChannel(char cWorldSetID,TPrivateChatChannelInfo tPrivateChatChannel)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendAddPrivateChatChannel( tPrivateChatChannel );
}

void CDNMasterConnectionManager::SendAddPrivateChatChannelMember(char cWorldSetID,INT64 nChannelID, TPrivateChatChannelMember Member)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendAddPrivateChatChannelMember( nChannelID, Member );
}

void CDNMasterConnectionManager::SendInvitePrivateChatChannel(char cWorldSetID, INT64 nChannelID, UINT nAccountDBID, WCHAR* wszInviteCharacterName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendInvitePrivateChatChannel( nChannelID, nAccountDBID, wszInviteCharacterName );
}

void CDNMasterConnectionManager::SendInvitePrivateChatChannelResult(char cWorldSetID, int nRet, UINT nAccountDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendInvitePrivateChatChannelResult( nRet, nAccountDBID );
}

void CDNMasterConnectionManager::SendDelPrivateChatChannelMember(char cWorldSetID,PrivateChatChannel::Common::eModType eType, INT64 nChannelID, INT64 biCharacterDBID, const WCHAR* wszKickName)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendDelPrivateChatChannelMember( eType, nChannelID, biCharacterDBID, wszKickName );
}

void CDNMasterConnectionManager::SendKickPrivateChatChannelMemberResult(char cWorldSetID, INT64 biCharacterDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendKickPrivateChatChannelMemberResult( biCharacterDBID );
}

void CDNMasterConnectionManager::SendModPrivateChatChannelInfo(char cWorldSetID,INT64 nChannelID, PrivateChatChannel::Common::eModType eModType, int nPassWord, INT64 biCharacterDBID)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendModPrivateChatChannelInfo( nChannelID, eModType, nPassWord, biCharacterDBID );
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )

void CDNMasterConnectionManager::SendDelWorldPvPRoom( char cWorldSetID, int nRoomIndex )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendDelWorldPvPRoom( cWorldSetID, nRoomIndex );
}

void CDNMasterConnectionManager::SendWorldPvPRoomJoinResult( char cWorldSetID, int nRet, UINT uiAccountDBID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendWorldPvPRoomJoinResult( cWorldSetID, nRet, uiAccountDBID );
}

void CDNMasterConnectionManager::SendWorldPvPRoomGMCreateResult( char cWorldSetID, int nRet, UINT uiAccountDBID, TWorldPvPRoomDBData* Data, UINT uiPvPIndex )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendWorldPvPRoomGMCreateResult( cWorldSetID, nRet, uiAccountDBID, Data, uiPvPIndex );
}

#endif

#if defined( PRE_ADD_MUTE_USERCHATTING)
void CDNMasterConnectionManager::SendMuteUserFind(UINT uiAccountDBID, char cWorldSetID, const WCHAR * pMuteName, int nMuteTime )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendMuteUserFind(uiAccountDBID, pMuteName, nMuteTime);
}
#endif	// #if defined( PRE_ADD_MUTE_USERCHATTING)

#if defined( PRE_MOD_71820 )
void CDNMasterConnectionManager::SendNotifyMail(char cWorldSetID, UINT nToAccountDBID, INT64 biToCharacterDBID, short wTotalMailCount, short wNotReadMailCount, short w7DaysLeftCount, bool bNewMail)
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendNotifyMail( nToAccountDBID, biToCharacterDBID, wTotalMailCount, wNotReadMailCount, w7DaysLeftCount, bNewMail );
}
#endif // #if defined( PRE_MOD_71820 )

#if defined( PRE_ALTEIAWORLD_EXPLORE )

void CDNMasterConnectionManager::SendAddAlteiaWorldSendTicketResult( char cWorldSetID, int nRetCode, INT64 biSendCharacterDBID )
{
#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock, m_bUseLock );
#else		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER
	ScopeLock<CSyncLock> Lock( m_MasterLock );
#endif		//#ifdef PRE_MOD_GAMESERVER_EXCEPT_MASTERMANAGER

	CDNMasterConnection *pMasterCon = GetMasterConnectionByWorldID(cWorldSetID);
	if (!pMasterCon) return;
	pMasterCon->SendAddAlteiaWorldSendTicketResult( nRetCode, biSendCharacterDBID );
}

#endif

