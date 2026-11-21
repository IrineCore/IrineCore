#pragma once

#include "DnMonsterActor.h"
#include "MAStaticMovement.h"
#include "MASingleBody.h"
#include "MASingleDamage.h"


// Monster actor for cannon used starting from Sea Dragon Nest.
// New cannon monster added as monster ID 235792.
// AI is not used.

class CDnCannonMonsterActor : public TDnMonsterActor< MASingleBody, MASingleDamage >
{
private:
	// Handle of the actor owning this cannon monster.
	DnActorHandle m_hMasterPlayerActor;
	EtVector3 m_vCannonLookDirection;

	// Z-axis vector of cannon camera from client. Arrives via CannonTargeting message, so use this when firing actual projectile.
	EtVector3 m_vCannonTargetingDir;
	EtVector3 m_vShootDir;
	EtVector3 m_vCannonGroundHitPos;

public:
	CDnCannonMonsterActor( CMultiRoom *pRoom, int nClassID );
	virtual ~CDnCannonMonsterActor();

	virtual MAMovementBase* CreateMovement();

	virtual void OnDie( DnActorHandle hHitter );
	virtual void OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex );
	virtual void Look( EtVector2& vVec, bool bForceRotate=true);

	virtual void OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor, HitStruct *pStruct );
	virtual void OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct );

	void SetMasterPlayerActor( DnActorHandle hMasterPlayerActor );
	void ClearMasterPlayerActor( void );
	bool IsPossessed( void ) { return m_hMasterPlayerActor ? true : false; };
	void SetCannonProjectileInfo( const EtVector3& vCannonDir, const EtVector3& vShootDir, const EtVector3& vCannonGroundHitPos )
	{ m_vCannonTargetingDir = vCannonDir; m_vShootDir = vShootDir; m_vCannonGroundHitPos = vCannonGroundHitPos; };
	void OnMasterPlayerActorDie( void );
	DnActorHandle GetMasterPlayerActor( void ) { return m_hMasterPlayerActor; };
	
	void SetCannonLookDirection(EtVector3 *Direction) { m_vCannonLookDirection = *Direction; }
	EtVector3 GetCannonLookDricetion(){return m_vCannonLookDirection;}
	bool IsCannonMonsterActor(){ return true; }
};
