#pragma once

#include "DnCamera.h"

/*
Simple walking camera used in triggers.
Move from prop A to prop B.
*/
class CDnTriggerCameraType1 : public CDnCamera {
public:
	CDnTriggerCameraType1( DnCameraHandle hCurCamera, MatrixEx &BeginCross, MatrixEx &EndCross, int nDelay, bool bStartVel = true, bool bEndVel = true );
	virtual ~CDnTriggerCameraType1();

protected:
	int m_nDelay;
	LOCAL_TIME m_StartTime;
	DnCameraHandle m_hSourceCamera;
	MatrixEx m_CrossOffset[2];

	bool m_bBlindControl;
	bool m_bStartVel;
	bool m_bEndVel;

public:
	virtual bool Activate();
	virtual bool DeActivate();

	virtual void Process( LOCAL_TIME LocalTime, float fDelta );

	DnCameraHandle GetSourceCamera() { return m_hSourceCamera; }
};
