#include "stdafx.h"
#include "DnPropStateDoAction.h"
#include "DnWorldActProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPropStateDoAction::CDnPropStateDoAction( DnPropHandle hEntity ) : CDnPropState( hEntity ),
																	 m_bUseCmdAction( false )
{
	m_iType = DO_ACTION;
}

CDnPropStateDoAction::~CDnPropStateDoAction(void)
{

}

// Randomize if 1 or more. Seed value is received from the prop, because it must match the seed sent in the packet to the client.
bool CDnPropStateDoAction::OnEntry( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnEntry( LocalTime, fDelta );

	if( !m_hEntity )
		return false;

	CDnWorldActProp* pActProp = static_cast<CDnWorldActProp*>(m_hEntity.GetPointer());
	if( NULL == pActProp )
	{
#ifdef ENABLE_PROP_STATE_LOG
		OutputDebug( "[?? Prop FSM Log] CDnPropStateDoAction::OnEntry Entity Prop is NULL!\n" );
#endif
		return false;
	}

#ifdef ENABLE_PROP_STATE_LOG
	string strActions;

	for( int i = 0; i < (int)m_vlAvailActionList.size(); ++i )
	{
		strActions.append( m_vlAvailActionList.at(i) );
		strActions.append( ", " );
	}

	if( 1 < (int)m_vlAvailActionList.size() )
		OutputDebug( "[Prop(%d) FSM Log] CDnPropStateDoAction::OnEntry - %s actions HitRandomSeed: %d\n", m_hEntity->GetUniqueID(), strActions.c_str(), pActProp->GetLastHitRandomSeed() );
	else
	if( m_vlAvailActionList.empty() )
		OutputDebug( "[??Prop(%d) FSM Log] CDnPropStateDoAction::OnEntry action list is EMPTY!!\n", m_hEntity->GetUniqueID(), strActions.c_str() );
	else
	if( m_bUseCmdAction )
		OutputDebug( "[Prop(%d) FSM Log] CDnPropStateDoAction::OnEntry - %s action (Use CmdAction)\n", m_hEntity->GetUniqueID(), strActions.c_str() );
	else
		OutputDebug( "[Prop(%d) FSM Log] CDnPropStateDoAction::OnEntry - %s action\n", m_hEntity->GetUniqueID(), strActions.c_str() );
#endif

	if( m_vlAvailActionList.empty() )
		return false;

	// If there are multiple actions, output randomly..
	int iActionIndex = 0;
	int iNumActionList = (int)m_vlAvailActionList.size();
	if( iNumActionList > 1 )
	{
		int iSeed = pActProp->GetLastHitRandomSeed();

#ifdef _GAMESERVER
		_srand( m_hEntity->GetRoom(), iSeed );
		iActionIndex = _rand( m_hEntity->GetRoom() ) % (int)m_vlAvailActionList.size();
#else
		_srand( iSeed );
		iActionIndex = _rand() % (int)m_vlAvailActionList.size();
#endif
	}

	// Execute action!
	if( false == m_bUseCmdAction )
		pActProp->SetActionQueue( m_vlAvailActionList.at(iActionIndex).c_str() );
	else
		pActProp->CmdAction( m_vlAvailActionList.at(iActionIndex).c_str() ); 

	return true;
}


bool CDnPropStateDoAction::OnProcess( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnProcess( LocalTime, fDelta );

	// Do nothing..

	return true;
}


bool CDnPropStateDoAction::OnLeave( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnLeave( LocalTime, fDelta );

#ifdef ENABLE_PROP_STATE_LOG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateDoAction::OnLeave", m_hEntity->GetUniqueID() );
#endif

	return true;
}