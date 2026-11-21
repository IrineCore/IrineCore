
#include "stdafx.h"
#include "NpcReputationSystem.h"
#if defined( _SERVER )
#include "DNGameDataManager.h"
#else
#include "DnTableDB.h"
#endif // #if defined( _SERVER )

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

CNpcReputationSystem::CNpcReputationSystem( const int iNpcID, IReputationSystemEventHandler* pHandler )
:IReputationSystem(pHandler),m_iNpcID(iNpcID),m_Favor(std::make_pair(0,0)),m_Malice(std::make_pair(0,0))
{
}

// Read Max value from _ReputeTable. 
void CNpcReputationSystem::Init()
{
#if defined( _SERVER )
	TReputeTableData* pReputationTable = g_pDataManager->GetReputeTableData( m_iNpcID );
	_ASSERT( pReputationTable );
	if( pReputationTable )
	{
		m_Favor.second	= pReputationTable->iMaxFavor;
		m_Malice.second = pReputationTable->iMaxMalice;
	}
#else
	DNTableFileFormat* pNpcReputeTable = GetDNTable( CDnTableDB::TREPUTE );

	vector<int> vlReputeList;
	pNpcReputeTable->GetItemIDListFromField( "_NpcID", m_iNpcID, vlReputeList );
	_ASSERT( 1 == vlReputeList.size() );
	m_Favor.second = pNpcReputeTable->GetFieldFromLablePtr( vlReputeList.front(), "_MaxFavor" )->GetInteger();

	vlReputeList.clear();
	pNpcReputeTable->GetItemIDListFromField( "_NpcID", m_iNpcID, vlReputeList );
	_ASSERT( 1 == vlReputeList.size() );
	m_Malice.second = pNpcReputeTable->GetFieldFromLablePtr( vlReputeList.front(), "_MaxMalice" )->GetInteger();
#endif // #if defined( _SERVER )
}

REPUTATION_TYPE	CNpcReputationSystem::Get( const eType Type )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
			return m_Favor.first;
		case IReputationSystem::NpcMalice:
			return m_Malice.first;
	}

	return -1;
}

REPUTATION_TYPE	CNpcReputationSystem::GetMax( const eType Type )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
			return m_Favor.second;
		case IReputationSystem::NpcMalice:
			return m_Malice.second;
	}

	return 0;
}

float CNpcReputationSystem::GetPercent( const eType Type )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
		{
			if( m_Favor.second > 0 )
				return m_Favor.first/static_cast<float>(m_Favor.second)*100;
			break;
		}
		case IReputationSystem::NpcMalice:
		{
			if( m_Malice.second > 0 )
				return m_Malice.first/static_cast<float>(m_Malice.second)*100;
			break;
		}
	}

	return 0.f;
}

bool CNpcReputationSystem::IsComplete( const eType Type )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
		{
			if( m_Favor.second > 0 )
				return (m_Favor.first >= m_Favor.second);
			break;
		}
		case IReputationSystem::NpcMalice:
		{
			if( m_Malice.second > 0 )
				return (m_Malice.first >= m_Malice.second);
			break;
		}
	}	

	return false;
}

// * value can also be negative.
void CNpcReputationSystem::_Add( const eType Type, const REPUTATION_TYPE value, bool bIsReachMax )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
		{
			m_Favor.first += value;
			if( m_Favor.first > m_Favor.second )
				m_Favor.first = m_Favor.second;
			if( m_Favor.first < 0 )
				m_Favor.first = 0;
			
			if( bIsReachMax == false )
			{
				if( m_Favor.first == m_Favor.second )
					m_Favor.first = m_Favor.second-1;
			}
			break;
		}
		case IReputationSystem::NpcMalice:
		{
			m_Malice.first += value;
			if( m_Malice.first > m_Malice.second )
				m_Malice.first = m_Malice.second;
			if( m_Malice.first < 0 )	
				m_Malice.first = 0;

			if( bIsReachMax == false )
			{
				if( m_Malice.first == m_Malice.second )
					m_Malice.first = m_Malice.second-1;
			}
			break;
		}
	}	
}

void CNpcReputationSystem::_Set( const eType Type, const REPUTATION_TYPE value )
{
	switch( Type )
	{
		case IReputationSystem::NpcFavor:
		{
			m_Favor.first = value;
			if( m_Favor.first > m_Favor.second )
				m_Favor.first = m_Favor.second;
			if( m_Favor.first < 0 )
				m_Favor.first = 0;
			break;
		}
		case IReputationSystem::NpcMalice:
		{
			m_Malice.first = value;
			if( m_Malice.first > m_Malice.second )
				m_Malice.first = m_Malice.second;
			if( m_Malice.first < 0 )	
				m_Malice.first = 0;
			break;
		}
	}	
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )