#include "StdAfx.h"
#include "DnMissionDailyQuestDlg.h"
#include "DnMissionListItem.h"
#include "DnGuildTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMissionDailyQuestDlg::CDnMissionDailyQuestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pListBoxEx(NULL)
, m_pTreeCtrl(NULL)
{
	m_LastSelectType = CDnMissionTask::Daily;
}

CDnMissionDailyQuestDlg::~CDnMissionDailyQuestDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnMissionDailyQuestDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX0" );
	m_pTreeCtrl = GetControl<CEtUITreeCtl>("ID_TREE01");
}

void CDnMissionDailyQuestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MissionSubTabDlg.ui" ).c_str(), bShow );
}

void CDnMissionDailyQuestDlg::RefreshSubCategory()
{
	m_pListBoxEx->RemoveAllItems();
	m_pTreeCtrl->DeleteAllItems();

	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();

	CTreeItem *pTreeItemDaily = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9740 ) );
	pTreeItemDaily->SetItemValueInt( CDnMissionTask::Daily );

	CTreeItem *pTreeItemWeekly = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 9741 ) );
	pTreeItemWeekly->SetItemValueInt( CDnMissionTask::Weekly );

#ifdef PRE_REMOVE_GUILD_WAR_UI
	CTreeItem *pTreeItemGuild = NULL;
	if( CDnGuildTask::IsActive() && GetGuildTask().GetCurrentGuildWarEventStep() == GUILDWAR_STEP_TRIAL )
	{
		pTreeItemGuild = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126187 ) );
		pTreeItemGuild->SetItemValueInt( CDnMissionTask::GuildWar );
	}

	if( CDnGuildTask::IsActive() && GetGuildTask().GetCurrentGuildWarEventStep() != GUILDWAR_STEP_TRIAL && m_LastSelectType == CDnMissionTask::GuildWar )
		m_LastSelectType = CDnMissionTask::Daily;
#endif // PRE_REMOVE_GUILD_WAR_UI

	CTreeItem * pTreeItemWeekendEvent = NULL;
	DWORD dwWeekendEventCount = pTask->GetDailyMissionCount( CDnMissionTask::WeekendEvent );
	if( 0 != dwWeekendEventCount )
	{
		pTreeItemWeekendEvent = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10077 ) );
		pTreeItemWeekendEvent->SetItemValueInt( CDnMissionTask::WeekendEvent );	
	}

	CTreeItem * pTreeItemRepeat = NULL;
	DWORD dwDailyCount = pTask->GetDailyMissionCount( CDnMissionTask::WeekendRepeat );
	if( 0 != dwDailyCount )
	{
		pTreeItemRepeat = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10079 ) );
		pTreeItemRepeat->SetItemValueInt( CDnMissionTask::WeekendRepeat );	
	}

#ifdef PRE_ADD_MONTHLY_MISSION
	CTreeItem * pTreeItemMonthly = NULL;
	DWORD dwMonthlyCount = pTask->GetDailyMissionCount( CDnMissionTask::MonthlyEvent );
	if( 0 != dwMonthlyCount )
	{
		pTreeItemMonthly = m_pTreeCtrl->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 10167 ) );
		pTreeItemMonthly->SetItemValueInt( CDnMissionTask::MonthlyEvent );
	}
#endif 

	m_pTreeCtrl->ExpandAll();
	switch( m_LastSelectType )
	{
		case CDnMissionTask::Daily: m_pTreeCtrl->SetSelectItem( pTreeItemDaily ); break;
		case CDnMissionTask::Weekly: m_pTreeCtrl->SetSelectItem( pTreeItemWeekly ); break;
#ifdef PRE_REMOVE_GUILD_WAR_UI
		case CDnMissionTask::GuildWar: if( pTreeItemGuild ) m_pTreeCtrl->SetSelectItem( pTreeItemGuild ); break;
#endif // PRE_REMOVE_GUILD_WAR_UI
		case CDnMissionTask::WeekendEvent:	if( pTreeItemWeekendEvent ) m_pTreeCtrl->SetSelectItem( pTreeItemWeekendEvent ); break;
		case CDnMissionTask::WeekendRepeat:	if( pTreeItemRepeat ) m_pTreeCtrl->SetSelectItem( pTreeItemRepeat ); break;
#ifdef PRE_ADD_MONTHLY_MISSION
		case CDnMissionTask::MonthlyEvent:	if( pTreeItemMonthly ) m_pTreeCtrl->SetSelectItem( pTreeItemMonthly ); break;	
#endif 
	}
	RefreshList( m_LastSelectType );
}

