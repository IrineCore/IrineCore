#include "stdafx.h"
#include "DnWorldProp.h"
#include "DnPropStateScanActor.h"
#include "SMScanActor.h"
#include <boost/shared_ptr.hpp>


CDnPropStateScanActor::CDnPropStateScanActor( DnPropHandle hEntity ) : CDnPropState( hEntity ),
																	   m_fScanRange( 0.0f )
{
	m_iType = SCAN_ACTOR;
}

CDnPropStateScanActor::~CDnPropStateScanActor(void)
{
}

bool CDnPropStateScanActor::OnEntry( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnEntry( LocalTime, fDelta );

#ifdef _DEBUG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateScanActor::OnEntry\n", m_hEntity->GetUniqueID() );
#endif

	return true;
}

bool CDnPropStateScanActor::OnProcess( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnProcess( LocalTime, fDelta );

	if( 0.0f == m_fScanRange )
		return false;
	
	DNVector(DnActorHandle) vlhScanResult;
	CDnActor::ScanActor( m_hEntity->GetRoom(), m_hEntity->GetMatEx()->m_vPosition, m_fScanRange, vlhScanResult );

	//if( false == vlhScanResult.empty() )
	//{
		// delete is handled automatically by shared_ptr.
		boost::shared_ptr<IStateMessage> pScanActorMsg = boost::shared_ptr<IStateMessage>(new CSMScanActor( vlhScanResult ));
		m_hEntity->OnMessage( pScanActorMsg );
	//}

	return true;
}


bool CDnPropStateScanActor::OnLeave( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnLeave( LocalTime, fDelta );

#ifdef _DEBUG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateScanActor::OnLeave\n", m_hEntity->GetUniqueID() );
#endif

	return true;
}