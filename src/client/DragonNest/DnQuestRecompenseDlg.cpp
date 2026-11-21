#include "StdAfx.h"
#include "DnQuestRecompenseDlg.h"
#include "DnItemSlotButton.h"
#include "DnItemTask.h"
#include "TaskManager.h"
#include "DnCashShopTask.h"
#ifdef PRE_ADD_REMOTE_QUEST
#include "DnQuestTask.h"
#endif // PRE_ADD_REMOTE_QUEST


#ifdef _DEBUG
#define new new(_NORMAL_BLOCK,__FILE__,__LINE__)
#endif

CDnQuestRecompenseDlg::CDnQuestRecompenseDlg( UI_DIALOG_TYPE dialogType, CEtUIDialog *pParentDialog, int nID, CEtUICallback *pCallback  )
	: CDnCustomDlg( dialogType, pParentDialog, nID, pCallback  )
	, m_nSetItemCount( 0 )
	, m_nSetAddItemCount( 0 )
	, m_pStaticGold(NULL)
	, m_pStaticSilver(NULL)
	, m_pStaticComment(NULL)
	, m_pStaticAddTitle( NULL )
	, m_pStaticAddBoard( NULL )
	, m_pStaticAddOnlyTitle( NULL )
	, m_pStaticAddOnlyBoard( NULL )
	, m_pStaticBronze(NULL)
	, m_pStaticExp(NULL)
	, m_pBoardFTG( NULL )
	, m_pTextFTG( NULL )
#ifdef PRE_ADD_REMOTE_QUEST
	, m_nQuestIndex( 0 )
	, m_nTypeRecompense( QuestRecompense_None )
	, m_nAvailableCount( 0 )
#endif // PRE_ADD_REMOTE_QUEST
{
}

CDnQuestRecompenseDlg::~CDnQuestRecompenseDlg(void)
{
	ResetAllData();

#ifdef PRE_ADD_REMOTE_QUEST
	m_vecItemSlotSelectStatic.clear();
#endif // PRE_ADD_REMOTE_QUEST
}

void CDnQuestRecompenseDlg::Initialize(bool bShow)
{
	CEtUIDialog::Initialize( CEtResourceMng::GetInstance().GetFullName( "QuestRecompenseDlg.ui" ).c_str(), bShow );
}

void CDnQuestRecompenseDlg::InitialUpdate()
{
	m_pStaticGold = GetControl<CEtUIStatic>("ID_GOLD");
	m_pStaticSilver = GetControl<CEtUIStatic>("ID_SILVER");
	m_pStaticBronze = GetControl<CEtUIStatic>("ID_BRONZE");
	m_pStaticExp = GetControl<CEtUIStatic>("ID_STATIC_EXP");

	m_pStaticComment = GetControl<CEtUIStatic>("ID_RECOMPENSE_COMMENT");

	m_pStaticAddTitle = GetControl<CEtUIStatic>("ID_RIGHT_BAR");
	m_pStaticAddBoard = GetControl<CEtUIStatic>("ID_RIGHT_BOARD");

	m_pStaticAddOnlyTitle = GetControl<CEtUIStatic>("ID_LEFT_BAR");
	m_pStaticAddOnlyBoard = GetControl<CEtUIStatic>("ID_LEFT_BOARD");

	m_pBoardFTG = GetControl<CEtUIStatic>("ID_STATIC4");
	m_pTextFTG = GetControl<CEtUIStatic>("ID_STATIC_FTG");

#ifdef PRE_ADD_REMOTE_QUEST
	char szControlName[32]={0};
	for( int i=0; i<6; i++ )
	{
		sprintf_s( szControlName, 32, "ID_STATIC_SELECT%d", i + 1 );
		CEtUIStatic* pStaticControl = GetControl<CEtUIStatic>( szControlName );
		if( pStaticControl )
		{
			m_vecItemSlotSelectStatic.push_back( pStaticControl );
			pStaticControl->Show( false );
		}
	}
#endif // PRE_ADD_REMOTE_QUEST
}

void CDnQuestRecompenseDlg::InitCustomControl( CEtUIControl *pControl )
{
	CDnItemSlotButton *pItemSlotButton(NULL);
	pItemSlotButton = static_cast<CDnItemSlotButton*>(pControl);
	pItemSlotButton->SetSlotType( ST_QUEST );

	if( strstr( pControl->GetControlName(), "ID_ITEM" ) )
		m_vecItemSlotButton.push_back(pItemSlotButton);
	else if( strstr( pControl->GetControlName(), "ID_RIGHT_ITEM" ) )
		m_vecAddItemSlotButton.push_back( pItemSlotButton );
	else if( strstr( pControl->GetControlName(), "ID_LEFT_ITEM" ) )
		m_vecOnlyAddItemSlotButton.push_back( pItemSlotButton );
}

void CDnQuestRecompenseDlg::Show( bool bShow ) 
{
	if( m_bShow == bShow )
		return;

	CEtUIDialog::Show( bShow );
}

void CDnQuestRecompenseDlg::ProcessCommand( UINT nCommand, bool bTriggeredByUser, CEtUIControl *pControl, UINT uMsg )
{
	SetCmdControlName( pControl->GetControlName() );
#ifdef PRE_ADD_REMOTE_QUEST
	if( nCommand == EVENT_BUTTON_CLICKED )
	{
		if( m_nTypeRecompense == QuestRecompense_Select && GetQuestTask().IsExistRemoteQuestAskList( m_nQuestIndex, CDnQuestTree::REMOTEQUEST_COMPLETE ) )
		{
			if( GetSlotSelectCount() >= m_nAvailableCount )
			{
				ResetSlotSelect();
			}

			for( int i=0; i<static_cast<int>( m_vecItemSlotButton.size()); i++ )
			{
				float fMouseX, fMouseY;
				GetScreenMouseMovePoints( fMouseX, fMouseY );
				fMouseX -= GetXCoord();
				fMouseY -= GetYCoord();
				SUICoord uiCoord;
				if( m_vecItemSlotButton[i] )
					m_vecItemSlotButton[i]->GetUICoord( uiCoord );

				if( uiCoord.IsInside( fMouseX, fMouseY ) ) 
				{
					CDnItem* pItem = static_cast<CDnItem*>( m_vecItemSlotButton[i]->GetItem() );
					if( pItem && i >= 0 && i < static_cast<int>( m_vecItemSlotSelectStatic.size() ) )
						m_vecItemSlotSelectStatic[i]->Show( true );

					break;
				}
			}
		}

		return;
	}
#else // PRE_ADD_REMOTE_QUEST
	if( nCommand == EVENT_BUTTON_CLICKED )
		return;
#endif // PRE_ADD_REMOTE_QUEST

	CEtUIDialog::ProcessCommand( nCommand, bTriggeredByUser, pControl, uMsg );
}

#ifdef PRE_ADD_REMOTE_QUEST
int CDnQuestRecompenseDlg::GetSlotSelectCount()
{
	int nSlotSelectCount = 0;
	for( int i=0; i<static_cast<int>( m_vecItemSlotSelectStatic.size() ); i++ )
	{
		if( m_vecItemSlotSelectStatic[i] && m_vecItemSlotSelectStatic[i]->IsShow() )
			nSlotSelectCount++;
	}

	return nSlotSelectCount;
}

void CDnQuestRecompenseDlg::ResetSlotSelect()
{
	for( int i=0; i<static_cast<int>( m_vecItemSlotSelectStatic.size() ); i++ )
	{
		if( m_vecItemSlotSelectStatic[i] )
			m_vecItemSlotSelectStatic[i]->Show( false );
	}
}

void CDnQuestRecompenseDlg::GetRemoteQuestRecompenseItemArray( bool itemArray[], bool cashitemArray[] )
{
	if( m_nTypeRecompense == QuestRecompense_Select )
	{
		for( int i=0; i<static_cast<int>( m_vecItemSlotSelectStatic.size() ); i++ )
		{
			itemArray[i] = cashitemArray[i] = false;
			if( m_vecItemSlotSelectStatic[i] == NULL ) continue;
			bool bChecked = m_vecItemSlotSelectStatic[i]->IsShow() ? true : false;
			if( i < m_nRewardCashItemStartSlotIndex )
				itemArray[i] = bChecked;
			else
				cashitemArray[i-m_nRewardCashItemStartSlotIndex] = bChecked;
		}
	}
	else
	{
		for( int i=0; i<static_cast<int>( m_vecItemSlotButton.size() ); i++ )
		{
			itemArray[i] = cashitemArray[i] = false;
			if( m_vecItemSlotButton[i] == NULL ) continue;
			bool bChecked = m_vecItemSlotButton[i]->IsShow() ? true : false;
			if( i < m_nRewardCashItemStartSlotIndex )
				itemArray[i] = bChecked;
			else
				cashitemArray[i-m_nRewardCashItemStartSlotIndex] = bChecked;
		}
	}
}
#endif // PRE_ADD_REMOTE_QUEST

void CDnQuestRecompenseDlg::DeleteVectorItem( std::vector<CDnItemSlotButton*> & vecItemSlotButton, const bool bShow )
{
	for( DWORD itr = 0; itr < vecItemSlotButton.size(); ++itr )
	{
		CDnItem * pItem = (CDnItem *)vecItemSlotButton[itr]->GetItem();
		SAFE_DELETE( pItem );
		vecItemSlotButton[itr]->ResetSlot();
		vecItemSlotButton[itr]->Show( bShow );
	}
}

void CDnQuestRecompenseDlg::ResetAllData()
{
	m_nSetItemCount = 0;
	m_nSetAddItemCount = 0;

	DeleteVectorItem( m_vecItemSlotButton, false );

	m_pStaticAddOnlyTitle->Show( false );
	m_pStaticAddOnlyBoard->Show( false );
	m_pStaticAddTitle->Show( false );
	m_pStaticAddBoard->Show( false );

	DeleteVectorItem( m_vecAddItemSlotButton, false );
	DeleteVectorItem( m_vecOnlyAddItemSlotButton, false );

	m_pBoardFTG->Show( false );
	m_pTextFTG->Show( false );
}

void CDnQuestRecompenseDlg::SetCoinText( const UINT uiCoin )
{
	int nGold = uiCoin/10000;
	int nSilver = (uiCoin%10000)/100;
	int nBronze = uiCoin%100;

	m_pStaticGold->SetIntToText( nGold );
	m_pStaticSilver->SetIntToText( nSilver );
	m_pStaticBronze->SetIntToText( nBronze );
}

void CDnQuestRecompenseDlg::SetExpText( const int nExp )
{
	m_pStaticExp->SetIntToText( nExp );
}

void CDnQuestRecompenseDlg::SetCommentText( const UINT nStringIndex )
{
	m_pStaticComment->ClearText();
	m_pStaticComment->SetText( GetEtUIXML().GetUIString( CEtUIXML::idCategory1, nStringIndex ) );
}

void CDnQuestRecompenseDlg::SetItemOption( const int nTableID, const int nCount, TItemInfo & itemInfo )
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

bool CDnQuestRecompenseDlg::GetNormalItemInfo( const int nItemID, const int nItemCount, OUT TItemInfo & itemInfo )
{
	if( false == CDnItem::MakeItemInfo( nItemID, nItemCount, itemInfo ) )
		return false;

	return true;
}

CDnItem * CDnQuestRecompenseDlg::CreateNormalItem( TItemInfo & itemInfo )
{
	CDnItemTask* pTask = static_cast<CDnItemTask*>(CTaskManager::GetInstance().GetTask("ItemTask"));
	if( !pTask )
		return NULL;

	return pTask->CreateItem( itemInfo );
}

bool CDnQuestRecompenseDlg::GetCashItemInfo( const int nCashItemSN, TItemInfo & itemInfo )
{
	if( nCashItemSN == 0 )
		return false;

	const SCashShopItemInfo* pCashInfo = GetCashShopTask().GetItemInfo( nCashItemSN );
	if( !pCashInfo )
		return false;

	if( CDnItem::MakeItemInfo( pCashInfo->presentItemId, pCashInfo->count, itemInfo ) == false )
		return false;

	return true;
}

CDnItem * CDnQuestRecompenseDlg::CreateCashItem( const int nCashItemSN, TItemInfo & itemInfo )
{
	CDnItem * pItem = CreateNormalItem( itemInfo );

	if( NULL == pItem )
		return NULL;

	pItem->SetCashItemSN( nCashItemSN );

	return pItem;
}

bool CDnQuestRecompenseDlg::SetItemSlot( CDnItem * pItem )
{
	if( NULL == pItem )
		return false;

	if ( m_vecItemSlotButton.size() > m_nSetItemCount )
	{
		m_vecItemSlotButton[m_nSetItemCount]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
		m_vecItemSlotButton[m_nSetItemCount]->Show( true );
	}
	else
		return false;

	++m_nSetItemCount;

	return true;
}

void CDnQuestRecompenseDlg::SetRecompense( TQuestRecompense& table )
{
	ResetAllData();

	CDnItemTask* pTask = static_cast<CDnItemTask*>(CTaskManager::GetInstance().GetTask("ItemTask"));
	if( !pTask ) return;

	SetCoinText( table.nCoin );
	SetExpText( table.nExp );
	SetCommentText( table.nStringIndex );
#ifdef PRE_ADD_REMOTE_QUEST
	if( m_nQuestIndex != table.nQuestIndex )
		ResetSlotSelect();
	m_nQuestIndex = table.nQuestIndex;
	m_nTypeRecompense = table.cType;
	m_nAvailableCount = table.cSelectMax;
#endif // PRE_ADD_REMOTE_QUEST

	SetFTGText( table.nRewardFatigue );

	for( DWORD itr = 0; itr < RECOMPENSE_ITEM_MAX; ++itr )
	{
		TItemInfo itemInfo;
		if( false == GetNormalItemInfo( table.ItemArray[itr].nItemID, table.ItemArray[itr].nItemCount, itemInfo ) )
			continue;

		SetItemOption( table.nIndex, itr, itemInfo );
		CDnItem * pItem = CreateNormalItem( itemInfo );
		if( false == SetItemSlot( pItem ) ) 
			_ASSERT( 0 && "Quest reward : Combined count of normal + cash items exceeded 6, or item creation failed. Check data." );
	}
#ifdef PRE_ADD_REMOTE_QUEST
	m_nRewardCashItemStartSlotIndex = m_nSetItemCount;
#endif // PRE_ADD_REMOTE_QUEST
	for( DWORD itr = 0; itr < RECOMPENSE_ITEM_MAX; ++itr )
	{
		TItemInfo itemInfo;
		if( false == GetCashItemInfo( table.CashItemSNArray[itr], itemInfo ) ) 
			continue;

		CDnItem *pItem = CreateCashItem( table.CashItemSNArray[itr], itemInfo );
		if( false == SetItemSlot( pItem ) ) 
			_ASSERT( 0 && "Quest reward : Combined count of normal + cash items exceeded 6, or item creation failed. Check data." );
	}
}

void CDnQuestRecompenseDlg::InitAddRecompenseUI()
{
	if( 0 == m_nSetItemCount )
	{
		m_pStaticAddOnlyTitle->Show( true );
		m_pStaticAddOnlyBoard->Show( true );
	}
	else
	{
		m_pStaticAddTitle->Show( true );
		m_pStaticAddBoard->Show( true );
	}
}

bool CDnQuestRecompenseDlg::SetAddItemSlot( CDnItem * pItem, std::vector<CDnItemSlotButton*> & vecItemSlotButton )
{
	if( NULL == pItem )
		return false;

	if ( vecItemSlotButton.size() > m_nSetAddItemCount )
	{
		vecItemSlotButton[m_nSetAddItemCount]->SetItem(pItem, CDnSlotButton::ITEM_ORIGINAL_COUNT);
		vecItemSlotButton[m_nSetAddItemCount]->Show( true );
	}
	else
		return false;

	++m_nSetAddItemCount;

	return true;
}

void CDnQuestRecompenseDlg::SetLevelCapRecompense( const TQuestRecompense & questRecompese, const TQuestLevelCapRecompense & questLevelCapRecompense )
{
	InitAddRecompenseUI();
	SetCommentText( questRecompese.nLevelCapStringIndex );

	for( DWORD itr = 0; itr < MAX_QUEST_LEVEL_CAP_REWARD; ++itr )
	{
		TItemInfo itemInfo;
		if( false == GetNormalItemInfo( questLevelCapRecompense.RecompenseItem[itr].nItemID, questLevelCapRecompense.RecompenseItem[itr].nItemCount, itemInfo ) )
			continue;

		CDnItem * pItem = CreateNormalItem( itemInfo );
		if( false == SetAddItemSlot( pItem, m_nSetItemCount == 0 ? m_vecOnlyAddItemSlotButton : m_vecAddItemSlotButton ) )
			_ASSERT( 0 && "Quest bonus reward : Combined count of normal + cash items exceeded 6, or item creation failed. Check data." );
	}
}

void CDnQuestRecompenseDlg::SetFTGText( const int nFTG )
{
	if( NULL == m_pBoardFTG || NULL == m_pTextFTG || 0 >= nFTG )
		return;

	m_pBoardFTG->Show( true );
	m_pTextFTG->Show( true );

	m_pTextFTG->SetIntToText( nFTG );
}
