#include "stdafx.h"
#include "DnAssaultProcessor.h"
#include "DnActor.h"
#include "DnSkill.h"

#if !defined( USE_BOOST_MEMPOOL )
#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif
#endif

CDnAssaultProcessor::CDnAssaultProcessor( void )
	: m_nHitInterval( 0 )
	, m_nHitLimitCount( 0 )
	, m_nExtraParam( 0 )
	, m_nTime( 0 )
	, m_fElapsedTime( 0.0f )
	, m_fMoveSpeed( 0.0f )
	, m_vOffset( 0.0f, 0.0f, 0.0f )
{
	m_iType = ASSAULT;
	memset( &m_HitStruct, 0, sizeof( m_HitStruct ) );
	m_HitStruct.vOffset = &m_vOffset;
}

CDnAssaultProcessor::CDnAssaultProcessor( DnActorHandle hActor,
										  const char* pStartActionName,
										  const char* pLoopActionName,
										  const char* pEndActionName,
										  int iTime,
										  float fMoveSpeed,
										  const char* pHitActionName,
										  const char* pHitParam,
										  const char* pHitAreaParam )
	: IDnSkillProcessor( hActor )
	, m_nHitInterval( 0 )
	, m_nHitLimitCount( 0 )
	, m_nExtraParam( 0 )
	, m_nTime( iTime )
	, m_fElapsedTime( 0.0f )
	, m_fMoveSpeed( fMoveSpeed )
	, m_vOffset( 0.0f, 0.0f, 0.0f )
	, m_strStartActionName( pStartActionName ? pStartActionName : "" )
	, m_strLoopActionName( pLoopActionName ? pLoopActionName : "" )
	, m_strHitActionName( pHitActionName ? pHitActionName : "" )
	, m_strEndActionName( pEndActionName ? pEndActionName : "" )
{
	m_iType = ASSAULT;

	std::vector<std::string> vlTokens;
	if( pHitParam && pHitParam[0] != '\0' )
	{
		TokenizeA( std::string( pHitParam ), vlTokens, ";" );
		if( vlTokens.size() >= 2 )
		{
			m_nHitInterval = atoi( vlTokens[0].c_str() );
			m_nHitLimitCount = atoi( vlTokens[1].c_str() );
		}
		if( vlTokens.size() >= 3 )
		{
			m_nExtraParam = atoi( vlTokens[2].c_str() );
		}
	}

	memset( &m_HitStruct, 0, sizeof( m_HitStruct ) );
	m_HitStruct.vOffset = &m_vOffset;

	vlTokens.clear();
	if( pHitAreaParam && pHitAreaParam[0] != '\0' )
	{
		TokenizeA( std::string( pHitAreaParam ), vlTokens, ";" );
		if( vlTokens.size() >= 2 )
		{
			m_HitStruct.fDistanceMax = (float)atof( vlTokens[0].c_str() );
			m_HitStruct.fDistanceMin = (float)atof( vlTokens[1].c_str() );
		}
		if( vlTokens.size() >= 4 )
		{
			m_HitStruct.fHeightMax = (float)atof( vlTokens[2].c_str() );
			m_HitStruct.fHeightMin = (float)atof( vlTokens[3].c_str() );
		}
		if( vlTokens.size() >= 6 )
		{
			m_HitStruct.fCenterAngle = (float)atof( vlTokens[4].c_str() );
			m_HitStruct.fAngle = (float)atof( vlTokens[5].c_str() );
		}
		if( vlTokens.size() >= 9 )
		{
			m_vOffset.x = (float)atof( vlTokens[6].c_str() );
			m_vOffset.y = (float)atof( vlTokens[7].c_str() );
			m_vOffset.z = (float)atof( vlTokens[8].c_str() );
		}
	}
}

CDnAssaultProcessor::~CDnAssaultProcessor( void )
{
}

bool CDnAssaultProcessor::IsActionMatch( const std::string& strTarget, const char* szAction ) const
{
	if( !szAction )
		return false;
	return (strTarget == szAction);
}

void CDnAssaultProcessor::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	if( !m_hHasActor )
		return;

	m_fElapsedTime = 0.0f;
	m_fTimeLength = (float)m_nTime / 1000.0f;
	m_hHasActor->SetMovable( false );
	m_hHasActor->SetActionQueue( m_strStartActionName.c_str() );
}

void CDnAssaultProcessor::Process( LOCAL_TIME LocalTime, float fDelta )
{
	if( !m_hHasActor )
		return;

	const char* szCurrentAction = m_hHasActor->GetCurrentAction();
	if( IsActionMatch( m_strLoopActionName, szCurrentAction ) )
	{
		m_fElapsedTime += fDelta;
		if( 0 == m_hHasActor->GetCantXZMoveSEReferenceCount() )
		{
			m_hHasActor->MoveZ( m_fMoveSpeed * fDelta );
		}

		if( (float)m_nTime < m_fElapsedTime * 1000.0f )
		{
			if( !m_hHasActor->IsDie() )
			{
				m_hHasActor->SetActionQueue( m_strEndActionName.c_str(), 0, 3.0f, 0.0f, true, false );
			}
		}
	}
}

void CDnAssaultProcessor::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	m_fElapsedTime = 0.0f;
}

bool CDnAssaultProcessor::IsFinished( void )
{
	if( !m_hHasActor )
		return true;

	const char* szCurrentAction = m_hHasActor->GetCurrentAction();
	if( IsActionMatch( m_strStartActionName, szCurrentAction ) )
		return false;
	if( IsActionMatch( m_strLoopActionName, szCurrentAction ) )
		return false;
	if( IsActionMatch( m_strHitActionName, szCurrentAction ) )
		return false;
	if( IsActionMatch( m_strEndActionName, szCurrentAction ) )
		return false;

	return true;
}

void CDnAssaultProcessor::CopyFrom( IDnSkillProcessor* pProcessor )
{
	if( !pProcessor || GetType() != pProcessor->GetType() )
		return;

	CDnAssaultProcessor* pSource = static_cast<CDnAssaultProcessor*>( pProcessor );
	m_strStartActionName = pSource->m_strStartActionName;
	m_strLoopActionName = pSource->m_strLoopActionName;
	m_strHitActionName = pSource->m_strHitActionName;
	m_strEndActionName = pSource->m_strEndActionName;
	m_nHitInterval = pSource->m_nHitInterval;
	m_nHitLimitCount = pSource->m_nHitLimitCount;
	m_nExtraParam = pSource->m_nExtraParam;
	m_nTime = pSource->m_nTime;
	m_fMoveSpeed = pSource->m_fMoveSpeed;
	memcpy( &m_HitStruct, &pSource->m_HitStruct, sizeof( HitStruct ) );
	m_vOffset = pSource->m_vOffset;
	m_HitStruct.vOffset = &m_vOffset;
}
