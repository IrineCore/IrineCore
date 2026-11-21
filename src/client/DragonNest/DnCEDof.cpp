#include "StdAfx.h"
#include "DnCEDof.h"
#include "EtOptionController.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCEDof::CDnCEDof( DnCameraHandle hCamera, DWORD dwFrame, float fFocusDist, float fNearStart, float fNearEnd, float fFarStart, float fFarEnd, float fNearBlur, float fFarBlur, bool bDefaultEffect )
: CDnCameraEffectBase( hCamera, bDefaultEffect )
{
	m_StartTime = hCamera->GetLocalTime();
	m_dwFrame = dwFrame;
	m_vNear = EtVector2( fNearStart, fNearEnd );
	m_vFar = EtVector2( fFarStart, fFarEnd );
	m_fFocusDist = fFocusDist;
	m_fNearBlur = fNearBlur;
	m_fFarBlur = fFarBlur;
	m_bDestroy = false;

	m_pDOFFilter = ( CEtDOFFilter * )EternityEngine::CreateFilter( SF_DOF );

	m_pDOFFilter->SetNearDOFStart( fNearStart );
	m_pDOFFilter->SetNearDOFEnd( fNearEnd );
	m_pDOFFilter->SetFarDOFStart( fFarStart );
	m_pDOFFilter->SetFarDOFEnd( fFarEnd );

	m_pDOFFilter->SetFocusDistance( fFocusDist );
	m_pDOFFilter->SetNearBlurSize( fNearBlur );
	m_pDOFFilter->SetFarBlurSize( fFarBlur );
}

CDnCEDof::~CDnCEDof()
{
	SAFE_DELETE( m_pDOFFilter );
}

void CDnCEDof::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_StartTime == 0 ) m_StartTime = LocalTime;
	if( m_dwFrame != -1 && LocalTime > m_StartTime + m_dwFrame ) {
		m_bDestroy = true;
		return;
	}
	// Nothing specific to control yet, so leave it as is.
}

bool CDnCEDof::IsDestroy()
{
	return m_bDestroy;
}


void CDnCEDof::SetNear( float fStart, float fEnd )
{
	m_vNear.x = fStart;
	m_vNear.y = fEnd;
	if( !m_pDOFFilter ) return;

	m_pDOFFilter->SetNearDOFStart( m_vNear.x );
	m_pDOFFilter->SetNearDOFEnd( m_vNear.y );
}

void CDnCEDof::SetFar( float fStart, float fEnd )
{
	m_vFar.x = fStart;
	m_vFar.y = fEnd;
	if( !m_pDOFFilter ) return;

	m_pDOFFilter->SetFarDOFStart( m_vFar.x );
	m_pDOFFilter->SetFarDOFEnd( m_vFar.y );
}

void CDnCEDof::SetFocusDistance( float fValue )
{
	m_fFocusDist = fValue;
	if( m_pDOFFilter ) m_pDOFFilter->SetFocusDistance( m_fFocusDist );
}

void CDnCEDof::SetBlurSize( float fNear, float fFar )
{
	m_fNearBlur = fNear;
	m_fFarBlur = fFar;
	if( !m_pDOFFilter ) return;

	m_pDOFFilter->SetNearBlurSize( m_fNearBlur );
	m_pDOFFilter->SetFarBlurSize( m_fFarBlur );
}
