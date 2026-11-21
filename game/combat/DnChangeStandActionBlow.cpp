#include "stdafx.h"
#include "DnChangeStandActionBlow.h"
#include "DnCantMoveBlow.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnChangeStandActionBlow::CDnChangeStandActionBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_121; 
	
	DNVector(std::string)		vSplitStr;
	TokenizeA(szValue, vSplitStr, ";");


	if (vSplitStr.size() > 0)
	{
		m_strActionPrefixName = vSplitStr[0].substr(0, vSplitStr[0].find_last_of("_"));
	}

	if (vSplitStr.size() == 3)
	{
		//int+str attack
		//Skill_SiegeStance_Loop;15;Skill_SiegeStance_Attack
	}

	SetValue(vSplitStr[0].c_str());

	m_strStartActionName = m_strActionPrefixName + "_Start";
	m_strLoopActionName = m_strActionPrefixName + "_Loop";
	m_strEndActionName = m_strActionPrefixName + "_End";

	// End action may be absent. Set if needed, otherwise it looks abrupt when the state effect ends.
	CEtActionBase::ActionElementStruct* pEndActionElement = m_hActor->GetElement( m_strEndActionName.c_str() );
	if( NULL == pEndActionElement )
		m_strEndActionName.clear();

#ifdef _CLIENT
	if( m_hActor && m_hActor->IsPlayerActor() ) 
	{
		CDnPlayerActor* pPlayerActor = static_cast<CDnPlayerActor*>(m_hActor.GetPointer());
		if( pPlayerActor&& pPlayerActor->IsLocalActor() && !pPlayerActor->IsBattleMode() )
			pPlayerActor->CmdToggleBattle( true );
	}
#endif

	m_bIgnoreEndAction = false;
}

CDnChangeStandActionBlow::~CDnChangeStandActionBlow(void)
{

}

void CDnChangeStandActionBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CEtActionBase::ActionElementStruct* pActionElement = m_hActor->GetElement( m_StateBlow.szValue.c_str() );
	if( pActionElement )
	{
		// When using Siege Stance while continuously moving, stop immediately to use it.. (#15948)
		//m_hActor->SetActionQueue( m_strStartActionName.c_str() );
		if( m_hActor->IsCustomAction() ) 
		{
			m_hActor->ResetCustomAction();
#ifndef _GAMESERVER
			m_hActor->ResetMixedAnimation();
#endif
		}

		// If there is no Start action, perform the designated action.
		CEtActionBase::ActionElementStruct* pStartActionElement = m_hActor->GetElement( m_strStartActionName.c_str() );
		if( pStartActionElement )
			m_hActor->CmdStop( m_strStartActionName.c_str() );
		else
			m_hActor->CmdStop( m_StateBlow.szValue.c_str() );
	}
	else if( NULL == pActionElement )
	{
		SetState( STATE_BLOW::STATE_END );
	}
}


void CDnChangeStandActionBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnChangeStandActionBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	if( false == m_hActor->IsDie() )
	{
		if( m_hActor->IsMonsterActor() ) 
		{
			if( m_hActor->IsProcessSkill() ) m_hActor->CancelUsingSkill();
			// Call with flag set so action is queued even when unable to move/act. (#17694)
			if( false == m_strEndActionName.empty() )
			{
				m_hActor->SetActionQueue( m_strEndActionName.c_str(), 0, 3.0f, 0.0f, true, false );
			}
			else
			{
				m_hActor->SetActionQueue( "Stand", 0, 3.0f, 0.0f, true, false );
			}
		}
		else 
		{
			if( strstr( m_hActor->GetCurrentAction(), m_strActionPrefixName.c_str() ) )
			{
				// Call with flag set so action is queued even when unable to move/act. (#17694)
				if( false == m_strEndActionName.empty() )
				{
					m_hActor->SetActionQueue( m_strEndActionName.c_str(), 0, 3.0f, 0.0f, true, false );
				}
				else
				{
					// If the current action equals the loop action, switch to the Stand action.
					// If an action with a stand-action-change state-effect cancel signal (like jump) was executed
					// Check to prevent switching to Stand action during Jump action.
					if( m_bIgnoreEndAction == false )
						m_hActor->SetActionQueue( "Stand", 0, 3.0f, 0.0f, true, false );
				}
			}
			else
			{
				bool bApplyAction = true;

				// #26278 If not a designated skill action and neither airborne nor downed, immediately take the Stand action.
				// #28122, #28107 Do not transition to Stand as the end action when using a skill while a stand action change state effect is active.
				if( m_hActor->IsAir() || m_hActor->IsDown() || m_hActor->IsProcessSkill() )
					bApplyAction = false;

#ifdef PRE_FIX_81750
				if( m_hActor->IsHit() )
					bApplyAction = false;
#endif
				
				if( bApplyAction == true )
					m_hActor->SetActionQueue( "Stand", 0, 3.0f, 0.0f, true, false );
			}
		}
	}
}

