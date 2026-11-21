#pragma once
#include "SignalHeader.h"
#include "EtMatrixEx.h"


struct S_PROJECTILE_PROPERTY;		// Defined in CDnProjectile::S_ORBIT_PROPERTY.


// Class handling homing projectile for OrbitType:Homing, VelocityType:Constant
class IDnOrbitProcessor
{
protected:
#ifdef _GAMESERVER
	CMultiRoom* m_pRoom;
#endif


public:
#ifdef _GAMESERVER
	IDnOrbitProcessor( void ) : m_pRoom( NULL ) 
	{
		m_nTargetPartsBoneIndex = m_nTargetPartsIndex = -1;
	};
#else
	IDnOrbitProcessor( void ) 
	{
		m_nTargetPartsBoneIndex = m_nTargetPartsIndex = -1;
	};
#endif
	virtual ~IDnOrbitProcessor( void ) {};
	
	// Common interface for values needed for trajectory calculation
	virtual void Initialize( const MatrixEx& Cross, const MatrixEx& OffsetCross ) {};
	virtual void SetTargetActor( DnActorHandle hActor ) {};
	virtual void SetTargetPosition( EtVector3 vTarget ) {};
	virtual void SetValidTimePointer( int* piValidTime ) {};

	// Set up room object for game server
#ifdef _GAMESERVER
	void SetRoom( CMultiRoom* pRoom ) { m_pRoom = pRoom; };
#endif

	virtual void ProcessOrbit( /*IN OUT*/ MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta ) = 0;

	// By default, processes projectile orbit set to fallgravity in CDnProjectile::ProcessDestroyOrbit(). Used only on client.
	virtual void ProcessFallGravityOrbit( MatrixEx& Cross, EtVector3& vPrevPos, LOCAL_TIME LocalTime, float fDelta, float& fGravity, float fFallGravityAccell );

	// Factory that creates trajectory handler objects
	static IDnOrbitProcessor* Create( const MatrixEx& Cross, const MatrixEx& OffsetCross, const S_PROJECTILE_PROPERTY* pOrbitProperty );

protected:
	int m_nTargetPartsIndex;
	int m_nTargetPartsBoneIndex;	//Index holding actual Bone info in selected Parts.. (MonsterParts has a vector, and _Info has _PartsInfo as a vector)
public:
	//Set the selected parts index if it is a parts monster
	virtual void SetTargetPartsIndex(int nPartsIndex, int nBoneIndex) { m_nTargetPartsIndex = nPartsIndex; m_nTargetPartsBoneIndex = nBoneIndex; };

	EtVector3 FindTargetPosition(DnActorHandle hActor, int nPartsIndex, int nBoneIndex);
};