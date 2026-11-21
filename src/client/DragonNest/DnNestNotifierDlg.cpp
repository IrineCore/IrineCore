#include "StdAfx.h"
#include "DnNestNotifierDlg.h"
#include "DnNestInfoTask.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include <algorithm>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnNestNotifierListItemDlg
//-----------------------------------------------------------------------------
CDnNestNotifierListItemDlg::CDnNestNotifierListItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStaticName( NULL )
	, m_pStaticCount( NULL )
{
}

CDnNestNotifierListItemDlg::~CDnNestNotifierListItemDlg()
{
}

void CDnNestNotifierListItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyCreateInfoListDlg.ui" ).c_str(), bShow );
}

void CDnNestNotifierListItemDlg::InitialUpdate()
{
	m_pStaticName = GetControl<CEtUIStatic>( "ID_TEXT_NAME" );
	m_pStaticCount = GetControl<CEtUIStatic>( "ID_TEXT_COUNT" );
}

void CDnNestNotifierListItemDlg::SetNestInfo( const WCHAR *wszNestName, int nRemainCount, int nMaxCount )
{
	if( m_pStaticName && wszNestName )
		m_pStaticName->SetText( wszNestName );
	if( m_pStaticCount )
	{
		WCHAR wszBuf[64] = { 0 };
		swprintf_s( wszBuf, _countof( wszBuf ), L"%d / %d", nRemainCount, nMaxCount );
		m_pStaticCount->SetText( wszBuf );
	}
}

//-----------------------------------------------------------------------------
// CDnNestNotifierDlg
//-----------------------------------------------------------------------------
CDnNestNotifierDlg::CDnNestNotifierDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pListBoxEx( NULL )
	, m_pStaticNotice( NULL )
{
}

CDnNestNotifierDlg::~CDnNestNotifierDlg()
{
	ClearNestList();
}

void CDnNestNotifierDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "PartyCreateInfoDlg.ui" ).c_str(), bShow );
}

void CDnNestNotifierDlg::InitialUpdate()
{
	m_pListBoxEx = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );
	m_pStaticNotice = GetControl<CEtUIStatic>( "ID_TEXT_NOTICE" );
}

void CDnNestNotifierDlg::ClearNestList()
{
	if( m_pListBoxEx )
		m_pListBoxEx->RemoveAllItems();
}

void CDnNestNotifierDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( bShow )
		RefreshNestList();

	CDnCustomDlg::Show( bShow );
}

struct SNestNotifierSortEntry
{
	int nEnterConditionID;
	int nRecommendHardLevel;
	int nListSort;
	std::wstring wszMapName;
};

static bool CompareNestNotifierSort( const SNestNotifierSortEntry &a, const SNestNotifierSortEntry &b )
{
	return a.nListSort < b.nListSort;
}

void CDnNestNotifierDlg::RefreshNestList()
{
	ClearNestList();
	if( CDnNestInfoTask::IsActive() )
	{
		CDnNestInfoTask &nestTask = CDnNestInfoTask::GetInstance();
		DWORD dwCount = nestTask.GetNestDungeonCount();
		for( DWORD i = 0; i < dwCount; ++i )
		{
			CDnNestInfoTask::NestDungeonInfoStruct *pInfo = nestTask.GetNestDungeonInfo( i );
			if( !pInfo )
				continue;
			int nRemain = pInfo->nMaxTryCount - pInfo->nTryCount;
			if( nRemain < 0 ) nRemain = 0;
			AddNestItem( pInfo->szMapName.c_str(), nRemain, pInfo->nMaxTryCount );
		}
		if( dwCount > 0 )
			return;
	}

	DNTableFileFormat *pMapSox = GetDNTable( CDnTableDB::TMAP );
	DNTableFileFormat *pEnterSox = GetDNTable( CDnTableDB::TDUNGEONENTER );
	if( !pMapSox || !pEnterSox )
		return;

	std::vector<int> vecMapIDs;
	pMapSox->GetItemIDListFromField( "_MapSubType", 1, vecMapIDs, true );
	pMapSox->GetItemIDListFromField( "_MapSubType", 8, vecMapIDs, false );

	std::vector<SNestNotifierSortEntry> vecEntries;
	for( size_t i = 0; i < vecMapIDs.size(); ++i )
	{
		int nMapID = vecMapIDs[i];
		DNTableCell *pCondCell = pMapSox->GetFieldFromLablePtr( nMapID, "_EnterConditionTableID" );
		int nEnterCondID = pCondCell ? pCondCell->GetInteger() : 0;
		if( nEnterCondID <= 0 || !pEnterSox->IsExistItem( nEnterCondID ) )
			continue;

		bool bDup = false;
		for( size_t j = 0; j < vecEntries.size(); ++j )
		{
			if( vecEntries[j].nEnterConditionID == nEnterCondID )
			{
				bDup = true;
				break;
			}
		}
		if( bDup ) continue;

		DNTableCell *pIncCell = pMapSox->GetFieldFromLablePtr( nMapID, "_IncludeBuild" );
		if( pIncCell && pIncCell->GetInteger() == 0 )
			continue;

		DNTableCell *pHardCntCell = pEnterSox->GetFieldFromLablePtr( nEnterCondID, "_RecommendHardPartyCount" );
		int nHardPartyCount = pHardCntCell ? pHardCntCell->GetInteger() : 0;
		if( nHardPartyCount != 8 )
			continue;

		SNestNotifierSortEntry entry;
		entry.nEnterConditionID = nEnterCondID;
		DNTableCell *pNameCell = pMapSox->GetFieldFromLablePtr( nMapID, "_MapNameID" );
		int nNameID = pNameCell ? pNameCell->GetInteger() : 0;
		entry.wszMapName = ( nNameID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nNameID ) : L"";
		DNTableCell *pHardLvCell = pEnterSox->GetFieldFromLablePtr( nEnterCondID, "_RecommendHardLevel" );
		entry.nRecommendHardLevel = pHardLvCell ? pHardLvCell->GetInteger() : 0;
		DNTableCell *pSortCell = pEnterSox->GetFieldFromLablePtr( nEnterCondID, "_ListSort" );
		entry.nListSort = pSortCell ? pSortCell->GetInteger() : 0;
		vecEntries.push_back( entry );
	}

	std::sort( vecEntries.begin(), vecEntries.end(), CompareNestNotifierSort );
	for( size_t i = 0; i < vecEntries.size(); ++i )
		AddNestItem( vecEntries[i].wszMapName.c_str(), 0, 1 );
}

void CDnNestNotifierDlg::AddNestItem( const WCHAR *wszNestName, int nRemainCount, int nMaxCount )
{
	if( !m_pListBoxEx )
		return;
	CDnNestNotifierListItemDlg *pItem = m_pListBoxEx->AddItem<CDnNestNotifierListItemDlg>();
	if( pItem )
		pItem->SetNestInfo( wszNestName, nRemainCount, nMaxCount );
}

void CDnNestNotifierDlg::SetNoticeText( const WCHAR *wszNotice )
{
	if( m_pStaticNotice && wszNotice )
		m_pStaticNotice->SetText( wszNotice );
}

void CDnNestNotifierDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
	if( nCommand == EVENT_BUTTON_CLICKED && ( IsCmdControl( "ID_BT_CLOSE" ) || IsCmdControl( "ID_BT_CANCEL" ) ) )
	{
		Show( false );
		return;
	}
	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}
