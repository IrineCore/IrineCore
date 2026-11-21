#include "stdafx.h"
#include "DnPlayAniProcess.h"
#include "DnActor.h"
#include "IDnSkillUsableChecker.h"
#include "DnSkill.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPlayAniProcess::CDnPlayAniProcess( DnActorHandle hActor, const char* pActionName ) : IDnSkillProcessor( hActor )
{
	m_iType = PLAY_ANI;

	std::vector<std::string> tokens;
	TokenizeA( pActionName, tokens, ";" );

	if( tokens.size() > 0 )
		m_strActionName = tokens[0];

#ifdef PRE_ADD_ADDITIOANL_SKILL_ACTION
	if( tokens.size() > 1  )
	{
		for( DWORD i=1; i<tokens.size(); ++i )
		{
			if( strstr( tokens[i].c_str(), m_strActionName.c_str() ) )
				m_vecAdditionalAction.push_back( tokens[i] );
		}
	}
#endif

	if( hActor )
		CheckLandingAction( pActionName, hActor );
}

CDnPlayAniProcess::~CDnPlayAniProcess(void)
{
}

void CDnPlayAniProcess::CopyFrom( IDnSkillProcessor* pProcessor )
{
	if( NULL == pProcessor )
		return;

	if( GetType() != pProcessor->GetType() )
		return;

	CDnPlayAniProcess* pSource = static_cast<CDnPlayAniProcess*>( pProcessor );
	m_strActionName = pSource->m_strActionName;
	m_strChainActionName = pSource->m_strChainActionName;
	m_strLandingActionName = pSource->m_strLandingActionName;

#ifdef PRE_ADD_ADDITIOANL_SKILL_ACTION
	m_vecAdditionalAction.resize( pSource->m_vecAdditionalAction.size() );
	std::copy( pSource->m_vecAdditionalAction.begin(), pSource->m_vecAdditionalAction.end(), m_vecAdditionalAction.begin() );
#endif

}


void CDnPlayAniProcess::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	if( m_hHasActor )
	{
		if( hParentSkill->IsUseCheckerType( IDnSkillUsableChecker::HIT_CHECKER ) )
		{
			CDnDamageBase::SHitParam* pHitParam = m_hHasActor->GetHitParam();
			pHitParam->szActionName = m_strActionName;
		}
		else
		{
			m_hHasActor->SetMovable( false );
			m_hHasActor->ResetCustomAction();
#ifndef _GAMESERVER
			m_hHasActor->ResetMixedAnimation();
#endif //_GAMESERVER
			m_hHasActor->SetActionQueue( m_strActionName.c_str() );
		}

		CEtActionBase::ActionElementStruct* pActionElement = m_hHasActor->GetElement( m_strActionName.c_str() );
		if( pActionElement )
			m_fTimeLength = pActionElement->dwLength / s_fDefaultFps;

		m_strChainActionName.clear();
	}
}


void CDnPlayAniProcess::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_fTimeLength -= fDelta;
	if( m_fTimeLength <= 0.0f )
		m_fTimeLength = 0.0f;
}


bool CDnPlayAniProcess::IsFinished( void )
{
	if( !m_strLandingActionName.empty() && m_strLandingActionName == m_hHasActor->GetCurrentAction() )
		return false;

	bool bFinished = false;

	if( m_strChainActionName.empty() == false )
	{
		if( m_strChainActionName != m_hHasActor->GetCurrentAction() )
			bFinished = true;
	}
	else if( m_strActionName != m_hHasActor->GetCurrentAction() )
	{
		bFinished = true;

#ifdef PRE_ADD_ADDITIOANL_SKILL_ACTION
		if( m_vecAdditionalAction.empty() == false )
		{
			for( DWORD i=0; i<m_vecAdditionalAction.size(); i++ )
			{
				if( m_vecAdditionalAction[i] == m_hHasActor->GetCurrentAction() )
				{
					bFinished = false;
					break;
				}
			}
		}
#endif

	}

	if( bFinished )
	{
		m_fTimeLength = 0.0f;
		return true;
	}

	return false;
}


void CDnPlayAniProcess::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	RestoreActionNameOnce();
}

void CDnPlayAniProcess::OnChainInput( const char* pActionName )
{
	if( m_hHasActor && pActionName )
	{
		CEtActionBase::ActionElementStruct* pActionElement = m_hHasActor->GetElement( pActionName );
	
		if( pActionElement )
		{
			m_fTimeLength = pActionElement->dwLength / s_fDefaultFps;
			m_strChainActionName.assign( pActionName );
		}
		else
			OutputDebug( "CDnPlayAniProcess::OnBegin() -> Element matching action name not found." );
	}
}

void CDnPlayAniProcess::ChangeActionNameOnce( const char* pActionName )
{
	if( pActionName )
	{
		m_strOriginalActionName = m_strActionName;
		m_strActionName = pActionName;
		CheckLandingAction( m_strActionName.c_str(), m_hHasActor );
	}
}

void CDnPlayAniProcess::RestoreActionNameOnce( void )
{
	if( false == m_strOriginalActionName.empty() )
	{
		m_strActionName = m_strOriginalActionName;
	}
}

bool CDnPlayAniProcess::CheckAnimation()
{
#if !defined(_GAMESERVER)
	return m_hHasActor->CheckAnimation( m_strActionName.c_str() );
#else
	return true;
#endif // _GAMESERVER
}

void CDnPlayAniProcess::CheckLandingAction( const char* pActionName, DnActorHandle hActor )
{
	char acBuffer[ MAX_PATH ] = { 0 };
	sprintf_s( acBuffer, "%s_Landing", pActionName );

	if( hActor->IsExistAction( acBuffer ) )
		m_strLandingActionName.assign( acBuffer );
}
