#include "stdafx.h"
#include "DnBeholderMonsterActor.h"
#include "DnActorClassDefine.h"
#include "DnGameTask.h"
#include "TaskManager.h"
#include "MAAiBase.h"


CDnBeholderMonsterActor::CDnBeholderMonsterActor( CMultiRoom *pRoom, int nClassID )
:  CDnPartsMonsterActor( pRoom, nClassID )
{
	m_fShakeDelta = 0.f;
	m_HitCheckType = HitCheckTypeEnum::Collision;
/*
	m_DummyBeholderEye = CreateActor(GetRoom(),99);
	if( m_DummyBeholderEye ) {
		m_DummyBeholderEye->SetProcess( false );
		m_DummyBeholderEye->Show(false);
		m_DummyBeholderEye->Initialize();
	}
*/
}

CDnBeholderMonsterActor::~CDnBeholderMonsterActor()
{
}


bool CDnBeholderMonsterActor::Initialize()
{
	CDnPartsMonsterActor::Initialize();
	
	
	return true;
}


void CDnBeholderMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	float fValue = 1.f;
	if( m_fShakeDelta > 0.f ) {
		m_fShakeDelta -= fDelta;
		if( m_fShakeDelta <= 0.f ) {
			m_fShakeDelta = 0.f;
		}
		fValue = 1.f - ( ( _rand(m_hActor->GetRoom())%100 ) * 0.0005f );
	}
	MAActorRenderBase::SetScale( EtVector3( GetScale() * fValue, GetScale(), GetScale() * fValue ) );

	if( !IsDie() ) {
		DNVector(DnActorHandle) hVecList;
		EtVector3 vAniDist = *GetPosition();
		vAniDist.x -= GetAniDistance()->x;
		vAniDist.z -= GetAniDistance()->z;

		int nCount =  ScanActor( GetRoom(), vAniDist, (float)GetUnitSize(), hVecList );
		if( nCount > 0 ) {
			EtVector2 vDir;
			int nSumSize;
			for( int i=0; i<nCount; i++ ) {
				if( !hVecList[i] ) continue;
				if( hVecList[i] == GetMySmartPtr() ) continue;
				if( hVecList[i]->IsDie() ) continue;
				vDir = EtVec3toVec2( *hVecList[i]->GetPosition() - *GetPosition() );
				nSumSize = GetUnitSize() + hVecList[i]->GetUnitSize();
				float fLength = D3DXVec2Length( &vDir );
				if( fLength > (float)nSumSize ) continue;

				D3DXVec2Normalize( &vDir, &vDir );
				MAMovementBase *pMovement = hVecList[i]->GetMovement();
				if( pMovement ) pMovement->MoveToWorld( vDir * (float)( nSumSize - fLength ) );
			}
		}
	}

	CDnPartsMonsterActor::Process( LocalTime, fDelta );
	
}


void CDnBeholderMonsterActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{
	std::string szActionName = HitParam.szActionName;
	HitParam.szActionName.clear();
	m_fShakeDelta = 0.3f;
	CDnPartsMonsterActor::OnDamage( pHitter, HitParam, pHitStruct );


	if ( IsDie() )
	{
		m_pAi->OnDie(CDnActionBase::m_LocalTime);
		SetDieDelta(10.0f);
	}

	HitParam.szActionName = szActionName;
}