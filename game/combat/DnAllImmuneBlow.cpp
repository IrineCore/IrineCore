#include "stdafx.h"
#include "DnAllImmuneBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnAllImmuneBlow::CDnAllImmuneBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ), 
																			m_fResistPercent( 0 )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_150;
	SetValue( szValue );
	
	// Set exception state effect indices that do not become immune, delimited by ';'.
	string strValue( szValue );
	std::vector<string> vlTokens;
	TokenizeA( strValue, vlTokens, ";" );

	for( int i = 0; i < (int)vlTokens.size(); ++i )
	{

		const char* pToken = vlTokens.at( i ).c_str();
		int iBlowIndex = atoi( pToken );
		m_setExceptionBlowIndices.insert( iBlowIndex );
	}
}

CDnAllImmuneBlow::~CDnAllImmuneBlow(void)
{
}

void CDnAllImmuneBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
//	OutputDebug( "CDnAllImmuneBlow::OnBegin, Value: %d\n", m_nValue );
}


void CDnAllImmuneBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	// This one does nothing other than immunity..	
	CDnBlow::Process( LocalTime, fDelta );
}


bool CDnAllImmuneBlow::IsImmuned( const CDnSkill::SkillInfo* pParentSkillInfo, STATE_BLOW::emBLOW_INDEX BlowIndex )
{
	if( NULL == pParentSkillInfo )
		return false;

	bool bResult = true;

	// State effect indices present in the exception set bypass immunity and pass through. 
	// Immune to everything else.
	set<int>::iterator iter = m_setExceptionBlowIndices.find( (int)BlowIndex );
	if( m_setExceptionBlowIndices.end() != iter )
		bResult = false;

	return bResult;
};


void CDnAllImmuneBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
//	OutputDebug( "CDnAllImmuneBlow::OnEndn" );
}

void CDnAllImmuneBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	// State effect immunity state effect does nothing.
}

bool CDnAllImmuneBlow::IsImmuned( STATE_BLOW::emBLOW_INDEX BlowIndex )
{
	bool bResult = true;

	// State effect indices present in the exception set bypass immunity and pass through. 
	// Immune to everything else.
	set<int>::iterator iter = m_setExceptionBlowIndices.find( (int)BlowIndex );
	if( m_setExceptionBlowIndices.end() != iter )
		bResult = false;

	return bResult;
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAllImmuneBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
	//Append ";" if missing at the end of szOrigValue, then append szAddValue.
	int nLength = (int)strlen(szOrigValue);
	
	if (nLength == 0 || szOrigValue[nLength - 1] == ';')
		szNewValue += szAddValue;
	else
	{
		szNewValue += ";";
		szNewValue += szAddValue;
	}
	
}

void CDnAllImmuneBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//"###;###;###;###;....;##;##;##"
	//|<--Previous string->|<-AddValue->|

	//Trim szOrigValue from the end by the length of szAddValue.
	int nOrigLength = (int)strlen(szOrigValue);
	int nAddLength = (int)strlen(szAddValue);
	int nCount = nOrigLength - nAddLength;

	sprintf_s(szBuff, "%s", szOrigValue);
	szBuff[nCount] = 0;

	szNewValue = szBuff;	
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
