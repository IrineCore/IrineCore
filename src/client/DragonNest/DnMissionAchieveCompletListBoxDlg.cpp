#include "StdAfx.h"
#include "DnMissionAchieveCompletListBoxDlg.h"
#include "DnMissionTask.h"
#include "DnMissionListItem.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif


#ifdef PRE_MOD_APPELLATIONBOOK_RENEWAL


CDnMissionAchieveCompletListBoxDlg::CDnMissionAchieveCompletListBoxDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
: CEtUIDialog( dialogType, pParentDialog, nID, pCallback, true )
, m_pListBoxEx(NULL)
{
}

CDnMissionAchieveCompletListBoxDlg::~CDnMissionAchieveCompletListBoxDlg(void)
{
	m_pListBoxEx->RemoveAllItems();
}

void CDnMissionAchieveCompletListBoxDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MissionTiltle_CompleteDlg.ui" ).c_str(), bShow );
}

void CDnMissionAchieveCompletListBoxDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_COMPLETETITLE");
	m_pListBoxEx->Show(false);
	m_pListBoxEx->SetRenderSelectBar( false );
	m_pListBoxEx->SetRenderScrollBar( false );
}

void CDnMissionAchieveCompletListBoxDlg::Show( bool bShow )
{
	if( bShow == IsShow() )
		return;

	if( bShow )
	{
		m_pListBoxEx->Show(bShow);
		RefreshInfo();
	}
	else
	{
		m_pListBoxEx->Show(bShow);
		m_pListBoxEx->RemoveAllItems();
	}

	CEtUIDialog::Show( bShow );
}

void CDnMissionAchieveCompletListBoxDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	
	if(nCommand == EVENT_LISTBOX_SELECTION)
	{
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnMissionAchieveCompletListBoxDlg::RefreshInfo()
{
	if(!CDnMissionTask::IsActive()) 
		return;

	CDnMissionTask::MissionInfoStruct *pStruct;
	CDnMissionTask *pTask = &CDnMissionTask::GetInstance();

	// Update mission list
	m_pListBoxEx->RemoveAllItems();
	for( int i = 0; i < LASTMISSIONACHIEVEMAX; i++ ) 
	{
		int nArrayIndex = pTask->GetLastUpdateAchieveIndex( i );
		if( nArrayIndex == -1 ) continue;

		pStruct = pTask->GetMissionFromArrayIndex( nArrayIndex );
		if( !pStruct ) continue;

		CDnMissionListItem *pItem = m_pListBoxEx->AddItem<CDnMissionListItem>();
		pItem->SetInfo( pStruct );
#ifdef PRE_MOD_MISSION_HELPER
		pItem->ShowCheckBox( false );
#endif
	}
}

#endif