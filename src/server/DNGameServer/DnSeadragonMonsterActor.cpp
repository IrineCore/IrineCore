#include "stdafx.h"
#include "DnSeadragonMonsterActor.h"

CDnSeadragonMonsterActor::CDnSeadragonMonsterActor( CMultiRoom *pRoom, int nClassID )
: CDnPartsMonsterActor( pRoom, nClassID )
{
	m_HitCheckType = HitCheckTypeEnum::Collision;
}

CDnSeadragonMonsterActor::~CDnSeadragonMonsterActor()
{
}

MAMovementBase* CDnSeadragonMonsterActor::CreateMovement()
{
	MAMovementBase* pMovement = new MAStaticLookMovement();
	return pMovement;
}

bool CDnSeadragonMonsterActor::Initialize()
{
	CDnPartsMonsterActor::Initialize();
	m_hObject->SetCalcPositionFlag( 0 );
	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
	m_hObject->SetTargetCollisionGroup( 0 );

	return true;
}

void CDnSeadragonMonsterActor::OnFinishAction(const char* szPrevAction, LOCAL_TIME time)
{
	if( strstr( szPrevAction, "_ChangeAxis" ) )
	{		
		EtVector2 vView = EtVec3toVec2( *GetLookDir() );
		vView *= -1.f;
		Look( vView, true );
	}

	CDnMonsterActor::OnFinishAction( szPrevAction, time );
}

void CDnSeadragonMonsterActor::OnBreakSkillSuperAmmor( SHitParam &HitParam )
{
}

void CDnSeadragonMonsterActor::CheckDamageVelocity( DnActorHandle hActor )
{
	m_HitParam.vVelocity = EtVector3( 0.f, 0.f, 0.f );
	m_HitParam.vResistance = EtVector3( 0.f, 0.f, 0.f );
}
