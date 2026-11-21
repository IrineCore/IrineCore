#include "StdAfx.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnCharSelectCostumePreviewDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM

CDnCharSelectCostumePreviewDlg::CDnCharSelectCostumePreviewDlg( UI_DIALOG_TYPE dialogType , CEtUIDialog *pParentDialog , int nID , CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback ) 
, m_pRevertMyCostumeButton(NULL)
, m_pShowCashCostumeButton(NULL)
, m_nLastClickRadioBtnID(-1)
{
}

CDnCharSelectCostumePreviewDlg::~CDnCharSelectCostumePreviewDlg()
{
}

void CDnCharSelectCostumePreviewDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "CharCreate_CostumeDlg.ui" ).c_str(), bShow );
}

void CDnCharSelectCostumePreviewDlg::InitialUpdate()
{
	m_pRevertMyCostumeButton = GetControl<CEtUIRadioButton>("ID_RBT_COSTUME0");
	m_pRevertMyCostumeButton->SetChecked(true);

	m_pShowCashCostumeButton = GetControl<CEtUIRadioButton>("ID_RBT_COSTUME1");
	m_pShowCashCostumeButton->SetChecked(false);
}

void CDnCharSelectCostumePreviewDlg::Show( bool bShow )
{
	CEtUIDialog::Show( bShow );
}

void CDnCharSelectCostumePreviewDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	
	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) return;

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
	}
	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( strstr( pControl->GetControlName(),"ID_RBT_COSTUME" ) )
		{
			int nCurrentRadioBtnID = static_cast<CEtUIRadioButton*>(pControl)->GetTabID();
			if(m_nLastClickRadioBtnID != nCurrentRadioBtnID) // Prevent duplicate clicks
			{
				m_nLastClickRadioBtnID = nCurrentRadioBtnID;
				pTask->ShowPreviewCashCostume(nCurrentRadioBtnID);
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnCharSelectCostumePreviewDlg::OnUICallbackProc( int nID, UINT nCommand, CEtUIControl *pControl, UINT uiMsg )
{
	SetCmdControlName( pControl->GetControlName() );
}

void CDnCharSelectCostumePreviewDlg::ResetRadioButton()
{
	m_pRevertMyCostumeButton->SetChecked( true, true, false );
	m_pShowCashCostumeButton->SetChecked( false, true, false );
}

#endif