#include "stdafx.h"
#include "DnPropState.h"
#include "DnWorldActProp.h"
#include "DnPropStateDoAction.h"
#include "DnPropStateItemDrop.h"
#include "DnPropStateNormal.h"
#ifdef _GAMESERVER
#include "DnPropStateTrigger.h"
#include "DnPropStateCmdOperation.h"
#include "DnPropStateScanActor.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnPropState::CDnPropState(DnPropHandle hEntity) : TDnFSMState<DnPropHandle>( hEntity ),
												   m_iType( COUNT )
{
	// Accepts only CDnWorldActProp type
	_ASSERT( hEntity );
	_ASSERT( dynamic_cast<CDnWorldActProp*>(hEntity.GetPointer()) );
}

CDnPropState::~CDnPropState(void)
{

}


CDnPropState* CDnPropState::Create( DnPropHandle hEntity, int iType )
{
	CDnPropState* pNewState = NULL;

	switch( iType )
	{
		case NORMAL:
			pNewState = new CDnPropStateNormal( hEntity );
			break;

		case DO_ACTION:
			pNewState = new CDnPropStateDoAction( hEntity );
			break;

		case ITEM_DROP:
			pNewState = new CDnPropStateItemDrop( hEntity );
			break;

#ifdef _GAMESERVER
		case TRIGGER:
			pNewState = new CDnPropStateTrigger( hEntity );
			break;

		case CMD_OPERATION:
			pNewState = new CDnPropStateCmdOperation( hEntity );
			break;

		case SCAN_ACTOR:
			pNewState = new CDnPropStateScanActor( hEntity );
			break;
#endif
	}

	return pNewState;
}