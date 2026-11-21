#include "StdAfx.h"
#include "DnTriggerCameraType1.h"
#include "DnInterface.h"
#include "DnBlindDlg.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnTriggerCameraType1::CDnTriggerCameraType1( DnCameraHandle hCurCamera, MatrixEx &BeginCross, MatrixEx &EndCross, int nDelay, bool bStartVel, bool bEndVel )
{
	m_CameraType = CameraTypeEnum::TriggerControl1Camera;
	m_hSourceCamera = hCurCamera;
	m_nDelay = nDelay;
	m_CrossOffset[0] = BeginCross;
	m_CrossOffset[1] = EndCross;
	m_StartTime = 0;
	m_bStartVel = bStartVel;
	m_bEndVel = bEndVel;
	m_bBlindControl = false;
}

CDnTriggerCameraType1::~CDnTriggerCameraType1()
{
}

bool CDnTriggerCameraType1::Activate()
{
	m_CameraInfo = *m_hSourceCamera->GetCameraInfo();
	m_hCamera = EternityEngine::CreateCamera( &m_CameraInfo );
	if( !m_hCamera ) return false;

	m_hCamera->Update( m_CrossOffset[0] );

	CDnBlindDlg *pDlg = GetInterface().GetBlindDialog();
	if( pDlg && pDlg->GetBlindMode() <= CDnBlindDlg::modeOpened ) m_bBlindControl = false;
	else m_bBlindControl = true;

	if( m_bBlindControl ) GetInterface().OpenBlind();

	return true;
}

bool CDnTriggerCameraType1::DeActivate()
{
	CDnCamera::DeActivate();
	SAFE_RELEASE_SPTR( m_hCamera );
	return true;
}

void CDnTriggerCameraType1::Process( LOCAL_TIME LocalTime, float fDelta )
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


	EtVec3Lerp( &m_matExWorld.m_vPosition, &m_CrossOffset[0].m_vPosition, &m_CrossOffset[1].m_vPosition, fWeight );
	EtVec3Lerp( &m_matExWorld.m_vXAxis, &m_CrossOffset[0].m_vXAxis, &m_CrossOffset[1].m_vXAxis, fWeight );
	EtVec3Lerp( &m_matExWorld.m_vYAxis, &m_CrossOffset[0].m_vYAxis, &m_CrossOffset[1].m_vYAxis, fWeight );
	EtVec3Lerp( &m_matExWorld.m_vZAxis, &m_CrossOffset[0].m_vZAxis, &m_CrossOffset[1].m_vZAxis, fWeight );
	EtVec3Normalize( &m_matExWorld.m_vXAxis, &m_matExWorld.m_vXAxis );
	EtVec3Normalize( &m_matExWorld.m_vYAxis, &m_matExWorld.m_vYAxis );
	EtVec3Normalize( &m_matExWorld.m_vZAxis, &m_matExWorld.m_vZAxis );
	m_matExWorld.MakeUpCartesianByZAxis();

	ProcessEffect( LocalTime, fDelta );
	m_hCamera->Update( m_matExWorld );
}
