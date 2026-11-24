#include "stdafx.h"
#include "DnActor.h"
#include "DnWorld.h"
#include "DnWorldGrid.h"
#include "MAActorRenderBase.h"
#include "MAMovementBase.h"
#include "DnWeapon.h"
#include "EtActionSignal.h"
#include "VelocityFunc.h"
#include "DnWorldBrokenProp.h"
#include "DnWorldSector.h"
#include "DnProjectile.h"
#include "DnMonsterActor.h"
#include "TaskManager.h"
#include "DnPlayerActor.h"
#include "PerfCheck.h"
#include "DnStateBlow.h"
#include "DnBlow.h"
#include "DnGameTask.h"
#include "DnItemTask.h"
#include "DNUserData.h"
#include "DNUserSession.h"
#include "DnWorldTrapProp.h"
#include "DnChangeActionStrProcessor.h"
//#include "DNUserConnection.h"
// Siva Shaodw Test temporary
#ifdef _SHADOW_TEST
#include "DNProtocol.h"
#include "GameSendPacket.h"
bool g_bEnableShadow[MAX_SESSION_COUNT] = { 0, };
#endif
#include "EtComputeDist.h"
#include "DNAggroSystem.h"
#include "DnParryBlow.h"
#include "DnCooltimeParryBlow.h"
#include "DnChargerBlow.h"
#include "DnBuffProp.h"
#include "DnWorldBrokenBuffProp.h"
#include "DnReverseTeamBlow.h"
#include "DnHighLanderBlow.h"
#include "DnCannonMonsterActor.h"
#include "DnPlayerSpeedHackChecker.h"
#include "DnFrostbiteBlow.h"

#include "DnChangeActionSetBlow.h"
#ifdef IRINE_MOD_PLAYER_BOTS
#include "DnAIBotController.h"
#endif
#include "DnInvincibleAtBlow.h"
#include "DnBreakSuperArmorBlow.h"
#include "DnBurnCriticalBlow.h"
#include "DnDisableActionBlow.h"

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
#include "DnDPSReporter.h"
#endif

#include "DnManaShieldBlow.h"
#include "DnCurseBlow.h"
#include "DNGameDataManager.h"
#include "DnStateBlowSignalProcessor.h"
#include "DnInvincibleTypeBlow.h"
#include "DnImmuneByTypeBlow.h"
#include "DnComboDamageLimitBlow.h"
#include "DnTransmitDamageBlow.h"
#include "DnExtendedCombatBlows.h"
#include "DnChangeStandActionBlow.h"

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
#include "DnTotalLevelSkillBlows.h"
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

#if defined(PRE_FIX_68898)
#include "DnPartialPlayProcessor.h"
#endif // PRE_FIX_68898

#include "SmartPtrDef.h"

#include "DnBubbleSystem.h"
#include "DnObserverEventMessage.h"


DECL_DN_SMART_PTR_STATIC( CDnActor, MAX_SESSION_COUNT, 100 )

STATIC_DECL_INIT( CDnActor, CEtOctree<DnActorHandle> *, s_pOctree ) = { NULL, };
STATIC_DECL_INIT( CDnActor, DWORD, s_dwUniqueCount ) = { 0, };
STATIC_DECL_INIT( CDnActor, CDnActor::mapActorSearch, s_dwMapActorSearch );

float CDnActor::s_fDieDelta = 20.f;

template<> float CEtOctreeNode<DnActorHandle>::s_fMinRadius = 1000.0f;

#define DOWN_DELAY_RANDOM_RANGE 0.2f // 20 percent longer or shorter~

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
STATIC_DECL_INIT( CDnActor, CFrameSkip, s_MonsterProcess );
STATIC_DECL_INIT( CDnActor, float, s_fMonsterProcessDelta );
STATIC_DECL_INIT( CDnActor, CFrameSkip, s_NpcProcess );
STATIC_DECL_INIT( CDnActor, float, s_fNpcProcessDelta );
#endif

CDnActor::CDnActor( CMultiRoom *pRoom, int nClassID )
	: CDnUnknownRenderObject( pRoom, true )
	, m_vStaticPosition( 0.f, 0.f, 0.f )
	, m_vPrevPosition( 0.f, 0.f, 0.f )
{
	CDnActorState::Initialize( this );
	m_nClassID = nClassID;
	m_fAddHeight = 0.f;

	m_bShow = true;
	m_fDownDelta = 0.f;
	m_fDieDelta = 0.f;
	m_fMaxDieDelta = CDnActor::s_fDieDelta;
	m_fStiffDelta = 0.f;
	m_fLastDownRatio = 1.f;
	m_LastHitSignalTime = 0;
	m_nLastHitSignalIndex = -1;
	m_nLastDamageHitterActionIndex = 0;
	m_nTeam = 0;
	m_dwUniqueID = -1;

	m_pAggroSystem = NULL;

	m_bModifyPlaySpeed = false;
	m_PlaySpeedStartTime = 0;
	m_dwPlaySpeedTime = 0;

	m_bEnableNormalSuperAmmor = false;
	m_nNormalSuperAmmorTime = 0;
	m_fBreakNormalSuperAmmorDurability = 0.f;

	m_HitCheckType = HitCheckTypeEnum::BoundingBox;
	memset( m_bSelfDeleteWeapon, 0, sizeof( m_bSelfDeleteWeapon ) );

	m_pStateBlow = new CDnStateBlow( GetMySmartPtr() );

	memset( m_nSkillSuperAmmorValue, 0, sizeof(m_nSkillSuperAmmorValue) );
	memset( m_nLastUpdateSkillSuperAmmorValue, 0, sizeof(m_nLastUpdateSkillSuperAmmorValue) );
	m_nSkillSuperAmmorTime = 0;
	m_fSkillSuperAmmorDamageDecreaseProb = 0.f;

	m_bIngnoreNormalSuperArmor = false;
	InsertOctreeNode();

	m_bUseSignalSkillCheck = false;
	ZeroMemory( m_abSignalSkillCheck, sizeof(m_abSignalSkillCheck) );

	m_iCantMoveReferenceCount = 0;
	m_iCantActionReferenceCount = 0;
	m_iCantUseSkillReferenceCount = 0;
	m_iCantAttackReferenceCount = 0;

	m_iCantXZMoveSE = 0;

#ifdef _SHADOW_TEST
	m_bIsShadowActor = false;
	m_bEnableShadowActor = true;
#endif

	m_iLastDamage = 0;

	m_bOnSignalFromChargerSE = false;
	
	m_bAllowCalcCombo = true;
	m_bOctreeUpdate = true;

	m_bToggleIngnoreHit = false;

	m_dwGenTick = timeGetTime();

	m_iFrameStopRefCount = 0;
	m_fPlaySpeed = 0.0f;
	m_iLastProcessPressCount = 0;

	m_bCompleteKill_AfterProcessStateBlow = false;

	m_pStateBlowSignalProcessor = new CDnStateBlowSignalProcessor();

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

#if defined(PRE_FIX_59347)
	m_bApplyPartsDamage = false;
#endif // PRE_FIX_59347

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
}

CDnActor::~CDnActor()
{
	RemoveUniqueSearchMap( GetRoom(), this );
	FreeAction();
	for( int i=0; i<2; i++ ) SAFE_RELEASE_SPTR( m_hWeapon[i] );
	RemoveOctreeNode();
	SAFE_DELETE_VEC( m_VecPreActionState );
	SAFE_DELETE_VEC( m_VecPreActionCustomState );
	SAFE_DELETE(m_pStateBlow);
	SAFE_DELETE( m_pAggroSystem );

	SAFE_DELETE(m_pStateBlowSignalProcessor);
}

void CDnActor::InsertOctreeNode()
{
	if( STATIC_INSTANCE(s_pOctree) ) {
		SSphere Sphere;
		GetBoundingSphere( Sphere );
		m_pCurrentNode = STATIC_INSTANCE(s_pOctree)->Insert( GetMySmartPtr(), Sphere );
	}
}

void CDnActor::RemoveOctreeNode()
{
	if( STATIC_INSTANCE(s_pOctree) ) {
		if( !STATIC_INSTANCE(s_pOctree)->Remove( GetMySmartPtr(), m_pCurrentNode ) ) {
			STATIC_INSTANCE(s_pOctree)->Remove( GetMySmartPtr(), NULL );
		}
		m_pCurrentNode = NULL;
	}
}

void CDnActor::SetUniqueID( DWORD dwUniqueID )
{
	RemoveUniqueSearchMap( GetRoom(), this );
	m_dwUniqueID = dwUniqueID;
	InsertUniqueSearchMap( GetRoom(), this );
}

void CDnActor::InsertUniqueSearchMap( CMultiRoom *pRoom, CDnActor *pActor )
{
	if( pActor->GetUniqueID() == -1 ) return;
	STATIC_INSTANCE_(s_dwMapActorSearch).insert( make_pair( pActor->GetUniqueID(), pActor->GetMySmartPtr() ) );
}

void CDnActor::RemoveUniqueSearchMap( CMultiRoom *pRoom, CDnActor *pActor )
{
	if( pActor->GetUniqueID() == -1 ) return;
	std::map<DWORD, DnActorHandle>::iterator it;
	it = STATIC_INSTANCE_(s_dwMapActorSearch).find( pActor->GetUniqueID() );
	if( it != STATIC_INSTANCE_(s_dwMapActorSearch).end() ) {
		STATIC_INSTANCE_(s_dwMapActorSearch).erase( it );
	}
}

// Static Func

void CDnActor::InitializeNextStage( CMultiRoom* pRoom  )
{
	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(pRoom);

	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) 
	{
		if( !STATIC_INSTANCE_(s_pVecProcessList)[i]->IsPlayerActor() ) 
		{
			//g_Log.Log( LogType::_INVALIDACTOR, "[%d] MapIdx:%d ClassID:%d LocalTime=%I64d\r\n", g_Config.nManagedID, pGameRoom->m_iMapIdx, STATIC_INSTANCE_(s_pVecProcessList)[i]->GetClassID(), pGameTask ? pGameTask->m_pFrameSync->GetMSTime() : 0 );
			STATIC_INSTANCE_(s_pVecProcessList)[i]->Release();
			i--;
		}
	}
}

bool CDnActor::InitializeClass( CMultiRoom *pRoom )
{
	if( !CDnWorld::IsActive(pRoom) ) return false;

	DNVector(DnActorHandle) VecList;
	GetActorList( pRoom, VecList );
	
	int iNumActor = (int)VecList.size();
	for( int i = 0; i < iNumActor; ++i )
	{
		DnActorHandle hActor = VecList.at( i );

		CDnActor::ActorTypeEnum ActorType = hActor->GetActorType();
		if( ActorType > -1 && ActorType <= ActorTypeEnum::Reserved6 )
			hActor->OnInitializeNextStage();
	}
	float fCenter, fSize;
	CDnWorld::GetInstance(pRoom).CalcWorldSize( fCenter, fSize );

	SAFE_DELETE( STATIC_INSTANCE_(s_pOctree) );
	STATIC_INSTANCE_(s_pOctree) = new CEtOctree<DnActorHandle>( false );
	STATIC_INSTANCE_(s_pOctree)->Initialize( EtVector3( 0.f, fCenter, 0.f ), fSize );
	STATIC_INSTANCE_(CDnActor::s_dwUniqueCount) = 0;

#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	STATIC_INSTANCE_(s_MonsterProcess).SetFramePerSec( 10.f );
	STATIC_INSTANCE_(s_fMonsterProcessDelta) = 0.f;
	STATIC_INSTANCE_(s_NpcProcess).SetFramePerSec( 1.f );
	STATIC_INSTANCE_(s_fNpcProcessDelta) = 0.f;
#endif
	return true;
}

void CDnActor::ProcessClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta )
{
#ifdef PRE_FIX_GAMESERVER_OPTIMIZE
	float fResultDelta[2] = { fDelta, fDelta };
	bool bProcessSkip[2] = { false, false };
	if( STATIC_INSTANCE_(s_MonsterProcess).Update( fDelta ) ) {
		STATIC_INSTANCE_(s_fMonsterProcessDelta) += fDelta;
		fResultDelta[0] = STATIC_INSTANCE_(s_fMonsterProcessDelta);
		bProcessSkip[0] = false;
		STATIC_INSTANCE_(s_fMonsterProcessDelta) = 0.f;
	}
	else {
		bProcessSkip[0] = true;
		STATIC_INSTANCE_(s_fMonsterProcessDelta) += fDelta;
	}

	if( STATIC_INSTANCE_(s_NpcProcess).Update( fDelta ) ) {
		STATIC_INSTANCE_(s_fNpcProcessDelta) += fDelta;
		fResultDelta[1] = STATIC_INSTANCE_(s_fNpcProcessDelta);
		bProcessSkip[1] = false;
		STATIC_INSTANCE_(s_fNpcProcessDelta) = 0.f;
	}
	else {
		bProcessSkip[1] = true;
		STATIC_INSTANCE_(s_fNpcProcessDelta) += fDelta;
	}


	float fTempDelta;
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) {
		CDnActor *pActor = STATIC_INSTANCE_(s_pVecProcessList)[i];
		pActor->SyncClassTime( LocalTime );

		fTempDelta = fDelta;

		if( !pActor->IsPlayerActor() ) {
			if( pActor->IsMonsterActor() ) {

#ifndef PRE_FIX_MOMSTER_PROCESS_DELTA
				// Because monster and player frame rates differ, client and server sync diverges in sensitive parts
				// Subtle desync occurs in interpolation or code using specific Delta values, so configure monsters not to apply frame sync.
				fTempDelta = fResultDelta[0];
				if( bProcessSkip[0] ) continue;
#endif
			}
			if( pActor->IsNpcActor() ) {
				fTempDelta = fResultDelta[1];
				if( bProcessSkip[1] ) continue;
			}
		}
		/*
		if( bProcessSkip == true && !STATIC_INSTANCE_(s_pVecProcessList)[i]->IsPlayerActor() ) {
			continue;
		}
		if( !STATIC_INSTANCE_(s_pVecProcessList)[i]->IsPlayerActor() ) {
			fTempDelta = fResultDelta;
		}
		else fTempDelta = fDelta;
		*/

		STATIC_INSTANCE_(s_pVecProcessList)[i]->Process( LocalTime, fTempDelta );

		SSphere Sphere;
		STATIC_INSTANCE_(s_pVecProcessList)[i]->GetBoundingSphere( Sphere );
		if( STATIC_INSTANCE_(s_pVecProcessList)[i]->m_bOctreeUpdate )
			STATIC_INSTANCE_(s_pVecProcessList)[i]->m_pCurrentNode = STATIC_INSTANCE_(s_pOctree)->Update( STATIC_INSTANCE_(s_pVecProcessList)[i]->GetMySmartPtr(), Sphere, STATIC_INSTANCE_(s_pVecProcessList)[i]->m_pCurrentNode );

		if( STATIC_INSTANCE_(s_pVecProcessList)[i]->IsDestroy() ) {
			STATIC_INSTANCE_(s_pVecProcessList)[i]->Release();
			i--;
		}
	}
#else
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) {
		STATIC_INSTANCE_(s_pVecProcessList)[i]->SyncClassTime( LocalTime );
		STATIC_INSTANCE_(s_pVecProcessList)[i]->Process( LocalTime, fDelta );

		SSphere Sphere;
		STATIC_INSTANCE_(s_pVecProcessList)[i]->GetBoundingSphere( Sphere );
		if( STATIC_INSTANCE_(s_pVecProcessList)[i]->m_bOctreeUpdate )
			STATIC_INSTANCE_(s_pVecProcessList)[i]->m_pCurrentNode = STATIC_INSTANCE_(s_pOctree)->Update( STATIC_INSTANCE_(s_pVecProcessList)[i]->GetMySmartPtr(), Sphere, STATIC_INSTANCE_(s_pVecProcessList)[i]->m_pCurrentNode );

		if( STATIC_INSTANCE_(s_pVecProcessList)[i]->IsDestroy() ) {
			STATIC_INSTANCE_(s_pVecProcessList)[i]->Release();
			i--;
		}
	}
#endif
}

void CDnActor::ProcessAIClass( CMultiRoom *pRoom, LOCAL_TIME LocalTime, float fDelta )
{
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) 
	{
		STATIC_INSTANCE_(s_pVecProcessList)[i]->ProcessAI( LocalTime, fDelta );

	}
}

void CDnActor::ReleaseClass( CMultiRoom *pRoom )
{
	SAFE_DELETE( STATIC_INSTANCE_(s_pOctree) );
	SAFE_DELETE_MAP( STATIC_INSTANCE_(s_dwMapActorSearch) );
	DeleteAllObject( pRoom );

	STATIC_INSTANCE_(s_pVecProcessList).clear();
}

int CDnActor::ScanActor( CMultiRoom *pRoom, EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside )
{
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;
	STATIC_INSTANCE_(s_pOctree)->Pick( Sphere, VecList, bInside );
	return (int)VecList.size();
}

int CDnActor::ScanActorByActorSize( CMultiRoom *pRoom, EtVector3 &vPos, float fRadius, DNVector(DnActorHandle) &VecList, bool bInside, bool bActorSize )
{
	SSphere Sphere;
	Sphere.Center = vPos;
	Sphere.fRadius = fRadius;
	STATIC_INSTANCE_(s_pOctree)->Pick( Sphere, VecList, bInside, bActorSize );
	return (int)VecList.size();
}

void CDnActor::GetActorList(CMultiRoom *pRoom, DNVector(DnActorHandle) &VecList)
{
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) 
	{
		DnActorHandle hActor = STATIC_INSTANCE_(s_pVecProcessList)[i]->GetMySmartPtr();
		VecList.push_back(hActor);
	}
}

void CDnActor::GetOpponentActorList(CMultiRoom *pRoom, int nTeam, DNVector(DnActorHandle) &VecList, EtVector3* pPos/*=NULL*/, float fMaxRange/*=0.f*/ )
{
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) 
	{
		DnActorHandle hActor = STATIC_INSTANCE_(s_pVecProcessList)[i]->GetMySmartPtr();
		if( hActor->GetTeam() != nTeam )
		{
			bool bPush = true;
			if( pPos && fMaxRange > 0.f )
			{
				if( EtVec3LengthSq( &(*pPos-*hActor->GetPosition()) ) > fMaxRange*fMaxRange )
					bPush = false;
			}

			if( bPush )
				VecList.push_back(hActor);
		}
	}
}

DnActorHandle CDnActor::FindActorFromUniqueID( CMultiRoom *pRoom, DWORD dwUniqueID )
{
	std::map<DWORD, DnActorHandle>::iterator it = STATIC_INSTANCE_(s_dwMapActorSearch).find( dwUniqueID );
	if( it != STATIC_INSTANCE_(s_dwMapActorSearch).end() ) return it->second;
	return CDnActor::Identity();
}

DnActorHandle CDnActor::FindActorFromName( CMultiRoom *pRoom, TCHAR *szName )
{
	for( DWORD i=0; i<STATIC_INSTANCE_(s_pVecProcessList).size(); i++ ) {
		if( !wcscmp( STATIC_INSTANCE_(s_pVecProcessList)[i]->GetName(), szName ) ) return STATIC_INSTANCE_(s_pVecProcessList)[i]->GetMySmartPtr();
	}
	return CDnActor::Identity();
}

void CDnActor::ProcessReservedRemoveBlows( void )
{
	if( false == m_vlReservedRemoveBlows.empty() )
	{
		for( int i = 0; i < (int)m_vlReservedRemoveBlows.size(); ++i )
		{
			DnBlowHandle hBlow = m_vlReservedRemoveBlows.at( i );
			if( hBlow )
				this->CmdRemoveStateEffectFromID( hBlow->GetBlowID() );
		}

		m_vlReservedRemoveBlows.clear();
	}
}

void CDnActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	UpdateStealMagicBuff();

	int nState = GetState();
	m_bEnableNormalSuperAmmor = false;
	CDnActionBase::ProcessAction( LocalTime, fDelta );

	if( 0 < GetCantMoveSEReferenceCount() )
		m_vAniDistance.x = m_vAniDistance.z = 0.0f;

	if( !IsNpcActor() )
	{
		PROFILE_TIME_TEST( OnSkillProcess( LocalTime, fDelta ) );
	}

	ProcessDown( LocalTime, fDelta );
	ProcessDie( LocalTime, fDelta );
	ProcessStiff( LocalTime, fDelta );
	ProcessPlaySpeed( LocalTime, fDelta );

	ProcessRotate( LocalTime, fDelta ); // Rotate;

	PROFILE_TIME_TEST( ProcessState( LocalTime, fDelta ) );
	PROFILE_TIME_TEST( ProcessPress() );

	if (m_pStateBlowSignalProcessor && !IsDie() )
		m_pStateBlowSignalProcessor->Process(LocalTime, fDelta);

	//Function to apply the state effect registered in state blow #229.
	ApplySkillStateEffect();

	for( int i=0; i<2; i++ )
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

#ifdef _SHADOW_TEST
	if( g_bEnableShadow[GetRoom()->GetRoomID()] && m_bEnableShadowActor && GetGameRoom() ) {
		char pBuf[256];
		CPacketCompressStream Stream( pBuf, 256 );
		int nIndex = GetElementIndex( m_szAction.c_str() );
		float fFrame = GetCurFrame();
		EtVector3 vMoveX = *GetMoveVectorX();
		EtVector3 vMoveZ = *GetMoveVectorZ();
		Stream.Write( &nIndex, sizeof(int) );
		Stream.Write( &fFrame, sizeof(float) );
		Stream.Write( &m_Cross.m_vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
		Stream.Write( &m_Cross.m_vXAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
		Stream.Write( &m_Cross.m_vZAxis, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
		Stream.Write( &vMoveX, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );
		Stream.Write( &vMoveZ, sizeof(EtVector3), CPacketCompressStream::VECTOR3_SHORT );

		bool bMoveTarget = ( GetMoveTarget() ) ? true : false;
		bool bLookTarget = ( GetLookTarget() ) ? true : false;
		Stream.Write( &bMoveTarget, sizeof(bool) );
		if( bMoveTarget ) {
			DnActorHandle hMoveTarget = GetMoveTarget();
			Stream.Write( hMoveTarget->GetPosition(), sizeof(EtVector3) );
		}

		Stream.Write( &bLookTarget, sizeof(bool) );
		if( bLookTarget ) {
			DnActorHandle hLookTarget = GetLookTarget();
			Stream.Write( hLookTarget->GetPosition(), sizeof(EtVector3) );
		}

		for( DWORD i=0; i<GetGameRoom()->GetUserCount(); i++ ) {
			SendActorShadowMsg( GetGameRoom()->GetUserData(i), GetUniqueID(), eActor::SC_SHADOW, (BYTE*)Stream.GetBuffer(), Stream.Tell() );
		}
	}
#endif
	// Initialize last received damage. (Used in fire state effect)
	m_iLastDamage = 0;
}

void CDnActor::GetBoundingSphere( SSphere &Sphere, bool bActorSize/* = false*/ )
{
	MAActorRenderBase *pRenderBase = static_cast<MAActorRenderBase *>(this);
	if( pRenderBase ) {
		if( pRenderBase->GetBoundingSphere( Sphere ) == false ) {
			Sphere.Center = m_Cross.m_vPosition;
			Sphere.fRadius = (float)GetUnitSize();
		}
	}
	else {
		Sphere.Center = m_Cross.m_vPosition;
		Sphere.fRadius = (float)GetUnitSize();
	}

	if (bActorSize == true)
	{
		Sphere.Center = FindAutoTargetPos();
		Sphere.fRadius = (float)GetUnitSize();
	}
}

void CDnActor::GetBoundingBox( SAABox &Box )
{
	MAActorRenderBase *pRenderBase = static_cast<MAActorRenderBase *>(this);
	if( pRenderBase ) {
		pRenderBase->GetBoundingBox( Box );
	}
	else {
		Box.Min = m_Cross.m_vPosition + EtVector3( -50.f, -50.f, -50.f );
		Box.Max = m_Cross.m_vPosition + EtVector3( 50.f, 50.f, 50.f );
	}
}


void CDnActor::AddStateEffectQueue( const CDnSkill::SkillInfo& ParentSkillInfo, const CDnSkill::StateEffectStruct& StateEffectInfo )
{
	m_dqApplySelfStateBlowQ.push_back( S_NO_PACKET_SELF_STATEBLOW(ParentSkillInfo, StateEffectInfo) );
}

void CDnActor::ClearSelfStateSignalBlowQueue( bool bItemSkill/* = false*/ )
{
	deque<S_NO_PACKET_SELF_STATEBLOW>::iterator iter = m_dqApplySelfStateBlowQ.begin();
	for( iter; iter != m_dqApplySelfStateBlowQ.end(); )
	{
		if( bItemSkill == iter->ParentSkillInfo.bIsItemSkill )
		{
			iter = m_dqApplySelfStateBlowQ.erase( iter );
		}
		else
		{
			iter++;
		}
	}
}


void CDnActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type ) {
		case STE_Hit:
			{
				if( IsDie() ) break;

				if (ProcessIgnoreHitSignal() == true)
					break;

				HitStruct *pStruct = (HitStruct *)pPtr;
				
				DNVector(DnActorHandle) hVecList;
				DNVector(DnActorHandle) hVecActorToApplyStateEffect;

				int nWeaponLength = 0;
				bool bOnDamageCalled = false;
				if( pStruct->bIncludeWeaponLength && GetWeapon() ) nWeaponLength = GetWeapon()->GetWeaponLength();

				MatrixEx CrossTemp = m_Cross;

				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;
				// Because for scaled monsters the hit signal's center axis does not always match the actor's origin position 
				// Scale value must also be applied accurately to offset (#18971)
				EtVector3 vOffset = *pStruct->vOffset * GetScale()->y;
				CrossTemp.MoveLocalZAxis( vOffset.z );
				CrossTemp.MoveLocalXAxis( vOffset.x );
				CrossTemp.MoveLocalYAxis( vOffset.y );
				
				// If linked to a specific bone..
				if( pStruct->szLinkBoneName && strlen(pStruct->szLinkBoneName) > 0 )
				{
					int nBoneIndex = GetBoneIndex( pStruct->szLinkBoneName );
					if( -1 != nBoneIndex )
					{
						EtMatrix matBoneWorld = GetBoneMatrix( pStruct->szLinkBoneName );
						EtMatrixMultiply( (EtMatrix*)&CrossTemp, (EtMatrix*)&CrossTemp, &matBoneWorld );
					}
				}
				// Monsters need to be adjusted according to scale.
				float fDistanceMax = pStruct->fDistanceMax * GetScale()->y;
				float fDistanceMin = pStruct->fDistanceMin * GetScale()->y;
				float fHeightMax = pStruct->fHeightMax * GetScale()->y;

				EtVector3 vPos = CrossTemp.m_vPosition;

				float fDistance = max( fDistanceMax, fHeightMax - pStruct->fHeightMin ) + nWeaponLength;
				float fXZDistanceSQ = fDistanceMax + nWeaponLength;
				float fXZDistanceMinSQ = fDistanceMin; // Min is not affected by Weapon length.

				fXZDistanceSQ *= fXZDistanceSQ;
				fXZDistanceMinSQ *= fXZDistanceMinSQ;

				ScanActor( GetRoom(), vPos, fDistance, hVecList );

				//#53454 If it is a Puppet summon actor, exclude the Puppet owner from HitList.
#if defined(PRE_FIX_61382)
				ExceptionHitList2(hVecList, m_Cross, GetMySmartPtr(), pStruct, hVecActorToApplyStateEffect, 0, fDistance, m_vPrevPosition);
#else
				ExceptionHitList(hVecList, m_Cross, GetMySmartPtr(), pStruct);
#endif // PRE_FIX_61382

				EtVector3 vDir;
				EtVector3 vZVec = m_Cross.m_vZAxis;

				if( pStruct->fCenterAngle != 0.f ) {
					EtMatrix matRotate;
					EtMatrixRotationY( &matRotate, EtToRadian( pStruct->fCenterAngle ) );
					EtVec3TransformNormal( &vZVec, &vZVec, &matRotate );

#if defined(PRE_FIX_63356)
					//#63356 Case for Circle Bomber when hit area is rotated 180 degrees.
					//Since vPos is offset from the character's current direction and the hit area is rotated 180 degrees
					//Apply this rotation value in the allowed angle calculation.
					CrossTemp.m_vZAxis = vZVec;
#endif // PRE_FIX_63356
				}
				SAABox Box;
				float fDot = 0.0f;
				SHitParam HitParam;

				bool bHit = false;
				bool bFirstHit = true;
				if( m_LastHitSignalTime > LocalTime ) bFirstHit = false;
				if( m_nLastHitSignalIndex != nSignalIndex ) {
					bFirstHit = true;
					m_nLastHitSignalIndex = nSignalIndex;
				}

				HitParam.szActionName = pStruct->szTargetHitAction;
				HitParam.fDamage = pStruct->fDamageProb * 0.01f;
				HitParam.fDurability = pStruct->fDurabilityDamageProb * 0.01f;
				HitParam.vVelocity = *pStruct->vVelocity;
				HitParam.vResistance = *pStruct->vResistance;
				HitParam.hHitter = GetMySmartPtr();
				HitParam.hWeapon = GetActiveWeapon(0);
				HitParam.vPosition = vPos;
				HitParam.fStiffProb = pStruct->fStiffProb * 0.01f;
				HitParam.RemainTime = SignalEndTime;
				HitParam.nDamageType = pStruct->nDamageType;
				HitParam.nSkillSuperAmmorIndex = pStruct->nApplySuperAmmorIndex - 1;
				HitParam.nSkillSuperAmmorDamage = pStruct->nApplySuperAmmorDamage;
				HitParam.cAttackType	= (char)pStruct->nAttackType;
				HitParam.DistanceType	= (pStruct->nDistanceType == 0) ? CDnDamageBase::DistanceTypeEnum::Melee : CDnDamageBase::DistanceTypeEnum::Range;
				HitParam.HasElement = CalcHitElementType( ( pStruct->bUseSkillApplyWeaponElement == TRUE ) ? true : false );
				HitParam.bIgnoreCanHit = ( pStruct->bIgnoreCanHit == TRUE );
				HitParam.bIgnoreParring = ( pStruct->bIgnoreParring == TRUE );
				HitParam.bReleaseCatchActor = ( pStruct->bReleaseCatchedActor == TRUE );
				HitParam.nWeightValueLimit = pStruct->nWeightValueLimit;

			
				bool isFirstHitter = true;

 				HitParam.nHitLimitCount = pStruct->nHitLimitCount;
				
				bool isHitLimited = false;
				HitLimitCountInfo* pHitLimitCountInfo = NULL;

				if (HitParam.nHitLimitCount != 0)
				{
					//Check if HitLimitCount info is registered..
					HIT_LIMIT_COUNT_INFO_LIST::iterator findIter = m_HitLimitCountInfoList.find(nSignalIndex);
					if (findIter != m_HitLimitCountInfoList.end())
						pHitLimitCountInfo = &findIter->second;
					else
					{
						//Add new info and get the pointer to that info in the list.
						HitLimitCountInfo newHitLimitCountInfo(HitParam.nHitLimitCount, HitParam.nHitLimitCount);
						m_HitLimitCountInfoList.insert(HIT_LIMIT_COUNT_INFO_LIST::value_type(nSignalIndex, newHitLimitCountInfo));
						
						HIT_LIMIT_COUNT_INFO_LIST::iterator findIter = m_HitLimitCountInfoList.find(nSignalIndex);
						if (findIter != m_HitLimitCountInfoList.end())
							pHitLimitCountInfo = &findIter->second;
					}
				}

				// Note: Clearing here may carry over previously hit targets without restoring their hit signal end time. 
				// When an entity with super armor is hit, FPS becomes 1.8 and EndTime shifts back; to restore it properly in ProcessPlaySpeed 
				// Must not clear the last-hit targets before restoring them to the list.
				m_hVecLastHitList.clear();
				// Check Actor
				for( DWORD i=0; i<hVecList.size(); i++ ) 
				{
					if( !hVecList[i] )
						continue;

					// Limit hit count
					//If max Hit count is set, stop when Hit count exceeds max Hit count.
					isHitLimited = (pHitLimitCountInfo && (pHitLimitCountInfo->nHitLimitCount != 0 && pHitLimitCountInfo->nHitCount <= 0));

					if (isHitLimited)
						break;

					// If it is a Charger state effect, use that state effect's ID as the unique ID.
					// To trigger multiple hit signals simultaneously per Charger state effect instance count in the same action, like Shock of Relic.
					if( m_hChargerBlowCalledOnSignal )
					{
						if( !hVecList[i]->IsHittable( GetMySmartPtr(), LocalTime, pStruct, m_hChargerBlowCalledOnSignal->GetBlowID() ) ) 
							continue;

						// Set hitparam's unique ID here so ondamage can organize by this ID and use it for later ishittable calls.
						HitParam.iUniqueID = m_hChargerBlowCalledOnSignal->GetBlowID();
					}
					else
					{
						if( !hVecList[i]->IsHittable( GetMySmartPtr(), LocalTime, pStruct ) ) 
							continue;
					}

					switch( hVecList[i]->GetHitCheckType() ) {
						case HitCheckTypeEnum::BoundingBox: 
							{
								HitParam.vPosition = vPos;
								vDir = *hVecList[i]->GetPosition() - vPos;
								vDir.y = 0.f;
								
								hVecList[i]->GetBoundingBox( Box );

								if( SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
								if( SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

								EtVec3Normalize( &vDir, &vDir );
								fDot = EtVec3Dot( &vZVec, &vDir );
								if( EtToDegree( acos( fDot ) ) > pStruct->fAngle ) continue;

								if( Box.Min.y < vPos.y + pStruct->fHeightMin && 
									Box.Max.y < vPos.y + pStruct->fHeightMin ) continue;
								if( Box.Min.y > vPos.y + fHeightMax && 
									Box.Max.y > vPos.y + fHeightMax ) continue;

								HitParam.vViewVec = -vDir;
							}
							break;
						case HitCheckTypeEnum::Collision:
							{
								SCollisionCapsule				Capsule;
								SCollisionResponse				CollisionResult;
								DNVector(SCollisionResponse)	vCollisionResult;

								Capsule.Segment.vOrigin = vPos;
								float fHeight = fHeightMax - pStruct->fHeightMin;
								Capsule.Segment.vOrigin.y = Capsule.Segment.vOrigin.y - ( pStruct->fHeightMin + ( fHeight / 2.f ) );
								Capsule.Segment.vDirection = EtVector3( 0.f, fHeight / 2.f, 0.f );
								Capsule.fRadius = ( fDistanceMax + nWeaponLength );

								EtVector3 vDestPos;
								if( hVecList[i]->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == false ) continue;
								if( CollisionResult.pCollisionPrimitive )
								{
									for( UINT k=0 ; k<vCollisionResult.size() ; ++k )
									{
										if( vCollisionResult[k].pCollisionPrimitive ) {
											vCollisionResult[k].pCollisionPrimitive->GetBoundingBox( Box );
											if( Box.Min.y < vPos.y + pStruct->fHeightMin && Box.Max.y < vPos.y + pStruct->fHeightMin ) {
												vCollisionResult.erase( vCollisionResult.begin() + k );
												k--;
												continue;
											}
											if( Box.Min.y > vPos.y + fHeightMax && Box.Max.y > vPos.y + fHeightMax ) {
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
												//#63356 Case for Circle Bomber when hit area is rotated 180 degrees.
												//Since vPos is offset from the character's current direction and the hit area is rotated 180 degrees
												//Apply this rotation value in the allowed angle calculation.
												if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], pStruct->fAngle) == true ) {
#else
												if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], pStruct->fAngle) == true ) {
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
											HitParam.vBoneIndex.push_back( hVecList[i]->GetObjectHandle()->GetParentBoneIndex( vCollisionResult[k].pCollisionPrimitive ) );
										}
									}
									if( vCollisionResult.empty() ) continue;

									GetCenterPos( *CollisionResult.pCollisionPrimitive, vDestPos );
									HitParam.vPosition = vDestPos;
								}
								else
								{
									ASSERT( 0 );
								}

								if( pStruct->fDistanceMin > 100.f ) 
								{
									vCollisionResult.clear();
									Capsule.fRadius = pStruct->fDistanceMin;
									if( hVecList[i]->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == true ) {
										if( CollisionResult.pCollisionPrimitive )
										{
											for( UINT k=0 ; k<vCollisionResult.size() ; ++k )
											{
												if( vCollisionResult[k].pCollisionPrimitive ) {
													vCollisionResult[k].pCollisionPrimitive->GetBoundingBox( Box );
													if( Box.Min.y < vPos.y + pStruct->fHeightMin && Box.Max.y < vPos.y + pStruct->fHeightMin ) {
														vCollisionResult.erase( vCollisionResult.begin() + k );
														k--;
														continue;
													}
													if( Box.Min.y > vPos.y + fHeightMax && Box.Max.y > vPos.y + fHeightMax ) {
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
														if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], pStruct->fAngle) == false ) {
#else
														if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], pStruct->fAngle) == false ) {
#endif // PRE_FIX_63356
#else
#if defined(PRE_FIX_63356)
														if( hVecList[i]->CheckCollisionHitCondition(vPos, CrossTemp, vPointList[m], pStruct->fAngle) == true ) {
#else
														if( hVecList[i]->CheckCollisionHitCondition(vPos, m_Cross, vPointList[m], pStruct->fAngle) == true ) {
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
													std::vector<int>::iterator it = std::find( HitParam.vBoneIndex.begin(), HitParam.vBoneIndex.end(), nBoneIndex );
													if( it != HitParam.vBoneIndex.end() ) {
														HitParam.vBoneIndex.erase( it );
													}
												}
											}
										}
									}
									if( HitParam.vBoneIndex.empty() ) continue;
								}

								HitParam.vViewVec = vPos - vDestPos;
								EtVec3Normalize( &HitParam.vViewVec, &HitParam.vViewVec );
							}
							break;
					}

					HitParam.bFirstHit = bFirstHit;
					bFirstHit = false;
					bHit = true;

					if( !HitParam.szActionName.empty() ) {
						m_hVecLastHitList.push_back( hVecList[i] );

						// Duplicate actors may be added when frame speed increases.
						if( m_hVecLastHitListByRemainTime.end() == find( m_hVecLastHitListByRemainTime.begin(), m_hVecLastHitListByRemainTime.end(), hVecList[i] ) )
						{
							m_hVecLastHitListByRemainTime.push_back( hVecList[i] );
						}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
						// Trigger prefix skill in HitSignal regardless of whether it's a skill
						// Not applied if a state effect that ignores prefix state effects is active
						// #40186 Changed suffix activation condition (prevent activation when damage ratio is 0).
						if (HitParam.bFirstHit && 
							pStruct->fDamageProb != 0.0f &&
							!IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) && 
							isFirstHitter)
						{
							ProcessPrefixOffenceSkill_New();
							isFirstHitter = false;
						}
#else
						//////////////////////////////////////////////////////////////////////////
						// Prepare prefix attack skill activation
						// Trigger prefix system attack skill only on normal attack
						// When it is a normal attack, first entry into Hit frame, and the first target hit
						
						bool isCanUsePrefixSkill = CanUsePrefixSkill();

						if (isCanUsePrefixSkill && HitParam.bFirstHit && isFirstHitter)
						{
							OutputDebug("CDnActor::OnSignal -> STE_Hit start %d current %d end %d\n", (int)SignalStartTime, (int)LocalTime, (int)SignalEndTime);

							//Here, only apply state effects that apply to self,
							//State effects to apply to the hit target are handled in Target->OnDamage.??
							ProcessPrefixOffenceSkill( 1.0f );

							isFirstHitter = false;
						}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW

#ifdef PRE_ADD_MONSTER_CATCH
						if( HitParam.bReleaseCatchActor )
						{
							if( IsMonsterActor() )
							{
								static_cast<CDnMonsterActor*>(this)->ReleaseThisActor( hVecList[ i ] );
							}
						}
#endif // #ifdef PRE_ADD_MONSTER_CATCH

						hVecList[i]->OnDamage( GetMySmartPtr(), HitParam, pStruct );

						if( 0 != pStruct->StateEffectFilter )
						{
							OnHitSignalStateEffectFilterException( hVecList[ i ]->GetUniqueID(), pStruct->StateEffectFilter );
						}

						bOnDamageCalled = true;

						//Increment hit count
						if (pHitLimitCountInfo)
							pHitLimitCountInfo->nHitCount--;
					}

					hVecActorToApplyStateEffect.push_back( hVecList[i] );

#ifdef PRE_ADD_TRIGGER_BY_HIT_CONDITION
					if( pStruct->szTriggerEventParameter != NULL && IsPlayerActor() == false && hVecList[i]->IsMonsterActor() )
					{
						CDnMonsterActor* pTrigerTargetMonsterActor = static_cast<CDnMonsterActor*>( hVecList[i].GetPointer());
						if( pTrigerTargetMonsterActor )
						{
							std::string strParam = pStruct->szTriggerEventParameter;
							std::vector<std::string> tokens;
							TokenizeA(strParam, tokens, ";");
							if (tokens.size() == 2)
							{
								if( pTrigerTargetMonsterActor->GetMonsterClassID() == atoi(tokens[0].c_str()) )
								{
									CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventArea", pTrigerTargetMonsterActor->GetBirthAreaHandle() );
									CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "ActorHandle", pTrigerTargetMonsterActor->GetUniqueID() );
									CDnWorld::GetInstance(GetRoom()).InsertTriggerEventStore( "EventID", atoi(tokens[1].c_str()) );
									CDnWorld::GetInstance(GetRoom()).OnTriggerEventCallback( "CDnActor::TriggerEvent", CDnActionBase::m_LocalTime, 0.f );
								}
							}
						}
					}
#endif

				}

				bool bIsFarm = false;
				CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
				if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::Farm )
					bIsFarm = true;

				if( false == bIsFarm && IsProcessSkill() ) 
				{
					int iNumActorApplyStateEffect = (int)hVecActorToApplyStateEffect.size();
					m_hProcessSkill->CheckTargetCount( iNumActorApplyStateEffect );
					for( int i = 0; i < iNumActorApplyStateEffect; ++i )
					{
						// Ignore applying state effects when the target is in Ice Prison state
						if (hVecActorToApplyStateEffect[ i ]->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
							continue;

						// Distinguish state effect application based on whether the skill target is set to ally or target.
						switch( m_hProcessSkill->GetTargetType() )
						{
							case CDnSkill::Enemy:
							case CDnSkill::Self:
								if( GetTeam() == hVecActorToApplyStateEffect[ i ]->GetTeam() )
									continue;
								break;

							case CDnSkill::Friend:
							case CDnSkill::Party:
								if( GetTeam() != hVecActorToApplyStateEffect[ i ]->GetTeam() )
									continue;
								break;

							// If the skill target includes both allies and enemies, distinguish whether to apply in the state effect application logic.
							case CDnSkill::All:
								break;
						}

						// Need to handle duration type index when adding status ailment.
						// Identify the Actor's continuous effect index and add the state effect on success.
						map<int, bool> mapDuplicateResult;
						CDnSkill::CanApply eResult = CDnSkill::CanApplySkillStateEffect( hVecActorToApplyStateEffect[i], m_hProcessSkill, mapDuplicateResult, true );
						if( CDnSkill::CanApply::Fail != eResult )
						{
							for( DWORD k = 0; k < m_hProcessSkill->GetStateEffectCount(); k++ ) 
							{
								CDnSkill::StateEffectStruct *pLocalStruct = m_hProcessSkill->GetStateEffectFromIndex(k);

								if (pStruct->szSkipStateBlows && CDnSkill::IsSkipStateBlow(pStruct->szSkipStateBlows, (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID))
									continue;

								// For skills that also heal allies, two state effects are declared: Self heal and Target heal.
								// Self state effects are already applied to self, and since this is where Hit signals are judged, only Target is applied.
								switch( pLocalStruct->ApplyType )
								{
									case CDnSkill::ApplySelf:
										continue;
										break;

									case CDnSkill::ApplyTarget:
										break;

									case CDnSkill::ApplyEnemy:
										if( GetTeam() == hVecActorToApplyStateEffect[ i ]->GetTeam() )
											continue;
										break;

									case CDnSkill::ApplyFriend:
										if( GetTeam() != hVecActorToApplyStateEffect[ i ]->GetTeam() )
											continue;
										break;
								}

								// When stacking the same skill, probability-based skill effects already have their probability checked in CanApplySkillStateEffect
								// Already passed. So here, check if the probability check was already done, and if so, call the function to skip checking probability again.
								bool bAllowAddThisSE = true;
								bool bCheckCanBegin = true;
								if( CDnSkill::ApplyDuplicateSameSkill == eResult )
								{
									map<int, bool>::iterator iter = mapDuplicateResult.find( pLocalStruct->nID );
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

								if( bAllowAddThisSE )
								{
									// #72931 Screamer curse shared cooldown handling -> generalize later if more skills use this pattern
									bool bShareCurseCoolTime = false;
									if( pLocalStruct->nID == STATE_BLOW::BLOW_244 )
									{
										DNVector(DnBlowHandle) vlhBlows;
										GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_244, vlhBlows );
										if( static_cast<int>( vlhBlows.size() ) > 0 )
										{
											CDnCurseBlow* pDnCurseBlow = static_cast<CDnCurseBlow*>( vlhBlows[0].GetPointer() );
											if( pDnCurseBlow )
											{
												pLocalStruct->szValue += ";";
												pLocalStruct->szValue += FormatA( "%f", pDnCurseBlow->GetCoolTime() );
												bShareCurseCoolTime = true;
											}
										}
									}

									// Remove registered state effects marked for removal here. [2010/12/08 semozz]
									hVecActorToApplyStateEffect[i]->RemoveResetStateBlow();

									hVecActorToApplyStateEffect[i]->CmdAddStateEffect( m_hProcessSkill->GetInfo(), (STATE_BLOW::emBLOW_INDEX)pLocalStruct->nID, 
																					   pLocalStruct->nDurationTime, pLocalStruct->szValue.c_str(), false, bCheckCanBegin );

									if( bShareCurseCoolTime )	// #72931 Removed the previously added cooldown
									{
										std::string::size_type delimiterindex = pLocalStruct->szValue.rfind( ";" );
										if( delimiterindex != std::string::npos )
										{
											pLocalStruct->szValue.erase( delimiterindex, pLocalStruct->szValue.length() - delimiterindex );
										}
									}
									
									// Originally, calling OnDamage() sets DamageRemainTime inside. However,
									// Since there are also hit signals like heal that only add state effects without damage, even if OnDamage() was not called
									// Set LastDamageTime here for any that don't have it set.
									// Must do this so only one hit registers per hit signal.
									if( !bOnDamageCalled )
									{
										hVecActorToApplyStateEffect[ i ]->SetDamageRemainTime( GetUniqueID(), HitParam.RemainTime );
										hVecActorToApplyStateEffect[ i ]->SetLastDamageHitUniqueID( GetUniqueID(), HitParam.iUniqueID );
									}
								}

							}
						}
						// Clear reset list when skill usage ends [2010/12/09 semozz]
						hVecActorToApplyStateEffect[i]->InitStateBlowIDToRemove();
					}
				}

				if( bHit ) {
					m_LastHitSignalTime = SignalEndTime;
					OnHitFinish( LocalTime, pStruct );
				}
				else m_LastHitSignalTime = -1;

				// Check prop
				// Note: Prop check should skip state-effect-only skills like Heal. Distinguished by having no HitAction.
				if( false == HitParam.szActionName.empty() 
					//Props can only take damage when no filter is applied.
					&& pStruct->StateEffectFilter == 0 
					)
				{
					DNVector(CEtWorldProp*) pVecProp;
					CDnWorld* pWorld = CDnWorld::GetInstancePtr(GetRoom());

					pWorld->ScanProp( vPos, fDistance, pVecProp );

					for( DWORD i=0; i<pVecProp.size(); i++ ) 
					{
						if( false == ((CDnWorldProp*)pVecProp[i])->IsBrokenType() )
							continue;

						if( !((CDnWorldBrokenProp*)pVecProp[i])->IsHittable( GetMySmartPtr(), LocalTime ) ) 
							continue;

						pVecProp[i]->GetBoundingBox( Box );

						if( SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
						if( SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

						vDir = ((CDnWorldProp*)pVecProp[i])->GetMatEx()->m_vPosition - vPos;

						EtVec3Normalize( &vDir, &vDir );
						fDot = EtVec3Dot( &vZVec, &vDir );
						if( EtToDegree( acos( fDot ) ) > pStruct->fAngle ) 
							continue;

						if( Box.Min.y < vPos.y + pStruct->fHeightMin && Box.Max.y < m_Cross.m_vPosition.y + pStruct->fHeightMin ) 
							continue;
						if( Box.Min.y > vPos.y + fHeightMax && Box.Max.y > m_Cross.m_vPosition.y + fHeightMax ) 
							continue;

						HitParam.vViewVec = -vDir;
						((CDnWorldProp*)pVecProp[i])->OnDamage( GetMySmartPtr(), HitParam );
					}
				}

				//////////////////////////////////////////////////////////////////////////
				// End of prefix attack skill
				InitPrefixOffenceSkills();
				//////////////////////////////////////////////////////////////////////////

//				PROFILE_TICK_TEST_BLOCK_END();
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
				// When frozen, frames of the currently playing action may stop 
				// Fixed issue where Hit did not register even while frozen when STE_CanHit was False for a specific action, so while frozen always...
				// Make it match.
				if(GetStateBlow()->IsApplied( STATE_BLOW::BLOW_041)) 
				{
					SetHittable(true);
					break;
				}

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

				if( 2 <= pStruct->nWeaponIndex )	// Index for sub-weapon.
					break;

				if( !GetActiveWeapon( pStruct->nWeaponIndex ) ) break;
				if( pStruct->szActionName ) 
				{
					// For player actors, add the weapon's projectile count to the number of projectiles that can be fired in the current action.
					if( GetActiveWeapon( pStruct->nWeaponIndex )->IsExistAction( pStruct->szActionName ) )
					{
						GetActiveWeapon( pStruct->nWeaponIndex )->SetActionQueue( pStruct->szActionName );
					}
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
			}
			break;
		case STE_NormalSuperAmmor:
			{
				NormalSuperAmmorStruct *pStruct = (NormalSuperAmmorStruct *)pPtr;
				m_bEnableNormalSuperAmmor = ( pStruct->bEnable == TRUE ) ? true : false;
				m_nNormalSuperAmmorTime = pStruct->nTime;
				m_fBreakNormalSuperAmmorDurability = pStruct->fBreakDurability;

				if ( m_bIngnoreNormalSuperArmor )
				{
					m_bEnableNormalSuperAmmor = false;
				}
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
				ProjectileStruct *pStruct = (ProjectileStruct *)pPtr;

				OutputDebug("%s m_bOnSignalFromChargerSE: %d   m_ActorType: %d", __FUNCTION__, m_bOnSignalFromChargerSE, m_ActorType);
				// Note: For players, all projectile packets are received from client to spawn.
				// Even for Direction type, there are settings per local camera direction..
				bool bAIBotProjectile = ( IsPlayerActor() && static_cast<CDnPlayerActor*>(this)->IsAIBotActor() );
				if( false == m_bOnSignalFromChargerSE && false == bAIBotProjectile )
					if( m_ActorType <= CDnActorState::Reserved6 )
						break;

				MatrixEx LocalCross = *GetMatEx();
				EtVector3 vBotTargetPos( 0.f, 0.f, 0.f );
				DnActorHandle hBotTarget;
				if( bAIBotProjectile )
				{
#ifdef IRINE_MOD_PLAYER_BOTS
					CDnAIBotController* pBotCtrl = static_cast<CDnPlayerActor*>(this)->GetAIBotController();
					if( pBotCtrl )
					{
						hBotTarget = pBotCtrl->GetTargetActor();
						vBotTargetPos = pBotCtrl->GetAimPosition();
					}
					else
#endif
					{
						vBotTargetPos = *GetPosition() + ( LocalCross.m_vZAxis * 600.f );
					}
				}
				CDnProjectile *pProjectile = CDnProjectile::CreateProjectile( GetRoom(), GetMySmartPtr(), LocalCross, pStruct,
					bAIBotProjectile ? &vBotTargetPos : NULL,
					bAIBotProjectile ? hBotTarget : CDnActor::Identity() );
				if( pProjectile == NULL ) break;
				pProjectile->SetShooterType( GetMySmartPtr(), m_nActionIndex, nSignalIndex );

				//bool bActorAttachWeapon = false;
				//if( pStruct->nWeaponTableID == 0 && GetWeapon(1) ) bActorAttachWeapon = true;
				if( m_bOnSignalFromChargerSE && m_hChargetDestActor )
				{
					DNVector(DnBlowHandle) vlhChargerBlows;
					m_hChargetDestActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_062, vlhChargerBlows );	// Better not to allow more than 1 duplicate, though depends on design.
					for( int i = 0; i < (int)vlhChargerBlows.size(); ++i )
					{
						if( vlhChargerBlows[i] )
							static_cast<CDnChargerBlow*>( vlhChargerBlows.at( i ).GetPointer() )->OnProjectileSignal( pProjectile );
					}
				}

				if( false == m_bOnSignalFromChargerSE )
				{
#ifdef IRINE_MOD_PLAYER_BOTS
					if( bAIBotProjectile )
						CDnAIBotController::SendBotProjectilePacket( static_cast<CDnPlayerActor*>(this), pProjectile, pStruct, m_nActionIndex, nSignalIndex );
#endif
					OnProjectile( pProjectile, pStruct, LocalCross, nSignalIndex );
					OnSkillProjectile( pProjectile );
				}
			}
			break;
		case STE_SkillSuperAmmor:
			{
				SkillSuperAmmorStruct *pStruct = (SkillSuperAmmorStruct *)pPtr;

				m_nSkillSuperAmmorValue[0] = (int)( GetSuperAmmor() * pStruct->fSuperAmmorOneProb );
				m_nSkillSuperAmmorValue[1] = (int)( GetSuperAmmor() * pStruct->fSuperAmmorTwoProb );
				m_nSkillSuperAmmorValue[2] = (int)( GetSuperAmmor() * pStruct->fSuperAmmorThreeProb );
				m_nSkillSuperAmmorValue[3] = (int)( GetSuperAmmor() * pStruct->fSuperAmmorFourProb );
				memcpy( m_nLastUpdateSkillSuperAmmorValue, m_nSkillSuperAmmorValue, sizeof(m_nSkillSuperAmmorValue) );
				m_nSkillSuperAmmorTime = pStruct->nTime;
				m_fSkillSuperAmmorDamageDecreaseProb = pStruct->fDamageDecreaseProb;
			}
			break;
		case STE_SummonMonster:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonMonsterStruct* pStruct = (SummonMonsterStruct *)pPtr;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));
				if( pGameTask )
				{
					// Must copy before use because it modifies the value internally.
#ifdef PRE_FIX_MEMOPT_SIGNALH
					SummonMonsterStruct Struct;
					CopyShallow_SummonMonsterStruct(Struct, pStruct);
#else
					SummonMonsterStruct Struct = *pStruct;
#endif
					// Handle state effect #167: force summoned monster skill level.
					if( IsPlayerActor() )
					{
						if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_167 ) )
						{
							DNVector(DnBlowHandle) vlBlows;
							GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_167, vlBlows );
							if( false == vlBlows.empty() )
							{
								Struct.nForceSkillLevel = (int)vlBlows.front()->GetFloatValue();
							}
						}
					}
					//////////////////////////////////////////////////////////////////////////

					if(Struct.PositionCheck == TRUE )
					{
						MatrixEx Cross = *GetMatEx();
						Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
						Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
						Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

						if( INSTANCE(CDnWorld).IsOnNavigationMesh( Cross.GetPosition() ) == false )
							break;
					}
			
					pGameTask->RequestSummonMonster( GetActorHandle(), &Struct );
				}
			}
			break;
		//case STE_SummonProp:
		//	{
		//		SummonPropStruct *pStruct = (SummonPropStruct *)pPtr;

		//		MatrixEx Cross = m_Cross;
		//		Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
		//		Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
		//		Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

		//		float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
		//		EtVector3 vRotate;
		//		vRotate = *pStruct->vRotate;
		//		vRotate.y += EtToDegree( acos(fDot) );

		//		CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

		//		DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, Cross.m_vPosition, vRotate, *pStruct->vScale );
		//	}
		//	break;

		// ****************************************************************************************************************
		// Prop classes spawned by the prop summon signal must implement void ReleasePostCustomParam()!! 
		// If the prop struct is not deleted according to its type, boost::pool will suffer memory corruption
		// ****************************************************************************************************************
		case STE_SummonChestProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonChestPropStruct *pStruct = (SummonChestPropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				ChestStruct* pChestPropInfo = new ChestStruct;
				pChestPropInfo->nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				pChestPropInfo->nNeedKeyID = pStruct->nNeedKeyID;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_Chest, Cross.m_vPosition, vRotate, *pStruct->vScale, pStruct->nLifeTime, (void*)pChestPropInfo );

				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		case STE_SummonBrokenProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonBrokenPropStruct *pStruct = (SummonBrokenPropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				BrokenStruct* pBrokenPropInfo = new BrokenStruct;
				pBrokenPropInfo->nDurability = pStruct->nDurability;
				pBrokenPropInfo->nItemDropGroupTableID = pStruct->nItemDropGroupTableID;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_Broken, Cross.m_vPosition, vRotate, *pStruct->vScale, pStruct->nLifeTime, (void*)pBrokenPropInfo );
				
				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		case STE_SummonBrokenDamageProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonBrokenDamagePropStruct *pStruct = (SummonBrokenDamagePropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				BrokenDamageStruct* pBrokenDamagePropInfo = new BrokenDamageStruct;
				pBrokenDamagePropInfo->nMonsterTableID = 102;	// goblin
				pBrokenDamagePropInfo->nDurability = pStruct->nDurability;
				pBrokenDamagePropInfo->nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				pBrokenDamagePropInfo->nSkillTableID = pStruct->nSkillTableID;
				pBrokenDamagePropInfo->nSkillLevel = pStruct->nSkillLevel;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_BrokenDamage, Cross.m_vPosition, vRotate, 
																		*pStruct->vScale, pStruct->nLifeTime, (void*)pBrokenDamagePropInfo );
				if( !hProp )
					break;

				MAActorProp* pActorProp = dynamic_cast<MAActorProp*>(hProp.GetPointer());
				_ASSERT( pActorProp );
				if( pActorProp )
					pActorProp->CopyActorStateFromThis( GetMySmartPtr() );

				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		case STE_SummonHitMoveDamageBrokenProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonHitMoveDamageBrokenPropStruct *pStruct = (SummonHitMoveDamageBrokenPropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				HitMoveDamageBrokenStruct* pHitMoveDamageBrokenPropInfo = new HitMoveDamageBrokenStruct;
				pHitMoveDamageBrokenPropInfo->nMonsterTableID = 102;	// goblin
				pHitMoveDamageBrokenPropInfo->nDurability = pStruct->nDurability;
				pHitMoveDamageBrokenPropInfo->nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				pHitMoveDamageBrokenPropInfo->MovingAxis = pStruct->MovingAxis;
				pHitMoveDamageBrokenPropInfo->fDefaultAxisMovingSpeed = pStruct->fDefaultAxisMovingSpeed;
				pHitMoveDamageBrokenPropInfo->fMaxAxisMoveDistance = pStruct->fMaxAxisMoveDistance;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_HitMoveDamageBroken, Cross.m_vPosition, vRotate, 
																	   *pStruct->vScale, pStruct->nLifeTime, (void*)pHitMoveDamageBrokenPropInfo );
				if( !hProp )
					break;

				MAActorProp* pActorProp = dynamic_cast<MAActorProp*>(hProp.GetPointer());
				_ASSERT( pActorProp );
				if( pActorProp )
					pActorProp->CopyActorStateFromThis( GetMySmartPtr() );

				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		case STE_SummonBuffProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonBuffPropStruct *pStruct = (SummonBuffPropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				BuffStruct* pBuffPropInfo = new BuffStruct;
				pBuffPropInfo->nMonsterTableID = 102; // goblin
				pBuffPropInfo->nSkillTableID = pStruct->nSkillTableID;
				pBuffPropInfo->nSkillLevel = pStruct->nSkillLevel;
				pBuffPropInfo->nTeam = pStruct->nTeam;
				pBuffPropInfo->fCheckRange = pStruct->fCheckRange;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_Buff, Cross.m_vPosition, vRotate, *pStruct->vScale, pStruct->nLifeTime, (void*)pBuffPropInfo );

				if( !hProp )
					break;

				MAActorProp* pActorProp = dynamic_cast<MAActorProp*>(hProp.GetPointer());
				_ASSERT( pActorProp );
				if( pActorProp )
					pActorProp->CopyActorStateFromThis( GetMySmartPtr() );

				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				static_cast<CDnBuffProp*>(hProp.GetPointer())->SetSummoner( GetMySmartPtr() );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		case STE_SummonBuffBrokenProp:
			{
				// #48747 When attacking via summons like Academic, summoning should also be blocked if caught by speedhack detection.
				if( IsPlayerActor() && ((CDnPlayerActor*)this)->IsInvalidPlayerChecker() ) break;

				SummonBuffBrokenPropStruct *pStruct = (SummonBuffBrokenPropStruct *)pPtr;

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vPosition->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vPosition->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vPosition->z;

				float fDot = EtVec3Dot( &EtVector3( 0.f, 0.f, 1.f ), &Cross.m_vZAxis );
				EtVector3 vRotate;
				vRotate = *pStruct->vRotate;
				vRotate.y += EtToDegree( acos(fDot) );

				BuffBrokenStruct* pBuffBrokenPropInfo = new BuffBrokenStruct;
				pBuffBrokenPropInfo->nMonsterTableID = 102; // goblin
				pBuffBrokenPropInfo->nDurability = pStruct->nDurability;
				pBuffBrokenPropInfo->nItemDropGroupTableID = pStruct->nItemDropGroupTableID;
				pBuffBrokenPropInfo->nSkillTableID = pStruct->nSkillTableID;
				pBuffBrokenPropInfo->nSkillLevel = pStruct->nSkillLevel;
				pBuffBrokenPropInfo->nTeam = pStruct->nTeam;
				pBuffBrokenPropInfo->fCheckRange = pStruct->fCheckRange;

				CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstancePtr(GetRoom())->GetTask( "GameTask" ));

				DnPropHandle hProp = pGameTask->RequestGenerationProp( pStruct->nSummonPropTableID, PTE_BuffBroken, Cross.m_vPosition, vRotate, *pStruct->vScale, pStruct->nLifeTime, (void*)pBuffBrokenPropInfo );

				if( !hProp )
					break;

				MAActorProp* pActorProp = dynamic_cast<MAActorProp*>(hProp.GetPointer());
				_ASSERT( pActorProp );
				if( pActorProp )
					pActorProp->CopyActorStateFromThis( GetMySmartPtr() );

				if( -1 < pStruct->nLifeTime )
					hProp->SetLifeTime( pStruct->nLifeTime );

				static_cast<CDnWorldBrokenBuffProp*>(hProp.GetPointer())->SetSummoner( GetMySmartPtr() );

				hProp->SetMasterActor(GetMySmartPtr());
			}
			break;
		// ****************************************************************************************************************
		// Prop classes spawned by the prop summon signal must implement void ReleasePostCustomParam()!! 
		// ****************************************************************************************************************
		case STE_DropItem:
			{
				DropItemStruct *pStruct = (DropItemStruct*)pPtr;

				DNVector(CDnItem::DropItemStruct) VecList;
				CDnDropItem::CalcDropItemList( GetRoom(), pStruct->nDropItemTableID, VecList );

				MatrixEx Cross = m_Cross;
				Cross.m_vPosition += Cross.m_vXAxis * pStruct->vOffset->x;
				Cross.m_vPosition += Cross.m_vYAxis * pStruct->vOffset->y;
				Cross.m_vPosition += Cross.m_vZAxis * pStruct->vOffset->z;
				CDnItemTask *pTask = (CDnItemTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "ItemTask" );

				int nSeed = 0;
				for( DWORD i=0; i<VecList.size(); i++ ) 
				{
#if defined(PRE_FIX_46730)
					//When bFixPosition is set to true, set nSeed to 0 to skip item position adjustment.
					if (( pStruct->bFixPosition?true:false) == true)
						nSeed = 0;
					else
						nSeed = VecList[i].nSeed;
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[i].dwUniqueID, Cross.m_vPosition, VecList[i].nItemID, nSeed, VecList[i].nCount, 0, -1, VecList[i].nEnchantID );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[i].dwUniqueID, Cross.m_vPosition, VecList[i].nItemID, nSeed, VecList[i].nCount, 0 );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
#else
#if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[i].dwUniqueID, Cross.m_vPosition, VecList[i].nItemID, VecList[i].nSeed, VecList[i].nCount, 0, -1, VecList[i].nEnchantID );
#else	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
					pTask->RequestDropItem( VecList[i].dwUniqueID, Cross.m_vPosition, VecList[i].nItemID, VecList[i].nSeed, VecList[i].nCount, 0 );
#endif	// #if defined(PRE_ADD_STAGE_CLEAR_ENCHANT_REWARD)
#endif // PRE_FIX_46730
				}
			}
			break;
			// Hanki 2009.7.27 
			// Related to effect application duration used on self (#1911)
		case STE_ApplyStateEffect:
			{
				// If the actor dies upon skill use before the state effect is applied, the state effect is not added, so 
				// Do not proceed to the state effect routine below.
				if( GetHP() <= 0 )
				{
					if( m_hAuraSkill && m_hAuraSkill->IsAuraOn() )
					{
						OnSkillAura( m_hAuraSkill, false );
					}

					ClearSelfStateSignalBlowQueue();

					break;
				}

				if( false == m_dqApplySelfStateBlowQ.empty() )
				{
					ApplyStateEffectStruct* pStruct = (ApplyStateEffectStruct*)pPtr;
					if( pStruct->StateEffectIndex < (int)m_dqApplySelfStateBlowQ.size() )
					{
						const S_NO_PACKET_SELF_STATEBLOW& StateBlowInfo = m_dqApplySelfStateBlowQ.at( pStruct->StateEffectIndex );
						ApplyStateEffectSignalProcess(StateBlowInfo, pStruct);
					}
					else
					{
						OutputDebug( "[Error!!] CDnActor::OnSignal STE_ApplyStateEffect: skill state effect count (%d) exceeded by applied index (%d).\n", 
							(int)m_dqApplySelfStateBlowQ.size(), pStruct->StateEffectIndex );
						_ASSERT( !"STE_ApplyStateEffect: state effect index out of skill state effect count" );
					}
				}
			}
			break;
		case STE_Parring:
			{
				ParringStruct* pStruct = (ParringStruct*)pPtr;

				DNVector(DnBlowHandle) vlhParryBlows;
				m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_031, vlhParryBlows );
				
				int iNumParryBlow = (int)vlhParryBlows.size();
				for( int i = 0; i < iNumParryBlow; ++i )
				{
					DnBlowHandle hParryBlow = vlhParryBlows.at( i );
					if( !hParryBlow )
						continue;

					const CDnSkill::SkillInfo* pParentSkillInfo = hParryBlow->GetParentSkillInfo();
					if( pParentSkillInfo && pStruct->nSkillID == pParentSkillInfo->iSkillID )
					{
						CDnParryBlow* pParryBlow = static_cast<CDnParryBlow*>(hParryBlow.GetPointer());

						if( false == pParryBlow->IsEnabled() )
						{
							pParryBlow->EnableParrying( pStruct->fProb );
							pParryBlow->SetParringActionName( pStruct->szParringActionName );
						}
					}
				}
			}
			break;
		case STE_CooltimeParring:
			{
				CooltimeParringStruct* pStruct = (CooltimeParringStruct*)pPtr;

				DNVector(DnBlowHandle) vlhParryBlows;
				m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_153, vlhParryBlows );

				int iNumParryBlow = (int)vlhParryBlows.size();
				for( int i = 0; i < iNumParryBlow; ++i )
				{
					DnBlowHandle hParryBlow = vlhParryBlows.at( i );
					if( !hParryBlow )
						continue;

					const CDnSkill::SkillInfo* pParentSkillInfo = hParryBlow->GetParentSkillInfo();
					if( pParentSkillInfo && pStruct->nSkillID == pParentSkillInfo->iSkillID )
					{
						CDnCooltimeParryBlow* pParryBlow = static_cast<CDnCooltimeParryBlow*>(hParryBlow.GetPointer());

						if( false == pParryBlow->IsEnabled() )
						{
							pParryBlow->EnableParrying( pStruct->fProb );
							pParryBlow->SetParringActionName( pStruct->szParringActionName );
						}
					}
				}
			}
			break;
		case STE_Gravity:
			{
				GravityStruct *pStruct = (GravityStruct *)pPtr;

				MatrixEx CrossOffset = m_Cross;
				CrossOffset.MoveLocalXAxis( pStruct->vOffset->x );
				CrossOffset.MoveLocalYAxis( pStruct->vOffset->y );
				CrossOffset.MoveLocalZAxis( pStruct->vOffset->z );

				DNVector(DnActorHandle) hVecList;
				CDnActor::ScanActor( GetRoom(), CrossOffset.m_vPosition, pStruct->fDistanceMax, hVecList );

#ifdef PRE_ADD_GRAVITY_PROPERTY
				EtVector3 vPos = CrossOffset.m_vPosition;

				float fXZDistanceMaxSQ = pStruct->fDistanceMax;
				float fXZDistanceMinSQ = pStruct->fDistanceMin;

				fXZDistanceMaxSQ *= fXZDistanceMaxSQ;
				fXZDistanceMinSQ *= fXZDistanceMinSQ;
		
				SAABox Box;
				float fDot = 0.0f;
				EtVector3 vZVec = m_Cross.m_vZAxis;

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
					bool bCheckArea = true;
					DnActorHandle hActor = hVecList[i];
					EtVector3 vDirActorToPos;
					vDirActorToPos = *hActor->GetPosition() - vPos;
					vDirActorToPos.y = 0.f;

					hActor->GetBoundingBox( Box );
					
					float fSQDistance = SquaredDistance( vPos, Box );

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
						hActor->SetVelocityZ( 0.0f );
						hActor->SetResistanceZ( 0.0f );
						hActor->ResetLook();
						continue;
					}
#endif // PRE_ADD_GRAVITY_PROPERTY

					MAMovementBase *pMovement = hVecList[i]->GetMovement();
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
					if( pMovement ) {
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
				float fLength = (float)( SignalEndTime - SignalStartTime ) * (CDnActionBase::m_fFPS / 60.0f); 
				DWORD dwTime = (DWORD)( fLength - 16.6666f );
				SetRotate( dwTime, pStruct->fStartSpeed, pStruct->fEndSpeed, *(pStruct->vAxis), pStruct->bLeft == TRUE ? true : false );
			
			}
			break;
	}

}

LOCAL_TIME CDnActor::CheckRemainFrameTime()
{
	float fCurFrame = GetCurFrame();
	int nCurActionIndex = GetCurrentActionIndex();
	ActionElementStruct *pStruct = GetElement(nCurActionIndex);
	LOCAL_TIME remainTime = 0;

	if (pStruct)
		remainTime = pStruct->dwLength - (LOCAL_TIME)fCurFrame;

	return remainTime;
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
			{
				LOCAL_TIME remainTime = CheckRemainFrameTime();
				
				//If the current action is nearly finished, treat it as ended and queue the next action.
				if (remainTime >= 20)
					return false;
			}
		}
		else {
			if( strcmp( szName, m_szActionQueue.c_str() ) == NULL ) 
			{
				LOCAL_TIME remainTime = CheckRemainFrameTime();

				//If the current action is nearly finished, treat it as ended and queue the next action.
				if (remainTime >= 20)
					return false;
			}
		}
	}

	int nCurrentActionIndex = GetElementIndex( szName );
	if( bCheckStateEffect && nCurrentActionIndex > -1 )
	{
		int nActionState = GetState( nCurrentActionIndex );

		// Subdivided in detail according to State
		// Cannot use actions in attack state when unable to attack.
		if( (nActionState & ActorStateEnum::Attack) && !( nActionState & ActorStateEnum::IgnoreCantAction) )
		{
			if( (GetStateEffect() & CDnActorState::Cant_AttackAction) == CDnActorState::Cant_AttackAction ) 
			{
				if( !IsDie() || IsGMTrace() ) 
					return false;
			}
		}
	}

	CDnActionBase::SetActionQueue( szName, nLoopCount, fBlendFrame, fStartFrame );
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
	if( m_hToggleSkill && m_hToggleSkill->IsToggleOn() )
	{
		CDnChangeActionStrProcessor* pChangeAction = static_cast<CDnChangeActionStrProcessor*>( m_hToggleSkill->GetProcessor( IDnSkillProcessor::CHANGE_ACTIONSTR ) );
		if( pChangeAction ) {
			pChangeAction->GetChangeActionName( szActionName );
		}
	}
	CDnActionBase::SetCustomAction( szActionName, fFrame );
}

