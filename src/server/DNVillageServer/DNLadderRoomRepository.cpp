
#include "stdafx.h"
#include "DNLadderRoomRepository.h"
#include "DNLadderRoom.h"
#include "DNUserSession.h"
#include "DnLadderMatchingSystem.h"
#include "DNLadderSystemManager.h"
#include "DNDBConnectionManager.h"
#include "DNDBConnection.h"
#include "DNFieldManager.h"

extern TVillageConfig g_Config;

using namespace LadderSystem;

CRoomRepository::CRoomRepository()
:m_bIIncRoomIndex(timeGetTime())
{
	m_pMatchingSystem		= new CMatchingSystem( this );
	m_dwCUCountUpdateTick	= timeGetTime();
}

CRoomRepository::~CRoomRepository()
{
	delete m_pMatchingSystem;
}

void CRoomRepository::_QueryCUCount( MatchType::eCode MatchType )
{
	if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
		return;

	size_t Sum = 0;
	for( std::map<INT64,CRoom*>::iterator itor=m_mRoomIndexRoomPtr.begin() ; itor!=m_mRoomIndexRoomPtr.end() ; ++itor )
	{
		if( (*itor).second->GetMatchType() == MatchType )
			Sum += (*itor).second->GetUserInfo().size();
	}

	BYTE cThreadID;
	CDNDBConnection* pDBCon = g_pDBConnectionManager->GetDBConnection( cThreadID );
	pDBCon->QueryAddPvPLadderCUCount( cThreadID, g_Config.nWorldSetID, MatchType, static_cast<int>(Sum) );
	//g_Log.Log( LogType::_LADDER, L"[Ladder] MatchType:%d CU:%d\r\n", MatchType, static_cast<int>(Sum) );
}

void CRoomRepository::Process( DWORD dwCurTick )
{
	m_pMatchingSystem->Process( dwCurTick );

	for( std::map<INT64,CRoom*>::iterator itor=m_mRoomIndexRoomPtr.begin() ; itor!=m_mRoomIndexRoomPtr.end() ; ++itor )
		(*itor).second->Process( dwCurTick );

	// Temporary 30 minutes
	if( dwCurTick-m_dwCUCountUpdateTick >= 1000*60*30 )
	{
		m_dwCUCountUpdateTick = dwCurTick;

		// Only village servers that include PvP lobby send logs.
		if( g_pFieldManager->bIsIncludePvPLobby() == true )
		{
			_QueryCUCount( MatchType::_1vs1 );
			_QueryCUCount( MatchType::_2vs2 );
			_QueryCUCount( MatchType::_3vs3 );
			_QueryCUCount( MatchType::_4vs4 );
		}
	}
}

int	CRoomRepository::Create( CDNUserSession* pSession, MatchType::eCode MatchType )
{
	if( LadderSystem::bIsServiceMatchType( MatchType ) == false )
		return ERROR_INVALIDPACKET;

	// Cannot enter while in a PvP room~
	if( pSession->GetPvPIndex() > 0 )
		return ERROR_PVP_ALREADY_PVPROOM;

	// Check if already in ladder room
	if( bIsExist( pSession->GetCharacterName() ) )
		return ERROR_ALREADY_LADDERROOM;

	// Check DWC permission
		// Check level restriction
		if( pSession->GetLevel() < static_cast<int>(CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::PvPLadder_LimitLevel )) )
			return ERROR_LADDERSYSTEM_CANT_ENTER_LEVELLIMIT;

	CRoom* pRoom = new CRoom( _GenerateRoomIndex(), MatchType, pSession );


	m_mRoomIndexRoomPtr.insert( std::make_pair( pRoom->GetRoomIndex(), pRoom ) );
	m_mCharNameRoomIndex.insert( std::make_pair(pSession->GetCharacterName(), pRoom->GetRoomIndex() ) );
	pSession->SetLadderUser( true );
	return ERROR_NONE;
}

int	CRoomRepository::Join( CDNUserSession* pSession, CRoom* pRoom )
{
	m_mRoomIndexRoomPtr.insert( std::make_pair( pRoom->GetRoomIndex(), pRoom ) );
	m_mCharNameRoomIndex.insert( std::make_pair(pSession->GetCharacterName(), pRoom->GetRoomIndex() ) );
	pSession->SetLadderUser( true );

	pRoom->JoinUser( pSession );
	return ERROR_NONE;
}

