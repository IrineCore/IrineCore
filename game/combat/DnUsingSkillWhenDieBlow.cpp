#include "stdafx.h"
#include "DnUsingSkillWhenDieBlow.h"
#include "DnMonsterActor.h"
#include "DnInvincibleBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnUsingSkillWhenDieBlow::CDnUsingSkillWhenDieBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																								  m_bUsedDieSkill( false ),
																								  m_bDead( false ) 
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_137;
	SetValue( szValue );

	m_fValue = (float)atof( szValue );
}

CDnUsingSkillWhenDieBlow::~CDnUsingSkillWhenDieBlow( void )
{

}

#ifdef _GAMESERVER
bool CDnUsingSkillWhenDieBlow::CanBegin( void )
{
	bool bResult = true;

	// End the state effect immediately if the skill is not owned.
	DnSkillHandle hSkill = m_hActor->FindSkill( (int)m_fValue );
	if( !hSkill )
	{
		OutputDebug( "CDnUsingSkillWhenDieBlow::CanBegin - Don't have skill\n" );
		bResult = false;
	}

	return bResult;
}
#endif

void CDnUsingSkillWhenDieBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	_ASSERT( m_hActor->IsMonsterActor() && "State effect that uses a specific skill upon death is only available for monsters." );
	if( false == m_hActor->IsMonsterActor() )
		SetState( STATE_BLOW::STATE_END );

	// Check whether HP is 0 or not, and if dead, on the game server
	// When CDnActor::Die() is called, restore hp to 1 and defer function processing.
	// Kill it when the used skill ends.
	// Apply invincible state effect from the moment of death..

	OutputDebug( "CDnUsingSkillWhenDieBlow::OnBegin\n");
}

void CDnUsingSkillWhenDieBlow::OnDie( void )
{
	// Apply invincible state effect from the time of death identically on server and client..
	m_hActor->SetHP( 1 );
	m_bDead = true;

	// On game server, set HP to 1 and execute the specified skill.
	// Stop monster AI.
#ifdef _GAMESERVER
	CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
	pMonsterActor->SetAIState( MAAiReceiver::Disable );

	m_hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_069, 0, "10", false , false);
	m_hActor->CmdAddStateEffect( NULL, STATE_BLOW::BLOW_099, -1, "", false, false );
#endif
}

void CDnUsingSkillWhenDieBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifdef _GAMESERVER
	if( m_bDead )
	{
#endif
		if( false == m_bUsedDieSkill )
		{
			// Check whether a skill was used upon dying.	
			// Set criteria so skills are used when in Stand action.
			DnSkillHandle hSkill = m_hActor->GetProcessSkill();
			if( hSkill &&
				(hSkill->GetClassID() == (int)m_fValue) )
			{
				m_bUsedDieSkill = true;
			}
#ifdef _GAMESERVER
			else
			{
				// For using skills in midair, let's implement it first and see.
				// Probably need to defer skill usage until landing on the ground.
				if( strstr( m_hActor->GetCurrentAction(), "Stand") )
				{
					m_hActor->UseSkill( (int)m_fValue, false );


					//#44884 Server/client sync issue occurs when monster uses DieSkill
					//Cases occur where check is not performed on client.
					//Server uses skill and sends sync packet to client..
					//Use existing state effect ShowStateEffect function..

					BYTE pBuffer[128];
					CPacketCompressStream Stream( pBuffer, 128 );

					DWORD dwUniqueID = m_hActor ? m_hActor->GetUniqueID() : -1;
					STATE_BLOW::emBLOW_INDEX blowIndex = STATE_BLOW::BLOW_137;
					bool bShowEffect = true;
					Stream.Write( &dwUniqueID, sizeof(dwUniqueID) );
					Stream.Write( &blowIndex, sizeof(blowIndex));
					Stream.Write( &bShowEffect, sizeof(bShowEffect));

					m_hActor->Send(eActor::SC_SHOW_STATE_EFFECT, &Stream);
				}
				else if( !m_hActor->IsAir() )
				{
					m_hActor->CmdAction("Stand");
				}
			}
#endif
		}

		if( m_bUsedDieSkill )
		{
			// Set HP to 0, play death action, and delete the object.
			// When HP reaches 0, destroy is set in ProcessDie.
			if( false == m_hActor->IsProcessSkill() )
			{
				m_hActor->SetActionQueue( "Die" );
				m_hActor->SetHP( 0 );

#if defined(PRE_FIX_44884)
				//Finally call OnDie here.
				//Because client CDnActor::OnDie is declared protected
				//This state effect is configured to work only on monster actors, so cast to monster actor and call OnDie.
				CDnMonsterActor* pMonsterActor = static_cast<CDnMonsterActor*>(m_hActor.GetPointer());
				if (pMonsterActor)
					pMonsterActor->OnDie(m_FinalHitterActor);

				SetState(STATE_BLOW::STATE_END);
#endif // PRE_FIX_44884

#ifndef _GAMESERVER
				m_hActor->SetDieDelta( 3.0f );
#endif
			}
		}
#ifdef _GAMESERVER
	}
#endif
}

void CDnUsingSkillWhenDieBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	OutputDebug( "CDnUsingSkillWhenDieBlow::OnEnd\n");
}


#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnUsingSkillWhenDieBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}

void CDnUsingSkillWhenDieBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW