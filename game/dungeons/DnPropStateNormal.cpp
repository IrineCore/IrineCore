#include "stdafx.h"
#include "DnPropStateNormal.h"
#include "DnWorldActProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPropStateNormal::CDnPropStateNormal( DnPropHandle hEntity ) : CDnPropState(hEntity)
{
	m_iType = NORMAL;
}

CDnPropStateNormal::~CDnPropStateNormal(void)
{
}


bool CDnPropStateNormal::OnEntry( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnEntry( LocalTime, fDelta );

#ifdef _DEBUG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateNormal::OnEntry\n", m_hEntity->GetUniqueID() );
#endif

	bool bResult = true;

	// TODO: Initialization code... 

	return bResult;
}


bool CDnPropStateNormal::OnProcess( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnProcess( LocalTime, fDelta );

	bool bResult = true;

	// Do nothing for now..

	return bResult;
}


bool CDnPropStateNormal::OnLeave( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnLeave( LocalTime, fDelta );

#ifdef ENABLE_PROP_STATE_LOG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateNormal::OnLeave\n", m_hEntity->GetUniqueID() );
#endif

	bool bResult = true;

	return bResult;
}