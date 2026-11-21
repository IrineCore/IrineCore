#include "StdAfx.h"
#include "DnDungeonExpectQuestDlg.h"
#include "DnDungeonExpectDlg.h"
#include "DnQuestTask.h"
#include "DnItemTask.h"
#include "TaskManager.h"
#include "DnCashShopTask.h"

#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif 


CDnDungeonExpectQuestDlg::CDnDungeonExpectQuestDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback )
, m_pQuestListBox( NULL )
, m_pDnDungeonExpectQuestRewardDlg( NULL )
{
}

CDnDungeonExpectQuestDlg::~CDnDungeonExpectQuestDlg(void)
{
	m_pQuestListBox->RemoveAllItems();
	SAFE_DELETE( m_pDnDungeonExpectQuestRewardDlg );
}

void CDnDungeonExpectQuestDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabListDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectQuestDlg::InitialUpdate()
{
	m_pQuestListBox = GetControl<CEtUIListBoxEx>( "ID_LISTBOXEX_LIST" );

	m_pDnDungeonExpectQuestRewardDlg = new CDnDungeonExpectQuestRewardDlg( UI_TYPE_CHILD, this );
	m_pDnDungeonExpectQuestRewardDlg->Initialize( false );
}

void CDnDungeonExpectQuestDlg::Process( float fElapsedTime )
{
	CEtUIDialog::Process( fElapsedTime );
}

void CDnDungeonExpectQuestDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );

	if( nCommand == EVENT_LISTBOX_SELECTION )
	{
		if( strstr( pControl->GetControlName(), "ID_LISTBOXEX_LIST" ) )
		{
			SListBoxItem* pItem = m_pQuestListBox->GetSelectedItem();
			if( pItem )
			{
				CDnDungeonExpectQuestItemDlg* pDnDungeonExpectQuestItemDlg = static_cast<CDnDungeonExpectQuestItemDlg*>( pItem->pData );

				if( pDnDungeonExpectQuestItemDlg && pDnDungeonExpectQuestItemDlg->GetQuestIndex() > -1 )
				{
					Journal* pJournal = g_DataManager.GetJournalData( pDnDungeonExpectQuestItemDlg->GetQuestIndex() );
					if( !pJournal ) return;
					if( !CDnActor::s_hLocalActor ) return;

					int nRecompenseID = pJournal->GetMyRecompenseID( CDnActor::s_hLocalActor->GetClassID() );
					if( nRecompenseID != 0 )
					{
						TQuestRecompense table;
						g_DataManager.GetQuestRecompense( nRecompenseID, table );
						m_pDnDungeonExpectQuestRewardDlg->SetReward( table );

						SUICoord uiCoordReward, uiCoordItem;
						m_pDnDungeonExpectQuestRewardDlg->GetDlgCoord( uiCoordReward );
						pDnDungeonExpectQuestItemDlg->GetDlgCoord( uiCoordItem );
						uiCoordReward.fX = uiCoordItem.fX - uiCoordReward.fWidth;
						uiCoordReward.fY = uiCoordItem.fY;
						m_pDnDungeonExpectQuestRewardDlg->SetDlgCoord( uiCoordReward );

						ShowChildDialog( m_pDnDungeonExpectQuestRewardDlg, true );
					}
				}
			}
		}
	}

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

void CDnDungeonExpectQuestDlg::Show( bool bShow )
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		ShowChildDialog( m_pDnDungeonExpectQuestRewardDlg, false );
		m_pQuestListBox->DeselectItem();
	}

	CEtUIDialog::Show( bShow );
}

bool CDnDungeonExpectQuestDlg::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( !IsShow() )
		return false;

	if( m_pDnDungeonExpectQuestRewardDlg->IsShow() )
	{
		if( ( uMsg == WM_LBUTTONDOWN && !m_pDnDungeonExpectQuestRewardDlg->IsMouseInDlg() ) || uMsg == WM_MOUSEWHEEL )
		{
			m_pDnDungeonExpectQuestRewardDlg->Show( false );
		}
	}

	return CEtUIDialog::MsgProc( hWnd, uMsg, wParam, lParam );
}

void CDnDungeonExpectQuestDlg::SetQuestList( std::vector<TQuest*>& vecQuestList, int nMapIndex, int nDifficult )
{
	DNTableFileFormat* pMapSox = GetDNTable( CDnTableDB::TMAP );
	if( !pMapSox )
		return;

#ifdef PRE_ADD_RENEW_RANDOM_MAP
	DNTableCell* pField = pMapSox->GetFieldFromLablePtr( nMapIndex , "_MapNameID");
	int nMapTableStringId = pField->GetInteger();
	const std::vector<int>* pMapGroupList = CDnWorldDataManager::GetInstance().GetMapGroupByName( nMapTableStringId );
#endif

	std::vector<TQuest*>::iterator iter = vecQuestList.begin();
	for( ; iter != vecQuestList.end(); iter++ )
	{
		Journal* pJournal = g_DataManager.GetJournalData( (*iter)->nQuestID );
		if( !pJournal )
			continue;

		JournalPage* pJournalPage = pJournal->FindJournalPage( (*iter)->cQuestJournal );
		if( !pJournalPage )
			continue;

		bool bFindQuest = false;
		bFindQuest = CheckAvailableQuest( pJournalPage , nMapIndex );

#ifdef PRE_ADD_RENEW_RANDOM_MAP
		if( bFindQuest == false )
		{
			std::vector<int>::const_iterator iter = pMapGroupList->begin();
			for( iter = pMapGroupList->begin(); iter != pMapGroupList->end(); ++iter )
			{
				int nGroupMapIndex = (*iter);
				bFindQuest = CheckAvailableQuest( pJournalPage, nGroupMapIndex );
				if( bFindQuest == true ) break;
			}
		}
#endif

		if( bFindQuest )	// Difficulty check
		{
			int nRecompenseID = pJournal->GetMyRecompenseID( CDnActor::s_hLocalActor->GetClassID() );
			if( nRecompenseID != 0 )
			{
				TQuestRecompense questRecompense;

				if( g_DataManager.GetQuestRecompense( nRecompenseID, questRecompense ) )
				{
					switch( questRecompense.nOperator )
					{
						case 0:	if( questRecompense.nDifficult != nDifficult ) bFindQuest = false; break;
						case 1:	if( questRecompense.nDifficult == nDifficult ) bFindQuest = false; break;
						case 2:	if( questRecompense.nDifficult >= nDifficult ) bFindQuest = false; break;
						case 3:	if( questRecompense.nDifficult <= nDifficult ) bFindQuest = false; break;
						case 4:	if( questRecompense.nDifficult > nDifficult ) bFindQuest = false; break;
						case 5:	if( questRecompense.nDifficult < nDifficult ) bFindQuest = false; break;
					}
				}
			}
		}

		if( bFindQuest )
		{
			CDnDungeonExpectQuestItemDlg* pItem = m_pQuestListBox->AddItem<CDnDungeonExpectQuestItemDlg>();

			std::vector< boost::tuple<std::wstring, int, int> > vecGetList;
			std::wstring szProgress;
			GetQuestTask().GetCurJournalCountingInfoString( pJournal->nQuestIndex, pJournalPage->nJournalPageIndex, vecGetList, szProgress );
			pItem->SetInfo( true, pJournal->wszQuestTitle.c_str(), pJournalPage->szDestination.c_str(), vecGetList, szProgress );
			pItem->SetQuestIndex( pJournal->nQuestIndex );
		}
	}
}

bool CDnDungeonExpectQuestDlg::CheckAvailableQuest( JournalPage* pJournalPage, int nMapIndex )
{
	if( !pJournalPage )
		return false;

	bool bFindQuest = false;

	if( pJournalPage->nDestnationMapIndex == nMapIndex )
	{
		bFindQuest = true;
	}
	else
	{
		for( int i=0; i<(int)pJournalPage->CustomDestnationMark.size(); i++ )
		{
			if( pJournalPage->CustomDestnationMark[i].nMapIdx == nMapIndex )
			{
				bFindQuest = true;
				break;
			}
		}
	}

	return bFindQuest;
}

int CDnDungeonExpectQuestDlg::LoadQuestInCurrentMap( int nMapIndex, int nDifficult )
{
	CDnDungeonExpectDlg* pParentDlg = static_cast<CDnDungeonExpectDlg*>( GetParentDialog() );

	if( pParentDlg == NULL )
		return 0;

	m_pQuestListBox->RemoveAllItems();

	std::vector<TQuest*> vecProgMainQuest;
	GetQuestTask().GetPlayMainQuest( vecProgMainQuest );
	SetQuestList( vecProgMainQuest, nMapIndex, nDifficult );
	std::vector<TQuest*> vecProgSubQuest;
	GetQuestTask().GetPlaySubQuest( vecProgSubQuest );
	SetQuestList( vecProgSubQuest, nMapIndex, nDifficult );

	return m_pQuestListBox->GetSize();
}


//////////////////////////////////////////////////////////////////////////
// class CDnDungeonExpectQuestItemDlg
//////////////////////////////////////////////////////////////////////////

CDnDungeonExpectQuestItemDlg::CDnDungeonExpectQuestItemDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticMainTitle( NULL )
, m_pStaticSubTitle( NULL )
, m_pStaticMainIcon( NULL )
, m_pStaticSubIcon( NULL )
, m_pStaticMissinoIcon( NULL )
, m_pStaticSelect( NULL )
, m_pTextBoxDescription( NULL )
, m_nQuestIndex( -1 )
{
}

CDnDungeonExpectQuestItemDlg::~CDnDungeonExpectQuestItemDlg( void )
{
}

void CDnDungeonExpectQuestItemDlg::Initialize( bool bShow )
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabSubListDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectQuestItemDlg::InitialUpdate()
{
	m_pStaticMainTitle = GetControl<CEtUIStatic>( "ID_MAIN_TITLE" );
	m_pStaticSubTitle = GetControl<CEtUIStatic>( "ID_SUB_TITLE" );
	m_pTextBoxDescription = GetControl<CEtUITextBox>( "ID_MIS_TEXTBOX" );

	m_pStaticMainIcon = GetControl<CEtUIStatic>( "ID_MAIN_ICON" );
	m_pStaticMainIcon->Show( false );
	m_pStaticSubIcon = GetControl<CEtUIStatic>( "ID_SUB0_ICON" );
	m_pStaticSubIcon->Show( false );
	m_pStaticMissinoIcon = GetControl<CEtUIStatic>( "ID_MIS_ICON" );
	m_pStaticMissinoIcon->Show( false );
	m_pStaticSelect = GetControl<CEtUIStatic>( "ID_STATIC_SELSCT" );
	m_pStaticSelect->Show( false );
}

void CDnDungeonExpectQuestItemDlg::SetInfo( bool bMainQuest, const wchar_t *szTitle, const wchar_t *szDest, std::vector< boost::tuple<std::wstring, int, int> > &vecGetList, std::wstring szProgress )
{
	m_pStaticMainTitle->ClearText();
	m_pStaticSubTitle->ClearText();
	m_pTextBoxDescription->ClearText();
	m_pTextBoxDescription->SetTooltipText( L"" );

	wchar_t szTemp[ 256 ] = {0, };
	swprintf_s( szTemp, _countof(szTemp), L"%s", szTitle );
	if( bMainQuest )
	{
		m_pStaticMainIcon->Show( true );
		m_pStaticMainTitle->SetTextWithEllipsis( szTemp, L"..." );
	}
	else
	{
		m_pStaticSubIcon->Show( true );
		m_pStaticSubTitle->SetTextWithEllipsis( szTemp, L"..." );
	}

	m_pTextBoxDescription->AppendText( szDest );

	if( vecGetList.size() == 1 ) 
	{
		int nCnt = vecGetList[0].get<1>();
		int nTotalCnt = vecGetList[0].get<2>();
		if( nCnt < 0 && nTotalCnt < 0 )
			swprintf_s( szTemp, L"%s", vecGetList[0].get<0>().c_str() );			
		else if( nTotalCnt < 0 )
			swprintf_s( szTemp, L"- %s [%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>()  );
		else
			swprintf_s( szTemp, L"- %s [%d/%d]", vecGetList[0].get<0>().c_str(), vecGetList[0].get<1>(), vecGetList[0].get<2>() );						
		
		m_pTextBoxDescription->AddText( szTemp );
	}
	else if( !vecGetList.empty() ) 
	{
		int nItemCnt = 0;
		int nItemTotalCnt = 0;
		std::wstring szGetString;
		int nMaxLen = 0;
		for each( boost::tuple<std::wstring, int, int> e in vecGetList )
		{
			if( e.get<1>() < 0 ) continue;
			
			int nTotalCount = e.get<2>();
			nItemCnt += e.get<1>();
			nItemTotalCnt += nTotalCount < 0 ? 1 : nTotalCount;
			
			if( nTotalCount < 0 ) 
			{
				std::wstring szAdd = FormatW( L"  %s [%d]    \n",  e.get<0>().c_str(), e.get<1>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length());
				szGetString += szAdd;
			}
			else 
			{
				std::wstring szAdd = FormatW( L"  %s [%d/%d]    \n",  e.get<0>().c_str(), e.get<1>(), e.get<2>() );
				nMaxLen = EtMax( nMaxLen, (int)szAdd.length());
				szGetString += szAdd;
			}
		}

		for( int i = 0; i < 5*nMaxLen/2; i++)
			szGetString += L" ";

		szGetString += L"\n";
		
		if( nItemTotalCnt != 0 &&  nItemCnt <= nItemTotalCnt) // #22983 Item count never exceeds the maximum value.
		{
			m_pTextBoxDescription->AddText( FormatW( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, 422 ), 100 * nItemCnt / nItemTotalCnt ).c_str(), textcolor::YELLOW );
			m_pTextBoxDescription->SetTooltipText( szGetString.c_str() );
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// class CDnDungeonExpectQuestRewardDlg
//////////////////////////////////////////////////////////////////////////

CDnDungeonExpectQuestRewardDlg::CDnDungeonExpectQuestRewardDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
, m_pStaticBoard1( NULL )
, m_pStaticBottom1( NULL )
, m_pStaticBoard2( NULL )
, m_pStaticBottom2( NULL )
, m_pStaticBoard3( NULL )
, m_pStaticBottom3( NULL )
, m_pStaticGold( NULL )
, m_pStaticSilver( NULL )
, m_pStaticBronze( NULL )
, m_pStaticExp( NULL )
{
}

CDnDungeonExpectQuestRewardDlg::~CDnDungeonExpectQuestRewardDlg()
{
	DeleteAllItem();
}

void CDnDungeonExpectQuestRewardDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "DungeonEnterTabQuestTipDlg.ui" ).c_str(), bShow );
}

void CDnDungeonExpectQuestRewardDlg::InitialUpdate()
{
	m_pStaticBoard1 = GetControl<CEtUIStatic>( "ID_STATIC_BOARD0" );
	m_pStaticBottom1 = GetControl<CEtUIStatic>( "ID_STATIC_BOTTOM0" );
	m_pStaticBoard2 = GetControl<CEtUIStatic>( "ID_STATIC_BOARD1" );
	m_pStaticBottom2 = GetControl<CEtUIStatic>( "ID_STATIC_BOTTOM1" );
	m_pStaticBoard3 = GetControl<CEtUIStatic>( "ID_STATIC_BOARD2" );
	m_pStaticBottom3 = GetControl<CEtUIStatic>( "ID_STATIC_BOTTOM2" );

	m_pStaticGold = GetControl<CEtUIStatic>( "ID_GOLD" );
	m_pStaticSilver = GetControl<CEtUIStatic>( "ID_SILVER" );
	m_pStaticBronze = GetControl<CEtUIStatic>( "ID_BRONZE" );
	m_pStaticExp = GetControl<CEtUIStatic>( "ID_STATIC_EXP" );
}

void CDnDungeonExpectQuestRewardDlg::InitCustomControl( CEtUIControl *pControl )
{
	CDnItemSlotButton* pItemSlotButton( NULL );
	pItemSlotButton = static_cast<CDnItemSlotButton*>( pControl );
	pItemSlotButton->SetSlotType( ST_QUEST );

	m_vecItemSlotButton.push_back( pItemSlotButton );
}

void CDnDungeonExpectQuestRewardDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	if( !bShow )
	{
		m_pStaticGold->SetText( L"" );
		m_pStaticSilver->SetText( L"" );
		m_pStaticBronze->SetText( L"" );
		m_pStaticExp->SetText( L"" );

		DeleteAllItem();
	}

	CEtUIDialog::Show( bShow );
}

void CDnDungeonExpectQuestRewardDlg::DeleteAllItem()
{
	for( int i=0; i<(int)m_vecItemSlotButton.size(); i++ )
	{
		CDnItem* pItem = (CDnItem*)m_vecItemSlotButton[i]->GetItem();
		SAFE_DELETE( pItem );
		m_vecItemSlotButton[i]->ResetSlot();
	}
}

void CDnDungeonExpectQuestRewardDlg::SetItemOption( const int nTableID, const int nCount, TItemInfo & itemInfo )
{
	DNTableFileFormat* pSoxRecompense = GetDNTable( CDnTableDB::TQUESTREWARD );

	if( pSoxRecompense && pSoxRecompense->IsExistItem( nTableID ) )
	{
		int nRecompsendOptionIndex = pSoxRecompense->GetFieldFromLablePtr( nTableID, FormatA("_ItemOptionID%d", nCount+1).c_str() )->GetInteger();

		if(nRecompsendOptionIndex > 0)
		{
			DNTableFileFormat* pSoxRewardOption = GetDNTable( CDnTableDB::TITEMOPTION );

			if( pSoxRewardOption && pSoxRewardOption->IsExistItem(nRecompsendOptionIndex) )
			{
				itemInfo.Item.cLevel = pSoxRewardOption->GetFieldFromLablePtr( nRecompsendOptionIndex , "_Enchant" )->GetInteger();
				itemInfo.Item.cOption = pSoxRewardOption->GetFieldFromLablePtr( nRecompsendOptionIndex , "_Potential1" )->GetInteger();
				itemInfo.Item.cPotential = pSoxRewardOption->GetFieldFromLablePtr( nRecompsendOptionIndex , "Potential2" )->GetInteger();
			}
		}
	}
}

void CDnDungeonExpectQuestRewardDlg::SetReward( TQuestRecompense& table )
{
	DeleteAllItem();

	CDnItemTask* pTask = static_cast<CDnItemTask*>( CTaskManager::GetInstance().GetTask( "ItemTask" ) );
	if( !pTask ) return;

	int nGold = table.nCoin/10000;
	int nSilver = (table.nCoin%10000)/100;
	int nBronze = table.nCoin%100;

	m_pStaticGold->SetIntToText( nGold );
	m_pStaticSilver->SetIntToText( nSilver );
	m_pStaticBronze->SetIntToText( nBronze );
	m_pStaticExp->SetIntToText( table.nExp );

	int nRewardItemCount = 0;
	// Show normal item rewards first, then cash items. Total does not exceed RECOMPENSE_ITEM_MAX.
	int nLastRewardItemIndex = -1;
	for( int i=0; i<RECOMPENSE_ITEM_MAX; i++ )
	{
		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( table.ItemArray[i].nItemID, table.ItemArray[i].nItemCount, itemInfo ) == false ) continue;

		SetItemOption( table.nIndex, i, itemInfo );

		CDnItem *pItem = pTask->CreateItem( itemInfo );
		if( pItem ) 
		{
			if( int(m_vecItemSlotButton.size()) > i )
			{
				m_vecItemSlotButton[i]->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
				nRewardItemCount++;
			}
			nLastRewardItemIndex = i;
		}
	}


	int nStartSlotIndex = nLastRewardItemIndex+1;
	for( int i=0; i<RECOMPENSE_ITEM_MAX; i++ )
	{
		if( table.CashItemSNArray[i] == 0 ) continue;
		const SCashShopItemInfo* pCashInfo = GetCashShopTask().GetItemInfo( table.CashItemSNArray[i] );
		if( !pCashInfo ) continue;

		TItemInfo itemInfo;
		if( CDnItem::MakeItemInfo( pCashInfo->presentItemId, pCashInfo->count, itemInfo ) == false ) continue;
		CDnItem *pItem = pTask->CreateItem( itemInfo );
		if( pItem ) 
		{
			pItem->SetCashItemSN( table.CashItemSNArray[i] );

			if( int(m_vecItemSlotButton.size()) > i+nStartSlotIndex )
			{
				m_vecItemSlotButton[i+nStartSlotIndex]->SetItem( pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT );
				nRewardItemCount++;
			}
#ifndef _FINAL_BUILD
			else
				_ASSERT(0&&"Quest reward: combined count of normal + cash items exceeds 6. Check the data.");
#endif // _FINAL_BUILD
		}
	}

	if( nRewardItemCount == 0 )
	{
		m_pStaticBoard1->Show( true );
		m_pStaticBottom1->Show( true );
		m_pStaticBoard2->Show( false );
		m_pStaticBottom2->Show( false );
		m_pStaticBoard3->Show( false );
		m_pStaticBottom3->Show( false );
	}
	else if( nRewardItemCount > 0 && nRewardItemCount <= 3 )
	{
		m_pStaticBoard1->Show( false );
		m_pStaticBottom1->Show( false );
		m_pStaticBoard2->Show( true );
		m_pStaticBottom2->Show( true );
		m_pStaticBoard3->Show( false );
		m_pStaticBottom3->Show( false );
	}
	else if( nRewardItemCount > 3 )
	{
		m_pStaticBoard1->Show( false );
		m_pStaticBottom1->Show( false );
		m_pStaticBoard2->Show( false );
		m_pStaticBottom2->Show( false );
		m_pStaticBoard3->Show( true );
		m_pStaticBottom3->Show( true );
	}
}
