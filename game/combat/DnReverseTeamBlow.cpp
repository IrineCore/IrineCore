#include "stdafx.h"
#include "DnReverseTeamBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnReverseTeamBlow::CDnReverseTeamBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																					  m_iType( HIT_SIGNAL_TARGET_CHANGE )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_086;
	SetValue( szValue );
	m_fValue = (float)atof( szValue );

	m_iType = (int)m_fValue;
}

CDnReverseTeamBlow::~CDnReverseTeamBlow(void)
{

}

void CDnReverseTeamBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	switch( m_iType )
	{
		case HIT_SIGNAL_TARGET_CHANGE:
			{
				OutputDebug( "CDnReverseTeamBlow::OnBegin (Type:HIT_SIGNAL_TARGET_CHANGE)\n");
			}
			break;

		case TEAM_NUMBER_CHANGE:
			{
				// Invert team
				CDnActor::ActorTypeEnum ActorType = m_hActor->GetActorType();

				// If player, treat as monster.
				if( ActorType <= CDnActorState::Reserved6 ) 
					m_hActor->SetTeam( 1 );
				else
					// If monster, treat as player.
					m_hActor->SetTeam( 0 );

				OutputDebug( "CDnReverseTeamBlow::OnBegin (Type:TEAM_NUMBER_CHANGE)\n");
			}
			break;

		case HIT_SIGNAL_TARGET_ALL:
			{
				OutputDebug( "CDnReverseTeamBlow::OnBegin (Type:HIT_SIGNAL_TARGET_ALL)\n");
			}
			break;
	}
}


void CDnReverseTeamBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnReverseTeamBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	switch( m_iType )
	{
		case HIT_SIGNAL_TARGET_CHANGE:
			{
				OutputDebug( "CDnReverseTeamBlow::OnEnd (Type:HIT_SIGNAL_TARGET_CHANGE)\n");
			}
			break;

		case TEAM_NUMBER_CHANGE:
			{
				// Restore team to original setting
				CDnActor::ActorTypeEnum ActorType = m_hActor->GetActorType();
				//if( ActorType <= CDnActorState::Reserved6 ) 
				if( m_hActor->IsPlayerActor() )
					m_hActor->SetTeam( 0 );
				else
					m_hActor->SetTeam( 1 );

				OutputDebug( "CDnReverseTeamBlow::OnEnd (Type:TEAM_NUMBER_CHANGE)\n");
			}
			break;

		case HIT_SIGNAL_TARGET_ALL:
			{
				OutputDebug( "CDnReverseTeamBlow::OnEnd (Type:HIT_SIGNAL_TARGET_ALL)\n");
			}
			break;
	}

}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnReverseTeamBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnReverseTeamBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW
