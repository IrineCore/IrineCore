#include "StdAfx.h"
#include "DnProjectile.h"
#include "DnWeapon.h"
#include "MAActorRenderBase.h"
#include "VelocityFunc.h"
#include "EtActionSignal.h"
#include "DnWorld.h"
#include "DnLocalPlayerActor.h"
#include "DnMonsterActor.h"
#include "MAAiBase.h"
#include "DnSkill.h"
#include "NavigationMesh.h"
#include "DnRangeChecker.h"
#include "DnHideMonsterActor.h"
#include "IDnOrbitProcessor.h"
#include "DnTableDB.h"

#include "DnPartsMonsterActor.h"

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
#include "GameOption.h"
#include "TaskManager.h"
#include "DnCannonMonsterActor.h"
#endif

#ifndef _FINAL_BUILD
#include "DnInterface.h"
#endif // #ifndef _FINAL_BUILD

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const float FALL_GRAVITY_ACCEL = -15.0f;

#define SHOOT_ACTION_NAME "Shoot"
#define HIT_ACTION_NAME "Hit"
#define DESTROY_ACTION_NAME "Destroy"


// Game server and client have separate DnProjectile class files, so update both when modifying.
const int PROJECTILE_PACKET_BUFFER_SIZE = 128;


CDnProjectile::CDnProjectile( DnActorHandle hActor, bool bProcess, bool bIncreaseUniqueID )
: CDnWeapon( bProcess, bIncreaseUniqueID )
, m_vPrevPos( 0.f, 0.f, 0.f )
, m_vStartPos( 0.f, 0.f, 0.f )
, m_vTrailOffset( 0.f, 0.f, 0.f )
, m_vTargetPosition( 0.f, 0.f, 0.f )
, m_bOnCollisionCalled( false )
, m_bPierce( false )
, m_fElapsedTime( 0.0f )
, m_pProjectileSignal( NULL )
, m_pOrbitProcessor( NULL )
, m_pPacketBuffer( NULL )
, m_pPacketStream( NULL )
#ifndef PRE_MOD_PROJECTILE_HACK
, m_iSignalArrayIndex( 0 )
#endif
{
	SetWeaponType( WeaponTypeEnum::Projectile );
	m_hActor = hActor;

	m_InvalidLocalTime = 0;
	m_CreateLocalTime = 0;

	m_bValidDamage = true;
	m_nStateCondition = 0;
	m_fThickness = 0.f;
	m_fSpeed = 3000.f;
	m_fResistance = 0.f;

	// Time interval before projectile object is removed after becoming invalid when it has no destroy action. 3 sec is too long, should set to 1 sec. Discuss with planner later.
	m_nDestroyOrbitTimeGap = 3000;

	m_bStick = false;
	m_fGravity = 0.f;

	m_bFirstProcess = true;

	m_DestroyOrbitType = FallGravity;
	m_ValidType = ValidTypeEnum( WeaponLength | Stick );
	m_nValidTime = 0;

	m_VelocityType = Accell;

	m_nMaxHitCount = 0;
	m_bDirectionFollowView = false;
	m_bHasHitAction = false;
	m_bHasHitSignalInHitAction = false;
	m_bHitFXUseWorldAxis = false;
	m_bHasDestroyAction = false;

#ifndef PRE_MOD_PROJECTILE_HACK
	m_dwParentProjectileID = UINT_MAX;
#endif

	m_fProjectileOrbitRotateZDegree = 0.0f;

	m_iShootActionIndex = -1;
	m_iHitActionIndex = -1;
	m_iDestroyActionIndex = -1;

	m_fLineEffectLifeTime = 0.0f;

	m_bTraceHitTarget = false;
	m_bTraceHitActorHittable = false;
	m_cShooterType = 0;
	m_dwShooterUniqueID = -1;
	m_nShooterSerialID = -1;
	m_nShooterActionIndex = -1;
	m_nShooterSignalIndex = -1;

	m_nTargetPartsIndex = -1;
	m_nTargetPartsBoneIndex = -1;

	m_bDoNotDestroyEffect = false;

	m_OrbitType = Linear;
	m_TargetType = CrossHair; 

#if defined(PRE_FIX_52329)
	m_nIgnoreHitType = 0;
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	m_bHitActionVectorInit = false;
#endif // PRE_ADD_55295
}

CDnProjectile::~CDnProjectile()
{
	StopParticleFXList();
	SAFE_RELEASE_SPTR( m_hLineTrail );
	SAFE_RELEASE_SPTR( m_hLine );
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

	m_matExWorld = Offset;
	m_OffsetCross = Offset;
	m_vStartPos = m_vPrevPos = m_matExWorld.m_vPosition;
	SetActionQueue( SHOOT_ACTION_NAME, 0, 0.f );

	m_iShootActionIndex = GetElementIndex( SHOOT_ACTION_NAME );

	m_bFirstProcess = true;

	if( m_OrbitType == Homing ||
		m_OrbitType == TerrainHoming ) 
	{
		m_ValidType = (ValidTypeEnum)( m_ValidType | Time );
		m_ValidType = (ValidTypeEnum)( m_ValidType & ~WeaponLength );
	}

	if( IsExistAction( HIT_ACTION_NAME ) )
	{
		// Has a hit action.
		m_bHasHitAction = true;

		m_iHitActionIndex = GetElementIndex( HIT_ACTION_NAME );

		// The hit action actually has a hit signal.
		CEtActionBase::ActionElementStruct* pHitActionElement = this->GetElement( "Hit" );
		int iNumSignals = (int)pHitActionElement->pVecSignalList.size();
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

	// TODO: For accelerating homing, it always hits unless it collides with a prop, so set lifetime to infinite.
	if( OrbitTypeEnum::Homing == m_OrbitType &&
		VelocityTypeEnum::Accell == m_VelocityType )
	{
		m_nValidTime = INT_MAX;
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
	m_pOrbitProcessor = IDnOrbitProcessor::Create( m_matExWorld, m_OffsetCross, &OrbitProperty );		// Initial position equals the offset, but stored separately for clarity.

	_ASSERT( m_pOrbitProcessor );
	if( NULL != m_pOrbitProcessor )
	{
		m_pOrbitProcessor->SetTargetActor( m_hTargetActor );
		m_pOrbitProcessor->SetTargetPosition( m_vTargetPosition );
		m_pOrbitProcessor->SetValidTimePointer( &m_nValidTime );
		m_pOrbitProcessor->SetTargetPartsIndex(m_nTargetPartsIndex, m_nTargetPartsBoneIndex);

		bResult = true;
	}

	return bResult;
}

bool CDnProjectile::AttachTrail( EtTextureHandle hTexture, float fLifeTime, float fLineWidth, float fMinSegment, EtVector3 &vOffset )
{
	m_hLineTrail = EternityEngine::CreateLineTrail( hTexture, fLifeTime, fLineWidth, fMinSegment );
	if( !m_hLineTrail ) return false;
	m_vTrailOffset = vOffset;
	return true;
}


void CDnProjectile::AttachLine( EtTextureHandle hTexture, float fLifeTime, float fLineWidth, float fUMoveSpeed )
{
	// Use line effect only for homing acceleration that currently has a target actor.
	_ASSERT( m_hTargetActor );
	if( m_hTargetActor )
	{
		m_hLine = EternityEngine::CreateLine( hTexture, fLifeTime, fLineWidth, fUMoveSpeed );
		m_fLineEffectLifeTime = fLifeTime;
		EtVector3 vTargetPos = *m_hTargetActor->GetPosition();
		vTargetPos.y += m_hTargetActor->GetHeight() / 2.0f;
		m_hLine->AddPoint( m_matExWorld.m_vPosition, vTargetPos );
		m_hLine->Show( IsShow() );
	}
}

void CDnProjectile::DoDestroyAction( void )
{
	if( m_bHasDestroyAction )
	{
		SetActionQueue( DESTROY_ACTION_NAME );

		CEtActionBase::ActionElementStruct* pActionElement = GetElement( DESTROY_ACTION_NAME );
		m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f);
	}
}


void CDnProjectile::ProcessOrbit( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bTraceHitTarget )
	{
		if( m_hTraceActor )
		{
			EtVector3 vPos = *m_hTraceActor->GetPosition();
			m_matExWorld.SetPosition( vPos );
			return;
		}
	}

	if( m_bStick )
		return;

	// Partially created projectiles may not have had PostInitialize called.
	// A typical example is Aerial Chain Shot firing without a quiver sub-weapon.
	if( m_pOrbitProcessor )
		m_pOrbitProcessor->ProcessOrbit( m_matExWorld, m_vPrevPos, LocalTime, fDelta );
}

void CDnProjectile::ProcessValid( LOCAL_TIME LocalTime, float fDelta )
{
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
					bRangeOut = (fRangeFallGravityLength < EtVec3Length( &( m_matExWorld.m_vPosition - m_vStartPos ) ));
				}
				else
				{
					bRangeOut = ( EtVec3Length( &( m_matExWorld.m_vPosition - m_vStartPos ) ) > GetWeaponLength() );
				}

				if( bRangeOut )
				{
					m_bValidDamage = false;
					m_InvalidLocalTime = LocalTime;

					if (m_bDoNotDestroyEffect == false)
						StopParticleFXList();

					if( m_bHasDestroyAction )
						DoDestroyAction();
				}
			}
		}
	}

	if( 0 == m_InvalidLocalTime && ( m_ValidType & ValidTypeEnum::Time ) ) 
	{
		if( (int)( LocalTime - m_CreateLocalTime ) > m_nValidTime ) {
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;

			// Accelerated homing is guaranteed to hit when the time limit expires, so this is needed for accurate presentation.
			if( (OrbitTypeEnum::Homing == m_OrbitType) && 
				(VelocityTypeEnum::Accell == m_VelocityType) )
			{
				OnCollisionWithActor();
				m_bStick = true;
			}
			else
			{
				if( m_bHasDestroyAction )
					DoDestroyAction();
			}
		}
	}

#if defined(PRE_FIX_52329)
	//Skip Valid check if IgnoreHitType is non-zero..
	if (m_nIgnoreHitType == 0)
	{
#endif // PRE_FIX_52329

	// If terrain linear, make projectile disappear when current position is in an impassable area.
	if( OrbitTypeEnum::TerrainLinear == m_OrbitType ||
		OrbitTypeEnum::TerrainHoming == m_OrbitType )
	{
		if( false == MAWalkMovement::IsMovableBlock( &INSTANCE(CDnWorld), m_matExWorld.GetPosition() ) )
		{
			if (m_bDoNotDestroyEffect == false)
				StopParticleFXList();

			OnCollisionWithGround();

			if( m_bValidDamage ) 
			{
				m_bValidDamage = false;
				m_InvalidLocalTime = LocalTime;
			}

			m_bStick = true;
		}
	}

	if( !m_bStick && ( m_ValidType & ValidTypeEnum::Stick ) )
	{
#if defined(PRE_FIX_55855)
		//A Prop's NaviMesh may be placed on top of the HeightMap.
		float fHeight	 = INSTANCE(CDnWorld).GetMaxHeightWithProp( m_matExWorld.m_vPosition );
#else
		float fHeight = CDnWorld::GetInstance().GetHeight( m_matExWorld.m_vPosition );
#endif // PRE_FIX_55855

		if( m_matExWorld.m_vPosition.y <= fHeight ) 
		{
			// Calculate direction based on the projectile's absolute position.
			// ZVector is controlled by OrbitProcessor and may differ from the actual direction.
			EtVector3 vDir = m_matExWorld.m_vPosition - m_vPrevPos;
			EtVec3Normalize( &vDir, &vDir );

			EtVector3 vPickPos;
			if( CDnWorld::GetInstance().Pick( m_vPrevPos, vDir, vPickPos ) == true ) 
			{
				m_matExWorld.m_vPosition = vPickPos;

#if defined(PRE_FIX_55855)
				m_matExWorld.m_vPosition.y = INSTANCE(CDnWorld).GetMaxHeightWithProp( vPickPos );
#endif // PRE_FIX_55855
			}

			//m_matExWorld.m_vPosition += m_matExWorld.m_vZAxis * -10.f;
			if( m_bValidDamage ) 
			{
				m_bValidDamage = false;
				m_InvalidLocalTime = LocalTime;
			}
			m_bStick = true; 

			if (m_bDoNotDestroyEffect == false)
				StopParticleFXList();

			OnCollisionWithGround();
		}
	}

	// Check if collided with a prop. ///////////////////////////////////////////////
	if( false == m_bStick )
	{
		float fLength = EtVec3Length( &( m_matExWorld.m_vPosition - m_vPrevPos ) );
		EtVector3 vCenter = m_vPrevPos + ( m_matExWorld.m_vZAxis * ( fLength * 0.5f ) );
		DnPropHandle hResultProp = _CheckPropCollision( vCenter, fLength );

		if( hResultProp )
		{
			OnCollisionWithProp();
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;
			m_bStick = true;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	
#if defined(PRE_FIX_52329)
	}
#endif // PRE_FIX_52329

	// If already executing hit action, do not re-execute the action. (#21384)
	// If OnCollision~ was not called separately but action changed to hit, in shoot action 
	// Since it was changed directly, call OnCollisionWithGround().
	if( false == m_bOnCollisionCalled && m_iHitActionIndex == GetCurrentActionIndex() )
	{
		if( m_bValidDamage ) 
		{
			m_bValidDamage = false;
			m_InvalidLocalTime = LocalTime;
		}
		m_bStick = true; 

		if (m_bDoNotDestroyEffect == false)
			StopParticleFXList();

		OnChangedNextActionToHit();
	}
}


void CDnProjectile::ProcessDestroyOrbit( LOCAL_TIME LocalTime, float fDelta )
{
	if( LocalTime - m_InvalidLocalTime > m_nDestroyOrbitTimeGap ) 
	{
		if( m_hLine )
		{
			// If a line effect exists, defer until that effect's lifetime expires.
			ShowWeapon( false );
			return;
		}
		else
		{
			SetDestroy();
		}
	}

	if( GetObjectHandle() ) {
		if( !IsSignalRange( STE_AlphaBlending ) )
			GetObjectHandle()->SetObjectAlpha( 1.f - ( 1.f / m_nDestroyOrbitTimeGap * ( LocalTime - m_InvalidLocalTime ) ) );
	}

	switch( m_DestroyOrbitType ) {
		case Instantly:
			{
				// 2010.2.24 
				// For constant-velocity homing like Gargoyle Blue Charge Bolt where the Hit action is a dummy hit action without a hit signal,
				// Make it disappear immediately when DestroyOrbit occurs.
				bool bConstantHoming = (OrbitTypeEnum::Homing == m_OrbitType) && (VelocityTypeEnum::Constant == m_VelocityType) && (false == m_bHasHitSignalInHitAction);
				if( false == m_bHasHitAction ||
					bConstantHoming )		// Projectiles with hit action auto-destroy when hit action finishes or validtime expires.
				{
					if (m_bDoNotDestroyEffect == false)
						StopParticleFXList();

					if( m_hObject ) m_hObject->ShowObject( false );
					SetDestroy();
				}
			}
			break;
		case FallGravity:
		case RangeFallGravity:
			{
				if( !m_bStick ) 
				{
					if( m_pOrbitProcessor )
						m_pOrbitProcessor->ProcessFallGravityOrbit( m_matExWorld, m_vPrevPos, LocalTime, fDelta, m_fGravity, FALL_GRAVITY_ACCEL );
				}
			}
			break;
	}
}


DnPropHandle CDnProjectile::_CheckPropCollision( EtVector3& vCenter, float fLength )
{
	DnPropHandle hResult;
	DNVector(DnPropHandle) hVecProp;
	int nCount = CDnWorld::GetInstance().ScanProp( vCenter, fLength * 0.5f, (PropTypeEnum)-1, hVecProp );
	if( nCount > 0 ) 
	{
		float fMinDistance = FLT_MAX;
		float fDist = 0.f;
		SSegment Segment;
		Segment.vOrigin = m_vPrevPos;
		Segment.vDirection = m_matExWorld.m_vZAxis * fLength;

		for( int i=0; i<nCount; i++ ) 
		{
			if( !hVecProp[i] || !hVecProp[i]->GetObjectHandle() )
				continue;

			if( hVecProp[i]->IsProjectileSkip() )
				continue;

			if( hVecProp[i] == m_hShooterProp )
				continue;

			if( hVecProp[i]->GetObjectHandle()->TestSegmentCollision( Segment ) ) 
			{
				fDist = EtVec3Length( &( hVecProp[i]->GetMatEx()->m_vPosition - m_matExWorld.m_vPosition ) );

				if( fDist < fMinDistance ) 
				{
					fMinDistance = fDist;
					hResult = hVecProp[i];
				}
			}
		}
	}

	return hResult;
}


// Check if hit a prop
void CDnProjectile::ProcessDamage( LOCAL_TIME LocalTime, float fDelta )
{

}


void CDnProjectile::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_CreateLocalTime == 0 ) m_CreateLocalTime = LocalTime;

	// Hanki 2009.7.22
	// Elapsed time since projectile was created
	// Related to projectile trajectory.. (#4851)
	m_fElapsedTime = float(LocalTime - m_CreateLocalTime) * 0.001f;

	ProcessOrbit( LocalTime, fDelta );

	CDnWeapon::Update( m_matExWorld );
	CDnWeapon::Process( LocalTime, fDelta );

	if( !m_bValidDamage  ) 
		ProcessDestroyOrbit( LocalTime, fDelta );	// Move up... Delay deletion by one frame so that Hit Signal can trigger.

	if( m_bValidDamage ) 
		ProcessDamage( LocalTime, fDelta );
	
	ProcessValid( LocalTime, fDelta );
	NonScaledMatrixValidCheck( m_matExWorld );

	m_vPrevPos = m_matExWorld.m_vPosition;

	if( !m_bStick ) 
	{
		if( m_hLineTrail ) {
			m_hLineTrail->AddPoint( m_matExWorld.m_vPosition + m_vTrailOffset );
			m_hLineTrail->Show( IsShow() );
		}
	}

	// Currently uses the line effect only for homing acceleration with a target actor.
	if( m_hLine && m_hActor && m_hTargetActor )
	{
		// If projectile propagates chain state effect, use that actor's info as the base.
		DnActorHandle hActor = m_hActor;
		if( m_hChainShooter )
			hActor = m_hChainShooter;

		EtVector3 vStartPos = *hActor->GetPosition();
		vStartPos.y += hActor->GetHeight() / 2.0f;
		m_hLine->ModifyStartPoint( vStartPos );

		EtVector3 vTargetPos = *m_hTargetActor->GetPosition();
		vTargetPos.y += m_hTargetActor->GetHeight() / 2.0f;
		m_hLine->ModifyEndPoint( vTargetPos );

		m_fLineEffectLifeTime -= fDelta;
		if( m_fLineEffectLifeTime < 0.0f )
			SAFE_RELEASE_SPTR( m_hLine );
	}

	if (m_hTargetActor)
	{
		if (GetOrbitType() == OrbitTypeEnum::Homing || GetOrbitType() == OrbitTypeEnum::TerrainHoming)
			GetObjectCross()->Look( m_hTargetActor->GetPosition() );
	}

	m_bFirstProcess = false;

