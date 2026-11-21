
#include "stdafx.h"
#include "DNPvPGameRoom.h"
#include "DNUserSession.h"
#include "DNBreakIntoUserSession.h"
#include "DnActor.h"
#include "DnPlayerActor.h"
#include "TaskManager.h"
#include "DnPvPPartyTask.h"
#include "DnPvPGameTask.h"
#include "DNMasterConnection.h"
#include "DNMasterConnectionManager.h"
#include "GameSendPacket.h"
#include "DnBlow.h"
#include "DNGameDataManager.h"
#include "PvPRespawnMode.h"
#include "PvPRoundMode.h"
#include "PvPScoreSystem.h"
#include "PvPRespawnLogic.h"
#include "DnDropItem.h"
#include "DnItemTask.h"
#include "DNLogConnection.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#include "PvPCaptainMode.h"
#include "PvPIndividualRespawnMode.h"
#include "PvPZombieMode.h"
#include "PvPOccupationMode.h"
#include "PvPOccupationSystem.h"
#include "DnSkill.h"
#include "LadderStats.h"
#include "PvPAllKillMode.h"
#if defined(PRE_ADD_RACING_MODE)
#include "PvPRacingMode.h"
#endif //#if defined(PRE_ADD_RACING_MODE)
#if defined(PRE_ADD_PVP_TOURNAMENT)
#include "PvPTournamentMode.h"
#endif // #if defined(PRE_ADD_PVP_TOURNAMENT)
#if defined(PRE_ADD_REVENGE)
#include "RevengeSystem.h"
#endif
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
#include "PvPComboExerciseMode.h"
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

CDNPvPGameRoom::CDNPvPGameRoom( CDNRUDPGameServer* pServer, unsigned int iRoomID, MAGAReqRoomID* pPacket )
:CDNGameRoom( pServer, iRoomID, pPacket )
,m_biSNMain(0),m_iSNSub(0),m_bIsPWRoom(false),m_cMaxUser(0),m_pPvPGameMode(NULL)
{
	//g_Log.LogA( "PvP room created!!!\r\n" );
	m_pLadderStatsRepository	= NULL;
	m_bIsLadderRoom				= false;
	m_biLadderRoomIndexArr[0]	= pPacket->biLadderRoomIndexArr[0];
	m_biLadderRoomIndexArr[1]	= pPacket->biLadderRoomIndexArr[1];
	m_unRoomOptionBit = PvPCommon::RoomOption::None;
	for (int i = 0; i < PvPCommon::Common::DefaultGuildCount; i++)
	{
		m_nGuildDBID[i] = pPacket->nGuildDBID[i];
		m_nGuildQualifyingScore[i] = pPacket->nGuildQualifyingScore[i];
	}
	for( int i=0; i<PvPCommon::Common::MaxPlayer; ++i ) // Find room master CharacterDBID
	{
		if( pPacket->arrMemberInfo[i].MemberAccountDBID == pPacket->nLeaderAccountDBID )
		{
			m_biRoomMasterCharacterDBID = pPacket->arrMemberInfo[i].biMemberCharacterDBID;
#if defined(PRE_ADD_PVP_COMBOEXERCISE)
			m_uiRoomMasterSessionID		= pPacket->arrMemberInfo[i].MemberSessionID;
#endif //#if defined(PRE_ADD_PVP_COMBOEXERCISE)
			break;
		}
	}	
	m_biCreateRoomCharacterDBID = pPacket->biCreateRoomCharacterDBID;
#if defined( PRE_WORLDCOMBINE_PVP )
	m_bWorldPvPRoomStart = false;
	m_nWorldPvPRoomDBIndex = 0;
#endif
#if defined( PRE_PVP_GAMBLEROOM )
	m_cGambleType = pPacket->cGambleType;
	m_nGamblePrice = pPacket->nGamblePrice;
#endif
}

CDNPvPGameRoom::~CDNPvPGameRoom()
{
	if (m_nEventRoomIndex > 0)
		g_Log.Log(LogType::_PVPROOM, L"PVP Destroyed [Index:%d][Room:%d][Event:%d][MID:%d] \r\n", GetPvPIndex(), GetRoomID(), m_nEventRoomIndex, g_Config.nManagedID);

	//g_Log.LogA( "PvP room destroyed!!!\r\n" );
	if( g_pMasterConnectionManager )
	{
		if( bIsLadderRoom() )
			g_pMasterConnectionManager->SendPvPLadderRoomSync( m_iWorldID, m_biLadderRoomIndexArr[0], m_biLadderRoomIndexArr[1], LadderSystem::RoomState::GameRoomDestroy );
		else
			g_pMasterConnectionManager->SendPvPCommand( m_iWorldID, PvPCommon::GAMA_Command::DeleteGameRoom, GetPvPIndex(), GetRoomID() );
	}
	else
		_DANGER_POINT();

	// PvP end log
	BYTE cThreadID;
	CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
	if( pDBCon )
	{
		INT64 biSNMain = 0;
		int nSNSub = 0;
		GetPvPSN(biSNMain, nSNSub);
		pDBCon->QueryAddPvPEndLog( cThreadID, GetWorldSetID(), 0, biSNMain, nSNSub );
	}

	SAFE_DELETE( m_pPvPGameMode );
	SAFE_DELETE( m_pLadderStatsRepository );
}