void CDnActor::ProcessDown( LOCAL_TIME LocalTime, float fDelta )
{
	if( IsDie() ) return;
	if( GetState() == CDnActorState::Down && m_fDownDelta > 0.f ) 
	{
		m_fDownDelta -= ( fDelta * ( ( 1.f / 60.f ) * CDnActionBase::GetFPS() ) );
		if( m_fDownDelta <= 0.f ) {
			m_fDownDelta = 0.f;
			m_fLastDownRatio = 1.f;
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

void CDnActor::RequestKillAfterProcessStateBlow(DnActorHandle hHitter)
{
	m_bCompleteKill_AfterProcessStateBlow = true;
	m_hCompleteKillActor = hHitter;
}

void CDnActor::ExecuteKillAfterProcessStateBlow()
{
	if(m_bCompleteKill_AfterProcessStateBlow)
	{
		SetHP( 0 );
		RequestHPMPDelta( CDnState::ElementEnum_Amount, -( GetMaxHP() *2 ), GetUniqueID() );
		m_bCompleteKill_AfterProcessStateBlow = false;

		//Final Die call
		Die(m_hCompleteKillActor);
		m_hCompleteKillActor.Identity();
	}
}

void CDnActor::CmdRefreshHPSP( INT64 nHP, int nSP )
{
	SetHP( min( nHP, GetMaxHP() ) );
	SetSP( min( nSP, GetMaxSP() ) );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	Stream.Write( &m_nHP, sizeof(INT64) );
	Stream.Write( &m_nSP, sizeof(int) );
	

	Send( eActor::SC_REFRESHHPSP, &Stream );
}

void CDnActor::CmdAction( const char *szActionName, int nLoopCount, float fBlendFrame, bool bCheckOverlapAction, bool bFromStateBlow/* = false*/, bool bSkillChain/* = false*/ )
{
	SetActionQueue( szActionName, nLoopCount, fBlendFrame, 0.f, bCheckOverlapAction );

	// Note: When a skill branches into multiple actions depending on input, like Rising Slash, 
	// Notify the skill side since the state effect must persist.
	// Sent to the server in the packet when CmdActon is called.
	if( bSkillChain )
	{
		if( m_hProcessSkill )
			m_hProcessSkill->OnChainInput( szActionName );
	}
}

int CDnActor::CmdAddStateEffect( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX emBlowIndex, int nDurationTime, const char *szParam, bool bOnPlayerInit/* = false*/, bool bCheckCanBegin/* = true*/ ,  bool bEternity /*= false*/ )
{
	if( emBlowIndex < STATE_BLOW::BLOW_NONE || emBlowIndex >= STATE_BLOW::BLOW_MAX )
	{
		if( pParentSkillInfo )
			g_Log.Log(LogType::_ERROR, L"[CDnActor::CmdAddStateEffect] SkillID:%d, STATE_BLOW:%d\r\n", pParentSkillInfo->iSkillID, emBlowIndex );
		else
			g_Log.Log(LogType::_ERROR, L"[CDnActor::CmdAddStateEffect] STATE_BLOW:%d\r\n", emBlowIndex);
		return -1;
	}

	// [2011/03/18 semozz]
	// Modified code to force Begin on state effect
	int nBlowID = AddStateBlow( emBlowIndex, pParentSkillInfo, nDurationTime, szParam, bOnPlayerInit, bCheckCanBegin , bEternity );
	
	DnBlowHandle hBlow = GetStateBlowFromID(nBlowID);
	if (pParentSkillInfo && pParentSkillInfo->bItemPrefixSkill)
		ForceBeginStateBlow(hBlow);

	return nBlowID;
}

void CDnActor::CmdModifyStateEffect( int iBlowID, STATE_BLOW& StateBlowInfo )
{
	DnBlowHandle hBlow = m_pStateBlow->GetStateBlowFromID( iBlowID );

	if( hBlow )
	{
		char acBuffer[ 32 ];
		CPacketCompressStream Stream( acBuffer, sizeof(acBuffer) );

		Stream.Write( &iBlowID, sizeof(int) );
		Stream.Write( &StateBlowInfo.fDurationTime, sizeof(float) );

		Send( eActor::SC_CMDMODIFYSTATEEFFECT, &Stream );	
	}
}

DnBlowHandle CDnActor::GetStateBlowFromID( int nStateBlowID )
{
	return m_pStateBlow->GetStateBlowFromID( nStateBlowID ); 
}

void CDnActor::CmdRemoveStateEffect( STATE_BLOW::emBLOW_INDEX emBlowIndex, bool bRemoveFromServerToo/* = true*/ )
{
	DelStateBlow( emBlowIndex );
}

void CDnActor::CmdRemoveStateEffectImmediately( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	if( !m_pStateBlow ) 
		return;

	m_pStateBlow->RemoveImediatlyStateEffectByBlowIndex( emBlowIndex );
}

void CDnActor::CmdRemoveStateEffectFromID( int nID )
{
#ifdef PRE_FIX_REMOVE_STATE_EFFECT_PACKET
	// Putting it here might cause performance overhead, so skipping it for now.
	// In most cases blow ID is inserted after being found, so checking doesn't seem necessary.
	// if( m_pStateBlow->IsExistStateBlowFromBlowID( nID ) == false )
	//	return;
#endif

	m_pStateBlow->RemoveStateBlowFromID( nID );
	SendRemoveStateEffectFromID(nID);
}

void CDnActor::ClearTriggerBuff()
{
	for( int itr = 0; itr < (int)m_vTriggerBuff.size(); ++itr )
		CmdRemoveStateEffectFromID( m_vTriggerBuff[itr] );

	m_vTriggerBuff.clear();
}

void CDnActor::CmdWarp()
{
	CmdWarp( *GetPosition(), EtVec3toVec2( *GetLookDir() ) );
}

void CDnActor::CmdWarp( const EtVector3 &vPos, const EtVector2 &vLook, CDNUserSession* pGameSession/*=NULL*/, bool bCheckPlayerFollowSummonedMonster/*=false*/ )
{
	EtVector3 pos = vPos;
	EtVector2 look = vLook;
	SetPosition( pos );
	SetPrevPosition( pos );
	Look( look, true );
	if( abs( CDnWorld::GetInstance(GetRoom()).GetHeight( pos ) - pos.y ) > 5.f ) {
		SetVelocityY( -3.0f );
		SetResistanceY( -18.0f );
	}

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &pos, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &look, sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );

	// If pGameSession exists, send the packet only to that session.
	if( pGameSession )
		Send( pGameSession, eActor::SC_CMDWARP, GetUniqueID(), &Stream );
	else
		Send( eActor::SC_CMDWARP, &Stream );
}

bool CDnActor::CheckDamageAction( DnActorHandle hActor )
{
	MAMovementBase *pMovement = GetMovement();
	// Set appearance
	if( ( IsAir() ) ||
		( IsHit() && IsDown() && !IsStun() ) 
		|| GetState() == ActorStateEnum::Down ) 
	{
			if( IsStandHit( m_HitParam.szActionName.c_str() ) ) 
			{
				m_HitParam.szActionName = "Hit_AirBounce";
				m_HitParam.vVelocity.y = pMovement->GetLastVelocity()->y * 0.8f;
				m_HitParam.vResistance.y = pMovement->GetResistance()->y;

				if( m_HitParam.vVelocity.y > 0.f && m_HitParam.vResistance.y >= 0.f )
				{
					m_HitParam.vResistance.y = -18.f;
				}
			}
	}

	return true;
}

void CDnActor::CheckDamageVelocity( DnActorHandle hActor )
{
	MAMovementBase *pMovement = GetMovement();

	// Set look-at direction
	EtVector2 vViewVec = EtVec3toVec2( m_HitParam.vViewVec );
	EtVec2Normalize( &vViewVec, &vViewVec );
	if( EtVec2LengthSq( &vViewVec ) > 0.f ) {
		pMovement->Look( vViewVec );
	}

	// In case an Air Hit motion arrives even when not airborne at all, launch upward with an arbitrary value here
	// (e.g., when Warrior hits a knocked-down enemy and gets hit by stun, it should launch, but without y-axis acceleration the animation can stay floating in midair.)
	ActorStateEnum HitState = (ActorStateEnum)GetState( m_HitParam.szActionName.c_str() );
	if( ( HitState & ActorStateEnum::Air ) && m_HitParam.vVelocity.y == 0.f ) {
		m_HitParam.vVelocity.y = 3.f;
		m_HitParam.vResistance.y = -18.f;
	}
	if( m_HitParam.vVelocity.y == 0.f && pMovement->GetVelocity() && pMovement->GetVelocity()->y != 0.f && 
		( GetAddHeight() != 0.f ) || ( m_HitParam.szActionName.empty() && ( GetState() & ActorStateEnum::Air ) ) ) {
		m_HitParam.vVelocity.y = pMovement->GetVelocity()->y;
		m_HitParam.vResistance.y = pMovement->GetResistance()->y;
	}

	// If Weight is 0, it should not float. However, if it becomes Air state during a signal, it ignores the exception handling routine above, so
	// Since some entities on the ground can stay in Air state, those with 0 must never enter Air state.
	if( GetWeight() == 0.f ) 
	{
		// Cases like Remote Island (301) vine trap where weight is changed to 0.0 upon being hit by cannon etc. 
		// Fixed potential bug where airborne knockback action prevents landing on the ground. (#12253)
		if( IsAir( m_HitParam.szActionName.c_str() ) )
			m_HitParam.szActionName.assign( GetCurrentAction() );

		m_HitParam.vVelocity = EtVector3( 0.f, 0.f, 0.f );
		m_HitParam.vResistance = EtVector3( 0.f, 0.f, 0.f );
	}
	pMovement->SetVelocity( m_HitParam.vVelocity );
	pMovement->SetResistance( m_HitParam.vResistance );
}

int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom )
{
	float f1 = ( _rand(pRoom) % 10001 ) / 10000.f;
	float f2 = ( _rand(pRoom) % 10001 ) / 10000.f;

	int nResult = (int)( ((nMin+nMax)/2.f) + ( ( sqrt(-2.f*log(f1)) * cos(2.f*3.1415926f*f2) ) * ((nMax-nMin)/6.f) ) );

	if( nResult < nMin || nResult > nMax )
		nResult = (nMin + nMax) / 2; 

	return nResult;
};

float GetGaussianRandom( float fMin, float fMax, CMultiRoom *pRoom, float fPrecision = 0.01f )
{
	float f1 = ( _rand(pRoom) % 10001 ) / 10000.f;
	float f2 = ( _rand(pRoom) % 10001 ) / 10000.f;

	float fResult = ((fMin+fMax)/2.f) + ( ( sqrt(-2.f*log(f1)) * cos(2.f*3.1415926f*f2) ) * ((fMax-fMin)/6.f) );

	if( fResult < fMin || fResult > fMax )
		fResult = (fMin + fMax) / 2; 

	return fResult;
};


CDnState::ElementEnum CDnActor::CalcHitElementType( bool bUseSkillApplyWeaponElement, CDnState::ElementEnum eSkillElement, bool bUsingSkill )
{
	// #33312 If an element was forcefully set in a specific situation, process with that element.
	if( false == m_vlForceSettedHitElement.empty() )
		return static_cast<CDnState::ElementEnum>(m_vlForceSettedHitElement.back());

	bool bProcessSkill = (m_hProcessSkill ? true : false);
	ElementEnum eProcessSkillElement = CDnState::ElementEnum_Amount;
	if( m_hProcessSkill )
		eProcessSkillElement = m_hProcessSkill->GetElement();

	// When called externally with skill usage info (projectile fired by skill)
	if( bUsingSkill )
	{
		bProcessSkill = bUsingSkill;
		eProcessSkillElement = eSkillElement;
	}

	ElementEnum Type = CDnState::ElementEnum_Amount;
	if( bProcessSkill ) 
	{
		if( eProcessSkillElement == CDnState::ElementEnum_Amount ) 
		{
			if( bUseSkillApplyWeaponElement ) 
			{
				if( IsPlayerActor() )
				{
					Type = ( CDnActor::GetWeapon() ) ? CDnActor::GetWeapon()->GetElementType() : CDnState::ElementEnum_Amount;
				}
				else if( IsMonsterActor() )
				{
					Type = static_cast<CDnMonsterActor*>(this)->GetElementType();
				}
			}
		}
		else
		{
			Type = eProcessSkillElement;
		}
	}
	else
	{
		if( m_ActorType <= ActorTypeEnum::Reserved6 )
			Type = ( CDnActor::GetWeapon() ) ? CDnActor::GetWeapon()->GetElementType() : CDnState::ElementEnum_Amount;
		else
			Type = static_cast<CDnMonsterActor*>(this)->GetElementType();
	}

	return Type;
}


// Wrap state-effect-related sections with #pragma region so they can be folded
// Setting a color for "Collapsible Text" in Visual Studio font options improves readability.
float CDnActor::PreCalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam, const float fDefenseRate/*=1.f*/, float fStateEffectAttackM /*= -1.0f */ )
{
	float fResult = 0.f;
	float fHitPer = HitParam.fDamage;
	float fAttackPower = 0.0f;
	
	int nDefense = 0;
	float fDefenseWeight = 0.0f;

	float fElementWeight = 1.f;
	float fGausianResultProb = 1.f;

	DnActorHandle hHitter = pHitter->GetActorHandle();
	CDnState *pState = NULL;
	CDnStateBlow *pHitterStateBlow = hHitter->GetStateBlow();

	bool bCalcDamageFromStateEffect = ( fStateEffectAttackM != -1.0f );

	if( bCalcDamageFromStateEffect == true )
	{
		if( hHitter )
			pState = static_cast<CDnActorState *>(hHitter.GetPointer());

		fAttackPower = fStateEffectAttackM;
		nDefense = GetDefenseM();
	}
	else
	{
		if( hHitter && !HitParam.bFromProjectile ) 
		{ 
			pState = static_cast<CDnActorState *>(hHitter.GetPointer());
		}
		else 
		{
			if( HitParam.hWeapon )
			{
				CDnProjectile *pHitterProjectile = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() );
				if( pHitterProjectile )
				{
					pState = pHitterProjectile->GetShooterStateSnapshot().get();
#if defined(PRE_ADD_PROJECTILE_SE_INFO)					
					CDnStateBlow *pProjectileStateBlow = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() )->GetShooterStateBlow().get();
					if( pProjectileStateBlow )
						pHitterStateBlow = pProjectileStateBlow;
#endif
				}
			}

#if !defined(PRE_ADD_PROJECTILE_SE_INFO)
			if (hHitter&& hHitter->IsApplyPrefixOffenceSkill())
				pState = static_cast<CDnActorState *>(hHitter.GetPointer());
#endif

			if( hHitter && pState == NULL )
				pState = static_cast<CDnActorState *>(hHitter.GetPointer());
		}
	}


	if( bCalcDamageFromStateEffect == false )
	{
		int nAttack[2] = { 0, };

		switch( HitParam.cAttackType ) 
		{
		case 0: // Physical
			{
				nAttack[0] = pState->GetAttackPMin();
				nAttack[1] = pState->GetAttackPMax();

#pragma region ++         [   ADD DAMAGE BY SELF ( BLOW_109 )    ]               ++

				if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_109 ) )
				{
					int aiAdditionalPAttack[ 2 ] = { 0 };
					DNVector(DnBlowHandle) vlhBlows;
					GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_109, vlhBlows );
					int iNumBlow = (int)vlhBlows.size();
					for( int i = 0; i < iNumBlow; ++i )
					{
						aiAdditionalPAttack[ 0 ] += int((float)GetAttackPMin() * vlhBlows.at(i)->GetFloatValue());
						aiAdditionalPAttack[ 1 ] += int((float)GetAttackPMax() * vlhBlows.at(i)->GetFloatValue());
					}

					nAttack[ 0 ] += aiAdditionalPAttack[ 0 ];
					nAttack[ 1 ] += aiAdditionalPAttack[ 1 ];
				}

#pragma endregion

				nDefense = GetDefenseP();
			}
			break;

		case 1: // Magic
		{
				nAttack[0] = pState->GetAttackMMin();
				nAttack[1] = pState->GetAttackMMax();

#pragma region ++         [   ADD DAMAGE BY HITTER ( BLOW_180 )    ]               ++

				if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_180) )
				{
					CDnActorState* pActorState = static_cast<CDnActorState *>(hHitter.GetPointer());
					CDnState* pSkillState = pActorState ? pActorState->GetStateStep(1) : NULL;

					if (pSkillState)
					{
						nAttack[0] += pSkillState->GetAttackPMin();
						nAttack[1] += pSkillState->GetAttackPMax();
					}
				}

#pragma endregion

#pragma region ++         [   ADD DAMAGE BY SELF ( BLOW_110 )    ]               ++

				if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_110 ) )
				{
					int aiAdditionalMAttack[ 2 ] = { 0 };
					DNVector(DnBlowHandle) vlhBlows;
					GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_110, vlhBlows );
					int iNumBlow = (int)vlhBlows.size();
					for( int i = 0; i < iNumBlow; ++i )
					{
						aiAdditionalMAttack[ 0 ] += int((float)GetAttackMMin() * vlhBlows.at(i)->GetFloatValue());
						aiAdditionalMAttack[ 1 ] += int((float)GetAttackMMax() * vlhBlows.at(i)->GetFloatValue());
					}

					nAttack[ 0 ] += aiAdditionalMAttack[ 0 ];
					nAttack[ 1 ] += aiAdditionalMAttack[ 1 ];
				}

#pragma endregion

				nDefense = GetDefenseM();
			}
			break;
		}

#pragma region ++         [   ADD DAMAGE BY HITTER ( BLOW_213 )    ]               ++
		
		if( pHitterStateBlow && m_pStateBlow->IsApplied(STATE_BLOW::BLOW_213) )
		{
			int nAddAttack[2] = { 0, };
			int HitterAttackP[2] = {pState->GetAttackPMin(), pState->GetAttackPMax()};

			DNVector(DnBlowHandle) vlhBlows;
			pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_213, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				nAddAttack[ 0 ] += int((float)HitterAttackP[0] * vlhBlows.at(i)->GetFloatValue());
				nAddAttack[ 1 ] += int((float)HitterAttackP[1] * vlhBlows.at(i)->GetFloatValue());
			}

			nAttack[ 0 ] += nAddAttack[ 0 ];
			nAttack[ 1 ] += nAddAttack[ 1 ];
		}

#pragma endregion

#pragma region ++         [   ADD DAMAGE BY HITTER ( BLOW_214 )    ]               ++

		if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_214) )
		{
			int nAddAttack[2] = { 0, };
			int HitterAttackM[2] = {pState->GetAttackMMin(), pState->GetAttackMMax()};

			DNVector(DnBlowHandle) vlhBlows;
			pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_214, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				nAddAttack[ 0 ] += int((float)HitterAttackM[0] * vlhBlows.at(i)->GetFloatValue());
				nAddAttack[ 1 ] += int((float)HitterAttackM[1] * vlhBlows.at(i)->GetFloatValue());
			}

			nAttack[ 0 ] += nAddAttack[ 0 ];
			nAttack[ 1 ] += nAddAttack[ 1 ];
		}

#pragma endregion

#pragma region ++         [   ADD DAMAGE BY HITTER ( BLOW_221 )    ]               ++

		if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_221))
		{
			int nAddAttack[2] = { 0, };
			int HitterAttackValue[2] = {0, };
			
			switch( HitParam.cAttackType ) 
			{
			case 0: //When physical attack.
				HitterAttackValue[0] = pState->GetAttackPMin();
				HitterAttackValue[1] = pState->GetAttackPMax();
				break;
			case 1: //When it is a magic attack.
				HitterAttackValue[0] = pState->GetAttackMMin();
				HitterAttackValue[1] = pState->GetAttackMMax();
				break;
			}
			
			DNVector(DnBlowHandle) vlhBlows;
			pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_221, vlhBlows );
			int iNumBlow = (int)vlhBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				nAddAttack[ 0 ] += int((float)HitterAttackValue[0] * vlhBlows.at(i)->GetFloatValue());
				nAddAttack[ 1 ] += int((float)HitterAttackValue[1] * vlhBlows.at(i)->GetFloatValue());
			}

			nAttack[ 0 ] += nAddAttack[ 0 ];
			nAttack[ 1 ] += nAddAttack[ 1 ];
		}

#pragma endregion


		float fFinalDamageValue[4] = 
		{ 
			CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue1 ), 
			CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue2 ), 
			CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue3 ),
			CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::FinalDamageValue4 ),
		};

		float fFValue = GetFinalDamageConstant();

		float fFinalDamage1 = (float)( pState->GetFinalDamage() / fFValue ) * fFinalDamageValue[3];
		float fFinalDamage2 = powf( (float)( pState->GetFinalDamage() / fFValue ), fFinalDamageValue[1] ) * fFinalDamageValue[2];
		float fResultFinalDamage = min( max(fFinalDamage1, fFinalDamage2), fFinalDamageValue[2] );

		// State effects #184, #185: apply attacker final damage separately for physical/magic attack
		float fResultAdditionalFinalDamage = 0.0f;
		
		if( 0 == HitParam.cAttackType ) // Physical attack
		{

#pragma region ++         [   fResultAdditionalFinalDamage BY HITTER  ( BLOW_184 )    ]               ++

			if( pHitterStateBlow && pHitterStateBlow->IsApplied( STATE_BLOW::BLOW_184 ) )
			{
				DNVector(DnBlowHandle) vlhFinalPDamageBlows;
				pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_184, vlhFinalPDamageBlows );
				int iNumBlow = (int)vlhFinalPDamageBlows.size();
				for( int i = 0; i < iNumBlow; ++i )
				{
					DnBlowHandle hBlow = vlhFinalPDamageBlows.at( i );
					fResultAdditionalFinalDamage += hBlow->GetFloatValue();
				}
			}

#pragma endregion

		}
		else if( 1 == HitParam.cAttackType )
		{

#pragma region ++         [   fResultAdditionalFinalDamage BY HITTER  ( BLOW_185 )    ]               ++

			if( pHitterStateBlow && pHitterStateBlow->IsApplied( STATE_BLOW::BLOW_185 ) )
			{
				DNVector(DnBlowHandle) vlhFinalPDamageBlows;
				pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_185, vlhFinalPDamageBlows );
				int iNumBlow = (int)vlhFinalPDamageBlows.size();
				for( int i = 0; i < iNumBlow; ++i )
				{
					DnBlowHandle hBlow = vlhFinalPDamageBlows.at( i );
					fResultAdditionalFinalDamage += hBlow->GetFloatValue();
				}
			}

#pragma endregion

		}

		float fGausianResult = (float)GetGaussianRandom( nAttack[0], nAttack[1], GetRoom() );
		fGausianResultProb = fGausianResult / nAttack[1];
		fAttackPower = fGausianResult * fHitPer;

		float fDamageRatioBlowValue = 0.f;

#pragma region ++         [   fDamageRatioBlowValue BY HITTER  ( BLOW_050 )    ]               ++
		
		if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_050))
		{
			DNVector(DnBlowHandle) vlhDamageRatioBlows;
			pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_050, vlhDamageRatioBlows );
			int iNumBlow = (int)vlhDamageRatioBlows.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				DnBlowHandle hBlow = vlhDamageRatioBlows.at( i );
				fDamageRatioBlowValue += hBlow->GetFloatValue();
			}
		}

#pragma endregion


#if defined(PRE_FIX_65287)
		//If FinalDamageRate is set by the projectile, use that configured value.
		//After the skill ends, state effect 50 is gone, so this setting value must be used.
		float fShooterFinalDamageRate = 0.0f;
		if (HitParam.bFromProjectile)
		{
			CDnProjectile* pProjectile = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() );

			fShooterFinalDamageRate = pProjectile ? pProjectile->GetShooterFinalDamageRate() : 0.0f;

			if (fShooterFinalDamageRate != 0.0f)
				fDamageRatioBlowValue = fShooterFinalDamageRate;
		}
#endif // PRE_FIX_65287
		

		fAttackPower = fAttackPower * ( 1.f + fResultFinalDamage + fDamageRatioBlowValue + fResultAdditionalFinalDamage );

#pragma region ++         [   fAttackPower BY HITTER  ( BLOW_074 )    ]               ++
		// State effect 74. Always calculated at max damage.
		if( pHitterStateBlow && pHitterStateBlow->IsApplied( STATE_BLOW::BLOW_074 ) ) 
			fAttackPower = (float)nAttack[1];
#pragma endregion


#pragma region ++         [   fAttackPower BY HITTER  ( BLOW_209 )    ]               ++		
		// New character (Academic-related)
		// Increase base attack power ratio..(only for normal attacks....) // When not using a skill, and either not a projectile or, if a projectile, not a skill-used projectile 
		// Currently configured to be usable only for Academic; please test edge cases more when applying to other classes 
		// < Ex> Exception case identified while stance skill is active, currently unhandled > 
		if (false == hHitter->IsProcessSkill() && (( HitParam.bFromProjectile && !HitParam.bFromProjectileSkill ) || !HitParam.bFromProjectile ) && 
			!hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_121) && !hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_129) //Hacking Stance type)
			)
		{
			if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_209) )
			{
				DNVector(DnBlowHandle) vlBlowList;
				float fAddtionalAttackPower = 0.f;
				pHitterStateBlow->GetStateBlowFromBlowIndex(STATE_BLOW::BLOW_209, vlBlowList);

				for (int i=0 ; i< (int)vlBlowList.size(); ++i )
				{
					if(vlBlowList[i]->GetFloatValue() > 0.f)
					{
						fAddtionalAttackPower = fAttackPower * vlBlowList[i]->GetFloatValue(); 
						break;
					}
				}

				fAttackPower += fAddtionalAttackPower;
			}
		}
#pragma endregion

		if( fAttackPower < 0.f ) 
			fAttackPower = 0.f;
	}


#pragma region ++         [   nDefense BY SELF  ( BLOW_101 ~ BLOW_108 )    ]               ++		
	// [OBT] Implement additional skill effect - Defense modification (#7839) ///////////////////////////////////////
	STATE_BLOW::emBLOW_INDEX AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_NONE;
	switch( HitParam.DistanceType )
	{
		case CDnDamageBase::DistanceTypeEnum::Melee:
			{	
				if( 0 == HitParam.cAttackType )
				{
					// Modify physical melee defense absolute : 101
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_101 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_101;
					else
					// Modify physical melee defense ratio
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_102 ) )						
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_102;
				}
				else
				if( 1 == HitParam.cAttackType )
				{
					// Modify magic melee defense absolute value
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_105 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_105;
					else
					// Modify magic melee defense ratio
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_106 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_106;
				}
			}
			break;

		case CDnDamageBase::DistanceTypeEnum::Range:
			{
				if( 0 == HitParam.cAttackType )
				{
					// Modify physical ranged defense absolute : 101
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_103 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_103;
					else
					// Modify physical ranged defense ratio
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_104 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_104;
				}
				else
				if( 1 == HitParam.cAttackType )
				{
					// Modify magic range defense absolute value
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_107 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_107;
					else
					// Modify magic range defense ratio
					if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_108 ) )
						AppliedDefenseChangeBlowIndex = STATE_BLOW::BLOW_108;
				}
			}
			break;
	}

	if( STATE_BLOW::BLOW_NONE != AppliedDefenseChangeBlowIndex )
	{
		switch( AppliedDefenseChangeBlowIndex )
		{
			// Change absolute value
			case STATE_BLOW::BLOW_101: 
			case STATE_BLOW::BLOW_103:
			case STATE_BLOW::BLOW_105:
			case STATE_BLOW::BLOW_107:
				{
					DNVector(DnBlowHandle) vlhDefenseAbChangeBlow;
					GetStateBlow()->GetStateBlowFromBlowIndex( AppliedDefenseChangeBlowIndex, vlhDefenseAbChangeBlow );
					int iNumBlow = (int)vlhDefenseAbChangeBlow.size();
					for( int i = 0; i < iNumBlow; ++i )
					{
						DnBlowHandle hBlow = vlhDefenseAbChangeBlow.at( i );
						int iValue = (int)hBlow->GetFloatValue();
						nDefense += iValue;
					}
				}
				break;

			// Change ratio
			case STATE_BLOW::BLOW_102:
			case STATE_BLOW::BLOW_104:
			case STATE_BLOW::BLOW_106:
			case STATE_BLOW::BLOW_108:
				{
					DNVector(DnBlowHandle) vlhDefenseRtChangeBlow;
					GetStateBlow()->GetStateBlowFromBlowIndex( AppliedDefenseChangeBlowIndex, vlhDefenseRtChangeBlow );
					float fChangeRatio = 1.0f;		// Considering multiple instances, accumulate only the value exceeding 100%.
					int iNumBlow = (int)vlhDefenseRtChangeBlow.size();
					for( int i = 0; i < iNumBlow; ++i )
					{
						DnBlowHandle hBlow = vlhDefenseRtChangeBlow.at( i );
						fChangeRatio += (hBlow->GetFloatValue()-1.0f);
					}

					nDefense += int( (float)nDefense*fChangeRatio );
				}
				break;
		}
	}
#pragma endregion


	float fDefensePower = 1.f;

#if defined(PRE_ADD_SKILLBUF_RENEW)

	//Defense is applied to all attack power calculations.

	fDefenseWeight = hHitter->GetDefenseConstant();

	fDefensePower = nDefense / fDefenseWeight;
	if( fDefensePower < 0.f ) fDefensePower = 0.f;
	else if( fDefensePower > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax ) ) 
		fDefensePower = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax );

	fDefensePower = 1.f - fDefensePower;

	//If critical, apply configured constant value to AttackPower.
	if (HitParam.HitType == CDnWeapon::Critical)
	{
		//#57510 Critical damage 1.5x in PvP
		float fCriticalIncValue = 1.0f;

		if( GetGameRoom()->bIsPvPRoom() == true )
			fCriticalIncValue = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalIncrease_PVP );
		else
			fCriticalIncValue = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalIncrease );

		fAttackPower = fAttackPower * fCriticalIncValue;
	}

#else // PRE_ADD_SKILLBUF_RENEW

	if( HitParam.HitType == CDnWeapon::Critical ) 
	{
		fDefensePower = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalIncrease );
	}
	else 
	{
		fDefenseWeight = hHitter->GetDefenseConstant();

		fDefensePower = nDefense / fDefenseWeight;
		if( fDefensePower < 0.f ) fDefensePower = 0.f;
		else if( fDefensePower > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax ) ) 
			fDefensePower = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DefenseMax );

		fDefensePower = 1.f - fDefensePower;
	}

#endif // PRE_ADD_SKILLBUF_RENEW


	fResult = fAttackPower * fDefensePower;

#pragma region ++         [   fResult *= fChangeRatio BY SELF  ( BLOW_134, BLOW_135 )    ]               ++

	// Modify physical/magic damage #12354, #12353 ///////////////////////////////////////////
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_134 ) && 0 == HitParam.cAttackType )
	{

		DNVector( DnBlowHandle ) vlhBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_134, vlhBlows );
		
		float fChangeRatio = 1.0f;		// Considering multiple instances, accumulate only the value exceeding 100%.
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		float fBuffStateRatioValue = 1.f;
#endif
		for( int i = 0; i < (int)vlhBlows.size(); ++i )
		{
			DnBlowHandle hBlow = vlhBlows.at( i );
			float fValue = hBlow->GetFloatValue();
			fChangeRatio += (fValue-1.0f);
#ifdef PRE_ADD_BUFF_STATE_LIMIT
			if( hBlow->GetAddBlowStateType() == CDnActorState::AddBlowStateType::Equip_Buff_Level )
				fBuffStateRatioValue += (fValue-1.0f);
#endif
		}


#ifdef PRE_ADD_BUFF_STATE_LIMIT
		float fMinLimit = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Change_DamageTaken_Physical_Min );
		float fMaxLimit = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Change_DamageTaken_Physical_Max );
		
		if(IsPlayerActor())
		{
			if( fChangeRatio < fMinLimit ) //#77817 When damage is taken by a user, configure based on total damage reduction.
				fChangeRatio = fMinLimit;
		}
		else if(IsMonsterActor())
		{
			if( fBuffStateRatioValue > fMaxLimit )
				fChangeRatio += -fBuffStateRatioValue + fMaxLimit;
		}
#else
		float fDamageChangeClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DamageChangeClampLowestRatio );
		if( fChangeRatio < fDamageChangeClampLowestRatio )
			fChangeRatio = fDamageChangeClampLowestRatio;
#endif

		if(fChangeRatio < 0) 
			fChangeRatio = 0.f;

		fResult *= fChangeRatio;


	}
	else if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_135 ) && 1 == HitParam.cAttackType )
	{

		DNVector( DnBlowHandle ) vlhBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_135, vlhBlows );
		
		float fChangeRatio = 1.f;
#ifdef PRE_ADD_BUFF_STATE_LIMIT
		float fBuffStateRatioValue = 1.f;

#endif
		for( int i = 0; i < (int)vlhBlows.size(); ++i )
		{
			DnBlowHandle hBlow = vlhBlows.at( i );
			float fValue = hBlow->GetFloatValue();
			fChangeRatio += (fValue-1.0f);

#ifdef PRE_ADD_BUFF_STATE_LIMIT
			if( hBlow->GetAddBlowStateType() == CDnActorState::AddBlowStateType::Equip_Buff_Level )
				fBuffStateRatioValue += (fValue-1.0f);
#endif
		}

#ifdef PRE_ADD_BUFF_STATE_LIMIT
		float fMinLimit = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Change_DamageTaken_Magical_Min );
		float fMaxLimit = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::Change_DamageTaken_Magical_Max );

		if(IsPlayerActor())
		{
			if( fChangeRatio < fMinLimit ) //#77817 When damage is taken by a user, configure based on total damage reduction.
				fChangeRatio = fMinLimit;
		}
		else if(IsMonsterActor())
		{
			if( fBuffStateRatioValue > fMaxLimit )
				fChangeRatio += -fBuffStateRatioValue + fMaxLimit;
		}
#else
		float fDamageChangeClampLowestRatio = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::DamageChangeClampLowestRatio );
		if( fChangeRatio < fDamageChangeClampLowestRatio )
			fChangeRatio = fDamageChangeClampLowestRatio;
#endif


		if(fChangeRatio < 0) 
			fChangeRatio = 0.f;

		fResult *= fChangeRatio;

	}

#pragma endregion


#pragma region ++         [   fResult += fResult * fChangeRatio BY SELF  ( BLOW_268 )    ]               ++
#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	if (IsAppliedThisStateBlow( STATE_BLOW::BLOW_268 ))
	{
		DNVector( DnBlowHandle ) vlhBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_268, vlhBlows );

		float fChangeRatio = 0.0f;
		for( int i = 0; i < (int)vlhBlows.size(); ++i )
		{
			DnBlowHandle hBlow = vlhBlows.at( i );
			if (hBlow && hBlow->IsEnd() == false)
			{
				CDnDamageChagneBlow* pDamageChageBlow = static_cast<CDnDamageChagneBlow*>(hBlow.GetPointer());
				if (pDamageChageBlow && pDamageChageBlow->IsActivated() == true && pDamageChageBlow->GetLeftCoolTime() > 0.0f)
				{
					fChangeRatio += pDamageChageBlow->GetDamageRate();
				}
			}
		}

		if (fChangeRatio >= 0.0f)
		{
			fResult += fResult * fChangeRatio;
		}
	}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL
#pragma endregion


	if( HitParam.HasElement != CDnActorState::ElementEnum_Amount ) 
	{
		fElementWeight = 1.f;
		fElementWeight = ( ( 1.f + pState->GetElementAttack( HitParam.HasElement ) ) * ( 1.f - GetElementDefense( HitParam.HasElement ) ) );
#ifndef PRE_ADD_BUFF_STATE_LIMIT
		fElementWeight = max( fElementWeight, CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::ElementMinRevision ) );
#endif
		fResult *= fElementWeight;
	}

	// Calculate critical resist and reduce damage (Breaking Point (Super Armor Break) must not be set).
	if( HitParam.HitType == CDnWeapon::Critical && false == HitParam.bBreakSuperAmmor) 
	{
		float fCriticalValue = hHitter->GetCriticalConstant();
		float fCriticalResistProb = GetCriticalResistance() / fCriticalValue;
		if( fCriticalResistProb > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalResistanceMax ) )
			fCriticalResistProb = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalResistanceMax );
		if( _rand(GetRoom())%100 < (int)( fCriticalResistProb * 100.f ) ) {
#if defined(PRE_FIX_51491)
			//Remove added damage caused by critical.
			//#57510 Critical damage 1.5x in PvP

			float fCriticalIncValue = 1.0f;
			if( GetGameRoom()->bIsPvPRoom() == true )
				fCriticalIncValue = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalIncrease_PVP );
			else
				fCriticalIncValue = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalIncrease );

			fResult = fResult / fCriticalIncValue;
#else
			fResult *= CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalResistance );
#endif // PRE_FIX_51491

			m_HitParam.HitType = CDnWeapon::CriticalRes;
		}
	}

#pragma region ++         [   pComboDamageLimitBlow BY SELF  ( BLOW_242 )    ]               ++
	// #52332 Implement damage reduction effect after a certain number of hits
	// If state effect 242 is applied, update Hit count limit using the skill/actor used in the current damage calculation..
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_242 ) )
	{
		CDnSkill::SkillInfo *pSkillInfo = NULL;
		DnSkillHandle hSkill;

		if (HitParam.bFromProjectile && HitParam.hWeapon)
			hSkill = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() )->GetParentSkill();
		else
			hSkill = hHitter->GetProcessSkill();
		
		if (hSkill)
			pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hSkill->GetInfo());
		
		DNVector(DnBlowHandle) vlBlows;
		m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_242, vlBlows );
		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
			{
				CDnComboDamageLimitBlow* pComboDamageLimitBlow = dynamic_cast<CDnComboDamageLimitBlow*>(hBlow.GetPointer());
				if (pComboDamageLimitBlow)
					pComboDamageLimitBlow->UpdateHitCount(pSkillInfo, HitParam);
			}
		}
	}
