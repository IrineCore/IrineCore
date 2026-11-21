#include "stdafx.h"
#include "DnCooltimeParryBlow.h"
#ifdef _GAMESERVER
#include "DnStateBlow.h"
#include "DnPlayerActor.h"
#else
#include "DnLocalPlayerActor.h"
#include "DnInterface.h"
#include "DnMainDlg.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#endif

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnCooltimeParryBlow::CDnCooltimeParryBlow( DnActorHandle hActor, const char *szValue )
											: CDnBlow(hActor)
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_153;
	AddCallBackType(SB_ONDEFENSEATTACK);
	SetValue(szValue);
	//m_nValue = atoi( szValue );
	m_fValue = (float)atof(szValue);

#ifdef _GAMESERVER
	m_strParringActionName.assign( "Skill_Parrying" );
	m_bEnable = false;
	m_bEnableLastLoop = false;
	m_fAdditionalSignalProb = 0.0f;

	m_dwCoolTime = 0;
	m_dwLastParryingTime = 0;

	m_fAdditionalSignalProbLastLoop = 0.0f;
#else
	m_pLocalPlayerActor = NULL;
#endif

}

CDnCooltimeParryBlow::~CDnCooltimeParryBlow(void)
{
#ifndef _GAMESERVER
	// TODO: Leveling up a skill while cooldown is active might crash/break.. needs testing.
	SAFE_RELEASE_SPTR( m_hParentSkillForCoolTime );
#endif
}

void CDnCooltimeParryBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	// State effect process runs near the end of actor process, so resetting here will match the parry signal's enable interval.
	m_bEnableLastLoop = m_bEnable;
	m_fAdditionalSignalProbLastLoop = m_fAdditionalSignalProb;
	m_bEnable = false;
	m_fAdditionalSignalProb = 0.0f;
#else
	if( m_pLocalPlayerActor && m_hParentSkillForCoolTime )
	{
		// Advance cooldown.
		m_hParentSkillForCoolTime->Process( LocalTime, fDelta ); 
	}
#endif
}

void CDnCooltimeParryBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	// Use parent skill's cooldown info as-is.
	if( m_ParentSkillInfo.hSkillUser )
	{
		DnSkillHandle hParentSkill = m_ParentSkillInfo.hSkillUser->FindSkill( m_ParentSkillInfo.iSkillID );
		m_dwCoolTime = DWORD(hParentSkill->GetDelayTime() * 1000.0f);
	}
#else
	m_pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>( m_hActor.GetPointer() );
	if( !m_hParentSkillForCoolTime && m_pLocalPlayerActor )
	{
		m_hParentSkillForCoolTime = CDnSkill::CreateSkill( m_hActor, m_ParentSkillInfo.iSkillID, 
#if defined(PRE_FIX_NEXTSKILLINFO)
															m_ParentSkillInfo.nSkillLevel);
#else
														   m_ParentSkillInfo.iSkillLevelID - m_ParentSkillInfo.iSkillLevelIDOffset + 1 );
#endif // PRE_FIX_NEXTSKILLINFO

#if defined( PRE_FIX_82147 )
		if( m_hParentSkillForCoolTime && m_hActor->IsPlayerActor() )
		{
			int iSkillLevelDataType = CDnSkill::PVE;

			CDnGameTask *pGameTask = (CDnGameTask *)CTaskManager::GetInstance().GetTask( "GameTask" );
			if( pGameTask && pGameTask->GetGameTaskType() == GameTaskType::PvP )
				iSkillLevelDataType = CDnSkill::PVP;

			m_hParentSkillForCoolTime->SelectLevelDataType( iSkillLevelDataType );
		}
#endif

	}
#endif

	OutputDebug( "CDnCooltimeParryBlow::OnBegin\n" );
}

void CDnCooltimeParryBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
#ifndef _GAMESERVER
	//if( m_pLocalPlayerActor )
	//	m_hParentSkillForCoolTime->OnEnd( 0, 0.0f );
#endif

	OutputDebug( "CDnCooltimeParryBlow::OnEnd\n" );
}