int	CRoomRepository::Leave( CDNUserSession* pSession, Reason::eCode Reason )
{
	CRoom* pLadderRoom = GetRoomPtr( pSession->GetCharacterName() );
	if( pLadderRoom == NULL )
		return ERROR_INVALIDUSER_LADDERROOM;

	if( pLadderRoom->OutUser( pSession->GetCharacterDBID(), pSession->GetCharacterName(), Reason ) == false )
		return ERROR_INVALIDUSER_LADDERROOM;

	pSession->SetLadderUser( false );
	m_mCharNameRoomIndex.erase( pSession->GetCharacterName() );

	// Delete LadderRoom
	if( pLadderRoom->bIsEmpty() )
	{
		m_mRoomIndexRoomPtr.erase( pLadderRoom->GetRoomIndex() );
		_OnDestroyRoom( pLadderRoom );
	}
	pLadderRoom->AdjustNewLeader();
	return ERROR_NONE;
}

int	CRoomRepository::Matching( CDNUserSession* pSession, bool bIsCancel )
{
	CRoom* pLadderRoom = GetRoomPtr( pSession->GetCharacterName() );
	if( pLadderRoom == NULL )
		return ERROR_INVALIDUSER_LADDERROOM;

	if( pLadderRoom->GetLeaderCharDBID() != pSession->GetCharacterDBID() )
		return ERROR_LADDERSYSTEM_NOLEADER;

	// Match cancel request
	if( bIsCancel == true )
	{
		switch( pLadderRoom->GetRoomState() )
		{
			case RoomState::WaitMatching:
			case RoomState::Matched:
			{
				break;
			}
			default:
			{
				return ERROR_LADDERSYSTEM_CANT_MATCHINGCANCLE_ROOMSTATE;
			}
		}
		pLadderRoom->ChangeRoomState( RoomState::WaitUser );
	}
	// Match request
	else
	{
		if( pLadderRoom->bIsFullUser() == false )
			return ERROR_LADDERSYSTEM_NOTENOUGH_USER;
		if( pLadderRoom->bIsAllConnectUser() == false )
			return ERROR_LADDERSYSTEM_CANT_MATCHING_ROOMSTATE;


		// Check RoomState
		if( pLadderRoom->GetRoomState() != RoomState::WaitUser )
			return ERROR_LADDERSYSTEM_CANT_MATCHINGCANCLE_ROOMSTATE;
		pLadderRoom->ChangeRoomState( RoomState::WaitMatching );
	}

	return ERROR_NONE;
}

void CRoomRepository::DisconnectUser( INT64 biCharDBID, WCHAR* pwszCharName )
{
	CRoom* pLadderRoom = GetRoomPtr( pwszCharName );
	if( pLadderRoom == NULL )
		return;
	
	pLadderRoom->OutUser( biCharDBID, pwszCharName, Reason::Disconnect );
	m_mCharNameRoomIndex.erase( pwszCharName );
	// Delete LadderRoom
	if( pLadderRoom->bIsEmpty() )
	{
		m_mRoomIndexRoomPtr.erase( pLadderRoom->GetRoomIndex() );
		_OnDestroyRoom( pLadderRoom );
	}
	else if( pLadderRoom->GetLeaderCharDBID() == 0 )
	{
		pLadderRoom->AdjustNewLeader();
	}
}

void CRoomRepository::SendPlayingRoomList( CDNUserSession* pSession, MatchType::eCode MatchType, UINT uiPage/*=1*/ )
{
	std::vector<CRoom*> vRoomList;
	
	UINT	uiMaxPage		= 0;
	UINT	uiRoomPerPage	= Common::RoomListPerPage/MatchType;
	int		iRet			= ERROR_GENERIC_INVALIDREQUEST;

	std::map<MatchType::eCode,std::list<INT64>>::iterator itor = m_mMatchTypePlayingRoomIndex.find( MatchType );
	if( itor != m_mMatchTypePlayingRoomIndex.end() )
	{
		UINT uiRoomSize = static_cast<UINT>((*itor).second.size());
		uiMaxPage = uiRoomSize%uiRoomPerPage == 0 ? uiRoomSize/uiRoomPerPage : (uiRoomSize/uiRoomPerPage)+1;
	}

	if( uiPage > uiMaxPage || uiPage == 0 )
	{
		iRet = uiMaxPage == 0 ? ERROR_NONE : ERROR_LADDERSYSTEM_CANT_EXIST_ROOMLIST_PAGE;
	}
	else
	{
		iRet = ERROR_NONE;
		vRoomList.reserve( Common::RoomListPerPage );

		std::list<INT64>::iterator Litor = (*itor).second.begin();
		if( (*itor).second.size() >= (uiPage-1)*Common::RoomListPerPage )
		{
			std::advance( Litor, (uiPage-1)*Common::RoomListPerPage );

			for( Litor ; Litor!=(*itor).second.end() ; ++Litor )
			{
				vRoomList.push_back( GetRoomPtr( *Litor ) );
				if( vRoomList.size() >= uiRoomPerPage )
					break;
			}
		}
		else
		{
			iRet = ERROR_LADDERSYSTEM_CANT_EXIST_ROOMLIST_PAGE;
		}
	}

	pSession->SendLadderPlayingRoomList( iRet, vRoomList, uiMaxPage );
}

bool CRoomRepository::bIsExist( const WCHAR* pwszCharName )
{
	std::map<std::wstring,INT64>::iterator itor = m_mCharNameRoomIndex.find( pwszCharName );
	if( itor == m_mCharNameRoomIndex.end() )
		return false;

	_ASSERT( m_mRoomIndexRoomPtr.end() != m_mRoomIndexRoomPtr.find( (*itor).second ) );

	return true;
}

CRoom* CRoomRepository::GetRoomPtr( const WCHAR* pwszCharName )
{
	std::map<std::wstring,INT64>::iterator itor = m_mCharNameRoomIndex.find( pwszCharName );
	if( itor == m_mCharNameRoomIndex.end() )
		return NULL;

	std::map<INT64,CRoom*>::iterator itor2 = m_mRoomIndexRoomPtr.find( (*itor).second );
	if( itor2 == m_mRoomIndexRoomPtr.end() )
		return NULL;

	return (*itor2).second;
}

CRoom* CRoomRepository::GetRoomPtr( INT64 biRoomIndex )
{
	std::map<INT64,CRoom*>::iterator itor2 = m_mRoomIndexRoomPtr.find( biRoomIndex );
	if( itor2 == m_mRoomIndexRoomPtr.end() )
		return NULL;

	return (*itor2).second;
}

int	CRoomRepository::GetAvgMatchingTimeSec( MatchType::eCode MatchType )
{ 
	return m_pMatchingSystem->GetAvgMatchingTimeSec(MatchType); 
}

void CRoomRepository::_OnDestroyRoom( CRoom* pLadderRoom )
{
	if( pLadderRoom->GetOpponentRoomIndex() > 0 )
	{
		std::map<INT64,CRoom*>::iterator itor = m_mRoomIndexRoomPtr.find( pLadderRoom->GetOpponentRoomIndex() );
		if( itor != m_mRoomIndexRoomPtr.end() )
		{
			CRoom* pOpponentLadderRoom = (*itor).second;
			if( pOpponentLadderRoom->GetOpponentRoomIndex() == pLadderRoom->GetRoomIndex() )
			{
				switch( pOpponentLadderRoom->GetRoomState() )
				{
					case RoomState::WaitMatching:
					case RoomState::Matched:
					{
						pOpponentLadderRoom->ChangeRoomState( RoomState::WaitUser );
						break;
					}
				}
				pOpponentLadderRoom->ClearOpponentRoomIndex();
			}
		}
	}

	if( pLadderRoom->GetRoomState() == RoomState::Playing )
	{
		DeletePlayingList( pLadderRoom );
	}


	SAFE_DELETE( pLadderRoom );
}

void CRoomRepository::InsertPlayingList( CRoom* pLadderRoom )
{
	CRoom* pOpponentRoom = GetRoomPtr( pLadderRoom->GetOpponentRoomIndex() );
	if( pOpponentRoom == NULL )
		return;

	if( pLadderRoom->bIsValidOpponentRoom( pOpponentRoom, RoomState::Playing )	== false )
		return;

	std::map<MatchType::eCode,std::list<INT64>>::iterator itor = m_mMatchTypePlayingRoomIndex.find( pLadderRoom->GetMatchType() );
	if( itor == m_mMatchTypePlayingRoomIndex.end() )
	{
		std::list<INT64> Temp;
		Temp.push_back( pLadderRoom->GetRoomIndex() );
		m_mMatchTypePlayingRoomIndex.insert( std::make_pair(pLadderRoom->GetMatchType(),Temp) );
	}
	else
	{
		std::list<INT64>::iterator Litor  = std::find( (*itor).second.begin(), (*itor).second.end(), pLadderRoom->GetRoomIndex() );
		if( Litor != (*itor).second.end() )
		{
			_ASSERT(0);
			return;
		}
		
		Litor = std::find( (*itor).second.begin(), (*itor).second.end(), pOpponentRoom->GetRoomIndex() );
		if( Litor != (*itor).second.end() )
		{
			return;
		}
		
		(*itor).second.push_front( pLadderRoom->GetRoomIndex() );
	}
}

void CRoomRepository::DeletePlayingList( CRoom* pLadderRoom )
{
	std::map<MatchType::eCode,std::list<INT64>>::iterator itor = m_mMatchTypePlayingRoomIndex.find( pLadderRoom->GetMatchType() );
	if( itor == m_mMatchTypePlayingRoomIndex.end() )
		return;


	(*itor).second.remove( pLadderRoom->GetRoomIndex() );

}
