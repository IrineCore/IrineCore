
#include "stdafx.h"
#include "DNAggroRange.h"
#include "DnMonsterActor.h"
#include "MAAiScript.h"

CDNAggroRange::CDNAggroRange()
{
	m_pAI						= NULL;
	m_fThreatRange				= 0.f;
	m_fCognizanceThreatRange	= 0.f;
	m_fCognizanceGentleRange	= 0.f;
}

void CDNAggroRange::Initialize( CDnMonsterActor* pMonsterActor, DNTableFileFormat* pSox )
{
	m_pAI = static_cast<MAAiScript*>(pMonsterActor->GetAIBase());

	int iItemID = pMonsterActor->GetMonsterClassID();

	m_fThreatRange			 = static_cast<float>( pSox->GetFieldFromLablePtr( iItemID, "_ThreatRange" )->GetInteger() );
	m_fCognizanceThreatRange = static_cast<float>( pSox->GetFieldFromLablePtr( iItemID, "_CognizanceThreatRange" )->GetInteger() ) ;
	m_fCognizanceGentleRange = static_cast<float>( pSox->GetFieldFromLablePtr( iItemID, "_CognizanceGentleRange" )->GetInteger() );
}

float CDNAggroRange::GetThreatRange()
{ 	
	float fThreatRange = m_fThreatRange;

	do 
	{
		if( m_pAI == NULL )
			break;
		DnActorHandle hActor = m_pAI->GetActor();
		if( !hActor )
			break;

		int iPercent = hActor->GetHPPercent();
		
		for( UINT i=0 ; i<m_pAI->GetScriptData().m_AIAggroRange.size() ; ++i )
		{
			if( iPercent > m_pAI->GetScriptData().m_AIAggroRange[i].iSelfHPPercent )
				break;
			else
				fThreatRange = static_cast<float>(m_pAI->GetScriptData().m_AIAggroRange[i].iThreatRange);
		}
	}while(false);

	return fThreatRange; 
}

float CDNAggroRange::GetCognizanceThreatRange()
{ 
	float fCognizanceThreatRange = m_fCognizanceThreatRange;

	do 
	{
		if( m_pAI == NULL )
			break;
		DnActorHandle hActor = m_pAI->GetActor();
		if( !hActor )
			break;

		int iPercent = hActor->GetHPPercent();

		for( UINT i=0 ; i<m_pAI->GetScriptData().m_AIAggroRange.size() ; ++i )
		{
			if( iPercent > m_pAI->GetScriptData().m_AIAggroRange[i].iSelfHPPercent )
				break;
			else
				fCognizanceThreatRange = static_cast<float>(m_pAI->GetScriptData().m_AIAggroRange[i].iCognizanceThreatRange);
		}
	}while(false);

	return fCognizanceThreatRange; 
}

float CDNAggroRange::GetCognizanceThreatRangeSq()
{ 
	float fValue = GetCognizanceThreatRange();
	return fValue*fValue;
}

float CDNAggroRange::GetCognizanceGentleRangeSq()
{ 
	float fCognizanceGentleRange = m_fCognizanceGentleRange;

	do 
	{
		if( m_pAI == NULL )
			break;
		DnActorHandle hActor = m_pAI->GetActor();
		if( !hActor )
			break;

		int iPercent = hActor->GetHPPercent();

		for( UINT i=0 ; i<m_pAI->GetScriptData().m_AIAggroRange.size() ; ++i )
		{
			if( iPercent > m_pAI->GetScriptData().m_AIAggroRange[i].iSelfHPPercent )
				break;
			else
				fCognizanceGentleRange = static_cast<float>(m_pAI->GetScriptData().m_AIAggroRange[i].iCognizanceGentleRange);
		}
	}while(false);

	return fCognizanceGentleRange*fCognizanceGentleRange; 
}