#ifdef _GAMESERVER
bool CDnCooltimeParryBlow::OnDefenseAttack( DnActorHandle hHitter, CDnState* pAttackerState, CDnDamageBase::SHitParam &HitParam, bool bHitSuccess )
{
	if( !m_bEnableLastLoop )
		return false;

	if( !bHitSuccess ) 
		return false;

#if defined(PRE_FIX_BLOCK_CONDITION)
	//Allow block to trigger if attacker is an enemy even when fDamage is 0..
	//For allies, do not block when fDamage is 0 as before.
	if (IsCanBlock(hHitter, m_hActor, HitParam) == false)
		return false;
#else
	// Changed so it does not trigger when hit percent is 0%. (#21175)
	if( 0.0f == HitParam.fDamage )
		return false;
#endif // PRE_FIX_BLOCK_CONDITION

#if defined(PRE_ADD_49166)
	// Prevent block from triggering if target has Freezing / FrameStop debuff.
	if (IsInVaildBlockCondition(m_hActor) == true)
		return false;
#endif // PRE_ADD_49166

	// Cannot be used if elapsed time since last use is less than cooldown.
	if( GetTickCount() - m_dwLastParryingTime < m_dwCoolTime )
		return false;

	// If it is a player,
	if( m_hActor->IsPlayerActor() )
	{
		// Does not activate when not holding a weapon. #26772
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( false == (pPlayerActor->IsBattleMode() && pPlayerActor->GetWeapon( 0 ) && pPlayerActor->GetWeapon( 1 )) )
			return false;
		if( pPlayerActor->IsTransformMode() )
			return false;
	}

	float fAddProb = m_fAdditionalSignalProbLastLoop;
	if( m_hActor->GetStateBlow()->IsApplied( STATE_BLOW::BLOW_056 ) )
	{
		DNVector(DnBlowHandle) vlhParryingProbBlow;
		m_hActor->GetStateBlow()->GetStateBlowFromBlowIndex( STATE_BLOW::BLOW_056, vlhParryingProbBlow );
		int iNumBlow = (int)vlhParryingProbBlow.size();
		for( int i = 0; i < iNumBlow; ++i )
			fAddProb += vlhParryingProbBlow.at( i )->GetFloatValue();
	}

	int iProb = int((m_fValue+fAddProb) * 10000.0f);
	int iRandValue = (_rand(GetRoom())%10000);
	if( iRandValue <= iProb )
	{
		// Set appropriate values in HitParam.
		HitParam.szActionName = m_strParringActionName;
		OutputDebug( "CDnCooltimeParryingBlow::OnDefenseAttack\n" );

		// Update the last time parrying was used.
		m_dwLastParryingTime = GetTickCount();

		// Notify those who applied it. /////////////////////////////////////////////
		boost::shared_ptr<IDnObserverNotifyEvent> pEvent( IDnObserverNotifyEvent::Create( EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS ) );
		pEvent->SetSkillID( m_ParentSkillInfo.iSkillID );
		Notify( pEvent );
		//////////////////////////////////////////////////////////////////////////

		// Notify client via packet; currently client has no other way to know this timing.
		static_cast<CDnPlayerActor*>(m_hActor.GetPointer())->RequestCooltimeParrySuccess( m_ParentSkillInfo.iSkillID );

		return true;
	}
	else
	{
		return false;
	}
}
#else
void CDnCooltimeParryBlow::OnSuccess( void )
{
	// Called when a separate packet is sent from server upon successful cooldown parry.
	// Only show cooldown for the local client.
	if( m_pLocalPlayerActor )
	{
		// Only advance cooldown. For UI display.
		m_hParentSkillForCoolTime->OnBeginCoolTime();

		if( m_hParentSkillForCoolTime->HasCoolTime() )
		{
			CDnMainDlg *pMainDlg = (CDnMainDlg*)GetInterface().GetMainBarDialog();
			if( pMainDlg ) 
			{
				pMainDlg->AddPassiveSkill( m_hParentSkillForCoolTime );
			}
		}
	}
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnCooltimeParryBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnCooltimeParryBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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
