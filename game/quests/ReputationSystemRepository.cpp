
#include "stdafx.h"
#include "ReputationSystemRepository.h"
#include "NpcReputationSystem.h"
#include "ReputationSystemEventHandler.h"
#if defined( _SERVER )
#include "ReputationSystemEventHandlerForServer.h"
#include "DNGameDataManager.h"
#endif // #if defined( _SERVER )

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

#if defined( _SERVER )
CReputationSystemRepository::CReputationSystemRepository( CDNUserSession* pSession )
{
	m_pEventHandler = new CServerReputationSystemEventHandler( pSession );	
}
#else
CReputationSystemRepository::CReputationSystemRepository()
{
	m_pEventHandler = new CNullReputationSystemEventHandler();
	memset(m_UnionPoint, -1, sizeof(m_UnionPoint));
	m_LastUpdateNpcID = -1;
}
#endif;

CReputationSystemRepository::~CReputationSystemRepository()
{
	SAFE_DELETE( m_pEventHandler );
#if !defined ( _SERVER )
	memset(m_UnionPoint, -1, sizeof(m_UnionPoint));
#endif // #if !defined ( _SERVER )
	Clear();
}

void CReputationSystemRepository::Clear()
{
	for( std::map<int,IReputationSystem*>::iterator itor=m_NpcReputation.begin() ; itor!=m_NpcReputation.end() ; ++itor )
		SAFE_DELETE( (*itor).second );
	m_NpcReputation.clear();
}

void CReputationSystemRepository::Transaction()
{
	m_CommitReputationSystem.clear();
}

void CReputationSystemRepository::Commit( bool bClientSend/*=true*/ )
{
	if( m_pEventHandler )
		m_pEventHandler->OnCommit( m_CommitReputationSystem, bClientSend );
}

void CReputationSystemRepository::GetCommitData( std::vector<TNpcReputation>& Data )
{
	if( m_CommitReputationSystem.empty() )
		return;

#if defined( PRE_ADD_REPUTATION_EXPOSURE )
	Data.resize( m_CommitReputationSystem.size() );
	for( std::map<IReputationSystem*,size_t>::iterator itor=m_CommitReputationSystem.begin() ; itor!=m_CommitReputationSystem.end() ; ++itor )
	{
		_ASSERT( dynamic_cast<CNpcReputationSystem*>((*itor).first) );
		CNpcReputationSystem* pReputation = static_cast<CNpcReputationSystem*>((*itor).first);

		Data[(*itor).second] = TNpcReputation( pReputation->GetNpcID(), static_cast<int>(pReputation->Get( IReputationSystem::NpcFavor )), static_cast<int>(pReputation->Get( IReputationSystem::NpcMalice)) );
	}
#else
	for( std::map<IReputationSystem*,int>::iterator itor=m_CommitReputationSystem.begin() ; itor!=m_CommitReputationSystem.end() ; ++itor )
	{
		_ASSERT( dynamic_cast<CNpcReputationSystem*>((*itor).first) );
		CNpcReputationSystem* pReputation = static_cast<CNpcReputationSystem*>((*itor).first);

		Data.push_back( TNpcReputation( pReputation->GetNpcID(), static_cast<int>(pReputation->Get( IReputationSystem::NpcFavor )), static_cast<int>(pReputation->Get( IReputationSystem::NpcMalice)) ) );
	}
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )

	m_CommitReputationSystem.clear();
}

void CReputationSystemRepository::GetAllNpcData( std::vector<int>& Data )
{
	if( m_NpcReputation.empty() )
		return;

	Data.reserve( m_NpcReputation.size() );
	for( std::map<int,IReputationSystem*>::iterator itor=m_NpcReputation.begin() ; itor!=m_NpcReputation.end() ; ++itor )
	{
		Data.push_back( (*itor).first );
	}
}

int CReputationSystemRepository::GetNpcReputationPercent( const int iNpcID, IReputationSystem::eType Type )
{
	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor == m_NpcReputation.end() )
		return 0;

	return static_cast<int>((*itor).second->GetPercent( Type ));
}

float CReputationSystemRepository::GetNpcReputationPercentF( const int iNpcID, IReputationSystem::eType Type )
{
	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor == m_NpcReputation.end() )
		return 0;

	return (*itor).second->GetPercent( Type );
}

REPUTATION_TYPE	CReputationSystemRepository::GetNpcReputation( const int iNpcID, IReputationSystem::eType Type )
{
	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor == m_NpcReputation.end() )
		return 0;

	return (*itor).second->Get( Type );
}

REPUTATION_TYPE	CReputationSystemRepository::GetNpcReputationMax( const int iNpcID, IReputationSystem::eType Type )
{
	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor == m_NpcReputation.end() )
		return 0;

	return (*itor).second->GetMax( Type );
}

// Absolute value
void CReputationSystemRepository::SetNpcReputation( const int iNpcID, IReputationSystem::eType Type, REPUTATION_TYPE value )
{
	IReputationSystem* pReputationSystem = NULL;

	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor != m_NpcReputation.end() )
		pReputationSystem = (*itor).second;
	else
	{
		pReputationSystem = new CNpcReputationSystem( iNpcID, m_pEventHandler );
		pReputationSystem->Init();
		m_NpcReputation.insert( std::make_pair(iNpcID,pReputationSystem) );
	}

	if( pReputationSystem )
	{
		pReputationSystem->Set( Type, value );
#if defined( PRE_ADD_REPUTATION_EXPOSURE )
		m_CommitReputationSystem.insert( std::make_pair( pReputationSystem, m_CommitReputationSystem.size() ) );
#else
		m_CommitReputationSystem.insert( std::make_pair( pReputationSystem,1 ) );
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )
	}
}

// Relative value
void CReputationSystemRepository::AddNpcReputation( const int iNpcID, IReputationSystem::eType Type, REPUTATION_TYPE value, bool bIsReachMax/*=true*/ )
{
	IReputationSystem* pReputationSystem = NULL;

	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor != m_NpcReputation.end() )
		pReputationSystem = (*itor).second;
	else
	{
		pReputationSystem = new CNpcReputationSystem( iNpcID, m_pEventHandler );
		pReputationSystem->Init();
		m_NpcReputation.insert( std::make_pair(iNpcID, pReputationSystem) );
	}

	if( pReputationSystem )
	{
#if defined(PRE_ADD_WEEKLYEVENT)
#if defined( _SERVER )
		int nThreadID = 0;
		float fEventValue = g_pDataManager->GetWeeklyEventValuef(WeeklyEvent::Player, 0, WeeklyEvent::Event_8, nThreadID);
		if (fEventValue != 0.f)
			value += (int)(value * fEventValue);
#endif	// #if defined( _SERVER )
#endif	// #if defined(PRE_ADD_WEEKLYEVENT)

		pReputationSystem->Add( Type, value, bIsReachMax );
#if defined( PRE_ADD_REPUTATION_EXPOSURE )
		m_CommitReputationSystem.insert( std::make_pair( pReputationSystem, m_CommitReputationSystem.size() ) );
#else
		m_CommitReputationSystem.insert( std::make_pair( pReputationSystem, 1 ) );
#endif // #if defined( PRE_ADD_REPUTATION_EXPOSURE )
	}
}

bool CReputationSystemRepository::IsFull( const int iNpcID, IReputationSystem::eType Type )
{
	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor == m_NpcReputation.end() )
		return false;

	if( (*itor).second->GetPercent( Type ) >= 100.f )
		return true;

	return false;
}

bool CReputationSystemRepository::IsExistNpcReputation( int iNpcID )
{
	bool bResult = false;

	std::map<int,IReputationSystem*>::iterator itor = m_NpcReputation.find( iNpcID );
	if( itor != m_NpcReputation.end() )
		bResult = true;

	return bResult;
}

#ifdef _CLIENT
UNIONPT_TYPE CReputationSystemRepository::GetUnionPoint(const int& unionType) const
{
	if (unionType > NpcReputation::UnionType::Etc)
		return -1;

	return m_UnionPoint[unionType];
}

void CReputationSystemRepository::SetUnionPoint(const int& unionType, const UNIONPT_TYPE& value)
{
	if (unionType > NpcReputation::UnionType::Etc)
		return;

	m_UnionPoint[unionType] = value;
}

void CReputationSystemRepository::SetUnionMembershipData(const int& unionType, const int& itemId, const __time64_t& receivedDate, const int& leftUseTime)
{
	std::vector<CDnInterface::SUnionMembershipData>::iterator iter = m_UnionReputationData.begin();
	for (; iter != m_UnionReputationData.end(); ++iter)
	{
		CDnInterface::SUnionMembershipData& data = *iter;
		if (data.unionType == unionType)
		{
			data.itemId = itemId;
			data.leftUseTime = leftUseTime;
			data.expireDate = receivedDate + (leftUseTime * 60);

			return;
		}
	}

	if (itemId > 0)
	{
		CDnInterface::SUnionMembershipData data;
		data.unionType	= unionType;
		data.itemId		= itemId;
		data.expireDate = receivedDate + (leftUseTime * 60);
		data.leftUseTime = leftUseTime;
		m_UnionReputationData.push_back(data);
	}
}

#ifdef PRE_FIX_71455_REPUTE_RENEW
void CReputationSystemRepository::DeleteUnionMembershipData( int nUnionType, int nItemID )
{
	std::vector<CDnInterface::SUnionMembershipData>::iterator iter = m_UnionReputationData.begin();
	for( ; iter != m_UnionReputationData.end(); ++iter )
	{
		CDnInterface::SUnionMembershipData& data = *iter;
		if( data.unionType == nUnionType && data.itemId == nItemID )
		{
			m_UnionReputationData.erase( iter );
			break;
		}
	}
}
#endif // PRE_FIX_71455_REPUTE_RENEW
#endif // _CLIENT

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )