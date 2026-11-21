#include "stdafx.h"
#include "DnPropNULLCondition.h"
#include "DnWorldProp.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnPropNULLCondition::CDnPropNULLCondition( DnPropHandle hEntity ) : CDnPropCondition(hEntity)
{

}

CDnPropNULLCondition::~CDnPropNULLCondition(void)
{
}

bool CDnPropNULLCondition::IsSatisfy( void )
{
#ifdef ENABLE_PROP_CONDITION_LOG
	OutputDebug( "[Prop(%d) FSM Condition Log] CDnPropNULLCondition::IsSatisfy always return \"true\"\n", m_hEntity->GetUniqueID() );
#endif
	return true;
}