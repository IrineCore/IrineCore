#include "stdafx.h"
#include "DnEffectBlow.h"
#include "DnStateBlow.h"
#ifndef _GAMESERVER
#include "DnInCodeResource.h"
#include "DnEtcObject.h"
#include "EtActionSignal.h"
#endif

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnEffectBlow::CDnEffectBlow( DnActorHandle hActor, const char* szValue )
	: CDnBlow( hActor )
	, m_nEffectType( 0 )
#ifndef _GAMESERVER
	, m_pActionElement( NULL )
	, m_fFrame( 0.0f )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_100;
	SetValue( szValue );

	std::vector<std::string> vlTokens;
	std::string strValue = szValue ? szValue : "";
	TokenizeA( strValue, vlTokens, ";" );

	if( vlTokens.size() == 2 )
	{
		m_nEffectType = atoi( vlTokens[0].c_str() );
		m_strEffectName = vlTokens[1];
	}
	else
	{
		m_nEffectType = 0;
		m_fValue = szValue ? (float)atof( szValue ) : 0.0f;
	}
}

CDnEffectBlow::~CDnEffectBlow( void )
{
#ifndef _GAMESERVER
	m_vlpSignalList.clear();
#endif
}

void CDnEffectBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	if( !m_hActor )
		return;

	if( m_nEffectType == 0 )
	{
		m_pEffectOutputInfo = m_hActor->GetStateBlow()->GetEffectOutputInfo( (int)m_fValue );
		if( m_pEffectOutputInfo )
			_AttachGraphicEffect( true );
	}
	else if( m_nEffectType == 1 )
	{
		DnEtcHandle hHandle = CDnInCodeResource::GetInstance().CreatePlayerCommonEffect();
		if( hHandle )
		{
			hHandle->SetActionQueue( m_strEffectName.c_str() );
			hHandle->SetParentActor( m_hActor );
			EtcObjectSignalStruct* pResult = m_hActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::InsertSignalHandle( -1, -1, hHandle );
			if( pResult )
			{
				pResult->bLinkObject = true;
				pResult->bDefendenceParent = true;
				sprintf_s( pResult->szBoneName, "Bip01" );
			}
		}
	}
	else if( m_nEffectType == 2 )
	{
		if( m_ParentSkillInfo.hSkillUser )
		{
			m_pActionElement = m_ParentSkillInfo.hSkillUser->GetElement( m_strEffectName.c_str() );
			if( m_pActionElement )
			{
				m_vlpSignalList = m_pActionElement->pVecSignalList;
				if( GetBlowState() == STATE_BLOW::STATE_BEGIN )
				{
					m_hActor->AddStateBlowSignal( GetMySmartPtr() );
					m_matEx = *m_hActor->GetMatEx();
				}
			}
		}
	}
#endif
}

void CDnEffectBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	if( m_nEffectType == 0 )
	{
		_DetachGraphicEffect();
	}
	else if( m_nEffectType == 2 )
	{
		if( m_hActor )
			m_hActor->RemoveStateBlowSignal( GetMySmartPtr() );
	}
#endif
}

#ifndef _GAMESERVER
void CDnEffectBlow::SignalProcess( LOCAL_TIME LocalTime, float fDelta )
{
	if( m_nEffectType != 2 )
		return;

	if( !m_ParentSkillInfo.hSkillUser || !m_hActor || !m_pActionElement )
		return;

	float fFPS = 60.0f;
	float fPrevFrame = m_fFrame;

	MatrixEx matBackup = *m_ParentSkillInfo.hSkillUser->GetMatEx();
	*m_ParentSkillInfo.hSkillUser->GetMatEx() = m_matEx;
	m_ParentSkillInfo.hSkillUser->SetChargerAction( m_strEffectName.c_str() );

	for( DWORD i = 0; i < m_vlpSignalList.size(); ++i )
	{
		CEtActionSignal* pSignal = m_vlpSignalList[i];
		if( pSignal && pSignal->CheckSignal( fPrevFrame, m_fFrame ) )
		{
			LOCAL_TIME StartTime = LocalTime - (LOCAL_TIME)( 1000.f / fFPS * ( m_fFrame - pSignal->GetStartFrame() ) );
			LOCAL_TIME EndTime = LocalTime + (LOCAL_TIME)( 1000.f / fFPS * ( pSignal->GetEndFrame() - m_fFrame ) );
			m_ParentSkillInfo.hSkillUser->OnSignal(
				(SignalTypeEnum)pSignal->GetSignalIndex(),
				pSignal->GetData(),
				LocalTime,
				StartTime,
				EndTime,
				pSignal->GetSignalListArrayIndex() );
		}
	}

	m_ParentSkillInfo.hSkillUser->SetChargerAction( "" );
	*m_ParentSkillInfo.hSkillUser->GetMatEx() = matBackup;
	m_fFrame += (fDelta * fFPS);
}
#endif
