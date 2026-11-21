#include "StdAfx.h"
#include "DnItemsOwnedAlarm.h"
#include "DnTableDB.h"
#include "DNTableFile.h"
#include "DnItemTask.h"
#include <algorithm>

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

//-----------------------------------------------------------------------------
// CDnAlarmMarkButton
//-----------------------------------------------------------------------------
CDnAlarmMarkButton::CDnAlarmMarkButton( CEtUIDialog *pParent )
	: CEtUIButton( pParent )
	, m_bBlink( false )
	, m_fBlinkTime( 0.0f )
{
}

CDnAlarmMarkButton::~CDnAlarmMarkButton()
{
}

void CDnAlarmMarkButton::Initialize( SUIControlProperty *pProperty )
{
	CEtUIButton::Initialize( pProperty );
}

void CDnAlarmMarkButton::Process( float fElapsedTime )
{
	CEtUIButton::Process( fElapsedTime );
	if( m_bBlink )
		m_fBlinkTime += fElapsedTime;
}

//-----------------------------------------------------------------------------
// CDnItemsOwnedAlarm
//-----------------------------------------------------------------------------
CDnItemsOwnedAlarm::CDnItemsOwnedAlarm( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
	, m_pStatic0( NULL )
	, m_pTextBoxTitle( NULL )
	, m_pTextBoxSubTitle( NULL )
	, m_fAutoHideTime( 0.0f )
{
}

CDnItemsOwnedAlarm::~CDnItemsOwnedAlarm()
{
}

void CDnItemsOwnedAlarm::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "VIPAlarmDlg.ui" ).c_str(), bShow );
}

void CDnItemsOwnedAlarm::InitialUpdate()
{
	m_pStatic0 = GetControl<CEtUIStatic>( "ID_STATIC0" );
	m_pTextBoxTitle = GetControl<CEtUITextBox>( "ID_TEXTBOX_TITLE" );
	m_pTextBoxSubTitle = GetControl<CEtUITextBox>( "ID_TEXTBOX_SUBTITLE" );
}

void CDnItemsOwnedAlarm::ShowAlarm( const WCHAR *wszTitle, const WCHAR *wszSubTitle, float fDuration )
{
	if( m_pTextBoxTitle )
	{
		m_pTextBoxTitle->ClearText();
		if( wszTitle ) m_pTextBoxTitle->AddText( wszTitle );
	}
	if( m_pTextBoxSubTitle )
	{
		m_pTextBoxSubTitle->ClearText();
		if( wszSubTitle ) m_pTextBoxSubTitle->AddText( wszSubTitle );
	}
	m_fAutoHideTime = fDuration;
	Show( true );
}

void CDnItemsOwnedAlarm::Process( float fElapsedTime )
{
	CDnCustomDlg::Process( fElapsedTime );
	if( IsShow() && m_fAutoHideTime > 0.0f )
	{
		m_fAutoHideTime -= fElapsedTime;
		if( m_fAutoHideTime <= 0.0f )
			Show( false );
	}
}

//-----------------------------------------------------------------------------
// CDnItemsOwnedAlarmMng
//-----------------------------------------------------------------------------
CDnItemsOwnedAlarmMng::CDnItemsOwnedAlarmMng()
	: m_pAlarmDlg( NULL )
{
}

CDnItemsOwnedAlarmMng::~CDnItemsOwnedAlarmMng()
{
	Finalize();
}

void CDnItemsOwnedAlarmMng::Initialize()
{
	if( !m_pAlarmDlg )
	{
		m_pAlarmDlg = new CDnItemsOwnedAlarm( UI_TYPE_TOP );
		m_pAlarmDlg->Initialize( false );
	}
}

void CDnItemsOwnedAlarmMng::Finalize()
{
	SAFE_DELETE( m_pAlarmDlg );
}

static bool CompareItemNoticeOrder( const CDnItemsOwnedAlarmMng::SItemNoticeEntry &a, const CDnItemsOwnedAlarmMng::SItemNoticeEntry &b )
{
	return a.nOrder < b.nOrder;
}

void CDnItemsOwnedAlarmMng::CheckOwnedItemNotice( int nItemID )
{
	if( !m_pAlarmDlg || nItemID <= 0 )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TITEMNOTICE );
	if( !pSox || pSox->GetItemCount() <= 0 )
		return;

	m_vecNotices.clear();
	for( int i = 0; i < pSox->GetItemCount(); ++i )
	{
		int nRowID = pSox->GetItemID( i );
		DNTableCell *pTypeCell = pSox->GetFieldFromLablePtr( nRowID, "_Type" );
		int nType = pTypeCell ? pTypeCell->GetInteger() : 0;
		if( nType != 0 )
			continue;

		DNTableCell *pNeedCell = pSox->GetFieldFromLablePtr( nRowID, "_NeedItemID" );
		int nNeedItemID = pNeedCell ? pNeedCell->GetInteger() : 0;
		if( nNeedItemID != nItemID )
			continue;

		SItemNoticeEntry entry;
		entry.nID = nRowID;
		entry.nType = nType;
		DNTableCell *pOrderCell = pSox->GetFieldFromLablePtr( nRowID, "_Order" );
		DNTableCell *pNameCell = pSox->GetFieldFromLablePtr( nRowID, "_NameID" );
		DNTableCell *pDescCell = pSox->GetFieldFromLablePtr( nRowID, "_DescriptionID" );
		entry.nOrder = pOrderCell ? pOrderCell->GetInteger() : 0;
		entry.nNeedItemID = nNeedItemID;
		entry.nPCCafeRank = 0;
		entry.nNameID = pNameCell ? pNameCell->GetInteger() : 0;
		entry.nDescID = pDescCell ? pDescCell->GetInteger() : 0;
		m_vecNotices.push_back( entry );
	}

	if( m_vecNotices.empty() )
		return;

	std::sort( m_vecNotices.begin(), m_vecNotices.end(), CompareItemNoticeOrder );
	const SItemNoticeEntry &top = m_vecNotices.front();
	const WCHAR *wszTitle = ( top.nNameID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, top.nNameID ) : L"";
	const WCHAR *wszDesc = ( top.nDescID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, top.nDescID ) : L"";
	m_pAlarmDlg->ShowAlarm( wszTitle, wszDesc, 5.0f );
}

void CDnItemsOwnedAlarmMng::CheckAllOwnedItemNotices()
{
	if( !m_pAlarmDlg || !CDnItemTask::IsActive() )
		return;

	DNTableFileFormat *pSox = GetDNTable( CDnTableDB::TITEMNOTICE );
	if( !pSox || pSox->GetItemCount() <= 0 )
		return;

	m_vecNotices.clear();
	for( int i = 0; i < pSox->GetItemCount(); ++i )
	{
		int nRowID = pSox->GetItemID( i );
		DNTableCell *pTypeCell = pSox->GetFieldFromLablePtr( nRowID, "_Type" );
		int nType = pTypeCell ? pTypeCell->GetInteger() : 0;

		bool bMatch = false;
		int nNeedItemID = 0;
		int nPCCafeRank = 0;
		if( nType == 0 )
		{
			DNTableCell *pNeedCell = pSox->GetFieldFromLablePtr( nRowID, "_NeedItemID" );
			nNeedItemID = pNeedCell ? pNeedCell->GetInteger() : 0;
			if( nNeedItemID > 0 )
			{
				if( GetItemTask().GetCharInventory().FindItem( nNeedItemID ) != NULL ||
					GetItemTask().GetCashInventory().FindItem( nNeedItemID ) != NULL )
					bMatch = true;
			}
		}
		else if( nType == 1 )
		{
			DNTableCell *pPcCell = pSox->GetFieldFromLablePtr( nRowID, "_PCCafeRank" );
			nPCCafeRank = pPcCell ? pPcCell->GetInteger() : 0;
		}

		if( !bMatch )
			continue;

		SItemNoticeEntry entry;
		entry.nID = nRowID;
		entry.nType = nType;
		DNTableCell *pOrderCell = pSox->GetFieldFromLablePtr( nRowID, "_Order" );
		DNTableCell *pNameCell = pSox->GetFieldFromLablePtr( nRowID, "_NameID" );
		DNTableCell *pDescCell = pSox->GetFieldFromLablePtr( nRowID, "_DescriptionID" );
		entry.nOrder = pOrderCell ? pOrderCell->GetInteger() : 0;
		entry.nNeedItemID = nNeedItemID;
		entry.nPCCafeRank = nPCCafeRank;
		entry.nNameID = pNameCell ? pNameCell->GetInteger() : 0;
		entry.nDescID = pDescCell ? pDescCell->GetInteger() : 0;
		m_vecNotices.push_back( entry );
	}

	if( m_vecNotices.empty() )
		return;

	std::sort( m_vecNotices.begin(), m_vecNotices.end(), CompareItemNoticeOrder );
	const SItemNoticeEntry &top = m_vecNotices.front();
	const WCHAR *wszTitle = ( top.nNameID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, top.nNameID ) : L"";
	const WCHAR *wszDesc = ( top.nDescID > 0 ) ? GetEtUIXML().GetUIString( CEtUIXML::idCategory1, top.nDescID ) : L"";
	m_pAlarmDlg->ShowAlarm( wszTitle, wszDesc, 5.0f );
}
