#pragma once

#include "DnCamera.h"

// #56274
// Camera Roll rotation - left to right by fAngle.
class CDnCESwing : public CDnCameraEffectBase
{

protected:

	
	// Signal Data.
	float m_fSpeed;
	float m_fAngle;
	BOOL m_bSmooth;

	//
	LOCAL_TIME m_StartTime; // Camera effect start time.
	DWORD m_dwFrame;	
	bool m_bDestroy;

	float m_fCrrAngle; // Accumulated rotation angle so far.
	float m_Sign;      // Rotation angle sign.

	float m_fHalfAngle;

public:

	CDnCESwing( DnCameraHandle hCamera, DWORD dwFrame, float fAngle, float fSpeed, BOOL bSmooth, bool bDefaultEffect=false );
	virtual ~CDnCESwing(){}

	// Override - CDnCameraEffectBase //
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual bool IsDestroy() { return m_bDestroy; }

};
