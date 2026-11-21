#include "stdafx.h"
#include "DnCoolTimeChangeBlow.h"
#if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnCoolTimeChangeBlow::CDnCoolTimeChangeBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_171;
	SetValue( szValue );
	
	m_nSkillType = -1;
	m_fCoolTimeRate = 1.0f;

	std::string str = szValue;//"SkillType(Active/Passive);CooldownRatio";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_nSkillType = atoi( tokens[0].c_str() );
		m_fCoolTimeRate = (float)atof( tokens[1].c_str() );
	}
	else
		OutputDebug("%s Invalid value!!!\n", __FUNCTION__);
	
}

CDnCoolTimeChangeBlow::~CDnCoolTimeChangeBlow(void)
{
	if (!m_OrigSkillCoolTimeAdjustValues.empty())
		ResetCoolTime();
}

void CDnCoolTimeChangeBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);
	OutputDebug( "CDnCoolTimeChangeBlow::OnBegin\n");

	DNVector(DnSkillHandle) vlSkillList;
	m_hActor->FindSkillBySkillType((CDnSkill::SkillTypeEnum)m_nSkillType, vlSkillList);

	if (vlSkillList.empty())
		return;

	int nCount = (int)vlSkillList.size();
	for (int i = 0; i < nCount; ++i)
	{
		DnSkillHandle hSkill = vlSkillList.at(i);
		if (!hSkill)
			continue;

		//Save Adjust values of existing skills...
		int nSkillID = hSkill->GetClassID();
		float fOrigCoolTimeAdjustValue = hSkill->GetCoolTimeAdjustValue();
		m_OrigSkillCoolTimeAdjustValues.insert(std::map<int, float>::value_type(nSkillID, fOrigCoolTimeAdjustValue));

		//Change skill cooldown ratio.
		hSkill->SetCoolTimeAdjustValue(m_fCoolTimeRate);
	}
}

void CDnCoolTimeChangeBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
}


void CDnCoolTimeChangeBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);
	OutputDebug( "CDnCoolTimeChangeBlow::OnEnd\n");

	ResetCoolTime();
	return;
}

void CDnCoolTimeChangeBlow::ResetCoolTime()
{
	DNVector(DnSkillHandle) vlSkillList;
	m_hActor->FindSkillBySkillType((CDnSkill::SkillTypeEnum)m_nSkillType, vlSkillList);

	if (vlSkillList.empty())
		return;

	int nCount = (int)vlSkillList.size();
	for (int i = 0; i < nCount; ++i)
	{
		DnSkillHandle hSkill = vlSkillList.at(i);
		if (!hSkill)
			continue;

		float fOrigAdjustValue = 1.0f;

		//Find the skill's base cooldown ratio value and restore it to its original state
		int nSkillID = hSkill->GetClassID();
		std::map<int, float>::iterator iter = m_OrigSkillCoolTimeAdjustValues.find(nSkillID);
		if (iter != m_OrigSkillCoolTimeAdjustValues.end())
			fOrigAdjustValue = iter->second;

		hSkill->SetCoolTimeAdjustValue(fOrigAdjustValue);
	}

	m_OrigSkillCoolTimeAdjustValues.clear();
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnCoolTimeChangeBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nSkillType[2] = { 0, };
	float fCoolTimeRate[2] = { 0.0f, };
	
	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		nSkillType[0] = atoi( vlTokens[0][0].c_str() );
		fCoolTimeRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		nSkillType[1] = atoi( vlTokens[1][0].c_str() );
		fCoolTimeRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultSkillType = nSkillType[0];
	float fResultCoolTimeRate = fCoolTimeRate[0] + fCoolTimeRate[1];

	sprintf_s(szBuff, "%d;%f", nResultSkillType, fResultCoolTimeRate);

	szNewValue = szBuff;
}

void CDnCoolTimeChangeBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nSkillType[2] = { 0, };
	float fCoolTimeRate[2] = { 0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		nSkillType[0] = atoi( vlTokens[0][0].c_str() );
		fCoolTimeRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		nSkillType[1] = atoi( vlTokens[1][0].c_str() );
		fCoolTimeRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultSkillType = nSkillType[0];
	float fResultCoolTimeRate = fCoolTimeRate[0] - fCoolTimeRate[1];

	sprintf_s(szBuff, "%d;%f", nResultSkillType, fResultCoolTimeRate);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
