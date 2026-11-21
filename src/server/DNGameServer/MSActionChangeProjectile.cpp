#include "stdafx.h"
#include "MSActionChangeProjectile.h"
#include "DnWeapon.h"
#include "DnProjectile.h"
#include "DnSkillBase.h"


int MSActionChangeProjectile::ID = -1;


MSActionChangeProjectile::MSActionChangeProjectile()
{
	m_nSourceWeaponID = -1;
	m_nChangeWeaponID = -1;
	m_pProjectile = NULL;
}

MSActionChangeProjectile::~MSActionChangeProjectile()
{
	SAFE_DELETE( m_pProjectile );
}

void MSActionChangeProjectile::Initialize( int nSourceWeaponID, int nChangeWeaponID )
{
	m_nSourceWeaponID = nSourceWeaponID;
	m_nChangeWeaponID = nChangeWeaponID;

	CDnSkillBase *pSkill = dynamic_cast<CDnSkillBase *>(this);
	if( !pSkill ) return;

	DnActorHandle hActor = pSkill->GetActor();
	if( !hActor ) return;

	m_pProjectile = new CDnProjectile( STATICFUNC() hActor, false, false );
	m_pProjectile->CDnWeapon::Initialize( m_nChangeWeaponID, 0 );
	m_pProjectile->CDnWeapon::CreateObject();
}

void MSActionChangeProjectile::Execute( LOCAL_TIME LocalTime, float fDelta )
{
}

void MSActionChangeProjectile::Process( LOCAL_TIME LocalTime, float fDelta )
{
}

bool MSActionChangeProjectile::IsFinishAction()
{
	return true;
}
