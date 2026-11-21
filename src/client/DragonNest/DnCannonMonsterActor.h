#pragma once
#include "DnMonsterActor.h"
#include "MAStaticMovement.h"
#include "MASingleBody.h"
#include "MASingleDamage.h"
#include "DnSkill.h"


// Monster actor for cannon used starting from Sea Dragon Nest.
// New cannon monster added as monster ID 235792.
// AI is not used.
class CDnCannonMonsterActor : public TDnMonsterActor< MASingleBody, MASingleDamage >
{
private:
	// Handle of the actor owning this cannon monster.
	DnActorHandle m_hMasterPlayerActor;
	EtVector3 m_vNowShootDir;				// Projectile direction in cannon local coordinates. Same meaning as the direction set in ActionTool.
	map<int, const ProjectileStruct*> m_mapCannonProjectileSignalBySkillID;
	EtVector3 m_vCannonLookDirection;
	bool m_bHaveMaster; // Rotha: Is there currently a master on the cannon?

	// EtMatrixEx containing the rotation value of the cannon barrel's facing direction
	MatrixEx m_matExCannonRotated;
	MatrixEx m_matExOriginal;

	// Actual skills are only held on the server.
	// On the client, only the data needed to display cooldown is used.
	// UseSkill and the skill system do not actually run on the client.
	// Currently there are two firing strong/weak cannons.	

public:
	CDnCannonMonsterActor( int nClassID, bool bProcess = true );
	virtual ~CDnCannonMonsterActor();

	virtual MAMovementBase* CreateMovement()
	{
		MAMovementBase* pMovement = new MAStaticMovement();
		return pMovement;
	}

	virtual bool Initialize( void );
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void OnDie( DnActorHandle hHitter );
	virtual void SetDestroy();
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	virtual void ProcessCannonLook(bool bLocal);
	virtual void Look( EtVector2& vVec, bool bForceRotate=true);
	
	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor );
	// from CDnMonsterActor
	virtual void OnSkillUse( int iSkillID, int iLevel );
	void SetMasterPlayerActor( DnActorHandle hMasterPlayerActor );
	void ClearMasterPlayerActor( void );
	bool GetCannonGroundHitPosition( /*IN*/ DnCameraHandle hCamera,  /*OUT*/ EtVector3& vCannonGroundHitPos );
	void OnMasterPlayerActorDie( void );
	DnActorHandle GetMasterPlayerActor( void ) { return m_hMasterPlayerActor; };

	const EtVector3& GetNowShootDir( void ) { return m_vNowShootDir; };

	void SetCannonLookDirection(EtVector3 *Direction) { m_vCannonLookDirection = *Direction; }
	EtVector3 GetCannonLookDricetion(){return m_vCannonLookDirection;}
	bool IsHaveMaster(){return m_bHaveMaster;}

	virtual void OnCreateProjectileBegin( /*IN OUT*/ MatrixEx& LocalCross );
	virtual void OnCreateProjectileEnd( void ); 
	virtual void OnComboCount( int nValue, int nDelay );
	
};
