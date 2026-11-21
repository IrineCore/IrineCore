#include "StdAfx.h"
#include "DnEnemyGaugeDlg.h"
#include "DnInterface.h"
#include "GameOption.h"
#include "DnMonsterActor.h"
#include "DnTableDB.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnEnemyGaugeDlg::CDnEnemyGaugeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUIDialog( UI_TYPE_BOTTOM, pParentDialog, nID, pCallback  )
{
	m_pHPBar = NULL;
	m_pSuperArmorBar = NULL;
	
	memset( m_pName, 0, sizeof(m_pName) );
	m_fDelayTime = 0.0f;
	m_vDamageOld = EtVector3(0.0f, 0.0f, 0.0f);
	m_pBalloon = NULL;
	memset( m_pNameIcon, 0, sizeof(m_pNameIcon) );
}

CDnEnemyGaugeDlg::~CDnEnemyGaugeDlg(void)
{
}

void CDnEnemyGaugeDlg::Initialize( bool bShow )
{
	if( !m_pHPBar )
	{
		CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "EnemyGaugeDlg.ui" ).c_str(), bShow );
	}
}

void CDnEnemyGaugeDlg::InitialUpdate()
{
	m_pHPBar = GetControl< CEtUIProgressBar >( "ID_HP_GAUGE" );
//#define NORMALMONSTER_HP_LAYER_hayannal2009
#ifdef NORMALMONSTER_HP_LAYER_hayannal2009
	m_pHPBar->UseLayer( true );
#endif

	m_pSuperArmorBar = GetControl< CEtUIProgressBar >( "ID_SUPER_GAUGE" );
	m_pName[0] = GetControl< CEtUIStatic >( "ID_PLAYER_NAME" );
	m_pName[1] = GetControl< CEtUIStatic >( "ID_PLAYER_NAME2" );
	m_pName[2] = GetControl< CEtUIStatic >( "ID_PLAYER_NAME3" );
	m_pBalloon = GetControl<CEtUIStatic>("ID_BALLOON");
	m_pBalloon->Show( false );
	m_pBalloon->SetTextureColor( 0 );

	m_pNameIcon[0][0] = GetControl< CEtUIStatic >( "ID_ICON_LEFE" );
	m_pNameIcon[0][1] = GetControl< CEtUIStatic >( "ID_ICON_RIGHT" );
	m_pNameIcon[1][0] = GetControl< CEtUIStatic >( "ID_ICON_NEST_LEFE" );
	m_pNameIcon[1][1] = GetControl< CEtUIStatic >( "ID_ICON_NEST_RIGHT" );

	for( int i=0; i<2; i++ ) {
		for( int j=0; j<2; j++ ) m_pNameIcon[i][j]->Show( false );
	}

	m_pSuperArmorBar->SetProgress( 0.0f );

	MoveToHead( m_pBalloon );

	SetPosition( -100.0f, -100.0f );
}

void CDnEnemyGaugeDlg::Process( float fElapsedTime )
{
	if( m_hActor && !m_hActor->IsShow() ) {
		m_fDelayTime = 0.f;
		return;
	}

	for( int i=0; i<3; i++ )
		m_pName[i]->Show( false );

	int nIconIndex = -1;
	if( m_hActor && m_hActor->IsMonsterActor() ) {
		CDnMonsterActor *pMonster = static_cast<CDnMonsterActor *>(m_hActor.GetPointer());
		switch( pMonster->GetGrade() ) {
			case CDnMonsterState::Minion:
			case CDnMonsterState::Normal:
				m_pName[0]->Show( CGameOption::GetInstance().bMonsterName );
				break;
			case CDnMonsterState::Champion:
			case CDnMonsterState::Elite:
				m_pName[1]->Show( CGameOption::GetInstance().bMonsterName );
				break;
			case CDnMonsterState::Named:
				nIconIndex = 2;	// To hide both the HP gauge and the icon next to the name.
				m_pName[1]->Show( CGameOption::GetInstance().bMonsterName );
				break;
			case CDnMonsterState::Boss: 
			case CDnMonsterState::BossHP4:
				nIconIndex = 0; 
				m_pName[2]->Show( CGameOption::GetInstance().bMonsterName );
				break;
			case CDnMonsterState::NestBoss: 
			case CDnMonsterState::NestBoss8:
				nIconIndex = 1; 
				m_pName[2]->Show( CGameOption::GetInstance().bMonsterName );
				break;
		}
	}
	m_pHPBar->Show( ( ( nIconIndex == -1 ) && CGameOption::GetInstance().bMonsterHPBar ) ? true : false );
	m_pSuperArmorBar->Show( ( ( nIconIndex == -1 ) && CGameOption::GetInstance().bMonsterHPBar ) ? true : false );

	for( int i=0; i<2; i++ ) {
		for( int j=0; j<2; j++ ) {
			m_pNameIcon[i][j]->Show( false );
		}
	}

	if( nIconIndex == 0 || nIconIndex == 1 ) {
		for( int i=0; i<2; i++ ) m_pNameIcon[nIconIndex][i]->Show( CGameOption::GetInstance().bMonsterName );
	}

	CEtUIDialog::Process( fElapsedTime );

	if( !m_hActor ) {
		m_fDelayTime = 0.f;
		return;
	}

	switch( ((CDnMonsterActor*)m_hActor.GetPointer())->GetGrade() ) {
		case CDnMonsterState::Champion:
		case CDnMonsterState::Elite:
		case CDnMonsterState::Named:
			if( ((CDnMonsterActor*)m_hActor.GetPointer())->GetAggroTarget() ) {
				m_fDelayTime = FLT_MAX;
			}
			else {
				if( m_fDelayTime > 2.f ) m_fDelayTime = 2.f;
			}
			break;
		case CDnMonsterState::Boss:
		case CDnMonsterState::BossHP4:
		case CDnMonsterState::NestBoss:
		case CDnMonsterState::NestBoss8:
			{
				if( ((CDnMonsterActor*)m_hActor.GetPointer())->GetAggroTarget() ) {
					m_fDelayTime = FLT_MAX;
				}
				else {
					if( m_fDelayTime > 2.f ) m_fDelayTime = 2.f;
				}

				SUICoord FontCoord;
				SUICoord CoordIcon;
				SUICoord CoordText;
				CalcTextRect( m_pName[2]->GetText(), m_pName[2]->GetElement(0), FontCoord, -1 );

				CoordText = m_pName[2]->GetUICoord();
				float fOffsetX = CoordText.fX + ( CoordText.fWidth / 2.f );

				int nIconIndex = ( ((CDnMonsterActor*)m_hActor.GetPointer())->GetGrade() < CDnMonsterState::NestBoss ) ? 0 : 1;
				CoordIcon = m_pNameIcon[nIconIndex][0]->GetUICoord();
				CoordIcon.fX = fOffsetX - ( FontCoord.fWidth / 2.f ) - CoordIcon.fWidth;
				m_pNameIcon[nIconIndex][0]->SetUICoord( CoordIcon );

				CoordIcon = m_pNameIcon[nIconIndex][1]->GetUICoord();
				CoordIcon.fX = fOffsetX + ( FontCoord.fWidth / 2.f );
				m_pNameIcon[nIconIndex][1]->SetUICoord( CoordIcon );
			}

			break;
	}
	if( ((CDnMonsterActor*)m_hActor.GetPointer())->IsAlwaysShowExposureInfo() ) {
		m_fDelayTime = FLT_MAX;
	}
	if( m_fDelayTime > 0.0f )
	{
		m_fDelayTime -= fElapsedTime;

		UpdateHP();
		//UpdateSP();
		UpdateGaugePos();
	}
}

void CDnEnemyGaugeDlg::Render( float fElapsedTime )
{
	if( GetInterface().IsOpenBlind() )
		return;

	CEtUIDialog::Render( fElapsedTime );
}

void CDnEnemyGaugeDlg::Show( bool bShow )
{ 	
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_hActor.Identity();
		m_fDelayTime = 0.0f;
		if( m_pBalloon )
		{		
			m_pBalloon->Show( false );
			m_pBalloon->SetTextureColor( 0 );
		}
	}
	else
	{
		m_fDelayTime = 2.0f;
	}

	CEtUIDialog::Show( bShow );
}

void CDnEnemyGaugeDlg::UpdateHP()
{
	float fHP(0.0f);

	if( m_hActor->GetMaxHP() > 0 )
	{
		fHP = m_hActor->GetHP() / ( float )m_hActor->GetMaxHP();
	}

#ifdef NORMALMONSTER_HP_LAYER_hayannal2009
	int nSeparateCount = 2;
	int nStartCount = 0;
	int nProgressIndex = EtMin( nSeparateCount-1, (int)(fHP * nSeparateCount));
	m_pHPBar->SetProgressIndex( nProgressIndex + nStartCount );
	m_pHPBar->SetProgress( 100.0f * (fHP * nSeparateCount - nProgressIndex) );
#else
	m_pHPBar->SetProgress( fHP * 100.0f );
#endif
	m_pSuperArmorBar->SetProgress( m_hActor->GetMaxSuperArmor() <= 50 ? 0.0f : (100.0f * m_hActor->GetCurrentSuperArmor() / m_hActor->GetMaxSuperArmor()) );

	if( m_hActor->GetMaxSuperArmor() <= 50.0f )
		m_pSuperArmorBar->SetImmediateChange();

	int nMaxSuperArmor = m_hActor->GetMaxSuperArmor();
	if( nMaxSuperArmor <= 50 ) 
		m_pSuperArmorBar->SetProgressIndex( 0 );
	else if( nMaxSuperArmor <= 200 ) 
		m_pSuperArmorBar->SetProgressIndex( 1 );
	else if( nMaxSuperArmor <= 600 ) 
		m_pSuperArmorBar->SetProgressIndex( 2 );
	else if( nMaxSuperArmor <= 1800 ) 
		m_pSuperArmorBar->SetProgressIndex( 3 );
	else
		m_pSuperArmorBar->SetProgressIndex( 4 );
}

//void CDnEnemyGaugeDlg::UpdateSP()
//{
//	float fSP(0.0f);
//
//	if( m_hActor->GetMaxSP() > 0 )
//	{
//		fSP = m_hActor->GetSP() / ( float )m_hActor->GetMaxSP();
//	}
//
//	m_pSPBar->SetProgress( fSP * 100.0f );
//}

/*
void CDnEnemyGaugeDlg::SetName( int nLevel, LPCWSTR pwszName )
{
//	wchar_t wszTemp[80] = {0};
	//swprintf_s( wszTemp, 80, L"Lv.%d %s", nLevel, pwszName );
//	swprintf_s( wszTemp, 80, L"%s", pwszName );
//	m_pName->SetTextColor( ( dwColor == -1 ) ? m_dwDefaultColor : dwColor );
	m_pName->SetText( pwszName );
}
*/

void CDnEnemyGaugeDlg::SetEnemy( DnActorHandle hActor ) 
{ 
	m_hActor = hActor;
	for( int i=0; i<3; i++ ) m_pName[i]->SetText( L"" );

	int nNameIndex[] = { 0, 0, 1, 1, 1, 2, 2, 2, 2, };
	DWORD dwColorArray[] = { 
		EtInterface::textcolor::MINIONMONSTER, 
		EtInterface::textcolor::MONSTER, 
		EtInterface::textcolor::CHAMPIONMONSTER, 
		EtInterface::textcolor::ELITEMONSTER, 
		EtInterface::textcolor::NAMEDMONSTER, 
		EtInterface::textcolor::BOSSMONSTER, 
		EtInterface::textcolor::BOSSMONSTER,
		EtInterface::textcolor::NESTBOSSMONSTER, 
		EtInterface::textcolor::NESTBOSSMONSTER, 
	};
	DWORD dwShadowArray[] = { 
		EtInterface::textcolor::MINIONMONSTER_S, 
		EtInterface::textcolor::MONSTER_S, 
		EtInterface::textcolor::CHAMPIONMONSTER_S, 
		EtInterface::textcolor::ELITEMONSTER_S, 
		EtInterface::textcolor::NAMEDMONSTER_S, 
		EtInterface::textcolor::BOSSMONSTER_S, 
		EtInterface::textcolor::BOSSMONSTER_S,
		EtInterface::textcolor::NESTBOSSMONSTER_S, 
		EtInterface::textcolor::NESTBOSSMONSTER_S, 
	};

	if( !m_hActor->IsMonsterActor() ) return;
	CDnMonsterState::GradeEnum Grade = ((CDnMonsterActor*)m_hActor.GetPointer())->GetGrade();

	int nArray = nNameIndex[Grade];
	m_pName[nArray]->SetTextColor( dwColorArray[Grade], true );
	m_pName[nArray]->SetShadowColor( dwShadowArray[Grade], true );
	m_pName[nArray]->SetText( hActor->GetName() );

	SetColorProgressBar();
}

void CDnEnemyGaugeDlg::UpdateGaugePos()
{
	SAABox box;
	m_hActor->GetBoundingBox(box);
	float fHeight = box.Max.y - box.Min.y;

	float fDepth = box.Max.z - box.Min.z;
	//fHeight *= m_hActor->GetScale()->y;
	EtVector3 vPos = m_hActor->GetMatEx()->m_vPosition;
	vPos.y += fHeight;

	if( !CEtCamera::GetActiveCamera() ) {
		return;
	}

	EtVector4 vDamagePos, vDamagePosDepth;
	EtVec4Transform( &vDamagePos, &EtVector4(vPos, 1.0f), CEtCamera::GetActiveCamera()->GetViewMat() );
	if( vDamagePos.z < 0.0f ) {
		SetPosition( -100.0f, -100.0f );
		return;
	}

	vDamagePosDepth = vDamagePos;
	vDamagePosDepth.z -= (fDepth+10.0f);
	EtVec4Transform( &vDamagePos, &vDamagePos, CEtCamera::GetActiveCamera()->GetProjMat() );
	EtVec4Transform( &vDamagePosDepth, &vDamagePosDepth, CEtCamera::GetActiveCamera()->GetProjMat() );
	vDamagePos.x /= vDamagePos.w;
	vDamagePos.y /= vDamagePos.w;
	vDamagePos.z = vDamagePosDepth.z / vDamagePosDepth.w;

	m_pHPBar->SetZValue( vDamagePos.z );
	m_pSuperArmorBar->SetZValue( vDamagePos.z );
	m_pBalloon->SetZValue( vDamagePos.z );
	for( int i=0; i<2; i++ ) {
		for( int j=0; j<2; j++ ) m_pNameIcon[i][j]->SetZValue( vDamagePos.z );
	}
	for( int i=0; i<3; i++ )
		m_pName[i]->SetZValue( vDamagePos.z );

	vDamagePos.x *= GetScreenWidthRatio();
	vDamagePos.x = (vDamagePos.x*0.5f)+0.5f;
	vDamagePos.y *= GetScreenHeightRatio();
	vDamagePos.y = (vDamagePos.y*0.5f)+0.5f;
	vDamagePos.y = 1.0f - vDamagePos.y;

	static float fUpVal = 0.035f;
	vDamagePos.y -= fUpVal;

	bool bUseGaugeBalloon = false;

	if( m_hActor ) {
		CDnMonsterActor *pMonsterActor = dynamic_cast<CDnMonsterActor*>(m_hActor.GetPointer());
		if( pMonsterActor ) {
			DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TMONSTER );
			int nUseGaugeBalloon = pSox->GetFieldFromLablePtr( pMonsterActor->GetMonsterClassID(), "_UseGaugeBalloon" )->GetInteger();
			if( nUseGaugeBalloon != 0 ) {
				bUseGaugeBalloon = true;
			}
		}
	}

	static float fMinY = 0.025f * DEFAULT_UI_SCREEN_HEIGHT / GetEtDevice()->Height();
	static float fMinBallonY = -0.1f * DEFAULT_UI_SCREEN_HEIGHT / GetEtDevice()->Height();
	if( vDamagePos.y < fMinY ) {
		if( vDamagePos.y < fMinBallonY ) {
			m_pBalloon->Show( bUseGaugeBalloon ? true : false );
		}
		vDamagePos.y = fMinY;				
	}
	else {
		m_pBalloon->Show( false );
	}

	//if( ( abs( vDamagePos.x - m_vDamageOld.x ) > 0.001f ) || ( abs( vDamagePos.y - m_vDamageOld.y ) > 0.001f ) )
	{
		SUICoord dlgCoord;
		GetDlgCoord( dlgCoord );
		dlgCoord.SetPosition( vDamagePos.x-(dlgCoord.fWidth * 0.5f), vDamagePos.y );
		SetDlgCoord( dlgCoord );

		//m_vDamageOld = vDamagePos;
	}
}

void CDnEnemyGaugeDlg::ResetDelayTime()
{
	m_fDelayTime = 2.0f;
}

void CDnEnemyGaugeDlg::SetColorProgressBar()
{
	m_pHPBar->UseProgressIndex( false );

	if( !CDnActor::s_hLocalActor )
		return;

	CDnLocalPlayerActor * pLocalPlayer = dynamic_cast<CDnLocalPlayerActor *>(CDnActor::s_hLocalActor.GetPointer());

	if( NULL == pLocalPlayer )
		return;

	if( m_hActor->GetTeam() == pLocalPlayer->GetTeam() )
		return;

	m_pHPBar->UseProgressIndex( true );
	m_pHPBar->SetProgressIndex( 2 );
}