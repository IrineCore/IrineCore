
#include "stdafx.h"
#include "MAAiMultipleTarget.h"
#include "MAAiScript.h"
#include "DnMonsterActor.h"
#include "DnProjectile.h"
#include "MAScanner.h"
#include "DnPlayerActor.h"
#if defined (PRE_MOD_AIMULTITARGET)
#include "DNAggroSystem.h"
#endif

CMAAiMultipleTarget::CMAAiMultipleTarget()
:m_eType(MultipleTargetType::Max)
{
	Reset();
}

CMAAiMultipleTarget::~CMAAiMultipleTarget()
{

}

void CMAAiMultipleTarget::Reset()
{ 
#if defined (PRE_MOD_AIMULTITARGET)
	m_nMultipleTarget = 0; 
	m_nSummonerTarget =0;
#else
	m_bIsMultipleTarget	= false; 
#endif
	m_iMaxMultipleTargetCount = 0;
	m_bIsAnyMultipleTarget = false;
	m_bIsExceptCannonPlayer = false;
	m_pActionTable = NULL;
	m_pSkillTable = NULL;
}

void CMAAiMultipleTarget::SetMultipleTargetAction( ActionTable* pActionTable )
{
#if defined (PRE_MOD_AIMULTITARGET)
	m_nMultipleTarget = pActionTable->nMultipleTarget;
	if( m_nMultipleTarget <= 0 )
		return;
	m_nSummonerTarget = pActionTable->nSummonerTarget;
#else
	m_bIsMultipleTarget = pActionTable->bMultipleTarget;
	if( !m_bIsMultipleTarget )
		return;
#endif

	m_eType						= MultipleTargetType::Action;
	m_pActionTable				= pActionTable;
	m_iMaxMultipleTargetCount	= pActionTable->iMaxMultipleTargetCount;
	m_bIsAnyMultipleTarget		= pActionTable->bIsAnyMultipleTarget;
	m_bIsExceptCannonPlayer		= pActionTable->bIsExceptCannonPlayer;
}

void CMAAiMultipleTarget::SetMultipleTargetSkill( MonsterSkillTable* pSkillTable )
{
#if defined (PRE_MOD_AIMULTITARGET)
	m_nMultipleTarget = pSkillTable->nMultipleTarget;
	if( m_nMultipleTarget <= 0 )
		return;
	m_nSummonerTarget = pSkillTable->nSummonerTarget;
#else
	m_bIsMultipleTarget = pSkillTable->bMultipleTarget;
	if( !m_bIsMultipleTarget )
		return;
#endif

	m_eType						= MultipleTargetType::Skill;
	m_pSkillTable				= pSkillTable;
	m_iMaxMultipleTargetCount	= pSkillTable->iMaxMultipleTargetCount;
	m_bIsAnyMultipleTarget		= pSkillTable->bIsAnyMultipleTarget;
	m_bIsExceptCannonPlayer		= pSkillTable->bIsExceptCannonPlayer;
}

