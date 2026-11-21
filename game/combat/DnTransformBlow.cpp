#include "stdafx.h"
#include "DnTransformBlow.h"
#include "DnPlayerActor.h"

CDnTransformBlow::CDnTransformBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_232;
	SetValue( szValue );

	std::string str = szValue;
	std::vector<std::string> tokens;
	TokenizeA(str, tokens, ";");

	m_nTransformIndex = 0;
	m_bSkillDefendency = false;

	if( tokens.size() == 3 )
	{
		m_nTransformIndex = atoi(tokens[0].c_str());
		m_strEndActionName = tokens[1];
		m_bSkillDefendency = atoi(tokens[2].c_str()) == 1 ? true : false;
	}
}

CDnTransformBlow::~CDnTransformBlow(void)
{

}


void CDnTransformBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	if(m_hActor && m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		pPlayer->ToggleTransformMode( true, m_nTransformIndex , true ); // Transform regardless of whether already transformed.
	}
}


void CDnTransformBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if(m_hActor && m_hActor->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		pPlayer->ToggleTransformMode( false, -1, false, m_strEndActionName.c_str() );
	}
}
