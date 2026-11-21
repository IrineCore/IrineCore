#include "stdafx.h"

#include "DnCECameraSwing.h"

CDnCESwing::CDnCESwing( DnCameraHandle hCamera, DWORD dwFrame, float fAngle, float fSpeed, BOOL bSmooth, bool bDefaultEffect ) :
CDnCameraEffectBase( hCamera, bDefaultEffect ),
m_fCrrAngle(0.0f),
m_Sign( 1.0f )
{
	m_StartTime = hCamera->GetLocalTime();
	m_dwFrame = dwFrame;	
	m_bDestroy = false;

	m_fAngle = fAngle;
	m_fSpeed = fSpeed;
	m_bSmooth = bSmooth;

	m_fHalfAngle = m_fAngle * 0.5f;
}


void CDnCESwing::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// Exit condition.
	if( m_StartTime == 0 ) 
		m_StartTime = LocalTime;
	if( LocalTime > (m_StartTime + m_dwFrame) )
	{
		m_bDestroy = true;
		return;
	}

	// Rotation angle.
	float fAng = m_fSpeed * fDelta;

	// Invert rotation direction.
	if( fabs( m_fCrrAngle ) >= m_fAngle )
	{
		m_Sign = -m_Sign;
		//m_fCrrAngle = 0.0f;
	}

	fAng *= m_Sign;

	if( m_bSmooth == TRUE )
	{
		float f = 1.0f;
		float crrAngle = fabs( m_fCrrAngle );
		if( crrAngle < m_fHalfAngle )
		{
			f = crrAngle / m_fHalfAngle;
			f = f < 0.1f ? 0.1f : f;
		}
		else if( crrAngle > m_fHalfAngle )
		{
			f = m_fHalfAngle / crrAngle;
			f = f < 0.1f ? 0.1f : f;
		}
		fAng *= f;
		//
	}

	m_fCrrAngle += fAng;


	

	m_hCamera->GetMatEx()->RotateRoll( m_fCrrAngle );
}

