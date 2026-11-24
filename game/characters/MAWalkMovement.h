#pragma once

#include "Timer.h"
#include "MAMovementBase.h"
#include "navigationwaypoint.h"


class MatrixEx;
class CDnActorState;
class MAActorRenderBase;

#define MAX_CLIMB_HEIGHT 30.0f
#ifndef _GAMESERVER
#define FORCE_REVISION_HISTORY_COUNT 3
#define FORCE_REVISION_DELTA 0.3f
#endif

#ifdef PRE_MOD_NAVIGATION_PATH
class NavigationMesh;
#endif // PRE_MOD_NAVIGATION_PATH

class CDnWorld;
class MAWalkMovement : public MAMovementBase{
public:
	MAWalkMovement();
	virtual ~MAWalkMovement();

protected:
	float m_fLimitDgrees;

	LOCAL_TIME m_PrevLocalTime;
	EtVector2 m_vMovement;
	EtVector3 m_vMoveVectorX;
	EtVector3 m_vMoveVectorZ;
	bool m_bRefreshZVector;

	float m_fJumpMoveSpeed;
	EtVector2 m_vJumpMovement;
	EtVector3 m_vJumpXVector;
	EtVector3 m_vJumpZVector;

	// #48950 Do not use anywhere other than spider monsters. /////////////////////////////
	float m_fMoveYDistancePerSec;
	float m_fLeftMoveYDistance;
	bool m_bAppliedYDistance;
	bool m_bMaintainYDistanceOnArriveDestPosition;
	//////////////////////////////////////////////////////////////////////////

	EtVector2 m_vTargetLookVec;

	EtVector3 m_vVelocity;
	EtVector3 m_vVelocityResist;
	EtVector3 m_vVelocityXVector;
	EtVector3 m_vVelocityYVector;
	EtVector3 m_vVelocityZVector;
	EtVector3 m_vLastVelocityValue;

	EtVector3 m_vLastVelocity;

	EtVector3 m_vMovePos;
	DnActorHandle m_hMoveTarget;
	DnActorHandle m_hLookTarget;
	DnActorHandle m_hNaviTarget;
	float m_fTargetMinDistance;

	EtVector2 m_vMagnetDir;
	float m_fMagnetLength;
	float m_fAngleAssist;

	bool	m_bEnableNaviMode;
	int		m_nNaviType;
#ifdef PRE_MOD_NAVIGATION_PATH
	bool	m_bAutoMoving;
	WAYPOINT_LIST m_LOSWayPointList;	// LOS Waypoint List for debugging
#endif // PRE_MOD_NAVIGATION_PATH	
	WAYPOINT_LIST m_WayPointList;
	WayPointID	 m_WayPointId;
	float m_fNaviTargetMinDistance;
	std::string m_szNaviTargetActionName;

	bool m_bFloorForceVelocity;
	bool m_bLastFloorForceVelocity;
	bool m_bFloorCollision;

	LOCAL_TIME m_LastLookTargetTime;

	DNVector(SCollisionResponse) m_vecResult;

//	float m_fLastHeight;
	bool m_bOnDrop;
	bool m_bOnFall;
	bool m_bOnStop;
	float m_fDropVelocity;

#ifndef _GAMESERVER
	float m_fLastMoveDelta;
	float m_fLastMoveLength[FORCE_REVISION_HISTORY_COUNT];
	int m_nLastMoveCount;
#endif

	bool		m_bDebugRenderAttr;

#ifdef _GAMESERVER
	struct AniDistStruct {
		LOCAL_TIME Time;
		float fDist;
	};
	float m_fAverageAniDist;
	std::vector<AniDistStruct> m_VecAniDistList;
#endif
protected:

	virtual bool CheckDiagonalBlock( float fX, float fY );
	bool CheckMovableBlock( char cAttr );

public:

	virtual void EnableDebugDraw(bool bDebugRender ) { m_bDebugRenderAttr = bDebugRender;}
	static bool IsMovableBlock( CDnWorld* World, const EtVector3 &vPos );
	void ProcessBlock( EtVector3 vPrevPos );
	void ProcessProp( EtVector3 vPrevPos );
	void ProcessCommon( LOCAL_TIME LocalTime, float fDelta );
	void PostProcess( LOCAL_TIME LocalTime );
	virtual void Process( LOCAL_TIME LocalTime, float fDelta );
	void ProcessCollision( CDnActor *pActor, EtObjectHandle hObject, const EtVector3 &vPrevPos, bool &bFloorCollision );
	bool CheckClimb( EtObjectHandle hObject, const EtVector3 &vPrevPos, DNVector(SCollisionResponse) &vecResult, bool &bFloorCollision );
	void CalcSlide( EtVector3 &SlideDir, float &fWeight, EtVector3 &CollisionNormal, EtVector3 &MoveDir );
	virtual void PushAndCollisionCheck( EtVector3 &vPush );

	virtual void SetMoveVectorX( EtVector3 &vVec );
	virtual void SetMoveVectorZ( const EtVector3 &vVec );
	virtual EtVector3 *GetMoveVectorX() { return &m_vMoveVectorX; }
	virtual EtVector3 *GetMoveVectorZ() { return &m_vMoveVectorZ; }

	virtual void MoveZ( float fSpeed );
	virtual void MoveX( float fSpeed );
	virtual void MovePos( EtVector3 &vPos, bool bRefreshZVec = true );
	virtual void MoveTarget( DnActorHandle &hActor, float fMinDistance );
	virtual void MoveTargetNavi( DnActorHandle &hActor, float fMinDistance, const char* szActionName );
	virtual void MoveTargetNavi( EtVector3& hTargetPos, float fMinDistance, const char* szActionName );
	virtual int  GetNaviType(){ return m_nNaviType; }
#ifdef PRE_MOD_NAVIGATION_PATH
	virtual void AutoMoving( EtVector3& vTargetPos, float fMinDistance, const char* szActionName, bool bGeneratePath );
	void GenerateAutoMovingPath( NavigationMesh* pNaviMesh, NavigationCell* pStartCell, NavigationCell* pEndCell, EtVector3& vStartPos, EtVector3& vEndPos );
#endif // PRE_MOD_NAVIGATION_PATH
	virtual void MoveToWorld( const EtVector2 &vVec );
	virtual EtVector3 *GetMovePos() { return &m_vMovePos; }
	virtual void ResetMove();
	virtual DnActorHandle GetMoveTarget();

	virtual void Look( const EtVector2 &vVec, bool bForceRotate = true );
	virtual EtVector3 *GetLookDir();
	virtual EtVector2 *GetLookDirTarget() { return &m_vTargetLookVec; }
	virtual void LookTarget( const DnActorHandle &hActor );
	virtual DnActorHandle GetLookTarget();
	virtual void ResetLook();

	virtual void SetVelocity( const EtVector3 &vVec );
	virtual void SetVelocityX( float fPower );
	virtual void SetVelocityY( float fPower );
	virtual void SetVelocityZ( float fPower );

	virtual void SetResistance( const EtVector3 &vVec );
	virtual void SetResistanceX( float fPower );
	virtual void SetResistanceY( float fPower );
	virtual void SetResistanceZ( float fPower );

	virtual EtVector3 *GetLastVelocity();
	virtual EtVector3 *GetVelocity();
	virtual EtVector3 *GetResistance();
	virtual EtVector3 *GetVelocityValue();
#ifdef PRE_ADD_GRAVITY_PROPERTY
	virtual void SetAppliedYDistance( bool bAppliedYDistance ) { m_bAppliedYDistance = bAppliedYDistance; }
#endif // PRE_ADD_GRAVITY_PROPERTY

	virtual void SetMagnetDir( const EtVector2 &vPos );
	virtual void SetMagnetLength( float fLength );

	float GetAngleAssistValue() { return m_fAngleAssist; }

	void SetLimitDgrees( float fDgrees );
	float GetLimitDgrees() { return m_fLimitDgrees; }
	virtual void Jump( float fPower, EtVector2 &vVec );
	virtual void MoveJumpX( float fSpeed );
	virtual void MoveJumpZ( float fSpeed );
	virtual void SetVelocityByMoveVector( EtVector3 &vVec );
	virtual void SetMagnetPosition( EtVector3 &vPos );

	virtual void SetMoveYDistancePerSec( float fMoveYDistancePerSec, float fWholeMoveYDistance, bool bMaintainYDistance );
	virtual void ResetMoveYDistance( void );

	WAYPOINT_LIST& GetWayPoints() { return m_WayPointList; }
#ifdef PRE_MOD_NAVIGATION_PATH
	WAYPOINT_LIST& GetLOSWayPoints() { return m_LOSWayPointList; }
#endif // PRE_MOD_NAVIGATION_PATH
	virtual bool IsNaviMode() { return m_bEnableNaviMode; }
	virtual void ResetNaviMode() { m_bEnableNaviMode = false; }
	virtual bool IsFloorCollision() { return m_bFloorCollision; }
	virtual size_t	GetWayPointSize(){ return m_WayPointList.size(); }
//	float GetLastHeight() { return m_fLastHeight; }

#ifndef _GAMESERVER
	void DebugRenderAttr();
#endif	
	virtual DnActorHandle	GetNaviTarget(){ return m_hNaviTarget; }
	virtual float			GetNaviTargetMinDistance(){ return m_fNaviTargetMinDistance; }
	virtual const char*		GetNaviTargetActionName(){ return m_szNaviTargetActionName.c_str(); }
	virtual EtVector2*		GetJumpMovement() { return &m_vJumpMovement; }
	virtual void			SetJumpMovement( const EtVector2& vVec ){ m_vJumpMovement= vVec; }
	virtual bool			GetFloorForceVelocity(){ return m_bFloorForceVelocity; }
	virtual bool			GetLastFloorForceVelocity(){ return m_bLastFloorForceVelocity; }

#ifdef _GAMESERVER
	float GetAverageAniDist() { return m_fAverageAniDist; }
#endif

};


class MAFlyMovement : public MAWalkMovement 
{
public:
	MAFlyMovement()	{ }
	virtual ~MAFlyMovement() {}
};

class IBoostPoolMAWalkMovement : public MAWalkMovement, public TBoostMemoryPool<IBoostPoolMAWalkMovement>
{
public :
	IBoostPoolMAWalkMovement()
		:MAWalkMovement(){}
	virtual ~IBoostPoolMAWalkMovement(){}
};
