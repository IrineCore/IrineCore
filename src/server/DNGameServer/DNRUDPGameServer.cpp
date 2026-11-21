#include "stdafx.h"
#include "DNGameServerManager.h"
#include "DNRUDPGameServer.h"
#include "DNGameRoom.h"
#include "DNPvPGameRoom.h"
#include "DNDLGameRoom.h"
#include "DNUserSession.h"
#include "Log.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "DNDBConnection.h"
#include "DNDBConnectionManager.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnItemTask.h"
#include "DnPartyTask.h"
#include "PerfCheck.h"
#include "DnScriptManager.h"
#include "Util.h"
#include "SpinBuffer.h"
#include "NoticeSystem.h"
#include "DNFarmUserSession.h"
#include "DNGuildSystem.h"
#include "DNGameDataManager.h"
#ifdef _USE_VOICECHAT
#include "DNVoiceChat.h"
#endif
#include "DNFarmGameRoom.h"
#include "SecondarySkill.h"
#include "SecondarySkillRepository.h"
#include "NpcReputationProcessor.h"
#if defined( PRE_THREAD_ROOMDESTROY )
#include "DNBackGroundLoader.h"
#endif // #if defined( PRE_THREAD_ROOMDESTROY )
#include "PvPGameMode.h"
#if defined( PRE_PRIVATECHAT_CHANNEL )
#include "DnPrivateChatManager.h"
#include "DNPrivateChatChannel.h"
#endif
#include "DNMissionSystem.h"

extern TGameConfig g_Config;
const int s_nServerSecureFrame = 20;

CDNRUDPGameServer::CDNRUDPGameServer(CDNGameServerManager * pManager, const char * pExternalProbeIP, unsigned short nExternalProbePort)
{
	m_iGameServerID = 0;
	m_iUDPAddr = 0;
	m_iUDPPort = 0;
	m_pServerManager = pManager;

	char namelist[256];
	int l = gethostname(namelist, sizeof(namelist));
	struct hostent * hh = gethostbyname(namelist);
	m_iUDPAddr = *((unsigned long*)hh->h_addr_list[0]);

#ifdef _USE_NATPROBE
	_strcpy(m_szExternalProbeIP, _countof(m_szExternalProbeIP), pExternalProbeIP, (int)strlen(pExternalProbeIP));
	m_nExternalProbePort = nExternalProbePort;
#else
	memset(m_szExternalProbeIP, 0, sizeof(m_szExternalProbeIP));
	m_nExternalProbePort = 0;
#endif

	m_dwChangeSaveTick = m_dwLastSaveTick = 0;

	m_pStoredPacketBuffer = new (std::nothrow) CRawSpinBuffer( 1, 1024*1024*10 );
	m_pChangeSaveBuffer = new CBuffer(1024 * 1024 * 10);
	m_pLastSaveBuffer = new CBuffer(1024 * 1024 * 10);

	m_bIsAcceptRoom = true;
	m_bIsCloseService = false;
	m_bIsAllFlushSaveData = false;

	m_dwPreviewTime = timeGetTime();
	m_nVecFPSList.reserve( 20 * 120 ); // Save profile for 2 minutes

	ResetTimer();

	m_bFrameAlert		= false;
	m_uiFrameAlertCount	= 0;
	m_bZeroPopulation = false;
	m_uiConnectReqCount		= 0;
	m_uiConnectSuccessCount	= 0;
	m_nFrameReportPivot = 0;
#ifdef PRE_ADD_FRAMEDELAY_LOG
	m_nFrameReportLogPivot = 0;
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
}

CDNRUDPGameServer::~CDNRUDPGameServer()
{
	// 2009.01.20 kimbap
	// Reset pointer after normal thread termination
	Close();

	// 2009.02.24 kimbap
	// Added memory deallocation
	delete m_pStoredPacketBuffer;
	delete m_pChangeSaveBuffer;
	delete m_pLastSaveBuffer;
	m_pServerManager = NULL;
}

bool CDNRUDPGameServer::Start(int iPort, int iServerID, int nAffinity)
{
	m_iGameServerID = iServerID;
	if (Open(iServerID, nAffinity, iPort, false, false) == false)
	{
		_tprintf(_T("GameServer Port Open Failed [ID:%d][Port:%d]\n"), iServerID, iPort);
		return false;
	}

	SOCKADDR_IN addr;
	//Send probes for about 4 seconds to discover internal/external port and address.
	if (m_nExternalProbePort > 0)
	{
		for(int i=0; m_iUDPPort == 0 && i < 4000; i+=4000/10)
		{
			addr.sin_family				= AF_INET;
			addr.sin_port				= ntohs(m_nExternalProbePort);
			addr.sin_addr.S_un.S_addr	= inet_addr(m_szExternalProbeIP);
			if (sendto(m_hSocket, "ping", 4, 0, (struct sockaddr *)&addr, sizeof(addr)) == -1)
				g_Log.Log(LogType::_ERROR, L"PROBE_SEND_ERR ProbePort[%d] ErrNum(%d)\n", m_nExternalProbePort, GetLastError());
			Sleep(4000/10);
		}

		if (m_iUDPPort == 0)
		{//If not, just acquire it
			int iLen = sizeof(addr);
			getsockname(m_hSocket, (struct sockaddr*)&addr, &iLen);
			m_iUDPPort = ntohs(addr.sin_port);
		}
	}
	else
	{
		GetHostAddr();
		m_iUDPAddr = inet_addr(GetPublicIP());
		m_iUDPPort = iPort;
	}
	return true;
}

void CDNRUDPGameServer::GetAddress(unsigned long * pAddr, unsigned short * pPort)
{
	*pAddr = m_iUDPAddr;
	*pPort = m_iUDPPort;
}

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNRUDPGameServer::StoreExternalBuffer(UINT nAccountDBID, int iMainCmd, int iSubCmd, const char * pPacket, int iSize, BYTE cType, USHORT nMasterID, int iWorldSetID/*=0*/ )

#else
void CDNRUDPGameServer::StoreExternalBuffer(UINT nAccountDBID, int iMainCmd, int iSubCmd, const char * pPacket, int iSize, BYTE cType, USHORT nMasterID)
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
{
	_STORE_BUFFER_HEADER StorePacketHeader;
	StorePacketHeader.iBufSize		= static_cast<unsigned short>(sizeof(StorePacketHeader)+ iSize);
	StorePacketHeader.iSize			= iSize;
	StorePacketHeader.nAccountDBID = nAccountDBID;
	StorePacketHeader.iMainCmd		= iMainCmd;
	StorePacketHeader.iSubCmd		= iSubCmd;
	StorePacketHeader.cMsgType = cType;
	StorePacketHeader.nMasterID = nMasterID;
#if defined( PRE_WORLDCOMBINE_PARTY )
	StorePacketHeader.iWorldSetID = iWorldSetID;
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
	
	// CRawBuffer requires manual lock mechanism implementation.
	m_pStoredPacketBuffer->Lock();

	int Ret = 0;
	if( (Ret = m_pStoredPacketBuffer->Push( &StorePacketHeader, sizeof(StorePacketHeader) )) < 0 )
		_DANGER_POINT();
	
	if( Ret > 0 && iSize > 0 )
	{
		if( (Ret = m_pStoredPacketBuffer->Push( pPacket, iSize )) < 0 )
			_DANGER_POINT();
	}

	m_pStoredPacketBuffer->UnLock();
}

int CDNRUDPGameServer::Send(int iNetID, int iMainCmd, int iSubCmd, const void * pPacket, int iSize, int iPrior, BYTE cSeq)
{
	DNGAME_PACKET packet;
	int enlen = EncodeGamePacket(&packet, iMainCmd, iSubCmd, pPacket, iSize, cSeq);
	return CRUDPSocketFrame::Send(iNetID, (char*)&packet, enlen, iPrior);
}

bool CDNRUDPGameServer::AddConnect(int iNetID, CDNUserSession * pSession)
{
	std::map <int, CDNUserSession*>::iterator ii = m_ClientList.find(iNetID);
	if (ii == m_ClientList.end())
	{
		bool bRet = pSession->RUDPConnected(iNetID);		//notice to session
		if (bRet)
		{
			m_ClientList[iNetID] = pSession;			//insert managed conteiner
			return true;
		}
	}
	return false;
}

void CDNRUDPGameServer::RemoveConnection(int nGameRoomID, int nNetID, UINT nAccountDBID, UINT nSessionID)
{
	if (nNetID != 0)
	{
		std::map <int, CDNUserSession*>::iterator in;
		in = m_ClientList.find(nNetID);
		if (in != m_ClientList.end())
			m_ClientList.erase(in);
	}

	std::map <unsigned int, CDNUserSession*>::iterator is, ia;
	is = m_VerifyList.find(nSessionID);
	if (is != m_VerifyList.end())
		m_VerifyList.erase(is);
	else 
	{
#if defined( STRESS_TEST )
#else
		_DANGER_POINT();
#endif // #if defined( STRESS_TEST )
	}

	ia = m_ConnectList.find(nAccountDBID);
	if (ia != m_ConnectList.end())
		m_ConnectList.erase(ia);
	else _DANGER_POINT();

	m_pServerManager->RemoveGameUser(nGameRoomID, nAccountDBID, nSessionID, this);
	DisConnect(nNetID);
}

void CDNRUDPGameServer::SendRestoreMaster(char cWorldSetID)
{
	CDNRUDPGameServer::tRestoreMaster restore;
	memset(&restore, 0, sizeof(restore));

	restore.cWorldSetID = cWorldSetID;
	restore.nStep = _RESTORESTEP_SENDUSER;

	m_RestoreSync.Lock();
	m_RestoreMaster.push_back(restore);
	m_RestoreSync.UnLock();
}

#ifdef PRE_ADD_CHAGNESERVER_LOGGING
bool CDNRUDPGameServer::AddSaveSendData(int nMainCmd, char *pData, int iLen)
{
	int nRet = ERROR_DB;
	switch (nMainCmd)
	{
	case QUERY_CHANGESERVERUSERDATA:
		nRet = m_pChangeSaveBuffer->Push(pData, iLen);
		break;

	case QUERY_LASTUPDATEUSERDATA:
		nRet = m_pLastSaveBuffer->Push(pData, iLen);
		break;

	default:
		g_Log.Log(LogType::_ERROR, L"AddSaveSendData [MCMD:%d][LEN:%d]\n", nMainCmd, iLen);
		break;
	}

	if (nRet != ERROR_NONE)
	{
		g_Log.Log(LogType::_ERROR, L"AddSaveSendData [MCMD:%d][LEN:%d]\n", nMainCmd, iLen);
	}
	return nRet == ERROR_NONE ? true : false;
}
#else		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING
void CDNRUDPGameServer::AddSaveSendData(int nMainCmd, char *pData, int iLen)
{
	switch (nMainCmd)
	{
	case QUERY_CHANGESERVERUSERDATA:
		m_pChangeSaveBuffer->Push(pData, iLen);
		break;

	case QUERY_LASTUPDATEUSERDATA:
		m_pLastSaveBuffer->Push(pData, iLen);
		break;
	}
}
#endif		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING

CDNGameRoom * CDNRUDPGameServer::GetRoom(unsigned int iRoomID)
{
	std::map <unsigned int, CDNGameRoom*>::iterator ii = m_GameRoomList.find(iRoomID);
	if (ii != m_GameRoomList.end())
		return (*ii).second;
	return NULL;
}

CDNUserSession * CDNRUDPGameServer::GetSession(unsigned int iUID)
{
	std::map <unsigned int, CDNUserSession*>::iterator ii = m_VerifyList.find(iUID);
	if (ii != m_VerifyList.end())
		return (*ii).second;
	return NULL;
}

bool CDNRUDPGameServer::Accept(int iNetID, SOCKADDR_IN *pAddr, const void * pData, int iLen)
{
	if (m_nExternalProbePort != 0 &&
		pAddr->sin_port == ntohs(m_nExternalProbePort) &&
		pAddr->sin_addr.S_un.S_addr == inet_addr(m_szExternalProbeIP))
	{
		struct _RETURN_UDP_ADDR
		{
			unsigned short	port[2];
			unsigned long	addr[2];
		}	* packet = (_RETURN_UDP_ADDR*) pData;

		_ASSERT(pAddr->sin_port == ntohs(m_nExternalProbePort) &&
				pAddr->sin_addr.S_un.S_addr == inet_addr(m_szExternalProbeIP));

		if (iLen == sizeof(*packet) &&
			packet->port[0] == packet->port[1] &&
			packet->addr[0] == packet->addr[1] &&
			m_iUDPPort == 0)
		{
			m_iUDPPort = packet->port[0];
			m_iUDPAddr = packet->addr[0];

			g_Log.Log(LogType::_ERROR, L"UDP External Addr Recv [%d.%d.%d.%d (%d)] => [%d.%d.%d.%d (%d)]\n",
				pAddr->sin_addr.S_un.S_un_b.s_b1, pAddr->sin_addr.S_un.S_un_b.s_b2,
				pAddr->sin_addr.S_un.S_un_b.s_b3, pAddr->sin_addr.S_un.S_un_b.s_b4,
				ntohs(pAddr->sin_port),
				((unsigned char*)&m_iUDPAddr)[0], ((unsigned char*)&m_iUDPAddr)[1],
				((unsigned char*)&m_iUDPAddr)[2], ((unsigned char*)&m_iUDPAddr)[3],
				m_iUDPPort);
		}
		return false;
	}

	DNGAME_PACKET TestPacket;
	int iPacketLen;
	iPacketLen = CheckPacket(pData, iLen, &TestPacket);
	if (CalcGamePacketSize((unsigned char *)&TestPacket, iPacketLen) == iPacketLen)
		if (DecodeGamePacket(&TestPacket) == true)
			if (TestPacket.header == CS_SYSTEM && TestPacket.sub_header == eSystem::CS_CONNECT_REQUEST)
				return true;
	return false;
}

void CDNRUDPGameServer::DisConnected(int iNetID, bool bForce, bool bUnreachable)
{
	//Only disconnected at RUDP layer; actual disconnect is handled only when TCP disconnects, and if RUDP disconnected abnormally
	//Induce reconnection, and if it still fails after a certain time, fall back to TCP-only or disconnect
	std::map <int, CDNUserSession*>::iterator ii = m_ClientList.find(iNetID);
	if (ii != m_ClientList.end())
	{
		CDNUserSession * pSession = (*ii).second;
		m_ClientList.erase(ii);
		if (!pSession) return;
		if (bForce)
		{
			if (bUnreachable)
				pSession->DetachConnection(L"CDNRUDPGameServer::DisConnected Unreachable");
			else
				pSession->DetachConnection(L"CDNRUDPGameServer::DisConnected");
		}
		else
			pSession->RudpDisConnected();

		if (bUnreachable)
			g_Log.Log(LogType::_ERROR, pSession, L"Connect|Unreachable AID[%d]\n", pSession->GetAccountDBID());
	}
}

void CDNRUDPGameServer::ResetTimer()
{
	m_dwLastUpdateTime = timeGetTime();
	m_nCallCount = 0;
	m_nFPS = 0;

	ResetFPSHistory();
}

void CDNRUDPGameServer::TimeEvent()
{
	DWORD dwLocalTime = timeGetTime();

	if( dwLocalTime - m_dwPreviewTime >= 50 ) {
#ifndef _SKIP_BLOCK
		m_pServerManager->FlushConnectionBuffer(dwLocalTime);
#endif
		PROFILE_TIME_TEST_BLOCK_START( "CDNRUDPGameServer::TimeEvent()" );
		SendRestoreMaster();
		StoredProcess();
		IdleProcess();
		PROFILE_TIME_TEST_BLOCK_END();

		m_dwPreviewTime += 50;
		m_nCallCount++;

#ifdef PRE_ADD_FRAMEDELAY_LOG
		DWORD dwStartTime = dwLocalTime;
		dwLocalTime = timeGetTime();

		if (bIsFrameAlert())
		{
			if (m_nFrameReportLogPivot < DELAYED_PRCESS_LOG_COUNT && dwLocalTime - dwStartTime > 0)
			{
				g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] Report|Frame Elapsed Check SID:[%d] CallTick[%d] CallCount[%d] Elapsed[%d]\n", g_Config.nManagedID, GetServerID(), dwStartTime, m_nCallCount, dwLocalTime - dwStartTime);

			}
			m_nFrameReportLogPivot++;
		}
		else
		{
			m_nFrameReportLogPivot = 0;
		}
#else		//#ifdef PRE_ADD_FRAMEDELAY_LOG
		dwLocalTime = timeGetTime();
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
		if( dwLocalTime - m_dwLastUpdateTime >= 1000 ) {
			m_nFPS = m_nCallCount;
			m_nCallCount = 0;
			m_dwLastUpdateTime = dwLocalTime;

			if( m_nFPS < m_nMinFPS ) m_nMinFPS = m_nFPS;
			if( m_nFPS > m_nMaxFPS ) m_nMaxFPS = m_nFPS;
			m_nVecFPSList.push_back( m_nFPS );
			m_nTotalFPS += m_nFPS;
		}

#ifdef PRE_ADD_THREADAFFINITY
		CDNRUDPGameServer::CheckAffinitySetting(dwLocalTime);
#endif		//#ifdef PRE_ADD_THREADAFFINITY
		g_pDataManager->DoUpdate(dwLocalTime, GetServerID());
	}
}

bool CDNRUDPGameServer::IsAcceptRoom()
{
	return m_bIsAcceptRoom;
}

void CDNRUDPGameServer::CheckAcceptRoom()
{
	if( m_nVecFPSList.empty() ) {
		m_bIsAcceptRoom = true;
		return;
	}

	int nAveFrame = m_nTotalFPS / (int)m_nVecFPSList.size();
	if( nAveFrame < 17 )
	{
		m_bIsAcceptRoom = false;
		return;
	}

	if( m_uiConnectReqCount > 1000 )
	{
		float fConnectRate = m_uiConnectSuccessCount/static_cast<float>(m_uiConnectReqCount);
		if( fConnectRate <= 0.97 )
		{
			if( m_bIsAcceptRoom )
				g_Log.Log(LogType::_ENTERGAMECHECK, L"[%d] ConnectRate:%d%% dropped below threshold - room acceptance restricted! %d/%d\n", g_Config.nManagedID, static_cast<int>(fConnectRate*100),m_uiConnectSuccessCount, m_uiConnectReqCount );
			m_bIsAcceptRoom = false;

			return;
		}
		else
		{
			if( !m_bIsAcceptRoom )
			{
				g_Log.Log(LogType::_ENTERGAMECHECK, L"[%d] ConnectRate:%d%% recovered - room acceptance restored! %d/%d\n", g_Config.nManagedID, static_cast<int>(fConnectRate*100),m_uiConnectSuccessCount, m_uiConnectReqCount );
			}
		}
	}

	m_bIsAcceptRoom = true;
}

void CDNRUDPGameServer::ResetFPSHistory()
{
	m_nMinFPS = 20;
	m_nMaxFPS = 0;
	m_nTotalFPS = 0;
	m_nVecFPSList.clear();

//	m_nTotalProfile = 0;
//	m_nLastProfile = 0;
}

void CDNRUDPGameServer::IdleProcess()
{
	//Data may be modified by external events such as game room creation! Watch synchronization!
	unsigned long iCurTick = g_nPerfCounter = timeGetTime();
	if (m_nFrameReportPivot == 0)
		m_nFrameReportPivot = iCurTick;	

#ifdef PRE_ADD_FRAMEDELAY_LOG
	DWORD dwStartTime = 0;
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
	
	//Room creation occurs in stored message processing. Runs on a single thread so no sync needed, right? If a problem occurs.........????????????? oops..
	if( !m_GameRoomList.empty() )
	{
		std::map <unsigned int, CDNGameRoom*>::iterator ii;
		for (ii = m_GameRoomList.begin(); ii != m_GameRoomList.end();)
		{		//Change each room to Process state and move the destruction cycle inside Process.
			if ( (*ii).second->GetRoomState() == _GAME_STATE_DESTROYED )
			{	//Delete room
				CDNGameRoom * pRoom = (*ii).second;
				UINT nRoomID = pRoom->GetRoomID();

#if defined( PRE_THREAD_ROOMDESTROY )
				if( InterlockedCompareExchange( &pRoom->m_lRoomDestroyInterLocked, CDNGameRoom::eRoomDestoryStep::PushQueue, CDNGameRoom::eRoomDestoryStep::None ) == CDNGameRoom::eRoomDestoryStep::None )
				{
					if( g_pBackLoader->PushToDestroyProcess( pRoom ) == true )
					{
					}
					else
					{
						g_Log.Log(LogType::_ERROR, L"[%d] PushToDestroyProcess RoomID:%d Failed!\n", g_Config.nManagedID, nRoomID);
					}

					++ii;
					continue;
				}
				else if( InterlockedCompareExchange( &pRoom->m_lRoomDestroyInterLocked, CDNGameRoom::eRoomDestoryStep::Max, CDNGameRoom::eRoomDestoryStep::Destroyed ) != CDNGameRoom::eRoomDestoryStep::Destroyed )
				{
					++ii;
					continue;
				}


#else
				pRoom->FinalizeGameRoom();
#endif // #if defined( PRE_THREAD_ROOMDESTROY )

				bool bCrashed = pRoom->IsRoomCrash();
				SAFE_DELETE(pRoom);
				ii = m_GameRoomList.erase(ii);

				m_pServerManager->RemoveGameRoom(nRoomID, this, bCrashed);
				g_Log.Log(LogType::_NORMAL, L"[%d] _GAME_STATE_DESTROYED RoomID:%d \n", g_Config.nManagedID, nRoomID);
			}	
			else
			{
				CScopeInterlocked Scope( &((*ii).second->m_lRoomProcessInterLocked) );

				//Process if not deleted
				if ((*ii).second->IsRoomCrash() == false)
				{
#ifdef PRE_ADD_FRAMEDELAY_LOG
					dwStartTime = timeGetTime();
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
					(*ii).second->TcpProcess();
					(*ii).second->Process();
#ifdef PRE_ADD_FRAMEDELAY_LOG
					(*ii).second->m_dwProcessElapsedTime = timeGetTime() - dwStartTime;
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
				}
				ii++;
			}
		}

		//Reporting To MasterServer Process Count
		if (iCurTick - m_nFrameReportPivot > DELAYED_PRCESS_CHECKTICK)
		{
			// Min FPS : m_nMinFPS
			// Max FPS : m_nMaxFPS
			// Average FPS : m_nTotalFPS / m_nVecFPSList.size()
			// FPS history : m_nVecFPSList
			// Profile min value : m_nMinProfile
			// Profile max value : m_nMaxProfile
			// Profile average value : m_nTotalProfile / m_nVecProfileList.size();
			// Profile history : m_nVecProfileList
			// Send once and call ResetFPSHistory()~
			int nAvrF = m_nVecFPSList.size() == 0 ? 0 : m_nTotalFPS / (int)m_nVecFPSList.size();
			if (g_pMasterConnectionManager)
			{
				g_pMasterConnectionManager->SendGameFrame(m_iGameServerID, m_pServerManager->GetRoomCount(), (int)m_GameRoomList.size(), (int)m_ClientList.size(), \
					m_nMinFPS, m_nMaxFPS, nAvrF, IsAcceptRoom());
			}
				
			if( nAvrF < 17 )
			{
				m_bFrameAlert		= true;
				m_uiFrameAlertCount = 1;
				g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] Report|Frame Delayed SID:[%d] RoomCnt:[%d] UserCnt:[%d] AvrF:[%d] MinF:[%d]\n", g_Config.nManagedID, GetServerID(), (int)m_GameRoomList.size(), (int)m_ClientList.size(), nAvrF, m_nMinFPS);


				for( std::map<unsigned int, CDNGameRoom*>::iterator itor=m_GameRoomList.begin(); itor!=m_GameRoomList.end(); ++ itor )
				{
					CDNGameRoom* pRoom = (*itor).second;
					if( pRoom && pRoom->GetGameTask() )
					{
						int iFps = pRoom->GetGameTask()->m_pFrameSync ? static_cast<int>(pRoom->GetGameTask()->m_pFrameSync->GetFps()) : -1;
#ifdef PRE_ADD_FRAMEDELAY_LOG
						if (pRoom->m_dwProcessElapsedTime > 0)
							g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] Report|Frame Delayed MapIndex=%d UserCount=%d FPS=%d Elapsed=%d\n", g_Config.nManagedID, pRoom->GetGameTask()->GetMapTableID(), pRoom->GetUserCount(), iFps, pRoom->m_dwProcessElapsedTime );
#else		//#ifdef PRE_ADD_FRAMEDELAY_LOG
						g_Log.Log(LogType::_GAMESERVERDELAY, L"[%d] Report|Frame Delayed MapIndex=%d UserCount=%d FPS=%d\n", g_Config.nManagedID, pRoom->GetGameTask()->GetMapTableID(), pRoom->GetUserCount(), iFps );
#endif		//#ifdef PRE_ADD_FRAMEDELAY_LOG
					}
				}

#ifdef PRE_ADD_THREADAFFINITY
				CDNRUDPGameServer::EnableAffinitySetting();
#endif		//#ifdef PRE_ADD_THREADAFFINITY
			}
			else
			{
				m_bFrameAlert = false;
				if( m_uiFrameAlertCount )
				{
					if( ++m_uiFrameAlertCount >= 5 )
					{
						m_uiFrameAlertCount = 0;
						g_Log.Log(LogType::_GAMESERVERDELAY, L"[Alert][%d] Report|Frame Delayed SID:[%d] RoomCnt:[%d] UserCnt:[%d] AvrF:[%d]\n", g_Config.nManagedID, GetServerID(), (int)m_GameRoomList.size(), (int)m_ClientList.size(), nAvrF);

					}
				}
			}
			m_nFrameReportPivot = iCurTick;

			CheckAcceptRoom();
			ResetFPSHistory();
		}
	}
	else
	{
		if (iCurTick - m_nFrameReportPivot > DELAYED_PRCESS_CHECKTICK)
		{
			CheckAcceptRoom();
			if (g_pMasterConnectionManager)
				g_pMasterConnectionManager->SendGameFrame( m_iGameServerID, 0, 0, 0, 20, 20, 20, m_bIsAcceptRoom );
			m_nFrameReportPivot = iCurTick;
			ResetFPSHistory();
		}
	}

	//Process items to update in DB
	DataBaseUpdate(iCurTick);
}

