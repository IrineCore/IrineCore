#include "stdafx.h"
#include "DnWorldEnvironmentProp.h"
#include "TaskManager.h"
#include "DnPartyTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif



CDnWorldEnvironmentProp::CDnWorldEnvironmentProp( void ) : m_fRange( 0.0f ), m_bActorIsInRange( false )
{

}


CDnWorldEnvironmentProp::~CDnWorldEnvironmentProp( void )
{
	ReleasePostCustomParam();
}

bool CDnWorldEnvironmentProp::InitializeTable( int nTableID )
{
	bool bResult = CDnWorldActProp::InitializeTable( nTableID );
	if( bResult )
	{
		if( GetData() )
		{
			EnvironmentPropStruct* pEnvironmentPropStruct = static_cast<EnvironmentPropStruct*>( m_pData );
			m_fRange = pEnvironmentPropStruct->fRange;

			if( pEnvironmentPropStruct->OnRangeInActionName && pEnvironmentPropStruct->OnRangeOutActionName &&
				strlen( pEnvironmentPropStruct->OnRangeInActionName ) > 0 && strlen( pEnvironmentPropStruct->OnRangeOutActionName ) > 0 )
			{
				m_strOnRangeInAction = pEnvironmentPropStruct->OnRangeInActionName;
				m_strOnRangeOutAction = pEnvironmentPropStruct->OnRangeOutActionName;
			}
		}
	}

	return bResult;
}


void CDnWorldEnvironmentProp::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnWorldActProp::Process( LocalTime, fDelta );

	if( m_strOnRangeOutAction.empty() || m_strOnRangeOutAction.empty() )
		return;

	// Take a specific action when a party member enters or leaves range.
	CDnPartyTask* pPartyTask = static_cast<CDnPartyTask*>( CTaskManager::GetInstance().GetTask( "PartyTask" ) );

	if( pPartyTask )
	{
		m_bActorIsInRange = false;
		DWORD dwNumParty = pPartyTask->GetPartyCount();
		for( DWORD i = 0; i < dwNumParty; ++i )
		{
			CDnPartyTask::PartyStruct* pPartyStruct = pPartyTask->GetPartyData( i );
			if( pPartyStruct->hActor )
			{
				EtVector3* pActorPos = pPartyStruct->hActor->GetPosition();
				float fLength = EtVec3Length( &EtVector3(m_matExWorld.m_vPosition - *pActorPos) );
				if( fLength < m_fRange )
				{
					m_bActorIsInRange = true;
					break;
				}
			}
		}
	}

	// First entry into region; must respond immediately.
	if( m_bActorIsInRange )
	{
		if( strcmp( GetCurrentAction(), "Idle" ) == 0 )
			SetActionQueue( m_strOnRangeInAction.c_str() );
	}
	else
	{
		// Next action is not specified after Summon_Off.
		// Therefore, if the action plays to the end, OnFinishAction is not called, so check here.
		if( strlen( GetCurrentAction() ) == 0 )
			SetActionQueue( m_strOnRangeOutAction.c_str() );
	}
}


void CDnWorldEnvironmentProp::OnFinishAction( const char* szPrevAction, const char *szNextAction, LOCAL_TIME time )
{
	if( m_strOnRangeOutAction.empty() || m_strOnRangeOutAction.empty() )
		return;

	// Exited the region.
	// Action must not switch immediately during Summon_On action, so check here when Summon_On action ends.
	if( false == m_bActorIsInRange )
	{
		if( strcmp( szPrevAction, m_strOnRangeInAction.c_str() ) == 0 )
			SetActionQueue( m_strOnRangeOutAction.c_str() );
	}
}