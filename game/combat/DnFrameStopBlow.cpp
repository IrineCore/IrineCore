#include "stdafx.h"
#include "DnFrameStopBlow.h"

#ifdef _GAMESERVER
// #include "DnGameRoom.h"
// #include "DnGameDataManager.h"
// #include "DnUserSession.h"
#include "DnStateBlow.h"
#else
#include "DnEtcObject.h"
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "navigationmesh.h"
#include "InputWrapper.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const LOCAL_TIME SHAKE_TIME = 3000;

CDnFrameStopBlow::CDnFrameStopBlow(DnActorHandle hActor, const char *szValue) : CDnBlow(hActor)
#if defined(PRE_FIX_53274)
#ifndef _GAMESERVER
,m_ComboCalc( CDnComboCalculator::CIRCULAR_CHECK )
#endif
#endif // PRE_FIX_53274
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_146;

#if defined(PRE_FIX_53274)
	SetValue(szValue);
	m_fValue = (szValue == NULL ? 0.0f : (float)atof(szValue));

	m_bPlayerCharacter = false;

#ifndef _GAMESERVER
	if (m_fValue != 0)
	{
		std::vector<BYTE> vlKeysToCheck;
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVELEFT] );
		vlKeysToCheck.push_back( g_WrappingKeyData[IW_MOVERIGHT] );

		m_ComboCalc.SetKeysToCheck( vlKeysToCheck );

		vlKeysToCheck.clear();
		vlKeysToCheck.push_back( IW_MOVELEFT );		vlKeysToCheck.push_back( IW_MOVELEFT );		vlKeysToCheck.push_back( IW_MOVELEFT );
		vlKeysToCheck.push_back( IW_MOVERIGHT );	vlKeysToCheck.push_back( IW_MOVERIGHT );	vlKeysToCheck.push_back( IW_MOVERIGHT );
		vlKeysToCheck.push_back( IW_MOVEBACK );		vlKeysToCheck.push_back( IW_MOVEBACK );		vlKeysToCheck.push_back( IW_MOVEBACK );
		vlKeysToCheck.push_back( IW_MOVEFRONT );	vlKeysToCheck.push_back( IW_MOVEFRONT );	vlKeysToCheck.push_back( IW_MOVEFRONT );

		m_ComboCalc.SetPadsToCheck( vlKeysToCheck, 3.0f );

		m_bPlayerCharacter = dynamic_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer()) ? true : false;
	}
#endif // _GAMESERVER
#endif // PRE_FIX_53274
}

CDnFrameStopBlow::~CDnFrameStopBlow(void)
{
}

void CDnFrameStopBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	//// Immediately reset play speed and stop. 
	//m_hActor->ResetPlaySpeed();
	//m_hActor->SetPlaySpeed( 1000000, 0.0f );

	m_hActor->AddedFrameStop();

	// #29627 Add hit state when not in hit state so Relieve can be used.
	if( m_hActor->IsPlayerActor() )		// Only when it is a player.
	{
		if( false == m_hActor->IsHit() )
		{
			int nState = m_hActor->GetState();
			nState |= CDnActorState::ActorStateEnum::Hit;
			m_hActor->SetState( (CDnActorState::ActorStateEnum)nState );
		}
	}
	//////////////////////////////////////////////////////////////////////////

#if defined(PRE_FIX_53274)
#ifndef _GAMESERVER
	// Show button-mash UI when the player is affected
	if( m_fValue != 0.0f && m_bPlayerCharacter )
	{
#if defined(PRE_FIX_57706)
		if (m_hActor->GetStickAniDlgRefCount() == 0)
		{
			GetInterface().ShowStickAniDialog( true );
		}

		m_hActor->AddStickAniDlgRefCount();
#else
		GetInterface().ShowStickAniDialog( true );
#endif // PRE_FIX_57706
	}
#endif // _GAMESERVER
#endif // PRE_FIX_53274

	OutputDebug( "CDnFrameStopBlow::OnBegin, Value:%f \n", m_fValue );
}

#if defined(PRE_FIX_53274)
void CDnFrameStopBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	__super::Process(LocalTime, fDelta);

#ifndef _GAMESERVER
	if (m_fValue != 0.0f)
	{
		if( CDnLocalPlayerActor::s_hLocalActor == m_hActor )
		{
			// TODO: Decrease time on each left/right input. Eventually needs to be sent to server as well...
			int iNumComboCount = 0;
			bool bComboSatisfy = m_ComboCalc.Process( LocalTime, fDelta, &iNumComboCount );

			for( int i = 0; i < iNumComboCount; ++i )
				m_StateBlow.fDurationTime -= 1.0f;

			if( bComboSatisfy )
			{
				OutputDebug( "[Emergency Escape] %d-hit combo succeeded, time reduced to %2.2f\n", iNumComboCount, m_StateBlow.fDurationTime );
			}
		}

		// Only one's own player character can reduce duration via emergency escape, so check and notify server to end the state effect.
		if( CDnLocalPlayerActor::s_hLocalActor == m_hActor )
		{
			if( m_StateBlow.fDurationTime < 0.2f )
			{
				// Pre-send to game server.
#if defined(PRE_FIX_57706)
				//State effect end condition confirmed on client.. Modified to send packet to server first, then server broadcasts so it is removed upon receiving packet..
				//This client does not actually remove it and only sends a packet; it is removed automatically when time expires or upon receiving a packet from the server.
				CDnLocalPlayerActor* pLocalPlayerActor = static_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer());
				if (pLocalPlayerActor)
					pLocalPlayerActor->CmdRemoveStateEffectByServerBlowID(GetServerBlowID());
#else
				m_hActor->CmdRemoveStateEffect( m_StateBlow.emBlowIndex, true );
#endif // PRE_FIX_57706
			}
		}
	}
#endif // _GAMESERVER
}
#endif // PRE_FIX_53274

void CDnFrameStopBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	//m_hActor->ResetPlaySpeed();

	m_hActor->RemovedFrameStop();

#if defined(PRE_FIX_53274)
#ifndef _GAMESERVER
	// Show button-mash UI when the player is affected
	if( dynamic_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer()) )
	{
#if defined(PRE_FIX_57706)
		m_hActor->RemoveStickAniDlgRefCount();

		if (m_hActor->GetStickAniDlgRefCount() == 0)
		{
			GetInterface().ShowStickAniDialog( false );
		}
#else
		GetInterface().ShowStickAniDialog( false );
#endif // PRE_FIX_57706
	}
#endif // _GAMESERVER
#endif // PRE_FIX_53274

#if defined(_GAMESERVER)
	if (m_fValue != 0.0f)
	{
		//Find state effects added by the same skill and remove them together.
		CDnStateBlow* pStateBlow = m_hActor->GetStateBlow();
		int nStateCount = pStateBlow ? pStateBlow->GetNumStateBlow() : 0;
		for( int iBlow = 0; iBlow < nStateCount; ++iBlow )
		{
			DnBlowHandle hExistingBlow = pStateBlow->GetStateBlow( iBlow );
			CDnSkill::SkillInfo* pSkillInfo = const_cast<CDnSkill::SkillInfo*>(hExistingBlow->GetParentSkillInfo());

			//When the skill and skill user are the same.
			if (pSkillInfo && 
				pSkillInfo->iSkillID == m_ParentSkillInfo.iSkillID && 
				pSkillInfo->hSkillUser &&
				pSkillInfo->hSkillUser == m_ParentSkillInfo.hSkillUser)
			{
				hExistingBlow->SetState(STATE_BLOW::STATE_END);
				m_hActor->CmdRemoveStateEffectFromID(hExistingBlow->GetBlowID());
			}
		}
	}
#endif // _GAMESERVER

	OutputDebug( "CDnFrameStopBlow::OnEnd\n" );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFrameStopBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnFrameStopBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW