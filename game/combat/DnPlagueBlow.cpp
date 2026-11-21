#include "stdafx.h"
#include "DnPlagueBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

const LOCAL_TIME PLAGUE_DAMAGE_INTERVAL = 2000;


CDnPlagueBlow::CDnPlagueBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																			m_IntervalChecker( hActor, GetMySmartPtr() ),
																			m_fDamageProb( 0.f ),
																			m_nPlageProbability( 0 ),
																			m_fDamageAbsolute( 0.f ),
																			m_fPlagueRange( 0.f ),
																			m_fEffectLength( 0.f ),
																			m_bEffecting( false )
#ifdef _GAMESERVER
																			,m_fIntervalDamage( 0.0f )
#endif
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_136;
	SetValue( szValue );

	std::vector<std::string> tokens;
	TokenizeA( szValue, tokens , ";" );

	if( tokens.size() == 2 )
	{
		m_nPlageProbability = 100;
		m_fDamageProb = (float)atof( tokens[0].c_str() );
		m_fPlagueRange = (float)atof( tokens[1].c_str() );
	}
	else if( tokens.size() == 4 )
	{
		m_nPlageProbability = (float)atof( tokens[0].c_str() ) * 100.f;
		m_fDamageProb = (float)atof( tokens[1].c_str() );
		m_fDamageAbsolute = (float)atof( tokens[2].c_str() );
		m_fPlagueRange = (float)atof( tokens[3].c_str() );
	}

#ifdef _GAMESERVER
	// If transmitted, the damage ratio will exceed 1.0; in this case, apply the damage as is.
	if( m_fDamageProb < 1.0f )
		m_fIntervalDamage = (m_hActor->GetLastDamage() * m_fDamageProb) + m_fDamageAbsolute;
	else
		m_fIntervalDamage = m_fDamageProb;
#endif

#ifndef _GAMESERVER
	UseTableDefinedGraphicEffect( false );
#endif

}

CDnPlagueBlow::~CDnPlagueBlow(void)
{

}


#ifdef _GAMESERVER
bool CDnPlagueBlow::CanBegin( void )
{
	return true;
}
#endif

bool CDnPlagueBlow::OnCustomIntervalProcess( void )
{
#ifdef _GAMESERVER
	if( 0.0f < m_fIntervalDamage )
	{
#if defined(PRE_FIX_61382)
		DnActorHandle hActor = CDnActor::GetOwnerActorHandle(m_hActor);

		if (hActor && 
			hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);
		}
#else
		if (m_hActor->IsDie() == false)
		{
			m_hActor->RequestDamageFromStateBlow(GetMySmartPtr(), (int)m_fIntervalDamage);
		}
#endif // PRE_FIX_61382
	}

	// Search nearby allies for a target to transfer to.
	// When transferred, ensure it is processed with the same damage.
	DNVector( DnActorHandle ) vlActorsInRange;
	CDnActor::ScanActor( m_hActor->GetRoom(), *m_hActor->GetPosition(), m_fPlagueRange, vlActorsInRange );

	// Transfer to the nearest target. Do not transfer to a target already afflicted with the contagion state effect.
	float fShortestLengthSQ = FLT_MAX;
	int iNumActorsInRange = (int)vlActorsInRange.size();
	DnActorHandle hResultTarget;
	for( int i = 0; i < iNumActorsInRange; ++i )
	{
		DnActorHandle hActor = vlActorsInRange.at( i );
		// Allies only.
		if( hActor->GetTeam() == m_hActor->GetTeam() )
		{
			// Except those already infected.
			if( false == hActor->IsAppliedThisStateBlow( STATE_BLOW::BLOW_136 ) )
			{
				int iRandVal = _rand(GetRoom()) % 100;
				if( iRandVal < m_nPlageProbability )
				{
					EtVector3 vLength = *hActor->GetPosition() - *m_hActor->GetPosition();
					float fLengthSQ = EtVec3LengthSq( &vLength );
					if( fLengthSQ < fShortestLengthSQ )
					{
						fShortestLengthSQ = fLengthSQ;
						hResultTarget = hActor;
						break;
					}
				}
			}
		}
	}

	if( hResultTarget )
	{
		// If a resulting actor exists, add contagion state effect.
		// Since it spreads by contagion rather than being inflicted by an attack, pass the determined damage as is.
		char acBuf[ STATE_BLOW_ARGUMENT_MAX_SIZE ] = { 0 };
		sprintf_s( acBuf, sizeof(acBuf), "%2.2f;%2.2f", m_fIntervalDamage, m_fPlagueRange );
		hResultTarget->CmdAddStateEffect( &m_ParentSkillInfo, m_StateBlow.emBlowIndex, 
			int(m_fOriginalDurationTime*1000.0f), acBuf );
	}
