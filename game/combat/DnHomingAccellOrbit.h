#pragma once
#include "IDnOrbitProcessor.h"
#include "SignalHeader.h"
#include "DnCalcCatmullrom.h"


// Class handling trajectory of homing accelerating projectile for OrbitType:Homing, VelocityType:Accell.
class CDnHomingAccellOrbit : public IDnOrbitProcessor
{
private:
	float m_fSpeed;
	MatrixEx m_OffsetCross;
	DnActorHandle m_hTargetActor;
	EtVector3 m_vTargetPosition;
	CDnHoming m_Homing;
	bool m_bFirstProcess;
	int* m_piValidTime;

public:
	CDnHomingAccellOrbit( const S_PROJECTILE_PROPERTY* pProjectileInfo );
	virtual ~CDnHomingAccellOrbit( void );

	void Initialize( const MatrixEx& Cross, const MatrixEx& OffsetCross );
	void SetTargetActor( DnActorHandle hActor ) { m_hTargetActor = hActor; };
	void SetTargetPosition( EtVector3 vTarget ) { m_vTargetPosition = vTarget; };		// Homing acceleration, added because some cases use TargetPosition. #20922
	void SetValidTimePointer( int* piValidTime ) { m_piValidTime = piValidTime; }
	void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta );

#if defined(PRE_FIX_HOMINGTARGET)
	static bool ms_ShowHomingTarget;
#endif // PRE_FIX_HOMINGTARGET
};