#include "StdAfx.h"
#include "DnTriggerCameraType2.h"
#include "DnInterface.h"
#include "DnBlindDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnTriggerCameraType2::CDnTriggerCameraType2( DnCameraHandle hCurCamera, MatrixEx &BeginCross, MatrixEx &OffsetCross, int nDelay, int nTotalAngle, bool bStartVel, bool bEndVel )
{
	m_CameraType = CameraTypeEnum::TriggerControl2Camera;
	m_hSourceCamera = hCurCamera;
	m_nDelay = nDelay;
	m_nTotalAngle = nTotalAngle;
	m_CrossOffset[0] = BeginCross;
	m_CrossOffset[1] = OffsetCross;
	m_StartTime = 0;

	m_fCameraDistance = EtVec3Length( &EtVector3( m_CrossOffset[0].m_vPosition - m_CrossOffset[1].m_vPosition ) );
	m_bStartVel = bStartVel;
	m_bEndVel = bEndVel;
	m_bBlindControl = false;
}

CDnTriggerCameraType2::~CDnTriggerCameraType2()
{
}

bool CDnTriggerCameraType2::Activate()
{
	m_CameraInfo = *m_hSourceCamera->GetCameraInfo();
	m_hCamera = EternityEngine::CreateCamera( &m_CameraInfo );
	if( !m_hCamera ) return false;

	m_matExWorld = m_CrossOffset[0];
	m_matExWorld.m_vZAxis = m_CrossOffset[1].m_vPosition - m_CrossOffset[0].m_vPosition;
	EtVec3Normalize( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vZAxis );
	m_matExWorld.MakeUpCartesianByZAxis();
	m_hCamera->Update( m_matExWorld );

	CDnBlindDlg *pDlg = GetInterface().GetBlindDialog();
	if( pDlg && pDlg->GetBlindMode() <= CDnBlindDlg::modeOpened ) m_bBlindControl = false;
	else m_bBlindControl = true;

	if( m_bBlindControl ) GetInterface().OpenBlind();

	return true;
}

bool CDnTriggerCameraType2::DeActivate()
{
	CDnCamera::DeActivate();
	SAFE_RELEASE_SPTR( m_hCamera );
	return true;
}

void CDnTriggerCameraType2::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_StartTime == 0 ) m_StartTime = LocalTime;

	float fWeight = ( 1.f / (float)m_nDelay ) * ( LocalTime - m_StartTime );

	if( fWeight > 1.f ) {
		if( m_bBlindControl ) GetInterface().CloseBlind( true );
		CDnCamera::SetActiveCamera( m_hSourceCamera );
		SetDestroy();
		return;
	}
	if( fWeight < 0.5f ) {
		if( m_bStartVel ) {
			fWeight += fWeight;
			fWeight = abs( cos( EtToRadian( 90.f * fWeight ) ) - 1.f ) * 0.5f;
		}
	}
	else {
		if( m_bEndVel ) {
			fWeight -= 0.5f;
			fWeight += fWeight;

			fWeight = 0.5f + ( abs( cos( EtToRadian( 90.f + ( 90.f * fWeight ) ) ) ) * 0.5f );
		}
	}

	m_matExWorld = m_CrossOffset[0];
	m_matExWorld.m_vZAxis = m_CrossOffset[1].m_vPosition - m_CrossOffset[0].m_vPosition;
	EtVec3Normalize( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vZAxis );
	m_matExWorld.MakeUpCartesianByZAxis();

	m_matExWorld.MoveLocalZAxis( m_fCameraDistance );
	m_matExWorld.RotateYAxis( m_nTotalAngle * fWeight );
	m_matExWorld.MoveLocalZAxis( -m_fCameraDistance );

	ProcessEffect( LocalTime, fDelta );
	m_hCamera->Update( m_matExWorld );
}
