#include "StdAfx.h"
#include "DnQuestTabDlg.h"
#include "DnQuestDlg.h"
#include "TaskManager.h"
#include "DnMainMenuDlg.h"
#include "DnTableDB.h"
#include "GameOption.h"
#include "DnLocalPlayerActor.h"
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
#include "DnSystemDlg.h"
#endif
#ifdef PRE_ADD_REMOTE_QUEST
#include "DnMainDlg.h"
#endif // PRE_ADD_REMOTE_QUEST


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnQuestTabDlg::CDnQuestTabDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CEtUITabDialog( dialogType, pParentDialog, nID, pCallback, true )
	, m_pMainQuestDlg(NULL)
	, m_pSubQuestDlg(NULL)
	, m_pMainTabButton(NULL)
	, m_pSubTabButton(NULL)
	, m_nLocalLevel( -1 )
	, m_pPeriodQuestDlg( NULL )
	, m_pPeriodTabButton( NULL )
#ifdef PRE_ADD_QUEST_SORT_BY_JOURNAL_MAP
	, m_pStaticQuestCount( NULL )
#endif
#ifdef PRE_ADD_REMOTE_QUEST
	, m_pRemoteTabButton( NULL )
	, m_pRemoteQuestDlg( NULL )
#endif // PRE_ADD_REMOTE_QUEST
{
}

CDnQuestTabDlg::~CDnQuestTabDlg(void)
{

}

void CDnQuestTabDlg::Initialize(bool bShow)
{
#if defined(PRE_ADD_INTEGERATE_QUEST_REPUT)
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "Tab_QuestTabDlg.ui" ).c_str(), bShow );
#else
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestTabDlg.ui" ).c_str(), bShow );
#endif // PRE_ADD_INTEGERATE_QUEST_REPUT

	if( CDnQuestTask::IsActive() )
		GetQuestTask().SetQuestDialog( this );
}

void CDnQuestTabDlg::InitialUpdate()
{
	m_pMainTabButton = GetControl<CEtUIRadioButton>("ID_TAB_MAIN");
	m_pMainQuestDlg = new CDnQuestDlg( UI_TYPE_CHILD, this );
	m_pMainQuestDlg->SetQuestType( CDnQuestDlg::typeMain );
	m_pMainQuestDlg->Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestMainDlg.ui" ).c_str(), false );
	AddTabDialog( m_pMainTabButton, m_pMainQuestDlg );

	m_pSubTabButton = GetControl<CEtUIRadioButton>("ID_TAB_SUB");
	m_pSubQuestDlg = new CDnQuestDlg( UI_TYPE_CHILD, this );
	m_pSubQuestDlg->SetQuestType( CDnQuestDlg::typeSub );
	m_pSubQuestDlg->Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSubDlg.ui" ).c_str(), false );
	AddTabDialog( m_pSubTabButton, m_pSubQuestDlg );
	
	/// Identical to the sub quest dialog, but created separately to distinguish them.
	m_pPeriodTabButton = GetControl<CEtUIRadioButton>( "ID_TAB_DAILY" );
	m_pPeriodQuestDlg = new CDnQuestDlg( UI_TYPE_CHILD, this );
	m_pPeriodQuestDlg->SetQuestType( CDnQuestDlg::typePeriod );
	m_pPeriodQuestDlg->Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSubDlg.ui" ).c_str(), false );
	AddTabDialog( m_pPeriodTabButton, m_pPeriodQuestDlg );

#ifdef PRE_ADD_REMOTE_QUEST
	m_pRemoteTabButton = GetControl<CEtUIRadioButton>( "ID_TAB_QUEST" );
	m_pRemoteTabButton->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 1200019 ) );
	m_pRemoteTabButton->Show( true );
	m_pRemoteQuestDlg = new CDnQuestDlg( UI_TYPE_CHILD, this );
	m_pRemoteQuestDlg->SetQuestType( CDnQuestDlg::typeRemote );
	m_pRemoteQuestDlg->Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestSubDlg.ui" ).c_str(), false );
	AddTabDialog( m_pRemoteTabButton, m_pRemoteQuestDlg );
#endif // PRE_ADD_REMOTE_QUEST

#ifdef PRE_ADD_QUEST_SORT_BY_JOURNAL_MAP
	m_pStaticQuestCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
#endif

	SetCheckedTab( m_pMainTabButton->GetTabID() );

	m_SmartMoveEx.SetControl( m_pMainTabButton );
}

void CDnQuestTabDlg::Show( bool bShow )
{ 
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		m_SmartMoveEx.MoveCursor();
#ifdef PRE_ADD_REMOTE_QUEST
		CDnMainDlg *pMainBarDlg = GetInterface().GetMainBarDialog();
		if( pMainBarDlg && pMainBarDlg->IsShowQuestNotify() ) 
		{
			std::vector<CDnQuestTask::stRemoteQuestAsk>& vecRemoteQuestAskList = GetQuestTask().GetRemoteQuestAskList();
			int nRemoteQuestAskListCount = static_cast<int>( vecRemoteQuestAskList.size() );
			if( nRemoteQuestAskListCount > 0 )
				ShowQuestFromNotifier( 0, vecRemoteQuestAskList[nRemoteQuestAskListCount-1].nQuestIndex, false );

			pMainBarDlg->ShowQuestNotify( false );
			pMainBarDlg->CloseBlinkMenuButton( CDnMainMenuDlg::QUEST_DIALOG );
		}
#endif // PRE_ADD_REMOTE_QUEST
	}

	CEtUITabDialog::Show( bShow );

	if( m_pCallback )
	{
		m_pCallback->OnUICallbackProc( GetDialogID(), bShow?EVENT_DIALOG_SHOW:EVENT_DIALOG_HIDE, NULL );
	}
}

void CDnQuestTabDlg::Process( float fElapsedTime )
{
	if( CDnActor::s_hLocalActor )
	{
		CDnLocalPlayerActor *pLocalActor = dynamic_cast<CDnLocalPlayerActor *>(CDnLocalPlayerActor::s_hLocalActor.GetPointer());
		if( pLocalActor )
		{
			if( pLocalActor->GetLevel() != m_nLocalLevel )
			{
				m_nLocalLevel = pLocalActor->GetLevel();
				RefreshQuest();
			}
		}
	}

	m_SmartMoveEx.Process();
	CEtUITabDialog::Process( fElapsedTime );
}

void CDnQuestTabDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_CHECKBOX_CHANGED )
	{		
	}
	else if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( IsCmdControl( "ID_BT_CANCEL" ) || IsCmdControl("ID_BT_CLOSE") ) 
		{
			Show( false );
		}
#ifdef PRE_ADD_SHORTCUT_HELP_DIALOG
		else if( IsCmdControl("ID_BT_SMALLHELP") )
		{
			CDnMainMenuDlg* pMainMenuDlg = GetInterface().GetMainMenuDialog();
			CDnSystemDlg* pSystemDlg = pMainMenuDlg->GetSystemDlg();
			if(pSystemDlg)
				pSystemDlg->ShowChoiceHelpDlg(HELP_SHORTCUT_QUEST);
		}
#endif
	}

	CEtUITabDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnQuestTabDlg::AddQuest( const TQuest *pQuest )
{
	ASSERT(pQuest&&"CDnQuestTabDlg::AddQuest");

	CDnQuestDlg *pQuestDlg = GetQuestDialog(static_cast<EnumQuestType>(g_DataManager.GetQuestType(pQuest->nQuestID)));
	if( !pQuestDlg ) return;

	pQuestDlg->AddQuest( pQuest );
}

