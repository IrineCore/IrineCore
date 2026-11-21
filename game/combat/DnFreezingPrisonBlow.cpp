#include "stdafx.h"
#include "DnFreezingPrisonBlow.h"
#include "DnHighLanderBlow.h"
#include "DnCantMoveBlow.h"
#include "DnCantActionBlow.h"
#include "DnFrameStopBlow.h"

#include "EtActionBase.h"
#include "EtActionSignal.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif // #if !defined( USE_BOOST_MEMPOOL )

const int FREEZING_PRISON_INTERVAL_TIME = 1000;


CDnFreezingPrisonBlow::CDnFreezingPrisonBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																							m_pCantMoveBlow( new CDnCantMoveBlow(hActor, NULL ) ),
																							m_pCantActionBlow( new CDnCantActionBlow(hActor, NULL) ),
																							m_pFrameStopBlow( new CDnFrameStopBlow(hActor, NULL) ),
																							m_IntervalChecker( hActor, GetMySmartPtr() ),
																							m_fIntervalDamage( 0.0f ),
																							m_iDurability( 0 ),
																							m_bDurabilityDestroyed( false )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_149;

	SetValue( szValue );
	
	m_iFinalDamage = 0;
	m_iOriginalTeam = -1;

	// Total of 3 arguments.
	// Damage on explosion when durability/duration runs out, damage per second
	string strValue( szValue );
	
	// After splitting the string by whitespace
	std::vector<string> vlTokens;
	TokenizeA( strValue, vlTokens, ";" );

	bool bValidArgument = (3 == (int)vlTokens.size());
	_ASSERT( bValidArgument && "Invalid Ice Prison state effect argument settings." );

	if( bValidArgument )
	{
		string strDurability = vlTokens.at( 0 );
		string strFinalDamage = vlTokens.at( 1 );
		string strIntervalDamage = vlTokens.at( 2 );

		m_iDurability = atoi( strDurability.c_str() );
		m_iFinalDamage = atoi( strFinalDamage.c_str() );
		m_fIntervalDamage = (float)atoi( strIntervalDamage.c_str() );

#ifdef _GAMESERVER
		AddCallBackType( SB_ONDEFENSEATTACK );
		AddCallBackType( SB_ONCALCDAMAGE );
#endif

		m_pCantActionBlow->SetActionWhenCancelAttack( "Freezing" );
	}

	m_fOrigWight = 0.0f;
	m_iOrigPressLevel = 0;
	m_iMaxDurability = m_iDurability;

	// [2010/12/13 semozz]
	// To keep maintaining it when the state effect holds a state
	if (m_pCantMoveBlow)
		m_pCantMoveBlow->SetPermanent(true);
	if (m_pCantActionBlow)
		m_pCantActionBlow->SetPermanent(true);
	if (m_pFrameStopBlow)
		m_pFrameStopBlow->SetPermanent(true);
}

CDnFreezingPrisonBlow::~CDnFreezingPrisonBlow(void)
{
	SAFE_DELETE( m_pCantMoveBlow );
	SAFE_DELETE( m_pCantActionBlow );
	SAFE_DELETE( m_pFrameStopBlow );
}

void CDnFreezingPrisonBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_IntervalChecker.OnBegin( LocalTime, FREEZING_PRISON_INTERVAL_TIME );

	// Cannot act, cannot move
	if( m_pCantMoveBlow ) 
		m_pCantMoveBlow->OnBegin( LocalTime, fDelta );

	if( m_pCantActionBlow ) 
		m_pCantActionBlow->OnBegin( LocalTime, fDelta );

	if( m_pFrameStopBlow )
		m_pFrameStopBlow->OnBegin( LocalTime, fDelta );


#if defined(PRE_FIX_50482)
	if (m_hActor && m_hActor->GetChangeTeamRefCount() == 0)
	{
		// Changes the team to that of the actor (currently a monster) who used this skill.
		m_iOriginalTeam = m_hActor->GetTeam();
		//Set what the original team was.
		m_hActor->SetOriginalTeam(m_iOriginalTeam);
	}
	m_hActor->AddChangeTeamRefCount();

	m_hActor->SetTeam( m_ParentSkillInfo.iSkillUserTeam );
#else
	// Changes the team to that of the actor (currently a monster) who used this skill.
	m_iOriginalTeam = m_hActor->GetTeam();
	m_hActor->SetTeam( m_ParentSkillInfo.iSkillUserTeam );

	//Set what the original team was.
	m_hActor->SetOriginalTeam(m_iOriginalTeam);
#endif // PRE_FIX_50482
	
#ifndef _GAMESERVER
	_AttachGraphicEffect();

	// Initially execute the idle action.
	if( m_hEtcObjectEffect )
		m_hEtcObjectEffect->SetActionQueue( "Idle" );
#endif

	//Save Weight/PressLevel when starting.
	m_fOrigWight = m_hActor->GetWeight();
	m_iOrigPressLevel = m_hActor->GetPressLevel();

	//Configured to prevent character pushing/sliding.
	m_hActor->SetWeight(0.0f);
	m_hActor->SetPressLevel(-1);

#if defined(_GAMESERVER)
	//Display UI when StateBlow starts
	m_hActor->CmdFreezingPrisonDurablity(GetBlowID(), 100.0f, true);
#endif

	OutputDebug( "CDnFreezingPrisonBlow::OnBegin\n" );
}

#ifdef _GAMESERVER
void CDnFreezingPrisonBlow::_RequestDamage( float fDamage )
{
#if defined(PRE_FIX_44884)
#if defined(PRE_FIX_61382)
	DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
	if (hActor &&
		hActor->IsDie() == false)
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)fDamage);

		//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
		//No need to call Die here.
		//if (m_hActor->IsDie())
		//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
	}
#else
	if (m_hActor->IsDie() == false)
	{
		m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)fDamage);

		//#48491 Die is being called from ActorStatIntervlaManipulator when applying damage via timer.
		//No need to call Die here.
		//if (m_hActor->IsDie())
		//	m_hActor->Die(m_ParentSkillInfo.hSkillUser);
	}
#endif // PRE_FIX_61382
#else
	// Must not die while the Highlander state effect is active.
	float fResultDamage = fDamage;
	if( m_hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_143 ) )
	{
		// IsAppliedThisStateBlow() returns true, but duration has expired
		// State effects are not retrieved from GatherAppliedStateBlowByBlowIndex(), so the list may be empty.
		DNVector( DnBlowHandle ) vlhHighLanderBlow;
		m_hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_143, vlhHighLanderBlow );
		if( 0 < (int)vlhHighLanderBlow.size() )
		{
			CDnHighlanderBlow* pHighlanderBlow = static_cast<CDnHighlanderBlow*>(vlhHighLanderBlow.front().GetPointer());
			fResultDamage += pHighlanderBlow->CalcDamage( fResultDamage );
		}
	}

	DWORD dwHitterUniqueID = m_ParentSkillInfo.hSkillUser ? m_ParentSkillInfo.hSkillUser->GetUniqueID() : -1;
	m_hActor->SetHP( m_hActor->GetHP()-(INT64)fResultDamage );
	m_hActor->RequestHPMPDelta( m_ParentSkillInfo.eSkillElement, -(INT64)fResultDamage, dwHitterUniqueID );
#endif // PRE_FIX_44884
}
#endif

bool CDnFreezingPrisonBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	if( 0.0f < m_fIntervalDamage )
	{
		_RequestDamage( m_fIntervalDamage );
	}
#else
	//if( m_hEtcObjectEffect )
	//{	
	//	CEtActionBase::ActionElementStruct* pStruct = m_hEtcObjectEffect->GetElement( "Idle" );
	//	if( pStruct )
	//	{
	//		m_fEffectLength = (float)pStruct->dwLength / 1000.0f;
	//	}

	//	_SetDiffuse( 0.0f, 1.0f, 0.0f, 1.0f );
	//	m_bEffecting = true;
	//}
#endif

	return true;
}

void CDnFreezingPrisonBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	m_IntervalChecker.Process( LocalTime, fDelta );

	if( m_pCantMoveBlow ) 
		m_pCantMoveBlow->Process( LocalTime, fDelta );

	if( m_pCantActionBlow ) 
		m_pCantActionBlow->Process( LocalTime, fDelta );

	if( m_pFrameStopBlow )
		m_pFrameStopBlow->Process( LocalTime, fDelta );
}


void CDnFreezingPrisonBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	//_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );

	// Delete effect. Okay even if it already disappeared.
	//_DetachGraphicEffect();
#endif

	if( m_pCantMoveBlow ) 
		m_pCantMoveBlow->OnEnd( LocalTime, fDelta );

	if( m_pCantActionBlow ) 
		m_pCantActionBlow->OnEnd( LocalTime, fDelta );

	if( m_pFrameStopBlow )
		m_pFrameStopBlow->OnEnd( LocalTime, fDelta );

	// Must do it this way so stiffness is released immediately.
	m_hActor->SetStiffDelta( 0.001f );

