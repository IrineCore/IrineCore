#include "stdafx.h"
#include "DnStealBuffBlow.h"
#include "DnSkill.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnStealBuffBlow::CDnStealBuffBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_174;
	SetValue( szValue );

	m_nCount = 0;
	m_nNewDelayTime = 0;
	m_fScanRange = 0.0f;
	
	std::string str = szValue;//"Count;Duration(ms(10000 == 1 sec))";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (3 == tokens.size())
	{
		m_nCount = atoi( tokens[0].c_str() );
		m_nNewDelayTime = atoi( tokens[1].c_str() );

		m_fScanRange = (float)atof(tokens[2].c_str());
	}
	else
		OutputDebug("%s Invalid value!!!\n", __FUNCTION__);

}

CDnStealBuffBlow::~CDnStealBuffBlow(void)
{

}

void CDnStealBuffBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

#if defined(_GAMESERVER)
	StealBuffProcess(LocalTime);
#endif // _GAMESERVER
}


void CDnStealBuffBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);
}


void CDnStealBuffBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}


#if defined(_GAMESERVER)
void CDnStealBuffBlow::FindSkillBlowList(DnActorHandle hActor, int skillID, DNVector(DnBlowHandle)& vlBlowList)
{
	if (!hActor)
		return;

	int nStateBlowCount = hActor->GetNumAppliedStateBlow();

	for (int i = 0; i < nStateBlowCount; ++i)
	{
		DnBlowHandle hBlow = hActor->GetAppliedStateBlow(i);

		//Skip if StealBuff.
		if (!hBlow || STATE_BLOW::BLOW_174 == hBlow->GetBlowIndex())
			continue;

		//Invisibility state effect
		if (STATE_BLOW::BLOW_073 == hBlow->GetBlowIndex())
			continue;
		
		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		//Skip if no skill info (there may be no skill user, e.g., state effect applied by a prop).
		if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
			continue;

		if (pSkillInfo->iSkillID == skillID)
			vlBlowList.push_back(hBlow);
	}
}

void CDnStealBuffBlow::StealBuffProcess()
{
	if (m_nCount <= 0) return;

	if (!m_hActor) return;

	if (!m_ParentSkillInfo.hSkillUser)
		return;

	//List of Blows to remove
	DNVector(DnBlowHandle) vlRemoveBlowList;
	typedef std::map<int, int> SKILL_ID_LIST;
	SKILL_ID_LIST skillList;

	int nLoopCount = m_nCount;
	int nStateBlowCount = m_hActor->GetNumAppliedStateBlow();

	//1. Collect skill IDs
	for (int i = 0; i < nStateBlowCount; ++i)
	{
		DnBlowHandle hBlow = m_hActor->GetAppliedStateBlow(i);

		//Skip if StealBuff.
		if (!hBlow || STATE_BLOW::BLOW_174 == hBlow->GetBlowIndex())
			continue;

		//Invisibility state effect
		if (STATE_BLOW::BLOW_073 == hBlow->GetBlowIndex())
			continue;

		//Skip if no skill info or not a buff.
		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		if (!pSkillInfo || CDnSkill::DurationTypeEnum::Buff != pSkillInfo->eDurationType )
			continue;

		//Active skills only..
		if (CDnSkill::Active != pSkillInfo->eSkillType)
			continue;

		skillList.insert(SKILL_ID_LIST::value_type(pSkillInfo->iSkillID, pSkillInfo->iSkillID));	
	}

	//2. Collect Blows attached to the skill and apply them to the new character
	SKILL_ID_LIST::iterator iter = skillList.begin();
	SKILL_ID_LIST::iterator endIter = skillList.end();
	for (; iter != endIter; ++iter)
	{
		if (nLoopCount == 0)
			break;

		//Retrieve blows of the skill..
		DNVector(DnBlowHandle) vlSkillBlowList;
		FindSkillBlowList(m_hActor, iter->first, vlSkillBlowList);

		int nBlowCount = (int)vlSkillBlowList.size();
		for (int n = 0; n < nBlowCount; ++n)
		{
			DnBlowHandle hBlow = vlSkillBlowList[n];
			if (!hBlow)
				continue;

			CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
			if (!pSkillInfo || !pSkillInfo->hSkillUser)
				continue;

			//Get skill handle from the user who cast the skill; skip if none
			DnSkillHandle hSkill = pSkillInfo->hSkillUser->FindSkill(pSkillInfo->iSkillID);
			if (!hSkill)
				continue;

			//If no specific weapon is required
			if (CDnWeapon::EquipTypeEnum_Amount == hSkill->GetNeedEquipType(0) &&
				CDnWeapon::EquipTypeEnum_Amount == hSkill->GetNeedEquipType(1))
			{
				//Register the removable state effect to the user who cast the skill with a new duration.
				m_ParentSkillInfo.hSkillUser->CmdAddStateEffect(pSkillInfo, hBlow->GetBlowIndex(), m_nNewDelayTime, hBlow->GetValue(), false, false);
			}
		}

		nLoopCount--;
	}
}

void CDnStealBuffBlow::StealBuffProcess(LOCAL_TIME LocalTime)
{
	DNVector(DnActorHandle) VecList;

	m_hActor->ScanActor( m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fScanRange, VecList );

	for( DWORD i=0; i<VecList.size(); i++ ) {
		if( !VecList[i] || !VecList[i]->IsHittable( m_hActor, LocalTime, NULL ) ) 
			continue;

		if (VecList[i]->GetTeam() == m_hActor->GetTeam())
			continue;

		StealBuffFromActor(VecList[i]);
	}
}

