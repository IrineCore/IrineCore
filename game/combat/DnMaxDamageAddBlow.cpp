#include "stdafx.h"
#include "DnMaxDamageAddBlow.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )


CDnMaxDamageAddBlow::CDnMaxDamageAddBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
#if !defined(_GAMESERVER)
,m_IntervalChecker( hActor, GetMySmartPtr() )
#endif // _GAMESERVER
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_160;
	SetValue( szValue );

	m_fValue = (float)atof(szValue);

	m_fRate = m_fValue;

#ifdef _GAMESERVER
	m_bCalcProb = false;
	m_bOnDamaged = false;

	AddCallBackType( SB_ONTARGETHIT );
#else
	//Do not display effect by default..
	UseTableDefinedGraphicEffect( false );
	m_bGraphicEffectShow = false;
	m_bTrigger = false;
#endif
}

CDnMaxDamageAddBlow::~CDnMaxDamageAddBlow(void)
{

}

void CDnMaxDamageAddBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);
}

void CDnMaxDamageAddBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
	
#if defined(_GAMESERVER)
	//Reset flags
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


void CDnMaxDamageAddBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

#if !defined(_GAMESERVER)
	_DetachGraphicEffect();
#endif // _GAMESERVER
}

#if defined(_GAMESERVER)

bool CDnMaxDamageAddBlow::CalcProb()
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

void CDnMaxDamageAddBlow::OnTargetHit( DnActorHandle hTargetActor )
{
	//#30953 Prevent 1-hit 2-kill case. - Do not process damage if the actor is already dead..
	if (!m_hActor || m_hActor->IsDie())
		return;

	if (!hTargetActor || hTargetActor->IsDie())
		return;

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

	//If usable, add TargetActor to the list.
	if (m_bOnDamaged)
	{
		// [2011/03/09 semozz]
		// Need to subtract the increase from skills from max physical attack and get the max physical attack from character and items...
		int nAddDamage = m_hActor->GetAttackPMaxWithoutSkill();

		char buffer[65];
		_itoa_s(nAddDamage, buffer, 65, 10 );
		hTargetActor->CmdAddStateEffect(&m_ParentSkillInfo, STATE_BLOW::BLOW_177, 0, buffer, false, false);
	}

}
#else

bool CDnMaxDamageAddBlow::OnCustomIntervalProcess( void )
{
	if (m_hEtcObjectEffect)
	{
		_DetachGraphicEffect();
		m_IntervalChecker.OnEnd(0, 0.0f);
	}

	return true;
}

void CDnMaxDamageAddBlow::ShowGraphicEffect(bool bShow)
{
	m_bGraphicEffectShow = bShow;
	m_bTrigger = true;
}

#endif // _GAMESERVER

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnMaxDamageAddBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnMaxDamageAddBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
