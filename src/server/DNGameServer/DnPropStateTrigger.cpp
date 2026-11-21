#include "stdafx.h"
#include "DnPropStateTrigger.h"
#include "DnWorldActProp.h"
#include "DnWorld.h"


CDnPropStateTrigger::CDnPropStateTrigger( DnPropHandle hEntity ) : CDnPropState( hEntity )
{
	m_iType = TRIGGER;
}

CDnPropStateTrigger::~CDnPropStateTrigger(void)
{
}


bool CDnPropStateTrigger::OnEntry( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnEntry( LocalTime, fDelta );

#ifdef ENABLE_PROP_STATE_LOG
	if( m_strTriggerFuncName.empty() )
		OutputDebug( "[?? Prop FSM Log] CDnPropStateTrigger::OnEntry FuncName is NULL!\n");
	else
	{
		if( m_bUseLastOperateSetting )
			OutputDebug( "[Prop(%d) FSM Log] CDnPropStateTrigger::OnEntry FuncName \"%s\", Use last operate setting to world\n", m_hEntity->GetUniqueID(), m_strTriggerFuncName.c_str() );
		else
			OutputDebug( "[Prop(%d) FSM Log] CDnPropStateTrigger::OnEntry FuncName \"%s\"\n", m_hEntity->GetUniqueID(), m_strTriggerFuncName.c_str() );
	}
#endif

	if( !m_hEntity )
		return false;

	CDnWorldActProp* pActProp = static_cast<CDnWorldActProp*>(m_hEntity.GetPointer());
	if( NULL == pActProp )
	{
#ifdef ENABLE_PROP_CONDITION_LOG
		OutputDebug( "[?? Prop FSM Condition Log] CDnPropCondiDurability::IsSatisfy - Entity Prop is NULL!\n" );
#endif
		return false;
	}


	_ASSERT( !m_strTriggerFuncName.empty() );
	if( m_strTriggerFuncName.empty() )
		return false;

	CDnWorld& World = CDnWorld::GetInstance( m_hEntity->GetRoom() );

	AddFuncParam( "LastOperationProp", ( m_hEntity->GetMySmartPtr() ) ? m_hEntity->GetMySmartPtr()->GetCreateUniqueID() : -1 );
	for( DWORD i=0; i<m_VecParamList.size(); i++ ) {
		World.InsertTriggerEventStore( m_VecParamList[i].szParamName.c_str(), m_VecParamList[i].nValue );
	}

	World.OnTriggerEventCallback( m_strTriggerFuncName.c_str(), LocalTime, 0.f );

	m_VecParamList.clear();
	pActProp->ResetOperateState();

	return true;
}


bool CDnPropStateTrigger::OnProcess( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnProcess( LocalTime, fDelta );

	return true;
}


bool CDnPropStateTrigger::OnLeave( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPropState::OnLeave( LocalTime, fDelta );

#ifdef ENABLE_PROP_STATE_LOG
	OutputDebug( "[Prop(%d) FSM Log] CDnPropStateTrigger::OnLeave\n", m_hEntity->GetUniqueID() );
#endif

	return true;
}

void CDnPropStateTrigger::AddFuncParam( const char *pParamName, int nValue )
{
	ParamStruct Struct;
	Struct.szParamName = pParamName;
	Struct.nValue = nValue;

	m_VecParamList.push_back( Struct );
}