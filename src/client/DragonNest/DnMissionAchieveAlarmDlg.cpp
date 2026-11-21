#include "StdAfx.h"
#include "DnMissionAchieveAlarmDlg.h"
#include "DnItemTask.h"
#include "DnTableDB.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMissionAchieveAlarmDlg::CDnMissionAchieveAlarmDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback  )
, m_fElapsedTime(-1.0f)
, m_fConst(1.0f)
, m_fShowTime(0.0f)
, m_fShowRatio(0.8f)
{
	for( int i = 0; i < NUM_ACHIEVEICON; ++i ) {
		m_pTitle[i] = NULL;
		m_pSubTitle[i] = NULL;
		m_pAchieveIcon[i] = NULL;
	}
}

CDnMissionAchieveAlarmDlg::~CDnMissionAchieveAlarmDlg(void)
{
}

void CDnMissionAchieveAlarmDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MainTitleDlg.ui" ).c_str(), bShow );
	SetFadeMode( CEtUIDialog::CancelRender );
}

void CDnMissionAchieveAlarmDlg::InitialUpdate()
{
	char szName[32];
	for( int i = 0; i < NUM_ACHIEVEICON; ++i )
	{
		sprintf_s( szName, 32, "ID_STATIC_TITLE%d", i );
		m_pTitle[i] = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, 32, "ID_STATIC_SUBTITLE%d", i );
		m_pSubTitle[i] = GetControl<CEtUIStatic>( szName );

		sprintf_s( szName, 32, "ID_MISSION_ICON%d", i );
		m_pAchieveIcon[i] = GetControl<CEtUIStatic>( szName );
	}
}

void CDnMissionAchieveAlarmDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );

	if( m_fElapsedTime < 0.0f )
	{
		if( IsShow() ) Show( false );
		return;
	}

	if( m_fShowTime > 0.0f )
	{
		fElapsedTime = min( fElapsedTime, 0.5f );	// When entering village for the first time, fDelta can exceed 2.5s due to loading. So forcibly lower fDelta.
		m_fShowTime -= fElapsedTime;
		return;
	}
	else
	{
		Show( false );
	}

	m_fElapsedTime -= fElapsedTime;
}

void CDnMissionAchieveAlarmDlg::Show( bool bShow )
{ 
	CEtUIDialog::Show( bShow );
}

void CDnMissionAchieveAlarmDlg::SetMissionAlarm( CDnMissionTask::MissionInfoStruct *pInfo, float fFadeTime )
{
	for( int i = 0; i < NUM_ACHIEVEICON; ++i ) {
		m_pTitle[i]->Show( false );
		m_pSubTitle[i]->Show( false );
		m_pAchieveIcon[i]->Show( false );
	}
	int nIndex = pInfo->nAchieveIcon;
	if( nIndex < 0 || nIndex >= NUM_ACHIEVEICON ) nIndex = 0;

	m_pTitle[nIndex]->Show( true );
	m_pSubTitle[nIndex]->Show( true );
	m_pTitle[nIndex]->SetText( pInfo->szTitle );
	m_pSubTitle[nIndex]->SetText( pInfo->szSubTitle );
	m_pAchieveIcon[nIndex]->Show( true );

	SetFadeTime( fFadeTime );
}

void CDnMissionAchieveAlarmDlg::SetFadeTime( float fFadeTime )
{
	m_fShowTime = fFadeTime * m_fShowRatio;
	m_fElapsedTime = fFadeTime - m_fShowTime;
	m_fConst = m_fElapsedTime;
}