#ifdef _GAMESERVER
	// If ending because the duration expired, deal the specified damage.
	if( false == m_bDurabilityDestroyed )
	{
		_RequestDamage( (float)m_iFinalDamage );

		//In #48491, Die is called from ActorStatIntervlaManipulator when applying damage via timer, but OnEnd still needs to handle Die.
#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);
		if (hActor && hActor->IsDie())
			hActor->Die(m_ParentSkillInfo.hSkillUser);

#else
		if (m_hActor->IsDie())
			m_hActor->Die(m_ParentSkillInfo.hSkillUser);
#endif // PRE_FIX_61382
	}
#else

	if (m_hActor->IsDie())
	{
		// Let Effect disappear naturally if the character died..
		if( m_hEtcObjectEffect )
			m_hEtcObjectEffect->SetActionQueue( "Summon_Off" );

		OutputDebug("SetActionQueue ---> Summon_Off\n");
	}
	else
	{
		// When packet arrives from server to delete before duration expires while duration remains on client
		// Show different actions because running out of durability destroys it while running out of duration explodes it.
		if( IsEnd() )
		{
			// Execute explode action on the effect object.
			if( m_hEtcObjectEffect )
				m_hEtcObjectEffect->SetActionQueue( "Attack" );

			OutputDebug("SetActionQueue ---> Attack\n");
		}
		else
		{
			// Execute action where effect object disappears after running out of durability.
			if( m_hEtcObjectEffect )
				m_hEtcObjectEffect->SetActionQueue( "Break" );

			OutputDebug("SetActionQueue ---> Break\n");
		}
	}

	// Break the link when the shatter/break action executes
	TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::SmartPtrSignalImpStruct *pStruct = 
		(TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::SmartPtrSignalImpStruct *)m_hActor->TSmartPtrSignalImp<DnEtcHandle, EtcObjectSignalStruct>::GetStruct( STATEBLOWEFFECT_ETCOFFSET + m_StateBlow.emBlowIndex, GetBlowID() );
	if( pStruct ) 
		pStruct->bLinkObject = false;

	// Remove effect... does not actually delete the effect; the effect itself terminates via the destroy signal.
	// However, since the state effect ends at this point, clear the skill info used as criteria when adding effects here.
	// Otherwise the skill info remains and the effect cannot be added.
	m_hActor->DetachSEEffectSkillInfo( m_ParentSkillInfo );
#endif

	m_IntervalChecker.OnEnd( LocalTime, fDelta );


	//Restore to previously saved value when the state ends
	m_hActor->SetWeight(m_fOrigWight);
	m_hActor->SetPressLevel(m_iOrigPressLevel);

#if defined(_GAMESERVER)
	//Must remove UI when StateBlow ends.
	m_hActor->CmdFreezingPrisonDurablity(GetBlowID(), 0.0f, false);
#endif

#if defined(PRE_FIX_50482)
	if (m_hActor && m_hActor->GetChangeTeamRefCount() == 1)
	{
		m_hActor->SetTeam(m_iOriginalTeam);
	}
	m_hActor->RemoveChangeTeamRefCount();
#else
	// Changes the team to that of the actor (currently a monster) who used this skill.
	m_hActor->SetTeam( m_iOriginalTeam );
#endif // PRE_FIX_50482

	OutputDebug( "CDnFreezingPrisonBlow::OnEnd\n" );
}

#if !defined(_GAMESERVER)
void CDnFreezingPrisonBlow::OnHit()
{
	if (m_hEtcObjectEffect)
		m_hEtcObjectEffect->SetActionQueue( "Hit" );
}

//Get Action signal info from Guage action to check Offset info.
EtVector3 CDnFreezingPrisonBlow::GetGaugePos()
{
	// m_hEtcObjectEffect may be invalid. [2010/12/15 semozz]
	EtVector3 vPos = EtVector3(0.0f, 0.0f, 0.0f);

	if (m_hEtcObjectEffect)
	{
		vPos = *m_hEtcObjectEffect->GetPosition();

		//Only Hit Signal should be here.
		CEtActionBase::ActionElementStruct* pActionElement = m_hEtcObjectEffect->GetElement("Gauge");
		if (pActionElement)
		{
			if (pActionElement->pVecSignalList.size() != 1)
			{
				OutputDebug("Hit Signal does not exist in Gauge info.. !!!\n");
			}
			else
			{
				CEtActionSignal *pSignal = pActionElement->pVecSignalList[0];
				HitStruct *pStruct = pSignal ? (HitStruct *)pSignal->GetData() : NULL;

				if (pStruct)
					vPos += *pStruct->vOffset;
			}
		}

	}

	return vPos;
}
#endif


#ifdef _GAMESERVER

bool CDnFreezingPrisonBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	// For setting to prevent being launched airborne on hit?
	HitParam.vResistance = EtVector3( 0.0f, 0.0f, 0.0f );
	HitParam.vVelocity = EtVector3( 0.0f, 0.0f, 0.0f );
	HitParam.vViewVec = *(m_hActor->GetLookDir());

	return false;
}

float CDnFreezingPrisonBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	// Use the durability reduction info recorded in the hit signal as is.
	m_iDurability -= (int)(HitParam.fDurability * 100.0f);
	
	//Show hit effect & update durability gauge whenever durability drops
	float fCurrentValue = min((float)m_iMaxDurability, max(0.0f, (float)m_iDurability));
	float fRate = m_iMaxDurability != 0 ? (fCurrentValue / (float)m_iMaxDurability) : 0.0f;
	
	// Passing false here hides the UI first and removes the effect in OnEnd
	// There is a sync issue. So always send bShowGauge as true here.
	// Call function in OnEnd to hide the UI..
	bool bShowGauge = true;//fRate > 0.0f;
	m_hActor->CmdFreezingPrisonDurablity(GetBlowID(), fRate, bShowGauge);

	//If durability is 0 or less
	if( m_iDurability <= 0 )
	{
		// Broken due to 0 durability, so end the state effect with nothing happening.
		m_bDurabilityDestroyed = true;
		
		// As soon as CDnStateBlow::Process() finishes, state effects queued here are removed immediately.
		// Also send packet to client side.
		m_hActor->AddReserveRemoveBlow( GetMySmartPtr() );
		OutputDebug( "CDnFreezingBlow::OnCalcDamage - Freezing Broken by Attack!\n" );
	}

	return -fOriginalDamage;
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnFreezingPrisonBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int iDurability[2] = {0, };
	int iFinalDamage[2] = {0, };
	float fIntervalDamage[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		iDurability[0] = atoi( vlTokens[0][0].c_str() );
		iFinalDamage[0] = atoi( vlTokens[0][1].c_str() );
		fIntervalDamage[0] = (float)atof( vlTokens[0][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		iDurability[1] = atoi( vlTokens[1][0].c_str() );
		iFinalDamage[1] = atoi( vlTokens[1][1].c_str() );
		fIntervalDamage[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultDurability = iDurability[0] + iDurability[1];
	int iResultFinalDamage = iFinalDamage[0] + iFinalDamage[1];
	float fResultIntervalDamage = fIntervalDamage[0] + fIntervalDamage[1];

	sprintf_s(szBuff, "%d;%d;%f", iResultDurability, iResultFinalDamage, fResultIntervalDamage);

	szNewValue = szBuff;
}

void CDnFreezingPrisonBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	//Declare variables needed for parsing
	std::vector<string> vlTokens[2];
	string strArgument[2];

	//Required value variable
	int iDurability[2] = {0, };
	int iFinalDamage[2] = {0, };
	float fIntervalDamage[2] = {0.0f, };

	//////////////////////////////////////////////////////////////////////////
	//Parse the first string.
	strArgument[0] = szOrigValue;
	TokenizeA( strArgument[0], vlTokens[0], ";" );

	if( vlTokens[0].size() == 3 ) 
	{
		iDurability[0] = atoi( vlTokens[0][0].c_str() );
		iFinalDamage[0] = atoi( vlTokens[0][1].c_str() );
		fIntervalDamage[0] = (float)atof( vlTokens[0][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//Parse second string
	strArgument[1] = szAddValue;
	TokenizeA( strArgument[1], vlTokens[1], ";" );

	if( vlTokens[1].size() == 3 ) 
	{
		iDurability[1] = atoi( vlTokens[1][0].c_str() );
		iFinalDamage[1] = atoi( vlTokens[1][1].c_str() );
		fIntervalDamage[1] = (float)atof( vlTokens[1][2].c_str() );
	}
	//////////////////////////////////////////////////////////////////////////

	//Add the two values.
	int iResultDurability = iDurability[0] - iDurability[1];
	int iResultFinalDamage = iFinalDamage[0] - iFinalDamage[1];
	float fResultIntervalDamage = fIntervalDamage[0] - fIntervalDamage[1];

	sprintf_s(szBuff, "%d;%d;%f", iResultDurability, iResultFinalDamage, fResultIntervalDamage);

	szNewValue = szBuff;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW