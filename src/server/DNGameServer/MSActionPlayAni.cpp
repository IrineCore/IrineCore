#include "stdafx.h"
#include "MSActionPlayAni.h"
#include "DnActor.h"
#include "DnSkillBase.h"
#include "DnDamageBase.h"

int MSActionPlayAni::ID = -1;

MSActionPlayAni::MSActionPlayAni()
{
	m_fTimeDelta = 0.f;
}

MSActionPlayAni::~MSActionPlayAni()
{
}

void MSActionPlayAni::Execute( LOCAL_TIME LocalTime, float fDelta )
{
	CDnSkillBase *pSkill = dynamic_cast<CDnSkillBase *>(this);
	if( !pSkill ) return;

	DnActorHandle hActor = pSkill->GetActor();
	
	if( hActor ) {
		MSUsingTypeBase *pUsingType = dynamic_cast<MSUsingTypeBase *>(pSkill);
		if( pUsingType && pUsingType->GetMethodID() == MSPassiveHitCheckDamageAction::ID ) {
			CDnDamageBase::SHitParam *pHitParam = hActor->GetHitParam();
			pHitParam->szActionName = m_szActionName;
		}
		else {
			hActor->SetMovable( false );
			hActor->SetActionQueue( m_szActionName.c_str() );
		}

		CEtActionBase::ActionElementStruct *pStruct = hActor->GetElement( m_szActionName.c_str() );
		if( pStruct == NULL ) return;
		m_fTimeDelta = (float)pStruct->dwLength / s_fDefaultFps;
	}
}

void MSActionPlayAni::Process( LOCAL_TIME LocalTime, float fDelta )
{
	m_fTimeDelta -= fDelta;
	if( m_fTimeDelta <= 0.f ) m_fTimeDelta = 0.f;

	CDnSkillBase *pSkill = dynamic_cast<CDnSkillBase *>(this);
	if( !pSkill ) return;
	DnActorHandle hActor = pSkill->GetActor();
}

void MSActionPlayAni::Initialize( const char *szActionName )
{
	m_szActionName = szActionName;
}


bool MSActionPlayAni::IsFinishAction()
{
	if( m_fTimeDelta == 0.f ) return true;
	return false;
}
