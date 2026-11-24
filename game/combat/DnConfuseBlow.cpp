#include "stdafx.h"
#include "DnConfuseBlow.h"
#include "DnStateBlow.h"
#if defined(_CLIENT)
#include "InputDevice.h"
#include "DnLocalPlayerActor.h"
#endif // _CLIENT


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnConfuseBlow::CDnConfuseBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_225;
	SetValue( szValue );

	m_OrigInverseInputMode = 0;

	//Set szValue
	m_fValue = (float)atof(szValue);

	//1: Invert keyboard & mouse input
	//2: Invert keyboard input
	//3 -> Invert mouse input
}

CDnConfuseBlow::~CDnConfuseBlow(void)
{

}


void CDnConfuseBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnBegin(LocalTime, fDelta);

#if defined(_CLIENT)
	if (m_hActor && m_hActor->IsPlayerActor())
	{
		CDnLocalPlayerActor* pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer());
		if (pLocalPlayerActor)
		{
			CInputDevice::ms_InverseModeCount++;

			//Save the original InverseMode value first...
			m_OrigInverseInputMode = CInputDevice::GetInverseMode();
			CInputDevice::SetInverseMode((int)m_fValue);
		}
	}
#endif // _CLIENT

	OutputDebug( "%s\n", __FUNCTION__ );
}


void CDnConfuseBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );
}


void CDnConfuseBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::OnEnd(LocalTime, fDelta);

#if defined(_CLIENT)
	if (m_hActor && m_hActor->IsPlayerActor())
	{
		CDnLocalPlayerActor* pLocalPlayerActor = dynamic_cast<CDnLocalPlayerActor*>(m_hActor.GetPointer());
		if (pLocalPlayerActor)
		{
			//Get current mode value.
			int nCurrentMode = CInputDevice::GetInverseMode();

			//If the Mode value at save time differs from the current Mode value, another state effect already reverted it to the previous value.
			if (m_OrigInverseInputMode != 0 && nCurrentMode != m_OrigInverseInputMode)
				m_OrigInverseInputMode = nCurrentMode;

			//Restore to the original InverseMode value.
			CInputDevice::SetInverseMode(m_OrigInverseInputMode);

			CInputDevice::ms_InverseModeCount--;
		}
	}
#endif // _CLIENT

	OutputDebug( "%s\n", __FUNCTION__ );
}

#if defined(PRE_ADD_PREFIX_SYSTE_RENEW)
void CDnConfuseBlow::AddStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };


	szNewValue = szOrigValue;
}

void CDnConfuseBlow::RemoveStateEffectValue(const char* szOrigValue, const char* szAddValue, std::string& szNewValue)
{
	char szBuff[128] = {0, };

	szNewValue = szOrigValue;
}
#endif // PRE_ADD_PREFIX_SYSTE_RENEW