float CDNPvPGameRoom::GetDieDelta()
{
	if( m_pPvPGameMode && m_pPvPGameMode->GetPvPGameModeTable() )
	{
		if (m_pPvPGameMode->GetOccupationSystem() && m_pPvPGameMode->GetOccupationSystem()->GetRespawnSec() > 0)
			return static_cast<float>(m_pPvPGameMode->GetOccupationSystem()->GetRespawnSec());

		return static_cast<float>(m_pPvPGameMode->GetPvPGameModeTable()->uiRespawnTimeSec);
	}

	return CDnActor::s_fDieDelta;
}

void CDNPvPGameRoom::OnSendPartyMemberInfo( CDNUserSession* pBreakIntoSession )
{
	if( pBreakIntoSession )
	{
		CDNGameRoom::OnSendPartyMemberInfo( pBreakIntoSession );
	}
	else
	{
		for( UINT i=0; i<m_VecMember.size() ; ++i )
		{
			SCROOM_SYNC_MEMBERINFO TxPacket;
			memset( &TxPacket, 0, sizeof(TxPacket) );

			TxPacket.bIsBreakInto	= false;
			TxPacket.bIsStart		= true;

			UINT uiSendCount = 0;

			for( UINT j=0 ; j<m_VecMember.size() ; ++j )
			{
				m_VecMember[j].pSession->GetPartyMemberInfo( TxPacket.Member[j-uiSendCount] );
				++TxPacket.nCount;

				// Page completed
				if( TxPacket.nCount%_countof(TxPacket.Member) == 0 )
				{
					m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

					uiSendCount		   += _countof(TxPacket.Member);
					TxPacket.nCount		= 0;
					TxPacket.bIsStart	= false;
				}
			}

			if( TxPacket.nCount > 0 )
			{
				int iSize = sizeof(TxPacket)-sizeof(TxPacket.Member)+TxPacket.nCount*sizeof(TxPacket.Member[0]);

				m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERINFO, reinterpret_cast<char*>(&TxPacket), iSize );
			}
		}
	}
}

void CDNPvPGameRoom::OnSendTeamData( CDNUserSession* pBreakIntoSession )
{
	SCROOM_SYNC_MEMBERTEAM TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	if( pBreakIntoSession )
	{
		UINT uiSendCount = 0;
		
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
#ifdef PRE_MOD_SYNCPACKET
			TxPacket.tTeam[TxPacket.cCount].nSessionID = m_VecMember[i].pSession->GetSessionID();
			TxPacket.tTeam[TxPacket.cCount].usTeam = m_VecMember[i].pSession->GetTeam();
#else		//#ifdef PRE_MOD_SYNCPACKET
			TxPacket.usTeam[TxPacket.cCount] = m_VecMember[i].pSession->GetTeam();
#endif		//#ifdef PRE_MOD_SYNCPACKET
			++TxPacket.cCount;

			// Page completed			
#ifdef PRE_MOD_SYNCPACKET
			if( TxPacket.cCount% _countof(TxPacket.tTeam) == 0 )
			{
				pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

				uiSendCount		+= _countof(TxPacket.tTeam);
				TxPacket.cCount  = 0;
			}
#else		//#ifdef PRE_MOD_SYNCPACKET
			if( TxPacket.cCount% _countof(TxPacket.usTeam) == 0 )
			{
				TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);

				pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

				uiSendCount		+= _countof(TxPacket.usTeam);
				TxPacket.cCount  = 0;
			}
#endif		//#ifdef PRE_MOD_SYNCPACKET
		}

		if( TxPacket.cCount > 0 )
		{
#ifdef PRE_MOD_SYNCPACKET
			int iSize = sizeof(TxPacket)-sizeof(TxPacket.tTeam)+TxPacket.cCount*sizeof(TSyncTeam);
#else		//#ifdef PRE_MOD_SYNCPACKET
			TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);
			int iSize = sizeof(TxPacket)-sizeof(TxPacket.usTeam)+TxPacket.cCount*sizeof(TxPacket.usTeam[0]);
