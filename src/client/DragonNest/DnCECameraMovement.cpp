#include "stdafx.h"

#include "DnLocalPlayerActor.h"

#include "DnCamera.h"
#include "DnCECameraMovement.h"



CDnCEMovement::CDnCEMovement( DnCameraHandle hCamera, DWORD dwFrame, EtVector2 & vDir, float fSpeedBegin, float fSpeedEnd, bool bDefaultEffect ) :
CDnCameraEffectBase( hCamera, bDefaultEffect),
m_vDir(vDir.x, vDir.y),
m_fSpeedBegin(fSpeedBegin),
m_fSpeedEnd(fSpeedEnd),
m_fSpeedCrr (0.0f),
m_vPos(0.0f,0.0f),
m_fSign( 1.0f ),
m_bInvSign( true )
{
	m_fSpeedGap = m_fSpeedBegin - m_fSpeedEnd;

	m_StartTime = hCamera->GetLocalTime();
	m_HalfTime = m_StartTime + ( dwFrame / 2 );
	m_dwFrame = dwFrame;
	m_bDestroy = false;

	//CDnLocalPlayerActor::LockInput(false);
}


void CDnCEMovement::Process( LOCAL_TIME LocalTime, float fDelta )
{	
	if( m_StartTime == 0 )
		m_StartTime = LocalTime;		

	// End signal.
	if( LocalTime > (m_StartTime + m_dwFrame) )
	{
		m_bDestroy = true;
		return;
	}
	
	

	// Return to previous position - invert sign (execute only once).
	if( m_bInvSign && LocalTime > m_HalfTime )
	{
		m_bInvSign = false; 
		m_fSign = -m_fSign;
	}

	// Movement speed.
	if( m_fSpeedBegin == m_fSpeedEnd )
		m_fSpeedCrr = m_fSpeedBegin;
	else
	{
		float fRatio = (float)LocalTime / (float)( m_StartTime + m_dwFrame );

		m_fSpeedCrr = m_fSpeedBegin + (m_fSpeedGap * fRatio);		
	}

	MatrixEx * mat = m_hCamera->GetMatEx();

	//EtVector2 vec = m_vDir * (m_fSpeedCrr * fDelta);
	//m_vPos.x += vec.x;
	//m_vPos.y += vec.y;

	EtVector2 vec = m_vDir * (m_fSpeedCrr * fDelta);
	if( m_bInvSign )
	{
		m_vPos.x += vec.x;
		m_vPos.y += vec.y;
	}
	else
	{
		m_vPos.x -= vec.x;
		m_vPos.y -= vec.y;
	}
	
	mat->MoveLocalXAxis( m_vPos.x );
	mat->MoveLocalYAxis( m_vPos.y );	


}