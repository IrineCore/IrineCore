
#include "stdafx.h"
#include "DNAggroSystem.h"
#include "DnActor.h"
#include "DnMonsterActor.h"
#include "DnWorldProp.h"
#include "DnStateBlow.h"

CDNAggroSystem::CDNAggroSystem( DnActorHandle hActor )
: m_hActor( hActor )
{
	m_bIgnore = false;
}

CDNAggroSystem::~CDNAggroSystem()
{

}

void CDNAggroSystem::ResetAggro()
{
	m_AggroList.clear();
}

void CDNAggroSystem::ResetAggro( DnActorHandle hActor )
{
	if( !hActor )
		return;

	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; ++itor )
	{
		if( (*itor).hActor == hActor )
		{
			m_AggroList.erase( itor );
			return;
		}
	}
}

void CDNAggroSystem::AddAggro( DnActorHandle hActor, int iAggroValue, bool bForce/*=false*/ )
{
	if( !hActor || m_hActor == hActor )
		return;

	// Aggro does not accumulate among teammates.
	if( m_hActor->GetTeam() == hActor->GetTeam() )
		return;

	// GM intrusion does not draw aggro.
	if( hActor->IsGMTrace() )
		return;

	// 148) Aggro does not accumulate while this state effect is active.
	if( m_hActor && m_hActor->GetStateBlow() && m_hActor->GetStateBlow()->IsApplied( STATE_BLOW::BLOW_148) )
		return;

	// Check CannonActor
	if( hActor->GetActorType() == CDnActorState::ActorTypeEnum::Cannon )
		return;
	if( hActor->GetActorType() == CDnActorState::ActorTypeEnum::NoAggro || hActor->GetActorType() == CDnActorState::ActorTypeEnum::NoAggroTrap )
		return;

	// Check Prop actor
	if( hActor->GetActorType() == CDnActorState::ActorTypeEnum::PropActor )
	{
		bool bCheckProp = false;
		if( hActor->IsMonsterActor() )
		{
			CDnMonsterActor*	pMonster	= static_cast<CDnMonsterActor*>(hActor.GetPointer());
			DnPropHandle		hProp		= pMonster->GetPropHandle();
			if( hProp )
			{
				if( hProp->IsEnableAggro() && hProp->IsShow() )
					bCheckProp = true;
			}
		}
		if( !bCheckProp )
			return;
	}

	// Apply aggro modifier
	OnAggroRegulation( hActor, iAggroValue );

	if( iAggroValue == 0 )
		iAggroValue = 1;

	if( m_bIgnore == true && bForce == false )
		return;

	AggroStruct* pStruct = GetAggroStruct( hActor );
	if( pStruct ) 
		pStruct->iAggro += iAggroValue;
	else 
	{
		if( iAggroValue > 0 )
			m_AggroList.push_back( AggroStruct( hActor, iAggroValue ) );
	}
}

CDNAggroSystem::AggroStruct* CDNAggroSystem::GetAggroStruct( DnActorHandle hActor )
{
	if( !hActor )
		return NULL;

	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; ++itor )
	{
		if( (*itor).hActor == hActor )
		{
			return &(*itor);
		}
	}

	return NULL;
}

void CDNAggroSystem::Convert( DNVector(DnActorHandle)& vOutput )
{
	vOutput.reserve( m_AggroList.size() );
	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; ++itor )
		vOutput.push_back( (*itor).hActor );
}

int CDNAggroSystem::GetAggroSum() const
{
	int aggroSum = 0;
	std::list<AggroStruct>::const_iterator iter = m_AggroList.begin();
	for( ; iter != m_AggroList.end() ; ++iter )
	{
		const AggroStruct& curAggroStruct = *iter;
		aggroSum += curAggroStruct.iAggro;
	}

	return aggroSum;
}