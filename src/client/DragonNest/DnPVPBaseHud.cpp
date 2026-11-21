#include "StdAfx.h"
#include "DnPVPBaseHUD.h"
#include "DnTableDB.h"
#include "DnPvPKillListDlg.h"
#include "Timeset.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPBaseHUD::CDnPVPBaseHUD( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
,m_pRoomName(NULL)
,m_pMYTeam(NULL)
,m_pEnemyTeam(NULL)
,m_pMYTeamScore(NULL)
,m_pEneyTeamScore(NULL)
,m_pTimer_Min(NULL)
,m_pTimer_Sec(NULL)
,m_pObjectiveCount(NULL)
,m_pKillMark(NULL)
,m_pKillListDlg(NULL)
,m_pTimer_Semi(NULL)
{
	m_StartTimer= false;

	m_fRemainSec = 0.0f;
	m_fTotalSec = 0.0f;
	m_fKillMarkTime = 0.0f;	

	m_nMin = -1;
	m_nSec = -1;
	m_nKillSoundID = -1;
	m_nStartSound = -1;	
	m_nMyTeam_Score = 0;
	m_nEnemyTeam_Score = 0;
	m_nCurContinuousKillCount = 0;
}

CDnPVPBaseHUD::~CDnPVPBaseHUD(void)
{
	SAFE_DELETE( m_pKillListDlg );
	if( m_nKillSoundID != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nKillSoundID );

	if( m_nStartSound != -1 )
		CEtSoundEngine::GetInstance().RemoveSound( m_nStartSound );	
}

void CDnPVPBaseHUD::SetTime( float nSec, __time64_t sTime ,int sMSec )
{
	m_fRemainSec = float(nSec);
	m_fTotalSec = m_fRemainSec-(float)sMSec / 1000; // sMSec remaining milliseconds
	m_tAxisTime = sTime;
	m_tMSec = (float)sMSec / 1000;
	UpdateTimer();
}

void CDnPVPBaseHUD::UpdateTimer()
{
	int CurSec = static_cast<int>(ceil(m_fRemainSec))%60;

	if( m_nSec  !=  CurSec ) 
	{
		if( CurSec < 0 )
			CurSec = 0;
		m_nSec  =  CurSec;
		WCHAR wszSec[128];
		wsprintf(wszSec,L"%.2d",m_nSec);
		m_pTimer_Sec->SetText(wszSec);
	}

	int CurMin = static_cast<int>(ceil(m_fRemainSec))/60;

	if( m_nMin  !=  CurMin ) //Minute update
	{
		if( m_nMin < 0 )
			m_nMin = 0;

		m_nMin = CurMin;

		WCHAR wszMin[128];
		wsprintf(wszMin,L"%.2d",m_nMin);
		m_pTimer_Min->SetText(wszMin);
	}
}

void CDnPVPBaseHUD::SetMyScore( int nScore )
{
	WCHAR wszScore[128];
	wsprintf(wszScore,L"%d",nScore);

	m_nMyTeam_Score = nScore;
	m_pMYTeamScore->SetText(wszScore);

}

void CDnPVPBaseHUD::SetEnemyScore( int nScore )
{
	WCHAR wszScore[128];
	wsprintf(wszScore,L"%d",nScore);

	m_nEnemyTeam_Score = nScore;
	m_pEneyTeamScore->SetText(wszScore);
}

void CDnPVPBaseHUD::Initialize( bool bShow )
{
	CDnCustomDlg::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpRespawnInGame.ui" ).c_str(), bShow );
}

void CDnPVPBaseHUD::SetRoomName( const WCHAR * wszRoomName )
{
	m_pRoomName->SetText(wszRoomName);
}

void CDnPVPBaseHUD::SetWinCondition( WCHAR * wszString )
{
	if( wszString )
		m_pObjectiveCount->SetText(wszString);
}

void CDnPVPBaseHUD::InitialUpdate()
{	
	m_pMYTeamScore = GetControl<CEtUIStatic>("ID_SCORE_MYTEAM");
	m_pEneyTeamScore = GetControl<CEtUIStatic>("ID_SCORE_ENEMY");
	m_pTimer_Min = GetControl<CEtUIStatic>("ID_TIME_MINUTE");
	m_pTimer_Sec = GetControl<CEtUIStatic>("ID_TIME_SECOND");
	m_pTimer_Semi = GetControl<CEtUIStatic>("22");
	m_pObjectiveCount = GetControl<CEtUIStatic>("ID_PVP_OBJECT");

	m_pTimer_Min->SetText(L"00");
	m_pTimer_Sec->SetText(L"00");

	m_pKillMark = GetControl<CEtUIStatic>("ID_KILLMARK0"); 
	m_pKillMark->Show(false);

	SetMyScore( 0 );
	SetEnemyScore(  0 );

	const char *szFileName = CDnTableDB::GetInstance().GetFileName( 10013 );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nKillSoundID == -1 )
			m_nKillSoundID = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );			
	}

	szFileName = CDnTableDB::GetInstance().GetFileName( 10008 );
	if( strlen( szFileName ) > 0 )
	{
		if( m_nStartSound == -1 )
			m_nStartSound = CEtSoundEngine::GetInstance().LoadSound( CEtResourceMng::GetInstance().GetFullName( szFileName ).c_str(), false, false );			
	}		

	// Temporary hide handling
	char szName[32];
	for( int i = 0; i < MAX_KILLMARK_COUNT; ++i )
	{
		sprintf_s( szName, _countof(szName), "ID_KILLMARK%d", i );
		m_pKillMarks[i] = GetControl<CEtUIStatic>(szName);

		sprintf_s( szName, _countof(szName), "ID_KILLTEXT%d", i );
		m_pKillTexts[i] = GetControl<CEtUIStatic>(szName);

		m_pKillMarks[i]->Show(false);
		m_pKillTexts[i]->Show(false);
	}

	m_uiKillMarkCenter = m_pKillMarks[0]->GetUICoord();
	m_uiKillMarkLeft = m_pKillMarks[1]->GetUICoord();
	m_uiKillMarkRight = m_pKillMarks[2]->GetUICoord();

	m_pKillListDlg = new CDnPvPKillListDlg( UI_TYPE_TOP );
	m_pKillListDlg->Initialize( true );
}

void CDnPVPBaseHUD::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );

	if( m_StartTimer && m_fRemainSec > 0.0f )
	{
		m_fRemainSec = (m_fTotalSec - (float)((CTimeSet().GetTimeT64_GM() - (float)m_tAxisTime)));
		UpdateTimer();
	}
	else if(!m_StartTimer)
	{
		m_tAxisTime = CTimeSet().GetTimeT64_GM();
		m_tMSec = (float)CTimeSet().GetMilliseconds()/1000;
	}

	if( m_fKillMarkTime > 0.0f)
	{
		m_fKillMarkTime -= fElapsedTime;
		if( m_fKillMarkTime <= 0.0f )
		{
			m_fKillMarkTime = 0.0f;

			int nContinuousKillCount = m_nCurContinuousKillCount;
			if( nContinuousKillCount > MAX_KILLMARK_COUNT )
				nContinuousKillCount = MAX_KILLMARK_COUNT;

			for( int i = 0; i < nContinuousKillCount; ++i ) {
				m_pKillMarks[i]->SetBlendRate( 0.5f );
				m_pKillMarks[i]->Show(false);
			}
			for( int i = 0; i < nContinuousKillCount; ++i ) {
				if( m_pKillTexts[i]->IsShow() ) {
					m_pKillTexts[i]->SetBlendRate( 0.5f );
					m_pKillTexts[i]->Show(false);
				}
			}
		}
	}	
}

