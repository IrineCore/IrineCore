
#include "stdafx.h"
#include "MAScanner.h"
#include "DnActor.h"

MAScanner::MAScanner()
{
	_CreateFilter();
	_CreateScanner();
}

void MAScanner::_CreateFilter()
{
	m_vFilterRepository.reserve( IMAScanFilter::Max );
	m_vFilterRepository.push_back( new MAScanSameTeamFilter() );
	m_vFilterRepository.push_back( new MAScanOpponentTeamFilter() );
	m_vFilterRepository.push_back( new MAScanDestActorDieFilter() );
	m_vFilterRepository.push_back( new MAScanDestActorNpcFilter() );
	m_vFilterRepository.push_back( new MAScanMinDistanceFilter() );
	m_vFilterRepository.push_back( new MAScanEqualActorFilter() );
	
	_ASSERT( m_vFilterRepository.size() == IMAScanFilter::Max );
}

void MAScanner::_CreateScanner()
{
	_CreateOpponentTeamScanner();
	_CreateMonsterSkillSameTeamScanner();
	_CreateMonsterSkillSameTeamExpectMeScanner();
	_CreateMonsterSkillOpponentTeamScanner();
}

void MAScanner::_CreateOpponentTeamScanner()
{
	m_vScanner[MAScanner::eType::OpponentTeamScan].push_back( _GetFilter( IMAScanFilter::eFilterType::SameTeam ) );
	m_vScanner[MAScanner::eType::OpponentTeamScan].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorDie ) );
	m_vScanner[MAScanner::eType::OpponentTeamScan].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorNpc ) );
}

void MAScanner::_CreateMonsterSkillSameTeamScanner()
{
	m_vScanner[MAScanner::eType::MonsterSkillSameTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorNpc ) );
	m_vScanner[MAScanner::eType::MonsterSkillSameTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::OpponentTeam ) );
	m_vScanner[MAScanner::eType::MonsterSkillSameTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorDie ) );
	m_vScanner[MAScanner::eType::MonsterSkillSameTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::MinDistance ) );
}

void MAScanner::_CreateMonsterSkillSameTeamExpectMeScanner()
{
	m_vScanner[MAScanner::eType::MonsterSkillSameTeamExpectMe].insert( m_vScanner[MAScanner::eType::MonsterSkillSameTeamExpectMe].end(), m_vScanner[MAScanner::eType::MonsterSkillSameTeam].begin(), m_vScanner[MAScanner::eType::MonsterSkillSameTeam].end() );
	m_vScanner[MAScanner::eType::MonsterSkillSameTeamExpectMe].push_back( _GetFilter( IMAScanFilter::eFilterType::EqualActor ) );
}

void MAScanner::_CreateMonsterSkillOpponentTeamScanner()
{
	m_vScanner[MAScanner::eType::MonsterSkillOpponentTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorNpc ) );
	m_vScanner[MAScanner::eType::MonsterSkillOpponentTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::SameTeam ) );
	m_vScanner[MAScanner::eType::MonsterSkillOpponentTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::DestActorDie ) );
	m_vScanner[MAScanner::eType::MonsterSkillOpponentTeam].push_back( _GetFilter( IMAScanFilter::eFilterType::MinDistance ) );
}

IMAScanFilter* MAScanner::_GetFilter( IMAScanFilter::eFilterType Type )
{
	_ASSERT( Type < IMAScanFilter::Max );
	return m_vFilterRepository[Type];
}

MAScanner::~MAScanner()
{
	for( UINT i=0 ; i<m_vFilterRepository.size() ; ++i )
		delete m_vFilterRepository[i];
	m_vFilterRepository.clear();
}

void MAScanner::Scan( MAScanner::eType Type, DnActorHandle hActor, float fMinRange, float fMaxRange, DNVector(DnActorHandle)& vOutputActor )
{
	DNVector(DnActorHandle) vScanActor;

	CDnActor::ScanActor( hActor->GetRoom(), *(hActor->GetPosition()), fMaxRange, vScanActor );

	if( !vScanActor.empty() )
	{
		for( UINT i=0; i<vScanActor.size(); i++ ) 
		{
			bool bCheck = true;
			for( UINT j=0 ; j<m_vScanner[Type].size() ; ++j )
			{
				if( m_vScanner[Type][j]->bIsCheck( hActor, vScanActor[i], fMinRange ) )
				{
					bCheck = false;
					break;
				}
			}

			if( bCheck )
				vOutputActor.push_back( vScanActor[i] );
		}
	}
}
