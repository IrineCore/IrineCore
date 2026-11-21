#include "stdafx.h"
#include "DnTimeBoomBlow.h"
#include "DnSkill.h"
#ifdef _GAMESERVER
#include "DnHighLanderBlow.h"
#endif 

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnTimeBoomBlow::CDnTimeBoomBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_170;
	SetValue( szValue );
	
	std::string str = szValue;//"ExplosionRange;ExplosionDamage";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_fLimitRadius = (float)atof( tokens[0].c_str() );
		m_nBoomDamage = atoi(tokens[1].c_str());
	}
	else
	{
		m_fLimitRadius = 0.0f;
		m_nBoomDamage = 0;

		OutputDebug("%s Invalid Value!!!!!!\n", __FUNCTION__);
	}
	
// #ifndef _GAMESERVER
// 	UseTableDefinedGraphicEffect( false );
// #endif
}

CDnTimeBoomBlow::~CDnTimeBoomBlow(void)
{

}

void CDnTimeBoomBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

	if( m_fLimitRadius == 0.0f || m_nBoomDamage == 0.0f)
	{
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnTimeBoomBlow::Damage or IntervalTime was wrong!!\n" );
	}
}

void CDnTimeBoomBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	//OutputDebug( "CDnFireBurnBlow::Process, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}


void CDnTimeBoomBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);

#if defined(_GAMESERVER)
	if (m_hActor && !m_hActor->IsDie())
	{
		DNVector(DnActorHandle) hVecList;

		//Check the actor that used the skill.
		DnActorHandle hSkillActor;
		const CDnSkill::SkillInfo* pSkillInfo = GetParentSkillInfo();
		if (pSkillInfo)
			hSkillActor = pSkillInfo->hSkillUser;

		//Search targets based on the actor to which this state effect is applied
		m_hActor->ScanActor( m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fLimitRadius, hVecList );

		for( DWORD i=0; i<hVecList.size(); i++ ) {
			if( !hVecList[i]->IsHittable( m_hActor, LocalTime, NULL ) ) 
				continue;

			//If there is no skill user actor, same team as the actor with this state effect
			if (!hSkillActor)
			{
				if (hVecList[i]->GetTeam() != m_hActor->GetTeam())
					continue;
			}
			//If there is a skill user actor, different team from the skill user actor
			else
			{
				if (hVecList[i]->GetTeam() == hSkillActor->GetTeam())
					continue;
			}
			

#if defined(PRE_FIX_61382)
			DnActorHandle hActor = CDnActor::GetOwnerActorHandle(hVecList[i]);
			if (hActor &&
				hActor->IsDie() == false)
			{
				hVecList[i]->RequestDamageFromStateBlow(GetMySmartPtr(), m_nBoomDamage);

				if (hActor->IsDie())
					hActor->Die(m_hActor);
			}
#else
			hVecList[i]->RequestDamageFromStateBlow(GetMySmartPtr(), m_nBoomDamage);

			// Check if dead
			if( hVecList[i]->GetHP() <= 0.f )
				hVecList[i]->Die( m_hActor	);
#endif // PRE_FIX_61382
		}
	}
#endif // _GAMESERVER
	OutputDebug( "CDnFreezingShieldBlow::OnEnd\n");
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnTimeBoomBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fLimitRadius[2] = { 0.0f, };
	int nBoomDamage[2] = { 0, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fLimitRadius[0] = (float)atof( vlTokens[0][0].c_str() );
		nBoomDamage[0] = atoi( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fLimitRadius[1] = (float)atof( vlTokens[1][0].c_str() );
		nBoomDamage[1] = atoi( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultLimitRadius = max(fLimitRadius[0], fLimitRadius[1]);
	int nResultBoomDamage = nBoomDamage[0] + nBoomDamage[1];

	sprintf_s(szBuff, "%f;%d", fResultLimitRadius, nResultBoomDamage);

	szNewValue = szBuff;
}

void CDnTimeBoomBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fLimitRadius[2] = { 0.0f, };
	int nBoomDamage[2] = { 0, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fLimitRadius[0] = (float)atof( vlTokens[0][0].c_str() );
		nBoomDamage[0] = atoi( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fLimitRadius[1] = (float)atof( vlTokens[1][0].c_str() );
		nBoomDamage[1] = atoi( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultLimitRadius = min(fLimitRadius[0], fLimitRadius[1]);
	int nResultBoomDamage = nBoomDamage[0] - nBoomDamage[1];

	sprintf_s(szBuff, "%f;%d", fResultLimitRadius, nResultBoomDamage);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
