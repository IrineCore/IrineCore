#include "stdafx.h"
#include "DnDissolveStateBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnDissolveStateBlow::CDnDissolveStateBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_155;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
	m_fValue = 0.0f;//(float)atof(szValue);

	m_BeginLevel = m_EndLevel = 0;

	AddDissolveStateID(szValue);
}

CDnDissolveStateBlow::~CDnDissolveStateBlow(void)
{
}

void CDnDissolveStateBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

void CDnDissolveStateBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);

	// Should only need to run on the server. [2010/12/17 semozz]
	// State effects to remove on server.. modified to send packet.
#if defined(_GAMESERVER)
	//Skip if outside apply level range.
	int nCharLevel = m_hActor->GetLevel();
	if (nCharLevel < m_BeginLevel || m_EndLevel < nCharLevel)
		return;

	DNVector(int) vlAppliedDebuffBlowIds;

	//Iterate through registered StateBlowIndex
	DISSOLVE_LIST::iterator iter = m_DissolveStateIDList.begin();
	DISSOLVE_LIST::iterator endIter = m_DissolveStateIDList.end();
	for ( ; iter != endIter; ++iter)
	{
		DNVector(DnBlowHandle) vResult;
		m_hActor->GatherAppliedStateBlowByBlowIndex((STATE_BLOW::emBLOW_INDEX)iter->second, vResult);

		//Remove each registered StateBlow..
		for( UINT i=0 ; i<vResult.size() ; ++i ) 
		{
			DnBlowHandle hBlow = vResult.at( i );
			if( !hBlow )
				continue;

			vlAppliedDebuffBlowIds.push_back(hBlow->GetBlowID());
		}
	}
	
	// Dispel only dispellable harmful skills and state effects
	int iNumBlow = (int)vlAppliedDebuffBlowIds.size();
	for( int iBlow = 0; iBlow < iNumBlow; ++iBlow )
	{
		int iBlowID = vlAppliedDebuffBlowIds.at( iBlow );
		//m_hActor->RemoveStateBlowFromID( iBlowID );
		
		m_hActor->CmdRemoveStateEffectFromID(iBlowID);
	}
#endif // _GAMESERVER
}

void CDnDissolveStateBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "%s\n", __FUNCTION__);
}

void CDnDissolveStateBlow::AddDissolveStateID(const char* szValue)
{
	// Added apply level feature [2010/11/30 semozz]
	// [##;##;...##;][##:##]
	// [StateEffect Indices][ApplyLevel Start : ApplyLevel End]

	std::string str = szValue;//"[45;46;47;][23:45]";
	std::vector<std::string> tokens;
	std::string delimiters = "[]";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);
	}
	else
	{
		//2. Separate state blow index list
		std::string stateIndexList = tokens[0];
		std::vector<std::string> stateIndexTokens;
		delimiters = ";";

		TokenizeA(stateIndexList, stateIndexTokens, delimiters);
		std::vector<std::string>::iterator iter = stateIndexTokens.begin();
		for ( ; iter != stateIndexTokens.end(); ++iter)
		{
			AddDissolveStateID(atoi(iter->c_str()));
		}

		//3. Applied level classification
		std::string levelInfo = tokens[1];
		std::vector<std::string> levelInfoTokens;
		delimiters = ":";
		TokenizeA(levelInfo, levelInfoTokens, delimiters);
		
		if (levelInfoTokens.size() != 2)
		{
			OutputDebug("%s :: %s --> Need to check state effect application level!!!\n", __FUNCTION__, szValue);
		}
		else
		{
			int startLevel = atoi(levelInfoTokens[0].c_str());
			int endLevel = atoi(levelInfoTokens[1].c_str());

			SetLevelInfo(startLevel, endLevel);
		}
		
	}
}