void CDNRUDPGameServer::StoredProcess()
{	
	CScopeRoomListInterLocked RoomListScope( m_GameRoomList );

	ScopeSpinBufferSwitch Scope( m_pStoredPacketBuffer );

	char*	pBuffer = Scope.pGetBuffer();
	UINT	uiSize	= Scope.uiGetSize();

	while( uiSize )
	{
		_STORE_BUFFER_HEADER*	pHeader		= (_STORE_BUFFER_HEADER*)pBuffer;
		char*					pDataBuffer = pBuffer+sizeof(_STORE_BUFFER_HEADER);

		if( pHeader->cMsgType == EXTERNALTYPE_DATABASE )
		{
			if( DataBaseParsePacket( pHeader->iMainCmd, pHeader->iSubCmd, pDataBuffer, pHeader->iSize ) == false )
			{
				bool bError = true;
				switch( pHeader->iMainCmd )
				{
				case MAINCMD_AUTH:
					{
						switch (pHeader->iSubCmd)
						{
						case QUERY_RESETAUTH:
							{
								bError = false;
								break;
							}
						}
						break;
					}

					case MAINCMD_STATUS:
					{
						switch( pHeader->iSubCmd )
						{
							case QUERY_CHANGESTAGEUSERDATA:
							case QUERY_LASTUPDATEUSERDATA:
							{
								bError = false;
								break;
							}
						}
						break;
					}
					case MAINCMD_PVP:
					{
						switch( pHeader->iSubCmd )
						{
							case QUERY_ADD_PVP_LADDERRESULT:
							{
								bError = false;
								break;
							}
#if defined( PRE_WORLDCOMBINE_PVP )
							case QUERY_ADD_WORLDPVPROOM:
								{		
									TAAddWorldPvPRoom* pPacket = reinterpret_cast<TAAddWorldPvPRoom*>(pDataBuffer);
									CDNGameRoom * pRoom = GetRoom(pPacket->nRoomID);
									if ( pRoom )
									{
										pRoom->SetWorldPvPRoomDBIndex(pPacket->nPvPRoomDBIndex);
										if( pPacket->nAccountDBID > 0)
										{
											pRoom->SetWorldPvPRoomData(pPacket->Data, pPacket->nAccountDBID);											
										}
									}

									bError = false;		
								}
								break;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
							case QUERY_ADD_GAMBLEROOM:
								{		
									TAAddGambleRoom* pPacket = reinterpret_cast<TAAddGambleRoom*>(pDataBuffer);
									CDNGameRoom * pRoom = GetRoom(pPacket->nRoomID);
									if ( pRoom )
									{
										pRoom->SetGambleRoomDBID( pPacket->nGambleDBID );
									}

									bError = false;		
								}
								break;
#endif
						}
						break;
					}
					case MAINCMD_GUILD:
					{
						switch( pHeader->iSubCmd )
						{
						case QUERY_GET_GUILDREWARDITEM:
							{
								bError = false;

								DBGetGuildRewardItem *pPacket = reinterpret_cast<DBGetGuildRewardItem*>(pDataBuffer);
								CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

								// Resource inspection
								if (!pGuild)
									break;

#if !defined( PRE_ADD_NODELETEGUILD )
								CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
								if (FALSE == pGuild->IsEnable()) break;
#endif
								
								TGuildRewardItem *RewardItemInfo = pGuild->GetGuildRewardItem();
								
								std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
								pGuild->GetMemberList (aList);

								if (aList.empty())
									break;

								std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

								for (; iter != aList.end (); iter++)
								{
									CDNUserSession *pSession = GetSessionByADBID (iter->first);
									if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
										continue;

									pSession->SendGuildRewardItem(RewardItemInfo);
									pSession->SetGuildRewardItem(RewardItemInfo);
#if defined( PRE_ADD_GUILDREWARDITEM )
									pSession->ApplyGuildRewardSkill();
#endif
								}																
							}
							break;
						}						
					}
					break;
#if defined (PRE_ADD_DONATION)
					case MAINCMD_DONATION:
						{
							switch( pHeader->iSubCmd )
							{
							case QUERY_DONATION_TOP_RANKER:
								{
									TADonationTopRanker* pPacket = reinterpret_cast<TADonationTopRanker*>(pDataBuffer);

									for each (std::map<unsigned int, CDNUserSession*>::value_type v in m_ConnectList)
									{
										CDNUserSession* pSession = v.second;
										if (pSession->GetWorldSetID() != pPacket->cWorldID)
											continue;

										if (pSession->GetState() != SESSION_STATE_GAME_PLAY)
											continue;

										pSession->SendWorldSystemMsg(WORLDCHATTYPE_DONATION, 0, pPacket->wszCharacterName, 0);
									}

									bError = false;
								}
								break;
							}
						}
						break;
#endif // #if defined (PRE_ADD_DONATION)
#if defined( PRE_PARTY_DB )
						case MAINCMD_PARTY:
						{
							switch( pHeader->iSubCmd )
							{
								case QUERY_ADDPARTYANDMEMBERGAME:
								{
									TAAddPartyAndMemberGame* pPacket = reinterpret_cast<TAAddPartyAndMemberGame*>(pDataBuffer);
									if(pPacket->nRetCode == ERROR_NONE)
									{										
										CDNGameRoom * pRoom = GetRoom(pPacket->Data.PartyData.iRoomID);
										if ( pRoom )
											const_cast<Party::Data&>(pRoom->GetPartyStructData()) = pPacket->Data.PartyData;
									}
									bError = false;									
									break;						
								}
								case QUERY_MODPARTY:
								{
									TAModParty* pPacket = reinterpret_cast<TAModParty*>(pDataBuffer);
									if( pPacket->nRetCode != ERROR_NONE )
										break;
									
									CDNGameRoom * pRoom = GetRoom(pPacket->iRoomID);
									if ( pRoom )
									{
										const_cast<Party::Data&>(pRoom->GetPartyStructData()) = pPacket->PartyData;
										pRoom->SendRefreshParty(0,NULL);
									}
									bError = false;
									break;
								}
								case QUERY_OUTPARTY:
								{
									bError = false;
								}
								break;
#if defined( PRE_WORLDCOMBINE_PARTY )
								case QUERY_ADDWORLDPARTY:
								{
									TAAddParty* pPacket = reinterpret_cast<TAAddParty*>(pDataBuffer);
									if(pPacket->nRetCode == ERROR_NONE)
									{										
										CDNGameRoom * pRoom = GetRoom(pPacket->Data.PartyData.iRoomID);
										if ( pRoom )
											const_cast<Party::Data&>(pRoom->GetPartyStructData()) = pPacket->Data.PartyData;
									}									

									if( g_pMasterConnectionManager)
									{
										CDNGameRoom * pRoom = GetRoom(pPacket->Data.PartyData.iRoomID);
										if ( pRoom )
											g_pMasterConnectionManager->SendDelWorldParty(pRoom->GetWorldSetID(), pRoom->GetWorldPartyPrimaryIndex() );
									}

									bError = false;
								}
								break;
#endif
							}
							break;
						}
#endif

				}
				if( bError )
					_DANGER_POINT();
			}
		}
		else if( pHeader->cMsgType == EXTERNALTYPE_CASH )
		{
			if( CashParsePacket( pHeader->iMainCmd, pHeader->iSubCmd, pDataBuffer, pHeader->iSize ) == false )
				_DANGER_POINT();
		}
		else if (pHeader->cMsgType == EXTERNALTYPE_MASTER)
		{
			switch( pHeader->iMainCmd )
			{
				case IN_DESTROY:
				{
#if defined( PRE_WORLDCOMBINE_PARTY )
					DestroyAllFarmGameRoom( pHeader->iWorldSetID );
#else
					DestroyAllFarmGameRoom();
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
					break;
				}
				case MAGA_REQROOMID:			//MasterServer GameRoomId Request
				{
					RequestedCreateGameRoomFromMaster( (char)pHeader->nMasterID, pDataBuffer );
					break;
				}
				case MAGA_REQTUTORIALROOMID:	//MasterServer GameRoomId Request
				{
					RequestedCreateGameRoomFromMaster( (char)pHeader->nMasterID, pDataBuffer, true );
					break;
				}
				case MAGA_VILLAGESTATE:			//Change the handling
				{
					MAGAVillageState *pState = (MAGAVillageState*)pDataBuffer;

					RequestedVillageState( pState );
					break;
				}

				case MAGA_NOTICE:
				{
					MANotice * pPacket = (MANotice*)pDataBuffer;

					if (pPacket->nLen > CHATLENMAX-1)	break;
					WCHAR wszChatMsg[CHATLENMAX] = { 0, };
					_wcscpy(wszChatMsg, CHATLENMAX, pPacket->wszNoticeMsg, pPacket->nLen);

					//Too many notices per type, so consolidated into one
					switch (pPacket->Info.nNoticeType)
					{
					case _NOTICETYPE_WORLD: 
						{
							std::map <unsigned int, CDNUserSession*>::iterator ii;
							for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
								if ((*ii).second->GetWorldSetID() == pHeader->nMasterID)
									(*ii).second->SendNotice(wszChatMsg, pPacket->nLen, pPacket->Info.nSlideShowSec);
						}
						break;
					case _NOTICETYPE_EACHSERVER:
						{
							if (pPacket->Info.nManagedID != 0 && pPacket->Info.nManagedID == g_Config.nManagedID)
							{
								std::map <unsigned int, CDNUserSession*>::iterator ii;
								for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
									if ((*ii).second->GetWorldSetID() == pHeader->nMasterID)
										(*ii).second->SendNotice(wszChatMsg, pPacket->nLen, pPacket->Info.nSlideShowSec);
							}
						}
						break;
					}
					break;
				}

				case MAGA_CANCELNOTICE:
				{
					std::map <unsigned int, CDNUserSession*>::iterator ii;
					for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
						(*ii).second->SendNoticeCancel();
					break;
				}

				case MAGA_CHAT:
				{
					MAChat * pPacket = (MAChat*)pDataBuffer;

					if (pPacket->wChatLen > CHATLENMAX-1 || pPacket->wChatLen <= 0)	break;
					WCHAR wszChatMsg[CHATLENMAX] = { 0, };
					_wcscpy(wszChatMsg, CHATLENMAX, pPacket->wszChatMsg, pPacket->wChatLen);

					std::map <unsigned int, CDNUserSession*>::iterator ii;
					for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
					{
						if ((*ii).second->GetWorldSetID() == pHeader->nMasterID)
						{
							if ((*ii).second->GetState() == SESSION_STATE_GAME_PLAY )
								(*ii).second->SendChat(static_cast<eChatType>(pPacket->cType), pPacket->wChatLen, pPacket->wszFromCharacterName, wszChatMsg);
						}
					}
					break;
				}

				case MAGA_WORLDSYSTEMMSG:
				{
					MAWorldSystemMsg * pPacket = (MAWorldSystemMsg*)pDataBuffer;
					std::map <unsigned int, CDNUserSession*>::iterator ii;
					for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
					{
						if ((*ii).second->GetWorldSetID() == pHeader->nMasterID)
						{
							if ((*ii).second->GetState() == SESSION_STATE_GAME_PLAY )
								(*ii).second->SendWorldSystemMsg(pPacket->cType, pPacket->nID, pPacket->wszFromCharacterName, pPacket->nValue, pPacket->wszToCharacterName);					
						}
					}
					break;
				}

#if defined( PRE_PRIVATECHAT_CHANNEL )
				case MAGA_PRIVATECHANNELCHAT:
					{
						MAPrivateChannelChat * pPacket = (MAPrivateChannelChat*)pDataBuffer;
						CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nChannelID );

						if(pPrivateChatChannel)
						{
							std::list<TPrivateChatChannelMember> MemberList;
							MemberList.clear();
							pPrivateChatChannel->GetPrivateChannelMember(MemberList);

							for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin();itor != MemberList.end();itor++)
							{
								CDNUserSession *pSession = GetSessionByADBID(itor->nAccountDBID);
								if (!pSession) continue;

								pSession->SendChat( CHATTYPE_PRIVATECHANNEL, pPacket->wChatLen, pPacket->wszFromCharacterName, pPacket->wszChatMsg );
								
							}
						}
					}
					break;
#endif
				case MAGA_RESRECALL:
				{
					MAGAResRecall* pPacket = reinterpret_cast<MAGAResRecall*>(pDataBuffer);
					if( _CmdRecall( pPacket ) == false )
					{
						std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4413) ) );
						g_pMasterConnectionManager->SendChat( (char)pHeader->nMasterID, CHATTYPE_GM, pPacket->uiAccountDBID, wString.c_str(), static_cast<short>(wString.size()) );
					}
					break;
				}

				//=============================================================================================================
				// PvP Start
				//=============================================================================================================

				case MAGA_PVP_GAMEMODE:
				{
					InitializePvPGameMode( reinterpret_cast<MAGAPVP_GAMEMODE*>(pDataBuffer) );
					break;
				}
#if defined(PRE_ADD_PVP_TOURNAMENT)
				case MAGA_PVP_TOURNAMENT_INFO :
					{
						MAGAPVP_TOURNAMENT_INFO * pPacket = (MAGAPVP_TOURNAMENT_INFO*)pDataBuffer;

						CDNGameRoom * pRoom = GetRoom(pPacket->nGameRoomID);
						if (pRoom && pRoom->GetPvPGameMode() )
						{
							pRoom->GetPvPGameMode()->SetTournamentUserInfo(pPacket);							
						}
						else
							_DANGER_POINT();
						break;
					}
#endif
				case MAGA_BREAKINTOROOM:
				{
					RequestedBreakintoGameRoomFromMaster( (char)pHeader->nMasterID, reinterpret_cast<MAGABreakIntoRoom*>(pDataBuffer) );
					break;
				}
				case MAGA_PVP_MEMBERUSERSTATE:
					{
						MAGAPVPChangeUserState * pPacket = (MAGAPVPChangeUserState*)pDataBuffer;

						CDNGameRoom * pRoom = GetRoom(pPacket->nPvPRoomID);
						if (pRoom)
							pRoom->SendPvPUserState(pPacket->nAccountDBID, pPacket->uiUserState);
						else
							_DANGER_POINT();
						break;
					}
				case MAGA_PVP_FORCESTOP:
					{
						MAGAForceStopPvP * pPacket = (MAGAForceStopPvP*)pDataBuffer;

						CDNGameRoom * pRoom = GetRoom(pPacket->nGameRoomID);
						if (pRoom)
						{
							if (pPacket->nForceWinGuildDBID == 0)
							{
								//Splitting it like this skips result processing
								pRoom->OnSendPvPLobby();								
							}
							else
							{
								if (pRoom->GetPvPGameMode())
								{
									//Probably better to set the winning team beforehand before doing this
									pRoom->GetPvPGameMode()->SetForceWinGuild(pPacket->nForceWinGuildDBID);	//Something like this?
									pRoom->GetPvPGameMode()->OnCheckFinishRound(PvPCommon::FinishReason::OpponentTeamForceWin);
								}
							}
						}
						else
						{
							_DANGER_POINT();
						}
						break;
					}
				case MAGA_PVP_ALLSTOPGUILDWAR :
					{
						for( std::map<unsigned int, CDNGameRoom*>::iterator itor=m_GameRoomList.begin(); itor!=m_GameRoomList.end(); ++ itor )
						{
							CDNGameRoom* pRoom = (*itor).second;
							if( pRoom && pRoom->bIsGuildWarSystem() )
							{
								// Just destroy it without producing a result..
								pRoom->GetPvPGameMode()->FinishGameMode(PvPCommon::Team::Max, PvPCommon::FinishReason::TimeOver);								
							}
						}
					}
					break;

				//=============================================================================================================
				// PvP End
				//=============================================================================================================
				case MAGA_MASTERSYSTEM_SYNC_SIMPLEINFO:
				{
					MasterSystem::MAGASyncSimpleInfo* pPacket = reinterpret_cast<MasterSystem::MAGASyncSimpleInfo*>(pDataBuffer);

					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
					{
						if( pSession->CheckDBConnection() == false )
							break;

						pSession->GetDBConnection()->QueryGetMasterSystemSimpleInfo( pSession->GetDBThreadID(), pSession, true, pPacket->EventCode );
						pSession->QueryGetMasterSystemCountInfo( true );
					}
					break;
				}
				case MAGA_MASTERSYSTEM_SYNC_JOIN:
				{
					MasterSystem::MAGASyncJoin* pPacket = reinterpret_cast<MasterSystem::MAGASyncJoin*>(pDataBuffer);

					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
						pSession->SendMasterSystemJoin( ERROR_NONE, true, pPacket->bIsAddPupil, false );
					break;
				}
				case MAGA_MASTERSYSTEM_SYNC_LEAVE:
				{
					MasterSystem::MAGASyncLeave* pPacket = reinterpret_cast<MasterSystem::MAGASyncLeave*>(pDataBuffer);

					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
						pSession->SendMasterSystemLeave( ERROR_NONE, pPacket->bIsDelPupil );
					break;
				}
				case MAGA_MASTERSYSTEM_SYNC_GRADUATE:
				{
					MasterSystem::MAGASyncGraduate* pPacket = reinterpret_cast<MasterSystem::MAGASyncGraduate*>(pDataBuffer);

					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
						pSession->SendMasterSystemGraduate( pPacket->wszCharName );
					break;
				}
				case MAGA_MASTERSYSTEM_SYNC_CONNECT:
				{
					MasterSystem::MAGASyncConnect* pPacket = reinterpret_cast<MasterSystem::MAGASyncConnect*>(pDataBuffer);

					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
						pSession->SendMasterSystemConnect( pPacket->bIsConnect, pPacket->wszCharName );
					break;
				}
#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
				case MAGA_SYNC_SYSTEMMAIL:
				{
					MAGASyncSystemMail* pPacket = reinterpret_cast<MAGASyncSystemMail*>(pDataBuffer);
					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
						pSession->SendNotifyMail( pPacket->iTotalMailCount, pPacket->iNotReadMailCount, pPacket->i7DaysLeftMailCount, true );
					break;
				}
#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
				case MAGA_FARM_SYNC:
				{
					MAGAFarmSync* pPacket = reinterpret_cast<MAGAFarmSync*>(pDataBuffer);
					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession && pSession->GetCharacterDBID() == pPacket->biCharacterDBID )
					{
						if( pSession->GetGameRoom() && pSession->GetGameRoom()->bIsFarmRoom() )
						{
							switch( pPacket->Type )
							{
								case Farm::ServerSyncType::FIELDCOUNT:
								{
									pSession->GetDBConnection()->QueryGetFieldCountByCharacter( pSession->GetDBThreadID(), pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetCharacterDBID(), true );
									break;
								}
								case Farm::ServerSyncType::FIELDLIST:
								{
									static_cast<CDNFarmUserSession*>(pSession)->SetUpdateFieldListFlag( true );
									break;
								}
								case Farm::ServerSyncType::WAREHOUSE_ITEMCOUNT:
								{
									pSession->GetDBConnection()->QueryGetCountHarvestDepotItem( pSession );
									break;
								}
							}
						}
					}
					break;
				}
				case MAGA_FARM_SYNC_ADDWATER:
				{
					MAGAFarmSyncAddWater* pPacket = reinterpret_cast<MAGAFarmSyncAddWater*>(pDataBuffer);
					CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
					if( pSession )
					{
						if( pSession->GetGameRoom() && pSession->GetGameRoom()->bIsFarmRoom() )
						{
							// Secondary skill proficiency
							CSecondarySkill* pSkill = pSession->GetSecondarySkillRepository()->Get( SecondarySkill::SubType::CultivationSkill );
							if( pSkill )
							{
								pSession->SendFarmAddWaterAnotherUser( pPacket->wszCharName );
								int nAddPoint = pPacket->iAddPoint;
								CNpcReputationProcessor::CheckAndCalcUnionBenefit( pSession, TStoreBenefitData::CultivateProficiencyUp, nAddPoint );
								pSession->GetSecondarySkillRepository()->AddExp( pSkill->GetSkillID(), nAddPoint );
							}
						}
					}
					break;
				}

				// Guild
				case MAGA_DISMISSGUILD:				// Guild disband notification
					{
						MADismissGuild *pPacket = reinterpret_cast<MADismissGuild*>(pDataBuffer);
					}
					break;

				case MAGA_ADDGUILDMEMBER:				// Notify add guild member
					{
						MAAddGuildMember *pPacket = reinterpret_cast<MAAddGuildMember*>(pDataBuffer);
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendInviteGuildMemberAck (pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->wszCharacterName, ERROR_NONE, 0, 0, pPacket->nJob, pPacket->cLevel, &pPacket->Location, &pPacket->GuildUID);
						}

					}
					break;

				case MAGA_DELGUILDMEMBER:				// Guild member removal (leave/kick) notification
					{
						MADelGuildMember *pPacket = reinterpret_cast<MADelGuildMember*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);
						if (!pGuild) 
							break;
						
#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++) 
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;
							
							if (!pPacket->bIsExiled)
								pSession->SendLeaveGuildMember(pPacket->nAccountDBID, pPacket->nCharacterDBID, ERROR_NONE, &pPacket->GuildUID, false);
							else 
							{
								pSession->SendExileGuildMember(pPacket->nAccountDBID, pPacket->nCharacterDBID, ERROR_NONE, &pPacket->GuildUID);

								// Initialize guild visual info
								if (pPacket->nAccountDBID == pSession->GetAccountDBID()) 
								{
									pSession->ResetGuildSelfView();
									pSession->SendExileGuildMember(pSession->GetAccountDBID(), pSession->GetCharacterDBID(), ERROR_NONE, &pPacket->GuildUID);
								}
							}
						}

						if (pPacket->bIsExiled) {
							// In case of expulsion, already removed from guild member list so must search across all users (20100210 b4nfter)

							CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);
							if (pSession && pSession->GetGuildUID().IsSet() && pGuild->GetUID() == pSession->GetGuildUID()) {
								pSession->ResetGuildSelfView();
								pSession->SendExileGuildMember(pSession->GetAccountDBID(), pSession->GetCharacterDBID(), ERROR_NONE, &pPacket->GuildUID);
								pSession->GetItem()->RemoveGuildReversionItem( true );
								pSession->GetItem()->RemoveGuildReversionVehicleItemData( true );
								pSession->SendGuildRewardItem(NULL);
							}
						}
					}
					break;

				case MAGA_CHANGEGUILDINFO:			// Guild info change notification
					{
						MAChangeGuildInfo *pPacket = reinterpret_cast<MAChangeGuildInfo*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendChangeGuildInfo(pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->btGuildUpdate, ERROR_NONE, pPacket->Int1, pPacket->Int2, pPacket->Int64, pPacket->Text, &pPacket->GuildUID);
						}
					}
					break;

				case MAGA_CHANGEGUILDMEMBERINFO:		// Guild member info update notification
					{
						MAChangeGuildMemberInfo *pPacket = reinterpret_cast<MAChangeGuildMemberInfo*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendChangeGuildMemberInfo(pPacket->nReqAccountDBID, pPacket->nReqCharacterDBID, pPacket->nChgAccountDBID, pPacket->nChgCharacterDBID, pPacket->btGuildMemberUpdate, ERROR_NONE, pPacket->Int1, pPacket->Int2, pPacket->Int64, pPacket->Text, &pPacket->GuildUID);

							switch(pPacket->btGuildMemberUpdate) 
							{
								// Change rank (Guild Master -> Guild Member ?)
								case GUILDMEMBUPDATE_TYPE_ROLE:				
								{
									if (pPacket->nChgAccountDBID == pSession->GetAccountDBID()) 
									{
										pSession->GetGuildSelfView().btGuildRole = static_cast<BYTE>(pPacket->Int1);

										// Update guild visual info
										pSession->RefreshGuildSelfView();
									}
								}
								break;

								// Delegate guild master (guild master -> member ?)
								case GUILDMEMBUPDATE_TYPE_GUILDMASTER:		
								{
									if (pPacket->nChgAccountDBID == pSession->GetAccountDBID()) 
									{
										pSession->GetGuildSelfView().btGuildRole = static_cast<BYTE>(GUILDROLE_TYPE_MASTER);

										// Update guild visual info
										pSession->RefreshGuildSelfView();
									}
								}
								break;

								default:
								break;
							}
						}
					}
					break;

				case MAGA_REFRESHGUILDSELFVIEW:		// Guild visual info (personal/group) change notification
					{
						MARefreshGuildSelfView *pPacket = reinterpret_cast<MARefreshGuildSelfView*>(pDataBuffer);
						DN_ASSERT(NULL != pPacket,				"Invalid!");
						DN_ASSERT(pPacket->GuildUID.IsSet(),	"Invalid!");

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						if (0 != pPacket->nAccountDBID && 0 != pPacket->nCharacterDBID) 
						{
							CDNUserSession *pSession = GetSessionByADBID (pPacket->nAccountDBID);
							if (pSession && pGuild->GetUID() == pSession->GetGuildUID()) 
							{
								pSession->SetGuildSelfView(pGuild->GetInfo()->GuildView);
							}
						}
						else
						{
							std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
							pGuild->GetMemberList (aList);

							if (aList.empty())
								break;

							std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

							for (; iter != aList.end (); iter++)
							{
								CDNUserSession *pSession = GetSessionByADBID (iter->first);
								if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
									continue;

								pSession->SetGuildSelfView(pGuild->GetInfo()->GuildView);
							}
						}
					}
					break;

				case MAGA_GUILDCHAT:				// Guild chat notification
					{
						MAGuildChat *pPacket = reinterpret_cast<MAGuildChat*>(pDataBuffer);
						DN_ASSERT(NULL != pPacket,					"Invalid!");
						DN_ASSERT(pPacket->GuildUID.IsSet(),		"Invalid!");
						DN_ASSERT(LONG_MAX == pPacket->nManagedID,	"Invalid!");

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendGuildChat(pPacket->GuildUID, pPacket->nAccountDBID, pPacket->nCharacterDBID, pPacket->wszChatMsg, pPacket->nLen);
						}
					}
					break;

#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
				case MAGA_GUILDCHAT_MOBILE:
					{
						MADoorsGuildChat *pPacket = reinterpret_cast<MADoorsGuildChat*>(pDataBuffer);
						DN_ASSERT(NULL != pPacket,					"Invalid!");
						DN_ASSERT(pPacket->GuildUID.IsSet(),		"Invalid!");
						DN_ASSERT(LONG_MAX == pPacket->nManagedID,	"Invalid!");

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						if (pGuild->IsMemberExist(pPacket->nCharacterDBID, NULL))
						{
							std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
							pGuild->GetMemberList (aList);

							if (aList.empty())
								break;

							std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

							for (; iter != aList.end (); iter++)
							{
								CDNUserSession *pSession = GetSessionByADBID (iter->first);
								if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
									continue;

								pSession->SendDoorsGuildChat(pPacket->GuildUID, pPacket->nCharacterDBID, pPacket->wszChatMsg, pPacket->nLen);
							}
							return;
						}
						_DANGER_POINT_MSG(L"Received mobile guild message from non-guild member");
					}
					break;