#ifndef _FINAL_BUILD
	// Add the same on the client to print a confirmation message.
	// Added defensive code for map 301 (Isolated Island) where spit-flower projectile direction vector becomes (0, 0, 0) and fails to disappear when player position is off.
	if( EtVec3LengthSq( &m_matExWorld.m_vZAxis ) < 0.001f )
	{
		// Projectile-type projectiles may have speed set to 0, because speed is calculated in real time~
		if( Projectile != m_OrbitType )
		{
			SetDestroy();
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", L"Projectile disappears immediately because direction vector is 0, 0, 0.", false );
			GetInterface().AddChatMessage( CHATTYPE_SYSTEM, L"", L"Please set the direction vector for projectiles with trajectory types other than Projectile.", false );
		}
	}
#endif // #ifndef _FINAL_BUILD

}

void CDnProjectile::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{

#ifdef PRE_ADD_ENABLE_EFFECT_SIGNAL
	bool bShow = false; 
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
	if( pTask && pTask->GetGameTaskType() != GameTaskType::PvP )	
	{ 
		if( ! CGameOption::GetInstance().m_bEnablePlayerEffect && m_hActor  )
		{	
			if( m_hActor->IsPlayerActor() && CDnActor::s_hLocalActor->GetUniqueID() != m_hActor->GetUniqueID() )
				bShow = true;
			if( m_hActor->IsMonsterActor() )
			{
				CDnCannonMonsterActor* pCannonMonster = dynamic_cast<CDnCannonMonsterActor*>( m_hActor.GetPointer() );
				if( ! pCannonMonster )
				{	
					CDnMonsterActor* pMonster = dynamic_cast<CDnMonsterActor*>( m_hActor.GetPointer() );
					if( pMonster 
						&& pMonster->GetSummonerPlayerActor() 
						&& pMonster->GetSummonerPlayerActor()->IsPlayerActor() 
						&& CDnActor::s_hLocalActor->GetUniqueID() != m_hActor->GetUniqueID() )
					{
						bShow = true;
					}
				}
			}
			if( bShow && ! SetEnableSignal( Type, pPtr ) )		
				return;
		}			
	}
#endif

	switch( Type ) {
		case STE_Hit:
			{

			}
			break;
		case STE_AttachTrail:
			if( m_hLineTrail ) break;
			{
				AttachTrailStruct *pStruct = (AttachTrailStruct *)pPtr;
				if( pStruct->nTextureIndex != -1 ) {
					EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
					if ( hTexture )
						AttachTrail( hTexture, pStruct->fLifeTime, pStruct->fTrailWidth, pStruct->fMinSegment, *pStruct->vOffset );
				}
			}
			break;
		
		case STE_AttachLine:
			{
				if( !m_hLine )
				{
					AttachLineStruct* pStruct = (AttachLineStruct*)pPtr;
					if( pStruct->nTextureIndex != -1 )
					{
						EtTextureHandle hTexture = CEtResource::GetSmartPtr( pStruct->nTextureIndex );
						if( hTexture )
							AttachLine( hTexture, pStruct->fLifeTime, pStruct->fLineWidth, pStruct->fUVMoveSpeed );
					}
				}
			}
			break;

		case STE_Gravity:
			{
				if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) break;
				if( !m_hActor ) break;
				GravityStruct *pStruct = (GravityStruct *)pPtr;

				EtVector3 vPos = m_matExWorld.m_vPosition + *pStruct->vOffset;
				DNVector(DnActorHandle) hVecList;
				CDnActor::ScanActor( vPos, pStruct->fDistanceMax, hVecList );

				for( DWORD i=0; i<hVecList.size(); i++ ) {
					if( !hVecList[i] ) continue;
					if( hVecList[i] == m_hActor ) continue;
					if( hVecList[i]->IsDie() ) continue;
					if( hVecList[i]->GetWeight() == 0.f ) continue;
					if( hVecList[i]->IsNpcActor() ) continue;
					if( hVecList[i]->ProcessIgnoreGravitySignal() )	continue;

					bool bHittable = false;
					switch( pStruct->nTargetType ) {
						case 0: // Enemy
							if( hVecList[i]->GetTeam() == m_hActor->GetTeam() ) break;
							bHittable = true;
							break;
						case 1: // Friend
							if( hVecList[i]->GetTeam() != m_hActor->GetTeam() ) break;
							bHittable = true;
							break;
						case 2: // All
							bHittable = true;
							break;
					}
					if( !bHittable ) continue;

					bHittable = true;
					switch( pStruct->nApplyTargetState ) {
						case 0:	// Normal
							if( hVecList[i]->GetState() == CDnActorState::Down ) break;
							bHittable = true;
							break;
						case 1:	// Down
							if( !( hVecList[i]->GetState() & CDnActorState::Down ) ) break;
							bHittable = true;
							break;
						case 2:	// Normal + Down
							bHittable = true;
							break;
					}
					if( !bHittable ) continue;


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


void CDnProjectile::StopParticleFXList()
{
	// Since all are template classes of the parent class, they hold nothing other than the particles and effects they created.
	for( DWORD i=0; i<TSmartPtrSignalImp<EtBillboardEffectHandle,NormalParticleSignalStruct>::m_VecList.size(); i++ ) {
		if( TSmartPtrSignalImp<EtBillboardEffectHandle,NormalParticleSignalStruct>::m_VecList[i].hHandle ) TSmartPtrSignalImp<EtBillboardEffectHandle,NormalParticleSignalStruct>::m_VecList[i].hHandle->StopPlay();
	}
	for( DWORD i=0; i<TSmartPtrSignalImp<EtBillboardEffectHandle, LinkParticleSignalStruct>::m_VecList.size(); i++ ) {
		if( TSmartPtrSignalImp<EtBillboardEffectHandle, LinkParticleSignalStruct>::m_VecList[i].hHandle ) TSmartPtrSignalImp<EtBillboardEffectHandle,LinkParticleSignalStruct>::m_VecList[i].hHandle->StopPlay();
	}

	// #24862 Try removing the effect object when SetDestroy() is called.
	TSmartPtrSignalImp<EtEffectObjectHandle, TSmartPtrSignalStruct::Default>::RemoveAllSignalHandle( true );
	TSmartPtrSignalImp<EtEffectObjectHandle, LinkFXSignalStruct>::RemoveAllSignalHandle( true );
	TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveAllSignalHandle( true );
}

void CDnProjectile::SetDestroy( void )
{
	CDnWeapon::SetDestroy();
}

// Called when hit by another actor
void CDnProjectile::OnDamageSuccess( DnActorHandle hDamagedActor, CDnActor::SHitParam &HitParam )
{
	// If the projectile has homing option set, assign the homing target here. #22666
	if( !m_hTraceActor &&
		m_bTraceHitTarget && hDamagedActor )
		m_hTraceActor = hDamagedActor;

	// Objects meant to pass through continue passing through
	if( false == m_bPierce && /*m_bHasHitAction &&*/ !m_bOnCollisionCalled )
	{
		// Set whether to remove items like piercings depending on type here.

		// Currently when hidden, effects do not show either.. (#17497)
		if( false == m_bHasHitAction )
			ShowWeapon( false );

		// If Process() has never been called and m_LocalTime is 0, assign the time.
		if( 0 == m_LocalTime && hDamagedActor )
			m_LocalTime = hDamagedActor->GetLocalTime();

		m_bValidDamage = false;
		m_InvalidLocalTime = m_LocalTime;
		m_DestroyOrbitType = FallGravity;
		m_nDestroyOrbitTimeGap = 1000;
		m_bStick = true;
		
		if (m_bDoNotDestroyEffect == false)
			StopParticleFXList();

		OnCollisionWithActor( true );
	}
}


void CDnProjectile::OnCollisionWithActor( bool bHitFromServer/* = false*/ )
{
	if( false == m_bPierce && false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
			m_bOnCollisionCalled = true;

			// #36693 Do not run hit action while destroy action is running.
			if( bHitFromServer ||
				GetCurrentActionIndex() != m_iDestroyActionIndex )
			{
#if defined(PRE_ADD_55295)
				//#55295
				//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
				if (m_bHitActionVectorInit)
					ChangeProjectileRotation();
#endif // PRE_ADD_55295

				// Extract duration of hit action.. and client gives ~2 sec extra grace period for presentation.
				SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );
				CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
				m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f)/* + 2000*/;
			}
		}
	}
}


