#pragma once

#include "DnActor.h"
#include "MAWalkMovement.h"
#include "MASingleBody.h"
#include "DNNpc.h"

class CDNUserBase;

DnActorHandle CreateNpcActor(CMultiRoom *pRoom, UINT nNpcID, TNpcData* pNpcData, EtVector3 Position, float fRotate);

class CDnNPCActor : public CDnActor, public MASingleBody, public CDnNpc, public TBoostMemoryPool< CDnNPCActor >
{
public:

	CDnNPCActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnNPCActor();

	virtual MAMovementBase* CreateMovement();

protected:
	DNVector(DnActorHandle) m_hVecDisableActor;
	DnPropHandle m_hPropHandle;

public:
	virtual bool IsNpcActor(){ return true; }

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void Initialize(TNpcData* pNpcData);
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	virtual bool Initialize();
#endif

	virtual bool Talk(CDNUserBase* pUser, UINT nNpcUniqueID, IN std::wstring& wszIndex, IN std::wstring& wszTarget) override;

	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true, bool bFromStateBlow = false, bool bSkillChain = false );
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f );
	virtual void CmdEnableOperator( DnActorHandle hActor, bool bEnable );

	void StopNpcMove() { m_bNpcMoveActive = false; }
	bool IsNpcMoveActive() const { return m_bNpcMoveActive; }

	// ActionBase
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void OnChangeAction( const char *szPrevAction );
	virtual void OnChangeActionQueue( const char *szPrevAction );

	void SetPropHandle( DnPropHandle hProp ) { m_hPropHandle = hProp; }
	void SetTriggerNpc(){ m_bIsTriggerNpc = true; }
	bool bIsTriggerNpc(){ return m_bIsTriggerNpc; }

private:
	bool m_bIsTriggerNpc;
	bool m_bNpcMoveActive;
	EtVector3 m_vNpcMoveTarget;
	float m_fNpcMoveSpeed;

};