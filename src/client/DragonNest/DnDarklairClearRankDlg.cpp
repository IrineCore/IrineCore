#include "StdAfx.h"
#include "DnDarklairClearRankDlg.h"
#include "DnActorState.h"
#include "DnDLGameTask.h"
#include "DnDarklairClearImp.h"
#include "TaskManager.h"
#include "DnTableDB.h"
#include "DnPlayerActor.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


CDnDarklairClearRankDlg::CDnDarklairClearRankDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
{
	m_pStaticPlayerInfo = NULL;
	m_pStaticCurrentRound = NULL;
	m_pStaticCurrentRank = NULL;
	m_pStaticCurrentTime = NULL;
	memset( m_pStaticCurrentJob, 0, sizeof(m_pStaticCurrentJob) );

	m_pStaticBestRound = NULL;
	m_pStaticBestRank = NULL;
	m_pStaticBestTime = NULL;
	memset( m_pStaticBestJob, 0, sizeof(m_pStaticBestJob) );

	m_pStaticHistoryTitle = NULL;
	memset( m_pStaticTopHistoryRank, 0, sizeof(m_pStaticTopHistoryRank) );
	memset( m_pStaticTopHistoryName, 0, sizeof(m_pStaticTopHistoryName) );
	memset( m_pStaticTopHistoryRound, 0, sizeof(m_pStaticTopHistoryRound) );
	memset( m_pStaticTopHistoryTime, 0, sizeof(m_pStaticTopHistoryTime) );
	memset( m_pStaticTopHistoryJob, 0, sizeof(m_pStaticTopHistoryJob) );
}

CDnDarklairClearRankDlg::~CDnDarklairClearRankDlg()
{
}

void CDnDarklairClearRankDlg::InitialUpdate()
{
	char szStr[64];
	m_pStaticPlayerInfo = GetControl<CEtUIStatic>("ID_TEXT_NAME");

	m_pStaticCurrentRound = GetControl<CEtUIStatic>("ID_TEXT_ROUNDNOW");
	m_pStaticCurrentRank = GetControl<CEtUIStatic>("ID_TEXT_RANKNOW_LIST");
	m_pStaticCurrentTime = GetControl<CEtUIStatic>("ID_TEXT_RANKNOW_TIME");
	for( int i=0; i<4; i++ )
	{
		sprintf_s( szStr, "ID_NOW_CLASS%d", i );
		m_pStaticCurrentJob[i] = GetControl<CDnJobIconStatic>( szStr );
/*
		for( int j=0; j<CDnActorState::Reserved1; j++ ) 
		{
			sprintf_s( szStr, "ID_NOW_JOB%d_%d", i, j );
			m_pStaticCurrentJob[i][j] = GetControl<CEtUIStatic>( szStr );
		}
*/
	}

	m_pStaticBestRound = GetControl<CEtUIStatic>("ID_TEXT_ROUNDBEST");
	m_pStaticBestRank = GetControl<CEtUIStatic>("ID_TEXT_RANKBEST_LIST");
	m_pStaticBestTime = GetControl<CEtUIStatic>("ID_TEXT_RANKBEST_TIME");
	for( int i=0; i<4; i++ )
	{
		sprintf_s( szStr, "ID_BEST_CLASS%d", i );
		m_pStaticBestJob[i] = GetControl<CDnJobIconStatic>( szStr );
/*
		for( int j=0; j<CDnActorState::Reserved1; j++ ) 
		{
			sprintf_s( szStr, "ID_BEST_JOB%d_%d", i, j );
			m_pStaticBestJob[i][j] = GetControl<CEtUIStatic>( szStr );
			m_pStaticBestJob[i][j]->Show( false );
		}
*/
	}

	m_pStaticHistoryTitle = GetControl<CEtUIStatic>("ID_TEXT_DARKRANK_TITLE");
	for( int i=0; i<5; i++ )
	{
		sprintf_s( szStr, "ID_TEXT_RANK%d", i );
		m_pStaticTopHistoryRank[i] = GetControl<CEtUIStatic>( szStr );

		sprintf_s( szStr, "ID_TEXT_RANK_NAME%d", i );
		m_pStaticTopHistoryName[i] = GetControl<CEtUIStatic>( szStr );

		sprintf_s( szStr, "ID_TEXT_RANK_ROUND%d", i );
		m_pStaticTopHistoryRound[i] = GetControl<CEtUIStatic>( szStr );

		sprintf_s( szStr, "ID_TEXT_RANK_TIME%d", i );
		m_pStaticTopHistoryTime[i] = GetControl<CEtUIStatic>( szStr );

		for( int j=0; j<4; j++ ) 
		{
			sprintf_s( szStr, "ID_RANK%d_CLASS%d", i, j );
			m_pStaticTopHistoryJob[i][j] = GetControl<CDnJobIconStatic>( szStr );
/*
			for( int k=0; k<CDnActorState::Reserved1; k++ ) 
			{
				sprintf_s( szStr, "ID_RANK%d_JOB%d_%d", i, j, k );
				m_pStaticTopHistoryJob[i][j][k] = GetControl<CEtUIStatic>( szStr );
				m_pStaticTopHistoryJob[i][j][k]->Show( false );
			}
*/
		}		
	}
}

void CDnDarklairClearRankDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DarkRareClearListDlg.ui" ).c_str(), bShow );
}

void CDnDarklairClearRankDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDarklairClearRankDlg::Show( bool bShow )
{
	if( m_bShow == bShow ) return;

	if( bShow ) {
		Refresh();
	}

	CEtUIDialog::Show( bShow );
}

void CDnDarklairClearRankDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
}

void CDnDarklairClearRankDlg::Render( float fElapsedTime )
{
	CEtUIDialog::Render( fElapsedTime );
}


void CDnDarklairClearRankDlg::Refresh()
{
	// Hide for now
	for( int i=0; i<4; i++ ) 
	{
		m_pStaticCurrentJob[i]->SetIconID( -1 );
//		for( int j=0; j<CDnActorState::Reserved1; j++ ) 
//			m_pStaticCurrentJob[i][j]->Show( false );
	}
	m_pStaticCurrentRound->Show( false );
	m_pStaticCurrentRank->Show( false );
	m_pStaticCurrentTime->Show( false );

	for( int i=0; i<4; i++ )
	{
		m_pStaticBestJob[i]->SetIconID( -1 );
//		for( int j=0; j<CDnActorState::Reserved1; j++ )
//			m_pStaticBestJob[i][j]->Show( false );
	}
	m_pStaticBestRound->Show( false );
	m_pStaticBestRank->Show( false );
	m_pStaticBestTime->Show( false );

	for( int i=0; i<5; i++ ) 
	{
		for( int j=0; j<4; j++ ) 
		{
			m_pStaticTopHistoryJob[i][j]->SetIconID( -1 );
//			for( int k=0; k<CDnActorState::Reserved1; k++ )
//				m_pStaticTopHistoryJob[i][j][k]->Show( false );
		}
		m_pStaticTopHistoryRank[i]->Show( false );
		m_pStaticTopHistoryName[i]->Show( false );
		m_pStaticTopHistoryRound[i]->Show( false );
		m_pStaticTopHistoryTime[i]->Show( false );
	}

	CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
	if( !pGameTask ) return;

	int nSessionID = CDnBridgeTask::GetInstance().GetSessionID();
	CDnDarklairClearImp *pClearImp = (CDnDarklairClearImp *)pGameTask->GetStageClearImp();
	if( !pClearImp ) return;

	WCHAR wszStr[128];
	DnActorHandle hActor = CDnActor::s_hLocalActor;
	if( !hActor ) return;

	CDnPlayerActor *pActor = static_cast<CDnPlayerActor *>(hActor.GetPointer());
#ifdef PRE_MOD_CONSTANT_TEXT_CONVERT_TO_TABLE_TEXT
	swprintf_s( wszStr, L"%s ( %s %s%d )", pActor->GetName(), pActor->GetJobName(), GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 7888 ), pActor->GetLevel() );
#else
	swprintf_s( wszStr, L"%s ( %s Lv.%d )", pActor->GetName(), pActor->GetJobName(), pActor->GetLevel() );
#endif 
	m_pStaticPlayerInfo->SetText( wszStr );

	// Current
	TDLRankHistoryPartyInfo *pInfo = pClearImp->GetCurrentScoreInfo();

	char cPartyUserCount = pInfo->cPartyUserCount;
	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 808 ), pInfo->nPlayRound );
	m_pStaticCurrentRound->Show( true );
	m_pStaticCurrentRound->SetText( wszStr );

	switch( pInfo->nRank ) {
		case 1: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 813 ) ); break;
		case 2: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 814 ) ); break;
		case 3: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 815 ) ); break;
		default: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 809 ), pInfo->nRank ); break;
	}
	m_pStaticCurrentRank->Show( true );
	m_pStaticCurrentRank->SetText( wszStr );

	int nH = pInfo->nPlaySec / (60 * 60);
	int nM = (pInfo->nPlaySec - (nH * 60 * 60)) / 60;
	int nS = (pInfo->nPlaySec - ((nH * 60 * 60) + (nM * 60)));
	swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
	m_pStaticCurrentTime->Show( true );
	m_pStaticCurrentTime->SetText( wszStr );

