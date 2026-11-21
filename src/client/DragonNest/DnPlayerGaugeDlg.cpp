#include "StdAfx.h"
#include "DnPlayerGaugeDlg.h"
#include "DnInterface.h"
#include "DnPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPlayerGaugeDlg::CDnPlayerGaugeDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnEnemyGaugeDlg( dialogType, pParentDialog, nID, pCallback  )
{
	m_pSPBar = NULL;	
}

CDnPlayerGaugeDlg::~CDnPlayerGaugeDlg(void)
{	
	
}

void CDnPlayerGaugeDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpGaugeDlg.ui" ).c_str(), bShow );	
}

void CDnPlayerGaugeDlg::InitialUpdate()
{
	m_pHPBar = GetControl< CEtUIProgressBar >( "ID_HP_GAUGE" );	
	m_pSPBar = GetControl< CEtUIProgressBar >( "ID_MP_GAUGE" );
	//m_pName = GetControl< CEtUIStatic >( "ID_PLAYER_NAME" );

	//m_dwDefaultColor = m_pName->GetTextColor();

	// Initial position is completely off-screen.
	SetPosition( -100.0f, -100.0f );
}

void CDnPlayerGaugeDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( !m_hActor ) {
		m_fDelayTime = 0.f;
		return;
	}
	if( !m_hActor->IsProcess() ) {
		m_fDelayTime = 0.f;
		return;
	}

	//if( m_fDelayTime > 0.0f )
	//{
	//	m_fDelayTime -= fElapsedTime;

	UpdateHP();
	UpdateSP();
	UpdateGaugePos();			
	//}
}

void CDnPlayerGaugeDlg::UpdateHP()
{
	if( !m_hActor ) {
		return;
	}
	float fHP(0.0f);

	if( m_hActor->GetMaxHP() > 0 )
	{
		fHP = m_hActor->GetHP() / ( float )m_hActor->GetMaxHP();
	}

	m_pHPBar->SetProgress( fHP * 100.0f );

	/*
	DWORD dwColor(0);
	DWORD dwColorS(0);

	if( fHP > 0.3f )	
	{
		dwColor = EtInterface::textcolor::MONSTER;
		dwColorS = EtInterface::textcolor::MONSTER_S;
	}
	else					
	{
		dwColor = EtInterface::textcolor::MONSTERi;
		dwColorS = EtInterface::textcolor::MONSTERi_S;
	}
	*/
	
}

void CDnPlayerGaugeDlg::UpdateSP()
{
	if( !m_hActor ) {
		return;
	}

	float fHP(0.0f);

	if( m_hActor->GetMaxSP() > 0 )
	{
		fHP = m_hActor->GetSP() / ( float )m_hActor->GetMaxSP();
	}

	m_pSPBar->SetProgress( fHP * 100.0f );

	/*
	DWORD dwColor(0);
	DWORD dwColorS(0);

	if( fHP > 0.3f )	
	{
		dwColor = EtInterface::textcolor::MONSTER;
		dwColorS = EtInterface::textcolor::MONSTER_S;
	}
	else					
	{
		dwColor = EtInterface::textcolor::MONSTERi;
		dwColorS = EtInterface::textcolor::MONSTERi_S;
	}
	*/
}

void CDnPlayerGaugeDlg::SetActor( DnActorHandle hActor ) 
{ 
	m_hActor = hActor;

	SetColorProgressBar();
}


void CDnPlayerGaugeDlg::UpdateGaugePos()
{
	if(!m_hActor) {
		return;
	}
	if( !CEtCamera::GetActiveCamera() ) {
		return;
	}
	SAABox box;
	m_hActor->GetBoundingBox(box);
	float fHeight = box.Max.y - box.Min.y + m_hActor->GetIncreaseHeight(); // Also calculate HeadName position.
	fHeight /= m_hActor->GetScale()->y;
	EtVector3 vPos = m_hActor->GetMatEx()->m_vPosition;

	CDnPlayerActor *pPlayerActor = static_cast<CDnPlayerActor*>( CDnActor::s_hLocalActor.GetPointer() );
	if( pPlayerActor )
	{
		// #59366 HP bar overlapping with ID in Ghoul mode
		(pPlayerActor->IsSwapSingleSkin()) ? vPos.y += fHeight - 8.0f :// Ghoul mode.
											 vPos.y += fHeight - 5.0f ;// Normal mode: positioned 5.0f below HeadName.
	}

	EtVector4 vUIPos;
	EtVec4Transform( &vUIPos, &EtVector4(vPos, 1.0f), CEtCamera::GetActiveCamera()->GetViewMat() );
	if( vUIPos.z < 0.0f ) {
		// If inside the screen.. (Not strictly needed, kept for safety.)
		SetPosition( -100.0f, -100.0f );
		return;
	}

	static float fMovePos = 50.0f;

	//vUIPos.z -= 30.0f;		
	EtVector4 vSample = vUIPos;
	vSample.z -= fMovePos;			// Pull slightly forward to prevent being occluded by the monster.
	EtVec4Transform( &vUIPos, &vUIPos, CEtCamera::GetActiveCamera()->GetProjMat() );	
	EtVec4Transform( &vSample, &vSample, CEtCamera::GetActiveCamera()->GetProjMat() );	
	vUIPos.x /= vUIPos.w;
	vUIPos.y /= vUIPos.w;
	vUIPos.z = vSample.z / vSample.w;
	
	m_pHPBar->SetZValue( vUIPos.z );
	m_pSPBar->SetZValue( vUIPos.z );
//	m_pName->SetZValue( vDamagePos.z );

	vUIPos.x *= GetScreenWidthRatio();
	vUIPos.x = (vUIPos.x*0.5f)+0.5f;
	vUIPos.y *= GetScreenHeightRatio();
	vUIPos.y = (vUIPos.y*0.5f)+0.5f;
	vUIPos.y = 1.0f - vUIPos.y;

	static float fMinY = 0.025f;
	if( vUIPos.y < fMinY ) {
		vUIPos.y = fMinY;
	}

	SUICoord dlgCoord;
	GetDlgCoord( dlgCoord );
	dlgCoord.SetPosition( vUIPos.x-(dlgCoord.fWidth * 0.5f), vUIPos.y );
	//dlgCoord.SetPosition( vDamagePos.x-(dlgCoord.fWidth * 0.5f) , vDamagePos.y  );
	SetDlgCoord( dlgCoord );	

}
