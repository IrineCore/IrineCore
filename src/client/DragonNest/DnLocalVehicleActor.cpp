#include "stdafx.h"
#include <mmsystem.h>
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnDataManager.h"
#include "InputWrapper.h"
#include "DnWorld.h"
#include "navigationmesh.h"
#include "DnPlayerCamera.h"
#include "DnLocalVehicleActor.h"
#include "DnLocalPlayerActor.h"
#include "DnActorClassDefine.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "DnMovableChecker.h"
#include "DnJumpableChecker.h"
#include "DnGroundMovableChecker.h"
#include "DnMainMenuDlg.h"


int CDnLocalVehicleActor::s_nVillageMaximumMoveSendTime = 1000;
int CDnLocalVehicleActor::s_nGameMaximumMoveSendTime = 500;

#define REFRESH_VIEWVEC_ANGLE 0.995f
#define REFRESH_VIEWVEC_TICK 200
#define REFRESH_SKILL_VIEWVEC_TICK 300


CDnLocalVehicleActor::CDnLocalVehicleActor(int nClassID, bool bProcess )
: CDnVehicleActor( nClassID, bProcess )
{
	m_bResetMoveMsg = true;
	m_bApplyInputProcess = true;

	m_LastSendMoveMsg = 0;
	m_cMovePushKeyFlag = 0;
	
	m_cLastPushDirKeyIndex = 0;
	m_cLastPushDirKey = 0;
	m_fDelta = 0.f;

	m_vLastValidPos = EtVector3( 0.f, 0.f, 0.f );
	
	// Related to speed hack
	m_dwSyncDatumTick = 0;
}

CDnLocalVehicleActor::~CDnLocalVehicleActor()
{
}

bool CDnLocalVehicleActor::Initialize()
{
	CDnVehicleActor::Initialize();
	return true;
}

void CDnLocalVehicleActor::Process(LOCAL_TIME LocalTime, float fDelta )
{
	if( !CDnActor::s_hLocalActor || !m_hMyActor )
		return;

	m_fDelta = fDelta;

	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( !hCamera )
		return;
	
	if( hCamera->GetCameraType() == CDnCamera::PlayerCamera && 
		m_hMyActor->IsDestroy() == false && 
		GetMyPlayerActor()->IsVehicleMode() && 
		m_hMyActor == pPlayer->GetActorHandle() ) 
	{
		if(!pPlayer->IsLockInput() ) 
			ProcessInput( LocalTime, fDelta );

		if( hCamera->IsFreeze(1) && !IsSignalRange( STE_FreezeCamera ) ) 
			hCamera->SetFreeze( false, 1 );
	}

	RefreshMoveVector( hCamera );

	CDnVehicleActor::Process( LocalTime, fDelta );

	ProcessActionSyncView();
	ProcessPositionRevision();
	ProcessLastValidNavCell(LocalTime,fDelta);

	hCamera->PostProcess( LocalTime, fDelta );
}

