#include "stdafx.h"
#include "DnBubbleConditionChecker.h"
#include "DnObserverEventMessage.h"
#include "DnObservable.h"
#include "DnBubbleSystem.h"
#include "DNGameRoom.h"
#include "DnActor.h"

namespace BubbleSystem
{

// Create an object for each condition check type.
IDnConditionChecker* IDnConditionChecker::Create( int iConditionCheckerType, const char* pArgument )
{
	IDnConditionChecker* pConditionChecker = NULL;

	switch( iConditionCheckerType )
	{
		case BUBBLE_CONDITION::ON_USE_SKILL:
			pConditionChecker = new CDnUseSkill;
			break;

		case BUBBLE_CONDITION::BLOCK_SUCCESS:
			pConditionChecker = new CDnBlockSuccess;
			break;
		
		case BUBBLE_CONDITION::BUBBLE_COUNT_GREATER:
			pConditionChecker = new CDnBubbleCountGreater;
			break;

		case BUBBLE_CONDITION::BUBBLE_COUNT_UPDATED:
			pConditionChecker = new CDnBubbleCountUpdated;
			break;

		case BUBBLE_CONDITION::OWN_THIS_SKILL:
			pConditionChecker = new CDnOwnThisSkill;
			break;

		case BUBBLE_CONDITION::PARRING_SUCCESS:
			pConditionChecker = new CDnParringSuccess;
			break;

		case BUBBLE_CONDITION::COOLTIME_PARRING_SUCCESS:
			pConditionChecker = new CDnCooltimeParringSuccess;
			break;

		case BUBBLE_CONDITION::DO_NORMAL_ATTACK:
			pConditionChecker = new CDnDoNormalAttack;
			break;

		case BUBBLE_CONDITION::PLAYER_KILL_TARGET_ON_GHOUL_MODE:
			pConditionChecker = new CDnPlayerKillTargetOnGhoulMode;
			break;

		case BUBBLE_CONDITION::ON_USE_SKILL_WITH_SPECIFIC_SKILLLEVEL:
			pConditionChecker = new CDnUseSkillWithSpecificSkillLevel;
			break;

		case BUBBLE_CONDITION::PROBABILITY:
			pConditionChecker = new CDnProbability;
			break;

		//Master Dark Avenger incrase bubble on critical damage !
		case BUBBLE_CONDITION::ONCRITICALHIT:
			pConditionChecker = new CDnOnCriticalHit;
			break;

		default:
			return NULL;
	}

	pConditionChecker->Initialize( pArgument );

	return pConditionChecker;
}

// CDnUseSkill 
void CDnUseSkill::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iSkillID = atoi( pArgument );
}

bool CDnUseSkill::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	if( EVENT_BUBBLE_ON_USE_SKILL == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		if( static_cast<CDnUseSkillMessage*>( pEvent )->GetSkillID() == m_iSkillID )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////


// CDnBlockSuccess
void CDnBlockSuccess::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iSkillID = atoi( pArgument );
}

bool CDnBlockSuccess::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	// Is a block success message, and
	if( EVENT_BUBBLE_BLOCK_SUCCESS == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		if( static_cast<CDnBlockSuccessMessage*>( pEvent )->GetSkillID() == m_iSkillID )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////


// CDnBubbleCountGreater
void CDnBubbleCountGreater::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	// After splitting the string by whitespace
	std::vector<string> vlTokens;
	string strArgument( pArgument );
	TokenizeA( strArgument, vlTokens, ";" );

	_ASSERT( 2 == (int)vlTokens.size() );

	m_iBubbleTypeID = atoi( vlTokens.at(0).c_str() );
	m_iBasisCount = atoi( vlTokens.at(1).c_str() );
}

