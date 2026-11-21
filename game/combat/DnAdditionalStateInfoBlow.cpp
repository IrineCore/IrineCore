#include "stdafx.h"
#include "DnAdditionalStateInfoBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnAdditionalStateInfoBlow::CDnAdditionalStateInfoBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_253;
	SetValue( szValue );
	m_fValue = (float)atof( szValue );

	SetInfo(szValue);
}

CDnAdditionalStateInfoBlow::~CDnAdditionalStateInfoBlow(void)
{

}

void CDnAdditionalStateInfoBlow::SetInfo(const char* szValue)
{
	std::string str = szValue;//"[ConditionStateEffectIndex;TargetStateEffectIndex;ApplyType][StateEffectSettingValue]";
	std::vector<std::string> tokens;
	std::string delimiters = "[]";

	//1. Category
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);

		SetState(STATE_BLOW::STATE_END);
	}
	else
	{
		//2. Basic info
		std::string defaultInfo = tokens[0];
		std::vector<std::string> infoTokens;
		delimiters = ";";

		TokenizeA(defaultInfo, infoTokens, delimiters);
		if (3 == infoTokens.size())
		{
			m_DestStateIndex = (STATE_BLOW::emBLOW_INDEX)(atoi(infoTokens[0].c_str()));	//State effect index for condition check

			m_TargetStateInfo.nID = atoi(infoTokens[1].c_str());											//Index of state effect to add
			m_TargetStateInfo.ApplyType = (CDnSkill::StateEffectApplyType)(atoi(infoTokens[2].c_str()));	//ApplyType of state effect to add
			//m_TargetStateInfo.nDurationTime = m_StateBlow.fDurationTime * 1000.0f; // Duration of the state effect to add.
			m_TargetStateInfo.bAddtionalStateInfo = true;

		}
		else
		{
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);
			SetState(STATE_BLOW::STATE_END);
		}

		//3. State effect config value
		m_TargetStateInfo.szValue = tokens[1];
	}
}

void CDnAdditionalStateInfoBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAggroResetBlow::OnBegin\n");
}


void CDnAdditionalStateInfoBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnAdditionalStateInfoBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAggroResetBlow::OnEnd\n");
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAdditionalStateInfoBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnAdditionalStateInfoBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