#pragma endregion

	// Additional damage can be dealt from the state effect. 
	fResult += m_pStateBlow->OnCalcDamage( fResult, m_HitParam );

	// Extended attacker damage bonus blows (BLOW_292 weapon-based, BLOW_293 distance, BLOW_294 weight)
	if( hHitter && fResult > 0.0f )
	{
		if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_292 ) )
		{
			DNVector(DnBlowHandle) vlBlows292;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_292, vlBlows292 );
			float fWeaponAtk = ( HitParam.cAttackType == 1 ) ? (float)pState->GetAttackMMax() : (float)pState->GetAttackPMax();
			for( DWORD bi = 0; bi < vlBlows292.size(); ++bi )
			{
				if( vlBlows292[bi] )
				{
					CDnAddWeaponBasedDamageBlow* pWBlow = static_cast<CDnAddWeaponBasedDamageBlow*>( vlBlows292[bi].GetPointer() );
					fResult += fWeaponAtk * pWBlow->GetDamageRatio();
				}
			}
		}
		if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_293 ) )
		{
			DNVector(DnBlowHandle) vlBlows293;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_293, vlBlows293 );
			EtVector3 vDist = *GetPosition() - *hHitter->GetPosition();
			float fDist = EtVec3Length( &vDist );
			float fDistBonus = 0.0f;
			for( DWORD bi = 0; bi < vlBlows293.size(); ++bi )
			{
				if( vlBlows293[bi] )
					fDistBonus += static_cast<CDnAddDamageByTargetDistanceBlow*>( vlBlows293[bi].GetPointer() )->CalcDistanceDamageRatio( fDist );
			}
			if( fDistBonus > 0.0f )
				fResult += fResult * fDistBonus;
		}
		if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_294 ) )
		{
			DNVector(DnBlowHandle) vlBlows294;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_294, vlBlows294 );
			float fWeightBonus = 0.0f;
			for( DWORD bi = 0; bi < vlBlows294.size(); ++bi )
			{
				if( vlBlows294[bi] )
					fWeightBonus += static_cast<CDnAddDamageByWeightBlow*>( vlBlows294[bi].GetPointer() )->CalcWeightDamageRatio( GetWeight() );
			}
			if( fWeightBonus > 0.0f )
				fResult += fResult * fWeightBonus;
		}
	}

#if defined(PRE_FIX_59680)
	//When calling Summon Puppet's OnCalcDamage, the owner actor's StateBlow OnCalcDamage must also be called so that effects apply properly when damage is applied to the owner actor.
	//For Cleric's Holy Shield, CDnAddStateOnHitBlow(178) state effect is applied to self, and if Summon Puppet is summoned afterward, Summon Puppet... this state effect
	//does not have it. So damage is applied to the Cleric via Summon Puppet, but the HP-increase-on-damage state effect does not trigger.
	if (IsMonsterActor())
	{
		DnActorHandle hSummonerActor;
		CDnStateBlow *pOwnerStateBlow = NULL;

		CDnMonsterActor* pMonsterActor = dynamic_cast<CDnMonsterActor*>(m_MySmartPtr.GetPointer());
		if (pMonsterActor && pMonsterActor->IsSummonedMonster() && pMonsterActor->IsPuppetSummonMonster())
			hSummonerActor = pMonsterActor->GetSummonerPlayerActor();

		if(hSummonerActor)
			pOwnerStateBlow = hSummonerActor->GetStateBlow();

		if (pOwnerStateBlow)
			pOwnerStateBlow->OnCalcDamage(fResult, m_HitParam);
	}
#endif // PRE_FIX_59680

	// For checking damage on OnTargetHit [2011/02/15 semozz]
	m_HitParam.nCalcDamage = (int)fResult;
	hHitter->GetStateBlow()->OnTargetHit( GetMySmartPtr() );

#pragma region ++         [   fResult -= fAbsorbDamage BY SELF  ( BLOW_243 )    ]               ++
	if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_243 ) )
	{
		float fAbsorbDamage = 0.f;
		int nAbsorbSP = 0;

		DNVector( DnBlowHandle ) vlhManaShieldBlow;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_243, vlhManaShieldBlow );
		for( DWORD n=0; n<vlhManaShieldBlow.size(); n++ )
		{
			CDnManaShieldBlow* pManShieldBlow = static_cast<CDnManaShieldBlow*>(vlhManaShieldBlow[n].GetPointer());
			pManShieldBlow->CalcManaShield( fResult , fAbsorbDamage , nAbsorbSP );
		}

		if( nAbsorbSP > GetSP() )
		{
			fAbsorbDamage -= (fAbsorbDamage * (nAbsorbSP - GetSP()) / nAbsorbSP);
			nAbsorbSP = GetSP();
		}

		if( fAbsorbDamage > fResult )
			fAbsorbDamage = fResult;

		fResult -= fAbsorbDamage;

		SetSP( GetSP() -nAbsorbSP );
		RequestHPMPDelta( ElementEnum::ElementEnum_Amount, -nAbsorbSP, UINT_MAX, true , true );
	}
#pragma endregion

#pragma region ++         [   fResult = 0.0f; BY SELF  ( BLOW_051 )    ]               ++
#if defined(PRE_FIX_61382)
	if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_051))
	{
		DNVector(DnBlowHandle) vlhTransmitBlows;
		GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_051, vlhTransmitBlows );

		int iNumBlow = (int)vlhTransmitBlows.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			CDnTransmitDamageBlow* pTransmitDamageBlow = static_cast<CDnTransmitDamageBlow*>(vlhTransmitBlows[i].GetPointer());
			
			//Check if the damage-receiving actor can be hit, and transfer damage only when hittable.
			DnActorHandle hTargetActor = pTransmitDamageBlow->GetActorHandle();
			
			bool isEnableTransmitDamage = false;

			if (hTargetActor)
			{
				if (hTargetActor->IsMonsterActor())
				{
					CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hTargetActor.GetPointer());
					isEnableTransmitDamage = pMonsterActor ? pMonsterActor->IsEnableNoDamage() == false : true;
				}
				else
					isEnableTransmitDamage = true;				
			}

			//If damage transfer is possible, transfer the damage.
			if (isEnableTransmitDamage == true)
				pTransmitDamageBlow->TransmitDamage( fResult , m_HitParam );
		}

		//Reset damage to 0 after transferring damage..
		fResult = 0.0f;
	}
#endif // PRE_FIX_61382
#pragma endregion

#pragma region ++         [  STAGE DAMAGE LIMIT  ]               ++
#ifdef PRE_ADD_STAGE_DAMAGE_LIMIT
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );
	if( pTask && pTask->GetStageDamageLimit()->fDamageLimit > 0.f )
	{
		if( hHitter )
		{
			CDnPlayerActor *pPlayerActor = NULL;

			if( hHitter->IsPlayerActor() )
			{
				pPlayerActor = static_cast<CDnPlayerActor*>(hHitter.GetPointer());
			}
			else if( hHitter->IsMonsterActor() )
			{
				CDnMonsterActor *pMonsterActor = static_cast<CDnMonsterActor*>(hHitter.GetPointer());
				if( pMonsterActor && pMonsterActor->GetSummonerPlayerActor() && pMonsterActor->GetSummonerPlayerActor()->IsPlayerActor() )
					pPlayerActor = static_cast<CDnPlayerActor*>(pMonsterActor->GetSummonerPlayerActor().GetPointer());
			}

			if( pPlayerActor )
			{
				bool bNotifyInterface = false;

				float fBaseAttackPower = 0.f;
				if( HitParam.cAttackType == 0)
				{
					fBaseAttackPower = pState->GetAttackPMax() * fGausianResultProb * fHitPer;
					bNotifyInterface = pPlayerActor->GetDamageLimitInfo()->bPhysicalAttack;
				}
				else if (HitParam.cAttackType == 1)
				{
					fBaseAttackPower = pState->GetAttackMMax() * fGausianResultProb * fHitPer;
					bNotifyInterface = pPlayerActor->GetDamageLimitInfo()->bMagicalAttack;
				}

				if( fResult > (fBaseAttackPower * pTask->GetStageDamageLimit()->fDamageLimit) )
				{
					fResult = (fBaseAttackPower * pTask->GetStageDamageLimit()->fDamageLimit);
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
					if( bNotifyInterface == true )
						HitParam.bStageLimit = true;
#endif
				}
			}
		}
	}
#endif
#pragma endregion


	if( fResult < 0.f ) 
		fResult = 0.f;

	return fResult;
}


float CDnActor::CalcDamage( CDnDamageBase *pHitter, SHitParam &HitParam )
{
	float fResult = PreCalcDamage( pHitter, HitParam );

#if defined( NODAMAGE ) || defined( STRESS_TEST )
	return fResult;
#endif

#if defined(PRE_ADD_MISSION_COUPON)
	INT64 nSaveHP = m_nHP;
	SetHP((INT64)( m_nHP - fResult ));
	m_nHP = (INT64)( nSaveHP - fResult );
#else
	m_nHP = (INT64)( m_nHP - fResult );
#endif

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
	if (IsAppliedThisStateBlow( STATE_BLOW::BLOW_268 ))
	{
		float fHpRate = 1.0f;
		if (m_nMaxHP > 0)
			fHpRate = (float)m_nHP / (float)m_nMaxHP;

		DNVector( DnBlowHandle ) vlhBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_268, vlhBlows );

		float fChangeRatio = 0.0f;
		for( int i = 0; i < (int)vlhBlows.size(); ++i )
		{
			DnBlowHandle hBlow = vlhBlows.at( i );
			if (hBlow && hBlow->IsEnd() == false)
			{
				CDnDamageChagneBlow* pDamageChageBlow = static_cast<CDnDamageChagneBlow*>(hBlow.GetPointer());
				if (pDamageChageBlow)
				{
					float fHPLimit = pDamageChageBlow->GetHpLimit();

					if (fHpRate < fHPLimit &&
						pDamageChageBlow->IsActivated() == false &&
						pDamageChageBlow->GetLeftCoolTime() == 0.0f)
					{
						pDamageChageBlow->ApplyDamageChange();
					}
				}
			}
		}
	}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

	return fResult;
}

void CDnActor::Die( DnActorHandle hHitter )
{
	// Monster last-hit also needs CP score processing, so substitute it here.
	if( hHitter && hHitter->IsMonsterActor() && IsMonsterActor() )
	{
		DnActorHandle hSummonMasterPlayerActor = static_cast<CDnMonsterActor*>(hHitter.GetPointer())->GetSummonerPlayerActor();
		if( hSummonMasterPlayerActor )
			hHitter = hSummonMasterPlayerActor;
	}

#if defined( PRE_ADD_58761 )
	// Leave log on death	
	CDNGameRoom *pRoom = GetGameRoom();	
	if( pRoom && CDnWorld::GetInstance(pRoom).GetMapSubType() == EWorldEnum::MapSubTypeNest)
	{
		if( hHitter && hHitter->IsMonsterActor() && IsPlayerActor() )
		{
			DnMonsterActorHandle hMonster = hHitter;
			int nSkillID = 0;
			if(hMonster->GetProcessSkill())
				nSkillID = hMonster->GetProcessSkill()->GetClassID();
			CDNUserSession *pSession = pRoom->GetUserSession( GetSessionID() );
			if (pSession)
				pRoom->NestDeathLog( pSession, hMonster->GetMonsterClassID(), nSkillID, pSession->GetUserJob(), pSession->GetLevel());
			else
				_DANGER_POINT();
		}
	}
#endif
#if defined(PRE_FIX_44884)
	bool bStateBlowProcessed = true;

	if (IsDie())
		bStateBlowProcessed = m_pStateBlow->OnDie( hHitter );

	if (IsDie())
	{
		if( IsMonsterActor() )
		{
			ResetPlaySpeed();
			if( m_iFrameStopRefCount > 0 )
				m_iFrameStopRefCount = 0;
			UpdateFPS();
		}
		OnDie(hHitter);
	}
#else
	OnDie( hHitter );
	bool bStateBlowProcessed = m_pStateBlow->OnDie( hHitter );
#endif // PRE_FIX_44884

	bool bCheckDie = false;
	
	if( false == bStateBlowProcessed )
	{
		if( IsStandHit( m_HitParam.szActionName.c_str() ) )
			m_HitParam.szActionName = "Die", bCheckDie = true;
		if( IsStun( m_HitParam.szActionName.c_str() ) && !IsAir( m_HitParam.szActionName.c_str() ) ) 
			m_HitParam.szActionName = "Die", bCheckDie = true;

		if( !bCheckDie ) {
			// Include Stay condition due to hold skills.. otherwise Die action won't trigger and if killed in one hit while held, client switches to "stand" action 
			// Dies standing up upon receiving OnDamage packet..
			if( IsStay( m_HitParam.szActionName.c_str() ) )
				m_HitParam.szActionName = "Die", bCheckDie = true;
			
			if( IsAir( m_HitParam.szActionName.c_str() ) ) {
				if( IsExistAction( "Die_Air" ) ) m_HitParam.szActionName = "Die_Air", bCheckDie = true;
			}
			if( IsDown( m_HitParam.szActionName.c_str() ) ) {
				if( IsExistAction( "Die_Down" ) ) m_HitParam.szActionName = "Die_Down", bCheckDie = true;
			}

			if( !bCheckDie ) m_HitParam.szActionName = "Die";
		}
	}
	else
	{
		// Due to skills used by monsters while dying, make it stand instead of Die action. (#16331)
		m_HitParam.szActionName = "Stand";
	}

	m_bEnableNormalSuperAmmor = false;

	if (hHitter)
	{
		if (hHitter->IsMonsterActor())
		{
			CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(hHitter.GetPointer());
			if (pMonster && pMonster->IsCannonMonsterActor())
			{
				CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>(hHitter.GetPointer());
				if (pCannonMonster && pCannonMonster->GetMasterPlayerActor() && pCannonMonster->GetMasterPlayerActor()->IsPlayerActor())
				{
					CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(pCannonMonster->GetMasterPlayerActor().GetPointer());
					if( pPlayerActor )
						pPlayerActor->OnKillMonster(GetMySmartPtr());
				}
			}
		}
		else if (hHitter->IsPlayerActor())
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hHitter.GetPointer());
			if( pPlayerActor )
				pPlayerActor->OnKillMonster(GetMySmartPtr());
		}
	}	

	// Process monster death for each party member.
	// NOTE: Closing client at the moment of dying from poison crashes the server. pTask has garbage value..
	CDnGameTask *pTask = (CDnGameTask *)CTaskManager::GetInstance(GetRoom()).GetTask( "GameTask" );

	if ( pTask )
	{
		for( DWORD i=0; i< pTask->GetRoom()->GetUserCount(); i++ ) 
		{
			CDNGameRoom::PartyStruct *pStruct = pTask->GetRoom()->GetPartyData(i);
			if (!pStruct) 
				continue;
			if( !IsMonsterActor() )
				continue;
			CDnMonsterActor* pMonster = static_cast<CDnMonsterActor*>(this);
			if( pStruct->pSession && pStruct->pSession->GetQuest() )
				pStruct->pSession->GetQuest()->OnDieMonster(pMonster->GetMonsterClassID());
		}
	}
}

void CDnActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{
	//At this point, it has already passed the IsHitable check.
	//Remove state effect before damage processing..
	RemoveStateEffectByHitSignal(pHitStruct);

	DnActorHandle hHitterActor = pHitter->GetActorHandle();

	if( hHitterActor && hHitterActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) == false )
	{
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
		hHitterActor->ApplyPrefixOffenceSkillToTarget_New(GetActorHandle());
#else
		hHitterActor->ApplyPrefixOffenceSkillToTarget(GetActorHandle());
#endif
	}

	m_HitParam = HitParam;
	DnActorHandle hHitter = pHitter->GetActorHandle();
	CDnStateBlow *pHitterStateBlow = hHitter->GetStateBlow();

#if defined(PRE_ADD_PROJECTILE_SE_INFO)	
	if( hHitter && HitParam.bFromProjectile && HitParam.hWeapon )
	{
		CDnStateBlow *pProjectileStateBlow = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() )->GetShooterStateBlow().get();
		if( pProjectileStateBlow )
			pHitterStateBlow = pProjectileStateBlow;
	}
#endif

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
	if( CDnDPSReporter::IsActive() && hHitter && hHitter->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>( hHitter.GetPointer());

		if(CDnDPSReporter::GetInstance().IsEnabledUser(pPlayerActor->GetCharacterDBID()))
			CDnDPSReporter::GetInstance().StartReport();
	}
	float fDPS_DAMAGE = 0;
#endif

	CDnWeapon::HitTypeEnum HitType = CDnWeapon::Normal;
	// Calculate hit rate
	SetDamageRemainTime( hHitter->GetUniqueID(), HitParam.RemainTime , hHitter->GetCurrentActionIndex() );
	SetLastDamageHitUniqueID( hHitter->GetUniqueID(), HitParam.iUniqueID );
	m_fStiffDelta = 0.f;
	if( m_fDownDelta > 0.f ) m_fLastDownRatio *= 0.5f;
	m_fDownDelta = 0.f;

	bool bAir = false;
	if( IsAir() && IsHit() ) bAir = true;

	bool bHitSuccess = CheckDamageAction( hHitter );


	// Change physical attack to magic attack.. [2011/02/21 semozz]
	if (hHitter && hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_180))
	{
		if (0 == m_HitParam.cAttackType)
		{
			//If BLOW_180 is applied to the attacker, convert all physical attacks to magic attacks.
			m_HitParam.cAttackType = 1;
		}
	}

	if( pHitterStateBlow && pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_182) )
	{
		//Change only when elemental attack is not set??
		if( m_HitParam.HasElement == CDnActorState::ElementEnum_Amount ) 
		{
			DNVector(DnBlowHandle) vlChangeElementBlows;
			pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_182, vlChangeElementBlows );
			CDnState::ElementEnum eElementType = CDnState::ElementEnum_Amount;
			if (!vlChangeElementBlows.empty())
			{
				eElementType = CDnState::ElementEnum(int(vlChangeElementBlows.at(0)->GetFloatValue()));
			}

			if (eElementType != CDnActorState::ElementEnum_Amount)
				m_HitParam.HasElement = eElementType;
		}
	}

	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_149 ) == false  ) // When affected by Ice Prison state effect, do not process parry or block state effects via OnDefenseAttack.
	{
		CDnState* pAttackerState = NULL;
		if( hHitter && !HitParam.bFromProjectile )
		{ 
			pAttackerState = static_cast<CDnActorState *>(hHitter.GetPointer());
		}
		else
		{
			if( HitParam.hWeapon )
				pAttackerState = static_cast<CDnProjectile*>( HitParam.hWeapon.GetPointer() )->GetShooterStateSnapshot().get();

#if !defined(PRE_ADD_PROJECTILE_SE_INFO)
			if (hHitter&& hHitter->IsApplyPrefixOffenceSkill())
				pAttackerState = static_cast<CDnActorState *>(hHitter.GetPointer());
#endif

			if( hHitter && pAttackerState == NULL )
				pAttackerState = static_cast<CDnActorState *>(hHitter.GetPointer());
		}
		
#ifdef PRE_ADD_MONSTER_CATCH
		bool bCheckOnDefense = true;
		if( IsPlayerActor() )
		{
			// Ignore skill-related processing while grabbed by a monster.
			if( static_cast<CDnPlayerActor*>(this)->IsCatchedByMonster() || m_HitParam.bReleaseCatchActor )
				bCheckOnDefense = false;
		}

		// Using state from the attacker hActor may reflect normal attack/defense since the skill action might already be finished, so 
		// Use pAttackerState which also accounts for being carried by a projectile. 
		// If pAttackerState is used inside the function, it must be NULL-checked internally.
		if( bCheckOnDefense )
			bHitSuccess = m_pStateBlow->OnDefenseAttack( hHitter, pAttackerState, m_HitParam, bHitSuccess );
#else
		bHitSuccess = m_pStateBlow->OnDefenseAttack( hHitter, pAttackerState, m_HitParam, bHitSuccess );
#endif 

	}

	// Due to damage reflection state effect, if hitter is a monster, it may be processed as Die and destroyed.
	// For players, the object is not destroyed, so CheckDamageVelocity() is called appropriately
	// Process so that the hit action can play. #23559
	// If a monster dies from damage reflection right after hitting, velocity may not be set properly and cause the same issue, though 
	// Handle only the player side for now.
	if( hHitter->IsMonsterActor() )
	{
		if( !hHitter || hHitter->IsDie() )
			return;
	}

	bool isBreakSuperArmorBlow = false;

	if( bHitSuccess ) {
		// Calculate stagger duration
		float fStiffResult = 0.f;
		if( GetStiffResistance() > 0 ) fStiffResult = ( hHitter->GetStiff() * m_HitParam.fStiffProb ) / GetStiffResistance();
		fStiffResult *= CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffRevision );

		if( fStiffResult < 0.001f )
			fStiffResult = 0.001f; // Since CPacketCompressStream::FLOAT_SHORT is used, ensure value does not become small enough to go out of range.

		if( fStiffResult <= 0.f )
		{
			fStiffResult = 0.05f;
		}
		else
		{
			float fStiffMax = 0.f; 
			
#if defined PRE_ADD_PVP_STIFF_MAX
			if( GetGameRoom()->bIsPvPRoom() == true )
				fStiffMax = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffMax_PVP );
			else
				fStiffMax = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffMax );
#else
			fStiffMax = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffMax );
#endif
			if( fStiffResult > fStiffMax ) 
				fStiffResult = fStiffMax;
		}

		m_fStiffDelta = s_fMaxStiffTime * fStiffResult;

		// Set random down delay value
		if( !IsDown() ) {
			m_fLastDownRatio = 1.f;
		}

		float fTemp = ( 3.f * ( 1.f - GetDownDelayProb() ) ) * m_fLastDownRatio;
		int nTemp = (int)( fTemp * DOWN_DELAY_RANDOM_RANGE * 100.f );
		if( nTemp == 0 ) m_fDownDelta = fTemp;
		else m_fDownDelta = fTemp - DOWN_DELAY_RANDOM_RANGE + ( _rand(GetRoom())%( nTemp * 2 ) / 100.f );
		if( m_fDownDelta >= 3.f ) m_fDownDelta = 3.f;
		if( m_fDownDelta <= 0.01f ) m_fDownDelta = 0.01f;

		// Calculate critical
		float fCritical = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalValue );
		float fCriticalValue = hHitter->GetLevelWeightValue() * fCritical;
		float fCriticalProb = 0.f;
		if( fCriticalValue > 0.f )
			fCriticalProb = hHitter->GetCritical() / fCriticalValue;
		else
			g_Log.Log(LogType::_SESSIONCRASH, L"fCriticalValue = 0.f ClassID=%d\n", hHitter->GetClassID() );


		float fAddCiriticalValue = 0.0f;

		if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_250))
		{
			DNVector( DnBlowHandle ) vlhCriticalIncBlows;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_250, vlhCriticalIncBlows );
			int nListCount = (int)vlhCriticalIncBlows.size();
			for (int i = 0; i < nListCount; ++i)
			{
				DnBlowHandle hBlow = vlhCriticalIncBlows[i];
				if (hBlow)
					fAddCiriticalValue += hBlow->GetFloatValue();
			}
		}


		if( pHitterStateBlow )
		{
			if( pHitterStateBlow->IsApplied(STATE_BLOW::BLOW_251) )
			{
				DNVector(DnBlowHandle) vlhCriticalIncBlows;
				pHitterStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_251, vlhCriticalIncBlows );
				int nListCount = (int)vlhCriticalIncBlows.size();
				for (int i = 0; i < nListCount; ++i)
				{
					DnBlowHandle hBlow = vlhCriticalIncBlows[i];
					if (hBlow)
						fAddCiriticalValue += hBlow->GetFloatValue();
				}
			}
		}

		fCriticalProb += fAddCiriticalValue;

		if( fCriticalProb > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax ) )
			fCriticalProb = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax );

		// BLOW_282: Target suffers forced critical damage
		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_282 ) )
			fCriticalProb = 1.0f;

		bool bBreakSuperAmmor = false;
		//If Super Armor Break state effect is active and probability succeeds, change to critical (must not be immune).
		if (hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_162) && !IsImmuned(STATE_BLOW::BLOW_162))
		{
			DNVector( DnBlowHandle ) vlhBreakSuperArmors;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_162, vlhBreakSuperArmors );

			float fRate = 0.0f;
			DnSkillHandle hSkill;

			if (!vlhBreakSuperArmors.empty())
			{
				const CDnSkill::SkillInfo* pSkillInfo = vlhBreakSuperArmors.at(0)->GetParentSkillInfo();
				if (pSkillInfo)
					hSkill = hHitter->FindSkill(pSkillInfo->iSkillID);

				if (hSkill && CDnSkill::UsingResult::Success == hSkill->CanExecute())
				{
					fRate = vlhBreakSuperArmors.at(0)->GetFloatValue();
				}
			}

			bBreakSuperAmmor = rand() % 10000 <= (fRate * 10000.0f);

			if (bBreakSuperAmmor)
			{
				fCriticalProb = 1.0f;
				m_HitParam.bBreakSuperAmmor = true;
				
				if (hSkill && CDnSkill::Passive == hSkill->GetSkillType())
				{
					// Send packet to display PassiveSkill cooldown...
					BYTE pBuffer[128];
					CPacketCompressStream Stream( pBuffer, 128 );

					DWORD dwUniqueID = hHitter ? hHitter->GetUniqueID() : -1;
					int nSkillID = hSkill->GetClassID();

					Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
					Stream.Write( &nSkillID, sizeof(nSkillID));

					hHitter->Send(eActor::SC_PASSIVESKILL_COOLTIME, &Stream);

					//Start actual cooldown.
					hSkill->OnBeginCoolTime();
				}

				//Send packet to display Super Armor Break effect.
				{
					//Send packet to display effect..
					BYTE pBuffer[128];
					CPacketCompressStream Stream( pBuffer, 128 );

					DWORD dwUniqueID = hHitter ? hHitter->GetUniqueID() : -1;
					STATE_BLOW::emBLOW_INDEX blowIndex = STATE_BLOW::BLOW_162;
					bool bShowEffect = true;
					Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
					Stream.Write( &blowIndex, sizeof(blowIndex));
					Stream.Write( &bShowEffect, sizeof(bShowEffect));

					hHitter->Send(eActor::SC_SHOW_STATE_EFFECT, &Stream);
				}

			}
		}
		//Check if super armor was broken by state effect #162
		if (bBreakSuperAmmor)
			isBreakSuperArmorBlow = true;
	
		//If state effect 79 is active, Super Armor always breaks.
		if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_079))
			isBreakSuperArmorBlow = true;
		
		EtVector3 vVelocity = m_HitParam.vVelocity;

		// Stun calculation
		int nStunProb = (int)( CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunMax ) * 100.f );
		if( GetStunResistance() > 0 ) {
			float fLocalTemp = powf( (float)( hHitter->GetStun() / GetStunResistance() ), CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunRevisioin ) ) * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunRevision2 );
			if( fLocalTemp > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunMax ) )
				fLocalTemp = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunMax );
			nStunProb = (int)( fLocalTemp * 100.f );
		}

		bool isAppliedForceSturn = false;

		// If the attacker has a forced stun state effect, add the probability value (0.0~1.0) from that state effect.
		if( hHitter && hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_133 ) )
		{
			DNVector( DnBlowHandle ) vlhStunBlow;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_133, vlhStunBlow );
			for( int i = 0; i < (int)vlhStunBlow.size(); ++i )
				nStunProb += int(vlhStunBlow.at(i)->GetFloatValue() * 100.0f);

			isAppliedForceSturn = true;
		}

//		if( hHitter->GetStun() > 0 ) nStunProb = (int)( ( ( 1.f - ( hHitter->GetStun() / GetStunResistance() ) ) * 100 ) * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunRevisioin ) );
		nStunProb = (int)( nStunProb * m_HitParam.fDamage );
		if( _rand(GetRoom())%100 < nStunProb ) {
			if( IsStandHit( m_HitParam.szActionName.c_str() ) ) {
				m_HitParam.szActionName = "Stun";
			}
			else if( IsAir( m_HitParam.szActionName.c_str() ) ) {
				if( strstr( m_HitParam.szActionName.c_str(), "High" ) == NULL ) {
					m_HitParam.szActionName = "Hit_AirUp";
				}
				vVelocity = m_HitParam.vVelocity * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StunVelocityRevision ) * ( 1.f + pHitStruct->fStunVelocityRevision );
			}
			else if( IsDown( m_HitParam.szActionName.c_str() ) ) {
				m_HitParam.szActionName = "Hit_AirUp";
			}
			
			
			//Critical hit must still occur even when stunned.
			//So if STATE_BLOW::BLOW_133 state effect is present, unconditionally set to Stun and
			//If STATE_BLOW::BLOW_133 state effect is active and hitType is set, keep hitType instead of changing it to Sturn
			if (!isAppliedForceSturn)
				HitType = CDnWeapon::Stun;
			else if (HitType == CDnWeapon::Normal)
				HitType = CDnWeapon::Stun;

			m_fStiffDelta = 0.f;
		}

		if ( m_HitParam.HasElement == CDnActorState::Fire && IsAppliedThisStateBlow(STATE_BLOW::BLOW_042)) // If the hit element is fire and the target is inflicted with burn
		{
			if (hHitter && hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_222)) // If the attacker has a fire-attribute critical state effect.
			{
				DNVector( DnBlowHandle ) vBurnCriticalBlows;
				hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_222, vBurnCriticalBlows );

				if (!vBurnCriticalBlows.empty())
				{
					if(vBurnCriticalBlows.at(0))
					{
						CDnBurnCriticalBlow *pBurnCriticalBlow = static_cast<CDnBurnCriticalBlow*>(vBurnCriticalBlows.at(0).GetPointer());

						if(pBurnCriticalBlow->GetMultiplyRatio() > 0)
							fCriticalProb = fCriticalProb + ( fCriticalProb * ( (float)pBurnCriticalBlow->GetMultiplyRatio() * 1/100 ) );

						if(pBurnCriticalBlow->GetAddValueRatio() > 0)
							fCriticalProb = fCriticalProb + (pBurnCriticalBlow->GetAddValueRatio() / fCriticalValue);


						if( fCriticalProb > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax ) )
							fCriticalProb = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalMax );
					}
				}
			}
		}

		//Changed to calculate critical rate after stun rate calculation.
		if( _rand(GetRoom())%100 < (int)( fCriticalProb * 100.f ) ) {
			vVelocity = m_HitParam.vVelocity * CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::CriticalVelocityRevision ) * ( 1.f + pHitStruct->fCriticalVelocityRevision );
			HitType = CDnWeapon::Critical;

#if defined(PRE_ADD_TOTAL_LEVEL_SKILL)
			//Unified level skill state effect on critical 
			if (hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_267))
			{
				DNVector( DnBlowHandle ) vlhBlows;
				hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_267, vlhBlows );
				
				int nCount = (int)vlhBlows.size();
				for( int i = 0; i < (int)vlhBlows.size(); ++i )
				{
					CDnAddCriticalRateBlow* pBlow = static_cast<CDnAddCriticalRateBlow*>(vlhBlows[i].GetPointer());
					if (pBlow && pBlow->IsActivated() == false && pBlow->GetLeftCoolTime() == 0.0f)
						pBlow->ApplyCriticalIncBlow();
				}
			}
#endif // PRE_ADD_TOTAL_LEVEL_SKILL

			CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(hHitter.GetPointer());
			if (pPlayer && pPlayer->IsPlayerActor()) {
				boost::shared_ptr<IDnObserverNotifyEvent> pNotifyEvent(new BubbleSystem::CDnOnCriticalHitMessage);
				boost::shared_ptr<BubbleSystem::CDnOnCriticalHitMessage> pOnCriticalEvent = boost::shared_polymorphic_downcast<BubbleSystem::CDnOnCriticalHitMessage>(pNotifyEvent);
				if (hHitter->GetProcessSkill())
					pOnCriticalEvent->SetSkillID(hHitter->GetProcessSkill()->GetClassID());
				pPlayer->Notify(pNotifyEvent);
			}

		}

		// Air combo calculation
		if( IsAir() && IsHit() ) {
			m_nAirComboCount++;
			hHitter->OnAirCombo( m_nAirComboCount );
		}
		else m_nAirComboCount = 0;

		m_HitParam.vVelocity = vVelocity;
	}
	else HitType = CDnWeapon::Defense;

	m_HitParam.HitType = HitType;
	// Damage calculation
	switch( HitType ) {
		case CDnWeapon::Normal:
		case CDnWeapon::Critical:
		case CDnWeapon::Stun:
		case CDnWeapon::CriticalRes:
			{
				// Reduce acceleration according to weight.
				if( GetWeight() > 0.f )
					m_HitParam.vVelocity.y /= GetWeight();
				if( GetWeight() > 1.f ) {
					float fTemp = m_HitParam.vVelocity.z;
					float fTest = CalcMovement( fTemp, 1.f, FLT_MAX, FLT_MIN, m_HitParam.vResistance.z );
					if( m_HitParam.vVelocity.z * fTest > 0.f ) {
						m_HitParam.vVelocity.x /= powf( 1.1f, GetWeight() );
						m_HitParam.vVelocity.z /= powf( 1.1f, GetWeight() );
					}
				}

				// Check damage correlation
				int nDamageType = m_HitParam.nDamageType;
				
				// Related to issue #13885.
				if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_086 ) )
				{
					// Only one team-reversal state effect applies.
					DNVector( DnBlowHandle ) vlReverseTeamBlows;
					hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_086, vlReverseTeamBlows );
					_ASSERT( false == vlReverseTeamBlows.empty() );
					if( false == vlReverseTeamBlows.empty() )
					{
						DnBlowHandle hBlow = vlReverseTeamBlows.front();
						CDnReverseTeamBlow* pReverseTeamBlow = static_cast<CDnReverseTeamBlow*>(hBlow.GetPointer());
						if( CDnReverseTeamBlow::HIT_SIGNAL_TARGET_CHANGE == pReverseTeamBlow->GetType() )
						{
							switch( nDamageType )
							{
								case 0: // Enemy -> Friend
									nDamageType = 1;
									break;

								case 1: // Friend -> Enemy
									nDamageType = 0; // #50166 Process only in Hittalbe.
									break;

								case 2: // All -> All
									break;
							}
						}
						else 
						if( CDnReverseTeamBlow::HIT_SIGNAL_TARGET_ALL == pReverseTeamBlow->GetType() )
						{
							// Change hit target to all.
							nDamageType = 2;
						}
					}
				}

				bool bCalcDamage = true;
				float fDamage = 0.f;
				switch( nDamageType )
				{
					case 0:	// Enemy
						if( GetTeam() == hHitter->GetTeam() ) bCalcDamage = false;
						break;
					case 1: // Friend
						if( GetTeam() != hHitter->GetTeam() ) bCalcDamage = false;
						break;
					case 2: // All
						break;
				}

				if( bCalcDamage ) 
				{
					fDamage = CalcDamage( hHitter, m_HitParam );
#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
					fDPS_DAMAGE = fDamage;
#endif
				}

				// Check if dead
				if( GetHP() <= 0.f )
					Die( hHitter );

				if (fDamage > 0.0f &&	//If there is damage
					GetHP() > 0.0f &&	//Not a death
					!IsAppliedThisStateBlow(STATE_BLOW::BLOW_183)) //When not ignoring prefix state effect (Ghoul mode)
				{
#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
					ProcessPrefixDefenceSkill_New(hHitterActor);
#else				
					ProcessPrefixDefenceSkill(hHitterActor);
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
				}

				bool bSuccessNormalDamage = true;
				int nSuperAmmorTime = 0;

				if( !IsDie() ) {
					// Skill Super Ammor Check
//					if( m_HitParam.nSkillSuperAmmorIndex == -1 || ( m_HitParam.nSkillSuperAmmorIndex != -1 && m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] > 0 ) {
					bool bApplySuperAmmor = false;
					if( m_HitParam.nSkillSuperAmmorIndex != -1 && m_HitParam.nSkillSuperAmmorIndex < 0)
						break;
					if( m_HitParam.nSkillSuperAmmorIndex == -1 ) {
						bApplySuperAmmor = true;
					}
					else if( m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] > 0 ) {
						int nAmmorDamage = m_HitParam.nSkillSuperAmmorDamage;// * (( HitType == CDnWeapon::Critical ) ? 2 : 1);

						// Skip super armor calculation if Super Armor Protection state effect is active. [2010/12/28 semozz]
						bool isApplySuperArmorShield = false;

						isApplySuperArmorShield = this->IsAppliedThisStateBlow(STATE_BLOW::BLOW_156);

						// [2011/02/09 semozz]
						// Super armor does not break even when Guardian Force's Shield state effect is applied.
						// Super armor protection has the highest priority.
						if (!isApplySuperArmorShield)
							isApplySuperArmorShield = this->IsAppliedThisStateBlow(STATE_BLOW::BLOW_055);

						//If Super Armor Break state effect is active, unconditionally set super armor to 0
						if (isBreakSuperArmorBlow)
						{
							m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] = 0;
						}
						else if (!isApplySuperArmorShield)						
						{
							//////////////////////////////////////////////////////////////////////////
							// State effect #142: super armor attack power ratio
							if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_142 ) )
							{
								DNVector(DnBlowHandle) vlBlows;
								float fValue = 1.0f;
								hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_142, vlBlows );
								for( int i = 0; i < (int)vlBlows.size(); ++i )
								{
									DnBlowHandle hBlow = vlBlows.at( i );
									fValue += (hBlow->GetFloatValue() - 1.0f);
								}
								nAmmorDamage = int((float)nAmmorDamage * fValue);
							}
							// BLOW_314: Target takes increased super armor damage (decreased super armor)
							if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_314 ) )
							{
								DNVector(DnBlowHandle) vlBlows314;
								GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_314, vlBlows314 );
								float fReduceRatio = 0.0f;
								for( DWORD bi = 0; bi < vlBlows314.size(); ++bi )
								{
									if( vlBlows314[bi] )
										fReduceRatio += static_cast<CDnDecreaseSuperArmorBlow*>( vlBlows314[bi].GetPointer() )->GetDecreaseRatio();
								}
								if( fReduceRatio > 0.0f )
									nAmmorDamage = (int)( (float)nAmmorDamage * ( 1.0f + fReduceRatio ) );
							}
							//////////////////////////////////////////////////////////////////////////

							m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] -= nAmmorDamage;
						}

						if( m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] <= 0 ) {
							m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] = 0;
							hHitter->OnBreakSkillSuperAmmor( m_HitParam.nSkillSuperAmmorIndex, m_nLastUpdateSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex], nAmmorDamage );
							OnBreakSkillSuperAmmor( m_HitParam );
							// BLOW_295: AddStateOnBreakSuperArmorBlow
							if( hHitter && hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_295 ) )
							{
								DNVector(DnBlowHandle) vlBlows295;
								hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_295, vlBlows295 );
								for( DWORD bi = 0; bi < vlBlows295.size(); ++bi )
								{
									if( vlBlows295[bi] )
										static_cast<CDnAddStateOnBreakSuperArmorBlow*>( vlBlows295[bi].GetPointer() )->OnBreakSuperArmor( GetMySmartPtr() );
								}
							}

						}
						else bApplySuperAmmor = true;
					}
					if( bApplySuperAmmor ) {
						bSuccessNormalDamage = false;
						nSuperAmmorTime = m_nSkillSuperAmmorTime;
						// Attenuate damage.
						SetHP( GetHP() + (INT64)( fDamage * m_fSkillSuperAmmorDamageDecreaseProb ) );
#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
						fDPS_DAMAGE += (int)( fDamage * m_fSkillSuperAmmorDamageDecreaseProb );
#endif
					}

					// Normal Super Ammor Check
					if( m_bEnableNormalSuperAmmor == true ) {
						if( m_HitParam.fDurability >= m_fBreakNormalSuperAmmorDurability )
							m_bEnableNormalSuperAmmor = false;
						else {
							bSuccessNormalDamage = false;
							nSuperAmmorTime = m_nNormalSuperAmmorTime;
						}
					}
					if( pHitStruct->nSuperAmmorTime > 0 ) nSuperAmmorTime = pHitStruct->nSuperAmmorTime;
				}

				bool bSuperAmmorReset = true;

				//////////////////////////////////////////////////////////////////////////
				// Handle Academic Force Out skill [2011/07/04 semozz]
				// If there is a weight limit and the actor's weight is less than the set weight, apply physics regardless of super armor
				if( 0 < m_HitParam.nWeightValueLimit )
				{
					if( (float)m_HitParam.nWeightValueLimit >= GetWeight() )
					{
						//Set bSuccessNormalDamage to true to apply physics.
						bSuccessNormalDamage = true;
						//Prevent super armor value from resetting.
						bSuperAmmorReset = false;
						//Play hit action if super armor breaks (0); otherwise remove hit action.
						//#39137 Skip if the monster is dead
						if (!IsDie() && m_HitParam.nSkillSuperAmmorIndex>=0 && m_nSkillSuperAmmorValue[m_HitParam.nSkillSuperAmmorIndex] > 0)
							m_HitParam.szActionName.clear();
					}                            
				}
				//////////////////////////////////////////////////////////////////////////

				//If hit motion plays while this state effect is active, the hit motion set in Escape is canceled mid-way.
				if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_218))
				{
#if defined(PRE_FIX_50007)
					//#50007 Must still transition to death action during Escape (remove Hit action only when not Die action).
					if (strstr(m_HitParam.szActionName.c_str(), "Die") == NULL)
#endif // PRE_FIX_50007
						m_HitParam.szActionName.clear();
				}

				m_HitParam.bSuccessNormalDamage = bSuccessNormalDamage;
				m_HitParam.nSuperAmmorDelay = nSuperAmmorTime;

				if( bSuccessNormalDamage ) {
					m_nNormalSuperAmmorTime = 0;

					if (bSuperAmmorReset)
						memset( m_nSkillSuperAmmorValue, 0, sizeof(m_nSkillSuperAmmorValue) );

					CheckDamageVelocity( hHitter );

					// Reset items to reset when hit
					MAMovementBase *pMovement = GetMovement();
					if( pMovement ) {
						pMovement->ResetMove();
						pMovement->ResetLook();
					}

					if( !m_HitParam.szActionName.empty() ) 
					{
#ifdef PRE_FIX_81750
						CDnChangeStandActionBlow::ReleaseStandChangeSkill( GetActorHandle(), true );
#endif
						SetActionQueue( m_HitParam.szActionName.c_str(), 0, 3.f, 0.f, false );
					}
				}
				else {
					int nDelay = (int)( nSuperAmmorTime * ( m_fStiffDelta / s_fMaxStiffTime ) );
					if( nDelay > 0 ) {
						SetPlaySpeed( (DWORD)( nSuperAmmorTime * ( m_fStiffDelta / s_fMaxStiffTime ) ), 0.03f );
						if( m_HitParam.hWeapon && m_HitParam.hWeapon->GetEquipType() != CDnWeapon::Arrow ) 
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
					m_HitParam.szActionName.clear();
				}
			}
			break;
		case CDnWeapon::Defense: 
			m_HitParam.vVelocity *= 0.5f;
			m_HitParam.vVelocity.y = 0.f;
			if( m_HitParam.hWeapon && m_HitParam.hWeapon->GetEquipType() != CDnWeapon::Arrow ) 
			{
//#ifdef PRE_FIX_HITSTIFF
				// Frame-change state effect takes priority, so ignore SetPlaySpeed() while the state effect is active.
				if( false == (0 < hHitter->GetFrameStopRefCount() || hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 )) )
				{
					hHitter->SetPlaySpeed( 100, 0.03f );
				}