#endif		//#ifdef PRE_MOD_SYNCPACKET			

			pBreakIntoSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), iSize );
		}
	}
	else
	{
		for( UINT i=0 ; i<m_VecMember.size() ; ++i )
		{
			UINT uiSendCount = 0;
			memset( &TxPacket, 0, sizeof(TxPacket) );

			for( UINT j=0 ; j<m_VecMember.size() ; ++j )
			{
#ifdef PRE_MOD_SYNCPACKET
				TxPacket.tTeam[TxPacket.cCount].nSessionID = m_VecMember[j].pSession->GetSessionID();
				TxPacket.tTeam[TxPacket.cCount].usTeam = m_VecMember[j].pSession->GetTeam();
#else		//#ifdef PRE_MOD_SYNCPACKET
				TxPacket.usTeam[TxPacket.cCount] = m_VecMember[j].pSession->GetTeam();
#endif		//#ifdef PRE_MOD_SYNCPACKET
				++TxPacket.cCount;

				// Page completed				
#ifdef PRE_MOD_SYNCPACKET
				if( TxPacket.cCount% _countof(TxPacket.tTeam) == 0 )
				{
					m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

					uiSendCount		+= _countof(TxPacket.tTeam);
					TxPacket.cCount  = 0;
				}
#else		//#ifdef PRE_MOD_SYNCPACKET
				if( TxPacket.cCount% _countof(TxPacket.usTeam) == 0 )
				{
					TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);
					m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );

					uiSendCount		+= _countof(TxPacket.usTeam);
					TxPacket.cCount  = 0;
				}
#endif		//#ifdef PRE_MOD_SYNCPACKET
			}

			if( TxPacket.cCount > 0 )
			{
#ifdef PRE_MOD_SYNCPACKET
				int iSize = sizeof(TxPacket)-sizeof(TxPacket.tTeam)+TxPacket.cCount*sizeof(TSyncTeam);
				m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), iSize );
#else		//#ifdef PRE_MOD_SYNCPACKET
				TxPacket.unStartIndex	= static_cast<USHORT>(uiSendCount);
				int iSize = sizeof(TxPacket)-sizeof(TxPacket.usTeam)+TxPacket.cCount*sizeof(TxPacket.usTeam[0]);

				m_VecMember[i].pSession->AddSendData( SC_ROOM, eRoom::SC_SYNC_MEMBERTEAM, reinterpret_cast<char*>(&TxPacket), iSize );
#endif		//#ifdef PRE_MOD_SYNCPACKET
			}
		}
	}
}

void CDNPvPGameRoom::OnSetPlayState()
{
	CDNGameRoom::OnSetPlayState();
	if( bIsLadderRoom() )
	{
			m_pLadderStatsRepository->OnSetPlayState();

		// Sync LadderRoom State to master server
		g_pMasterConnectionManager->SendPvPLadderRoomSync(m_iWorldID, m_biLadderRoomIndexArr[0], m_biLadderRoomIndexArr[1], LadderSystem::RoomState::Playing );
	}
	else
	{
		// Notify PvP RoomState to master server
#if defined( PRE_WORLDCOMBINE_PVP )
		if( bIsWorldPvPRoom() == false )
			g_pMasterConnectionManager->SendPvPRoomSyncOK(m_iWorldID, GetPvPIndex());
#else
		g_pMasterConnectionManager->SendPvPRoomSyncOK(m_iWorldID, GetPvPIndex());
#endif
	}
}

void CDNPvPGameRoom::OnDelPartyMember( UINT iDelMemberSessionID, char cKickKind )
{
	if( m_pTaskMng )
	{
		CDnPartyTask* pPartyTask = (CDnPartyTask*)(m_pTaskMng->GetTask("PartyTask"));
		if( pPartyTask )
			pPartyTask->OutPartyMember( iDelMemberSessionID, 0, cKickKind );
	}
}

bool CDNPvPGameRoom::bIsPvPStart()
{ 
	if( m_pPvPGameMode && !m_pPvPGameMode->bIsFinishFlag() )
		return m_pPvPGameMode->GetRemainStartTick() > 0 ? false : true;
	return false;
}

void CDNPvPGameRoom::OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage )
{
	if( m_pPvPGameMode )
		m_pPvPGameMode->OnDamage( hActor, hHitter, iDamage );
}

