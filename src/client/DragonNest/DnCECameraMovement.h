#pragma once


// Move Camera.
class CDnCEMovement : public CDnCameraEffectBase
{

protected:

	// Signal Data.
	EtVector2 m_vDir; // Movement direction.
	float m_fSpeedBegin;
	float m_fSpeedEnd;


	//
	LOCAL_TIME m_StartTime; // Camera effect start time.
	LOCAL_TIME m_HalfTime;  // Midpoint time.	
	DWORD m_dwFrame;	
	bool m_bDestroy;

	bool m_bInvSign;		
	float m_fSign;			// Movement sign.

	
	//
	float m_fSpeedGap; // m_fSpeedBegin - m_fSpeedEnd.
	float m_fSpeedCrr; // Current speed.
	EtVector2 m_vPos;
	

public:

	CDnCEMovement( DnCameraHandle hCamera, DWORD dwFrame, EtVector2 & vDir, float fSpeedBegin, float fSpeedEnd, bool bDefaultEffect=false );
	virtual ~CDnCEMovement(){}

	// Override - CDnCameraEffectBase //
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual bool IsDestroy(){ return m_bDestroy; }

};