void CDnQuestTabDlg::DeleteQuest( const TQuest *pQuest )
{
	ASSERT(pQuest&&"CDnQuestTabDlg::DeleteQuest");

	DNTableFileFormat* pSox = GetDNTable( CDnTableDB::TQUEST);
	int nQuestTypeIndex = -1; // Quest type
	nQuestTypeIndex = pSox->GetFieldFromLablePtr( pQuest->nQuestID, "_QuestType" )->GetInteger();

	if(nQuestTypeIndex == -1)
	{
		ErrorLog("Quest Cancle Failed %d", pQuest->nQuestID );
		return;
	}
    // Currently 2 quest types are used, so handle appropriately if more are added later.
	if(nQuestTypeIndex == EnumQuestType::QuestType_MainQuest)
	{
		if(!CDnQuestTask::GetInstance().IsClearQuest( pQuest->nQuestID )) // Cannot delete a cleared quest!
			m_pMainQuestDlg->DeleteQuest(pQuest);
	}
	EnumQuestType eType = (EnumQuestType)(g_DataManager.GetQuestType(pQuest->nQuestID));

	if( QuestType_SubQuest == eType )
		m_pSubQuestDlg->DeleteQuest(pQuest);
	else if( QuestType_PeriodQuest == eType )
		m_pPeriodQuestDlg->DeleteQuest(pQuest);
#ifdef PRE_ADD_REMOTE_QUEST
	else if( QuestType_RemoteQuest == eType )
		m_pRemoteQuestDlg->DeleteQuest( pQuest );
#endif // PRE_ADD_REMOTE_QUEST
}

void CDnQuestTabDlg::AddJournal( const TQuest *pQuest, bool bAll )
{
	ASSERT(pQuest&&"CDnQuestTabDlg::AddJournal");

	CDnQuestDlg *pQuestDlg = GetQuestDialog(static_cast<EnumQuestType>(g_DataManager.GetQuestType(pQuest->nQuestID)));
	if( !pQuestDlg ) return;

	pQuestDlg->AddJournal( pQuest, bAll );
}

void CDnQuestTabDlg::CompleteQuest( const TQuest *pQuest )
{
	ASSERT(pQuest&&"CDnQuestTabDlg::CompleteQuest");

	CDnQuestDlg *pQuestDlg = GetQuestDialog(static_cast<EnumQuestType>(g_DataManager.GetQuestType(pQuest->nQuestID)));
	if( !pQuestDlg ) return;

	pQuestDlg->CompleteQuest( pQuest );
}

void CDnQuestTabDlg::RefreshJournal( const TQuest *pQuest )
{
	ASSERT(pQuest&&"CDnQuestTabDlg::RefreshJournal");

	CDnQuestDlg *pQuestDlg = GetQuestDialog(static_cast<EnumQuestType>(g_DataManager.GetQuestType(pQuest->nQuestID)));
	if( !pQuestDlg ) return;

	pQuestDlg->RefreshJournal( pQuest );
}

void CDnQuestTabDlg::RefreshQuest()
{
	CDnQuestTask *pTask = (CDnQuestTask*)CTaskManager::GetInstance().GetTask( "QuestTask" );
	if( !pTask ) return;

	m_pMainQuestDlg->Clear();
	m_pMainQuestDlg->RefreshCompleteMainQuest();
	m_pMainQuestDlg->RefreshProgMainQuest();

	m_pSubQuestDlg->Clear();
	m_pSubQuestDlg->RefreshCompleteSubQuest();
	m_pSubQuestDlg->RefreshProgSubQuest();

	m_pPeriodQuestDlg->Clear();
	m_pPeriodQuestDlg->RefreshCompletePeriodQuest();
	m_pPeriodQuestDlg->RefreshProgPeriodQuest();

#ifdef PRE_ADD_REMOTE_QUEST
	m_pRemoteQuestDlg->Clear();
	m_pRemoteQuestDlg->RefreshCompleteRemoteQuest();
	m_pRemoteQuestDlg->RefreshProgRemoteQuest();
	SetCheckedTab( GetCurrentTabID() );
#endif // PRE_ADD_REMOTE_QUEST

	RefreshQuestNotifierCheck();
}

CDnQuestDlg* CDnQuestTabDlg::GetQuestDialog( EnumQuestType emQuestType )
{
	switch( emQuestType )
	{
	case QuestType_MainQuest:	return m_pMainQuestDlg;
	case QuestType_SubQuest:	return m_pSubQuestDlg;
	case QuestType_PeriodQuest: return m_pPeriodQuestDlg;
#ifdef PRE_ADD_REMOTE_QUEST
	case QuestType_RemoteQuest:	return m_pRemoteQuestDlg;
#endif // PRE_ADD_REMOTE_QUEST
	}

	return NULL;
}

void CDnQuestTabDlg::RefreshSummaryInfo()
{
	CDnMainMenuDlg *pMainMenuDlg = GetInterface().GetMainMenuDialog();
	if( pMainMenuDlg ) 
	{
		pMainMenuDlg->RefreshQuestSumInfo();
#ifdef PRE_ADD_QUEST_SORT_BY_JOURNAL_MAP
		if( m_pStaticQuestCount )
		{
			int nQuestCount = GetQuestTask().GetQuestCount();
			m_pStaticQuestCount->SetText( FormatW( L"%d/%d" , nQuestCount , MAX_PLAY_QUEST ).c_str() );
		}
#endif
	}
}

void CDnQuestTabDlg::ShowQuestFromNotifier( int nChapterIndex, int nQuestIndex, bool bMainQuest )
{
	if( bMainQuest ) 
	{
		SetCheckedTab( m_pMainTabButton->GetTabID() );
		m_pMainQuestDlg->CollapseAll();
		CEtUIQuestTreeItem *pJournalItem = m_pMainQuestDlg->Expand( nChapterIndex, nQuestIndex );
		m_pMainQuestDlg->SetSelectItem( pJournalItem );
		m_pMainQuestDlg->OnTreeSelected();
	}

	EnumQuestType eQuestType = (EnumQuestType)(g_DataManager.GetQuestType( nQuestIndex ));
	CDnQuestDlg * pQuestDlg = GetQuestDialog( eQuestType );

	if( NULL == pQuestDlg )
		return;

	if( QuestType_SubQuest == eQuestType )
		SetCheckedTab( m_pSubTabButton->GetTabID() );
	else if( QuestType_PeriodQuest == eQuestType )
		SetCheckedTab( m_pPeriodTabButton->GetTabID() );
#ifdef PRE_ADD_REMOTE_QUEST
	else if( QuestType_RemoteQuest == eQuestType )
		SetCheckedTab( m_pRemoteTabButton->GetTabID() );
#endif // PRE_ADD_REMOTE_QUEST

	pQuestDlg->CollapseAll();
	CEtUIQuestTreeItem *pJournalItem = pQuestDlg->Expand( nChapterIndex, nQuestIndex );
	pQuestDlg->SetSelectItem( pJournalItem );
	pQuestDlg->OnTreeSelected();
}

void CDnQuestTabDlg::RefreshQuestNotifierCheck()
{
	if( m_pMainQuestDlg )
		m_pMainQuestDlg->ImportNotifierToCheckBox();
	if( m_pSubQuestDlg )
		m_pSubQuestDlg->ImportNotifierToCheckBox();
	if( m_pPeriodQuestDlg )
		m_pPeriodQuestDlg->ImportNotifierToCheckBox();
#ifdef PRE_ADD_REMOTE_QUEST
	if( m_pRemoteQuestDlg )
		m_pRemoteQuestDlg->ImportNotifierToCheckBox();
#endif // PRE_ADD_REMOTE_QUEST
}

#ifdef PRE_ADD_REMOTE_QUEST
void CDnQuestTabDlg::RecieveRemoteQuest( int nQuestIndex, CDnQuestTree::eRemoteQuestState remoteQuestState )
{
	CDnQuestDlg* pQuestDlg = GetQuestDialog( static_cast<EnumQuestType>( g_DataManager.GetQuestType( nQuestIndex ) ) );
	if( pQuestDlg )
		pQuestDlg->RecieveRemoteQuest( nQuestIndex, remoteQuestState );
}
#endif // PRE_ADD_REMOTE_QUEST

void CDnQuestTabDlg::SetCheckDailyTab()
{
	if( m_pPeriodTabButton )
		m_pPeriodTabButton->SetChecked( true );
}