void CDNPvPGameRoom::OnRebirth( DnActorHandle hActor )
{
	if( m_pPvPGameMode )
		m_pPvPGameMode->OnRebirth( hActor );

	if(bIsIgnorePrefixSkill() && hActor )
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_183, -1, NULL );	// Cannot use prefix-related features
}

void CDNPvPGameRoom::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	if (!hActor)
		return;

	if (hActor->IsPlayerActor())
	{
		// 1.SC_PLAYERDIE
		BYTE pBuffer[128];
		CPacketCompressStream Stream( pBuffer, 128 );

		DWORD dwHitterUniqueID = hHitter ? hHitter->GetUniqueID() : -1;
		Stream.Write( &dwHitterUniqueID, sizeof(DWORD) );

		hActor->Send( eActor::SC_PLAYERDIE, &Stream );

		// 2.DropItem
		_OnDropItem( hActor );

		float fDieDelta = GetDieDelta();
		hActor->SetMaxDieDelta( fDieDelta );
		hActor->SetDieDelta( fDieDelta );

		if( bIsLadderRoom() )
			m_pLadderStatsRepository->OnDie( hActor, hHitter );
	}

	if( m_pPvPGameMode )
		m_pPvPGameMode->OnDie( hActor, hHitter );
}

void CDNPvPGameRoom::OnCmdAddStateEffect( const CDnSkill::SkillInfo* pSkillInfo )
{
	if( m_pPvPGameMode )
		m_pPvPGameMode->OnCmdAddStateEffect( pSkillInfo );
}

void CDNPvPGameRoom::OnLeaveUser( const UINT uiSessionID )
{
	if( m_pPvPGameMode )
	{
		int iSeq;
		CDNGameRoom::PartyStruct* pStruct = GetPartyDatabySessionID( uiSessionID, iSeq );
		if( pStruct )
		{
			bool bQuery = true;
			DnActorHandle hActor = pStruct->pSession ? pStruct->pSession->GetActorHandle() : CDnActor::Identity();
			if( hActor && hActor->GetTeam() == PvPCommon::Team::Observer )
				bQuery = false;
			IScoreSystem* pScoreSystem = m_pPvPGameMode->GetScoreSystem();
			if( bQuery && pScoreSystem )
				pScoreSystem->QueryUpdatePvPData( PvPCommon::Team::Max, pStruct->pSession, PvPCommon::QueryUpdatePvPDataType::Disconnect );

			m_pPvPGameMode->OnLeaveUser( hActor );
		}
	}
}

// Called in ProcessDie() - resurrection timing
void CDNPvPGameRoom::OnFinishProcessDie( DnActorHandle hActor )
{
	if( m_pPvPGameMode )
		m_pPvPGameMode->OnFinishProcessDie( hActor );
}

bool CDNPvPGameRoom::bIsOccupationMode()
{
	if (GetPvPGameMode())
		return GetPvPGameMode()->bIsOccupationMode();
	return false;
}

bool CDNPvPGameRoom::bIsGuildWarSystem()
{
	if (GetPvPGameMode())
		return GetPvPGameMode()->bIsGuildWarSystem();
	return false;
}

void CDNPvPGameRoom::GetGuildWarInfoByIndex(int nIndex, UINT &nGuildDBID, int &nScore)
{
	if (nIndex < 0 || nIndex >= PvPCommon::Common::DefaultGuildCount)
	{
		_DANGER_POINT();
		return;
	}

	nGuildDBID = m_nGuildDBID[nIndex];
	nScore = m_nGuildQualifyingScore[nIndex];
}

void CDNPvPGameRoom::OnSendPvPLobby()
{
	m_bForceDestroyRoom = true;
	CDNUserSession * pSession;
	for (UINT i = 0 ; i < GetUserCount(); ++i)
	{
		pSession = GetUserData(i);
		if (pSession)
			pSession->SendPvPGameToPvPLobby();
	}
}

bool CDNPvPGameRoom::bIsLevelRegulation()
{
	if( m_unRoomOptionBit&PvPCommon::RoomOption::NoRegulation )
		return false;

	if( m_pPvPGameMode )
	{
		const TPvPGameModeTable* pTable = m_pPvPGameMode->GetPvPGameModeTable();
		if( pTable )
			return pTable->bIsLevelRegulation;
	}

	return false;
}

