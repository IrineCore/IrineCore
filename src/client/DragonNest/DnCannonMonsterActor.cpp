#include "StdAfx.h"
#include "DnCannonMonsterActor.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnGravityOrbit.h"
#include "EtActionSignal.h"
#include "DnPlayAniProcess.h"
#include "DnCannonCamera.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "DnMainDlg.h"


const float CANNON_CROSSHAIR_TARGET_DIST = 300.0f;

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCannonMonsterActor::CDnCannonMonsterActor( int nClassID, bool bProcess ) : TDnMonsterActor( nClassID, bProcess ),
																			  m_vNowShootDir( 0.0f, 0.0f, 0.0f )
{
	m_bHaveMaster = false;
}


CDnCannonMonsterActor::~CDnCannonMonsterActor()
{
}

void CDnCannonMonsterActor::SetDestroy()
{
	// Notify the player actor that the cannon monster died.
	if( m_hMasterPlayerActor )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hMasterPlayerActor.GetPointer());
		pPlayerActor->OnCannonMonsterDie();
	}

	CDnMonsterActor::SetDestroy();
}

bool CDnCannonMonsterActor::Initialize( void )
{
	bool bResult = CDnMonsterActor::Initialize();
	if( false == bResult )
		return false;

	DNTableFileFormat* pMonsterTable = GetDNTable( CDnTableDB::TMONSTER );
	int nSkillTableId = pMonsterTable->GetFieldFromLablePtr( GetMonsterClassID(), "_SkillTable")->GetInteger();

	DNTableFileFormat*  pMonsterSkillTable = GetDNTable( CDnTableDB::TMONSTERSKILL );
	for( int i = 0; i < 2; ++i )
	{
		char szLabel1[128]={0,};
		char szLabel2[128]={0,};
		sprintf_s( szLabel1, "_SkillIndex%d", i + 1);
		sprintf_s( szLabel2, "_SkillLevel%d", i + 1);
		DNTableCell* pField = pMonsterSkillTable->GetFieldFromLablePtr( nSkillTableId, szLabel1 );
		if ( !pField )
			continue;

		int nSkillId = pField->GetInteger();

		pField = pMonsterSkillTable->GetFieldFromLablePtr( nSkillTableId, szLabel2 );

		int nSkillLevel = pField->GetInteger();

		if ( nSkillId < 0 || nSkillLevel < 0 )
			continue;
		
		bool bResult = AddSkill( nSkillId, nSkillLevel );
		_ASSERT( bResult );
		DnSkillHandle hSkill = FindSkill( nSkillId );
		if( hSkill )
		{
			// Extract projectile signals from actions used by the skill so they can be used later in targeting calculations.
			//const set<string>& setUseActionSet = hSkill->GetUseActionSet();
			//// Actually, since the action is firing a cannon, there should only be one.
			//_ASSERT( 1 == setUseActionSet.size() );
			//set<string>::const_iterator iter = setUseActionSet.begin();
			//for( iter; iter != setUseActionSet.end(); ++iter )
			//{
			CDnPlayAniProcess* pProcessor = static_cast<CDnPlayAniProcess*>(hSkill->GetProcessor( IDnSkillProcessor::PLAY_ANI ));
			string strActionName = pProcessor->GetActionName();
			ActionElementStruct* pActionElement = GetElement( strActionName.c_str() );
			int iNumSignals = (int)pActionElement->pVecSignalList.size();
			for( int iSignal = 0; iSignal < iNumSignals; ++iSignal )
			{
				CEtActionSignal* pSignal = pActionElement->pVecSignalList.at( iSignal );
				int iSignalIndex = pSignal->GetSignalIndex();
				if( STE_Projectile == iSignalIndex )
				{
					// Currently assumes each cannon action has only one projectile signal.
					_ASSERT( m_mapCannonProjectileSignalBySkillID.end() != m_mapCannonProjectileSignalBySkillID.find(nSkillId) );
					m_mapCannonProjectileSignalBySkillID[ nSkillId ] = (ProjectileStruct*)pSignal->GetData();
					break;
				}
			}
		}
	}

	m_vCannonLookDirection = GetMatEx()->m_vZAxis;
	return true;
}

void CDnCannonMonsterActor::Look( EtVector2& vVec, bool bForceRotate)
{
	if( bForceRotate ) 
	{
		if( EtVec2LengthSq( &vVec ) > 0.f ) {
			GetMatEx()->m_vZAxis = EtVector3( vVec.x, 0.f, vVec.y );
			EtVec3Normalize( &GetMatEx()->m_vZAxis, &GetMatEx()->m_vZAxis );
			GetMatEx()->MakeUpCartesianByZAxis();
		}
	}
}

void CDnCannonMonsterActor::SetMasterPlayerActor( DnActorHandle hMasterPlayerActor )
{
	m_hMasterPlayerActor = hMasterPlayerActor;
	m_vCannonLookDirection = GetMatEx()->m_vZAxis;
	// TODO: Cannon test code
	SetTeam( hMasterPlayerActor->GetTeam() );
	m_bHaveMaster = true;
}

void CDnCannonMonsterActor::ClearMasterPlayerActor( void )
{
	m_hMasterPlayerActor.Identity();
	m_vCannonLookDirection = GetMatEx()->m_vZAxis;
	m_bHaveMaster = false;
}

//float fCrosshairTargetDist = 300.0f;
bool CDnCannonMonsterActor::GetCannonGroundHitPosition( /*IN*/ DnCameraHandle hCamera,  /*OUT*/ EtVector3& vCannonGroundHitPos )
{
	bool bSuccess = false;

	// Whether cannon light or heavy attack, force (speed) must match for pre-placed decals to work.
	// Because if light and heavy attacks have different projectile speeds, the landing point differs and it is unknown which the player will fire
	//// Consequently, cannot render the decal.
	//map<int, const ProjectileStruct*>::iterator iter = m_mapCannonProjectileSignalBySkillID.find( iSkillID );
	//if( iter != m_mapCannonProjectileSignalBySkillID.end() )
	if( false == m_mapCannonProjectileSignalBySkillID.empty() )
	{
		map<int, const ProjectileStruct*>::iterator iter = m_mapCannonProjectileSignalBySkillID.begin();
		{
			const ProjectileStruct* pProjectileInfo = iter->second;
			EtVector3 vShootPos = m_matExCannonRotated.m_vPosition;
			vShootPos += m_matExCannonRotated.m_vXAxis * (pProjectileInfo->vOffset->x * GetScale());
			vShootPos += m_matExCannonRotated.m_vYAxis * (pProjectileInfo->vOffset->y * GetScale());
			vShootPos += m_matExCannonRotated.m_vZAxis * (pProjectileInfo->vOffset->z * GetScale());
			
			// For testing. Use the local-coordinate direction vector in the projectile struct as-is.
			//EtVector3 vShootDir = *pProjectileInfo->vDirection;

			int nWidth = CEtDevice::GetInstance().Width() / 2;
			int nHeight = (int)( CEtDevice::GetInstance().Height() * 0.46f/* * s_fCrossHairHeightRatio*/ );
			EtVector3 vOrig, vCrosshairDir;
			hCamera->GetCameraHandle()->CalcPositionAndDir( nWidth, nHeight, vOrig, vCrosshairDir );
			
			// Set to an arbitrary far position and retrieve only the direction.
			vOrig += vCrosshairDir * static_cast<CDnCannonCamera*>(hCamera.GetPointer())->GetDistance();
			EtVector3 vDestination = vOrig + (vCrosshairDir*CANNON_CROSSHAIR_TARGET_DIST);
			EtVector3 vDirToCrosshairDest = vDestination - vShootPos;
			EtVec3Normalize( &vDirToCrosshairDest, &vDirToCrosshairDest );

			// Since the current cannon camera's Z-axis direction is in world coordinates, extract and use only the X-axis rotation value.
			m_vNowShootDir = EtVector3( 0.0f, 0.0f, 1.0f );

			EtVector3 vDirToCrosshairDestZBasis( vDirToCrosshairDest.x, 0.0f, vDirToCrosshairDest.z );
			EtVec3Normalize( &vDirToCrosshairDestZBasis, &vDirToCrosshairDestZBasis );
			float fDot = EtVec3Dot( &vDirToCrosshairDest, &vDirToCrosshairDestZBasis );
			float fRotX = EtAcos( fDot );
			if( vDirToCrosshairDest.y < 0.0f )
				fRotX *= -1.0f;

			EtMatrix matRot;
			EtMatrixRotationX( &matRot, -fRotX );
			EtVec3TransformNormal( &m_vNowShootDir, &m_vNowShootDir, &matRot );

			CDnGravityOrbitCalculator::CalcHitGroundPos( m_matExCannonRotated, vShootPos, pProjectileInfo->fSpeed, m_vNowShootDir, vCannonGroundHitPos );

			bSuccess = true;
		}
	}

	return bSuccess;
}


void CDnCannonMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	TDnMonsterActor::Process(LocalTime, fDelta);

	if(m_hMasterPlayerActor)
	{
		bool bIsLocalMaster = false;

		if(CDnActor::s_hLocalActor && m_hMasterPlayerActor == CDnActor::s_hLocalActor)
			bIsLocalMaster = true;

		ProcessCannonLook(bIsLocalMaster);
	}
}

void CDnCannonMonsterActor::ProcessCannonLook(bool bLocal)
{

	if(bLocal)
	{
		int BoneIndex = GetBoneIndex("~Dummy11");

#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
		DNTableFileFormat* pTableCannon = GetDNTable( CDnTableDB::TCANNON );
		if( pTableCannon->IsExistItem(GetClassID()) )
		{
			const char* pCannonActionName = pTableCannon->GetFieldFromLablePtr( GetClassID(), "_Axis" )->GetString();
			BoneIndex = GetBoneIndex(pCannonActionName);
		}
#endif
		
		EtVector3 vec_Rotation = EtVector3(0.f,0.f,0.f);

		CDnLocalPlayerActor *pPlayer = dynamic_cast<CDnLocalPlayerActor*>(CDnActor::s_hLocalActor.GetPointer());
		DnCameraHandle hCamera = CDnCamera::GetActiveCamera();
		if( !hCamera || !pPlayer->IsCannonMode() ) return;
		if( hCamera->GetCameraType() != CDnCamera::CannonCamera ) return;

		CDnCannonCamera* pCamera = ((CDnCannonCamera*)hCamera.GetPointer());
		if(!pCamera) return;

		float fDot = EtVec3Dot( &EtVector3(0.f,1.f,0.f), &pCamera->GetMatEx()->m_vZAxis );
		vec_Rotation.x = (EtToDegree( EtAcos(fDot) ) - 90.f +10.f); 
		// Originally -90 degrees, but lowered slightly so the crosshair renders a bit higher. Not pointing at the exact spot, just for visual effect

		vec_Rotation.y = pCamera->GetYawValue();

		m_hObject->SetBoneRotation( BoneIndex, vec_Rotation);
		m_vCannonLookDirection = pCamera->GetMatEx()->m_vZAxis; //  Keep the info for now.
	}
	else
	{
		int BoneIndex = GetBoneIndex("~Dummy11");

#ifdef PRE_ADD_MODIFY_PLAYER_CANNON
		DNTableFileFormat* pTableCannon = GetDNTable( CDnTableDB::TCANNON );
		if( pTableCannon->IsExistItem(GetClassID()) )
		{
			const char* pCannonActionName = pTableCannon->GetFieldFromLablePtr( GetClassID(), "_Axis" )->GetString();
			BoneIndex = GetBoneIndex(pCannonActionName);
		}
#endif

		EtVector3 vec_Rotation = EtVector3(0.f,0.f,0.f);
		float fDot = EtVec3Dot( &EtVector3(0.f,1.f,0.f), &m_vCannonLookDirection );
		vec_Rotation.x = (EtToDegree( EtAcos(fDot) ) - 90.f +10.f); 

		float fCannonAngle = atan2f( GetMatEx()->m_vZAxis.x,GetMatEx()->m_vZAxis.z );
		float fCameraAngle = atan2f( m_vCannonLookDirection.x,m_vCannonLookDirection.z );

		vec_Rotation.y = EtToDegree(fCameraAngle) - EtToDegree(fCannonAngle);
		m_hObject->SetBoneRotation( BoneIndex, vec_Rotation);
	}

	// Base transform matrix for cannon firing
	// When firing a projectile, it references the firing actor's Cross, so keep a separate EtMatrixEx for the projectile.
	m_matExCannonRotated = m_matexWorld;
	m_matExCannonRotated.m_vZAxis = m_vCannonLookDirection;		// m_vCannonLookDirection is the same as the camera's ZVector.
	m_matExCannonRotated.m_vZAxis.y = 0.0f;
	EtVec3Normalize( &m_matExCannonRotated.m_vZAxis, &m_matExCannonRotated.m_vZAxis );
	m_matExCannonRotated.MakeUpCartesianByZAxis();

}
void CDnCannonMonsterActor::OnMasterPlayerActorDie( void )
{
	ClearMasterPlayerActor();
}


void CDnCannonMonsterActor::OnDie( DnActorHandle hHitter )
{
	// Notify the player actor that the cannon monster died.
	if( m_hMasterPlayerActor )
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hMasterPlayerActor.GetPointer());
		pPlayerActor->OnCannonMonsterDie();
	}
}

void CDnCannonMonsterActor::OnSkillUse( int iSkillID, int iLevel )
{
	// Start cooldown. Display only on the client that fired the cannon.
	if( CDnActor::s_hLocalActor == m_hMasterPlayerActor )
	{
		DnSkillHandle hSkill = FindSkill( iSkillID );
		CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
		if( pMainDlg && hSkill && hSkill->HasCoolTime() ) 
		{
			pMainDlg->AddPassiveSkill( hSkill );
		}
	}
}

void CDnCannonMonsterActor::OnCreateProjectileBegin( MatrixEx& matExLocal )
{
	// While monster fires a projectile, determines Target inside CreateProjectile by referencing the monster's EtMatrixEx 
	// Temporarily modified the value here and restored it due to an existing routine.
	m_matExOriginal = m_matexWorld;
	m_matexWorld = matExLocal;
}

void CDnCannonMonsterActor::OnCreateProjectileEnd( void )
{
	m_matexWorld = m_matExOriginal;
}

void CDnCannonMonsterActor::OnSignal( SignalTypeEnum Type, void *pPtr, LOCAL_TIME LocalTime, LOCAL_TIME SignalStartTime, LOCAL_TIME SignalEndTime, int nSignalIndex )
{
	switch( Type )
	{
		case STE_Projectile:
			{
				// Projectiles fired by server will be displayed upon receiving packet. Do not create separately on client.
				return;
			}
			break;
	}

	CDnMonsterActor::OnSignal( Type, pPtr, LocalTime, SignalStartTime, SignalEndTime, nSignalIndex );
}

void CDnCannonMonsterActor::OnHitSuccess( LOCAL_TIME LocalTime, DnActorHandle hActor )
{
	if( m_hMasterPlayerActor && m_hMasterPlayerActor->IsPlayerActor() && CDnActor::s_hLocalActor == m_hMasterPlayerActor ) {
		m_hMasterPlayerActor->OnHitSuccess( LocalTime, hActor );
	}
	CDnMonsterActor::OnHitSuccess( LocalTime, hActor );
}

void CDnCannonMonsterActor::OnComboCount( int nValue, int nDelay )
{
	if( m_hMasterPlayerActor && m_hMasterPlayerActor->IsPlayerActor() && CDnActor::s_hLocalActor == m_hMasterPlayerActor ) 
	{
		m_hMasterPlayerActor->OnComboCount( nValue, nDelay );
	}
}
