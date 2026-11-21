
#include "stdafx.h"
#include "DnGreenDragonMonsterActor.h"

CDnGreenDragonMonsterActor::CDnGreenDragonMonsterActor( CMultiRoom *pRoom, int nClassID )
: CDnPartsMonsterActor( pRoom, nClassID )
{
	m_HitCheckType = HitCheckTypeEnum::Collision;

	SetFindAutoTargetName("Bip01 Head");
}

CDnGreenDragonMonsterActor::~CDnGreenDragonMonsterActor()
{
}

MAMovementBase* CDnGreenDragonMonsterActor::CreateMovement()
{
	MAMovementBase* pMovement = new MAStaticLookMovement();
	return pMovement;
}

bool CDnGreenDragonMonsterActor::Initialize()
{
	CDnPartsMonsterActor::Initialize();
	m_hObject->SetCollisionGroup( COLLISION_GROUP_DYNAMIC( 3 ) );
	m_hObject->SetTargetCollisionGroup( 0 );

	return true;
}

void CDnGreenDragonMonsterActor::OnFinishAction(const char* szPrevAction, LOCAL_TIME time)
{
	if( strstr( szPrevAction, "_ChangeAxis" ) )
	{		
		EtVector2 vView = EtVec3toVec2( *GetLookDir() );
		vView *= -1.f;
		Look( vView, true );
	}

	CDnMonsterActor::OnFinishAction( szPrevAction, time );
}

void CDnGreenDragonMonsterActor::OnBreakSkillSuperAmmor( SHitParam &HitParam )
{
}

void CDnGreenDragonMonsterActor::CheckDamageVelocity( DnActorHandle hActor )
{
	m_HitParam.vVelocity = EtVector3( 0.f, 0.f, 0.f );
	m_HitParam.vResistance = EtVector3( 0.f, 0.f, 0.f );
}
