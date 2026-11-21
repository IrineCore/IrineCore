#include "stdafx.h"
#include "DnProjectile.h"
#include "DnWeapon.h"
#include "MAActorRenderBase.h"
#include "VelocityFunc.h"
#include "EtActionSignal.h"
#include "DnWorld.h"
#include "DnMonsterActor.h"
#include "MAAiBase.h"
#include "DnWorldBrokenProp.h"
#include "DNGameServerManager.h"
#include "DnChainAttackBlow.h"
#include "DnPingpongBlow.h"
#include "MAWalkMovement.h"
#include "DnPlayerActor.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "IDnOrbitProcessor.h"
#include "DNMissionSystem.h"
#include "DnPartsMonsterActor.h"
#include "DnComboDamageLimitBlow.h"
#include "DnStateBlow.h"
#include "DnCurseBlow.h"
#include "DnChangeStandActionBlow.h"
#ifdef IRINE_MOD_PLAYER_BOTS
#include "DnAIBotController.h"
#endif


int CDnProjectile::s_iHitUniqueID = 0;

#define SHOOT_ACTION_NAME "Shoot"
#define HIT_ACTION_NAME "Hit"
#define DESTROY_ACTION_NAME "Destroy"		// Do not execute this action on the server.

// Game server and client have separate DnProjectile class files, so update both when modifying.
const int PROJECTILE_PACKET_BUFFER_SIZE = 128;

CDnProjectile::CDnProjectile( CMultiRoom *pRoom, DnActorHandle hActor, bool bProcess, bool bIncreaseUniqueID )
: CDnWeapon( pRoom, bProcess, bIncreaseUniqueID )
, m_vPrevPos( 0.f, 0.f, 0.f )
, m_vStartPos( 0.f, 0.f, 0.f )
, m_vTargetPosition( 0.f, 0.f, 0.f )
, m_vHitPos( 0.f, 0.f, 0.f )
, m_bOnCollisionCalled( false )
, m_bPierce( false )
, m_LastHitSignalEndTime( 0 )
, m_iHitUniqueID( -1 )
, m_bHasHitAction( false )
, m_bHasHitSignalInHitAction( false )
, m_bHasHitSignalInShootAction( false )
, m_nLastHitSignalIndex( 0 )
, m_fElapsedTime( 0.0f )
, m_iShooterShootActionIndex( -1 )
, m_iSignalArrayIndex( -1 )
, m_fHitApplyPercent( -1.0f )
, m_pOrbitProcessor( NULL )
, m_pPacketBuffer( NULL )
, m_pPacketStream( NULL )
, m_vForceDir( 0.0f, 0.0f, 0.0f )
, m_bUseForceDir( false )
{
	SetWeaponType( WeaponTypeEnum::Projectile );
	m_hShooter = hActor;

	m_InvalidLocalTime = 0;
	m_CreateLocalTime = 0;

	m_bValidDamage = true;
	m_bFallGravity = false;
	m_fSpeed = 3000.f;
	m_fResistance = 0.f;
	m_nDestroyOrbitTimeGap = 3000;

	m_bStick = false;
	m_fGravity = 0.f;

	m_bFirstProcess = true;

	m_DestroyOrbitType = FallGravity;
	m_ValidType = ValidTypeEnum( WeaponLength | Stick );
	memset( &m_HitStruct, 0, sizeof(m_HitStruct) );

	m_iHitUniqueID = ++s_iHitUniqueID;

	m_bHitSignalProcessed = false;
	m_bBombHitSignalStarted = false; // 2009.7.29 hanki - Since the hit signal can follow after an explosion, turn on the flag at the start of the hit signal.
	

	m_VelocityType = Accell;
	m_ParentSkillInfo.hSkillUser = hActor;
	m_nMaxHitCount = 0;

	m_bHitActionStarted = false;

	m_bProcessingBombHitSignal = false;
	
	m_bFromCharger = false;

	m_fProjectileOrbitRotateZDegree = 0.0f;

	m_iShootActionIndex = -1;
	m_iHitActionIndex = -1;
	m_iDestroyActionIndex = -1;

	m_bHasDestroyAction = false;
	
	// Process damage calculation for fired projectiles based on the state captured at the time of firing.
	m_HitParam.bFromProjectile = true;

	m_bTraceHitTarget = false;
	m_bTraceHitActorHittable = false;

	m_cShooterType = 0;
	m_dwShooterUniqueID = -1;
	m_nShooterSerialID = -1;
	m_nShooterActionIndex = -1;
	m_nShooterSignalIndex = -1;

	m_eForceHitElement = ElementEnum::ElementEnum_Amount;

	m_iSummonMonsterForceSkillLevel = 0;
	m_OrbitType = OrbitTypeCount;
	m_TargetType = TargetTypeCount;
	m_nTargetPartsBoneIndex = 0;
	m_nTargetPartsIndex = 0;
	m_nValidTime = 0;

#if defined(PRE_FIX_52329)
	m_nIgnoreHitType = 0;
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	m_bHitActionVectorInit = false;
#endif // PRE_ADD_55295

	m_SkillStartTime = 0;	

#if defined(PRE_FIX_65287)
	m_fShooterFinalDamageRate = 0.0f;
#endif // PRE_FIX_65287

}

CDnProjectile::~CDnProjectile()
{
	if (m_ParentSkillInfo.hSkillUser)
	{
		DnSkillHandle hSkill = m_ParentSkillInfo.hSkillUser->FindSkill(m_ParentSkillInfo.iSkillID);
		if (hSkill)
			hSkill->RemoveProjectile(this);
	}

	SAFE_DELETE( m_pOrbitProcessor );
	SAFE_DELETE( m_pPacketBuffer );
	SAFE_DELETE( m_pPacketStream );

	FreeAction();
}

bool CDnProjectile::Initialize( MatrixEx &Offset, OrbitTypeEnum OrbitType, DestroyOrbitTypeEnum DestroyType, TargetTypeEnum TargetType )
{
	m_OrbitType = OrbitType;
	m_DestroyOrbitType = DestroyType;
	m_TargetType = TargetType;

	m_Cross = Offset;
	m_OffsetCross = Offset;
	m_vStartPos = m_vPrevPos = m_Cross.m_vPosition;
	SetActionQueue( SHOOT_ACTION_NAME, 0, 0.f );

	m_iShootActionIndex = GetElementIndex( SHOOT_ACTION_NAME );

	m_nValidTime = 0;

	m_bFirstProcess = true;
	if( m_OrbitType == Homing ||
		m_OrbitType == TerrainHoming ) {
		m_ValidType = (ValidTypeEnum)( m_ValidType | Time );
		m_ValidType = (ValidTypeEnum)( m_ValidType & ~WeaponLength );
	}

	if( IsExistAction( SHOOT_ACTION_NAME ) )
	{
		// Check if there is a hit signal.
		// For homing acceleration, damage is applied when time expires, so if there is no Hit signal in the Shoot action
		// Mark in advance here to distinguish so damage processing is skipped.
		CEtActionBase::ActionElementStruct* pShootActionElement = this->GetElement( SHOOT_ACTION_NAME );
		int iNumSignals = static_cast<int>(pShootActionElement->pVecSignalList.size());
		for( int iSignal = 0; iSignal < iNumSignals; ++iSignal )
		{
			SignalTypeEnum eSignalType = (SignalTypeEnum)pShootActionElement->pVecSignalList.at( iSignal )->GetSignalIndex();
			if( STE_Hit == eSignalType )
			{
				m_bHasHitSignalInShootAction = true;
				break;
			}
		}
	}

	if( IsExistAction( HIT_ACTION_NAME ) )
	{
		m_bHasHitAction = true;

		// A hit action is considered valid only when a hit signal exists.
		CEtActionBase::ActionElementStruct* pHitActionElement = this->GetElement( HIT_ACTION_NAME );
		m_iHitActionIndex = GetElementIndex( HIT_ACTION_NAME );
		int iNumSignals = static_cast<int>(pHitActionElement->pVecSignalList.size());
		for( int iSignal = 0; iSignal < iNumSignals; ++iSignal )
		{
			SignalTypeEnum eSignalType = (SignalTypeEnum)pHitActionElement->pVecSignalList.at( iSignal )->GetSignalIndex();
			if( STE_Hit == eSignalType ||
				STE_Projectile == eSignalType )		// Distortion Arrow etc. fire projectiles again in the projectile's hit action..
			{
				m_bHasHitSignalInHitAction = true;
				break;
			}
		}
	}

	if( IsExistAction( DESTROY_ACTION_NAME ) )
	{
		m_iDestroyActionIndex = GetElementIndex( DESTROY_ACTION_NAME );
		m_bHasDestroyAction = true;
	}

	return true;
}

bool CDnProjectile::PostInitialize( void )
{
	bool bResult = false;

	_ASSERT( NULL == m_pOrbitProcessor );

	S_PROJECTILE_PROPERTY OrbitProperty;
	OrbitProperty.eOrbitType = m_OrbitType;
	OrbitProperty.eTargetType = m_TargetType;
	OrbitProperty.eVelocityType = m_VelocityType;
	OrbitProperty.fSpeed = m_fSpeed;
	OrbitProperty.fResistance = m_fResistance;
	OrbitProperty.fProjectileOrbitRotateZ = m_fProjectileOrbitRotateZDegree;
	m_pOrbitProcessor = IDnOrbitProcessor::Create( m_Cross, m_OffsetCross, &OrbitProperty );		// Initial position equals the offset, but stored separately for clarity.

	_ASSERT( m_pOrbitProcessor );
	if( NULL != m_pOrbitProcessor )
	{
		m_pOrbitProcessor->SetRoom( GetRoom() );
		m_pOrbitProcessor->SetTargetActor( m_hTargetActor );
		m_pOrbitProcessor->SetTargetPosition( m_vTargetPosition );
		m_pOrbitProcessor->SetValidTimePointer( &m_nValidTime );
		m_pOrbitProcessor->SetTargetPartsIndex(m_nTargetPartsIndex, m_nTargetPartsBoneIndex);

		bResult = true;
	}

	return bResult;
}


void CDnProjectile::ProcessOrbit( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bTraceHitTarget )
	{
		if( m_hTraceActor )
		{
			EtVector3 vPos = *m_hTraceActor->GetPosition();
			m_Cross.SetPosition( vPos );
			return;
		}
	}

	if( m_bStick )
		return;

	// Partially created projectiles may not have had PostInitialize called.
	// A typical example is Aerial Chain Shot firing without a quiver sub-weapon.
	if( m_pOrbitProcessor )
		m_pOrbitProcessor->ProcessOrbit( m_Cross, m_vPrevPos, LocalTime, fDelta );
}

void CDnProjectile::ProcessValid( LOCAL_TIME LocalTime, float fDelta )
{
	// Once marked invalid, do not check again.
	// For projectiles that explode on hit, the m_bValidDamage flag is turned on at explosion and then...
	// There are cases where ValidDamage becomes false again inside ProcessValid because the range has expired.
	if( 0 == m_InvalidLocalTime )
	{
		if( m_bValidDamage && ( m_ValidType & ValidTypeEnum::WeaponLength ) )
		{
			if( GetWeaponLength() != -1 )
			{
				bool bRangeOut = false;
				if( RangeFallGravity == m_DestroyOrbitType )
				{
					float fAdditionalRange = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::RangeFallGravityAdditionalProjectileRange );
					float fRangeFallGravityLength = float(GetWeaponLength()) * fAdditionalRange;
					bRangeOut = (fRangeFallGravityLength < EtVec3Length( &( m_Cross.m_vPosition - m_vStartPos ) ));
				}
				else
				{
					bRangeOut = ( EtVec3Length( &( m_Cross.m_vPosition - m_vStartPos ) ) > GetWeaponLength() );
				}

				if( bRangeOut )
				{
					m_bValidDamage = false;
					m_InvalidLocalTime = LocalTime;

					// #43085 Point where client runs Destroy action.. server removes the projectile immediately.
					if( m_bHasDestroyAction )
					{
						// For FallGravity, high-angle shots that drop like Punishing Swing or Fireball must hit, so only process RangeFallGravity. 
						if( RangeFallGravity == m_DestroyOrbitType )
							SetDestroy(); 
					}
				}
			}
		}
	}

	// Once invalidated, prevent it from being invalidated again.
	// When a hit action exists, validdamage is turned back on, so distinguish it using m_bOnCollisionCalled.
	if( m_bValidDamage && ( m_ValidType & ValidTypeEnum::Time ) && false == m_bOnCollisionCalled ) 
	{
		if( (int)( LocalTime - m_CreateLocalTime ) > m_nValidTime ) 
		{
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;

			if( m_hTargetActor && m_hShooter &&
				OrbitTypeEnum::Homing == m_OrbitType &&
				VelocityTypeEnum::Accell == m_VelocityType )
			{
				if( m_HitParam.szActionName.empty() )
					m_HitParam.szActionName.assign( "Hit_Front" );

				// If the hit action has a hit signal, set the valid damage go on flag to true.
				// Even with homing acceleration, sometimes hit processing happens first in ProcessDamage and sometimes here.
				// If collision is processed by projectile position, it is handled in ProcessDamage; otherwise it is handled here.
				// For homing acceleration projectiles, if there is no Hit signal in the Shoot action, Hit processing naturally won't occur,
				// Call only OnCollisionWithActor() since state effects added inside CheckAndApplyDamage() must not be applied.
				// Enters here on first hit, so only during Shoot action.
				if( m_bHasHitSignalInShootAction )
				{
					if( m_bTraceHitTarget || m_hTargetActor->IsHittable( m_hShooter, LocalTime, &m_HitStruct ) )
					{
#if defined(PRE_FIX_59238)
						//Puppet and its summoner owner actor must not be hit simultaneously.
						if (IsHittable(m_hTargetActor))
						{
							CheckAndApplyDamage( m_hTargetActor, m_bHasHitSignalInHitAction );
						}
#else
						CheckAndApplyDamage( m_hTargetActor, m_bHasHitSignalInHitAction );
#endif // PRE_FIX_59238
					}
				}
				else
				{
					OnCollisionWithActor();
				}
			}
			else
			{
				// #38437 Client executes Destroy action, so server only checks existence and if present 
				// Remove immediately instead of executing Destroy action.
				if( m_bHasDestroyAction )
					SetDestroy(); 
			}
		}
	}

#if defined(PRE_FIX_52329)
	//Skip Valid check if IgnoreHitType is non-zero..
	if (m_nIgnoreHitType == 0)
	{
#endif // PRE_FIX_52329

	// If terrain linear, make projectile disappear when current position is in an impassable area.
	// For Rolling Lava, even when terraing linear and hitting an impassable area, hit's hit signal
	// Changed to process damage for projectiles with a hit action so it is handled properly.
	if( OrbitTypeEnum::TerrainLinear == m_OrbitType ||
		OrbitTypeEnum::TerrainHoming == m_OrbitType)
	{
		if( false == MAWalkMovement::IsMovableBlock( &INSTANCE(CDnWorld), m_Cross.GetPosition() ) )
		{
			if( m_bValidDamage ) 
			{
				m_bValidDamage = false;
				m_InvalidLocalTime = LocalTime;
			}

			m_bStick = true;

			OnCollisionWithGround();
		}
	}

	if( !m_bStick && ( m_ValidType & ValidTypeEnum::Stick ) )
	{
#if defined(PRE_FIX_55855)
		float fHeight = CDnWorld::GetInstance( GetRoom() ).GetMaxHeightWithProp( m_Cross.m_vPosition );
#else
		float fHeight = CDnWorld::GetInstance(GetRoom()).GetHeight( m_Cross.m_vPosition );
		
#endif // PRE_FIX_55855
		if( m_Cross.m_vPosition.y < fHeight )
		{
			// Calculate direction based on the projectile's absolute position.
			// ZVector is controlled by OrbitProcessor and may differ from the actual direction.
			EtVector3 vDir = m_Cross.m_vPosition - m_vPrevPos;
			EtVec3Normalize( &vDir, &vDir );

			EtVector3 vPickPos;
			if( CDnWorld::GetInstance(GetRoom()).Pick( m_vPrevPos, vDir, vPickPos ) == true ) 
			{
				m_Cross.m_vPosition = vPickPos;
#if defined(PRE_FIX_55855)
				m_Cross.m_vPosition.y = CDnWorld::GetInstance( GetRoom() ).GetMaxHeightWithProp( vPickPos );
#endif // PRE_FIX_55855
			}
			else m_Cross.m_vPosition.y = fHeight;

			//m_Cross.m_vPosition += m_Cross.m_vZAxis * -10.f;
			if( m_bValidDamage ) {
				m_bValidDamage = false;
				m_InvalidLocalTime = LocalTime;
			}
			m_bStick = true;

			OnCollisionWithGround();
		}
	}

	// For acceleration homing, check if it collided with a prop.
	if( m_hTargetActor && 
		OrbitTypeEnum::Homing == m_OrbitType &&
		VelocityTypeEnum::Accell == m_VelocityType )
	{
		float fLength = EtVec3Length( &( m_Cross.m_vPosition - m_vPrevPos ) );
		EtVector3 vCenter = m_vPrevPos + ( m_Cross.m_vZAxis * ( fLength * 0.5f ) );
		float fPropContactDistance = FLT_MAX;
		float fPropContactTime = FLT_MAX;
		DnPropHandle hResultProp = _CheckPropCollision( vCenter, fLength, fPropContactTime, fPropContactDistance );

		if( hResultProp )
		{
			OnCollisionWithProp();
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;
			m_bStick = true;
		}
	}

#if defined(PRE_FIX_52329)
	}
#endif // PRE_FIX_52329

	// If already executing hit action, do not re-execute the action. (#21384)
	// If OnCollision~ was not called separately but action changed to hit, in shoot action 
	// Since it was changed directly, call OnCollisionWithGround().
	if( false == m_bOnCollisionCalled && m_iHitActionIndex == GetCurrentActionIndex() && m_iHitActionIndex != -1 )
	{
		if( m_bValidDamage ) 
		{
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;
		}
		m_bStick = true;
		OnChangedNextActionToHit();
	}
}


void CDnProjectile::ProcessDestroyOrbit( LOCAL_TIME LocalTime, float fDelta )
{
	if( LocalTime - m_InvalidLocalTime > m_nDestroyOrbitTimeGap ) {
		SetDestroy();
	}
	switch( m_DestroyOrbitType ) {
		case Instantly:
			if( false == m_bHasHitAction )		// Projectiles with a hit action are automatically destroyed when the hit action ends or valid time expires.
				SetDestroy();
			break;
		case FallGravity:
		case RangeFallGravity:
			{
				if( !m_bStick ) 
				{
					m_Cross.m_vPosition.y += CalcMovement( m_fGravity, fDelta, FLT_MAX, FLT_MIN, -15.f );

					// Since there are cases like summon comet where it falls with fall gravity once max range is reached 
					// Allow taking damage even while falling with fall gravity.
					m_bValidDamage = true;
					m_bFallGravity = true;
				}

			}
			break;
	}
}