//	int nClassIcon;
	for( int i=0; i<pInfo->cPartyUserCount; i++ ) {
		if( pInfo->Info[i].nJobIndex <= 0 ) continue;
//		nClassIcon = CDnPlayerState::GetJobToBaseClassID( pInfo->Info[i].nJobIndex ) - 1;
		if( m_pStaticCurrentJob[i] )
			m_pStaticCurrentJob[i]->SetIconID( pInfo->Info[i].nJobIndex, true );
//		if( m_pStaticCurrentJob[i][nClassIcon] )
//			m_pStaticCurrentJob[i][nClassIcon]->Show( true );
	}

	// Player Top
	pInfo = pClearImp->GetUserTopScoreInfo();
	if( pInfo->nRank != 0 ) {
		swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 808 ), pInfo->nPlayRound );
		m_pStaticBestRound->Show( true );
		m_pStaticBestRound->SetText( wszStr );

		switch( pInfo->nRank ) {
			case 1: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 813 ) ); break;
			case 2: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 814 ) ); break;
			case 3: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 815 ) ); break;
			default: swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 809 ), pInfo->nRank ); break;
		}
		m_pStaticBestRank->Show( true );
		m_pStaticBestRank->SetText( wszStr );

		int nH = pInfo->nPlaySec / (60 * 60);
		int nM = (pInfo->nPlaySec - (nH * 60 * 60)) / 60;
		int nS = (pInfo->nPlaySec - ((nH * 60 * 60) + (nM * 60)));
		swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
		m_pStaticBestTime->Show( true );
		m_pStaticBestTime->SetText( wszStr );

		for( int i=0; i<pInfo->cPartyUserCount; i++ ) {
			if( pInfo->Info[i].nJobIndex <= 0 ) continue;
//			nClassIcon = CDnPlayerState::GetJobToBaseClassID( pInfo->Info[i].nJobIndex ) - 1;
			if( m_pStaticBestJob[i] ) 
				m_pStaticBestJob[i]->SetIconID( pInfo->Info[i].nJobIndex, true );
//			if( m_pStaticBestJob[i][nClassIcon] ) 
//				m_pStaticBestJob[i][nClassIcon]->Show( true );
		}
	}

	// History
	swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 810 ), cPartyUserCount );
	m_pStaticHistoryTitle->SetText( wszStr );
	for( int i=0; i<5; i++ ) {
		pInfo = pClearImp->GetTopHistoryInfo( i );
		if( pInfo->nRank == 0 ) continue;
		if( pInfo->nPlaySec == 0 ) continue;

		m_pStaticTopHistoryRank[i]->Show( true );

		m_pStaticTopHistoryName[i]->Show( true );
		m_pStaticTopHistoryName[i]->SetText( pInfo->wszPartyName );

		swprintf_s( wszStr, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 811 ), pInfo->nPlayRound );
		m_pStaticTopHistoryRound[i]->Show( true );
		m_pStaticTopHistoryRound[i]->SetText( wszStr );

		int nH = pInfo->nPlaySec / (60 * 60);
		int nM = (pInfo->nPlaySec - (nH * 60 * 60)) / 60;
		int nS = (pInfo->nPlaySec - ((nH * 60 * 60) + (nM * 60)));
		swprintf_s( wszStr, L"%02d : %02d : %02d", nH, nM, nS );
		m_pStaticTopHistoryTime[i]->Show( true );
		m_pStaticTopHistoryTime[i]->SetText( wszStr );

		for( int j=0; j<pInfo->cPartyUserCount; j++ ) {
			if( pInfo->Info[j].nJobIndex <= 0 ) continue;
//			nClassIcon = CDnPlayerState::GetJobToBaseClassID( pInfo->Info[j].nJobIndex ) - 1;
			if( m_pStaticTopHistoryJob[i][j] )
				m_pStaticTopHistoryJob[i][j]->SetIconID( pInfo->Info[j].nJobIndex, true );
//			if( m_pStaticTopHistoryJob[i][j][nClassIcon] )
//				m_pStaticTopHistoryJob[i][j][nClassIcon]->Show( true );
		}
	}
}

void CDnDarklairClearRankDlg::OnBlindClose()
{
	SetCallback( NULL );
	Show( false );
}

void CDnDarklairClearRankDlg::OnBlindClosed()
{
	GetInterface().OpenBaseDialog();

	CDnGameTask* pGameTask = static_cast<CDnGameTask*>(CTaskManager::GetInstance().GetTask("GameTask"));
	if (!pGameTask)
		return;

	CDnDarklairClearImp *pClearImp = static_cast<CDnDarklairClearImp *>(pGameTask->GetStageClearImp());
	if (pClearImp)
	{
		const CDnDarklairClearImp::SDungeonClearValue& dlClearValue = pClearImp->GetDungeonClearValue();
		CDnLocalPlayerActor::LockInput(!dlClearValue.bClear);

		GetInterface().CloseRebirthFailDlg();
	}
}