void CDnPVPBaseHUD::ShowKillMark( int nContinuousKillCount, float fTime )
{
	// Exception handling: if a kill count smaller than current arrives, hide all and restart.
	if( m_fKillMarkTime > 0.0f && m_nCurContinuousKillCount > nContinuousKillCount ) {
		for( int i = 0; i < MAX_KILLMARK_COUNT; ++i ) {
			m_pKillMarks[i]->SetBlendRate( 0.0f );
			m_pKillMarks[i]->Show(false);
		}
	}

	// Only one kill text should be visible at a time, so call Show( false ) initially whenever setting it.
	for( int i = 0; i < MAX_KILLMARK_COUNT; ++i ) {
		m_pKillTexts[i]->SetBlendRate( 0.0f );
		m_pKillTexts[i]->Show(false);
	}

	m_nCurContinuousKillCount = nContinuousKillCount;

	if( nContinuousKillCount > MAX_KILLMARK_COUNT )
		nContinuousKillCount = MAX_KILLMARK_COUNT;

	UpdateKillMarkPos( nContinuousKillCount );

	for( int i = 0; i < nContinuousKillCount; ++i ) {
		m_pKillMarks[i]->SetBlendRate( 0.0f );
		m_pKillMarks[i]->Show(true);
	}

	if( m_nCurContinuousKillCount > 1 ) {
		int nTextIndex = m_nCurContinuousKillCount-2;
		if( nTextIndex > MAX_KILLMARK_COUNT-1 )
			nTextIndex = MAX_KILLMARK_COUNT-1;
			
		m_pKillTexts[nTextIndex]->SetBlendRate( 0.0f );
		m_pKillTexts[nTextIndex]->Show( true );
	}
	m_fKillMarkTime = fTime;

	if( m_nKillSoundID != -1 )
		CEtSoundEngine::GetInstance().PlaySound( "2D", m_nKillSoundID, false );
}

void CDnPVPBaseHUD::UpdateKillMarkPos( int nContinuousKillCount )
{
	// After obtaining default spacing
	float fXGap = m_uiKillMarkRight.fX - m_uiKillMarkLeft.fX;
	SUICoord uiStartCoord;

	if( nContinuousKillCount % 2 == 0 ) {
		// Calculate leftmost fX when count is even
		int nCount = nContinuousKillCount / 2;
		uiStartCoord = m_uiKillMarkRight;
		uiStartCoord.fX -= ( nCount * fXGap );
	}
	else {
		// Calculate the leftmost fX when count is odd
		int nCount = nContinuousKillCount / 2;
		uiStartCoord = m_uiKillMarkCenter;
		uiStartCoord.fX -= ( nCount * fXGap );
	}

	for( int i = 0; i < nContinuousKillCount; ++i ){
		m_pKillMarks[i]->SetPosition( uiStartCoord.fX + (fXGap * i), uiStartCoord.fY );
	}
}

void CDnPVPBaseHUD::StartTimer( )
{
	m_StartTimer = true;

	if( m_nStartSound != -1 )
		CEtSoundEngine::GetInstance().PlaySound( "2D", m_nStartSound, false );
};

void CDnPVPBaseHUD::ShowKillInfo( DnActorHandle hKiller, DnActorHandle hKilled, CDnWeapon::EquipTypeEnum EquipType )
{
	if( m_pKillListDlg )
		m_pKillListDlg->AddInfo( hKiller, hKilled, EquipType );
}


#ifdef PRE_MOD_PVPOBSERVER
bool CDnPVPBaseHUD::SetTextObserverTeam( CEtUIStatic * pStaticFriend, CEtUIStatic * pStaticEnemy, int textIdxFriend, int textIdxEnemy )
{
	if( CDnActor::s_hLocalActor && CDnActor::s_hLocalActor->GetTeam() == PvPCommon::Team::eTeam::Observer )
	{
		if( pStaticFriend)
			pStaticFriend->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, textIdxFriend ) );
		if( pStaticEnemy )
			pStaticEnemy->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, textIdxEnemy ) );

		return true;
	}

	return false;
}
#endif // PRE_MOD_PVPOBSERVER
