#include "stdafx.h"
#include "DnCoolTimeChargingProcess.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnCoolTimeChargingProcess::CDnCoolTimeChargingProcess( void )
	: m_nMaxChargeCount( 0 )
{
	m_iType = COOLTIME_CHARGING;
}

CDnCoolTimeChargingProcess::CDnCoolTimeChargingProcess( DnActorHandle hActor, const char* szMaxChargeCount )
	: IDnSkillProcessor( hActor )
	, m_nMaxChargeCount( 0 )
{
	m_iType = COOLTIME_CHARGING;
	if( szMaxChargeCount )
		m_nMaxChargeCount = atoi( szMaxChargeCount );
}

CDnCoolTimeChargingProcess::~CDnCoolTimeChargingProcess( void )
{
}

void CDnCoolTimeChargingProcess::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	m_fTimeLength = 0.0f;
}

void CDnCoolTimeChargingProcess::Process( LOCAL_TIME LocalTime, float fDelta )
{
}

void CDnCoolTimeChargingProcess::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
}

void CDnCoolTimeChargingProcess::CopyFrom( IDnSkillProcessor* pProcessor )
{
	if( !pProcessor || GetType() != pProcessor->GetType() )
		return;

	CDnCoolTimeChargingProcess* pSource = static_cast<CDnCoolTimeChargingProcess*>( pProcessor );
	m_nMaxChargeCount = pSource->m_nMaxChargeCount;
}
