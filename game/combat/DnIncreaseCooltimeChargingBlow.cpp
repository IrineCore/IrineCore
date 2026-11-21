#include "stdafx.h"
#include "DnIncreaseCooltimeChargingBlow.h"
#include "DnSkill.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnIncreaseCooltimeChargingBlow::CDnIncreaseCooltimeChargingBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_nSkillID( 0 )
	, m_nAddChargeCount( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_304;
	SetValue( szValue );

	std::vector<std::string> vlTokens;
	std::string strValue = szValue ? szValue : "";
	TokenizeA( strValue, vlTokens, ";" );
	if( vlTokens.size() >= 2 )
	{
		m_nSkillID = atoi( vlTokens[0].c_str() );
		m_nAddChargeCount = atoi( vlTokens[1].c_str() );
	}
}

CDnIncreaseCooltimeChargingBlow::~CDnIncreaseCooltimeChargingBlow( void )
{
}

void CDnIncreaseCooltimeChargingBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor )
		return;

	DnSkillHandle hSkill = m_hActor->FindSkill( m_nSkillID );
	if( hSkill )
		hSkill->SetAdditionalCoolTimeChargeCount( m_nAddChargeCount );
}

void CDnIncreaseCooltimeChargingBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnIncreaseCooltimeChargingBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hActor )
		return;

	DnSkillHandle hSkill = m_hActor->FindSkill( m_nSkillID );
	if( hSkill )
		hSkill->SetAdditionalCoolTimeChargeCount( 0 );
}
