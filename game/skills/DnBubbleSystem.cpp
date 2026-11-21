#include "stdafx.h"
#include "DnBubbleSystem.h"
#include "DnBubbleConditionChecker.h"
#include "DnObserverEventMessage.h"
#include "DnBubbleEventHandler.h"
#include "DnBubble.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "Stream.h"

#ifdef _CLIENT
#include "DnInterface.h"
#endif

namespace BubbleSystem
{

CDnBubbleSystem::S_DEFINED_BUBBLE_EVENT::~S_DEFINED_BUBBLE_EVENT( void )
{
	SAFE_DELETE_PVEC( vlpConditions );
	SAFE_DELETE_PVEC( vlpEventHandlers );
}


CDnBubbleSystem::CDnBubbleSystem( void )
{

}

CDnBubbleSystem::~CDnBubbleSystem( void )
{
	RemoveAllBubbles( false );

	// Release all loaded info.
	SAFE_DELETE_PVEC( m_vlpDefinedBubbleEvent );

	//map<int, IDnBubbleEventHandler*>::iterator iter = m_mapBubbleRemoveEventHandlers.begin();
	//for( iter; iter != m_mapBubbleRemoveEventHandlers.end(); ++iter )
	//{
	//	SAFE_DELETE( iter->second );
	//}
}


void CDnBubbleSystem::Initialize( DnActorHandle hActor )
{	
	// Event processing is handled only on the game server. Client only receives packets.
#ifdef _GAMESERVER
	// Currently, the bubble system is only available for players.
	if( false == hActor->IsPlayerActor() )
		return;

	m_hActor = hActor;

	// Fetch bubble table info for the job class and build the data.
	DNTableFileFormat* pBubbleTable = GetDNTable( CDnTableDB::TSKILLBUBBLE );
	char acBuffer[ 512 ] = { 0 };
	int iItemCount = pBubbleTable->GetItemCount();
	for( int iIndex = 0; iIndex < iItemCount; ++iIndex )
	{
		int iBubbleTableID = pBubbleTable->GetItemID( iIndex );

		// Check if there is a class restriction. Add later if there are more.
		// If 0, no job restriction. If a job restriction exists, search job history and collect only matching entries.
		int iNeedJobCode = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, "_NeedJob" )->GetInteger();
		if( 0 < iNeedJobCode )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
			bool bAvailableJob = pPlayerActor->IsPassJob( iNeedJobCode );
			if( false == bAvailableJob )
			{
				// Required class not met.
				continue;
			}
		}

		// Create new bubble event definition struct.
		S_DEFINED_BUBBLE_EVENT* pNewBubbleInfo = new S_DEFINED_BUBBLE_EVENT;
		pNewBubbleInfo->iTableID = iBubbleTableID;

		// Build condition info.
		for( int i = 0; i < 5; ++i )
		{
			sprintf_s( acBuffer, "_ConditionType%d", i+1 );
			int iConditionType = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, acBuffer )->GetInteger();

			sprintf_s( acBuffer, "_ConditionFactor%d", i+1 );
			string strArgument = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, acBuffer )->GetString();

			// If result is NULL, condition type is set to 0 in table. i.e., empty.
			IDnConditionChecker* pConditionChecker = IDnConditionChecker::Create( iConditionType, strArgument.c_str() );
			if( pConditionChecker )
			{
				pNewBubbleInfo->vlpConditions.push_back( pConditionChecker );

				// Link specific events with meaningful bubble event info here by condition type to reduce search time.
				// Essentially sorts each condition by its associated event message type.
				int iEventType = _GetRelatedEventMessageType( iConditionType );
				if( NONE_BUBBLE_EVENT_MESSAGE != iEventType )
					m_mmapDefinedByEvent.insert( make_pair(iEventType, pNewBubbleInfo) );
				//////////////////////////////////////////////////////////////////////////
			}
			else
				break;
		}

		// Construct event handler info.
		for( int i = 0; i < 5; ++i )
		{
			sprintf_s( acBuffer, "_BubbleType%d", i+1 );
			int iEventHandlerType = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, acBuffer )->GetInteger();

			sprintf_s( acBuffer, "_BubbleFactor%d", i+1 );
			string strArgument = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, acBuffer )->GetString();

			IDnBubbleEventHandler* pEventHandler = IDnBubbleEventHandler::Create( iEventHandlerType, hActor, strArgument.c_str() );
			if( pEventHandler )
			{
				pNewBubbleInfo->vlpEventHandlers.push_back( pEventHandler );
				_OnCreateEventHandler( pBubbleTable, iBubbleTableID, pEventHandler, strArgument.c_str() );
			}
			else
				break;
		}

		m_vlpDefinedBubbleEvent.push_back( pNewBubbleInfo );
	}