void CDnMissionDailyQuestDlg::RefreshList( CDnMissionTask::DailyMissionTypeEnum Type )
{
	m_pListBoxEx->RemoveAllItems();

	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();

	std::vector<CDnMissionTask::MissionInfoStruct *> pVecList[2];
	DWORD dwCount = pTask->GetDailyMissionCount( Type );
	for( DWORD i=0; i<dwCount; i++ ) {
		CDnMissionTask::MissionInfoStruct *pInfo = pTask->GetDailyMissionInfo( Type, i );
		if( !pInfo ) continue;
		if( Type == CDnMissionTask::GuildWar )	// Guild missions repeat infinitely, so always reset Achieve and nCounter
		{
			// pInfo->bAchieve = false; // Design intent changed related to #47709.
			if( pInfo->nRewardCounter == 1 )
				pInfo->nCounter = 0;
		}
		pVecList[pInfo->bAchieve].push_back( pInfo );
	}

#ifdef PRE_MOD_MISSION_HELPER
	bool bFull = pTask->IsMissionNotifierFull();
#endif
	for( int i=0; i<2; i++ ) {
		for( DWORD j=0; j<pVecList[i].size(); j++ ) {
			CDnMissionListItem *pItem = m_pListBoxEx->AddItem<CDnMissionListItem>();
#ifdef PRE_MOD_MISSION_HELPER
			pItem->SetParentListBoxExControl( m_pListBoxEx );
#else
			pItem->SetParentDailyDialog( this );
#endif
			pItem->SetInfo( pVecList[i][j] );
#ifdef PRE_MOD_MISSION_HELPER
			if( bFull && pItem->IsChecked() == false ) pItem->EnableCheckBox( false );
#else
			int nArrayIndex = pItem->GetItemID();
			if( nArrayIndex >= 0 && CDnMissionTask::GetInstance().IsRegisterNotifier( Type, nArrayIndex ) ) 
			{
				pItem->SetCheckBox( true );
			}
#endif
		}
	}
	m_LastSelectType = Type;
}

void CDnMissionDailyQuestDlg::SelectMissionItemFromNotifier( int nMissionItemID )
{
	if( nMissionItemID == -1 )
		return;

	int nSize = m_pListBoxEx->GetSize();
	for( int i = 0; i < nSize; i++)  {
		CDnMissionListItem *pListItem = m_pListBoxEx->GetItem< CDnMissionListItem > ( i );
		if( nMissionItemID == pListItem->GetItemID() )  {
			m_pListBoxEx->SelectItem( i );
		}
	}
}

#ifdef PRE_MOD_MISSION_HELPER
#else
void CDnMissionDailyQuestDlg::InvertCheckOthers( CDnMissionListItem *pMissionListItem )
{
	if( CDnMissionTask::IsActive() )  {
		ASSERT( pMissionListItem->GetItemID() >= 0 );
		CDnMissionTask::GetInstance().RegisterNotifier( m_LastSelectType, pMissionListItem->GetItemID() );
	}

	int nSize = m_pListBoxEx->GetSize();
	for( int i = 0; i < nSize; i++)  {
		CDnMissionListItem *pListItem = m_pListBoxEx->GetItem< CDnMissionListItem > ( i );
		if( pMissionListItem != pListItem )  {
			pListItem->SetCheckBox( false );
		}
	}
}
#endif

void CDnMissionDailyQuestDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
	{
		RefreshSubCategory();
	}
	else
	{
		m_pListBoxEx->RemoveAllItems();
		m_pTreeCtrl->DeleteAllItems();
	}

	CEtUIDialog::Show( bShow );
}

void CDnMissionDailyQuestDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_TREECONTROL_SELECTION ) {
		if( IsCmdControl("ID_TREE01" ) ) {
			CTreeItem *pItem = m_pTreeCtrl->GetSelectedItem();
			if( pItem ) {
				
				RefreshList( (CDnMissionTask::DailyMissionTypeEnum)pItem->GetItemValueInt() );
			}
		}
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