//#else
//				hHitter->SetPlaySpeed( 100, 0.03f );
//#endif // #ifdef PRE_FIX_HITSTIFF
			}

			CheckDamageVelocity( hHitter );
			// Play animation and
			if( !m_HitParam.szActionName.empty() )
				SetActionQueue( m_HitParam.szActionName.c_str(), 0, 3.f, 0.f, false );
			break;
	}

//	OutputDebug( "Damage : %s, %.2f, %.2f ( %.2f, %.2f )\n", m_HitParam.szActionName.c_str(), m_HitParam.vVelocity.y, m_HitParam.vResistance.y, GetVelocity() ? GetVelocity()->y :0.f, GetResistance() ? GetResistance()->y : 0.f);

	// Call to handle weapon sound, particles, etc.
	if( m_HitParam.hWeapon ) {
		m_HitParam.hWeapon->OnHitSuccess( GetMySmartPtr(), HitType, m_HitParam.vPosition, m_HitParam.bFirstHit );
		if( HitParam.hWeapon->GetWeaponType() & CDnWeapon::Projectile ) {
			CDnProjectile *pProjectile = static_cast<CDnProjectile *>(HitParam.hWeapon.GetPointer());
			if( pProjectile ) pProjectile->OnDamageSuccess( GetMySmartPtr(), m_HitParam );
			else HitParam.hWeapon->SetDestroy();
		}
	}


	// Cancel skill if hit while casting. 
	if( false == m_HitParam.szActionName.empty() )
	{
		// For skill cancellation on hit during a skill action, check only object existence instead of IsProcessSkill().
		// If checked internally via IsFinished(), GetCurrentAction() inside PlayAniProcess prioritizes the action queue value 
		// Because it fetches it, if a hit action is set via SetActionQueue above, the skill is judged as ended and the block below does not execute.
		if( m_hProcessSkill )
		{
			// During aura skill cast.. i.e. if hit before all state effects are applied, the aura skill was not fully used
			if( IsEnabledAuraSkill() && m_hProcessSkill->IsAuraOn() )
			{
				//#41352 Must send cancel packet to client when aura skill is canceled.
				CmdFinishAuraSkill(m_hAuraSkill->GetClassID());

				OnSkillAura( m_hAuraSkill, false );
				m_hProcessSkill.Identity();

				OutputDebug( "### Aura skill canceled on hit!!\n" );
			}
			else
			{
				m_hCanceledSkill = m_hProcessSkill;
				m_hProcessSkill->OnEnd( CDnActionBase::m_LocalTime, 0.f );
				m_hProcessSkill.Identity();
			}
		}
	}

	// Currently auto passive skills can only trigger when hit.
	UseAutoPassiveSkill( 0, 0.0f );

	if( hHitter ) hHitter->OnHitSuccess( CDnActionBase::m_LocalTime, GetMySmartPtr(), pHitStruct );

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
	if( CDnDPSReporter::IsActive() && hHitter && hHitter->IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>( hHitter.GetPointer());
		if(CDnDPSReporter::GetInstance().IsEnabledUser(pPlayerActor->GetCharacterDBID()))
		{
			DNVector(DnActorHandle) hVecList;
			ScanActor( GetRoom(), *hHitter->GetPosition() , 500.f , hVecList );

			CDnDPSReporter::GetInstance().ApplyAreaMonsterCount( (int)hVecList.size() );
			CDnDPSReporter::GetInstance().ApplyBaseData( &m_HitParam , fDPS_DAMAGE );
			CDnDPSReporter::GetInstance().ApplyTargetData(GetActorHandle());
			CDnDPSReporter::GetInstance().EndReport();
		}
	}
#endif
}

void CDnActor::RequestDamage( CDnDamageBase *pHitter, int nSeed, INT64 nDamage )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	char cHitterType = pHitter->GetDamageObjectType();
	DWORD dwUniqueID = pHitter->GetDamageObjectUniqueID();
	ASSERT( dwUniqueID != -1 && "Invalid DamageBase ID" );
	bool bSendWeaponSerialID = false;
	DWORD dwWeaponUniqueID = -1;
	INT64 nWeaponSerialID = -1;

	int nActionIndex = GetElementIndex( m_HitParam.szActionName.c_str() );
	if( m_HitParam.hWeapon ) {
		if( m_HitParam.hWeapon->GetSerialID() == -1 ) 
			dwWeaponUniqueID = m_HitParam.hWeapon->GetUniqueID();
		else {
			bSendWeaponSerialID = true;
			nWeaponSerialID = m_HitParam.hWeapon->GetSerialID();
		}
	}
	INT64 nCurrentHP = GetHP();

	Stream.Write( &nSeed, sizeof(int) );
	Stream.Write( &cHitterType, sizeof(char) );
	Stream.Write( &dwUniqueID, sizeof(DWORD) );
	Stream.Write( &bSendWeaponSerialID, sizeof(bool) );
	if( bSendWeaponSerialID ) Stream.Write( &nWeaponSerialID, sizeof(INT64) );
	else Stream.Write( &dwWeaponUniqueID, sizeof(DWORD) );
	Stream.Write( &nActionIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &nDamage, sizeof(INT64) );
	Stream.Write( &nCurrentHP, sizeof(INT64) ); // Put it in for now.
	Stream.Write( &m_HitParam.HitType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( ( GetVelocity() ) ? GetVelocity() : &EtVector3( 0.f, 0.f, 0.f ), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( ( GetResistance() ) ? GetResistance() : &EtVector3( 0.f, 0.f, 0.f ), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &m_HitParam.vPosition, sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( &EtVec3toVec2(m_HitParam.vViewVec), sizeof(EtVector2), CPacketCompressStream::VECTOR2_SHORT );
	Stream.Write( &m_HitParam.bFirstHit, sizeof(bool) );
	//Stream.Write( &m_HitParam.nBoneIndex, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &m_fStiffDelta, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 10000.f );
	Stream.Write( &m_fDownDelta, sizeof(float), CPacketCompressStream::FLOAT_SHORT, 1000.f );
	Stream.Write( &m_HitParam.nSuperAmmorDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	Stream.Write( &m_HitParam.bSuccessNormalDamage, sizeof(bool) );
	Stream.Write( &m_HitParam.DistanceType, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

	// #32722 Fix issue where only the client cancels the skill action if hit on the server before the client's skill-use packet arrives.
	int iCanceledSkillID = 0;
	if( IsPlayerActor() )
	{
		if( m_hCanceledSkill )
		{
			iCanceledSkillID = m_hCanceledSkill->GetClassID();
			m_hCanceledSkill.Identity();
		}
		else
		if( IsDie() )	// If killed while using a skill, send CanceledSkillID as -1 so the client can know.
		{
			iCanceledSkillID = -1;
		}
	}
	Stream.Write( &iCanceledSkillID, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	//////////////////////////////////////////////////////////////////////////

	// Temporary
	// ToDo : Junyoung needs to check this later. Temporarily prevented from crashing.
	int nSuperArmorValue = 0;
	// Player super armor can reach up to 33000, so the server sends it uncompressed. (#10065)
	Stream.Write( (m_HitParam.nSkillSuperAmmorIndex >= 0 && m_HitParam.nSkillSuperAmmorIndex <= 3) ? &m_nSkillSuperAmmorValue[ m_HitParam.nSkillSuperAmmorIndex ] : &nSuperArmorValue, sizeof(int) );
	Stream.Write( (m_HitParam.nSkillSuperAmmorIndex >= 0 && m_HitParam.nSkillSuperAmmorIndex <= 3) ? &m_nLastUpdateSkillSuperAmmorValue[ m_HitParam.nSkillSuperAmmorIndex ] : &nSuperArmorValue, sizeof(int) );
	//Stream.Write( (m_HitParam.nSkillSuperAmmorIndex >= 0 && m_HitParam.nSkillSuperAmmorIndex <= 3) ? &m_nSkillSuperAmmorValue[ m_HitParam.nSkillSuperAmmorIndex ] : &nSuperArmorValue, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
	//Stream.Write( (m_HitParam.nSkillSuperAmmorIndex >= 0 && m_HitParam.nSkillSuperAmmorIndex <= 3) ? &m_nLastUpdateSkillSuperAmmorValue[ m_HitParam.nSkillSuperAmmorIndex ] : &nSuperArmorValue, sizeof(int), CPacketCompressStream::INTEGER_SHORT );

	// Combo sync

	Stream.Write( &m_HitParam.HasElement, sizeof(int), CPacketCompressStream::INTEGER_CHAR );

#if defined( PRE_ADD_LOTUSGOLEM )
	Stream.Write( &m_HitParam.bIgnoreShowDamage, sizeof(bool) );
#endif // #if defined( PRE_ADD_LOTUSGOLEM )
#ifdef PRE_ADD_STAGE_LIMIT_INTERFACE
	Stream.Write( &m_HitParam.bStageLimit, sizeof(bool) );
#endif

	switch( pHitter->GetDamageObjectType() ) {
		case DamageObjectTypeEnum::Actor:
			{
				DnActorHandle hActor = pHitter->GetActorHandle();
				if( !hActor ) break;
				if( dwUniqueID >= 100000 ) {
					CDnPlayerActor *pPlayer = ( hActor->IsPlayerActor() ) ? static_cast<CDnPlayerActor*>(hActor.GetPointer()) : NULL;
					if( !pPlayer ) break;
					int nComboCount = pPlayer->GetComboCount();
					int nComboDelay = pPlayer->GetComboDelay();
					Stream.Write( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
					Stream.Write( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
				}
				// (#11415) Include attacks from summoned relics in the combo count. 
				// Check if it is a monster type summoned directly by a player, and if so, include it in the packet and send.
				// Determine the same on client side as well.
				else if( hActor->IsMonsterActor() )
				{
					CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
				
					// Update combo for the summoning player when a relic (summon) hits. (#11415)
					DnActorHandle hSummonMasterPlayer = pMonsterActor->GetSummonerPlayerActor();
					if( hSummonMasterPlayer && hSummonMasterPlayer->IsPlayerActor() )
					{
						CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hSummonMasterPlayer.GetPointer());
						int nComboCount = pPlayerActor->GetComboCount();
						int nComboDelay = pPlayerActor->GetComboDelay();
						Stream.Write( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
						Stream.Write( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
					}
					else
					if( CDnActorState::Cannon == pMonsterActor->GetActorType() )
					{
						// When a cannon monster lands a hit, update the master actor's combo count. (#25778)
						CDnCannonMonsterActor* pCannonMonster = static_cast<CDnCannonMonsterActor*>( pMonsterActor );
						DnActorHandle hPlayerActor = pCannonMonster->GetMasterPlayerActor();

						int nComboCount = 0;
						int nComboDelay = 0;
						if( hPlayerActor && hPlayerActor->IsPlayerActor() )
						{
							CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(hPlayerActor.GetPointer());
							int nComboCount = pPlayerActor->GetComboCount();
							int nComboDelay = pPlayerActor->GetComboDelay();
						}
						Stream.Write( &nComboCount, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
						Stream.Write( &nComboDelay, sizeof(int), CPacketCompressStream::INTEGER_SHORT );
					}
				}
			}
			break;
		case DamageObjectTypeEnum::Prop:
			break;
	}
	OnDamageWriteAdditionalPacket( &Stream );

	Send( eActor::SC_ONDAMAGE, &Stream );

	// #31022 Do not record as last damage if hit by a hit signal from an action run by a charger state effect. 
	// To avoid affecting copied logic used in the fire state effect.
	DnActorHandle hHitter = pHitter->GetActorHandle();
	if( hHitter && false == hHitter->IsOnSignalFromChargerSE() )
		m_iLastDamage = nDamage;
}


void CDnActor::OnRequestHPMPDelta( /*IN OUT*/ INT64& nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta /* = false */ )
{
	if( false == bIsMPDelta && 0 < nHPMPDelta )
	{
		// State effect #141: change heal ratio.
		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_141 ) )
		{
			DNVector(DnBlowHandle) vlBlows;
			float fValue = 1.0f;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_141, vlBlows );
			for( int i = 0; i < (int)vlBlows.size(); ++i )
			{
				DnBlowHandle hBlow = vlBlows.at( i );
				fValue += (hBlow->GetFloatValue() - 1.0f);
			}

			nHPMPDelta = INT64((float)nHPMPDelta * fValue);
		}
	}
}


// Server-side HP/MP does not change, so be sure to adjust server HP separately!
#ifdef PRE_FIX_77172
void CDnActor::RequestHPMPDelta( ElementEnum eElement, INT64 nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta /*= false */, 
								bool bShowValue, CDnDamageBase::SHitParam* pHitParam/* = NULL*/, bool bKeepJumpMovement /*=false*/ )
#else // PRE_FIX_77172
void CDnActor::RequestHPMPDelta( ElementEnum eElement, INT64 nHPMPDelta, const DWORD dwHitterUniqueID, bool bIsMPDelta /*= false */, bool bShowValue, CDnDamageBase::SHitParam* pHitParam/* = NULL*/ )
#endif // PRE_FIX_77172
{
	OnRequestHPMPDelta( nHPMPDelta, dwHitterUniqueID, bIsMPDelta );

#if defined(PRE_FIX_59680)
	//When normal Hit signal processing fails due to Summon Puppet's TransmitDamageBlow(51), the RecoverHPByAttack(248) state effect is not processed properly.
	//State effect 51 calls this function to process damage to the owner actor; damage must be set in HitParam here so state effect 248 works properly
	//Can recover HP. (In state effect 248, if TargetActor is SummonMonster, must retrieve owner actor's HitParam info.)
	if (bIsMPDelta == false)
	{
		m_HitParam.nCalcDamage = -nHPMPDelta;
	}
#endif // PRE_FIX_59680

	BYTE pBuffer[ 32 ];
	CPacketCompressStream Stream( pBuffer, 32 );

	Stream.Write( &eElement, sizeof(ElementEnum), CPacketCompressStream::INTEGER_CHAR );
	Stream.Write( &nHPMPDelta, sizeof(INT64) );
	Stream.Write( &dwHitterUniqueID, sizeof(DWORD) );
	Stream.Write( &bIsMPDelta, sizeof(bool) );
	Stream.Write( &bShowValue, sizeof(bool) );

#if defined(PRE_FIX_59308)
	int nHitType = pHitParam ? pHitParam->HitType : CDnWeapon::Normal;
	Stream.Write( &nHitType, sizeof(int) );
#endif // PRE_FIX_59308
#ifdef PRE_FIX_77172
	Stream.Write( &bKeepJumpMovement, sizeof(bool) );
#endif // PRE_FIX_77172

	Send( eActor::SC_SETHPMP_DELTA, &Stream );
}


// Notify the client that a probability-triggered state effect was activated by the server. Currently used only for server-to-client notifications.
// Currently used by Payback Mana.
void CDnActor::RequestSEProbSuccess( int iSkillID, STATE_BLOW::emBLOW_INDEX eBlowIndex )
{
	BYTE pBuffer[ 16 ];
	CPacketCompressStream Stream( pBuffer, 16 );

	Stream.Write( &iSkillID, sizeof(int) );
	Stream.Write( &eBlowIndex, sizeof(STATE_BLOW::emBLOW_INDEX) );

	Send( eActor::SC_STATEEFFECT_PROB_SUCCESS, &Stream );
}

void CDnActor::RequestCooltimeReset( int iSkillID )
{
	BYTE pBuffer[ 4 ];

	CPacketCompressStream Stream( pBuffer, 16 );

	Stream.Write( &iSkillID, sizeof(int) );

	Send( eActor::SC_COOLTIME_RESET, &Stream );
}

bool CDnActor::Compare_PreStateOrder( CDnActor::PreStateStruct &a, CDnActor::PreStateStruct &b )
{
	return ( a.nOffset < b.nOffset ) ? true : false;
}

void CDnActorActionStateCache::LoadAction( CDnActor* pActor, const char* szFullPathName,								\
										   std::vector<CEtActionBase::ActionElementStruct*>* pVecActionElementList,		\
										   std::vector<std::vector<CDnActor::PreStateStruct>>& ActionState,				\
										   std::vector<std::vector<CDnActor::PreStateStruct>>& ActionCustomState,		\
										   std::vector<std::vector<CDnActor::PreStateStruct>>& CanMoveState )
{
	// Check if there is cached info
	{
		//CPerformanceLog log("loadaction-cache");

		std::map<std::string,SData>::iterator itor;
		{
			ScopeLock<CSyncLock> lock(m_Lock);
			itor = m_mRepository.find( szFullPathName );
		}

		if( itor != m_mRepository.end() )
		{
			ActionState.reserve( (*itor).second.VecPreActionState.size() );
			ActionCustomState.reserve( (*itor).second.VecPreActionCustomState.size() );
			CanMoveState.reserve( (*itor).second.VecPreCanMoveState.size() );

			ActionState			= (*itor).second.VecPreActionState;
			ActionCustomState	= (*itor).second.VecPreActionCustomState;
			CanMoveState		= (*itor).second.VecPreCanMoveState;
			return;
		}
	}


	SData data;

	int nValue = 0;
	std::vector<CDnActor::PreStateStruct> VecList;
	CEtActionSignal *pSignal;
	for( DWORD i=0; i< pVecActionElementList->size(); i++ ) 
	{
		// STE_State
		VecList.clear();
		for( int j=0; ; j++ ) 
		{
			nValue = 0;
			pSignal = pActor->GetSignal( i, STE_State, j );
			if( pSignal ) 
			{
				StateStruct *pStruct = (StateStruct *)pSignal->GetData();
				nValue = CDnActorState::s_nActorStateIndex[pStruct->nStateOne] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateTwo] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateThree];
			}
			else 
				break;

			VecList.push_back( CDnActor::PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		
		if( VecList.empty() ) 
			VecList.push_back( CDnActor::PreStateStruct( nValue, 0 ) );
		else 
			std::sort( VecList.begin(), VecList.end(), pActor->Compare_PreStateOrder );

		data.VecPreActionState.push_back( VecList );

		// STE_CustomState
		VecList.clear();
		for( int j=0; ; j++ ) 
		{
			nValue = 0;
			pSignal = pActor->GetSignal( i, STE_CustomState, j );
			if( pSignal ) {
				CustomStateStruct *pStruct = (CustomStateStruct *)pSignal->GetData();
				nValue = CDnActorState::s_nActorStateIndex[pStruct->nStateOne] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateTwo] | 
					CDnActorState::s_nActorStateIndex[pStruct->nStateThree];
			}
			else 
				break;

			VecList.push_back( CDnActor::PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		if( VecList.empty() ) 
			VecList.push_back( CDnActor::PreStateStruct( nValue, 0 ) );
		else 
			std::sort( VecList.begin(), VecList.end(), pActor->Compare_PreStateOrder );

		data.VecPreActionCustomState.push_back( VecList );

		// STE_CanMove
		VecList.clear();
		for( int j=0; ; j++ ) 
		{
			nValue = 0;
			pSignal = pActor->GetSignal( i, STE_CanMove, j );
			if( pSignal ) 
			{
				CanMoveStruct *pStruct = (CanMoveStruct *)pSignal->GetData();
				nValue = pStruct->bCanMove;
			}
			else 
				break;

			VecList.push_back( CDnActor::PreStateStruct( nValue, pSignal->GetStartFrame() ) );
		}
		if( VecList.empty() ) 
			VecList.push_back( CDnActor::PreStateStruct( nValue, 0 ) );
		else 
			std::sort( VecList.begin(), VecList.end(), pActor->Compare_PreStateOrder );

		data.VecPreCanMoveState.push_back( VecList );
	}

	// Record cash info
	{
		ScopeLock<CSyncLock> lock(m_Lock);
		m_mRepository.insert( std::make_pair(szFullPathName, data) );
	}

	ActionState.reserve( data.VecPreActionState.size() );
	ActionCustomState.reserve( data.VecPreActionCustomState.size() );
	CanMoveState.reserve( data.VecPreCanMoveState.size() );

	ActionState			= data.VecPreActionState;
	ActionCustomState	= data.VecPreActionCustomState;
	CanMoveState		= data.VecPreCanMoveState;
}

bool CDnActor::LoadAction( const char *szFullPathName )
{
	bool bResult = CDnActionBase::LoadAction( szFullPathName );
	if( !bResult ) return false;

	CDnActorActionStateCache::GetInstance().LoadAction( this, szFullPathName, m_pVecActionElementList, m_VecPreActionState, m_VecPreActionCustomState, m_VecPreCanMoveState );

	return bResult;
}

void CDnActor::FreeAction()
{
	CDnActionBase::FreeAction();
	SAFE_DELETE_VEC( m_VecPreActionState );
}

void CDnActor::LinkWeapon( int nEquipIndex )
{
	switch( m_hWeapon[nEquipIndex]->GetEquipType() ) {
		case CDnWeapon::Sword:
		case CDnWeapon::Axe:
		case CDnWeapon::Hammer:
		case CDnWeapon::SmallBow:
		case CDnWeapon::BigBow:
		case CDnWeapon::CrossBow:
		case CDnWeapon::Staff:
		case CDnWeapon::Book:
		case CDnWeapon::Orb:
		case CDnWeapon::Puppet:
		case CDnWeapon::Mace:
		case CDnWeapon::Flail:
		case CDnWeapon::Wand:
		case CDnWeapon::Shield:
		case CDnWeapon::Gauntlet:
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex );
			break;
		case CDnWeapon::Arrow:
			m_hWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), m_hWeapon[0] );
			break;
	}
}

void CDnActor::AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex, bool bDelete )
{
#if defined(PRE_ADD_50907)
	if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_237))
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

	if( m_hWeapon[nEquipIndex] ) {
		DetachWeapon( nEquipIndex );
	}
	m_hWeapon[nEquipIndex] = hWeapon;
	m_bSelfDeleteWeapon[nEquipIndex] = bDelete;
	m_hWeapon[nEquipIndex]->CreateObject();

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
	if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_237))
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

bool CDnActor::IsStay( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stay ) ? true : false;
	return false;
}

bool CDnActor::IsMove( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Move ) ? true : false;
	return false;
}

bool CDnActor::IsAttack( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Attack ) ? true : false;
	return false;
}

bool CDnActor::IsHit( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Hit ) ? true : false;
	return false;
}

bool CDnActor::IsAir( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Air ) ? true : false;
	return false;
}

bool CDnActor::IsDown( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Down ) ? true : false;
	return false;
}

bool CDnActor::IsStun( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stun ) ? true : false;
	return false;
}

bool CDnActor::IsStiff( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionState[nIndex][0].nState & ActorStateEnum::Stiff ) ? true : false;
	return false;
}

bool CDnActor::IsFly( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_Fly ) ? true : false;
	return false;
}

bool CDnActor::IsGround( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return ( m_VecPreActionCustomState[nIndex][0].nState & ActorCustomStateEnum::Custom_Ground ) ? true : false;
	return false;
}

bool CDnActor::IsStandHit( const char *szActionName )
{
	int nIndex;
	int nState = 0;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) nState = m_VecPreActionState[nIndex][0].nState;
	else return false;
	if( nState == ActorStateEnum::Hit || nState == ( ActorStateEnum::Hit | ActorStateEnum::Stiff ) ) return true;
	return false;
}

int CDnActor::GetState( const char *szActionName )
{
	int nIndex;
	if( ( nIndex = GetElementIndex( szActionName ) ) != -1 ) return m_VecPreActionState[nIndex][0].nState;
	return 0;
}

int CDnActor::GetState( int nElementIndex )
{
	if( nElementIndex < 0 || nElementIndex >= (int)m_VecPreActionState.size() ) return 0;
	return m_VecPreActionState[nElementIndex][0].nState;
}


// Separate from SetPlaySpeed, this function updates whenever frame change or frame stop state effects Begin/End.
// Collect the values of applied frame-change state effects, sum them up, and set them at once.
// If FrameStop state effect is active, process as 0 frames.
// If state effect is empty, restore to 60 frames.
// Runs separately from SetPlaySpeed, but after ProcessPlaySpeed() finishes, call this function once more for state effects
// Must re-update the final affected frame. Therefore, SetPlaySpeed must not be used on the state effect side.
void CDnActor::UpdateFPS( void )
{
	// Prop actors do not run process, so playspeed must not be adjusted. #26354 
	if( ActorTypeEnum::PropActor == GetActorType() )
		return;

	float fFrameRatio = 1.0f;
	if( m_bModifyPlaySpeed )
	{
		fFrameRatio = m_fPlaySpeed;

		float fPrevFPS = CDnActionBase::m_fFPS;
		CDnActionBase::SetFPS( 60.f * fFrameRatio );

		for( DWORD i=0; i<m_hVecLastHitListByRemainTime.size(); i++ )
		{
			if( m_hVecLastHitListByRemainTime[i] ) 
			{
				// First restore based on 60 frames, then
				float fFrame = ( ( m_hVecLastHitListByRemainTime[i]->GetDamageRemainTime( GetUniqueID() ) - CDnActionBase::m_LocalTime ) / 1000.f ) * fPrevFPS;

				// Recalculate RemainTime with the newly applied frame speed.
				if( CDnActionBase::m_fFPS > 0.f )
					m_hVecLastHitListByRemainTime[i]->SetDamageRemainTime( GetUniqueID(), CDnActionBase::m_LocalTime + (LOCAL_TIME)( fFrame / ( CDnActionBase::m_fFPS) * 1000.f ) );
			}
		}
	}
	else
	{
		ResetPlaySpeed();

		// Frame-freeze state effect. If even one is active, freeze all frames.
		if( 0 < GetFrameStopRefCount() )
		{
			float fPrevFPS = CDnActionBase::m_fFPS;
			CDnActionBase::SetFPS( 0.0f );
			fFrameRatio = 0.0f;
		}
		else
		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 ) ||
			IsAppliedThisStateBlow( STATE_BLOW::BLOW_144 ) || 
			IsAppliedThisStateBlow( STATE_BLOW::BLOW_220)
			)
		{
#ifdef PRE_ADD_SLOW_DEBUFF_LIMIT
			float fBuffFrameRatio = 0.0f;
#endif // PRE_ADD_SLOW_DEBUFF_LIMIT
			// Collect and apply values from frame-change state effects.
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
			if( GetGameRoom() && GetGameRoom()->bIsPvPRoom() )
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

			float fPrevFPS = CDnActionBase::m_fFPS;
			CDnActionBase::SetFPS( 60.0f * fFrameRatio );

			for( DWORD i=0; i<m_hVecLastHitListByRemainTime.size(); i++ )
			{
				if( m_hVecLastHitListByRemainTime[i] ) 
				{
					// First restore based on 60 frames, then
					float fFrame = ( ( m_hVecLastHitListByRemainTime[i]->GetDamageRemainTime( GetUniqueID() ) - CDnActionBase::m_LocalTime ) / 1000.f ) * fPrevFPS;
		
					// Recalculate RemainTime with the newly applied frame speed.
					m_hVecLastHitListByRemainTime[i]->SetDamageRemainTime( GetUniqueID(), CDnActionBase::m_LocalTime + (LOCAL_TIME)( fFrame / ( CDnActionBase::m_fFPS ) * 1000.f ) );
				}
			}
		}
		else
		{
			fFrameRatio = 1.0f;

			float fPrevFPS = CDnActionBase::m_fFPS;
			
			// Restore original state
			CDnActionBase::SetFPS( 60.0f );

			for( DWORD i=0; i<m_hVecLastHitListByRemainTime.size(); i++ )
			{
				if( m_hVecLastHitListByRemainTime[i] )
				{
					float fFrame = ( ( m_hVecLastHitListByRemainTime[i]->GetDamageRemainTime( GetUniqueID() ) - CDnActionBase::m_LocalTime ) / 1000.f ) * fPrevFPS;
					if( m_hVecLastHitListByRemainTime[i] )
						m_hVecLastHitListByRemainTime[i]->SetDamageRemainTime( GetUniqueID(), CDnActionBase::m_LocalTime + (LOCAL_TIME)( fFrame / ( CDnActionBase::m_fFPS ) * 1000.f ) );
				}
			}

			m_hVecLastHitListByRemainTime.clear();
		}
	}

	if( IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(this);
		pPlayerActor->OnChangePlaySpeed( 0, fFrameRatio );
	}
}

void CDnActor::AddedFrameStop( void ) 
{ 
	++m_iFrameStopRefCount;

	// #28345 For monsters, if AI is not disabled, a new action gets set while the frame is frozen
	// There is an issue where it changes to frame 0.
	if( IsMonsterActor() )
	{
		static_cast<CDnMonsterActor*>(this)->SetAIState( MAAiReceiver::Disable, false );
	}

	UpdateFPS(); 
};

void CDnActor::RemovedFrameStop( void ) 
{
	m_iFrameStopRefCount--; 
	if( m_iFrameStopRefCount < 0 )
		m_iFrameStopRefCount = 0;

	// #28345 For monsters, if AI is not disabled, a new action gets set while the frame is frozen
	// There is an issue where it changes to frame 0.
	if( IsMonsterActor() )
	{
		if( 0 == m_iFrameStopRefCount )
		{
			static_cast<CDnMonsterActor*>(this)->SetAIState( MAAiReceiver::Threat, false );
		}
	}

	UpdateFPS(); 
};


void CDnActor::SetPlaySpeed( DWORD dwFrame, float fSpeed )
{
	//// Frame-change state effect takes priority, so ignore SetPlaySpeed() while the state effect is active.
	//if( 0 < GetFrameStopRefCount() || IsAppliedThisStateBlow( STATE_BLOW::BLOW_025 ) )
	//	return;

	if( m_bModifyPlaySpeed == true ) 
		return;

	// Prop actors do not run process, so playspeed must not be adjusted. #26354 
	if( ActorTypeEnum::PropActor == GetActorType() )
		return;

	m_bModifyPlaySpeed = true;
	m_PlaySpeedStartTime = CDnActionBase::m_LocalTime;
	m_dwPlaySpeedTime = dwFrame;

	m_fPlaySpeed = fSpeed;
	UpdateFPS();
}


void CDnActor::ProcessPlaySpeed( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_bModifyPlaySpeed == false ) return;
	if( m_PlaySpeedStartTime == 0 ) m_PlaySpeedStartTime = LocalTime;
	if( LocalTime - m_PlaySpeedStartTime > m_dwPlaySpeedTime ) 
	{
		//m_bModifyPlaySpeed = false; // Called inside ResetPlaySpeed function.
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

	for( DWORD i=0; i<m_hVecLastHitListByRemainTime.size(); i++ ) {
		if( m_hVecLastHitListByRemainTime[i] ) {
			float fFrame = ( ( m_hVecLastHitListByRemainTime[i]->GetDamageRemainTime( GetUniqueID() ) - CDnActionBase::m_LocalTime ) / 1000.f ) * CDnActionBase::m_fFPS;
			m_hVecLastHitListByRemainTime[i]->SetDamageRemainTime( GetUniqueID(), CDnActionBase::m_LocalTime + (LOCAL_TIME)( fFrame / 60.f * 1000.f ) );
		}
	}

	CDnActionBase::SetFPS( 60.f );

	if( IsPlayerActor() )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(this);
		pPlayerActor->OnChangePlaySpeed( 0, 60.0f / CDnActionBase::m_fFPS );
	}

	// Note: Entries restored by SignalEndTime are removed here.
	//m_hVecLastHitListByRemainTime.clear();

	m_bModifyPlaySpeed = false;
}

void CDnActor::SyncClassTime( LOCAL_TIME LocalTime )
{
	CDnActionBase::m_LocalTime = LocalTime;
	MASkillUser::m_LocalTime = LocalTime;
}

