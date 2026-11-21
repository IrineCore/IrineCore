#include "stdafx.h"
#include "DnActionChangeRatioBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnActionChangeRatioBlow::CDnActionChangeRatioBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_206;
	SetValue( szValue );

	m_fValue = 0.0f;

	std::string str = szValue;//"Ratio;Action";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_fRate = (float)atof( tokens[0].c_str() );
		m_strActionName = tokens[1];
	}
	else
	{
		m_fRate = 0.0f;
		m_strActionName = "";
	}

#if defined(_GAMESERVER)
	AddCallBackType(SB_ONTARGETHIT);
#endif // _GAMESERVER
}

CDnActionChangeRatioBlow::~CDnActionChangeRatioBlow(void)
{

}

void CDnActionChangeRatioBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	OnChangeAction();
#endif // _GAMESERVER
}

void CDnActionChangeRatioBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
	
}


void CDnActionChangeRatioBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

}

#if defined(_GAMESERVER)
// float CDnActionChangeRatioBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
// {
// 	OnChangeAction(m_hActor);
// }

void CDnActionChangeRatioBlow::OnChangeAction()
{
	//Probability calculation..
	bool bExecuteable = rand() % 10000 <= (m_fRate * 10000.0f);

	//////////////////////////////////////////////////////////////////////////
	//For Wax, it is not a Debuff, so immunity is not applied when adding the state effect.
	//So check immunity here and skip processing if immune.
	if (m_hActor->IsImmuned(m_StateBlow.emBlowIndex))
		bExecuteable = false;
	//////////////////////////////////////////////////////////////////////////

	if (m_hActor && bExecuteable && !m_strActionName.empty())
	{
		m_hActor->CmdAction(m_strActionName.c_str(), 0, 3.0f, true, true, false);

		// #37325 [Cannot get up after Wax] For actions including down, <m_fDownDelta should be included; normally set when calculated by Hit, however, 
		// In this case m_fDownDelta may not be set; if 0, ProcessDown() does not run, so for actions including down, check down delta and set it if missing.
		if((m_hActor->GetState() & CDnActorState::Down) && m_hActor->GetDownRemainDelta() <= 0) 
			m_hActor->SetDownRemainDelta(0.1f); // Exceptional case, so make it stand up immediately.


		// #37553
		//For monster actors, packets are sent to the client from CdmAction, whereas
		//Do not send packets to the client if it is a player actor, so
		//Send SC_ACTION_CHANGE packet here.
		if (!m_hActor->IsMonsterActor())
		{
			int	nActionIndex = m_hActor->GetElementIndex( m_strActionName.c_str() );
			m_hActor->RequestActionChange(nActionIndex);
		}
		
	}
}
#endif // _GAMESERVER


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnActionChangeRatioBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fRate[2] = {0.0f, };
	std::string strActionName[2];

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fRate[0] = (float)atof( vlTokens[0][0].c_str() );
		strActionName[0] = vlTokens[0][1];
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fRate[1] = (float)atof( vlTokens[1][0].c_str() );
		strActionName[1] = vlTokens[1][1];
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultRate = fRate[0] + fRate[1];

	sprintf_s(szBuff, "%f;%s", fResultRate, strActionName[0].c_str());

	szNewValue = szBuff;
}

void CDnActionChangeRatioBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fRate[2] = {0.0f, };
	std::string strActionName[2];
	
	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fRate[0] = (float)atof( vlTokens[0][0].c_str() );
		strActionName[0] = vlTokens[0][1];
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fRate[1] = (float)atof( vlTokens[1][0].c_str() );
		strActionName[1] = vlTokens[1][1];
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultRate = fRate[0] - fRate[1];

	sprintf_s(szBuff, "%f;%s", fResultRate, strActionName[0].c_str());

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