void CDnLocalVehicleActor::ProcessInput( LOCAL_TIME LocalTime, float fDelta )
{
	EtVector2 vMoveVec( 0.f, 0.f );

	std::string szAction;
	std::string szCurAction = GetCurrentAction();

	bool bMovable = false;
	m_bApplyInputProcess = true;

	if(!CDnActor::s_hLocalActor)
		return;
	
	CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer());

	bool isInverseMode = false;
	isInverseMode = CInputDevice::IsInverseKeyboard();

	// Move auto-run up because it needs to work even when cursor is visible.
	if( IsMovable() && !IsHit() && !IsDown() && (GetState() != ActorStateEnum::Air)) 
	{
		if( pPlayer->IsAutoRun() ) 
		{
			szAction = "Move_Front";
			vMoveVec.y = GetPushKeyDelta( IW( (isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK ), LocalTime );
			bMovable = true;
		}
	}

	if( GetInterface().IsFocusEditBox() ) // Handling when touching the edit box while moving
	{
		if( IsMove() && IsMovable() && !pPlayer->IsAutoRun() && !pPlayer->IsFollowing() )
		{
			CmdStop("Stand",0,10.f);
		}
		else if(strstr( GetCurrentAction(), "Turn" ))
		{
			CmdStop("Stand",0,10.f);
		}

		m_bApplyInputProcess = false;
	}

	if(m_bApplyInputProcess)           // Key input process
	{
		if(IsMovable() && !IsHit() && !IsDown())
		{
			if( IsPushKey( IW( (isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK )) ) // Cannot move forward while moving left/right 
			{
				if(GetState() == ActorStateEnum::Air)
				{

					if(0 == GetCantXZMoveSEReferenceCount())
						MoveJumpZ(GetPushKeyDelta(IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK),LocalTime) * GetMoveSpeed());
				}
				else
				{
					szAction = "Move_Front";
					vMoveVec.y = GetPushKeyDelta( IW( (isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK ), LocalTime );
					bMovable = true;
				}
				pPlayer->ResetAutoRun();
			}

			if( IsPushKey( IW( (isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT )) ) // Cannot move backward while moving left/right
			{
				if(GetState() == ActorStateEnum::Air)
				{
					if(0 == GetCantXZMoveSEReferenceCount())
						MoveJumpZ(GetPushKeyDelta(IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT),LocalTime) * GetMoveSpeed());
				}
				else
				{
					szAction = "Move_Back";
					vMoveVec.y = -GetPushKeyDelta( IW( (isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT ), LocalTime )/2;
					bMovable = true;
				}
				pPlayer->ResetAutoRun();
			}

			if( !IsPushKey(IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK )) && !IsPushKey(IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT )) && IsPushKey(IW((isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT )))
			{
				if(GetState() == ActorStateEnum::Stay)
				{
					szAction = "Turn_Left";
					bMovable = true;
				}
				pPlayer->ResetAutoRun();
			}

			if(!IsPushKey(IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK )) && !IsPushKey(IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT )) && IsPushKey(IW((isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT )))
			{
				if(GetState() == ActorStateEnum::Stay)
				{
					szAction = "Turn_Right";
					bMovable = true;
				}
				pPlayer->ResetAutoRun();
			}
		}
		else
		{
			ResetPushKeyDelta( IW(IW_MOVEFRONT), LocalTime );
			ResetPushKeyDelta( IW(IW_MOVEBACK), LocalTime );
			ResetPushKeyDelta( IW(IW_MOVELEFT), LocalTime );
			ResetPushKeyDelta( IW(IW_MOVERIGHT), LocalTime );
		}

		if( szCurAction.empty() ) 
		{ 
			szAction = "Stand"; 
		}
		else
		{
			if( IsMovable() && bMovable == false && m_szActionQueue.empty() && strstr( szCurAction.c_str(), "Move") != NULL ) 
			{
				szAction = "Stand";
			}
			else if( strstr(m_szAction.c_str() , "Turn_Right") || strstr(m_szAction.c_str() , "Turn_Left"))
			{
				if(!IsPushKey( IW( (isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT )) && !IsPushKey( IW( (isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT )))
				{
					szAction = "Stand";
				}
			}  // If keys are released while turning, set to the default motion.
		}
	}

	if(bMovable)
	{
		EtVec2Normalize(&vMoveVec,&vMoveVec);
		float fSpeed = (float)GetMoveSpeed();
		vMoveVec *= ( fDelta * fSpeed );

		if(strstr(szAction.c_str(),"Stand") || strstr(szAction.c_str(),"Turn")) // Turning motion and standing jump should be treated as stationary actions.
		{
			if(strstr(GetCurrentAction(),szAction.c_str()) == NULL)
			{
				CmdAction(szAction.c_str(),-1,10.f); // A bit smoother
			}
		}
		else
		{
			CmdMove(vMoveVec,szAction.c_str(),-1,6.f);
		}

		if(GetInterface().GetMainBarDialog())
		{
			GetInterface().GetMainBarDialog()->ShowMinimapOption(false);
#ifdef PRE_ADD_AUTO_DICE
			GetInterface().GetMainBarDialog()->ShowAutoDiceOption( false );
#endif
		}

		if(CDnMouseCursor::IsActive() && CDnMouseCursor::GetInstance().IsShowCursor())
		{
			if(CDnMouseCursor::GetInstance().GetShowCursorRefCount() == 0) { CDnMouseCursor::GetInstance().ShowCursor(false,true); }
		}

		GetInterface().CloseDirectDialog();
	}
	else if (strcmp(szAction.c_str(), "Stand") == NULL )
	{
		if(EtVec3LengthSq(GetMovePos()) == 0.f) 
		{ 
			CmdStop(szAction.c_str(),0,6.f); 
		}
	}
	if( bMovable || pPlayer->IsFollowing() )
	{
		if( GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHATROOM_DIALOG ) && GetInterface().GetMainMenuDialog( CDnMainMenuDlg::CHATROOM_DIALOG )->IsShow() )
			GetInterface().GetMainMenuDialog()->ShowChatRoomCreateDlg( false );
		
		GetInterface().ShowChatRoomPassWordDlg( false );
	}
}

void CDnLocalVehicleActor::ProcessVehicle(LOCAL_TIME LocalTime, float fDelta)
{
	if(!m_hMyActor || !GetMyPlayerActor()->IsVehicleMode() || IsDestroy() || !m_bAttachToPlayer)
		return;

	CDnVehicleActor::ProcessVehicle(LocalTime,fDelta);
}

void CDnLocalVehicleActor::ProcessTurning(LOCAL_TIME LocalTime, float fDelta)
{
	if(!m_hMyActor || !GetMyPlayerActor()->IsVehicleMode() || IsDestroy() || !m_bAttachToPlayer)
		return;

	CDnPlayerCamera *pCamera = (CDnPlayerCamera*)CDnCamera::GetActiveCamera().GetPointer();
	float fRotatePower = 0.f;
	if(pCamera && pCamera->GetCameraType() == CDnCamera::PlayerCamera )
	{
		pCamera->ForceYawCamera(0.f); 
		if( strstr( GetCurrentAction(), "Turn" ))
		{
			if(strstr(GetCurrentAction(),"Left"))
			{
				GetMatEx()->RotateYaw( Vehicle::Common::ROTATE_YAWDELTA  * Vehicle::Common::ROTATE_ANGLEPOWER * fDelta); 
				fRotatePower =  - (float)(Vehicle::Common::ROTATE_YAWDELTA * Vehicle::Common::ROTATE_ANGLEPOWER);
			}
			else if(strstr(GetCurrentAction(),"Right"))
			{
				GetMatEx()->RotateYaw( -Vehicle::Common::ROTATE_YAWDELTA  * Vehicle::Common::ROTATE_ANGLEPOWER * fDelta); 
				fRotatePower =  (float)(Vehicle::Common::ROTATE_YAWDELTA * Vehicle::Common::ROTATE_ANGLEPOWER);
			}
			pCamera->ForceYawCamera(fRotatePower);        
		}
	}

	CDnVehicleActor::ProcessTurning(LocalTime,fDelta);
}

void CDnLocalVehicleActor::ProcessLastValidNavCell( LOCAL_TIME LocalTime, float fDelta )
{
	if( !CDnWorld::IsActive() ) 
		return;

	NavigationMesh *pNavMesh = CDnWorld::GetInstance().GetNavMesh( GetMatEx()->m_vPosition );
	if( !pNavMesh ) 
		return;

	NavigationCell *pCell = pNavMesh->FindCell( m_matexWorld.m_vPosition );
	if( pCell ) 
	{
		m_vLastValidPos = m_matexWorld.m_vPosition;
	}
}

bool CDnLocalVehicleActor::IsAllowMovement()
{
	CTask *pTask = CTaskManager::GetInstance().GetTask( "VillageTask" );
	if( pTask ) 
	{
		if( m_LastSendMoveMsg == 0 || CDnActionBase::m_LocalTime - m_LastSendMoveMsg >= s_nVillageMaximumMoveSendTime ) m_bResetMoveMsg = true;
	}

	char bAutoRun = 0;

	if(m_hMyActor && !m_hMyActor->IsDestroy() && m_hMyActor->GetActorHandle() && m_hMyActor->GetActorHandle() == CDnActor::s_hLocalActor)
	{
		CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor*>(m_hMyActor.GetPointer());

		if(	pPlayer->IsAutoRun())
			bAutoRun = 2;
	}

	bool isInverseMode = false;
	//When input reverse effect is applied
	if (m_hMyActor)
		isInverseMode = CInputDevice::IsInverseKeyboard();

	EtVector2 vCurZVec( GetMoveVectorZ()->x, GetMoveVectorZ()->z );
	if( m_bResetMoveMsg == false ) 
	{
		if( ( IsPushKey( IW((isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT) )	)	== ( (m_cMovePushKeyFlag & 0x01) == 0x01 ) &&
			( IsPushKey( IW((isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT) ) )	== ( (m_cMovePushKeyFlag & 0x02) == 0x02 ) &&
			( IsPushKey( IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK)	)	== ( (m_cMovePushKeyFlag & 0x04) == 0x04 ) || ( bAutoRun == 2 ) == ( (m_cMovePushKeyFlag & 0x04) == 0x04 ) ) &&
			( IsPushKey( IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT) )	)	== ( (m_cMovePushKeyFlag & 0x08) == 0x08 ) &&
			GetState() == ActorStateEnum::Move && EtVec2Dot( &vCurZVec, &m_vPrevMoveVector ) >= REFRESH_VIEWVEC_ANGLE ) 
		{
			return true;
		}

		if( m_LastSendMoveMsg == 0 || CDnActionBase::m_LocalTime - m_LastSendMoveMsg < REFRESH_VIEWVEC_TICK ) 
		{
			if( m_LastSendMoveMsg == 0 ) m_LastSendMoveMsg = ( CDnActionBase::m_LocalTime == 0 ) ? 1 : CDnActionBase::m_LocalTime;
			return true;
		}
	}

	m_bResetMoveMsg = false;
	m_vPrevMoveVector = vCurZVec;
	m_cMovePushKeyFlag = 0;

	if( m_bApplyInputProcess ) 
	{
		if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT) ) ) m_cMovePushKeyFlag |= 0x08;
		if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK) ) || bAutoRun == 2 ) m_cMovePushKeyFlag |= 0x04;
		if( IsPushKey( IW((isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT)) && !IsPushKey(IW(IW_MOVEFRONT)) && !IsPushKey(IW(IW_MOVEBACK)) ) m_cMovePushKeyFlag |= 0x01; // Mounts cannot move diagonally. 
		if( IsPushKey( IW((isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT)) && !IsPushKey(IW(IW_MOVEFRONT)) && !IsPushKey(IW(IW_MOVEBACK)) ) m_cMovePushKeyFlag |= 0x02;// Mounts cannot move diagonally.
	}

	m_LastSendMoveMsg = ( CDnActionBase::m_LocalTime == 0 ) ? 1 : CDnActionBase::m_LocalTime;

	return false;
}


