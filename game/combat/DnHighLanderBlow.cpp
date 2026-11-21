#include "stdafx.h"
#include "DnHighLanderBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

// dnactor.cpp
#ifdef _GAMESERVER
extern int GetGaussianRandom( int nMin, int nMax, CMultiRoom *pRoom );
#endif

CDnHighlanderBlow::CDnHighlanderBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_143;

	// Use only 2 time values; takes the duration of the HP 1 state as an argument.
	SetValue( szValue );
	m_fValue = (float)atof( szValue );
	
	m_iState = NORMAL;

	AddCallBackType( SB_ONCALCDAMAGE );
}

CDnHighlanderBlow::~CDnHighlanderBlow(void)
{

}

void CDnHighlanderBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_iState = NORMAL;

	OutputDebug( "CDnHighlanderBlow::OnBegin Value:%2.2f\n", m_fValue );
}


void CDnHighlanderBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnHighlanderBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{

	OutputDebug( "CDnHighlanderBlow::OnEnd Value:%2.2f\n", m_fValue );
}

#ifdef _GAMESERVER
float CDnHighlanderBlow::CalcDamage( float fOriginalDamage )
{
	float fResult = 0.0f;

	switch( m_iState )
	{
		case NORMAL:
			{
				int iNowHP = (int)m_hActor->GetHP();

				// If taking this hit would drop HP to 0 or below and kill, adjust the damage so 1 HP remains.
				// And switch to Highlander state.
				if( (iNowHP - (int)fOriginalDamage) <= 0 )
				{
					//fOriginalDamage = float(iNowHP - 1);
					fResult = float(iNowHP - 1)-fOriginalDamage;

					// Change state to Highlander, and state effect duration 
					// Set duration to Highlander state time.
					m_iState = HIGHLANDER;
					m_StateBlow.fDurationTime = m_fValue;

					// Also update this state effect's duration on client to the newly updated value.
					m_hActor->CmdModifyStateEffect( GetBlowID(), m_StateBlow );
				}

			}
			break;

		case HIGHLANDER:
			{
				// In Highlander state, cannot die until the set duration expires and the state effect ends.
				// If HP became 1 or more via heal or potion, allow damage only down to 1 HP remaining.
				int iNowHP = (int)m_hActor->GetHP();
				if( 1 < iNowHP )
				{
					if( (iNowHP - (int)fOriginalDamage) <= 0 )
					{
						//fOriginalDamage = float(iNowHP - 1);
						fResult = float(iNowHP - 1)-fOriginalDamage;
					}
				}
				else
				{
					fResult = -fOriginalDamage;
				}
			}
			break;
	}

	return fResult;
}

float CDnHighlanderBlow::OnCalcDamage( float fOriginalDamage, CDnDamageBase::SHitParam& HitParam )
{
	// #25781 If hit signal ignores CanHit, Highlander is also ignored.
	if( HitParam.bIgnoreCanHit )
		return 0.0f;

	return CalcDamage( fOriginalDamage );
}
#endif

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnHighlanderBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnHighlanderBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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