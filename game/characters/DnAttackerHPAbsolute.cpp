#include "stdafx.h"
#include "DnAttackerHPAbsolute.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnAttackerHPAbsolute::CDnAttackerHPAbsolute( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_091;
	SetValue( szValue );
	m_fValue = (float)atof( szValue );

#ifdef _GAMESERVER
	AddCallBackType( SB_ONDEFENSEATTACK );
	AddCallBackType( SB_ONCALCDAMAGE );
#endif

}

CDnAttackerHPAbsolute::~CDnAttackerHPAbsolute(void)
{

}

void CDnAttackerHPAbsolute::OnSetParentSkillInfo()
{

#if defined( _GAMESERVER )
	CalcHealValueLimit( m_StateBlow.emBlowIndex, m_fValue );
#endif

}

void CDnAttackerHPAbsolute::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAttackerHPAbsolute::OnBegin \n");
}


void CDnAttackerHPAbsolute::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnAttackerHPAbsolute::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnAttackerHPAbsolute::OnEnd \n");
}

#ifdef _GAMESERVER
bool CDnAttackerHPAbsolute::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	// Restore HP on every hit
	// Send HP recovery packet. (May be negative)
	INT64 iMaxHP = hHitter->GetMaxHP();
	INT64 iNowHP = hHitter->GetHP();
	INT64 iDelta = INT64(m_fValue);
	INT64 iResultHP = iNowHP+iDelta;

	if( iResultHP > iMaxHP )
	{
		iDelta = iMaxHP - iNowHP;
		iResultHP = iMaxHP;
	}
	else
	if( iResultHP < 0 )
	{
		iDelta = iDelta - iResultHP;
		iResultHP = 0;
	}

	if( 0 != iDelta )
	{
		hHitter->SetHP( iResultHP );
		hHitter->RequestHPMPDelta( CDnState::ElementEnum_Amount, iDelta, hHitter->GetUniqueID() );
	}

	// Since OnCalcDamage sets damage to 0, super armor naturally prevents the hit reaction action from playing
	// Just let it pass through here.
	//HitParam.szActionName.clear();
	HitParam.vResistance = EtVector3( 0.0f, 0.0f, 0.0f );
	HitParam.vVelocity = EtVector3( 0.0f, 0.0f, 0.0f );
	HitParam.vViewVec = *(m_hActor->GetLookDir());
	
	return false;
}

float CDnAttackerHPAbsolute::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	return -fOriginalDamage;
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAttackerHPAbsolute::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	float fResultValue = fValue[0] + fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}

void CDnAttackerHPAbsolute::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Required value variable
	float fValue[2];

	//////////////////////////////////////////////////////////////////////////
	//First value
	fValue[0] = (float)atof( szOrigValue );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Second
	fValue[1] = (float)atof( szAddValue );
	//////////////////////////////////////////////////////////////////////////

	//Calculate value
	float fResultValue = fValue[0] - fValue[1];

	sprintf_s(szBuff, "%f", fResultValue);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
