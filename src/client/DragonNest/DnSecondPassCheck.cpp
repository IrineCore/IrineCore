#include "StdAfx.h"
#include "DnSecondPassCheck.h"
#include "DnInterface.h"
#include "DnChannelListDlg.h"
#include "DnLoginTask.h"
#include "TaskManager.h"
#include "DnAuthTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnSecondPassCheck::CDnSecondPassCheck( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback )
{
	m_pCheckBox = NULL;
	m_pProgressBar = NULL;
}

CDnSecondPassCheck::~CDnSecondPassCheck(void)
{
}

void CDnSecondPassCheck::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "2ndPassCheck.ui" ).c_str(), bShow );

}

void CDnSecondPassCheck::InitialUpdate()
{
	// m_pProgressBar = GetControl<CEtUIProgressBar>("ID_PROGRESSBAR_START") ; 
	m_pCheckBox =  GetControl<CEtUICheckBox>("ID_CHECKBOX_OFF");
#ifdef PRE_ADD_SECOND_SETCHECK
	m_pCheckBox->Show(true);
#else 
	m_pCheckBox->Show(false);
	GetControl<CEtUIStatic>("ID_STATIC2")->Show(false);
#endif 
}

void CDnSecondPassCheck::Show( bool bShow )
{
	//m_fTimer = m_MaxTimer = 5.0f ;
#if defined(PRE_ADD_23829)
	m_pCheckBox->Show(true);
	GetControl<CEtUIStatic>("ID_STATIC2")->Show(true);	
#endif

	m_pCheckBox->SetChecked(false);

	CEtUIDialog::Show(bShow );
}

void CDnSecondPassCheck::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_BUTTON_CLICKED || 
		nCommand == EVENT_CHECKBOX_CHANGED )
	{
		if(pControl == m_pCheckBox )
		{
		}
		else 	
			if( IsCmdControl( "ID_BUTTON_PASSCREATE" ) )
			{
#if defined(PRE_ADD_23829)
				SecondPassCheck(m_pCheckBox ? m_pCheckBox->IsChecked() : false);
#endif // PRE_ADD_23829
				GetInterface().OpenSecurityCreateDlg();
#ifdef PRE_ADD_SETCHECK
				SetCheckShow( m_pCheckBox->IsChecked());
#endif 
				Show( false );

			}
			else if( IsCmdControl( "ID_BUTTON_START" ) )
			{
#if defined(PRE_ADD_23829)
				SecondPassCheck(m_pCheckBox ? m_pCheckBox->IsChecked() : false);
#endif // PRE_ADD_23829

#ifdef PRE_MDD_USA_SECURITYNUMPAD
				// For US, secondary password entry is mandatory. 
				Show( false );
#else
				CDnLoginTask *pLoginTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask("LoginTask");
				int nValue[4] = { 0, };
				pLoginTask->SetAuthPassword( 0, nValue );
				pLoginTask->ChangeState(CDnLoginTask::LoginStateEnum::ChannelList);
				//SelectChannel();

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
				pLoginTask->ResetPreviewCashCostume();
#endif

#ifdef PRE_ADD_SETCHECK
				SetCheckShow( m_pCheckBox->IsChecked());
#endif 

#endif // PRE_MDD_USA_SECURITYNUMPAD
				Show( false );	
			}
			else if( IsCmdControl("ID_BUTTON_CLOSE"))
			{
#ifdef PRE_ADD_SETCHECK
				SetCheckShow( m_pCheckBox->IsChecked());
#endif 

#if defined(PRE_ADD_23829)
				SecondPassCheck(m_pCheckBox ? m_pCheckBox->IsChecked() : false);
#endif // PRE_ADD_23829

				Show( false);

#ifdef PRE_ADD_CHARSELECT_PREVIEW_CASHITEM
				GetInterface().ResetPreviewCostumeRadioButton();
#endif
			}

	}


	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
void CDnSecondPassCheck::SetCheckShow(bool bCheck)
{
#ifdef PRE_ADD_SETCHECK
	if( bCheck )
	{
		CDnAuthTask *pTask = (CDnAuthTask*)CTaskManager::GetInstance().GetTask("AuthTask");
		if(pTask != NULL)
		{
			pTask->RequestAuthNotifyDate();
		}
	}
#endif 

}

void CDnSecondPassCheck::SelectChannel()
{
	CDnLoginTask *pTask = (CDnLoginTask *)CTaskManager::GetInstance().GetTask( "LoginTask" );
	if( !pTask ) return;

	pTask->SelectChannel(m_ChannelID);

}

void CDnSecondPassCheck::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime ); 

	if(!m_bShow)
		return;
	/*
	m_fTimer -= fElapsedTime;

	if( m_fTimer <= 0.f ) 
	{
	m_fTimer = 0.f;
	Show( false );
	SelectChannel();
	}

	m_pProgressBar->SetProgress( m_fTimer*(100.f/m_MaxTimer)  );
	*/

}

#if defined(PRE_ADD_23829)
void CDnSecondPassCheck::SecondPassCheck(bool bCheck)
{
	CDnAuthTask *pTask = (CDnAuthTask*)CTaskManager::GetInstance().GetTask("AuthTask");
	if(pTask != NULL)
	{
		pTask->SetIgnore2ndPassNotify(bCheck);
	}
}
#endif // PRE_ADD_23829
