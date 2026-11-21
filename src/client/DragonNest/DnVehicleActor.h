#pragma once

#include "DnActor.h"
#include "DnSimpleParts.h"
#include "MASingleBody.h"
#include "MAWalkMovement.h"
#include "DnVehicleState.h"

/// Rotha //
// VehicleActor //


class CEtWorldEventArea;

class CDnVehicleActor : public CDnActor, public CDnVehicleState , public MASingleBody 
{

public:
	CDnVehicleActor( int nClassID, bool bProcess = true );
	virtual ~CDnVehicleActor();

	virtual MAMovementBase* CreateMovement()
	{
		MAMovementBase* pMovement = new MAWalkMovement();
		return pMovement;
	}

	virtual bool IsVehicleActor() { return true; }
	virtual bool Initialize();
	virtual bool LoadSkin( const char *szSkinName, const char *szAniName, bool bTwoPassAlpha);
	virtual int GetMoveSpeed();
	virtual void OnDrop( float fCurVelocity );
	virtual void OnFall( float fCurVelocity );
	virtual void CmdMove( EtVector2 &vDir, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f ){}
	virtual void CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f );
	virtual void CmdStop( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bResetStop = false, bool bForce = false );
	virtual void CmdAction( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, bool bCheckOverlapAction = true,	bool bFromStateBlow = false, bool bSkillChain = false );
	virtual void CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, int nLoopCount, float fFrame, float fBlendFrame );
	virtual void RenderCustom( float fElapsedTime );		
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessInput( LOCAL_TIME LocalTime, float fDelta ){}
	virtual void ProcessHideVehicle( LOCAL_TIME LocalTime, float fDelta );
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket );
	virtual void SetDestroy();
	virtual void SetPartsColor(EtAniObjectHandle hObject,char* Type,EtColor Color); // Set color on parts.
	virtual bool SetActionQueue( const char *szActionName, int nLoopCount = 0, float fBlendFrame = 3.f, float fStartFrame = 0.f, bool bCheck = true, bool bCheckStateEffect = true );
	virtual bool LinkPlayerToVehicle();
	virtual void ProcessVehicle(LOCAL_TIME LocalTime, float fDelta); 
	virtual void ProcessTurning(LOCAL_TIME LocalTime, float fDelta); 
	virtual void ProcessRotateVehicle();
	virtual void SetMagnetPosition( EtVector3 &vPos );
	virtual void Show( bool bShow );

public:
	void SetMyPlayerActor(DnActorHandle hActor){m_hMyActor = hActor;};
	void SetAttachToPlayer(bool bAttach){m_bAttachToPlayer = bAttach;} 
	CDnPlayerActor* GetMyPlayerActor();
	void ChangeHairColor(DWORD dwColor);
	void SetDefaultHairColor();
	void EnableTurning(bool bTrue){m_bProcessTruning = bTrue;}
	bool IsLocalVehicle(){return m_bIsLocalVehicle;} // Am I the local player's mount?
	void SetLocalVehicle(bool bTrue){m_bIsLocalVehicle = bTrue;};
	UINT GetItemID(){return m_nItemID;}
	void SetItemID(UINT nID){m_nItemID = nID;}
	void SetDefaultParts();

	void EquipItem(int nPartIndex);
	void EquipItem(TVehicleItem tInfo);
	void UnEquipItem(Vehicle::Parts::eVehicleParts Type);
	DnSimplePartsHandle GetVehicleParts(Vehicle::Parts::eVehicleParts Slot){return m_hSimpleParts[Slot];}
#ifdef PRE_ADD_VEHICLE_ACTION_STRING
	std::string GetVehicleActionString(){return m_strVehicleActionString;}
	void SetVehicleActionString(const char *szVehicleActionString) { m_strVehicleActionString = szVehicleActionString; }
#endif

protected:

	UINT m_nItemID;             // Index of the mount body itself
	DWORD m_dwHairColor;
	bool m_bAttachToPlayer;     // Is it currently attached to the player?
	bool m_bProcessTruning;
	bool m_bIsLocalVehicle;  
#ifdef PRE_ADD_VEHICLE_ACTION_STRING
	std::string m_strVehicleActionString;
#endif

	DnActorHandle m_hMyActor; // Actor<Player> attached to the mount
	DnSimplePartsHandle m_hSimpleParts[2]; // Add two for now and add more depending on whether more are needed.

	// Key Input value 
	char m_cMovePushKeyFlag;
};
