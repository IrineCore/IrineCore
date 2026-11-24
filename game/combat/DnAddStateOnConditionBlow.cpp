#include "stdafx.h"
#include "DnAddStateOnConditionBlow.h"
#if defined( _GAMESERVER )
#include "DnPlayerActor.h"
#endif // #if defined( _GAMESERVER )

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

CDnAddStateOnConditionBlow::CDnAddStateOnConditionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
#if !defined(_GAMESERVER)
,m_IntervalChecker( hActor, GetMySmartPtr() )
#endif // _GAMESERVER
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_172;
	SetValue( szValue );
	
	SetInfo(szValue);

#ifdef _GAMESERVER
	AddCallBackType( SB_ONTARGETHIT );

	m_bCalcProb = false;
	m_bOnDamaged = false;
#else
	//Do not display effect by default..
	UseTableDefinedGraphicEffect( false );
	m_bGraphicEffectShow = false;
	m_bTrigger = false;
#endif
}

CDnAddStateOnConditionBlow::~CDnAddStateOnConditionBlow(void)
{

}

void CDnAddStateOnConditionBlow::OnBegin(LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}

void CDnAddStateOnConditionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#if defined(_GAMESERVER)
	m_bCalcProb = m_bOnDamaged = false;
#else
	if (m_bTrigger)
	{
		if (m_bGraphicEffectShow)
		{
			_AttachGraphicEffect();
			if( m_hEtcObjectEffect )
			{
				CEtActionBase::ActionElementStruct* pStruct = m_hEtcObjectEffect->GetElement( "Idle" );
				if( pStruct )
					m_IntervalChecker.OnBegin( LocalTime, pStruct->dwLength + 1000);
			}
		}
		else
		{
			if (m_hEtcObjectEffect)
			{
				_DetachGraphicEffect();
				m_IntervalChecker.OnEnd(LocalTime, fDelta);
			}
		}

		m_bTrigger = false;
	}

	m_IntervalChecker.Process( LocalTime, fDelta );
#endif // _GAMESERVER
}


void CDnAddStateOnConditionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

#if !defined(_GAMESERVER)
	_DetachGraphicEffect();
#endif // _GAMESERVER
}

void CDnAddStateOnConditionBlow::SetInfo(const char* szValue)
{
	std::string str = szValue;//"[Probability;HitStateIndex;HitAttackAttributeIndex;DestStateIndex;StateEffectDuration][StateEffectSettingValue]";
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
		if (5 == infoTokens.size())
		{
			m_fRate = m_fValue = (float)atof(infoTokens[0].c_str());
			m_nHitStateBlowIndex = atoi(infoTokens[1].c_str());
			m_nHitAttackAttribute = atoi(infoTokens[2].c_str());
			m_nDestStateBlowIndex = atoi(infoTokens[3].c_str());
			m_nStateDurationTime = atoi(infoTokens[4].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szValue);

		//3. State effect config value
		m_strStateAttribute = tokens[1];
	}
}


#ifdef _GAMESERVER
bool CDnAddStateOnConditionBlow::CalcProb()
{
	bool bExecuteable = false;

	//Need to apply skill cooldown.
	//Only when it's a passive skill?
	DnSkillHandle hSkill;
	const CDnSkill::SkillInfo* pSkillInfo = GetParentSkillInfo();
	if (pSkillInfo)
	{
		hSkill = m_hActor->FindSkill(pSkillInfo->iSkillID);

		if (!hSkill)
			return bExecuteable;

		if (CDnSkill::UsingResult::Success != hSkill->CanExecute())
		{
			OutputDebug("%s CanExecute failed\n", __FUNCTION__);
			return bExecuteable;
		}
	}
	else
		return bExecuteable;

	//Probability calculation..
	bExecuteable = rand() % 10000 <= (m_fRate * 10000.0f);
	if (!bExecuteable)
	{
		OutputDebug("%s filtered by probability\n", __FUNCTION__);
		return bExecuteable;
	}

	// Request cooldown application [2011/03/15 semozz]
	//Send packet to client to apply passive skill cooldown..
	bool isPassiveSkill = CDnSkill::Passive == hSkill->GetSkillType();
	if (isPassiveSkill)
	{
		BYTE pBuffer[128];
		CPacketCompressStream Stream( pBuffer, 128 );

		DWORD dwUniqueID = m_hActor ? m_hActor->GetUniqueID() : -1;
		int nSkillID = hSkill->GetClassID();

		Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
		Stream.Write( &nSkillID, sizeof(nSkillID));

		m_hActor->Send(eActor::SC_PASSIVESKILL_COOLTIME, &Stream);
	}

	if (hSkill)
		hSkill->OnBeginCoolTime();

	return bExecuteable;
}

void CDnAddStateOnConditionBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	if (!hTargetActor)
		return;

	// 1. Check if a state effect corresponding to HitStateIndex exists.
	DNVector(DnBlowHandle) vlAppliedBlows;
	hTargetActor->GatherAppliedStateBlowByBlowIndex((STATE_BLOW::emBLOW_INDEX)m_nHitStateBlowIndex, vlAppliedBlows);
	if (vlAppliedBlows.empty())
		return;

	//////////////////////////////////////////////////////////////////////////
	//Related to #52905
	//If the additional state effect duration is set to -1, set it to the remaining duration of the applied state effect.
	int nListCount = (int)vlAppliedBlows.size();
	float fDurationTime = 0.0f;
	for (int i = 0; i < nListCount; ++i)
	{
		DnBlowHandle hBlow = vlAppliedBlows[i];
		if (hBlow && hBlow->GetDurationTime() > fDurationTime)
			fDurationTime = hBlow->GetDurationTime();
	}
	//////////////////////////////////////////////////////////////////////////

	// 2. Check attack element
	CDnDamageBase::SHitParam* pHitParam = hTargetActor->GetHitParam();
	if (NULL == pHitParam || pHitParam->HasElement != m_nHitAttackAttribute)
		return;
	
	// 3. Apply if probability roll succeeds
	//If probability has not been calculated, check whether to use the skill via probability calculation.
	if (false == m_bCalcProb)
	{
		m_bOnDamaged = CalcProb();
		m_bCalcProb = true;

		if (m_bOnDamaged)
		{
			//Send packet to display effect..
			BYTE pBuffer[128];
			CPacketCompressStream Stream( pBuffer, 128 );

			DWORD dwUniqueID = m_hActor ? m_hActor->GetUniqueID() : -1;
			bool bShowEffect = true;
			Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
			Stream.Write( &m_StateBlow.emBlowIndex, sizeof(m_StateBlow.emBlowIndex));
			Stream.Write( &bShowEffect, sizeof(bShowEffect));

			m_hActor->Send(eActor::SC_SHOW_STATE_EFFECT, &Stream);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Related to #52905
	// If m_nStateDurationTime is set to -1, apply the fDurationTime saved above.
	// fDurationTime is 1.0 -> 1 sec. ===> Must be changed to milliseconds (1/1000 sec).
	int nApplyDurationTime = m_nStateDurationTime;
	if (m_nStateDurationTime == -1)
		nApplyDurationTime = (int)(fDurationTime * 1000.0f);
	//////////////////////////////////////////////////////////////////////////
	
	if (m_bOnDamaged)
		hTargetActor->CmdAddStateEffect(&m_ParentSkillInfo, (STATE_BLOW::emBLOW_INDEX)m_nDestStateBlowIndex, nApplyDurationTime, m_strStateAttribute.c_str());
}
#else
bool CDnAddStateOnConditionBlow::OnCustomIntervalProcess( void )
{
	if (m_hEtcObjectEffect)
	{
		_DetachGraphicEffect();
		m_IntervalChecker.OnEnd(0, 0.0f);
	}

	return true;
}

void CDnAddStateOnConditionBlow::ShowGraphicEffect(bool bShow)
{
	m_bGraphicEffectShow = bShow;
	m_bTrigger = true;
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnAddStateOnConditionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	std::string strValue[2];
	std::vector<std::string> tokens[2];
	std::string delimiters = "[]";


	float fRate[2] = {0.0f, };
	int nHitStateBlowIndex[2] = {0, };
	int nHitAttackAttribute[2] = {0, };
	int nDestStateBlowIndex[2] = {0, };
	int nStateDurationTime[2] = {0, };
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
			nHitStateBlowIndex[0] = atoi(infoTokens[1].c_str());
			nHitAttackAttribute[0] = atoi(infoTokens[2].c_str());
			nDestStateBlowIndex[0] = atoi(infoTokens[3].c_str());
			nStateDurationTime[0] = atoi(infoTokens[4].c_str());
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
			nHitStateBlowIndex[1] = atoi(infoTokens[1].c_str());
			nHitAttackAttribute[1] = atoi(infoTokens[2].c_str());
			nDestStateBlowIndex[1] = atoi(infoTokens[3].c_str());
			nStateDurationTime[1] = atoi(infoTokens[4].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);

		//3. State effect config value
		szArgment[1] = tokens[1][1];
	}


	float fResultRate = fRate[0] + fRate[1];
	int nResultHitStateBlowIndex = nHitStateBlowIndex[0];
	int nResultHitAttackAttribute = nHitAttackAttribute[0];
	int nResultDestStateBlowIndex = nDestStateBlowIndex[0];
	int nResultStateDurationTime = max(nStateDurationTime[0], nStateDurationTime[1]);

	sprintf_s(szBuff, "[%f;%d;%d;%d;%d][%s]", fResultRate, nResultHitStateBlowIndex, nResultHitAttackAttribute, nResultDestStateBlowIndex, nResultStateDurationTime, szArgment[0].c_str());

	szNewValue = szBuff;
}

void CDnAddStateOnConditionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	std::string strValue[2];
	std::vector<std::string> tokens[2];
	std::string delimiters = "[]";


	float fRate[2] = {0.0f, };
	int nHitStateBlowIndex[2] = {0, };
	int nHitAttackAttribute[2] = {0, };
	int nDestStateBlowIndex[2] = {0, };
	int nStateDurationTime[2] = {0, };
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
			nHitStateBlowIndex[0] = atoi(infoTokens[1].c_str());
			nHitAttackAttribute[0] = atoi(infoTokens[2].c_str());
			nDestStateBlowIndex[0] = atoi(infoTokens[3].c_str());
			nStateDurationTime[0] = atoi(infoTokens[4].c_str());
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
			nHitStateBlowIndex[1] = atoi(infoTokens[1].c_str());
			nHitAttackAttribute[1] = atoi(infoTokens[2].c_str());
			nDestStateBlowIndex[1] = atoi(infoTokens[3].c_str());
			nStateDurationTime[1] = atoi(infoTokens[4].c_str());
		}
		else
			OutputDebug("%s :: %s --> State effect info check needed!!!\n", __FUNCTION__, szAddValue);

		//3. State effect config value
		szArgment[1] = tokens[1][1];
	}


	float fResultRate = fRate[0] + fRate[1];
	int nResultHitStateBlowIndex = nHitStateBlowIndex[0];
	int nResultHitAttackAttribute = nHitAttackAttribute[0];
	int nResultDestStateBlowIndex = nDestStateBlowIndex[0];
	int nResultStateDurationTime = max(nStateDurationTime[0], nStateDurationTime[1]);

	sprintf_s(szBuff, "[%f;%d;%d;%d;%d][%s]", fResultRate, nResultHitStateBlowIndex, nResultHitAttackAttribute, nResultDestStateBlowIndex, nResultStateDurationTime, szArgment[0].c_str());

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
