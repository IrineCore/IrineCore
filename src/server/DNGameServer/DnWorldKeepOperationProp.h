#pragma once

#include "DnWorldOperationProp.h"

class CDnPropStateTrigger;
class CDnWorldKeepOperationProp : public CDnWorldOperationProp
{
public:
	CDnWorldKeepOperationProp(CMultiRoom * pRoom);
	virtual ~CDnWorldKeepOperationProp();

	virtual void Process(LOCAL_TIME LocalTime, float fDelta);
	virtual void OnDispatchMessage( CDNUserSession *pSession, DWORD dwProtocol, BYTE *pPacket );

	bool OnClickKeepProp( CDNUserSession * pSession, int nOperationTime, int nStringID, const char * szActionName );
	void OnCancelKeepOperationProp( CDNUserSession * pSession );
	UINT GetClickedSessionID() const { return m_nClickedSessionID; }

	void SendKeepOperationResult( DnActorHandle hActor, bool bRsult );
	void SendKeepOperationStart( DnActorHandle hActor, int nOperationTime, int nStringID, const char * szActionName );
	
protected:
	
	bool m_bClicked;

	LONG m_nPropKeepClickTick;			//How long to wait while clicked

	UINT m_nClickedSessionID;			//Session ID of the player who clicked the prop
	DWORD m_nClickedUniqueID;			//Unique ID of the player who clicked the prop
	LOCAL_TIME m_PropClickedTime;		//Time when prop was clicked

	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );

private:
	void _ClearClickedInfo( CDNUserSession * pSession );
	bool _IsActivated(LOCAL_TIME LocalTime);
	void _CheckKeepProp(LOCAL_TIME LocalTime);
};

class IBoostPoolDnWorldKeepOperationProp : public CDnWorldKeepOperationProp, public TBoostMemoryPool<IBoostPoolDnWorldKeepOperationProp>
{
public:
	IBoostPoolDnWorldKeepOperationProp( CMultiRoom *pRoom ):CDnWorldKeepOperationProp(pRoom){}
	virtual ~IBoostPoolDnWorldKeepOperationProp(){}
};