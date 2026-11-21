#include "stdafx.h"
#include "DnFreezingShieldBlow.h"
#if defined( _GAMESERVER )
#include "DnActor.h"
#endif // #if defined( _GAMESERVER )

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnFreezingShieldBlow::CDnFreezingShieldBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_173;
	SetValue( szValue );
	
	m_nDuration = 0;

	SetInfo(szValue);

#if defined(_GAMESERVER)
	AddCallBackType( SB_ONDEFENSEATTACK );
#endif // _GAMESERVER
}

CDnFreezingShieldBlow::~CDnFreezingShieldBlow(void)
{

}

void CDnFreezingShieldBlow::SetInfo(const char* szValue)
{
	std::string str = szValue;//"[MagicAttackRatio;AttackRange;FreezeDuration][FreezeSettingValue]";
	std::vector<std::string> tokens;
	std::string delimiters = "[]";

	//1. Category
	TokenizeA(str, tokens, delimiters);

	if (tokens.size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);
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
			m_fRate = (float)atof( infoTokens[0].c_str() );
			m_fLimitRadius = (float)atof( infoTokens[1].c_str() );
			m_nFreezingTime = atoi(infoTokens[2].c_str());
		}
		else
		{
			m_fRate = 0.0f;
			m_fLimitRadius = 0.0f;
			m_nFreezingTime = 0;

			OutputDebug("%s Invalid Value!!!!\n", __FUNCTION__);
		}

		//3. State effect config value
		m_strStateAttribute = tokens[1];
	}
}


void CDnFreezingShieldBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);

	if (m_hActor)
	{
		//Actor magic attack * ratio => determines barrier durability..
		m_nDuration = int((float)m_hActor->GetAttackMMax() * m_fRate);
	}
	
	OutputDebug( "CDnFreezingShieldBlow::OnBegin, Magic Attack : %d , Ratio : %f, Durability : %d\n", m_hActor->GetAttackMMax(), m_fRate, m_nDuration );
}

void CDnFreezingShieldBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	//On the server side, keep active for the duration or until the shield reaches 0
	if( STATE_BLOW::STATE_END == m_StateBlow.emBlowState || m_nDuration <= 0 )
	{
		// Send only state effect removal packet to client; server releases blow normally.
		m_hActor->CmdRemoveStateEffect( m_StateBlow.emBlowIndex, false );
		SetState( STATE_BLOW::STATE_END );
	}
#else
	//Keep client running and end via server packet...
	SetState(STATE_BLOW::STATE_DURATION);
#endif
}


void CDnFreezingShieldBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

#if defined(_GAMESERVER)
	if (m_hActor && !m_hActor->IsDie())
	{
		DNVector(DnActorHandle) hVecList;

		m_hActor->ScanActor( m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fLimitRadius, hVecList );

		// Use effect #3 in state effect #41 [2011/03/07 semozz]
		char szBuff[128] = {0, };
		_snprintf_s(szBuff, _countof(szBuff), _TRUNCATE, "%d", 3 );
		m_ParentSkillInfo.szEffectOutputIDs = szBuff;
		m_ParentSkillInfo.szEffectOutputIDToClient = szBuff;

		for( DWORD i=0; i<hVecList.size(); i++ ) {
			if( !hVecList[i]->IsHittable( m_hActor, LocalTime, NULL ) ) 
				continue;

			if (hVecList[i]->GetTeam() == m_hActor->GetTeam())
				continue;

			hVecList[i]->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_041, m_nFreezingTime, m_strStateAttribute.c_str(), true);
		}
	}
#endif // _GAMESERVER
	OutputDebug( "CDnFreezingShieldBlow::OnEnd\n");
}


#ifdef _GAMESERVER
bool CDnFreezingShieldBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	//  #25797 Do not trigger if bIgnoreParring is enabled on the hit signal.
	if( HitParam.bIgnoreParring )
		return false;

#if defined(PRE_FIX_BLOCK_CONDITION)
	//Allow block to trigger if attacker is an enemy even when fDamage is 0..
	//For allies, do not block when fDamage is 0 as before.
	if (IsCanBlock(hHitter, m_hActor, HitParam) == false)
		return false;
#else
	// Changed so it does not trigger when hit percent is 0%. (#21175)
	if( 0.0f == HitParam.fDamage )
		return false;
#endif // PRE_FIX_BLOCK_CONDITION

#if defined(PRE_ADD_49166)
	// Prevent block from triggering if target has Freezing / FrameStop debuff.
	if (IsInVaildBlockCondition(m_hActor) == true)
		return false;
#endif // PRE_ADD_49166

	bool bResult = false;

	float fDamage = m_hActor->PreCalcDamage(hHitter, HitParam);
	
	if( m_nDuration >= fDamage )
	{
		m_nDuration -= (int)fDamage;

		//Prevent hit motion from playing.
		HitParam.szActionName.assign( "" );	
		
		bResult = true;
	}
	else
	{
		//If damage exceeds remaining durability, set durability to 0
		//Perform hit motion.
		m_nDuration = 0;
	}

	return bResult;
}
#endif


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFreezingShieldBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	std::string strValue[2];
	std::vector<std::string> tokens[2];
	std::string delimiters = "[]";


	float fRate[2] = {0.0f, };
	float fLimitRadius[2] = {0.0f, };
	int nFreezingTime[2] = {0, };
	
	std::string szArgment[2];


	strValue[0] = szOrigValue;
	//1. Category
	TokenizeA(strValue[0], tokens[0], delimiters);

	if (tokens[0].size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);
	}
	else
	{
		//2. Basic info
		std::string defaultInfo = tokens[0][0];
		std::vector<std::string> infoTokens;
		delimiters = ";";

		TokenizeA(defaultInfo, infoTokens, delimiters);
		if (5 == infoTokens.size())
		{
			fRate[0] = (float)atof(infoTokens[0].c_str());
			fLimitRadius[0] = (float)atof(infoTokens[1].c_str());
			nFreezingTime[0] = atoi(infoTokens[2].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);

		//3. State effect config value
		szArgment[0] = tokens[0][1];
	}

	//////////////////////////////////////////////////////////////////////////
	strValue[1] = szAddValue;
	//1. Category
	TokenizeA(strValue[1], tokens[1], delimiters);

	if (tokens[1].size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);
	}
	else
	{
		//2. Basic info
		std::string defaultInfo = tokens[1][0];
		std::vector<std::string> infoTokens;
		delimiters = ";";

		TokenizeA(defaultInfo, infoTokens, delimiters);
		if (5 == infoTokens.size())
		{
			fRate[1] = (float)atof(infoTokens[0].c_str());
			fLimitRadius[1] = (float)atof(infoTokens[1].c_str());
			nFreezingTime[1] = atoi(infoTokens[2].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);

		//3. State effect config value
		szArgment[1] = tokens[1][1];
	}


	float fResultRate = fRate[0] + fRate[1];
	float fResultLimitRadius = max(fLimitRadius[0], fLimitRadius[1]);
	int nResultFreezingTime = max(nFreezingTime[0], nFreezingTime[1]);
	
	sprintf_s(szBuff, "[%f;%f;%d;][%s]", fResultRate, fResultLimitRadius, nResultFreezingTime, szArgment[0].c_str());

	szNewValue = szBuff;
}

void CDnFreezingShieldBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	std::string strValue[2];
	std::vector<std::string> tokens[2];
	std::string delimiters = "[]";


	float fRate[2] = {0.0f, };
	float fLimitRadius[2] = {0.0f, };
	int nFreezingTime[2] = {0, };

	std::string szArgment[2];


	strValue[0] = szOrigValue;
	//1. Category
	TokenizeA(strValue[0], tokens[0], delimiters);

	if (tokens[0].size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);
	}
	else
	{
		//2. Basic info
		std::string defaultInfo = tokens[0][0];
		std::vector<std::string> infoTokens;
		delimiters = ";";

		TokenizeA(defaultInfo, infoTokens, delimiters);
		if (5 == infoTokens.size())
		{
			fRate[0] = (float)atof(infoTokens[0].c_str());
			fLimitRadius[0] = (float)atof(infoTokens[1].c_str());
			nFreezingTime[0] = atoi(infoTokens[2].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szOrigValue);

		//3. State effect config value
		szArgment[0] = tokens[0][1];
	}

	//////////////////////////////////////////////////////////////////////////
	strValue[1] = szAddValue;
	//1. Category
	TokenizeA(strValue[1], tokens[1], delimiters);

	if (tokens[1].size() != 2)
	{
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);
	}
	else
	{
		//2. Basic info
		std::string defaultInfo = tokens[1][0];
		std::vector<std::string> infoTokens;
		delimiters = ";";

		TokenizeA(defaultInfo, infoTokens, delimiters);
		if (5 == infoTokens.size())
		{
			fRate[1] = (float)atof(infoTokens[0].c_str());
			fLimitRadius[1] = (float)atof(infoTokens[1].c_str());
			nFreezingTime[1] = atoi(infoTokens[2].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);

		//3. State effect config value
		szArgment[1] = tokens[1][1];
	}


	float fResultRate = fRate[0] - fRate[1];
	float fResultLimitRadius = min(fLimitRadius[0], fLimitRadius[1]);
	int nResultFreezingTime = min(nFreezingTime[0], nFreezingTime[1]);

	sprintf_s(szBuff, "[%f;%f;%d;][%s]", fResultRate, fResultLimitRadius, nResultFreezingTime, szArgment[0].c_str());

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
