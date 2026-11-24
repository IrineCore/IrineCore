#include "stdafx.h"
#include "DnPushBlow.h"
#include "DnSkill.h"
#ifdef _GAMESERVER
#include "DnHighLanderBlow.h"
#endif 

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

EtVector3 CDnPushBlow::s_vTestVelocity = EtVector3(0.0f, 0.0f, -4.0f);;
EtVector3 CDnPushBlow::s_vTestResistance = EtVector3(0.0f, 0.0f, 15.0f);

CDnPushBlow::CDnPushBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_175;
	SetValue( szValue );
	
	m_fValue = 0.0f;


	SetInfo(szValue);

#ifdef _GAMESERVER
	switch(m_TriggerType)
	{
	case eTriggerType::OnDefense:
		{
			AddCallBackType( SB_ONDEFENSEATTACK );
		}
		break;
	case eTriggerType::OnHit:
		{
			AddCallBackType( SB_ONTARGETHIT );
		}
		break;
	default:
		{
			OutputDebug("%s TriggerType Failed!!!!\n", __FUNCTION__);
		}
		break;
	}
#endif

	//Numeric value for knockback..
	m_vVelocity[0] = EtVector3(0.0f, 3.0f, -12.0f);
	m_vResistance[0] = EtVector3(0.0f, -20.0f, 14.0f);
	m_ActionName[0] = "Down_KnockBack";

	//Numeric value for knockback..
	m_vVelocity[1] = EtVector3(0.0f, 0.0f, -4.0f);
	m_vResistance[1] = EtVector3(0.0f, 0.0f, 5.0f);
	m_ActionName[1] = "Hit_Front";
}

CDnPushBlow::~CDnPushBlow(void)
{

}

void CDnPushBlow::SetInfo(const char* szValue)
{
	std::string str = szValue;//"PushType;TriggerType";
	std::vector<std::string> tokens;
	std::string delimiters = ";";

	TokenizeA(str, tokens, delimiters);
	if (2 == tokens.size())
	{
		m_PushType = (ePushType)atoi(tokens[0].c_str());
		m_TriggerType = (eTriggerType)atoi(tokens[1].c_str());
	}
	else
		OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);

}

void CDnPushBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER

#endif
}

void CDnPushBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
	//OutputDebug( "CDnFireBurnBlow::Process, Value:%2.2f (HP : %d ) \n", m_fValue, m_hActor->GetHP());
}


void CDnPushBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

#ifndef _GAMESERVER

#endif
}


#if defined(_GAMESERVER)

bool CDnPushBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	if (hHitter && hHitter->IsImmuned(m_StateBlow.emBlowIndex))
		return false;

	PushActor(hHitter, HitParam);
	return false;
}

void CDnPushBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if (!hTargetActor)
		return;

	if (hTargetActor->IsImmuned(m_StateBlow.emBlowIndex))
		return;

	CDnDamageBase::SHitParam tempHitParam = *hTargetActor->GetHitParam();
	tempHitParam.vViewVec = -tempHitParam.vViewVec;
	PushActor(hTargetActor, tempHitParam);
}

void CDnPushBlow::PushActor(DnActorHandle hActor, CDnDamageBase::SHitParam& HitParam)
{
	if (!hActor)
		return;

	CDnDamageBase::SHitParam newHitParam = HitParam;
	newHitParam.fDamage = 0.0f;
	newHitParam.szActionName = m_ActionName[m_PushType];
	newHitParam.vVelocity = m_vVelocity[m_PushType];
	newHitParam.vResistance = m_vResistance[m_PushType];
	newHitParam.DistanceType = CDnDamageBase::Melee;//Melee, Range
	newHitParam.cAttackType = 0;//Physical / Magic
	newHitParam.fStiffProb = 100.0f;
	newHitParam.vViewVec = -HitParam.vViewVec;

	hActor->ForceKnockBack(m_hActor, newHitParam);
}

#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnPushBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnPushBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
