#include "stdafx.h"
#include "DnChargerBlow.h"
#include "DnActionBase.h"
#include "EtActionSignal.h"
#include "DnProjectile.h"
#include "DnActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnChargerBlow::CDnChargerBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
//#ifdef _GAMESERVER
																			  ,m_pActionElement( NULL ),
																			  m_fFrame( 0.0f ),
																			  m_iAdditionalWeaponLength( 0 )
//#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_062;

	// Argument is the action name to attack with as caster's charger
	_ASSERT( strlen(szValue) < STATE_BLOW_ARGUMENT_MAX_SIZE );
	SetValue( szValue );

//#ifdef _GAMESERVER
	// Must be modified if multiple arguments are needed in the future.
	m_strChargerActionName = szValue;
//#endif

#if defined(PRE_FIX_65287)
	m_fShooterFinalDamageRate = 0.0f;
#endif // PRE_FIX_65287
}

CDnChargerBlow::~CDnChargerBlow(void)
{

}

void CDnChargerBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	// Receive action.
//#ifdef _GAMESERVER
	if( !m_ParentSkillInfo.hSkillUser )
	{		
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnChargerBlow::OnBegin fail!! SkillUser is Invalid!!\n" );
		return;
	}
		
	m_pActionElement = NULL;
	m_pActionElement = m_ParentSkillInfo.hSkillUser->GetElement( m_strChargerActionName.c_str() );
	OutputDebug("%s %s", __FUNCTION__, m_strChargerActionName.c_str());

	_ASSERT( m_pActionElement && "Charger action info not found." );

	// Get additional range when using a skill.
	DnSkillHandle hParentSkill = m_ParentSkillInfo.hSkillUser->GetProcessSkill();
	if( hParentSkill )
		m_iAdditionalWeaponLength = hParentSkill->GetIncreaseRange();

	if( m_pActionElement )
	{
		m_vlpSignalList = m_pActionElement->pVecSignalList;

#ifdef _GAMESERVER
		if( m_ParentSkillInfo.hSkillUser )
		{
			m_pShooterStateSnapshot.reset( new CDnState );
			*m_pShooterStateSnapshot= *static_cast<CDnState*>( m_ParentSkillInfo.hSkillUser.GetPointer() );

#if defined(PRE_FIX_65287)

			m_fShooterFinalDamageRate = 0.0f;
			if (m_ParentSkillInfo.hSkillUser && m_ParentSkillInfo.hSkillUser->IsAppliedThisStateBlow(STATE_BLOW::BLOW_050))
			{
				DNVector(DnBlowHandle) vlhBlows;
				m_ParentSkillInfo.hSkillUser->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_050, vlhBlows );
				int iNumBlow = (int)vlhBlows.size();
				for( int i = 0; i < iNumBlow; ++i )
				{
					m_fShooterFinalDamageRate += vlhBlows[i]->GetFloatValue();
				}
			}
#endif // PRE_FIX_65287
		}
#endif

		// Add only in normal state since it can terminate immediately
		if (STATE_BLOW::STATE_BEGIN == GetBlowState())
			m_hActor->AddStateBlowSignal(this->GetMySmartPtr());

		OutputDebug( "CDnChargerBlow::OnBegin\n" );
	}
	else
	{
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnChargerBlow::OnBegin fail!! Can't find Charger action element!!\n" );
	}
//#else
//	OutputDebug( "CDnChargerBlow::OnBegin\n" );
//#endif
}


void CDnChargerBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
 
}


void CDnChargerBlow::OnProjectileSignal( CDnProjectile* pProjectile )
{
	// Add skill range.
	pProjectile->SetWeaponLength( pProjectile->GetWeaponLength() + m_iAdditionalWeaponLength );

	// Mark on the projectile that it was fired due to a charge state effect.
	// Exclude from the player's combo count even if this hits. (#11329)
#ifdef _GAMESERVER
	pProjectile->FromCharger();
	pProjectile->FromSkill( true );
	pProjectile->SetShooterStateSnapshot( m_pShooterStateSnapshot );
	pProjectile->SetForceHitElement( m_ParentSkillInfo.eSkillElement );

#if defined(PRE_FIX_65287)
	pProjectile->SetShooterFinalDamageRate(m_fShooterFinalDamageRate);
#endif // PRE_FIX_65287
#endif
}


void CDnChargerBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_hActor->RemoveStateBlowSignal(this->GetMySmartPtr());

	OutputDebug( "CDnChargerBlow::OnEnd\n" );
}

void CDnChargerBlow::SignalProcess( LOCAL_TIME LocalTime, float fDelta )
{
//#ifdef _GAMESERVER
	if( m_ParentSkillInfo.hSkillUser &&
		m_hActor )
	{
		// Process signal after changing only the position to the location of the actor on which this state effect is active
		if( m_pActionElement )
		{
			//float fFPS = m_ParentSkillInfo.hSkillUser->CDnActionBase::GetFPS();
			float fFPS = 60.0f;			// Ensure charger action signals are processed at the default frame without changing frames. #26439

			// [2011/03/07 semozz]
			// When fDelta is smaller than 1/60 (0.01666)
			// m_fFrame value becomes smaller than 1..
			// Caught in the first if statement in CheckSignal, and because fCur is not 0.0
			// Signals on the first frame might not be processed.
			// So defer the frame increment to the end, and set fPrevFrame to a value slightly subtracted from m_fFrame.
			// When cur is within Frame [S, E], if (int)cur == (int)pre and cur != 0.0, fail..
			// Adjust by 1.0 difference so cur/pre int cast comparisons are not equal..
			float fPrevFrame = m_fFrame - 1.0f;

			//float fPrevFrame = m_fFrame;
			//m_fFrame += (fDelta * fFPS);

			if( m_pActionElement->dwLength < (DWORD)m_fFrame )
			{
				m_fFrame = 1.0f;
				fPrevFrame = -0.1f;
			}

			//if( 0.0f == fPrevFrame )
			//	fPrevFrame = -(m_fFrame * 0.1f);

			// Temporarily change only position to current actor, run signal, then restore.
			MatrixEx OriginalCross = *(m_ParentSkillInfo.hSkillUser->GetMatEx());
			MatrixEx ChargerActorCross = *(m_hActor->GetMatEx());
			*(m_ParentSkillInfo.hSkillUser->GetMatEx()) = ChargerActorCross;
			m_ParentSkillInfo.hSkillUser->SetOnSignalFromChargerSE( true );
			m_ParentSkillInfo.hSkillUser->SetChargerDestActor( m_hActor );
#ifdef _GAMESERVER
			m_ParentSkillInfo.hSkillUser->SetChargerBlow( GetMySmartPtr() );
			m_ParentSkillInfo.hSkillUser->PushForceHitElement( m_ParentSkillInfo.eSkillElement );
#endif

			CEtActionSignal *pSignal = NULL;
			
#if defined(PRE_FIX_55378)
			m_ParentSkillInfo.hSkillUser->SetChargerAction(m_strChargerActionName.c_str());
#endif // PRE_FIX_55378

			for( DWORD i=0; i < m_vlpSignalList.size(); i++ )
			{
				pSignal = m_vlpSignalList[i];
				if( pSignal->CheckSignal( fPrevFrame, m_fFrame ) == true ) 
				{
					LOCAL_TIME StartTime = LocalTime - (LOCAL_TIME)( 1000.f / fFPS * ( m_fFrame - pSignal->GetStartFrame() ) );
					LOCAL_TIME EndTime = LocalTime + (LOCAL_TIME)( 1000.f / fFPS * ( pSignal->GetEndFrame() - m_fFrame ) );

					m_ParentSkillInfo.hSkillUser->OnSignal( (SignalTypeEnum)pSignal->GetSignalIndex(), pSignal->GetData(), LocalTime, 
						StartTime, EndTime, pSignal->GetSignalListArrayIndex() );
				}

				//////////////////////////////////////////////////////////////////////////
				//OutputDebug("%s, Signal start=%d, preFrame=%f, curFrame=%f, end=%d ====> %s\n", __FUNCTION__, pSignal->GetStartFrame(), fPrevFrame, m_fFrame, pSignal->GetEndFrame(), (isCheckSignal ? "true" : "false"));
				//////////////////////////////////////////////////////////////////////////
			}

#if defined(PRE_FIX_55378)
			m_ParentSkillInfo.hSkillUser->SetChargerAction(NULL);
#endif // PRE_FIX_55378

			*(m_ParentSkillInfo.hSkillUser->GetMatEx()) = OriginalCross;

			m_ParentSkillInfo.hSkillUser->SetOnSignalFromChargerSE( false );
			DnActorHandle hNullActor;
			m_ParentSkillInfo.hSkillUser->SetChargerDestActor( hNullActor );

#ifdef _GAMESERVER
			DnBlowHandle hNullBlow;
			m_ParentSkillInfo.hSkillUser->SetChargerBlow( hNullBlow );
			m_ParentSkillInfo.hSkillUser->PopForceHitElement();
#endif


			//Defer frame increment for later.
			m_fFrame += (fDelta * fFPS);
		}
	}
	//#endif
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChargerBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnChargerBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