DnPropHandle CDnProjectile::_CheckPropCollision( EtVector3& vCenter, float fLength, float& fPropContactDistance, float& fPropContactTime )
{
	DnPropHandle hResult;

	DNVector(DnPropHandle) hVecProp;
	int nCount = CDnWorld::GetInstance(GetRoom()).ScanProp( vCenter, fLength * 0.5f, hVecProp, FilterProjectileHitSignal() );

	if( nCount > 0 ) 
	{
		float fMinContactTime = FLT_MAX;
		float fNowContactTime = 0.f;
		SCollisionResponse Response;
		SSegment Segment;
		Segment.vOrigin = m_vPrevPos;
		Segment.vDirection = m_Cross.m_vZAxis * fLength;

		for( int i=0; i<nCount; i++ ) 
		{
			if( !hVecProp[i] )
				continue;

			if( hVecProp[i]->IsProjectileSkip() )
				continue;

			if( hVecProp[i] == m_hShooterProp )
				continue;

			if( hVecProp[i]->GetObjectHandle() && hVecProp[i]->GetObjectHandle()->FindSegmentCollision( Segment, Response ) ) 
			{
				// Since server frames may lag, do not compare against the projectile's current position.
				// Filter based on the arrival time provided by the engine.
				//fNowContactTime = EtVec3LengthSq( &( hVecProp[i]->GetMatEx()->m_vPosition - m_Cross.m_vPosition ) );
				fNowContactTime = Response.fContactTime;

				if( fNowContactTime < fMinContactTime ) 
				{
					fMinContactTime = fNowContactTime;
					hResult = hVecProp[i];

					//						EtVector3 vVec = m_Cross.m_vPosition - m_vPrevPos;
					//						EtVec3Normalize( &vVec, &vVec );
					m_vHitPos = m_vPrevPos + ( Segment.vDirection * Response.fContactTime );
					fPropContactDistance = EtVec3LengthSq( &( Segment.vDirection * Response.fContactTime ) );
					fPropContactTime = Response.fContactTime;
				}
			}
		}

	}	
	
	return hResult;
}

bool CDnProjectile::ProcessDamage( LOCAL_TIME LocalTime, float fDelta, LOCAL_TIME SignalStartTime, 
								   LOCAL_TIME SignalEndTime, int nSignalIndex, BOOL bUseHitSignalArea )
{
	bool bHitResult = false;
	// Do not deal damage again to a target that has already been hit.

	// #40133 Kill count to report to mission.
	int iKillCount = 0;

	if( !m_hShooter ) return false;
	if( (m_hShooter->GetActorType() != CDnActorState::PropActor) && m_hShooter->IsDie() ) return false;

	float fLength = EtVec3Length( &( m_Cross.m_vPosition - m_vPrevPos ) );
	//if( /*fLength > 0.f ||*/ m_bOnCollisionCalled ) 
	{
		EtVector3 vCenter = m_vPrevPos + ( m_Cross.m_vZAxis * ( fLength * 0.5f ) );

		// Prop Check
		float fPropContactDistance = FLT_MAX;
		float fPropContactTime = FLT_MAX;
		DnPropHandle hResultProp = _CheckPropCollision( vCenter, fLength, fPropContactDistance, fPropContactTime );

		// Actor check
		float fNowFrameActorContactDistanceSQ = FLT_MAX;
		DNVector(DnActorHandle) hVecList;

		float fMinDistance = FLT_MAX;
		float fDist = 0.f;

		// Shared by projectiles carrying a hit signal and explosion hit signals.
		if( TRUE == bUseHitSignalArea || m_bOnCollisionCalled )
		{
			m_bProcessingBombHitSignal = true;

			MatrixEx CrossTemp = m_Cross;
			CrossTemp.MoveLocalZAxis( m_HitStruct.vOffset->z );
			CrossTemp.MoveLocalXAxis( m_HitStruct.vOffset->x );
			CrossTemp.MoveLocalYAxis( m_HitStruct.vOffset->y );

			EtVector3 vPos = CrossTemp.m_vPosition;

			float fDistance = max( m_HitStruct.fDistanceMax, (m_HitStruct.fHeightMax - m_HitStruct.fHeightMin) );
			float fXZDistanceSQ = m_HitStruct.fDistanceMax;
			float fXZDistanceMinSQ = m_HitStruct.fDistanceMin;

			fXZDistanceSQ *= fXZDistanceSQ;
			fXZDistanceMinSQ *= fXZDistanceMinSQ;

			CDnActor::ScanActor( GetRoom(), vPos, fDistance, hVecList );

			//#53454 If it is a Puppet summon actor, exclude the Puppet owner from HitList.
#if defined(PRE_FIX_61382)
			DNVector(DnActorHandle) hVecActorToApplyStateEffect;
			CDnActor::ExceptionHitList2(hVecList, m_Cross, GetShooterActor(), &m_HitStruct, hVecActorToApplyStateEffect, 1, fDistance, m_vPrevPos);
#else
			CDnActor::ExceptionHitList(hVecList, m_Cross, GetShooterActor(), &m_HitStruct);
#endif // PRE_FIX_61382

			EtVector3 vDir;
			EtVector3 vZVec = m_Cross.m_vZAxis;

			if( m_HitStruct.fCenterAngle != 0.f ) {
				EtMatrix matRotate;
				EtMatrixRotationY( &matRotate, EtToRadian( m_HitStruct.fCenterAngle ) );
				EtVec3TransformNormal( &vZVec, &vZVec, &matRotate );

#if defined(PRE_FIX_63356)
				CrossTemp.m_vZAxis = vZVec;
#endif // PRE_FIX_63356
			}

			SAABox Box;
			float fDot = 0.0f;
			bool bFirstHit = true;
			bool bCheckEndTime = false;

#ifdef PRE_FIX_PIERCE_WITH_HIT_AREA
			if( m_LastHitSignalEndTime > LocalTime )
			{
				if( m_bPierce )
				{
					bCheckEndTime = true;
				}
				else
				{
					return false;
				}
			}
#else
			if( m_LastHitSignalEndTime > LocalTime )
				return false;
#endif

			if( m_nLastHitSignalIndex != nSignalIndex ) {
				bFirstHit = true;
				m_nLastHitSignalIndex = nSignalIndex;
			}

			bool isFirstHitActor = true;

			int nHitCount = 0;
			bool isHitLimited = false;

			// Check Actor
			for( DWORD i=0; i<hVecList.size(); i++ ) 
			{
#ifdef PRE_FIX_PIERCE_WITH_HIT_AREA
				bool bFindHittedActor = false;
				if( m_bPierce == true )
				{
					if( !m_VecHittedActor.empty() )
					{
						std::vector<DnActorHandle>::iterator iter = find( m_VecHittedActor.begin(), m_VecHittedActor.end(), hVecList[i] );
						if( iter != m_VecHittedActor.end()  )
						{
							bFindHittedActor = true;
							if( bCheckEndTime )
								continue;
						}
					}
				}
#endif

				//////////////////////////////////////////////////////////////////////////
				// Limit hit count
				//If max Hit count is set, stop when Hit count exceeds max Hit count.
				isHitLimited = (m_HitParam.nHitLimitCount != 0 && nHitCount >= m_HitParam.nHitLimitCount);
				if (isHitLimited)
					break;
				//////////////////////////////////////////////////////////////////////////

				if (false == m_HitStruct.isSelfCheck)
				{
					// Do not check self.
					if( m_hShooter == hVecList[i] )
						continue;
				}

#if defined(PRE_FIX_59238)
				//Puppet and its summoner owner actor must not be hit simultaneously.
				if (IsHittable(hVecList[i]) == false)
					continue;
#endif // PRE_FIX_59238

				bool bSendRegist = false;
				if( !hVecList[i]->IsHittableSkill( m_ParentSkillInfo.iSkillID , bSendRegist ) )
				{
					if( bSendRegist )
					{
						hVecList[i]->SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE , STATE_BLOW::BLOW_154 );
					}
					continue;
				}

				if( !hVecList[i]->IsHittable( m_hShooter, LocalTime, &m_HitStruct, m_HitParam.iUniqueID ) ) 
					continue;

				switch( hVecList[i]->GetHitCheckType() ) {
					case CDnActor::BoundingBox: 
						{
							m_HitParam.vPosition = vPos;
							vDir = *hVecList[i]->GetPosition() - vPos;
							vDir.y = 0.f;

							hVecList[i]->GetBoundingBox( Box );

							if( CDnActor::SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
							if( CDnActor::SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

							EtVec3Normalize( &vDir, &vDir );
							fDot = EtVec3Dot( &vZVec, &vDir );
							if( EtToDegree( acos( fDot ) ) > m_HitStruct.fAngle ) continue;

							if( Box.Min.y < vPos.y + m_HitStruct.fHeightMin && 
								Box.Max.y < vPos.y + m_HitStruct.fHeightMin ) continue;
							if( Box.Min.y > vPos.y + m_HitStruct.fHeightMax && 
								Box.Max.y > vPos.y + m_HitStruct.fHeightMax ) continue;
							fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( vPos - m_vPrevPos ) );
							if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;

							m_HitParam.vViewVec = -vDir;
						}
						break;
					case CDnActor::Collision:
						{
							SCollisionCapsule				Capsule;
							SCollisionResponse				CollisionResult;
							DNVector(SCollisionResponse)	vCollisionResult;
							//
							Capsule.Segment.vOrigin = vPos;
							float fHeight = m_HitStruct.fHeightMax - m_HitStruct.fHeightMin;
							Capsule.Segment.vOrigin.y = Capsule.Segment.vOrigin.y - ( m_HitStruct.fHeightMin + ( fHeight / 2.f ) );
							Capsule.Segment.vDirection = EtVector3( 0.f, fHeight / 2.f, 0.f );
							Capsule.fRadius = m_HitStruct.fDistanceMax;

							EtVector3 vDestPos;
							if( hVecList[i]->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == false ) continue;
							if( CollisionResult.pCollisionPrimitive )
							{
								for( UINT k=0 ; k<vCollisionResult.size() ; ++k )
								{
									if( vCollisionResult[k].pCollisionPrimitive ) {
										vCollisionResult[k].pCollisionPrimitive->GetBoundingBox( Box );
										if( Box.Min.y < vPos.y + m_HitStruct.fHeightMin && Box.Max.y < vPos.y + m_HitStruct.fHeightMin ) {
											vCollisionResult.erase( vCollisionResult.begin() + k );
											k--;
											continue;
										}
										if( Box.Min.y > vPos.y + m_HitStruct.fHeightMax && Box.Max.y > vPos.y + m_HitStruct.fHeightMax ) {
											vCollisionResult.erase( vCollisionResult.begin() + k );
											k--;
											continue;
										}

										DNVector(EtVector3) vPointList;
										vPointList.push_back( Box.GetCenter() );
#ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
										if( vCollisionResult[k].pCollisionPrimitive->Type == CT_BOX ||
											vCollisionResult[k].pCollisionPrimitive->Type == CT_CAPSULE ) {
#else
										if( vCollisionResult[k].pCollisionPrimitive->Type == CT_BOX ) {
#endif // #ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
											Box.GetVertices( vPointList );
										}
										bool bCheck = false;
										for( DWORD m=0; m<vPointList.size(); m++ ) {
#if defined(PRE_FIX_63356)
											if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], m_HitStruct.fAngle) == true ) {
#else
											if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], m_HitStruct.fAngle) == true ) {
#endif // PRE_FIX_63356
												bCheck = true;
												break;
											}
										}
										if( bCheck == false ) {
											vCollisionResult.erase( vCollisionResult.begin() + k );
											k--;
											continue;
										}
										m_HitParam.vBoneIndex.push_back( hVecList[i]->GetObjectHandle()->GetParentBoneIndex( vCollisionResult[k].pCollisionPrimitive ) );
									}
								}
								if( vCollisionResult.empty() ) continue;

								GetCenterPos( *CollisionResult.pCollisionPrimitive, vDestPos );
								m_HitParam.vPosition = vDestPos;
							}
							else
							{
								ASSERT( 0 );
							}

							if( m_HitStruct.fDistanceMin > 100.f ) {
								vCollisionResult.clear();
								Capsule.fRadius = m_HitStruct.fDistanceMin;
								if( hVecList[i]->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == true ) {
									if( CollisionResult.pCollisionPrimitive )
									{
										for( UINT k=0 ; k<vCollisionResult.size() ; ++k )
										{
											if( vCollisionResult[k].pCollisionPrimitive ) {
												vCollisionResult[k].pCollisionPrimitive->GetBoundingBox( Box );
												if( Box.Min.y < vPos.y + m_HitStruct.fHeightMin && Box.Max.y < vPos.y + m_HitStruct.fHeightMin ) {
													vCollisionResult.erase( vCollisionResult.begin() + k );
													k--;
													continue;
												}
												if( Box.Min.y > vPos.y + m_HitStruct.fHeightMax && Box.Max.y > vPos.y + m_HitStruct.fHeightMax ) {
													vCollisionResult.erase( vCollisionResult.begin() + k );
													k--;
													continue;
												}
												DNVector(EtVector3) vPointList;
												vPointList.push_back( Box.GetCenter() );
#ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
												if( vCollisionResult[k].pCollisionPrimitive->Type == CT_BOX ||
													vCollisionResult[k].pCollisionPrimitive->Type == CT_CAPSULE ) {
#else
												if( vCollisionResult[k].pCollisionPrimitive->Type == CT_BOX ) {
#endif // #ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
													Box.GetVertices( vPointList );
												}
												bool bCheck = false;
												for( DWORD m=0; m<vPointList.size(); m++ ) {
#ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
													// If even one point is not inside the exclusion area, it overlaps the hit area, so do not exclude it...
#if defined(PRE_FIX_63356)
													if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], m_HitStruct.fAngle) == false ) {
#else
													if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], m_HitStruct.fAngle) == false ) {
#endif // PRE_FIX_63356
#else
#if defined(PRE_FIX_63356)
													if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], m_HitStruct.fAngle) == true ) {
#else
													if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], m_HitStruct.fAngle) == true ) {
#endif // PRE_FIX_63356
#endif // #ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
														bCheck = true;
														break;
													}
												}
#ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
												if( bCheck == true ) {
#else
												if( bCheck == false ) {
#endif // #ifdef PRE_FIX_COLMESH_RECTANGLE_HITSIGNAL
													vCollisionResult.erase( vCollisionResult.begin() + k );
													k--;
													continue;
												}
												int nBoneIndex = hVecList[i]->GetObjectHandle()->GetParentBoneIndex( vCollisionResult[k].pCollisionPrimitive );
												std::vector<int>::iterator it = std::find( m_HitParam.vBoneIndex.begin(), m_HitParam.vBoneIndex.end(), nBoneIndex );
												if( it != m_HitParam.vBoneIndex.end() ) {
													m_HitParam.vBoneIndex.erase( it );
												}
											}
										}
									}
								}
								if( m_HitParam.vBoneIndex.empty() ) continue;
							}

							m_HitParam.vViewVec = vPos - vDestPos;
							fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &m_HitParam.vViewVec );
							if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;

							EtVec3Normalize( &m_HitParam.vViewVec, &m_HitParam.vViewVec );
						}
						break;
				}

				m_HitParam.bFirstHit = bFirstHit;
				bFirstHit = false;
				bHitResult = true;
				bool bExistActionName = !m_HitParam.szActionName.empty();

				if( bExistActionName ) 
				{
					m_vHitPos = vPos;
					hResultProp.Identity();
				}

#if !defined( PRE_FIX_PROJECTILE_PREFIX_APPLY_POINT )
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
				// Trigger prefix skill in HitSignal regardless of whether it's a skill
				// Not applied if a state effect that ignores prefix state effects is active
				// #40186 Changed suffix activation condition (prevent activation when damage ratio is 0).
				if (m_HitParam.bFirstHit && 
					m_HitStruct.fDamageProb != 0.0f &&
					!m_hShooter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) && 
					isFirstHitActor)
				{
					m_hShooter->ProcessPrefixOffenceSkill_New();
					isFirstHitActor = false;
				}
#else
				//////////////////////////////////////////////////////////////////////////
				// Prepare prefix attack skill activation
				// For projectiles, if bFromProjectileSkill is set, it is a projectile from a skill.
				// Not applied if a state effect that ignores prefix state effects is active [2011/03/23 semozz]
				if( (!m_HitParam.bFromProjectileSkill && !m_hShooter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183))
					&& m_HitParam.bFirstHit && isFirstHitActor)
				{
					OutputDebug("CDnProjectile::ProcessDamage 1 -> STE_Hit start %d current %d end %d\n", SignalStartTime, LocalTime, SignalEndTime);

					//Here, only apply state effects that apply to self,
					//State effects to apply to the hit target are handled in Target->OnDamage.??
					m_hShooter->ProcessPrefixOffenceSkill(1.0f);

					isFirstHitActor = false;
				}

#endif // PRE_ADD_PREFIX_SYSTE_RENEW
#endif // PRE_FIX_PROJECTILE_PREFIX_APPLY_POINT

#ifdef PRE_FIX_PIERCE_WITH_HIT_AREA
				if( m_bPierce == true && bFindHittedActor == false )
					m_VecHittedActor.push_back( hVecList[i] );
#endif
				// Since CheckAndApplyDamage handles cases with and without an action name separately, it can be called outside the action name existence check.
				// For issues #23510, #23591: modified so state effects are still applied even if no damage is dealt.
				CheckAndApplyDamage( hVecList[i], true, &nHitCount );

				if( bHitResult )
				{
					// [2010/12/02 semozz]
					// Putting the Hit signal end time can catch the last frame and cause an extra hit.
					// So after Hit processing, set m_LastHitSignalEndTime to SignalEndTime + 1 so Hit is no longer processed.
					m_LastHitSignalEndTime = SignalEndTime + 1;

					if( hVecList[ i ]->IsDie() )
						iKillCount++;
				}

			}

#if defined(PRE_FIX_61382)
			//When both a summoned actor and its owner actor exist in the hit area, exclude the owner actor from the hit list.
			//State effects must be applied, so apply them at this point.
			int nApplyStateEffectActorListSize = (int)hVecActorToApplyStateEffect.size();
			for (int iIndex = 0; iIndex < nApplyStateEffectActorListSize; ++iIndex)
			{
				DnActorHandle hActor = hVecActorToApplyStateEffect[iIndex];
				ApplySkillStateEffect(hActor);
			}
#endif // PRE_FIX_61382

#if defined(PRE_ADD_50903)
			//Set final HitCount for damage distribution state effect.
			DnSkillHandle hParentSkill = GetParentSkill();
			if (hParentSkill)
				hParentSkill->SetHitCountForVarianceDamage(nHitCount);
#endif // PRE_ADD_50903

			// For projectiles carrying a hit signal, check props as well
			// Note: Prop check should skip state-effect-only skills like Heal. Distinguished by having no HitAction.
			if( false == m_HitParam.szActionName.empty() )
			{
				DNVector(CEtWorldProp*) pVecProp;
				CDnWorld* pWorld = CDnWorld::GetInstancePtr(GetRoom());
				pWorld->ScanProp( vPos, fDistance, pVecProp );

				for( DWORD i=0; i<pVecProp.size(); i++ ) 
				{
					if( false == ((CDnWorldProp*)pVecProp[i])->IsBrokenType() )
						continue;

					if( !((CDnWorldBrokenProp*)pVecProp[i])->IsHittable( m_hShooter, LocalTime ) ) 
						continue;

					pVecProp[i]->GetBoundingBox( Box );

					if( CDnActor::SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
					if( CDnActor::SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

					vDir = ((CDnWorldProp*)pVecProp[i])->GetMatEx()->m_vPosition - vPos;

					EtVec3Normalize( &vDir, &vDir );
					fDot = EtVec3Dot( &vZVec, &vDir );
					if( EtToDegree( acos( fDot ) ) > m_HitStruct.fAngle ) 
						continue;

					if( Box.Min.y < vPos.y + m_HitStruct.fHeightMin && 
						Box.Max.y < vPos.y + m_HitStruct.fHeightMin ) 
						continue;
					if( Box.Min.y > vPos.y + m_HitStruct.fHeightMax && 
						Box.Max.y > vPos.y + m_HitStruct.fHeightMax ) 
						continue;

					m_HitParam.vViewVec = -vDir;
					((CDnWorldProp*)pVecProp[i])->OnDamage( m_hShooter, m_HitParam );
				}
			}
		}
		else
		{
			size_t nCount = 0;
			nCount = CDnActor::ScanActor( GetRoom(), vCenter, fLength, hVecList );

			//#53454 If it is a Puppet summon actor, exclude the Puppet owner from HitList.
#if defined(PRE_FIX_61382)
			DNVector(DnActorHandle) hVecActorToApplyStateEffect;
			nCount = CDnActor::ExceptionHitList2(hVecList, m_Cross, GetShooterActor(), &m_HitStruct, hVecActorToApplyStateEffect, 2, fLength, m_vPrevPos);
#else
			nCount = CDnActor::ExceptionHitList(hVecList, m_Cross, GetShooterActor(), &m_HitStruct);
#endif // PRE_FIX_61382

			if( nCount > 0 )
			{
				m_LastHitSignalEndTime = SignalEndTime;

				float fThickness = max( fabs( m_HitStruct.fHeightMin ), fabs( m_HitStruct.fHeightMax ) );
				SSegment Segment;
				Segment.vOrigin = m_vPrevPos;
				Segment.vDirection = m_Cross.m_vZAxis * fLength;

				SCollisionCapsule Capsule;
				Capsule.Segment = Segment;
				Capsule.fRadius = fThickness;

				bool isFirstHitActor = true;

				int nHitCount = 0;
				bool isHitLimited = false;

				for( size_t i=0; i<nCount; i++ ) 
				{
					// Limit hit count
					//If max Hit count is set, stop when Hit count exceeds max Hit count.
					isHitLimited = (m_HitParam.nHitLimitCount != 0 && nHitCount >= m_HitParam.nHitLimitCount);
					if (isHitLimited)
						break;

					// Note: Slow-moving projectiles must be able to hit multiple times, so exclude this part..
					vector<DnActorHandle>::iterator iter = find( m_VecHittedActor.begin(), m_VecHittedActor.end(), hVecList[i] );
					if( !m_VecHittedActor.empty() )
					{
						if( iter != m_VecHittedActor.end() )
							continue;
					}

#if defined(PRE_FIX_59238)
					//Puppet and its summoner owner actor must not be hit simultaneously.
					if (IsHittable(hVecList[i]) == false)
						continue;
#endif // PRE_FIX_59238

					if( !hVecList[i]->IsHittable( m_hShooter, LocalTime, &m_HitStruct, m_HitParam.iUniqueID ) ) 
						continue;

					if( !hVecList[i]->GetObjectHandle() )
						continue;

					// For Chain Lightning projectiles, prevent the actor that fired the chain projectile from being hit again.
					// hPrevAttacker is set only when Chain Lightning is used.
					if( m_ParentSkillInfo.hPrevAttacker == hVecList[i] )
						continue;

					bool bResult = false;
					SCollisionResponse				Response;
					DNVector(SCollisionResponse)	vResponse;

					CDnActor::HitCheckTypeEnum eHitCheckType = hVecList[i]->GetHitCheckType();
					switch( eHitCheckType ) 
					{
						case CDnActor::BoundingBox:
							if( fThickness == 0.f ) bResult = hVecList[i]->GetObjectHandle()->FindSegmentCollision( Segment, Response );
							else bResult = hVecList[i]->GetObjectHandle()->FindCapsuleCollision( Capsule, Response );

							fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( *hVecList[i]->GetPosition() - m_vPrevPos ) );
							if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;
							break;
						case CDnActor::Collision:
							{
								int nParentBoneIndex = -1;
								EtVector3 vDestPos;
								if( fThickness == 0.f ) 
								{
									bResult = hVecList[i]->GetObjectHandle()->FindSegmentCollision( Segment, Response, &vResponse );
								}
								else
								{
									bResult = hVecList[i]->GetObjectHandle()->FindCapsuleCollision( Capsule, Response, &vResponse );
								}

								if( bResult ) 
								{
									fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( Segment.vDirection * Response.fContactTime ) );
									if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;

									for( UINT k=0 ; k<vResponse.size() ; ++k )
									{
										if( vResponse[k].pCollisionPrimitive )
											m_HitParam.vBoneIndex.push_back( hVecList[i]->GetObjectHandle()->GetParentBoneIndex( vResponse[k].pCollisionPrimitive ) );
									}
								}
							}
							break;
					}

					if( bResult )
					{
						EtVector3 vHitPos = Segment.vOrigin + (Segment.vDirection * Response.fContactTime);
						fDist = EtVec3Length( &( *hVecList[i]->GetPosition() - m_Cross.m_vPosition ) );
						float fLength2 = EtVec3Length( &( *hVecList[i]->GetPosition() - m_vStartPos ) );
						SSphere Sphere;
						hVecList[i]->GetBoundingSphere( Sphere );

						// This type is deleted immediately once extra range is exhausted after entering fallgravity state.
						if( RangeFallGravity == m_DestroyOrbitType )
						{
							float fAdditionalRange = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::RangeFallGravityAdditionalProjectileRange );
							float fRangeFallGravityLength = float(GetWeaponLength()) * fAdditionalRange;

							if( CDnActor::Collision == eHitCheckType )
							{
								// If it is a capsule, also account for the capsule's radius.
								if( 0.0f < fThickness )
								{
									fRangeFallGravityLength += Capsule.fRadius;
									if( CollisionType::CT_CAPSULE == Response.pCollisionPrimitive->Type )
									{
										fRangeFallGravityLength += static_cast<SCollisionCapsule*>(Response.pCollisionPrimitive)->fRadius;
									}
								}

								float fWeaponLengthSQ = fRangeFallGravityLength*fRangeFallGravityLength;
								EtVector3 vShootToHitPosDist = vHitPos - m_vStartPos;
								float fShootToHitPosDistSQ = EtVec3LengthSq( &vShootToHitPosDist );
								if( fWeaponLengthSQ < fShootToHitPosDistSQ )
									continue;
							}
							else
							if( fLength2 > fRangeFallGravityLength + ( Sphere.fRadius / 2.f ) )
							{
								continue;
							}
						}
						else
						{
							float fWeaponLength = float(GetWeaponLength());
							if( CDnActor::Collision == eHitCheckType )
							{
								// If it is a capsule, also account for the capsule's radius.
								if( 0.0f < fThickness )
								{
									fWeaponLength += Capsule.fRadius;
									if( CollisionType::CT_CAPSULE == Response.pCollisionPrimitive->Type )
									{
										fWeaponLength += static_cast<SCollisionCapsule*>(Response.pCollisionPrimitive)->fRadius;
									}
								}

								EtVector3 vShootToHitPosDist = vHitPos - m_vStartPos;
								float fShootToHitPosDistSQ = EtVec3LengthSq( &vShootToHitPosDist );
								if( fWeaponLength*fWeaponLength < fShootToHitPosDistSQ )
									continue;

							}
							else
							if( fLength2 > fWeaponLength + ( Sphere.fRadius / 2.f ) ) 
							{
								continue;
							}
						}

						if( fDist < fMinDistance ) 
						{
							fMinDistance = fDist;

							// Hits whatever is reached first along a straight line. If a prop is hit first, do not process hit on the actor.
							m_vHitPos = vHitPos;
							fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &(Segment.vDirection * Response.fContactTime) );
							if( Response.fContactTime > fPropContactTime ) continue;

							// Set the projectile position to the hit position.
							// Must do this for accurate checking when computing hit signal of explosion action.
							if( false == m_bPierce )
								m_Cross.m_vPosition = m_vHitPos;
						}

#if !defined( PRE_FIX_PROJECTILE_PREFIX_APPLY_POINT )				
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
						// Trigger prefix skill in HitSignal regardless of whether it's a skill
						// Not applied if a state effect that ignores prefix state effects is active
						// #40186 Changed suffix activation condition (prevent activation when damage ratio is 0).
						if (m_HitParam.bFirstHit && 
							m_HitStruct.fDamageProb != 0.0f &&
							!m_hShooter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) && 
							isFirstHitActor)
						{
							m_hShooter->ProcessPrefixOffenceSkill_New();
							isFirstHitActor = false;
						}
#else
						//////////////////////////////////////////////////////////////////////////
						// Prepare prefix attack skill activation
						// Trigger prefix system attack skill only on normal attack
						// For projectiles, if bFromProjectileSkill is set, it is a projectile from a skill.
						// Not applied if a state effect that ignores prefix state effects is active [2011/03/23 semozz]
						if( (!m_HitParam.bFromProjectileSkill && !m_hShooter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183))
							&& m_HitParam.bFirstHit && isFirstHitActor)
						{
							OutputDebug("CDnProjectile::ProcessDamage 2 -> STE_Hit start %d current %d end %d\n", SignalStartTime, LocalTime, SignalEndTime);

							//Here, only apply state effects that apply to self,
							//State effects to apply to the hit target are handled in Target->OnDamage.??
							m_hShooter->ProcessPrefixOffenceSkill(1.0f);

							isFirstHitActor = false;
						}
						//////////////////////////////////////////////////////////////////////////
#endif	// PRE_ADD_PREFIX_SYSTE_RENEW
#endif

						m_VecHittedActor.push_back( hVecList[i] );
						
						CheckAndApplyDamage( hVecList[i], true, &nHitCount );
						
						bHitResult = true;
						if( hVecList[ i ]->IsDie() )
							iKillCount++;
						
						if(false == m_bPierce)
							return bHitResult;
					}
				}

#if defined(PRE_FIX_61382)
				//When both a summoned actor and its owner actor exist in the hit area, exclude the owner actor from the hit list.
				//State effects must be applied, so apply them at this point.
				int nApplyStateEffectActorListSize = (int)hVecActorToApplyStateEffect.size();
				for (int iIndex = 0; iIndex < nApplyStateEffectActorListSize; ++iIndex)
				{
					DnActorHandle hActor = hVecActorToApplyStateEffect[iIndex];
					ApplySkillStateEffect(hActor);
				}
#endif // PRE_FIX_61382

#if defined(PRE_ADD_50903)
				//Set final HitCount for damage distribution state effect.
				DnSkillHandle hParentSkill = GetParentSkill();
				if (hParentSkill)
					hParentSkill->SetHitCountForVarianceDamage(nHitCount);
#endif // PRE_ADD_50903
			}
		}

		// Since m_vHitPos used in MakeHitParam is updated during actor check, process props here after actor processing finishes.
		if( hResultProp )
		{
			switch( hResultProp->GetPropType() ) 
			{
				case PTE_Static:
					{
#if defined(PRE_FIX_52329)
						//Skip Valid check if IgnoreHitType is non-zero..
						if (m_nIgnoreHitType == 0)
						{
							if( m_bValidDamage ) 
							{
								m_bValidDamage = false;
								m_InvalidLocalTime = LocalTime;
							}
							m_bStick = true;
						}
#endif
					}
					break;
				case PTE_Action:
				case PTE_Trap:
					if( m_bValidDamage ) 
					{
						m_bValidDamage = false;
						m_InvalidLocalTime = LocalTime;
					}
					m_bStick = true;
					break;

				case PTE_Broken:
				case PTE_BrokenDamage:
				case PTE_HitMoveDamageBroken:
				case PTE_ShooterBroken:
				case PTE_BuffBroken:
					{
						if( !m_bValidDamage ) break;

						// Should not hit if distance to prop exceeds attack range. #23322
						float fLengthToPropSQ = EtVec3LengthSq(&EtVector3(m_vStartPos - hResultProp->GetMatEx()->m_vPosition));

						// This type is deleted immediately once extra range is exhausted after entering fallgravity state.
						float fWeaponLength = 0.0f;
						if( RangeFallGravity == m_DestroyOrbitType )
						{
							float fAdditionalRange = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::RangeFallGravityAdditionalProjectileRange );
							fWeaponLength = float(GetWeaponLength()) * fAdditionalRange;
						}
						else
						{
							fWeaponLength = (float)GetWeaponLength();
						}

						float fWeaponLengthSQ = fWeaponLength * fWeaponLength;
						if( fWeaponLengthSQ < fLengthToPropSQ )		// Contains squared fPropDistance value.
							break;

						if( ((CDnWorldBrokenProp*)hResultProp.GetPointer())->IsHittable( m_hShooter, LocalTime ) ) {
							CDnDamageBase::SHitParam HitParam;
							MakeHitParam( HitParam );

							hResultProp->OnDamage( m_hShooter, HitParam );
						}
						else {
							m_bValidDamage = false;
							m_InvalidLocalTime = LocalTime;
							m_bStick = true;
						}
					}
					break;
			}

#if defined(PRE_FIX_52329)
			//Skip Valid check if IgnoreHitType is non-zero..
			if (m_nIgnoreHitType == 0)
			{
				OnCollisionWithProp();
			}
#endif
		}
	}

	if( 2 <= iKillCount && m_hShooter->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hShooter.GetPointer());
		CDNUserSession* pSession = pPlayerActor->GetUserSession();
		pSession->GetEventSystem()->OnEvent( EventSystem::OnKillMonster, 1,
											   EventSystem::GenocideCount, iKillCount );
	}

	return bHitResult;
}

void CDnProjectile::CheckAndApplyDamage( DnActorHandle hActor, bool bValidDamageGoOn/* = true*/, int *pHitCount /*= NULL*/ )
{
	if( !m_hShooter || !hActor ) return;

	// Prevent hitting tracked actor depending on option. Default is no hit.
	if( false == m_bTraceHitActorHittable )
	{
		if( m_hTraceActor == hActor ) 
			return;
	}

	if( !m_HitParam.szActionName.empty() && !hActor->IsDie() ) 
	{
		CDnDamageBase::SHitParam HitParam;
		MakeHitParam( HitParam );

		// If projectile was fired by Charger state effect, set it on actor, call OnDamage, and immediately clear it.
		// Exclude projectiles fired from Charger state effect from combo calculation.
		if( m_bFromCharger && m_hShooter )
			m_hShooter->SetAllowCombo( false );

		hActor->OnDamage( m_hShooter, HitParam, &m_HitStruct );

#if defined(PRE_FIX_59238)
		AddHittedActor(hActor);
#endif // PRE_FIX_59238

		if (pHitCount)
			(*pHitCount)++;

		if( m_bFromCharger && m_hShooter )
			m_hShooter->SetAllowCombo( true );

		bValidDamageGoOn = SetTraceActor( hActor, bValidDamageGoOn );

		OnCollisionWithActor();

		// Note: Although ValidDamage is turned back to true in OnCollision() to handle explosion damage
		// If bValidDamageGoOn is off when calling this function, turn off ValidDamage since explosion damage is not applied.
		if( false == bValidDamageGoOn )
			m_bValidDamage = false;
	}

	//Apply projectile skill state effect.
	ApplySkillStateEffect(hActor);

	// Only for piercing type; if max hit count is 0, it keeps hitting until the end as originally processed.
	if( m_bPierce && m_nMaxHitCount != 0 )
	{
		//m_nMaxHitCount--;
		map<CDnActor*, int>::iterator iter = m_mapMaxHitCount.find(hActor.GetPointer() );
		if( m_mapMaxHitCount.end() == iter )
			m_mapMaxHitCount[ hActor.GetPointer() ] = 0;
		else
		{
			++(iter->second);
			if( m_nMaxHitCount-1 == iter->second )
			{
				m_bValidDamage = false;
				m_InvalidLocalTime = m_LocalTime;

				// Client cannot know, so server sends the packet.
				BYTE pBuffer[ 32 ];
				CPacketCompressStream Stream( pBuffer, 32 );
				Stream.Write( &m_dwUniqueID, sizeof(DWORD) );
				m_hShooter->Send( eActor::SC_INVALID_PROJECTILE, &Stream );
			}
		}
	}
}

void CDnProjectile::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_CreateLocalTime == 0 ) m_CreateLocalTime = LocalTime;

	// Hanki 2009.7.22
	// Elapsed time since projectile was created
	// Related to projectile trajectory.. (#4851)
	m_fElapsedTime = float(LocalTime - m_CreateLocalTime) * 0.001f;

	m_bHitSignalProcessed = false;

	ProcessOrbit( LocalTime, fDelta );

	CDnWeapon::Update( m_Cross );
	CDnWeapon::Process( LocalTime, fDelta );

	if(  0 != m_InvalidLocalTime ) 
		ProcessDestroyOrbit( LocalTime, fDelta );

	ProcessValid( LocalTime, fDelta );

	m_vPrevPos = m_Cross.m_vPosition;

	m_bFirstProcess = false;

	// Added defensive code for map 301 (Isolated Island) where spit-flower projectile direction vector becomes (0, 0, 0) and fails to disappear when player position is off.
	if( EtVec3LengthSq( &m_Cross.m_vZAxis ) < 0.001f )
	{
		// Projectile-type projectiles may have speed set to 0, because speed is calculated in real time~
		if( Projectile != m_OrbitType )
			SetDestroy();
	}
}

void CDnProjectile::MakeHitParam( CDnDamageBase::SHitParam &HitParam )
{
	HitParam = m_HitParam;

	HitParam.RemainTime = m_LastHitSignalEndTime;
	HitParam.hHitter = m_hShooter;
	HitParam.hWeapon = GetMySmartPtr();
	HitParam.bFirstHit = true;
	HitParam.vPosition = m_vHitPos;

	//  Exploding projectiles using area hit signals already have the proper view vector based on the hit signal center. 
	if( !m_bProcessingBombHitSignal )
		HitParam.vViewVec = -m_Cross.m_vZAxis;

	HitParam.iUniqueID = m_iHitUniqueID;

	if( -1.0f != m_fHitApplyPercent )
		HitParam.fDamage = m_fHitApplyPercent;
}

void CDnProjectile::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, 
							  LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Hit:
			{
				//OutputDebug( "STE_Hit\n" );

				if (m_hShooter && m_hShooter->ProcessIgnoreHitSignal() == true)
					break;

				HitStruct *pStruct = (HitStruct *)pPtr;
#ifdef PRE_FIX_MEMOPT_SIGNALH
				CopyShallow_HitStruct(m_HitStruct, pStruct);
#else
				m_HitStruct = *pStruct;
#endif

				m_HitParam.szActionName = pStruct->szTargetHitAction;
				m_HitParam.fDamage = pStruct->fDamageProb * 0.01f;
				m_HitParam.fDurability = pStruct->fDurabilityDamageProb * 0.01f;
				m_HitParam.vVelocity = *pStruct->vVelocity;
				m_HitParam.vResistance = *pStruct->vResistance;
				m_HitParam.fStiffProb = pStruct->fStiffProb * 0.01f;
				m_HitParam.nSkillSuperAmmorIndex = pStruct->nApplySuperAmmorIndex - 1;
				m_HitParam.nSkillSuperAmmorDamage = pStruct->nApplySuperAmmorDamage;
				m_HitParam.nDamageType = pStruct->nDamageType;
				m_HitParam.DistanceType	= (pStruct->nDistanceType == 0) ? CDnDamageBase::DistanceTypeEnum::Melee : CDnDamageBase::DistanceTypeEnum::Range;
				m_HitParam.cAttackType = (char)pStruct->nAttackType;
				
				if( ElementEnum_Amount != m_eForceHitElement )
				{
					m_HitParam.HasElement = m_eForceHitElement;
				}
				else
				{
					m_HitParam.HasElement = ( m_hShooter ) ? m_hShooter->CalcHitElementType( ( pStruct->bUseSkillApplyWeaponElement == TRUE ) ? true : false, m_ParentSkillInfo.eSkillElement, true ) : CDnActorState::ElementEnum_Amount;
				}

				m_HitParam.bIgnoreCanHit = ( pStruct->bIgnoreCanHit == TRUE );
				m_HitParam.bIgnoreParring = ( pStruct->bIgnoreParring == TRUE );

				m_HitParam.nHitLimitCount = pStruct->nHitLimitCount;

				if( m_bValidDamage )
				{
					bool bHitResult = ProcessDamage( LocalTime, 0.0f, SignalStartTime, SignalEndTime, nSignalIndex, pStruct->bUseHitAreaOnProjectile );
					if( bHitResult )
					{
						// #59118 Apply g_Lua_BeHitSkill to projectiles as well.
						if( m_hShooter && m_hShooter->IsMonsterActor() )
						{
							CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(m_hShooter.GetPointer());
#ifdef PRE_FIX_PROPMON_PROJ
							//	Prop monsters can be MonsterActor without having AI. (#59952)
							if (pMonster)
							{
								MAAiBase* pCurBase = pMonster->GetAIBase();
								if (pCurBase)
									pCurBase->OnHitFinish( LocalTime, &m_HitStruct );
							}
#else
							pMonster->GetAIBase()->OnHitFinish( LocalTime, &m_HitStruct );
#endif
						}
					}
				}

				m_bHitSignalProcessed = true;

				// Since the hit signal can appear later during explosion, turn on the flag when the hit signal starts.
				// Otherwise m_bValidDamage becomes false and explosion processing fails.
				if( m_bOnCollisionCalled && m_bHitActionStarted )
					m_bBombHitSignalStarted = true; 
			}
			break;
			case STE_SummonMonster:
			{
				SummonMonsterStruct* pStruct = (SummonMonsterStruct *)pPtr;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));
				if( pGameTask )
				{
					if( m_hShooter )
					{
						// Must copy before use because it modifies the value internally.
#ifdef PRE_FIX_MEMOPT_SIGNALH
						SummonMonsterStruct Struct;
						CopyShallow_SummonMonsterStruct(Struct, pStruct);
#else
						SummonMonsterStruct Struct = *pStruct;
#endif

						// Handle state effect #167: force summoned monster skill level.
						if( m_hShooter->IsPlayerActor() )
						{
							if( 0 < m_iSummonMonsterForceSkillLevel )
								Struct.nForceSkillLevel = m_iSummonMonsterForceSkillLevel;
						}
						//////////////////////////////////////////////////////////////////////////

						// When summoning a monster from a projectile, spawn it at the projectile's position instead. #18315

						// #36873 Since navigation mesh or attributes are ignored, use the adjusted position.
						// #37991 Check if navmesh is valid at the previous position; if not, summon at the character's current standing position.
						MatrixEx CrossRevisionedPos;
						if( true == INSTANCE(CDnWorld).IsOnNavigationMesh( m_vPrevPos ) )
						{
							CrossRevisionedPos = m_Cross;
							INSTANCE(CDnWorld).RevisionPosByNaviMesh( CrossRevisionedPos, m_vPrevPos );
						}
						else
						{
							CrossRevisionedPos.m_vPosition = m_hShooter->GetMatEx()->m_vPosition;
							if( Struct.PositionCheck == TRUE )
								break;
						}

						EtVector3 vTemp = *(m_hShooter->GetPosition());;
						m_hShooter->SetPosition( CrossRevisionedPos.m_vPosition );
						pGameTask->RequestSummonMonster( m_hShooter, &Struct, false, m_ParentSkillInfo.iSkillID );
						m_hShooter->SetPosition( vTemp );
					}
				}
				break;
			}
		case STE_Gravity:
			{
				if( !m_hShooter ) break;
				GravityStruct *pStruct = (GravityStruct *)pPtr;

				EtVector3 vPos = m_Cross.m_vPosition + *pStruct->vOffset;
				DNVector(DnActorHandle) hVecList;
				CDnActor::ScanActor( GetRoom(), vPos, pStruct->fDistanceMax, hVecList );

				for( DWORD i=0; i<hVecList.size(); i++ ) {
					if( !hVecList[i] ) continue;
					if( hVecList[i] == m_hShooter ) continue;
					if( hVecList[i]->IsDie() ) continue;
					if( hVecList[i]->GetWeight() == 0.f ) continue;
					if( hVecList[i]->IsNpcActor() ) continue;
					if( hVecList[i]->ProcessIgnoreGravitySignal() ) continue;

					bool bHittable = false;
					switch( pStruct->nTargetType ) {
						case 0: // Enemy
							if( hVecList[i]->GetTeam() == m_hShooter->GetTeam() ) break;
							bHittable = true;
							break;
						case 1: // Friend
							if( hVecList[i]->GetTeam() != m_hShooter->GetTeam() ) break;
							bHittable = true;
							break;
						case 2: // All
							bHittable = true;
							break;
					}
					if( !bHittable ) 
						continue;

					MAMovementBase *pMovement = hVecList[i]->GetMovement();
					if( !pMovement ) continue;

					EtVector3 vDir = *hVecList[i]->GetPosition() - vPos;
					float fDistance = EtVec3Length( &vDir );
					float fDistance2 = EtVec3Length( &EtVector3( vDir.x, 0.f, vDir.z ) );
					EtVec3Normalize( &vDir, &vDir );

					float fTemp = pStruct->fMinVelocity + ( ( (pStruct->fMaxVelocity - pStruct->fMinVelocity) / pStruct->fDistanceMax ) * ( pStruct->fDistanceMax - fDistance ) );

					EtVector3 vVel;
					if( pMovement ) {
						pMovement->Look( EtVec3toVec2(vDir) );
						pMovement->SetVelocityZ( -fTemp );
						pMovement->SetResistanceZ( fTemp * 2.f );
					}
				}
			}
			break;
	}
	CDnWeapon::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}


void CDnProjectile::OnDamageSuccess( DnActorHandle hDamagedActor, CDnDamageBase::SHitParam &HitParam )
{
	// Set whether to remove or not here depending on piercing or other types.
	// Do not process Destroy when hit by an exploding object.
	if( false == m_bPierce && /*m_bHasHitAction &&*/ !m_bOnCollisionCalled )
	{
		ShowWeapon( false );
		m_bValidDamage = false;
		m_InvalidLocalTime = m_LocalTime;
		m_DestroyOrbitType = FallGravity;
		m_nDestroyOrbitTimeGap = 1000;
		m_bStick = true;
	}

	// Notify the actor that fired the projectile that the projectile hit.
	if( m_hShooter )
	{
		m_hShooter->OnHitProjectile( m_LocalTime, hDamagedActor, HitParam );
	}
}

void CDnProjectile::AddStateEffect( CDnSkill::StateEffectStruct &Struct )
{
#if defined(PRE_FIX_59336)
	//Manage only state effect 242 in a separate list.
	if (Struct.nID == STATE_BLOW::BLOW_242)
	{
		m_ComboLimitStateEffectList.push_back(Struct);
	}
	else
	{
		m_VecStateEffectList.push_back( Struct );
	}
#else
	m_VecStateEffectList.push_back( Struct );
#endif // PRE_FIX_59336
}


void CDnProjectile::OnCollisionWithActor( void )
{
	if( false == m_bPierce && false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
#if defined(PRE_ADD_55295)
			//#55295
			//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
			if (m_bHitActionVectorInit)
				ChangeProjectileRotation();
#endif // PRE_ADD_55295

			SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );

			m_bOnCollisionCalled = true;

			if( false == m_bValidDamage )
				m_bValidDamage = true;

			// Extract duration matching the hit action length
			CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
			m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFPS) * 1000.0f);

			// Since a new explosion hit signal will run, reset the end signal time based on the first actor hit by the projectile. Otherwise explosion damage will not apply.
			m_LastHitSignalEndTime = 0;
		}
	}
}


void CDnProjectile::OnCollisionWithGround( void )
{
	if( false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
#if defined(PRE_ADD_55295)
			//#55295
			//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
			if (m_bHitActionVectorInit)
				ChangeProjectileRotation();
#endif // PRE_ADD_55295

			SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );

			m_bOnCollisionCalled = true;

			if( false == m_bValidDamage )
				m_bValidDamage = true;

			// Extract duration matching the hit action length
			CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
			m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFPS) * 1000.0f);

			// If TargetPositionType, set destination position immediately
			if( 2 == m_TargetType )
				m_Cross.SetPosition( m_vTargetPosition );

			// Since a new explosion hit signal will run, reset the end signal time based on the first actor hit by the projectile. Otherwise explosion damage will not apply.
			m_LastHitSignalEndTime = 0;
		}
	}
}


void CDnProjectile::OnCollisionWithProp( void )
{
	if( false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
#if defined(PRE_ADD_55295)
			//#55295
			//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
			if (m_bHitActionVectorInit)
				ChangeProjectileRotation();
#endif // PRE_ADD_55295

			// If already executing hit action, do not re-execute the action. (#21384)
			SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );

			m_bOnCollisionCalled = true;

			if( false == m_bValidDamage )
				m_bValidDamage = true;

			// Extract duration matching the hit action length
			CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
			m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFPS) * 1000.0f);

			// Since a new explosion hit signal will run, reset the end signal time based on the first actor hit by the projectile. Otherwise explosion damage will not apply.
			m_LastHitSignalEndTime = 0;
		}
		else
		{
			// If hitting a prop, immediately delete projectiles that have no hit action or have a hit action without a hit signal.
			SetDestroy();
		}
	}
}


void CDnProjectile::OnChangedNextActionToHit( void )
{
	if( false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
			m_bOnCollisionCalled = true;

			if( false == m_bValidDamage )
				m_bValidDamage = true;

			// Extract duration matching the hit action length
			CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
			m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFPS) * 1000.0f);

			// If TargetPositionType, set destination position immediately
			if( 2 == m_TargetType )
				m_Cross.SetPosition( m_vTargetPosition );
		}
	}
}


void CDnProjectile::OnChangeAction( const char *szPrevAction )
{
	if( m_bHasHitAction )
	{
		if( NULL != szPrevAction && 
			strcmp( SHOOT_ACTION_NAME, szPrevAction ) )
		{
			m_bHitActionStarted = true;
		}
	}

	// m_LastHitSignalEndTime exists to prevent a hit target from being hit again, but if the projectile's action itself changes, EndTime is set based on the previous signal, so
	// Even if the action changes and a new Hit arrives, using the previous value causes a malfunction.
	// So reset m_LastHitSignalEndTime when action changes; to be explicit, remember previous hit target info per projectile action so hit targets aren't hit again
	// Should be configured, but for now set it as currently written.
	m_LastHitSignalEndTime = 0;
}

void CDnProjectile::OnFinishAction( const char* szPrevAction, LOCAL_TIME time )
{
	// Delete immediately when explosion action ends.
	if( m_bHasHitAction ) 
		if( 0 == strcmp( HIT_ACTION_NAME, szPrevAction ) )
			SetDestroy();
}


