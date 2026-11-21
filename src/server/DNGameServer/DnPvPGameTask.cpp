
#include "stdafx.h"
#include "DnPvPGameTask.h"
#include "EtWorldSector.h"
#include "EtWorldEventControl.h"
#include "EtWorldEventArea.h"
#include "DNUserSession.h"
#include "DnActorClassDefine.h"
#include "DnPlayerActor.h"
#include "DnItemTask.h"
#include "DnStateBlow.h"
#include "PvPGameMode.h"
#include "PvPRespawnLogic.h"
#include "PvPIndividualRespawnMode.h"
#include "DNPvPGameRoom.h"
#include "DNGameDataManager.h"
#include "PvPOccupationTeam.h"
#include "PvPOccupationSystem.h"
#if defined( PRE_ADD_PVP_COMBOEXERCISE )
#include "DnMonsterActor.h"
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )

CDnPvPGameTask::CDnPvPGameTask( CDNGameRoom* pRoom )
:CDnGameTask( pRoom ),m_pRespawnLogic(new CPvPRespawnLogic( pRoom ))
{
	m_nGameTaskType = GameTaskType::PvP;
}

CDnPvPGameTask::~CDnPvPGameTask()
{
	SAFE_DELETE( m_pRespawnLogic );
}

int CDnPvPGameTask::OnDispatchMessage(CDNUserSession *pSession, int nMainCmd, int nSubCmd, char *pData, int nLen )
{
	switch (nMainCmd)
	{
	case CS_ACTOR: OnRecvActorMessage(pSession, nSubCmd, pData, nLen); break;
	case CS_PVP: OnRecvPvPMessage(pSession, nSubCmd, pData, nLen); break;
	}

	CGameServerSession::OnDispatchMessage( pSession, nMainCmd, nSubCmd, pData, nLen );
	return ERROR_NONE;
}

int CDnPvPGameTask::OnRecvActorMessage( CDNUserSession * pSession, int nSubCmd, char *pData, int nLen )
{
	switch (nSubCmd)
	{
	case eActor::CS_CMDMOVE:
	case eActor::CS_CMDSTOP:
		{
			CPvPGameMode * pMode = GetRoom()->GetPvPGameMode();
			if (pMode)
				pMode->OnCancelAcquirePoint(pSession->GetActorHandle());
			else
				_DANGER_POINT();
		}
	}
	return ERROR_NONE;
}

int CDnPvPGameTask::OnRecvPvPMessage( CDNUserSession * pSession, int nSubCmd, char *pData, int nLen )
{
	CPvPGameMode* pPvPGameMode = GetRoom()->GetPvPGameMode();
	if (pPvPGameMode == NULL)
		return ERROR_GENERIC_UNKNOWNERROR;
	return pPvPGameMode->OnRecvPvPMessage(m_LocalTime, pSession, nSubCmd, pData, nLen);
}

void CDnPvPGameTask::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CPvPGameMode* pPvPGameMode = GetRoom()->GetPvPGameMode();
	if( pPvPGameMode )
		pPvPGameMode->Process( LocalTime, fDelta );
	m_pRespawnLogic->Process( LocalTime, fDelta );

	CDnGameTask::Process( LocalTime, fDelta );
}

void CDnPvPGameTask::InitializeRespawnArea()
{
	for( DWORD i=0; i<m_pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
	{
		CEtWorldSector*			pSector	 = m_pWorld->GetGrid()->GetActiveSector(i);
		int nGetAreType = ETE_PvPRespawnItemArea;		//This is the default
		if (GetRoom() && GetRoom()->bIsOccupationMode() && GetRoom()->bIsGuildWarMode() == false)
			nGetAreType = ETE_PvPOccupationItemArea;

		CEtWorldEventControl*	pControl = pSector->GetControlFromUniqueID( nGetAreType );
		if( !pControl ) 
			continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) 
		{
			CEtWorldEventArea* pArea = pControl->GetAreaFromIndex(j);
			if( pArea )
				m_pRespawnLogic->AddItemRespawnArea( pArea );
		}
	}
}

void CDnPvPGameTask::InitializeOccupationSystem()
{
	TBattleGourndModeInfo Info;
	CPvPGameMode* pPvPGameMode = GetRoom()->GetPvPGameMode();
	if (pPvPGameMode == NULL)
	{
		_DANGER_POINT();
		return;
	}

	if (pPvPGameMode->GetPvPGameModeTable()->nBattleGroundTableID <= 0) return;
	if (g_pDataManager->GetBattleGroundModeInfo(pPvPGameMode->GetPvPGameModeTable()->nBattleGroundTableID, Info) == false)
		return;

	if (pPvPGameMode->InitializeOccupationSystem(&Info) == false)
	{
		_DANGER_POINT();
		return;
	}

	for (int j = 0; j < PvPCommon::TeamIndex::Max; j++)
	{
		UINT nGuildDBID = 0;
		int nScore = 0;
		static_cast<CDNPvPGameRoom*>(GetRoom())->GetGuildWarInfoByIndex(j, nGuildDBID, nScore);
		CPvPOccupationTeam * pTeam = pPvPGameMode->GetOccupationSystem()->AddOccupationTeam(j == 0 ? PvPCommon::Team::A : PvPCommon::Team::B, nGuildDBID, nScore);
		if (pTeam == NULL)
		{
			_DANGER_POINT();
			continue;
		}

		if (Info.ModeInfo.nBossID[j] > 0)
		{
			if (Info.ModeInfo.nBossSpawnAreaID[j] <= 0)
			{
				_ASSERT_EXPR(0, L"Occupation mode Boss ID requires a configured spawn position!");
				continue;
			}

			for( DWORD i=0; i<m_pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
			{
				CEtWorldSector*			pSector	 = m_pWorld->GetGrid()->GetActiveSector(i);
				CEtWorldEventArea* pArea = pSector->GetEventAreaFromCreateUniqueID(Info.ModeInfo.nBossSpawnAreaID[j]);
				if (pArea)
					pTeam->InitializeOccupationTeam(Info.ModeInfo.nBossID[j], pArea);
			}
		}
	}

	std::vector<TPositionAreaInfo> vAreaList;
	if (g_pDataManager->GetBattleGroundResourceAreaInfo(GetMapTableID(), pPvPGameMode->GetPvPGameModeTable()->nItemID, vAreaList) == false)
		return;

	for( DWORD i=0; i<m_pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
	{
		CEtWorldSector*			pSector	 = m_pWorld->GetGrid()->GetActiveSector(i);
		CEtWorldEventControl*	pControl = pSector->GetControlFromUniqueID( ETE_GuildWarFlag );
		if( !pControl ) 
			continue;

		for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) 
		{
			CEtWorldEventArea* pArea = pControl->GetAreaFromIndex(j);
			if (pArea)
			{
				for (std::vector<TPositionAreaInfo>::iterator ii = vAreaList.begin(); ii != vAreaList.end(); ii++)
				{
					if (pArea->GetCreateUniqueID() == (*ii).nAreaID)
						pPvPGameMode->GetOccupationSystem()->AddOccupationArea(pArea, &(*ii));
				}
			}
		}
	}
}

bool CDnPvPGameTask::InitializeStage( int nCurrentMapIndex, int nGateIndex, TDUNGEONDIFFICULTY StageDifficulty, int nRandomSeed, bool bContinueStage, bool bDirectConnect, int nGateSelect )
{
	bool bResult = CDnGameTask::InitializeStage( nCurrentMapIndex, nGateIndex, StageDifficulty, nRandomSeed, bContinueStage, bDirectConnect, nGateSelect );
	if( !bResult ) 
		return false;

	InitializeRespawnArea();
	InitializeOccupationSystem();

	return true;
}

bool CDnPvPGameTask::OnInitializePlayerActor( CDNUserSession* pSession, const int iVectorIndex )
{
	if( pSession->GetTeam() == PvPCommon::Team::Observer )
	{
		return true;
	}

	return CDnGameTask::OnInitializePlayerActor( pSession, iVectorIndex );
}

void CDnPvPGameTask::OnInitializeStartPosition( CDNUserSession* pSession, const int iPartyIndex )
{
	DnActorHandle hActor = pSession->GetActorHandle();
	if( hActor ) 
	{
		if( GetRoom() && GetRoom()->GetPvPGameMode() )
			GetRoom()->GetPvPGameMode()->OnInitializeActor( pSession );
	}

	OnInitializeRespawnPosition( hActor, true );
}

void CDnPvPGameTask::OnInitializeRespawnPosition( DnActorHandle hActor, const bool bIsStartPositon/*=false*/ )
{
	if( hActor )
	{
		CEtWorldEventControl*	pControl	= NULL;
		CEtWorldSector*			pSector		= NULL;
		CEtWorldEventArea*		pArea		= NULL;

		std::vector<CEtWorldEventArea*> vRespawnArea;

		CPvPGameMode* pPvPGameMode = GetRoom()->GetPvPGameMode();

		bool bIndividualMode = pPvPGameMode->bIsIndividualMode();
		bool bAllKillMode = pPvPGameMode->bIsAllKillMode();
#if defined(PRE_ADD_PVP_TOURNAMENT)
		bool bTournament = pPvPGameMode->bIsTournamentMode();
		if( bIsStartPositon || bIndividualMode || bAllKillMode || bTournament )
#else
		if( bIsStartPositon || bIndividualMode || bAllKillMode )
#endif //#if defined(PRE_ADD_PVP_TOURNAMENT)		
		{
			for( DWORD i=0; i<m_pWorld->GetGrid()->GetActiveSectorCount(); i++ ) 
			{
				pSector	 = m_pWorld->GetGrid()->GetActiveSector(i);
				pControl = pSector->GetControlFromUniqueID( ETE_PvPRespawnArea );
				if( !pControl ) 
					continue;

				for( DWORD j=0; j<pControl->GetAreaCount(); j++ ) 
				{
					pArea = pControl->GetAreaFromIndex(j);
					if( pArea )
					{
						PvPRespawnAreaStruct* pStruct = reinterpret_cast<PvPRespawnAreaStruct*>(pArea->GetData());
						if( pStruct == NULL )
							continue;

						bool bTeamCheck = false;
						if( hActor->bIsObserver() || pStruct->nTeam == hActor->GetTeam() )
							bTeamCheck = true;

						if( bIndividualMode )
						{
							vRespawnArea.push_back( pArea );
							continue;
						}
						if( pPvPGameMode->bIsZombieMode() && bIsStartPositon )
						{
							bTeamCheck = (pStruct->nTeam == PvPCommon::Team::Human);
						}
#if defined( PRE_ADD_PVP_TOURNAMENT)
						if( bAllKillMode == true || bTournament == true )
#else
						if( bAllKillMode == true )
#endif
						{
							if( bTeamCheck && (pStruct->IsStartArea?true:false) == bIsStartPositon )
								vRespawnArea.push_back( pArea );
						}
						else
						{
							if( bTeamCheck && pStruct->IsStartArea )
								vRespawnArea.push_back( pArea );
						}
						m_pRespawnLogic->AddRespawnArea( pArea );
					}
				}
			}
	
			// Since this function is called frequently, set a flag to prevent repeated AddRespawnArea() calls.
			m_pRespawnLogic->FinishInitialize();
		}
		else
		{
			CEtWorldEventArea* pRespawnArea = m_pRespawnLogic->OnRespawn( hActor );
			if( pRespawnArea )
				vRespawnArea.push_back( pRespawnArea );
		}

		if( !vRespawnArea.empty() )
		{
			int SeedValue = hActor->GetSessionID();
			if( bIsStartPositon == false && bIndividualMode == true )
			{
				SeedValue += timeGetTime();
				CPvPIndividualRespawnMode* pIndividualMode = static_cast<CPvPIndividualRespawnMode*>(pPvPGameMode);
				SeedValue += InterlockedIncrement( &pIndividualMode->m_lDieCount );
			}
			// Prevent restarting at the exact same spot as round 1 in Round Mode
			else if( bIsStartPositon && static_cast<CDNPvPGameRoom*>(GetRoom())->GetPvPGameMode()->GetPlayRound() != 0 )
			{
				SeedValue += timeGetTime();
			}
			_srand( GetRoom(), SeedValue );
			CEtWorldEventArea* pRespawnArea = vRespawnArea[_rand(GetRoom())%vRespawnArea.size()];

			// If it is an individual match and selecting start positions...
			if( bIndividualMode && bIsStartPositon )
			{
				CPvPIndividualRespawnMode* pIndividualMode = static_cast<CPvPIndividualRespawnMode*>(pPvPGameMode);

				if( pIndividualMode->bIsUsedRespawnArea( pRespawnArea->GetName() ) == true )
				{
					bool bFind = false;
					for( UINT i=0 ; i<vRespawnArea.size() ; ++i )
					{
						if( pIndividualMode->bIsUsedRespawnArea( vRespawnArea[i]->GetName() ) == false )
						{
							bFind			= true;
							pRespawnArea	= vRespawnArea[i];
							break;
						}
					}
					if( bFind == false )
					{
						// Insufficient number of PvP respawn areas.
						// Need to talk to the level team to expand RespawnArea. - Kimbap -
						_ASSERT(0);
					}
				}
				pIndividualMode->PushUsedRespawnArea( pRespawnArea->GetName() );
			}

			EtVector3	vRandPos	= GetGenerationRandomPosition( pRespawnArea->GetOBB() );
			float		fRotate		= pRespawnArea->GetRotate();
			EtVector2	vStartPos( vRandPos.x,vRandPos.z );

			// Set respawn position/LOOK
			hActor->SetPosition( EtVector3( vStartPos.x, m_pWorld->GetHeight( vStartPos.x, vStartPos.y ), vStartPos.y ) );

			EtVector2 vDir;
			vDir.x = sin( EtToRadian( fRotate ) );
			vDir.y = cos( EtToRadian( fRotate ) );
			hActor->Look( vDir );

			if( hActor->GetObjectHandle() )
				hActor->GetObjectHandle()->Update( *hActor->GetMatEx() );
		}
		else
			_DANGER_POINT();
	}
	else
		_DANGER_POINT();
}

void CDnPvPGameTask::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	if( m_pRespawnLogic )
		m_pRespawnLogic->OnDie( hActor );
}

#if defined( PRE_ADD_PVP_COMBOEXERCISE )
void CDnPvPGameTask::RequestRecallAllMonster( EtVector3 &vPos, EtVector2 &vLook )
{
	for( UINT i=0 ; i<m_hVecMonsterList.size() ; ++i )
	{
		DnActorHandle hMonster = m_hVecMonsterList[i];
		if( !hMonster || !hMonster->IsMonsterActor() )
			continue;

		if( hMonster->IsDie() || hMonster->IsDestroy() )
			continue;

		CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(hMonster.GetPointer());
		if( pMonster->IsSummonedMonster() )
			continue;

		hMonster->CmdWarp( vPos, vLook );
	}
}
#endif // #if defined( PRE_ADD_PVP_COMBOEXERCISE )