#include "StdAfx.h"
#include "DnActor.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "MAActorRenderBase.h"
#include "MAMovementBase.h"
#include "MAFaceAniBase.h"
#include "EtSoundChannel.h"
#include "DnWeapon.h"
#include "EtActionSignal.h"
#include "EtActionCoreMng.h"
#include "VelocityFunc.h"
#include "DnWorldBrokenProp.h"
#include "DnWorldSector.h"
#include "DnProjectile.h"
#include "DnMonsterActor.h"
#include "DnInterface.h"
#include "TaskManager.h"
#include "MATransAction.h"
#include "DnStateBlow.h"
#include "DnBlow.h"
#include "DnSkill.h"
#include "DnTableDB.h"
#include "DnWorldTrapProp.h"
#include "DnActorClassDefine.h"
#include "DnPvPGameTask.h"
#include "DnQuestTask.h"
#include "GameOption.h"
#include "DnChangeActionStrProcessor.h"
#include "EtComputeDist.h"
#include "DnLocalPlayerActor.h"
#include "DnChargerBlow.h"
#include "DnChangeActionSetBlow.h"
#include "DnPAyBackMPBlow.h"
#include "DnSkillTask.h"
#include "DnFrostbiteBlow.h"
#include "DnVillageTask.h"
#include "DnFreezingPrisonBlow.h"
#include "DnProbInvincibleAtBlow.h"
#include "DnDurabilityGaugeMng.h"
#include "DnShockInfectionBlow.h"
#include "DnAddDamageOnCriticalBlow.h"
#include "DnDisableActionBlow.h"
#include "navigationmesh.h"
#include "DnStateBlowSignalProcessor.h"

#ifdef PRE_FIX_MEMOPT_SIGNALH
#include "DnCommonUtil.h"
#endif

#if defined(PRE_FIX_68898)
#include "DnPartialPlayProcessor.h"
#endif // PRE_FIX_68898

#ifndef USE_BOOST_MEMPOOL
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

DECL_DN_SMART_PTR_STATIC( CDnActor, 500 )

CEtOctree<CDnActor*> *CDnActor::s_pOctree = NULL;
float CEtOctreeNode<CDnActor*>::s_fMinRadius = 1000.0f;
DnActorHandle CDnActor::s_hLocalActor;
DWORD CDnActor::s_dwUniqueCount = 0;
int CDnActor::s_nFontIndex = 5;
float CDnActor::s_fDieDelta = 20.f;
std::map<DWORD, DnActorHandle> CDnActor::s_dwMapActorSearch;
CSyncLock CDnActor::s_LockSearchMap;
std::vector<CDnPlayerActor *> CDnActor::s_pVecDeleteList;

const char* CDnActor::s_acEffectBoneName[ CDnActor::Max_FX_Dummy_Bone ] = { "#FX_01", "#FX_02", "#FX_03", "#FX_04", "#FX_05", 
																			"#FX_06", "#FX_07", "#FX_08", "#FX_09", "#FX_10",
																			"#FX_11", "#FX_12", "#FX_13", "#FX_14" };	//Added for prefix

#ifdef PRE_ADD_GRAVITY_PROPERTY
float SquaredDistance( const EtVector3& vPos, const SAABox& BBox, bool bNear )
{
	float fSQDist = 0.0f;

	for( int i = 0; i < 3; ++i )
	{
		// Ignore y axis
		if( 1 == i )
			continue;

		float v = vPos[ i ];

		if( bNear ) {
			if( v < BBox.Min[ i ] ) 
				fSQDist += (BBox.Min[ i ] - v) * (BBox.Min[ i ] - v);

			if( v > BBox.Max[ i ] )
				fSQDist += (v - BBox.Max[ i ]) * (v - BBox.Max[ i ]);
		}
		else {
			if( v > BBox.Min[ i ] ) 
				fSQDist += (BBox.Min[ i ] - v) * (BBox.Min[ i ] - v);

			if( v < BBox.Max[ i ] )
				fSQDist += (v - BBox.Max[ i ]) * (v - BBox.Max[ i ]);
		}
	}

	return fSQDist;
}
#endif // PRE_ADD_GRAVITY_PROPERTY

#ifdef RENDER_PRESS_hayannal2009
bool CDnActor::s_bEnableRenderPress = false;
#endif

CDnActor::CDnActor( int nClassID, bool bProcess )
	: CDnUnknownRenderObject( bProcess )
	, m_vStaticPos( 0.f, 0.f, 0.f )
	, m_vPrevPos( 0.f, 0.f, 0.f )	
{
	m_nClassID = nClassID;
	m_fAddHeight = 0.f;
	m_dwUniqueID = -1;

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	m_pCheckPreSignalFunc = CDnActor::CheckPreSignal;
#endif
	
#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
	// Environment Effect Load
	MAEnvironmentEffect::Initialize( m_nClassID );
#endif
	CDnActorState::Initialize( this );
	MAFaceAniBase::Initialize( m_nClassID );

	m_bShow = true;
	m_fDownDelta = 0.f;
	m_fDieDelta = 0.f;
	m_fMaxDieDelta = CDnActor::s_fDieDelta;
	m_fStiffDelta = 0.f;	
	m_LastDamageTime = 0;
	m_LastHitSignalTime = 0;
	m_nLastHitSignalIndex = -1;
	m_nTeam = 0;

	m_bModifyPlaySpeed = false;
	m_PlaySpeedStartTime = 0;
	m_dwPlaySpeedTime = 0;

	m_nSuperAmmorTime = 0;
	m_bShowHeadName = false;
	memset( m_bHideExposureInfo, 0, sizeof(m_bHideExposureInfo) );

	m_HitCheckType = HitCheckTypeEnum::BoundingBox;
	m_DamageCheckType = DamageCheckTypeEnum::DifferentTeam;
	memset( m_bSelfDeleteWeapon, 0, sizeof(m_bSelfDeleteWeapon) );

	m_pStateBlow = new CDnStateBlow( GetMySmartPtr() );

	m_bIngnoreNormalSuperArmor = false;

	m_pCurrentNode = NULL;
	if( bProcess ) InsertOctreeNode();
	

	m_bUseSignalSkillCheck = false;
	
	m_iCantMoveReferenceCount = 0;
	m_iCantActionReferenceCount = 0;
	m_iCantUseSkillReferenceCount = 0;
	m_iCantAttackReferenceCount = 0;

	m_iCantXZMoveSE = 0;

#ifdef _SHADOW_TEST
	m_bEnableShadowActor = false;
	m_bIsShadowActor = false;
#endif 	
	m_nCurrSuperArmor = 0;
	m_nMaxSuperArmor = 0;
	m_SoftAppearStep = AS_NONE;
	m_SoftTime = 0;

	m_bOnSignalFromChargerSE = false;
	m_iFrameStopRefCount = 0;

	// Stores the fSpeed argument passed into SetPlaySpeed().
	m_fPlaySpeed = 0.0f;
	m_lBreakIntoLoadingFlag = false;

	m_pStateBlowSignalProcessor = new CDnStateBlowSignalProcessor();

	CDnHeadEffectRender::Initialize( GetMySmartPtr() );

	m_bChangeWeaponLocked = false;
	m_bSkipChangeWeaponAction = false;
	m_bSkipOnAttatchDetachWeapon = false;

#if defined(PRE_ADD_50907)
	m_ChangeWeaponRefCount = 0;
	m_bOrigWeaponWhenChangeWeaponSelfDelete = false;
#endif // PRE_ADD_50907
	
#if defined(PRE_ADD_50907)
	m_DisarmamentRefCount = 0;
	m_bOrigWeaponWhenDisarmamentSelfDelete = false;
#endif // PRE_ADD_50907

#if defined(PRE_FIX_50482)
	m_nChangeTeamRefCount = 0;
#endif // PRE_FIX_50482

#if defined(PRE_FIX_57706)
	m_nStickAniDlgRefCount = 0;
#endif // PRE_FIX_57706

#ifdef PRE_ADD_GRAVITY_PROPERTY
	m_fGravityEnd = 0.0f;
#endif // PRE_ADD_GRAVITY_PROPERTY
	// Rotate.
	m_dwRotateStartTime = 0;
	m_dwRotateTime = 0;	
	m_bRotate = false;;
	m_bRotLeft = false;	  // Rotation direction.	
	m_fSpeedRot = 0.0f;    // Rotation speed.
	m_fStartSpeed = 0.0f;  // Initial speed.
	m_fEndSpeed = 0.0f;    // End speed.
	m_vRotAxis.x = m_vRotAxis.y = m_vRotAxis.z = 0.0f; // Rotation axis coordinates.

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
	m_isMailBoxOpenByShortCutKey = false;
#endif // PRE_ADD_MAILBOX_OPEN



}

CDnActor::~CDnActor()
{
#ifdef _SHADOW_TEST
	SAFE_RELEASE_SPTR( m_hShadowActor );
#endif	
	RemoveUniqueSearchMap( this );
	ReleaseSignalImp();
	FreeAction();
	for( int i=0; i<CDnWeapon::EquipSlot_Amount; i++ ) {
		DetachWeapon( i );
	}
	RemoveOctreeNode();
	for( DWORD i=0; i<m_VecPreActionState.size(); i++ ) {
		SAFE_DELETE_VEC( m_VecPreActionState[i] );
	}
	for( DWORD i=0; i<m_VecPreActionCustomState.size(); i++ ) {
		SAFE_DELETE_VEC( m_VecPreActionCustomState[i] );
	}
	for( DWORD i=0; i<m_VecPreCanMoveState.size(); i++ ) {
		SAFE_DELETE_VEC( m_VecPreCanMoveState[i] );
	}

	SAFE_DELETE(m_pStateBlow);

	for ( std::map< int , EtSwordTrailHandle >::iterator it = m_TrailList.begin(); it != m_TrailList.end(); ++it) {
		SAFE_RELEASE_SPTR( it->second );
	}
	m_TrailList.clear();

	SAFE_DELETE(m_pStateBlowSignalProcessor);

}

void CDnActor::InsertOctreeNode()
{
	if( s_pOctree ) {
		if( m_pCurrentNode == NULL ) {
			SSphere Sphere;
			GetBoundingSphere( Sphere );
			m_pCurrentNode = s_pOctree->Insert( this, Sphere );
		}
	}
}

void CDnActor::RemoveOctreeNode()
{
	if( s_pOctree ) {
		if( m_pCurrentNode ) {
			s_pOctree->Remove( this, m_pCurrentNode );
			m_pCurrentNode = NULL;
		}
	}
}

bool CDnActor::Initialize()
{
#ifdef PRE_FIX_CLIENT_MEMOPTIMIZE
	// Environment Effect Load
	MAEnvironmentEffect::Initialize( m_nClassID );
#endif

#ifdef _SHADOW_TEST
	if( m_bEnableShadowActor ) {
		m_hShadowActor = CreateActor( m_nClassID, false, false, false );
		m_hShadowActor->EnableShadowActor( false );
		m_hShadowActor->Initialize();
		m_hShadowActor->SetAlphaBlend( 0.5f );
		m_hShadowActor->SetMaxHP( 10000 );
		m_hShadowActor->SetHP( 10000 );
		m_hShadowActor->SetShadowActor( true );
	}
#endif	
	InitializeHeadLook( GetObjectHandle() );

	if( IsPlayerActor() )
		MakePassiveSkillActionInfo();

	return true;
}

// Static Func
bool CDnActor::InitializeClass()
{
	if( !CDnWorld::GetInstance().IsActive() ) return false;

	float fCenter, fSize;
	CDnWorld::GetInstance().CalcWorldSize( fCenter, fSize );

	SAFE_DELETE( s_pOctree );
	s_pOctree = new CEtOctree<CDnActor*>;
	s_pOctree->Initialize( EtVector3( 0.f, fCenter, 0.f ), fSize );

	s_dwUniqueCount = 0;

	CDnStateBlow::InitializeClass();

	return true;
}

bool CDnActor::OnReleaseActorCallback( void *pThis, void *pParam, int nSize, LOCAL_TIME LocalTime )
{
	CDnActor *pActor = (CDnActor*)pThis;
	SAFE_DELETE( pActor );
	return true;
}

int CDnActor::OnReleaseActorUniqueID( void *pParam, int nSize )
{
	return *((int*)pParam);
}

bool CDnActor::OnCheckLoadingPacket( void *pThis, UINT nUniqueID, int nMainCmd, int nSubCmd, void *pParam, int nSize )
{
	return false;
}

void CDnActor::ProcessClass( LOCAL_TIME LocalTime, float fDelta )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		s_pVecProcessList[i]->SyncClassTime( LocalTime );
		s_pVecProcessList[i]->Process( LocalTime, fDelta );

		SSphere Sphere;
		s_pVecProcessList[i]->GetBoundingSphere( Sphere );
		s_pVecProcessList[i]->m_pCurrentNode = s_pOctree->Update( s_pVecProcessList[i], Sphere, s_pVecProcessList[i]->m_pCurrentNode );

		if( s_pVecProcessList[i]->IsDestroy() ) {
			bool bEnableDestroy = true;
			if( s_pVecProcessList[i]->IsPlayerActor() ) {
				MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody*>(s_pVecProcessList[i]);
				if( pPartsBody && pPartsBody->IsCombiningParts() ) {
					bEnableDestroy = false;
				}
			}

			if(s_pVecProcessList[i]->IsWaitingThread()) // If this actor itself is tied to another thread, delete it after the call finishes.
				continue;

			if( bEnableDestroy ) 
			{
				if( s_pVecProcessList[i]->IsNpcActor() )
				{
					CDnGameTask* pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
					if( pGameTask )
						pGameTask->RemoveNpcList( s_pVecProcessList[i]->GetActorHandle() );
				}

#ifdef PRE_MOD_PRELOAD_MONSTER
				bool bCheck = false;
				if( s_pVecProcessList[i]->IsMonsterActor() )
				{
					CDnGameTask* pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
					if( pGameTask && pGameTask->IsNoPreloadMonsterMap() && pGameTask->IsSyncComplete() )
					{
						CDnMonsterActor *pMonster = dynamic_cast<CDnMonsterActor *>( s_pVecProcessList[i]->GetActorHandle().GetPointer() );
						if( pMonster && pMonster->GetTeam() != 3 )
						{

							if (pMonster->GetGrade() == CDnMonsterState::Boss || pMonster->GetGrade() == CDnMonsterState::BossHP4  )
								bCheck = true;
						}
					}
				}
#endif
				s_pVecProcessList[i]->Release();
				i--;
#ifdef PRE_MOD_PRELOAD_MONSTER
				if( bCheck )
				{
					if( CEtActionCoreMng::IsActive() ) CEtActionCoreMng::GetInstance().FlushWaitDelete();
					CEtResource::FlushWaitDelete();
				}
#endif
			}
			else {
				// Storing as PlayerActor in the actor class is a bit awkward, but repeated dynamic_cast is worse, so cast once and push it.
				CDnPlayerActor *pPlayerActor = dynamic_cast<CDnPlayerActor*>(s_pVecProcessList[i]);
				if( pPlayerActor ) {
					s_pVecDeleteList.push_back( pPlayerActor );		// UniqueID is already set to -1 by SetDestroy on leave, and it is already removed from the Find map.
					s_pVecProcessList[i]->SetProcess( false );
					i--;
				}

				// To be more precise, we should search commands queued in LoadingTask to find and remove repeated equipment swap commands sent by that actor, but
				// Because commands queued in LoadingTask do not store a type, it is hard to clearly distinguish whether loading is for stage entry or equipment swap.
				// Could distinguish by size, but it might be an equipment swap sent after entry, so finding and deleting all is tricky.
				// However, thinking about it anyway,
				// When popping and executing commands queued here, it will Find nearby actors using the session ID, and
				// Since it is already removed from the Find list, actor lookup will find nothing and skip (Combine will no longer run).
			}
		}
	}
	for( DWORD i=0; i<s_pVecDeleteList.size(); i++ ) {
		if( s_pVecDeleteList[i]->IsCombiningParts() ) {
			// Have no choice but to wait until it ends...
		}
		else {
			delete s_pVecDeleteList[i];
			s_pVecDeleteList.erase( s_pVecDeleteList.begin() + i );
			--i;
		}
	}
}

void CDnActor::ProcessFlushPacketQueue()
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		s_pVecProcessList[i]->FlushPacketQueue( -1 );
	}
}

void CDnActor::ReleaseClass()
{
	SAFE_DELETE( s_pOctree );
	SAFE_DELETE_MAP( s_dwMapActorSearch );

 	DeleteAllProcessObject();
	DeleteAllActor();

	CDnStateBlow::ReleaseClass();
}

int CDnActor::ScanActor( EtVector3 &vPos, float fRadius, DNVector(CDnActor*) &VecList )
{
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;
	s_pOctree->Pick( Sphere, VecList );
	return (int)VecList.size();
}

int CDnActor::ScanActor( EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList )
{
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;

	DNVector(CDnActor*) VecNonSmartPtrList;
	VecNonSmartPtrList.reserve( 200 );
	s_pOctree->Pick( Sphere, VecNonSmartPtrList );

	VecList.clear();
	for each( CDnActor *pActor in VecNonSmartPtrList ) 
	{
		VecList.push_back( pActor->GetMySmartPtr() );
	}
	return (int)VecList.size();
}

int CDnActor::ScanActorByActorSize( EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside/* = false*/, bool bActorSize/* = false*/ )
{
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;

	DNVector(CDnActor*) VecNonSmartPtrList;
	s_pOctree->Pick( Sphere, VecNonSmartPtrList, bInside, bActorSize );

	VecList.clear();
	for each( CDnActor *pActor in VecNonSmartPtrList ) 
	{
		VecList.push_back( pActor->GetMySmartPtr() );
	}
	return (int)VecList.size();
}

DnActorHandle CDnActor::FindActorFromUniqueID( DWORD dwUniqueID )
{
	ScopeLock<CSyncLock> Lock(s_LockSearchMap);
	std::map<DWORD, DnActorHandle>::iterator it = s_dwMapActorSearch.find( dwUniqueID );
	if( it != s_dwMapActorSearch.end() ) {
		if( !it->second->IsProcess() ) return CDnActor::Identity();
		return it->second;
	}
	return CDnActor::Identity();

}

DnActorHandle CDnActor::FindActorFromName( TCHAR *szName )
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) {
		if( s_pVecProcessList[i]->IsPetActor() ) continue;
		if( !__wcsicmp_l( s_pVecProcessList[i]->GetName(), szName ) ) return s_pVecProcessList[i]->GetMySmartPtr();
	}
	return CDnActor::Identity();
}

DnActorHandle CDnActor::FindActorFromUniqueIDBySmartPtrList( DWORD dwUniqueID )
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	DnActorHandle hResultActor = CDnActor::Identity();
	int nCount = CDnActor::GetItemCount();
	for( int i=0; i<nCount; i++ ) {
		DnActorHandle hActor = CDnActor::GetItem(i);
		if( hActor->GetUniqueID() == dwUniqueID ) {
			hResultActor = hActor;
			break;
		}
	}
	return hResultActor;
}

void CDnActor::AllMonsterKill()
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) 
	{
		DnActorHandle hActor = s_pVecProcessList[i]->GetMySmartPtr();
		
		if ( hActor->GetActorType() > Reserved6 ) 
		{
			hActor->SetHP(0);
			hActor->SetDieDelta(3.0f);
		}
	}
}

void CDnActor::SetAllMonsterDebugDrawing(bool bEnableCollision, bool bEnableBound)
{
	ScopeLock<CSyncLock> Lock(s_Lock);
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) 
	{
		DnActorHandle hActor = s_pVecProcessList[i]->GetMySmartPtr();
		CDnActor* pActor = hActor->GetMySmartPtr();

		MAActorRenderBase* pRenderBase = dynamic_cast<MAActorRenderBase*>(pActor);
		if ( pRenderBase && pRenderBase->GetObjectHandle() )
		{
			pRenderBase->GetObjectHandle()->ShowBoundingBox(bEnableBound );
			pRenderBase->GetObjectHandle()->ShowCollisionPrimitive(bEnableCollision );
		}
	}
}

// When inheriting and using Process, if position update or movement happens after CDnActor::Process, effect sync breaks, so
// Call ProcessActor(), and then call ProcessVisual after all position updates are complete.
void CDnActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	ProcessActor( LocalTime , fDelta );
	ProcessVisual( LocalTime, fDelta );
}

void CDnActor::ProcessActor( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActionBase::ProcessAction( LocalTime, fDelta );

	if( 0 < GetCantXZMoveSEReferenceCount() )
		m_vAniDistance.x = m_vAniDistance.z = 0.0f;

	OnSkillProcess( LocalTime, fDelta );

	ProcessDown( LocalTime, fDelta );
	ProcessDie( LocalTime, fDelta );
	ProcessStiff( LocalTime, fDelta );
	ProcessPlaySpeed( LocalTime, fDelta );
	ProcessState( LocalTime, fDelta );
	ProcessPress();
	ProcessSoftAppear( LocalTime );

	ProcessRotate( LocalTime, fDelta ); // Rotate.

	if (m_pStateBlowSignalProcessor && !IsDie())
		m_pStateBlowSignalProcessor->Process(LocalTime, fDelta);

	for( int i=0; i<CDnWeapon::EquipSlot_Amount; i++ )
	{
		if( m_hWeapon[i] ) 
			m_hWeapon[i]->Process( LocalTime, fDelta );
	}

#ifdef PRE_ADD_GRAVITY_PROPERTY
	if( m_fGravityEnd > 0.0f )
	{
		m_fGravityEnd -= fDelta;
		if( m_fGravityEnd <= 0.0f )
		{
			SetVelocityY( 0.01f );
			SetResistanceY( -30.0f );
			SetAppliedYDistance( false );
			m_fGravityEnd = 0.0f;
		}
	}
#endif // PRE_ADD_GRAVITY_PROPERTY

}

void CDnActor::ProcessVisual( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActionSignalImp::Process( LocalTime, fDelta );
	m_bHideExposureInfo[0] = IsSignalRange( STE_HideExposureInfo );

	CDnHeadEffectRender::Process( LocalTime, fDelta );

#ifdef _SHADOW_TEST
	if( !CGlobalValue::GetInstance().m_bEnableShadow ) 
	{
		if( m_hShadowActor ) 
			m_hShadowActor->Show( false );
		return;
	}
	else 
	{
		if( m_hShadowActor ) 
			m_hShadowActor->Show( true );
	}
	if( m_bEnableShadowActor && m_hShadowActor ) 
	{
		m_hShadowActor->Process( LocalTime, fDelta );
		m_hShadowActor->SetAlphaBlend( 0.5f );
	}
#endif
}

void CDnActor::SetPosition( EtVector3 &vPos )
{
	m_matexWorld.m_vPosition = vPos;
	m_vStaticPos = vPos;
}

void CDnActor::SetStaticPosition( EtVector3 &vPos )
{
	m_vStaticPos = vPos;
}

EtVector3 *CDnActor::GetStaticPosition()
{
	return &m_vStaticPos;
}

void CDnActor::SetPrevPosition( EtVector3 &vPos )
{
	m_vPrevPos = vPos;
}

EtVector3 *CDnActor::GetPrevPosition()
{
	return &m_vPrevPos;
}

void CDnActor::GetBoundingSphere( SSphere &Sphere, bool bActorSize/* = false*/ )
{
	MAActorRenderBase *pRenderBase = dynamic_cast<MAActorRenderBase *>(this);
	if( pRenderBase ) {
		if( pRenderBase->GetBoundingSphere( Sphere ) == false ) {
			Sphere.Center = m_matexWorld.m_vPosition;
			Sphere.fRadius = (float)GetUnitSize();
		}
	}
	else {
		Sphere.Center = m_matexWorld.m_vPosition;
		Sphere.fRadius = (float)GetUnitSize();
	}

	if (bActorSize = true)
	{
		Sphere.Center = FindAutoTargetPos();
		Sphere.fRadius = (float)GetUnitSize();
	}
}

void CDnActor::GetBoundingBox( SAABox &Box )
{
	MAActorRenderBase *pRenderBase = dynamic_cast<MAActorRenderBase *>(this);
	if( pRenderBase ) {
		if( pRenderBase->GetBoundingBox( Box ) == false ) {
			Box.Min = m_matexWorld.m_vPosition + EtVector3( -50.f, -50.f, -50.f );
			Box.Max = m_matexWorld.m_vPosition + EtVector3( 50.f, 50.f, 50.f );
		}
	}
	else {
		Box.Min = m_matexWorld.m_vPosition + EtVector3( -50.f, -50.f, -50.f );
		Box.Max = m_matexWorld.m_vPosition + EtVector3( 50.f, 50.f, 50.f );
	}
}

EtVector3 CDnActor::GetHeadPosition()
{
	EtVector3 vPos = m_matexWorld.m_vPosition;
	EtMatrix HeadMat = GetBoneMatrix("Bip01 Head");
	if(HeadMat._42 != 0.f ) {
		vPos = *(EtVector3*)&HeadMat._41;
	}
	else {
		SAABox box;
		GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y;
		vPos.y += fHeight;
	}
	return vPos;
}

EtVector3 CDnActor::GetBodyPosition()
{
	EtVector3 vPos = m_matexWorld.m_vPosition;
	EtMatrix HeadMat = GetBoneMatrix("Bip01");
	if(HeadMat._42 != 0.f ) {
		vPos = *(EtVector3*)&HeadMat._41;
	}
	else {
		SAABox box;
		GetBoundingBox(box);
		float fHeight = ( box.Max.y - box.Min.y ) / 2.f;
		vPos.y += fHeight;
	}
	return vPos;
}

#ifdef _SOCIAL_NEXT_ACTION_KALLISTE
void CDnActor::SetSocialCtrl(int nCondition, const char* szNextActionName)
{
	m_SocialActCtrler.Clear();
	m_SocialActCtrler.nCondition = nCondition;
	m_SocialActCtrler.szNextActionName = szNextActionName;
}
#endif

void CDnActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
#ifdef _SHADOW_TEST
	if( IsShadowActor() && Type != STE_MoveY ) return;
