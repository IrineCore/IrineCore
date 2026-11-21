#include "StdAfx.h"
#include "DnMissionAchieveEventListBoxDlg.h"
#include "DnTableDB.h"
#include "DNTableFile.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnMissionAchieveEventListBoxDlg::CDnMissionAchieveEventListBoxDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pListBoxCompleteTitle( NULL )
{
}

CDnMissionAchieveEventListBoxDlg::~CDnMissionAchieveEventListBoxDlg()
{
	ClearList();
}

void CDnMissionAchieveEventListBoxDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "MissionTiltle_CompleteDlg.ui" ).c_str(), bShow );
	LoadMissionEventTable();
}

void CDnMissionAchieveEventListBoxDlg::InitialUpdate()
{
	m_pListBoxCompleteTitle = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_COMPLETETITLE" );
}

void CDnMissionAchieveEventListBoxDlg::LoadMissionEventTable()
{
	m_mapMissionEventGroups.clear();
	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TMISSIONEVENT );
	if( !pSox )
		return;

	int nCount = pSox->GetItemCount();
	for( int i = 0; i < nCount; ++i )
	{
		int nRowID = pSox->GetItemID( i );
		int nEventGroup = 0;
		int nScheduleGroup = 0;
		if( pSox->GetFieldNum( "_MissionEventGroup" ) >= 0 )
		{
			DNTableCell *pCell = pSox->GetFieldFromLablePtr( nRowID, "_MissionEventGroup" );
			if( pCell ) nEventGroup = pCell->GetInteger();
		}
		if( pSox->GetFieldNum( "_ScheduleEventGroup" ) >= 0 )
		{
			DNTableCell *pCell = pSox->GetFieldFromLablePtr( nRowID, "_ScheduleEventGroup" );
			if( pCell ) nScheduleGroup = pCell->GetInteger();
		}
		if( nEventGroup > 0 )
			m_mapMissionEventGroups[nEventGroup] = nScheduleGroup;
	}
}

void CDnMissionAchieveEventListBoxDlg::ClearList()
{
	if( m_pListBoxCompleteTitle )
		m_pListBoxCompleteTitle->RemoveAllItems();
}

bool CDnMissionAchieveEventListBoxDlg::IsEventMissionGroup( int nGroupID ) const
{
	return m_mapMissionEventGroups.find( nGroupID ) != m_mapMissionEventGroups.end();
}