void CDnLocalVehicleActor::ProcessActionSyncView()
{
	if( m_bResetMoveMsg == false ) 
		return;

	if( IsStay() || IsHit() ) 
		return;

	if(m_szAction == "Jump_Stand")
		return;

	EtVector2 vCurZVec( GetMoveVectorZ()->x, GetMoveVectorZ()->z );

	if(m_szAction == "Jump")
	{
		if( m_LastSendMoveMsg == 0 || CDnActionBase::m_LocalTime - m_LastSendMoveMsg < 300 ) 
			return;
	}
	else
	{
		if( EtVec2Dot( &vCurZVec, &m_vPrevMoveVector ) >= 0.995f ) 
			return;

		if( m_LastSendMoveMsg == 0 || CDnActionBase::m_LocalTime - m_LastSendMoveMsg < 100 ) 
			return;
	}


	m_vPrevMoveVector = vCurZVec;
	m_LastSendMoveMsg = CDnActionBase::m_LocalTime;

	BYTE pBuffer[128] = { 0, };
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwGap = GetSyncDatumGap();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

 	Send( eActor::CS_VIEWSYNC, &Stream );
}


void CDnLocalVehicleActor::ProcessPositionRevision( bool bForce )
{
	bool bSendRevision = false;
	if( !bForce ) 
	{
		if( IsStay() || IsHit() ) 
			return;

		int nMoveSendTime = s_nGameMaximumMoveSendTime;
	
		CTask *pTask = CTaskManager::GetInstance().GetTask( "VillageTask" );
		if( pTask ) 
			nMoveSendTime = s_nVillageMaximumMoveSendTime;

		if( m_LastSendMoveMsg == 0 || CDnActionBase::m_LocalTime - m_LastSendMoveMsg < nMoveSendTime ) 
			return;

		bSendRevision = true;
	}
	else
	{
		bSendRevision = true;
	}

	if( bSendRevision ) 
	{
		BYTE pBuffer[128] = { 0, };
		CPacketCompressStream Stream( pBuffer, 128 );
		DWORD dwGap	= GetSyncDatumGap();
		int nMoveSpeed = CDnActor::GetMoveSpeed();

		CTask *pTask = CTaskManager::GetInstance().GetTask( "VillageTask" );
		if( pTask ) {
			if( EtVec3Length( GetMovePos() ) == 0.f ) 
			{
				Stream.Write( &dwGap, sizeof(DWORD) );
				Stream.Write( &m_nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				Stream.Write( &m_cMovePushKeyFlag, sizeof(char) );
				Stream.Write( &nMoveSpeed, sizeof(int) );

				Send( eActor::CS_CMDMOVE, &Stream );
			}
		}

		pTask = CTaskManager::GetInstance().GetTask( "GameTask" );
		if( pTask ) 
		{
			bool bMove	= IsMove();
			Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
			Stream.Write( &dwGap, sizeof(DWORD) );
			Stream.Write( &bMove, sizeof(bool) );
			if( bMove ) 
				Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

			Send( eActor::CS_POSREV, &Stream );
		}

		m_LastSendMoveMsg = CDnActionBase::m_LocalTime;
	}
}


void CDnLocalVehicleActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) 
	{

	case STE_Input:
		{

			if( IsPushMouseButton(2) ) 
				return;

			if( CDnMouseCursor::GetInstance().IsShowCursor() ) 
				return;

			if( IsDie() ) 
				return;

			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( !hCamera || hCamera->GetCameraType() != CDnCamera::PlayerCamera ) 
				return;

			InputStruct *pStruct = (InputStruct *)pPtr;

			bool bCheck = false;
			bool isInverseMode = false;
			
			if (m_hMyActor)
				isInverseMode = CInputDevice::IsInverseKeyboard(); //When input reverse effect is applied


			if( pStruct->nButton <= WM_BUTTON )
			{
				bool bReleaseButton = false;

				int nMouseButton = pStruct->nButton; //When input reverse effect is applied
				if (isInverseMode == true)
				{
					switch(nMouseButton)
					{
					case LM_BUTTON: nMouseButton = RM_BUTTON; break;
					case RM_BUTTON: nMouseButton = LM_BUTTON; break;
					}
				}

				switch( pStruct->nEventType ) 
				{
				case 0:	// Push
					if( IsPushMouseButton( (BYTE)nMouseButton ) ) 
					{
						LOCAL_TIME TimeTemp;
						switch( nMouseButton ) {
					case LM_BUTTON: TimeTemp = GetEventMouseTime( IR_MOUSE_LB_DOWN ); break;
					case RM_BUTTON: TimeTemp = GetEventMouseTime( IR_MOUSE_RB_DOWN ); break;
					case WM_BUTTON: TimeTemp = GetEventMouseTime( IR_MOUSE_WB_DOWN ); break;
						}

						TimeTemp += (int)( m_fDelta * 1000 );
						if( TimeTemp >= SignalStartTime ) 
						{
							bCheck = true;
							bReleaseButton = true;
						}
					}
					break;

				case 1:	// Charge
					bCheck = IsPushMouseButton((BYTE)nMouseButton);
					if (bCheck) {
						CInputReceiver::IR_STATE State;
						switch (nMouseButton) {
						case LM_BUTTON: State = IR_MOUSE_LB_DOWN; break;
						case RM_BUTTON: State = IR_MOUSE_RB_DOWN; break;
						case WM_BUTTON: State = IR_MOUSE_WB_DOWN; break;
						}
						if (GetEventMouseTime(State) < SignalStartTime)
							SetEventMouseTime(State, SignalStartTime);
					}
					break;
				case 2:	// Release
					if (!IsPushMouseButton((BYTE)nMouseButton))
					{
						if (!IsPushMouseButton((BYTE)nMouseButton))
						{
							int nState[2] = { 0, };
							switch (nMouseButton) {
							case LM_BUTTON: nState[0] = IR_MOUSE_LB_UP; nState[1] = IR_MOUSE_LB_DOWN;  break;
							case RM_BUTTON: nState[0] = IR_MOUSE_RB_UP; nState[1] = IR_MOUSE_RB_DOWN;  break;
							case WM_BUTTON:	nState[0] = IR_MOUSE_WB_UP; nState[1] = IR_MOUSE_WB_DOWN;  break;
							}
							LOCAL_TIME PushTime = GetEventMouseTime((IR_STATE)nState[1]);
							LOCAL_TIME ReleaseTime = GetEventMouseTime((IR_STATE)nState[0]);

							int nResult = (int)(ReleaseTime - PushTime);
							if (pStruct->nMinTime == -1 && pStruct->nMaxTime == -1) {
								bCheck = true;
								break;
							}
							if (nResult >= pStruct->nMinTime && (DWORD)nResult <= (DWORD)pStruct->nMaxTime) {
								SetEventMouseTime((IR_STATE)nState[1], LocalTime);
								bCheck = true;
							}
						}
					}
					break;
				case 3:	// DoublePush
					if (IsPushMouseButton((BYTE)nMouseButton)) {
						int nState[2] = { 0, };
						switch (nMouseButton) {
						case LM_BUTTON: nState[0] = IR_MOUSE_LB_UP; nState[1] = IR_MOUSE_LB_DOWN;  break;
						case RM_BUTTON: nState[0] = IR_MOUSE_RB_UP; nState[1] = IR_MOUSE_RB_DOWN;  break;
						case WM_BUTTON:	nState[0] = IR_MOUSE_WB_UP; nState[1] = IR_MOUSE_WB_DOWN;  break;
						}
						LOCAL_TIME PushTime = GetEventMouseTime((IR_STATE)nState[1]);
						LOCAL_TIME ReleaseTime = GetEventMouseTime((IR_STATE)nState[0]);
						int nResult = (int)(ReleaseTime - PushTime);
						if (nResult >= pStruct->nMinTime && (DWORD)nResult <= (DWORD)pStruct->nMaxTime) bCheck = true;
					}
					bReleaseButton = true;
					break;
				case 4: // Press
					if (IsPushMouseButton((BYTE)nMouseButton)) {
						bCheck = true;
						/*
						LOCAL_TIME TimeTemp;
						switch( pStruct->nButton ) {
						case 0: TimeTemp = GetEventMouseTime( IR_MOUSE_LB_DOWN ); break;
						case 1: TimeTemp = GetEventMouseTime( IR_MOUSE_RB_DOWN ); break;
						case 2: TimeTemp = GetEventMouseTime( IR_MOUSE_WB_DOWN ); break;
						}

						if( TimeTemp >= CDnActionBase::m_ActionTime ) {
						bCheck = true;
						//									bReleaseButton = true;
						}
						*/
					}
					break;
				case 5: // DoublePush or Shift+Push
					if (IsPushMouseButton((BYTE)nMouseButton)) {
						int nState[2] = { 0, };
						switch (nMouseButton) {
						case LM_BUTTON: nState[0] = IR_MOUSE_LB_UP; nState[1] = IR_MOUSE_LB_DOWN;  break;
						case RM_BUTTON: nState[0] = IR_MOUSE_RB_UP; nState[1] = IR_MOUSE_RB_DOWN;  break;
						case WM_BUTTON:	nState[0] = IR_MOUSE_WB_UP; nState[1] = IR_MOUSE_WB_DOWN;  break;
						}
						LOCAL_TIME PushTime = GetEventMouseTime((IR_STATE)nState[1]);
						LOCAL_TIME ReleaseTime = GetEventMouseTime((IR_STATE)nState[0]);
						int nResult = (int)(ReleaseTime - PushTime);
						if ((nResult >= pStruct->nMinTime && (DWORD)nResult <= (DWORD)pStruct->nMaxTime) || IsPushKey(DIK_LSHIFT)) bCheck = true;
					}
					bReleaseButton = true;
					break;
				}

				if( bCheck ) 
				{
#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
					if( strstr(pStruct->szChangeAction, "Special_") )
					{
						int nCurrentMapID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
						DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
						if( pSox && pSox->GetFieldFromLablePtr( nCurrentMapID, "_USEVehicleSpecialmotion" )->GetInteger() == 0 )
							break;
					}
#endif
					if( IsCustomAction() ) 
					{
						ResetCustomAction();
						ResetMixedAnimation();
					}

					if( pStruct->szActionBoneName && pStruct->szMaintenanceBoneName && strlen( pStruct->szActionBoneName ) && strlen( pStruct->szMaintenanceBoneName ) ) 
					{
						if( pStruct->nEventType != 2 ) 
							m_VecMixedAni.clear();

						std::string szAction = pStruct->szChangeAction;
						SetCustomAction( pStruct->szChangeAction, (float)pStruct->nChangeActionFrame );
						if( m_szCustomAction.empty() ) 
						{

#if defined(PRE_FIX_68898)
							if (pStruct->isSkipEndAction == TRUE)
							{
								SetSkipEndAction(pStruct->isSkipEndAction == TRUE);
							}
#endif // PRE_FIX_68898
							CmdAction( m_szAction.c_str(), m_nLoopCount, m_fBlendFrame, true, false, ( pStruct->bSkillChain == TRUE ) );
						}
						else 
						{
							CmdMixedAction( pStruct->szActionBoneName, pStruct->szMaintenanceBoneName, pStruct->szChangeAction, 0, (float)pStruct->nChangeActionFrame, (float)pStruct->nBlendFrame );
						}
					}
					else 
					{
#if defined(PRE_FIX_68898)
						if (pStruct->isSkipEndAction == TRUE)
						{
							SetSkipEndAction(pStruct->isSkipEndAction == TRUE);
						}
#endif // PRE_FIX_68898
						CmdAction( pStruct->szChangeAction, 0, (float)pStruct->nBlendFrame, false, false, ( pStruct->bSkillChain == TRUE ) );
					}

					if( bReleaseButton ) 
					{
						ReleasePushButton( (BYTE)nMouseButton );
					}

					if( !IsMovable() ) 
					{
						SetMovable( false );
						ResetMove();
					}
				}
			}
			else if( pStruct->nButton < 10 )
			{
				int nButton = pStruct->nButton;

				// Ignore directional keys when movement is disabled.
				if( 0 < GetCantMoveSEReferenceCount() )
				{
					if( Front_Key <= nButton && nButton <= Right_Key )
						return;
				}
				// Changed jump to check unactionable state. 
				if( 0 < GetCantActionSEReferenceCount() )
				{
					if( Jump_Key == nButton )
						return;
				}

				int nMapButton;

				switch( nButton ) 
				{
				case 3:	nMapButton = IW( (isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK );	break;
				case 4:	nMapButton = IW( (isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT );   break;
				case 5:	nMapButton = IW( (isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT );	break;
				case 6:	nMapButton = IW( (isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT );	break;
				case 7: nMapButton = IW( IW_JUMP );		    break;
				}

				bool bReleaseButton = false;

				switch (pStruct->nEventType)  // Removed unnecessary parts for now; refer to LocalPlayerActor when adding additional input types.
				{
				case 0:	// Push
					if (GetPushKeyTime(nMapButton) >= SignalStartTime && !(SignalStartTime < 0))
					{
						bCheck = IsPushKey(nMapButton);
						bReleaseButton = true;
					}
					break;


				case 1:	// Charge
					bCheck = IsPushKey(nMapButton);
					if (bCheck) SetEventKeyTime(nMapButton, SignalStartTime);
					break;

				case 2:	// Release
					if (!IsPushKey(nMapButton)) {

						if (!IsPushKey(nMapButton))
						{
						/*	LOCAL_TIME PushTime = CDnActor::s_hLocalActor->m_LastPushDirKeyTime[nButton - 3];
							LOCAL_TIME ReleaseTime = CDnActor::s_hLocalActor->m_LastReleaseDirKeyTime[nButton - 3];
							int nResult = (int)(ReleaseTime - PushTime);
							if (pStruct->nMinTime == -1 && pStruct->nMaxTime == -1) {
								bCheck = true;
								break;
							}
							if (nResult >= pStruct->nMinTime && (DWORD)nResult <= (DWORD)pStruct->nMaxTime) bCheck = true;*/
							bCheck = true;
						}
					}
					break;

				case 3:	// DoublePush
						//if (CheckSpecialCommand(LocalTime, nButton + 7)) {
							bCheck = true;
							bReleaseButton = true;
						//}
					break;

				case 4: // Press
					bCheck = IsPushKey(nMapButton);
					break;

				case 5: // DoublePush or Shift+Push
					//if (CDnActor::s_hLocalActor->CheckSpecialCommand(LocalTime, nButton + 7) || (IsPushKey(DIK_LSHIFT) && IsPushKey(nMapButton))) {
						bCheck = true;
						bReleaseButton = true;
					//}
				default:
					break;

				}

				if( bCheck ) 
				{
#ifdef PRE_ADD_VEHICLE_SPECIAL_ACTION
					if( strstr(pStruct->szChangeAction, "Special_") )
					{
						int nCurrentMapID = CGlobalInfo::GetInstance().m_nCurrentMapIndex;
						DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMAP );
						if( pSox && pSox->GetFieldFromLablePtr( nCurrentMapID, "_USEVehicleSpecialmotion" )->GetInteger() == 0 )
							break;
					}
#endif
					DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
					if( hCamera ) 
					{
						EtVector3 vZVec;
						EtVec3Cross( &vZVec, &hCamera->GetMatEx()->m_vXAxis, &EtVector3( 0.f, 1.f, 0.f ) );

						if(strstr(m_szAction.c_str(),"Stand")==NULL)
							Look( EtVec3toVec2( vZVec ) );

						RefreshMoveVector( hCamera );
					}

					CmdAction( pStruct->szChangeAction, 0, (float)pStruct->nBlendFrame);

					if( nButton == 7 ) 
					{
						ReleasePushKey( nMapButton );
					}
					if( !IsMovable() ) 
					{
						SetMovable( false );
						ResetMove();
					}

					if( IsCustomAction() ) 
					{
						ResetCustomAction();
						ResetMixedAnimation();
					}
				}
			}
		}
		break;

	case STE_FreezeCamera:
		{
			FreezeCameraStruct *pStruct = (FreezeCameraStruct *)pPtr;
			DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
			if( hCamera )
			{
				switch( hCamera->GetCameraType() )
				{
				case CDnCamera::PlayerCamera:
					{
						CDnPlayerCamera *pCamera = static_cast<CDnPlayerCamera *>(hCamera.GetPointer());
						if( pCamera && pCamera->GetAttachActor() == GetMySmartPtr() ) 
						{
							pCamera->SetFreeze( pStruct->fResistanceRatio, 1 );
							
							if( CDnActor::s_hLocalActor )
							{
								CDnLocalPlayerActor *pLocalPlayer = static_cast<CDnLocalPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
								if( pLocalPlayer )
									pLocalPlayer->SetIgnoreRefreshViewCamera( ( pStruct->bIgnoreRefreshPlayerView == TRUE ) ? true : false );
							}
						}
					}
					break;

				case CDnCamera::CannonCamera:
					{
						hCamera->SetFreeze( pStruct->fResistanceRatio, 1 );
					}
					break;
				}
			}
		}
		break;


	}

	CDnVehicleActor::OnSignal(Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
	
}

void CDnLocalVehicleActor::CmdMove( EtVector2 &vDir, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	MAMovementBase *pMovement = GetMovement();
	if( !pMovement ) 
		return;

	pMovement->MoveX( vDir.x );
	pMovement->MoveZ( vDir.y );

	bool bAllow = IsAllowMovement();
	SetActionQueue( szActionName, nLoopCount, fBlendFrame );
	if( bAllow ) 
		return;

	if(!m_hMyActor || m_hMyActor->IsDestroy())
		return;

	BYTE pBuffer[128] = { 0, };
	CPacketCompressStream Stream( pBuffer, 128 );
	int nActionIndex = m_hMyActor->GetElementIndex(szActionName); // Action info is character
	DWORD dwGap	= GetSyncDatumGap();
	int nMoveSpeed = CDnActor::GetMoveSpeed();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &m_cMovePushKeyFlag, sizeof(char) );
	Stream.Write( &nMoveSpeed, sizeof(int) );

	Send( eActor::CS_CMDMOVE, &Stream);

}


void CDnLocalVehicleActor::CmdMove( EtVector3 &vPos, const char *szActionName, int nLoopCount, float fBlendFrame )
{
	CDnVehicleActor::CmdMove(vPos,szActionName,nLoopCount,fBlendFrame);

	BYTE pBuffer[128] = { 0, };
	CPacketCompressStream Stream( pBuffer, 128 );

	if(!m_hMyActor || m_hMyActor->IsDestroy()) 
		return;

	int		nActionIndex	= m_hMyActor->GetElementIndex( szActionName );
	DWORD	dwGap			= GetSyncDatumGap();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( GetMovePos(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

	Send( eActor::CS_CMDMOVEPOS, &Stream );
}



void CDnLocalVehicleActor::CmdStop( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, bool bResetStop, bool bForce )
{
	CDnVehicleActor::CmdStop(szActionName,nLoopCount,fBlendFrame,fStartFrame,bResetStop,bForce);

	BYTE pBuffer[128] = { 0, };
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwGap = GetSyncDatumGap();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &bResetStop, sizeof(bool) );

	Send( eActor::CS_CMDSTOP, &Stream );
	m_cMovePushKeyFlag = 0;
}


void CDnLocalVehicleActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, 
								bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{
	CDnVehicleActor::CmdAction( szActionName, nLoopCount, fBlendFrame, bCheckOverlapAction, bFromStateBlow, bSkillChain );

	DWORD dwGap = GetSyncDatumGap();
	m_cMovePushKeyFlag = 0;

	char bAutoRun = 0;

	if(!m_hMyActor || m_hMyActor->IsDestroy())
		return;

	if(m_hMyActor->GetActorHandle() == CDnActor::s_hLocalActor)
	{
		CDnLocalPlayerActor *pPlayer = static_cast<CDnLocalPlayerActor*>(m_hMyActor.GetPointer());
		if(	pPlayer->IsAutoRun())
			bAutoRun = 2;
	}

	bool isInverseMode = false;
	//When input reverse effect is applied
	if (m_hMyActor)
		isInverseMode = CInputDevice::IsInverseKeyboard();

	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK) ) || bAutoRun == 2 ) m_cMovePushKeyFlag |= 0x04;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT) ) ) m_cMovePushKeyFlag |= 0x08;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT) ) ) m_cMovePushKeyFlag |= 0x01;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT) ) ) m_cMovePushKeyFlag |= 0x02;

	int nActionIndex = GetElementIndex( szActionName );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nLoopCount, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2( *GetMoveVectorZ() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &EtVec3toVec2( *GetLookDir() ), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &m_cMovePushKeyFlag, sizeof(char) );
	Stream.Write( &bFromStateBlow, sizeof(bFromStateBlow) );
	Stream.Write( &bSkillChain, sizeof(bSkillChain) );

	Send( eActor::CS_CMDACTION, &Stream );

	m_bResetMoveMsg = true;
	m_vPrevMoveVector = EtVec3toVec2( *GetMoveVectorZ() );
	m_LastSendMoveMsg = CDnActionBase::m_LocalTime;

}


void CDnLocalVehicleActor::CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, int nLoopCount, float fFrame, float fBlendFrame )
{
	CDnVehicleActor::CmdMixedAction( szActionBone, szMaintenanceBone, szActionName, nLoopCount, fFrame, fBlendFrame );

	bool isInverseMode = false;
	//When input reverse effect is applied
	if (m_hMyActor)
		isInverseMode = CInputDevice::IsInverseKeyboard();

	m_cMovePushKeyFlag = 0;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEFRONT : IW_MOVEBACK) ) ) m_cMovePushKeyFlag |= 0x04;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVEBACK : IW_MOVEFRONT) ) ) m_cMovePushKeyFlag |= 0x08;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVELEFT : IW_MOVERIGHT) ) ) m_cMovePushKeyFlag |= 0x01;
	if( IsPushKey( IW((isInverseMode == false) ? IW_MOVERIGHT : IW_MOVELEFT) ) ) m_cMovePushKeyFlag |= 0x02;

	int nActionIndex = GetElementIndex( szActionName );
	int nMaintenanceBoneIndex = GetBoneIndex( szMaintenanceBone );
	int nActionBoneIndex = GetBoneIndex( szActionBone );

	int nBlendAniIndex = m_nAniIndex;
	
	if( !m_VecMixedAni.empty() ) 
		nBlendAniIndex = m_VecMixedAni[0].nBlendAniIndex;

	if( nActionIndex == -1 ) 
		return;

	if( nActionBoneIndex == -1 || nMaintenanceBoneIndex == -1 ) 
		return;

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	DWORD dwGap = GetSyncDatumGap();

	Stream.Write( &dwGap, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nActionBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nMaintenanceBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &fFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
	Stream.Write( &fBlendFrame, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10.f );
	Stream.Write( &m_cMovePushKeyFlag, sizeof(char) );

	Send( eActor::CS_CMDMIXEDACTION, &Stream );

	m_bResetMoveMsg = true;
}


void CDnLocalVehicleActor::RefreshMoveVector( DnCameraHandle hCamera )
{
	if( EtVec3LengthSq( GetMovePos() ) > 0.f ) return;

	// When moving, do not use the character's EtMatrixEx value, instead
	// Set MoveVector because it needs to be based on the camera's Vector value.

	EtVector3 vXVec, vZVec;

	vXVec = hCamera->GetMatEx()->m_vXAxis;
	vZVec = hCamera->GetMatEx()->m_vZAxis;

	vXVec.y = vZVec.y = 0.f;
	EtVec3Normalize( &vXVec, &vXVec );
	EtVec3Normalize( &vZVec, &vZVec );

	SetMoveVectorX( vXVec );
	SetMoveVectorZ( vZVec );

}

void CDnLocalVehicleActor::OnDrop( float fCurVelocity )
{
	if( IsAir() && !IsHit() ) 
	{
		m_bResetMoveMsg = true;

		BYTE pBuffer[128] = { 0, };
		CPacketCompressStream Stream( pBuffer, 128 );
		DWORD dwGap = GetSyncDatumGap();

		Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
		Stream.Write( &dwGap, sizeof(DWORD) );

		Send( eActor::CS_ONDROP, &Stream );
	}

	CDnVehicleActor::OnDrop(fCurVelocity);
}



void CDnLocalVehicleActor::OnFall( float fCurVelocity )
{
	if( ( IsStay() || IsMove() ) && !IsFloorCollision() ) 
	{
		if( fCurVelocity < -5.f ) 
			m_bResetMoveMsg = true;
	}

	CDnVehicleActor::OnFall(fCurVelocity);
}
