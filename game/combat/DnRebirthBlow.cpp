
#include "stdafx.h"
#include "DnRebirthBlow.h"
#include "DnWorld.h"

#ifndef _GAMESERVER
#include <mmsystem.h>

#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "DnInCodeResource.h"
#endif //_GAMESERVER

#include "DnHPIncBlow.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnRebirthBlow::CDnRebirthBlow(DnActorHandle hActor, const char *szValue)
	: CDnBlow(hActor)
	, m_bImmortal(false)
	, m_nCount(5)
	, m_fElapseTime(0.0f)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_057;
	AddCallBackType(SB_ONDEFENSEATTACK);
	
	// Parsing
	std::string					strValue(szValue);
	DNVector(std::string)		vSplit;
	vSplit.reserve(4);
	boost::algorithm::split( vSplit, strValue, boost::algorithm::is_any_of("/") );

	m_uiRespawnHPPercent		= (vSplit.size() >= 2) ? atoi( vSplit[1].c_str() ) : 100;
	m_uiRespawnMPPercent		= (vSplit.size() >= 3) ? atoi( vSplit[2].c_str() ) : 100;
	m_uiRespawnImmortalTimeSec	= (vSplit.size() >= 4) ? atoi( vSplit[3].c_str() ) : m_nCount;

	if( vSplit.empty() )
		SetValue( szValue );
	else
		SetValue( vSplit[0].c_str() );

	if (strcmp(GetValue(), "StageClear") == 0)
	{
		SetRebirthState();
		m_Type = eStageClear;
	}
	else
	{
		m_Type = eNormal;
	}
}

CDnRebirthBlow::~CDnRebirthBlow(void)
{
}

#if !defined _GAMESERVER
void CDnRebirthBlow::DoRebirthAction()
{
	DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
	if( hHandle ) {
		hHandle->SetPosition( *m_hActor->GetPosition() );
		hHandle->SetActionQueue( "Rebirth" );
	}
}
#endif

void CDnRebirthBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_StateBlow.fDurationTime -= fDelta;

	if (m_Type == eStageClear)
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}

	if( m_StateBlow.fDurationTime <= 0.f )
	{
		if( !m_bImmortal )
		{
			SetRebirthValue();

#if !defined _GAMESERVER
			DoRebirthAction();
#else
#ifndef _GAMESERVER
			DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
			if( hHandle ) {
				hHandle->SetPosition( *m_hActor->GetPosition() );
				hHandle->SetActionQueue( "Rebirth" );
			}
#endif
#endif // !defined _GAMESERVER
		}
		else
		{
			SetState( STATE_BLOW::STATE_END );
		}
	}

#ifndef _GAMESERVER
	CDnLocalPlayerActor *pActor = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)m_hActor);
	if( pActor && pActor->IsLocalActor() )
	{
		m_fElapseTime += fDelta;

		if( m_fElapseTime >= 1.0f )
		{
			wchar_t wszTemp[80]={0};

			if( m_bImmortal )
			{
				if( m_nCount > 0 )
				{
					float fHideTime = (m_nCount == 1) ? 1.0f : 3.0f;
					swprintf_s( wszTemp, 80, L"%d%s", m_nCount--, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120413 ) );
					GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2, wszTemp, EtInterface::textcolor::TOMATO, fHideTime );
				}
			}
			else
			{
				if( m_nCount > 0 )
				{
					float fHideTime = (m_nCount == 1) ? 1.0f : 3.0f;
					swprintf_s( wszTemp, 80, L"%d%s", m_nCount--, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120414 ) );
					GetInterface().ShowCaptionDialog( CDnInterface::typeCaption2, wszTemp, EtInterface::textcolor::GOLDENROD, fHideTime );
				}
			}

			m_fElapseTime = m_fElapseTime - 1.0f;
		}
	}
#endif
}

void CDnRebirthBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnRebirthBlow::OnBegin(%d)\n", timeGetTime() );

	if( strcmp( GetValue(), "Coin" ) == 0 )
	{
		SetRebirthValue();
		m_Type = eNormal;
#ifndef _GAMESERVER
		CDnLocalPlayerActor::LockInput( false );
#endif
	}
	else if (strcmp(GetValue(), "StageClear") == 0)
	{
#ifndef _GAMESERVER
		DoRebirthAction();
#endif
	}
}

void CDnRebirthBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnRebirthBlow::OnEnd(%d)\n", timeGetTime() );

#ifdef _GAMESERVER
	m_hActor->OnAddStateBlowProcessAfterType( CDnActor::eStateBlowAfterProcessType::eRebirth );
#else
	CDnLocalPlayerActor *pActor = dynamic_cast<CDnLocalPlayerActor*>((CDnActor*)m_hActor);
	if( pActor && pActor->IsLocalActor() )
	{
		CDnWorldEnvironment* pWorldEnv = CDnWorld::GetInstance().GetEnvironment();
		if( pWorldEnv )
		{
			pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Saturation, CDnLocalPlayerActor::GetSaturationColor(), 3000 );
			if (pActor->GetCurrentGhostType() == Ghost::Type::PartyRestore)
			{
				pWorldEnv->ControlColorFilter( CDnWorldEnvironment::Highlight, pActor->GetLastHighLightColor(), 2000 );
				pActor->SetCurrentGhostType(Ghost::Type::Normal);
			}
		}
	}
#endif
}

bool CDnRebirthBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	HitParam.szActionName.clear();
	return true;
}

void CDnRebirthBlow::SetRebirthValue()
{
//	// Pass through 100% HP recovery ratio state effect processing, in case a passive skill has an HP-increasing state effect 
//	// Max HP is not yet known at this point because it has not been applied yet.
//#ifndef _GAMESERVER
//	int iBlowID = m_hActor->AddStateBlow( NULL, STATE_BLOW::BLOW_016, -1, "1.0", true );
//#else
//	int iBlowID = m_hActor->AddStateBlow( STATE_BLOW::BLOW_016, NULL, -1, "1.0", true );
//#endif
//	
//	// Clamp HP to 0 since it can become negative.
//	if( m_hActor->GetHP() < 0 )
//		m_hActor->SetHP( 0 );
//
//	DnBlowHandle hBlow = m_hActor->GetStateBlowFromID( iBlowID );
//	_ASSERT( hBlow );
//	if( hBlow )
//	{
//		CDnHPIncBlow* pBlow = static_cast<CDnHPIncBlow*>( hBlow.GetPointer() );
//		pBlow->FromRebirth( true );
//	}

	//m_hActor->RefreshState();
	SetRebirthState();

	m_StateBlow.fDurationTime	= static_cast<float>(m_uiRespawnImmortalTimeSec);
	m_nCount					= m_uiRespawnImmortalTimeSec;
	m_bImmortal					= true;
	m_fElapseTime				= 1.f;
}

void CDnRebirthBlow::SetRebirthState()
{
	if( !m_hActor )
		return;

	m_hActor->SetHP( m_hActor->GetMaxHP()*m_uiRespawnHPPercent/100 );
	m_hActor->SetSP( m_hActor->GetMaxSP()*m_uiRespawnMPPercent/100 );
	CDnPlayerActor *pPlayer = static_cast<CDnPlayerActor *>(m_hActor.GetPointer());
	if( pPlayer ) {
		pPlayer->ToggleGhostMode( false );

#ifndef _GAMESERVER
#ifdef PRE_ADD_CASH_COSTUME_AURA
		if(pPlayer->IsEnableCostumeAura())
			pPlayer->LoadCostumeAuraEffect(true);
#endif
#endif
	}
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnRebirthBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

// 	std::string firstMsg[2];
// 	UINT RespawnHPPercent[2] = {0, };
// 	UINT RespawnMPPercent[2] = {0, };	
// 	UINT RespawnImmortalTimeSec[2] = {0, };
// 
// 	std::string					strValue(szOrigValue);
// 	DNVector(std::string)		vSplit;
// 	vSplit.reserve(4);
// 	boost::algorithm::split( vSplit, strValue, boost::algorithm::is_any_of("/") );
// 
// 	RespawnHPPercent[0]		= (vSplit.size() >= 2) ? atoi( vSplit[1].c_str() ) : 100;
// 	RespawnMPPercent[0]		= (vSplit.size() >= 3) ? atoi( vSplit[2].c_str() ) : 100;
// 	RespawnImmortalTimeSec[0]	= (vSplit.size() >= 4) ? atoi( vSplit[3].c_str() ) : m_nCount;
// 	firstMsg[0] = vSplit[0].c_str();
// 
// 	strValue = szAddValue;
// 	boost::algorithm::split( vSplit, strValue, boost::algorithm::is_any_of("/") );
// 
// 	RespawnHPPercent[1]		= (vSplit.size() >= 2) ? atoi( vSplit[1].c_str() ) : 100;
// 	RespawnMPPercent[1]		= (vSplit.size() >= 3) ? atoi( vSplit[2].c_str() ) : 100;
// 	RespawnImmortalTimeSec[1]	= (vSplit.size() >= 4) ? atoi( vSplit[3].c_str() ) : m_nCount;
// 	firstMsg[1] = vSplit[0].c_str();

	
	szNewValue = szOrigValue;
}

void CDnRebirthBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW