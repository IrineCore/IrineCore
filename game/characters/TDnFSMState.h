#pragma once

#include "TDnFSMTransitCondition.h"


// Base class for FSMState.. Must be inherited to be used.. 
template< class EntityHandle >
class TDnFSMState
{
public:
	static int s_iUniqueID;

protected:
	struct S_AVAILABLE_TRANSIT_STATE
	{
		TDnFSMState<EntityHandle>* pFSMState;
		vector< TDnFSMTransitCondition<EntityHandle>* > pvlConditions;

		S_AVAILABLE_TRANSIT_STATE( void ) : pFSMState(NULL)
		{};

		//// Not the cleanest design, but free only Condition memory here...
		//~S_AVAILABLE_TRANSIT_STATE( void ) 
		//{
		//	SAFE_DELETE( pCondition );
		//};
	};

	struct IsEqual : unary_function<const S_AVAILABLE_TRANSIT_STATE&, bool>
	{
		int iUniqueIDToFind;
		IsEqual( int iUniqueID ) : iUniqueIDToFind(iUniqueID) {};

		bool operator () ( const S_AVAILABLE_TRANSIT_STATE& Rhs )
		{
			return Rhs.pFSMState->GetUniqueID() == iUniqueIDToFind;
		}
	};

	int m_iUniqueID;
	EntityHandle m_hEntity;

	vector<S_AVAILABLE_TRANSIT_STATE> m_vlAvailTransitState;

public:
	TDnFSMState( EntityHandle hEntity ) : m_hEntity( hEntity )
	{
		++s_iUniqueID;
		m_iUniqueID = s_iUniqueID;
	};

	virtual ~TDnFSMState(void) {};

	virtual bool OnEntry( LOCAL_TIME LocalTime, float fDelta ) 
	{ 
		if( m_hEntity )
			m_hEntity->OnFSMStateEntry( this ); 

		return true; 
	};
	
	virtual bool OnProcess( LOCAL_TIME LocalTime, float fDelta ) 
	{ 
		if( m_hEntity )
			m_hEntity->OnFSMStateProcess( this ); 
		
		return true; 
	};

	virtual bool OnLeave( LOCAL_TIME LocalTime, float fDelta ) 
	{ 
		if( m_hEntity )
			m_hEntity->OnFSMStateLeave( this ); 
		
		return true; 
	};

	bool AddTransitState( TDnFSMState<EntityHandle>* pFSMState, vector< TDnFSMTransitCondition<EntityHandle>* > pvlCondition )
	{
		// Insert after testing if it is the same state.
		bool bResult = false;

		bool bNewState = true;
		if( false == m_vlAvailTransitState.empty() )
		{
			vector<S_AVAILABLE_TRANSIT_STATE>::iterator iter = find_if( m_vlAvailTransitState.begin(), m_vlAvailTransitState.end(), IsEqual(GetUniqueID()) );
			bNewState = (m_vlAvailTransitState.end() == iter);
			_ASSERT( bNewState && "Cannot add identical States to the Transit State list." );
		}
		if( bNewState )
		{
			S_AVAILABLE_TRANSIT_STATE AvailState;
			AvailState.pFSMState = pFSMState;
			AvailState.pvlConditions = pvlCondition;
			m_vlAvailTransitState.push_back( AvailState );
			bResult = true;
		}

		return bResult;
	}

	bool AddTransitState( TDnFSMState<EntityHandle>* pFSMState, TDnFSMTransitCondition<EntityHandle>* pCondition )
	{
		// Insert after testing if it is the same state.
		bool bResult = false;

		bool bNewState = true;
		if( false == m_vlAvailTransitState.empty() )
		{
			vector<S_AVAILABLE_TRANSIT_STATE>::iterator iter = find_if( m_vlAvailTransitState.begin(), m_vlAvailTransitState.end(), IsEqual(GetUniqueID()) );
			bNewState = (m_vlAvailTransitState.end() == iter);
			_ASSERT( bNewState && "Cannot add identical States to the Transit State list." );
		}
		if( bNewState )
		{
			S_AVAILABLE_TRANSIT_STATE AvailState;
			AvailState.pFSMState = pFSMState;
			AvailState.pvlConditions.push_back( pCondition );
			m_vlAvailTransitState.push_back( AvailState );
			bResult = true;
		}

		return bResult;
	}

	TDnFSMState<EntityHandle>* CanTransition( void )
	{
		TDnFSMState<EntityHandle>* pChangeState = NULL;

		int iNumAvailTransit =	(int)m_vlAvailTransitState.size();
		for( int iState = 0; iState < iNumAvailTransit; ++iState )
		{
			vector< TDnFSMTransitCondition<EntityHandle>* >& pvlCondition = m_vlAvailTransitState.at( iState ).pvlConditions;
			bool bSatisfy = false; 
			
			int iNumCondition = (int)pvlCondition.size();
			for( int iCondition = 0; iCondition < iNumCondition; ++iCondition )
			{
				bSatisfy = pvlCondition.at(iCondition)->IsSatisfy();
				if( false == bSatisfy )
					break;
			}

			if( bSatisfy )
			{
				pChangeState = m_vlAvailTransitState.at( iState ).pFSMState;
				break;
			}
		}

		return pChangeState;
	}

	int GetUniqueID( void ) { return m_iUniqueID; };
};

template< class EntityHandle >
int TDnFSMState<EntityHandle>::s_iUniqueID = 0;