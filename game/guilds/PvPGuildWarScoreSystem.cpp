
#include "stdafx.h"
#include "PvPGuildWarScoreSystem.h"
#if defined( _GAMESERVER )
#include "DnActor.h"
#include "PvPOccupationMode.h"
#include "PvPOccupationSystem.h"
#include "DNGameRoom.h"
#else // #if defined( _GAMESERVER )
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnMonsterActor.h"
#include "DnInterface.h"
#include <MMSystem.h>
#endif // #if defined( _GAMESERVER )

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CPvPGuildWarScoreSystem::CPvPGuildWarScoreSystem()
{
#ifdef _CLIENT
	DNTableFileFormat*  pSox = GetDNTable( CDnTableDB::TBATTLEGROUNDMODE );
	char szColumName[32];

	for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
	{
		if( PvPCommon::TeamIndex::A == itr )
		{
			for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
			{
				sprintf_s( szColumName, _countof(szColumName), "_CosmosGate%dID", jtr+1 );
				m_DoorID[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

				sprintf_s( szColumName, _countof(szColumName), "_CosmosGate%dUnderAtt", jtr+1 );
				m_DoorDamageString[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

				sprintf_s( szColumName, _countof(szColumName), "_CosmosGate%dDestroy", jtr+1 );
				m_DoorDieString[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
			}

			sprintf_s( szColumName, _countof(szColumName), "_CosmosBossID" );
			m_BossID[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

			sprintf_s( szColumName, _countof(szColumName), "_CosmosBossUnderAtt" );
			m_BossString[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
		}
		else if( PvPCommon::TeamIndex::B == itr )
		{
			for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
			{
				sprintf_s( szColumName, _countof(szColumName), "_ChaosGate%dID", jtr+1 );
				m_DoorID[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

				sprintf_s( szColumName, _countof(szColumName), "_ChaosGate%dUnderAtt", jtr+1 );
				m_DoorDamageString[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

				sprintf_s( szColumName, _countof(szColumName), "_ChaosGate%dDestroy", jtr+1 );
				m_DoorDieString[itr][jtr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
			}

			sprintf_s( szColumName, _countof(szColumName), "_ChaosBossID" );
			m_BossID[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();

			sprintf_s( szColumName, _countof(szColumName), "_ChaosBossUnderAtt" );
			m_BossString[itr] = pSox->GetFieldFromLablePtr( 1, szColumName)->GetInteger();
		}

		m_BossDamageTime[itr] = 0;

		for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
			m_DoorDamageTime[itr][jtr] = 0;
	}

	m_nDamageSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( "UI_Guild_BossAttacked.ogg" ).c_str(), false, false );			

	m_bRedBossKill = false;
	m_bBlueBossKill = false;
#endif
}

CPvPGuildWarScoreSystem::~CPvPGuildWarScoreSystem()
{
}

bool CPvPGuildWarScoreSystem::GetOccupationTeamScore(int nTeam, int &nTeamScore)
{
	for (std::vector <STeamOccupationScore>::iterator ii = m_vOccupationScoreTeam.begin(); ii != m_vOccupationScoreTeam.end(); ii++)
	{
		if ((*ii).nTeamID == nTeam)
		{		
			nTeamScore = (*ii).nKillScore + (*ii).nAcquireScore;
			return true;
		}
	}
	return false;
}

bool CPvPGuildWarScoreSystem::GetOccupationTeamScore(int nTeam, TPvPOccupationTeamScoreInfo &TeamScore)
{
	for (std::vector <STeamOccupationScore>::iterator ii = m_vOccupationScoreTeam.begin(); ii != m_vOccupationScoreTeam.end(); ii++)
	{
		if ((*ii).nTeamID == nTeam)
		{
			TeamScore.nTeamKillScore = (*ii).nKillScore;
			TeamScore.nTeamAcquireScore = (*ii).nAcquireScore;
			TeamScore.nTeamStealAcquireScore = (*ii).nStealAquireScore;
			return true;
		}
	}
	return false;;
}
void CPvPGuildWarScoreSystem::OnDamage( DnActorHandle hActor, DnActorHandle hHitter, const INT64 iDamage  )
{
#ifdef _CLIENT
	if ((hActor && hActor->IsPlayerActor() == false) && (hHitter && hHitter->IsPlayerActor()))
	{
		if( hActor->IsMonsterActor() && !hActor->IsDie() )
		{
			DnMonsterActorHandle hMonster = hActor;

			int nStringID = -1;
			LOCAL_TIME localTime = timeGetTime();
			int nActorID = hMonster->GetMonsterClassID();

			for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
			{
				if( nActorID == m_BossID[itr] )
				{
					if( 0 == m_BossDamageTime[itr] )
						nStringID = m_BossString[itr];
					else if( PvPCommon::Common::DamagePrintTime < localTime - m_BossDamageTime[itr] )
						nStringID = m_BossString[itr];

					m_BossDamageTime[itr] = localTime;
					break;
				}

				for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
				{
					if( nActorID == m_DoorID[itr][jtr] )
					{
						if( 0 == m_DoorDamageTime[itr][jtr] )
							nStringID = m_DoorDamageString[itr][jtr];
						else if( PvPCommon::Common::DamagePrintTime < localTime - m_DoorDamageTime[itr][jtr] )
							nStringID = m_DoorDamageString[itr][jtr];

						m_DoorDamageTime[itr][jtr] = localTime;
						break;
					}	
				}

				if( -1 != nStringID )
					break;
			}

			if( -1 != nStringID )
				GetInterface().ShowOccupationWarnning( hActor, nStringID, true, true, true, true, m_nDamageSound );
		}
	}
#endif
}

void CPvPGuildWarScoreSystem::OnDie(DnActorHandle hActor, DnActorHandle hHitter)
{
	CPvPScoreSystem::OnDie(hActor, hHitter);

	if ((hActor && hActor->IsPlayerActor() == false) && (hHitter && hHitter->IsPlayerActor()))
	{
		if (hActor->GetTeam() == hHitter->GetTeam())
			return;

#if defined( _GAMESERVER )
		if (hActor->GetGameRoom() == NULL)
			return;

		CPvPGameMode * pMode = hActor->GetGameRoom()->GetPvPGameMode();
		if (pMode && pMode->bIsBoss(hActor))
		{
			SMyOccupationCount * pScore = GetMyOccupationCount(hHitter->GetName());
			if (pScore)
			{
				pScore->cBossKillCount++;
				COccupationScoreSystem::AddOccupationTeamScore(hHitter, PvPCommon::OccupationScoreType::Kill);
				return;
			}
		}
#else
		if( hActor && hActor->IsMonsterActor() )
		{
			DnMonsterActorHandle hMonster = hActor;

			if( hMonster->GetGrade() == CDnMonsterState::Boss )
			{
				SMyOccupationCount * pScore = GetMyOccupationCount(hHitter->GetName());
				if (pScore)
				{
					pScore->cBossKillCount++;
					COccupationScoreSystem::AddOccupationTeamScore(hHitter, PvPCommon::OccupationScoreType::Kill);

					if( PvPCommon::Team::A == hMonster->GetTeam() )
						m_bBlueBossKill = true;
					else if( PvPCommon::Team::B == hMonster->GetTeam() )
						m_bRedBossKill = true;

					return;
				}	
			}

			int nActorID = hMonster->GetMonsterClassID();

			for( int itr = 0; itr < PvPCommon::TeamIndex::Max; ++itr )
			{
				for( int jtr = 0; jtr < PvPCommon::Common::DoorCount; ++jtr )
				{
					if( m_DoorID[itr][jtr] == nActorID )
					{
						GetInterface().ShowOccupationWarnning( hActor, m_DoorDieString[itr][jtr], false, true, true, true, -1 );
						return;
					}
				}
			}
		}
#endif
	}
	else if ( hHitter && hHitter->IsPlayerActor() )
	{
		COccupationScoreSystem::AddOccupationTeamScore(hHitter, PvPCommon::OccupationScoreType::Kill);
	}
}

bool CPvPGuildWarScoreSystem::OnAcquirePoint(int nTeam, const WCHAR * pName, bool bSnatched, bool bBreakInto)
{
	if (pName)
	{
		SMyOccupationCount * pScore = GetMyOccupationCount(pName);
		if (pScore)
		{
			int nPointType;			
			pScore->nAcquireCount++;
			nPointType = PvPCommon::OccupationScoreType::Acquire;

			if( !bBreakInto )
				AddOccupationTeamScore(nTeam, nPointType);			
			return true;
		}
	}
	return false;
}

void CPvPGuildWarScoreSystem::AddOccupationTeamScore(int nTeam, int nType)
{
	std::vector <STeamOccupationScore>::iterator ii;
	for (ii = m_vOccupationScoreTeam.begin(); ii != m_vOccupationScoreTeam.end(); ii++)
	{
		if ((*ii).nTeamID == nTeam)
		{
			if (nType == PvPCommon::OccupationScoreType::Kill) (*ii).nKillScore++;
			else if (nType == PvPCommon::OccupationScoreType::Acquire) (*ii).nAcquireScore++;
			return;
		}
	}

	STeamOccupationScore Score;
	Score.nTeamID = nTeam;

	if (nType == PvPCommon::OccupationScoreType::Kill) Score.nKillScore++;
	else if (nType == PvPCommon::OccupationScoreType::Acquire) Score.nAcquireScore++;

	m_vOccupationScoreTeam.push_back(Score);
}

#ifdef _CLIENT
int CPvPGuildWarScoreSystem::GetDoorID( int nTeam, int nCount )
{
	if( PvPCommon::TeamIndex::Max <= nTeam || PvPCommon::Common::DoorCount <= nCount )
		return 0;

	return m_DoorID[nTeam][nCount];
}
#endif
