#include "StdAfx.h"
#include "DnPVPLadderObserverInfoDlg.h"
#include "DnInterface.h"
#include "TaskManager.h"
#include "DnWorld.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnPVPLadderObserverInfoDlg::CDnPVPLadderObserverInfoDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, true )
{
}

CDnPVPLadderObserverInfoDlg::~CDnPVPLadderObserverInfoDlg(void)
{
}

void CDnPVPLadderObserverInfoDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PvpLadderInfoDlg.ui" ).c_str(), bShow );
}

void CDnPVPLadderObserverInfoDlg::InitialUpdate()
{
	InitializeLadderUserInfoUI();
	ClearLadderUserInfo();
}

void CDnPVPLadderObserverInfoDlg::InitializeLadderUserInfoUI()
{
	char szUIName[256] = "";

	for(int i=0; i<eLadderUserInfoCommon::TeamMax ;i++)
	{
		sprintf(szUIName,"ID_TEXTUREL_CLASS0_%d",i);
		m_sLadderInfo_A[i].pJobIcon = GetControl<CDnJobIconStatic>( szUIName );
		sprintf(szUIName,"ID_TEXT_NANE0_%d",i);
		m_sLadderInfo_A[i].pName = GetControl<CEtUIStatic>( szUIName );

		sprintf(szUIName,"ID_TEXTUREL_CLASS1_%d",i);
		m_sLadderInfo_B[i].pJobIcon = GetControl<CDnJobIconStatic>( szUIName );
		sprintf(szUIName,"ID_TEXT_NANE1_%d",i);
		m_sLadderInfo_B[i].pName = GetControl<CEtUIStatic>( szUIName );
	}
}

void CDnPVPLadderObserverInfoDlg::ClearLadderUserInfo()
{
	for(int i=0; i<eLadderUserInfoCommon::TeamMax ;i++)
	{
		m_sLadderInfo_A[i].pJobIcon->Show(false);
		m_sLadderInfo_A[i].pName->ClearText();

		m_sLadderInfo_B[i].pJobIcon->Show(false);
		m_sLadderInfo_B[i].pName->ClearText();
	}
}

void CDnPVPLadderObserverInfoDlg::SetLadderUserInfo(int nLadderType,int nSelectedCount,LadderSystem::SC_PLAYING_ROOMLIST *pData)
{
	ClearLadderUserInfo();

	int LadderNameCount = LadderSystem::MatchType::_2vs2 * 2; // Default

	
	if(nLadderType >= LadderSystem::MatchType::_1vs1 && nLadderType <= LadderSystem::MatchType::MaxCount)
		LadderNameCount = nLadderType * 2; //      2 / 4/ 6 / 8

	int nStartIndex = LadderNameCount * nSelectedCount;
	if(nStartIndex + LadderNameCount > pData->unNameCount || nStartIndex < 0)
		nStartIndex = 0;

	for(int i= 0; i<eLadderUserInfoCommon::TeamMax ;i++)
	{
		if(i < nLadderType)
		{
			m_sLadderInfo_A[i].pName->SetText(pData->wszCharName[nStartIndex+i]);
			m_sLadderInfo_A[i].pJobIcon->SetIconID(pData->cJob[nStartIndex+i]);
			m_sLadderInfo_A[i].pJobIcon->Show(true);

			m_sLadderInfo_B[i].pName->SetText(pData->wszCharName[nStartIndex + LadderNameCount/2 + i ]);
			m_sLadderInfo_B[i].pJobIcon->SetIconID(pData->cJob[nStartIndex + LadderNameCount/2 + i ]);
			m_sLadderInfo_B[i].pJobIcon->Show(true);
		}
	}
}


void CDnPVPLadderObserverInfoDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnPVPLadderObserverInfoDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );


	if( nCommand == EVENT_BUTTON_CLICKED )
	{
	}
}