#endif _SHADOW_TEST

	CDnActionSignalImp::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );

	switch( Type ) {
		case STE_Hit:
			{
			}
			break;
		case STE_VelocityAccel:
			{
				VelocityAccelStruct *pStruct = (VelocityAccelStruct *)pPtr;

				MAMovementBase *pMovement = GetMovement();
				
				// #24949 Also block parts using the VelocityAccel signal.
				EtVector3 vVelocity = *pStruct->vVelocity;
				EtVector3 vResistance = *pStruct->vResistance;
				if( 0 < GetCantXZMoveSEReferenceCount() )
				{
					vVelocity.x = vVelocity.z = 0.0f;
					vResistance.x = vResistance.z = 0.0f;
				}

				if( pMovement )
				{
					pMovement->SetVelocity( vVelocity );
					pMovement->SetResistance( vResistance );
				}
			}
			break;
		case STE_State:
			{
				if( IsCustomProcessSignal() ) break;
				StateStruct *pStruct = (StateStruct *)pPtr;

				int nState = CDnActorState::s_nActorStateIndex[pStruct->nStateOne] | 
							CDnActorState::s_nActorStateIndex[pStruct->nStateTwo] | 
							CDnActorState::s_nActorStateIndex[pStruct->nStateThree];
				SetState( (ActorStateEnum)nState );
			}
			break;
		case STE_CustomState:
			{
				if( IsCustomProcessSignal() ) break;
				CustomStateStruct *pStruct = (CustomStateStruct *)pPtr;

				int nState = CDnActorState::s_nActorCustomStateIndex[pStruct->nStateOne] | 
					CDnActorState::s_nActorCustomStateIndex[pStruct->nStateTwo] | 
					CDnActorState::s_nActorCustomStateIndex[pStruct->nStateThree];
				SetCustomState( (ActorCustomStateEnum)nState );

			}
			break;

		case STE_CanHit:
			{
				CanHitStruct *pStruct = (CanHitStruct *)pPtr;
				SetHittable( ( pStruct->bHittable == TRUE ) ? true : false );

			}
			break;
		case STE_CanMove:
			{
				if( CDnActorState::Cant_Move != (m_StateEffect & CDnActorState::Cant_Move) )
				{
					CanMoveStruct *pStruct = (CanMoveStruct *)pPtr;
					SetMovable( ( pStruct->bCanMove == TRUE ) ? true : false );
				}
			}
			break;
		case STE_SendAction_Weapon:
			{
				SendAction_WeaponStruct *pStruct = (SendAction_WeaponStruct *)pPtr;

				if( CDnWeapon::EquipSlot_Amount == pStruct->nWeaponIndex )
					break;

				if( !GetActiveWeapon( pStruct->nWeaponIndex ) ) break;
				if( pStruct->szActionName ) {
					if( GetActiveWeapon( pStruct->nWeaponIndex )->IsExistAction( pStruct->szActionName ) ) {
						// If a weapon has its own animation like Sorceress weapons, blending it causes animation popping.
						// Mostly occurs in PullOut and PutIn animations, so set blend value to 0.0 here.
						// Even with this handling, animation mismatch pops can happen when cash and normal weapon types differ. Anim team says it is unavoidable.
						float fBlendFrame = 3.0f;
						if( strstr( GetActiveWeapon( pStruct->nWeaponIndex )->GetCurrentAction(), "_Weapon" ) ) fBlendFrame = 0.0f;
						GetActiveWeapon( pStruct->nWeaponIndex )->SetActionQueue( pStruct->szActionName, pStruct->nLoopCount, fBlendFrame );
					}
				}

			}
			break;
		case STE_EnvironmentEffect:
			{
				// #41767 For players, skip EnvironmentEfefct signal when stealth state effect is active.
				if( IsPlayerActor() &&
					IsAppliedThisStateBlow( STATE_BLOW::BLOW_073 ) )
					return;

				EnvironmentEffectStruct *pStruct = (EnvironmentEffectStruct *)pPtr;
				MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(this);
				if( !pRender ) break;

				MatrixEx Cross;
				if( strlen(pStruct->szBoneName) ) {
					EtMatrix mat = pRender->GetBoneMatrix( pStruct->szBoneName );
					memcpy( &Cross.m_vXAxis, &mat._11, sizeof(EtVector3) );
					memcpy( &Cross.m_vYAxis, &mat._21, sizeof(EtVector3) );
					memcpy( &Cross.m_vZAxis, &mat._31, sizeof(EtVector3) );
					memcpy( &Cross.m_vPosition, &mat._41, sizeof(EtVector3) );
				}
				else {
					Cross = m_matexWorld;
				}
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vOffset->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vOffset->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vOffset->z;
				Cross.RotateYaw( -pStruct->vRotate->y );
				Cross.RotatePitch( pStruct->vRotate->x );
				Cross.RotateRoll( pStruct->vRotate->z );

				ConditionEnum Condition = (ConditionEnum)pStruct->nCondition;
				float fHeight;
				TileTypeEnum TileType = CDnWorld::GetInstance().GetTileType( m_matexWorld.m_vPosition, &fHeight );

				if( TileType == TileTypeEnum::Water ) Cross.m_vPosition.y = fHeight;

				CreateEnviEffectParticle( Condition, TileType, &Cross );
				CreateEnviEffectSound( Condition, TileType, Cross.m_vPosition );
			}
			break;
		case STE_ShowWeapon:
			{
				ShowWeaponStruct *pStruct = (ShowWeaponStruct *)pPtr;

				ShowWeapon( pStruct->nEquipIndex, ( pStruct->bEnable == TRUE ) ? true : false );
			}
			break;
		case STE_AlphaBlending:
			{
				MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(this);
				AlphaBlendingStruct *pStruct = (AlphaBlendingStruct *)pPtr;
				float fValue = 1.f / ( SignalEndTime - SignalStartTime - 16.6666f ) * ( LocalTime - SignalStartTime );
				fValue = EtClamp( fValue, 0.0f, 1.0f );
				float fCurAlpha = pStruct->fStartAlpha + ( ( pStruct->fEndAlpha - pStruct->fStartAlpha ) * fValue );
				fCurAlpha = EtClamp( fCurAlpha, 0.0f, 1.0f );
				pRender->SetAlphaBlend( fCurAlpha ,  AL_SIGNAL );
			}
			break;
		case STE_CameraEffect_Shake:
			{
				if( !IsShow() ) break;
				CameraEffect_ShakeStruct *pStruct = (CameraEffect_ShakeStruct *)pPtr;
				DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
				if( !hCamera ) break;
				if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
				if( hCamera->FindCameraEffect( pStruct->nCameraEffectRefIndex ) ) break;

				if( s_hLocalActor ) {
					float fDistance = EtVec3Length( &EtVector3( *GetPosition() - *s_hLocalActor->GetPosition() ) );
					if( fDistance <= pStruct->fRange ) {
						float fRolloffDist = pStruct->fRange * pStruct->fRolloff;
						float fRatio = 1.f;

						if( fDistance > fRolloffDist ) 
							fRatio = 1.f - 1.f / ( pStruct->fRange - fRolloffDist ) * ( fDistance - fRolloffDist );

						float fLength = (float)( SignalEndTime - SignalStartTime );
						if( IsModifyPlaySpeed() ) {
							float fTemp = CDnActionBase::m_fFps / 60.f;
							fLength *= fTemp;
						}
						DWORD dwTime = (DWORD)( ( fLength - 16.6666f ) * fRatio );
						pStruct->nCameraEffectRefIndex = hCamera->Quake( dwTime, pStruct->fBeginRatio, pStruct->fEndRatio, pStruct->fShakeValue * fRatio );
					}
				}
			}
			break;
		case STE_CameraEffect_KeyFrame:
			{
				if( !IsShow() ) break;
				CameraEffect_KeyFrameStruct * pStruct = (CameraEffect_KeyFrameStruct *)pPtr;
				DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
				if( !hCamera ) break;
				if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
				if( hCamera->FindCameraEffect( pStruct->nCameraEffectRefIndex ) ) break;

				if( s_hLocalActor )
				{
					pStruct->nCameraEffectRefIndex = hCamera->KeyFrame( pStruct->strCamFile );
				}
			}
			break;
		case STE_CameraEffect_Movement:
			{
				if( !IsShow() ) break;
				CameraEffect_MovementStruct * pStruct = (CameraEffect_MovementStruct *)pPtr;
				DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
				if( !hCamera ) break;
				if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
				if( hCamera->FindCameraEffect( pStruct->nCameraEffectRefIndex ) ) break;

				if( s_hLocalActor )
				{
					float fLength = (float)( SignalEndTime - SignalStartTime );
					if( IsModifyPlaySpeed() )
					{
						float fTemp = CDnActionBase::m_fFps / 60.0f;
						fLength *= fTemp;
					}
					DWORD dwTime = (DWORD)( fLength - 16.6666f );
					pStruct->nCameraEffectRefIndex = hCamera->Movement( dwTime, *(pStruct->vDir), pStruct->fSpeedBegin, pStruct->fSpeedEnd );
				}
			}
			break;
		case STE_CameraEffect_Swing:
			{
				if( !IsShow() ) break;	
				CameraEffect_SwingStruct * pStruct = (CameraEffect_SwingStruct *)pPtr;
				DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
				if( !hCamera ) break;
				if( hCamera->GetCameraType() != CDnCamera::PlayerCamera ) break;
				if( hCamera->FindCameraEffect( pStruct->nCameraEffectRefIndex ) ) break;

				if( s_hLocalActor )
				{
					float fLength = (float)( SignalEndTime - SignalStartTime );
					if( IsModifyPlaySpeed() ){
						float fTemp = CDnActionBase::m_fFps / 60.0f;
						fLength *= fTemp;
					}
					DWORD dwTime = (DWORD)( fLength - 16.6666f );
					pStruct->nCameraEffectRefIndex = hCamera->Swing( dwTime, pStruct->fAngle, pStruct->fSpeed, pStruct->bSmooth );
				}
			}
			break;
		case STE_MotionSpeed:
			{
				if( IsModifyPlaySpeed() ) break;
				MotionSpeedStruct *pStruct = (MotionSpeedStruct *)pPtr;
				float fIncreadSpeed = 1.f / pStruct->fFrame * 60.f;
				DWORD dwTime = (DWORD)( ( SignalEndTime - SignalStartTime ) * fIncreadSpeed );
				float fSpeed = 1.f / 60.f * pStruct->fFrame;
				SetPlaySpeed( dwTime, fSpeed );
				OutputDebug( "Motion Speed : %d %d~%d : %d\n", nSignalIndex, (int)SignalStartTime, (int)SignalEndTime, (int)LocalTime );
			}
			break;
		case STE_ResetVelocity:
			{
				ResetVelocityStruct *pStruct = (ResetVelocityStruct *)pPtr;
				if( pStruct->bXAxis ) {
					SetVelocityX( 0.f );
					SetResistanceX( 0.f );
				}
				if( pStruct->bYAxis ) {
					SetVelocityY( 0.f );
					SetResistanceY( 0.f );
				}
				if( pStruct->bZAxis ) {
					SetVelocityZ( 0.f );
					SetResistanceZ( 0.f );
				}
			}
			break;
		case STE_Projectile:
			{
				if( CGlobalInfo::GetInstance().IsPlayingCutScene() )
					break;

				// In-game, all projectiles of the current player are sent from client to game server, so do not process projectile signals other than the player's own.
				// (Same for weapon projectile signal)
				if( GetActorType() <= CDnActorState::Reserved6 )
				{
					// For Charger state effect, projectile signals can execute based on another object rather than local,
					// Since it is not local, returning here would skip rendering effects or projectiles, so for rendering when triggered by a charger here
					// Render the projectile, but do not send a packet to the server.
					if( CDnActor::s_hLocalActor != GetMySmartPtr() && !m_bOnSignalFromChargerSE )
						break;
				}

				ProjectileStruct *pStruct = (ProjectileStruct *)pPtr;

				//////////////////////////////////////////////////////////////////////////
				//Make it fly to the position of the actor with the stigma effect.
				DNVector(DnActorHandle) StigmaActorList;
				if (pStruct->nTargetStateIndex != 0)
				{
					ScanActorByStateIndex(StigmaActorList, STATE_BLOW::BLOW_246);
				}

				//Process only when there is a brand target
				if (StigmaActorList.empty() == false)
				{
					int nStigmaActorCount = (int)StigmaActorList.size();
					for (int i = 0; i < nStigmaActorCount; ++i)
					{
						//Target actor.
						DnActorHandle hTargetActor = StigmaActorList[i];

						MatrixEx LocalCross = *GetMatEx();
						CDnProjectile *pProjectile = CDnProjectile::CreateProjectile( GetMySmartPtr(), LocalCross, pStruct, EtVector3(0.0f, 0.0f, 0.0f), hTargetActor );
						if( pProjectile == NULL ) break;
						pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

						if( m_bOnSignalFromChargerSE && m_hChargetDestActor )
						{
							DNVector(DnBlowHandle) vlhChargerBlows;
							m_hChargetDestActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_062, vlhChargerBlows );	// Better not to allow more than 1 duplicate, though depends on design.
							for( int i = 0; i < (int)vlhChargerBlows.size(); ++i )
								static_cast<CDnChargerBlow*>( vlhChargerBlows.at( i ).GetPointer() )->OnProjectileSignal( pProjectile );
						}

						// Charger state effect fires projectiles separately on server and client, so do not send packet to server.
						if( false == m_bOnSignalFromChargerSE )
						{
#ifdef PRE_MOD_PROJECTILE_HACK
							OnProjectile( pProjectile );
#else
							OnProjectile( pProjectile, nSignalIndex );
#endif
							OnSkillProjectile( pProjectile );
						}

						if( GetActorType() == PropActor )
						{
							CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>( this );
							pProjectile->SetShooterProp( pMonsterActor->GetPropHandle() );
						}

						pProjectile->ShowWeapon( IsShow() );
					}
				}
				//If there is no stigma target, process as before
				else
				{
					//#52808 - Additional request (if brand projectile setting is 2, do not create projectile when there is no branded target).
					if (pStruct->nTargetStateIndex == 2)
						break;

					MatrixEx LocalCross = *GetMatEx();
					CDnProjectile *pProjectile = CDnProjectile::CreateProjectile( GetMySmartPtr(), LocalCross, pStruct );
					if( pProjectile == NULL ) break;
					pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

					if( m_bOnSignalFromChargerSE && m_hChargetDestActor )
					{
						DNVector(DnBlowHandle) vlhChargerBlows;
						m_hChargetDestActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_062, vlhChargerBlows );	// Better not to allow more than 1 duplicate, though depends on design.
						for( int i = 0; i < (int)vlhChargerBlows.size(); ++i )
							static_cast<CDnChargerBlow*>( vlhChargerBlows.at( i ).GetPointer() )->OnProjectileSignal( pProjectile );
					}

					// Charger state effect fires projectiles separately on server and client, so do not send packet to server.
					if( false == m_bOnSignalFromChargerSE )
					{
#ifdef PRE_MOD_PROJECTILE_HACK
						OnProjectile( pProjectile );
#else
						OnProjectile( pProjectile, nSignalIndex );
#endif
						OnSkillProjectile( pProjectile );
					}

					if( GetActorType() == PropActor )
					{
						CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>( this );
						pProjectile->SetShooterProp( pMonsterActor->GetPropHandle() );
					}

					pProjectile->ShowWeapon( IsShow() );
				}
			}
			break;
		case STE_CanRotate:
			{
				CanRotateStruct *pStruct = (CanRotateStruct *)pPtr;
				SetRotatable( ( pStruct->bRotate == TRUE ) ? true : false );
			}
			break;
		case STE_ObjectVisible:
			{
				ObjectVisibleStruct *pStruct = (ObjectVisibleStruct *)pPtr;
				Show( ( pStruct->bShow == TRUE ) ? true : false );
			}
			break;
		case STE_ShowSwordTrail:
			{
				ShowSwordTrailStruct *pStruct = (ShowSwordTrailStruct *)pPtr;
				if( pStruct->nWeaponIndex < CDnWeapon::EquipSlot_Amount ) { // 0 , 1
					if( GetActiveWeapon( pStruct->nWeaponIndex ) )
						GetActiveWeapon( pStruct->nWeaponIndex )->ShowTrail( SignalEndTime - LocalTime, pStruct );
				}
				else if( pStruct->nWeaponIndex >= 4 ) {	// 4, 5, 6, 7

					EtTextureHandle hTex = ( pStruct->nTextureIndex != -1 ) ? CEtResource::GetSmartPtr(  pStruct->nTextureIndex ) : CEtTexture::Identity();
					EtTextureHandle hNormalTex = ( pStruct->nNormalTextureIndex != -1 ) ? CEtResource::GetSmartPtr(  pStruct->nNormalTextureIndex ) : CEtTexture::Identity();

					if( m_TrailList.count( pStruct->nWeaponIndex ) == 0 ) {
						float fLifeTime = ( pStruct->fLifeTime == 0.f ) ? 0.2f : pStruct->fLifeTime;
						float fMinSegment = ( pStruct->fMinSegment == 0.f ) ? 5.0f : pStruct->fMinSegment;
						m_TrailList[ pStruct->nWeaponIndex ] = EternityEngine::CreateSwordTrail( GetObjectHandle(), hTex, hNormalTex, fLifeTime, fMinSegment );						
					}
					else {						
						m_TrailList[ pStruct->nWeaponIndex ]->ChangeTexture( hTex, hNormalTex );						
					}
					m_TrailList[ pStruct->nWeaponIndex ]->SetBlendOP( (EtBlendOP)( pStruct->nBlendOP + 1 ) );
					m_TrailList[ pStruct->nWeaponIndex ]->SetSrcBlend( (EtBlendMode)( GetValue2ExceptionSwap( pStruct->nSrcBlend, 1, 0, 4 ) + 1 ) );
					m_TrailList[ pStruct->nWeaponIndex ]->SetDestBlend( (EtBlendMode)( GetValue2ExceptionSwap( pStruct->nDestBlend, 1, 0, 5 ) + 1 ) );
					m_TrailList[ pStruct->nWeaponIndex ]->AddPoint( pStruct->nWeaponIndex - 4, (int)(SignalEndTime - LocalTime) );
				}
			}
			break;
		case STE_ChangeWeaponLink:
			{
				if( CGlobalInfo::GetInstance().IsPlayingCutScene() )
				{
					ChangeWeaponLinkStruct* pStruct = (ChangeWeaponLinkStruct *)pPtr;
					if( pStruct->szLinkBoneName ) {
						if( GetActiveWeapon( pStruct->nWeaponIndex ) )
							GetActiveWeapon( pStruct->nWeaponIndex )->ChangeLinkBone( pStruct->szLinkBoneName );
					}
				}
			}
			break;
		case STE_SocialAction:
			{
				if( !IsShow() ) break;
				SocialActionStruct *pStruct = (SocialActionStruct *)pPtr;

				if( CGlobalInfo::GetInstance().IsPlayingCutScene() && (pStruct->nRandom < 100) )
					break;

				if( _rand()%100 < pStruct->nRandom ) 
				{
					SetFaceAction( pStruct->szActionName );
#ifdef _SOCIAL_NEXT_ACTION_KALLISTE
					if (pStruct->nCondition != 0)
						SetSocialCtrl(pStruct->nCondition, pStruct->szNextActionName);
					else
						m_SocialActCtrler.Clear();
#endif
				}
			}
			break;
		case STE_HeadLook:
			{
				HeadLookStruct *pStruct = (HeadLookStruct *)pPtr;
				OnSignalHeadLook( pStruct );
			}
			break;
		case STE_Destroy:
			{
				SetDestroy();
			}
			break;
		case STE_Gravity:
			{
				if( CDnWorld::GetInstance().GetMapType() == CDnWorld::MapTypeVillage ) break;
				GravityStruct *pStruct = (GravityStruct *)pPtr;

				MatrixEx CrossOffset = m_matexWorld;
				CrossOffset.MoveLocalXAxis( pStruct->vOffset->x );
				CrossOffset.MoveLocalYAxis( pStruct->vOffset->y );
				CrossOffset.MoveLocalZAxis( pStruct->vOffset->z );

				DNVector(DnActorHandle) hVecList;
				CDnActor::ScanActor( CrossOffset.m_vPosition, pStruct->fDistanceMax, hVecList );

#ifdef PRE_ADD_GRAVITY_PROPERTY
				EtVector3 vPos = CrossOffset.m_vPosition;

				float fXZDistanceMaxSQ = pStruct->fDistanceMax;
				float fXZDistanceMinSQ = pStruct->fDistanceMin;

				fXZDistanceMaxSQ *= fXZDistanceMaxSQ;
				fXZDistanceMinSQ *= fXZDistanceMinSQ;
	
				SAABox Box;
				float fDot = 0.0f;
				EtVector3 vZVec = m_matexWorld.m_vZAxis;

				if( pStruct->fCenterAngle != 0.f ) 
				{
					EtMatrix matRotate;
					EtMatrixRotationY( &matRotate, EtToRadian( pStruct->fCenterAngle ) );
					EtVec3TransformNormal( &vZVec, &vZVec, &matRotate );
				}
#endif // PRE_ADD_GRAVITY_PROPERTY

				for( DWORD i=0; i<hVecList.size(); i++ ) {
					if( !hVecList[i] ) continue;
					if( hVecList[i] == GetMySmartPtr() ) continue;
					if( hVecList[i]->IsDie() ) continue;
					if( hVecList[i]->GetWeight() == 0.f ) continue;
					if( hVecList[i]->IsNpcActor() ) continue;
					if( hVecList[i]->ProcessIgnoreGravitySignal() ) continue;

					bool bHittable = false;
					switch( pStruct->nTargetType ) {
						case 0: // Enemy
							if( hVecList[i]->GetTeam() == GetTeam() ) break;
							bHittable = true;
							break;
						case 1: // Friend
							if( hVecList[i]->GetTeam() != GetTeam() ) break;
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

#ifdef PRE_ADD_GRAVITY_PROPERTY
					DnActorHandle hActor = hVecList[i];
					EtVector3 vDirActorToPos;
					vDirActorToPos = *hActor->GetPosition() - vPos;
					vDirActorToPos.y = 0.f;

					hActor->GetBoundingBox( Box );

					bool bCheckArea = true;
					float fSQDistance = SquaredDistance( vPos, Box, true );

					if( fSQDistance > fXZDistanceMaxSQ ) 
						bCheckArea = false;

					if( fSQDistance <= fXZDistanceMinSQ )
						bCheckArea = false;

					if( pStruct->fAngle != 0.0f )
					{
						EtVec3Normalize( &vDirActorToPos, &vDirActorToPos );
						fDot = EtVec3Dot( &vZVec, &vDirActorToPos );
						if( EtToDegree( acos( fDot ) ) > pStruct->fAngle )
							bCheckArea = false;
					}

					if( pStruct->fHeightMax == 0.0f )
						pStruct->fHeightMax = 3000.0f;

					if( Box.Min.y < vPos.y + pStruct->fHeightMin && Box.Max.y < vPos.y + pStruct->fHeightMin ) 
						bCheckArea = false;

					if( Box.Min.y > vPos.y + pStruct->fHeightMax && Box.Max.y > vPos.y + pStruct->fHeightMax ) 
						bCheckArea = false;

					if( !bCheckArea )
					{
						hVecList[i]->SetVelocityZ( 0.0f );
						hVecList[i]->SetResistanceZ( 0.0f );
						hVecList[i]->ResetLook();
						continue;
					}
#endif // PRE_ADD_GRAVITY_PROPERTY

					MAMovementBase *pMovement = (hVecList[i]->GetMovement());
					if( !pMovement ) continue;

					EtVector3 vDir = *hVecList[i]->GetPosition() - CrossOffset.m_vPosition;
					float fDistance = EtVec3Length( &vDir );
					float fDistance2 = EtVec3Length( &EtVector3( vDir.x, 0.f, vDir.z ) );
					EtVec3Normalize( &vDir, &vDir );

#ifdef PRE_ADD_GRAVITY_PROPERTY
					float fDistanceMaxToMin = pStruct->fDistanceMax - pStruct->fDistanceMin;
					float fTemp = 0.0f;
					if( fDistanceMaxToMin > 0.0f )
						fTemp = pStruct->fMinVelocity + ( ( (pStruct->fMaxVelocity - pStruct->fMinVelocity) / fDistanceMaxToMin ) * ( fDistanceMaxToMin - fDistance ) );

					float fHeight = CrossOffset.m_vPosition.y - hVecList[i]->GetPosition()->y;
					float fHeightMaxToMin = pStruct->fHeightMax - pStruct->fHeightMin;
					float fTempVertical = 0.0f;
					if( fHeightMaxToMin > 0.0f )
						fTempVertical = pStruct->fMinVelocity_Vertical + ( ( (pStruct->fMaxVelocity_Vertical - pStruct->fMinVelocity_Vertical) / fHeightMaxToMin ) * ( fHeightMaxToMin - fHeight ) );
#else // PRE_ADD_GRAVITY_PROPERTY
					float fTemp = pStruct->fMinVelocity + ( ( (pStruct->fMaxVelocity - pStruct->fMinVelocity) / pStruct->fDistanceMax ) * ( pStruct->fDistanceMax - fDistance ) );
#endif // PRE_ADD_GRAVITY_PROPERTY
					EtVector3 vVel;
					if( pMovement ) 
					{
						EtVector3 vCurLook = *pMovement->GetLookDir();
						pMovement->Look( EtVec3toVec2(vDir), true );
						pMovement->SetVelocityZ( -fTemp );
						pMovement->SetResistanceZ( fTemp * 2.f );
#ifdef PRE_ADD_GRAVITY_PROPERTY
						if( fTempVertical > 0.0f )
						{
							pMovement->SetVelocityY( fTempVertical );
							pMovement->SetResistanceY( fTempVertical * 2.f );
							pMovement->SetAppliedYDistance( true );
							hVecList[i]->SetGravityEnd( 0.2f + ( float(SignalEndTime - SignalStartTime) / 1000.0f ) );
						}
#endif // PRE_ADD_GRAVITY_PROPERTY
						pMovement->Look( EtVec3toVec2(vCurLook), true );
					}
				}
			}
			break;
		case STE_MoveY:
			{
				if( m_pMovement )
				{
					// Slightly ambiguous, in ActionTool the MoveY signal has a duration range for designers to view easily, but 
					// Actually, the same direction per action is set only once, though internally it changes if the Y direction differs
					// Even if the function is repeatedly called toward Movement, the same direction is registered only once.
					MoveYStruct* pStruct = (MoveYStruct*)pPtr;
					float fYDelta = pStruct->fVelocityY * ((float(SignalEndTime - SignalStartTime) / 1000.0f));
					float fWholeMoveYDistance = (0.0f < fYDelta) ? fYDelta : -fYDelta;
					m_pMovement->SetMoveYDistancePerSec( pStruct->fVelocityY, fWholeMoveYDistance, (pStruct->bMaintainYPos == TRUE) ? true : false );
				}
			}
			break;
		// Signal Rotate.
		case STE_Rotate:
			{				
				RotateStruct * pStruct = static_cast< RotateStruct * >( pPtr );
				float fLength = (float)( SignalEndTime - SignalStartTime ) * (CDnActionBase::m_fFps / 60.0f); 
				DWORD dwTime = (DWORD)( fLength - 16.6666f );
				SetRotate( dwTime, pStruct->fStartSpeed, pStruct->fEndSpeed, *(pStruct->vAxis), pStruct->bLeft == TRUE ? true : false );
			}
			break;
		case STE_ChangeWeapon:
			{
				//Ignored if disarm state effect is active..
				if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_237))
					break;

				ChangeWeaponStruct* pStruct = (ChangeWeaponStruct*)pPtr;
				if (pStruct == NULL)
					break;

				DnWeaponHandle hNewWeapon = CDnWeapon::CreateWeapon(pStruct->nWeaponID, 0);
				if (hNewWeapon)
					AttachWeapon(hNewWeapon, pStruct->nWeaponIndex, true);
			}
			break;

	}
}

bool CDnActor::SetActionQueue( const char *szActionName, int nLoopCount, float fBlendFrame, float fStartFrame, bool bCheck, bool bCheckStateEffect )
{
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_224 ) )
	{
		DNVector(DnBlowHandle) vlhDisableActionSetBlow;
		m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_224, vlhDisableActionSetBlow );

		// There can be multiple action set change state effects.
		int iNumBlow = (int)vlhDisableActionSetBlow.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			CDnDisableActionBlow* pDisableActionSetBlow = static_cast<CDnDisableActionBlow*>( vlhDisableActionSetBlow.at( i ).GetPointer() );
			if(pDisableActionSetBlow->IsMatchedAction(szActionName))
				return false;
		}
	}

	const char *szName = szActionName;


	if( bCheck ) {
		if( m_szActionQueue.empty() ) {
			if( strcmp( szName, m_szAction.c_str() ) == NULL ) 
				return false;
		}
		else {
			if( strcmp( szName, m_szActionQueue.c_str() ) == NULL ) 
				return false;
		}
	}
	
	int nCurrentActionIndex = GetElementIndex( szName );
	if( bCheckStateEffect )
	{
		int nActionState = GetState( szName );

		// Subdivided in detail according to State
		// Cannot use actions in attack state when unable to attack.
		if( (nActionState & ActorStateEnum::Attack) && !( nActionState & ActorStateEnum::IgnoreCantAction) )
		{
			if( (GetStateEffect() & CDnActorState::Cant_AttackAction) == CDnActorState::Cant_AttackAction ) 
			{
				if( !IsDie() ) 
					return false;
			}
		}
	}


#ifdef PACKET_DELAY
	/*
	if( dynamic_cast<CDnLocalPlayerActor*>(this) == NULL )
		CDnActionBase::SetActionQueue( szName, nLoopCount, fBlendFrame, fStartFrame );
	else CDnActionBase::SetActionQueueDelay( szName, nLoopCount, fBlendFrame, fStartFrame, 100 );
	*/
	CDnActionBase::SetActionQueue( szName, nLoopCount, fBlendFrame, fStartFrame );
#else
  	CDnActionBase::SetActionQueue( szName, nLoopCount, fBlendFrame, fStartFrame );
#endif //PACKET_DELAY
	if( nCurrentActionIndex != -1 ) {
		for( int i=(int)m_VecPreActionState[nCurrentActionIndex].size()-1; i>=0; i-- ) {
			if( (int)fStartFrame >= m_VecPreActionState[nCurrentActionIndex][i].nOffset ) {
				SetState( (CDnActorState::ActorStateEnum)m_VecPreActionState[nCurrentActionIndex][i].nState );
				break;
			}
		}
		for( int i=(int)m_VecPreActionCustomState[nCurrentActionIndex].size()-1; i>=0; i-- ) {
			if( (int)fStartFrame >= m_VecPreActionCustomState[nCurrentActionIndex][i].nOffset ) {
				SetCustomState( (CDnActorState::ActorCustomStateEnum)m_VecPreActionCustomState[nCurrentActionIndex][i].nState );
				break;
			}
		}
		for( int i=(int)m_VecPreCanMoveState[nCurrentActionIndex].size()-1; i>=0; i-- ) {
			if( (int)fStartFrame >= m_VecPreCanMoveState[nCurrentActionIndex][i].nOffset ) {
				SetMovable( ( m_VecPreCanMoveState[nCurrentActionIndex][i].nState == TRUE ) ? true : false );
				break;
			}
		}
	}

	return true;
}

void CDnActor::SetCustomAction( const char *szActionName, float fFrame )
{
	const char *szName = szActionName;
	if( m_hToggleSkill && m_hToggleSkill->IsToggleOn() )
	{
		CDnChangeActionStrProcessor* pChangeAction = static_cast<CDnChangeActionStrProcessor*>( m_hToggleSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR ) );
		if( pChangeAction ) {
			szName = pChangeAction->GetChangeActionName( szActionName );
		}
	}
	CDnActionBase::SetCustomAction( szName, fFrame );
}

void CDnActor::ProcessDown( LOCAL_TIME LocalTime, float fDelta )
{
	if( IsDie() ) return;
	if( GetState() == CDnActorState::Down && m_fDownDelta > 0.f ) 
	{
		m_fDownDelta -= ( fDelta * ( ( 1.f / 60.f ) * CDnActionBase::GetFPS() ) );
		if( m_fDownDelta <= 0.f ) {
			m_fDownDelta = 0.f;
			SetActionQueue( "Down_StandUp", 0, 0.f, 0.f, true, false );
		}
	}
}

void CDnActor::ProcessStiff( LOCAL_TIME LocalTime, float fDelta )
{
	if( IsDie() ) return;

	if( IsStiff() ) {
		if( m_fStiffDelta == 0.f ) return;
		m_fStiffDelta -= ( fDelta * ( ( 1.f / 60.f ) * CDnActionBase::GetFPS() ) );
		if( m_fStiffDelta <= 0.f ) {
			m_fStiffDelta = 0.f;
			ActionElementStruct *pStruct = GetElement( m_szAction.c_str() );
			if( pStruct ) SetActionQueue( pStruct->szNextActionName.c_str(), 0, 3.f );
			else SetActionQueue( "Stand", 0, 3.f );
		}
	}
	else m_fStiffDelta = 0.f;
}

void CDnActor::ProcessDie( LOCAL_TIME LocalTime, float fDelta )
{
	if( !IsDie() ) return;

	m_fDieDelta -= fDelta;
	if( m_fDieDelta <= 0.f ) {
		SetDestroy();
		return;
	}
}

void CDnActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/  )
{
	SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, bCheckOverlapAction );
}

bool CDnActor::_CheckEffectDuplicate( DnSkillHandle hNewSkill )
{
	bool bResult = true;

	DNTableFileFormat*  pSkillTable = GetDNTable( CDnTableDB::TSKILL );
	DNTableFileFormat*  pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );

	// TOOD: Apply effect output distinction according to output distinction index
	// Check all currently running StateBlow instances
	int iNumStateBlow = m_pStateBlow->GetNumStateBlow();

	for( int iBlow = 0; iBlow < iNumStateBlow; ++iBlow )
	{
		DnBlowHandle hExistingBlow = m_pStateBlow->GetStateBlow( iBlow );

		int iExistingSkillID = hExistingBlow->GetParentSkillInfo()->iSkillID;
		int iExistingSkillLevelID = hExistingBlow->GetParentSkillInfo()->iSkillLevelID;
		if( iExistingSkillID < 1 || iExistingSkillLevelID < 1 ) continue;

		int iExistingEffectDuplicateMethod = pSkillTable->GetFieldFromLablePtr( iExistingSkillID, "_EffectDuplicate" )->GetInteger();

		int iEffectDuplicateMethod = hNewSkill->GetEffectDuplicateMethod();

		if( 0 != iEffectDuplicateMethod &&
			iEffectDuplicateMethod == iExistingEffectDuplicateMethod )
		{
			// Need to replace effect output.
			// Removing the existing effect output here; the caller will add the new effect.
			STATE_BLOW::emBLOW_INDEX ExistingBlowIndex = hExistingBlow->GetBlowIndex();

			bool bValid = TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + ExistingBlowIndex, hExistingBlow->GetBlowID() );
			assert( bValid && "Blow Effect is currently displayed on screen, but missing from TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>." );
			if( bValid )
				TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + ExistingBlowIndex, hExistingBlow->GetBlowID() );
		}
	}

	return bResult;
}



bool CDnActor::AttachSEEffect( const CDnSkill::SkillInfo& ParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nIndex,
							   const EffectOutputInfo* pEffectInfo, /*OUT*/ DnEtcHandle& hEtcObject, /*OUT*/ DNVector(DnEtcHandle)& vlhDummyBoneEtcObjects,
							   bool bAllowSameSkillDuplicate/* = false*/ )
{
	bool bResult = false;

	std::vector<std::string> infoTokens1;
	std::vector<std::string> infoTokens2;
	std::string delimiters = ";";

	TokenizeA(ParentSkillInfo.effectOutputIDs, infoTokens1, delimiters);
	TokenizeA(ParentSkillInfo.debuffEffectOutputIDs, infoTokens2, delimiters);
	
	if (infoTokens1.size() > 1 || infoTokens2.size() > 2)
	{
		bAllowSameSkillDuplicate = true;
	}

	switch( pEffectInfo->iOutputType )
	{
		case EffectOutputInfo::ATTACH:
			{
				if( TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex ) ) {
					TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex );
					m_setNowPlayingEffectSkill.erase( ParentSkillInfo );
				}

				if( bAllowSameSkillDuplicate || m_setNowPlayingEffectSkill.find( ParentSkillInfo ) == m_setNowPlayingEffectSkill.end() )
				{
					hEtcObject = (new CDnEtcObject)->GetMySmartPtr();
					if( hEtcObject->Initialize( pEffectInfo->strSkinFileName.c_str(), pEffectInfo->strAniFileName.c_str(), pEffectInfo->strActFileName.c_str() ) == false ) {
						SAFE_RELEASE_SPTR( hEtcObject );
					}
					else 
					{
						*hEtcObject->GetMatEx() = m_matexWorld;

						// If Summon_On action exists, execute it.
						if( hEtcObject->IsExistAction( "Summon_On" ) )
							hEtcObject->SetActionQueue( "Summon_On" );
						else
							hEtcObject->SetActionQueue( "Idle" );

						hEtcObject->AsStateEffect();		// Used as a state effect visual
						hEtcObject->SetParentActor( GetMySmartPtr() );
						EtcObjectSignalStruct *pResult = TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex, hEtcObject );
						pResult->bDefendenceParent = true;
						pResult->bLinkObject = true;
						pResult->vOffset = EtVector3( 0.f, -50.f, 0.f );
						pResult->vRotate = EtVector3( 0.f, 0.f, 0.f );
						sprintf_s( pResult->szBoneName, "Bip01" );

						m_setNowPlayingEffectSkill.insert( ParentSkillInfo );						
						bResult = true;
					}
				}
			}
			break;

		case EffectOutputInfo::DUMMY_BONE_ATTACH:
			{	
				if( bAllowSameSkillDuplicate || m_setNowPlayingEffectSkill.find( ParentSkillInfo ) == m_setNowPlayingEffectSkill.end() )
				{
					//bool bAttached = false;
					int nNumBone = (int)pEffectInfo->vlDummyBoneIndices.size();
					for( int nBone = 0; nBone < nNumBone; ++nBone )
					{
						if( TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone ) ) 
						{
							TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone );
							m_setNowPlayingEffectSkill.erase( ParentSkillInfo );
						}

						int iBoneIndex = (int)pEffectInfo->vlDummyBoneIndices.at( nBone );
						_ASSERT( iBoneIndex < Max_FX_Dummy_Bone );
						if( iBoneIndex >= Max_FX_Dummy_Bone )
						{
							OutputDebug( "[CDnActor::AttachSEEffect] Failed to attach state effect to dummy bone. Check bone index! BlowIndex: %d, Current: %d, Max: %d\n", 
										 emBlowIndex, iBoneIndex, Max_FX_Dummy_Bone );

							continue;
						}

						// Attach this effect if this dummy bone exists on this actor; otherwise do not attach.
						const char* pDummyBoneName = s_acEffectBoneName[ iBoneIndex ];
						bool bExistBone = true;
						GetBoneMatrix( pDummyBoneName, &bExistBone );
						if( false == bExistBone )
						{
							OutputDebug( "[CDnActor::AttachSEEffect] Failed to attach state effect to dummy bone %s. Check bone index! BlowIndex: %d, Current: %d\n", 
										  pDummyBoneName, emBlowIndex, iBoneIndex );

							continue;
						}

						hEtcObject = (new CDnEtcObject)->GetMySmartPtr();
						if( hEtcObject->Initialize( pEffectInfo->strSkinFileName.c_str(), pEffectInfo->strAniFileName.c_str(), pEffectInfo->strActFileName.c_str() ) == false ) {
							SAFE_RELEASE_SPTR( hEtcObject );
							continue;
						}
						else 
						{
							*hEtcObject->GetMatEx() = m_matexWorld;
							
							// If Summon_On action exists, execute it.
							if( hEtcObject->IsExistAction( "Summon_On" ) )
								hEtcObject->SetActionQueue( "Summon_On" );
							else
								hEtcObject->SetActionQueue( "Idle" );

							hEtcObject->AsStateEffect();		// Used as a state effect visual
							hEtcObject->SetParentActor( GetMySmartPtr() );
							EtcObjectSignalStruct *pResult = TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, 
																																		 (nIndex*100)+nBone, hEtcObject );

							pResult->bDefendenceParent = true;
							pResult->bLinkObject = true;
							pResult->vOffset = EtVector3( 0.f, 0.f, 0.f );
							pResult->vRotate = EtVector3( 0.f, 0.f, 0.f );
							_ASSERT( nBone < (int)pEffectInfo->vlDummyBoneIndices.size() );
							_strcpy( pResult->szBoneName, _countof(pResult->szBoneName), s_acEffectBoneName[ iBoneIndex ], (int)strlen(s_acEffectBoneName[ iBoneIndex ]));

							// Buff items used on the current player character have their effects configured to play only once and not repeat. (#11171)
							if( false == IsPlayerActor() )
								m_pStateBlow->AddEffectAttachedBone( pEffectInfo->vlDummyBoneIndices.at(nBone), hEtcObject );

							bResult = true;
						}

						if( hEtcObject )
							vlhDummyBoneEtcObjects.push_back( hEtcObject );
					}

					if( bResult )
						m_setNowPlayingEffectSkill.insert( ParentSkillInfo );
				}
			}
			break;
	}

	return bResult;
}



bool CDnActor::DetachSEEffect( const CDnSkill::SkillInfo& ParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nIndex, 
							   const EffectOutputInfo* pEffectInfo )
{
	bool bResult = false;

	switch( pEffectInfo->iOutputType )
	{
		case EffectOutputInfo::ATTACH:
			if( TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex ) ) 
			{
				DnEtcHandle hEffectHandle = TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::GetSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex );
				if( hEffectHandle ) {
					if( hEffectHandle->IsExistAction( "Summon_Off" ) )
						hEffectHandle->SetActionQueue( "Summon_Off" );		// Destroy signal must exist for the object to be destroyed 
					else
						TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex );
				}
				else 
					TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, nIndex );

				m_setNowPlayingEffectSkill.erase( ParentSkillInfo );
				bResult = true;
			}
			break;

		case EffectOutputInfo::DUMMY_BONE_ATTACH:
			{
				int nNumBone = (int)pEffectInfo->vlDummyBoneIndices.size();
				for( int nBone = 0; nBone < nNumBone; ++nBone )
				{
					if( TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone ) ) 
					{
						DnEtcHandle hEffectHandle = TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::GetSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone );

						// Buff items used on the current character have their effects configured to play only once and not repeat. (#11171)
						if( false == IsPlayerActor() )
							m_pStateBlow->DelEffectAttachedBone( pEffectInfo->vlDummyBoneIndices.at(nBone), hEffectHandle );

						if( hEffectHandle ) {
							if( hEffectHandle->IsExistAction( "Summon_Off" ) )
								hEffectHandle->SetActionQueue( "Summon_Off" );	// Destroy signal must exist for the object to be destroyed 
							else
								TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone );
						}
						else 
							TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + emBlowIndex, (nIndex*100)+nBone );

						bResult = true;
					}
				}

				if( bResult )
					m_setNowPlayingEffectSkill.erase( ParentSkillInfo );
			}
			break;
	}

	return bResult;
}


bool CDnActor::DetachSEEffectSkillInfo( const CDnSkill::SkillInfo& ParentSkillInfo )
{
	bool bExist = (m_setNowPlayingEffectSkill.find( ParentSkillInfo ) != m_setNowPlayingEffectSkill.end());
	if( bExist )
		m_setNowPlayingEffectSkill.erase( ParentSkillInfo );

	return bExist;
}


DnBlowHandle CDnActor::CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit /*= false*/ )
{
	DnBlowHandle hAddedBlow;

	int nIndex = AddStateBlow( pParentSkillInfo, emBlowIndex, nDurationTime, szParam, bOnPlayerInit );
	
	if( nIndex != -1 )
		hAddedBlow = GetStateBlowFromID( nIndex );
	
	//if( nIndex != -1 && hHitter ) {
	//	hAddedBlow = GetStateBlowFromID( nIndex );

		//DnSkillHandle hSkill = hHitter->FindSkill( pParentSkillInfo->iSkillID );
		//if( !hSkill ) return hAddedBlow;

		//bool bShowEffect = _CheckEffectDuplicate( hSkill );

		//if( bShowEffect )
		//{
		//	DnEtcHandle hHandle = AttachSEEffect( *hSkill->GetInfo(), emBlowIndex, nIndex, hSkill->GetEffectSkinName(), hSkill->GetEffectAniName(), hSkill->GetEffectActName() );
		//	//_ASSERT( hHandle && "Failed to process Effect action file for state effect" );
		//}
	//}

	// [2011/03/18 semozz]
	// Force Begin state effect
	if ( pParentSkillInfo && pParentSkillInfo->bItemPrefixSkill )
		ForceBeginStateBlow(hAddedBlow);

	return hAddedBlow;
}

void CDnActor::CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bSendServer/* = false*/ )
{
	RemoveStateBlowByBlowDefineIndex( emBlowIndex );
}

void CDnActor::CmdRemoveStateEffectImmediately( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	m_pStateBlow->RemoveImediatlyStateEffectByBlowIndex( emBlowIndex );
}

void CDnActor::CmdMixedAction( const char *szActionBone, const char *szMaintenanceBone, const char *szActionName, int nLoopCount, float fFrame, float fBlendFrame )
{
	const char *szName = szActionName;
	if( m_hToggleSkill && m_hToggleSkill->IsToggleOn() )
	{
		CDnChangeActionStrProcessor* pChangeAction = static_cast<CDnChangeActionStrProcessor*>( m_hToggleSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR ) );
		if( pChangeAction ) {
			szName = pChangeAction->GetChangeActionName( szActionName );
		}
	}

	int nElementIndex = GetElementIndex( szName );
	if( nElementIndex == -1 ) return;
	int nAniIndex = m_nVecAniIndexList[nElementIndex];
	if( nAniIndex == -1 ) return;

	int nNextActionBlendFrame = GetElement(nElementIndex)->dwBlendFrame;
	AddMixedAnimation( szActionBone, szMaintenanceBone, nAniIndex, fFrame, fBlendFrame, (float)nNextActionBlendFrame, nLoopCount );
}

void CDnActor::OnBeginStateBlow( DnBlowHandle hBlow )
{
}

void CDnActor::OnEndStateBlow( DnBlowHandle hBlow )
{
	//// Exclude freeze state effect because it has a separate shatter action
	//if( STATE_BLOW::BLOW_041 == hBlow->GetBlowIndex() )
	//	return;

	//if( TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::IsExistSignalHandle( STATEBLOWEFFECT_ETCOFFSET + hBlow->GetBlowIndex(), hBlow->GetBlowID() ) )
	//{
	//	TSmartPtrSignalImp<DnEtcHandle,EtcObjectSignalStruct>::RemoveSignalHandle( STATEBLOWEFFECT_ETCOFFSET + hBlow->GetBlowIndex(), hBlow->GetBlowID() );
	//	//m_setNowPlayingEffectSkill.erase( *hBlow->GetParentSkillInfo() );
	//}
}

#ifdef PRE_FIX_77172
void CDnActor::CheckDamageVelocity( DnActorHandle hActor, bool bKeepJumpMovement )
#else // PRE_FIX_77172
void CDnActor::CheckDamageVelocity( DnActorHandle hActor )
#endif // PRE_FIX_77172
{
	MAMovementBase *pMovement = GetMovement();

	EtVector2 vViewVec = EtVec3toVec2( m_HitParam.vViewVec );
	EtVec2Normalize( &vViewVec, &vViewVec );
	if( EtVec2LengthSq( &vViewVec ) > 0.f ) {
		pMovement->Look( vViewVec );
	}

#ifdef PRE_FIX_77172
	EtVector2 vJumpMovement = EtVector2( 0.0f, 0.0f );
	if( bKeepJumpMovement && pMovement->GetJumpMovement() )
	{
		vJumpMovement.x = pMovement->GetJumpMovement()->x;
		vJumpMovement.y = pMovement->GetJumpMovement()->y;
	}
#endif // PRE_FIX_77172

#ifdef PRE_FIX_STUCK_LANDING_ACTION
	if( m_HitParam.vVelocity.y == 0.f && pMovement->GetVelocity() && pMovement->GetVelocity()->y != 0.f && 
		( GetAddHeight() != 0.f ) || ( m_HitParam.szActionName.empty() && ( GetState() & ActorStateEnum::Air ) ) ) 
	{
		m_HitParam.vVelocity.y = pMovement->GetVelocity()->y;
		m_HitParam.vResistance.y = pMovement->GetResistance()->y;
	}
#endif

	pMovement->SetVelocity( m_HitParam.vVelocity );
	pMovement->SetResistance( m_HitParam.vResistance );

#ifdef PRE_FIX_77172
	if( bKeepJumpMovement )
		pMovement->SetJumpMovement( vJumpMovement );
#endif // PRE_FIX_77172
}

void CDnActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam )
{
	if( IsDie() ) return;

	SetHP( HitParam.nCurrentHP );

	DnActorHandle hHitter = pHitter ? pHitter->GetActorHandle() : CDnActor::Identity();
	m_HitParam = HitParam;
	OnDamageActor( hHitter, HitParam );

	if( GetHP() < 0 ) 
		SetHP( 0 );
}

void CDnActor::OnDamageActor( DnActorHandle hHitter, SHitParam &HitParam )
{
	if( !HitParam.szActionName.empty() )
		SetActionQueue( HitParam.szActionName.c_str(), 0, 3.f, 0.f, false );
	switch( HitParam.HitType ) {
		case CDnWeapon::Normal:
		case CDnWeapon::Critical:
		case CDnWeapon::CriticalRes:
		case CDnWeapon::Stun:
			if( HitParam.bSuccessNormalDamage ) 
			{
#ifdef PRE_FIX_77172
				CheckDamageVelocity( hHitter, HitParam.bKeepJumpMovement );
#else // PRE_FIX_77172
				CheckDamageVelocity( hHitter );
#endif // PRE_FIX_77172
				MAMovementBase *pMovement = GetMovement();
				if( pMovement ) {
					pMovement->ResetMove();
					pMovement->ResetLook();
				}
			}
			else 
			{
				if( m_nSuperAmmorTime > 0 ) 
				{
					SetPlaySpeed( (DWORD)( m_nSuperAmmorTime * ( m_fStiffDelta / s_fMaxStiffTime ) ), 0.03f );
					if( m_HitParam.hWeapon && m_HitParam.hWeapon->GetEquipType() != CDnWeapon::Arrow )
					{
						if( hHitter )
						{
#ifdef PRE_FIX_HITSTIFF
							// Frame-change state effect takes priority, so ignore SetPlaySpeed() while the state effect is active.
							if( false == (0 < hHitter->GetFrameStopRefCount() || hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 )) )
							{
								hHitter->SetPlaySpeed( 100, 0.03f );
							}
#else
							hHitter->SetPlaySpeed( 100, 0.03f );
#endif // #ifdef PRE_FIX_HITSTIFF
						}
					}
				}
			}

#if defined(PRE_FIX_44884)
			//When processed via RequestHPMPDelta, OnDie is called first and state effects get removed.
			//After calling OnDie on the state effect so that OnDie of UsingSkillWhenDieBlow gets called
			if( IsDie() )
			{
				m_pStateBlow->OnDie( hHitter );				
			}

			//Modified to verify actual death before calling OnDie
			if (IsDie())
			{
				OnDie( hHitter );
			}
#else
			if( IsDie() )
			{
				OnDie( hHitter );
				m_pStateBlow->OnDie( hHitter );
			}
#endif // PRE_FIX_44884
			break;
		case CDnWeapon::Defense:
			if( m_HitParam.hWeapon && m_HitParam.hWeapon->GetEquipType() != CDnWeapon::Arrow ) 
			{
				if( hHitter )
				{
					// Frame-change state effect takes priority, so ignore SetPlaySpeed() while the state effect is active.
					if( false == (0 < hHitter->GetFrameStopRefCount() || hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 )) )
					{
						hHitter->SetPlaySpeed( 100, 0.03f );
					}
				}
			}

			CheckDamageVelocity( hHitter );
			break;
	}
	// Call to handle weapon sound, particles, etc.
	if( m_HitParam.hWeapon ) 
	{
		m_HitParam.hWeapon->OnHitSuccess( GetMySmartPtr(), m_HitParam.HitType, m_HitParam.vPosition, m_HitParam.bFirstHit );

		if( HitParam.hWeapon->GetWeaponType() & CDnWeapon::Projectile ) {
			CDnProjectile *pProjectile = dynamic_cast<CDnProjectile *>(HitParam.hWeapon.GetPointer());
			if( pProjectile ) pProjectile->OnDamageSuccess( GetMySmartPtr(), m_HitParam );
			else HitParam.hWeapon->SetDestroy();
		}
	}

	// Cancel skill if hit while casting. 
	// If a hit right before skill start on the server arrives during skill execution on the client, the skill executes fine on the server and only on the client the skill action 
	// Since it gets canceled, store whether a skill was in use from the server hit packet and handle it.
	if( false == HitParam.szActionName.empty() )
	{
		if( IsProcessSkill() )
		{
			// During aura skill cast.. i.e. if hit before all state effects are applied, the aura skill was not fully used
			if( IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn() )
			{
				OnSkillAura( m_hAuraSkill, false );
				m_hProcessSkill.Identity();
			}
			else
			{
				m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
				m_hProcessSkill.Identity();
			}
		}
	}

	if( hHitter )
	{
		if( hHitter->IsMonsterActor() )
		{
			DnActorHandle hSummonMasterPlayerActor = static_cast<CDnMonsterActor*>(hHitter.GetPointer())->GetSummonerPlayerActor();
			if( hSummonMasterPlayerActor )
				hHitter = hSummonMasterPlayerActor;

		}
		hHitter->OnHitSuccess( CDnActionBase::m_LocalTime, GetMySmartPtr() );
	}

#ifndef _FINAL_BUILD
	if( CGlobalValue::IsActive() && CGlobalValue::GetInstance().m_bShowDamage == true )
	{
		TCHAR *szTemp[ CDnWeapon::HitTypeEnum::Count ] = { L"Normal", L"Critical", L"Stun", L"Defense", L"CriticalRes" };
		std::wstring wzDebugMessage = FormatW( L"%s - Damage(%I64d), Stiff(%.2f), Down(%.2f), %s\n", GetName(), HitParam.nDamage, m_fStiffDelta, m_fDownDelta, szTemp[HitParam.HitType] );
		GetInterface().AddChatMessage(CHATTYPE_NORMAL, L"Debug", wzDebugMessage.c_str(), true, false, true, D3DCOLOR_XRGB(255,255,255), D3DCOLOR_ARGB(128, 0, 0, 255) );
	}
#endif
}

bool CDnActor::Compare_PreStateOrder( CDnActor::PreStateStruct &a, CDnActor::PreStateStruct &b )
{
	return ( a.nOffset < b.nOffset ) ? true : false;
}


bool CDnActor::LoadAction( const char *szFullPathName )
{
	bool bResult = CDnActionSignalImp::LoadAction( szFullPathName );
	if( !bResult ) return false;

	PreCalcStateList();

	return bResult;
}

void CDnActor::PreCalcStateList()
{
	int nValue = 0;
	std::vector<PreStateStruct> VecList;
	CEtActionSignal *pSignal;
	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		// STE_State
		VecList.clear();
		for( int j=0; ; j++ ) {
			nValue = 0;
			pSignal = GetSignal( i, STE_State, j );
			if( pSignal ) {
				StateStruct *pStruct = (StateStruct *)pSignal->GetData();
				nValue = CDnActorState::s_nActorStateIndex[pStruct->nStateOne] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateTwo] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateThree];
			}
			else break;

			VecList.push_back( PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		if( VecList.empty() ) VecList.push_back( PreStateStruct( nValue, 0 ) );
		else std::sort( VecList.begin(), VecList.end(), Compare_PreStateOrder );

		m_VecPreActionState.push_back( VecList );

		// STE_CustomState
		VecList.clear();
		for( int j=0; ; j++ ) {
			nValue = 0;
			pSignal = GetSignal( i, STE_CustomState, j );
			if( pSignal ) {
				CustomStateStruct *pStruct = (CustomStateStruct *)pSignal->GetData();
				nValue = CDnActorState::s_nActorStateIndex[pStruct->nStateOne] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateTwo] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateThree];
			}
			else break;

			VecList.push_back( PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		if( VecList.empty() ) VecList.push_back( PreStateStruct( nValue, 0 ) );
		else std::sort( VecList.begin(), VecList.end(), Compare_PreStateOrder );

		m_VecPreActionCustomState.push_back( VecList );

		// STE_CanMove
		VecList.clear();
		for( int j=0; ; j++ ) {
			nValue = 0;
			pSignal = GetSignal( i, STE_CanMove, j );
			if( pSignal ) {
				CanMoveStruct *pStruct = (CanMoveStruct*)pSignal->GetData();
				nValue = pStruct->bCanMove;
			}
			else break;

			VecList.push_back( PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		if( VecList.empty() ) VecList.push_back( PreStateStruct( nValue, 0 ) );
		else std::sort( VecList.begin(), VecList.end(), Compare_PreStateOrder );

		m_VecPreCanMoveState.push_back( VecList );
	}
}

void CDnActor::FreeAction()
{
	CDnActionSignalImp::FreeAction();
	SAFE_DELETE_VEC( m_VecPreActionState );
	SAFE_DELETE_VEC( m_VecPreActionCustomState );
	SAFE_DELETE_VEC( m_VecPreCanMoveState );
}

void CDnActor::LinkWeapon( int nEquipIndex )
{
	switch( m_hWeapon[nEquipIndex]->GetEquipType() ) {
		case CDnWeapon::Sword:
		case CDnWeapon::Axe:
		case CDnWeapon::Hammer:
		case CDnWeapon::Staff:
		case CDnWeapon::Book:
		case CDnWeapon::Orb:
		case CDnWeapon::Puppet:
		case CDnWeapon::Mace:
		case CDnWeapon::Flail:
		case CDnWeapon::Wand:
		case CDnWeapon::Shield:
		case CDnWeapon::Cannon:
		case CDnWeapon::BubbleGun:
		case CDnWeapon::Chakram:
		case CDnWeapon::Fan:
		case CDnWeapon::Scimiter:
		case CDnWeapon::Dagger:
		case CDnWeapon::Crook:
		case CDnWeapon::Spear:
		//case CDnWeapon::KnuckleGear:
		//case CDnWeapon::LE_UNK:
		//case CDnWeapon::MC_UNK:
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex );
			break;
		case CDnWeapon::Gauntlet:
		case CDnWeapon::Glove:
		case CDnWeapon::Charm:
		case CDnWeapon::Bracelet:
		case CDnWeapon::Claw:
		case CDnWeapon::KnuckleGear:
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex, "" );
			break;
		case CDnWeapon::SmallBow:
		case CDnWeapon::BigBow:
		case CDnWeapon::CrossBow:
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex );
			if( nEquipIndex == 0 && m_hWeapon[0] && m_hWeapon[1] ) {
				m_hWeapon[1]->LinkWeapon( GetMySmartPtr(), m_hWeapon[0] );
			}
			break;
		case CDnWeapon::Arrow:
			if( !m_hWeapon[0] ) break;
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), m_hWeapon[0] );
			break;
	}
}

void CDnActor::AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex, bool bDelete )
{
	if (nEquipIndex < 0 || nEquipIndex >= CDnWeapon::EquipSlot_Amount)
		return;

#if defined(PRE_ADD_50907)
	if (GetDisarmamentRefCount() > 0 )
	{
		if (IsChangeWeaponLock() == true)
		{
			//If weapon equip occurs while weapon unequip state effect is active, skip changing at this point,
			//Update OrigWeapon weapon info.
			SetOrigWeaponWhenDisarmament(hWeapon, bDelete);
			return;
		}
	}
#endif // PRE_ADD_50907

	if( m_hWeapon[nEquipIndex] != hWeapon ) {
		if( m_hWeapon[nEquipIndex] ) {
			DetachWeapon( nEquipIndex );
		}
		m_hWeapon[nEquipIndex] = hWeapon;
		m_bSelfDeleteWeapon[nEquipIndex] = bDelete;
		if( m_hWeapon[nEquipIndex] ) m_hWeapon[nEquipIndex]->CreateObject( this );
	}

	if( !m_hWeapon[nEquipIndex] ) return;
	if( m_hWeapon[nEquipIndex]->IsExistAction( "Idle" ) ) m_hWeapon[nEquipIndex]->SetAction( "Idle", 0.f, 0.f );
	LinkWeapon( nEquipIndex );

#if defined(PRE_ADD_50907)
	if (IsSkipOnAttatchDetachWeapon() == true)
		return;
#endif // PRE_ADD_50907

	MASkillUser::OnAttachWeapon( m_hWeapon[ nEquipIndex ], nEquipIndex );
}

void CDnActor::DetachWeapon( int nEquipIndex )
{
	if( !m_hWeapon[nEquipIndex] ) return;

#if defined(PRE_ADD_50907)
	if (GetDisarmamentRefCount() > 0)
	{
		if (IsChangeWeaponLock() == true)
			return;
	}
#endif // PRE_ADD_50907

	m_hWeapon[nEquipIndex]->FreeObject();
	m_hWeapon[nEquipIndex]->UnlinkWeapon();

	if( m_bSelfDeleteWeapon[nEquipIndex] ) {
		SAFE_RELEASE_SPTR( m_hWeapon[nEquipIndex] );
		m_bSelfDeleteWeapon[nEquipIndex] = false;
	}
	m_hWeapon[nEquipIndex].Identity();

#if defined(PRE_ADD_50907)
	if (IsSkipOnAttatchDetachWeapon() == true)
		return;
#endif // PRE_ADD_50907

	MASkillUser::OnDetachWeapon( m_hWeapon[ nEquipIndex ], nEquipIndex );
}

void CDnActor::ShowWeapon( int nEquipIndex, bool bShow )
{
	if( m_hWeapon[nEquipIndex] )
		m_hWeapon[nEquipIndex]->ShowWeapon( bShow );
}

void CDnActor::ReleaseWeaponSignalImp()
{
	for( DWORD i=0; i<CDnWeapon::EquipSlot_Amount; i++ ) {
		if( m_hWeapon[i] ) {
			m_hWeapon[i]->ReleaseSignalImp();
			m_hWeapon[i]->ResetDefaultAction( i );
		}
	}
}

bool CDnActor::IsStay( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stay ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Stay ) ? true : false;
}

bool CDnActor::IsMove( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Move ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Move ) ? true : false;
}

bool CDnActor::IsAttack( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Attack ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Attack ) ? true : false;
}

bool CDnActor::IsHit( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Hit ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Hit ) ? true : false;
}

bool CDnActor::IsAir( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Air ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Air ) ? true : false;
}

bool CDnActor::IsDown( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Down ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Down ) ? true : false;
}

bool CDnActor::IsStun( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stun ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Stun ) ? true : false;
}

bool CDnActor::IsStiff( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stiff ) ? true : false;
	return ( CDnActorState::GetState() & ActorStateEnum::Stiff ) ? true : false;
}

bool CDnActor::IsFly( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_Fly ) ? true : false;
	return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_Fly ) ? true : false;
}

bool CDnActor::IsGround( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_Ground ) ? true : false;
	return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_Ground ) ? true : false;
}


bool CDnActor::IsUnderGround( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_UnderGround ) ? true : false;
	return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_UnderGround ) ? true : false;
}

bool CDnActor::IsLifeSkill( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_LifeSkill ) ? true : false;
	return ( CDnActorState::GetCustomState() & ActorCustomStateEnum::Custom_LifeSkill ) ? true : false;
}

bool CDnActor::IsStandHit( const char *szActionName )
{
	int nIndex;
	int nState = 0;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) nState = m_VecPreActionState[nIndex][0].nState;
	else nState = CDnActorState::GetState();

	if( nState == ActorStateEnum::Hit || nState == ( ActorStateEnum::Hit | ActorStateEnum::Stiff ) ) return true;
	return false;
}

int CDnActor::GetState( const char *szActionName )
{
	int nIndex;
	if( szActionName && ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return m_VecPreActionState[nIndex][0].nState;
	return CDnActorState::GetState();
}

// Separate from SetPlaySpeed, this function updates whenever frame change or frame stop state effects Begin/End.
// Collect the values of applied frame-change state effects, sum them up, and set them at once.
// If FrameStop state effect is active, process as 0 frames.
// If state effect is empty, restore to 60 frames.
// Runs separately from SetPlaySpeed, but after ProcessPlaySpeed() finishes, call this function once more for state effects
// Must re-update the final affected frame. Therefore, SetPlaySpeed must not be used on the state effect side.
void CDnActor::UpdateFPS( void )
{
	if( m_bModifyPlaySpeed )		// If SetPlaySpeed has been called, that value takes precedence.
	{
		CDnActionBase::SetFPS( 60.f * m_fPlaySpeed );
		SetWeaponFPS( 60.f * m_fPlaySpeed );
	}
	else
	{
		ResetPlaySpeed();

		// Frame-freeze state effect. If even one is active, freeze all frames.
		if( 0 < GetFrameStopRefCount() )
		{
			CDnActionBase::SetFPS( 0.0f );
			SetWeaponFPS( 0.0f );
		}
		else
		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 ) ||
			IsAppliedThisStateBlow( STATE_BLOW::BLOW_144 ) || 
			IsAppliedThisStateBlow( STATE_BLOW::BLOW_220)
			)
		{
			// Collect and apply values from frame-change state effects.
			float fFrameRatio = 1.0f;
#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
			float fBuffFrameRatio = 0.0f;
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT
			DNVector( DnBlowHandle ) vlhFrameBlows;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_025, vlhFrameBlows );
			for( int i = 0; i < (int)vlhFrameBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlhFrameBlows.at( i );
#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
				if( hBlow->GetAddBlowStateType() == AddBlowStateType::Equip_Buff_Level )
					fBuffFrameRatio += hBlow->GetFloatValue();
				else
					fFrameRatio += hBlow->GetFloatValue();
#else // PRE_ADD_SLOW_DEBUFF_LIMIT
				fFrameRatio += hBlow->GetFloatValue();
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT
			}

			// #27320 Frostbite state effect also has a frame-slowing effect.
			vlhFrameBlows.clear();
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_144, vlhFrameBlows );
			for( int i = 0; i < (int)vlhFrameBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlhFrameBlows.at( i );
#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
				if( hBlow->GetAddBlowStateType() == AddBlowStateType::Equip_Buff_Level )
					fBuffFrameRatio += static_cast<CDnFrostbiteBlow*>(hBlow.GetPointer())->GetFrameBlowArg();
				else
					fFrameRatio += static_cast<CDnFrostbiteBlow*>(hBlow.GetPointer())->GetFrameBlowArg();
#else // PRE_ADD_SLOW_DEBUFF_LIMIT
				fFrameRatio += static_cast<CDnFrostbiteBlow*>(hBlow.GetPointer())->GetFrameBlowArg();
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT
			}

			vlhFrameBlows.clear();
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_220, vlhFrameBlows );
			for( int i = 0; i < (int)vlhFrameBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlhFrameBlows.at( i );
#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
				if( hBlow->GetAddBlowStateType() == AddBlowStateType::Equip_Buff_Level )
					fBuffFrameRatio += hBlow->GetFloatValue();
				else
					fFrameRatio += hBlow->GetFloatValue();
#else // PRE_ADD_SLOW_DEBUFF_LIMIT
				fFrameRatio += hBlow->GetFloatValue();
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT
			}

#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
			CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
			if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
			{
				if( fBuffFrameRatio < CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FrameSpeedRatio_Limit_Min ) )
					fBuffFrameRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FrameSpeedRatio_Limit_Min );
				else if( fBuffFrameRatio > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FrameSpeedRatio_Limit_Max ) )
					fBuffFrameRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FrameSpeedRatio_Limit_Max );
			}

			fFrameRatio += fBuffFrameRatio;
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT

			if( fFrameRatio < 0.0f )
				fFrameRatio = 0.0f;

			if( CDnActionBase::GetFPS() != (60.0f * fFrameRatio) )
				SyncBindActionObjectFPS( 60.0f * fFrameRatio );

			CDnActionBase::SetFPS( 60.0f * fFrameRatio );
			SetWeaponFPS( 60.0f * fFrameRatio );
		}
		else
		{
			// Restore original state
			CDnActionBase::SetFPS( 60.0f );
			SetWeaponFPS( 60.0f );
		}
	}
}

void CDnActor::AddedFrameStop( void ) 
{ 
	++m_iFrameStopRefCount;

	UpdateFPS(); 
};

void CDnActor::RemovedFrameStop( void ) 
{
	m_iFrameStopRefCount--; 
	if( m_iFrameStopRefCount < 0 ) 
		m_iFrameStopRefCount = 0;

	UpdateFPS(); 
};


void CDnActor::SetPlaySpeed( DWORD dwFrame, float fSpeed )
{
	if( m_bModifyPlaySpeed == true ) return;
	m_bModifyPlaySpeed = true;
	m_PlaySpeedStartTime = CDnActionBase::m_LocalTime;
	m_dwPlaySpeedTime = dwFrame;

	m_fPlaySpeed = fSpeed;
	UpdateFPS();
	//CDnActionBase::SetFPS( 60.f * fSpeed );
	//SetWeaponFPS( 60.f * fSpeed );
}


//void CDnActor::ModifyPlaySpeedDuration( DWORD dwNewTimeGap )
//{
//	if( m_bModifyPlaySpeed )
//		m_dwPlaySpeedTime = dwNewTimeGap;
//}


void CDnActor::ProcessPlaySpeed( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bModifyPlaySpeed == false ) return;
	if( m_PlaySpeedStartTime == 0 ) m_PlaySpeedStartTime = LocalTime;
	if( LocalTime - m_PlaySpeedStartTime > m_dwPlaySpeedTime ) {
		//float fPrevFPS = CDnActionBase::GetFPS();
		//CDnActionBase::SetFPS( 60.f );
		//SetWeaponFPS( 60.f );

		//m_bModifyPlaySpeed = false; // Called inside ResetPlaySpeed.
		//OnRestorePlaySpeed( fPrevFPS );
		ResetPlaySpeed();
		UpdateFPS();

		return;
	}
}



// Related to issue #15065.
// Frame freeze when frozen: when calling SetPlaySpeed, PlaySpeed may already be temporarily adjusted due to hitstop 
// Function call sometimes gets dropped. Reset before calling SetPlaySpeed() in CDnFreezingBlow::OnBegin().
void CDnActor::ResetPlaySpeed()
{
	if( m_bModifyPlaySpeed == false ) 
		return;

	float fPrevFPS = CDnActionBase::GetFPS();
	CDnActionBase::SetFPS( 60.f );
	SetWeaponFPS( 60.f );
	m_bModifyPlaySpeed = false;
	OnRestorePlaySpeed( fPrevFPS );	
}

void CDnActor::SetWeaponFPS( float fValue )
{
	for( DWORD i=0; i<CDnWeapon::EquipSlot_Amount; i++ ) {
		if( m_hWeapon[i] ) m_hWeapon[i]->CDnActionBase::SetFPS( fValue );
	}
}


void CDnActor::SyncClassTime( LOCAL_TIME LocalTime )
{
	CDnActionBase::m_LocalTime = LocalTime;
	MASkillUser::m_LocalTime = LocalTime;
	MAActorRenderBase::m_LocalTime = LocalTime;
	MAActorRenderBase::MAFaceAniBase::m_LocalTime = LocalTime;
}


void CDnActor::OnClash( EtVector3 &vPosition, OnClashFloorCheckType bFloorClash )
{
	if( IsDie() ) return;
	if( IsHit() && ( IsAir() || IsDown() ) && !IsStun() ) {
		float fDecrease = 0.85f;

		if( GetVelocity()->x != 0.f ) {
			SetVelocityX( -GetVelocity()->x * fDecrease );
			SetResistanceX( -GetResistance()->x );
		}

		if( GetVelocityValue()->z < -120.f ) {
			float fVelocityValue = GetVelocityValue()->z; // If the impact exceeds a certain threshold, reduce it. 300 is optimal for now..
			if( fVelocityValue < -300.f ) fVelocityValue = -300.f;
			float fYVel = 5.f + ( -fVelocityValue * 0.01f );

			SetVelocityZ( -GetVelocity()->z * ( 1.35f - ( 1.f / 4.f * ( -fVelocityValue * 0.01f  ) ) ) );
			SetResistanceZ( -GetResistance()->z );

			SetVelocityY( fYVel );
			SetResistanceY( -18.f );

			EtVector3 vLook;
			vLook = -m_matexWorld.m_vZAxis;
			vLook.y = 0.f;
			EtVec3Normalize( &vLook, &vLook );
			Look( EtVector2( vLook.x, vLook.z ) );
			if( IsDie() ) SetActionQueue( "Die_Air", 0, 0.f );
			else SetActionQueue( "Hit_AirUp", 0, 0.f );
			if( m_HitParam.hWeapon ) {
				m_HitParam.hWeapon->OnHitSuccess( GetMySmartPtr(), CDnWeapon::Stun, vPosition, true );
			}
			OutputDebug( "OnClash : Bounced!! %.2f\n", fVelocityValue );
		}
		else {
			if( GetVelocity()->z != 0.f ) {
				SetVelocityZ( -GetVelocity()->z * fDecrease );
				SetResistanceZ( -GetResistance()->z );
			}
			OutputDebug( "OnClash : Did not bounce!! %.2f\n", GetVelocity()->z );
		}

	}
}

bool CDnActor::IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal )
{
	bool isSelfCheck = false;
	isSelfCheck = pHitSignal ? (pHitSignal->isSelfCheck?true:false) : false;

	//Only when self-check is not set (false by default; set to true only for specific skills like Wax..).
	//If set to true, also process Hit on self.
	if (false == isSelfCheck)
		if( GetMySmartPtr() == hHitter->GetMySmartPtr() ) return false;

	switch( hHitter->GetDamageCheckType() ) {
		case DamageCheckTypeEnum::DifferentTeam:
			if( GetTeam() == hHitter->GetTeam() ) return false;
			break;
		case DamageCheckTypeEnum::Anyone_AllDamage:
			break;
		case DamageCheckTypeEnum::Anyone_DifferentTeam:
			break;
	}
	
	if( !CDnActorState::IsHittable() ) return false;
	if( IsDie() ) return false;
	if( GetDamageRemainTime() > LocalTime ) return false;

	if( pHitSignal ) {
		switch( pHitSignal->nStateCondition ) {
			case 0:	// Normal
				if( GetState() == CDnActorState::Down ) return false;
				break;
			case 1:	// Down
				if( !( GetState() & CDnActorState::Down ) ) return false;
				break;
			case 2:	// Normal + Down
				break;
		}
	}
	return true;
}

void CDnActor::Show( bool bShow )
{
	m_bShow = bShow;
	MAActorRenderBase *pRender = dynamic_cast<MAActorRenderBase *>(this);
	if( pRender ) pRender->ShowRenderBase( bShow );
}

void CDnActor::ResetActor()
{
	Show( true );
	m_fDownDelta = 0.f;
	m_fDieDelta = 0.f;
	m_fStiffDelta = 0.f;
	m_LastDamageTime = 0;
	m_LastHitSignalTime = 0;
	m_nLastHitSignalIndex = -1;

	m_bModifyPlaySpeed = false;
	m_PlaySpeedStartTime = 0;
	m_dwPlaySpeedTime = 0;

	m_nSuperAmmorTime = 0;
	CDnActionBase::ResetActionBase();
	MAActorRenderBase::ResetActorRenderBase();

	ResetMove();
	ResetLook();

	SetVelocity( EtVector3( 0.f, 0.f, 0.f ) );
	SetResistance( EtVector3( 0.f, 0.f, 0.f ) );
	SetAddHeight( 0.f );

	SetMagnetLength( 0.f );
	SetMagnetDir( EtVector2( 0.f, 0.f ) );
	if( IsCustomAction() ) {
		m_VecMixedAni.clear();
		ResetCustomAction();
	}
	ReleaseSignalImp( CDnActionSignalImp::All ^ CDnActionSignalImp::EtcObject );
	if( IsProcessSkill() ) CancelUsingSkill();
	SetActionQueue( "Stand", 0, 0.f, 0.f, false, false );

#if !defined( PRE_ADD_ACTION_OBJECT_END_ACTION )
	m_mapBindingActionObjects.clear();
#endif

	m_vecBindActionObjectHandle.clear();
}

void CDnActor::ProcessCollision( EtVector3 &vMove )
{
	PushAndCollisionCheck( vMove );
}

void CDnActor::SetAddHeight( float fValue )
{
	m_fAddHeight = fValue;
}

void CDnActor::ProcessState( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActorState::ProcessState( LocalTime, fDelta );

	// #48950 Reset MoveY signal value when super armor breaks.
	if( IsHit() )
	{
		ResetMoveYDistance();
	}

	m_pStateBlow->Process( LocalTime, fDelta );
}

int CDnActor::AddStateBlow( const CDnSkill::SkillInfo* pParentSkill, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szValue, bool bOnPlayerInit/* = false*/ )
{
	// Do not add state effect when dead, except on resurrection
	if( false == bOnPlayerInit )
		if( IsDie() && STATE_BLOW::emBLOW_INDEX::BLOW_057 != emBlowIndex ) 
			return -1;

	if( !m_pStateBlow ) 
		return -1;
	
	int iAddSEResult = m_pStateBlow->CanAddThisBlow( pParentSkill, emBlowIndex );
	switch( iAddSEResult )
	{
		case CDnStateBlow::ADD_FAIL_BY_IMMUNE:		// Immunity judgment is performed only on the game server.
		case CDnStateBlow::ADD_FAIL_BY_DIED:
			return -1;
			break;
	}

	if( pParentSkill && CDnSkillTask::IsActive() && GetSkillTask().IsGuildWarSkill( pParentSkill->iSkillID ) )	// If guild war skill, check for duplicate StateBlow and remove it.
	{
		DNVector(DnBlowHandle) vlhStateBlows;
		m_pStateBlow->GetStateBlowFromBlowIndex( emBlowIndex, vlhStateBlows );
		for( UINT i=0; i<vlhStateBlows.size(); i++ )
		{
			m_pStateBlow->RemoveStateBlowFromID( vlhStateBlows.at( i )->GetBlowID() );
		}
	}

	//When applying speed-changing effect, if Accel effect is already applied
	//Accel effect is removed (STATE_END?).
	switch(emBlowIndex)
	{
	case STATE_BLOW::BLOW_076:
	case STATE_BLOW::BLOW_025:
	case STATE_BLOW::BLOW_144:
		{
			if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_220))
			{
				DNVector(DnBlowHandle) vlhFrameBlows;
				GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_220, vlhFrameBlows );
				for( int i = 0; i < (int)vlhFrameBlows.size(); ++i )
				{
					DnBlowHandle hBlow = vlhFrameBlows.at(i);
					if (hBlow)
						hBlow->SetState(STATE_BLOW::STATE_END);
				}
			}
		}
		break;
	}

#if defined(PRE_ADD_50923)
	//Only one state effect 240 should be applied at a time. When a new 240 arrives, existing one...
	//Remove it.
	if (emBlowIndex == STATE_BLOW::BLOW_240)
	{
		if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_240))
		{
			DNVector(DnBlowHandle) vlhFrameBlows;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_240, vlhFrameBlows );
			for( int i = 0; i < (int)vlhFrameBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlhFrameBlows.at(i);
				if (hBlow)
					hBlow->SetState(STATE_BLOW::STATE_END);
			}
		}
	}
#endif // PRE_ADD_50923

	DnBlowHandle hBlow = m_pStateBlow->CreateStateBlow( GetMySmartPtr(), pParentSkill, emBlowIndex, nDurationTime, szValue );
	if( !hBlow ) 
		return -1;

	if( hBlow->IsDuplicated() )
	{
		return hBlow->GetBlowID();
	}
	else
	{
		hBlow->SetParentSkillInfo( pParentSkill );
		return m_pStateBlow->AddStateBlow( hBlow );
	}
}

int CDnActor::RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	return m_pStateBlow->RemoveStateBlowByBlowDefineIndex( emBlowIndex );
}


DnBlowHandle CDnActor::GetStateBlowFromID( int nStateBlowID )
{
	return m_pStateBlow->GetStateBlowFromID( nStateBlowID );
}


void CDnActor::RemoveStateBlowFromID( int nStateBlowID )
{
	m_pStateBlow->RemoveStateBlowFromID( nStateBlowID );
}

void CDnActor::RemoveAllBlow()
{
	DNVector(DnBlowHandle) vlAppliedStateBlow;
	GetAllAppliedStateBlow( vlAppliedStateBlow );
	int iNumStateBlow = (int)vlAppliedStateBlow.size();
	for( int iBlow = 0; iBlow < iNumStateBlow; ++iBlow )
		m_pStateBlow->RemoveStateBlowFromID( vlAppliedStateBlow.at( iBlow )->GetBlowID() );

	ApplyPassiveSkills();
}

void CDnActor::RemoveAllBlowExpectPassiveSkill()
{
	DNVector(DnBlowHandle) vlAppliedStateBlow;
	GetAllAppliedStateBlow( vlAppliedStateBlow );
	int iNumStateBlow = (int)vlAppliedStateBlow.size();
	for( int iBlow = 0; iBlow < iNumStateBlow; ++iBlow )
	{
		DnBlowHandle hBlow = vlAppliedStateBlow.at( iBlow );
		if( bIsPassiveSkill( hBlow ) )
			continue;

		// The probability skill-ignore state effect of a source item must remain even after death.
		if( IsShouldRemainBlowWhenDie( hBlow ) )
			continue;

		m_pStateBlow->RemoveStateBlowFromID( hBlow->GetBlowID() );
	}
}

bool CDnActor::IsShouldRemainBlowWhenDie( DnBlowHandle hBlow )
{
	bool bResult = false;

	if( hBlow )
	{
		if( hBlow->IsFromSourceItem() )
			return true;

		if( hBlow->IsEternity() )
			return true;
	}

	return bResult;
}

void CDnActor::ResetProcessSkill()
{
	if( IsProcessSkill() )
	{
		if( m_hProcessSkill )
		{
			m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
			m_hProcessSkill.Identity();
		}
	}
}

int CDnActor::GetNumAppliedStateBlow( void )
{ 
	return m_pStateBlow->GetNumStateBlow();
}



DnBlowHandle CDnActor::GetAppliedStateBlow( int iIndex )
{ 
	return m_pStateBlow->GetStateBlow( iIndex ); 
}

void CDnActor::GetAllAppliedStateBlow( DNVector(DnBlowHandle)& /*IN OUT*/ out )
{
	int nSize = m_pStateBlow->GetNumStateBlow();
	if( 0 < nSize ) out.reserve( out.capacity() + nSize );
	for ( int i = 0 ; i < nSize ; i++ )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);
		if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			out.push_back(hBlow);
	}
}


void CDnActor::GetAllAppliedStateBlowBySkillID(int nSkillID, DNVector(DnBlowHandle)& /*IN OUT*/ out)
{
	int nSize = m_pStateBlow->GetNumStateBlow();
	if( 0 < nSize ) out.reserve( out.capacity() + nSize ); // 20080916 performance improvement

	for ( int i = 0 ; i < nSize ; i++ )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);
		if( hBlow )
		{
			const CDnSkill::SkillInfo* pSkillinfo = hBlow->GetParentSkillInfo();
			if( pSkillinfo && pSkillinfo->iSkillID == nSkillID &&
				STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			{
				out.push_back(hBlow);
			}
		}
	}

}


void CDnActor::GatherAppliedStateBlowByBlowIndex( STATE_BLOW::emBLOW_INDEX BlowIndex, DNVector(DnBlowHandle)& /*IN OUT*/ out )
{
	int iNumStateBlow = m_pStateBlow->GetNumStateBlow();
	for( int i = 0; i < iNumStateBlow; ++i )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow( i );
		if( hBlow->GetBlowIndex() == BlowIndex &&
			STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			out.push_back( hBlow );
	}
}



void CDnActor::OnDispatchMessage( DWORD dwActorProtocol, BYTE *pPacket )
{
	switch( dwActorProtocol ) {
		case eActor::SC_PARTSDAMAGE:
		{
			int iPartsTableID, iHP, iHitterID;

			CPacketCompressStream Stream( pPacket, 128 );
			Stream.Read( &iPartsTableID, sizeof(int) );
			Stream.Read( &iHP, sizeof(int) );
			Stream.Read( &iHitterID , sizeof(int) );
			OnSetPartsHP( GetActorHandle(), iPartsTableID, iHP ,iHitterID );

			break;
		}
		case eActor::SC_ONDAMAGE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nSeed;
				char cHitterType;
				DWORD dwUniqueID, dwWeaponUniqueID = -1;
				int nActionIndex;
				bool bSendWeaponSerialID;
				INT64 nWeaponSerialID = -1;
				SHitParam HitParam;
				EtVector3 vPos;
				EtVector2 vVeiwVec;

				Stream.Read( &nSeed, sizeof(int) );
				Stream.Read( &cHitterType, sizeof(char) );
				Stream.Read( &dwUniqueID, sizeof(DWORD) );
				Stream.Read( &bSendWeaponSerialID, sizeof(bool) );
				if( bSendWeaponSerialID ) Stream.Read( &nWeaponSerialID, sizeof(INT64) );
				else Stream.Read( &dwWeaponUniqueID, sizeof(DWORD) );
				Stream.Read( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &HitParam.nDamage, sizeof(INT64) );
				Stream.Read( &HitParam.nCurrentHP, sizeof(INT64) );
				Stream.Read( &HitParam.HitType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &HitParam.vVelocity, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &HitParam.vResistance, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &HitParam.vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vVeiwVec, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
				HitParam.vViewVec = EtVec2toVec3( vVeiwVec );
				Stream.Read( &HitParam.bFirstHit, sizeof(bool) );
				//Stream.Read( &HitParam.nBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &m_fStiffDelta, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10000.f );
				Stream.Read( &m_fDownDelta, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1000.f );
				Stream.Read( &m_nSuperAmmorTime, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				Stream.Read( &HitParam.bSuccessNormalDamage, sizeof(bool) );
				Stream.Read( &HitParam.DistanceType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &HitParam.iCanceledSkillIDInServer, sizeof(int), CPacketCompressStream::INTEGER_SHORT ); 	// #32722 Fix issue where only the client cancels the skill action if hit on the server before the client's skill-use packet arrives.

				// Player super armor can reach up to 33000, so the server sends it uncompressed. (#10065)
				Stream.Read( &m_nCurrSuperArmor, sizeof(int) );
				Stream.Read( &m_nMaxSuperArmor, sizeof(int) );
				//Stream.Read( &m_nCurrSuperArmor, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				//Stream.Read( &m_nMaxSuperArmor, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

				Stream.Read( &HitParam.HasElement, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

#if defined( PRE_ADD_LOTUSGOLEM )
				Stream.Read( &HitParam.bIgnoreShowDamage, sizeof(bool) );
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
				Stream.Read( &HitParam.bStageLimit, sizeof(bool) );
#endif

				if( nActionIndex != -1 && GetElement( nActionIndex ) )
					HitParam.szActionName = GetElement( nActionIndex )->szName;

				if( IsPlayerActor() )
				{
					// #32722 Fix issue where only the client cancels the skill action if hit on the server before the client's skill-use packet arrives.
					// Only cancel the skill action on the client if hit while actually using a skill on the server.
					if( nActionIndex != -1 &&
						0 == HitParam.iCanceledSkillIDInServer )			// On the server, was not using a skill when hit.
					{
						//#41353 For auto-passive skills, server sends UseSkill -> OnDamage packets in order.
						//HitParma.szActionName stores the action for auto-passive skills; clearing it on an auto-passive skill prevents the skill motion from playing.
						if( m_hProcessSkill && m_hProcessSkill->GetSkillType() != CDnSkill::SkillTypeEnum::AutoPassive)		// If the skill was in use on the client.
						{
							// Do not play hit reaction on the client so the skill is not canceled.
							HitParam.szActionName.clear();
						}
					}
				}

				if( bSendWeaponSerialID ) {
					if( nWeaponSerialID != -1 ) {
						HitParam.hWeapon = CDnWeapon::FindWeaponFromSerialID( nWeaponSerialID );
					}
				}
				else {
					if( dwWeaponUniqueID != -1 ) {
						HitParam.hWeapon = CDnWeapon::FindWeaponFromUniqueID( dwWeaponUniqueID );
					}
				}

				CDnDamageBase *pHitter = NULL;
				switch( cHitterType ) {
					case DamageObjectTypeEnum::Actor:
						{
							int nComboCount;
							int nComboDelay;
							if( dwUniqueID >= 100000 ) 
							{
								Stream.Read( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
								Stream.Read( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
							}
							DnActorHandle hHitter = FindActorFromUniqueID( dwUniqueID );
							pHitter = hHitter;

#ifdef PRE_ADD_LOTUSGOLEM
							if (HitParam.bIgnoreShowDamage)
								break;
#endif

							if( !HitParam.hWeapon && hHitter ) {
								HitParam.hWeapon = ( hHitter->GetWeapon(1) ) ? hHitter->GetWeapon(1) : hHitter->GetWeapon(0);
							}

							if( hHitter && dwUniqueID >= 100000 ) {
								hHitter->OnComboCount( nComboCount, nComboDelay );
							}

							// Update combo for the summoning player when a relic (summon) hits. (#11415)
							if( hHitter && dwUniqueID < 100000 && hHitter->IsMonsterActor() ) 
							{
								// If monster was summoned from a relic, increment original summoner owner's combo.
								DnActorHandle hSummonMasterPlayerActor = static_cast<CDnMonsterActor*>(hHitter.GetPointer())->GetSummonerPlayerActor();
								if( hSummonMasterPlayerActor && hSummonMasterPlayerActor->IsPlayerActor() )
								{
									Stream.Read( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
									Stream.Read( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
									hSummonMasterPlayerActor->OnComboCount( nComboCount, nComboDelay );
								}
							}

							// If hit by a cannon and the player holding the cannon is the local player, update combo count
							if( hHitter && CDnActorState::Cannon == hHitter->GetActorType() )
							{
								Stream.Read( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
								Stream.Read( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
								hHitter->OnComboCount( nComboCount, nComboDelay );
							}
						}
						break;
					case DamageObjectTypeEnum::Prop:
						{
							DnPropHandle hHitter = CDnWorldProp::FindPropFromUniqueID( dwUniqueID );
							pHitter = dynamic_cast<CDnDamageBase*>(hHitter.GetPointer());
						}
						break;
				}

				OnDamageReadAdditionalPacket( &Stream );
				
				//#65346
				//When a projectile with Self check enabled hits continuously, position error causes a lag-like appearance.
				//Do not set position value when it is own hit
				if( dwUniqueID != m_dwUniqueID )
				{
					bool bSafetyHit = false;
					if( cHitterType == DamageObjectTypeEnum::Actor && HitParam.nDamage == 0 && nActionIndex == -1 )
					{
						DnActorHandle hHitter = FindActorFromUniqueID( dwUniqueID );
						if( hHitter && hHitter->GetTeam() == GetTeam() )
							bSafetyHit = true;
					}

					if( !bSafetyHit )
						SetPosition( vPos );
				}

				OutputDebug( "OnDamage : %s, %.2f, %.2f\n", HitParam.szActionName.c_str(), HitParam.vVelocity.y, HitParam.vResistance.y );
				_srand( nSeed );
				
				m_pStateBlow->OnCalcDamage( 0.0f, HitParam );

				OnDamage( pHitter, HitParam );

			}
			break;
		case eActor::SC_CMDADDSTATEEFFECT:
			{
				CPacketCompressStream PacketStream( pPacket, 128 );

				int nDurationTime;
				CDnSkill::SkillInfo ParentSkillInfo;
				DWORD dwHitterUniqueID;
				std::string szParam;
				STATE_BLOW::emBLOW_INDEX emState;
				BYTE cItemSkill,cOnInit;
				int nServerBlowID = 0;
				std::string szEffectOutputIDs;
				CDnSkill::TargetTypeEnum eTargetType = CDnSkill::TargetTypeEnum::Self;
				bool bEternity = false;

				PacketStream.Read( &dwHitterUniqueID, sizeof(DWORD) );
				PacketStream.Read( &cItemSkill, sizeof(BYTE) );
				PacketStream.Read( &ParentSkillInfo.iSkillID, sizeof(int) );
				PacketStream.Read( &ParentSkillInfo.iSkillLevelID, sizeof(int) );
				PacketStream.Read( &emState, sizeof(int) );
				PacketStream.Read( &nDurationTime, sizeof(int) );

				//LOCAL_TIME ServerStartTime = 0;
				//Stream.Read( &ServerStartTime, sizeof(__int64) );
				ReadStdString( szParam, &PacketStream );

				PacketStream.Read( &cOnInit, sizeof(BYTE) );
				PacketStream.Read( &nServerBlowID, sizeof(int) );
				ReadStdString( szEffectOutputIDs, &PacketStream);
				
				PacketStream.Read( &ParentSkillInfo.bFromInputHasPassiveSignal, sizeof(bool) );
				PacketStream.Read( &ParentSkillInfo.bFromBuffProp, sizeof(bool) );
				PacketStream.Read( &ParentSkillInfo.eTargetType, sizeof(CDnSkill::TargetTypeEnum) );

				// Whether to force Begin Blow [2011/03/18 semozz]
				PacketStream.Read( &ParentSkillInfo.bItemPrefixSkill, sizeof(ParentSkillInfo.bItemPrefixSkill) );

				// Added code to ignore immunity handling for guild war skills (castle gate related) [2011/04/14 semozz]
				PacketStream.Read( &ParentSkillInfo.bIgnoreImmune, sizeof(ParentSkillInfo.bIgnoreImmune));

#if defined(PRE_FIX_NEXTSKILLINFO)
				PacketStream.Read(&ParentSkillInfo.nSkillLevel, sizeof(ParentSkillInfo.nSkillLevel));
#endif // PRE_FIX_NEXTSKILLINFO

				PacketStream.Read( &bEternity, sizeof(bool) );

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
				PacketStream.Read(&ParentSkillInfo.iAppliedEnchantSkillID, sizeof(int));
				bool bAppliedEnchantSkill = (ParentSkillInfo.iAppliedEnchantSkillID > 0);
#endif

				DNTableFileFormat*  pSkillTable = NULL;
				DNTableFileFormat*  pSkillLevelTable = NULL;

				if( CDnSkillTask::IsActive() && GetSkillTask().IsGuildWarSkill( ParentSkillInfo.iSkillID ) )	// Read guild war table if it is a guild war skill.
				{
					pSkillTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILL );
					pSkillLevelTable = GetDNTable( CDnTableDB::TBATTLEGROUNDSKILLLEVEL );
				}
				else
				{
					pSkillTable = GetDNTable( CDnTableDB::TSKILL );
					pSkillLevelTable = GetDNTable( CDnTableDB::TSKILLLEVEL );
				}

				ParentSkillInfo.hSkillUser		= FindActorFromUniqueID( dwHitterUniqueID );
				ParentSkillInfo.bIsItemSkill	= (cItemSkill == 0) ? false : true;

				if( ParentSkillInfo.iSkillID != 0 && pSkillTable != NULL && pSkillLevelTable != NULL )
				{
					ParentSkillInfo.eSkillType		= (CDnSkill::SkillTypeEnum)pSkillTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillID, "_SkillType" )->GetInteger();
#ifdef PRE_FIX_SYNC_ENCHANT_SKILL
					const int nSkillIdApplied = (bAppliedEnchantSkill) ? ParentSkillInfo.iAppliedEnchantSkillID : ParentSkillInfo.iSkillID;

					ParentSkillInfo.eDurationType	= (CDnSkill::DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( nSkillIdApplied, "_DurationType" )->GetInteger();
#else
					ParentSkillInfo.eDurationType	= (CDnSkill::DurationTypeEnum)pSkillTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillID, "_DurationType" )->GetInteger();
#endif
					ParentSkillInfo.iDissolvable	= pSkillTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillID, "_Dissolvable" )->GetInteger();

					if (szEffectOutputIDs.empty())
						ParentSkillInfo.effectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillLevelID, "_StateEffectTableID" )->GetString();
					else
						ParentSkillInfo.effectOutputIDs = szEffectOutputIDs;

#ifdef PRE_FIX_SYNC_ENCHANT_SKILL

					ParentSkillInfo.iBuffIconImageIndex = pSkillTable->GetFieldFromLablePtr( nSkillIdApplied, "_BuffIconImageIndex" )->GetInteger();
					ParentSkillInfo.iDebuffIconImageIndex = pSkillTable->GetFieldFromLablePtr( nSkillIdApplied, "_DeBuffIconImageIndex" )->GetInteger();
					ParentSkillInfo.debuffEffectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillLevelID, "_StateEffectOtherID" )->GetString();

#else // PRE_FIX_SYNC_ENCHANT_SKILL
															ParentSkillInfo.iBuffIconImageIndex = pSkillTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillID, "_BuffIconImageIndex" )->GetInteger();
										#ifdef PRE_ADD_SKILL_ADDTIONAL_BUFF
															ParentSkillInfo.iDebuffIconImageIndex = pSkillTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillID, "_DeBuffIconImageIndex" )->GetInteger();

															if (szEffectOutputIDs.empty())
																ParentSkillInfo.debuffEffectOutputIDs = pSkillLevelTable->GetFieldFromLablePtr( ParentSkillInfo.iSkillLevelID, "_StateEffectOtherID" )->GetString();
										#endif
#endif // PRE_FIX_SYNC_ENCHANT_SKILL

					if( ParentSkillInfo.hSkillUser )
						ParentSkillInfo.iSkillUserTeam = ParentSkillInfo.hSkillUser->GetTeam();
					else
						ParentSkillInfo.iSkillUserTeam = m_nTeam;		// If there is no skill user, set the team to own team.

					vector<int> vlSkillLevelList;
					//if( pSkillLevelTable->GetItemIDListFromField( "_SkillIndex", ParentSkillInfo.iSkillID, vlSkillLevelList ) != 0 )
					GetSkillTask().GetSkillLevelList( ParentSkillInfo.iSkillID, vlSkillLevelList, CDnSkill::PVE );		// Just process based on PvE.
					if( false == vlSkillLevelList.empty() )
					{
						ParentSkillInfo.iSkillLevelIDOffset = vlSkillLevelList.front();
					}
				}

#if defined(PRE_FIX_NEXTSKILLINFO)
				//For monsters, a state effect add packet can arrive before the SC_USESKILL packet.
				//On the server, skill info is added when creating a monster, whereas the client has no skill info.
				//At this point, when a state effect is added, the corresponding skill might not exist, so add it.... Are monster skills PVE only??
				if( ParentSkillInfo.hSkillUser == GetActorHandle() && IsMonsterActor() == true)
				{
					AddSkill( ParentSkillInfo.iSkillID, ParentSkillInfo.nSkillLevel );
				}
#endif // PRE_FIX_NEXTSKILLINFO

#if defined( PRE_FIX_BREAKINTO_BLOW_SYNC )
				if( ParentSkillInfo.eDurationType == CDnSkill::DurationTypeEnum::Aura )
				{
					DnSkillHandle hSkill = FindSkill( ParentSkillInfo.iSkillID );
					if( hSkill && hSkill->IsAuraOn() == false )
					{
						hSkill->EnableAura( true );
					}
				}
				else if( ParentSkillInfo.eDurationType == CDnSkill::DurationTypeEnum::ActiveToggle 
					|| ParentSkillInfo.eDurationType == CDnSkill::DurationTypeEnum::ActiveToggleForSummon
					|| ParentSkillInfo.eDurationType == CDnSkill::DurationTypeEnum::TimeToggle )
				{
					DnSkillHandle hSkill = FindSkill( ParentSkillInfo.iSkillID );
					if( hSkill && hSkill->IsToggleOn() == false )
					{
						hSkill->EnableToggle( true );
					}
				}
#endif
	
				DnBlowHandle hAddedBlow = CmdAddStateEffect( ( ParentSkillInfo.iSkillID == 0 ) ? NULL : &ParentSkillInfo, emState, nDurationTime, szParam.c_str(), cOnInit==1 ? true : false );
				if( hAddedBlow )
				{
					hAddedBlow->SetServerBlowID( nServerBlowID );
					
					// If there is additional packet data per state effect, pass it along to handle automatically.
					hAddedBlow->OnReceiveAddPacket( PacketStream );
					hAddedBlow->SetEternity( bEternity );

					CDnGameTask* pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
					if( pGameTask )
						pGameTask->OnCmdAddStateEffect( &ParentSkillInfo );
				}
			}
			break;
		case eActor::SC_CMDREMOVESTATEEFFECT:
			{
				CPacketCompressStream Stream( pPacket, 32 );

				STATE_BLOW::emBLOW_INDEX BlowIndex = STATE_BLOW::BLOW_NONE;
				Stream.Read( &BlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );
				
				if( m_pStateBlow->IsApplied( BlowIndex ) )
				{
					// [2010/12/17 semozz]
					// Immediately remove state effects to be deleted received via packet.
					m_pStateBlow->RemoveImediatlyStateEffectByBlowIndex(BlowIndex);
				}
			}
			break;

		case eActor::SC_CMDREMOVESTATEEFFECTFROMID:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				int nID = 0;

				Stream.Read( &nID, sizeof(int) );

				// [2010/12/17 semozz]
				// Immediately remove state effects to be deleted received via packet.
				m_pStateBlow->RemoveImediatlyStateEffectByServerID(nID);
			}
			break;

		case eActor::SC_CMDMODIFYSTATEEFFECT:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				int nServerBlowID = 0;
				float fDurationTime = 0.0f;

				Stream.Read( &nServerBlowID, sizeof(int) );
				Stream.Read( &fDurationTime, sizeof(float) );

				DnBlowHandle hBlow = m_pStateBlow->GetStateBlowFromServerID( nServerBlowID );
				if( hBlow )
					hBlow->SetDurationTime( fDurationTime );
			}
			break;
		case eActor::SC_CMDSUICIDE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				bool bDropItem, bDropExp;
				Stream.Read( &bDropItem, sizeof(bool) );
				Stream.Read( &bDropExp, sizeof(bool) );

				CmdSuicide( bDropItem, bDropExp );
			}
			break;
		case eActor::SC_SETHPMP_DELTA:
			{
				if(IsDie())
					break;

				CPacketCompressStream Stream( pPacket, 32 );

				ElementEnum eElement = ElementEnum::ElementEnum_Amount;
				INT64 nHPMPDelta = 0;
				DWORD dwUniqueID;
				bool bIsMPDelta = false;
				bool bShowValue;

				Stream.Read( &eElement, sizeof(ElementEnum), CPacketCompressStream::INTEGER_CHAR );
				Stream.Read( &nHPMPDelta, sizeof(INT64) );
				Stream.Read( &dwUniqueID, sizeof(DWORD) );
				Stream.Read( &bIsMPDelta, sizeof(bool) );
				Stream.Read( &bShowValue, sizeof(bool) );

#if defined(PRE_FIX_59308)
				int nHitType = CDnWeapon::Normal;
				Stream.Read( &nHitType, sizeof(int) );
#endif // PRE_FIX_59308
#ifdef PRE_FIX_77172
				bool bKeepJumpMovement = false;
				Stream.Read( &bKeepJumpMovement, sizeof(bool) );
#endif // PRE_FIX_77172

				if( false == bIsMPDelta )
				{
					if( nHPMPDelta < 0 )
					{
						DnActorHandle hHitter = FindActorFromUniqueID( dwUniqueID );
						CDnDamageBase::SHitParam HitParam;

						HitParam.bRecvOnDamage	= false;
#if defined(PRE_FIX_59308)
						HitParam.HitType = (CDnWeapon::HitTypeEnum)nHitType;
#endif // PRE_FIX_59308
						HitParam.HasElement = eElement;
						HitParam.nDamage = -nHPMPDelta;
						HitParam.vPosition = *GetPosition();
						HitParam.vViewVec = ( GetLookDir() ) ? *GetLookDir() : EtVector3( 0.f, 0.f, 0.f );

						// Note: When airborne, default hit parameter constructor may set y velocity to 0.0f, so 
						// Set the current original speed so it has no effect.
						HitParam.vVelocity = ( GetVelocity() ) ? *GetVelocity() : EtVector3( 0.0f, 0.0f, 0.0f );
						HitParam.vResistance = ( GetResistance() ) ? *GetResistance() : EtVector3( 0.0f, 0.0f, 0.0f );
#ifdef PRE_FIX_77172
						HitParam.bKeepJumpMovement = bKeepJumpMovement;
#endif // PRE_FIX_77172
						bool bCheckActorDie = false;
						INT64 iAfterDamageHP = GetHP() - HitParam.nDamage;
						if( iAfterDamageHP <= 0 )
						{
							//In OnDamage below, state effect 137 sets HP to 1, but the final code resets HP back to 0 so the monster
							//Occurs when a skill needed on death fails to trigger and gets removed immediately. If state effect 137 exists, bCheckActorDie
							//Do not set the flag so that HP can stay at 1.
							//Allow the Die action to be set only when state effect #137 is not active.
							if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_137) == false)
							{
								string strDieAction;
								GetProperlyDieAction( strDieAction );
								_ASSERT( !strDieAction.empty() );
								if( !strDieAction.empty() )
									HitParam.szActionName = strDieAction;

								bCheckActorDie = true;
							}

							HitParam.nDamage = GetHP();
							OutputDebug( "Die By StateEffect : %s\n", HitParam.szActionName.c_str() );
						}

						CDnDamageBase* pHitter = NULL;
						if( hHitter )
							pHitter = dynamic_cast<CDnDamageBase*>(hHitter.GetPointer());
						
						// Note: Added current HP here too since it was added to SC_ONDAMAGE.
						HitParam.nCurrentHP = iAfterDamageHP;
						OnDamage( pHitter, HitParam );

						if( bCheckActorDie && !IsDie() )
							SetHP(0);
					}
					else
					{
						// Recovery simply restores HP
						SetHP( GetHP() + nHPMPDelta );					
						if( bShowValue && nHPMPDelta > 0 ) {

							CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>(this);
							bool bSameTeam = (GetTeam() == CDnActor::s_hLocalActor->GetTeam());

							GetInterface().SetRecovery( GetHeadPosition(), (int)nHPMPDelta, 0, ( pMonsterActor != NULL || !bSameTeam ),		// If it is a monster or on a different team, it is an enemy.
																										(GetMySmartPtr() == CDnActor::s_hLocalActor)  );
#ifdef _RDEBUG
							g_Log.Log(LogType::_FILELOG, "[DMGERR] SC_SETHPMP_DELTA : false == bIsMPDelta / nHPMPDelta(%d)\n", nHPMPDelta);
#endif
						}				
						if( GetMaxHP() < GetHP() )
							SetHP( GetMaxHP() );
					}
				}
				else
				{
					int iNewSP = GetSP() + (int)nHPMPDelta;
					if( iNewSP > GetMaxSP() )
						iNewSP = GetMaxSP();
					else
					if( iNewSP < 0 )
						iNewSP = 0;

					if( bShowValue && nHPMPDelta > 0 ) 
					{
						GetInterface().SetRecovery( GetHeadPosition(), 0, (int)nHPMPDelta, false, (GetMySmartPtr() == CDnActor::s_hLocalActor)  );
					}

					SetSP( iNewSP );
				}
			}
			break;

		case eActor::SC_STATEEFFECT_PROB_SUCCESS:
			{
				CPacketCompressStream Stream( pPacket, 16 );
				int iSkillID = 0;
				STATE_BLOW::emBLOW_INDEX eBlowIndex = STATE_BLOW::BLOW_NONE;
				
				Stream.Read( &iSkillID, sizeof(int) );
				Stream.Read( &eBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );

				DNVector( DnBlowHandle ) vlhBlows;
				m_pStateBlow->GetStateBlowFromBlowIndex( eBlowIndex, vlhBlows );
				for( int i = 0; i < (int)vlhBlows.size(); ++i )
				{
					DnBlowHandle hBlow = vlhBlows.at( i );

					const CDnSkill::SkillInfo* pParentSkillInfo = hBlow->GetParentSkillInfo();
					if( pParentSkillInfo && iSkillID == pParentSkillInfo->iSkillID )
					{
						// If payback mana state effect.
						if( STATE_BLOW::BLOW_068 == eBlowIndex )
						{
							CDnPayBackMPBlow* pPayBackMPBlow = static_cast<CDnPayBackMPBlow*>(hBlow.GetPointer());

							// Show the effect for a few seconds on the client side.
							pPayBackMPBlow->OnActivate();
						}
					}
				}
			}
			break;

		case eActor::SC_ADD_STATE_EFFECT_DENIED:
			{
				CPacketCompressStream Stream( pPacket, 16 );

				int iAddSEResult = 0;
				Stream.Read( &iAddSEResult, sizeof(int) );
				ASSERT( (iAddSEResult != CDnStateBlow::ADD_SUCCESS) && "State effect add succeeded, but received failure packet" );
				switch( iAddSEResult )
				{
					case CDnStateBlow::ADD_FAIL_BY_IMMUNE:
#ifdef PRE_ADD_DECREASE_EFFECT
					case CDnStateBlow::ADD_DECREASE_EFFECT_BY_IMMUNE:
#endif // PRE_ADD_DECREASE_EFFECT
						{
							// Display immune
							SAABox box;
							GetBoundingBox(box);
							float fHeight = box.Max.y - box.Min.y;// + 10.f;
#ifdef PRE_ADD_DECREASE_EFFECT
							if( fHeight == 0.0f && GetObjectHandle() )
							{
								((MAActorRenderBase*)this)->Process( m_matexWorld, 0, 0.0f );
								GetBoundingBox(box);
								fHeight = box.Max.y - box.Min.y;
							}
#endif // PRE_ADD_DECREASE_EFFECT
							EtVector3 vPos = m_matexWorld.m_vPosition;
							vPos.y += fHeight;

							if( CDnActor::s_hLocalActor ) 
							{
								bool bPartyMember = !IsMonsterActor() && (GetMySmartPtr() != CDnActor::s_hLocalActor) && (GetTeam() == CDnActor::s_hLocalActor->GetTeam());
								if(  !bPartyMember )	 
								{		
									if( CGameOption::GetInstance().bMyCombatInfo ) 
									{
#ifdef PRE_ADD_DECREASE_EFFECT
										if( iAddSEResult == CDnStateBlow::ADD_FAIL_BY_IMMUNE )
											GetInterface().SetResist( vPos );
										else if( iAddSEResult == CDnStateBlow::ADD_DECREASE_EFFECT_BY_IMMUNE )
											GetInterface().SetDecreaseEffect( vPos );
#else // PRE_ADD_DECREASE_EFFECT
										GetInterface().SetResist( vPos );
#endif // PRE_ADD_DECREASE_EFFECT
									}								
								}
							}
						}
						break;
					case CDnStateBlow::ADD_FAIL_BY_DIED:
						break;
					case CDnStateBlow::ADD_FAIL_BY_PROBABILITY:
						OutputDebug( "[Add State Effect Failed by probability fail!]\n" );
						break;
				}
			}
			break;

			/*
		case eActor::SC_POSITION:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3	vPosition;
				EtVector2	vLook;

				Stream.Read( &vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

				SetPosition( vPosition );
				Look( vLook );

				break;
			}
			*/
		case eActor::SC_CMDWARP:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3	vPos;
				EtVector2	vLook;

				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vLook, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

				CmdWarp( vPos, vLook );
			}
			break;
		case eActor::SC_PLAYERDIE:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				DWORD dwHitterUniqueID;
				Stream.Read( &dwHitterUniqueID, sizeof(DWORD) );

				DnActorHandle hHitter = FindActorFromUniqueID( dwHitterUniqueID );

				// ScoreSystem
				CDnGameTask* pGameTask = (CDnGameTask*)CTaskManager::GetInstance().GetTask( "GameTask" );
				if( pGameTask )
					pGameTask->OnDie( GetActorHandle(), hHitter );

				break;
			}
			
		case eActor::SC_INVALID_PROJECTILE:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				
				DWORD dwUniqueID = 0;
				Stream.Read( &dwUniqueID, sizeof(DWORD) );

				DnWeaponHandle hProjectile = CDnProjectile::FindWeaponFromUniqueID( dwUniqueID );
				if( hProjectile )
				{
					CDnProjectile* pProjectile = dynamic_cast<CDnProjectile*>( hProjectile.GetPointer() );
					if( pProjectile )
					{
						pProjectile->SetInvalid( CDnActionBase::m_LocalTime );
					}
				}
			}
			break;
		case eActor::SC_CMDCHANGETEAM:
			{
				CPacketCompressStream Stream( pPacket, 32 );

				int nTeam;
				Stream.Read( &nTeam, sizeof(int) );

				SetTeam( nTeam ); 
#ifdef PRE_ADD_IMPROVE_MASTER_OBSERVER
				GetInterface().RefreshPvpPlayerGauge();
#endif

			}
			break;
		case eActor::SC_CMDFORCEVELOCITY:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				EtVector3 vPos, vVel, vRes;
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vVel, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vRes, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

				SetPosition( vPos );
				SetVelocity( vVel );
				SetResistance( vRes );
			}
			break;
		case eActor::SC_CMDCHATBALLOON:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				int nLength;
				WCHAR wszChat[64] = { 0, };

				Stream.Read( &nLength, sizeof(int) );
				Stream.Read( wszChat, nLength );

				if( __wcsicmp_l( L"HIDE_TRIGGER_CHATBALLOON", wszChat ) == 0 )
				{
					StopHeadEffect( CDnHeadEffectRender::HeadEffect_ChatBalloon );
					break;
				}

#ifdef PRE_ADD_CASH_AMULET
				if(GetChatBalloon().IsEnableCustomChatBallon())
				{
					GetChatBalloon().StopChatBalloonCustom();
					GetChatBalloon().ClearCustomChatBalloonText();
				}	

				if(CGameOption::GetInstance().m_bShowCashChatBalloon && IsPlayerActor())
				{
					CDnPlayerActor* pPlayer = static_cast<CDnPlayerActor*>(GetActorHandle().GetPointer());
					DnPartsHandle hCashAmulet = pPlayer->GetCashParts(CDnParts::CashNecklace);

					if(hCashAmulet)
					{
						int nItemID = hCashAmulet->GetClassID();
						DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TCASHCHATBALLOONTABLE );

						if(pSox && pSox->IsExistItem(nItemID))
							GetChatBalloon().SetChatBalloonCustom( pSox->GetFieldFromLablePtr( nItemID, "_UIResourceName1" )->GetString() , wszChat , chatroomcolor::NORMAL, GetTickCount(), 1 );
						else
							SetChatBalloonText( wszChat, GetTickCount(), 1, true );

					}
					else
						SetChatBalloonText( wszChat, GetTickCount(), 1, true );
						
				}
				else
					SetChatBalloonText( wszChat, GetTickCount(), 1, true );

#else
				SetChatBalloonText( wszChat, GetTickCount(), 1, true );
#endif
			}
			break;
		case eActor::SC_SHOWEXPOSUREINFO:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				bool bShow;
				Stream.Read( &bShow, sizeof(bool) );

				CmdShowExposureInfo( bShow );
			}
			break;

		case eActor::SC_CMDSHOW:
			{
				Show( true );

				if( CDnQuestTask::IsActive() && IsNpcActor() )
				{
					CDnQuestTask *pTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask("QuestTask"));
					pTask->SetRefreshNpcQuestState(true);
				}
				break;
			}
		case eActor::SC_CMDHIDE:
			{
				Show( false );
				ReleaseSignalImp();
				
				if( CDnQuestTask::IsActive() && IsNpcActor() )
				{
					CDnQuestTask *pTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask("QuestTask"));
					pTask->SetRefreshNpcQuestState(true);
				}
				break;
			}
		case eActor::SC_REFRESHHPSP:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				INT64 nHP;
				int nSP;
				Stream.Read( &nHP, sizeof(INT64) );
				Stream.Read( &nSP, sizeof(int) );

				m_pStateBlow->Process( 0, 0.f );

				SetHP( nHP );
				SetSP( nSP );
			}
			break;

		case eActor::SC_FINISH_PROCESSSKILL:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nSkillID;
				Stream.Read( &nSkillID, sizeof(int) );

				if( IsProcessSkill() )
				{
					DnSkillHandle hSkill = GetProcessSkill();
					if( hSkill && hSkill->GetClassID() == nSkillID )
						ResetProcessSkill();
				}
				break;
			}

#ifdef _SHADOW_TEST
		case eActor::SC_SHADOW:
			{
				if( m_bEnableShadowActor == false ) break;

				CPacketCompressStream Stream( pPacket, 256 );
				int nActionIndex;
				float fFrame;
				EtVector3 vPos;
				EtVector3 vViewX, vViewZ;
				EtVector3 vMoveX, vMoveZ;

				Stream.Read( &nActionIndex, sizeof(int) );
				Stream.Read( &fFrame, sizeof(float) );
				Stream.Read( &vPos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
				Stream.Read( &vViewX, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				Stream.Read( &vViewZ, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				Stream.Read( &vMoveX, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
				Stream.Read( &vMoveZ, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

				bool bMoveTarget;
				bool bLookTarget;
				Stream.Read( &bMoveTarget, sizeof(bool) );
				if( bMoveTarget ) {
					EtVector3 vPos;
					Stream.Read( &vPos, sizeof(EtVector3) );
				}
				Stream.Read( &bLookTarget, sizeof(bool) );
				if( bLookTarget ) {
					EtVector3 vPos;
					Stream.Read( &vPos, sizeof(EtVector3) );
				}
				if( m_hShadowActor ) {
					MatrixEx *pCross = m_hShadowActor->GetMatEx();
					pCross->m_vPosition = vPos;
					pCross->m_vXAxis = vViewX;
					pCross->m_vZAxis = vViewZ;
					pCross->MakeUpCartesianByXAxis();
					m_hShadowActor->SetMoveVectorX( vMoveX );
					m_hShadowActor->SetMoveVectorZ( vMoveZ );

					if( nActionIndex != -1 && m_hShadowActor->GetElement( nActionIndex ) ) {
						char *szActionName = (char *)m_hShadowActor->GetElement( nActionIndex )->szName.c_str();
						if( fFrame < 0.f ) fFrame = 0.f;
						m_hShadowActor->SetAction( szActionName, fFrame, 0.f );
					}
				}

			}
			break;
#endif
		case eActor::SC_FREEZINGPRISON_DURABILITY:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nStateBlowID = -1;
				float fDurabilityRate = 0.0f;
				bool bShowGauge = false;

				Stream.Read(&nStateBlowID, sizeof(nStateBlowID));
				Stream.Read(&fDurabilityRate, sizeof(fDurabilityRate));
				Stream.Read(&bShowGauge, sizeof(bShowGauge));

				//Show hit effect
				// Added check to skip Hit action when initial fDurabilityRate is 100.0f [2010/12/16 semozz]
				if (bShowGauge && 
					fDurabilityRate != 100.0f &&
					fDurabilityRate > 0.0f)
				{
					DnBlowHandle hBlow = m_pStateBlow->GetStateBlowFromServerID( nStateBlowID );
					CDnFreezingPrisonBlow* pFreezingPrisonBlow = hBlow.GetPointer() ? static_cast<CDnFreezingPrisonBlow*>( hBlow.GetPointer() ) : NULL;
					if (pFreezingPrisonBlow)
						pFreezingPrisonBlow->OnHit();
				}

				//Display Gauge UI
//				pFreezingPrisonBlow->ShowGuage(fDurabilityRate, bShowGauge);
				GetInterface().ShowDurabilityGauge(GetActorHandle(), nStateBlowID, fDurabilityRate, bShowGauge);

				OutputDebug("SC_FREEZINGPRISON_DURABILITY : ActorHandle 0x%x, BlowID : %d, Durability : %f\n", this, nStateBlowID, fDurabilityRate);
			}
			break;

		case eActor::SC_SHOW_STATE_EFFECT:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				DWORD dwUniqueID = 0;
				STATE_BLOW::emBLOW_INDEX BlowIndex;
				bool bShowGraphicEffect = false;
				Stream.Read( &dwUniqueID, sizeof(dwUniqueID) );
				Stream.Read( &BlowIndex, sizeof(BlowIndex) );
				Stream.Read( &bShowGraphicEffect, sizeof(bShowGraphicEffect));

				//DnBlowHandle hBlowHandle = m_pStateBlow->GetStateBlowFromServerID( BlowServerID );
				DNVector(DnBlowHandle) vlhHandles;
				m_pStateBlow->GetStateBlowFromBlowIndex(BlowIndex, vlhHandles);
				if (!vlhHandles.empty())
				{
					vlhHandles[0]->ShowGraphicEffect(bShowGraphicEffect);
				}
			}
			break;


		case eActor::SC_FINISH_AURASKILL:
			{
				CPacketCompressStream Stream( pPacket, 32 );
				DWORD dwUniqueID = -1;
				DWORD dwSkillID = -1;
				Stream.Read( &dwUniqueID, sizeof(dwUniqueID) );
				Stream.Read( &dwSkillID, sizeof(dwSkillID) );

				if (dwUniqueID == GetUniqueID())
				{
					CmdFinishAuraSkill(dwSkillID);
				}
			}
			break;
		case eActor::SC_ACTION_CHANGE:
			{
				CPacketCompressStream Stream( pPacket, 16 );
				int nActionIndex = -1;

				Stream.Read( &nActionIndex, sizeof(nActionIndex) );
				ActionChange(nActionIndex);
			}
			break;

		case eActor::SC_CHAINATTACK_PROJECTILE:
			{
				CPacketCompressStream Stream( pPacket, 256 );

				CreateChainAttackProjectile(Stream);
			}
			break;

		case eActor::SC_PROBINVINCIBLE_SUCCESS:
			{
				// No separate packet data. Just display resist!!.
				// Show immunity for party members including self.
				SAABox box;
				GetBoundingBox(box);
				float fHeight = box.Max.y - box.Min.y;
				EtVector3 vPos = m_matexWorld.m_vPosition;
				vPos.y += fHeight;

				if( CDnActor::s_hLocalActor ) 
				{
					CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>(this);
					bool bPartyMember = (pMonsterActor == NULL) &&		// Is not a monster and
						(GetTeam() == CDnActor::s_hLocalActor->GetTeam());	// If on the same team, they are a party member.
					if(  bPartyMember )
					{
						if( CGameOption::GetInstance().bMyCombatInfo ) {
							GetInterface().SetResist( vPos );
						}								
					}
				}

				// Also notify state effect side for effect output.
				DNVector(DnBlowHandle) vlhBlow;
				m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_226, vlhBlow );
				if( false == vlhBlow.empty() )
				{
					CDnProbInvincibleAtBlow* pBlow = static_cast<CDnProbInvincibleAtBlow*>(vlhBlow.front().GetPointer());
					pBlow->OnSuccess();
				}
			}
			break;

		case eActor::SC_FORCE_ADD_SKILL:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nSkillID;
				Stream.Read( &nSkillID , sizeof(int) );
				AddSkill(nSkillID);
			}
			break;

		case eActor::SC_FORCE_REMOVE_SKILL:
			{
				CPacketCompressStream Stream( pPacket, 128 );

				int nSkillID;
				Stream.Read( &nSkillID , sizeof(int) );
				RemoveSkill(nSkillID);
			}
			break;

#if defined(PRE_ADD_MAILBOX_OPEN) || defined(PRE_ADD_68286)
		case eActor::SC_MAILBOX_OPEN:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				bool isOpen = false;

				Stream.Read(&isOpen, sizeof(bool));
				
				SetMailBoxOpenByShortCutKey(isOpen);
			}
			break;
		case eActor::SC_MAILBOX_CLOSE:
			{
				CPacketCompressStream Stream( pPacket, 128 );
				bool isOpen = false;

				Stream.Read(&isOpen, sizeof(bool));

				SetMailBoxOpenByShortCutKey(isOpen);
			}
			break;
#endif // PRE_ADD_MAILBOX_OPEN

#if defined(PRE_FIX_68898)
		case eActor::SC_SKIP_END_ACTION:
			{
				bool isSkipEndAction = false;

				CPacketCompressStream Stream( pPacket, 128 );
				Stream.Read( &isSkipEndAction, sizeof(bool) );

				if (isSkipEndAction == true)
					SetSkipEndAction(isSkipEndAction);
			}
			break;
#endif // PRE_FIX_68898
	}
}

void CDnActor::CmdWarp( EtVector3 &vPos, EtVector2 &vLook )
{	
	SetPosition( vPos );
	SetPrevPosition( vPos );
	if( EtVec2LengthSq( &vLook ) > 0.f )
		Look( vLook );

	if( abs( CDnWorld::GetInstance().GetHeight( vPos ) - vPos.y ) > 5.f ) {
		SetVelocityY( -3.0f );
		SetResistanceY( -18.0f );
	}
}


void CDnActor::OnInitializeNextStage( void )
{
	// Turn off active passive action skill
	if( m_hProcessSkill )
	{
		if( (m_hProcessSkill->GetSkillType() == CDnSkill::Passive || m_hProcessSkill->GetSkillType() == CDnSkill::AutoPassive) &&
			 m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
		{
			m_hProcessSkill->OnEnd( 0, 0.0f );
			m_hProcessSkill.Identity();
		}
	}

	// Also turn off active aura skill
	if( m_hAuraSkill )
	{
		OnSkillAura( m_hAuraSkill, false );
	}

	// It's rare to change maps while frozen, but if it happens, remove the freeze state effect.
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_041 ) )
		m_pStateBlow->RemoveStateBlowByBlowDefineIndex( STATE_BLOW::BLOW_041 );

	m_iCantActionReferenceCount = 0;
	m_iCantMoveReferenceCount = 0;

	// Call CreateObject for weapon. (Creating EtcObject for light effects causes it to be deleted during CDnEtcObject::ReleaseClass.)
	for( int i=0; i<2; i++ ) {
		if( m_hWeapon[i] ) {
			m_hWeapon[i]->LoadEtcObject();
		}
	}
}


void CDnActor::OnInitializePVPRoundRestart( void )
{
	// End active passive action if any.
	if( m_hProcessSkill )
	{
		if( (m_hProcessSkill->GetSkillType() == CDnSkill::Passive || m_hProcessSkill->GetSkillType() == CDnSkill::AutoPassive) &&
			 m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
		{
			m_hProcessSkill->OnEnd( 0, 0.0f );
			m_hProcessSkill.Identity();
		}
	}

	// End item skill
	if( m_hItemSkill )
	{
		if( m_hItemSkill->IsFinished() )
		{
			m_hItemSkill->OnEnd( 0, 0.f );

			deque<DnSkillHandle>::iterator iter = find( m_dqhItemSkillList.begin(), m_dqhItemSkillList.end(), m_hItemSkill );
			_ASSERT( m_dqhItemSkillList.end() != iter );
			m_dqhItemSkillList.erase( iter );

			SAFE_RELEASE_SPTR( m_hItemSkill );
		}
	}

	// End active aura skill if any.
	if( m_hAuraSkill )
		OnSkillAura( m_hAuraSkill, false );

	// Terminate any active toggle skills.
	if( m_hToggleSkill )
		OnSkillToggle( m_hToggleSkill, false );

	// Reset all state effects
	RemoveAllBlowExpectPassiveSkill();
	
	if( IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(this);
		pPlayer->RemoveAllBubbles( false );
	}
}


void CDnActor::GetProperlyDieAction( /*IN OUT*/ string& strDieAction )
{
	strDieAction.assign( "Die" );
	const char* pCurrentAction = GetCurrentAction();
	if( pCurrentAction )
	{
		bool bCheckDie = false;
		if( IsStandHit( pCurrentAction ) )
			strDieAction = "Die", bCheckDie = true;
		if( IsStun( pCurrentAction ) && !IsAir( strDieAction.c_str() ) ) 
			strDieAction = "Die", bCheckDie = true;

		if( !bCheckDie ) {
			if( IsAir( pCurrentAction ) ) {
				if( IsExistAction( "Die_Air" ) ) strDieAction = "Die_Air";
			}
			if( IsDown( pCurrentAction ) ) {
				if( IsExistAction( "Die_Down" ) ) strDieAction = "Die_Down";
			}
		}
	}
}

void CDnActor::SetProcess( bool bFlag )
{
	if( IsProcess() == bFlag ) return;

	CDnUnknownRenderObject::SetProcess( bFlag );

	if( IsProcess() ) InsertOctreeNode();
	else RemoveOctreeNode();
}

CDnRenderBase *CDnActor::GetRenderBase()
{
	CDnRenderBase *pRender = dynamic_cast<CDnRenderBase *>(this);
	return pRender;
}

void CDnActor::ProcessPress()
{
	if( m_nPressLevel == 0 ) return;
	if( IsDie() ) return;
	if( IsFly() ) return;
	if( IsUnderGround() ) return;

	DNVector(DnActorHandle) hVecList;
	EtVector3 vAniDist = *GetPosition();
	vAniDist.x -= GetAniDistance()->x;
	vAniDist.z -= GetAniDistance()->z;

	float fScanRadius = 0.0f;
	if( m_Press == Press_Circle ) fScanRadius = (float)GetUnitSize() * 2.f;
	else if( m_Press == Press_Capsule ) fScanRadius = (float)GetUnitSize()*2.0f + 50.f;

	int nCount = ScanActor( vAniDist, fScanRadius, hVecList );
	if( nCount > 0 ) {
		for( int i=0; i<nCount; i++ ) {
			if( !hVecList[i] ) continue;
			if( hVecList[i] == GetMySmartPtr() ) continue;
			if( hVecList[i]->IsDie() ) continue;
			if( hVecList[i]->GetPressLevel() > m_nPressLevel ) continue;
			if( hVecList[i]->GetPressLevel() == -1 ) continue;
			if( abs( GetPosition()->y - hVecList[i]->GetPosition()->y ) > GetHeight() ) continue;
			if( m_Press == Press_NoneSameCircle && hVecList[i]->m_Press == Press_NoneSameCircle ) continue;

			ProcessPress( hVecList[i] );
		}
	}
}

void CDnActor::ProcessPress( DnActorHandle hTarget )
{
	bool bCheckPressed = false;
	EtVector2 Dir;
	float Dist = 0.0f;
	if( GetPress() == CDnActorState::Press_Circle && hTarget->GetPress() == CDnActorState::Press_Circle ) {
		if( CheckPressCircle2Clrcle2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			if( CDnActor::s_hLocalActor && hTarget == CDnActor::s_hLocalActor ) bCheckPressed = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Capsule && hTarget->GetPress() == CDnActorState::Press_Capsule ) {
		if( CheckPressCapsule2Capsule2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			if( CDnActor::s_hLocalActor && hTarget == CDnActor::s_hLocalActor ) bCheckPressed = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Capsule && hTarget->GetPress() == CDnActorState::Press_Circle ) {
		if( CheckPressCapsule2Circle2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			if( CDnActor::s_hLocalActor && hTarget == CDnActor::s_hLocalActor ) bCheckPressed = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Circle && hTarget->GetPress() == CDnActorState::Press_Capsule ) {
		if( CheckPressCapsule2Circle2( hTarget, GetMySmartPtr(), Dir, Dist ) ) {
			hTarget->MoveToWorld( -Dir * Dist );
			if( CDnActor::s_hLocalActor && hTarget == CDnActor::s_hLocalActor ) bCheckPressed = true;
		}
	}
	if( bCheckPressed ) {
		((CDnLocalPlayerActor*)hTarget.GetPointer())->AddPressedCount( (Dist > 10.0f) ? 10 : 1 );
	}
}

void CDnActor::ProcessSoftAppear( LOCAL_TIME LocalTime )
{
	if( m_SoftAppearStep == AS_NONE || m_SoftAppearStep == AS_SUSPEND ) {
		return;
	}
	if( !IsShow() ) {
		return;
	}
	const int APPEAR_TIME = 500;
	const int DISAPPEAR_TIME = 500;

	if( m_SoftTime == 0 ) m_SoftTime = LocalTime;

	if( m_SoftAppearStep == AS_APPEAR ) {
		float fAlpha = (float)(LocalTime - m_SoftTime) / APPEAR_TIME;
		if( fAlpha > 1.0f ) {
			SetAlphaBlend( 1.0f , AL_APPEAR );
			m_SoftAppearStep = AS_SUSPEND;
			m_SoftTime = 0;
		}
		else {
			SetAlphaBlend( fAlpha , AL_APPEAR );
		}
	}
	else if( m_SoftAppearStep == AS_DISAPPEAR ) {
		float fAlpha = 1.0f - (float)(LocalTime - m_SoftTime) / DISAPPEAR_TIME;
		if( fAlpha < 0.0f ) {
			SetAlphaBlend( 0.0f , AL_APPEAR );
			m_SoftAppearStep = AS_NONE;
			m_SoftTime = 0;
			CDnUnknownRenderObject<CDnActor>::SetDestroy();
		}
		else {
			SetAlphaBlend(  fAlpha , AL_APPEAR );
		}
	}
}

void CDnActor::SetDestroy()
{
	if( m_SoftAppearStep != AS_NONE  ) {
		if( m_SoftAppearStep != AS_DISAPPEAR )
			m_SoftTime = 0;
		m_SoftAppearStep = AS_DISAPPEAR;
		SetUniqueID( -1 );
	}
	else {
		CDnUnknownRenderObject<CDnActor>::SetDestroy();
	}
}

bool CDnActor::CheckPressCircle2Clrcle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist )
{
	EtVector2 vDir;
	int nSumSize;
	vDir = EtVec3toVec2( *hActor2->GetPosition() - *hActor1->GetPosition() );
	nSumSize = hActor1->GetUnitSize() + hActor2->GetUnitSize();
	float fLength = D3DXVec2Length( &vDir );
	if( fLength > (float)nSumSize ) return false;

	D3DXVec2Normalize( &Dir, &vDir );
	Dist = (float)( nSumSize - fLength );
	return true;
}

bool CDnActor::CheckPressCapsule2Capsule2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist )
{
	// Compute own world-space capsule area (straight line)
	int nSizeX = hActor1->GetUnitSizeParam1();
	int nSizeZ = hActor1->GetUnitSizeParam2();
	EtVector3 vCapsule1, vCapsule2;
	int nSumSize = 0;
	if( nSizeX < nSizeZ ) {
		vCapsule1.x = vCapsule2.x = vCapsule1.y = vCapsule2.y = 0.0f;
		vCapsule1.z = (float)(nSizeZ - nSizeX);
		vCapsule2.z = -vCapsule1.z;
		nSumSize += nSizeX;
	}
	else {
		vCapsule1.z = vCapsule2.z = vCapsule1.y = vCapsule2.y = 0.0f;
		vCapsule1.x = (float)(nSizeX - nSizeZ);
		vCapsule2.x = -vCapsule1.x;
		nSumSize += nSizeZ;
	}
	EtMatrix matWorld = *hActor1->GetMatEx();
	EtVec3TransformCoord( &vCapsule1, &vCapsule1, &matWorld );
	EtVec3TransformCoord( &vCapsule2, &vCapsule2, &matWorld );
	EtVector2 vLine1, vLine2;
	vLine1 = EtVec3toVec2( vCapsule1 );
	vLine2 = EtVec3toVec2( vCapsule2 );

	// Get target's world-space capsule area (line)
	int nTargetSizeX = hActor2->GetUnitSizeParam1();
	int nTargetSizeZ = hActor2->GetUnitSizeParam2();
	EtVector3 vTargetCapsule1, vTargetCapsule2;
	if( nTargetSizeX < nTargetSizeZ ) {
		vTargetCapsule1.x = vTargetCapsule2.x = vTargetCapsule1.y = vTargetCapsule2.y = 0.0f;
		vTargetCapsule1.z = (float)(nTargetSizeZ - nTargetSizeX);
		vTargetCapsule2.z = -vTargetCapsule1.z;
		nSumSize += nTargetSizeX;
	}
	else {
		vTargetCapsule1.z = vTargetCapsule2.z = vTargetCapsule1.y = vTargetCapsule2.y = 0.0f;
		vTargetCapsule1.x = (float)(nTargetSizeX - nTargetSizeZ);
		vTargetCapsule2.x = -vTargetCapsule1.x;
		nSumSize += nTargetSizeZ;
	}
	EtMatrix matTargetWorld = *hActor2->GetMatEx();
	EtVec3TransformCoord( &vTargetCapsule1, &vTargetCapsule1, &matTargetWorld );
	EtVec3TransformCoord( &vTargetCapsule2, &vTargetCapsule2, &matTargetWorld );
	EtVector2 vTargetLine1, vTargetLine2;
	vTargetLine1 = EtVec3toVec2( vTargetCapsule1 );
	vTargetLine2 = EtVec3toVec2( vTargetCapsule2 );

	if( hActor1 == hActor2 ) return false;

	// After calculating the line-to-line distance
	EtVector2 vPressDir;
	float fLength = DistLine2DToLine2D( vLine1, vLine2, vTargetLine1, vTargetLine2, vPressDir );
	if( fLength > 0.0f ) {
		// If not intersecting, PressDir should be properly set.
		// And at this time, since it's a normal point-to-line distance, compare against SumSize.
		if( fLength > (float)nSumSize ) return false;
		Dir = vPressDir;
		Dist = (float)( nSumSize - fLength );
	}
	else {
		// If intersecting
		// Measure the distance using both endpoints of the first and second line segments, then
		EtVector2 vDir1, vDir2;
		float fDist1 = DistPointToLine2D( vTargetLine1, vLine1, vLine2, vDir1 );
		float fDist2 = DistPointToLine2D( vTargetLine2, vLine1, vLine2, vDir2 );

		// Store the shorter distance. Store the direction in reverse.
		if( fDist1 < fDist2 ) {
			vPressDir = -vDir1;
			fLength = fDist1;
		}
		else {
			vPressDir = -vDir2;
			fLength = fDist2;
		}

		// If intersecting, no need to compare distance with SumSize.
		// If already overlapping enough to intersect, the only option is to push out beyond minimum distance.
		Dir = vPressDir;
		Dist = (float)( nSumSize + fLength );
	}
	
	return true;
}

bool CDnActor::CheckPressCapsule2Circle2( DnActorHandle hActor1, DnActorHandle hActor2, EtVector2 &Dir, float &Dist )
{
	int nSizeX = hActor1->GetUnitSizeParam1();
	int nSizeZ = hActor1->GetUnitSizeParam2();
	EtVector3 vCapsule1, vCapsule2;
	int nSumSize;
	if( nSizeX < nSizeZ ) {
		vCapsule1.x = vCapsule2.x = vCapsule1.y = vCapsule2.y = 0.0f;
		vCapsule1.z = (float)(nSizeZ - nSizeX);
		vCapsule2.z = -vCapsule1.z;
		nSumSize = nSizeX + hActor2->GetUnitSize();
	}
	else {
		vCapsule1.z = vCapsule2.z = vCapsule1.y = vCapsule2.y = 0.0f;
		vCapsule1.x = (float)(nSizeX - nSizeZ);
		vCapsule2.x = -vCapsule1.x;
		nSumSize = nSizeZ + hActor2->GetUnitSize();
	}
	EtMatrix matWorld = *hActor1->GetMatEx();
	EtVec3TransformCoord( &vCapsule1, &vCapsule1, &matWorld );
	EtVec3TransformCoord( &vCapsule2, &vCapsule2, &matWorld );
	EtVector2 vLine1, vLine2;
	vLine1 = EtVec3toVec2( vCapsule1 );
	vLine2 = EtVec3toVec2( vCapsule2 );

	EtVector2 vTargetPos = EtVec3toVec2( *hActor2->GetPosition() );
	EtVector2 vPressDir;
	float fLength = DistPointToLine2D( vTargetPos, vLine1, vLine2, vPressDir );
	if( fLength > (float)nSumSize ) return false;

	Dir = vPressDir;
	Dist = (float)( nSumSize - fLength );
	return true;
}

#ifdef RENDER_PRESS_hayannal2009
void CDnActor::RenderPress()
{
	if( !s_bEnableRenderPress )
		return;

	EtMatrix matWorld = *GetMatEx();
	unsigned short Indices[] = {
		0, 1, 1, 2, 2, 0,
		0, 2, 2, 3, 3, 0,
		0, 3, 3, 4, 4, 0,
		0, 4, 4, 5, 5, 0,
		0, 5, 5, 6, 6, 0,
		0, 6, 6, 7, 7, 0,
		0, 7, 7, 8, 8, 0,
		0, 8, 8, 9, 9, 0,
	};

	if( m_Press == Press_Circle ) {
		// Must draw with DrawLine3D to render as wireframe.
		for( int i = 0; i < 8 * 3; ++i )
			EternityEngine::DrawLine3D( m_PressVertices1[Indices[i*2]].Position, m_PressVertices1[Indices[i*2+1]].Position, m_PressVertices1[Indices[i]].dwColor, &matWorld );
	}
	else if( m_Press == Press_Capsule ) {
		for( int i = 0; i < 8 * 3; ++i )
			EternityEngine::DrawLine3D( m_PressVertices1[Indices[i*2]].Position, m_PressVertices1[Indices[i*2+1]].Position, m_PressVertices1[Indices[i]].dwColor, &matWorld );
		for( int i = 0; i < 8 * 3; ++i )
			EternityEngine::DrawLine3D( m_PressVertices2[Indices[i*2]].Position, m_PressVertices2[Indices[i*2+1]].Position, m_PressVertices2[Indices[i]].dwColor, &matWorld );
		for( int i = 0; i < 2 * 3; ++i )
			EternityEngine::DrawLine3D( m_PressVertices3[Indices[i*2]].Position, m_PressVertices3[Indices[i*2+1]].Position, m_PressVertices3[Indices[i]].dwColor, &matWorld );
	}

//#define _VOICE_TEST
#ifdef _VOICE_TEST
	EtMatrix matWorldPos;
	EtMatrixIdentity( &matWorldPos );
	matWorldPos._41 = matWorld._41;
	matWorldPos._42 = matWorld._42 + 20.0f;
	matWorldPos._43 = matWorld._43;

	SPrimitiveDraw3D WorldVertices1[6];
	WorldVertices1[ 0 ].Position = EtVector3( 0.0f, 0.0f, 0.0f );
	WorldVertices1[ 1 ].Position = EtVector3( 60.0f, 0.0f, 0.0f );
	WorldVertices1[ 2 ].Position = EtVector3( 0.0f, 0.0f, 0.0f );
	WorldVertices1[ 3 ].Position = EtVector3( 0.0f, 60.0f, 0.0f );
	WorldVertices1[ 4 ].Position = EtVector3( 0.0f, 0.0f, 0.0f );
	WorldVertices1[ 5 ].Position = EtVector3( 0.0f, 0.0f, 60.0f );
	WorldVertices1[ 0 ].dwColor = 0xFFFF0000;
	WorldVertices1[ 1 ].dwColor = 0xFFFF0000;
	WorldVertices1[ 2 ].dwColor = 0xFF00FF00;
	WorldVertices1[ 3 ].dwColor = 0xFF00FF00;
	WorldVertices1[ 4 ].dwColor = 0xFF0000FF;
	WorldVertices1[ 5 ].dwColor = 0xFF0000FF;

	//GetEtDevice()->SetWorldTransform( &matWorldPos );
	//GetEtDevice()->DrawPrimitiveUP( PT_LINELIST, 3, WorldVertices1, sizeof( SPrimitiveDraw3D ) );

	EternityEngine::DrawLine3D( WorldVertices1[0].Position, WorldVertices1[1].Position, WorldVertices1[0].dwColor, &matWorldPos );
	EternityEngine::DrawLine3D( WorldVertices1[2].Position, WorldVertices1[3].Position, WorldVertices1[2].dwColor, &matWorldPos );
	EternityEngine::DrawLine3D( WorldVertices1[4].Position, WorldVertices1[5].Position, WorldVertices1[4].dwColor, &matWorldPos );

	EtVector3 vPos[2];
	vPos[0] = GetMatEx()->m_vPosition;
	vPos[1] = GetMatEx()->m_vPosition + ( GetMatEx()->m_vZAxis * 100.f );
	for( int i=0; i<2; i++ ) vPos[i].y += 50.f;
	EternityEngine::DrawLine3D( vPos[0], vPos[1], 0xffffffff );
	vPos[0] = GetMatEx()->m_vPosition;
	vPos[1] = GetMatEx()->m_vPosition + ( GetMatEx()->m_vXAxis * 100.f );
	for( int i=0; i<2; i++ ) vPos[i].y += 50.f;
	EternityEngine::DrawLine3D( vPos[0], vPos[1], 0xffffffff );
#endif
}
#endif

void CDnActor::AddBindeActionObjectHandle( DnEtcHandle hHandle )
{ 
	m_vecBindActionObjectHandle.push_back(hHandle);
}

void CDnActor::ReleaseBindActionObject(const char* szPrevAction)
{

#if defined( PRE_ADD_ACTION_OBJECT_END_ACTION )
	for( DWORD i=0; i<m_vecBindActionObjectHandle.size(); i++ )
	{
		if( m_vecBindActionObjectHandle[i] && m_vecBindActionObjectHandle[i]->IsDestroy() == false )
		{
			ActionElementStruct *pStruct = GetElement( szPrevAction );
			if( pStruct && strcmp ( GetCurrentAction() , pStruct->szNextActionName.c_str() ) == NULL )
			{
				m_vecBindActionObjectHandle[i]->SetDestroy();
			}
			else if( m_vecBindActionObjectHandle[i]->IsExistAction( "End_Action" ) == true )
			{
				if( strcmp( m_vecBindActionObjectHandle[i]->GetCurrentAction(), "End_Action" ) != NULL )
					m_vecBindActionObjectHandle[i]->SetActionQueue( "End_Action" );
			}
			else
			{
				m_vecBindActionObjectHandle[i]->SetDestroy();
			}
		}
	}
#else
	// When the action that created an action object is interrupted, make the created action object disappear as well. (#14162)
	int nActionIndex = GetElementIndex( szPrevAction );
	if( m_mapBindingActionObjects.end() != m_mapBindingActionObjects.find( nActionIndex ) )
	{
		vector<CDnActionSignalImp*>& vlEtcObjects = m_mapBindingActionObjects[ nActionIndex ];
		int iNumActionObjects = (int)vlEtcObjects.size();
		for( int i = 0; i < iNumActionObjects; ++i )
		{
			CDnActionSignalImp* pActionObject = vlEtcObjects.at( i );
			if( pActionObject )
				pActionObject->ReleaseSignalImp( CDnActionSignalImp::EtcObject );
		}
		vlEtcObjects.clear();
	}
#endif

	m_vecBindActionObjectHandle.clear();
}

void CDnActor::SyncBindActionObjectFPS( float fFPS )
{
	for( DWORD i=0; i<m_vecBindActionObjectHandle.size(); i++ )
	{
		if( m_vecBindActionObjectHandle[i] )
			m_vecBindActionObjectHandle[i]->CDnActionBase::SetFPS( fFPS );
	}
}

void CDnActor::OnChangeAction( const char *szPrevAction )
{
	ReleaseBindActionObject(szPrevAction);

	if( szPrevAction == NULL )
		return;

	CDnActionSignalImp::OnChangeAction( szPrevAction );
#ifdef _SOCIAL_NEXT_ACTION_KALLISTE
	if (m_SocialActCtrler.IsEmpty() == false)
	{
		if (m_SocialActCtrler.nCondition == 1)	// FrameEnd - todo : to string
		{
			SetFaceAction(m_SocialActCtrler.szNextActionName.c_str());
			m_SocialActCtrler.Clear();
		}
	}
#endif
	// Skip if repeating the same action
	if( szPrevAction && strcmp(GetCurrentAction(), szPrevAction) == 0 )
		return;

	if( m_hProcessSkill )
	{
		// if instantly passive skill, then cancel skill. ( ex) archer's spinkick)
		// because state effect must be deactivate when change to another attack action.
		set<string> setPrevAction;
		setPrevAction.insert( szPrevAction );

		// If the current action is the next action of the prev action, consider the skill to be chaining.
		// If previous action was used by the currently active skill and current action is not the next action of previous action
		// Consider the passive skill finished.
		ActionElementStruct* pElement = GetElement( szPrevAction );
		bool bIsNextAction = false;
		if( pElement )
		{
			// #25154 Default Stand action is not treated as continuing the next action specified by the skill.
			// When this function is called after aura skill action ends, current action is Stand at that point
			// m_hProcessSkill should be considered finished and set to NULL. 
			// m_hProcessSkill must not remain, as it could be forcibly ended (onend) when another skill is used.
			// Therefore bIsNextAction becomes false, and inside m_hProcessSkill->IsUseSkillActionNames()
			// The skill action should be considered finished.
			bIsNextAction = ((pElement->szNextActionName != "Stand") && (pElement->szNextActionName == GetCurrentAction()));
		}

		if( false == bIsNextAction &&
			m_hProcessSkill->IsUseActionNames( setPrevAction ) )
		{
			// When an active skill is registered and used as a passive, it can be checked with GetPassiveSkillLengh().
			if( ( m_hProcessSkill->GetPassiveSkillLength() != 0.f || m_hProcessSkill->GetSkillType() == CDnSkill::Passive ) &&
				m_hProcessSkill->GetDurationType() == CDnSkill::Instantly )
			{
				m_hProcessSkill->OnEnd( MAActorRenderBase::m_LocalTime, 0.0f );
				m_hProcessSkill.Identity();
			}
			else if( IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn() )
			{
				// Note Hanki: m_hProcessSkill smart pointer must remain valid while the aura skill action is playing
				// Filtered in CDnPlayerActor::CmdStop() on the game server, so on the game server the action signal to the end 
				// Processed. Therefore, make ProcessAction an identity in CDnActor::OnChangeAction.
				m_hProcessSkill.Identity();
			}
		}
	}
}

void CDnActor::OnDie( DnActorHandle hHitter )
{
	m_fDieDelta = m_fMaxDieDelta;
}


void CDnActor::InsertUniqueSearchMap( CDnActor *pActor )
{
	ScopeLock<CSyncLock> Lock(s_LockSearchMap);
	if( pActor->GetUniqueID() == -1 ) return;
	s_dwMapActorSearch.insert( make_pair( pActor->GetUniqueID(), pActor->GetMySmartPtr() ) );
}

void CDnActor::RemoveUniqueSearchMap( CDnActor *pActor )
{
	ScopeLock<CSyncLock> Lock(s_LockSearchMap);
	if( pActor->GetUniqueID() == -1 ) return;
	std::map<DWORD, DnActorHandle>::iterator it;
	it = s_dwMapActorSearch.find( pActor->GetUniqueID() );
	if( it != s_dwMapActorSearch.end() ) {
		s_dwMapActorSearch.erase( it );
	}
}

void CDnActor::SetUniqueID( DWORD dwUniqueID )
{ 
	RemoveUniqueSearchMap( this );
	m_dwUniqueID = dwUniqueID;
	InsertUniqueSearchMap( this );
}

void CDnActor::RenderCustom( float fElapsedTime )
{
	if( m_bShowHeadName && IsShow() ) {
		SAABox box;
		GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y + GetIncreaseHeight();

		EtVector3 vPos = m_matexWorld.m_vPosition;
		vPos.y += fHeight;

		EtMatrix HeadMat = GetBoneMatrix("Bip01 Head");
		EtVector3 vHeadPos = *(EtVector3*)&HeadMat._41;
		m_ChatBalloon.RenderChatBalloon( vPos, vHeadPos, fElapsedTime );

	}
	MAHeadNameRender::Render();
}

void CDnActor::CalcCustomRenderDepth()
{
	DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
	if( hCamera ) {
		SAABox box;
		GetBoundingBox(box);
		float fHeight = box.Max.y - box.Min.y + m_fIncreaseHeight;
		EtVector3 vPos = m_matexWorld.m_vPosition;
		vPos.y += fHeight;
		m_fCustomRenderDepth = EtVec3Dot(&vPos, &hCamera->GetMatEx()->m_vZAxis);
	}
}

bool CDnActor::bIsAppliedSkill( const int iSkillIndex )
{
	if( iSkillIndex == 0 )
		return true;

	for( int i=0 ; i<GetNumAppliedStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = GetAppliedStateBlow( i );
		if( hBlow )
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if( !pSkillInfo )
				continue;

			if( pSkillInfo->iSkillID == iSkillIndex )
				return true;
		}
	}

	return false;
}
 

bool CDnActor::IsAppliedThisStateBlow( STATE_BLOW::emBLOW_INDEX iBlowIndex )
{
	if( !m_pStateBlow )
		return false;

	return m_pStateBlow->IsApplied( iBlowIndex );
}


void CDnActor::PreThreadRelease()
{
	SetProcess( false );
	EraseCustomRenderList();
}

void CDnActor::InitializeNextStage()
{
	for( DWORD i=0; i<s_pVecProcessList.size(); i++ ) 
	{
		if( !s_pVecProcessList[i]->IsPlayerActor() ) 
		{
			s_pVecProcessList[i]->Release();
			i--;
		}
	}
}

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
void CDnActor::CheckPreSignal( ActionElementStruct *pElement, int nElementIndex, CEtActionSignal *pSignal, int nSignalIndex, CEtActionBase *pActionBase )
{
	switch( pSignal->GetSignalIndex() ) {
		case STE_Sound:
			{
				std::string szTemp = pElement->szName;
				ToLowerA( szTemp );
				if( strstr( szTemp.c_str(), "skill_" ) ) {
					SoundStruct *pStruct = (SoundStruct *)pSignal->GetData();
					pStruct->bLoadingComplete = false;
					pStruct->nVecSoundIndex = new std::vector<int>;
					return;
				}
			}
			break;
		case STE_ActionObject:
			break;
		case STE_Particle:
			break;
		case STE_FX:
			break;
		case STE_Decal:
			break;
		case STE_Projectile:
			break;
#ifdef PRE_ADD_MARK_PROJECTILE
		case STE_MarkProjectile:
			break;
#endif // PRE_ADD_MARK_PROJECTILE
	}
	CDnActionBase::CheckPreSignal( pElement, nElementIndex, pSignal, nSignalIndex, pActionBase );
}
#endif

#ifndef PRE_FIX_CLIENT_MEMOPTIMIZE
void CDnActor::FlushSoundSignal()
{
	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		ActionElementStruct *pElement = (*m_pVecActionElementList)[i];

		std::string szTemp = pElement->szName;
		ToLowerA( szTemp );
		if( strstr( szTemp.c_str(), "skill_" ) ) {
			for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
				CEtActionSignal *pSignal = pElement->pVecSignalList[j];
				if( pSignal->GetSignalIndex() == STE_Sound ) {
					CheckPostSignal( pElement, i, pSignal, j, NULL );
					CheckPreSignal( pElement, i, pSignal, j, NULL );
				}
			}
		}
	}
}

void CDnActor::PreloadSkillSoundSignal()
{
	for( DWORD i=0; i<m_pVecActionElementList->size(); i++ ) {
		ActionElementStruct *pElement = (*m_pVecActionElementList)[i];

		std::string szTemp = pElement->szName;
		ToLowerA( szTemp );
		if( strstr( szTemp.c_str(), "skill_" ) && IsUsingSkillAction( pElement->szName.c_str() ) ) {
			for( DWORD j=0; j<pElement->pVecSignalList.size(); j++ ) {
				CEtActionSignal *pSignal = pElement->pVecSignalList[j];
				if( pSignal->GetSignalIndex() == STE_Sound ) {
					LoadSoundStruct( (SoundStruct*)pSignal->GetData() );
				}
			}
		}
	}
}
#endif

void CDnActor::DeleteAllActor()
{
	ScopeLock<CSyncLock> Lock( s_SmartPtrLock );
	for( int i=0; i<CDnActor::GetItemCount(); i++ ) {
		DnActorHandle hActor = CDnActor::GetItem(i);
		if( !hActor ) continue;
		if( !hActor->IsPlayerActor() ) continue;
		if( ((CDnPlayerActor*)hActor.GetPointer())->IsDeveloperAccountLevel() ) {
			SAFE_RELEASE_SPTR( hActor );
			i--;
		}
	}
}

void CDnActor::MakePassiveSkillActionInfo()
{
	for( int i = 0; i < (int)GetElementCount(); ++i )
	{
		CEtActionBase::ActionElementStruct* pElement = GetElement( i );
		for( int k = 0; k < (int)pElement->pVecSignalList.size(); ++k )
		{
			CEtActionSignal* pSignal = pElement->pVecSignalList.at(k);
			if( STE_InputHasPassiveSkill == pSignal->GetSignalIndex() )
			{	
				// Collect action indices based on passive skill index to gather action names of owned skills
				InputHasPassiveSkillStruct* pInfo = (InputHasPassiveSkillStruct*)pSignal->GetData();
				if( false == pInfo->bOnlyCheck )
					m_mapPassiveSkillActions[ pInfo->nSkillIndex ].insert( pInfo->szChangeAction );
			}
		}
	}
}

void CDnActor::FillPassiveSkillActions( int iSkillID, set<string>& setUseActionNames )
{
	map<int, set<string> >::iterator iter = m_mapPassiveSkillActions.find( iSkillID );
	if( m_mapPassiveSkillActions.end() != iter )
	{
		const set<string>& setActions = iter->second;
		set<string>::const_iterator iterSet = setActions.begin();
		for( iterSet; iterSet != setActions.end(); ++iterSet )
		{
			setUseActionNames.insert( *iterSet );
		}
	}
}

void CDnActor::FindSkillBySkillType(CDnSkill::SkillTypeEnum eSkillType, DNVector(DnSkillHandle)& vlSkillList)
{
#ifndef PRE_FIX_SKILLLIST
	for( DWORD i=0; i<m_vlhSkillList.size(); i++ ) 
	{
		// If toggle or aura skill, remove from the separate list
		DnSkillHandle hSkill = m_vlhSkillList.at(i);
#else
	for( DWORD i = 0; i < GetSkillCount(); ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
#endif // #ifndef PRE_FIX_SKILLLIST
		if(!hSkill)
			continue;

		if (eSkillType == hSkill->GetSkillType())
			vlSkillList.push_back(hSkill);
	}
}

void CDnActor::OnLoopAction( float fFrame, float fPrevFrame )
{

}



bool CDnActor::CheckAnimation( const char *szActionName )
{
	const char *szName = szActionName;

	if( m_szActionQueue.empty() ) {
		if( strcmp( szName, m_szAction.c_str() ) == NULL ) 
			return false;
	}
	else {
		if( strcmp( szName, m_szActionQueue.c_str() ) == NULL ) 
			return false;
	}

	return true;
}

void CDnActor::CmdFinishAuraSkill(DWORD dwSkillID)
{
	DnSkillHandle hSkill = FindSkill(dwSkillID);
	if (hSkill)
	{
		// [2011/03/22 semozz]
		// If the current skill is the same as the aura skill, reset the current skill as well.
		if (m_hProcessSkill == hSkill)
			m_hProcessSkill.Identity();

		OnSkillAura(hSkill, false);
	}
}

const char *CDnActor::CheckChangeActionBlow( const char* szActionName )
{
	const char *szResultActionName = szActionName;
//	std::string szResultAction = szActionName;
	// State effect #129: action name replacement
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_129 ) )
	{
		DNVector(DnBlowHandle) vlhChangeActionSetBlow;
		m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_129, vlhChangeActionSetBlow );

		// There can be multiple action set change state effects.
		int iNumBlow = (int)vlhChangeActionSetBlow.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			CDnChangeActionSetBlow* pChangeActionSetBlow = static_cast<CDnChangeActionSetBlow*>( vlhChangeActionSetBlow.at(i).GetPointer() );
			if( STATE_BLOW::STATE_END != pChangeActionSetBlow->GetBlowState() )
			{
				CDnChangeActionStrProcessor* pProcessor = pChangeActionSetBlow->GetChangeActionStrProcessor();
				if( pProcessor )
				{
					// #31697 If it is a player and switched to a specific weapon action, restore it before calling.
					if( this->IsPlayerActor() )
					{
						CDnPlayerActor* pPlayerActor = dynamic_cast<CDnPlayerActor*>(this);
						if( pPlayerActor->IsChangedEquipAction() )
						{
							szResultActionName = pPlayerActor->GetOriginalNoneActionEquipAction();
						}
					}
					//////////////////////////////////////////////////////////////////////////

					string strNowActionName(szResultActionName);
					const char* pChangeActionName = pProcessor->GetChangeActionName( strNowActionName );
					if( pChangeActionName )
					{
						szResultActionName = pChangeActionName;

						// If the action changed, exit the loop. Check state effect #129 next.
						// Multiple #129 state effects changing the same action are not supported.
						break;
					}
					else
					{
						// #32234 If not changed, revert to the original action.
						szResultActionName = szActionName;
					}
				}
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////

	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_121 ) )
	{
		if( 0 == strcmp(szActionName, "Stand") )
		{
			DNVector(DnBlowHandle) vlhChangeStandActionBlow;
			m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_121, vlhChangeStandActionBlow );
			if( false == vlhChangeStandActionBlow.empty() )
			{
				// Always treat as only one.
				_ASSERT( 1 == vlhChangeStandActionBlow.size() );
				if( STATE_BLOW::STATE_END != vlhChangeStandActionBlow.front()->GetBlowState() )
					szResultActionName = vlhChangeStandActionBlow.front()->GetValue();
			}
		}
	}

	return szResultActionName;
}


void CDnActor::ForceBeginStateBlow(DnBlowHandle hBlow)
{
	if (!hBlow)
		return;
	
	if( hBlow->IsUseTableDefinedGraphicEffect() )
		hBlow->AttachGraphicEffectDefaultType();

	hBlow->OnBegin(CDnActionBase::m_LocalTime, 0.0f);
	OnBeginStateBlow( hBlow );

	// Check because some blows set state to end immediately.
	if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
		hBlow->SetState( STATE_BLOW::STATE_DURATION );
}

void CDnActor::AddStateBlowSignal(DnBlowHandle hBlow)
{
	if (m_pStateBlowSignalProcessor && hBlow)
		m_pStateBlowSignalProcessor->AddBlow(hBlow);
}

void CDnActor::RemoveStateBlowSignal(DnBlowHandle hBlow)
{
	if (m_pStateBlowSignalProcessor && hBlow)
		m_pStateBlowSignalProcessor->RemoveBlow(hBlow);
}


void CDnActor::ResetStateEffects()
{
	m_setNowPlayingEffectSkill.clear();

	if (m_pStateBlow)
		m_pStateBlow->RestoreAllBlowGraphicEffect();
}

void CDnActor::ActionChange(int nActionIndex)
{
	if (nActionIndex == -1)
		return;

	ActionElementStruct *pActionElement = GetElement(nActionIndex);
	if (pActionElement == NULL)
		return;

	std::string szAction = pActionElement->szName;
	CmdAction( szAction.c_str(), 0, 3.0f, true, true, false );

	// #37325 [Cannot get up after Wax] For actions including down, <m_fDownDelta should be included; normally set when calculated by Hit, however, 
	// In this case m_fDownDelta may not be set; if 0, ProcessDown() does not run, so for actions including down, check down delta and set it if missing.
	if((GetState() & CDnActorState::Down) && GetDownRemainDelta() <= 0) 
		SetDownRemainDelta(0.1f); // Exceptional case, so make it stand up immediately.
}


void CDnActor::CreateChainAttackProjectile(CPacketCompressStream& Stream)
{
	DWORD dwRootAttackerID = 0;
	DWORD dwPrevAttackerActorUniqueID = 0;
	DWORD dwActorToAttackID = 0;

	int iActionIndex = -1;
	int iProjectileSignalArrayIndex = -1;
	int iSkillID = -1;


	Stream.Read(&dwRootAttackerID, sizeof(DWORD));
	Stream.Read(&dwPrevAttackerActorUniqueID, sizeof(DWORD));
	Stream.Read(&dwActorToAttackID, sizeof(DWORD));

	Stream.Read(&iActionIndex, sizeof(int));
	Stream.Read(&iProjectileSignalArrayIndex, sizeof(int));
	Stream.Read(&iSkillID, sizeof(int));

	//1. Set projectile signal info.
	//////////////////////////////////////////////////////////////////////////
	DnActorHandle hRootAttacker = FindActorFromUniqueID(dwRootAttackerID);
	DnActorHandle hPrevActor = FindActorFromUniqueID(dwPrevAttackerActorUniqueID);
	DnActorHandle hActorToAttack = FindActorFromUniqueID(dwActorToAttackID);

	//PrevActor may not exist.
	if (!hRootAttacker || /*!hPrevActor ||*/ !hActorToAttack)
		return;

	int iRootAttackerTeam = hRootAttacker->GetTeam();

	
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
	
	//Prevent projectile creation if Signal info is not found... (related to client dump)
	if (bFound == false)
		return;
	
	//3. Configure projectile info and spawn
	//////////////////////////////////////////////////////////////////////////
	ProjectileStruct* pProjectileSignalInfo = &projectileSignalInfo;

	// Misc settings for firing projectiles.
	DnSkillHandle hSkill = hRootAttacker->FindSkill( iSkillID );

	// Set to designate the target via homing.
	pProjectileSignalInfo->nOrbitType = CDnProjectile::Homing;
	pProjectileSignalInfo->nTargetType = CDnProjectile::Target;
	pProjectileSignalInfo->VelocityType = CDnProjectile::Accell;
	pProjectileSignalInfo->fSpeed = 1000.0f;//CHAINATTACK_PROJECTILE_SPEED;
	pProjectileSignalInfo->nValidTime = 5000;


	CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( hRootAttacker, *GetMatEx(), pProjectileSignalInfo, EtVector3( 0.0f, 0.0f, 0.0f), hActorToAttack );
	if( pProjectile )
	{
		pProjectile->SetTargetActor( hActorToAttack );
		pProjectile->SetChainShooter( GetMySmartPtr() );
	}

	
	//////////////////////////////////////////////////////////////////////////
}

void CDnActor::RequestSummonOff(DWORD dwSummonMonsterUniqueID)
{
	BYTE pBuffer[64] = { 0, };
	CPacketCompressStream Stream( pBuffer, 64 );

	Stream.Write( &dwSummonMonsterUniqueID, sizeof(DWORD) );
	Send( eActor::CS_SUMMONOFF, &Stream );
}

EtVector3 CDnActor::FindAutoTargetPos()
{
	EtVector3 vTargetPosition = *GetPosition();

	if( !m_FindAutoTargetName.empty())
	{
		int nBoneIndex = GetBoneIndex( m_FindAutoTargetName.c_str() );
		if( nBoneIndex != -1 )
		{
			EtMatrix boneMatrix = GetBoneMatrix(m_FindAutoTargetName.c_str());
			vTargetPosition = *(EtVector3*)&boneMatrix._41;
		}
	}

	return vTargetPosition;
}


#if defined(PRE_FIX_51048)
void CDnActor::RemoveDebufAction(LOCAL_TIME LocalTime, float fDelta)
{
	//Freezing
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_041 ) )
	{
		DNVector(DnBlowHandle) vlBlows;
		GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_041, vlBlows );

		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
				hBlow->RemoveDebufAction(LocalTime, fDelta);
		}
	}

	//Frostbite
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_144 ) )
	{
		DNVector(DnBlowHandle) vlBlows;
		GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_144, vlBlows );

		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
				hBlow->RemoveDebufAction(LocalTime, fDelta);
		}
	}

	//ElectricShock
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_043 ) )
	{
		DNVector(DnBlowHandle) vlBlows;
		GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_043, vlBlows );

		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
				hBlow->RemoveDebufAction(LocalTime, fDelta);
		}
	}

	//Escape
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_218 ) )
	{
		DNVector(DnBlowHandle) vlBlows;
		GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_218, vlBlows );

		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
				hBlow->RemoveDebufAction(LocalTime, fDelta);
		}
	}

}
#endif // PRE_FIX_51048


void CDnActor::ScanActorByStateIndex(DNVector(DnActorHandle) &Veclist, STATE_BLOW::emBLOW_INDEX blowIndex)
{
	DNVector(DnActorHandle) scanActorList;
	ScanActor(*GetPosition(), FLT_MAX, scanActorList);
	int nListCount = (int)scanActorList.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnActorHandle hActor = scanActorList[i];
		if (hActor && hActor->IsDie() == false)
		{
			DNVector(DnBlowHandle) blowList;
			if (hActor->IsAppliedThisStateBlow(blowIndex))
				hActor->GetStateBlow()->GetStateBlowFromBlowIndex(blowIndex, blowList);

			int nBlowCount = (int)blowList.size();
			for (int j = 0; j < nBlowCount; ++j)
			{
				DnBlowHandle hBlow = blowList[j];
				if (hBlow && hBlow->IsEnd() == false)
				{
					//Add to the list if the caster of the actor's brand state effect is self.
					CDnSkill::SkillInfo *pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
					if (pSkillInfo && pSkillInfo->hSkillUser == GetMySmartPtr())
						Veclist.push_back(hActor);
				}
			}
		}
	}
}


// Signal Rotate.
void CDnActor::SetRotate( DWORD dwTime, float fStart, float fEnd, EtVector3 & vPos, bool bLeft )
{
	m_bRotate = true;
	m_dwRotateStartTime = CDnActionBase::m_LocalTime;
	m_dwRotateTime = dwTime;
	m_fStartSpeed = fStart;  m_fEndSpeed = fEnd;
	m_vRotAxis = vPos;  m_bRotLeft = bLeft;
}

// Rotate.
void CDnActor::ProcessRotate( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_bRotate )
		return;

	if( (LocalTime - m_dwRotateStartTime) > m_dwRotateTime )
	{
		m_bRotate = false;
		return;
	}

	if( m_dwRotateStartTime == 0 ) m_dwRotateStartTime = LocalTime;
	
	// Speed (rotation angle amount)	
	if( m_fStartSpeed == m_fEndSpeed )
	{
		m_fSpeedRot = m_fStartSpeed;
	}
	else
	{
		/*float frame = pObject->GetCurFrame();
		float w = ( frame - m_pSignal->GetStartFrame() ) / ( m_pSignal->GetEndFrame() - m_pSignal->GetStartFrame() );		
		m_fSpeedRot = m_fStartSpeed + ( ( m_fEndSpeed - m_fStartSpeed ) * w );		*/	
		m_fSpeedRot = m_fStartSpeed;
	}

	// Direction.
	if( m_bRotLeft )
		m_fSpeedRot = -m_fSpeedRot;

	m_fSpeedRot *= fDelta;

	//// Move rotation axis coordinates.
	m_matexWorld.MoveLocalXAxis( m_vRotAxis.x );
	m_matexWorld.MoveLocalYAxis( m_vRotAxis.y );
	m_matexWorld.MoveLocalZAxis( m_vRotAxis.z );

	// Rotation.
	m_matexWorld.RotateYaw( (-m_fSpeedRot) );
}

bool CDnActor::ProcessIgnoreGravitySignal()
{
	bool bIgnoreGravitySignal = false;
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_273 ) )
	{
		DNVector(DnBlowHandle) vecBlowList;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_273, vecBlowList );
		for( int i=0; i<(int)vecBlowList.size(); ++i )
		{
			DnBlowHandle hBlow = vecBlowList[i];
			if( hBlow && hBlow->CanBegin() )
			{
				bIgnoreGravitySignal = true;
				break;
			}
		}
	}

	return bIgnoreGravitySignal;
}

#if defined(PRE_FIX_61382)
//For puppet monsters, display the status effect on the owner actor.
DnActorHandle CDnActor::GetOwnerActorHandle(DnActorHandle hActor)
{
	DnActorHandle hOwnerActor = hActor;

	if (hActor && hActor->IsMonsterActor())
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if (pMonsterActor && pMonsterActor->IsPuppetSummonMonster() && pMonsterActor->GetSummonerPlayerActor())
			hOwnerActor = pMonsterActor->GetSummonerPlayerActor();
	}

	return hOwnerActor;
}
#endif // PRE_FIX_61382

#if defined(PRE_ADD_65808)
void CDnActor::AddSummonMonsterGlyphInfo(int monsterID, int glyphID)
{
	std::map<int, std::list<int>>::iterator findIter = m_SummonMonsterGlyphInfo.find(monsterID);
	if (findIter != m_SummonMonsterGlyphInfo.end())
	{
		findIter->second.push_back(glyphID);
	}
	else
	{
		std::list<int> glyphIDList;
		glyphIDList.push_back(glyphID);

		m_SummonMonsterGlyphInfo.insert(std::make_pair(monsterID, glyphIDList));
	}
}

void CDnActor::AddSummonMonsterGlyphStateEffectID(int monsterID, int glyphID, DWORD dwMonsterUniqueID, int stateEffectID)
{
	std::map<int, std::map<int, std::list<_StateEffectInfo>>>::iterator findIter = m_SummonMonsterGlyphStateEffectIDs.find(monsterID);

	//If there is no monster info
	if (findIter == m_SummonMonsterGlyphStateEffectIDs.end())
	{
		//Create and register a new glyphID and stateEffectIDList.
		std::map<int, std::list<_StateEffectInfo>> newList;
		std::list<_StateEffectInfo> stateEffectList;
		_StateEffectInfo info;
		info.nStateEffectID = stateEffectID;
		info.dwUniqueID = dwMonsterUniqueID;

		stateEffectList.push_back(info);
		newList.insert(std::make_pair(glyphID, stateEffectList));

		m_SummonMonsterGlyphStateEffectIDs.insert(std::make_pair(monsterID, newList));
	}
	else
	{
		std::map<int, std::list<_StateEffectInfo>>& oldList = findIter->second;

		std::map<int, std::list<_StateEffectInfo>>::iterator iter = oldList.find(glyphID);
		if (iter != oldList.end())
		{
			//If glyphID is already registered, add stateEffectID to the existing list.
			_StateEffectInfo info;
			info.nStateEffectID = stateEffectID;
			info.dwUniqueID = dwMonsterUniqueID;

			iter->second.push_back(info);
		}
		else
		{
			//If monster is registered, but glyphID is not registered..
			//Create the state effect list,
			std::list<_StateEffectInfo> stateEffectList;
			_StateEffectInfo info;
			info.nStateEffectID = stateEffectID;
			info.dwUniqueID = dwMonsterUniqueID;

			stateEffectList.push_back(info);

			//Register the state effect list to glyphID.
			oldList.insert(std::make_pair(glyphID, stateEffectList));
		}
	}
}

void CDnActor::RemoveSummonMonsterGlyphInfo(int monsterID, int glyphID)
{
	std::map<int, std::list<int>>::iterator findIter = m_SummonMonsterGlyphInfo.find(monsterID);
	if (findIter != m_SummonMonsterGlyphInfo.end())
	{
		std::list<int>& oldList = findIter->second;
		std::list<int>::iterator iter = oldList.begin();
		std::list<int>::iterator endIter = oldList.end();

		int nGlyphID = 0;
		for (; iter != endIter; ++iter)
		{
			nGlyphID = (*iter);

			if (nGlyphID == glyphID)
			{
				oldList.erase(iter);
				break;
			}
		}
	}

	//Client does not remove state effects (removed upon receiving server packet).
	DnMonsterActorHandle hMonsterActor;
	RemoveSummonMonsterStateEffectByGlyph(hMonsterActor, monsterID, glyphID);

	//Remove actual info.
	RemoveSummonMonsterGlyphStateEffects(monsterID);
}

void CDnActor::RemoveSummonMonsterStateEffectByGlyph(DnMonsterActorHandle hMonsterActor, int monsterID, int glyphID)
{
	std::map<int, std::map<int, std::list<_StateEffectInfo>>>::iterator findIter = m_SummonMonsterGlyphStateEffectIDs.find(monsterID);

	if (findIter != m_SummonMonsterGlyphStateEffectIDs.end())
	{
		std::map<int, std::list<_StateEffectInfo>>& oldList = findIter->second;

		std::map<int, std::list<_StateEffectInfo>>::iterator glyphIter = oldList.find(glyphID);
		if (glyphIter != oldList.end())
		{
			std::list<_StateEffectInfo>& stateEffectIDList = glyphIter->second;

			if (stateEffectIDList.empty() == false)
			{
// 				std::list<_StateEffectInfo>::iterator iter = stateEffectIDList.begin();
// 				std::list<_StateEffectInfo>::iterator endIter = stateEffectIDList.end();
// 
// 				for (; iter != endIter; ++iter)
// 				{
// 					_StateEffectInfo& info = (*iter);
// 
// 					if (hMonsterActor && hMonsterActor->GetUniqueID() == info.dwUniqueID)
// 						hMonsterActor->CmdRemoveStateEffectFromID(info.nStateEffectID);
// 				}
			}
		}
	}
}

void CDnActor::RemoveSummonMonsterGlyphStateEffects(int monsterID)
{
	std::map<int, std::map<int, std::list<_StateEffectInfo>>>::iterator findIter = m_SummonMonsterGlyphStateEffectIDs.find(monsterID);
	if (findIter != m_SummonMonsterGlyphStateEffectIDs.end())
	{
		m_SummonMonsterGlyphStateEffectIDs.erase(findIter);
	}
}

bool CDnActor::GetSummonMonsterGlyphInfo(int monsterID, std::list<int>& glyphIDs)
{
	std::map<int, std::list<int>>::iterator findIter = m_SummonMonsterGlyphInfo.find(monsterID);
	if (findIter != m_SummonMonsterGlyphInfo.end())
	{
		std::list<int>& oldList = findIter->second;
		std::list<int>::iterator iter = oldList.begin();
		std::list<int>::iterator endIter = oldList.end();

		int nGlyphID = 0;
		for (; iter != endIter; ++iter)
		{
			nGlyphID = (*iter);

			glyphIDs.push_back(nGlyphID);
		}

		return true;
	}

	return false;	
}

#endif // PRE_ADD_65808

#if defined(PRE_FIX_68898)
void CDnActor::SetSkipEndAction(bool isSkipEndAction)
{
	if (IsProcessSkill() && m_hProcessSkill)
	{
		CDnPartialPlayProcessor* pPartialPlayProcess =  static_cast<CDnPartialPlayProcessor*>(m_hProcessSkill->GetProcessor( IDnSkillProcessor::PARTIAL_PLAY_ANI ));
		if (pPartialPlayProcess)
			pPartialPlayProcess->SetSkipEndAction(isSkipEndAction);
	}
}
#endif // PRE_FIX_68898




void CDnActor::TransformToActorID(int nActorIndex)
{
	if (m_nSwapActorID == nActorIndex) return;
	m_nSwapActorID = nActorIndex;

	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TACTOR);

#ifdef PRE_FIX_MEMOPT_EXT
	DNTableFileFormat*  pFileNameSox = GetDNTable(CDnTableDB::TFILE);
	std::string szSkinName, szAniName, szActName;
	CommonUtil::GetFileNameFromFileEXT(szSkinName, pSox, nActorIndex, "_SkinName", pFileNameSox);
	CommonUtil::GetFileNameFromFileEXT(szAniName, pSox, nActorIndex, "_AniName", pFileNameSox);
	CommonUtil::GetFileNameFromFileEXT(szActName, pSox, nActorIndex, "_ActName", pFileNameSox);
#else
	std::string szSkinName = pSox->GetFieldFromLablePtr(m_nSwapActorID, "_SkinName")->GetString();
	std::string szAniName = pSox->GetFieldFromLablePtr(m_nSwapActorID, "_AniName")->GetString();
	std::string szActName = pSox->GetFieldFromLablePtr(m_nSwapActorID, "_ActName")->GetString();
#endif

	SAFE_RELEASE_SPTR(GetObjectHandle());


	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(this);
	if (pPartsBody)
		pPartsBody->LoadSkin(CEtResourceMng::GetInstance().GetFullName(szSkinName).c_str(), CEtResourceMng::GetInstance().GetFullName(szAniName).c_str());
	
	LoadAction(CEtResourceMng::GetInstance().GetFullName(szActName).c_str());

	if (m_hObject)
	{
		m_hObject->SetCollisionGroup(COLLISION_GROUP_DYNAMIC(1));
		m_hObject->SetTargetCollisionGroup(COLLISION_GROUP_STATIC(1) | COLLISION_GROUP_DYNAMIC(2) | COLLISION_GROUP_DYNAMIC(3));
	}

	//if (IsExistAction("Summon_On") == true)
	//	SetAction("Summon_On", 0.f, 0.f);
	//else
	//	SetAction("Stand", 0.f, 0.f);
		

	if (pPartsBody)
		pPartsBody->ShowParts(false);

	for (int i = 0; i<CDnWeapon::EquipSlot_Amount; i++)
	{
		DnWeaponHandle hWeapon = GetWeapon(i);
		if (hWeapon)
			LinkWeapon(i);

		ShowWeapon(i, false);
	}
}


void CDnActor::TransformToNormal()
{
	DNTableFileFormat* pSox = GetDNTable(CDnTableDB::TACTOR);
	DNTableFileFormat*  pFileNameSox = GetDNTable(CDnTableDB::TFILE);

	std::string szSkinName, szAniName, szActName;
	CommonUtil::GetFileNameFromFileEXT(szSkinName, pSox, m_nClassID, "_SkinName", pFileNameSox);
	CommonUtil::GetFileNameFromFileEXT(szAniName, pSox, m_nClassID, "_AniName", pFileNameSox);
	CommonUtil::GetFileNameFromFileEXT(szActName, pSox, m_nClassID, "_ActName", pFileNameSox);

	SAFE_RELEASE_SPTR(GetObjectHandle());

	MAPartsBody *pPartsBody = dynamic_cast<MAPartsBody *>(this);
	if (pPartsBody)
	{
		pPartsBody->LoadSkin(CEtResourceMng::GetInstance().GetFullName(szSkinName).c_str(), CEtResourceMng::GetInstance().GetFullName(szAniName).c_str());
		pPartsBody->CombineParts();
		pPartsBody->ShowParts(true);
	}

	//SetAction("Stand", 0.f, 0.f);

	m_nSwapActorID = -1;
}