void CDnDissolveStateBlow::AddDissolveStateID(int nStateBlowIndex)
{
	m_DissolveStateIDList.insert(DISSOLVE_LIST::value_type(nStateBlowIndex, nStateBlowIndex));
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDissolveStateBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[256] = {0, };

	std::string szStateID[2];
	int startLevel[2] = {0, };
	int endLevel[2] = {0, };

	std::string str = szOrigValue;//"[45;46;47;][23:45]";
	std::vector<std::string> tokens;
	std::string delimiters = "[]";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);
	}
	else
	{
		//2. Separate state blow index list
		szStateID[0] = tokens[0];
		
		//3. Applied level classification
		std::string levelInfo = tokens[1];
		std::vector<std::string> levelInfoTokens;
		delimiters = ":";
		TokenizeA(levelInfo, levelInfoTokens, delimiters);

		if (levelInfoTokens.size() != 2)
		{
			OutputDebug("%s :: %s --> Need to check state effect application level!!!\n", __FUNCTION__, szOrigValue);
		}
		else
		{
			startLevel[0] = atoi(levelInfoTokens[0].c_str());
			endLevel[0] = atoi(levelInfoTokens[1].c_str());
		}
	}


	//////////////////////////////////////////////////////////////////////////
	str = szAddValue;//"[45;46;47;][23:45]";
	
	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);
	}
	else
	{
		//2. Separate state blow index list
		szStateID[1] = tokens[0];

		//3. Applied level classification
		std::string levelInfo = tokens[1];
		std::vector<std::string> levelInfoTokens;
		delimiters = ":";
		TokenizeA(levelInfo, levelInfoTokens, delimiters);

		if (levelInfoTokens.size() != 2)
		{
			OutputDebug("%s :: %s --> Need to check state effect application level!!!\n", __FUNCTION__, szAddValue);
		}
		else
		{
			startLevel[1] = atoi(levelInfoTokens[0].c_str());
			endLevel[1] = atoi(levelInfoTokens[1].c_str());
		}
	}


	//////////////////////////////////////////////////////////////////////////
	std::string szResultStateIDList = szStateID[0];
	//Append ";" if missing at the end of szOrigValue, then append szAddValue.
	int nLength = (int)szResultStateIDList.size();

	if (nLength == 0 || szOrigValue[nLength - 1] == ';')
		szResultStateIDList += szStateID[1];
	else
	{
		szResultStateIDList += ";";
		szResultStateIDList += szStateID[1];
	}

	int nResultStartLevel = min(startLevel[0], startLevel[1]);
	int nResultEndLevel = max(endLevel[0], endLevel[1]);
	//////////////////////////////////////////////////////////////////////////

	sprintf(szBuff, "[%s][%d;%d]", szResultStateIDList.c_str(), nResultStartLevel, nResultEndLevel);

	szNewValue = szBuff;
}

void CDnDissolveStateBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	std::string szStateID[2];
	int startLevel[2] = {0, };
	int endLevel[2] = {0, };

	std::string str = szOrigValue;//"[45;46;47;][23:45]";
	std::vector<std::string> tokens;
	std::string delimiters = "[]";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);
	}
	else
	{
		//2. Separate state blow index list
		szStateID[0] = tokens[0];

		//3. Applied level classification
		std::string levelInfo = tokens[1];
		std::vector<std::string> levelInfoTokens;
		delimiters = ":";
		TokenizeA(levelInfo, levelInfoTokens, delimiters);

		if (levelInfoTokens.size() != 2)
		{
			OutputDebug("%s :: %s --> Need to check state effect application level!!!\n", __FUNCTION__, szOrigValue);
		}
		else
		{
			startLevel[0] = atoi(levelInfoTokens[0].c_str());
			endLevel[0] = atoi(levelInfoTokens[1].c_str());
		}
	}


	//////////////////////////////////////////////////////////////////////////
	str = szAddValue;//"[45;46;47;][23:45]";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);
	}
	else
	{
		//2. Separate state blow index list
		szStateID[1] = tokens[0];

		//3. Applied level classification
		std::string levelInfo = tokens[1];
		std::vector<std::string> levelInfoTokens;
		delimiters = ":";
		TokenizeA(levelInfo, levelInfoTokens, delimiters);

		if (levelInfoTokens.size() != 2)
		{
			OutputDebug("%s :: %s --> Need to check state effect application level!!!\n", __FUNCTION__, szAddValue);
		}
		else
		{
			startLevel[1] = atoi(levelInfoTokens[0].c_str());
			endLevel[1] = atoi(levelInfoTokens[1].c_str());
		}
	}


	//////////////////////////////////////////////////////////////////////////
	std::string szResultStateIDList = "";
	
	//"###;###;###;###;....;##;##;##"
	//|<--Previous string->|<-AddValue->|

	//Trim szOrigValue from the end by the length of szAddValue.
	int nOrigLength = (int)szStateID[0].size();
	int nAddLength = (int)szStateID[1].size();
	int nCount = nOrigLength - nAddLength;

	sprintf_s(szBuff, "%s", szStateID[0].c_str());
	szBuff[nCount] = 0;

	szResultStateIDList = szBuff;


	int nResultStartLevel = min(startLevel[0], startLevel[1]);
	int nResultEndLevel = max(endLevel[0], endLevel[1]);
	//////////////////////////////////////////////////////////////////////////

	sprintf(szBuff, "[%s][%d;%d]", szResultStateIDList.c_str(), nResultStartLevel, nResultEndLevel);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
