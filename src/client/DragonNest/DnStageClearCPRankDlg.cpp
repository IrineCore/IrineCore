#include "stdafx.h"
#include "DnStageClearCPRankDlg.h"
#include "TaskManager.h"
#include "DnGameTask.h"
#include "DnBridgeTask.h"
#include "DnLocalPlayerActor.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnStageClearCPRankDlg::CDnStageClearCPRankDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	memset( m_pLegendStatic, NULL, sizeof(m_pLegendStatic) );
	memset( m_pAbyssStatic, NULL, sizeof(m_pAbyssStatic) );
	memset( m_pMyStatic, NULL, sizeof(m_pMyStatic) );
}

CDnStageClearCPRankDlg::~CDnStageClearCPRankDlg(void)
{

}

void CDnStageClearCPRankDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "StageClearCPRankDlg.ui" ).c_str(), bShow );
}

void CDnStageClearCPRankDlg::InitialUpdate()
{
	m_pLegendStatic[E_NAME] = GetControl<CEtUIStatic>( "ID_TEXT_LEGENDNAME" );
	m_pLegendStatic[E_RANK] = GetControl<CEtUIStatic>( "ID_TEXT_LEGENDRANK" );
	m_pLegendStatic[E_CP_POINT] = GetControl<CEtUIStatic>( "ID_TEXT_LEGENDCPPOINT" );

	m_pAbyssStatic[E_NAME] = GetControl<CEtUIStatic>( "ID_TEXT_ABYSSNAME" );
	m_pAbyssStatic[E_RANK] = GetControl<CEtUIStatic>( "ID_TEXT_ABYSSRANK" );
	m_pAbyssStatic[E_CP_POINT] = GetControl<CEtUIStatic>( "ID_TEXT_ABYSSCPPOINT" );

	m_pMyStatic[E_NAME] = GetControl<CEtUIStatic>( "ID_TEXT_SELFNAME" );
	m_pMyStatic[E_RANK] = GetControl<CEtUIStatic>( "ID_TEXT_SELFRANK" );
	m_pMyStatic[E_CP_POINT] = GetControl<CEtUIStatic>( "ID_TEXT_SELFCPPOINT" );
}

void CDnStageClearCPRankDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( true == bShow )
	{
		CDnGameTask* pGameTask = (CDnGameTask*)(CTaskManager::GetInstance().GetTask("GameTask"));
		if( NULL != pGameTask )
		{
			CDnGameTask::SDungeonClearBestInfo & sDungeonClearBestInfo = pGameTask->GetDungeonClearBestInfo();
			SetLegendData( sDungeonClearBestInfo.sLegendClearBest.wszName.c_str()
						, sDungeonClearBestInfo.sLegendClearBest.cRank
						, sDungeonClearBestInfo.sLegendClearBest.nClearPoint
						, sDungeonClearBestInfo.sLegendClearBest.bNew );

			SetAbyssData( sDungeonClearBestInfo.sMonthlyClearBest.wszName.c_str()
				, sDungeonClearBestInfo.sMonthlyClearBest.cRank
				, sDungeonClearBestInfo.sMonthlyClearBest.nClearPoint
				, sDungeonClearBestInfo.sMonthlyClearBest.bNew );

			CDnLocalPlayerActor * pLocalActor = (CDnLocalPlayerActor *)(CDnActor::s_hLocalActor.GetPointer());
			if( NULL != pLocalActor )
			{
				SetMyData( pLocalActor->GetName()
					, sDungeonClearBestInfo.sMyClearBest.cRank
					, sDungeonClearBestInfo.sMyClearBest.nClearPoint
					, sDungeonClearBestInfo.sMyClearBest.bNew );
			}
		}
	}

	CEtUIDialog::Show( bShow );
}

void CDnStageClearCPRankDlg::SetLegendData( const WCHAR * wszName, const char cRank, int nCP_Point, bool bNew )
{
	SetData( m_pLegendStatic, wszName, cRank, nCP_Point, bNew );
}

void CDnStageClearCPRankDlg::SetAbyssData( const WCHAR * wszName, const char cRank, int nCP_Point, bool bNew )
{
	SetData( m_pAbyssStatic, wszName, cRank, nCP_Point, bNew );
}

void CDnStageClearCPRankDlg::SetMyData( const WCHAR * wszName, const char cRank, int nCP_Point, bool bNew )
{
	SetData( m_pMyStatic, wszName, cRank, nCP_Point, bNew );
}

void CDnStageClearCPRankDlg::SetData( CEtUIStatic ** pStatic, const WCHAR * wszName, const char cRank, int nCP_Point, bool bNew )
{
	pStatic[E_RANK]->Show( false );	// Dongju requested hiding the rank because party rank differs from personal rank.

	if( 0 == nCP_Point )
	{
		pStatic[E_NAME]->SetText( L"-" );
		//pStatic[E_RANK]->SetText( L"-" );
		pStatic[E_CP_POINT]->SetText( L"-" );
		return;
	}

	static WCHAR *wszStaticRank[7] = { L"SSS", L"SS", L"S", L"A", L"B", L"C", L"D" };

	pStatic[E_NAME]->SetText( wszName );

	//if( 0 <= cRank && 7 > cRank )
	//	pStatic[E_RANK]->SetText( wszStaticRank[cRank] );

	WCHAR wszCPText[256] = {0,};
	swprintf_s( wszCPText, _countof(wszCPText), L"%d CP", nCP_Point );
	pStatic[E_CP_POINT]->SetText( wszCPText );

	pStatic[E_NAME]->SetBlink( bNew );
	//pStatic[E_RANK]->SetBlink( bNew );
	pStatic[E_CP_POINT]->SetBlink( bNew );
}