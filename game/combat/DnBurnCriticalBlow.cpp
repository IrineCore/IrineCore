#include "stdafx.h"
#include "DnBurnCriticalBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnBurnCriticalBlow::CDnBurnCriticalBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_222;
	SetValue( szValue );

#if defined(_GAMESERVER)
	m_nMultiplyRatio = 0;   // Product of existing probabilities.
	m_nAddValueRatio = 0; // Add absolute critical value

	SetBlowInfo();
#endif

}

#if defined(_GAMESERVER)
void CDnBurnCriticalBlow::SetBlowInfo()
{
	std::string str = m_StateBlow.szValue;

	std::vector<std::string> tokens;
	std::string delimiters = ";";
	TokenizeA(str, tokens, delimiters);

	if(!tokens.empty())
	{
		switch(tokens.size())
		{
		case 1:
			m_nMultiplyRatio = atoi(tokens[0].c_str());
			break;

		case 2:
			m_nMultiplyRatio = atoi(tokens[0].c_str());
			m_nAddValueRatio = atoi(tokens[1].c_str());
			break;
		default:
			break;
		}
	}
}
#endif

CDnBurnCriticalBlow::~CDnBurnCriticalBlow(void)
{

}


void CDnBurnCriticalBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnBurnCriticalBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnBurnCriticalBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnBurnCriticalBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{	
}

void CDnBurnCriticalBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{	
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