void CDnActor::OnClash( EtVector3 &vPosition, OnClashFloorCheckType bFloorClash )
{
	if( !IsDie() && IsHit() && ( IsAir() || IsDown() ) && !IsStun() ) {
		float fDecrease = 0.85f;

		if( GetVelocity()->x != 0.f ) {
			SetVelocityX( -GetVelocity()->x * fDecrease );
			SetResistanceX( -GetResistance()->x );
		}

		if( GetVelocityValue()->z < -120.f ) {
			float fVelocityValue = GetVelocityValue()->z; // If the impact exceeds a certain threshold, reduce it. 300 is optimal for now..
			if( fVelocityValue < -300.f ) fVelocityValue = -300.f;
			float fYVel = 5.f + ( -fVelocityValue * 0.01f );
//			if( fYVel > 8.f ) fYVel = 8.f;
//			if( fVelocityValue >
//			OutputDebug( "PrevVel Z : %.2f PrevVel Y : %.2f - %.2f\n", GetVelocity()->z, GetVelocity()->y, GetVelocityValue()->z );
			SetVelocityZ( -GetVelocity()->z * ( 1.35f - ( 1.f / 4.f * ( -fVelocityValue * 0.01f  ) ) ) );
			SetResistanceZ( -GetResistance()->z );

			SetVelocityY( fYVel );
			SetResistanceY( -18.f );

//			OutputDebug( "Vel Z : %.2f Vel Y : %.2f\n", GetVelocity()->z, GetVelocity()->y );
//			OutputDebug( "Res Z : %.2f Res Y : %.2f\n", GetResistance()->z, GetResistance()->y );

			EtVector3 vLook;
			vLook = -m_Cross.m_vZAxis;
			vLook.y = 0.f;
			EtVec3Normalize( &vLook, &vLook );
			Look( EtVector2( vLook.x, vLook.z ) );
			if( IsDie() ) SetActionQueue( "Die_Air", 0, 0.f );
			else SetActionQueue( "Hit_AirUp", 0, 0.f );
			if( m_HitParam.hWeapon ) {
				m_HitParam.hWeapon->OnHitSuccess( GetMySmartPtr(), CDnWeapon::Stun, vPosition, true );
			}
		}
		else {
			if( GetVelocity()->z != 0.f ) {
				SetVelocityZ( -GetVelocity()->z * fDecrease );
				SetResistanceZ( -GetResistance()->z );
			}
		}

	}
}

bool CDnActor::IsHittable( DnActorHandle hHitter, LOCAL_TIME LocalTime, HitStruct *pHitSignal, int iHitUniqueID /*= -1*/ )
{
#if !defined( _FINAL_BUILD )
	if( bIsIgnoreHit() ) return false;
#endif // #if !defined( _FINAL_BUILD )

	bool isSelfCheck = false;
	isSelfCheck = pHitSignal ? (pHitSignal->isSelfCheck?true:false) : false;

	//Only when self-check is not set (false by default; set to true only for specific skills like Wax..).
	//If set to true, also process Hit on self.
	if (false == isSelfCheck)
		if( hHitter == GetMySmartPtr() ) return false;
	
	if( IsDie() ) return false;
	
	CDnActorState::ActorTypeEnum actorType = GetActorType();

	if( actorType == CDnActorState::Npc ) return false;

	//ActorType [0:All/1:User/2:Monster]
	if (pHitSignal)
	{
		switch(pHitSignal->nActorType)
		{
		case 1:	//User (if target is a user but current actor is not a user)
			{
				if (!IsPlayerActor())
					return false;
			}
			break;
		case 2:	//Monster (if the target should be a monster but is not)
			{
				if (!IsMonsterActor())
					return false;
			}
			break;
		}
	}

	if( GetLastDamageHitUniqueID( hHitter->GetUniqueID() ) != -1 )
	{
		if( GetLastDamageHitUniqueID( hHitter->GetUniqueID() ) == iHitUniqueID )
			if( GetDamageRemainTime( hHitter->GetUniqueID() ) >= LocalTime ) 
				return false;
	}
	else if( GetDamageRemainTime( hHitter->GetUniqueID() ) >= LocalTime ) 
	{
		bool bIsChangedByInputAction = false;

		// #69997 Fix hit signal from previous action affecting the next action.
		if( m_nLastDamageHitterActionIndex > 0 && m_nLastDamageHitterActionIndex != hHitter->GetCurrentActionIndex() )
		{
			if( hHitter->GetProcessSkill() )
			{
				bIsChangedByInputAction = hHitter->GetProcessSkill()->IsChainInputAction( hHitter->GetCurrentAction() );
			}
		}

		if( bIsChangedByInputAction == false && hHitter->IsOnSignalFromChargerSE() == false )
		{
			return false;
		}
	}

	// As long as the hit signal interval and team are properly checked, if state effect 149 (Ice Prison) is active, it will always hit. //
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_149 ) && pHitSignal )
	{
		//Even in Ice Prison, check state effect filter first if one is applied
		if( pHitSignal->StateEffectFilter != 0 )
		{
			// Currently only Elemental Lord's Icy Fraction exceptionally filters 2 state effects
			// Want only one of the two to be hit, so prevent the first checked one from being caught in the next filtering. #28747
			// Process normally if no exception condition is hit.
			if( false == hHitter->CheckHitSignalStateEffectFilterException( GetUniqueID(), pHitSignal->StateEffectFilter) )
			{
				return false;
			}
			else
			if( false == m_pStateBlow->IsApplied( (STATE_BLOW::emBLOW_INDEX)pHitSignal->StateEffectFilter ) )
			{
				return false;
			}
		}

		int nOriginalTeam = GetOriginalTeam();
		int nHitterTeam = hHitter->GetTeam();
		//In Ice Prison state, must not be hit by enemies, and must be hit by allies.??
		if (nOriginalTeam != nHitterTeam)
			return false;
		else
		{
			//Case of hitting an enemy.. if it is self
			if (pHitSignal->nTargetType == 0 && GetMySmartPtr() == hHitter->GetMySmartPtr() ) 
				return false;
			
			//In Ice Prison state, must always be hit by allies..
			return true;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// If ignorecanhit is off and invincible state effect is active, do not take hits.
	if( pHitSignal )
	{
		if( pHitSignal->bIgnoreCanHit == TRUE )
		{
			DNVector(DnBlowHandle) vBlows;
			m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_099, vBlows );
			for( int i = 0; i < (int)vBlows.size(); ++i )
			{
				if( vBlows[i]->GetFloatValue() == -1 )
					return false;
			}
		}
		else if( m_pStateBlow->IsApplied(STATE_BLOW::BLOW_099) )
		{
			return false;
		}

#if !defined(PRE_FIX_73833)
		// If there is a weight limit, only get hit when weight is at or below the specified value.
		if( 0 < pHitSignal->nWeightValueLimit )
		{
			if( (float)pHitSignal->nWeightValueLimit < GetWeight() )
				return false;
		}
#endif
	}
	else if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_099 ) )
		return false;

	if( hHitter->GetProcessSkill() )
	{
		bool bResist = false;
		if( !IsHittableSkill( hHitter->GetProcessSkill()->GetClassID() , bResist ) )
		{
			if( bResist )
			{
				SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_IMMUNE , STATE_BLOW::BLOW_154 );
			}

			return false;
		}
	}

#if defined(PRE_ADD_50923)
	//If applied by invincibility state effect Type
	if ( m_pStateBlow->IsApplied(STATE_BLOW::BLOW_239))
	{
		DNVector(DnBlowHandle) vResult;
		m_pStateBlow->GetStateBlowFromBlowIndex(STATE_BLOW::BLOW_239, vResult);

		//Store invincibility Type values in a list.
		std::map<int, int> invincibleTypeList;
		for( UINT i=0 ; i<vResult.size() ; ++i ) 
		{
			DnBlowHandle hBlow = vResult.at( i );
			if( !hBlow )
				continue;

			CDnInvincibleTypeBlow* pInvincibleTypeBlow = static_cast<CDnInvincibleTypeBlow*>(hBlow.GetPointer());
			if (NULL == pInvincibleTypeBlow)
				continue;

			int nInvincibleTypeValue = pInvincibleTypeBlow->GetInvincibleTypeValue();
			invincibleTypeList.insert(std::make_pair(nInvincibleTypeValue, nInvincibleTypeValue));
		}

		if (hHitter->IsAppliedThisStateBlow(STATE_BLOW::BLOW_240))
		{
			DNVector(DnBlowHandle) vResultImmunByType;
			CDnStateBlow *pHitterStateBlow = hHitter->GetStateBlow(); 
			if (pHitterStateBlow)
				pHitterStateBlow->GetStateBlowFromBlowIndex(STATE_BLOW::BLOW_240, vResultImmunByType);

			for( UINT i=0 ; i<vResultImmunByType.size() ; ++i ) 
			{
				DnBlowHandle hBlow = vResultImmunByType.at( i );
				if( !hBlow )
					continue;

				CDnImmuneByType* pImmuneTypeBlow = static_cast<CDnImmuneByType*>(hBlow.GetPointer());
				if (NULL == pImmuneTypeBlow)
					continue;

				int nImmuneTypeValue = pImmuneTypeBlow->GetImmuneTypeValue();
				
				std::map<int, int>::iterator findIter = invincibleTypeList.find(nImmuneTypeValue);
				if (findIter != invincibleTypeList.end())
					invincibleTypeList.erase(findIter);
			}

			//Can only be hit if all invincibility Types are removed by ignore-invincibility state effects.
			//If list remains, there is no ignore state effect so it cannot be Hit.
			if (invincibleTypeList.empty() == false)
				return false;
		}
		//Cannot be hit unless the Hitter has a state effect that ignores the invincibility state effect Type.
		else
			return false;
	}
#endif // PRE_ADD_50923

	// Check if state effect filtering is specified in the action tool.
	if( pHitSignal )
	{
#ifdef PRE_ADD_HITDIRECTION
		EtVector3 vDir = *GetPosition() - *hHitter->GetPosition();
		EtVec3Normalize( &vDir, &vDir );
		switch( pHitSignal->CheckHitDirection )
		{
		// front
		case 1:
			{
				// Process hit when the target is facing the attacker.
				float fDot = EtVec3Dot( &vDir, &(m_Cross.m_vZAxis) );
				if( fDot >= -0.1f )	// Set the range in favor of the target being hit (typically the player).
					return false;
			}
			break;
		// back
		case 2:
			{
				// Process hit when the target has their back turned to the attacker.
				float fDot = EtVec3Dot( &vDir, &(m_Cross.m_vZAxis) );
				if( fDot < 0.1f )
					return false;
			}
			break;
		}
#endif // #ifdef PRE_ADD_HITDIRECTION

#if defined(PRE_ADD_57090)
		//////////////////////////////////////////////////////////////////////////
		//#57090 Feature to prevent a specific target from being attacked - cannot be hit while the configured state effect is active.
		//Check before StateEffectFilter...
		std::vector<string> vlTokens;
		string strArgument = pHitSignal->szUnStateEffectFilter ? pHitSignal->szUnStateEffectFilter : "";
		TokenizeA( strArgument, vlTokens, ";" );

		if (vlTokens.empty() == false)
		{
			int nTokenSize = (int)vlTokens.size();
			for (int i = 0; i < nTokenSize; ++i)
			{
				STATE_BLOW::emBLOW_INDEX emBlowIndex = (STATE_BLOW::emBLOW_INDEX)atoi(vlTokens[i].c_str());
				if( true == m_pStateBlow->IsApplied( emBlowIndex ) )
				{
					return false;
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
#endif // PRE_ADD_57090

		if( pHitSignal->StateEffectFilter != 0 )
		{
			// Currently only Elemental Lord's Icy Fraction exceptionally filters 2 state effects
			// Want only one of the two to be hit, so prevent the first checked one from being caught in the next filtering. #28747
			// Process normally if no exception condition is hit.
			if( false == hHitter->CheckHitSignalStateEffectFilterException( GetUniqueID(), pHitSignal->StateEffectFilter) )
			{
				return false;
			}
			else
			if( false == m_pStateBlow->IsApplied( (STATE_BLOW::emBLOW_INDEX)pHitSignal->StateEffectFilter ) )
			{
				return false;
			}
		}

#ifdef PRE_ADD_MONSTER_CATCH
		// If the check-grabbed-by-monster flag is on, distinguish only cases grabbed by a monster.
		if( TRUE == pHitSignal->bIsCatchedActor )
		{
			if( IsPlayerActor() )
			{
				CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(this);
				if( false == pPlayerActor->IsCatchedByMonster() )
				{
					return false;
				}
			}
		}
#endif // #ifdef PRE_ADD_MONSATER_CATCH
		
		// Determine whether nearby monsters are relic monsters summoned by me. Used in Shock of Relic skill.
		if( TRUE == pHitSignal->bUseMyRelicFilter )
		{
			bool bIsMyRelicMonster = hHitter->IsMyRelicMonster( GetMySmartPtr() );
			if( bIsMyRelicMonster )
				return true;
			else
				return false;
		}

		// Related to issue #13885.
		int nTargetType = pHitSignal->nTargetType;
		if( hHitter->IsAppliedThisStateBlow( STATE_BLOW::BLOW_086 ) )
		{
			// Only one team-reversal state effect applies.
			DNVector( DnBlowHandle ) vlReverseTeamBlows;
			hHitter->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_086, vlReverseTeamBlows );
			_ASSERT( false == vlReverseTeamBlows.empty() );
			if( false == vlReverseTeamBlows.empty() )
			{
				DnBlowHandle hBlow = vlReverseTeamBlows.front();
				CDnReverseTeamBlow* pReverseTeamBlow = static_cast<CDnReverseTeamBlow*>(hBlow.GetPointer());
				if( CDnReverseTeamBlow::HIT_SIGNAL_TARGET_CHANGE == pReverseTeamBlow->GetType() )
				{
					switch( nTargetType )
					{
						case 0: // Enemy -> Friend
							nTargetType = 1;
							break;

						case 1: // Friend -> Enemy
							{
								// #50166 Keep existing behavior for explosion-related signals.
								if(pHitSignal->fDamageProb > 0) 
									nTargetType = 1;
								else
									nTargetType = 0;
							}
							break;

						case 2: // All -> All
							{
								// #51871 Even for All type, configure not to affect when damage is present.
								if(pHitSignal->fDamageProb > 0) 
									nTargetType = 1;
							}
							break;
					}
				}
				else
				if( CDnReverseTeamBlow::HIT_SIGNAL_TARGET_ALL == pReverseTeamBlow->GetType() )
				{
					// Change hit target to all.
					nTargetType = 2;
				}
			}
		}

		switch( nTargetType )
		{
			case 0: // Enemy
				if( GetTeam() == hHitter->GetTeam() ) return false; 
				if( GetMySmartPtr() == hHitter->GetMySmartPtr() ) return false;
				if( false == pHitSignal->bIgnoreCanHit &&
					false == IsAppliedThisStateBlow( STATE_BLOW::BLOW_138 ) )
				{
					if( !CDnActorState::IsHittable() )
						return false;
				}
				break;

			case 1: // Friend
				
				if( GetTeam() != hHitter->GetTeam() ) return false;
				
#ifdef PRE_FIX_63315
				// #63315 - Added Hittable check even when TargetType is Friend.
				if( false == pHitSignal->bIgnoreCanHit &&
					false == IsAppliedThisStateBlow( STATE_BLOW::BLOW_138 ) )
				{
					if( !CDnActorState::IsHittable() )
						return false;
				}
#endif // PRE_FIX_63315
				break;

			case 2: // All
				// Fixed so enemies are not hit when CanHit signal is false even if hit signal targettype is all. (#10570)
				if( GetTeam() != hHitter->GetTeam() )
				{
					// If the ignore-CanHit flag is enabled on the Hit signal, unconditionally treat as hittable.
					// Same when state effect #138 is applied. (#19619)
					if( false == pHitSignal->bIgnoreCanHit &&
						false == IsAppliedThisStateBlow( STATE_BLOW::BLOW_138 ) )
					{
						if( !CDnActorState::IsHittable() )
							return false;
					}
				}
				break;
		}
	
	
		switch( pHitSignal->nStateCondition )
		{
			case 0:	// Normal
				if( GetState() == CDnActorState::Down ) return false;
				break;
			case 1:	// Down
				if( !( GetState() & CDnActorState::Down ) ) return false;
				break;
			case 2:	// Normal + Down
				break;
			case 3:	// All
				break;
		}
	}
	return true;
}

bool CDnActor::IsHittableSkill( int iSkillIndex , bool &bSendResist )
{
	// Invincibility handling for specific skills [2010/11/22 semozz]
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_154 ) )
	{
		DNVector(DnBlowHandle) vResult;
		m_pStateBlow->GetStateBlowFromBlowIndex(STATE_BLOW::BLOW_154, vResult);

		for( UINT i=0 ; i<vResult.size() ; ++i ) 
		{
			DnBlowHandle hBlow = vResult.at( i );
			if( hBlow )
			{
				CDNInvincibleAtBlow* pInvincibleAtBlow = static_cast<CDNInvincibleAtBlow*>(hBlow.GetPointer());
				if ( pInvincibleAtBlow && pInvincibleAtBlow->IsInvincibleAt(iSkillIndex) )
				{
					bSendResist = pInvincibleAtBlow->IsEnableSendResist();
					return false;
				}
			}
		}
	}

	return true;
}

void CDnActor::Show( bool bShow )
{
	m_bShow = bShow;
}

void CDnActor::ResetActor()
{
	Show( true );
	m_fDownDelta = 0.f;
	m_fDieDelta = 0.f;
	m_fStiffDelta = 0.f;
	m_mapLastDamageTime.clear();
	m_mapLastHitUniqueID.clear();
	m_nLastDamageHitterActionIndex = 0;
	m_LastHitSignalTime = 0;
	m_nLastHitSignalIndex = -1;

	m_bModifyPlaySpeed = false;
	m_PlaySpeedStartTime = 0;
	m_dwPlaySpeedTime = 0;

	m_bEnableNormalSuperAmmor = false;
	m_nNormalSuperAmmorTime = 0;
	m_fBreakNormalSuperAmmorDurability = 0.f;
//	CDnActionBase::SetFPS( 60.f );
	CDnActionBase::ResetActionBase();
	MAActorRenderBase::ResetActorRenderBase();

	ResetMove();
	ResetLook();

	SetVelocity( EtVector3( 0.f, 0.f, 0.f ) );
	SetResistance( EtVector3( 0.f, 0.f, 0.f ) );
	SetAddHeight( 0.f );

	SetMagnetLength( 0.f );
	SetMagnetDir( EtVector2( 0.f, 0.f ) );

	m_vlForceSettedHitElement.clear();

	if( IsCustomAction() ) ResetCustomAction();
	SetActionQueue( "Stand", 0, 0.f, 0.f, false, false );
}

void CDnActor::ProcessCollision( EtVector3 &vMove )
{
	m_pMovement->PushAndCollisionCheck( vMove );
}

void CDnActor::ProcessState( LOCAL_TIME LocalTime, float fDelta )
{
	CDnActorState::ProcessState( LocalTime, fDelta );

	// #48950 Reset MoveY signal value when super armor breaks.
	if( IsHit() )
	{
		ResetMoveYDistance();
	}

	if( m_pStateBlow &&
		false == IsNpcActor() )
	{
		m_pStateBlow->Process( LocalTime, fDelta );
		ProcessReservedRemoveBlows();
		ExecuteKillAfterProcessStateBlow();
	}
}

void CDnActor::SendAddSEFail( int iAddSEResult, STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	BYTE pBuffer[ 16 ];
	CPacketCompressStream Stream( pBuffer, 16 );

	Stream.Write( &iAddSEResult, sizeof(int) );

	Send( eActor::SC_ADD_STATE_EFFECT_DENIED, &Stream );
}

void CDnActor::PushSummonMonster( DnMonsterActorHandle hMonster, const SummonMonsterStruct* pSummonMonsterStruct, bool bReCreateFollowStageMonster/* = false*/ )
{
	//if( bReCreateFollowStageMonster )
	//	return;

	if( 0 == pSummonMonsterStruct->nGroupID )
	{
		m_listSummonMonster.push_back( hMonster );
	}
}

bool CDnActor::bIsCanSummonMonster( const SummonMonsterStruct* pSummonMonsterStruct )
{
	// 0 means no entity count limit!
	if( pSummonMonsterStruct->nMaxCount == 0 )
		return true;

	// If group ID is defined, find the list in the group map and process it.
	int iCount = 0;
	if( IsPlayerActor() && 0 < pSummonMonsterStruct->nGroupID )
	{
		if( 0 < m_mapSummonMonsterByGroup.count( pSummonMonsterStruct->nGroupID ) )
		{
			std::list<DnMonsterActorHandle>& listSummonedGroupMonster = m_mapSummonMonsterByGroup[ pSummonMonsterStruct->nGroupID ];
			for( std::list<DnMonsterActorHandle>::iterator itor=listSummonedGroupMonster.begin(); itor!=listSummonedGroupMonster.end(); )
			{
				DnMonsterActorHandle hMonster = *itor;
				if( hMonster )
				{
					++iCount;
					++itor;
				}
				else
					itor = listSummonedGroupMonster.erase( itor );
			}

			if( iCount < pSummonMonsterStruct->nMaxCount )
				return true;
		}
		else
			return true;
	}
	else
	{
		for( std::list<DnMonsterActorHandle>::iterator itor=m_listSummonMonster.begin() ; itor!=m_listSummonMonster.end() ; )
		{
			DnMonsterActorHandle hMonster = *itor;
			if( hMonster )
			{
				if( hMonster->GetMonsterClassID() == pSummonMonsterStruct->MonsterID )
					++iCount;

				++itor;
			}
			else
				itor = m_listSummonMonster.erase( itor );
		}

		if( iCount < pSummonMonsterStruct->nMaxCount )
			return true;
	}

	return false;
}

int CDnActor::AddStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex, const CDnSkill::SkillInfo* pParentSkill, 
							int nDurationTime, const char *szParam, bool bOnPlayerInit/* = false*/, bool bCheckCanBegin/* = true*/ , bool bEternity /*=false*/ )
{
	// Do not add state effect when dead, except on resurrection
	// In PvP round mode, when applying state effects while dead on round restart, bOnPlayerInit = true is passed so Die check is skipped. by Kimbap
	if( false == bOnPlayerInit )
		if( IsDie() && STATE_BLOW::emBLOW_INDEX::BLOW_057 != emBlowIndex ) return -1;

	if( !m_pStateBlow ) return -1;
	int iAddSEResult = m_pStateBlow->CanAddThisBlow( pParentSkill, emBlowIndex );
	switch( iAddSEResult ) 
	{	
		case CDnStateBlow::ADD_FAIL_BY_IMMUNE:
			{
				// Depending on the failure reason..
				SendAddSEFail( iAddSEResult, emBlowIndex );
				return -1;
			}
			break;

		case CDnStateBlow::ADD_FAIL_BY_DIED:
			OutputDebug( "[Add State Effect Failed by die]\n" );
			return -1;

		case CDnStateBlow::ADD_FAIL_BY_INVINCIBLE:
			OutputDebug( "[Add State Effect Failed by Invincible]\n" );
			return -1;

		case CDnStateBlow::ADD_FAIL_BY_REVIVAL:
			OutputDebug( "[Add State Effect Failed by Revival blow!]\n" );
			return -1;
		case CDnStateBlow::ADD_FAIL_BY_GUILDBLOW_PRIORITY:
			return -1;
		case CDnStateBlow::ADD_FAIL_BY_PROB_SKILL_INVINCIBLE:
			OutputDebug( "[Add State Effect Failed by Prob Skill Invincible]\n" );
			return -1;
		case CDnStateBlow::ADD_FAIL_BY_COMBOLIMITBLOW:
			OutputDebug( "[Add State Effect Failed by ComboLimitBlow]\n" );
			return -1;
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

	DnBlowHandle hBlow = m_pStateBlow->CreateStateBlow( GetMySmartPtr(), pParentSkill, emBlowIndex, nDurationTime, szParam );
	if( !hBlow ) 
		return -1;

	// State effects with a probability do not apply when blocked by the probability check.
	// Client also needs to know why adding state effect failed, so send reason in packet.
	if( bCheckCanBegin && (false == hBlow->CanBegin()) )
	{
		SendAddSEFail( CDnStateBlow::ADD_FAIL_BY_PROBABILITY, emBlowIndex );
		//hBlow->Release();

#if defined(PRE_FIX_49208)
		//#49208 If a duplicated state effect is deleted immediately here while OnBegin of an already applied state effect is called, the reference count gets mismatched.
		if (hBlow->IsDuplicated() == false)
#endif // PRE_FIX_49208
			SAFE_RELEASE_SPTR( hBlow );

		OutputDebug( "[Add State Effect Failed by probability fail!] BlowIndex: %d\n", emBlowIndex );
		return -1;
	}
	else
	{
		if( hBlow->IsDuplicated() )
			return hBlow->GetBlowID();
		else
		{
			hBlow->SetParentSkillInfo( pParentSkill );
			//hBlow->SetParentSkillID( iParentSkillID );
			//hBlow->SetParentSkillLevelID( iParentSkillLevelID );

			// Remove poison and burn state effects when entering invincibility state effect..
			if( hBlow->GetBlowIndex() == STATE_BLOW::BLOW_099 )
			{
				// Remove burn and poison state effects.
				if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_042 ) ||
					IsAppliedThisStateBlow( STATE_BLOW::BLOW_044 ) )
				{
					DNVector(DnBlowHandle) vlBlows;
					GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_042, vlBlows );
					GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_044, vlBlows );
					for( int i = 0; i < (int)vlBlows.size(); ++i )
					{
						DnBlowHandle hBlow = vlBlows.at( i );
						int iBlowID = hBlow->GetBlowID();

						CmdRemoveStateEffectFromID( iBlowID );
					}
				}
			}

			hBlow->SetEternity( bEternity );

			return m_pStateBlow->AddStateBlow( hBlow );
		}
	}
}

void CDnActor::DelStateBlow( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	if( !m_pStateBlow ) return;
	m_pStateBlow->RemoveStateBlowByBlowDefineIndex( emBlowIndex );
}

int CDnActor::RemoveStateBlowByBlowDefineIndex( STATE_BLOW::emBLOW_INDEX emBlowIndex )
{
	if( !m_pStateBlow )	return -1;
	return m_pStateBlow->RemoveStateBlowByBlowDefineIndex( emBlowIndex );
}

void CDnActor::RemoveStateBlowFromID( int nStateBlowID )
{
	if( !m_pStateBlow ) return;
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
		if( !hBlow )
			continue;
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

int CDnActor::GetNumAppliedStateBlow( void )
{ 
	return m_pStateBlow->GetNumStateBlow();
}

DnBlowHandle CDnActor::GetAppliedStateBlow( int iIndex )
{ 
	return m_pStateBlow->GetStateBlow( iIndex ); 
}

bool CDnActor::IsAppliedThisStateBlow( STATE_BLOW::emBLOW_INDEX iBlowIndex )
{
	if( !m_pStateBlow )
		return false;

	return m_pStateBlow->IsApplied( iBlowIndex );
}


void CDnActor::GetAllAppliedStateBlow(DNVector(DnBlowHandle)& /*IN OUT*/ out)
{
	int nSize = m_pStateBlow->GetNumStateBlow();
	if( 0 < nSize ) out.reserve( out.capacity() + nSize ); // 20080916 performance improvement
	for ( int i = 0 ; i < nSize ; i++ )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);
		if( hBlow && STATE_BLOW::STATE_END != hBlow->GetBlowState() )
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
		if( hBlow && hBlow->GetBlowIndex() == BlowIndex &&
			STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			out.push_back( hBlow );
	}
}

void CDnActor::OnSkillProjectile( CDnProjectile *pProjectile )
{
	MASkillUser::OnSkillProjectile( pProjectile );

	if( pProjectile )
	{
		if( IsProcessSkill()  || IsEnabledToggleSkill() )
		{		
			DnSkillHandle hSkill;
			if( IsProcessSkill() )
				hSkill = m_hProcessSkill;
			else
			if( IsEnabledToggleSkill() )
				hSkill = GetEnabledToggleSkill();

			_ASSERT( hSkill && "Projectile has no source skill" );

			pProjectile->SetParentSkill( hSkill );
			
			for( DWORD i = 0; i < hSkill->GetStateEffectCount(); i++ )
			{
				CDnSkill::StateEffectStruct *pStruct = hSkill->GetStateEffectFromIndex(i);
				if( pStruct->ApplyType == CDnSkill::ApplySelf ) continue;
				pProjectile->AddStateEffect( *pStruct );
			}

			if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_167 ) )
			{
				DNVector(DnBlowHandle) vlBlows;
				GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_167, vlBlows );
				if( false == vlBlows.empty() )
				{
					int iForceSkillLevel = (int)vlBlows.front()->GetFloatValue();
					pProjectile->SetSummonMonsterForceSkillLevel( iForceSkillLevel );
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//To process state effects for the Aero Shower skill, store the projectile in the current skill when a projectile is created while casting.
			//If the currently active skill has state effect 242
			if (hSkill)
			{
				bool bExistState = false;
				int nStateEffectCount = hSkill->GetStateEffectCount();
				for (int i = 0; i < nStateEffectCount; ++i)
				{
					CDnSkill::StateEffectStruct* pSE = hSkill->GetStateEffectFromIndex( i );
					if (pSE && (STATE_BLOW::emBLOW_INDEX)pSE->nID == STATE_BLOW::BLOW_242)
					{
						bExistState = true;
						break;
					}
				}

				if (bExistState == true)
				{
					LOCAL_TIME skillStartTime = hSkill->GetSkillStartTime();
					pProjectile->SetSkillStartTime(skillStartTime);	//Set the skill start time on the projectile as well.
					hSkill->AddProjectile(pProjectile);
				}
			}
		}

#if defined(PRE_FIX_68645)
		//If state effect #129 is present with a forced element setting, set it on the projectile.
		if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_129 ) )
		{
			DNVector(DnBlowHandle) vlhChangeActionSetBlow;
			m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_129, vlhChangeActionSetBlow );

			// There can be multiple action set change state effects.
			int iNumBlow = (int)vlhChangeActionSetBlow.size();
			for( int i = 0; i < iNumBlow; ++i )
			{
				if( !vlhChangeActionSetBlow[i] )
					continue;
				CDnChangeActionSetBlow* pChangeActionSetBlow = static_cast<CDnChangeActionSetBlow*>( vlhChangeActionSetBlow.at(i).GetPointer() );
				
				if (pChangeActionSetBlow->IsEnable() == true)
				{
					CDnChangeActionStrProcessor* pProcessor = pChangeActionSetBlow->GetChangeActionStrProcessor();	// Returns NULL when the action change trigger type is inactive.
					if( pProcessor && m_vlForceSettedHitElement.empty() == false)
					{	
						ElementEnum forceElement = (ElementEnum)m_vlForceSettedHitElement[0];

						pProjectile->SetForceHitElement(forceElement);
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////
#endif // PRE_FIX_68645
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

	// Moving while frozen state effect is active is rare, but remove it if so.
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_041 ) )
		m_pStateBlow->RemoveStateBlowByBlowDefineIndex( STATE_BLOW::BLOW_041 );

	RemoveNonAvailableStateBlow();

	// Note: Called on map transition. If the list of last-hit targets is not cleared here, from the previous stage
	// Previously hit targets may remain; running the Hit process in that state crashes.
	m_hVecLastHitList.clear();
	m_hVecLastHitListByRemainTime.clear();

	m_iCantMoveReferenceCount = 0;
	m_iCantActionReferenceCount = 0;

	// Update info related to the last hit signal
	m_mapLastDamageTime.clear();
	m_mapLastHitUniqueID.clear();
	m_nLastDamageHitterActionIndex = 0;

	RefreshState();
}

// If this function's logic changes, the SC_START_PVPROUND packet handler on the client must also be updated.
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

    // End active AutoPassive skill.
	EndAutoPassiveSkill( CDnActionBase::m_LocalTime, 0.0f );

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
	{
		OnSkillAura( m_hAuraSkill, false );
	}

	// End active toggle skill if any.
	if( m_hToggleSkill )
	{
		OnSkillToggle( m_hToggleSkill, false );
	}

	// Reset all state effects
	RemoveAllBlowExpectPassiveSkill();

	if( IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(this);
		pPlayer->RemoveAllBubbles( false );
	}


	m_hVecLastHitList.clear();
	m_hVecLastHitListByRemainTime.clear();

	// Update last hit signal info
	m_mapLastDamageTime.clear();
	m_mapLastHitUniqueID.clear();
	m_nLastDamageHitterActionIndex = 0;

	for( std::list<DnMonsterActorHandle>::iterator itor=m_listSummonMonster.begin() ; itor!=m_listSummonMonster.end() ; itor++ )
	{
		DnMonsterActorHandle hMonster = *itor;
		if( hMonster ) hMonster->CmdSuicide( false, false );
	}
	SAFE_DELETE_VEC( m_listSummonMonster );

	std::list<DnMonsterActorHandle> suicideMonsterList;

	map<int, list<DnMonsterActorHandle> >::iterator iter = m_mapSummonMonsterByGroup.begin();
	for( iter; iter != m_mapSummonMonsterByGroup.end(); ++iter )
	{	
		// Account for summoned monsters that expired and disappeared on their own, leaving only invalid handles in the list.
		list<DnMonsterActorHandle>& listSummonedMonsters = iter->second;
		list<DnMonsterActorHandle>::iterator iterList = listSummonedMonsters.begin();
		for( iterList; iterList != listSummonedMonsters.end(); )
		{
			if( *iterList )
			{
				DnMonsterActorHandle hMonster = *iterList;
				//PlayerActor::OnDieSummonedMonster is called inside CmdSuicide.
				//Removed from m_mapSummonMonsterByGroup/listSummonMonster lists in this function.
				//So collect the ones to CmdSuicide into a list and process them below..
				//hMonster->CmdSuicide( false, false );
				suicideMonsterList.push_back(hMonster);

				++iterList;
			}
			else
			{
				iterList = listSummonedMonsters.erase( iterList );
			}

			
		}
	}

	//Process entities queued in the list above for CmdSuicide.
	for( std::list<DnMonsterActorHandle>::iterator itor=suicideMonsterList.begin() ; itor!=suicideMonsterList.end() ; itor++ )
	{
		DnMonsterActorHandle hMonster = *itor;
		if( hMonster ) hMonster->CmdSuicide( false, false );
	}

	m_mapSummonMonsterByGroup.clear();
}


void CDnActor::ResetStateEffect( int nSkillDurationType )
{
	m_pStateBlow->ResetStateBlowBySkillType( nSkillDurationType );
}


void CDnActor::ProcessPress()
{
	if( m_nPressLevel == 0 ) return;
	if( IsDie() ) return;
	if( IsFly() ) return;
	if( IsUnderGround() ) return;

	DNVector(DnActorHandle) hVecList;
	if( m_iLastProcessPressCount > 0 )
		hVecList.reserve( m_iLastProcessPressCount );
	EtVector3 vAniDist = *GetPosition();
	vAniDist.x -= GetAniDistance()->x;
	vAniDist.z -= GetAniDistance()->z;

	float fScanRadius = 0.0f;
	if( m_Press == Press_Circle ) fScanRadius = (float)GetUnitSize() + 50.f;
	else if( m_Press == Press_Capsule ) fScanRadius = (float)GetUnitSize()*2.0f + 50.f;

	int nCount = ScanActor( GetRoom(), vAniDist, fScanRadius, hVecList );
	if( nCount > 0 ) {
		for( int i=0; i<nCount; i++ ) {
			if( hVecList[i] == GetMySmartPtr() ) continue;
			if( hVecList[i]->IsDie() ) continue;
			if( hVecList[i]->GetPressLevel() > m_nPressLevel ) continue;
			if( hVecList[i]->GetPressLevel() == -1 ) continue;
			if( m_Press == Press_NoneSameCircle && hVecList[i]->m_Press == Press_NoneSameCircle ) continue;

			ProcessPress( hVecList[i] );
		}
		m_iLastProcessPressCount = nCount;
	}
}

void CDnActor::ProcessPress( DnActorHandle hTarget )
{
	EtVector2 Dir;
	float Dist = 0.0f;
	bool bCheckPress = false;
	if( GetPress() == CDnActorState::Press_Circle && hTarget->GetPress() == CDnActorState::Press_Circle ) {
		if( CheckPressCircle2Clrcle2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			bCheckPress = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Capsule && hTarget->GetPress() == CDnActorState::Press_Capsule ) {
		if( CheckPressCapsule2Capsule2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			bCheckPress = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Capsule && hTarget->GetPress() == CDnActorState::Press_Circle ) {
		if( CheckPressCapsule2Circle2( GetMySmartPtr(), hTarget, Dir, Dist ) ) {
			hTarget->MoveToWorld( Dir * Dist );
			bCheckPress = true;
		}
	}
	else if( GetPress() == CDnActorState::Press_Circle && hTarget->GetPress() == CDnActorState::Press_Capsule ) {
		if( CheckPressCapsule2Circle2( hTarget, GetMySmartPtr(), Dir, Dist ) ) {
			hTarget->MoveToWorld( -Dir * Dist );
			bCheckPress = true;
		}
	}

	if( bCheckPress ) {
		if( hTarget->IsPlayerActor() ) {
			CDnPlayerActor *pPlayer = (CDnPlayerActor *)hTarget.GetPointer();
			if( pPlayer->GetPlayerSpeedHackChecker() )
				((CDnPlayerSpeedHackChecker*)pPlayer->GetPlayerSpeedHackChecker())->SetCheckPress( true, Dist );
		}
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

bool CDnActor::bIsObserver()
{
	if( GetGameRoom() && GetGameRoom()->bIsPvPRoom() )
	{
		if( GetTeam() == PvPCommon::Team::Observer )
			return true;
	}

	return false;
}

float CDnActor::SquaredDistance( const EtVector3& vPos, const SAABox& BBox, bool bNear )
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


void CDnActor::CmdChangeTeam( int nTeam )
{
	SetTeam( nTeam );

	printf("CDnActor::CmdChangeTeam %d\n",nTeam);

	char pBuffer[32];
	CPacketCompressStream Stream( pBuffer, sizeof(pBuffer) );
	Stream.Write( &m_nTeam, sizeof(int) );

	Send( eActor::SC_CMDCHANGETEAM, &Stream );
}

void CDnActor::OnHitFinish( LOCAL_TIME LocalTime, HitStruct *pStruct )
{
	if( !pStruct->vSelfVelocity || !pStruct->vSelfResistance ) return;
	if( GetVelocity() == NULL || GetResistance() == NULL ) return;
	if( EtVec3LengthSq( pStruct->vSelfVelocity ) == 0.f && EtVec3LengthSq( pStruct->vSelfResistance ) == 0.f ) return;

	EtVector3 vVelocity = *pStruct->vSelfVelocity;
	EtVector3 vResistance = *pStruct->vSelfResistance;

	// Correct y value since overwriting an existing y value here could cause issues.
	if( GetVelocity()->y != 0.f && pStruct->vSelfVelocity->y == 0.f ) {
		vVelocity.y = GetVelocity()->y;
		vResistance.y = GetResistance()->y;
	}

	CmdForceVelocity( vVelocity, vResistance );
}

void CDnActor::CmdForceVelocity( EtVector3 &vVelocity, EtVector3 &vResistance )
{
	SetVelocity( vVelocity );
	SetResistance( vResistance );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( GetPosition(), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( ( GetVelocity() ) ? GetVelocity() : &EtVector3( 0.f, 0.f, 0.f ), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );
	Stream.Write( ( GetResistance() ) ? GetResistance() : &EtVector3( 0.f, 0.f, 0.f ), sizeof(EtVector3), CPacketCompressStream::VECTOR3_BIT );

	Send( eActor::SC_CMDFORCEVELOCITY, &Stream );
}

void CDnActor::CmdChatBalloon( LPCWSTR wszMessage )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	int nLength = static_cast<int>(wcslen( wszMessage ) * 2);
	if( nLength > 120 ) nLength = 120;

	Stream.Write( &nLength, sizeof(int) );
	Stream.Write( wszMessage, nLength );

	Send( eActor::SC_CMDCHATBALLOON, &Stream );
}


void CDnActor::CmdShowExposureInfo( bool bShow )
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &bShow, sizeof(bool) );

	Send( eActor::SC_SHOWEXPOSUREINFO, &Stream );
}

void CDnActor::CmdForceAddSkill( int nSkillID )
{
	AddSkill( nSkillID );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nSkillID, sizeof(int) );
	Send( eActor::SC_FORCE_ADD_SKILL, &Stream );
}

void CDnActor::CmdForceRemoveSkill( int nSkillId )
{
	RemoveSkill( nSkillId );

	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );

	Stream.Write( &nSkillId, sizeof(int) );
	Send( eActor::SC_FORCE_REMOVE_SKILL, &Stream );
}

void CDnActor::OnProjectile( CDnProjectile *pProjectile, ProjectileStruct* pStruct, MatrixEx& LocalCross, int nSignalIndex )
{
	int iShootActionIndex = GetElementIndex( GetCurrentAction() );

	if( -1 != iShootActionIndex )
		pProjectile->SetShootActionIndex( iShootActionIndex );

	pProjectile->SetSignalArrayIndex( nSignalIndex );

	boost::shared_ptr<ProjectileStruct> pProjectileStruct = boost::shared_ptr<ProjectileStruct>(new ProjectileStruct);
#ifdef PRE_FIX_MEMOPT_SIGNALH
	CopyShallow_ProjectileStruct(*pProjectileStruct, pStruct);
#else
	*pProjectileStruct = *pStruct;
#endif

	pProjectile->SetProjectileSignal( pProjectileStruct );
}

void CDnActor::CmdSyncBlow( CDNUserSession* pGameSession )
{
	for( int i=0 ; i<GetNumAppliedStateBlow() ; ++i )
	{
		DnBlowHandle hBlow = GetAppliedStateBlow( i );
		if( hBlow )
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if( !pSkillInfo )
			{
				_DANGER_POINT();
				continue;
			}

			// #24143 Do not synchronize if it is a state effect from a passive buff skill.
			// Passive skills are in other users' skill lists received when a client drops in 
			// Automatically applied during initialization.
			if( CDnSkill::Passive == pSkillInfo->eSkillType &&
				CDnSkill::Buff == pSkillInfo->eDurationType &&
				CDnSkill::ApplySelf == pSkillInfo->eTargetType )
			{
				continue;
			}

			const CPacketCompressStream* pPacket = hBlow->GetPacketStream();
			Send( pGameSession, eActor::SC_CMDADDSTATEEFFECT, GetUniqueID(), const_cast<CPacketCompressStream*>(pPacket) );
		}
	}	
}

void CDnActor::OnDie( DnActorHandle hHitter )
{
	m_fDieDelta = m_fMaxDieDelta;
}

void CDnActor::UseMP( int iMPDelta )
{
	int iResult = m_pStateBlow->OnUseMP( iMPDelta );
	SetSP( GetSP() + iResult );

	RequestHPMPDelta( ElementEnum::ElementEnum_Amount, iResult, UINT_MAX, true );
}

void CDnActor::RequestDamageFromStateBlow( DnBlowHandle hFromBlow, int iDamage, CDnDamageBase::SHitParam* pHitParam/* = NULL*/ )
{
	// Ignore if invincibility state effect is active.
	if( hFromBlow && false == IsAppliedThisStateBlow( STATE_BLOW::BLOW_099 ) )
	{
		// Must not die while the Highlander state effect is active.
		float fDamage = (float)iDamage;
		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_143 ) )
		{
			// IsAppliedThisStateBlow() returns true, but duration has expired
			// State effects are not retrieved from GatherAppliedStateBlowByBlowIndex(), so the list may be empty.
			DNVector( DnBlowHandle ) vlhHighLanderBlow;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_143, vlhHighLanderBlow );
			if( 0 < (int)vlhHighLanderBlow.size() )
			{
				CDnHighlanderBlow* pHighlanderBlow = static_cast<CDnHighlanderBlow*>(vlhHighLanderBlow.front().GetPointer());
				fDamage += pHighlanderBlow->CalcDamage( fDamage );
			}
		}

		CDnSkill::SkillInfo* pParentSkillInfo = const_cast<CDnSkill::SkillInfo*>(hFromBlow->GetParentSkillInfo());
		DWORD dwHitterUniqueID = 0;
		ElementEnum eElement = ElementEnum::ElementEnum_Amount;
		if( pParentSkillInfo )
		{
			dwHitterUniqueID = pParentSkillInfo->hSkillUser ? pParentSkillInfo->hSkillUser->GetUniqueID() : -1;
			eElement =  pParentSkillInfo->eSkillElement;
		}

		if( IsAppliedThisStateBlow( STATE_BLOW::BLOW_243 ) )
		{
			float fAbsorbDamage = 0.f;
			int nAbsorbSP = 0;

			DNVector( DnBlowHandle ) vlhManaShieldBlow;
			GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_243, vlhManaShieldBlow );
			for( DWORD n=0; n<vlhManaShieldBlow.size(); n++ )
			{
				CDnManaShieldBlow* pManShieldBlow = static_cast<CDnManaShieldBlow*>(vlhManaShieldBlow[n].GetPointer());
				pManShieldBlow->CalcManaShield( fDamage , fAbsorbDamage , nAbsorbSP );
			}

			if( nAbsorbSP > GetSP() )
			{
				fAbsorbDamage -= (fAbsorbDamage * (nAbsorbSP - GetSP()) / nAbsorbSP);
				nAbsorbSP = GetSP();
			}

			if( fAbsorbDamage > fDamage )
				fAbsorbDamage = fDamage;

			fDamage -= fAbsorbDamage;
			SetSP( GetSP() -nAbsorbSP );
			RequestHPMPDelta( ElementEnum::ElementEnum_Amount, -nAbsorbSP, UINT_MAX, true , true );
		}

#if defined(PRE_FIX_59347)
		//For parts monster damage adjustment?
#ifdef PRE_FIX_67656
		ApplyPartsDamage(fDamage, pParentSkillInfo->hSkillUser);
#else
		if (GetApplyPartsDamage() == true)
			ApplyPartsDamage(fDamage, pParentSkillInfo->hSkillUser);
#endif

		//Display element
		if (pHitParam)
			eElement = pHitParam->HasElement;
#endif // PRE_FIX_59347

		SetHP( GetHP()-(INT64)fDamage );
#ifdef PRE_FIX_77172
		if( hFromBlow->GetBlowIndex() == STATE_BLOW::BLOW_042 )
			RequestHPMPDelta( eElement, -(INT64)fDamage, dwHitterUniqueID, false, true, pHitParam, true );
		else
			RequestHPMPDelta( eElement, -(INT64)fDamage, dwHitterUniqueID, false, true, pHitParam );
#else // PRE_FIX_77172
		RequestHPMPDelta( eElement, -(INT64)fDamage, dwHitterUniqueID, false, true, pHitParam );
#endif // PRE_FIX_77172

#ifdef PRE_ADD_EXPORT_DPS_INFORMATION
		if( CDnDPSReporter::IsActive() && pParentSkillInfo && pParentSkillInfo->hSkillUser && pParentSkillInfo->hSkillUser->IsPlayerActor() )
		{
			CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>( pParentSkillInfo->hSkillUser.GetPointer());

			if(CDnDPSReporter::GetInstance().IsEnabledUser( pPlayerActor->GetCharacterDBID() ))
			{
				CDnDPSReporter::GetInstance().StartReport();
				DNVector(DnActorHandle) hVecList;
				ScanActor( GetRoom(), *GetActorHandle()->GetPosition() , 500.f , hVecList );
				CDnDPSReporter::GetInstance().ApplyAreaMonsterCount( (int)hVecList.size() );
				CDnDPSReporter::GetInstance().ApplyDotDamageData( (int)fDamage , GetActorHandle() , hFromBlow );
				CDnDPSReporter::GetInstance().EndReport();
			}
		}
#endif

	}
}

void CDnActor::CmdFreezingPrisonDurablity(int nStateBlowID, float fDuabilityRate, bool bShowGauge)
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	Stream.Write( &nStateBlowID, sizeof(nStateBlowID) );
	Stream.Write( &fDuabilityRate, sizeof(fDuabilityRate) );
	Stream.Write( &bShowGauge, sizeof(bShowGauge) );

	Send( eActor::SC_FREEZINGPRISON_DURABILITY, &Stream );
}

void CDnActor::SendRemoveStateEffectFromID( int nID )
{
	char acBuffer[ 32 ];
	CPacketCompressStream Stream( acBuffer, sizeof(acBuffer) );
	Stream.Write( &nID, sizeof(nID) );

	Send( eActor::SC_CMDREMOVESTATEEFFECTFROMID, &Stream );
}

void CDnActor::RemoveResetStateBlow()
{
	// [2010/12/15 semozz]
	// Because it must be replaced with new state effects after calling this function
	// Must remove state effects immediately here.

	std::map<int, int>::iterator iter = m_vlStateBlowIDToRemove.begin();
	for (; iter != m_vlStateBlowIDToRemove.end(); ++iter)
	{
		//Immediately remove state effect and
		m_pStateBlow->RemoveImediatlyStateEffectFromID(iter->second);
	
		//Send state effect removal packet to client.
		SendRemoveStateEffectFromID(iter->second);
	}

	m_vlStateBlowIDToRemove.clear();
}


void CDnActor::ForceKnockBack(DnActorHandle hHitter, CDnDamageBase::SHitParam& HitParam)
{
	m_HitParam = HitParam;
	
	int nSeed = CRandom::Seed(GetRoom());
	_srand( GetRoom(), nSeed );

	CDnWeapon::HitTypeEnum HitType = CDnWeapon::Normal;
	
	//////////////////////////////////////////////////////////////////////////
	// Calculate stagger duration
	float fStiffResult = 0.f;
	if( GetStiffResistance() > 0 ) fStiffResult = ( hHitter->GetStiff() * m_HitParam.fStiffProb ) / GetStiffResistance();
	fStiffResult *= CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffRevision );

	if( fStiffResult <= 0.f ) fStiffResult = 0.05f;
	else if( fStiffResult > CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffMax ) ) 
		fStiffResult = CGlobalWeightTable::GetInstance().GetValue( CGlobalWeightTable::StiffMax );

	m_fStiffDelta = s_fMaxStiffTime * fStiffResult;

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Set random down delay value
	if( !IsDown() ) {
		m_fLastDownRatio = 1.f;
	}

	float fTemp = ( 3.f * ( 1.f - GetDownDelayProb() ) ) * m_fLastDownRatio;
	int nTemp = (int)( fTemp * DOWN_DELAY_RANDOM_RANGE * 100.f );
	if( nTemp == 0 ) m_fDownDelta = fTemp;
	else m_fDownDelta = fTemp - DOWN_DELAY_RANDOM_RANGE + ( _rand(GetRoom())%( nTemp * 2 ) / 100.f );
	if( m_fDownDelta >= 3.f ) m_fDownDelta = 3.f;
	if( m_fDownDelta <= 0.1f ) m_fDownDelta = 0.01f;
	//////////////////////////////////////////////////////////////////////////

	m_HitParam.HitType = HitType;
	// Damage calculation
	switch( HitType ) 
	{
	case CDnWeapon::Normal:
	case CDnWeapon::Critical:
	case CDnWeapon::Stun:
	case CDnWeapon::CriticalRes:
		{
			// Reduce acceleration according to weight.
			if( GetWeight() > 0.f )
				m_HitParam.vVelocity.y /= GetWeight();
			if( GetWeight() > 1.f ) 
			{
				float fTemp = m_HitParam.vVelocity.z;
				float fTest = CalcMovement( fTemp, 1.f, FLT_MAX, FLT_MIN, m_HitParam.vResistance.z );
				if( m_HitParam.vVelocity.z * fTest > 0.f ) 
				{
					m_HitParam.vVelocity.x /= powf( 1.1f, GetWeight() );
					m_HitParam.vVelocity.z /= powf( 1.1f, GetWeight() );
				}
			}

			bool bSuccessNormalDamage = true;
			int nSuperAmmorTime = 0;

			m_HitParam.bSuccessNormalDamage = bSuccessNormalDamage;
			m_HitParam.nSuperAmmorDelay = nSuperAmmorTime;

			memset( m_nSkillSuperAmmorValue, 0, sizeof(m_nSkillSuperAmmorValue) );
			OnBreakSkillSuperAmmor( m_HitParam );

			if( bSuccessNormalDamage ) {
				CheckDamageVelocity( hHitter );

				// Reset items to reset when hit
				MAMovementBase *pMovement = GetMovement();
				if( pMovement ) {
					pMovement->ResetMove();
					pMovement->ResetLook();
				}

				if( !m_HitParam.szActionName.empty() ) {
					SetActionQueue( m_HitParam.szActionName.c_str(), 0, 3.f, 0.f, false );
				}
			}
		}
		break;
	}
	
	INT64 nDamage = 0;

	RequestDamage(hHitter, nSeed, nDamage);
}

void CDnActor::RemoveNonAvailableStateBlow()
{
	if (NULL == m_pStateBlow)
		return;

	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(GetRoom());
	CDNUserSession *pUserSession = pGameRoom ? pGameRoom->GetUserSession(GetSessionID()) : NULL;

	if (!pUserSession)
		return;

	const TMapInfo* pMapData = g_pDataManager->GetMapInfo( pUserSession->GetMapIndex() );
	if (!pMapData)
		return;
	
	//Skip if world map
	if (GlobalEnum::MAP_WORLDMAP == pMapData->MapType)
		return;

	// Get the AllowType of the map the user is currently in..
	int nAllowMapType = pMapData->nAllowMapType;

	
	int nSize = m_pStateBlow->GetNumStateBlow();
	for ( int i = 0 ; i < nSize ; i++ )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);

		if( hBlow && STATE_BLOW::STATE_END != hBlow->GetBlowState() )
		{
			//Check whether the state effect is from an item skill
			//Get the used item ID.
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if (pSkillInfo && pSkillInfo->bIsItemSkill)
			{
				int nItemID = pSkillInfo->nItemID;

				const TItemData* pItemData = g_pDataManager->GetItemData(nItemID);
				if (pItemData)
				{
					//If the current map's AllowMapType differs from the item's AllowMapType setting
					if ((nAllowMapType & pItemData->nAllowMapType) == 0)
					{
						CmdRemoveStateEffectFromID(hBlow->GetBlowID());
					}
				}
			}
		}
	}
}

void CDnActor::RemovedActivatedStateBlow( bool bIgnoreItemSkill )
{
	if (NULL == m_pStateBlow)
		return;

	CDNGameRoom* pGameRoom = static_cast<CDNGameRoom*>(GetRoom());
	CDNUserSession *pUserSession = pGameRoom ? pGameRoom->GetUserSession(GetSessionID()) : NULL;

	if (!pUserSession)
		return;

	int nSize = m_pStateBlow->GetNumStateBlow();
	for ( int i = 0 ; i < nSize ; i++ )
	{
		DnBlowHandle hBlow = m_pStateBlow->GetStateBlow(i);

		if( hBlow && STATE_BLOW::STATE_END != hBlow->GetBlowState() )
		{
			const CDnSkill::SkillInfo* pSkillInfo = hBlow->GetParentSkillInfo();
			if( bIgnoreItemSkill == true && pSkillInfo && pSkillInfo->bIsItemSkill )
				continue;

			if ( hBlow->IsFromSourceItem() )
				continue;

			if( hBlow->IsEternity() )
				continue;

			if ( !bIsPassiveSkill( hBlow ) )
			{
				CmdRemoveStateEffectFromID(hBlow->GetBlowID());
			}
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
		if(!hSkill)
			continue;

		if (eSkillType == hSkill->GetSkillType())
			vlSkillList.push_back(hSkill);
	}
#else
	DWORD dwNumSkill = GetSkillCount();
	for( DWORD i = 0; i < dwNumSkill; ++i )
	{
		DnSkillHandle hSkill = GetSkillFromIndex( i );
		if( !hSkill )
			continue;

		if( eSkillType == hSkill->GetSkillType() )
			vlSkillList.push_back( hSkill );
	}
#endif // #ifndef PRE_FIX_SKILLLIST
}

void CDnActor::OnLoopAction( float fFrame, float fPrevFrame )
{
	/*
	if( m_nActionIndex == -1 || m_nVecAniIndexList[m_nActionIndex] == -1 ) return;
	MAActorRenderBase *pRenderBase = static_cast<MAActorRenderBase *>(this);
	if( !pRenderBase ) return;

	ActionElementStruct *pStruct = GetElement( m_nActionIndex );

	EtVector3 vDist;
	pRenderBase->CalcAniDistance( m_nVecAniIndexList[m_nActionIndex], min( (float)pStruct->dwLength, fFrame ), fPrevFrame, vDist );
	pRenderBase->AddAniDistance( vDist );
	*/
}

void CDnActor::CmdUpdateStateBlow(int nBlowID)
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwUniqueID = GetUniqueID();
	Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
	Stream.Write( &nBlowID, sizeof(nBlowID) );

	Send( eActor::SC_CMDUPDATESTATEBLOW, &Stream );
}

void CDnActor::CmdFinishAuraSkill(DWORD nSkillID)
{
	BYTE pBuffer[128];
	CPacketCompressStream Stream( pBuffer, 128 );
	DWORD dwUniqueID = GetUniqueID();
	Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
	Stream.Write( &nSkillID, sizeof(nSkillID) );

	Send( eActor::SC_FINISH_AURASKILL, &Stream );
}


bool CDnActor::IsImmuned(STATE_BLOW::emBLOW_INDEX blowIndex)
{
	CDnStateBlow *pStateBlow = GetStateBlow();
	if (!pStateBlow)
		return false;

	return pStateBlow->IsImmuned(blowIndex);
}

void CDnActor::SetAction( const char *szActionName, float fFrame, float fBlendFrame, bool bLoop /* = false */ )
{
	CDnActionBase::SetAction( CheckChangeActionBlow( szActionName ), fFrame, fBlendFrame, bLoop );

	//Reset HitLimitCount info when action changes
	m_HitLimitCountInfoList.clear();
}


const char *CDnActor::CheckChangeActionBlow( const char *szActionName )
{
	const char *szResultActionName = szActionName;
	// Move action also needs to change sometimes, so inevitably modified SetAction as well...
	// State effect #129: action name replacement
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_129 ) )
	{
		DNVector(DnBlowHandle) vlhChangeActionSetBlow;
		m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_129, vlhChangeActionSetBlow );

		// There can be multiple action set change state effects.
		int iNumBlow = (int)vlhChangeActionSetBlow.size();
		for( int i = 0; i < iNumBlow; ++i )
		{
			if( !vlhChangeActionSetBlow[i] )
				continue;

			CDnChangeActionSetBlow* pChangeActionSetBlow = static_cast<CDnChangeActionSetBlow*>( vlhChangeActionSetBlow.at(i).GetPointer() );
			if( STATE_BLOW::STATE_END != pChangeActionSetBlow->GetBlowState() )
			{
				CDnChangeActionStrProcessor* pProcessor = pChangeActionSetBlow->GetChangeActionStrProcessor();
				if( pProcessor )
				{
					// For checking game server dumps.
					string strNowActionName( szResultActionName );
					const char* pChangeActionName = pProcessor->GetChangeActionName( strNowActionName );
					if( pChangeActionName )
						szResultActionName = pChangeActionName;
				}
			}
		}
	}

	// If state effect #121 (stand action change) is active, switch to that action.
	if( m_pStateBlow->IsApplied( STATE_BLOW::BLOW_121 ) )
	{
		if( 0 == strcmp(szResultActionName, "Stand") )
		{
			DNVector(DnBlowHandle) vlhChangeStandActionBlow;
			m_pStateBlow->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_121, vlhChangeStandActionBlow );
			if( false == vlhChangeStandActionBlow.empty() )
			{
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
	if (hBlow)
	{
		hBlow->OnBegin( CDnActionBase::m_LocalTime, 0.0f );
		OnBeginStateBlow( hBlow );

		// Check because some blows set state to end immediately.
		if( STATE_BLOW::STATE_END != hBlow->GetBlowState() )
			hBlow->SetState( STATE_BLOW::STATE_DURATION );
	}
}

bool CDnActor::CanUsePrefixSkill()
{
	bool isCanUse = false;

	// Triggers if it is a normal attack or if the prefix skill activation flag is set on the skill. [2011/01/31 semozz]
	// [2011/02/11 semozz]
	// Remove prefix skill activation flag check
	// [2011/03/14 semozz]
	// Hacking Stance type skills have the ChangeStandAction state effect applied, so they are in a skill-using state even without m_hProcessSkill.
	// For now, treat it as not a normal attack if ChangeStandAction state effect is present.
	// Not applied if a state effect that ignores prefix state effects is active [2011/03/23 semozz]

	isCanUse = (false == IsProcessSkill() && //Normal attack
		!IsAppliedThisStateBlow(STATE_BLOW::BLOW_121) && !IsAppliedThisStateBlow(STATE_BLOW::BLOW_129) && //Hacking Stance type
		!IsAppliedThisStateBlow(STATE_BLOW::BLOW_183) //Ignore prefix
		/* || IsPrefixTriggerSkill()*/);

	return isCanUse;
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

void CDnActor::ReserveStealMagicBuff(DnActorHandle hActor, CDnSkill::SkillInfo *pSkillInfo, int nDurationTime, STATE_BLOW::emBLOW_INDEX blowIndex, const char* szValue)
{
	StateBlowInfo newBlowInfo;
	newBlowInfo.blowIndex = blowIndex;
	newBlowInfo.strValue = szValue ? szValue : "";

	STEAL_MAGIC_BUFF_INFO_LIST::iterator findIter = m_StealMagicBuffAddList.find(hActor);
	if (findIter != m_StealMagicBuffAddList.end())
	{
		findIter->second.stateBlowList.push_back(newBlowInfo);
	}
	else
	{
		StealMagicBuffInfo newInfo;
		newInfo.nDurationTime = nDurationTime;
		if (pSkillInfo)
			newInfo.skillInfo = *pSkillInfo;

		newInfo.stateBlowList.push_back(newBlowInfo);

		m_StealMagicBuffAddList.insert(STEAL_MAGIC_BUFF_INFO_LIST::value_type(hActor, newInfo));
	}
}

void CDnActor::UpdateStealMagicBuff()
{
	//Remove any existing state effect applied by Steal Magic, if present..
	if (!m_StealMagicBuffList.empty())
	{
		//When a state effect caused by Steal Magic is among the deleted state effects, remove the state effect stored in the list.
		STEAL_MAGIC_BUFF_LIST::iterator firstIter = m_StealMagicBuffList.begin();
		if (firstIter != m_StealMagicBuffList.end())
		{
			std::list<int>::iterator iter = firstIter->second.begin();
			for (; iter != firstIter->second.end(); )
			{
				int nBlowID = (*iter);
				
				std::map<int, int>::iterator findIter = m_RemovedStealMagicStateBlowIDList.find(nBlowID);
				if (findIter != m_RemovedStealMagicStateBlowIDList.end())
				{
					iter = firstIter->second.erase(iter);
					continue;
				}

				++iter;
			}
		}
	}

	m_RemovedStealMagicStateBlowIDList.clear();

	//If no state effect is reserved by Steal Magic, skip...
	if (m_StealMagicBuffAddList.empty())
		return;

	//Remove any existing state effect applied by Steal Magic, if present..
	if (!m_StealMagicBuffList.empty())
	{
		//Remove state effects that are still applied..
		STEAL_MAGIC_BUFF_LIST::iterator firstIter = m_StealMagicBuffList.begin();
		if (firstIter != m_StealMagicBuffList.end())
		{
			std::list<int>::iterator iter = firstIter->second.begin();
			std::list<int>::iterator endIter = firstIter->second.end();
			for (; iter != endIter; ++iter)
			{
				int nBlowID = (*iter);
				
				//Immediately remove state effect and
				m_pStateBlow->RemoveImediatlyStateEffectFromID(nBlowID);

				//Send state effect removal packet to client.
				SendRemoveStateEffectFromID(nBlowID);
			}

			firstIter->second.clear();
		}

		m_StealMagicBuffList.clear();
	}

	//Apply reserved Steal Magic state effect
	//Pick one randomly..
	int nCount = (int)m_StealMagicBuffAddList.size();
	int nSelectIndex = rand() % nCount;

	STEAL_MAGIC_BUFF_INFO_LIST::iterator selectedIter = m_StealMagicBuffAddList.begin();
	for (int i = 0; i < nSelectIndex; ++i)
		++selectedIter;

	if (selectedIter != m_StealMagicBuffAddList.end())
	{
		StealMagicBuffInfo &buffInfo = selectedIter->second;

		STATE_BLOW_INFO_LIST::iterator iter = selectedIter->second.stateBlowList.begin();
		STATE_BLOW_INFO_LIST::iterator endIter = selectedIter->second.stateBlowList.end();
		
		for (; iter != endIter; ++iter)
		{
			int nBlowID = CmdAddStateEffect(&buffInfo.skillInfo, iter->blowIndex, buffInfo.nDurationTime, iter->strValue.c_str(), false, false);
			
			AddStealMagicStateBlow(buffInfo.skillInfo.iSkillID, nBlowID);
		}
	}
	m_StealMagicBuffAddList.clear();
}

void CDnActor::AddStealMagicStateBlow(int nSkillID, int nBlowID)
{
	STEAL_MAGIC_BUFF_LIST::iterator findIter = m_StealMagicBuffList.find(nSkillID);
	if (findIter != m_StealMagicBuffList.end())
	{
		findIter->second.push_back(nBlowID);
	}
	else
	{
		std::list<int> blowIDList;
		blowIDList.push_back(nBlowID);

		m_StealMagicBuffList.insert(STEAL_MAGIC_BUFF_LIST::value_type(nSkillID, blowIDList));
	}
}

void CDnActor::RemoveStealMagicStateBlow(int nDeletedBlowID)
{
	//If there is state effect info added by Steal Magic
	if (!m_StealMagicBuffList.empty())
	{
		//Currently Steal Magic only happens on one actor, so only check the one at begin..
		STEAL_MAGIC_BUFF_LIST::iterator firstIter = m_StealMagicBuffList.begin();
		if (firstIter != m_StealMagicBuffList.end())
		{
			std::list<int>::iterator iter = firstIter->second.begin();
			std::list<int>::iterator endIter = firstIter->second.end();
			for (; iter != firstIter->second.end(); )
			{
				// If an entry with the same BlowID as the state effect being removed is found, remove it from the list.
				if (nDeletedBlowID == (*iter))
				{
					m_RemovedStealMagicStateBlowIDList.insert(std::map<int, int>::value_type(nDeletedBlowID, nDeletedBlowID));
					break;
				}

				++iter;
			}
		}
	}

}


void CDnActor::OnEndStateBlow( DnBlowHandle hBlow )
{

	//Code to remove from the list when a state effect is removed if it was applied by Steal Magic
	if (!hBlow)
		return;

	int nDeletedBlowID = hBlow->GetBlowID();
	RemoveStealMagicStateBlow(nDeletedBlowID);

}

#ifdef PRE_ADD_MONSTER_CATCH
bool CDnActor::CatchCalcSuperArmor( DnActorHandle hCatcherActor, int iSuperArmorDamage )
{
	bool bResult = false;

	// Apply super armor damage; grabbed if <= 0.
	// Apply damage, and if super armor remains, slow down frames and pass. Currently only 0 is used for super armor.
	m_nSkillSuperAmmorValue[ 0 ] -= iSuperArmorDamage;
	if( m_nSkillSuperAmmorValue[ 0 ] < 0 )
	{
		bResult = true;
	}
	else
	{
		// Remove the frame slowdown for now.
		//// Withstood with super armor.
		//int iSuperArmorTime = m_nSkillSuperAmmorTime;
		//int iDelay = (int)( iSuperArmorTime * ( m_fStiffDelta / s_fMaxStiffTime ) );
		//if( iDelay > 0 ) 
		//{
		//	SetPlaySpeed( (DWORD)( iSuperArmorTime * ( m_fStiffDelta / s_fMaxStiffTime ) ), 0.03f );
		//}
	}

	return bResult;
}
#endif // #ifdef PRE_ADD_MONSTER_CATCH

bool CDnActor::CheckCollisionHitCondition(const EtVector3& vObjPos, const MatrixEx& objCross, EtVector3 &vTargetPos, float angleAllow)
{
	EtVector3 vDir = vTargetPos - vObjPos;
	vDir.y = 0.f;

	EtVec3Normalize(&vDir, &vDir);

	EtVector3 vZVec = objCross.m_vZAxis;
	float fDeg = EtToDegree(acos(EtVec3Dot(&vZVec, &vDir)));
	if (fDeg > angleAllow)
		return false;

	return true;
}

DnMonsterActorHandle CDnActor::FindOldSummonMonster(const SummonMonsterStruct* pSummonMonsterStruct)
{
	DnMonsterActorHandle hSelectedSummon;

	LOCAL_TIME tempTime = 0x7FFFFFFFFFFFFFFF;
	if( 0 < pSummonMonsterStruct->nGroupID )
	{
		if( 0 < m_mapSummonMonsterByGroup.count( pSummonMonsterStruct->nGroupID ) )
		{
			std::list<DnMonsterActorHandle>& listSummonedGroupMonster = m_mapSummonMonsterByGroup[ pSummonMonsterStruct->nGroupID ];
			for( std::list<DnMonsterActorHandle>::iterator itor=listSummonedGroupMonster.begin(); itor!=listSummonedGroupMonster.end(); ++itor)
			{
				DnMonsterActorHandle hMonster = *itor;

				//If Group is set, do not compare MonsterID; select the summoned monster in the group with the oldest summon time.
				if (!hMonster)
					continue;

				LOCAL_TIME requestSummonTime = hMonster->GetRequestSummonTime();
				if( requestSummonTime < tempTime)
				{
					hSelectedSummon = hMonster;
					tempTime = requestSummonTime;
				}
			}
		}
	}
	else
	{
		for( std::list<DnMonsterActorHandle>::iterator itor=m_listSummonMonster.begin() ; itor!=m_listSummonMonster.end(); ++itor)
		{
			DnMonsterActorHandle hMonster = *itor;
			if (!hMonster || hMonster->GetMonsterClassID() != pSummonMonsterStruct->MonsterID)
				continue;

			LOCAL_TIME requestSummonTime = hMonster->GetRequestSummonTime();
			if( requestSummonTime < tempTime)
			{
				hSelectedSummon = hMonster;
				tempTime = requestSummonTime;
			}
		}
	}

	return hSelectedSummon;
}

void CDnActor::RequestActionChange(int nActionIndex)
{
	BYTE pBuffer[ 16 ];
	CPacketCompressStream Stream( pBuffer, 16 );

	Stream.Write( &nActionIndex, sizeof(int) );
	
	Send( eActor::SC_ACTION_CHANGE, &Stream );
}

const std::list<DnMonsterActorHandle>& CDnActor::GetSummonedMonsterList( void )
{
	for( std::list<DnMonsterActorHandle>::iterator itor=m_listSummonMonster.begin() ; itor!=m_listSummonMonster.end() ; )
	{
		if( !(*itor) )
		{
			itor = m_listSummonMonster.erase( itor );
		}
		else
		{
			++itor;
		}
	}

	return m_listSummonMonster;
}

const std::map<int, std::list<DnMonsterActorHandle> >& CDnActor::GetGroupingSummonedMonsterList( void )
{
	map<int, list<DnMonsterActorHandle> >::iterator iter = m_mapSummonMonsterByGroup.begin();
	for( iter; iter != m_mapSummonMonsterByGroup.end(); ++iter )
	{	
		list<DnMonsterActorHandle>& listSummonedMonsters = iter->second;
		list<DnMonsterActorHandle>::iterator iterList = listSummonedMonsters.begin();
		for( iterList; iterList != listSummonedMonsters.end(); )
		{
			if( !(*iterList) )
			{
				iterList = listSummonedMonsters.erase( iterList );
			}
			else
			{
				++iterList;
			}
		}
	}

	return m_mapSummonMonsterByGroup;
}

void CDnActor::SendChainAttackProjectile(DnActorHandle hRootAttacker, DWORD dwPrevAttackerActorUniqueID, int iActionIndex, int iProjectileSignalArrayIndex, DnActorHandle hActorToAttack, int iSkillID)
{
	if (!hRootAttacker || !hActorToAttack)
		return;

	BYTE pBuffer[ 256 ];
	CPacketCompressStream Stream( pBuffer, 256 );

	//
	DWORD dwRootAttackerID = hRootAttacker->GetUniqueID();
	DWORD dwActorToAttackID = hActorToAttack->GetUniqueID();

	Stream.Write(&dwRootAttackerID, sizeof(DWORD));
	Stream.Write(&dwPrevAttackerActorUniqueID, sizeof(DWORD));
	Stream.Write(&dwActorToAttackID, sizeof(DWORD));

	Stream.Write(&iActionIndex, sizeof(int));
	Stream.Write(&iProjectileSignalArrayIndex, sizeof(int));
	Stream.Write(&iSkillID, sizeof(int));
		
	Send( eActor::SC_CHAINATTACK_PROJECTILE, &Stream );
}

void CDnActor::RemoveStateEffectByHitSignal(HitStruct* pHitStruct)
{
	if (NULL == pHitStruct)
		return;

	//Modified to ignore RemoveStateIndex per request to disable RemoveStateIndex feature.
#if defined(PRE_FIX_51988)
	if (pHitStruct->szRemoveStateIndexList == NULL)
		return;
#else
	if (pHitStruct->RemoveStateIndex == 0 && pHitStruct->szRemoveStateIndexList == NULL)
		return;
#endif // PRE_FIX_51988

#if defined(PRE_FIX_51988)
	std::string str = pHitStruct->szRemoveStateIndexList ? pHitStruct->szRemoveStateIndexList : "";
#else
	std::string str = FormatA("%d;%s", pHitStruct->RemoveStateIndex, pHitStruct->szRemoveStateIndexList ? pHitStruct->szRemoveStateIndexList : "");
#endif // PRE_FIX_51988
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	for (int i = 0; i < (int)tokens.size(); ++i)
	{

		STATE_BLOW::emBLOW_INDEX eBlowIndex = (STATE_BLOW::emBLOW_INDEX)(atoi(tokens[i].c_str()));

		if (eBlowIndex == STATE_BLOW::emBLOW_INDEX::BLOW_NONE)
			continue;

		//Send packet...
		SendRemoveStateEffect(eBlowIndex);

		//Delete directly right here.
		m_pStateBlow->RemoveImediatlyStateEffectByBlowIndex(eBlowIndex);
	}
}

bool CDnActor::ProcessIgnoreHitSignal()
{
	bool bIgnoreHitSignal = false;
	DnBlowHandle hIgnoreHitSignalBlow;
	if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_228))
	{
		DNVector(DnBlowHandle) vecBlowList;
		GatherAppliedStateBlowByBlowIndex(STATE_BLOW::BLOW_228, vecBlowList);
		for (int i = 0; i < (int)vecBlowList.size(); ++i)
		{
			DnBlowHandle hBlow = vecBlowList[i];
			if (hBlow && hBlow->CanBegin())
			{
				bIgnoreHitSignal = true;
				hIgnoreHitSignalBlow = hBlow;
				break;
			}
		}
	}

	//If ignore hit signal is set, send a packet to display the state effect VFX and skip further damage processing.
	if (hIgnoreHitSignalBlow && bIgnoreHitSignal == true)
	{
		BYTE pBuffer[128];
		CPacketCompressStream Stream( pBuffer, 128 );

		DWORD dwUniqueID = GetUniqueID();
		STATE_BLOW::emBLOW_INDEX stateBlowIndex = hIgnoreHitSignalBlow->GetBlowIndex();
		bool bShowEffect = true;

		Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
		Stream.Write( &stateBlowIndex, sizeof(stateBlowIndex));
		Stream.Write( &bShowEffect, sizeof(bShowEffect));

		Send(eActor::SC_SHOW_STATE_EFFECT, &Stream);

		return true;
	}

	return false;
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

void CDnActor::AddSkillStateEffect(DnSkillHandle hSkill)
{
	m_ApplySkillStateEffectList.push_back(hSkill);
}

void CDnActor::ApplySkillStateEffect()
{
	if (m_ApplySkillStateEffectList.empty())
		return;

	std::vector<DnSkillHandle>::iterator iter = m_ApplySkillStateEffectList.begin();
	for (; iter != m_ApplySkillStateEffectList.end(); ++iter)
	{
		DnSkillHandle hSkill = *(iter);
		if (hSkill)
		{
			DWORD stateCount = hSkill->GetStateEffectCount();
			CDnStateBlow *pStateBlow = GetStateBlow();
			const CDnSkill::SkillInfo* skillInfo = hSkill->GetInfo();
			if (pStateBlow)
			{
				for (DWORD dwIndex = 0; dwIndex < stateCount; ++dwIndex)
				{
					CDnSkill::StateEffectStruct *pStateEffect = hSkill->GetStateEffectFromIndex(dwIndex);
					if (pStateEffect == NULL || pStateEffect->ApplyType != CDnSkill::StateEffectApplyType::ApplySelf)
						continue;

					CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;
					eResult = pStateBlow->CanApplySkillStateEffect(skillInfo, *pStateEffect);

					if (CDnSkill::CanApply::Fail != eResult)
					{
						RemoveResetStateBlow();

						int nBlowID = CmdAddStateEffect( skillInfo, (STATE_BLOW::emBLOW_INDEX)pStateEffect->nID, 
							pStateEffect->nDurationTime, pStateEffect->szValue.c_str(), false, false );

						//If it is a state effect without duration, register it to the skill.
						if (-1 != nBlowID && pStateEffect->nDurationTime == -1)
						{
							hSkill->AddSignalApplyNoPacketStateEffectHandle(nBlowID);
						}
					}
				}
			}
		}
	}

	m_ApplySkillStateEffectList.clear();
}

void CDnActor::SendProbInvincibleSuccess( void )
{
	BYTE pBuffer[ 4 ] = { 0 };
	CPacketCompressStream Stream( pBuffer, 4 );

	Send( eActor::SC_PROBINVINCIBLE_SUCCESS, &Stream );
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

void CDnActor::ApplyStateEffectSignalProcess( const S_NO_PACKET_SELF_STATEBLOW &StateBlowInfo, ApplyStateEffectStruct* pStruct )
{
	// Check whether StateBlow is used first [2010/11/11 semozz]
	if (false == StateBlowInfo.bUsed)
	{
		CDnSkill::CanApply eResult = CDnSkill::CanApply::Apply;

		// [2010/11/11 semozz]
		// Check if the StateBlow of the registered skill here is also usable before calling CmdAddStateEffect.
		// Usable at check time in CDnSkill::OnBegin, but another StateBlow may already be registered before this call.
		// This issue occurs when using heal from #20008 simultaneously.
		eResult = m_pStateBlow->CanApplySkillStateEffect(&StateBlowInfo.ParentSkillInfo, StateBlowInfo.StateEffect);

		if( CDnSkill::CanApply::Fail != eResult )
		{
			// [2010/12/08 semozz]
			// Items stored in the reset state list in CanApplySkillStateEffect at skill use time
			// Remove here.
			RemoveResetStateBlow();

			// Note Hanki: State effects applied immediately to self for the skill action duration when skill activates 
			// Since the skill object knows when to remove it, add it to the skill object's nopacket state effect list.
			// The client also automatically adds and removes nopacket status effects when the skill is executed.
			// A self-buff state effect with a duration has a fixed duration, so when time runs out
			// Since it is removed automatically, use the existing system to call CmdAddStateEffect. It also sends the packet automatically. In this case, without triggering the state effect on the client
			// Wait for packet from server.
			if( StateBlowInfo.StateEffect.ApplyType == CDnSkill::ApplySelf && 
				StateBlowInfo.StateEffect.nDurationTime == -1 /*&& // Removed this condition since it can also be a buff/debuff/aura, etc. 
															  CDnSkill::Instantly == StateBlowInfo.ParentSkillInfo.eDurationType*/ )
			{
				// [2010/12/09 semozz]
				// Must send the add state effect packet.
				int iBlowID = CmdAddStateEffect( &StateBlowInfo.ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)StateBlowInfo.StateEffect.nID, 
					StateBlowInfo.StateEffect.nDurationTime, StateBlowInfo.StateEffect.szValue.c_str() );

				if( -1 != iBlowID )
				{
					if( m_hProcessSkill )
					{
						//_ASSERT( m_hProcessSkill && -1 != iBlowID );
						// Block state effect (#30) was changed to have no duration, so do not add it here as an exception.
						// When added, it ends together the moment the action finishes. 2010.09.20
						if( !((-1 == StateBlowInfo.StateEffect.nDurationTime) && 
							(STATE_BLOW::BLOW_030 == StateBlowInfo.StateEffect.nID)) )
							m_hProcessSkill->AddSignalApplyNoPacketStateEffectHandle( iBlowID );
					}
					else
						if( m_hAuraSkill && CDnSkill::Aura == StateBlowInfo.ParentSkillInfo.eDurationType )
						{
							m_hAuraSkill->AddSignalApplyNoPacketStateEffectHandle( iBlowID );
						}
#ifdef _DEBUG
						else
							_ASSERT( !"Failed to register state effect signal timing" );
#endif

				}
			}
			else
			{
				// Self state effect with duration
				int iBlowID = CmdAddStateEffect( &StateBlowInfo.ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)StateBlowInfo.StateEffect.nID, 
					StateBlowInfo.StateEffect.nDurationTime, StateBlowInfo.StateEffect.szValue.c_str() );
			}
		}
		// Clear reset list when skill usage ends [2010/12/09 semozz]
		InitStateBlowIDToRemove();

		// Need to check if it should count as used even if the skill cannot be used [2010/11/11 semozz]
		const_cast<S_NO_PACKET_SELF_STATEBLOW&>(StateBlowInfo).Used();
	}
	else
	{
		OutputDebug( "[Error!!] State effect apply signal reused state effect index (%d)!!\n", pStruct->StateEffectIndex );
		_ASSERT( !"State effect apply signal reused an already-used state effect index" );
	}
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

void CDnActor::SetState(ActorStateEnum State)
{
#if defined(PRE_FIX_59939)
	//SetState is called every frame, but since curse state effect (244) controls its own cooldown, just call it...
	//
	CDnActorState::SetState(State);

	bool isAttackState = IsAttack();
	if (isAttackState)
		OnAttackChange();

#else
	bool isPreAttackState = IsAttack();

	CDnActorState::SetState(State);

	bool isNewAttackState = IsAttack();
	if (isPreAttackState == false && isNewAttackState == true)
		OnAttackChange();
#endif // PRE_FIX_59939
}

void CDnActor::OnAttackChange()
{
	if (IsAppliedThisStateBlow(STATE_BLOW::BLOW_244))
	{
		DNVector(DnBlowHandle) vlBlows;
		GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_244, vlBlows );

		int nBlowCount = (int)vlBlows.size();
		for (int i = 0; i < nBlowCount; ++i)
		{
			DnBlowHandle hBlow = vlBlows[i];
			if (hBlow)
			{
				CDnCurseBlow* pCurseBlow = dynamic_cast<CDnCurseBlow*>(hBlow.GetPointer());
				if (pCurseBlow)
					pCurseBlow->OnAttackChange();
			}
		}
	}
}

