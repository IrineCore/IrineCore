
#include "stdafx.h"
#include "MAAiProcessor.h"
#include "DnMonsterActor.h"
#include "DNAggroSystem.h"

// Target with the highest aggro
void CMAAiChangeTargetByAggroProcessorByAggro::Process( DnActorHandle hActor )
{
	if( m_dwBeginTime == 0 )
		OnBegin();

	if( timeGetTime()-m_dwBeginTime >= m_uiInterval )
	{
		m_dwBeginTime = timeGetTime();

		if( !hActor->IsMonsterActor() )
		{
			_ASSERT(0);
			return;
		}

		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

		if( !pMonster->GetAggroSystem() )
		{
			_ASSERT(0);
			return;
		}

		DnActorHandle hPrevTarget	= pMonster->GetAggroTarget();
		bool bIsProvocationTarget;
		DnActorHandle hTarget		= pMonster->GetAggroSystem()->OnGetAggroTarget( bIsProvocationTarget, hPrevTarget );
		if( hTarget && pMonster->GetAIBase() )
		{
			pMonster->GetAIBase()->ChangeTarget( hTarget, false );
		}
	}
}

// Closest target
void CMAAiChangeTargetProcessorByNearDiatance::Process( DnActorHandle hActor )
{
	if( m_dwBeginTime == 0 )
		OnBegin();

	if( timeGetTime()-m_dwBeginTime >= m_uiInterval )
	{
		m_dwBeginTime = timeGetTime();

		if( !hActor->IsMonsterActor() )
		{
			_ASSERT(0);
			return;
		}

		CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hActor.GetPointer());

		if( !pMonster->GetAggroSystem() )
		{
			_ASSERT(0);
			return;
		}

		bool bIsProvocationTarget;
		DnActorHandle hCurTarget = pMonster->GetAggroSystem()->OnGetAggroTarget( bIsProvocationTarget );

		std::list<CDNAggroSystem::AggroStruct> AggroList = pMonster->GetAggroSystem()->GetAggroList();
		
		DnActorHandle	hTarget;
		float			fMin = FLT_MAX;
		for( std::list<CDNAggroSystem::AggroStruct>::iterator itor=AggroList.begin() ; itor!=AggroList.end() ; ++itor )
		{
			if( !(*itor).hActor )
				continue;

			// Exclude current target
			if( (*itor).hActor == hCurTarget )
				continue;

			float fSQ = EtVec3LengthSq( &(*hActor->GetPosition()-*(*itor).hActor->GetPosition()) );
			if( fSQ < fMin )
			{
				fMin	= fSQ;
				hTarget = (*itor).hActor;
			}
		}

		if( hTarget && pMonster->GetAIBase() )
		{
			pMonster->GetAIBase()->ChangeTarget( hTarget, false );
		}
	}
}

//*****************************************************************************
// MAAiProcessManager
//*****************************************************************************

CMAAiProcessorManager::~CMAAiProcessorManager()
{
	for( UINT i=0 ; i<m_vProcessor.size() ; ++i )
		SAFE_DELETE( m_vProcessor[i] );
}

void CMAAiProcessorManager::Begin()
{
	for( UINT i=0 ; i<m_vProcessor.size() ; ++i )
		m_vProcessor[i]->OnBegin();
}

void CMAAiProcessorManager::End()
{
	for( UINT i=0 ; i<m_vProcessor.size() ; ++i )
		m_vProcessor[i]->OnEnd();
}

void CMAAiProcessorManager::Process( DnActorHandle hActor )
{
	for( UINT i=0 ; i<m_vProcessor.size() ; ++i )
		m_vProcessor[i]->Process( hActor );
}

bool CMAAiProcessorManager::bLoad( lua_tinker::table& t )
{
	// ChangeTarget
	const char* pszChangeTarget = t.get<const char*>("changetarget");
	if( pszChangeTarget )
	{
		std::string					strString(pszChangeTarget);
		std::vector<std::string>	vSplit;
		std::vector<UINT>			vTableID;

		boost::algorithm::split( vSplit, strString, boost::algorithm::is_any_of(",") );

		for( UINT i=0 ; i<vSplit.size() ; ++i )
			vTableID.push_back( atoi(vSplit[i].c_str()) );

		if( vTableID.size() == 2 )
		{
			switch( vTableID[1] )
			{
				// Target with the highest aggro
				case 0:
				{
					m_vProcessor.push_back( new CMAAiChangeTargetByAggroProcessorByAggro( atoi(vSplit[0].c_str()) ) );
					break;
				}
				// Closest target
				case 1:
				{
					m_vProcessor.push_back( new CMAAiChangeTargetProcessorByNearDiatance( atoi(vSplit[0].c_str()) ) );
					break;
				}
			}
		}
	}

	return true;
}
