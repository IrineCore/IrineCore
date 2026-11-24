#include "stdafx.h"
#include "DnAddStateBySkillGroupBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnAddStateBySkillGroupBlow::CDnAddStateBySkillGroupBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_276;
	SetValue( szValue );

	m_fValue = 0.0f;

#ifdef _GAMESERVER
	m_bStateEffectApplied = false;
	m_nServerBlowID = 0;
	m_nStateBlowIndex = 0;

	std::string szOriginalValue = szValue;
	std::vector<std::string> vecTokens;

	TokenizeA( szOriginalValue, vecTokens, "_" );

	if( vecTokens.size() > 0 )
	{
		std::vector<std::string> vecStateBlowTokens;
		TokenizeA( vecTokens[0], vecStateBlowTokens, "/" );
		for( DWORD i=0; i<vecStateBlowTokens.size(); i++ )
			m_vecAvailableSkillIndex.push_back( atoi(vecStateBlowTokens[i].c_str()) );

		if( vecTokens.size() > 0 ) m_nStateBlowIndex = atoi( vecStateBlowTokens[0].c_str() );
		if( vecTokens.size() > 1 ) m_szAdditionalBlowParam = vecStateBlowTokens[1];
	}

	if( vecTokens.size() > 1 )
	{
		std::vector<std::string> vecSkillIndexTokens;
		TokenizeA( vecTokens[1], vecSkillIndexTokens, "," );
		for( DWORD i=0; i<vecSkillIndexTokens.size(); i++ )
			m_vecAvailableSkillIndex.push_back( atoi(vecSkillIndexTokens[i].c_str()) );
	}
#endif

}

CDnAddStateBySkillGroupBlow::~CDnAddStateBySkillGroupBlow(void)
{

}

void CDnAddStateBySkillGroupBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}

void CDnAddStateBySkillGroupBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnAddStateBySkillGroupBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

#ifdef _GAMESERVER
	RemoveAdditionalStateBlow();
#endif

}

#ifdef _GAMESERVER
void CDnAddStateBySkillGroupBlow::ApplyAddtionalStateBlowFromSkill( int nSkillIndex )
{
	if( !m_hActor ) return;

	if( std::find(m_vecAvailableSkillIndex.begin() , m_vecAvailableSkillIndex.end(), nSkillIndex ) != m_vecAvailableSkillIndex.end() )
	{
		if( m_bStateEffectApplied == false )
		{
			m_nServerBlowID = m_hActor->CmdAddStateEffect( 
				GetParentSkillInfo(), 
				(STATE_BLOW::emBLOW_INDEX)m_nStateBlowIndex, 
				(int)(GetDurationTime() * 1000),
				m_szAdditionalBlowParam.c_str() );

			m_bStateEffectApplied = true;
		}
	}
}

void CDnAddStateBySkillGroupBlow::RemoveAdditionalStateBlow()
{
	if( !m_hActor ) return;

	if( m_bStateEffectApplied == true )
	{
		m_hActor->CmdRemoveStateEffectFromID( m_nServerBlowID );
		m_nServerBlowID = 0;
		m_bStateEffectApplied = false;
	}
}
#endif