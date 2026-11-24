#include "stdafx.h"
#include "DnAddDamageOnStateBlow.h"
#include "DnSkill.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnAddDamageOnStateBlow::CDnAddDamageOnStateBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_166;
	SetValue( szValue );

	m_nDestStateBlowIndex = -1;
	m_fAddDamageRate = 0.0f;
	
	std::string str = szValue;//"StateEffectIndex;ExtraDamageRatio";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_nDestStateBlowIndex = atoi( tokens[0].c_str() );
		m_fAddDamageRate = (float)atof( tokens[1].c_str() );
	}
	else
		OutputDebug("%s Invalid value!!!\n", __FUNCTION__);

#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
}

CDnAddDamageOnStateBlow::~CDnAddDamageOnStateBlow(void)
{

}

void CDnAddDamageOnStateBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}


void CDnAddDamageOnStateBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process(LocalTime, fDelta);
}


void CDnAddDamageOnStateBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
}

#if defined(_GAMESERVER)
void CDnAddDamageOnStateBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
	if (!hTargetActor || hTargetActor->IsDie())
		return;

	CDnDamageBase::SHitParam* pHitParam = hTargetActor->GetHitParam();

	if (hTargetActor->IsAppliedThisStateBlow((STATE_BLOW::emBLOW_INDEX)m_nDestStateBlowIndex))
	{
		int nAddDamage = (int)(pHitParam->nCalcDamage * m_fAddDamageRate);

		//hTargetActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)fAddDamage);
		char buffer[65];
		_itoa_s(nAddDamage, buffer, 65, 10 );
		hTargetActor->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer, false, false);
	}

}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAddDamageOnStateBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nDestStateBlowIndex[2] = {0, };
	float fAddDamageRate[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		nDestStateBlowIndex[0] = atoi( vlTokens[0][0].c_str() );
		fAddDamageRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		nDestStateBlowIndex[1] = atoi( vlTokens[1][0].c_str() );
		fAddDamageRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultDestBlowIndex = nDestStateBlowIndex[0];
	float fResultAddDamageRate = fAddDamageRate[0] + fAddDamageRate[1];

	sprintf_s(szBuff, "%d;%f", nResultDestBlowIndex, fResultAddDamageRate);

	szNewValue = szBuff;


}

void CDnAddDamageOnStateBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int nDestStateBlowIndex[2] = {0, };
	float fAddDamageRate[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		nDestStateBlowIndex[0] = atoi( vlTokens[0][0].c_str() );
		fAddDamageRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		nDestStateBlowIndex[1] = atoi( vlTokens[1][0].c_str() );
		fAddDamageRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int nResultDestBlowIndex = nDestStateBlowIndex[0];
	float fResultAddDamageRate = fAddDamageRate[0] - fAddDamageRate[1];

	sprintf_s(szBuff, "%d;%f", nResultDestBlowIndex, fResultAddDamageRate);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW