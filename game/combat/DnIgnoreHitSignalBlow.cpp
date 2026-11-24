#include "stdafx.h"
#include "DnIgnoreHitSignalBlow.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnIgnoreHitSignalBlow::CDnIgnoreHitSignalBlow(DnActorHandle hActor, const char *szValue)
	: CDnBlow(hActor)
#if defined(_CLIENT)
	,m_IntervalChecker( hActor, GetMySmartPtr() )
#endif // _CLIENT
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_228;
	SetValue(szValue);
	
	m_fValue = (float)atof(szValue);

#if defined(_CLIENT)
	//Do not display effect by default..
	UseTableDefinedGraphicEffect( false );
	m_bGraphicEffectShow = false;
	m_bTrigger = false;
#endif // _CLIENT
}

CDnIgnoreHitSignalBlow::~CDnIgnoreHitSignalBlow(void)
{
}

void CDnIgnoreHitSignalBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

#if defined(_CLIENT)
	if (m_bTrigger)
	{
		if (m_bGraphicEffectShow)
		{
			_AttachGraphicEffect();
			if( m_hEtcObjectEffect )
			{
				CEtActionBase::ActionElementStruct* pStruct = m_hEtcObjectEffect->GetElement( "Idle" );
				if( pStruct )
					m_IntervalChecker.OnBegin( LocalTime, pStruct->dwLength + 1000);
			}
		}
		else
		{
			if (m_hEtcObjectEffect)
			{
				_DetachGraphicEffect();
				m_IntervalChecker.OnEnd(LocalTime, fDelta);
			}
		}

		m_bTrigger = false;
	}

	m_IntervalChecker.Process( LocalTime, fDelta );
#endif // _CLIENT
}

void CDnIgnoreHitSignalBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

	if( m_hActor->IsDie() )
	{
		SetState( STATE_BLOW::STATE_END );
		return;
	}
}

void CDnIgnoreHitSignalBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);
	OutputDebug( "%s\n", __FUNCTION__ );

#if defined(_CLIENT)
	_DetachGraphicEffect();
#endif // _CLIENT
}

bool CDnIgnoreHitSignalBlow::CanBegin( void )
{
	bool bCanBegin = rand() % 10000 <= (m_fValue * 10000.0f);

	return bCanBegin;
}

#if defined(_CLIENT)
bool CDnIgnoreHitSignalBlow::OnCustomIntervalProcess( void )
{
	if (m_hEtcObjectEffect)
	{
		_DetachGraphicEffect();
		m_IntervalChecker.OnEnd(0, 0.0f);
	}

	return true;
}

void CDnIgnoreHitSignalBlow::ShowGraphicEffect(bool bShow)
{
	m_bGraphicEffectShow = bShow;
	m_bTrigger = true;
}
#endif // _CLIENT

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnIgnoreHitSignalBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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

void CDnIgnoreHitSignalBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
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