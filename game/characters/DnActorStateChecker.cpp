#include "stdafx.h"
#include "DnActorStateChecker.h"
#include "DnActor.h"

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnActorStateChecker::CDnActorStateChecker( DnActorHandle hActor, 
										    CDnActorState::ActorStateEnum eRequiredState, int iOperator ) : IDnSkillUsableChecker( hActor ),
																											m_iOperator( iOperator ),
																											m_eRequiredState( eRequiredState )
{
	_ASSERT( 0 <= m_iOperator && m_iOperator < OP_COUNT && "Invalid operator assignment" );
	m_iType = STATE_CHECKER;
}

CDnActorStateChecker::~CDnActorStateChecker(void)
{

}

bool CDnActorStateChecker::CanUse( void )
{
	bool bResult = false;

	if( m_hHasActor )
	{
		// Each element of DnActorState::StateEnum is declared to allow bit combinations.
		switch( m_iOperator )
		{
			case OP_EQUAL:
				bResult = ( m_hHasActor->GetState() == m_eRequiredState );
				break;
			
			case OP_NOT_EQUAL:
				bResult = ( m_hHasActor->GetState() != m_eRequiredState );

#ifdef _GAMESERVER
				// After using an attack skill, immediately chaining a skill unusable during attacks can hit this check and fail on the server.
				// This happens when server and client frames drift slightly out of sync; exclude the 'while attacking' condition for now since it occurs frequently.
				if((m_eRequiredState == CDnActorState::ActorStateEnum::Attack) && (bResult == false) ) 
					bResult = true;
#endif

				break;

			case OP_AND:
				bResult = ( m_hHasActor->GetState() & m_eRequiredState ) ? true : false;
				break;

			case OP_NOT_AND:
				bResult = !( m_hHasActor->GetState() & m_eRequiredState ) ? true : false;
				break;
		}
	}

	return bResult;
}

IDnSkillUsableChecker* CDnActorStateChecker::Clone()
{
	CDnActorStateChecker* pNewChecker = new CDnActorStateChecker(m_hHasActor, m_eRequiredState, m_iOperator);

	return pNewChecker;
}
