#include "stdafx.h"
#include "DnTriggerElement.h"
#include "DNGameServerScriptAPI.h"
#include "DNGameRoom.h"
#include "DnTriggerObject.h"
#include "DnWorldSector.h"
#include "DnWorldGrid.h"
#include "DnTrigger.h"

CDnTriggerElement::CDnTriggerElement( CEtTriggerObject *pObject )
: CEtTriggerElement( pObject, ((CDnTrigger*)pObject->GetTrigger())->GetLuaState() )
{
}

CDnTriggerElement::~CDnTriggerElement()
{
}

void CDnTriggerElement::RegisterLuaApi()
{
	if( !m_bSelfOpenLua && ((CDnTrigger*)m_pTriggerObj->GetTrigger())->IsRegisterLuaAPI() ) return;

	if( !m_bSelfOpenLua ) 
		((CDnTrigger*)m_pTriggerObj->GetTrigger())->SetRegisterLuaAPI( true );
	CEtTriggerElement::RegisterLuaApi();
	DefAllAPIFunc( m_pLuaState );


	CDNGameRoom *pRoom = ((CDnWorld*)m_pTriggerObj->GetTrigger()->GetSector()->GetParentGrid()->GetWorld())->GetCurrentTask();
	lua_tinker::set<CDNGameRoom *>( m_pLuaState, "g_room", pRoom );
	lua_tinker::set<CEtTriggerElement *>( m_pLuaState, "g_element", this );
}

bool CDnTriggerElement::CallLuaFunction()
{
	if( !m_bSelfOpenLua ) {
		if( m_pLuaState ) lua_tinker::set<CEtTriggerElement *>( m_pLuaState, "g_element", this );
	}

	return CEtTriggerElement::CallLuaFunction();
}