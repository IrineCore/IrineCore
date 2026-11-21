#include "stdafx.h"
#include "DnPetPlayAniProcess.h"
#include "DnPlayerActor.h"
#include "IDnSkillUsableChecker.h"
#include "DnSkill.h"

#if !defined(_GAMESERVER)
#include "DnPetActor.h"
#endif	// #if !defined(_GAMESERVER)

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPetPlayAniProcess::CDnPetPlayAniProcess( DnActorHandle hActor, const char* pActionName ) : IDnSkillProcessor( hActor ),
m_strActionName( pActionName )
{
	m_iType = PLAY_PET_ANI;
}

CDnPetPlayAniProcess::~CDnPetPlayAniProcess(void)
{
}

void CDnPetPlayAniProcess::CopyFrom( IDnSkillProcessor* pProcessor )
{
	if( NULL == pProcessor )
		return;

	if( GetType() != pProcessor->GetType() )
		return;

	CDnPetPlayAniProcess* pSource = static_cast<CDnPetPlayAniProcess*>( pProcessor );
	m_strActionName = pSource->m_strActionName;
}


void CDnPetPlayAniProcess::OnBegin( LOCAL_TIME LocalTime, float fDelta, DnSkillHandle hParentSkill )
{
	if( m_hHasActor )
	{
#if !defined(_GAMESERVER)
		CDnPlayerActor * pActor = ( CDnPlayerActor * )( m_hHasActor.GetPointer() );
		if( !pActor || !pActor->IsSummonPet() )
		{
			OutputDebug( "CDnPetPlayAniProcess::OnBegin() -> No Actor or no Pet." );
			return;
		}

		CDnPetActor * pPet = pActor->GetMyPetActor();
		if( !pPet )
		{
			OutputDebug( "CDnPetPlayAniProcess::OnBegin() -> Pet Actor not found.." );
			return;
		}

		pPet->SetMovable( false );
		pPet->ResetCustomAction();
		pPet->SetActionQueue( m_strActionName.c_str() );

		CEtActionBase::ActionElementStruct* pActionElement = pPet->GetElement( m_strActionName.c_str() );

		if ( !pActionElement )
			OutputDebug( "CDnPetPlayAniProcess::OnBegin() -> Element matching action name not found." );


		if( pActionElement )
			m_fTimeLength = pActionElement->dwLength / s_fDefaultFps;
#endif	// #if !defined(_GAMESERVER)
	}
}


void CDnPetPlayAniProcess::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_fTimeLength -= fDelta;
	if( m_fTimeLength <= 0.0f )
		m_fTimeLength = 0.0f;
}


bool CDnPetPlayAniProcess::IsFinished( void )
{
	return true;
}


void CDnPetPlayAniProcess::OnEnd( LOCAL_TIME LocalTime, float fDelta )
{
	
}