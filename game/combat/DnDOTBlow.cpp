#include "stdafx.h"
#include "DnDOTBlow.h"
#include "DnSkill.h"
#ifdef _GAMESERVER
#include "DnHighLanderBlow.h"
#endif 

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

const LOCAL_TIME FIRE_DAMAGE_INTERVAL = 2000;


CDnDOTBlow::CDnDOTBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ), 
m_IntervalChecker( hActor, GetMySmartPtr() ), 
m_fEffectLength( 0.0f ),
m_bEffecting( false ),
m_IntervalTime(0)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_157;
	SetValue( szValue );
	
	std::string str = szValue;//"damage;IntervalTime";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_fValue = (float)atof( tokens[0].c_str() );
		m_IntervalTime = atoi(tokens[1].c_str());
	}
	else
	{
		m_fValue = 0.0f;
		m_IntervalTime = 0;
	}

	m_IntervalDamage = 0.0f;
	m_nDamageCount = 0;
	
#ifndef _GAMESERVER
	UseTableDefinedGraphicEffect( false );
#endif
}

CDnDOTBlow::~CDnDOTBlow(void)
{

}

void CDnDOTBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	if( m_pEffectOutputInfo ) {	// Added due to crash.. by realgaia
		_ASSERT( EffectOutputInfo::OWNER_HANDLING != m_pEffectOutputInfo->iOutputType );
	}
#else

	// [2011/02/10 semozz]
	// Change setting so damage reduces % of the Actor's max HP.
	
	// 1. How many IntervalTime ticks occur within the state effect's total duration
	float nCount = m_StateBlow.fDurationTime / (m_IntervalTime * 0.001f);
	
	//Store the damage count.
	m_nDamageCount = (int)nCount;

	// 2. Total damage (n% of midpoint of attacker's max/min attack power)
	
	// Determine whether it is a physical or magic attack.
	float fTotalDamage = 0.0f;
	CDnDamageBase::SHitParam *pHitparam = m_hActor->GetHitParam();
	switch(pHitparam->cAttackType)
	{
	case 0://Physical attack
		{
			if (m_ParentSkillInfo.hSkillUser)
			{
				float fDiff = (float)(m_ParentSkillInfo.hSkillUser->GetAttackPMax() - m_ParentSkillInfo.hSkillUser->GetAttackPMin());
				fTotalDamage = (float)m_ParentSkillInfo.hSkillUser->GetAttackPMin() + (fDiff * 0.5f);
			}
		}
		break;
	case 1://Magic attack
		{
			if (m_ParentSkillInfo.hSkillUser)
			{
				float fDiff = (float)(m_ParentSkillInfo.hSkillUser->GetAttackMMax() - m_ParentSkillInfo.hSkillUser->GetAttackMMin());
				fTotalDamage = (float)m_ParentSkillInfo.hSkillUser->GetAttackMMin() + (fDiff * 0.5f);
			}
		}
		break;
	}

	fTotalDamage = fTotalDamage * m_fValue;

	// 3. Calculate damage per IntervalTime
	m_IntervalDamage = fTotalDamage / nCount;

	
	if( m_fValue == 0.0f || m_IntervalTime == 0.0f)
	{
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnDOTBlow::Damage or IntervalTime was wrong!!\n" );
	}
#endif

	m_IntervalChecker.OnBegin( LocalTime, static_cast<DWORD>(m_IntervalTime) );

#if defined(_GAMESERVER)
	RequestDamage();
#endif // _GAMESERVER

	m_bEffecting = false;
	m_fEffectLength = 0.0f;

#if !defined(_GAMESERVER)
	_AttachGraphicEffect();
#endif // _GAMESERVER

	OutputDebug( "CDnDOTBlow::OnBegin, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}

bool CDnDOTBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	RequestDamage();
#else
#endif

	return true;
}


void CDnDOTBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
	m_IntervalChecker.Process( LocalTime, fDelta );
}


void CDnDOTBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

#ifndef _GAMESERVER
	_DetachGraphicEffect();
#else
	//If damage count remains, multiply by remaining count, set remaining count to 1, and invoke damage
	if (m_nDamageCount > 0 && m_hActor->IsDie() == false)
	{
		m_IntervalDamage = m_IntervalDamage * m_nDamageCount;
		m_nDamageCount = 1;

		RequestDamage();

#if defined(PRE_FIX_61382)
		//In #48491, Die is called from ActorStatIntervlaManipulator when applying damage via timer, but OnEnd still needs to handle Die.
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if (hActor && hActor->IsDie())
			hActor->Die(m_ParentSkillInfo.hSkillUser);
#else
		//In #48491, Die is called from ActorStatIntervlaManipulator when applying damage via timer, but OnEnd still needs to handle Die.
		if (m_hActor->IsDie())
			m_hActor->Die(m_ParentSkillInfo.hSkillUser);
#endif // PRE_FIX_61382
	}
#endif
 
	OutputDebug( "CDnDOTBlow::OnEnd, (HP : %d ) \n", m_hActor->GetHP());
}


#if defined(_GAMESERVER)
bool CDnDOTBlow::CalcDuplicateValue( const char* szValue )
{
	float fValue = 0.0;
	
	std::string str = szValue;//"damage;IntervalTime";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		fValue = (float)atof( tokens[0].c_str() );
	}
	
	//Keep the duration and only increase damage (temporary).

	m_fValue += fValue;

	return true;
}

void CDnDOTBlow::RequestDamage()
{
	//If there is damage and remaining count..
	if (0.0f < m_IntervalDamage && 
		m_nDamageCount > 0)
	{
#if defined(PRE_FIX_44884)
#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if (hActor &&
			hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), static_cast<int>(m_IntervalDamage));

			//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
			//No need to call Die here.
			//if (m_hActor->IsDie())
			//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
		}
#else
		if (m_hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), static_cast<int>(m_IntervalDamage));

			//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
			//No need to call Die here.
			//if (m_hActor->IsDie())
			//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
		}
#endif // PRE_FIX_61382
#else
		// Must not die while the Highlander state effect is active.
		float fDamage = m_IntervalDamage;
		if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_143 ) )
		{
			DNVector( DnBlowHandle ) vlhHighLanderBlow;
			m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_143, vlhHighLanderBlow );
			_ASSERT( 1 == (int)vlhHighLanderBlow.size() );
			CDnHighlanderBlow* pHighlanderBlow = static_cast<CDnHighlanderBlow*>(vlhHighLanderBlow.front().GetPointer());
			fDamage += pHighlanderBlow->CalcDamage( fDamage );
		}

		DWORD dwHitterUniqueID = m_ParentSkillInfo.hSkillUser ? m_ParentSkillInfo.hSkillUser->GetUniqueID() : -1;
		m_hActor->SetHP( m_hActor->GetHP()-(INT64)fDamage );
		m_hActor->RequestHPMPDelta( m_ParentSkillInfo.eSkillElement, -(INT64)fDamage, dwHitterUniqueID );
#endif // PRE_FIX_44884

		m_nDamageCount--;
	}
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnDOTBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fValue[2] = {0.0f, };
	float fIntervalTime[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		fIntervalTime[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		fIntervalTime[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];
	float fResultIntervalTime = min(fIntervalTime[0], fIntervalTime[1]);
	
	sprintf_s(szBuff, "%f;%f", fResultValue, fResultIntervalTime);

	szNewValue = szBuff;
}

void CDnDOTBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fValue[2] = {0.0f, };
	float fIntervalTime[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) 
	{
		fValue[0] = (float)atof( vlTokens[0][0].c_str() );
		fIntervalTime[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) 
	{
		fValue[1] = (float)atof( vlTokens[1][0].c_str() );
		fIntervalTime[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] - fValue[1];
	float fResultIntervalTime = fIntervalTime[0] - fIntervalTime[1];

	sprintf_s(szBuff, "%f;%f", fResultValue, fResultIntervalTime);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
