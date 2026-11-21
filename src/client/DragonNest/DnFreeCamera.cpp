#include "StdAfx.h"
#include "DnFreeCamera.h"
#include "InputWrapper.h"
#include "DnWorld.h"
#include "GlobalValue.h"
#include "FrameSync.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnFreeCamera::CDnFreeCamera()
: CInputReceiver( false )
{
	m_CameraType = CameraTypeEnum::FreeCamera;

	m_fSensitivity = 0.1f;
	m_CameraInfo.fFogNear = 1000000.f;
	m_CameraInfo.fFogFar = 1000000.f;
	m_CameraInfo.fFar = 1000000.f;
	m_fSensitivityScale = 1.0f;

	m_fShowSensitivity = 0.0f;
	m_nMoveFlag = 0;
	m_fSlowScale = 1.0f;

	m_PrevLocalTime = 0;
	m_fDelta = 0.0f;
	m_nMouseRBDownPosX = 0;
	m_nMouseRBDownPosY = 0;
}

bool CDnFreeCamera::Activate()
{
	m_hCamera = EternityEngine::CreateCamera( &m_CameraInfo );
	m_hCamera->Activate();

	QueryPerformanceFrequency( &m_qpFreq );
	QueryPerformanceCounter( &m_qpBaseTime );

	UseEventMessage( true );

	return ( m_hCamera ) ? true : false;
}

bool CDnFreeCamera::DeActivate()
{
	CDnCamera::DeActivate();
	SAFE_RELEASE_SPTR( m_hCamera );
	UseEventMessage( false );
	return true;
}

static LOCAL_TIME sTime = 0;

void CDnFreeCamera::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// Use its own timer so it works even in Pause Mode.
	LARGE_INTEGER qpTime;
	QueryPerformanceCounter( &qpTime );
	LocalTime = ( qpTime.QuadPart - m_qpBaseTime.QuadPart ) * 1000 / m_qpFreq.QuadPart;
	fDelta = ( LocalTime - m_PrevLocalTime ) / 1000.0f;
	m_PrevLocalTime = LocalTime;
	
	bool bPress = false;
	int nMoveFlag = 0;
	float fMoveDelta;
	if( IsPushKey( IW( IW_MOVELEFT ) ) || IsPushKey( DIK_LEFTARROW ) ) {
		nMoveFlag |= 1;
		fMoveDelta = ( IsPushKey( IW( IW_MOVELEFT ) ) ) ? GetPushKeyDelta( IW( IW_MOVELEFT ), LocalTime ) : GetPushKeyDelta( DIK_LEFTARROW, LocalTime );
		bPress = true;
	}
	if( IsPushKey( IW( IW_MOVERIGHT ) ) || IsPushKey( DIK_RIGHTARROW ) ) {
		nMoveFlag |= 2;
		fMoveDelta = ( IsPushKey( IW( IW_MOVERIGHT ) ) ) ? GetPushKeyDelta( IW( IW_MOVERIGHT ), LocalTime ) : GetPushKeyDelta( DIK_RIGHTARROW, LocalTime );
		bPress = true;
	}
	if( IsPushKey( IW( IW_MOVEFRONT ) ) || IsPushKey( DIK_UPARROW ) ) {
		nMoveFlag |= 4;
		fMoveDelta = ( IsPushKey( IW( IW_MOVEFRONT ) ) ) ? GetPushKeyDelta( IW( IW_MOVEFRONT ), LocalTime ) : GetPushKeyDelta( DIK_UPARROW, LocalTime );
		bPress = true;
	}
	if( IsPushKey( IW( IW_MOVEBACK ) ) || IsPushKey( DIK_DOWNARROW ) ) {
		nMoveFlag |= 8;
		fMoveDelta = ( IsPushKey( IW( IW_MOVEBACK ) ) ) ? GetPushKeyDelta( IW( IW_MOVEBACK ), LocalTime ) : GetPushKeyDelta( DIK_DOWNARROW, LocalTime );
		bPress = true;
	}

	if( bPress ) {
		m_fSensitivityScale *= 1.2f;
		if( m_fSensitivityScale > 1.0f ) {
			m_fSensitivityScale = 1.0f;
		}

		m_fShowSensitivity = 2.0f;
		m_nMoveFlag = nMoveFlag;
		m_fDelta = fMoveDelta;
	}
	else {
		m_fSensitivityScale *= 0.85f;
		if( m_fSensitivityScale < 0.1f ) {
			m_fSensitivityScale = 0.1f;
		}
	}

	if( IsPushKey( DIK_LCONTROL ) || IsPushKey( DIK_RCONTROL ) ) {
		m_fSlowScale *= 0.9f;
		if( m_fSlowScale < 0.25f ) {
			m_fSlowScale = 0.25f;
		}
	}
	else {
		m_fSlowScale *= 1.1f;
		if( m_fSlowScale > 1.0f ) {
			m_fSlowScale = 1.0f;
		}
	}

	if( m_fSensitivityScale > 0.1f ) {
		float fValue = 1.0f;
		if( IsPushKey( DIK_LSHIFT ) || IsPushKey( DIK_RSHIFT ) ) fValue = 1.5f;
		if( IsPushKey( DIK_LCONTROL ) || IsPushKey( DIK_RCONTROL ) ) fValue = m_fSlowScale;

		if( m_nMoveFlag & 1 ) {
			m_matExWorld.m_vPosition -= m_matExWorld.m_vXAxis * m_fDelta * ( 5000.f * m_fSensitivity * m_fSensitivityScale ) * fValue;
		}
		if( m_nMoveFlag & 2 ) {
			m_matExWorld.m_vPosition += m_matExWorld.m_vXAxis * m_fDelta * ( 5000.f * m_fSensitivity * m_fSensitivityScale) * fValue;
		}
		if( m_nMoveFlag & 4 ) {
			m_matExWorld.m_vPosition += m_matExWorld.m_vZAxis * m_fDelta * ( 5000.f * m_fSensitivity * m_fSensitivityScale) * fValue;
		}
		if( m_nMoveFlag & 8 ) {
			m_matExWorld.m_vPosition -= m_matExWorld.m_vZAxis * m_fDelta * ( 5000.f * m_fSensitivity * m_fSensitivityScale ) * fValue;
		}
	}

	if( IsPushMouseButton(1) ) {
		if( GetMouseVariation().x != 0.f ) {
			m_matExWorld.RotateYAxis( 60.0f * fDelta * GetMouseVariation().x * 0.08f );
		}
		if( GetMouseVariation().y != 0.f ) {
			m_matExWorld.RotatePitch( 60.0f * fDelta * GetMouseVariation().y * 0.08f );
		}
		CDnMouseCursor::GetInstance().SetCursorPos( m_nMouseRBDownPosX, m_nMouseRBDownPosY );
	}

	if( GetMouseVariation().z > 0.f ) {
		m_fSensitivity += 0.01f;
		if( IsPushKey( DIK_LSHIFT ) || IsPushKey( DIK_RSHIFT ) ) m_fSensitivity += 0.09f;
		m_fShowSensitivity = 2.0f;
	}
	if( GetMouseVariation().z < 0.f ) {
		m_fSensitivity -= 0.01f;
		if( IsPushKey( DIK_LSHIFT ) || IsPushKey( DIK_RSHIFT ) ) m_fSensitivity -= 0.09f;
		m_fShowSensitivity = 2.0f;
	}
	if( m_fSensitivity < 0.01f ) m_fSensitivity = 0.01f;

	m_hCamera->Update( m_matExWorld );

#ifndef _FINAL_BUILD
	// Pick and record coordinates.
	EtVector3& vOutPos = CGlobalValue::GetInstance().m_vOutPos;
	bool& bPicked = CGlobalValue::GetInstance().m_bPicked;

	DnCameraHandle hPlayerCamera = CDnCamera::FindCamera( CDnCamera::PlayerCamera );
	if( hPlayerCamera ) hPlayerCamera->ResetRadialBlurEffect();

	if ( IsPushMouseButton(2) )
	{
		POINT pt;
		GetCursorPos(&pt);
		EtVector2 Pos;
		Pos.x = (float)pt.x;
		Pos.y = (float)pt.y;
		
		CDnWorld::GetInstance().PickFromScreenCoord(Pos.x, Pos.y, vOutPos);
		vOutPos.y += 50.0f;
		
		if ( LocalTime - 500 > sTime )
		{
			
			WCHAR buff[256] = {0,};
			swprintf_s( buff, L"%0.2f %0.2f %0.2f", vOutPos.x, vOutPos.y, vOutPos.z );
			LogWnd::Log(1, L"Pick : %s", buff);

			swprintf_s( buff, L"%0.2f %0.2f %0.2f", GetMatEx()->GetPosition().x, GetMatEx()->GetPosition().y, GetMatEx()->GetPosition().z );
			LogWnd::Log(1, L"CAM : %s", buff);
			
			sTime = LocalTime;
		}
		
		bPicked = true;
	}

	if( m_fShowSensitivity > 0.0f )
	{
		m_fShowSensitivity -= fDelta;

		char szBuf[256];
		sprintf_s( szBuf, "Sensitivity - Basis : %.2f", m_fSensitivity );
		EternityEngine::DrawText2D( EtVector2( 0.20f, 0.973f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );
		//sprintf_s( szBuf, "Sensitivity - Accum : %.2f", m_fSensitivityScale );
		//EternityEngine::DrawText2D( EtVector2( 0.02f, 0.88f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );
		//sprintf_s( szBuf, "Slow - Accum : %.2f", m_fSlowScale );
		//EternityEngine::DrawText2D( EtVector2( 0.02f, 0.90f ), szBuf, D3DCOLOR_ARGB(255,255,0,0) );
	}
#endif
}

void CDnFreeCamera::OnInputReceive( int nReceiverState, LOCAL_TIME LocalTime )
{
	if( nReceiverState & IR_MOUSE_RB_DOWN )
	{
		CDnMouseCursor::GetInstance().GetCursorPos( &m_nMouseRBDownPosX, &m_nMouseRBDownPosY );
	}
}