// NOTE: For CrossHair projectiles, the server receives projectile info directly as created by the client (CS_PROJECTILE)
// Not created here.
// Create by directly receiving the projectile signal.
CDnProjectile *CDnProjectile::CreateProjectile( CMultiRoom *pRoom, DnActorHandle hShooter, MatrixEx &LocalCross, ProjectileStruct *pStruct, 
												EtVector3* pForceTargetPos/*=NULL*/, DnActorHandle hForceTarget/*=CDnActor::Identity()*/, EtVector3* pForceDir/* = NULL*/ )
{
	MatrixEx CrossResult = LocalCross;
	CrossResult.m_vPosition += CrossResult.m_vXAxis * ( pStruct->vOffset->x * hShooter->GetScale()->x );
	CrossResult.m_vPosition += CrossResult.m_vYAxis * ( pStruct->vOffset->y * hShooter->GetScale()->y );
	CrossResult.m_vPosition += CrossResult.m_vZAxis * ( pStruct->vOffset->z * hShooter->GetScale()->z );

	CDnProjectile *pProjectile = NULL;

	float fVelocityMultiply = 1.0f;

	// The direction value in the signal struct only represents direction regardless of length,
	// Normalize CrossResult.m_vZAxis; speed is determined solely by fSpeed specified in the signal struct.

	switch( pStruct->nTargetType )
	{
		case TargetTypeEnum::CrossHair:
			{
				if( hShooter && hShooter->IsPlayerActor() && static_cast<CDnPlayerActor*>(hShooter.GetPointer())->IsAIBotActor() )
				{
					pProjectile = new CDnProjectile( pRoom, hShooter );
					EtVector3 vAimDir = LocalCross.m_vZAxis;
					if( pForceTargetPos )
					{
						vAimDir = *pForceTargetPos - CrossResult.GetPosition();
						if( EtVec3LengthSq( &vAimDir ) > 1.f )
							EtVec3Normalize( &vAimDir, &vAimDir );
						else
							vAimDir = LocalCross.m_vZAxis;
					}
					CrossResult.m_vZAxis = vAimDir;
					CrossResult.MakeUpCartesianByZAxis();
					break;
				}
				SAFE_DELETE( pProjectile );
				return NULL;
			}
			break;

		case TargetTypeEnum::Direction:
			{
				pProjectile = new CDnProjectile( pRoom, hShooter );

				EtVector3	vDirection	= *pStruct->vDirection;
				bool		bAILook		= false;

				// MonsterActor
				if( hShooter && hShooter->IsMonsterActor() )
				{
					CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(hShooter.GetPointer());

					if( pMonster && pMonster->bIsAILook() )
					{
						// STE_ProjectileTargetPosition is a signal used only for TargetPosition.
						if( !pMonster->bIsProjectileTargetSignal() )
						{
							vDirection	= EtVec2toVec3( *pMonster->GetAIBase()->GetAILook() );
							bAILook		= true;
						}
					}
				}

				EtVector3 vTemp;
				if( bAILook )
					vTemp = vDirection;
				else
					EtVec3TransformNormal( &vTemp, &vDirection, LocalCross );

				if( FALSE == pStruct->bNormalizeDirectionVector )
				{
					fVelocityMultiply = EtVec3Length( &vTemp );
					CrossResult.m_vZAxis = vTemp / fVelocityMultiply;
				}
				else
				{
					fVelocityMultiply = 1.0f;
					EtVec3Normalize( &vTemp, &vTemp );
					CrossResult.m_vZAxis = vTemp;
				}

				CrossResult.MakeUpCartesianByZAxis();
			}
			break;

		case TargetTypeEnum::TargetPosition:
		case TargetTypeEnum::Target:
		case TargetTypeEnum::Shooter:
			{
				if( hShooter->IsPlayerActor() )
				{
					CDnPlayerActor* pBotPlayer = static_cast<CDnPlayerActor*>( hShooter.GetPointer() );
					if( !pBotPlayer->IsAIBotActor() )
					{
						SAFE_DELETE( pProjectile );
						return NULL;
					}
					pProjectile = new CDnProjectile( pRoom, hShooter );
					DnActorHandle hTarget = hForceTarget;
					EtVector3 vTargetPos = pForceTargetPos ? *pForceTargetPos : ( *hShooter->GetPosition() + ( hShooter->GetMatEx()->m_vZAxis * 600.f ) );
					if( hTarget )
						vTargetPos = *hTarget->GetPosition();
					if( pStruct->nTargetType == TargetTypeEnum::TargetPosition )
					{
						pProjectile->SetTargetPosition( vTargetPos );
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					else if( pStruct->nTargetType == TargetTypeEnum::Target )
					{
						if( !hTarget ) hTarget = hShooter;
						pProjectile->SetTargetPosition( vTargetPos );
						pProjectile->SetTargetActor( hTarget );
					}
					else if( pStruct->nTargetType == TargetTypeEnum::Shooter )
					{
						hTarget = hShooter;
						vTargetPos = *hShooter->GetPosition();
						pProjectile->SetTargetActor( hTarget );
						pProjectile->SetTargetPosition( vTargetPos );
					}
					CrossResult.MakeUpCartesianByZAxis();
				}
				else if( hShooter->IsMonsterActor() )
				{
					pProjectile = new CDnProjectile( pRoom, hShooter );

					CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(hShooter.GetPointer());

					// When the firing target is the Shooter itself
					DnActorHandle hTarget;

					// Retrieve pre-designated target from ReserveProjectileTarget signal if available.
					// If none, get it from the original AI.
					hTarget = pMonster->GetReservedProjectileTarget();
					if( !hTarget )
						hTarget = pMonster->GetAggroTarget();

					EtVector3 vTargetPos;

					bool bTarget = true;
					// Force target setting
					if( pForceTargetPos )
						vTargetPos = *pForceTargetPos;
					else if( pStruct->nTargetType == TargetTypeEnum::TargetPosition && pMonster->bIsProjectileTargetSignal() )
						vTargetPos = *pMonster->GetAIBase()->GetProjectileTarget();
					else if( hTarget )
						vTargetPos = *hTarget->GetPosition();
					else
						bTarget = false;

					if( bTarget )
					{
						// TargetPosRandomValue
						if( pStruct->nTargetPosRandomValue > 0 )
						{
							_srand( pRoom, pProjectile->GetUniqueID() );
							vTargetPos.x += cos( EtToRadian( _rand(pRoom)%360 ) ) * pStruct->nTargetPosRandomValue;
							vTargetPos.z += sin( EtToRadian( _rand(pRoom)%360 ) ) * pStruct->nTargetPosRandomValue;
						}

						float fHeight = CDnWorld::GetInstance(pRoom).GetHeight( vTargetPos );
						
						// #28665 Only if not a cannon mob. Do not touch the calculated cannon destination.
#ifdef PRE_FIX_PARTSMONSTER_AI_TARGETTING
						if( hTarget && fHeight < vTargetPos.y && (pMonster->GetActorType() != CDnActorState::Cannon) && hTarget->IsPartsMonsterActor() == false )
							vTargetPos -= hTarget->GetMatEx()->m_vZAxis * ( vTargetPos.y - fHeight );
#else
						if( hTarget && fHeight < vTargetPos.y && (pMonster->GetActorType() != CDnActorState::Cannon) )
							vTargetPos -= hTarget->GetMatEx()->m_vZAxis * ( vTargetPos.y - fHeight );
#endif
					}
					else
					{
						vTargetPos = *hShooter->GetPosition() + ( hShooter->GetMatEx()->m_vZAxis * pMonster->GetThreatRange() );
					}

					// TargetPosition
					if( pStruct->nTargetType == TargetTypeEnum::TargetPosition ) 
					{
						pProjectile->SetTargetPosition( vTargetPos );

						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}

					// Target
					if( pStruct->nTargetType == TargetTypeEnum::Target )
					{
						// Force target setting
						if( hForceTarget )
							hTarget = hForceTarget;

						if( !hTarget ) 
							hTarget = hShooter;

						if (pStruct->nTargetStateIndex != 0)
						{
							vTargetPos = *hTarget->GetPosition();
							pProjectile->SetTargetPosition(vTargetPos);
						}

						pProjectile->SetTargetActor( hTarget );
					}
					else if( pStruct->nTargetType == TargetTypeEnum::Shooter )
					{
						hTarget = hShooter;
						vTargetPos = *hTarget->GetPosition();

						pProjectile->SetTargetActor( hTarget );
						pProjectile->SetTargetPosition( vTargetPos );
					}

					//For a projectile flying to a target point, update the direction vector from current projectile position to target point.
					//Modify to work only on brand projectiles for now.
					if (pStruct->nTargetStateIndex != 0)
					{
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					else if( EtVec3LengthSq( pStruct->vDirection ) == 0.f ) 
					{
						_ASSERT( !(pStruct->nOrbitType == Projectile && pStruct->VelocityType == Accell && pStruct->nTargetType == 2) &&
							"Gravity-affected Projectile/TargetPosition projectiles must specify a valid direction." );
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					else if( !( (pStruct->nOrbitType == OrbitTypeEnum::Linear) && 
						(pStruct->nTargetType == TargetTypeEnum::TargetPosition) ) )
					{
						// When firing in the direction chosen by the user from the cannon, set that direction.
						if( NULL == pForceDir )
						{
							EtVector3 vTemp;
							EtVec3TransformNormal( &vTemp, pStruct->vDirection, LocalCross );

							if( pStruct->bUseTargetPositionDir )
							{
								EtVector3 vTargetPosDir = vTargetPos - CrossResult.m_vPosition;
								EtVec3Normalize( &vTargetPosDir, &vTargetPosDir );

								MatrixEx CrossTargetPosDir;
								CrossTargetPosDir.m_vZAxis = vTargetPosDir;
								CrossTargetPosDir.MakeUpCartesianByZAxis();

								EtVector3 vToolDefinedDir = *pStruct->vDirection;
								EtVector3 vDirZ( vToolDefinedDir.x, 0.0f, vToolDefinedDir.z );
								EtVec3Normalize( &vDirZ, &vDirZ );
								EtVec3Normalize( &vToolDefinedDir, &vToolDefinedDir );
								float fDot = EtVec3Dot( &vToolDefinedDir, &vDirZ );
								float fRotX = EtAcos( fDot );

								CrossTargetPosDir.RotatePitch( EtToDegree( -fRotX ) );
								vTemp = CrossTargetPosDir.m_vZAxis;
							}

							if( FALSE == pStruct->bNormalizeDirectionVector )
							{
								fVelocityMultiply = EtVec3Length( &vTemp );
								CrossResult.m_vZAxis = vTemp / fVelocityMultiply;
							}
							else
							{
								fVelocityMultiply = 1.0f;
								EtVec3Normalize( &vTemp, &vTemp );
								CrossResult.m_vZAxis = vTemp;
							}
						}
						else
						{
							EtVector3 vTemp;
							EtVec3TransformNormal( &vTemp, pForceDir, LocalCross );
							EtVec3Normalize( &CrossResult.m_vZAxis, &vTemp );
							pProjectile->SetForceDir( *pForceDir );
						}
					}

					CrossResult.MakeUpCartesianByZAxis();
				}
			}
			break;

		case TargetTypeEnum::DestPosition:
			{
				pProjectile = new CDnProjectile( pRoom, hShooter );

				EtVector3 vDestOffset = *(pStruct->vDestPosition);
				if( pStruct->nTargetPosRandomValue > 0 )
				{
					_srand( pRoom, pProjectile->GetUniqueID() );
					vDestOffset.x += cos( EtToRadian( _rand(pRoom)%360 ) ) * pStruct->nTargetPosRandomValue;
					vDestOffset.z += sin( EtToRadian( _rand(pRoom)%360 ) ) * pStruct->nTargetPosRandomValue;
				}

				// If the direction vector is set in the tool, turn toward that direction instead of dest position.
				EtVector3 vDirection;
				if( EtVec3LengthSq( pStruct->vDirection ) != 0.f )
				{
					vDirection = vDestOffset;
					EtVec3Normalize( &vDirection, &vDirection );

					MatrixEx CrossDestDir;
					CrossDestDir.m_vZAxis = vDirection;
					CrossDestDir.MakeUpCartesianByZAxis();

					EtVector3 vToolDefinedDir = *pStruct->vDirection;
					EtVector3 vDirZ( vToolDefinedDir.x, 0.0f, vToolDefinedDir.z );
					EtVec3Normalize( &vDirZ, &vDirZ );
					EtVec3Normalize( &vToolDefinedDir, &vToolDefinedDir );
					float fDot = EtVec3Dot( &vToolDefinedDir, &vDirZ );
					float fRotX = EtAcos( fDot );

					CrossDestDir.RotatePitch( EtToDegree( -fRotX ) );
					vDirection = CrossDestDir.m_vZAxis;
				}
				else
				{
					// If the direction vector is not set, orient toward dest position.
					vDirection = vDestOffset - (*pStruct->vOffset);
					EtVec3Normalize( &vDirection, &vDirection );
				}

				EtVector3 vWorldDirection;
				EtVec3TransformNormal( &vWorldDirection, &vDirection, LocalCross );
				CrossResult.m_vZAxis = vWorldDirection;
				EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );

				CrossResult.MakeUpCartesianByZAxis();

				// Since direction toward DestPosition is already set, move destination along local Z axis by the distance value.
				float fDestLength = EtVec3Length( &vDestOffset );
				MatrixEx crossDestPosition = CrossResult;
				crossDestPosition.m_vPosition += CrossResult.m_vZAxis * fDestLength;

				crossDestPosition.m_vPosition.y = CDnWorld::GetInstance( hShooter->GetRoom() ).GetHeightWithProp( crossDestPosition.m_vPosition );

				pProjectile->SetTargetPosition( crossDestPosition.m_vPosition );
			}
			break;
	}

	if( pStruct->nWeaponTableID > 0 ) {
		if( pStruct->nProjectileIndex != -1 ) {
			DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr( (CMultiRoom*)g_pGameServerManager->GetRootRoom(), pStruct->nProjectileIndex );
			if( hWeapon ) *(CDnWeapon*)pProjectile = *hWeapon.GetPointer();
		}
		else {
			if (pProjectile == NULL)
			{
				_DANGER_POINT();
				return NULL;
			}

			pProjectile->CDnWeapon::Initialize( pStruct->nWeaponTableID, 0 );
			pProjectile->CreateObject();
		}
	}
	else if( hShooter->GetWeapon(1) ) {
		*(CDnWeapon*)pProjectile = *hShooter->GetWeapon(1);
	}
	else
	{
		SAFE_DELETE( pProjectile );
		return NULL;
	}

	pProjectile->SetWeaponType( (WeaponTypeEnum)( pProjectile->GetWeaponType() | WeaponTypeEnum::Projectile ) );
	pProjectile->Initialize( CrossResult, (CDnProjectile::OrbitTypeEnum)pStruct->nOrbitType, (CDnProjectile::DestroyOrbitTypeEnum)pStruct->nDestroyOrbitType,
		(CDnProjectile::TargetTypeEnum)pStruct->nTargetType);

	pProjectile->SetSpeed( pStruct->fSpeed * fVelocityMultiply );
	pProjectile->SetResistance( pStruct->fResistance );
	pProjectile->SetValidTime( pStruct->nValidTime );

	if( pProjectile->GetOrbitType() == OrbitTypeEnum::Homing ||
		pProjectile->GetOrbitType() == OrbitTypeEnum::TerrainHoming ||
		pProjectile->GetOrbitType() == OrbitTypeEnum::Projectile )
	{
		pProjectile->SetVelocityType( (VelocityTypeEnum)pStruct->VelocityType );
	}

	int nLength = pProjectile->GetWeaponLength();
	if( pStruct->bIncludeMainWeaponLength )
	{
		DnWeaponHandle hWeapon = hShooter->GetWeapon(0);
		if ( hWeapon )
			nLength += hWeapon->GetWeaponLength();
	}

	if( hShooter->IsProcessSkill() )
	{
		nLength += hShooter->GetProcessSkill()->GetIncreaseRange();
	}

	if( hShooter->IsEnabledToggleSkill() )
	{
		nLength += hShooter->GetEnabledToggleSkill()->GetIncreaseRange();
	}

	pProjectile->SetWeaponLength( nLength );

	pProjectile->SetPierce( ( pStruct->bPierce == TRUE ) ? true : false );
	pProjectile->SetMaxHitCount( pStruct->nMaxHitCount );
	pProjectile->SetProjectileOrbitRotateZ( pStruct->fProjectileOrbitRotateZ );
	pProjectile->SetTraceHitTarget( (pStruct->bTraceHitTarget == TRUE) ? true : false, 
									(pStruct->bTraceHitActorHittable == TRUE) ? true : false );

#if defined(PRE_FIX_52329)
	pProjectile->SetIgnoreHitType(pStruct->nIgnoreHitType);
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	pProjectile->SetHitActionVectorInit(pStruct->bHitActionVectorInit == TRUE ? true : false);
#endif // PRE_ADD_55295

	pProjectile->PostInitialize();

	return pProjectile;
}


// Projectile object created upon receiving packet from client.
// Packets arrive from each client's LocalPlayer.
CDnProjectile* CDnProjectile::CreatePlayerProjectileFromClientPacket( DnActorHandle hShooter, const BYTE* pPacket )
{
	if( false == hShooter->IsPlayerActor() )
		return NULL;

	CPacketCompressStream Stream( const_cast<BYTE*>(pPacket), 128 );
	CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hShooter.GetPointer());
	if( pPlayerActor->IsInvalidPlayerChecker() ) return NULL;

	bool bActorAttachWeapon;
	int nWeaponTableID, nWeaponLength = 0;
	DWORD dwUniqueID,dwGap;
	float fSpeed = 0.f;
	float fResistance = 0.f;
	int nValidTime = 0;
	int VelocityType = 0;
	EtVector3 vTargetPos;
	DWORD dwTargetUniqueID;
	CDnProjectile::OrbitTypeEnum OrbitType;
	CDnProjectile::DestroyOrbitTypeEnum DestroyType;
	CDnProjectile::TargetTypeEnum TargetType;
	MatrixEx Cross;
	EtVector3 vDirection( 0.0f, 0.0f, 0.0f );
	int nActionIndex;
	int nMaxHitCount = 0;
	int nSignalIndex = -1;
	float fProjectileOrbitRotateZ = 0.0f;
	ProjectileStruct* pProjectileStruct = NULL;

	Stream.Read( &dwGap, sizeof(DWORD) );
	Stream.Read( &bActorAttachWeapon, sizeof(bool) );
	Stream.Read( &dwUniqueID, sizeof(DWORD) );
	if( !bActorAttachWeapon ) 
	{
		Stream.Read( &nWeaponTableID, sizeof(int) );
	}

	Stream.Read( &Cross.m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Read( &Cross.m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	Stream.Read( &Cross.m_vYAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
	Stream.Read( &Cross.m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

#ifdef PRE_MOD_PROJECTILE_HACK
	char cShooterType;
	DWORD dwParentShooterUniqueID = -1;
	DWORD dwShooterUniqueID = -1;
	INT64 nShooterSerialID = -1;
	int nShooterActionIndex;
	int nShooterSignalIndex;
	bool bSendSerialID;
	DnWeaponHandle hParentProjectileWeapon;

	Stream.Read( &cShooterType, sizeof(char) );
	Stream.Read( &dwParentShooterUniqueID, sizeof(DWORD) );
	switch( cShooterType ) {
		case 0:
			dwShooterUniqueID = dwParentShooterUniqueID;
			break;
		case 1:
			{
				Stream.Read( &bSendSerialID, sizeof(bool) );
				if( bSendSerialID )
					Stream.Read( &nShooterSerialID, sizeof(INT64) );
				else Stream.Read( &dwShooterUniqueID, sizeof(DWORD) );
			}
			break;
	}

	Stream.Read( &nShooterActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Read( &nShooterSignalIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	ProjectileStruct *pShooterStruct = NULL;
	nWeaponLength = 0;
	switch( cShooterType ) {
		case 0:
			{
				DnActorHandle hHandle = CDnActor::FindActorFromUniqueID( pPlayerActor->GetRoom(), dwShooterUniqueID );
				if( hHandle ) 
				{
					if( hHandle->GetCurrentActionIndex() == nShooterActionIndex )
					{
						bool bCheckStandAction = CDnChangeStandActionBlow::CheckUsableAction( pPlayerActor->GetActorHandle(), true, pPlayerActor->GetCurrentAction() );
						if( bCheckStandAction == false )
						{
							pPlayerActor->ReportInvalidAction();
							return NULL;
						}

						CEtActionSignal *pSignal = hHandle->GetSignal( nShooterActionIndex, nShooterSignalIndex );
						if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) 
						{
							pShooterStruct = static_cast<ProjectileStruct*>( pSignal->GetData() );
						}
						else if( hHandle->GetCustomActionIndex() > 0 )
						{
							CEtActionSignal *pCustomActionSignal = hHandle->GetSignal( hHandle->GetCustomActionIndex(), nShooterSignalIndex );
							if( pCustomActionSignal && pCustomActionSignal->GetSignalIndex() == STE_Projectile ) 
							{
								pShooterStruct = static_cast<ProjectileStruct*>( pCustomActionSignal->GetData() );
							}
						}
					}
					else
					{
						// Action index differs between client and server. Treat as hack.
						return NULL;
					}
				}
			}
			break;
		case 1:
			{
				DnWeaponHandle hHandle;
				if( bSendSerialID ) {
					for( DWORD i=0; i<2; i++ ) { // Should implement FindItemFromSerialID, but not needed for now so only search in EquipWeapon.
						DnWeaponHandle hWeapon = pPlayerActor->GetWeapon(i);
						if( hWeapon && hWeapon->GetSerialID() == nShooterSerialID ) {
							hHandle = hWeapon;
							break;
						}
						hWeapon = pPlayerActor->GetCashWeapon(i);
						if( hWeapon && hWeapon->GetSerialID() == nShooterSerialID ) {
							hHandle = hWeapon;
							break;
						}
					}
				}
				else {
					hHandle = CDnWeapon::FindWeaponFromUniqueIDAndShooterUniqueID( pPlayerActor->GetRoom(), dwShooterUniqueID, dwParentShooterUniqueID );
					if( hHandle ) {
						hParentProjectileWeapon = hHandle;
//						nWeaponLength += hHandle->GetWeaponLength();
					}
				}
				if( hHandle ) {
					CEtActionSignal *pSignal = hHandle->GetSignal( nShooterActionIndex, nShooterSignalIndex );
					if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) {
						pShooterStruct = static_cast<ProjectileStruct*>( pSignal->GetData() );
					}					
				}
			}
			break;
	}
	if( !pShooterStruct ) return NULL;
	pProjectileStruct = pShooterStruct;
	OrbitType = (CDnProjectile::OrbitTypeEnum)pShooterStruct->nOrbitType;
	DestroyType = (CDnProjectile::DestroyOrbitTypeEnum)pShooterStruct->nDestroyOrbitType;
	TargetType = (CDnProjectile::TargetTypeEnum)pShooterStruct->nTargetType;
	nValidTime = pShooterStruct->nValidTime;
	fProjectileOrbitRotateZ = pShooterStruct->fProjectileOrbitRotateZ;
#ifdef PRE_ADD_PROJECTILE_RANDOM_WEAPON
	// Check random projectile
	if( pShooterStruct->RandomWeaponParam && strlen( pShooterStruct->RandomWeaponParam ) > 0 )
	{
		std::vector<int> vecWeaponID;
		vecWeaponID.push_back( pShooterStruct->nWeaponTableID );
		std::vector<std::string> tokens;
		TokenizeA( pShooterStruct->RandomWeaponParam, tokens, "/" );
		if( tokens.size() > 1 )
		{
			for( int i=1; i<static_cast<int>( tokens.size() ); i++ )
			{
				std::vector<std::string> tokensparam;
				TokenizeA( tokens[i].c_str(), tokensparam, ";" );
				if( tokensparam.size() > 0 )
				{
					vecWeaponID.push_back( atoi( tokensparam[0].c_str() ) );
				}
				tokensparam.clear();
			}
		}

		bool bUsableWeaponTableID = false;
		for( int i=0; i<static_cast<int>( vecWeaponID.size() ); i++ )
		{
			if( vecWeaponID[i] == nWeaponTableID )
			{
				bUsableWeaponTableID = true;
				break;
			}
		}

		tokens.clear();
		vecWeaponID.clear();

		if( !bUsableWeaponTableID )	// Unusable WeaponID!
			return NULL;
	}
#endif // PRE_ADD_PROJECTILE_RANDOM_WEAPON
#else

	Stream.Read( &OrbitType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Read( &DestroyType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Read( &TargetType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Read( &nValidTime, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Read( &nWeaponLength, sizeof(int) );
	Stream.Read( &fProjectileOrbitRotateZ, sizeof(float) );
#endif

	int nTargetPartsIndex = -1;
	int nTargetBoneIndex = -1;

	Stream.Read(&nTargetPartsIndex, sizeof(int));
	Stream.Read(&nTargetBoneIndex, sizeof(int));

	switch( TargetType )
	{
		case CDnProjectile::CrossHair:
		case CDnProjectile::Direction:
			break;

		case CDnProjectile::TargetPosition:
		case CDnProjectile::Target:
		case CDnProjectile::DestPosition:
		case CDnProjectile::Shooter:
			Stream.Read( &vTargetPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
			Stream.Read( &dwTargetUniqueID, sizeof(DWORD), CPacketCompressStream::NOCOMPRESS );
			break;
	}

	Stream.Read( &fSpeed, sizeof(float) );

#ifdef PRE_MOD_PROJECTILE_HACK
	bool bPierce = ( pShooterStruct->bPierce == TRUE );
	nMaxHitCount = pShooterStruct->nMaxHitCount;
	bool bTraceHitTarget = ( pShooterStruct->bTraceHitTarget == TRUE );
	bool bTraceHitActorHittable = (pShooterStruct->bTraceHitActorHittable == TRUE);

	switch( OrbitType )  {
		case CDnProjectile::Linear:
		case CDnProjectile::TerrainLinear:
			break;

		case CDnProjectile::Projectile:
			VelocityType = pShooterStruct->VelocityType;
			break;

		case CDnProjectile::Homing:
		case CDnProjectile::TerrainHoming:
			VelocityType = pShooterStruct->VelocityType;
			break;
		case CDnProjectile::Acceleration:
			fResistance = pShooterStruct->fResistance;
			break;
	}
	Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

	if( false == pPlayerActor->IsIgnoreSkillCoolTime() )
	{
		if( nActionIndex != -1 && nActionIndex != pPlayerActor->GetCurrentActionIndex() ) 
		{ 
			// Using a hack.
			//break;
			return NULL;
		}
	}
	nSignalIndex = nShooterSignalIndex;

#else
	bool bPierce = false;
	Stream.Read( &bPierce, sizeof(bool) );

	if( bPierce )
		Stream.Read( &nMaxHitCount, sizeof(int) );

	bool bTraceHitTarget = false;
	bool bTraceHitActorHittable = false;
	Stream.Read( &bTraceHitTarget, sizeof(bool) );
	Stream.Read( &bTraceHitActorHittable, sizeof(bool) );

	switch( OrbitType ) 
	{
		case CDnProjectile::Linear:
		case CDnProjectile::TerrainLinear:
			break;

		case CDnProjectile::Projectile:
			Stream.Read( &VelocityType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
			break;

		case CDnProjectile::Homing:
		case CDnProjectile::TerrainHoming:
			Stream.Read( &VelocityType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
			break;
		case CDnProjectile::Acceleration:
			Stream.Read( &fResistance, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );
			break;
	}

	Stream.Read( &nActionIndex, sizeof(int) );

	if( false == pPlayerActor->IsIgnoreSkillCoolTime() )
	{
		if( nActionIndex != -1 && nActionIndex != pPlayerActor->GetCurrentActionIndex() ) 
		{ 
			return NULL;
		}
	}

	// Note Hanki: Need an index to look up the signal struct for chain attack state effect.
	Stream.Read( &nSignalIndex, sizeof(int) );

	// Note Hanki: Find projectile signal struct.
	// ActionIndex is valid only when it is a skill.
	if( -1 != nSignalIndex && -1 != nActionIndex )
	{
		CEtActionSignal *pSignal = pPlayerActor->GetSignal( nActionIndex, nSignalIndex );
		if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) {
			pProjectileStruct = static_cast<ProjectileStruct*>( pSignal->GetData() );
		}
	}
#endif

#ifdef PRE_MOD_PROJECTILE_HACK
#else
	// From the first projectile fired in a projectile-firing skill such as Bow Master's Revolutionary Ballista
	// Info for transferring stats...
	DWORD dwParentID = 0;
	Stream.Read( &dwParentID, sizeof(DWORD) );

	DWORD dwParentShooterUniqueID = 0;
	Stream.Read( &dwParentShooterUniqueID, sizeof(DWORD) );
#endif

#if defined(PRE_FIX_52329)
	int nIgnoreHitType = 0;
	Stream.Read(&nIgnoreHitType, sizeof(nIgnoreHitType));
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	bool bHitActionVectorInit = false;
	Stream.Read(&bHitActionVectorInit, sizeof(bHitActionVectorInit));
#endif // PRE_ADD_55295

	pPlayerActor->GetPlayerSpeedHackChecker()->OnSyncDatumGap( dwGap );

	CDnProjectile* pParentProjectile = NULL;
	DnSkillHandle hParentSkill;
	bool bFromParentProjectile = false;
#ifndef PRE_MOD_PROJECTILE_HACK
	DnWeaponHandle hParentProjectileWeapon = CDnWeapon::FindWeaponFromUniqueIDAndShooterUniqueID( pPlayerActor->GetRoom(), 
																								 dwParentID, dwParentShooterUniqueID );
#endif
	if( hParentProjectileWeapon )
	{
		// When firing a projectile from a projectile, the packet arrives from the client like this.
		if( hParentProjectileWeapon->GetWeaponType() == CDnWeapon::Projectile )
		{
			pParentProjectile = static_cast<CDnProjectile*>(hParentProjectileWeapon.GetPointer());
			hParentSkill = pParentProjectile->GetParentSkill();
			if( hParentSkill )		// Select only skills that fire a projectile from a projectile.
			{
				pPlayerActor->AddReservedProjectileCount();
				bFromParentProjectile = true;
			}
		}
	}

	// Exclude cases of firing a projectile from a projectile from the check.
	// Reason: even if a hack sends projectile spawn position to server, the dynamically created parent projectile ID is also
	// If not sent, it gets caught by the routine above and is not recognized as a projectile firing a projectile.
	// For now, block only normal attacks.
	if( (false == bFromParentProjectile) &&
		(-1 == nSignalIndex || -1 == nActionIndex) )
	{
		// Check only on the X, Z plane. Add height checking later if needed.
		EtVector2 vShooterXZPos( hShooter->GetPosition()->x, hShooter->GetPosition()->z );
		EtVector2 vProjectileShootPosXZ( Cross.m_vPosition.x, Cross.m_vPosition.z );
		EtVector2 vDist = vProjectileShootPosXZ - vShooterXZPos;
		float fDistanceSQ = EtVec2LengthSq( &vDist );
		//#37327 For Demolition projectiles, Offset value can be set over 100.
		//Modify to check against a generous max value of 200 for now (200*200 = 40000)
		if( 40000.0f < fDistanceSQ )
		{
			// Judged as a hack that sent an arbitrarily manipulated projectile start position.
#ifndef _FINAL_BUILD
			OutputDebug( "Projectile spawn position distance check failed; suspected hack.\n" );
#endif // #ifndef _FINAL_BUILD
			return NULL;
		}
	}

	bool bValidProjectile = true;
	//For brand projectiles, more projectiles than the projectile signal count are fired.
	//Modified to check projectiles only when not a brand projectile.
	if (pShooterStruct->nTargetStateIndex == 0)
		bValidProjectile = pPlayerActor->UseAndCheckAvailProjectileCount();

	if( false == bValidProjectile )
		return NULL;

	// Check if the projectile matches the current timing
	// Exclude the special case of firing a projectile from a projectile.
	if( false == bFromParentProjectile )
	{
		bValidProjectile = true;

		//For brand projectiles, more projectiles than the projectile signal count are fired.
		//Modified to check projectiles only when not a brand projectile.
		if (pShooterStruct->nTargetStateIndex == 0)
			bValidProjectile = pPlayerActor->CheckProjectileSignalFrameTerm();
		
		if( false == bValidProjectile )
		{
#ifndef _FINAL_BUILD
			OutputDebug( "CDnPlayerActor::CheckProjectileSignalFrameTerm(): projectile signal arrived outside valid frame window; suspected hack.\n" );
#endif // #ifndef _FINAL_BUILD
			return NULL;
		}
	}

	CDnProjectile *pProjectile = new CDnProjectile( pPlayerActor->GetRoom(), hShooter );
	pProjectile->SetUniqueID( dwUniqueID );
	pProjectile->SetPierce( bPierce );
	pProjectile->SetMaxHitCount( nMaxHitCount );
	pProjectile->SetTargetPartsIndex(nTargetPartsIndex, nTargetBoneIndex);

	if( bActorAttachWeapon ) 
	{
		CDnWeapon *pWeapon = pPlayerActor->GetWeapon(1);

		if( pWeapon )
		{
			*(CDnWeapon*)pProjectile = *pWeapon;
		}
	}
	else 
	{
		if( pProjectileStruct && pProjectileStruct->nProjectileIndex != -1 )
		{
			DnWeaponHandle hLocalParentProjectileWeapon = CDnWeapon::GetSmartPtr( (CMultiRoom*)g_pGameServerManager->GetRootRoom(), 
				pProjectileStruct->nProjectileIndex );
			if( hLocalParentProjectileWeapon ) *(CDnWeapon*)pProjectile = *hLocalParentProjectileWeapon.GetPointer();
		}
		else
		{
			// Check for cases in CN where weapon ID is swapped on normal attack before sending. 
			// Exclude the special case of firing a projectile from a projectile.
			if( false == bFromParentProjectile )
			{
				if( false == pPlayerActor->CheckAndEraseWeaponIDUsingProjectileSignal( nWeaponTableID ) )
				{
					// TODO: It's a hack! Wish we could disconnect them..
#ifndef _FINAL_BUILD
					OutputDebug( "CDnProjectile::CreatePlayerProjectileFromClientPacket(): weapon table index not used by current action projectile signals; suspected hack.\n" );
#endif // #ifndef _FINAL_BUILD
					return NULL;
				}
			}
			pProjectile->CDnWeapon::Initialize( nWeaponTableID, 0 );
			pProjectile->CDnWeapon::CreateObject();
		}
	}

#ifdef PRE_MOD_PROJECTILE_HACK
	nWeaponLength = pProjectile->GetWeaponLength();
	if( pShooterStruct->bIncludeMainWeaponLength && hShooter->GetWeapon(0) ) nWeaponLength += hShooter->GetWeapon(0)->GetWeaponLength();
	if( hShooter->IsProcessSkill() ) nWeaponLength += hShooter->GetProcessSkill()->GetIncreaseRange();
	if( hShooter->IsEnabledToggleSkill() ) nWeaponLength += hShooter->GetEnabledToggleSkill()->GetIncreaseRange();
#endif

	pProjectile->SetWeaponLength( nWeaponLength );
	pProjectile->SetWeaponType( (CDnWeapon::WeaponTypeEnum)( pProjectile->GetWeaponType() | CDnWeapon::Projectile ) );

	pProjectile->SetSpeed( fSpeed );
	switch( OrbitType ) 
	{
		case CDnProjectile::Acceleration:
			pProjectile->SetResistance( fResistance );
			break;
	}

	switch( TargetType )
	{
		case CDnProjectile::TargetPosition:
		case CDnProjectile::Target:
		case CDnProjectile::DestPosition:
			pProjectile->SetTargetPosition( vTargetPos );
			pProjectile->SetTargetActor( CDnActor::FindActorFromUniqueID( pPlayerActor->GetRoom(), dwTargetUniqueID ), false );

			//////////////////////////////////////////////////////////////////////////
			//For a projectile flying to a target point, update the direction vector from current projectile position to target point.
			//Modify to work only on brand projectiles for now.
			if (pShooterStruct->nTargetStateIndex != 0)
			{
				Cross.m_vZAxis = vTargetPos - Cross.GetPosition();
				EtVec3Normalize( &Cross.m_vZAxis, &Cross.m_vZAxis );
			}
			//////////////////////////////////////////////////////////////////////////
			break;
	}

	pProjectile->Initialize( Cross, OrbitType, DestroyType, TargetType );
	pProjectile->SetValidTime( nValidTime );
	pProjectile->SetVelocityType( (CDnProjectile::VelocityTypeEnum)VelocityType );
	pProjectile->SetProjectileOrbitRotateZ( fProjectileOrbitRotateZ );
	pProjectile->SetTraceHitTarget( bTraceHitTarget, bTraceHitActorHittable );

	// Note: nActionIndex is set and sent in packet only when client uses a skill.
	// If pProjectileStruct is NULL, it is because nSignalIndex does not match between client and server.
	// That means the action file resources differ between server and client.
	if( pProjectileStruct && -1 != nSignalIndex && -1 != nActionIndex )
		pPlayerActor->OnProjectile( pProjectile, pProjectileStruct, Cross, nSignalIndex );

	if( bFromParentProjectile )
	{
		pProjectile->SetShooterStateSnapshot( pParentProjectile->GetShooterStateSnapshot() );
		pProjectile->SetParentSkill( hParentSkill );

#ifdef PRE_ADD_PROJECTILE_SE_INFO
		pProjectile->SetShooterStateBlow( pParentProjectile->GetShooterStateBlow() );
#endif

		//When firing a projectile from a projectile, register the state effects that were on the parent projectile.
		pProjectile->ApplyParentProjectile(pParentProjectile);

		//Projectile -> Projectile: when firing a projectile from a projectile, get info from parent projectile to set whether it is from a skill.
		pProjectile->FromSkill(pParentProjectile->IsFromSkill());

#if defined(PRE_FIX_65287)
		pProjectile->SetShooterFinalDamageRate(pParentProjectile->GetShooterFinalDamageRate());
#endif // PRE_FIX_65287
	}
	else
	{
		bool bApplyStateBlowInfo = pProjectileStruct->bApplyStateBlowInfo == TRUE;

#if defined( PRE_FIX_PROJECTILE_PREFIX_APPLY_POINT )				
		if( hShooter && hShooter->IsPlayerActor() )
		{
			if( hShooter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) == false )
			{
				hShooter->ProcessPrefixOffenceSkill_New();
				if( hShooter->IsApplyPrefixOffenceSkill() )
					bApplyStateBlowInfo = true;
			}
		}
#endif

		// Player needs stats set whether it is a skill or a normal attack. (#17829)
		boost::shared_ptr<CDnState> pActorStateSnapshot( new CDnState ); 		
		*pActorStateSnapshot = *static_cast<CDnState*>( pPlayerActor );
		pProjectile->SetShooterStateSnapshot( pActorStateSnapshot );

#ifdef PRE_ADD_PROJECTILE_SE_INFO
		if( bApplyStateBlowInfo == true )
		{
			boost::shared_ptr<CDnStateBlow> pActorStateBlow = boost::shared_ptr<CDnStateBlow>(new CDnStateBlow(pPlayerActor->GetMySmartPtr()));
			pActorStateBlow->MakeCloneStateBlowList( pPlayerActor->GetStateBlow()->GetStateBlowList() );
			pProjectile->SetShooterStateBlow( pActorStateBlow );
		}
#endif

#if defined(PRE_FIX_65287)

		float fFinalDamageRate = 0.0f;
		if (pPlayerActor && pPlayerActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
		{
			DNVector(DnBlowHandle) vlhBlows;
			pPlayerActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				fFinalDamageRate += vlhBlows[i]->GetFloatValue();
			}
		}

		pProjectile->SetShooterFinalDamageRate(fFinalDamageRate);
#endif // PRE_FIX_65287

		// #30571 Call only for projectiles fired from the currently executing skill action.
		// else In the above case where a projectile fires another projectile, later when a projectile fires a projectile...
		// Another packet arrives from the client when firing, so distinguish and call it here.
		hShooter->OnSkillProjectile( pProjectile );
	}

#if defined(PRE_FIX_52329)
	pProjectile->SetIgnoreHitType(nIgnoreHitType);
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	pProjectile->SetHitActionVectorInit(bHitActionVectorInit);
#endif // PRE_FIX_52329

	pProjectile->PostInitialize();


	return pProjectile;
}


// Create a packet from the created projectile object.
// Must be an identical routine to client (note: this file is split between client and server).
CPacketCompressStream* CDnProjectile::GetPacketStream( void )
{
	if( !m_hShooter )
		return NULL;

	// If this is the first request and no object exists, create a packet stream.
	// Note that changes to the projectile object's contents after creation will not be reflected.
	// Currently, created projectiles are never modified after Create.
	if( NULL == m_pPacketStream )
	{
		m_pPacketBuffer = new char[ PROJECTILE_PACKET_BUFFER_SIZE ];
		m_pPacketStream = new CPacketCompressStream( m_pPacketBuffer, PROJECTILE_PACKET_BUFFER_SIZE*sizeof(char) );

		bool bActorAttachWeapon = false;
		int VelocityType = 0;
		int nValidTime = 0;
		float fValue;
		if( m_hShooter->GetWeapon(1) && 
			this->GetClassID() == m_hShooter->GetWeapon(1)->GetClassID() ) 
			bActorAttachWeapon = true;

		DWORD dwGap = 0;

		m_pPacketStream->Write( &dwGap, sizeof(DWORD) );		// No need to process since it is sent from the server.
		m_pPacketStream->Write( &bActorAttachWeapon, sizeof(bool) );
		int nValue = 0;
		DWORD dwValue = this->GetUniqueID();
		m_pPacketStream->Write( &dwValue, sizeof(DWORD) );

		if( bActorAttachWeapon == false ) {
			nValue = this->GetClassID();
			m_pPacketStream->Write( &nValue, sizeof(int) );
		}

		m_pPacketStream->Write( &this->GetMatEx()->m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
		m_pPacketStream->Write( &this->GetMatEx()->m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
		m_pPacketStream->Write( &this->GetMatEx()->m_vYAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
		m_pPacketStream->Write( &this->GetMatEx()->m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

#ifdef PRE_MOD_PROJECTILE_HACK
		DWORD dwShooterUniqueID = m_hShooter->GetUniqueID();
		m_pPacketStream->Write( &m_cShooterType, sizeof(char) );
		m_pPacketStream->Write( &dwShooterUniqueID, sizeof(DWORD) );
		switch( m_cShooterType ) {
			case 0: 
				break;
			case 1:
				{
					bool bSendSerialID = ( m_nShooterSerialID == -1 ) ? false : true;
					m_pPacketStream->Write( &bSendSerialID, sizeof(bool) );
					if( bSendSerialID )
						m_pPacketStream->Write( &m_nShooterSerialID, sizeof(INT64) );
					else m_pPacketStream->Write( &m_dwShooterUniqueID, sizeof(DWORD) );
				}
				break;
		}
		m_pPacketStream->Write( &m_nShooterActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
		m_pPacketStream->Write( &m_nShooterSignalIndex, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
#else
		nValue = (int)this->GetOrbitType();
		m_pPacketStream->Write( &nValue, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
		nValue = (int)this->GetDestroyOrbitType();
		m_pPacketStream->Write( &nValue, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
		nValue = (int)this->GetTargetType();
		m_pPacketStream->Write( &nValue, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

		nValidTime = this->GetValidTime();
		m_pPacketStream->Write( &nValidTime, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

		nValue = this->GetWeaponLength();
		m_pPacketStream->Write( &nValue, sizeof(int) );

		fValue = this->GetProjectileOrbitRotateZ();
		m_pPacketStream->Write( &fValue, sizeof(float) );
#endif

		m_pPacketStream->Write(&m_nTargetPartsIndex, sizeof(int));
		m_pPacketStream->Write(&m_nTargetPartsBoneIndex, sizeof(int));

		switch( this->GetTargetType() ) {
		case CDnProjectile::CrossHair:
		case CDnProjectile::Direction:
			break;
		case CDnProjectile::TargetPosition:
		case CDnProjectile::Target:
		case CDnProjectile::DestPosition:
		case CDnProjectile::Shooter:
			{
				m_pPacketStream->Write( this->GetTargetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				DWORD dwUniqueID = ( this->GetTargetActor() ) ? this->GetTargetActor()->GetUniqueID() : -1;
				m_pPacketStream->Write( &dwUniqueID, sizeof(DWORD) );
			}
			break;
		}

		fValue = this->GetSpeed();
		m_pPacketStream->Write( &fValue, sizeof(float) );

#ifdef PRE_MOD_PROJECTILE_HACK
		int nActionIndex = -1;
		if( m_hShooter->GetProcessSkill() ) 
			nActionIndex = m_hShooter->GetCurrentActionIndex();

		m_pPacketStream->Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
#else
		bool bPierce = this->GetPierce();
		m_pPacketStream->Write( &bPierce, sizeof(bool) );

		if( bPierce )
		{
			int nMaxHitCount = this->GetMaxHitCount();
			m_pPacketStream->Write( &nMaxHitCount, sizeof(int) );
		}

		bool bTraceHitTarget = this->IsTraceHitTarget();
		bool bTraceHitActorHittable = this->IsTraceHitActorHittable();
		m_pPacketStream->Write( &bTraceHitTarget, sizeof(bool) );
		m_pPacketStream->Write( &bTraceHitActorHittable, sizeof(bool) );

		VelocityType = (int)this->GetVelocityType();

		switch( this->GetOrbitType() ) {
		case CDnProjectile::Linear:
		case CDnProjectile::TerrainLinear:
			break;

		case CDnProjectile::Projectile:
			m_pPacketStream->Write( &VelocityType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
			break;

		case CDnProjectile::Homing:
		case CDnProjectile::TerrainHoming:
			m_pPacketStream->Write( &VelocityType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
			break;

		case CDnProjectile::Acceleration:
			fValue = this->GetResistance();
			m_pPacketStream->Write( &fValue, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1.f );
			break;
		}

		// The following below is not used for monsters.

		int nActionIndex = -1;
		if( m_hShooter->GetProcessSkill() ) 
			nActionIndex = m_hShooter->GetCurrentActionIndex();

		m_pPacketStream->Write( &nActionIndex, sizeof(int) );

		// Note : Need an index to look up the signal struct for chain attack state effect.
		m_pPacketStream->Write( &m_iSignalArrayIndex, sizeof(int) );

		// From the first projectile fired in a projectile-firing skill such as Bow Master's Revolutionary Ballista
		// Info for transferring stats... 
		// Sent from client to server; server never sends this to client. 
		// Written to match the format.
		DWORD dwParentID = UINT_MAX;//this->GetParentProjectileID();
		DWORD dwParentShooterUniqueID = m_hShooter->GetUniqueID();
		m_pPacketStream->Write( &dwParentID, sizeof(DWORD) );
		m_pPacketStream->Write( &dwParentShooterUniqueID, sizeof(DWORD) );
#endif

#if defined(PRE_FIX_52329)
		m_pPacketStream->Write( &m_nIgnoreHitType, sizeof(m_nIgnoreHitType));
#endif // PRE_FIX_52329
	}

	return m_pPacketStream;
}


void CDnProjectile::SetShooterType( DnActorHandle hActor, int nActionIndex, int nSignalIndex )
{
	m_cShooterType = 0;
	m_dwShooterUniqueID = hActor->GetUniqueID();
	m_nShooterActionIndex = nActionIndex;
	m_nShooterSignalIndex = nSignalIndex;
}

void CDnProjectile::SetShooterType( DnWeaponHandle hWeapon, int nActionIndex, int nSignalIndex )
{
	m_cShooterType = 1;

	m_dwShooterUniqueID = -1;
	m_nShooterSerialID = -1;
	if( hWeapon->GetSerialID() == -1 )
		m_dwShooterUniqueID = hWeapon->GetUniqueID();
	else m_nShooterSerialID = hWeapon->GetSerialID();

	m_nShooterActionIndex = nActionIndex;
	m_nShooterSignalIndex = nSignalIndex;
}


void CDnProjectile::ApplyParentProjectile(CDnProjectile* pParentProjectile)
{
	if (pParentProjectile == NULL)
		return;

	const std::vector<CDnSkill::StateEffectStruct>& stateEffectList = pParentProjectile->GetStateEffectList();
	if (stateEffectList.empty())
		return;

	int nStateEffectCount = (int)stateEffectList.size();
	for (int i = 0; i < nStateEffectCount; ++i)
	{
		CDnSkill::StateEffectStruct& stateEffect = const_cast<CDnSkill::StateEffectStruct&>(stateEffectList.at(i));
		if( stateEffect.ApplyType == CDnSkill::ApplySelf ) continue;

		this->AddStateEffect( stateEffect );
	}
}

void CDnProjectile::SetTargetActor( DnActorHandle hActor, bool bUpdateTargetPartsIndex/* = true*/ )
{ 
	m_hTargetActor = hActor;

	if (bUpdateTargetPartsIndex)
		UpdateTargetPartsIndex();
}

void CDnProjectile::UpdateTargetPartsIndex()
{
	int nSelectPartsIndex = -1;
	int nSelectBoneIndex = -1;

	//If it is a parts monster, the corresponding part must be selected.
	if (m_hTargetActor && m_hTargetActor->IsMonsterActor())
	{
		if( m_hTargetActor->IsPartsMonsterActor() ) {
			CDnPartsMonsterActor* pPartsMonsterActor = static_cast<CDnPartsMonsterActor*>(m_hTargetActor.GetPointer());
			int nCount = pPartsMonsterActor->GetPartsSize();

			//Store selectable Parts Index..
			struct SelectPartsInfo
			{
				int PartsIndex;
				int BoneCount;
			};

			std::vector<SelectPartsInfo> indexList;

			for (int i = 0; i < nCount; ++i)
			{
				MonsterParts* pMonsterParts = pPartsMonsterActor->GetPartsByIndex(i);
				if (pMonsterParts == NULL)
					continue;

				const MonsterParts::_Info& boneInfo = pMonsterParts->GetPartsInfo();
				if (boneInfo.PartsState != MonsterParts::ePartsState::eNormal)
					continue;

				//Stores valid parts indices and the bone count of these parts.
				SelectPartsInfo _tempInfo;
				_tempInfo.PartsIndex = i;
				_tempInfo.BoneCount = (int)boneInfo.vParts.size();

				indexList.push_back(_tempInfo);
			}

			int nListCount = (int)indexList.size();

			if (nListCount > 0)
			{
				//Select one from the contained parts..
				int nIndex = rand() % nListCount;
				SelectPartsInfo &tempPartsInfo = indexList[nIndex];

				nSelectPartsIndex = tempPartsInfo.PartsIndex;
				
				//Select one bone among the selected parts.
				nSelectBoneIndex = rand() % tempPartsInfo.BoneCount;
			}
		}
	}

	SetTargetPartsIndex(nSelectPartsIndex, nSelectBoneIndex);
}

bool CDnProjectile::SetTraceActor( DnActorHandle hActor, bool bValidDamageGoOn )
{
	bool bValidDamageGoOnResult = bValidDamageGoOn;

	// For homing projectiles, enable valid damage since damage can still be dealt in hit state. #22666
	// Needs to be set only once during shoot action.
	if( m_nActionIndex == m_iShootActionIndex && 
		m_bTraceHitTarget && hActor &&
		!m_hTraceActor )
	{
		bValidDamageGoOn = true;
		m_hTraceActor = hActor;

		// Once it becomes a homing projectile, it does not disappear regardless of time until the hit action ends.
		m_ValidType = (ValidTypeEnum)( m_ValidType & ~Time );
	}

	return bValidDamageGoOnResult;
}

void CDnProjectile::GetChainAttackInfo(const CDnSkill::StateEffectStruct& stateEffectStruct, float& fRange, int& nMaxCount)
{
	switch(stateEffectStruct.nID)
	{
	case STATE_BLOW::BLOW_060:
		{
			std::string str = stateEffectStruct.szValue;
			std::vector<std::string> tokens;
			std::string delimiters = ";";

			//1. Category
			TokenizeA(str, tokens, delimiters);

			if (tokens.size() != 3)
				fRange = 1000.0f;
			else
			{
				fRange = (float)atoi(tokens[0].c_str());
				nMaxCount = atoi(tokens[1].c_str());
			}
		}
		break;
	case STATE_BLOW::BLOW_208:
		{
			std::string str = stateEffectStruct.szValue;//"MaxHitCount;Range(cm);Ratio";
			std::vector<std::string> tokens;
			std::string delimiters = ";";

			//1. Category
			TokenizeA(str, tokens, delimiters);

			if (tokens.size() != 3)
				fRange = 1000.0f;
			else
			{
				nMaxCount = atoi(tokens[0].c_str());
				fRange = (float)atoi(tokens[1].c_str());
			}
		}
		break;
	}
}

DnActorHandle CDnProjectile::FindNextChainActor(int iRootAttackerTeam, DnActorHandle hActor, DnActorHandle hPrevActor, float fRange)
{
	DNVector(DnActorHandle) vlActorsInRange;
	CDnActor::ScanActor( hActor->GetRoom(), *hActor->GetPosition(), fRange, vlActorsInRange );

	float fShortestDistanceSQ = FLT_MAX;
	DnActorHandle hActorToAttack;
	DWORD dwNumActors = (DWORD)vlActorsInRange.size();
	for( DWORD dwActor = 0; dwActor < dwNumActors; ++dwActor )
	{
		DnActorHandle hTargetActor = vlActorsInRange.at( dwActor );
		if (!hTargetActor)
			continue;

		if( false == (hTargetActor->IsShow() && hTargetActor->IsProcess()) )
			continue;

		//Skip dead ones...
		if (hTargetActor->IsDie())
			continue;

		// Do not pass the state effect back to the actor that just passed it to me.
		if( hPrevActor != hTargetActor )
		{
			// #30643 Everyone except me is an enemy
			if( iRootAttackerTeam != hTargetActor->GetTeam() &&
				hTargetActor != hActor )
			{
				EtVector3 vDistance = (*hActor->GetPosition()) - (*hTargetActor->GetPosition());

				float fLengthSQ = EtVec3LengthSq( &vDistance );
				if( fLengthSQ < fShortestDistanceSQ )
				{
					fShortestDistanceSQ = fLengthSQ;
					hActorToAttack = hTargetActor;
				}
			}
		}
	}

	return hActorToAttack;
}

void CDnProjectile::CreateChainAttackProjectile(DnActorHandle hRootAttacker, DnActorHandle hActor, DnActorHandle hActorToAttack, 
												ProjectileStruct* pProjectileSignalInfo, CDnSkill::SkillInfo& parentSkillInfo)
{
	// Misc settings for firing projectiles.
	DnSkillHandle hSkill = hRootAttacker->FindSkill( parentSkillInfo.iSkillID );

	// Set to designate the target via homing.
	pProjectileSignalInfo->nOrbitType = CDnProjectile::Homing;
	pProjectileSignalInfo->nTargetType = CDnProjectile::Target;
	pProjectileSignalInfo->VelocityType = CDnProjectile::Accell;
	pProjectileSignalInfo->fSpeed = 1000.0f;//CHAINATTACK_PROJECTILE_SPEED;
	pProjectileSignalInfo->nValidTime = 5000;

	MatrixEx Cross = *hActor->GetMatEx();
	Cross.m_vPosition.y += hActor->GetHeight() / 2.0f;

	CDnProjectile *pProjectile = new CDnProjectile( GetRoom(), hRootAttacker );

	pProjectile->SetPierce( pProjectileSignalInfo->bPierce == TRUE ? true : false );
	pProjectile->SetMaxHitCount( pProjectileSignalInfo->nMaxHitCount );

	if( pProjectileSignalInfo->nWeaponTableID > 0 ) 
	{
		if( pProjectileSignalInfo->nProjectileIndex != -1 )
		{
			DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr( (CMultiRoom*)g_pGameServerManager->GetRootRoom(), pProjectileSignalInfo->nProjectileIndex );
			if( hWeapon ) *(CDnWeapon*)pProjectile = *hWeapon.GetPointer();
		}
	}

	int nLength = pProjectile->GetWeaponLength();
	if( pProjectileSignalInfo->bIncludeMainWeaponLength ) 
	{
		DnWeaponHandle hWeapon = hActor->GetWeapon(0);
		if ( hWeapon )
			nLength += hWeapon->GetWeaponLength();
	}
	pProjectile->SetWeaponLength( nLength );

	pProjectile->SetWeaponType( (CDnWeapon::WeaponTypeEnum)( pProjectile->GetWeaponType() | CDnWeapon::Projectile ) );

	pProjectile->SetSpeed( pProjectileSignalInfo->fSpeed );

	pProjectile->SetTargetPosition( *hActorToAttack->GetPosition() );
	pProjectile->SetTargetActor( hActorToAttack );

	pProjectile->Initialize( Cross, static_cast<CDnProjectile::OrbitTypeEnum>(pProjectileSignalInfo->nOrbitType), 
		static_cast<CDnProjectile::DestroyOrbitTypeEnum>(pProjectileSignalInfo->nDestroyOrbitType), 
		static_cast<CDnProjectile::TargetTypeEnum>(pProjectileSignalInfo->nTargetType) );
	pProjectile->SetValidTime( pProjectileSignalInfo->nValidTime );
	pProjectile->SetVelocityType( static_cast<CDnProjectile::VelocityTypeEnum>(pProjectileSignalInfo->VelocityType) );

	pProjectile->SetParentSkill( hSkill );

	parentSkillInfo.hPrevAttacker = hActor;
	parentSkillInfo.iLeaveCount -= 1; // Decrement configured count by one.
	pProjectile->SetParentSkillInfo( parentSkillInfo );
	pProjectile->FromSkill( true );

	CDnState rootAttackerState;
	int iNumSE = hSkill->GetStateEffectCount();
	for( int i = 0; i < iNumSE; ++i )
	{
		const CDnSkill::StateEffectStruct* pStateEffect = hSkill->GetStateEffectFromIndex( i );
		if( pStateEffect->ApplyType == CDnSkill::ApplySelf ) 
			continue;

		CDnSkill::StateEffectStruct SE = *pStateEffect;

		pProjectile->AddStateEffect( SE );
	}

	//#45331
	//Pass through the actor's state value saved when the projectile was created.
	pProjectile->SetShooterStateSnapshot( m_pShooterState );

#if defined(PRE_FIX_65287)
	pProjectile->SetShooterFinalDamageRate(m_fShooterFinalDamageRate);
#endif // PRE_FIX_65287

	pProjectile->PostInitialize();
}

#if defined(PRE_ADD_55295)
void CDnProjectile::ChangeProjectileRotation()
{
	//Keep the direction as is and reset the Y axis to re-set only the Y axis
	//Recalculate X, Z direction vectors
	m_Cross.m_vYAxis = EtVector3(0.0f, 1.0f, 0.0f);

	m_Cross.MakeUpCartesianByYAxis();
}
#endif // PRE_ADD_55295


#if defined(PRE_FIX_59238)
void CDnProjectile::AddHittedActor(DnActorHandle hHittedActor)
{
	if (hHittedActor)
		m_HittedActorList.insert(std::make_pair(hHittedActor->GetUniqueID(), hHittedActor));
}

bool CDnProjectile::IsHittable(DnActorHandle hActor)
{
	//Check actor validity first..
	if (!hActor)
		return false;

	bool isSummonMonster = false;
	bool isOwnerActor = false;

	//Check if owner actor has puppet state effect..
	if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_247))
	{
		isOwnerActor = true;
	}
	//Whether it is a summon monster
	else if (hActor->IsMonsterActor())
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if (pMonsterActor && 
			pMonsterActor->IsSummonedMonster() &&
			pMonsterActor->IsPuppetSummonMonster())
			isSummonMonster = true;		
	}

	//Can hit if neither a summon monster nor an actor with the Puppet state effect.
	//Something is wrong if it is a summon monster with the Puppet state effect (just allow it to be hit)
	if ((isSummonMonster == false && isOwnerActor == false) ||
		(isSummonMonster == true && isOwnerActor == true))
		return true;


	//If actor has puppet state effect, check summoned monster actor in already-hit actor list
	//If included, self should not be hit.
	if (isOwnerActor == true)
	{
		//Own summoned monster list
		const std::list<DnMonsterActorHandle> & listSummonMonster = hActor->GetSummonedMonsterList();
		std::list<DnMonsterActorHandle>::const_iterator iter = listSummonMonster.begin();
		for( iter; iter != listSummonMonster.end(); ++iter )
		{
			DnMonsterActorHandle hMonster = (*iter);
			if (!hMonster)
				continue;

			//Skip if not a summon monster and not a PuppetSummon monster
			if (hMonster->IsSummonedMonster() == false || hMonster->IsPuppetSummonMonster() == false)
				continue;

			//If the summoned monster is already in the hit list, self must not be hit.
			std::map<DWORD, DnActorHandle>::iterator iter = m_HittedActorList.find(hMonster->GetUniqueID());
			if (iter != m_HittedActorList.end())
				return false;
		}

		//Own summoned group monster list
		const std::map<int, std::list<DnMonsterActorHandle> >& groupSummonMonster = hActor->GetGroupingSummonedMonsterList();
		std::map<int, std::list<DnMonsterActorHandle> >::const_iterator mapIter;
		for (mapIter = groupSummonMonster.begin(); mapIter != groupSummonMonster.end(); ++mapIter)
		{
			const std::list<DnMonsterActorHandle> & listSummonMonster = mapIter->second;
			std::list<DnMonsterActorHandle>::const_iterator iter = listSummonMonster.begin();
			for (; iter != listSummonMonster.end(); ++iter)
			{
				DnMonsterActorHandle hMonster = (*iter);
				if (!hMonster)
					continue;

				//Skip if not a summon monster and not a PuppetSummon monster
				if (hMonster->IsSummonedMonster() == false || hMonster->IsPuppetSummonMonster() == false)
					continue;

				//If the summoned monster is already in the hit list, self must not be hit.
				std::map<DWORD, DnActorHandle>::iterator iter = m_HittedActorList.find(hMonster->GetUniqueID());
				if (iter != m_HittedActorList.end())
					return false;
			}
		}

		return true;
	}
	//If summon monster, must not be hit if its master actor is in the already-hit actor list.
	else if (isSummonMonster == true)
	{
		DnActorHandle hOwnerActor;
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if (pMonsterActor)
			hOwnerActor = pMonsterActor->GetSummonerPlayerActor();

		//May be hit if the summon master actor does not have the Puppet state effect.
		if (!hOwnerActor || hOwnerActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_247) == false)
			return true;

		//If the summon monster's master actor is already in the hit list, this actor must not be hit,
		//If the owner actor is not in the hit list, it should be hit.
		std::map<DWORD, DnActorHandle>::iterator iter = m_HittedActorList.find(hOwnerActor->GetUniqueID());
		if (iter != m_HittedActorList.end())
			return false;
		else
			return true;
	}

	return true;
}
#endif // PRE_FIX_59238

#if defined(PRE_FIX_59336)
void CDnProjectile::ApplyComboLimitStateEffect( DnActorHandle hActor )
{
	for( DWORD i= 0 ; i < m_ComboLimitStateEffectList.size(); i++ ) 
	{
		const CDnSkill::StateEffectStruct& SE = m_ComboLimitStateEffectList.at(i);

		if (m_HitStruct.szSkipStateBlows && CDnSkill::IsSkipStateBlow(m_HitStruct.szSkipStateBlows, (STATE_BLOW::emBLOW_INDEX)SE.nID))
			continue;

		// For skills that also heal allies, two state effects are declared: Self heal and Target heal.
		// Self state effects are already applied to self, and since this is where Hit signals are judged, only Target is applied.
		switch( SE.ApplyType )
		{
		case CDnSkill::ApplySelf:
			continue;
			break;

		case CDnSkill::ApplyTarget:
			break;

		case CDnSkill::ApplyEnemy:
			if( m_hShooter->GetTeam() == hActor->GetTeam() )
				continue;
			break;

		case CDnSkill::ApplyFriend:
			if( m_hShooter->GetTeam() != hActor->GetTeam() )
				continue;
			break;
		}

		m_ParentSkillInfo.iProjectileShootActionIndex = m_iShooterShootActionIndex;
		m_ParentSkillInfo.iProjectileSignalArrayIndex = m_iSignalArrayIndex;

		//Prevent adding if the same skill has the same start time..
		bool isExistSameSkillTimeBlow = false;
		DNVector(DnBlowHandle) vlAppliedBlows;
		hActor->GatherAppliedStateBlowByBlowIndex( (STATE_BLOW::emBLOW_INDEX)m_ComboLimitStateEffectList[i].nID, vlAppliedBlows );
		int iNumAppliedBlow = (int)vlAppliedBlows.size();
		for( int iAppliedBlow = 0; iAppliedBlow < iNumAppliedBlow; ++iAppliedBlow )
		{
			DnBlowHandle hBlow = vlAppliedBlows.at( iAppliedBlow );
			if (!hBlow)
				continue;

			CDnSkill::SkillInfo* pExistSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());

			//////////////////////////////////////////////////////////////////////////
			// #56880
			// Also need to check if the skill start time is the same...
			LOCAL_TIME parentSkillStartTime = 0;
			LOCAL_TIME nowBlowSkillStartTime = 0;

			parentSkillStartTime = GetSkillStartTime();
			CDnComboDamageLimitBlow* pNowBlow = static_cast<CDnComboDamageLimitBlow*>(hBlow.GetPointer());
			if (pNowBlow)
				nowBlowSkillStartTime = pNowBlow->GetSkillStartTime();
			//////////////////////////////////////////////////////////////////////////

			if (pExistSkillInfo)
			{
				if (m_ParentSkillInfo.hSkillUser && 
					m_ParentSkillInfo.hSkillUser == pExistSkillInfo->hSkillUser &&	//If the skill user is the same and
					m_ParentSkillInfo.iSkillID == pExistSkillInfo->iSkillID &&		//When the skill is the same
					nowBlowSkillStartTime == parentSkillStartTime					//When skill start time is the same
					)
				{
					isExistSameSkillTimeBlow = true;
					break;
				}
			}

		}

		int iID = -1;
		bool bCheckCanBegin = true;
		if (isExistSameSkillTimeBlow == false)
		{
			//When adding a state effect from a projectile, include and pass the projectile skill start time.
			m_ParentSkillInfo.projectileSkillStartTime = GetSkillStartTime();

#ifdef PRE_FIX_REMOVE_STATE_EFFECT_PACKET // Do not send a packet because this form is only used on the server.
			iID = hActor->CDnActor::CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_ComboLimitStateEffectList[i].nID, 
				m_ComboLimitStateEffectList[i].nDurationTime, m_ComboLimitStateEffectList[i].szValue.c_str(), false, bCheckCanBegin );
#else
			iID = hActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_ComboLimitStateEffectList[i].nID, 
				m_ComboLimitStateEffectList[i].nDurationTime, m_ComboLimitStateEffectList[i].szValue.c_str(), false, bCheckCanBegin );
#endif

			//Reset projectile time.
			m_ParentSkillInfo.projectileSkillStartTime = 0;
		}

		// If there is a chain attack state effect among the added state effects, set projectile-related info.
		// Ping Pong Bomb state effect must also be added similarly.
		if( iID != -1 )
		{
			DnBlowHandle hBlow = hActor->GetStateBlowFromID( iID );
			//#56880 When adding state effect via projectile, if state effect 242, set the projectile skill's start time..??
			if (m_ComboLimitStateEffectList[i].nID == STATE_BLOW::BLOW_242)
			{
				LOCAL_TIME skillStartTime = GetSkillStartTime();

				DnBlowHandle hBlow = hActor->GetStateBlowFromID( iID );
				if( hBlow )
				{
					CDnComboDamageLimitBlow* pComboLimitBlow = static_cast<CDnComboDamageLimitBlow*>( hBlow.GetPointer() );
					if (pComboLimitBlow)
						pComboLimitBlow->SetSkillStartTime(skillStartTime);
				}
			}
		}
	}
}
#endif // PRE_FIX_59336


void CDnProjectile::ApplySkillStateEffect(DnActorHandle hActor)
{
	if (!hActor)
		return;

	// Ignore applying state effects when the target is in Ice Prison state
	if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
		return;

	// Distinguish state effect application based on whether the skill target is set to ally or target.
	switch( m_ParentSkillInfo.eTargetType )
	{
	case CDnSkill::Enemy:
	case CDnSkill::Self:
		if( m_hShooter->GetTeam() == hActor->GetTeam() )
			return;
		break;

	case CDnSkill::Friend:
	case CDnSkill::Party:
		if( m_hShooter->GetTeam() != hActor->GetTeam() )
			return;
		break;

		// If the skill target includes both allies and enemies, distinguish whether to apply in the state effect application logic.
	case CDnSkill::All:
		break;
	}

	// Add status ailment
	// If it is a Projectile created by some skill,
	CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;
	map<int, bool> mapDuplicateResult;

	//Projectile skill info can become invalid due to skill level-up.
	//If the projectile actor exists and skill ID is set but the skill is invalid, re-set the skill.
	if (m_hShooter && m_ParentSkillInfo.iSkillID != 0 && !m_hParentSkill)
	{
		DnSkillHandle hSkill = m_hShooter->FindSkill(m_ParentSkillInfo.iSkillID);
		if (hSkill)
			SetParentSkill(hSkill);
	}

	if( m_hParentSkill )
	{
		// Identify the Actor's continuous effect index and add the state effect on success.
		eResult = CDnSkill::CanApplySkillStateEffect( hActor, m_hParentSkill, mapDuplicateResult, true );
	}


	if( CDnSkill::CanApply::Fail != eResult )
	{
		for( DWORD i= 0 ; i < m_VecStateEffectList.size(); i++ ) 
		{
			const CDnSkill::StateEffectStruct& SE = m_VecStateEffectList.at(i);

			if (m_HitStruct.szSkipStateBlows && CDnSkill::IsSkipStateBlow(m_HitStruct.szSkipStateBlows, (STATE_BLOW::emBLOW_INDEX)SE.nID))
				continue;


			// For skills that also heal allies, two state effects are declared: Self heal and Target heal.
			// Self state effects are already applied to self, and since this is where Hit signals are judged, only Target is applied.
			switch( SE.ApplyType )
			{
			case CDnSkill::ApplySelf:
				continue;
				break;

			case CDnSkill::ApplyTarget:
				break;

			case CDnSkill::ApplyEnemy:
				if( m_hShooter->GetTeam() == hActor->GetTeam() )
					continue;
				break;

			case CDnSkill::ApplyFriend:
				if( m_hShooter->GetTeam() != hActor->GetTeam() )
					continue;
				break;
			}

			// When stacking the same skill, probability-based skill effects already have their probability checked in CanApplySkillStateEffect
			// Already passed. So here, check if the probability check was already done, and if so, call the function to skip checking probability again.
			bool bAllowAddThisSE = true;
			bool bCheckCanBegin = true;
			if( CDnSkill::ApplyDuplicateSameSkill == eResult )
			{
				map<int, bool>::iterator iter = mapDuplicateResult.find( m_VecStateEffectList[i].nID );
				// If not in map, the current actor does not have the state effect, so just run the normal state effect addition routine.
				if( mapDuplicateResult.end() != iter )
				{
					// Called CanAdd because a probabilistic state effect of the same skill is currently applied, but it failed.
					// Do not add state effect in this case.
					if( false == (iter->second) )
						bAllowAddThisSE = false;
					else
						// Since CanAdd already passed, skip separate check when calling CmdAddStateEffect.
						bCheckCanBegin = false;
				}
			}

			m_ParentSkillInfo.iProjectileShootActionIndex = m_iShooterShootActionIndex;
			m_ParentSkillInfo.iProjectileSignalArrayIndex = m_iSignalArrayIndex;

			if( bAllowAddThisSE )
			{
				// #72931 Screamer curse shared cooldown handling -> generalize later if more skills use this pattern
				bool bShareCurseCoolTime = false;
				if( m_VecStateEffectList[i].nID == STATE_BLOW::BLOW_244 )
				{
					DNVector(DnBlowHandle) vlhBlows;
					hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_244, vlhBlows );
					if( static_cast<int>( vlhBlows.size() ) > 0 )
					{
						CDnCurseBlow* pDnCurseBlow = static_cast<CDnCurseBlow*>( vlhBlows[0].GetPointer() );
						if( pDnCurseBlow )
						{
							m_VecStateEffectList[i].szValue += ";";
							m_VecStateEffectList[i].szValue += FormatA( "%f", pDnCurseBlow->GetCoolTime() );
							bShareCurseCoolTime = true;
						}
					}
				}

				// Remove registered state effects marked for removal here. [2010/12/08 semozz]
				hActor->RemoveResetStateBlow();

				//When adding a state effect from a projectile, include and pass the projectile skill start time.
				m_ParentSkillInfo.projectileSkillStartTime = GetSkillStartTime();
				int iID = hActor->CmdAddStateEffect( &m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_VecStateEffectList[i].nID, 
					m_VecStateEffectList[i].nDurationTime, m_VecStateEffectList[i].szValue.c_str(), false,
					bCheckCanBegin );

				if( bShareCurseCoolTime )	// #72931 Removed the previously added cooldown
				{
					std::string::size_type delimiterindex = m_VecStateEffectList[i].szValue.rfind( ";" );
					if( delimiterindex != std::string::npos )
					{
						m_VecStateEffectList[i].szValue.erase( delimiterindex, m_VecStateEffectList[i].szValue.length() - delimiterindex );
					}
				}

				//Reset projectile time.
				m_ParentSkillInfo.projectileSkillStartTime = 0;

				// If there is a chain attack state effect among the added state effects, set projectile-related info.
				// Ping Pong Bomb state effect must also be added similarly.
				if( iID != -1 )
				{
					DnBlowHandle hBlow = hActor->GetStateBlowFromID( iID );
					if( hBlow )
					{
						if( STATE_BLOW::BLOW_060 == m_VecStateEffectList.at( i ).nID )
						{
							CDnChainAttackBlow* pChainAttackBlow = static_cast<CDnChainAttackBlow*>( hBlow.GetPointer() );
							pChainAttackBlow->SetProjectileSignal( m_pProjectileSignal.get() );
							pChainAttackBlow->SetRootAttackerState( m_pShooterState.get() );
						}
						else
							if( STATE_BLOW::BLOW_208 == m_VecStateEffectList.at( i ).nID )
							{
								CDnPingpongBlow* pPingPongBlow = static_cast<CDnPingpongBlow*>( hBlow.GetPointer() );
								pPingPongBlow->SetProjectileSignal( m_pProjectileSignal.get() );
								pPingPongBlow->SetRootAttackerState( m_pShooterState.get() );
							}
					}

					//#56880 When adding state effect via projectile, if state effect 242, set the projectile skill's start time..??
					if (m_VecStateEffectList[i].nID == STATE_BLOW::BLOW_242)
					{
						LOCAL_TIME skillStartTime = GetSkillStartTime();

						DnBlowHandle hBlow = hActor->GetStateBlowFromID( iID );
						if( hBlow )
						{
							CDnComboDamageLimitBlow* pComboLimitBlow = static_cast<CDnComboDamageLimitBlow*>( hBlow.GetPointer() );
							if (pComboLimitBlow)
								pComboLimitBlow->SetSkillStartTime(skillStartTime);
						}
					}
				}
				else
				{
					//Skip the routine below if not ChainAttack.
					STATE_BLOW::emBLOW_INDEX eBlowIndex = (STATE_BLOW::emBLOW_INDEX)m_VecStateEffectList[i].nID;
					if (eBlowIndex != STATE_BLOW::BLOW_060 && eBlowIndex != STATE_BLOW::BLOW_208)
						continue;

					//0. Check state effect config values...
					float fRange = 1000.0f;
					int nMaxCount = -1;
					GetChainAttackInfo(m_VecStateEffectList[i], fRange, nMaxCount);

					//Set projectile transfer count.
					if (m_ParentSkillInfo.iLeaveCount == -1)
						m_ParentSkillInfo.iLeaveCount = nMaxCount;
					//////////////////////////////////////////////////////////////////////////

					//2. Select next target character
					//////////////////////////////////////////////////////////////////////////
					DnActorHandle hRootAttacker = m_ParentSkillInfo.hSkillUser;
					int iRootAttackerTeam = hRootAttacker->GetTeam();

					DnActorHandle hActorToAttack = FindNextChainActor(iRootAttackerTeam, hActor, m_ParentSkillInfo.hPrevAttacker, fRange);

					//Skip if projectile transfer count is 0 or target actor to transfer to is not found.
					if (m_ParentSkillInfo.iLeaveCount <= 0 || !hActorToAttack)
						continue;
					//////////////////////////////////////////////////////////////////////////

					//1. Set projectile signal info.
					//////////////////////////////////////////////////////////////////////////

					// Unique ID of the first attacking actor
					DWORD dwRootAttackerActorUniqueID = hRootAttacker->GetUniqueID();
					DWORD dwPrevAttackerActorUniqueID = m_ParentSkillInfo.hPrevAttacker ? m_ParentSkillInfo.hPrevAttacker->GetUniqueID() : UINT_MAX;


					// Unique info needed to locate the projectile signal on the client.
					int iActionIndex = m_ParentSkillInfo.iProjectileShootActionIndex;
					int iProjectileSignalArrayIndex = m_ParentSkillInfo.iProjectileSignalArrayIndex;

					ProjectileStruct projectileSignalInfo;
					bool bFound = false;

					CEtActionSignal *pSignal = hRootAttacker->GetSignal( iActionIndex, iProjectileSignalArrayIndex );
					if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) {
#ifdef PRE_FIX_MEMOPT_SIGNALH
						CopyShallow_ProjectileStruct(projectileSignalInfo, static_cast<ProjectileStruct*>(pSignal->GetData()));
#else
						projectileSignalInfo = *(static_cast<ProjectileStruct*>(pSignal->GetData()));
#endif

						bFound = true;
					}
					//////////////////////////////////////////////////////////////////////////

					//Send packet to client...
					hActor->SendChainAttackProjectile(hRootAttacker, dwPrevAttackerActorUniqueID, iActionIndex, iProjectileSignalArrayIndex, hActorToAttack, m_ParentSkillInfo.iSkillID);


					//3. Configure projectile info and spawn
					//////////////////////////////////////////////////////////////////////////
					CreateChainAttackProjectile(hRootAttacker, hActor, hActorToAttack, &projectileSignalInfo, m_ParentSkillInfo);
					//////////////////////////////////////////////////////////////////////////

				}
			}
		}
	}

#if defined(PRE_FIX_59336)
	//Added state effect 242..
	ApplyComboLimitStateEffect(hActor);
#endif // PRE_FIX_59336

}