#else
	_AttachGraphicEffect();
	if( m_hEtcObjectEffect )
	{	
		CEtActionBase::ActionElementStruct* pStruct = m_hEtcObjectEffect->GetElement( "Idle" );
		if( pStruct )
		{
			m_fEffectLength = (float)pStruct->dwLength / 1000.0f;
		}

		_SetDiffuse( 0.0f, 1.0f, 0.0f, 1.0f );
		m_bEffecting = true;
	}
#endif

	return true;
}


void CDnPlagueBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
#ifdef _GAMESERVER
	if( m_fIntervalDamage < 1.0f )
	{
		SetState( STATE_BLOW::STATE_END );
		OutputDebug( "CDnPlagueBlow::OnBegin - Damage is too small. ( < 1) Plague Fail\n" );
	}
#endif

	m_IntervalChecker.OnBegin( LocalTime, PLAGUE_DAMAGE_INTERVAL );

	m_fEffectLength = 0.0f;
	m_bEffecting = false;

	m_fOriginalDurationTime  = m_StateBlow.fDurationTime;

}


void CDnPlagueBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#ifndef _GAMESERVER
	if( m_bEffecting )
	{
		m_fEffectLength -= fDelta;
		if( m_fEffectLength < 0.0f )
		{
			m_fEffectLength = 0.0f;
			_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );

			m_bEffecting = false;
		}
	}
#endif

	// Since Die is handled internally, self may die, which can invalidate this state effect on self.
	// Therefore, place it at the very end.
	m_IntervalChecker.Process( LocalTime, fDelta );

}


void CDnPlagueBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_IntervalChecker.OnEnd( LocalTime, fDelta );

#ifndef _GAMESERVER
	_SetDiffuse( 1.0f, 1.0f, 1.0f, 1.0f );
	_DetachGraphicEffect();
#endif

}


#if defined(_GAMESERVER)
void CDnPlagueBlow::OnDuplicate( const STATE_BLOW& StateBlowInfo )
{
	// TODO: How should we handle stacking/overlapping?
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnPlagueBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	std::string strArgument[2];
	strArgument[0] = szOrigValue;
	strArgument[1] = szAddValue;

	std::vector<string> vlOriginTokens, vlNewTokens;
	TokenizeA( strArgument[0], vlOriginTokens, ";" );
	TokenizeA( strArgument[1], vlNewTokens, ";" );

	if( vlOriginTokens.size() == vlNewTokens.size() ) 
	{
		std::string strResult;

		for( DWORD i=0; i<vlOriginTokens.size(); i++ )
		{
			strResult += FormatA( "%f" , ( (float)atof( vlOriginTokens[i].c_str() ) + (float)atof( vlNewTokens[i].c_str() ) ) );
			if( i < vlOriginTokens.size()-1 )
			{
				strResult += ';';
			}
		}

		szNewValue = strResult.c_str();
	}
}

void CDnPlagueBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	std::string strArgument[2];
	strArgument[0] = szOrigValue;
	strArgument[1] = szAddValue;

	std::vector<string> vlOriginTokens, vlNewTokens;
	TokenizeA( strArgument[0], vlOriginTokens, ";" );
	TokenizeA( strArgument[1], vlNewTokens, ";" );

	if( vlOriginTokens.size() == vlNewTokens.size() ) 
	{
		std::string strResult;

		for( DWORD i=0; i<vlOriginTokens.size(); i++ )
		{
			strResult += FormatA( "%f" , ( (float)atof( vlOriginTokens[i].c_str() ) - (float)atof( vlNewTokens[i].c_str() ) ) );
			if( i < vlOriginTokens.size()-1 )
			{
				strResult += ';';
			}
		}

		szNewValue = strResult.c_str();
	}
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW