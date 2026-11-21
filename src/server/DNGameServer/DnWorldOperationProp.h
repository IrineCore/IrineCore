#pragma once

#include "DnWorldActProp.h"
#include "DnActor.h"
#include "DnDropItem.h"

class CDnPropState;
class CDnPropStateTrigger;
class CDnWorldOperationProp : public CDnWorldActProp
{
public:
	enum
	{
		OFF,
		ON,
	};

	CDnWorldOperationProp( CMultiRoom *pRoom );
	virtual ~CDnWorldOperationProp();

	bool m_bIsOperated;		// Becomes true when clicked once from client, then resets back to false after transitioning to TriggerState.
	DNVector(DnActorHandle) m_hVecDisableActor;
	CDnPropStateTrigger* m_pTriggerActionState;
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	DNVector(DnActorHandle) m_hVecClickActor;
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
protected:
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );

	void CmdOperarionProp(CDNUserSession * pSession);
	void DiableActorManage( DnActorHandle hActor, bool bEnable );

public:
	virtual int GetOperateState( void ) { return m_bIsOperated ? 1 : 0; };
	virtual void ResetOperateState( void ) { m_bIsOperated = false; };

	virtual void OnSyncComplete( CDNUserSession* pBreakIntoGameSession=NULL );
	virtual void CmdEnableOperator( DnActorHandle hActor, bool bEnable );

	virtual void OnDispatchMessage( CDNUserSession *pSession, DWORD dwProtocol, BYTE *pPacket );
#if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
	bool IsClickPropAndCheckUser(DnActorHandle hActor);
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ADD_REWARD)
};

class IBoostPoolDnWorldOperationProp:public CDnWorldOperationProp, public TBoostMemoryPool< IBoostPoolDnWorldOperationProp >
{
public:
	IBoostPoolDnWorldOperationProp( CMultiRoom *pRoom ):CDnWorldOperationProp(pRoom){}
	virtual ~IBoostPoolDnWorldOperationProp(){}
};
