#include "stdafx.h"
#include "DnDisableActionBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnDisableActionBlow::CDnDisableActionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_224;
	SetValue( szValue );
	SetBlowInfo();
}

void CDnDisableActionBlow::SetBlowInfo()
{
	std::string str = m_StateBlow.szValue;;
	std::string delimiters = ";";
	TokenizeA(str, m_vecActionList, delimiters);
}

bool CDnDisableActionBlow::IsMatchedAction(const char* strAction)
{
	for(DWORD i=0; i<m_vecActionList.size() ; i++)
	{
		if(	strcmp(strAction , m_vecActionList[i].c_str() ) == 0)
			return true;
	}

	return false;
}

CDnDisableActionBlow::~CDnDisableActionBlow(void)
{

}


void CDnDisableActionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{

}

void CDnDisableActionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnDisableActionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDisableActionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{	
}

void CDnDisableActionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{	
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
