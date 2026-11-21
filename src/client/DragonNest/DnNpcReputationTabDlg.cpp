#include "StdAfx.h"
#include "DnNpcReputationTabDlg.h"
#include "DnNpcReputationDlg.h"
#include "DnNpcReputationAlbumDlg.h"
#include "DnNpcReputationBromideDlg.h"
#include "DnRepUnionDlg.h"
#include "DnQuestTask.h"
#include "TaskManager.h"
#include "ReputationSystemRepository.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#include "DnMainMenuDlg.h"
#endif

CDnNpcReputationTabDlg::CDnNpcReputationTabDlg( UI_DIALOG_TYPE dialogType/* = UI_TYPE_FOCUS*/, CEtUIDialog *pParentDialog/* = NULL*/, 
											   int nID/* = -1*/, CEtUICallback *pCallback/* = NULL*/ ) 
											   : CEtUITabDialog( dialogType, pParentDialog, nID, pCallback, true ),
											   m_pBtnListTab( NULL ),
											   m_pBtnAlbumTab( NULL ),
											   m_pNpcListDlg( NULL ),
											   m_pNpcAlbumDlg( NULL ),
											   m_pBromideDlg( NULL )
											   , m_pUnionDlg(NULL)
											   , m_pBtnUnionTab(NULL)
{
}

CDnNpcReputationTabDlg::~CDnNpcReputationTabDlg( void )
{
	SAFE_DELETE( m_pBromideDlg );
}

void CDnNpcReputationTabDlg::Initialize( bool bShow )
{
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	CEtUITabDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Tab_ReputeDlg.ui" ).c_str(), bShow );
#else
	CEtUITabDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "ReputeDlg.ui" ).c_str(), bShow );
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

	

}

void CDnNpcReputationTabDlg::InitialUpdate( void )
{
	m_pBtnListTab = GetControl<CEtUIRadioButton>("ID_RBT_LIST");
	m_pBtnListTab->EnableDragMode( false );

	m_pNpcListDlg = new CDnNpcReputationDlg( UI_TYPE_CHILD, this );
	m_pNpcListDlg->Initialize( false );
	AddTabDialog( m_pBtnListTab, m_pNpcListDlg );

	m_pBtnAlbumTab = GetControl<CEtUIRadioButton>("ID_RBT_ALBUM");
	m_pBtnAlbumTab->EnableDragMode( false );

	m_pNpcAlbumDlg = new CDnNpcReputationAlbumDlg( UI_TYPE_CHILD, this );
	m_pNpcAlbumDlg->Initialize( false );
	AddTabDialog( m_pBtnAlbumTab, m_pNpcAlbumDlg );

	SetCheckedTab( m_pBtnListTab->GetTabID() );

	// Not added to tabs because it is a child dialog shown only when clicking a picture in the album.
	m_pBromideDlg = new CDnNpcReputationBromideDlg( /*UI_TYPE_CHILD, this*/ );
	m_pBromideDlg->Initialize( false );

	m_pNpcAlbumDlg->SetBromideDlg( m_pBromideDlg );
	m_pBromideDlg->SetAlbumDlg( m_pNpcAlbumDlg );

	m_pBtnUnionTab = GetControl<CEtUIRadioButton>("ID_RBT_GROUP");
	m_pBtnUnionTab->EnableDragMode(false);

	m_pUnionDlg = new CDnRepUnionDlg(UI_TYPE_CHILD, this);
	m_pUnionDlg->Initialize(false);
	AddTabDialog(m_pBtnUnionTab, m_pUnionDlg);
}

void CDnNpcReputationTabDlg::Show( bool bShow )
{
	if( IsShow() == bShow )
		return;

	CEtUITabDialog::Show( bShow );

	// Not included in tabs, so must be called together during show handling.
	if( m_pBromideDlg->IsShow() )
		m_pBromideDlg->Show( bShow );

	if (m_pCallback)
		m_pCallback->OnUICallbackProc(GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL);

	if (IsShow())
	{
		CDnQuestTask* pQuestTask = static_cast<CDnQuestTask*>(CTaskManager::GetInstance().GetTask( "QuestTask" ));	
		if (pQuestTask)
		{
			CReputationSystemRepository* pReputationRepos = pQuestTask->GetReputationRepository();
			if (pReputationRepos)
			{
				int lastUpdateNpcId = pReputationRepos->GetLastUpdateNpcID();
				if (lastUpdateNpcId > 0)
				{
					OnSelectNpcInUnionDlg(lastUpdateNpcId);
					pReputationRepos->SetLastUpdateNpcID(-1);
				}
			}
		}
	}
}

void CDnNpcReputationTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg /* = 0 */ )
{
	SetCmdControlName( pControl->GetControlName() );

	// If the affinity list dialog button tab is selected, hide the album dialog if it is open.
	if ((m_pBtnListTab == pControl) || (m_pBtnUnionTab == pControl))
	{
		if( m_pBromideDlg && m_pBromideDlg->IsShow() )
			m_pBromideDlg->Show( false );
	}

	if( nCommand == EVENT_BUTTON_CLICKED )
	{
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_REPUTATION);
		}
#endif
	}

	CEtUITabDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnNpcReputationTabDlg::OnSelectNpcInUnionDlg(int npcId)
{
	if (m_pBtnListTab && m_pNpcListDlg)
	{
		SetCheckedTab( m_pBtnListTab->GetTabID() );
		ProcessCommand(EVENT_RADIOBUTTON_CHANGED, true, m_pBtnListTab, 0);
		m_pNpcListDlg->SetNpcPage(npcId);
	}
}