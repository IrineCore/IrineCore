#pragma once


#include "InputReceiver.h"
#include "DnVehicleActor.h"


/// Rotha //
// VehicleActor //


class CEtWorldEventArea;

class CDnLocalVehicleActor : public CDnVehicleActor , public CInputReceiver
{

public:

	CDnLocalVehicleActor( int nClassID, bool bProcess = true );
	virtual ~CDnLocalVehicleActor();
	virtual bool Initialize();
	virtual void OnDrop( float fCurVelocity );
	virtual void OnFall( float fCurVelocity );
	virtual void CmdMove( EtVector2 &vDir, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bResetStop = false, bool bForce = false );
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true,	bool bFromStateBlow = false, bool bSkillChain = false );
	virtual void CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, int nLoopCount, float fFrame, float fBlendFrame );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessInput( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessTurning(LOCAL_TIME LocalTime, float fDelta);
	virtual void ProcessVehicle(LOCAL_TIME LocalTime, float fDelta);
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );

public:

	void	SetSyncDatumTick(){ m_dwSyncDatumTick = GetTickCount(); }
	DWORD	GetSyncDatumGap() const { return GetTickCount() - m_dwSyncDatumTick; }

	void ProcessLastValidNavCell( LOCAL_TIME LocalTime, float fDelta );
	void RefreshMoveVector( DnCameraHandle hCamera );
	void ProcessActionSyncView();
	void ProcessPositionRevision( bool bForce = false );
	bool IsAllowMovement();

protected:

	EtVector3 m_vLastValidPos;
	bool m_bResetMoveMsg;
	
	EtVector2 m_vPrevMoveVector;
	std::string m_szPrevMoveAction;
	LOCAL_TIME m_LastSendMoveMsg;

	static int s_nVillageMaximumMoveSendTime;
	static int s_nGameMaximumMoveSendTime;

	// Key Input value 
	bool m_bApplyInputProcess;
	char m_cLastPushDirKeyIndex;
	BYTE m_cLastPushDirKey;
	float m_fDelta;

	// Related to speed hack
	DWORD		m_dwSyncDatumTick;
};