void CMAAiMultipleTarget::CalcTarget( DnActorHandle hActor, MAAiScript* pScript )
{
	m_vTargetPos.clear();
	m_vTargetActor.clear();
	
#if defined (PRE_MOD_AIMULTITARGET)
	if( m_nMultipleTarget <= 0)
		return;
#else
	if( !m_bIsMultipleTarget )
		return;
#endif

	DNVector(DnActorHandle)	vTarget;
	CMAAiCheckerManager*	pCheckerManager = NULL;

	if( m_eType == MultipleTargetType::Action )
	{
		DN_ASSERT( m_pActionTable != NULL, "m_pActionTable == NULL" );

		float fMaxRange = pScript->GetScriptData().m_AITable[m_pActionTable->nDistanceState].fNearValue;
		if( m_bIsAnyMultipleTarget )
			GetMAScanner().Scan( MAScanner::eType::MonsterSkillOpponentTeam, hActor, 0.f, fMaxRange, vTarget );
		else
			pScript->GetTargetDistance( m_pActionTable->nDistanceState, vTarget, false );
		
		pCheckerManager = m_pActionTable->pMAAiCheckerManager;
	}
	else if( m_eType == MultipleTargetType::Skill )
	{
		DN_ASSERT( m_pSkillTable != NULL, "m_pSkillTable == NULL" );

		if( m_bIsAnyMultipleTarget )
			GetMAScanner().Scan( MAScanner::eType::MonsterSkillOpponentTeam, hActor, 0.f, static_cast<float>(m_pSkillTable->nRangeMax), vTarget );
		else
			pScript->GetTargetDistance( m_pSkillTable->nRangeMin, m_pSkillTable->nRangeMax, vTarget, false );
		if( vTarget.empty() )
			return;

		pCheckerManager = m_pSkillTable->pMAAiCheckerManager;
	}

	// Check condition with AIChecker
	if( pCheckerManager )
	{
		for( UINT i=0 ; i<vTarget.size() ; ++i )
		{
			// Check CannonActor
			if( vTarget[i]->GetActorType() == CDnActorState::ActorTypeEnum::Cannon )
				continue;

			if( m_bIsExceptCannonPlayer && vTarget[i]->IsPlayerActor() )
			{	
				if( static_cast<CDnPlayerActor*>(vTarget[i].GetPointer())->IsCannonMode() )
					continue;
			}

#if defined (PRE_MOD_AIMULTITARGET)
			if (m_nSummonerTarget && vTarget[i]->IsMonsterActor())
			{
				DnActorHandle hSummonMasterPlayerActor = static_cast<CDnMonsterActor*>(vTarget[i].GetPointer())->GetSummonerPlayerActor();
				if (hSummonMasterPlayerActor)
					continue;
			}
#endif

			if( !pCheckerManager->bIsTargetActorChecker( hActor, vTarget[i] ) )
				continue;

			m_vTargetPos.push_back( *vTarget[i]->GetPosition() );
			m_vTargetActor.push_back( vTarget[i] );
		}
	}

	// If this value is 0, all targets as before
	if( m_iMaxMultipleTargetCount == 0 )
		return;

#if defined (PRE_MOD_AIMULTITARGET)
	// Type = 1 Normal
	if (m_nMultipleTarget == 2) // Type = 2 Aggro 1st priority -> Random
	{
		if (true == CalcTargetByAggro(hActor))
			return;
	}

	// If less than max MultipleTarget count, just fire at all of them.
	if( m_vTargetPos.size() <= static_cast<size_t>(m_iMaxMultipleTargetCount) )
		return;

	do 
	{
		size_t RandVal = _rand( hActor->GetRoom() )%m_vTargetPos.size();

		m_vTargetPos.erase( m_vTargetPos.begin()+RandVal );
		m_vTargetActor.erase( m_vTargetActor.begin()+RandVal );
	}while( m_vTargetPos.size() > static_cast<size_t>(m_iMaxMultipleTargetCount) );

#else // #if defined (PRE_MOD_AIMULTITARGET)

	// If less than max MultipleTarget count, just fire at all of them.
	if( m_vTargetPos.size() <= static_cast<size_t>(m_iMaxMultipleTargetCount-1) )
		return;

	do 
	{
		size_t RandVal = _rand( hActor->GetRoom() )%m_vTargetPos.size();

		m_vTargetPos.erase( m_vTargetPos.begin()+RandVal );
		m_vTargetActor.erase( m_vTargetActor.begin()+RandVal );
	}while( m_vTargetPos.size() > static_cast<size_t>(m_iMaxMultipleTargetCount-1) );

#endif // #if defined (PRE_MOD_AIMULTITARGET)
}

#if defined (PRE_MOD_AIMULTITARGET)
bool CMAAiMultipleTarget::CalcTargetByAggro(DnActorHandle hActor)
{
	if( m_iMaxMultipleTargetCount <= 0 )
		return true;

	bool bIsProvocationTarget;
	DnActorHandle hTarget = hActor->GetAggroSystem()->OnGetAggroTarget (bIsProvocationTarget, CDnActor::Identity(), &m_vTargetActor);

	// Get index of TargetPos
	int nIndex = -1;
	for (UINT i=0; i<m_vTargetActor.size(); i++)
	{
		if (m_vTargetActor[i] == hTarget)
		{
			nIndex = i;
			break;
		}
	}

	if (m_vTargetActor.size() <= 1)
		return true;

	// If less than max MultipleTarget count, just fire at all of them.
	if( m_vTargetPos.size() <= static_cast<size_t>(m_iMaxMultipleTargetCount) )
		return true;

	EtVector3 vTargetPos;
	if (nIndex >= 0)	// Index found.
	{
		if (nIndex >= m_vTargetPos.size())
			return false;

		vTargetPos = m_vTargetPos[nIndex];
		m_vTargetPos.erase(m_vTargetPos.begin()+nIndex);
		m_vTargetActor.erase(m_vTargetActor.begin()+nIndex);
	}
	else // If index is not found
		return false;

	if (m_vTargetActor.size() == 0)
	{
		m_vTargetActor.push_back(hTarget);
		m_vTargetPos.push_back(vTargetPos);
		return true;
	}

	//Make a slot for the aggro target hTarget.
	do 
	{
		size_t RandVal = _rand( hActor->GetRoom() )%m_vTargetPos.size();

		m_vTargetPos.erase( m_vTargetPos.begin()+RandVal );
		m_vTargetActor.erase( m_vTargetActor.begin()+RandVal );
	}while( m_vTargetPos.size() > static_cast<size_t>(m_iMaxMultipleTargetCount-1) );

	m_vTargetActor.push_back(hTarget);
	m_vTargetPos.push_back(vTargetPos);

	return true;
}
#endif

void CMAAiMultipleTarget::CreateProjectile( CDnMonsterActor* pMonsterActor, ProjectileStruct* pStruct, int iSignalIndex )
{
	// If TargetType == Target, calculate at the current time and acquire Multiple targets.
	if( pStruct->nTargetType == CDnProjectile::TargetTypeEnum::Target || pStruct->nTargetType == CDnProjectile::TargetTypeEnum::TargetPosition )
	{
		CalcTarget( pMonsterActor->GetActorHandle(), static_cast<MAAiScript*>(pMonsterActor->GetAIBase()) );
	}

	DN_ASSERT( m_vTargetPos.size() == m_vTargetActor.size(), "m_vTargetPos.size() != m_vTargetActor.size()" );

	MatrixEx OrgLocalCross = *pMonsterActor->GetMatEx();

	for( UINT i=0 ; i<m_vTargetPos.size() ; ++i )
	{
		// Set Projectile direction
		MatrixEx LocalCross = OrgLocalCross;

		// For Homing/Terrain Homing, modifying the original monster's LocalCross would rotate the firing monster, so 
		// The projectile firing offset set in the action tool also rotates. Block it only for the problematic projectile types for now. #26226
		if( false == (CDnProjectile::Homing == pStruct->nOrbitType ||
					  CDnProjectile::TerrainHoming == pStruct->nOrbitType) )
		{
			LocalCross.Look( &m_vTargetPos[i] );
		}

		CDnProjectile* pProjectile = CDnProjectile::CreateProjectile( pMonsterActor->GetRoom(), pMonsterActor->GetActorHandle(), LocalCross, pStruct, &m_vTargetPos[i], m_vTargetActor[i] );
		if( pProjectile == NULL ) 
			return;
		pProjectile->SetShooterType( pMonsterActor->GetMySmartPtr(), pMonsterActor->GetCurrentActionIndex(), iSignalIndex );

		pMonsterActor->SendProjectile( pProjectile, pStruct, LocalCross, iSignalIndex );
		pMonsterActor->OnProjectile( pProjectile, pStruct, LocalCross, iSignalIndex );
		pMonsterActor->OnSkillProjectile( pProjectile );
	}
}
