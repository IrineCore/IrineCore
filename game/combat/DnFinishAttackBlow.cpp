#include "stdafx.h"
#include "DnFinishAttackBlow.h"
#include "DnSkill.h"
#include "DnProjectile.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnFinishAttackBlow::CDnFinishAttackBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_159;
	SetValue( szValue );

	m_fDestHpRate = 0.0f;
	m_fAddDamageRate = 0.0f;

	std::string str = szValue;//"Count;Duration(ms(10000 == 1 sec))";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	//1. Separate state effect index list and applied level
	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_fDestHpRate = (float)atof( tokens[0].c_str() );
		m_fAddDamageRate = (float)atof( tokens[1].c_str() );
	}
	else
		OutputDebug("%s Invalid value!!!\n", __FUNCTION__);

#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );
#endif
}

CDnFinishAttackBlow::~CDnFinishAttackBlow(void)
{

}

void CDnFinishAttackBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnBegin(LocalTime, fDelta);
}


void CDnFinishAttackBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process(LocalTime, fDelta);
}


void CDnFinishAttackBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	__super::OnEnd(LocalTime, fDelta);
}

#if defined(_GAMESERVER)
void CDnFinishAttackBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
	if (!hTargetActor || hTargetActor->IsDie())
		return;

	CDnDamageBase::SHitParam* pHitParam = hTargetActor->GetHitParam();

	if( pHitParam->bFromProjectile && pHitParam->hWeapon )
	{
		if( pHitParam->hWeapon->GetWeaponType() ==  CDnWeapon::Projectile )
		{
			CDnProjectile *pProjectile = static_cast<CDnProjectile *>(pHitParam->hWeapon.GetPointer());
			if( pProjectile && pProjectile->GetParentSkill() )
			{
				if( pProjectile->GetParentSkill()->GetClassID() != GetParentSkillInfo()->iSkillID )
					return;
			}
		}
	}

	float fCurrentHPRate = hTargetActor->GetHPPercentFloat();
	if (fCurrentHPRate <= m_fDestHpRate)
	{
		//Calculate bonus damage ratio per %
		//m_fDestHpRate = 0.4(40%), fCurrentHPRate = 0.3(30) = 0.1(10%)
		//Multiply by 100.0f to calculate extra damage % per 1% (0.1 = 10%), then apply extra damage ratio per %.
		float fDamageRate = ((m_fDestHpRate - fCurrentHPRate) * 100.0f) * m_fAddDamageRate;

		int nAddDamage = (int)(fDamageRate * pHitParam->nCalcDamage);

		//hTargetActor->RequestDamageFromStateBlow(GetMySmartPtr(), nAddDamage);
		char buffer[65];
		_itoa_s(nAddDamage, buffer, 65, 10 );
		hTargetActor->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer, false, false);

		OutputDebug("%s Damage ratio %f, bonus damage %d\n", __FUNCTION__, fDamageRate, nAddDamage);
	}
}
#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFinishAttackBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fDestHpRate[2] = {0.0f, };
	float fAddDamageRate[2] = {0.0f, };


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) {
		fDestHpRate[0] = (float)atof( vlTokens[0][0].c_str() );
		fAddDamageRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) {
		fDestHpRate[1] = (float)atof( vlTokens[1][0].c_str() );
		fAddDamageRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultDestHpRate = fDestHpRate[0] + fDestHpRate[1];
	float fResultAddDamageRate = fAddDamageRate[0] + fAddDamageRate[1];

	sprintf_s(szBuff, "%f;%f", fResultDestHpRate, fResultAddDamageRate);

	szNewValue = szBuff;
}

void CDnFinishAttackBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	float fDestHpRate[2] = {0.0f, };
	float fAddDamageRate[2] = {0.0f, };


	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 2 ) {
		fDestHpRate[0] = (float)atof( vlTokens[0][0].c_str() );
		fAddDamageRate[0] = (float)atof( vlTokens[0][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 2 ) {
		fDestHpRate[1] = (float)atof( vlTokens[1][0].c_str() );
		fAddDamageRate[1] = (float)atof( vlTokens[1][1].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultDestHpRate = fDestHpRate[0] - fDestHpRate[1];
	float fResultAddDamageRate = fAddDamageRate[0] - fAddDamageRate[1];

	sprintf_s(szBuff, "%f;%f", fResultDestHpRate, fResultAddDamageRate);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
