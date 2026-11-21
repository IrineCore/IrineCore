#include "StdAfx.h"
#include "DnNpcTalkCamera.h"
#include "DnPlayerCamera.h"
#include "DnLocalPlayerActor.h"
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
#include "DnNpcTalkReturnDlg.h"
#endif


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnNpcTalkCamera::CDnNpcTalkCamera( DnCameraHandle hCurCamera, DnActorHandle hTargetActor )
{
	m_CameraType = NpcTalkCamera;

	m_CameraInfo.fFogNear = hCurCamera->GetCameraInfo()->fFogNear;
	m_CameraInfo.fFogFar = hCurCamera->GetCameraInfo()->fFogFar;
	m_CameraInfo.fFar = hCurCamera->GetCameraInfo()->fFar;

	m_hSourceCamera = hCurCamera;
	m_hTargetActor = hTargetActor;
	m_StartTime = 0;
	m_State = TalkNone;
}

CDnNpcTalkCamera::~CDnNpcTalkCamera()
{
}

bool CDnNpcTalkCamera::Activate()
{
	m_CameraInfo = *m_hSourceCamera->GetCameraInfo();
	m_hCamera = EternityEngine::CreateCamera( &m_CameraInfo );
	if( !m_hCamera ) return false;
	if( !m_hTargetActor ) return false;

	SetWorkState( BeginTalk );

	m_StartCross = *m_hSourceCamera->GetMatEx();
	m_StartCross.MoveLocalZAxis( ((CDnPlayerCamera*)m_hSourceCamera.GetPointer())->GetCurrentDistance() );

//	EtVector3 vTemp = *m_hTargetActor->GetPosition();
//	vTemp.y = CDnWorld::GetInstance().GetHeight( *m_hTargetActor->GetPosition() );
	EtVector3 vView = *m_hTargetActor->GetPosition() - *CDnActor::s_hLocalActor->GetPosition();
	vView.y = 0.f;
	float fLength = EtVec3Length( &vView );
	if( fLength > 150.f ) fLength = 150.f;
	else if( fLength < 0.f ) fLength = 0.f;
	EtVec3Normalize( &vView, &vView );

	EtMatrix matTemp;
	EtMatrixIdentity( &matTemp );
	EtMatrixRotationY( &matTemp, D3DXToRadian( -20.f - ( 15.f / 70.f * ( 150.f - fLength ) ) ) );

	EtVec3TransformNormal( &vView, &vView, &matTemp );

	float HeightDistance = 0;
	HeightDistance = CDnPlayerCamera::s_fCenterPushHeight;

	// Adjust Height value because height changes when riding a horse.
	if(CDnActor::s_hLocalActor)
	{
		CDnLocalPlayerActor* pLocalPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		if(pLocalPlayer && pLocalPlayer->IsVehicleMode()) HeightDistance = CDnPlayerCamera::s_fCenterPushHeight / 2.5f;
	}

	m_EndCross.m_vPosition = *CDnActor::s_hLocalActor->GetPosition();
	m_EndCross.m_vPosition += m_StartCross.m_vXAxis * CDnPlayerCamera::s_fCenterPushWidth;
	m_EndCross.m_vPosition.y += HeightDistance;

	m_EndCross.m_vPosition+= -vView * 170.f;
//	m_EndCross.m_vPosition.y += m_hTargetActor->GetPosition()->y - CDnWorld::GetInstance().GetHeight( *m_hTargetActor->GetPosition() );

	m_EndCross.m_vZAxis = vView;
	m_EndCross.m_vYAxis = EtVector3( 0.f, 1.f, 0.f );
	m_EndCross.MakeUpCartesianByZAxis();
	m_EndCross.RotatePitch( 5.f );

	return true;
}

bool CDnNpcTalkCamera::DeActivate()
{
	CDnCamera::DeActivate();
	SAFE_RELEASE_SPTR( m_hCamera );
	return true;
}

void CDnNpcTalkCamera::Process( LOCAL_TIME LocalTime, float fDelta )
{
	switch( m_State ) {
		case BeginTalk:
			{
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
				// Disable button while NPC camera is moving (pressing Back while camera moves breaks the camera).
				CDnNpcTalkReturnDlg* pReturnDlg = GetInterface().GetNpcTalkReturnDlg();
				if(pReturnDlg) pReturnDlg->SetButtonEnable(false);
#endif
				if( m_StartTime == 0 ) m_StartTime = LocalTime;

				float fWeight = ( 1.f / 800.f ) * ( LocalTime - m_StartTime );
				if( fWeight >= 1.f ) {
					fWeight = 1.f;
					SetWorkState( Talking );
				}
				else if( fWeight < 0.5f ) {
					fWeight += fWeight;
					fWeight = abs( cos( EtToRadian( 90.f * fWeight ) ) - 1.f ) * 0.5f;
				}
				else {
					fWeight -= 0.5f;
					fWeight += fWeight;

					fWeight = 0.5f + ( abs( cos( EtToRadian( 90.f + ( 90.f * fWeight ) ) ) ) * 0.5f );
				}

				EtVec3Lerp( &m_matExWorld.m_vPosition, &m_StartCross.m_vPosition, &m_EndCross.m_vPosition, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vXAxis, &m_StartCross.m_vXAxis, &m_EndCross.m_vXAxis, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vYAxis, &m_StartCross.m_vYAxis, &m_EndCross.m_vYAxis, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vZAxis, &m_StartCross.m_vZAxis, &m_EndCross.m_vZAxis, fWeight );
				EtVec3Normalize( &m_matExWorld.m_vXAxis, &m_matExWorld.m_vXAxis );
				EtVec3Normalize( &m_matExWorld.m_vYAxis, &m_matExWorld.m_vYAxis );
				EtVec3Normalize( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vZAxis );
				m_matExWorld.MakeUpCartesianByZAxis();
			}
			break;
		case Talking:
			{
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
				CDnNpcTalkReturnDlg* pReturnDlg = GetInterface().GetNpcTalkReturnDlg();
				if(pReturnDlg) pReturnDlg->SetButtonEnable(true);
#endif
			}
			break;
		case EndTalk:
			{
#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
				// Disable button while NPC camera is moving (pressing Back while camera moves breaks the camera).
				CDnNpcTalkReturnDlg* pReturnDlg = GetInterface().GetNpcTalkReturnDlg();
				if(pReturnDlg) pReturnDlg->SetButtonEnable(false);
#endif
				if( m_StartTime == 0 ) m_StartTime = LocalTime;

				float fWeight = ( 1.f / 800.f ) * ( LocalTime - m_StartTime );
				if( fWeight >= 1.f ) {
					fWeight = 1.f; 
					SetWorkState( RestoreCamera );
				}
				else if( fWeight < 0.5f ) {
					fWeight += fWeight;
					fWeight = abs( cos( EtToRadian( 90.f * fWeight ) ) - 1.f ) * 0.5f;
				}
				else {
					fWeight -= 0.5f;
					fWeight += fWeight;

					fWeight = 0.5f + ( abs( cos( EtToRadian( 90.f + ( 90.f * fWeight ) ) ) ) * 0.5f );
				}

				EtVec3Lerp( &m_matExWorld.m_vPosition, &m_EndCross.m_vPosition, &m_StartCross.m_vPosition, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vXAxis, &m_EndCross.m_vXAxis, &m_StartCross.m_vXAxis, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vYAxis, &m_EndCross.m_vYAxis, &m_StartCross.m_vYAxis, fWeight );
				EtVec3Lerp( &m_matExWorld.m_vZAxis, &m_EndCross.m_vZAxis, &m_StartCross.m_vZAxis, fWeight );
				EtVec3Normalize( &m_matExWorld.m_vXAxis, &m_matExWorld.m_vXAxis );
				EtVec3Normalize( &m_matExWorld.m_vYAxis, &m_matExWorld.m_vYAxis );
				EtVec3Normalize( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vZAxis );
				m_matExWorld.MakeUpCartesianByZAxis();
			}
			break;
		case RestoreCamera:
			{
				CDnCamera::SetActiveCamera( m_hSourceCamera, false );
				SetDestroy();

#ifdef PRE_ADD_QUEST_BACK_DIALOG_BUTTON
				CDnNpcTalkReturnDlg* pReturnDlg = GetInterface().GetNpcTalkReturnDlg();
				if(pReturnDlg) pReturnDlg->SetButtonEnable(true);
#endif
			}
			break;
	}
	m_hCamera->Update( m_matExWorld );
}

void CDnNpcTalkCamera::SetWorkState( WorkState State )
{
	switch( m_State ) {
		/*
		case TalkNone:
			{
				m_State = State;
				CDnCamera::SetActiveCamera( m_hSourceCamera, false );
				SetDestroy();
				return;
			}
			break;
		*/
		case BeginTalk:
			{
				if( State == EndTalk ) {
					m_State = State;
					float fWeight = ( 1.f / 800.f ) * ( m_LocalTime - m_StartTime );
					m_StartTime = m_LocalTime - (LOCAL_TIME)( 800 * ( 1.f - fWeight ) );
					return;
				}
			}
			break;
	}
	m_State = State;
	m_StartTime = 0;
}