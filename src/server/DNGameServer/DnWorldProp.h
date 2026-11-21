#pragma once

#pragma once

#include "EtWorldProp.h"
#include "Timer.h"
#include "EtMatrixEx.h"
#include "MPTransAction.h"
#include "DnActor.h"
#include "PropHeader.h"
#include "IStateMessage.h"
#include "TDnFSMState.h"

class CDnWorldProp : public CEtWorldProp, public CMultiSmartPtrBase< CDnWorldProp, MAX_SESSION_COUNT >, public MPTransAction
{
public:
	CDnWorldProp( CMultiRoom *pRoom );
	virtual ~CDnWorldProp();

protected:
	EtMatrix m_matWorld;
	MatrixEx m_Cross;
	bool m_bIsStaticCollision;
	bool m_bShow;
	bool m_bProjectileSkip;
	bool m_bEnableAggro;
	bool m_bEnableOperator;

	PropTypeEnum m_PropType;

#ifdef PRE_FIX_PROP_RANDOM_VISIBLE
	bool m_bRandomVisibleProp;
	#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	PropDef::Option::eValue m_VisibleOptions[PropDef::Option::eMaxVisibleOption];
	#else
	bool m_bRandomResultIsVisible;
	#endif
#endif // #ifdef PRE_FIX_PROP_RANDOM_VISIBLE

protected:
	virtual bool InitializeTable( int nTableID );
	virtual bool CreateObject();

public:
	static DnPropHandle FindPropFromUniqueID( CMultiRoom *pRoom, DWORD dwUniqueID );

	// CEtWorldProp Message
	virtual bool Initialize( CEtWorldSector *pParentSector, const char *szPropName, EtVector3 &vPos, EtVector3 &vRotate, EtVector3 &vScale );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta ) {}
	virtual void EnableIgnoreBuildColMesh( bool bEnable );

	// CDnWorldProp Message
	virtual void OnDamage( CDnDamageBase *pHitter, CDnDamageBase::SHitParam &HitParam ) {}
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdAction( DnActorHandle hActor, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ) {}
	virtual void CmdShow( bool bShow, CDNUserSession* pBreakIntoGameSession=NULL );
	virtual void CmdOperation() {}
	virtual void CmdEnableOperator( DnActorHandle hActor, bool bEnable ) {}
	virtual void CmdChatBalloon( int nUIStringIndex );
#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	bool SetShowInfo(PropDef::Option::eTypeWithPriority option, PropDef::Option::eValue value);
	bool IsVisible() const;
#endif

	virtual DnActorHandle GetLastAccessActor( void ) { return CDnActor::Identity(); };

	void UpdateMatrix();
	MatrixEx *GetMatEx() { return &m_Cross; }

	PropTypeEnum GetPropType() { return m_PropType; }

//	void SetUniqueID( DWORD dwValue ) { m_dwUniqueID = dwValue; }
//	DWORD GetUniqueID() { return m_dwUniqueID; }
	DWORD GetUniqueID() { return GetCreateUniqueID(); }
	bool IsShow() { return m_bShow; }
	bool IsStaticCollision() { return m_bIsStaticCollision; }
	bool IsProjectileSkip(){ return m_bProjectileSkip; }
	bool IsEnableAggro(){ return m_bEnableAggro; }

	void EnableCollision( bool bEnable );
	// Message sent from FSM.
	virtual void OnMessage( const boost::shared_ptr<IStateMessage>& pMessage ) {  };

	virtual void OnFSMStateEntry( const TDnFSMState<DnPropHandle>* pState ) {};
	virtual void OnFSMStateProcess( const TDnFSMState<DnPropHandle>* pState ) {};
	virtual void OnFSMStateLeave( const TDnFSMState<DnPropHandle>* pState ) {};
	
	virtual void SetLifeTime( LOCAL_TIME LifeTimeMS ) {};

	// Whether it is a breakable-type prop.
	virtual bool IsBrokenType( void ) { return false; };

//#if defined(PropOwnerSetting)
protected:
	DnActorHandle m_hMasterActor;
public:
	DnActorHandle GetMasterActor() { return m_hMasterActor; }
	void SetMasterActor(DnActorHandle hActor) { m_hMasterActor = hActor; }
//#endif // Prop owner setting feature

	void EnableOperator( bool bEnable );

	void* AllocPropData(int& usingCount);
private:
#ifdef PRE_FIX_PROP_SHOW_BY_TRIGGER
	void ClearVisibleOptions();
	void SendPropShowInfo();
#endif
};

class IBoostPoolDnWorldProp:public CDnWorldProp, public TBoostMemoryPool< IBoostPoolDnWorldProp >
{
public:
	IBoostPoolDnWorldProp( CMultiRoom *pRoom ):CDnWorldProp(pRoom){}
	virtual ~IBoostPoolDnWorldProp(){}
};
