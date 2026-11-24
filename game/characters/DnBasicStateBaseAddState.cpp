#include "stdafx.h"
#include "DnBasicStateBaseAddState.h"

CDnBasicStateBaseAddState::CDnBasicStateBaseAddState( DnActorHandle hActor, const char* szValue )
: CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_271;
	SetValue( szValue );
}

CDnBasicStateBaseAddState::~CDnBasicStateBaseAddState(void)
{
	SAFE_DELETE_PVEC( m_vecBasicBlow );
}

void CDnBasicStateBaseAddState::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	std::string str = m_StateBlow.szValue;
	std::vector<std::string> tokens;
	TokenizeA( str, tokens, ";" );

	for( DWORD itr = 0; itr < tokens.size(); ++itr )
	{
		int nStateType = atoi( tokens[itr].c_str() );
		++itr;

		if( itr < tokens.size() )
		{
			float fStateValue = (float)atof( tokens[itr].c_str() );

			AddState( nStateType, fStateValue, LocalTime, fDelta );
		}
	}

	CDnBlow::OnBegin( LocalTime, fDelta );
}

void CDnBasicStateBaseAddState::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}

void CDnBasicStateBaseAddState::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	for( DWORD itr = 0; itr < m_vecBasicBlow.size(); ++itr )
	{
		if( NULL == m_vecBasicBlow[itr] )
			continue;

		m_vecBasicBlow[itr]->OnEnd( LocalTime, fDelta );
	}

	CDnBlow::OnEnd( LocalTime, fDelta );
}

// Reason nStateType changes mid-way is because the table enters the value as a ratio 
// What is actually applied is the base stat multiplied by the ratio, set as an absolute value.
// So when an absolute value is needed, convert it to an absolute-value state blow.
void CDnBasicStateBaseAddState::AddState( int nStateType, const float fStateValue, LOCAL_TIME LocalTime, float fDelta )
{
	CDnState * pBaseState = const_cast<CDnState*>(m_hActor->GetBaseState());

	if( NULL == pBaseState )
		return;

	char buff[64] = {0,};
	switch( nStateType )
	{
	case STATE_BLOW::BLOW_058:
		{
			INT64 nMaxHP = (INT64)(pBaseState->GetMaxHP() * fStateValue);
			sprintf_s( buff, "%I64d", nMaxHP );

			nStateType = STATE_BLOW::BLOW_009;
			AddBasicBlow( nStateType, buff, LocalTime, fDelta );

#ifdef _GAMESERVER
			m_hActor->SetHP(m_hActor->GetMaxHP());
			m_hActor->RequestHPMPDelta( CDnState::ElementEnum_Amount, m_hActor->GetMaxHP(), m_hActor->GetUniqueID() , false , false );
#endif
		}
		return;

	case STATE_BLOW::BLOW_002:
		{
			int nAttackP = (int)( pBaseState->GetAttackPMin() + pBaseState->GetAttackPMax() ) / 2;
			nAttackP = (int)(nAttackP * fStateValue);
			sprintf_s( buff, "%d", nAttackP );
			nStateType = STATE_BLOW::BLOW_001;
		}
		break;

	case STATE_BLOW::BLOW_029:
		{
			int nAttackM = (int)( pBaseState->GetAttackMMin() + pBaseState->GetAttackMMax() ) / 2;
			nAttackM = (int)(nAttackM * fStateValue);
			sprintf_s( buff, "%d", nAttackM );
			nStateType = STATE_BLOW::BLOW_028;
		}
		break;

	case STATE_BLOW::BLOW_004:
		{
			int nDefenseP = (int)(pBaseState->GetDefenseP() * fStateValue);
			sprintf_s( buff, "%d", nDefenseP );
			nStateType = STATE_BLOW::BLOW_003;
		}
		break;

	case STATE_BLOW::BLOW_094:
		{
			int nDefenseM = (int)(pBaseState->GetDefenseM() * fStateValue);
			sprintf_s( buff, "%d", nDefenseM );
			nStateType = STATE_BLOW::BLOW_093;
		}
		break;

	case STATE_BLOW::BLOW_061:
		{
			int nSuperAmmor = (int)(pBaseState->GetSuperAmmor() * fStateValue);
			sprintf_s( buff, "%d", nSuperAmmor );
		}
		break;

	default:
		return;
	}

	AddBasicBlow( nStateType, buff, LocalTime, fDelta );
}

void CDnBasicStateBaseAddState::AddBasicBlow( const int nStateType, char * buff, LOCAL_TIME LocalTime, float fDelta )
{
	CDnBasicBlow * pBasicBlow = new CDnBasicBlow( m_hActor, buff );
	pBasicBlow->SetBlow( (STATE_BLOW::emBLOW_INDEX)nStateType );
	pBasicBlow->SetParentSkillInfo( &m_ParentSkillInfo );

	pBasicBlow->SetDurationTime( m_StateBlow.fDurationTime );
	pBasicBlow->OnBegin(LocalTime, fDelta);

	m_vecBasicBlow.push_back( pBasicBlow );
}