
#include "stdafx.h"
#include "ReputationSystem.h"
#include "ReputationSystemEventHandler.h"

#if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )

void IReputationSystem::Add( const eType Type, const REPUTATION_TYPE value, bool bIsReachMax )
{
	bool bIsBeforeComplete = IsComplete( Type );
	_Add( Type, value, bIsReachMax );
	if( m_pEventHandler )
		m_pEventHandler->OnChange( Type );			// Change Event
	bool bIsAfterComplete = IsComplete( Type );

	if( m_pEventHandler && bIsAfterComplete && !bIsBeforeComplete )
		m_pEventHandler->OnComplete( this, Type );	// Complete Event
}

void IReputationSystem::Set( const eType Type, const REPUTATION_TYPE value )
{
	bool bIsBeforeComplete = IsComplete( Type );
	_Set( Type, value );
	if( m_pEventHandler )
		m_pEventHandler->OnChange( Type );			// Change Event
	bool bIsAfterComplete = IsComplete( Type );

	if( m_pEventHandler && bIsAfterComplete && !bIsBeforeComplete )
		m_pEventHandler->OnComplete( this, Type );	// Complete Event
}

#endif // #if defined( PRE_ADD_NPC_REPUTATION_SYSTEM )
