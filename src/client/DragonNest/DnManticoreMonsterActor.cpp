#include "StdAfx.h"
#include "DnManticoreMonsterActor.h"
#include "DnActorClassDefine.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "MAAiBase.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnManticoreMonsterActor::CDnManticoreMonsterActor( int nClassID, bool bProcess )
:  CDnPartsMonsterActor( nClassID, bProcess )
{
	m_nIgnoreNormalSuperArmorTime = 0;
	
	m_HitCheckType = HitCheckTypeEnum::Collision;
}

CDnManticoreMonsterActor::~CDnManticoreMonsterActor()
{
}


bool CDnManticoreMonsterActor::Initialize()
{
	CDnPartsMonsterActor::Initialize();

	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
	m_hObject->SetTargetCollisionGroup( 0 );

	SetUseDamageColor(true);

	return true;
}


void CDnManticoreMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnPartsMonsterActor::Process( LocalTime, fDelta );
	/*
	if( !IsDie() ) {
		std::vector<DnActorHandle> hVecList;
		EtVector3 vAniDist = *GetPosition();
		vAniDist.x -= GetAniDistance()->x;
		vAniDist.z -= GetAniDistance()->z;

		int nCount =  ScanActor( vAniDist, (float)GetUnitSize(), hVecList );
		if( nCount > 0 ) {
			EtVector2 vDir;
			int nSumSize;
			for( int i=0; i<nCount; i++ ) {
				if( hVecList[i] == GetMySmartPtr() ) continue;
				if( hVecList[i]->IsDie() ) continue;
				vDir = EtVec3toVec2( *hVecList[i]->GetPosition() - *GetPosition() );
				nSumSize = GetUnitSize() + hVecList[i]->GetUnitSize();
				float fLength = D3DXVec2Length( &vDir );
				if( fLength > (float)nSumSize ) continue;

				D3DXVec2Normalize( &vDir, &vDir );
				MAMovementBase *pMovement = dynamic_cast<MAMovementBase *>(hVecList[i].GetPointer());
				if( pMovement ) pMovement->MoveToWorld( vDir * (float)( nSumSize - fLength ) );
			}
		}
	}
	*/
}

void CDnManticoreMonsterActor::OnDrop( float fCurVelocity )
{
	/*
	if( IsAir() && IsHit() ) {
		std::string szAction;
		float fBlendFrame = 2.f;
		// If falling speed is 10 or more, bounce up once more with bigBounce.
		if( fCurVelocity < -6.f && m_HitParam.vVelocity.y != 0.f ) {
			if( m_HitParam.vVelocity.y > 0.f ) {
				m_HitParam.vVelocity.y *= 0.6f;
				SetVelocityY( m_HitParam.vVelocity.y );
			}
else { // If acceleration is already pointing toward the ground from the start, invert it.
				m_HitParam.vVelocity.y *= -0.6f;
				m_HitParam.vResistance.y *= -1.f;
				SetVelocityY( m_HitParam.vVelocity.y );
				SetResistanceY( m_HitParam.vResistance.y );
			}
			szAction = "Hit_AirBounce";
		}
		else {
			szAction = "Down_SmallBounce";
			fBlendFrame = 0.f;
		}
		SetActionQueue( szAction.c_str(), 0, fBlendFrame );
*/
		TileTypeEnum TileType = CDnWorld::GetInstance().GetTileType( m_matexWorld.m_vPosition );

		CreateEnviEffectParticle( ConditionEnum::Down, TileType, &m_matexWorld );
		CreateEnviEffectSound( ConditionEnum::Down, TileType, m_matexWorld.m_vPosition );
	//}
}