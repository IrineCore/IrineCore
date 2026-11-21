#include "stdafx.h"
#include "DnFireBurnExplosionBlow.h"
#include "DnMonsterActor.h"
#include "EtActionSignal.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

#define FIREBURN_EXPLOSION_ACTION_NAME "Hit_Burn"

CDnFireBurnExplosionBlow::CDnFireBurnExplosionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																								  m_pActionElement( NULL ),
																								  m_fFrame( 0.0f )
#ifdef _GAMESERVER
																								,m_iOriAttackMMin( 0 ),
																								 m_iOriAttackMMax( 0 )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_116;

	SetValue( szValue );
	m_fValue = (float)atof(szValue);

	m_pActionElement = m_hActor->GetElement( FIREBURN_EXPLOSION_ACTION_NAME );
	_ASSERT( m_pActionElement && "Burn explosion action info not found." );

	m_ActionDurationTime = 0;
	if (m_pActionElement)
	{
		m_ActionDurationTime = m_pActionElement->dwLength;
		m_vlpSignalList = m_pActionElement->pVecSignalList;
	}
}

CDnFireBurnExplosionBlow::~CDnFireBurnExplosionBlow(void)
{

}

bool CDnFireBurnExplosionBlow::CanBegin( void )
{
	// Cannot start unless the burn state effect is active.
	bool bCanBegin = m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_042 );

	if( false == bCanBegin )
		OutputDebug( "CDnFireBurnExplosionBlow::CanBegin - Burn state effect is not active, so Burn Explosion state effect failed to trigger.\n" );

	// Do not trigger when there is no explosion action.
	if( NULL == m_pActionElement )
	{
		bCanBegin = false;
		OutputDebug( "CDnFireBurnExplosionBlow::CanBegin - No Burn Explosion action, so Burn Explosion state effect failed to trigger.\n" );
	}

	return bCanBegin;
}

void CDnFireBurnExplosionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Remove burn state effect
#ifdef _GAMESERVER
	m_hActor->CmdRemoveStateEffect( STATE_BLOW::BLOW_042 );
#endif


	// Replaces the affected actor's magic attack power with the magic attack power of the skill caster who applied this state effect for its duration.
#ifdef _GAMESERVER
	_ASSERT( m_ParentSkillInfo.hSkillUser );
	if( !m_ParentSkillInfo.hSkillUser )
	{
		g_Log.Log( LogType::_ERROR, L"CDnFireBurnExplosionBlow::OnBegin - Burn Explosion state effect actor is invalid, failed to trigger..\n" );
		SetState( STATE_BLOW::STATE_END );
		return;
	}

	m_iOriAttackMMin = m_hActor->GetAttackMMin();
	m_iOriAttackMMax = m_hActor->GetAttackMMax();

	m_hActor->SetAttackMMin( m_ParentSkillInfo.hSkillUser->GetAttackMMin() );
	m_hActor->SetAttackMMax( m_ParentSkillInfo.hSkillUser->GetAttackMMax() );

	// If monster, set up for damage display on the client.
	if( m_hActor && CDnActorState::Reserved6 < m_hActor->GetActorType() )
		static_cast<CDnMonsterActor*>(m_hActor.GetPointer())->SlaveOf( m_ParentSkillInfo.hSkillUser->GetUniqueID() );
#endif

	// Duration is handled as the duration of the explosion action.
	float fFPS = 60.0f;
	//m_StateBlow.fDurationTime = (float)m_pActionElement->dwLength / fFPS;
	m_StateBlow.fDurationTime = m_ActionDurationTime != 0 ? (float)m_ActionDurationTime / fFPS : 0.0f;

	// Add only in normal state since it can terminate immediately
	if (STATE_BLOW::STATE_BEGIN == GetBlowState())
		m_hActor->AddStateBlowSignal(this->GetMySmartPtr());

	OutputDebug( "CDnFireBurnExplosionBlow::OnBegin\n" );
}


void CDnFireBurnExplosionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

}


void CDnFireBurnExplosionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	m_hActor->SetAttackMMin( m_iOriAttackMMin );
	m_hActor->SetAttackMMax( m_iOriAttackMMax );

	// If monster, restore the settings made for damage display on the client.
	//if( CDnActorState::Reserved6 < m_hActor->GetActorType() )
	if( m_hActor && m_hActor->IsMonsterActor() )
	{
		DWORD dwInvalidMasterActorID = UINT_MAX;
		static_cast<CDnMonsterActor*>(m_hActor.GetPointer())->SlaveOf( dwInvalidMasterActorID );
	}
#endif

	m_hActor->RemoveStateBlowSignal(this->GetMySmartPtr());

	OutputDebug( "CDnFireBurnExplosionBlow::OnEnd\n" );
}

//#ifdef _GAMESERVER
//
//#endif


void CDnFireBurnExplosionBlow::SignalProcess( LOCAL_TIME LocalTime, float fDelta )
{
	// Execute only signals without executing the actual action.
	float fFPS = 60.0f;
	float fPrevFrame = m_fFrame;
	m_fFrame += (fDelta * fFPS);

	//if(m_pActionElement->dwLength < (DWORD)m_fFrame )
	if (m_ActionDurationTime < (DWORD)m_fFrame)
	{
		m_fFrame = 1.0f;
		fPrevFrame = -0.1f;

		// Once all actions finish, stop processing signals and end the state effect. 
		// Duration is set to action length, so state effect will end soon.
		return;
	}

	if( 0.0f == fPrevFrame )
		fPrevFrame = -(m_fFrame * 0.1f);

	CEtActionSignal *pSignal = NULL;

#if defined(PRE_FIX_55378)
	m_hActor->SetChargerAction(FIREBURN_EXPLOSION_ACTION_NAME);
#endif // PRE_FIX_55378

#if defined(PRE_FIX_62309)
#if defined(_GAMESERVER)
	//#62309
	//Attributes may not be set on Hit via OnSignal while running the signal processor.
	//Set forced attributes before calling the signal processor, and use the set attribute values when processing the Hit signal
	//Force setting..
	m_hActor->PushForceHitElement(m_ParentSkillInfo.eSkillElement);
#endif // _GAMESERVER
#endif // PRE_FIX_62309

	//for( DWORD i=0; i < m_pActionElement->pVecSignalList.size(); i++ )
	for( DWORD i=0; i < m_vlpSignalList.size(); i++ )
	{
		//pSignal = m_pActionElement->pVecSignalList[i];
		pSignal = m_vlpSignalList[i];
		if( pSignal->CheckSignal( fPrevFrame, m_fFrame ) == true ) 
		{
			LOCAL_TIME StartTime = LocalTime - (LOCAL_TIME)( 1000.f / fFPS * ( m_fFrame - pSignal->GetStartFrame() ) );
			LOCAL_TIME EndTime = LocalTime + (LOCAL_TIME)( 1000.f / fFPS * ( pSignal->GetEndFrame() - m_fFrame ) );

			m_hActor->OnSignal( (SignalTypeEnum)pSignal->GetSignalIndex(), pSignal->GetData(), LocalTime, 
				StartTime, EndTime, pSignal->GetSignalListArrayIndex() );
		}
	}

#if defined(PRE_FIX_62309)
#if defined(_GAMESERVER)
	m_hActor->PopForceHitElement();
#endif // _GAMESERVER
#endif // PRE_FIX_62309

#if defined(PRE_FIX_55378)
	m_hActor->SetChargerAction(NULL);
#endif // PRE_FIX_55378
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFireBurnExplosionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnFireBurnExplosionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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