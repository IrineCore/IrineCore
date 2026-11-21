
#include "stdafx.h"
#include "PvPZombieScoreSystem.h"
#if defined( _GAMESERVER )
#include "DnActor.h"
#include "PvPGameMode.h"
#include "DNGameRoom.h"
#include "PvPZombieMode.h"
#include "DNUserSession.h"
#include "DnPlayerActor.h"
#else // #if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

void CPvPZombieScoreSystem::OnDie( DnActorHandle hActor, DnActorHandle hHitter )
{
	bool bZombieKill = false;

#if defined( _GAMESERVER )
	if( hActor && hActor->GetGameRoom() )
	{
		CDNGameRoom*	pGameRoom	 = hActor->GetGameRoom();
		CPvPGameMode*	pPvPGameMode = pGameRoom->GetPvPGameMode();

		if( pPvPGameMode )
		{
			_ASSERT( pPvPGameMode->bIsZombieMode() );
			CPvPZombieMode* pCaptainMode = static_cast<CPvPZombieMode*>(pPvPGameMode);

			bZombieKill = pCaptainMode->bIsZombie( hActor );
		}
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hHitter.GetPointer());
		if( pPlayer )
		{
			if( bZombieKill )			
				pPlayer->GetUserSession()->AddGhoulScore(GhoulMode::PointType::GhoulKill);		
			else
				pPlayer->GetUserSession()->AddGhoulScore(GhoulMode::PointType::HumanKill);
		}
	}
#else 
	if( !hActor || !hActor->IsPlayerActor())
		return;

	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(hActor.GetPointer());
	if( pPlayer && pPlayer->IsTransformMode() )
		bZombieKill = true;

#endif // #if defined( _GAMESERVER )

	if( bZombieKill )
	{
		_UpdateZombieKillCount( hHitter );
	}

	CPvPScoreSystem::OnDie( hActor, hHitter );
}

void CPvPZombieScoreSystem::_UpdateZombieKillCount( DnActorHandle hActor, UINT uiCount/*=0*/ )
{
	if( !hActor )
		return;

	std::map<std::wstring,UINT>::iterator itor = m_mZombieKillCount.find( hActor->GetName() );
	if( itor == m_mZombieKillCount.end() )
	{
		m_mZombieKillCount.insert( std::make_pair(hActor->GetName(),uiCount ? uiCount : 1) );
	}
	else
	{
		if( uiCount )
		{
			(*itor).second = uiCount;
		}
		else
		{
			++(*itor).second;
		}
	}
}

void CPvPZombieScoreSystem::OnLeaveUser( DnActorHandle hActor )
{	
	if( hActor )
		m_mZombieKillCount.erase( hActor->GetName() );

	CPvPScoreSystem::OnLeaveUser( hActor );
}

UINT CPvPZombieScoreSystem::GetZombieKillCount( DnActorHandle hActor )
{
	if( !hActor )	
		return 0;

	std::map<std::wstring,UINT>::iterator itor = m_mZombieKillCount.find( hActor->GetName() );
	if( itor == m_mZombieKillCount.end() )
		return 0;

	return (*itor).second;
}

int CPvPZombieScoreSystem::GetHumanWinCount( const WCHAR* pwszCharName )
{
	std::map<std::wstring,int>::iterator itor = m_mHumanWinCount.find( pwszCharName );
	if( itor != m_mHumanWinCount.end() )
		return (*itor).second;
	return 0;
}

void CPvPZombieScoreSystem::OnFinishRound( DnActorHandle hActor, const bool bIsWin )
{
	if( hActor && bIsWin == true && hActor->GetTeam() == PvPCommon::Team::Human )	
	{
		std::map<std::wstring,int>::iterator itor = m_mHumanWinCount.find( hActor->GetName() );
		if( itor == m_mHumanWinCount.end() )
		{
			m_mHumanWinCount.insert( std::make_pair(hActor->GetName(),1) );
		}
		else
		{
			++(*itor).second;
		}
	}

	CPvPScoreSystem::OnFinishRound( hActor, bIsWin );
}

#if defined( _GAMESERVER )

int CPvPZombieScoreSystem::GetModeExp( DnActorHandle hActor )
{
	if( !hActor || hActor->GetGameRoom() == NULL || hActor->GetGameRoom()->GetPvPGameMode() == NULL )
		return 0;

	int iWinCount = GetHumanWinCount( hActor->GetName() );
	return hActor->GetGameRoom()->GetPvPGameMode()->GetPvPGameModeTable()->uiWinXPPerRound*iWinCount;
}

void CPvPZombieScoreSystem::SendScore( CDNUserSession* pGameSession )
{
	CPvPScoreSystem::SendScore( pGameSession );

	// In Captain mode, additionally send captain kill info.
	_SendZombieKillCount( pGameSession );
}

void CPvPZombieScoreSystem::_SendZombieKillCount( CDNUserSession* pSession )
{
	if( !pSession )
		return;

	CDNGameRoom* pGameRoom = pSession->GetGameRoom();
	if( !pGameRoom )
		return;

	if( m_mZombieKillCount.size() > PvPCommon::Common::MaxPlayer )
		return;

	SCPVP_ZOMBIEKILL_COUNT TxPacket;
	memset( &TxPacket, 0, sizeof(TxPacket) );

	char* pStartBuffer	= reinterpret_cast<char*>(TxPacket.sZombieKillCount);
	char* pBuffer		= pStartBuffer;

	for( std::map<std::wstring,UINT>::iterator itor=m_mZombieKillCount.begin() ; itor!=m_mZombieKillCount.end() ; ++itor )
	{
		TPvPZombieKillCount* pCount = reinterpret_cast<TPvPZombieKillCount*>(pBuffer);

		CDNGameRoom::PartyStruct* pStruct =  pGameRoom->GetPartyData( const_cast<WCHAR*>((*itor).first.c_str()) );
		if( !pStruct || !pStruct->pSession )
			continue;

		pCount->uiSessionID			= pStruct->pSession->GetSessionID();
		pCount->uiZombieKillCount	= (*itor).second;

		++TxPacket.cCount;
		pBuffer += sizeof(TPvPZombieKillCount);
	}

	int iSize = static_cast<int>(sizeof(SCPVP_ZOMBIEKILL_COUNT)-sizeof(TxPacket.sZombieKillCount)+(pBuffer-pStartBuffer));

	pSession->AddSendData( SC_PVP, ePvP::SC_ZOMBIEKILL_COUNT, reinterpret_cast<char*>(&TxPacket), iSize );
}

#else // #if defined( _GAMESERVER )

bool CPvPZombieScoreSystem::SetZombieKillCount( DnActorHandle hActor, const void* pData )
{
	const TPvPZombieKillCount* pCount = static_cast<const TPvPZombieKillCount*>(pData);
	_UpdateZombieKillCount( hActor, pCount->uiZombieKillCount );
	return true;
}

#endif // #if defined( _GAMESERVER )