void CDnStealBuffBlow::StealBuffFromActor( DnActorHandle hTargetActor )
{
	if (m_nCount <= 0) return;
	if (!m_hActor) return;

	//Immunity handling...
	if (!hTargetActor || hTargetActor->IsImmuned(m_StateBlow.emBlowIndex)) return;

	//List of Blows to remove
	DNVector(DnBlowHandle) vlRemoveBlowList;
	typedef std::map<int, int> SKILL_ID_LIST;
	SKILL_ID_LIST skillList;

	int nLoopCount = m_nCount;
	int nStateBlowCount = hTargetActor->GetNumAppliedStateBlow();

	//1. Collect skill IDs
	for (int i = 0; i < nStateBlowCount; ++i)
	{
		DnBlowHandle hBlow = hTargetActor->GetAppliedStateBlow(i);

		//Skip if StealBuff.
		if (!hBlow || STATE_BLOW::BLOW_174 == hBlow->GetBlowIndex())
			continue;

		//Invisibility state effect
		if (STATE_BLOW::BLOW_073 == hBlow->GetBlowIndex())
			continue;

		//Skip if no skill info or not a buff.
		CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
		if (!pSkillInfo || CDnSkill::DurationTypeEnum::Buff != pSkillInfo->eDurationType )
			continue;

		//Active skills only..
		if (CDnSkill::Active != pSkillInfo->eSkillType)
			continue;

		skillList.insert(SKILL_ID_LIST::value_type(pSkillInfo->iSkillID, pSkillInfo->iSkillID));	
	}

	//2. Collect Blows attached to the skill and apply them to the new character
	SKILL_ID_LIST::iterator iter = skillList.begin();
	SKILL_ID_LIST::iterator endIter = skillList.end();
	for (; iter != endIter; ++iter)
	{
		if (nLoopCount == 0)
			break;

		//Retrieve blows of the skill..
		DNVector(DnBlowHandle) vlSkillBlowList;
		FindSkillBlowList(hTargetActor, iter->first, vlSkillBlowList);

		int nBlowCount = (int)vlSkillBlowList.size();
		for (int n = 0; n < nBlowCount; ++n)
		{
			DnBlowHandle hBlow = vlSkillBlowList[n];
			if (!hBlow)
				continue;

			// [2011/03/30 semozz]
			// Skip checking skill user actor (state effects added by props have no skill user actor).
			CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hBlow->GetParentSkillInfo());
			if (!pSkillInfo /*|| !pSkillInfo->hSkillUser*/)
				continue;

			//When skill user info is set
			if (pSkillInfo->hSkillUser)
			{
				//Check skill if skill user info exists.
				DnSkillHandle hSkill = pSkillInfo->hSkillUser->FindSkill(pSkillInfo->iSkillID);
				//Skip if no skill.
				if (!hSkill)
					continue;

				//If a searched skill exists, skip if it requires a specific weapon
				if (CDnWeapon::EquipTypeEnum_Amount != hSkill->GetNeedEquipType(0) ||
					CDnWeapon::EquipTypeEnum_Amount != hSkill->GetNeedEquipType(1))
						continue;
			}
 			
			//Queue state effect to apply to self.
			m_hActor->ReserveStealMagicBuff(hTargetActor, pSkillInfo, m_nNewDelayTime, hBlow->GetBlowIndex(), hBlow->GetValue());
		}

		nLoopCount--;
	}
}

#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnStealBuffBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nCount[2] = {0, };
	int nNewDelayTime[2] = {0, };
	float fScanRange[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		nCount[0] = atoi( vlTokens[0][0].c_str() );
		nNewDelayTime[0] = atoi( vlTokens[0][1].c_str() );
		fScanRange[0] = (float)atof( vlTokens[0][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		nCount[1] = atoi( vlTokens[1][0].c_str() );
		nNewDelayTime[1] = atoi( vlTokens[1][1].c_str() );
		fScanRange[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultCount = nCount[0] + nCount[1];
	int nResultDelayTime = max(nNewDelayTime[0], nNewDelayTime[1]);
	float fResultScanRange = max(fScanRange[0], fScanRange[1]);

	sprintf_s(szBuff, "%d;%d;%f", nResultCount, nResultDelayTime, fResultScanRange);

	szNewValue = szBuff;
}

void CDnStealBuffBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nCount[2] = {0, };
	int nNewDelayTime[2] = {0, };
	float fScanRange[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		nCount[0] = atoi( vlTokens[0][0].c_str() );
		nNewDelayTime[0] = atoi( vlTokens[0][1].c_str() );
		fScanRange[0] = (float)atof( vlTokens[0][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		nCount[1] = atoi( vlTokens[1][0].c_str() );
		nNewDelayTime[1] = atoi( vlTokens[1][1].c_str() );
		fScanRange[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultCount = nCount[0] - nCount[1];
	int nResultDelayTime = min(nNewDelayTime[0], nNewDelayTime[1]);
	float fResultScanRange = min(fScanRange[0], fScanRange[1]);

	sprintf_s(szBuff, "%d;%d;%f", nResultCount, nResultDelayTime, fResultScanRange);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