void CDnProjectile::OnCollisionWithGround( void )
{
	if( false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
			m_bOnCollisionCalled = true;

			// #36693 For RangeFallGravity DestoryType, do not run hit action on ground impact while destroy action is running.
			bool bDoHitAction = false;
			if( RangeFallGravity == m_DestroyOrbitType )
			{
				if( GetCurrentActionIndex() != m_iDestroyActionIndex )
				{
					bDoHitAction = true;
				}
			}
			else
			{
				bDoHitAction = true;
			}

			// Extract duration of hit action.. and client gives ~2 sec extra grace period for presentation.
			if( bDoHitAction )
			{
#if defined(PRE_ADD_55295)
				//#55295
				//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
				if (m_bHitActionVectorInit)
					ChangeProjectileRotation();
#endif // PRE_ADD_55295

				SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );
				CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
				m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f);
			}
		}
	}
}


void CDnProjectile::OnCollisionWithProp( void )
{
	if( false == m_bOnCollisionCalled )
	{
		if( m_bHasHitAction )
		{
			m_bOnCollisionCalled = true;

			// #36693 For RangeFallGravity DestoryType, do not run hit action on ground impact while destroy action is running.
			bool bDoHitAction = false;
			if( RangeFallGravity == m_DestroyOrbitType )
			{
				if( GetCurrentActionIndex() != m_iDestroyActionIndex )
				{
					bDoHitAction = true;
				}
			}
			else
			{
				bDoHitAction = true;
			}

			// #36693 Do not run hit action while destroy action is running.
			if( bDoHitAction )
			{
#if defined(PRE_ADD_55295)
				//#55295
				//When changing to a hit action, if this flag is set, adjust the direction vector to be parallel to the plane.
				if (m_bHitActionVectorInit)
					ChangeProjectileRotation();
#endif // PRE_ADD_55295

				// Extract duration of hit action.. and client gives ~2 sec extra grace period for presentation.
				SetActionQueue( HIT_ACTION_NAME, 0, 0.0f );
				CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
				m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f)/* + 2000*/;
			}
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

			// Extract duration of hit action.. and client gives ~2 sec extra grace period for presentation.
			CEtActionBase::ActionElementStruct* pActionElement = GetElement( HIT_ACTION_NAME );
			m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f)/* + 2000*/;
		}
	}
}


void CDnProjectile::NonScaledMatrixValidCheck( EtMatrix &EtMatrix )
{
#ifndef _FINAL_BUILD_
	float fDet = EtMatrixDeterminant(&EtMatrix);
#endif
}

MatrixEx * CDnProjectile::GetObjectCross()
{
	if( m_bOnCollisionCalled && m_bHitFXUseWorldAxis ) {
		m_ObjectCross.m_vXAxis = EtVector3(1, 0, 0) * EtVec3Length(&m_matExWorld.m_vXAxis);
		m_ObjectCross.m_vYAxis = EtVector3(0, 1, 0) * EtVec3Length(&m_matExWorld.m_vYAxis);
		m_ObjectCross.m_vZAxis = EtVector3(0, 0, 1) * EtVec3Length(&m_matExWorld.m_vZAxis);
		m_ObjectCross.m_vPosition = m_matExWorld.GetPosition();
		return &m_ObjectCross;
	}
	else {
		return &m_matExWorld;
	}	
}


void CDnProjectile::OnFinishAction( const char* szPrevAction, const char *szNextAction, LOCAL_TIME time )
{
	// #24931 When there is a hit action without a hit signal, make it disappear immediately after the shoot action ends.
	if( false == m_bValidDamage &&
		false == m_bHasHitSignalInHitAction )
	{
		// Remove immediately if the hit action has no hit signal and the shoot action has ended.
		if( 0 == strcmp( SHOOT_ACTION_NAME, szPrevAction ) )
			SetDestroy();
	}

	// After hit action ends, execute destroy action if it exists. 
	if( m_bHasHitAction )
	{
		if( 0 == strcmp( HIT_ACTION_NAME, szPrevAction ) )
		{
			if( m_bHasDestroyAction )
			{
				//CEtActionBase::ActionElementStruct* pActionElement = GetElement( DESTROY_ACTION_NAME );
				//m_nDestroyOrbitTimeGap = int(((float)pActionElement->dwLength / CDnActionBase::m_fFps) * 1000.0f);
				//SetActionQueue( DESTROY_ACTION_NAME, 0, 0.0f );
				DoDestroyAction();
			}
		}
	}
}


// TODO: Places that do new CDnProjectile and immediately return NULL.. Delete the created object before returning. Check TB side too..
CDnProjectile *CDnProjectile::CreateProjectile( DnActorHandle hShooter, MatrixEx &LocalCross, ProjectileStruct *pStruct, EtVector3 vForceTargetPos/*=EtVector3(0.f,0.f,0.f)*/, 
											    DnActorHandle hForceTargetActor/*=CDnActor::Identity()*/, const EtVector3* pForceDir/* = NULL*/, bool bCheckLocalUser/* = true*/ )
{
	if( !hShooter )
		return NULL;

	MatrixEx CrossResult = LocalCross;

	CrossResult.m_vPosition += CrossResult.m_vXAxis * ( pStruct->vOffset->x * hShooter->GetScale()->x );
	CrossResult.m_vPosition += CrossResult.m_vYAxis * ( pStruct->vOffset->y * hShooter->GetScale()->y );
	CrossResult.m_vPosition += CrossResult.m_vZAxis * ( pStruct->vOffset->z * hShooter->GetScale()->z );
	
	CDnProjectile *pProjectile = NULL;

	float fVelocityMultiply = 1.0f;

	switch( pStruct->nTargetType ) 
	{
	case TargetTypeEnum::CrossHair: // Crosshair
			{
				// Note: Must ignore other players' characters reaching here via signal
				// CrossHair projectiles arriving via packet are created properly. 
				// If you skip this, 2 projectiles appear including one fired by someone else toward your own CrossHair
				
				////////////// For crosshair mode, only LocalPlayer creates the projectile. ////////////////////////////////////////////////////////
				if( CDnActor::s_hLocalActor != hShooter )
				{
					SAFE_DELETE( pProjectile );
					return NULL;
				}
				/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

				pProjectile = new CDnProjectile( hShooter );

				((CDnLocalPlayerActor*)hShooter.GetPointer())->CalcCrosshairOffset( &CrossResult.m_vPosition, &CrossResult, NULL, NULL, 150.f );
			}
			break;
		case TargetTypeEnum::Direction: // Direction
			{
				pProjectile = new CDnProjectile( hShooter );

				EtVector3	vDirection	= *pStruct->vDirection;
				bool		bAILook		= false;
				bool		bMonster	= false;

				// MonsterActor
				if( hShooter->GetActorType() > CDnActor::Reserved6 )
				{
					CDnMonsterActor *pMonster = dynamic_cast<CDnMonsterActor *>(hShooter.GetPointer());
					if( pMonster )
					{
						bMonster = true;
						if( pMonster->bIsAILook() )
						{
							// STE_ProjectileTargetPosition is a signal used only for TargetPosition.
							if( !pMonster->bIsProjectileTargetSignal() )
							{
								vDirection	= EtVec2toVec3( *pMonster->GetAIBase()->GetAILook() );
								bAILook		= true;
							}
						}
					}
				}

				EtVector3 vTemp;
				
				if( bAILook )
					vTemp = vDirection;
				else
				if( false == bMonster && pStruct->bDirectionFollowView )
				{
					// Not the current actor's matrix, but the rotation value of the view vector it is looking at 
					// Apply relatively to the direction currently set in the action tool
					// This applies when the player fires, so it is not implemented this way on the game server.
					// #1876 Related to projectile direction.
					//EtVec3TransformNormal( &vTemp, &vDirection, LocalCross );
					EtVector3 vViewDirection;

					DnCameraHandle hCamera = CDnCamera::GetActiveCamera();

					EtVector3 vOrig;
					int nWidth = CEtDevice::GetInstance().Width() / 2;

					// Defined in CDnLocalPlayerActor.cpp as float s_fCrossHairHeightRatio = 0.46f.
					// Cannot know others' settings, so put default values for now..
					int nHeight = (int)( CEtDevice::GetInstance().Height() * 0.46f );

					if( !hCamera ) 
					{
						SAFE_DELETE( pProjectile );
						return NULL;
					}

					hCamera->GetCameraHandle()->CalcPositionAndDir( nWidth, nHeight, vOrig, vViewDirection );
					EtVector3 vLookDir = vViewDirection;
					EtVector3 vDirZ( vViewDirection.x, 0.0f, vViewDirection.z );
					EtVec3Normalize( &vDirZ, &vDirZ );

					float fDot = EtVec3Dot( &vLookDir, &vDirZ );
					float fRotX = EtAcos( fDot );

					if( vViewDirection.y < 0.0f )
						fRotX *= -1.0f;

					EtMatrix matRot;
					EtMatrixRotationX( &matRot, -fRotX );

					EtVec3Normalize( &vDirection, &vDirection );
					EtVec3TransformCoord( &vDirection, &vDirection, &matRot );
					EtVec3TransformNormal( &vTemp, &vDirection, LocalCross );

				}
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
					//Occurs when LocalUser is checked while creating a projectile on the client, preventing projectile creation (Chain Lightning / Ping-Pong Bomb).
					//Checks LocalUser via bCheckLocalUser by default; modified to skip check for Chain Lightning/Ping-Pong Bomb.
					if( true == bCheckLocalUser && CDnActor::s_hLocalActor != hShooter ) 
					{
						SAFE_DELETE( pProjectile );
						return NULL;
					}

					DnActorHandle hResultActor;
					EtVector3 vTargetPos;
					bool bCheckTarget;

					// Forcibly set projectile target takes highest priority.
					if( hForceTargetActor )
					{
						hResultActor = hForceTargetActor;
						vTargetPos = *hForceTargetActor->GetPosition();
					}
					else if( hShooter->IsProcessSkill() )
					{
						hResultActor = hShooter->GetSkillTarget();
						vTargetPos = *hShooter->GetSkillTargetPos();
					}
					else 
					{
						if(pStruct->nTargetType == TargetTypeEnum::Shooter )
						{
#ifdef PRE_ADD_PLAYER_HITBACK_PROJECTILE
							hResultActor = hShooter;
							vTargetPos = *hShooter->GetPosition();
#endif
						}
						else
						{
							bCheckTarget = ((CDnLocalPlayerActor*)hShooter.GetPointer())->CalcCrosshairOffset( NULL, &CrossResult, NULL, &hResultActor );
							if( bCheckTarget == true ) vTargetPos = CrossResult.m_vPosition;
							if( bCheckTarget == false ) vTargetPos = *hShooter->GetPosition();
						}
					}

					pProjectile = new CDnProjectile( hShooter );

					pProjectile->SetTargetPosition( vTargetPos );

					if( pStruct->nTargetType == TargetTypeEnum::Target ) 
					{
						// If there is no target actor locked by the crosshair, select the closest one at this moment.
						// For now, only check actors on a different team from the projectile shooter.
						if( !hResultActor )
						{
							// Range to search for targets (for acceleration homing like Guided Arrow, RangeChecker has the exact usable skill radius, so reuse it)
							float fSearchRange = 1000.0f;
							if( hShooter->IsProcessSkill() )
							{
								DnSkillHandle hSkill = hShooter->GetProcessSkill();
								IDnSkillUsableChecker* pChecker = hSkill->GetChecker( IDnSkillUsableChecker::RANGE_CHECKER );
								CDnRangeChecker* pRangeChecker = static_cast<CDnRangeChecker*>(pChecker);
								if( pRangeChecker )
									fSearchRange = (float)pRangeChecker->GetRange();
							}

							// For Guided Shot, it was already fired within range, so scan with a slight margin
							DNVector(DnActorHandle) vlActors;
							CDnActor::ScanActor( (*hShooter->GetPosition()), fSearchRange*2.0f, vlActors );
							
							float fNearestDistSQ = FLT_MAX;
							int nNearestActor = -1;
							int iNumActors = (int)vlActors.size();
							for( int iActor = 0; iActor < iNumActors; ++iActor )
							{
								DnActorHandle hTargetActor = vlActors.at( iActor );

								// #32115 Skip actors created as invisible monsters for a specific purpose.
								if( hTargetActor->IsMonsterActor() )
								{
									if( NULL != dynamic_cast<CDnHideMonsterActor*>(hTargetActor.GetPointer()) )
										continue;
								}
								if( hTargetActor->IsPlayerActor() )
								{
									CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hTargetActor.GetPointer());
									if(pPlayer->IsSpectatorMode())
										continue;
								}

								if( hTargetActor->GetTeam() != hShooter->GetTeam() &&
									(false == hTargetActor->IsDie()) /*&& hTargetActor->CDnActorState::IsHittable()*/ )	// #26934 Fixed an issue where the target actor could not be set if the target rolls right before firing.
								{
									if( hTargetActor->GetActorType() != CDnActorState::Npc &&
										hTargetActor->GetActorType() != CDnActorState::Pet &&
										hTargetActor->GetActorType() != CDnActorState::Vehicle && 
										hTargetActor->GetActorType() != CDnActorState::NoAggroTrap )
									{
										EtVector3 vDir = *hShooter->GetPosition() - *hTargetActor->GetPosition();
										float fDistSQ = EtVec3LengthSq( &vDir );
										if( fDistSQ < fNearestDistSQ )
										{
											fNearestDistSQ = fDistSQ;
											hResultActor = hTargetActor;
										}
									}
								}
							}
						}


						pProjectile->SetTargetActor( hResultActor );
					}

					//////////////////////////////////////////////////////////////////////////
					//For a projectile flying to a target point, update the direction vector from current projectile position to target point.
					//Modify to work only on brand projectiles for now.
					if (pStruct->nTargetStateIndex != 0)
					{
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					//////////////////////////////////////////////////////////////////////////
					else
					if( EtVec3LengthSq( pStruct->vDirection ) != 0.f )
					{
						EtVector3 vTemp;
						EtVec3TransformNormal( &vTemp, pStruct->vDirection, LocalCross );
						CrossResult.m_vZAxis = vTemp;
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
						CrossResult.MakeUpCartesianByZAxis();
					}

				}
				else
				{
					pProjectile = new CDnProjectile( hShooter );

					CDnMonsterActor *pMonster = dynamic_cast<CDnMonsterActor *>(hShooter.GetPointer());
					
					// NpcActor creates a Projecttile.. (/go 174 causes NpcActor to fire a projectile in cutscene, which isn't handled in code, so temporarily block it.
					if( !pMonster )
					{
						SAFE_DELETE( pProjectile );
						return NULL;
					}

					DnActorHandle	hTarget = pMonster->GetAggroTarget();
					EtVector3		vTargetPos;
					
					bool bTarget = true;
					if( pStruct->nTargetType == TargetTypeEnum::TargetPosition && pMonster->bIsProjectileTargetSignal() )
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
							_srand( pProjectile->GetUniqueID() );
							vTargetPos.x += cos( EtToRadian( _rand()%360 ) ) * pStruct->nTargetPosRandomValue;
							vTargetPos.z += sin( EtToRadian( _rand()%360 ) ) * pStruct->nTargetPosRandomValue;
						}

						float fHeight = CDnWorld::GetInstance().GetHeight( vTargetPos );

#ifdef PRE_FIX_PARTSMONSTER_AI_TARGETTING
						if( hTarget && hTarget->IsPartsMonsterActor() == false && fHeight < vTargetPos.y )
							vTargetPos -= hTarget->GetMatEx()->m_vZAxis * ( vTargetPos.y - fHeight );
#else
						if( hTarget && fHeight < vTargetPos.y )
							vTargetPos -= hTarget->GetMatEx()->m_vZAxis * ( vTargetPos.y - fHeight );
#endif
					}
					else
					{
						float fThreatRange = 1.0f;
						MAAiBase* pAi = pMonster->GetAIBase();
						if( pAi )
						{
							fThreatRange = (float)pMonster->GetAIBase()->GetThreatRange();
						}

						vTargetPos = *hShooter->GetPosition() + ( hShooter->GetMatEx()->m_vZAxis * fThreatRange );
					}

					// TargetPosition
					if( pStruct->nTargetType == TargetTypeEnum::TargetPosition ) 
					{
						// Set forced target position
						if( EtVec3LengthSq( &vForceTargetPos) > 0.f )
							vTargetPos = vForceTargetPos;

						pProjectile->SetTargetPosition( vTargetPos );

						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					// Target
					else if( pStruct->nTargetType == TargetTypeEnum::Target )
					{
						// Set forced target actor
						if( hForceTargetActor )
							hTarget = hForceTargetActor;

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
#ifdef PRE_ADD_PLAYER_HITBACK_PROJECTILE
						hTarget = hShooter;
						vTargetPos = *hTarget->GetPosition();

						pProjectile->SetTargetActor( hTarget );
						pProjectile->SetTargetPosition( vTargetPos );
#endif
					}

					//////////////////////////////////////////////////////////////////////////
					//For a projectile flying to a target point, update the direction vector from current projectile position to target point.
					//Modify to work only on brand projectiles for now.
					if (pStruct->nTargetStateIndex != 0)
					{
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					//////////////////////////////////////////////////////////////////////////
					else
					if( EtVec3LengthSq( pStruct->vDirection ) == 0.f ) {
						_ASSERT( !(pStruct->nOrbitType == Projectile && pStruct->VelocityType == Accell && pStruct->nTargetType == 2) &&
							"Projectiles of Projectile/TargetPosition type with gravity acceleration must have a direction set." );
						CrossResult.m_vZAxis = vTargetPos - CrossResult.GetPosition();
						EtVec3Normalize( &CrossResult.m_vZAxis, &CrossResult.m_vZAxis );
					}
					else
					// Skip this block only when it is Linear type and TargetPosition.
					// For Projectile type, use the direction computed here. 
					if( !( (pStruct->nOrbitType == OrbitTypeEnum::Linear) && 
						   (pStruct->nTargetType == TargetTypeEnum::TargetPosition) ) )
					{
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
						}
					}


					CrossResult.MakeUpCartesianByZAxis();
				}
			}
			break;

		case TargetTypeEnum::DestPosition:
			{
				pProjectile = new CDnProjectile( hShooter );

				EtVector3 vDestOffset = *(pStruct->vDestPosition);
				if( pStruct->nTargetPosRandomValue > 0 )
				{
					_srand( pProjectile->GetUniqueID() );
					vDestOffset.x += cos( EtToRadian( _rand()%360 ) ) * pStruct->nTargetPosRandomValue;
					vDestOffset.z += sin( EtToRadian( _rand()%360 ) ) * pStruct->nTargetPosRandomValue;
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

				crossDestPosition.m_vPosition.y = INSTANCE(CDnWorld).GetHeightWithProp( crossDestPosition.m_vPosition );

				pProjectile->SetTargetPosition( crossDestPosition.m_vPosition );
			}
			break;
	}

	if( pStruct->nWeaponTableID > 0 ) 
	{
#ifdef PRE_ADD_PROJECTILE_RANDOM_WEAPON
		int nRandomWeaponIndex = pStruct->nWeaponTableID;
		if( pStruct->RandomWeaponParam && strlen( pStruct->RandomWeaponParam ) > 0 )
		{
			std::vector<int> vecWeaponID;
			vecWeaponID.push_back( pStruct->nWeaponTableID );
			std::vector<int> vecPercent;
			std::vector<std::string> tokens;
			TokenizeA( pStruct->RandomWeaponParam, tokens, "/" );
			if( tokens.size() > 1 )
			{

				for( int i=0; i<static_cast<int>( tokens.size() ); i++ )
				{
					std::vector<std::string> tokensparam;
					TokenizeA( tokens[i].c_str(), tokensparam, ";" );
					if( tokensparam.size() > 0 )
					{
						if( i == 0 )
						{
							vecPercent.push_back( atof( tokensparam[i].c_str() ) * 1000.0f );
						}
						else
						{
							vecWeaponID.push_back( atoi( tokensparam[0].c_str() ) );
							if( tokensparam.size() > 1 )
							{
								vecPercent.push_back( atof( tokensparam[1].c_str() ) * 1000.0f );
							}
						}
					}
					tokensparam.clear();
				}
			}

			int nRadndom = _rand() % 1000;
			int nPercent = 0;
			for( int i=0; i<static_cast<int>( vecPercent.size() ); i++ )
			{
				if( nRadndom >= nPercent && nRadndom < vecPercent[i] + nPercent )
				{
					nRandomWeaponIndex = vecWeaponID[i];
					break;
				}
				else
				{
					nPercent = vecPercent[i];
				}
			}

			tokens.clear();
			vecWeaponID.clear();
			vecPercent.clear();
		}



		if (pStruct->nProjectileIndex != -1)
		{
			DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr(pStruct->nProjectileIndex);
			if (hWeapon) *(CDnWeapon*)pProjectile = *hWeapon.GetPointer();
		}
		else 
		{
			pProjectile->CDnWeapon::Initialize( nRandomWeaponIndex, 0 );
			pProjectile->CreateObject();
		}
#else // PRE_ADD_PROJECTILE_RANDOM_WEAPON
		if( pStruct->nProjectileIndex != -1 ) 
		{
			DnWeaponHandle hWeapon = CDnWeapon::GetSmartPtr( pStruct->nProjectileIndex );
			if( hWeapon ) *(CDnWeapon*)pProjectile = *hWeapon.GetPointer();
		}
		else 
		{
			pProjectile->CDnWeapon::Initialize( pStruct->nWeaponTableID, 0 );
			pProjectile->CreateObject();
		}
#endif // PRE_ADD_PROJECTILE_RANDOM_WEAPON
	}
	else if( hShooter->GetWeapon(1) ) 
	{
		*(CDnWeapon*)pProjectile = *hShooter->GetWeapon(1);
	}
	else
	{
		SAFE_DELETE( pProjectile );
		return NULL;
	}

	NonScaledMatrixValidCheck( CrossResult );

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

	pProjectile->SetPierce( ( pStruct->bPierce == TRUE ) );
	pProjectile->SetMaxHitCount( pStruct->nMaxHitCount );
	pProjectile->SetDirectionFollowView( (pStruct->bDirectionFollowView == TRUE) ? true : false );
	pProjectile->SetHitFXUseWorldAxis( (pStruct->bHitFXUseWorldAxis ==TRUE) ? true : false );
	pProjectile->SetProjectileOrbitRotateZ( pStruct->fProjectileOrbitRotateZ );
	pProjectile->SetTraceHitTarget( (pStruct->bTraceHitTarget == TRUE) ? true : false, 
									(pStruct->bTraceHitActorHittable == TRUE) ? true : false );


	pProjectile->SetDoNotDestroyEffect(pStruct->bDoNotDestroyEffect == TRUE ? true : false);
	pProjectile->SetProjectileSignal( pStruct );

#if defined(PRE_FIX_52329)
	pProjectile->SetIgnoreHitType(pStruct->nIgnoreHitType);
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
	pProjectile->SetHitActionVectorInit(pStruct->bHitActionVectorInit == TRUE ? true : false);
#endif // PRE_ADD_55295

	pProjectile->PostInitialize();

	return pProjectile;
}

CDnProjectile* CDnProjectile::CreateProjectileFromServerPacket( DnActorHandle hShooter, const BYTE* pPacket )
{
	CPacketCompressStream Stream( const_cast<BYTE*>(pPacket), 128 );

	bool bActorAttachWeapon;
	int nWeaponTableID, nWeaponLength = 0;
	DWORD dwUniqueID,dwGap;
	float fSpeed = 0.f;
	float fResistance = 0.f;
	int nValidTime = 0;
	int VelocityType = 0;
	EtVector3 vTargetPos;
	DWORD dwTargetUniqueID = -1;
	bool bPierce = false;
	CDnProjectile::OrbitTypeEnum OrbitType;
	CDnProjectile::DestroyOrbitTypeEnum DestroyType;
	CDnProjectile::TargetTypeEnum TargetType;
	MatrixEx Cross;
	Cross.Identity();
	bool bLoadTargetUniqueID = false;
	int nMaxHitCount = 0;
	bool bHitFXUseWorldAxis = false;
	float fProjectileOrbitRotateZ = 0.0f;

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
				DnActorHandle hHandle = CDnActor::FindActorFromUniqueID( dwShooterUniqueID );
				if( hHandle ) {
					CEtActionSignal *pSignal = hHandle->GetSignal( nShooterActionIndex, nShooterSignalIndex );
					if( pSignal && pSignal->GetSignalIndex() == STE_Projectile ) {
						pShooterStruct = static_cast<ProjectileStruct*>( pSignal->GetData() );
					}
				}
			}
			break;
		case 1:
			{
				DnWeaponHandle hHandle;
				if( bSendSerialID ) {
					for( DWORD i=0; i<2; i++ ) { // Should implement FindItemFromSerialID, but not needed for now so only search in EquipWeapon.
						DnWeaponHandle hWeapon = hShooter->GetWeapon(i);
						if( hWeapon && hWeapon->GetSerialID() == nShooterSerialID ) {
							hHandle = hWeapon;
							break;
						}
						if( hShooter->IsPlayerActor() ) {
							CDnPlayerActor *pPlayer = (CDnPlayerActor*)hShooter.GetPointer();
							hWeapon = pPlayer->GetCashWeapon(i);
							if( hWeapon && hWeapon->GetSerialID() == nShooterSerialID ) {
								hHandle = hWeapon;
								break;
							}
						}
					}
				}
				else {
					hHandle = CDnWeapon::FindWeaponFromUniqueID( dwShooterUniqueID );
					if( hHandle ) {
						hParentProjectileWeapon = hHandle;
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
	OrbitType = (CDnProjectile::OrbitTypeEnum)pShooterStruct->nOrbitType;
	DestroyType = (CDnProjectile::DestroyOrbitTypeEnum)pShooterStruct->nDestroyOrbitType;
	TargetType = (CDnProjectile::TargetTypeEnum)pShooterStruct->nTargetType;
	nValidTime = pShooterStruct->nValidTime;
	bHitFXUseWorldAxis = (pShooterStruct->bHitFXUseWorldAxis == TRUE) ? true : false;
	fProjectileOrbitRotateZ = pShooterStruct->fProjectileOrbitRotateZ;
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
			{
				Stream.Read( &vTargetPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &dwTargetUniqueID, sizeof(DWORD) );
			}
			break;
	}

	Stream.Read( &fSpeed, sizeof(float) );

#ifdef PRE_MOD_PROJECTILE_HACK
	bPierce = ( pShooterStruct->bPierce == TRUE );
	nMaxHitCount = pShooterStruct->nMaxHitCount;
	bool bTraceHitTarget = ( pShooterStruct->bTraceHitTarget == TRUE );
	bool bTraceHitActorHittable = ( pShooterStruct->bTraceHitActorHittable == TRUE );

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

	//Added missing packet info from the server.
	int nActionIndex = 0;
	Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

#else
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
#endif

#if defined(PRE_FIX_52329)
	int nIgnoreHitType = 0;
	Stream.Read(&nIgnoreHitType, sizeof(nIgnoreHitType));
#endif // PRE_FIX_52329

	CDnProjectile *pProjectile = new CDnProjectile( hShooter );
	pProjectile->SetUniqueID( dwUniqueID );
	pProjectile->SetPierce( bPierce );
	pProjectile->SetMaxHitCount( nMaxHitCount );

	pProjectile->SetTargetPartsIndex(nTargetPartsIndex, nTargetBoneIndex);

	if( bActorAttachWeapon )
	{
		CDnWeapon *pWeapon = hShooter->GetWeapon(1);
		if( pWeapon ) *(CDnWeapon*)pProjectile = *pWeapon;
	}
	else
	{
		pProjectile->CDnWeapon::Initialize( nWeaponTableID, 0 );
		pProjectile->CDnWeapon::CreateObject();
	}
	

#ifdef PRE_MOD_PROJECTILE_HACK
	nWeaponLength = pProjectile->GetWeaponLength();
	if( pShooterStruct->bIncludeMainWeaponLength && hShooter->GetWeapon(0) ) 
		nWeaponLength += hShooter->GetWeapon(0)->GetWeaponLength();
	if( hShooter->IsProcessSkill() )
		nWeaponLength += hShooter->GetProcessSkill()->GetIncreaseRange();

	if( hShooter->IsEnabledToggleSkill() )
		nWeaponLength += hShooter->GetEnabledToggleSkill()->GetIncreaseRange();
#endif
	pProjectile->SetWeaponLength( nWeaponLength );
	pProjectile->SetWeaponType( (CDnWeapon::WeaponTypeEnum)( pProjectile->GetWeaponType() | CDnWeapon::Projectile ) );

	pProjectile->SetSpeed( fSpeed );
	pProjectile->SetResistance( fResistance );
	pProjectile->SetTargetPosition( vTargetPos );
	pProjectile->SetTraceHitTarget( bTraceHitTarget, bTraceHitActorHittable );

	if( dwTargetUniqueID != -1 )
	{
		pProjectile->SetTargetActor( hShooter->CDnActor::FindActorFromUniqueID( dwTargetUniqueID ), false );
	}

	pProjectile->Initialize( Cross, OrbitType, DestroyType, TargetType);
	pProjectile->SetValidTime( nValidTime );
	pProjectile->SetVelocityType( (CDnProjectile::VelocityTypeEnum)VelocityType );
	pProjectile->ShowWeapon( hShooter->IsShow() );
	pProjectile->SetHitFXUseWorldAxis( bHitFXUseWorldAxis );
	pProjectile->SetProjectileOrbitRotateZ( fProjectileOrbitRotateZ );
	
#if defined(PRE_FIX_52329)
	pProjectile->SetIgnoreHitType(nIgnoreHitType);
#endif // PRE_FIX_52329

	pProjectile->PostInitialize();

	return pProjectile;
}


// Create a packet from the created projectile object.
// Must be an identical routine to client (note: this file is split between client and server).
CPacketCompressStream* CDnProjectile::GetPacketStream( void )
{
	DnActorHandle hShooter = m_hActor;

	if( !hShooter )
		return NULL;

	if( NULL == m_pPacketStream )
	{
		m_pPacketBuffer = new char[ PROJECTILE_PACKET_BUFFER_SIZE ];
		m_pPacketStream = new CPacketCompressStream( m_pPacketBuffer, PROJECTILE_PACKET_BUFFER_SIZE*sizeof(char) );

		// Note: For players, all projectile packets must be sent, since even Direction type depends on camera orientation.
		bool bActorAttachWeapon = false;
		int VelocityType = 0;
		int nValidTime = 0;
		float fValue;
		if( hShooter->GetWeapon(1) && 
			this->GetClassID() == hShooter->GetWeapon(1)->GetClassID() ) 
			bActorAttachWeapon = true;

		DWORD dwGap = 0;
		
		if( hShooter->IsPlayerActor() )
		{
			CDnLocalPlayerActor* pPlayerActor = static_cast<CDnLocalPlayerActor*>( hShooter.GetPointer() );
			pPlayerActor->GetSyncDatumGap();
		}

		m_pPacketStream->Write( &dwGap, sizeof(DWORD) );
		m_pPacketStream->Write( &bActorAttachWeapon, sizeof(bool) );
		int nValue;
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
		DWORD dwShooterUniqueID = hShooter->GetUniqueID();
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
		if( hShooter->GetProcessSkill() ) 
			nActionIndex = hShooter->GetCurrentActionIndex();

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

		int nActionIndex = -1;
		if( hShooter->GetProcessSkill() ) 
			nActionIndex = hShooter->GetCurrentActionIndex();

		m_pPacketStream->Write( &nActionIndex, sizeof(int) );

		// Note : Need an index to look up the signal struct for chain attack state effect.
		m_pPacketStream->Write( &m_iSignalArrayIndex, sizeof(int) );

		// From the first projectile fired in a projectile-firing skill such as Bow Master's Revolutionary Ballista
		// Info for transferring stats...
		DWORD dwParentID = this->GetParentProjectileID();
		m_pPacketStream->Write( &dwParentID, sizeof(DWORD) );

		DWORD dwParentShooterUniqueID = hShooter->GetUniqueID();
		m_pPacketStream->Write( &dwParentShooterUniqueID, sizeof(DWORD) );
#endif

#if defined(PRE_FIX_52329)
		m_pPacketStream->Write( &m_nIgnoreHitType, sizeof(m_nIgnoreHitType));
#endif // PRE_FIX_52329

#if defined(PRE_ADD_55295)
		m_pPacketStream->Write( &m_bHitActionVectorInit, sizeof(m_bHitActionVectorInit));
#endif // PRE_ADD_55295
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

float CDnProjectile::GetTraceHitProjectileActorAdditionalHeight( void )
{
	float fResult = 0.0f;
	
	if( m_hTraceActor )
	{
		SAABox box;
		m_hTraceActor->GetBoundingBox(box);
		fResult = box.Max.y - box.Min.y + m_hTraceActor->GetIncreaseHeight();
	}

	return fResult;
};

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
		CDnPartsMonsterActor* pPartsMonsterActor = dynamic_cast<CDnPartsMonsterActor*>(m_hTargetActor.GetPointer());
		if (pPartsMonsterActor)
		{
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

#if defined(PRE_ADD_55295)
void CDnProjectile::ChangeProjectileRotation()
{
	//Keep the direction as is and reset the Y axis to re-set only the Y axis
	//Recalculate X, Z direction vectors
	m_matExWorld.m_vYAxis = EtVector3(0.0f, 1.0f, 0.0f);

	m_matExWorld.MakeUpCartesianByYAxis();
}
#endif // PRE_ADD_55295