#endif		//#ifdef PRE_ADD_DOORS_GUILDCHAT_DISCONNECT
				case MAGA_CHANGEGUILDNAME :			// Change guild name
					{
						MAGuildChangeName *pPacket = reinterpret_cast<MAGuildChangeName*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendChangeGuildName(pPacket->wszGuildName);
							_wcscpy( pSession->GetGuildSelfView().wszGuildName, _countof(pSession->GetGuildSelfView().wszGuildName), pPacket->wszGuildName, (int)wcslen(pPacket->wszGuildName) );
							pSession->RefreshGuildSelfView();
						}
					}
					break;
				case MAGA_CHANGEGUILDMARK:
					{
						MAGuildChangeMark *pPacket = reinterpret_cast<MAGuildChangeMark*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();

						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendChangeGuildMark(pPacket);
							pSession->GetGuildSelfView().wGuildMark = pPacket->wMark;
							pSession->GetGuildSelfView().wGuildMarkBG = pPacket->wMarkBG;
							pSession->GetGuildSelfView().wGuildMarkBorder = pPacket->wMarkBorder;
							pSession->RefreshGuildSelfView();
						}
					}
					break;
				case MAGA_GUILDLEVELUP:
					{
						MAGuildLevelUp *pPacket = reinterpret_cast<MAGuildLevelUp*>(pDataBuffer);
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();
						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;


							pSession->SendGuildLevelUp(pPacket->nLevel);
						}
					}
					break;

				case MAGA_UPDATEGUILDEXP:
					{
						MAGuildExp *pPacket = reinterpret_cast<MAGuildExp*>(pDataBuffer);
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> aList;
						pGuild->GetMemberList (aList);

						if (aList.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = aList.begin ();
						for (; iter != aList.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							if (pPacket->cPointType == GUILDPOINTTYPE_STAGE)
							{
								if (pSession->GetCharacterDBID() == pPacket->biCharacterDBID)
								{
									pSession->SendUpdateGuildExp(ERROR_NONE, pPacket->cPointType, pPacket->nPointValue, pPacket->nGuildExp, pPacket->biCharacterDBID, pPacket->nMissionID);
									break;
								}
							}
							else
								pSession->SendUpdateGuildExp(ERROR_NONE, pPacket->cPointType, pPacket->nPointValue, pPacket->nGuildExp, pPacket->biCharacterDBID, pPacket->nMissionID);
						}
					}
					break;
				case MAGA_ENROLLGUILDWAR:			// Notify Guild War application
					{
						MAEnrollGuildWar *pPacket = reinterpret_cast<MAEnrollGuildWar*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> vGuildMember;
						pGuild->GetMemberList (vGuildMember);

						if (vGuildMember.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = vGuildMember.begin ();

						for (; iter != vGuildMember.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->GetGuildSelfView().wWarSchduleID = pPacket->wScheduleID;
							pSession->GetGuildSelfView().cTeamColorCode = pPacket->cTeamColorCode;
							pSession->RefreshGuildSelfView();

							pSession->SendEnrollGuildWar(ERROR_NONE, pPacket->wScheduleID, pPacket->cTeamColorCode);
						}
					}
					break;
				case MAGA_CHANGE_GUILDWAR_STEP:
					{
						MAChangeGuildWarStep * pPacket = (MAChangeGuildWarStep*)pDataBuffer;						

						std::map <unsigned int, CDNUserSession*>::iterator ii;
						for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
						{							
							(*ii).second->SendGuildWarEvent(pPacket->wScheduleID, pPacket->cEventStep, pPacket->cEventType);
						}
					}
					break;
				case MAGA_SETGUILDWAR_PRE_WIN_GUILD :
					{
						MAGuildWarPreWinGuild *pPacket = reinterpret_cast<MAGuildWarPreWinGuild*>(pDataBuffer);
						for( std::map <unsigned int, CDNGameRoom*>::iterator itor=m_GameRoomList.begin() ; itor!=m_GameRoomList.end() ; ++itor )						
						{
							CDNGameRoom* pRoom = (*itor).second;
							if( pRoom && pRoom->bIsFarmRoom() == true && pRoom->GetRoomState() == _GAME_STATE_PLAY )
							{
								static_cast<CDNFarmGameRoom*>(pRoom)->OnStartGuildWarFinal();
							}
						}
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif


						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> vGuildMember;
						pGuild->GetMemberList (vGuildMember);

						if (vGuildMember.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = vGuildMember.begin ();

						for (; iter != vGuildMember.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendGuildWarPreWinGuild(pPacket->bPreWin);
						}
					}
					break;
				case MAGA_SETGUILDWAR_TOURNAMENTWIN :
					{
						MASetGuildWarTournamentWin *pPacket = reinterpret_cast<MASetGuildWarTournamentWin*>(pDataBuffer);		
						std::map <unsigned int, CDNUserSession*>::iterator ii;

						SCGuildWarTournamentWin GuildWarTournamentWin;
						memset(&GuildWarTournamentWin, 0, sizeof(GuildWarTournamentWin));
						GuildWarTournamentWin.cMatchTypeCode = pPacket->cMatchTypeCode;
						memcpy(GuildWarTournamentWin.wszGuildName, pPacket->wszGuildName, sizeof(GuildWarTournamentWin.wszGuildName));

						for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
						{							
							(*ii).second->SendGuildWarTournamentWin(&GuildWarTournamentWin);
						}
					}
					break;
				case MAGA_GUILDRECRUIT_MEMBER :
					{
						MAGuildRecruitMember *pPacket = reinterpret_cast<MAGuildRecruitMember*>(pDataBuffer);
						CDNUserSession* pSession = GetSessionByADBID( pPacket->uiAccountDBID );
						if( pSession )
						{
							if(pPacket->cAcceptType == GuildRecruitSystem::AcceptType::AcceptOn)
							{
								pSession->SetGuildSelfView(TGuildSelfView(pPacket->GuildView, GUILDROLE_TYPE_JUNIOR));
								if (pSession->GetGuildUID().IsSet()) 
								{	
									CDNGuildBase* pGuild = g_pGuildManager->At(pSession->GetGuildUID());
									if (!pGuild) 
									{
										if (pSession->GetDBConnection() && pSession->GetDBConnection()->GetActive())
										{
											pSession->GetDBConnection()->QueryGetGuildInfo(pSession, pSession->GetGuildUID().nDBID, true);
											pSession->GetDBConnection()->QueryGetGuildRewardItem( pSession->GetDBThreadID(), pSession->GetWorldSetID(), pSession->GetAccountDBID(), pSession->GetGuildUID().nDBID );
										}
									}
								}
								pSession->GetEventSystem()->OnEvent( EventSystem::OnGuildJoin );
							}
							pSession->SendGuildRecruitMemberResult(ERROR_NONE, pPacket->GuildView.wszGuildName, (GuildRecruitSystem::AcceptType::eType)pPacket->cAcceptType);
						}
						break;
					}
					break;
				case MAGA_GUILD_ADD_REWARDITEM:
					{
						MAGuildRewardItem *pPacket = reinterpret_cast<MAGuildRewardItem*>(pDataBuffer);
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif
						
						TGuildRewardItem RewardItemInfo;
						memset( &RewardItemInfo, 0, sizeof(RewardItemInfo) );
						pGuild->AddGuildRewardItem( pPacket->nItemID, pPacket->m_tExpireDate, RewardItemInfo );

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> vGuildMember;
						pGuild->GetMemberList (vGuildMember);

						if (vGuildMember.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = vGuildMember.begin ();

						for (; iter != vGuildMember.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;							

							pSession->SendAddGuildRewardItem(RewardItemInfo);
						}
					}
					break;
				case MAGA_GUILD_EXTEND_GUILDSIZE:
					{
						MAExtendGuildSize *pPacket = reinterpret_cast<MAExtendGuildSize*>(pDataBuffer);
						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						pGuild->SetInfo()->wGuildSize = pPacket->nGuildSize;
						
						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> vGuildMember;
						pGuild->GetMemberList (vGuildMember);

						if (vGuildMember.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = vGuildMember.begin ();

						for (; iter != vGuildMember.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;							

							pSession->SendExtendGuildSize(pPacket->nGuildSize);
						}
					}
					break;
				case MAGA_CHANGE_CHARACTERNAME:
					{
						MAChangeCharacterName *pPacket = reinterpret_cast<MAChangeCharacterName*>(pDataBuffer);
						std::map <unsigned int, CDNGameRoom*>::iterator ii;
						for (ii = m_GameRoomList.begin(); ii != m_GameRoomList.end(); ii++)
						{
							if ((*ii).second->bIsFarmRoom())
								(*ii).second->UpdateCharacterName(pPacket);
						}
					}
					break;
				case MAGA_GUILDMEMBLOGINLIST:		// Guild member login list notification
					{
						MAGuildMemberLoginList *pPacket = reinterpret_cast<MAGuildMemberLoginList*>(pDataBuffer);

						CDNGuildBase* pGuild = g_pGuildManager->At (pPacket->GuildUID);

						// Resource inspection
						if (!pGuild)
							break;

#if !defined( PRE_ADD_NODELETEGUILD )
						CDetachAutoEx<CDNGuildBase>	AutoDetach (pGuild);
						if (FALSE == pGuild->IsEnable()) break;
#endif

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>> List;
						pGuild->GetMemberList (List);

						if (List.empty())
							break;

						std::vector <std::pair<UINT, INT64>, boost::pool_allocator<UINT>>::iterator iter = List.begin ();
						for (; iter != List.end (); iter++)
						{
							CDNUserSession *pSession = GetSessionByADBID (iter->first);
							if (!pSession || !pSession->GetGuildUID().IsSet() || pGuild->GetUID() != pSession->GetGuildUID() || (iter->second != pSession->GetCharacterDBID()))
								continue;

							pSession->SendGuildMemberLoginList(pPacket->GuildUID, pPacket->nCount, pPacket->List);
						}
					}
					break;

				case MAGA_ZEROPOPULATION:
					{
						MAGAZeroPopulation * pPacket = (MAGAZeroPopulation*)pDataBuffer;
						m_bZeroPopulation = pPacket->bZeroPopulation;

						std::map <unsigned int, CDNUserSession*>::iterator ii;
						for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
							(*ii).second->SendZeroPopulation(pPacket->bZeroPopulation);
						break;
					}

				case MAGA_ASSIGN_PERIODQUEST:
					{
						MAGAAssginPeriodQuest * pPacket = (MAGAAssginPeriodQuest*)pDataBuffer;
						
						std::map <unsigned int, CDNUserSession*>::iterator ii;
						for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
						{							
							(*ii).second->GetQuest()->OnPeriodEvent(pPacket->nQuestID, pPacket->bFlag);
							(*ii).second->SendAssginPeriodQuest(pPacket->nQuestID, pPacket->bFlag);	
						}
						break;
					}

				case MAGA_NOTICE_PERIODQUEST:
					{
						MAGANoticePeriodQuest * pPacket = (MAGANoticePeriodQuest*)pDataBuffer;

						std::map <unsigned int, CDNUserSession*>::iterator ii;
						for (ii = m_ConnectList.begin(); ii != m_ConnectList.end(); ii++)
						{				
							if ((*ii).second->GetWorldSetID() == pPacket->cWorldSetID)
								(*ii).second->SendPeriodQuestNotice(pPacket->nItemID, pPacket->nNoticeCount);	
						}

						break;
					}
				case MAGA_CHECK_LASTDUNGEONINFO:
				{
					MAGACheckLastDungeonInfo* pPacket = reinterpret_cast<MAGACheckLastDungeonInfo*>(pDataBuffer);

					CDNGameRoom* pGameRoom = GetRoom( pPacket->iRoomID );

					bool bIsCheck = false;
					if( pGameRoom )
						bIsCheck = pGameRoom->bIsBackupPartyInfo( pPacket->biCharacterDBID );
					g_pMasterConnectionManager->SendCheckLastDungeonInfo( pPacket->cWorldSetID, pPacket->uiAccountDBID, pPacket->biCharacterDBID, bIsCheck, bIsCheck ? pGameRoom->GetPartyName() : NULL );
					break;
				}
				case MAGA_DELETE_BACKUPDUNGEONINFO:
				{
					MAGADeleteBackupDungeonInfo* pPacket = reinterpret_cast<MAGADeleteBackupDungeonInfo*>(pDataBuffer);

					CDNGameRoom* pGameRoom = GetRoom( pPacket->iRoomID );
					if( pGameRoom )
						pGameRoom->DelBackupPartyInfo( pPacket->biCharacterDBID );
					break;
				}

#if defined( PRE_WORLDCOMBINE_PARTY )
				case MAGA_GET_WORLDPARTYMEMBER:
				{
					SendWorldPartyMember( (char)pHeader->nMasterID, reinterpret_cast<MAGetWorldPartyMember*>(pDataBuffer) );					
				}
				break;
#endif
#if defined( PRE_ADD_BESTFRIEND )
				case MAGA_CANCEL_BESTFRIEND:
				{
					MACancelBestFriend *pPacket = (MACancelBestFriend*)pDataBuffer;

					CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);
					if (!pSession) break;

					pSession->GetDBConnection()->QueryGetBestFriend(pSession->GetDBThreadID(), pSession, true);

					pSession->SendCancelBestFriend(ERROR_NONE, pPacket->wszFromName, pPacket->bCancel);
				}
				break;
				case MAGA_CLOSE_BESTFRIEND:
				{
					MACloseBestFriend *pPacket = (MACloseBestFriend*)pDataBuffer;

					CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);
					if (!pSession) break;						

					pSession->SendCloseBestFriend(ERROR_NONE, pPacket->wszFromName);

					pSession->CloseBestFirend();
				}
				break;
				case MAGA_LEVELUP_BESTFRIEND:
				{
					MALevelUpBestFriend *pPacket = (MALevelUpBestFriend*)pDataBuffer;

					CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);
					if (!pSession) break;						

					pSession->BestFriendChangeLevel(pPacket->cLevel);											   
				}
				break;
#endif
#if defined( PRE_PRIVATECHAT_CHANNEL )
				case MAGA_PRIVATECHATCHANNEL_MEMBERADD:
					{
						MAAddPrivateChannelMember *pPacket = (MAAddPrivateChannelMember*)pDataBuffer;

						CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nPrivateChatChannelID );
						
						if(pPrivateChatChannel)
						{
							std::list<TPrivateChatChannelMember> MemberList;
							MemberList.clear();
							pPrivateChatChannel->GetPrivateChannelMember(MemberList);

							for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin();itor != MemberList.end();itor++)
							{
								if(itor->nAccountDBID == pPacket->Member.nAccountDBID )
									continue;

								CDNUserSession *pSession = GetSessionByADBID(itor->nAccountDBID);
								if (!pSession) continue;						

								pSession->SendPrivateChatChannelMemberAdd(ERROR_NONE, pPacket->Member);
							}
						}						
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MEMBERINVITE:
					{
						MAInvitePrivateChannelMember *pPacket = (MAInvitePrivateChannelMember*)pDataBuffer;

						CDNUserSession *pSession = GetSessionByADBID(pPacket->nInviteAccountDBID);
						if (!pSession) break;						
						if(pSession)
						{
							if(pSession->GetPrivateChannelID())
							{
								if(g_pMasterConnectionManager)
								{
									g_pMasterConnectionManager->SendInvitePrivateChatChannelResult(pSession->GetWorldSetID(), ERROR_ALREADY_JOINCHANNEL, pPacket->nMasterAccountDBID);
								}								
							}
							else
							{
								pSession->SetPrivateChannelID(pPacket->nPrivateChatChannelID);
								pSession->GetDBConnection()->QueryInvitePrivateChatChannelMember(pSession, pPacket->nPrivateChatChannelID, pPacket->nMasterAccountDBID);
							}
						}
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MEMBERINVITERESULT:
					{
						MAInvitePrivateChannelMemberResult *pPacket = (MAInvitePrivateChannelMemberResult*)pDataBuffer;

						CDNUserSession *pSession = GetSessionByADBID(pPacket->nMasterAccountDBID);
						if (!pSession) break;						
						pSession->SendPrivateChatChannelInviteResult(pPacket->nRet);
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MEMBERDEL:
					{
						MADelPrivateChannelMember *pPacket = (MADelPrivateChannelMember*)pDataBuffer;

						CDNPrivateChaChannel* pPrivateChatChannel = g_pPrivateChatChannelManager->GetPrivateChannelInfo( pPacket->cWorldSetID, pPacket->nPrivateChatChannelID );

						if(pPrivateChatChannel)
						{
							std::list<TPrivateChatChannelMember> MemberList;
							MemberList.clear();
							pPrivateChatChannel->GetPrivateChannelMember(MemberList);							

							for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin();itor != MemberList.end();itor++)
							{
								if(itor->biCharacterDBID == pPacket->biCharacterDBID )
									continue;

								CDNUserSession *pSession = GetSessionByADBID(itor->nAccountDBID);
								if (!pSession) continue;								

								if(pPacket->eType == PrivateChatChannel::Common::KickMember)
									pSession->SendPrivateChatChannelMemberKick(ERROR_NONE, pPacket->biCharacterDBID, pPacket->wszName );
								else
									pSession->SendPrivateChatChannelMemberDel(ERROR_NONE, pPacket->biCharacterDBID, pPacket->wszName );
							}
						}						
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MEMBERKICKRESULT:
					{
						MAKickPrivateChannelMemberResult* pPacket = (MAKickPrivateChannelMemberResult*)pDataBuffer;
						CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);						
						if( pSession  )
						{
							pSession->SendPrivateChatChannelKickResult(ERROR_NONE, true);
							pSession->SetPrivateChannelID(0);
						}						
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MOD:
					{
						MAModPrivateChannel* pPacket = (MAModPrivateChannel*)pDataBuffer;
						if( g_pPrivateChatChannelManager )
						{
							CDNPrivateChaChannel* pPrivateChatChannel =  g_pPrivateChatChannelManager->GetPrivateChannelInfo(pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);
							if( pPrivateChatChannel )
							{								
								std::list<TPrivateChatChannelMember> MemberList;
								MemberList.clear();
								pPrivateChatChannel->GetPrivateChannelMember(MemberList);

								CDNUserSession* pSession = NULL;
								WCHAR MasterName[NAMELENMAX];
								memset(MasterName, 0, sizeof(MasterName));
								pPrivateChatChannel->GetMasterCharacterName(MasterName);

								for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin(); itor != MemberList.end();itor++)
								{
									CDNUserSession *pSession = GetSessionByADBID(itor->nAccountDBID);						
									if(pSession)
									{									
										pSession->SendPrivateChatChannelInfoMod( ERROR_NONE, pPacket->eType, MasterName, pPacket->biCharacterDBID );
									}
								}								
							}
						}
					}
					break;
				case MAGA_PRIVATECHATCHANNEL_MODMEMBERNAME:
					{
						MAModPrivateChannelMemberName* pPacket = (MAModPrivateChannelMemberName*)pDataBuffer;
						if( g_pPrivateChatChannelManager )
						{
							CDNPrivateChaChannel* pPrivateChatChannel =  g_pPrivateChatChannelManager->GetPrivateChannelInfo(pPacket->cWorldSetID, pPacket->nPrivateChatChannelID);
							if( pPrivateChatChannel )
							{			
								TPrivateChatChannelMember Member = pPrivateChatChannel->GetPrivateChannelMember( pPacket->biCharacterDBID );
								if( Member.biCharacterDBID <= 0)
									break;
								std::list<TPrivateChatChannelMember> MemberList;
								MemberList.clear();
								pPrivateChatChannel->GetPrivateChannelMember(MemberList);

								CDNUserSession* pSession = NULL;								
								for(std::list<TPrivateChatChannelMember>::iterator itor = MemberList.begin(); itor != MemberList.end();itor++)
								{
									CDNUserSession *pSession = GetSessionByADBID(itor->nAccountDBID);						
									if(pSession)
									{
										pSession->SendPrivateChatChannelMemberAdd(ERROR_NONE, Member);
									}
								}								
							}
						}
					}
					break;
#endif
#if defined(PRE_ADD_MUTE_USERCHATTING)
				case MAGA_MUTE_USERCHAT:
					{
						MAMuteUserChat* pPacket = (MAMuteUserChat*)pDataBuffer;
						
						CDNUserSession *pSession = GetSessionByADBID(pPacket->uiMuteUserAccountID);
						if(pSession)
							pSession->ModCommonVariableData(CommonVariable::Type::MuteChatting, pPacket->nMuteMinute);
						
						break;
					}
#endif	// #if defined(PRE_ADD_MUTE_USERCHATTING)
#if defined( PRE_ALTEIAWORLD_EXPLORE )
				case MAGA_ALTEIAWORLD_SENDTICKET:
					{
						MAAlteiaWorldSendTicket* pPacket = (MAAlteiaWorldSendTicket*)pDataBuffer;
						CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);						
						if(pSession)
						{							
							int nMaxSendTicketCount = static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::AlteiaWorldSendTicketCount ));
							pSession->GetDBConnection()->QueryAddAlteiaWorldSendTicketList(pSession, pPacket->biSendCharacterDBID, pPacket->wszSendCharacterName, nMaxSendTicketCount );
						}
						else
						{
							if (g_pMasterConnectionManager)
								g_pMasterConnectionManager->SendAddAlteiaWorldSendTicketResult( pPacket->cWorldSetID, ERROR_ALTEIAWORLD_NOTEXIST_RECVCHARACTER, pPacket->biSendCharacterDBID );
						}
						
					}
					break;					
#endif
				default:
				{
					CDNUserSession *pSession = GetSessionByADBID(pHeader->nAccountDBID);
					if (pSession)
						pSession->MasterMessageProcess( pHeader->iMainCmd, pHeader->iSubCmd, pDataBuffer, pHeader->iSize );
					else
						//Some cases can occur, check logs occasionally
						g_Log.Log(LogType::_ERROR, L"Session Not Found [AID:%u] [MC:%d] [SC:%d]\n", pHeader->nAccountDBID, pHeader->iMainCmd, pHeader->iSubCmd);
					break;
				}
			}
		}
		else
			g_Log.Log(LogType::_ERROR, L"Unknown Ext MessageType Check [Type:%d]\n", pHeader->cMsgType);

		uiSize	-= pHeader->iBufSize;
		pBuffer	+= pHeader->iBufSize;
	}
}

bool CDNRUDPGameServer::GameRoomDataBaseParsePacket(int iMainCmd, int iSubCmd, const void * pPacket, int iLen)
{
	switch( iMainCmd )
	{
		case MAINCMD_DARKLAIR:
		{
			switch (iSubCmd)
			{
				case QUERY_UPDATE_DARKLAIR_RESULT:
				{
					TAUpdateDarkLairResult* pData		= reinterpret_cast<TAUpdateDarkLairResult*>(const_cast<void*>(pPacket));
					CDNGameRoom*			pGameRoom	= GetRoom( pData->iRoomID );
					if( pGameRoom )
						pGameRoom->OnDBMessageProcess( iMainCmd, iSubCmd, static_cast<char*>(const_cast<void*>(pPacket)), iLen );
					
					return true;
				}
			}
			break;
		}
		case MAINCMD_FARM:
		{
			switch (iSubCmd)
			{
				case QUERY_GETLIST_FIELD:
				{
					TAGetListField* pData = reinterpret_cast<TAGetListField*>(const_cast<void*>(pPacket));
					CDNGameRoom * pFarmRoom = (CDNFarmGameRoom*)GetRoom(pData->iRoomID);
					if (pFarmRoom)
					{
						if (pFarmRoom->GetFarmIndex() > 0)
							((CDNFarmGameRoom*)pFarmRoom)->OnDBMessageProcess(iMainCmd, iSubCmd, reinterpret_cast<char*>(pData), iLen);
						else
							_DANGER_POINT();		//This must not happen.
					}
					return true;
				}

				case QUERY_GETLIST_FARM:
				{
					TAGetListFarm * pData = (TAGetListFarm*)pPacket;
					
					for (int i = 0; i < pData->cCount && i < Farm::Max::FARMCOUNT; i++)
					{
						std::map <unsigned int, CDNGameRoom*>::iterator ii;
						for (ii = m_GameRoomList.begin(); ii != m_GameRoomList.end(); ii++)
						{
							if ((*ii).second->bIsFarmRoom() && (*ii).second->GetFarmIndex() == pData->Farms[i].iFarmDBID)
							{
								(*ii).second->SetFarmActivation(pData->Farms[i].bStartActivate);
								break;
							}
						}
					}
					return true;
				}
				// The top-level variable of TA~ packets below MUST be RoomID!!!!!!
				case QUERY_ADD_FIELD:
				case QUERY_DEL_FIELD:
				case QUERY_HARVEST:
				case QUERY_ADD_FIELD_ATTACHMENT:
				case QUERY_GET_FIELDITEMCOUNT:
				case QUERY_GETLIST_FIELD_FORCHARACTER:
				case QUERY_ADD_FIELD_FORCHARACTER:
				case QUERY_DEL_FIELD_FORCHARACTER:
				case QUERY_ADD_FIELD_FORCHARACTER_ATTACHMENT:
				case QUERY_HARVEST_FORCHARACTER:
				{
					int iRoomID;
					memcpy( &iRoomID, static_cast<const char*>(pPacket)+sizeof(TAHeader), sizeof(int) );

					CDNGameRoom* pGameRoom = GetRoom( iRoomID );
					if( pGameRoom && pGameRoom->bIsFarmRoom() )
					{
						static_cast<CDNFarmGameRoom*>(pGameRoom)->OnDBMessageProcess( iMainCmd, iSubCmd, reinterpret_cast<char*>(const_cast<void*>(pPacket)), iLen );
					}

					return true;
				}
			}
			break;
		}
	}

	return false;
}

bool CDNRUDPGameServer::CashParsePacket(int iMainCmd, int iSubCmd, const void * pPacket, int iLen)
{
	UINT nAccountDBID;
	memcpy(&nAccountDBID, pPacket, sizeof(UINT));

	CDNUserSession *pSession = GetSessionByADBID(nAccountDBID);		//key = uid, list of created session objects
	if (pSession)
	{
		if ((pSession->GetState() >= SESSION_STATE_READY && pSession->GetState() <= SESSION_STATE_READY_TO_LOGIN))
			pSession->CashMessageProcess(iMainCmd, iSubCmd, (char*)pPacket, iLen);
		return true;
	}
	return false;
}

bool CDNRUDPGameServer::DataBaseParsePacket(int iMainCmd, int iSubCmd, const void * pPacket, int iLen)
{
	if( GameRoomDataBaseParsePacket( iMainCmd, iSubCmd, pPacket, iLen ) )
		return true;

	UINT nAccountDBID;
	memcpy(&nAccountDBID, pPacket, sizeof(UINT));

	CDNUserSession *pSession = GetSessionByADBID(nAccountDBID);		//key = uid, list of created session objects
	if (pSession)
	{
		if ((pSession->GetState() >= SESSION_STATE_READY && pSession->GetState() <= SESSION_STATE_READY_TO_LOGIN))
			pSession->DBMessageProcess(iMainCmd, iSubCmd, (char*)pPacket, iLen);
		return true;
	}
	return false;
}

void CDNRUDPGameServer::DataBaseUpdate(unsigned long CurTick)
{
	TQUpdateCharacter Update;
	CDNDBConnection *pDBCon = NULL;

	if (m_pChangeSaveBuffer->GetCount() > 0){
		if (CurTick >= m_dwChangeSaveTick + 50){
			if (m_pChangeSaveBuffer->View((char*)&Update, sizeof(TQUpdateCharacter)) == 0){
				CDNUserSession* pSession = GetUserSession( Update.nAccountDBID );
				if( pSession )
				{
					pDBCon = pSession->GetDBConnection();
				}
				else
				{
					pDBCon = g_pDBConnectionManager->GetDBConnection( Update.uiDBRandomSeed, Update.cThreadID );
				}
				if (pDBCon)
				{
					pDBCon->QueryDelExpireitem(Update.cThreadID, Update.cWorldSetID, Update.nAccountDBID, Update.biCharacterDBID);
					pDBCon->AddSendData(MAINCMD_STATUS, QUERY_CHANGESERVERUSERDATA, (char*)&Update, sizeof(TQUpdateCharacter));
					m_pChangeSaveBuffer->Skip(sizeof(TQUpdateCharacter));
				}
#ifdef PRE_ADD_CHAGNESERVER_LOGGING
				else
				{
					g_Log.Log(LogType::_ERROR, L"ExternalDoUpdate pDBCon == NULL [ADBID:%d]\n", Update.nAccountDBID);
				}
#endif		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING
			}
			m_dwChangeSaveTick = CurTick;
		}
	}

	if (m_pLastSaveBuffer->GetCount() > 0){
		if (CurTick >= m_dwLastSaveTick + 50){
			if (m_pLastSaveBuffer->View((char*)&Update, sizeof(TQUpdateCharacter)) == 0){
				CDNUserSession* pSession = GetUserSession( Update.nAccountDBID );
				if( pSession )
				{
					pDBCon = pSession->GetDBConnection();
				}
				else
				{
					pDBCon = g_pDBConnectionManager->GetDBConnection( Update.uiDBRandomSeed, Update.cThreadID );
				}
				if (pDBCon){
					pDBCon->AddSendData(MAINCMD_STATUS, QUERY_LASTUPDATEUSERDATA, (char*)&Update, sizeof(TQUpdateCharacter));
					m_pLastSaveBuffer->Skip(sizeof(TQUpdateCharacter));
				}
#ifdef PRE_ADD_CHAGNESERVER_LOGGING
				else
				{
					g_Log.Log(LogType::_ERROR, L"ExternalDoUpdate pDBCon == NULL [ADBID:%d]\n", Update.nAccountDBID);
				}
#endif		//#ifdef PRE_ADD_CHAGNESERVER_LOGGING
			}
			m_dwLastSaveTick = CurTick;
		}
	}

	if (m_bIsCloseService)
	{
		if (m_GameRoomList.empty())
		if (m_pChangeSaveBuffer->GetCount() <= 0 && m_pLastSaveBuffer->GetCount() <= 0)
		{
			m_bIsAllFlushSaveData = true;
			m_bIsCloseService = false;
			m_pServerManager->CheckCloseGameServer(this);
		}
	}
}

void CDNRUDPGameServer::Parse(int iMainCmd, int iSubCmd, const void * pData, int iLen, int iNetID, BYTE cSeqLevel)
{
	//Parses incoming messages at RUDP layer; delegates actual packet parsing to each session layer.
	std::map <int, CDNUserSession*>::iterator ii = m_ClientList.find(iNetID);							//key = netid, list of actually connected objects
	if (ii != m_ClientList.end())
	{
		if ((*ii).second->IsConnected())
		{
			if (iMainCmd >= IN_DISCONNECT)
			{
				_DANGER_POINT();
				return;
			}
			//If info exists, consider it already connected; proceed directly to message patching
			if ((*ii).second->GetGameRoom())
				(*ii).second->GetGameRoom()->OnDispatchMessage((*ii).second, iMainCmd, iSubCmd, (char*)pData, iLen, cSeqLevel);
			else _DANGER_POINT();
		}
#ifdef PRE_MOD_INDUCE_TCPCONNECT
		else if (iMainCmd == CS_SYSTEM && iSubCmd == eSystem::CS_TCP_CONNECT_REQ)
		{
			(*ii).second->RecvReqTCPConnect();
		}
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
	}	
	else
	{
		//If none, this is the first connection. Let's enforce validation.
		CSConnectRequest * pMsg = (CSConnectRequest*)pData;
		if (sizeof(CSConnectRequest) == iLen && iMainCmd == CS_SYSTEM && iSubCmd == eSystem::CS_CONNECT_REQUEST)
		{
			//Entered normally.
			//Setting up validation. This is the only chance to validate the connection.
			std::map <unsigned int , CDNUserSession*>::iterator ic = m_VerifyList.find(pMsg->nSessionID);
			if (ic == m_VerifyList.end())
			{
				//Fake! Only discards rudp connection info. Session destruction is handled at the room level.
				DisConnect(iNetID);
				g_Log.Log(LogType::_ERROR, L"Connect|RUDP Illegal Connect [SID:%d]\n", pMsg->nSessionID);
				return;
			}
			if (AddConnect(iNetID, (*ic).second) == true)
			{
#ifdef PRE_MOD_INDUCE_TCPCONNECT
				(*ic).second->SendReqTCPConnect(timeGetTime());
#else		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
				(*ic).second->SendPacket(SC_SYSTEM, eSystem::SC_TCP_CONNECT_REQ, NULL, 0, _RELIABLE);
				g_Log.Log(LogType::_GAMECONNECTLOG, (*ic).second, L"[%d] CS_CONNECT_REQUEST\n", g_Config.nManagedID );
#endif		//#ifdef PRE_MOD_INDUCE_TCPCONNECT
				++m_uiConnectReqCount;
			}
			else
			{
				DisConnect(iNetID);
				g_Log.Log(LogType::_ERROR, L"RUDP AddConnect Error sid:[%d]\n", pMsg->nSessionID);
			}
		}
		else
			DisConnect(iNetID);
	}
}

void CDNRUDPGameServer::Recv(int iNetID, void * pData, int iLen)
{
	for( int i=0 ; i<iLen ; )
	{
		void* pSrc = static_cast<char*>(pData)+i;
		int iPacketLen = CalcGamePacketSize(pSrc, iLen-i);
		if( iLen < iPacketLen+i )
			break;

		DNGAME_PACKET* pPacket = static_cast<DNGAME_PACKET*>(pSrc);
		if(DecodeGamePacket(pPacket) == true)
		{
			Parse(pPacket->header, pPacket->sub_header, pPacket->data, pPacket->datasize, iNetID, pPacket->seq);
		}

		i += iPacketLen;
	}
}

CDNUserSession * CDNRUDPGameServer::GetSessionByADBID(unsigned int nAccountDBID)
{
	std::map <unsigned int, CDNUserSession*>::iterator ii = m_ConnectList.find(nAccountDBID);
	if (ii != m_ConnectList.end())
		return (*ii).second;
	return NULL;
}

CDNGameRoom * CDNRUDPGameServer::CreateGameRoom(MAGAReqRoomID * pPacket)
{
	if (m_bIsCloseService) return NULL;	//Game server is closing.

	unsigned int nRoomID = m_pServerManager->GenRoomID(this);	
	if (nRoomID == 0)
		return NULL;

	CDNGameRoom* pRoom = NULL;
	switch( pPacket->GameTaskType ) {
		case GameTaskType::Normal:
			pRoom = new IBoostPoolDNGameRoom(this, nRoomID, pPacket );
			break;
		case GameTaskType::PvP:
			pRoom = new CDNPvPGameRoom( this, nRoomID, pPacket );
			break;
		case GameTaskType::DarkLair:
			pRoom = new CDNDLGameRoom(this, nRoomID, pPacket );
			break;
		case GameTaskType::Farm:
		{
			pRoom = new CDNFarmGameRoom( this, nRoomID, pPacket );
			break;
		}
	}

	if( pRoom == NULL )
	{
		m_pServerManager->RemoveGameRoom(nRoomID, this);
		return NULL;
	}

	pRoom->SetPartyKickedMemberList( pPacket->nKickedMemberList );

	m_GameRoomList[pRoom->GetRoomID()] = pRoom;
	unsigned int iRoomID = pRoom->GetRoomID();

#if defined( PRE_PARTY_DB )
	pRoom->SetPartyRoomID(iRoomID);
#endif
	
	return pRoom;
}

CDNGameRoom * CDNRUDPGameServer::CreateGameRoom(MAGAReqTutorialRoomID * pPacket)
{
	if (m_bIsCloseService) return NULL; //Game server is closing.
	unsigned int nRoomID = m_pServerManager->GenRoomID(this);
	if (nRoomID == 0)
		return NULL;

	MAGAReqRoomID Packet;
	memset( &Packet, 0, sizeof(Packet) );
	
	Packet.GameTaskType		= GameTaskType::Normal;
	Packet.cReqGameIDType	= REQINFO_TYPE_SINGLE;
	Packet.InstanceID		= pPacket->nAccountDBID;
	Packet.cMemberCount		= 1;
	Packet.nMapIndex		= pPacket->nTutorialMapIndex;
	Packet.cGateNo			= pPacket->cTutorialGateNo;
	Packet.nRandomSeed		= pPacket->nRandomSeed;
#if defined( PRE_PARTY_DB )
	Packet.PartyData.Clear();
#else
	Packet.ItemLootRule	= ITEMLOOTRULE_NONE;
#endif
	Packet.bDirectConnect	= true;

	CDNGameRoom* pRoom = new IBoostPoolDNGameRoom(this, nRoomID, &Packet );
	if( pRoom == NULL )
	{
		m_pServerManager->RemoveGameRoom(nRoomID, this);
		return NULL;
	}

	m_GameRoomList[pRoom->GetRoomID()] = pRoom;
	unsigned int iRoomID = pRoom->GetRoomID();
	
	return pRoom;
}

bool CDNRUDPGameServer::VerifyUserIDs(UINT nAccountDBID, UINT nSessionID)
{
	//ID validation on initial entry. If this fails, the user session has not yet been properly disconnected.
	std::map <unsigned int, CDNUserSession*>::iterator ii = m_ConnectList.find(nAccountDBID);
	if (ii != m_ConnectList.end()) return false;
	ii = m_VerifyList.find(nSessionID);
	if (ii != m_VerifyList.end()) return false;
	return m_pServerManager->VerifyUserIDs(nAccountDBID, nSessionID);
}

void CDNRUDPGameServer::RequestedCreateGameRoomFromMaster(char cWorldSetID, char * pData, bool bTutorial)
{
	if (bTutorial){	// Tutorial in Login
		MAGAReqTutorialRoomID *pReqRoomID = (MAGAReqTutorialRoomID*)pData;

		//Check user IDs. (Disconnect processing may not have run yet)
#if !defined( STRESS_TEST )
		if (VerifyUserIDs(pReqRoomID->nAccountDBID, pReqRoomID->nSessionID) == false)
		{
			g_Log.Log(LogType::_ERROR, cWorldSetID, 0, 0, 0, L"VerifyUserIDs Fail ADBID[%d] SID[%d]\n", pReqRoomID->nAccountDBID, pReqRoomID->nSessionID);
			g_pMasterConnectionManager->SendSetTutorialRoomID(cWorldSetID, pReqRoomID->nAccountDBID, GetServerID(), 0, pReqRoomID->nLoginServerID);
			return ;
		}
#endif // #if defined( STRESS_TEST )

		CDNGameRoom* pRoom = CreateGameRoom(pReqRoomID);
		if (!pRoom)
		{
			g_pMasterConnectionManager->SendSetTutorialRoomID(cWorldSetID, pReqRoomID->nAccountDBID, GetServerID(), 0, pReqRoomID->nLoginServerID);
			_DANGER_POINT();
			return;
		}

		// Set Room WorldID
		pRoom->m_iWorldID = pReqRoomID->cWorldSetID;		

		//Room creation complete; pre-create objects for peers connecting via RUDP.
		int nSize = 0;
		CDNUserSession * pSession = pRoom->CreateGameSession(pReqRoomID->wszAccountName, pReqRoomID->nAccountDBID, pReqRoomID->nSessionID, pReqRoomID->biCharacterDBID, 0, pReqRoomID->cWorldSetID, 0, true, 
			bTutorial, pReqRoomID->bAdult, PCBang::Grade::None );
		if (pSession != NULL)
		{
			BYTE cThreadID = 0;
			CDNDBConnection* pDBCon = NULL;
			pRoom->CopyDBConnectionInfo( pDBCon, cThreadID );
			pSession->SetDBConInfo( pDBCon, cThreadID );
			m_ConnectList[pReqRoomID->nAccountDBID] = pSession;
			m_VerifyList[pReqRoomID->nSessionID] = pSession;
			m_pServerManager->AddGameUser(pRoom->GetRoomID(), pReqRoomID->nAccountDBID, pReqRoomID->nSessionID, this);	//Defer default setup to connection at the master connect layer.
		}
		else
			_DANGER_POINT();

		g_pMasterConnectionManager->SendSetTutorialRoomID(cWorldSetID, pReqRoomID->nAccountDBID, GetServerID(), pRoom->GetRoomID(), pReqRoomID->nLoginServerID);
		g_Log.Log(LogType::_NORMAL, cWorldSetID, 0, 0, 0, L"Report|[RoomCnt:%d][RoomID:%d MAGA_REQTUTORIALROOMID] ADBID:%u SID:%u RandomSeed:%d Map:%d GateNo:%d\r\n", \
			g_pGameServerManager->GetRoomCount(), pRoom->GetRoomID(), pReqRoomID->nAccountDBID, pReqRoomID->nSessionID, pReqRoomID->nRandomSeed, pReqRoomID->nTutorialMapIndex, pReqRoomID->cTutorialGateNo);
	}
	else 
	{
		MAGAReqRoomID*	pReqRoomID	= reinterpret_cast<MAGAReqRoomID*>(pData);

		if (pReqRoomID->nEventRoomIndex > 0)
			g_Log.Log(LogType::_PVPROOM, pReqRoomID->cWorldSetID, 0, 0, 0, L"RequestedCreateGameRoomFromMaster - Recv Packet [Index:%d][Event:%d] \r\n", static_cast<UINT>(pReqRoomID->InstanceID), pReqRoomID->nEventRoomIndex);

		//Check user IDs. (Disconnect processing may not have run yet)
		for( int i=0; i <pReqRoomID->cMemberCount ; ++i )
		{
#if !defined( STRESS_TEST )
			if (VerifyUserIDs(pReqRoomID->arrMemberInfo[i].MemberAccountDBID, pReqRoomID->arrMemberInfo[i].MemberSessionID) == false)
			{
				g_Log.Log(LogType::_GAMECONNECTLOG, cWorldSetID, pReqRoomID->arrMemberInfo[i].MemberAccountDBID, pReqRoomID->arrMemberInfo[i].biMemberCharacterDBID, 0, L"VerifyUserIDs Fail!!\n" );
				g_Log.Log(LogType::_NORMAL, cWorldSetID, 0, 0, 0, L"VerifyUserIDs Fail ADBID[%d] SID[%d]\n", pReqRoomID->arrMemberInfo[i].MemberAccountDBID, pReqRoomID->arrMemberInfo[i].MemberSessionID);
				g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pReqRoomID->GameTaskType, pReqRoomID->cReqGameIDType, pReqRoomID->InstanceID, GetServerID(), 0, pReqRoomID->cVillageID, NULL );
				return ;
			}
#endif // #if defined( STRESS_TEST )
		}
#if defined( PRE_WORLDCOMBINE_PVP )
		if( pReqRoomID->eWorldReqType > WorldPvPMissionRoom::Common::NoneType )
		{
			pReqRoomID->cMemberCount = 0;
		}
#endif

		CDNGameRoom* pRoom = CreateGameRoom(pReqRoomID);
		if( pRoom )
		{
			// Set Room WorldID
			pRoom->m_iWorldID = pReqRoomID->cWorldSetID;

			if (pReqRoomID->cReqGameIDType == REQINFO_TYPE_FARM)
			{
				pRoom->SetSync2SyncStateTemplateMethod(timeGetTime());
				return;
			}

			if (pReqRoomID->nEventRoomIndex > 0)
				g_Log.Log(LogType::_PVPROOM, pReqRoomID->cWorldSetID, 0, 0, 0, L"RequestedCreateGameRoomFromMaster - Success to create room [Index:%d][Event:%d] \r\n", static_cast<UINT>(pReqRoomID->InstanceID), pReqRoomID->nEventRoomIndex);

			if (pReqRoomID->cReqGameIDType == REQINFO_TYPE_PVP && pReqRoomID->nGuildDBID[0] > 0 && pReqRoomID->cMemberCount == 0)
			{
				//In PvP Guild War (system-created), it can start even without users.
				SendSetGameRoomID(cWorldSetID, pRoom, pReqRoomID, NULL);
				pRoom->SetSync2PvPModeStateTemplateMethod(timeGetTime());
				return;
			}
			//Room creation complete; pre-create objects for peers connecting via RUDP.
			for( int i=0; i <pReqRoomID->cMemberCount ; ++i )
			{
				WCHAR wszAccountName[IDLENMAX];
				_wcscpy( wszAccountName, _countof(wszAccountName), pReqRoomID->arrMemberInfo[i].wszAccountName, (int)wcslen(pReqRoomID->arrMemberInfo[i].wszAccountName) );
				
				CDNUserSession* pSession = pRoom->CreateGameSession( wszAccountName, pReqRoomID->arrMemberInfo[i].MemberAccountDBID,																			\
																	 pReqRoomID->arrMemberInfo[i].MemberSessionID, pReqRoomID->arrMemberInfo[i].biMemberCharacterDBID, pReqRoomID->arrMemberInfo[i].MemberTeam,	\
																	 pReqRoomID->cWorldSetID, pReqRoomID->cVillageID,																								\
																	 pReqRoomID->arrMemberInfo[i].MemberAccountDBID == pReqRoomID->nLeaderAccountDBID ? true : false, bTutorial,								\
																	 pReqRoomID->arrMemberInfo[i].bAdult, pReqRoomID->arrMemberInfo[i].cPCBangGrade, &pReqRoomID->arrMemberInfo[i].VoiceInfo );

				if( pSession )
				{
					BYTE cThreadID = 0;
					CDNDBConnection* pDBCon = NULL;
					pRoom->CopyDBConnectionInfo( pDBCon, cThreadID );
					pSession->SetDBConInfo( pDBCon, cThreadID );

					if( !pRoom->bIsFarmRoom() )
					{
						pSession->SetPartyMemberIndex(pReqRoomID->arrMemberInfo[i].cMemberIndex);
						pRoom->SetPartyMemberIndex(pSession->GetPartyMemberIndex(), pSession->GetSessionID(), pSession->GetTeam());
					}
					m_ConnectList[pReqRoomID->arrMemberInfo[i].MemberAccountDBID]	= pSession;
					m_VerifyList[pReqRoomID->arrMemberInfo[i].MemberSessionID]		= pSession;
					m_pServerManager->AddGameUser(pRoom->GetRoomID(), pReqRoomID->arrMemberInfo[i].MemberAccountDBID, pReqRoomID->arrMemberInfo[i].MemberSessionID, this );	//Defer default setup to connection at the master connect layer.
					pSession->SetPvPFatigueOption(pReqRoomID->arrMemberInfo[i].bPvPFatigue);
					pSession->SetPvPUserState(pReqRoomID->arrMemberInfo[i].uiPvPUserState);
				}
				else
					_DANGER_POINT();
			}

			UINT nMemberArr[PARTYMAX];
			memset(&nMemberArr, 0, sizeof(nMemberArr));
			for (int i = 0; i < pReqRoomID->cMemberCount; i++)
			{
				if (pReqRoomID->arrMemberInfo[i].MemberAccountDBID <= 0) continue;
				nMemberArr[i] = pReqRoomID->arrMemberInfo[i].MemberAccountDBID;
			}

#if defined( PRE_WORLDCOMBINE_PVP )
			if( pRoom->bIsWorldPvPRoom() )
			{
				// If mission room, set game mode here
				InitialzeWorldPvPGameMode(pRoom, pReqRoomID);
				// Save to DB				
				pRoom->AddDBWorldPvPRoom( &pReqRoomID->MissonRoomData, pReqRoomID->arrMemberInfo[0].MemberAccountDBID );				
			}
#endif
#if defined( PRE_PVP_GAMBLEROOM )
			if( pReqRoomID->cGambleType > 0 )
			{
				pRoom->CreateGambleRoom( pReqRoomID->cGambleType, pReqRoomID->nGamblePrice );
			}
#endif
			if(  pReqRoomID->cReqGameIDType	== REQINFO_TYPE_LADDER )
				static_cast<CDNPvPGameRoom*>(pRoom)->SetPvPLadderMatchType( pReqRoomID->MatchType );

			if (pReqRoomID->nEventRoomIndex > 0)
				g_Log.Log(LogType::_PVPROOM, pReqRoomID->cWorldSetID, 0, 0, 0, L"RequestedCreateGameRoomFromMaster - Before SendSetRoomID [Index:%d][Event:%d] \r\n", static_cast<UINT>(pReqRoomID->InstanceID), pReqRoomID->nEventRoomIndex);

			SendSetGameRoomID(cWorldSetID, pRoom, pReqRoomID, nMemberArr);
		}
		else	//Creation failed!
		{
			g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pReqRoomID->GameTaskType, pReqRoomID->cReqGameIDType, pReqRoomID->InstanceID, GetServerID(), 0, pReqRoomID->cVillageID, NULL );
			_DANGER_POINT();
		}
	}
}

void CDNRUDPGameServer::RequestedBreakintoGameRoomFromMaster( char cWorldSetID, MAGABreakIntoRoom* pPacket )
{
	CDNGameRoom* pRoom = GetRoom( pPacket->iGameRoomID );
	if( pRoom )
	{
		CDnGameTask* pGameTask = pRoom->GetGameTask();
		if( pRoom->GetRoomState() != _GAME_STATE_PLAY || pGameTask==NULL || pGameTask->IsWaitPlayCutScene() )
		{
			if( pPacket->BreakIntoType == BreakInto::Type::GMTrace )
			{
				std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,3433) ) );
				g_pMasterConnectionManager->SendChat(pRoom->m_iWorldID, CHATTYPE_GM, pPacket->uiAccountDBID, const_cast<WCHAR*>(wString.c_str()), static_cast<short>(wString.size()) );
			}
			else if( pPacket->BreakIntoType == BreakInto::Type::WorldZoneParty )
			{
				if (pRoom->bIsPvPRoom() == false)
					g_pMasterConnectionManager->SendInvitedMemberReturnMsg(pRoom->m_iWorldID, pPacket->uiAccountDBID, ERROR_PARTY_JOINFAIL);
			}
			else if( pPacket->BreakIntoType == BreakInto::Type::PartyRestore )
			{
#if defined( PRE_PARTY_DB )
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pRoom->m_iWorldID, ERROR_CANT_RESTORE_PARTY_STATE, pPacket->uiAccountDBID, pPacket->BreakIntoType );
#else
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pRoom->m_iWorldID, ERROR_CANT_RESTORE_PARTY_STATE, pPacket->uiAccountDBID );
#endif // #if defined( PRE_PARTY_DB )
			}
#if defined( PRE_PARTY_DB )
			else if( pPacket->BreakIntoType == BreakInto::Type::PartyJoin )
			{
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pRoom->m_iWorldID, ERROR_PARTY_JOINFAIL, pPacket->uiAccountDBID, pPacket->BreakIntoType );
			}
#endif // #if defined( PRE_PARTY_DB )
			return;
		} // if( pRoom->GetRoomState() != _GAME_STATE_PLAY || pGameTask==NULL || pGameTask->IsWaitPlayCutScene() )

#if defined( PRE_WORLDCOMBINE_PVP )
		if( pRoom->bIsWorldPvPRoom()  )
		{			
			if( !pRoom->CheckWorldPvPRoomBreakInto( pPacket ) )
				return;
		}
#endif
		if( pRoom->bIsLadderRoom() )
		{
			if( pPacket->BreakIntoType==BreakInto::Type::GMTrace )
			{
				g_pMasterConnectionManager->SendInvitedMemberReturnMsg(pRoom->m_iWorldID, pPacket->uiAccountDBID, ERROR_PARTY_GM_JOINFAIL_LADDERROOM);
				return;
			}
			DWORD dwCount = pRoom->GetUserCount( PvPCommon::Team::Observer );
			if( dwCount >= LadderSystem::Common::MaxObserver )
			{
				g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pRoom->m_GameTaskType, REQINFO_TYPE_LADDER_OBSERVER, pPacket->uiAccountDBID, GetServerID(), 0, pPacket->cVillageID, NULL );
				return;
			}
		}

		if( pPacket->BreakIntoType==BreakInto::Type::PartyRestore )
		{
			if( pRoom->bIsBackupPartyInfo( pPacket->biCharacterDBID ) == false )
			{
#if defined( PRE_PARTY_DB )
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID, pPacket->BreakIntoType );
#else
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID);
#endif
				return;
			}
		}

		WCHAR wszAccountName[IDLENMAX];
		_wcscpy( wszAccountName, _countof(wszAccountName), pPacket->wszAccountName, (int)wcslen(pPacket->wszAccountName) );

		CDNUserSession* pSession = pRoom->CreateBreakIntoGameSession( wszAccountName, pPacket->wszCharacterName, pPacket->uiAccountDBID, pPacket->uiSessionID, pPacket->biCharacterDBID, pPacket->uiTeam, 
			pPacket->cWorldSetID, pPacket->cVillageID, pPacket->bAdult, pPacket->cPCBangGrade, pPacket->BreakIntoType );
		if( pSession )
		{
			m_ConnectList[pPacket->uiAccountDBID]	= pSession;
			m_VerifyList[pPacket->uiSessionID]		= pSession;
			pSession->SetPvPFatigueOption(pPacket->bPvPFatigue);
			pSession->SetPvPUserState(pPacket->uiPvPUserState);
			pSession->SetPartyMemberIndex(pPacket->cPvPTeamIndex);
			if( m_pServerManager )
				m_pServerManager->AddGameUser( pRoom->GetRoomID(), pPacket->uiAccountDBID, pPacket->uiSessionID, this );	//Defer default setup to connection at the master connect layer.
			else
				_DANGER_POINT();
		}
		else
		{
			if( pPacket->BreakIntoType==BreakInto::Type::PartyRestore )
			{
#if defined( PRE_PARTY_DB )
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pPacket->cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID, pPacket->BreakIntoType );
#else
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pPacket->cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID );
#endif
				return;
			}
#if defined( PRE_PARTY_DB )
			else if( pPacket->BreakIntoType == BreakInto::Type::PartyJoin )
			{
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pRoom->m_iWorldID, ERROR_PARTY_JOINFAIL, pPacket->uiAccountDBID, pPacket->BreakIntoType );
				return;
			}
#endif // #if defined( PRE_PARTY_DB )

			// Cannot drop in. Notify error.
			int nReqType = REQINFO_TYPE_SINGLE;
			if( pRoom->bIsPvPRoom() )
				nReqType = pRoom->bIsLadderRoom() ? REQINFO_TYPE_LADDER_OBSERVER : REQINFO_TYPE_PVP_BREAKINTO;
			g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pRoom->m_GameTaskType, nReqType, pPacket->uiAccountDBID, GetServerID(), 0, pPacket->cVillageID, NULL );
			return;
		}

		// GM intrusion
		// In prototype, farm intrusion is treated as a user.
		if( pRoom->bIsFarmRoom() )
		{
			pSession->SetGMTrace( false );
		}
		else
		{
			pSession->SetGMTrace( pPacket->BreakIntoType==BreakInto::Type::GMTrace ? true : false );
		}

		if( g_pMasterConnectionManager )
		{
			int nReqType = REQINFO_TYPE_SINGLE;
			if (pRoom->bIsPvPRoom())		//Join mid-match if PvP
				nReqType = pRoom->bIsLadderRoom() ? REQINFO_TYPE_LADDER_OBSERVER : REQINFO_TYPE_PVP_BREAKINTO;
			else if (pRoom->GetPartyIndex() > 0)		//Party invite or GM drop-in
				nReqType = pPacket->BreakIntoType==BreakInto::Type::GMTrace ? REQINFO_TYPE_SINGLE : REQINFO_TYPE_PARTY_BREAKINTO; //If GM drops in, treat like solo (so party isn't formed when moving to village)

			if( pPacket->BreakIntoType==BreakInto::Type::PartyRestore )
			{
#if defined( PRE_PARTY_DB )
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( cWorldSetID, ERROR_NONE, pPacket->uiAccountDBID, pPacket->BreakIntoType );
#else
				g_pMasterConnectionManager->SendConfirmLastDungeonInfo( cWorldSetID, ERROR_NONE, pPacket->uiAccountDBID );
#endif
			}
			g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pRoom->m_GameTaskType, nReqType, pPacket->uiAccountDBID, GetServerID(), pRoom->GetRoomID(), pPacket->cVillageID, NULL, nReqType == REQINFO_TYPE_PARTY_BREAKINTO ? pRoom->GetPartyIndex() : 0 );
		}
		else
			_DANGER_POINT();
	}
	else
	{
		if (pPacket->BreakIntoType==BreakInto::Type::WorldZoneParty)
			g_pMasterConnectionManager->SendInvitedMemberReturnMsg(pPacket->cWorldSetID, pPacket->uiAccountDBID, ERROR_PARTY_JOINFAIL);
		else if( pPacket->BreakIntoType==BreakInto::Type::PartyRestore )
#if defined( PRE_PARTY_DB )
			g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pPacket->cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID, pPacket->BreakIntoType );
#else
			g_pMasterConnectionManager->SendConfirmLastDungeonInfo( pPacket->cWorldSetID, ERROR_CANT_EXSIT_BACKUPPARTYINFO, pPacket->uiAccountDBID );
#endif // #if defined( PRE_PARTY_DB )
	}
}

void CDNRUDPGameServer::SendRestoreMaster()
{
	if (m_RestoreMaster.empty()) return;

	m_RestoreSync.Lock();
	std::vector <tRestoreMaster>::iterator ii;
	for (ii = m_RestoreMaster.begin(); ii != m_RestoreMaster.end(); )
	{
		if ((*ii).nStep == _RESTORESTEP_SENDUSER)		//Send user.
		{
			std::map <unsigned int, CDNUserSession*>::iterator is;
			for (is = m_ConnectList.begin(); is != m_ConnectList.end(); is++)
			{
				if ((*is).second->IsConnected() && (*is).second->GetWorldSetID() == (*ii).cWorldSetID)
					g_pMasterConnectionManager->SendAddUserList((*is).second->GetWorldSetID(), (*is).second);
			}
			(*ii).nStep = _RESTORESTEP_END;
		}
		else if ((*ii).nStep == _RESTORESTEP_END)
		{
			g_pMasterConnectionManager->SendEndofVillageInfo((*ii).cWorldSetID, GetServerID());
			ii = m_RestoreMaster.erase(ii);
		}
	}
		
	m_RestoreSync.UnLock();
}

void CDNRUDPGameServer::DestroyAllGameRoom()
{
	//Change all game rooms' state to destroy and prevent further room creation.
	if (m_bIsCloseService) return;
	m_bIsCloseService = true;	//Change the flag
	std::map <unsigned int, CDNGameRoom*>::iterator ii;
	for (ii = m_GameRoomList.begin(); ii != m_GameRoomList.end(); ii++)
	{
		(*ii).second->DestroyGameRoom();
	}
}


#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNRUDPGameServer::DestroyAllFarmGameRoom( int iWorldSetID )
#else
void CDNRUDPGameServer::DestroyAllFarmGameRoom()
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
{
	std::map <unsigned int, CDNGameRoom*>::iterator ii;
	for (ii = m_GameRoomList.begin(); ii != m_GameRoomList.end(); ii++)
	{
#if defined( PRE_WORLDCOMBINE_PARTY )
		if ((*ii).second->bIsFarmRoom() && (*ii).second->GetWorldSetID() == iWorldSetID )
#else
		if ((*ii).second->bIsFarmRoom())
#endif // #if defined( PRE_WORLDCOMBINE_PARTY )
		{
			(*ii).second->DestroyGameRoom(true);
		}
	}
}

void CDNRUDPGameServer::SendSetGameRoomID(char cWorldSetID, CDNGameRoom * pRoom, MAGAReqRoomID * pReqRoomID, UINT * pPartyMember)
{
	if (pReqRoomID == NULL || pRoom == NULL)
	{
		_DANGER_POINT();
		return;
	}

#if defined( PRE_WORLDCOMBINE_PARTY )
#if defined( PRE_WORLDCOMBINE_PVP )
	bool bRet = g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pReqRoomID->GameTaskType, pReqRoomID->cReqGameIDType, pReqRoomID->InstanceID, GetServerID(), pRoom->GetRoomID(), pReqRoomID->cVillageID, pPartyMember, 0, pReqRoomID->PartyData.Type, pReqRoomID->eWorldReqType );
#else // #if defined( PRE_WORLDCOMBINE_PVP )
	bool bRet = g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pReqRoomID->GameTaskType, pReqRoomID->cReqGameIDType, pReqRoomID->InstanceID, GetServerID(), pRoom->GetRoomID(), pReqRoomID->cVillageID, pPartyMember, 0, pReqRoomID->PartyData.Type );
#endif // #if defined( PRE_WORLDCOMBINE_PVP )
#else
	bool bRet = g_pMasterConnectionManager->SendSetRoomID( cWorldSetID, pReqRoomID->GameTaskType, pReqRoomID->cReqGameIDType, pReqRoomID->InstanceID, GetServerID(), pRoom->GetRoomID(), pReqRoomID->cVillageID, pPartyMember );
#endif
	if( bRet )
	{
#if defined( STRESS_TEST )
#else
		g_Log.Log(LogType::_NORMAL, cWorldSetID, 0, 0, 0, L"Report|[RoomCnt:%d][RoomID:%d MAGA_REQROOMID] ADBID:%u PartyIndex:%d RandomSeed:%d Map:%d GateNo:%d UCNT:%d\r\n", \
			g_pGameServerManager->GetRoomCount(), pRoom->GetRoomID(), pReqRoomID->nLeaderAccountDBID, pRoom->GetPartyIndex(), \
			pReqRoomID->nRandomSeed, pReqRoomID->nMapIndex, pReqRoomID->cGateNo, pReqRoomID->cMemberCount );
#endif // #if defined( STRESS_TEST )
	}
	else
	{
		g_Log.Log(LogType::_ERROR, cWorldSetID, 0, 0, 0, L"Report|[RoomCnt:%d][RoomID:%d MAGA_REQROOMID] ADBID:%u PartyIndex:%d RandomSeed:%d Map:%d GateNo:%d UCNT:%d\r\n", \
			g_pGameServerManager->GetRoomCount(), pRoom->GetRoomID(), pReqRoomID->nLeaderAccountDBID, pRoom->GetPartyIndex(), \
			pReqRoomID->nRandomSeed, pReqRoomID->nMapIndex, pReqRoomID->cGateNo, pReqRoomID->cMemberCount );
		g_Log.Log(LogType::_GAMECONNECTLOG, cWorldSetID, 0, 0, 0, L"ErrorReport|[RoomCnt:%d][RoomID:%d MAGA_REQROOMID] ADBID:%u PartyIndex:%d RandomSeed:%d Map:%d GateNo:%d UCNT:%d\r\n", \
			g_pGameServerManager->GetRoomCount(), pRoom->GetRoomID(), pReqRoomID->nLeaderAccountDBID, pRoom->GetPartyIndex(), \
			pReqRoomID->nRandomSeed, pReqRoomID->nMapIndex, pReqRoomID->cGateNo, pReqRoomID->cMemberCount );
	}
}

void CDNRUDPGameServer::InitializePvPGameMode( const MAGAPVP_GAMEMODE* pPacket )
{
	CDNGameRoom* pRoom = GetRoom( pPacket->iGameRoomID );
	if( pRoom )
	{
		if( pRoom->bIsPvPRoom()&& pRoom->GetPvPIndex() == pPacket->uiPvPIndex )
		{
			if( pRoom->InitializePvPGameMode( pPacket ) == false )
				_ASSERT(0);
		}
		else
			_DANGER_POINT();
	}
	else
		_DANGER_POINT();
}

void CDNRUDPGameServer::RequestedVillageState( MAGAVillageState *pPacket )
{
	CDNUserSession *pSession = GetSessionByADBID(pPacket->nAccountDBID);
	if (pSession == NULL) return;

	CDNGameRoom *pRoom = GetRoom( pPacket->nRoomID );
	if( pRoom ) {
		CDnGameTask *pTask = (CDnGameTask *)pRoom->GetTaskMng()->GetTask( "GameTask" );
		if( pTask ) {
			pTask->OnRecvVillageState( pSession, pPacket );
		}
	}
}

bool CDNRUDPGameServer::_CmdRecall( MAGAResRecall* pPacket )
{
	CDNUserSession* pRecallSession = GetSessionByADBID( pPacket->uiRecallAccountDBID );
	if( !pRecallSession || !pRecallSession->GetGameRoom() )
		return false;


	CDNGameRoom::PartyStruct * pStruct = pRecallSession->GetGameRoom()->GetPartyData( pRecallSession );
	if( !pStruct )
		return false;

	if( pStruct->nEnteredGateIndex != -1 )
	{
		std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4415) ) % pRecallSession->GetCharacterName() );
		return g_pMasterConnectionManager->SendChat(pRecallSession->GetWorldSetID(), CHATTYPE_GM, pPacket->uiAccountDBID, wString.c_str(), static_cast<short>(wString.size()) );
	}
	
	CDnGameTask* pGameTask =  pRecallSession->GetGameRoom()->GetGameTask();
	if( !pGameTask )
		return false;

	if( !pGameTask->IsSyncComplete() )
	{
		std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4416) ) % pRecallSession->GetCharacterName() );
		return g_pMasterConnectionManager->SendChat(pRecallSession->GetWorldSetID(), CHATTYPE_GM, pPacket->uiAccountDBID, wString.c_str(), static_cast<short>(wString.size()) );
	}

	if( pRecallSession->GetState() != SESSION_STATE_GAME_PLAY )
	{
		std::wstring wString = boost::io::str( boost::wformat( GetEtUIXML().GetUIString(CEtUIXML::idCategory1,4417) ) % pRecallSession->GetCharacterName() % static_cast<int>(pRecallSession->GetState() ) );
		return g_pMasterConnectionManager->SendChat(pRecallSession->GetWorldSetID(), CHATTYPE_GM, pPacket->uiAccountDBID, wString.c_str(), static_cast<short>(wString.size()) );
	}

	return ( pRecallSession->CmdAbandonStage( false, true, false, pPacket->iRecallMapIndex ) == ERROR_NONE ) ? true : false;
}

#if defined( PRE_WORLDCOMBINE_PARTY )
void CDNRUDPGameServer::SendWorldPartyMember(char cWorldSetID, MAGetWorldPartyMember* pPacket)
{
	CDNGameRoom* pRoom = GetRoom( pPacket->iRoomID );
	Party::MemberData MemberData[PARTYCOUNTMAX];
	memset(MemberData, 0, sizeof(Party::MemberData)*PARTYCOUNTMAX);

	if( pRoom )
	{
		if( Party::bIsWorldCombineParty( pRoom->GetPartyType()) )
		{
			if (CDnWorld::GetInstance(pRoom).GetMapType() == GlobalEnum::eMapTypeEnum::MAP_WORLDMAP)
			{	
				int nCount = 0;
				for(DWORD i=0;i<pRoom->GetUserCount();i++)
				{
					CDNUserSession *pSession = pRoom->GetUserData(i);
					if(pSession)
					{
						MemberData[nCount].biCharacterDBID = pSession->GetCharacterDBID();
						_wcscpy(MemberData[nCount].wszCharName, _countof(MemberData[nCount].wszCharName), pSession->GetCharacterName(), (int)wcslen(pSession->GetCharacterName()));
						MemberData[nCount].cLevel = pSession->GetLevel();
						MemberData[nCount].cJob = pSession->GetUserJob();
						MemberData[nCount].bLeader = false;
						MemberData[nCount].nWorldSetID = pSession->GetWorldSetID();
						nCount++;
					}
				}
				g_pMasterConnectionManager->SendWorldPartyMember(cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, pPacket->PartyID, nCount, MemberData, ERROR_NONE);
				return;
			}		
		}		
	}

	g_pMasterConnectionManager->SendWorldPartyMember(cWorldSetID, pPacket->nAccountDBID, pPacket->biCharacterDBID, pPacket->PartyID, 0, MemberData, ERROR_PARTY_JOINFAIL);
}
#endif

#if defined( PRE_WORLDCOMBINE_PVP )

void CDNRUDPGameServer::InitialzeWorldPvPGameMode( CDNGameRoom * pRoom, MAGAReqRoomID * pReqRoomID )
{	
	SYSTEMTIME st;
	GetLocalTime( &st );
	char szSN[MAX_PATH];			

	MAGAPVP_GAMEMODE GameMode;
	memset(&GameMode, 0, sizeof(GameMode));
	GameMode.iGameRoomID = pRoom->GetRoomID();
	GameMode.uiPvPIndex = (UINT)pReqRoomID->InstanceID;
	GameMode.uiPvPGameModeTableID = pReqRoomID->MissonRoomData.cModeID;
	GameMode.uiWinCondition = pReqRoomID->MissonRoomData.nWinCondition;
	GameMode.uiPlayTimeSec = pReqRoomID->MissonRoomData.nPlayTime;
	GameMode.unGameModeCheck = PvPCommon::Check::AllCheck;
	sprintf( szSN, "%.4d%.2d%.2d%.2d%.2d%.2d%.3d%", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds );
	GameMode.biSNMain = _atoi64( szSN );
	sprintf( szSN, "%.2d%.2d%.4d", pReqRoomID->cWorldSetID, pReqRoomID->cVillageID, pReqRoomID->InstanceID );
	GameMode.iSNSub = atoi( szSN );
	GameMode.cIsPWRoom = 0;
	GameMode.unRoomOptionBit = pReqRoomID->MissonRoomData.unRoomOptionBit;		
	
	GameMode.cMaxUser = pReqRoomID->MissonRoomData.cMaxPlayers;		
	GameMode.bIsGuildWar =	false;
#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	GameMode.cPvPChannelType = pReqRoomID->MissonRoomData.nChannelType;
#endif		//#ifdef PRE_ADD_COLOSSEUM_BEGINNER
	GameMode.nStartPlayer = pReqRoomID->MissonRoomData.nStartPlayers;
	InitializePvPGameMode(&GameMode);	
}
#endif