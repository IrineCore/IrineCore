#pragma once

#include "TDnFSMState.h"
#include "TDnFSMTransitCondition.h"

// Finite state machine; assumes the template parameter object being handled inherits from a smart pointer.
template < class EntityHandle >
class TDnFiniteStateMachine: public TBoostMemoryPool<TDnFiniteStateMachine<EntityHandle>>
{
private:
	EntityHandle m_hEntity;

	TDnFSMState<EntityHandle>* m_pEntryState;
	TDnFSMState<EntityHandle>* m_pNowState;

	vector< TDnFSMState<EntityHandle>* > m_vlpStates;
	vector< TDnFSMTransitCondition<EntityHandle>* > m_vlpConditions;

	TDnFiniteStateMachine( void ) {};
public:
	TDnFiniteStateMachine( EntityHandle hEntity ) : m_hEntity(hEntity), 
													m_pEntryState( NULL ), 
													m_pNowState( NULL ) {};
	~TDnFiniteStateMachine(void)
	{
		SAFE_DELETE_PVEC( m_vlpStates );
		SAFE_DELETE_PVEC( m_vlpConditions );
	}


	void Initialize( void ) 
	{
		// Handled objects must implement functions that configure FSM states.
		//bool bSuccess = m_hEntity->MakeUpFSM();
		//_ASSERT( bSuccess && "Failed to build FSM!" );
	};

	TDnFSMState<EntityHandle>* GetCurrentState( void ) { return m_pNowState; };

	void AddState( TDnFSMState<EntityHandle>* pState )
	{
		m_vlpStates.push_back( pState );
	}

	void AddCondition( TDnFSMTransitCondition<EntityHandle>* pCondition )
	{
		m_vlpConditions.push_back( pCondition );
	}

	void SetEntryState( TDnFSMState<EntityHandle>* pState )
	{
		m_pEntryState= pState;
		m_pNowState = pState;
	}

	void ChangeState( TDnFSMState<EntityHandle>* pState, LOCAL_TIME LocalTime, float fDelta )
	{
		m_pNowState->OnLeave( LocalTime, fDelta );

		m_pNowState = pState;
		m_pNowState->OnEntry( LocalTime, fDelta );
	}

	void Process( LOCAL_TIME LocalTime, float fDelta )
	{
		if( m_pNowState )
		{
			TDnFSMState<EntityHandle>* pState = NULL;
			do
			{
				pState = m_pNowState->CanTransition();

				if( pState )
					ChangeState( pState, LocalTime, fDelta );

			} while( pState );

			m_pNowState->OnProcess( LocalTime, fDelta );

			//TDnFSMState<EntityHandle>* pState = m_pNowState->CanTransition();

			//if( pState )
			//	ChangeState( pState, LocalTime, fDelta );
			//else
			//{
			//	m_pNowState->OnProcess( LocalTime, fDelta );
			//}
		}
	}

};
