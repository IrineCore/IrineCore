#include "StdAfx.h"
#include "DnPvPKillListDlg.h"
#include "DnWeapon.h"
#include "DnActor.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPvPKillListDlg::CDnPvPKillListDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
{
	memset( m_pStaticLeftName, 0, sizeof(m_pStaticLeftName) );
	memset( m_pStaticRightName, 0, sizeof(m_pStaticRightName) );
	memset( m_pStaticWeapon, 0, sizeof(m_pStaticWeapon) );
	memset( m_pStaticBar, 0, sizeof(m_pStaticBar) );
	memset( m_pStaticGhoul, 0, sizeof(m_pStaticGhoul) );
	memset( m_pStaticOccupation, 0, sizeof(m_pStaticOccupation) );
}

CDnPvPKillListDlg::~CDnPvPKillListDlg()
{
	Clear();
}

void CDnPvPKillListDlg::InitialUpdate()
{
	char szStr[32];
	/*
	CDnWeapon::EquipTypeEnum nWeaponMatchControlID[] = { 
		CDnWeapon::BigBow, CDnWeapon::SmallBow, CDnWeapon::Orb, CDnWeapon::Puppet,
		CDnWeapon::Sword, CDnWeapon::Axe, CDnWeapon::Hammer, CDnWeapon::CrossBow,
		CDnWeapon::Flail, CDnWeapon::Wand, CDnWeapon::Mace, CDnWeapon::Book,
#ifdef PRE_ADD_ACADEMIC
		CDnWeapon::BubbleGun, CDnWeapon::Cannon, 
#endif
#ifdef PRE_ADD_KALI
		CDnWeapon::Chakram , CDnWeapon::Fan,
#endif
	};
	
	int nControlCount = sizeof(nWeaponMatchControlID) / sizeof(int);
	*/
	for( int i=0; i<4; i++ ) {
		sprintf_s( szStr, "ID_STATIC_BAR%d", i );
		m_pStaticBar[i] = GetControl<CEtUIStatic>( szStr );
		m_pStaticBar[i]->Show( false );
		for( int j=0; j<2; j++ ) {
			sprintf_s( szStr, "ID_TEXT_%s_KILLNAME%d", ( j == 0 ) ? "RED" : "BLUE", i * 2 );
			m_pStaticLeftName[i][j] = GetControl<CEtUIStatic>( szStr );
			m_pStaticLeftName[i][j]->Show( false );

			sprintf_s( szStr, "ID_TEXT_%s_KILLNAME%d", ( j == 0 ) ? "RED" : "BLUE", ( i * 2 ) + 1 );
			m_pStaticRightName[i][j] = GetControl<CEtUIStatic>( szStr );
			m_pStaticRightName[i][j]->Show( false );
		}

		
		sprintf_s(szStr, "ID_STATIC_KILLMARK%d", i);
		m_pStaticWeapon[i] = GetControl<CEtUIStatic>(szStr);
		m_pStaticWeapon[i]->Show(false);
/*
		for( int j=0; j<nControlCount; j++ ) {
			sprintf_s( szStr, "ID_STATIC%d_WEAPON%d", i, j );
			m_pStaticWeapon[i][nWeaponMatchControlID[j]] = GetControl<CEtUIStatic>( szStr );
			
			if( m_pStaticWeapon[i][nWeaponMatchControlID[j]] )
				m_pStaticWeapon[i][nWeaponMatchControlID[j]]->Show( false );
		}

		sprintf_s( szStr, "ID_STATIC%d_GHOUL", i );
		m_pStaticGhoul[i] = GetControl<CEtUIStatic>( szStr );
		if( m_pStaticGhoul[i] ) m_pStaticGhoul[i]->Show( false );

		sprintf_s( szStr, "ID_STATIC%d_POINT", i );
		m_pStaticOccupation[i] = GetControl<CEtUIStatic>( szStr );
		if( m_pStaticOccupation[i] ) m_pStaticOccupation[i]->Show( false );
		*/
	}
}

void CDnPvPKillListDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvPKillListDlg.ui" ).c_str(), bShow );
}

void CDnPvPKillListDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPvPKillListDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	CEtUIDialog::Show( bShow );
}

void CDnPvPKillListDlg::Process( float fElapsedTime )
{

	for( DWORD i=0; i<m_pVecInfoStruct.size(); i++ ) {
		InfoStruct *pStruct = m_pVecInfoStruct[i];
		pStruct->fDelta -= fElapsedTime;
		if( pStruct->fDelta <= 0.f ) {
			SAFE_DELETE( pStruct );
			UpdateLine( i, NULL );
			UpdateLine( (int)m_pVecInfoStruct.size()-1, NULL );
			m_pVecInfoStruct.erase( m_pVecInfoStruct.begin() + i );
			i--;
			continue;
		}
		UpdateLine( i, pStruct );
	}

	CEtUIDialog::Process( fElapsedTime );
}

