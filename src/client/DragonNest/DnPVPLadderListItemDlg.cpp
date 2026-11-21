#include "StdAfx.h"
#include "DnPVPLadderListItemDlg.h"
#include "DnTableDB.h"

CDnPVPLadderListItemDlg::CDnPVPLadderListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_pStaticClassName = NULL;
	m_pStaticWinCount = NULL;
	m_pStaticLossCount = NULL;
	m_pStaticDrawCount = NULL;
	m_pStaticWinPercent = NULL;
}

CDnPVPLadderListItemDlg::~CDnPVPLadderListItemDlg(void)
{
}

void CDnPVPLadderListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpLadderListDlg.ui" ).c_str(), bShow );
}

void CDnPVPLadderListItemDlg::InitialUpdate()
{
	m_pStaticClassName =  GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticWinCount =  GetControl<CEtUIStatic>( "ID_TEXT_WIN" );
	m_pStaticLossCount =  GetControl<CEtUIStatic>( "ID_TEXT_DEFEAT" );
	m_pStaticDrawCount =  GetControl<CEtUIStatic>( "ID_TEXT_TIE" );
	m_pStaticWinPercent =  GetControl<CEtUIStatic>( "ID_TEXT_WINCOUNT" );
}


void CDnPVPLadderListItemDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnPVPLadderListItemDlg::SetClassInfo(TPvPLadderJobScore *JobInfo)
{
	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TJOB );	
	if(!pSox)
		return;

	std::wstring m_wszJobName;
	WCHAR wszWinningRate[20];

	if( pSox->IsExistItem( JobInfo->cJobCode ) ) {		
		m_wszJobName = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pSox->GetFieldFromLablePtr( JobInfo->cJobCode, "_JobName" )->GetInteger() );	
		m_pStaticClassName->SetText(m_wszJobName.c_str());// Most won class
	}

	m_pStaticWinCount->SetIntToText(JobInfo->iWin);
	m_pStaticLossCount->SetIntToText(JobInfo->iLose);
	m_pStaticDrawCount->SetIntToText(JobInfo->iDraw);

	int TotalCount = (JobInfo->iWin + JobInfo->iLose + JobInfo->iDraw) ? (JobInfo->iWin + JobInfo->iLose + JobInfo->iDraw) : 1;
	int WinCount = JobInfo->iWin;

	wsprintf(wszWinningRate,L"%d%%", (int)(((float)WinCount / (float)TotalCount) * 100));
	m_pStaticWinPercent->SetText(wszWinningRate);         // Win rate
}
