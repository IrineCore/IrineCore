#pragma once

#include "DnCamera.h"

/*
Simple walking camera used in triggers.
Rotate around prop B from prop A's position.
*/
class CDnTriggerCameraType2 : public CDnCamera {
public:
	CDnTriggerCameraType2( DnCameraHandle hCurCamera, MatrixEx &BeginCross, MatrixEx &OffsetCross, int nDelay, int nTotalAngle, bool bStartVel = true, bool bEndVel = true );
	virtual ~CDnTriggerCameraType2();

protected:
	int m_nDelay;
	int m_nTotalAngle;
	LOCAL_TIME m_StartTime;
	DnCameraHandle m_hSourceCamera;
	MatrixEx m_CrossOffset[2];
	float m_fCameraDistance;

	bool m_bBlindControl;
	bool m_bStartVel;
	bool m_bEndVel;

public:
	virtual bool Activate();
	virtual bool DeActivate();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	DnCameraHandle GetSourceCamera() { return m_hSourceCamera; }
};