bool CDNPvPGameRoom::InitializePvPGameMode( const MAGAPVP_GAMEMODE* pPacket )
{
	if (pPacket->nEventID > 0)
		g_Log.Log(LogType::_PVPROOM, L"InitializePvPGameMode [Index:%d][Event:%d] \r\n", pPacket->uiPvPIndex, pPacket->nEventID);

	if( m_pPvPGameMode )
	{
		_DANGER_POINT();
		return false;
	}

	if( !g_pDataManager )
	{
		_DANGER_POINT();
		return false;
	}

	// Set S/N
	m_biSNMain	= pPacket->biSNMain;
	m_iSNSub	= pPacket->iSNSub;

	// Set room info
	m_nEventRoomIndex	= pPacket->nEventID;
	m_bIsLadderRoom		= pPacket->bIsLadderRoom;
	m_bIsPWRoom			= (pPacket->cIsPWRoom == 1) ? true : false;
	m_unRoomOptionBit	= pPacket->unRoomOptionBit;
	m_cMaxUser			= pPacket->cMaxUser;


	const TPvPGameModeTable* pPvPGameModeTable = g_pDataManager->GetPvPGameModeTable( pPacket->uiPvPGameModeTableID );
	if( pPvPGameModeTable )
	{	

		switch( pPvPGameModeTable->uiGameMode )
		{
		
			case PvPCommon::GameMode::PvP_Respawn:
			{
				m_pPvPGameMode = new IBoostPoolPvPRespawnMode( this, pPvPGameModeTable, pPacket );
				break;
			}
			case PvPCommon::GameMode::PvP_Round:
			{
				m_pPvPGameMode = new IBoostPoolPvPRoundMode( this, pPvPGameModeTable, pPacket );
				break;
			}
			case PvPCommon::GameMode::PvP_Captain:
			{
				m_pPvPGameMode = new CPvPCaptainMode( this, pPvPGameModeTable, pPacket );
				break;
			}
			case PvPCommon::GameMode::PvP_IndividualRespawn:
			{
				m_pPvPGameMode = new CPvPIndividualRespawnMode( this, pPvPGameModeTable, pPacket );
				break;
			}
			case PvPCommon::GameMode::PvP_Zombie_Survival:
			{
				m_pPvPGameMode = new CPvPZombieMode( this, pPvPGameModeTable, pPacket );
				break;
			}
			case PvPCommon::GameMode::PvP_GuildWar:
			case PvPCommon::GameMode::PvP_Occupation:
				{
					m_pPvPGameMode = new CPvPOccupationMode( this, pPvPGameModeTable, pPacket );
					break;
				}
			case PvPCommon::GameMode::PvP_AllKill:
			{
				m_pPvPGameMode = new CPvPAllKillMode( this, pPvPGameModeTable, pPacket );
				break;
			}
#if defined( PRE_ADD_RACING_MODE)
			case PvPCommon::GameMode::PvP_Racing :
				{
					m_pPvPGameMode = new CPvPRacingMode( this, pPvPGameModeTable, pPacket);
					break;
				}
#endif //#if defined( PRE_ADD_RACING_MODE)
#if defined( PRE_ADD_PVP_TOURNAMENT)
			case PvPCommon::GameMode::PvP_Tournament:
				{
					m_pPvPGameMode = new CPvPTournamentMode( this, pPvPGameModeTable, pPacket);
					break;
				}
#endif //#if defined( PRE_ADD_PVP_TOURNAMENT)
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
			case PvPCommon::GameMode::PvP_ComboExercise:
				{
					m_pPvPGameMode = new CPvPComboExerciseMode( this, pPvPGameModeTable, pPacket );
					break;
				}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )			
			default:
			{
				DN_RETURN(false);
			}
		}

		if( !m_pPvPGameMode->InitializeBase( pPacket->uiWinCondition ) )
		{
			_DANGER_POINT();
			DN_RETURN(false);
		}
	}
	else
	{
		_DANGER_POINT();
		DN_RETURN(false);
	}

	bool bShuffleTeam = false;
	if( GetEventRoomIndex() > 0 )
		bShuffleTeam = true;
	if( m_unRoomOptionBit&PvPCommon::RoomOption::RandomTeam )
		bShuffleTeam = true;

	if( bShuffleTeam == true )
		_ShuffleTeam();

	if( bIsLadderRoom() )
	{
			m_pLadderStatsRepository = LadderSystem::CStatsFactory::CreateRepository( this );
			if( m_pLadderStatsRepository == NULL )
				return false;
	}

	if (m_nEventRoomIndex > 0)
		g_Log.Log(LogType::_PVPROOM, L"PVP Init [Index:%d][Room:%d][Event:%d][MID:%d] \r\n", GetPvPIndex(), GetRoomID(), m_nEventRoomIndex, g_Config.nManagedID);

	if(GetRoomState() == _GAME_STATE_PVP_SYNC2GAMEMODE)		//If state is waiting for game mode, transition to sync state.
		SetSync2SyncStateTemplateMethod(timeGetTime());

	return true;
}

// State effect applied when PvP starts
void CDNPvPGameRoom::CmdPvPStartAddStateEffect( DnActorHandle hActor, const int iTick, const bool bAddForce/*=false*/ )
{
	if( !hActor )
		return;

	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_070, iTick, NULL, bAddForce );	// Cannot move
	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_071, iTick, NULL, bAddForce );	// Cannot act	
	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_099, iTick, "-1", bAddForce );	// Invincible

	if(bIsIgnorePrefixSkill())
		hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_183, -1, NULL, bAddForce );	// Cannot use prefix-related features
	CmdObserverAddStateEffect( hActor, bAddForce );

	m_pPvPGameMode->OnCmdPvPStartAddStateEffect( hActor );
}

void CDNPvPGameRoom::CmdObserverAddStateEffect( DnActorHandle hActor, const bool bAddForce/*=false*/ )
{
	if( !hActor || !hActor->bIsObserver() )
		return;

	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_099, -1, "-1", bAddForce );		// Invincible
	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_071, -1, NULL, bAddForce );		// Cannot act
	hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_120, -1, NULL, bAddForce );		// Cannot use item
}

void CDNPvPGameRoom::CmdPvPOccupationAddStateEffect(CDnSkill::SkillInfo * pSkillInfo, DnActorHandle hActor, int nBLow, const int iTick, const char * pVal, const bool bAddForce/* = false*/)
{
	if( !hActor || !pVal || pSkillInfo == NULL || hActor->IsDie() )
		return;	

	hActor->CmdAddStateEffect( pSkillInfo, (STATE_BLOW::emBLOW_INDEX)nBLow, iTick, pVal, bAddForce );

	// Apply move speed / frame rate boost to mounts as well. 
	if((nBLow == STATE_BLOW::BLOW_025 || nBLow == STATE_BLOW::BLOW_076 ) && hActor && hActor->IsPlayerActor())
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());

		if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor() )
			pPlayer->GetMyVehicleActor()->CmdAddStateEffect( pSkillInfo, (STATE_BLOW::emBLOW_INDEX)nBLow, iTick, pVal, bAddForce );
	}
}

void CDNPvPGameRoom::CmdPvPOccupationRemoveStateEffect(DnActorHandle hActor, int nBLow)
{
	//hActor->CmdRemoveStateEffect((STATE_BLOW::emBLOW_INDEX)nBLow);
	hActor->CmdRemoveStateEffectImmediately((STATE_BLOW::emBLOW_INDEX)nBLow);
	hActor->SendRemoveStateEffect((STATE_BLOW::emBLOW_INDEX)nBLow);

	// Apply move speed / frame rate boost to mounts as well. 
	if((nBLow == STATE_BLOW::BLOW_025 || nBLow == STATE_BLOW::BLOW_076 ) && hActor && hActor->IsPlayerActor())
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hActor.GetPointer());

		if(pPlayer && pPlayer->IsVehicleMode() && pPlayer->GetMyVehicleActor() )
			pPlayer->RemoveVehicleStateEffectImmediately( (STATE_BLOW::emBLOW_INDEX)nBLow );
	}
}

void CDNPvPGameRoom::OnRequestSyncStartMsg( CDNUserSession* pGameSession )
{
	if( m_pPvPGameMode )
	{
		UINT uiRemainStartTick = m_pPvPGameMode->GetPvPStartStateEffectDurationTick();
		if( uiRemainStartTick )
		{
			// PvP room applies state effects on initial start.
			CmdPvPStartAddStateEffect( pGameSession->GetActorHandle(), uiRemainStartTick );
		}
		else
		{
			if(bIsIgnorePrefixSkill())
			{
				DnActorHandle hActor = pGameSession->GetActorHandle();
				if( hActor )
					hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_183, -1, NULL, false );	// Cannot use prefix-related features
			}

		}
	}
	else
	{
		_DANGER_POINT();
	}

	CDNGameRoom::OnRequestSyncStartMsg( pGameSession );
}

void CDNPvPGameRoom::OnSuccessBreakInto( CDNUserSession* pSession )
{
	if( m_pPvPGameMode )
		m_pPvPGameMode->OnSuccessBreakInto( pSession );
}

void CDNPvPGameRoom::OnSuccessBreakInto( std::list<CDNUserSession*>::iterator& itor )
{
	// Notify master server of drop-in success	
	if( g_pMasterConnectionManager )
	{
		g_pMasterConnectionManager->SendPvPBreakIntoOK( GetWorldSetID(), GetPvPIndex(), (*itor)->GetAccountDBID() );
	}
	else
		_DANGER_POINT();

	_SuccessBreakInto( *itor );
	itor = m_BreakIntoUserList.erase( itor );
}

void CDNPvPGameRoom::_SuccessBreakInto( CDNUserSession* pBreakIntoGameSession )
{
	for( UINT i=0 ; i<m_VecMember.size() ; ++i )
	{
		CDNUserSession* pSession = m_VecMember[i].pSession;
		if( pSession == pBreakIntoGameSession )
			continue;

		if( pSession && pSession->GetState() == SESSION_STATE_GAME_PLAY )
		{
			pSession->SendSuccessBreakInto( pBreakIntoGameSession->GetSessionID() );
		}
	}
}

void CDNPvPGameRoom::_OnDropItem( DnActorHandle hDieActor )
{
	_ASSERT( m_pPvPGameMode );
	_ASSERT( g_pDataManager );
	_ASSERT( m_pItemTask );
	
	if( !hDieActor )
		return;

	if( !bIsDropItemRoom() )
		return;

	const TPvPGameModeTable* pPvPGameModeTable = m_pPvPGameMode->GetPvPGameModeTable();
	if( !pPvPGameModeTable )
	{
		_DANGER_POINT();
		return;
	}

	TItemDropData* pItemDropData = g_pDataManager->GetItemDropData( pPvPGameModeTable->uiItemDropTableID );
	if( !pItemDropData )
		return;

	RequestItemDropTable( pPvPGameModeTable->uiItemDropTableID, hDieActor->GetPosition() );
}

void CDNPvPGameRoom::SendSelectCaptain( DnActorHandle hActor )
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		GetUserData(i)->SendPvPSelectCaptain( hActor->GetSessionID(), hActor->GetName() );
	}
}

void CDNPvPGameRoom::SendSelectZombie( std::map<DWORD,DnActorHandle>& mZombie )
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		GetUserData(i)->SendPvPSelectZombie( mZombie );
	}
}

void CDNPvPGameRoom::SendSelectZombie( DnActorHandle hActor, int nTableID, bool bZombie, bool bRemoveStateBlow, int iScale/*=100*/ )
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		GetUserData(i)->SendPvPSelectZombie( hActor, nTableID, bZombie, bRemoveStateBlow, iScale );
	}
}

void CDNPvPGameRoom::SendMemberGrade(USHORT nTeam, UINT uiUserState, UINT nChangedSessionID, int nRetCode)
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		if (GetUserData(i)->GetTeam() == nTeam)
		{
			if (nChangedSessionID == GetUserData(i)->GetSessionID())
				GetUserData(i)->SetPvPUserState(uiUserState);
			GetUserData(i)->SendPvPGuildWarMemberGrade(uiUserState, nChangedSessionID, nRetCode);
		}
	}
}

void CDNPvPGameRoom::SendMemberIndex(USHORT nTeam, BYTE cCount, const TSwapMemberIndex * pIndex, int nRetCode)
{
	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		//if (GetUserData(i)->GetTeam() == nTeam)
			GetUserData(i)->SendPvPMemberIndex(nTeam, cCount, pIndex, nRetCode);
	}
}

void CDNPvPGameRoom::_ShuffleTeam()
{
	// Shuffle is not needed for individual match.
	if( GetPvPGameMode()->GetPvPGameModeTable()->uiGameMode == PvPCommon::GameMode::PvP_IndividualRespawn )
		return;
	if( GetPvPGameMode()->GetPvPGameModeTable()->uiGameMode == PvPCommon::GameMode::PvP_Zombie_Survival )
		return;
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
	if( GetPvPGameMode()->GetPvPGameModeTable()->uiGameMode == PvPCommon::GameMode::PvP_ComboExercise )
		return;
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

	std::vector<UINT> vTeamList;
	vTeamList.reserve( GetUserCount() );

	for( UINT i=0 ; i<GetUserCount() ; ++i )
		vTeamList.push_back( i );

	srand (unsigned (time (NULL)));
	random_shuffle (vTeamList.begin(), vTeamList.end());

	UINT	uiATeam		= 0;
	UINT	uiBTeam		= 0;

	_ASSERT( vTeamList.size() == GetUserCount() );

	for( UINT i=0 ; i<vTeamList.size() ; ++i )
	{
		CDNUserSession* pSession = GetUserData(vTeamList[i]);
		if( pSession && pSession->GetTeam() != PvPCommon::Team::Observer )
		{
			if (uiATeam <= uiBTeam)
			{
				pSession->SetTeam (PvPCommon::Team::A);
				uiATeam++;
			}
			else
			{
				pSession->SetTeam (PvPCommon::Team::B);
				uiBTeam++;
			}
		}
	}
}


UINT CDNPvPGameRoom::GetPvPGameModeCode()
{
	if( !m_pPvPGameMode ) return 0;
	if( !m_pPvPGameMode->GetPvPGameModeTable() ) return 0;
	return m_pPvPGameMode->GetPvPGameModeTable()->uiGameMode;
}

//GuildWar
void CDNPvPGameRoom::QueryAddGuildWarFinalResults(UINT nGuildDBID, char cMatchResultCode)
{
	short wScheduleID = 0;
	char cMatchSequence = 0;
	char cMatchTypeCode = 0;
	g_pMasterConnectionManager->GetGuildWarSeqInfo(GetWorldSetID(), wScheduleID, cMatchSequence, cMatchTypeCode);

	if (GetDBConnection())
		GetDBConnection()->QueryAddGuildWarFinalResults(GetDBThreadID(), GetWorldSetID(), nGuildDBID, wScheduleID, cMatchSequence, cMatchTypeCode, cMatchResultCode);
}

void CDNPvPGameRoom::SendAllKillShowSelectPlayer()
{
	if( GetPvPGameMode()->bIsAllKillMode() == false )
		return;

	DnActorHandle hPreWinActor = static_cast<CPvPAllKillMode*>(GetPvPGameMode())->GetPreWinActor();

	SCPVP_ALLKILL_SHOW_SELECTPLAYER TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetUserData(i);
		if( pSession == NULL )
			continue;

		bool bSelectPlayer = true;
		if( pSession->GetTeam() == PvPCommon::Team::Observer )
		{
#ifdef PRE_MOD_PVPOBSERVER
			bSelectPlayer = true;
#else
			bSelectPlayer = false;
#endif // PRE_MOD_PVPOBSERVER
		}
		else
		{
			DnActorHandle hActor = pSession->GetActorHandle();
			if( !hActor )
				continue;
			if( GetPvPGameMode()->bIsPlayingUser( hActor ) == false )
				continue;

			if( hPreWinActor )
			{
				if( hActor->GetTeam() == hPreWinActor->GetTeam() )
					bSelectPlayer = false;
			}
		}

		TxPacket.bSelectPlayer = bSelectPlayer;

		pSession->AddSendData( SC_PVP, ePvP::SC_PVP_ALLKILL_SHOW_SELECTPLAYER, reinterpret_cast<char*>(&TxPacket), sizeof(TxPacket) );
	}
}

#if defined(PRE_ADD_REVENGE)
void CDNPvPGameRoom::BroadcastRevengeSuccess( UINT uiSessionID, UINT uiRevengeTargetSessionID )
{
	SCPvPSuccessRevenge packet;
	packet.uiSessionID	= uiSessionID;
	packet.uiRevengeTargetSessionID	= uiRevengeTargetSessionID;

	for( UINT i=0 ; i<GetUserCount() ; ++i )
	{
		CDNUserSession* pSession = GetUserData(i);
		if( pSession )
			pSession->AddSendData( SC_PVP, ePvP::SC_PVP_SUCCESS_REVENGE, reinterpret_cast<char*>(&packet), sizeof(packet) );
	}
}
//Reset revenge if leaving PvP room before normal end
void CDNPvPGameRoom::OnLeaveRoomBeforeFinish(UINT uiSessionID)
{
	if( GetEventRoomIndex() > 0 && m_pPvPGameMode && m_pPvPGameMode->GetRevengeSystem() )
	{
		m_pPvPGameMode->GetRevengeSystem()->LeaveUser(uiSessionID);
	}
}
#endif

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
void CDNPvPGameRoom::ChangeRoomMaster( INT64 biRoomMasterCharacterDBID, UINT uiRoomMasterSessionID )
{
	m_biRoomMasterCharacterDBID = biRoomMasterCharacterDBID;
	m_uiRoomMasterSessionID		= uiRoomMasterSessionID;

	if( NULL == m_pPvPGameMode )
		return;

	CDNUserSession * pRoomMasterSession = GetUserSession( uiRoomMasterSessionID );
	if( NULL == pRoomMasterSession )
		return;

	if( m_pPvPGameMode->bIsComboExerciseMode() )
	{
		for( UINT i=0 ; i<GetUserCount() ; ++i )
		{
			CDNUserSession* pSession = GetUserData(i);
			if( pSession == NULL )
				continue;

			pSession->SendPvPComboExerciseChangeRoomMaster( uiRoomMasterSessionID );
		}
	}
}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )
