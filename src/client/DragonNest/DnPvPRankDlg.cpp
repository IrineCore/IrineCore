#include "Stdafx.h"

#ifdef PRE_ADD_PVP_RANKING

#include "DnTableDB.h"
#include "DNTableFile.h"
#include "PvPSendPacket.h"

#include "DnGuildTask.h"
#include "DnPlayerActor.h"

#include "DnInterface.h"
#include "DnMainMenuDlg.h"
#include "DnMainFrame.h"
#include "DnPvPRankListItemDlg.h"
#include "DnPvPRankListItemLadderDlg.h"
#if defined( PRE_ADD_DWC )
#include "DnPVPDWCTabDlg.h"
#include "DnDWCMarkInformer.h"
#include "DnDWCTask.h"
#endif
#include "DnInterfaceString.h"
#include "DnPvPRankDlg.h"


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 

CDnPvPRankDlg::CDnPvPRankDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback, bool bAutoCursor )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback, bAutoCursor )
, m_pListRank(NULL)
, m_pTreeJob(NULL)
, m_pComboCondition(NULL)
, m_pBtnSearch(NULL)
, m_pBtnColosseum(NULL)
, m_pBtnLadder(NULL)
#if defined( PRE_ADD_DWC )
, m_pBtnDWC(NULL)
#endif
, m_pIMEEditBox(NULL)
, m_pListColosseum(NULL)
, m_pListLadder(NULL)
, m_pStaticSubTitle(NULL)
, m_pJobIcon(NULL)
, m_pTextureControl(NULL)
, m_pTextureControlGuild(NULL)
, m_CrrPage(1) // Currently displayed page.
, m_RequestPage(1) // Request Page.
, m_Job(0)
, m_SubClass(0)
, m_StrSearch(NULL)
, m_RankType(ERankType::ERankType_None)
, m_crrSelectPage(0)
, m_crrMouseOverPage(0)
, m_totScrollPage(1)
, m_bScroll(false)
, m_pStaticBack(NULL)
, m_pStaticNext(NULL)
, m_pStaticFirst(NULL)
, m_pStaticLast(NULL)
, m_eCurrentLadderType( LadderSystem::MatchType::None )
, m_pStaticFirstRank( NULL )
, m_pStaticGuild( NULL )
, m_pStaticMyPoint( NULL )
, m_pStaticMyResult( NULL )
, m_pStaticName( NULL )
, m_pStaticPoint( NULL )
, m_pStaticResult( NULL )
, m_pStaticSecondRank( NULL )
, m_pStaticTotRank( NULL )
#ifdef PRE_ADD_PVPRANK_INFORM
, m_pBtnRankInform(NULL)
#endif
{
#if defined( PRE_ADD_DWC )
	for( int i = 0; i < 3; ++i )
	{
		m_pStaticDWCTab[i] = NULL;
		m_pTextDWCTab[i] = NULL;
	}
#endif
}

void CDnPvPRankDlg::ReleaseDlg()
{
	m_vecPages.clear();
	SAFE_DELETE( m_pListColosseum );
	SAFE_DELETE( m_pListLadder );

	m_mapParentJobBySub.clear();
	m_mapParentJob.clear();

	if( m_pTreeJob )
		m_pTreeJob->DeleteAllItems();
}

void CDnPvPRankDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName("RankBoardDlg.ui").c_str(), bShow );
}

void CDnPvPRankDlg::InitialUpdate()
{
	m_pStaticBack = GetControl<CEtUIButton>("ID_BT_BACK"); // <
	m_pStaticNext = GetControl<CEtUIButton>("ID_BT_NEXT"); // >
	m_pStaticFirst = GetControl<CEtUIButton>("ID_BT_FIRST"); // <<
	m_pStaticLast = GetControl<CEtUIButton>("ID_BT_LAST");   // >>

	m_pBtnColosseum = GetControl<CEtUIRadioButton>("ID_RBT0");
	m_pBtnColosseum->SetDisableTime( 1.0f );
	m_pBtnColosseum->SetChecked( true );

	m_pBtnSearch = GetControl<CEtUIButton>("ID_BT_SEARCH");
	m_pBtnSearch->SetDisableTime( 1.0f );
	
	m_pBtnLadder = GetControl<CEtUIRadioButton>("ID_RBT1");
	m_pBtnLadder->SetDisableTime( 1.0f );

#if defined( PRE_ADD_DWC )
	{
		bool bExist = false;
		m_pBtnDWC = GetControl<CEtUIRadioButton>("ID_RBT_DWC", &bExist);
		if( bExist && m_pBtnDWC )
			m_pBtnDWC->SetDisableTime( 1.0f );
		for( int i = 0; i < 3; ++i )
		{
			char szDwcCtrl[64] = { 0 };
			sprintf_s( szDwcCtrl, sizeof( szDwcCtrl ), "ID_STATIC_DWCTAB%d", i );
			bExist = false;
			m_pStaticDWCTab[i] = GetControl<CEtUIStatic>( szDwcCtrl, &bExist );
			if( bExist && m_pStaticDWCTab[i] ) m_pStaticDWCTab[i]->Show( false );

			sprintf_s( szDwcCtrl, sizeof( szDwcCtrl ), "ID_TEXT_DWCTAB%d", i );
			bExist = false;
			m_pTextDWCTab[i] = GetControl<CEtUIStatic>( szDwcCtrl, &bExist );
			if( bExist && m_pTextDWCTab[i] ) m_pTextDWCTab[i]->Show( false );
		}
	}
#endif

	m_pTreeJob = GetControl<CEtUITreeCtl>("ID_TREE_LIST");

	m_pComboCondition = GetControl<CEtUIComboBox>("ID_COMBOBOX_TITLE");
	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 5001 ), NULL, (int)EComboCondition::EComboCondition_Character, true ); // "Character Name"
	m_pComboCondition->AddItem( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 121050 ), NULL, (int)EComboCondition::EComboCondition_Guild, true ); // "Guild Name"		

#ifdef PRE_ADD_PVPRANK_INFORM
	m_pBtnRankInform = GetControl<CEtUIButton>("ID_BT_RANKINFO");
#endif

	m_pListRank = GetControl<CEtUIListBoxEx>("ID_LISTBOXEX_LIST");

	m_pIMEEditBox = GetControl<CEtUIIMEEditBox>("ID_IMEEDITBOX_NAME");

	m_pStaticMyPoint = GetControl<CEtUIStatic>("ID_TEXT0");  // Rating.
	m_pStaticMyResult = GetControl<CEtUIStatic>("ID_TEXT1"); // Result.

	m_pStaticTotRank = GetControl<CEtUIStatic>("ID_TEXT_MYRANK0"); // Overall rank.
	m_pStaticFirstRank = GetControl<CEtUIStatic>("ID_TEXT_MYRANK1"); // 1st job class ranking.
	m_pStaticSecondRank = GetControl<CEtUIStatic>("ID_TEXT_MYRANK2"); // 2nd job rank.
	m_pStaticGuild = GetControl<CEtUIStatic>("ID_TEXT_GUILD");   // Guild name.
	m_pStaticName = GetControl<CEtUIStatic>("ID_TEXT_NAME");    // Character name.
	m_pStaticPoint = GetControl<CEtUIStatic>("ID_TEXT_COUNT");   // Points.
	m_pStaticResult = GetControl<CEtUIStatic>("ID_TEXT_RESULT");  // Result.
	m_pTextureControl = GetControl<CEtUITextureControl>("ID_TEXTUREL_RANK"); // Colosseum rank icon.
	m_pTextureControlGuild = GetControl<CEtUITextureControl>("ID_TEXTUREL_GUILD"); // Guild icon.
	m_pTextureControlGuild->Show( false );
	m_pStaticSubTitle = GetControl<CEtUIStatic>("ID_TEXT_SUBTITLE");
	m_pStaticSubTitle->SetText( GetSubString( ERankType::ERankType_Colosseum ) );

	m_pJobIcon = GetControl<CDnJobIconStatic>("ID_STATIC_CLASS");

	char str[256] = {0,};
	m_vecPages.resize( RankingSystem::RANKINGPAGECOUNT );
	for( int i=0; i<RankingSystem::RANKINGPAGECOUNT; ++i )
	{
		sprintf_s( str, 256, "ID_TEXT_PAGE%d", i+1 );
		m_vecPages[ i ] = GetControl<CEtUIStatic>( str );		
		m_vecPages[ i ]->SetButton( true );
	}

	int size = PVPTYPESIZE;
	for( int i=0; i<size; ++i )
	{	
		sprintf_s( str, 256, "ID_STATIC_RESULTTAB%d", i );
		m_arrControls[ i ].pResultTab = GetControl<CEtUIStatic>(str);

		sprintf_s( str, 256, "ID_STATIC_POINTTAB%d", i );
		m_arrControls[ i ].pPointTab = GetControl<CEtUIStatic>(str);

		sprintf_s( str, 256, "ID_TEXT_RESULTTAB%d", i );
		m_arrControls[ i ].pResultText = GetControl<CEtUIStatic>(str);
		
		sprintf_s( str, 256, "ID_TEXT_POINTTAB%d", i );
		m_arrControls[ i ].pPointText = GetControl<CEtUIStatic>(str);
	}

	for( int i=0; i<LadderSystem::MatchType::MaxMatchType; i++ )
	{
		if( i > LadderSystem::MatchType::None )
		{
			CEtUIRadioButton *pRadioButton = GetControl<CEtUIRadioButton>( FormatA( "ID_RBT%d" , i ).c_str() );
			if( pRadioButton )
			{
#if defined( PRE_FIX_TEAM_LADDER_3vs3 )
				if( i <= LadderSystem::MatchType::MaxCount )
				{
					pRadioButton->Show( i <= LadderSystem::MatchType::_2vs2 );

					if( i== LadderSystem::MatchType::_2vs2 )
						pRadioButton->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126028 ) ); // 3vs3 Ladder 
				}
				else
				{
					pRadioButton->Show( false );
				}
#else
				pRadioButton->Show( i <= LadderSystem::MatchType::MaxCount );
#endif
			}
		}
	}
}

std::wstring CDnPvPRankDlg::GetSubString( ERankType type )
{
	std::wstring str;
	if( type == ERankType::ERankType_Colosseum )
		str.assign( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2419 ) ); // "Colosseum"
	else if( type == ERankType::ERankType_Ladder )
		str.assign( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126040 ) ); // "Ladder"

	str.append( L" " );
	str.append( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126095 ) ); // "Rank"

	return str;
}

void CDnPvPRankDlg::Show( bool bShow )
{
 	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );

	// Open.
	if( bShow )
	{
		BuildTreeJob();  // Build TreeControl per job.
		m_pTreeJob->SetSelectItem( m_pTreeJob->GetBeginItem() );
		RequestMyInfo( ERankType::ERankType_Colosseum ); // Request my info.
	}
	// Close.
	else
	{
		Reset();

		m_crrSelectPage = m_crrMouseOverPage = 0;		
		m_RankType = ERankType::ERankType_None;
		m_pBtnLadder->SetChecked( false );
		m_pBtnColosseum->SetChecked( true );
		m_bScroll = false;		

		if( m_pTreeJob )
			m_pTreeJob->ResetSelectedItem();
	}

	if (m_pCallback)
		m_pCallback->OnUICallbackProc( CDnMainMenuDlg::PVP_RANK_DIALOG, bShow ? EVENT_DIALOG_SHOW : EVENT_DIALOG_HIDE, NULL);

	
	// Right-side screen notifier windows - missions, quests, crafting, etc..
	GetInterface().OpenRightAlramDlg( bShow );
}

bool CDnPvPRankDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() ) {
		return false;
	}
	/*if( uMsg == WM_LBUTTONDOWN )
	{
		float fMouseX, fMouseY;
		POINT MousePoint;
		MousePoint.x = short( LOWORD( lParam ) );
		MousePoint.y = short( HIWORD( lParam ) );
		PointToFloat( MousePoint, fMouseX, fMouseY );

		if( CONTROL( IMEEditBox, ID_IMEEDITBOX_SEARCHWORD )->IsInside( fMouseX, fMouseY ) )
		{
			std::wstring curText = CONTROL( IMEEditBox, ID_IMEEDITBOX_SEARCHWORD )->GetText();
			OnSetSearchItemList( curText );
		}
	}*/

	if( m_pComboCondition && !m_pComboCondition->IsOpenedDropDownBox() )
	{
		switch( uMsg )
		{
			case WM_KEYDOWN:
				{
					if( wParam == VK_RETURN )
					{
						if(m_pIMEEditBox->IsFocus())
						{
							this->ProcessCommand(EVENT_BUTTON_CLICKED, true, GetControl("ID_BT_SEARCH"), 0);
						}
						return true;
					}
				}
				break;

			case WM_MOUSEWHEEL:
				{
					POINT MousePoint;
					MousePoint.x = short( LOWORD( lParam ) );
					MousePoint.y = short( HIWORD( lParam ) );

					ScreenToClient( hWnd, &MousePoint );
					lParam = MAKELPARAM( MousePoint.x, MousePoint.y );

					if( IsMouseInDlg() )
					{
						UINT uLines;
						SystemParametersInfo( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
						int nScrollAmount = int( ( short )HIWORD( wParam ) ) / WHEEL_DELTA * uLines;
						if( nScrollAmount > 0 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_BACK"), 0 );
						else if( nScrollAmount < 0 )
							ProcessCommand( EVENT_BUTTON_CLICKED, false, GetControl<CEtUIButton>("ID_BT_NEXT"), 0 );

						CEtUIDialogBase::StaticMsgProc( CDnMainFrame::GetInstance().GetHWnd(), WM_MOUSEMOVE, 0, (LPARAM)lParam );
						return true;
					}
				}
				break;

			case WM_MOUSEMOVE:
				{				
					int size = (int)m_vecPages.size();
					for( int i=0; i<size; ++i )
					{
						if( m_vecPages[ i ]->IsMouseEnter() )
						{
							m_crrMouseOverPage = i;
							break;
						}	
					}				
				}
				break;
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnPvPRankDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl * pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	//// TEST.
	//if( nCommand == EVENT_TREECONTROL_SELECTION )
	//{
	//	if( IsCmdControl("ID_TREE_LIST") )
	//	{
	//		CTreeItem * pItem = m_pTreeJob->GetSelectedItem();
	//		if( pItem )
	//		{
	//			WCHAR str[256]={0,};
	//			_itow( pItem->GetItemValueInt(), str, 10 );
	//			GetControl<CEtUIStatic>("ID_TEXT_GUILD")->SetText( str );
	//		}
	//	}
	//}

	if( nCommand == EVENT_BUTTON_CLICKED )
	{			
		// Search.
		if( IsCmdControl("ID_BT_SEARCH") )
			SelectionTreeJob();

		// Close.
		else if( IsCmdControl("ID_BT_CLOSE") )
			Show( false );
		
		// <.
		else if( IsCmdControl("ID_BT_BACK") )
		{
			m_CrrPage -= 1;
			PageChange();
		}
		// >.
		else if( IsCmdControl("ID_BT_NEXT") )
		{
			m_CrrPage += 1;
			PageChange();
		}
		// <<.
		else if( IsCmdControl("ID_BT_FIRST") )
		{
			m_bScroll = true;
			PageScroll( -1 );
		}
		// >>.
		else if( IsCmdControl("ID_BT_LAST") )
		{
			m_bScroll = true;
			PageScroll( 1 );
		}
		else if( strstr( pControl->GetControlName(), "ID_TEXT_PAGE" ) != NULL )
		{
			int nIndex = -1;
			if( sscanf( pControl->GetControlName() + strlen("ID_TEXT_PAGE"), "%d", &nIndex) == 1 )
			{	
				if( m_RequestPage > 1 )
				{
					int denominator = ( m_RequestPage - 1 ) * RankingSystem::RANKINGPAGECOUNT;
					int crrSelectionPage = ( m_CrrPage - 1 ) / denominator;
					crrSelectionPage += ( m_CrrPage - 1 ) % denominator;
					if( nIndex != crrSelectionPage )
					{
						m_CrrPage += ( nIndex - crrSelectionPage );
						PageChange();
					}
				}
				else
				{
					if( m_CrrPage != nIndex )
					{	
						m_CrrPage += ( nIndex - m_CrrPage );
						PageChange();
					}
				}
								
			}
		}
#ifdef PRE_ADD_PVPRANK_INFORM
		else if (IsCmdControl("ID_BT_RANKINFO"))
		{
			GetInterface().ShowPVPRankInformDlg(true);
		}
#endif
	}

	else if( nCommand == EVENT_RADIOBUTTON_CHANGED )
	{
		if( IsShow() && m_pListRank )
		{
			// Colosseum
			if( IsCmdControl("ID_RBT0") )
				RequestMyInfo( ERankType::ERankType_Colosseum ); // Request my info.
			
			// 1:1 Ladder
			else if( IsCmdControl("ID_RBT1") )
				RequestMyInfo( ERankType::ERankType_Ladder, LadderSystem::MatchType::_1vs1 ); // Request my info.

#if defined(PRE_FIX_TEAM_LADDER_3vs3)
			// 3:3 Ladder
			else if( IsCmdControl("ID_RBT2") )
				RequestMyInfo( ERankType::ERankType_Ladder, LadderSystem::MatchType::_3vs3 ); // Request my info.
#else
			// 2:2 Ladder
			else if( IsCmdControl("ID_RBT2") )
				RequestMyInfo( ERankType::ERankType_Ladder, LadderSystem::MatchType::_2vs2 ); // Request my info.
#endif

			// 3:3 Ladder
			else if( IsCmdControl("ID_RBT3") )
				RequestMyInfo( ERankType::ERankType_Ladder, LadderSystem::MatchType::_3vs3 ); // Request my info.
#if defined( PRE_ADD_DWC )
			else if( IsCmdControl("ID_RBT_DWC") )
			{
				m_RankType = ERankType::ERankType_DWC;
				for( int i = 0; i < 3; ++i )
				{
					if( m_pStaticDWCTab[i] ) m_pStaticDWCTab[i]->Show( true );
					if( m_pTextDWCTab[i] ) m_pTextDWCTab[i]->Show( true );
				}
				if( CDnDWCTask::IsActive() )
					GetDWCTask().RequestGetRankPage( m_CrrPage );
				RefreshDWCRankList();
			}
#endif

		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

// Build TreeControl per job.
void CDnPvPRankDlg::BuildTreeJob()
{	
	if( m_pTreeJob->GetExpandedItemCount() > 0 )
		return;
	
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TJOB );
	if( !pTable )
		return;

	DNTableCell * pCell = NULL;
		
	std::map< int, CTreeItem * >::iterator itFind;
	
	CTreeItem * pTreeItem = m_pTreeJob->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 601 ) ); // "All"
	
	int nSize = pTable->GetItemCount();
	for( int i=0; i<nSize; ++i )
	{
		int nID = pTable->GetItemID( i );
		
		// Not a player class.
		pCell = pTable->GetFieldFromLablePtr( nID, "_Class" );
		if( pCell == NULL ||  pCell->GetInteger() == 0 )
			continue;
		
#ifdef PRE_FIX_PVPRANK_NOSERVICE_JOB
		// Class not yet added.
		pCell = pTable->GetFieldFromLablePtr( nID, "_Service" );
		if( pCell == NULL || pCell->GetInteger() == 0 )
			continue;
#endif

		int nClass = pCell->GetInteger();

		pCell = pTable->GetFieldFromLablePtr( nID, "_JobNumber" );
		if( !pCell )
			continue;

		// Major category.
		if( pCell->GetInteger() == 0 )
		{
			pCell = pTable->GetFieldFromLablePtr( nID, "_JobName" );
			if( pCell )
			{
				pTreeItem = m_pTreeJob->AddItem( CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pCell->GetInteger() ) );	
				pTreeItem->SetItemValueInt( nID );
				m_mapParentJob.insert( std::map< int, CTreeItem * >::value_type( nID, pTreeItem ) ); 			
			}
		}

		// Subcategory - up to 1st class advancement only.
		else if( pCell->GetInteger() == 1 )
		{
#ifdef PRE_FIX_PVPRANK_NOSERVICE_JOB
#else
			// Class not yet added.
			pCell = pTable->GetFieldFromLablePtr( nID, "_Service" );
			if( !pCell || ( pCell && pCell->GetInteger() == 0) )
				continue;
#endif

			pCell = pTable->GetFieldFromLablePtr( nID, "_ParentJob" );
			if( pCell )
			{
				itFind = m_mapParentJob.find( pCell->GetInteger() );
				if( itFind != m_mapParentJob.end() )
				{
					// Subclass job type.
					m_mapParentJobBySub.insert( std::map< int, int >::value_type( nID, nClass ) );

					pCell = pTable->GetFieldFromLablePtr( nID, "_JobName" );
					if( pCell )
					{
						pTreeItem = m_pTreeJob->AddChildItem( itFind->second, CTreeItem::typeOpen, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, pCell->GetInteger() ) );
						pTreeItem->SetItemValueInt( nID );
					}
				}				
			}
		}
	}
}

// Job selection.
void CDnPvPRankDlg::SelectionTreeJob()
{
	m_StrSearch = const_cast< wchar_t * >( m_pIMEEditBox->GetText() );

	// ComboBox.
	EComboCondition comboCondition = EComboCondition::EComboCondition_NONE;
	SComboBoxItem * pComboItem = m_pComboCondition->GetSelectedItem();
	if( pComboItem )
		comboCondition = (EComboCondition)pComboItem->nValue;

	// Validate string when searching by character name.
	int strSize = m_pIMEEditBox->GetTextLength() + CEtUIIME::GetCompStringLength();
	if( comboCondition == EComboCondition::EComboCondition_Character && strSize > 0 && strSize < CHARNAMEMIN )
	{		
		GetInterface().MessageBox( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 100303 ), CHARNAMEMIN ).c_str() ); // "Character name must be at least %d characters."
		return;
	}

	// Initialize Page.
	m_CrrPage = m_RequestPage = 1;

	// #68604 - When pressing search button, select the first item if no tree control item is selected.
	if( m_pTreeJob->GetSelectedItem() == NULL )
		m_pTreeJob->SetSelectItem( m_pTreeJob->GetBeginItem() );


	// No search condition - search by class (guild name search uses the same handling).
	if( m_StrSearch == NULL || strSize < 1 ||
		EComboCondition::EComboCondition_Guild == comboCondition )
	{
		CTreeItem * pSelectedItem = m_pTreeJob->GetSelectedItem();
		if( !pSelectedItem )
		{
			return;
		}

		m_SubClass = 0;
		m_Job = pSelectedItem->GetItemValueInt();			
		std::map< int, CTreeItem * >::iterator itFind;
		itFind = m_mapParentJob.find( m_Job );
	
		// SubJob.
		if( itFind == m_mapParentJob.end() )
		{
			m_SubClass = m_Job;

			// Job type of the corresponding SubClass.
			//std::map< int, int >::iterator it = m_mapParentJobBySub.find( m_SubClass );
			//if( it != m_mapParentJobBySub.end() )
			//	m_Job = it->second;

			m_Job = 0;
		}

		//if( m_Job == m_SubClass )
		//	m_SubClass = 0;

		// #68497 - Ignore class category during guild search (searches all guild members)
		if( m_StrSearch != NULL && strSize > 0 )
		{
			m_Job = m_SubClass = 0;
			m_pTreeJob->SetSelectItem( m_pTreeJob->GetBeginItem() );
		}

		// Send.
		if( m_RankType == ERankType::ERankType_Colosseum )
			OnSendPvPRankList( m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
		else if( m_RankType == ERankType::ERankType_Ladder )
			OnSendPvPRankLadderList( m_eCurrentLadderType, m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
		
		return;
	}

	// With search condition - search by character name.	
	else if( EComboCondition::EComboCondition_Character == comboCondition )
	{	
		// Send.
		if( m_RankType == ERankType::ERankType_Colosseum )
			OnSendPvPRankCharacter( m_StrSearch );
		else if( m_RankType == ERankType::ERankType_Ladder )
			OnSendPvPRankLadderCharacter( m_eCurrentLadderType, m_StrSearch );
		m_pTreeJob->SetSelectItem( m_pTreeJob->GetBeginItem() );
		return;	
	}
}

// Request my info.
void CDnPvPRankDlg::RequestMyInfo( ERankType rank , LadderSystem::MatchType::eCode eMatchType )
{
	if( m_RankType == rank )
	{
		if( rank == ERankType::ERankType_Ladder )
		{
			if( m_eCurrentLadderType == eMatchType )
				return;
		}
		else
		{
			return;
		}
	}

	// Reset previous info.
	Reset();

	m_RankType = rank;	

	// Change SubTitle.
	m_pStaticSubTitle->SetText( GetSubString( rank ) );

	// Change List Header.
	int size = PVPTYPESIZE;
	for( int i=0; i<size; ++i )
	{	
		bool bShow = ( i==(int)rank ) ? true : false;

		m_arrControls[ i ].pResultTab->Show( bShow );		
		m_arrControls[ i ].pPointTab->Show( bShow );		
		m_arrControls[ i ].pResultText->Show( bShow );		
		m_arrControls[ i ].pPointText->Show( bShow );
	}

	// Send.
	if( rank == ERankType::ERankType_Colosseum )
	{		
		OnSendPvPRankMyInfo();
		OnSendPvPRankList( m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
	}

	else if( rank == ERankType::ERankType_Ladder )
	{
		OnSendPvPLadderRankMyInfo( eMatchType );
		OnSendPvPRankLadderList( eMatchType , m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
	}	
}

// My info - Colosseum.
void CDnPvPRankDlg::SetInfoMyRankColosseum( struct TPvPRankingDetail * pInfo )
{
	m_totScrollPage = 1;

	wchar_t str[256] = {0,};

	if( pInfo->biRank > 0 )
	{
		swprintf_s( str, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120203 ), pInfo->biRank ); // "Overall Rank #%d"
		m_pStaticTotRank->SetText( str );	
	}
	else
		m_pStaticTotRank->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120210 ) ); // "No Record"
		
	int job = pInfo->cJobCode;	

	// Colosseum rank icon.
	if( pInfo->cPvPLevel > 0 )
	{
		m_pTextureControl->Show( true );
		GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );
	}
	else
		m_pTextureControl->Show( false );

	m_pStaticMyPoint->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120206 ) );// "PVP Points"
	m_pStaticMyResult->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120205 ) ); // "Kills/Deaths"

	if( pInfo->wszCharName != NULL && ( wcslen( pInfo->wszCharName ) > 1 ) )
	{
		std::wstring strName( pInfo->wszCharName );

		// Job icon.
		LPCWSTR strJob = DN_INTERFACE::STRING::GetJobString( job );
		m_pJobIcon->Show( true );
		m_pJobIcon->SetIconID( job );
		m_pJobIcon->SetTooltipText( strJob );

		// Name (Class LVxx).
		strName.append( L" (" );
		strName.append( strJob );
		swprintf_s( str, 256, GetUIString( CEtUIXML::idCategory1, 2020022 ), pInfo->iLevel ); // "Lv%d"
		strName.append( str );
		strName.append( L")" );
		m_pStaticName->SetText( strName );

		// Guild mark.		
		SetGuildIcon(); 
	}
	else
	{
		m_pTextureControlGuild->Show( false );
		m_pJobIcon->Show( false );
		m_pStaticName->SetText( L"" );
	}

	// Set ranking per job.
	SetJobRank( pInfo->cJobCode, pInfo->biClassRank, pInfo->biSubClassRank );		

	// Guild name.
	m_pStaticGuild->SetText( pInfo->wszGuildName );

	std::wstring strPoint = AtoCommaString( pInfo->uiExp );
	m_pStaticPoint->SetText( strPoint.c_str() );

	// Result.
	/*int nPersent = 0;
	if( pInfo->iDeath == 0 )
		nPersent = pInfo->iKill * 100;
	else if( pInfo->iDeath > 0 )
		nPersent = (int)( ( (float)pInfo->iKill / (float)pInfo->iDeath ) * 100.0f );	
	swprintf_s( str, 256, L"%d / %d (%d%%)", pInfo->iKill, pInfo->iDeath, nPersent );*/
	swprintf_s( str, 256, L"%d / %d", pInfo->iKill, pInfo->iDeath );
	m_pStaticResult->SetText( str );
}

// My info - Ladder.
void CDnPvPRankDlg::SetInfoMyRankLadder( struct TPvPLadderRankingDetail * pInfo )
{
	m_totScrollPage = 1;

	wchar_t str[256] = {0,};

	if( pInfo->biRank > 0 )
	{
		swprintf_s( str, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120203 ), pInfo->biRank ); // "Overall Rank #%d"
		m_pStaticTotRank->SetText( str );

		SetGuildIcon(); // Guild mark.
	}
	else
		m_pStaticTotRank->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120210 ) ); // "No Record"
	

	int job = pInfo->cJobCode;	

	// Colosseum rank icon.
	if( pInfo->cPvPLevel > 0 )
	{
		m_pTextureControl->Show( true );
		GetInterface().SetPvPClassIcon( m_pTextureControl, pInfo->cPvPLevel );
	}
	else
		m_pTextureControl->Show( false );

	m_pStaticMyPoint->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126020 ) );// "Rating"
	m_pStaticMyResult->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 2331 ) ); // "Result"

	if( pInfo->wszCharName != NULL && ( wcslen( pInfo->wszCharName ) > 1 ) )
	{
		std::wstring strName( pInfo->wszCharName );
		
		// Job icon.
		LPCWSTR strJob = DN_INTERFACE::STRING::GetJobString( job );
		m_pJobIcon->Show( true );
		m_pJobIcon->SetIconID( job );
		m_pJobIcon->SetTooltipText( strJob );

		// Name (Class LVxx).
		strName.append( L" (" );
		strName.append( strJob );
		swprintf_s( str, 256, GetUIString( CEtUIXML::idCategory1, 2020022 ), pInfo->iLevel ); // "Lv%d"
		strName.append( str );
		strName.append( L")" );
		m_pStaticName->SetText( strName );
	}
	else
	{
		m_pTextureControlGuild->Show( false );
		m_pJobIcon->Show( false );
		m_pStaticName->SetText( L"" );
	}

	// Set ranking per job.
	SetJobRank( pInfo->cJobCode, pInfo->biClassRank, pInfo->biSubClassRank );		
	
	// Guild name.
	m_pStaticGuild->SetText( pInfo->wszGuildName );
	
	std::wstring strPoint = AtoCommaString( pInfo->iPvPLadderGradePoint );
	m_pStaticPoint->SetText( strPoint.c_str() );

	// Result.
	int nPersent = 0;
	int denominator = pInfo->iWin + pInfo->iLose;
	if( denominator > 0 )
		nPersent = (int)( ( (float)pInfo->iWin / (float)denominator ) * 100 );

	swprintf_s( str, 256, GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126039 ), pInfo->iWin, pInfo->iLose, pInfo->iDraw ); // "%d W %d L %d D"
	std::wstring strResult( str );
	strResult.append( L" (" );
	swprintf_s( str, 256, L"%s %d%%", GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 126177 ), nPersent );  // "Win Rate :"
	strResult.append( str );
	strResult.append( L")" );
	m_pStaticResult->SetText( strResult.c_str() );
	memset( str, 0, 256 );	

}

// Set character info - Colosseum.
void CDnPvPRankDlg::SetInfoColosseum( struct TPvPRankingDetail * pInfo )
{
	if( !m_pListRank )
		return;

	// Reset list.
	ResetList();

	// 
	ResetPvPList();

	if( pInfo->iLevel > 0 )
	{
		CDnPvPRankListItemDlg * pListItemDlg = m_pListRank->AddItem< CDnPvPRankListItemDlg >();
		if( pListItemDlg )
			pListItemDlg->SetInfoUserColosseum( pInfo );
	}
}

// Set character info - Colosseum.
void CDnPvPRankDlg::SetInfoLadder( struct TPvPLadderRankingDetail * pInfo )
{
	if( !m_pListRank )
		return;

	// Reset list.
	ResetList();

	// 
	ResetPvPList();

	if( pInfo->iLevel > 0 )
	{
		//CDnPvPRankListItemDlg * pListItemDlg = m_pListRank->AddItem< CDnPvPRankListItemDlg >();
		CDnPvPRankListItemLadderDlg * pListItemDlg = m_pListRank->AddItem< CDnPvPRankListItemLadderDlg >();
		if( pListItemDlg )
			pListItemDlg->SetInfoUserLadder( pInfo );
	}
}


//// Return parent job name of job.
//int CDnPvPRankDlg::GetParentJob( int job )
//{
//	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TJOB );
//	if( !pTable )
//		return job;
//
//	DNTableCell * pCell = NULL;
//
//	int nParentJob = job;
//	int nSize = pTable->GetItemCount();
//	for( int i=0; i<nSize; ++i )
//	{
//		int nID = pTable->GetItemID( i );
//		if( nID != job )
//			continue;
//		
//		/*pCell = pTable->GetFieldFromLablePtr( nID, "_JobNumber" );
//		if( pCell && pCell->GetInteger() == 2 )
//		{*/
//			pCell = pTable->GetFieldFromLablePtr( nID, "_ParentJob" );
//			if( pCell && pCell->GetInteger() > 0 )
//			{
//				nParentJob = pCell->GetInteger();
//				break;
//			}
//		//}		
//	}
//
//	return nParentJob;	
//}

// Return parent job name of job.
int CDnPvPRankDlg::GetParentJob( int job )
{
	DNTableFileFormat * pTable = GetDNTable( CDnTableDB::TJOB );
	if( !pTable )
		return 0;

	DNTableCell * pCell = NULL;

	int nParentJob = 0;
	int nSize = pTable->GetItemCount();
	for( int i=0; i<nSize; ++i )
	{
		int nID = pTable->GetItemID( i );
		if( nID != job )
			continue;

		pCell = pTable->GetFieldFromLablePtr( nID, "_ParentJob" );
		if( pCell && pCell->GetInteger() > 0 )
		{
			nParentJob = pCell->GetInteger();
			break;
		}		
	}

	return nParentJob;	
}

// List - Colosseum.
void CDnPvPRankDlg::SetListColosseum( SCPvPRankList * pInfo )
{
	// Reset list.
	ResetList();

	INT64 tot = 0;
	if( m_pListColosseum ) 
		tot = m_pListColosseum->nRankingTotalCount;
	SAFE_DELETE( m_pListColosseum );
	
	if( pInfo->nRankingTotalCount != -1 )
		m_totScrollPage = (int)( pInfo->nRankingTotalCount / (INT64)( RankingSystem::RANKINGPAGEMAX * RankingSystem::RANKINGPAGECOUNT ) + 1 );

	// Calculate current page.
	if( m_bScroll )
		m_CrrPage = ( m_RequestPage - 1 ) * RankingSystem::RANKINGPAGECOUNT + 1;

	m_pListColosseum = new SCPvPRankList;
	m_pListColosseum->cRankingCount = pInfo->cRankingCount;
	m_pListColosseum->nRankingTotalCount = pInfo->nRankingTotalCount != -1 ? pInfo->nRankingTotalCount : tot;
	for( int i=0; i<m_pListColosseum->cRankingCount; ++i )
		m_pListColosseum->RankingInfo[ i ] = pInfo->RankingInfo[ i ];	


	// Set user list.
	PageChange();
}


// List - Ladder.
void CDnPvPRankDlg::SetListLadder( SCPvPLadderRankList * pInfo )
{
	// Reset list.
	ResetList();

	INT64 tot = 0;
	if( m_pListLadder )  
		tot = m_pListLadder->nRankingTotalCount;
	SAFE_DELETE( m_pListLadder );

	if( pInfo->nRankingTotalCount != -1 )
		m_totScrollPage = (int)( pInfo->nRankingTotalCount / (INT64)( RankingSystem::RANKINGPAGEMAX * RankingSystem::RANKINGPAGECOUNT ) + 1 );
		//m_totScrollPage = pInfo->nRankingTotalCount / ( RankingSystem::RANKINGPAGEMAX * RankingSystem::RANKINGPAGECOUNT ) + 1;

	// Calculate current page.
	if( m_bScroll )
		m_CrrPage = ( m_RequestPage - 1 ) * RankingSystem::RANKINGPAGECOUNT + 1;

	m_pListLadder = new SCPvPLadderRankList;
	m_pListLadder->cRankingCount = pInfo->cRankingCount;
	m_pListLadder->nRankingTotalCount = pInfo->nRankingTotalCount != -1 ? pInfo->nRankingTotalCount : tot;
	for( int i=0; i<m_pListLadder->cRankingCount; ++i )
		m_pListLadder->RankingInfo[ i ] = pInfo->RankingInfo[ i ];	

	m_eCurrentLadderType = pInfo->MatchType;

	// Set user list.
	PageChange();
}

// Change by 1 page & set user list.
void CDnPvPRankDlg::PageChange()
{	
	if( m_CrrPage < 1 )
	{
		m_crrSelectPage = m_CrrPage = 1;
		return;
	}

	// Same situation as <<.
	int cnt = m_CrrPage * RankingSystem::RANKINGPAGEMAX;
	if( cnt < ( m_RequestPage - 1 ) * RankingSystem::RANKINGMAX + 1 )
	{
		m_bScroll = false;
		PageScroll( -1 );
		return;
	}

	// Same situation as >>.
	if( cnt > m_RequestPage * RankingSystem::RANKINGMAX )
	{
		m_bScroll = false;
		PageScroll( 1 );
		return;
	}

	// Max Page.
	INT64 tot = _I64_MAX;
	if( m_RankType == ERankType::ERankType_Colosseum )
	{
		if( m_pListColosseum == NULL )
			return;

		tot = m_pListColosseum->nRankingTotalCount;
	}
	else if( m_RankType == ERankType::ERankType_Ladder )
	{
		if( m_pListLadder == NULL )
			return;
		tot = m_pListLadder->nRankingTotalCount;
	}

	if( tot < ( RankingSystem::RANKINGPAGEMAX * (m_CrrPage-1) ) )
	{
		m_CrrPage -= 1;
		return;
	}

	// SelectPage.
	if( m_RequestPage > 1 )
	{
		int denominator = ( m_RequestPage - 1 ) * RankingSystem::RANKINGPAGECOUNT;
		m_crrSelectPage = ( m_CrrPage - 1 ) / denominator;
		m_crrSelectPage += ( m_CrrPage - 1 ) % denominator;
	}
	else
	{
		m_crrSelectPage = m_CrrPage;
	}
	RefreshPageControls();

	// Configure list.
	if( !m_pListRank )
		return;

	m_pListRank->RemoveAllItems();

	// Calculate number of Items to display per Page.
	int page = m_CrrPage - ( (m_RequestPage - 1) * RankingSystem::RANKINGPAGECOUNT ) ;	
	int min = ( page - 1 ) * RankingSystem::RANKINGPAGEMAX;
	int max = page * RankingSystem::RANKINGPAGEMAX;

	int cRankingCount = 0;
	if( m_RankType == ERankType::ERankType_Colosseum )
		cRankingCount = m_pListColosseum->cRankingCount;
	else if( m_RankType == ERankType::ERankType_Ladder )
		cRankingCount = m_pListLadder->cRankingCount;

	if( max > cRankingCount )
	{
		if( RankingSystem::RANKINGPAGEMAX > ( max - cRankingCount ) )		
		{
			if( page > 1 )
			{
				max = cRankingCount % ( ( page - 1 ) * RankingSystem::RANKINGPAGEMAX );
				max += min;
			}
			else
				max = cRankingCount - min;
		}
	}
	
	int nRankCount = 0;
	if( m_RankType == ERankType::ERankType_Colosseum && m_pListColosseum )
	{
		nRankCount = m_pListColosseum->cRankingCount;
		
		if( m_pListColosseum->cRankingCount > 0 )
		{
			for( int i=min; i<max; ++i )	
			{
				CDnPvPRankListItemDlg * pListItemDlg = m_pListRank->AddItem< CDnPvPRankListItemDlg >();
				if( pListItemDlg )
					pListItemDlg->SetInfoColosseum( &m_pListColosseum->RankingInfo[ i ] );			
			}
		}
	}
	
	else if( m_RankType == ERankType::ERankType_Ladder && m_pListLadder )
	{
		nRankCount = m_pListLadder->cRankingCount;

		if( m_pListLadder->cRankingCount > 0 )
		{
			for( int i=min; i<max; ++i )	
			{
				CDnPvPRankListItemLadderDlg * pListItemDlg = m_pListRank->AddItem< CDnPvPRankListItemLadderDlg >();
				if( pListItemDlg )
					pListItemDlg->SetInfoLadder( &m_pListLadder->RankingInfo[ i ] );			
			}
		}
	}



	// Set Page control.
	std::wstring strP;
	wchar_t strPage[256] = {0,};
	int pageCnt = ( nRankCount - 1 ) / RankingSystem::RANKINGPAGEMAX + 1;
	for( int i=0; i<RankingSystem::RANKINGPAGECOUNT; ++i )
	{
		if( i < pageCnt )
		{		
			strP.assign( L" " );
			_itow( i + ( ( m_RequestPage - 1 ) * RankingSystem::RANKINGPAGECOUNT ) + 1, strPage, 10 );
			strP.append( strPage );
			strP.append( L" " );
			m_vecPages[ i ]->SetText( strP.c_str() );
			m_vecPages[ i ]->Show( true );
		}
		else
			m_vecPages[ i ]->Show( false );
	}

}

// Change RANKINGPAGECOUNT unit.
void CDnPvPRankDlg::PageScroll( int n )
{
	m_RequestPage += n;
	if( m_RequestPage < 1 )
	{
		m_RequestPage = 1;
		return;
	}

	if( m_RequestPage > m_totScrollPage )
	{
		m_RequestPage = m_totScrollPage;
		return;
	}

	INT64 tot = _I64_MAX;
	if( m_RankType == ERankType::ERankType_Colosseum && m_pListColosseum )
		tot = (int)m_pListColosseum->nRankingTotalCount;
	else if( m_RankType == ERankType::ERankType_Ladder && m_pListLadder )
		tot = m_pListLadder->nRankingTotalCount;
	
	if( tot < RankingSystem::RANKINGPAGEMAX * RankingSystem::RANKINGPAGECOUNT * (m_RequestPage-1) )
		return;
	
	// Send.
	if( m_RankType == ERankType::ERankType_Colosseum )
		OnSendPvPRankList( m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
	else if( m_RankType == ERankType::ERankType_Ladder )
		OnSendPvPRankLadderList( m_eCurrentLadderType, m_RequestPage, m_Job, m_SubClass, ( m_StrSearch == NULL ? L"" : m_StrSearch ) );
}

void CDnPvPRankDlg::Reset()
{	
	m_pListRank->SelectItem( 0 );
	m_pComboCondition->ClearSelectedItem();
	m_pComboCondition->SetSelectedByIndex( (int)ERankType::ERankType_Colosseum );

	m_pIMEEditBox->SetText( L"" );	

	m_totScrollPage = m_CrrPage = m_RequestPage = 1;
	m_Job = m_SubClass = 0;
	m_StrSearch = NULL;

	int size = (int)m_vecPages.size();
	for( int i=0; i<size; ++i )
		m_vecPages[ i ]->Show( false );


	//for( int i=0; i<RankingSystem::RANKINGPAGECOUNT; ++i )
	//	m_vecPages[ i ]->SetBgTextColor( D3DCOLOR_ARGB(90, 0, 0, 0), UI_STATE_NORMAL );

	//pStaticPage->SetBgTextColor( D3DCOLOR_ARGB(128, 64, 86, 128), UI_STATE_MOUSEENTER );			
	//pStaticPage->SetBgTextColor( D3DCOLOR_ARGB(90, 0, 0, 0), UI_STATE_NORMAL );		
}


void CDnPvPRankDlg::RefreshPageControls()
{
	int size = (int)m_vecPages.size();
	for( int i=0; i<size; ++i )
	{
		if( i == (m_crrSelectPage-1) ) {
			m_vecPages[ i ]->SetBgTextColor( D3DCOLOR_ARGB(90, 164, 32, 48), UI_STATE_MOUSEENTER );
			m_vecPages[ i ]->SetBgTextColor(  D3DCOLOR_ARGB(90, 164, 32, 48) , UI_STATE_NORMAL );
		}
		else {
			m_vecPages[ i ]->SetBgTextColor( D3DCOLOR_ARGB(128, 64, 86, 128), UI_STATE_MOUSEENTER );			
			m_vecPages[ i ]->SetBgTextColor( D3DCOLOR_ARGB(90, 0, 0, 0), UI_STATE_NORMAL );		
		}			
	}


	m_pStaticBack->Enable( true );
	m_pStaticNext->Enable( true );
	m_pStaticFirst->Enable( true );
	m_pStaticLast->Enable( true );

	INT64 totPage = 0;
	if( m_RankType == ERankType::ERankType_Colosseum )
	{
		if( m_pListColosseum )
		{
			totPage = m_pListColosseum->nRankingTotalCount / RankingSystem::RANKINGPAGEMAX;
			//totPage += ( m_pListColosseum->nRankingTotalCount % RankingSystem::RANKINGPAGEMAX ) / RankingSystem::RANKINGPAGEMAX + 1;
			int rest = (int)m_pListColosseum->nRankingTotalCount % RankingSystem::RANKINGPAGEMAX;
			if( rest > 0 )
				rest = rest / RankingSystem::RANKINGPAGEMAX + 1;
			totPage += rest;
		}
		else
		{
			totPage = 1;
		}
	}
	else if( m_RankType == ERankType::ERankType_Ladder )
	{
		if( m_pListLadder )
		{
			totPage = m_pListLadder->nRankingTotalCount / RankingSystem::RANKINGPAGEMAX;
			//totPage += ( m_pListLadder->nRankingTotalCount % RankingSystem::RANKINGPAGEMAX ) / RankingSystem::RANKINGPAGEMAX + 1;
			int rest = (int)m_pListLadder->nRankingTotalCount % RankingSystem::RANKINGPAGEMAX;
			if( rest > 0 )
				rest = rest / RankingSystem::RANKINGPAGEMAX + 1;
			totPage += rest;			
		}
		else
		{
			totPage = 1;
		}
	}

	if( m_CrrPage >= totPage )
	{
		m_CrrPage = (int)totPage;
		m_pStaticNext->Enable( false );
	}

	if( m_CrrPage == 1 )
	{
		m_pStaticBack->Enable( false );  // <
		m_pStaticFirst->Enable( false ); // <<
	}
	
	if( m_RequestPage == 1 )
		m_pStaticFirst->Enable( false ); // <<

	if( m_RequestPage == m_totScrollPage )
		m_pStaticLast->Enable( false ); // >>
}

// Guild mark.
void CDnPvPRankDlg::SetGuildIcon()
{	
	if( CDnActor::s_hLocalActor )
	{
		CDnPlayerActor * pPlayerActor = static_cast< CDnPlayerActor * >( CDnActor::s_hLocalActor.GetPointer() );

		if( pPlayerActor->IsJoinGuild() )
		{			
			if( pPlayerActor->GetGuildSelfView().IsSet() ) 
			{								
				const TGuildView &GuildView = pPlayerActor->GetGuildSelfView();
				if( GetGuildTask().IsShowGuildMark( GuildView ) )
				{
					m_pTextureControlGuild->Show( true );
					EtTextureHandle hGuildMark = GetGuildTask().GetGuildMarkTexture( GuildView );
					m_pTextureControlGuild->SetTexture( hGuildMark );					
				}
			}
		}	
	}

}

// Reset list.
void CDnPvPRankDlg::ResetList()
{
	m_pListRank->RemoveAllItems();

	int size = (int)m_vecPages.size();
	for( int i=1; i<size; ++i )
		m_vecPages[ i ]->Show( false );
}


void CDnPvPRankDlg::ResetPvPList()
{
	m_CrrPage = m_RequestPage = m_totScrollPage = 1;
	if( m_RankType == ERankType::ERankType_Colosseum )
		SAFE_DELETE( m_pListColosseum );
	if( m_RankType == ERankType::ERankType_Ladder )
		SAFE_DELETE( m_pListLadder );

	int size = (int)m_vecPages.size();
	for( int i=0; i<size; ++i )
		m_vecPages[ i ]->Show( false );
	m_vecPages[ 0 ]->Show( true );
	m_vecPages[ 0 ]->SetText( FormatW( L"%d", m_CrrPage) );

	RefreshPageControls();
}

#if defined( PRE_ADD_DWC )
void CDnPvPRankDlg::RefreshDWCRankList()
{
	if( !m_pListRank || !CDnDWCTask::IsActive() )
		return;

	m_pListRank->RemoveAllItems();
	const std::vector<TDWCRankInfo> &vecRanks = GetDWCTask().GetRankList();
	for( size_t i = 0; i < vecRanks.size(); ++i )
	{
		CDnPvPRankListItemDwcDlg *pItem = m_pListRank->AddItem<CDnPvPRankListItemDwcDlg>();
		if( pItem )
			pItem->SetRankInfo( vecRanks[i] );
	}

	if( GetInterface().GetDWCMarkTooltipDlg() && GetDWCTask().HasDWCTeam() )
	{
		const TDWCTeamInfo &teamInfo = GetDWCTask().GetTeamInfo();
		GetInterface().GetDWCMarkTooltipDlg()->SetDWCMarkInfo( teamInfo.wszTeamName, GetDWCTask().GetDWCRank(), teamInfo.nDWCPoint );
	}
}
#endif


void CDnPvPRankDlg::SetJobRank( int job, INT64 classRank, INT64 subClassRank )
{	
	// No record.
	if( job == 0 )
	{
		m_pStaticFirstRank->SetText( L"" );
		m_pStaticSecondRank->SetText( L"" );
		return;
	}

	int parentJob = job;

	int changeClass = 1;
	while( true )
	{
		parentJob = GetParentJob( parentJob );
		if( parentJob == 0 )
			break;

		++changeClass;
	}

	const wchar_t * pStr = GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 120204 ); // "- %s Rank #%d"
	
	// 1st job class.
	if( changeClass == 1 )
	{
		m_pStaticFirstRank->SetText( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( job ), classRank ) );
		m_pStaticSecondRank->SetText( L"" );
	}

	// 2nd job
	else if( changeClass == 2 )
	{
		m_pStaticFirstRank->SetText( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( GetParentJob(job) ), classRank ) );
		m_pStaticSecondRank->SetText( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( job ), subClassRank) );
	}

	// 3rd job.
	else if( changeClass == 3 )
	{
		int paJob = GetParentJob(job);
		m_pStaticFirstRank->SetText( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( GetParentJob( paJob ) ), classRank ) );
		m_pStaticSecondRank->SetText( FormatW( pStr, DN_INTERFACE::STRING::GetJobString( paJob ), subClassRank ) );
	}
}


#endif // PRE_ADD_PVP_RANKING