#endif
}

void CDnBubbleSystem::_OnCreateEventHandler( DNTableFileFormat* pBubbleTable, int iBubbleTableID, IDnBubbleEventHandler* pEventHandler, const char* pArgument )
{
#ifdef _GAMESERVER
	// If this is a bubble add event handler, also add icon info; icon info will be sent to the client later.
	if( BUBBLE_HANDLER::GETTING_BUBBLE == pEventHandler->GetType() )
	{
		int iIconIndex = pBubbleTable->GetFieldFromLablePtr( iBubbleTableID, "_IconIndex" )->GetInteger();
		static_cast<CDnGettingBubbleHandler*>(pEventHandler)->SetIconIndex( iIconIndex );
	}
#endif // #ifdef _GAMESERVER


	//// When removing state effects added by bubbles, release them directly in the bubble system.
	//if( BUBBLE_HANDLER::ADD_STATE_EFFECT == pEventHandler->GetType() )
	//{
	//	// Added condition type and event handler to remove the corresponding state effect when a bubble is removed.
	//	IDnBubbleEventHandler* pRemoveEventHandler = IDnBubbleEventHandler::Create( BUBBLE_HANDLER::REMOVE_STATE_EFFECT, m_hActor, pArgument );
	//	m_mapBubbleRemoveEventHandlers[ static_cast<CDnAddStateEffectHandler*>(pEventHandler)->GetBubbleTypeID() ] = pRemoveEventHandler;
	//}
}

// For externally sent messages, pre-associate events to reduce search time.
// When EVENT_BUBBLE_ON_USE_SKILL message arrives, search in event definition info sorted by ON_USE_SKILL condition.
// Therefore, at least one uncleaned event must exist here for events to be processed.
// This part may be removed later.. 
int CDnBubbleSystem::_GetRelatedEventMessageType( int iConditionType )
{
	int iResultMessageType = NONE_BUBBLE_EVENT_MESSAGE;

	switch( iConditionType )
	{
		case ON_USE_SKILL:
			{
				iResultMessageType = EVENT_BUBBLE_ON_USE_SKILL;
			}
			break;

		case BLOCK_SUCCESS:
			{
				iResultMessageType = EVENT_BUBBLE_BLOCK_SUCCESS;
			}
			break;

		case BUBBLE_COUNT_UPDATED:
			{
				iResultMessageType = EVENT_BUBBLE_COUNT_UPDATED;
			}
			break;

		case PARRING_SUCCESS:
			{
				iResultMessageType = EVENT_BUBBLE_PARRING_SUCCESS;
			}
			break;

		case COOLTIME_PARRING_SUCCESS:
			{
				iResultMessageType = EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS;
			}
			break;

		case DO_NORMAL_ATTACK:
			{
				iResultMessageType = EVENT_ONCHANGEACTION;
			}
			break;

		case PLAYER_KILL_TARGET_ON_GHOUL_MODE:
			{
				iResultMessageType = EVENT_PLAYER_KILL_TARGET;
			}
			break;

		case ON_USE_SKILL_WITH_SPECIFIC_SKILLLEVEL:
			{
				iResultMessageType = EVENT_BUBBLE_ON_USE_SKILL;
			}

		case ONCRITICALHIT:
			{
				iResultMessageType = EVENT_ONCRITICALHIT;
			}
			break;
	}

	return iResultMessageType;
}


void CDnBubbleSystem::Clear( void )
{
	RemoveAllBubbles();
}


void CDnBubbleSystem::SetDurationTime( int iBubbleTypeID, float fDurationTime )
{
	// Change the duration of all remaining bubbles to the newly acquired bubble's duration.
	deque<CDnBubble*>& dqBubbles = m_mapBubblesByTypeID[ iBubbleTypeID ];
	for( int i = 0; i < (int)dqBubbles.size(); ++i )
	{
		CDnBubble* pBubble = dqBubbles.at( i );
		pBubble->SetDurationTime( fDurationTime );
	}
}


void CDnBubbleSystem::_CreateBubble( const S_CREATE_BUBBLE& Info )
{
	CDnBubble* pNewBubble = _CreateNewBubble( Info );
	_OnCreatedBubble( pNewBubble );
}


CDnBubble* CDnBubbleSystem::_CreateNewBubble( const S_CREATE_BUBBLE &Info )
{
	CDnBubble* pNewBubble = new CDnBubble;
	pNewBubble->SetTypeID( Info.iBubbleTypeID );
	pNewBubble->SetIconIndex( Info.iIconIndex );

	m_mapBubblesByTypeID[ Info.iBubbleTypeID ].push_front( pNewBubble );
	SetDurationTime( Info.iBubbleTypeID, Info.fDurationTime );

	return pNewBubble;
}


void CDnBubbleSystem::_OnCreatedBubble( CDnBubble* pCreatedBubble )
{	
	// Handle bubble count update event.
	if( pCreatedBubble )
	{
		boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_BUBBLE_COUNT_UPDATED ) );
		pEvent->SetBubbleTypeID( pCreatedBubble->GetTypeID() );
		OnEvent( pEvent );

	}
}

//void CDnBubbleSystem::_OnRemoveBubble( CDnBubble* pBubbleToRemove )
//{
//	// TODO: Remove state effects when a bubble is removed as defined in the table..
//	// If state effect is granted by bubbles, remove the old count-based state effect and add a new one based on remaining count.
//
//}

void CDnBubbleSystem::_OnRemovedBubbles( int iBubbleTypeID, int iCount )
{
	//map<int, IDnBubbleEventHandler*>::iterator iter = m_mapBubbleRemoveEventHandlers.find( iBubbleTypeID );
	//if( m_mapBubbleRemoveEventHandlers.end() != iter )
	//{
	//	iter->second->ProcessEvent( this, NULL );
	//}

	// Handle bubble count update event.
	boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_BUBBLE_COUNT_UPDATED ) );
	pEvent->SetBubbleTypeID( iBubbleTypeID );
	OnEvent( pEvent );
}


int CDnBubbleSystem::GetBubbleCountByTypeID( int iBubbleTypeID )
{
	int iResult = 0;

	BubblesByTypeID_iter iter= m_mapBubblesByTypeID.find( iBubbleTypeID );
	if( m_mapBubblesByTypeID.end() != iter )
	{
		iResult = (int)iter->second.size();
	}

	return iResult;	
}

void CDnBubbleSystem::GetAllAppliedBubbles( /*OUT*/ vector<S_BUBBLE_INFO>& vlBubbleInfos )
{
	BubblesByTypeID_iter iter = m_mapBubblesByTypeID.begin();
	for( iter; iter != m_mapBubblesByTypeID.end(); ++iter )
	{
		// For now, ignore remaining duration on each bubble and delete in order of earliest added.
		CDnBubble* pBubble = iter->second.front();
		S_BUBBLE_INFO Info;
		Info.iBubbleTypeID = pBubble->GetTypeID();
		Info.iCount = (int)iter->second.size();
		Info.iIconIndex = pBubble->GetIconIndex();
		Info.fDurationTime = pBubble->GetDurationTime();
		Info.fRemainTime = pBubble->GetRemainTime();

		vlBubbleInfos.push_back( Info );
#ifdef _CLIENT
		//GetInterface().SetBubble(Info.iCount,Info.fRemainTime,Info.fDurationTime);
#endif
	}
}


void CDnBubbleSystem::AddBubble( const S_CREATE_BUBBLE& Info )
{
	_CreateBubble( Info );
}

void CDnBubbleSystem::AddBubbleAndCountRevision( const S_CREATE_BUBBLE& Info )
{
	_CreateBubble( Info );
	RevisionBubbleCount( Info );
}


void CDnBubbleSystem::RevisionBubbleCount( const S_CREATE_BUBBLE& Info )
{
	// Match bubble count to the server's count.
	deque<CDnBubble*>& dqBubbles = m_mapBubblesByTypeID[ Info.iBubbleTypeID ];
	int iClientBubbleCount = (int)dqBubbles.size();
	if( iClientBubbleCount < Info.iServerBubbleCount )
	{
		int iAddCount = Info.iServerBubbleCount - iClientBubbleCount;
		for( int i = 0; i < iAddCount; ++i )
			_CreateNewBubble( Info );
	}
	else
	if( iClientBubbleCount > Info.iServerBubbleCount )
	{
		int iRemoveCount = iClientBubbleCount - Info.iServerBubbleCount;
		RemoveBubbleByTypeID( Info.iBubbleTypeID, iRemoveCount );
	}
}


void CDnBubbleSystem::RemoveBubbleByTypeID( int iBubbleTypeID, int iRemoveCount )
{
	BubblesByTypeID_iter iter= m_mapBubblesByTypeID.find( iBubbleTypeID );
	if( m_mapBubblesByTypeID.end() != iter )
	{
		// For now, ignore remaining duration on each bubble and delete in order of earliest added.
		deque<CDnBubble*>& dqpBubbles = iter->second;
		for( int i = 0; i < iRemoveCount; ++i )
		{
			// Add later if needed.
			//_OnRemoveBubble( &dqpBubbles.back() );
			dqpBubbles.pop_back();
		}

		// Remove container when all bubbles of that type are gone.
		if( dqpBubbles.empty() )
			m_mapBubblesByTypeID.erase( iter );

		_OnRemovedBubbles( iBubbleTypeID, iRemoveCount );
	}
}


void CDnBubbleSystem::RemoveAllBubbleByTypeID( int iBubbleTypeID )
{
	BubblesByTypeID_iter iter= m_mapBubblesByTypeID.find( iBubbleTypeID );
	if( m_mapBubblesByTypeID.end() != iter )
	{
		int iRemovedBubbleCount = (int)iter->second.size();		

		SAFE_DELETE_PVEC( iter->second );
		m_mapBubblesByTypeID.erase( iter );

		_OnRemovedBubbles( iBubbleTypeID, iRemovedBubbleCount );
	}
}


void CDnBubbleSystem::RemoveAllBubbles( bool bHandleRemoveEvent /*= true*/ )
{
	//SAFE_DELETE_PVEC( m_vlpBubbles );
	BubblesByTypeID_iter iter = m_mapBubblesByTypeID.begin();
	for( iter; iter != m_mapBubblesByTypeID.end(); ++iter )
	{
		int iBubbleTypeID = iter->first;
		int iRemovedBubbleCount = (int)iter->second.size();		

		SAFE_DELETE_PVEC( iter->second );

		// If called from the destructor, do not process the bubble destruction event.
		if( bHandleRemoveEvent )
			_OnRemovedBubbles( iter->first, iRemovedBubbleCount );
	}

	//m_vlpBubbles.clear();
	m_mapBubblesByTypeID.clear();
}


CDnBubble* CDnBubbleSystem::GetBubble( int iBubbleTypeID )
{
	CDnBubble* pResult = NULL;

	BubblesByTypeID_iter iter= m_mapBubblesByTypeID.find( iBubbleTypeID );
	if( m_mapBubblesByTypeID.end() != iter )
	{
		deque<CDnBubble*>& dqBubbles = iter->second;
		if( dqBubbles.empty() )
		{
			pResult = dqBubbles.front();
		}
	}

	return pResult;
}


void CDnBubbleSystem::Process( LOCAL_TIME LocalTime, float fDelta )
{
	BubblesByTypeID_iter iter = m_mapBubblesByTypeID.begin();
	for( iter; iter != m_mapBubblesByTypeID.end(); )
	{
		deque<CDnBubble*>& dqpBubbles = iter->second;
		int iNumBubbles = (int)dqpBubbles.size();
		deque<CDnBubble*>::iterator iterDeque = dqpBubbles.begin();
		for( iterDeque; iterDeque != iter->second.end(); )
		{
			CDnBubble* pBubble = *iterDeque;
			pBubble->Process( LocalTime, fDelta );

			if( pBubble->IsEnd() )
			{	
				iterDeque = dqpBubbles.erase( iterDeque );
				continue;
			}

			++iterDeque;
		}

		int iNumRemoved = iNumBubbles - (int)dqpBubbles.size();
		if( 0 < iNumRemoved )
			_OnRemovedBubbles( iter->first, iNumRemoved );

		if( dqpBubbles.empty() )
			iter = m_mapBubblesByTypeID.erase( iter );
		else
			++iter;
	}
}


void CDnBubbleSystem::OnEvent( boost::shared_ptr<::IDnObserverNotifyEvent> pEvent )
{
	IDnObserverNotifyEvent* pEventObject = pEvent.get();

	if( pEvent == NULL )
		return;

	pair<DefinedByEventMMap_iter, DefinedByEventMMap_iter> iter_pair = m_mmapDefinedByEvent.equal_range( pEvent->GetEventType() );
	DefinedByEventMMap_iter iter = iter_pair.first;
	for( iter; iter != iter_pair.second; ++iter )
	{
		S_DEFINED_BUBBLE_EVENT* pDefinedEventInfo = iter->second;

		// Whether all conditions are met.
		bool bResult = true;
		int iNumConditions = (int)pDefinedEventInfo->vlpConditions.size();
		for( int iCondition = 0; iCondition < iNumConditions; ++iCondition )
		{
			IDnConditionChecker* pCondition = pDefinedEventInfo->vlpConditions.at( iCondition );
			bResult = pCondition->IsSatisfy( this, pEventObject );
			if( false == bResult )
				break;
		}

		// If even one condition is not met, this is unrelated to the defined bubble info..
		if( false == bResult )
			continue;

		// All conditions met. Execute as defined in data...
		int iNumEventHandler = (int)pDefinedEventInfo->vlpEventHandlers.size();
		for( int iEventHandler = 0; iEventHandler < iNumEventHandler; ++iEventHandler )
		{
			IDnBubbleEventHandler* pEventHandler = pDefinedEventInfo->vlpEventHandlers.at( iEventHandler );
			pEventHandler->ProcessEvent( this, pEventObject );
		}
	}
}

#ifdef _GAMESERVER
void CDnBubbleSystem::AddBubbleStateBlow( int iBubbleTypeID, int iBlowID )
{
	m_mapBubbleStateBlowsByBubbleType[ iBubbleTypeID ].push_back( iBlowID );
}

void CDnBubbleSystem::RemoveBubbleStateBlow( int iBubbleTypeID )
{
	map<int, vector<int> >::iterator iter = m_mapBubbleStateBlowsByBubbleType.find( iBubbleTypeID );
	if( m_mapBubbleStateBlowsByBubbleType.end() != iter )
	{
		vector<int>& vlBlowIDs = iter->second;
		for( int i = 0; i < (int)vlBlowIDs.size(); ++i )
		{
			int iBlowID = vlBlowIDs.at( i );
			m_hActor->CmdRemoveStateEffectFromID( iBlowID );		// It should be deleted after one frame on the server, need to check.
		}
		iter->second.clear();
	}
}
#else
void CDnBubbleSystem::CreateBubbleFromPacketStream( ::CPacketCompressStream* pStream )
{
	if( NULL == pStream )
		return;

	S_CREATE_BUBBLE Info;
	
	pStream->Read( &Info.iBubbleTypeID, sizeof(int) );
	pStream->Read( &Info.iIconIndex, sizeof(int) );
	pStream->Read( &Info.fDurationTime, sizeof(float) );
	pStream->Read( &Info.iServerBubbleCount, sizeof(int) );

	this->AddBubbleAndCountRevision( Info );
}

void CDnBubbleSystem::RefreshBubbleDurationTimeFromPacketStream( ::CPacketCompressStream* pStream )
{
	if( NULL == pStream )
		return;

	S_CREATE_BUBBLE Info;

	pStream->Read( &Info.iBubbleTypeID, sizeof(int) );
	pStream->Read( &Info.iIconIndex, sizeof(int) );
	pStream->Read( &Info.fDurationTime, sizeof(float) );
	pStream->Read( &Info.iServerBubbleCount, sizeof(int) );

	SetDurationTime( Info.iBubbleTypeID, Info.fDurationTime );
	RevisionBubbleCount( Info );

}
#endif // #ifdef _GAMESERVER

} // namespace BubbleSystem