#include "stdafx.h"
#include "DnLamiaMonsterActor.h"
#include "DnWeapon.h"


CDnLamiaMonsterActor::CDnLamiaMonsterActor( CMultiRoom *pRoom, int nClassID )
: CDnDefenseMonsterActor( pRoom, nClassID )
{
}

CDnLamiaMonsterActor::~CDnLamiaMonsterActor()
{
	for( int i=0; i<2; i++ ) 
		SAFE_RELEASE_SPTR( m_hSubWeapon[i] );
}

void CDnLamiaMonsterActor::AttachWeapon( DnWeaponHandle hWeapon, int nEquipIndex, bool bDelete )
{
	CDnDefenseMonsterActor::AttachWeapon( hWeapon, nEquipIndex, bDelete );

	m_hSubWeapon[nEquipIndex] = CDnWeapon::CreateWeapon( GetRoom(), hWeapon->GetClassID(), hWeapon->GetSeed(), 0, 0, 0, false, false );
	m_hSubWeapon[nEquipIndex]->CreateObject();
	m_hSubWeapon[nEquipIndex]->LinkWeapon( GetMySmartPtr(), nEquipIndex );
}

void CDnLamiaMonsterActor::DetachWeapon( int nEquipIndex )
{
	CDnDefenseMonsterActor::DetachWeapon( nEquipIndex );

	if( m_hSubWeapon[nEquipIndex] ) {
		m_hSubWeapon[nEquipIndex]->FreeObject();
		m_hSubWeapon[nEquipIndex]->UnlinkWeapon();
	}
	SAFE_RELEASE_SPTR( m_hSubWeapon[nEquipIndex] );
}


void CDnLamiaMonsterActor::DefenseWeapon( bool bCrash )
{
	CDnDefenseMonsterActor::DefenseWeapon( bCrash );

	if( bCrash ) {
		if( m_hSubWeapon[1]->IsExistAction( "Crash" ) )
			m_hSubWeapon[1]->SetActionQueue( "Crash" );
		else m_hSubWeapon[1]->SetDestroy();

		m_hSubWeapon[1]->UnlinkWeapon();
		m_hSubWeapon[1]->SetProcess( true );
	}
	else {
		if( m_hSubWeapon[1]->IsExistAction( "Defense" ) )
			m_hSubWeapon[1]->SetActionQueue( "Defense" );
	}
}
