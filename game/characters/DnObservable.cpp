#include "stdafx.h"
#include "DnObservable.h"
#include "DnObserverEventMessage.h"

// CDnObservable /////////////////////////////////////////////////////////
IDnObserverNotifyEvent* IDnObserverNotifyEvent::Create( int iType )
{
	IDnObserverNotifyEvent* pResult = NULL;
	switch( iType )
	{
		case EVENT_BUBBLE_ON_USE_SKILL:
			{
				pResult = new BubbleSystem::CDnUseSkillMessage;
			}
			break;

		case EVENT_BUBBLE_BLOCK_SUCCESS:
			{
				pResult = new BubbleSystem::CDnBlockSuccessMessage;
			}
			break;

		case EVENT_BUBBLE_COUNT_UPDATED:
			{
				pResult = new BubbleSystem::CDnBubbleCountUpdatedMessage;
			}
			break;

		case EVENT_BUBBLE_PARRING_SUCCESS:
			{
				pResult = new BubbleSystem::CDnParringSuccessMessage;
			}
			break;

		case EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS:
			{
				pResult = new BubbleSystem::CDnCooltimeParringSuccessMessage;
			}
			break;

		case EVENT_ONCHANGEACTION:
			{
				pResult = new BubbleSystem::CDnOnChangeActionMessage;
			}
			break;

		case EVENT_PLAYER_KILL_TARGET:
			{
				pResult = new BubbleSystem::CDnPlayerKillTargetMessage;
			}
			break;

		case EVENT_ONCRITICALHIT:
			{
				pResult = new BubbleSystem::CDnOnCriticalHitMessage;
			}
			break;

		default:
			break;
	}

	return pResult;
}
//////////////////////////////////////////////////////////////////////////


// CDnObservable ////////////////////////////////////////////////////////////
CDnObservable::CDnObservable( void )
{

}

CDnObservable::~CDnObservable( void )
{

}

bool CDnObservable::RegisterObserver( CDnObserver* pObserver )
{
	bool bSuccess = false;
	if( pObserver )
	{
		m_vlpObservers.push_back( pObserver );
		bSuccess = true;
	}
	
	return bSuccess;
}

bool CDnObservable::RemoveObserver( CDnObserver* pObserver )
{
	bool bSuccess = false;
	if( pObserver )
	{
		vector<CDnObserver*>::iterator iter = find( m_vlpObservers.begin(), m_vlpObservers.end(), pObserver );
		if( m_vlpObservers.end() != iter )
		{
			m_vlpObservers.erase( iter );
			bSuccess = true;
		}
	}

	return bSuccess;
}

void CDnObservable::Notify( boost::shared_ptr<IDnObserverNotifyEvent> pEvent )
{
	if( pEvent )
	{
		for( int i = 0; i < (int)m_vlpObservers.size(); ++i )
		{
			CDnObserver* pObserver = m_vlpObservers.at( i );
			pObserver->OnEvent( pEvent );
		}
	}
}
//////////////////////////////////////////////////////////////////////////