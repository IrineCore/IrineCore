#pragma once
#include "DnCamera.h"
#include "InputReceiver.h"


class CDnCannonCamera : public CDnCamera, public CInputReceiver
{
private:
	DnActorHandle m_hAttachActor;
	
	// Monster used as a cannon. Its position serves as the axis for horizontal camera rotation.
	// Moving mouse left/right rotates camera by that value and also rotates attached actor.
	DnActorHandle m_hAttachCannonMonster;

	float m_fPitchInputValue;
	float m_fYawInputValue;
	float m_fZDistanceAccel;
	float m_fNowZDistanceValue;

	float m_fNowPitchValue;
	float m_fNowYawValue;

	float m_fSmoothAnglePower;
	float m_fSmoothZoomPower;

	bool m_bFirstProcess;
	float m_fTargetDistance;
	float m_fCurrentDistance;

	float m_fMinZoom;
	float m_fMaxZoom;

	float m_fZDistanceBeforeSmooth;

	static float s_fLastZDistance;

	float m_fPlayerActorZDistance;			// Local Z-axis distance from the character holding the cannon.

public:
	CDnCannonCamera(void);
	virtual ~CDnCannonCamera(void);

	virtual bool Activate();
	virtual bool DeActivate();

	void AttachActor( DnActorHandle hActor );
	void DetachActor( void ) { m_hAttachActor.Identity(); };

	void AttachCannonMonster( DnActorHandle hActor );
	void DetachCannonMonster( void ) { m_hAttachCannonMonster.Identity(); };

	void RotateActorToCamZDirection( void );

	virtual void ResetCamera( void );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void PostProcess( LOCAL_TIME LocalTime, float fDelta );

	float GetDistance() { return m_fNowZDistanceValue - m_fCurrentDistance; }
	float GetYawValue() { return m_fNowYawValue; }

	//// Set the direction vector directly for now as a prototype.
	//void ViewActorDir( void );
};