bool CDnBubbleCountGreater::IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* /*pEvent*/ )
{
	bool bResult = false;

	// When the count of a specific bubble is at least this count.
	if( m_iBasisCount <= pBubbleSystem->GetBubbleCountByTypeID( m_iBubbleTypeID ) )
	{
		bResult = true;
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnBubbleCountUpdated
void CDnBubbleCountUpdated::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iBubbleTypeID = atoi( pArgument );
}

bool CDnBubbleCountUpdated::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	if( EVENT_BUBBLE_COUNT_UPDATED == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		if( static_cast<CDnBubbleCountUpdatedMessage*>( pEvent )->GetBubbleTypeID() == m_iBubbleTypeID )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnOwnThisSkill
void CDnOwnThisSkill::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iSkillID = atoi( pArgument );
}

bool CDnOwnThisSkill::IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	// Matches the skill ID required by this condition check object.
	DnActorHandle hActor = pBubbleSystem->GetHasActor();
	if( hActor && hActor->FindSkill( m_iSkillID ) )
	{
		bResult = true;
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnParringSuccess
void CDnParringSuccess::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iSkillID = atoi( pArgument );
}

bool CDnParringSuccess::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	// Is a block success message, and
	if( EVENT_BUBBLE_PARRING_SUCCESS == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		if( static_cast<CDnParringSuccessMessage*>( pEvent )->GetSkillID() == m_iSkillID )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnCooltimeParringSuccess
void CDnCooltimeParringSuccess::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	m_iSkillID = atoi( pArgument );
}

bool CDnCooltimeParringSuccess::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	// Is a block success message, and
	if( EVENT_BUBBLE_COOLTIME_PARRING_SUCCESS == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		if( static_cast<CDnCooltimeParringSuccessMessage*>( pEvent )->GetSkillID() == m_iSkillID )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnCooltimeParringSuccess
void CDnDoNormalAttack::Initialize( const char* pArgument )
{
	// Takes no arguments.
	if( NULL == pArgument )
		return;

}

bool CDnDoNormalAttack::IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	DnActorHandle hActor = pBubbleSystem->GetHasActor();
	if( EVENT_ONCHANGEACTION == pEvent->GetEventType() )
	{
		// TODO: Code distinguishing normal attacks exists in about two other places besides here; extract into a function.
		if( false == hActor->IsProcessSkill() &&
			false == hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_121) &&
			false == hActor->IsAppliedThisStateBlow(STATE_BLOW::BLOW_129) )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnCooltimeParringSuccess
void CDnPlayerKillTargetOnGhoulMode::Initialize( const char* pArgument )
{
	// Takes no arguments.
	if( NULL == pArgument )
		return;

}

bool CDnPlayerKillTargetOnGhoulMode::IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	DnActorHandle hActor = pBubbleSystem->GetHasActor();
	if( EVENT_PLAYER_KILL_TARGET == pEvent->GetEventType() )
	{

		// true if in Ghoul mode.
		if(hActor && hActor->GetGameRoom()->bIsZombieMode())
			bResult = true;
	}

	return bResult;
}
/////////////////////////////////////////////////////////////////////////

// CDnUseSkillWithSpecificSkillLevel 
void CDnUseSkillWithSpecificSkillLevel::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;

	vector<string> vlArgs;
	TokenizeA( pArgument, vlArgs, ";" );

	if( 2 == (int)vlArgs.size() )
	{
		m_iSkillID = atoi( vlArgs.at( 0 ).c_str() );
		m_iSkillLevel = atoi( vlArgs.at( 1 ).c_str() );
	}
}

bool CDnUseSkillWithSpecificSkillLevel::IsSatisfy( CDnBubbleSystem* /*pBubbleSystem*/, ::IDnObserverNotifyEvent* pEvent )
{
	bool bResult = false;

	if( EVENT_BUBBLE_ON_USE_SKILL == pEvent->GetEventType() )
	{
		// Matches the skill ID required by this condition check object.
		CDnUseSkillMessage* pMsg = static_cast<CDnUseSkillMessage*>( pEvent );
		if( pMsg->GetSkillID() == m_iSkillID &&
			pMsg->GetSkillLevel() == m_iSkillLevel )
		{
			bResult = true;
		}
	}

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnProbability 
void CDnProbability::Initialize( const char* pArgument )
{
	if( NULL == pArgument )
		return;
	
	// Range 0.0 ~ 1.0
	m_fProbability = (float)atof( pArgument );
	m_fProbability *= 10000.0f;
}

bool CDnProbability::IsSatisfy( CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* /*pEvent*/ )
{
	bool bResult = false;

	float fRandomResult = float(_rand(pBubbleSystem->GetHasActor()->GetRoom()) % 10000);
	bResult = fRandomResult <= m_fProbability;

	return bResult;
}
//////////////////////////////////////////////////////////////////////////

// CDnProbability 
void CDnOnCriticalHit::Initialize(const char* pArgument)
{
	if (NULL == pArgument)
		return;
	
	m_vecCheckSkills.clear();

	std::vector<std::string> tempVecSkillList;
	TokenizeA(pArgument, tempVecSkillList, ";");

	if (tempVecSkillList.size() == 0)
		return;

	for (DWORD i = 0; i < tempVecSkillList.size(); i++)
		m_vecCheckSkills.push_back(atoi(tempVecSkillList[i].c_str()));

}

bool CDnOnCriticalHit::IsSatisfy(CDnBubbleSystem* pBubbleSystem, ::IDnObserverNotifyEvent* pEvent)
{
	bool bResult = false;

	if (EVENT_ONCRITICALHIT == pEvent->GetEventType())
	{
		CDnOnCriticalHitMessage* pMsg = static_cast<CDnOnCriticalHitMessage*>(pEvent);
		if (!pMsg)
			return false;

		int nSkillID = pMsg->GetSkillID();
		if(nSkillID > 0)
		{
			if (std::find(m_vecCheckSkills.begin(), m_vecCheckSkills.end(), nSkillID) != m_vecCheckSkills.end())
			{
				bResult = true;
			}
		}
	}

	return bResult;
}

//////////////////////////////////////////////////////////////////////////


}