int CDnActor::ExceptionHitList(DNVector(DnActorHandle) &hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct)
{
	std::map<DWORD, DnActorHandle> hOwnerActorList;		//List of candidate owner actors with puppet state effect
	std::map<DWORD, DnActorHandle> eraseActorList;		//List of actors to be removed
	std::vector<CDnMonsterActor*> hSummonActorList;		//Summoned monster actor list

	DNVector(DnActorHandle) hNewVecList;				//New actor list

	int nWeaponLength = 0;
	bool bOnDamageCalled = false;
	if (hHiterActor)
	{
		if( pStruct->bIncludeWeaponLength && hHiterActor->GetWeapon() ) 
			nWeaponLength = hHiterActor->GetWeapon()->GetWeaponLength();
	}

	MatrixEx CrossTemp = Cross;

	// Because for scaled monsters the hit signal's center axis does not always match the actor's origin position 
	// Scale value must also be applied accurately to offset (#18971)
	EtVector3 vScale = EtVector3(1.0f, 1.0f, 1.0f);
	if (hHiterActor)
		vScale = *hHiterActor->GetScale();

	EtVector3 vOffset = *pStruct->vOffset * vScale.y;

	CrossTemp.MoveLocalZAxis( vOffset.z );
	CrossTemp.MoveLocalXAxis( vOffset.x );
	CrossTemp.MoveLocalYAxis( vOffset.y );
	
	// If linked to a specific bone..
	if( hHiterActor &&
		pStruct->szLinkBoneName && strlen(pStruct->szLinkBoneName) > 0 )
	{
		int nBoneIndex = hHiterActor->GetBoneIndex( pStruct->szLinkBoneName );
		if( -1 != nBoneIndex )
		{
			EtMatrix matBoneWorld = hHiterActor->GetBoneMatrix( pStruct->szLinkBoneName );
			EtMatrixMultiply( (EtMatrix*)&CrossTemp, (EtMatrix*)&CrossTemp, &matBoneWorld );
		}
	}
	// Monsters need to be adjusted according to scale.
	float fDistanceMax = pStruct->fDistanceMax * vScale.y;
	float fDistanceMin = pStruct->fDistanceMin * vScale.y;
	float fHeightMax = pStruct->fHeightMax * vScale.y;
	///////////////////////////////////////////////

	EtVector3 vPos = CrossTemp.m_vPosition;
	
	float fDistance = max( fDistanceMax, fHeightMax - pStruct->fHeightMin ) + nWeaponLength;
	float fXZDistanceSQ = fDistanceMax + nWeaponLength;
	float fXZDistanceMinSQ = fDistanceMin; // Min is not affected by Weapon length.

	fXZDistanceSQ *= fXZDistanceSQ;
	fXZDistanceMinSQ *= fXZDistanceMinSQ;

	EtVector3 vDir;
	EtVector3 vZVec = Cross.m_vZAxis;

	if( pStruct->fCenterAngle != 0.f ) 
	{
		EtMatrix matRotate;
		EtMatrixRotationY( &matRotate, EtToRadian( pStruct->fCenterAngle ) );
		EtVec3TransformNormal( &vZVec, &vZVec, &matRotate );
	}
	SAABox Box;
	float fDot = 0.0f;

#if defined(PRE_FIX_59680)
	int nMyTeamID = -1;
	if (hHiterActor)
	{
		nMyTeamID = hHiterActor->GetTeam();
		if (hHiterActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
			nMyTeamID = hHiterActor->GetOriginalTeam();
	}
#endif // PRE_FIX_59680

	int nListCount = (int)hVecList.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnActorHandle hActor = hVecList[i];
		if (!hActor)
			continue;

		switch( hActor->GetHitCheckType() ) 
		{
		case HitCheckTypeEnum::BoundingBox: 
			{
				vDir = *hActor->GetPosition() - vPos;
				vDir.y = 0.f;

				hActor->GetBoundingBox( Box );

				if( SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
				if( SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

				EtVec3Normalize( &vDir, &vDir );
				fDot = EtVec3Dot( &vZVec, &vDir );
				if( EtToDegree( acos( fDot ) ) > pStruct->fAngle ) continue;

				if( Box.Min.y < vPos.y + pStruct->fHeightMin && 
					Box.Max.y < vPos.y + pStruct->fHeightMin ) continue;
				if( Box.Min.y > vPos.y + fHeightMax && 
					Box.Max.y > vPos.y + fHeightMax ) continue;
			}
			break;
		case HitCheckTypeEnum::Collision:
			{
				SCollisionCapsule				Capsule;
				SCollisionResponse				CollisionResult;
				DNVector(SCollisionResponse)	vCollisionResult;
				//
				Capsule.Segment.vOrigin = vPos;
				float fHeight = fHeightMax - pStruct->fHeightMin;
				Capsule.Segment.vOrigin.y = Capsule.Segment.vOrigin.y - ( pStruct->fHeightMin + ( fHeight / 2.f ) );
				Capsule.Segment.vDirection = EtVector3( 0.f, fHeight / 2.f, 0.f );
				Capsule.fRadius = ( fDistanceMax + nWeaponLength );

				//int nParentBoneIndex = -1;
				EtVector3 vDestPos;
				if( hActor->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == false ) 
					continue;
			}
		}

		
		//Build candidate list of owner actors with puppet state effect..
		if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_247))
		{
			hOwnerActorList.insert(std::make_pair(hActor->GetUniqueID(), hActor));
		}
		//Build list of monsters summoned as puppets..
		else if (hActor->IsMonsterActor())
		{
			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
			if (pMonsterActor && 
				pMonsterActor->IsSummonedMonster() &&
				pMonsterActor->IsPuppetSummonMonster())
			{
				
#if defined(PRE_FIX_59680)
				//For Relic of Heal skill, the relic is set to the same team.
				//During Relic Hit signal processing, Summon Puppet is added to the list and the owner actor is excluded below, causing heal to fail.
				//Summon puppets on the same team must be excluded from this list.
				int nTargetTeam = hActor->GetTeam();
				if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
					nTargetTeam = hActor->GetOriginalTeam();

				//Hiter actor may be absent in a projectile, e.g. for a Prop (add handling when Hiter actor is absent)
				if (hHiterActor)
				{
					//If the Summon Puppet is on a different team, add it to the summon actor list and
					//If on the same team, add the summon puppet actor to the removal list so it gets removed from the final list.
					if (nMyTeamID != nTargetTeam)
						hSummonActorList.push_back(pMonsterActor);
					else
						eraseActorList.insert(std::make_pair(hActor->GetUniqueID(), hActor));
				}
				else
					hSummonActorList.push_back(pMonsterActor);
#else
				hSummonActorList.push_back(pMonsterActor);
#endif // PRE_FIX_59680
			}
		}
	}

	//When a summon monster actor exists
	//Check if the summon monster actor's master actor exists, and if so
	//Add the owner actor to the deletion list.
	int nSummonActorList = (int)hSummonActorList.size();
	for (int iIndex = 0; iIndex < nSummonActorList; ++iIndex)
	{
		CDnMonsterActor* pSummonMonsterActor = hSummonActorList[iIndex];

		//Check the master actor of the summon actor.
		DnActorHandle hOwnerActor = pSummonMonsterActor->GetSummonerPlayerActor();
		DWORD dwOwnerUniqueID = hOwnerActor ? hOwnerActor->GetUniqueID() : -1;
		if (dwOwnerUniqueID == -1)
			continue;

		//Find actor in the owner-candidate actor list.
		std::map<DWORD, DnActorHandle>::iterator findIter = hOwnerActorList.find(dwOwnerUniqueID);
		if (findIter != hOwnerActorList.end())
		{
			//Add to deleted actor list.
			eraseActorList.insert(std::make_pair(dwOwnerUniqueID, hOwnerActor));
		}
	}


	nListCount = (int)hVecList.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnActorHandle hActor = hVecList[i];
		if (!hActor)
			continue;

		//Iterate over original actor list; if not found in the remove-actor list, add to the new list.
		DWORD dwOwnerUniqueID = hActor->GetUniqueID();
		std::map<DWORD, DnActorHandle>::iterator findIter = eraseActorList.find(dwOwnerUniqueID);
		if (findIter == eraseActorList.end())
			hNewVecList.push_back(hActor);
	}

	std::swap(hVecList, hNewVecList);

	return (int)hVecList.size();
}

//Build a list of only actors inside the Hit area from the scanned actor list.
void CDnActor::CheckHitAreaActorList(DNVector(DnActorHandle)& hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct, int nCheckType, float fScanLength, EtVector3& vPrePos)
{
	DNVector(DnActorHandle) hNewVecList;				//New actor list

	int nWeaponLength = 0;
	bool bOnDamageCalled = false;
	if (hHiterActor)
	{
		if( pStruct->bIncludeWeaponLength && hHiterActor->GetWeapon() ) 
			nWeaponLength = hHiterActor->GetWeapon()->GetWeaponLength();
	}

	MatrixEx CrossTemp = Cross;

	// Because for scaled monsters the hit signal's center axis does not always match the actor's origin position 
	// Scale value must also be applied accurately to offset (#18971)
	EtVector3 vScale = EtVector3(1.0f, 1.0f, 1.0f);
	if (hHiterActor)
		vScale = *hHiterActor->GetScale();

	EtVector3 vOffset = *pStruct->vOffset * vScale.y;

	CrossTemp.MoveLocalZAxis( vOffset.z );
	CrossTemp.MoveLocalXAxis( vOffset.x );
	CrossTemp.MoveLocalYAxis( vOffset.y );

	// If linked to a specific bone..
	if( pStruct->szLinkBoneName && strlen(pStruct->szLinkBoneName) > 0 )
	{
		int nBoneIndex = hHiterActor->GetBoneIndex( pStruct->szLinkBoneName );
		if( -1 != nBoneIndex )
		{
			EtMatrix matBoneWorld = hHiterActor->GetBoneMatrix( pStruct->szLinkBoneName );
			EtMatrixMultiply( (EtMatrix*)&CrossTemp, (EtMatrix*)&CrossTemp, &matBoneWorld );
		}
	}
	// Monsters need to be adjusted according to scale.
	float fDistanceMax = pStruct->fDistanceMax * vScale.y;
	float fDistanceMin = pStruct->fDistanceMin * vScale.y;
	float fHeightMax = pStruct->fHeightMax * vScale.y;
	///////////////////////////////////////////////

	EtVector3 vPos = CrossTemp.m_vPosition;

	float fDistance = max( fDistanceMax, fHeightMax - pStruct->fHeightMin ) + nWeaponLength;
	float fXZDistanceSQ = fDistanceMax + nWeaponLength;
	float fXZDistanceMinSQ = fDistanceMin; // Min is not affected by Weapon length.

	fXZDistanceSQ *= fXZDistanceSQ;
	fXZDistanceMinSQ *= fXZDistanceMinSQ;

	EtVector3 vDir;
	EtVector3 vZVec = Cross.m_vZAxis;

	if( pStruct->fCenterAngle != 0.f ) 
	{
		EtMatrix matRotate;
		EtMatrixRotationY( &matRotate, EtToRadian( pStruct->fCenterAngle ) );
		EtVec3TransformNormal( &vZVec, &vZVec, &matRotate );
	}
	SAABox Box;
	float fDot = 0.0f;

	int nListCount = (int)hVecList.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnActorHandle hActor = hVecList[i];
		if (!hActor)
			continue;

		if( !hActor->GetObjectHandle() )
			continue;

		switch( hActor->GetHitCheckType() ) 
		{
		case HitCheckTypeEnum::BoundingBox: 
			{
				switch(nCheckType)
				{
				case 0:	//Check method in CDnActor::OnSignal.		
					{
						vDir = *hActor->GetPosition() - vPos;
						vDir.y = 0.f;

						hActor->GetBoundingBox( Box );

						if( SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
						if( SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

						EtVec3Normalize( &vDir, &vDir );
						fDot = EtVec3Dot( &vZVec, &vDir );
						if( EtToDegree( acos( fDot ) ) > pStruct->fAngle ) continue;

						if( Box.Min.y < vPos.y + pStruct->fHeightMin && 
							Box.Max.y < vPos.y + pStruct->fHeightMin ) continue;
						if( Box.Min.y > vPos.y + fHeightMax && 
							Box.Max.y > vPos.y + fHeightMax ) continue;
					}
					break;
				case 1:	//First check method for Projectile.
					{
						float fPropContactDistance = FLT_MAX;
						float fNowFrameActorContactDistanceSQ = FLT_MAX;

						vDir = *hActor->GetPosition() - vPos;
						vDir.y = 0.f;

						hActor->GetBoundingBox( Box );

						if( CDnActor::SquaredDistance( vPos, Box ) > fXZDistanceSQ ) continue;
						if( CDnActor::SquaredDistance( vPos, Box, false ) < fXZDistanceMinSQ ) continue;

						EtVec3Normalize( &vDir, &vDir );
						fDot = EtVec3Dot( &vZVec, &vDir );
						if( EtToDegree( acos( fDot ) ) > pStruct->fAngle ) continue;

						if( Box.Min.y < vPos.y + pStruct->fHeightMin && 
							Box.Max.y < vPos.y + pStruct->fHeightMin ) continue;
						if( Box.Min.y > vPos.y + pStruct->fHeightMax && 
							Box.Max.y > vPos.y + pStruct->fHeightMax ) continue;
						fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( vPos - vPrePos ) );
						if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;
					}
					break;
				case 2: //Second check method for Projectile
					{
						bool bResult = false;
						float fPropContactDistance = FLT_MAX;
						float fNowFrameActorContactDistanceSQ = FLT_MAX;

						SCollisionResponse				Response;
						DNVector(SCollisionResponse)	vResponse;

						float fThickness = max( fabs( pStruct->fHeightMin ), fabs( pStruct->fHeightMax ) );
						
						SSegment Segment;
						Segment.vOrigin = vPrePos;
						Segment.vDirection = Cross.m_vZAxis * fScanLength;

						SCollisionCapsule Capsule;
						Capsule.Segment = Segment;
						Capsule.fRadius = fThickness;

						
						if( fThickness == 0.f ) bResult = hActor->GetObjectHandle()->FindSegmentCollision( Segment, Response );
						else bResult = hActor->GetObjectHandle()->FindCapsuleCollision( Capsule, Response );

						fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( *hActor->GetPosition() - vPrePos ) );
						if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;

						if (bResult == false)
							continue;
					}
					break;
				}
			}
			break;
		case HitCheckTypeEnum::Collision:
			{
				switch(nCheckType)
				{
				case 0://CDnActor OnSignal check method.
					{
						SCollisionCapsule				Capsule;
						SCollisionResponse				CollisionResult;
						DNVector(SCollisionResponse)	vCollisionResult;
						//
						Capsule.Segment.vOrigin = vPos;
						float fHeight = fHeightMax - pStruct->fHeightMin;
						Capsule.Segment.vOrigin.y = Capsule.Segment.vOrigin.y - ( pStruct->fHeightMin + ( fHeight / 2.f ) );
						Capsule.Segment.vDirection = EtVector3( 0.f, fHeight / 2.f, 0.f );
						Capsule.fRadius = ( fDistanceMax + nWeaponLength );

						//int nParentBoneIndex = -1;
						EtVector3 vDestPos;
						if( hActor->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == false ) 
							continue;
					}
					break;
				case 1://First check method for Projectile..
					{
						SCollisionCapsule				Capsule;
						SCollisionResponse				CollisionResult;
						DNVector(SCollisionResponse)	vCollisionResult;
						//
						Capsule.Segment.vOrigin = vPos;
						float fHeight = pStruct->fHeightMax - pStruct->fHeightMin;
						Capsule.Segment.vOrigin.y = Capsule.Segment.vOrigin.y - ( pStruct->fHeightMin + ( fHeight / 2.f ) );
						Capsule.Segment.vDirection = EtVector3( 0.f, fHeight / 2.f, 0.f );
						Capsule.fRadius = pStruct->fDistanceMax;

						//int nParentBoneIndex = -1;
						EtVector3 vDestPos;
						if( hActor->GetObjectHandle()->CEtCollisionEntity::FindCapsuleCollision( Capsule, CollisionResult, &vCollisionResult ) == false ) 
							continue;
					}
					break;
				case 2://Second check method for Projectile..
					{
						float fPropContactDistance = FLT_MAX;
						float fNowFrameActorContactDistanceSQ = FLT_MAX;

						bool bResult = false;
						float fThickness = max( fabs( pStruct->fHeightMin ), fabs( pStruct->fHeightMax ) );
						SSegment Segment;
						Segment.vOrigin = vPrePos;
						Segment.vDirection = Cross.m_vZAxis * fScanLength;

						SCollisionCapsule Capsule;
						Capsule.Segment = Segment;
						Capsule.fRadius = fThickness;

						SCollisionResponse				Response;
						DNVector(SCollisionResponse)	vResponse;

						int nParentBoneIndex = -1;
						EtVector3 vDestPos;
						if( fThickness == 0.f ) 
							bResult = hActor->GetObjectHandle()->FindSegmentCollision( Segment, Response, &vResponse );
						else
							bResult = hActor->GetObjectHandle()->FindCapsuleCollision( Capsule, Response, &vResponse );
						
						if (bResult)
						{
							fNowFrameActorContactDistanceSQ = EtVec3LengthSq( &EtVector3( Segment.vDirection * Response.fContactTime ) );
							if( fNowFrameActorContactDistanceSQ > fPropContactDistance ) continue;
						}
						else
							continue;
					}
					break;
				}
			}
		}

		hNewVecList.push_back(hActor);
	}

	std::swap(hVecList, hNewVecList);
}

int CDnActor::ExceptionHitList2(DNVector(DnActorHandle) &hVecList, MatrixEx& Cross, DnActorHandle hHiterActor, HitStruct* pStruct, 
								DNVector(DnActorHandle)& hAddStateEffectActorList, 
								int nCheckType, float fScanLength, EtVector3& vPrePos)
{
	//Add only actors within the hit area to the list.
	CDnActor::CheckHitAreaActorList(hVecList, Cross, hHiterActor, pStruct, nCheckType, fScanLength, vPrePos);

	int nMyTeamID = -1;
	if (hHiterActor)
	{
		nMyTeamID = hHiterActor->GetTeam();
		if (hHiterActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
			nMyTeamID = hHiterActor->GetOriginalTeam();
	}

	struct _PuppetNOwnerInfo
	{
		DWORD dwOwnerID;
		CDnMonsterActor* pPuppetMonster;
		DnActorHandle hPuppetActor;
		DnActorHandle hOwnerActor;
	};

	std::map<DWORD, _PuppetNOwnerInfo> PuppetnOnwerInfoList;

	DNVector(DnActorHandle) hNewVecList;				//New actor list
	
	EtVector3 vPos = *hHiterActor->GetPosition();

	int nListCount = (int)hVecList.size();
	for (int i = 0; i < nListCount; ++i)
	{
		DnActorHandle hActor = hVecList[i];
		if (!hActor)
			continue;

		bool bSeperated = false;

		//Build candidate list of owner actors with puppet state effect..
		if (hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_247))
		{
			bSeperated = true;
			DWORD dwOwnerUniqueID = hActor->GetUniqueID();

			std::map<DWORD, _PuppetNOwnerInfo>::iterator findIter = PuppetnOnwerInfoList.find(dwOwnerUniqueID);
			if (findIter == PuppetnOnwerInfoList.end())
			{
				_PuppetNOwnerInfo _info;
				_info.dwOwnerID = dwOwnerUniqueID;
				_info.hOwnerActor = hActor;

				PuppetnOnwerInfoList.insert(std::make_pair(dwOwnerUniqueID, _info));
			}
			else
			{
				_PuppetNOwnerInfo& _info = findIter->second;
				_info.hOwnerActor = hActor;
			}
		}
		//Build list of monsters summoned as puppets..
		else if (hActor->IsMonsterActor())
		{
			CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
			if (pMonsterActor && 
				pMonsterActor->IsSummonedMonster() &&
				pMonsterActor->IsPuppetSummonMonster())
			{
				bSeperated = true;

				DnActorHandle hOwnerActor = pMonsterActor->GetSummonerPlayerActor();
				DWORD dwOwnerUniqueID = hOwnerActor ? hOwnerActor->GetUniqueID() : -1;

				std::map<DWORD, _PuppetNOwnerInfo>::iterator findIter = PuppetnOnwerInfoList.find(dwOwnerUniqueID);
				if (findIter == PuppetnOnwerInfoList.end())
				{
					_PuppetNOwnerInfo _info;
					_info.dwOwnerID = dwOwnerUniqueID;
					_info.hPuppetActor = hActor;
					_info.pPuppetMonster = pMonsterActor;

					PuppetnOnwerInfoList.insert(std::make_pair(dwOwnerUniqueID, _info));
				}
				else
				{
					_PuppetNOwnerInfo& _info = findIter->second;
					_info.hPuppetActor = hActor;
					_info.pPuppetMonster = pMonsterActor;
				}
			}
		}

		//Pre-store actors not included in the owner actor / summon actor list into a new list.
		if (bSeperated == false)
			hNewVecList.push_back(hActor);
	}


	std::map<DWORD, _PuppetNOwnerInfo>::iterator iter = PuppetnOnwerInfoList.begin();
	std::map<DWORD, _PuppetNOwnerInfo>::iterator endIter = PuppetnOnwerInfoList.end();
	for (; iter != endIter; ++iter)
	{
		_PuppetNOwnerInfo& _info = iter->second;

		if (_info.hOwnerActor && _info.hPuppetActor)
		{
			/*
			//Add the nearest actor.
			//If the owner actor is excluded, add it to the excluded actor list (for adding state effects)
			EtVector3 vOwnerDiff = (*_info.hOwnerActor->GetPosition()) - vPos;
			EtVector3 vPuppetDiff = (*_info.hPuppetActor->GetPosition()) - vPos;

			float fOwnerLength = EtVec3LengthSq(&vOwnerDiff);
			float fPuppetLength = EtVec3LengthSq(&vPuppetDiff);
			*/

			int nTargetTeam = _info.hOwnerActor->GetTeam();
			if (_info.hOwnerActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_149))
				nTargetTeam = _info.hOwnerActor->GetOriginalTeam();
			
			/*
			//If on the same team, add the owner actor regardless of distance
			//If owner actor distance is farther, add summon actor to the list and add owner actor to the excluded actor list
			if (nMyTeamID != nTargetTeam && fOwnerLength > fPuppetLength)
			*/


			//Add the summon actor to the hit list if within the same hit range regardless of distance.
			if (hHiterActor &&
				nMyTeamID != nTargetTeam)
			{
				hNewVecList.push_back(_info.hPuppetActor);

				hAddStateEffectActorList.push_back(_info.hOwnerActor);
			}
			else
			{
				hNewVecList.push_back(_info.hOwnerActor);
			}
			
		}
		//Add to list if not a summon actor / master actor pair (only one of the two exists)
		else
		{
			DnActorHandle hActor = _info.hOwnerActor ? _info.hOwnerActor : _info.hPuppetActor;
			if (hActor)
				hNewVecList.push_back(hActor);
		}
	}

	std::swap(hVecList, hNewVecList);

	return (int)hVecList.size();	
}

void CDnActor::ScanActorByStateIndex(DNVector(DnActorHandle) &Veclist, STATE_BLOW::emBLOW_INDEX blowIndex)
{
	DNVector(DnActorHandle) scanActorList;
	ScanActor(GetRoom(), *GetPosition(), FLT_MAX, scanActorList);
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
					DnActorHandle hSkillActor = GetMySmartPtr();
					if (IsMonsterActor())
					{
						CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hSkillActor.GetPointer());
						if (pMonsterActor)
							hSkillActor = pMonsterActor->GetSummonerPlayerActor();
					}

					if (pSkillInfo && pSkillInfo->hSkillUser == hSkillActor)
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
	m_Cross.MoveLocalXAxis( m_vRotAxis.x );
	m_Cross.MoveLocalYAxis( m_vRotAxis.y );
	m_Cross.MoveLocalZAxis( m_vRotAxis.z );

	// Rotation.
	m_Cross.RotateYaw( (-m_fSpeedRot) );
	
}

#if defined(PRE_FIX_61382)
//Return owner actor if it is a puppet monster.
DnActorHandle CDnActor::GetOwnerActorHandle(DnActorHandle hActor)
{
	DnActorHandle hOwnerActor = hActor;

	if (hActor && hActor->IsMonsterActor())
	{
		CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(hActor.GetPointer());
		if (pMonsterActor && pMonsterActor->IsPuppetSummonMonster())
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

	//Find monsters matching monsterID among summoned monsters.
	std::list<DnMonsterActorHandle> monsterActorList;
	FindSummonMonster(monsterID, monsterActorList);

	std::list<DnMonsterActorHandle>::iterator iter = monsterActorList.begin();
	std::list<DnMonsterActorHandle>::iterator endIter = monsterActorList.end();
	for (; iter != endIter; ++iter)
	{
		DnMonsterActorHandle hMonsterActor = (*iter);

		//Information must not be erased while iterating through the monster actor list..
		RemoveSummonMonsterStateEffectByGlyph(hMonsterActor, monsterID, glyphID);
	}

	//Ensure actual info is removed here.
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
				std::list<_StateEffectInfo>::iterator iter = stateEffectIDList.begin();
				std::list<_StateEffectInfo>::iterator endIter = stateEffectIDList.end();

				for (; iter != endIter; ++iter)
				{
					_StateEffectInfo& info = (*iter);

					if (hMonsterActor && hMonsterActor->GetUniqueID() == info.dwUniqueID)
						hMonsterActor->CmdRemoveStateEffectFromID(info.nStateEffectID);
				}
			}
		}
	}
}

//Called from PlayerActor::OnDieSummonMonster when the actual monster dies.
//Since the monster is going to be removed, probably no need to find and erase state effects one by one..
//Remove only configured info..
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

void CDnActor::FindSummonMonster(int monsterID, std::list<DnMonsterActorHandle>& monsterActorList)
{
	std::map<int, std::list<DnMonsterActorHandle> >::iterator iter = m_mapSummonMonsterByGroup.begin();
	std::map<int, std::list<DnMonsterActorHandle> >::iterator endIter = m_mapSummonMonsterByGroup.end();

	for (; iter != endIter; ++iter)
	{
		std::list<DnMonsterActorHandle>::iterator monsterIter = iter->second.begin();
		std::list<DnMonsterActorHandle>::iterator monsterEndIter = iter->second.end();
		for (; monsterIter != monsterEndIter; ++monsterIter)
		{
			DnMonsterActorHandle hMonster = *monsterIter;

			if (!hMonster)
				continue;

			if (hMonster->GetMonsterClassID() == monsterID)
			{
				monsterActorList.push_back(hMonster);
			}
		}
	}
	
	{
		for( std::list<DnMonsterActorHandle>::iterator itor=m_listSummonMonster.begin() ; itor!=m_listSummonMonster.end(); ++itor)
		{
			DnMonsterActorHandle hMonster = *itor;
			if (!hMonster || hMonster->GetMonsterClassID() != monsterID)
				continue;

			monsterActorList.push_back(hMonster);
		}
	}
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


void CDnActor::ResetDamageRemainTime()
{
	m_mapLastDamageTime.clear();
	m_nLastDamageHitterActionIndex = 0;
}

LOCAL_TIME CDnActor::GetDamageRemainTime( DWORD dwActorUniqueID ) 
{ 
	map<DWORD, LOCAL_TIME>::iterator iter = m_mapLastDamageTime.find( dwActorUniqueID );
	if( m_mapLastDamageTime.end() != iter )
		return iter->second;
	else
		return 0;
}

void CDnActor::SetDamageRemainTime( DWORD dwActorUniqueID, LOCAL_TIME LastDamageTime, int nCurrentActionIndex ) 
{ 
	m_mapLastDamageTime[ dwActorUniqueID ] = LastDamageTime; 
	m_nLastDamageHitterActionIndex = nCurrentActionIndex;
}

void CDnActor::SetLastDamageHitUniqueID( DWORD dwActorUniqueID, int iLastHitUniqueID ) 
{ 
	m_mapLastHitUniqueID[ dwActorUniqueID ] = iLastHitUniqueID; 
}

int CDnActor::GetLastDamageHitUniqueID( DWORD dwActorUniqueID ) 
{
	if( m_mapLastHitUniqueID.end() != m_mapLastHitUniqueID.find( dwActorUniqueID ) )
		return m_mapLastHitUniqueID[ dwActorUniqueID ];
	else
		return -1;
};