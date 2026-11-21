#include "stdafx.h"
#include "DnRemoveStateBlow.h"
#if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnRemoveStateBlow::CDnRemoveStateBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_211;
	SetValue( szValue );
	m_fValue = 0.0f;

	SetInfo(szValue);
}

CDnRemoveStateBlow::~CDnRemoveStateBlow(void)
{

}

void CDnRemoveStateBlow::SetInfo(const char* szValue)
{
	std::string str = szValue;//"ID1;ID2;.....;";//List of skill IDs to remove..
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//
	TokenizeA(str, tokens, delimiters);
	
	int nCount = (int)tokens.size();

	for (int i = 0; i < nCount; ++i)
	{
		int nSkillID = atoi(tokens[i].c_str());
		if (nSkillID != 0)
			m_SkillIDList.insert(std::make_pair(nSkillID, nSkillID));
		else
			OutputDebug("%s SkillID[%d]=%s is invalid!!!\n", __FUNCTION__, i, tokens[i].c_str());
	}	
}

void CDnRemoveStateBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	RemoveStateBlowBySkillID();
#endif // _GAMESERVER
}

void CDnRemoveStateBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process( LocalTime, fDelta );
}


void CDnRemoveStateBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);
}

#if defined(_GAMESERVER)
void CDnRemoveStateBlow::RemoveStateBlowBySkillID()
{
	DNVector(DnBlowHandle) vlBlowList;

	FindSkillBlowList(m_hActor, vlBlowList);
	
	int nBlowCount = (int)vlBlowList.size();
	for (int n = 0; n < nBlowCount; ++n)
	{
		DnBlowHandle hBlow = vlBlowList[n];
		if (!hBlow)
			continue;

		// Skip checking skill user actor (state effects added by props have no skill user actor).
		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
			continue;

		//Queue state effect to apply to self.
		m_hActor->CmdRemoveStateEffectFromID(hBlow->GetBlowID());
	}

}

bool CDnRemoveStateBlow::FindSkillID(int nSkillID)
{
	std::map<int, int>::iterator iter = m_SkillIDList.find(nSkillID);
	
	return (iter != m_SkillIDList.end());
}

void CDnRemoveStateBlow::FindSkillBlowList(DnActorHandle hActor, DNVector(DnBlowHandle)& vlBlowList)
{
	if (!hActor)
		return;

	int nStateBlowCount = hActor->GetNumAppliedStateBlow();

	for (int i = 0; i < nStateBlowCount; ++i)
	{
		DnBlowHandle hBlow = hActor->GetAppliedStateBlow(i);
		if (!hBlow)
			continue;

		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		//Skip if no skill info (there may be no skill user, e.g., state effect applied by a prop).
		if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
			continue;

		if (FindSkillID(pSkillInfo->iSkillID))
			vlBlowList.push_back(hBlow);
	}
}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnRemoveStateBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnRemoveStateBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