void CDnPvPKillListDlg::UpdateLine( int nLine, InfoStruct *pStruct )
{
	if( pStruct == NULL ) {
		m_pStaticBar[nLine]->Show( false );
		for( int j=0; j<2; j++ ) {
			m_pStaticLeftName[nLine][j]->Show( false );
			m_pStaticRightName[nLine][j]->Show( false );
		}
		
		if( m_pStaticWeapon[nLine] ) m_pStaticWeapon[nLine]->Show( false );
		
	/*	if( m_pStaticGhoul[nLine] ) m_pStaticGhoul[nLine]->Show( false );
		if( m_pStaticOccupation[nLine]  )
			m_pStaticOccupation[nLine]->Show( false ); */
	}
	else {
		m_pStaticBar[nLine]->Show( true );
		m_pStaticLeftName[nLine][pStruct->bKillerTeam]->Show( true );
		m_pStaticLeftName[nLine][!pStruct->bKillerTeam]->Show( false );
		m_pStaticLeftName[nLine][pStruct->bKillerTeam]->SetText( pStruct->wszKillerName );
		m_pStaticRightName[nLine][pStruct->bKilledTeam]->Show( true );
		m_pStaticRightName[nLine][!pStruct->bKilledTeam]->Show( false );
		m_pStaticRightName[nLine][pStruct->bKilledTeam]->SetText( pStruct->wszKilledName );
		
		if( m_pStaticWeapon[nLine] && !pStruct->bIsGhoul && !pStruct->bIsOccupation	)
			m_pStaticWeapon[nLine]->Show( true );

		if( m_pStaticGhoul[nLine] && pStruct->bIsGhoul )
			m_pStaticGhoul[nLine]->Show( true );

		if( m_pStaticOccupation[nLine] && pStruct->bIsOccupation )
			m_pStaticOccupation[nLine]->Show( true );
	}
}

void CDnPvPKillListDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );
}

void CDnPvPKillListDlg::AddInfo( DnActorHandle hKiller, DnActorHandle hKilled, CDnWeapon::EquipTypeEnum EquipType )
{
	if( !CDnActor::s_hLocalActor || !hKilled->IsPlayerActor()) return;
	if( m_pVecInfoStruct.size() == 4 ) {
		SAFE_DELETE( m_pVecInfoStruct[0] );
		m_pVecInfoStruct.erase( m_pVecInfoStruct.begin() );
	}
	InfoStruct *pStruct = new InfoStruct;
	pStruct->wszKillerName = hKiller->GetName();
	pStruct->wszKilledName = hKilled->GetName();

	pStruct->bKillerTeam = ( CDnActor::s_hLocalActor->GetTeam() == hKiller->GetTeam() ) ? true : false;
	pStruct->bKilledTeam = ( CDnActor::s_hLocalActor->GetTeam() == hKilled->GetTeam() ) ? true : false;


	if(CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
	{
		if(hKiller->GetTeam() == PvPCommon::Team::A)
		{
			pStruct->bKillerTeam = true;
			pStruct->bKilledTeam = false;
		}
		else if(hKiller->GetTeam() == PvPCommon::Team::B)
		{
			pStruct->bKillerTeam = false;
			pStruct->bKilledTeam = true;
		}
	}

	if( hKiller && hKiller->IsPlayerActor() )
	{
		CDnPlayerActor *pPlayer = dynamic_cast<CDnPlayerActor*>(hKiller.GetPointer());
		if(pPlayer && pPlayer->IsTransformMode()) pStruct->bIsGhoul = true;
		else pStruct->bIsGhoul = false;
	}

	pStruct->EquipType = EquipType;
	pStruct->fDelta = 3.f;
	m_pVecInfoStruct.push_back( pStruct );
	UpdateLine( (int)m_pVecInfoStruct.size() - 1, pStruct );
	return;
}

void CDnPvPKillListDlg::AddOccupationInfo( DnActorHandle hActor, std::wstring & wszFlagName )
{
	if( !CDnActor::s_hLocalActor || !hActor->IsPlayerActor()) return;
	if( m_pVecInfoStruct.size() == 4 ) {
		SAFE_DELETE( m_pVecInfoStruct[0] );
		m_pVecInfoStruct.erase( m_pVecInfoStruct.begin() );
	}
	InfoStruct *pStruct = new InfoStruct;
	pStruct->wszKillerName = hActor->GetName();
	pStruct->wszKilledName = wszFlagName;

	pStruct->bKillerTeam = ( CDnActor::s_hLocalActor->GetTeam() == hActor->GetTeam() ) ? true : false;
	pStruct->bKilledTeam = pStruct->bKillerTeam;

	if(CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::Observer)
	{
		if(hActor->GetTeam() == PvPCommon::Team::A)
		{
			pStruct->bKillerTeam = true;
			pStruct->bKilledTeam = true;
		}
		else if(hActor->GetTeam() == PvPCommon::Team::B)
		{
			pStruct->bKillerTeam = false;
			pStruct->bKilledTeam = false;
		}
	}

	pStruct->bIsOccupation = true;
	pStruct->EquipType = CDnWeapon::EquipTypeEnum::Sword;
	pStruct->fDelta = 3.f;
	m_pVecInfoStruct.push_back( pStruct );
	UpdateLine( (int)m_pVecInfoStruct.size() - 1, pStruct );
}

void CDnPvPKillListDlg::Clear()
{
	SAFE_DELETE_PVEC( m_pVecInfoStruct );
}