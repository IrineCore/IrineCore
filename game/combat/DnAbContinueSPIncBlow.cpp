#include "stdafx.h"
#include "DnAbContinueSPIncBlow.h"

const LOCAL_TIME SP_INCREASE_INTERVAL = 2000;


CDnAbContinueSPIncBlow::CDnAbContinueSPIncBlow( DnActorHandle hActor, const char* szValue ) : CDnBlow( hActor ),
																							  m_SPManipulator( m_hActor, GetMySmartPtr() )
{
	m_StateBlow.emBlowIndex = STATE_BLOW::BLOW_012;
	SetValue( szValue );
	//m_nValue = atoi( szValue );
	m_fValue = (float)atof(szValue);
}

CDnAbContinueSPIncBlow::~CDnAbContinueSPIncBlow(void)
{

}

void CDnAbContinueSPIncBlow::OnBegin( LOCAL_TIME LocalTime, float fDelta )
{
	m_SPManipulator.OnBegin( LocalTime, fDelta, GetDurationTime(), 
							 SP_INCREASE_INTERVAL, CDnActorStatIntervalManipulator::SP_PLUS );

	OutputDebug( "CDnAbContinueSPIncBlow::OnBegin, Value:%2.2f (HP : %d ) \n", (int)m_fValue, m_hActor->GetSP());
}

void CDnAbContinueSPIncBlow::Process( LOCAL_TIME LocalTime, float fDelta )
{
	CDnBlow::Process( LocalTime, fDelta );

	m_SPManipulator.Process( LocalTime, fDelta );

	//LOCAL_TIME ElapsedTime = LocalTime - m_StartTime;
	//int iNowSPInc = int(ElapsedTime / SP_INCREASE_INTERVAL);

	//while( m_iSPIncCount < iNowSPInc )
	//{
	//	int iMaxSP = m_hActor->GetMaxSP();
	//	int iNowSP = m_hActor->GetSP();

	//	int iSPResult = iNowSP+m_nValue;
	//	if( iSPResult < iNowSP )
	//	{
	//		//m_hActor->SetSP( iSPResult );
	//	}

	//	m_LastIncreaseTime = LocalTime;

	//	++m_iSPIncCount;
	//}

}

void CDnAbContinueSPIncBlow::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	// If called with LocalTime == 0, force-terminates for deletion, stacking handling, etc.
	if( LocalTime != 0 )
		m_SPManipulator.OnEnd( LocalTime, fDelta );

	OutputDebug( "CDnAbContinueHPIncBlow::OnEnd, (HP : %d ) \n", m_hActor->GetSP());
}
