
#include "stdafx.h"
#include "DNPvPPlayerAggroSystem.h"
#include "DnActor.h"
#include "DnBlow.h"
#include "DnPlayerActor.h"
#include "PvPScoreSystem.h"
#include "DnTableDB.h"
#include "DnMonsterActor.h"

CDNPvPPlayerAggroSystem::CDNPvPPlayerAggroSystem( DnActorHandle hActor )
: CDNAggroSystem( hActor ), m_uiMaxAggroValue( 0 )
{

}

CDNPvPPlayerAggroSystem::~CDNPvPPlayerAggroSystem()
{

}

void CDNPvPPlayerAggroSystem::OnProcessAggro( const LOCAL_TIME LocalTime, const float fDelta )
{
	m_uiMaxAggroValue = 0;

	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; )
	{
		DnActorHandle hActor = (*itor).hActor;

		if( !hActor || m_hActor->GetTeam() == hActor->GetTeam() || (*itor).iAggro <= 0 )
		{
			itor = m_AggroList.erase( itor );
			continue;
		}

		m_uiMaxAggroValue += (*itor).iAggro;
		++itor;
	}
}

DnActorHandle CDNPvPPlayerAggroSystem::OnGetAggroTarget( bool& bIsProvocationTarget, DnActorHandle hExceptActor/*=CDnActor::Identity()*/, DNVector(DnActorHandle)* vTarget )
{
	return CDnActor::Identity();
}

void CDNPvPPlayerAggroSystem::OnDamageAggro( DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam, int iDamage )
{
	if( !hActor )
		return;

	if( !hActor->IsPlayerActor() )
	{
		bool bOK = false;
		if( hActor->IsMonsterActor() )
		{
			DnActorHandle hMaster = static_cast<CDnMonsterActor*>(hActor.GetPointer())->GetSummonerPlayerActor();
			if( hMaster && hMaster->IsPlayerActor() )
			{
				hActor = hMaster;
				bOK	= true;
			}
		}

		if( !bOK )
			return;
	}

	int iAggroValue = iDamage * 2;
	if( hActor->IsProcessSkill() ) 
		iAggroValue += hActor->GetProcessSkill()->GetAdditionalThreat();

	AddAggro( hActor, iAggroValue );
}

void CDNPvPPlayerAggroSystem::OnStateBlowAggro( DnBlowHandle hBlow )
{
	if( !hBlow || !hBlow->GetActorHandle() )
		return;

	if( !hBlow->GetActorHandle()->IsPlayerActor() )
		return;

	if( hBlow->GetThreatAggro() <= 0 )
		return;

	AddAggro( hBlow->GetActorHandle(), hBlow->GetThreatAggro() );
}

bool CDNPvPPlayerAggroSystem::bOnCheckPlayerBeginStateBlow( CDnPlayerActor* pPlayer )
{
	EtVector3	vTemp = *pPlayer->GetPosition() - *m_hActor->GetPosition();
	float		fTempSq = 400.f; // 20.f*20.f;

	if( EtVec3LengthSq( &vTemp ) > fTempSq )
		return false;

	return true;
}

void CDNPvPPlayerAggroSystem::OnAggroRegulation( DnActorHandle hActor, int& iAggroValue )
{
	int iAggroPer = GetPlayerLevelTable().GetValue( m_hActor->OnGetJobClassID(), m_hActor->GetLevel(), CPlayerLevelTable::AggroPer );

	iAggroValue = iAggroValue * iAggroPer / 100;
}

void CDNPvPPlayerAggroSystem::Die( CPvPScoreSystem* pScoreSystem, DnActorHandle hHitter )
{
	_ASSERT( dynamic_cast<CPvPScoreSystem*>(pScoreSystem) != NULL );
	// Clean up Aggro
	OnProcessAggro( 0, 0 );
	if( m_uiMaxAggroValue == 0 )
		return;

	// Distribute AssistPoint
	for( std::list<AggroStruct>::iterator itor=m_AggroList.begin() ; itor!=m_AggroList.end() ; ++itor )
	{
		DnActorHandle hActor = (*itor).hActor;

		if( hActor == hHitter )
			continue;

		int iAssistDecision = GetPlayerLevelTable().GetValue( hActor->OnGetJobClassID(), hActor->GetLevel(), CPlayerLevelTable::AssistDecision );
		int iPercent		= static_cast<int>( (*itor).iAggro*100 / static_cast<float>(m_uiMaxAggroValue) );

		if( iPercent < iAssistDecision )
			continue;

		int iAddPoint = GetPlayerLevelTable().GetValue( hActor->OnGetJobClassID(), hActor->GetLevel(), CPlayerLevelTable::AssistScore ) * iPercent / 100;
		if( iAddPoint <= 0 )
			continue;

		CPvPScoreSystem::SMyScore* pScore = pScoreSystem->FindMyScoreDataIfnotInsert( hActor );
		if( !pScore )
			continue;

		pScore->uiAssistScore += iAddPoint;

		pScoreSystem->OnNotify( hActor, IScoreSystem::ScoreType::AssistScore, iAddPoint );
	}
}