#ifdef _GAMESERVER
bool CDnChangeStandActionBlow::IsChangeActionSet( const char* szActionName )
{
	string strPrefixName = szActionName;
	strPrefixName = strPrefixName.substr( 0, strPrefixName.find_last_of("_") );
	if( strcmp( strPrefixName.c_str(), m_strActionPrefixName.c_str() ) == 0 )
		return true;

	return false;
}

void CDnChangeStandActionBlow::ReleaseStandChangeSkill( DnActorHandle hActor ,bool bCheckUnRemovableSkill, const char* szEndAction )
{
	if( !hActor ) 
		return;

	if( hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_121 ) == true )
	{
		DNVector( DnBlowHandle ) vlhAllAppliedBlows;
		hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_121, vlhAllAppliedBlows );

		for( DWORD i = 0; i < vlhAllAppliedBlows.size(); ++i )
		{
			if( vlhAllAppliedBlows[i] && vlhAllAppliedBlows[i]->IsBegin() == false )
			{
				const CDnSkill::SkillInfo* pSkillInfo = vlhAllAppliedBlows[i]->GetParentSkillInfo();
				if( pSkillInfo && pSkillInfo->iSkillID > 0 )
				{
					const int SKILL_STATE_OF_FATE = 3221;
					if( ( pSkillInfo->iSkillID != SKILL_STATE_OF_FATE ) || bCheckUnRemovableSkill == false )
					{
						DNVector( DnBlowHandle ) vlhSameSkillAppliedBlows;
						hActor->GetAllAppliedStateBlowBySkillID( pSkillInfo->iSkillID, vlhSameSkillAppliedBlows );
						for( int k = 0; k < (int)vlhSameSkillAppliedBlows.size(); ++k ) 
						{
							DnBlowHandle hSameSkillBlow = vlhSameSkillAppliedBlows.at( k );
							hActor->CmdRemoveStateEffectFromID( hSameSkillBlow->GetBlowID() );
						}
					}

					if( szEndAction != NULL )
						static_cast<CDnChangeStandActionBlow*>(vlhAllAppliedBlows[i].GetPointer())->SetEndAction( szEndAction );
				}
			}
		}
	}
}

bool CDnChangeStandActionBlow::CheckUsableAction( DnActorHandle hActor, bool bCheckUnRemovableSkill, const char* szActionName )
{
	if( !hActor )
		return false;

	if( hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_121 ) )
	{
		DNVector( DnBlowHandle ) vecBlow;
		hActor->GatherAppliedStateBlowByBlowIndex( STATE_BLOW::BLOW_121, vecBlow );

		for( DWORD i = 0; i < vecBlow.size(); ++i )
		{
			if( vecBlow[i] )
			{
				CDnChangeStandActionBlow* pBlow = static_cast<CDnChangeStandActionBlow*>( vecBlow[i].GetPointer() );
				if( bCheckUnRemovableSkill == true )
				{
					const int SKILL_STATE_OF_FATE = 3221;
					if( pBlow->GetParentSkillInfo()->iSkillID == SKILL_STATE_OF_FATE )
						return true;
				}

				if( pBlow->IsChangeActionSet( szActionName ) == false )
					return false;
			}
		}
	}

	return true;
}

#endif

void CDnChangeStandActionBlow::SetEndAction( const char* pEndAction )
{
	if( pEndAction && 0 < strlen( pEndAction ) )
	{
		m_strEndActionName = pEndAction;
	}
	else
	{
		m_strEndActionName.clear();
		m_bIgnoreEndAction = true;
	}
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnChangeStandActionBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnChangeStandActionBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW