#include "stdafx.h"
#include "DnGiantMonsterActor.h"


CDnGiantMonsterActor::CDnGiantMonsterActor( CMultiRoom *pRoom, int nClassID )
: CDnNormalMonsterActor( pRoom, nClassID )
{
	m_fShakeDelta = 0.f;
	
}

CDnGiantMonsterActor::~CDnGiantMonsterActor()
{
}

void CDnGiantMonsterActor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	float fValue = 1.f;
	if( m_fShakeDelta > 0.f ) {
		m_fShakeDelta -= fDelta;
		if( m_fShakeDelta <= 0.f ) {
			m_fShakeDelta = 0.f;
		}
		fValue = 1.f - ( ( _rand(GetRoom())%100 ) * 0.0005f );
	}
	MAActorRenderBase::SetScale( EtVector3( GetScale() * fValue, GetScale(), GetScale() * fValue ) );

	CDnNormalMonsterActor::Process( LocalTime, fDelta );
}

void CDnGiantMonsterActor::OnDamage( CDnDamageBase *pHitter, SHitParam &HitParam, HitStruct *pHitStruct )
{
	m_fShakeDelta = 0.3f;
	CDnNormalMonsterActor::OnDamage( pHitter, HitParam, pHitStruct );
}